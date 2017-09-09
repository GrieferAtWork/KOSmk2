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
#define _KOS_SOURCE 2

#include "../debug-config.h"

#include <hybrid/align.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/critical.h>
#include <hybrid/limits.h>
#include <hybrid/traceback.h>
#include <kernel/arch/cpustate.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/syslog.h>
#include <kernel/stack.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/task.h>
#include <string.h>
#include <sched/paging.h>
#include <sys/mman.h>
#include <hybrid/minmax.h>

DECL_BEGIN

//#undef CONFIG_SMP

STATIC_ASSERT(sizeof(struct cpu)                 == CPU_SIZE);
STATIC_ASSERT(sizeof(struct task)                == TASK_SIZE);
STATIC_ASSERT(offsetof(struct task,t_ic)         == TASK_OFFSETOF_IC);
STATIC_ASSERT(offsetof(struct task,t_addrlimit)  == TASK_OFFSETOF_ADDRLIMIT);
STATIC_ASSERT(offsetof(struct task,t_ustack)     == TASK_OFFSETOF_USTACK);
STATIC_ASSERT(offsetof(struct task,t_hstack)     == TASK_OFFSETOF_HSTACK);
STATIC_ASSERT(offsetof(struct task,t_affinity)   == TASK_OFFSETOF_AFFINITY);
STATIC_ASSERT(offsetof(struct task,t_mman_tasks) == TASK_OFFSETOF_MMAN_TASKS);


INTERN void KCALL
task_service_interrupt(struct task *__restrict self,
                       signal_t interrupt_signal) {
 /* XXX: raise(interrupt_signal); */
}

PUBLIC errno_t KCALL
task_start(struct task *__restrict self) {
 errno_t error = -EOK;
 struct cpu *task_cpu;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->t_cstate);
 assert(self->t_mode == TASKMODE_RUNNING);
 CHECK_HOST_DOBJ(self->t_cpu);
 assert(!self->t_signals.ts_first.tss_sig);
 /* Generic initialization. */
 atomic_rwlock_cinit(&self->t_signals.ts_lock);
 self->t_signals.ts_first.tss_self = self;
 sig_cinit(&self->t_join);
 assert(self->t_refcnt == 0);
 assert(self->t_critical == 0);
 assert(self->t_nointr == 0);
 assert(self->t_ic == NULL);
 assert(self->t_suspend == 0);
 assert(self->t_prioscore == 0);
 CHECK_HOST_DOBJ(self->t_mman);
 self->t_addrlimit = KERNEL_BASE;
 self->t_refcnt = 2; /* One for the caller + one for the cpu. */
 /* Schedule the task for its first time. */
#ifdef CONFIG_SMP
 task_cpu = self->t_cpu;
 if (task_cpu != THIS_CPU) {
  atomic_rwlock_write(&task_cpu->c_lock);
  LIST_INSERT(task_cpu->c_wakeup,self,t_sched.sd_wakeup);
  atomic_rwlock_endwrite(&task_cpu->c_lock);
 } else
#endif
 {
  bool was = PREEMPTION_ENABLED();
  PREEMPTION_DISABLE();
  /* Schedule the task to run next/last, based on
   * its priority when compared to the caller. */
  if (self->t_priority >= THIS_CPU->c_running->t_priority)
       RING_INSERT_AFTER(THIS_CPU->c_running,self,t_sched.sd_running);
  else RING_INSERT_BEFORE(THIS_CPU->c_running,self,t_sched.sd_running);
#ifdef CONFIG_SMP
  atomic_rwlock_write(&self->t_mman->m_tasks_lock);
  LIST_INSERT(self->t_mman->m_tasks,self,t_mman_tasks);
  atomic_rwlock_endwrite(&self->t_mman->m_tasks_lock);
#endif
  if (was) PREEMPTION_ENABLE();
 }
 return error;
}

PUBLIC void KCALL
task_destroy(struct task *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(!self->t_critical);
#ifdef CONFIG_SMP
 assert(self->t_mode == self->t_realstate);
#endif
 assert(self->t_mode == TASKMODE_NOTSTARTED ||
        self->t_mode == TASKMODE_TERMINATED);
 assert(IS_ALIGNED((uintptr_t)self->t_hstack.hs_begin,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)self->t_hstack.hs_end,  PAGESIZE));
 assertf(self != &__boottask,"Cannot destroy boot task (like this)");
 assertf(self != &self->t_cpu->c_idle,"Cannot destroy cpu IDLE-task");

 CHECK_HOST_DOBJ(self->t_mman);

 /* Unmap host+user stacks. */
 task_nointr();
 MMAN_LOCK_WRITE(&mman_kernel);
 /* NOTE: We use the address of the thread as
  *       tag when unmapping kernel stacks! */
 mman_munmap_unlocked(&mman_kernel,
                      self->t_hstack.hs_begin,
                     (uintptr_t)self->t_hstack.hs_end-
                     (uintptr_t)self->t_hstack.hs_begin,
                      MMAN_MUNMAP_TAG,self);
 if (self->t_mman != &mman_kernel) {
  /* Switch locks to the task's memory manager. */
  MMAN_LOCK_ENDWRITE(&mman_kernel);
  MMAN_LOCK_WRITE(self->t_mman);
 }
 /* Unmap a potential user-stack. */
 if (self->t_ustack)
     mman_munmap_stack_unlocked(self->t_mman,self->t_ustack);

 /* XXX: Unmap thread-local data? */
 MMAN_LOCK_ENDWRITE(self->t_mman);
 task_endnointr();

 /* Drop references. */
 if (self->t_ustack)
     STACK_DECREF(self->t_ustack);
 MMAN_DECREF(self->t_mman);
 free(self);
}

#ifdef CONFIG_DEBUG
#define CONFIG_HOST_STACKINIT KERNEL_DEBUG_MEMPAT_HOSTSTACK
#endif

