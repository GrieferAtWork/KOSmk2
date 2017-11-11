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
#ifndef GUARD_KERNEL_CORE_ARCH_INTERRUPT_C
#define GUARD_KERNEL_CORE_ARCH_INTERRUPT_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <kernel/arch/cpustate.h>
#include <kernel/arch/gdt.h>
#include <kernel/arch/idt_pointer.h>
#include <kernel/interrupt.h>
#include <asm/instx.h>
#include <sched/percpu.h>
#include <hybrid/asm.h>
#include <hybrid/list/list.h>
#include <malloc.h>
#include <kernel/malloc.h>
#include <kernel/arch/preemption.h>
#include <sched.h>
#include <sched/smp.h>
#include <linker/module.h>
#include <hybrid/panic.h>
#include <sched/cpu.h>
#include <kernel/irq.h>
#include <asm/registers.h>

DECL_BEGIN

struct entry {
 LIST_HEAD(struct interrupt) e_head; /*< [0..1] Chain of interrupt handlers in this entry. */
};

struct PACKED interrupt_table {
 struct PACKED idtentry it_idt[256]; /* Internal CPU Interrupt Descriptor Table. */
 struct entry           it_tab[256]; /* Per-cpu + per-vector descriptors. */
};


INTERN CPU_BSS struct PACKED interrupt_table inttab;
GLOBAL_ASM(
L(SYM_PUBLIC(intno_offset)                         )
L(intno_offset = inttab+256*IDTENTRY_SIZE          )
);

#ifdef CONFIG_SMP
/* Lock that must be held when modifying any `inttab'. */
PRIVATE DEFINE_ATOMIC_RWLOCK(inttab_lock);
#define IFDEF_SMP(x) x
#else
#define IFDEF_SMP(x) (void)0
#endif


struct PACKED dirq_asm { byte_t entry_point[10]; }; /* sizeof(DIRQ(...)) */
INTDEF struct dirq_asm dirq_tab[256]; /* Table of default IRQ entry points. */
INTDEF struct dirq_asm aseg_tab[256]; /* Table of assembly segment-safe IRQ entry points. */
INTDEF struct dirq_asm fast_tab[256]; /* Table of fast c-function IRQ entry points. */
INTDEF struct dirq_asm stat_tab[256]; /* Table of full cpu-state IRQ entry points. */
#define DIRQ_ENTRY(intno) ((uintptr_t)dirq_tab[intno].entry_point)
#define ASEG_ENTRY(intno) ((uintptr_t)aseg_tab[intno].entry_point)
#define FAST_ENTRY(intno) ((uintptr_t)fast_tab[intno].entry_point)
#define STAT_ENTRY(intno) ((uintptr_t)stat_tab[intno].entry_point)


