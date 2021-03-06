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
#ifndef GUARD_ARCH_I386_KOS_INCLUDE_ARCH_CPUSTATE_H
#define GUARD_ARCH_I386_KOS_INCLUDE_ARCH_CPUSTATE_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>
#include <arch/asm.h>

DECL_BEGIN

#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(push,1)
#endif

#define __COMMON_REG2_EX_32(prefix,n) union PACKED { u32 prefix##x##n; u32 prefix##e##n; u16 prefix##n; };
#define __COMMON_REG1_EX_32(prefix,n) union PACKED { u32 prefix##x##n##x; u32 prefix##e##n##x; u16 prefix##n##x; struct PACKED { u8 prefix##n##l,prefix##n##h; }; };
#ifdef __INTELLISENSE__
#define __COMMON_REG2_32(n)           union PACKED { u32 x##n; u32 e##n; u16 n; };
#define __COMMON_REG1_32(n)           union PACKED { u32 x##n##x; u32 e##n##x; u16 n##x; struct PACKED { u8 n##l,n##h; }; };
#else
#define __COMMON_REG2_32(n)           union PACKED { u32 e##n; u16 n; };
#define __COMMON_REG1_32(n)           union PACKED { u32 e##n##x; u16 n##x; struct PACKED { u8 n##l,n##h; }; };
#endif
#define __SEGMENT32(x)             union PACKED { struct PACKED { u16 x##16,__##x##16hi; }; u32 x##32; u32 x; }
#ifdef __x86_64__
#define __COMMON_REG3_EX(prefix,n) union PACKED { u64 prefix##x##n; u64 prefix##r##n; };
#define __COMMON_REG2_EX(prefix,n) union PACKED { u64 prefix##x##n; u64 prefix##r##n; u32 prefix##e##n; u16 prefix##n; };
#define __COMMON_REG1_EX(prefix,n) union PACKED { u64 prefix##x##n##x; u64 prefix##r##n##x; u32 prefix##e##n##x; u16 prefix##n##x; struct PACKED { u8 prefix##n##l,prefix##n##h; }; };
#ifdef __INTELLISENSE__
#define __COMMON_REG3(n)           union PACKED { u64 x##n; u64 r##n; };
#define __COMMON_REG2(n)           union PACKED { u64 x##n; u64 r##n; u32 e##n; u16 n; };
#define __COMMON_REG1(n)           union PACKED { u64 x##n##x; u64 r##n##x; u32 e##n##x; u16 n##x; struct PACKED { u8 n##l,n##h; }; };
#else
#define __COMMON_REG3(n)           union PACKED { u64 r##n; };
#define __COMMON_REG2(n)           union PACKED { u64 r##n; u32 e##n; u16 n; };
#define __COMMON_REG1(n)           union PACKED { u64 r##n##x; u32 e##n##x; u16 n##x; struct PACKED { u8 n##l,n##h; }; };
#endif
#define __SEGMENT64(x)             union PACKED { struct PACKED { u16 x##16,__##x##16hi; }; struct PACKED { u32 x##32,__##x##32hi; }; u64 x; }
#define IRET_SEGMENT               __SEGMENT64
#else
#define __COMMON_REG2_EX(prefix,n) __COMMON_REG2_EX_32(prefix,n)
#define __COMMON_REG1_EX(prefix,n) __COMMON_REG1_EX_32(prefix,n)
#define __COMMON_REG2(n)           __COMMON_REG2_32(n)
#define __COMMON_REG1(n)           __COMMON_REG1_32(n)
#define IRET_SEGMENT               __SEGMENT32
#endif


/* Register prefix in strings. */
#define REGISTER_PREFIX32 "E"
#define REGISTER_PREFIX16 ""
#ifdef __x86_64__
#define REGISTER_PREFIX64 "R"
#define REGISTER_PREFIX   REGISTER_PREFIX64
#else
#define REGISTER_PREFIX   REGISTER_PREFIX32
#endif

/* General purpose registers. */
/* HINT: Use `__ASM_PUSH_GPREGS' / `__ASM_POP_GPREGS' to push/pop this structure. */

#define GPREGS32_OFFSETOF_EDI  0
#define GPREGS32_OFFSETOF_ESI  4
#define GPREGS32_OFFSETOF_EBP  8
#define GPREGS32_OFFSETOF_ESP  12
#define GPREGS32_OFFSETOF_EBX  16
#define GPREGS32_OFFSETOF_EDX  20
#define GPREGS32_OFFSETOF_ECX  24
#define GPREGS32_OFFSETOF_EAX  28
#define GPREGS32_OFFSETOF_DI   GPREGS32_OFFSETOF_EDI
#define GPREGS32_OFFSETOF_SI   GPREGS32_OFFSETOF_ESI
#define GPREGS32_OFFSETOF_BP   GPREGS32_OFFSETOF_EBP
#define GPREGS32_OFFSETOF_SP   GPREGS32_OFFSETOF_ESP
#define GPREGS32_OFFSETOF_BX   GPREGS32_OFFSETOF_EBX
#define GPREGS32_OFFSETOF_DX   GPREGS32_OFFSETOF_EDX
#define GPREGS32_OFFSETOF_CX   GPREGS32_OFFSETOF_ECX
#define GPREGS32_OFFSETOF_AX   GPREGS32_OFFSETOF_EAX
#define GPREGS32_OFFSETOF_BL   GPREGS32_OFFSETOF_EBX
#define GPREGS32_OFFSETOF_BH  (GPREGS32_OFFSETOF_EBX+1)
#define GPREGS32_OFFSETOF_DL   GPREGS32_OFFSETOF_EDX
#define GPREGS32_OFFSETOF_DH  (GPREGS32_OFFSETOF_EDX+1)
#define GPREGS32_OFFSETOF_CL   GPREGS32_OFFSETOF_ECX
#define GPREGS32_OFFSETOF_CH  (GPREGS32_OFFSETOF_ECX+1)
#define GPREGS32_OFFSETOF_AL   GPREGS32_OFFSETOF_EAX
#define GPREGS32_OFFSETOF_AH  (GPREGS32_OFFSETOF_EAX+1)
#define GPREGS32_SIZE          32

