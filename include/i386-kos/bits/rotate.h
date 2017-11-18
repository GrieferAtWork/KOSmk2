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
#ifndef _I386_KOS_BITS_ROTATE_H
#define _I386_KOS_BITS_ROTATE_H 1
#define _BITS_ROTATE_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

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
#endif /* Asm... */

__SYSDECL_END

#include <bits-generic/rotate.h>

#endif /* !_I386_KOS_BITS_ROTATE_H */
