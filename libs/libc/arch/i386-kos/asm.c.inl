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
#ifndef GUARD_LIBS_LIBC_ARCH_I386_KOS_ASM_C
#define GUARD_LIBS_LIBC_ARCH_I386_KOS_ASM_C 1

#include "../../libc.h"
#include "../../asm.h"
#include <alloca.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <hybrid/limits.h>
#include <setjmp.h>
#include <unistd.h>
#include <bits/sigaction.h>
#include <asm/instx.h>

DECL_BEGIN

#ifdef __x86_64__
#define SAVEMASK_OFFSET 64

GLOBAL_ASM(
L(.section .text                                                              )
/*INTERN int (LIBCCALL libc_setjmp)(jmp_buf buf); */
L(INTERN_ENTRY(libc_setjmp)                                                   )
L(    movq   %rbx,  0(%rdi)                                                   )
L(    movq   %rbp,  8(%rdi)                                                   )
L(    movq   %r12, 16(%rdi)                                                   )
L(    movq   %r13, 24(%rdi)                                                   )
L(    movq   %r14, 32(%rdi)                                                   )
L(    movq   %r15, 40(%rdi)                                                   )
L(    leaq  8(%rsp),  %rax  /* RSP */                                         )
L(    movq   %rsp, 48(%rdi)                                                   )
L(    movq  0(%rsp),  %rax  /* RIP */                                         )
L(    movq   %rcx, 56(%rdi)                                                   )
L(    xorq   %rax,    %rax /* Return ZERO(0) the first time around. */        )
L(    ret                                                                     )
L(SYM_END(libc_setjmp)                                                        )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text                                                              )
/*INTERN int (LIBCCALL libc_sigsetjmp)(sigjmp_buf buf, int savemask); */
L(INTERN_ENTRY(libc_sigsetjmp)                                                )
L(    /* TODO */                                                              )
L(SYM_END(libc_sigsetjmp)                                                     )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text                                                              )
/*INTERN void (LIBCCALL libc_siglongjmp)(sigjmp_buf buf, int sig); */
L(INTERN_ENTRY(libc_siglongjmp)                                               )
L(    movq    SAVEMASK_OFFSET(%rdi), %rax                                     )
L(    testq   %rax, %rax                                                      )
L(    jz      1f                                                              )
L(    /* TODO: Restore signal mask */                                         )
/*INTERN ATTR_NORETURN void (LIBCCALL libc_longjmp)(jmp_buf buf, int sig); */
L(INTERN_ENTRY(libc_longjmp)                                                  )
L(1:  testq   %rsi, %rsi                                                      )
L(    jnz     1f                                                              )
L(    incq    %rsi /* Return 1 instead! */                                    )
/*INTERN ATTR_NORETURN void (LIBCCALL libc___longjmp2)(jmp_buf buf, int sig); */
L(INTERN_ENTRY(libc___longjmp2)                                               )
L(1:  movq    %rsi,  %rax                                                     )
L(    movq  0(%rdi), %rbx                                                     )
L(    movq  8(%rdi), %rbp                                                     )
L(    movq 16(%rdi), %r12                                                     )
L(    movq 24(%rdi), %r13                                                     )
L(    movq 32(%rdi), %r14                                                     )
L(    movq 40(%rdi), %r15                                                     )
L(    movq 48(%rdi), %rsp                                                     )
L(    jmpq *56(%rdi)                                                          )
L(SYM_END(libc___longjmp2)                                                    )
L(SYM_END(libc_longjmp)                                                       )
L(SYM_END(libc_siglongjmp)                                                    )
L(.previous                                                                   )
);

#else
#define SAVEMASK_OFFSET 28

