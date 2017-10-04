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
#ifndef _BITS_SIGNUM_H
#define _BITS_SIGNUM_H 1

#include <__stdinc.h>
#include <features.h>

__DECL_BEGIN

/* Signal number definitions.  Linux version.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
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

/* Fake signal functions. */
#define SIG_ERR   ((__sighandler_t)-1) /*< Error return. */
#define SIG_DFL   ((__sighandler_t)0)  /*< Default action. */
#define SIG_IGN   ((__sighandler_t)1)  /*< Ignore signal. */

#define __DOS_SIG_GET  ((__sighandler_t)2)  /*< Return current value. */
#define __DOS_SIG_SGE  ((__sighandler_t)3)  /*< Signal gets error. */
#define __DOS_SIG_ACK  ((__sighandler_t)4)  /*< Acknowledge. */
#define __DOS_SIG_HOLD ((__sighandler_t)99) /*< Add signal to hold mask. (Needs a different value due to collision with '__DOS_SIG_GET') */

#ifdef __USE_DOS
#define SIG_GET   __DOS_SIG_GET
#define SIG_SGE   __DOS_SIG_SGE
#define SIG_ACK   __DOS_SIG_ACK
#ifdef __USE_UNIX98
#define SIG_HOLD  __DOS_SIG_HOLD
#endif /* __USE_UNIX98 */
#else /* __USE_DOS */
#ifdef __USE_UNIX98
#define SIG_HOLD  ((__sighandler_t)2)  /*< Add signal to hold mask. */
#endif /* __USE_UNIX98 */
#endif /* !__USE_DOS */

/* Signals. */
#define SIGHUP    1     /*< Hangup (POSIX). */
#define SIGINT    2     /*< Interrupt (ANSI). */
#define SIGQUIT   3     /*< Quit (POSIX). */
#define SIGILL    4     /*< Illegal instruction (ANSI). */
#define SIGTRAP   5     /*< Trace trap (POSIX). */
#define SIGABRT   6     /*< Abort (ANSI). */
#define SIGIOT    6     /*< IOT trap (4.2 BSD). */
#define SIGBUS    7     /*< BUS error (4.2 BSD). */
#define SIGFPE    8     /*< Floating-point exception (ANSI). */
#define SIGKILL   9     /*< Kill, unblockable (POSIX). */
#define SIGUSR1   10    /*< User-defined signal 1 (POSIX). */
#define SIGSEGV   11    /*< Segmentation violation (ANSI). */
#define SIGUSR2   12    /*< User-defined signal 2 (POSIX). */
#define SIGPIPE   13    /*< Broken pipe (POSIX). */
#define SIGALRM   14    /*< Alarm clock (POSIX). */
#define SIGTERM   15    /*< Termination (ANSI). */
#define SIGSTKFLT 16    /*< Stack fault. */
#define SIGCLD    SIGCHLD /*< Same as SIGCHLD (System V). */
#define SIGCHLD   17    /*< Child status has changed (POSIX). */
#define SIGCONT   18    /*< Continue (POSIX). */
#define SIGSTOP   19    /*< Stop, unblockable (POSIX). */
#define SIGTSTP   20    /*< Keyboard stop (POSIX). */
#define SIGTTIN   21    /*< Background read from tty (POSIX). */
#define SIGTTOU   22    /*< Background write to tty (POSIX). */
#define SIGURG    23    /*< Urgent condition on socket (4.2 BSD). */
#define SIGXCPU   24    /*< CPU limit exceeded (4.2 BSD). */
#define SIGXFSZ   25    /*< File size limit exceeded (4.2 BSD). */
#define SIGVTALRM 26    /*< Virtual alarm clock (4.2 BSD). */
#define SIGPROF   27    /*< Profiling alarm clock (4.2 BSD). */
#define SIGWINCH  28    /*< Window size change (4.3 BSD, Sun). */
#define SIGPOLL   SIGIO /*< Pollable event occurred (System V). */
#define SIGIO     29    /*< I/O now possible (4.2 BSD). */
#define SIGPWR    30    /*< Power failure restart (System V). */
#define SIGSYS    31    /*< Bad system call. */
#define SIGUNUSED 31
#define _NSIG     65    /*< Biggest signal number + 1 (including real-time signals). */

#define SIGRTMIN   (__libc_current_sigrtmin())
#define SIGRTMAX   (__libc_current_sigrtmax())

/* These are the hard limits of the kernel.
 * These values should not be used directly at user level. */
#define __SIGRTMIN  32
#define __SIGRTMAX (_NSIG - 1)

#ifdef __USE_DOS
/* Define DOS's signal name aliases. */
#define SIGBREAK        21 /*< Background read from tty (POSIX). */
#define SIGABRT_COMPAT  6  /*< Abort (ANSI). */
/* Wow! Except for this oddity, DOS's signal codes are actually quite compatible. */
#ifndef __KERNEL__
#undef SIGABRT
#define SIGABRT         22 /*< Background write to tty (POSIX). */
#endif /* !__KERNEL__ */
#endif


__DECL_END

#endif /* !_BITS_SIGNUM_H */
