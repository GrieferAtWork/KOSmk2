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
#ifndef GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_INTERRUPTS_H
#define GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_INTERRUPTS_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <format-printer.h>

DECL_BEGIN

#define ARM4_XRQ_RESET   0x00
#define ARM4_XRQ_UNDEF   0x01
#define ARM4_XRQ_SWINT   0x02
#define ARM4_XRQ_ABRTP   0x03
#define ARM4_XRQ_ABRTD   0x04
#define ARM4_XRQ_RESV1   0x05
#define ARM4_XRQ_IRQ     0x06
#define ARM4_XRQ_FIQ     0x07

/* Interrupt vector table indices.                        MODE/BANK  AFI */
#define INTNO_RESET   0 /* CPU reset.                     Supervisor 111 */
#define INTNO_UNDEF   1 /* Undefined instruction handler. Undefined  --1 */
#define INTNO_SWINT   2 /* Software interrupt.            Supervisor --1 */
#define INTNO_ABRTP   3 /* Abort (prefetch).              Abort      1-1 */
#define INTNO_ABRTD   4 /* Abort (data).                  Abort      1-1 */
//      INTNO_RESRV   5 /* ------------------------------ ---------- ??? */
#define INTNO_IRQ     6 /* Interrupt handler.             IRQ        1-1 */
#define INTNO_FIQ     7 /* Fast interrupt handler.        FIQ        111 */
#define INTNO_COUNT   8 /* Amount of interrupt vectors. */


/* Physical address of the interrupt vector table.
 * Don't be confused if this is `NULL': The vector table actually starts out there. */
#define ARM_INTERRUPT_TABLE   (*arm_interrupt_table)
DATDEF PHYS u32 (*arm_interrupt_table)[INTNO_COUNT];


#define INTMODE_HOST  0
#define INTMODE_USER  0
typedef u8 intmode_t;

/* Return a pointer to the effective interrupt IRREGS-IRET tail,
 * assuming that the calling thread entered kernel-space using
 * a system-call, or interrupt.
 * HINT: The returned structure is compatible with `IRREGS_SYSCALL_GET()' */
#define XCREGS_INTERRUPT_GET()            XCREGS_INTERRUPT_GET_FOR(THIS_TASK)
#define XCREGS_INTERRUPT_GET_FOR(task) (((struct xcregs *)(task)->t_hstack.hs_end)-1)

struct cpu;
struct task;
struct cpustate;

#define PANIC_GPREGS    0x0001 /* Dump general purpose registers. */
#define PANIC_TRACEBACK 0x0020 /* Dump a trackback. */
#define PANIC_STACK     0x0040 /* Dump the immediate contents of the stack. */
#define PANIC_MMAN      0x0200 /* Dump effective memory manager mappings. */
#define PANIC_PDIR      0x0400 /* Dump effective page directory mappings. */

/* Default value of `kernel_panic_mask' when
 * `panic_mask' isn't passed on the commandline. */
#define PANIC_DEFAULT_MASK (PANIC_GPREGS|PANIC_TRACEBACK|PANIC_STACK)

/* Process kernel-panic by dumping information
 * described by `panic_mask' to `printer'
 * HINT: Pass `kernel_panic_mask' for `panic_mask' to dump an information mask
 *       that can be configured via the `panic_mask' boot option.
 * @param: panic_mask: Set of `PANIC_*' */
FUNDEF ssize_t KCALL
kernel_panic_process(struct cpu *__restrict this_cpu,
                     struct task *__restrict this_task,
                     struct cpustate *__restrict state, u32 panic_mask,
                     pformatprinter printer, void *closure);
DATDEF u32 kernel_panic_mask;

DECL_END

#endif /* !GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_INTERRUPTS_H */
