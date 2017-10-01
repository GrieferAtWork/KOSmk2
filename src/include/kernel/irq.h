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
#ifndef GUARD_INCLUDE_KERNEL_IRQ_H
#define GUARD_INCLUDE_KERNEL_IRQ_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/types.h>
#include <stdbool.h>
#include <kernel/arch/gdt.h>

DECL_BEGIN

struct cpustate;
struct cpustate_irq_c;



/* PIC (Programmable Interrupt Controller) API. */
#define PIC1       0x20 /* IO base address for master PIC */
#define PIC2       0xA0 /* IO base address for slave PIC */
#define PIC1_CMD   PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_CMD   PIC2
#define PIC2_DATA (PIC2+1)

#define PIC_EOI         0x20 /* End-of-interrupt command code */
#define ICW1_ICW4       0x01 /* ICW4 (not) needed */
#define ICW1_SINGLE     0x02 /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08 /* Level triggered (edge) mode */
#define ICW1_INIT       0x10 /* Initialization - required! */
#define ICW4_8086       0x01 /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02 /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08 /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM       0x10 /* Special fully nested (not) */
#define PIC_READ_IRR    0x0a /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR    0x0b /* OCW3 irq service next CMD read */
 
#define IRQ_PIC1_IRR()  (outb(PIC1_CMD,PIC_READ_IRR),inb(PIC2_CMD))
#define IRQ_PIC2_IRR()  (outb(PIC2_CMD,PIC_READ_IRR),inb(PIC2_CMD))
#define IRQ_PIC1_ISR()  (outb(PIC1_CMD,PIC_READ_ISR),inb(PIC2_CMD))
#define IRQ_PIC2_ISR()  (outb(PIC2_CMD,PIC_READ_ISR),inb(PIC2_CMD))

/* Signal EOI (End of interrupt) to the
 * first (master), or second (slave) PIC. */
#define IRQ_PIC_EOI(id) ((id) >= IRQ_PIC2_BASE ? outb(PIC2_CMD,PIC_EOI) : (void)0,outb(PIC1_CMD,PIC_EOI))
#define IRQ_PIC1_EOI()  (outb(PIC1_CMD,PIC_EOI))
#define IRQ_PIC2_EOI()  (outb(PIC2_CMD,PIC_EOI),outb(PIC1_CMD,PIC_EOI))

/* Check if a given IRQ number is spurious.
 * NOTE: This is done by checking the ISR register of the associated PIC
 *       controller to see if the 7th interrupt pin has been raised.
 *       A spurious interrupt was raised if it is up, in which case
 *       no EOI must be send.
 * NOTE: This function must be called at the start of any PIC hardware
 *       interrupt handler (aka. when 'IRQ_ISPIC(irq_t)' is true):
 * >>
 * >> #define MY_INTNO  0x2e // Same behavior for 0x27 as well
 * >>
 * >> PRIVATE void my_inthandler(void);
 * >> DEFINE_INT_HANDLER(my_irqhandler,my_inthandler);
 * >>
 * >> PRIVATE void my_inthandler(void) {
 * >>     if (IRQ_PIC_SPURIOUS(MY_INTNO)) return;
 * >>     
 * >>     syslog(LOG_DEBUG,"[IRQ] Interrupt #%d fired\n",MY_INTNO);
 * >>     
 * >>     IRQ_PIC_EOI(MY_INTNO);
 * >> }
 * >> 
 * >> INTERN MODULE_INIT void KCALL mymodule_init(void) {
 * >>     isr_t handler;
 * >>     handler.i_num   = MY_INTNO;
 * >>     handler.i_flags = IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE;
 * >>     handler.i_func  = &my_irqhandler;
 * >>     irq_set(&handler,NULL,true);
 * >> }
 * >> 
 * HINT: When a non-hardware interrupt number is passed, this function behaves as a no-op.
 * NOTE: In the event of a spurious slave interrupt, this function
 *       will automatically send an EOI command to the master PIC.
 */
#define IRQ_PIC_SPURIOUS(id)  ((id) >= IRQ_PIC2_BASE ? IRQ_PIC2_SPURIOUS(id) : IRQ_PIC1_SPURIOUS(id))
#define IRQ_PIC1_SPURIOUS(id) ((id) == IRQ_PIC1(7) && irq_pic_1_spurious())
#define IRQ_PIC2_SPURIOUS(id) ((id) == IRQ_PIC2(7) && irq_pic_2_spurious())