#ifdef __x86_64__
#define GPREGS_OFFSETOF_R15    0
#define GPREGS_OFFSETOF_R14    8
#define GPREGS_OFFSETOF_R13    16
#define GPREGS_OFFSETOF_R12    24
#define GPREGS_OFFSETOF_R11    32
#define GPREGS_OFFSETOF_R10    40
#define GPREGS_OFFSETOF_R9     48
#define GPREGS_OFFSETOF_R8     56
#define GPREGS_OFFSETOF_RDI    64
#define GPREGS_OFFSETOF_RSI    72
#define GPREGS_OFFSETOF_RBP    80
#define GPREGS_OFFSETOF_RBX    88
#define GPREGS_OFFSETOF_RDX    96
#define GPREGS_OFFSETOF_RCX    104
#define GPREGS_OFFSETOF_RAX    112
#define GPREGS_OFFSETOF_XDI    GPREGS_OFFSETOF_RDI
#define GPREGS_OFFSETOF_XSI    GPREGS_OFFSETOF_RSI
#define GPREGS_OFFSETOF_XBP    GPREGS_OFFSETOF_RBP
#define GPREGS_OFFSETOF_XBX    GPREGS_OFFSETOF_RBX
#define GPREGS_OFFSETOF_XDX    GPREGS_OFFSETOF_RDX
#define GPREGS_OFFSETOF_XCX    GPREGS_OFFSETOF_RCX
#define GPREGS_OFFSETOF_XAX    GPREGS_OFFSETOF_RAX
#define GPREGS_OFFSETOF_EDI    GPREGS_OFFSETOF_RDI
#define GPREGS_OFFSETOF_ESI    GPREGS_OFFSETOF_RSI
#define GPREGS_OFFSETOF_EBP    GPREGS_OFFSETOF_RBP
#define GPREGS_OFFSETOF_EBX    GPREGS_OFFSETOF_RBX
#define GPREGS_OFFSETOF_EDX    GPREGS_OFFSETOF_RDX
#define GPREGS_OFFSETOF_ECX    GPREGS_OFFSETOF_RCX
#define GPREGS_OFFSETOF_EAX    GPREGS_OFFSETOF_RAX
#define GPREGS_OFFSETOF_BX     GPREGS_OFFSETOF_RBX
#define GPREGS_OFFSETOF_DX     GPREGS_OFFSETOF_RDX
#define GPREGS_OFFSETOF_CX     GPREGS_OFFSETOF_RCX
#define GPREGS_OFFSETOF_AX     GPREGS_OFFSETOF_RAX
#define GPREGS_OFFSETOF_BL     GPREGS_OFFSETOF_RBX
#define GPREGS_OFFSETOF_BH    (GPREGS_OFFSETOF_RBX+1)
#define GPREGS_OFFSETOF_DL     GPREGS_OFFSETOF_RDX
#define GPREGS_OFFSETOF_DH    (GPREGS_OFFSETOF_RDX+1)
#define GPREGS_OFFSETOF_CL     GPREGS_OFFSETOF_RCX
#define GPREGS_OFFSETOF_CH    (GPREGS_OFFSETOF_RCX+1)
#define GPREGS_OFFSETOF_AL     GPREGS_OFFSETOF_RAX
#define GPREGS_OFFSETOF_AH    (GPREGS_OFFSETOF_RAX+1)
#define GPREGS_SIZE            120
#else
#define GPREGS_OFFSETOF_XDI GPREGS32_OFFSETOF_EDI
#define GPREGS_OFFSETOF_XSI GPREGS32_OFFSETOF_ESI
#define GPREGS_OFFSETOF_XBP GPREGS32_OFFSETOF_EBP
#define GPREGS_OFFSETOF_XSP GPREGS32_OFFSETOF_ESP
#define GPREGS_OFFSETOF_XBX GPREGS32_OFFSETOF_EBX
#define GPREGS_OFFSETOF_XDX GPREGS32_OFFSETOF_EDX
#define GPREGS_OFFSETOF_XCX GPREGS32_OFFSETOF_ECX
#define GPREGS_OFFSETOF_XAX GPREGS32_OFFSETOF_EAX
#define GPREGS_OFFSETOF_EDI GPREGS32_OFFSETOF_EDI
#define GPREGS_OFFSETOF_ESI GPREGS32_OFFSETOF_ESI
#define GPREGS_OFFSETOF_EBP GPREGS32_OFFSETOF_EBP
#define GPREGS_OFFSETOF_ESP GPREGS32_OFFSETOF_ESP
#define GPREGS_OFFSETOF_EBX GPREGS32_OFFSETOF_EBX
#define GPREGS_OFFSETOF_EDX GPREGS32_OFFSETOF_EDX
#define GPREGS_OFFSETOF_ECX GPREGS32_OFFSETOF_ECX
#define GPREGS_OFFSETOF_EAX GPREGS32_OFFSETOF_EAX
#define GPREGS_OFFSETOF_DI  GPREGS32_OFFSETOF_DI
#define GPREGS_OFFSETOF_SI  GPREGS32_OFFSETOF_SI
#define GPREGS_OFFSETOF_BP  GPREGS32_OFFSETOF_BP
#define GPREGS_OFFSETOF_SP  GPREGS32_OFFSETOF_SP
#define GPREGS_OFFSETOF_BX  GPREGS32_OFFSETOF_BX
#define GPREGS_OFFSETOF_DX  GPREGS32_OFFSETOF_DX
#define GPREGS_OFFSETOF_CX  GPREGS32_OFFSETOF_CX
#define GPREGS_OFFSETOF_AX  GPREGS32_OFFSETOF_AX
#define GPREGS_OFFSETOF_BL  GPREGS32_OFFSETOF_BL
#define GPREGS_OFFSETOF_BH  GPREGS32_OFFSETOF_BH
#define GPREGS_OFFSETOF_DL  GPREGS32_OFFSETOF_DL
#define GPREGS_OFFSETOF_DH  GPREGS32_OFFSETOF_DH
#define GPREGS_OFFSETOF_CL  GPREGS32_OFFSETOF_CL
#define GPREGS_OFFSETOF_CH  GPREGS32_OFFSETOF_CH
#define GPREGS_OFFSETOF_AL  GPREGS32_OFFSETOF_AL
#define GPREGS_OFFSETOF_AH  GPREGS32_OFFSETOF_AH
#define GPREGS_SIZE         GPREGS32_SIZE
#endif


/* lvalue-access to the nth system call argument. */
#ifdef __x86_64__
/* x86_64: IN(rdi, rsi, rdx, r10, r8,  r9)  OUT(rax[,rdx]) */
#define GPREGS_SYSCALL_ARG1(x) ((x).rdi)
#define GPREGS_SYSCALL_ARG2(x) ((x).rsi)
#define GPREGS_SYSCALL_ARG3(x) ((x).rdx)
#define GPREGS_SYSCALL_ARG4(x) ((x).r10)
#define GPREGS_SYSCALL_ARG5(x) ((x).r8)
#define GPREGS_SYSCALL_ARG6(x) ((x).r9)
#define GPREGS_SYSCALL_RET1(x) ((x).rax)
#define GPREGS_SYSCALL_RET2(x) ((x).rdx) /* NOTE: Requires `CONFIG_HAVE_SYSCALL_LONGBIT' */

