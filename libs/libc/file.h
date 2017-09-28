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

#define CONFIG_STDIO_FILE_RECURSIVE_LOCK 1

#include "libc.h"
#include "stdio.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#ifdef CONFIG_STDIO_FILE_RECURSIVE_LOCK
#include <hybrid/sync/atomic-owner-rwlock.h>
#else
#include <hybrid/sync/atomic-rwlock.h>
#endif
#include <uchar.h>

DECL_BEGIN

__NAMESPACE_STD_BEGIN
struct _IO_FILE {
#ifdef CONFIG_STDIO_FILE_RECURSIVE_LOCK
 atomic_owner_rwlock_t
                   f_lock;   /*< Lock used to synchronize access to this file. */
#else
 atomic_rwlock_t   f_lock;   /*< Lock used to synchronize access to this file. */
#endif
 int               f_fd;     /*< [lock(f_lock)] The underlying system file descriptor.
                              *   NOTE: The R/W pointer of this descriptor is assumed to point
                              *         after the currently loaded buffer when 'FILE_BUFLOD',
                              *         or to point at its start when 'FILE_BUFLOD' isn't set. */
#define FILE_ERROR   0x0001  /*< The file is in an error state. */
#define FILE_BUFCNG  0x0002  /*< The current contents of 'f_buffer' have changed since being loaded. */
#define FILE_BUFLOD  0x0004  /*< The current contents of 'f_buffer' have been loaded from disk. */
/*                   0x07f8   */
#define FILE_NOBUF   0x0200  /*< The file isn't buffered. (Always use direct I/O)
                              *  NOTE: When set, 'f_bufpos == f_bufmax == f_buffer' */
#define FILE_LNBUFIT 0x0200  /*< The file is line-buffered if it refers to a tty device. (Deleted the first time the file is written to) */
#define FILE_LNBUF   0x0400  /*< The file is line-buffered. (Flush after a '\n' character is written) */
#define FILE_USERBUF 0x0800  /*< The current buffer is user-allocated and may not be replaced, or free()'d. */
#define FILE_READOK  0x1000  /*< The file was opened for reading ('r' / 'w+'). */
#define FILE_WRITEOK 0x2000  /*< The file was opened for writing ('w' / 'r+'). */
#define FILE_APPEND  0x4000  /*< The file was opened in append mode ('a'). */
#define FILE_BINARY  0x8000  /*< The file was opened in binary mode ('b'). */
 unsigned int      f_flags;  /*< [lock(f_lock)] Set of 'FILE_*', describing the file's state. */
 byte_t           *f_buffer; /*< [lock(f_lock)][0..1][owned_if(!FILE_USERBUF)] Base address of the allocated buffer. */
 byte_t           *f_bufpos; /*< [lock(f_lock)][0..1] Current buffer position. */
 byte_t           *f_bufend; /*< [lock(f_lock)][0..1] End of the currently loaded (valid) buffer. */
 byte_t           *f_bufmax; /*< [lock(f_lock)][0..1] End of the currently allocated buffer, or
                              *   the first address no longer apart of a user-specified buffer. */
 byte_t           *f_bufptr; /*< [lock(f_lock)][0..1] Pointer offset from 'f_buffer', describing the current . */
 __pos_t           f_bfaddr; /*< [lock(f_lock)] In-file address of 'f_buffer'. */
};
#define FILE_ISERR(self)    ((self)->f_flags&FILE_ERROR)
#define FILE_CLERR(self)    ((self)->f_flags &= ~FILE_ERROR)
#define FILE_ISEOF(self)    ((self)->f_bufmax != (self)->f_bufend)
#ifdef CONFIG_STDIO_FILE_RECURSIVE_LOCK
#define FILE_INIT(fd,flags) {ATOMIC_OWNER_RWLOCK_INIT,fd,flags,NULL,NULL,NULL,NULL,NULL,0}
#define file_reading(x)     atomic_owner_rwlock_reading(&(x)->f_lock)
#define file_writing(x)     atomic_owner_rwlock_writing(&(x)->f_lock)
#define file_tryread(x)     atomic_owner_rwlock_tryread(&(x)->f_lock)
#define file_trywrite(x)    atomic_owner_rwlock_trywrite(&(x)->f_lock)
#define file_tryupgrade(x)  atomic_owner_rwlock_tryupgrade(&(x)->f_lock)
#define file_read(x)        atomic_owner_rwlock_read(&(x)->f_lock)
#define file_write(x)       atomic_owner_rwlock_write(&(x)->f_lock)
#define file_upgrade(x)     atomic_owner_rwlock_upgrade(&(x)->f_lock)
#define file_downgrade(x)   atomic_owner_rwlock_downgrade(&(x)->f_lock)
#define file_endread(x)     atomic_owner_rwlock_endread(&(x)->f_lock)
#define file_endwrite(x)    atomic_owner_rwlock_endwrite(&(x)->f_lock)
#else
#define FILE_INIT(fd,flags) {ATOMIC_RWLOCK_INIT,fd,flags,NULL,NULL,NULL,NULL,NULL,0}
#define file_reading(x)     atomic_rwlock_reading(&(x)->f_lock)
#define file_writing(x)     atomic_rwlock_writing(&(x)->f_lock)
#define file_tryread(x)     atomic_rwlock_tryread(&(x)->f_lock)
#define file_trywrite(x)    atomic_rwlock_trywrite(&(x)->f_lock)
#define file_tryupgrade(x)  atomic_rwlock_tryupgrade(&(x)->f_lock)
#define file_read(x)        atomic_rwlock_read(&(x)->f_lock)
#define file_write(x)       atomic_rwlock_write(&(x)->f_lock)
#define file_upgrade(x)     atomic_rwlock_upgrade(&(x)->f_lock)
#define file_downgrade(x)   atomic_rwlock_downgrade(&(x)->f_lock)
#define file_endread(x)     atomic_rwlock_endread(&(x)->f_lock)
#define file_endwrite(x)    atomic_rwlock_endwrite(&(x)->f_lock)
#endif


