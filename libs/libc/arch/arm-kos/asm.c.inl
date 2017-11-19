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
#ifndef GUARD_LIBS_LIBC_ARCH_ARM_KOS_ASM_C
#define GUARD_LIBS_LIBC_ARCH_ARM_KOS_ASM_C 1

#include "../../libc.h"
#include "../../asm.h"
#include <alloca.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/host.h>

DECL_BEGIN

GLOBAL_ASM(
L(.section .text                                                              )
/*INTERN int (LIBCCALL libc_setjmp)(jmp_buf buf); */
L(INTERN_ENTRY(libc_setjmp)                                                   )
L(    /* TODO */                                                              )
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
L(    /* TODO */                                                              )
/*INTERN ATTR_NORETURN void (LIBCCALL libc_longjmp)(jmp_buf buf, int sig); */
L(INTERN_ENTRY(libc_longjmp)                                                  )
L(    /* TODO */                                                              )
/*INTERN ATTR_NORETURN void (LIBCCALL libc___longjmp2)(jmp_buf buf, int sig); */
L(INTERN_ENTRY(libc___longjmp2)                                               )
L(    /* TODO */                                                              )
L(SYM_END(libc___longjmp2)                                                    )
L(SYM_END(libc_longjmp)                                                       )
L(SYM_END(libc_siglongjmp)                                                    )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text                                                              )
L(INTERN_ENTRY(libc_alloca)                                                   )
L(    /* TODO */                                                              )
L(SYM_END(libc_alloca)                                                        )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text                                                              )
L(INTERN_ENTRY(libc_syscall)                                                  )
L(    /* TODO */                                                              )
L(SYM_END(libc_syscall)                                                       )
L(.previous                                                                   )
);

DECL_END

#endif /* !GUARD_LIBS_LIBC_ARCH_ARM_KOS_ASM_C */
