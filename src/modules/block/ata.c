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

#include <dev/device.h>
#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/sched/yield.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <kos/syslog.h>
#include <malloc.h>
#include <modules/ata.h>
#include <sched/cpu.h>
#include <sched/rpc.h>
#include <sched/smp.h>
#include <sync/rwlock.h>
#include <sys/io.h>

#define ATA_READONLY 1

DECL_BEGIN

typedef struct atablkdev ata_t;
STATIC_ASSERT(sizeof(struct ata_spec) == 512);

struct ata_descr {
 struct sig    ad_wake; /* Signal broadcast when the ATA sends an interrupt. */
 struct rwlock ad_lock; /* I/O lock for this ATA device. */
};

PRIVATE struct ata_descr ata_descrs[2] = {
    /* ATA_IOPORT_PRIMARY   */{SIG_INIT,RWLOCK_INIT},
    /* ATA_IOPORT_SECONDARY */{SIG_INIT,RWLOCK_INIT},
};
#define ATA_PRIMARY    ata_descrs[0]
#define ATA_SECONDARY  ata_descrs[1]
#define ATA_DESCR(bus) ata_descrs[(bus) < ATA_IOPORT_PRIMARY]

/* ATA interrupt handler. */
PRIVATE ATTR_USED void ata1_inthandler(void);
PRIVATE ATTR_USED void ata2_inthandler(void);
INTERN DEFINE_INT_HANDLER(ata1_irqhandler,ata1_inthandler);
INTERN DEFINE_INT_HANDLER(ata2_irqhandler,ata2_inthandler);

PRIVATE ATTR_FREETEXT REF ata_t *KCALL ata_getprobe(u16 ctrl, ata_bus_t bus, ata_drv_t drive);
PRIVATE ATTR_FREETEXT bool KCALL ata_probe(dev_t id, u16 ctrl, ata_bus_t bus, ata_drv_t drive);
LOCAL void KCALL ata_sleep(ata_bus_t bus);
LOCAL errno_t KCALL ata_poll(ata_bus_t bus, u8 mask, u8 state);

PRIVATE ssize_t KCALL ata_pio28_readlba(ata_t const *__restrict self, blkaddr_t lba, void *__restrict buf, size_t n_blocks);
PRIVATE ssize_t KCALL ata_pio28_writelba(ata_t *__restrict self, blkaddr_t lba, void const *__restrict buf, size_t n_blocks);
PRIVATE ssize_t KCALL ata_pio48_readlba(ata_t const *__restrict self, blkaddr_t lba, void *__restrict buf, size_t n_blocks);
PRIVATE ssize_t KCALL ata_pio48_writelba(ata_t *__restrict self, blkaddr_t lba, void const *__restrict buf, size_t n_blocks);




/* Probe for an ATA device at the given location. */
PRIVATE ATTR_FREETEXT bool KCALL
ata_probe(dev_t id, u16 ctrl, ata_bus_t bus, ata_drv_t drive) {
 ata_t *dev;
 dev = ata_getprobe(ctrl,bus,drive);
 if (dev != E_PTR(-ENODEV)) {
  if (E_ISOK(dev)) {
   errno_t error;
   syslogf(LOG_HW|LOG_INFO,
           FREESTR("[ATA] Created ATA disk driver %[dev_t] at %d/%d (%I64ux%Iu bytes; %s)\n"),
           id,bus,drive,dev->a_device.bd_blockcount,dev->a_device.bd_blocksize,
           dev->a_device.bd_read == (ssize_t (KCALL *)(struct blkdev *__restrict,blkaddr_t,USER void *,size_t))&ata_pio48_readlba
           ? "LBA48" : "LBA");
   error = devns_insert(&ns_blkdev,&dev->a_device.bd_device,id);
   if (E_ISERR(error)) {
    syslogf(LOG_HW|LOG_ERROR,
            FREESTR("[ATA] Failed to register ATA disk driver %[dev_t]: %[errno]\n"),
            id,-error);
   } else {
    blkdev_autopart(&dev->a_device,(MINOR(ATA_PRIMARY_SLAVE)-MINOR(ATA_PRIMARY_MASTER))-1);
   }
   BLKDEV_DECREF(&dev->a_device);
   return true;
  } else {
   syslogf(LOG_HW|LOG_ERROR,
           FREESTR("[ATA] Error while probing ATA device %[dev_t] at %d/%d: %[errno]\n"),
           id,bus,drive,-(intptr_t)dev);
  }
 }
 return false;
}

PRIVATE ATTR_FREEDATA isr_t const ata1_handler = ISR_DEFAULT(IRQ_PIC2_ATA1,&ata1_irqhandler);
PRIVATE ATTR_FREEDATA isr_t const ata2_handler = ISR_DEFAULT(IRQ_PIC2_ATA2,&ata2_irqhandler);

