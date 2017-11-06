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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_CPUSTATE_H
#define GUARD_INCLUDE_KERNEL_ARCH_CPUSTATE_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>

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
#define IRET_SEGMENT(x)            union PACKED { struct PACKED { u16 x##16,__##x##16hi; }; struct PACKED { u32 x##32,__##x##32hi; }; u64 x; }
#else
#define __COMMON_REG2_EX(prefix,n) __COMMON_REG2_EX_32(prefix,n)
#define __COMMON_REG1_EX(prefix,n) __COMMON_REG1_EX_32(prefix,n)
#define __COMMON_REG2(n)           __COMMON_REG2_32(n)
#define __COMMON_REG1(n)           __COMMON_REG1_32(n)
#define IRET_SEGMENT(x)            union PACKED { struct PACKED { u16 x##16,__##x##16hi; }; u32 x##32; u32 x; }
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
    __COMMON_REG2(sp)
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
#define __ASM_PUSH_GPREGS \
    pushq %rax; pushq %rcx; pushq %rdx; pushq %rbx; \
                pushq %rbp; pushq %rsi; pushq %rdi; \
    pushq %r8;  pushq %r9;  pushq %r10; pushq %r11; \
    pushq %r12; pushq %r13; pushq %r14; pushq %r15;
#define __ASM_LOAD_GPREGS(src) \
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
    movq GPREGS_OFFSETOF_RCX+src, %rcx; \
    movq GPREGS_OFFSETOF_RAX+src, %rax;
#define __ASM_POP_GPREGS \
    popq %r15; popq %r14; popq %r13; popq %r12; \
    popq %r11; popq %r10; popq %r9;  popq %r8;  \
    popq %rdi; popq %rsi; popq %rbp;            \
    popq %rbx; popq %rdx; popq %rcx; popq %rax;
#define __ASM_IPUSH_GPREGS \
    pushq %%rax; pushq %%rcx; pushq %%rdx; pushq %%rbx; \
                 pushq %%rbp; pushq %%rsi; pushq %%rdi; \
    pushq %%r8;  pushq %%r9;  pushq %%r10; pushq %%r11; \
    pushq %%r12; pushq %%r13; pushq %%r14; pushq %%r15;
#define __ASM_ILOAD_GPREGS(src) \
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
    movq GPREGS_OFFSETOF_RCX+src, %%rcx; \
    movq GPREGS_OFFSETOF_RAX+src, %%rax;
#define __ASM_IPOP_GPREGS \
    popq %%r15; popq %%r14; popq %%r13; popq %%r12; \
    popq %%r11; popq %%r10; popq %%r9;  popq %%r8;  \
    popq %%rdi; popq %%rsi; popq %%rbp;            \
    popq %%rbx; popq %%rdx; popq %%rcx; popq %%rax;
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
#define SGREGS_OFFSETOF_GS 0
#define SGREGS_OFFSETOF_FS 8
#define SGREGS_SIZE        16
#ifdef __CC__
struct PACKED sgregs { u64 gs,fs; }; /* Use 64-bit types to preserve 8-byte alignment. (There is no `pushl %fs'. - Else we could use that) */
#endif /* __CC__ */
#define __ASM_PUSH_SGREGS       pushq %fs; pushq %gs;
#define __ASM_LOAD_SGREGS(src)  movw SGREGS_OFFSETOF_FS+src, %gs; \
                                movw SGREGS_OFFSETOF_GS+src, %fs;
#define __ASM_POP_SGREGS        popq %gs;  popq %fs;
#define __ASM_IPUSH_SGREGS      pushq %%fs; pushq %%gs;
#define __ASM_ILOAD_SGREGS(src) movw SGREGS_OFFSETOF_FS+src, %%gs; \
                                movw SGREGS_OFFSETOF_GS+src, %%fs;