GLOBAL_ASM(
L(.section .text                                                              )
/*INTERN int (LIBCCALL libc_setjmp)(jmp_buf buf); */
L(INTERN_ENTRY(libc_setjmp)                                                   )
L(    popl   %ecx                                                             )
L(    movl 0(%esp),                %eax                                       )
L(    movl   %ebx,               0(%eax)                                      )
L(    movl   %esp,               4(%eax)                                      )
L(    movl   %ebp,               8(%eax)                                      )
L(    movl   %esi,              12(%eax)                                      )
L(    movl   %edi,              16(%eax)                                      )
L(    movl   %ecx,              20(%eax) /* EIP */                            )
#if 1 /* Indicate that no signal mask is available (Not really required
       * but allows the regular 'jmp_buf' to be used with `sigsetjmp'). */
L(    movl   $0,   SAVEMASK_OFFSET(%eax)                                      )
#endif
L(    xorl   %eax,                 %eax /* Return ZERO(0) the first time around. */)
L(    jmpl  *%ecx                                                             )
L(SYM_END(libc_setjmp)                                                        )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text                                                              )
/*INTERN int (LIBCCALL libc_sigsetjmp)(sigjmp_buf buf, int savemask); */
L(INTERN_ENTRY(libc_sigsetjmp)                                                )
L(    popl   %ecx                                                             )
L(    movl 0(%esp),                  %eax                                     )
L(    movl 4(%esp),                  %edx                                     )
L(    movl   %ebx,                 0(%eax)                                    )
L(    movl   %esp,                 4(%eax)                                    )
L(    movl   %ebp,                 8(%eax)                                    )
L(    movl   %esi,                12(%eax)                                    )
L(    movl   %edi,                16(%eax)                                    )
L(    movl   %ecx,                20(%eax) /* EIP */                          )
L(    movl   %edx,   SAVEMASK_OFFSET(%eax) /* SAFEMASK */                     )
L(    pushl  %ebx  /* Save protected registers. */                            )
L(    pushl  %esi                                                             )
L(    pushl  %ecx  /* ECX contains the return address (preserve it) */        )
L(    testl  %edx,                   %edx                                     )
L(    jz     1f /* Skip the signal mask if if shouldn't be saved */           )
L(    movl   %eax,                   %edx /* EDX = __buf; */                  )
L(    movl   $(__NR_sigprocmask),    %eax                                     )
L(    movl   $(SIG_SETMASK),         %ebx /* how = SIG_SETMASK; */            )
L(    movl   $0,                     %ecx /* set = NULL; */                   )
L(    addl   $32,                    %edx /* oldset = &__buf->__sig; */       )
L(    movl   $(__SIZEOF_SIGSET_T__), %esi /* sigsetsize = sizeof(sigset_t); */)
L(    int    $0x80                        /* Restore signal mask */           )
L(    xorl   %eax,    %eax  /* Return ZERO(0) the first time around. */       )
L(    popl   %ecx  /* Restore return address */                               )
L(    popl   %esi                                                             )
L(    popl   %ebx                                                             )
L(1:  jmpl  *%ecx                                                             )
L(SYM_END(libc_sigsetjmp)                                                     )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text                                                              )
/*INTERN void (LIBCCALL libc_siglongjmp)(sigjmp_buf buf, int sig); */
L(INTERN_ENTRY(libc_siglongjmp)                                               )
L(    movl   4(%esp), %eax                                                    )
L(    testl  $-1,    SAVEMASK_OFFSET(%eax)                                    )
L(    jz     1f /* If the signal mask wasn't saved, don't restore it. */      )
L(    movl   $(__NR_sigprocmask),    %eax                                     )
L(    movl   $(SIG_SETMASK),         %ebx /* how = SIG_SETMASK; */            )
L(    movl   4(%esp),                %ecx                                     )
L(    addl   $32,                    %ecx /* set = &buf->__sig; */            )
L(    movl   $0,                     %edx /* oldset = NULL; */                )
L(    movl   $(__SIZEOF_SIGSET_T__), %esi /* sigsetsize = sizeof(sigset_t); */)
L(    int    $0x80                        /* Restore signal mask */           )
/*INTERN ATTR_NORETURN void (LIBCCALL libc_longjmp)(jmp_buf buf, int sig); */
L(INTERN_ENTRY(libc_longjmp)                                                  )
L(1:  movl   8(%esp), %eax                                                    )
L(    movl   4(%esp), %esp                                                    )
L(    testl  %eax, %eax                                                       )
L(    jnz    1f                                                               )
L(    incl   %eax /* Return 1 instead! */                                     )
L(    jmp    1f                                                               )
/*INTERN ATTR_NORETURN void (LIBCCALL libc___longjmp2)(jmp_buf buf, int sig); */
L(INTERN_ENTRY(libc___longjmp2)                                               )
L(    movl   8(%esp), %eax                                                    )
L(    movl   4(%esp), %esp                                                    )
/* XXX: This isn't signal-safe! */
L(1:  popl   %ebx                                                             )
L(    popl   %edx /* ESP */                                                   )
L(    popl   %ebp                                                             )
L(    popl   %esi                                                             )
L(    popl   %edi                                                             )
L(    popl   %ecx /* EIP */                                                   )
L(    movl   %edx, %esp                                                       )
L(    jmpl  *%ecx                                                             )
L(SYM_END(libc___longjmp2)                                                    )
L(SYM_END(libc_longjmp)                                                       )
L(SYM_END(libc_siglongjmp)                                                    )
L(.previous                                                                   )
);
#endif

GLOBAL_ASM(
L(.section .text                                                              )
L(INTERN_ENTRY(libc_alloca)                                                   )
L(    popx  %xdx        /* Return address. */                                 )
#ifdef __x86_64__
#define COUNT  %rdi
#else
#define COUNT  %ecx
L(    popl  %ecx        /* Allocation count. */                               )
#endif
#if 1
L(    cmpx  $(PAGESIZE), COUNT                                                )
L(    jbe   70f                                                               )
L(    /* Align ESP by `PAGESIZE' */                                           )
L(    movx  %xsp, %xax                                                        )
L(    andx  $(~(PAGESIZE-1)), %xsp                                            )
L(    andx  $( (PAGESIZE-1)), %xax                                            )
L(    subx  %xax, COUNT                                                       )
L(1:  cmpx  $(PAGESIZE), COUNT                                                )
L(    jbe   70f                                                               )
L(    subx  $(PAGESIZE), %xsp                                                 )
L(    subx  $(PAGESIZE), COUNT                                                )
L(    testb $0,  (%xsp) /* Probe the next page. */                            )
L(    jmp   1b                                                                )
L(70: subx  COUNT, %xsp                                                       )
L(    testb $0,  (%xsp) /* Probe the last page. */                            )
#else
L(    subx  COUNT, %xsp                                                       )
#endif
L(    movx  %xsp, %xax                                                        )
L(    jmpx *%xdx                                                              )
L(SYM_END(libc_alloca)                                                        )
L(.previous                                                                   )
);


