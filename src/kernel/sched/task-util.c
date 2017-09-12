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
#include <kernel/arch/cpustate.h>
#include <kernel/irq.h>
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

DECL_BEGIN

PUBLIC struct cpu *KCALL
cpu_get_suitable(__cpu_set_t const *__restrict affinity) {
 CHECK_HOST_DOBJ(affinity);
 /* TODO */
 return &__bootcpu;
}


PUBLIC errno_t KCALL
task_get_affinity(struct task *__restrict self,
                  USER cpu_set_t *affinity) {
 errno_t error = -EOK;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->t_affinity_lock);
 if (copy_to_user(affinity,&self->t_affinity,sizeof(cpu_set_t)))
     error = -EFAULT;
 atomic_rwlock_endread(&self->t_affinity_lock);
 return error;
}
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
#ifndef CONFIG_SMP
   error = -ENODEV;
#else
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
   if (E_ISERR(error))
#endif
   {
    memcpy(&self->t_affinity,&old_affinity,sizeof(cpu_set_t));
   }
  }
 }
 atomic_rwlock_endwrite(&self->t_affinity_lock);
 return error;
}



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
 error = mman_read(&mman_kernel);
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
 ustack->s_begin = mman_findspace_unlocked(mm,(ppage_t)(TASK_USERSTACK_ADDRHINT-(n_bytes+gap_size)),
                                           n_bytes+guard_size,MIN(PAGESIZE,TASK_USERSTACK_ALIGN),
                                           gap_size,MMAN_FINDSPACE_BELOW);
 if (ustack->s_begin != PAGE_ERROR &&
    (uintptr_t)ustack->s_begin+n_bytes < KERNEL_BASE) goto got_space;
 ustack->s_begin = mman_findspace_unlocked(mm,(ppage_t)(TASK_USERSTACK_ADDRHINT-n_bytes),
                                           n_bytes+guard_size,MIN(PAGESIZE,TASK_USERSTACK_ALIGN),0,
                                           MMAN_FINDSPACE_BELOW);
 if (ustack->s_begin != PAGE_ERROR &&
    (uintptr_t)ustack->s_begin+n_bytes < KERNEL_BASE) goto got_space;
 ustack->s_begin = mman_findspace_unlocked(mm,(ppage_t)0,n_bytes+guard_size,
                                           MIN(PAGESIZE,TASK_USERSTACK_ALIGN),
                                           0,MMAN_FINDSPACE_ABOVE);
 if (ustack->s_begin != PAGE_ERROR &&
    (uintptr_t)ustack->s_begin+n_bytes < KERNEL_BASE) goto got_space;
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
   struct {
    struct ccpustate state;
    u32              eflag;
   union{
    byte_t          *esp_minus_4;
    void           **p_eip;
   };
   } data;
#ifndef __i386__
#error FIXME
#endif
   /* Trigger this handler! */
   memcpy(&data.state,cstate,sizeof(struct ccpustate));
   data.eflag = eflags;
   data.p_eip = (void **)&iter->ic_int;

#if 0
   {
    static int inside = 0;
    __asm__ __volatile__("pushf; sti; hlt; popf\n");
    if (ATOMIC_XCH(inside,1) == 0) {
     syslog(LOG_DEBUG,"[INT] Execuring local interrupt handler: %p, %p, %p, %p (CR2 = %p)\n",
             iter,iter->ic_prev,*(void **)&iter->ic_irq,iter->ic_int,
             THIS_TASK->t_lastcr2);
     syslog(LOG_DEBUG,"EAX %p ECX %p EDX %p EBX %p\n",
             data.state.eax,data.state.ecx,data.state.edx,data.state.ebx);
     syslog(LOG_DEBUG,"ESP %p EBP %p ESI %p EDI %p\n",
             data.esp_minus_4+4,data.state.ebp,data.state.esi,data.state.edi);
     syslog(LOG_DEBUG,"DS %.4I16X ES %.4I16X FS %.4I16X GS %.4I16X\n",
             data.state.ds,data.state.es,data.state.fs,data.state.gs);
     __assertion_tbprint(0);
     struct mman *omm;
     TASK_PDIR_KERNEL_BEGIN(omm);
     mman_read(omm);
     mman_print_unlocked(omm,&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_CONFIRM));
     pdir_print(&omm->m_pdir,&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_CONFIRM));
     mman_endread(omm);
     TASK_PDIR_KERNEL_END(omm);
     ATOMIC_WRITE(inside,0);
    }
    __asm__ __volatile__("pushf; sti; hlt; popf\n");
    __asm__ __volatile__("pushf; sti; hlt; popf\n");
    __asm__ __volatile__("pushf; sti; hlt; popf\n");
   }
#endif

   *pchain = iter->ic_prev;
   __asm__ __volatile__("movl %0, %%esp\n"
                        "popal\n" /* state */
                        "popw %%gs\n"
                        "popw %%fs\n"
                        "popw %%es\n"
                        "popw %%ds\n"
                        "popfl\n" /* eflag */
                        "popl %%esp\n"
                        "ret\n"
                        :
                        : "g" (&data)
                        : "memory");
   __builtin_unreachable();
  }
 }
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
sig_timedrecv(struct sig *__restrict self,
              struct timespec const *abstime) {
 CHECK_HOST_DOBJ(self);
 sig_write(self);
 return sig_vtimedrecv_endwrite(self,NULL,0,abstime);
}
PUBLIC errno_t KCALL
sig_timedrecv_endwrite(struct sig *__restrict self,
                       struct timespec const *abstime) {
 return sig_vtimedrecv_endwrite(self,NULL,0,abstime);
}
PUBLIC errno_t KCALL
sig_vtimedrecv(struct sig *__restrict self,
               USER void *msg_buf, size_t bufsize,
               struct timespec const *abstime) {
 CHECK_HOST_DOBJ(self);
 sig_write(self);
 return sig_vtimedrecv_endwrite(self,msg_buf,bufsize,abstime);
}
PUBLIC errno_t KCALL
sig_vtimedrecv_endwrite(struct sig *__restrict self,
                        USER void *msg_buf, size_t bufsize,
                        struct timespec const *abstime) {
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
