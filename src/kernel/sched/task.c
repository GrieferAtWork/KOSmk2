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
#ifndef GUARD_KERNEL_SCHED_TASK_C
#define GUARD_KERNEL_SCHED_TASK_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <bits/signum.h>
#include <dev/rtc.h>
#include <fs/fd.h>
#include <hybrid/align.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <hybrid/traceback.h>
#include <kernel/arch/cpustate.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/paging-util.h>
#include <kernel/paging.h>
#include <kernel/stack.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/signal.h>
#include <sched/smp.h>
#include <sched/task.h>
#include <sched/types.h>
#include <stdalign.h>
#include <string.h>
#include <sys/io.h>

DECL_BEGIN

#undef CONFIG_LOG_WAITING
//#undef CONFIG_SMP

STATIC_ASSERT(IS_ALIGNED(offsetof(struct task,t_signals.ts_first),TASKSIGSLOT_ALIGN));
STATIC_ASSERT(sizeof(struct tasksigslot)           == TASKSIGSLOT_SIZE);
STATIC_ASSERT(offsetof(struct tasksig,ts_first)    == TASKSIG_OFFSETOF_FIRST);
STATIC_ASSERT(sizeof(struct tasksig)               == TASKSIG_SIZE);
STATIC_ASSERT(offsetof(struct task,t_affinity)     == TASK_OFFSETOF_AFFINITY);
STATIC_ASSERT(offsetof(struct task,t_flags)        == TASK_OFFSETOF_FLAGS);
STATIC_ASSERT(offsetof(struct task,t_priority)     == TASK_OFFSETOF_PRIORITY);
STATIC_ASSERT(offsetof(struct task,t_prioscore)    == TASK_OFFSETOF_PRIOSCORE);
STATIC_ASSERT(offsetof(struct task,t_timeout)      == TASK_OFFSETOF_TIMEOUT);
STATIC_ASSERT(offsetof(struct task,t_signals)      == TASK_OFFSETOF_SIGNALS);
STATIC_ASSERT(offsetof(struct task,t_exitcode)     == TASK_OFFSETOF_EXITCODE);
STATIC_ASSERT(offsetof(struct task,t_ic)           == TASK_OFFSETOF_IC);
STATIC_ASSERT(offsetof(struct task,t_addrlimit)    == TASK_OFFSETOF_ADDRLIMIT);
STATIC_ASSERT(offsetof(struct task,t_event)        == TASK_OFFSETOF_EVENT);
STATIC_ASSERT(offsetof(struct task,t_hstack)       == TASK_OFFSETOF_HSTACK);
STATIC_ASSERT(offsetof(struct task,t_lastcr2)      == TASK_OFFSETOF_LASTCR2);
STATIC_ASSERT(offsetof(struct task,t_mman_tasks)   == TASK_OFFSETOF_MMAN_TASKS);
STATIC_ASSERT(offsetof(struct task,t_mman)         == TASK_OFFSETOF_MMAN);
STATIC_ASSERT(offsetof(struct task,t_fdman)        == TASK_OFFSETOF_FDMAN);
STATIC_ASSERT(offsetof(struct task,t_ustack)       == TASK_OFFSETOF_USTACK);
STATIC_ASSERT(offsetof(struct task,t_sigblock)     == TASK_OFFSETOF_SIGBLOCK);
STATIC_ASSERT(offsetof(struct task,t_sigpend)      == TASK_OFFSETOF_SIGPEND);
STATIC_ASSERT(offsetof(struct task,t_sigshare)     == TASK_OFFSETOF_SIGSHARE);
STATIC_ASSERT(offsetof(struct sigenter,se_eip)     == SIGENTER_OFFSETOF_EIP);

#ifdef ARCHTASK_SIZE
STATIC_ASSERT(offsetof(struct task,t_arch) == TASK_OFFSETOF_ARCH);
#ifndef CONFIG_NO_FPU
STATIC_ASSERT(offsetof(struct archtask,at_fpu) == ARCHTASK_OFFSETOF_FPU);
#endif /* !CONFIG_NO_FPU */
#ifndef CONFIG_NO_LDT
STATIC_ASSERT(offsetof(struct archtask,at_ldt_tasks) == ARCHTASK_OFFSETOF_LDT_TASKS);
STATIC_ASSERT(offsetof(struct archtask,at_ldt_gdt)   == ARCHTASK_OFFSETOF_LDT_GDT);
STATIC_ASSERT(offsetof(struct archtask,at_ldt_tls)   == ARCHTASK_OFFSETOF_LDT_TLS);
#endif /* !CONFIG_NO_LDT */
#endif /* ARCHTASK_SIZE */

STATIC_ASSERT(sizeof(sigset_t)            == __SIZEOF_SIGSET_T__);
STATIC_ASSERT(offsetof(struct cpu,c_idle) == CPU_OFFSETOF_IDLE);
STATIC_ASSERT(sizeof(struct task)         == TASK_SIZE);
STATIC_ASSERT(offsetof(struct cpu,c_idle) == CPU_OFFSETOF_IDLE);
STATIC_ASSERT(IS_ALIGNED(offsetof(struct cpu,c_idle),TASK_ALIGN));

STATIC_ASSERT(sizeof(struct cpu)                 == CPU_SIZE);
STATIC_ASSERT(sizeof(atomic_rwptr_t)             == __SIZEOF_POINTER__);
STATIC_ASSERT(offsetof(struct cpustate,useresp)  == 52);

PUBLIC struct task *KCALL
task_cinit(struct task *t) {
 if (t) {
  CHECK_HOST_DOBJ(t);
  CHECK_HOST_DOBJ(t->t_cstate);
  assert(t->t_mode == TASKMODE_RUNNING);
  assert(!t->t_signals.ts_first.tss_sig);
  assert(t->t_refcnt        == 0);
  assert(t->t_weakcnt       == 0);
  assert(t->t_critical      == 0);
  assert(t->t_nointr        == 0);
  assert(t->t_ic            == NULL);
  assert(t->t_suspend[0]    == 0);
  assert(t->t_suspend[1]    == 0);
  assert(t->t_prioscore     == 0);
  assert(t->t_pid.tp_leader == NULL);
  assert(t->t_pid.tp_parent == NULL);
  assert(t->t_sighand       == NULL);
  assert(t->t_sigshare      == NULL);

  t->t_mode = TASKMODE_NOTSTARTED;
  t->t_priority = TASKPRIO_DEFAULT;

  /* Generic initialization. */
  t->t_signals.ts_first.tss_self = t;
  sig_cinit(&t->t_event);
  sigpending_cinit(&t->t_sigpend);
  atomic_rwlock_cinit(&t->t_pid.tp_parlock);
  atomic_rwlock_cinit(&t->t_pid.tp_leadlock);
  atomic_rwlock_cinit(&t->t_pid.tp_childlock);
  atomic_rwlock_cinit(&t->t_pid.tp_grouplock);

  t->t_addrlimit = KERNEL_BASE;
  t->t_weakcnt   = 1; /* The initial weak reference owned by 't_refcnt' itself. */
  t->t_refcnt    = 1;

  assert(t->t_arch.at_ldt_gdt == 0);
  assert(t->t_arch.at_ldt_tls == 0);
  assert(t->t_arch.at_ldt_tasks.le_pself == NULL);
  assert(t->t_arch.at_ldt_tasks.le_next == NULL);
  t->t_arch.at_ldt_tls = LDT_ERROR;

  COMPILER_WRITE_BARRIER();
 }
 return t;
}

PUBLIC errno_t KCALL
task_set_leader(struct task *__restrict self,
                struct task *__restrict leader) {
 errno_t result = -EOK;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(leader);
 assert(self->t_pid.tp_leader == NULL);
 assert(self->t_mode == TASKMODE_NOTSTARTED);
 if unlikely(leader->t_flags&TASKFLAG_NOTALEADER)
    return -ESRCH;
 atomic_rwlock_write(&leader->t_pid.tp_grouplock);
 if unlikely(leader->t_mode == TASKMODE_TERMINATED)
    result = -EINVAL;
 else {
  self->t_pid.tp_leader = leader;
  if (leader != self) TASK_INCREF(leader);
  THREAD_PID_ADDGROUP(&leader->t_pid,self);
 }
 atomic_rwlock_endwrite(&leader->t_pid.tp_grouplock);
 return result;
}
PUBLIC errno_t KCALL
task_set_parent(struct task *__restrict self, struct task *__restrict parent) {
 errno_t result = -EOK;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(parent);
 assert(self->t_pid.tp_parent == NULL);
 assert(self->t_mode == TASKMODE_NOTSTARTED);
 atomic_rwlock_write(&parent->t_pid.tp_childlock);
 if (parent->t_mode == TASKMODE_TERMINATED)
     result = -EINVAL;
 else {
  self->t_pid.tp_parent = parent;
  if (parent != self) {
   TASK_INCREF(parent);
   /* This is where the parent-child weak reference for wait() is stored. */
   TASK_WEAK_INCREF(self);
  }
  THREAD_PID_ADDCHILD(&parent->t_pid,self);
 }
 atomic_rwlock_endwrite(&parent->t_pid.tp_childlock);
 return result;
}



PUBLIC errno_t KCALL
task_set_id(struct task *__restrict self,
            struct pid_namespace *__restrict ns) {
 struct thread_link *link,*existing_link;
 bool has_write_lock = false;
 struct pid_bucket *bucket;
 struct task *chain; pidtype_t type;
 pid_t first_attempt,used_pid;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(ns);
 assert(self->t_mode == TASKMODE_NOTSTARTED);
 type = ns->pn_type;
 assert(type < PIDTYPE_COUNT);
 link = &self->t_pid.tp_ids[type];
 assertf(!link->tl_ns,"Link already set");
 /* TODO: Rehash pid namespace. */

 assert(ns->pn_mapa != 0);
 atomic_rwlock_read(&ns->pn_lock);
 used_pid = ns->pn_next;
scan_again2:
 first_attempt = ns->pn_next;
scan_again:
 bucket = &ns->pn_map[used_pid % ns->pn_mapa];
 /* Make sure that 'used_pid' isn't already in use. */
 for (chain = bucket->pb_chain; chain;) {
  existing_link = &chain->t_pid.tp_ids[type];
  if (existing_link->tl_pid == used_pid) {
   /* Try the next PID. */
   if (++used_pid > ns->pn_max)
         used_pid = ns->pn_min;
   if (used_pid == first_attempt) {
    /* We've gone all the way around, but nothing was available... */
    if (has_write_lock)
         atomic_rwlock_endwrite(&ns->pn_lock);
    else atomic_rwlock_endread(&ns->pn_lock);
    /* No more available ids. */
    return -ENOMEM;
   }
   goto scan_again;
  }
  chain = existing_link->tl_link.le_next;
 }

 if (!has_write_lock) {
  has_write_lock = true;
  if (!atomic_rwlock_upgrade(&ns->pn_lock))
      goto scan_again2;
 }
 ns->pn_next = used_pid+1;
 if (ns->pn_next > ns->pn_max)
     ns->pn_next = ns->pn_min;
 /* Insert the task into this PID namespace. */
 link->tl_pid = used_pid;
 link->tl_ns  = ns; /* Inherit reference (created in the next line) */
 PID_NAMESPACE_INCREF(ns);
 LIST_INSERT(bucket->pb_chain,self,t_pid.tp_ids[type].tl_link);
 /* Mirror the addition of the task within the namespace. */
 ++ns->pn_mapc;
 atomic_rwlock_endwrite(&ns->pn_lock);
 return -EOK;
}


PUBLIC errno_t KCALL
task_start(struct task *__restrict t) {
 pflag_t was; errno_t error = -EOK;
 CHECK_HOST_DOBJ(t);
 CHECK_HOST_DOBJ(t->t_cstate);
 assert(t->t_mode == TASKMODE_NOTSTARTED);
 assert(!t->t_signals.ts_first.tss_sig);
 assert(t->t_signals.ts_first.tss_self == t);
 assert(t->t_refcnt    >= 1);
 assert(t->t_weakcnt   >= 1);
 assert(t->t_critical  == 0);
 assert(t->t_nointr    == 0);
 assert(t->t_ic        == NULL);
 assert(t->t_prioscore == 0);
 assertf(t->t_pid.tp_parent != NULL,"No thread parent set (forgot to call 'task_set_parent()')");
 assertf(t->t_pid.tp_leader != NULL,"No group leader set (forgot to call 'task_set_leader()')");
 assert(t->t_mman != NULL);
 assert(t->t_fdman != NULL);
 assert(t->t_sighand != NULL);
 assert(t->t_sigshare != NULL);
 assert(t->t_cstate != NULL);
 assert((uintptr_t)t->t_cstate >= (uintptr_t)t->t_hstack.hs_begin &&
        (uintptr_t)t->t_cstate <= (uintptr_t)t->t_hstack.hs_end);
 assertf((t->t_cstate->host.cs&3) != 3 ||
         (t->t_cstate->host.eflags&EFLAGS_IF),
         "User-level tasks must run with the #IF flag enabled\n"
         "Either change the task CPL to non-3, or set EFLAGS_IF in eflags.");
 t->t_real_mman = t->t_mman;
 t->t_addrlimit = KERNEL_BASE;
 COMPILER_WRITE_BARRIER();
 /* Schedule the task for its first time. */

 atomic_rwlock_write(&t->t_mman->m_tasks_lock);
 LIST_INSERT(t->t_mman->m_tasks,t,t_mman_tasks);
 atomic_rwlock_endwrite(&t->t_mman->m_tasks_lock);

 was = PREEMPTION_PUSH();
#ifdef CONFIG_SMP
 /* Determine a usable CPU based on affinity. */
 t->t_cpu = cpu_get_suitable(&t->t_affinity);
 if unlikely(!t->t_cpu) { error = -ENODEV; goto end; }
#endif
 /* Create the reference owned by the CPU the task is running on. */
 TASK_INCREF(t);

#ifndef CONFIG_NO_LDT
 /* With the CPU determined, add the task to the associated mman's LDT chain. */
 { struct ldt *ldt;
   error = mman_read(t->t_mman);
   if (E_ISERR(error)) goto end;
   ldt = t->t_mman->m_ldt;
   assert(ldt);
   CHECK_HOST_DOBJ(ldt);
   ldt_write(ldt);
   LIST_INSERT(ldt->l_tasks,t,t_arch.at_ldt_tasks);
   ldt_endwrite(ldt);
   mman_endread(t->t_mman);
 }
#endif

#ifdef CONFIG_SMP
 if (t->t_cpu != THIS_CPU) {
  PREEMPTION_POP(was);
  cpu_write(t->t_cpu);
  if (TASK_ISSUSPENDED(t)) {
   ATOMIC_WRITE(t->t_mode,TASKMODE_SUSPENDED);
   cpu_add_suspended(t->t_cpu,t);
  } else {
   ATOMIC_WRITE(t->t_mode,TASKMODE_WAKEUP);
   t->t_timeout.tv_sec  = 0;
   t->t_timeout.tv_nsec = 0;
   cpu_add_sleeping(t->t_cpu,t);
  }
  cpu_endwrite(t->t_cpu);
 } else
#endif
 {
  if (TASK_ISSUSPENDED(t)) {
   ATOMIC_WRITE(t->t_mode,TASKMODE_SUSPENDED);
   cpu_add_suspended(THIS_CPU,t);
  } else {
   ATOMIC_WRITE(t->t_mode,TASKMODE_RUNNING);
   cpu_add_running(t);
  }
end: ATTR_UNUSED;
  PREEMPTION_POP(was);
 }
 return error;
}

