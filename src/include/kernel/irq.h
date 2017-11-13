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
#include <stdbool.h>
#include <arch/gdt.h>
#include <arch/cpustate.h>
#include <arch/interrupt.h>

#undef CONFIG_USE_OLD_INTERRUPTS
#ifndef __x86_64__
//#define CONFIG_USE_OLD_INTERRUPTS 1
#endif

#ifndef CONFIG_USE_OLD_INTERRUPTS
#include "interrupt.h"

#define IRQ_PIC_SPURIOUS(id)  0
#define IRQ_PIC1_SPURIOUS(id) 0
#define IRQ_PIC2_SPURIOUS(id) 0

#else /* !CONFIG_USE_OLD_INTERRUPTS */

DECL_BEGIN

struct cpustate;
struct cpustate_e;

#define IRQ_PIC_SPURIOUS(id)  ((id) >= INTNO_PIC2_BASE ? IRQ_PIC2_SPURIOUS(id) : IRQ_PIC1_SPURIOUS(id))
#define IRQ_PIC1_SPURIOUS(id) ((id) == INTNO_PIC1(7) && irq_pic_1_spurious())
#define IRQ_PIC2_SPURIOUS(id) ((id) == INTNO_PIC2(7) && irq_pic_2_spurious())
FUNDEF bool KCALL irq_pic_1_spurious(void);
FUNDEF bool KCALL irq_pic_2_spurious(void);

typedef void (ASMCALL *isr_fun_t)(void);

struct instance;

/* Raw, low-level IRQ handler.
 * No additional handling is performed before this handler is called. */
typedef struct {
 irq_t                i_num;     /*< Interrupt service routine number. */
 u8                   i_flags;   /*< Set of `IDTFLAG_*|IDTTYPE_*'. */
 u16                  i_padding; /* ... */
 isr_fun_t            i_func;    /*< Interrupt handler function. */
 REF struct instance *i_owner;   /*< [1..1] Function owner. */
} isr_t;

#define ISR_DEFAULT(id,func)      {id,IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE,0,func,THIS_INSTANCE}
#define ISR_DEFAULT_DPL3(id,func) {id,IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE|IDTFLAG_DPL(3),0,func,THIS_INSTANCE}

/* Fill on `i_flags' and `i_func' using `num'.
 * WARNING: The caller inherits a reference to `handler->i_owner' */
FUNDEF void KCALL irq_get(irq_t num, isr_t *__restrict handler);

/* Install the interrupt service routine `new_handler' in the current CPU,
 * storing the old handler in `*old_handler' when non-NULL is passed.
 * @param: new_handler: (Required) The new IRQ handler. (NOTE: This function will create a reference to `i_owner')
 * @param: old_handler: (Optional) The old IRQ handler. (NOTE: This function will return a reference to `i_owner')
 * @param: mode:        A set of `IRQ_SET_*'
 * @return: true:       Successfully installed the new interrupt handler.
 * @return: false:      The instance associated with `new_handler' does not permit new references being created. */
FUNDEF SAFE bool KCALL irq_set(isr_t const *__restrict new_handler,
                           REF isr_t *old_handler, int mode);
#define IRQ_SET_QUICK   0x00 /*< Quickly install the given IRQ handler. */
#define IRQ_SET_RELOAD  0x01 /*< When set, reload the IDT vector, thus safely activating the interrupt. */
#define IRQ_SET_INHERIT 0x02 /*< Inherit a reference from `new_handler->i_owner', thus never failing. */

/* Delete the custom interrupt handler for `num', restoring the default/fallback handler. */
FUNDEF SAFE void KCALL irq_del(irq_t num, bool reload);


/* The default IRQ handler that will either:
 * - cause user-space applications to terminate (unless they provide appropriate handlers)
 * - syslog() a warning for unhandled PIC interrupts (hardware interrupts)
 * - log all it can and cause kernel panic
 * NOTE: When calling this function from assembly, make sure to
 *       notice that the fastcall calling-convention is used.
 */
FUNDEF void FCALL irq_default(int intno, struct cpustate_e *__restrict state);


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


/* A higher-level interrupt handler that can safely
 * be called implemented from a higher-level language, such as C. */
typedef void int_handler(void);

/* Define an interrupt handler wrapper `h_irq' that
 * calls an int_handler-compatible `h_int' */
#ifdef __x86_64__
#define DEFINE_INT_HANDLER(h_irq,h_int) \
void (ASMCALL h_irq)(void); __asm__( \
".section .text\n" \
"" PP_STR(h_irq) ":\n" \
__INT_ENTER \
"    movq  %rsp, %rdi\n" \
__DEBUG_CODE("pushq 88(%esp)\n") \
__DEBUG_CODE("pushq %rbp\n") \
__DEBUG_CODE("movq %rsp, %rbp\n") \
"    call " PP_STR(h_int) "\n" \
__DEBUG_CODE("addq $16, %rsp\n") \
__INT_LEAVE \
".size " PP_STR(h_irq) ", . - " PP_STR(h_irq) "\n" \
".previous\n" \
)
#else
#define DEFINE_INT_HANDLER(h_irq,h_int) \
void (ASMCALL h_irq)(void); __asm__( \
".section .text\n" \
"" PP_STR(h_irq) ":\n" \
__INT_ENTER \
"    movl  %esp, %ecx\n" \
__DEBUG_CODE("pushl 40(%esp)\n") \
__DEBUG_CODE("pushl %ebp\n") \
__DEBUG_CODE("movl %esp, %ebp\n") \
"    call " PP_STR(h_int) "\n" \
__DEBUG_CODE("addl $8, %esp\n") \
__INT_LEAVE \
".size " PP_STR(h_irq) ", . - " PP_STR(h_irq) "\n" \
".previous\n" \
)
#endif


