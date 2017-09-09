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
#ifndef GUARD_MODULES_PS2_C
#define GUARD_MODULES_PS2_C 1
#define _KOS_SOURCE 1

#include <dev/chrdev.h>
#include <fcntl.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <hybrid/compiler.h>
#include <hybrid/traceback.h>
#include <kernel/arch/cpustate.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <kernel/module.h>
#include <kernel/syslog.h>
#include <kernel/user.h>
#include <malloc.h>
#include <modules/ps2.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <sched/rpc.h>
#include <sched/smp.h>
#include <stddef.h>
#include <sync/sig.h>
#include <sys/io.h>

DECL_BEGIN

PRIVATE struct kbdev *kbd;
INTERN DEFINE_INT_HANDLER(kbd_irq_handler,kbd_int_handler);


PRIVATE ATTR_USED void kbd_int_handler(void) {
 scan_t e;
 ssize_t error;
 if (IRQ_PIC_SPURIOUS(IRQ_PIC1_KBD)) return;

 /* Wait for the keyboard to become ready */
 while (!(inb(PS2_STATUS) & PS2_STATUS_OUTFULL))
          task_yield();

 /* Read the scancode. */
 e = inb(PS2_DATA);
 syslogf(LOG_HW|LOG_INFO,"KEY(%#I8x)\n",e);

 if (SCAN_ISDOWN(e)) {
  if (e == 0x58) { /* F12 */
   syslogf(LOG_DEBUG,"__boottask:\n");
   __assertion_tbprintl((void *)__boottask.t_cstate->host.eip,
                        (void *)__boottask.t_cstate->host.eip,0);
   __assertion_tbprint2((void *)__boottask.t_cstate->host.ebp,0);
   syslogf(LOG_DEBUG,"__bootcpu.c_idle:\n");
   __assertion_tbprintl((void *)__bootcpu.c_idle.t_cstate->host.eip,
                        (void *)__bootcpu.c_idle.t_cstate->host.eip,0);
   __assertion_tbprint2((void *)__bootcpu.c_idle.t_cstate->host.ebp,0);
   syslogf(LOG_DEBUG,"THIS_TASK:\n");
#undef __assertion_tbprint
   __assertion_tbprint();
  }
 }

 /* Broadcast the key event. */
 HOSTMEMORY_BEGIN {
  error = iobuffer_write(&kbd->kb_backlog,&e,sizeof(e),IO_BLOCKNONE);
 }
 HOSTMEMORY_END;
 if (E_ISERR(error)) {
  syslogf(LOG_HW|LOG_ERROR,
          "[KBD] Failed to queue key press %#.2I8x: %[errno]\n",
          e,-error);
 } else if (!error) {
  syslogf(LOG_HW|LOG_WARN,
          "[KBD] Failed to queue key press %#.2I8x: buffer is full\n",e);
 }

end: ATTR_UNUSED;
 IRQ_PIC_EOI(IRQ_PIC1_KBD);
}


struct kbfile {
     struct file   f_file; /*< Underlying file. */
 REF struct kbdev *f_kbd;  /*< [1..1] Connected keyboard device. */
};



