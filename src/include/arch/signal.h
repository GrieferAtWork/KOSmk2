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
#include <asm/registers.h>

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


DECL_END

#endif /* !GUARD_INCLUDE_ARCH_SIGNAL_H */
