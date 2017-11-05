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
#ifndef GUARD_INCLUDE_SCHED_TASK_H
#define GUARD_INCLUDE_SCHED_TASK_H 1

#include <hybrid/compiler.h>

#ifdef __CC__
#include <errno.h>
#include <hybrid/types.h>
#include <kernel/malloc.h>
#include <sched/percpu.h>
#include <sched/types.h>

DECL_BEGIN

#ifdef CONFIG_NO_LDT
#define __TASK_SWITCH_LDT(old,new) /* Nothing */
#else
#define __TASK_SWITCH_LDT(old,new) \
    if ((old)->t_arch.at_ldt_gdt != \
        (new)->t_arch.at_ldt_gdt) { \
     __asm__ __volatile__("lldt %0\n" : : "g" ((new)->t_arch.at_ldt_gdt)); \
    }
#endif

#ifdef CONFIG_NO_FPU
#define __TASK_SWITCH_FPU(old,new) /* Nothing */
#else
#define __TASK_SWITCH_FPU(old,new) FPUSTATE_DISABLE();
#endif

#ifdef CONFIG_NO_TLB
#define __TASK_SWITCH_TLB(old,new) /* Nothing */
#else
/* Update the TLB pointers in the current cpu's GDT. */
#define __TASK_SWITCH_TLB(old,new) \
    { struct segment *seg = &CPU(cpu_gdt).ip_gdt[SEG_USER_TLB]; \
      SEGMENT_STBASE(seg[SEG_USER_TLB-SEG_USER_TLB],(new)->t_tlb); \
      SEGMENT_STBASE(seg[SEG_USER_TIB-SEG_USER_TLB],&(new)->t_tlb->tl_tib); \
    }
#endif


/* Switch secondary context registers such as LDT, page-directory or
 * the FPU-state as is required when switching from 'old' to `new'. */
#define TASK_SWITCH_CONTEXT(old,new) \
do{ struct mman *const new_mm = (new)->t_mman; \
    if ((old)->t_mman != new_mm) { \
     /* Switch page directories. */ \
     PDIR_STCURR(new_mm->m_ppdir); \
     /* Switch LDT descriptors. (NOTE: Must always \
      * be equal within the same page-directory) */ \
     __TASK_SWITCH_LDT(old,new) \
    } \
    /* Disable the FPU to cause lazy register save/ \
     * restore the next time operations are performed. */ \
    __TASK_SWITCH_FPU(old,new) \
    /* Load the new task's TLB and TIB. */ \
    __TASK_SWITCH_TLB(old,new) \
}while(0)


struct task;

/* Allocate a new task. The caller must initialize the following
 * members before calling `task_start()' to start the thread:
 *  - t_cstate (Must be apart of `t_hstack')
 *  - t_affinity (The set of CPUs that the task may run on)
 *  - t_ustack (Optional; leave it set to NULL if no kernel-managed user-stack is used)
 *  - t_hstack (Memory must be locked in-core; `task_mkhstack' may be used for a quick setup)
 *              NOTE: Memory must be allocated as a virtual mapping in shared memory 
 *                    within `mman_kernel', and the address of the `task' itself must
 *                    be used as argument for `closure' in `mman_mmap_unlocked',
 *                    while `notify' must remain NULL!
 *  - t_tlb (Optional; Use `task_mktlb()'; pre-initialized to `PAGE_ERROR')
 *  - t_mman (As a real reference)
 *  - t_fdman (As a real reference)
 *  - t_sighand (As a real reference; Set to `sighand_kernel' for kernel threads)
 *  - t_sigshare (As a real reference; Set to `sigshare_kernel' for kernel threads')
 *  - t_priority (Optional; pre-initialized to `TASKPRIO_DEFAULT')
 * @return: * :   A reference to the newly allocated task.
 * @return: NULL: Not enough available memory. */
#define task_new() task_cinit((struct task *)kmemalign(TASK_ALIGN,sizeof(struct task), \
                                                       GFP_SHARED|GFP_LOCKED|GFP_CALLOC))
FUNDEF struct task *KCALL task_cinit(struct task *self);


