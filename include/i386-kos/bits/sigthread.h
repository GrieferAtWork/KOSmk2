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
#ifndef _I386_KOS_BITS_SIGTHREAD_H
#define _I386_KOS_BITS_SIGTHREAD_H 1
#define _BITS_SIGTHREAD_H 1

#include <__stdinc.h>

/* Signal handling function for threaded programs.
   Copyright (C) 1998-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

__SYSDECL_BEGIN

#ifndef __KERNEL__
__LIBC int (__LIBCCALL pthread_sigmask)(int __how, const __sigset_t *__restrict __newmask, __sigset_t *__restrict __oldmask);
__LIBC int (__LIBCCALL pthread_kill)(pthread_t __threadid, int __signo);
#ifdef __USE_GNU
__LIBC int (__LIBCCALL pthread_sigqueue)(pthread_t __threadid, int __signo, union sigval const __value);
#endif /* __USE_GNU */
#endif /* __KERNEL__ */

__SYSDECL_END

#endif /* !_I386_KOS_BITS_SIGTHREAD_H */
