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
#ifndef GUARD_LIBS_LIBC_ARCH_I386_KOS_ASSERT_C_INL
#define GUARD_LIBS_LIBC_ARCH_I386_KOS_ASSERT_C_INL 1
#define _KOS_SOURCE 2

#include "../../libc.h"
#include "../../format-printer.h"
#include "../../misc.h"
#include "../../assert.h"

#include <assert.h>
#include <hybrid/asm.h>
#include <hybrid/atomic.h>
#include <hybrid/host.h>
#include <hybrid/debug.h>
#include <hybrid/compiler.h>
#include <hybrid/debuginfo.h>
#include <hybrid/traceback.h>
#include <hybrid/types.h>
#include <hybrid/host.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/syslog.h>
#include <asm/registers.h>
#include <asm/instx.h>

#ifdef __KERNEL__
#include <sched/percpu.h>
#include <sched/cpu.h>
#include <sched/types.h>
#include <modules/tty.h>
#include <kernel/interrupt.h>
#include <kernel/mman.h>
#include <linker/module.h>
#include <arch/cpustate.h>
#else /* __KERNEL__ */
#include "../../system.h"
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include "../../string.h"
#include "../../unicode.h"
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */

DECL_BEGIN

/* Allow for calls from assembly. */
#define HAVE_LIBC_DEBUG_TBPRINT 1
GLOBAL_ASM(
L(.section .text.cold                               )
L(INTERN_ENTRY(libc_debug_tbprint)                  )
L(    __ASM_PUSH_SCRATCH /* Preserve registers */   )
#ifdef __x86_64__
L(    xorq   %rdi, %rdi                             )
L(    movq   %rbp, %rsi                             )
L(    call PLT_SYM(libc_debug_tbprint2)             )
#elif defined(__i386__)
L(    pushl  $0                                     )
L(    pushl  %ebp                                   )
L(    call PLT_SYM(libc_debug_tbprint2)             )
#else
#error "Unsupported arch"
#endif
L(    __ASM_POP_SCRATCH /* Restore registers */     )
L(    ret                                           )
L(SYM_END(libc_debug_tbprint)                       )
L(.previous                                         )
);


#if defined(__KERNEL__) && 1
#define HAVE_LIBC_DEBUG_PRINT 1
INTERN ssize_t LIBCCALL
libc_debug_print(char const *__restrict data, size_t datalen,
                 void *UNUSED(ignored_closure)) {
 ssize_t result;
 TTY_PUSHCOLOR(vtty_entry_color(VTTY_COLOR_WHITE,
                                VTTY_COLOR_DARK_GREY));
 result = tty_printer(data,datalen,NULL);
 TTY_POPCOLOR();
 return result;
}
#endif

#define HAVE_LIBC_ASSERTION_COREFAIL 1
INTERN ATTR_NORETURN ATTR_NOINLINE void LIBCCALL
libc_assertion_corefail(char const *expr, DEBUGINFO_MUNUSED,
                        char const *format, va_list args) {
 static int in_core = 0;
#ifdef PREEMPTION_DISABLE
 PREEMPTION_DISABLE();
#endif
 if (ATOMIC_XCH(in_core,1) == 0) {
  debug_printf("\n\n%s(%d) : %q : Assertion failed : %q\n",
                     __file,__line,__func,expr);
  if (format) {
   debug_vprintf(format,args);
   debug_print("\n",1,NULL);
  }
  debug_tbprint(1);
#if defined(__KERNEL__) && 1
  { struct task *start,*iter;
    iter = start = THIS_CPU->c_running;
    if (start) do {
     debug_printf("RUNNING TASK %p (PID = %d/%d) - %[file]\n",iter,
                  iter->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,
                  iter->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
                  iter->t_real_mman->m_inst ? iter->t_real_mman->m_inst->i_module->m_file : NULL);
     if (iter == THIS_TASK) {
#undef debug_tbprint
      libc_debug_tbprint();
     } else {
      debug_tbprintl((void *)iter->t_cstate->iret.xip,NULL,0);
      debug_tbprint2((void *)iter->t_cstate->gp.xbp,0);
     }
    } while ((iter = iter->t_sched.sd_running.re_next) != start);
    for (iter = THIS_CPU->c_idling;
         iter; iter = iter->t_sched.sd_running.re_next) {
     debug_printf("IDLING TASK %p (PID = %d/%d) - %[file]\n",iter,
                  iter->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,
                  iter->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
                  iter->t_real_mman->m_inst ? iter->t_real_mman->m_inst->i_module->m_file : NULL);
     if (iter == THIS_TASK) {
#undef debug_tbprint
      libc_debug_tbprint();
     } else {
      debug_tbprintl((void *)iter->t_cstate->iret.xip,NULL,0);
      debug_tbprint2((void *)iter->t_cstate->gp.xbp,0);
     }
    }
    task_crit();
    if (cpu_tryread(THIS_CPU)) {
     for (iter = THIS_CPU->c_suspended; iter;
          iter = iter->t_sched.sd_suspended.le_next) {
      debug_printf("SUSPENDED TASK %p (PID = %d/%d) - %[file]\n",iter,
                   iter->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,
                   iter->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
                   iter->t_real_mman->m_inst ? iter->t_real_mman->m_inst->i_module->m_file : NULL);
      debug_tbprintl((void *)iter->t_cstate->iret.xip,NULL,0);
      debug_tbprint2((void *)iter->t_cstate->gp.xbp,0);
     }
     for (iter = THIS_CPU->c_sleeping; iter;
          iter = iter->t_sched.sd_sleeping.le_next) {
      debug_printf("SLEEPING TASK %p (PID = %d/%d) - %[file]\n",iter,
                   iter->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,
                   iter->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
                   iter->t_real_mman->m_inst ? iter->t_real_mman->m_inst->i_module->m_file : NULL);
      debug_tbprintl((void *)iter->t_cstate->iret.xip,NULL,0);
      debug_tbprint2((void *)iter->t_cstate->gp.xbp,0);
     }
     cpu_endread(THIS_CPU);
    }
    task_endcrit();
  }
  debug_printf("==DONE\n");
#endif
 } else if (ATOMIC_XCH(in_core,2) == 1) {
  char buffer[__SIZEOF_POINTER__*2],*iter;
  uintptr_t return_address;
  debug_print("\n\nASSERTION CORE RECURSION\n",27,NULL);
#ifdef __x86_64__
  __asm__ __volatile__("movq 8(%%rbp), %0\n" : "=r" (return_address) : : "memory");
#else
  __asm__ __volatile__("movl 4(%%ebp), %0\n" : "=r" (return_address) : : "memory");
#endif
  iter = COMPILER_ENDOF(buffer);
  while (iter-- != buffer) {
   uintptr_t temp;
   temp = return_address % 16;
   return_address /= 16;
   if (temp >= 10) *iter = (char)('A'+(temp-10));
   else            *iter = (char)('0'+temp);
  }
  ++iter;
  debug_print("EIP = ",6,NULL);
  debug_print(iter,COMPILER_ENDOF(buffer)-iter,NULL);
  debug_print("\n",1,NULL);
 }
#ifndef __KERNEL__
 __asm__ __volatile__("int $3\n");
#endif
#ifdef PREEMPTION_FREEZE
 PREEMPTION_FREEZE();
#else
 for (;;) {} /* TODO: abort() */
#endif
}