#define GPREGS_SYSV_ARG1(x)    ((x).rdi)
#define GPREGS_SYSV_ARG2(x)    ((x).rsi)
#define GPREGS_SYSV_ARG3(x)    ((x).rdx)
#define GPREGS_SYSV_ARG4(x)    ((x).rcx)
#define GPREGS_SYSV_ARG5(x)    ((x).r8)
#define GPREGS_SYSV_ARG6(x)    ((x).r9)
#define GPREGS_SYSV_RET1(x)    ((x).rax)
#define GPREGS_SYSV_RET2(x)    ((x).rdx)

/* Just an alias for `SYSV' as this point. */
#define GPREGS_FASTCALL_ARG1(x) ((x).rdi)
#define GPREGS_FASTCALL_ARG2(x) ((x).rsi)
#define GPREGS_FASTCALL_RET1(x) ((x).rax)
#define GPREGS_FASTCALL_RET2(x) ((x).rdx)

#else
/* i386:   IN(ebx, ecx, edx, esi, edi, ebp) OUT(eax[,edx]) */
#define GPREGS_SYSCALL_ARG1(x)  ((x).ebx)
#define GPREGS_SYSCALL_ARG2(x)  ((x).ecx)
#define GPREGS_SYSCALL_ARG3(x)  ((x).edx)
#define GPREGS_SYSCALL_ARG4(x)  ((x).esi)
#define GPREGS_SYSCALL_ARG5(x)  ((x).edi)
#define GPREGS_SYSCALL_ARG6(x)  ((x).ebp)
#define GPREGS_SYSCALL_RET1(x)  ((x).eax)
#define GPREGS_SYSCALL_RET2(x)  ((x).edx) /* NOTE: Requires `CONFIG_HAVE_SYSCALL_LONGBIT' */

#define GPREGS_FASTCALL_ARG1(x) ((x).ecx)
#define GPREGS_FASTCALL_ARG2(x) ((x).edx)
#define GPREGS_FASTCALL_RET1(x) ((x).eax)
#define GPREGS_FASTCALL_RET2(x) ((x).edx)
#define GPREGS_CDECL_RET1(x)    ((x).eax)
#define GPREGS_CDECL_RET2(x)    ((x).edx)
#endif

#ifdef __CC__
struct PACKED gpregs {
#ifdef __x86_64__
    __COMMON_REG3(15)
    __COMMON_REG3(14)
    __COMMON_REG3(13)
    __COMMON_REG3(12)
    __COMMON_REG3(11)
    __COMMON_REG3(10)
    __COMMON_REG3(9)
    __COMMON_REG3(8)
#endif
    __COMMON_REG2(di)
    __COMMON_REG2(si)
    __COMMON_REG2(bp)
#ifndef __x86_64__
    __COMMON_REG2(sp) /* Don't use this value! - Instead, take a look at how
                       * `kernel_panic_process' calculates the correct ESP. */
#endif
    __COMMON_REG1(b)
    __COMMON_REG1(d)
    __COMMON_REG1(c)
    __COMMON_REG1(a)
};
struct PACKED gpregs32 {
    __COMMON_REG2_32(di)
    __COMMON_REG2_32(si)
    __COMMON_REG2_32(bp)
    __COMMON_REG2_32(sp)
    __COMMON_REG1_32(b)
    __COMMON_REG1_32(d)
    __COMMON_REG1_32(c)
    __COMMON_REG1_32(a)
};
#endif /* __CC__ */

#define __ASM_PUSH_GPREGS32  pushal;
#define __ASM_LOAD_GPREGS32(src) \
    movl GPREGS32_OFFSETOF_EDI+src, %edi; \
    movl GPREGS32_OFFSETOF_ESI+src, %esi; \
    movl GPREGS32_OFFSETOF_EBP+src, %ebp; \
    movl GPREGS32_OFFSETOF_EBX+src, %ebx; \
    movl GPREGS32_OFFSETOF_EDX+src, %edx; \
    movl GPREGS32_OFFSETOF_ECX+src, %ecx; \
    movl GPREGS32_OFFSETOF_EAX+src, %eax;
#define __ASM_POP_GPREGS32   popal;
#define __ASM_IPUSH_GPREGS32 pushal;
#define __ASM_ILOAD_GPREGS32(src) \
    movl GPREGS32_OFFSETOF_EDI+src, %%edi; \
    movl GPREGS32_OFFSETOF_ESI+src, %%esi; \
    movl GPREGS32_OFFSETOF_EBP+src, %%ebp; \
    movl GPREGS32_OFFSETOF_EBX+src, %%ebx; \
    movl GPREGS32_OFFSETOF_EDX+src, %%edx; \
    movl GPREGS32_OFFSETOF_ECX+src, %%ecx; \
    movl GPREGS32_OFFSETOF_EAX+src, %%eax;
#define __ASM_IPOP_GPREGS32  popal;

#ifdef __x86_64__
#define __ASM_PUSH_GPREGS_NOXAX \
                pushq %rcx; pushq %rdx; pushq %rbx; \
                pushq %rbp; pushq %rsi; pushq %rdi; \
    pushq %r8;  pushq %r9;  pushq %r10; pushq %r11; \
    pushq %r12; pushq %r13; pushq %r14; pushq %r15;
#define __ASM_PUSH_GPREGS \
    pushq %rax; __ASM_PUSH_GPREGS_NOXAX
#define __ASM_LOAD_GPREGS_NOXAX(src) \
    movq GPREGS_OFFSETOF_R15+src, %r15; \
    movq GPREGS_OFFSETOF_R14+src, %r14; \
    movq GPREGS_OFFSETOF_R13+src, %r13; \
    movq GPREGS_OFFSETOF_R12+src, %r12; \
    movq GPREGS_OFFSETOF_R11+src, %r11; \
    movq GPREGS_OFFSETOF_R10+src, %r10; \
    movq GPREGS_OFFSETOF_R9+src,  %r9;  \
    movq GPREGS_OFFSETOF_R8+src,  %r8;  \
    movq GPREGS_OFFSETOF_RDI+src, %rdi; \
    movq GPREGS_OFFSETOF_RSI+src, %rsi; \
    movq GPREGS_OFFSETOF_RBP+src, %rbp; \
    movq GPREGS_OFFSETOF_RBX+src, %rbx; \
    movq GPREGS_OFFSETOF_RDX+src, %rdx; \
    movq GPREGS_OFFSETOF_RCX+src, %rcx;
#define __ASM_LOAD_GPREGS(src) \
    __ASM_LOAD_GPREGS_NOXAX(src) \
    movq GPREGS_OFFSETOF_RAX+src, %rax;
#define __ASM_POP_GPREGS_NOXAX \
    popq %r15; popq %r14; popq %r13; popq %r12; \
    popq %r11; popq %r10; popq %r9;  popq %r8;  \
    popq %rdi; popq %rsi; popq %rbp;            \
    popq %rbx; popq %rdx; popq %rcx;
