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
#include <fs/fd.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/traceback.h>
#include <kernel/arch/cpu.h>
#include <kernel/arch/cpustate.h>
#include <kernel/arch/gdt.h>
#include <kernel/arch/mp.h>
#include <kernel/boot.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <kos/syslog.h>
#include <sched/cpu.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>

DECL_BEGIN

#if (BOOTSTACK_SIZE%PAGESIZE) != 0
#error "'BOOTSTACK_SIZE' must be page-aligned"
#endif

PUBLIC
ATTR_FREEDATA struct task inittask = {
    /* Reference counter:
     *   - inittask
     *   - __bootcpu.c_running
     *   - pid_global.pn_map[BOOTTASK_PID]
     *   - pid_init.pn_map[BOOTTASK_PID]
     */
    .t_refcnt    = 4,
#ifdef CONFIG_SMP
    .t_affinity_lock = ATOMIC_RWLOCK_INIT,
    .t_affinity  = CPU_SETALL,
    .t_cpu       = &__bootcpu,
#endif
    .t_flags    = TASKFLAG_NONE,
    .t_mode     = TASKMODE_RUNNING,
    .t_sched = {
        .sd_running = {
            .re_prev = &inittask,
            .re_next = &inittask,
        },
    },
    .t_priority  = TASKPRIO_DEFAULT,
    .t_prioscore = TASKPRIO_DEFAULT,
    .t_signals   = {
        .ts_slotc = 0,
        .ts_slota = 0,
        .ts_slotv = NULL,
        .ts_first = {
            .tss_self = &inittask,
        },
    },
    .t_event      = SIG_INIT,
    .t_critical  = 1, /* Start out as a critical task. */
    .t_nointr    = 1, /* Start out as a no-interrupt task. */
    .t_addrlimit = KERNEL_BASE,
    .t_suspend   = {0,0},
    .t_ustack    = NULL,
    .t_pid = {
        .tp_parlock   = ATOMIC_RWLOCK_INIT,
        .tp_leadlock  = ATOMIC_RWLOCK_INIT,
        .tp_childlock = ATOMIC_RWLOCK_INIT,
        .tp_grouplock = ATOMIC_RWLOCK_INIT,
        .tp_parent = &inittask,
        .tp_leader = &inittask,
        .tp_ids = {
            [PIDTYPE_GPID] = {
                .tl_pid = BOOTTASK_PID,
                .tl_ns  = &pid_global,
            },
            [PIDTYPE_PID] = {
                .tl_pid = BOOTTASK_PID,
                .tl_ns  = &pid_init,
            },
        },
        .tp_children  = &inittask,
        .tp_siblings  = { NULL, &inittask.t_pid.tp_children },
        .tp_group     = &inittask,
        .tp_grplink   = { NULL, &inittask.t_pid.tp_group },
    },
    .t_hstack = {
        .hs_begin = (ppage_t)(BOOTSTACK_ADDR),
        .hs_end   = (ppage_t)(BOOTSTACK_ADDR+BOOTSTACK_SIZE),
    },
    .t_mman_tasks = {
        .le_next  = &__bootcpu.c_idle,
        .le_pself = &mman_kernel.m_tasks,
    },
    .t_real_mman = &mman_kernel,
    .t_mman = &mman_kernel,
    .t_fdman = &fdman_kernel,
    .t_sighand = &sighand_kernel,
    .t_sigpend = SIGPENDING_INIT,
    .t_sigshare = &sigshare_kernel,
#ifndef CONFIG_NO_LDT
    .t_arch = {
        .at_ldt_tasks = {
            .le_pself = &ldt_kernel.l_tasks,
            .le_next  = &__bootcpu.c_idle,
        },
        .at_ldt_gdt = SEG(SEG_KERNEL_LDT),
        .at_ldt_tls = LDT_ERROR,
    },
#endif
};


INTDEF void ASMCALL cpu_idle(void);

PRIVATE ATTR_USED void invalid_idle_task(void) {
 __assertion_failedf(NULL,DEBUGINFO_GEN,
                     "Invalid IDLE task %p != %p",
                     THIS_TASK,&THIS_CPU->c_idle);
}

