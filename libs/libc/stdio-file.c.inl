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
#ifndef GUARD_LIBS_LIBC_STDIO_FILE_C_INL
#define GUARD_LIBS_LIBC_STDIO_FILE_C_INL 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include "libc.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <format-printer.h>
#include <malloc.h>
#include <unistd.h>
#include <assert.h>
#include <hybrid/atomic.h>
#include <fcntl.h>

DECL_BEGIN

struct _IO_FILE {
 int f_fd;
 /* TODO */
};

#undef stdin
#undef stdout
#undef stderr
PRIVATE FILE libc_std_files[3] = {
 [0] = { STDIN_FILENO },
 [1] = { STDOUT_FILENO },
 [2] = { STDERR_FILENO },
};
PUBLIC FILE *stdin  = &libc_std_files[0];
PUBLIC FILE *stdout = &libc_std_files[1];
PUBLIC FILE *stderr = &libc_std_files[2];


PUBLIC size_t (LIBCCALL fread_unlocked)(void *__restrict ptr, size_t size,
                                        size_t n, FILE *__restrict stream) {
 /* TODO: proper implementation! */
 ssize_t result = read(stream->f_fd,ptr,size*n);
 if (result < 0) result = 0;
 return (size_t)result;
}
PUBLIC size_t (LIBCCALL fwrite_unlocked)(void const *__restrict ptr, size_t size,
                                         size_t n, FILE *__restrict stream) {
 /* TODO: proper implementation! */
#if 0
 syslog(LOG_DEBUG,"FWRITE(%$q)\n",size*n,ptr);
 if (size == 1 && n == 1 && ((char *)ptr)[0] == '\n') {
  __asm__("int $3");
 }
#endif
 ssize_t result = write(stream->f_fd,ptr,size*n);
 if (result < 0) result = 0;
 return (size_t)result;
}


PUBLIC ssize_t (LIBCCALL file_printer)(char const *__restrict data,
                                       size_t datalen, void *closure) {
 return fwrite(data,sizeof(char),datalen,(FILE *)closure);
}

PUBLIC ssize_t LIBCCALL
vfprintf(FILE *__restrict stream,
         char const *__restrict format,
         va_list args) {
 return format_vprintf(&file_printer,stream,format,args);
}
PUBLIC ssize_t ATTR_CDECL
printf(char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = vfprintf(stdout,format,args);
 va_end(args);
 return result;
}
PUBLIC ssize_t ATTR_CDECL
fprintf(FILE *__restrict stream,
        char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = vfprintf(stream,format,args);
 va_end(args);
 return result;
}
PUBLIC ssize_t LIBCCALL
vprintf(char const *__restrict format, va_list args) {
 return vfprintf(stdout,format,args);
}

#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define vfscanf_scanner    getc
#define vfscanf_return   ungetc
#else
PRIVATE ssize_t LIBCCALL vfscanf_scanner(FILE *stream) { return getc(stream); }
PRIVATE ssize_t LIBCCALL vfscanf_return(unsigned int c, FILE *stream) { return ungetc(c,stream); }
#endif

PUBLIC ssize_t LIBCCALL
vfscanf(FILE *__restrict stream, char const *__restrict format, va_list args) {
 return format_vscanf((pformatscanner)&getc,
                      (pformatreturn)&ungetc,
                       stream,format,args);
}
PUBLIC ssize_t LIBCCALL
vscanf(char const *__restrict format, va_list args) {
 return vfscanf(stdin,format,args);
}
PUBLIC ssize_t ATTR_CDECL
fscanf(FILE *__restrict stream, char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = vfscanf(stream,format,args);
 va_end(args);
 return result;
}
PUBLIC ssize_t ATTR_CDECL
scanf(char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = vfscanf(stdin,format,args);
 va_end(args);
 return result;
}

PUBLIC int (LIBCCALL fgetc_unlocked)(FILE *stream) {
 unsigned char result;
 return fread_unlocked(&result,sizeof(result),1,stream) == sizeof(result) ? (int)result : EOF;
}
PUBLIC int (LIBCCALL fputc_unlocked)(int c, FILE *stream) {
 unsigned char ch = (unsigned char)c;
 return fwrite_unlocked(&ch,sizeof(ch),1,stream) == sizeof(ch) ? 0 : EOF;
}
PUBLIC int (LIBCCALL getw)(FILE *stream) {
 u16 result;
 return fread(&result,sizeof(result),1,stream) == sizeof(result) ? (int)result : EOF;
}
PUBLIC int (LIBCCALL putw)(int w, FILE *stream) {
 u16 ch = (u16)w;
 return fwrite(&ch,sizeof(ch),1,stream) == sizeof(ch) ? 0 : EOF;
}
PUBLIC int (LIBCCALL fgetc)(FILE *stream) {
 int result;
 flockfile(stream);
 result = fgetc_unlocked(stream);
 funlockfile(stream);
 return result;
}
PUBLIC int (LIBCCALL fputc)(int c, FILE *stream) {
 int result;
 flockfile(stream);
 result = fputc_unlocked(c,stream);
 funlockfile(stream);
 return result;
}

