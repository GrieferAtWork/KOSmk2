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
#ifndef GUARD_KERNEL_CORE_ARCH_FPU_C
#define GUARD_KERNEL_CORE_ARCH_FPU_C 1
#define _KOS_SOURCE 2

#include <assert.h>
#include <hybrid/atomic.h>
#include <kernel/arch/task.h>
#include <kernel/export.h>
#include <sched/percpu.h>
#include <sched/task.h>
#include <sched/cpu.h>
#include <string.h>
#include <syslog.h>
#include <kernel/irq.h>
#include <hybrid/section.h>
#include <hybrid/align.h>
#include <sched/paging.h>
#include <kernel/mman.h>
#include <kernel/arch/cpustate.h>

#ifndef CONFIG_NO_FPU
DECL_BEGIN

STATIC_ASSERT(sizeof(struct fpustate) == FPUSTATE_SIZE);
PUBLIC CPU_BSS struct task *fpu_current;

PRIVATE ATTR_USED void FCALL fpu_irq_nm(struct cpustate *__restrict info);
__asm__(".global fpu_asm_nm\n"
        ".hidden fpu_asm_nm\n");
INTERN DEFINE_EXC_HANDLER(fpu_asm_nm,fpu_irq_nm);
PRIVATE ATTR_FREERODATA isr_t const fpu_switch = ISR_DEFAULT(IRQ_EXC_NM,&fpu_asm_nm);


PRIVATE void FCALL
fpu_irq_nm(struct cpustate *__restrict info) {
 struct task *old_task = CPU(fpu_current);
 struct task *new_task = THIS_TASK;
 if (new_task != old_task) {
  syslog(LOG_DEBUG,"[FPU] Switch context %p -> %p\n",old_task,new_task);
  FPUSTATE_ENABLE();
  if (old_task) {
   /* Save the old FPU register state. */
   if unlikely(old_task->t_arch.at_fpu == FPUSTATE_NULL &&
              (old_task->t_arch.at_fpu = FPUSTATE_ALLOC()) == FPUSTATE_NULL) {
    syslog(LOG_ERR,"[FPU] Failed to allocate FPU state (gpid %d): %[errno]\n",
           old_task->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,ENOMEM);
   } else {
    FPUSTATE_SAVE(*old_task->t_arch.at_fpu);
   }
  }
  if (new_task->t_arch.at_fpu == FPUSTATE_NULL) {
   /* Initialize the default FPU register state. */
   FPUSTATE_INIT();
  } else {
   /* Load an existing FPU state. */
   assert(IS_ALIGNED((uintptr_t)new_task->t_arch.at_fpu,FPUSTATE_ALIGN));
   FPUSTATE_LOAD(*new_task->t_arch.at_fpu);
  }
  CPU(fpu_current) = new_task;
  /* Continue execution normally. */
  return;
 } else {
  /* If the task-switched flag is set, unset it because it
   * is possible that the calling task is the only actually
   * using the FPU right now. */
  register register_t temp;
  __asm__ __volatile__("mov %%cr0, %0\n" : "=r" (temp));
  if (temp&CR0_TS) {
   __asm__ __volatile__("mov %0, %%cr0\n" : : "r" (temp&~CR0_TS));
   return;
  }
 }

 /* Fallback: This is something else... */
 { struct cpustate_e state;
   CPUSTATE_TO_CPUSTATE_E(*info,state,0);
   irq_default(IRQ_EXC_NM,&state);
 }
}

PRIVATE MODULE_INIT void KCALL fpu_init(void) {
 register register_t temp;
 /* Enable the FPU. */
 __asm__ __volatile__("clts\n"
                      "mov %%cr0, %0\n"
                      "and $(" __PP_STR(~CR0_EM) "), %0\n"
                      "or  $(" __PP_STR(CR0_MP) "), %0\n"
                      "mov %0, %%cr0\n"
                      "mov %%cr4, %0\n"
                      "or  $(" __PP_STR(CR4_OSFXSR) "), %0\n"
                      "mov %0, %%cr4\n"
                      : "=r" (temp));
 /* Install the IRQ handler used when switching FPU tasks. */
 asserte(irq_set(&fpu_switch,NULL,IRQ_SET_RELOAD));
}

DECL_END
#endif /* !CONFIG_NO_FPU */

#endif /* !GUARD_KERNEL_CORE_ARCH_FPU_C */
