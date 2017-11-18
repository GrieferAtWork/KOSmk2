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
#ifndef GUARD_KERNEL_SCHED_SIGNAL_C
#define GUARD_KERNEL_SCHED_SIGNAL_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#ifndef CONFIG_NO_SIGNALS
#include <asm/instx.h>
#include <asm/unistd.h>
#include <bits/signum.h>
#include <bits/waitstatus.h>
#include <dev/rtc.h>
#include <errno.h>
#include <asm/cpu-flags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/traceback.h>
#include <arch/gdt.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <linker/coredump.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <sched/types.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/syslog.h>
#include <sys/ucontext.h>
#include <kos/thread.h>
#include <arch/hints.h>
#include <arch/asm.h>
#include <bits/sigstack.h>

DECL_BEGIN

STATIC_ASSERT(sizeof(siginfo_t) <= __SI_MAX_SIZE);
STATIC_ASSERT(offsetof(siginfo_t,si_signo) == __SIGINFO_OFFSETOF_SIGNO);
STATIC_ASSERT(offsetof(siginfo_t,si_errno) == __SIGINFO_OFFSETOF_ERRNO);
STATIC_ASSERT(offsetof(siginfo_t,si_code) == __SIGINFO_OFFSETOF_CODE);
STATIC_ASSERT(offsetof(siginfo_t,si_pid) == __SIGINFO_OFFSETOF_PID);
STATIC_ASSERT(offsetof(siginfo_t,si_uid) == __SIGINFO_OFFSETOF_UID);
STATIC_ASSERT(offsetof(siginfo_t,si_timerid) == __SIGINFO_OFFSETOF_TIMERID);
STATIC_ASSERT(offsetof(siginfo_t,si_overrun) == __SIGINFO_OFFSETOF_OVERRUN);
STATIC_ASSERT(offsetof(siginfo_t,si_value) == __SIGINFO_OFFSETOF_VALUE);
STATIC_ASSERT(offsetof(siginfo_t,si_int) == __SIGINFO_OFFSETOF_INT);
STATIC_ASSERT(offsetof(siginfo_t,si_ptr) == __SIGINFO_OFFSETOF_PTR);
STATIC_ASSERT(offsetof(siginfo_t,si_status) == __SIGINFO_OFFSETOF_STATUS);
STATIC_ASSERT(offsetof(siginfo_t,si_utime) == __SIGINFO_OFFSETOF_UTIME);
STATIC_ASSERT(offsetof(siginfo_t,si_stime) == __SIGINFO_OFFSETOF_STIME);
STATIC_ASSERT(offsetof(siginfo_t,si_addr) == __SIGINFO_OFFSETOF_ADDR);
STATIC_ASSERT(offsetof(siginfo_t,si_addr_lsb) == __SIGINFO_OFFSETOF_ADDR_LSB);
STATIC_ASSERT(offsetof(siginfo_t,si_lower) == __SIGINFO_OFFSETOF_LOWER);
STATIC_ASSERT(offsetof(siginfo_t,si_upper) == __SIGINFO_OFFSETOF_UPPER);
STATIC_ASSERT(offsetof(siginfo_t,si_band) == __SIGINFO_OFFSETOF_BAND);
STATIC_ASSERT(offsetof(siginfo_t,si_fd) == __SIGINFO_OFFSETOF_FD);
STATIC_ASSERT(offsetof(siginfo_t,si_call_addr) == __SIGINFO_OFFSETOF_CALL_ADDR);
STATIC_ASSERT(offsetof(siginfo_t,si_syscall) == __SIGINFO_OFFSETOF_SYSCALL);
STATIC_ASSERT(offsetof(siginfo_t,si_arch) == __SIGINFO_OFFSETOF_ARCH);
STATIC_ASSERT(sizeof(siginfo_t) == __SIGINFO_SIZE);


/* Signal default actions. */
typedef u8 dact_t; enum{
    DA_TERM, /* Terminate application immediately. */
    DA_CORE, /* Terminate + generate core dump. */
    DA_IGN,  /* Ignore signal. */
    DA_STOP, /* Suspend application. */
    DA_CONT, /* Continue application. */
};

