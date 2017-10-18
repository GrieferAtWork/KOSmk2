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
#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H 1

#include <__stdinc.h>
#include <asm/types.h>
#include <hybrid/typecore.h>
#include "posix_types.h"

__DECL_BEGIN

#ifndef ____lebesuX_defined
#define ____lebesuX_defined 1

#ifndef __bitwise__
#ifdef __CHECKER__
#   define __bitwise__ __attribute__((bitwise))
#else /* __CHECKER__ */
#   define __bitwise__
#endif /* !__CHECKER__ */
#endif /* !__bitwise__ */

#ifndef __bitwise
#ifdef __CHECK_ENDIAN__
#   define __bitwise __bitwise__
#else /* __CHECK_ENDIAN__ */
#   define __bitwise
#endif /* !__CHECK_ENDIAN__ */
#endif /* !__bitwise */


#ifdef ____INTELLISENSE_STDINC_SYNTAX_H
typedef ::__int::____INTELLISENSE_integer<1234,__UINT16_TYPE__> __le16;
typedef ::__int::____INTELLISENSE_integer<4321,__UINT16_TYPE__> __be16;
typedef ::__int::____INTELLISENSE_integer<1234,__UINT32_TYPE__> __le32;
typedef ::__int::____INTELLISENSE_integer<4321,__UINT32_TYPE__> __be32;
typedef ::__int::____INTELLISENSE_integer<1234,__UINT64_TYPE__> __le64;
typedef ::__int::____INTELLISENSE_integer<4321,__UINT64_TYPE__> __be64;
#else /* ____INTELLISENSE_STDINC_SYNTAX_H */
typedef __UINT16_TYPE__ __bitwise __le16;
typedef __UINT32_TYPE__ __bitwise __le32;
typedef __UINT64_TYPE__ __bitwise __le64;
typedef __UINT16_TYPE__ __bitwise __be16;
typedef __UINT32_TYPE__ __bitwise __be32;
typedef __UINT64_TYPE__ __bitwise __be64;
#endif /* !____INTELLISENSE_STDINC_SYNTAX_H */
#endif /* !____lebesuX_defined */

typedef __UINT16_TYPE__ __sum16;
typedef __UINT32_TYPE__ __wsum;

#define __aligned_u64  __ATTR_ALIGNED(8) __u64
#define __aligned_be64 __ATTR_ALIGNED(8) __be64
#define __aligned_le64 __ATTR_ALIGNED(8) __le64


__DECL_END

#endif /* !_LINUX_TYPES_H */
