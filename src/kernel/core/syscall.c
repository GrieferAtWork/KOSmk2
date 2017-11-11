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
#ifndef GUARD_KERNEL_CORE_SYSCALL_C
#define GUARD_KERNEL_CORE_SYSCALL_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <dev/blkdev.h>
#include <errno.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <kernel/arch/gdt.h>
#include <kernel/arch/idt_pointer.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <linux/unistd.h>
#include <sched/cpu.h>
#include <sched/task.h>
#include <stdlib.h>
#include <asm/instx.h>

DECL_BEGIN

INTERN void ASMCALL syscall_irq(void);
INTERN syscall_ulong_t ASMCALL sys_nosys(void);
INTERN syscall_ulong_t ASMCALL sys_ccall(void);
INTERN syscall_ulong_t ASMCALL sys_xcall(void);
#ifndef __x86_64__
INTERN syscall_ulong_t ASMCALL sys_lccall(void);
INTERN syscall_ulong_t ASMCALL sys_lxcall(void);
#endif /* !__x86_64__ */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"

#define __SYSCALL(attrib,nr,sys_name)

#undef __SYSCALL

#define __SYSCALL(x,y)      INTDEF syscall_ulong_t y(void);
#define __XSYSCALL(x,y)     INTDEF syscall_ulong_t y(void);
#define __SYSCALL_ASM(x,y)  INTDEF syscall_ulong_t ASMCALL y(void);
#define __XSYSCALL_ASM(x,y) INTDEF syscall_ulong_t ASMCALL y(void);
#include <asm/syscallno.ci>

PUBLIC CACHELINE_ALIGNED ATTR_HOTDATA ATTR_USED
syscall_t syscall_table[(__NR_syscall_max-__NR_syscall_min)+1] = {
    [0 ... (__NR_syscall_max-__NR_syscall_min)] = &sys_nosys,
#ifdef __x86_64__
#define __SYSCALL(x,y)     [ x-__NR_syscall_min ] = &sys_ccall,
#else /* __x86_64__ */
#define __SYSCALL(x,y)     [ x-__NR_syscall_min ] = __SYSCALL_ISLNG(x) ? &sys_lccall : &sys_ccall,
#endif /* !__x86_64__ */
#define __SYSCALL_ASM(x,y) [ x-__NR_syscall_min ] = &y,
#include <asm/syscallno.ci>
};

PUBLIC CACHELINE_ALIGNED ATTR_HOTDATA ATTR_USED
syscall_t xsyscall_table[(__NR_xsyscall_max-__NR_xsyscall_min)+1] = {
    [0 ... (__NR_xsyscall_max-__NR_xsyscall_min)] = &sys_nosys,
#ifdef __x86_64__
#define __XSYSCALL(x,y)     [ x-__NR_xsyscall_min ] = &sys_xcall,
#else /* __x86_64__ */
#define __XSYSCALL(x,y)     [ x-__NR_xsyscall_min ] = __SYSCALL_ISLNG(x) ? &sys_lxcall : &sys_xcall,
#endif /* !__x86_64__ */
#define __XSYSCALL_ASM(x,y) [ x-__NR_xsyscall_min ] = &y,
#include <asm/syscallno.ci>
};

PUBLIC CACHELINE_ALIGNED ATTR_HOTDATA ATTR_USED
syscall_t syscall_c_table[(__NR_syscall_max-__NR_syscall_min)+1] = {
    [0 ... (__NR_syscall_max-__NR_syscall_min)] = &sys_nosys,
#define __SYSCALL(x,y)   [ x-__NR_syscall_min ] = &y,
#include <asm/syscallno.ci>
};


PUBLIC CACHELINE_ALIGNED ATTR_HOTDATA ATTR_USED
syscall_t xsyscall_c_table[(__NR_xsyscall_max-__NR_xsyscall_min)+1] = {
    [0 ... (__NR_xsyscall_max-__NR_xsyscall_min)] = &sys_nosys,
#define __XSYSCALL(x,y)   [ x-__NR_xsyscall_min ] = &y,
#include <asm/syscallno.ci>
};

#pragma GCC diagnostic pop

