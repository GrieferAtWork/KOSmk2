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
#ifndef GUARD_KERNEL_SCHED_TASK_UTIL_C
#define GUARD_KERNEL_SCHED_TASK_UTIL_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include "../debug-config.h"

#include <errno.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/traceback.h>
#include <arch/cpustate.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <kernel/mman.h>
#include <kernel/stack.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <sched.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/task.h>
#include <sched/types.h>
#include <string.h>
#include <sync/sig.h>
#include <sys/mman.h>
#include <arch/hints.h>
#ifndef CONFIG_NO_TLB
#include <kos/thread.h>
#endif /* !CONFIG_NO_TLB */

DECL_BEGIN

#ifdef CONFIG_SMP
PUBLIC struct cpu *KCALL
cpu_get_suitable(__cpu_set_t const *__restrict affinity) {
 CHECK_HOST_DOBJ(affinity);
 /* TODO */
 return &__bootcpu;
}
#endif /* CONFIG_SMP */


PUBLIC errno_t KCALL
task_get_affinity(struct task *__restrict self,
                  USER cpu_set_t *affinity) {
#ifndef CONFIG_SMP
 CHECK_HOST_DOBJ(self);
 (void)self;
 return memset_user(affinity,0xff,sizeof(cpu_set_t)) ? -EFAULT : -EOK;
#else
 errno_t error = -EOK;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->t_affinity_lock);
 if (copy_to_user(affinity,&self->t_affinity,sizeof(cpu_set_t)))
     error = -EFAULT;
 atomic_rwlock_endread(&self->t_affinity_lock);
 return error;
#endif
}

#ifdef CONFIG_SMP
PUBLIC errno_t KCALL
task_set_affinity(struct task *__restrict self,
                  USER cpu_set_t const *affinity) {
 errno_t error = -EOK;
 cpu_set_t old_affinity;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->t_affinity_lock);
 memcpy(&old_affinity,&self->t_affinity,sizeof(cpu_set_t));
 if (!atomic_rwlock_upgrade(&self->t_affinity_lock))
      memcpy(&old_affinity,&self->t_affinity,sizeof(cpu_set_t));
 if (copy_from_user(&self->t_affinity,affinity,sizeof(cpu_set_t))) {
  memcpy(&self->t_affinity,&old_affinity,sizeof(cpu_set_t));
  error = -EFAULT;
 } else {
  cpuid_t id = ATOMIC_READ(self->t_cpu)->c_id;
  if (!CPU_ISSET(id,&self->t_affinity)) {
   /* Must migrate the task to a different CPU. */
   struct cpu *c = cpu_get_suitable(&self->t_affinity);
   if unlikely(!c) {
    /* No such CPU. */
    error = -ENODEV;
   } else {
    c = task_setcpu(self,c);
    if (E_ISERR(c)) error = E_GTERR(c);
   }
   /* Restore the old CPU affinity if an error occurred. */
   if (E_ISERR(error)) {
    memcpy(&self->t_affinity,&old_affinity,sizeof(cpu_set_t));
   }
  }
 }
 atomic_rwlock_endwrite(&self->t_affinity_lock);
 return error;
}
#endif /* CONFIG_SMP */


#ifndef CONFIG_NO_TLB

