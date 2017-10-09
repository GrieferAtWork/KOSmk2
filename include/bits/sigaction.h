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
#ifndef _BITS_SIGACTION_H
#define _BITS_SIGACTION_H 1

#include <__stdinc.h>
#include <bits/siginfo.h>

__SYSDECL_BEGIN

/* The proper definitions for Linux's sigaction.
   Copyright (C) 1993-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */


typedef void (*__sighandler_t)(int);

/* Structure describing the action to be taken when a signal arrives. */
struct sigaction {
 /* Signal handler. */
#ifdef __USE_POSIX199309
 union {
  __sighandler_t sa_handler; /*< Used if SA_SIGINFO is not set. */
  void(*sa_sigaction) (int,siginfo_t *,void *); /*< Used if SA_SIGINFO is set. */
 };
#else /* __USE_POSIX199309 */
 __sighandler_t sa_handler;
#endif /* !__USE_POSIX199309 */
 __sigset_t sa_mask;  /*< Additional set of signals to be blocked. */
 int        sa_flags; /*< Special flags. */
 void     (*sa_restorer)(void); /*< Restore handler. */
};

/* Bits in `sa_flags'. */
#define SA_NOCLDSTOP  1 /*< Don't send SIGCHLD when children stop. */
#define SA_NOCLDWAIT  2 /*< Don't create zombie on child death. */
#define SA_SIGINFO    4 /*< Invoke signal-catching function with three arguments instead of one. */
#if defined(__USE_UNIX98) || defined(__USE_MISC)
#define SA_ONSTACK    0x08000000 /*< Use signal stack by using `sa_restorer'. */
#endif /* __USE_UNIX98 || __USE_MISC */
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
#define SA_RESTART    0x10000000 /*< Restart syscall on signal return. */
#define SA_NODEFER    0x40000000 /*< Don't automatically block the signal when its handler is being executed. */
#define SA_RESETHAND  0x80000000 /*< Reset to SIG_DFL on entry to handler. */
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */
#ifdef __USE_MISC
#define SA_INTERRUPT  0x20000000 /*< Historical no-op. */
/* Some aliases for the SA_ constants. */
#define SA_NOMASK     SA_NODEFER
#define SA_ONESHOT    SA_RESETHAND
#define SA_STACK      SA_ONSTACK
#endif /* __USE_MISC */

/* Values for the HOW argument to `sigprocmask'. */
#define SIG_BLOCK     0 /*< Block signals. */
#define SIG_UNBLOCK   1 /*< Unblock signals. */
#define SIG_SETMASK   2 /*< Set the set of blocked signals. */

__SYSDECL_END

#endif /* !_BITS_SIGACTION_H */
