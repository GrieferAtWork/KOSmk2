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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_INTERRUPT_H
#define GUARD_INCLUDE_KERNEL_ARCH_INTERRUPT_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN


/* Amount of `struct idtentry' that make up a full IDT table. */
#define IDT_TABLESIZE 256

#define IDTENTRY_OFFSETOF_OFF1  0
#define IDTENTRY_OFFSETOF_SEL   2
#define IDTENTRY_OFFSETOF_ZERO  4
#define IDTENTRY_OFFSETOF_FLAGS 5
#define IDTENTRY_OFFSETOF_OFF2  6
#ifdef __x86_64__
#define IDTENTRY_OFFSETOF_OFF3  8
#define IDTENTRY_SIZE           16
#else
#define IDTENTRY_SIZE           8
#endif

struct PACKED idtentry {
    u16 ie_off1;  /*< Lower 16 bits of an `irq_handler' pointer. */
    u16 ie_sel;   /*< Kernel code segment (always `__KERNEL_CS') */
#ifdef __x86_64__
    u8  ie_ist;   /*< Nits 0..2 hold Interrupt Stack Table offset, rest of bits zero. */
#else
    u8  ie_zero;  /*< Always ZERO(0). */
#endif
    u8  ie_flags; /*< Set of `IDTFLAG_*|IDTTYPE_*' */
    u16 ie_off2;  /*< Upper 16 bits of an `irq_handler' pointer. */
#ifdef __x86_64__
    u32 ie_off3;  /*< Bits 32..63 of the vector offset. */
    u32 ie_unused;/* Unused ata. */
#endif
};

#define IDTFLAG_PRESENT                 0x80 /*< Set to 0 for unused interrupts. */
/* Descriptor Privilege LevelGate call protection.
 * Specifies which privilege Level the calling Descriptor minimum should have.
 * So hardware and CPU interrupts can be protected from being called out of userspace. */
#define IDTFLAG_DPL(n)          (((n)&3)<<5) /*< Mask: 0x60 */
#define IDTFLAG_STORAGE_SEGMENT         0x10 /*< Set to 0 for interrupt gates. */
#define IDTTYPE_80386_32_TASK_GATE      0x05
#define IDTTYPE_80286_16_INTERRUPT_GATE 0x06
#define IDTTYPE_80286_16_TRAP_GATE      0x07
#define IDTTYPE_80386_32_INTERRUPT_GATE 0x0E
#define IDTTYPE_80386_32_TRAP_GATE      0x0F


/* Default interrupt descriptor flags for host-private
 * callbacks and callbacks available from user-space. */
#define INTMODE_HOST  (IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE|IDTFLAG_DPL(0)) /* Regular interrupt handler only callable by hardware or from within the kernel. */
#define INTMODE_USER  (IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE|IDTFLAG_DPL(3)) /* Interrupt handler accessible by anyone (including userspace). */
typedef u8 intmode_t;


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_INTERRUPT_H */