/* Module initialization. */
PRIVATE MODULE_INIT void KCALL ata_init(void) {
 bool found_some;

 /* Setup ATA interrupt handlers. */
 irq_vset(BOOTCPU,&ata1_handler,NULL,IRQ_SET_QUICK);
 irq_vset(BOOTCPU,&ata2_handler,NULL,IRQ_SET_RELOAD);

 /* Probe for ATA devices. */
 found_some  = ata_probe(ATA_PRIMARY_MASTER,  ATA_IOPORT_CTRL_PRIMARY,  ATA_IOPORT_PRIMARY,  ATA_DRIVE_MASTER);
 found_some |= ata_probe(ATA_PRIMARY_SLAVE,   ATA_IOPORT_CTRL_PRIMARY,  ATA_IOPORT_PRIMARY,  ATA_DRIVE_SLAVE);
 found_some |= ata_probe(ATA_SECONDARY_MASTER,ATA_IOPORT_CTRL_SECONDARY,ATA_IOPORT_SECONDARY,ATA_DRIVE_MASTER);
 found_some |= ata_probe(ATA_SECONDARY_SLAVE, ATA_IOPORT_CTRL_SECONDARY,ATA_IOPORT_SECONDARY,ATA_DRIVE_SLAVE);
 if (!found_some) {
  syslogf(LOG_HW|LOG_ERROR,FREESTR("[ATA] Failed to detect any ATA device\n"));
  /* TODO: If no devices were found, this entire module can be unloaded. */

  /* Restore the old IRQ handlers. */
  irq_vdel(BOOTCPU,IRQ_PIC2_ATA1);
  irq_vdel(BOOTCPU,IRQ_PIC2_ATA2);
 }
}

PRIVATE MODULE_FINI void KCALL ata_fini(void) {
 devns_erase(&ns_blkdev,ATA_SECONDARY_SLAVE,DEVNS_ERASE_NORMAL);
 devns_erase(&ns_blkdev,ATA_SECONDARY_MASTER,DEVNS_ERASE_NORMAL);
 devns_erase(&ns_blkdev,ATA_PRIMARY_SLAVE,DEVNS_ERASE_NORMAL);
 devns_erase(&ns_blkdev,ATA_PRIMARY_MASTER,DEVNS_ERASE_NORMAL);

 irq_vdel(BOOTCPU,IRQ_PIC2_ATA1);
 irq_vdel(BOOTCPU,IRQ_PIC2_ATA2);
}

LOCAL void KCALL ata_sleep(ata_bus_t bus) {
 /* As recommended here: http://wiki.osdev.org/ATA_PIO_Mode */
 inb(ATA_IOPORT_ALTSTATUS(bus));
 inb(ATA_IOPORT_ALTSTATUS(bus));
 inb(ATA_IOPORT_ALTSTATUS(bus));
 inb(ATA_IOPORT_ALTSTATUS(bus));
}


LOCAL errno_t KCALL ata_poll(ata_bus_t bus, u8 mask, u8 state) {
 u8 status;
 for (;;) {
  status = inb(ATA_IOPORT_STATUS(bus));
  if (status & ATA_STATUS_ERR) {
   syslogf(LOG_HW|LOG_ERROR,
           "[ATA] ERR received while polling (flags = %#I8x)\n",status);
   return -EIO;
  }
  if ((status & mask) == state) break;
 }
 return -EOK;
}

LOCAL errno_t KCALL
ata_poll_timeout(ata_bus_t bus, u8 mask, u8 state, unsigned int attempts) {
 u8 status;
 for (;;) {
  status = inb(ATA_IOPORT_STATUS(bus));
#if 0
  if (status & ATA_STATUS_ERR) {
   syslogf(LOG_HW|LOG_ERROR,
           "[ATA] ERR received while polling (flags = %#I8x)\n",status);
   return -EIO;
  }
#endif
  if ((status & mask) == state) break;
  if (!attempts) return -ETIMEDOUT;
  --attempts;
 }
 return -EOK;
}

PRIVATE void ata1_inthandler(void) {
 if (IRQ_PIC_SPURIOUS(IRQ_PIC2_ATA1)) return;
 IRQ_PIC_EOI(IRQ_PIC2_ATA1);
 /* broadcast the primary ATA interrupt. */
 sig_broadcast(&ATA_PRIMARY.ad_wake);
}
PRIVATE void ata2_inthandler(void) {
 if (IRQ_PIC_SPURIOUS(IRQ_PIC2_ATA2)) return;
 IRQ_PIC_EOI(IRQ_PIC2_ATA2);
 /* broadcast the secondary ATA interrupt. */
 sig_broadcast(&ATA_SECONDARY.ad_wake);
}


