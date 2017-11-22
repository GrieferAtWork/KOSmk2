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
#ifndef GUARD_KERNEL_ARCH_CPU_C
#define GUARD_KERNEL_ARCH_CPU_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1 /* CPU_* */

#include "../../../kernel/debug-config.h"

#include <assert.h>
#include <fs/fd.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/traceback.h>
#include <arch/cpu.h>
#include <arch/cpustate.h>
#include <arch/gdt.h>
#include <arch/mp.h>
#include <kernel/boot.h>
#include <kernel/interrupt.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <sys/syslog.h>
#include <sched/cpu.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>
#include <kernel/memory.h>
#include <kernel/export.h>
#include <kos/thread.h>
#include <asm/instx.h>
#include <arch/hints.h>
#include <arch/pic.h>
#include <arch/asm.h>
#include <arch/memory.h>

DECL_BEGIN

#if (HOST_BOOT_STCKSIZE%PAGESIZE) != 0
#error "'HOST_BOOT_STCKSIZE' must be page-aligned"
#endif

PUBLIC
ATTR_FREEDATA struct task inittask = {
    /* Reference counter:
     *   - inittask
     *   - __bootcpu.c_running
     *   - pid_global.pn_map[BOOTTASK_PID]
     *   - pid_init.pn_map[BOOTTASK_PID]
     */
#ifdef CONFIG_DEBUG
    .t_refcnt    = 4,
#else
    .t_refcnt    = 0x80000004,
#endif
    .t_weakcnt   = 1, /* Held by the non-zero `t_refcnt' */
#ifdef CONFIG_SMP
    .t_affinity_lock = ATOMIC_RWLOCK_INIT,
    .t_affinity  = CPU_SETALL,
    .t_cpu       = &__bootcpu,
#endif
    .t_flags     = TASKFLAG_NONE,
    .t_mode      = TASKMODE_RUNNING,
    .t_sched = { /* When booting, only `inittask' is running. */
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
    .t_event     = SIG_INIT,
    .t_critical  = 1, /* Start out as a critical task. */
    .t_nointr    = 1, /* Start out as a no-interrupt task. */
    .t_addrlimit = VM_USER_MAX+1,
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
        .hs_begin = (ppage_t)(__bootstack),
        .hs_end   = (ppage_t)(__bootstack+HOST_BOOT_STCKSIZE),
    },
    .t_mman_tasks = {
        .le_next  = &__bootcpu.c_idle,
        .le_pself = &mman_kernel.m_tasks,
    },
    .t_real_mman = &mman_kernel,
    .t_mman = &mman_kernel,
    .t_fdman = &fdman_kernel,
#ifndef CONFIG_NO_SIGNALS
    .t_sighand = &sighand_kernel,
    .t_sigpend = SIGPENDING_INIT,
    .t_sigshare = &sigshare_kernel,
#endif /* !CONFIG_NO_SIGNALS */
#ifndef CONFIG_NO_LDT
    .t_arch = {
        .at_ldt_tasks = {
            .le_pself = &ldt_kernel.l_tasks,
            .le_next  = &__bootcpu.c_idle,
        },
        .at_ldt_gdt = SEG(SEG_KERNEL_LDT),
    },
#endif
};


INTDEF void ASMCALL cpu_idle(void);

#ifdef CONFIG_DEBUG
PRIVATE ATTR_USED void invalid_idle_task(void) {
 __NAMESPACE_INT_SYM
 __afailf(NULL,DEBUGINFO_GEN,
          "Invalid IDLE task %p != %p",
          THIS_TASK,&THIS_CPU->c_idle);
}
#endif


PRIVATE ATTR_USED void switch_before_idle(void) {
 assert(PREEMPTION_ENABLED());
 while (THIS_CPU->c_n_run > 1) {
  /* Switch to another task. */
  PREEMPTION_DISABLE();
  assert(THIS_CPU->c_running == &THIS_CPU->c_idle);
  THIS_CPU->c_running = THIS_CPU->c_idle.t_sched.sd_running.re_next;
  TASK_SWITCH_CONTEXT(&THIS_CPU->c_idle,THIS_CPU->c_running);
  cpu_sched_setrunning_savef(&THIS_CPU->c_idle,EFLAGS_IF);
 }
}