/*[[[deemon
local ycode = [8,10,11,12,13,14,17,30];
function x(n) -> (n in ycode ? "yirq" : "nirq")+("(0x%.2I8x)" % n);
print "#define IRQ_TABLE(nirq,yirq) \\";
for (local i = 0; i < 32; ++i) {
    print " {} {} {} {} {} {} {} {} {}"
          .format({ x(i*8+0),x(i*8+1),x(i*8+2),x(i*8+3),
                    x(i*8+4),x(i*8+5),x(i*8+6),x(i*8+7),
                   (i == 31 ? "" : "\\"),
          });
}
]]]*/
#define IRQ_TABLE(nirq,yirq) \
 nirq(0x00) nirq(0x01) nirq(0x02) nirq(0x03) nirq(0x04) nirq(0x05) nirq(0x06) nirq(0x07) \
 yirq(0x08) nirq(0x09) yirq(0x0a) yirq(0x0b) yirq(0x0c) yirq(0x0d) yirq(0x0e) nirq(0x0f) \
 nirq(0x10) yirq(0x11) nirq(0x12) nirq(0x13) nirq(0x14) nirq(0x15) nirq(0x16) nirq(0x17) \
 nirq(0x18) nirq(0x19) nirq(0x1a) nirq(0x1b) nirq(0x1c) nirq(0x1d) yirq(0x1e) nirq(0x1f) \
 nirq(0x20) nirq(0x21) nirq(0x22) nirq(0x23) nirq(0x24) nirq(0x25) nirq(0x26) nirq(0x27) \
 nirq(0x28) nirq(0x29) nirq(0x2a) nirq(0x2b) nirq(0x2c) nirq(0x2d) nirq(0x2e) nirq(0x2f) \
 nirq(0x30) nirq(0x31) nirq(0x32) nirq(0x33) nirq(0x34) nirq(0x35) nirq(0x36) nirq(0x37) \
 nirq(0x38) nirq(0x39) nirq(0x3a) nirq(0x3b) nirq(0x3c) nirq(0x3d) nirq(0x3e) nirq(0x3f) \
 nirq(0x40) nirq(0x41) nirq(0x42) nirq(0x43) nirq(0x44) nirq(0x45) nirq(0x46) nirq(0x47) \
 nirq(0x48) nirq(0x49) nirq(0x4a) nirq(0x4b) nirq(0x4c) nirq(0x4d) nirq(0x4e) nirq(0x4f) \
 nirq(0x50) nirq(0x51) nirq(0x52) nirq(0x53) nirq(0x54) nirq(0x55) nirq(0x56) nirq(0x57) \
 nirq(0x58) nirq(0x59) nirq(0x5a) nirq(0x5b) nirq(0x5c) nirq(0x5d) nirq(0x5e) nirq(0x5f) \
 nirq(0x60) nirq(0x61) nirq(0x62) nirq(0x63) nirq(0x64) nirq(0x65) nirq(0x66) nirq(0x67) \
 nirq(0x68) nirq(0x69) nirq(0x6a) nirq(0x6b) nirq(0x6c) nirq(0x6d) nirq(0x6e) nirq(0x6f) \
 nirq(0x70) nirq(0x71) nirq(0x72) nirq(0x73) nirq(0x74) nirq(0x75) nirq(0x76) nirq(0x77) \
 nirq(0x78) nirq(0x79) nirq(0x7a) nirq(0x7b) nirq(0x7c) nirq(0x7d) nirq(0x7e) nirq(0x7f) \
 nirq(0x80) nirq(0x81) nirq(0x82) nirq(0x83) nirq(0x84) nirq(0x85) nirq(0x86) nirq(0x87) \
 nirq(0x88) nirq(0x89) nirq(0x8a) nirq(0x8b) nirq(0x8c) nirq(0x8d) nirq(0x8e) nirq(0x8f) \
 nirq(0x90) nirq(0x91) nirq(0x92) nirq(0x93) nirq(0x94) nirq(0x95) nirq(0x96) nirq(0x97) \
 nirq(0x98) nirq(0x99) nirq(0x9a) nirq(0x9b) nirq(0x9c) nirq(0x9d) nirq(0x9e) nirq(0x9f) \
 nirq(0xa0) nirq(0xa1) nirq(0xa2) nirq(0xa3) nirq(0xa4) nirq(0xa5) nirq(0xa6) nirq(0xa7) \
 nirq(0xa8) nirq(0xa9) nirq(0xaa) nirq(0xab) nirq(0xac) nirq(0xad) nirq(0xae) nirq(0xaf) \
 nirq(0xb0) nirq(0xb1) nirq(0xb2) nirq(0xb3) nirq(0xb4) nirq(0xb5) nirq(0xb6) nirq(0xb7) \
 nirq(0xb8) nirq(0xb9) nirq(0xba) nirq(0xbb) nirq(0xbc) nirq(0xbd) nirq(0xbe) nirq(0xbf) \
 nirq(0xc0) nirq(0xc1) nirq(0xc2) nirq(0xc3) nirq(0xc4) nirq(0xc5) nirq(0xc6) nirq(0xc7) \
 nirq(0xc8) nirq(0xc9) nirq(0xca) nirq(0xcb) nirq(0xcc) nirq(0xcd) nirq(0xce) nirq(0xcf) \
 nirq(0xd0) nirq(0xd1) nirq(0xd2) nirq(0xd3) nirq(0xd4) nirq(0xd5) nirq(0xd6) nirq(0xd7) \
 nirq(0xd8) nirq(0xd9) nirq(0xda) nirq(0xdb) nirq(0xdc) nirq(0xdd) nirq(0xde) nirq(0xdf) \
 nirq(0xe0) nirq(0xe1) nirq(0xe2) nirq(0xe3) nirq(0xe4) nirq(0xe5) nirq(0xe6) nirq(0xe7) \
 nirq(0xe8) nirq(0xe9) nirq(0xea) nirq(0xeb) nirq(0xec) nirq(0xed) nirq(0xee) nirq(0xef) \
 nirq(0xf0) nirq(0xf1) nirq(0xf2) nirq(0xf3) nirq(0xf4) nirq(0xf5) nirq(0xf6) nirq(0xf7) \
 nirq(0xf8) nirq(0xf9) nirq(0xfa) nirq(0xfb) nirq(0xfc) nirq(0xfd) nirq(0xfe) nirq(0xff) 