#define __ASM_POP_GPREGS \
    __ASM_POP_GPREGS_NOXAX popq %rax;
#define __ASM_IPUSH_GPREGS_NOXAX \
                 pushq %%rcx; pushq %%rdx; pushq %%rbx; \
                 pushq %%rbp; pushq %%rsi; pushq %%rdi; \
    pushq %%r8;  pushq %%r9;  pushq %%r10; pushq %%r11; \
    pushq %%r12; pushq %%r13; pushq %%r14; pushq %%r15;
#define __ASM_IPUSH_GPREGS \
    pushq %%rax; __ASM_IPUSH_GPREGS_NOXAX
#define __ASM_ILOAD_GPREGS_NOXAX(src) \
    movq GPREGS_OFFSETOF_R15+src, %%r15; \
    movq GPREGS_OFFSETOF_R14+src, %%r14; \
    movq GPREGS_OFFSETOF_R13+src, %%r13; \
    movq GPREGS_OFFSETOF_R12+src, %%r12; \
    movq GPREGS_OFFSETOF_R11+src, %%r11; \
    movq GPREGS_OFFSETOF_R10+src, %%r10; \
    movq GPREGS_OFFSETOF_R9+src,  %%r9;  \
    movq GPREGS_OFFSETOF_R8+src,  %%r8;  \
    movq GPREGS_OFFSETOF_RDI+src, %%rdi; \
    movq GPREGS_OFFSETOF_RSI+src, %%rsi; \
    movq GPREGS_OFFSETOF_RBP+src, %%rbp; \
    movq GPREGS_OFFSETOF_RBX+src, %%rbx; \
    movq GPREGS_OFFSETOF_RDX+src, %%rdx; \
    movq GPREGS_OFFSETOF_RCX+src, %%rcx;
#define __ASM_ILOAD_GPREGS(src) \
    __ASM_ILOAD_GPREGS_NOXAX(src) \
    movq GPREGS_OFFSETOF_RAX+src, %%rax;
#define __ASM_IPOP_GPREGS_NOXAX \
    popq %%r15; popq %%r14; popq %%r13; popq %%r12; \
    popq %%r11; popq %%r10; popq %%r9;  popq %%r8;  \
    popq %%rdi; popq %%rsi; popq %%rbp;             \
    popq %%rbx; popq %%rdx; popq %%rcx;
#define __ASM_IPOP_GPREGS \
    __ASM_IPOP_GPREGS_NOXAX popq %%rax;
#else
#define __ASM_PUSH_GPREGS   __ASM_PUSH_GPREGS32
#define __ASM_LOAD_GPREGS   __ASM_LOAD_GPREGS32
#define __ASM_POP_GPREGS    __ASM_POP_GPREGS32
#define __ASM_IPUSH_GPREGS  __ASM_IPUSH_GPREGS32
#define __ASM_ILOAD_GPREGS  __ASM_ILOAD_GPREGS32
#define __ASM_IPOP_GPREGS   __ASM_IPOP_GPREGS32
#endif

#define SGREGS32_OFFSETOF_GS 0
#define SGREGS32_OFFSETOF_FS 2
#define SGREGS32_OFFSETOF_ES 4
#define SGREGS32_OFFSETOF_DS 6
#define SGREGS32_SIZE        8
#ifdef __CC__
struct PACKED sgregs32 { u16 gs,fs,es,ds; };
#endif /* __CC__ */

#define __ASM_PUSH_SGREGS32       pushw %ds; pushw %es; pushw %fs; pushw %gs;
#define __ASM_LOAD_SGREGS32(src)  movw  SGREGS32_OFFSETOF_GS+src, %gs; \
                                  movw  SGREGS32_OFFSETOF_FS+src, %fs; \
                                  movw  SGREGS32_OFFSETOF_ES+src, %es; \
                                  movw  SGREGS32_OFFSETOF_DS+src, %ds;
#define __ASM_POP_SGREGS32        popw  %gs; popw  %fs; popw  %es; popw  %ds;
#define __ASM_IPUSH_SGREGS32      pushw %%ds; pushw %%es; pushw %%fs; pushw %%gs;
#define __ASM_ILOAD_SGREGS32(src) movw  SGREGS32_OFFSETOF_GS+src, %%gs; \
                                  movw  SGREGS32_OFFSETOF_FS+src, %%fs; \
                                  movw  SGREGS32_OFFSETOF_ES+src, %%es; \
                                  movw  SGREGS32_OFFSETOF_DS+src, %%ds;
#define __ASM_IPOP_SGREGS32       popw  %%gs; popw  %%fs; popw  %%es; popw  %%ds;


/* Segment registers */
#ifdef __x86_64__
#define SGREGS_OFFSETOF_GS_BASE 0
#define SGREGS_OFFSETOF_FS_BASE 8
#define SGREGS_SIZE             16
#ifdef __CC__
struct PACKED sgregs { u64 gs_base; u64 fs_base; };
#endif /* __CC__ */
#define __ASM_PUSH_SGREGS       ASM_RDFSBASE(r10); pushq  %r10; ASM_RDGSBASE(r10); pushq  %r10;
#define __ASM_IPUSH_SGREGS      ASM_RDFSBASE(r10); pushq %%r10; ASM_RDGSBASE(r10); pushq %%r10;
#define __ASM_LOAD_SGREGS(src)  movq SGREGS_OFFSETOF_GS_BASE+src,  %r10; ASM_WRGSBASE(r10); \
                                movq SGREGS_OFFSETOF_FS_BASE+src,  %r10; ASM_WRFSBASE(r10);
#define __ASM_ILOAD_SGREGS(src) movq SGREGS_OFFSETOF_GS_BASE+src, %%r10; ASM_WRGSBASE(r10); \
                                movq SGREGS_OFFSETOF_FS_BASE+src, %%r10; ASM_WRFSBASE(r10);
#define __ASM_POP_SGREGS        popq  %r10; ASM_WRGSBASE(r10); popq  %r10; ASM_WRFSBASE(r10);
#define __ASM_IPOP_SGREGS       popq %%r10; ASM_WRGSBASE(r10); popq %%r10; ASM_WRFSBASE(r10);
#else
#define SGREGS_OFFSETOF_GS 0
#define SGREGS_OFFSETOF_FS 2
#define SGREGS_OFFSETOF_ES 4
#define SGREGS_OFFSETOF_DS 6
#define SGREGS_SIZE        8
#ifdef __CC__
struct PACKED sgregs { u16 gs,fs,es,ds; };
#endif /* __CC__ */
#define __ASM_LOAD_SEGMENTS(temp) \
    /* Load the proper kernel segment registers */ \
    movw  $(__USER_DS), temp; \
    movw  temp, %ds; \
    movw  temp, %es; \
    movw  temp, %gs; \
    movw  $(__KERNEL_PERCPU), temp; \
    movw  temp, %fs;
