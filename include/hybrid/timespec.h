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
#ifndef __GUARD_HYBRID_TIMESPEC_H
#define __GUARD_HYBRID_TIMESPEC_H 1

#include <__stdinc.h>
#include <bits/types.h>
#include <features.h>

__SYSDECL_BEGIN

#define __SIZEOF_TIMESPEC       (__SIZEOF_TIME_T__+__SIZEOF_SYSCALL_LONG__)

#ifdef __CC__
#ifdef _MSC_VER
#pragma pack(push,1)
#endif
struct timespec {
 __TM_TYPE(time)   tv_sec;  /*< Seconds. */
 __syscall_slong_t tv_nsec; /*< Nano seconds. */
};

/* Define 64-bit and 32-bit alternatives of `struct timespec' */
#ifdef __USE_TIME_BITS64
#define __timespec64 timespec
#ifdef __USE_TIME64
#define timespec64 timespec
#endif /* __USE_TIME64 */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_TIME64
#define __timespec64 timespec64
#endif /* __USE_TIME64 */
struct __timespec64 {
 __time64_t        tv_sec;  /*< Seconds. */
 __syscall_slong_t tv_nsec; /*< Nano seconds. */
};
#endif /* !__USE_TIME_BITS64 */

#ifndef __USE_TIME_BITS64
#define __timespec32 timespec
#ifdef __USE_KOS
#define timespec32 timespec
#endif /* __USE_KOS */
#else /* !__USE_TIME_BITS64 */
#ifdef __USE_KOS
#define __timespec32 timespec32
#endif /* __USE_KOS */
struct __timespec32 {
 __time32_t        tv_sec;  /*< Seconds. */
 __syscall_slong_t tv_nsec; /*< Nano seconds. */
};
#endif /* __USE_TIME_BITS64 */

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#define __NSECS_PER_SEC   1000000000l

#define __TIMESPEC_ADD(x,y) \
 (void)((x).tv_nsec += (y).tv_nsec, \
        (x).tv_sec  += (x).tv_nsec/__NSECS_PER_SEC, \
        (x).tv_nsec %= __NSECS_PER_SEC, \
        (x).tv_sec  += (y).tv_sec)
#define __TIMESPEC_SUB(x,y) \
 ((x).tv_sec -= (y).tv_sec,((x).tv_nsec -= (y).tv_nsec) < 0 \
   ? (void)((x).tv_sec -= (-(x).tv_nsec)/__NSECS_PER_SEC, \
            (x).tv_nsec = (-(x).tv_nsec)%__NSECS_PER_SEC) : (void)0)
#define __TIMESPEC_LOWER(x,y)         ((x).tv_sec <  (y).tv_sec || \
                                      ((x).tv_sec == (y).tv_sec && (x).tv_nsec < (y).tv_nsec))
#define __TIMESPEC_LOWER_EQUAL(x,y)   ((x).tv_sec <  (y).tv_sec || \
                                      ((x).tv_sec == (y).tv_sec && (x).tv_nsec <= (y).tv_nsec))
#define __TIMESPEC_EQUAL(x,y)         ((x).tv_sec == (y).tv_sec && (x).tv_nsec == (y).tv_nsec)
#define __TIMESPEC_NOT_EQUAL(x,y)     ((x).tv_sec != (y).tv_sec || (x).tv_nsec != (y).tv_nsec)
#define __TIMESPEC_GREATER(x,y)       ((x).tv_sec >  (y).tv_sec || \
                                      ((x).tv_sec == (y).tv_sec && (x).tv_nsec > (y).tv_nsec))
#define __TIMESPEC_GREATER_EQUAL(x,y) ((x).tv_sec >  (y).tv_sec || \
                                      ((x).tv_sec == (y).tv_sec && (x).tv_nsec >= (y).tv_nsec))

#ifdef __GUARD_HYBRID_COMPILER_H
#   define TIMESPEC_ADD           __TIMESPEC_ADD
#   define TIMESPEC_SUB           __TIMESPEC_SUB
#   define TIMESPEC_LOWER         __TIMESPEC_LOWER
#   define TIMESPEC_LOWER_EQUAL   __TIMESPEC_LOWER_EQUAL
#   define TIMESPEC_EQUAL         __TIMESPEC_EQUAL
#   define TIMESPEC_NOT_EQUAL     __TIMESPEC_NOT_EQUAL
#   define TIMESPEC_GREATER       __TIMESPEC_GREATER
#   define TIMESPEC_GREATER_EQUAL __TIMESPEC_GREATER_EQUAL
#endif
#endif /* __CC__ */

__SYSDECL_END

#endif /* !__GUARD_HYBRID_TIMESPEC_H */