//[[[end]]]




GLOBAL_ASM(
#define DIRQ(n,target) \
   .byte 0x68; .long MOD(n);           /* pushq $(MOD(n)) */ \
   .byte 0xe9; .long target - 97f; 97: /* jmp target */
L(.section .text                                                              )
#define MOD(n)    ASM_IRREGS_ENCODE_INTNO(n)
#define YIRQ(n) L(DIRQ(n,default_irq_ycode))
#define NIRQ(n) L(DIRQ(n,default_irq_ncode))
L(PRIVATE_ENTRY(dirq_tab)                                                     )
IRQ_TABLE(NIRQ,YIRQ)
L(SYM_END(dirq_tab)                                                           )
#undef YIRQ
#undef NIRQ
#define YIRQ(n) L(DIRQ(n,user_irq_ycode))
#define NIRQ(n) L(DIRQ(n,user_irq_ncode))
L(PRIVATE_ENTRY(aseg_tab)                                                     )
IRQ_TABLE(NIRQ,YIRQ)
L(SYM_END(aseg_tab)                                                           )
#undef YIRQ
#undef NIRQ
#define YIRQ(n) L(DIRQ(n,fast_irq_ycode))
#define NIRQ(n) L(DIRQ(n,fast_irq_ncode))
L(PRIVATE_ENTRY(fast_tab)                                                     )
IRQ_TABLE(NIRQ,YIRQ)
L(SYM_END(fast_tab)                                                           )
#undef YIRQ
#undef NIRQ
#undef YIRQ
#undef NIRQ
#define YIRQ(n) L(DIRQ(n,stat_irq_ycode))
#define NIRQ(n) L(DIRQ(n,stat_irq_ncode))
L(PRIVATE_ENTRY(stat_tab)                                                     )
IRQ_TABLE(NIRQ,YIRQ)
L(SYM_END(stat_tab)                                                           )
#undef YIRQ
#undef NIRQ
#undef MOD
L(                                                                            )
L(PRIVATE_ENTRY(default_irq_ncode)                                            )
L(PRIVATE_ENTRY(default_irq_ycode)                                            )
L(                                                                            )
L(    __ASM_IRET                                                              )
L(SYM_END(default_irq_ycode)                                                  )
L(SYM_END(default_irq_ncode)                                                  )
L(                                                                            )
L(PRIVATE_ENTRY(user_irq_ncode)                                               )
#ifdef __x86_64__
L(    testq $0x3, (8+IRREGS_HOST_OFFSETOF_CS)(%esp)                           )
L(    jz 1f /* if (ORIGINATES_FROM_USERSPACE()) { ... */                      )
L(    swapgs                                                                  )
L(    pushq $4f /* return_address */                                          )
L(    pushq %rax                                                              )
L(    movq 16(%rsp), %rax /* intno*INTERRUPT_SIZE */                          )
L(    addq  ASM_CPU(CPU_OFFSETOF_SELF), %rax /* XAX now contains the user-defined interrupt */)
L(    movq  INTERRUPT_OFFSETOF_CALLBACK(%rax), %rax /* XAX now contains the interrupt entry point */)
L(    xchgq 0(%rsp), %rax /* Restore original XAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  swapgs /* Restore the user-space GS */                                  )
L(    addq $8, %rsp /* intno */                                               )
L(    __ASM_IRET                                                              )
L(1:  pushq $4f /* } else { ... */                                            )
L(    pushq %rax                                                              )
L(    movq 16(%rsp), %rax /* intno*INTERRUPT_SIZE */                          )
L(    addq  ASM_CPU(CPU_OFFSETOF_SELF), %rax /* RAX now contains the user-defined interrupt */)
L(    movq  INTERRUPT_OFFSETOF_CALLBACK(%rax), %rax /* RAX now contains the interrupt entry point */)
L(    xchgq 0(%rsp), %rax /* Restore original RAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  addq $8, %rsp /* intno */                                               )
L(    __ASM_IRET /*  } */                                                     )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    pushl $4f /* return_address */                                          )
L(    pushl %eax                                                              )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    movl (SGREGS_SIZE+8)(%esp), %eax /* intno*INTERRUPT_SIZE */             )
L(    addl  ASM_CPU(CPU_OFFSETOF_SELF), %eax /* EAX now contains the user-defined interrupt */)
L(    movl  INTERRUPT_OFFSETOF_CALLBACK(%eax), %eax /* EAX now contains the interrupt entry point */)
L(    xchgl 0(%esp), %eax /* Restore original EAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  __ASM_POP_SGREGS                                                        )
L(    addl $4, %esp /* intno */                                               )
L(    __ASM_IRET                                                              )
#endif
L(SYM_END(user_irq_ncode)                                                     )
L(                                                                            )
L(PRIVATE_ENTRY(user_irq_ycode)                                               )
#ifdef __x86_64__
L(    testq $0x3, (8+IRREGS_HOST_E_OFFSETOF_CS)(%esp)                         )
L(    jz 1f /* if (ORIGINATES_FROM_USERSPACE()) { ... */                      )
L(    swapgs                                                                  )
L(    pushq $4f /* return_address */                                          )
L(    pushq %rax                                                              )
L(    movq 16(%rsp), %rax /* intno*INTERRUPT_SIZE */                          )
L(    addq  ASM_CPU(CPU_OFFSETOF_SELF), %rax /* XAX now contains the user-defined interrupt */)
L(    movq  INTERRUPT_OFFSETOF_CALLBACK(%rax), %rax /* XAX now contains the interrupt entry point */)
L(    xchgq 0(%rsp), %rax /* Restore original XAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  swapgs /* Restore the user-space GS */                                  )
L(    addq $16, %rsp /* ecx_code + intno */                                   )
L(    __ASM_IRET                                                              )
L(1:  pushq $4f /* } else { ... */                                            )
L(    pushq %rax                                                              )
L(    movq 16(%rsp), %rax /* intno*INTERRUPT_SIZE */                          )
L(    addq  ASM_CPU(CPU_OFFSETOF_SELF), %rax /* RAX now contains the user-defined interrupt */)
L(    movq  INTERRUPT_OFFSETOF_CALLBACK(%rax), %rax /* RAX now contains the interrupt entry point */)
L(    xchgq 0(%rsp), %rax /* Restore original RAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  addq $16, %rsp /* ecx_code + intno */                                   )
L(    __ASM_IRET /*  } */                                                     )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    pushl $4f /* return_address */                                          )
L(    pushl %eax                                                              )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    movl (8+SGREGS_SIZE)(%esp), %eax /* intno*INTERRUPT_SIZE */             )
L(    addl  ASM_CPU(CPU_OFFSETOF_SELF), %eax /* EAX now contains the user-defined interrupt */)
L(    movl  INTERRUPT_OFFSETOF_CALLBACK(%eax), %eax /* EAX now contains the interrupt entry point */)
L(    xchgl 0(%esp), %eax /* Restore original EAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  __ASM_POP_SGREGS                                                        )
L(    addl $8, %esp /* ecx_code + intno */                                    )
L(    __ASM_IRET                                                              )
#endif
L(SYM_END(user_irq_ycode)                                                     )
L(                                                                            )
L(PRIVATE_ENTRY(fast_irq_ncode)                                               )
L(    __ASM_PUSH_SCRATCH                                                      )
L(    movx __ASM_SCRATCH_SIZE(%xsp), %FASTCALL_REG1 /* intno*INTERRUPT_SIZE */)
#ifdef __x86_64__
L(    testq $0x3, (8+IRREGS_HOST_E_OFFSETOF_CS)(%esp)                         )
L(    jz 1f /* if (ORIGINATES_FROM_USERSPACE()) { ... */                      )
L(    swapgs                                                                  )
L(    addq ASM_CPU(CPU_OFFSETOF_SELF), %FASTCALL_REG1                         )
L(    call exec_fastirq                                                       )
L(    swapgs /* Restore the user-space GS */                                  )
L(    addq $8, %rsp /* intno */                                               )
L(    __ASM_IRET                                                              )
L(1:  addq ASM_CPU(CPU_OFFSETOF_SELF), %FASTCALL_REG1                         )
L(    call exec_fastirq                                                       )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    addl ASM_CPU(CPU_OFFSETOF_SELF), %FASTCALL_REG1                         )
L(    call exec_fastirq                                                       )
L(    __ASM_POP_SGREGS                                                        )
#endif
L(    __ASM_POP_SCRATCH                                                       )
L(    addx $(XSZ), %xsp                                                       )
L(    __ASM_IRET                                                              )
L(SYM_END(fast_irq_ncode)                                                     )
L(                                                                            )
/* Only need to safe scratch registers on fast-path interrupt handlers. */
L(PRIVATE_ENTRY(fast_irq_ycode)                                               )
L(    __ASM_PUSH_SCRATCH                                                      )
L(    movx __ASM_SCRATCH_SIZE(%xsp), %FASTCALL_REG1 /* intno*INTERRUPT_SIZE */)
#ifdef __x86_64__
L(    testq $0x3, (8+IRREGS_HOST_E_OFFSETOF_CS)(%esp)                         )
L(    jz 1f /* if (ORIGINATES_FROM_USERSPACE()) { ... */                      )
L(    swapgs                                                                  )
L(    addq ASM_CPU(CPU_OFFSETOF_SELF), %FASTCALL_REG1                         )
L(    call exec_fastirq                                                       )
L(    swapgs /* Restore the user-space GS */                                  )
L(    addq $16, %rsp /* intno */                                              )
L(    __ASM_IRET                                                              )
L(1:  addq ASM_CPU(CPU_OFFSETOF_SELF), %FASTCALL_REG1                         )
L(    call exec_fastirq                                                       )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    addl ASM_CPU(CPU_OFFSETOF_SELF), %FASTCALL_REG1                         )
L(    call exec_fastirq                                                       )
L(    __ASM_POP_SGREGS                                                        )
#endif
L(    __ASM_POP_SCRATCH                                                       )
L(    addx $(2*XSZ), %xsp                                                     )
L(    __ASM_IRET                                                              )
L(SYM_END(fast_irq_ycode)                                                     )
L(                                                                            )
L(PRIVATE_ENTRY(stat_irq_ncode)                                               )
L(                                                                            )
L(    addx $(XSZ), %xsp                                                       )
L(    __ASM_IRET                                                              )
L(SYM_END(stat_irq_ncode)                                                     )
L(                                                                            )
L(PRIVATE_ENTRY(stat_irq_ycode)                                               )
L(                                                                            )
L(    addx $(2*XSZ), %xsp                                                     )
L(    __ASM_IRET                                                              )
L(SYM_END(stat_irq_ycode)                                                     )
L(                                                                            )
L(.previous                                                                   )
);

