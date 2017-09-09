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
#ifndef GUARD_KERNEL_SCHED_CPU_C
#define GUARD_KERNEL_SCHED_CPU_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1 /* CPU_* */

#include <assert.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/traceback.h>
#include <kernel/arch/cpustate.h>
#include <kernel/arch/cpu.h>
#include <kernel/arch/mp.h>
#include <kernel/arch/gdt.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <kernel/syslog.h>
#include <sched/cpu.h>
#include <sched/task.h>
#include <sys/io.h>
#include <kernel/boot.h>
#include <string.h>
#include <stdlib.h>

DECL_BEGIN

#if (BOOTSTACK_SIZE%PAGESIZE) != 0
#error "'BOOTSTACK_SIZE' must be page-aligned"
#endif

__BOOTTASK_IMPL
ATTR_FREEDATA struct task __boottask = {
    .t_refcnt    = 1,
#ifdef CONFIG_SMP
    .t_cpu       = &__bootcpu,
#endif
    .t_flags     = TASKFLAG_NONE,
    .t_state     = TASKSTATE_RUNNING,
#ifdef CONFIG_SMP
    .t_realstate = TASKSTATE_RUNNING,
#endif
    .t_sched = {
        .sd_running = {
            .re_prev = &__bootcpu.c_idle,
            .re_next = &__bootcpu.c_idle,
        },
    },
    .t_priority  = TASKPRIO_DEFAULT,
    .t_prioscore = TASKPRIO_DEFAULT,
    .t_signals   = {
        .ts_lock  = ATOMIC_RWLOCK_INIT,
        .ts_slotc = 0,
        .ts_slota = 0,
        .ts_slotv = NULL,
        .ts_first = {
            .tss_self = &__boottask,
        },
    },
    .t_join      = SIG_INIT,
    .t_critical  = 1, /* Start out as a critical task. */
    .t_nointr    = 1, /* Start out as a no-interrupt task. */
    .t_addrlimit = KERNEL_BASE,
    .t_suspend   = 0,
    .t_ustack    = NULL,
    .t_hstack = {
        .hs_begin = (ppage_t)(BOOTSTACK_ADDR),
        .hs_end   = (ppage_t)(BOOTSTACK_ADDR+BOOTSTACK_SIZE),
    },
    .t_affinity = CPU_SETALL,
#ifdef CONFIG_SMP
    .t_mman_tasks = {
        .le_next  = &__bootcpu.c_idle,
        .le_pself = &mman_kernel.m_tasks,
    },
#endif
    .t_mman = &mman_kernel,
};


INTDEF void ASMCALL cpu_idle(void);

GLOBAL_ASM(
/* Nothing really to see here:
 * >> hlt forever, automatically serving
 *    interrupts as they arrive. */
L(.section .text              )
L(cpu_idle:                   )
/* TODO: Before interrupts are enabled, a different idle-function
 *       should be used that switches back to the caller immediatly. */
L(    hlt                     )
#ifdef CONFIG_DEBUG
/* Add a bunch of hlt instructions to we can
 * watch them cycle when everything's working! */
L(    hlt                     )
L(    hlt                     )
L(    hlt                     )
#endif
L(    jmp cpu_idle            )
L(.size cpu_idle, . - cpu_idle)
L(.previous                   )
);


/* Allocate a small stack for the IDLE task of the boot CPU.
 * NOTE: This stack isn't actually require for the task itself,
 *       but for interrupts such as PIT timer, or others. */
PRIVATE ATTR_FREEDATA ATTR_ALIGNED(16) struct PACKED {
 byte_t               s_data[TASK_HOSTSTACK_IDLESIZE-sizeof(struct host_cpustate)];
 struct host_cpustate s_boot;
} __bootidlestack = {
    .s_boot = {
#ifdef __i386__
        .gs     = SEG(SEG_KERNEL_DATA),
        .fs     = SEG(SEG_CPUSELF),
#else
        .gs     = SEG(SEG_CPUSELF),
        .fs     = SEG(SEG_KERNEL_DATA),
#endif
        .es     = SEG(SEG_KERNEL_DATA),
        .ds     = SEG(SEG_KERNEL_DATA),
        .eip    = (uintptr_t)&cpu_idle,
        .cs     = SEG(SEG_KERNEL_CODE),
        /* All other flags don't matter, but 'IF' (interrupt flag) must be set.
         * If it wasn't, the idle task would otherwise block forever! */
        .eflags = EFLAGS_IF|EFLAGS_IOPL(0),
    },
};

