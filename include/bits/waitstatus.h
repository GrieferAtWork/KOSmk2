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
#ifndef _BITS_WAITSTATUS_H
#define _BITS_WAITSTATUS_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/waitflags.h>
#ifdef __USE_MISC
#include <bits/endian.h>
#include <hybrid/byteorder.h>
#endif /* __USE_MISC */

__SYSDECL_BEGIN

/* NOTE: This file based on the GLIBC header of the same name: */
/* Definitions of status bits for `wait' et al.
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


/* Everything extant so far uses these same bits. */


#define __WEXITSTATUS(status)  (((status)&0xff00)>>8)    /* If WIFEXITED(STATUS), the low-order 8 bits of the status. */
#define __WTERMSIG(status)      ((status)&0x7f)          /* If WIFSIGNALED(STATUS), the terminating signal. */
#define __WSTOPSIG(status)      __WEXITSTATUS(status)    /* If WIFSTOPPED(STATUS), the signal that stopped the child. */
#define __WIFEXITED(status)    (__WTERMSIG(status)==0)   /* Nonzero if STATUS indicates normal termination. */
#define __WIFSIGNALED(status)  (((signed char)(((status)&0x7f)+1)>>1)>0) /* Nonzero if STATUS indicates termination by a signal. */
#define __WIFSTOPPED(status)   (((status)&0xff)==0x7f)   /* Nonzero if STATUS indicates the child is stopped. */
#define __WCOREDUMP(status)    ((status)&__WCOREFLAG)    /* Nonzero if STATUS indicates the child dumped core. */
#ifdef WCONTINUED
#define __WIFCONTINUED(status) ((status)==__W_CONTINUED) /* Nonzero if STATUS indicates the child continued after a stop. */
#endif

/* Macros for constructing status values. */
#define __W_EXITCODE(ret,sig) ((ret)<<8|(sig))
#define __W_STOPCODE(sig)     ((sig)<<8|0x7f)
#define __W_CONTINUED           0xffff
#define __WCOREFLAG             0x80

#ifdef __USE_MISC
union wait {
    int w_status;
#ifdef __COMPILER_HAVE_TRANSPARENT_STRUCT
    struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        unsigned int w_termsig  : 7; /*< Terminating signal. */
        unsigned int w_coredump : 1; /*< Set if dumped core. */
        unsigned int w_retcode  : 8; /*< Return code if exited normally. */
        unsigned int __w_pad0   : 16;
#elif __BYTE_ORDER == __BIG_ENDIAN
        unsigned int __w_pad0   : 16;
        unsigned int w_retcode  : 8; /*< Return code if exited normally. */
        unsigned int w_coredump : 1; /*< Set if dumped core. */
        unsigned int w_termsig  : 7; /*< Terminating signal. */
#endif /* Endian... */
    };
    struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        unsigned int w_stopval : 8; /*< W_STOPPED if stopped. */
        unsigned int w_stopsig : 8; /*< Stopping signal. */
        unsigned int __w_pad1  : 16;
#elif __BYTE_ORDER == __BIG_ENDIAN
        unsigned int __w_pad1  : 16;
        unsigned int w_stopsig : 8; /*< Stopping signal. */
        unsigned int w_stopval : 8; /*< W_STOPPED if stopped. */
#endif /* Endian... */
    };
#endif /* __COMPILER_HAVE_TRANSPARENT_STRUCT */
#if !defined(__USE_KOS) || \
    !defined(__COMPILER_HAVE_TRANSPARENT_STRUCT)
    struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        unsigned int __w_termsig  : 7; /* Terminating signal. */
        unsigned int __w_coredump : 1; /* Set if dumped core. */
        unsigned int __w_retcode  : 8; /* Return code if exited normally. */
        unsigned int __w_pad0     : 16;
#elif __BYTE_ORDER == __BIG_ENDIAN
        unsigned int __w_pad0     : 16;
        unsigned int __w_retcode  : 8; /* Return code if exited normally. */
        unsigned int __w_coredump : 1; /* Set if dumped core. */
        unsigned int __w_termsig  : 7; /* Terminating signal. */
#endif /* Endian... */
    } __wait_terminated;
    struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        unsigned int __w_stopval : 8; /*< W_STOPPED if stopped. */
        unsigned int __w_stopsig : 8; /*< Stopping signal. */
        unsigned int __w_pad1    : 16;
#elif __BYTE_ORDER == __BIG_ENDIAN
        unsigned int __w_pad1    : 16;
        unsigned int __w_stopsig : 8; /*< Stopping signal. */
        unsigned int __w_stopval : 8; /*< W_STOPPED if stopped. */
#endif /* Endian... */
    } __wait_stopped;
#ifndef __COMPILER_HAVE_TRANSPARENT_STRUCT
#define w_termsig  __wait_terminated.__w_termsig
#define w_coredump __wait_terminated.__w_coredump
#define w_retcode  __wait_terminated.__w_retcode
#define w_stopsig  __wait_stopped.__w_stopsig
#define w_stopval  __wait_stopped.__w_stopval
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
#endif /* !__USE_KOS || !__COMPILER_HAVE_TRANSPARENT_STRUCT */
};

#endif /* __USE_MISC */

__SYSDECL_END

#endif /* !_BITS_WAITSTATUS_H */
