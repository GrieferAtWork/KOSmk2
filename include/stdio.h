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
#ifndef _STDIO_H
#define _STDIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <bits/stdio_lim.h>
#if defined(__USE_DOS) || defined(__BUILDING_LIBC)
#include <bits/io-file.h>
#endif /* __USE_DOS || __BUILDING_LIBC */

__DECL_BEGIN

#ifdef __NAMESPACE_STD_EXISTS
#ifndef __std_size_t_defined
#define __std_size_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __SIZE_TYPE__ size_t;
__NAMESPACE_STD_END
#endif /* !__std_size_t_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __size_t_defined
#define __size_t_defined 1
__NAMESPACE_STD_USING(size_t)
#endif /* !__size_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#else /* __NAMESPACE_STD_EXISTS */
#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */
#endif /* !__NAMESPACE_STD_EXISTS */

#ifndef NULL
#ifdef __INTELLISENSE__
#   define NULL nullptr
#elif defined(__cplusplus) || defined(__LINKER__)
#   define NULL          0
#else
#   define NULL ((void *)0)
#endif
#endif

#ifdef __USE_XOPEN2K8
#ifndef __off_t_defined
#define __off_t_defined 1
typedef __typedef_off_t off_t;
#endif /* !__off_t_defined */

#ifndef __ssize_t_defined
#define __ssize_t_defined 1
typedef __ssize_t ssize_t;
#endif /* !__ssize_t_defined */

#ifdef __USE_LARGEFILE64
#ifndef __off64_t_defined
#define __off64_t_defined 1
typedef __off64_t off64_t;
#endif /* !__off64_t_defined */
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN2K8 */

#ifdef __NAMESPACE_STD_EXISTS
#ifndef __std_fpos_t_defined
#define __std_fpos_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __FS_TYPE(pos) fpos_t;
__NAMESPACE_STD_END
#endif /* !__std_fpos_t_defined */
#ifndef __fpos_t_defined
#define __fpos_t_defined 1
__NAMESPACE_STD_USING(fpos_t)
#endif /* !__fpos_t_defined */
#else
#ifndef __fpos_t_defined
#define __fpos_t_defined 1
typedef __FS_TYPE(pos) fpos_t;
#endif /* !__fpos_t_defined */
#endif

#ifdef __USE_LARGEFILE64
#ifndef __fpos64_t_defined
#define __fpos64_t_defined 1
typedef __pos64_t      fpos64_t;
#endif /* !__fpos64_t_defined */
#endif

/* Dos has different values for some of these.
 * Yet since they don't collide with each other, 'setvbuf()' accepts either. */
#define __DOS_IOFBF 0x0000 /*< Fully buffered. */
#define __DOS_IOLBF 0x0040 /*< Line buffered. */
#define __DOS_IONBF 0x0004 /*< No buffering. */

/* The possibilities for the third argument to 'setvbuf()'. */
#ifdef __USE_DOS
#define _IOFBF __DOS_IOFBF /*< Fully buffered. */
#define _IOLBF __DOS_IOLBF /*< Line buffered. */
#define _IONBF __DOS_IONBF /*< No buffering. */
#else
#define _IOFBF 0 /*< Fully buffered. */
#define _IOLBF 1 /*< Line buffered. */
#define _IONBF 2 /*< No buffering. */
#endif


/* Default buffer size.  */
#ifndef BUFSIZ
#ifdef __USE_DOS
#define BUFSIZ 512
#else
#define BUFSIZ 8192
#endif
#endif

#ifndef EOF
#define EOF (-1)
#endif

#define SEEK_SET  0 /*< Seek from beginning of file. */
#define SEEK_CUR  1 /*< Seek from current position. */
#define SEEK_END  2 /*< Seek from end of file. */
#ifdef __USE_GNU
#define SEEK_DATA 3 /*< Seek to next data. */
#define SEEK_HOLE 4 /*< Seek to next hole. */
#endif

#if defined(__USE_MISC) || defined(__USE_XOPEN)
#ifdef __USE_DOS
#define P_tmpdir "\\"
#else
#define P_tmpdir "/tmp"
#endif
#endif

#ifndef __KERNEL__
#ifdef __NAMESPACE_STD_EXISTS
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
#else /* __NAMESPACE_STD_EXISTS */
#ifndef __FILE_defined
#define __FILE_defined 1
typedef __FILE FILE;
#endif /* !__FILE_defined */
#endif /* !__NAMESPACE_STD_EXISTS */
#endif /* !__KERNEL__ */

#ifndef __KERNEL__

/* Standard streams. */
#ifndef __stdstreams_defined
#define __stdstreams_defined 1
#undef stdin
#undef stdout
#undef stderr
#ifdef __PE__
__LIBC FILE *(__LIBCCALL __iob_func)(void);
#define stdin  (__iob_func()+0)
#define stdout (__iob_func()+1)
#define stderr (__iob_func()+2)
#else /* __PE__ */
__LIBC __FILE *(stdin);
__LIBC __FILE *(stdout);
__LIBC __FILE *(stderr);
#define stdin   stdin
#define stdout  stdout
#define stderr  stderr
#endif /* !__PE__ */
#endif /* !__stdstreams_defined */

__NAMESPACE_STD_BEGIN
#ifndef __remove_defined
__LIBC __NONNULL((1)) int (__LIBCCALL remove)(char const *__file) __UFS_FUNC(remove);
#endif /* !__remove_defined */
#ifndef __rename_defined
__LIBC int (__LIBCCALL rename)(char const *__old, char const *__new) __UFS_FUNC(rename);
#endif /* !__rename_defined */
#ifdef __USE_KOS
__LIBC __WUNUSED char *(__LIBCCALL tmpnam)(char __buf[L_tmpnam]);
#else
__LIBC __WUNUSED char *(__LIBCCALL tmpnam)(char *__buf);
#endif
__LIBC int (__LIBCCALL fclose)(__FILE *__stream);
__LIBC int (__LIBCCALL fflush)(__FILE *__stream);
__LIBC void (__LIBCCALL setbuf)(__FILE *__restrict __stream, char *__restrict __buf);
__LIBC int (__LIBCCALL setvbuf)(__FILE *__restrict __stream, char *__restrict __buf, int __modes, size_t __n);
__LIBC int (__LIBCCALL fgetc)(__FILE *__stream);
__LIBC int (__LIBCCALL getc)(__FILE *__stream);
__LIBC int (__LIBCCALL getchar)(void);
__LIBC int (__LIBCCALL fputc)(int __ch, __FILE *__stream);
__LIBC int (__LIBCCALL putc)(int __ch, __FILE *__stream);
__LIBC int (__LIBCCALL putchar)(int __ch);

