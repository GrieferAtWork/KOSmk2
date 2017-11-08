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
#ifndef GUARD_KERNEL_CORE_ARCH_USER64_C_INL
#define GUARD_KERNEL_CORE_ARCH_USER64_C_INL 1
#define _KOS_SOURCE 2

#include <hybrid/host.h>
#ifdef __x86_64__
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

GLOBAL_ASM(
L(.section .text                                                 )
L(.align CACHELINE                                               )
L(PUBLIC_ENTRY(copy_to_user)                                     )
L(    movq  %rdx, %rcx                                           )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %rax                  )
L(    movq  %rdi, %rdx                                           )
L(    addq  %rcx, %rdx                                           )
L(    jo    1f                                                   )
L(    cmpq  TASK_OFFSETOF_ADDRLIMIT(%rax), %rdx                  )
L(    jae   1f                                                   )
L(    jmp   4f                                                   )
L(SYM_END(copy_to_user)                                          )
L(PUBLIC_ENTRY(copy_from_user)                                   )
L(    movq  %rdx, %rcx                                           )
L(    /* Load the currently running task */                      )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %rax                  )
L(                                                               )
L(    /* Make sure that EDI is located in user-space */          )
L(2:  movq  %rsi, %rdx                                           )
L(3:  addq  %rcx, %rdx                                           )
L(    jo    1f                                                   )
L(    cmpq  TASK_OFFSETOF_ADDRLIMIT(%rax), %rdx                  )
L(    jae   3f                                                   )
L(                                                               )
L(    /* Create interrupt chain entry for handling pagefaults. */)
L(    /* NOTE: The handler isn't executed for ALLOA/COW faults! */)
L(4:  leaq  1f(%rip), %rdx; pushq %rdx                           )
L(    pushq $(EXC_PAGE_FAULT)                                    )
L(    pushq TASK_OFFSETOF_IC(%rax)                               )
L(    movq  %rsp, TASK_OFFSETOF_IC(%rax)                         )
L(                                                               )
L(    /* Copy memory (s.a.: `struct intchain') */                )
L(    rep   movsb                                                )
L(                                                               )
L(    /* Cleanup the custom interrupt handler */                 )
L(    popq  TASK_OFFSETOF_IC(%rax)                               )
L(    addq  $16, %rsp                                            )
L(                                                               )
L(1:  /* Load the amount of bytes not copied into EAX */         )
L(    /* NOTE: When rep finished without faulting, ECX is 0 */   )
L(    movq  %rcx, %rax                                           )
L(    ret                                                        )
L(    /* Check if the user-range is part of user-share memory */ )
L(3:  leaq  __kernel_user_start(%rip), %r10                      )
L(    cmpq  %r10, %rsi                                           )
L(    jb    1b /* if (dst < __kernel_user_start) fail(); */      )
L(    leaq  __kernel_user_end(%rip), %r10                        )
L(    cmpq  %r10, %rdx                                           )
L(    jae   1b /* if (dst >= __kernel_user_end) fail(); */       )
L(    jmp   4b                                                   )
L(SYM_END(copy_from_user)                                        )
L(PUBLIC_ENTRY(copy_in_user)                                     )
L(    movq  %rdx, %rcx                                           )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %rax                  )
L(    movq  %rdi, %rdx                                           )
L(    addq  %rcx, %rdx                                           )
L(    jo    1b                                                   )
L(    cmpq  TASK_OFFSETOF_ADDRLIMIT(%rax), %rdx                  )
L(    jnae  2b                                                   )
L(    jmp   1b                                                   )
L(SYM_END(copy_in_user)                                          )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(memset_user)                                      )
L(    movq %rsi, %rax /* byte */                                 )
L(    movq %rdx, %rcx /* n_bytes */                              )
L(                                                               )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %rdx                  )
L(                                                               )
L(    /* Make sure that EDI is located in user-space */          )
L(    movq  %rdi, %rsi                                           )
L(    addq  %rcx, %rsi                                           )
L(    jo    1f                                                   )
L(    cmpq  TASK_OFFSETOF_ADDRLIMIT(%rdx), %rsi                  )
L(    jae   1f                                                   )
L(                                                               )
L(    /* Create interrupt chain entry for handling pagefaults. */)
L(    leaq  1f(%rip), %r10; pushq %r10                           )
L(    pushq $(EXC_PAGE_FAULT)                                    )
L(    pushq TASK_OFFSETOF_IC(%rdx)                               )
L(    movq  %rsp, TASK_OFFSETOF_IC(%rdx)                         )
L(                                                               )
L(    /* Fill memory (s.a.: `struct intchain') */                )
L(    rep   stosb                                                )
L(                                                               )
L(    /* Cleanup the custom interrupt handler */                 )
L(    popq  TASK_OFFSETOF_IC(%rdx)                               )
L(    addq  $16, %rsp                                            )
L(                                                               )
L(1:  movq  %rcx, %rax /* Return the amount of missing bytes */  )
L(    ret                                                        )
L(SYM_END(memset_user)                                           )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(strend_user)                                      )
L(    xorq  %rax, %rax /* Scan for ZERO-bytes & error-return */  )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %rdx                  )
L(    movq  TASK_OFFSETOF_ADDRLIMIT(%rdx), %rcx                  )
L(    cmpq  %rcx, %rdi                                           )
L(    jae   2f /* if (str >= addr_limit) goto end; */            )
L(    subq  %rdi, %rcx                                           )
L(3:  leaq  1f(%rip), %r10; pushq %r10                           )
L(    pushq $(EXC_PAGE_FAULT)                                    )
L(    pushq TASK_OFFSETOF_IC(%rdx)                               )
L(    movq  %rsp, TASK_OFFSETOF_IC(%rdx)                         )
L(    repne scasb                                                )
L(    popq  TASK_OFFSETOF_IC(%rdx)                               )
L(    addq  $16, %rsp                                            )
L(    leaq -1(%rdi), %rax /* EAX = EDI-1; */                     )
L(1:  ret                                                        )
L(2:  leaq  __kernel_user_start(%rip), %r10                      )
L(    cmpq  %r10, %rdi                                           )
L(    jb    1b /* if (str < __kernel_user_start) return NULL; */ )
L(    leaq  __kernel_user_end(%rip), %r10                        )
L(    cmpq  %r10, %rdi                                           )
L(    jae   1b /* if (str >= __kernel_user_end) return NULL; */  )
L(    movq  %r10, %rcx                                           )
L(    subq  %rdi, %rcx                                           )
L(    jmp   3b                                                   )
L(SYM_END(strend_user)                                           )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(stpncpy_from_user)                                )
L(    movq  %rdx, %rcx /* max_chars */                           )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %rdx                  )
L(    movq  TASK_OFFSETOF_ADDRLIMIT(%rdx), %rax                  )
L(    cmpq  %rax, %rsi                                           )
L(    jae   3f /* if (str >= addr_limit) return NULL; */         )
L(    subq  %rsi, %rax                                           )
L(4:  cmpq  %rcx, %rax                                           )
L(    cmovb %rax, %rcx /* if (EAX < ECX) ECX = EAX */            )
L(                                                               )
L(    leaq  1f(%rip), %r10; pushq %r10                           )
L(    pushq $(EXC_PAGE_FAULT)                                    )
L(    pushq TASK_OFFSETOF_IC(%rdx)                               )
L(    movq  %rsp, TASK_OFFSETOF_IC(%rdx)                         )
L(                                                               )
L(    /* Copy up to ECX bytes from ESI to EDI, stopping at NUL */)
L(7:  lodsb /* AL = *ESI++; */                                   )
L(    stosb /* *EDI++ = AL; */                                   )
L(    testb %al, %al                                             )
L(    jnz   7b /* if (AL != '\0') continue; */                   )
L(                                                               )
L(    leaq  -1(%rdi), %rax /* Return a pointer to the new NUL-byte. */)
L(    popq  TASK_OFFSETOF_IC(%rdx)                               )
L(    addq  $16, %rsp                                            )
L(                                                               )
L(    ret                                                        )
L(1:  xorq  %rax, %rax /* return NULL; */                        )
L(    ret                                                        )
L(    /* Check for `str' apart of user-share */                  )
L(3:  leaq  __kernel_user_start(%rip), %r10                      )
L(    cmpq  %r10, %rsi                                           )
L(    jb    1b /* if (str < __kernel_user_start) return NULL; */ )
L(    leaq  __kernel_user_end(%rip), %r10                        )
L(    cmpq  %r10, %rsi                                           )
L(    jae   1b /* if (str >= __kernel_user_end) return NULL; */  )
L(    movq  %r10, %rax                                           )
L(    subq  %rsi, %rax                                           )
L(    jmp   4b                                                   )
L(SYM_END(stpncpy_from_user)                                     )
L(.previous                                                      )
);