PUBLIC errno_t KCALL
task_mkhstack(struct task *__restrict self, size_t n_bytes) {
 REF struct mregion *stack_region; struct mman *old_mman;
 ppage_t stack_addr; bool has_write_lock; errno_t error = -EOK;
 CHECK_HOST_DOBJ(self);
 TASK_PDIR_KERNEL_BEGIN(old_mman);
 stack_region = (REF struct mregion *)kcalloc(sizeof(struct mregion),
                                              MMAN_DATAGFP(&mman_kernel));
 if unlikely(!stack_region) goto nomem;
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 assert(n_bytes != 0);
 stack_region->mr_refcnt = 1;
#ifdef CONFIG_HOST_STACKINIT
 /* Use debug pre-initialization of stack memory. */
 stack_region->mr_init           = MREGION_INIT_BYTE;
 stack_region->mr_setup.mri_byte = CONFIG_HOST_STACKINIT;
#endif
 stack_region->mr_size = n_bytes;
 atomic_rwptr_cinit(&stack_region->mr_futex);
 rwlock_cinit(&stack_region->mr_plock);
 stack_region->mr_parts                   = &stack_region->mr_part0;
 stack_region->mr_part0.mt_chain.le_pself = &stack_region->mr_parts;
 /* Force the region to be loaded in-core, and be locked there! */
 stack_region->mr_part0.mt_state  = MPART_STATE_INCORE;
 stack_region->mr_part0.mt_locked = 1;
 /* Allocate the initial physical memory for the stack region. */
 if unlikely(!page_malloc_scatter(&stack_region->mr_part0.mt_memory,
                                   n_bytes,PAGESIZE,PAGEATTR_NONE,
                                   MZONE_ANY)) {
  free(stack_region);
  goto nomem;
 }
#ifdef CONFIG_HOST_STACKINIT
 { struct mscatter *iter = &stack_region->mr_part0.mt_memory;
   while (iter) {
    memsetl(iter->m_start,CONFIG_HOST_STACKINIT,iter->m_size/4);
    iter = iter->m_next;
   }
 }
#endif

 /* perform final setup on the region. */
 mregion_setup(stack_region);
 /* At this point, we've created the stack region! */

 has_write_lock = false;
 error = MMAN_LOCK_READ(&mman_kernel);
 if (E_ISERR(error)) goto end_region;
reload_spc:
 /* Search for suitable stack. */
 stack_addr = mman_findspace_unlocked(&mman_kernel,
                                     (ppage_t)(TASK_HOSTSTACK_ADDRHINT-n_bytes),n_bytes,
                                      MAX(TASK_HOSTSTACK_ALIGN,PAGESIZE),0,
                                      MMAN_FINDSPACE_BELOW);
 if unlikely(stack_addr == PAGE_ERROR) {
  /* Search above the previous search range. (This could get bad...) */
  stack_addr = mman_findspace_unlocked(&mman_kernel,
                                      (ppage_t)(TASK_HOSTSTACK_ADDRHINT-n_bytes),n_bytes,
                                       MAX(TASK_HOSTSTACK_ALIGN,PAGESIZE),0,
                                       MMAN_FINDSPACE_ABOVE);
  if unlikely(stack_addr == PAGE_ERROR) goto nospc;
 }
 if (!has_write_lock) {
  error = MMAN_LOCK_UPGRADE(&mman_kernel);
  has_write_lock = true;
  if (E_ISERR(error)) {
   if (error == -ERELOAD) goto reload_spc;
   goto end_region;
  }
 }
 /* At this point we've got a suitable address to allocate the kernel stack at! */
 assert(stack_addr != PAGE_ERROR);
 self->t_hstack.hs_begin = stack_addr;
 self->t_hstack.hs_end   = (ppage_t)((uintptr_t)stack_addr+n_bytes);
 /* Now we just have to map the stack! */
 error = mman_mmap_unlocked(&mman_kernel,stack_addr,n_bytes,0,stack_region,
                             PROT_READ|PROT_WRITE|PROT_NOUSER,NULL,self);
 MMAN_LOCK_ENDWRITE(&mman_kernel);
end_region: MREGION_DECREF(stack_region);
end: TASK_PDIR_KERNEL_END(old_mman);
 return error;
nospc: error = -ENOMEM;
 if (has_write_lock) MMAN_LOCK_ENDWRITE(&mman_kernel);
 else                MMAN_LOCK_ENDREAD (&mman_kernel);
 goto end_region;
nomem: error = -ENOMEM; goto end;
}



PRIVATE void KCALL
task_clear_signals(struct task *__restrict self,
                   struct sig *caller_locked) {
 struct sig *s;
 struct tasksigslot *slot;
 CHECK_HOST_DOBJ(self);
 assert(atomic_rwlock_writing(&self->t_signals.ts_lock));
again:
 if ((s = self->t_signals.ts_first.tss_sig) != NULL) {
  if (s == caller_locked) {
   self->t_signals.ts_first.tss_sig = NULL;
   LIST_REMOVE(&self->t_signals.ts_first,tss_chain);
  } else {
   atomic_rwlock_endwrite(&self->t_signals.ts_lock);
   sig_write(s);
   atomic_rwlock_write(&self->t_signals.ts_lock);
   if (s != self->t_signals.ts_first.tss_sig) goto again;
   self->t_signals.ts_first.tss_sig = NULL;
   LIST_REMOVE(&self->t_signals.ts_first,tss_chain);
   sig_endwrite(s);
  }
 }
 while (self->t_signals.ts_slotc) {
  assert(self->t_signals.ts_slotc <=
         self->t_signals.ts_slota);
  slot = self->t_signals.ts_slotv;
  while ((assert(slot < self->t_signals.ts_slotv+
                        self->t_signals.ts_slota),
         (s = slot->tss_sig) == NULL)) ++slot;
  if (s == caller_locked) {
   --self->t_signals.ts_slotc;
   slot->tss_sig = NULL;
   LIST_REMOVE(slot,tss_chain);
  } else {
   *(uintptr_t *)&slot -= (uintptr_t)self->t_signals.ts_slotv;
   atomic_rwlock_endwrite(&self->t_signals.ts_lock);
   sig_write(s);
   atomic_rwlock_write(&self->t_signals.ts_lock);
   *(uintptr_t *)&slot += (uintptr_t)self->t_signals.ts_slotv;
   if (slot >= self->t_signals.ts_slotv+
               self->t_signals.ts_slota) goto again;
   if (s != slot->tss_sig) { sig_endwrite(s); goto again; }
   --self->t_signals.ts_slotc;
   slot->tss_sig = NULL;
   LIST_REMOVE(slot,tss_chain);
   sig_endwrite(s);
   if (self->t_signals.ts_first.tss_sig)
       goto again;
  }
 }
}

