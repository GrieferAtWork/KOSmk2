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
#ifndef GUARD_KERNEL_ARCH_INTERRUPT_C
#define GUARD_KERNEL_ARCH_INTERRUPT_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <hybrid/debug.h>
#include <arch/cpustate.h>
#include <arch/gdt.h>
#include <arch/idt_pointer.h>
#include <kernel/interrupt.h>
#include <asm/instx.h>
#include <sched/percpu.h>
#include <hybrid/asm.h>
#include <hybrid/traceback.h>
#include <hybrid/list/list.h>
#include <malloc.h>
#include <kernel/malloc.h>
#include <arch/preemption.h>
#include <sched.h>
#include <sched/smp.h>
#include <sched/paging.h>
#include <linker/module.h>
#include <hybrid/panic.h>
#include <sched/smp.h>
#include <sched/cpu.h>
#include <kernel/irq.h>
#include <asm/registers.h>
#include <syslog.h>
#include <kernel/paging.h>
#include <sys/io.h>
#include <kernel/boot.h>
#include <string.h>
#include <kernel/stack.h>
#include <arch/pic.h>
#include <sched/types.h>

#include "interrupt_intern.h"
#include <hybrid/minmax.h>

DECL_BEGIN

STATIC_ASSERT(offsetof(struct interrupt,i_intno) == INTERRUPT_OFFSETOF_INTNO);
STATIC_ASSERT(offsetof(struct interrupt,i_mode) == INTERRUPT_OFFSETOF_MODE);
STATIC_ASSERT(offsetof(struct interrupt,i_type) == INTERRUPT_OFFSETOF_TYPE);
STATIC_ASSERT(offsetof(struct interrupt,i_prio) == INTERRUPT_OFFSETOF_PRIO);
STATIC_ASSERT(offsetof(struct interrupt,i_flags) == INTERRUPT_OFFSETOF_FLAGS);
STATIC_ASSERT(offsetof(struct interrupt,i_callback) == INTERRUPT_OFFSETOF_CALLBACK);
STATIC_ASSERT(offsetof(struct interrupt,i_closure) == INTERRUPT_OFFSETOF_CLOSURE);
STATIC_ASSERT(offsetof(struct interrupt,i_owner) == INTERRUPT_OFFSETOF_OWNER);
STATIC_ASSERT(offsetof(struct interrupt,i_fini) == INTERRUPT_OFFSETOF_FINI);
STATIC_ASSERT(offsetof(struct interrupt,i_hits) == INTERRUPT_OFFSETOF_HITS);
STATIC_ASSERT(offsetof(struct interrupt,i_miss) == INTERRUPT_OFFSETOF_MISS);
STATIC_ASSERT(offsetof(struct interrupt,i_link) == INTERRUPT_OFFSETOF_LINK);
STATIC_ASSERT(sizeof(struct interrupt) == INTERRUPT_SIZE);
STATIC_ASSERT(sizeof(struct idtentry) == IDTENTRY_SIZE);
STATIC_ASSERT(offsetof(struct interrupt_table,it_tab) == 256*IDTENTRY_SIZE);