//FUNDEF SAFE ssize_t (ATTR_CDECL call_user_worker)(void *__restrict worker, size_t argc, ...);
GLOBAL_ASM(
L(.section .rodata                                               )
L(PRIVATE_OBJECT(transmute)                                      )
L(    .quad transmute0                                           )
L(    .quad transmute1                                           )
L(    .quad transmute2                                           )
L(    .quad transmute3                                           )
L(    .quad transmute4                                           )
L(    .quad transmute5                                           )
L(    .quad transmute6                                           )
#define TRANSMUTE_COUNT  7
L(SYM_END(transmute)                                             )
L(.previous                                                      )
L(PRIVATE_ENTRY(transmute6)                                      )
L(    movq  %rdx,     %rdi                                       )
L(    movq  %rcx,     %rsi                                       )
L(    movq  %r8,      %rdx                                       )
L(    movq  %r9,      %rcx                                       )
L(    movq  32(%rsp), %r8                                        )
L(    movq  40(%rsp), %r9                                        )
L(    jmp   99f                                                  )
L(SYM_END(transmute6)                                            )
L(PRIVATE_ENTRY(transmute5)                                      )
L(    movq  %rdx,     %rdi                                       )
L(    movq  %rcx,     %rsi                                       )
L(    movq  %r8,      %rdx                                       )
L(    movq  %r9,      %rcx                                       )
L(    movq  32(%rsp), %r8                                        )
L(    jmp   99f                                                  )
L(SYM_END(transmute5)                                            )
L(PRIVATE_ENTRY(transmute4)                                      )
L(    movq  %rdx, %rdi                                           )
L(    movq  %rcx, %rsi                                           )
L(    movq  %r8,  %rdx                                           )
L(    movq  %r9,  %rcx                                           )
L(    jmp   99f                                                  )
L(SYM_END(transmute4)                                            )
L(PRIVATE_ENTRY(transmute3)                                      )
L(    movq  %rdx, %rdi                                           )
L(    movq  %rcx, %rsi                                           )
L(    movq  %r8,  %rdx                                           )
L(    jmp   99f                                                  )
L(SYM_END(transmute3)                                            )
L(PRIVATE_ENTRY(transmute2)                                      )
L(    movq  %rcx, %rsi                                           )
L(PRIVATE_ENTRY(transmute1)                                      )
L(    movq  %rdx, %rdi                                           )
L(PRIVATE_ENTRY(transmute0)                                      )
L(    jmp   99f                                                  )
L(SYM_END(transmute0)                                            )
L(SYM_END(transmute1)                                            )
L(SYM_END(transmute2)                                            )
L(.section .text                                                 )
L(PUBLIC_ENTRY(call_user_worker)                                 )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %rax                  )
L(    leaq  1f(%rip), %r10; pushq %r10                           )
L(    pushq $(EXC_PAGE_FAULT)                                    )
L(    pushq TASK_OFFSETOF_IC(%rax)                               )
L(    movq  %rsp, TASK_OFFSETOF_IC(%rax)                         )
L(                                                               )
L(    movq  %rdi, %rax                                           )
L(    /* Transform registers (Stack arguments are currently stored at RSP+32). */)
L(    /* ARGC: %rsi; ARGV: %rdx, %rcx, %r8,  %r9,  RSP+32... */    )
L(    /*                   %rdi, %rsi, %rdx, %rcx, %r8, %r9, RSP+32... */)
L(    /*                   1     2     3     4     5    6    7... */)
L(    cmpq  $(TRANSMUTE_COUNT), %rsi                             )
L(    jae   98f                                                  )
L(    leaq  transmute(%rip), %r10                                )
L(    jmpq *(%r10,%rsi,8) /* Transmute registers. */             )
L(                                                               )
L(99: call *%rax /* worker(...) */                               )
L(                                                               )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %r11                  )
L(    popq  TASK_OFFSETOF_IC(%r11)                               )
L(    addq  $16, %rsp                                            )
L(                                                               )
L(2:  ret                                                        )
L(1:  movq $-EFAULT, %rax                                        )
L(    jmp 2b                                                     )
L(98:                                                            )
L(    /* TODO: Transmute stack-based argument vector. */         )
L(                                                               )
L(    call *%rax /* worker(...) */                               )
L(                                                               )
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %r11                  )
L(    popq  TASK_OFFSETOF_IC(%r11)                               )
L(    addq  $16, %rsp                                            )
L(                                                               )
L(    ret                                                        )
L(SYM_END(call_user_worker)                                      )
L(.previous                                                      )
);


