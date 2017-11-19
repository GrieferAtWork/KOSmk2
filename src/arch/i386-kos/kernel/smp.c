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
#ifndef GUARD_KERNEL_ARCH_SMP_C
#define GUARD_KERNEL_ARCH_SMP_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#ifdef CONFIG_SMP
#include <assert.h>
#include <dev/rtc.h>
#include <errno.h>
#include <fs/fd.h>
#include <hybrid/align.h>
#include <asm/cpu-flags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/critical.h>
#include <hybrid/section.h>
#include <arch/apic.h>
#include <arch/apicdef.h>
#include <arch/gdt.h>
#include <arch/idt_pointer.h>
#include <arch/mp.h>
#include <arch/realmode.h>
#include <kernel/export.h>
#include <kernel/interrupt.h>
#include <kernel/paging.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <sched/signal.h>
#include <sched/smp.h>
#include <sched/task.h>
#include <sched/types.h>
#include <string.h>
#include <sys/io.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <arch/asm.h>
#include <asm/instx.h>
#include "interrupt_intern.h"


DECL_BEGIN

#if __SIZEOF_POINTER__ == 8
#   define MEMSETX   memsetq
#   define MEMCPYX   memcpyq
#elif __SIZEOF_POINTER__ == 4
#   define MEMSETX   memsetl
#   define MEMCPYX   memcpyl
#elif __SIZEOF_POINTER__ == 2
#   define MEMSETX   memsetw
#   define MEMCPYX   memcpyw
#elif __SIZEOF_POINTER__ == 1
#   define MEMSETX   memsetb
#   define MEMCPYX   memcpyb
#else
#   error "Unsupported sizeof(void *)"
#endif


#define I8253_IOPORT_CMD  0x43
#define I8253_IOPORT_DATA 0x40

#define I8253_CMD_READCOUNT 0

FORCELOCAL u16 KCALL i8253_readcount(void) {
 /* TODO: Protect this function with a spinlock. */
 outb(I8253_IOPORT_CMD,I8253_CMD_READCOUNT);
 return (u16)inb(I8253_IOPORT_DATA) |
       ((u16)inb(I8253_IOPORT_DATA) << 8);
}

FORCELOCAL void KCALL i8253_delay10ms(void) {
 u16 curr_count,prev_count;
 curr_count = i8253_readcount();
 do {
  prev_count = curr_count;
  curr_count = i8253_readcount();
 } while ((curr_count-prev_count) < 300);
}

FORCELOCAL void KCALL i8253_delay(unsigned int n10ms) {
 while (n10ms--) i8253_delay10ms();
}


#if defined(CONFIG_RESERVE_NULL_PAGE) && \
    FLOOR_ALIGN(TRAMPOLINE_PHYS_LOW,PAGESIZE) == 0
INTDEF VIRT ppage_t cpu_trampoline_jmppage;
#define LAPIC_JMPADDR(x) ((uintptr_t)cpu_trampoline_jmppage+((x)&(PAGESIZE-1)))
#else
#define LAPIC_JMPADDR(x)  (x)
#endif


/* Default-initialize SMP data structures. */
INTERN struct cpu *const pboot_cpu = &__bootcpu;
PUBLIC struct hwcpu smp_hwcpu = {
    .hw_onln = 1,
    .hw_cpuc = 1,
    .hw_cpuv = &pboot_cpu,
};




PUBLIC DEFINE_RWLOCK(apic_lock);
#define CPUINIT_IPI_ATTEMPTS 3    /*< Amount of times init should be attempted through IPI. */

INTDEF uintptr_t           cpu_bootstrap;
INTDEF struct idt_pointer *cpu_bootstrap_gdt; /*< [lock(apic_lock)] realmode symbol describing the GDT loaded during bootup. */
INTDEF ATTR_NORETURN void  cpu_bootstrap_c(void);


GLOBAL_ASM(
L(RM_BEGIN_EX(0x1000)                                   )
L(DEFINE_PRIVATE(cpu_bootstrap)                            )
L(cpu_bootstrap:                                        )
//L(1:  jmp 1b                                            )
L(    /* This is where a newly started CPU begins */    )
L(    /* Note, that the CPU is currently in real-mode */)
L(    cli /* Just to be safe: Disable interrupts */     )
L(                                                      )
L(    /* Load our descriptor table. */                  )
L(    RM_LGDTL(cpu_bootstrap_gdt)                       )
L(                                                      )
L(    /* Enable long-mode */                            )
L(    movw  $0x1, %ax                                   )
L(    lmsw  %ax                                         )
L(                                                      )
L(    /* Escape from real-mode */                       )
L(    ljmpl $(__KERNEL_CS), $cpu_bootstrap_32           )
L(DEFINE_BSS(cpu_bootstrap_gdt,6)                       )
L(RM_END                                                )
);

