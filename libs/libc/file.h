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
#ifndef GUARD_LIBS_LIBC_STDIO_FILE_H
#define GUARD_LIBS_LIBC_STDIO_FILE_H 1

#include "libc.h"
#include <bits/io-file.h>
#include <hybrid/list/list.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/sync/atomic-owner-rwlock.h>
#include <hybrid/types.h>
#include <stdarg.h>
#include <uchar.h>
#include <xlocale.h>


/* Config: call 'datasync()' on the underlying file
 *         descriptor after fflush() was used to write data. */
#undef CONFIG_FILE_DATASYNC_DURING_FLUSH
//#define CONFIG_FILE_DATASYNC_DURING_FLUSH

DECL_BEGIN

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __fpos_t_defined
#define __fpos_t_defined 1
typedef __FS_TYPE(pos) fpos_t;
#endif /* !__fpos_t_defined */

#ifndef __fpos64_t_defined
#define __fpos64_t_defined 1
typedef __pos64_t   fpos64_t;
#endif /* !__fpos64_t_defined */

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifndef __std_FILE_defined
#define __std_FILE_defined 1
__NAMESPACE_STD_BEGIN
typedef __FILE FILE;
__NAMESPACE_STD_END
#endif /* !__std_FILE_defined */
#ifndef __FILE_defined
#define __FILE_defined 1
__NAMESPACE_STD_USING(FILE)
#endif /* !__FILE_defined */




struct iofile_data {
 uintptr_t             io_zero; /*< Always ZERO(0). - Required for binary compatibility with DOS. */
 atomic_owner_rwlock_t io_lock; /*< Lock for the file. */
 __pos_t               io_pos;  /*< The current in-file position of 'if_fd' */
 size_t                io_read; /*< The amount of bytes within the currently loaded buffer that were read from disk.
                                 *  When 'IO_R' is set, 'io_pos' is located within the buffer at 'if_base+io_read' */
 LIST_NODE(FILE)       io_link; /*< [lock(libc_ffiles_lock)][0..1] Entry in the global chain of open files. (Used by 'fcloseall()', as well as flushing all open files during 'exit()') */
 LIST_NODE(FILE)       io_lnch; /*< [lock(libc_flnchg_lock)][0..1] Chain of line-buffered file that have changed and must be flushed before another line-buffered file is read.
                                 *   NOTE: The standard streams stdin, stdout and stderr are not apart of this list! */
 //mbstate_t           io_mbs;  /*< MB State used for translating unicode data. */
};

INTDEF atomic_rwlock_t libc_ffiles_lock; /* [LOCK_ORDER(AFTER(io_lock))] */
INTDEF atomic_rwlock_t libc_flnchg_lock; /* [LOCK_ORDER(BEFORE(io_lock))] */
INTDEF LIST_HEAD(FILE) libc_ffiles;
INTDEF LIST_HEAD(FILE) libc_flnchg;

#define IO_R       __IO_FILE_IOR /*< The current buffer was read from disk (Undefined when 'if_cnt == 0'). */
#define IO_W       __IO_FILE_IOW /*< The current buffer has changed since being read. */
#define IO_NBF     __IO_FILE_IONBF     /*< ??? */
#define IO_MALLBUF __IO_FILE_IOMALLBUF /*< The buffer was allocated internally. */
#define IO_EOF     __IO_FILE_IOEOF     /*< Set when the file pointed to by 'if_fd' has been exhausted. */
#define IO_ERR     __IO_FILE_IOERR     /*< Set when an I/O error occurred. */
#define IO_NOFD    __IO_FILE_IONOFD    /*< The file acts as output to buffer only. - 'if_fd' is not valid. (Currently ignored when loading/flushing data) */
#define IO_RW      __IO_FILE_IORW      /*< The file was opened for read+write permissions ('+' flag) */
#define IO_USERBUF __IO_FILE_IOUSERBUF /*< The buffer was given by the user. */
#define IO_LNBUF   __IO_FILE_IOLNBUF   /*< NOT DEFINED BY DOS: Use line-buffer mode. */
#define IO_LNIFTYY __IO_FILE_IOLNIFTYY /*< NOT ORIGINALLY DEFINED IN DOS: Determine 'isatty()' on first access and set '__IO_FILE_IOLNBUF' accordingly. */

