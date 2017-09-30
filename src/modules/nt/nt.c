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
#ifndef GUARD_MODULES_NT_NT_C
#define GUARD_MODULES_NT_NT_C 1
#define _KOS_SOURCE 2

#include <hybrid/align.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/debuginfo.h> /* For 'THIS_INSTANCE' */
#include <hybrid/host.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <sched/percpu.h>
#include <sched/rpc.h>
#include <sched/smp.h>
#include <sched/types.h>

#include "flavor.h"

DECL_BEGIN

PUBLIC struct nt_flavor *nt_flavor;

#ifdef __i386__
GLOBAL_ASM(
L(.section .data                                                              )
L(.align __SIZEOF_POINTER__                                                   )
L(INTERN_ENTRY(nt_syscall_entry)                                              )
L(   /* System call number: %eax */                                           )
L(   /* Argument vector:    %edx */                                           )
L(                                                                            )
L(   pushl  %ecx                                                              )
L(   pushl  %ebp                                                              )
L(   pushl  %esi                                                              )
L(   pushl  %edi                                                              )
L(                                                                            )
L(   /* Check if the system call number is valid. */                          )
L(   cmpl   $(NT_SYSCALL_COUNT), %eax                                         )
L(   jae    8f                                                                )
L(                                                                            )
L(   /* Load the address of the current NT flavor. */                         )
L(   .byte 0xbd /* movl $nt_current_flavor, %ebp */                           )
L(PUBLIC_ENTRY(nt_current_flavor)                                             )
L(   /* Pre-initialize to point to the default flavor. */                     )
L(   .long nt_flavors+(NT_FLAVOR_DEFAULT*NT_FLAVOR_SIZE)                      )
L(SYM_END(nt_current_flavor)                                                  )
L(                                                                            )
L(   /* Allocate stack memory for arguments. */                               )
L(   movl   %edx,                                 %esi                        )
L(   movzbl NT_FLAVOR_OFFSETOF_ARGC(%ebp,%eax,1), %ecx                        )
L(   leal   (%esp,%ecx,__SIZEOF_SYSCALL_LONG__),  %esp                        )
L(   movl   %esp,                                 %edi                        )
L(                                                                            )
L(   /* Safely copy argument vector to kernel space. */                       )
L(   pushl  %ebp                                                              )
L(   movl   ASM_CPU(CPU_OFFSETOF_RUNNING), %ebp                               )
L(   pushl  $9f                                                               )
L(   pushl  $(EXC_PAGE_FAULT)                                                 )
L(   pushl  TASK_OFFSETOF_IC(%ebp)                                            )
L(   rep    movsl                                                             )
L(   popl   TASK_OFFSETOF_IC(%ebp)                                            )
L(   addl   $8, %esp                                                          )
L(   popl   %ebp                                                              )
L(                                                                            )
L(   /* Actually call the NT system call. */                                  )
L(   call   *NT_FLAVOR_OFFSETOF_CALL(%ebp,%eax,__SIZEOF_POINTER__)            )
L(                                                                            )
L(   /* Restore the stack before the argument copy. */                        )
L(   movl   %edi, %esp                                                        )
L(1: popl   %edi                                                              )
L(   popl   %esi                                                              )
L(   popl   %ebp                                                              )
L(   popl   %ecx                                                              )
L(   iret                                                                     )
L(8: pushl  %eax                                                              )
L(   call   NtBadSysCall                                                      )
L(   jmp    2f                                                                )
L(9: pushl  %eax                                                              )
L(   call   NtSegFault                                                        )
L(2: popl   %eax                                                              )
L(   jmp    1b                                                                )
L(SYM_END(nt_syscall_entry)                                                   )
L(.previous                                                                   )
);
#else
#error "Not implemented"
#endif



INTDEF void (ASMCALL nt_syscall_entry)(void);
PRIVATE isr_t nt_syscall_isr = ISR_DEFAULT(42,&nt_syscall_entry); /* TODO: What number? */

PRIVATE void MODULE_INIT KCALL pe_init(void) {
 struct cpu *c; /* Install the NT IRQ handler on all CPUs. */
 assertf(IS_ALIGNED((uintptr_t)&nt_current_flavor,__SIZEOF_POINTER__),
         "Must adjust offset of 'nt_current_flavor' using %Iu nops",
        (uintptr_t)&nt_current_flavor & (__SIZEOF_POINTER__-1));
 FOREACH_CPU(c) irq_vset(c,&nt_syscall_isr,NULL,IRQ_SET_RELOAD);
 /* TODO: Kernel commandline switch to change NT flavor. */
}

DECL_END

#endif /* !GUARD_MODULES_NT_NT_C */