// L(1:  mov $'!', %al                                     )
// L(    mov $0x3f8, %dx                                   )
// L(    outb %al, %dx                                     )
// L(    jmp 1b                                            )

GLOBAL_ASM(
L(.section .text.phys                                   )
L(cpu_bootstrap_32:                                     )
L(    /* Load proper segment registers */               )
L(    movw  $(SEG(SEG_HOST_DATA)), %ax                )
L(    movw  $(SEG(SEG_CPUSELF)), %bx                    )
L(    movw  %ax, %ds                                    )
L(    movw  %ax, %es                                    )
#ifdef __i386__
L(    movw  %bx, %fs                                    )
L(    movw  %ax, %gs                                    )
#else
L(    movw  %ax, %fs                                    )
L(    movw  %bx, %gs                                    )
#endif
L(    movw  %ax, %ss                                    )
L(                                                      )
L(    /* Enable paging */                               )
L(    movl $(pdir_kernel), %eax                         )
L(    movl %eax,           %cr3                         )
L(    movl %cr0,           %eax                         )
L(    orl  $(CR0_PG),      %eax                         )
L(    movl %eax,           %cr0                         )
L(                                                      )
L(    /* Load the stack of the IDLE task. */            )
L(    movl ASM_CPU(CPU_OFFSETOF_IDLE+ \
                   TASK_OFFSETOF_HSTACK+ \
                   HSTACK_OFFSETOF_END), \
           %esp                                         )
L(                                                      )
L(    /* Time to switch to C */                         )
L(DEFINE_PRIVATE(cpu_bootstrap_c)                          )
L(    jmp cpu_bootstrap_c                               )
L(.size cpu_bootstrap_32, . - cpu_bootstrap_32          )
L(.previous                                             )
);

INTERN ATTR_NORETURN void cpu_bootstrap_c(void) {
 /* Acknowledge the CPU boot sequence. */
 sig_write(&THIS_CPU->c_arch.ac_sigonoff);
 switch (THIS_CPU->c_arch.ac_mode) {

 case CPUMODE_STARTUP:
  /* Acknowledge a startup sequence. */
  ATOMIC_WRITE(THIS_CPU->c_arch.ac_mode,CPUMODE_ONLINE);
  break;

 case CPUMODE_SHUTDOWN:
 case CPUMODE_SHUTDOWN_NOMIGRATE:
 case CPUMODE_OFFLINE: /* This can happen due to a race condition with startup timeouts. */
  syslog(LOG_SCHED|LOG_INFO,
         "[SMP] CPU #%d shutting down immediately!\n",
         THIS_CPU->c_id);
  __cpu_shutdown_now_endwrite();
  __builtin_unreachable();

 default:
  assertf(0,"CPU #%d is in an invalid mode %d",
          THIS_CPU->c_id,THIS_CPU->c_arch.ac_mode);
  break;
 }

 ATOMIC_FETCHINC(smp_hwcpu.hw_onln);

 /* Signal that the CPU is now online! */
 sig_broadcast_unlocked(&THIS_CPU->c_arch.ac_sigonoff);
 sig_endwrite(&THIS_CPU->c_arch.ac_sigonoff);

 /* At this point, this CPU is fully online, and we're ready to start working!
  * NOTE: Interrupts are still disabled. */
 assert(!PREEMPTION_ENABLED());
 assert(THIS_CPU != &__bootcpu);
 assert(THIS_CPU->c_id != CPUID_BOOTCPU);

 /* Load our own IDT table. */
 { struct idt_pointer idt;
   idt.ip_idt   = CPU(inttab).it_idt;
   idt.ip_limit = sizeof(inttab.it_idt)-1;
   __asm__ __volatile__("lidt %0\n" : : "m" (idt));
 }

 syslog(LOG_SCHED|LOG_DEBUG,"[SMP] Secondary CPU!\n");

 if (THIS_CPU->c_arch.ac_flags&CPUFLAG_LAPIC) {
  /* Enable LAPIC before turning on interrupts! */
  apic_write(APIC_SPIV,INTNO_LAPIC_SPURIOUS|APIC_SPIV_APIC_ENABLED);
 }

 /* TODO: Switch to the currently scheduled task. */

#if 0
 PREEMPTION_ENABLE();
 for (;;) PREEMPTION_IDLE();
#endif
 
 __cpu_shutdown_now();
 __builtin_unreachable();
}


PUBLIC void KCALL __cpu_shutdown_now_endwrite(void) {
 /* Actually do shut down the CPU. */
 assert(sig_writing(&THIS_CPU->c_arch.ac_sigonoff));
 assertf(THIS_CPU->c_arch.ac_mode != CPUMODE_OFFLINE,
         "CPU already marked as offline (Did someone else write this?)");
 ATOMIC_WRITE(THIS_CPU->c_arch.ac_mode,CPUMODE_OFFLINE);
 /* Update the online-cpu counter. */
 ATOMIC_FETCHDEC(smp_hwcpu.hw_onln);
 sig_broadcast_unlocked(&THIS_CPU->c_arch.ac_sigonoff);
 sig_endwrite(&THIS_CPU->c_arch.ac_sigonoff);
 PREEMPTION_FREEZE();
}