GLOBAL_ASM(
/* Nothing really to see here:
 * >> hlt forever, automatically serving
 *    interrupts as they arrive. */
L(.section .text                                   )
L(PRIVATE_ENTRY(cpu_idle)                          )
#ifdef CONFIG_DEBUG
L(    movx ASM_CPU(CPU_OFFSETOF_RUNNING), %xax     )
L(    movx ASM_CPU(CPU_OFFSETOF_SELF),    %xbx     )
L(    addx $(CPU_OFFSETOF_IDLE),          %xbx     )
L(    cmpx %xbx, %xax                              )
L(    je   1f                                      )
L(    call invalid_idle_task                       )
L(1:                                               )
#endif
#if CONFIG_NO_IDLE
L(    call task_yield                              )
#else /* CONFIG_NO_IDLE */
L(    call switch_before_idle                      )
L(    hlt                                          )
#endif /* !CONFIG_NO_IDLE */
L(    jmp cpu_idle                                 )
L(SYM_END(cpu_idle)                                )
L(.previous                                        )
);


INTDEF byte_t __kernel_nofree_size[];

/* Allocate a small stack for the IDLE task of the boot CPU.
 * NOTE: This stack isn't actually require for the task itself,
 *       but for interrupts such as PIT timer, or others. */
INTERN ATTR_ALIGNED(16) struct PACKED {
 struct meminfo       s_kmeminfo[MEMORY_PREDEF_COUNT];
#ifdef __x86_64__
 byte_t               s_data[HOST_IDLE_STCKSIZE-
                            (sizeof(struct cpustate)+
                             sizeof(struct meminfo)*MEMORY_PREDEF_COUNT+
                             sizeof(size_t))];
 struct cpustate      s_boot;
#else
 byte_t               s_data[HOST_IDLE_STCKSIZE-
                            (sizeof(struct cpustate_host)+
                             sizeof(struct meminfo)*MEMORY_PREDEF_COUNT+
                             sizeof(size_t))];
 struct cpustate_host s_boot;
#endif
} __bootidlestack = {
    /* Bootstrap kernel memory info.
     * >> Used to describe the kernel itself in physical memory. */
    .s_kmeminfo = { MEMORY_PREDEF_LIST },
#ifdef CONFIG_DEBUG
    .s_data = {
        [0 ... COMPILER_LENOF(__bootidlestack.s_data)-1] = (KERNEL_DEBUG_MEMPAT_HOSTSTACK & 0xff)
    },
#endif /* CONFIG_DEBUG */
    .s_boot = {
        .sg = {
#ifdef __x86_64__
            .gs_base = (uintptr_t)&__bootcpu,
            .fs_base = 0,
#else
            .gs = __KERNEL_DS,
            .fs = __KERNEL_PERCPU,
            .es = __KERNEL_DS,
            .ds = __KERNEL_DS,
#endif
        },
        .iret = {
            .xip    = (uintptr_t)&cpu_idle,
            .cs     = __KERNEL_CS,
            /* All other flags don't matter, but `IF' (interrupt flag) must be set.
             * If it wasn't, the idle task would otherwise block forever! */
            .xflags = EFLAGS_IF|EFLAGS_IOPL(0),
#ifdef __x86_64__
            .userxsp = (uintptr_t)&__bootidlestack+HOST_IDLE_STCKSIZE,
            .ss      = __KERNEL_DS,
#endif
        },
    },
};

#ifndef CONFIG_NO_JOBS
INTERN ATTR_RAREBSS ATTR_ALIGNED(16) u8 __bootworkstack[HOST_WOKER_STCKSIZE];
#ifdef __x86_64__
#define WORKSTATE ((struct cpustate *)(__bootworkstack+(HOST_WOKER_STCKSIZE-sizeof(struct cpustate))))
#else
#define WORKSTATE ((struct cpustate_host *)(__bootworkstack+(HOST_WOKER_STCKSIZE-sizeof(struct cpustate_host))))
#endif

#endif /* !CONFIG_NO_JOBS */