PRIVATE errno_t
task_addsignal(struct task *__restrict self,
               struct sig *__restrict sig) {
 struct tasksigslot *slot,*end;
 assert(atomic_rwlock_writing(&self->t_signals.ts_lock));
 assert(atomic_rwlock_writing(&sig->s_lock));
 slot = &self->t_signals.ts_first;
 if (!self->t_signals.ts_first.tss_sig) {
  slot = &self->t_signals.ts_first;
  goto use_slot;
 }
 ++self->t_signals.ts_slotc;
 /* Allocate a new signal slot. */
 end = (slot = self->t_signals.ts_slotv)+
               self->t_signals.ts_slota;
 for (; slot != end; ++slot)
  if (!slot->tss_sig) goto use_slot;
 {
  size_t oldalloc,newalloc;
  oldalloc = newalloc = self->t_signals.ts_slota;
  if (!newalloc) newalloc = 1;
  newalloc *= 2;
realloc_again:
  slot = trealloc(struct tasksigslot,
                  self->t_signals.ts_slotv,
                  newalloc);
  if unlikely(!slot) {
   if (newalloc != self->t_signals.ts_slota-1) {
    newalloc = self->t_signals.ts_slota-1;
    goto realloc_again;
   }
   assert(self->t_signals.ts_slotc);
   --self->t_signals.ts_slotc;
   return -ENOMEM;
  }
  self->t_signals.ts_slota = newalloc;
  self->t_signals.ts_slotv = slot;
  while (oldalloc < newalloc) {
   /* Initialize newly allocated slots. */
   slot[oldalloc].tss_self = self;
   slot[oldalloc].tss_sig  = NULL;
   ++oldalloc;
  }
  slot += oldalloc;
 }
use_slot:
 assert(slot->tss_self == self);
 slot->tss_sig           = sig;
 slot->tss_chain.le_next = NULL;
 if (!sig->s_task) {
  sig->s_task              = slot;
  slot->tss_last           = slot;
  slot->tss_chain.le_pself = &sig->s_task;
 } else {
#ifdef CONFIG_DEBUG
  slot->tss_last = NULL;
#endif
  CHECK_HOST_DOBJ(sig->s_task);
  CHECK_HOST_DOBJ(sig->s_task->tss_last);
  LIST_INSERT_AFTER(sig->s_task->tss_last,slot,tss_chain);
  sig->s_task->tss_last = slot;
 }
 return -EOK;
}





PUBLIC size_t KCALL
sig_send(struct sig *__restrict sig,
         size_t max_threads) {
 size_t result;
 CHECK_HOST_DOBJ(sig);
 sig_write(sig);
 result = sig_vsend_unlocked(sig,NULL,0,max_threads);
 sig_endwrite(sig);
 return result;
}
PUBLIC size_t KCALL
sig_send_unlocked(struct sig *__restrict sig,
                  size_t max_threads) {
 return sig_vsend_unlocked(sig,NULL,0,max_threads);
}
PUBLIC size_t KCALL
sig_vsend(struct sig *__restrict sig,
          void *msg, size_t msgsize,
          size_t max_threads) {
 size_t result;
 CHECK_HOST_DOBJ(sig);
 sig_write(sig);
 result = sig_vsend_unlocked(sig,msg,msgsize,max_threads);
 sig_endwrite(sig);
 return result;
}
PUBLIC size_t KCALL
sig_vsend_unlocked(struct sig *__restrict sig,
                   void *msg, size_t msgsize,
                   size_t max_threads) {
 size_t result;
 CHECK_HOST_DOBJ(sig);
 assert(atomic_rwlock_writing(&sig->s_lock));
 while (max_threads-- &&
        sig_vsendone_unlocked(sig,msg,msgsize))
        ++result;
 return result;
}

PUBLIC bool KCALL
sig_vsendone_unlocked(struct sig *__restrict sig,
                      void const *data, size_t datasize) {
 struct tasksigslot *slot;
 struct task *thread;
 struct cpu *task_cpu;
 CHECK_HOST_DOBJ(sig);
 assert(atomic_rwlock_writing(&sig->s_lock));
 if (!sig->s_task) return false;
 /* Pop one signal slot and update links to the next. */
 slot = sig->s_task;
 CHECK_HOST_DOBJ(slot);
 assert(slot->tss_chain.le_pself == &sig->s_task);
 assert(slot->tss_sig            == sig);
 thread = slot->tss_self;
 CHECK_HOST_DOBJ(thread);
 task_cpu = TASK_CPULOCK_WRITE(thread);
 sig->s_task = slot->tss_chain.le_next;
 if (sig->s_task) {
  sig->s_task->tss_last           = slot->tss_last;
  sig->s_task->tss_chain.le_pself = &sig->s_task;
 }
#ifdef CONFIG_DEBUG
 slot->tss_last = NULL;
#endif
 atomic_rwlock_write(&thread->t_signals.ts_lock);
 /* Clear all receiver signals. */
 task_clear_signals(thread,sig);
 slot->tss_sig = NULL;
 atomic_rwlock_endwrite(&thread->t_signals.ts_lock);
 /* Check if the thread must be re-scheduled. */
 if (thread->t_mode == TASKMODE_SLEEPING ||
     thread->t_mode == TASKMODE_SUSPENDED) {
  /* Copy signal data. */
  if (thread->t_sigval != NULL) {
   if (data) memcpy(thread->t_sigval,data,datasize);
   else      thread->t_sigval = NULL;
  }
  /* Special case: The thread is explicitly suspended. */
  if (ATOMIC_READ(thread->t_suspend) > 0) {
   thread->t_flags &= ~(TASKFLAG_SUSP_NOCONT);
   goto done;
  }
  /* Remove from the current scheduler chain.
   * NOTE: 'sd_sleeping' / 'sd_suspended' both do the same thing. */
  LIST_REMOVE(thread,t_sched.sd_sleeping);
  /* LIST_REMOVE(thread,t_sched.sd_suspended); */
#ifdef CONFIG_SMP
  /* Re-schedule on a different CPU. */
  if (task_cpu != THIS_CPU) {
   /* Insert into the wakeup chain. */
   LIST_INSERT(task_cpu->c_wakeup,
               thread,t_sched.sd_wakeup);
  } else
#endif
  {
   /* Re-schedule on our own CPU. */
   struct task *tself = task_cpu->c_running;
   bool was = PREEMPTION_ENABLED();
   PREEMPTION_DISABLE();
   assert(tself == THIS_TASK);
   /* Re-schedule the thread to run last. */
   RING_INSERT_BEFORE(tself,thread,t_sched.sd_running);
#ifdef CONFIG_SMP
   thread->t_realstate = TASKMODE_RUNNING;
#endif
   if (was) PREEMPTION_ENABLE();
  }
  thread->t_mode = TASKMODE_RUNNING;
 } else {
  /* Shouldn't happen:
   *  - Sending a signal to a running task.
   *  - Sending a signal to a terminated task. */
 }
done:
 atomic_rwlock_endwrite(&task_cpu->c_lock);
 return true;
}

/* Set the task state of 'self' to suspended and unlock the CPU.
 * NOTE: In the event that 'self == THIS_TASK', this function will
 *       not return until the task has continued.
 * WARNING: The caller is responsible to only pass a running or waking task, as well
 *          as to lock the associated cpu's 'c_lock' lock for writing. */