FUNDEF bool KCALL irq_pic_1_spurious(void);
FUNDEF bool KCALL irq_pic_2_spurious(void);

struct spurious_pic {
 u32 sp_pic1; /*< Amount of spurious interrupts that occurred on PIC #1. */
 u32 sp_pic2; /*< Amount of spurious interrupts that occurred on PIC #2. */
};

/* Tracking information about spurious interrupts.
 * >> Useful for detecting faulty software that sends EOI commands at the
 *    wrong time, or for tracking problems in hardware, such as line noise. */
DATDEF struct spurious_pic irq_pic_spurious;


/* Get/Set the mask of disabled interrupt lines.
 * >> 'IRQ_PIC1_STMASK()' disables irq_t: 0x20...0x27
 * >> 'IRQ_PIC2_STMASK()' disables irq_t: 0x28...0x2f
 */
#define IRQ_PIC1_GTMASK()   inb_p(PIC1_DATA)
#define IRQ_PIC1_STMASK(m) outb_p(PIC1_DATA,m)
#define IRQ_PIC2_GTMASK()   inb_p(PIC2_DATA)
#define IRQ_PIC2_STMASK(m) outb_p(PIC2_DATA,m)



/* IRQ number. */

#define IRQ_PIC1_BASE 0x20
#define IRQ_PIC2_BASE 0x28

/* Return the IRQ numbers of hardware interrupt
 * lines wired either to the master, or slave PIC.
 * @param: i :  The line number (0..7)
 * @return: * : The IRQ number. */
#define IRQ_PIC1(i) (IRQ_PIC1_BASE+(i))
#define IRQ_PIC2(i) (IRQ_PIC2_BASE+(i))

/* Check the type of interrupt, given its IRQ number. */
#define IRQ_ISEXC(i) ((i) < 0x20)                /* Exception. */
#define IRQ_ISPIC(i) ((i) >= 0x20 && (i) < 0x30) /* Hardware, PIC interrupt. */
#define IRQ_ISUSR(i) ((i) >= 0x30)               /* Custom interrupts. */


/* Standard ISA IRQs.
 * >> This is the default wireing of hardware interrupts. */
#define IRQ_PIC1_PIT   IRQ_PIC1(0) /*< Programmable Interrupt Timer Interrupt. */
#define IRQ_PIC1_KBD   IRQ_PIC1(1) /*< Keyboard Interrupt. */
#define IRQ_PIC1_CAS   IRQ_PIC1(2) /*< Cascade (used internally by the two PICs. never raised). */
#define IRQ_PIC1_COM2  IRQ_PIC1(3) /*< COM2 (if enabled). */
#define IRQ_PIC1_COM1  IRQ_PIC1(4) /*< COM1 (if enabled). */
#define IRQ_PIC1_LPT2  IRQ_PIC1(5) /*< LPT2 (if enabled). */
#define IRQ_PIC1_FLOP  IRQ_PIC1(6) /*< Floppy Disk. */
#define IRQ_PIC1_LPT1  IRQ_PIC1(7) /*< LPT1 / Unreliable "spurious" interrupt (usually). */
#define IRQ_PIC2_CMOS  IRQ_PIC2(0) /*< CMOS real-time clock (if enabled). */
#define IRQ_PIC2_FREE1 IRQ_PIC2(1) /*< Free for peripherals / legacy SCSI / NIC. */
#define IRQ_PIC2_FREE2 IRQ_PIC2(2) /*< Free for peripherals / SCSI / NIC. */
#define IRQ_PIC2_FREE3 IRQ_PIC2(3) /*< Free for peripherals / SCSI / NIC. */
#define IRQ_PIC2_PS2M  IRQ_PIC2(4) /*< PS2 Mouse. */
#define IRQ_PIC2_FPU   IRQ_PIC2(5) /*< FPU / Coprocessor / Inter-processor. */
#define IRQ_PIC2_ATA1  IRQ_PIC2(6) /*< Primary ATA Hard Disk. */
#define IRQ_PIC2_ATA2  IRQ_PIC2(7) /*< Secondary ATA Hard Disk. */

