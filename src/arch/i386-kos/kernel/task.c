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
#ifndef GUARD_KERNEL_ARCH_TASK_C
#define GUARD_KERNEL_ARCH_TASK_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <bits/signum.h>
#include <dev/rtc.h>
#include <fs/fd.h>
#include <hybrid/align.h>
#include <asm/cpu-flags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <hybrid/traceback.h>
#include <arch/cpustate.h>
#include <kernel/interrupt.h>
#include <hybrid/host.h>
#include <kernel/mman.h>
#include <kernel/paging-util.h>
#include <kernel/paging.h>
#include <kernel/stack.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <linker/module.h>
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
#include <asm/instx.h>
#include <kernel/syscall.h>
#include <arch/pic.h>
#ifndef CONFIG_NO_TLB
#include <arch/gdt.h>
#include <kos/thread.h>
#include <arch/asm.h>
#endif /* !CONFIG_NO_TLB */

DECL_BEGIN

/* Assert CPU-state/register pack sizes. */
STATIC_ASSERT(sizeof(struct gpregs) == GPREGS_SIZE);
STATIC_ASSERT(sizeof(struct gpregs32) == GPREGS32_SIZE);
STATIC_ASSERT(sizeof(struct sgregs) == SGREGS_SIZE);
STATIC_ASSERT(sizeof(struct sgregs32) == SGREGS32_SIZE);
STATIC_ASSERT(sizeof(struct irregs_host) == IRREGS_HOST_SIZE);
STATIC_ASSERT(sizeof(struct irregs) == IRREGS_SIZE);
STATIC_ASSERT(sizeof(struct irregs_host_e) == IRREGS_HOST_E_SIZE);
STATIC_ASSERT(sizeof(struct irregs_e) == IRREGS_E_SIZE);
STATIC_ASSERT(sizeof(struct cpustate16) == CPUSTATE16_SIZE);
STATIC_ASSERT(sizeof(struct comregs) == COMREGS_SIZE);
STATIC_ASSERT(sizeof(struct comregs32) == COMREGS32_SIZE);
STATIC_ASSERT(sizeof(struct cpustate_host) == CPUSTATE_HOST_SIZE);
STATIC_ASSERT(sizeof(struct cpustate) == CPUSTATE_SIZE);
STATIC_ASSERT(sizeof(struct cpustate_host_e) == CPUSTATE_HOST_E_SIZE);
STATIC_ASSERT(sizeof(struct cpustate_e) == CPUSTATE_E_SIZE);

#ifndef CONFIG_NO_LDT
STATIC_ASSERT(offsetof(struct ldt,l_gdt) == LDT_OFFSETOF_GDT);
STATIC_ASSERT(offsetof(struct ldt,l_idt) == LDT_OFFSETOF_IDT);
STATIC_ASSERT(offsetof(struct ldt,l_refcnt) == LDT_OFFSETOF_REFCNT);
STATIC_ASSERT(offsetof(struct ldt,l_lock) == LDT_OFFSETOF_LOCK);
#ifdef CONFIG_SMP
STATIC_ASSERT(offsetof(struct ldt,l_valid) == LDT_OFFSETOF_VALID);
#endif
STATIC_ASSERT(offsetof(struct ldt,l_tasks) == LDT_OFFSETOF_TASKS);
STATIC_ASSERT(sizeof(struct ldt) == LDT_SIZE);
#endif

/* Assert tss offsets. */
#if defined(__x86_64__) || defined(__i386__)
#ifndef __x86_64__
STATIC_ASSERT(offsetof(struct tss,eflags) == TSS_OFFSETOF_EFLAGS);
#endif
STATIC_ASSERT(offsetof(struct tss,iomap_base) == TSS_OFFSETOF_IOMAP);
STATIC_ASSERT(sizeof(struct tss) == TSS_SIZE);
#endif

#ifdef ARCHTASK_SIZE
#ifndef CONFIG_NO_LDT
STATIC_ASSERT(offsetof(struct archtask,at_ldt_tasks) == ARCHTASK_OFFSETOF_LDT_TASKS);
STATIC_ASSERT(offsetof(struct archtask,at_ldt_gdt) == ARCHTASK_OFFSETOF_LDT_GDT);
#endif /* !CONFIG_NO_LDT */
#ifndef CONFIG_NO_FPU
STATIC_ASSERT(offsetof(struct archtask,at_fpu) == ARCHTASK_OFFSETOF_FPU);
#endif /* !CONFIG_NO_FPU */
#endif /* ARCHTASK_SIZE */