/* Quickly allocate a host (kernel) stack for the given task, consisting of `n_bytes'.
 * In addition, this function will ensure that all of the stack is allocated+locked
 * in-core, as is required to prevent triple faults when accessing unallocated memory
 * without an allocated stack (which would cause an infinite recursion...)
 * NOTE: The stack is allocated in `mman_kernel' and mapped in shared memory,
 *       meaning that (for obvious reasons of pure functionality), the stack
 *       can later be accessed from any page directory.
 * NOTE: The caller is responsible never to pass ZERO(0) for `n_bytes'
 * @param: n_bytes:  The min amount of bytes to allocate for
 *                   the stack (will be ceil-aligned by PAGESIZE)
 * @return: -EOK:    Successfully allocated the stack.
 * @return: -ENOMEM: Not enough available memory / address space.
 * @return: -EINTR:  The calling thread was interrupted. */
FUNDEF SAFE errno_t KCALL task_mkhstack(struct task *__restrict self, size_t n_bytes);
#define TASK_HOSTSTACK_ADDRHINT    0xf0000000 /*< Search for suitable memory below this address,
                                               *  but if no space is found, search above as well. */
#define TASK_HOSTSTACK_DEFAULTSIZE 0x4000 /*< Default, generic size for host stacks. */
#define TASK_HOSTSTACK_BOOTSIZE    0x4000 /*< For reference: The size of the boot stack. */
#define TASK_HOSTSTACK_IDLESIZE    0x4000 /*< For reference: The size of IDLE-thread stacks. */
#ifndef CONFIG_NO_JOBS
#define TASK_HOSTSTACK_WORKSIZE    0x4000 /*< For reference: The size of WORK-thread stacks. */
#endif /* !CONFIG_NO_JOBS */
#define TASK_HOSTSTACK_ALIGN       16     /*< Alignment hint that should be respected by all host-stack allocators. */

/* Similar to `task_mkhstack()', but the stack is allocated lazily for user-space. */
FUNDEF SAFE errno_t KCALL task_mkustack(struct task *__restrict self, size_t n_bytes, size_t guard_size, u16 funds);
#define TASK_USERSTACK_ADDRHINT    0x10000000 /*< Search for suitable memory below this address, but if no space is found, search above as well. */
#define TASK_USERSTACK_DEFAULTSIZE 0x4000     /*< Default, generic size for user stacks. */
#define TASK_USERSTACK_FUNDS       8          /*< Default amount of funding for guard-pages in user-space stacks. */
#define TASK_USERSTACK_ALIGN       16         /*< Alignment hint that should be respected by all user-stack allocators. */
#define TASK_USERSTACK_GUARDSIZE   PAGESIZE   /*< Default user-space stack guard size. */
#define TASK_USERTLB_ADDRHINT      0xa0000000 /*< Search for suitable memory below this address, but if no space is found, search above as well. */


#ifndef CONFIG_NO_TLB
/* Allocate the Thread local block for the given task.
 * TODO: Add an API for ELF TLS memory relocations.
 * @return: -EOK:    Successfully created and mapped the TLB block.
 * @return: -ENOMEM: Not enough available memory.
 * @return: -EINTR:  The calling thread was interrupted. */
FUNDEF SAFE errno_t KCALL task_mktlb(struct task *__restrict self);
/* Same as `task_mktlb()', but the caller must be holding a write-lock on `self->t_mman' */
FUNDEF SAFE errno_t KCALL task_mktlb_unlocked(struct task *__restrict self);

/* mman notification used for tracking TLB mappings.
 * @param: closure: The `struct task' associated with the TLB block. */
FUNDEF ssize_t KCALL task_tlb_mnotify(unsigned int type, void *__restrict closure,
                                      struct mman *mm, ppage_t addr, size_t size);

/* A safe wrapper around `task_filltlb()' */
FUNDEF void KCALL task_ldtlb(struct task *__restrict self);

/* Fill in the TLB information block of the given task.
 * WARNING: This function may cause a PAGEFAULT, meaning that it must be
 *          called in a protected context, or as a user helper function.
 * WARNING: Additionally, the caller must switch to the page directory of `self'. */