#if defined(CONFIG_DEBUG) && 1
PRIVATE ATTR_NORETURN ATTR_USED void FCALL preemption_not_enabled_leave(void) {
 __NAMESPACE_INT_SYM
 __afail("Preemption not enabled on SYSCALL_LEAVE",DEBUGINFO_NUL);
}

#if 0
PRIVATE ATTR_USED void FCALL syscall_enter(syscall_ulong_t sysno) {
 if (1) {
  syslog(LOG_DEBUG,"SYSCALL_ENTER:0x%.8I32X (%I32u) (EIP: %p)\n",
         sysno,sysno,THIS_SYSCALL_EIP);
  volatile unsigned int i = 0;
  while (++i < (1 << 28));
 }
}
PRIVATE ATTR_USED void FCALL syscall_leave(void) {
 if (1) {
#if 1
  task_crit();
#if 1
  { void *temp = malloc(4096); ssize_t error;
    struct blkdev *b = get_bootpart();
    assert(temp);
    HOSTMEMORY_BEGIN {
     error = blkdev_read(b,0xd0f800,temp,4096);
     assertf(E_ISOK(error),"%Id",error);
    }
    HOSTMEMORY_END;
    BLKDEV_DECREF(b);
    free(temp);
  }
#endif
  task_endcrit();
#endif
  THIS_SYSCALL_EFLAGS &= ~(EFLAGS_IF);
  syslog(LOG_DEBUG,"SYSCALL_LEAVE: (EIP: %p)\n",
         THIS_SYSCALL_EIP);
  syslog(LOG_DEBUG,"SS = %p; ESP = %p; EFLAGS = %p;\n",
         THIS_SYSCALL_SS,THIS_SYSCALL_USERESP,THIS_SYSCALL_EFLAGS);
  syslog(LOG_DEBUG,"CS = %p; EBP = %p;\n",
         THIS_SYSCALL_CS,THIS_SYSCALL_EBP);
  { struct idt_pointer p;
    p.ip_idt   = CPU(cpu_idt).i_vector;
    p.ip_limit = sizeof(cpu_idt.i_vector);
    __asm__ __volatile__("lidt %0\n" : : "m" (p));
  }
  { struct idtentry e;
    e = CPU(cpu_idt).i_vector[0x80];
    syslog(LOG_DEBUG,"e.ie_off1  = %p\n",e.ie_off1);
    syslog(LOG_DEBUG,"e.ie_sel   = %p\n",e.ie_sel);
    syslog(LOG_DEBUG,"e.ie_zero  = %p\n",e.ie_zero);
    syslog(LOG_DEBUG,"e.ie_flags = %p\n",e.ie_flags);
    syslog(LOG_DEBUG,"e.ie_off2  = %p\n",e.ie_off2);
  }

  //volatile unsigned int i = 0;
  //while (++i < (1 << 28));
  //for (;;);
  //PREEMPTION_FREEZE();
 }
}
#endif

GLOBAL_ASM(
L(.section .text.hot                                                          )
L(.align CACHELINE                                                            )
L(PRIVATE_ENTRY(dbg_sysleave)                                                 )
L(    pushfx                                                                  )
#if 0
L(    __ASM_PUSH_SGREGS                                                       )
L(    __ASM_PUSH_GPREGS                                                       )
L(    call  syscall_leave                                                     )
L(    __ASM_POP_GPREGS                                                        )
L(    __ASM_POP_SGREGS                                                        )
#endif
L(    testx $(EFLAGS_IF), 0(%xsp)                                             )
L(    jz    preemption_not_enabled_leave                                      )
L(    addx  $(XSZ), %xsp                                                      )
L(    __ASM_IRET                                                              )
L(SYM_END(dbg_sysleave)                                                       )
L(.previous                                                                   )
);
#if 0
PRIVATE ATTR_USED void FCALL syscall_enter(syscall_ulong_t sysno) {
 syslog(LOG_DEBUG,"SYSCALL:%Iu\n",sysno);
}
#define SYS_ENTER pushal; movl %eax, %ecx; call syscall_enter; popal;
#else
#define SYS_ENTER /* Nothing */
#endif
#define SYS_LEAVE jmp dbg_sysleave
#else /* CONFIG_DEBUG */
#define SYS_ENTER /* Nothing */
#define SYS_LEAVE __ASM_IRET
#endif /* !CONFIG_DEBUG */



