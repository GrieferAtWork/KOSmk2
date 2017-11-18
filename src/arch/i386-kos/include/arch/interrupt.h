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
#ifndef GUARD_INCLUDE_ARCH_INTERRUPT_H
#define GUARD_INCLUDE_ARCH_INTERRUPT_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <format-printer.h>

DECL_BEGIN


/* Amount of `struct idtentry' that make up a full IDT table. */
#define IDT_TABLESIZE 256

#define IDTENTRY_OFFSETOF_OFF1  0
#define IDTENTRY_OFFSETOF_SEL   2
#define IDTENTRY_OFFSETOF_ZERO  4
#define IDTENTRY_OFFSETOF_FLAGS 5
#define IDTENTRY_OFFSETOF_OFF2  6
#ifdef __x86_64__
#define IDTENTRY_OFFSETOF_OFF3  8
#define IDTENTRY_SIZE           16
#else
#define IDTENTRY_SIZE           8
#endif

struct PACKED idtentry {
    u16 ie_off1;  /*< Lower 16 bits of an `irq_handler' pointer. */
    u16 ie_sel;   /*< Kernel code segment (always `__KERNEL_CS') */
#ifdef __x86_64__
    u8  ie_ist;   /*< Nits 0..2 hold Interrupt Stack Table offset, rest of bits zero. */
#else
    u8  ie_zero;  /*< Always ZERO(0). */
#endif
    u8  ie_flags; /*< Set of `IDTFLAG_*|IDTTYPE_*' */
    u16 ie_off2;  /*< Upper 16 bits of an `irq_handler' pointer. */
#ifdef __x86_64__
    u32 ie_off3;  /*< Bits 32..63 of the vector offset. */
    u32 ie_unused;/* Unused ata. */
#endif
};

#define IDTFLAG_PRESENT                 0x80 /*< Set to 0 for unused interrupts. */
/* Descriptor Privilege LevelGate call protection.
 * Specifies which privilege Level the calling Descriptor minimum should have.
 * So hardware and CPU interrupts can be protected from being called out of userspace. */
#define IDTFLAG_DPL(n)          (((n)&3)<<5) /*< Mask: 0x60 */
#define IDTFLAG_DPL_MASK                0x60
#define IDTFLAG_STORAGE_SEGMENT         0x10 /*< Set to 0 for interrupt gates. */
#define IDTTYPE_MASK                    0x0f
#define IDTTYPE_80386_32_TASK_GATE      0x05
#define IDTTYPE_80286_16_INTERRUPT_GATE 0x06
#define IDTTYPE_80286_16_TRAP_GATE      0x07
#define IDTTYPE_80386_32_INTERRUPT_GATE 0x0e
#ifndef __x86_64__
#define IDTTYPE_80386_32_TRAP_GATE      0x0f
#endif
/* NOTE: Difference trap/interrupt:
 *     - trap:      Do not modify `XFLAGS.IF'
 *       WARNING:   This type of interrupt can't really be used
 *                  in KOS because it interferes with the operation
 *                  of the `swapgs' instruction.
 *               >> If we were to use it and another interrupt occurrs
 *                  before `swapgs' was execute in the event that the
 *                  CPU was in user-space before, then the second handler
 *                  would think that GS had already been adjusted to
 *                  contain the kernel's GS base address.
 *                  But since that isn't the case, we're left with only two choices:
 *                    #1: Never use traps and always use interrupts instead
 *                    #2: 
 *     - interrupt: Disable further interrupts by clearing `XFLAGS.IF'
 */


/* Default interrupt descriptor flags for host-private
 * callbacks and callbacks available from user-space. */
#define INTMODE_HW           (IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE|IDTFLAG_DPL(0)) /* Interrupt that is triggered by hardware. */
#define INTMODE_HOST         (IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE|IDTFLAG_DPL(0)) /* Regular interrupt handler only callable by hardware or from within the kernel. */
#define INTMODE_USER         (IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE|IDTFLAG_DPL(3)) /* Interrupt handler accessible by anyone (including userspace). */
#define INTMODE_EXCEPT        INTMODE_HOST /* Interrupt that is triggered by exceptions. */
#define INTMODE_EXCEPT_NOINT  INTMODE_HW   /* Same as to `INTMODE_EXCEPT', but disable further interrupts before executing the handler. */
typedef u8 intmode_t;

