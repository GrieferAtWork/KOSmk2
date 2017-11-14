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
#define __ASM_SCRATCH_OFFSETOF_R11 0
#define __ASM_SCRATCH_OFFSETOF_R10 8
#define __ASM_SCRATCH_OFFSETOF_R9  16
#define __ASM_SCRATCH_OFFSETOF_R8  24
#define __ASM_SCRATCH_OFFSETOF_RDI 32
#define __ASM_SCRATCH_OFFSETOF_RSI 40
#define __ASM_SCRATCH_OFFSETOF_RDX 48
#define __ASM_SCRATCH_OFFSETOF_RCX 56
#define __ASM_SCRATCH_OFFSETOF_RAX 64
#define __ASM_SCRATCH_SIZE         72
#define __ASM_PUSH_SCRATCH \
        pushq %rax; pushq %rcx; pushq %rdx; pushq %rsi; \
        pushq %rdi; pushq %r8;  pushq %r9;  pushq %r10; \
        pushq %r11;

#define __ASM_POP_SCRATCH_BEFORE_XDX \
        popq %r11; popq %r10; popq %r9;  popq %r8;  \
        popq %rdi; popq %rsi;
#define __ASM_POP_SCRATCH_AFTER_XDX \
        popq %rcx; popq %rax;
#define __ASM_POP_SCRATCH \
        __ASM_POP_SCRATCH_BEFORE_XDX popq %rdx; \
        __ASM_POP_SCRATCH_AFTER_XDX

#define __ASM_SCRATCH_NOXAX_OFFSETOF_R11 0
#define __ASM_SCRATCH_NOXAX_OFFSETOF_R10 8
#define __ASM_SCRATCH_NOXAX_OFFSETOF_R9  16
#define __ASM_SCRATCH_NOXAX_OFFSETOF_R8  24
#define __ASM_SCRATCH_NOXAX_OFFSETOF_RDI 32
#define __ASM_SCRATCH_NOXAX_OFFSETOF_RSI 40
#define __ASM_SCRATCH_NOXAX_OFFSETOF_RDX 48
#define __ASM_SCRATCH_NOXAX_OFFSETOF_RCX 56
#define __ASM_SCRATCH_NOXAX_SIZE         64
#define __ASM_PUSH_SCRATCH_NOXAX \
        pushq %rcx; pushq %rdx; pushq %rsi; pushq %rdi; \
        pushq %r8;  pushq %r9;  pushq %r10; pushq %r11;
#define __ASM_POP_SCRATCH_NOXAX \
        popq %r11; popq %r10; popq %r9;  popq %r8;  \
        popq %rdi; popq %rsi; popq %rdx; popq %rcx;
#define __ASM_POP_SCRATCH_NOXAX_BEFORE_XDX \
        popq %r11; popq %r10; popq %r9;  popq %r8;  \
        popq %rdi; popq %rsi;
#define __ASM_POP_SCRATCH_NOXAX_AFTER_XDX \
        popq %rcx;

/* Transform a scratch-safe in `0(%rsp)'
 * into a full `struct gpregs' structure:
 *
 *                         %r15  <- %rsp
 *                         %r14
 *                         %r13
 *                         %r12
 *                 /------ %r11
 *                / /----- %r10
 * %rsp -> %r11 -/ / /---- %r9
 *         %r10 --/ / /--- %r8
 *         %r9  ---/ / /-- %rdi
 *         %r8  ----/ / /- %rsi
 *         %rdi -----/ /   %rbp
 *         %rsi ------/    %rbx
 *         %rdx ---------- %rdx
 *         %rcx ---------- %rcx
 *         %rax ---------- %rax
 */
#define __ASM_SCRACH_XSP_TO_GPREGS(temp)  \
        subq $16, %rsp; \
        movq 16(%rsp), temp; movq temp, 0(%rsp); /* %r11 */ \
        movq 24(%rsp), temp; movq temp, 8(%rsp); /* %r10 */ \
        movq 32(%rsp), temp; movq temp, 16(%rsp); /* %r9 */ \
        movq 40(%rsp), temp; movq temp, 24(%rsp); /* %r8 */ \
        movq 48(%rsp), temp; movq temp, 32(%rsp); /* %rdi */ \
        movq 56(%rsp), temp; movq temp, 40(%rsp); /* %rsi */ \
        movq %rbp, 48(%rsp); /* %rbp */ \
        movq %rbx, 56(%rsp); /* %rbx */ \
        pushq %r12; pushq %r13; pushq %r14; pushq %r15;

#define __ASM_PRESERVE_OFFSETOF_R15 0
#define __ASM_PRESERVE_OFFSETOF_R14 8
#define __ASM_PRESERVE_OFFSETOF_R13 16
#define __ASM_PRESERVE_OFFSETOF_R12 24
#define __ASM_PRESERVE_OFFSETOF_RBP 32
#define __ASM_PRESERVE_OFFSETOF_RBX 40
#define __ASM_PRESERVE_SIZE         48
#define __ASM_PUSH_PRESERVE \
        pushq %rbx; pushq %rbp; pushq %r12; pushq %r13; \
        pushq %r14; pushq %r15;
#define __ASM_POP_PRESERVE \
        pushq %r15; pushq %r14; pushq %r13; pushq %r12; \
        pushq %rbp; pushq %rbx;