PRIVATE void KCALL
task_unsched_unlockcpu(struct task *__restrict self,
                       taskmode_t newstate) {
 bool was;
 struct cpu *task_cpu = self->t_cpu;
 assert(newstate == TASKMODE_SLEEPING ||
        newstate == TASKMODE_SUSPENDED);
 assert(atomic_rwlock_writing(&task_cpu->c_lock));
#ifdef CONFIG_SMP
 if (self->t_mode == TASKMODE_WAKEUP) {
  /* Simple case: Remove the task from the wake list again. */
  LIST_REMOVE(self,t_sched.sd_wakeup);
  assert(self->t_realstate != TASKMODE_RUNNING);
  self->t_mode     = newstate;
  self->t_realstate = newstate;
  if (newstate == TASKMODE_SLEEPING)
       CPU_INSERT_SLEEPING (task_cpu,self);
  else CPU_INSERT_SUSPENDED(task_cpu,self);
  atomic_rwlock_endwrite(&task_cpu->c_lock);
  return;
 }
 was = PREEMPTION_ENABLED();
 PREEMPTION_DISABLE();
 if (task_cpu != THIS_CPU) {
  /* Another simple case: Mark the task's state as suspended.
   * The next time it would be preempted, it will be suspended. */
  assert(self != THIS_TASK);
  self->t_mode = newstate;
  atomic_rwlock_endwrite(&task_cpu->c_lock);
  if (was) PREEMPTION_ENABLE();
  return;
 }
#else
 was = PREEMPTION_ENABLED();
 PREEMPTION_DISABLE();
#endif
 assert(self->t_mode == TASKMODE_RUNNING);

 if (self != task_cpu->c_running) {
  assert(THIS_TASK == task_cpu->c_running);
  assert(THIS_TASK != self);
  /* Un-schedule a different task on the current CPU. */
  assertf(self != task_cpu->c_running,
          "But 'THIS_TASK' should be running...");
  RING_REMOVE(self,t_sched.sd_running);
  self->t_mode     = newstate;
#ifdef CONFIG_SMP
  self->t_realstate = newstate;
#endif
  if (newstate == TASKMODE_SLEEPING)
       CPU_INSERT_SLEEPING (task_cpu,self);
  else CPU_INSERT_SUSPENDED(task_cpu,self);
  if (was) PREEMPTION_ENABLE();
  atomic_rwlock_endwrite(&task_cpu->c_lock);
  return;
 }
 /* Special case: Unschedule the current task. */
 assert(self == THIS_TASK);
 assert(self == task_cpu->c_running);
 /* Rotate to select a new running task. */
 cpu_sched_rotate_selectnew(task_cpu);
 assert(self != task_cpu->c_running);
 RING_REMOVE(self,t_sched.sd_running);
 /* Re-insert the into the proper CPU chain. */
 self->t_mode     = newstate;
#ifdef CONFIG_SMP
 self->t_realstate = newstate;
#endif
 if (newstate == TASKMODE_SLEEPING)
      CPU_INSERT_SLEEPING (task_cpu,self);
 else CPU_INSERT_SUSPENDED(task_cpu,self);
 atomic_rwlock_endwrite(&task_cpu->c_lock);
 {
  struct mman *new_mman = task_cpu->c_running->t_mman;
  if (self->t_mman != new_mman) {
   /* Switch to the new task's page directory. */
#if 1
   assert(!memcmp(&new_mman->m_pdir.pd_directory[KERNEL_BASE/PDTABLE_SIZE],
                  &pdir_kernel_v.pd_directory[KERNEL_BASE/PDTABLE_SIZE],
                ((0-KERNEL_BASE)/PDTABLE_SIZE)*sizeof(union pd_table)));
#endif
   __asm__ __volatile__("movl %0, %%cr3\n"
                        :
                        : "r" (&new_mman->m_ppdir->pd_directory)
                        : "memory");
  }
 }
 assert(!PREEMPTION_ENABLED());
 cpu_sched_setrunning_save(self);
 if (was) PREEMPTION_ENABLE();
}

PUBLIC errno_t KCALL
sig_timedrecv(struct sig *__restrict self,
              struct timespec const *abstime) {
 CHECK_HOST_DOBJ(self);
 sig_write(self);
 return sig_vtimedrecv_endwrite(self,NULL,abstime);
}
PUBLIC errno_t KCALL
sig_timedrecv_endwrite(struct sig *__restrict self,
                       struct timespec const *abstime) {
 return sig_vtimedrecv_endwrite(self,NULL,abstime);
}
PUBLIC errno_t KCALL
sig_vtimedrecv(struct sig *__restrict self,
               void *msg_buf, struct timespec const *abstime) {
 CHECK_HOST_DOBJ(self);
 sig_write(self);
 return sig_vtimedrecv_endwrite(self,msg_buf,abstime);
}
PUBLIC errno_t KCALL
sig_vtimedrecv_endwrite(struct sig *__restrict self,
                        void *msg_buf, struct timespec const *abstime) {
 struct sigset set;
 CHECK_HOST_DOBJ(self);
 assert(atomic_rwlock_writing(&self->s_lock));
 SIGSET_SETONE(set,self);
 return task_waitforany_endwrite(THIS_TASK,&set,abstime,msg_buf);
}

/* General purpose unlock-and-wait-for-signals-with-optional-timeout-and-v-buffer.
 * >> This is the top-level kernel call that implements all signal-based scheduling. */
