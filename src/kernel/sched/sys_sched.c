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
#ifndef GUARD_KERNEL_SCHED_SYS_SCHED_C
#define GUARD_KERNEL_SCHED_SYS_SCHED_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <bits/resource.h>
#include <bits/sched.h>
#include <bits/waitflags.h>
#include <bits/waitstatus.h>
#include <fs/fd.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <sched.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/percpu.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <sched/types.h>
#include <string.h>
#include <alloca.h>

DECL_BEGIN

GLOBAL_ASM(
L(.section .text                         )
L(INTERN_ENTRY(sys_exit)                 )
L(    pushl %ebx                         )
L(    pushl ASM_CPU(CPU_OFFSETOF_RUNNING))
L(    call task_terminate                )
#ifdef CONFIG_DEBUG
L(.global __assertion_unreachable        )
L(    call __assertion_unreachable       )
#endif
L(SYM_END(sys_exit)                      )
L(.previous                              )
);

GLOBAL_ASM(
L(.section .text                         )
L(INTERN_ENTRY(sys_sched_yield)          )
L(    call task_yield                    )
L(    xorl %eax, %eax                    )
L(    iret                               )
L(SYM_END(sys_sched_yield)               )
L(.previous                              )
);


SYSCALL_DEFINE1(unshare,int,flags) {
 errno_t error = -EOK;
 task_crit();
 if ((flags&CLONE_VM) && (error = task_unshare_mman(true),E_ISERR(error))) goto end;
 if ((flags&CLONE_FILES) && (error = task_unshare_fdman(),E_ISERR(error))) goto end;
 if ((flags&CLONE_SIGHAND) && (error = task_unshare_sighand(),E_ISERR(error))) goto end;
 /* TODO: All the other things? */
end:
 task_endcrit();
 return error;
}


INTDEF errno_t KCALL
mman_init_copy_unlocked(struct mman *__restrict nm,
                        struct mman *__restrict om);
INTDEF errno_t KCALL
fdman_duplicate_table_unlocked(struct fdman *__restrict dst,
                               struct fdman *__restrict src);


