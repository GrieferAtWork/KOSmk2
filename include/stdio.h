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

__DECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif

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
typedef __FS_TYPE(off) off_t;
#endif

#if defined(__USE_LARGEFILE64) && !defined(__off64_t_defined)
#define __off64_t_defined 1
typedef __off64_t off64_t;
#endif
#ifndef __ssize_t_defined
#define __ssize_t_defined 1
typedef __ssize_t ssize_t;
#endif
#endif /* __USE_XOPEN2K8 */

#ifndef __fpos_t_defined
#define __fpos_t_defined 1
typedef __FS_TYPE(pos) fpos_t;
#endif /* !__fpos_t_defined */

#ifdef __USE_LARGEFILE64
#ifndef __fpos64_t_defined
#define __fpos64_t_defined 1
typedef __pos64_t      fpos64_t;
#endif /* !__fpos64_t_defined */
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
#define P_tmpdir "/tmp"
#endif

#ifndef __KERNEL__
#ifndef __FILE_defined
#define __FILE_defined 1
typedef __FILE FILE;
#endif
#endif


#ifndef __KERNEL__

/* Standard streams. */
#undef stdin
#undef stdout
#undef stderr
__LIBC __FILE *(stdin);
__LIBC __FILE *(stdout);
__LIBC __FILE *(stderr);
#define stdin  stdin
#define stdout stdout
#define stderr stderr

__LIBC int (__LIBCCALL remove)(char const *__filename);
__LIBC int (__LIBCCALL rename)(char const *__old, char const *__new);
__LIBC __WUNUSED char *(__LIBCCALL tmpnam)(char *__s);
__LIBC int (__LIBCCALL fclose)(__FILE *__stream);
__LIBC int (__LIBCCALL fflush)(__FILE *__stream);
__LIBC void (__LIBCCALL setbuf)(__FILE *__restrict __stream, char *__restrict __buf);
__LIBC int (__LIBCCALL setvbuf)(__FILE *__restrict __stream, char *__restrict __buf, int __modes, size_t __n);
__LIBC int (__LIBCCALL fgetc)(__FILE *__stream);
__LIBC int (__LIBCCALL getc)(__FILE *__stream);
__LIBC int (__LIBCCALL getchar)(void);
__LIBC int (__LIBCCALL fputc)(int __c, __FILE *__stream);
__LIBC int (__LIBCCALL putc)(int __c, __FILE *__stream);
__LIBC int (__LIBCCALL putchar)(int __c);
#ifdef __USE_KOS
__LIBC int (__LIBCCALL removeat)(int __fd, char const *__filename);
#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict __s, size_t __n, __FILE *__restrict __stream);
#else
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict __s, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgets_sz");
#endif
__LIBC __ssize_t (__LIBCCALL fputs)(char const *__restrict __s, __FILE *__restrict __stream);
__LIBC __ssize_t (__LIBCCALL puts)(char const *__s);
#else
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict __s, int __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fputs)(char const *__restrict __s, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL puts)(char const *__s);
#endif
__LIBC int (__LIBCCALL ungetc) (int __c, __FILE *__stream);
__LIBC __WUNUSED size_t (__LIBCCALL fread)(void *__restrict __ptr, size_t __size, size_t __n, __FILE *__restrict __stream);
__LIBC size_t (__LIBCCALL fwrite)(void const *__restrict __ptr, size_t __size, size_t __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fseek)(__FILE *__stream, long int __off, int __whence);
__LIBC __WUNUSED long int (__LIBCCALL ftell)(__FILE *__stream);
__LIBC void (__LIBCCALL rewind)(__FILE *__stream);
__LIBC void (__LIBCCALL clearerr)(__FILE *__stream);
__LIBC __WUNUSED int (__LIBCCALL feof)(__FILE *__stream);
__LIBC __WUNUSED int (__LIBCCALL ferror)(__FILE *__stream);
__LIBC void (__LIBCCALL perror)(char const *__s);

