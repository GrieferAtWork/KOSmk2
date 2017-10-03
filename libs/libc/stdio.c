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
#ifndef GUARD_LIBS_LIBC_STDIO_C
#define GUARD_LIBS_LIBC_STDIO_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include "libc.h"
#include "errno.h"
#include "format-printer.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"
#ifndef __KERNEL__
#include "file.h"
#include "unistd.h"
#endif /* !__KERNEL__ */

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdarg.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <assert.h>

DECL_BEGIN

#ifndef __KERNEL__
INTERN char *LIBCCALL libc_tmpnam(char *s) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_tmpnam_r(char *s) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_tempnam(char const *dir, char const *pfx) { NOT_IMPLEMENTED(); return NULL; }

struct obstack;
INTERN int LIBCCALL libc_obstack_vprintf(struct obstack *__restrict obstack, char const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_obstack_printf(struct obstack *__restrict obstack,
                                        char const *__restrict format, ...) {
 int result; va_list args;
 va_start(args,format);
 result = libc_obstack_vprintf(obstack,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_COLDTEXT void LIBCCALL libc_perror(char const *s) {
 libc_fprintf(stderr,COLDSTR("%s: %[errno]\n"),s,GET_ERRNO());
}

DEFINE_PUBLIC_ALIAS(tmpnam,libc_tmpnam);
DEFINE_PUBLIC_ALIAS(tmpnam_r,libc_tmpnam_r);
DEFINE_PUBLIC_ALIAS(tempnam,libc_tempnam);
DEFINE_PUBLIC_ALIAS(obstack_vprintf,libc_obstack_vprintf);
DEFINE_PUBLIC_ALIAS(obstack_printf,libc_obstack_printf);
DEFINE_PUBLIC_ALIAS(perror,libc_perror);
#endif /* !__KERNEL__ */


DECL_END

#endif /* !GUARD_LIBS_LIBC_STDIO_C */