INTERN CPU_BSS struct PACKED interrupt_table inttab;
GLOBAL_ASM(
L(SYM_PUBLIC(intno_offset)                         )
L(intno_offset = inttab+(256*IDTENTRY_SIZE)        )
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
INTDEF struct dirq_asm basi_tab[256]; /* Table of basic c-function IRQ entry points. */
INTDEF struct dirq_asm stat_tab[256]; /* Table of full cpu-state IRQ entry points. */
#define DIRQ_ENTRY(intno) ((uintptr_t)dirq_tab[intno].entry_point)
#define ASEG_ENTRY(intno) ((uintptr_t)aseg_tab[intno].entry_point)
#define FAST_ENTRY(intno) ((uintptr_t)fast_tab[intno].entry_point)
#define BASI_ENTRY(intno) ((uintptr_t)basi_tab[intno].entry_point)
#define STAT_ENTRY(intno) ((uintptr_t)stat_tab[intno].entry_point)


/*[[[deemon
local ycode = [8,10,11,12,13,14,17,30];
local spuri = [0x20+7,0x28+7];
function x(n) -> (n in spuri ? "spur" : n in ycode ? "yirq" : "nirq")+("(0x%.2I8x)" % n);
print "#define IRQ_TABLE(nirq,yirq,spur) \\";
for (local i = 0; i < 32; ++i) {
    print " {} {} {} {} {} {} {} {} {}"
          .format({ x(i*8+0),x(i*8+1),x(i*8+2),x(i*8+3),
                    x(i*8+4),x(i*8+5),x(i*8+6),x(i*8+7),
                   (i == 31 ? "" : "\\"),
          });
}
]]]*/
#define IRQ_TABLE(nirq,yirq,spur) \
 nirq(0x00) nirq(0x01) nirq(0x02) nirq(0x03) nirq(0x04) nirq(0x05) nirq(0x06) nirq(0x07) \
 yirq(0x08) nirq(0x09) yirq(0x0a) yirq(0x0b) yirq(0x0c) yirq(0x0d) yirq(0x0e) nirq(0x0f) \
 nirq(0x10) yirq(0x11) nirq(0x12) nirq(0x13) nirq(0x14) nirq(0x15) nirq(0x16) nirq(0x17) \
 nirq(0x18) nirq(0x19) nirq(0x1a) nirq(0x1b) nirq(0x1c) nirq(0x1d) yirq(0x1e) nirq(0x1f) \
 nirq(0x20) nirq(0x21) nirq(0x22) nirq(0x23) nirq(0x24) nirq(0x25) nirq(0x26) spur(0x27) \
 nirq(0x28) nirq(0x29) nirq(0x2a) nirq(0x2b) nirq(0x2c) nirq(0x2d) nirq(0x2e) spur(0x2f) \
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



#ifdef CONFIG_DEBUG
#define FRAME_SIZE          (2*XSZ)
#define PUSH_FRAME(xip_offset) pushx xip_offset(%xsp); pushx %xbp; movx %xsp, %xbp
#define POP_FRAME              popx %xbp; addx $(XSZ), %xsp
#define SKIP_FRAME             addx $(2*XSZ), %xsp
#else
#define FRAME_SIZE             0
#define PUSH_FRAME(xip_offset) /* nothing */
#define POP_FRAME              /* nothing */
#define SKIP_FRAME             /* nothing */
#endif


GLOBAL_ASM(
/* TODO: The bytecode below is only tested on x86_64 (i386 may use different bytes?) */
#define DIRQ(n,target) \
   .byte 0x68; .long MOD(n);           /* pushq $(MOD(n)) */ \
   .byte 0xe9; .long target - 97f; 97: /* jmp target */
L(.section .text                                                              )
#define MOD(n)    ASM_IRREGS_ENCODE_INTNO(n)
#define YIRQ(n) L(DIRQ(n,default_irq_ycode))
#define NIRQ(n) L(DIRQ(n,default_irq_ncode))
#define SPUR(n) L(DIRQ(n,default_irq_spuri))
L(PRIVATE_ENTRY(dirq_tab)                                                     )
IRQ_TABLE(NIRQ,YIRQ,SPUR)
L(SYM_END(dirq_tab)                                                           )
#undef SPUR
#undef YIRQ
#undef NIRQ
#define YIRQ(n) L(DIRQ(n,user_irq_ycode))
#define NIRQ(n) L(DIRQ(n,user_irq_ncode))
#define SPUR(n) L(DIRQ(n,user_irq_spuri))
L(PRIVATE_ENTRY(aseg_tab)                                                     )
IRQ_TABLE(NIRQ,YIRQ,SPUR)
L(SYM_END(aseg_tab)                                                           )
#undef SPUR
#undef YIRQ
#undef NIRQ
#define YIRQ(n) L(DIRQ(n,fast_irq_ycode))
#define NIRQ(n) L(DIRQ(n,fast_irq_ncode))
#define SPUR(n) L(DIRQ(n,fast_irq_spuri))
L(PRIVATE_ENTRY(fast_tab)                                                     )
IRQ_TABLE(NIRQ,YIRQ,SPUR)
L(SYM_END(fast_tab)                                                           )
#undef SPUR
#undef YIRQ
#undef NIRQ
#define YIRQ(n) L(DIRQ(n,basi_irq_ycode))
#define NIRQ(n) L(DIRQ(n,basi_irq_ncode))
#define SPUR(n) L(DIRQ(n,basi_irq_spuri))
L(PRIVATE_ENTRY(basi_tab)                                                     )
IRQ_TABLE(NIRQ,YIRQ,SPUR)
L(SYM_END(basi_tab)                                                           )
#undef SPUR
#undef YIRQ
#undef NIRQ
#define YIRQ(n) L(DIRQ(n,stat_irq_ycode))
#define NIRQ(n) L(DIRQ(n,stat_irq_ncode))
#define SPUR(n) L(DIRQ(n,stat_irq_spuri))
L(PRIVATE_ENTRY(stat_tab)                                                     )
IRQ_TABLE(NIRQ,YIRQ,SPUR)
L(SYM_END(stat_tab)                                                           )
#undef SPUR
#undef YIRQ
#undef NIRQ
#undef MOD
L(.previous                                                                   )
);

#define ADJUST_INTNO_TO_INTERRUPT(reg) /* reg = intno*INTERRUPT_SIZE. */ \
 addx ASM_CPU(CPU_OFFSETOF_SELF), reg; /* reg = CPU+intno*INTERRUPT_SIZE. */ \
 movx                      (reg), reg; /* reg = interrupt_handler. */


GLOBAL_ASM(
L(.section .text.hot                                                          )
L(PRIVATE_ENTRY(default_irq_spuri)                                            )
L(    call check_spuri                                                        )
L(PRIVATE_ENTRY(default_irq_ncode)                                            )
L(    cld /* Clear the direction bit in XFLAGS. */                            )
L(    __ASM_PUSH_COMREGS                                                      )
L(    movx %xsp, %FASTCALL_REG1 /* state */                                   )
L(    PUSH_FRAME(CPUSTATE_IE_OFFSETOF_IRET+IRREGS_I_OFFSETOF_IP)              )
#ifdef __x86_64__
L(    testq $3, FRAME_SIZE+CPUSTATE_I_OFFSETOF_IRET+IRREGS_I_OFFSETOF_CS(%rsp))
L(    jz 1f                                                                   )
L(    swapgs                                                                  )
L(    call exec_deflirq                                                       )
L(    SKIP_FRAME                                                              )
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs /* Restore the user-space GS */                                  )
L(    __ASM_POP_COMREGS                                                       )
L(    addq $8, %rsp /* intno */                                               )
L(    ASM_IRET                                                                )
L(1:  call exec_deflirq                                                       )
#else
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    call exec_deflirq                                                       )
#endif
L(    SKIP_FRAME                                                              )
L(    __ASM_POP_COMREGS                                                       )
L(    addx $(XSZ), %xsp /* intno */                                           )
L(    ASM_IRET                                                                )
L(SYM_END(default_irq_ncode)                                                  )
L(SYM_END(default_irq_spuri)                                                  )
L(                                                                            )
L(PRIVATE_ENTRY(default_irq_ycode)                                            )
L(    cld /* Clear the direction bit in XFLAGS. */                            )
L(    __ASM_PUSH_COMREGS                                                      )
L(    movx %xsp, %FASTCALL_REG1 /* state */                                   )
L(    PUSH_FRAME(CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_IP)             )
#ifdef __x86_64__
L(    testq $3, FRAME_SIZE+CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_CS(%rsp))
L(    jz 1f                                                                   )
L(    swapgs                                                                  )
L(    call exec_deflirq                                                       )
L(    SKIP_FRAME                                                              )
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs /* Restore the user-space GS */                                  )
L(    __ASM_POP_COMREGS                                                       )
L(    addq $8, %rsp /* intno */                                               )
L(    ASM_IRET                                                                )
L(1:  call exec_deflirq                                                       )
#else
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    call exec_deflirq                                                       )
#endif
L(    SKIP_FRAME                                                              )
L(    __ASM_POP_COMREGS                                                       )
L(    addx $(2*XSZ), %xsp /* exc_code + intno */                              )
L(    ASM_IRET                                                                )
L(SYM_END(default_irq_ycode)                                                  )
L(                                                                            )
L(.previous                                                                   )
);


