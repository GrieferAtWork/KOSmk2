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

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */


#ifndef __KERNEL__
struct obstack;
INTDEF char *LIBCCALL libc_tmpnam(char *__restrict buf);
INTDEF char *LIBCCALL libc_tmpnam_r(char *buf);
INTDEF ATTR_MALLOC char *LIBCCALL libc_tempnam(char const *dir, char const *pfx);
INTDEF int LIBCCALL libc_obstack_vprintf(struct obstack *__restrict obstack, char const *__restrict format, va_list args);
INTDEF int LIBCCALL libc_obstack_printf(struct obstack *__restrict obstack, char const *__restrict format, ...);
INTDEF void LIBCCALL libc_perror(char const *__restrict message);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF char *LIBCCALL libc_dos_tmpnam(char *__restrict buf);
INTDEF char *LIBCCALL libc_dos_tmpnam_r(char *buf);
INTERN ATTR_MALLOC char *LIBCCALL libc_dos_tempnam(char const *dir, char const *pfx);
INTDEF char *LIBCCALL libc_tmpnam_s(char *buf, size_t bufsize);
INTDEF char *LIBCCALL libc_dos_tmpnam_s(char *buf, size_t bufsize);

INTDEF char16_t *LIBCCALL libc_16wtmpnam(char16_t *__restrict buf);
INTDEF char32_t *LIBCCALL libc_32wtmpnam(char32_t *__restrict buf);
INTDEF char16_t *LIBCCALL libc_dos_16wtmpnam(char16_t *__restrict buf);
INTDEF char32_t *LIBCCALL libc_dos_32wtmpnam(char32_t *__restrict buf);

INTDEF char16_t *LIBCCALL libc_16wtmpnam_s(char16_t *__restrict buf, size_t buflen);
INTDEF char32_t *LIBCCALL libc_32wtmpnam_s(char32_t *__restrict buf, size_t buflen);
INTDEF char16_t *LIBCCALL libc_dos_16wtmpnam_s(char16_t *__restrict buf, size_t buflen);
INTDEF char32_t *LIBCCALL libc_dos_32wtmpnam_s(char32_t *__restrict buf, size_t buflen);

INTERN ATTR_MALLOC char16_t *LIBCCALL libc_16wtempnam(char16_t const *dir, char16_t const *pfx);
INTERN ATTR_MALLOC char32_t *LIBCCALL libc_32wtempnam(char32_t const *dir, char32_t const *pfx);
INTERN ATTR_MALLOC char16_t *LIBCCALL libc_dos_16wtempnam(char16_t const *dir, char16_t const *pfx);
INTERN ATTR_MALLOC char32_t *LIBCCALL libc_dos_32wtempnam(char32_t const *dir, char32_t const *pfx);

INTERN void LIBCCALL libc_16wperror(char16_t const *__restrict errmsg);
INTERN void LIBCCALL libc_32wperror(char32_t const *__restrict errmsg);
#endif /* CONFIG_LIBC_NO_DOS_LIBC */

#endif /* !__KERNEL__ */


DECL_END


#endif /* !GUARD_LIBS_LIBC_STDIO_H */