GLOBAL_ASM(
L(.section .text                                                              )
L(PUBLIC_ENTRY(cpu_sched_setrunning_savef)                                    )
/* Prevent interrupts from breaking our stack structures below. (Must already be disabled!) */
//L(    cli                                                                   )
#ifdef __x86_64__
L(    popq -(5*8)(%rsp)    /* RIP (return address). */                        )
L(    pushq $(__KERNEL_DS) /* %ss */                                          )
L(    pushq %rsp           /* %userrsp */                                     )
L(    addq  $8, (%rsp)     /* Adjust to point to the ~real~ userrsp */        )
L(    pushq %FASTCALL_REG2 /* iret-compatible return block. */                )
#else
L(    popl  -(3*4)(%esp)   /* XIP (return address). */                        )
L(    pushl XSZ(%esp)      /* xflags. */                                      )
#endif
L(    jmp   1f                                                                )
L(PUBLIC_ENTRY(cpu_sched_setrunning_save)                                     )
L(    /* Generate an on-stack CPU state. */                                   )
/* Prevent interrupts from breaking our stack structures below. (Must already be disabled!) */
//L(    cli                                                                   )
#ifdef __x86_64__
L(    popq -(5*8)(%rsp)    /* RIP (return address). */                        )
L(    pushq $(__KERNEL_DS) /* %ss */                                          )
L(    pushq %rsp           /* %userrsp */                                     )
L(    addq  $8, (%rsp)     /* Adjust to point to the ~real~ userrsp */        )
L(    pushfq               /* iret-compatible return block. */                )
L(1:  pushq $(__KERNEL_CS)                                                    )
#else
L(    popl -(3*4)(%esp)    /* EIP (return address). */                        )
L(    pushfl               /* iret-compatible return block. */                )
L(1:  pushl %cs                                                               )
#endif
L(    subx  $(XSZ), %xsp   /* Skip EIP (already saved above) */               )
L(    __ASM_PUSH_COMREGS   /* Push registers. */                              )
#ifndef __x86_64__
      /* Load the given `task' argument into 'EAX'
       * NOTE: Use use `CPUSTATE_HOST_SIZE' as offset because that's
       *       the data structure we've just created on-stack, and
       *       now we want to access the first XWORD that follows.
       */
L(    movx CPUSTATE_HOST_SIZE(%xsp), %FASTCALL_REG1                           )
#endif
L(    /* Save the CPU state that we've just created within the given task. */ )
L(    movx  %xsp, TASK_OFFSETOF_CSTATE(%FASTCALL_REG1)                        )
L(PUBLIC_ENTRY(cpu_sched_setrunning)                                          )
L(    /* Load the running task into EAX */                                    )
L(    movx ASM_CPU(CPU_OFFSETOF_RUNNING), %FASTCALL_REG1                      )
L(    /* Load the new CPU state into ESP */                                   )
L(    movx TASK_OFFSETOF_CSTATE(%FASTCALL_REG1), %xsp                         )
L(    /* Load the base address of the kernel stack. */                        )
L(    movx (TASK_OFFSETOF_HSTACK+HSTACK_OFFSETOF_END)(%FASTCALL_REG1), %xax   )
L(    /* Save the proper kernel stack address in the CPU's TSS. */            )
L(    movx %xax, ASM_CPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_TSS+TSS_OFFSETOF_XSP0))
#ifdef __x86_64__
L(    /* Must swap GS base addresses if we're about to jump to user-space. */ )
L(    testb $3, CPUSTATE_HOST_OFFSETOF_IRET+IRREGS_OFFSETOF_CS(%rsp)          )
L(    jz    1f                                                                )
L(    swapgs                                                                  )
L(1:                                                                          )
#endif
L(    __ASM_POP_COMREGS /* Pop registers. */                                  )
L(    ASM_IRET          /* Iret -> pop XIP, CS + XFLAGS */                    )
L(SYM_END(cpu_sched_setrunning)                                               )
L(SYM_END(cpu_sched_setrunning_save)                                          )
L(SYM_END(cpu_sched_setrunning_savef)                                         )
L(.previous                                                                   )
);