PRIVATE REF struct task *KCALL task_do_fork(void) {
 struct task *caller = THIS_TASK;
 REF struct task *result; errno_t error;
 REF struct stack *ustack = NULL;
 REF struct fdman *nfd;
 REF struct mman *nm;
 struct mman *om;
 error = -ENOMEM;

 TASK_PDIR_KERNEL_BEGIN(om);

 /* First up: Allocate the new task controller. */
 if unlikely((result = task_new()) == NULL) goto err0;
 if unlikely((nm = result->t_mman = mman_new()) == NULL) goto err1;
 if unlikely((nfd = result->t_fdman = fdman_new()) == NULL) goto err1;
 if unlikely((result->t_sighand = sighand_copy(THIS_TASK->t_sighand)) == NULL) goto err1;
 if unlikely((result->t_sigshare = sigshare_new()) == NULL) goto err1;

 /* Setup child process ids. */
 if ((error = task_set_parent(result,THIS_TASK),E_ISERR(error))) goto err1;
 if ((error = task_set_leader(result,result),E_ISERR(error))) goto err1;
 if ((error = task_set_id(result,&pid_global),E_ISERR(error))) goto err1;
 if ((error = task_set_id(result,THIS_NAMESPACE),E_ISERR(error))) goto err1;

 assertf(om != &mman_kernel,"No... You can't fork() the kernel itself...");
 if (E_ISERR(error = mman_write(om))) goto err1;
 if (E_ISERR(error = mman_write(nm))) { mman_endwrite(om); goto err1; }

 /* Start with the simple stuff: Clone the page directory. */
 if (!pdir_load_copy(&nm->m_pdir,&om->m_pdir)) { error = -ENOMEM; goto end_double_lock; }

 /* Duplicate the memory map and setup copy-on-write for memory branches.
  * HINT: This function also handle duplication of module instances. */
 error = mman_init_copy_unlocked(nm,om);
 if (E_ISERR(error)) goto end_double_lock;


#ifndef CONFIG_NO_LDT
 { ldt_t *ldt_vec,*iter; size_t ldt_cnt = 0;
   struct task *ldt_user;
   ldt_read(nm->m_ldt);
   LDT_FOREACH_TASK(ldt_user,nm->m_ldt) {
    if (ldt_user != caller) ++ldt_cnt;
   }
   if (ldt_cnt) {
    /* Must delete the LDT entires of all of these tasks. */
    ldt_vec = (ldt_t *)amalloc(ldt_cnt*sizeof(ldt_t));
    if unlikely(!ldt_vec) { error = -ENOMEM; goto end_double_lock; }
    iter = ldt_vec;
    LDT_FOREACH_TASK(ldt_user,nm->m_ldt) {
     if (ldt_user != caller) {
      assert(iter < ldt_vec+ldt_cnt);
      /* XXX: You're relying on the fact that other tasks running
       *      in the calling process won't decide to change their
       *      TLS segment. - This is wrong: they are allowed to... */
      *iter++ = ATOMIC_READ(ldt_user->t_arch.at_ldt_tls);
     }
    }
    ldt_endread(nm->m_ldt);
    assert(iter == ldt_vec+ldt_cnt);
    iter = ldt_vec+ldt_cnt;
    /* Delete all LDT entries used for TLS by tasks
     * that won't be apart of the new process. */
    while (iter-- != ldt_vec) {
     if (*iter != LDT_ERROR)
          mman_delldt_unlocked(nm,*iter);
    }
    afree(ldt_vec);
   } else {
    ldt_endread(nm->m_ldt);
   }
 }
 /* Simply inherit the TLS segment index used by the returned task. */
 result->t_arch.at_ldt_tls = caller->t_arch.at_ldt_tls;
#endif

 //pdir_print(&om->m_pdir,&syslog_printer,SYSLOG_PRINTER_CLOSURE(0));

 /* That's already it for the mman! - We've copied the all of user-space memory!
  * >> At this point, all we still need to do, is to create a new stack-controller
  *    for the resulting thread, and update its mapping callback in the new
  *    memory manager. */
 if (caller->t_ustack) {
  ustack = (REF struct stack *)memdup(caller->t_ustack,
                                      sizeof(struct stack));
  if unlikely(!ustack) { error = -ENOMEM; goto end_double_lock; }
 }
end_double_lock:
 //syslog(LOG_CONFIRM,"OLD PAGEDIRECTORY\n");
 //pdir_print(&om->m_pdir,&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_CONFIRM));
 //syslog(LOG_CONFIRM,"NEW PAGEDIRECTORY\n");
 //pdir_print(&nm->m_pdir,&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_CONFIRM));
 //syslog(LOG_CONFIRM,"--END\n");

 mman_endwrite(om);
 if (E_ISERR(error)) { mman_endwrite(nm); goto err1; }

 /* Now to relocate the user-stack. */
 assert((caller->t_ustack != NULL) == (ustack != NULL));
 if (ustack) {
  struct mbranch *branch;
#if 0
  syslog(LOG_DEBUG,"Duplicating stack: %p -> %p (%p...%p | %p)\n",
         caller->t_ustack,ustack,ustack->s_begin,(uintptr_t)ustack->s_end-1,
         THIS_SYSCALL_USERESP);
#endif
  ustack->s_refcnt = 1; /* The reference owned by 'result->t_ustack' */
  ustack->s_branch = 0;
  MMAN_FOREACH(branch,nm) {
   if (branch->mb_notify  != &stack_mnotify ||
       branch->mb_closure != caller->t_ustack) continue;
   asserte(ATOMIC_DECFETCH(caller->t_ustack->s_branch) >= 1);
   branch->mb_closure = ustack;
   ++ustack->s_branch;
  }
  if (ustack->s_branch) ++ustack->s_refcnt;
  result->t_ustack = ustack; /* Inherit reference. */
 }
 mman_endwrite(nm);

 /* Now just copy the file descriptor table. */
 error = fdman_read(caller->t_fdman);
 if (E_ISERR(error)) goto err1;
 error = fdman_duplicate_table_unlocked(nfd,caller->t_fdman);
 fdman_endread(caller->t_fdman);
 if (E_ISERR(error)) goto err1;

 /* Alright! - User-space memory & files have been fully set up.
  * >> Now to create a new kernel-stack for 'result'. */
 error = task_mkhstack(result,TASK_HOSTSTACK_DEFAULTSIZE);
 if (E_ISERR(error)) goto err1;

 /* XXX: Assign a new PID? */

 /* NOTE: From this point onwards, all remaining setup operation are noexcept! */

 /* Inherit CPU-affinity from the calling thread. */
 atomic_rwlock_read(&caller->t_affinity_lock);
 memcpy(&result->t_affinity,&caller->t_affinity,sizeof(cpu_set_t));
 atomic_rwlock_endread(&caller->t_affinity_lock);

 /* Inherit thread-priority from the calling thread. */
 result->t_priority = ATOMIC_READ(caller->t_priority);

 /* The last step now, is to set up how the new process should
  * return to user-space. (i.e.: we need to set up its registers) */
 { HOST struct cpustate *cs;
   result->t_cstate = cs = ((HOST struct cpustate *)result->t_hstack.hs_end-1);
   /* Inherit practically all registers from the calling thread. */
#undef __STACKBASE_TASK
#define __STACKBASE_TASK caller
#ifdef __i386__
   cs->ss          = THIS_SYSCALL_SS;
   cs->_n2         = 0;
   cs->useresp     = THIS_SYSCALL_USERESP;
   cs->host.eax    = 0; /* The child process returns ZERO(0) in EAX. */
   cs->host.ecx    = THIS_SYSCALL_ECX;
   cs->host.edx    = THIS_SYSCALL_EDX;
   cs->host.ebx    = THIS_SYSCALL_EBX;
   cs->host.esi    = THIS_SYSCALL_ESI;
   cs->host.edi    = THIS_SYSCALL_EDI;
   cs->host.ebp    = THIS_SYSCALL_EBP;
   cs->host.eip    = (u32)THIS_SYSCALL_EIP;
   cs->host.cs     = THIS_SYSCALL_CS;
   cs->host._n1    = 0;
   cs->host.eflags = THIS_SYSCALL_EFLAGS;
   __asm__ __volatile__("movw %%gs, %0\n" : "=g" (cs->host.gs));
   __asm__ __volatile__("movw %%fs, %0\n" : "=g" (cs->host.fs));
   __asm__ __volatile__("movw %%es, %0\n" : "=g" (cs->host.es));
   __asm__ __volatile__("movw %%ds, %0\n" : "=g" (cs->host.ds));
#if 0
   syslog(LOG_DEBUG,"FORK at %p\n",cs->host.eip);
   syslog(LOG_DEBUG,"EAX %p  ECX %p  EDX %p  EBX %p EFLAGS %p\n",
                     cs->host.eax,
                     cs->host.ecx,
                     cs->host.edx,
                     cs->host.ebx,cs->host.eflags);
   syslog(LOG_DEBUG,"ESP %p  EBP %p  ESI %p  EDI %p\n",
                     cs->host.eip,cs->useresp,
                     cs->host.ebp,
                     cs->host.esi,
                     cs->host.edi);
#endif
#else
#error FIXME
#endif
#undef __STACKBASE_TASK
#define __STACKBASE_TASK THIS_TASK
 }

 /* And we're done! */
done:
 TASK_PDIR_KERNEL_END(om);
 return result;
err1: TASK_DECREF(result);
err0: result = E_PTR(error);
 goto done;
}


