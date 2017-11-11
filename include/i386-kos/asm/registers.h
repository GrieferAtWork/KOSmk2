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
#ifndef _X86_KOS_ASM_REGISTERS_H
#define _X86_KOS_ASM_REGISTERS_H 1

#include <hybrid/host.h>

#ifdef __x86_64__
#define __ASM_SCRATCH_SIZE  72
#define __ASM_PERSERVE_SIZE 48
#define __ASM_PUSH_SCRATCH \
        pushq %rax; pushq %rcx; pushq %rdx; pushq %rsi; \
        pushq %rdi; pushq %r8;  pushq %r9;  pushq %r10; \
        pushq %r11;
#define __ASM_POP_SCRATCH \
        popq %r11; popq %r10; popq %r9;  popq %r8;  \
        popq %rdi; popq %rsi; popq %rdx; popq %rcx; \
        popq %rax;
#define __ASM_PUSH_PRESERVE \
        pushq %rbx; pushq %rbp; pushq %r12; pushq %r13; \
        pushq %r14; pushq %r15;
#define __ASM_POP_PRESERVE \
        pushq %r15; pushq %r14; pushq %r13; pushq %r12; \
        pushq %rbp; pushq %rbx;
#elif defined(__OPTIMIZE_SIZE__)
#define __ASM_SCRATCH_SIZE  32
#define __ASM_PERSERVE_SIZE 32
#define __ASM_PUSH_SCRATCH  pushal;
#define __ASM_POP_SCRATCH   popal;
#define __ASM_PUSH_PRESERVE pushal;
#define __ASM_POP_PRESERVE  popal;
#else
#define __ASM_SCRATCH_SIZE  12
#define __ASM_PERSERVE_SIZE 16
#define __ASM_PUSH_SCRATCH  pushl %eax; pushl %ecx; pushl %edx;
#define __ASM_POP_SCRATCH   popl %edx; popl %ecx; popl %eax;
#define __ASM_PUSH_PRESERVE pushl %ebx; pushl %ebp; pushl %edi; pushl %esi;
#define __ASM_POP_PRESERVE  popl %esi; popl %edi; popl %ebp; popl %ebx;
#endif

#endif /* !_X86_KOS_ASM_REGISTERS_H */
