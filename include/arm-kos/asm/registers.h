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
#ifndef _ARM_KOS_ASM_REGISTERS_H
#define _ARM_KOS_ASM_REGISTERS_H 1

#include <hybrid/typecore.h>

#if __SIZEOF_REGISTER__ == 4
#   define __ASM_SCRATCH_OFFSETOF_R0   0
#   define __ASM_SCRATCH_OFFSETOF_R1   4
#   define __ASM_SCRATCH_OFFSETOF_R2   8
#   define __ASM_SCRATCH_OFFSETOF_R3   12
#   define __ASM_SCRATCH_SIZE          16 /* 4*4 */
#elif __SIZEOF_REGISTER__ == 8
#   define __ASM_SCRATCH_OFFSETOF_R0   0
#   define __ASM_SCRATCH_OFFSETOF_R1   8
#   define __ASM_SCRATCH_OFFSETOF_R2   16
#   define __ASM_SCRATCH_OFFSETOF_R3   24
#   define __ASM_SCRATCH_SIZE          32 /* 4*8 */
#else
#   error "Unsupported sizeof(register_t)"
#endif
#define __ASM_PUSH_SCRATCH          push {r0-r3};
#define __ASM_POP_SCRATCH           pop  {r0-r3};

#if __SIZEOF_REGISTER__ == 4
#   define __ASM_SCRATCH_OFFSETOF_R4   0
#   define __ASM_SCRATCH_OFFSETOF_R5   4
#   define __ASM_SCRATCH_OFFSETOF_R6   8
#   define __ASM_SCRATCH_OFFSETOF_R7   12
#   define __ASM_SCRATCH_OFFSETOF_R8   16
#   define __ASM_SCRATCH_OFFSETOF_R9   20
#   define __ASM_SCRATCH_OFFSETOF_R10  24
#   define __ASM_SCRATCH_OFFSETOF_R11  28
#   define __ASM_SCRATCH_OFFSETOF_R13  32
#   define __ASM_PRESERVE_SIZE         36 /* 9*4 */
#elif __SIZEOF_REGISTER__ == 8
#   define __ASM_SCRATCH_OFFSETOF_R4   0
#   define __ASM_SCRATCH_OFFSETOF_R5   8
#   define __ASM_SCRATCH_OFFSETOF_R6   16
#   define __ASM_SCRATCH_OFFSETOF_R7   24
#   define __ASM_SCRATCH_OFFSETOF_R8   32
#   define __ASM_SCRATCH_OFFSETOF_R9   40
#   define __ASM_SCRATCH_OFFSETOF_R10  48
#   define __ASM_SCRATCH_OFFSETOF_R11  56
#   define __ASM_SCRATCH_OFFSETOF_R13  64
#   define __ASM_PRESERVE_SIZE         72 /* 9*8 */
#else
#   error "Unsupported sizeof(register_t)"
#endif
#define __ASM_PUSH_PRESERVE         push {r4-r11,r13};
#define __ASM_POP_PRESERVE          pop  {r13,r4-r11};


#endif /* !_ARM_KOS_ASM_REGISTERS_H */