PUBLIC void KCALL __cpu_shutdown_now(void) {
 /* Actually do shut down the CPU. */
 sig_write(&THIS_CPU->c_arch.ac_sigonoff);
 __cpu_shutdown_now_endwrite();
}

PRIVATE SAFE errno_t KCALL
apic_exec_ipc(u32 icr_word, u32 icr2_word) {
 unsigned int attempt = CPUINIT_IPI_ATTEMPTS;
 errno_t error;
 for (;;) {
  apic_write(APIC_ESR,0);
  apic_write(APIC_ICR2,icr2_word);
  apic_write(APIC_ICR,icr_word);
  assert(TASK_ISSAFE());

  /* Wait for the IPC command to be processed. */
  i8253_delay10ms();

  /* Start polling the busy-bit. */
  while (apic_read(APIC_ICR) & APIC_ICR_BUSY)
         task_yield();

  /* Wait a bit more for the error code to come in. */
  i8253_delay10ms();
  error = apic_read(APIC_ESR) ? -ECOMM : -EOK;
  if (E_ISOK(error) || !--attempt) break;
  syslog(LOG_SCHED|LOG_DEBUG,"[SMP] Resending LAPIC IPC-irc command %.8I32x:%.8I32x #%d: %[errno]\n",
         icr_word,icr2_word,CPUINIT_IPI_ATTEMPTS-attempt,-error);
 }
 if (E_ISERR(error)) {
  syslog(LOG_SCHED|LOG_ERROR,
         "[SMP] Failed to execute LAPIC IPC-irc command %.8I32x:%.8I32x: %[errno]\n",
         icr_word,icr2_word);
 }
 return error;
}


INTDEF byte_t cpu_bootstrap_32[];

/* Active the given CPU and stop writing to `ac_sigonoff'. */
PRIVATE SAFE errno_t KCALL
cpu_do_activate_endwrite(struct cpu *__restrict self) {
 struct idt_pointer *gdt; bool need_ipi;
 uintptr_t start_ip; errno_t error = -EOK;
 assert(sig_writing(&self->c_arch.ac_sigonoff));
 start_ip = REALMODE_SYM(cpu_bootstrap);
 /* Prevent interrupts from screwing with CPU initialization. */
 task_nointr();

 /* Fix the CPU's state to allow it to actually start up. */
 ATOMIC_WRITE(self->c_arch.ac_mode,CPUMODE_STARTUP);

 /* Fill in the target CPU's GDT (Used to allow for self-identification; s.a.: `SEG_CPUSELF') */
 gdt = (struct idt_pointer *)REALMODE_SYM(cpu_bootstrap_gdt);
 gdt->ip_limit = sizeof(gdt_builtin)-1;
 gdt->ip_gdt   = VCPU(self,cpu_gdt).ip_gdt;
 gdt->ip_gdt   = (struct segment *)PDIR_TRANSLATE(&pdir_kernel_v,gdt->ip_gdt);
 syslog(LOG_SCHED|LOG_DEBUG,"[SMP] Booting CPU #%d at %p\n",self->c_id,start_ip);

 /* Set BIOS shutdown code to warm start. */
 outb(0x70,0xf);
 outb(0x71,0xa);

 /* Set the reset vector address. */
 assert(start_ip <= 0xfffff);
#if 1
 writew(LAPIC_JMPADDR(TRAMPOLINE_PHYS_HIGH),(u16)((start_ip >> 16) << 12));
 writew(LAPIC_JMPADDR(TRAMPOLINE_PHYS_LOW),(u16)(start_ip & 0xffff));
#else
 writew(LAPIC_JMPADDR(TRAMPOLINE_PHYS_HIGH),(u16)(start_ip >> 4));
 writew(LAPIC_JMPADDR(TRAMPOLINE_PHYS_LOW),(u16)(start_ip & 0xf));
#endif

 need_ipi = (self->c_arch.ac_lapic_version & 0xf0) != APICVER_82489DX;
  
 if (need_ipi) {
  /* Clear APIC errors. */
  apic_write(APIC_ESR,0);
  apic_read(APIC_ESR);
 }

 /* Turn INIT on. */
 apic_write(APIC_ICR2,SET_APIC_DEST_FIELD(self->c_arch.ac_lapic_id));
 apic_write(APIC_ICR,APIC_INT_LEVELTRIG|APIC_INT_ASSERT|APIC_DM_INIT);

 i8253_delay10ms();
 i8253_delay10ms();

 /* De-assert INIT. */
 apic_write(APIC_ICR2,SET_APIC_DEST_FIELD(self->c_arch.ac_lapic_id));
 apic_write(APIC_ICR,APIC_INT_LEVELTRIG|APIC_DM_INIT);

 if (need_ipi) {
  error = apic_exec_ipc(APIC_DM_STARTUP|((start_ip >> 12) & APIC_VECTOR_MASK),
                        SET_APIC_DEST_FIELD(self->c_arch.ac_lapic_id));
  if (E_ISERR(error)) {
   /* Mark the CPU as offline if we've failed. */
   assert(ATOMIC_READ(self->c_arch.ac_mode) == CPUMODE_STARTUP);
   /* NOTE: The race condition that could arise if startup did actually succeed,
    *       but the CPU was too slow to respond is handled in `cpu_bootstrap_c()' */
   ATOMIC_WRITE(self->c_arch.ac_mode,CPUMODE_OFFLINE);
   sig_endwrite(&self->c_arch.ac_sigonoff);
   goto init_fail;
  }
 }

 /* Unlock the sigon/off and wait for the CPU to respond that it's finished booting. */
 error = sig_timedrecv_endwrite(&self->c_arch.ac_sigonoff,JTIME_INFINITE);

init_fail:
 task_endnointr();
 return error;
}