PUBLIC SAFE void KCALL
task_weak_destroy(struct task *__restrict t) {
 struct thread_link *link; pidtype_t i;
 for (i = 0; i < PIDTYPE_COUNT; ++i) {
  struct pid_namespace *ns;
  /* Remove this task from the PID namespace. */
  link = &t->t_pid.tp_ids[i];
  ns = link->tl_ns;
  if (ns) {
   CHECK_HOST_DOBJ(ns);
   assert(ATOMIC_READ(ns->pn_refcnt) >= 1);
   atomic_rwlock_write(&ns->pn_lock);
   assert(ns->pn_mapc);
   /* Decrement the task counter to mirror this task being removed. */
   --ns->pn_mapc;
   LIST_REMOVE(t,t_pid.tp_ids[i].tl_link);
   /* Lazily re-use lower ids, thus prolonging the necessity of re-scanning. */
   if (ns->pn_next == link->tl_pid+1)
       ns->pn_next = link->tl_pid;
   COMPILER_WRITE_BARRIER();
   atomic_rwlock_endwrite(&ns->pn_lock);
   PID_NAMESPACE_DECREF(ns);
  }
 }
 free(t);
}

PRIVATE SAFE void KCALL
task_is_terminating(struct task *__restrict t) {
 struct task *leader,*parent;

#ifndef CONFIG_NO_FPU
 /* Use atomics so as not to disable interrupts. */
 ATOMIC_CMPXCH(CPU(fpu_current),t,NULL);
 /* Free saved FPU registers. */
 if (t->t_arch.at_fpu != FPUSTATE_NULL)
     FPUSTATE_FREE(t->t_arch.at_fpu);
#endif

 /* If this task is a process group leader,
  * kill all threads apart of that group. */
 for (;;) {
  struct task *group_task;
  atomic_rwlock_read(&t->t_pid.tp_grouplock);
  if (!t->t_pid.tp_group) break;
  if (!atomic_rwlock_upgrade(&t->t_pid.tp_grouplock) &&
      !ATOMIC_READ(t->t_pid.tp_group)) {
   atomic_rwlock_downgrade(&t->t_pid.tp_grouplock);
   break;
  }
  group_task = t->t_pid.tp_group;
  assert(group_task->t_pid.tp_leader == t);
  assert(group_task->t_pid.tp_grplink.le_pself == &t->t_pid.tp_group);
  THREAD_PID_DELGROUP(t->t_pid.tp_group,group_task);
  assert(t->t_pid.tp_group != group_task);
  if (group_task == t || !TASK_TRYINCREF(group_task))
      group_task = NULL;
  atomic_rwlock_endwrite(&t->t_pid.tp_grouplock);
  if (group_task) {
   /* Terminate all processes part of this group. */
   task_terminate(group_task,t->t_exitcode);
   TASK_DECREF(group_task);
  }
 }
 assert(!t->t_pid.tp_group);
 atomic_rwlock_endread(&t->t_pid.tp_grouplock);

 /* Remove the task from its group. */
 atomic_rwlock_write(&t->t_pid.tp_leadlock);
 if ((leader = t->t_pid.tp_leader) != NULL) {
  atomic_rwlock_write(&leader->t_pid.tp_grouplock);
  if (THREAD_PID_ISGROUP(&leader->t_pid,t))
      THREAD_PID_DELGROUP(&leader->t_pid,t);
  atomic_rwlock_endwrite(&leader->t_pid.tp_grouplock);
 }
 atomic_rwlock_endwrite(&t->t_pid.tp_leadlock);

 /* Orphan all child processes. */
 atomic_rwlock_write(&t->t_pid.tp_childlock);
 if (t->t_pid.tp_children) {
  struct task **pend = &t->t_pid.tp_children,*end;
  while ((end = *pend) != NULL) {
   /* Ignore the self-entry of a task that is its own parent. */
   if (end == t) {
    assert(t->t_pid.tp_parent == t);
    pend = &end->t_pid.tp_siblings.le_next;
    LIST_REMOVE(end,t_pid.tp_siblings);
    LIST_MKUNBOUND(end,t_pid.tp_siblings);
    continue;
   }

#if 1
   syslog(LOG_SCHED|LOG_INFO,
          "[SCHED] Transferring orphaned %sthread %p (gpid %d) to init\n",
          end->t_mode == TASKMODE_TERMINATED ? "zombie " : "",
          end,end->t_pid.tp_ids[PIDTYPE_GPID].tl_pid);
#endif

   /* Create the reference to 'inittask' used below. */
   if (end->t_mode != TASKMODE_TERMINATED)
       TASK_INCREF(&inittask);

   /* Replace the task's parent. */
   atomic_rwlock_write(&end->t_pid.tp_parlock);
   assert(end->t_pid.tp_parent == t);
   ATOMIC_WRITE(end->t_pid.tp_parent,&inittask);
   atomic_rwlock_endwrite(&end->t_pid.tp_parlock);

   /* XXX: We're probably supposed to be sending some signal here... */

   if (end->t_mode != TASKMODE_TERMINATED)
       asserte(ATOMIC_DECFETCH(t->t_refcnt) > 0);
   pend = &end->t_pid.tp_siblings.le_next;
  }
  /* NOTE: Children may have been deleted above. */
  if (t->t_pid.tp_children) {
   atomic_rwlock_write(&inittask.t_pid.tp_childlock);
   assertf(inittask.t_pid.tp_children != NULL,"What about 'init' itself?");
   /* Prepend the list of child processes. */
   *pend = inittask.t_pid.tp_children;
   inittask.t_pid.tp_children->t_pid.tp_siblings.le_pself = pend;
   inittask.t_pid.tp_children = t->t_pid.tp_children;
   inittask.t_pid.tp_children->t_pid.tp_siblings.le_pself = &inittask.t_pid.tp_children;
   atomic_rwlock_endwrite(&inittask.t_pid.tp_childlock);
   t->t_pid.tp_children = NULL;
  }
 }
 atomic_rwlock_endwrite(&t->t_pid.tp_childlock);

 /* Signal our parent process.
  * NOTE: These may still be NULL if the task is
  *       being destroyed before 'task_start()' is called. */
 atomic_rwlock_write(&t->t_pid.tp_parlock);
 parent = t->t_pid.tp_parent;
 if (t->t_mode == TASKMODE_NOTSTARTED ||
     parent == t) parent = NULL;
 else TASK_INCREF(parent);
 atomic_rwlock_endwrite(&t->t_pid.tp_parlock);

 if (parent != NULL) {
  siginfo_t info; int flags; struct thread_link *link;
  flags = ATOMIC_READ(parent->t_sighand->sh_actions[SIGCHLD].sa_flags);
  if (flags&SA_NOCLDWAIT) {
   atomic_rwlock_write(&parent->t_pid.tp_childlock);
   assert(THREAD_PID_ISCHILD(&parent->t_pid,t));
   LIST_REMOVE(t,t_pid.tp_siblings);
#ifdef CONFIG_DEBUG
   LIST_MKUNBOUND(t,t_pid.tp_siblings);
#endif
   atomic_rwlock_endwrite(&parent->t_pid.tp_childlock);
  }
  if (flags&SA_NOCLDSTOP) goto done_parent;
  memset(&info,0,sizeof(siginfo_t));
  link          = &t->t_pid.tp_ids[PIDTYPE_PID];
  info.si_signo = SIGCHLD;
  info.si_pid   = link->tl_ns == parent->t_pid.tp_ids[PIDTYPE_PID].tl_ns ? link->tl_pid : 0;
  info.si_uid   = 0; /* TODO: User-id. */
  info.si_ptr   = t->t_exitcode;
  /* Raise the signal in the parent task. */
  task_kill2(parent,&info,0,0);
done_parent:
  TASK_DECREF(parent);
 }

}

PRIVATE SAFE void KCALL
task_waitfor_clr(struct task *__restrict t);

PUBLIC SAFE void KCALL
task_destroy(struct task *__restrict t) {
 assert(t != THIS_TASK);
 CHECK_HOST_DOBJ(t);
 assert(!t->t_critical);
 assert(ATOMIC_READ(t->t_weakcnt) != 0);
 assert(t->t_mode == TASKMODE_NOTSTARTED ||
        t->t_mode == TASKMODE_TERMINATED);
 assert(IS_ALIGNED((uintptr_t)t->t_hstack.hs_begin,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)t->t_hstack.hs_end,  PAGESIZE));
 assertf(t != &inittask,"Cannot destroy boot task");
 assertf(!t->t_cpu || t != &t->t_cpu->c_idle,"Cannot destroy cpu IDLE-task");
 /* Validate the consistency of the task's state. */
 assert(t->t_mode == TASKMODE_NOTSTARTED || (t->t_cpu != NULL));
 assert(t->t_mode == TASKMODE_NOTSTARTED || (t->t_mman != NULL));
 assert(t->t_mode == TASKMODE_NOTSTARTED || (t->t_mman == t->t_real_mman));
 assert(t->t_mode == TASKMODE_NOTSTARTED || (t->t_fdman != NULL));
 assert(t->t_mode == TASKMODE_NOTSTARTED || (t->t_pid.tp_parent != NULL));
 assert(t->t_mode == TASKMODE_NOTSTARTED || (t->t_pid.tp_leader != NULL));

 /* Since it is possible for a task to die while waiting for signals,
  * we must clear them now so make sure it doesn't leave any of them
  * in an undefined, or invalid state. */
 task_waitfor_clr(t);

 /* Orphan all child processes + kill all group elements if 't' was a leader.
  * NOTE: Only required if the task was never started, as a terminated
  *       task will have already done this when it was terminated. */
 if (t->t_mode == TASKMODE_NOTSTARTED) {
  task_is_terminating(t);
 } else {
  assert(!t->t_pid.tp_group);
  assert(!t->t_pid.tp_children);
 }


 /* The process group leader can only be deleted here,
  * because it is protected by the [const] attribute. */
 if (t->t_pid.tp_leader &&
     t->t_pid.tp_leader != t)
     TASK_DECREF(t->t_pid.tp_leader);

 if (t->t_pid.tp_parent &&
     t->t_pid.tp_parent != t)
     TASK_DECREF(t->t_pid.tp_parent);

 /* Unmap host+user stacks. */
 task_nointr();
 mman_write(&mman_kernel);
 /* NOTE: We use the address of the thread as
  *       tag when unmapping kernel stacks! */
 if likely(t->t_hstack.hs_begin != t->t_hstack.hs_end) {
  struct mman *old_mman;
  TASK_PDIR_KERNEL_BEGIN(old_mman);
  mman_munmap_unlocked(&mman_kernel,
                       t->t_hstack.hs_begin,
                      (uintptr_t)t->t_hstack.hs_end-
                      (uintptr_t)t->t_hstack.hs_begin,
                       MMAN_MUNMAP_TAG,t);
  TASK_PDIR_KERNEL_END(old_mman);
 }
 if (t->t_mman) {
  if (t->t_mman != &mman_kernel) {
   /* Switch locks to the task's memory manager. */
   mman_endwrite(&mman_kernel);
   mman_write(t->t_mman);
  }
  /* Unmap a potential user-stack. */
  if (t->t_ustack)
      mman_munmap_stack_unlocked(t->t_mman,t->t_ustack);

#ifndef CONFIG_NO_LDT
  if (t->t_arch.at_ldt_tasks.le_pself) {
   ldt_write(t->t_mman->m_ldt);
   COMPILER_READ_BARRIER();
   if (t->t_arch.at_ldt_tasks.le_pself)
       LIST_REMOVE(t,t_arch.at_ldt_tasks);
   ldt_endwrite(t->t_mman->m_ldt);
  }
  /* Delete an LDT entry. */
  if (t->t_arch.at_ldt_tls != LDT_ERROR)
      mman_delldt_unlocked(t->t_mman,t->t_arch.at_ldt_tls);
#endif

  /* XXX: Unmap thread-local data? */
  mman_endwrite(t->t_mman);
  task_endnointr();

  /* Drop references. */
  if (t->t_ustack)
      STACK_DECREF(t->t_ustack);

  /* Remove the task from the list of tasks
   * within the associated memory manager. */
  atomic_rwlock_write(&t->t_mman->m_tasks_lock);
  LIST_REMOVE(t,t_mman_tasks);
  atomic_rwlock_endwrite(&t->t_mman->m_tasks_lock);

  MMAN_DECREF(t->t_mman);
 } else {
  assertf(!t->t_ustack,"Userspace stack, but no memory manager?");
  mman_endwrite(&mman_kernel);
  task_endnointr();
 }

 if (t->t_fdman)
     FDMAN_DECREF(t->t_fdman);
 if (t->t_sighand)
     SIGHAND_DECREF(t->t_sighand);
 if (t->t_sigshare)
     SIGSHARE_DECREF(t->t_sigshare);

 /* Cleanup the cached set of signals. */
 free(t->t_signals.ts_slotv);
 TASK_WEAK_DECREF(t);
}