#define FEOF(self)      ((self)->if_flag&IO_EOF)
#define FERROR(self)    ((self)->if_flag&IO_ERR)
#define FCLEARERR(self) ((self)->if_flag&=~IO_ERR)

#define IOBUF_MAX                8192
#define IOBUF_MIN                512
#define IOBUF_RELOCATE_THRESHOLD 2048 /* When >= this amount of bytes are unused in the buffer, free them. */


#define file_reading(x)     atomic_owner_rwlock_reading(&(x)->if_exdata->f_lock)
#define file_writing(x)     atomic_owner_rwlock_writing(&(x)->if_exdata->f_lock)
#define file_tryread(x)     atomic_owner_rwlock_tryread(&(x)->if_exdata->f_lock)
#define file_trywrite(x)    atomic_owner_rwlock_trywrite(&(x)->if_exdata->f_lock)
#define file_tryupgrade(x)  atomic_owner_rwlock_tryupgrade(&(x)->if_exdata->f_lock)
#define file_read(x)        atomic_owner_rwlock_read(&(x)->if_exdata->f_lock)
#define file_write(x)       atomic_owner_rwlock_write(&(x)->if_exdata->f_lock)
#define file_upgrade(x)     atomic_owner_rwlock_upgrade(&(x)->if_exdata->f_lock)
#define file_downgrade(x)   atomic_owner_rwlock_downgrade(&(x)->if_exdata->f_lock)
#define file_endread(x)     atomic_owner_rwlock_endread(&(x)->if_exdata->f_lock)
#define file_endwrite(x)    atomic_owner_rwlock_endwrite(&(x)->if_exdata->f_lock)


#if 1
/* Lockless */
#undef file_reading
#undef file_writing
#undef file_tryread
#undef file_trywrite
#undef file_tryupgrade
#undef file_read
#undef file_write
#undef file_upgrade
#undef file_downgrade
#undef file_endread
#undef file_endwrite
#define file_reading(x)           1
#define file_writing(x)           1
#define file_tryread(x)           1
#define file_trywrite(x)          1
#define file_tryupgrade(x)        1
#define file_read(x)        (void)0
#define file_write(x)       (void)0
#define file_upgrade(x)     (void)0
#define file_downgrade(x)   (void)0
#define file_endread(x)     (void)0
#define file_endwrite(x)    (void)0
#endif

/* The default buffer limit that should not be surpassed by dynamically allocated buffers.
 * WARNING: Due to user-specified buffers, a file's buffer may surpass this length. */
#define FILE_BUFFERLIMIT  8192
/* Alignment when allocating/increasing buffer size. */
#define FILE_BUFFERALIGN  512



#undef stdin
#undef stdout
#undef stderr
INTDEF FILE libc_std_files[3];
DATDEF FILE *stdin;
DATDEF FILE *stdout;
DATDEF FILE *stderr;