#ifdef __USE_KOS
#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgets_sz");
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */
__LIBC __ssize_t (__LIBCCALL fputs)(char const *__restrict __buf, __FILE *__restrict __stream);
__LIBC __ssize_t (__LIBCCALL puts)(char const *__restrict __str);
#else /* __USE_KOS */
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict __buf, int __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fputs)(char const *__restrict __str, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL puts)(char const *__restrict __str);
#endif /* !__USE_KOS */
__LIBC int (__LIBCCALL ungetc)(int __ch, __FILE *__restrict __stream);
__LIBC __WUNUSED size_t (__LIBCCALL fread)(void *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream);
__LIBC size_t (__LIBCCALL fwrite)(void const *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fseek)(__FILE *__restrict __stream, long int __off, int __whence);
__LIBC __WUNUSED long int (__LIBCCALL ftell)(__FILE *__restrict __stream);
__LIBC void (__LIBCCALL rewind)(__FILE *__restrict __stream);
__LIBC void (__LIBCCALL clearerr)(__FILE *__restrict __stream);
__LIBC __WUNUSED int (__LIBCCALL feof)(__FILE *__restrict __stream);
__LIBC __WUNUSED int (__LIBCCALL ferror)(__FILE *__restrict __stream);
#ifndef __perror_defined
__LIBC void (__LIBCCALL perror)(char const *__restrict __message);
#endif /* !__perror_defined */
__LIBC __WUNUSED __FILE *(__LIBCCALL tmpfile)(void) __FS_FUNC(tmpfile);
__LIBC __WUNUSED __FILE *(__LIBCCALL fopen)(char const *__restrict __filename, char const *__restrict __modes) __UFS_FUNCn(fopen);
__LIBC __WUNUSED __FILE *(__LIBCCALL freopen)(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream) __UFS_FUNCn(freopen);
__LIBC int (__LIBCCALL fgetpos)(__FILE *__restrict __stream, fpos_t *__restrict __pos) __FS_FUNC(fgetpos);
__LIBC int (__LIBCCALL fsetpos)(__FILE *__restrict __stream, fpos_t const *__restrict __pos) __FS_FUNC(fsetpos);
#ifdef __USE_KOS
__LIBC __ssize_t (__ATTR_CDECL printf)(char const *__restrict __format, ...);
__LIBC __ssize_t (__ATTR_CDECL fprintf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __ssize_t (__LIBCCALL vprintf)(char const *__restrict __format, __VA_LIST __args);
__LIBC __ssize_t (__LIBCCALL vfprintf)(__FILE *__restrict __stream, char const *__restrict __format, __VA_LIST __args);
__LIBC __WUNUSED __size_t (__ATTR_CDECL fscanf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __WUNUSED __size_t (__ATTR_CDECL scanf)(char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC __WUNUSED __size_t (__LIBCCALL vfscanf)(__FILE *__restrict __stream, char const *__restrict __format, __VA_LIST __args);
__LIBC __WUNUSED __size_t (__LIBCCALL vscanf)(char const *__restrict __format, __VA_LIST __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#else /* __USE_KOS */
__LIBC int (__ATTR_CDECL printf)(char const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL fprintf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vprintf)(char const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vfprintf)(__FILE *__restrict __stream, char const *__restrict __format, __VA_LIST __args);
__LIBC __WUNUSED int (__ATTR_CDECL fscanf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __WUNUSED int (__ATTR_CDECL scanf)(char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC __WUNUSED int (__LIBCCALL vfscanf)(__FILE *__restrict __stream, char const *__restrict __format, __VA_LIST __args);
__LIBC __WUNUSED int (__LIBCCALL vscanf)(char const *__restrict __format, __VA_LIST __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#endif /* !__USE_KOS */
#if !defined(__USE_ISOC11) || \
    (defined(__cplusplus) && __cplusplus <= 201103L)
__LIBC __WUNUSED __ATTR_DEPRECATED("No buffer size checks") char *(__LIBCCALL gets)(char *__restrict __buf);
#endif
__NAMESPACE_STD_END

#ifndef __remove_defined
#define __remove_defined 1
__NAMESPACE_STD_USING(remove)
#endif /* !__remove_defined */
#ifndef __rename_defined
#define __rename_defined 1
__NAMESPACE_STD_USING(rename)
#endif /* !__rename_defined */
__NAMESPACE_STD_USING(tmpnam)
__NAMESPACE_STD_USING(fclose)
__NAMESPACE_STD_USING(fflush)
__NAMESPACE_STD_USING(setbuf)
__NAMESPACE_STD_USING(setvbuf)
__NAMESPACE_STD_USING(fgetc)
__NAMESPACE_STD_USING(getc)
__NAMESPACE_STD_USING(getchar)
__NAMESPACE_STD_USING(fputc)
__NAMESPACE_STD_USING(putc)
__NAMESPACE_STD_USING(putchar)
__NAMESPACE_STD_USING(fgets)
__NAMESPACE_STD_USING(fputs)
__NAMESPACE_STD_USING(puts)
__NAMESPACE_STD_USING(ungetc)
__NAMESPACE_STD_USING(fread)
__NAMESPACE_STD_USING(fwrite)
__NAMESPACE_STD_USING(fseek)
__NAMESPACE_STD_USING(ftell)
__NAMESPACE_STD_USING(rewind)
__NAMESPACE_STD_USING(clearerr)
__NAMESPACE_STD_USING(feof)
__NAMESPACE_STD_USING(ferror)
#ifndef __perror_defined
#define __perror_defined 1
__NAMESPACE_STD_USING(perror)
#endif /* !__perror_defined */
__NAMESPACE_STD_USING(tmpfile)
__NAMESPACE_STD_USING(fopen)
__NAMESPACE_STD_USING(freopen)
__NAMESPACE_STD_USING(fgetpos)
__NAMESPACE_STD_USING(fsetpos)
__NAMESPACE_STD_USING(printf)
__NAMESPACE_STD_USING(fprintf)
__NAMESPACE_STD_USING(vprintf)
__NAMESPACE_STD_USING(vfprintf)
__NAMESPACE_STD_USING(fscanf)
__NAMESPACE_STD_USING(scanf)
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__NAMESPACE_STD_USING(vfscanf)
__NAMESPACE_STD_USING(vscanf)
#endif /* __USE_ISOC99 || __USE_DOS */
#if !defined(__USE_ISOC11) || \
    (defined(__cplusplus) && __cplusplus <= 201103L)
__NAMESPACE_STD_USING(gets)
#endif

#ifdef __USE_KOS
/* Reopen the given file stream using a provided file descriptor.
 * @param: MODE: Set of 'FDREOPEN_*'
 * @return: STREAM: Successfully re-opened the given file.
 * @return: NULL:   Failed to re-open the file (see 'errno' for details) */
__LIBC __WUNUSED __FILE *(__LIBCCALL fdreopen)(int __fd, char const *__restrict __modes,
                                               __FILE *__restrict __stream, int __mode);
#define FDREOPEN_DUP            0x0 /*< Duplicate the given descriptor, creating a private copy for the stream. */
#define FDREOPEN_INHERIT        0x1 /*< Inherit the given 'fd' on success, using that same number for the stream. */
#define FDREOPEN_CLOSE_ON_ERROR 0x2 /*< Close 'FD' if an error occurred during the attempt at re-opening it. */
#endif /* __USE_KOS */

#ifdef __USE_XOPEN2K8
#ifdef __USE_KOS
__LIBC __ssize_t (__LIBCCALL vdprintf)(int __fd, char const *__restrict __format, __VA_LIST __args);
__LIBC __ssize_t (__ATTR_CDECL dprintf)(int __fd, char const *__restrict __format, ...);
#else /* __USE_KOS */
__LIBC int (__LIBCCALL vdprintf)(int __fd, char const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL dprintf)(int __fd, char const *__restrict __format, ...);
#endif /* !__USE_KOS */
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_KOS
__LIBC int (__LIBCCALL removeat)(int __fd, char const *__filename) __UFS_FUNC(removeat);
#endif /* __USE_KOS */
#ifdef __USE_ATFILE
__LIBC int (__LIBCCALL renameat)(int __oldfd, char const *__old, int __newfd, char const *__new) __UFS_FUNC(renameat);
#ifdef __USE_KOS
__LIBC int (__LIBCCALL frenameat)(int __oldfd, char const *__old, int __newfd, char const *__new, int __flags) __UFS_FUNC(frenameat);
#endif /* __USE_KOS */
#endif /* __USE_ATFILE */
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED __FILE *(__LIBCCALL tmpfile64)(void);
__LIBC __WUNUSED __FILE *(__LIBCCALL fopen64)(char const *__restrict __filename, char const *__restrict __modes) __UFS_FUNC(fopen64);
__LIBC __WUNUSED __FILE *(__LIBCCALL freopen64)(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream) __UFS_FUNC(freopen64);
__LIBC int (__LIBCCALL fseeko64)(__FILE *__stream, __off64_t __off, int __whence) __PE_ASMNAME("_fseeki64");
__LIBC __WUNUSED __off64_t (__LIBCCALL ftello64)(__FILE *__stream) __PE_ASMNAME("_ftelli64");
__LIBC int (__LIBCCALL fgetpos64)(__FILE *__restrict __stream, fpos64_t *__restrict __pos);
__LIBC int (__LIBCCALL fsetpos64)(__FILE *__stream, fpos64_t const *__pos);
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_MISC
__LIBC __WUNUSED char *(__LIBCCALL tmpnam_r)(char *__buf);
__LIBC int (__LIBCCALL fflush_unlocked)(__FILE *__stream) __PE_ASMNAME("_fflush_nolock");
__LIBC void (__LIBCCALL setbuffer)(__FILE *__restrict __stream, char *__restrict __buf, size_t __size);
__LIBC void (__LIBCCALL setlinebuf)(__FILE *__stream);
__LIBC int (__LIBCCALL fgetc_unlocked)(__FILE *__stream);
__LIBC int (__LIBCCALL fputc_unlocked)(int __ch, __FILE *__stream);
__LIBC size_t (__LIBCCALL fread_unlocked)(void *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream) __PE_ASMNAME("_fread_nolock");
__LIBC size_t (__LIBCCALL fwrite_unlocked)(void const *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream) __PE_ASMNAME("_fwrite_nolock");
__LIBC void (__LIBCCALL clearerr_unlocked)(__FILE *__stream);
__LIBC __WUNUSED int (__LIBCCALL feof_unlocked)(__FILE *__stream);
__LIBC __WUNUSED int (__LIBCCALL ferror_unlocked)(__FILE *__stream);
__LIBC __WUNUSED int (__LIBCCALL fileno_unlocked)(__FILE *__stream) __ASMNAME2("fileno","_fileno");
#endif /* __USE_MISC */
#if defined(__USE_MISC) || defined(__USE_XOPEN) || defined(__USE_DOS)
__LIBC __ATTR_MALLOC __WUNUSED char *(__LIBCCALL tempnam)(char const *__dir, char const *__pfx) __PE_ASMNAME("_tempnam");
#endif /* __USE_MISC || __USE_XOPEN || __USE_DOS */
#if defined(__USE_POSIX) || defined(__USE_DOS)
__LIBC __WUNUSED __FILE *(__LIBCCALL fdopen)(int __fd, char const *__restrict __modes) __PE_ASMNAME("_fdopen");
__LIBC __WUNUSED int (__LIBCCALL fileno)(__FILE *__stream) __PE_ASMNAME("_fileno");
#endif /* __USE_POSIX || __USE_DOS */
#ifdef __USE_XOPEN2K8
__LIBC __WUNUSED __FILE *(__LIBCCALL fmemopen)(void *__mem, size_t __len, char const *__modes);
__LIBC __WUNUSED __FILE *(__LIBCCALL open_memstream)(char **__bufloc, size_t *__sizeloc);
__LIBC __WUNUSED __ssize_t (__LIBCCALL __getdelim)(char **__restrict __lineptr, size_t *__restrict __n, int __delimiter, __FILE *__restrict __stream) __ASMNAME("getdelim");
__LIBC __WUNUSED __ssize_t (__LIBCCALL getdelim)(char **__restrict __lineptr, size_t *__restrict __n, int __delimiter, __FILE *__restrict __stream);
__LIBC __WUNUSED __ssize_t (__LIBCCALL getline)(char **__restrict __lineptr, size_t *__restrict __n, __FILE *__restrict __stream);
#endif /* __USE_XOPEN2K8 */
#ifdef __USE_POSIX
__LIBC int (__LIBCCALL getc_unlocked)(__FILE *__stream);
__LIBC int (__LIBCCALL getchar_unlocked)(void);
__LIBC int (__LIBCCALL putc_unlocked)(int __ch, __FILE *__stream);
__LIBC int (__LIBCCALL putchar_unlocked)(int __ch);
#ifndef __ctermid_defined
#define __ctermid_defined 1
__LIBC char *(__LIBCCALL ctermid)(char *__buf);
#endif /* !__ctermid_defined */
__LIBC __WUNUSED int (__LIBCCALL ftrylockfile)(__FILE *__stream);
__LIBC void (__LIBCCALL flockfile)(__FILE *__stream) __PE_ASMNAME("_lock_file");
__LIBC void (__LIBCCALL funlockfile)(__FILE *__stream) __PE_ASMNAME("_unlock_file");
#endif /* __USE_POSIX */
#ifdef __USE_POSIX2
__LIBC __WUNUSED __FILE *(__LIBCCALL popen)(char const *__command, char const *__modes) __PE_ASMNAME("_popen");
__LIBC int (__LIBCCALL pclose)(__FILE *__stream) __PE_ASMNAME("_pclose");
#endif /* __USE_POSIX2 */
#if defined(__USE_MISC) || defined(__USE_DOS) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__LIBC int (__LIBCCALL getw)(__FILE *__stream) __PE_ASMNAME("_getw");
__LIBC int (__LIBCCALL putw)(int __w, __FILE *__stream) __PE_ASMNAME("_putw");
#endif
#if defined(__USE_GNU) || defined(__USE_DOS)
__LIBC int (__LIBCCALL fcloseall)(void) __PE_ASMNAME("_fcloseall");
#endif /* __USE_GNU || __USE_DOS */
#ifdef __USE_GNU
//__LIBC __WUNUSED __FILE *(__LIBCCALL fopencookie)(void *__restrict __magic_cookie, char const *__restrict __modes, _IO_cookie_io_functions_t __io_funcs);
#ifdef __USE_KOS
#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
__LIBC __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else
__LIBC __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgets_unlocked_sz");
#endif
__LIBC __ssize_t (__LIBCCALL fputs_unlocked)(char const *__restrict __str, __FILE *__restrict __stream);
#else /* __USE_KOS */
__LIBC __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict __str, int __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fputs_unlocked)(char const *__restrict __str, __FILE *__restrict __stream);
#endif /* !__USE_KOS */

struct obstack;
__LIBC int (__LIBCCALL obstack_printf)(struct obstack *__restrict __obstack, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL obstack_vprintf)(struct obstack *__restrict __obstack, char const *__restrict __format, __VA_LIST __args);
#endif /* __USE_GNU */
#if defined(__USE_LARGEFILE) || defined(__USE_XOPEN2K)
#ifdef __PE__
#ifdef __USE_FILE_OFFSET64
__LIBC int (__LIBCCALL fseeko)(__FILE *__stream, __FS_TYPE(off) __off, int __whence) __ASMNAME("_fseeki64");
__LIBC __WUNUSED __FS_TYPE(off) (__LIBCCALL ftello)(__FILE *__stream) __ASMNAME("_ftelli64");
#else /* __USE_FILE_OFFSET64 */
#if __SIZEOF_OFF_T__ == __SIZEOF_LONG__
__LIBC int (__LIBCCALL fseeko)(__FILE *__stream, __FS_TYPE(off) __off, int __whence) __ASMNAME("fseek");
__LIBC __WUNUSED __FS_TYPE(off) (__LIBCCALL ftello)(__FILE *__stream) __ASMNAME("ftell");
#else /* __SIZEOF_OFF_T__ == __SIZEOF_LONG__ */
__LIBC int (__LIBCCALL fseeko)(__FILE *__stream, __FS_TYPE(off) __off, int __whence);
__LIBC __WUNUSED __FS_TYPE(off) (__LIBCCALL ftello)(__FILE *__stream);
#endif /* __SIZEOF_OFF_T__ != __SIZEOF_LONG__ */
#endif /* !__USE_FILE_OFFSET64 */
#else /* __PE__ */
__LIBC int (__LIBCCALL fseeko)(__FILE *__stream, __FS_TYPE(off) __off, int __whence) __FS_FUNC(fseeko);
__LIBC __WUNUSED __FS_TYPE(off) (__LIBCCALL ftello)(__FILE *__stream) __FS_FUNC(ftello);
#endif /* !__PE__ */
#endif /* __USE_LARGEFILE || __USE_XOPEN2K */
#ifdef __USE_XOPEN
__LIBC char *(__LIBCCALL cuserid)(char *__buf);
#endif /* __USE_XOPEN */
#ifdef __USE_KOS
/* For use with 'format_printf()' and friends: Prints to a 'FILE *' closure argument. */
__LIBC __ssize_t (__LIBCCALL file_printer)(char const *__restrict __data,
                                           size_t __datalen, void *__closure);
#endif /* __USE_KOS */
#endif /* !__KERNEL__ */


__NAMESPACE_STD_BEGIN
#ifdef __USE_KOS
__LIBC __ssize_t (__ATTR_CDECL sprintf)(char *__restrict __buf, char const *__restrict __format, ...);
__LIBC __ssize_t (__LIBCCALL vsprintf)(char *__restrict __buf, char const *__restrict __format, __VA_LIST __args);
__LIBC __size_t (__ATTR_CDECL sscanf)(char const *__restrict __buf, char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
__LIBC __ssize_t (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) __PE_ASMNAME("_snprintf_c");
#endif /* __USE_ISOC99 || __USE_UNIX98 */
#if defined(__USE_ISOC99) || defined(__USE_UNIX98) || defined(__USE_DOS)
__LIBC __ssize_t (__LIBCCALL vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __VA_LIST __args) __PE_ASMNAME("_vsnprintf_c");
#endif /* __USE_ISOC99 || __USE_UNIX98 || __USE_DOS */
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC __size_t (__LIBCCALL vsscanf)(char const *__restrict __buf, char const *__restrict __format, __VA_LIST __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#else /* __USE_KOS */
__LIBC int (__ATTR_CDECL sprintf)(char *__restrict __buf, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vsprintf)(char *__restrict __buf, char const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL sscanf)(char const *__restrict __buf, char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
__LIBC int (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) __PE_ASMNAME("_snprintf_c");
__LIBC int (__LIBCCALL vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __VA_LIST __args) __PE_ASMNAME("_vsnprintf_c");
#endif /* __USE_ISOC99 || __USE_UNIX98 */
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC int (__LIBCCALL vsscanf)(char const *__restrict __buf, char const *__restrict __format, __VA_LIST __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#endif /* !__USE_KOS */
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(sprintf)
__NAMESPACE_STD_USING(vsprintf)
__NAMESPACE_STD_USING(sscanf)
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
__NAMESPACE_STD_USING(snprintf)
#endif /* __USE_ISOC99 || __USE_UNIX98 */
#if defined(__USE_ISOC99) || defined(__USE_UNIX98) || defined(__USE_DOS)
__NAMESPACE_STD_USING(vsnprintf)
#endif /* __USE_ISOC99 || __USE_UNIX98 || __USE_DOS */
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__NAMESPACE_STD_USING(vsscanf)
#endif /* __USE_ISOC99 || __USE_DOS */


#ifdef __USE_GNU
#ifndef __KERNEL__
#ifdef __USE_KOS
__LIBC __WUNUSED __ssize_t (__ATTR_CDECL asprintf)(char **__restrict __pstr, char const *__restrict __format, ...);
__LIBC __WUNUSED __ssize_t (__ATTR_CDECL __asprintf)(char **__restrict __pstr, char const *__restrict __format, ...) __ASMNAME("asprintf");
__LIBC __WUNUSED __ssize_t (__LIBCCALL vasprintf)(char **__restrict __pstr, char const *__restrict __format, __VA_LIST __args);
#else /* __USE_KOS */
__LIBC __WUNUSED int (__ATTR_CDECL asprintf)(char **__restrict __pstr, char const *__restrict __format, ...);
__LIBC __WUNUSED int (__ATTR_CDECL __asprintf)(char **__restrict __pstr, char const *__restrict __format, ...) __ASMNAME("asprintf");
__LIBC __WUNUSED int (__LIBCCALL vasprintf)(char **__restrict __pstr, char const *__restrict __format, __VA_LIST __args);
#endif /* !__USE_KOS */
#endif /* !__KERNEL__ */
#endif /* __USE_GNU */

#if defined(__USE_XOPEN) && !defined(__USE_XOPEN2K) && !defined(__USE_GNU)
#include <getopt.h>
#endif

/* DOS Extensions */
#ifdef __USE_DOS
#ifndef _iobuf
#define _iobuf   _IO_FILE
#endif /* !_iobuf */

#define _NFILE        512
#define _NSTREAM_     512
#define _IOB_ENTRIES  20
#define _P_tmpdir     "\\"
#define _wP_tmpdir   L"\\"
#define _SYS_OPEN     20
#ifdef __USE_DOS_SLIB
#define L_tmpnam_s    18
#define TMP_MAX_S     2147483647
#define _TMP_MAX_S    2147483647
#endif /* __USE_DOS_SLIB */

#ifndef _FPOS_T_DEFINED
#define _FPOS_T_DEFINED 1
#endif /* !_FPOS_T_DEFINED */
#ifndef _STDSTREAM_DEFINED
#define _STDSTREAM_DEFINED 1
#endif /* !_STDSTREAM_DEFINED */
#ifndef _FILE_DEFINED
#define _FILE_DEFINED 1
#endif /* !_FILE_DEFINED */

#define _IOREAD  __IO_FILE_IOR
#define _IOWRT   __IO_FILE_IOW
#define _IOMYBUF __IO_FILE_IOMALLBUF
#define _IOEOF   __IO_FILE_IOEOF
#define _IOERR   __IO_FILE_IOERR
#define _IOSTRG  __IO_FILE_IONOFD
#define _IORW    __IO_FILE_IORW

#define _TWO_DIGIT_EXPONENT 0x1

#ifndef _CRT_PERROR_DEFINED
#define _CRT_PERROR_DEFINED 1
#endif  /* _CRT_PERROR_DEFINED */

#ifndef _CRT_DIRECTORY_DEFINED
#define _CRT_DIRECTORY_DEFINED 1
#ifndef __remove_defined
#define __remove_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __NONNULL((1)) int (__LIBCCALL remove)(char const *__file) __UFS_FUNC(remove);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(remove)
#endif /* !__remove_defined */
#ifndef __rename_defined
#define __rename_defined 1
__NAMESPACE_STD_BEGIN
__LIBC int (__LIBCCALL rename)(char const *__old, char const *__new) __UFS_FUNC(rename);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(rename)
#endif /* !__rename_defined */
#ifndef ___unlink_defined
#define ___unlink_defined 1
__LIBC int (__LIBCCALL _unlink)(char const *__file) __UFS_FUNC_OLDPEB(unlink);
#endif /* !___unlink_defined */
#ifndef __unlink_defined
#define __unlink_defined 1
__LIBC int (__LIBCCALL unlink)(char const *__file) __UFS_FUNC_OLDPEA(unlink);
#endif /* !__unlink_defined */
#endif  /* _CRT_DIRECTORY_DEFINED */

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __rsize_t_defined
#define __rsize_t_defined 1
typedef size_t rsize_t;
#endif /* !__rsize_t_defined */

#ifndef _STDIO_DEFINED
#define _STDIO_DEFINED 1
__LIBC __FILE *(__LIBCCALL _popen)(char const *__command, char const *__mode) __KOS_ASMNAME("popen");
__LIBC int (__LIBCCALL _pclose)(__FILE *__restrict __file) __KOS_ASMNAME("pclose");
__LIBC __FILE *(__LIBCCALL _fsopen)(char const *__file, char const *__mode, int __shflag) __KOS_ASMNAME("fopen"); /* XXX: Assume caller-argument-cleanup for 'LIBCCALL'. */
__LIBC __FILE *(__LIBCCALL _fdopen)(int __fd, char const *__restrict __mode) __KOS_ASMNAME("fdopen");
__LIBC int (__LIBCCALL _flushall)(void);
__LIBC int (__LIBCCALL _fcloseall)(void) __KOS_ASMNAME("fcloseall");
__LIBC int (__LIBCCALL _fileno)(__FILE *__restrict __file) __KOS_ASMNAME("fileno");
__LIBC int (__LIBCCALL _rmtmp)(void);
__LIBC int (__LIBCCALL _filbuf)(__FILE *__restrict __file);
__LIBC int (__LIBCCALL _flsbuf)(int __ch, __FILE *__restrict __file);
__LIBC int (__LIBCCALL _fgetchar)(void) __ASMNAME("getchar");
__LIBC int (__LIBCCALL _fputchar)(int __ch) __ASMNAME("putchar");
__LIBC int (__LIBCCALL _getw)(__FILE *__restrict __file) __KOS_ASMNAME("getw");
__LIBC int (__LIBCCALL _putw)(int __w, __FILE *__restrict __file) __KOS_ASMNAME("putw");
__LIBC char *(__LIBCCALL _tempnam)(char const *__dir, char const *__pfx) __KOS_ASMNAME("tempnam");
__LIBC int (__LIBCCALL _fseeki64)(__FILE *__restrict __file, __INT64_TYPE__ __off, int __whence) __KOS_ASMNAME("fseeko64");
__LIBC __INT64_TYPE__ (__LIBCCALL _ftelli64)(__FILE *__restrict __file) __KOS_ASMNAME("ftello64");

__LIBC int (__LIBCCALL _getmaxstdio)(void);
__LIBC int (__LIBCCALL _setmaxstdio)(int __val);
__LIBC int (__LIBCCALL _set_printf_count_output)(int __val);
__LIBC int (__LIBCCALL _get_printf_count_output)(void);
__LIBC __UINT32_TYPE__ (__LIBCCALL _set_output_format)(__UINT32_TYPE__ __format);
__LIBC __UINT32_TYPE__ (__LIBCCALL _get_output_format)(void);

__LIBC int (__ATTR_CDECL _fscanf_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _fscanf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_fscanf_l");
__LIBC int (__ATTR_CDECL _scanf_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _scanf_s_l)(char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_scanf_l");
__LIBC int (__ATTR_CDECL _sscanf_l)(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _sscanf_s_l)(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_sscanf_l");
__LIBC int (__ATTR_CDECL _snscanf)(char const *__restrict __src, size_t __buflen, char const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _snscanf_s)(char const *__restrict __src, size_t __buflen, char const *__restrict __format, ...) __ASMNAME("_snscanf");
__LIBC int (__ATTR_CDECL _snscanf_l)(char const *__restrict __src, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _snscanf_s_l)(char const *__restrict __src, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_snscanf_l");

__LIBC int (__ATTR_CDECL _sprintf_l)(char *__restrict __buf, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsprintf_l)(char *__restrict __buf, char const *__restrict __format, __locale_t, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _sprintf_p)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vsprintf_p)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _sprintf_p_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsprintf_p_l)(char *__restrict __buf, size_t __buflen, char const*__fmt, __locale_t __locale,  __VA_LIST __args);
__LIBC int (__ATTR_CDECL _sprintf_s_l)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsprintf_s_l)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __locale_t __locale, __VA_LIST __args);

__LIBC int (__ATTR_CDECL _scprintf)(char const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vscprintf)(char const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _scprintf_p)(char const *__restrict __format, ...) __ASMNAME("_scprintf");
__LIBC int (__LIBCCALL _vscprintf_p)(char const *__restrict __format, __VA_LIST __args) __ASMNAME("_vscprintf");
__LIBC int (__ATTR_CDECL _scprintf_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vscprintf_l)(char const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _scprintf_p_l)(char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_scprintf_l");
__LIBC int (__LIBCCALL _vscprintf_p_l)(char const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME("_vscprintf_l");

/* The following 2 return an error, rather than the required size when the buffer is too small */
__LIBC int (__ATTR_CDECL _snprintf)(char *__restrict __buf, size_t __buflen, const char *__restrict __format, ...);
__LIBC int (__LIBCCALL _vsnprintf)(char *__restrict __buf, size_t __buflen, const char *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _snprintf_c)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) __KOS_ASMNAME("snprintf");
__LIBC int (__LIBCCALL _vsnprintf_c)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __VA_LIST __args) __KOS_ASMNAME("vsnprintf");
__LIBC int (__ATTR_CDECL _snprintf_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsnprintf_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _snprintf_c_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsnprintf_c_l)(char *__restrict __buf, size_t __buflen, char const *, __locale_t __locale, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _snprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vsnprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _snprintf_s_l)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsnprintf_s_l)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const*__fmt, __locale_t __locale, __VA_LIST __args);

__LIBC int (__ATTR_CDECL _printf_p)(char const *__restrict __format, ...) __ASMNAME("printf");
__LIBC int (__LIBCCALL _vprintf_p)(char const *__restrict __format, __VA_LIST __args) __ASMNAME("vprintf");
__LIBC int (__ATTR_CDECL _printf_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vprintf_l)(char const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _printf_p_l)(char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_printf_l");
__LIBC int (__LIBCCALL _vprintf_p_l)(char const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME("_vprintf_l");
__LIBC int (__ATTR_CDECL _printf_s_l)(char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_printf_l");
__LIBC int (__LIBCCALL _vprintf_s_l)(char const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME("_vprintf_l");

__LIBC int (__ATTR_CDECL _fprintf_p)(__FILE *__restrict __file, char const *__restrict __format, ...) __ASMNAME("fprintf");
__LIBC int (__LIBCCALL _vfprintf_p)(__FILE *__restrict __file, char const *__restrict __format, __VA_LIST __args) __ASMNAME("vfprintf");
__LIBC int (__ATTR_CDECL _fprintf_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vfprintf_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _fprintf_p_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_fprintf_l");
__LIBC int (__LIBCCALL _vfprintf_p_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME("_vfprintf_l");
__LIBC int (__ATTR_CDECL _fprintf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_fprintf_l");
__LIBC int (__LIBCCALL _vfprintf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME("_vfprintf_l");

#ifdef __USE_DOS_SLIB
/* WARNING: 'fopen_s' and 'freopen_s' returns DOS error codes in DOS-FS mode! */
__LIBC errno_t (__LIBCCALL fopen_s)(__FILE **__pfile, char const *__file, char const *__mode) __UFS_FUNC(fopen_s);
__LIBC errno_t (__LIBCCALL freopen_s)(__FILE **__pfile, char const *__file, char const *__mode, __FILE *__oldfile) __UFS_FUNC(freopen_s);
__LIBC errno_t (__LIBCCALL clearerr_s)(__FILE *__restrict __file);
__LIBC size_t (__LIBCCALL fread_s)(void *__buf, size_t __bufsize, size_t __elemsize, size_t __elemcount, __FILE *__restrict __file);
__LIBC char *(__LIBCCALL gets_s)(char *__restrict __buf, rsize_t __bufsize);
__LIBC errno_t (__LIBCCALL tmpfile_s)(__FILE **__pfile);
__LIBC errno_t (__LIBCCALL tmpnam_s)(char *__restrict __buf, rsize_t __bufsize);

__LIBC int (__ATTR_CDECL printf_s)(char const *__restrict __format, ...) __ASMNAME("printf");
__LIBC int (__LIBCCALL vprintf_s)(char const *__restrict __format, __VA_LIST __args) __ASMNAME("vprintf");
__LIBC int (__ATTR_CDECL fprintf_s)(__FILE *__restrict __file, char const *__restrict __format, ...) __ASMNAME("fprintf");
__LIBC int (__LIBCCALL vfprintf_s)(__FILE *__restrict __file, char const *__restrict __format, __VA_LIST __args) __ASMNAME("vfprintf");
__LIBC int (__ATTR_CDECL sprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, ...) __ASMNAME("snprintf");
__LIBC int (__LIBCCALL vsprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vsnprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __VA_LIST __args);

__LIBC int (__ATTR_CDECL fscanf_s)(__FILE *__restrict __file, char const *__restrict __format, ...) __ASMNAME("fscanf");
__LIBC int (__LIBCCALL vfscanf_s)(__FILE *__restrict __file, char const *__restrict __format, __VA_LIST __args) __ASMNAME("vfscanf");
__LIBC int (__ATTR_CDECL scanf_s)(char const *__restrict __format, ...) __ASMNAME("scanf");
__LIBC int (__LIBCCALL vscanf_s)(char const *__restrict __format, __VA_LIST __args) __ASMNAME("vscanf");
__LIBC int (__ATTR_CDECL sscanf_s)(char const *__restrict __src, char const *__restrict __format, ...) __ASMNAME("sscanf");
__LIBC int (__LIBCCALL vsscanf_s)(char const *__restrict __src, char const *__restrict __format, __VA_LIST __args) __ASMNAME("vsscanf");
#endif /* __USE_DOS_SLIB */

#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED 1
__LIBC void (__LIBCCALL _wperror)(wchar_t const *__restrict __errmsg) __KOS_ASMNAME("wperror");
#endif /* !_CRT_WPERROR_DEFINED */

#ifndef _WSTDIO_DEFINED
#define _WSTDIO_DEFINED 1

#ifndef WEOF
#if __SIZEOF_WCHAR_T__ == 4
#   define WEOF             0xffffffffu
#else
#   define WEOF    (wint_t)(0xffff)
#endif
#endif /* !WEOF */

#ifndef __wprintf_defined
#define __wprintf_defined 1
__NAMESPACE_STD_BEGIN
#ifdef __USE_KOS
__LIBC __ssize_t (__ATTR_CDECL fwprintf)(FILE *__restrict __file, wchar_t const *__restrict __format, ...);
__LIBC __ssize_t (__LIBCCALL vfwprintf)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC __ssize_t (__ATTR_CDECL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC __ssize_t (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC __ssize_t (__ATTR_CDECL fwscanf)(FILE *__restrict __file, wchar_t const *__restrict __format, ...);
__LIBC __ssize_t (__LIBCCALL vfwscanf)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC __ssize_t (__ATTR_CDECL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC __ssize_t (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC __ssize_t (__ATTR_CDECL swscanf)(wchar_t const *__src, wchar_t const *__restrict __format, ...);
__LIBC __ssize_t (__LIBCCALL vswscanf)(wchar_t const *__src, wchar_t const *__restrict __format, __VA_LIST __args);
#if !defined(__PE__) || !defined(__NO_ASMNAME)
__LIBC __ssize_t (__LIBCCALL vswprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args)  __PE_ASMNAME("_vswprintf_c");
__LIBC __ssize_t (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __PE_ASMNAME("_swprintf_c");
#else /* !__NO_ASMNAME */
#ifndef ___vswprintf_c_defined
#define ___vswprintf_c_defined 1
__LIBC __ssize_t (__LIBCCALL _vswprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args);
#endif /* !___vswprintf_c_defined */
__LOCAL __ssize_t (__LIBCCALL vswprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) { return _vswprintf_c(__buf,__buflen,__format,__args); }
__LOCAL __ssize_t (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { __VA_LIST __args; int __result; __builtin_va_start(__args,__format); __result = vswprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* __NO_ASMNAME */
#else /* __USE_KOS */
__LIBC int (__ATTR_CDECL fwprintf)(FILE *__restrict __file, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vfwprintf)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL fwscanf)(FILE *__restrict __file, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vfwscanf)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL swscanf)(wchar_t const *__src, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vswscanf)(wchar_t const *__src, wchar_t const *__restrict __format, __VA_LIST __args);
#if !defined(__PE__) || !defined(__NO_ASMNAME)
__LIBC int (__LIBCCALL vswprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args)  __PE_ASMNAME("_vswprintf_c");
__LIBC int (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __PE_ASMNAME("_swprintf_c");
#else /* !__NO_ASMNAME */
#ifndef ___vswprintf_c_defined
#define ___vswprintf_c_defined 1
__LIBC int (__LIBCCALL _vswprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args);
#endif /* !___vswprintf_c_defined */
__LOCAL int (__LIBCCALL vswprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) { return _vswprintf_c(__buf,__buflen,__format,__args); }
__LOCAL int (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { __VA_LIST __args; int __result; __builtin_va_start(__args,__format); __result = vswprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* __NO_ASMNAME */
#endif /* !__USE_KOS */
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(fwprintf)
__NAMESPACE_STD_USING(vfwprintf)
__NAMESPACE_STD_USING(wprintf)
__NAMESPACE_STD_USING(vwprintf)
__NAMESPACE_STD_USING(fwscanf)
__NAMESPACE_STD_USING(vfwscanf)
__NAMESPACE_STD_USING(wscanf)
__NAMESPACE_STD_USING(vwscanf)
__NAMESPACE_STD_USING(swscanf)
__NAMESPACE_STD_USING(vswscanf)
__NAMESPACE_STD_USING(vswprintf)
__NAMESPACE_STD_USING(swprintf)
#endif /* !__wprintf_defined */

__LIBC FILE *(__LIBCCALL _wfsopen)(wchar_t const *__file, wchar_t const *__mode, int __shflag) __WFS_FUNC(_wfsopen);
__LIBC FILE *(__LIBCCALL _wfopen)(wchar_t const *__file, wchar_t const *__mode) __WFS_FUNC(_wfopen);
__LIBC FILE *(__LIBCCALL _wfreopen)(wchar_t const *__file, wchar_t const *__mode, FILE *__oldfile) __WFS_FUNC(_wfreopen);
__LIBC errno_t (__LIBCCALL _wfopen_s)(FILE **__pfile, wchar_t const *__file, wchar_t const *__mode) __WFS_FUNC(_wfopen_s);
__LIBC errno_t (__LIBCCALL _wfreopen_s)(FILE **__pfile, wchar_t const *__file, wchar_t const *__mode, FILE *__oldfile) __WFS_FUNC(_wfreopen_s);
__LIBC FILE *(__LIBCCALL _wfdopen)(int __fd, wchar_t const *__mode) __KOS_ASMNAME("wfdopen");
__LIBC FILE *(__LIBCCALL _wpopen)(wchar_t const *__command, wchar_t const *__mode) __KOS_ASMNAME("wpopen");

/* Get wide character functions */
__LIBC wint_t (__LIBCCALL _fgetwchar)(void) __ASMNAME("getwchar");
__LIBC wint_t (__LIBCCALL _fgetwc_nolock)(FILE *__restrict __file) __KOS_ASMNAME("fgetwc_unlocked");
#ifndef __getwchar_defined
#define __getwchar_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wint_t (__LIBCCALL getwchar)(void);
__LIBC wint_t (__LIBCCALL fgetwc)(FILE *__restrict __file);
__LIBC wint_t (__LIBCCALL getwc)(FILE *__restrict __file);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(getwchar)
__NAMESPACE_STD_USING(fgetwc)
__NAMESPACE_STD_USING(getwc)
#endif /* !__getwchar_defined */

/* Put wide character functions */
__LIBC wint_t (__LIBCCALL _fputwchar)(wchar_t __ch) __ASMNAME("putwchar");
__LIBC wint_t (__LIBCCALL _fputwc_nolock)(wchar_t __ch, FILE *__restrict __file) __KOS_ASMNAME("fputwc_unlocked");
#ifndef __putwchar_defined
#define __putwchar_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wint_t (__LIBCCALL putwchar)(wchar_t __ch);
__LIBC wint_t (__LIBCCALL fputwc)(wchar_t __ch, FILE *__restrict __file);
__LIBC wint_t (__LIBCCALL putwc)(wchar_t __ch, FILE *__restrict __file);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(putwchar)
__NAMESPACE_STD_USING(fputwc)
__NAMESPACE_STD_USING(putwc)
#endif /* !__putwchar_defined */

/* Unget character functions */
__LIBC wint_t (__LIBCCALL _ungetwc_nolock)(wint_t __ch, FILE *__restrict __file) __KOS_ASMNAME("ungetwc_unlocked");
#ifndef __ungetwc_defined
#define __ungetwc_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wint_t (__LIBCCALL ungetwc)(wint_t __ch, FILE *__restrict __file);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(ungetwc)
#endif /* !__ungetwc_defined */

/* Get wide string functions */
__LIBC wchar_t *(__LIBCCALL _getws)(wchar_t *__restrict __buf) __KOS_ASMNAME("getws");
__LIBC wchar_t * (__LIBCCALL _getws_s)(wchar_t *__restrict __str, size_t __buflen) __KOS_ASMNAME("getws_s");
#ifndef __fgetws_defined
#define __fgetws_defined 1
__NAMESPACE_STD_BEGIN
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#ifdef __PE__ /* In PE-mode, we don't export the size_t version */
__LIBC wchar_t *(__LIBCCALL __pe_fgetws)(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream) __ASMNAME("fgetws");
__LOCAL wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) { return __pe_fgetws(__ws,(int)__n,__stream); }
#else /* __PE__ */
__LIBC wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgetws_sz");
#endif /* !__PE__ */
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__LIBC wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream);
#endif /* !__USE_KOS */
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(fgetws)
#endif /* !__fgetws_defined */

/* Put wide string functions */
__LIBC int (__LIBCCALL _putws)(wchar_t const *__restrict __str) __KOS_ASMNAME("putws");
#ifndef __fputws_defined
#define __fputws_defined 1
__NAMESPACE_STD_BEGIN
__LIBC int (__LIBCCALL fputws)(wchar_t const *__restrict __str, FILE *__restrict __file);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(fputws)
#endif /* !__fputws_defined */

__LIBC int (__ATTR_CDECL _scwprintf)(wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vscwprintf)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _swprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __KOS_ASMNAME("swprintf");
__LIBC int (__LIBCCALL _vswprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) __KOS_ASMNAME("vswprintf");
__LIBC int (__ATTR_CDECL _snwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, ...) __KOS_ASMNAME("snwprintf_s");
__LIBC int (__LIBCCALL _vsnwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __VA_LIST __args) __KOS_ASMNAME("vsnwprintf_s");
/* The following 2 return an error, rather than the required size when the buffer is too small */
__LIBC int (__ATTR_CDECL _snwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vsnwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _fwprintf_p)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwprintf");
__LIBC int (__LIBCCALL _vfwprintf_p)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vfwprintf");
__LIBC int (__ATTR_CDECL _wprintf_p)(wchar_t const *__restrict __format, ...) __ASMNAME("wprintf");
__LIBC int (__LIBCCALL _vwprintf_p)(wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vwprintf");
__LIBC int (__ATTR_CDECL _swprintf_p)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME2("swprintf","_swprintf_c");
__LIBC int (__LIBCCALL _vswprintf_p)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME2("vswprintf","_vswprintf_c");
__LIBC int (__ATTR_CDECL _scwprintf_p)(wchar_t const *__restrict __format, ...) __ASMNAME("_scwprintf");
__LIBC int (__LIBCCALL _vscwprintf_p)(wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("_vscwprintf");
__LIBC int (__ATTR_CDECL _wprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __KOS_ASMNAME("wprintf_l");
__LIBC int (__LIBCCALL _vwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __KOS_ASMNAME("vwprintf_l");
__LIBC int (__ATTR_CDECL _wprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("wprintf_l","_wprintf_l");
__LIBC int (__LIBCCALL _vwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vwprintf_l","_vwprintf_l");
__LIBC int (__ATTR_CDECL _wprintf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("wprintf_l","_wprintf_l");
__LIBC int (__LIBCCALL _vwprintf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vwprintf_l","_vwprintf_l");
__LIBC int (__ATTR_CDECL _fwprintf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __KOS_ASMNAME("fwprintf_l");
__LIBC int (__LIBCCALL _vfwprintf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __KOS_ASMNAME("vfwprintf_l");
__LIBC int (__ATTR_CDECL _fwprintf_p_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("fwprintf_l","_fwprintf_l");
__LIBC int (__LIBCCALL _vfwprintf_p_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vfwprintf_l","_vfwprintf_l");
__LIBC int (__ATTR_CDECL _fwprintf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("fwprintf_l","_fwprintf_l");
__LIBC int (__LIBCCALL _vfwprintf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vfwprintf_l","_vfwprintf_l");
__LIBC int (__ATTR_CDECL _swprintf_c_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __KOS_ASMNAME("swprintf_c_l");
__LIBC int (__LIBCCALL _vswprintf_c_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __KOS_ASMNAME("vswprintf_c_l");
__LIBC int (__ATTR_CDECL _swprintf_p_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swprintf_c_l","_swprintf_c_l");
__LIBC int (__LIBCCALL _vswprintf_p_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vswprintf_c_l","_vswprintf_c_l");
__LIBC int (__ATTR_CDECL _swprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swprintf_c_l","_swprintf_c_l");
__LIBC int (__LIBCCALL _vswprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vswprintf_c_l","_vswprintf_c_l");
__LIBC int (__ATTR_CDECL _scwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __KOS_ASMNAME("scwprintf_l");
__LIBC int (__LIBCCALL _vscwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __KOS_ASMNAME("vscwprintf_l");
__LIBC int (__ATTR_CDECL _scwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("scwprintf_l","_scwprintf_l");
__LIBC int (__LIBCCALL _vscwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vscwprintf_l","_vscwprintf_l");
__LIBC int (__ATTR_CDECL _snwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __KOS_ASMNAME("snwprintf_l");
__LIBC int (__LIBCCALL _vsnwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __KOS_ASMNAME("vsnwprintf_l");
__LIBC int (__ATTR_CDECL _snwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) __KOS_ASMNAME("snwprintf_s_l");
__LIBC int (__LIBCCALL _vsnwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __KOS_ASMNAME("snwvprintf_s_l");

/* NOTE: ~safe~ functions are re-directed to the regular versions. (For the reason, see below) */
__LIBC int (__ATTR_CDECL _fwscanf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __KOS_ASMNAME("fwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _fwscanf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("fwscanf_l","_fwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _swscanf_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) __KOS_ASMNAME("swscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _swscanf_s_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swscanf_l","_swscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _snwscanf)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, ...) __KOS_ASMNAME("snwscanf"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _snwscanf_s)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, ...) __ASMNAME2("snwscanf","_snwscanf"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _snwscanf_l)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) __KOS_ASMNAME("snwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _snwscanf_s_l)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("snwscanf_l","_snwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _wscanf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __KOS_ASMNAME("wscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _wscanf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("wscanf_l","_wscanf_l"); /* No varargs version. */

#ifdef __USE_DOS_SLIB
/* Simply redirect these so-called ~safe~ functions to the regular version.
 * In KOS, they're already ~safe~ to begin with, because unknown format strings are always handled.
 * NOTE: For binary compatibility, assembly names such as 'fwprintf_s' are exported as alias,
 *       but should never be used by newly compiled applications. */
__LIBC int (__ATTR_CDECL fwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwprintf");
__LIBC int (__LIBCCALL vfwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vfwprintf");
__LIBC int (__ATTR_CDECL wprintf_s)(wchar_t const *__restrict __format, ...) __ASMNAME("wprintf");
__LIBC int (__LIBCCALL vwprintf_s)(wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vwprintf");
__LIBC int (__ATTR_CDECL fwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwscanf");
__LIBC int (__LIBCCALL vfwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vfwscanf");
__LIBC int (__ATTR_CDECL wscanf_s)(wchar_t const *__restrict __format, ...) __ASMNAME("wscanf");
__LIBC int (__LIBCCALL vwscanf_s)(wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vwscanf");
__LIBC int (__ATTR_CDECL swprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME2("swprintf","_swprintf_c");
__LIBC int (__LIBCCALL vswprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME2("vswprintf","_vswprintf_c");
__LIBC int (__ATTR_CDECL swscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) __ASMNAME("swscanf");
__LIBC int (__LIBCCALL vswscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vswscanf");
#endif /* __USE_DOS_SLIB */

__LIBC wchar_t *(__LIBCCALL _wtmpnam)(wchar_t *__restrict __buf) __KOS_ASMNAME("wtmpnam");
__LIBC errno_t (__LIBCCALL _wtmpnam_s)(wchar_t *__restrict __buf, size_t __buflen) __KOS_ASMNAME("wtmpnam_s");
__LIBC wchar_t *(__LIBCCALL _wtempnam)(wchar_t const *__dir, wchar_t const *__pfx) __KOS_ASMNAME("wtempnam");
__LIBC int (__LIBCCALL _wremove)(wchar_t const *__restrict __file) __WFS_FUNC(_wremove);

#ifdef __PE__
/* Versions lacking the C standard mandated BUFLEN argument...
 * NOTE: Internally, these functions will link against '.dos._swprintf' and '.dos._vswprintf' */
__LIBC int (__LIBCCALL _vswprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _swprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, ...);
#else /* __PE__ */
/* Outside of PE-mode, wchar_t is 32 bits wide and '.dos.' isn't inserted before symbol names. */
__LIBC int (__LIBCCALL __kos_vswprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vswprintf");
/* libc doesn't export these superfluous and confusion version of swprintf.
 * (They're lacking the BUFLEN argument mandated by the C standard).
 * So instead, they're implemented as a hack. */
__LOCAL int (__LIBCCALL _vswprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __VA_LIST __args) { return __kos_vswprintf(__buf,(size_t)-1,__format,__args); }
__LOCAL int (__ATTR_CDECL _swprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, ...) { __VA_LIST __args; int __result; __builtin_va_start(__args,__format); __result = _vswprintf(__buf,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__PE__ */

__LIBC int (__LIBCCALL __vswprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vswprintf_l","_vswprintf_l");
__LIBC int (__ATTR_CDECL __swprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swprintf_l","__swprintf_l");

#ifndef __NO_ASMNAME
__LIBC int (__ATTR_CDECL _swprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swprintf_c_l","_swprintf_c_l");
__LIBC int (__LIBCCALL _vswprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vswprintf_c_l","_vswprintf_c_l");
#else /* !__NO_ASMNAME */
__LOCAL int (__ATTR_CDECL _swprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) { __VA_LIST __args; int __result; __builtin_va_start(__args,__locale); __result = _vswprintf_c_l(__buf,__buflen,__format,__locale,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__LIBCCALL _vswprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) { return _vswprintf_c_l(__buf,__buflen,__format,__locale,__args); }
#endif /* __NO_ASMNAME */

#define getwchar()            fgetwc(stdin)
#define putwchar(c)           fputwc((c),stdout)
#define getwc(file)           fgetwc(file)
#define putwc(c,file)         fputwc(c,file)
#define _putwc_nolock(c,file) _fputwc_nolock(c,file)
#define _getwc_nolock(file)   _fgetwc_nolock(file)
#endif  /* _WSTDIO_DEFINED */
#endif  /* _STDIO_DEFINED */

#define _fgetc_nolock(_stream)    (--(_stream)->_cnt >= 0 ? 0xff & *(_stream)->_ptr++ : _filbuf(_stream))
#define _fputc_nolock(c,_stream)  (--(_stream)->_cnt >= 0 ? 0xff & (*(_stream)->_ptr++ = (char)(c)) :  _flsbuf((c),(_stream)))
#define _getc_nolock(_stream)     _fgetc_nolock(_stream)
#define _putc_nolock(c, _stream)  _fputc_nolock(c, _stream)
#define _getchar_nolock()         _getc_nolock(stdin)
#define _putchar_nolock(c)        _putc_nolock((c),stdout)
#define _getwchar_nolock()        _getwc_nolock(stdin)
#define _putwchar_nolock(c)       _putwc_nolock((c),stdout)

__LIBC void (__LIBCCALL _lock_file)(__FILE *__restrict __file) __KOS_ASMNAME("flockfile");
__LIBC void (__LIBCCALL _unlock_file)(__FILE *__restrict __file) __KOS_ASMNAME("funlockfile");
__LIBC int (__LIBCCALL _fclose_nolock)(__FILE *__restrict __file);
__LIBC int (__LIBCCALL _fflush_nolock)(__FILE *__restrict __file) __KOS_ASMNAME("fflush_unlocked");
__LIBC size_t (__LIBCCALL _fread_nolock)(void *__restrict __buf, size_t __elemsize, size_t __elemcount, __FILE *__restrict __file) __KOS_ASMNAME("fread_unlocked");
__LIBC size_t (__LIBCCALL _fwrite_nolock)(void const *__restrict __buf, size_t __elemsize, size_t __elemcount, __FILE *__restrict __file) __KOS_ASMNAME("fwrite_unlocked");
__LIBC size_t (__LIBCCALL _fread_nolock_s)(void *__restrict __buf, size_t __bufsize, size_t __elemsize, size_t __elemcount, __FILE *__restrict __file);
__LIBC int (__LIBCCALL _fseek_nolock)(__FILE *__restrict __file, __LONG32_TYPE__ __off, int __whence);
__LIBC __LONG32_TYPE__ (__LIBCCALL _ftell_nolock)(__FILE *__restrict __file);
__LIBC int (__LIBCCALL _fseeki64_nolock)(__FILE *__restrict __file, __INT64_TYPE__ __off, int __whence);
__LIBC __INT64_TYPE__ (__LIBCCALL _ftelli64_nolock)(__FILE *__restrict __file);
__LIBC int (__LIBCCALL _ungetc_nolock)(int __ch, __FILE *__restrict __file);

#define SYS_OPEN  _SYS_OPEN
__LIBC int (__LIBCCALL fgetchar)(void) __ASMNAME("_fgetchar");
__LIBC int (__LIBCCALL flushall)(void) __ASMNAME("_flushall");
__LIBC int (__LIBCCALL fputchar)(int __ch) __ASMNAME("_fputchar");
__LIBC int (__LIBCCALL rmtmp)(void) __ASMNAME("_rmtmp");

#endif /* __USE_DOS */

__DECL_END

#endif /* !_STDIO_H */