#define __ASM_PUSH_SGREGS   __ASM_PUSH_SGREGS32
#define __ASM_LOAD_SGREGS   __ASM_LOAD_SGREGS32
#define __ASM_POP_SGREGS    __ASM_POP_SGREGS32
#define __ASM_IPUSH_SGREGS  __ASM_IPUSH_SGREGS32
#define __ASM_ILOAD_SGREGS  __ASM_ILOAD_SGREGS32
#define __ASM_IPOP_SGREGS   __ASM_IPOP_SGREGS32
#endif


/* Interrupt return registers (_MUST_ be iret-compatible). */
#define IRREGS_ISUSER(x) (((x).cs&3) == 3) /* When true, must use `struct irregs' */


#ifdef __x86_64__
#define IRREGS_HOST_OFFSETOF_IP          0
#define IRREGS_HOST_OFFSETOF_CS          8
#define IRREGS_HOST_OFFSETOF_FLAGS       16
#define IRREGS_HOST_SIZE                 24
#define IRREGS_OFFSETOF_HOST             0 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_OFFSETOF_IP               0
#define IRREGS_OFFSETOF_CS               8
#define IRREGS_OFFSETOF_FLAGS            16
#define IRREGS_OFFSETOF_USERSP           24
#define IRREGS_OFFSETOF_SS               32
#define IRREGS_SIZE                      40
#define IRREGS_HOST_E_OFFSETOF_ECX_CODE  0
#define IRREGS_HOST_E_OFFSETOF_TAIL      8 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_HOST_E_OFFSETOF_IP        8
#define IRREGS_HOST_E_OFFSETOF_CS        16
#define IRREGS_HOST_E_OFFSETOF_FLAGS     24
#define IRREGS_HOST_E_SIZE               32
#define IRREGS_E_OFFSETOF_HOST           0 /* + IRREGS_HOST_E_OFFSETOF_* */
#define IRREGS_E_OFFSETOF_EXC_CODE       0
#define IRREGS_E_OFFSETOF_TAIL           8 /* + IRREGS_OFFSETOF_* */
#define IRREGS_E_OFFSETOF_IP             8
#define IRREGS_E_OFFSETOF_CS             16
#define IRREGS_E_OFFSETOF_FLAGS          24
#define IRREGS_E_OFFSETOF_USERSP         32
#define IRREGS_E_OFFSETOF_SS             40
#define IRREGS_E_SIZE                    48
#define IRREGS_HOST_I_OFFSETOF_INTNO     0
#define IRREGS_HOST_I_OFFSETOF_TAIL      8 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_HOST_I_OFFSETOF_IP        8
#define IRREGS_HOST_I_OFFSETOF_CS        16
#define IRREGS_HOST_I_OFFSETOF_FLAGS     24
#define IRREGS_HOST_I_SIZE               32
#define IRREGS_I_OFFSETOF_HOST           0 /* + IRREGS_HOST_I_OFFSETOF_* */
#define IRREGS_I_OFFSETOF_INTNO          0
#define IRREGS_I_OFFSETOF_TAIL           8 /* + IRREGS_OFFSETOF_* */
#define IRREGS_I_OFFSETOF_IP             8
#define IRREGS_I_OFFSETOF_CS             16
#define IRREGS_I_OFFSETOF_FLAGS          24
#define IRREGS_I_OFFSETOF_USERSP         32
#define IRREGS_I_OFFSETOF_SS             40
#define IRREGS_I_SIZE                    48
#define IRREGS_HOST_IE_OFFSETOF_INTNO    0
#define IRREGS_HOST_IE_OFFSETOF_BASE     8 /* + IRREGS_HOST_E_OFFSETOF_* */
#define IRREGS_HOST_IE_OFFSETOF_ECX_CODE 8
#define IRREGS_HOST_IE_OFFSETOF_TAIL     16 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_HOST_IE_OFFSETOF_IP       16
#define IRREGS_HOST_IE_OFFSETOF_CS       24
#define IRREGS_HOST_IE_OFFSETOF_FLAGS    32
#define IRREGS_HOST_IE_SIZE              40
#define IRREGS_IE_OFFSETOF_HOST          0 /* + IRREGS_HOST_IE_OFFSETOF_* */
#define IRREGS_IE_OFFSETOF_INTNO         0
#define IRREGS_IE_OFFSETOF_BASE          8 /* + IRREGS_E_OFFSETOF_* */
#define IRREGS_IE_OFFSETOF_EXC_CODE      8
#define IRREGS_IE_OFFSETOF_TAIL          16 /* + IRREGS_OFFSETOF_* */
#define IRREGS_IE_OFFSETOF_IP            16
#define IRREGS_IE_OFFSETOF_CS            24
#define IRREGS_IE_OFFSETOF_FLAGS         32
#define IRREGS_IE_OFFSETOF_USERSP        40
#define IRREGS_IE_OFFSETOF_SS            48
#define IRREGS_IE_SIZE                   56
#else
#define IRREGS_HOST_OFFSETOF_IP          0
#define IRREGS_HOST_OFFSETOF_CS          4
#define IRREGS_HOST_OFFSETOF_FLAGS       8
#define IRREGS_HOST_SIZE                 12
#define IRREGS_OFFSETOF_HOST             0 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_OFFSETOF_IP               0
#define IRREGS_OFFSETOF_CS               4
#define IRREGS_OFFSETOF_FLAGS            8
#define IRREGS_OFFSETOF_USERSP           12
#define IRREGS_OFFSETOF_SS               16
#define IRREGS_SIZE                      20
#define IRREGS_HOST_E_OFFSETOF_ECX_CODE  0
#define IRREGS_HOST_E_OFFSETOF_TAIL      4 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_HOST_E_OFFSETOF_IP        4
#define IRREGS_HOST_E_OFFSETOF_CS        8
#define IRREGS_HOST_E_OFFSETOF_FLAGS     12
#define IRREGS_HOST_E_SIZE               16
#define IRREGS_E_OFFSETOF_HOST           0 /* + IRREGS_HOST_E_OFFSETOF_* */
#define IRREGS_E_OFFSETOF_EXC_CODE       0
#define IRREGS_E_OFFSETOF_TAIL           4 /* + IRREGS_OFFSETOF_* */
#define IRREGS_E_OFFSETOF_IP             4
#define IRREGS_E_OFFSETOF_CS             8
#define IRREGS_E_OFFSETOF_FLAGS          12
#define IRREGS_E_OFFSETOF_USERSP         16
#define IRREGS_E_OFFSETOF_SS             20
#define IRREGS_E_SIZE                    24
#define IRREGS_HOST_I_OFFSETOF_INTNO     0
#define IRREGS_HOST_I_OFFSETOF_TAIL      4 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_HOST_I_OFFSETOF_IP        4
#define IRREGS_HOST_I_OFFSETOF_CS        8
#define IRREGS_HOST_I_OFFSETOF_FLAGS     12
#define IRREGS_HOST_I_SIZE               16
#define IRREGS_I_OFFSETOF_HOST           0 /* + IRREGS_HOST_I_OFFSETOF_* */
#define IRREGS_I_OFFSETOF_INTNO          0
#define IRREGS_I_OFFSETOF_TAIL           4 /* + IRREGS_OFFSETOF_* */
#define IRREGS_I_OFFSETOF_IP             4
#define IRREGS_I_OFFSETOF_CS             8
#define IRREGS_I_OFFSETOF_FLAGS          12
#define IRREGS_I_OFFSETOF_USERSP         16
#define IRREGS_I_OFFSETOF_SS             20
#define IRREGS_I_SIZE                    24
#define IRREGS_HOST_IE_OFFSETOF_INTNO    0
#define IRREGS_HOST_IE_OFFSETOF_BASE     4 /* + IRREGS_HOST_E_OFFSETOF_* */
#define IRREGS_HOST_IE_OFFSETOF_ECX_CODE 4
#define IRREGS_HOST_IE_OFFSETOF_TAIL     8 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_HOST_IE_OFFSETOF_IP       8
#define IRREGS_HOST_IE_OFFSETOF_CS       12
#define IRREGS_HOST_IE_OFFSETOF_FLAGS    16
#define IRREGS_HOST_IE_SIZE              20
#define IRREGS_IE_OFFSETOF_HOST          0 /* + IRREGS_HOST_IE_OFFSETOF_* */
#define IRREGS_IE_OFFSETOF_INTNO         0
#define IRREGS_IE_OFFSETOF_BASE          4 /* + IRREGS_E_OFFSETOF_* */
#define IRREGS_IE_OFFSETOF_EXC_CODE      4
#define IRREGS_IE_OFFSETOF_TAIL          8 /* + IRREGS_OFFSETOF_* */
#define IRREGS_IE_OFFSETOF_IP            8
#define IRREGS_IE_OFFSETOF_CS            12
#define IRREGS_IE_OFFSETOF_FLAGS         16
#define IRREGS_IE_OFFSETOF_USERSP        20
#define IRREGS_IE_OFFSETOF_SS            24
#define IRREGS_IE_SIZE                   28
#endif