/* A higher-level interrupt handler, capable of
 * directly inspecting/modifying the cpu-state that
 * should be restored once it returns normally.
 * NOTE: Use of such a handler requires an IRQ
 *       wrapper generated by `DEFINE_EXC_HANDLER'. */
typedef void FCALL exc_handler(struct cpustate *__restrict state);

/* Define an exception handler wrapper `h_irq' that
 * calls an exc_handler-compatible `h_exc' */
#ifdef __x86_64__
#define DEFINE_EXC_HANDLER(h_irq,h_exc) \
void (ASMCALL h_irq)(void); __asm__( \
".section .text\n" \
"" PP_STR(h_irq) ":\n" \
__INT_ENTER \
"    movq  %rsp, %rdi\n" \
__DEBUG_CODE("pushq 88(%rsp)\n") \
__DEBUG_CODE("pushq %rbp\n") \
__DEBUG_CODE("movq %rsp, %rbp\n") \
"    call " PP_STR(h_exc) "\n" \
__DEBUG_CODE("addq $16, %rsp\n") \
__INT_LEAVE \
".size " PP_STR(h_irq) ", . - " PP_STR(h_irq) "\n" \
".previous\n" \
)
#else
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
#endif

/* An even higher-level version specifically designed
 * to swapping cpu-states by-pointer with the ability
 * to store the previous state.
 * >> This function should return the new state. */
typedef struct cpustate *FCALL
task_handler(struct cpustate *__restrict old_state);

/* Define a task handler wrapper `h_irq' that
 * calls an task_handler-compatible `h_task' */
#ifdef __x86_64__
#define DEFINE_TASK_HANDLER(h_irq,h_task) \
void (ASMCALL h_irq)(void); __asm__( \
".section .text\n" \
"" PP_STR(h_irq) ":\n" \
__INT_ENTER \
"    movq  %rsp, %rdi\n" \
__DEBUG_CODE("pushq 88(%rsp)\n") \
__DEBUG_CODE("pushq %rbp\n") \
__DEBUG_CODE("movq %rsp, %rbp\n") \
"    call " PP_STR(h_task) "\n" \
/*__DEBUG_CODE("addq $16, %rsp\n")*/ \
"    movq  %rax, %rsp\n" \
__INT_LEAVE \
".size " PP_STR(h_irq) ", . - " PP_STR(h_irq) "\n" \
".previous\n" \
)
#else
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
#endif

/* A high-level interrupt handler meant to be used
 * for messages carrying an additional exception code,
 * such as PAGEFAULT and others.
 * >> The function may modify the passed state,
 *    which will be restored once it returns.
 * WARNING: The user must ensure that interrupts triggering
 *          an XCODE-handler _ALWAYS_ include the exc_code field!
 *          It may never be absent!
 *          The same way, no other kind of handler may be used if there is a code! */
typedef void FCALL code_handler(struct cpustate_e *__restrict info);


/* Define a task handler wrapper `h_irq' that
 * calls an code_handler-compatible `h_code' */
#ifdef __x86_64__
#define DEFINE_CODE_HANDLER(h_irq,h_code) \
void (ASMCALL h_irq)(void); __asm__( \
".section .text\n" \
"" PP_STR(h_irq) ":\n" \
__INT_ENTER \
"    movq  %rsp, %rdi\n" \
__DEBUG_CODE("pushq 96(%rsp)\n") \
__DEBUG_CODE("pushq %rbp\n") \
__DEBUG_CODE("movq %rsp, %rbp\n") \
"    call " PP_STR(h_code) "\n" \
__DEBUG_CODE("addq $16, %rsp\n") \
__INT_LEAVE_E \
".size " PP_STR(h_irq) ", . - " PP_STR(h_irq) "\n" \
".previous\n" \
)
#else
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
#endif

DECL_END
#endif /* CONFIG_USE_OLD_INTERRUPTS */


#ifdef __x86_64__
#define __ASM_LOAD_SEGMENTS(temp) \
    /* Load the proper kernel segment registers */ \
  /*movw  $(__USER_DS), temp; \
    movw  temp, %ds; \
    movw  temp, %es; \
    movw  temp, %fs; \
    movw  $(__KERNEL_PERCPU), temp; \
    movw  temp, %gs; */
#else
#define __ASM_LOAD_SEGMENTS(temp) \
    /* Load the proper kernel segment registers */ \
    movw  $(__USER_DS), temp; \
    movw  temp, %ds; \
    movw  temp, %es; \
    movw  temp, %gs; \
    movw  $(__KERNEL_PERCPU), temp; \
    movw  temp, %fs;
#endif
#define __LOAD_SEGMENTS(temp) \
    __PP_STR(__ASM_LOAD_SEGMENTS(temp))

#define __INT_ENTER \
   PP_STR(__ASM_PUSH_COMREGS) \
   __LOAD_SEGMENTS(%dx)
#ifdef __x86_64__
#define __INT_LEAVE \
   PP_STR(__ASM_POP_COMREGS) \
   "iretq\n"
#define __INT_LEAVE_E \
   PP_STR(__ASM_POP_COMREGS) \
   "addq $8, %rsp\n" /* Error code */ \
   "iretq\n"
#else
#define __INT_LEAVE \
   PP_STR(__ASM_POP_COMREGS) \
   "iret\n"
#define __INT_LEAVE_E \
   PP_STR(__ASM_POP_COMREGS) \
   "addl $4, %esp\n" /* Error code */ \
   "iret\n"
#endif


#endif /* !GUARD_INCLUDE_KERNEL_IRQ_H */
