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
#ifndef __GUARD_HYBRID_ASM_H
#define __GUARD_HYBRID_ASM_H 1

#include "compiler.h"

DECL_BEGIN

#if defined(__ASSEMBLY__) || defined(__ASSEMBLER__)
#   define L(...)                  __VA_ARGS__;
#   define GLOBAL_ASM(...)         __VA_ARGS__
#elif defined(__CC__)
#   define __PRIVATE_ASM_LINE(...) #__VA_ARGS__ "\n"
#   define L(...)                  __PRIVATE_ASM_LINE(__VA_ARGS__)
#   define GLOBAL_ASM              __asm__
#else
#   define L(...)                  /* Nothing */
#   define GLOBAL_ASM(...)         /* Nothing */
#endif
#define SYM_PRIVATE(x)           .hidden x; .local x
#define SYM_INTERN(x)            .hidden x; .globl x
#define SYM_PUBLIC(x)                       .globl x
#define SYM_END(x)               .size x, . - x
#define FUN_PRIVATE(x)           .hidden x; .local x; .type x, @function
#define FUN_INTERN(x)            .hidden x; .globl x; .type x, @function
#define FUN_PUBLIC(x)                       .globl x; .type x, @function
#define OBJ_PRIVATE(x)           .hidden x; .local x; .type x, @object
#define OBJ_INTERN(x)            .hidden x; .globl x; .type x, @object
#define OBJ_PUBLIC(x)                       .globl x; .type x, @object
#define PRIVATE_LABEL(x)         .hidden x; .local x; x:
#define INTERN_LABEL(x)          .hidden x; .globl x; x:
#define PUBLIC_LABEL(x)                     .globl x; x:
#define PRIVATE_OBJECT(x)        .hidden x; .local x; .type x, @object; x:
#define INTERN_OBJECT(x)         .hidden x; .globl x; .type x, @object; x:
#define PUBLIC_OBJECT(x)                    .globl x; .type x, @object; x:
#define PRIVATE_ENTRY(x)         .hidden x; .local x; .type x, @function; x:
#define INTERN_ENTRY(x)          .hidden x; .globl x; .type x, @function; x:
#define PUBLIC_ENTRY(x)                     .globl x; .type x, @function; x:
#define DEFINE_BSS(name,n_bytes) name: .skip (n_bytes); .size name, . - name
#define PRIVATE_STRING(x,str)    PRIVATE_OBJECT(x) .string str; SYM_END(x)
#define INTERN_STRING(x,str)     INTERN_OBJECT(x)  .string str; SYM_END(x)
#define PUBLIC_STRING(x,str)     PUBLIC_OBJECT(x)  .string str; SYM_END(x)

#ifdef __PIC__
#   define PLT_SYM(x) x@PLT
#else
#   define PLT_SYM(x) x
#endif

DECL_END

#endif /* !__GUARD_HYBRID_ASM_H */