#ifdef CONFIG_SYSCALL_CHECK_SEGMENTS
PRIVATE ATTR_NORETURN ATTR_USED ATTR_COLDTEXT void KCALL
sysreturn_bad_segment(u32 current, u32 correct, char const *name) {
 __NAMESPACE_INT_SYM
 __afailf("Bad segment upon system-call return",DEBUGINFO_NUL,
          "SEGMENT: %q\n"
          "CURRENT: %.4I32X\n"
          "CORRECT: %.4I32X\n",
          name,current,correct);
}


/* These are the correct segment values when a system call is invoked. */
#define SYSRETURN_CORRECT_CS __KERNEL_CS
#define SYSRETURN_CORRECT_DS __USER_DS
#define SYSRETURN_CORRECT_ES __USER_DS
#ifdef __x86_64__
#define SYSRETURN_CORRECT_FS __USER_DS
#define SYSRETURN_CORRECT_GS __KERNEL_PERCPU
#else
#define SYSRETURN_CORRECT_FS __KERNEL_PERCPU
#define SYSRETURN_CORRECT_GS __USER_DS
#endif
#define SYSRETURN_CORRECT_SS __KERNEL_DS

GLOBAL_ASM(
L(.section .rodata.cold                                                       )
L(PRIVATE_OBJECT(name_cs) .string "cs"; SYM_END(name_cs);                     )
#ifndef __x86_64__
L(PRIVATE_OBJECT(name_ss) .string "ss"; SYM_END(name_ss);                     )
L(PRIVATE_OBJECT(name_ds) .string "ds"; SYM_END(name_ds);                     )
L(PRIVATE_OBJECT(name_es) .string "es"; SYM_END(name_es);                     )
#endif /* !__x86_64__ */
L(PRIVATE_OBJECT(name_fs) .string "fs"; SYM_END(name_fs);                     )
L(PRIVATE_OBJECT(name_gs) .string "gs"; SYM_END(name_gs);                     )
L(.previous                                                                   )
L(                                                                            )
L(.section .text.cold                                                         )
L(PRIVATE_ENTRY(sysreturn_bad_cs)                                             )
#ifdef __x86_64__
#define LOAD_ADDR(x,y) leaq x(%rip), y
#else
#define LOAD_ADDR(x,y) movl $(x), y
#endif
L(    movw  $(SYSRETURN_CORRECT_CS),  %bx                                     )
L(    LOAD_ADDR(name_cs,%xsi)                                                 )
L(    jmp   1f                                                                )
L(SYM_END(sysreturn_bad_cs)                                                   )
#ifndef __x86_64__
L(PRIVATE_ENTRY(sysreturn_bad_ss)                                             )
L(    movw  $(SYSRETURN_CORRECT_SS),  %bx                                     )
L(    LOAD_ADDR(name_ss,%xsi)                                                 )
L(    jmp   1f                                                                )
L(SYM_END(sysreturn_bad_ss)                                                   )
L(PRIVATE_ENTRY(sysreturn_bad_ds)                                             )
L(    movw  $(SYSRETURN_CORRECT_DS),  %bx                                     )
L(    LOAD_ADDR(name_ds,%xsi)                                                 )
L(    jmp   1f                                                                )
L(SYM_END(sysreturn_bad_ds)                                                   )
L(PRIVATE_ENTRY(sysreturn_bad_es)                                             )
L(    movw  $(SYSRETURN_CORRECT_ES),  %bx                                     )
L(    LOAD_ADDR(name_es,%xsi)                                                 )
L(    jmp   1f                                                                )
L(SYM_END(sysreturn_bad_es)                                                   )
#endif /* !__x86_64__ */
L(PRIVATE_ENTRY(sysreturn_bad_fs)                                             )
L(    movw  $(SYSRETURN_CORRECT_FS),  %bx                                     )
L(    LOAD_ADDR(name_fs,%xsi)                                                 )
L(    jmp   1f                                                                )
L(SYM_END(sysreturn_bad_fs)                                                   )
L(PRIVATE_ENTRY(sysreturn_bad_gs)                                             )
L(    movw  $(SYSRETURN_CORRECT_GS),  %bx                                     )
L(    LOAD_ADDR(name_gs,%xsi)                                                 )
L(1:  movzwx %bx, %xbx                                                        )
L(    movzwx %ax, %xax                                                        )
#ifdef __x86_64__
L(    movx  %xax, %xdi                                                        )
L(    movx  %xbx, %xsi                                                        )
L(    movx  %xsi, %xdx                                                        )
#else /* __x86_64__ */
L(    pushx %xsi                                                              )
L(    pushx %xbx                                                              )
L(    pushx %xax                                                              )
#endif /* !__x86_64__ */
L(    call  sysreturn_bad_segment                                             )
#undef LOAD_ADDR
L(SYM_END(sysreturn_bad_gs)                                                   )
L(.previous                                                                   )
L(                                                                            )
L(.section .text                                                              )
L(INTERN_ENTRY(sysreturn_check_segments)                                      )
L(    pushw %ax                                                               )
L(    movw  %cs, %ax                                                          )
L(    cmpw  $(SYSRETURN_CORRECT_CS), %ax                                      )
L(    jne   sysreturn_bad_cs                                                  )
#ifndef __x86_64__
L(    movw  %ss, %ax                                                          )
L(    cmpw  $(SYSRETURN_CORRECT_SS), %ax                                      )
L(    jne   sysreturn_bad_ss                                                  )
L(    movw  %ds, %ax                                                          )
L(    cmpw  $(SYSRETURN_CORRECT_DS), %ax                                      )
L(    jne   sysreturn_bad_ds                                                  )
L(    movw  %es, %ax                                                          )
L(    cmpw  $(SYSRETURN_CORRECT_ES), %ax                                      )
L(    jne   sysreturn_bad_es                                                  )
#endif /* !__x86_64__ */
L(    movw  %fs, %ax                                                          )
L(    cmpw  $(SYSRETURN_CORRECT_FS), %ax                                      )
L(    jne   sysreturn_bad_fs                                                  )
L(    movw  %gs, %ax                                                          )
L(    cmpw  $(SYSRETURN_CORRECT_GS), %ax                                      )
L(    jne   sysreturn_bad_gs                                                  )
L(    popw  %ax                                                               )
L(SYM_END(sysreturn_check_segments)                                           )
L(.previous                                                                   )
);
#endif