/* Returns the suggested default interrupt mode, given the interrupt's number.
 * NOTE: This functions grants user-access to the following interrupts:
 *   - INTNO_EXC_DE *< Divide-by-zero.
 *   - INTNO_EXC_BP *< Breakpoint.
 *   - INTNO_EXC_OF *< Overflow.
 *   - INTNO_EXC_BR *< Bound Range Exceeded. */
#define INTMODE_DEFAULT(intno) (((intno) <= 5 && (intno) != INTNO_EXC_DB && (intno) != INTNO_EXC_NMI) ? INTMODE_USER : INTMODE_HOST)
#define __INTMODE_USER_MASK   0


/* Interrupt numbers. */
#define INTNO_PIC1_BASE 0x20
#define INTNO_PIC2_BASE 0x28

/* Return the IRQ numbers of hardware interrupt
 * lines wired either to the master, or slave PIC.
 * @param: i :  The line number (0..7)
 * @return: * : The IRQ number. */
#define INTNO_PIC1(i) (INTNO_PIC1_BASE+(i))
#define INTNO_PIC2(i) (INTNO_PIC2_BASE+(i))

/* Check the type of interrupt, given its IRQ number. */
#define INTNO_ISEXC(i) ((i) < 0x20)                /*< Exception. */
#define INTNO_ISPIC(i) ((i) >= 0x20 && (i) < 0x30) /*< Hardware, PIC interrupt. */
#define INTNO_ISUSR(i) ((i) >= 0x30)               /*< Custom interrupts. */

/* X86 exception IRQ numbers. */
#define INTNO_EXC(x)            x
#define INTNO_EXC_DE  INTNO_EXC(0)  /*< Divide-by-zero. */
#define INTNO_EXC_DB  INTNO_EXC(1)  /*< Debug. */
#define INTNO_EXC_NMI INTNO_EXC(2)  /*< Non-maskable Interrupt. */
#define INTNO_EXC_BP  INTNO_EXC(3)  /*< Breakpoint. */
#define INTNO_EXC_OF  INTNO_EXC(4)  /*< Overflow. */
#define INTNO_EXC_BR  INTNO_EXC(5)  /*< Bound Range Exceeded. */
#define INTNO_EXC_UD  INTNO_EXC(6)  /*< Invalid Opcode. */
#define INTNO_EXC_NM  INTNO_EXC(7)  /*< Device Not Available. */
#define INTNO_EXC_DF  INTNO_EXC(8)  /*< Double Fault. */
#define INTNO_EXC_TS  INTNO_EXC(10) /*< Invalid TSS. */
#define INTNO_EXC_NP  INTNO_EXC(11) /*< Segment Not Present. */
#define INTNO_EXC_SS  INTNO_EXC(12) /*< Stack-Segment Fault. */
#define INTNO_EXC_GP  INTNO_EXC(13) /*< General Protection Fault. */
#define INTNO_EXC_PF  INTNO_EXC(14) /*< Page Fault. */
#define INTNO_EXC_MF  INTNO_EXC(16) /*< x87 Floating-Point Exception. */
#define INTNO_EXC_AC  INTNO_EXC(17) /*< Alignment Check. */
#define INTNO_EXC_MC  INTNO_EXC(18) /*< Machine Check. */
#define INTNO_EXC_XM  INTNO_EXC(19) /*< SIMD Floating-Point Exception. */
#define INTNO_EXC_XF  INTNO_EXC_XM  /*< SIMD Floating-Point Exception. */
#define INTNO_EXC_VE  INTNO_EXC(20) /*< Virtualization Exception. */
#define INTNO_EXC_SX  INTNO_EXC(30) /*< Security Exception. */

