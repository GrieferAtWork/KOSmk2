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
#ifndef GUARD_LIBS_LIBC_ASM_H
#define GUARD_LIBS_LIBC_ASM_H 1

#include "libc.h"
#include <alloca.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <setjmp.h>
#include <unistd.h>
#include <bits/sigaction.h>

DECL_BEGIN

INTDEF int LIBCCALL libc_setjmp(jmp_buf buf);
INTDEF int LIBCCALL libc_sigsetjmp(sigjmp_buf buf, int savemask); 
INTDEF void LIBCCALL libc_siglongjmp(sigjmp_buf buf, int sig);
INTDEF ATTR_NORETURN void LIBCCALL libc_longjmp(jmp_buf buf, int sig);
INTDEF ATTR_NORETURN void LIBCCALL libc___longjmp2(jmp_buf buf, int sig);
INTDEF void *LIBCCALL libc_alloca(size_t s);

DECL_END

#endif /* !GUARD_LIBS_LIBC_ASM_H */