PRIVATE ATTR_FREETEXT taskprio_t KCALL
prio_from_string(char *__restrict s) {
 taskprio_t p;
 if (!strcasecmp(s,SETUPSTR("idle")) ||
     !strcasecmp(s,SETUPSTR("maxidle"))) p = TASKPRIO_MAXIDLE;
 else if (!strcasecmp(s,SETUPSTR("minidle"))) p = TASKPRIO_MINIDLE;
 else if (!strcasecmp(s,SETUPSTR("min"))) p = TASKPRIO_MIN;
 else if (!strcasecmp(s,SETUPSTR("def"))) p = TASKPRIO_MAX;
 else if (!strcasecmp(s,SETUPSTR("max"))) p = TASKPRIO_MAX;
 else p = strtol(s,NULL,0);
 return p;
}

DEFINE_EARLY_SETUP("boot-priority=",boot_priority) {
 __boottask.t_priority = 
 __boottask.t_prioscore = prio_from_string(arg);
 return true;
}
DEFINE_EARLY_SETUP("idle-priority=",idle_priority) {
 __bootcpu.c_idle.t_priority = 
 __bootcpu.c_idle.t_prioscore = prio_from_string(arg);
 return true;
}

ATTR_SECTION(".cpu.boot")
PUBLIC struct cpu __bootcpu = {
    .c_self       = &__bootcpu,
    .c_running    = &__boottask,
    .c_id         = CPUID_BOOTCPU,
    .c_lock = ATOMIC_RWLOCK_INIT,
    .c_idle       = {
        .t_refcnt    = 2, /* NOTE: The CPU itself always keeps a symbolic reference here! */
        /* The location of the bootstrap cpu-state block. */
        .t_cstate    = (struct cpustate *)&__bootidlestack.s_boot,
#ifdef CONFIG_SMP
        .t_cpu       = &__bootcpu,
#endif
        .t_flags     = TASKFLAG_NONE,
        .t_state     = TASKSTATE_RUNNING,
#ifdef CONFIG_SMP
        .t_realstate = TASKSTATE_RUNNING,
#endif
        .t_sched = {
            .sd_running = {
                .re_prev = &__boottask,
                .re_next = &__boottask,
            },
        },
        .t_priority  = TASKPRIO_MAXIDLE,
        .t_prioscore = TASKPRIO_MAXIDLE,
        .t_signals   = {
            .ts_lock  = ATOMIC_RWLOCK_INIT,
            .ts_slotc = 0,
            .ts_slota = 0,
            .ts_slotv = NULL,
            .ts_first = {
                .tss_self = &__bootcpu.c_idle,
            },
        },
        .t_join      = SIG_INIT,
#ifdef CONFIG_DEBUG
        .t_critical  = 1,
#else
        .t_critical  = 0x80000001,
#endif
        .t_nointr    = 0,
        .t_addrlimit = KERNEL_BASE,
        .t_suspend   = 0,
        .t_ustack    = NULL,
        .t_hstack = {
            .hs_begin = (ppage_t)((uintptr_t)&__bootidlestack),
            .hs_end   = (ppage_t)((uintptr_t)&__bootidlestack+BOOTSTACK_SIZE),
        },
        .t_affinity = CPU_SETONE(CPUID_BOOTCPU),
        /* NOTE: Technically, IDLE tasks can run under any page directory,
         *       but adding an exception just for them may already negate
         *       the overhead that such additional checks would otherwise
         *       carry. - Especially considering that IDLE tasks aren't
         *       meant to be run continuously at all! */
#ifdef CONFIG_SMP
        .t_mman_tasks = {
            .le_next  = NULL,
            .le_pself = &__boottask.t_mman_tasks.le_next,
        },
#endif
        .t_mman       = &mman_kernel,
    },
    .c_arch = {
#if defined(__i386__) || defined(__x86_64__)
        .ac_tss = {
            /* Define initial boot-cpu TSS data. */
            .esp0       = (uintptr_t)(BOOTSTACK_ADDR+BOOTSTACK_SIZE),
            .ss0        = SEG(SEG_KERNEL_DATA),
            .iomap_base = sizeof(struct tss),
        },
        .ac_mode          = CPUMODE_ONLINE,
        .ac_flags         = CPUFLAG_NONE,
        .ac_lapic_id      = 0, /* Likely updated later */
        .ac_lapic_version = APICVER_82489DX, /* Likely updated later */
#endif
    },
};

