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
#ifndef GUARD_LIBS_LIBC_STDIO_H
#define GUARD_LIBS_LIBC_STDIO_H 1

#include "libc.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdarg.h>
#include <uchar.h>
#include <stdio.h>

DECL_BEGIN

#ifndef __fpos64_t_defined
#define __fpos64_t_defined 1
typedef __pos64_t   fpos64_t;
#endif /* !__fpos64_t_defined */

#ifndef __KERNEL__
struct obstack;
INTDEF char *LIBCCALL libc_tmpnam(char *s);
INTDEF char *LIBCCALL libc_tmpnam_r(char *s);
INTDEF char *LIBCCALL libc_tempnam(char const *dir, char const *pfx);
INTDEF int LIBCCALL libc_obstack_vprintf(struct obstack *__restrict obstack, char const *__restrict format, va_list args);
INTDEF int LIBCCALL libc_obstack_printf(struct obstack *__restrict obstack, char const *__restrict format, ...);
INTDEF void LIBCCALL libc_perror(char const *s);
#endif /* !__KERNEL__ */


DECL_END


#endif /* !GUARD_LIBS_LIBC_STDIO_H */