PRIVATE ATTR_USED void FCALL
exec_fastirq(struct interrupt *__restrict handler) {
}

PRIVATE ATTR_USED void FCALL
exec_statirq(struct interrupt *__restrict handler,
             struct cpustate_i *__restrict state) {
}



/* Update the interrupt descriptor table entry for `intno' */
PRIVATE void KCALL
receive_rpc_update_idt(irq_t intno) {
 struct idt_pointer idt_ptr;
 struct idtentry *idt; struct interrupt *ent;
 uintptr_t handler_eip = DIRQ_ENTRY(intno);
 pflag_t was = PREEMPTION_PUSH();
 IFDEF_SMP(atomic_rwlock_read(&inttab_lock));
 idt = &CPU(inttab).it_idt[intno];
 ent = CPU(inttab).it_tab[intno].e_head;
 if (ent) {
  if ((ent->i_type&INTTYPE_MASK) == (INTTYPE_ASM&INTTYPE_MASK)) {
   /* Directly bind an assembly interrupt handler. */
   handler_eip = (uintptr_t)ent->i_callback;
  } else {
   inttype_t min_level = (ent->i_type&INTTYPE_MASK);
   struct interrupt *other = ent;
   while ((other = other->i_link.le_next) != NULL) {
    inttype_t other_level = (other->i_type&INTTYPE_MASK);
    if (min_level < other_level) min_level = other_level;
   }
   if (min_level == (INTTYPE_ASM_SEG&INTTYPE_MASK)) {
    /* Bind a segment-safe assembly handler. */
    handler_eip = ASEG_ENTRY(intno);
   } else if (min_level == (INTTYPE_FAST&INTTYPE_MASK)) {
    /* Bind a fast c-function handler. */
    handler_eip = FAST_ENTRY(intno);
   }
  }
 }
 idt->ie_off1 = (u16)handler_eip;
 idt->ie_sel = __KERNEL_CS;
#ifdef __x86_64__
 idt->ie_ist = 0;
#else
 idt->ie_zero = 0;
#endif
 idt->ie_flags = ent ? ent->i_mode : (intno < 32 ? INTMODE_USER : INTMODE_HOST);
 idt->ie_off2 = (u16)(handler_eip >> 16);
#ifdef __x86_64__
 idt->ie_off3 = (u32)(handler_eip >> 32);
 idt->ie_unused = 0;
#endif

 /* Reload the IDT */
 idt_ptr.ip_idt   = CPU(inttab).it_idt;
 idt_ptr.ip_limit = sizeof(inttab.it_idt)-1;
 COMPILER_WRITE_BARRIER();
 __asm__ __volatile__("lidt %0\n" : : "m" (idt_ptr));
 IFDEF_SMP(atomic_rwlock_endread(&inttab_lock));
 PREEMPTION_POP(was);
}



