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
#ifndef GUARD_KERNEL_SCHED_RPC_C
#define GUARD_KERNEL_SCHED_RPC_C 1

#include <errno.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/arch/apic.h>
#include <kernel/paging.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <sched/rpc.h>
#include <sched/smp.h>
#include <sync/rwlock.h>

DECL_BEGIN

__asm__(".hidden rpc_irq_handler\n"
        ".global rpc_irq_handler\n");
INTDEF void ASMCALL rpc_handler(void);
INTERN DEFINE_INT_HANDLER(rpc_irq_handler,rpc_handler);

struct rpc_command {
 struct sig c_done; /*< Signal broadcast when the command is done executing. */
 cpu_rpc_t  c_cmd;  /*< [lock(c_done)] RPC command (NOTE: 'CPU_RPC_NOOP' when no command is being executed). */
 void      *c_arg;  /*< [lock(c_done)] RPC command argument. */
};

PRIVATE CPU_DATA struct rpc_command rpc = {
    .c_done = SIG_INIT,
    .c_cmd  = CPU_RPC_NOOP,
    .c_arg  = NULL,
};


#define RPC CPU(rpc)
INTERN ssize_t ASMCALL rpc_exec(void) {
 ssize_t result = 0;
 switch (RPC.c_cmd) {
#define ARG(T) ((T)RPC.c_arg)

 case CPU_RPC_CALLBACK:
  /* Simply execute the given callback. */
  result = (*ARG(struct rpc_callback *)->c_callback)
            (ARG(struct rpc_callback *)->c_closure);
  break;

 case CPU_RPC_SHUTDOWN:
 case CPU_RPC_SHUTDOWN_NOMIGRATE:
  /* TODO */
  result = -EBUSY;
  break;

 case CPU_RPC_IDT_SET:
  /* Install the given ISR descriptor. */
  irq_set(&ARG(struct rpc_update_idt *)->ui_isr,
          &ARG(struct rpc_update_idt *)->ui_isr,
           IRQ_SET_RELOAD|IRQ_SET_INHERIT);
  break;

 case CPU_RPC_IDT_DEL:
  /* Install the given ISR descriptor. */
  irq_del(*ARG(irq_t *),true);
  break;

 case CPU_RPC_TLB_SHOOTDOWN:
  /* Flash all page-directory descriptors within the given address range. */
  pdir_flush(ARG(struct rpc_tlb_shootdown *)->ts_begin,
             ARG(struct rpc_tlb_shootdown *)->ts_size);
  break;

 default: break;
 }
 return result;
}





INTERN void ASMCALL rpc_handler(void) {
 ssize_t return_value;
 /* Acknowledge the LAPIC IPC interrupt. */
 apic_write(APIC_EOI,APIC_EOI_ACK);

 return_value = rpc_exec();

 /* Broadcast the interrupt service return value. */
 sig_write(&RPC.c_done);
 RPC.c_cmd = CPU_RPC_NOOP;
 sig_vbroadcast_unlocked(&RPC.c_done,&return_value,sizeof(return_value));
 sig_endwrite(&RPC.c_done);
}
#undef RPC


#define RPC VCPU(self,rpc)
PUBLIC SAFE ssize_t KCALL
cpu_rpc_send(struct cpu *__restrict self,
             cpu_rpc_t command, void *arg) {
 pflag_t was; ssize_t return_value,result;
 for (;;) {
  sig_write(&RPC.c_done);
  if (RPC.c_cmd == CPU_RPC_NOOP) break;
  result = sig_recv_endwrite(&RPC.c_done);
  if (E_ISERR(result)) return result;
 }
 /* Disable preemption to prevent the current CPU from changing. */
 was = PREEMPTION_PUSH();
 if (self == THIS_CPU) {
  /* Somewhat simpler case: Execute on the local CPU. */
  RPC.c_cmd = command;
  RPC.c_arg = arg;
  result = rpc_exec();
  /* Broadcast the RPC return value to anything else listening. */
  RPC.c_cmd = CPU_RPC_NOOP;
  sig_vbroadcast_unlocked(&RPC.c_done,&result,sizeof(result));
  goto end;
 }

 /* Lock LAPIC. */
 result = rwlock_write(&apic_lock);
 if (E_ISERR(result)) goto end;
 /* Fill in RPC command data. */
 RPC.c_cmd = command;
 RPC.c_arg = arg;
 /* Actually send the IPC interrupt. */
 result = cpu_sendipc_unlocked(self,IRQ_LAPIC_RPC);
 rwlock_endwrite(&apic_lock);
 if (E_ISERR(result)) goto end;
 HOSTMEMORY_BEGIN {
  result = sig_vrecv_endwrite(&RPC.c_done,&return_value,sizeof(return_value));
 }
 HOSTMEMORY_END;
 if (E_ISOK(result)) result = return_value;
 goto end2;
end:  sig_endwrite(&RPC.c_done);
end2: PREEMPTION_POP(was);
 return result;
}
#undef RPC


DECL_END

#endif /* !GUARD_KERNEL_SCHED_RPC_C */
