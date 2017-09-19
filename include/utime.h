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
#ifndef _UTIME_H
#define _UTIME_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>

__DECL_BEGIN

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K)
#ifndef __time_t_defined
#define __time_t_defined 1
typedef __TM_TYPE(time) time_t;
#endif /* !__time_t_defined */
#endif /* __USE_XOPEN || __USE_XOPEN2K */

struct utimbuf {
 __TM_TYPE(time) actime;  /*< Access time. */
 __TM_TYPE(time) modtime; /*< Modification time. */
};

#ifdef __USE_TIME64
struct utimbuf64 {
 __time64_t      actime;  /*< Access time. */
 __time64_t      modtime; /*< Modification time. */
};
#endif

#ifndef __KERNEL__
__LIBC __NONNULL((1)) int (__LIBCCALL utime)(char const *__file, struct utimbuf const *__file_times) __TM_FUNC(utime);
#ifdef __USE_TIME64
__LIBC __NONNULL((1)) int (__LIBCCALL utime64)(char const *__file, struct utimbuf64 const *__file_times);
#endif /* __USE_TIME64 */
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_UTIME_H */