SYSCALL_DEFINE0(fork) {
 pid_t result;
 REF struct task *child;
 task_crit();
 child = task_do_fork();
 if (E_ISOK(child)) {
  result = task_start(child);
  /* Return the pid of 'result'. */
  if (E_ISOK(result))
      result = child->t_pid.tp_ids[PIDTYPE_PID].tl_pid;
  /* Drop the reference returned by 'task_do_fork()' */
  TASK_DECREF(child);
 } else {
  result = E_GTERR(child);
 }
 task_endcrit();
 assert(!THIS_TASK->t_critical);
 return result;
}

PRIVATE SAFE errno_t KCALL
set_process_group(struct task *__restrict t,
                  struct task *__restrict new_leader) {
 REF struct task *old_leader = NULL;
 CHECK_HOST_DOBJ(t);
 CHECK_HOST_DOBJ(new_leader);
 if unlikely(new_leader->t_flags&TASKFLAG_NOTALEADER)
    return -ESRCH;

 atomic_rwlock_write(&t->t_pid.tp_leadlock);
 if (t->t_pid.tp_leader != new_leader) {
  old_leader = t->t_pid.tp_leader; /* Inherit reference. */

  /* Update the task group links. */
  atomic_rwlock_write(&old_leader->t_pid.tp_grouplock);
  if (THREAD_PID_ISGROUP(&old_leader->t_pid,t))
      THREAD_PID_DELGROUP(&old_leader->t_pid,t);
  atomic_rwlock_endwrite(&old_leader->t_pid.tp_grouplock);

  COMPILER_WRITE_BARRIER();
  if (new_leader != t) TASK_INCREF(new_leader);
  t->t_pid.tp_leader = new_leader; /* Inherit reference. */
  COMPILER_WRITE_BARRIER();

  atomic_rwlock_write(&new_leader->t_pid.tp_grouplock);
  THREAD_PID_ADDGROUP(&new_leader->t_pid,t);
  atomic_rwlock_endwrite(&new_leader->t_pid.tp_grouplock);
 }
 atomic_rwlock_endwrite(&t->t_pid.tp_leadlock);
 if (old_leader && old_leader != t) TASK_DECREF(old_leader);
 return -EOK;
}