/* X86 exception IRQ numbers. */
#define IRQ_EXC(x)  x
#define IRQ_EXC_DE  IRQ_EXC(0)  /*< Divide-by-zero. */
#define IRQ_EXC_DB  IRQ_EXC(1)  /*< Debug. */
#define IRQ_EXC_NMI IRQ_EXC(2)  /*< Non-maskable Interrupt. */
#define IRQ_EXC_BP  IRQ_EXC(3)  /*< Breakpoint. */
#define IRQ_EXC_OF  IRQ_EXC(4)  /*< Overflow. */
#define IRQ_EXC_BR  IRQ_EXC(5)  /*< Bound Range Exceeded. */
#define IRQ_EXC_UD  IRQ_EXC(6)  /*< Invalid Opcode. */
#define IRQ_EXC_NM  IRQ_EXC(7)  /*< Device Not Available. */
#define IRQ_EXC_DF  IRQ_EXC(8)  /*< Double Fault. */
#define IRQ_EXC_TS  IRQ_EXC(10) /*< Invalid TSS. */
#define IRQ_EXC_NP  IRQ_EXC(11) /*< Segment Not Present. */
#define IRQ_EXC_SS  IRQ_EXC(12) /*< Stack-Segment Fault. */
#define IRQ_EXC_GP  IRQ_EXC(13) /*< General Protection Fault. */
#define IRQ_EXC_PF  IRQ_EXC(14) /*< Page Fault. */
#define IRQ_EXC_MF  IRQ_EXC(16) /*< x87 Floating-Point Exception. */
#define IRQ_EXC_AC  IRQ_EXC(17) /*< Alignment Check. */
#define IRQ_EXC_MC  IRQ_EXC(18) /*< Machine Check. */
#define IRQ_EXC_XM  IRQ_EXC(19) /*< SIMD Floating-Point Exception. */
#define IRQ_EXC_XF  IRQ_EXC_XM  /*< SIMD Floating-Point Exception. */
#define IRQ_EXC_VE  IRQ_EXC(20) /*< Virtualization Exception. */
#define IRQ_EXC_SX  IRQ_EXC(30) /*< Security Exception. */

#define EXC_DIVIDE_BY_ZERO   IRQ_EXC_DE
#define EXC_BREAKPOINT       IRQ_EXC_BP
#define EXC_INTEGER_OVERFLOW IRQ_EXC_OF
#define EXC_OUT_OF_BOUNDS    IRQ_EXC_BR
#define EXC_INVALID_OPCODE   IRQ_EXC_UD
#define EXC_DOUBLE_FAULT     IRQ_EXC_DF
#define EXC_PROTECTION_FAULT IRQ_EXC_GP
#define EXC_PAGE_FAULT       IRQ_EXC_PF
#define EXC_FPU_EXCEPTION    IRQ_EXC_MF
#define EXC_ALIGNMENT_ERROR  IRQ_EXC_AC

/* Additional, KOS-specific IRQ vector numbers. */
#define IRQ_SYSCALL        0x80 /*< Interrupt vector used for linux-compatible & kos-specific system calls. */
#ifdef CONFIG_SMP
#define IRQ_LAPIC_RPC      0xfe /*< Interrupt slot for inter-processor communication. */
#define IRQ_LAPIC_SPURIOUS 0xff /*< Spurious LAPIC IRQ handler. */
#endif




typedef void (ASMCALL *isr_fun_t)(void);

struct instance;

/* Raw, low-level IRQ handler.
 * No additional handling is performed before this handler is called. */
typedef struct {
 irq_t                i_num;     /*< Interrupt service routine number. */
 u8                   i_flags;   /*< Set of 'IDTFLAG_*|IDTTYPE_*'. */
 u16                  i_padding; /* ... */
 isr_fun_t            i_func;    /*< Interrupt handler function. */
 REF struct instance *i_owner;   /*< [1..1] Function owner. */
} isr_t;

#define ISR_DEFAULT(id,func)      {id,IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE,0,func,THIS_INSTANCE}
#define ISR_DEFAULT_DPL3(id,func) {id,IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE|IDTFLAG_DPL(3),0,func,THIS_INSTANCE}