GLOBAL_ASM(
L(.section .text.hot                                                          )
L(.align CACHELINE                                                            )
L(    /* KOS System call entry point */                                       )
L(INTERN_ENTRY(syscall_irq)                                                   )
L(    SYS_ENTER                                                               )
L(    /* Check for underflow in system call ID */                             )
#if __NR_syscall_min != 0
L(    cmpx  $(__NR_syscall_min), %xax                                         )
L(    jb    1f                                                                )
#endif
L(                                                                            )
L(    /* Check for overflow in system call ID */                              )
L(    cmpx  $(__NR_syscall_max), %xax                                         )
L(    ja    1f                                                                )
L(                                                                            )
#if defined(__x86_64__) && (defined(ASM_USE_MOVABS) || defined(ASM_USE_LEAIP))
L(    /* This is a bit more complicated because we're a higher-half kernel. */)
L(    pushq %rax                                                              )
L(    pushq %rbp                                                              )
#ifdef ASM_USE_MOVABS
L(    movabs $(syscall_table-__NR_syscall_min*8), %rbp                        )
#else
L(    leaq (syscall_table-__NR_syscall_min*8)(%rip), %rbp                     )
#endif
L(    shrq  $3,   %rax                                                        )
L(    addq  %rbp, %rax                                                        )
L(    popq  %rbp                                                              )
L(    xchgq %rax, 0(%rsp)                                                     )
L(    ret                                                                     )
#else
L(    /* Simply jump to the address of the system call. */                    )
L(    jmpx *(syscall_table-__NR_syscall_min*4)(,%xax,4)                       )
#endif
L(1:                                                                          )
#ifdef __x86_64__
L(    movl %eax, %eax /* This may seem odd, but it actually clears the upper 32 bits. */)
#endif
L(                                                                            )
L(    /* Extended (x) system call handling. */                                )
#if __NR_xsyscall_min != 0
L(    cmpl  $(__NR_xsyscall_min), %eax                                        )
L(    jb    sys_nosys                                                         )
#endif
L(    cmpl  $(__NR_xsyscall_max), %eax                                        )
L(    ja    sys_nosys                                                         )
#if defined(__x86_64__) && (defined(ASM_USE_MOVABS) || defined(ASM_USE_LEAIP))
L(    /* This is a bit more complicated because we're a higher-half kernel. */)
L(    pushq %rax                                                              )
L(    pushq %rbp                                                              )
#ifdef ASM_USE_MOVABS
L(    movabs $(xsyscall_table-((__NR_xsyscall_min*8) & 0xffffffff)), %rbp     )
#else
L(    leaq (xsyscall_table-((__NR_xsyscall_min*8) & 0xffffffff))(%rip), %rbp  )
#endif
L(    shrq  $3,   %rax                                                        )
L(    addq  %rbp, %rax                                                        )
L(    popq  %rbp                                                              )
L(    xchgq %rax, 0(%rsp)                                                     )
L(    ret                                                                     )
#else
L(    /* Jump to the extended system call handler. */                         )
L(    jmpx *(xsyscall_table-((__NR_xsyscall_min*4) & 0xffffffff))(,%xax,4)    )
#endif
L(SYM_END(syscall_irq)                                                        )
L(.previous                                                                   )
);


