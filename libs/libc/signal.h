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
#ifndef GUARD_LIBS_LIBC_SIGNAL_H
#define GUARD_LIBS_LIBC_SIGNAL_H 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include "libc.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <signal.h>

DECL_BEGIN

#ifndef __sighandler_t_defined
#define __sighandler_t_defined 1
typedef __sighandler_t sighandler_t;
#endif /* !__sighandler_t_defined */

INTDEF sighandler_t LIBCCALL libc_sysv_signal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libc_bsd_signal(int sig, sighandler_t handler);
INTDEF int LIBCCALL libc_siginterrupt(int sig, int interrupt);
INTDEF sighandler_t LIBCCALL libc_sigset(int sig, sighandler_t disp);
INTDEF int LIBCCALL libc_sigismember(sigset_t const *set, int sig);
INTDEF int LIBCCALL libc_sigaddset(sigset_t *set, int sig);
INTDEF int LIBCCALL libc_sigdelset(sigset_t *set, int sig);
INTDEF int LIBCCALL libc_sigemptyset(sigset_t *set);
INTDEF int LIBCCALL libc_sigfillset(sigset_t *set);
INTDEF int LIBCCALL libc_sigisemptyset(sigset_t const *set);
INTDEF int LIBCCALL libc_sigandset(sigset_t *set, sigset_t const *left, sigset_t const *right);
INTDEF int LIBCCALL libc_sigorset(sigset_t *set, sigset_t const *left, sigset_t const *right);
INTDEF int LIBCCALL libc_sigaction(int sig, struct sigaction const *__restrict act, struct sigaction *__restrict oact);
INTDEF int LIBCCALL libc_sigprocmask(int how, sigset_t const *__restrict set, sigset_t *__restrict oset);
INTDEF int LIBCCALL libc_pthread_sigmask(int how, sigset_t const *__restrict newmask, sigset_t *__restrict oldmask);
INTDEF int LIBCCALL libc_sigwaitinfo(sigset_t const *__restrict set, siginfo_t *__restrict info);
INTDEF int LIBCCALL libc_sigpending(sigset_t *set);
INTDEF int LIBCCALL libc_sigwait(sigset_t const *__restrict set, int *__restrict sig);
INTDEF int LIBCCALL libc_kill(pid_t pid, int sig);
INTDEF int LIBCCALL libc_killpg(pid_t pgrp, int sig);
INTDEF int LIBCCALL libc_raise(int sig);
INTDEF ATTR_NORETURN void LIBCCALL libc_sigreturn(struct sigcontext const *scp);
INTDEF int LIBCCALL libc_sigblock(int mask);
INTDEF int LIBCCALL libc_sigsetmask(int mask);
INTDEF int LIBCCALL libc_siggetmask(void);
INTDEF int LIBCCALL libc_sighold(int sig);
INTDEF int LIBCCALL libc_sigrelse(int sig);
INTDEF int LIBCCALL libc_sigignore(int sig);
INTDEF int LIBCCALL libc___libc_current_sigrtmin(void);
INTDEF int LIBCCALL libc___libc_current_sigrtmax(void);
INTDEF int LIBCCALL libc_sigaltstack(struct sigaltstack const *__restrict ss, struct sigaltstack *__restrict oss);
INTDEF int LIBCCALL libc_sigsuspend(sigset_t const *set);
INTDEF int LIBCCALL libc_pthread_kill(pthread_t threadid, int signo);
INTDEF int LIBCCALL libc_pthread_sigqueue(pthread_t threadid, int signo, union sigval const value);
INTDEF int LIBCCALL libc___xpg_sigpause(int sig);
INTDEF int LIBCCALL libc_sigqueue(pid_t pid, int sig, union sigval const val);
INTDEF void LIBCCALL libc_psignal(int sig, char const *s);
INTDEF void LIBCCALL libc_psiginfo(siginfo_t const *pinfo, char const *s);
INTDEF int LIBCCALL libc_sigstack(struct sigstack *ss, struct sigstack *oss);

DECL_END

#endif /* !GUARD_LIBS_LIBC_SIGNAL_H */
