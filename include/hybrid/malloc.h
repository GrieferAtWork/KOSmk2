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
#ifndef __GUARD_HYBRID_MALLOC_H
#define __GUARD_HYBRID_MALLOC_H 1

#include <hybrid/typecore.h>
#include <bits/types.h>
#include <features.h>
#include <__malldefs.h>

#ifdef __USE_DEBUG
#include <hybrid/debuginfo.h>
#endif /* __USE_DEBUG */

#ifdef __CC__
__SYSDECL_BEGIN

__REDIRECT(__LIBC,__SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC,
           void *,__LIBCCALL,__libc_malloc,(__SIZE_TYPE__ __n_bytes),malloc,(__n_bytes))
__REDIRECT(__LIBC,__SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC,
           void *,__LIBCCALL,__libc_calloc,(__SIZE_TYPE__ __count, __SIZE_TYPE__ __n_bytes),calloc,(__count,__n_bytes))
__REDIRECT_VOID(__LIBC,__SAFE,__LIBCCALL,__libc_free,(void *__restrict __mallptr),free,(__mallptr))
#ifdef __USE_DEBUG
#if __USE_DEBUG != 0 && defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__REDIRECT(__LIBC,__SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC,
           void *,__LIBCCALL,__libc_malloc_d,(__SIZE_TYPE__ __n_bytes, __DEBUGINFO),_malloc_d,(__n_bytes,__DEBUGINFO_FWD))
__REDIRECT(__LIBC,__SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC,
           void *,__LIBCCALL,__libc_calloc_d,(__SIZE_TYPE__ __count, __SIZE_TYPE__ __n_bytes, __DEBUGINFO),
           _calloc_d,(__count,__n_bytes,__DEBUGINFO_FWD))
#ifdef __KERNEL__
__REDIRECT_VOID(__LIBC,__SAFE,__LIBCCALL,__libc_free_d,(void *__restrict __mallptr, __DEBUGINFO),_kfree_d,(__mallptr,__DEBUGINFO_FWD))
#else /* __KERNEL__ */
__REDIRECT_VOID(__LIBC,__SAFE,__LIBCCALL,__libc_free_d,(void *__restrict __mallptr, __DEBUGINFO),_free_d,(__mallptr,__DEBUGINFO_FWD))
#endif /* !__KERNEL__ */
#else /* __USE_DEBUG != 0 */
#define __libc_malloc_d(n_bytes,...)       __libc_malloc(n_bytes)
#define __libc_calloc_d(count,n_bytes,...) __libc_calloc(count,n_bytes)
#define __libc_free_d(mallptr,...)         __libc_free(mallptr)
#endif /* __USE_DEBUG == 0 */
#endif /* __USE_DEBUG */

#ifdef __USE_DEBUG_HOOK
#   define __hybrid_malloc(n_bytes)       __libc_malloc_d(n_bytes,__DEBUGINFO_GEN)
#   define __hybrid_calloc(count,n_bytes) __libc_calloc_d(count,n_bytes,__DEBUGINFO_GEN)
#   define __hybrid_free(mallptr)         __libc_free_d(mallptr,__DEBUGINFO_GEN)
#else
#   define __hybrid_malloc(n_bytes)       __libc_malloc(n_bytes)
#   define __hybrid_calloc(count,n_bytes) __libc_calloc(count,n_bytes)
#   define __hybrid_free(mallptr)         __libc_free(mallptr)
#endif

__SYSDECL_END
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_MALLOC_H */