struct stackframe {
 struct stackframe *sf_caller;
 void              *sf_return;
};
#define HAVE_LIBC_DEBUG_TBPRINT2 1
#ifdef __KERNEL__
PRIVATE ATTR_USED
void (ATTR_STDCALL libc___assertion_tbprint2_impl)(void const *ebp, size_t n_skip);

#ifdef __x86_64__
GLOBAL_ASM(
L(.section .text                                                 )
L(INTERN_ENTRY(libc_debug_tbprint2)                              )
L(    pushq %rbp                                                 )
L(    movq  %rsp, %rbp                                           )
L(    __ASM_PUSH_SCRATCH                                         )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %rax                  )
L(    pushx_sym(%r10,1f)                                         )
L(    pushq $(EXC_PAGE_FAULT)                                    )
L(    pushq TASK_OFFSETOF_IC(%rax)                               )
L(    movq  %rsp, TASK_OFFSETOF_IC(%rax)                         )
L(                                                               )
L(    call  libc___assertion_tbprint2_impl                       )
L(                                                               )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %rax                  )
L(    popq  TASK_OFFSETOF_IC(%rax)                               )
L(    addq  $16, %rsp                                            )
L(1:  __ASM_POP_SCRATCH                                          )
L(    leave                                                      )
L(    ret                                                        )
L(SYM_END(libc_debug_tbprint2)                                   )
L(.previous                                                      )
);
#elif defined(__i386__)
GLOBAL_ASM(
L(.section .text                                                 )
L(INTERN_ENTRY(libc_debug_tbprint2)                              )
L(    pushl %ebp                                                 )
L(    movl  %esp, %ebp                                           )
L(    __ASM_PUSH_SCRATCH                                         )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %eax                  )
L(    pushl $1f                                                  )
L(    pushl $(EXC_PAGE_FAULT)                                    )
L(    pushl TASK_OFFSETOF_IC(%eax)                               )
L(    movl  %esp, TASK_OFFSETOF_IC(%eax)                         )
L(                                                               )
L(    pushl 12(%ebp) /* n_skip */                                )
L(    pushl 8(%ebp) /* ebp */                                    )
L(    call  libc___assertion_tbprint2_impl                       )
L(                                                               )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %eax                  )
L(    popl  TASK_OFFSETOF_IC(%eax)                               )
L(    addl  $8, %esp                                             )
L(1:  __ASM_POP_SCRATCH                                          )
L(    leave                                                      )
L(    ret $8                                                     )
L(SYM_END(libc_debug_tbprint2)                                   )
L(.previous                                                      )
);
#else
#error "Unsupported arch"
#endif

PRIVATE void (ATTR_STDCALL libc___assertion_tbprint2_impl)(void const *ebp, size_t n_skip)
#else
INTERN void (LIBCCALL libc_debug_tbprint2)(void const *ebp, size_t n_skip)
#endif
{
 struct stackframe *iter2,*iter;
 size_t check_id,tb_id = 0,skipped = 0;
 iter = (struct stackframe *)ebp;
 while (iter) {
  if (!OK_HOST_DATA(iter,sizeof(struct stackframe))) break;
  if (tb_id != 0) {
   iter2 = (struct stackframe *)ebp;
   for (check_id = 0;;) {
    if (iter2 == iter) return; /* Prevent infinite loop on stack-overflow */
    if (++check_id == tb_id) break;
    iter2 = iter2->sf_caller;
   }
  }
#if 1
  if (skipped < n_skip)
   ++skipped;
  else
#endif
  {
   /* Print one traceback line. */
   libc_debug_tbprintl(iter->sf_return,iter,tb_id-n_skip);
  }
  iter = iter->sf_caller;
  ++tb_id;
#ifndef __KERNEL__
  break;
#endif
 }
}

DECL_END

#endif /* !GUARD_LIBS_LIBC_ARCH_I386_KOS_ASSERT_C_INL */
