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
#ifndef GUARD_INCLUDE_SCHED_SIGNAL_H
#define GUARD_INCLUDE_SCHED_SIGNAL_H 1

#include <hybrid/compiler.h>

#ifndef CONFIG_NO_SIGNALS
#include <errno.h>
#include <hybrid/atomic.h>
#include <hybrid/list/list.h>
#include <hybrid/types.h>
#include <sched/types.h>
#include <signal.h>
#include <stdbool.h>

DECL_BEGIN

struct task;

struct sigqueue {
 /* Descriptor for a signal that was send, but was
  * blocked and therefor scheduled to be received later. */
 SLIST_NODE(struct sigqueue) sq_chain;  /*< [owned] Chain of queued signals. */
 siginfo_t                   sq_info;   /*< Signal information. */
};


struct sigpending;
/* >> struct sigpending { ... };
 * Defined in <sched/types.h> */
#define sigpending_reading(x)     sig_reading(&(x)->sp_newsig)
#define sigpending_writing(x)     sig_writing(&(x)->sp_newsig)
#define sigpending_tryread(x)     sig_tryread(&(x)->sp_newsig)
#define sigpending_trywrite(x)    sig_trywrite(&(x)->sp_newsig)
#define sigpending_tryupgrade(x)  sig_tryupgrade(&(x)->sp_newsig)
#define sigpending_read(x)        sig_read(&(x)->sp_newsig)
#define sigpending_write(x)       sig_write(&(x)->sp_newsig)
#define sigpending_upgrade(x)     sig_upgrade(&(x)->sp_newsig)
#define sigpending_downgrade(x)   sig_downgrade(&(x)->sp_newsig)
#define sigpending_endread(x)     sig_endread(&(x)->sp_newsig)
#define sigpending_endwrite(x)    sig_endwrite(&(x)->sp_newsig)

/* Enqueue a signal given its information within the given sigpending controller.
 * NOTE: The caller must be holding a write-lock to the given `sigpending' controller.
 * NOTE: Upon success, the caller is responsible for broadcasting the 'sp_newsig'
 *       signal, thus notifying sleepers that a new signal has arrived.
 * @return: -EOK:    Successfully added the signal as pending.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF errno_t KCALL sigpending_enqueue_unlocked(struct sigpending *__restrict self,
                                                 siginfo_t const *__restrict signal_info);
/* Same as the above, but automatically locks and braodcasts the controller signal upon success. */
FUNDEF errno_t KCALL sigpending_enqueue(struct sigpending *__restrict self,
                                        siginfo_t const *__restrict signal_info);

/* Discard all queued signals matching `signo', returning the exact amount deleted. */
FUNDEF size_t KCALL sigpending_discard(struct sigpending *__restrict self, int signo);

/* Wait for the first signal apart of `wait_mask' to become available and return it.
 * NOTE: The caller must either be holding a read, or write-lock on `self',
 *       indicating the state of said lock through '*has_write_lock'.
 *       If only a read-lock was held, this function is allowed to upgrade the lock
 *       to write-mode, and indicating this upgrade by setting '*has_write_lock' to true.
 *       WARNING: This behavior may even be triggered in error-cases.
 * @return: * :       The queue entry now inherited by the caller (must be free()'ed)
 * @return: SIGPENDING_UNDEFINED(signum):
 *                    A pending signal was dequeued, but it had no associated data.
 *                    The caller should assume default behavior for `signum'.
 * @return: NULL:     No signal apart of the given mask was queued.
 * @return: -ERELOAD: Failed to atomically upgrade a read-lock to a write-lock,
 *                    meaning that the `sigpending' controller may have changed
 *                    state in the mean time.
 *                    NOTE: Only returned when '*has_write_lock' was false upon entry,
 *                          in which case '*has_write_lock' will always be true upon exit.
 *                 >> When this is returned, the caller should try again after
 *                   (optionally) re-performing additional checks on the state
 *                    of the `sigpending' controller.
 * @CALLER_CLEANUP: if (!SIGPENDING_IS_UNDEFINED(return)) free(return);
 */
FUNDEF /*inherit*/struct sigqueue *KCALL
sigpending_try_dequeue_unlocked(struct sigpending *__restrict self,
                                __sigset_t const *__restrict deque_mask,
                                bool *__restrict has_write_lock);