GLOBAL_ASM(
L(.section .text                             )
L(PUBLIC_ENTRY(cpu_sched_setrunning_savef)   )
//L(    cli                                  ) /* Prevent interrupts from breaking our stack structures below. (Must already be disabled!) */
L(    popl -12(%esp)                         ) /* EIP (return address). */
L(    pushl 4(%esp)                          ) /* eflags. */
L(    jmp 1f                                 )
L(PUBLIC_ENTRY(cpu_sched_setrunning_save)    ) /* Generate an on-stack CPU state. */
//L(    cli                                  ) /* Prevent interrupts from breaking our stack structures below. (Must already be disabled!) */
L(    popl -12(%esp)                         ) /* EIP (return address). */
L(    pushfl                                 ) /* iret-compatible return block. */
L(1:  pushw $0                               )
L(    pushw %cs                              )
L(    subl $4, %esp                          ) /* Skip EIP (saved above) */
L(    pushw %ds                              ) /* Push segment registers. */
L(    pushw %es                              )
L(    pushw %fs                              )
L(    pushw %gs                              )
L(    pushal                                 ) /* Push general purpose registers. */
L(    movl  52(%esp), %eax                   ) /* Load the given 'task' argument into 'EAX' */
L(    movl  %esp, TASK_OFFSETOF_CSTATE(%eax) ) /* Save the CPU state that we've just created within the given task. */
L(PUBLIC_ENTRY(cpu_sched_setrunning)         )
L(    movl ASM_CPU(CPU_OFFSETOF_RUNNING), %eax) /* Load the running task into EAX */
L(    movl TASK_OFFSETOF_CSTATE(%eax),    %esp) /* Load the new CPU state into ESP */
L(    movl (TASK_OFFSETOF_HSTACK+HSTACK_OFFSETOF_END)(%eax), %eax) /* Load the base address of the kernel stack. */
L(    movl %eax, ASM_CPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_TSS+TSS_OFFSETOF_ESP0)) /* Save the proper kernel stack address in the CPU's TSS. */
L(    popal                                  ) /* Pop general purpose registers. */
L(    popw %gs                               ) /* Pop segment registers. */
L(    popw %fs                               )
L(    popw %es                               )
L(    popw %ds                               )
L(    iret                                   ) /* Iret -> pop EIP, CS + EFLAGS */
L(SYM_END(cpu_sched_setrunning)              )
L(SYM_END(cpu_sched_setrunning_save)         )
L(SYM_END(cpu_sched_setrunning_savef)        )
L(.previous                                  )
);

#ifdef CONFIG_LOG_WAITING
PUBLIC u8 dont_log_waiting = 0;
#endif

PUBLIC void KCALL task_yield(void);
GLOBAL_ASM(
L(.section .text                                                              )
L(PUBLIC_ENTRY(task_yield)                                                    )
#ifdef CONFIG_DEBUG
L(    pushl %ebp                                                              )
L(    movl  %esp, %ebp                                                        )
#endif
#if CONFIG_LOG_WAITING
L(    testb $0xff, dont_log_waiting                                           )
L(    jnz 1f                                                                  )
L(    call  __assertion_tbprint                                               )
L(1:                                                                          )
#endif
L(                                                                            )
L(    pushfl  /* Push eflags (will be restored later) */                      )
      /* Special handling:
       *  - When interrupts have been disabled by the caller, we assume that
       *    they wish to remain in full control over when they'll be re-enabled,
       *    meaning we're not actually allowed to switch to another task even
       *    though 'task_yield()' got called.
       *  - This must be done as a precaution, since 'task_yield()' may even
       *    be called when sending a signal to another task (from within 'cpu_write()') */
L(    testl $(EFLAGS_IF), 0(%esp)                                             )
L(    jz    2f                                                                )
L(    cli                                  /* Disable pre-emption. */         )
L(    pushl ASM_CPU(CPU_OFFSETOF_RUNNING)  /* Push the old task (Argument for 'cpu_sched_setrunning_save'). */)
      /* TODO: Use a special CPU rotation that tries not to re-schedule the calling task. */
L(    call  cpu_sched_rotate               /* Rotate running tasks. */        )
L(    pushl %eax                                                              )
L(    pushl %ecx                                                              )
L(    movl 8(%esp),                   %eax                                    )
L(    movl ASM_CPU(CPU_OFFSETOF_RUNNING),%ecx                                 )
#ifndef CONFIG_NO_LDT
L(    /* Switch the LDT if it changed */                                      )
L(    pushl %ebx                                                              )
L(    movl (TASK_OFFSETOF_ARCH+ARCHTASK_OFFSETOF_LDT_GDT)(%eax), %ebx         )
L(    cmpl  %ebx, (TASK_OFFSETOF_ARCH+ARCHTASK_OFFSETOF_LDT_GDT)(%ecx)        )
L(    je    1f /* No change required */                                       )
L(    /* Load the new LDT table. */                                           )
L(    lldt (TASK_OFFSETOF_ARCH+ARCHTASK_OFFSETOF_LDT_GDT)(%ecx)               )
L(1:                                                                          )
#endif
#ifndef CONFIG_NO_FPU
#ifdef CONFIG_NO_LDT
L(    pushl %ebx                                                              )
#endif /* CONFIG_NO_LDT */
L(    /* Disable the FPU for lazy context switching */                        )
L(    movl  %cr0, %ebx                                                        )
L(    orl   $(CR0_TS), %ebx                                                   )
L(    movl  %ebx, %cr0                                                        )
#endif /* !CONFIG_NO_FPU */
#if !defined(CONFIG_NO_LDT) || !defined(CONFIG_NO_FPU)
L(    popl  %ebx                                                              )
#endif
L(    movl TASK_OFFSETOF_MMAN(%eax),  %eax                                    )
L(    movl TASK_OFFSETOF_MMAN(%ecx),  %ecx                                    )
L(    cmpl %eax, %ecx /* Check if memory managers (and thereby page directories) changed. */)
L(    je   1f                                                                 )
L(    movl MMAN_OFFSETOF_PPDIR(%ecx), %ecx                                    )
#if PDIR_OFFSETOF_DIRECTORY
L(    addl $(PDIR_OFFSETOF_DIRECTORY),%ecx                                    )
#endif
L(    movl %ecx, %cr3 /* Switch to the new task's page directory. */          )
L(1:  popl %ecx                                                               )
L(    popl %eax                                                               )
#if 1
L(    call cpu_sched_setrunning_savef /* Switch to the next task. */          )
L(    addl $8, %esp                                                           )
#else
L(    call cpu_sched_setrunning_save /* Switch to the next task. */           )
L(    addl $4, %esp                                                           )
L(    popfl                                                                   )
#endif
L(3:                                                                          )
#ifdef CONFIG_DEBUG
L(    leave                                                                   )
#endif
L(    ret                                                                     )
L(2:  pause                                                                   )
#ifdef CONFIG_DEBUG
L(    pushl 8(%esp)                                                           )
L(    call  noyield_without_irq                                               )
L(    addl  $4, %esp                                                          )
#endif
L(    popfl                                                                   )
L(    jmp 3b                                                                  )
L(SYM_END(task_yield)                                                         )
L(.previous                                                                   )
);

#ifdef CONFIG_DEBUG
INTERN bool interrupts_enabled_initial = false;
INTERN void ATTR_CDECL noyield_without_irq(void *eip) {
 if (!interrupts_enabled_initial) return;
#ifdef CONFIG_SMP
 if (SMP_ONLINE > 1) {
  PRIVATE CPU_DATA void *noyield_last_eip = (void *)-1;
  if (eip == CPU(noyield_last_eip)) return;
  syslog(LOG_SCHED|LOG_WARN,
         "#!$ addr2line(%Ix) '{file}({line}) : {func} : %p : Cannot yield while interrupts are disabled'\n",
        (uintptr_t)eip-1,eip);
  __assertion_tbprint(0);
  CPU(noyield_last_eip) = eip;
 } else
#endif
 {
  __assertion_failed("Cannot yield while interrupts are disabled",DEBUGINFO_GEN);
 }
}
#endif /* CONFIG_DEBUG */


PRIVATE void KCALL
task_set_running(struct task *__restrict t) {
 pflag_t was = PREEMPTION_PUSH();
#ifdef CONFIG_SMP
 if (t->t_cpu != THIS_CPU) {
  ATOMIC_WRITE(t->t_mode,TASKMODE_WAKEUP);
  t->t_timeout.tv_sec  = 0;
  t->t_timeout.tv_nsec = 0;
  cpu_add_sleeping(t->t_cpu,t);
 } else
#endif
 {
  ATOMIC_WRITE(t->t_mode,TASKMODE_RUNNING);
  cpu_add_running(t);
 }
 PREEMPTION_POP(was);
}

#ifdef CONFIG_DEBUG
PUBLIC SAFE void KCALL cpu_validate_counters(bool private_only) {
 struct task *iter;
 pflag_t was;
 size_t n_running = 0;
 size_t n_idling = 0;
 size_t n_suspended = 0;
 size_t n_sleeping = 0;
 was = PREEMPTION_PUSH();
 if (THIS_CPU->c_running) {
  CPU_FOREACH_RUNNING_DO(iter,THIS_CPU) ++n_running;
  CPU_FOREACH_RUNNING_WHILE(iter,THIS_CPU);
 }
 CPU_FOREACH_IDLING(iter,THIS_CPU) ++n_idling;
 if (!private_only) {
  assert(cpu_reading(THIS_CPU));
  CPU_FOREACH_SUSPENDED(iter,THIS_CPU) ++n_suspended;
  CPU_FOREACH_SLEEPING(iter,THIS_CPU) ++n_sleeping;
 }
 assertf(THIS_CPU->c_n_run == n_running,"Invalid run counter (%Iu != %Iu)",THIS_CPU->c_n_run,n_running);
 assertf(THIS_CPU->c_n_idle == n_idling,"Invalid idle counter (%Iu != %Iu)",THIS_CPU->c_n_idle,n_idling);
 if (!private_only) {
  assertf(THIS_CPU->c_n_susp == n_suspended,"Invalid suspended counter (%Iu != %Iu)",THIS_CPU->c_n_susp,n_suspended);
  assertf(THIS_CPU->c_n_sleep == n_sleeping,"Invalid sleeping counter (%Iu != %Iu)",THIS_CPU->c_n_sleep,n_sleeping);
 }
 PREEMPTION_POP(was);
}
#endif

PRIVATE void KCALL cpu_reload_priority(void) {
 struct task *start = THIS_CPU->c_running,*iter = start;
 taskprio_t pmin,pmax; pmin = pmax = start->t_priority;
 while ((iter = iter->t_sched.sd_running.re_next) != start) {
  if (pmin > iter->t_priority) pmin = iter->t_priority;
  if (pmax < iter->t_priority) pmax = iter->t_priority;
 }
 THIS_CPU->c_prio_min = pmin;
 THIS_CPU->c_prio_max = pmax;
 COMPILER_WRITE_BARRIER();
}

PRIVATE void KCALL cpu_add_idling(struct task *__restrict t) {
 struct task **piter,*iter;
 ++THIS_CPU->c_n_idle;
 /* Schedule as an idling task.
  * NOTE: The chain of idling tasks is descendingly
  *       ordered by priority & memory manager. */
 piter = &THIS_CPU->c_idling;
 while ((iter = *piter) != NULL &&
        (iter->t_priority <  t->t_priority ||
        (iter->t_priority == t->t_priority &&
         iter->t_mman     <  t->t_mman)))
         piter = &iter->t_sched.sd_running.re_next;
 /* Insert the given task 't' before 'iter' / after 'piter'. */
 t->t_sched.sd_running.re_next = iter;
 if (iter) iter->t_sched.sd_running.re_prev = t;
 *piter = t;
 if (piter == &THIS_CPU->c_idling)
      t->t_sched.sd_running.re_prev = NULL;
 else t->t_sched.sd_running.re_prev = container_of(piter,struct task,t_sched.sd_running.re_next);
}


/* Park all IDLE tasks from 'c_running' into 'c_idling'
 * WARNING: Upon completion, 'c_running' may be set to NULL if all tasks were parked,
 *          meaning it is the caller's responsibility to install at least one new task. */
PRIVATE SAFE void KCALL cpu_park_idle(void) {
 struct task *iter,*next,*start;
 assert(TASK_ISSAFE());
 /* NOTE: Critical tasks may not be parked
  *    >> In addition, we must never part the calling task to prevent inconsistencies.
  *       For that reason, 'pit_exc' checks the old task for needing to be parked,
  *       because a task must never be parked when one of the two is true:
  *        - Interrupts are disabled
  *        - The task is critical
  *  Now: Take a look at that 'SAFE' tag of this function.
  *       Yes! A function is only SAFE when the caller ensure one of the same
  *            restrictions: THIS_TASK is critical, or interrupts are disabled!
  *  >> So essentially, we must guaranty that 'THIS_CPU->c_running'
  *     is never parked, because the only way to write to 'THIS_CPU->c_running',
  *     is to disable interrupts (which in turn would make it illegal to
  *     park said task, also meaning that a task can never part itself,
  *     which makes sense again because there must always be another task
  *     to continue doing stuff!) */
 iter = start = THIS_CPU->c_running;
 while ((iter = iter->t_sched.sd_running.re_next) != start) {
  if (TASKPRIO_ISIDLE(iter->t_priority) &&
     !iter->t_critical) {
   next = iter->t_sched.sd_running.re_next;
   RING_REMOVE(iter,t_sched.sd_running);
   assert(THIS_CPU->c_n_run != 0);
   --THIS_CPU->c_n_run;
   if (iter == next) next = NULL;
   if (iter == start) THIS_CPU->c_running = start = next;
   cpu_add_idling(iter);
   if (!start) break;
   iter = next;
  }
 }
 cpu_validate_counters(true);
}


INTDEF void KCALL cpu_add_running(REF struct task *__restrict t) {
 assert(!PREEMPTION_ENABLED());
 assert(t->t_mode == TASKMODE_RUNNING);
 assert(TASK_CPU(t) == THIS_CPU);
 if (t->t_priority < THIS_CPU->c_prio_min &&
     TASKPRIO_ISIDLE(t->t_priority)) {
  cpu_add_idling(t);
 } else {
  /* Schedule as a normal, running task. */
  if (THIS_CPU->c_prio_min > t->t_priority) THIS_CPU->c_prio_min = t->t_priority;
  if (THIS_CPU->c_prio_max < t->t_priority) THIS_CPU->c_prio_max = t->t_priority;
  /* If adding this task incremented the max-priority in
   * such a way that IDLE task can be parked, park them now! */
  if (TASKPRIO_ISIDLE(THIS_CPU->c_prio_min) &&
     !TASKPRIO_ISIDLE(THIS_CPU->c_prio_max))
      cpu_park_idle();
  if unlikely(!THIS_CPU->c_running) {
   /* Special case: All other tasks were parked. */
   assert(THIS_CPU->c_n_run == 0);
   t->t_sched.sd_running.re_prev = t;
   t->t_sched.sd_running.re_next = t;
   THIS_CPU->c_running = t;
   THIS_CPU->c_prio_min = 
   THIS_CPU->c_prio_max = t->t_priority;
  } else {
   struct task *insert_after,*first;
   assert(THIS_CPU->c_n_run != 0);
   /* Find a suitable insert location, preferably next
    * to other tasks within the same page directory. */
   first = insert_after = THIS_CPU->c_running;
   do if (insert_after->t_mman == t->t_mman) break;
   while ((insert_after = insert_after->t_sched.sd_running.re_next) != first);
   /* Insert the given task after this one. */
   RING_INSERT_AFTER(insert_after,t,t_sched.sd_running);
   cpu_reload_priority();
  }
  t->t_prioscore = THIS_CPU->c_prio_max-t->t_priority;
  ++THIS_CPU->c_n_run;
 }
 COMPILER_WRITE_BARRIER();
 cpu_validate_counters(true);
}

