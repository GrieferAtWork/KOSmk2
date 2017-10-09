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
#ifndef __GUARD_HYBRID_TIMEVAL_H
#define __GUARD_HYBRID_TIMEVAL_H 1

#include <__stdinc.h>
#include <bits/types.h>
#include <features.h>

__SYSDECL_BEGIN

#ifdef __CC__

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

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

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif /* __CC__ */

__SYSDECL_END

#endif /* !__GUARD_HYBRID_TIMEVAL_H */
