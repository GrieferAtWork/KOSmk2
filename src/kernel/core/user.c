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
#ifndef GUARD_KERNEL_CORE_USER_C
#define GUARD_KERNEL_CORE_USER_C 1
#define _KOS_SOURCE 2

#include <assert.h>
#include <hybrid/align.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/panic.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <kernel/user.h>
#include <malloc.h>
#include <sched/task.h>
#include <stdarg.h>
#include <string.h>
#include <sys/mman.h>
#include <syslog.h>

DECL_BEGIN

#ifndef __i386__
#error FIXME
#endif

#define ARG(o) ((o)+12)(%esp)
#define SAFE_REGS pushl %edi; pushl %esi;
#define LOAD_REGS popl  %esi; popl  %edi;
GLOBAL_ASM(
L(.section .text                                                 )
L(.align CACHELINE                                               )
L(PUBLIC_ENTRY(copy_to_user)                                     )
L(    SAFE_REGS                                                  )
L(    movl  ARG(0), %edi                                         )
L(    movl  ARG(4), %esi                                         )
L(    movl  ARG(8), %ecx                                         )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %eax                  )
L(    movl  %edi, %edx                                           )
L(    addl  %ecx, %edx                                           )
L(    jo    1f                                                   )
L(    cmpl  TASK_OFFSETOF_ADDRLIMIT(%eax), %edx                  )
L(    jae   1f                                                   )
L(    jmp   4f                                                   )
L(SYM_END(copy_to_user)                                          )
L(PUBLIC_ENTRY(copy_from_user)                                   )
L(    SAFE_REGS                                                  )
L(    movl  ARG(0), %edi                                         )
L(    movl  ARG(4), %esi                                         )
L(    movl  ARG(8), %ecx                                         )
L(    /* Load the currently running task */                      )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %eax                  )
L(                                                               )
L(    /* Make sure that EDI is located in user-space */          )
L(2:  movl  %esi, %edx                                           )
L(3:  addl  %ecx, %edx                                           )
L(    jo    1f                                                   )
L(    cmpl  TASK_OFFSETOF_ADDRLIMIT(%eax), %edx                  )
L(    jae   3f                                                   )
L(                                                               )
L(    /* Create interrupt chain entry for handling pagefaults. */)
L(    /* NOTE: The handler isn't executed for ALLOA/COW faults! */)
L(4:  pushl $1f                                                  )
L(    pushl $(EXC_PAGE_FAULT)                                    )
L(    pushl TASK_OFFSETOF_IC(%eax)                               )
L(    movl  %esp, TASK_OFFSETOF_IC(%eax)                         )
L(                                                               )
L(    /* Copy memory (s.a.: 'struct intchain') */                )
L(    rep   movsb                                                )
L(                                                               )
L(    /* Cleanup the custom interrupt handler */                 )
L(    popl  TASK_OFFSETOF_IC(%eax)                               )
L(    addl  $8, %esp                                             )
L(                                                               )
L(1:  LOAD_REGS                                                  )
L(    /* Load the amount of bytes not copied into EAX */         )
L(    /* NOTE: When rep finished without faulting, ECX is 0 */   )
L(    movl  %ecx, %eax                                           )
L(    ret   $12                                                  )
L(    /* Check if the user-range is part of user-share memory */ )
L(3:  cmpl  $__kernel_user_start, %esi                           )
L(    jb    1b /* if (dst < __kernel_user_start) fail(); */      )
L(    cmpl  $__kernel_user_end,   %edx                           )
L(    jae   1b /* if (dst >= __kernel_user_end) fail(); */       )
L(    jmp   4b                                                   )
L(SYM_END(copy_from_user)                                        )
L(PUBLIC_ENTRY(copy_in_user)                                     )
L(    SAFE_REGS                                                  )
L(    movl  ARG(0), %edi                                         )
L(    movl  ARG(4), %esi                                         )
L(    movl  ARG(8), %ecx                                         )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %eax                  )
L(    movl  %edi, %edx                                           )
L(    addl  %ecx, %edx                                           )
L(    jo    1b                                                   )
L(    cmpl  TASK_OFFSETOF_ADDRLIMIT(%eax), %edx                  )
L(    jnae  2b                                                   )
L(    jmp   1b                                                   )
L(SYM_END(copy_in_user)                                          )
L(.previous                                                      )
);
#undef LOAD_REGS
#undef SAFE_REGS
#undef ARG

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(memset_user)                                      )
L(    pushl %edi                                                 )
L(    pushl %esi                                                 )
L(    movl 12(%esp), %edi /* dst */                              )
L(    movl 16(%esp), %eax /* byte */                             )
L(    movl 20(%esp), %ecx /* n_bytes */                          )
L(                                                               )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %edx                  )
L(                                                               )
L(    /* Make sure that EDI is located in user-space */          )
L(    movl  %edi, %esi                                           )
L(    addl  %ecx, %esi                                           )
L(    jo    1f                                                   )
L(    cmpl  TASK_OFFSETOF_ADDRLIMIT(%edx), %esi                  )
L(    jae   1f                                                   )
L(                                                               )
L(    /* Create interrupt chain entry for handling pagefaults. */)
L(    pushl $1f                                                  )
L(    pushl $(EXC_PAGE_FAULT)                                    )
L(    pushl TASK_OFFSETOF_IC(%edx)                               )
L(    movl  %esp, TASK_OFFSETOF_IC(%edx)                         )
L(                                                               )
L(    /* Fill memory (s.a.: 'struct intchain') */                )
L(    rep   stosb                                                )
L(                                                               )
L(    /* Cleanup the custom interrupt handler */                 )
L(    popl  TASK_OFFSETOF_IC(%edx)                               )
L(    addl  $8, %esp                                             )
L(                                                               )
L(1:  movl  %ecx, %eax /* Return the amount of missing bytes */  )
L(    popl  %esi                                                 )
L(    popl  %edi                                                 )
L(    ret   $12                                                  )
L(SYM_END(memset_user)                                           )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(strend_user)                                      )
L(    xorl  %eax, %eax /* Scan for ZERO-bytes & error-return */  )
L(    pushl %edi                                                 )
L(    movl  8(%esp), %edi /* str */                              )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %edx                  )
L(    movl  TASK_OFFSETOF_ADDRLIMIT(%edx), %ecx                  )
L(    cmpl  %ecx, %edi                                           )
L(    jae   2f /* if (str >= addr_limit) goto end; */            )
L(    subl  %edi, %ecx                                           )
L(3:  pushl $1f                                                  )
L(    pushl $(EXC_PAGE_FAULT)                                    )
L(    pushl TASK_OFFSETOF_IC(%edx)                               )
L(    movl  %esp, TASK_OFFSETOF_IC(%edx)                         )
L(    repne scasb                                                )
L(    popl  TASK_OFFSETOF_IC(%edx)                               )
L(    addl  $8, %esp                                             )
L(    leal -1(%edi), %eax /* EAX = EDI-1; */                     )
L(1:  popl  %edi                                                 )
L(    ret   $4                                                   )
L(2:  cmpl  $__kernel_user_start, %edi                           )
L(    jb    1b /* if (edi < __kernel_user_start) return NULL; */ )
L(    cmpl  $__kernel_user_end, %edi                             )
L(    jae   1b /* if (edi >= __kernel_user_end) return NULL; */  )
L(    movl  $__kernel_user_end, %ecx                             )
L(    subl  %edi, %ecx                                           )
L(    jmp   3b                                                   )
L(SYM_END(strend_user)                                           )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(stpncpy_from_user)                                )
L(    pushl %esi                                                 )
L(    pushl %edi                                                 )
L(    movl  12(%esp), %edi /* dst */                             )
L(    movl  16(%esp), %esi /* str */                             )
L(    movl  20(%esp), %ecx /* max_chars */                       )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %edx                  )
L(    movl  TASK_OFFSETOF_ADDRLIMIT(%edx), %eax                  )
L(    cmpl  %eax, %esi                                           )
L(    jae   3f /* if (str >= addr_limit) return NULL; */         )
L(    subl  %esi, %eax                                           )
L(4:  cmpl  %ecx, %eax                                           )
L(    cmovb %eax, %ecx /* if (EAX < ECX) ECX = EAX */            )
L(                                                               )
L(    pushl $1f                                                  )
L(    pushl $(EXC_PAGE_FAULT)                                    )
L(    pushl TASK_OFFSETOF_IC(%edx)                               )
L(    movl  %esp, TASK_OFFSETOF_IC(%edx)                         )
L(                                                               )
L(    /* Copy up to ECX bytes from ESI to EDI, stopping at NUL */)
L(7:  lodsb /* AL = *ESI++; */                                   )
L(    stosb /* *EDI++ = AL; */                                   )
L(    testb %al, %al                                             )
L(    jnz   7b /* if (AL != '\0') continue; */                   )
L(                                                               )
L(    leal  -1(%edi), %eax /* Return a pointer to the new NUL-byte. */)
L(    popl  TASK_OFFSETOF_IC(%edx)                               )
L(    addl  $8, %esp                                             )
L(                                                               )
L(2:  popl  %edi                                                 )
L(    popl  %esi                                                 )
L(    ret   $12                                                  )
L(1:  xorl  %eax, %eax /* return NULL; */                        )
L(    jmp 2b                                                     )
L(    /* Check for 'str' apart of user-share */                  )
L(3:  cmpl  $__kernel_user_start, %esi                           )
L(    jb    1b /* if (esi < __kernel_user_start) return NULL; */ )
L(    cmpl  $__kernel_user_end, %esi                             )
L(    jae   1b /* if (esi >= __kernel_user_end) return NULL; */  )
L(    movl  $__kernel_user_end, %eax                             )
L(    subl  %esi, %eax                                           )
L(    jmp 4b                                                     )
L(SYM_END(stpncpy_from_user)                                     )
L(.previous                                                      )
);


