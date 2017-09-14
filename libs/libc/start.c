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
#ifndef GUARD_LIBS_LIBC_START_C
#define GUARD_LIBS_LIBC_START_C 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <unistd.h>
#include <stdlib.h>
#include <kos/environ.h>
#include <hybrid/asm.h>

DECL_BEGIN

typedef int (*pmain)(int argc, char **argv, char **envp);
__LIBC ATTR_NORETURN void (FCALL __entry)(struct envdata *__restrict env, pmain main);

#undef environ
DEFINE_PUBLIC_ALIAS(__environ,environ);

INTDEF int LIBCCALL user_initialize_dlmalloc(void);

PUBLIC char **environ = NULL;
PUBLIC struct envdata *appenv;
PUBLIC ATTR_NORETURN
void (FCALL __entry)(struct envdata *__restrict env, pmain main) {
 appenv  = env;
 environ = env->e_envp;
 user_initialize_dlmalloc();
 exit((*main)(env->e_argc,env->e_argv,environ));
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_START_C */