#ifdef __CC__
/* NOTE: `intno' isn't actually the ~real~ interrupt number,
 *        but is encoded/decoded using the macros below. */

struct PACKED irregs_host     { __COMMON_REG2(ip); IRET_SEGMENT(cs); __COMMON_REG2(flags); };
struct PACKED irregs          { union PACKED { struct irregs_host host; struct PACKED {
                                __COMMON_REG2(ip); IRET_SEGMENT(cs); __COMMON_REG2(flags); };};
                                __COMMON_REG2_EX(user,sp); IRET_SEGMENT(ss); };
struct PACKED irregs_host_e   { register_t exc_code; union PACKED { struct irregs_host tail; struct PACKED {
                                __COMMON_REG2(ip); IRET_SEGMENT(cs); __COMMON_REG2(flags); };};};
struct PACKED irregs_e        { union PACKED { struct irregs_host_e host; struct PACKED {
                                register_t exc_code; union PACKED { struct irregs tail; struct PACKED {
                                __COMMON_REG2(ip); IRET_SEGMENT(cs); __COMMON_REG2(flags);
                                __COMMON_REG2_EX(user,sp); IRET_SEGMENT(ss); };};};};};
struct PACKED irregs_host_i   { register_t intno; union PACKED { struct irregs_host tail; struct PACKED {
                                __COMMON_REG2(ip); IRET_SEGMENT(cs); __COMMON_REG2(flags); };};};
struct PACKED irregs_i        { union PACKED { struct irregs_host_i host; struct PACKED {
                                register_t intno; union PACKED { struct irregs tail; struct PACKED {
                                __COMMON_REG2(ip); IRET_SEGMENT(cs); __COMMON_REG2(flags);
                                __COMMON_REG2_EX(user,sp); IRET_SEGMENT(ss); };};};};};
struct PACKED irregs_host_ie  { register_t intno; union PACKED { struct irregs_host_e base;
                                struct irregs_host __tail_noxcode; struct PACKED {
                                register_t exc_code; union PACKED { struct irregs_host tail; struct PACKED {
                                __COMMON_REG2(ip); IRET_SEGMENT(cs); __COMMON_REG2(flags); };};};};};
struct PACKED irregs_ie       { union PACKED { struct irregs_host_ie host; struct PACKED {
                                register_t intno; union PACKED { struct irregs_e base;
                                struct irregs __tail_noxcode; struct PACKED {
                                register_t exc_code; union PACKED { struct irregs tail; struct PACKED {
                                __COMMON_REG2(ip); IRET_SEGMENT(cs); __COMMON_REG2(flags);
                                __COMMON_REG2_EX(user,sp); IRET_SEGMENT(ss); };};};};};};};

#ifdef __INTELLISENSE__
#define IRREGS_INTNO  IRREGS_INTNO
#define IRREGS_TAIL   IRREGS_TAIL
/* Return the decoded interrupt number contained in the iret-descriptor. */
extern "C++" {
FUNDEF irq_t IRREGS_INTNO(struct irregs_host_i const *x);
FUNDEF irq_t IRREGS_INTNO(struct irregs_i const *x);
FUNDEF irq_t IRREGS_INTNO(struct irregs_host_ie const *x);
FUNDEF irq_t IRREGS_INTNO(struct irregs_ie const *x);
/* Return the proper tail structure of the given IRRegs structure. */
FUNDEF struct irregs *IRREGS_TAIL(struct irregs_ie *x);
FUNDEF struct irregs const *IRREGS_TAIL(struct irregs_ie const *x);
FUNDEF struct irregs_host *IRREGS_TAIL(struct irregs_host_ie *x);
FUNDEF struct irregs_host const *IRREGS_TAIL(struct irregs_host_ie const *x);
}
#else
#define IRREGS_INTNO(x) ((irq_t)IRREGS_DECODE_INTNO((x)->intno))
#define IRREGS_TAIL(x)  (INTNO_HAS_EXC_CODE(IRREGS_INTNO(x)) ? &(x)->tail : &(x)->__tail_noxcode)
#endif

/* Return true if `no' is one of `8,10,11,12,13,14,17,30' */
#define INTNO_HAS_EXC_CODE(no) ((no) < 32 && 0x40027d00&(1<<(no)))
DATDEF byte_t intno_offset[];
#endif /* __CC__ */

