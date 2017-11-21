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
#ifndef GUARD_KERNEL_ARCH_TASK_C
#define GUARD_KERNEL_ARCH_TASK_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <arch/cpustate.h>
#include <stdbool.h>

DECL_BEGIN

/* Assert CPU-state/register pack sizes. */
STATIC_ASSERT(sizeof(struct gpregs) == GPREGS_SIZE);
STATIC_ASSERT(sizeof(struct hiregs) == HIREGS_SIZE);
STATIC_ASSERT(sizeof(struct xcregs) == XCREGS_SIZE);
STATIC_ASSERT(sizeof(struct comregs) == COMREGS_SIZE);
STATIC_ASSERT(offsetof(struct cpustate,gp) == CPUSTATE_OFFSETOF_GP);
STATIC_ASSERT(offsetof(struct cpustate,hi) == CPUSTATE_OFFSETOF_HI);
STATIC_ASSERT(offsetof(struct cpustate,xc) == CPUSTATE_OFFSETOF_XC);
STATIC_ASSERT(sizeof(struct cpustate) == CPUSTATE_SIZE);

GLOBAL_ASM(
L(.section .text                                                              )
L(PUBLIC_ENTRY(cpu_sched_setrunning_save)                                     )
L(    /* TODO */                                                              )
L(PUBLIC_ENTRY(cpu_sched_setrunning)                                          )
L(    /* TODO */                                                              )
L(SYM_END(cpu_sched_setrunning)                                               )
L(SYM_END(cpu_sched_setrunning_save)                                          )
L(.previous                                                                   )
);


/* PUBLIC errno_t KCALL task_yield(void); */
GLOBAL_ASM(
L(.section .text                                                              )
L(PUBLIC_ENTRY(task_yield)                                                    )
L(    /* TODO */                                                              )
L(SYM_END(task_yield)                                                         )
L(.previous                                                                   )
);

#ifdef CONFIG_DEBUG
INTERN bool interrupts_enabled_initial = false;
#endif


DECL_END

#endif /* !GUARD_KERNEL_ARCH_TASK_C */
