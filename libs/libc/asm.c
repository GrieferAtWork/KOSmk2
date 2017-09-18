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
#ifndef GUARD_LIBS_LIBC_ASM_C
#define GUARD_LIBS_LIBC_ASM_C 1

#include "libc.h"
#include "asm.h"
#include <alloca.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <setjmp.h>
#include <unistd.h>
#include <bits/sigaction.h>

DECL_BEGIN

#ifdef __i386__
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
       * but allows the regular 'jmp_buf' to be used with 'sigsetjmp'). */
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


GLOBAL_ASM(
L(.section .text                                                              )
L(INTERN_ENTRY(libc_alloca)                                                   )
L(    popl     %edx  /* Return address. */                                    )
L(    popl     %eax  /* Allocation count. */                                  )
#ifdef CONFIG_DEBUG
L(    cmpl     $8,  %eax                                                      )
L(    jl       1f                                                             )
L(    pushfl                                                                  )
L(    pushl    %edi                                                           )
L(    leal  -8(%eax), %ecx                                                    )
L(    movl     %esp,  %edi                                                    )
L(    decl     %edi                                                           )
L(    movb     $(CRTDBG_INIT_ALLOCA), %al                                     )
L(    std                                                                     )
L(    rep stosb                                                               )
L(    incl     %edi                                                           )
L(    movl     %edi,  %eax                                                    )
L(    popl	   %edi                                                           )
L(    popfl                                                                   )
L(    movl     $(CRTDBG_INIT_ALLOCA4), -4(%esp)                               )
L(    movl     $(CRTDBG_INIT_ALLOCA4), -8(%esp)                               )
L(    movl     %eax,  %esp                                                    )
L(    jmp      2f                                                             )
L(1:  decl     %esp                                                           )
L(    movb     $(CRTDBG_INIT_ALLOCA), (%esp)                                  )
L(    dec      %eax                                                           )
L(    test	   %eax, %eax                                                     )
L(    jnz      1b                                                             )
L(    movl     %esp, %eax                                                     )
L(2:                                                                          )
#elif 1
L(    subl     %eax,    %esp                                                  )
L(    movl     %esp,    %eax                                                  )
#else
L(    cmpl $(PAGESIZE), %eax                                                  )
L(    jl       2f                                                             )
L(1:  subl $(PAGESIZE), %esp   /* Advance to the next page. */                )
L(    testb    $0,     (%esp)  /* Touch the page. */                          )
L(    subl $(PAGESIZE), %eax   /* Advance the size counter. */                )
L(    jg       1b              /* ZF == 0 && OF == SF */                      )
L(2:  subl     %eax,    %esp                                                  )
L(    movl     %esp,    %eax                                                  )
#endif
L(    jmpl    *%edx                                                           )
L(SYM_END(libc_alloca)                                                        )
L(.previous                                                                   )
);
#else
#error FIXME
#endif

#undef setjmp
#undef sigsetjmp
#undef siglongjmp
#undef __longjmp2
#undef alloca
DEFINE_PUBLIC_ALIAS(setjmp,libc_setjmp);
DEFINE_PUBLIC_ALIAS(sigsetjmp,libc_sigsetjmp);
DEFINE_PUBLIC_ALIAS(siglongjmp,libc_siglongjmp);
DEFINE_PUBLIC_ALIAS(longjmp,libc_longjmp);
DEFINE_PUBLIC_ALIAS(__longjmp2,libc___longjmp2);
DEFINE_PUBLIC_ALIAS(alloca,libc_alloca);

DECL_END

#endif /* !GUARD_LIBS_LIBC_ASM_C */
