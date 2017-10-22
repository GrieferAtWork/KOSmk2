/* Copyright (c) 2017 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_MODULES_BLOCK_ATA_C
#define GUARD_MODULES_BLOCK_ATA_C 1
#define _KOS_SOURCE 1

#include <dev/blkdev.h>
#include <dev/rtc.h>
#include <errno.h>
#include <hybrid/minmax.h>
#include <hybrid/compiler.h>
#include <hybrid/sched/yield.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <limits.h>
#include <malloc.h>
#include <modules/ata.h>
#include <sched/cpu.h>
#include <sched/rpc.h>
#include <sched/smp.h>
#include <sched/task.h>
#include <sync/rwlock.h>
#include <sync/sem.h>
#include <sys/io.h>
#include <stdbool.h>
#include <sys/syslog.h>
#include <kernel/paging.h>
#include <kernel/user.h>

DECL_BEGIN

#define ata_insw(port,addr,size)  insw(port,addr,size)
#define ata_outsw(port,addr,size) outsw(port,addr,size)

STATIC_ASSERT(sizeof(struct ata_spec) == 512);
typedef struct atablkdev ata_t;

#define ATA_ISPRIMARY(iobase) ((iobase) == ATA_PRIMARY)
#define ATA_ISMASTER(bus)     ((bus) == ATA_DRIVE_MASTER)

struct bus {
 rwlock_t    b_lock;     /*< Lock for this bus. (Held for the duration of read/write operations being performed) */
 sem_t       b_signaled; /*< Signal broadcast when an IRQ signalled completion. */
 WEAK ata_t *b_devs[2];  /*< [0..1][2][lock(b_lock)] ATA devices present on this bus. */
 WEAK ata_t *b_active;   /*< [0..1][valid_if(rwlock_writing(&b_lock))] The device currently performing work. */
};

/* Global ATA bus control. */
PRIVATE struct bus busses[2] = {
    [0 ... 1] = {
        .b_lock     = RWLOCK_INIT,
        .b_signaled = SEM_INIT(0),
        .b_devs     = {NULL,NULL},
        .b_active   = NULL,
    },
};
#define PRIMARY_BUS    busses[0]
#define SECONDARY_BUS  busses[1]

#define BUS(self)               (&busses[!ATA_ISPRIMARY((self)->a_iobase)])
#define ATA_LOCK(self)          (&BUS(self)->b_lock)
#define ATA(is_primary,is_master) busses[!(is_primary)].b_devs[!!(is_master)]
#define ata_reading(x)            rwlock_reading(ATA_LOCK(x))
#define ata_writing(x)            rwlock_writing(ATA_LOCK(x))
#define ata_tryread(x)            rwlock_tryread(ATA_LOCK(x))
#define ata_trywrite(x)           rwlock_trywrite(ATA_LOCK(x))
#define ata_tryupgrade(x)         rwlock_tryupgrade(ATA_LOCK(x))
#define ata_read(x)               rwlock_read(ATA_LOCK(x))
#define ata_write(x)              rwlock_write(ATA_LOCK(x))
#define ata_upgrade(x)            rwlock_upgrade(ATA_LOCK(x))
#define ata_downgrade(x)          rwlock_downgrade(ATA_LOCK(x))
#define ata_endread(x)            rwlock_endread(ATA_LOCK(x))
#define ata_endwrite(x)           rwlock_endwrite(ATA_LOCK(x))


PRIVATE ATTR_USED void ata0_inthandler(void);
PRIVATE ATTR_USED void ata1_inthandler(void);
INTERN DEFINE_INT_HANDLER(ata0_irqhandler,ata0_inthandler);
INTERN DEFINE_INT_HANDLER(ata1_irqhandler,ata1_inthandler);
PRIVATE ATTR_FREEDATA isr_t const ata0_handler = ISR_DEFAULT(IRQ_PIC2_ATA1,&ata0_irqhandler);
PRIVATE ATTR_FREEDATA isr_t const ata1_handler = ISR_DEFAULT(IRQ_PIC2_ATA2,&ata1_irqhandler);

PRIVATE ATTR_USED void ata0_inthandler(void) {
 if (IRQ_PIC_SPURIOUS(IRQ_PIC2_ATA1)) return;
 /* Primary ATA interrupt */
 inb(ATA_PRIMARY+ATA_STATUS);
 IRQ_PIC_EOI(IRQ_PIC2_ATA1);
 sem_release(&PRIMARY_BUS.b_signaled,1);
}
PRIVATE ATTR_USED void ata1_inthandler(void) {
 if (IRQ_PIC_SPURIOUS(IRQ_PIC2_ATA2)) return;
 /* Secondary ATA interrupt */
 inb(ATA_SECONDARY+ATA_STATUS);
 IRQ_PIC_EOI(IRQ_PIC2_ATA2);
 sem_release(&SECONDARY_BUS.b_signaled,1);
}