GLOBAL_ASM(
L(.section .text.hot                                                          )
L(PRIVATE_ENTRY(user_irq_spuri)                                               )
L(    call check_spuri                                                        )
L(PRIVATE_ENTRY(user_irq_ncode)                                               )
#ifdef __x86_64__
L(    testq $0x3, IRREGS_I_OFFSETOF_CS(%rsp)                                  )
L(    jz 1f /* if (ORIGINATES_FROM_USERSPACE()) { ... */                      )
L(    swapgs                                                                  )
L(    pushq $4f /* return_address */                                          )
L(    pushq %rax                                                              )
L(    movq 16(%rsp), %rax /* intno */                                         )
L(    ADJUST_INTNO_TO_INTERRUPT(%rax)                                         )
L(    movq  INTERRUPT_OFFSETOF_CALLBACK(%rax), %rax /* XAX now contains the interrupt entry point */)
L(    xchgq 0(%rsp), %rax /* Restore original XAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs /* Restore the user-space GS */                                  )
L(    addq $8, %rsp /* intno */                                               )
L(    ASM_IRET                                                                )
L(1:  pushq $4f /* } else { ... */                                            )
L(    pushq %rax                                                              )
L(    movq 16(%rsp), %rax /* intno */                                         )
L(    ADJUST_INTNO_TO_INTERRUPT(%rax)                                         )
L(    movq  INTERRUPT_OFFSETOF_CALLBACK(%rax), %rax /* RAX now contains the interrupt entry point */)
L(    xchgq 0(%rsp), %rax /* Restore original RAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  addq $8, %rsp /* intno */                                               )
L(    ASM_IRET /*  } */                                                     )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    pushl $4f /* return_address */                                          )
L(    pushl %eax                                                              )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    movl (SGREGS_SIZE+8)(%esp), %eax /* intno */                            )
L(    ADJUST_INTNO_TO_INTERRUPT(%eax)                                         )
L(    movl  INTERRUPT_OFFSETOF_CALLBACK(%eax), %eax /* EAX now contains the interrupt entry point */)
L(    xchgl 0(%esp), %eax /* Restore original EAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  __ASM_POP_SGREGS                                                        )
L(    addl $4, %esp /* intno */                                               )
L(    ASM_IRET                                                                )
#endif
L(SYM_END(user_irq_ncode)                                                     )
L(SYM_END(user_irq_spuri)                                                     )
L(                                                                            )
L(PRIVATE_ENTRY(user_irq_ycode)                                               )
#ifdef __x86_64__
L(    testq $0x3, IRREGS_IE_OFFSETOF_CS(%rsp)                                 )
L(    jz 1f /* if (ORIGINATES_FROM_USERSPACE()) { ... */                      )
L(    swapgs                                                                  )
L(    pushq $4f /* return_address */                                          )
L(    pushq %rax                                                              )
L(    movq 16(%rsp), %rax /* intno */                                         )
L(    ADJUST_INTNO_TO_INTERRUPT(%rax)                                         )
L(    movq  INTERRUPT_OFFSETOF_CALLBACK(%rax), %rax /* XAX now contains the interrupt entry point */)
L(    xchgq 0(%rsp), %rax /* Restore original XAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs /* Restore the user-space GS */                                  )
L(    addq $16, %rsp /* ecx_code + intno */                                   )
L(    ASM_IRET                                                                )
L(1:  pushq $4f /* } else { ... */                                            )
L(    pushq %rax                                                              )
L(    movq 16(%rsp), %rax /* intno */                                         )
L(    ADJUST_INTNO_TO_INTERRUPT(%rax)                                         )
L(    movq  INTERRUPT_OFFSETOF_CALLBACK(%rax), %rax /* RAX now contains the interrupt entry point */)
L(    xchgq 0(%rsp), %rax /* Restore original RAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  addq $16, %rsp /* ecx_code + intno */                                   )
L(    ASM_IRET /*  } */                                                     )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    pushl $4f /* return_address */                                          )
L(    pushl %eax                                                              )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    movl (8+SGREGS_SIZE)(%esp), %eax /* intno */                            )
L(    ADJUST_INTNO_TO_INTERRUPT(%eax)                                         )
L(    movl  INTERRUPT_OFFSETOF_CALLBACK(%eax), %eax /* EAX now contains the interrupt entry point */)
L(    xchgl 0(%esp), %eax /* Restore original EAX and safe interrupt entry point on-stack. */)
L(    ret /* ~return~ to the interrupt handler. */                            )
L(4:  __ASM_POP_SGREGS                                                        )
L(    addl $8, %esp /* ecx_code + intno */                                    )
L(    ASM_IRET                                                                )
#endif
L(SYM_END(user_irq_ycode)                                                     )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text.hot                                                          )
L(PRIVATE_ENTRY(fast_irq_spuri)                                               )
L(    call check_spuri                                                        )
L(PRIVATE_ENTRY(fast_irq_ncode)                                               )
L(    cld /* Clear the direction bit in XFLAGS. */                            )
L(    __ASM_PUSH_SCRATCH                                                      )
L(    movx __ASM_SCRATCH_SIZE(%xsp), %FASTCALL_REG1 /* intno */               )
L(    PUSH_FRAME(__ASM_SCRATCH_SIZE+IRREGS_I_OFFSETOF_IP)                     )
#ifdef __x86_64__
L(    testq $0x3, FRAME_SIZE+__ASM_SCRATCH_SIZE+IRREGS_I_OFFSETOF_CS(%rsp)    )
L(    jz    1f /* if (ORIGINATES_FROM_USERSPACE()) { ... */                   )
L(    swapgs                                                                  )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call   exec_fastirq                                                     )
L(    POP_FRAME                                                               )
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs /* Restore the user-space GS */                                  )
L(    testq  %rax, %rax                                                       )
L(    jz     2f                                                               )
L(    __ASM_POP_SCRATCH                                                       )
L(    addq   $8, %rsp /* intno */                                             )
L(    ASM_IRET                                                                )
L(2:  __ASM_POP_SCRATCH                                                       )
L(    jmp   default_irq_ncode                                                 )
L(1:  ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call  exec_fastirq                                                      )
L(    POP_FRAME                                                               )
L(    testq %rax, %rax                                                        )
L(    jz    2b                                                                )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call  exec_fastirq                                                      )
L(    __ASM_POP_SGREGS                                                        )
L(    POP_FRAME                                                               )
L(    testl %eax, %eax                                                        )
L(    jz    1f                                                                )
#endif
L(    __ASM_POP_SCRATCH                                                       )
L(    addx $(XSZ), %xsp                                                       )
L(    ASM_IRET                                                                )
#ifndef __x86_64__
L(1:  __ASM_POP_SCRATCH                                                       )
L(    jmp   default_irq_ncode                                                 )
#endif
L(SYM_END(fast_irq_ncode)                                                     )
L(SYM_END(fast_irq_spuri)                                                     )
L(                                                                            )
L(PRIVATE_ENTRY(fast_irq_ycode)                                               )
/* Only need to safe scratch registers on fast-path interrupt handlers. */
L(    cld /* Clear the direction bit in XFLAGS. */                            )
L(    __ASM_PUSH_SCRATCH                                                      )
L(    movx __ASM_SCRATCH_SIZE(%xsp), %FASTCALL_REG1 /* intno */               )
L(    PUSH_FRAME(__ASM_SCRATCH_SIZE+IRREGS_I_OFFSETOF_IP)                     )
#ifdef __x86_64__
L(    testq $0x3, __ASM_SCRATCH_SIZE+IRREGS_IE_OFFSETOF_CS(%rsp)              )
L(    jz    1f /* if (ORIGINATES_FROM_USERSPACE()) { ... */                   )
L(    swapgs                                                                  )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call exec_fastirq                                                       )
L(    POP_FRAME                                                               )
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs /* Restore the user-space GS */                                  )
L(    testq  %rax, %rax                                                       )
L(    jz     2f                                                               )
L(    __ASM_POP_SCRATCH                                                       )
L(    addq $16, %rsp /* intno */                                              )
L(    ASM_IRET                                                                )
L(2:  __ASM_POP_SCRATCH                                                       )
L(    jmp   default_irq_ycode                                                 )
L(1:  ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call exec_fastirq                                                       )
L(    POP_FRAME                                                               )
L(    testq  %rax, %rax                                                       )
L(    jz     2b                                                               )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call exec_fastirq                                                       )
L(    __ASM_POP_SGREGS                                                        )
L(    POP_FRAME                                                               )
L(    testl %eax, %eax                                                        )
L(    jz    1f                                                                )
#endif
L(    __ASM_POP_SCRATCH                                                       )
L(    addx $(2*XSZ), %xsp                                                     )
L(    ASM_IRET                                                                )
#ifndef __x86_64__
L(1:  __ASM_POP_SCRATCH                                                       )
L(    jmp   default_irq_ycode                                                 )
#endif
L(SYM_END(fast_irq_ycode)                                                     )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text.hot                                                          )
L(PRIVATE_ENTRY(basi_irq_spuri)                                               )
L(    call check_spuri                                                        )
L(PRIVATE_ENTRY(basi_irq_ncode)                                               )
L(    cld /* Clear the direction bit in XFLAGS. */                            )
L(    __ASM_PUSH_SCRATCH                                                      )
L(    movx   __ASM_SCRATCH_SIZE(%xsp), %FASTCALL_REG1 /* intno */             )
L(    leax   __ASM_SCRATCH_SIZE(%xsp), %FASTCALL_REG2 /* struct irregs_i * */ )
L(    PUSH_FRAME(__ASM_SCRATCH_SIZE+IRREGS_I_OFFSETOF_IP)                     )
#ifdef __x86_64__
L(    testq  $0x3, FRAME_SIZE+__ASM_SCRATCH_SIZE+IRREGS_I_OFFSETOF_CS(%rsp)   )
L(    jz     1f /* if (ORIGINATES_FROM_USERSPACE()) { ... */                  )
L(    swapgs                                                                  )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call   exec_basiirq                                                     )
L(    POP_FRAME                                                               )
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs /* Restore the user-space GS */                                  )
L(    testq  %rax, %rax                                                       )
L(    jz     2f                                                               )
L(    __ASM_POP_SCRATCH                                                       )
L(    addq   $8, %rsp /* intno */                                             )
L(    ASM_IRET                                                                )
L(2:  __ASM_POP_SCRATCH                                                       )
L(    jmp    default_irq_ncode                                                )
L(1:  ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call   exec_basiirq                                                     )
L(    POP_FRAME                                                               )
L(    testq  %rax, %rax                                                       )
L(    jz     2b                                                               )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call   exec_basiirq                                                     )
L(    __ASM_POP_SGREGS                                                        )
L(    POP_FRAME                                                               )
L(    testl  %eax, %eax                                                       )
L(    jz     1f                                                               )
#endif
L(    __ASM_POP_SCRATCH                                                       )
L(    addx   $(XSZ), %xsp /* intno */                                         )
L(    ASM_IRET                                                                )
#ifndef __x86_64__
L(1:  __ASM_POP_SCRATCH                                                       )
L(    jmp    default_irq_ncode                                                )
#endif
L(SYM_END(basi_irq_ncode)                                                     )
L(SYM_END(basi_irq_spuri)                                                     )
L(                                                                            )
L(PRIVATE_ENTRY(basi_irq_ycode)                                               )
/* Only need to safe scratch registers on basi-path interrupt handlers. */
L(    cld /* Clear the direction bit in XFLAGS. */                            )
L(    __ASM_PUSH_SCRATCH                                                      )
L(    movx   __ASM_SCRATCH_SIZE(%xsp), %FASTCALL_REG1 /* intno */             )
L(    leax   __ASM_SCRATCH_SIZE(%xsp), %FASTCALL_REG2 /* struct irregs_ie * */)
L(    PUSH_FRAME(__ASM_SCRATCH_SIZE+IRREGS_IE_OFFSETOF_IP)                    )
#ifdef __x86_64__
L(    testq  $0x3, FRAME_SIZE+__ASM_SCRATCH_SIZE+IRREGS_IE_OFFSETOF_CS(%rsp)  )
L(    jz     1f /* if (ORIGINATES_FROM_USERSPACE()) { ... */                  )
L(    swapgs                                                                  )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call   exec_basiirq                                                     )
L(    POP_FRAME                                                               )
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs /* Restore the user-space GS */                                  )
L(    testq  %rax, %rax                                                       )
L(    jz     2f                                                               )
L(    __ASM_POP_SCRATCH                                                       )
L(    addq   $16, %rsp /* intno */                                            )
L(    ASM_IRET                                                                )
L(2:  __ASM_POP_SCRATCH                                                       )
L(    jmp    default_irq_ycode                                                )
L(1:  ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call   exec_basiirq                                                     )
L(    POP_FRAME                                                               )
L(    testq  %rax, %rax                                                       )
L(    jz     2b                                                               )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call   exec_basiirq                                                     )
L(    __ASM_POP_SGREGS                                                        )
L(    POP_FRAME                                                               )
L(    testl  %eax, %eax                                                       )
L(    jz     1f                                                               )
#endif
L(    __ASM_POP_SCRATCH                                                       )
L(    addx   $(2*XSZ), %xsp /* intno, exc_code */                             )
L(    ASM_IRET                                                                )
#ifndef __x86_64__
L(1:  __ASM_POP_SCRATCH                                                       )
L(    jmp    default_irq_ycode                                                )
#endif
L(SYM_END(basi_irq_ycode)                                                     )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text.hot                                                          )
L(PRIVATE_ENTRY(stat_irq_spuri)                                               )
L(    call check_spuri                                                        )
L(PRIVATE_ENTRY(stat_irq_ncode)                                               )
L(    cld /* Clear the direction bit in XFLAGS. */                            )
L(    __ASM_PUSH_COMREGS                                                      )
L(    movx CPUSTATE_I_OFFSETOF_IRET+IRREGS_I_OFFSETOF_INTNO(%xsp), \
                 %FASTCALL_REG1 /* intno */                                   )