#ifdef __USE_ATFILE
__LIBC int (__LIBCCALL renameat)(int __oldfd, char const *__old, int __newfd, char const *__new);
#endif
#ifndef __USE_FILE_OFFSET64
__LIBC __WUNUSED __FILE *(__LIBCCALL tmpfile)(void);
__LIBC __WUNUSED __FILE *(__LIBCCALL fopen)(char const *__restrict __filename, char const *__restrict __modes);
__LIBC __WUNUSED __FILE *(__LIBCCALL freopen)(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fgetpos)(__FILE *__restrict __stream, fpos_t *__restrict __pos);
__LIBC int (__LIBCCALL fsetpos)(__FILE *__stream, fpos_t const *__pos);
#else
__LIBC __WUNUSED __FILE *(__LIBCCALL tmpfile)(void) __ASMNAME("tmpfile64");
__LIBC __WUNUSED __FILE *(__LIBCCALL fopen)(char const *__restrict __filename, char const *__restrict __modes) __ASMNAME("fopen64");
__LIBC __WUNUSED __FILE *(__LIBCCALL freopen)(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream) __ASMNAME("freopen64");
__LIBC int (__LIBCCALL fgetpos)(__FILE *__restrict __stream, fpos_t *__restrict __pos) __ASMNAME("fgetpos64");
__LIBC int (__LIBCCALL fsetpos)(__FILE *__stream, fpos_t const *__pos) __ASMNAME("fsetpos64");
#endif
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED __FILE *(__LIBCCALL tmpfile64)(void);
__LIBC __WUNUSED __FILE *(__LIBCCALL fopen64)(char const *__restrict __filename, char const *__restrict __modes);
__LIBC __WUNUSED __FILE *(__LIBCCALL freopen64)(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fseeko64)(__FILE *__stream, __off64_t __off, int __whence);
__LIBC __WUNUSED __off64_t (__LIBCCALL ftello64)(__FILE *__stream);
__LIBC int (__LIBCCALL fgetpos64)(__FILE *__restrict __stream, fpos64_t *__restrict __pos);
__LIBC int (__LIBCCALL fsetpos64)(__FILE *__stream, fpos64_t const *__pos);
#endif
#ifdef __USE_MISC
__LIBC __WUNUSED char *(__LIBCCALL tmpnam_r)(char *__s);
__LIBC int (__LIBCCALL fflush_unlocked)(__FILE *__stream);
__LIBC void (__LIBCCALL setbuffer)(__FILE *__restrict __stream, char *__restrict __buf, size_t __size);
__LIBC void (__LIBCCALL setlinebuf)(__FILE *__stream);
__LIBC int (__LIBCCALL fgetc_unlocked)(__FILE *__stream);
__LIBC int (__LIBCCALL fputc_unlocked)(int __c, __FILE *__stream);
__LIBC size_t (__LIBCCALL fread_unlocked)(void *__restrict __ptr, size_t __size, size_t __n, __FILE *__restrict __stream);
__LIBC size_t (__LIBCCALL fwrite_unlocked)(void const *__restrict __ptr, size_t __size, size_t __n, __FILE *__restrict __stream);
__LIBC void (__LIBCCALL clearerr_unlocked)(__FILE *__stream);
__LIBC __WUNUSED int (__LIBCCALL feof_unlocked)(__FILE *__stream);
__LIBC __WUNUSED int (__LIBCCALL ferror_unlocked)(__FILE *__stream);
__LIBC __WUNUSED int (__LIBCCALL fileno_unlocked)(__FILE *__stream);
#endif
#if defined(__USE_MISC) || defined(__USE_XOPEN)
__LIBC __ATTR_MALLOC __WUNUSED char *(__LIBCCALL tempnam)(char const *__dir, char const *__pfx);
#endif
#ifdef __USE_POSIX
__LIBC __WUNUSED __FILE *(__LIBCCALL fdopen)(int __fd, char const *__modes);
__LIBC __WUNUSED int (__LIBCCALL fileno)(__FILE *__stream);
#endif
#ifdef __USE_XOPEN2K8
__LIBC __WUNUSED __FILE *(__LIBCCALL fmemopen)(void *__s, size_t __len, char const *__modes);
__LIBC __WUNUSED __FILE *(__LIBCCALL open_memstream)(char **__bufloc, size_t *__sizeloc);
__LIBC __WUNUSED __ssize_t (__LIBCCALL __getdelim)(char **__restrict __lineptr, size_t *__restrict __n, int __delimiter, __FILE *__restrict __stream) __ASMNAME("getdelim");
__LIBC __WUNUSED __ssize_t (__LIBCCALL getdelim)(char **__restrict __lineptr, size_t *__restrict __n, int __delimiter, __FILE *__restrict __stream);
__LIBC __WUNUSED __ssize_t (__LIBCCALL getline)(char **__restrict __lineptr, size_t *__restrict __n, __FILE *__restrict __stream);
#endif
#ifdef __USE_KOS
__LIBC __ssize_t (__ATTR_CDECL printf)(char const *__restrict __format, ...);
__LIBC __ssize_t (__ATTR_CDECL fprintf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __ssize_t (__LIBCCALL vprintf)(char const *__restrict __format, __VA_LIST __args);
__LIBC __ssize_t (__LIBCCALL vfprintf)(__FILE *__restrict __stream, char const *__restrict __format, __VA_LIST __args);
__LIBC __WUNUSED __ssize_t (__ATTR_CDECL fscanf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __WUNUSED __ssize_t (__ATTR_CDECL scanf)(char const *__restrict __format, ...);
#ifdef __USE_ISOC99
__LIBC __WUNUSED __ssize_t (__LIBCCALL vfscanf)(__FILE *__restrict __stream, char const *__restrict __format, __VA_LIST __args);
__LIBC __WUNUSED __ssize_t (__LIBCCALL vscanf)(char const *__restrict __format, __VA_LIST __args);
#endif
#ifdef __USE_XOPEN2K8
__LIBC __ssize_t (__LIBCCALL vdprintf)(int __fd, char const *__restrict __format, __VA_LIST __args);
__LIBC __ssize_t (__ATTR_CDECL dprintf)(int __fd, char const *__restrict __format, ...);
#endif
#else
__LIBC int (__ATTR_CDECL printf)(char const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL fprintf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vprintf)(char const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vfprintf)(__FILE *__restrict __stream, char const *__restrict __format, __VA_LIST __args);
__LIBC __WUNUSED int (__ATTR_CDECL fscanf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __WUNUSED int (__ATTR_CDECL scanf)(char const *__restrict __format, ...);
#ifdef __USE_ISOC99
__LIBC __WUNUSED int (__LIBCCALL vfscanf)(__FILE *__restrict __stream, char const *__restrict __format, __VA_LIST __args);
__LIBC __WUNUSED int (__LIBCCALL vscanf)(char const *__restrict __format, __VA_LIST __args);
#endif
#ifdef __USE_XOPEN2K8
__LIBC int (__LIBCCALL vdprintf)(int __fd, char const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL dprintf)(int __fd, char const *__restrict __format, ...);
#endif
#endif
#ifdef __USE_POSIX
__LIBC int (__LIBCCALL getc_unlocked)(__FILE *__stream);
__LIBC int (__LIBCCALL getchar_unlocked)(void);
__LIBC int (__LIBCCALL putc_unlocked)(int __c, __FILE *__stream);
__LIBC int (__LIBCCALL putchar_unlocked)(int __c);
#ifndef __ctermid_defined
#define __ctermid_defined 1
__LIBC char *(__LIBCCALL ctermid)(char *__s);
#endif /* !__ctermid_defined */
__LIBC void (__LIBCCALL flockfile)(__FILE *__stream);
__LIBC __WUNUSED int (__LIBCCALL ftrylockfile)(__FILE *__stream);
__LIBC void (__LIBCCALL funlockfile)(__FILE *__stream);
#endif
#ifdef __USE_POSIX2
__LIBC __WUNUSED __FILE *(__LIBCCALL popen)(char const *__command, char const *__modes);
__LIBC int (__LIBCCALL pclose)(__FILE *__stream);
#endif /* __USE_POSIX2 */
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__LIBC int (__LIBCCALL getw)(__FILE *__stream);
__LIBC int (__LIBCCALL putw)(int __w, __FILE *__stream);
#endif
#ifdef __USE_GNU
__LIBC int (__LIBCCALL fcloseall)(void);
//__LIBC __WUNUSED __FILE *(__LIBCCALL fopencookie)(void *__restrict __magic_cookie, char const *__restrict __modes, _IO_cookie_io_functions_t __io_funcs);
#ifdef __USE_KOS
#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
__LIBC __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict __s, size_t __n, __FILE *__restrict __stream);
#else
__LIBC __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict __s, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgets_unlocked_sz");
#endif
__LIBC __ssize_t (__LIBCCALL fputs_unlocked)(char const *__restrict __s, __FILE *__restrict __stream);
#else
__LIBC __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict __s, int __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fputs_unlocked)(char const *__restrict __s, __FILE *__restrict __stream);
#endif

struct obstack;
__LIBC int (__LIBCCALL obstack_printf)(struct obstack *__restrict __obstack, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL obstack_vprintf)(struct obstack *__restrict __obstack, char const *__restrict __format, __VA_LIST __args);
#endif /* __USE_GNU */
#if !defined(__USE_ISOC11) || \
    (defined(__cplusplus) && __cplusplus <= 201103L)
__LIBC __WUNUSED __ATTR_DEPRECATED("No buffer size checks") char *(__LIBCCALL gets)(char *__s);
#endif
#if defined(__USE_LARGEFILE) || defined(__USE_XOPEN2K)
__LIBC int (__LIBCCALL fseeko)(__FILE *__stream, __FS_TYPE(off) __off, int __whence) __FS_FUNC(fseeko);
__LIBC __WUNUSED __FS_TYPE(off) (__LIBCCALL ftello)(__FILE *__stream) __FS_FUNC(ftello);
#endif /* __USE_LARGEFILE || __USE_XOPEN2K */
#ifdef __USE_XOPEN
__LIBC char *(__LIBCCALL cuserid)(char *__s);
#endif /* __USE_XOPEN */
#ifdef __USE_KOS
/* For use with 'format_printf()' and friends: Prints to a 'FILE *' closure argument. */
__LIBC __ssize_t (__LIBCCALL file_printer)(char const *__restrict __data,
                                           __size_t __datalen, void *__closure);
#endif /* __USE_KOS */

#endif /* !__KERNEL__ */




#ifdef __USE_KOS
__LIBC size_t (__ATTR_CDECL sprintf)(char *__restrict __s, char const *__restrict __format, ...);
__LIBC size_t (__LIBCCALL vsprintf)(char *__restrict __s, char const *__restrict __format, __VA_LIST __args);
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
__LIBC size_t (__ATTR_CDECL snprintf)(char *__restrict __s, size_t __maxlen, char const *__restrict __format, ...);
__LIBC size_t (__LIBCCALL vsnprintf)(char *__restrict __s, size_t __maxlen, char const *__restrict __format, __VA_LIST __args);
#endif /* __USE_ISOC99 || __USE_UNIX98 */
__LIBC size_t (__ATTR_CDECL sscanf)(char const *__restrict __s, char const *__restrict __format, ...);
#ifdef __USE_ISOC99
__LIBC size_t (__LIBCCALL vsscanf)(char const *__restrict __s, char const *__restrict __format, __VA_LIST __args);
#endif /* __USE_ISOC99 */
#else /* __USE_KOS */
__LIBC int (__ATTR_CDECL sprintf)(char *__restrict __s, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vsprintf)(char *__restrict __s, char const *__restrict __format, __VA_LIST __args);
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
__LIBC int (__ATTR_CDECL snprintf)(char *__restrict __s, size_t __maxlen, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vsnprintf)(char *__restrict __s, size_t __maxlen, char const *__restrict __format, __VA_LIST __args);
#endif /* __USE_ISOC99 || __USE_UNIX98 */
__LIBC int (__ATTR_CDECL sscanf)(char const *__restrict __s, char const *__restrict __format, ...);
#ifdef __USE_ISOC99
__LIBC int (__LIBCCALL vsscanf)(char const *__restrict __s, char const *__restrict __format, __VA_LIST __args);
#endif /* __USE_ISOC99 */
#endif /* !__USE_KOS */

#ifdef __USE_GNU
#ifndef __KERNEL__
#ifdef __USE_KOS
__LIBC __WUNUSED __ssize_t (__ATTR_CDECL asprintf)(char **__restrict __ptr, char const *__restrict __format, ...);
__LIBC __WUNUSED __ssize_t (__ATTR_CDECL __asprintf)(char **__restrict __ptr, char const *__restrict __format, ...) __ASMNAME("asprintf");
__LIBC __WUNUSED __ssize_t (__LIBCCALL vasprintf)(char **__restrict __ptr, char const *__restrict __format, __VA_LIST __args);
#else /* __USE_KOS */
__LIBC __WUNUSED int (__ATTR_CDECL asprintf)(char **__restrict __ptr, char const *__restrict __format, ...);
__LIBC __WUNUSED int (__ATTR_CDECL __asprintf)(char **__restrict __ptr, char const *__restrict __format, ...) __ASMNAME("asprintf");
__LIBC __WUNUSED int (__LIBCCALL vasprintf)(char **__restrict __ptr, char const *__restrict __format, __VA_LIST __args);
#endif /* !__USE_KOS */
#endif /* !__KERNEL__ */
#endif /* __USE_GNU */

#if defined(__USE_XOPEN) && !defined(__USE_XOPEN2K) && !defined(__USE_GNU)
#include <getopt.h>
#endif

__DECL_END

#endif /* !_STDIO_H */
