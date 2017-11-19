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
#ifndef _BIT_GENERIC_SIGSTACK_H
#define _BIT_GENERIC_SIGSTACK_H 1
#define _BITS_SIGSTACK_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

/* sigstack, sigaltstack definitions.
   Copyright (C) 1998-2016 Free Software Foundation, Inc.
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

#ifdef __CC__
/* Structure describing a signal stack (obsolete). */
struct sigstack {
    void *ss_sp;      /*< Signal stack pointer. */
    int   ss_onstack; /*< Nonzero if executing on this stack. */
};
#endif /* __CC__ */

#ifdef __COMPILER_PREFERR_ENUMS
enum { /* Possible values for `ss_flags.'. */
    SS_ONSTACK = 1,
    SS_DISABLE = 2
#define SS_ONSTACK SS_ONSTACK
#define SS_DISABLE SS_DISABLE
};
#else
/* Possible values for `ss_flags.'. */
#define SS_ONSTACK 1
#define SS_DISABLE 2
#endif

#ifndef MINSIGSTKSZ
#define MINSIGSTKSZ 2048 /*< Minimum stack size for a signal handler. */
#endif /* !MINSIGSTKSZ */
#ifndef SIGSTKSZ
#define SIGSTKSZ    8192 /*< System default stack size. */
#endif /* !SIGSTKSZ */


#define __STACK_OFFSETOF_SP       0
#define __STACK_OFFSETOF_FLAGS    __SIZEOF_POINTER__
#define __STACK_OFFSETOF_SIZE  (2*__SIZEOF_POINTER__)
#define __STACK_SIZE           (2*__SIZEOF_POINTER__+__SIZEOF_SIZE_T__)

#ifdef __CC__
/* Alternate, preferred interface. */
typedef struct sigaltstack {
    void         *ss_sp;
    int           ss_flags;
#if __SIZEOF_POINTER__ > __SIZEOF_INT__
    __BYTE_TYPE__ __ss_pad[__SIZEOF_POINTER__-__SIZEOF_INT__];
#endif
    __SIZE_TYPE__ ss_size;
} stack_t;

#endif /* __CC__ */


__SYSDECL_END

#endif /* !_BIT_GENERIC_SIGSTACK_H */
