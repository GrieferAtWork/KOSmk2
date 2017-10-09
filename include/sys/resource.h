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
#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H 1

#include <features.h>
#include <bits/resource.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifndef __id_t_defined
#define __id_t_defined
typedef __id_t id_t;
#endif

#if defined(__USE_GNU) && !defined(__cplusplus)
typedef enum __rlimit_resource __rlimit_resource_t;
typedef enum __rusage_who      __rusage_who_t;
typedef enum __priority_which  __priority_which_t;
#else
typedef int __rlimit_resource_t;
typedef int __rusage_who_t;
typedef int __priority_which_t;
#endif

#ifndef __KERNEL__
__LIBC int (__LIBCCALL getrlimit)(__rlimit_resource_t __resource, struct rlimit *__rlimits) __FS_FUNC(getrlimit);
__LIBC int (__LIBCCALL setrlimit)(__rlimit_resource_t __resource, struct rlimit const *__rlimits) __FS_FUNC(setrlimit);
__LIBC int (__LIBCCALL getrusage)(__rusage_who_t __who, struct rusage *__usage);
__LIBC int (__LIBCCALL getpriority)(__priority_which_t __which, id_t __who);
__LIBC int (__LIBCCALL setpriority)(__priority_which_t __which, id_t __who, int __prio);
#ifdef __USE_LARGEFILE64
__LIBC int (__LIBCCALL getrlimit64)(__rlimit_resource_t __resource, struct rlimit64 *__rlimits);
__LIBC int (__LIBCCALL setrlimit64)(__rlimit_resource_t __resource, struct rlimit64 const *__rlimits);
#endif
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif    /* sys/resource.h  */