FUNDEF void KCALL task_filltlb(struct task *__restrict self);

#endif /* !CONFIG_NO_TLB */

#define TASK_NOTIFY_PID_CHANGED(self) (void)0


/* Set the leader/parent of a given task before being started with `task_start()'
 * NOTE: Both of these functions must be called _exactly_ ONCE before `task_start()'.
 * HINT: The bootstrap task is its own parent/leader.
 * @return: -EOK:    Successfully set the given task as leader/parent.
 * @return: -EINVAL: The given leader/parent task has been terminated.
 * @return: -ESRCH: [task_set_leader] The given 'leader' cannot be used as head of a
 *                                     process group (`TASKFLAG_NOTALEADER' is set). */
FUNDEF errno_t KCALL task_set_leader(struct task *__restrict self, struct task *__restrict leader);
FUNDEF errno_t KCALL task_set_parent(struct task *__restrict self, struct task *__restrict parent);

/* Add the task to the local/global PID namespace.
 * NOTE: Both of these functions must be called _exactly_ ONCE before `task_start()'.
 * @return: -EOK:    Successfully added the task.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF errno_t KCALL task_set_id(struct task *__restrict self, struct pid_namespace *__restrict ns);
#define task_set_gpid(self)                 task_set_id(self,&pid_global)
#define task_set_lpid(self,local_namespace) task_set_id(self,local_namespace)

/* Perform final setup on the given task, registering it in
 * the associated cpu and allowing it to start executing.
 * @return: -EOK:   Successfully started the task.
 * #ifdef CONFIG_SMP
 * @return: -ENODEV: Failed to find/start an accepting CPU to run `self' under.
 *             NOTE: This error is never returned when either `__bootcpu' (id #0) is allowed.
 * #endif
 * HINT: Even in the event of this function failing, you
 *       may still use `TASK_DECREF()' to destroy the task.
 */
FUNDEF errno_t KCALL task_start(struct task *__restrict self);

#ifdef CONFIG_SMP
/* Safely set the CPU that `self' is running on.
 * NOTE: If `self' is the caller's task, it this function will return
 *       in the context of `new_cpu' (UPON_SUCCESS(THIS_CPU == new_cpu))
 * NOTE: This function is a no-op when `new_cpu' already was the running CPU.
 * NOTE: The caller is responsible for ensuring that `self' is allowed to be running on `new_cpu'
 *      (You should be hold a read-lock to `t_affinity_lock' when calling this function)
 * @return: * :         The old CPU that `self' was running under before.
 * @return: -ECOMM:     Failed to communicate with the CPU that `self' was (is) running under.
 * @return: E_ISERR(*): Failed to change the task's CPU for some reason. */
FUNDEF struct cpu *KCALL
task_setcpu(struct task *__restrict self,
            struct cpu *__restrict new_cpu);

/* Get the CPU that `self' is currently running on.
 * WARNING: By the time this call returns, the task may
 *          have already switched to a different CPU!
 * NOTE: When stable access to the task's cpu is required,
 *      `TASK_CPULOCK_(READ|WRITE)' must be used. */
#define task_getcpu(self) ATOMIC_READ((self)->t_cpu)
#else
#define task_getcpu(self) (&__bootcpu)
#endif

#define task_checkcap(self,cap) 1
#define capable(cap) task_checkcap(THIS_TASK,cap)

FUNDEF SAFE void KCALL task_destroy(struct task *__restrict self);
FUNDEF SAFE void KCALL task_weak_destroy(struct task *__restrict self);
#define TASK_TRYINCREF(self)      ATOMIC_INCIFNONZERO((self)->t_refcnt)
#define TASK_INCREF(self)      (void)(ATOMIC_FETCHINC((self)->t_refcnt))
#define TASK_DECREF(self)      (void)(ATOMIC_DECFETCH((self)->t_refcnt) || (task_destroy(self),0))
#define TASK_WEAK_INCREF(self) (void)(ATOMIC_FETCHINC((self)->t_weakcnt))
#define TASK_WEAK_DECREF(self) (void)(ATOMIC_DECFETCH((self)->t_weakcnt) || (task_weak_destroy(self),0))