PRIVATE dact_t const default_actions[_NSIG-1] = {
#define ENTRY(i) [i-1]
    ENTRY(SIGHUP)    = DA_TERM,
    ENTRY(SIGINT)    = DA_TERM,
    ENTRY(SIGQUIT)   = DA_CORE,
    ENTRY(SIGILL)    = DA_CORE,
    ENTRY(SIGABRT)   = DA_CORE,
    ENTRY(SIGFPE)    = DA_CORE,
    ENTRY(SIGKILL)   = DA_TERM,
    ENTRY(SIGSEGV)   = DA_CORE,
    ENTRY(SIGPIPE)   = DA_TERM,
    ENTRY(SIGALRM)   = DA_TERM,
    ENTRY(SIGTERM)   = DA_TERM,
    ENTRY(SIGUSR1)   = DA_TERM,
    ENTRY(SIGUSR2)   = DA_TERM,
    ENTRY(SIGCHLD)   = DA_IGN,
    ENTRY(SIGCONT)   = DA_CONT,
    ENTRY(SIGSTOP)   = DA_STOP,
    ENTRY(SIGTSTP)   = DA_STOP,
    ENTRY(SIGTTIN)   = DA_STOP,
    ENTRY(SIGTTOU)   = DA_STOP,
    ENTRY(SIGBUS)    = DA_CORE,
    ENTRY(SIGPOLL)   = DA_TERM,
    ENTRY(SIGPROF)   = DA_TERM,
    ENTRY(SIGSYS)    = DA_CORE,
    ENTRY(SIGTRAP)   = DA_CORE,
    ENTRY(SIGURG)    = DA_IGN,
    ENTRY(SIGVTALRM) = DA_TERM,
    ENTRY(SIGXCPU)   = DA_CORE,
    ENTRY(SIGXFSZ)   = DA_CORE,
#if defined(SIGIOT) && SIGIOT != SIGABRT
    ENTRY(SIGIOT)    = DA_CORE,
#endif
#ifdef SIGEMT
    ENTRY(SIGEMT)    = DA_TERM,
#endif
    ENTRY(SIGSTKFLT) = DA_TERM,
#if defined(SIGIO) && SIGIO != SIGPOLL
    ENTRY(SIGIO)     = DA_TERM,
#endif
#if defined(SIGCLD) && SIGCLD != SIGCHLD
    ENTRY(SIGCLD)    = DA_IGN,
#endif
    ENTRY(SIGPWR)    = DA_TERM,
#ifdef SIGLOST
    ENTRY(SIGLOST)   = DA_TERM,
#endif
    ENTRY(SIGWINCH)  = DA_IGN,
#if defined(SIGUNUSED) && SIGUNUSED != SIGSYS
    ENTRY(SIGUNUSED) = DA_CORE,
#endif
#undef ENTRY
};

#define SIGSET_WORDS (__SIZEOF_SIGSET_T__ / SIGWORD_SIZE)
#if ((__SIZEOF_SIGSET_T__ % 8) == 0) && __SIZEOF_BUSINT__ >= 8
typedef u64 sigword_t;
#define SIGWORD_SIZE 8
#elif ((__SIZEOF_SIGSET_T__ % 4) == 0) && __SIZEOF_BUSINT__ >= 4
typedef u32 sigword_t;
#define SIGWORD_SIZE 4
#elif ((__SIZEOF_SIGSET_T__ % 2) == 0) && __SIZEOF_BUSINT__ >= 2
typedef u16 sigword_t;
#define SIGWORD_SIZE 2
#else
typedef u8  sigword_t;
#define SIGWORD_SIZE 1
#endif


