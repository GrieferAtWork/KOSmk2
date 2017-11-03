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
#ifndef _BITS_ROTATE_H
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

#if defined(__COMPILER_HAVE_GCC_ASM)
#if defined(__NO_XBLOCK) || !defined(__NO_ATTR_FORCEINLINE)
#define __DEFINE_GCCASM_ROLR(name,inst,T,Ic) \
__FORCELOCAL __ATTR_CONST T __NOTHROW((__LIBCCALL name)(T __x, __UINT8_TYPE__ __shift)) { \
    register T __result; \
    __asm__ __volatile__(inst " %b1, %0\n" \
                         : "=g" (__result) \
                         : Ic (__shift), "0" (__x) \
                         : "cc"); \
    return __result; \
}
/* HINT: 'Ic' means: Prefer constant integer <= 31; else use the CX register.
 *       'Jc' means: Prefer constant integer <= 63; else use the CX register. */
__DEFINE_GCCASM_ROLR(__rol_nonconst_8,"rolb",__UINT8_TYPE__,"Ic")
__DEFINE_GCCASM_ROLR(__ror_nonconst_8,"rorb",__UINT8_TYPE__,"Ic")
__DEFINE_GCCASM_ROLR(__rol_nonconst_16,"rolw",__UINT16_TYPE__,"Ic")
__DEFINE_GCCASM_ROLR(__ror_nonconst_16,"rorw",__UINT16_TYPE__,"Ic")
__DEFINE_GCCASM_ROLR(__rol_nonconst_32,"roll",__UINT32_TYPE__,"Ic")
__DEFINE_GCCASM_ROLR(__ror_nonconst_32,"rorl",__UINT32_TYPE__,"Ic")
#ifdef __x86_64__
__DEFINE_GCCASM_ROLR(__rol_nonconst_64,"rolq",__UINT64_TYPE__,"Jc")
__DEFINE_GCCASM_ROLR(__ror_nonconst_64,"rorq",__UINT64_TYPE__,"Jc")
#endif /* __x86_64__ */
#undef __DEFINE_GCCASM_ROLR
#else
#define __GCCASM_ROLR(inst,Ic,T,x,shift) \
__XBLOCK({ register T __rl_res; \
           __asm__ __volatile__(inst " %b1, %0\n" \
                                : "=g" (__rl_res) \
                                : Ic (shift)\
                                , "0" (x) \
                                : "cc"); \
           __XRETURN __rl_res; \
})
#define __rol_nonconst_8(x,shift)  __GCCASM_ROLR("rolb","Ic",__UINT8_TYPE__,x,shift)
#define __ror_nonconst_8(x,shift)  __GCCASM_ROLR("rorb","Ic",__UINT8_TYPE__,x,shift)
#define __rol_nonconst_16(x,shift) __GCCASM_ROLR("rolw","Ic",__UINT16_TYPE__,x,shift)
#define __ror_nonconst_16(x,shift) __GCCASM_ROLR("rorw","Ic",__UINT16_TYPE__,x,shift)
#define __rol_nonconst_32(x,shift) __GCCASM_ROLR("roll","Ic",__UINT32_TYPE__,x,shift)
#define __ror_nonconst_32(x,shift) __GCCASM_ROLR("rorl","Ic",__UINT32_TYPE__,x,shift)
#ifdef __x86_64__
#define __rol_nonconst_64(x,shift) __GCCASM_ROLR("rolq","Jc",__UINT64_TYPE__,x,shift)
#define __ror_nonconst_64(x,shift) __GCCASM_ROLR("rorq","Jc",__UINT64_TYPE__,x,shift)
#endif /* __x86_64__ */
#endif
#elif defined(_MSC_VER)
__NAMESPACE_INT_BEGIN
extern unsigned char (__cdecl _rotl8)(unsigned char __x, unsigned char __shift);
extern unsigned short (__cdecl _rotl16)(unsigned short __x, unsigned char __shift);
extern unsigned int (__cdecl _rotl)(unsigned int __x, int __shift);
extern unsigned __int64 (__cdecl _rotl64)(unsigned __int64 __x, int __shift);
extern unsigned char (__cdecl _rotr8)(unsigned char __x, unsigned char __shift);
extern unsigned short (__cdecl _rotr16)(unsigned short __x, unsigned char __shift);
extern unsigned int (__cdecl _rotr)(unsigned int __x, int __shift);
extern unsigned __int64 (__cdecl _rotr64)(unsigned __int64 __x, int __shift);
#pragma intrinsic(_rotl8)
#pragma intrinsic(_rotl16)
#pragma intrinsic(_rotl)
#pragma intrinsic(_rotl64)
#pragma intrinsic(_rotr8)
#pragma intrinsic(_rotr16)
#pragma intrinsic(_rotr)
#pragma intrinsic(_rotr64)
__NAMESPACE_INT_END
#define __rol_nonconst_8(x,shift)  (__NAMESPACE_INT_SYM _rotl8)(x,shift)
#define __ror_nonconst_8(x,shift)  (__NAMESPACE_INT_SYM _rotr8)(x,shift)
#define __rol_nonconst_16(x,shift) (__NAMESPACE_INT_SYM _rotl16)(x,shift)
#define __ror_nonconst_16(x,shift) (__NAMESPACE_INT_SYM _rotr16)(x,shift)
#define __rol_nonconst_32(x,shift) (__NAMESPACE_INT_SYM _rotl)(x,shift)
#define __ror_nonconst_32(x,shift) (__NAMESPACE_INT_SYM _rotr)(x,shift)
#define __rol_nonconst_64(x,shift) (__NAMESPACE_INT_SYM _rotl64)(x,shift)
#define __ror_nonconst_64(x,shift) (__NAMESPACE_INT_SYM _rotr64)(x,shift)
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

#endif /* _BITS_ROTATE_H */
