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
#ifndef __GUARD_HYBRID_STRING_H
#define __GUARD_HYBRID_STRING_H 1

#include <hybrid/typecore.h>
#include <bits/types.h>
#include <asm-generic/string.h>
#include <features.h>

#ifdef __CC__
__SYSDECL_BEGIN

#define __hybrid_memcpy(dst,src,n_bytes)         __opt_memcpy(dst,src,n_bytes)
#define __hybrid_memmove(dst,src,n_bytes)        __opt_memmove(dst,src,n_bytes)
#define __hybrid_memset(dst,byte,n_bytes)        __opt_memset(dst,byte,n_bytes)
#define __hybrid_memcmp(a,b,n_bytes)             __opt_memcmp(a,b,n_bytes)
#define __hybrid_memchr(haystack,needle,n_bytes) __opt_memchr(haystack,needle,n_bytes)
#define __hybrid_strlen(s)                       __opt_strlen(s)
#define __hybrid_strnlen(s,max_chars)            __libc_strnlen(s,max_chars)
#define __hybrid_strcmp(s1,s2)                   __libc_strcmp(s1,s2)

__SYSDECL_END
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_STRING_H */
