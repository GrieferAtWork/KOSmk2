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
#ifndef GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_SIGNAL_H
#define GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_SIGNAL_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>
#include <sys/ucontext.h>
#include <bits/siginfo.h>
#include <arch/syscall.h>

DECL_BEGIN

#define SIGENTER_BASE_SIZE   __COMPILER_OFFSETAFTER(struct sigenter_info,se_base)
#define SIGENTER_FULL_SIZE   __COMPILER_OFFSETAFTER(struct sigenter_info,se_full)

#define SIGENTER_TAIL_OFFSETOF_CTX 0
#define SIGENTER_TAIL_SIZE         __UCONTEXT_SIZE

#ifdef __CC__
struct PACKED sigenter_tail {
 ucontext_t t_ctx; /*< User-space CPU context before the signal was invoked. */
};

struct PACKED sigenter_bhead { struct sigenter_tail b_tail; };
struct PACKED sigenter_fhead { struct sigenter_tail f_tail; };
struct PACKED sigenter_info {
union PACKED {
 struct PACKED { struct sigenter_bhead b_tail; } se_base;
 struct PACKED { struct sigenter_fhead f_tail; } se_full;
};
};
#endif /* __CC__ */

#define SIGENTER_OFFSETOF_COUNT     0
#define SIGENTER_OFFSETOF_NEXT      __SIZEOF_POINTER__
#define SIGENTER_OFFSETOF_IP     (2*__SIZEOF_POINTER__)
#define SIGENTER_OFFSETOF_SP     (3*__SIZEOF_POINTER__)
#define SIGENTER_OFFSETOF_R0     (4*__SIZEOF_POINTER__)
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
#define SIGENTER_OFFSETOF_R1     (5*__SIZEOF_POINTER__)
#define SIGENTER_SIZE            (6*__SIZEOF_POINTER__)
#else
#define SIGENTER_SIZE            (5*__SIZEOF_POINTER__)
#endif

#ifdef __CC__
struct PACKED sigenter {
 size_t                     se_count; /* The amount of signals current raised. */
 USER struct sigenter_tail *se_next;  /* [0..1][valid_if(se_count != 0)] Next user-space signal handler context. */
 uintptr_t                  se_ip;
 uintptr_t                  se_sp;
 uintptr_t                  se_r0;
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
 uintptr_t                  se_r1;
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */
};

#ifdef CONFIG_BUILDING_KERNEL_CORE
struct task;
struct sigaction;
struct __siginfo_struct;

#ifndef __greg_t_defined
#define __greg_t_defined 1
typedef __SREGISTER_TYPE__ greg_t;
#endif /* !__greg_t_defined */

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

/* Deliver a signal to a given task `t', invoking `action'.
 * WARNING: This function does not apply the action's blocking mask. */
INTDEF errno_t KCALL
deliver_signal(struct task *__restrict t,
               struct sigaction const *__restrict action,
               struct __siginfo_struct const *__restrict signal_info,
               greg_t reg_trapno, greg_t reg_err);

INTDEF void KCALL
coredump_task(struct task *__restrict t,
              siginfo_t const *__restrict reason,
              greg_t reg_trapno, greg_t reg_err);

#endif /* CONFIG_BUILDING_KERNEL_CORE */
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_SIGNAL_H */