INTDEF void KCALL cpu_del_running(/*OUT REF*/struct task *__restrict t) {
 assert(!PREEMPTION_ENABLED());
 assert(t->t_mode == TASKMODE_RUNNING);
 assert(TASK_CPU(t) == THIS_CPU);
 /* Check for special case: Deleting the current task. */
 if (t == THIS_CPU->c_running)
  cpu_sched_remove_current();
 else {
  if (t == THIS_CPU->c_idling)
      THIS_CPU->c_idling = t->t_sched.sd_running.re_next;
  assert(t != THIS_CPU->c_idling);
#define NEXT(t) (t)->t_sched.sd_running.re_next
#define PREV(t) (t)->t_sched.sd_running.re_prev
  /* NOTE: Must check for NULL-pointers, because the IDLE-ring isn't actually a ring... */
  if (PREV(t)) NEXT(PREV(t)) = NEXT(t);
  if (NEXT(t)) PREV(NEXT(t)) = PREV(t);
#undef PREV
#undef NEXT

  /* Must update active-task priorities if
   * the given task was in the running-chain. */
  if (t->t_priority >= THIS_CPU->c_prio_min) {
   cpu_reload_priority();
   assert(THIS_CPU->c_n_run);
   --THIS_CPU->c_n_run;
  } else {
   assert(THIS_CPU->c_n_idle);
   --THIS_CPU->c_n_idle;
  }
 }
 COMPILER_WRITE_BARRIER();
 cpu_validate_counters(true);
}

INTDEF void KCALL
cpu_add_sleeping(struct cpu *__restrict c,
             REF struct task *__restrict t) {
 struct task **piter,*iter;
 assert(cpu_writing(c));
 assert(t->t_mode == TASKMODE_SLEEPING ||
        t->t_mode == TASKMODE_WAKEUP);
 assert(TASK_CPU(t) == c);
 ++THIS_CPU->c_n_sleep;
 piter = &c->c_sleeping;
 if (t->t_mode == TASKMODE_WAKEUP) {
  /* Install the given task for a future wakeup. */
  assertf(t->t_timeout.tv_nsec == 0 && t->t_timeout.tv_sec == 0,
          "Wakeup tasks must have their timeout set to {0,0}");
  while ((iter = *piter) != NULL &&
          iter->t_mode == TASKMODE_WAKEUP &&
          iter->t_mman <  t->t_mman)
          piter = &iter->t_sched.sd_sleeping.le_next;
 } else {
  /* Use a different search mechanism that compares time. */
  while ((iter = *piter) != NULL &&
         (assert(iter->t_mode == TASKMODE_WAKEUP ||
                 iter->t_mode == TASKMODE_SLEEPING),
         (iter->t_mode == TASKMODE_WAKEUP ||
         (TIMESPEC_LOWER(iter->t_timeout,t->t_timeout) ||
         (TIMESPEC_EQUAL(iter->t_timeout,t->t_timeout) &&
          iter->t_mman < t->t_mman)))))
          piter = &iter->t_sched.sd_sleeping.le_next;
 }
 t->t_sched.sd_sleeping.le_next  = iter;
 t->t_sched.sd_sleeping.le_pself = piter;
 if (iter) iter->t_sched.sd_sleeping.le_pself = &t->t_sched.sd_sleeping.le_next;
 *piter = t;
 COMPILER_WRITE_BARRIER();
 cpu_validate_counters(false);
}
INTDEF void KCALL
cpu_add_suspended(struct cpu *__restrict c,
              REF struct task *__restrict t) {
 struct task **piter,*iter;
 assert(cpu_writing(c));
 assert(t->t_mode == TASKMODE_SUSPENDED);
 assert(TASK_CPU(t) == c);
 ++THIS_CPU->c_n_susp;
 piter = &c->c_suspended;
 while ((iter = *piter) != NULL &&
        (assert(iter->t_mode == TASKMODE_SUSPENDED),
         iter->t_mman < t->t_mman))
         piter = &iter->t_sched.sd_suspended.le_next;
 t->t_sched.sd_suspended.le_next  = iter;
 t->t_sched.sd_suspended.le_pself = piter;
 if (iter) iter->t_sched.sd_suspended.le_pself = &t->t_sched.sd_suspended.le_next;
 *piter = t;
 COMPILER_WRITE_BARRIER();
 cpu_validate_counters(false);
}

INTDEF void KCALL
cpu_del_sleeping(struct cpu *__restrict c,
      /*OUT REF*/struct task *__restrict t) {
 assert(cpu_writing(c));
 assert(t->t_mode == TASKMODE_SLEEPING ||
        t->t_mode == TASKMODE_WAKEUP);
 assert(TASK_CPU(t) == c);
 LIST_REMOVE(t,t_sched.sd_sleeping);
 assert(THIS_CPU->c_n_sleep);
 --THIS_CPU->c_n_sleep;
 COMPILER_WRITE_BARRIER();
 cpu_validate_counters(false);
}
INTDEF void KCALL
cpu_del_suspended(struct cpu *__restrict c,
       /*OUT REF*/struct task *__restrict t) {
 assert(cpu_writing(c));
 assert(t->t_mode == TASKMODE_SUSPENDED);
 assert(TASK_CPU(t) == c);
 LIST_REMOVE(t,t_sched.sd_suspended);
 assert(THIS_CPU->c_n_susp);
 --THIS_CPU->c_n_susp;
 COMPILER_WRITE_BARRIER();
 cpu_validate_counters(false);
}


PUBLIC struct task *FCALL cpu_sched_rotate(void) {
 struct task *new_task;
 assert(!PREEMPTION_ENABLED());
 assert(THIS_CPU->c_running);
 new_task = THIS_CPU->c_running->t_sched.sd_running.re_next;
 while (new_task->t_prioscore--) new_task = new_task->t_sched.sd_running.re_next;
 new_task->t_prioscore = THIS_CPU->c_prio_max-new_task->t_priority;
 THIS_CPU->c_running = new_task;
 COMPILER_WRITE_BARRIER();
 cpu_validate_counters(true);
 return new_task;
}
PUBLIC REF struct task *FCALL cpu_sched_remove_current(void) {
 REF struct task *result,*new_task;
 assert(!PREEMPTION_ENABLED());
 cpu_validate_counters(true);
 result = THIS_CPU->c_running;
 assert(result != NULL);
 assertf(result != &THIS_CPU->c_idle,"Cannot remove the IDLE task!");
 assert((result->t_sched.sd_running.re_next == result) ==
        (result->t_sched.sd_running.re_prev == result));
 if (result->t_sched.sd_running.re_next == result) {
  /* If this is the only running task, then at least
   * IDLE task must be apart of the IDLE-chain.
   * >> Load all IDLE tasks with the same priority as the first. */
  taskprio_t load_prio;
  struct task *first_idle,*last_idle,*next;
  first_idle = THIS_CPU->c_idling;
  assertf(first_idle != NULL,"Where's the IDLE task itself?");
  load_prio = first_idle->t_priority;
  last_idle = first_idle;
  for (;;) {
   next = last_idle->t_sched.sd_running.re_next;
   assertf(next != first_idle,
           "next      = %p\n"
           "last_idle = %p\n",
           next,last_idle);
   assert(THIS_CPU->c_n_idle);
   --THIS_CPU->c_n_idle;
   ++THIS_CPU->c_n_run;
   if (!next ||
      ((assert(next->t_priority <= load_prio),
        next->t_priority != load_prio)))
        break;
   last_idle = next;
  }
  /* Update the idling task chain to contain what's
   * left after we've inherited one priority level. */
  THIS_CPU->c_idling = last_idle->t_sched.sd_running.re_next;
  /* Close the ring of new idle tasks. */
  last_idle->t_sched.sd_running.re_next  = first_idle;
  first_idle->t_sched.sd_running.re_prev = last_idle;
  THIS_CPU->c_running                    = first_idle;
  /* Reload task priorities. */
  cpu_reload_priority();
  /* Initialize the priority scores of all tasks we've just scheduled. */
  do first_idle->t_prioscore = THIS_CPU->c_prio_max-first_idle->t_priority;
  while ((first_idle = first_idle->t_sched.sd_running.re_next) != last_idle);
 } else {
  /* Select the next task. */
  while ((new_task = cpu_sched_rotate()) == result);
  assert(THIS_CPU->c_running == new_task);
  assert(THIS_CPU->c_running != result);
  /* Remove the task from the ring. */
  RING_REMOVE(result,t_sched.sd_running);
  /* Check if we must reload the CPU min/max priority scores. */
  assert(result->t_priority >= THIS_CPU->c_prio_min);
  assert(result->t_priority <= THIS_CPU->c_prio_max);
  if (result->t_priority == THIS_CPU->c_prio_min ||
      result->t_priority == THIS_CPU->c_prio_max)
      cpu_reload_priority();
 }
 --THIS_CPU->c_n_run;
 assert(THIS_CPU->c_n_run != 0);
 assert(result != THIS_CPU->c_running);
 COMPILER_WRITE_BARRIER();
 cpu_validate_counters(true);
 return result;
}



INTERN ATTR_NOINLINE struct cpustate *FCALL
pit_exc(struct cpustate *__restrict state) {
 struct task *old_task;
 struct task *new_task;
 assert(!PREEMPTION_ENABLED());
 if (IRQ_PIC_SPURIOUS(IRQ_PIC1_PIT)) return state;

#if 0
 __assertion_printf("#!$ addr2line(%Ix) '{file}({line}) : {func} : PIT Trigger: %p'\n",
                   (uintptr_t)state->host.eip-1,state->host.eip);
 __assertion_tbprint2((void *)state->host.ebp,0);
#endif

 /* Update the sub-second clock. */
 sysrtc_periodic();

 if (cpu_trywrite(THIS_CPU)) {
  struct task *wake;
  /* Check for wakeups in sleeping tasks. */
  if ((wake = THIS_CPU->c_sleeping) != NULL) {
   struct timespec now; sysrtc_get(&now);
   for (;;) {
    if (TIMESPEC_LOWER(now,wake->t_timeout)) break;
    assert(wake->t_mode == TASKMODE_SLEEPING ||
           wake->t_mode == TASKMODE_WAKEUP);
    /* Mark the task as having timed out. */
    if (wake->t_mode  == TASKMODE_SLEEPING)
        wake->t_flags |= TASKFLAG_TIMEDOUT;

    /* Steal the wake-task from the sleeper chain */
    THIS_CPU->c_sleeping = wake->t_sched.sd_sleeping.le_next;
    --THIS_CPU->c_n_sleep;
    ATOMIC_WRITE(wake->t_mode,TASKMODE_RUNNING);
    cpu_add_running(wake);
    if ((wake = THIS_CPU->c_sleeping) == NULL) break;
    wake->t_sched.sd_sleeping.le_pself = &THIS_CPU->c_sleeping;
   }
  }
  cpu_endwrite(THIS_CPU);
 }

 cpu_validate_counters(true);
 old_task = THIS_CPU->c_running;
 new_task = cpu_sched_rotate();

 /* Check if the previous task is idling.
  * NOTE: The secondary criteria of the task having
  *       interrupts enabled is already a given,
  *       since it is that 'old_task' which we're
  *       originating from, meaning that its
  *       cpustate _must_ have interrupts enabled.
  *      (Or the task executed an 'int $0' manually?) */
#if 0
 syslog(LOG_SCHED|LOG_WARN,"[IRQ] %#.2I8x, %#.2I8x, %#.2I8x %Iu %Iu\n",
        THIS_CPU->c_prio_min,THIS_CPU->c_prio_max,
        old_task->t_priority,old_task->t_critical,
        THIS_CPU->c_n_run);
#endif
 if (TASKPRIO_ISIDLE(old_task->t_priority) &&
    !TASKPRIO_ISIDLE(THIS_CPU->c_prio_max) &&
    !old_task->t_critical && old_task != new_task) {
#if 0
  syslog(LOG_SCHED|LOG_DEBUG,"[IRQ] Parking task %p (priority %#.2I8x) during IRQ\n",
         old_task,old_task->t_priority);
#endif
  /* Must park the old task. */
  assert(THIS_CPU->c_n_run >= 2);
  --THIS_CPU->c_n_run;
  RING_REMOVE(old_task,t_sched.sd_running);
  cpu_add_idling(old_task);
  cpu_reload_priority();
  COMPILER_WRITE_BARRIER();
 }


#if 0
 if (old_task != new_task) {
  syslog(LOG_DEBUG,"#PIT %p(%p) --> %p(%p) (IF 1->%d)\n",
         old_task,old_task->t_cstate->host.eip,
         new_task,new_task->t_cstate->host.eip,
      !!(new_task->t_cstate->host.eflags&EFLAGS_IF));
 }
#endif

 TASK_SWITCH_CONTEXT(old_task,new_task);
 /* Signal completion of the PIT interrupt.
  * NOTE: Actual new signals will only be received once iret turns interrupts back on.
  *       And in the event that the new task didn't have interrupts
  *       enabled, they wont turn back on for a while. */
 IRQ_PIC_EOI(IRQ_PIC1_PIT);
 assert(!PREEMPTION_ENABLED());

 /* Update the CPU's TSS with the new task's kernel stack.
  * NOTE: We're only safe to do so, because interrupts are still disabled! */
 THIS_CPU->c_arch.ac_tss.esp0 = (uintptr_t)new_task->t_hstack.hs_end;
 old_task->t_cstate = state;
 COMPILER_BARRIER(); /* Make sure that everything above has been done. */

#if 0
 __assertion_printf("#!$ addr2line(%Ix) '{file}({line}) : {func} : Returning to: %p'\n",
                   (uintptr_t)new_task->t_cstate->host.eip,new_task->t_cstate->host.eip);
 __assertion_tbprint2((void *)new_task->t_cstate->host.ebp,0);
 __assertion_printf("EAX %p  ECX %p  EDX %p  EBX %p\n",
                    new_task->t_cstate->host.eax,
                    new_task->t_cstate->host.ecx,
                    new_task->t_cstate->host.edx,
                    new_task->t_cstate->host.ebx);
 __assertion_printf("ESP %p  EBP %p  ESI %p  EDI %p\n",
                    new_task->t_cstate,
                    new_task->t_cstate->host.ebp,
                    new_task->t_cstate->host.esi,
                    new_task->t_cstate->host.edi);
#endif

 assertf(new_task->t_cstate,"Task %p has no cpu state",new_task);
 assertf(state->host.eflags&EFLAGS_IF,"How did you manage to preempt without interrupts enabled?");
 assertf((new_task->t_cstate->host.cs&3) != 3 ||
         (new_task->t_cstate->host.eflags&EFLAGS_IF),
          "Must not switch to user-level task with #IF flag disabled");

 /* Return the new CPU state that should be loaded next.
  * NOTE: Interrupts are re-enabled when iret is triggered. */
 return new_task->t_cstate;
}








