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
#include "file.h"
#include "unistd.h"
#include "unicode.h"

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdarg.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <assert.h>

DECL_BEGIN

INTERN char *LIBCCALL libc_tmpnam(char *__restrict buf) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_tmpnam_r(char *buf) { return buf ? libc_tmpnam(buf) : NULL; }
INTERN char *LIBCCALL libc_tempnam(char const *dir, char const *pfx) { NOT_IMPLEMENTED(); return NULL; }

struct obstack;
INTERN int LIBCCALL
libc_obstack_vprintf(struct obstack *__restrict obstack,
                     char const *__restrict format, va_list args) {
 NOT_IMPLEMENTED();
 return 0;
}
INTERN int LIBCCALL
libc_obstack_printf(struct obstack *__restrict obstack,
                    char const *__restrict format, ...) {
 int result; va_list args;
 va_start(args,format);
 result = libc_obstack_vprintf(obstack,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_COLDTEXT void LIBCCALL
libc_perror(char const *__restrict message) {
 libc_fprintf(stderr,COLDSTR("%s: %[errno]\n"),
              message,GET_ERRNO());
}

DEFINE_PUBLIC_ALIAS(tmpnam,libc_tmpnam);
DEFINE_PUBLIC_ALIAS(tmpnam_r,libc_tmpnam_r);
DEFINE_PUBLIC_ALIAS(tempnam,libc_tempnam);
DEFINE_PUBLIC_ALIAS(obstack_vprintf,libc_obstack_vprintf);
DEFINE_PUBLIC_ALIAS(obstack_printf,libc_obstack_printf);
DEFINE_PUBLIC_ALIAS(perror,libc_perror);


#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN ATTR_DOSTEXT char *LIBCCALL libc_dos_tmpnam(char *__restrict buf) { NOT_IMPLEMENTED(); return NULL; }
INTERN ATTR_DOSTEXT char *LIBCCALL libc_dos_tmpnam_r(char *buf) { return buf ? libc_tmpnam(buf) : NULL; }
INTERN ATTR_DOSTEXT char *LIBCCALL libc_dos_tempnam(char const *dir, char const *pfx) { NOT_IMPLEMENTED(); return NULL; }
INTDEF ATTR_DOSTEXT char *LIBCCALL libc_tmpnam_s(char *buf, size_t bufsize) { NOT_IMPLEMENTED(); return NULL; }
INTDEF ATTR_DOSTEXT char *LIBCCALL libc_dos_tmpnam_s(char *buf, size_t bufsize) { NOT_IMPLEMENTED(); return NULL; }

INTERN ATTR_DOSTEXT char16_t *LIBCCALL libc_16wtmpnam(char16_t *__restrict buf) { NOT_IMPLEMENTED(); return NULL; }
INTERN ATTR_DOSTEXT char32_t *LIBCCALL libc_32wtmpnam(char32_t *__restrict buf) { NOT_IMPLEMENTED(); return NULL; }
INTERN ATTR_DOSTEXT char16_t *LIBCCALL libc_dos_16wtmpnam(char16_t *__restrict buf) { NOT_IMPLEMENTED(); return NULL; }
INTERN ATTR_DOSTEXT char32_t *LIBCCALL libc_dos_32wtmpnam(char32_t *__restrict buf) { NOT_IMPLEMENTED(); return NULL; }

INTERN ATTR_DOSTEXT char16_t *LIBCCALL libc_16wtmpnam_s(char16_t *__restrict buf, size_t buflen) { NOT_IMPLEMENTED(); return NULL; }
INTERN ATTR_DOSTEXT char32_t *LIBCCALL libc_32wtmpnam_s(char32_t *__restrict buf, size_t buflen) { NOT_IMPLEMENTED(); return NULL; }
INTERN ATTR_DOSTEXT char16_t *LIBCCALL libc_dos_16wtmpnam_s(char16_t *__restrict buf, size_t buflen) { NOT_IMPLEMENTED(); return NULL; }
INTERN ATTR_DOSTEXT char32_t *LIBCCALL libc_dos_32wtmpnam_s(char32_t *__restrict buf, size_t buflen) { NOT_IMPLEMENTED(); return NULL; }

INTERN ATTR_DOSTEXT char16_t *LIBCCALL libc_16wtempnam(char16_t const *dir, char16_t const *pfx) { NOT_IMPLEMENTED(); return NULL; }
INTERN ATTR_DOSTEXT char32_t *LIBCCALL libc_32wtempnam(char32_t const *dir, char32_t const *pfx) { NOT_IMPLEMENTED(); return NULL; }
INTERN ATTR_DOSTEXT char16_t *LIBCCALL libc_dos_16wtempnam(char16_t const *dir, char16_t const *pfx) { NOT_IMPLEMENTED(); return NULL; }
INTERN ATTR_DOSTEXT char32_t *LIBCCALL libc_dos_32wtempnam(char32_t const *dir, char32_t const *pfx) { NOT_IMPLEMENTED(); return NULL; }

INTERN void LIBCCALL libc_16wperror(char16_t const *__restrict errmsg) {
 errno_t e = GET_ERRNO();
 char *utf8_errmsg = libc_utf16to8m(errmsg);
 SET_ERRNO(e),libc_perror(utf8_errmsg);
 libc_free(utf8_errmsg);
}
INTERN void LIBCCALL libc_32wperror(char32_t const *__restrict errmsg) {
 errno_t e = GET_ERRNO();
 char *utf8_errmsg = libc_utf32to8m(errmsg);
 SET_ERRNO(e),libc_perror(utf8_errmsg);
 libc_free(utf8_errmsg);
}

DEFINE_PUBLIC_ALIAS(__DSYM(tmpnam),libc_dos_tmpnam);
DEFINE_PUBLIC_ALIAS(__DSYM(tmpnam_r),libc_dos_tmpnam_r);
DEFINE_PUBLIC_ALIAS(_tempnam,libc_dos_tempnam);

DEFINE_PUBLIC_ALIAS(__KSYMw16(_wtmpnam),libc_16wtmpnam);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wtmpnam),libc_32wtmpnam);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wtmpnam),libc_dos_16wtmpnam);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wtmpnam),libc_dos_32wtmpnam);

DEFINE_PUBLIC_ALIAS(__KSYMw16(_wtempnam),libc_16wtempnam);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wtempnam),libc_32wtempnam);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wtempnam),libc_dos_16wtempnam);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wtempnam),libc_dos_32wtempnam);

DEFINE_PUBLIC_ALIAS(tmpnam_s,libc_tmpnam_s);
DEFINE_PUBLIC_ALIAS(__DSYM(tmpnam_s),libc_dos_tmpnam_s);

DEFINE_PUBLIC_ALIAS(__KSYMw16(_wtmpnam_s),libc_16wtmpnam_s);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wtmpnam_s),libc_32wtmpnam_s);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wtmpnam_s),libc_dos_16wtmpnam_s);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wtmpnam_s),libc_dos_32wtmpnam_s);

DEFINE_PUBLIC_ALIAS(wperror,libc_32wperror);
DEFINE_PUBLIC_ALIAS(_wperror,libc_16wperror);
#endif /* CONFIG_LIBC_NO_DOS_LIBC */


DECL_END

#endif /* !GUARD_LIBS_LIBC_STDIO_C */