/* IOS stands for InOutString... */
#define DEFINE_IOS(name,io_ins,ifi,ifo,ifb,ifl,n) \
GLOBAL_ASM( \
L(.section .text                                                 ) \
L(PUBLIC_ENTRY(name)                                             ) \
L(    movq  %rdx, %rcx /* count */                               ) \
L(    movq  %rdi, %rdx /* port */                                ) \
ifb(L(movq  %rsi, %rdi /* addr */                                )) \
ifb(L(addq  %rcx, %rdi                                           )) \
ifb(L(jo    1f /* if (DOES_OVERFLOW(addr+count)) return count; */)) \
ifl(L(leaq  0(%rsi,%rcx,n), %rdi                                 )) \
ifl(L(cmpq  %rdx, %rsi                                           )) \
ifl(L(jbe   1f /* if (addr+count*n >= addr) return count; */     )) \
ifi(L(movq  %rsi, %rdi /* addr */                                )) \
L(                                                               ) \
L(    /* Push an exception handler. */                           ) \
L(    movq  ASM_CPU(CPU_OFFSETOF_RUNNING), %rax                  ) \
L(    leaq  1f(%rip), %r10; pushq %r10;                          ) \
L(    pushq $(EXC_PAGE_FAULT)                                    ) \
L(    pushq TASK_OFFSETOF_IC(%rax)                               ) \
L(    movq  %rsp, TASK_OFFSETOF_IC(%rax)                         ) \
L(                                                               ) \
L(    /* while (count--) *addr++ = inN(port); */                 ) \
L(    /* while (count--) outN(port,*addr++); */                  ) \
L(    rep io_ins                                                 ) \
L(                                                               ) \
L(    /* Cleanup on success */                                   ) \
L(    popq  TASK_OFFSETOF_IC(%rax)                               ) \
L(    addq  $16, %rsp                                            ) \
L(                                                               ) \
L(1:  movq %rcx, %rax /* Return the number of bytes not transferred */) \
L(    ret                                                        ) \
L(SYM_END(name)                                                  ) \
L(.previous                                                      ) \
)

#define I0(x) /* Nothing */
#define I1(x) x
/* Define user-buffered I/O string functions. */
DEFINE_IOS(insb_user,insb,I1,I0,I1,I0,1);
DEFINE_IOS(insw_user,insw,I1,I0,I0,I1,2);
DEFINE_IOS(insl_user,insl,I1,I0,I0,I1,4);
DEFINE_IOS(outsb_user,outsb,I0,I1,I1,I0,1);
DEFINE_IOS(outsw_user,outsw,I0,I1,I0,I1,2);
DEFINE_IOS(outsl_user,outsl,I0,I1,I0,I1,4);
#undef DEFINE_IOS
#undef I1
#undef I0

DECL_END
#endif /* __x86_64__ */

#endif /* !GUARD_KERNEL_CORE_ARCH_USER64_C_INL */