#define __ASM_IPOP_SGREGS       popq %%gs;  popq %%fs;
#else
#define SGREGS_OFFSETOF_GS 0
#define SGREGS_OFFSETOF_FS 2
#define SGREGS_OFFSETOF_ES 4
#define SGREGS_OFFSETOF_DS 6
#define SGREGS_SIZE        8
#ifdef __CC__
struct PACKED sgregs { u16 gs,fs,es,ds; };
#endif /* __CC__ */
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
#define IRREGS_HOST_OFFSETOF_IP         0
#define IRREGS_HOST_OFFSETOF_CS         8
#define IRREGS_HOST_OFFSETOF_FLAGS      16
#define IRREGS_HOST_SIZE                24
#define IRREGS_OFFSETOF_HOST            0 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_OFFSETOF_IP              0
#define IRREGS_OFFSETOF_CS              8
#define IRREGS_OFFSETOF_FLAGS           16
#define IRREGS_OFFSETOF_USERSP          24
#define IRREGS_OFFSETOF_SS              32
#define IRREGS_SIZE                     40
#define IRREGS_HOST_E_OFFSETOF_ECX_CODE 0
#define IRREGS_HOST_E_OFFSETOF_TAIL     8 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_HOST_E_OFFSETOF_IP       8
#define IRREGS_HOST_E_OFFSETOF_CS       16
#define IRREGS_HOST_E_OFFSETOF_FLAGS    24
#define IRREGS_HOST_E_SIZE              32
#define IRREGS_E_OFFSETOF_HOST          0 /* + IRREGS_HOST_E_OFFSETOF_* */
#define IRREGS_E_OFFSETOF_EXC_CODE      0
#define IRREGS_E_OFFSETOF_TAIL          8 /* + IRREGS_OFFSETOF_* */
#define IRREGS_E_OFFSETOF_IP            8
#define IRREGS_E_OFFSETOF_CS            16
#define IRREGS_E_OFFSETOF_FLAGS         24
#define IRREGS_E_OFFSETOF_USERSP        32
#define IRREGS_E_OFFSETOF_SS            40
#define IRREGS_E_SIZE                   48
#else
#define IRREGS_HOST_OFFSETOF_IP         0
#define IRREGS_HOST_OFFSETOF_CS         4
#define IRREGS_HOST_OFFSETOF_FLAGS      8
#define IRREGS_HOST_SIZE                12
#define IRREGS_OFFSETOF_HOST            0 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_OFFSETOF_IP              0
#define IRREGS_OFFSETOF_CS              4
#define IRREGS_OFFSETOF_FLAGS           8
#define IRREGS_OFFSETOF_USERSP          12
#define IRREGS_OFFSETOF_SS              16
#define IRREGS_SIZE                     20
#define IRREGS_HOST_E_OFFSETOF_ECX_CODE 0
#define IRREGS_HOST_E_OFFSETOF_TAIL     4 /* + IRREGS_HOST_OFFSETOF_* */
#define IRREGS_HOST_E_OFFSETOF_IP       4
#define IRREGS_HOST_E_OFFSETOF_CS       8
#define IRREGS_HOST_E_OFFSETOF_FLAGS    12
#define IRREGS_HOST_E_SIZE              16
#define IRREGS_E_OFFSETOF_HOST          0 /* + IRREGS_HOST_E_OFFSETOF_* */
#define IRREGS_E_OFFSETOF_EXC_CODE      0
#define IRREGS_E_OFFSETOF_TAIL          4 /* + IRREGS_OFFSETOF_* */
#define IRREGS_E_OFFSETOF_IP            4
#define IRREGS_E_OFFSETOF_CS            8
#define IRREGS_E_OFFSETOF_FLAGS         12
#define IRREGS_E_OFFSETOF_USERSP        16
#define IRREGS_E_OFFSETOF_SS            20
#define IRREGS_E_SIZE                   24
#endif

#ifdef __CC__
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
#endif /* __CC__ */

#ifdef __x86_64__
#define __ASM_IRET   iretq
#else
#define __ASM_IRET   iret
#endif


#define CPUSTATE16_OFFSETOF_GP     0 /* +GPREGS_OFFSETOF_* */
#define CPUSTATE16_OFFSETOF_SG     GPREGS32_SIZE /* +SGREGS_OFFSETOF_* */
#define CPUSTATE16_OFFSETOF_SS    (GPREGS32_SIZE+SGREGS32_SIZE)
#define CPUSTATE16_OFFSETOF_FLAGS (GPREGS32_SIZE+SGREGS32_SIZE+2)
#define CPUSTATE16_SIZE           (GPREGS32_SIZE+SGREGS32_SIZE+6)
#ifdef __CC__
/* CPU state for 16-bit realmode interrupts. */
struct PACKED cpustate16 {
    struct gpregs32 gp;
    struct sgregs32 sg;
    u16             ss;
    __COMMON_REG2_32(flags);
};
#endif /* __CC__ */


