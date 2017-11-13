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
#ifndef GUARD_INCLUDE_ARCH_PIC_H
#define GUARD_INCLUDE_ARCH_PIC_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <sys/io.h>
#include <kernel/interrupt.h>
#include <string.h>

DECL_BEGIN

/* PIC (Programmable Interrupt Controller) API. */
#define PIC1       0x20 /* IO base address for master PIC */
#define PIC2       0xa0 /* IO base address for slave PIC */
#define PIC1_CMD   PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_CMD   PIC2
#define PIC2_DATA (PIC2+1)

#define PIC_CMD_EOI     0x20 /* End-of-interrupt command code */
#define ICW1_ICW4       0x01 /* ICW4 (not) needed */
#define ICW1_SINGLE     0x02 /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08 /* Level triggered (edge) mode */
#define ICW1_INIT       0x10 /* Initialization - required! */
#define ICW4_8086       0x01 /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02 /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08 /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0c /* Buffered mode/master */
#define ICW4_SFNM       0x10 /* Special fully nested (not) */
#define PIC_READ_IRR    0x0a /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR    0x0b /* OCW3 irq service next CMD read */

#define PIC1_IRR()  (outb(PIC1_CMD,PIC_READ_IRR),inb(PIC2_CMD))
#define PIC2_IRR()  (outb(PIC2_CMD,PIC_READ_IRR),inb(PIC2_CMD))
#define PIC1_ISR()  (outb(PIC1_CMD,PIC_READ_ISR),inb(PIC2_CMD))
#define PIC2_ISR()  (outb(PIC2_CMD,PIC_READ_ISR),inb(PIC2_CMD))

/* Signal EOI (End of interrupt) to the first (master), or second (slave) PIC. */
#define PIC_EOI(intno) ((intno) >= INTNO_PIC2_BASE ? outb(PIC2_CMD,PIC_CMD_EOI) : (void)0, \
                         outb(PIC1_CMD,PIC_CMD_EOI))
#define PIC1_EOI()      (outb(PIC1_CMD,PIC_CMD_EOI))
#define PIC2_EOI()      (outb(PIC2_CMD,PIC_CMD_EOI),outb(PIC1_CMD,PIC_CMD_EOI))

struct spurious_pic {
 u32 sp_pic1; /*< Amount of spurious interrupts that occurred on PIC #1. */
 u32 sp_pic2; /*< Amount of spurious interrupts that occurred on PIC #2. */
};

/* Tracking information about spurious interrupts.
 * >> Useful for detecting faulty software that sends EOI commands at the
 *    wrong time, or for tracking problems in hardware, such as line noise. */
DATDEF struct spurious_pic pic_spurious;


/* Get/Set the mask of disabled interrupt lines.
 * >> `INTNO_PIC1_STMASK()' disables irq_t: 0x20...0x27
 * >> `INTNO_PIC2_STMASK()' disables irq_t: 0x28...0x2f
 */
#define PIC1_GTMASK()   inb_p(PIC1_DATA)
#define PIC1_STMASK(m) outb_p(PIC1_DATA,m)
#define PIC2_GTMASK()   inb_p(PIC2_DATA)
#define PIC2_STMASK(m) outb_p(PIC2_DATA,m)

/* Standard ISA IRQ numbers.
 * >> This is the default wireing of hardware interrupts. */
#define INTNO_PIC1_PIT   INTNO_PIC1(0) /*< Programmable Interrupt Timer Interrupt. */
#define INTNO_PIC1_KBD   INTNO_PIC1(1) /*< Keyboard Interrupt. */
#define INTNO_PIC1_CAS   INTNO_PIC1(2) /*< Cascade (used internally by the two PICs. never raised). */
#define INTNO_PIC1_COM2  INTNO_PIC1(3) /*< COM2 (if enabled). */
#define INTNO_PIC1_COM1  INTNO_PIC1(4) /*< COM1 (if enabled). */
#define INTNO_PIC1_LPT2  INTNO_PIC1(5) /*< LPT2 (if enabled). */
#define INTNO_PIC1_FLOP  INTNO_PIC1(6) /*< Floppy Disk. */
#define INTNO_PIC1_LPT1  INTNO_PIC1(7) /*< LPT1 / Unreliable "spurious" interrupt (usually). */
#define INTNO_PIC2_CMOS  INTNO_PIC2(0) /*< CMOS real-time clock (if enabled). */
#define INTNO_PIC2_FREE1 INTNO_PIC2(1) /*< Free for peripherals / legacy SCSI / NIC. */
#define INTNO_PIC2_FREE2 INTNO_PIC2(2) /*< Free for peripherals / SCSI / NIC. */
#define INTNO_PIC2_FREE3 INTNO_PIC2(3) /*< Free for peripherals / SCSI / NIC. */
#define INTNO_PIC2_PS2M  INTNO_PIC2(4) /*< PS2 Mouse. */
#define INTNO_PIC2_FPU   INTNO_PIC2(5) /*< FPU / Coprocessor / Inter-processor. */
#define INTNO_PIC2_ATA1  INTNO_PIC2(6) /*< Primary ATA Hard Disk. */
#define INTNO_PIC2_ATA2  INTNO_PIC2(7) /*< Secondary ATA Hard Disk. */


/* PIC interrupt handler.
 * @return: * :     One of `INTCODE_*' (e.g.: `INTCODE_HANDLED')
 *         WARNING: In the event that `INTCODE_HANDLED' is returned,
 *                  the handler is responsible for sending `EOI' to
 *                  the PIC, preferably using `PIC_EOI(intno)'
 * @param: closure: The closure argument used when registering the handler. */
typedef int (INTCALL *pic_handler_t)(void *closure);


/* Initialize an interrupt descriptor for a given PIC interrupt number.
 * @param: intno:   One of `INTNO_PIC(1|2)_*'
 * @param: handler: The handler to register/delete.
 * @param: closure: The closure that should be passed  */
#define PIC_INTERRUPT(intno,handler,closure) \
 { .i_intno = (intno), \
   .i_mode  = INTMODE_HW, \
   .i_type  = INTTYPE_FAST_ARG, \
   .i_prio  = INTPRIO_NORMAL, \
   .i_flags = INTFLAG_NORMAL, \
   .i_proto = { \
       .p_fast_arg = (handler) \
   }, \
   .i_closure = (closure), \
   .i_owner = THIS_INSTANCE }

/* Initialize an interrupt descriptor for a given PIC interrupt number.
 * @param: intno:   One of `INTNO_PIC(1|2)_*'
 * @param: handler: The handler to register/delete.
 * @param: closure: The closure that should be passed  */
LOCAL void KCALL
pic_interrupt(struct interrupt *__restrict self,
              irq_t intno, pic_handler_t handler,
              void *closure) {
 memset(self,0,sizeof(struct interrupt));
 self->i_intno = intno;
#if INTMODE_HW != 0
 self->i_mode  = INTMODE_HW;
#endif
#if INTTYPE_FAST_ARG != 0
 self->i_type  = INTTYPE_FAST_ARG;
#endif
#if INTPRIO_NORMAL != 0
 self->i_prio  = INTPRIO_NORMAL;
#endif
#if INTFLAG_NORMAL != 0
 self->i_flags = INTFLAG_NORMAL;
#endif
 self->i_proto.p_fast_arg = handler;
 self->i_closure = closure;
 self->i_owner = THIS_INSTANCE;
}



DECL_END

#endif /* !GUARD_INCLUDE_ARCH_PIC_H */
