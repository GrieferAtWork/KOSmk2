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
#ifndef _BITS_GENERIC_ROTATE_H
#define _BITS_GENERIC_ROTATE_H 1
#define _BITS_ROTATE_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#define __rol_constant_8(x,shift)  (((x)>>(0x8-((shift)&0x07)))|((x)<<((shift)&0x07)))
#define __ror_constant_8(x,shift)  (((x)<<(0x8-((shift)&0x07)))|((x)>>((shift)&0x07)))
#define __rol_constant_16(x,shift) (((x)>>(0x10-((shift)&0x0f)))|((x)<<((shift)&0x0f)))
#define __ror_constant_16(x,shift) (((x)<<(0x10-((shift)&0x0f)))|((x)>>((shift)&0x0f)))
#define __rol_constant_32(x,shift) (((x)>>(0x20-((shift)&0x1f)))|((x)<<((shift)&0x1f)))
#define __ror_constant_32(x,shift) (((x)<<(0x20-((shift)&0x1f)))|((x)>>((shift)&0x1f)))
#define __rol_constant_64(x,shift) (((x)>>(0x40-((shift)&0x3f)))|((x)<<((shift)&0x3f)))
#define __ror_constant_64(x,shift) (((x)<<(0x40-((shift)&0x3f)))|((x)>>((shift)&0x3f)))

#if defined(_MSC_VER)
__NAMESPACE_INT_BEGIN
#ifndef __rol_nonconst_8
extern unsigned char (__cdecl _rotl8)(unsigned char __x, unsigned char __shift);
extern unsigned char (__cdecl _rotr8)(unsigned char __x, unsigned char __shift);
#define __rol_nonconst_8(x,shift)  (__NAMESPACE_INT_SYM _rotl8)(x,shift)
#define __ror_nonconst_8(x,shift)  (__NAMESPACE_INT_SYM _rotr8)(x,shift)
#pragma intrinsic(_rotl8)
#pragma intrinsic(_rotr8)
#endif /* !__rol_nonconst_8 */
#ifndef __rol_nonconst_16
extern unsigned short (__cdecl _rotl16)(unsigned short __x, unsigned char __shift);
extern unsigned short (__cdecl _rotr16)(unsigned short __x, unsigned char __shift);
#define __rol_nonconst_16(x,shift) (__NAMESPACE_INT_SYM _rotl16)(x,shift)
#define __ror_nonconst_16(x,shift) (__NAMESPACE_INT_SYM _rotr16)(x,shift)
#pragma intrinsic(_rotl16)
#pragma intrinsic(_rotr16)
#endif /* !__rol_nonconst_16 */
#ifndef __rol_nonconst_32
extern unsigned int (__cdecl _rotl)(unsigned int __x, int __shift);
extern unsigned int (__cdecl _rotr)(unsigned int __x, int __shift);
#define __rol_nonconst_32(x,shift) (__NAMESPACE_INT_SYM _rotl)(x,shift)
#define __ror_nonconst_32(x,shift) (__NAMESPACE_INT_SYM _rotr)(x,shift)
#pragma intrinsic(_rotl)
#pragma intrinsic(_rotr)
#endif /* !__rol_nonconst_32 */
#ifndef __rol_nonconst_64
extern unsigned __int64 (__cdecl _rotl64)(unsigned __int64 __x, int __shift);
extern unsigned __int64 (__cdecl _rotr64)(unsigned __int64 __x, int __shift);
#define __rol_nonconst_64(x,shift) (__NAMESPACE_INT_SYM _rotl64)(x,shift)
#define __ror_nonconst_64(x,shift) (__NAMESPACE_INT_SYM _rotr64)(x,shift)
#pragma intrinsic(_rotl64)
#pragma intrinsic(_rotr64)
#endif /* !__rol_nonconst_64 */
__NAMESPACE_INT_END
#endif /* Intrin... */

#ifndef __rol_nonconst_8
#define __rol_nonconst_8(x,shift)  __rol_constant_8(x,shift)
#define __ror_nonconst_8(x,shift)  __ror_constant_8(x,shift)
#endif /* !__rol_nonconst_8 */
#ifndef __rol_nonconst_16
#define __rol_nonconst_16(x,shift) __rol_constant_16(x,shift)
#define __ror_nonconst_16(x,shift) __ror_constant_16(x,shift)
#endif /* !__rol_nonconst_16 */
#ifndef __rol_nonconst_32
#define __rol_nonconst_32(x,shift) __rol_constant_32(x,shift)
#define __ror_nonconst_32(x,shift) __ror_constant_32(x,shift)
#endif /* !__rol_nonconst_32 */
#ifndef __rol_nonconst_64
#define __rol_nonconst_64(x,shift) __rol_constant_64(x,shift)
#define __ror_nonconst_64(x,shift) __ror_constant_64(x,shift)
#endif /* !__rol_nonconst_64 */

#ifdef __NO_builtin_constant_p
#define __rol_8(x,shift)    __rol_nonconst_8(x,shift)
#define __rol_16(x,shift)   __rol_nonconst_16(x,shift)
#define __rol_32(x,shift)   __rol_nonconst_32(x,shift)
#define __rol_64(x,shift)   __rol_nonconst_64(x,shift)
#define __ror_8(x,shift)    __ror_nonconst_8(x,shift)
#define __ror_16(x,shift)   __ror_nonconst_16(x,shift)
#define __ror_32(x,shift)   __ror_nonconst_32(x,shift)
#define __ror_64(x,shift)   __ror_nonconst_64(x,shift)
#else /* __NO_builtin_constant_p */
#define __rol_8(x,shift)  ((__builtin_constant_p(x) && __builtin_constant_p(shift)) ? __rol_constant_8(x,shift)  : __rol_nonconst_8(x,shift))
#define __rol_16(x,shift) ((__builtin_constant_p(x) && __builtin_constant_p(shift)) ? __rol_constant_16(x,shift) : __rol_nonconst_16(x,shift))
#define __rol_32(x,shift) ((__builtin_constant_p(x) && __builtin_constant_p(shift)) ? __rol_constant_32(x,shift) : __rol_nonconst_32(x,shift))
#define __rol_64(x,shift) ((__builtin_constant_p(x) && __builtin_constant_p(shift)) ? __rol_constant_64(x,shift) : __rol_nonconst_64(x,shift))
#define __ror_8(x,shift)  ((__builtin_constant_p(x) && __builtin_constant_p(shift)) ? __ror_constant_8(x,shift)  : __ror_nonconst_8(x,shift))
#define __ror_16(x,shift) ((__builtin_constant_p(x) && __builtin_constant_p(shift)) ? __ror_constant_16(x,shift) : __ror_nonconst_16(x,shift))
#define __ror_32(x,shift) ((__builtin_constant_p(x) && __builtin_constant_p(shift)) ? __ror_constant_32(x,shift) : __ror_nonconst_32(x,shift))
#define __ror_64(x,shift) ((__builtin_constant_p(x) && __builtin_constant_p(shift)) ? __ror_constant_64(x,shift) : __ror_nonconst_64(x,shift))
#endif /* !__NO_builtin_constant_p */

__SYSDECL_END

#endif /* !_BITS_GENERIC_ROTATE_H */
