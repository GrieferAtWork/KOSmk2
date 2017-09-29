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
#include <stdbool.h>
#include <stdarg.h>

DECL_BEGIN

INTDEF ssize_t LIBCCALL libc_format_vprintf(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_format_printf(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_format_quote(pformatprinter printer, void *closure, char const *__restrict text, size_t textlen, u32 flags);
INTDEF ssize_t LIBCCALL libc_format_hexdump(pformatprinter printer, void *closure, void const *__restrict data, size_t size, size_t linesize, u32 flags);

#ifndef __KERNEL__

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF ssize_t LIBCCALL libc_xformat_vprintf(bool wch16, pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t LIBCCALL libc_dos_format_vprintf(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_format_printf(pformatprinter printer, void *closure, char const *__restrict format, ...);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

/* NOTE: There 4 are implemented in "/libs/libc/unicode.c" */
INTDEF ssize_t LIBCCALL libc_format_16wsztomb(pformatprinter printer, void *closure, char16_t const *__restrict c16, size_t c16len, mbstate_t *__restrict ps);
INTDEF ssize_t LIBCCALL libc_format_32wsztomb(pformatprinter printer, void *closure, char32_t const *__restrict c32, size_t c32len, mbstate_t *__restrict ps);
INTDEF ssize_t LIBCCALL libc_format_16wsntomb(pformatprinter printer, void *closure, char16_t const *__restrict c16, size_t c16max, mbstate_t *__restrict ps);
INTDEF ssize_t LIBCCALL libc_format_32wsntomb(pformatprinter printer, void *closure, char32_t const *__restrict c32, size_t c32max, mbstate_t *__restrict ps);

INTDEF ssize_t ATTR_CDECL libc_format_scanf(pformatscanner scanner, pformatreturn returnch, void *closure, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_format_vscanf(pformatscanner scanner, pformatreturn returnch, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t LIBCCALL libc_format_strftime(pformatprinter printer, void *closure, char const *__restrict format, struct tm const *tm);
INTDEF int LIBCCALL libc_stringprinter_init(struct stringprinter *__restrict self, size_t hint);
INTDEF char *LIBCCALL libc_stringprinter_pack(struct stringprinter *__restrict self, size_t *length);
INTDEF void LIBCCALL libc_stringprinter_fini(struct stringprinter *__restrict self);
INTDEF ssize_t LIBCCALL libc_stringprinter_print(char const *__restrict data, size_t datalen, void *closure);

INTDEF void LIBCCALL libc_buffer_init(struct buffer *__restrict self, pformatprinter printer, void *closure);
INTDEF ssize_t LIBCCALL libc_buffer_fini(struct buffer *__restrict buf);
INTDEF ssize_t LIBCCALL libc_buffer_flush(struct buffer *__restrict buf);
INTDEF ssize_t LIBCCALL libc_buffer_print(char const *__restrict data, size_t datalen, void *closure);

INTDEF ssize_t ATTR_CDECL libc_format_bprintf(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_format_vbprintf(pformatprinter printer, void *closure, char const *__restrict format, va_list args);


INTDEF void LIBCCALL libc_16wprinter_fini(struct c16printer *__restrict wp);
INTDEF void LIBCCALL libc_32wprinter_fini(struct c32printer *__restrict wp);
INTDEF void LIBCCALL libc_16wprinter_init(struct c16printer *__restrict wp, pc16formatprinter printer, void *closure);
INTDEF void LIBCCALL libc_32wprinter_init(struct c32printer *__restrict wp, pc32formatprinter printer, void *closure);
INTDEF ssize_t LIBCCALL libc_16wprinter_print(char const *__restrict data, size_t datalen, void *closure);
INTDEF ssize_t LIBCCALL libc_32wprinter_print(char const *__restrict data, size_t datalen, void *closure);

#else /* !__KERNEL__ */
#define libc_format_bprintf(printer,closure,...)          libc_format_printf(printer,closure,...)
#define libc_format_vbprintf(printer,closure,format,args) libc_format_vprintf(printer,closure,format,args)
#endif /* __KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_FORMAT_PRINTER_H */