//FUNDEF SAFE ssize_t (ATTR_CDECL call_user_worker)(void *__restrict worker, size_t argc, ...);
GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(call_user_worker)                                 )
L(    pushl %esi                                                 )
L(    pushl %edi                                                 )
L(    movl  12(%esp), %eax /* worker */                          )
L(    movl  16(%esp), %ecx /* argc */                            )
L(    leal  20(%esp), %esi /* First argument */                  )
L(    pushl %ebp                                                 )
L(                                                               )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %edx                  )
L(    pushl $1f                                                  )
L(    pushl $(EXC_PAGE_FAULT)                                    )
L(    pushl TASK_OFFSETOF_IC(%edx)                               )
L(    movl  %esp, TASK_OFFSETOF_IC(%edx)                         )
L(                                                               )
#ifdef CONFIG_DEBUG
L(    pushl 28(%esp)                                             )
L(    pushl %ebp                                                 )
#endif
L(    movl  %esp, %ebp                                           )
L(                                                               )
L(    /* Copy the argument list. */                              )
L(    negl  %ecx                                                 )
L(    leal 0(%esp,%ecx,4), %esp /* ESP -= ECX*4; */              )
L(    negl  %ecx                                                 )
L(    movl  %esp, %edi                                           )
L(    rep   movsl /* Copy ECX*4 bytes of argument data from ESI to EDI */)
L(                                                               )
L(    call *%eax /* worker(...) */                               )
L(                                                               )
#ifdef CONFIG_DEBUG
L(    leal  8(%ebp), %esp /* Restore the old stack. */           )
#else
L(    movl  %ebp, %esp /* Restore the old stack. */              )
#endif
L(                                                               )
L(    /* NOTE: Leave 'EDX' alive for 64-bit return values. */    )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %ecx                  )
L(    popl  TASK_OFFSETOF_IC(%ecx)                               )
L(    addl  $8, %esp                                             )
L(                                                               )
L(2:  popl  %ebp                                                 )
L(    popl  %edi                                                 )
L(    popl  %esi                                                 )
L(    ret                                                        )
L(1:  movl $-EFAULT, %eax                                        )
L(    jmp 2b                                                     )
L(SYM_END(call_user_worker)                                      )
L(.previous                                                      )
);