PUBLIC ssize_t (LIBCCALL fputs)(char const *__restrict s, FILE *__restrict stream) {
 ssize_t result;
 flockfile(stream);
 result = fputs_unlocked(s,stream);
 funlockfile(stream);
 return result;
}
PUBLIC ssize_t (LIBCCALL fputs_unlocked)(char const *__restrict s, FILE *__restrict stream) {
 return fwrite_unlocked(s,sizeof(char),strlen(s),stream);
}
PUBLIC void (LIBCCALL clearerr)(FILE *stream) {
 flockfile(stream);
 clearerr_unlocked(stream);
 funlockfile(stream);
}



PUBLIC int (LIBCCALL fclose)(FILE *stream) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL fflush)(FILE *stream) { /*NOT_IMPLEMENTED();*/ return 0; }
PUBLIC void (LIBCCALL setbuf)(FILE *__restrict stream, char *__restrict buf) { NOT_IMPLEMENTED(); }
PUBLIC int (LIBCCALL setvbuf)(FILE *__restrict stream, char *__restrict buf, int modes, size_t n) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL ungetc)(int c, FILE *stream) { NOT_IMPLEMENTED(); return -1; }
PUBLIC FILE *(LIBCCALL tmpfile64)(void) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC FILE *(LIBCCALL tmpfile)(void) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC FILE *(LIBCCALL fopen)(char const *__restrict filename, char const *__restrict modes) {
#if 1
 /* Temporary hack to pipe curses trace logging into the system log. */
 //syslog(LOG_DEBUG,"LIBC: fopen(%q,%q)\n",filename,modes);
 if (!strcmp(filename,"//trace")) {
  FILE *result = omalloc(FILE);
  if (result) result->f_fd = open("/dev/kmsg",O_WRONLY);
  return result;
 }
#endif
 NOT_IMPLEMENTED();
 return NULL;
}
PUBLIC FILE *(LIBCCALL freopen)(char const *__restrict filename, char const *__restrict modes, FILE *__restrict stream) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL fflush_unlocked)(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
PUBLIC void (LIBCCALL setbuffer)(FILE *__restrict stream, char *__restrict buf, size_t size) { NOT_IMPLEMENTED(); }
PUBLIC void (LIBCCALL setlinebuf)(FILE *stream) { NOT_IMPLEMENTED(); }
PUBLIC int (LIBCCALL feof_unlocked)(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL ferror_unlocked)(FILE *stream) { NOT_IMPLEMENTED(); return 0; }
PUBLIC FILE *(LIBCCALL fdopen)(int fd, char const *modes) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC FILE *(LIBCCALL fmemopen)(void *s, size_t len, char const *modes) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC FILE *(LIBCCALL open_memstream)(char **bufloc, size_t *sizeloc) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC ssize_t (LIBCCALL getdelim)(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict stream) { NOT_IMPLEMENTED(); return -1; }
PUBLIC ssize_t (LIBCCALL getline)(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict stream) { NOT_IMPLEMENTED(); return -1; }
PUBLIC FILE *(LIBCCALL popen)(char const *command, char const *modes) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL pclose)(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL fcloseall)(void) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL fseeko64)(FILE *stream, off64_t off, int whence) { NOT_IMPLEMENTED(); return -1; }
PUBLIC off64_t (LIBCCALL ftello64)(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
PUBLIC void (LIBCCALL clearerr_unlocked)(FILE *stream) { NOT_IMPLEMENTED(); }
PUBLIC int (LIBCCALL feof)(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL ferror)(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
//PUBLIC FILE *(LIBCCALL fopencookie)(void *__restrict magic_cookie, char const *__restrict modes, _IO_cookie_io_functions_t io_funcs);
#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
__LIBC __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict s, size_t n, FILE *__restrict stream)
#else
__LIBC __WUNUSED char *(__LIBCCALL libc_fgets_unlocked)(char *__restrict s, int n,
                                               FILE *__restrict stream)
__ASMNAME("fgets_unlocked") { return fgets(s,(size_t)n,stream); }
__LIBC __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict s, size_t n,
                                                   FILE *__restrict stream) __ASMNAME("fgets_unlocked_sz")
#endif
{
 NOT_IMPLEMENTED();
 return NULL;
}
#undef NOT_IMPLEMENTED