PUBLIC errno_t KCALL
task_kill2_cpu_endwrite(struct task *__restrict t,
                        siginfo_t const *__restrict signal_info,
                        greg_t reg_trapno, greg_t reg_err,
                        pflag_t was) {
 errno_t error = -EOK; int signo;
 struct cpu *c;
 CHECK_HOST_DOBJ(t);
 CHECK_HOST_DOBJ(signal_info);
 c = TASK_CPU(t);
 CHECK_HOST_DOBJ(c);
 assert(cpu_writing(c));
 assert(!PREEMPTION_ENABLED());
 signo = signal_info->si_signo;
 if (signo <= 0 || --signo >= (_NSIG-1)) { error = -EINVAL; goto done; }

#ifdef CONFIG_SMP
 if (c != THIS_CPU) {
  /* TODO: Send IPC communication. */
 } else
#endif
 {
  struct sigaction *action;
  struct sighand *hand;
  /* Check if the signal is currently being blocked. */
  if (TASK_ISBLOCKING(t,signo+1)) {
enqueue_later:
   /* Enqueue the signal to be send at a later time. */
   cpu_endwrite(c);
   PREEMPTION_POP(was);
   /* Check if we're actually allowed to send signals to this task. */
   if (t->t_flags&TASKFLAG_NOSIGNALS)
       return -EPERM;
   return sigpending_enqueue(&t->t_sigpend,signal_info);
  }
  syslog(LOG_DEBUG,"[SIG] kill(%d,%d) %p -> %p\n",
        (int)t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,signo,
         THIS_TASK,t);
#if 0
  debug_tbprint(0);
#endif

  /* NOTE: We know the task is running on our CPU, and with interrupts disabled
   *       we are either that task, or know that it won't be running until we've
   *       re-enabled interrupt, meaning we can full access to `t_sighand', knowing
   *       it will not be exchanged until we're done here.
   * >> In either case, the task isn't actually active in
   *    user-space, and won't randomly change into being! */
  hand = t->t_sighand;
  CHECK_HOST_DOBJ(hand);
  action = &hand->sh_actions[signo];
  if (action->sa_handler == SIG_DFL) {
   switch (default_actions[signo]) {

   case DA_IGN:
    goto ignore_signal;

   case DA_STOP:
do_stop:
    return task_suspend_cpu_endwrite(t,TASK_SUSP_USER|TASK_SUSP_NOW,was);

   case DA_CONT:
do_cont:
    return task_resume_cpu_endwrite(t,TASK_SUSP_USER|TASK_SUSP_NOW,was);

   case DA_CORE:
do_core:
    task_crit();
    error = task_terminate_cpu_endwrite(c,t,(void *)(uintptr_t)
                                       (__WCOREFLAG|__W_EXITCODE(0,signal_info->si_signo)));
    /* Actually generate the coredump. (arch-specific)
     * NOTE: This function may override the IRET tail and call `task_crit()'
     *       to perform the coredump before the task would have switched to
     *       user-space, thus allowing current kernel-operations to finish 
     *       and all user-space registers to be restored. */
    coredump_task(t,signal_info,reg_trapno,reg_err);
    task_endcrit();
    goto ppop_end;

   default:
    error = task_terminate_cpu_endwrite(c,t,(void *)(uintptr_t)__W_EXITCODE(0,signal_info->si_signo));
    goto ppop_end;
   }
  }
  /* KOS extension builtin signal handlers. */
  if (action->sa_handler == SIG_CONT) goto do_cont;
  if (action->sa_handler == SIG_STOP) goto do_stop;
  if (action->sa_handler == SIG_CORE) goto do_core;
  if (action->sa_handler == SIG_HOLD) {
   /* Add the signal to the hold mask (blocking). */
   unsigned long int mask = __sigmask(signo+1);
   unsigned long int word = __sigword(signo+1);
   if (ATOMIC_FETCHOR(t->t_sigblock.__val[word],mask)&mask)
       goto enqueue_later;
   goto done;
  }

  if (action->sa_handler == SIG_IGN) {
   /* Nothing to do here... */
   /* XXX: What about exception signals?
    *      We should probably still do something about those... */
ignore_signal:;
  } else {
   sigword_t *dst,*end,*src;
   /* Ignore the signal if the task is being terminated.
    * NOTE: This is especially required if the task is pending for a
    *       coredump, in which case it's IRET tail has been overwritten,
    *       and the task itself is no longer suitable for scheduling more
    *       signal handlers in userspace. */
   if unlikely(t->t_flags&TASKFLAG_WILLTERM)
      goto ignore_signal;

   /* Actually deliver the signal to the task. (arch-specific) */
   error = deliver_signal(t,action,signal_info,reg_trapno,reg_err);
   /* Mask all blocked signals in this task */
   end = (dst = (sigword_t *)&t->t_sigblock)+SIGSET_WORDS;
   src = (sigword_t *)&action->sa_mask;
   for (; dst != end; ++dst,++src) *dst |= *src;

   /* Delete the handler before it will be executed. */
   if (E_ISOK(error) && action->sa_flags&SA_RESETHAND)
       memset(action,0,sizeof(struct sigaction));
  }
  /* Interrupt (wake) the task, so-as to execute the signal handler. */
  if (E_ISOK(error)) { error = task_interrupt_cpu_endwrite(t); goto ppop_end; }
 }
done:
 cpu_endwrite(c);
ppop_end:
 PREEMPTION_POP(was);
 return error;
}


PUBLIC errno_t KCALL
task_kill2(struct task *__restrict t,
           siginfo_t const *__restrict signal_info,
           greg_t reg_trapno, greg_t reg_err) {
 struct cpu *c; pflag_t was; errno_t error;
 CHECK_HOST_DOBJ(t);
 task_crit();
 was = PREEMPTION_PUSH();
 for (;;) {
  c = TASK_CPU(t);
  COMPILER_READ_BARRIER();
  cpu_write(c);
  if (TASK_CPU(t) == c) break;
  cpu_endwrite(c);
 }
 error = task_kill2_cpu_endwrite(t,signal_info,reg_trapno,reg_err,was);
 task_endcrit();
 return error;
}

