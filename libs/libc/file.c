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
#include "termios.h"

#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/atomic.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>

DECL_BEGIN

#undef stdin
#undef stdout
#undef stderr

INTERN FILE libc_std_files[3] = {
 [0] = FILE_INIT(STDIN_FILENO,FILE_READOK|FILE_LNBUFIT),
 [1] = FILE_INIT(STDOUT_FILENO,FILE_WRITEOK|FILE_LNBUFIT),
 [2] = FILE_INIT(STDERR_FILENO,FILE_WRITEOK|FILE_LNBUFIT),
};
PUBLIC FILE *stdin  = &libc_std_files[0];
PUBLIC FILE *stdout = &libc_std_files[1];
PUBLIC FILE *stderr = &libc_std_files[2];

INTERN size_t LIBCCALL
libc_fread_impl(void *__restrict ptr, size_t size,
                FILE *__restrict self) {
 return libc_read(self->f_fd,ptr,size);
}
INTERN size_t LIBCCALL
libc_fwrite_impl(void const *__restrict ptr, size_t size,
                 FILE *__restrict self) {
 return libc_write(self->f_fd,ptr,size);
}

INTERN int LIBCCALL
libc_fseek_impl(FILE *__restrict self, __off_t off, int whence) {
#ifdef CONFIG_32BIT_FILESYSTEM
 return libc_lseek(self->f_fd,off,whence);
#else
 return libc_lseek64(self->f_fd,off,whence);
#endif
}
INTERN __off_t LIBCCALL
libc_ftell_impl(FILE *__restrict self) {
 assert(self);
#ifdef CONFIG_32BIT_FILESYSTEM
 return libc_lseek(self->f_fd,0,SEEK_CUR);
#else
 return libc_lseek64(self->f_fd,0,SEEK_CUR);
#endif
}


INTDEF size_t LIBCCALL
libc_fread_unlocked(void *__restrict ptr, size_t size,
                    size_t n, FILE *__restrict self) {
 if (!self) return 0;
 return libc_fread_impl(ptr,size*n,self);;
}
INTDEF size_t LIBCCALL
libc_fwrite_unlocked(void const *__restrict ptr, size_t size,
                     size_t n, FILE *__restrict self) {
 if (!self) return 0;
 return libc_fwrite_impl(ptr,size*n,self);;
}
INTDEF size_t LIBCCALL
libc_fread(void *__restrict ptr, size_t size,
           size_t n, FILE *__restrict self) {
 size_t result;
 if (!self) return 0;
 file_write(self);
 result = libc_fread_impl(ptr,size*n,self);;
 file_endwrite(self);
 return result;
}
INTDEF size_t LIBCCALL
libc_fwrite(void const *__restrict ptr, size_t size,
            size_t n, FILE *__restrict self) {
 size_t result;
 if (!self) return 0;
 file_write(self);
 result = libc_fwrite_impl(ptr,size*n,self);;
 file_endwrite(self);
 return result;
}

INTERN ssize_t LIBCCALL
libc_file_printer(char const *__restrict data,
                  size_t datalen, void *closure) {
 ssize_t result;
 if unlikely(!closure) return 0;
 file_write((FILE *)closure);
 result = (ssize_t)libc_fwrite_impl(data,datalen*sizeof(char),(FILE *)closure);
 if unlikely(!result && FILE_ISERR((FILE *)closure)) result = -1;
 file_endwrite((FILE *)closure);
 return result;
}


#ifdef CONFIG_LIBCCALL_HAS_RETURN_64_IS_32
DEFINE_INTERN_ALIAS(libc_ftello,libc_ftello64);
#else
INTERN off_t LIBCCALL libc_ftello(FILE *self) { return (off_t)libc_ftello64(self); }
#endif
INTERN off64_t LIBCCALL
libc_ftello64(FILE *self) {
 off64_t result;
 if unlikely(!self) return -1;
 file_read(self);
 result = (off64_t)libc_ftell_impl(self);
 file_endread(self);
 return result;
}
INTERN int LIBCCALL
libc_fseeko64(FILE *self, off64_t off, int whence) {
 int result;
 if unlikely(!self) return -1;
 file_write(self);
 result = libc_fseek_impl(self,(__off_t)off,whence);
 file_endwrite(self);
 return result;
}
INTERN int LIBCCALL libc_fseeko(FILE *self, off_t off, int whence) {
 int result;
 if unlikely(!self) return -1;
 file_write(self);
 result = libc_fseek_impl(self,(__off_t)off,whence);
 file_endwrite(self);
 return result;
}

