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
#ifndef GUARD_ARCH_ARM_KOS_KERNEL_SIGNAL_C
#define GUARD_ARCH_ARM_KOS_KERNEL_SIGNAL_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#ifndef CONFIG_NO_SIGNALS
#include <stddef.h>
#include <arch/signal.h>
#include <asm/sigcontext.h>
#include <bits/siginfo.h>
#include <hybrid/panic.h>
#include <sched/types.h>
#include <sys/ucontext.h>
#include <kernel/syscall.h>

DECL_BEGIN

STATIC_ASSERT(offsetof(struct sigcontext,trap_no) == __SIGCONTEXT_OFFSETOF_TRAP_NO);
STATIC_ASSERT(offsetof(struct sigcontext,error_code) == __SIGCONTEXT_OFFSETOF_ERROR_CODE);
STATIC_ASSERT(offsetof(struct sigcontext,oldmask) == __SIGCONTEXT_OFFSETOF_OLDMASK);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r0) == __SIGCONTEXT_OFFSETOF_ARM_R0);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r1) == __SIGCONTEXT_OFFSETOF_ARM_R1);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r2) == __SIGCONTEXT_OFFSETOF_ARM_R2);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r3) == __SIGCONTEXT_OFFSETOF_ARM_R3);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r4) == __SIGCONTEXT_OFFSETOF_ARM_R4);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r5) == __SIGCONTEXT_OFFSETOF_ARM_R5);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r6) == __SIGCONTEXT_OFFSETOF_ARM_R6);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r7) == __SIGCONTEXT_OFFSETOF_ARM_R7);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r8) == __SIGCONTEXT_OFFSETOF_ARM_R8);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r9) == __SIGCONTEXT_OFFSETOF_ARM_R9);
STATIC_ASSERT(offsetof(struct sigcontext,arm_r10) == __SIGCONTEXT_OFFSETOF_ARM_R10);
STATIC_ASSERT(offsetof(struct sigcontext,arm_fp) == __SIGCONTEXT_OFFSETOF_ARM_FP);
STATIC_ASSERT(offsetof(struct sigcontext,arm_ip) == __SIGCONTEXT_OFFSETOF_ARM_IP);
STATIC_ASSERT(offsetof(struct sigcontext,arm_sp) == __SIGCONTEXT_OFFSETOF_ARM_SP);
STATIC_ASSERT(offsetof(struct sigcontext,arm_lr) == __SIGCONTEXT_OFFSETOF_ARM_LR);
STATIC_ASSERT(offsetof(struct sigcontext,arm_pc) == __SIGCONTEXT_OFFSETOF_ARM_PC);
STATIC_ASSERT(offsetof(struct sigcontext,arm_cpsr) == __SIGCONTEXT_OFFSETOF_ARM_CPSR);
STATIC_ASSERT(offsetof(struct sigcontext,fault_address) == __SIGCONTEXT_OFFSETOF_ARM_FAULT_ADDRESS);
STATIC_ASSERT(sizeof(struct sigcontext) == __SIGCONTEXT_SIZE);
STATIC_ASSERT(offsetof(ucontext_t,uc_flags) == __UCONTEXT_OFFSETOF_FLAGS);
STATIC_ASSERT(offsetof(ucontext_t,uc_link) == __UCONTEXT_OFFSETOF_LINK);
STATIC_ASSERT(offsetof(ucontext_t,uc_stack) == __UCONTEXT_OFFSETOF_STACK);
STATIC_ASSERT(offsetof(ucontext_t,uc_mcontext) == __UCONTEXT_OFFSETOF_MCONTEXT);
STATIC_ASSERT(offsetof(ucontext_t,uc_sigmask) == __UCONTEXT_OFFSETOF_SIGMASK);
STATIC_ASSERT(offsetof(ucontext_t,uc_regspace) == __UCONTEXT_OFFSETOF_REGSPACE);
STATIC_ASSERT(sizeof(ucontext_t) == __UCONTEXT_SIZE);

STATIC_ASSERT(offsetof(struct sigenter,se_count) == SIGENTER_OFFSETOF_COUNT);
STATIC_ASSERT(offsetof(struct sigenter,se_next) == SIGENTER_OFFSETOF_NEXT);
STATIC_ASSERT(offsetof(struct sigenter,se_ip) == SIGENTER_OFFSETOF_IP);
STATIC_ASSERT(offsetof(struct sigenter,se_sp) == SIGENTER_OFFSETOF_SP);
STATIC_ASSERT(offsetof(struct sigenter,se_r0) == SIGENTER_OFFSETOF_R0);
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
STATIC_ASSERT(offsetof(struct sigenter,se_r1) == SIGENTER_OFFSETOF_R1);
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */
STATIC_ASSERT(sizeof(struct sigenter) == SIGENTER_SIZE);

SYSCALL_DEFINE1(sigreturn,USER struct sigcontext *,context) {
 /* TODO */
 return -ENOSYS;
}

INTERN void KCALL
coredump_task(struct task *__restrict t,
              siginfo_t const *__restrict reason,
              greg_t reg_trapno, greg_t reg_err) {
 PANIC("Not implemented");
}

INTERN errno_t KCALL
deliver_signal(struct task *__restrict t,
               struct sigaction const *__restrict action,
               siginfo_t const *__restrict signal_info,
               greg_t reg_trapno, greg_t reg_err) {
 PANIC("Not implemented");
}

DECL_END

#endif /* !CONFIG_NO_SIGNALS */


#endif /* !GUARD_ARCH_ARM_KOS_KERNEL_SIGNAL_C */