/* IOS stands for InOutString... */
#define DEFINE_IOS(name,io_ins,ifb,ifl,preg,n) \
GLOBAL_ASM( \
L(.section .text                                                 ) \
L(PUBLIC_ENTRY(name)                                             ) \
L(    pushl %esi                                                 ) \
L(    pushl %edi                                                 ) \
L(    movl 20(%esp), %ecx /* count */                            ) \
L(                                                               ) \
L(    /* Validate limits */                                      ) \
ifb(L(movl 16(%esp), preg /* addr */                             )) \
ifb(L(addl  %ecx, preg                                           )) \
ifb(L(jo    1f /* if (DOES_OVERFLOW(addr+count)) return count; */)) \
ifl(L(movl 16(%esp), %edx /* addr */                             )) \
ifl(L(leal  0(%edx,%ecx,n), preg /* EDI = addr+count*n; */       )) \
ifl(L(cmpl  %edx, preg                                           )) \
ifl(L(jbe   1f /* if (addr+count*n >= addr) return count; */     )) \
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %eax                  ) \
L(    movl  TASK_OFFSETOF_ADDRLIMIT(%eax), %edx                  ) \
L(    cmpl  %edx, preg                                           ) \
L(    jae   1f /* if (addr+count >= addr_limit) return count; */ ) \
L(                                                               ) \
L(    /* (re-)load remaining registers */                        ) \
L(    movl  16(%esp), preg /* addr */                            ) \
L(    movw  12(%esp), %dx  /* port */                            ) \
L(                                                               ) \
L(    /* Push an exception handler. */                           ) \
L(    pushl $1f                                                  ) \
L(    pushl $(EXC_PAGE_FAULT)                                    ) \
L(    pushl TASK_OFFSETOF_IC(%eax)                               ) \
L(    movl  %esp, TASK_OFFSETOF_IC(%eax)                         ) \
L(                                                               ) \
L(    /* while (count--) *addr++ = inN(port); */                 ) \
L(    /* while (count--) outN(port,*addr++); */                  ) \
L(    rep io_ins                                                 ) \
L(                                                               ) \
L(    /* Cleanup on success */                                   ) \
L(    popl  TASK_OFFSETOF_IC(%eax)                               ) \
L(    addl  $8, %esp                                             ) \
L(                                                               ) \
L(1:  popl %edi                                                  ) \
L(    popl %esi                                                  ) \
L(    movl %ecx, %eax /* Return the number of bytes not transferred */) \
L(    ret $12                                                    ) \
L(SYM_END(name)                                                  ) \
L(.previous                                                      ) \
)