LOCAL SAFE errno_t KCALL
task_mktlb_impl(struct task *__restrict self, bool caller_locked) {
 size_t tlb_size; errno_t error;
 struct mman *mm; bool has_write_lock = false;
 USER ppage_t tlb_address; struct mregion *tlb_region;
 CHECK_HOST_DOBJ(self);
 /* NOTE: This function is also called to allocate a new TLB during exec(),
  *       meaning that the following two assertions cannot be made! */
 //assert(self->t_mode == TASKMODE_NOTSTARTED);
 //assert(self->t_tlb == PAGE_ERROR);
 assert(self->t_mman != NULL);
 mm = self->t_mman;
 assert(!caller_locked || mman_writing(mm));

 tlb_size = CEIL_ALIGN(sizeof(struct tlb),PAGESIZE);
 if (!caller_locked) {
again_findspace:
  error = mman_read(mm);
  if (E_ISERR(error)) return error;
 }
 tlb_address = mman_findspace_unlocked(mm,(ppage_t)(USER_TASK_TLB_ADDRHINT-tlb_size),
                                       tlb_size,MAX(PAGESIZE,USER_STCK_ALIGN),USER_STCK_ADDRGAP,
                                       MMAN_FINDSPACE_BELOW|MMAN_FINDSPACE_TRYHARD|MMAN_FINDSPACE_PRIVATE);
 if (tlb_address != PAGE_ERROR) goto got_space;
 /* Failed to find sufficient space. */
 if (!caller_locked) mman_endread(mm);
 error = -ENOMEM;
err:
 return error;
got_space:
 if (!caller_locked && !has_write_lock) {
  error = mman_upgrade(mm);
  if (E_ISERR(error)) {
   if (error != -ERELOAD) goto err;
   has_write_lock = true;
   goto again_findspace;
  }
 }
 assert(tlb_address != PAGE_ERROR);

 /* Allocate the region controller for the TLB region. */
 tlb_region = mregion_new(MMAN_DATAGFP(mm));
 if unlikely(!tlb_region) {
  if (!caller_locked) mman_endwrite(mm);
  return -ENOMEM;
 }
 tlb_region->mr_size = tlb_size;
 tlb_region->mr_init = MREGION_INIT_ZERO; /* Use ZERO-initialization. */

 /* Finalize the region setup. */
 mregion_setup(tlb_region);

 /* Now try to map it in the associated address space.
  * NOTE: We pass a custom notifier to track the region even when it has moved. */
 error = mman_mmap_unlocked(mm,tlb_address,tlb_size,0,
                            tlb_region,PROT_READ|PROT_WRITE,
                            NULL,self);
 if (E_ISOK(error)) self->t_tlb = (PAGE_ALIGNED USER struct tlb *)tlb_address;
 /* NOTE: The TLB region will be initialized once the thread is started. */

 if (!caller_locked) mman_endwrite(mm);
 MREGION_DECREF(tlb_region);
 return error;
}

PUBLIC SAFE errno_t KCALL
task_mktlb(struct task *__restrict self) {
 return task_mktlb_impl(self,false);
}
PUBLIC SAFE errno_t KCALL
task_mktlb_unlocked(struct task *__restrict self) {
 return task_mktlb_impl(self,true);
}


PUBLIC ssize_t KCALL
task_tlb_mnotify(unsigned int type, void *__restrict closure,
                 struct mman *mm, ppage_t addr, size_t size) {
 struct task *self = (struct task *)closure;
 CHECK_HOST_DOBJ(self);
 assert(ATOMIC_READ(self->t_refcnt) >= 1);

 switch (type) {
  /* Reference counting is implemented through weak references. */
 case MNOTIFY_INCREF: TASK_WEAK_INCREF(self); break;
 case MNOTIFY_DECREF: TASK_WEAK_DECREF(self); break;

 case MNOTIFY_UNSHARE_DROP:
  /* Drop all stack mappings, but that of the
   * calling thread during un-sharing (aka. fork()). */
  if (self != THIS_TASK)
      return 1;
  break;

 default: break;
 }
 return -EOK;
}

PUBLIC void KCALL
task_ldtlb(struct task *__restrict self) {
 struct mman *omm;
 if (self->t_tlb == PAGE_ERROR) return;
 TASK_PDIR_BEGIN(omm,self->t_mman);
 call_user_worker(&task_filltlb,1,self);
 TASK_PDIR_END(omm,self->t_mman);
}

#ifdef __x86_64__
/* TODO: Assert the correct offset for x86_64 */
#else
/* `0x18' is a hard-coded number used by various DOS compilers. */
STATIC_ASSERT(offsetof(struct tib,ti_self) == 0x18);
#endif

PUBLIC void KCALL
task_filltlb(struct task *__restrict self) {
 USER struct tlb *info = self->t_tlb;
 /* Fill in TLB information. */
 assert(addr_isuser_r(info,CEIL_ALIGN(sizeof(struct tlb),PAGESIZE)));
 assert(THIS_TASK->t_mman == self->t_mman);
 info->tl_self        = info;
 info->tl_env         = self->t_mman->m_environ;
 info->tl_tib.ti_self = &info->tl_tib;
 info->tl_tib.ti_seh  = SEH_FRAME_NULL;
 info->tl_tib.ti_pid  = THREAD_PID_GETPID(&self->t_pid);
 info->tl_tib.ti_tid  = THREAD_PID_GETTID(&self->t_pid);
 if likely(self->t_ustack) {
  info->tl_tib.ti_stackhi = self->t_ustack->s_end;
  info->tl_tib.ti_stacklo = self->t_ustack->s_begin;
 }
}

