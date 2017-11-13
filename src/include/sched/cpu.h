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
#ifndef GUARD_INCLUDE_SCHED_CPU_H
#define GUARD_INCLUDE_SCHED_CPU_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>
#include <sched/percpu.h>
#include <sched/types.h>
#include <stdbool.h>
#include <arch/preemption.h>

DECL_BEGIN

#ifndef cpu_relax
#define cpu_relax() (void)0
#endif

/* CPU locking control.
 * NOTE: To prevent dead-locks when a hardware-interrupt
 *       such as a key press tries to send a signal and
 *       therefor needs to acquire a write-lock to all
 *       CPUs with tasks waiting for the event, you
 *       must _always_ disable interrupts before acquiring any
 *       kind of lock to your own CPU's `c_lock' lock.
 *    >> In connection to the fact that the scheduler is
 *       allowed to change the CPU you're running on at
 *       any time, you'll probably have to always disable
 *       interrupts when accessing any kind of CPU.
 * NOTE: A similar rule applies to signal locks, but to safely
 *       use those, you can simply disable preemption before
 *       locking the signal to start receiving, and re-enabling
 *       it once it has been received.
 */
#define cpu_reading(x)      atomic_rwlock_reading(&(x)->c_lock)
#define cpu_writing(x)      atomic_rwlock_writing(&(x)->c_lock)
#if 1
#define cpu_tryread(x)     (assert(!PREEMPTION_ENABLED()),atomic_rwlock_tryread(&(x)->c_lock))
#define cpu_trywrite(x)    (assert(!PREEMPTION_ENABLED()),atomic_rwlock_trywrite(&(x)->c_lock))
#define cpu_tryupgrade(x)  (assert(!PREEMPTION_ENABLED()),atomic_rwlock_tryupgrade(&(x)->c_lock))
#define cpu_read(x)        (assert(!PREEMPTION_ENABLED()),atomic_rwlock_read(&(x)->c_lock))
#define cpu_write(x)       (assert(!PREEMPTION_ENABLED()),atomic_rwlock_write(&(x)->c_lock))
#define cpu_upgrade(x)     (assert(!PREEMPTION_ENABLED()),atomic_rwlock_upgrade(&(x)->c_lock))
#define cpu_downgrade(x)   (assert(!PREEMPTION_ENABLED()),atomic_rwlock_downgrade(&(x)->c_lock))
#define cpu_endread(x)     (assert(!PREEMPTION_ENABLED()),atomic_rwlock_endread(&(x)->c_lock))
#define cpu_endwrite(x)    (assert(!PREEMPTION_ENABLED()),atomic_rwlock_endwrite(&(x)->c_lock))
#else
#define cpu_tryread(x)      atomic_rwlock_tryread(&(x)->c_lock)
#define cpu_trywrite(x)     atomic_rwlock_trywrite(&(x)->c_lock)
#define cpu_tryupgrade(x)   atomic_rwlock_tryupgrade(&(x)->c_lock)
#define cpu_read(x)         atomic_rwlock_read(&(x)->c_lock)
#define cpu_write(x)        atomic_rwlock_write(&(x)->c_lock)
#define cpu_upgrade(x)      atomic_rwlock_upgrade(&(x)->c_lock)
#define cpu_downgrade(x)    atomic_rwlock_downgrade(&(x)->c_lock)
#define cpu_endread(x)      atomic_rwlock_endread(&(x)->c_lock)
#define cpu_endwrite(x)     atomic_rwlock_endwrite(&(x)->c_lock)
#endif

#ifdef CONFIG_SMP
/* Find the most suitable (least used) CPU allowed by the given affinity.
 * To aid in this choice, many factors are considered:
 *   - The amount/set of CPUs allowed by affinity (If only one CPU is allowed, that one is used).
 *   - The current state of different CPUs that could be used (Try not to turn on too much).
 *   - The current power configuration (How willing is the kernel to turn on multiple CPUs)
 *   - The per-cpu load average over the past few seconds/minutes/hours.
 *   - etc.
 * @return: * :   The CPU that the kernel is suggesting to use.
 * @return: NULL: No CPU matching restrictions put forth by 'affinity' found. */