/* Tasking API */

PUBLIC struct cpu *KCALL
task_setcpu(struct task *__restrict t,
            struct cpu *__restrict new_cpu) {
 struct cpu *result,*old_cpu; pflag_t was;
 bool has_newcpu_writelock = false;
 CHECK_HOST_DOBJ(t);
 CHECK_HOST_DOBJ(new_cpu);

 was = PREEMPTION_PUSH();

 /* Kind-of complicated way to lock both the old & new CPU. */
 for (;;) {
  assert(ATOMIC_READ(t->t_mode) != TASKMODE_NOTSTARTED);
  old_cpu = ATOMIC_READ(t->t_cpu);
  /* Check if the two CPUs actually differ. */
  if (old_cpu == new_cpu) {
   if (has_newcpu_writelock)
       cpu_endwrite(new_cpu);
   result = old_cpu;
   goto end;
  }
  if (has_newcpu_writelock) {
   if (!cpu_trywrite(old_cpu)) {
    has_newcpu_writelock = false;
    cpu_endwrite(new_cpu);
    cpu_write(old_cpu);
   }
   goto has_old_lock;
  }
  cpu_write(old_cpu);
has_old_lock:
  if unlikely(old_cpu != ATOMIC_READ(t->t_cpu)) {
   cpu_endwrite(old_cpu);
   continue;
  }
  if (has_newcpu_writelock) break;
  if (cpu_trywrite(new_cpu)) break;
  cpu_endwrite(old_cpu);
  cpu_write(new_cpu);
  has_newcpu_writelock = true;
 }
 assert(t != &t->t_cpu->c_idle);

 assert(ATOMIC_READ(t->t_mode) != TASKMODE_NOTSTARTED);

 /* Handle task mode other than running. */
 switch (t->t_mode) {
 case TASKMODE_SUSPENDED:
  /* Transfer the suspended task. */
  assert(ATOMIC_READ(t->t_cpu) == old_cpu);
  cpu_del_suspended(old_cpu,t);
  ATOMIC_WRITE(t->t_cpu,new_cpu);
  cpu_endwrite(old_cpu);
  cpu_add_suspended(new_cpu,t);
  cpu_endwrite(new_cpu);
  result = old_cpu;
  goto end;

 case TASKMODE_SLEEPING:
 case TASKMODE_WAKEUP:
  /* Transfer the sleeping task. */
  assert(ATOMIC_READ(t->t_cpu) == old_cpu);
  cpu_del_sleeping(old_cpu,t);
  ATOMIC_WRITE(t->t_cpu,new_cpu);
  cpu_endwrite(old_cpu);
  cpu_add_sleeping(new_cpu,t);
  cpu_endwrite(new_cpu);
  result = old_cpu;
  goto end;

 case TASKMODE_NOTSTARTED:
 case TASKMODE_TERMINATED:
  cpu_endwrite(old_cpu);
  cpu_endwrite(new_cpu);
  result = E_PTR(-EINVAL);
  goto end;

 default: break;
 }

 assert(t->t_cpu == old_cpu);
 if (old_cpu == THIS_CPU) {
  bool was_running;
  result = old_cpu;

#ifndef CONFIG_NO_FPU
  if (CPU(fpu_current) == t) {
   /* Store the FPU context before switching CPUs. */
   if unlikely(t->t_arch.at_fpu == FPUSTATE_NULL &&
              (t->t_arch.at_fpu = FPUSTATE_ALLOC()) == FPUSTATE_NULL) {
    syslog(LOG_ERR,"[FPU] Failed to allocate FPU state (gpid %d): %[errno]\n",
           t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,ENOMEM);
   } else {
    FPUSTATE_ENABLE();
    FPUSTATE_SAVE(t->t_arch.at_fpu);
   }
   FPUSTATE_DISABLE();
   CPU(fpu_current) = NULL;
  }
#endif

  assert(!PREEMPTION_ENABLED());
  assert(TASK_CPU(t) == THIS_CPU);
  assert(TASK_CPU(t) != new_cpu);
  assert(t->t_mode   == TASKMODE_RUNNING);
  was_running = t == THIS_CPU->c_running;
  cpu_del_running(t);

  /* Add the task to the new CPU using the wakeup chain. */
  ATOMIC_WRITE(t->t_mode,TASKMODE_WAKEUP);
  t->t_timeout.tv_nsec = 0;
  t->t_timeout.tv_sec  = 0;
  cpu_add_sleeping(new_cpu,t);

  cpu_endwrite(old_cpu);
  cpu_endwrite(new_cpu);
  /* If we've set our own new CPU, we must make
   * sure not to return in the context of the old! */
  if (was_running) {
   TASK_SWITCH_CONTEXT(t,THIS_CPU->c_running);
   cpu_sched_setrunning_savef(t,was);
   return result;
  }
 } else {
  cpu_endwrite(old_cpu);
  cpu_endwrite(new_cpu);
  /* TODO: Trigger an RPC on 'old_cpu' that will attempt to set the new CPU. */
  result = E_PTR(-ECOMM);
 }
end:
 PREEMPTION_POP(was);
 return result;
}



PUBLIC SAFE bool KCALL
sig_vsendone_unlocked(struct sig *__restrict s,
                      void const *data, size_t datasize) {
 struct tasksigslot *slot,*next; pflag_t was;
 struct task *t; struct cpu *c;
 CHECK_HOST_DOBJ(s);
 assert(TASK_ISSAFE());
 assert(sig_writing(s));
 cpu_validate_counters(true);
 /* Pop one signal slot and update links to the next. */
 slot = SIG_GETTASK(s);
 if (!slot) return false;
 CHECK_HOST_DOBJ(slot);
 assert(slot->tss_chain.le_pself == (struct tasksigslot **)&s->s_task);
 assert(slot->tss_sig == s);
 t = slot->tss_self;
 if unlikely(!(slot == &t->t_signals.ts_first ||
              (slot >=  t->t_signals.ts_slotv &&
               slot <   t->t_signals.ts_slotv+
                        t->t_signals.ts_slotc))) {
  /* Special case: If the signal isn't targeting the active signal buffer,
   *              (such as after 'task_pushwait()' was called) simply mark
   *               it as being send.
   * It is important that signals can still be sent and be considered
   * as such, even when not actively being apart of the target thread's
   * set of receiving tasks. */
  ATOMIC_CMPXCH(t->t_signals.ts_recv,NULL,s);
  return true;
 }

 CHECK_HOST_DOBJ(t);
 was = PREEMPTION_PUSH();
 for (;;) {
  c = TASK_CPU(t);
  COMPILER_READ_BARRIER();
  cpu_write(c);
  if likely(TASK_CPU(t) == c) break;
  cpu_endwrite(c);
 }
 cpu_validate_counters(c != THIS_CPU);

 next = slot->tss_chain.le_next;
 assert(IS_ALIGNED((uintptr_t)next,TASKSIGSLOT_ALIGN));
#ifdef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
 /* NOTE: Must keep the write-lock held by the caller intact! */
 ATOMIC_WRITE(s->s_task.ap_data,(uintptr_t)next|ATOMIC_RWPTR_MASK_WMODE);
#else
 s->s_task = next;
#endif
 if (next) {
  next->tss_last           = slot->tss_last;
  next->tss_chain.le_pself = (struct tasksigslot **)&s->s_task;
 }
 slot->tss_chain.le_pself = NULL;
#ifdef CONFIG_DEBUG
 slot->tss_last = NULL;
#endif

 /* Only the first signal can be received by the target. */
 if (!ATOMIC_CMPXCH(t->t_signals.ts_recv,NULL,s))
      goto done;

 /* Copy signal data. */
 if (slot->tss_siz && datasize)
     copy_to_user(slot->tss_buf,data,
                  MIN(slot->tss_siz,datasize));

 cpu_validate_counters(c != THIS_CPU);
 switch (t->t_mode) {

 case TASKMODE_SUSPENDED:
  if (TASK_ISSUSPENDED(t)) break;
 case TASKMODE_SLEEPING:
  assert(t != THIS_TASK);
  /* Add the thread to it's cpu's wakeup list. */
  if (t->t_mode == TASKMODE_SLEEPING)
       cpu_del_sleeping (c,t);
  else cpu_del_suspended(c,t);
  task_set_running(t);
  assertf(t != THIS_TASK,
          "This shouldn't happen because the caller must ensure SAFE-restrictions");

 case TASKMODE_WAKEUP:
  break;

 default: break;
 }

done:
 cpu_validate_counters(false);
 cpu_endwrite(c);
 PREEMPTION_POP(was);
 return true;
}


#ifdef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
LOCAL void KCALL
tasksigslot_list_remove_endwrite(struct tasksigslot *__restrict slot,
                                 struct sig *__restrict s) {
 struct tasksigslot *next;
 assert(slot->tss_sig == s);
 slot->tss_sig = NULL;
 if (!slot->tss_chain.le_pself) goto unlinked;
 if ((next = slot->tss_chain.le_next) != NULL) {
  assertf(next != slot,"Slot self-reference at %p\n",slot);
  assertf(next->tss_chain.le_pself == &slot->tss_chain.le_next,
          "Invalid link: %p != %p",next->tss_chain.le_pself,&slot->tss_chain.le_next);
  assertf(next->tss_sig == s,"Invalid signals: %p != %p (tasks %p, %p)",
          next->tss_sig,s,next->tss_self,slot->tss_self);
  next->tss_chain.le_pself = slot->tss_chain.le_pself;
 }
 assert(IS_ALIGNED((uintptr_t)next,TASKSIGSLOT_ALIGN));
 if (slot->tss_chain.le_pself == (struct tasksigslot **)&s->s_task) {
  /* Update the link & unlock the signal at the same time. */
  ATOMIC_WRITE(s->s_task.ap_data,(uintptr_t)next);
 } else {
  *slot->tss_chain.le_pself = next;
  if (next) next->tss_chain.le_pself = slot->tss_chain.le_pself;
unlinked:
  sig_endwrite(s);
 }
}
#else
#define tasksigslot_list_remove_endwrite(slot,s) \
{ assert((slot)->tss_sig == (s)); \
  (slot)->tss_sig = NULL; \
  if ((slot)->tss_chain.le_pself) \
       LIST_REMOVE(slot,tss_chain); \
  sig_endwrite(s); }
#endif

PRIVATE SAFE void KCALL
task_waitfor_clr(struct task *__restrict t) {
 struct sig *s; struct tasksigslot *slot,*end;
 if ((s = t->t_signals.ts_first.tss_sig) != NULL) {
  sig_write(s);
  tasksigslot_list_remove_endwrite(&t->t_signals.ts_first,s);
 }
 end = (slot = t->t_signals.ts_slotv)+
               t->t_signals.ts_slotc;
 t->t_signals.ts_slotc = 0;
 for (; slot != end; ++slot) {
  s = slot->tss_sig;
  assert(s);
  sig_write(s);
  tasksigslot_list_remove_endwrite(slot,s);
 }
}
#undef tasksigslot_list_remove_endwrite

PUBLIC SAFE struct sig *KCALL task_clrwait(void) {
 struct sig *result;
 struct task *t = THIS_TASK;
 task_waitfor_clr(t);
 COMPILER_WRITE_BARRIER();
 result = t->t_signals.ts_recv;
 t->t_signals.ts_recv = NULL;
 COMPILER_WRITE_BARRIER();
 return result;
}

PRIVATE void KCALL
tasksigslot_relocate(struct tasksigslot *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(self->tss_sig);
 sig_write(self->tss_sig);
 if (!self->tss_chain.le_pself) goto unlinked;
 if (self->tss_chain.le_next) {
  assert(self->tss_chain.le_next->tss_sig == self->tss_sig);
  self->tss_chain.le_next->tss_chain.le_pself = &self->tss_chain.le_next;
 } else {
  struct tasksigslot *head;
  /* This is the last signal slot (Must update the signal-chain head entry). */
  head = SIG_GETTASK(self->tss_sig);
  assert(head->tss_sig == self->tss_sig);
  self->tss_last = self;
 }
 if ((void **)&self->tss_sig->s_task ==
     (void **)self->tss_chain.le_pself) {
  /* Update the initial link (and unlock the signal). */
  ATOMIC_WRITE(*self->tss_chain.le_pself,self);
#ifndef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
  /* Still have to manually unlock the signal. */
  sig_endwrite(self->tss_sig);
#endif
 } else {
  *self->tss_chain.le_pself = self;
unlinked:
  sig_endwrite(self->tss_sig);
 }
}

PUBLIC SAFE void KCALL
task_pushwait(struct tasksig *__restrict sigs) {
 struct task *t = THIS_TASK;
 struct tasksigslot *iter,*end;
 CHECK_HOST_DOBJ(sigs);
 memcpy(sigs,&t->t_signals,sizeof(struct tasksig));
 COMPILER_WRITE_BARRIER();
 /* Relocate signal slots. */
 if (sigs->ts_first.tss_sig)
     tasksigslot_relocate(&sigs->ts_first);
 end = (iter = sigs->ts_slotv)+sigs->ts_slotc;
 for (; iter != end; ++iter) {
  assert(iter->tss_sig);
  tasksigslot_relocate(iter);
 }
 COMPILER_WRITE_BARRIER();
 /* At this point we've transferred all active signals to the stored set.
  * Note, that in the mean time, any one of those signals may have been sent! */
 COMPILER_READ_BARRIER();
 /* If a signal was received in the mean time,
  * update the send signal in the saved signal set. */
 if (t->t_signals.ts_recv) {
  ATOMIC_CMPXCH(sigs->ts_recv,NULL,t->t_signals.ts_recv);
  t->t_signals.ts_recv = NULL;
 }
 /* With everything transferred, clear out saved signal data. */
 t->t_signals.ts_first.tss_sig = NULL;
 t->t_signals.ts_slota = 0;
 t->t_signals.ts_slotc = 0;
 t->t_signals.ts_slotv = NULL;
 COMPILER_WRITE_BARRIER();
}
PUBLIC SAFE void KCALL
task_popwait(struct tasksig *__restrict sigs) {
 struct tasksigslot *iter,*end;
 struct task *t = THIS_TASK;
 CHECK_HOST_DOBJ(sigs);
 /* Do some validating on these signals. */
 assertf(sigs->ts_first.tss_self == t,
         "The given signal set does not originate from this thread (%p != %p)",
         sigs->ts_first.tss_self,t);

 /* Clear any signals that the caller may currently be waiting for.
  * This behavior is defined by the specs, and prevents undefined
  * behavior when a signal set is restored while a task */
 task_waitfor_clr(t);
 memcpy(&t->t_signals,sigs,sizeof(struct tasksig));
 COMPILER_WRITE_BARRIER();
 /* Relocate signal slots. */
 if (t->t_signals.ts_first.tss_sig)
     tasksigslot_relocate(&t->t_signals.ts_first);
 end = (iter = t->t_signals.ts_slotv)+t->t_signals.ts_slotc;
 for (; iter != end; ++iter) {
  assert(iter->tss_self == t);
  assert(iter->tss_sig);
  tasksigslot_relocate(iter);
 }
 COMPILER_READ_BARRIER();
 /* If a signal was sent while we were relocating, save that signal. */
 if (sigs->ts_recv)
     ATOMIC_CMPXCH(t->t_signals.ts_recv,NULL,sigs->ts_recv);
 COMPILER_WRITE_BARRIER();
}


