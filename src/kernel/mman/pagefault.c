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
#ifndef GUARD_KERNEL_MMAN_PAGEFAULT_C
#define GUARD_KERNEL_MMAN_PAGEFAULT_C 1
#define _KOS_SOURCE 2

#include "intern.h"
#include <hybrid/align.h>
#include <asm/cpu-flags.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/traceback.h>
#include <arch/cpustate.h>
#include <kernel/export.h>
#include <kernel/interrupt.h>
#include <sys/syslog.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/percpu.h>
#include <sched/task.h>
#include <sched/types.h>
#include <asm/instx.h>

DECL_BEGIN

#define PF_P (1 << 0) /*< 0x01: Present. */
#define PF_W (1 << 1) /*< 0x02: Write. */
#define PF_U (1 << 2) /*< 0x04: User. */
#define PF_R (1 << 3) /*< 0x08: Reserved write. */
#define PF_I (1 << 4) /*< 0x10: Instruction Fetch. */

INTERN int FCALL
mman_interrupt_pf_handler(struct irregs_ie *__restrict info) {
 u32 core_mode; VIRT uintptr_t fault_addr,fault_page;
 VIRT struct mman *user_mman; ssize_t error;
 struct tasksig old_sigset;

 /* Load the address that caused the fault in the first place. */
 __asm__ __volatile__("mov %%cr2, %0\n" : "=r" (fault_addr));

 /* NOTE: Re-enable preemption to allow for better parallelization
  *       of PAGEFAULT handling, but make sure to keep the original
  *       `fault_addr' around to tell later error handling about it.
  * NOTE: In order to always respect the interrupt flag, don't
  *       enable them when the caller had been turned off! */
 assert(!PREEMPTION_ENABLED());
 if (info->xflags&EFLAGS_IF) {
  PREEMPTION_ENABLE();
 } else {
#if 1 /* TO-DO: Re-enable me */
  assertf((info->cs&3) != 3,"User-level task with interrupts disabled");
  assertf(info->xip >= KERNEL_BASE &&
         (info->xip <  (uintptr_t)__kernel_user_start ||
          info->xip >= (uintptr_t)__kernel_user_end),
         "User-space address %p with interrupts disabled & ring-0 permissions");
#endif
 }
#if defined(CONFIG_DEBUG) && 0
 syslog(LOG_DEBUG,"#PF at %p (IF=%d)\n",
        info->xip,!!(info->xflags&EFLAGS_IF));
#endif

#if PF_W == MMAN_MCORE_WRITE && \
    PF_U == MMAN_MCORE_USER
 core_mode = info->exc_code&(PF_W|PF_U);
#else
 core_mode = MMAN_MCORE_READ;
 if (info->exc_code&PF_W) core_mode |= MMAN_MCORE_WRITE;
 if (info->exc_code&PF_U) core_mode |= MMAN_MCORE_USER;
#endif

#if 0
 syslog(LOG_MEM|LOG_DEBUG,
        "[MEM] Checking to load core memory after PAGEFAULT near %p %p %p\n",
        fault_addr,&fault_addr,info->xip);
#endif

#if 0
 syslog(LOG_DEBUG,"#!$ addr2line(%Ix) '{file}({line}) : {func} : %p #PF at %p (%x)'\n",
       (uintptr_t)info->xip-1,info->xip,fault_addr,info->exc_code);
#endif

 fault_page = FLOOR_ALIGN(fault_addr,PAGESIZE);
 user_mman = THIS_TASK->t_mman;
 assert(addr_isglob(user_mman));
#ifdef CONFIG_DEBUG
 { PHYS pdir_t *cr3;
   __asm__ __volatile__("mov %%cr3, %0\n" : "=r" (cr3));
   assertf(user_mman->m_ppdir == cr3,
           "Incorrect page directory set (%p != %p) (fault_addr = %p at %p)",
           user_mman->m_ppdir,cr3,fault_addr,info->xip);
 }
#endif

 /* Let's not deal with interrupts for this part... */
 task_crit();
 task_nointr();
 {
  struct mman *eff_mman = user_mman;
  struct mman *old_mman = NULL;
  /* Must always search for core data in the kernel memory
   * manager for addresses greater than the kernel base. */
  if ((uintptr_t)fault_page >= KERNEL_BASE) {
   TASK_PDIR_KERNEL_BEGIN(old_mman);
   eff_mman = &mman_kernel;
  }
#if 0
  error = mman_trywrite(eff_mman);
  if (E_ISERR(error)) goto end_mcore;
#else
  task_pushwait(&old_sigset);
#ifndef NDEBUG
  { struct task *old_owner = ATOMIC_READ(eff_mman->m_lock.orw_lock.aorw_owner);
    if (old_owner == THIS_TASK) {
     assertf(eff_mman->m_lock.orw_lock.aorw_lock&ATOMIC_OWNER_RWLOCK_WFLAG,
             "Code that is not locked in-core must always use write-locks "
             "when accessing their effective memory manager. - A read-lock "
             "cannot be used because page-faults must temporarily acquire a "
             "write-lock that could potentially fail when a read-lock is shared.\n"
             "eff_mman->m_lock.orw_lock.aorw_lock = %.8I32x\n",
             eff_mman->m_lock.orw_lock.aorw_lock);
     asserte(E_ISOK(mman_trywrite(eff_mman)));
    } else {
     mman_write(eff_mman);
    }
  }
#else
  mman_write(eff_mman);
#endif
#endif

  /* With paging set up correctly, load memory into the core. */
  error = mman_mcore_unlocked(eff_mman,(ppage_t)fault_page,PAGESIZE,core_mode);
#if 0
  { extern void *mall_check_ptr;
    if (mall_check_ptr) COMPILER_UNUSED(malloc_usable_size(mall_check_ptr));
  }
#endif
  if (old_mman) TASK_PDIR_KERNEL_END(old_mman);

#ifdef CONFIG_DEBUG
  { PHYS pdir_t *cr3;
    __asm__ __volatile__("mov %%cr3, %0\n" : "=r" (cr3));
    assertf(user_mman->m_ppdir == cr3,
            "Incorrect page directory set (%p != %p)",
            user_mman->m_ppdir,cr3);
  }
#endif
  /* NOTE: Since the fault occurred, some other thread may have already
   *       loaded the specific page into the core, meaning we have to
   *       check if it's been loaded since to prevent the fault from
   *       propagating when the data had actually already been loaded. */
  if (!error) {
   pdir_attr_t req_attr = PDIR_ATTR_PRESENT;
#if PF_W == PDIR_ATTR_WRITE && PF_U == PDIR_ATTR_USER
   req_attr |= info->exc_code&(PF_W|PF_U);
#else
   if (info->exc_code&PF_W) req_attr |= PDIR_ATTR_WRITE;
   if (info->exc_code&PF_U) req_attr |= PDIR_ATTR_USER;
#endif
   /* Sadly, this lookup must be performed in the context of the kernel page directory! */
   /* XXX: Not really. - We could use page-directory self-mappings here... */
   {
    pflag_t was = PREEMPTION_PUSH();
    if (user_mman != &mman_kernel)
        __asm__ __volatile__("mov %0, %%cr3\n" : : "r" (&pdir_kernel.pd_directory)/* : "memory"*/);
    if (!pdir_maccess_addr(&user_mman->m_pdir,(void *)fault_page,req_attr)) {
#if 0
     syslog(LOG_DEBUG,"Faulty address: %p\n",fault_page);
#endif
     error = -EFAULT;
    }
    if (user_mman != &mman_kernel)
        __asm__ __volatile__("mov %0, %%cr3\n" : : "r" (&user_mman->m_ppdir->pd_directory)/* : "memory"*/);
    PREEMPTION_POP(was);
   }
  }
  /*mman_print_unlocked(mspace,&tty_printer,NULL);*/
  mman_endwrite(eff_mman);
  task_popwait(&old_sigset);
 }
end_mcore: ATTR_UNUSED;
 task_endnointr();
 task_endcrit();
 
 if (E_ISOK(error))
     pdir_flush((ppage_t)fault_page,PAGESIZE);
 else {
  if (error != -EFAULT)
      syslog(LOG_MEM|LOG_ERROR,"[MEM] Failed to load core at %p: %[errno]\n",
             fault_addr,-error);
  /* Save the latest fault address in the current task, thus preserving it
   * throughout preemption, as well as allowing later handling code to refer to it. */
  THIS_TASK->t_lastcr2 = (VIRT void *)fault_addr;
  return INTCODE_SEARCH;
 }

 return INTCODE_HANDLED;
}


DECL_END

#endif /* !GUARD_KERNEL_MMAN_PAGEFAULT_C */
