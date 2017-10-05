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
#define _KOS_SOURCE 2
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
#include <kernel/irq.h>
#include <sys/mman.h>

DECL_BEGIN

GLOBAL_ASM(
L(.section .text                                                              )
L(INTERN_ENTRY(sys_exit)                                                      )
L(    sti                                                                     )
L(    /* Load segment registers */                                            )
L(    __ASM_LOAD_SEGMENTS(%dx)                                                )
L(    pushl %ebx                                                              )
L(    pushl ASM_CPU(CPU_OFFSETOF_RUNNING)                                     )
L(    call task_terminate                                                     )
#ifdef CONFIG_DEBUG
L(.global __assertion_unreachable                                             )
L(    call __assertion_unreachable                                            )
#endif
L(SYM_END(sys_exit)                                                           )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text                                                              )
L(INTERN_ENTRY(sys_sched_yield)                                               )
L(    sti                                                                     )
L(    /* Safe & load segment registers */                                     )
L(    __ASM_PUSH_SEGMENTS                                                     )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    call task_yield                                                         )
L(    __ASM_POP_SEGMENTS                                                      )
L(    xorl %eax, %eax                                                         )
L(    iret                                                                    )
L(SYM_END(sys_sched_yield)                                                    )
L(.previous                                                                   )
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

PRIVATE void KCALL
stack_init_vm_copy(struct mman *__restrict nm,
                   struct task *__restrict caller,
                   struct task *__restrict result,
                   struct stack *__restrict ustack) {
 struct mbranch *branch;
#if 0
 syslog(LOG_DEBUG,"Duplicating stack: %p -> %p (%p...%p | %p)\n",
        caller->t_ustack,ustack,ustack->s_begin,(uintptr_t)ustack->s_end-1,
        THIS_SYSCALL_USERESP);
#endif
 ustack->s_refcnt = 1; /* The reference owned by 'result->t_ustack' */
 ustack->s_branch = 0;
 MMAN_FOREACH(branch,nm) {
#ifndef CONFIG_NO_TLB
  /* Also relocate the TLB. */
  if (branch->mb_notify  == &task_tlb_mnotify &&
      branch->mb_closure == caller->t_tlb) {
   asserte(ATOMIC_DECFETCH(caller->t_weakcnt) >= 1);
   branch->mb_closure = result;
   /* NOTE: No need to use atomic operations here, because we're
    *       still holding a lock to the new memory manager, which
    *       is currently the only path anyone else could take to
    *       access our newly created task. */
   ++result->t_weakcnt;
  }
#endif /* !CONFIG_NO_TLB */
  if (branch->mb_notify  != &stack_mnotify ||
      branch->mb_closure != caller->t_ustack) continue;
  asserte(ATOMIC_DECFETCH(caller->t_ustack->s_branch) >= 1);
  branch->mb_closure = ustack;
  ++ustack->s_branch;
 }
 if (ustack->s_branch) ++ustack->s_refcnt;
 result->t_ustack = ustack; /* Inherit reference. */
}


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

#ifndef CONFIG_NO_TLB
 /* Since the TLB would have been copied during
  * 'mman_init_copy_unlocked()', its address stayed the same. */
 result->t_tlb = caller->t_tlb;
#endif /* !CONFIG_NO_TLB */

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
 if (ustack) stack_init_vm_copy(nm,caller,result,ustack);
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
   cs->host.gs     = THIS_SYSCALL_GS;
   cs->host.fs     = THIS_SYSCALL_FS;
   cs->host.es     = THIS_SYSCALL_ES;
   cs->host.ds     = THIS_SYSCALL_DS;
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