SYSCALL_DEFINE0(getpid) { pid_t result; task_crit(); result = GET_THIS_PID(); task_endcrit(); return result; }
SYSCALL_DEFINE0(getppid) { pid_t result; task_crit(); result = GET_THIS_PPID(); task_endcrit(); return result; }
SYSCALL_DEFINE0(gettid) { return GET_THIS_TID(); }
SYSCALL_DEFINE2(setpgid,pid_t,pid,pid_t,pgid) {
 struct task *target,*leader; errno_t result = -ESRCH;
 if (pid < 0 || pgid < 0) return -EINVAL;
 task_crit();

 /* Lookup task information. */
 if (!pid) target = THIS_TASK;
 else if ((target = pid_namespace_lookup(THIS_NAMESPACE,pid)) == NULL) goto done;
 if (!pgid) leader = target;
 else if ((leader = pid_namespace_lookup(THIS_NAMESPACE,pgid)) == NULL) goto done2;

 /* Set the new process group. */
 result = set_process_group(target,leader);

       if (pgid) TASK_DECREF(leader);
done2: if (pid) TASK_DECREF(target);
done:  task_endcrit();
 return result;
}
SYSCALL_DEFINE1(getpgid,pid_t,pid) {
 WEAK REF struct task *t;
 pid_t result = -ESRCH;
 if unlikely(pid < 0) return -EINVAL;
 task_crit();
 if (pid == 0) result = GET_THIS_PGID();
 else if ((t = pid_namespace_lookup_weak(THIS_NAMESPACE,pid)) != NULL) {
  result = TASK_GETPGID(t);
  TASK_WEAK_DECREF(t);
 }
 task_endcrit();
 return result;
}


PRIVATE errno_t KCALL
get_rusage(struct task *__restrict t,
           USER struct rusage *ru) {
 struct rusage info;
 memset(&info,0,sizeof(struct rusage));

 /* XXX: Usage information? */

 if (copy_to_user(ru,&info,sizeof(struct rusage)))
     return -EFAULT;
 return -EOK;
}