PUBLIC void KCALL
cpu_inssleeping_unlocked(struct cpu *__restrict ins_cpu,
                         struct task *__restrict ins_task) {
 CHECK_HOST_DOBJ(ins_cpu);
 CHECK_HOST_DOBJ(ins_task);
 assert(atomic_rwlock_writing(&ins_cpu->c_lock));
 /* TODO: Must insert the cpu, keeping the sleeper list sorted! */
 LIST_INSERT(ins_cpu->c_sleeping,ins_task,t_sched.sd_sleeping);
}

PUBLIC void FCALL
cpu_sched_rotate_selectnew(struct cpu *__restrict self) {
 assert(self == THIS_CPU);
 assert(self->c_running != NULL);
 assert(self->c_running->t_sched.sd_running.re_prev != NULL);
 assert(self->c_running->t_sched.sd_running.re_next != NULL);
 assert(self->c_running->t_sched.sd_running.re_prev != self->c_running);
 assert(self->c_running->t_sched.sd_running.re_next != self->c_running);

 /* TODO: Consider low priority task! */
 RING_NEXT(self->c_running,t_sched.sd_running);
}
PUBLIC void FCALL
cpu_sched_rotate(struct cpu *__restrict self) {
 assertf(self == THIS_CPU,
         "self == %p\n"
         "THIS_CPU == %p",
         self,THIS_CPU);
 assert(self->c_running != NULL);
 assert(self->c_running->t_sched.sd_running.re_prev != NULL);
 assert(self->c_running->t_sched.sd_running.re_next != NULL);

 /* TODO: Consider low/high priority task! */
 RING_NEXT(self->c_running,t_sched.sd_running);
}
PUBLIC void KCALL cpu_sched_rotate_yield(void) {
 struct cpu *self = THIS_CPU;
 assert(self->c_running != NULL);
 assert(self->c_running->t_sched.sd_running.re_prev != NULL);
 assert(self->c_running->t_sched.sd_running.re_next != NULL);

 /* TODO: Consider low/high priority task, but don't switch back to the same task. */
 RING_NEXT(self->c_running,t_sched.sd_running);
}

PUBLIC ATTR_NORETURN void KCALL cpu_sched_setrunning(void);
PUBLIC void ATTR_CDECL cpu_sched_setrunning_save(struct task *__restrict task);


#undef CPU
#define CPU  ASM_CPU

STATIC_ASSERT(offsetof(struct cpustate,useresp) == 52);

GLOBAL_ASM(
L(.section .text                             )
L(.global cpu_sched_setrunning_save          )
L(.global cpu_sched_setrunning               )
L(cpu_sched_setrunning_save:                 ) /* Generate an on-stack CPU state. */
L(    cli                                    ) /* Prevent interrupts from breaking our stack structures below. */
L(    popl -12(%esp)                         ) /* EIP (return address). */
L(    pushfl                                 ) /* iret-compatible return block. */
L(    pushw $0                               )
L(    pushw %cs                              )
L(    subl $4, %esp                          ) /* Skip EIP (saved above) */
L(    pushw %ds                              ) /* Push segment registers. */
L(    pushw %es                              )
L(    pushw %fs                              )
L(    pushw %gs                              )
L(    pushal                                 ) /* Push general purpose registers. */
L(    movl  52(%esp), %eax                   ) /* Load the given 'task' argument into 'EAX' */
L(    movl  %esp, TASK_OFFSETOF_CSTATE(%eax) ) /* Save the CPU state that we've just created within the given task. */
//L(    int $3                                 )
L(cpu_sched_setrunning:                      )
L(    movl CPU(CPU_OFFSETOF_RUNNING),  %eax  ) /* Load the running task into EAX */
L(    movl TASK_OFFSETOF_CSTATE(%eax), %esp  ) /* Load the new CPU state into ESP */
L(    movl (TASK_OFFSETOF_HSTACK+HSTACK_OFFSETOF_END)(%eax), %eax) /* Load the base address of the kernel stack. */
L(    movl %eax, CPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_TSS+TSS_OFFSETOF_ESP0)) /* Save the proper kernel stack address in the CPU's TSS. */
L(    popal                                  ) /* Pop general purpose registers. */
L(    popw %gs                               ) /* Pop segment registers. */
L(    popw %fs                               )
L(    popw %es                               )
L(    popw %ds                               )
L(    iret                                   ) /* Iret -> pop EIP, CS + EFLAGS */
L(.size cpu_sched_setrunning_save, . - cpu_sched_setrunning_save)
L(.size cpu_sched_setrunning, . - cpu_sched_setrunning)
L(.previous                                  )
);