PUBLIC errno_t KCALL
task_kill2_ok(struct task *__restrict target,
              siginfo_t const *__restrict signal_info) {
 CHECK_HOST_DOBJ(target);
 CHECK_HOST_DOBJ(signal_info);
 /* TODO */
 return -EOK;
}

PUBLIC errno_t KCALL
task_kill(struct task *__restrict self, int signo) {
 siginfo_t info;
 memset(&info,0,sizeof(siginfo_t));
 info.si_signo = signo;
 info.si_pid   = GET_THIS_PID();
 info.si_uid   = GET_THIS_UID();
 return task_kill2(self,&info,0,0);
}



SYSCALL_DEFINE4(sigaction,int,sig,
                USER struct sigaction const *,act,
                USER struct sigaction *,oact,
                size_t,sigsetsize) {
 struct sighand *hand = THIS_SIGHAND;
 struct sigaction *action;
 errno_t error = -EINVAL;
 if (sigsetsize != sizeof(sigset_t)) goto end;
 if (sig == SIGKILL || sig == SIGSTOP ||
     sig <= 0 || --sig >= (_NSIG-1)) goto end;
 error = -EOK;
 action = &hand->sh_actions[sig];
 /* Copy the old signal action to user-space. */
 if (oact && copy_to_user(oact,action,sizeof(struct sigaction)))
     return -EFAULT;
 if (act) {
  struct sigaction new_action; pflag_t was;
  if (copy_from_user(&new_action,act,sizeof(struct sigaction)))
      return -EFAULT;
  was = PREEMPTION_PUSH();
  /* Make sure never to unmask KILL or STOP */
  sigdelset(&new_action.sa_mask,SIGKILL);
  sigdelset(&new_action.sa_mask,SIGSTOP);
  /* Add the signal itself to the list of signals
   * being masked during execution of the handler. */
  if (!(new_action.sa_flags&SA_NODEFER))
        sigaddset(&new_action.sa_mask,sig+1);
  /* Now load the new action into the kernel. */
  memcpy(action,&new_action,sizeof(struct sigaction));
  PREEMPTION_POP(was);

  /*
   * POSIX 3.3.1.3:
   *  "Setting a signal action to SIG_IGN for a signal that is
   *   pending shall cause the pending signal to be discarded,
   *   whether or not it is blocked."
   *
   *  "Setting a signal action to SIG_DFL for a signal that is
   *   pending and whose default action is to ignore the signal
   *   (for example, SIGCHLD), shall cause the pending signal to
   *   be discarded, whether or not it is blocked"
   */
  if (new_action.sa_handler == SIG_IGN ||
     (new_action.sa_handler == SIG_DFL &&
      default_actions[sig] == DA_IGN)) {
   struct task *t = THIS_TASK;
   /* Discard pending signals. */
   task_crit();
   sigpending_discard(&t->t_sigpend,sig+1);
   sigpending_discard(&t->t_sigshare->ss_pending,sig+1);
   task_endcrit();
  }
 }
end:
 return error;
}

PUBLIC SAFE errno_t KCALL
task_check_signals(sigset_t *__restrict changes) {
 siginfo_t return_info; ssize_t num_signals = 0;
 struct sigqueue *return_signal; struct task *t = THIS_TASK;
 sigword_t *iter,*end,*dst;
 for (;;) {
  bool share_write_lock,local_write_lock = false;
  sigpending_read(&t->t_sigpend);
  while ((return_signal = sigpending_try_dequeue_unlocked
         (&t->t_sigpend,changes,&local_write_lock)) == E_PTR(-ERELOAD));
  if (!return_signal) {
   share_write_lock = false;
   sigpending_read(&t->t_sigshare->ss_pending);
   do return_signal = sigpending_try_dequeue_unlocked(&t->t_sigshare->ss_pending,
                                                      changes,&share_write_lock);
   while (return_signal == E_PTR(-ERELOAD));
   if (local_write_lock)
        sigpending_endwrite(&t->t_sigshare->ss_pending);
   else sigpending_endread(&t->t_sigshare->ss_pending);
  }
  if (local_write_lock)
       sigpending_endwrite(&t->t_sigpend);
  else sigpending_endread(&t->t_sigpend);
  if (!return_signal) break;
  /* Managed to deque a return signal. - Handle it now. */
  if (SIGPENDING_IS_UNDEFINED(return_signal)) {
   memset(&return_info,0,sizeof(siginfo_t));
   return_info.si_signo = SIGPENDING_GT_UNDEFINED((uintptr_t)return_signal);
   return_info.si_code  = SI_USER;
  } else {
   memcpy(&return_info,&return_signal->sq_info,sizeof(siginfo_t));
   free(return_signal);
  }
  /* Assert this to prevent an infinite loop. */
  assertf(sigismember(changes,return_info.si_signo),
          "signal %d wasn't changed",return_info.si_signo);
  assertf(!sigismember(&t->t_sigblock,return_info.si_signo),
          "Cannot deque signal %d currently being blocked",
          return_info.si_signo);
  task_kill2(t,&return_info,0,0);
  ++num_signals;

  /* Must unmask all potentially changed signals based on what
   * is being in the calling thread was the last signal was raised. */
  end = (iter = (sigword_t *)&t->t_sigblock)+SIGSET_WORDS;
  dst = (sigword_t *)changes;
  for (; iter != end; ++iter,++dst) *dst &= ~*iter;
 }
 return num_signals ? -EINTR : -EOK;
}

