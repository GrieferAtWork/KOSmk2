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
#ifndef GUARD_LIBS_START_START_C
#define GUARD_LIBS_START_START_C 1

#include <hybrid/compiler.h>
#include <stdlib.h>
#include <hybrid/asm.h>
#include <kos/environ.h>

DECL_BEGIN

typedef int (*pmain)(int argc, char **argv, char **envp);

__ATTR_VISIBILITY("hidden") ATTR_USED
extern int main(int argc, char **argv, char **envp);
__LIBC ATTR_NORETURN void FCALL __entry(struct envdata *__restrict env, pmain m);

#ifdef __i386__
GLOBAL_ASM(
L(.section .data.free        ) /* Initialization (part of .free) */
L(INTERN_ENTRY(_start)       )
/* NOTE: The kernel will have initialized 'ECX' to point at 'appenv' */
/* We let libc deal with all startup. (Keeps executables small) */
L(    movl $main, %edx       )
L(.global __entry            )
L(    jmp __entry            )
L(SYM_END(_start)            )
L(.previous                  )
);
#else
__ATTR_VISIBILITY("hidden") ATTR_SECTION(".data.free")
void FCALL _start(struct envdata *__restrict env) {
 __entry(env,&main);
}
#endif


DECL_END

#endif /* !GUARD_LIBS_START_START_C */
