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
#ifndef GUARD_INCLUDE_KERNEL_RPC_H
#define GUARD_INCLUDE_KERNEL_RPC_H 1

#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/irq.h>

DECL_BEGIN

struct cpu;
typedef u32 cpu_rpc_t;

#define CPU_RPC_FLAG_ASYNC         0x80000000 /*< Execute the RPC asynchronously. */
#define CPU_RPC_MASKMODE           0xff000000
#define CPU_RPC_MASKNAME           0x00ffffff
#define CPU_RPC_NOOP               0x00000000

/* Execute a given user-callback within the specified CPU.
 * NOTES:
 *   - The return value of the callback is later returned by 'cpu_sendrpc_unlocked()'.
 *   - Interrupts on the target CPU are disabled
 *     for the duration of the given callback.
 *   - Upon execution of 'callback', the kernel-stack of some random
 *     task running under `self' will be used for 'ESP', leaving all
 *     other non-segment registers in an undefined state.
 * @return: * : Same as 'c_callback()'. */
#define CPU_RPC_CALLBACK           0x00000001 /*< [ARG(struct rpc_callback)] */

/* Shutdown the given CPU.
 * @return: -EBUSY: [CPU_RPC_SHUTDOWN] The CPU contains tasks that could not be migrated. */
#define CPU_RPC_SHUTDOWN           0x00000002
#define CPU_RPC_SHUTDOWN_NOMIGRATE 0x00000003

/* Exchange the given argument with the previous IDT entry. */
#define CPU_RPC_IDT_SET            0x00000004 /*< [ARG(struct rpc_update_idt)] */
#define CPU_RPC_IDT_DEL            0x00000005 /*< [ARG(irq_t)] */
#define CPU_RPC_TLB_SHOOTDOWN      0x00000006 /*< [ARG(struct rpc_tlb_shootdown)] */


struct rpc_callback { ssize_t (KCALL *c_callback)(void *closure); void *c_closure; };
struct rpc_update_idt { REF isr_t ui_isr; }; /* NOTE: 'ui_isr' contains a reference to the associated instance _BOTH_ on entry & exit. */
struct rpc_tlb_shootdown { void *ts_begin; size_t ts_size; };


/* Execute a RemoteProcedureCall on the given cpu and wait until it is done.
 * NOTE: When the caller _is_ the given CPU, 
 * WARNING: Data pointed to by `arg' is not copied, meaning that
 *          some command cannot safely be executed asynchronously.
 * @param: command:      One of 'CPU_RPC_*', stating the command name.
 * @param: arg:          Argument passed alongside 'command' (usage/meaning depends on 'command')
 * @return: * :          Dependent on 'command'
 * @return: -EINVAL:     The given CPU is offline. (NOTE: Not returned if it was starting up)
 * @return: -ECOMM:      Failed to communicated with the given CPU.
 * @return: E_ISERR(*'): Command execution failed for some reason. */
FUNDEF SAFE ssize_t KCALL cpu_rpc_send(struct cpu *__restrict self, cpu_rpc_t command, void *arg);

/* Similar to 'cpu_rpc_send', but ignores errors and executes the RPC on all running CPUs. */
LOCAL SAFE void KCALL cpu_rpc_broadcast(cpu_rpc_t command, void *arg);



/* Cross-cpu IRQ setter helper function.
 * >> Behaves the same as 'irq_set', but takes an additional cpu-argument `self'. */
LOCAL SAFE bool KCALL irq_vset(struct cpu *__restrict self,
                               isr_t const *__restrict new_handler,
                           REF isr_t *old_handler, int mode);
LOCAL SAFE void KCALL irq_vdel(struct cpu *__restrict self, irq_t num);

DECL_END

#ifndef __INTELLISENSE__
#include <linker/module.h>
#include <sched/smp.h>
DECL_BEGIN

LOCAL SAFE bool KCALL irq_vset(struct cpu *__restrict self,
                               isr_t const *__restrict new_handler,
                           REF isr_t *old_handler, int mode) {
 struct rpc_update_idt arg; bool result;
 arg.ui_isr = *new_handler;
 if (!(mode&IRQ_SET_INHERIT) &&
     !INSTANCE_INCREF(arg.ui_isr.i_owner))
      return false;
 result = E_ISOK(cpu_rpc_send(self,CPU_RPC_IDT_SET,&arg));
 if (old_handler) *old_handler = arg.ui_isr;
 else INSTANCE_DECREF(arg.ui_isr.i_owner);
 return result;
}
LOCAL SAFE void KCALL irq_vdel(struct cpu *__restrict self, irq_t num) {
 cpu_rpc_send(self,CPU_RPC_IDT_DEL,&num);
}

LOCAL SAFE void KCALL cpu_rpc_broadcast(cpu_rpc_t command, void *arg) {
 struct cpu *c;
 FOREACH_CPU(c) cpu_rpc_send(c,command,arg);
}

DECL_END
#endif

#endif /* !GUARD_INCLUDE_KERNEL_RPC_H */