GLOBAL_ASM(
L(.section .text                                                              )
L(INTERN_ENTRY(libc_syscall)                                                  )
#ifdef __x86_64__
/* Must transform registers as follows:
 * %rdi, %rsi, %rdx, %rcx, %r8,  %r9, 8(%rsp)
 *   v     v      v    v     v    v    v
 * %rax, %rdi, %rsi, %rdx, %r10, %r8, %r9
 * NOTE: No registers must be saved, because none that are
 *       callee-save ones are used for system calls arguments. */
L(    movq  %rdi,    %rax /* SYSNO */                                         )
L(    movq  %rsi,    %rdi /* ARG #1 */                                        )
L(    movq  %rdx,    %rsi /* ARG #2 */                                        )
L(    movq  %rcx,    %rdx /* ARG #3 */                                        )
L(    movq  %r8,     %r10 /* ARG #4 */                                        )
L(    movq  %r9,     %r8  /* ARG #5 */                                        )
L(    movq  8(%rsp), %r9  /* ARG #6 */                                        )
L(    int   $0x80         /* Invoke the system call. */                       )
L(    ret                                                                     )
#else
L(    /* Save callee-save registers */                                        )
L(    pushl %ebx                                                              )
L(    pushl %edi                                                              )
L(    pushl %esi                                                              )
L(    pushl %ebp                                                              )
L(                                                                            )
L(    /* Load arguments */                                                    )
#define    A  20 /* return+4*register (4+4*4) */
L(    movl A+ 0(%esp), %eax /* sysno */                                       )
L(    movl A+ 4(%esp), %ebx /* Arg #1 */                                      )
L(    movl A+ 8(%esp), %ecx /* Arg #2 */                                      )
L(    movl A+12(%esp), %edx /* Arg #3 */                                      )
L(    movl A+16(%esp), %esi /* Arg #4 */                                      )
L(    movl A+20(%esp), %edi /* Arg #5 */                                      )
L(    movl A+24(%esp), %ebp /* Arg #6 */                                      )
#undef A
L(                                                                            )
L(    int $0x80 /* Invoke the system call interrupt vector */                 )
L(                                                                            )
L(    /* Restore callee-save registers */                                     )
L(    popl %ebp                                                               )
L(    popl %esi                                                               )
L(    popl %edi                                                               )
L(    popl %ebx                                                               )
L(    ret                                                                     )
#endif
L(SYM_END(libc_syscall)                                                       )
L(.previous                                                                   )
);


DECL_END

#endif /* !GUARD_LIBS_LIBC_ARCH_I386_KOS_ASM_C */