/* Fill on 'i_flags' and 'i_func' using 'num'.
 * WARNING: The caller inherits a reference to 'handler->i_owner' */
FUNDEF void KCALL irq_get(irq_t num, isr_t *__restrict handler);

/* Install the interrupt service routine 'new_handle' in the current CPU,
 * storing the old handler in '*old_handler' when non-NULL is passed.
 * @param: new_handler: (Required) The new IRQ handler. (NOTE: This function will create a reference to 'i_owner')
 * @param: old_handler: (Optional) The old IRQ handler. (NOTE: This function will return a reference to 'i_owner')
 * @param: mode:        A set of 'IRQ_SET_*'
 * @return: true:       Successfully installed the new interrupt handler.
 * @return: false:      The instance associated with 'new_handler' does not permit new references being created. */
FUNDEF SAFE bool KCALL irq_set(isr_t const *__restrict new_handler,
                           REF isr_t *old_handler, int mode);
#define IRQ_SET_QUICK   0x00 /*< Quickly install the given IRQ handler. */
#define IRQ_SET_RELOAD  0x01 /*< When set, reload the IDT vector, thus safely activating the interrupt. */
#define IRQ_SET_INHERIT 0x02 /*< Inherit a reference from 'new_handler->i_owner', thus never failing. */

/* Delete the custom interrupt handler for 'num', restoring the default/fallback handler. */
FUNDEF SAFE void KCALL irq_del(irq_t num, bool reload);





/* Amount of 'struct idtentry' that make up a full IDT table. */
#define IDT_TABLESIZE 256

#define IDTENTRY_OFFSETOF_OFF1  0
#define IDTENTRY_OFFSETOF_SEL   2
#define IDTENTRY_OFFSETOF_ZERO  4
#define IDTENTRY_OFFSETOF_FLAGS 5
#define IDTENTRY_OFFSETOF_OFF2  6
#define IDTENTRY_SIZE           8
struct PACKED idtentry {
union PACKED {
 u64 ie_data;
struct PACKED {
 u16 ie_off1;  /*< Lower 16 bits of an 'irq_handler' pointer. */
 u16 ie_sel;   /*< Kernel code segment (always '__KERNEL_CS') */
 u8  ie_zero;  /*< Always ZERO(0). */
 u8  ie_flags; /*< Set of 'IDTFLAG_*|IDTTYPE_*' */
 u16 ie_off2;  /*< Upper 16 bits of an 'irq_handler' pointer. */
};};};

#define IDTFLAG_PRESENT                 0x80 /*< Set to 0 for unused interrupts. */
/* Descriptor Privilege LevelGate call protection.
 * Specifies which privilege Level the calling Descriptor minimum should have.
 * So hardware and CPU interrupts can be protected from being called out of userspace. */
#define IDTFLAG_DPL(n)          (((n)&3)<<5) /*< Mask: 0x60 */
#define IDTFLAG_STORAGE_SEGMENT         0x10 /*< Set to 0 for interrupt gates. */
#define IDTTYPE_80386_32_TASK_GATE      0x05
#define IDTTYPE_80286_16_INTERRUPT_GATE 0x06
#define IDTTYPE_80286_16_TRAP_GATE      0x07
#define IDTTYPE_80386_32_INTERRUPT_GATE 0x0E
#define IDTTYPE_80386_32_TRAP_GATE      0x0F





#if 1
/* The default IRQ handler that will either:
 * - cause user-space applications to terminate (unless they provide appropriate handlers)
 * - syslog() a warning for unhandled PIC interrupts (hardware interrupts)
 * - log all it can and cause kernel panic
 * NOTE: When calling this function from assembly, make sure to notice
 *       that it takes an extended CPU-state structure, as well as
 *       using the fastcall calling-convention. */
FUNDEF void FCALL irq_unhandled(irq_t code, struct cpustate *__restrict state);
FUNDEF void FCALL irq_unhandled_c(irq_t code, struct cpustate_irq_c *__restrict state);
#endif


#ifdef CONFIG_BUILDING_KERNEL_CORE
struct cpu;

/* WARNING: init-function: Initialize the cpu-local IDT table. */
INTDEF void KCALL irq_initialize(void);
INTDEF void KCALL irq_setup(struct cpu *__restrict self);