FUNDEF CRIT void (KCALL task_endcrit)(void);
#ifdef __INTELLISENSE__
FUNDEF bool (KCALL task_iscrit)(void);
FUNDEF bool (KCALL task_issafe)(void);
FUNDEF bool (KCALL task_isnointr)(void);
FUNDEF void (KCALL task_crit)(void);
FUNDEF void (KCALL task_nointr)(void);
FUNDEF void (KCALL task_endnointr)(void);
#else
#define task_iscrit()    (THIS_TASK->t_critical != 0)
#define task_issafe()    (task_iscrit() || !XBLOCK({ register u32 __efl; __asm__ __volatile__("pushf; popl %0" : "=g" (__efl)); XRETURN __efl&0x200; }))
#define task_isnointr()  (THIS_TASK->t_nointr != 0)
/* Increment the critical counter with a write-barrier directly afterwards. */
#define task_crit()      (void)(++THIS_TASK->t_critical)
#define task_endcrit()   (void)(THIS_TASK->t_critical > 1 ? (void)--THIS_TASK->t_critical : (task_endcrit)())
/* Simply increment/decrement the nointr-counter. */
#define task_nointr()    (void)(++THIS_TASK->t_nointr)
#define task_endnointr() (void)(assert(THIS_TASK->t_nointr),--THIS_TASK->t_nointr)
#endif

/* Push/pop the current set of wait-signals.
 * >> Required when a task is forced to allocate more signal-wait
 *    slots, as `malloc()' uses functions like 'rwlock_write()'
 *    internally, which in turn would normally register new
 *    signals to-be waited for, as well as run `task_waitfor()',
 *    which clears all wait-for signals.
 *    Another, even more dangerous case is mman's #PF handler,
 *    which may be called at any point whenever execution doesn't
 *    originate from a core-locked & pre-faulted region of memory,
 *    or accidentally uses some piece of data that got swapped.
 * NOTE: It is important that signals can still be sent and be considered
 *       as such, even when not actively being apart of the target thread's
 *       set of receiving tasks.
 * HINT: When doing this, transfer signals one at a time while holding
 *       temporary write-locks on each. (Nothing will be able to modify
 *       any of the signal's pointers while you're doing this)
 *       Also make sure to add an exception to `sig_vsendone_unlocked()'
 *       for handling signals being sent to tasks not actively having
 *       the signal apart of their wait-for set.
 * NOTE: Calling `task_pushwait()' will transfer all signals that the
 *       calling thread is currently waiting for to `sigs', meaning
 *       that following a call to this function, the caller's set
 *       of wait-for signals will be empty.
 *    >> Similarly, `task_popwait()' overrides the calling threads
 *       signal set with that inside the given `sigs', discarding
 *       any signal (either pending, or received) that may have been
 *       active at the time the same way `(void)task_clrwait()' would.
 * WARNING: Attempting to restore a signal set in a task other than
 *          the one it was created by causes undefined behavior.
 * WARNING: Attempting to restore a signal set not previously created
 *          by another call to `task_pushwait()' also causes undefined
 *          behavior.
 * WARNING: As far as saving/restoring signal sets goes, unless documented
 *          otherwise, signal sets must be caller-saved around any kind
 *          of function call that may potentially block.
 *    NOTE: This includes functions like malloc()!
 *    NOTE: This (obviously) does not include functions used to control
 *          the addition/removing/waiting-for of per-thread signals, as
 *          controlled using the functions below.
 *          It also does NOT include code that may potentially trigger
 *          an ALLOA/COW fault, meaning that you are safe to access
 *          memory that isn't locked in-core & pre-faulted, as the
 *          pagefault handler uses these exact functions to save/restore
 *          the calling thread's signal-wait-set. */
FUNDEF SAFE void KCALL task_pushwait(struct tasksig *__restrict sigs);
FUNDEF SAFE void KCALL task_popwait(struct tasksig *__restrict sigs);


