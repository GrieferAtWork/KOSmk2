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
#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H 1

#include <features.h>
#include <signal.h>
#include <bits/types.h>
#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#include <bits/siginfo.h> /* We'd only need 'siginfo_t' */
#endif /* __USE_XOPEN || __USE_XOPEN2K8 */

/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

__DECL_BEGIN

#ifndef __WAIT_MACROS_DEFINED
#define __WAIT_MACROS_DEFINED 1
#include <bits/waitflags.h>
#include <bits/waitstatus.h>

#ifdef __USE_MISC
#if defined(__GNUC__) && !defined __cplusplus
#   define __WAIT_INT(status) (__extension__(((union{ __typeof__(status) __in; int __i; }) { .__in = (status) }).__i))
#else
#   define __WAIT_INT(status) (*(int *)&(status))
#endif
#if !defined __GNUC__ || __GNUC__ < 2 || defined __cplusplus
#   define __WAIT_STATUS      void *
#   define __WAIT_STATUS_DEFN void *
#else
typedef union {
 union wait *__uptr;
 int        *__iptr;
} __WAIT_STATUS __attribute__((__transparent_union__));
#   define __WAIT_STATUS_DEFN int *
#endif
#else /* __USE_MISC */
#   define __WAIT_INT(status)  (status)
#   define __WAIT_STATUS        int *
#   define __WAIT_STATUS_DEFN   int *
#endif /* !__USE_MISC */
#   define WEXITSTATUS(status)  __WEXITSTATUS(__WAIT_INT(status))
#   define WTERMSIG(status)     __WTERMSIG(__WAIT_INT(status))
#   define WSTOPSIG(status)     __WSTOPSIG(__WAIT_INT(status))
#   define WIFEXITED(status)    __WIFEXITED(__WAIT_INT(status))
#   define WIFSIGNALED(status)  __WIFSIGNALED(__WAIT_INT(status))
#   define WIFSTOPPED(status)   __WIFSTOPPED(__WAIT_INT(status))
#ifdef __WIFCONTINUED
#   define WIFCONTINUED(status) __WIFCONTINUED(__WAIT_INT(status))
#endif
#endif /* !__WAIT_MACROS_DEFINED */

#ifdef __USE_MISC
#   define WCOREFLAG           __WCOREFLAG
#   define WCOREDUMP(status)   __WCOREDUMP(__WAIT_INT(status))
#   define W_EXITCODE(ret,sig) __W_EXITCODE(ret,sig)
#   define W_STOPCODE(sig)     __W_STOPCODE(sig)
#endif

#ifdef __USE_MISC
#   define WAIT_ANY     (-1) /*< Any process. */
#   define WAIT_MYPGRP    0  /*< Any process in my process group. */
#endif


#ifndef __KERNEL__
__LIBC __pid_t (__LIBCCALL wait)(__WAIT_STATUS __stat_loc);
__LIBC __pid_t (__LIBCCALL waitpid)(__pid_t __pid, __WAIT_STATUS __stat_loc, int __options);
#endif /* !__KERNEL__ */

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#ifndef __id_t_defined
#define __id_t_defined 1
typedef __id_t id_t;
#endif /* !__id_t_defined */
#ifndef __KERNEL__
__LIBC int (__LIBCCALL waitid)(idtype_t __idtype, __id_t __id, siginfo_t *__infop, int __options);
#endif /* !__KERNEL__ */
#endif /* __USE_XOPEN || __USE_XOPEN2K8 */

#ifndef __KERNEL__
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
struct rusage;
__LIBC __pid_t (__LIBCCALL wait3)(__WAIT_STATUS __stat_loc, int __options, struct rusage *__usage);
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_MISC
__LIBC __pid_t (__LIBCCALL wait4)(__pid_t __pid, __WAIT_STATUS __stat_loc, int __options, struct rusage *__usage);
#endif /* __USE_MISC */
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_SYS_WAIT_H */