PRIVATE ATTR_FREETEXT taskprio_t KCALL
prio_from_string(char *__restrict s) {
 taskprio_t p;
 if (!strcasecmp(s,SETUPSTR("idle")) ||
     !strcasecmp(s,SETUPSTR("maxidle"))) p = TASKPRIO_MAXIDLE;
 else if (!strcasecmp(s,SETUPSTR("minidle"))) p = TASKPRIO_MINIDLE;
 else if (!strcasecmp(s,SETUPSTR("min"))) p = TASKPRIO_MIN;
 else if (!strcasecmp(s,SETUPSTR("def"))) p = TASKPRIO_DEFAULT;
 else if (!strcasecmp(s,SETUPSTR("max"))) p = TASKPRIO_MAX;
 else p = (taskprio_t)strtol(s,NULL,0);
 return p;
}

DEFINE_EARLY_SETUP("init-priority=",init_priority) {
 __bootcpu.c_prio_min = __bootcpu.c_prio_max =
 inittask.t_priority = inittask.t_prioscore = prio_from_string(arg);
 return true;
}
DEFINE_EARLY_SETUP("idle-priority=",idle_priority) {
 __bootcpu.c_idle.t_priority = 
 __bootcpu.c_idle.t_prioscore = prio_from_string(arg);
 return true;
}
#ifndef CONFIG_NO_JOBS
DEFINE_EARLY_SETUP("work-priority=",work_priority) {
 __bootcpu.c_work.t_priority = 
 __bootcpu.c_work.t_prioscore = prio_from_string(arg);
 return true;
}
#endif /* !CONFIG_NO_JOBS */

ATTR_USED ATTR_SECTION(".boot.cpu")
PUBLIC struct cpu __bootcpu = {
    .c_self       = &__bootcpu,
    .c_running    = &inittask,
    .c_idling     = &__bootcpu.c_idle,
    .c_id         = CPUID_BOOTCPU,
    .c_prio_min   = TASKPRIO_DEFAULT,
    .c_prio_max   = TASKPRIO_DEFAULT,
#ifndef CONFIG_NO_JOBS
    .c_suspended  = &__bootcpu.c_work,
#else
    .c_suspended  = NULL,
#endif
    .c_sleeping   = NULL,
    .c_lock       = ATOMIC_RWLOCK_INIT,
    .c_idle = {
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
        .t_weakcnt   = 1, /* Held by the non-zero `t_refcnt' */
        /* The location of the bootstrap cpu-state block. */
        .t_cstate    = (struct cpustate *)&__bootidlestack.s_boot,
#ifdef CONFIG_SMP
        .t_affinity_lock = ATOMIC_RWLOCK_INIT,
        .t_affinity = CPU_SETONE(CPUID_BOOTCPU),
        .t_cpu       = &__bootcpu,
#endif
        .t_flags     = TASKFLAG_NOTALEADER|TASKFLAG_NOSIGNALS,
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
        .t_addrlimit = VM_USER_MAX+1,
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
            .hs_end   = (ppage_t)((uintptr_t)&__bootidlestack+HOST_IDLE_STCKSIZE),
        },
        /* NOTE: Technically, IDLE tasks can run under any page directory,
         *       but adding an exception just for them may already negate
         *       the overhead that such additional checks would otherwise
         *       carry. - Especially considering that IDLE tasks aren't
         *       meant to be run continuously at all! */
        .t_mman_tasks = {
#ifndef CONFIG_NO_JOBS
            .le_next  = &__bootcpu.c_work,
#else /* !CONFIG_NO_JOBS */
            .le_next  = NULL,
#endif /* CONFIG_NO_JOBS */
            .le_pself = &inittask.t_mman_tasks.le_next,
        },
        .t_real_mman = &mman_kernel,
        .t_mman = &mman_kernel,
        .t_fdman = &fdman_kernel,
#ifndef CONFIG_NO_SIGNALS
        .t_sighand = &sighand_kernel,
        .t_sigpend = SIGPENDING_INIT,
        .t_sigblock = __SIGSET_INIT_FULL,
        .t_sigshare = &sigshare_kernel,
#endif /* !CONFIG_NO_SIGNALS */
#ifndef CONFIG_NO_LDT
        .t_arch = {
            .at_ldt_tasks = {
                .le_pself = &inittask.t_arch.at_ldt_tasks.le_next,
#ifndef CONFIG_NO_JOBS
                .le_next  = &__bootcpu.c_work,
#else /* !CONFIG_NO_JOBS */
                .le_next  = NULL,
#endif /* CONFIG_NO_JOBS */
            },
            .at_ldt_gdt = SEG(SEG_KERNEL_LDT),
        },
#endif /* !CONFIG_NO_LDT */
    },
