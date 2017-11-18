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
#ifndef GUARD_KERNEL_ARCH_USER32_C_INL
#define GUARD_KERNEL_ARCH_USER32_C_INL 1
#define _KOS_SOURCE 2

#include <hybrid/host.h>
#ifndef __x86_64__
#include <assert.h>
#include <hybrid/align.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/panic.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <kernel/interrupt.h>
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
L(    /* Copy memory (s.a.: `struct intchain') */                )
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
L(    /* Fill memory (s.a.: `struct intchain') */                )
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
L(    jb    1b /* if (str < __kernel_user_start) return NULL; */ )
L(    cmpl  $__kernel_user_end, %edi                             )
L(    jae   1b /* if (str >= __kernel_user_end) return NULL; */  )
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
L(    /* Check for `str' apart of user-share */                  )
L(3:  cmpl  $__kernel_user_start, %esi                           )
L(    jb    1b /* if (str < __kernel_user_start) return NULL; */ )
L(    cmpl  $__kernel_user_end, %esi                             )
L(    jae   1b /* if (str >= __kernel_user_end) return NULL; */  )
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

#define I0(x) /* Nothing */
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

DECL_END
#endif /* !__x86_64__ */

#endif /* !GUARD_KERNEL_ARCH_USER32_C_INL */