L(    movx %xsp, %FASTCALL_REG2 /* state */                                   )
L(    PUSH_FRAME(CPUSTATE_I_OFFSETOF_IRET+IRREGS_I_OFFSETOF_IP)               )
#ifdef __x86_64__
L(    testq $3, FRAME_SIZE+CPUSTATE_I_OFFSETOF_IRET+IRREGS_I_OFFSETOF_CS(%rsp))
L(    jz 1f                                                                   )
L(    swapgs                                                                  )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call exec_statirq                                                       )
L(    SKIP_FRAME                                                              )
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs /* Restore the user-space GS */                                  )
L(    __ASM_POP_COMREGS                                                       )
L(    addq $8, %rsp /* intno */                                               )
L(    ASM_IRET                                                                )
L(1:  ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call exec_statirq                                                       )
#else
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call exec_statirq                                                       )
#endif
L(    SKIP_FRAME                                                              )
L(    __ASM_POP_COMREGS                                                       )
L(    addx $(XSZ), %xsp /* intno */                                           )
L(    ASM_IRET                                                                )
L(SYM_END(stat_irq_ncode)                                                     )
L(SYM_END(stat_irq_spuri)                                                     )
L(                                                                            )
L(PRIVATE_ENTRY(stat_irq_ycode)                                               )
L(    cld /* Clear the direction bit in XFLAGS. */                            )
L(    __ASM_PUSH_COMREGS                                                      )
L(    movx CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_INTNO(%xsp), \
                 %FASTCALL_REG1 /* intno */                                   )
L(    movx %xsp, %FASTCALL_REG2 /* state */                                   )
L(    PUSH_FRAME(CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_IP)             )
#ifdef __x86_64__
L(    testq $3, FRAME_SIZE+CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_CS(%rsp))
L(    jz 1f                                                                   )
L(    swapgs                                                                  )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call exec_statirq                                                       )
L(    SKIP_FRAME                                                              )
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs /* Restore the user-space GS */                                  )
L(    __ASM_POP_COMREGS                                                       )
L(    addq $8, %rsp /* intno */                                               )
L(    ASM_IRET                                                                )
L(1:  ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call exec_statirq                                                       )
#else
L(    __ASM_LOAD_SEGMENTS(%ax)                                                )
L(    ADJUST_INTNO_TO_INTERRUPT(%FASTCALL_REG1)                               )
L(    call exec_statirq                                                       )
#endif
L(    SKIP_FRAME                                                              )
L(    __ASM_POP_COMREGS                                                       )
L(    addx $(2*XSZ), %xsp /* exc_code + intno */                              )
L(    ASM_IRET                                                                )
L(SYM_END(stat_irq_ycode)                                                     )
L(                                                                            )
L(.previous                                                                   )
);


PUBLIC struct spurious_pic pic_spurious = {0,0};
PRIVATE ATTR_USED void KCALL pic1_is_spurious(void) {
 u32 num = ATOMIC_INCFETCH(pic_spurious.sp_pic1);
 syslog(LOG_HW|LOG_WARN,
        COLDSTR("[IRQ] Ignoring spurious interrupt on PIC #1 (#%I32u)\n"),
        num);
}
PRIVATE ATTR_USED void KCALL pic2_is_spurious(void) {
 u32 num = ATOMIC_INCFETCH(pic_spurious.sp_pic1);
 syslog(LOG_HW|LOG_WARN,
        COLDSTR("[IRQ] Ignoring spurious interrupt on PIC #1 (#%I32u)\n"),
        num);
}

