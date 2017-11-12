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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_ASM_H
#define GUARD_INCLUDE_KERNEL_ARCH_ASM_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <hybrid/types.h>

DECL_BEGIN

#ifdef __x86_64__
#define __ASM_REGOFF_1_rax  0
#define __ASM_REGOFF_1_rcx  0
#define __ASM_REGOFF_1_rdx  0
#define __ASM_REGOFF_1_rbx  0
#define __ASM_REGOFF_1_rsp  0
#define __ASM_REGOFF_1_rbp  0
#define __ASM_REGOFF_1_rsi  0
#define __ASM_REGOFF_1_rdi  0
#define __ASM_REGOFF_1_r8   1
#define __ASM_REGOFF_1_r9   1
#define __ASM_REGOFF_1_r10  1
#define __ASM_REGOFF_1_r11  1
#define __ASM_REGOFF_1_r12  1
#define __ASM_REGOFF_1_r13  1
#define __ASM_REGOFF_1_r14  1
#define __ASM_REGOFF_1_r15  1
#define __ASM_REGOFF_1(reg) __ASM_REGOFF_1_##reg
#define __ASM_REGOFF_2_rax  0
#define __ASM_REGOFF_2_rcx  1
#define __ASM_REGOFF_2_rdx  2
#define __ASM_REGOFF_2_rbx  3
#define __ASM_REGOFF_2_rsp  4
#define __ASM_REGOFF_2_rbp  5
#define __ASM_REGOFF_2_rsi  6
#define __ASM_REGOFF_2_rdi  7
#define __ASM_REGOFF_2_r8   0
#define __ASM_REGOFF_2_r9   1
#define __ASM_REGOFF_2_r10  2
#define __ASM_REGOFF_2_r11  3
#define __ASM_REGOFF_2_r12  4
#define __ASM_REGOFF_2_r13  5
#define __ASM_REGOFF_2_r14  6
#define __ASM_REGOFF_2_r15  7
#define __ASM_REGOFF_2(reg) __ASM_REGOFF_2_##reg

#define __BEGIN_FSGSBASE_INSTRUCTION \
        .pushsection .rodata.fixup.fsgsbase; \
        .long 991f - 0xffffffff80000000; \
        .popsection; 991:

/* Safely encode (rd|wr)(fs|gs)base instructions. */
#ifdef CONFIG_BUILDING_KERNEL_CORE
#define ASM_RDFSBASE(reg) __BEGIN_FSGSBASE_INSTRUCTION .byte 0xf3; .byte 0x48+__ASM_REGOFF_1(reg); .byte 0x0f; .byte 0xae; .byte 0xc0+__ASM_REGOFF_2(reg);
#define ASM_RDGSBASE(reg) __BEGIN_FSGSBASE_INSTRUCTION .byte 0xf3; .byte 0x48+__ASM_REGOFF_1(reg); .byte 0x0f; .byte 0xae; .byte 0xc8+__ASM_REGOFF_2(reg);
#define ASM_WRFSBASE(reg) __BEGIN_FSGSBASE_INSTRUCTION .byte 0xf3; .byte 0x48+__ASM_REGOFF_1(reg); .byte 0x0f; .byte 0xae; .byte 0xd0+__ASM_REGOFF_2(reg);
#define ASM_WRGSBASE(reg) __BEGIN_FSGSBASE_INSTRUCTION .byte 0xf3; .byte 0x48+__ASM_REGOFF_1(reg); .byte 0x0f; .byte 0xae; .byte 0xd8+__ASM_REGOFF_2(reg);
#else
/* This doesn't really work yet... */
#define ASM_RDFSBASE(reg) .byte 0xf3; .byte 0x48+__ASM_REGOFF_1(reg); .byte 0x0f; .byte 0xae; .byte 0xc0+__ASM_REGOFF_2(reg);
#define ASM_RDGSBASE(reg) .byte 0xf3; .byte 0x48+__ASM_REGOFF_1(reg); .byte 0x0f; .byte 0xae; .byte 0xc8+__ASM_REGOFF_2(reg);
#define ASM_WRFSBASE(reg) .byte 0xf3; .byte 0x48+__ASM_REGOFF_1(reg); .byte 0x0f; .byte 0xae; .byte 0xd0+__ASM_REGOFF_2(reg);
#define ASM_WRGSBASE(reg) .byte 0xf3; .byte 0x48+__ASM_REGOFF_1(reg); .byte 0x0f; .byte 0xae; .byte 0xd8+__ASM_REGOFF_2(reg);
#endif

#ifdef __CC__
LOCAL u64 KCALL asm_rdfsbase(void) { register u64 result __ASMNAME("r10"); __asm__ __volatile__(PP_STR(ASM_RDFSBASE(r10)) : "=r" (result)); return result; }
LOCAL u64 KCALL asm_rdgsbase(void) { register u64 result __ASMNAME("r10"); __asm__ __volatile__(PP_STR(ASM_RDGSBASE(r10)) : "=r" (result)); return result; }
LOCAL void KCALL asm_wrfsbase(u64 base) { register u64 baseval __ASMNAME("r10") = base; __asm__ __volatile__(PP_STR(ASM_WRFSBASE(r10)) : : "r" (baseval)); }
LOCAL void KCALL asm_wrgsbase(u64 base) { register u64 baseval __ASMNAME("r10") = base; __asm__ __volatile__(PP_STR(ASM_WRGSBASE(r10)) : : "r" (baseval)); }
#endif /* __CC__ */

#endif /* __x86_64__ */


#define ASM_LOCK  lock;
#ifdef __x86_64__
#define ASM_IRET  iretq;
#else
#define ASM_IRET  iret;
#endif


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_ASM_H */
