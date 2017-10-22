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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_CPU_H
#define GUARD_INCLUDE_KERNEL_ARCH_CPU_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/arch/tss.h>
#include <sync/sig.h>

DECL_BEGIN

/* NOTE: Only the cpu itself can set modes 'CPUMODE_OFFLINE' and 'CPUMODE_ONLINE'.
 *       Other CPUs may only set other modes. */
#define CPUMODE_OFFLINE            0 /*< [PRIVATE(THIS_CPU)] The CPU is offline and not running. */
#define CPUMODE_ONLINE             1 /*< [PRIVATE(THIS_CPU)] The CPU is currently executing tasks. */
#define CPUMODE_STARTUP            2 /*< The startup sequence has begun. */
#define CPUMODE_SHUTDOWN           3 /*< The CPU is currently shutting down. */
#define CPUMODE_SHUTDOWN_NOMIGRATE 4 /*< Same as 'CPUMODE_SHUTDOWN', but don't migrate running tasks,
                                      *  thereby never resuming execution because of local affinity. */


#define CPUSIG_SHUTDOWN_OK              0 /*< Shutdown was successful. */
#define CPUSIG_SHUTDOWN_ILLEGAL_AFFINTY 1 /*< Shutdown is illegal due to impossible task-affinity constraints. */
#ifdef __CC__
typedef int cpusig_t; /* Signals data object (sometimes) broadcast through 'ac_sigonoff' */
#endif /* __CC__ */

#define CPUFLAG_NONE  0x00
#define CPUFLAG_LAPIC 0x01 /*< The CPU is wired to & controlled by a LAPIC. */
#define CPUFLAG_486   0x02 /*< The CPU is an i486. */
#define CPUFLAG_CPUID 0x02 /*< The CPU supports the 'cpuid' instruction. */
#define CPUFLAG_IINIT 0x80 /*< Extended CPU information has been initialized (See below). */

#define ARCHCPU_OFFSETOF_TSS            0
#define ARCHCPU_OFFSETOF_SIGONOFF       TSS_SIZE
#define ARCHCPU_OFFSETOF_SPURIOUS_IRQ  (TSS_SIZE+SIG_SIZE)
#define ARCHCPU_OFFSETOF_MODE          (TSS_SIZE+SIG_SIZE+4)
#define ARCHCPU_OFFSETOF_FLAGS         (TSS_SIZE+SIG_SIZE+5)
#define ARCHCPU_OFFSETOF_LAPIC_ID      (TSS_SIZE+SIG_SIZE+6)
#define ARCHCPU_OFFSETOF_LAPIC_VERSION (TSS_SIZE+SIG_SIZE+7)
#define ARCHCPU_OFFSETOF_CPUSIG        (TSS_SIZE+SIG_SIZE+8)
#define ARCHCPU_OFFSETOF_FEATURES      (TSS_SIZE+SIG_SIZE+12)
#define ARCHCPU_OFFSETOF_CPUID_MAX     (TSS_SIZE+SIG_SIZE+16)
#define ARCHCPU_OFFSETOF_VENDORID      (TSS_SIZE+SIG_SIZE+20)
#define ARCHCPU_OFFSETOF_BRANDSTR      (TSS_SIZE+SIG_SIZE+32)
#define ARCHCPU_SIZE                   (TSS_SIZE+SIG_SIZE+80)

#ifdef __CC__
struct archcpu {
 struct tss      ac_tss;           /*< [lock(PRIVATE(THIS_CPU))] The TSS segment associated with this CPU. */
 struct sig      ac_sigonoff;      /*< Signal broadcast when the CPU starts up, or shuts down. */
 ATOMIC_DATA u32 ac_spurious_irq;  /*< Amount of spurious interrupts (IRQ_LAPIC_SPURIOUS) received. */
 u8              ac_mode;          /*< [lock(ac_sigonoff)] Current CPU mode (One of 'CPUMODE_*'). */
 u8              ac_flags;         /*< [const] Set of 'CPUFLAG_*' */
 u8              ac_lapic_id;      /*< [const][valid_if(CPUFLAG_LAPIC)] LAPIC id. */
 u8              ac_lapic_version; /*< [const][valid_if(CPUFLAG_LAPIC)] LAPIC version (One of 'APICVER_*'). */
 u32             ac_cpusig;        /*< [const] Processor Type signature. */
 u32             ac_features;      /*< [const] CPUID feature value. */
 u32             ac_cpuid_max;     /*< [valid_if(CPUFLAG_IINIT)][const] Max supported CPUid (NOTE: This is the max allowed value to pass as in EAX to the 'cpuid' instruction). */
 char            ac_vendorid[12];  /*< [valid_if(ARCHCPU_HAS_VENDORID(self))][const]
                                    *   Vendor ID (e.g.: 'GenuineIntel'; MAY NOT ZERO-TERMINATED!) */
 char            ac_brandstr[48];  /*< [valid_if(CPUFLAG_IINIT && ARCHCPU_HAS_BRANDSTR(self))][const]
                                    *   Branch string (e.g.: 'QEMU Virtual CPU version 2.5+'; MAY NOT ZERO-TERMINATED!) */
};
#define ARCHCPU_HAS_VENDORID(x) ((x).ac_flags&(CPUFLAG_IINIT&CPUFLAG_CPUID))
#define ARCHCPU_HAS_BRANDSTR(x) ((x).ac_cpuid_max >= 0x80000004)

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_CPU_H */