GLOBAL_ASM(
L(.section .text.hot                                                          )
L(PRIVATE_ENTRY(check_spuri)                                                  )
L(    pushx %xax                                                              )
L(    movb  $(PIC_READ_ISR), %al                                              )
L(    cmpx $(ASM_IRREGS_ENCODE_INTNO(INTNO_PIC1_LPT1)), \
             XSZ(%xsp) /* Check from which PIC this interrupt originates. */  )
L(    je 1f /* if (intno == INTNO_PIC1_LPT1) goto 2f; */                      )
L(    /* Check PIC2 */                                                        )
L(    outb  %al,         $(PIC2_CMD) /* outb(PIC2_CMD,PIC_READ_ISR); */       )
L(    inb   $(PIC2_CMD), %al                                                  )
L(    testb $0x80,       %al                                                  )
L(    jz    2f /* if unlikely(!(inb(PIC2_CMD) & 0x80)) goto 2f; */            )
L(    popx  %xax                                                              )
L(    ret                                                                     )
L(2:  __ASM_PUSH_SCRATCH_NOXAX                                                )
L(    call pic1_is_spurious                                                   )
L(    __ASM_POP_SCRATCH_NOXAX                                                 )
L(    jmp 99f                                                                 )
L(1:  /* Check PIC1 */                                                        )
L(    outb  %al,         $(PIC1_CMD) /* outb(PIC1_CMD,PIC_READ_ISR); */       )
L(    inb   $(PIC2_CMD), %al                                                  )
L(    testb $0x80,       %al                                                  )
L(    jz    2f /* if unlikely(!(inb(PIC2_CMD) & 0x80)) goto 2f; */            )
L(    popx  %xax                                                              )
L(    ret                                                                     )
L(2:  __ASM_PUSH_SCRATCH_NOXAX                                                )
L(    call pic2_is_spurious                                                   )
L(    __ASM_POP_SCRATCH_NOXAX                                                 )
L(99: popx  %xax                                                              )
L(    addx $(2*XSZ), %xsp /* return_addr + intno */                           )
L(    ASM_IRET                                                                )
L(SYM_END(check_spuri)                                                        )
L(.previous                                                                   )
);



#define PREV(x)  container_of((x)->i_link.le_pself,struct interrupt,i_link.le_next)
#define NEXT(x)              ((x)->i_link.le_next)

/* Fast C-level IRQ handler. */
PRIVATE ATTR_USED bool FCALL
exec_fastirq(struct interrupt *__restrict handler) {
 /* C-level IRQ handler with full CPU-state. */
 struct interrupt *iter = handler;
 assert(iter);
 for (;;) {
  int handler_code;
  assert((iter->i_type&INTTYPE_MASK) != (INTTYPE_ASM&INTTYPE_MASK));
  handler_code = (*iter->i_proto.p_fast_arg)(iter->i_closure);
  if unlikely(iter->i_type&INTTYPE_NOSHARE) goto ignored;
  switch (__builtin_expect(handler_code,INTCODE_HANDLED)) {
  case INTCODE_HANDLED:
   /* Track the number of hits. */
   ++iter->i_hits;
   /* Dynamically optimize the interrupt execution order. */
   if (iter != handler && !(iter->i_flags&(INTFLAG_PRIMARY|INTFLAG_SECONDARY)) &&
      (handler = PREV(iter),iter->i_hits > handler->i_hits &&
     !(handler->i_flags&INTFLAG_PRIMARY) && handler->i_prio == iter->i_prio)) {
    /* Swap the order of of `iter' and its predecessor (`handler') */
    if ((handler->i_link.le_next = iter->i_link.le_next) != NULL)
         handler->i_link.le_next->i_link.le_pself = &handler->i_link.le_next;
    iter->i_link.le_pself    = handler->i_link.le_pself;
    iter->i_link.le_next     = handler;
    handler->i_link.le_pself = &iter->i_link.le_next;
    *iter->i_link.le_pself   = iter;
   }
  case INTCODE_IGNORED: /* Ignore the interrupt and act as though it never happened. */
ignored:
   return true;
  default:
   ++iter->i_miss;
   break;
  }
  if ((iter = NEXT(iter)) == NULL) break;
 }
 /* Execute the default handler if the interrupt couldn't be processed. */
 return false;
}

/* Fast C-level IRQ handler. */
PRIVATE ATTR_USED bool FCALL
exec_basiirq(struct interrupt *__restrict handler,
             struct irregs_i *__restrict info) {
 /* C-level IRQ handler with full CPU-state. */
 struct interrupt *iter = handler;
 assert(iter);
 for (;;) {
  int handler_code;
  assert((iter->i_type&INTTYPE_MASK) != (INTTYPE_ASM&INTTYPE_MASK));
  if likely((iter->i_type&INTTYPE_MASK) == INTTYPE_FAST)
       handler_code = (*iter->i_proto.p_fast_arg)(iter->i_closure);
  else handler_code = (*iter->i_proto.p_basic_arg)(info,iter->i_closure);
  if (iter->i_type&INTTYPE_NOSHARE) goto ignored;
  switch (__builtin_expect(handler_code,INTCODE_HANDLED)) {
  case INTCODE_HANDLED:
   /* Track the number of hits. */
   ++iter->i_hits;
   /* Dynamically optimize the interrupt execution order. */
   if (iter != handler && !(iter->i_flags&(INTFLAG_PRIMARY|INTFLAG_SECONDARY)) &&
      (handler = PREV(iter),iter->i_hits > handler->i_hits &&
     !(handler->i_flags&INTFLAG_PRIMARY) && handler->i_prio == iter->i_prio)) {
    /* Swap the order of of `iter' and its predecessor (`handler') */
    if ((handler->i_link.le_next = iter->i_link.le_next) != NULL)
         handler->i_link.le_next->i_link.le_pself = &handler->i_link.le_next;
    iter->i_link.le_pself    = handler->i_link.le_pself;
    iter->i_link.le_next     = handler;
    handler->i_link.le_pself = &iter->i_link.le_next;
    *iter->i_link.le_pself   = iter;
   }
  case INTCODE_IGNORED: /* Ignore the interrupt and act as though it never happened. */
ignored:
   return true;
  default:
   ++iter->i_miss;
   break;
  }
  if ((iter = NEXT(iter)) == NULL) break;
 }
 /* Execute the default handler if the interrupt couldn't be processed. */
 return false;
}