PUBLIC void KCALL task_yield(void);
GLOBAL_ASM(
L(.section .text                             )
L(.global task_yield                         )
L(task_yield:                                )
L(    pushf                                  ) /* Push eflags (will be restored later) */
L(    cli                                    ) /* Disable pre-emption. */
#if 0
L(    call __assertion_tbprint               )
#endif
L(    pushl CPU(CPU_OFFSETOF_RUNNING)        ) /* Push the old task (Argument for 'cpu_sched_setrunning_save'). */
L(    call cpu_sched_rotate_yield            ) /* Rotate running tasks. */
L(    pushl %eax                             )
L(    pushl %ecx                             )
L(    movl 8(%esp),                   %eax   )
L(    movl CPU(CPU_OFFSETOF_RUNNING), %ecx   )
L(    movl TASK_OFFSETOF_MMAN(%eax),  %eax   )
L(    movl TASK_OFFSETOF_MMAN(%ecx),  %ecx   )
L(    cmpl %eax, %ecx                        ) /* Check if memory managers (and thereby page directories) changed. */
L(    je   1f                                )
L(    movl MMAN_OFFSETOF_PPDIR(%ecx), %ecx   )
#if PDIR_OFFSETOF_DIRECTORY
L(    addl $(PDIR_OFFSETOF_DIRECTORY),%ecx   )
#endif
L(    movl %ecx, %cr3                        ) /* Switch to the new task's page directory. */
L(1:  popl %ecx                              )
L(    popl %eax                              )
L(    call cpu_sched_setrunning_save         ) /* Switch to the next task. */
L(    addl $4, %esp                          )
L(    popf                                   )
L(    ret                                    )
L(.size task_yield, . - task_yield           )
L(.previous                                  )
);


#define PIT_INTNO  IRQ_PIC1_PIT
PRIVATE ATTR_USED struct cpustate *FCALL
pit_exc(struct cpustate *__restrict state);
INTERN DEFINE_TASK_HANDLER(pit_irq,pit_exc);

INTDEF void KCALL
task_service_interrupt(struct task *__restrict self,
                       signal_t interrupt_signal);

