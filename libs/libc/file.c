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
#ifndef GUARD_LIBS_LIBC_STDIO_FILE_C
#define GUARD_LIBS_LIBC_STDIO_FILE_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include "libc.h"
#include "file.h"
#include "format-printer.h"
#include "string.h"
#include "fcntl.h"
#include "malloc.h"
#include "unistd.h"
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/atomic.h>

DECL_BEGIN

#undef stdin
#undef stdout
#undef stderr

INTERN FILE libc_std_files[3] = {
 [0] = { STDIN_FILENO },
 [1] = { STDOUT_FILENO },
 [2] = { STDERR_FILENO },
};
PUBLIC FILE *stdin  = &libc_std_files[0];
PUBLIC FILE *stdout = &libc_std_files[1];
PUBLIC FILE *stderr = &libc_std_files[2];

INTERN size_t LIBCCALL
libc_fread_unlocked(void *__restrict ptr, size_t size,
                    size_t n, FILE *__restrict stream) {
 /* TODO: proper implementation! */
 ssize_t result = libc_read(stream->f_fd,ptr,size*n);
 if (result < 0) result = 0;
 return (size_t)result;
}
INTERN size_t LIBCCALL
libc_fwrite_unlocked(void const *__restrict ptr, size_t size,
                     size_t n, FILE *__restrict stream) {
 /* TODO: proper implementation! */
#if 0
 libc_syslog(LOG_DEBUG,"FWRITE(%$q)\n",size*n,ptr);
 if (size == 1 && n == 1 && ((char *)ptr)[0] == '\n') {
  __asm__("int $3");
 }
#endif
 ssize_t result = libc_write(stream->f_fd,ptr,size*n);
 if (result < 0) result = 0;
 return (size_t)result;
}


INTERN ssize_t LIBCCALL
libc_file_printer(char const *__restrict data,
                  size_t datalen, void *closure) {
 return libc_fwrite(data,sizeof(char),datalen,(FILE *)closure);
}

