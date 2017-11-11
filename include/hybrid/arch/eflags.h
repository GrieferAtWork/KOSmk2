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
#ifndef __GUARD_HYBRID_ARCH_EFLAGS_H
#define __GUARD_HYBRID_ARCH_EFLAGS_H 1

#include <hybrid/compiler.h>

DECL_BEGIN

#define EFLAGS_CF          0x00000001 /*< [bit(0)] Carry Flag Status. */
#define EFLAGS_PF          0x00000004 /*< [bit(2)] Parity Flag Status. */
#define EFLAGS_AF          0x00000010 /*< [bit(4)] Auxiliary Carry Flag Status. */
#define EFLAGS_ZF          0x00000040 /*< [bit(6)] Zero Flag Status. */
#define EFLAGS_SF          0x00000080 /*< [bit(7)] Sign Flag Status. */
#define EFLAGS_TF          0x00000100 /*< [bit(8)] Trap Flag (System). */
#define EFLAGS_IF          0x00000200 /*< [bit(9)] Interrupt Enable Flag (System). */
#define EFLAGS_DF          0x00000400 /*< [bit(10)] Direction Flag (Control). */
#define EFLAGS_OF          0x00000800 /*< [bit(11)] Overflow Flag Status. */
#define EFLAGS_IOPL(n) (((n)&3) << 12)/*< [bit(12,13)] I/O Privilege Level (System). */
#define EFLAGS_NT          0x00004000 /*< [bit(14)] Nested Task Flag (System). */
#define EFLAGS_RF          0x00010000 /*< [bit(16)] Resume Flag (System). */
#define EFLAGS_VM          0x00020000 /*< [bit(17)] Virtual 8086 Mode (System). */
#define EFLAGS_AC          0x00040000 /*< [bit(18)] Alignment Check (System). */
#define EFLAGS_VIF         0x00080000 /*< [bit(19)] Virtual Interrupt Flag (System). */
#define EFLAGS_VIP         0x00100000 /*< [bit(20)] Virtual Interrupt Pending (System). */
#define EFLAGS_ID          0x00200000 /*< [bit(21)] ID Flag (System). */
#define EFLAGS_GTIOPL(flags) (((flags) >> 12)&3)

#define CR0_PE             0x00000001 /*< [bit(0)] Protected Mode Enable. */
#define CR0_MP             0x00000002 /*< [bit(1)] Monitor CO-Processor. */
#define CR0_EM             0x00000004 /*< [bit(2)] Emulation. */
#define CR0_TS             0x00000008 /*< [bit(3)] Task Switched. */
#define CR0_ET             0x00000010 /*< [bit(4)] Extension Type. */
#define CR0_NE             0x00000020 /*< [bit(5)] Numeric Error. */
#define CR0_WP             0x00010000 /*< [bit(16)] Write Protect. */
#define CR0_AM             0x00040000 /*< [bit(18)] Alignment Mask. */
#define CR0_NW             0x20000000 /*< [bit(29)] Not-write Through. */
#define CR0_CD             0x40000000 /*< [bit(30)] Cache Disable. */
#define CR0_PG             0x80000000 /*< [bit(31)] Paging. */

#define CR4_VME            0x00000001 /*< [bit(0)] Virtual 8086 mode extensions. */
#define CR4_PVI            0x00000002 /*< [bit(1)] Protected mode virtual interrupts. */
#define CR4_TSD            0x00000004 /*< [bit(2)] Time stamp disable. */
#define CR4_DE             0x00000008 /*< [bit(3)] Debugging extensions. */
#define CR4_PSE            0x00000010 /*< [bit(4)] Page size extension. */
#define CR4_PAE            0x00000020 /*< [bit(5)] Physical address extension. */
#define CR4_MCE            0x00000040 /*< [bit(6)] Machine check exception. */
#define CR4_PGE            0x00000080 /*< [bit(7)] Page global enable. */
#define CR4_PCE            0x00000100 /*< [bit(8)] Performance Monitoring counter enable. */
#define CR4_OSFXSR         0x00000200 /*< [bit(9)] OS support for FXSAVE and FXRSTOR Instructions. */
#define CR4_OSXMMEXCPT     0x00000400 /*< [bit(10)] OS support for unmasked SIMD floating point Exceptions. */
#define CR4_VMXE           0x00002000 /*< [bit(13)] Virtual Machine extensions enable. */
#define CR4_SMXE           0x00004000 /*< [bit(14)] Safer mode extensions enable. */
#ifdef __x86_64__
#define CR4_FSGSBASE       0x00010000 /*< [bit(16)] Enable wr(fs|gs)base instructions. */
#endif
#define CR4_PCIDE          0x00020000 /*< [bit(17)] PCID enable. */
#define CR4_OSXSAVE        0x00040000 /*< [bit(18)] XSAVE and Processor extended states enable. */
#define CR4_SMEP           0x00100000 /*< [bit(20)] Supervisor Mode executions Protection enable. */
#define CR4_SMAP           0x00200000 /*< [bit(21)] Supervisor Mode access Protection enable. */

#define DR7_L0             0x00000001 /*< [bit(0)] local DR0 breakpoint. */
#define DR7_G0             0x00000002 /*< [bit(1)] global DR0 breakpoint. */
#define DR7_L1             0x00000004 /*< [bit(2)] local DR1 breakpoint. */
#define DR7_G1             0x00000008 /*< [bit(3)] global DR1 breakpoint. */
#define DR7_L2             0x00000010 /*< [bit(4)] local DR2 breakpoint. */
#define DR7_G2             0x00000020 /*< [bit(5)] global DR2 breakpoint. */
#define DR7_L3             0x00000040 /*< [bit(6)] local DR3 breakpoint. */
#define DR7_G3             0x00000080 /*< [bit(7)] global DR3 breakpoint. */
#define DR7_C0             0x00030000 /*< [bit(16..17)] conditions for DR0. */
#define DR7_S0             0x000C0000 /*< [bit(18..19)] size of DR0 breakpoint. */
#define DR7_C1             0x00300000 /*< [bit(20..21)] conditions for DR1. */
#define DR7_S1             0x00C00000 /*< [bit(22..23)] size of DR1 breakpoint. */
#define DR7_C2             0x03000000 /*< [bit(24..25)] conditions for DR2. */
#define DR7_S2             0x0C000000 /*< [bit(26..27)] size of DR2 breakpoint. */
#define DR7_C3             0x30000000 /*< [bit(28..29)] conditions for DR3. */
#define DR7_S3             0xC0000000 /*< [bit(30..31)] size of DR3 breakpoint . */

#ifdef __x86_64__
#define IA32_FS_BASE        0xc0000100
#define IA32_GS_BASE        0xc0000101
#define IA32_KERNEL_GS_BASE 0xc0000102
#endif

DECL_END

#endif /* !__GUARD_HYBRID_ARCH_EFLAGS_H */