PRIVATE ATTR_NOINLINE struct cpustate *FCALL
pit_exc(struct cpustate *__restrict state) {
 struct cpu *self = THIS_CPU;
 struct task *old_task;
 struct task *new_task;
 if (IRQ_PIC_SPURIOUS(PIT_INTNO)) return state;
 /* Will be re-enabled later, when iret restores EFLAGS.
  * Until then, we must make sure that no interrupts
  * cause a second preemption to be triggered. */
 PREEMPTION_DISABLE();

#if 0
 __assertion_printf("#!$ addr2line(%Ix) '{file}({line}) : {func} : PIT Trigger: %p'\n",
                   (uintptr_t)state->host.eip-1,state->host.eip);
#endif

 /* Rotate running tasks. */
 old_task = self->c_running;
 old_task->t_cstate = state;
 RING_NEXT(self->c_running,t_sched.sd_running);
 //cpu_sched_rotate(self);
 new_task = self->c_running;

 /* TODO: Update the sub-second clock. */

#if 1
 if (atomic_rwlock_trywrite(&self->c_lock)) {
  /* TODO: Wake sleeping tasks ('while (now() >= task.timeout) wakeup();'). */
#ifdef CONFIG_SMP
  /* Serve pending wake-ups. */
  while (self->c_wakeup) {
   struct task *wakeme = self->c_wakeup;
   assert(wakeme->t_sched.sd_wakeup.le_pself == &self->c_wakeup);
   LIST_REMOVE(wakeme,t_sched.sd_wakeup);
   RING_INSERT_BEFORE(new_task,wakeme,t_sched.sd_running);
   wakeme->t_state     = TASKSTATE_RUNNING;
   wakeme->t_realstate = TASKSTATE_RUNNING;
  }
  /* Check if the newly selected task has a pending state-change request. */
  assert(new_task->t_cpu == THIS_CPU);
  if (new_task->t_state != TASKSTATE_RUNNING) {
   struct task *next;
   assertf(new_task != &self->c_idle,
           "Cannot re-schedule the CPU-IDLE task");
   assert(new_task->t_sched.sd_running.re_prev != NULL);
   assert(new_task->t_sched.sd_running.re_prev != new_task);
   assert(new_task->t_sched.sd_running.re_next != NULL);
   assert(new_task->t_sched.sd_running.re_next != new_task);
   next = new_task->t_sched.sd_running.re_next;
   /* Update the task's real state to notify the change. */
   new_task->t_realstate = new_task->t_state;
   RING_REMOVE(new_task,t_sched.sd_running);
   self->c_running = next;
   /* Fix the list hooks of this task. */
   switch (new_task->t_state) {
   case TASKSTATE_SLEEPING:
    CPU_INSERT_SLEEPING(self,new_task);
    break;
   case TASKSTATE_SUSPENDED:
    CPU_INSERT_SUSPENDED(self,new_task);
    break;
   case TASKSTATE_TERMINATED:
    /* This case is special, because it requires us to drop */
    if (old_task == new_task) {
     /* Even more special: We can't delete this task, because we are this task.
      * Instead, we must switch to another task and later it delete us.
      * NOTE: For this case, even the IDLE task will work. */
     new_task->t_state     = TASKSTATE_TERMINATED;
     new_task->t_realstate = TASKSTATE_RUNNING;
     /* Insert the task after the next, so that once 'next' will
      * want to switch to it, it will finally be unscheduled. */
     RING_INSERT_AFTER(new_task,next,t_sched.sd_running);
     self->c_running = next;
    } else {
     /* We can actually delete this task! */
     atomic_rwlock_endwrite(&self->c_lock);
     /* Don't try to delete it while the cpu is still locked. */
     TASK_DECREF(new_task);
     goto done_sleepers;
    }
    break;
   default:
    assertf(0,"Invalid new state: %#x",(int)new_task->t_state);
    break;
   }
   new_task = next;
  }
#endif
  if ((new_task->t_flags&TASKFLAG_INTERRUPT) &&
       new_task->t_nointr == 0) {
   /* Notify of the interrupt. */
   signal_t intno = new_task->t_intno;
   bool call_service = !(new_task->t_flags&TASKFLAG_INTERRUPT_NOSERVE);
   new_task->t_flags &= ~(TASKFLAG_INTERRUPT|TASKFLAG_INTERRUPT_NOSERVE);
   atomic_rwlock_endwrite(&self->c_lock);
   if (call_service) task_service_interrupt(new_task,intno);
  } else {
   atomic_rwlock_endwrite(&self->c_lock);
  }
 }
#ifdef CONFIG_SMP
done_sleepers:
#endif
#endif
 if (old_task->t_mman != new_task->t_mman) {
  /* Switch page directories. */
  __asm__ __volatile__("movl %0, %%cr3\n"
                       :
                       : "r" (new_task->t_mman->m_ppdir)
                       : "memory");
 }
 /* Update the CPU's TSS with the new task's kernel stack.
  * NOTE: We're only safe to do so, because interrupts are still disabled! */
 self->c_arch.ac_tss.esp0 = (uintptr_t)new_task->t_hstack.hs_end;

 /* Signal completion of the PIT interrupt.
  * NOTE: Actual new signals will only be received once iret turns interrupts back on.
  *       And in the event that the new task didn't have interrupts
  *       enabled, they wont turn back on for a while. */
 IRQ_PIC_EOI(PIT_INTNO);

#if 0
 __assertion_printf("#!$ addr2line(%Ix) '{file}({line}) : {func} : Returning to: %p'\n",
                   (uintptr_t)new_task->t_cstate->host.eip,new_task->t_cstate->host.eip);
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

 /* Return the new CPU state that should be loaded next.
  * NOTE: Interrupts are re-enabled when iret is triggered. */
 return new_task->t_cstate;
}


PRIVATE ATTR_FREERODATA isr_t const pit_isr = ISR_DEFAULT(PIT_INTNO,&pit_irq);

INTERN ATTR_FREETEXT void KCALL sched_initialize(void) {

 /* Install the PIT IRQ handler. */
 irq_set(&pit_isr,NULL,IRQ_SET_RELOAD);

 /* Unmask the PIT Interrupt pin again. */
 IRQ_PIC1_STMASK(IRQ_PIC1_GTMASK() & ~(1 << (PIT_INTNO-IRQ_PIC1_BASE)));
}

DECL_END

#endif /* !GUARD_KERNEL_SCHED_CPU_C */