PUBLIC SAFE errno_t KCALL
cpu_enable_unlocked(struct cpu *__restrict self) {
 pflag_t was; errno_t error = -EOK;
 CHECK_HOST_DOBJ(self);
 assert(TASK_ISSAFE());
 assert(rwlock_writing(&apic_lock));
 was = PREEMPTION_PUSH();
 if (self == THIS_CPU) goto end;
again:
 assert(self != THIS_CPU);
 /* Check if communication with the CPU is even possible. */
 if unlikely(!(self->c_arch.ac_flags&CPUFLAG_LAPIC)) { error = -ECOMM; goto end; }
 /* Test for pending interrupts now, because we can't allow later code to do so */
 error = task_testintr();
 if (E_ISERR(error)) goto end;

 /* This is where the meat begins: Start writing the CPU's sigon/off signal. */
 sig_read(&self->c_arch.ac_sigonoff);
 if (self->c_arch.ac_mode == CPUMODE_ONLINE) {
  sig_endread(&self->c_arch.ac_sigonoff);
  goto end;
 }
 sig_upgrade(&self->c_arch.ac_sigonoff);

 switch (__builtin_expect(self->c_arch.ac_mode,CPUMODE_OFFLINE)) {

 case CPUMODE_OFFLINE:
  /* Most common case: It's up to us to activate the CPU. */
  error = cpu_do_activate_endwrite(self);
  break;

 {
  bool shutdown;
 case CPUMODE_SHUTDOWN:
 case CPUMODE_SHUTDOWN_NOMIGRATE:
  syslog(LOG_SCHED|LOG_WARN,
         "[SMP] Re-starting CPU #%d that is still shutting down (Race condition?)\n",
         self->c_id);
 case CPUMODE_STARTUP:
  shutdown = self->c_arch.ac_mode != CPUMODE_STARTUP;
  /* Someone else has already booted this cpu.
   * Join their wait of a response. */
  error = sig_timedrecv_endwrite(&self->c_arch.ac_sigonoff,JTIME_INFINITE);
  if (E_ISERR(error) || !shutdown) goto end;
  goto again;
 }

 default:
  /* The CPU was already activated. */
  assert(self->c_arch.ac_mode == CPUMODE_ONLINE);
  sig_endwrite(&self->c_arch.ac_sigonoff);
  break;
 }
end:
 PREEMPTION_POP(was);
 return error;
}


