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

DECL_BEGIN

INTERN void ASMCALL syscall_irq(void);
INTERN syscall_ulong_t ASMCALL sys_nosys(void);
INTERN syscall_ulong_t ASMCALL sys_ccall(void);
INTERN syscall_ulong_t ASMCALL sys_xcall(void);
INTERN syscall_ulong_t ASMCALL sys_lccall(void);
INTERN syscall_ulong_t ASMCALL sys_lxcall(void);

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
#define __SYSCALL(x,y)     [ x-__NR_syscall_min ] = __SYSCALL_ISLNG(x) ? &sys_lccall : &sys_ccall,
#define __SYSCALL_ASM(x,y) [ x-__NR_syscall_min ] = &y,
#include <asm/syscallno.ci>
};

PUBLIC CACHELINE_ALIGNED ATTR_HOTDATA ATTR_USED
syscall_t xsyscall_table[(__NR_xsyscall_max-__NR_xsyscall_min)+1] = {
    [0 ... (__NR_xsyscall_max-__NR_xsyscall_min)] = &sys_nosys,
#define __XSYSCALL(x,y)     [ x-__NR_xsyscall_min ] = __SYSCALL_ISLNG(x) ? &sys_lxcall : &sys_xcall,
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
PRIVATE ATTR_NORETURN ATTR_USED void FCALL preemption_not_enabled_enter(syscall_ulong_t sysno)
{ __assertion_failedf("Preemption not enabled on SYSCALL_ENTER",DEBUGINFO_NUL,"sysno = 0x%.8I32X (%I32u)",sysno,sysno); }
PRIVATE ATTR_NORETURN ATTR_USED void FCALL preemption_not_enabled_leave(void)
{ __assertion_failed("Preemption not enabled on SYSCALL_LEAVE",DEBUGINFO_NUL); }

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
    INTDEF struct segment cpu_gdt[SEG_BUILTIN];
    p.ip_limit = sizeof(cpu_gdt);
    p.ip_gdt   = CPU(cpu_gdt);
    __asm__("lgdt %0" : : "m" (p) : "memory");
  }
  INTDEF struct segment cpu_gdt[SEG_BUILTIN];
  CPU(cpu_gdt)[SEG_CPUTSS].rw = 0;
  __asm__ __volatile__("ltr %%bx\n"
                       :
                       : "b" (SEG(SEG_CPUTSS))
                       : "memory");
  { struct idt_pointer p;
    p.ip_idt   = CPU(cpu_idt).i_vector;
    p.ip_limit = sizeof(cpu_idt.i_vector);
    __asm__ __volatile__("lidt %0\n"
                         :
                         : "g" (p)
                         : "memory");
  }
  { struct idtentry e;
    e = CPU(cpu_idt).i_vector[0x80];
    syslog(LOG_DEBUG,"e.ie_data  = %p\n",e.ie_data);
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


#ifdef CONFIG_SYSCALL_CHECK_SEGMENTS
PRIVATE ATTR_NORETURN ATTR_USED ATTR_COLDTEXT void KCALL
sysreturn_bad_segment(u32 current, u32 correct, char const *name) {
 __assertion_failedf("Bad segment upon system-call return",DEBUGINFO_NUL,
                     "SEGMENT: %q\n"
                     "CURRENT: %.4I32X\n"
                     "CORRECT: %.4I32X\n",
                     name,current,correct);
}
GLOBAL_ASM(
L(.section .rodata.cold                          )
L(PRIVATE_OBJECT(name_ds) .string "ds"           )
L(PRIVATE_OBJECT(name_es) .string "es"           )
L(PRIVATE_OBJECT(name_fs) .string "fs"           )
L(PRIVATE_OBJECT(name_gs) .string "gs"           )
L(.previous                                      )
L(.section .text.cold                            )
L(PRIVATE_ENTRY(sysreturn_bad_ds)                )
L(    movw  2(%esp),  %bx                        )
L(    movl  $name_ds, %esi                       )
L(    jmp   1f                                   )
L(PRIVATE_ENTRY(sysreturn_bad_es)                )
L(    movw  4(%esp),  %bx                        )
L(    movl  $name_es, %esi                       )
L(    jmp   1f                                   )
L(PRIVATE_ENTRY(sysreturn_bad_fs)                )
L(    movw  6(%esp),  %bx                        )
L(    movl  $name_fs, %esi                       )
L(    jmp   1f                                   )
L(PRIVATE_ENTRY(sysreturn_bad_gs)                )
L(    movw  8(%esp),  %bx                        )
L(    movl  $name_gs, %esi                       )
L(1:  movzwl %bx, %ebx                           )
L(    movzwl %ax, %eax                           )
L(    pushl %esi                                 )
L(    pushl %ebx                                 )
L(    pushl %eax                                 )
L(    call  sysreturn_bad_segment                )
L(.previous                                      )
);
#endif

GLOBAL_ASM(
L(.section .text.hot                             )
L(.align CACHELINE                               )
L(PRIVATE_ENTRY(dbg_sysenter)                    )
L(    pushfl                                     )
#if 0
L(    pushal                                     )
L(    movl %eax, %ecx                            )
L(    call syscall_enter                         )
L(    popal                                      )
#endif
L(    testl $(EFLAGS_IF), 0(%esp)                )
L(    jz    1f                                   )
L(    popfl                                      )
L(    ret                                        )
L(1:  movl %eax, %ecx                            )
L(    jmp  preemption_not_enabled_enter          )
L(SYM_END(dbg_sysenter)                          )
L(PRIVATE_ENTRY(dbg_sysleave)                    )
L(    pushfl                                     )
#if 0
L(    pushw %gs                                  )
L(    pushw %fs                                  )
L(    pushw %es                                  )
L(    pushw %ds                                  )
L(    pushal                                     )
L(    call syscall_leave                         )
L(    popal                                      )
L(    popw  %ds                                  )
L(    popw  %es                                  )
L(    popw  %fs                                  )
L(    popw  %gs                                  )
#endif
L(    testl $(EFLAGS_IF), 0(%esp)                )
L(    jz    preemption_not_enabled_leave         )
L(    popfl                                      )
L(    iret                                       )
L(SYM_END(dbg_sysleave)                          )
L(.previous                                      )
);
#define SYS_ENTER sti; call dbg_sysenter
#define SYS_LEAVE      jmp  dbg_sysleave
#elif 1
#define SYS_ENTER /* nothing */
#define SYS_LEAVE iret
#else
#define SYS_ENTER sti
#define SYS_LEAVE iret
#endif

GLOBAL_ASM(
L(.section .text.hot                             )
L(.align CACHELINE                               )
L(PRIVATE_ENTRY(syscall_irq)                     )
L(    SYS_ENTER                                  )
#if __NR_syscall_min != 0
L(    cmpl  $(__NR_syscall_min), %eax            )
L(    jb    1f                                   )
#endif
L(    cmpl  $(__NR_syscall_max), %eax            )
L(    ja    1f                                   )
L(    jmp  *(syscall_table-__NR_syscall_min*4)(,%eax,4))
L(1:                                             )
#if __NR_xsyscall_min != 0
L(    cmpl  $(__NR_xsyscall_min), %eax           )
L(    jb    sys_nosys                            )
#endif
L(    cmpl  $(__NR_xsyscall_max), %eax           )
L(    ja    sys_nosys                            )
L(    jmp  *(xsyscall_table-((__NR_xsyscall_min*4) & 0xffffffff))(,%eax,4))
L(SYM_END(syscall_irq)                           )

L(PRIVATE_ENTRY(sys_ccall)                       )
#ifdef CONFIG_SYSCALL_CHECK_SEGMENTS
L(    pushw %gs                                  )
L(    pushw %fs                                  )
L(    pushw %es                                  )
L(    pushw %ds                                  )
L(    pushl 12(%esp) /* eip */                   )
#endif /* CONFIG_SYSCALL_CHECK_SEGMENTS */
L(    pushl %ebp                                 )
__DEBUG_CODE(L(movl %esp, %ebp))
L(    pushl %edi                                 )
L(    pushl %esi                                 )
L(    pushl %edx                                 )
L(    pushl %ecx                                 )
L(    pushl %ebx                                 )
L(    call *(syscall_c_table-__NR_syscall_min*4)(,%eax,4))
L(1:  popl  %ebx                                 )
L(    popl  %ecx                                 )
L(    popl  %edx                                 )
L(2:  popl  %esi                                 )
L(    popl  %edi                                 )
L(    popl  %ebp                                 )
#ifdef CONFIG_SYSCALL_CHECK_SEGMENTS
L(    addl  $4, %esp                             )
L(    pushw %ax                                  )
L(    movw  %ds, %ax                             )
L(    cmpw  %ax, 2(%esp)                         )
L(    jne   sysreturn_bad_ds                     )
L(    movw  %es, %ax                             )
L(    cmpw  %ax, 4(%esp)                         )
L(    jne   sysreturn_bad_es                     )
L(    movw  %fs, %ax                             )
L(    cmpw  %ax, 6(%esp)                         )
L(    jne   sysreturn_bad_fs                     )
L(    movw  %gs, %ax                             )
L(    cmpw  %ax, 8(%esp)                         )
L(    jne   sysreturn_bad_gs                     )
L(    popw  %ax                                  )
L(    addl  $8, %esp                             )
#endif /* CONFIG_SYSCALL_CHECK_SEGMENTS */
L(    SYS_LEAVE                                  )
L(SYM_END(sys_ccall)                             )

L(PRIVATE_ENTRY(sys_xcall)                       )
#ifdef CONFIG_SYSCALL_CHECK_SEGMENTS
L(    pushw %gs                                  )
L(    pushw %fs                                  )
L(    pushw %es                                  )
L(    pushw %ds                                  )
L(    pushl 12(%esp) /* eip */                   )
#endif /* CONFIG_SYSCALL_CHECK_SEGMENTS */
L(    pushl %ebp                                 )
__DEBUG_CODE(L(movl %esp, %ebp))
L(    pushl %edi                                 )
L(    pushl %esi                                 )
L(    pushl %edx                                 )
L(    pushl %ecx                                 )
L(    pushl %ebx                                 )
L(    pushl $1b                                  )
L(    jmp *(xsyscall_c_table-((__NR_xsyscall_min*4) & 0xffffffff))(,%eax,4))
L(SYM_END(sys_xcall)                             )

L(PRIVATE_ENTRY(sys_lccall)                      )
#ifdef CONFIG_SYSCALL_CHECK_SEGMENTS
L(    pushw %gs                                  )
L(    pushw %fs                                  )
L(    pushw %es                                  )
L(    pushw %ds                                  )
L(    pushl 12(%esp) /* eip */                   )
#endif /* CONFIG_SYSCALL_CHECK_SEGMENTS */
L(    pushl %ebp                                 )
__DEBUG_CODE(L(movl %esp, %ebp))
L(    pushl %edi                                 )
L(    pushl %esi                                 )
L(    pushl %edx                                 )
L(    pushl %ecx                                 )
L(    pushl %ebx                                 )
L(    call *(syscall_c_table-__NR_syscall_min*4)(,%eax,4))
L(1:  popl  %ebx                                 )
L(    popl  %ecx                                 )
L(    addl  $4, %esp /* Original EDX */          )
#ifdef CONFIG_SYSCALL_CHECK_SEGMENTS
L(    jmp   2b                                   )
#else
L(    popl  %esi                                 )
L(    popl  %edi                                 )
L(    popl  %ebp                                 )
L(    SYS_LEAVE                                  )
#endif
L(SYM_END(sys_lccall)                            )

L(PRIVATE_ENTRY(sys_lxcall)                      )
#ifdef CONFIG_SYSCALL_CHECK_SEGMENTS
L(    pushw %gs                                  )
L(    pushw %fs                                  )
L(    pushw %es                                  )
L(    pushw %ds                                  )
L(    pushl 12(%esp) /* eip */                   )
#endif /* CONFIG_SYSCALL_CHECK_SEGMENTS */
L(    pushl %ebp                                 )
__DEBUG_CODE(L(movl %esp, %ebp))
L(    pushl %edi                                 )
L(    pushl %esi                                 )
L(    pushl %edx                                 )
L(    pushl %ecx                                 )
L(    pushl %ebx                                 )
L(    pushl $1b                                  )
L(    jmp  *(xsyscall_c_table-((__NR_xsyscall_min*4) & 0xffffffff))(,%eax,4))
L(SYM_END(sys_lxcall)                            )

L(PRIVATE_ENTRY(sys_nosys)                       )
      /* TODO: raise(SIGSYS)? */
L(    movl $(-ENOSYS), %eax                      )
L(    SYS_LEAVE                                  )
L(SYM_END(sys_nosys)                             )

L(.previous                                      )
);
#undef PAUSE

PRIVATE ATTR_FREERODATA isr_t const syscall_isr = ISR_DEFAULT(SYSCALL_INT,&syscall_irq);
PRIVATE MODULE_INIT void syscall_init(void) { irq_set(&syscall_isr,NULL,IRQ_SET_RELOAD); }


DECL_END

#endif /* !GUARD_KERNEL_CORE_SYSCALL_C */