#define EXC_DIVIDE_BY_ZERO   INTNO_EXC_DE
#define EXC_BREAKPOINT       INTNO_EXC_BP
#define EXC_INTEGER_OVERFLOW INTNO_EXC_OF
#define EXC_OUT_OF_BOUNDS    INTNO_EXC_BR
#define EXC_INVALID_OPCODE   INTNO_EXC_UD
#define EXC_DOUBLE_FAULT     INTNO_EXC_DF
#define EXC_PROTECTION_FAULT INTNO_EXC_GP
#define EXC_PAGE_FAULT       INTNO_EXC_PF
#define EXC_FPU_EXCEPTION    INTNO_EXC_MF
#define EXC_ALIGNMENT_ERROR  INTNO_EXC_AC

/* Additional, KOS-specific IRQ vector numbers. */
#define INTNO_SYSCALL        0x80 /*< Interrupt vector used for linux-compatible & kos-specific system calls. */
#ifdef CONFIG_SMP
#define INTNO_LAPIC_RPC      0xfe /*< Interrupt slot for inter-processor communication. */
#define INTNO_LAPIC_SPURIOUS 0xff /*< Spurious LAPIC IRQ handler. */
#endif

/* Exception name database. */
struct interrupt_name {
 char in_nmemonic[4];     /*< Nmemonical name of the exception. */
 char in_description[28]; /*< Human-readable descriptor of the interrupt. */
};
DATDEF struct interrupt_name const exception_names[32];


/* Return a pointer to the effective interrupt IRREGS-IRET tail,
 * assuming that the calling thread entered kernel-space using
 * a system-call, or interrupt.
 * HINT: The returned structure is compatible with `IRREGS_SYSCALL_GET()' */
#define IRREGS_INTERRUPT_GET()         (((struct irregs *)THIS_CPU->c_arch.ac_tss.xsp0)-1)
#define IRREGS_INTERRUPT_GET_FOR(task) (((struct irregs *)(task)->t_hstack.hs_end)-1)


struct cpu;
struct task;
struct cpustate_ie;

#define PANIC_GPREGS    0x0001 /* Dump general purpose registers. */
#define PANIC_XFLAGS    0x0002 /* Dump xflags. */
#define PANIC_SGREGS    0x0004 /* Dump segment registers. */
#define PANIC_CRREGS    0x0008 /* Dump control registers. */
#define PANIC_DRREGS    0x0010 /* Dump debug registers. */
#define PANIC_TRACEBACK 0x0020 /* Dump a trackback. */
#define PANIC_STACK     0x0040 /* Dump the immediate contents of the stack. */
#define PANIC_TSS       0x0080 /* Dump the non-static portion of the TSS. */
#define PANIC_GDT_LDT   0x0100 /* Dump the entire GDT/LDT of the CPU. */
#define PANIC_MMAN      0x0200 /* Dump effective memory manager mappings. */
#define PANIC_PDIR      0x0400 /* Dump effective page directory mappings. */

/* Default value of `kernel_panic_mask' when
 * `panic_mask' isn't passed on the commandline. */
#define PANIC_DEFAULT_MASK (PANIC_GPREGS|PANIC_XFLAGS| \
                            PANIC_SGREGS|PANIC_CRREGS| \
                            PANIC_TRACEBACK|PANIC_STACK)

/* Process kernel-panic by dumping information
 * described by `panic_mask' to `printer'
 * HINT: Pass `kernel_panic_mask' for `panic_mask' to dump an information mask
 *       that can be configured via the `panic_mask' boot option.
 * @param: panic_mask: Set of `PANIC_*'
 * @param: state:      Either a `struct cpustate_ie *', or a `struct cpustate_i *'.
 */
FUNDEF ssize_t KCALL
kernel_panic_process(struct cpu *__restrict this_cpu,
                     struct task *__restrict this_task,
                     struct cpustate_ie *__restrict state, u32 panic_mask,
                     pformatprinter printer, void *closure);
DATDEF u32 kernel_panic_mask;

DECL_END

#endif /* !GUARD_INCLUDE_ARCH_INTERRUPT_H */
