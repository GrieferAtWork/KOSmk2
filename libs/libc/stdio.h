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

__NAMESPACE_STD_BEGIN
struct _IO_FILE {
 int f_fd;
 /* TODO */
};
__NAMESPACE_STD_END

INTDEF size_t LIBCCALL libc_fread_unlocked(void *__restrict ptr, size_t size, size_t n, FILE *__restrict stream);
INTDEF size_t LIBCCALL libc_fwrite_unlocked(void const *__restrict ptr, size_t size, size_t n, FILE *__restrict stream);
INTDEF ssize_t LIBCCALL libc_file_printer(char const *__restrict data, size_t datalen, void *closure);
INTDEF ssize_t LIBCCALL libc_vfprintf(FILE *__restrict stream, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_printf(char const *__restrict format, ...);
INTDEF ssize_t ATTR_CDECL libc_fprintf(FILE *__restrict stream, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_vprintf(char const *__restrict format, va_list args);
INTDEF ssize_t LIBCCALL libc_vfscanf(FILE *__restrict stream, char const *__restrict format, va_list args);
INTDEF ssize_t LIBCCALL libc_vscanf(char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_fscanf(FILE *__restrict stream, char const *__restrict format, ...);
INTDEF ssize_t ATTR_CDECL libc_scanf(char const *__restrict format, ...);
INTDEF int LIBCCALL libc_fgetc_unlocked(FILE *stream);
INTDEF int LIBCCALL libc_fputc_unlocked(int c, FILE *stream);
INTDEF int LIBCCALL libc_getw(FILE *stream);
INTDEF int LIBCCALL libc_putw(int w, FILE *stream);
INTDEF int LIBCCALL libc_fgetc(FILE *stream);
INTDEF int LIBCCALL libc_fputc(int c, FILE *stream);
INTDEF ssize_t LIBCCALL libc_fputs(char const *__restrict s, FILE *__restrict stream);
INTDEF ssize_t LIBCCALL libc_fputs_unlocked(char const *__restrict s, FILE *__restrict stream);
INTDEF void LIBCCALL libc_clearerr(FILE *stream);
INTDEF int LIBCCALL libc_fclose(FILE *stream);
INTDEF int LIBCCALL libc_fflush(FILE *stream);
INTDEF void LIBCCALL libc_setbuf(FILE *__restrict stream, char *__restrict buf);
INTDEF int LIBCCALL libc_setvbuf(FILE *__restrict stream, char *__restrict buf, int modes, size_t n);
INTDEF int LIBCCALL libc_ungetc(int c, FILE *stream);
INTDEF FILE *LIBCCALL libc_tmpfile64(void);
INTDEF FILE *LIBCCALL libc_tmpfile(void);
INTDEF FILE *LIBCCALL libc_fopen(char const *__restrict filename, char const *__restrict modes);
INTDEF FILE *LIBCCALL libc_freopen(char const *__restrict filename, char const *__restrict modes, FILE *__restrict stream);
INTDEF int LIBCCALL libc_fflush_unlocked(FILE *stream);
INTDEF void LIBCCALL libc_setbuffer(FILE *__restrict stream, char *__restrict buf, size_t size);
INTDEF void LIBCCALL libc_setlinebuf(FILE *stream);
INTDEF int LIBCCALL libc_feof_unlocked(FILE *stream);
INTDEF int LIBCCALL libc_ferror_unlocked(FILE *stream);
INTDEF FILE *LIBCCALL libc_fdopen(int fd, char const *modes);
INTDEF FILE *LIBCCALL libc_fmemopen(void *s, size_t len, char const *modes);
INTDEF FILE *LIBCCALL libc_open_memstream(char **bufloc, size_t *sizeloc);
INTDEF ssize_t LIBCCALL libc_getdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict stream);
INTDEF ssize_t LIBCCALL libc_getline(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict stream);
INTDEF FILE *LIBCCALL libc_popen(char const *command, char const *modes);
INTDEF int LIBCCALL libc_pclose(FILE *stream);
INTDEF int LIBCCALL libc_fcloseall(void);
INTDEF int LIBCCALL libc_fseeko64(FILE *stream, off64_t off, int whence);
INTDEF off64_t LIBCCALL libc_ftello64(FILE *stream);
INTDEF void LIBCCALL libc_clearerr_unlocked(FILE *stream);
INTDEF int LIBCCALL libc_feof(FILE *stream);
INTDEF int LIBCCALL libc_ferror(FILE *stream);
//INTDEF FILE *LIBCCALL libc_fopencookie(void *__restrict magic_cookie, char const *__restrict modes, _IO_cookie_io_functions_t io_funcs);
INTDEF char *LIBCCALL libc_fgets(char *__restrict s, size_t n, FILE *__restrict stream);
INTDEF char *LIBCCALL libc_fgets_unlocked(char *__restrict s, size_t n, FILE *__restrict stream);
#if __SIZEOF_SIZE_T__ != __SIZEOF_SIZE_T__
INTDEF char *LIBCCALL libc_fgets_int(char *__restrict s, int n, FILE *__restrict stream);
INTDEF char *LIBCCALL libc_fgets_unlocked_int(char *__restrict s, int n, FILE *__restrict stream);
#endif
INTDEF int LIBCCALL libc_fseeko(FILE *stream, off_t off, int whence);
INTDEF off_t LIBCCALL libc_ftello(FILE *stream);
#if __SIZEOF_LONG__ != __FS_SIZEOF(OFF) && __SIZEOF_LONG__ != __SIZEOF_OFF64_T__
INTDEF int LIBCCALL libc_fseek(FILE *stream, long int off, int whence);
INTDEF long int LIBCCALL libc_ftell(FILE *stream);
#endif
INTDEF size_t LIBCCALL libc_fread(void *__restrict ptr, size_t size, size_t n, FILE *__restrict stream);
INTDEF size_t LIBCCALL libc_fwrite(void const *__restrict ptr, size_t size, size_t n, FILE *__restrict stream);
INTDEF void LIBCCALL libc_flockfile(FILE *stream);
INTDEF int LIBCCALL libc_ftrylockfile(FILE *stream);
INTDEF void LIBCCALL libc_funlockfile(FILE *stream);
INTDEF int LIBCCALL libc_fgetpos(FILE *__restrict stream, fpos_t *__restrict pos);
INTDEF int LIBCCALL libc_fsetpos(FILE *stream, fpos_t const *pos);
INTDEF int LIBCCALL libc_fgetpos64(FILE *__restrict stream, fpos64_t *__restrict pos);
INTDEF int LIBCCALL libc_fsetpos64(FILE *stream, fpos64_t const *pos);
INTDEF int LIBCCALL libc_getchar(void);
INTDEF int LIBCCALL libc_putchar(int c);
INTDEF int LIBCCALL libc_getchar_unlocked(void);
INTDEF int LIBCCALL libc_putchar_unlocked(int c);
INTDEF void LIBCCALL libc_rewind(FILE *stream);
INTDEF int LIBCCALL libc_fileno(FILE *stream);
INTDEF char *LIBCCALL libc_gets(char *s);
INTDEF ssize_t LIBCCALL libc_puts(char const *s);

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */
#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

INTDEF ssize_t LIBCCALL libc_wprintf(wchar_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_swprintf(wchar_t *__restrict s, size_t n, wchar_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_vwprintf(wchar_t const *__restrict format, va_list arg);
INTDEF ssize_t LIBCCALL libc_vswprintf(wchar_t *__restrict s, size_t n, wchar_t const *__restrict format, va_list arg);
INTDEF ssize_t LIBCCALL libc_wscanf(wchar_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_swscanf(wchar_t const *__restrict s, wchar_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_vwscanf(wchar_t const *__restrict format, va_list arg);
INTDEF ssize_t LIBCCALL libc_vswscanf(wchar_t const *__restrict s, wchar_t const *__restrict format, va_list arg);

INTDEF wint_t LIBCCALL libc_fgetwc(FILE *stream);
INTDEF wint_t LIBCCALL libc_fputwc(wchar_t wc, FILE *stream);
INTDEF ssize_t LIBCCALL libc_fputws(wchar_t const *__restrict ws, FILE *__restrict stream);
INTDEF wchar_t *LIBCCALL libc_fgetws(wchar_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF wchar_t *LIBCCALL libc_fgetws_unlocked(wchar_t *__restrict ws, size_t n, FILE *__restrict stream);
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
INTDEF wchar_t *LIBCCALL libc_fgetws_int(wchar_t *__restrict ws, int n, FILE *__restrict stream);
INTDEF wchar_t *LIBCCALL libc_fgetws_unlocked_int(wchar_t *__restrict ws, int n, FILE *__restrict stream);
#endif
INTDEF wint_t LIBCCALL libc_ungetwc(wint_t wc, FILE *stream);
INTDEF int LIBCCALL libc_fwide(FILE *fp, int mode);
INTDEF ssize_t LIBCCALL libc_fwprintf(FILE *__restrict stream, wchar_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_vfwprintf(FILE *__restrict s, wchar_t const *__restrict format, va_list arg);
INTDEF ssize_t LIBCCALL libc_fwscanf(FILE *__restrict stream, wchar_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_vfwscanf(FILE *__restrict s, wchar_t const *__restrict format, va_list arg);
INTDEF FILE *LIBCCALL libc_open_wmemstream(wchar_t **bufloc, size_t *sizeloc);
INTDEF wint_t LIBCCALL libc_fgetwc_unlocked(FILE *stream);
INTDEF wint_t LIBCCALL libc_fputwc_unlocked(wchar_t wc, FILE *stream);
INTDEF int LIBCCALL libc_fputws_unlocked(wchar_t const *__restrict ws, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_getwchar(void);
INTDEF wint_t LIBCCALL libc_putwchar(wchar_t wc);
INTDEF wint_t LIBCCALL libc_getwchar_unlocked(void);
INTDEF wint_t LIBCCALL libc_putwchar_unlocked(wchar_t wc);
#endif /* !__KERNEL__ */


#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF FILE *LIBCCALL libc___iob_func(void);

INTDEF wint_t LIBCCALL libc_single_ungetwch(wint_t __wc);
INTDEF wint_t LIBCCALL libc_single_ungetwch_nolock(wint_t __wc);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


DECL_END


#endif /* !GUARD_LIBS_LIBC_STDIO_H */