/* Low-level wait for signals with optional timeout.
 * NOTE: During this operation, all signals previously
 *       registered using `task_addwait' are removed.
 * HINT: In the event that the caller is not waiting
 *       for any signals, they will become suspended
 *       until being interrupted due to being terminated
 *      (on if the caller was critical at the time), or
 *       by being interrupted (`task_interrupt()'), such
 *       as by being sent a signal (`task_kill()'), both
 *       of with will result in `-EINTR' being returned,
 *       or finally (only when non-NULL), the given `abstime'
 *       has expired, causing `-ETIMEDOUT' to be returned.
 * NOTE: You may imaging this function being implemented as follows:
 *      (But be aware that this is where ~TRUE~ scheduling takes places,
 *       meaning that there actually isn't any busy waiting involved!)
 *    >> struct sig *result;
 *    >> while ((result = task_trywait()) == NULL) {
 *    >>     if (TASK_TESTINTR()) return E_PTR(-EINTR);
 *    >>     if (abstime) {
 *    >>         struct timespec now;
 *    >>         sysrtc_get(&now);
 *    >>         if (TIMESPEC_GREATER_EQUAL(&now,abstime))
 *    >>             return E_PTR(-ETIMEDOUT);
 *    >>     }
 *    >>     task_yield();
 *    >> }
 *    >> return result;
 * @return: * :          A pointer to the signal that woke up the thread.
 * @return: -EINTR:      The calling thread was interrupted.
 * @return: -ETIMEDOUT: [abstime != NULL] The given timeout has expired. */
FUNDEF struct sig *KCALL task_waitfor(jtime_t abstime);
FUNDEF struct sig *KCALL task_waitfor_t(struct timespec const *abstime);

/* Check if any of the calling thread's pending signals have been sent,
 * returning the signal that was sent, as well as clearing the set of
 * signals currently being waited for.
 * @return: * :   The signal that was received first.
 *          NOTE: In this case, all signals added via `task_addwait()' are cleared.
 * @return: NULL: No signals in the calling thread's waiting-set has been sent. */
FUNDEF struct sig *KCALL task_trywait(void);

/* Same as `task_trywait() != NULL', but don't clear the signal-receive
 * vector in the event that a signal has already been sent and received.
 * @return: true:  A pending signal has been received.
 * @return: false: Either the caller is not receiving any signals,
 *                 or none of the signals they are waiting for have
 *                 been sent. */
#ifdef __INTELLISENSE__
FUNDEF bool KCALL task_tstwait(void);
#else
#define task_tstwait() (ATOMIC_READ(THIS_TASK->t_signal.ts_recv) != NULL)
#endif

/* Add a given to the calling thread's wait chain.
 * NOTE:    In the event that 's' is already being received from,
 *          this function is a no-op and simply returns -EOK.
 * WARNING: The caller is responsible for holding a write-lock to 's'
 * @param: buffer:   An optional pointer to a buffer to-be
 *                   filled with data sent over the given signal.
 * @param: bufsize:  The size of `buffer' (in bytes).
 * @return: -EOK:    Successfully added the given signal (guarantied for the first signal)
 * @return: -ENOMEM: Not enough available memory to allocate more signal slots. */
FUNDEF SAFE errno_t KCALL task_addwait(struct sig *__restrict s, void *buffer, size_t bufsize);

/* Check for a given signal from the waitfor-set of the calling thread.
 * @return: true:  The given signal was found.
 * @return: false: The given signal wasn't recognized. */
FUNDEF bool KCALL task_haswait(struct sig *__restrict s);

/* Clear all signals that the calling thread is waiting for.
 * @return: * :   In the event that before being cleared, at least one signal
 *                was sent a pointer to that signal is returned by this function.
 *             >> This allows the caller to detect signal transmission even if
 *                they decide not to block until a signal does arrive.
 * @return: NULL: All pending signals were cleared and no signal had been sent until now. */
FUNDEF SAFE struct sig *KCALL task_clrwait(void);

/* Set the scheduler priority of the given thread `self'.
 * NOTE: Using this function you can also set if a thread is considered IDLE.
 * @return: -EOK:    Successfully set the given thread's priority.
 * @return: -EINVAL: The given thread has been terminated.
 * @return: -ECOMM:  Failed to communicate with the CPU the thread is running under.
 * @return: -EINTR:  The calling thread was interrupted. */