/* Low-level read/write/seek/tell implementation. */
INTDEF size_t LIBCCALL libc_fdoread(void *__restrict buf, size_t size, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_fdowrite(void const *__restrict buf, size_t size, FILE *__restrict self);
INTDEF int LIBCCALL libc_fdoflush(FILE *__restrict self);
INTDEF __pos_t LIBCCALL libc_fdotell(FILE *__restrict self);
INTDEF int LIBCCALL libc_fdoseek(FILE *__restrict self, __off_t off, int whence);
INTDEF int LIBCCALL libc_dosetvbuf(FILE *__restrict self, char *__restrict buf, int modes, size_t n);
INTDEF int LIBCCALL libc_doungetc(int c, FILE *__restrict self);
/* Try to fill unused buffer memory with new data, allocating a new buffer if none was available before. */
INTDEF int LIBCCALL libc_doffill(FILE *__restrict self);

/* Flush all line-buffered file streams that have changed.
 * This function is called before data is read from a
 * line-buffered source 'sender' that is already write-locked. */
INTDEF void LIBCCALL libc_flush_changed_lnbuf_files(FILE *__restrict sender);

/* All all user-defined input streams, not including stdin, stdout or stderr.
 * NOTE: Errors that may occur during this act are ignored, so-as to
 *       ensure that attempts at flushing later files are always made. */
INTDEF void LIBCCALL libc_flushall_nostd(void);
/* Same as 'libc_flushall_nostd', but also flush standard streams. */
INTDEF void LIBCCALL libc_flushall(void);

INTDEF size_t LIBCCALL libc_fread_unlocked(void *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_fwrite_unlocked(void const *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_fread(void *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_fwrite(void const *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF ssize_t LIBCCALL libc_file_printer(char const *__restrict data, size_t datalen, void *closure);

INTDEF void LIBCCALL libc_flockfile(FILE *self);
INTDEF int LIBCCALL libc_ftrylockfile(FILE *self);
INTDEF void LIBCCALL libc_funlockfile(FILE *self);

INTDEF int LIBCCALL libc_fseek(FILE *self, long int off, int whence);
INTDEF int LIBCCALL libc_fseeko(FILE *self, off_t off, int whence);
INTDEF int LIBCCALL libc_fseeko64(FILE *self, off64_t off, int whence);
INTDEF long int LIBCCALL libc_ftell(FILE *self);
INTDEF off_t LIBCCALL libc_ftello(FILE *self);
INTDEF off64_t LIBCCALL libc_ftello64(FILE *self);
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF int LIBCCALL libc_fseeko_unlocked(FILE *self, off_t off, int whence);
INTDEF int LIBCCALL libc_fseeko64_unlocked(FILE *self, off64_t off, int whence);
INTDEF off_t LIBCCALL libc_ftello_unlocked(FILE *self);
INTDEF off64_t LIBCCALL libc_ftello64_unlocked(FILE *self);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
INTDEF int LIBCCALL libc_fgetpos(FILE *__restrict self, fpos_t *__restrict pos);
INTDEF int LIBCCALL libc_fsetpos(FILE *self, fpos_t const *pos);
INTDEF int LIBCCALL libc_fgetpos64(FILE *__restrict self, fpos64_t *__restrict pos);
INTDEF int LIBCCALL libc_fsetpos64(FILE *self, fpos64_t const *pos);

INTDEF FILE *LIBCCALL libc_tmpfile64(void);
INTDEF FILE *LIBCCALL libc_tmpfile(void);
INTDEF FILE *LIBCCALL libc_fdopen(int fd, char const *__restrict modes);
INTDEF FILE *LIBCCALL libc_fdreopen(int fd, char const *__restrict modes, FILE *__restrict self, int mode);
INTDEF FILE *LIBCCALL libc_fopen(char const *__restrict filename, char const *__restrict modes);
INTDEF FILE *LIBCCALL libc_fopen64(char const *__restrict filename, char const *__restrict modes);
INTDEF FILE *LIBCCALL libc_freopen(char const *__restrict filename, char const *__restrict modes, FILE *__restrict self);
INTDEF FILE *LIBCCALL libc_freopen64(char const *__restrict filename, char const *__restrict modes, FILE *__restrict self);
INTDEF FILE *LIBCCALL libc_fmemopen(void *s, size_t len, char const *modes);
INTDEF FILE *LIBCCALL libc_open_memstream(char **bufloc, size_t *sizeloc);
INTDEF FILE *LIBCCALL libc_popen(char const *command, char const *modes);

INTDEF int LIBCCALL libc_fclose(FILE *self);
INTDEF int LIBCCALL libc_pclose(FILE *self);
INTDEF int LIBCCALL libc_fcloseall(void);

INTDEF ssize_t LIBCCALL   libc_vfprintf(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_fprintf(FILE *__restrict self, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL   libc_vprintf(char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_printf(char const *__restrict format, ...);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF ssize_t LIBCCALL   libc_dos_vfprintf(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_fprintf(FILE *__restrict self, char const *__restrict format, ...);
INTDEF ssize_t LIBCCALL   libc_dos_vprintf(char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_dos_printf(char const *__restrict format, ...);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

INTDEF ssize_t LIBCCALL libc_vfscanf(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF ssize_t LIBCCALL libc_vscanf(char const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_fscanf(FILE *__restrict self, char const *__restrict format, ...);
INTDEF ssize_t ATTR_CDECL libc_scanf(char const *__restrict format, ...);
INTDEF int LIBCCALL libc_getc(FILE *self);
INTDEF int LIBCCALL libc_fgetc(FILE *self);
INTDEF int LIBCCALL libc_getc_unlocked(FILE *self);
INTDEF int LIBCCALL libc_fgetc_unlocked(FILE *self);
INTDEF int LIBCCALL libc_putc(int c, FILE *self);
INTDEF int LIBCCALL libc_fputc(int c, FILE *self);
INTDEF int LIBCCALL libc_putc_unlocked(int c, FILE *self);
INTDEF int LIBCCALL libc_fputc_unlocked(int c, FILE *self);
INTDEF int LIBCCALL libc_getw(FILE *self);
INTDEF int LIBCCALL libc_putw(int w, FILE *self);
INTDEF ssize_t LIBCCALL libc_fputs(char const *__restrict s, FILE *__restrict self);
INTDEF ssize_t LIBCCALL libc_fputs_unlocked(char const *__restrict s, FILE *__restrict self);
INTDEF void LIBCCALL libc_clearerr(FILE *self);
INTDEF int LIBCCALL libc_fflush(FILE *self);
INTDEF void LIBCCALL libc_setbuf(FILE *__restrict self, char *__restrict buf);
INTDEF int LIBCCALL libc_setvbuf(FILE *__restrict self, char *__restrict buf, int modes, size_t n);
INTDEF int LIBCCALL libc_ungetc(int c, FILE *self);
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF int LIBCCALL libc_ungetc_unlocked(int c, FILE *self);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
INTDEF int LIBCCALL libc_fflush_unlocked(FILE *self);
INTDEF void LIBCCALL libc_setbuffer(FILE *__restrict self, char *__restrict buf, size_t size);
INTDEF void LIBCCALL libc_setlinebuf(FILE *self);
INTDEF int LIBCCALL libc_feof_unlocked(FILE *self);
INTDEF int LIBCCALL libc_ferror_unlocked(FILE *self);
INTDEF ssize_t LIBCCALL libc_getdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict self);
INTDEF ssize_t LIBCCALL libc_getline(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict self);
INTDEF void LIBCCALL libc_clearerr_unlocked(FILE *self);
INTDEF int LIBCCALL libc_feof(FILE *self);
INTDEF int LIBCCALL libc_ferror(FILE *self);
//INTDEF FILE *LIBCCALL libc_fopencookie(void *__restrict magic_cookie, char const *__restrict modes, _IO_cookie_io_functions_t io_funcs);
INTDEF char *LIBCCALL libc_fgets(char *__restrict s, size_t n, FILE *__restrict self);
INTDEF char *LIBCCALL libc_fgets_unlocked(char *__restrict s, size_t n, FILE *__restrict self);
INTDEF char *LIBCCALL libc_fgets_int(char *__restrict s, int n, FILE *__restrict self);
INTDEF char *LIBCCALL libc_fgets_unlocked_int(char *__restrict s, int n, FILE *__restrict self);
INTDEF int LIBCCALL libc_getchar(void);
INTDEF int LIBCCALL libc_putchar(int c);
INTDEF int LIBCCALL libc_getchar_unlocked(void);
INTDEF int LIBCCALL libc_putchar_unlocked(int c);
INTDEF void LIBCCALL libc_rewind(FILE *self);
INTDEF int LIBCCALL libc_fileno(FILE *self);
INTDEF int LIBCCALL libc_fileno_unlocked(FILE *self);
INTDEF char *LIBCCALL libc_gets(char *s);
INTDEF ssize_t LIBCCALL libc_puts(char const *s);
INTDEF int LIBCCALL libc_fwide(FILE *file, int mode);

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */

/* UTF32 string functions. */
INTDEF wint_t LIBCCALL libc_32getwchar(void);
INTDEF wint_t LIBCCALL libc_32fgetwc(FILE *self);
INTDEF wint_t LIBCCALL libc_32getwc(FILE *self);
INTDEF wint_t LIBCCALL libc_32putwchar(char32_t wc);
INTDEF wint_t LIBCCALL libc_32fputwc(char32_t wc, FILE *self);
INTDEF wint_t LIBCCALL libc_32putwc(char32_t wc, FILE *self);
INTDEF char32_t *LIBCCALL libc_32fgetws(char32_t *__restrict buf, size_t n, FILE *__restrict self);
INTDEF char32_t *LIBCCALL libc_32fgetws_int(char32_t *__restrict buf, int n, FILE *__restrict self);
INTDEF int LIBCCALL libc_32fputws(char32_t const *__restrict str, FILE *__restrict self);
INTDEF wint_t LIBCCALL libc_32ungetwc(wint_t wc, FILE *self);
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF wint_t LIBCCALL libc_32ungetwc_unlocked(wint_t wc, FILE *self);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
INTDEF ssize_t LIBCCALL libc_32vfwprintf(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_32fwprintf(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_32vwprintf(char32_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_32wprintf(char32_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_32vfwscanf(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_32fwscanf(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_32vwscanf(char32_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_32wscanf(char32_t const *__restrict format, ...);
INTDEF FILE *LIBCCALL libc_32open_wmemstream(char32_t **bufloc, size_t *sizeloc);
INTDEF wint_t LIBCCALL libc_32getwc_unlocked(FILE *self);
INTDEF wint_t LIBCCALL libc_32getwchar_unlocked(void);
INTDEF wint_t LIBCCALL libc_32fgetwc_unlocked(FILE *self);
INTDEF wint_t LIBCCALL libc_32fputwc_unlocked(char32_t wc, FILE *self);
INTDEF wint_t LIBCCALL libc_32putwc_unlocked(char32_t wc, FILE *self);
INTDEF wint_t LIBCCALL libc_32putwchar_unlocked(char32_t wc);
INTDEF char32_t *LIBCCALL libc_32fgetws_unlocked(char32_t *__restrict wstr, size_t n, FILE *__restrict self);
INTDEF char32_t *LIBCCALL libc_32fgetws_unlocked_int(char32_t *__restrict wstr, int n, FILE *__restrict self);
INTDEF int LIBCCALL libc_32fputws_unlocked(char32_t const *__restrict wstr, FILE *__restrict self);


#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF FILE *LIBCCALL libc_p_iob(void);

INTDEF FILE *LIBCCALL libc_dos_fopen(char const *__restrict filename, char const *__restrict modes);
INTDEF FILE *LIBCCALL libc_dos_freopen(char const *__restrict filename, char const *__restrict modes, FILE *__restrict self);
INTDEF FILE *LIBCCALL libc_dos_fopen64(char const *__restrict filename, char const *__restrict modes);
INTDEF FILE *LIBCCALL libc_dos_freopen64(char const *__restrict filename, char const *__restrict modes, FILE *__restrict self);

INTDEF wint_t LIBCCALL libc_16getwchar(void);
INTDEF wint_t LIBCCALL libc_16fgetwc(FILE *self);
INTDEF wint_t LIBCCALL libc_16getwc(FILE *self);
INTDEF wint_t LIBCCALL libc_16putwchar(char16_t wc);
INTDEF wint_t LIBCCALL libc_16fputwc(char16_t wc, FILE *self);
INTDEF wint_t LIBCCALL libc_16putwc(char16_t wc, FILE *self);
INTDEF char16_t *LIBCCALL libc_16fgetws(char16_t *__restrict buf, size_t n, FILE *__restrict self);
INTDEF char16_t *LIBCCALL libc_16fgetws_int(char16_t *__restrict buf, int n, FILE *__restrict self);
INTDEF int LIBCCALL libc_16fputws(char16_t const *__restrict str, FILE *__restrict self);
INTDEF wint_t LIBCCALL libc_16ungetwc(wint_t wc, FILE *self);
INTDEF wint_t LIBCCALL libc_16ungetwc_unlocked(wint_t wc, FILE *self);
INTDEF ssize_t LIBCCALL libc_16vfwprintf(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_16fwprintf(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_16vwprintf(char16_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_16wprintf(char16_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_16vfwscanf(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_16fwscanf(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_16vwscanf(char16_t const *__restrict format, va_list args);
INTDEF ssize_t ATTR_CDECL libc_16wscanf(char16_t const *__restrict format, ...);
INTDEF FILE *LIBCCALL libc_16open_wmemstream(char16_t **bufloc, size_t *sizeloc);
INTDEF wint_t LIBCCALL libc_16getwc_unlocked(FILE *self);
INTDEF wint_t LIBCCALL libc_16getwchar_unlocked(void);
INTDEF wint_t LIBCCALL libc_16fgetwc_unlocked(FILE *self);
INTDEF wint_t LIBCCALL libc_16fputwc_unlocked(char16_t wc, FILE *self);
INTDEF wint_t LIBCCALL libc_16putwc_unlocked(char16_t wc, FILE *self);
INTDEF wint_t LIBCCALL libc_16putwchar_unlocked(char16_t wc);
INTDEF char16_t *LIBCCALL libc_16fgetws_unlocked(char16_t *__restrict wbuf, size_t n, FILE *__restrict self);
INTDEF char16_t *LIBCCALL libc_16fgetws_unlocked_int(char16_t *__restrict wbuf, int n, FILE *__restrict self);
INTDEF int LIBCCALL libc_16fputws_unlocked(char16_t const *__restrict wstr, FILE *__restrict self);

INTDEF FILE *LIBCCALL libc_dos_fsopen(char const *filename, char const *mode, int shflag);
INTDEF int LIBCCALL libc_rmtmp(void);
INTDEF int LIBCCALL libc_filbuf(FILE *__restrict self);
INTDEF int LIBCCALL libc_flsbuf(int ch, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_fread_s(void *__restrict buf, size_t bufsize, size_t elemsize, size_t elemcount, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_fread_unlocked_s(void *__restrict buf, size_t bufsize, size_t elemsize, size_t elemcount, FILE *__restrict self);

INTDEF int LIBCCALL libc_getmaxstdio(void);
INTDEF int LIBCCALL libc_setmaxstdio(int val);
INTDEF int LIBCCALL libc_get_printf_count_output(void);
INTDEF int LIBCCALL libc_set_printf_count_output(int val);
INTDEF u32 LIBCCALL libc_get_output_format(void);
INTDEF u32 LIBCCALL libc_set_output_format(u32 format);

INTDEF int ATTR_CDECL libc_fscanf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF int ATTR_CDECL libc_fscanf_s_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF int ATTR_CDECL libc_scanf_l(char const *__restrict format, locale_t locale, ...);
INTDEF int ATTR_CDECL libc_scanf_s_l(char const *__restrict format, locale_t locale, ...);

INTDEF int ATTR_CDECL libc_dos_printf_p(char const *__restrict format, ...);
INTDEF int LIBCCALL   libc_dos_vprintf_p(char const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_dos_printf_l(char const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_dos_vprintf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_dos_printf_p_l(char const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_dos_vprintf_p_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_dos_printf_s_l(char const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_dos_vprintf_s_l(char const *__restrict format, locale_t locale, va_list args);

INTDEF int ATTR_CDECL libc_dos_fprintf_p(FILE *__restrict self, char const *__restrict format, ...);
INTDEF int LIBCCALL   libc_dos_vfprintf_p(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_dos_fprintf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_dos_vfprintf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_dos_fprintf_p_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_dos_vfprintf_p_l(FILE *__restrict self, char const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_dos_fprintf_s_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_dos_vfprintf_s_l(FILE *__restrict self, char const *__restrict format, locale_t locale, va_list args);

INTDEF errno_t LIBCCALL libc_fopen_s(FILE **pfile, char const *filename, char const *mode);
INTDEF errno_t LIBCCALL libc_clearerr_s(FILE *__restrict self);
INTDEF errno_t LIBCCALL libc_freopen_s(FILE **pfile, char const *filename, char const *mode, FILE *oldfile);
INTDEF char *LIBCCALL libc_gets_s(char *__restrict buf, size_t bufsize);
INTDEF errno_t LIBCCALL libc_tmpfile_s(FILE **pfile);
INTDEF errno_t LIBCCALL libc_dos_fopen_s(FILE **pfile, char const *filename, char const *mode);
INTDEF errno_t LIBCCALL libc_dos_freopen_s(FILE **pfile, char const *filename, char const *mode, FILE *oldfile);
INTDEF errno_t LIBCCALL libc_dos_tmpfile_s(FILE **pfile);

INTDEF int ATTR_CDECL libc_dos_printf_s(char const *__restrict format, ...);
INTDEF int LIBCCALL   libc_dos_vprintf_s(char const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_dos_fprintf_s(FILE *__restrict self, char const *__restrict format, ...);
INTDEF int LIBCCALL   libc_dos_vfprintf_s(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_fscanf_s(FILE *__restrict self, char const *__restrict format, ...);
INTDEF int LIBCCALL   libc_vfscanf_s(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_scanf_s(char const *__restrict format, ...);
INTDEF int LIBCCALL   libc_vscanf_s(char const *__restrict format, va_list args);

/* DOS Widechar extension API. */
INTDEF FILE *LIBCCALL libc_16wfsopen(char16_t const *filename, char16_t const *mode, int shflag);
INTDEF FILE *LIBCCALL libc_16wfopen(char16_t const *filename, char16_t const *mode);
INTDEF FILE *LIBCCALL libc_16wfreopen(char16_t const *filename, char16_t const *mode, FILE *oldfile);
INTDEF FILE *LIBCCALL libc_32wfsopen(char32_t const *filename, char32_t const *mode, int shflag);
INTDEF FILE *LIBCCALL libc_32wfopen(char32_t const *filename, char32_t const *mode);
INTDEF FILE *LIBCCALL libc_32wfreopen(char32_t const *filename, char32_t const *mode, FILE *oldfile);
INTDEF errno_t LIBCCALL libc_16wfopen_s(FILE **pfile, char16_t const *filename, char16_t const *mode);
INTDEF errno_t LIBCCALL libc_16wfreopen_s(FILE **pfile, char16_t const *filename, char16_t const *mode, FILE *oldfile);
INTDEF errno_t LIBCCALL libc_32wfopen_s(FILE **pfile, char32_t const *filename, char32_t const *mode);
INTDEF errno_t LIBCCALL libc_32wfreopen_s(FILE **pfile, char32_t const *filename, char32_t const *mode, FILE *oldfile);
INTDEF FILE *LIBCCALL libc_dos_16wfsopen(char16_t const *filename, char16_t const *mode, int shflag);
INTDEF FILE *LIBCCALL libc_dos_16wfopen(char16_t const *filename, char16_t const *mode);
INTDEF FILE *LIBCCALL libc_dos_16wfreopen(char16_t const *filename, char16_t const *mode, FILE *oldfile);
INTDEF FILE *LIBCCALL libc_dos_32wfsopen(char32_t const *filename, char32_t const *mode, int shflag);
INTDEF FILE *LIBCCALL libc_dos_32wfopen(char32_t const *filename, char32_t const *mode);
INTDEF FILE *LIBCCALL libc_dos_32wfreopen(char32_t const *filename, char32_t const *mode, FILE *oldfile);
INTDEF errno_t LIBCCALL libc_dos_16wfopen_s(FILE **pfile, char16_t const *filename, char16_t const *mode);
INTDEF errno_t LIBCCALL libc_dos_16wfreopen_s(FILE **pfile, char16_t const *filename, char16_t const *mode, FILE *oldfile);
INTDEF errno_t LIBCCALL libc_dos_32wfopen_s(FILE **pfile, char32_t const *filename, char32_t const *mode);
INTDEF errno_t LIBCCALL libc_dos_32wfreopen_s(FILE **pfile, char32_t const *filename, char32_t const *mode, FILE *oldfile);
INTDEF FILE *LIBCCALL libc_16wpopen(char16_t const *command, char16_t const *mode);
INTDEF FILE *LIBCCALL libc_32wpopen(char32_t const *command, char32_t const *mode);
INTDEF FILE *LIBCCALL libc_16wfdopen(int fd, char16_t const *mode);
INTDEF FILE *LIBCCALL libc_32wfdopen(int fd, char32_t const *mode);

/* Get wide character functions */
INTDEF wint_t LIBCCALL libc_16fgetwchar(void);
INTDEF wint_t LIBCCALL libc_32fgetwchar(void);

/* Put wide character functions */
INTDEF wint_t LIBCCALL libc_16fputwchar(char16_t ch);
INTDEF wint_t LIBCCALL libc_32fputwchar(char32_t ch);

/* Get wide string functions */
INTDEF char16_t *LIBCCALL libc_16getws(char16_t *__restrict buf);
INTDEF char32_t *LIBCCALL libc_32getws(char32_t *__restrict buf);
INTDEF char16_t *LIBCCALL libc_16getws_s(char16_t *__restrict buf, size_t buflen);
INTDEF char32_t *LIBCCALL libc_32getws_s(char32_t *__restrict buf, size_t buflen);

/* Put wide string functions */
INTDEF int LIBCCALL libc_16putws(char16_t const *__restrict str);
INTDEF int LIBCCALL libc_32putws(char32_t const *__restrict str);

INTDEF int ATTR_CDECL libc_16wprintf_p(char16_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_16vwprintf_p(char16_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_16wprintf_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_16vwprintf_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_16wprintf_p_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_16vwprintf_p_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_16wprintf_s_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_16vwprintf_s_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_16fwprintf_p(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_16vfwprintf_p(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_16fwprintf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_16vfwprintf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_16fwprintf_p_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_16vfwprintf_p_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_16fwprintf_s_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_16vfwprintf_s_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_32wprintf_p(char32_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_32vwprintf_p(char32_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_32wprintf_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_32vwprintf_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_32wprintf_p_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_32vwprintf_p_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_32wprintf_s_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_32vwprintf_s_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_32fwprintf_p(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_32vfwprintf_p(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_32fwprintf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_32vfwprintf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_32fwprintf_p_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_32vfwprintf_p_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF int ATTR_CDECL libc_32fwprintf_s_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...);
INTDEF int LIBCCALL   libc_32vfwprintf_s_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, va_list args);

/* NOTE: ~safe~ functions are re-directed to the regular versions. (For the reason, see below) */
INTDEF int ATTR_CDECL libc_16fwscanf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF int ATTR_CDECL libc_16fwscanf_s_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF int ATTR_CDECL libc_16wscanf_l(char16_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF int ATTR_CDECL libc_16wscanf_s_l(char16_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF int ATTR_CDECL libc_32fwscanf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF int ATTR_CDECL libc_32fwscanf_s_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF int ATTR_CDECL libc_32wscanf_l(char32_t const *__restrict format, locale_t locale, ...); /* No varargs version. */
INTDEF int ATTR_CDECL libc_32wscanf_s_l(char32_t const *__restrict format, locale_t locale, ...); /* No varargs version. */

/* Simply redirect these so-called ~safe~ functions to the regular version.
 * In KOS, they're already ~safe~ to begin with, because unknown format strings are always handled.
 * NOTE: For binary compatibility, assembly names such as 'fwprintf_s' are exported as alias,
 *       but should never be used by newly compiled applications. */
INTDEF int ATTR_CDECL libc_16fwprintf_s(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_16vfwprintf_s(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_16wprintf_s(char16_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_16vwprintf_s(char16_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_16fwscanf_s(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_16vfwscanf_s(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_16wscanf_s(char16_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_16vwscanf_s(char16_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_32fwprintf_s(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_32vfwprintf_s(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_32wprintf_s(char32_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_32vwprintf_s(char32_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_32fwscanf_s(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_32vfwscanf_s(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF int ATTR_CDECL libc_32wscanf_s(char32_t const *__restrict format, ...);
INTDEF int LIBCCALL   libc_32vwscanf_s(char32_t const *__restrict format, va_list args);


/* DOS Console I/O function aliases. */
/* TODO: Add <conio.h> */

#endif /* CONFIG_LIBC_NO_DOS_LIBC */






/* FILE-unrelated stdio functions. */
struct obstack;
INTDEF void LIBCCALL perror(char const *__s);
INTDEF char *LIBCCALL ctermid(char *__s);
INTDEF int LIBCCALL obstack_printf(struct obstack *__restrict __obstack, char const *__restrict format, ...);
INTDEF int LIBCCALL obstack_vprintf(struct obstack *__restrict __obstack, char const *__restrict format, va_list args);
INTDEF char *LIBCCALL cuserid(char *__s);


DECL_END

#endif /* !GUARD_LIBS_LIBC_STDIO_FILE_H */