/* --------------- End of ARCH-dependent code --------------- */

#ifdef CONFIG_SMP
LOCAL bool KCALL int_is_unused(irq_t intno, cpu_set_t *__restrict affinity) {
 struct cpu *c;
 FOREACH_CPU(c) {
  if (!CPU_ISSET(c->c_id,affinity)) continue;
  if (VCPU(c,inttab).it_tab[intno].e_head)
      return false;
 }
 return true;
}
LOCAL bool KCALL int_is_unused2(irq_t intno, cpu_set_t *__restrict affinity) {
 struct cpu *c;
 FOREACH_CPU(c) {
  struct interrupt *entry;
  if (!CPU_ISSET(c->c_id,affinity)) continue;
  entry = VCPU(c,inttab).it_tab[intno].e_head;
  assert(entry);
  if (entry->i_type&INTTYPE_NOSHARE)
      return false;
 }
 return true;
}
#else
#define int_is_unused(intno,affinity) \
  (!CPU(inttab).it_tab[intno].e_head)
#define int_is_unused2(intno,affinity) \
 (!(CPU(inttab).it_tab[intno].e_head->i_type&INTTYPE_NOSHARE))
#endif

PRIVATE void KCALL
interrupt_delete(struct interrupt *__restrict entry) {
 if (entry->i_fini) (*entry->i_fini)(entry);
 INSTANCE_DECREF(entry->i_owner);
 if (entry->i_flags&INTFLAG_FREEDESCR)
     kfree(entry);
}