SYSCALL_DEFINE4(sigprocmask,int,how,USER sigset_t const *,set,
                USER sigset_t *,oldset,size_t,sigsetsize) {
 struct task *t = THIS_TASK; errno_t error = -EOK;
 sigset_t new_set; byte_t *dst,*end,*src;
 if (sigsetsize > sizeof(sigset_t)) return -EINVAL;
 if (set) {
  if (copy_from_user(&new_set,set,sigsetsize))
      return -EFAULT;
  sigdelset(&new_set,SIGKILL);
  sigdelset(&new_set,SIGSTOP);
 }
 task_crit();
 if (oldset &&
     copy_to_user(oldset,&t->t_sigblock,
                  sigsetsize))
 { error = -EFAULT; goto end; }
 if (set) {
  dst = (byte_t *)&t->t_sigblock;
  end = dst+sigsetsize;
  src = (byte_t *)&new_set;
  switch (how) {

   /* Simple case: Add to blocking mask. */
  case SIG_BLOCK:
   for (; dst != end; ++dst,++src) *dst |= *src;
   break;

   /* Difficult: Must return to the first signal that gets unblocked. */
  case SIG_UNBLOCK:
   /* NOTE: Store a mask of all changes in `src' */
   for (; dst != end; ++dst,++src) {
    byte_t changed = *dst & *src;
    *dst &= ~*src;
    *src = changed;
   }
deque_changes:
   error = task_check_signals(&new_set);
   break;

   /* Difficult: Must return to the first signal that gets unblocked. */
  case SIG_SETMASK:
   /* NOTE: Store a mask of all changes in `src' */
   for (; dst != end; ++dst,++src) if (*dst != *src) {
    byte_t changed = *dst & ~*src;
    *dst = *src;
    *src = changed;
   }
   goto deque_changes;

  default:
   error = -EINVAL;
   break;
  }
 }
end:
 task_endcrit();
 return error;
}



PUBLIC errno_t KCALL
task_set_sigblock(sigset_t *__restrict newset) {
 sigword_t *dst,*end,*src; errno_t error;
 dst = (sigword_t *)&THIS_TASK->t_sigblock;
 end = dst+SIGSET_WORDS;
 src = (sigword_t *)newset;
 for (; dst != end; ++dst,++src) if (*dst != *src) {
  sigword_t changed = *dst & ~*src;
  *dst = *src;
  *src = changed;
 }
 COMPILER_WRITE_BARRIER();
 task_crit();
 error = task_check_signals(newset);
 task_endcrit();
 return error;
}