INTDEF void KCALL pic_bios_begin(void);
INTDEF void KCALL pic_bios_end(void);

struct IDT {
     struct idtentry  i_vector[IDT_TABLESIZE];
 REF struct instance *i_owners[IDT_TABLESIZE]; /*< [0..1] Owner instances of different vectors. (NOTE: Never set to the kernel instance) */
};

/* The per-CPU interrupt descriptor table data structure.
 * [lock(THIS_CPU)] (May only be accessed by a CPU itself)
 * NOTE: For non-atomic access, preemption must be disabled. */
INTDEF PERCPU struct IDT cpu_idt;

#endif




#ifdef CONFIG_DEBUG
#define __DEBUG_CODE(...) __VA_ARGS__
#else
#define __DEBUG_CODE(...) 
#endif


#define __ASM_PUSH_SEGMENTS \
    pushw %ds; \
    pushw %es; \
    pushw %fs; \
    pushw %gs;
#define __ASM_POP_SEGMENTS \
    popw  %gs; \
    popw  %fs; \
    popw  %es; \
    popw  %ds;
#define __PUSH_SEGMENTS \
    "pushw %ds\n" \
    "pushw %es\n" \
    "pushw %fs\n" \
    "pushw %gs\n"
#define __POP_SEGMENTS \
    "popw  %gs\n" \
    "popw  %fs\n" \
    "popw  %es\n" \
    "popw  %ds\n"
#define __ASM_PUSH_REGISTERS pushal;
#define __ASM_POP_REGISTERS  popal;
#define __PUSH_REGISTERS "pushal\n"
#define __POP_REGISTERS  "popal\n"

#ifdef __x86_64__
#define __ASM_LOAD_SEGMENTS(temp) \
    /* Load the proper kernel segment registers */ \
    movw  $(__USER_DS), temp; \
    movw  temp, %ds; \
    movw  temp, %es; \
    movw  temp, %fs; \
    movw  $(__KERNEL_PERCPU), temp; \
    movw  temp, %gs;
#define __LOAD_SEGMENTS(temp) \
    /* Load the proper kernel segment registers */ \
    "movw  $(" __PP_STR(__USER_DS) "), " temp "\n" \
    "movw  " temp ", %ds\n" \
    "movw  " temp ", %es\n" \
    "movw  " temp ", %fs\n" \
    "movw  $(" __PP_STR(__KERNEL_PERCPU) "), " temp "\n" \
    "movw  " temp ", %gs\n"
#else
#define __ASM_LOAD_SEGMENTS(temp) \
    /* Load the proper kernel segment registers */ \
    movw  $(__USER_DS), temp; \
    movw  temp, %ds; \
    movw  temp, %es; \
    movw  temp, %gs; \
    movw  $(__KERNEL_PERCPU), temp; \
    movw  temp, %fs;
#define __LOAD_SEGMENTS(temp) \
    /* Load the proper kernel segment registers */ \
    "movw  $(" __PP_STR(__USER_DS) "), " temp "\n" \
    "movw  " temp ", %ds\n" \
    "movw  " temp ", %es\n" \
    "movw  " temp ", %gs\n" \
    "movw  $(" __PP_STR(__KERNEL_PERCPU) "), " temp "\n" \
    "movw  " temp ", %fs\n"
#endif

#define __INT_ENTER \
   __PUSH_SEGMENTS \
   __PUSH_REGISTERS \
   __LOAD_SEGMENTS("%dx") \
   "movl 16(%esp), %edx\n"
#define __INT_LEAVE \
   __POP_REGISTERS \
   __POP_SEGMENTS \
   "iret\n"
#define __INT_LEAVE_E \
   __POP_REGISTERS \
   __POP_SEGMENTS \
   "addl $4, %esp\n" /* Error code */ \
   "iret\n"




/* A higher-level interrupt handler that can safely
 * be called implemented from a higher-level language, such as C. */
typedef void int_handler(void);

/* Define an interrupt handler wrapper 'h_irq' that
 * calls an int_handler-compatible 'h_int' */