#define I0(x) /* nothing */
#define I1(x) x
/* Define user-buffered I/O string functions. */
DEFINE_IOS(insb_user,insb,I1,I0,%edi,1);
DEFINE_IOS(insw_user,insw,I0,I1,%edi,2);
DEFINE_IOS(insl_user,insl,I0,I1,%edi,4);
DEFINE_IOS(outsb_user,outsb,I1,I0,%esi,1);
DEFINE_IOS(outsw_user,outsw,I0,I1,%esi,2);
DEFINE_IOS(outsl_user,outsl,I0,I1,%esi,4);
#undef DEFINE_IOS
#undef I1
#undef I0



DATDEF byte_t __kernel_user_start[];
INTDEF byte_t __kernel_user_end[];

PUBLIC bool (KCALL addr_isuser)(void const *addr, size_t len) {
 uintptr_t endaddr;
 if (__builtin_add_overflow((uintptr_t)addr,len,&endaddr))
     return false;
 if ((uintptr_t)endaddr <= THIS_TASK->t_addrlimit)
      return true;
 if ((uintptr_t)addr    >= (uintptr_t)__kernel_user_start &&
     (uintptr_t)endaddr <= (uintptr_t)__kernel_user_end)
      return true;
 return false;
}



PUBLIC ssize_t ATTR_CDECL
sprintf_user(USER char *dst, char const *format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = vsprintf_user(dst,format,args);
 va_end(args);
 return result;
}
PUBLIC ssize_t ATTR_CDECL
snprintf_user(USER char *dst, size_t dst_max, char const *format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = vsnprintf_user(dst,dst_max,format,args);
 va_end(args);
 return result;
}


PRIVATE ssize_t KCALL
vsprintf_user_printer(char const *__restrict data,
                      size_t datalen,
                      USER char *HOST *ptarget) {
 if (copy_to_user(*ptarget,data,datalen))
     return -EFAULT;
 *ptarget += datalen;
 return datalen;
}
PUBLIC ssize_t KCALL
vsprintf_user(USER char *dst, char const *format, va_list args) {
 ssize_t result;
 result = format_printf((pformatprinter)&vsprintf_user_printer,
                        &dst,format,args);
 if (E_ISOK(result) && copy_to_user(dst,"",1*sizeof(char)))
     result = -EFAULT;
 return result;
}