PRIVATE pid_t KCALL
do_try_single(struct task *__restrict t, int options,
              USER int *stat_addr, USER siginfo_t *infop,
              USER struct rusage *ru) {
 errno_t error;
 if (options&(WSTOPPED|WCONTINUED) &&
     ATOMIC_FETCHAND(t->t_flags,~TASKFLAG_SIGSTOPCONT)&
                                 TASKFLAG_SIGSTOPCONT) {
  siginfo_t info; /* stop/continue was signaled. */
  memset(&info,0,sizeof(siginfo_t));
  info.si_signo  = SIGCHLD;
  info.si_code   = CLD_STOPPED;
  info.si_status = __W_STOPCODE(SIGSTOP); /* TODO: Return the signal number that caused the stop. */;
  info.si_pid    = t->t_pid.tp_ids[PIDTYPE_PID].tl_ns == THIS_NAMESPACE
                 ? t->t_pid.tp_ids[PIDTYPE_PID].tl_pid : 0;
  info.si_uid    = 0; /* ??? */
  if (infop && copy_to_user(infop,&info,sizeof(siginfo_t)))
      return -EFAULT;
  if (stat_addr && copy_to_user(stat_addr,&info.si_status,sizeof(int)))
      return -EFAULT;
  if (ru && (error = get_rusage(t,ru),E_ISERR(error))) return error;
  return info.si_pid;
 }
 if (options&WEXITED &&
     ATOMIC_READ(t->t_mode) == TASKMODE_TERMINATED) {
  siginfo_t info; /* exit was signaled. */
  memset(&info,0,sizeof(siginfo_t));
  info.si_signo  = SIGCHLD;
  info.si_code   = CLD_STOPPED;
  info.si_status = (int)(uintptr_t)t->t_exitcode;
  info.si_pid    = t->t_pid.tp_ids[PIDTYPE_PID].tl_ns == THIS_NAMESPACE
                 ? t->t_pid.tp_ids[PIDTYPE_PID].tl_pid : 0;
  info.si_uid    = 0; /* ??? */
  if (infop && copy_to_user(infop,&info,sizeof(siginfo_t)))
      return -EFAULT;
  if (stat_addr && copy_to_user(stat_addr,&info.si_status,sizeof(int)))
      return -EFAULT;

  /* Reap this dead child thread if requested to. */
  if (!(options&WNOWAIT)) {
   struct task *caller = THIS_TASK;
   /* Reap this zombie. */
   assertf(t->t_pid.tp_parent == caller,"This wasn't your child.");
   assert(atomic_rwlock_writing(&caller->t_pid.tp_childlock));
   if (THREAD_PID_ISCHILD(&caller->t_pid,t)) {
    THREAD_PID_DELCHILD(&caller->t_pid,t);
    if (t != caller) TASK_WEAK_DECREF(t);
   }
  }
  if (ru && (error = get_rusage(t,ru),E_ISERR(error))) return error;
  return info.si_pid;
 }

 return -EAGAIN;
}

LOCAL errno_t KCALL
wait_for_sigchld(struct timespec *abstime) {
 errno_t error; bool was_blocked;
 struct task *caller = THIS_TASK;

 /* Make sure to unmask SIGCHLD to guaranty the signal actually being received. */
 was_blocked = sigismember(&caller->t_sigblock,SIGCHLD);
 sigdelset(&caller->t_sigblock,SIGCHLD);
 error = task_pause_cpu_endwrite(abstime);

 /* Restore the old blocking behavior of 'SIGCHLD' */
 if (was_blocked)
     sigaddset(&caller->t_sigblock,SIGCHLD);
 return error;
}

PRIVATE pid_t KCALL
do_wait_single(struct task *__restrict t, int options,
               USER int *stat_addr, USER siginfo_t *infop,
               USER struct rusage *ru, struct timespec *abstime) {
 struct cpu *c; pflag_t was;
 pid_t result = -EOK; bool second_pass = false;
 struct task *caller = THIS_TASK;
 /* Lock our own CPU to prevent signals from being delivered. */
 was = PREEMPTION_PUSH();
again:
 assert(!PREEMPTION_ENABLED());
 for (;;) {
  c = TASK_CPU(caller);
  COMPILER_READ_BARRIER();
  cpu_write(c);
  if (TASK_CPU(caller) == c) break;
  cpu_endwrite(c);
 }
 if (options&WNOWAIT)
      atomic_rwlock_read(&caller->t_pid.tp_childlock);
 else atomic_rwlock_write(&caller->t_pid.tp_childlock);
 result = do_try_single(t,options,stat_addr,infop,ru);
 if (options&WNOWAIT)
      atomic_rwlock_endread(&caller->t_pid.tp_childlock);
 else atomic_rwlock_endwrite(&caller->t_pid.tp_childlock);
 if (result == -EAGAIN) {
  if (options&WNOHANG) { result = -EOK; goto unlock_cpu; }
  if (second_pass) { result = -EINTR; goto unlock_cpu; }
  second_pass = true;
  /* Wait for a child process to deliver a signal. */
  result = task_pause_cpu_endwrite(abstime);
  if (result == -EINTR) goto again;
 } else {
  if (second_pass)
      result = -EINTR;
unlock_cpu:
  cpu_endwrite(c);
 }
 PREEMPTION_POP(was);
 return result;
}