/* PUBLIC errno_t KCALL task_yield(void); */
GLOBAL_ASM(
L(.section .text                                                              )
L(PUBLIC_ENTRY(task_yield)                                                    )
L(    popx %xax  /* XIP (return address). (Currently, only XAX may be clobbered) */)
#ifdef __x86_64__ /* x86_64 always requires the full IRET tail. */
L(    pushq $(__KERNEL_DS) /* %ss */                                          )
L(    pushq %rsp           /* %userrsp */                                     )
L(    addq $8, (%rsp)      /* Adjust to point to the ~real~ userrsp */        )
#endif
L(    pushfx     /* iret-compatible return block. */                          )
#ifdef __x86_64__
L(    pushq $(__KERNEL_CS)                                                    )
#else
L(    pushl %cs                                                               )
#endif
L(    pushx %xax /* XIP */                                                    )
L(    __ASM_PUSH_COMREGS /* Push registers. */                                )
L(    /* All registers and segments are now saved. */                         )
L(                                                                            )
L(    /* Make sure that the caller has interrupts enabled. */                 )
L(    testx $(EFLAGS_IF), CPUSTATE_HOST_OFFSETOF_IRET+IRREGS_OFFSETOF_FLAGS(%xsp))
L(    jz    99f                                                               )
L(                                                                            )
L(    cli /* Disable interrupts while switching context */                    )
L(                                                                            )
L(    /* Load the calling task into XBX and the next one into XAX. */         )
L(    movx ASM_CPU(CPU_OFFSETOF_RUNNING), %xbx                                )
L(    call cpu_sched_rotate_yield                                             )
L(    cmpx %xax, %xbx /* Check if we're actually yielding */                  )
L(    je   98f                                                                )
L(                                                                            )
L(    /* Set the caller's return register to `-EOK'. */                       )
L(    movx $-EOK,    CPUSTATE_HOST_OFFSETOF_GP+GPREGS_OFFSETOF_XAX(%xsp)      )
L(                                                                            )
/* OLD: XBX; NEW: XAX */
L(                                                                            )
L(    /* Save the CPU state that we've just created within the calling task. */)
L(    movx  %xsp, TASK_OFFSETOF_CSTATE(%xbx)                                  )
L(                                                                            )
L(    /* Switch the LDT if it changed */                                      )
#ifndef CONFIG_NO_LDT
L(    movw (TASK_OFFSETOF_ARCH+ARCHTASK_OFFSETOF_LDT_GDT)(%xax), %cx          )
L(    cmpw  %cx, (TASK_OFFSETOF_ARCH+ARCHTASK_OFFSETOF_LDT_GDT)(%xbx)         )
L(    je    1f /* No change required */                                       )
L(    lldt  %cx                                                               )
L(1:                                                                          )
#endif /* !CONFIG_NO_LDT */
L(                                                                            )
#ifndef CONFIG_NO_FPU
L(    /* Disable the FPU for lazy context switching */                        )
L(    movx  %cr0, %xcx                                                        )
L(    orx   $(CR0_TS), %xcx                                                   )
L(    movx  %xcx, %cr0                                                        )
#endif /* !CONFIG_NO_FPU */
L(                                                                            )
#ifndef CONFIG_NO_TLB
L(    /* Switch TIB/TLB pointers */                                           )
L(    movx  TASK_OFFSETOF_TLB(%xax), %xsi                                     )
#if 1 /* OPTIMIZATION? Don't switch TIB pointers if no change is required. */
L(    cmpx  %xsi, TASK_OFFSETOF_TLB(%xbx)                                     )
L(    je    1f /* Same address. - No change required. */                      )
#endif
L(    movx  ASM_CPU(cpu_gdt+IDT_POINTER_OFFSETOF_GDT), %xdi                   )
#define TLB(off) ((off)+SEG_USER_TLB*SEGMENT_SIZE)(%xdi)
#define TIB(off) ((off)+SEG_USER_TIB*SEGMENT_SIZE)(%xdi)
L(                                                                            )
L(    /* Update the TLB pointer */                                            )
L(    movx  %xsi,        %xdx                                                 )
L(    shrx  $24,         %xdx                                                 )
L(    movb  %dl,         TLB(SEGMENT_OFFSETOF_BASEHI)                         )
#ifdef __x86_64__
L(    shrq  $8,          %rdx                                                 )
L(    movl  %edx,        TLB(SEGMENT_OFFSETOF_BASEUP)                         )
#endif
L(    andl  $0xff000000, TLB(SEGMENT_OFFSETOF_BASELO)                         )
L(    movx  %xsi,        %xdx                                                 )
L(    andx  $0x00ffffff, %xdx                                                 )
L(    orl   %edx,        TLB(SEGMENT_OFFSETOF_BASELO)                         )
L(                                                                            )
L(    /* Update the TIB pointer */                                            )
L(    addx  $(TLB_OFFSETOF_TIB), %xsi                                         )
L(    movx  %xsi,        %xdx                                                 )
L(    shrx  $24,         %xdx                                                 )
L(    movb  %dl,         TIB(SEGMENT_OFFSETOF_BASEHI)                         )
#ifdef __x86_64__
L(    shrq  $8,          %rdx                                                 )
L(    movl  %edx,        TIB(SEGMENT_OFFSETOF_BASEUP)                         )
#endif
L(    andl  $0xff000000, TIB(SEGMENT_OFFSETOF_BASELO)                         )
L(    andx  $0x00ffffff, %xsi                                                 )
L(    orl   %esi,        TIB(SEGMENT_OFFSETOF_BASELO)                         )
#undef TIB
#undef TLB
L(1:                                                                          )
#endif /* !CONFIG_NO_TLB */
L(                                                                            )
L(    /* Switch memory managers. */                                           )
L(    movx TASK_OFFSETOF_MMAN(%xax),  %xcx                                    )
L(    movx TASK_OFFSETOF_MMAN(%xbx),  %xdx                                    )
L(    cmpx %xdx, %xcx /* Check if memory managers (and thereby page directories) changed. */)
L(    je   71f /* No switch required. */                                      )
L(    movx MMAN_OFFSETOF_PPDIR(%xcx), %xcx                                    )
#if defined(PDIR_OFFSETOF_DIRECTORY) && PDIR_OFFSETOF_DIRECTORY != 0
L(    addx $(PDIR_OFFSETOF_DIRECTORY),%xcx                                    )
#endif
L(    movx %xcx, %cr3 /* Switch to the new task's page directory. */          )
L(                                                                            )
L(    /* Load general purpose registers */                                    )
L(71: movx TASK_OFFSETOF_CSTATE(%xax), %xsp                                   )
L(    /* Load the base address of the kernel stack. */                        )
L(    movx (TASK_OFFSETOF_HSTACK+HSTACK_OFFSETOF_END)(%xax), %xax             )
L(    /* Save the proper kernel stack address in the CPU's TSS. */            )
L(    movx %xax, ASM_CPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_TSS+TSS_OFFSETOF_XSP0))
L(70:                                                                         )
#if defined(CONFIG_DEBUG) && 0
#ifdef __x86_64__
L(    movq  $1, %rdi                                                          )
#else
L(    pushl $1                                                                )
#endif
L(    call cpu_validate_counters                                              )
#endif
#ifdef __x86_64__
L(    /* Must swap GS base addresses if we're about to jump to user-space. */ )
L(    testb $3, CPUSTATE_HOST_OFFSETOF_IRET+IRREGS_OFFSETOF_CS(%rsp)          )
L(    jz    1f                                                                )
L(    swapgs                                                                  )
L(1:                                                                          )
#endif
L(    __ASM_POP_COMREGS /* Pop registers. */                                  )
L(    ASM_IRET          /* Iret -> pop XIP, CS + XFLAGS */                    )
L(98: sti   /* Re-enable interrupts. */                                       )
L(    pause /* Dispite all, still allow the CPU to relax a bit when not yielding. */)
L(    movx $-EAGAIN, CPUSTATE_HOST_OFFSETOF_GP+GPREGS_OFFSETOF_XAX(%xsp)      )
#ifdef __x86_64__
L(    cli                                                                     )
#endif
L(    jmp  70b                                                                )
L(99: pause                                                                   )
#ifdef CONFIG_DEBUG
#ifdef __x86_64__
L(    movq  %rax, %rdi                                                        )
L(    call  noyield_without_irq                                               )
#else
L(    pushl %eax                                                              )
L(    call  noyield_without_irq                                               )
L(    addl  $4, %esp                                                          )
#endif
#endif
L(    movx $-EPERM, CPUSTATE_HOST_OFFSETOF_GP+GPREGS_OFFSETOF_XAX(%xsp)       )
#ifdef __x86_64__
L(    cli                                                                     )
#endif
L(    jmp  70b                                                                )
L(SYM_END(task_yield)                                                         )
L(.previous                                                                   )
);