FUNDEF errno_t KCALL task_set_priority(struct task *__restrict self,
                                       taskprio_t new_priority,
                                       taskprio_t *pold_priority);
#define task_get_priority(self) ATOMIC_READ((self)->t_priority)

#ifndef __pflag_t_defined
#define __pflag_t_defined 1
typedef register_t pflag_t; /* Push+disable/Pop preemption-enabled. */
#endif


/* Recursively suspend/resume the given task.
 * WARNING: `-EINVAL' can only be returned when the
 *          suspension counter of `self' rolls over.
 *          With that in mind, don't rely on the return value
 *          to determine if a task has already terminated!
 * @param: mode:     Set of `TASK_SUSP_*'
 * @return: -EOK:    Successfully suspended the given task.
 * @return: -EINVAL: The given task has terminated.
 * @return: -ECOMM:  Failed to communicate with the CPU the thread is running under.
 * @return: -EINTR:  The calling thread was interrupted. */
FUNDEF errno_t KCALL task_suspend(struct task *__restrict self, u32 mode);
FUNDEF errno_t KCALL task_resume(struct task *__restrict self, u32 mode);
FUNDEF errno_t KCALL task_suspend_cpu_endwrite(struct task *__restrict self, u32 mode, pflag_t was);
FUNDEF errno_t KCALL task_resume_cpu_endwrite(struct task *__restrict self, u32 mode, pflag_t was);
#define TASK_SUSP_REC  0x00 /*< Recursively suspend/resume the task. */
#define TASK_SUSP_USER 0x00 /*< Use user-level recursion for suspend/resume. */
#define TASK_SUSP_HOST 0x01 /*< Use host-level recursion for suspend/resume. */
#define TASK_SUSP_NOW  0x02 /*< NOTE: May only be used with `TASK_SUSP_USER': Suspend/resume the task _NOW_. */

/* Test for pending interrupts within the calling thread.
 * NOTE: No-op when interrupts are disabled using `task_nointr()'
 * @return: -EOK:   No pending interrupts were triggered.
 * @return: -EINTR: The calling thread was interrupted. */
FUNDEF errno_t KCALL task_testintr(void);
#define TASK_TESTINTR()  (task_testintr() == -EINTR)

/* After having received an interrupt request, reschedule it for later. */
#define task_intr_later() (void)ATOMIC_FETCHOR(THIS_TASK->t_flags,TASKFLAG_INTERRUPT)

/* Terminate the given task using the provided exitcode.
 * NOTE: If `THIS_TASK' is passed for `self', and the calling
 *       thread isn't executing within a critical section,
 *       this function doesn't return.
 * @return: -EOK:    Successfully terminated the given task.
 * @return: -EINVAL: The given task had already been terminated.
 * @return: -ECOMM:  Failed to communicate with the CPU the thread is running under.
 * @return: -EINTR:  The calling thread was interrupted. */
FUNDEF errno_t KCALL
task_terminate(struct task *__restrict self,
               void *exitcode);

/* Same as `task_terminate()', but allows the caller to terminate
 * a task while holding a write-lock on a given CPU, that will be
 * dropped by a call to this function.
 * The caller must also disable preemption before calling this function. */
FUNDEF SAFE errno_t KCALL
task_terminate_cpu_endwrite(struct cpu *__restrict c,
                            struct task *__restrict self,
                            void *exitcode);

/* Interrupt the current/next call to `task_waitfor()' of the given task.
 * @return: -EOK:    Successfully terminated the given task.
 * @return: -EINTR: `self' is THIS_TASK, and you're just interrupted yourself.
 * @return: -EINVAL: The given task had already been terminated. */
FUNDEF errno_t KCALL task_interrupt(struct task *__restrict self);
FUNDEF SAFE errno_t KCALL task_interrupt_cpu_endwrite(struct task *__restrict self);

/* Join the given task `self'.
 * @return: -EOK:       Successfully joined the given task.
 * @return: -ETIMEDOUT: The given timeout has expired.
 * @return: -EINTR:     The calling thread was interrupted. */