#define IRREGS_ENCODE_INTNO(x)     (__CCAST(uintptr_t)(intno_offset)+((x)*__SIZEOF_POINTER__))
#define IRREGS_DECODE_INTNO(x)     (((x)-__CCAST(uintptr_t)(intno_offset))/__SIZEOF_POINTER__)
#define ASM_IRREGS_ENCODE_INTNO(x) (intno_offset+((x)*__SIZEOF_POINTER__))
#define ASM_IRREGS_DECODE_INTNO(x) (((x)-intno_offset)/__SIZEOF_POINTER__)

#ifdef __x86_64__
/* TODO: Remove. - Use `ASM_IRET' from <arch/asm.h> instead. */
#define __ASM_IRET   iretq
#else
#define __ASM_IRET   iret
#endif


#define CPUSTATE16_OFFSETOF_GP     0 /* +GPREGS32_OFFSETOF_* */
#define CPUSTATE16_OFFSETOF_SG     SGREGS32_SIZE /* +SGREGS32_OFFSETOF_* */
#define CPUSTATE16_OFFSETOF_SS    (SGREGS32_SIZE+GPREGS32_SIZE)
#define CPUSTATE16_OFFSETOF_FLAGS (SGREGS32_SIZE+GPREGS32_SIZE+2)
#define CPUSTATE16_SIZE           (SGREGS32_SIZE+GPREGS32_SIZE+6)
#ifdef __CC__
/* CPU state for 16-bit realmode interrupts. */
struct PACKED cpustate16 {
    struct gpregs32 gp;
    struct sgregs32 sg;
    u16             ss;
    __COMMON_REG2_32(flags);
};
#endif /* __CC__ */


#define COMREGS_OFFSETOF_SG  0 /* +SGREGS_OFFSETOF_* */
#define COMREGS_OFFSETOF_GP  SGREGS_SIZE /* +GPREGS_OFFSETOF_* */
#define COMREGS_SIZE        (GPREGS_SIZE+SGREGS_SIZE)
#ifdef __CC__
/* Common CPU state. */
struct PACKED comregs {
    struct sgregs sg;
    struct gpregs gp;
};
#endif /* __CC__ */
#define __ASM_PUSH_COMREGS       __ASM_PUSH_GPREGS __ASM_PUSH_SGREGS
#define __ASM_LOAD_COMREGS(src)  __ASM_LOAD_SGREGS(src) __ASM_LOAD_GPREGS(src)
#define __ASM_POP_COMREGS        __ASM_POP_SGREGS __ASM_POP_GPREGS
#define __ASM_IPUSH_COMREGS      __ASM_IPUSH_GPREGS __ASM_IPUSH_SGREGS
#define __ASM_ILOAD_COMREGS(src) __ASM_ILOAD_SGREGS(src) __ASM_ILOAD_GPREGS(src)
#define __ASM_IPOP_COMREGS       __ASM_IPOP_SGREGS __ASM_IPOP_GPREGS



#define COMREGS32_OFFSETOF_SG  0 /* +SGREGS_OFFSETOF_* */
#define COMREGS32_OFFSETOF_GP  SGREGS32_SIZE /* +GPREGS_OFFSETOF_* */
#define COMREGS32_SIZE        (SGREGS32_SIZE+GPREGS32_SIZE)
#ifdef __CC__
struct PACKED comregs32 {
    struct gpregs32 gp;
    struct sgregs32 sg;
};
#endif /* __CC__ */


#define CPUSTATE_HOST_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_HOST_OFFSETOF_SG   0 /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_HOST_OFFSETOF_GP   SGREGS_SIZE /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_HOST_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_OFFSETOF_* */
#define CPUSTATE_HOST_SIZE         (COMREGS_SIZE+IRREGS_HOST_SIZE)
#ifdef __CC__
struct PACKED cpustate_host {
union PACKED {
    struct comregs com;
struct PACKED {
    struct sgregs sg;
    struct gpregs gp;
};};
    struct irregs_host iret;
};
#endif /* __CC__ */

#define CPUSTATE_OFFSETOF_HOST 0 /* +CPUSTATE_HOST_OFFSETOF_* */
#define CPUSTATE_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_OFFSETOF_SG   0 /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_OFFSETOF_GP   SGREGS_SIZE /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_OFFSETOF_* */
#define CPUSTATE_SIZE         (COMREGS_SIZE+IRREGS_SIZE)
#ifdef __CC__
struct PACKED cpustate {union PACKED {
    struct cpustate_host host; /* host CPU state. */
struct PACKED {
union PACKED {
    struct comregs com;
struct PACKED {
    struct sgregs sg;
    struct gpregs gp;
};};
    struct irregs iret;
};};};
#endif /* __CC__ */


#define CPUSTATE_HOST_E_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_HOST_E_OFFSETOF_SG   0 /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_HOST_E_OFFSETOF_GP   SGREGS_SIZE /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_HOST_E_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_HOST_E_OFFSETOF_* */
#define CPUSTATE_HOST_E_SIZE         (COMREGS_SIZE+IRREGS_HOST_E_SIZE)
#ifdef __CC__
struct PACKED cpustate_host_e {
union PACKED {
    struct comregs com;
struct PACKED {
    struct sgregs sg;
    struct gpregs gp;
};};
    struct irregs_host_e iret;
};
#endif /* __CC__ */

#define CPUSTATE_E_OFFSETOF_HOST 0 /* +CPUSTATE_HOST_E_OFFSETOF_* */
#define CPUSTATE_E_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_E_OFFSETOF_SG   0 /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_E_OFFSETOF_GP   SGREGS_SIZE /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_E_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_E_OFFSETOF_* */
#define CPUSTATE_E_SIZE         (COMREGS_SIZE+IRREGS_E_SIZE)
#ifdef __CC__
struct PACKED cpustate_e {union PACKED {
 struct cpustate_host_e host; /* host CPU state. */
struct PACKED {
union PACKED {
 struct comregs  com;
struct PACKED {
 struct sgregs   sg;
 struct gpregs   gp;
};};
 struct irregs_e iret;
};};};
#endif /* __CC__ */

#define CPUSTATE_HOST_I_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_HOST_I_OFFSETOF_SG   0 /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_HOST_I_OFFSETOF_GP   SGREGS_SIZE /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_HOST_I_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_HOST_I_OFFSETOF_* */
#define CPUSTATE_HOST_I_SIZE         (COMREGS_SIZE+IRREGS_HOST_I_SIZE)
#ifdef __CC__
struct PACKED cpustate_host_i {
union PACKED {
    struct comregs com;
struct PACKED {
    struct sgregs sg;
    struct gpregs gp;
};};
    struct irregs_host_i iret;
};
#endif /* __CC__ */