#ifdef __x86_64__
#define SYSCALL_SAFEREGISTERS \
    sti; /* Enable interrupts */ \
    __DEBUG_CODE(pushq %rbp; /* __initial_rbp */) \
    __ASM_PUSH_SEGMENTS \
    pushq %r9; \
    pushq %r8; \
    pushq %r10; \
    pushq %rdx; \
    pushq %rsi; \
    pushq %rdi; \
    xchgq %rcx, %r10;
#else
#define SYSCALL_SAFEREGISTERS \
    sti; /* Enable interrupts */ \
    __ASM_PUSH_SEGMENTS \
    __DEBUG_CODE(pushl 12(%esp); /* __initial_eip */) \
    pushl %ebp; \
    __DEBUG_CODE(movl %esp, %ebp;) \
    pushl %edi; \
    pushl %esi; \
    pushl %edx; \
    pushl %ecx; \
    pushl %ebx;
#endif
#define SYSCALL_LOADSEGMENTS \
    __ASM_LOAD_SEGMENTS(%dx)



GLOBAL_ASM(
L(.section .text.hot                                                          )
L(PRIVATE_ENTRY(sys_ccall)                                                    )
L(    SYSCALL_SAFEREGISTERS                                                   )
L(    SYSCALL_LOADSEGMENTS                                                    )
L(                                                                            )
#if defined(__x86_64__) && (defined(ASM_USE_MOVABS) || defined(ASM_USE_LEAIP))
L(    /* This is a bit more complicated because we're a higher-half kernel. */)
L(    pushq %rbp                                                              )
#ifdef ASM_USE_MOVABS
L(    movabs $(syscall_c_table-(__NR_syscall_min*8)), %rbp                    )
#else
L(    leaq (syscall_c_table-(__NR_syscall_min*8))(%rip), %rbp                 )
#endif
L(    shrq  $3, %rax                                                          )
L(    addq  %rbp, %rax                                                        )
L(    popq  %rbp                                                              )
L(    callq *%rax                                                             )
#else
L(    callx *(syscall_c_table-__NR_syscall_min*4)(,%xax,4)                    )
#endif
L(                                                                            )
#ifdef __x86_64__
L(1:  popq  %rdi                                                              )
L(    popq  %rsi                                                              )
L(    popq  %rdx                                                              )
L(    popq  %r10                                                              )
L(    popq  %r8                                                               )
L(    popq  %r9                                                               )
#else
L(1:  popl  %ebx                                                              )
L(    popl  %ecx                                                              )
L(    popl  %edx                                                              )
L(2:  popl  %esi                                                              )
L(    popl  %edi                                                              )
L(    popl  %ebp                                                              )
__DEBUG_CODE(L(addl $4, %esp /* __initial_eip */))
#endif
L(    ASM_SYSRETURN_CHECK_SEGMENTS                                            )
L(    __ASM_POP_SEGMENTS                                                      )
#ifdef __x86_64__
__DEBUG_CODE(L(addq $8, %rsp /* __initial_rbp */))
#endif
L(    SYS_LEAVE                                                               )
L(SYM_END(sys_ccall)                                                          )
L(                                                                            )
L(PRIVATE_ENTRY(sys_xcall)                                                    )
L(    SYSCALL_SAFEREGISTERS                                                   )
L(    SYSCALL_LOADSEGMENTS                                                    )
L(                                                                            )
#if defined(__x86_64__) && (defined(ASM_USE_MOVABS) || defined(ASM_USE_LEAIP))
L(    /* This is a bit more complicated because we're a higher-half kernel. */)
L(    pushq %rbp                                                              )
#ifdef ASM_USE_MOVABS
L(    movabs $(xsyscall_c_table-((__NR_xsyscall_min*8) & 0xffffffff)), %rbp   )
#else
L(    leaq (xsyscall_c_table-((__NR_xsyscall_min*8) & 0xffffffff))(%rip), %rbp)
#endif
L(    shrq  $3, %rax                                                          )
L(    addq  %rbp, %rax                                                        )
L(    popq  %rbp                                                              )
L(    callq *%rax                                                             )
L(    jmp   1b                                                                )
#else
L(    pushx $1b /* Push the cleanup return address of the system call */      )
L(    jmpx *(xsyscall_c_table-((__NR_xsyscall_min*4) & 0xffffffff))(,%xax,4)  )
#endif
L(SYM_END(sys_xcall)                                                          )
L(                                                                            )
#ifndef __x86_64__
L(PRIVATE_ENTRY(sys_lccall)                                                   )
L(    SYSCALL_SAFEREGISTERS                                                   )
L(    SYSCALL_LOADSEGMENTS                                                    )
L(                                                                            )
L(    calll *(syscall_c_table-__NR_syscall_min*4)(,%eax,4)                    )
L(1:  popl  %ebx                                                              )
L(    popl  %ecx                                                              )
L(    addl  $4, %esp /* Don't restore EDX */                                  )
L(    jmp   2b                                                                )
L(SYM_END(sys_lccall)                                                         )
L(                                                                            )
L(PRIVATE_ENTRY(sys_lxcall)                                                   )
L(    SYSCALL_SAFEREGISTERS                                                   )
L(    SYSCALL_LOADSEGMENTS                                                    )
L(                                                                            )
L(    pushl $1b                                                               )
L(    jmpl *(xsyscall_c_table-((__NR_xsyscall_min*4) & 0xffffffff))(,%eax,4)  )
L(SYM_END(sys_lxcall)                                                         )
L(.previous                                                                   )
#endif /* !__x86_64__ */
L(                                                                            )
L(.section .text.hot                                                          )
L(PRIVATE_ENTRY(sys_nosys)                                                    )
L(    /* TODO: raise(SIGSYS)? */                                              )
L(    movx $(-ENOSYS), %xax                                                   )
L(    SYS_LEAVE                                                               )
L(SYM_END(sys_nosys)                                                          )
L(.previous                                                                   )
);


PRIVATE ATTR_FREERODATA isr_t const syscall_isr = {
    .i_num   = SYSCALL_INT,
    .i_flags = IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE|IDTFLAG_DPL(3),
    .i_func  = &syscall_irq,
    .i_owner = THIS_INSTANCE
};


PRIVATE MODULE_INIT void syscall_init(void) {
 irq_set(&syscall_isr,NULL,IRQ_SET_RELOAD);
}


DECL_END

#endif /* !GUARD_KERNEL_CORE_SYSCALL_C */