#elif defined(__OPTIMIZE_SIZE__)
#define __ASM_SCRATCH_OFFSETOF_EAX  0
#define __ASM_SCRATCH_OFFSETOF_ECX  4
#define __ASM_SCRATCH_OFFSETOF_EDX  8
#define __ASM_SCRATCH_OFFSETOF_EBX  12
#define __ASM_SCRATCH_OFFSETOF_ESP  16
#define __ASM_SCRATCH_OFFSETOF_EBP  20
#define __ASM_SCRATCH_OFFSETOF_ESI  24
#define __ASM_SCRATCH_OFFSETOF_EDI  28
#define __ASM_SCRATCH_SIZE          32
#define __ASM_PUSH_SCRATCH           pushal;
#define __ASM_POP_SCRATCH            popal;
#define __ASM_POP_SCRATCH_BEFORE_XDX popl %edi; popl %esi; popl %ebp; addl $4, %esp; popl %ebx;
#define __ASM_POP_SCRATCH_AFTER_XDX  popl %ecx; popl %eax;
/* Transform a scratch-safe in `0(%esp)' into a full `struct gpregs' structure. */
#define __ASM_SCRACH_XSP_TO_GPREGS(temp) /* nothing */
#define __ASM_SCRACH_XSP_TO_GPREGS_IS_NOOP 1

#define __ASM_SCRATCH_NOXAX_OFFSETOF_EAX 0
#define __ASM_SCRATCH_NOXAX_OFFSETOF_ECX 4
#define __ASM_SCRATCH_NOXAX_OFFSETOF_EDX 8
#define __ASM_SCRATCH_NOXAX_OFFSETOF_EBX 12
#define __ASM_SCRATCH_NOXAX_OFFSETOF_ESP 16
#define __ASM_SCRATCH_NOXAX_OFFSETOF_EBP 20
#define __ASM_SCRATCH_NOXAX_OFFSETOF_ESI 24
#define __ASM_SCRATCH_NOXAX_OFFSETOF_EDI 28
#define __ASM_SCRATCH_NOXAX_SIZE         32
#define __ASM_PUSH_SCRATCH_NOXAX pushal;
#define __ASM_POP_SCRATCH_NOXAX  popal;
#define __ASM_POP_SCRATCH_NOXAX_BEFORE_XDX popl %edi; popl %esi; popl %ebp; addl $4, %esp; popl %ebx;
#define __ASM_POP_SCRATCH_NOXAX_AFTER_XDX  popl %ecx;

#define __ASM_PRESERVE_OFFSETOF_EAX 0
#define __ASM_PRESERVE_OFFSETOF_ECX 4
#define __ASM_PRESERVE_OFFSETOF_EDX 8
#define __ASM_PRESERVE_OFFSETOF_EBX 12
#define __ASM_PRESERVE_OFFSETOF_ESP 16
#define __ASM_PRESERVE_OFFSETOF_EBP 20
#define __ASM_PRESERVE_OFFSETOF_ESI 24
#define __ASM_PRESERVE_OFFSETOF_EDI 28
#define __ASM_PRESERVE_SIZE         32
#define __ASM_PUSH_PRESERVE      pushal;
#define __ASM_POP_PRESERVE       popal;
#else
#define __ASM_SCRATCH_OFFSETOF_EDX 0
#define __ASM_SCRATCH_OFFSETOF_ECX 4
#define __ASM_SCRATCH_OFFSETOF_EAX 8
#define __ASM_SCRATCH_SIZE         12
#define __ASM_PUSH_SCRATCH           pushl %eax; pushl %ecx; pushl %edx;
#define __ASM_POP_SCRATCH            popl %edx; popl %ecx; popl %eax;
#define __ASM_POP_SCRATCH_BEFORE_XDX /* nothing */
#define __ASM_POP_SCRATCH_AFTER_XDX  popl %ecx; popl %eax;
/* Transform a scratch-safe in `0(%esp)' into a full `struct gpregs' structure:
 *
 *                         %edi  <- %rsp
 *                         %esi
 *                         %ebp
 *                         %---
 *                         %ebx
 * %rsp -> %edx ---------- %edx
 *         %ecx ---------- %ecx
 *         %eax ---------- %eax
 */
#define __ASM_SCRACH_XSP_TO_GPREGS(temp) \
        pushl %ebx; pushl %esp; pushl %ebp; \
        pushl %esi; pushl %edi;
#define __ASM_SCRATCH_NOXAX_OFFSETOF_EDX 0
#define __ASM_SCRATCH_NOXAX_OFFSETOF_ECX 4
#define __ASM_SCRATCH_NOXAX_SIZE         8
#define __ASM_PUSH_SCRATCH_NOXAX pushl %ecx; pushl %edx;
#define __ASM_POP_SCRATCH_NOXAX  popl %edx; popl %ecx;
#define __ASM_POP_SCRATCH_NOXAX_BEFORE_XDX /* nothing */
#define __ASM_POP_SCRATCH_NOXAX_AFTER_XDX  popl %ecx;

#define __ASM_PRESERVE_OFFSETOF_ESI 0
#define __ASM_PRESERVE_OFFSETOF_EDI 4
#define __ASM_PRESERVE_OFFSETOF_EBP 8
#define __ASM_PRESERVE_OFFSETOF_EBX 12
#define __ASM_PRESERVE_SIZE         16
#define __ASM_PUSH_PRESERVE         pushl %ebx; pushl %ebp; pushl %edi; pushl %esi;
#define __ASM_POP_PRESERVE          popl %esi; popl %edi; popl %ebp; popl %ebx;
#endif

#endif /* !_X86_KOS_ASM_REGISTERS_H */