PUBLIC SAFE errno_t KCALL
cpu_disable_unlocked(struct cpu *__restrict self, int mode) {
 pflag_t was; errno_t error = -EOK;
 CHECK_HOST_DOBJ(self);
 assert(TASK_ISSAFE());
 assert(rwlock_writing(&apic_lock));
 /* Disable preemption to prevent our own CPU from changing. */
 was = PREEMPTION_PUSH();

 if (self == THIS_CPU) {
  if (!(mode&CPU_DISABLE_NOMIGRATE)) {
   /* TODO: Shut down your own CPU. */
   error = -EPERM;
   goto end;
  }
  __cpu_shutdown_now();
  goto end;
 }
again:
 assert(self != THIS_CPU);

 /* Check if communication with the CPU is even possible. */
 if unlikely(!(self->c_arch.ac_flags&CPUFLAG_LAPIC)) { error = -ECOMM; goto end; }

 /* Test for pending interrupts now, because we can't allow later code to do so */
 error = task_testintr();
 if (E_ISERR(error)) goto end;

 /* This is where the meat begins: Start writing the CPU's sigon/off signal. */
 sig_read(&self->c_arch.ac_sigonoff);
 if (self->c_arch.ac_mode == CPUMODE_OFFLINE) {
  sig_endread(&self->c_arch.ac_sigonoff);
  goto end;
 }
 sig_upgrade(&self->c_arch.ac_sigonoff);

 switch (__builtin_expect(self->c_arch.ac_mode,CPUMODE_ONLINE)) {

 case CPUMODE_ONLINE:
  /* Most common case: It's up to us to disable the CPU. */
  ATOMIC_WRITE(self->c_arch.ac_mode,(mode&CPU_DISABLE_NOMIGRATE)
               ? CPUMODE_SHUTDOWN_NOMIGRATE : CPUMODE_SHUTDOWN);

  /* Wait for the CPU to acknowledge the signal. */
  if (!(mode&CPU_DISABLE_ASYNC)) {
   cpusig_t reason = CPUSIG_SHUTDOWN_OK;
   task_nointr();
   HOSTMEMORY_BEGIN {
    /* Wait for the CPU to acknowledge the request. */
    error = sig_vtimedrecv_endwrite(&self->c_arch.ac_sigonoff,
                                    &reason,sizeof(reason),
                                    JTIME_INFINITE);
   }
   HOSTMEMORY_END;
   task_endnointr();
   if (E_ISOK(error)) {
    /* If we've received a signal because the CPU can't
     * migrated its tasks, fail with EPERM. */
    if (reason == CPUSIG_SHUTDOWN_ILLEGAL_AFFINTY) {
     /* Something went wrong here. - We're not supposed to migrate tasks...
      * >> This may be able to happen due to race conditions if another
      *    thread is attempting to shut down the same CPU? */
     if unlikely(mode&CPU_DISABLE_NOMIGRATE) goto again;
     error = -EPERM;
    }
    goto end;
   }
   assert(error == -ETIMEDOUT);
   /* We've timed out while waiting for the acknowledgement.
    * >> Check to see what really happened... */
   sig_write(&self->c_arch.ac_sigonoff);
   switch (self->c_arch.ac_mode) {

   case CPUMODE_OFFLINE:
   case CPUMODE_STARTUP:
    /* So it did still work! */
    error = -EOK;
    break;

   case CPUMODE_ONLINE:
    /* Really shouldn't happen without a signal being broadcast... (Communication/migration error) */
    error = (mode&CPU_DISABLE_NOMIGRATE) ? -ECOMM : -EPERM;
    break;

   default:
    assert(self->c_arch.ac_mode == CPUMODE_SHUTDOWN ||
           self->c_arch.ac_mode == CPUMODE_SHUTDOWN_NOMIGRATE);
    /* *sigh* - Lets change back what we did... */
    ATOMIC_WRITE(self->c_arch.ac_mode,CPUMODE_ONLINE);
    break;
   }
   sig_endwrite(&self->c_arch.ac_sigonoff);
  }
  break;

 {
  bool startup;
 case CPUMODE_STARTUP:
  syslog(LOG_SCHED|LOG_WARN,
         "[SMP] Shutting down CPU #%d that is still starting up (Race condition?)\n",
         self->c_id);
 case CPUMODE_SHUTDOWN:
 case CPUMODE_SHUTDOWN_NOMIGRATE:
  startup = self->c_arch.ac_mode == CPUMODE_STARTUP;
  /* Someone else has already booted this cpu.
   * Join their wait of a response. */
  error = sig_timedrecv_endwrite(&self->c_arch.ac_sigonoff,JTIME_INFINITE);
  if (E_ISERR(error) || !startup) goto end;
  goto again;
 }

 default:
  /* The CPU was already shut down. */
  assert(self->c_arch.ac_mode == CPUMODE_ONLINE);
  sig_endwrite(&self->c_arch.ac_sigonoff);
  break;
 }
end:
 PREEMPTION_POP(was);
 return error;
}


INTDEF void ASMCALL lapic_spurious_irq_handler(void);
GLOBAL_ASM(
L(.section .text.cold                                   )
L(DEFINE_PRIVATE(lapic_spurious_irq_handler)               )
L(lapic_spurious_irq_handler:                           )
L(    /* Atomically increment the sporadic IRQ counter */)
L(    ASM_LOCK incl ASM_CPU(CPU_OFFSETOF_ARCH+ \
                            ARCHCPU_OFFSETOF_SPURIOUS_IRQ))
L(    pushx %xdi                                        )
L(    movx  apic_base, %xdi                             )
L(    movl  $(APIC_EOI_ACK), APIC_EOI(%xdi)             )
L(    popx  %xdi                                        )
L(    ASM_IRET                                          )
L(SYM_END(lapic_spurious_irq_handler)                   )
L(.previous                                             )
);