PRIVATE pid_t KCALL
do_wait(int which, /*global*/pid_t pid, int options,
        USER int *stat_addr, USER siginfo_t *infop,
        USER struct rusage *ru, struct timespec *abstime) {
 struct cpu *c; pflag_t was; size_t n_candidates;
 pid_t result = -EOK; bool second_pass = false;
 struct task *child,*caller = THIS_TASK;
 assert(which == P_ALL || which == P_PGID);
 /* Lock our own CPU to prevent signals from being delivered. */
 was = PREEMPTION_PUSH();
again:
 assert(!PREEMPTION_ENABLED());
 for (;;) {
  c = TASK_CPU(caller);
  COMPILER_READ_BARRIER();
  cpu_write(c);
  if (TASK_CPU(caller) == c) break;
  cpu_endwrite(c);
 }
 n_candidates = 0;
 if (options&WNOWAIT)
      atomic_rwlock_read(&caller->t_pid.tp_childlock);
 else atomic_rwlock_write(&caller->t_pid.tp_childlock);
 TASK_CHILDREN_FOREACH(child,caller) {
  /* Check if the task's thread-group id matches the requested group. */
  if (which == P_PGID &&
      THREAD_PID_GLOBAL_GETTGID(&child->t_pid) != pid)
      continue;
  ++n_candidates;
  result = do_try_single(child,options,stat_addr,infop,ru);
  if (result != -EAGAIN) break;
 }
 if (options&WNOWAIT)
      atomic_rwlock_endread(&caller->t_pid.tp_childlock);
 else atomic_rwlock_endwrite(&caller->t_pid.tp_childlock);
 if (!n_candidates) result = -ECHILD;
 if (result == -EAGAIN) {
  if (options&WNOHANG) { result = -EOK; goto unlock_cpu; }
  if (second_pass) { result = -EINTR; goto unlock_cpu; }
  second_pass = true;
  /* Wait for a child process to deliver a signal. */
  result = task_pause_cpu_endwrite(abstime);
  if (result == -EINTR) goto again;
 } else {
unlock_cpu:
  cpu_endwrite(c);
 }
 PREEMPTION_POP(was);
 return result;
}


