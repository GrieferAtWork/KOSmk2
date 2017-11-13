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
#ifndef GUARD_KERNEL_ARCH_PIC_C
#define GUARD_KERNEL_ARCH_PIC_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <hybrid/compiler.h>
#include <arch/pic.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <dev/rtc.h>

DECL_BEGIN

/* PIC default initialization.
 * Clear the interrupt masks, thereby enabling all interrupt lines.
 * e.g.: Setting 'outb_p(PIC1_DATA,0x04)' would disable `INTNO_PIC1(3)'
 * NOTE: By default, we disable the 'Programmable Interrupt Timer',
 *       which is later re-enabled as a fallback technology for
 *       implementing preemption.
 *       If we wouldn't do this, 'Unmapped PIC interrupt' warnings,
 *       as emit by `irq_default' above may start flooding the terminal
 *       until we've finally gotten around to setting up scheduling.
 *      (Which may actually take some time, as we initialize core
 *       modules, such as our ATA driver first, which in turn may
 *       take a moment to spin up the disk) */
PRIVATE ATTR_COLDDATA u8 pic_bios_mask1 = 1 << (INTNO_PIC1_PIT-INTNO_PIC1_BASE);
PRIVATE ATTR_COLDDATA u8 pic_bios_mask2 = 0;

#define GET_RELOAD_VALUE(hz) ((3579545/(hz))/3)
#define pit_sethz(hz)       pit_sethz_(GET_RELOAD_VALUE(hz))
#define pit_sethz_default() pit_sethz_(65536) /* 18.2065hz */
LOCAL void KCALL pit_sethz_(int d) {
 outb(0x43,0x36); /* Mode #3: Square wave generator. */
 outb(0x40,d & 0xFF);
 outb(0x40,d >> 8);
}

INTERN ATTR_COLDTEXT void KCALL pic_bios_begin(void) {
 /* Save the current PIC mask. */
 pic_bios_mask1 = inb_p(PIC1_DATA);
 pic_bios_mask2 = inb_p(PIC2_DATA);

 /* XXX: Can we always rely on this configuration working for BIOS? */

 /* Load the real-mode (aka. BIOS) variant of the PIC. */
 outb_p(PIC1_CMD,ICW1_INIT|ICW1_ICW4);
 outb_p(PIC2_CMD,ICW1_INIT|ICW1_ICW4);
 outb_p(PIC1_DATA,0x08);
 outb_p(PIC2_DATA,0x70);
 outb_p(PIC1_DATA,4);
 outb_p(PIC2_DATA,2);
 outb_p(PIC1_DATA,ICW4_8086);
 outb_p(PIC2_DATA,ICW4_8086);

 /* Mask all interrupts while inside the bios, except for some that may actually be used. */
 outb_p(PIC1_DATA,0xff & ~((INTNO_PIC1_PIT-INTNO_PIC1_BASE)|    /* PIT Timer (may be used for timeouts...) */
                           (INTNO_PIC1_KBD-INTNO_PIC1_BASE)|    /* Keyboard (user input?) */
                           (INTNO_PIC1_CAS-INTNO_PIC1_BASE)|    /* Cascade (Needed to talk to PIC #2) */
                           (INTNO_PIC1_LPT1-INTNO_PIC1_BASE)|   /* Spurious interrupt vector (Better keep this enabled) */
                           (INTNO_PIC1_FLOP-INTNO_PIC1_BASE))); /* Floppy (Drive I/O) */
 outb_p(PIC2_DATA,0xff & ~((INTNO_PIC2_ATA1-INTNO_PIC2_BASE)|   /* ATA (Drive I/O) */
                           (INTNO_PIC2_ATA2-INTNO_PIC2_BASE)|   /* ATA (Drive I/O) */
                           (INTNO_PIC2_PS2M-INTNO_PIC2_BASE))); /* PS/2 mouse (user input?) */

 /* Restore the original PIC crystal speed set by the BIOS.
  * XXX: Is this required? */
 pit_sethz_default();
}

INTERN ATTR_COLDTEXT void KCALL pic_bios_end(void) {
 /* >> (re-)initialize the master & slave PICs.
  * Following this, each PIC will expect 3 additional "initialization words". */
 outb_p(PIC1_CMD,ICW1_INIT|ICW1_ICW4);
 outb_p(PIC2_CMD,ICW1_INIT|ICW1_ICW4);

 /* Word #1: Define the IRQ offsets.
  *          We map the master to 0x20..0x27,
  *          and the slave to 0x28..0x2f. */
 outb_p(PIC1_DATA,INTNO_PIC1_BASE);
 outb_p(PIC2_DATA,INTNO_PIC2_BASE);

 /* Word #2: Tell the master and slave how they are wired to each other. */
 outb_p(PIC1_DATA,4);
 outb_p(PIC2_DATA,2);

 /* Word #3: Define the environment mode. */
 outb_p(PIC1_DATA,ICW4_8086);
 outb_p(PIC2_DATA,ICW4_8086);

 outb_p(PIC1_DATA,pic_bios_mask1);
 outb_p(PIC2_DATA,pic_bios_mask2);

 /* Restore our PIT speed. */
 pit_sethz(HZ);
}


DECL_END

#endif /* !GUARD_KERNEL_ARCH_PIC_C */