struct snprintf_data {
 USER char *bufpos;
 USER char *bufend;
};
PRIVATE ssize_t LIBCCALL
snprintf_callback(char const *__restrict data, size_t datalen,
                  struct snprintf_data *__restrict buffer) {
 /* Don't exceed the buffer end */
 if (buffer->bufpos < buffer->bufend) {
  size_t maxwrite = (size_t)(buffer->bufend-buffer->bufpos);
  if (copy_to_user(buffer->bufpos,data,MIN(maxwrite,datalen)))
      return -EFAULT;
 }
 /* Still seek past the end, as to
  * calculate the required buffersize. */
 buffer->bufpos += datalen;
 return datalen;
}
PUBLIC ssize_t KCALL
vsnprintf_user(USER char *s, size_t maxlen,
               char const *__restrict format, va_list args) {
 struct snprintf_data data; ssize_t result;
 data.bufend = (data.bufpos = s)+maxlen;
 result = format_vprintf((pformatprinter)&snprintf_callback,&data,format,args);
 if (E_ISOK(result)) {
  ++result;
  if (data.bufpos < data.bufend &&
      copy_to_user(data.bufpos,"",sizeof(char)))
      result = -EFAULT;
 }
 return result;
}


#define STATE_VM_SINGLE  0
#define STATE_VM_STRDUP  1
#define STATE_VM_SUSPEND 2

