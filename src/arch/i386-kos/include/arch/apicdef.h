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
#ifndef GUARD_ARCH_I386_KOS_INCLUDE_ARCH_APICDEF_H
#define GUARD_ARCH_I386_KOS_INCLUDE_ARCH_APICDEF_H 1

#include <hybrid/compiler.h>

DECL_BEGIN

/* Slightly modified; from linux: "/arch/x86/include/asm/apicdef.h" */
/*
 * Constants for various Intel APICs. (local APIC, IOAPIC, etc.)
 *
 * Alan Cox <Alan.Cox@linux.org>, 1995.
 * Ingo Molnar <mingo@redhat.com>, 1999, 2000
 */

#define IO_APIC_DEFAULT_PHYS_BASE     0xfec00000
#define APIC_DEFAULT_PHYS_BASE        0xfee00000

/*
 * This is the IO-APIC register space as specified
 * by Intel docs:
 */
#define IO_APIC_SLOT_SIZE             1024

#define APIC_ID                       0x20

#define APIC_LVR                      0x30
#define     APIC_LVR_MASK             0xFF00FF
#define     APIC_LVR_DIRECTED_EOI    (1 << 24)
#define     GET_APIC_VERSION(x)     ((x) & 0xFFu)
#define     GET_APIC_MAXLVT(x)     (((x) >> 16) & 0xFFu)
#if __SIZEOF_POINTER__ == 4
#  define APIC_INTEGRATED(x)        ((x) & 0xF0u)
#else
#  define APIC_INTEGRATED(x)         (1)
#endif
#define     APIC_XAPIC(x)           ((x) >= 0x14)
#define     APIC_EXT_SPACE(x)       ((x) & 0x80000000)
#define APIC_TASKPRI                  0x80
#define     APIC_TPRI_MASK            0xFFu
#define APIC_ARBPRI                   0x90
#define     APIC_ARBPRI_MASK          0xFFu
#define APIC_PROCPRI                  0xA0
#define APIC_EOI                      0xB0
#define     APIC_EOI_ACK              0x0 /* Docs say 0 for future compat. */
#define APIC_RRR                      0xC0
#define APIC_LDR                      0xD0
#define     APIC_LDR_MASK            (0xFFu << 24)
#define     GET_APIC_LOGICAL_ID(x) (((x) >> 24) & 0xFFu)
#define     SET_APIC_LOGICAL_ID(x) (((x) << 24))
#define     APIC_ALL_CPUS             0xFFu
#define APIC_DFR                      0xE0
#define     APIC_DFR_CLUSTER          0x0FFFFFFFul
#define     APIC_DFR_FLAT             0xFFFFFFFFul
#define APIC_SPIV                     0xF0
#define     APIC_SPIV_DIRECTED_EOI   (1 << 12)
#define     APIC_SPIV_FOCUS_DISABLED (1 << 9)
#define     APIC_SPIV_APIC_ENABLED   (1 << 8)
#define APIC_ISR                      0x100
#define APIC_ISR_NR                   0x8 /* Number of 32 bit ISR registers. */
#define APIC_TMR                      0x180
#define APIC_IRR                      0x200
#define APIC_ESR                      0x280
#define     APIC_ESR_SEND_CS          0x00001
#define     APIC_ESR_RECV_CS          0x00002
#define     APIC_ESR_SEND_ACC         0x00004
#define     APIC_ESR_RECV_ACC         0x00008
#define     APIC_ESR_SENDILL          0x00020
#define     APIC_ESR_RECVILL          0x00040
#define     APIC_ESR_ILLREGA          0x00080
#define APIC_LVTCMCI                  0x2f0
#define APIC_ICR                      0x300
#define     APIC_DEST_SELF            0x40000
#define     APIC_DEST_ALLINC          0x80000
#define     APIC_DEST_ALLBUT          0xC0000
#define     APIC_ICR_RR_MASK          0x30000
#define     APIC_ICR_RR_INVALID       0x00000
#define     APIC_ICR_RR_INPROG        0x10000
#define     APIC_ICR_RR_VALID         0x20000
#define     APIC_INT_LEVELTRIG        0x08000
#define     APIC_INT_ASSERT           0x04000
#define     APIC_ICR_BUSY             0x01000 /*< You never write this. */
#define     APIC_DEST_LOGICAL         0x00800
#define     APIC_DEST_PHYSICAL        0x00000
#define     APIC_DM_FIXED             0x00000
#define     APIC_DM_FIXED_MASK        0x00700
#define     APIC_DM_LOWEST            0x00100
#define     APIC_DM_SMI               0x00200
#define     APIC_DM_REMRD             0x00300
#define     APIC_DM_NMI               0x00400
#define     APIC_DM_INIT              0x00500
#define     APIC_DM_STARTUP           0x00600
#define     APIC_DM_EXTINT            0x00700
#define     APIC_VECTOR_MASK          0x000FF
#define APIC_ICR2                     0x310
#define     GET_APIC_DEST_FIELD(x) (((x) >> 24) & 0xFF)
#define     SET_APIC_DEST_FIELD(x)  ((x) << 24)
#define APIC_LVTT                     0x320
#define APIC_LVTTHMR                  0x330
#define APIC_LVTPC                    0x340
#define APIC_LVT0                     0x350
#define     APIC_LVT_TIMER_BASE_MASK (0x3 << 18)
#define     GET_APIC_TIMER_BASE(x) (((x) >> 18) & 0x3)
#define     SET_APIC_TIMER_BASE(x) (((x) << 18))
#define     APIC_TIMER_BASE_CLKIN     0x0
#define     APIC_TIMER_BASE_TMBASE    0x1
#define     APIC_TIMER_BASE_DIV       0x2
#define     APIC_LVT_TIMER_ONESHOT   (0 << 17)
#define     APIC_LVT_TIMER_PERIODIC  (1 << 17)
#define     APIC_LVT_TIMER_TSCDEADLINE (2 << 17)
#define     APIC_LVT_MASKED          (1 << 16)
#define     APIC_LVT_LEVEL_TRIGGER   (1 << 15)
#define     APIC_LVT_REMOTE_IRR      (1 << 14)
#define     APIC_INPUT_POLARITY      (1 << 13)
#define     APIC_SEND_PENDING        (1 << 12)
#define     APIC_MODE_MASK            0x700
#define     GET_APIC_DELIVERY_MODE(x) (((x) >> 8) & 0x7)
#define     SET_APIC_DELIVERY_MODE(x,y) (((x) & ~0x700) | ((y) << 8))
#define      APIC_MODE_FIXED          0x0
#define      APIC_MODE_NMI            0x4
#define      APIC_MODE_EXTINT         0x7
#define APIC_LVT1                     0x360
#define APIC_LVTERR                   0x370
#define APIC_TMICT                    0x380
#define APIC_TMCCT                    0x390
#define APIC_TDCR                     0x3E0
#define APIC_SELF_IPI                 0x3F0
#define     APIC_TDR_DIV_TMBASE      (1 << 2)
#define     APIC_TDR_DIV_1            0xB
#define     APIC_TDR_DIV_2            0x0
#define     APIC_TDR_DIV_4            0x1
#define     APIC_TDR_DIV_8            0x2
#define     APIC_TDR_DIV_16           0x3
#define     APIC_TDR_DIV_32           0x8
#define     APIC_TDR_DIV_64           0x9
#define     APIC_TDR_DIV_128          0xA
#define APIC_EFEAT                    0x400
#define APIC_ECTRL                    0x410
#define APIC_EILVTn(n)               (0x500 + 0x10 * n)
#define     APIC_EILVT_NR_AMD_K8      1 /* # of extended interrupts */
#define     APIC_EILVT_NR_AMD_10H     4
#define     APIC_EILVT_NR_MAX         APIC_EILVT_NR_AMD_10H
#define     APIC_EILVT_LVTOFF(x)   (((x) >> 4) & 0xF)
#define     APIC_EILVT_MSG_FIX        0x0
#define     APIC_EILVT_MSG_SMI        0x2
#define     APIC_EILVT_MSG_NMI        0x4
#define     APIC_EILVT_MSG_EXT        0x7
#define     APIC_EILVT_MASKED        (1 << 16)
/* end... */

DECL_END

#endif /* !GUARD_ARCH_I386_KOS_INCLUDE_ARCH_APICDEF_H */