#define CPUSTATE_I_OFFSETOF_HOST 0 /* +CPUSTATE_HOST_I_OFFSETOF_* */
#define CPUSTATE_I_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_I_OFFSETOF_SG   0 /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_I_OFFSETOF_GP   SGREGS_SIZE /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_I_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_I_OFFSETOF_* */
#define CPUSTATE_I_SIZE         (COMREGS_SIZE+IRREGS_I_SIZE)
#ifdef __CC__
struct PACKED cpustate_i {union PACKED {
 struct cpustate_host_i host; /* host CPU state. */
struct PACKED {
union PACKED {
 struct comregs  com;
struct PACKED {
 struct sgregs   sg;
 struct gpregs   gp;
};};
 struct irregs_i iret;
};};};
#endif /* __CC__ */

#define CPUSTATE_HOST_IE_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_HOST_IE_OFFSETOF_SG   0 /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_HOST_IE_OFFSETOF_GP   SGREGS_SIZE /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_HOST_IE_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_HOST_IE_OFFSETOF_* */
#define CPUSTATE_HOST_IE_SIZE         (COMREGS_SIZE+IRREGS_HOST_IE_SIZE)
#ifdef __CC__
struct PACKED cpustate_host_ie {
union PACKED {
    struct comregs com;
struct PACKED {
    struct sgregs  sg;
    struct gpregs  gp;
};};
    struct irregs_host_ie iret;
};
#endif /* __CC__ */

#define CPUSTATE_IE_OFFSETOF_HOST 0 /* +CPUSTATE_HOST_IE_OFFSETOF_* */
#define CPUSTATE_IE_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_IE_OFFSETOF_SG   0 /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_IE_OFFSETOF_GP   SGREGS_SIZE /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_IE_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_IE_OFFSETOF_* */
#define CPUSTATE_IE_SIZE         (COMREGS_SIZE+IRREGS_IE_SIZE)
#ifdef __CC__
struct PACKED cpustate_ie {union PACKED {
 struct cpustate_host_ie host; /* host CPU state. */
struct PACKED {
union PACKED {
 struct comregs   com;
struct PACKED {
 struct sgregs    sg;
 struct gpregs    gp;
};};
 struct irregs_ie iret;
};};};
#endif /* __CC__ */

#ifdef __CC__
#ifdef __x86_64__
#define CPUSTATE_SP(x)  ((x)->iret.userrsp)
#define CPUSTATE_IP(x)  ((x)->iret.rip)
#else
#define CPUSTATE_SP(x)  ((x)->iret.useresp)
#define CPUSTATE_IP(x)  ((x)->iret.eip)
#endif


/* iret/cpustate conversion functions. */
#define IRREGS_TO_IRREGS_E(irr,irre,exc_code_)            (void)((irre).tail = (irr),(irre).exc_code = (exc_code_))
#define IRREGS_TO_IRREGS_I(irr,irri,intno_)               (void)((irri).tail = (irr),(irri).intno = (intno_))
#define IRREGS_TO_IRREGS_IE(irr,irrie,intno_,exc_code_)   (void)((irrie).tail = (irr),(irrie).intno = (intno_),(irrie).exc_code = (exc_code_))
#define IRREGS_E_TO_IRREGS(irre,irr)                      (void)((irr) = (irre).tail)
#define IRREGS_E_TO_IRREGS_I(irre,irri,intno_)            (void)((irri).tail = (irre).tail,(irri).intno = (intno_))
#define IRREGS_E_TO_IRREGS_IE(irre,irrie,intno_)          (void)((irrie).base = (irre),(irrie).intno = (intno_))
#define IRREGS_I_TO_IRREGS(irri,irr)                      (void)((irr) = (irri).tail)
#define IRREGS_I_TO_IRREGS_E(irri,irre,exc_code_)         (void)((irre).tail = (irri).tail,(irre).exc_code = (exc_code_))
#define IRREGS_I_TO_IRREGS_IE(irri,irrie,exc_code_)       (void)((irrie).tail = (irri).tail,(irrie).intno = (irri).intno,(irrie).exc_code = (exc_code_))
#define IRREGS_IE_TO_IRREGS(irrie,irr)                    (void)((irr) = (irrie).tail)
#define IRREGS_IE_TO_IRREGS_E(irrie,irre)                 (void)((irre) = (irrie).base)
#define IRREGS_IE_TO_IRREGS_I(irrie,irri)                 (void)((irri).tail = (irrie).tail,(irri).intno = (irrie).intno)
#define CPUSTATE_TO_CPUSTATE_E(cs,cse,exc_code_)          (void)((cse).com = (cs).com,IRREGS_TO_IRREGS_E((cs).iret,(cse).iret,exc_code_))
#define CPUSTATE_TO_CPUSTATE_I(cs,csi,intno_)             (void)((csi).com = (cs).com,IRREGS_TO_IRREGS_I((cs).iret,(csi).iret,intno_))
#define CPUSTATE_TO_CPUSTATE_IE(cs,csie,intno_,exc_code_) (void)((csie).com = (cs).com,IRREGS_TO_IRREGS_IE((cs).iret,(csie).iret,intno_,exc_code_))
#define CPUSTATE_E_TO_CPUSTATE(cse,cs)                    (void)((cs).com = (cse).com,IRREGS_E_TO_IRREGS((cse).iret,(cs).iret))
#define CPUSTATE_E_TO_CPUSTATE_I(cse,csi,intno_)          (void)((csi).com = (cse).com,IRREGS_E_TO_IRREGS_I((cse).iret,(csi).iret,intno_))
#define CPUSTATE_E_TO_CPUSTATE_IE(cse,csie,intno_)        (void)((csie).com = (cse).com,IRREGS_E_TO_IRREGS_IE((cse).iret,(csie).iret,intno_))
#define CPUSTATE_I_TO_CPUSTATE(csi,cs)                    (void)((cs).com = (csi).com,IRREGS_I_TO_IRREGS((csi).iret,(cs).iret))
#define CPUSTATE_I_TO_CPUSTATE_E(csi,cse,exc_code_)       (void)((cse).com = (csi).com,IRREGS_I_TO_IRREGS_E((csi).iret,(cse).iret,exc_code_))
#define CPUSTATE_I_TO_CPUSTATE_IE(csi,csie,exc_code_)     (void)((csie).com = (csi).com,IRREGS_I_TO_IRREGS_IE((csi).iret,(csie).iret,exc_code_))
#define CPUSTATE_IE_TO_CPUSTATE(csie,cs)                  (void)((cs).com = (csie).com,IRREGS_IE_TO_IRREGS((csie).iret,(cs).iret))
#define CPUSTATE_IE_TO_CPUSTATE_E(csie,cse)               (void)((cse).com = (csie).com,IRREGS_IE_TO_IRREGS_E((csie).iret,(cse).iret))
#define CPUSTATE_IE_TO_CPUSTATE_I(csie,csi)               (void)((csi).com = (csie).com,IRREGS_IE_TO_IRREGS_I((csie).iret,(csi).iret))
#endif /* __CC__ */

#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif

DECL_END

#endif /* !GUARD_ARCH_I386_KOS_INCLUDE_ARCH_CPUSTATE_H */