PRIVATE void KCALL ata_sleep(u16 bus) {
 /* As recommended here: http://wiki.osdev.org/ATA_PIO_Mode */
 inb(bus+ATA_ALTSTATUS);
 inb(bus+ATA_ALTSTATUS);
 inb(bus+ATA_ALTSTATUS);
 inb(bus+ATA_ALTSTATUS);
}

/* Wait until '(inb(status_port) & mask) == status' for up to 'timeout' ticks.
 * @return: * :         The latest value of the status register.
 * @return: -ETIMEDOUT: The given timeout has expired.
 * @return: -EIO:       An I/O error occurred.
 */
PRIVATE errno_t KCALL
ata_status_wait(u16 status_port, u8 mask, u8 status, jtime_t timeout) {
 jtime_t start = jiffies; u8 curr;
 for (;;) {
  curr = inb(status_port);
  if (curr&ATA_SR_ERR) return -EIO;
  if ((curr & mask) == status) break;
  if ((jiffies-start) >= timeout) return -ETIMEDOUT;
 }
 return (errno_t)(u32)curr;
}


PRIVATE ssize_t KCALL ata_lba48_read(struct blkdev *__restrict self, blkaddr_t block, USER void *buf, size_t n_blocks);
PRIVATE ssize_t KCALL ata_lba48_write(struct blkdev *__restrict self, blkaddr_t block, USER void const *buf, size_t n_blocks);
PRIVATE ssize_t KCALL ata_lba28_read(struct blkdev *__restrict self, blkaddr_t block, USER void *buf, size_t n_blocks);
PRIVATE ssize_t KCALL ata_lba28_write(struct blkdev *__restrict self, blkaddr_t block, USER void const *buf, size_t n_blocks);
PRIVATE ssize_t KCALL ata_chs_read(struct blkdev *__restrict self, blkaddr_t block, USER void *buf, size_t n_blocks);
PRIVATE ssize_t KCALL ata_chs_write(struct blkdev *__restrict self, blkaddr_t block, USER void const *buf, size_t n_blocks);

#define SELF container_of(self,ata_t,a_device)
PRIVATE void KCALL ata_fini(struct blkdev *__restrict self) {
 struct bus *b = &busses[!ATA_ISPRIMARY(SELF->a_iobase)];
 /* Delete the global ATA hook for this device. */
 assert(b->b_devs[ATA_ISMASTER(SELF->a_drive)] == SELF);
 assert(b->b_active != SELF);
 ATOMIC_WRITE(b->b_devs[ATA_ISMASTER(SELF->a_drive)],NULL);
}
#undef SELF







PRIVATE ATTR_FREERODATA dev_t const ata_devno[2][2] = {
 { ATA_SECONDARY_SLAVE, ATA_SECONDARY_MASTER },
 { ATA_PRIMARY_SLAVE, ATA_PRIMARY_MASTER },
};

/* Register an HD/CDROM drive. */
PRIVATE ATTR_FREETEXT errno_t KCALL /* hd%c */
ata_register(u16 iobase, u16 ctrl, u8 drive, bool is_atapi) {
 errno_t error; ata_t *self;
 if (is_atapi) return -ENOSYS; /* I don't fully understand this yet... */

 self = (ata_t *)blkdev_new(sizeof(ata_t));
 if unlikely(!self) return -ENOMEM;

 self->a_iobase     = iobase;
 self->a_ctrl       = ctrl;
 self->a_drive      = drive;
 self->a_io_timeout = ATA_DEFAULT_IO_TIMEOUT;
 self->a_cm_timeout = ATA_DEFAULT_CM_TIMEOUT;

#if 0 /* Already done by the caller */
 outb(iobase+ATA_HDDEVSEL,drive);
 ata_sleep(iobase);
#endif

 outb(iobase+ATA_FEATURES,1); /* ??? */

 /* Initiate an identify command to we can learn what this thing can do. */
 outb(iobase+ATA_CMD,is_atapi ? ATA_CMD_IDENTIFY_PACKET : ATA_CMD_IDENTIFY);
 ata_sleep(iobase);

 /* Wait for the command to go through. */
 error = ata_status_wait(ctrl,ATA_SR_BSY,0,
                         self->a_cm_timeout);
 if (E_ISERR(error)) goto err;

 /* Read device specifications. */
 ata_insw(iobase+ATA_DATA,&self->a_spec,256);

 self->a_device.bd_blocksize = ATA_BLOCKSIZE;
#if 1
 if (self->a_spec.CommandSetActive.BigLba) {
#if 1 /* Prefer using 28-bit LBA if possible. (It's faster) */
  if (self->a_spec.Capabilities.LbaSupported &&
     (u64)self->a_spec.UserAddressableSectors48 ==
     (u64)self->a_spec.UserAddressableSectors)
      goto use_lba28;
#endif
  self->a_device.bd_blockcount = (blkcnt_t)self->a_spec.UserAddressableSectors48;
  self->a_device.bd_read       = &ata_lba48_read;
  self->a_device.bd_write      = &ata_lba48_write;
 } else if (self->a_spec.Capabilities.LbaSupported) {
use_lba28:
  self->a_device.bd_blockcount = (blkcnt_t)self->a_spec.UserAddressableSectors;
  self->a_device.bd_read       = &ata_lba28_read;
  self->a_device.bd_write      = &ata_lba28_write;
 } else
#endif
 {
  /* Use CHS addressing. */
  self->a_device.bd_blockcount = (blkcnt_t)self->a_number_of_heads*
                                 (blkcnt_t)self->a_cylinders*
                                 (blkcnt_t)self->a_sectors_per_track;
  if (!self->a_device.bd_blockcount) { free(self); return -ENODEV; }
  self->a_device.bd_read  = &ata_chs_read;
  self->a_device.bd_write = &ata_chs_write;
 }

 self->a_device.bd_fini = &ata_fini;

 /* Setup and register the new device. */
 error = device_setup(&self->a_device.bd_device,THIS_INSTANCE);
 if (E_ISERR(error)) goto err;
 error = BLKDEV_REGISTER(&self->a_device,ata_devno[ATA_ISPRIMARY(iobase)][ATA_ISMASTER(drive)]);
 if (E_ISOK(error)) ATA(ATA_ISPRIMARY(iobase),ATA_ISMASTER(drive)) = self;
 BLKDEV_DECREF(&self->a_device);
 blkdev_autopart(&self->a_device,(MINOR(ATA_PRIMARY_SLAVE)-
                                  MINOR(ATA_PRIMARY_MASTER))-1);
 /* Provide this block-device as an alternative to the boot-disk. */
 replace_bootdev(&self->a_device);
 return error;
err:
 free(self);
 return error;
}