/* The default buffer limit that should not be surpassed by dynamically allocated buffers.
 * WARNING: Due to user-specified buffers, a file's buffer may surpass this length. */
#define FILE_BUFFERLIMIT  8192
/* Alignment when allocating/increasing buffer size. */
#define FILE_BUFFERALIGN  512



#ifndef __std_FILE_defined
#define __std_FILE_defined 1
typedef __FILE FILE;
#endif /* !__std_FILE_defined */
__NAMESPACE_STD_END
#ifndef __FILE_defined
#define __FILE_defined 1
__NAMESPACE_STD_USING(FILE)
#endif /* !__FILE_defined */


#undef stdin
#undef stdout
#undef stderr
INTDEF FILE libc_std_files[3];
DATDEF FILE *stdin;
DATDEF FILE *stdout;
DATDEF FILE *stderr;

/* Low-level read/write/seek/tell implementation. */
INTDEF NONNULL((3)) size_t LIBCCALL libc_fread_impl(void *__restrict ptr, size_t size, FILE *__restrict stream);
INTDEF NONNULL((3)) size_t LIBCCALL libc_fwrite_impl(void const *__restrict ptr, size_t size, FILE *__restrict stream);
INTDEF NONNULL((1)) __off_t LIBCCALL libc_ftell_impl(FILE *__restrict self);
INTDEF NONNULL((1)) int LIBCCALL libc_fseek_impl(FILE *__restrict self, __off_t off, int whence);

INTDEF size_t LIBCCALL libc_fread_unlocked(void *__restrict ptr, size_t size, size_t n, FILE *__restrict stream);
INTDEF size_t LIBCCALL libc_fwrite_unlocked(void const *__restrict ptr, size_t size, size_t n, FILE *__restrict stream);
INTDEF size_t LIBCCALL libc_fread(void *__restrict ptr, size_t size, size_t n, FILE *__restrict stream);
INTDEF size_t LIBCCALL libc_fwrite(void const *__restrict ptr, size_t size, size_t n, FILE *__restrict stream);
INTDEF ssize_t LIBCCALL libc_file_printer(char const *__restrict data, size_t datalen, void *closure);

INTDEF void LIBCCALL libc_flockfile(FILE *stream);
INTDEF int LIBCCALL libc_ftrylockfile(FILE *stream);
INTDEF void LIBCCALL libc_funlockfile(FILE *stream);

