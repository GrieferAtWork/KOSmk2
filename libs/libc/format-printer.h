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
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include <xlocale.h>
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_BEGIN

struct tm;

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
INTDEF ssize_t LIBCCALL libc_format_16wsztomb(pformatprinter printer, void *closure, char16_t const *__restrict c16, size_t c16len, mbstate_t *__restrict ps, u32 mode);
INTDEF ssize_t LIBCCALL libc_format_32wsztomb(pformatprinter printer, void *closure, char32_t const *__restrict c32, size_t c32len, mbstate_t *__restrict ps, u32 mode);
INTDEF ssize_t LIBCCALL libc_format_16wsntomb(pformatprinter printer, void *closure, char16_t const *__restrict c16, size_t c16max, mbstate_t *__restrict ps, u32 mode);
INTDEF ssize_t LIBCCALL libc_format_32wsntomb(pformatprinter printer, void *closure, char32_t const *__restrict c32, size_t c32max, mbstate_t *__restrict ps, u32 mode);

INTDEF ssize_t ATTR_CDECL libc_format_scanf(pformatgetc scanner, pformatungetc returnch, void *closure, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_format_vscanf(pformatgetc scanner, pformatungetc returnch, void *closure, char const *__restrict format, va_list args);
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

struct c16printer;
struct c32printer;

INTDEF void LIBCCALL libc_16wprinter_fini(struct c16printer *__restrict wp);
INTDEF void LIBCCALL libc_32wprinter_fini(struct c32printer *__restrict wp);
INTDEF void LIBCCALL libc_16wprinter_init(struct c16printer *__restrict wp, pc16formatprinter printer, void *closure);
INTDEF void LIBCCALL libc_32wprinter_init(struct c32printer *__restrict wp, pc32formatprinter printer, void *closure);
INTDEF ssize_t LIBCCALL libc_16wprinter_print(char const *__restrict data, size_t datalen, void *closure);
INTDEF ssize_t LIBCCALL libc_32wprinter_print(char const *__restrict data, size_t datalen, void *closure);
#endif /* !__KERNEL__ */

/* Formatted print to buffer. */
INTDEF size_t LIBCCALL libc_vsprintf(char *__restrict buf, char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_sprintf(char *__restrict buf, char const *__restrict format, ...);
INTDEF size_t LIBCCALL libc_vsnprintf(char *__restrict buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_snprintf(char *__restrict buf, size_t buflen, char const *__restrict format, ...);
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF size_t LIBCCALL libc_dos_vsprintf(char *__restrict buf, char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_sprintf(char *__restrict buf, char const *__restrict format, ...);
INTDEF size_t LIBCCALL libc_dos_vsnprintf(char *__restrict buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_snprintf(char *__restrict buf, size_t buflen, char const *__restrict format, ...);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

#ifndef __KERNEL__
/* Formatted print to a file descriptor (NOTE: These functions use an internal buffer). */
INTDEF ssize_t LIBCCALL libc_vdprintf(int fd, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dprintf(int fd, char const *__restrict format, ...);

/* Formatted string scanning. */
INTDEF size_t LIBCCALL libc_vsscanf(char const *__restrict src, char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_sscanf(char const *__restrict src, char const *__restrict format, ...);

/* Formatted printing to heap. */
INTDEF ssize_t LIBCCALL libc_vasprintf(char **__restrict ptr, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_asprintf(char **__restrict ptr, char const *__restrict format, ...);

/* Formatted printing to UTF-32 buffer. */
INTDEF ssize_t LIBCCALL libc_32vswprintf(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_32swprintf(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_32vswscanf(char32_t const *__restrict src, char32_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_32swscanf(char32_t const *__restrict src, char32_t const *__restrict format, ...);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
/* Formatted printing to UTF-16 buffer. */
INTDEF ssize_t LIBCCALL libc_dos_16vswprintf(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_16swprintf(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_dos_16vswprintf_unlimited(char16_t *__restrict buf, char16_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_16swprintf_unlimited(char16_t *__restrict buf, char16_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_dos_16vswprintf_l_unlimited(char16_t *__restrict buf, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_16swprintf_l_unlimited(char16_t *__restrict buf, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t LIBCCALL libc_16vswscanf(char16_t const *__restrict src, char16_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_16swscanf(char16_t const *__restrict src, char16_t const *__restrict format, ...);



INTDEF size_t ATTR_CDECL libc_sscanf_l(char const *__restrict src, char const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libc_sscanf_s_l(char const *__restrict src, char const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libc_snscanf(char const *__restrict src, size_t srclen, char const *__restrict format, ...);
INTDEF size_t ATTR_CDECL libc_snscanf_s(char const *__restrict src, size_t srclen, char const *__restrict format, ...);
INTDEF size_t ATTR_CDECL libc_snscanf_l(char const *__restrict src, size_t srclen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libc_snscanf_s_l(char const *__restrict src, size_t srclen, char const *__restrict format, locale_t locale, ...);

INTDEF size_t LIBCCALL   libc_dos_vsprintf_l(char *__restrict buf, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_sprintf_l(char *__restrict buf, char const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL   libc_dos_vsprintf_p(char *__restrict buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_sprintf_p(char *__restrict buf, size_t buflen, char const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_dos_vsprintf_p_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale,  va_list args);
INTDEF size_t ATTR_CDECL libc_dos_sprintf_p_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL   libc_dos_vsprintf_s_l(char *__restrict buf, size_t bufsize, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_sprintf_s_l(char *__restrict buf, size_t bufsize, char const *__restrict format, locale_t locale, ...);

INTDEF size_t LIBCCALL   libc_dos_vscprintf(char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_scprintf(char const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_dos_vscprintf_p(char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_scprintf_p(char const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_dos_vscprintf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_scprintf_l(char const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL   libc_dos_vscprintf_p_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_scprintf_p_l(char const *__restrict format, locale_t locale, ...);

/* The following 4 return an error, rather than the required size when the buffer is too small */
INTDEF ssize_t ATTR_CDECL libc_dos_snprintf_broken(char *__restrict buf, size_t buflen, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL   libc_dos_vsnprintf_broken(char *__restrict buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_snprintf_l_broken(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t LIBCCALL   libc_dos_vsnprintf_l_broken(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL  libc_dos_snprintf_c(char *__restrict buf, size_t buflen, char const *__restrict format, ...);
INTDEF size_t LIBCCALL    libc_dos_vsnprintf_c(char *__restrict buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL  libc_dos_snprintf_c_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL    libc_dos_vsnprintf_c_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_snprintf_s_broken(char *__restrict buf, size_t bufsize, size_t buflen, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL   libc_dos_vsnprintf_s_broken(char *__restrict buf, size_t bufsize, size_t buflen, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_snprintf_s_l_broken(char *__restrict buf, size_t bufsize, size_t buflen, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t LIBCCALL   libc_dos_vsnprintf_s_l_broken(char *__restrict buf, size_t bufsize, size_t buflen, char const *__restrict format, locale_t locale, va_list args);

INTDEF size_t ATTR_CDECL libc_dos_sprintf_s(char *__restrict buf, size_t bufsize, char const *__restrict format, ...) __ASMNAME("snprintf");
INTDEF size_t LIBCCALL   libc_dos_vsprintf_s(char *__restrict buf, size_t bufsize, char const *__restrict format, va_list args);
INTDEF size_t LIBCCALL   libc_dos_vsnprintf_s(char *__restrict buf, size_t bufsize, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_sscanf_s(char const *__restrict src, char const *__restrict format, ...) __ASMNAME("sscanf");
INTDEF size_t LIBCCALL   libc_vsscanf_s(char const *__restrict src, char const *__restrict format, va_list args) __ASMNAME("vsscanf");

/* The following 2 return an error, rather than the required size when the buffer is too small */
INTDEF ssize_t ATTR_CDECL libc_dos_16snwprintf_broken(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL   libc_dos_16vsnwprintf_broken(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, va_list args);

INTDEF size_t ATTR_CDECL  libc_dos_16scwprintf(char16_t const *__restrict format, ...);
INTDEF size_t LIBCCALL    libc_dos_16vscwprintf(char16_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL  libc_dos_16scwprintf_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL    libc_dos_16vscwprintf_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL  libc_dos_16swprintf_c(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF size_t LIBCCALL    libc_dos_16vswprintf_c(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL  libc_dos_16swprintf_c_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL    libc_dos_16vswprintf_c_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_16snwprintf_l_broken(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t LIBCCALL   libc_dos_16vsnwprintf_l_broken(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_16snwprintf_s_broken(char16_t *__restrict buf, size_t buflen, size_t maxlen, char16_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL   libc_dos_16vsnwprintf_s_broken(char16_t *__restrict buf, size_t buflen, size_t maxlen, char16_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_16snwprintf_s_l_broken(char16_t *__restrict buf, size_t buflen, size_t maxlen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t LIBCCALL   libc_dos_16vsnwprintf_s_l_broken(char16_t *__restrict buf, size_t buflen, size_t maxlen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL  libc_32scwprintf(char32_t const *__restrict format, ...);
INTDEF size_t LIBCCALL    libc_32vscwprintf(char32_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL  libc_32scwprintf_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL    libc_32vscwprintf_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL  libc_32swprintf_c(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, ...);
INTDEF size_t LIBCCALL    libc_32vswprintf_c(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL  libc_32swprintf_c_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL    libc_32vswprintf_c_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, va_list args);

INTDEF size_t ATTR_CDECL libc_dos_16scwprintf_p(char16_t const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_dos_16vscwprintf_p(char16_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_16scwprintf_p_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL   libc_dos_16vscwprintf_p_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_16swprintf_p(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_dos_16vswprintf_p(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_16swprintf_p_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL   libc_dos_16vswprintf_p_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL libc_dos_16swprintf_s_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL   libc_dos_16vswprintf_s_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL libc_32scwprintf_p(char32_t const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_32vscwprintf_p(char32_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_32scwprintf_p_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL   libc_32vscwprintf_p_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL libc_32swprintf_p(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_32vswprintf_p(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_32swprintf_p_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL   libc_32vswprintf_p_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t ATTR_CDECL libc_32swprintf_s_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL   libc_32vswprintf_s_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, va_list args);

INTDEF size_t ATTR_CDECL libc_16swscanf_l(char16_t const *src, char16_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_16swscanf_s_l(char16_t const *src, char16_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_16snwscanf(char16_t const *src, size_t srclen, char16_t const *__restrict format, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_16snwscanf_s(char16_t const *src, size_t srclen, char16_t const *__restrict format, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_16snwscanf_l(char16_t const *src, size_t srclen, char16_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_16snwscanf_s_l(char16_t const *src, size_t srclen, char16_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_32swscanf_l(char32_t const *src, char32_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_32swscanf_s_l(char32_t const *src, char32_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_32snwscanf(char32_t const *src, size_t srclen, char32_t const *__restrict format, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_32snwscanf_s(char32_t const *src, size_t srclen, char32_t const *__restrict format, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_32snwscanf_l(char32_t const *src, size_t srclen, char32_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF size_t ATTR_CDECL libc_32snwscanf_s_l(char32_t const *src, size_t srclen, char32_t const *__restrict format, locale_t locale, ...); /* No varargs version. */

INTDEF size_t ATTR_CDECL libc_dos_16swprintf_s(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_dos_16vswprintf_s(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_16swscanf_s(char16_t const *__restrict src, char16_t const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_16vswscanf_s(char16_t const *__restrict src, char16_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_32swprintf_s(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_32vswprintf_s(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_32swscanf_s(char32_t const *__restrict src, char32_t const *__restrict format, ...);
INTDEF size_t LIBCCALL   libc_32vswscanf_s(char32_t const *__restrict src, char32_t const *__restrict format, va_list args);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


#else /* !__KERNEL__ */
#define libc_format_bprintf(printer,closure,...)          libc_format_printf(printer,closure,...)
#define libc_format_vbprintf(printer,closure,format,args) libc_format_vprintf(printer,closure,format,args)
#endif /* __KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_FORMAT_PRINTER_H */
