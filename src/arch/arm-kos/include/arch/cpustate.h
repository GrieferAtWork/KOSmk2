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
#ifndef GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_CPUSTATE_H
#define GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_CPUSTATE_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(push,1)
#endif

/* lvalue-access to the nth system call argument. */
#define GPREGS_SYSCALL_ARG1(x)  ((x).r0)
#define GPREGS_SYSCALL_ARG2(x)  ((x).r1)
#define GPREGS_SYSCALL_ARG3(x)  ((x).r2)
#define GPREGS_SYSCALL_ARG4(x)  ((x).r3)
#define GPREGS_SYSCALL_ARG5(x)  ((x).r4)
#define GPREGS_SYSCALL_ARG6(x)  ((x).r5)
#define GPREGS_SYSCALL_RET1(x)  ((x).r0)
#define GPREGS_SYSCALL_RET2(x)  ((x).r1) /* NOTE: Requires `CONFIG_HAVE_SYSCALL_LONGBIT' */

#define GPREGS_RET1(x)    ((x).r0)
#define GPREGS_RET2(x)    ((x).r1)
#define GPREGS_ARG1(x)    ((x).r0)
#define GPREGS_ARG2(x)    ((x).r1)
#define GPREGS_ARG3(x)    ((x).r2)
#define GPREGS_ARG4(x)    ((x).r3)

#define GPREGS_FASTCALL_ARG1(x) ((x).r0)
#define GPREGS_FASTCALL_ARG2(x) ((x).r1)
#define GPREGS_FASTCALL_RET1(x) ((x).r0)
#define GPREGS_FASTCALL_RET2(x) ((x).r1)


#define GPREGS_OFFSETOF_R0  0
#define GPREGS_OFFSETOF_R1  4
#define GPREGS_OFFSETOF_R2  8
#define GPREGS_OFFSETOF_R3  12
#define GPREGS_OFFSETOF_R4  16
#define GPREGS_OFFSETOF_R5  20
#define GPREGS_OFFSETOF_R6  24
#define GPREGS_OFFSETOF_R7  28
#define GPREGS_SIZE         32
/* General purpose registers (Shared by all operating modes). */
#ifdef __CC__
struct PACKED gpregs {
    register_t r0;
    register_t r1;
    register_t r2;
    register_t r3;
    register_t r5;
    register_t r6;
    register_t r7;
};
#endif
#define __ASM_PUSH_GPREGS       push {r0-r7};
#define __ASM_POP_GPREGS        pop  {r0-r7};

#define HIREGS_OFFSETOF_R8  0
#define HIREGS_OFFSETOF_R9  4
#define HIREGS_OFFSETOF_R10 8
#define HIREGS_OFFSETOF_R11 12
#define HIREGS_OFFSETOF_FP  12
#define HIREGS_OFFSETOF_R12 16
#define HIREGS_OFFSETOF_IP  16
#define HIREGS_SIZE         20
/* High-order general purpose registers (Shared with all modes, but FIQ). */
#ifdef __CC__
struct PACKED hiregs {
    register_t r8;
    register_t r9;
    register_t r10;
union PACKED { register_t r11; register_t fp; }; /* Frame pointer. */
union PACKED { register_t r12; register_t ip; }; /* Intra-procedure scratch register. */
};
#endif /* __CC__ */
#define __ASM_PUSH_HIREGS       push {r8-r12};
#define __ASM_POP_HIREGS        pop  {r8-r12};


#define XCREGS_OFFSETOF_R13  0
#define XCREGS_OFFSETOF_SP   0
#define XCREGS_OFFSETOF_R14  4
#define XCREGS_OFFSETOF_LR   4
#define XCREGS_OFFSETOF_R15  8
#define XCREGS_OFFSETOF_PC   8
#define XCREGS_SIZE         12
/* eXeCution registers (Private to each mode). */
#ifdef __CC__
struct PACKED xcregs {
union PACKED { register_t r13; register_t sp; }; /* Stack pointer. */
union PACKED { register_t r14; register_t lr; }; /* Link registers (__builtin_return_address(0))
                                                  * NOTE: Bit #0 controls ARM (0) vs. THUMB (1) execution
                                                  *       mode when loaded using `bx' or `bLx' */
union PACKED { register_t r15; register_t pc; }; /* Program counter. (Current instruction address) */
};
#endif /* __CC__ */


#define COMREGS_OFFSETOF_GP  0 /* +GPREGS_OFFSETOF_* */
#define COMREGS_OFFSETOF_HI  GPREGS_SIZE /* +HIREGS_OFFSETOF_* */
#define COMREGS_SIZE        (GPREGS_SIZE+HIREGS_SIZE)
/* Common CPU registers (General purpose + hi-order general purpose registers). */
#ifdef __CC__
struct PACKED comregs {
    struct gpregs gp; /* General purpose registers. */
    struct hiregs hi; /* High-order general purpose registers. */
};
#endif /* __CC__ */
#define __ASM_PUSH_COMREGS       push {r0-r12};
#define __ASM_POP_COMREGS        pop  {r0-r12};


#define CPUSTATE_OFFSETOF_COM  0 /* +COMREGS_OFFSETOF_* */
#define CPUSTATE_OFFSETOF_GP   0 /* +GPREGS_OFFSETOF_* */
#define CPUSTATE_OFFSETOF_HI   GPREGS_SIZE /* +HIREGS_OFFSETOF_* */
#define CPUSTATE_OFFSETOF_XC   COMREGS_SIZE /* +XCREGS_OFFSETOF_* */
#define CPUSTATE_SIZE         (COMREGS_SIZE+XCREGS_SIZE)
#ifdef __CC__
#define CPUSTATE_SP(x)  ((x)->xc.sp)
#define CPUSTATE_IP(x)  ((x)->xc.pc)
struct PACKED cpustate {
union PACKED {
    struct comregs com;
struct PACKED {
    struct gpregs  gp;
    struct hiregs  hi;
};};
    struct xcregs  xc;
};
#endif /* __CC__ */


#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif

DECL_END

#endif /* !GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_CPUSTATE_H */