GLOBAL_ASM(
/* Nothing really to see here:
 * >> hlt forever, automatically serving
 *    interrupts as they arrive. */
L(.section .text                                   )
L(cpu_idle:                                        )
#ifdef CONFIG_DEBUG
L(    movl ASM_CPU(CPU_OFFSETOF_RUNNING), %eax     )
L(    movl ASM_CPU(CPU_OFFSETOF_SELF),    %ebx     )
L(    addl $(CPU_OFFSETOF_IDLE),          %ebx     )
L(    cmp %ebx, %eax                               )
L(    je 1f                                        )
L(    call invalid_idle_task                       )
L(1:                                               )
#endif
L(    hlt                                          )
#ifdef CONFIG_DEBUG
/* Add a bunch of hlt instructions to we can
 * watch them cycle when everything's working! */
L(    hlt                                          )
L(    hlt                                          )
L(    hlt                                          )
#endif
L(    jmp cpu_idle                                 )
L(.size cpu_idle, . - cpu_idle                     )
L(.previous                                        )
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
 else if (!strcasecmp(s,SETUPSTR("def"))) p = TASKPRIO_DEFAULT;
 else if (!strcasecmp(s,SETUPSTR("max"))) p = TASKPRIO_MAX;
 else p = strtol(s,NULL,0);
 return p;
}

DEFINE_EARLY_SETUP("boot-priority=",boot_priority) {
 __bootcpu.c_prio_min =
 __bootcpu.c_prio_max =
 inittask.t_priority = 
 inittask.t_prioscore = prio_from_string(arg);
 return true;
}
DEFINE_EARLY_SETUP("idle-priority=",idle_priority) {
 __bootcpu.c_idle.t_priority = 
 __bootcpu.c_idle.t_prioscore = prio_from_string(arg);
 return true;
}