FUNDEF errno_t KCALL task_join(struct task *__restrict self, jtime_t timeout, void **exitcode);

/* Yield the remainder of the caller's quantum to the next
 * scheduled task (no-op if no task to switch to exists). */
FUNDEF void KCALL task_yield(void);

/* Unlock the associated CPU and pause execution until the task is interrupted.
 * NOTE: The caller must have disabled pre-emption and be holding a write-lock to `THIS_CPU'
 * @return: -EINTR:      The calling thread was interrupted.
 * @return: -ETIMEDOUT: [abstime != NULL || abstime != JTIME_INFINITE] The given timeout has expired. */
FUNDEF SAFE errno_t KCALL task_pause_cpu_endwrite(jtime_t abstime);
FUNDEF SAFE errno_t KCALL task_pause_cpu_endwrite_t(struct timespec const *abstime);


/* Get/Set the CPU-affinity of a given task.
 * NOTE: When setting, the task is moved to a different
 *       CPU if the old one conflicts with the new mask.
 * @return: -EOK:     Successfully got/set the task's affinity.
 * @return: -EFAULT:  A faulty pointer was given.
 * @return: -ECOMM:  [task_set_affinity] While attempting to migrate the task, communication
 *                                       with the new CPU selected internally failed.
 * @return: -ENODEV: [task_set_affinity] No CPU matching 'affinity' found.
 * @return: -EINVAL: [task_set_affinity] The given task has been terminated. */
FUNDEF errno_t KCALL task_get_affinity(struct task *__restrict self, USER __cpu_set_t *affinity);
FUNDEF errno_t KCALL task_set_affinity(struct task *__restrict self, USER __cpu_set_t const *affinity);

/* Tries to un-share the memory manager associated with the current
 * task by creating a copy and replacing `THIS_TASK->t_mman' with it
 * when its reference counter is greater than 1.
 * WARNING: Don't call this function while inside a `TASK_PDIR_KERNEL_BEGIN()' block.
 * @param: unmap_old_ustack: When true, unmap the user-space stack within the old memory manager.
 *                           This argument is set to `true' when called because of an `unshare()'
 *                          (causing the stack to disappear from the old VM-space), but kept
 *                           as `false' when called due to a `fork()' (to allow for continued
 *                           execution using the original stack).
 * @return: -EOK:       The mman associated with `THIS_TASK' is now unique.
 * @return: -ENOMEM:    Not enough available memory.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): Some error code returned by an `mb_notify(MNOTIFY_UNSHARE_DROP)' callback. */
FUNDEF SAFE errno_t KCALL task_unshare_mman(bool unmap_old_ustack);

/* Unshare the calling task's FD-manager.
 * @return: -EOK:    The fdman associated with `THIS_TASK' is now unique.
 * @return: -ENOMEM: Not enough available memory.
 * @return: -EINTR:  The calling thread was interrupted. */
FUNDEF SAFE errno_t KCALL task_unshare_fdman(void);

/* Unshare the calling task's signal handlers.
 * @return: -EOK:    The sighand associated with `THIS_TASK' is now unique.
 * @return: -ENOMEM: Not enough available memory.
 * @return: -EINTR:  The calling thread was interrupted. */
FUNDEF SAFE errno_t KCALL task_unshare_sighand(void);

#if defined(CONFIG_DEBUG) && 1
#define CONFIG_LOG_WAITING   1
#if 1
#define SUPPRESS_WAITLOGS_BEGIN() (void)0
#define SUPPRESS_WAITLOGS_END()   (void)0
#else
DATDEF u8 dont_log_waiting;
#define SUPPRESS_WAITLOGS_BEGIN() (void)ATOMIC_FETCHINC(dont_log_waiting)
#define SUPPRESS_WAITLOGS_END()   (void)ATOMIC_FETCHDEC(dont_log_waiting)
#endif
#else
#define SUPPRESS_WAITLOGS_BEGIN() (void)0
#define SUPPRESS_WAITLOGS_END()   (void)0
#endif


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_INCLUDE_SCHED_TASK_H */