/* Initialize a new virtual CPU controller.
 * NOTE: The caller has already initialized:
 *        - c_id
 *        - c_arch.ac_flags
 *        - c_arch.ac_lapic_id
 *        - c_arch.ac_lapic_version
 *        - c_arch.ac_cpusig
 *        - c_arch.ac_features
 * NOTE: The caller will initialized later:
 *        - c_idle.t_mman
 *        - c_idle.t_mman_tasks
 */
INTERN ATTR_FREETEXT errno_t KCALL
smp_init_cpu(struct cpu *__restrict vcpu) {
 CHECK_HOST_DOBJ(vcpu);
 memset(&vcpu->c_idle,0,sizeof(struct task));
 vcpu->c_idle.t_cpu                       = vcpu;
 vcpu->c_idle.t_sched.sd_running.re_prev  = &vcpu->c_idle;
 vcpu->c_idle.t_sched.sd_running.re_next  = &vcpu->c_idle;
#ifdef CONFIG_DEBUG
 vcpu->c_idle.t_refcnt                    = 1;
#else
 vcpu->c_idle.t_refcnt                    = 0x80000001;
#endif
 vcpu->c_idle.t_weakcnt                   = 1;
 vcpu->c_idle.t_flags                     = TASKFLAG_NOTALEADER|TASKFLAG_NOSIGNALS;
 ATOMIC_WRITE(vcpu->c_idle.t_mode,TASKMODE_NOTSTARTED);
 vcpu->c_idle.t_prioscore                 =
 vcpu->c_idle.t_priority                  = ATOMIC_READ(__bootcpu.c_idle.t_priority);
 vcpu->c_idle.t_signals.ts_first.tss_self = &__bootcpu.c_idle;
 vcpu->c_idle.t_critical                  = 1;
 vcpu->c_idle.t_nointr                    = 1;
 vcpu->c_idle.t_addrlimit                 = USER_END;
 atomic_rwlock_cinit(&vcpu->c_idle.t_pid.tp_parlock);
 atomic_rwlock_cinit(&vcpu->c_idle.t_pid.tp_leadlock);
 atomic_rwlock_cinit(&vcpu->c_idle.t_pid.tp_childlock);
 atomic_rwlock_cinit(&vcpu->c_idle.t_pid.tp_grouplock);
 memset(&vcpu->c_idle.t_pid.tp_ids,0,sizeof(vcpu->c_idle.t_pid.tp_ids));
 vcpu->c_idle.t_pid.tp_parent             = &vcpu->c_idle;
 vcpu->c_idle.t_pid.tp_leader             = &vcpu->c_idle;
 vcpu->c_idle.t_pid.tp_children           = &vcpu->c_idle;
 vcpu->c_idle.t_pid.tp_group              = &vcpu->c_idle;
 vcpu->c_idle.t_pid.tp_siblings.le_pself  = &vcpu->c_idle.t_pid.tp_children;
 vcpu->c_idle.t_pid.tp_grplink.le_pself   = &vcpu->c_idle.t_pid.tp_group;
 vcpu->c_idle.t_fdman = &fdman_kernel;
 FDMAN_INCREF(&fdman_kernel);
 vcpu->c_idle.t_sighand = &sighand_kernel;
 SIGHAND_INCREF(&sighand_kernel);
 sig_cinit(&vcpu->c_idle.t_event);
 sigpending_cinit(&vcpu->c_idle.t_sigpend);
 memset(&vcpu->c_idle.t_sigblock,0xff,sizeof(sigset_t));
 vcpu->c_idle.t_sigshare = &sigshare_kernel;
 SIGSHARE_INCREF(&sigshare_kernel);
#ifndef CONFIG_NO_LDT
 vcpu->c_idle.t_arch.at_ldt_gdt = SEG(SEG_KERNEL_LDT);
#endif /* !CONFIG_NO_LDT */
#ifndef CONFIG_NO_JOBS
 memset(&vcpu->c_work,0,sizeof(struct task));
 vcpu->c_work.t_cpu = vcpu;
 vcpu->c_work.t_sched.sd_suspended.le_pself = &vcpu->c_suspended;
#ifdef CONFIG_DEBUG
 vcpu->c_work.t_refcnt                    = 1;
#else
 vcpu->c_work.t_refcnt                    = 0x80000001;
#endif
 vcpu->c_work.t_weakcnt                   = 1;
 vcpu->c_work.t_flags                     = TASKFLAG_NOTALEADER|TASKFLAG_NOSIGNALS;
 vcpu->c_work.t_mode                      = TASKMODE_SUSPENDED;
 vcpu->c_work.t_prioscore                 =
 vcpu->c_work.t_priority                  = ATOMIC_READ(__bootcpu.c_work.t_priority);
 vcpu->c_work.t_signals.ts_first.tss_self = &__bootcpu.c_work;
 vcpu->c_work.t_critical                  = 1;
 vcpu->c_work.t_nointr                    = 1;
 vcpu->c_work.t_addrlimit                 = USER_END;
 atomic_rwlock_cinit(&vcpu->c_work.t_pid.tp_parlock);
 atomic_rwlock_cinit(&vcpu->c_work.t_pid.tp_leadlock);
 atomic_rwlock_cinit(&vcpu->c_work.t_pid.tp_childlock);
 atomic_rwlock_cinit(&vcpu->c_work.t_pid.tp_grouplock);
 memset(&vcpu->c_work.t_pid.tp_ids,0,sizeof(vcpu->c_work.t_pid.tp_ids));
 vcpu->c_work.t_pid.tp_parent             = &vcpu->c_work;
 vcpu->c_work.t_pid.tp_leader             = &vcpu->c_work;
 vcpu->c_work.t_pid.tp_children           = &vcpu->c_work;
 vcpu->c_work.t_pid.tp_group              = &vcpu->c_work;
 vcpu->c_work.t_pid.tp_siblings.le_pself  = &vcpu->c_work.t_pid.tp_children;
 vcpu->c_work.t_pid.tp_grplink.le_pself   = &vcpu->c_work.t_pid.tp_group;
 vcpu->c_work.t_fdman = &fdman_kernel;
 FDMAN_INCREF(&fdman_kernel);
 vcpu->c_work.t_sighand = &sighand_kernel;
 SIGHAND_INCREF(&sighand_kernel);
 sig_cinit(&vcpu->c_work.t_event);
 sigpending_cinit(&vcpu->c_work.t_sigpend);
 memset(&vcpu->c_work.t_sigblock,0xff,sizeof(sigset_t));
 vcpu->c_work.t_sigshare = &sigshare_kernel;
 SIGSHARE_INCREF(&sigshare_kernel);
#ifndef CONFIG_NO_LDT
 vcpu->c_work.t_arch.at_ldt_gdt = SEG(SEG_KERNEL_LDT);
#endif /* !CONFIG_NO_LDT */
#endif /* !CONFIG_NO_JOBS */

 /* Setup the affinity of the IDLE task. */
 atomic_rwlock_init(&vcpu->c_idle.t_affinity_lock);
 CPU_ZERO(&vcpu->c_idle.t_affinity);
 CPU_SET(vcpu->c_id,&vcpu->c_idle.t_affinity);

#ifndef CONFIG_NO_LDT
 ldt_write(&ldt_kernel);
 LIST_INSERT(ldt_kernel.l_tasks,&vcpu->c_idle,t_arch.at_ldt_tasks);
#ifndef CONFIG_NO_JOBS
 LIST_INSERT(ldt_kernel.l_tasks,&vcpu->c_work,t_arch.at_ldt_tasks);
#endif /* !CONFIG_NO_JOBS */
 ldt_endwrite(&ldt_kernel);
#endif /* !CONFIG_NO_LDT */

 /* NOTE: `task_set_id()' only fails if there are no more IDs to hand out.
  *        But this function should only be called during early boot when
  *        it should be impossible for all PIDs to already be used up,
  *        meaning we are safe to assert that this always succeeds. */
 asserte(E_ISOK(task_set_id(&vcpu->c_idle,&pid_global)));
 asserte(E_ISOK(task_set_id(&vcpu->c_idle,&pid_init)));
#ifndef CONFIG_NO_JOBS
 asserte(E_ISOK(task_set_id(&vcpu->c_work,&pid_global)));
 asserte(E_ISOK(task_set_id(&vcpu->c_work,&pid_init)));
#endif /* !CONFIG_NO_JOBS */

 vcpu->c_self      = vcpu;
 vcpu->c_running   = &vcpu->c_idle;
 vcpu->c_sleeping  = NULL;
 vcpu->c_idling    = NULL;
#ifndef CONFIG_NO_JOBS
 vcpu->c_suspended = &vcpu->c_work;
#else
 vcpu->c_suspended = NULL;
#endif
 vcpu->c_prio_min  = vcpu->c_idle.t_priority;
 vcpu->c_prio_max  = vcpu->c_idle.t_priority;
 atomic_rwlock_init(&vcpu->c_lock);
 COMPILER_WRITE_BARRIER();
 /* The CPU's IDLE task is not fully initialized.
  * We can therefor mark it as running. */
 ATOMIC_WRITE(vcpu->c_idle.t_mode,TASKMODE_RUNNING);

 /* Setup initial TSS information. */
 sig_init(&vcpu->c_arch.ac_sigonoff);
 vcpu->c_arch.ac_mode           = CPUMODE_OFFLINE;
 vcpu->c_arch.ac_tss.xsp0       = (uintptr_t)vcpu->c_idle.t_hstack.hs_end;
#ifndef __x86_64__
 vcpu->c_arch.ac_tss.ss0        = __KERNEL_DS;
#endif
 vcpu->c_arch.ac_tss.iomap_base = sizeof(struct tss);

 /* Finally, initialize per-cpu memory. */
 MEMCPYX((void *)((uintptr_t)vcpu+ALIGNED_CPUSIZE),
         (void *)PERCPU_TEMPLATE,(size_t)PERCPU_DAT_XWORDS);
 MEMSETX((void *)((uintptr_t)vcpu+ALIGNED_CPUSIZE+(size_t)__percpu_datsize),
          0x00000000,(size_t)PERCPU_BSS_XWORDS);

 /* Encode TSS & CPU-SELF segments in the new CPU's address space.
  * NOTE: This is what the CPU will later use to identify itself! */
 { struct idt_pointer *gdt = &VCPU(vcpu,cpu_gdt);
   gdt->ip_gdt = (struct segment *)memdup(gdt_builtin,sizeof(gdt_builtin));
   if unlikely(!gdt->ip_gdt) return -ENOMEM;
   gdt->ip_gdt[SEG_CPUTSS].ul32  = __SEG_ENCODELO((uintptr_t)&vcpu->c_arch.ac_tss,sizeof(struct tss),SEG_TSS);
   gdt->ip_gdt[SEG_CPUTSS].uh32  = __SEG_ENCODEHI((uintptr_t)&vcpu->c_arch.ac_tss,sizeof(struct tss),SEG_TSS);
#if 1
   gdt->ip_gdt[SEG_CPUSELF].ul32 = __SEG_ENCODELO((uintptr_t)vcpu,sizeof(struct cpu),SEG_DATA_PL0);
   gdt->ip_gdt[SEG_CPUSELF].uh32 = __SEG_ENCODEHI((uintptr_t)vcpu,sizeof(struct cpu),SEG_DATA_PL0);
#else
   gdt->ip_gdt[SEG_CPUSELF].ul32 = __SEG_ENCODELO((uintptr_t)vcpu,sizeof(struct cpu),SEG_DATA_PL3);
   gdt->ip_gdt[SEG_CPUSELF].uh32 = __SEG_ENCODEHI((uintptr_t)vcpu,sizeof(struct cpu),SEG_DATA_PL3);
#endif
 }

 /* Initialize the CPU's Interrupt Descriptor Table (IDT) */
 cpu_interrupt_initialize(vcpu);

 return -EOK;
}