PUBLIC errno_t KCALL
task_waitforany_endwrite(struct task *__restrict self,
                         struct sigset *__restrict signals,
                         struct timespec const *abstime,
                         void *msg_buf) {
 bool has_siglock = false;
 errno_t error = -EOK;
 struct cpu *task_cpu;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(signals);
 /*__assertion_tbprint(0);*/
 task_cpu = TASK_CPULOCK_WRITE(self);
 /* Check for pending interrupts. */
 if (self->t_flags&TASKFLAG_INTERRUPT &&
     self->t_nointr == 0) {
  signal_t intno = self->t_intno;
  bool call_service = !(self->t_flags&TASKFLAG_INTERRUPT_NOSERVE);
  self->t_flags &= ~(TASKFLAG_INTERRUPT|TASKFLAG_INTERRUPT_NOSERVE);
  atomic_rwlock_endwrite(&task_cpu->c_lock);
  if (call_service) task_service_interrupt(self,intno);
  return -EINTR;
 }
 for (;;) {
  struct sig *signal;
  size_t i,c = signals->ss_elemcnt;
  if (c) {
   if (!has_siglock) {
    has_siglock = 1;
    atomic_rwlock_write(&self->t_signals.ts_lock);
   }
   if (signals->ss_elemsiz) for (i = 0; i < c; ++i) {
    signal = (struct sig *)((uintptr_t)signals->ss_eleminl+i*signals->ss_elemsiz);
    CHECK_HOST_DOBJ(signal);
    //syslogf(LOG_DEBUG,"Unlocking: %p\n",signal);
    if (E_ISOK(error)) error = task_addsignal(self,signal);
    sig_endwrite(signal);
   } else for (i = 0; i < c; ++i) {
    signal = (struct sig *)((uintptr_t)signals->ss_elemvec[i]+signals->ss_elemoff);
    CHECK_HOST_DOBJ(signal);
    //syslogf(LOG_DEBUG,"Unlocking: %p\n",signal);
    if (E_ISOK(error)) error = task_addsignal(self,signal);
    sig_endwrite(signal);
   }
  }
  signals = signals->ss_more;
  if (!signals) break;
 }
 if (E_ISERR(error)) {
  /* XXX: Only clear new signals? */
  assert(has_siglock);
  task_clear_signals(self,NULL);
  atomic_rwlock_endwrite(&self->t_signals.ts_lock);
  return error;
 }
 if (has_siglock) atomic_rwlock_endwrite(&self->t_signals.ts_lock);
 if (msg_buf) self->t_sigval = msg_buf;
 if (abstime) { CHECK_HOST_TOBJ(abstime); self->t_timeout = *abstime; }
 task_unsched_unlockcpu(self,abstime ? TASKMODE_SLEEPING : TASKMODE_SUSPENDED);
 if (self != THIS_TASK) return -EOK;
 /* Check for special-state flags. */
 atomic_rwlock_read(&task_cpu->c_lock);
 if (!(self->t_flags&(TASKFLAG_INTERRUPT|TASKFLAG_TIMEDOUT))) {
  atomic_rwlock_endread(&task_cpu->c_lock);
 } else {
  /* Update the lock to a write-lock, so we can modify our flags. */
  if (!atomic_rwlock_upgrade(&task_cpu->c_lock)) {
   if (!(self->t_flags&(TASKFLAG_INTERRUPT|TASKFLAG_TIMEDOUT))) {
    atomic_rwlock_endwrite(&task_cpu->c_lock);
    goto noflag;
   }
  }
  if (self->t_flags&TASKFLAG_INTERRUPT && self->t_nointr == 0) {
   signal_t intno = self->t_intno;
   bool call_service = !(self->t_flags&TASKFLAG_INTERRUPT_NOSERVE);
   self->t_flags &= ~(TASKFLAG_INTERRUPT|TASKFLAG_INTERRUPT_NOSERVE);
   atomic_rwlock_endwrite(&task_cpu->c_lock);
   if (call_service) task_service_interrupt(self,intno);
   return -EINTR;
  }
  if (self->t_flags&TASKFLAG_TIMEDOUT) {
   self->t_flags &= ~(TASKFLAG_TIMEDOUT);
   if (abstime) {
    atomic_rwlock_endwrite(&task_cpu->c_lock);
    return -ETIMEDOUT;
   }
  }
  atomic_rwlock_endwrite(&task_cpu->c_lock);
 }
noflag:
 if (msg_buf && !self->t_sigval) error = -ENODATA;
 return error;
}


PUBLIC errno_t KCALL task_testintr(void) {
 errno_t result = -EOK;
 struct task *self = THIS_TASK;
 struct cpu *task_cpu;
 if (self->t_nointr) goto end;
again:
 task_cpu = TASK_CPULOCK_READ(self);
 assert(self->t_nointr == 0);
 /* Check for pending interrupts. */
 if (self->t_flags&TASKFLAG_INTERRUPT) {
  bool call_service; signal_t intno;
  if (!atomic_rwlock_upgrade(&task_cpu->c_lock)) {
   if unlikely(self->t_cpu != task_cpu) { atomic_rwlock_endwrite(&task_cpu->c_lock); goto again; }
   if unlikely(!(self->t_flags&TASKFLAG_INTERRUPT)) { atomic_rwlock_endwrite(&task_cpu->c_lock); goto end; }
  }
  assert(self->t_nointr == 0);
  intno = self->t_intno;
  call_service = !(self->t_flags&TASKFLAG_INTERRUPT_NOSERVE);
  self->t_flags &= ~(TASKFLAG_INTERRUPT|TASKFLAG_INTERRUPT_NOSERVE);
  atomic_rwlock_endwrite(&task_cpu->c_lock);
  if (call_service) task_service_interrupt(self,intno);
  result = -EINTR;
 } else {
  atomic_rwlock_endread(&task_cpu->c_lock);
 }
end:
 return result;
}


PUBLIC errno_t KCALL
task_suspend(struct task *__restrict self) {
 s32 oldval; bool take_action = false;
 errno_t result = -EOK;
 struct cpu *task_cpu;
#ifdef CONFIG_SMP
again:
#endif
 CHECK_HOST_DOBJ(self);
#ifdef CONFIG_SMP
 task_cpu = ATOMIC_READ(self->t_cpu);
#else
 task_cpu = &__bootcpu;
#endif
 do {
  if (take_action) {
   atomic_rwlock_endwrite(&task_cpu->c_lock);
   take_action = false;
  }
  oldval = ATOMIC_READ(self->t_suspend);
  if (oldval == 0) {
   /* The thread must actually be suspended. */
   atomic_rwlock_write(&task_cpu->c_lock);
#ifdef CONFIG_SMP
   if (task_cpu != self->t_cpu) {
    atomic_rwlock_endwrite(&task_cpu->c_lock);
    goto again;
   }
#endif
   take_action = true;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->t_suspend,oldval,oldval+1));
 if (take_action) {
  assert(!oldval);
  /* Don't continue execution if the thread was already suspended. */
  if (self->t_mode == TASKMODE_TERMINATED ||
     (self->t_flags&TASKFLAG_WILLTERM)) {
   result = -EINVAL;
   atomic_rwlock_endwrite(&task_cpu->c_lock);
  } else if (self->t_mode == TASKMODE_RUNNING
#ifdef TASKMODE_WAKEUP
          || self->t_mode == TASKMODE_WAKEUP
#endif
             ) {
   task_unsched_unlockcpu(self,TASKMODE_SUSPENDED);
  } else {
   /* Set the no-continue flag if the task already is suspended. */
   self->t_flags |= TASKFLAG_SUSP_NOCONT;
   atomic_rwlock_endwrite(&task_cpu->c_lock);
  }
 }
 return result;
}
PUBLIC errno_t KCALL
task_resume(struct task *__restrict self) {
 s32 oldval; bool take_action = false;
 errno_t result = -EOK;
 struct cpu *task_cpu;
#ifdef CONFIG_SMP
again:
#endif
 CHECK_HOST_DOBJ(self);
#ifdef CONFIG_SMP
 task_cpu = ATOMIC_READ(self->t_cpu);
#else
 task_cpu = &__bootcpu;
#endif
 do {
  if (take_action) {
   atomic_rwlock_endwrite(&task_cpu->c_lock);
   take_action = false;
  }
  oldval = ATOMIC_READ(self->t_suspend);
  if (oldval == 1) {
   /* The thread must actually be suspended. */
   atomic_rwlock_write(&task_cpu->c_lock);
#ifdef CONFIG_SMP
   if (task_cpu != self->t_cpu) {
    atomic_rwlock_endwrite(&task_cpu->c_lock);
    goto again;
   }
#endif
   take_action = true;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->t_suspend,oldval,oldval-1));
 if (take_action) {
  assert(oldval == 1);
  if (self->t_mode == TASKMODE_TERMINATED ||
     (self->t_flags&TASKFLAG_WILLTERM))
   result = -EINVAL;
  else if (self->t_flags&TASKFLAG_SUSP_NOCONT) {
   /* Don't continue execution if the thread was already suspended. */
   self->t_flags &= ~(TASKFLAG_SUSP_NOCONT);
  } else {
   bool was;
   /* Unlink the thread from the suspended chain. */
   LIST_REMOVE(self,t_sched.sd_suspended);
   was = PREEMPTION_ENABLED();
   PREEMPTION_DISABLE();
#ifdef CONFIG_SMP
   if (task_cpu != THIS_CPU) {
    /* Insert into the wakeup chain. */
    CPU_INSERT_WAKEUP(task_cpu,self);
    self->t_mode = TASKMODE_WAKEUP;
   } else
#endif
   {
    struct task *tself = task_cpu->c_running;
    assert(tself == THIS_TASK);
    /* Re-schedule the thread to run last. */
    RING_INSERT_BEFORE(tself,self,t_sched.sd_running);
    self->t_mode     = TASKMODE_RUNNING;
#ifdef CONFIG_SMP
    self->t_realstate = TASKMODE_RUNNING;
#endif
   }
   if (was) PREEMPTION_ENABLE();
  }
  atomic_rwlock_endwrite(&task_cpu->c_lock);
 }
 return result;
}