PUBLIC SAFE errno_t KCALL
task_addwait(struct sig *__restrict s,
             USER void *buffer, size_t bufsize) {
 struct tasksigslot *slot;
 struct task *t = THIS_TASK;
 assert(IS_ALIGNED((uintptr_t)t,TASK_ALIGN));
 assert(sig_writing(s));
 /* No need to add more signals if we've already received one. */
 if unlikely(t->t_signals.ts_recv != NULL)
    return -EOK;
 slot = &t->t_signals.ts_first;
 if (!t->t_signals.ts_first.tss_sig) {
  slot = &t->t_signals.ts_first;
  goto use_slot;
 }
 /* Make sure we're not already waiting for this signal.
  * NOTE: If we are already waiting, ignore the second request of doing so. */
 if unlikely(task_haswait(s)) return -EOK;

 /* Allocate a new signal slot. */
 slot = t->t_signals.ts_slotv;
 if (t->t_signals.ts_slotc == t->t_signals.ts_slota) {
  size_t oldalloc,newalloc;
  struct tasksig old_sigset;
  oldalloc = newalloc = t->t_signals.ts_slota;
  if (!newalloc) newalloc = 1;
  newalloc *= 2;
realloc_again:
  /* Make sure to preserve the set of active signals. */
  task_pushwait(&old_sigset);
  assert(!slot || slot == old_sigset.ts_slotv);
  slot = (struct tasksigslot *)krealign(slot,TASKSIGSLOT_ALIGN,
                                        newalloc,GFP_SHARED);
  if (slot) {
   old_sigset.ts_slota = newalloc;
   old_sigset.ts_slotv = slot;
  }
  task_popwait(&old_sigset);
  if unlikely(!slot) {
   if (newalloc != t->t_signals.ts_slota-1) {
    newalloc = t->t_signals.ts_slota-1;
    goto realloc_again;
   }
   assert(t->t_signals.ts_slotc);
   --t->t_signals.ts_slotc;
   return -ENOMEM;
  }
  assert(IS_ALIGNED((uintptr_t)slot,TASKSIGSLOT_ALIGN));
  assert(t->t_signals.ts_slota == newalloc);
  assert(t->t_signals.ts_slotv == slot);
  while (oldalloc < newalloc) {
   /* Initialize newly allocated slots. */
   slot[oldalloc].tss_self = t;
   slot[oldalloc].tss_sig  = NULL;
   ++oldalloc;
  }
 }
 assert(IS_ALIGNED((uintptr_t)slot,TASKSIGSLOT_ALIGN));
 slot += t->t_signals.ts_slotc++;
 assert(IS_ALIGNED((uintptr_t)slot,TASKSIGSLOT_ALIGN));
use_slot:
 assert(slot->tss_self == t);
 slot->tss_buf           = buffer;
 slot->tss_siz           = bufsize;
 slot->tss_sig           = s;
 assert(IS_ALIGNED((uintptr_t)slot,TASKSIGSLOT_ALIGN));
 if (!SIG_GETTASK(s)) {
#ifdef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
  s->s_task.ap_data = (uintptr_t)slot|ATOMIC_RWPTR_MASK_WMODE;
#else
  s->s_task                = slot;
#endif
  slot->tss_last           = slot;
  slot->tss_chain.le_pself = (struct tasksigslot **)&s->s_task;
  slot->tss_chain.le_next  = NULL;
 } else {
#ifdef CONFIG_DEBUG
  slot->tss_last = NULL;
#endif
  CHECK_HOST_DOBJ(SIG_GETTASK(s));
  CHECK_HOST_DOBJ(SIG_GETTASK(s)->tss_last);
  assert(SIG_GETTASK(s)->tss_sig == s);
  assert(SIG_GETTASK(s)->tss_last->tss_sig == s);
  assert(!SIG_GETTASK(s)->tss_chain.le_next ||
          SIG_GETTASK(s)->tss_chain.le_next->tss_sig == s);
  LIST_INSERT_AFTER(SIG_GETTASK(s)->tss_last,slot,tss_chain);
  SIG_GETTASK(s)->tss_last = slot;
 }
 COMPILER_WRITE_BARRIER();
 return -EOK;
}

PUBLIC bool KCALL
task_haswait(struct sig *__restrict s) {
 struct tasksigslot *slot,*end;
 struct task *t = THIS_TASK;
 if (t->t_signals.ts_first.tss_sig == s)
     return true;
 end = (slot = t->t_signals.ts_slotv)+
               t->t_signals.ts_slotc;
 for (; slot != end; ++slot) {
  if (slot->tss_sig == s)
      return true;
 }
 return false;
}

PUBLIC struct sig *KCALL task_trywait(void) {
 struct sig *result;
 struct task *t = THIS_TASK;
 result = ATOMIC_READ(t->t_signals.ts_recv);
 if (result) {
  /* task has a signal!
   * >> Clear all pending signals and
   *    reset the receive-vector. */
  task_waitfor_clr(t);
  t->t_signals.ts_recv = NULL;
  COMPILER_WRITE_BARRIER();
 }
 return result;
}


PUBLIC struct sig *KCALL
task_waitfor(struct timespec const *abstime) {
 struct sig *result; pflag_t was;
 struct task *t = THIS_TASK;

#if CONFIG_LOG_WAITING
#undef __assertion_tbprint
 if (dont_log_waiting == 0)
     __assertion_tbprint();
#endif

 /* Prevent any of the signals from being send while we're still adding them. */
 was = CPU_WRITETHIS();
 assert(TASK_CPU(t) == THIS_CPU);

 /* Check for pending interrupts. */
 if (t->t_flags&TASKFLAG_INTERRUPT &&
    !t->t_nointr) {
  t->t_flags &= ~(TASKFLAG_INTERRUPT);
  cpu_endwrite(THIS_CPU);
  result = E_PTR(-EINTR);
  goto end_clear2;
 }
 result = ATOMIC_XCH(t->t_signals.ts_recv,NULL);
 if (result) goto end_clear; /* A signal has already been sent. */

 if unlikely(t == &THIS_CPU->c_idle) {
  /* Special case: wait for signals from within the IDLE task. */
  /* NOTE: The following code assumes that IDLE tasks cannot change CPU affinity. */
  cpu_endwrite(THIS_CPU);
#if 0
  syslog(LOG_DEBUG,"IDLE task waitfor()\n");
#endif
  if (was&EFLAGS_IF) PREEMPTION_ENABLE();
  for (;;) {
   /* Check for signals/flags. */
   if (ATOMIC_READ(t->t_signals.ts_recv) != NULL ||
       ATOMIC_READ(t->t_flags)&(TASKFLAG_INTERRUPT|TASKFLAG_TIMEDOUT)) {
    if (!(was&EFLAGS_IF)) PREEMPTION_DISABLE();
    goto got_signal;
   }

   /* Check for timeout. */
   if (abstime) {
    struct timespec now;
    sysrtc_get(&now);
    if (TIMESPEC_GREATER_EQUAL(now,*abstime)) {
     result = E_PTR(-ETIMEDOUT);
     if (!(was&EFLAGS_IF)) PREEMPTION_DISABLE();
     goto end_clear2;
    }
   }
   PREEMPTION_DISABLE();
   assert(THIS_CPU->c_running == &THIS_CPU->c_idle);
   /* Try to switch to another task. */
   if (THIS_CPU->c_idle.t_sched.sd_running.re_next != THIS_CPU->c_running)
       task_yield();
   else {
#ifdef CONFIG_SMP
    if (SMP_ONLINE <= 1)
#endif
    {
     assertf(was&EFLAGS_IF,
             "DEADLOCK: IDLE task has interrupts disabled, but no other task to switch to");
    }
    PREEMPTION_ENABLE();
    /* Idle for a while. */
    PREEMPTION_IDLE();
   }
  }
 }

 /* Remove the current task from the scheduler. */
 asserte(cpu_sched_remove_current());

 /* Re-add the task as either sleeping, or suspended. */
 if (abstime) {
  CHECK_HOST_TOBJ(abstime);
  t->t_timeout = *abstime;
  ATOMIC_WRITE(t->t_mode,TASKMODE_SLEEPING);
  cpu_add_sleeping(THIS_CPU,t);
 } else {
  ATOMIC_WRITE(t->t_mode,TASKMODE_SUSPENDED);
  cpu_add_suspended(THIS_CPU,t);
 }

 assert(!PREEMPTION_ENABLED());
 cpu_endwrite(THIS_CPU);
 TASK_SWITCH_CONTEXT(t,THIS_CPU->c_running);
 cpu_validate_counters(true);
 cpu_sched_setrunning_save(t); /* Switch to the new task. */
 cpu_validate_counters(true);

got_signal:
 PREEMPTION_POP(was);
 task_waitfor_clr(t);

 if unlikely(t->t_flags&(TASKFLAG_INTERRUPT|TASKFLAG_TIMEDOUT)) {
  if (t->t_flags&TASKFLAG_INTERRUPT) result = E_PTR(-EINTR);
  else { assert(abstime); result = E_PTR(-ETIMEDOUT); }
  t->t_flags &= ~(TASKFLAG_INTERRUPT|TASKFLAG_TIMEDOUT);
  ATOMIC_WRITE(t->t_signals.ts_recv,NULL);
 } else {
  result = ATOMIC_XCH(t->t_signals.ts_recv,NULL);
  assert(result != NULL);
 }
 return result;
end_clear2:
 ATOMIC_WRITE(t->t_signals.ts_recv,NULL);
end_clear:
 task_waitfor_clr(t);
 PREEMPTION_POP(was);
 return result;
}




#ifdef CONFIG_DEBUG
PRIVATE ATTR_USED void KCALL
__task_destroy2(struct task *__restrict t) {
 CHECK_HOST_DOBJ(t);
 assertf((uintptr_t)&t <  (uintptr_t)t->t_hstack.hs_begin ||
         (uintptr_t)&t >= (uintptr_t)t->t_hstack.hs_end,
         "Still on stack that'll be destroyed (%p in %p...%p)",
         &t,t->t_hstack.hs_begin,(uintptr_t)t->t_hstack.hs_end-1);
#if 0
 syslog(LOG_DEBUG,"DESTROYING TASK AFTER TERMINATE: %p\n",t);
#endif
 task_destroy(t);
}
#else
#define __task_destroy2 task_destroy
#endif