#ifdef CONFIG_SMP
PUBLIC ssize_t KCALL
int_add(struct interrupt *__restrict entry) {
 cpu_set_t affinity; CPU_ZERO(&affinity);
 return int_add_set(entry,&affinity);
}
PUBLIC ssize_t KCALL
int_add_set(struct interrupt *__restrict entry,
            cpu_set_t *__restrict affinity)
#else
PUBLIC ssize_t KCALL
int_add(struct interrupt *__restrict entry)
#endif
{
 struct cpu *c;
 struct interrupt *delete_chain = NULL;
 ssize_t result = 0;
 pflag_t was = PREEMPTION_PUSH();
 IFDEF_SMP(atomic_rwlock_write(&inttab_lock));
#ifdef CONFIG_SMP
 if (CPU_ISEMPTY(affinity))
     CPU_SET(THIS_CPU->c_id,affinity);
 else {
  FOREACH_CPU(c) {
   /* Make sure that the interrupt describes an existing CPU. */
   if (CPU_ISSET(c->c_id,affinity))
    goto cpu_exists;
  }
  result = -ESRCH;
  goto end;
 }
cpu_exists:
#endif
 if (entry->i_flags&INTFLAG_DYNINTNO) {
  /* Dynamically lookup an unused interrupt number. */
  result = 0xff;
  do if (int_is_unused(result,affinity))
         goto got_intno;
  while (result--);
  result = 0xff;
  do if (int_is_unused2(result,affinity))
         goto got_intno;
  while (result--);
  result = -ENOMEM;
  goto end;
got_intno:
  entry->i_intno = (irq_t)result;
 } else {
  /* Check for any prior uses of the specified interrupt number. */
  FOREACH_CPU(c) {
   struct interrupt *handler;
#ifdef CONFIG_SMP
   if (!CPU_ISSET(c->c_id,affinity)) continue;
#endif
   handler = VCPU(c,inttab).it_tab[entry->i_intno].e_head;
   /* No existing handler. - The entry can be used just like that. */
   if (handler) {
    /* Doesn't matter: The two handlers can share. */
    if ((handler->i_type&INTTYPE_NOSHARE) ||
        (entry->i_type&INTTYPE_NOSHARE)) {
     /* Never override primary handlers. */
     if (handler->i_flags&INTFLAG_PRIMARY) goto err_eexist;
     /* Don't override non-secondary handlers with non-primary. */
     if (!(handler->i_flags&INTFLAG_SECONDARY) &&
         !(entry->i_flags&INTFLAG_PRIMARY)) goto err_eexist;
    }
   }
  }
 }

 /* Add the interrupt handler to all affected CPUs. */
#ifndef CONFIG_SMP
 if (!INSTANCE_INCREF(entry->i_owner))
      goto err_eperm;
 result = 1;
#else
 result = 0;
 FOREACH_CPU(c) {
  if (!CPU_ISSET(c->c_id,affinity)) continue;
  ++result;
 }
 if (result) {
  if (!INSTANCE_INCREF_N(entry->i_owner,result))
       goto err_eperm;
 }
#endif

 FOREACH_CPU(c) {
  struct interrupt **piter,*iter;
#ifdef CONFIG_SMP
  if (!CPU_ISSET(c->c_id,affinity)) continue;
#endif
  piter = &VCPU(c,inttab).it_tab[entry->i_intno].e_head;
  if (entry->i_type&INTTYPE_NOSHARE ||
     ((iter = *piter) != NULL && iter->i_type&INTTYPE_NOSHARE)) {
   /* Override existing handlers. */
   delete_chain          = iter;
   entry->i_link.le_next = NULL;
  } else {
   while ((iter = *piter) != NULL && INTERRUPT_BEFORE(iter,entry))
           piter = &iter->i_link.le_next;
   /* Insert the new handler into this chain. */
   if ((entry->i_link.le_next = *piter) != NULL)
        entry->i_link.le_next->i_link.le_pself = &entry->i_link.le_next;
  }
  (*piter = entry)->i_link.le_pself = piter;
 }

#ifdef CONFIG_SMP
 /* TODO: Send an RPC command to all CPUs apart of `affinity' */
 if (CPU_ISSET(THIS_CPU->c_id,affinity))
#endif
 {
  receive_rpc_update_idt(entry->i_intno);
 }

end:
 IFDEF_SMP(atomic_rwlock_endwrite(&inttab_lock));
 PREEMPTION_POP(was);
 /* Delete any interrupts that were overwritten. */
 while (delete_chain) {
  entry = delete_chain->i_link.le_next;
  interrupt_delete(delete_chain);
  delete_chain = entry;
 }
 return result;
err_eperm:  result = -EPERM; goto end;
err_eexist: result = -EEXIST; goto end;
}