#define DEFINE_INT_HANDLER(h_irq,h_int) \
void (ASMCALL h_irq)(void); __asm__( \
".section .text\n" \
"" PP_STR(h_irq) ":\n" \
__INT_ENTER \
"    movl  %esp, %ecx\n" \
__DEBUG_CODE("pushl 32(%esp)\n") \
__DEBUG_CODE("pushl %ebp\n") \
__DEBUG_CODE("movl %esp, %ebp\n") \
"    call " PP_STR(h_int) "\n" \
__DEBUG_CODE("addl $8, %esp\n") \
__INT_LEAVE \
".size " PP_STR(h_irq) ", . - " PP_STR(h_irq) "\n" \
".previous\n" \
)


/* A higher-level interrupt handler, capable of
 * directly inspecting/modifying the cpu-state that
 * should be restored once it returns normally.
 * NOTE: Use of such a handler requires an IRQ
 *       wrapper generated by 'DEFINE_EXC_HANDLER'. */
typedef void FCALL exc_handler(struct cpustate *__restrict state);

/* Define an exception handler wrapper 'h_irq' that
 * calls an exc_handler-compatible 'h_exc' */
#define DEFINE_EXC_HANDLER(h_irq,h_exc) \
void (ASMCALL h_irq)(void); __asm__( \
".section .text\n" \
"" PP_STR(h_irq) ":\n" \
__INT_ENTER \
"    movl  %esp, %ecx\n" \
__DEBUG_CODE("pushl 40(%esp)\n") \
__DEBUG_CODE("pushl %ebp\n") \
__DEBUG_CODE("movl %esp, %ebp\n") \
"    call " PP_STR(h_exc) "\n" \
__DEBUG_CODE("addl $8, %esp\n") \
__INT_LEAVE \
".size " PP_STR(h_irq) ", . - " PP_STR(h_irq) "\n" \
".previous\n" \
)

/* An even higher-level version specifically designed
 * to swapping cpu-states by-pointer with the ability
 * to store the previous state.
 * >> This function should return the new state. */
typedef struct cpustate *FCALL
task_handler(struct cpustate *__restrict old_state);

/* Define a task handler wrapper 'h_irq' that
 * calls an task_handler-compatible 'h_task' */
#define DEFINE_TASK_HANDLER(h_irq,h_task) \
void (ASMCALL h_irq)(void); __asm__( \
".section .text\n" \
"" PP_STR(h_irq) ":\n" \
__INT_ENTER \
"    movl  %esp, %ecx\n" \
__DEBUG_CODE("pushl 40(%esp)\n") \
__DEBUG_CODE("pushl %ebp\n") \
__DEBUG_CODE("movl %esp, %ebp\n") \
"    call " PP_STR(h_task) "\n" \
/*__DEBUG_CODE("addl $8, %esp\n")*/ \
"    movl  %eax, %esp\n" \
__INT_LEAVE \
".size " PP_STR(h_irq) ", . - " PP_STR(h_irq) "\n" \
".previous\n" \
)

/* A high-level interrupt handler meant to be used
 * for messages carrying an additional exception code,
 * such as PAGEFAULT and others.
 * >> The function may modify the passed state,
 *    which will be restored once it returns.
 * NOTE: The default IRQ handler 'irq_default' is compatible with this.
 * WARNING: The user must ensure that interrupts triggering
 *          an XCODE-handler _ALWAYS_ include the exc_code field!
 *          It may never be absent!
 *          The same way, no other kind of handler may be used if there is a code! */
typedef void FCALL code_handler(struct cpustate_irq_c *__restrict info);


/* Define a task handler wrapper 'h_irq' that
 * calls an code_handler-compatible 'h_code' */
#define DEFINE_CODE_HANDLER(h_irq,h_code) \
void (ASMCALL h_irq)(void); __asm__( \
".section .text\n" \
"" PP_STR(h_irq) ":\n" \
__INT_ENTER \
"    movl  %esp, %ecx\n" \
__DEBUG_CODE("pushl 44(%esp)\n") \
__DEBUG_CODE("pushl %ebp\n") \
__DEBUG_CODE("movl %esp, %ebp\n") \
"    call " PP_STR(h_code) "\n" \
__DEBUG_CODE("addl $8, %esp\n") \
__INT_LEAVE_E \
".size " PP_STR(h_irq) ", . - " PP_STR(h_irq) "\n" \
".previous\n" \
)

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_IRQ_H */