#ifdef CONFIG_DEBUG
INTERN bool interrupts_enabled_initial = false;
INTERN void ATTR_CDECL noyield_without_irq(void *xip) {
 if (!interrupts_enabled_initial) return;
#ifdef CONFIG_SMP
 if (SMP_ONLINE > 1) {
  PRIVATE CPU_DATA void *noyield_last_eip = (void *)-1;
#ifdef CONFIG_NO_IDLE
  if (xip == CPU(noyield_last_eip)) return;
#endif /* CONFIG_NO_IDLE */
#ifdef CONFIG_USE_EXTERNAL_ADDR2LINE
  syslog(LOG_SCHED|LOG_WARN,
         "#!$ addr2line(%Ix) '{file}({line}) : {func} : %p : Cannot yield while interrupts are disabled'\n",
        (uintptr_t)xip-1,xip);
#else
  syslog(LOG_SCHED|LOG_WARN,"%[vinfo] : %p : Cannot yield while interrupts are disabled'\n",
        (uintptr_t)xip-1,xip);
#endif
  debug_tbprint(0);
  CPU(noyield_last_eip) = xip;
 } else
#endif
 {
  __NAMESPACE_INT_SYM
  __afail("Cannot yield while interrupts are disabled",DEBUGINFO_GEN);
 }
}
#endif /* CONFIG_DEBUG */