#define COMREGS_OFFSETOF_GP  0 /* +GPREGS_OFFSETOF_* */
#define COMREGS_OFFSETOF_SG  GPREGS_SIZE /* +SGREGS_OFFSETOF_* */
#define COMREGS_SIZE        (GPREGS_SIZE+SGREGS_SIZE)
#ifdef __CC__
/* Common CPU state. */
struct PACKED comregs {
    struct gpregs gp;
    struct sgregs sg;
};
#endif /* __CC__ */

#define COMREGS32_OFFSETOF_GP  0 /* +GPREGS_OFFSETOF_* */
#define COMREGS32_OFFSETOF_SG  GPREGS32_SIZE /* +SGREGS_OFFSETOF_* */
#define COMREGS32_SIZE        (GPREGS32_SIZE+SGREGS32_SIZE)
#ifdef __CC__
struct PACKED comregs32 {
    struct gpregs32 gp;
    struct sgregs32 sg;
};
#endif /* __CC__ */


#define CPUSTATE_HOST_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_HOST_OFFSETOF_GP   0 /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_HOST_OFFSETOF_SG   GPREGS_SIZE /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_HOST_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_OFFSETOF_* */
#define CPUSTATE_HOST_SIZE         (COMREGS_SIZE+IRREGS_HOST_SIZE)
#ifdef __CC__
struct PACKED cpustate_host {
union PACKED {
    struct comregs com;
struct PACKED {
    struct gpregs gp;
    struct sgregs sg;
};};
    struct irregs_host iret;
};
#endif /* __CC__ */

#define CPUSTATE_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_OFFSETOF_GP   0 /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_OFFSETOF_SG   GPREGS_SIZE /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_OFFSETOF_* */
#define CPUSTATE_SIZE         (COMREGS_SIZE+IRREGS_SIZE)
#ifdef __CC__
struct PACKED cpustate {union PACKED {
    struct cpustate_host host; /* host CPU state. */
struct PACKED {
union PACKED {
    struct comregs com;
struct PACKED {
    struct gpregs gp;
    struct sgregs sg;
};};
    struct irregs iret;
};};};
#endif /* __CC__ */


#define CPUSTATE_HOST_E_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_HOST_E_OFFSETOF_GP   0 /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_HOST_E_OFFSETOF_SG   GPREGS_SIZE /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_HOST_E_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_HOST_E_OFFSETOF_* */
#define CPUSTATE_HOST_E_SIZE         (COMREGS_SIZE+IRREGS_HOST_E_SIZE)
#ifdef __CC__
struct PACKED cpustate_host_e {
union PACKED {
    struct comregs com;
struct PACKED {
    struct gpregs gp;
    struct sgregs sg;
};};
    struct irregs_host_e iret;
};
#endif /* __CC__ */

#define CPUSTATE_E_OFFSETOF_HOST 0 /* +CPUSTATE_HOST_E_OFFSETOF_* */
#define CPUSTATE_E_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_E_OFFSETOF_GP   0 /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_E_OFFSETOF_SG   GPREGS_SIZE /* +SGREGS_OFFSETOF_* */
#define CPUSTATE_E_OFFSETOF_IRET COMREGS_SIZE /* +IRREGS_E_OFFSETOF_* */
#define CPUSTATE_E_SIZE         (COMREGS_SIZE+IRREGS_E_SIZE)
#ifdef __CC__
struct PACKED cpustate_e {union PACKED {
 struct cpustate_host_e host; /* host CPU state. */
struct PACKED {
union PACKED {
 struct comregs  com;
struct PACKED {
 struct gpregs   gp;
 struct sgregs   sg;
};};
 struct irregs_e iret;
};};};

#define CPUSTATE_TO_CPUSTATE_E(cs,cse,ecode) \
 ((cse).com = (cs).com, \
  (cse).iret.tail = (cs).iret, \
  (cse).iret.exc_code = (ecode))
#define CPUSTATE_E_TO_CPUSTATE(cse,cs) \
 ((cs).com = (cse).com, \
  (cs).iret.tail = (cse).iret)
#endif /* __CC__ */

#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_CPUSTATE_H */