#define SIGPENDING_IS_UNDEFINED(p)    ((uintptr_t)(p) < _NSIG)
#define SIGPENDING_GT_UNDEFINED(p)    ((int)(p))
#define SIGPENDING_UNDEFINED(signum)  ((struct sigqueue *)(uintptr_t)(signum))


struct sigshare {
 /* Descriptor for shared, pending signals.
  * >> Used when a signal is send to a thread-group, rather than a single thread. */
 ATOMIC_DATA ref_t  ss_refcnt;  /*< Reference counter for this structure. */
 struct sigpending  ss_pending; /*< Shared controller for pending signals. */
};

DATDEF struct sigshare sigshare_kernel;
#define THIS_SIGSHARE (THIS_TASK->t_sigshare)

#define SIGSHARE_INCREF(self) (void)(ATOMIC_FETCHINC((self)->ss_refcnt))
#define SIGSHARE_DECREF(self) (void)(ATOMIC_DECFETCH((self)->ss_refcnt) || (sigshare_destroy(self),0))
FUNDEF void KCALL sigshare_destroy(struct sigshare *__restrict self);
FUNDEF REF struct sigshare *KCALL sigshare_new(void);




struct sighand {
 ATOMIC_DATA ref_t sh_refcnt;           /*< Reference counter. */
 struct sigaction  sh_actions[_NSIG-1]; /*< [WEAK] Vector of user-space signal handlers.
                                         *   NOTE: This vector can't be protected by some kind of
                                         *         lock because that lock would have to be acquired
                                         *         whenever a signal is send using `task_kill2_cpu_endwrite'
                                         *         below. - Yet that function must be lock-less, as it is
                                         *         called while the scheduler is suspended (cpu-lock + no preemption),
                                         *         meaning that it would be illegal for us to switch tasks,
                                         *         as would have to be done if the lock wasn't available
                                         *         immediately.
                                         *      >> But we also can't just say that this vector is protected
                                         *         by `cpu_read()' / `cpu_write()', because it may be shared
                                         *         across multiple CPUs...
                                         */
};

DATDEF struct sighand sighand_kernel;
#define THIS_SIGHAND (THIS_TASK->t_sighand)

#define SIGHAND_INCREF(self) (void)(ATOMIC_FETCHINC((self)->sh_refcnt))
#define SIGHAND_DECREF(self) (void)(ATOMIC_DECFETCH((self)->sh_refcnt) || (sighand_destroy(self),0))
FUNDEF void KCALL sighand_destroy(struct sighand *__restrict self);

/* Allocate/Copy a signal handler controller.
 * @return: * :   A new reference to a unique signal handler.
 * @return: NULL: Not enough available memory. */
FUNDEF REF struct sighand *KCALL sighand_new(void);
FUNDEF REF struct sighand *KCALL sighand_copy(struct sighand *__restrict self);

/* Reset all signal handlers to their defaults. */
FUNDEF void KCALL sighand_reset(struct sighand *__restrict self);

/* Set the signal-blocking set of the calling task in such a way
 * that will deque all signals that were apart of the old set,
 * but are no longer apart of the new.
 * WARNING: To improve performance, this function may modify `newset'.
 * @return: -EOK:   Nothing had to be dequeued.
 * @return: -EINTR: At least one signal was dequeued and caused
 *                  the calling thread to be interrupted. */
FUNDEF errno_t KCALL task_set_sigblock(sigset_t *__restrict newset);

#ifndef __pflag_t_defined
#define __pflag_t_defined 1
typedef register_t pflag_t; /* Push+disable/Pop preemption-enabled. */
#endif

/* Raise a signal `signal_info->si_signo' within the given task `self',
 * using its latest user-space CPU state as exception context.
 * NOTE: That means you can call this on yourself and the
 *       context will be that which would otherwise be restored
 *       upon return from the system-call that has executed you.
 * @param: signal_info:   Signal information.
 * @param: reg_trapno:    The value for `context->uc_mcontext.gregs[REG_TRAPNO]'
 * @param: reg_err:       The value for `context->uc_mcontext.gregs[REG_ERR]'
 * @return: -EOK:         Successfully signaled the given task.
 * @return: -EINTR:       The calling thread was interrupted. (self == THIS_TASK)
 * @return: -EPERM:       Sending signals to the task is not allowed.
 * @return: -EINVAL:      The given task had already been terminated.
 *                        An invalid signal number was passed. */