#define CLONE_AUTOSTACK_HINT  0x10000000
#define CLONE_AUTOSTACK_SIZE  0x4000
#define CLONE_AUTOSTACK_FUNDS 8
PRIVATE errno_t KCALL
clone_create_auto_stack(struct task *__restrict thread) {
 REF struct stack *ustack; errno_t error;
 struct mman *mm = thread->t_mman;
 size_t gap_size;
 ustack = omalloc(struct stack);
 if unlikely(!ustack) return -ENOMEM;
 error = mman_write(mm);
 if (E_ISERR(error)) {
err_stack:
  free(ustack);
  return error;
 }
 gap_size = 16*PAGESIZE;
find_space:
 ustack->s_begin = mman_findspace_unlocked(mm,(ppage_t)(CLONE_AUTOSTACK_HINT-CLONE_AUTOSTACK_SIZE),
                                           CLONE_AUTOSTACK_SIZE,8,gap_size,MMAN_FINDSPACE_BELOW);
 if (ustack->s_begin == PAGE_ERROR ||
    (uintptr_t)ustack->s_begin+CLONE_AUTOSTACK_SIZE >= KERNEL_BASE) {
  ustack->s_begin = mman_findspace_unlocked(mm,(ppage_t)CLONE_AUTOSTACK_HINT,
                                            CLONE_AUTOSTACK_SIZE,8,gap_size,MMAN_FINDSPACE_ABOVE);
  if (ustack->s_begin == PAGE_ERROR ||
     (uintptr_t)ustack->s_begin+CLONE_AUTOSTACK_SIZE >= KERNEL_BASE) {
   if (gap_size) { gap_size = 0; goto find_space; } /* Try without a gap. */
   mman_endwrite(mm);
   error = -ENOMEM;
   goto err_stack;
  }
 }
 /* All right! We know where the stack should go. */
 thread->t_ustack      = ustack; /* Inherit reference. */
 ustack->s_refcnt      = 1;
 ustack->s_branch      = 0;
 ustack->s_task.ap_ptr = thread;
 ustack->s_end = (VIRT ppage_t)((uintptr_t)ustack->s_begin+CLONE_AUTOSTACK_SIZE);
 /* Now actuall map the stack! */
 error = mman_mmap_stack_unlocked(mm,ustack,PROT_READ|PROT_WRITE,
                                  MREGION_TYPE_LOGUARD,8,PAGESIZE,NULL);
 mman_endwrite(mm);
 if (E_ISERR(error)) {
  thread->t_ustack = NULL;
  free(ustack);
 }
 return error;
}