#ifdef CONFIG_DEBUG
PRIVATE ATTR_USED void KCALL
__task_destroy2(struct task *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assertf((uintptr_t)&self <  (uintptr_t)self->t_hstack.hs_begin ||
         (uintptr_t)&self >= (uintptr_t)self->t_hstack.hs_end,
         "Still on stack that'll be destroyed (%p in %p...%p)",
         &self,self->t_hstack.hs_begin,(uintptr_t)self->t_hstack.hs_end-1);
 syslogf(LOG_DEBUG,"DESTROYING TASK AFTER TERMINATE: %p\n",self);
 task_destroy(self);
}
#else
#define __task_destroy2 task_destroy
#endif

PRIVATE ATTR_NORETURN void
task_terminate_self_unlock_cpu(struct task *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(self == THIS_TASK);
#ifdef CONFIG_SMP
 assert(self->t_cpu == THIS_CPU);
#endif
 assert(THIS_CPU->c_running == self);
 PREEMPTION_DISABLE();

 /*syslogf(LOG_DEBUG,"Terminating SELF: %p\n",self);*/

 /* Must unlock the CPU _AFTER_ we disabled preemption,
  * because if we did so before, the PIC interrupt handler
  * might notice us being terminated and try to remove us
  * before we're done.
  * HINT: The CPU must lock 'c_lock' before checking
  *       if a task was marked as terminated in SMP-mode. */
 atomic_rwlock_endwrite(&THIS_CPU->c_lock);
 cpu_sched_rotate_selectnew(THIS_CPU);
 assert(self != THIS_CPU->c_running);

 /* WARNING: At this point, the cpu-local/THIS_TASK setup is extremely inconsistent.
  *         'THIS_TASK' does not match what is actually the current chain of execution. */

 RING_REMOVE(self,t_sched.sd_running);
 ATOMIC_WRITE(self->t_mode,TASKMODE_TERMINATED);
#ifdef CONFIG_SMP
 ATOMIC_WRITE(self->t_realstate,TASKMODE_TERMINATED);
#endif

 assert(!PREEMPTION_ENABLED());
 assert(self != THIS_CPU->c_running);

 /* Make sure to activate the new task's page directory as early as possible! */
 { struct mman *new_mman = THIS_CPU->c_running->t_mman;
   if (self->t_mman != new_mman) {
    /* Switch to the new task's page directory. */
    __asm__ __volatile__("movl %0, %%cr3\n"
                         :
                         : "r" (&new_mman->m_ppdir->pd_directory)
                         : "memory");
   }
 }

 /* Switch to the cpu's current (new) task's stack & register state,
  * where we can then broadcast termination & decref() the old task.
  * Afterwards, iret to the new task. */
 __asm__ __volatile__(
     L(    movl ASM_CPU2(CPU_OFFSETOF_RUNNING), %%eax   )
     /* Prevent the new task from being terminated while it's destroying the old.
      * NOTE: This precaution starts taking effect once 'sti' re-enables interrupts below. */
     L(    incl TASK_OFFSETOF_CRITICAL(%%eax)           )
     L(    movl TASK_OFFSETOF_CSTATE(%%eax),    %%esp   )
     L(    movl (TASK_OFFSETOF_HSTACK+HSTACK_OFFSETOF_END)(%%eax), %%eax) /* Load the base address of the kernel stack. */
     L(    movl %%eax, ASM_CPU2(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_TSS+TSS_OFFSETOF_ESP0)) /* Save the proper kernel stack address in the CPU's TSS. */
     L(    popl %%edi                                   )
     L(    popl %%esi                                   )
     L(    popl %%ebp                                   )
     L(    addl $8, %%esp  /* Skip ESP (_n0) and EBP */ )
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
     L(    pushl 20(%%esp) /* Push the proper EBX */    )
     /* At this point, we've essentially hacked our way into
      * the new task's execution stack and finally managed
      * to fix up the stack in such a way that could essentially
      * allow us to be pre-empted again.
      * Because of that, we are now safe to re-enable interrupts,
      * even before broadcasting the join-signal (which in itself
      * could potentially enable interrupts momentarily)
      * NOTE: All registers but 'EBX' are loaded with the proper
      *       return values, while the correct 'EBX' is saved in
      *       0(%ESP), and the current still contains a pointer
      *       to the old task, which we're supposed to finish
      *       terminating. */
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
     L(    leal  TASK_OFFSETOF_JOIN(%%ebx), %%eax       )
     L(    pushl $0xffffffff                            )
     L(    pushl %%eax                                  )
     L(    call  sig_send                               )
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
     : "b" (self)
     : "memory"
 );
 __builtin_unreachable();
}