SYSCALL_DEFINE4(sigtimedwait,USER sigset_t const *,uthese,USER siginfo_t *,uinfo,
                USER struct timespec const *,uts,size_t,sigsetsize) {
 sigset_t wait_mask,old_block; jtime_t timeout = JTIME_INFINITE;
 struct task *t = THIS_TASK; errno_t error; struct sigqueue *signal;
 bool local_write_lock,share_write_lock,is_blocking = false;
 if (sigsetsize > sizeof(sigset_t)) return -EINVAL;
 memset((u8 *)&wait_mask+sigsetsize,0,sizeof(sigset_t)-sigsetsize);
 if (copy_from_user(&wait_mask,uthese,sigsetsize))
     return -EFAULT;
 if (uts) {
  struct timespec tmo;
  if (copy_from_user(&tmo,uts,sizeof(struct timespec)))
      return -EFAULT;
  timeout = jiffies+TIMESPEC_OFF_TO_JIFFIES(tmo);
 }
 /* Make sure never to unmask these signals! */
 sigdelset(&wait_mask,SIGKILL);
 sigdelset(&wait_mask,SIGSTOP);
 task_crit();
scan_again:
 local_write_lock = false;
 share_write_lock = false;
 sigpending_read(&t->t_sigpend);
scan_again_local:
 do signal = sigpending_try_dequeue_unlocked(&t->t_sigpend,
                                             &wait_mask,&local_write_lock);
 while (signal == E_PTR(-ERELOAD));
 if (!signal) {
  sigpending_read(&t->t_sigshare->ss_pending);
scan_again_shared:
  do signal = sigpending_try_dequeue_unlocked(&t->t_sigshare->ss_pending,
                                              &wait_mask,&share_write_lock);
  while (signal == E_PTR(-ERELOAD));
  if (!signal) {
   /* Wait for more signals to arrive.
    * NOTE: Make sure we've got a write-lock on both pending controllers. */
   struct sig *wait_error;
   if (!local_write_lock) {
    local_write_lock = true;
    if (!sigpending_upgrade(&t->t_sigpend)) {
     /* Make sure to release the share-lock before reloading. */
     if (share_write_lock) share_write_lock = false,
          sigpending_endwrite(&t->t_sigshare->ss_pending);
     else sigpending_endread(&t->t_sigshare->ss_pending);
     goto scan_again_local;
    }
   }
   if (!share_write_lock &&
       !sigpending_upgrade(&t->t_sigshare->ss_pending))
        goto scan_again_shared;
   /* At this point we've confirmed that no signal is available
    * in either of the pending controllers, meaning now we have
    * to wait for it to arrive.
    * >> For this, we must also make sure that the intended signals
    *    will not execute handlers, but rather be scheduled as
    *    pending instead. */
   if (!is_blocking) {
    sigword_t *dst,*end,*src;
    /* Make sure to create a backup of the blocking-set, to-be restored later. */
    memcpy(&old_block,&t->t_sigblock,sizeof(sigset_t));
    /* Add all signals that we're waiting for to the blocking set. */
    dst = (sigword_t *)&THIS_TASK->t_sigblock;
    end = dst+SIGSET_WORDS;
    src = (sigword_t *)&wait_mask;
    for (; dst != end; ++dst,++src) *dst |= *src;
    is_blocking = true;
   }

   /* Wait for both signals at once. */
   task_addwait(&t->t_sigpend.sp_newsig,NULL,0);
   task_addwait(&t->t_sigshare->ss_pending.sp_newsig,NULL,0);

   /* TODO: This is totally unnecessary. - The way the new scheduler has
    *       been implemented, we can simply lock + `task_addwait()' + unlock
    *       any number of signals consecutively and have `task_waitfor()'
    *       return a pointer to the pending set to which a new signal was
    *       added.
    * >> Thus what we're currently doing by locking both sets
    *    at the same time is both wasteful and unnecessary! */
   sig_endwrite(&t->t_sigpend.sp_newsig);
   sig_endwrite(&t->t_sigshare->ss_pending.sp_newsig);

   wait_error = task_waitfor(timeout);
   if (E_ISERR(wait_error)) { error = E_GTERR(wait_error); goto end; }
   goto scan_again;
  }
  if (share_write_lock)
       sigpending_endwrite(&t->t_sigshare->ss_pending);
  else sigpending_endread(&t->t_sigshare->ss_pending);
 }
 if (local_write_lock)
      sigpending_endwrite(&t->t_sigpend);
 else sigpending_endread(&t->t_sigpend);
 /* We've managed to find a signal! */
 assert(signal && signal != E_PTR(-ERELOAD));
 error = -EOK;
 /* Copy the collected signal information to user-space and free the signal buffer. */
 if (SIGPENDING_IS_UNDEFINED(signal)) {
  if (uinfo) {
   siginfo_t return_info;
   memset(&return_info,0,sizeof(return_info));
   return_info.si_signo = SIGPENDING_GT_UNDEFINED((uintptr_t)signal);
   return_info.si_code  = SI_USER;
   if (copy_to_user(uinfo,&return_info,sizeof(siginfo_t)))
       error = -EFAULT;
  }
 } else {
  if (uinfo && copy_to_user(uinfo,&signal->sq_info,sizeof(siginfo_t)))
      error = -EFAULT;
  free(signal);
 }
end:
 task_endcrit();
 /* Restore the old blocking configuration, in the process handling
  * any additional signals that were not caught by the wait(). */
 if (is_blocking)
     task_set_sigblock(&old_block);
 return error;
}

