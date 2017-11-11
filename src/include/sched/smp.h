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
#ifndef GUARD_INCLUDE_KERNEL_SMP_H
#define GUARD_INCLUDE_KERNEL_SMP_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <errno.h>
#include <stdbool.h>

DECL_BEGIN

struct cpu;
struct timespec;

#ifdef CONFIG_SMP
struct hwcpu {
 ATOMIC_DATA size_t hw_onln; /*< [!0] Amount of cpus currently online. */
 size_t             hw_cpuc; /*< [const][!0] Amount of cpus the host has to offer. */
 struct cpu *const *hw_cpuv; /*< [1..1][1..hw_cpuc][INDEX(->c_id)] Vector of hardware CPU controllers.
                              *   NOTE: The boot-cpu is _ALWAYS_ located at index #0 */
};
DATDEF struct hwcpu smp_hwcpu;

#define CPU_OK(i) ((i) < smp_hwcpu.hw_cpuc)
#define CPUI(i)     smp_hwcpu.hw_cpuv[i]
#define SMP_COUNT   smp_hwcpu.hw_cpuc
#define SMP_ONLINE  smp_hwcpu.hw_onln

#define FOREACH_CPU(c) \
 for (struct cpu *const *_iter = smp_hwcpu.hw_cpuv, \
                 *const *_end = _iter+SMP_COUNT; \
      _iter != _end; ++_iter) \
 if (((c) = *_iter,0)); else

#else /* CONFIG_SMP */

#ifndef ____bootcpu_defined
#define ____bootcpu_defined 1
DATDEF struct cpu __bootcpu;
#endif /* !____bootcpu_defined */

#define CPU_OK(i)  ((i) == 0)
#define CPUI(i)    ((void)(i),&__bootcpu)
#define SMP_COUNT    1
#define SMP_ONLINE   1

#define FOREACH_CPU(cpu) \
 if (((cpu) = &__bootcpu,0)); else

#endif /* !CONFIG_SMP */


#ifdef CONFIG_SMP
/* Activate the given cpu and wait until it has started.
 * NOTE: No-op (returning -EOK) if the CPU is the caller, or is already enable.
 * NOTE: The caller must be holding a write-lock to `apic_lock'
 * @param: self:        The CPU to enable.
 * @return: -EOK:       The given CPU has been enabled.
 *             WARNING: By the time control is returned to the caller,
 *                      this may already have changed again...
 * @return: -ETIMEDOUT: Timed out when attempting to communicate with the CPU.
 * @return: -ECOMM:     Cannot communicate with the CPU.
 * @return: -EINTR:     The calling thread was interrupted. */
FUNDEF SAFE errno_t KCALL cpu_enable_unlocked(struct cpu *__restrict self);

/* Disable the given cpu, potentially waiting until it has shut down.
 * NOTE: The caller must be holding a write-lock to `apic_lock'
 * NOTE: No-op (returning -EOK) if the CPU was already deactivated.
 * @param: self:        The CPU to disable.
 * @param: mode:        Set of `CPU_DISABLE_*' describing how the CPU should be shut down.
 * @return: -EOK:       The given CPU has been disabled.
 *             WARNING: By the time control is returned to the caller,
 *                      this may already have changed again...
 * @return: -ETIMEDOUT: Timed out when attempting to communicate with the CPU.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ECOMM:     Cannot communicate with the CPU.
 * @return: -EPERM:     Cannot de-activate the boot-CPU (__bootcpu).
 *                   [!(mode&CPU_DISABLE_NOMIGRATE)] The CPU is hosting tasks that cannot be migrated.
 *                                                   Try specifying `CPU_DISABLE_NOMIGRATE' to force
 *                                                   it to shut down, leaving you responsible for
 *                                                   dealing with all of its remaining tasks.
 */
FUNDEF SAFE errno_t KCALL cpu_disable_unlocked(struct cpu *__restrict self, int mode);
#define CPU_DISABLE_SHUTDOWN  0x0000 /*< Shut down the CPU. */
#define CPU_DISABLE_ASYNC     0x0001 /*< Don't wait for the CPU to acknowledge shutdown. */
#define CPU_DISABLE_NOMIGRATE 0x0002 /*< Don't migrate running tasks. */


/* Execute an IPC interrupt #intno within the given CPU.
 * NOTE: The caller must be holding a write-lock to `apic_lock'
 * @return: -EOK:   Successfully invoked the given interrupt.
 * @return: -ECOMM: Failed to communicated with the given CPU. */
FUNDEF SAFE errno_t KCALL cpu_sendipc_unlocked(struct cpu *__restrict self, irq_t intno);



/* Shut down the current CPU _NOW_ (Not marked as ATTR_NORETURN,
 * because the caller may be re-scheduled on another CPU).
 * WARNING: Do not use these to deactivate your own CPU.
 *          Call `cpu_disable_unlocked(THIS_CPU,CPU_DISABLE_SHUTDOWN)' instead. */
FUNDEF void KCALL __cpu_shutdown_now(void);
FUNDEF void KCALL __cpu_shutdown_now_endwrite(void);


#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Initialize SMP by searching for available CPUs.
 * NOTE: This function must be called before paging was initialized! */
INTDEF INITCALL void KCALL smp_initialize(void);

/* Perform additional initialization once paging has been initialized.
 * NOTE: This function's main purpose is to relocate secondary CPU memory into
 *       the shared address space to ensure that no page directory switching
 *       would be required when scheduling or accessing per-cpu variables.
 *    -> In addition, this is where IDLE stacks are allocated for the first
 *       time, meaning that SMP CPUs can only be safely initialized _AFTER_
 *       this portion of initialization has been performed. */
INTDEF INITCALL void KCALL smp_initialize_repage(void);

/* Initialize LAPIC (if available) */
INTDEF INITCALL void KCALL smp_initialize_lapic(void);
#endif

#endif /* CONFIG_SMP */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_SMP_H */