INTDEF int LIBCCALL libc_fseek(FILE *stream, long int off, int whence);
INTDEF int LIBCCALL libc_fseeko(FILE *stream, off_t off, int whence);
INTDEF int LIBCCALL libc_fseeko64(FILE *stream, off64_t off, int whence);
INTDEF long int LIBCCALL libc_ftell(FILE *stream);
INTDEF off_t LIBCCALL libc_ftello(FILE *stream);
INTDEF off64_t LIBCCALL libc_ftello64(FILE *stream);
INTDEF int LIBCCALL libc_fgetpos(FILE *__restrict stream, fpos_t *__restrict pos);
INTDEF int LIBCCALL libc_fsetpos(FILE *stream, fpos_t const *pos);
INTDEF int LIBCCALL libc_fgetpos64(FILE *__restrict stream, fpos64_t *__restrict pos);
INTDEF int LIBCCALL libc_fsetpos64(FILE *stream, fpos64_t const *pos);


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
INTDEF void LIBCCALL libc_clearerr_unlocked(FILE *stream);
INTDEF int LIBCCALL libc_feof(FILE *stream);
INTDEF int LIBCCALL libc_ferror(FILE *stream);
//INTDEF FILE *LIBCCALL libc_fopencookie(void *__restrict magic_cookie, char const *__restrict modes, _IO_cookie_io_functions_t io_funcs);
INTDEF char *LIBCCALL libc_fgets(char *__restrict s, size_t n, FILE *__restrict stream);
INTDEF char *LIBCCALL libc_fgets_unlocked(char *__restrict s, size_t n, FILE *__restrict stream);
INTDEF char *LIBCCALL libc_fgets_int(char *__restrict s, int n, FILE *__restrict stream);
INTDEF char *LIBCCALL libc_fgets_unlocked_int(char *__restrict s, int n, FILE *__restrict stream);
INTDEF int LIBCCALL libc_getchar(void);
INTDEF int LIBCCALL libc_putchar(int c);
INTDEF int LIBCCALL libc_getchar_unlocked(void);
INTDEF int LIBCCALL libc_putchar_unlocked(int c);
INTDEF void LIBCCALL libc_rewind(FILE *stream);
INTDEF int LIBCCALL libc_fileno(FILE *stream);
INTDEF int LIBCCALL libc_fileno_unlocked(FILE *stream);
INTDEF char *LIBCCALL libc_gets(char *s);
INTDEF ssize_t LIBCCALL libc_puts(char const *s);

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */

INTDEF ssize_t LIBCCALL libc_32wprintf(char32_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_32vwprintf(char32_t const *__restrict format, va_list arg);
INTDEF ssize_t LIBCCALL libc_32wscanf(char32_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_32vwscanf(char32_t const *__restrict format, va_list arg);
INTDEF wint_t LIBCCALL libc_32fgetwc(FILE *stream);
INTDEF wint_t LIBCCALL libc_32fputwc(char32_t wc, FILE *stream);
INTDEF ssize_t LIBCCALL libc_32fputws(char32_t const *__restrict ws, FILE *__restrict stream);
INTDEF char32_t *LIBCCALL libc_32fgetws(char32_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF char32_t *LIBCCALL libc_32fgetws_unlocked(char32_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF char32_t *LIBCCALL libc_32fgetws_int(char32_t *__restrict ws, int n, FILE *__restrict stream);
INTDEF char32_t *LIBCCALL libc_32fgetws_unlocked_int(char32_t *__restrict ws, int n, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_32ungetwc(wint_t wc, FILE *stream);
INTDEF int LIBCCALL libc_32fwide(FILE *fp, int mode);
INTDEF ssize_t LIBCCALL libc_32fwprintf(FILE *__restrict stream, char32_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_32vfwprintf(FILE *__restrict s, char32_t const *__restrict format, va_list arg);
INTDEF ssize_t LIBCCALL libc_32fwscanf(FILE *__restrict stream, char32_t const *__restrict format, ...);
INTDEF ssize_t LIBCCALL libc_32vfwscanf(FILE *__restrict s, char32_t const *__restrict format, va_list arg);
INTDEF FILE *LIBCCALL libc_32open_wmemstream(char32_t **bufloc, size_t *sizeloc);
INTDEF wint_t LIBCCALL libc_32fgetwc_unlocked(FILE *stream);
INTDEF wint_t LIBCCALL libc_32fputwc_unlocked(char32_t wc, FILE *stream);
INTDEF int LIBCCALL libc_32fputws_unlocked(char32_t const *__restrict ws, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_32getwchar(void);
INTDEF wint_t LIBCCALL libc_32putwchar(char32_t wc);
INTDEF wint_t LIBCCALL libc_32getwchar_unlocked(void);
INTDEF wint_t LIBCCALL libc_32putwchar_unlocked(char32_t wc);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF FILE *LIBCCALL libc___iob_func(void);
INTDEF wint_t LIBCCALL libc_16single_ungetwch(wint_t __wc);
INTDEF wint_t LIBCCALL libc_16single_ungetwch_nolock(wint_t __wc);
INTDEF wint_t LIBCCALL libc_32single_ungetwch(wint_t __wc);
INTDEF wint_t LIBCCALL libc_32single_ungetwch_nolock(wint_t __wc);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_STDIO_FILE_H */