ATTR_USED ATTR_SECTION(".boot.cpu")
PUBLIC struct cpu __bootcpu = {
    .c_self       = &__bootcpu,
    .c_running    = &inittask,
    .c_idling     = &__bootcpu.c_idle,
    .c_id         = CPUID_BOOTCPU,
    .c_prio_min   = TASKPRIO_DEFAULT,
    .c_prio_max   = TASKPRIO_DEFAULT,
    .c_lock       = ATOMIC_RWLOCK_INIT,
    .c_idle       = {
        /* Reference counter:
         *   - __bootcpu.c_idle
         *   - __bootcpu.c_idling
         *   - pid_global.pn_map[BOOTCPU_IDLE_PID]
         *   - pid_init.pn_map[BOOTCPU_IDLE_PID]
         */
#ifdef CONFIG_DEBUG
        .t_refcnt    = 4,
#else
        .t_refcnt    = 0x80000004,
#endif
        /* The location of the bootstrap cpu-state block. */
        .t_cstate    = (struct cpustate *)&__bootidlestack.s_boot,
#ifdef CONFIG_SMP
        .t_affinity_lock = ATOMIC_RWLOCK_INIT,
        .t_affinity = CPU_SETONE(CPUID_BOOTCPU),
        .t_cpu       = &__bootcpu,
#endif
        .t_flags     = TASKFLAG_NOTALEADER,
        .t_mode      = TASKMODE_RUNNING,
        .t_sched = {
            .sd_running = {
                .re_prev = NULL,
                .re_next = NULL,
            },
        },
        .t_priority  = TASKPRIO_MINIDLE,
        .t_prioscore = TASKPRIO_MINIDLE,
        .t_signals   = {
            .ts_slotc = 0,
            .ts_slota = 0,
            .ts_slotv = NULL,
            .ts_first = {
                .tss_self = &__bootcpu.c_idle,
            },
        },
        .t_event     = SIG_INIT,
        .t_critical  = 1,
        .t_nointr    = 1,
        .t_addrlimit = KERNEL_BASE,
        .t_suspend   = {0,0},
        .t_ustack    = NULL,
        .t_pid = {
            .tp_parlock   = ATOMIC_RWLOCK_INIT,
            .tp_leadlock  = ATOMIC_RWLOCK_INIT,
            .tp_childlock = ATOMIC_RWLOCK_INIT,
            .tp_grouplock = ATOMIC_RWLOCK_INIT,
            .tp_parent = &__bootcpu.c_idle,
            .tp_leader = &__bootcpu.c_idle,
            .tp_ids = {
                [PIDTYPE_GPID] = {
                    .tl_pid = BOOTCPU_IDLE_PID,
                    .tl_ns  = &pid_global,
                },
                [PIDTYPE_PID] = {
                    .tl_pid = BOOTCPU_IDLE_PID,
                    .tl_ns  = &pid_init,
                },
            },
            .tp_children  = &__bootcpu.c_idle,
            .tp_siblings  = { NULL, &__bootcpu.c_idle.t_pid.tp_children },
            .tp_group     = &__bootcpu.c_idle,
            .tp_grplink   = { NULL, &__bootcpu.c_idle.t_pid.tp_group },
        },
        .t_hstack = {
            .hs_begin = (ppage_t)((uintptr_t)&__bootidlestack),
            .hs_end   = (ppage_t)((uintptr_t)&__bootidlestack+BOOTSTACK_SIZE),
        },
        /* NOTE: Technically, IDLE tasks can run under any page directory,
         *       but adding an exception just for them may already negate
         *       the overhead that such additional checks would otherwise
         *       carry. - Especially considering that IDLE tasks aren't
         *       meant to be run continuously at all! */
        .t_mman_tasks = {
            .le_next  = NULL,
            .le_pself = &inittask.t_mman_tasks.le_next,
        },
        .t_real_mman = &mman_kernel,
        .t_mman = &mman_kernel,
        .t_fdman = &fdman_kernel,
        .t_sighand = &sighand_kernel,
        .t_sigpend = SIGPENDING_INIT,
        .t_sigshare = &sigshare_kernel,
#ifndef CONFIG_NO_LDT
        .t_arch = {
            .at_ldt_tasks = {
                .le_pself = &inittask.t_arch.at_ldt_tasks.le_next,
                .le_next  = NULL,
            },
            .at_ldt_gdt = SEG(SEG_KERNEL_LDT),
            .at_ldt_tls = LDT_ERROR,
        },
#endif
    },
    .c_arch = {
#if defined(__i386__) || defined(__x86_64__)
        .ac_tss = {
            /* Define initial boot-cpu TSS data. */
            .esp0       = (uintptr_t)(BOOTSTACK_ADDR+BOOTSTACK_SIZE),
            .ss0        = SEG(SEG_KERNEL_DATA),
            .iomap_base = sizeof(struct tss),
            .eflags     = EFLAGS_IF,
        },
        .ac_mode          = CPUMODE_ONLINE,
        .ac_flags         = CPUFLAG_NONE,
        .ac_lapic_id      = 0, /* Likely updated later */
        .ac_lapic_version = APICVER_82489DX, /* Likely updated later */
#endif
    },
    .c_n_run   = 1,
    .c_n_idle  = 1,
    .c_n_susp  = 0,
    .c_n_sleep = 0,
};


INTDEF ATTR_USED struct cpustate *FCALL pit_exc(struct cpustate *__restrict state);
INTERN DEFINE_TASK_HANDLER(pit_irq,pit_exc);
PRIVATE ATTR_FREERODATA isr_t const pit_isr = ISR_DEFAULT(IRQ_PIC1_PIT,&pit_irq);
INTERN ATTR_FREETEXT void KCALL sched_initialize(void) {

 /* Install the PIT IRQ handler. */
 irq_set(&pit_isr,NULL,IRQ_SET_RELOAD);

 /* Unmask the PIT Interrupt pin again. */
 IRQ_PIC1_STMASK(IRQ_PIC1_GTMASK() & ~(1 << (IRQ_PIC1_PIT-IRQ_PIC1_BASE)));
}

DECL_END

#endif /* !GUARD_KERNEL_SCHED_CPU_C */
