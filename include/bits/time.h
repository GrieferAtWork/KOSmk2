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
#ifndef _BITS_TIME_H
#define _BITS_TIME_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>

__DECL_BEGIN

#ifndef _STRUCT_TIMEVAL
#define _STRUCT_TIMEVAL 1
#ifndef __timeval_defined
#define __timeval_defined 1
struct timeval {
    __time_t      tv_sec;  /*< Seconds. */
    __suseconds_t tv_usec; /*< Microseconds. */
};
#endif /* !__timeval_defined */
#endif /* !_STRUCT_TIMEVAL */

#ifndef __timeval32_t_defined
#define __timeval32_t_defined 1
#ifndef __USE_TIME_BITS64
#define __timeval32 timeval
#ifdef __USE_KOS
#define timeval32   timeval
#endif /* __USE_KOS */
#else /* !__USE_TIME_BITS64 */
#ifdef __USE_KOS
#define __timeval32 timeval32
#endif /* __USE_KOS */
struct __timeval32 {
    __time32_t    tv_sec;  /*< Seconds. */
    __suseconds_t tv_usec; /*< Microseconds. */
};
#endif /* __USE_TIME_BITS64 */
#endif /* !__timeval32_t_defined */

#ifndef __timeval64_t_defined
#define __timeval64_t_defined 1
#ifdef __USE_TIME_BITS64
#define __timeval64 timeval
#ifdef __USE_TIME64
#define timeval64   timeval
#endif /* __USE_TIME64 */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_TIME64
#define __timeval64 timeval64
#endif /* __USE_TIME64 */
struct __timeval64 {
    __time64_t    tv_sec;  /*< Seconds. */
    __suseconds_t tv_usec; /*< Microseconds. */
};
#endif /* !__USE_TIME_BITS64 */
#endif /* !__timeval64_t_defined */


/* System-dependent timing definitions.  Linux version.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
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

#define __DOS_CLOCKS_PER_SEC  ((clock_t)1000)
#define __KOS_CLOCKS_PER_SEC  ((clock_t)1000000)
#ifdef __USE_DOS
#define CLOCKS_PER_SEC  __DOS_CLOCKS_PER_SEC
#else /* __USE_DOS */
#define CLOCKS_PER_SEC  __KOS_CLOCKS_PER_SEC
#endif /* !__USE_DOS */

#ifndef __KERNEL__
#if (!defined(__STRICT_ANSI__) || defined(__USE_POSIX)) && \
     !defined(__USE_XOPEN2K)
__LIBC long int (__LIBCCALL __sysconf)(int);
#define CLK_TCK ((__clock_t)__sysconf(2))
#endif
#endif /* !__KERNEL__ */


#ifdef __USE_POSIX199309
#   define CLOCK_REALTIME           0 /*< Identifier for system-wide realtime clock. */
#   define CLOCK_MONOTONIC          1 /*< Monotonic system-wide clock. */
#   define CLOCK_PROCESS_CPUTIME_ID 2 /*< High-resolution timer from the CPU. */
#   define CLOCK_THREAD_CPUTIME_ID  3 /*< Thread-specific CPU-time clock. */
#   define CLOCK_MONOTONIC_RAW      4 /*< Monotonic system-wide clock, not adjusted for frequency scaling. */
#   define CLOCK_REALTIME_COARSE    5 /*< Identifier for system-wide realtime clock, updated only on ticks. */
#   define CLOCK_MONOTONIC_COARSE   6 /*< Monotonic system-wide clock, updated only on ticks. */
#   define CLOCK_BOOTTIME           7 /*< Monotonic system-wide clock that includes time spent in suspension. */
#   define CLOCK_REALTIME_ALARM     8 /*< Like CLOCK_REALTIME but also wakes suspended system. */
#   define CLOCK_BOOTTIME_ALARM     9 /*< Like CLOCK_BOOTTIME but also wakes suspended system. */
#   define CLOCK_TAI               11 /*< Like CLOCK_REALTIME but in International Atomic Time. */
#   define TIMER_ABSTIME            1 /*< Flag to indicate time is absolute. */
#endif

#ifdef __USE_GNU
struct timex;
__LIBC int (__LIBCCALL clock_adjtime)(__clockid_t __clock_id, struct timex *__utx);
#endif /* __USE_GNU */

__DECL_END

#endif /* !_BITS_TIME_H */