#ifndef CONFIG_NO_JOBS
    .c_work = {
        /* Reference counter:
         *   - __bootcpu.c_work
         *   - __bootcpu.c_idling->t_sched.re_[prev|next]
         *   - pid_global.pn_map[BOOTCPU_IDLE_PID]
         *   - pid_init.pn_map[BOOTCPU_IDLE_PID]
         */
#ifdef CONFIG_DEBUG
        .t_refcnt    = 4,
#else
        .t_refcnt    = 0x80000004,
#endif
        .t_weakcnt   = 1, /* Held by the non-zero `t_refcnt' */
        /* The location of the bootstrap cpu-state block. */
        .t_cstate    = (struct cpustate *)WORKSTATE,
#ifdef CONFIG_SMP
        .t_affinity_lock = ATOMIC_RWLOCK_INIT,
        .t_affinity  = CPU_SETONE(CPUID_BOOTCPU),
        .t_cpu       = &__bootcpu,
#endif
        .t_flags     = TASKFLAG_NOTALEADER|TASKFLAG_NOSIGNALS,
        .t_mode      = TASKMODE_SUSPENDED,
        .t_sched = {
            .sd_suspended = {
                .le_pself = &__bootcpu.c_suspended,
                .le_next  = NULL,
            },
        },
        .t_priority  = TASKPRIO_MAX,
        .t_prioscore = TASKPRIO_MAX,
        .t_signals   = {
            .ts_slotc = 0,
            .ts_slota = 0,
            .ts_slotv = NULL,
            .ts_first = {
                .tss_self = &__bootcpu.c_work,
            },
        },
        .t_event     = SIG_INIT,
        .t_critical  = 1,
        .t_nointr    = 1,
        .t_addrlimit = VM_USER_MAX+1,
        .t_suspend   = {0,0},
        .t_ustack    = NULL,
        .t_pid = {
            .tp_parlock   = ATOMIC_RWLOCK_INIT,
            .tp_leadlock  = ATOMIC_RWLOCK_INIT,
            .tp_childlock = ATOMIC_RWLOCK_INIT,
            .tp_grouplock = ATOMIC_RWLOCK_INIT,
            .tp_parent = &__bootcpu.c_work,
            .tp_leader = &__bootcpu.c_work,
            .tp_ids = {
                [PIDTYPE_GPID] = {
                    .tl_pid = BOOTCPU_WORK_PID,
                    .tl_ns  = &pid_global,
                },
                [PIDTYPE_PID] = {
                    .tl_pid = BOOTCPU_WORK_PID,
                    .tl_ns  = &pid_init,
                },
            },
            .tp_children  = &__bootcpu.c_work,
            .tp_siblings  = { NULL, &__bootcpu.c_work.t_pid.tp_children },
            .tp_group     = &__bootcpu.c_work,
            .tp_grplink   = { NULL, &__bootcpu.c_work.t_pid.tp_group },
        },
        .t_hstack = {
            .hs_begin = (ppage_t)((uintptr_t)&__bootworkstack),
            .hs_end   = (ppage_t)((uintptr_t)&__bootworkstack+HOST_WOKER_STCKSIZE),
        },
        /* NOTE: Technically, IDLE tasks can run under any page directory,
         *       but adding an exception just for them may already negate
         *       the overhead that such additional checks would otherwise
         *       carry. - Especially considering that IDLE tasks aren't
         *       meant to be run continuously at all! */
        .t_mman_tasks = {
            .le_next  = NULL,
            .le_pself = &__bootcpu.c_idle.t_mman_tasks.le_next,
        },
        .t_real_mman = &mman_kernel,
        .t_mman = &mman_kernel,
        .t_fdman = &fdman_kernel,
#ifndef CONFIG_NO_SIGNALS
        .t_sighand = &sighand_kernel,
        .t_sigpend = SIGPENDING_INIT,
        .t_sigblock = __SIGSET_INIT_FULL,
        .t_sigshare = &sigshare_kernel,
#endif /* !CONFIG_NO_SIGNALS */
#ifndef CONFIG_NO_LDT
        .t_arch = {
            .at_ldt_tasks = {
                .le_pself = &__bootcpu.c_idle.t_arch.at_ldt_tasks.le_next,
                .le_next  = NULL,
            },
            .at_ldt_gdt = SEG(SEG_KERNEL_LDT),
        },
#endif /* !CONFIG_NO_LDT */
    },