/* Terminate the calling task and unlock its associated CPU. */
PRIVATE ATTR_NORETURN void
task_terminate_self_unlock_cpu(struct task *__restrict t) {
 struct task *new_task;
 CHECK_HOST_DOBJ(t);
 assert(t == THIS_TASK);

 PREEMPTION_DISABLE();
 assert(TASK_CPU(t) == THIS_CPU);
 assert(THIS_CPU->c_running == t);
 ATOMIC_WRITE(t->t_critical,0);
 /* TODO: DEADLOCK:
RUNNING TASK C01A101C (PID = 0/0) - (null)
../??(0) : ?? : [0] : E000696E : C01BBCE8
../??(0) : ?? : [1] : E0005C4F : C01BBD28
../include/hybrid/sync/atomic-rwlock.h(138) : atomic_rwlock_write : [2] : C012F9F0 : C01BBD30
../src/kernel/memory/malloc.c(1612) : mheap_free : [3] : C0134FFF : C01BBD78
../src/kernel/memory/malloc.c(2256) : debug_free : [4] : C0137535 : C01BBDB8
../src/kernel/memory/malloc.c(3088) : _kffree_d : [5] : C013A816 : C01BBE08
../src/include/kernel/memory.h(483) : page_free_scatter : [6] : C014B9CA : C01BBE68
../src/kernel/mman/part.c(248) : action_do_decref : [7] : C014C69F : C01BBEA8
../src/kernel/mman/part.c(266) : action_decref : [8] : C014C799 : C01BBEF8
../src/kernel/mman/part.c(154) : mregion_part_action : [9] : C014C26C : C01BBF18
../src/kernel/mman/part.c(298) : mregion_part_decref : [a] : C014C83E : C01BBF68
../src/kernel/mman/part.c(370) : mregion_decref : [b] : C014CB7E : C01BBF98
../src/kernel/mman/mman.c(1421) : mman_munmap_impl : [c] : C0146843 : C01BBFC8
../src/kernel/mman/mman.c(1482) : mman_munmap_unlocked : [d] : C0146B5A : C01BBFF8
../src/kernel/sched/task.c(501) : task_destroy : [e] : C015ACB5 : C01BC038
../src/kernel/sched/task.c(1811) : __task_destroy2 : [f] : C015EA24 : C01BC078
../src/kernel/sched/task.c(1873) : task_terminate_self_unlock_cpu : [10] : C015ED49 : C01BC098
# -- SPLIT: The task was pre-empted here and us hi-jacking it doesn't
            account for its heap lock (Which we also can't work around
            because that would break the ABI when the thread we've hijacked
            is currently in the process of modifying the heap when we suddenly
            decide start freeing more stuff; locking rules 101)
../src/include/sync/owner-rwlock.h(194) : owner_rwlock_timedwrite : [11] : C0130618 : C01BC0F8
../src/kernel/memory/malloc.c(969) : core_page_free : [12] : C01329B0 : C01BC128
../src/kernel/memory/malloc.c(1448) : mheap_unmapfree : [13] : C0134629 : C01BC168
../src/kernel/memory/malloc.c(1383) : mheap_release : [14] : C0134254 : C01BC1B8
../src/kernel/memory/malloc.c(1613) : mheap_free : [15] : C0135015 : C01BC218
../src/kernel/memory/malloc.c(2256) : debug_free : [16] : C0137535 : C01BC258
../src/kernel/memory/malloc.c(3087) : _kfree_d : [17] : C013A79E : C01BC2A8
../src/kernel/sched/signal.c(259) : sighand_destroy : [18] : C0150624 : C01BC2F8
../src/kernel/sched/task.c(542) : task_destroy : [19] : C015AFF9 : C01BC328
../src/kernel/sched/task.c(1811) : __task_destroy2 : [1a] : C015EA24 : C01BC358
../src/kernel/sched/task.c(1873) : task_terminate_self_unlock_cpu : [1b] : C015ED49 : C01BC378 
 */


 /* Must unlock the CPU _AFTER_ we disabled preemption,
  * because if we did so before, the PIC interrupt handler
  * might notice us being terminated and try to remove us
  * before we're done.
  * HINT: The CPU must lock 'c_lock' before checking
  *       if a task was marked as terminated in SMP-mode. */
 cpu_endwrite(THIS_CPU);

 assert(THIS_CPU->c_running == t);
 asserte(cpu_sched_remove_current() == t);
 new_task = THIS_CPU->c_running;
 assert(t != new_task);
 assert(t != THIS_CPU->c_idling);

 /* WARNING: At this point, the cpu-local/THIS_TASK setup is extremely inconsistent.
  *         'THIS_TASK' does not match what is actually the current chain of execution. */
 ATOMIC_WRITE(t->t_mode,TASKMODE_TERMINATED);

 assert(!PREEMPTION_ENABLED());

 /* Make sure to activate the new task's page directory as early as possible! */
 assert(t->t_mman == new_task->t_mman ||
        memcmp(&       t->t_mman->m_pdir.pd_directory[KERNEL_BASE/PDTABLE_REPRSIZE],
               &new_task->t_mman->m_pdir.pd_directory[KERNEL_BASE/PDTABLE_REPRSIZE],
              ((THIS_PDIR_BASE-KERNEL_BASE)/PDTABLE_REPRSIZE)*4) == 0);
 TASK_SWITCH_CONTEXT(t,new_task);

#if 0
 syslog(LOG_DEBUG,"Terminating SELF: %p (%d)\n",t,t->t_refcnt);
#endif

 assert(THIS_TASK == new_task);

 /* Switch to the cpu's current (new) task's stack & register state,
  * where we can then broadcast termination & decref() the old task.
  * Afterwards, iret to the new task. */
 __asm__ __volatile__(
     L(    movl ASM_CPU2(CPU_OFFSETOF_RUNNING), %%eax   )
     /* Prevent the new task from being terminated while it's destroying the old.
      * NOTE: This precaution starts taking effect once 'sti' re-enables interrupts below. */
     L(    incl TASK_OFFSETOF_CRITICAL(%%eax)           )
     L(    movl TASK_OFFSETOF_CSTATE(%%eax), %%esp      )
     L(    movl (TASK_OFFSETOF_HSTACK+HSTACK_OFFSETOF_END)(%%eax), %%eax) /* Load the base address of the kernel stack. */
     L(    movl %%eax, ASM_CPU2(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_TSS+TSS_OFFSETOF_ESP0)) /* Save the proper kernel stack address in the CPU's TSS. */
     L(    popl %%edi                                   )
     L(    popl %%esi                                   )
     L(    popl %%ebp                                   )
     L(    addl $8, %%esp  /* Skip ESP and EBX */       )
     L(    popl %%edx                                   )
     L(    popl %%ecx                                   )
     L(    popl %%eax                                   )
     L(    popw %%gs                                    )
     L(    popw %%fs                                    )
     L(    popw %%es                                    )
     L(    popw %%ds                                    )
     /* At this point we've reached the IRET tail.
      * NOTE: Because interrupts are still enabled, all registers
      *       we've just popped are still saved on-stack, meaning we're
      *       save to look back and copy EBX for later consumption. */
     L(    pushl -24(%%esp) /* Push the proper EBX */   )
     /* At this point, we've essentially hacked our way into
      * the new task's execution stack and finally managed
      * to fix up the stack in such a way that could essentially
      * allow us to be pre-empted again.
      * Because of that, we are now safe to re-enable interrupts,
      * even before broadcasting the join-signal (which in itself
      * could potentially enable interrupts momentarily)
      * NOTE: All registers but 'EBX' are loaded with the proper
      *       return values, while the correct 'EBX' is saved in
      *       0(%ESP), and the current EBX still contains a pointer
      *       to the old task, of which we're supposed to signal
      *       termination before decref()-ing it. */
     L(    sti                                          )

     /* Save volatile registers that may be modified below. */
     L(    pushl %%eax                                  )
     L(    pushl %%ecx                                  )
     L(    pushl %%edx                                  )
     /* At this point we've entered the context of the new task, but havn't restored its registers.
      * Doing so later, for now we will instead broadcast the join old task's join signal.
      * NOTE: Broadcasting cannot be done earlier because:
      *       - The join signal may only be send once the task's state has changed to 'TASKMODE_TERMINATED'
      *       - The task's state can only change to 'TASKMODE_TERMINATED' after it has been removed from the ring.
      *       - Since 'sig_send()' may cause the caller to be preempted, a valid 'THIS_TASK' is required.
      *      >> But since we're trying to delete our old context, we can only
      *         broadcast having done so once we've acquired a new once.
      * HINT: '%EBX' need not be saved across this call, because it is a callee-saved register! */
     L(    leal  TASK_OFFSETOF_EVENT(%%ebx), %%eax       )
     L(    pushl $0xffffffff                            )
     L(    pushl %%eax                                  )
     L(    call  sig_send                               )
     L(                                                 )
     /* Run custom on-terminate handlers. */
     L(    pushl %%ebx                                  )
     L(    call  task_is_terminating                    )
     L(                                                 )
     L(    movl $-1, %%edx                              )
     L(    lock; xadd %%edx, TASK_OFFSETOF_REFCNT(%%ebx))
     /* If the result of the subtraction was non-zero, don't call 'task_destroy()' */
     L(    jnz 1f                                       )
     L(    pushl %%ebx                                  )
     L(    call __task_destroy2                         )
     /* End the critical execution block stared above.
      * >> This was required to keep the host (new) task from
      *    potentially terminating itself while it was dealing
      *    with the old one's termination. */
     L(1:  call task_endcrit                            )
     /* Restore all remaining registers and yield to what was
      * running in this task before we've started messing around. */
     L(    popl %%edx                                   )
     L(    popl %%ecx                                   )
     L(    popl %%eax                                   )
     L(    popl %%ebx                                   )
     L(    iret                                         )
     :
     : "b" (t)
     : "memory"
 );
 __builtin_unreachable();
}

PUBLIC SAFE errno_t KCALL
task_interrupt_cpu_endwrite(struct task *__restrict t) {
 errno_t error = -EOK;
 struct cpu *c = TASK_CPU(t);
 assert(cpu_writing(c));
 assert(!PREEMPTION_ENABLED());
 if (t == THIS_TASK) {
  /* Make sure to clear any additional interrupts send to this task. */
  //ATOMIC_FETCHAND(t->t_flags,~TASKFLAG_INTERRUPT);
  error = -EINTR;
  goto end;
 }
 /* NOTE: Don't set the interrupt flag for running tasks.
  *     - They won't be able to consume it.
  * >> With that in mind, interrupting a running task is basically a no-op.
  */
 if (t->t_mode != TASKMODE_RUNNING) {
  if (ATOMIC_FETCHOR(t->t_flags,TASKFLAG_INTERRUPT)&
                               (TASKFLAG_INTERRUPT))
      goto end; /* No-op if the task was already interrupted. */
 }
 switch (t->t_mode) {

 case TASKMODE_TERMINATED:
  error = -EINVAL;
  break;

 case TASKMODE_SUSPENDED:
  if (TASK_ISSUSPENDED(t)) break;
  cpu_del_suspended(c,t);
  goto install_wakeup;
 case TASKMODE_SLEEPING:
  cpu_del_sleeping(c,t);
install_wakeup:
  /* Schedule the task as wakeup. */
  task_set_running(t);
  break;

 default: break;
 }

end:
 cpu_endwrite(c);
 return error;
}

PUBLIC errno_t KCALL
task_interrupt(struct task *__restrict t) {
 struct cpu *c; pflag_t was; errno_t error;
 assert(ATOMIC_READ(t->t_mode) != TASKMODE_NOTSTARTED);
 was = PREEMPTION_PUSH();
 for (;;) {
  c = TASK_CPU(t);
  COMPILER_READ_BARRIER();
  cpu_write(c);
  if (TASK_CPU(t) == c) break;
  cpu_endwrite(c);
 }
 error = task_interrupt_cpu_endwrite(t);
 PREEMPTION_POP(was);
 return error;
}



PUBLIC errno_t KCALL
task_join(struct task *__restrict t,
          struct timespec const *timeout,
          void **exitcode) {
 errno_t result = -EOK;
 CHECK_HOST_DOBJ(t);
wait_again: ATTR_UNUSED;
 sig_read(&t->t_event);
 /* Special case: 'TASKMODE_TERMINATED' is written with a write-barrier
  *                before the join signal is broadcast, meaning that
  *                while locking the join-signal, we can read the
  *                task's real state with a write-barrier to confirm
  *                that the join-signal hasn't been send yet. */
 if (ATOMIC_READ(t->t_mode) == TASKMODE_TERMINATED) {
  sig_endread(&t->t_event);
  goto end;
 }
 if (!sig_upgrade(&t->t_event) &&
      ATOMIC_READ(t->t_mode) == TASKMODE_TERMINATED)
 { sig_endwrite(&t->t_event); goto end; }

 result = sig_timedrecv_endwrite(&t->t_event,timeout);
 if (E_ISERR(result)) return result;
 /* NOTE: Since 't_event' is not just send for termination, we
  *       must re-check that the thread has really terminated now. */
 goto wait_again;
end:
 if (exitcode) {
  /* Store the exitcode when requested, to. */
  CHECK_HOST_DOBJ(exitcode);
  *exitcode = t->t_exitcode;
 }
 return result;
}

PUBLIC SAFE errno_t KCALL
task_terminate_cpu_endwrite(struct cpu *__restrict c,
                            struct task *__restrict t,
                            void *exitcode) {
 errno_t error = -EOK;
 CHECK_HOST_DOBJ(c);
 CHECK_HOST_DOBJ(t);
 assert(c == TASK_CPU(t));
 assert(cpu_writing(c));
 assert(!PREEMPTION_ENABLED());
 assert(t != &c->c_idle);
 assert(ATOMIC_READ(t->t_mode) != TASKMODE_NOTSTARTED);
#if 0
 syslog(LOG_DEBUG,"Terminate: %p\n",exitcode);
#undef __assertion_tbprint
 __assertion_tbprint();
#endif

 if (ATOMIC_READ(t->t_critical)) {
  /* Interrupt the task. */
  if (t->t_flags&TASKFLAG_WILLTERM) {
einval:
   cpu_endwrite(c);
   error = -EINVAL;
  } else {
   t->t_flags |= TASKFLAG_WILLTERM;
   t->t_exitcode = exitcode;
   COMPILER_WRITE_BARRIER();
   task_interrupt_cpu_endwrite(t);
  }
  goto end;
 }

 switch (t->t_mode) {
 default: goto einval;

 case TASKMODE_SUSPENDED:
  cpu_del_suspended(c,t);
  break;

 case TASKMODE_SLEEPING:
 case TASKMODE_WAKEUP:
  cpu_del_sleeping(c,t);
  break;

 case TASKMODE_RUNNING:
  assertf(t->t_mode == TASKMODE_RUNNING,
         "t->t_mode = %d",(int)t->t_mode);
  if (c != THIS_CPU) {
   /* TODO: Emit an RPC interrupt. */
   cpu_endwrite(c);
   return -ECOMM;
  }
#if 0
  assert((t == THIS_TASK) ==
         (c->c_running == t));
  if (c->c_running == t) {
   --t->t_critical;
   COMPILER_WRITE_BARRIER();
   task_terminate_self_unlock_cpu(t);
   __builtin_unreachable();
  }
#endif

  /* Remove the task from the scheduler ring. */
  cpu_del_running(t);
  break;

 }
 assert(t != THIS_TASK);
 ATOMIC_WRITE(t->t_exitcode,exitcode);
 ATOMIC_WRITE(t->t_mode,TASKMODE_TERMINATED);
 cpu_endwrite(c);

 /* Broadcast the join-signal. */
 COMPILER_BARRIER();
 sig_broadcast(&t->t_event);
 COMPILER_BARRIER();

 task_is_terminating(t);
 TASK_DECREF(t);
end:
 return error;
}


PUBLIC errno_t KCALL
task_terminate(struct task *__restrict t,
               void *exitcode) {
 struct cpu *c; pflag_t was;
 errno_t error;
 task_crit();
 was = PREEMPTION_PUSH();
 for (;;) {
  c = TASK_CPU(t);
  COMPILER_READ_BARRIER();
  cpu_write(c);
  if (TASK_CPU(t) == c) break;
  cpu_endwrite(c);
 }
 error = task_terminate_cpu_endwrite(c,t,exitcode);
 PREEMPTION_POP(was);
 task_endcrit();
 return error;
}