PRIVATE ATTR_FREETEXT REF ata_t *KCALL
ata_getprobe(u16 ctrl, ata_bus_t bus, ata_drv_t drive) {
 u8 status,cl,ch; u16 *iter,*end;
 ata_t *result; errno_t error;

 /* Soft-reset the device. */
 outb(ctrl,0x04);
 ata_sleep(bus);
 outb(ctrl,0x00);

 ata_sleep(bus);

 outb(ATA_IOPORT_HDDEVSEL(bus),drive);
 ata_sleep(bus);

 if (E_ISERR(ata_poll_timeout(bus,ATA_STATUS_BSY,0,10000)))
     return E_PTR(-ENODEV);

 cl = inb(ATA_IOPORT_LBAMD(bus)); /* CYL_LO */
 ch = inb(ATA_IOPORT_LBAHI(bus)); /* CYL_HI */
 if ((cl == 0x00 && ch == 0x00) ||
     (cl == 0x3c && ch == 0xc3)) {
  /* Parallel ATA device, or emulated SATA */
  outb(ATA_IOPORT_FEATURES(bus),1);
  outb(ctrl,0x00);
 }

 outb(ATA_IOPORT_HDDEVSEL(bus),drive);
 ata_sleep(bus);
 outb(ATA_IOPORT_SECCOUNT0(bus),0);
 outb(ATA_IOPORT_LBALO(bus),    0);
 outb(ATA_IOPORT_LBAMD(bus),    0);
 outb(ATA_IOPORT_LBAHI(bus),    0);
 outb(ATA_IOPORT_COMMAND(bus),  ATA_CMD_IDENTIFY);

 status = inb(ATA_IOPORT_STATUS(bus));
 if (!status) return E_PTR(-ENODEV); /* No response: not a valid device */

 /* Create the result device. */
 result = (ata_t *)blkdev_new(sizeof(ata_t));
 if unlikely(!result) return E_PTR(-ENOMEM);

 /* Wait until the busy flag goes away. */
 for (;;) {
  /* Make sure this is really an ATA device. */
  if (inb(ATA_IOPORT_LBAMD(bus)) != 0 ||
      inb(ATA_IOPORT_LBAHI(bus)) != 0) {
   free(result);
   return E_PTR(-ENODEV);
  }
  if (!(status & ATA_STATUS_BSY)) break;
  SCHED_YIELD();
  status = inb(ATA_IOPORT_STATUS(bus));
 }

 /* Wait for the DRQ signal. */
 while (!(status & (ATA_STATUS_ERR|ATA_STATUS_DRQ)))
          SCHED_YIELD(),status = inb(ATA_IOPORT_STATUS(bus));
 if (status&ATA_STATUS_ERR) { free(result); return E_PTR(-EIO); }

 end = (iter = (__u16 *)&result->a_spec)+256;
 while (iter != end) *iter++ = inw(ATA_IOPORT_DATA(bus));

 /* Select most appropriate read/write operators */
 result->a_device.bd_blocksize = ATA_BLOCKSIZE;
 /* TODO: Don't use 48-bit LBA if 28-bit is sufficient
  * >> 28-bit is faster and can always be
  *    used when the drive is small enough! */
 if (result->a_spec.CommandSetActive.BigLba) {
  result->a_device.bd_read       = (ssize_t (KCALL *)(struct blkdev *__restrict,blkaddr_t,USER void *,size_t))&ata_pio48_readlba;
  result->a_device.bd_write      = (ssize_t (KCALL *)(struct blkdev *__restrict,blkaddr_t,USER void const *,size_t))&ata_pio48_writelba;
  result->a_device.bd_blockcount = (blkaddr_t)result->a_spec.UserAddressableSectors48;
 } else if (result->a_spec.Capabilities.LbaSupported) {
  result->a_device.bd_read       = (ssize_t (KCALL *)(struct blkdev *__restrict,blkaddr_t,USER void *,size_t))&ata_pio28_readlba;
  result->a_device.bd_write      = (ssize_t (KCALL *)(struct blkdev *__restrict,blkaddr_t,USER void const *,size_t))&ata_pio28_writelba;
  result->a_device.bd_blockcount = (blkaddr_t)result->a_spec.UserAddressableSectors;
 } else {
  free(result);
  syslogf(LOG_HW|LOG_WARN,FREESTR("[ATA] TODO: CHS-compatibility mode\n"));
  return E_PTR(-EINVAL);
 }
 result->a_descr = &ATA_DESCR(bus);
 result->a_bus   = bus;
 result->a_drive = drive;
 result->a_ctrl  = ctrl;
#define NODE result->a_device.bd_device.d_node
 /* For convenience, mirror the device size in node attributes. */
 NODE.i_attr.ia_siz      = (pos_t)(result->a_device.bd_blockcount*ATA_BLOCKSIZE);
 NODE.i_attr_disk.ia_siz = NODE.i_attr.ia_siz;
#undef NODE
 /* Perform final device setup. */
 error = device_setup(&result->a_device.bd_device,THIS_INSTANCE);
 if (E_ISERR(error)) goto err;

 return result;
err:
 free(result);
 return E_PTR(error);
}

DECL_END

#ifndef __INTELLISENSE__
#define PIO48
#include "ata-pio.c.inl"
#include "ata-pio.c.inl"
#endif

#endif /* !GUARD_MODULES_BLOCK_ATA_C */