PRIVATE ATTR_USED void FCALL
exec_deflirq(struct cpustate_ie *__restrict state);
PRIVATE ATTR_HOTTEXT ATTR_USED void FCALL
exec_statirq(struct interrupt *__restrict handler,
             struct cpustate_ie *__restrict state) {
 /* C-level IRQ handler with full CPU-state. */
 struct interrupt *iter = handler;
 for (;;) {
  int handler_code;
  assert(iter);
  assertf((iter->i_type&INTTYPE_MASK) != (INTTYPE_ASM&INTTYPE_MASK),
          "%d",IRREGS_INTNO(&state->iret));
  switch (__builtin_expect(iter->i_type&INTTYPE_MASK,INTTYPE_STATE)) {
  case INTTYPE_FAST:  handler_code = (*iter->i_proto.p_fast_arg)(iter->i_closure); break;
  case INTTYPE_BASIC: handler_code = (*iter->i_proto.p_baseexcept_arg)(&state->iret,iter->i_closure); break;
  default:            handler_code = (*iter->i_proto.p_except_arg)(state,iter->i_closure); break;
  }
  if unlikely(iter->i_type&INTTYPE_NOSHARE) goto ignored;
  switch (__builtin_expect(handler_code,INTCODE_HANDLED)) {
  case INTCODE_HANDLED:
   /* Track the number of hits. */
   ++iter->i_hits;
   /* Dynamically optimize the interrupt execution order. */
   if (iter != handler && !(iter->i_flags&(INTFLAG_PRIMARY|INTFLAG_SECONDARY)) &&
      (handler = PREV(iter),iter->i_hits > handler->i_hits &&
     !(handler->i_flags&INTFLAG_PRIMARY) && handler->i_prio == iter->i_prio)) {
    /* Swap the order of of `iter' and its predecessor (`handler') */
    if ((handler->i_link.le_next = iter->i_link.le_next) != NULL)
         handler->i_link.le_next->i_link.le_pself = &handler->i_link.le_next;
    iter->i_link.le_pself    = handler->i_link.le_pself;
    iter->i_link.le_next     = handler;
    handler->i_link.le_pself = &iter->i_link.le_next;
    *iter->i_link.le_pself   = iter;
   }
  case INTCODE_IGNORED: /* Ignore the interrupt and act as though it never happened. */
ignored:
   return;
  default:
   ++iter->i_miss;
   break;
  }
  if ((iter = NEXT(iter)) == NULL) break;
 }
 /* Execute the default handler if the interrupt couldn't be processed. */
 exec_deflirq(state);
}


#ifdef CONFIG_DEBUG
PRIVATE ATTR_NORETURN void KCALL
deflirq_illegal_recursion(struct cpustate_ie *__restrict state, irq_t intno) {
 PRIVATE int in_illegal_recursion = 0;
 if (ATOMIC_FETCHINC(in_illegal_recursion) == 0) {
  debug_printf("\n\nPANIC!!! IRQ HANDLER RECURSION!\n");
  debug_printf(REGISTER_PREFIX "IP   = %p\n",IRREGS_TAIL(&state->iret)->xip);
  debug_printf("I"             "NTNO = %d\n",intno);
 }
 /* Freeze the CPU */
 PREEMPTION_FREEZE();
}

#ifdef CONFIG_SMP
PRIVATE CPU_BSS  uintptr_t smp_deflirq_recursion[256/(8*sizeof(uintptr_t))];
#define deflirq_recursion  CPU(smp_deflirq_recursion)
#else
PRIVATE ATTR_RAREBSS uintptr_t deflirq_recursion[256/(8*sizeof(uintptr_t))];
#endif
#define DEFLIRQ_DO_ENTER(intno) \
 { uintptr_t addr = (intno)/(8*sizeof(uintptr_t)); \
   uintptr_t mask = (uintptr_t)1 << ((intno)%(8*sizeof(uintptr_t))); \
   if unlikely(deflirq_recursion[addr]&mask) \
      deflirq_illegal_recursion(state,intno); \
   deflirq_recursion[addr] |= mask; \
 }
#define DEFLIRQ_DO_LEAVE(intno) \
 { uintptr_t addr = (intno)/(8*sizeof(uintptr_t)); \
   uintptr_t mask = (uintptr_t)1 << ((intno)%(8*sizeof(uintptr_t))); \
   deflirq_recursion[addr] &= ~mask; \
 }
#ifdef CONFIG_SMP
LOCAL bool KCALL deflirq_enter_early(struct cpustate_ie *__restrict state, irq_t intno) {
 if (SMP_COUNT == 1) {
  /* Easy! We're the boot cpu. */
  uintptr_t addr = intno/(8*sizeof(uintptr_t));
  uintptr_t mask = (uintptr_t)1 << (intno%(8*sizeof(uintptr_t)));
  if unlikely(VCPU(BOOTCPU,smp_deflirq_recursion)[addr]&mask)
     deflirq_illegal_recursion(state,intno);
  VCPU(BOOTCPU,smp_deflirq_recursion)[addr] |= mask;
  return true; /* Don't try to enter again. */
 }
 return false;
}
#if defined(CONFIG_DEBUG) && defined(__x86_64__)
#define DEFLIRQ_DECLARE_VARS \
   bool __irq_did_enter;
#define DEFLIRQ_ENTER_EARLY(intno) \
 { __irq_did_enter = deflirq_enter_early(state,intno); }
#define DEFLIRQ_ENTER_LATER(intno) \
 { if (!__irq_did_enter) DEFLIRQ_DO_ENTER(intno) }
#else
#define DEFLIRQ_DECLARE_VARS       /* nothing */
#define DEFLIRQ_ENTER_EARLY(intno) /* nothing */
#define DEFLIRQ_ENTER_LATER(intno) DEFLIRQ_DO_ENTER(intno)
#endif
#define DEFLIRQ_LEAVE(intno)       DEFLIRQ_DO_LEAVE(intno)
#else
#define DEFLIRQ_DECLARE_VARS       /* nothing */
#define DEFLIRQ_ENTER_EARLY(intno) DEFLIRQ_DO_ENTER(intno)
#define DEFLIRQ_ENTER_LATER(intno) /* nothing */
#define DEFLIRQ_LEAVE(intno)       DEFLIRQ_DO_LEAVE(intno)
#endif
#else
#define DEFLIRQ_DECLARE_VARS       /* nothing */
#define DEFLIRQ_ENTER_EARLY(intno) /* nothing */
#define DEFLIRQ_ENTER_LATER(intno) /* nothing */
#define DEFLIRQ_LEAVE(intno)       /* nothing */
#endif