PUBLIC SAFE errno_t KCALL
cpu_sendipc_unlocked(struct cpu *__restrict self, irq_t intno) {
 errno_t error; pflag_t was;
 CHECK_HOST_DOBJ(self);
 assert(rwlock_writing(&apic_lock));
 was = PREEMPTION_PUSH();
 if (self == THIS_CPU) {
  /* Hacky way of executing `int $intno; ret' */
  char code[3];
  code[0] = 0xcd; /* `int ...' */
  code[1] = intno;
  code[2] = 0xc3; /* 'ret' */
  COMPILER_WRITE_BARRIER();
  __asm__ __volatile__("call *%0\n" : : "m" (*code));
  error = -EOK;
 } else {
  error = apic_exec_ipc(intno,SET_APIC_DEST_FIELD(self->c_arch.ac_lapic_id));
 }
 PREEMPTION_POP(was);
 return error;
}


INTDEF void INTCALL rpc_interrupt_handler(void);
PRIVATE struct interrupt lapic_spurious_interrupt = {
    .i_intno = INTNO_LAPIC_SPURIOUS,
    .i_mode  = INTMODE_HW,
    .i_type  = INTTYPE_ASM,
    .i_prio  = INTPRIO_MAX,
    .i_flags = INTFLAG_PRIMARY,
    .i_proto = {
        .p_asm = &lapic_spurious_irq_handler,
    },
    .i_owner = THIS_INSTANCE,
};
PRIVATE struct interrupt lapic_rpc_interrupt = {
    .i_intno = INTNO_LAPIC_RPC,
    .i_mode  = INTMODE_HOST,
    .i_type  = INTTYPE_FAST|INTTYPE_NOSHARE,
    .i_prio  = INTPRIO_MAX,
    .i_flags = INTFLAG_PRIMARY,
    .i_proto = {
        .p_noshare_fast = &rpc_interrupt_handler,
    },
    .i_owner = THIS_INSTANCE,
};

INTERN void KCALL smp_initialize_lapic(void) {
 if (!APIC_SUPPORTED()) return;

 /* Define the sporadic interrupt handler used by LAPIC. */
 asserte(E_ISOK(int_addall(&lapic_spurious_interrupt)));
 asserte(E_ISOK(int_addall(&lapic_rpc_interrupt)));

 apic_write(APIC_SPIV,INTNO_LAPIC_SPURIOUS|
            APIC_SPIV_DIRECTED_EOI|
            APIC_SPIV_APIC_ENABLED);
}


DECL_END
#endif /* CONFIG_SMP */

#endif /* !GUARD_KERNEL_ARCH_SMP_C */