PUBLIC ssize_t KCALL
int_del(struct interrupt *__restrict entry) {
 ssize_t result = 0;
 pflag_t was = PREEMPTION_PUSH();
 IFDEF_SMP(atomic_rwlock_write(&inttab_lock));

 PANIC("Not implemented");

 IFDEF_SMP(atomic_rwlock_endwrite(&inttab_lock));
 PREEMPTION_POP(was);
 return result;
}

#ifdef CONFIG_SMP
PUBLIC int KCALL
int_register(struct interrupt const *__restrict entry) {
 cpu_set_t affinity; CPU_ZERO(&affinity);
 return int_register_set(entry,&affinity);
}
PUBLIC int KCALL
int_register_set(struct interrupt const *__restrict entry,
                 cpu_set_t *__restrict affinity)
#else
PUBLIC int KCALL
int_register(struct interrupt const *__restrict entry)
#endif
{
 struct interrupt *copy; ssize_t result;
 copy = (struct interrupt *)memdup(entry,sizeof(struct interrupt));
 if unlikely(!copy) return -ENOMEM;
 copy->i_flags |= INTFLAG_FREEDESCR;
 result = int_add(copy);
 /* Return the interrupt number that was assigned. */
 if (E_ISERR(result)) free(copy);
 else result = copy->i_intno;
 return (int)result;
}