SYSCALL_DEFINE2(sigpending,USER sigset_t *,uset,size_t,sigsetsize) {
 sigset_t set; sigword_t *dst,*end,*src;
 struct task *t = THIS_TASK;
 if unlikely(sigsetsize > sizeof(sigset_t))
    return -EINVAL;
 task_crit();
 sigpending_read(&t->t_sigpend);
 memcpy(&set,&t->t_sigpend.sp_mask,sizeof(sigset_t));
 sigpending_endread(&t->t_sigpend);
 sigpending_read(&t->t_sigshare->ss_pending);
 end = (dst = (sigword_t *)&set)+SIGSET_WORDS;
 src = (sigword_t *)&t->t_sigshare->ss_pending.sp_mask;
 for (; dst != end; ++dst,++src) *dst |= *src;
 sigpending_endread(&t->t_sigshare->ss_pending);
 task_endcrit();
 if (copy_to_user(uset,&set,sigsetsize))
     return -EFAULT;
 return -EOK;
}


PRIVATE ssize_t KCALL
killgrp_info(struct task *__restrict leader,
             siginfo_t const *__restrict info) {
 struct task *iter; errno_t temp;
 size_t result = 1,count = 0; REF struct task **buf,**dst;
 /* First off: Kill the leader in case the signal needs
  *            to prevent more children from spawning. */
 temp = TASK_KILL2_IFOK(leader,info,0,0);
 if (!E_ISOK(temp)) return (ssize_t)temp;
 atomic_rwlock_read(&leader->t_pid.tp_grouplock);
 for (iter = leader->t_pid.tp_group; iter;
      iter = iter->t_pid.tp_grplink.le_next)
      if (iter != leader) ++count;
 buf = tmalloc(REF struct task *,count);
 if (!buf) {
  result = -ENOMEM;
  atomic_rwlock_endread(&leader->t_pid.tp_grouplock);
 } else {
  for (iter = leader->t_pid.tp_group,dst = buf; iter;
       iter = iter->t_pid.tp_grplink.le_next)
       if (iter != leader && TASK_TRYINCREF(iter)) *dst++ = iter;
  atomic_rwlock_endread(&leader->t_pid.tp_grouplock);
  /* Now kill all tasks apart of the group. */
  temp = -EOK;
  for (; dst != buf; --dst) {
   iter = *dst;
   if (E_ISOK(temp))
       temp = TASK_KILL2_IFOK(iter,info,0,0);
   TASK_DECREF(iter);
   ++result;
  }
  free(buf);
  /* If we've failed to kill something, return the error that caused this. */
  if (!E_ISOK(temp))
       return temp;
 }
 return result;
}

PRIVATE SAFE ssize_t KCALL
killpid_info(pid_t pid, siginfo_t const *__restrict info) {
 REF struct task *target;
 ssize_t result;
 if (pid > 0) {
  /* Kill The process matching `pid'. */
  target = pid_namespace_lookup(THIS_NAMESPACE,pid);
  if (!target) result = -ESRCH;
  else {
   result = TASK_KILL2_IFOK(target,info,0,0);
   TASK_DECREF(target);
   if (E_ISOK(result))
       result = 1;
  }
 } else if (pid == 0) {
  REF struct task *leader;
  /* Kill all processes in the calling thread's process group. */
  atomic_rwlock_read(&THIS_TASK->t_pid.tp_leadlock);
  leader = THIS_TASK->t_pid.tp_leader;
  TASK_INCREF(leader);
  atomic_rwlock_endread(&THIS_TASK->t_pid.tp_leadlock);
  result = killgrp_info(leader,info);
  TASK_DECREF(leader);
 } else if (pid == -1) {
  /* TODO: Kill all processes that we're allowed to. */
 } else {
  /* Kill the entirety of a process group. */
  target = pid_namespace_lookup(THIS_NAMESPACE,-pid);
  if (!target) result = -ESRCH;
  else {
   result = killgrp_info(target,info);
   TASK_DECREF(target);
  }
 }
 return result;
}

SYSCALL_DEFINE2(kill,pid_t,pid,int,sig) {
 siginfo_t info; ssize_t result;
 info.si_signo = sig;
 info.si_errno = 0;
 info.si_code  = SI_USER;
 task_crit();
 info.si_pid   = GET_THIS_PID();
 info.si_uid   = GET_THIS_UID();
 result = killpid_info(pid,&info);
 task_endcrit();

 /* NOTE: On success (At least one signal was sent), ZERO(0) is returned. */
 if (result > 1) result = 0;
 return result;
}