SYSCALL_DEFINE5(waitid,int,which,pid_t,upid,USER siginfo_t *,
                infop,int,options,USER struct rusage *,ru) {
 WEAK REF struct task *t; int result;
 if (options & ~(WNOHANG|WNOWAIT|WEXITED|WSTOPPED|WCONTINUED) ||
   !(options & (WEXITED|WSTOPPED|WCONTINUED))) return -EINVAL;
 task_crit();
 switch (which) {

 case P_ALL: break;

 case P_PID:
 case P_PGID:
  if (upid <= 0) goto inval;
  t = pid_namespace_lookup_weak(THIS_NAMESPACE,upid);
  if unlikely(!t) { result = -ECHILD; goto end; }
  if (which == P_PID) {
   if (TASK_TRYINCREF(t)) {
    result = do_wait_single(t,options,NULL,infop,ru,NULL);
    TASK_DECREF(t);
    if (E_ISOK(result)) {
     result = 0;
     if (ATOMIC_READ(t->t_pid.tp_parent) == THIS_TASK)
         goto reap_zombie;
    }
   } else if (ATOMIC_READ(t->t_pid.tp_parent) == THIS_TASK) {
    assert(t->t_mode == TASKMODE_TERMINATED);
    /* The child is dead. */
    result = -EOK;
    if (infop) {
     siginfo_t info;
     memset(&info,0,sizeof(siginfo_t));
     info.si_pid    = t->t_pid.tp_ids[PIDTYPE_PID].tl_ns == THIS_NAMESPACE
                    ? t->t_pid.tp_ids[PIDTYPE_PID].tl_pid : 0;
     info.si_signo  = SIGCHLD;
     info.si_status = (int)(uintptr_t)t->t_exitcode;
     if (__WIFSIGNALED(info.si_status))
      info.si_code = CLD_KILLED;
     else if (__WIFSTOPPED(info.si_status))
      info.si_code = CLD_STOPPED;
     else if (__WCOREDUMP(info.si_status))
      info.si_code = CLD_DUMPED;
     else if (__WIFCONTINUED(info.si_status))
      info.si_code = CLD_CONTINUED;
     else info.si_code = CLD_EXITED;
     if (copy_to_user(infop,&info,sizeof(siginfo_t)))
         result = -EFAULT;
    }
reap_zombie:
    if (!(options&WNOWAIT)) {
     /* Reap this thread. */
     struct task *caller = THIS_TASK;
     atomic_rwlock_write(&caller->t_pid.tp_childlock);
     if (THREAD_PID_ISCHILD(&caller->t_pid,t)) {
      THREAD_PID_DELCHILD(&caller->t_pid,t);
      if (t != caller) TASK_WEAK_DECREF(t);
     }
     atomic_rwlock_endwrite(&caller->t_pid.tp_childlock);
    }
   } else {
    /* Not our child. */
    result = -ECHILD;
   }
   TASK_WEAK_DECREF(t);
   goto end;
  }
  upid = THREAD_PID_GLOBAL_GETTID(&t->t_pid);
  TASK_WEAK_DECREF(t);
  break;

 default:
inval:
  result = -EINVAL;
  goto end;
 }
 result = do_wait(which,upid,options,NULL,infop,ru,NULL);
end:
 task_endcrit();
 return result;
}
SYSCALL_DEFINE4(wait4,pid_t,upid,USER int *,stat_addr,
                int,options,USER struct rusage *,ru) {
 WEAK REF struct task *t; pid_t result; int which;
 if (options & ~(WNOHANG|WUNTRACED|WCONTINUED|
                 __WNOTHREAD|__WCLONE|__WALL
#if 1 /* As an extension, we ignore 'WEXITED' here. */
                 |WEXITED
#endif
                 ))
     return -EINVAL;
 options |= WEXITED;
 task_crit();
 if (upid < -1) {
  /* wait for any child process whose process group ID is equal to the absolute value of pid. */
  t = pid_namespace_lookup_weak(THIS_NAMESPACE,-upid);
  if unlikely(!t) { result = -ECHILD; goto end; }
  upid = THREAD_PID_GLOBAL_GETTGID(&t->t_pid);
  TASK_WEAK_DECREF(t);
  which = P_PGID;
 } else if (upid == -1) {
  /* wait for any child process. */
  which = P_ALL;
  upid  = 0;
 } else if (upid == 0) {
  /* wait for any child process whose process group ID is equal to that of the calling process. */
  which = P_PGID;
  upid  = THREAD_PID_GLOBAL_GETTGID(&THIS_TASK->t_pid);
 } else {
  /* meaning wait for the child whose process ID is equal to the value of pid. */
  t = pid_namespace_lookup_weak(THIS_NAMESPACE,upid);
  if unlikely(!t) { result = -ECHILD; goto end; }
  if (TASK_TRYINCREF(t)) {
   result = do_wait_single(t,options,stat_addr,NULL,ru,NULL);
   TASK_DECREF(t);
   if (E_ISOK(result) && ATOMIC_READ(t->t_pid.tp_parent) == THIS_TASK)
       goto reap_zombie;
  } else if (ATOMIC_READ(t->t_pid.tp_parent) == THIS_TASK) {
   assert(t->t_mode == TASKMODE_TERMINATED);
   /* The child is dead. */
   result = t->t_pid.tp_ids[PIDTYPE_PID].tl_ns == THIS_NAMESPACE
          ? t->t_pid.tp_ids[PIDTYPE_PID].tl_pid : 0;
   if (stat_addr) {
    int exitcode = (int)(uintptr_t)t->t_exitcode;
    if (copy_to_user(stat_addr,&exitcode,sizeof(int)))
        result = -EFAULT;
   }
reap_zombie:
   if (!(options&WNOWAIT)) {
    /* Reap this thread. */
    struct task *caller = THIS_TASK;
    atomic_rwlock_write(&caller->t_pid.tp_childlock);
    if (THREAD_PID_ISCHILD(&caller->t_pid,t)) {
     THREAD_PID_DELCHILD(&caller->t_pid,t);
     if (t != caller) TASK_WEAK_DECREF(t);
    }
    atomic_rwlock_endwrite(&caller->t_pid.tp_childlock);
   }
  } else {
   /* Not our child. */
   result = -ECHILD;
  }
  TASK_WEAK_DECREF(t);
  goto end;
 }
 result = do_wait(which,upid,options,stat_addr,NULL,ru,NULL);
end:
 task_endcrit();
 return result;
}



DECL_END

#endif /* !GUARD_KERNEL_SCHED_SYS_SCHED_C */