INTERN ATTR_NOINLINE struct cpustate *FCALL
pit_interrupt_handler(struct cpustate *__restrict state) {
 struct task *old_task;
 struct task *new_task;
 assert(!PREEMPTION_ENABLED());

#if 0
 debug_printf("#!$ addr2line(%Ix) '{file}({line}) : {func} : PIT Trigger: %p'\n",
             (uintptr_t)state->iret.eip-1,state->iret.eip);
 debug_tbprint2((void *)state->gp.ebp,0);
#endif

 /* Update the sub-second clock. */
 sysrtc_periodic();

 if (cpu_trywrite(THIS_CPU)) {
  struct task *wake;
  /* Check for wakeups in sleeping tasks. */
  if ((wake = THIS_CPU->c_sleeping) != NULL) {
#ifndef CONFIG_JIFFY_TIMEOUT
   struct timespec now; sysrtc_get(&now);
#endif /* !CONFIG_JIFFY_TIMEOUT */
   for (;;) {
#ifdef CONFIG_JIFFY_TIMEOUT
    /* NOTE: Since we (the PIC interrupt timer) are the only ones who may update jiffies,
     *       we also know that we're only ever updating it through incrementing by one.
     *       This in return means that no tick is ever skipped as far as the jiffi counter
     *       is concerned, which also means that we can safely perform a comparison for
     *       equality, rather than lower, meaning this check is actually safe when it
     *       comes to counter overroll!
     * XXX: Don't do this! Remember how sleeping tasks are sorted:
     *  jiffies == 0xffffffffffffffe0
     *  thread #1: sleep(jiffies+10); (Sleep until `0xffffffffffffffea')
     *  thread #2: sleep(jiffies+60); (Sleep until `0x000000000000001c') // Overflow
     * Sort order:
     *  thread #2 == CPU->c_sleeping
     *  thread #1 == CPU->c_sleeping->t_sched.sd_sleeping.le_next
     * 
     * Checking for '!=' will actually cause execution of thread #1
     * to be paused until long after the end of the know universe!
     * 
     * TODO: A better solution for this would be to re-schedule all tasks with a
     *       timeout `>= 0x8000000000000000' when the jiffi counter rolls over,
     *       but still perform a check using '!=', as this still works for threads
     *       with long timeouts that extend past the end of jiffi's 64-bit timing.
     *      (Notice how thread #2 is still woken at the correct point in time, while
     *       this proposed solution would simply wait a bit too long before waking
     *       thread #1, while still waking it before thread #2, as was the originally
     *       intention by the programmer)
     */
#if 0
    if (jiffies != wake->t_timeout) break;
#else
    if (jiffies < wake->t_timeout) break;
#endif
#else /* CONFIG_JIFFY_TIMEOUT */
    if (TIMESPEC_LOWER(now,wake->t_timeout)) break;
#endif /* !CONFIG_JIFFY_TIMEOUT */
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
  *      (Or the task executed an `int $0' manually?) */
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
         old_task,old_task->t_cstate->iret.eip,
         new_task,new_task->t_cstate->iret.eip,
      !!(new_task->t_cstate->iret.eflags&EFLAGS_IF));
 }
#endif

 TASK_SWITCH_CONTEXT(old_task,new_task);
 /* Signal completion of the PIT interrupt.
  * NOTE: Actual new signals will only be received once iret turns interrupts back on.
  *       And in the event that the new task didn't have interrupts
  *       enabled, they won't turn back on for a while. */
 PIC_EOI(INTNO_PIC1_PIT);
 assert(!PREEMPTION_ENABLED());

 /* Update the CPU's TSS with the new task's kernel stack.
  * NOTE: We're only safe to do so, because interrupts are still disabled! */
 THIS_CPU->c_arch.ac_tss.xsp0 = (uintptr_t)new_task->t_hstack.hs_end;
 old_task->t_cstate = state;
 COMPILER_BARRIER(); /* Make sure that everything above has been done. */

#if 0
 debug_printf("#!$ addr2line(%Ix) '{file}({line}) : {func} : Returning to: %p'\n",
             (uintptr_t)new_task->t_cstate->iret.eip,new_task->t_cstate->iret.eip);
 debug_tbprint2((void *)new_task->t_cstate->gp.ebp,0);
 debug_printf("EAX %p  ECX %p  EDX %p  EBX %p\n",
              new_task->t_cstate->gp.eax,
              new_task->t_cstate->gp.ecx,
              new_task->t_cstate->gp.edx,
              new_task->t_cstate->gp.ebx);
 debug_printf("ESP %p  EBP %p  ESI %p  EDI %p\n",
              new_task->t_cstate,
              new_task->t_cstate->gp.ebp,
              new_task->t_cstate->gp.esi,
              new_task->t_cstate->gp.edi);
#endif

 assertf(new_task->t_cstate,"Task %p has no cpu state",new_task);
 assertf(state->iret.eflags&EFLAGS_IF,"How did you manage to preempt without interrupts enabled?");
 assertf((new_task->t_cstate->iret.cs&3) != 3 ||
         (new_task->t_cstate->iret.eflags&EFLAGS_IF),
          "Must not switch to user-level task with #IF flag disabled");

 /* Return the new CPU state that should be loaded next.
  * NOTE: Interrupts are re-enabled when iret is triggered. */
 return new_task->t_cstate;
}


INTDEF struct cpustate *FCALL
pit_interrupt_handler(struct cpustate *__restrict state);
INTDEF void ASMCALL pit_interrupt_wrapper(void);
GLOBAL_ASM(
L(.section .text.hot                                                          )
L(PRIVATE_ENTRY(pit_interrupt_wrapper)                                        )
L(    __ASM_PUSH_COMREGS                                                      )
L(    movx   %xsp, %FASTCALL_REG1 /* Pass a pointer to the generated cpustate. */)
#ifdef __x86_64__
L(    testb  $3, CPUSTATE_OFFSETOF_IRET+IRREGS_OFFSETOF_CS(%rsp)              )
L(    jz     1f                                                               )
L(    swapgs /* Load the kernel's GS base address */                          )
L(    call   pit_interrupt_handler                                            )
L(    swapgs /* Restore the user-space GS */                                  )
L(    movq   %rax, %rsp /* Use the handler's return value as new stack. */    )
L(    __ASM_POP_COMREGS                                                       )
L(    ASM_IRET                                                                )
L(1:  call   pit_interrupt_handler                                            )
#else
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    call   pit_interrupt_handler                                            )
#endif
L(    movx   %xax, %xsp /* Use the handler's return value as new stack. */    )
L(    __ASM_POP_COMREGS                                                       )
L(    ASM_IRET                                                                )
L(SYM_END(pit_interrupt_wrapper)                                              )
L(.previous                                                                   )
);

/* Registered in `cpu.c' */
INTERN struct interrupt pit_interrupt = {
    .i_intno = INTNO_PIC1_PIT,
    .i_mode  = INTMODE_HW,
    .i_type  = INTTYPE_ASM,
    .i_prio  = INTPRIO_MAX,
    .i_flags = INTFLAG_PRIMARY,
    .i_proto = {
        .p_asm = &pit_interrupt_wrapper,
    },
    .i_owner = THIS_INSTANCE,
};


DECL_END

#endif /* !GUARD_KERNEL_ARCH_TASK_C */