PRIVATE ATTR_FREETEXT errno_t KCALL
ata_probe(u16 iobase, u16 ctrl, u8 drive) {
 errno_t error; u8 cl,ch;

 /* Do a soft reset on both ATA device control ports. */
 outb(ctrl,ATA_CTRL_SRST);
 ata_sleep(iobase);
 outb(ctrl,0);
 ata_sleep(iobase);

 /* Select the requested drive. */
 outb(iobase+ATA_HDDEVSEL,drive);
 ata_sleep(iobase);

 /* Wait for the command to be acknowledged. */
 error = ata_status_wait(ctrl,ATA_SR_BSY,0,ATA_DEFAULT_CM_TIMEOUT);
 if (E_ISERR(error)) return -ENODEV;

 /* Figure out what kind of drive this is. */
 cl = inb(iobase+ATA_LBAMD);
 ch = inb(iobase+ATA_LBAHI);

 /* This combination indicates no-device. */
 if (cl == 0xff && ch == 0xff)
     return -ENODEV;

 error = -ENODEV;
 if ((cl == 0x00 && ch == 0x00) || /* PATA */
     (cl == 0x3c && ch == 0xc3))   /* SATA */
      error = ata_register(iobase,ctrl,drive,false);
  
 if ((cl == 0x14 && ch == 0xeb) || /* PATAPI */
     (cl == 0x69 && ch == 0x96))   /* SATAPI */
      error = ata_register(iobase,ctrl,drive,true);

 if (E_ISOK(error)) {
  syslog(LOG_INFO,
         FREESTR("[ATA] New drive registered: %.4I16x:%.2I8x - {%.2I8x,%.2I8x}\n"),
         iobase,drive,cl,ch);
 } else /*if (error != -ENODEV)*/ {
  syslog(LOG_ERROR,
         FREESTR("[ATA] %.4I16x:%.2I8x - {%.2I8x,%.2I8x} error: %[errno]\n"),
         iobase,drive,cl,ch,-error);
 }

 return error;
}


PRIVATE MODULE_INIT void KCALL ata_init(void) {

 irq_vset(BOOTCPU,&ata0_handler,NULL,IRQ_SET_QUICK);
 irq_vset(BOOTCPU,&ata1_handler,NULL,IRQ_SET_RELOAD);

 /* Probe for devices. */
 ata_probe(ATA_SECONDARY,ATA_CTRL_SECONDARY,ATA_DRIVE_SLAVE);
 ata_probe(ATA_SECONDARY,ATA_CTRL_SECONDARY,ATA_DRIVE_MASTER);
 ata_probe(ATA_PRIMARY,ATA_CTRL_PRIMARY,ATA_DRIVE_SLAVE);
 ata_probe(ATA_PRIMARY,ATA_CTRL_PRIMARY,ATA_DRIVE_MASTER);

 /* TODO: Lock read/write functions in-core */

}

DECL_END

#ifndef __INTELLISENSE__
#define WRITE
#define LBA48
#include "ata-io.c.inl"
#define WRITE
#define LBA28
#include "ata-io.c.inl"
#define WRITE
#define CHS
#include "ata-io.c.inl"
#define LBA48
#include "ata-io.c.inl"
#define LBA28
#include "ata-io.c.inl"
#define CHS
#include "ata-io.c.inl"
#endif


#endif /* !GUARD_MODULES_BLOCK_ATA_C */