#define KBD   container_of(ino,struct kbdev,kb_device.cd_device.d_node)
PRIVATE REF struct file *KCALL
kbd_fopen(struct inode *__restrict ino,
          struct dentry *__restrict node_ent,
          oflag_t oflags) {
 REF struct kbfile *result;
 if ((oflags&O_ACCMODE) != O_RDONLY)
      return E_PTR(-EPERM);
 result = (struct kbfile *)file_new(sizeof(struct kbfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 CHRDEV_INCREF(&KBD->kb_device);
 result->f_kbd = KBD;
 file_setup(&result->f_file,ino,node_ent,oflags);
 return &result->f_file;
}
#undef KBD

#define KBD  (container_of(fp,struct kbfile,f_file)->f_kbd)
PRIVATE void KCALL kbd_fclose(struct inode *__restrict ino,
                              struct file *__restrict fp) {
 CHRDEV_DECREF(&KBD->kb_device);
}


PRIVATE ssize_t KCALL kbd_read(struct file *__restrict fp,
                               USER void *buf, size_t bufsize) {
 /* TODO: Use ioctl() to change data format. */
 return iobuffer_read(&KBD->kb_backlog,buf,bufsize,IO_BLOCKFIRST);
}

PRIVATE struct inodeops kbd_ops = {
    .ino_fopen  = &kbd_fopen,
    .ino_fclose = &kbd_fclose,
    .f_read     = &kbd_read,
};


PRIVATE void KCALL ps2_write_command(u8 b) {
 while (inb(PS2_STATUS) & PS2_STATUS_INFULL);
 outb(PS2_CMD,b);
 while (inb(PS2_STATUS) & PS2_STATUS_INFULL);
}
PRIVATE void KCALL ps2_write_data(u8 b) {
 while (inb(PS2_STATUS) & PS2_STATUS_INFULL);
 outb(PS2_DATA,b);
}
PRIVATE u8 KCALL ps2_read_data(void) {
 while (!(inb(PS2_STATUS) & PS2_STATUS_OUTFULL));
 return inb(PS2_DATA);
}

PRIVATE void KCALL ps2_write(u8 b) {
 u8 resp; /* Write to first PS/2 device. */
 do ps2_write_data(b),
    resp = inb(PS2_DATA);
 while (resp != ACK);
}


PRIVATE isr_t kb_isr = ISR_DEFAULT(IRQ_PIC1_KBD,&kbd_irq_handler);
PRIVATE MODULE_INIT errno_t ps2_init(void) {
 errno_t error; u8 ps_scanset,cfg;
 kbd = (struct kbdev *)chrdev_new(sizeof(struct kbdev));
 if unlikely(!kbd) return -ENOMEM;

 iobuffer_cinit_ex(&kbd->kb_backlog,256);
 kbd->kb_device.cd_device.d_node.i_ops = &kbd_ops;
 error = device_setup(&kbd->kb_device.cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) goto err;

 ps2_write_command(0xad); /* Disable first PS/2 port */
 ps2_write_command(0x20); /* Read RAM #0 */
 cfg = ps2_read_data();
 syslogf(LOG_HW|LOG_WARN,"PS/2 config %#.2I8x\n",cfg);
 cfg &= ~(1 << 6);
 ps2_write_command(0x60); /* Write RAM #0 */
 ps2_write_data(cfg);

 ps2_write_command(0xae); /* Enable first PS/2 port */

 ps2_write(0xff); /* Reset + self-test. */
 ps2_write(0xf6); /* Set defaults. */
 ps2_write(0xf5); /* Disable scanning. */
 ps2_write(0xf0); /* Set scancode set. */
 ps2_write_data(2); /* Set #2 */

 ps2_write(0xf0); /* Get scancode set. */
 ps2_write_data(0); /* ... */
 do ps_scanset = ps2_read_data();
 while (ps_scanset == ACK);
 syslogf(LOG_HW|LOG_WARN,"Scanset byte %#.2I8x\n",ps_scanset);

 if (ps_scanset == 0x41) {
  ps2_write_command(0x20); /* Read RAM #0 */
  cfg = ps2_read_data();
  syslogf(LOG_HW|LOG_WARN,"PS/2 config %#.2I8x\n",cfg);
  cfg |= (1 << 6);
  ps2_write_command(0x60); /* Write RAM #0 */
  ps2_write_data(cfg);
 }

 ps2_write(0xf4); /* Enable scanning. */

 /* Install the keyboard IRQ handler. */
 irq_vset(BOOTCPU,&kb_isr,NULL,IRQ_SET_RELOAD);

 BLKDEV_REGISTER(kbd,PS2_KEYBOARD);
 return -EOK;
err:
 free(kbd);
 return error;
}

PRIVATE MODULE_FINI void ps2_fini(void) {
 irq_del(IRQ_PIC1_KBD,true);
}


DECL_END

#endif /* !GUARD_MODULES_PS2_C */
