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
#ifndef _BITS_SIGSTACK_H
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


/* Structure describing a signal stack (obsolete). */
struct sigstack {
    void *ss_sp;      /*< Signal stack pointer. */
    int   ss_onstack; /*< Nonzero if executing on this stack. */
};

enum { /* Possible values for `ss_flags.'. */
    SS_ONSTACK = 1,
    SS_DISABLE = 2
#define SS_ONSTACK SS_ONSTACK
#define SS_DISABLE SS_DISABLE
};

#define MINSIGSTKSZ 2048 /*< Minimum stack size for a signal handler. */
#define SIGSTKSZ    8192 /*< System default stack size. */


#define __STACK_OFFSETOF_SP     0
#define __STACK_OFFSETOF_FLAGS  __SIZEOF_POINTER__
#define __STACK_OFFSETOF_SIZE  (__SIZEOF_POINTER__+__SIZEOF_INT__)
#define __STACK_SIZE           (__SIZEOF_POINTER__+__SIZEOF_INT__+__SIZEOF_SIZE_T__)

/* Alternate, preferred interface. */
typedef struct sigaltstack {
    void         *ss_sp;
    int           ss_flags;
    __SIZE_TYPE__ ss_size;
} stack_t;

__SYSDECL_END

#endif /* !_BITS_SIGSTACK_H */
