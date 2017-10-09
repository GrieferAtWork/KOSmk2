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
#ifndef _BITS_BYTESWAP_H
#define _BITS_BYTESWAP_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#define __bswap_constant_16(x)  ((((x)&0xffu) << 8)|(((x) >> 8)&0xffu))
#define __bswap_constant_32(x)  ((__bswap_constant_16(x) << 16)|__bswap_constant_16((x) >> 16))
#define __bswap_constant_64(x)  ((__bswap_constant_32(x) << 32)|__bswap_constant_32((x) >> 32))

#if defined(__GNUC__) || __has_builtin(__builtin_bswap16)
#   define __bswap_nonconst_16(x) __builtin_bswap16(x)
#elif defined(__CRT_DOS)
#ifndef ___byteswap_ushort_defined
#define ___byteswap_ushort_defined 1
__LIBC __ATTR_CONST __UINT16_TYPE__ __NOTHROW((__LIBCCALL _byteswap_ushort)(__UINT16_TYPE__ x));
#ifdef _MSC_VER
#pragma intrinsic(_byteswap_ushort)
#endif /* _MSC_VER */
#endif /* !___byteswap_ushort_defined */
#   define __bswap_nonconst_16(x) _byteswap_ushort(x)
#else /* ... */
#   define __bswap_nonconst_16(x) __bswap_constant_16(x)
#endif /* !... */

#if defined(__GNUC__) || __has_builtin(__builtin_bswap32)
#   define __bswap_nonconst_32(x) __builtin_bswap32(x)
#elif defined(__CRT_DOS)
#ifndef ___byteswap_ulong_defined
#define ___byteswap_ulong_defined 1
__LIBC __ATTR_CONST __UINT32_TYPE__ __NOTHROW((__LIBCCALL _byteswap_ulong)(__UINT32_TYPE__ x));
#ifdef _MSC_VER
#pragma intrinsic(_byteswap_ulong)
#endif /* _MSC_VER */
#endif /* !___byteswap_ulong_defined */
#   define __bswap_nonconst_32(x) _byteswap_ulong(x)
#else /* ... */
#   define __bswap_nonconst_32(x) __bswap_constant_32(x)
#endif /* !... */

#if defined(__GNUC__) || __has_builtin(__builtin_bswap64)
#   define __bswap_nonconst_64(x) __builtin_bswap64(x)
#elif defined(__CRT_DOS)
#ifndef ___byteswap_uint64_defined
#define ___byteswap_uint64_defined 1
__LIBC __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _byteswap_uint64)(__UINT64_TYPE__ x));
#ifdef _MSC_VER
#pragma intrinsic(_byteswap_uint64)
#endif /* _MSC_VER */
#endif /* !___byteswap_uint64_defined */
#   define __bswap_nonconst_64(x) _byteswap_uint64(x)
#else /* ... */
#   define __bswap_nonconst_64(x) __bswap_constant_64(x)
#endif /* !... */

#ifndef __NO_builtin_constant_p
#define __bswap_16(x) (__builtin_constant_p(x) ? __bswap_constant_16(x) : __bswap_nonconst_16(x))
#define __bswap_32(x) (__builtin_constant_p(x) ? __bswap_constant_32(x) : __bswap_nonconst_32(x))
#define __bswap_64(x) (__builtin_constant_p(x) ? __bswap_constant_64(x) : __bswap_nonconst_64(x))
#else
#define __bswap_16(x)  __bswap_nonconst_16(x)
#define __bswap_32(x)  __bswap_nonconst_32(x)
#define __bswap_64(x)  __bswap_nonconst_64(x)
#endif

__SYSDECL_END

#endif /* _BITS_BYTESWAP_H */