/* clone() system call. */
SYSCALL_DEFINE5(clone,
                syscall_ulong_t,flags,
                USER void *,newsp,
                USER pid_t *,parent_tidptr,
                USER pid_t *,child_tidptr,
                USER void *,tls_val) {
 REF struct task *result; pid_t child_pid,error;
 struct task *caller = THIS_TASK;
 bool result_needs_stack = true;
 (void)tls_val; /* Not used in the current implementation... */
 task_crit();
 /* Start out my creating the new task. */
 if unlikely((result = task_new()) == NULL) { error = -ENOMEM; goto end; }

 /* Clone the VM first, so we get the new thread's address space all figured out. */
 if (flags&CLONE_VM) {
  /* Inherit the calling thread's VM. */
  result->t_mman = caller->t_mman;
  MMAN_INCREF(result->t_mman);
  /* Allocate a TLB block the new task. */
#ifndef CONFIG_NO_TLB
  error = task_mktlb(result);
  if (E_ISERR(error)) goto err1;
#endif /* !CONFIG_NO_TLB */
 } else {
  if unlikely((result->t_mman = mman_new()) == NULL) goto err1_nomem;
  /* Duplicate the VM and setup copy-on-write for memory branches.
   * HINT: This function also handle duplication of module instances. */
  error = mman_write(result->t_mman);
  if (E_ISERR(error)) goto err1;
  error = mman_write(caller->t_mman);
  if (E_ISERR(error)) { mman_endwrite(result->t_mman); goto err1; }
  error = mman_init_copy_unlocked(result->t_mman,caller->t_mman);
  if (E_ISERR(error)) goto end_double_lock;
#ifndef CONFIG_NO_TLB
  /* Since the TLB would have been copied during
   * 'mman_init_copy_unlocked()', its address stayed the same. */
  result->t_tlb = caller->t_tlb;
#endif /* !CONFIG_NO_TLB */
  /* KOS Extension: Automatically create a new stack. */
  if (newsp == CLONE_CHILDSTACK_AUTO) {
   /* Inherit the old stack. */
   if (caller->t_ustack) {
    result->t_ustack = (REF struct stack *)memdup(caller->t_ustack,
                                                  sizeof(struct stack));
    if unlikely(!result->t_ustack) { error = -ENOMEM; goto end_double_lock; }
   }
   result_needs_stack = false;
   newsp = (void *)THIS_SYSCALL_USERESP;
  }
end_double_lock:
  mman_endwrite(caller->t_mman);
  if (result->t_ustack)
      stack_init_vm_copy(result->t_mman,caller,result,result->t_ustack);
  mman_endwrite(result->t_mman);
  if (E_ISERR(error)) goto err1;
 }

 if (result_needs_stack) {
  if (newsp == CLONE_CHILDSTACK_AUTO) {
   /* Create a kernel-allocated stack. */
   error = clone_create_auto_stack(result);
   if unlikely(E_ISERR(error)) goto err1;
   newsp = result->t_ustack->s_end;
  } else {
   /* Allocate a new stack controller for the new thread. */
   result->t_ustack = omalloc(struct stack);
   if unlikely(!result->t_ustack) goto err1_nomem;
   result->t_ustack->s_begin       = (VIRT ppage_t)newsp; /* ??? */
   result->t_ustack->s_end         = (VIRT ppage_t)newsp;
   result->t_ustack->s_refcnt      = 1;
   result->t_ustack->s_branch      = 0; /* ??? */
   result->t_ustack->s_task.ap_ptr = result;
  }
 }

 /* We also need a new kernel-stack for the thread. */
 error = task_mkhstack(result,TASK_HOSTSTACK_DEFAULTSIZE);
 if (E_ISERR(error)) goto err1;

 /* All right! Everything memory-related has been cloned/inherited.
  * >> Now it's time to do all the other things... */

 if (flags&CLONE_FILES) {
  /* Re-use the same file descriptors. */
  result->t_fdman = caller->t_fdman;
  FDMAN_INCREF(result->t_fdman);
 } else {
  if unlikely((result->t_fdman = fdman_new()) == NULL) goto err1_nomem;
  error = fdman_read(caller->t_fdman);
  if (E_ISERR(error)) goto err1;
  error = fdman_duplicate_table_unlocked(result->t_fdman,
                                         caller->t_fdman);
  fdman_endread(caller->t_fdman);
  if (E_ISERR(error)) goto err1;
 }

 if (flags&CLONE_SIGHAND) {
  /* Re-use the same signal handlers. */
  result->t_sighand = caller->t_sighand;
  SIGHAND_INCREF(result->t_sighand);
 } else {
  result->t_sighand = sighand_copy(THIS_TASK->t_sighand);
  if unlikely(!result->t_sighand) goto err1_nomem;
 }

 if (flags&CLONE_THREAD) {
  REF struct task *leader;
  /* Create as a thread apart of the caller's thread-group. */
  result->t_sigshare = caller->t_sigshare;
  SIGSHARE_INCREF(result->t_sigshare);
  /* Being apart of another task's thread-group,
   * inherit that task's leader as out own. */
  atomic_rwlock_read(&caller->t_pid.tp_leadlock);
  leader = caller->t_pid.tp_leader;
  TASK_INCREF(leader);
  atomic_rwlock_endread(&caller->t_pid.tp_leadlock);
  error = task_set_leader(result,leader);
  TASK_DECREF(leader);
  if (E_ISERR(error)) goto err1;
 } else {
  /* Create a new sigshare controller if the thread belong to a different group. */
  if unlikely((result->t_sigshare = sigshare_new()) == NULL) goto err1_nomem;
  /* Not being created as a thread, this task becomes its own leader. */
  error = task_set_leader(result,result);
  if (E_ISERR(error)) goto err1;
 }

 /* Setup the task's parent link. */
 if (flags&CLONE_DETACHED) {
  /* Setup 'init' as the task's parent, essentially detaching it from the caller. */
  error = task_set_parent(result,&inittask);
 } else if (flags&CLONE_PARENT) {
  REF struct task *parent;
  /* Re-use the caller's parent as the new task's parent, essentially creating a sibling. */
  atomic_rwlock_read(&caller->t_pid.tp_parlock);
  parent = caller->t_pid.tp_parent;
  TASK_INCREF(parent);
  atomic_rwlock_endread(&caller->t_pid.tp_parlock);
  error = task_set_parent(result,parent);
  TASK_DECREF(parent);
 } else {
  /* Create the new task as a child of the caller themself. */
  error = task_set_parent(result,caller);
 }
 if (E_ISERR(error)) goto err1;

 if (flags&CLONE_NEWPID) {
  REF struct pid_namespace *result_ns;
  /* Create a new PID namespace for the new thread. */
  result_ns = pid_namespace_new(PIDTYPE_PID);
  if unlikely(!result_ns) goto err1_nomem;
  error = task_set_lpid(result,result_ns);
  PID_NAMESPACE_DECREF(result_ns);
  if (E_ISERR(error)) goto err1;
 } else {
  /* Assign a new LPID in the caller's PID namespace. */
  error = task_set_lpid(result,caller->t_pid.tp_ids[PIDTYPE_PID].tl_ns);
  if (E_ISERR(error)) goto err1;
 }

 /* Assign a GPID to the new task. */
 if ((error = task_set_gpid(result),E_ISERR(error))) goto err1;

 /* All right! that's everything.
  * Now all we need to do, is to store the new task's TID in
  * the optional buffers provided, and then we can just kick-start the new thread! */
 child_pid = result->t_pid.tp_ids[PIDTYPE_PID].tl_pid;
 if (flags&CLONE_PARENT_SETTID) {
  /* XXX: What if the child was created in a new PID namespace? ('CLONE_NEWPID') */
  if (copy_to_user(parent_tidptr,&child_pid,sizeof(pid_t))) goto err1_fault;
 }

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
   /* Inherit all registers other than EAX and ESP to-be used for argument passing.
    * NOTE: 'ESP' is also inherited when an automatic stack is used and the VM was copied. */
   cs->ss          = THIS_SYSCALL_SS;
   cs->_n2         = 0;
   cs->useresp     = (u32)newsp;
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
   cs->host.gs     = THIS_SYSCALL_GS;
   cs->host.fs     = THIS_SYSCALL_FS;
   cs->host.es     = THIS_SYSCALL_ES;
   cs->host.ds     = THIS_SYSCALL_DS;
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

 { struct mman *omm;
   if (flags&CLONE_CHILD_SETTID) {
    size_t copy_error;
    TASK_PDIR_BEGIN(omm,result->t_mman);
    copy_error = copy_to_user(child_tidptr,&child_pid,sizeof(pid_t));
    TASK_PDIR_END(omm,result->t_mman);
    if (copy_error) goto err1_fault;
   }
   /* One last thing: Initialize the TLB block of the new thread.
    * NOTE: If the VM was cloned, we must be careful doing this,
    *       because another thread may have unmap()-ed its memory
    *       in the mean time. */
   if (flags&CLONE_VM)
        task_ldtlb(result);
   else task_filltlb(result);
 }

 /* YAY! Now we're REALLY done! Simply kick-start it now! */
 error = task_start(result);
 if (E_ISOK(error)) {
  error = child_pid; /* Return the child PID on success */
  /* XXX: We can't return ZERO(0) to the parent,
   *      but what are we supposed to return here? */
  if (error == 0) error = 1;
 }
 TASK_DECREF(result); /* Drop our working reference. */
end:
 task_endcrit();
 return error;
err1_fault: error = -EFAULT; goto err1;
err1_nomem: error = -ENOMEM;
err1: TASK_DECREF(result); goto end;
}

DECL_END

#endif /* !GUARD_KERNEL_SCHED_SYS_SCHED_C */