FUNDEF errno_t KCALL task_kill2(struct task *__restrict self,
                                siginfo_t const *__restrict signal_info,
                                greg_t reg_trapno, greg_t reg_err);

/* Same as `task_kill2()', but allows the caller to raise
 * signals while holding a write-lock on a given CPU, that will
 * be dropped by a call to this function.
 * The caller must also disable preemption before calling this function. */
FUNDEF errno_t KCALL
task_kill2_cpu_endwrite(struct task *__restrict self,
                        siginfo_t const *__restrict signal_info,
                        greg_t reg_trapno, greg_t reg_err,
                        pflag_t was);
#define TASK_KILL2_IFOK(target,signal_info,reg_trapno,reg_err) \
 XBLOCK({ errno_t _e = task_kill2_ok(target,signal_info); \
          XRETURN E_ISOK(_e) ? task_kill2(target,signal_info,reg_trapno,reg_err) : _e; })

/* Check if raising a given signal is OK for the calling user-space thread.
 * @return: -EOK:   Yes, it is ok.
 * @return: -EPERM: No, it's not ok! */
FUNDEF errno_t KCALL task_kill2_ok(struct task *__restrict target,
                                   siginfo_t const *__restrict signal_info);

/* A simplified version of `task_kill2()', resemblant of `raise()' */
FUNDEF errno_t KCALL task_kill(struct task *__restrict self, int signo);

/* Check for pending signals in the calling task.
 * @param: changes: The set of signals to check (Updated to mask out signals that are being blocked).
 * @return: -EOK:   No signals were raised.
 * @return: -EINTR: At least one signal was raised and the caller got interrupted. */
FUNDEF SAFE errno_t KCALL task_check_signals(sigset_t *__restrict changes);


#define SIGENTER_INFO_OFFSETOF_RETURN     0
#define SIGENTER_INFO_OFFSETOF_SIGNO      __SIZEOF_POINTER__
#define SIGENTER_INFO_OFFSETOF_PINFO   (2*__SIZEOF_POINTER__)
#define SIGENTER_INFO_OFFSETOF_PCTX    (3*__SIZEOF_POINTER__)
#define SIGENTER_INFO_OFFSETOF_OLD_XBP (4*__SIZEOF_POINTER__)
#define SIGENTER_INFO_OFFSETOF_OLD_XIP (5*__SIZEOF_POINTER__)
#define SIGENTER_INFO_OFFSETOF_INFO    (6*__SIZEOF_POINTER__)
#define SIGENTER_INFO_OFFSETOF_CTX     (6*__SIZEOF_POINTER__+__SI_MAX_SIZE)
#define SIGENTER_INFO_SIZE             (6*__SIZEOF_POINTER__+__SI_MAX_SIZE+__UCONTEXT_SIZE)

/* Signal delivery implementation */
struct PACKED sigenter_info {
 /* All the data that is pushed onto the user-space stack, or sigalt-stack. */
 USER void       *ei_return;  /*< Signal handler return address. */
union PACKED {
 int              ei_signo;   /*< Signal number. */
 uintptr_t      __ei_align;   /*< Align by pointers. */
};
 USER siginfo_t  *ei_pinfo;   /*< == &ei_info. */
 USER ucontext_t *ei_pctx;    /*< == &ei_ctx. */
 /* SPLIT: The following is used to create a fake stack-frame to fix tracebacks
  *        generated from signal handlers, as well as provide a fixed base-line
  *        for restoring the stack-pointer after the signal handler has finished. */
 USER void       *ei_old_xbp; /*< Holds the value of the old EBP (stackframe pointer);
                               *  When the signal handler is called, this is also where the new EBP points to! */
 USER void       *ei_old_xip; /*< Second part of the stackframe: The return address */
 /* SPLIT: Everything above is setup for arguments to the signal handler. */
union{ siginfo_t  ei_info;
 int            __ei_info_pad[__SI_MAX_SIZE/sizeof(int)]; };
 ucontext_t       ei_ctx;
#ifdef __x86_64__
#define           ei_next  ei_ctx.uc_mcontext.gregs[REG_RSP]
#else
#define           ei_next  ei_ctx.uc_mcontext.gregs[REG_UESP]
#endif
};

DECL_END
#endif /* !CONFIG_NO_SIGNALS */

#endif /* !GUARD_INCLUDE_SCHED_SIGNAL_H */
