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
#define _ASM_REGISTERS_H 1
#define _ASMARM_SIGCONTEXT_H 1

#include <hybrid/typecore.h>

/* DISCLAIMER: Based on `/usr/include/arm-linux-gnueabi/asm/sigcontext.h' */

/* Signal context structure - contains all info to do with the state
 * before the signal handler was invoked.  Note: only add new entries
 * to the end of the structure. */
#define __SIGCONTEXT_OFFSETOF_TRAP_NO              0
#define __SIGCONTEXT_OFFSETOF_ERROR_CODE           __SIZEOF_POINTER__
#define __SIGCONTEXT_OFFSETOF_OLDMASK           (2*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R0            (3*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R1            (4*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R2            (5*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R3            (6*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R4            (7*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R5            (8*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R6            (9*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R7            (10*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R8            (11*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R9            (12*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_R10           (13*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_FP            (14*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_IP            (15*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_SP            (16*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_LR            (17*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_PC            (18*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_CPSR          (19*__SIZEOF_POINTER__)
#define __SIGCONTEXT_OFFSETOF_ARM_FAULT_ADDRESS (20*__SIZEOF_POINTER__)
#define __SIGCONTEXT_SIZE                       (21*__SIZEOF_POINTER__)
#ifdef __CC__
struct sigcontext {
    __ULONGPTR_TYPE__ trap_no;
    __ULONGPTR_TYPE__ error_code;
    __ULONGPTR_TYPE__ oldmask;
    __ULONGPTR_TYPE__ arm_r0;
    __ULONGPTR_TYPE__ arm_r1;
    __ULONGPTR_TYPE__ arm_r2;
    __ULONGPTR_TYPE__ arm_r3;
    __ULONGPTR_TYPE__ arm_r4;
    __ULONGPTR_TYPE__ arm_r5;
    __ULONGPTR_TYPE__ arm_r6;
    __ULONGPTR_TYPE__ arm_r7;
    __ULONGPTR_TYPE__ arm_r8;
    __ULONGPTR_TYPE__ arm_r9;
    __ULONGPTR_TYPE__ arm_r10;
    __ULONGPTR_TYPE__ arm_fp;
    __ULONGPTR_TYPE__ arm_ip;
    __ULONGPTR_TYPE__ arm_sp;
    __ULONGPTR_TYPE__ arm_lr;
    __ULONGPTR_TYPE__ arm_pc;
    __ULONGPTR_TYPE__ arm_cpsr;
    __ULONGPTR_TYPE__ fault_address;
};
#endif /* __CC__ */


#endif /* !_ARM_KOS_ASM_REGISTERS_H */
