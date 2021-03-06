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
#ifndef _BITS_GENERIC_WAITFLAGS_H
#define _BITS_GENERIC_WAITFLAGS_H 1
#define _BITS_WAITFLAGS_H 1

#include <__stdinc.h>
#include <features.h>

__SYSDECL_BEGIN

/* NOTE: This file based on the GLIBC header of the same name: */
/* Definitions of flag bits for `waitpid' et al.
   Copyright (C) 1992-2016 Free Software Foundation, Inc.
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

/* Bits in the third argument to `waitpid'. */
#ifndef WNOHANG
#define WNOHANG     1 /*< Don't block waiting. */
#endif /* !WNOHANG */
#ifndef WUNTRACED
#define WUNTRACED   2 /*< Report status of stopped children. */
#endif /* !WUNTRACED */

/* Bits in the fourth argument to `waitid'. */
#ifndef WSTOPPED
#define WSTOPPED    2 /*< Report stopped child (same as WUNTRACED). */
#endif /* !WSTOPPED */
#ifndef WEXITED
#define WEXITED     4 /*< Report dead child. */
#endif /* !WEXITED */
#ifndef WCONTINUED
#define WCONTINUED  8 /*< Report continued child. */
#endif /* !WCONTINUED */
#ifndef WNOWAIT
#define WNOWAIT     0x01000000 /* Don't reap, just poll status. */
#endif /* !WNOWAIT */

#ifndef __WNOTHREAD
#define __WNOTHREAD 0x20000000 /*< Don't wait on children of other threads in this group */
#endif /* !__WNOTHREAD */
#ifndef __WALL
#define __WALL      0x40000000 /*< Wait for any child. */
#endif /* !__WALL */
#ifndef __WCLONE
#define __WCLONE    0x80000000 /*< Wait for cloned process. */
#endif /* !__WCLONE */

/* The following values are used by the `waitid' function. */
#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#ifndef __ENUM_IDTYPE_T
#define __ENUM_IDTYPE_T 1
#   undef P_ALL
#   undef P_PID
#   undef P_PGID
#ifdef __COMPILER_PREFERR_ENUMS
typedef enum {
    P_ALL  = 0, /*< Wait for any child. */
    P_PID  = 1, /*< Wait for specified process. */
    P_PGID = 2  /*< Wait for members of process group. */
} idtype_t;
#   define P_ALL   P_ALL
#   define P_PID   P_PID
#   define P_PGID  P_PGID
#else /* __COMPILER_PREFERR_ENUMS */
typedef int idtype_t;
#   define P_ALL  0 /*< Wait for any child. */
#   define P_PID  1 /*< Wait for specified process. */
#   define P_PGID 2 /*< Wait for members of process group. */
#endif /* !__COMPILER_PREFERR_ENUMS */
#endif /* !__ENUM_IDTYPE_T */
#endif /* ... */

__SYSDECL_END

#endif /* !_BITS_GENERIC_WAITFLAGS_H */