PUBLIC int (LIBCCALL fseeko)(FILE *stream, off_t off, int whence) { return fseeko64(stream,(off64_t)off,whence); }
PUBLIC off_t (LIBCCALL ftello)(FILE *stream) { return (off_t)ftello64(stream); }
#if __SIZEOF_LONG__ == __FS_SIZEOF(OFF)
DEFINE_PUBLIC_ALIAS(fseek,fseeko);
DEFINE_PUBLIC_ALIAS(ftell,ftello);
#elif __SIZEOF_LONG__ == __SIZEOF_OFF64_T__
DEFINE_PUBLIC_ALIAS(fseek,fseeko64);
DEFINE_PUBLIC_ALIAS(ftell,ftello64);
#else
PUBLIC int (LIBCCALL fseek)(FILE *stream, long int off, int whence) { return fseeko64(stream,(off64_t)off,whence); }
PUBLIC long int (LIBCCALL ftell)(FILE *stream) { return (long int)ftello64(stream); }
#endif

#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict s, size_t n, FILE *__restrict stream)
#else
__LIBC __WUNUSED char *(__LIBCCALL libc_fgets)(char *__restrict s, int n,
                                               FILE *__restrict stream)
__ASMNAME("fgets") { return fgets(s,(size_t)n,stream); }
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict s, size_t n,
                                          FILE *__restrict stream) __ASMNAME("fgets_sz")
#endif
{
 char *result;
 flockfile(stream);
 result = fgets_unlocked(s,n,stream);
 funlockfile(stream);
 return result;
}

PUBLIC size_t (LIBCCALL fread)(void *__restrict ptr, size_t size,
                               size_t n, FILE *__restrict stream) {
 size_t result;
 flockfile(stream);
 result = fread_unlocked(ptr,size,n,stream);
 funlockfile(stream);
 return result;
}
PUBLIC size_t (LIBCCALL fwrite)(void const *__restrict ptr, size_t size,
                                size_t n, FILE *__restrict stream) {
 size_t result;
 flockfile(stream);
 result = fwrite_unlocked(ptr,size,n,stream);
 funlockfile(stream);
 return result;
}

PUBLIC void (LIBCCALL flockfile)(FILE *stream) { /* TODO */ }
PUBLIC int (LIBCCALL ftrylockfile)(FILE *stream) { /* TODO */ return 1; }
PUBLIC void (LIBCCALL funlockfile)(FILE *stream) { /* TODO */ }
PUBLIC int (LIBCCALL fgetpos)(FILE *__restrict stream, fpos_t *__restrict pos) { return (int)(*pos = (fpos_t)ftello(stream)); }
PUBLIC int (LIBCCALL fsetpos)(FILE *stream, fpos_t const *pos) { return fseeko(stream,*pos,SEEK_SET); }
PUBLIC int (LIBCCALL fgetpos64)(FILE *__restrict stream, fpos64_t *__restrict pos) { return (int)(*pos = (fpos64_t)ftello64(stream)); }
PUBLIC int (LIBCCALL fsetpos64)(FILE *stream, fpos64_t const *pos) { return fseeko64(stream,(off64_t)*pos,SEEK_SET); }
PUBLIC int (LIBCCALL getchar)(void) { return fgetc(stdin); }
PUBLIC int (LIBCCALL putchar)(int c) { return fputc(c,stdout); }
PUBLIC int (LIBCCALL getchar_unlocked)(void) { return fgetc_unlocked(stdin); }
PUBLIC int (LIBCCALL putchar_unlocked)(int c) { return fputc_unlocked(c,stdout); }
PUBLIC void (LIBCCALL rewind)(FILE *stream) { fseeko64(stream,0,SEEK_SET); }
PUBLIC int (LIBCCALL fileno)(FILE *stream) { return ATOMIC_READ(stream->f_fd); }
PUBLIC char *(LIBCCALL gets)(char *s) { return fgets(s,(size_t)-1,stdin); }
PUBLIC ssize_t (LIBCCALL puts)(char const *s) {
 ssize_t result;
 flockfile(stdout);
 result = fputs_unlocked(s,stdout);
 if (result >= 0)
     result += fwrite_unlocked("\n",sizeof(char),1,stdout);
 funlockfile(stdout);
 return result;

}

DEFINE_PUBLIC_ALIAS(fopen64,fopen);
DEFINE_PUBLIC_ALIAS(freopen64,freopen);
DEFINE_PUBLIC_ALIAS(fileno_unlocked,fileno); /* Doesn't really matter. - Use an atomic_read for both! */
DEFINE_PUBLIC_ALIAS(getc,fgetc);
DEFINE_PUBLIC_ALIAS(putc,fputc);
DEFINE_PUBLIC_ALIAS(getc_unlocked,fgetc_unlocked);
DEFINE_PUBLIC_ALIAS(putc_unlocked,fputc_unlocked);
DEFINE_PUBLIC_ALIAS(__getdelim,getdelim);

DECL_END

#endif /* !GUARD_LIBS_LIBC_STDIO_FILE_C_INL */