PRIVATE errno_t KCALL
tkill_info(pid_t tgid, pid_t pid, siginfo_t *__restrict info) {
 REF struct task *t;
 errno_t result = -ESRCH;
 /* Make sure we've been given a valid signal number. */
 if (info->si_signo < 0 || info->si_signo >= _NSIG)
     return -EINVAL;
 if ((t = pid_namespace_lookup(THIS_NAMESPACE,pid)) != NULL) {
  if (tgid <= 0 || TASK_GETTGID(t) == tgid) {
   result = task_kill2_ok(t,info);
   /* NOTE: Don't deliver signal `0'. - It's used to test access. */
   if (E_ISOK(result) && info->si_signo != 0)
       result = task_kill2(t,info,0,0);
  }
  TASK_DECREF(t);
 }
 return result;
}
PRIVATE errno_t KCALL
do_tkill(pid_t tgid, pid_t pid, int sig) {
 siginfo_t info; errno_t result;
 info.si_signo = sig;
 info.si_errno = 0;
 info.si_code  = SI_TKILL;
 task_crit();
 info.si_pid   = GET_THIS_PID();
 info.si_uid   = GET_THIS_UID();
 result = tkill_info(tgid,pid,&info);
 task_endcrit();
 return result;
}

SYSCALL_DEFINE3(tgkill,pid_t,tgid,pid_t,pid,int,sig) {
 if (pid <= 0 || tgid <= 0) return -EINVAL;
 return do_tkill(tgid,pid,sig);
}
SYSCALL_DEFINE2(tkill,pid_t,pid,int,sig) {
 if (pid <= 0) return -EINVAL;
 return do_tkill(0,pid,sig);
}

SYSCALL_DEFINE2(sigsuspend,USER sigset_t const *,unewset,size_t,sigsetsize) {
 sigset_t newset,oldset; errno_t result,temp;
 struct task *t = THIS_TASK;
 if (sigsetsize > sizeof(sigset_t))
     return -EINVAL;
 if (copy_from_user(&newset,unewset,sigsetsize))
     return -EFAULT;
 memset((byte_t *)&newset+sigsetsize,0,
         sizeof(sigset_t)-sigsetsize);
 memcpy(&oldset,&t->t_sigblock,sizeof(sigset_t));
 /* Make sure never to block these two... */
 sigdelset(&newset,SIGKILL);
 sigdelset(&newset,SIGSTOP);

 /* Time to do this! */
 task_crit();

 /* Set the new signal mask. */
 result = task_set_sigblock(&newset);
 if (E_ISERR(result)) goto end;

 /* Wait until we're sent a signal we can handle. */
 { struct sig *s = task_waitfor(JTIME_INFINITE); /* XXX: Timeout? */
   assert(E_ISERR(s));
   result = E_GTERR(s);
 }

 /* Restore the old signal mask. */
 temp = task_set_sigblock(&oldset);
 if (E_ISOK(result)) result = temp;
end:
 task_endcrit();
 return result;
}


SYSCALL_DEFINE2(sigaltstack,USER stack_t const *,new_stack,
                            USER stack_t *,old_stack) {
 stack_t stack;
 struct task *caller = THIS_TASK;
 /* No need for locking, because all data is effectively accessed weakly. */
 if (old_stack) {
  stack.ss_sp    = caller->t_sigstack.ss_base;
  stack.ss_size  = caller->t_sigstack.ss_size;
  stack.ss_flags = !stack.ss_size
                  ? SS_DISABLE
                  : KERNEL_SIGSTACK_CONTAINS(caller->t_sigstack,
                                             IRREGS_SYSCALL_GET()->userxsp)
                  ? SS_ONSTACK
                  : 0;
  if (copy_to_user(old_stack,&stack,sizeof(stack_t)))
      return -EFAULT;
 }
 if (new_stack) {
  if (copy_from_user(&stack,new_stack,sizeof(stack_t)))
      return -EFAULT;
  if (!stack.ss_size) stack.ss_sp = NULL;
  caller->t_sigstack.ss_base = stack.ss_sp;
  caller->t_sigstack.ss_size = stack.ss_size;
 }
 return -EOK;
}

/*
#define __NR_sigqueueinfo 138
__SYSCALL(__NR_sigqueueinfo,sys_sigqueueinfo)
*/

DECL_END

#ifndef __INTELLISENSE__
#include "sighand.c.inl"
#endif
#endif /* !CONFIG_NO_SIGNALS */


#endif /* !GUARD_KERNEL_SCHED_SIGNAL_C */