LOCAL void KCALL
exec_local_exception_handler(struct task *__restrict this_task,
                             struct cpustate_ie *__restrict state,
                             struct irregs *__restrict tail, irq_t intno) {
 struct intchain *iter;
check_again: ATTR_UNUSED;
 for (iter = this_task->t_ic; iter;
      iter = iter->ic_prev) {
  if ((iter->ic_irq == intno) ||
      (iter->ic_opt&INTCHAIN_OPT_EXC && INTNO_ISEXC(intno)) ||
      (iter->ic_opt&INTCHAIN_OPT_PIC && INTNO_ISPIC(intno)) ||
      (iter->ic_opt&INTCHAIN_OPT_USR && INTNO_ISUSR(intno))) {
   /* Trigger this exception handler. */
   this_task->t_ic = iter->ic_prev; /* Restore the handler before this one. */
   /* PREEMPTION_DISABLE(); // Not required. - At no point is our stack damaged. */
   DEFLIRQ_LEAVE(intno); /* Indicate that we have left this handler. */
   ((register_t *)&state->iret)[0] = tail->xflags;
   ((register_t *)&state->iret)[1] = (register_t)&iter->ic_int; /* p_eip (Load using `pop %esp; ret;') */
   __asm__ __volatile__(
    L(    movx %0, %%xsp     /* (ab-)use the CPU state as stack. */               )
    L(    __ASM_IPOP_COMREGS /* Restore common registers (We're now as xflags) */ )
    L(    popfx              /* Restore XFLAGS. */                                )
    L(    popx %%xsp         /* Restore XSP. */                                   )
    L(    ret                /* Jump to the exception handler. */                 )
    :
    : "g" (state)
    : "memory", "cc");
   __builtin_unreachable();
  }
 }
 /* XXX: Can this still happen on x86_64? (considering it doesn't really enforce segment limits) */
#if !defined(__x86_64__) || 1
 /* A lot of code assumes that it is enough to handle page-faults
  * in order to safely access a potentially faulty pointer, such
  * as may be given to the kernel from userspace, or when creating
  * a traceback, or when relocating a module.
  * Yet there exists a small problem with this idea:
  *     X86 has segmentation, and attempting to access memory that
  *     lies outside the associated segment (usually '%ds'), causes
  *     a protection fault to be raised.
  * Now you might say that shouldn't affect anything, since the KOS
  * kernel uses a flat memory model, but as it turns out (and this is
  * another one of those things that only happens on real hardware),
  * attempting to access memory above 4Gb will (rightfully so) raise
  * a general protection fault.
  * How does one access memory in that range?
  * This is how it can easily happen:
  * >> movl $0xffffffff, %esi // Pointer is given from user-space
  * >> movl 0(%esi),     %eax // Load data (This instruction would be guarded by 'EXC_PAGE_FAULT')
  * On real hardware, this will (apparently) try to read memory
  * from above 4Gb, rather than overflowing back to ZERO(0).
  * >> READ: 0xffffffff  --> OK (Part of the page-directory self mapping)
  * >> READ: 0x100000000 --> ERROR (Outside the %DS segment that ends after `0xffffffff')
  * >> READ: 0x100000001 --> ERROR (...)
  * >> READ: 0x100000002 --> ERROR (...)
  * So with that in mind, if a protection fault error occurs and there
  * is no local exception handler specifically for this purpose, also try to
  * handle it as a pagefault error.
  * NOTE: Only affects kernel-level local exception handlers.
  */
 if (intno == EXC_PROTECTION_FAULT) {
  intno    = EXC_PAGE_FAULT;
  goto check_again;
 }
#endif
}


/* Default/fallback IRQ handler. */
PRIVATE ATTR_USED void FCALL
exec_deflirq(struct cpustate_ie *__restrict state) {
 DEFLIRQ_DECLARE_VARS
 struct cpu *this_cpu; struct task *this_task;
 irq_t intno = (IRREGS_DECODE_INTNO(state->iret.intno) & 0xff);
 struct irregs *iret_tail = INTNO_HAS_EXC_CODE(intno)
  ? &state->iret.tail : &state->iret.__tail_noxcode;

 if (INTNO_ISPIC(intno)) {
  syslog(LOG_IRQ|LOG_WARN,
         COLDSTR("[IRQ] Unmapped PIC interrupt %#.2I8x (%I8d) (%s pin #%d)\n"),
         intno,intno,
         intno >= INTNO_PIC2_BASE ? COLDSTR("Slave") : COLDSTR("Master"),
        (intno-INTNO_PIC1_BASE) % 8);
  PIC_EOI(intno);
  return;
 } else if (!INTNO_ISEXC(intno)) {
  syslog(LOG_IRQ|LOG_WARN,
         COLDSTR("[INT] Unmapped interrupt %#.2Ix triggered in ring #%d at %p\n"),
         intno,(int)(iret_tail->cs&3),iret_tail->xip);
  return;
 }

 DEFLIRQ_ENTER_EARLY(intno);
#if defined(CONFIG_DEBUG) && defined(__x86_64__)
 /* Since we're dealing with exceptions here, try
  * to fix a potentially broken `gs_base' value. */
 { u64 old_gs_base = asm_rdgsbase();
#ifdef CONFIG_SMP
   if unlikely(old_gs_base < KERNEL_BASE)
#else
   if unlikely(old_gs_base != (u64)BOOTCPU)
#endif
   {
    u64 new_gs_base;
    PREEMPTION_DISABLE();
    __asm__ __volatile__("swapgs\n" : : : "memory"); /* Likely just a missing swapgs */
    new_gs_base = asm_rdgsbase();
#ifdef CONFIG_SMP
    if likely(new_gs_base >= KERNEL_BASE)
#else
    if likely(new_gs_base == (u64)BOOTCPU)
#endif
    {
     syslog(LOG_IRQ|LOG_ERROR,
            COLDSTR("[INT] Invalid `gs_base' set. - Missing `swapgs' for %p and %p\n"),
            old_gs_base,new_gs_base);
    } else {
     syslog(LOG_IRQ|LOG_ERROR,
            COLDSTR("[INT] Invalid `gs_base' set. - `swapgs' remained invalid for %p and %p\n"),
            old_gs_base,new_gs_base);
     __asm__ __volatile__("swapgs\n" : : : "memory"); /* Undo the swapgs. */
#ifdef CONFIG_SMP
     /* Use the GDT to fixup the correct GS base address. */
     { struct idt_pointer gdt; struct mman *omm;
       TASK_PDIR_KERNEL_BEGIN(omm);
       __asm__ __volatile__("sgdt %0\n" : "=m" (gdt));
       if unlikely((gdt.ip_limit+1) < (__KERNEL_PERCPU/SEG_INDEX_MULTIPLIER)*SEGMENT_SIZE ||
                   !pdir_test_readable(&pdir_kernel,gdt.ip_gdt+(__KERNEL_PERCPU/SEG_INDEX_MULTIPLIER))) {
        syslog(LOG_IRQ|LOG_ERROR,
               COLDSTR("[INT] Failed to determine `gs_base' (Default to `BOOTCPU')\n"));
set_default_gs_base:
        asm_wrgsbase((uintptr_t)BOOTCPU); /* Default to working with the boot cpu. */
       } else {
        struct segment percpu;
        percpu = gdt.ip_gdt[__KERNEL_PERCPU/SEG_INDEX_MULTIPLIER];
        new_gs_base = SEGMENT_GTBASE(percpu);
        if unlikely(new_gs_base < KERNEL_BASE) {
         syslog(LOG_IRQ|LOG_ERROR,
                COLDSTR("[INT] Invalid `gs_base' %p set in GDT\n"),
                new_gs_base);
         goto set_default_gs_base;
        }
        /* Set the fixed `gs_base' value. */
        asm_wrgsbase(new_gs_base);
       }
       TASK_PDIR_KERNEL_END(omm);
     }
#else
     asm_wrgsbase((uintptr_t)BOOTCPU);
#endif
    }
   }
 }
 COMPILER_BARRIER();
#endif /* CONFIG_DEBUG && __x86_64__ */
 DEFLIRQ_ENTER_LATER(intno);

 /* With per-cpu segments fixed, we should now be able
  * to safely access the current task structure. */
 this_cpu  = THIS_CPU;
 this_task = this_cpu->c_running;

 if (iret_tail->cs&3) {
  /* TODO: Exception doesn't originate from kernel-space.
   *    >> Send a signal to the calling task. */
 } else {
  /* Try to run for local exception handlers. */
  exec_local_exception_handler(this_task,state,iret_tail,intno);
 }

 /* Process the kernel panic and dump debug information. */
 kernel_panic_process(this_cpu,this_task,state,
                      kernel_panic_mask,
                     &debug_print,NULL);

 if (intno != EXC_BREAKPOINT)
     PREEMPTION_FREEZE();
 DEFLIRQ_LEAVE(intno);
}







