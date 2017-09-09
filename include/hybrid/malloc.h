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
#endif

#ifdef __CC__
__DECL_BEGIN

__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC
void *(__LIBCCALL __libc_malloc)(__SIZE_TYPE__ __n_bytes) __ASMNAME("malloc");
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC
void *(__LIBCCALL __libc_calloc)(__SIZE_TYPE__ __count, __SIZE_TYPE__ __n_bytes) __ASMNAME("calloc");
__LIBC __SAFE void (__LIBCCALL __libc_free)(void *__restrict __mallptr) __ASMNAME("free");

#ifdef __USE_DEBUG
#if __USE_DEBUG != 0
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC
void *(__LIBCCALL __libc_malloc_d)(__SIZE_TYPE__ __n_bytes, __DEBUGINFO) __ASMNAME("_malloc_d");
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC
void *(__LIBCCALL __libc_calloc_d)(__SIZE_TYPE__ __count, __SIZE_TYPE__ __n_bytes, __DEBUGINFO) __ASMNAME("_calloc_d");
#ifdef __KERNEL__
__LIBC __SAFE void (__LIBCCALL __libc_free_d)(void *__restrict __mallptr, __DEBUGINFO) __ASMNAME("_kfree_d");
#else
__LIBC __SAFE void (__LIBCCALL __libc_free_d)(void *__restrict __mallptr, __DEBUGINFO) __ASMNAME("_free_d");
#endif
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

__DECL_END
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_MALLOC_H */
