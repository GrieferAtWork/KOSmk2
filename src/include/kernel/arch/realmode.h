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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_REALMODE_H
#define GUARD_INCLUDE_KERNEL_ARCH_REALMODE_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/asm.h>
#include <hybrid/host.h>

DECL_BEGIN

#ifdef __CC__

DATDEF PHYS u16 realmode_base;   /*< [const] Runtime-generated realmode base address. */
DATDEF PHYS u16 realmode_stack;  /*< [const] Absolute address of a small realmode stack usable while in realmode, or for 'rm_interrupt'. */
#define REALMODE_STACK_SIZE 0x100

#define REALMODE_PREFERRED 0x7000 /*< Preferred realmode base address. */
#define REALMODE_STARTRELO 0x7000 /*< Default relocation address. */

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Begin/end symbols for realmode code sections. */
INTDEF byte_t __rm_core_start[];
INTDEF byte_t __rm_core_end[];

/* Realmode runtime relocations. */
INTDEF u32 __rm_rel_r_16_begin[];
INTDEF u32 __rm_rel_r_16_end[];
#endif

struct cpustate16;

/* Perform a bios-interrupt 'intno' from realmode.
 * The given 'state' contains the absolute CPU-state, when
 * the interrupt is performed, and following its completion
 * will be updated to contain all the new register states. */
FUNDEF SAFE void KCALL rm_interrupt(struct cpustate16 *__restrict state, irq_t intno);


#ifdef CONFIG_BUILDING_KERNEL_CORE
#define REALMODE_EARLY_STACK   REALMODE_STARTRELO

/* Same as 'rm_interrupt', but should be used
 * instead before 'realmode_initialize' was called. */
INTDEF INITCALL SAFE void KCALL early_rm_interrupt(struct cpustate16 *__restrict state, irq_t intno);
/* Allocate & relocate realmode code into low memory. */
INTDEF INITCALL void KCALL realmode_initialize(void);
#endif
#endif /* __CC__ */


/* Return the absolute, runtime address of a given realmode symbol 'x' */
#define REALMODE_SYM(x)   ((uintptr_t)realmode_base+((uintptr_t)&(x)-(uintptr_t)__rm_core_start))

#define RM_BEGIN_EX(alignment)  \
 .code16; \
 .hidden __rm_local_rel_end; .local __rm_local_rel_end; \
 .hidden __rm_local_begin; .local __rm_local_begin; \
 .section .realmode_rel.data; \
     .hidden __rm_local_rel; \
     .local __rm_local_rel; \
     __rm_local_rel:; \
 .previous; \
 .section .realmode_rel; \
     .long __rm_local_begin; \
     .long __rm_local_rel; \
     .long __rm_local_rel_end; \
 .previous; \
 .section .realmode; \
 .align alignment; \
 __rm_local_begin:

#define RM_BEGIN RM_BEGIN_EX(1)
#define RM_END \
 .previous; \
 .section .realmode_rel.data; __rm_local_rel_end:; .previous; \
 .code32;


#define RM_R_16(v) \
980: \
  .section .realmode_rel.data; \
  .word    980b - __rm_local_begin; \
  .previous; \
  .word    REALMODE_STARTRELO + (v) - __rm_local_begin


#if defined(__i386__)
#define __RM_REGID_al   0
#define __RM_REGID_ax   0
#define __RM_REGID_eax  0
#define __RM_REGID_cl   1
#define __RM_REGID_cx   1
#define __RM_REGID_ecx  1
#define __RM_REGID_dl   2
#define __RM_REGID_dx   2
#define __RM_REGID_edx  2
#define __RM_REGID_bl   3
#define __RM_REGID_bx   3
#define __RM_REGID_ebx  3
#define __RM_REGID_ah   4
#define __RM_REGID_sp   4
#define __RM_REGID_esp  4
#define __RM_REGID_ch   5
#define __RM_REGID_bp   5
#define __RM_REGID_ebp  5
#define __RM_REGID_dh   6
#define __RM_REGID_si   6
#define __RM_REGID_esi  6
#define __RM_REGID_bh   7
#define __RM_REGID_di   7
#define __RM_REGID_edi  7
#define __RM_REGID(reg) __RM_REGID_##reg
#define __RM_MODRM_MEM(dst_reg) 0x6|(dst_reg << 3)


/* Assembly helper macros for generating instructions using our custom relocations. */
#define RM_LGDT(sym)        /* lgdt  REL(sym)        */ .byte 0x0f; .byte 0x01; .byte 0x16; RM_R_16(sym)
#define RM_LLDT(sym)        /* lldt  REL(sym)        */ .byte 0x0f; .byte 0x00; .byte 0x16; RM_R_16(sym)
#define RM_LIDT(sym)        /* lidt  REL(sym)        */ .byte 0x0f; .byte 0x01; .byte 0x1e; RM_R_16(sym)
#define RM_LGDTL(sym)       /* lgdtl REL(sym)        */ .byte 0x66; RM_LGDT(sym)
#define RM_LLDTL(sym)       /* lldtl REL(sym)        */ .byte 0x66; RM_LLDT(sym)
#define RM_LIDTL(sym)       /* lidtl REL(sym)        */ .byte 0x66; RM_LIDT(sym)
#define RM_LJMPW(seg,sym)   /* ljmpw $seg, REL(sym)  */ .byte 0xea; RM_R_16(sym); .word seg
#define RM_LEAW(sym,reg)    /* movw  $REL(sym), %reg */ .byte 0xb8|__RM_REGID(reg); RM_R_16(sym)
#define RM_LEAL(sym,reg)    /* movl  $REL(sym), %reg */ .byte 0x66; RM_LEAW(sym,reg); .word 0x0000
#define RM_MOVW_R(sym,reg)  /* movw   REL(sym), %reg */ .byte 0x8b; .byte __RM_MODRM_MEM(__RM_REGID(reg)); RM_R_16(sym)
#define RM_MOVL_R(sym,reg)  /* movl   REL(sym), %reg */ .byte 0x66; RM_MOVW_R(sym,reg)
#define RM_MOVW_M(reg,sym)  /* movw   %reg, REL(sym) */ .byte 0x89; .byte __RM_MODRM_MEM(__RM_REGID(reg)); RM_R_16(sym)
#define RM_MOVL_M(reg,sym)  /* movl   %reg, REL(sym) */ .byte 0x66; RM_MOVW_M(reg,sym)
#endif


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_REALMODE_H */