FUNDEF struct cpu *KCALL cpu_get_suitable(__cpu_set_t const *__restrict affinity);
#else
#define cpu_get_suitable(affinity)       (&__bootcpu)
#endif

/* Special CPU ID reserved for the boot CPU. */
#define CPUID_BOOTCPU 0 /* NOTE: Always ZERO(0). */


#ifndef CONFIG_NO_JOBS
/* Schedule work to-be performed on the calling CPU at a later, safer time.
 * NOTE: When eventually being executed, the job will run in the context
 *       of the caller's CPU at the time of its work being scheduled.
 *       All registers will be in an undefined state, except for ESP,
 *       which points to a valid, unique stack that is used by the worker
 *       task held responsible for job completion.
 *       Additionally, the 'j_data' member of `work' is pushed onto the stack
 *       before the job function is invoked, which in return can simply
 *       pass execution to either the next scheduled job, or another task
 *       by execution a return-statement.
 * HINT: Worker tasks are per-cpu and cannot be assigned a different CPU,
 *       meaning that once scheduled, a job cannot be migrated to another CPU.
 * HINT: This function is 100% safe to be called in _ANY_ kind of context,
 *       most notable being an interrupt handler that can't safely acquire
 *       locks without expensive workarounds that disable preemption when
 *       other code accesses the same data.
 *       With that in mind, this function does momentarily disable preemption
 *       itself, yet will re-enable it so long as it was enabled when the
 *       caller passed control.
 * @return: -EOK:      The given job was scheduled.
 * @return: -EALREADY: The given job had already been scheduled, though had yet to be executed.
 * @return: -EPERM:    The module associated with the given job is currently being unloaded. */
FUNDEF errno_t KCALL schedule_work(struct job *__restrict work);

/* Schedule delayed execution of a job, given the absolute point in time when it should run.
 * HINT: Among other things, these functions are used to implement 'alarm()'.
 * NOTE: Internally, these differ from `schedule_work()' very little.
 *       Rather than scheduling the per-cpu worker thread as running, it is setup
 *       as sleeping until the lowest `abs_exectime' passes, as which point is
 *       is rescheduled just like any other thread that timed out, allowing it
 *       to then execute any that with an absolute point in time that has passed.
 * NOTE: In case the job was already scheduled (`-EALREADY' is returned),
 *       its execution time will be updated to 'MIN(*abstime,work->j_time)'
 * WARNING: Attempting to add a delay to a job with that was already scheduled
 *          using `schedule_work()' is illegal and causes undefined behavior.
 *          NOTE: This only applies to jobs until they are actually executed,
 *                meaning that a job re-scheduling itself with a delay, after
 *                being scheduled without one the first time is fully allowed.
 * @return: -EOK:      The given job was scheduled.
 * @return: -EALREADY: The given job had already been scheduled, though had yet to be executed.
 * @return: -EPERM:    The module associated with the given job is currently being unloaded. */
FUNDEF errno_t KCALL schedule_delayed_work(struct job *__restrict work, struct timespec const *__restrict abs_exectime);
FUNDEF errno_t KCALL schedule_delayed_work_j(struct job *__restrict work, jtime_t abs_exectime);
/* TODO: Using jobs, we can easily implement 'alarm()' */

// /* Schedule a job to-be executed the next time the calling CPU switches to IDLE mode.
//  * @return: -EOK:      The given job was scheduled.
//  * @return: -EALREADY: The given job had already been scheduled, though had yet to be executed.
//  * @return: -EPERM:    The module associated with the given job is currently being unloaded. */
// FUNDEF errno_t KCALL schedule_idle_work(struct job *__restrict work);
#endif /* !CONFIG_NO_JOBS */


