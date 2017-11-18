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
#ifndef GUARD_INCLUDE_ARCH_SIGNAL_H
#define GUARD_INCLUDE_ARCH_SIGNAL_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>
#include <sys/ucontext.h>
#include <bits/siginfo.h>
#include <arch/syscall.h>

DECL_BEGIN

#define SIGENTER_BASE_SIZE   __COMPILER_OFFSETAFTER(struct sigenter_info,se_base)
#define SIGENTER_FULL_SIZE   __COMPILER_OFFSETAFTER(struct sigenter_info,se_full)

#define SIGENTER_TAIL_OFFSETOF_CTX    0
#ifdef __x86_64__
#define SIGENTER_TAIL_OFFSETOF_SIGNO  __UCONTEXT_SIZE
#define SIGENTER_TAIL_OFFSETOF_OLDBP (__UCONTEXT_SIZE+__SIZEOF_POINTER__)
#define SIGENTER_TAIL_OFFSETOF_OLDIP (__UCONTEXT_SIZE+2*__SIZEOF_POINTER__)
#define SIGENTER_TAIL_SIZE           (__UCONTEXT_SIZE+3*__SIZEOF_POINTER__)
#else
#define SIGENTER_TAIL_OFFSETOF_OLDBP  __UCONTEXT_SIZE
#define SIGENTER_TAIL_OFFSETOF_OLDIP (__UCONTEXT_SIZE+__SIZEOF_POINTER__)
#define SIGENTER_TAIL_SIZE           (__UCONTEXT_SIZE+2*__SIZEOF_POINTER__)
#endif

#ifdef __CC__
struct PACKED sigenter_tail {
 ucontext_t       t_ctx;       /*< User-space CPU context before the signal was invoked. */
#ifdef __x86_64__
 register_t       t_signo;     /*< Signal number. */
#endif /* __x86_64__ */
 USER void       *t_oldbp;     /*< Holds the value of the old XBP (stackframe pointer);
                                *  When the signal handler is called, this is also where the new XBP points to! */
 USER void       *t_oldip;     /*< Second part of the stackframe: The return address.
                                *  NOTE: If `SA_RESTART' was set and the interrupt happened while inside of
                                *        a system-call that can be restarted (s.a.: `SYSCALL_FLAG_NORESTART'),
                                *        this points before the instruction that invoked the interrupt and
                                *        the value of `XAX' stored in `b_scratch' contains the original. */
};

struct PACKED sigenter_bhead {
 USER void       *sh_return;   /*< [1..1] Address of the `sigreturn' trampoline code. */
#ifndef __x86_64__
 uintptr_t        sh_signo;    /*< Signal number. */
#endif
};
struct PACKED sigenter_fhead {
 USER void       *sh_return;   /*< [1..1] Address of the `sigreturn' trampoline code. */
#ifndef __x86_64__
 uintptr_t        sh_signo;    /*< Signal number. */
 USER siginfo_t  *sh_pinfo;    /*< == USER(&f_info). */
 USER ucontext_t *sh_pctx;     /*< == USER(&f_tail.t_ctx). */
#endif
union{ siginfo_t  sh_info;     /*< Additional signal information. */
 int            __sh_info_pad[__SI_MAX_SIZE/sizeof(int)]; };
};

struct PACKED sigenter_info {
 /* Basic sigenter data (Pushed onto the stack when `SA_SIGINFO' isn't set). */
 USER void       *se_return;   /*< [1..1] Address of the `sigreturn' trampoline code. */
#ifndef __x86_64__
 uintptr_t        se_signo;    /*< Signal number. */
#endif
union PACKED { struct PACKED {
 struct sigenter_tail b_tail; /*< Signal information tail. */
}                 se_base;
struct PACKED {
#ifndef __x86_64__
 USER siginfo_t  *f_pinfo;    /*< == USER(&f_info). */
 USER ucontext_t *f_pctx;     /*< == USER(&f_tail.t_ctx). */
#endif
union{ siginfo_t  f_info;     /*< Additional signal information. */
 int            __f_info_pad[__SI_MAX_SIZE/sizeof(int)]; };
 struct sigenter_tail f_tail; /*< Signal information tail. */
}                 se_full; };
};
#endif /* __CC__ */


#define SIGENTER_OFFSETOF_COUNT     0
#define SIGENTER_OFFSETOF_NEXT      __SIZEOF_POINTER__
#define SIGENTER_OFFSETOF_XIP    (2*__SIZEOF_POINTER__)
#define SIGENTER_OFFSETOF_XAX    (3*__SIZEOF_POINTER__)
#define SIGENTER_OFFSETOF_XFLAGS (4*__SIZEOF_POINTER__)
#define SIGENTER_OFFSETOF_XSP    (5*__SIZEOF_POINTER__)
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
#define SIGENTER_OFFSETOF_XDX    (6*__SIZEOF_POINTER__)
#define SIGENTER_SIZE            (7*__SIZEOF_POINTER__)
#else /* CONFIG_HAVE_SYSCALL_LONGBIT */
#define SIGENTER_SIZE            (6*__SIZEOF_POINTER__)
#endif /* !CONFIG_HAVE_SYSCALL_LONGBIT */

#ifdef __CC__
struct PACKED sigenter {
 size_t                     se_count;  /* The amount of signals current raised. */
 USER struct sigenter_tail *se_next;   /* [0..1][valid_if(se_count != 0)] Next user-space signal handler context. */
 register_t                 se_xip;    /* offsetof(this) == offsetof(struct irregs,xip) */
 register_t                 se_xax;
 register_t                 se_xflags; /* offsetof(this) == offsetof(struct irregs,xflags) */
 register_t                 se_xsp;    /* offsetof(this) == offsetof(struct irregs,xsp) */
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
 register_t                 se_xdx;
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

#endif /* !GUARD_INCLUDE_ARCH_SIGNAL_H */