INTERN ssize_t LIBCCALL
libc_vfprintf(FILE *__restrict stream,
              char const *__restrict format,
              va_list args) {
 return libc_format_vbprintf(&libc_file_printer,stream,format,args);
}
INTERN ssize_t ATTR_CDECL
libc_printf(char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vfprintf(stdout,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t ATTR_CDECL
libc_fprintf(FILE *__restrict stream,
             char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vfprintf(stream,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t LIBCCALL
libc_vprintf(char const *__restrict format, va_list args) {
 return libc_vfprintf(stdout,format,args);
}

#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define vfscanf_scanner   libc_fgetc
#define vfscanf_return   libc_ungetc
#else
PRIVATE ssize_t LIBCCALL vfscanf_scanner(FILE *stream) { return libc_fgetc(stream); }
PRIVATE ssize_t LIBCCALL vfscanf_return(unsigned int c, FILE *stream) { return libc_ungetc(c,stream); }
#endif
INTERN ssize_t LIBCCALL
libc_vfscanf(FILE *__restrict stream, char const *__restrict format, va_list args) {
 return libc_format_vscanf((pformatscanner)&libc_fgetc,
                           (pformatreturn)&libc_ungetc,
                            stream,format,args);
}
INTERN ssize_t LIBCCALL
libc_vscanf(char const *__restrict format, va_list args) {
 return libc_vfscanf(stdin,format,args);
}
INTERN ssize_t ATTR_CDECL
libc_fscanf(FILE *__restrict stream, char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vfscanf(stream,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t ATTR_CDECL
libc_scanf(char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vfscanf(stdin,format,args);
 va_end(args);
 return result;
}
INTERN int LIBCCALL libc_fgetc_unlocked(FILE *stream) {
 unsigned char result;
 return libc_fread_unlocked(&result,sizeof(result),1,stream) ==
        sizeof(result) ? (int)result : EOF;
}
INTERN int LIBCCALL libc_fputc_unlocked(int c, FILE *stream) {
 unsigned char ch = (unsigned char)c;
 return libc_fwrite_unlocked(&ch,sizeof(ch),1,stream) ==
        sizeof(ch) ? 0 : EOF;
}
INTERN int LIBCCALL libc_getw(FILE *stream) {
 u16 result;
 return libc_fread(&result,sizeof(result),1,stream) ==
        sizeof(result) ? (int)result : EOF;
}
INTERN int LIBCCALL libc_putw(int w, FILE *stream) {
 u16 ch = (u16)w;
 return libc_fwrite(&ch,sizeof(ch),1,stream) ==
        sizeof(ch) ? 0 : EOF;
}
INTERN int LIBCCALL libc_fgetc(FILE *stream) {
 int result;
 libc_flockfile(stream);
 result = libc_fgetc_unlocked(stream);
 libc_funlockfile(stream);
 return result;
}
INTERN int LIBCCALL libc_fputc(int c, FILE *stream) {
 int result;
 libc_flockfile(stream);
 result = libc_fputc_unlocked(c,stream);
 libc_funlockfile(stream);
 return result;
}

INTERN ssize_t LIBCCALL libc_fputs(char const *__restrict s, FILE *__restrict stream) {
 ssize_t result;
 libc_flockfile(stream);
 result = libc_fputs_unlocked(s,stream);
 libc_funlockfile(stream);
 return result;
}
INTERN ssize_t LIBCCALL libc_fputs_unlocked(char const *__restrict s, FILE *__restrict stream) {
 return libc_fwrite_unlocked(s,sizeof(char),libc_strlen(s),stream);
}
INTERN void LIBCCALL libc_clearerr(FILE *stream) {
 libc_flockfile(stream);
 libc_clearerr_unlocked(stream);
 libc_funlockfile(stream);
}



INTERN int LIBCCALL libc_fflush(FILE *stream) {
 int result;
 libc_flockfile(stream);
 result = libc_fflush_unlocked(stream);;
 libc_funlockfile(stream);
 return result;
}
INTERN void LIBCCALL
libc_setbuf(FILE *__restrict stream,
            char *__restrict buf) {
 libc_setbuffer(stream,buf,BUFSIZ);
}
INTERN int LIBCCALL
libc_setvbuf(FILE *__restrict stream,
             char *__restrict buf,
             int modes, size_t n) {
 NOT_IMPLEMENTED();
 return -1;
}
INTERN void LIBCCALL
libc_setbuffer(FILE *__restrict stream,
               char *__restrict buf, size_t size) {
 NOT_IMPLEMENTED();
}
INTERN void LIBCCALL libc_setlinebuf(FILE *stream) {
 NOT_IMPLEMENTED();
}
INTERN int LIBCCALL libc_ungetc(int c, FILE *stream) {
 NOT_IMPLEMENTED();
 return -1;
}

INTERN int LIBCCALL libc_fclose(FILE *stream) {
 int result;
 if (!stream) { SET_ERRNO(EINVAL); return -1; }
 result = libc_close(stream->f_fd);
 libc_free(stream);
 return result;
}
PRIVATE int LIBCCALL parse_open_modes(char const *__restrict modes) {
 int mode = O_RDONLY;
 if (modes) for (; *modes; ++modes) {
  if (*modes == 'r') mode = O_RDONLY;
  if (*modes == 'w') mode = O_WRONLY|O_CREAT|O_TRUNC;
  if (*modes == '+') mode &= ~(O_TRUNC|O_ACCMODE),mode |= O_RDWR;
 }
 return mode;
}
INTERN FILE *LIBCCALL libc_tmpfile64(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN FILE *LIBCCALL libc_tmpfile(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN FILE *LIBCCALL libc_fopen(char const *__restrict filename, char const *__restrict modes) {
 int fd; FILE *result;
 libc_syslog(LOG_DEBUG,"LIBC: fopen(%q,%q)\n",filename,modes);
#if 1
 /* Temporary hack to pipe curses trace logging into the system log. */
 if (!libc_strcmp(filename,"//trace")) {
  fd = libc_open("/dev/kmsg",O_WRONLY);
 } else
#endif
 {
  fd = libc_open(filename,parse_open_modes(modes),0644);
 }
 if (fd < 0) return NULL;
 result = (FILE *)libc_malloc(sizeof(FILE));
 if (!result) sys_close(fd);
 else result->f_fd = fd;
 return result;
}
INTERN FILE *LIBCCALL
libc_freopen(char const *__restrict filename,
             char const *__restrict modes,
             FILE *__restrict stream) {
 if (stream) {
  int fd = libc_open(filename,parse_open_modes(modes),0644);
  if (fd < 0) return NULL;
  libc_dup2(fd,stream->f_fd);
  libc_close(fd);
 }
 return stream;
}
INTERN int LIBCCALL libc_fflush_unlocked(FILE *stream) {
 return stream ? libc_fdatasync(stream->f_fd) : -1;
}
INTERN int LIBCCALL libc_feof_unlocked(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_ferror_unlocked(FILE *stream) { NOT_IMPLEMENTED(); return 0; }
INTERN FILE *LIBCCALL libc_fdopen(int fd, char const *modes) {
 FILE *result = (FILE *)libc_malloc(sizeof(FILE));
 if (result) result->f_fd = fd;
 return result;
}
INTERN FILE *LIBCCALL libc_fmemopen(void *s, size_t len, char const *modes) { NOT_IMPLEMENTED(); return NULL; }
INTERN FILE *LIBCCALL libc_open_memstream(char **bufloc, size_t *sizeloc) { NOT_IMPLEMENTED(); return NULL; }
INTERN ssize_t LIBCCALL libc_getdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict stream) { NOT_IMPLEMENTED(); return -1; }
INTERN ssize_t LIBCCALL libc_getline(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict stream) { NOT_IMPLEMENTED(); return -1; }
INTERN FILE *LIBCCALL libc_popen(char const *command, char const *modes) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_pclose(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_fcloseall(void) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_fseeko64(FILE *stream, off64_t off, int whence) { return stream ? libc_lseek64(stream->f_fd,off,whence) >= 0 : -1; }
INTERN off64_t LIBCCALL libc_ftello64(FILE *stream) { return stream ? libc_lseek64(stream->f_fd,0,SEEK_CUR) : -1; }
INTERN void LIBCCALL libc_clearerr_unlocked(FILE *stream) { NOT_IMPLEMENTED(); }
INTERN int LIBCCALL libc_feof(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_ferror(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
//INTERN FILE *LIBCCALL libc_fopencookie(void *__restrict magic_cookie, char const *__restrict modes, _IO_cookie_io_functions_t io_funcs);

#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
INTERN char *LIBCCALL libc_fgets_unlocked(char *__restrict s, size_t n, FILE *__restrict stream)
#else
INTERN char *LIBCCALL
libc_fgets_unlocked_int(char *__restrict s, int n,
                        FILE *__restrict stream) {
 return fgets_unlocked(s,(size_t)n,stream);
}
INTERN char *LIBCCALL
libc_fgets_unlocked(char *__restrict s, size_t n,
                    FILE *__restrict stream)
#endif
{
 NOT_IMPLEMENTED();
 return NULL;
}
INTERN int LIBCCALL libc_fseeko(FILE *stream, off_t off, int whence) { return libc_fseeko64(stream,(off64_t)off,whence); }
INTERN off_t LIBCCALL libc_ftello(FILE *stream) { return (off_t)libc_ftello64(stream); }
#if __SIZEOF_LONG__ != __FS_SIZEOF(OFF) && __SIZEOF_LONG__ != __SIZEOF_OFF64_T__
INTERN int LIBCCALL libc_fseek(FILE *stream, long int off, int whence) { return libc_fseeko64(stream,(off64_t)off,whence); }
INTERN long int LIBCCALL libc_ftell(FILE *stream) { return (long int)libc_ftello64(stream); }
#endif

#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
INTERN char *LIBCCALL libc_fgets(char *__restrict s, size_t n, FILE *__restrict stream)
#else
INTERN char *LIBCCALL
libc_fgets_int(char *__restrict s, int n,
               FILE *__restrict stream) {
 return fgets(s,(size_t)n,stream);
}
INTERN char *LIBCCALL libc_fgets(char *__restrict s, size_t n,
                                 FILE *__restrict stream)
#endif
{
 char *result;
 libc_flockfile(stream);
 result = libc_fgets_unlocked(s,n,stream);
 libc_funlockfile(stream);
 return result;
}

INTERN size_t LIBCCALL
libc_fread(void *__restrict ptr, size_t size,
           size_t n, FILE *__restrict stream) {
 size_t result;
 libc_flockfile(stream);
 result = libc_fread_unlocked(ptr,size,n,stream);
 libc_funlockfile(stream);
 return result;
}
INTERN size_t LIBCCALL
libc_fwrite(void const *__restrict ptr, size_t size,
            size_t n, FILE *__restrict stream) {
 size_t result;
 libc_flockfile(stream);
 result = libc_fwrite_unlocked(ptr,size,n,stream);
 libc_funlockfile(stream);
 return result;
}

INTERN void LIBCCALL libc_flockfile(FILE *stream) { /* TODO */ }
INTERN int LIBCCALL libc_ftrylockfile(FILE *stream) { /* TODO */ return 1; }
INTERN void LIBCCALL libc_funlockfile(FILE *stream) { /* TODO */ }
INTERN int LIBCCALL libc_fgetpos(FILE *__restrict stream, fpos_t *__restrict pos) { return (int)(*pos = (fpos_t)libc_ftello(stream)); }
INTERN int LIBCCALL libc_fsetpos(FILE *stream, fpos_t const *pos) { return libc_fseeko(stream,*pos,SEEK_SET); }
INTERN int LIBCCALL libc_fgetpos64(FILE *__restrict stream, fpos64_t *__restrict pos) { return (int)(*pos = (fpos64_t)libc_ftello64(stream)); }
INTERN int LIBCCALL libc_fsetpos64(FILE *stream, fpos64_t const *pos) { return libc_fseeko64(stream,(off64_t)*pos,SEEK_SET); }
INTERN int LIBCCALL libc_getchar(void) { return libc_fgetc(stdin); }
INTERN int LIBCCALL libc_putchar(int c) { return libc_fputc(c,stdout); }
INTERN int LIBCCALL libc_getchar_unlocked(void) { return libc_fgetc_unlocked(stdin); }
INTERN int LIBCCALL libc_putchar_unlocked(int c) { return libc_fputc_unlocked(c,stdout); }
INTERN void LIBCCALL libc_rewind(FILE *stream) { libc_fseeko64(stream,0,SEEK_SET); }
INTERN int LIBCCALL libc_fileno(FILE *stream) { return ATOMIC_READ(stream->f_fd); }
INTERN char *LIBCCALL libc_gets(char *s) { return libc_fgets(s,(size_t)-1,stdin); }
INTERN ssize_t LIBCCALL libc_puts(char const *s) {
 ssize_t result;
 libc_flockfile(stdout);
 result = libc_fputs_unlocked(s,stdout);
 if (result >= 0)
     result += libc_fwrite_unlocked("\n",sizeof(char),1,stdout);
 libc_funlockfile(stdout);
 return result;
}



DEFINE_PUBLIC_ALIAS(fread_unlocked,libc_fread_unlocked);
DEFINE_PUBLIC_ALIAS(fwrite_unlocked,libc_fwrite_unlocked);
DEFINE_PUBLIC_ALIAS(file_printer,libc_file_printer);
DEFINE_PUBLIC_ALIAS(vfprintf,libc_vfprintf);
DEFINE_PUBLIC_ALIAS(printf,libc_printf);
DEFINE_PUBLIC_ALIAS(fprintf,libc_fprintf);
DEFINE_PUBLIC_ALIAS(vprintf,libc_vprintf);
DEFINE_PUBLIC_ALIAS(vfscanf,libc_vfscanf);
DEFINE_PUBLIC_ALIAS(vscanf,libc_vscanf);
DEFINE_PUBLIC_ALIAS(fscanf,libc_fscanf);
DEFINE_PUBLIC_ALIAS(scanf,libc_scanf);
DEFINE_PUBLIC_ALIAS(fgetc_unlocked,libc_fgetc_unlocked);
DEFINE_PUBLIC_ALIAS(fputc_unlocked,libc_fputc_unlocked);
DEFINE_PUBLIC_ALIAS(getw,libc_getw);
DEFINE_PUBLIC_ALIAS(putw,libc_putw);
DEFINE_PUBLIC_ALIAS(fgetc,libc_fgetc);
DEFINE_PUBLIC_ALIAS(fputc,libc_fputc);
DEFINE_PUBLIC_ALIAS(fputs,libc_fputs);
DEFINE_PUBLIC_ALIAS(fputs_unlocked,libc_fputs_unlocked);
DEFINE_PUBLIC_ALIAS(clearerr,libc_clearerr);
DEFINE_PUBLIC_ALIAS(fclose,libc_fclose);
DEFINE_PUBLIC_ALIAS(fflush,libc_fflush);
DEFINE_PUBLIC_ALIAS(setbuf,libc_setbuf);
DEFINE_PUBLIC_ALIAS(setvbuf,libc_setvbuf);
DEFINE_PUBLIC_ALIAS(ungetc,libc_ungetc);
DEFINE_PUBLIC_ALIAS(tmpfile64,libc_tmpfile64);
DEFINE_PUBLIC_ALIAS(tmpfile,libc_tmpfile);
DEFINE_PUBLIC_ALIAS(fopen,libc_fopen);
DEFINE_PUBLIC_ALIAS(freopen,libc_freopen);
DEFINE_PUBLIC_ALIAS(fflush_unlocked,libc_fflush_unlocked);
DEFINE_PUBLIC_ALIAS(setbuffer,libc_setbuffer);
DEFINE_PUBLIC_ALIAS(setlinebuf,libc_setlinebuf);
DEFINE_PUBLIC_ALIAS(feof_unlocked,libc_feof_unlocked);
DEFINE_PUBLIC_ALIAS(ferror_unlocked,libc_ferror_unlocked);
DEFINE_PUBLIC_ALIAS(fdopen,libc_fdopen);
DEFINE_PUBLIC_ALIAS(fmemopen,libc_fmemopen);
DEFINE_PUBLIC_ALIAS(open_memstream,libc_open_memstream);
DEFINE_PUBLIC_ALIAS(getdelim,libc_getdelim);
DEFINE_PUBLIC_ALIAS(getline,libc_getline);
DEFINE_PUBLIC_ALIAS(popen,libc_popen);
DEFINE_PUBLIC_ALIAS(pclose,libc_pclose);
DEFINE_PUBLIC_ALIAS(fcloseall,libc_fcloseall);
DEFINE_PUBLIC_ALIAS(fseeko64,libc_fseeko64);
DEFINE_PUBLIC_ALIAS(ftello64,libc_ftello64);
DEFINE_PUBLIC_ALIAS(clearerr_unlocked,libc_clearerr_unlocked);
DEFINE_PUBLIC_ALIAS(feof,libc_feof);
DEFINE_PUBLIC_ALIAS(ferror,libc_ferror);
//DEFINE_PUBLIC_ALIAS(fopencookie,libc_fopencookie);
#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
DEFINE_PUBLIC_ALIAS(fgets,libc_fgets);
DEFINE_PUBLIC_ALIAS(fgets_unlocked,libc_fgets_unlocked);
#else
DEFINE_PUBLIC_ALIAS(fgets,libc_fgets);
DEFINE_PUBLIC_ALIAS(fgets_sz,libc_fgets);
DEFINE_PUBLIC_ALIAS(fgets_unlocked,libc_fgets_unlocked_int);
DEFINE_PUBLIC_ALIAS(fgets_unlocked_sz,libc_fgets_unlocked);
#endif
DEFINE_PUBLIC_ALIAS(fseeko,libc_fseeko);
DEFINE_PUBLIC_ALIAS(ftello,libc_ftello);
#if __SIZEOF_LONG__ == __FS_SIZEOF(OFF)
DEFINE_PUBLIC_ALIAS(fseek,libc_fseeko);
DEFINE_PUBLIC_ALIAS(ftell,libc_ftello);
#elif __SIZEOF_LONG__ == __SIZEOF_OFF64_T__
DEFINE_PUBLIC_ALIAS(fseek,libc_fseeko64);
DEFINE_PUBLIC_ALIAS(ftell,libc_ftello64);
#else
DEFINE_PUBLIC_ALIAS(fseek,libc_fseek);
DEFINE_PUBLIC_ALIAS(ftell,libc_ftell);
#endif
DEFINE_PUBLIC_ALIAS(fread,libc_fread);
DEFINE_PUBLIC_ALIAS(fwrite,libc_fwrite);
DEFINE_PUBLIC_ALIAS(flockfile,libc_flockfile);
DEFINE_PUBLIC_ALIAS(ftrylockfile,libc_ftrylockfile);
DEFINE_PUBLIC_ALIAS(funlockfile,libc_funlockfile);
DEFINE_PUBLIC_ALIAS(fgetpos,libc_fgetpos);
DEFINE_PUBLIC_ALIAS(fsetpos,libc_fsetpos);
DEFINE_PUBLIC_ALIAS(fgetpos64,libc_fgetpos64);
DEFINE_PUBLIC_ALIAS(fsetpos64,libc_fsetpos64);
DEFINE_PUBLIC_ALIAS(getchar,libc_getchar);
DEFINE_PUBLIC_ALIAS(putchar,libc_putchar);
DEFINE_PUBLIC_ALIAS(getchar_unlocked,libc_getchar_unlocked);
DEFINE_PUBLIC_ALIAS(putchar_unlocked,libc_putchar_unlocked);
DEFINE_PUBLIC_ALIAS(rewind,libc_rewind);
DEFINE_PUBLIC_ALIAS(fileno,libc_fileno);
DEFINE_PUBLIC_ALIAS(gets,libc_gets);
DEFINE_PUBLIC_ALIAS(puts,libc_puts);
DEFINE_PUBLIC_ALIAS(fopen64,libc_fopen);
DEFINE_PUBLIC_ALIAS(freopen64,libc_freopen);
DEFINE_PUBLIC_ALIAS(fileno_unlocked,libc_fileno); /* Doesn't really matter. - Use an atomic_read for both! */
DEFINE_PUBLIC_ALIAS(getc,libc_fgetc);
DEFINE_PUBLIC_ALIAS(putc,libc_fputc);
DEFINE_PUBLIC_ALIAS(getc_unlocked,libc_fgetc_unlocked);
DEFINE_PUBLIC_ALIAS(putc_unlocked,libc_fputc_unlocked);
DEFINE_PUBLIC_ALIAS(__getdelim,libc_getdelim);


/* Wide-string API */
INTERN wint_t LIBCCALL libc_32fgetwc(FILE *stream) {
 wint_t result;
 libc_flockfile(stream);
 result = libc_32fgetwc_unlocked(stream);
 libc_funlockfile(stream);
 return result;
}
INTERN wint_t LIBCCALL
libc_32fputwc(char32_t wc, FILE *stream) {
 wint_t result;
 libc_flockfile(stream);
 result = libc_32fgetwc_unlocked(stream);
 libc_funlockfile(stream);
 return result;
}
INTERN char32_t *LIBCCALL
libc_32fgetws(char32_t *__restrict ws, size_t n, FILE *__restrict stream) {
 char32_t *result;
 libc_flockfile(stream);
 result = libc_32fgetws_unlocked(ws,n,stream);
 libc_funlockfile(stream);
 return result;
}
INTERN ssize_t LIBCCALL
libc_32fputws(char32_t const *__restrict ws, FILE *__restrict stream) {
 ssize_t result;
 libc_flockfile(stream);
 result = libc_32fputws_unlocked(ws,stream);
 libc_funlockfile(stream);
 return result;
}
INTERN wint_t LIBCCALL libc_32ungetwc(wint_t wc, FILE *stream) { NOT_IMPLEMENTED(); return libc_ungetc((int)wc,stream); }
INTERN int LIBCCALL libc_32fwide(FILE *fp, int mode) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32vfwprintf(FILE *__restrict s, char32_t const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32vfwscanf(FILE *__restrict s, char32_t const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32fwprintf(FILE *__restrict stream, char32_t const *__restrict format, ...) {
 va_list args; int result; va_start(args,format);
 result = libc_32vfwprintf(stream,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t LIBCCALL
libc_32fwscanf(FILE *__restrict stream, char32_t const *__restrict format, ...) {
 va_list args; ssize_t result; va_start(args,format);
 result = libc_32vfwscanf(stream,format,args);
 va_end(args);
 return result;
}
INTERN FILE *LIBCCALL
libc_32open_wmemstream(char32_t **bufloc, size_t *sizeloc) {
 NOT_IMPLEMENTED();
 return NULL;
}
INTERN wint_t LIBCCALL
libc_32fgetwc_unlocked(FILE *stream) {
 NOT_IMPLEMENTED();
 return (wint_t)libc_fgetc_unlocked(stream);
}
INTERN wint_t LIBCCALL
libc_32fputwc_unlocked(char32_t wc, FILE *stream) {
 NOT_IMPLEMENTED();
 return libc_fputc_unlocked((int)wc,stream);
}
INTERN char32_t *LIBCCALL
libc_32fgetws_unlocked(char32_t *__restrict ws, size_t n, FILE *__restrict stream) {
 NOT_IMPLEMENTED();
 if (n) *ws = '\0';
 return ws;
}
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
INTERN char32_t *LIBCCALL libc_32fgetws_int(char32_t *__restrict ws, int n, FILE *__restrict stream) { return libc_32fgetws(ws,(size_t)n,stream); }
INTERN char32_t *LIBCCALL libc_32fgetws_unlocked_int(char32_t *__restrict ws, int n, FILE *__restrict stream) { return libc_32fgetws_unlocked(ws,(size_t)n,stream); }
#endif
INTERN ssize_t LIBCCALL
libc_32fputws_unlocked(char32_t const *__restrict ws, FILE *__restrict stream) {
 ssize_t result = 0;
 NOT_IMPLEMENTED();
 for (; *ws; ++ws,++result) {
  if (libc_fputc_unlocked((int)*ws,stream) == EOF)
      break;
 }
 return result;
}
INTERN wint_t LIBCCALL libc_32getwchar(void) { return libc_32fgetwc(stdin); }
INTERN wint_t LIBCCALL libc_32putwchar(char32_t wc) { return libc_32fputwc(wc,stdout); }
INTERN wint_t LIBCCALL libc_32getwchar_unlocked(void) { return libc_32fgetwc_unlocked(stdin); }
INTERN wint_t LIBCCALL libc_32putwchar_unlocked(char32_t wc) { return libc_32fputwc_unlocked(wc,stdout); }
INTERN ssize_t LIBCCALL libc_32vwprintf(char32_t const *__restrict format, va_list arg) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32vwscanf(char32_t const *__restrict format, va_list arg) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32wprintf(char32_t const *__restrict format, ...) {
 va_list args; ssize_t result; va_start(args,format);
 result = libc_32vwprintf(format,args);
 va_end(args);
 return result;
}
INTERN ssize_t LIBCCALL libc_32wscanf(char32_t const *__restrict format, ...) {
 va_list args; ssize_t result; va_start(args,format);
 result = libc_32vwscanf(format,args);
 va_end(args);
 return result;
}


DEFINE_PUBLIC_ALIAS(fputws,libc_32fputws);
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
DEFINE_PUBLIC_ALIAS(fgetws,libc_32fgetws_int);
DEFINE_PUBLIC_ALIAS(fgetws_unlocked,libc_32fgetws_unlocked_int);
DEFINE_PUBLIC_ALIAS(fgetws_sz,libc_32fgetws);
DEFINE_PUBLIC_ALIAS(fgetws_unlocked_sz,libc_32fgetws_unlocked);
#else
DEFINE_PUBLIC_ALIAS(fgetws,libc_32fgetws);
DEFINE_PUBLIC_ALIAS(fgetws_unlocked,libc_32fgetws_unlocked);
#endif
DEFINE_PUBLIC_ALIAS(ungetwc,libc_32ungetwc);
DEFINE_PUBLIC_ALIAS(fwide,libc_32fwide);
DEFINE_PUBLIC_ALIAS(fwprintf,libc_32fwprintf);
DEFINE_PUBLIC_ALIAS(vfwprintf,libc_32vfwprintf);
DEFINE_PUBLIC_ALIAS(fwscanf,libc_32fwscanf);
DEFINE_PUBLIC_ALIAS(vfwscanf,libc_32vfwscanf);
DEFINE_PUBLIC_ALIAS(open_wmemstream,libc_32open_wmemstream);
DEFINE_PUBLIC_ALIAS(fputws_unlocked,libc_32fputws_unlocked);
DEFINE_PUBLIC_ALIAS(getwc,libc_32fgetwc);
DEFINE_PUBLIC_ALIAS(fgetwc,libc_32fgetwc);
DEFINE_PUBLIC_ALIAS(putwc,libc_32fputwc);
DEFINE_PUBLIC_ALIAS(fputwc,libc_32fputwc);
DEFINE_PUBLIC_ALIAS(getwc_unlocked,libc_32fgetwc_unlocked);
DEFINE_PUBLIC_ALIAS(fgetwc_unlocked,libc_32fgetwc_unlocked);
DEFINE_PUBLIC_ALIAS(putwc_unlocked,libc_32fputwc_unlocked);
DEFINE_PUBLIC_ALIAS(fputwc_unlocked,libc_32fputwc_unlocked);
DEFINE_PUBLIC_ALIAS(getwchar,libc_32getwchar);
DEFINE_PUBLIC_ALIAS(putwchar,libc_32putwchar);
DEFINE_PUBLIC_ALIAS(getwchar_unlocked,libc_32getwchar_unlocked);
DEFINE_PUBLIC_ALIAS(putwchar_unlocked,libc_32putwchar_unlocked);
DEFINE_PUBLIC_ALIAS(wprintf,libc_32wprintf);
DEFINE_PUBLIC_ALIAS(vwprintf,libc_32vwprintf);
DEFINE_PUBLIC_ALIAS(wscanf,libc_32wscanf);
DEFINE_PUBLIC_ALIAS(vwscanf,libc_32vwscanf);


#ifndef CONFIG_LIBC_NO_DOS_LIBC
/* DOS defines this one... */
INTERN FILE *LIBCCALL libc___iob_func(void) { return libc_std_files; }
DEFINE_PUBLIC_ALIAS(__iob_func,libc___iob_func);


INTERN wint_t LIBCCALL libc_16single_ungetwch(wint_t wc) { NOT_IMPLEMENTED(); return wc; }
INTERN wint_t LIBCCALL libc_32single_ungetwch(wint_t wc) { NOT_IMPLEMENTED(); return wc; }
INTERN wint_t LIBCCALL libc_16single_ungetwch_nolock(wint_t wc) { NOT_IMPLEMENTED(); return wc; }
INTERN wint_t LIBCCALL libc_32single_ungetwch_nolock(wint_t wc) { NOT_IMPLEMENTED(); return wc; }
DEFINE_PUBLIC_ALIAS(_ungetwch,libc_32single_ungetwch);
DEFINE_PUBLIC_ALIAS(_ungetwch_nolock,libc_32single_ungetwch_nolock);
DEFINE_PUBLIC_ALIAS(__DSYM(_ungetwch),libc_16single_ungetwch);
DEFINE_PUBLIC_ALIAS(__DSYM(_ungetwch_nolock),libc_16single_ungetwch_nolock);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_STDIO_FILE_C */