PRIVATE SAFE void KCALL
task_signal_stop_cpu_endwrite(struct cpu *__restrict c,
                              struct task *__restrict t,
                              pflag_t was) {
 struct task *parent;
 CHECK_HOST_DOBJ(c);
 CHECK_HOST_DOBJ(t);
 assert(c == TASK_CPU(t));
 assert(!PREEMPTION_ENABLED());
 assert(cpu_writing(c));

 /* Lookup this task's parent process. */
 atomic_rwlock_read(&t->t_pid.tp_parlock);
 parent = t->t_pid.tp_parent;
 assert(parent);
 TASK_INCREF(parent);
 atomic_rwlock_endread(&t->t_pid.tp_parlock);
 t->t_flags |= TASKFLAG_SIGSTOPCONT;
 COMPILER_WRITE_BARRIER();

 if (parent != t) {
  siginfo_t info;
  memset(&info,0,sizeof(siginfo_t));
  info.si_signo = SIGCHLD;
  info.si_pid   = THIS_NAMESPACE == t->t_pid.tp_ids[PIDTYPE_PID].tl_ns ? GET_THIS_PID() : 0;
  info.si_uid   = GET_THIS_UID();
  info.si_code  = CLD_STOPPED;
  /* Raise the SIGCHLD in the parent process. */
  task_kill2_cpu_endwrite(t,&info,0,0,was);
 } else {
  cpu_endwrite(c);
  PREEMPTION_POP(was);
 }
 TASK_DECREF(parent);
}
PRIVATE SAFE void KCALL
task_signal_cont_cpu_endwrite(struct cpu *__restrict c,
                              struct task *__restrict t,
                              pflag_t was) {
 struct task *parent;
 CHECK_HOST_DOBJ(c);
 CHECK_HOST_DOBJ(t);
 assert(c == TASK_CPU(t));
 assert(!PREEMPTION_ENABLED());
 assert(cpu_writing(c));

 /* Lookup this task's parent process. */
 atomic_rwlock_read(&t->t_pid.tp_parlock);
 parent = t->t_pid.tp_parent;
 assert(parent);
 TASK_INCREF(parent);
 atomic_rwlock_endread(&t->t_pid.tp_parlock);
 t->t_flags |= TASKFLAG_SIGSTOPCONT;
 COMPILER_WRITE_BARRIER();

 if (parent != t) {
  siginfo_t info;
  memset(&info,0,sizeof(siginfo_t));
  info.si_signo = SIGCHLD;
  info.si_pid   = THIS_NAMESPACE == t->t_pid.tp_ids[PIDTYPE_PID].tl_ns ? GET_THIS_PID() : 0;
  info.si_uid   = GET_THIS_UID();
  info.si_code  = CLD_CONTINUED;
  /* Raise the SIGCHLD in the parent process. */
  task_kill2_cpu_endwrite(t,&info,0,0,was);
 } else {
  cpu_endwrite(c);
  PREEMPTION_POP(was);
 }
 TASK_DECREF(parent);
}

PUBLIC errno_t KCALL
task_suspend_cpu_endwrite(struct task *__restrict t, u32 mode, pflag_t was) {
 errno_t result = -EOK; s32 *counter;
 struct cpu *c = TASK_CPU(t);
 assert(cpu_writing(c));
 assert(!PREEMPTION_ENABLED());
 assert(t != &c->c_idle);
 assertf((mode&(TASK_SUSP_HOST|TASK_SUSP_NOW)) != (TASK_SUSP_HOST|TASK_SUSP_NOW),
         "suspend-now using host-recursion is unsafe as many "
         "components rely on recursive suspend/resume");

#if TASK_SUSP_HOST == 1
 counter = &t->t_suspend[mode&TASK_SUSP_HOST];
#else
 counter = &t->t_suspend[mode&TASK_SUSP_HOST ? 1 : 0];
#endif
 if (mode&TASK_SUSP_NOW) *counter = 1;
 else if ((++*counter,TASK_ISSUSPENDED(t))) {
  /* Still suspended after recursion. */
  cpu_endwrite(c);
  PREEMPTION_POP(was);
  return -EOK;
 }

 switch (t->t_mode) {

 case TASKMODE_TERMINATED:
  t->t_suspend[0] = 0;
  t->t_suspend[1] = 0;
  result = -EINVAL;
  break;

 case TASKMODE_SLEEPING:
  /* Mark the task as using a timeout. */
  t->t_flags |= TASKFLAG_SUSP_TIMED;
 case TASKMODE_WAKEUP:
  cpu_del_sleeping(c,t);
  ATOMIC_WRITE(t->t_mode,TASKMODE_SUSPENDED);
  cpu_add_suspended(c,t);
  break;

 case TASKMODE_RUNNING:
#ifdef CONFIG_SMP
  if (c != THIS_CPU) {
   /* TODO: Send an RPC command to suspend a remote task. */
   result = -ECOMM;

   if (E_ISERR(result)) --*counter;
  } else
#endif
  {
   bool is_this_task = (t == THIS_TASK);
   cpu_del_running(t);
   ATOMIC_WRITE(t->t_mode,TASKMODE_SUSPENDED);
   cpu_add_suspended(c,t);

   /* Switch to the new CPU if necessary. */
   if (is_this_task) {
    assert(!PREEMPTION_ENABLED());
    TASK_SWITCH_CONTEXT(t,THIS_CPU->c_running);
    task_signal_stop_cpu_endwrite(c,t,was);
    cpu_sched_setrunning_save(t);
    return -EOK;
   }
  }
  break;

 default:
  break;
 }
 task_signal_stop_cpu_endwrite(c,t,was);
 return result;
}
PUBLIC errno_t KCALL
task_resume_cpu_endwrite(struct task *__restrict t, u32 mode, pflag_t was) {
 errno_t result = -EOK; s32 *counter;
 struct cpu *c = TASK_CPU(t); bool was_suspended;
 assert(cpu_writing(c));
 assert(t != THIS_TASK);
 assert(!PREEMPTION_ENABLED());
 assertf((mode&(TASK_SUSP_HOST|TASK_SUSP_NOW)) != (TASK_SUSP_HOST|TASK_SUSP_NOW),
         "suspend-now using host-recursion is unsafe as many "
         "components rely on recursive suspend/resume");

#if TASK_SUSP_HOST == 1
 counter = &t->t_suspend[mode&TASK_SUSP_HOST];
#else
 counter = &t->t_suspend[mode&TASK_SUSP_HOST ? 1 : 0];
#endif
 was_suspended = TASK_ISSUSPENDED(t);
 if (mode&TASK_SUSP_NOW) *counter = 0;
 else {
  assertf(*counter > 0 || !(mode&TASK_SUSP_HOST),
          "Illegal recursion: Host-level suspension cannot be negative "
          "(aka. you can't resume a task that isn't suspended using kernel recursion)");
  --*counter;
 }

 if (!was_suspended) {
  /* Wasn't suspended to begin with. */
  cpu_endwrite(c);
  PREEMPTION_POP(was);
  return -EOK;
 }

 switch (t->t_mode) {

 case TASKMODE_TERMINATED:
  t->t_suspend[0] = 0;
  t->t_suspend[1] = 0;
  result = -EINVAL;
  break;

 case TASKMODE_SUSPENDED:
  /* Suspended task. */
  if ((t->t_flags&(TASKFLAG_TIMEDOUT|TASKFLAG_SUSP_TIMED)) ==
                                    (TASKFLAG_SUSP_TIMED)) {
   struct timespec now; sysrtc_get(&now);
   /* Check for a timeout. */
   if (TIMESPEC_GREATER_EQUAL(now,t->t_timeout))
       t->t_flags |= TASKFLAG_TIMEDOUT;
  }
  t->t_flags &= ~(TASKFLAG_SUSP_TIMED);

  /* The task timed out, or was interrupted. - Wake it up for that! */
  if (t->t_flags&(TASKFLAG_TIMEDOUT|TASKFLAG_INTERRUPT))
      goto set_running;
  /* The task has received a signal. - Wake it up for that! */
  if (t->t_signals.ts_recv != NULL)
      goto set_running;
  break;
set_running:
  /* Switch the task's state to RUNNING. */
  task_set_running(t);
  break;

 default:
  break;
 }
 task_signal_cont_cpu_endwrite(c,t,was);
 return result;
}

PUBLIC errno_t KCALL task_suspend(struct task *__restrict t, u32 mode) {
 struct cpu *c; pflag_t was;
 CHECK_HOST_DOBJ(t);
 was = PREEMPTION_PUSH();
 for (;;) {
  c = TASK_CPU(t);
  COMPILER_READ_BARRIER();
  cpu_write(c);
  if (TASK_CPU(t) == c) break;
  cpu_endwrite(c);
 }
 return task_suspend_cpu_endwrite(t,mode,was);
}
PUBLIC errno_t KCALL task_resume(struct task *__restrict t, u32 mode) {
 struct cpu *c; pflag_t was;
 CHECK_HOST_DOBJ(t);
 was = PREEMPTION_PUSH();
 for (;;) {
  c = TASK_CPU(t);
  COMPILER_READ_BARRIER();
  cpu_write(c);
  if (TASK_CPU(t) == c) break;
  cpu_endwrite(c);
 }
 return task_resume_cpu_endwrite(t,mode,was);
}


PUBLIC errno_t KCALL
task_set_priority(struct task *__restrict t,
                  taskprio_t   new_priority,
                  taskprio_t *pold_priority) {
 struct cpu *c; pflag_t was;
 errno_t error = -EOK;
 CHECK_HOST_DOBJ(t);
 was = PREEMPTION_PUSH();
 for (;;) {
  c = TASK_CPU(t);
  COMPILER_WRITE_BARRIER();
  cpu_write(c);
  if (TASK_CPU(t) == c) break;
  cpu_endwrite(c);
 }

 switch (t->t_mode) {

 case TASKMODE_TERMINATED:
  error = -EINVAL;
  goto end;

 default:
  if (pold_priority) *pold_priority = t->t_priority;
  t->t_priority = new_priority;
  goto end;

 case TASKMODE_RUNNING:
  break;
 }

#ifdef CONFIG_SMP
 if (c != THIS_CPU) {
  /* TODO: Send an RPC command to re-prioritize a remote task. */
  error = -ECOMM;
 } else
#endif
 {
  taskprio_t old_priority = t->t_priority;
  if (pold_priority) *pold_priority = old_priority;
  //t->t_priority = new_priority;
  /* TODO: Fix priority chains/order. */
  error = -EINVAL;

 }
end:
 PREEMPTION_POP(was);
 cpu_endwrite(c);
 return error;
}

PUBLIC SAFE errno_t KCALL
task_pause_cpu_endwrite(struct timespec const *abstime) {
 REF struct task *caller; errno_t error = -EINTR;
 assert(!PREEMPTION_ENABLED());
 assert(cpu_writing(THIS_CPU));
 assert(TASK_CPU(THIS_TASK) == THIS_CPU);
 assert(THIS_CPU->c_n_run >= 1);
 assertf((THIS_CPU->c_n_run == 1) == (THIS_TASK->t_sched.sd_running.re_prev == THIS_TASK),
         "THIS_CPU->c_n_run = %Iu",THIS_CPU->c_n_run);
 assert((THIS_CPU->c_n_run == 1) == (THIS_TASK->t_sched.sd_running.re_next == THIS_TASK));

 /* Unschedule the calling thread. */
 caller = cpu_sched_remove_current();

 if (abstime) {
  /* Re-schedule as an interruptible sleeper. */
  caller->t_timeout = *abstime;
  ATOMIC_WRITE(caller->t_mode,TASKMODE_SLEEPING);
  cpu_add_sleeping(THIS_CPU,caller);
 } else {
  /* Re-schedule as an interruptible suspended. */
  caller->t_timeout.tv_nsec = 0;
  caller->t_timeout.tv_sec  = 0;
  ATOMIC_WRITE(caller->t_mode,TASKMODE_SUSPENDED);
  cpu_add_suspended(THIS_CPU,caller);
 }
 TASK_SWITCH_CONTEXT(caller,THIS_CPU->c_running);
 cpu_endwrite(THIS_CPU);

 /* Switch to the next thread. */
 cpu_validate_counters(true);
 cpu_sched_setrunning_save(caller);
 cpu_validate_counters(true);

 /* Once we're here, we've been interrupted. */
 assert(!PREEMPTION_ENABLED());
 assert(THIS_TASK == caller);
 assert(caller->t_flags&(TASKFLAG_INTERRUPT|TASKFLAG_TIMEDOUT));
 assert(caller->t_flags&TASKFLAG_INTERRUPT || abstime);
 /* Consume the interrupt. */
 if (caller->t_flags&TASKFLAG_TIMEDOUT)
     error = -ETIMEDOUT;
 caller->t_flags &= ~(TASKFLAG_INTERRUPT|TASKFLAG_TIMEDOUT);
 return error;
}


PUBLIC bool (KCALL task_issafe)(void) { return task_issafe(); }
PUBLIC bool (KCALL task_iscrit)(void) { return task_iscrit(); }
PUBLIC void (KCALL task_crit)(void) { task_crit(); }
PUBLIC ATTR_HOTTEXT CRIT void (KCALL task_endcrit)(void) {
 struct cpu *c; pflag_t was;
 struct task *t = THIS_TASK;
 CHECK_HOST_DOBJ(t);
 assertf(t->t_critical != 0,"Missing 'task_crit()'");
 if (t->t_critical > 1) { --t->t_critical; return; }
 /* End of a critical block. */
 was = PREEMPTION_PUSH();
 for (;;) {
  c = TASK_CPU(t);
  COMPILER_WRITE_BARRIER();
  cpu_write(c);
  if (TASK_CPU(t) == c) break;
  cpu_endwrite(c);
 }

 /* Bye Bye! */
 if (t->t_flags&TASKFLAG_WILLTERM)
     task_terminate_self_unlock_cpu(t);

 if (TASKPRIO_ISIDLE(t->t_priority) &&
    !TASKPRIO_ISIDLE(c->c_prio_max)) {
  /* Must park this one and all other IDLE tasks.
   * While this task was critical before, it couldn't be parked! */
  --t->t_critical;
  COMPILER_WRITE_BARRIER();
  assertf(c->c_prio_min <= c->c_prio_max,
          "c->c_prio_min = %x\n"
          "c->c_prio_max = %x\n",
          c->c_prio_min,c->c_prio_max);
  cpu_park_idle();
  COMPILER_WRITE_BARRIER();
  assertf(c->c_running != NULL,
          "But then how come there was more than one priority?");
  assertf(c->c_running->t_priority >= c->c_prio_min &&
          c->c_running->t_priority <= c->c_prio_max,
          "c->c_running->t_priority = %x\n"
          "c->c_prio_min            = %x\n"
          "c->c_prio_max            = %x\n",
          c->c_running->t_priority,
          c->c_prio_min,c->c_prio_max);
  cpu_endwrite(c);

  /* Switch to the next task if we've just got parked. */
  if (THIS_CPU->c_running != t) {
   TASK_SWITCH_CONTEXT(t,THIS_CPU->c_running);
   cpu_sched_setrunning_savef(t,was);
   return;
  }
  PREEMPTION_POP(was);
  return;
 }

 cpu_endwrite(c);
 PREEMPTION_POP(was);
 --t->t_critical;
}

PUBLIC errno_t KCALL task_testintr(void) {
 errno_t result = -EOK;
 struct task *t = THIS_TASK;
 if (!t->t_nointr &&
    (ATOMIC_FETCHAND(t->t_flags,~TASKFLAG_INTERRUPT)&TASKFLAG_INTERRUPT))
     result = -EINTR;
 return result;
}

DECL_END

#ifndef __INTELLISENSE__
#include "pid.c.inl"
#endif

#endif /* !GUARD_KERNEL_SCHED_TASK_C */