PRIVATE errno_t KCALL
task_doraise_unlockcpu(struct task *__restrict self,
                       signal_t code) {
 errno_t result = -EOK;
 struct cpu *task_cpu;
 bool was;
 CHECK_HOST_DOBJ(self);
#ifdef CONFIG_SMP
 task_cpu = self->t_cpu;
#else
 task_cpu = &__bootcpu;
#endif
 CHECK_HOST_DOBJ(task_cpu);
 assert(atomic_rwlock_writing(&task_cpu->c_lock));
 if (self->t_flags&TASKFLAG_INTERRUPT && self->t_nointr == 0) {
  if (self == THIS_TASK) {
   signal_t intno = self->t_intno;
   bool call_service = !(self->t_flags&TASKFLAG_INTERRUPT_NOSERVE);
   self->t_flags &= ~(TASKFLAG_INTERRUPT|TASKFLAG_INTERRUPT_NOSERVE);
   atomic_rwlock_endwrite(&task_cpu->c_lock);
   if (call_service) task_service_interrupt(self,intno);
   goto service_self;
  }
  result = -EALREADY;
  goto end2;
 }
 if (self->t_mode == TASKMODE_TERMINATED) {
  result = -EINVAL;
  goto end2;
 }
 self->t_flags |= TASKFLAG_INTERRUPT;
 self->t_intno  = code;
#ifdef CONFIG_SMP
 was = PREEMPTION_ENABLED();
 PREEMPTION_DISABLE();
 if (task_cpu != THIS_CPU) {
  if (self->t_mode == TASKMODE_RUNNING ||
      self->t_mode == TASKMODE_WAKEUP);
  else if (!self->t_nointr) {
   assert(self->t_mode == TASKMODE_SLEEPING ||
          self->t_mode == TASKMODE_SUSPENDED);
   LIST_REMOVE(self,t_sched.sd_suspended);
   /* Insert into the wakeup chain. */
   CPU_INSERT_WAKEUP(task_cpu,self);
   self->t_mode = TASKMODE_WAKEUP;
  }
 } else
#endif
 {
  struct task *tself = task_cpu->c_running;
  if (self->t_nointr) goto end;
  if (tself == self) {
#ifdef CONFIG_SMP
   if (was) PREEMPTION_ENABLE();
#endif
service_self:
   atomic_rwlock_endwrite(&task_cpu->c_lock);
   task_service_interrupt(self,code);
   return -EINTR;
  } else if (self->t_mode != TASKMODE_RUNNING) {
#ifdef TASKMODE_WAKEUP
   assert(self->t_mode == TASKMODE_WAKEUP ||
          self->t_mode == TASKMODE_SLEEPING ||
          self->t_mode == TASKMODE_SUSPENDED);
#else
   assert(self->t_mode == TASKMODE_SLEEPING ||
          self->t_mode == TASKMODE_SUSPENDED);
#endif
   LIST_REMOVE(self,t_sched.sd_suspended);
#ifndef CONFIG_SMP
   was = PREEMPTION_ENABLED();
   PREEMPTION_DISABLE();
#endif
   assert(tself == THIS_TASK);
   /* Re-schedule the thread to run last. */
   RING_INSERT_BEFORE(tself,self,t_sched.sd_running);
   self->t_mode     = TASKMODE_RUNNING;
#ifdef CONFIG_SMP
   self->t_realstate = TASKMODE_RUNNING;
#else
   if (was) PREEMPTION_ENABLE();
#endif
  }
 }
end:
#ifdef CONFIG_SMP
 if (was) PREEMPTION_ENABLE();
#endif
end2:
 atomic_rwlock_endwrite(&task_cpu->c_lock);
 return result;
}

PUBLIC errno_t KCALL
task_raise(struct task *__restrict self, signal_t code) {
 TASK_CPULOCK_WRITE(self);
 return task_doraise_unlockcpu(self,code);
}



PUBLIC errno_t KCALL
task_join(struct task *__restrict self,
          struct timespec const *timeout,
          void **exitcode) {
 errno_t result = -EOK;
 CHECK_HOST_DOBJ(self);
 sig_read(&self->t_join);
 /* Special case: 'TASKMODE_TERMINATED' is written with a write-barrier
  *                before the join signal is broadcast, meaning that
  *                while locking the join-signal, we can read the
  *                task's real state with a write-barrier to confirm
  *                that the join-signal hasn't been send yet. */
 if (ATOMIC_READ(TASK_REALSTATE(self)) == TASKMODE_TERMINATED) {
  sig_endread(&self->t_join);
  goto end;
 }
 if (!sig_upgrade(&self->t_join)) {
  if (ATOMIC_READ(TASK_REALSTATE(self)) == TASKMODE_TERMINATED) {
   sig_endwrite(&self->t_join);
   goto end;
  }
 }
 result = sig_timedrecv_endwrite(&self->t_join,timeout);
 if (E_ISERR(result)) return result;
end:
 if (exitcode) {
  /* Store the exitcode when requested, to. */
  CHECK_HOST_DOBJ(exitcode);
  *exitcode = self->t_exitcode;
 }
 return result;
}

PUBLIC errno_t KCALL
task_terminate(struct task *__restrict self,
               void *exitcode) {
 struct cpu *task_cpu;
 task_cpu = TASK_CPULOCK_WRITE(self);
 if (ATOMIC_READ(self->t_critical)) {
  /* Interrupt the task. */
  if (self->t_flags&TASKFLAG_WILLTERM) {
einval:
   atomic_rwlock_endwrite(&task_cpu->c_lock);
   return -EINVAL;
  }
  self->t_flags |=  (TASKFLAG_WILLTERM|TASKFLAG_INTERRUPT_NOSERVE);
  self->t_flags &= ~(TASKFLAG_INTERRUPT);
  return task_doraise_unlockcpu(self,(signal_t)-1);
 }
 if (self->t_mode == TASKMODE_TERMINATED) goto einval;
 self->t_exitcode = exitcode;
#ifdef TASKMODE_WAKEUP
 if (self->t_mode == TASKMODE_WAKEUP) {
  LIST_REMOVE(self,t_sched.sd_wakeup);
 } else
#endif
 if (self->t_mode == TASKMODE_SLEEPING ||
     self->t_mode == TASKMODE_SUSPENDED) {
  LIST_REMOVE(self,t_sched.sd_suspended);
 } else {
  bool was;
  assertf(self->t_mode == TASKMODE_RUNNING,
          "self->t_mode = %d",(int)self->t_mode);
  was = PREEMPTION_ENABLED();
  PREEMPTION_DISABLE();
#ifdef CONFIG_SMP
  if (task_cpu != THIS_CPU) {
   /* Must wait until IRQ terminates the task for us. */
   atomic_rwlock_endwrite(&task_cpu->c_lock);
   if (was) PREEMPTION_ENABLE();
   TASK_DECREF(self);
   return -EOK;
  }
#endif
  /* Terminate a task running on the current CPU. */
#ifdef CONFIG_SMP
  self->t_realstate = TASKMODE_TERMINATED;
#endif
  if (task_cpu->c_running == self) {
   task_terminate_self_unlock_cpu(self);
   __builtin_unreachable();
  }
  /* Remove the task from the scheduler ring. */
  RING_REMOVE(self,t_sched.sd_running);
  if (was) PREEMPTION_ENABLE();
 }
 self->t_mode = TASKMODE_TERMINATED;
 atomic_rwlock_endwrite(&task_cpu->c_lock);
 /* Broadcast the join-signal. */
 sig_broadcast(&self->t_join);
 TASK_DECREF(self);
 return -EOK;
}