PUBLIC SAFE VIRT char *KCALL
acquire_string(USER char const *str, size_t max_length,
               size_t *opt_pstrlen, int *__restrict pstate) {
 struct mman *mm = THIS_MMAN;
 /* NOTE: We need a write-lock in case accessing the string causes a #PF. */
 char *result = E_PTR(mman_write(mm));
 char *string_end; size_t string_length;
 if (E_ISERR(result)) return result;
 /* Figure out where the string ends, thus validating
  * it and figuring out its initial length. */
 string_end = strend_user(str);
 if unlikely(!string_end) { result = E_PTR(-EFAULT); goto end; }
 string_length = (size_t)(string_end-str);
 assert(mm->m_tasks != NULL);
 if (mm->m_tasks == THIS_TASK &&
    !THIS_TASK->t_mman_tasks.le_next) {
  /* Simple case: The calling process is single-threaded, so we've
   *              got an implicit single-user lock on the VM. */
  result  = (char *)str;
  *pstate = STATE_VM_SINGLE;
  if (opt_pstrlen) *opt_pstrlen = string_length;
 } else {
  if unlikely(string_length > max_length) { result = E_PTR(-EINVAL); goto end; }
#if 0 /* Currently this would have the potential to deadlock.
       * >> Instead, we must introduce a new system that allows 'ERESTART'
       *    to be returned by `task_waitfor()' alongside 'EINTR'.
       *    Both should have the same meaning, but 'ERESTART' would
       *    be returned if the task was suspended.
       * >> Essentially, we can't just randomly suspend a task while it is in kernel-space.
       *    Well... We could, but doing so introduces the problematic race-condition
       *    of the task being suspended currently holding some lock that we as the
       *    task having suspended them attempt to acquire at a later point, yet before
       *    we're ready to resume them.
       * >> As already noted, the solution is to introduce a new flag for 'task_suspend'
       *    that will interrupt a task that is currently in kernel-space using 'ERESTART', as
       *    well as override its system-call return address similar to how signal handlers work,
       *    with an internal function that will notify the task attempting the suspend that
       *    the task should now be considered suspended, before actually switching its scheduler
       *    to being suspended internally.
       * >> Later, when the task is resumed, execution will return to user-space,
       *    at which point either LIBC, or yet another piece of kernel code can
       *    restart the system call.
       * #define TASK_SUSP_RESTART 0x80
       */
#define SUSPEND_ALL_THRESHOLD (4*PAGESIZE)
  size_t threshold = 0;
  struct task *iter;
#if 1
  atomic_rwlock_read(&mm->m_tasks_lock);
#else
  /* Only try to lock the mman's task chain twice. */
  if (!atomic_rwlock_read(&mm->m_tasks_lock)) {
   task_yield();
   if (!atomic_rwlock_tryread(&mm->m_tasks_lock))
       goto copy_string;
  }
#endif
  iter = mm->m_tasks;
  do {
   assert(iter);
   threshold += string_length;
   if (threshold >= SUSPEND_ALL_THRESHOLD) {
    /*  */
    iter = mm->m_tasks;
    do {
     if (iter != THIS_TASK) {
      result = E_PTR(task_suspend(iter,TASK_SUSP_REC|TASK_SUSP_HOST|TASK_SUSP_RESTART));
      if (E_ISERR(result)) {
       struct task *iter2;
       /* Something went wrong. - Lets just copy the string instead. */
       for (iter2 = mm->m_tasks; iter2 != iter;
            iter2 = iter2->t_mman_tasks.le_next)
            task_resume(iter2,TASK_SUSP_REC|TASK_SUSP_HOST);
       atomic_rwlock_endread(&mm->m_tasks_lock);
       goto copy_string;
      }
     }
    } while ((iter = iter->t_mman_tasks.le_next) != NULL);
    /* With single-user safety now active, reload the string's ending. */
    string_end = strend_user(str);
    if unlikely(!string_end) {
     /* *sigh*... */
     iter = mm->m_tasks;
     do {
      if (iter != THIS_TASK)
          task_resume(iter,TASK_SUSP_REC|TASK_SUSP_HOST);
     } while ((iter = iter->t_mman_tasks.le_next) != NULL);
     atomic_rwlock_endread(&mm->m_tasks_lock);
     result = E_PTR(-EFAULT);
     goto end;
    }
    atomic_rwlock_endread(&mm->m_tasks_lock);
    if (opt_pstrlen) *opt_pstrlen = string_end-str;
    *pstate = STATE_VM_SUSPEND;
    result = (char *)str;
    goto end;
   }
  } while ((iter = iter->t_mman_tasks.le_next) != NULL);
  atomic_rwlock_endread(&mm->m_tasks_lock);
#endif
  if unlikely(!string_length) {
   /* Special case: Empty string */
   if (opt_pstrlen) *opt_pstrlen = 0;
   *pstate = STATE_VM_SINGLE;
   result = (char *)"";
  } else {
   char *result_end; size_t new_strlen;
copy_string: ATTR_UNUSED;
   result = (char *)kmalloc(string_length,GFP_SHARED);
   if unlikely(!result) { result = E_PTR(-ENOMEM); goto end; }
   result_end = stpncpy_from_user(result,str,string_length);
   /* Shouldn't happen, but checked anyways. */
   if unlikely(!result_end) { kfree(result); result = E_PTR(-EFAULT); goto end; }
   new_strlen = result_end-result;
   assert(new_strlen <= string_length);
   /* Truncate our string copy if user-space modified the string in the meantime. */
   if unlikely(new_strlen != string_length) {
    char *new_result;
    new_result = trealloc(char,result,new_strlen+1);
    if (new_result) result = new_result;
   }
   if (opt_pstrlen) *opt_pstrlen = new_strlen;
   *pstate = STATE_VM_STRDUP;
  }
 }
end:
 mman_endwrite(mm);
 return result;
}
PUBLIC SAFE void KCALL
release_string(VIRT char *__restrict virt_str, int state) {
 assert(TASK_ISSAFE());
 switch (state) {
 case STATE_VM_SINGLE:
  break;
 case STATE_VM_STRDUP:
  free(virt_str);
  break;

 {
  struct mman *mm;
  struct task *iter;
 case STATE_VM_SUSPEND:
  mm = THIS_MMAN;
  atomic_rwlock_read(&mm->m_tasks_lock);
  for (iter = mm->m_tasks; iter;
       iter = iter->t_mman_tasks.le_next) {
   if (iter != THIS_TASK)
       task_resume(iter,TASK_SUSP_HOST|TASK_SUSP_REC);
  }
  atomic_rwlock_endread(&mm->m_tasks_lock);
 } break;

 default:
  assertf(0,"Invalid state: %d(%x)\n",state,state);
 }
}

PUBLIC SAFE ATTR_MALLOC char *KCALL
copy_string(USER char const *str, size_t max_length, size_t *opt_pstrlen) {
 struct mman *mm = THIS_MMAN;
 /* NOTE: We need a write-lock in case accessing the string causes a #PF. */
 char *result = E_PTR(mman_write(mm));
 char *string_end; size_t string_length;
 if (E_ISERR(result)) return result;
 /* Figure out where the string ends, thus validating
  * it and figuring out its initial length. */
 string_end = strend_user(str);
 if unlikely(!string_end) { result = E_PTR(-EFAULT); goto end; }
 string_length = (size_t)(string_end-str);
 assert(mm->m_tasks != NULL);
 /* Make sure the string's length isn't too large. */
 if unlikely(string_length > max_length) { result = E_PTR(-EINVAL); goto end; }
 if (mm->m_tasks == THIS_TASK && !THIS_TASK->t_mman_tasks.le_next) {
  /* Simple case: The calling process is single-threaded,
   *              so there's no chance the string may change. */
  result = (char *)memdup(str,(max_length+1)*sizeof(char));
  if (result) result[max_length] = '\0';
  if (opt_pstrlen) *opt_pstrlen = string_length;
 } else {
  char *result_end; size_t new_strlen;
copy_string: ATTR_UNUSED;
  result = (char *)kmalloc(string_length,GFP_SHARED);
  if unlikely(!result) { result = E_PTR(-ENOMEM); goto end; }
  result_end = stpncpy_from_user(result,str,string_length);
  /* Shouldn't happen, but checked anyways. */
  if unlikely(!result_end) { kfree(result); result = E_PTR(-EFAULT); goto end; }
  new_strlen = result_end-result;
  assert(new_strlen <= string_length);
  /* Truncate our string copy if user-space modified the string in the meantime. */
  if unlikely(new_strlen != string_length) {
   char *new_result = trealloc(char,result,new_strlen+1);
   if (new_result) result = new_result;
  }
  if (opt_pstrlen) *opt_pstrlen = new_strlen;
 }
end:
 mman_endwrite(mm);
 return result;
}



PUBLIC HOST byte_t *usershare_writable;
DATDEF byte_t __kernel_user_start[];
INTDEF byte_t __kernel_user_end[];
INTDEF byte_t __kernel_user_size[];

PRIVATE struct mregion usershare_region = {
#ifdef CONFIG_DEBUG
    .mr_refcnt = 1,
#else
    .mr_refcnt = 0x80000001,
#endif
    .mr_type   = MREGION_TYPE_PHYSICAL,
    .mr_init   = MREGION_INIT_RAND, /* Actually `MREGION_INIT_FILE', but we don't have the file... */
    .mr_size   = (size_t)__kernel_user_size,
    .mr_futex  = {{0}},
    .mr_plock  = RWLOCK_INIT,
    .mr_parts  = &usershare_region.mr_part0,
    .mr_part0  = {
        .mt_chain = {
            .le_pself = &usershare_region.mr_parts,
            .le_next  = NULL,
        },
        .mt_start  = 0,
        .mt_refcnt = 1,
        .mt_state  = MPART_STATE_INCORE,
        .mt_flags  = MPART_FLAG_NONE,
        .mt_locked = 1,
        .mt_memory = {
            .m_next  = NULL,
            .m_start = (ppage_t)virt_to_phys(__kernel_user_start),
            .m_size  = (size_t)__kernel_user_size,
        },
    },
    .mr_global = {NULL,NULL},
};

PRIVATE MODULE_INIT void KCALL
usershare_writable_initialize(void) {
 ppage_t map_address; errno_t error;
 assert(IS_ALIGNED((uintptr_t)__kernel_user_start,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)__kernel_user_end,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)__kernel_user_size,PAGESIZE));
 assert(THIS_MMAN == &mman_kernel);
 task_nointr();
 mman_write(&mman_kernel);
 map_address = mman_findspace_unlocked(&mman_kernel,
                                      (ppage_t)(KERNEL_BASE-(uintptr_t)__kernel_user_size),
                                      (uintptr_t)__kernel_user_size,PAGESIZE,0,
                                       MMAN_FINDSPACE_BELOW);
 if unlikely(map_address == PAGE_ERROR) {
  map_address = mman_findspace_unlocked(&mman_kernel,(ppage_t)KERNEL_BASE,
                                       (uintptr_t)__kernel_user_size,PAGESIZE,0,
                                        MMAN_FINDSPACE_ABOVE);
  if unlikely(map_address == PAGE_ERROR) {
   error = -ENOMEM;
err:
   PANIC(FREESTR("Failed to find suitable location for writable user-share segment: %[errno]"),-error);
  }
 }
 error = mman_mmap_unlocked(&mman_kernel,map_address,
                           (uintptr_t)__kernel_user_size,0,
                            &usershare_region,
                            PROT_READ|PROT_WRITE|PROT_SHARED,
                            NULL,NULL);
 if (E_ISERR(error)) goto err;
 usershare_writable = (HOST byte_t *)map_address;
 syslog(LOG_BOOT|LOG_INFO,
        "[USER] Mapped writable user-share segment to %p...%p (KPD)\n",
        map_address,(uintptr_t)map_address+(uintptr_t)__kernel_user_size-1);
 COMPILER_WRITE_BARRIER();
 mman_endwrite(&mman_kernel);
 task_endnointr();
}





DECL_END

#endif /* !GUARD_KERNEL_CORE_USER_C */
