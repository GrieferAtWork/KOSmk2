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
#ifndef GUARD_ARCH_I386_KOS_INCLUDE_ARCH_THREAD_CONTEXT_H
#define GUARD_ARCH_I386_KOS_INCLUDE_ARCH_THREAD_CONTEXT_H 1

#include <hybrid/compiler.h>
#include <arch/gdt.h>
#include <arch/paging.h>

#ifndef CONFIG_NO_FPU
#include <arch/fpu.h>
#endif /* !CONFIG_NO_FPU */

DECL_BEGIN

#ifdef CONFIG_NO_LDT
#define __TASK_SWITCH_LDT(old,new) /* Nothing */
#else
#define __TASK_SWITCH_LDT(old,new) \
    if ((old)->t_arch.at_ldt_gdt != \
        (new)->t_arch.at_ldt_gdt) { \
     __asm__ __volatile__("lldt %w0\n" : : "g" ((new)->t_arch.at_ldt_gdt)); \
    }
#endif

#ifdef CONFIG_NO_FPU
#define __TASK_SWITCH_FPU(old,new) /* Nothing */
#else
#define __TASK_SWITCH_FPU(old,new) FPUSTATE_DISABLE();
#endif

#ifdef CONFIG_NO_TLB
#define __TASK_SWITCH_TLB(old,new) /* Nothing */
#else
/* Update the TLB pointers in the current cpu's GDT. */
#define __TASK_SWITCH_TLB(old,new) \
    { struct segment *seg = &CPU(cpu_gdt).ip_gdt[SEG_USER_TLB]; \
      SEGMENT_STBASE(seg[SEG_USER_TLB-SEG_USER_TLB],(new)->t_tlb); \
      SEGMENT_STBASE(seg[SEG_USER_TIB-SEG_USER_TLB],&(new)->t_tlb->tl_tib); \
    }
#endif

/* Switch secondary context registers such as LDT, page-directory or
 * the FPU-state as is required when switching from 'old' to `new'. */
#define TASK_SWITCH_CONTEXT(old,new) \
do{ struct mman *const new_mm = (new)->t_mman; \
    if ((old)->t_mman != new_mm) { \
     /* Switch page directories. */ \
     PDIR_STCURR(new_mm->m_ppdir); \
     /* Switch LDT descriptors. (NOTE: Must always \
      * be equal within the same page-directory) */ \
     __TASK_SWITCH_LDT(old,new) \
    } \
    /* Disable the FPU to cause lazy register save/ \
     * restore the next time operations are performed. */ \
    __TASK_SWITCH_FPU(old,new) \
    /* Load the new task's TLB and TIB. */ \
    __TASK_SWITCH_TLB(old,new) \
}while(0)

DECL_END

#endif /* !GUARD_ARCH_I386_KOS_INCLUDE_ARCH_THREAD_CONTEXT_H */