INTERN ATTR_FREETEXT void KCALL
cpu_interrupt_initialize(struct cpu *__restrict c) {
 struct idtentry *iter,*end; uintptr_t entry;
 struct interrupt_table *tab = &VCPU(c,inttab);
 end = (iter = tab->it_idt)+COMPILER_LENOF(tab->it_idt);
 entry = DIRQ_ENTRY(0);
 for (; iter != end; ++iter,entry += sizeof(struct dirq_asm)) {
  iter->ie_off1 = (u16)entry;
  iter->ie_sel = __KERNEL_CS;
#ifdef __x86_64__
  iter->ie_ist = 0;
#else
  iter->ie_zero = 0;
#endif
  iter->ie_flags = ((iter-tab->it_idt) < 32 ? INTMODE_USER : INTMODE_HOST);
  iter->ie_off2  = (u16)(entry >> 16);
#ifdef __x86_64__
  iter->ie_off3  = (u32)(entry >> 32);
  iter->ie_unused = 0;
#endif
 }
}


INTERN ATTR_FREETEXT void KCALL interrupt_initialize(void) {
 struct idt_pointer idt_ptr;
 /* Initialize the default IDT vector. */
 cpu_interrupt_initialize(BOOTCPU);
 /* Simply set the  */
 idt_ptr.ip_idt   = VCPU(BOOTCPU,inttab).it_idt;
 idt_ptr.ip_limit = sizeof(inttab.it_idt)-1;
 COMPILER_WRITE_BARRIER();
 __asm__ __volatile__("lidt %0\n" : : "m" (idt_ptr));
}



DECL_END

#endif /* !GUARD_KERNEL_CORE_ARCH_INTERRUPT_C */