/* Define the C standard seek/tell function pair. */
#if __SIZEOF_LONG__ == __FS_SIZEOF(OFF)
DEFINE_INTERN_ALIAS(libc_fseek,libc_fseeko);
DEFINE_INTERN_ALIAS(libc_ftell,libc_ftello);
#elif __SIZEOF_LONG__ == __SIZEOF_OFF64_T__
DEFINE_INTERN_ALIAS(libc_fseek,libc_fseeko64);
DEFINE_INTERN_ALIAS(libc_ftell,libc_ftello64);
#elif __SIZEOF_LONG__ > __SIZEOF_OFF64_T__
INTERN int LIBCCALL libc_fseek(FILE *self, long int off, int whence) { return libc_fseeko64(self,(off64_t)off,whence); }
INTERN long int LIBCCALL libc_ftell(FILE *self) { return (long int)libc_ftello64(self); }
#else
INTERN int LIBCCALL libc_fseek(FILE *self, long int off, int whence) { return libc_fseeko(self,(off_t)off,whence); }
INTERN long int LIBCCALL libc_ftell(FILE *self) { return (long int)libc_ftello(self); }
#endif




INTERN ssize_t LIBCCALL
libc_vfprintf(FILE *__restrict self,
              char const *__restrict format,
              va_list args) {
 return libc_format_vprintf(&libc_file_printer,self,format,args);
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
libc_fprintf(FILE *__restrict self,
             char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vfprintf(self,format,args);
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
PRIVATE ssize_t LIBCCALL vfscanf_scanner(FILE *self) { return libc_fgetc(self); }
PRIVATE ssize_t LIBCCALL vfscanf_return(unsigned int c, FILE *self) { return libc_ungetc(c,self); }
#endif
INTERN ssize_t LIBCCALL
libc_vfscanf(FILE *__restrict self, char const *__restrict format, va_list args) {
 return libc_format_vscanf((pformatscanner)&libc_fgetc,
                           (pformatreturn)&libc_ungetc,
                            self,format,args);
}
INTERN ssize_t LIBCCALL
libc_vscanf(char const *__restrict format, va_list args) {
 return libc_vfscanf(stdin,format,args);
}
INTERN ssize_t ATTR_CDECL
libc_fscanf(FILE *__restrict self, char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vfscanf(self,format,args);
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
INTERN int LIBCCALL libc_fgetc_unlocked(FILE *self) {
 unsigned char result;
 return libc_fread_unlocked(&result,sizeof(result),1,self) ==
        sizeof(result) ? (int)result : EOF;
}
INTERN int LIBCCALL libc_fputc_unlocked(int c, FILE *self) {
 unsigned char ch = (unsigned char)c;
 return libc_fwrite_unlocked(&ch,sizeof(ch),1,self) ==
        sizeof(ch) ? 0 : EOF;
}
INTERN int LIBCCALL libc_getw(FILE *self) {
 u16 result;
 return libc_fread(&result,sizeof(result),1,self) ==
        sizeof(result) ? (int)result : EOF;
}
INTERN int LIBCCALL libc_putw(int w, FILE *self) {
 u16 ch = (u16)w;
 return libc_fwrite(&ch,sizeof(ch),1,self) ==
        sizeof(ch) ? 0 : EOF;
}
INTERN int LIBCCALL libc_fgetc(FILE *self) {
 int result;
 file_write(self);
 result = libc_fgetc_unlocked(self);
 file_endwrite(self);
 return result;
}
INTERN int LIBCCALL libc_fputc(int c, FILE *self) {
 int result;
 file_write(self);
 result = libc_fputc_unlocked(c,self);
 file_endwrite(self);
 return result;
}

INTERN ssize_t LIBCCALL libc_fputs(char const *__restrict s, FILE *__restrict self) {
 ssize_t result;
 file_write(self);
 result = libc_fputs_unlocked(s,self);
 file_endwrite(self);
 return result;
}
INTERN ssize_t LIBCCALL libc_fputs_unlocked(char const *__restrict s, FILE *__restrict self) {
 return libc_fwrite_unlocked(s,sizeof(char),libc_strlen(s),self);
}

INTERN int LIBCCALL libc_fflush(FILE *self) {
 int result;
 if (self) {
  file_write(self);
  result = libc_fflush_unlocked(self);
  file_endwrite(self);
 }
 return result;
}
INTERN void LIBCCALL
libc_setbuf(FILE *__restrict self,
            char *__restrict buf) {
 libc_setbuffer(self,buf,BUFSIZ);
}
INTERN int LIBCCALL
libc_setvbuf(FILE *__restrict self,
             char *__restrict buf,
             int modes, size_t n) {
 NOT_IMPLEMENTED();
 return -1;
}
INTERN void LIBCCALL
libc_setbuffer(FILE *__restrict self,
               char *__restrict buf, size_t size) {
 NOT_IMPLEMENTED();
}
INTERN void LIBCCALL libc_setlinebuf(FILE *self) {
 NOT_IMPLEMENTED();
}
INTERN int LIBCCALL libc_ungetc(int c, FILE *self) {
 NOT_IMPLEMENTED();
 return -1;
}

INTERN int LIBCCALL libc_fclose(FILE *self) {
 int result;
 if (!self) { SET_ERRNO(EINVAL); return -1; }
 result = libc_close(self->f_fd);
 libc_free(self);
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
             FILE *__restrict self) {
 if (self) {
  int fd = libc_open(filename,parse_open_modes(modes),0644);
  if (fd < 0) return NULL;
  libc_dup2(fd,self->f_fd);
  libc_close(fd);
 }
 return self;
}
INTERN int LIBCCALL libc_fflush_unlocked(FILE *self) {
 return self ? libc_fdatasync(self->f_fd) : -1;
}
INTERN int LIBCCALL libc_feof_unlocked(FILE *self) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_ferror_unlocked(FILE *self) { NOT_IMPLEMENTED(); return 0; }
INTERN FILE *LIBCCALL libc_fdopen(int fd, char const *modes) {
 FILE *result = (FILE *)libc_malloc(sizeof(FILE));
 if (result) result->f_fd = fd;
 return result;
}
INTERN FILE *LIBCCALL libc_fmemopen(void *s, size_t len, char const *modes) { NOT_IMPLEMENTED(); return NULL; }
INTERN FILE *LIBCCALL libc_open_memself(char **bufloc, size_t *sizeloc) { NOT_IMPLEMENTED(); return NULL; }
INTERN ssize_t LIBCCALL libc_getdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict self) { NOT_IMPLEMENTED(); return -1; }
INTERN ssize_t LIBCCALL libc_getline(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict self) { NOT_IMPLEMENTED(); return -1; }
INTERN FILE *LIBCCALL libc_popen(char const *command, char const *modes) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_pclose(FILE *self) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_fcloseall(void) { NOT_IMPLEMENTED(); return -1; }

INTERN void LIBCCALL libc_clearerr(FILE *self) { if (self) { file_write(self); FILE_CLERR(self); file_endwrite(self); } }
INTERN void LIBCCALL libc_clearerr_unlocked(FILE *self) { if (self) FILE_CLERR(self); }
INTERN int LIBCCALL libc_feof(FILE *self) { return self && FILE_ISEOF(self); }
INTERN int LIBCCALL libc_ferror(FILE *self) { return self && FILE_ISERR(self); }
//INTERN FILE *LIBCCALL libc_fopencookie(void *__restrict magic_cookie, char const *__restrict modes, _IO_cookie_io_functions_t io_funcs);

INTERN char *LIBCCALL
libc_fgets_unlocked(char *__restrict s, size_t n,
                    FILE *__restrict self) {
 NOT_IMPLEMENTED();
 return NULL;
}
INTERN char *LIBCCALL
libc_fgets(char *__restrict s, size_t n, FILE *__restrict self) {
 char *result;
 file_write(self);
 result = libc_fgets_unlocked(s,n,self);
 file_endwrite(self);
 return result;
}
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
DEFINE_INTERN_ALIAS(libc_fgets_int,libc_fgets);
DEFINE_INTERN_ALIAS(libc_fgets_unlocked_int,libc_fgets_unlocked);
#else
INTERN char *LIBCCALL libc_fgets_int(char *__restrict s, int n, FILE *__restrict self) { return libc_fgets(s,(size_t)n,self); }
INTERN char *LIBCCALL libc_fgets_unlocked_int(char *__restrict s, int n, FILE *__restrict self) { return libc_fgets_unlocked(s,(size_t)n,self); }
#endif


/* Doesn't really matter. - Use an atomic_read for both! */
DEFINE_INTERN_ALIAS(libc_fileno_unlocked,libc_fileno);
INTERN void LIBCCALL libc_flockfile(FILE *self) { file_write(self); }
INTERN int LIBCCALL libc_ftrylockfile(FILE *self) { return file_trywrite(self); }
INTERN void LIBCCALL libc_funlockfile(FILE *self) { file_endwrite(self); }
INTERN int LIBCCALL libc_fgetpos(FILE *__restrict self, fpos_t *__restrict pos) { return (int)(*pos = (fpos_t)libc_ftello(self)); }
INTERN int LIBCCALL libc_fsetpos(FILE *self, fpos_t const *pos) { return libc_fseeko(self,*pos,SEEK_SET); }
INTERN int LIBCCALL libc_fgetpos64(FILE *__restrict self, fpos64_t *__restrict pos) { return (int)(*pos = (fpos64_t)libc_ftello64(self)); }
INTERN int LIBCCALL libc_fsetpos64(FILE *self, fpos64_t const *pos) { return libc_fseeko64(self,(off64_t)*pos,SEEK_SET); }
INTERN int LIBCCALL libc_getchar(void) { return libc_fgetc(stdin); }
INTERN int LIBCCALL libc_putchar(int c) { return libc_fputc(c,stdout); }
INTERN int LIBCCALL libc_getchar_unlocked(void) { return libc_fgetc_unlocked(stdin); }
INTERN int LIBCCALL libc_putchar_unlocked(int c) { return libc_fputc_unlocked(c,stdout); }
INTERN void LIBCCALL libc_rewind(FILE *self) { libc_fseeko64(self,0,SEEK_SET); }
INTERN int LIBCCALL libc_fileno(FILE *self) { return ATOMIC_READ(self->f_fd); }
INTERN char *LIBCCALL libc_gets(char *s) { return libc_fgets(s,(size_t)-1,stdin); }
INTERN ssize_t LIBCCALL libc_puts(char const *s) {
 ssize_t result;
 file_write(stdout);
 result = libc_fputs_unlocked(s,stdout);
 if (result >= 0)
     result += libc_fwrite_unlocked("\n",sizeof(char),1,stdout);
 file_endwrite(stdout);
 return result;
}


DEFINE_PUBLIC_ALIAS(fread,libc_fread);
DEFINE_PUBLIC_ALIAS(fwrite,libc_fwrite);
DEFINE_PUBLIC_ALIAS(fread_unlocked,libc_fread_unlocked);
DEFINE_PUBLIC_ALIAS(fwrite_unlocked,libc_fwrite_unlocked);

DEFINE_PUBLIC_ALIAS(fseek,libc_fseek);
DEFINE_PUBLIC_ALIAS(ftell,libc_ftell);
DEFINE_PUBLIC_ALIAS(fseeko,libc_fseeko);
DEFINE_PUBLIC_ALIAS(ftello,libc_ftello);
DEFINE_PUBLIC_ALIAS(fseeko64,libc_fseeko64);
DEFINE_PUBLIC_ALIAS(ftello64,libc_ftello64);
DEFINE_PUBLIC_ALIAS(fgetpos,libc_fgetpos);
DEFINE_PUBLIC_ALIAS(fsetpos,libc_fsetpos);
DEFINE_PUBLIC_ALIAS(fgetpos64,libc_fgetpos64);
DEFINE_PUBLIC_ALIAS(fsetpos64,libc_fsetpos64);

DEFINE_PUBLIC_ALIAS(flockfile,libc_flockfile);
DEFINE_PUBLIC_ALIAS(ftrylockfile,libc_ftrylockfile);
DEFINE_PUBLIC_ALIAS(funlockfile,libc_funlockfile);



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
DEFINE_PUBLIC_ALIAS(open_memself,libc_open_memself);
DEFINE_PUBLIC_ALIAS(getdelim,libc_getdelim);
DEFINE_PUBLIC_ALIAS(getline,libc_getline);
DEFINE_PUBLIC_ALIAS(popen,libc_popen);
DEFINE_PUBLIC_ALIAS(pclose,libc_pclose);
DEFINE_PUBLIC_ALIAS(fcloseall,libc_fcloseall);
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
DEFINE_PUBLIC_ALIAS(fileno_unlocked,libc_fileno_unlocked);
DEFINE_PUBLIC_ALIAS(getc,libc_fgetc);
DEFINE_PUBLIC_ALIAS(putc,libc_fputc);
DEFINE_PUBLIC_ALIAS(getc_unlocked,libc_fgetc_unlocked);
DEFINE_PUBLIC_ALIAS(putc_unlocked,libc_fputc_unlocked);
DEFINE_PUBLIC_ALIAS(__getdelim,libc_getdelim);


/* Wide-string API */
INTERN wint_t LIBCCALL libc_32fgetwc(FILE *self) {
 wint_t result;
 file_write(self);
 result = libc_32fgetwc_unlocked(self);
 file_endwrite(self);
 return result;
}
INTERN wint_t LIBCCALL
libc_32fputwc(char32_t wc, FILE *self) {
 wint_t result;
 file_write(self);
 result = libc_32fgetwc_unlocked(self);
 file_endwrite(self);
 return result;
}
INTERN char32_t *LIBCCALL
libc_32fgetws(char32_t *__restrict ws, size_t n, FILE *__restrict self) {
 char32_t *result;
 file_write(self);
 result = libc_32fgetws_unlocked(ws,n,self);
 file_endwrite(self);
 return result;
}
INTERN ssize_t LIBCCALL
libc_32fputws(char32_t const *__restrict ws, FILE *__restrict self) {
 ssize_t result;
 file_write(self);
 result = libc_32fputws_unlocked(ws,self);
 file_endwrite(self);
 return result;
}
INTERN wint_t LIBCCALL libc_32ungetwc(wint_t wc, FILE *self) { NOT_IMPLEMENTED(); return libc_ungetc((int)wc,self); }
INTERN int LIBCCALL libc_32fwide(FILE *fp, int mode) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32vfwprintf(FILE *__restrict s, char32_t const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32vfwscanf(FILE *__restrict s, char32_t const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32fwprintf(FILE *__restrict self, char32_t const *__restrict format, ...) {
 va_list args; int result; va_start(args,format);
 result = libc_32vfwprintf(self,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t LIBCCALL
libc_32fwscanf(FILE *__restrict self, char32_t const *__restrict format, ...) {
 va_list args; ssize_t result; va_start(args,format);
 result = libc_32vfwscanf(self,format,args);
 va_end(args);
 return result;
}
INTERN FILE *LIBCCALL
libc_32open_wmemself(char32_t **bufloc, size_t *sizeloc) {
 NOT_IMPLEMENTED();
 return NULL;
}
INTERN wint_t LIBCCALL
libc_32fgetwc_unlocked(FILE *self) {
 NOT_IMPLEMENTED();
 return (wint_t)libc_fgetc_unlocked(self);
}
INTERN wint_t LIBCCALL
libc_32fputwc_unlocked(char32_t wc, FILE *self) {
 NOT_IMPLEMENTED();
 return libc_fputc_unlocked((int)wc,self);
}
INTERN char32_t *LIBCCALL
libc_32fgetws_unlocked(char32_t *__restrict ws, size_t n, FILE *__restrict self) {
 NOT_IMPLEMENTED();
 if (n) *ws = '\0';
 return ws;
}
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
DEFINE_INTERN_ALIAS(libc_32fgetws_int,libc_32fgetws);
DEFINE_INTERN_ALIAS(libc_32fgetws_unlocked_int,libc_32fgetws_unlocked);
#else
INTERN char32_t *LIBCCALL libc_32fgetws_int(char32_t *__restrict ws, int n, FILE *__restrict self) { return libc_32fgetws(ws,(size_t)n,self); }
INTERN char32_t *LIBCCALL libc_32fgetws_unlocked_int(char32_t *__restrict ws, int n, FILE *__restrict self) { return libc_32fgetws_unlocked(ws,(size_t)n,self); }
#endif

INTERN ssize_t LIBCCALL
libc_32fputws_unlocked(char32_t const *__restrict ws, FILE *__restrict self) {
 ssize_t result = 0;
 NOT_IMPLEMENTED();
 for (; *ws; ++ws,++result) {
  if (libc_fputc_unlocked((int)*ws,self) == EOF)
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
DEFINE_PUBLIC_ALIAS(open_wmemself,libc_32open_wmemself);
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