PUBLIC bool (KCALL task_issafe)(void) { return task_issafe(); }
PUBLIC bool (KCALL task_iscrit)(void) { return task_iscrit(); }
PUBLIC void (KCALL task_crit)(void) { task_crit(); }
PUBLIC ATTR_HOTTEXT SAFE void (KCALL task_endcrit)(void) {
 struct task *self = THIS_TASK; u32 level;
#ifdef CONFIG_SMP
 struct cpu *task_cpu;
#endif
 CHECK_HOST_DOBJ(self);
 level = ATOMIC_READ(self->t_critical);
 assertf(level != 0,"Missing 'task_crit()'");
 if likely(level != 1) {
#ifdef CONFIG_DEBUG
  if (OATOMIC_FETCHDEC(self->t_critical,__ATOMIC_RELEASE) != level)
      assertf(0,"Some illegal entity is modify task critical counters");
#else
  OATOMIC_FETCHDEC(self->t_critical,
                   __ATOMIC_RELEASE);
#endif
  return;
 }
#ifdef CONFIG_SMP
 task_cpu = TASK_CPULOCK_WRITE(self);
#else
 TASK_CPULOCK_WRITE(self);
#endif
 if (self->t_flags&TASKFLAG_WILLTERM) {
  /* End of a critical block. */
#ifdef CONFIG_DEBUG
  if (OATOMIC_XCH(self->t_critical,0,__ATOMIC_RELEASE) != level)
      assertf(0,"Some illegal entity is modify task critical counters");
#else
  OATOMIC_STORE(self->t_critical,0);
#endif
  /* Bye Bye! */
  task_terminate_self_unlock_cpu(self);
 }
 atomic_rwlock_endwrite(&task_cpu->c_lock);
#ifdef CONFIG_DEBUG
 if (OATOMIC_XCH(self->t_critical,0,__ATOMIC_RELEASE) != level)
     assertf(0,"Some illegal entity is modify task critical counters");
#else
 OATOMIC_STORE(self->t_critical,0);
#endif
}

#ifdef CONFIG_SMP
PUBLIC struct cpu *KCALL
task_setcpu(struct task *__restrict self,
            struct cpu *__restrict new_cpu) {
 bool was;
 struct cpu *old_cpu;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(new_cpu);
 was = PREEMPTION_ENABLED();
 PREEMPTION_DISABLE();
 old_cpu = TASK_CPULOCK_WRITE(self);
 CHECK_HOST_DOBJ(old_cpu);
 if (old_cpu != new_cpu) {
  if (self->t_realstate == TASKMODE_RUNNING) {
   /* Migrate a running task. */
   if (old_cpu == THIS_CPU) {
    struct task *caller = THIS_TASK;
    RING_REMOVE(self,t_sched.sd_running);
    atomic_rwlock_endwrite(&old_cpu->c_lock);
    /* Re-schedule as wakeup on the new CPU. */
    atomic_rwlock_write(&new_cpu->c_lock);
    self->t_realstate = TASKMODE_WAKEUP;
    CPU_INSERT_WAKEUP(new_cpu,self);
    atomic_rwlock_endwrite(&new_cpu->c_lock);
    /* If we've just re-scheduled ourselves, safe registers and
     * switch to the next task until the given CPU wakes us again. */
    if (self == caller) {
     cpu_sched_setrunning_save(self);
     assert(THIS_CPU == new_cpu);
    }
   } else {
    /* TODO: The task is running on another CPU. */
    //struct cpu *result_cpu;
    //result_cpu = self->t_newcpu;
    //self->t_newcpu = new_cpu;
    atomic_rwlock_endwrite(&old_cpu->c_lock);
    //old_cpu = result_cpu;
   }
  } else if (self->t_realstate == TASKMODE_TERMINATED) {
   assert(self->t_mode == TASKMODE_TERMINATED);
   /* Special case: The task was terminated. */
   ATOMIC_WRITE(self->t_cpu,new_cpu);
  } else {
   assert(self->t_realstate == TASKMODE_SLEEPING ||
          self->t_realstate == TASKMODE_SUSPENDED ||
          self->t_realstate == TASKMODE_WAKEUP);
   /* Remove the task from its old list. */
   assert(self != THIS_TASK);
   LIST_REMOVE(self,t_sched.sd_sleeping); /* Inherit reference. */
   ATOMIC_WRITE(self->t_cpu,new_cpu);
   atomic_rwlock_endwrite(&old_cpu->c_lock);
   atomic_rwlock_write(&new_cpu->c_lock);
   /* Schedule the task as wake-up on the new CPU. */
   switch (self->t_realstate) {
   case TASKMODE_SLEEPING:
    CPU_INSERT_SLEEPING(new_cpu,self);
    break;
   case TASKMODE_SUSPENDED:
    CPU_INSERT_SUSPENDED(new_cpu,self);
    break;
   case TASKMODE_WAKEUP:
    if (new_cpu == THIS_CPU) {
     RING_INSERT_BEFORE(THIS_TASK,self,t_sched.sd_running);
    } else {
     CPU_INSERT_WAKEUP(new_cpu,self);
    }
    break;
   default: __builtin_unreachable();
   }
   atomic_rwlock_endwrite(&new_cpu->c_lock);
  }
 }
 /* Fix pre-emption state (it doesn't hurt if we've already did this above...) */
 if (was) PREEMPTION_ENABLE();
 return old_cpu;
}
#endif


PUBLIC void KCALL
intchain_trigger(struct intchain **__restrict pchain, irq_t irq,
                 struct ccpustate const *__restrict cstate, u32 eflags) {
 struct intchain *iter;
 CHECK_HOST_DOBJ(pchain);
 iter = *pchain;
 for (; iter; iter = iter->ic_prev) {
  if ((iter->ic_irq == irq) ||
      (iter->ic_opt&INTCHAIN_OPT_EXC && IRQ_ISEXC(irq)) ||
      (iter->ic_opt&INTCHAIN_OPT_PIC && IRQ_ISPIC(irq)) ||
      (iter->ic_opt&INTCHAIN_OPT_USR && IRQ_ISUSR(irq))) {
   uintptr_t target;
#ifndef __i386__
#error FIXME
#endif
   /* Trigger this handler! */
   *pchain = iter->ic_prev;
   target = (uintptr_t)&iter->ic_int;

   target -= sizeof(u32);
   *(u32 *)target = eflags;
   /* Copy the execution CPU state before the interrupt handler address.
    * Afterwards, we can simply use 'ret' to jump to where we need to. */
   target -= sizeof(struct ccpustate);
   *(struct ccpustate *)target = *cstate;
   __asm__ __volatile__("movl %0, %%esp\n"
                        "popal\n"
                        "popw %%gs\n"
                        "popw %%fs\n"
                        "popw %%es\n"
                        "popw %%ds\n"
                        "popfl\n"
                        "ret\n"
                        :
                        : "g" (target)
                        : "memory");
   __builtin_unreachable();
  }
 }
}


DECL_END

#endif /* !GUARD_KERNEL_SCHED_TASK_C */