#ifdef CONFIG_BUILDING_KERNEL_CORE
INTDEF void KCALL cpu_add_running(REF struct task *__restrict t);
INTDEF void KCALL cpu_del_running(/*OUT REF*/struct task *__restrict t);
INTDEF void KCALL cpu_add_sleeping(struct cpu *__restrict c, REF struct task *__restrict t);
INTDEF void KCALL cpu_del_sleeping(struct cpu *__restrict c, /*OUT REF*/struct task *__restrict t);
INTDEF void KCALL cpu_add_suspended(struct cpu *__restrict c, REF struct task *__restrict t);
INTDEF void KCALL cpu_del_suspended(struct cpu *__restrict c, /*OUT REF*/struct task *__restrict t);
#endif /* CONFIG_BUILDING_KERNEL_CORE */


/* Load the CPU state of the currently selected task in the calling CPU.
 * HINT: You may use 'jmp cpu_sched_setrunning' even when no valid stack is set in %ESP */
FUNDEF ATTR_NORETURN void KCALL cpu_sched_setrunning(void);

/* Same as `cpu_sched_setrunning', but saves the previous CPU state inside of `task'.
 * NOTE: To ensure that the saved CPU state is that of the caller,
 *       this function must be called using CDECL conventions (call-cleanup),
 *       as self-hosting cleanup would require at least one register
 *       to become clobbered before a CPU state can be generated.
 * NOTE: `cpu_sched_setrunning_savef()' is the same as `cpu_sched_setrunning_save()',
 *       but allows you to specify the EFLAGS that shall be set upon return, thus
 *       allowing you to atomically switch() and enable interrupts upon return.
 * HINT: You can use the 'pflag_t' returned by `PREEMPTION_PUSH()' for 'eflags'
 * WARNING: The caller is responsible to disable interrupts before calling either of these!
 */
FUNDEF void ATTR_CDECL cpu_sched_setrunning_save(struct task *__restrict task);
FUNDEF void ATTR_CDECL cpu_sched_setrunning_savef(struct task *__restrict task, u32 eflags);


/* Perform a regular task rotation, selecting the next appropriate task for execution.
 * NOTE: The caller must disable preemption before calling this function.
 * @return: * : The newly selected task (aka. what `THIS_TASK' resolves to). */
FUNDEF struct task *FCALL cpu_sched_rotate(void);
/* Same as `cpu_sched_rotate', but try not to return the previously running task. */
FUNDEF struct task *FCALL cpu_sched_rotate_yield(void);

/* Remove the current task, causing the caller to inherit a
 * reference to it, before switching to the next available task.
 * NOTE: The caller must disable preemption before calling this function.
 * WARNING: The caller is responsible to ensure that the current task wasn't the IDLE task.
 * @return: * : The task that was removed. */
FUNDEF REF struct task *FCALL cpu_sched_remove_current(void);

#ifdef CONFIG_DEBUG
FUNDEF void KCALL cpu_validate_counters(bool private_only);
#else
#define cpu_validate_counters(private_only) (void)0
#endif

/* The initial boot cpu (Always has id #0; same as 'smp_hwcpu.hw_cpuv[0]') */
#ifndef ____bootcpu_defined
#define ____bootcpu_defined 1
DATDEF struct cpu __bootcpu;
#endif /* !____bootcpu_defined */
#define BOOTCPU (&__bootcpu)

/* The original BOOT-task, as well as host for '/bin/init'. */
DATDEF struct task inittask;



#ifdef CONFIG_BUILDING_KERNEL_CORE
INTDEF byte_t __bootstack[];
#define BOOTSTACK_ADDR  (byte_t *)__bootstack
#define BOOTSTACK_SIZE            0x4000

struct mb_info; /* From `/proprietary/multiboot.h' */

/* Initialize the scheduler by installing a PIT IRQ
 * handler & enabling PIT interrupts on the boot CPU.
 * In addition, secondary CPUs are scanned for, initialized,
 * and controller structures initialized for them. */
INTDEF INITCALL void KCALL sched_initialize(void);

/* Kernel main() function, called after __bootstack has been installed.
 * By the time this function is called, nothing has been initialized, yet. */
INTDEF INITCALL void ATTR_FASTCALL kernel_boot(u32 mb_magic, struct mb_info *mb_mbt);
#endif

DECL_END

#endif /* !GUARD_INCLUDE_SCHED_CPU_H */
