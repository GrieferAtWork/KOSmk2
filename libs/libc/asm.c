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
#include <hybrid/compiler.h>
#include <hybrid/host.h>

DECL_BEGIN


#if defined(__i386__) || defined(__x86_64__)
#   include "arch/i386-kos/asm.c.inl"
#elif defined(__arm__)
#   include "arch/arm-kos/asm.c.inl"
#else
#   error "Unsupported architecture"
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

#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(_alloca,libc_alloca);
DEFINE_PUBLIC_ALIAS(_setjmp,libc_setjmp);
DEFINE_PUBLIC_ALIAS(_setjmpex,libc_setjmp); /* TODO: Must safe local exception handlers. */
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


DECL_END

#endif /* !GUARD_LIBS_LIBC_ASM_C */
