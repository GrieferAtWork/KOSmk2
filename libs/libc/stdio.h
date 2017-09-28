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

INTDEF size_t LIBCCALL libc_vsprintf(char *__restrict s, char const *__restrict format, va_list args);
INTDEF size_t LIBCCALL libc_vsnprintf(char *__restrict s, size_t maxlen, char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_sprintf(char *__restrict s, char const *__restrict format, ...);
INTDEF size_t ATTR_CDECL libc_snprintf(char *__restrict s, size_t maxlen, char const *__restrict format, ...);

#ifndef __KERNEL__
struct obstack;
INTDEF size_t LIBCCALL libc_vsscanf(char const *__restrict s, char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_sscanf(char const *__restrict s, char const *__restrict format, ...);
INTDEF char *LIBCCALL libc_tmpnam(char *s);
INTDEF char *LIBCCALL libc_tmpnam_r(char *s);
INTDEF char *LIBCCALL libc_tempnam(char const *dir, char const *pfx);
INTDEF int LIBCCALL libc_obstack_vprintf(struct obstack *__restrict obstack, char const *__restrict format, va_list args);
INTDEF int LIBCCALL libc_obstack_printf(struct obstack *__restrict obstack, char const *__restrict format, ...);
INTDEF void LIBCCALL libc_perror(char const *s);
INTDEF ssize_t LIBCCALL libc_vdprintf(int fd, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dprintf(int fd, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_vasprintf(char **__restrict ptr, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_asprintf(char **__restrict ptr, char const *__restrict format, ...);

INTDEF ssize_t LIBCCALL libc_32swprintf(char32_t *__restrict s, size_t n, char32_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_32vswprintf(char32_t *__restrict s, size_t n, char32_t const *__restrict format, va_list arg);
INTDEF ssize_t LIBCCALL libc_32swscanf(char32_t const *__restrict s, char32_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_32vswscanf(char32_t const *__restrict s, char32_t const *__restrict format, va_list arg);
#endif /* !__KERNEL__ */




DECL_END


#endif /* !GUARD_LIBS_LIBC_STDIO_H */