#endif /* !CONFIG_NO_TLB */


#ifdef CONFIG_DEBUG
#define CONFIG_HOST_STACKINIT KERNEL_DEBUG_MEMPAT_HOSTSTACK
#endif

PUBLIC SAFE errno_t KCALL
task_mkhstack(struct task *__restrict self, size_t n_bytes) {
 REF struct mregion *stack_region; struct mman *old_mman;
 ppage_t stack_addr; bool has_write_lock; errno_t error = -EOK;
 CHECK_HOST_DOBJ(self);
 TASK_PDIR_KERNEL_BEGIN(old_mman);
 stack_region = mregion_new(MMAN_DATAGFP(&mman_kernel));
 if unlikely(!stack_region) goto nomem;
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 assert(n_bytes != 0);
#ifdef CONFIG_HOST_STACKINIT
 /* Use debug pre-initialization of stack memory. */
 stack_region->mr_init           = MREGION_INIT_BYTE;
 stack_region->mr_setup.mri_byte = CONFIG_HOST_STACKINIT;
#endif
 stack_region->mr_size = n_bytes;
 /* Force the region to be loaded in-core, as well as be locked there! */
 stack_region->mr_part0.mt_state  = MPART_STATE_INCORE;
 stack_region->mr_part0.mt_locked = 1;
 /* Allocate the initial physical memory for the stack region. */
 if (!page_malloc_scatter(&stack_region->mr_part0.mt_memory,
                           n_bytes,PAGESIZE,PAGEATTR_NONE,
                           MZONE_ANY,MMAN_DATAGFP(&mman_kernel))) {
  free(stack_region);
  goto nomem;
 }
#ifdef CONFIG_HOST_STACKINIT
 { struct mscatter *iter = &stack_region->mr_part0.mt_memory;
   while (iter) {
#if __SIZEOF_POINTER__ >= 8
    memsetq(iter->m_start,CONFIG_HOST_STACKINIT,iter->m_size/8);
#else
    memsetl(iter->m_start,CONFIG_HOST_STACKINIT,iter->m_size/4);
#endif
    iter = iter->m_next;
   }
 }
#endif

 /* perform final setup on the region. */
 mregion_setup(stack_region);
 /* At this point, we've created the stack region! */

 has_write_lock = false;
 error = mman_read(&mman_kernel);
 if (E_ISERR(error)) goto end_region;
reload_spc:
 /* Search for suitable stack. */
 stack_addr = mman_findspace_unlocked(&mman_kernel,
                                     (ppage_t)(HOST_STCK_ADDRHINT-n_bytes),n_bytes,
                                      MAX(HOST_STCK_ALIGN,PAGESIZE),0,MMAN_FINDSPACE_BELOW|
                                      MMAN_FINDSPACE_TRYHARD|MMAN_FINDSPACE_PRIVATE);
 if unlikely(stack_addr == PAGE_ERROR) goto nospc;
 if (!has_write_lock) {
  error = mman_upgrade(&mman_kernel);
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
 mman_endwrite(&mman_kernel);
end_region: MREGION_DECREF(stack_region);
end: TASK_PDIR_KERNEL_END(old_mman);
 return error;
nospc: error = -ENOMEM;
 if (has_write_lock) mman_endwrite(&mman_kernel);
 else                mman_endread (&mman_kernel);
 goto end_region;
nomem: error = -ENOMEM; goto end;
}

PUBLIC SAFE errno_t KCALL
task_mkustack(struct task *__restrict self,
              size_t n_bytes, size_t guard_size, u16 funds) {
 REF struct stack *ustack;
 size_t gap_size; errno_t error;
 struct mman *mm = self->t_mman;
 bool has_write_lock = false;
 n_bytes    = CEIL_ALIGN(n_bytes,PAGESIZE);
 guard_size = CEIL_ALIGN(guard_size,PAGESIZE);
 if (__builtin_mul_overflow(guard_size,funds,&gap_size))
     gap_size = guard_size;
 ustack  = omalloc(struct stack);
 if unlikely(!ustack) return -ENOMEM;
again_findspace:
 error = mman_read(mm);
 ustack->s_begin = mman_findspace_unlocked(mm,(ppage_t)(USER_STCK_ADDRHINT-n_bytes),
                                           n_bytes+guard_size,MAX(PAGESIZE,USER_STCK_ALIGN),gap_size,
                                           MMAN_FINDSPACE_BELOW|MMAN_FINDSPACE_TRYHARD|MMAN_FINDSPACE_PRIVATE);
 if (ustack->s_begin != PAGE_ERROR) goto got_space;
 /* Failed to find sufficient space. */
 mman_endread(mm);
 error = -ENOMEM;
err_ustack:
 free(ustack);
 return error;
got_space:
 if (!has_write_lock) {
  error = mman_upgrade(mm);
  if (E_ISERR(error)) {
   if (error != -ERELOAD) goto err_ustack;
   has_write_lock = true;
   goto again_findspace;
  }
 }
 assert(ustack->s_begin != PAGE_ERROR);
 ustack->s_end = (VIRT ppage_t)((uintptr_t)ustack->s_begin+n_bytes);
 ustack->s_refcnt      = 1;
 ustack->s_branch      = 0;
 ustack->s_task.ap_ptr = self;
 error = mman_mmap_stack_unlocked(mm,ustack,PROT_READ|PROT_WRITE,
                                  MREGION_TYPE_LOGUARD,funds,guard_size,
                                  NULL);
 if (E_ISOK(error)) self->t_ustack = ustack; /* Inherit reference. */
 mman_endwrite(mm);
 return error;
}


PUBLIC size_t KCALL
sig_send(struct sig *__restrict sig,
         size_t max_threads) {
 size_t result = 0;
 CHECK_HOST_DOBJ(sig);
 sig_write(sig);
 while (max_threads-- &&
        sig_vsendone_unlocked(sig,NULL,0))
        ++result;
 sig_endwrite(sig);
 return result;
}
PUBLIC size_t KCALL
sig_send_unlocked(struct sig *__restrict sig,
                  size_t max_threads) {
 size_t result = 0;
 CHECK_HOST_DOBJ(sig);
 assert(sig_writing(sig));
 while (max_threads-- &&
        sig_vsendone_unlocked(sig,NULL,0))
        ++result;
 return result;
}
PUBLIC size_t KCALL
sig_vsend(struct sig *__restrict sig,
          void *msg, size_t msgsize,
          size_t max_threads) {
 size_t result = 0;
 CHECK_HOST_DOBJ(sig);
 sig_write(sig);
 while (max_threads-- &&
        sig_vsendone_unlocked(sig,msg,msgsize))
        ++result;
 sig_endwrite(sig);
 return result;
}
PUBLIC size_t KCALL
sig_vsend_unlocked(struct sig *__restrict sig,
                   void *msg, size_t msgsize,
                   size_t max_threads) {
 size_t result = 0;
 CHECK_HOST_DOBJ(sig);
 assert(sig_writing(sig));
 while (max_threads-- &&
        sig_vsendone_unlocked(sig,msg,msgsize))
        ++result;
 return result;
}


PUBLIC errno_t KCALL
sig_timedrecv(struct sig *__restrict self, jtime_t abstime) {
 CHECK_HOST_DOBJ(self);
 sig_write(self);
 return sig_vtimedrecv_endwrite(self,NULL,0,abstime);
}
PUBLIC errno_t KCALL
sig_timedrecv_endwrite(struct sig *__restrict self, jtime_t abstime) {
 return sig_vtimedrecv_endwrite(self,NULL,0,abstime);
}
PUBLIC errno_t KCALL
sig_vtimedrecv(struct sig *__restrict self,
               void *msg_buf, size_t bufsize, jtime_t abstime) {
 CHECK_HOST_DOBJ(self);
 sig_write(self);
 return sig_vtimedrecv_endwrite(self,msg_buf,bufsize,abstime);
}
PUBLIC errno_t KCALL
sig_vtimedrecv_endwrite(struct sig *__restrict self,
                        void *msg_buf, size_t bufsize,
                        jtime_t abstime) {
 struct sig *error;
 CHECK_HOST_DOBJ(self);
 assert(sig_writing(self));
 task_addwait(self,msg_buf,bufsize);
 sig_endwrite(self);
 error = task_waitfor(abstime);
 return E_ISOK(error) ? -EOK : E_GTERR(error);
}


DECL_END

#endif /* !GUARD_KERNEL_SCHED_TASK_UTIL_C */
