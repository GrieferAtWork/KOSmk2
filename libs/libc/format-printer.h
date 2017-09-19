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
#ifndef GUARD_LIBS_LIBC_FORMAT_PRINTER_H
#define GUARD_LIBS_LIBC_FORMAT_PRINTER_H 1

#include "libc.h"
#include <format-printer.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdarg.h>

DECL_BEGIN

INTDEF ssize_t LIBCCALL libc_format_vprintf(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_format_printf(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_format_quote(pformatprinter printer, void *closure, char const *__restrict text, size_t textlen, u32 flags);
INTDEF ssize_t LIBCCALL libc_format_hexdump(pformatprinter printer, void *closure, void const *__restrict data, size_t size, size_t linesize, u32 flags);

#ifndef __KERNEL__
INTDEF ssize_t ATTR_CDECL libc_format_scanf(pformatscanner scanner, pformatreturn returnch, void *closure, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_format_vscanf(pformatscanner scanner, pformatreturn returnch, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t LIBCCALL libc_format_strftime(pformatprinter printer, void *closure, char const *__restrict format, struct tm const *tm);
INTDEF int LIBCCALL libc_stringprinter_init(struct stringprinter *__restrict self, size_t hint);
INTDEF char *LIBCCALL libc_stringprinter_pack(struct stringprinter *__restrict self, size_t *length);
INTDEF void LIBCCALL libc_stringprinter_fini(struct stringprinter *__restrict self);
INTDEF ssize_t LIBCCALL libc_stringprinter_print(char const *__restrict data, size_t datalen, void *closure);
#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_FORMAT_PRINTER_H */