/* Update the interrupt descriptor table entry for `intno' */
PRIVATE void KCALL
receive_rpc_update_idt(irq_t intno) {
 struct idt_pointer idt_ptr; u8 mode;
 struct idtentry *idt; struct interrupt *ent;
 uintptr_t handler_eip = DIRQ_ENTRY(intno);
 pflag_t was = PREEMPTION_PUSH();
#ifdef __x86_64__
 u8 ist = 0;
#endif
 IFDEF_SMP(atomic_rwlock_read(&inttab_lock));
 idt = &CPU(inttab).it_idt[intno];
 ent = CPU(inttab).it_tab[intno].e_head;
  /* Let's face it: Interrupt handlers are registered
   *                more often than they are deleted.
   *             >> Ergo: `ent != NULL' is fairly likely. */
 if likely(ent) {
  struct interrupt *other;
  inttype_t min_level,max_level;
  min_level = (ent->i_type&INTTYPE_MASK);
  mode      = ent->i_mode; /* Start out with the hardware-mode of the first handler. */
#ifdef __x86_64__
  ist       = ent->i_ist;
#endif
  if (min_level == (INTTYPE_ASM&INTTYPE_MASK)) {
   handler_eip = (uintptr_t)ent->i_callback;
   goto got_eip; /* Directly bind a primary assembly handler. */
  }
  max_level = min_level,other = ent;
  while ((other = other->i_link.le_next) != NULL) {
   inttype_t other_level = (other->i_type&INTTYPE_MASK);
   /* Merge hardware modes to be more restrictive:
    * >> REQUIRED_PRIVILEGE_LEVEL = MIN(OLD_REQUIRED_PRIVILEGE_LEVEL,NEW_REQUIRED_PRIVILEGE_LEVEL);
    * >> IS_RECURSION_DISABLED    = OLD_IS_RECURSION_DISABLED || NEW_IS_RECURSION_DISABLED;
    */
#ifdef __x86_64__
   assertf((other->i_mode&IDTTYPE_MASK) == IDTTYPE_80386_32_INTERRUPT_GATE,
           "No interrupt types other than `IDTTYPE_80386_32_INTERRUPT_GATE' can safely be used on x86_64");
   mode = (MIN(mode&IDTFLAG_DPL_MASK,other->i_mode&IDTFLAG_DPL_MASK)|
          (IDTTYPE_80386_32_INTERRUPT_GATE|IDTFLAG_PRESENT));
   if (ist != other->i_ist) ist = 0;
#else
   mode = (MIN(mode&IDTTYPE_MASK,other->i_mode&IDTTYPE_MASK)|
           MIN(mode&IDTFLAG_DPL_MASK,other->i_mode&IDTFLAG_DPL_MASK)|
           IDTFLAG_PRESENT);
#endif
   assertf(other_level != (INTTYPE_ASM&INTTYPE_MASK),
           "Assembly handlers _must_ always come first");
   if (min_level > other_level) min_level = other_level;
   if (max_level < other_level) max_level = other_level;
  }
  if (min_level == (INTTYPE_ASM_SEG&INTTYPE_MASK)) {
   assertf((ent->i_type&INTTYPE_MASK) == (INTTYPE_ASM_SEG&INTTYPE_MASK),
            "The first handler must be the asm-segment callback.\n"
            "This should have been enforced by assembly-handlers being required to be INTFLAG_PRIMARY + INTPRIO_MAX");
   /* Bind a segment-safe assembly handler. */
   handler_eip = ASEG_ENTRY(intno);
   ent = other; /* Use arch-flags of this handler. */
  } else if (max_level == (INTTYPE_BASIC&INTTYPE_MASK)) {
   /* Bind a basic c-function handler if no handlers need the full CPU-state. */
   handler_eip = BASI_ENTRY(intno);
  } else if (max_level == (INTTYPE_FAST&INTTYPE_MASK)) {
   /* Bind a fast c-function handler if no handlers need the full CPU-state. */
   handler_eip = FAST_ENTRY(intno);
  } else {
   assertf(max_level == (INTTYPE_STATE&INTTYPE_MASK),
           "Invalid interrupt handler type: %x",max_level);
   /* Bind a full cpu-state handler. */
   handler_eip = STAT_ENTRY(intno);
  }
 } else {
  mode = INTMODE_DEFAULT(intno);
 }
got_eip:
 assertf(mode&IDTFLAG_PRESENT,
         "All interrupt vectors must always be present.\n"
         "And that includes #%d\n",intno);
 assertf(!(mode&IDTFLAG_STORAGE_SEGMENT),
         "KOS doesn't support storage segment interrupt "
         "vectors (intno = %d)\n",intno);
#ifdef __x86_64__
 assertf((mode&IDTTYPE_MASK) == IDTTYPE_80386_32_INTERRUPT_GATE,
         "No interrupt types other than `IDTTYPE_80386_32_INTERRUPT_GATE' can safely be used on x86_64");
#endif
 idt->ie_off1 = (u16)handler_eip;
 idt->ie_sel  = __KERNEL_CS;
#ifdef __x86_64__
 idt->ie_ist  = ist;
#else
 idt->ie_zero = 0;
#endif
 idt->ie_flags = mode;
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
  iter->ie_flags = INTMODE_DEFAULT((irq_t)(iter-tab->it_idt));
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

 /* Setup the initial PIC state. */
 pic_bios_end();
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
 return int_addset(entry,&affinity);
}
PUBLIC ssize_t KCALL
int_addset(struct interrupt *__restrict entry,
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
 assertf(entry->i_owner,"No interrupt owner assigned");
 assertf((entry->i_type&INTTYPE_MASK) <= (INTTYPE_ASM_SEG&INTTYPE_MASK)
         ? (entry->i_flags&INTFLAG_PRIMARY) && (entry->i_prio == INTPRIO_MAX) : 1,
         "Assembly-level interrupt handler _must_ always be `INTFLAG_PRIMARY' + `INTPRIO_MAX'");

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
  if ((iter = *piter) == NULL) {
   /* The first link. */
   entry->i_link.le_next = NULL;
  } else if (entry->i_type&INTTYPE_NOSHARE ||
             iter->i_type &INTTYPE_NOSHARE) {
   /* Override existing handlers. */
#ifdef CONFIG_SMP
   if (delete_chain) {
    /* Append an existing delete-chain at the end of what's getting added. */
    struct interrupt *last = iter;
    while (last->i_link.le_next)
           last = last->i_link.le_next;
    last->i_link.le_next = delete_chain;
    delete_chain->i_link.le_pself = &last->i_link.le_next;
    delete_chain = iter;
   }
#endif
   delete_chain = iter;
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
 atomic_rwlock_endwrite(&inttab_lock);
 /* TODO: Send an RPC command to all CPUs apart of `affinity' */
 if (CPU_ISSET(THIS_CPU->c_id,affinity))
#endif
 {
  receive_rpc_update_idt(entry->i_intno);
 }
done:
 PREEMPTION_POP(was);
 /* Delete any interrupts that were overwritten. */
 while (delete_chain) {
  entry = delete_chain->i_link.le_next;
  interrupt_delete(delete_chain);
  delete_chain = entry;
 }
 return result;
err_eperm:  result = -EPERM; goto end;
err_eexist: result = -EEXIST; /*goto end;*/
end: IFDEF_SMP(atomic_rwlock_endwrite(&inttab_lock)); goto done;
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

#ifndef CONFIG_USE_OLD_INTERRUPTS
INTERN void KCALL
irq_delete_from_instance(struct instance *__restrict inst) {
 /* TODO */
}
#endif


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


DECL_END

#endif /* !GUARD_KERNEL_ARCH_INTERRUPT_C */