#endif /* !CONFIG_NO_JOBS */
    .c_arch = {
#if defined(__i386__) || defined(__x86_64__)
        .ac_tss = {
            /* Define initial boot-cpu TSS data. */
#ifdef __x86_64__
            .rsp0       = (uintptr_t)(__bootstack+HOST_BOOT_STCKSIZE),
#else
            .esp0       = (uintptr_t)(__bootstack+HOST_BOOT_STCKSIZE),
            .ss0        = __KERNEL_DS,
            .eflags     = EFLAGS_IF,
#endif
            .iomap_base = sizeof(struct tss),
        },
        .ac_mode          = CPUMODE_ONLINE,
        /* These flags are deleted if the opposite is proven during early boot. */
        .ac_flags         = CPUFLAG_486|CPUFLAG_IINIT,
        .ac_lapic_id      = 0, /* Likely updated later */
        .ac_lapic_version = APICVER_82489DX, /* Likely updated later */
#endif
    },
    .c_n_run   = 1,
    .c_n_idle  = 1,
#ifndef CONFIG_NO_JOBS
    .c_n_susp  = 1,
#else /* !CONFIG_NO_JOBS */
    .c_n_susp  = 0,
#endif /* CONFIG_NO_JOBS */
    .c_n_sleep = 0,
};

#ifndef CONFIG_NO_JOBS
/* Function implementing the per-cpu worker loop, and associated scheduling. */
INTDEF ATTR_NORETURN void KCALL cpu_jobworker(void);
#endif /* !CONFIG_NO_JOBS */

INTDEF struct interrupt pit_interrupt;

INTERN ATTR_FREETEXT void KCALL sched_initialize(void) {
#ifndef CONFIG_NO_JOBS
 /* Initialize the stack of the boot CPU's worker task. */
 WORKSTATE->iret.cs     = __KERNEL_CS;
 WORKSTATE->iret.xflags = EFLAGS_IF|EFLAGS_IOPL(3);
 WORKSTATE->iret.xip    = (uintptr_t)&cpu_jobworker;
 memset(&WORKSTATE->gp,0,sizeof(WORKSTATE->gp));
#ifdef __x86_64__
 WORKSTATE->iret.userxsp = (uintptr_t)__bootworkstack+HOST_WOKER_STCKSIZE;
 WORKSTATE->iret.ss      = __KERNEL_DS;
 WORKSTATE->sg.gs_base   = (uintptr_t)&__bootcpu;
 WORKSTATE->sg.fs_base   = 0;
#else /* __x86_64__ */
 WORKSTATE->sg.ds       = __KERNEL_DS;
 WORKSTATE->sg.es       = __KERNEL_DS;
 WORKSTATE->sg.fs       = __KERNEL_PERCPU;
 WORKSTATE->sg.gs       = __KERNEL_DS;
#endif /* !__x86_64__ */
#endif /* !CONFIG_NO_JOBS */

 /* Install the PIT IRQ handler. */
 asserte(E_ISOK(int_addall(&pit_interrupt)));

 /* Unmask the PIT Interrupt pin again. */
 PIC1_STMASK(PIC1_GTMASK() & ~(1 << (INTNO_PIC1_PIT-INTNO_PIC1_BASE)));
}

DECL_END

#endif /* !GUARD_KERNEL_ARCH_CPU_C */
