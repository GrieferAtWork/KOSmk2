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
#if !defined(__CRT_GLC) || \
    (defined(__USE_DOS) || defined(__BUILDING_LIBC))
#include <bits/io-file.h>
#endif /* __USE_DOS || __BUILDING_LIBC */
#ifdef __CRT_DOS
#include <xlocale.h>
#endif /* __CRT_DOS */
#ifdef __CYG_COMPAT__
#include <sys/reent.h>
#endif /* __CYG_COMPAT__ */

__SYSDECL_BEGIN

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

#ifndef NULL
#define NULL __NULLPTR
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

#ifndef __std_fpos_t_defined
#define __std_fpos_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __FS_TYPE(pos) fpos_t;
__NAMESPACE_STD_END
#endif /* !__std_fpos_t_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __fpos_t_defined
#define __fpos_t_defined 1
__NAMESPACE_STD_USING(fpos_t)
#endif /* !__fpos_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __USE_LARGEFILE64
#ifndef __fpos64_t_defined
#define __fpos64_t_defined 1
typedef __pos64_t      fpos64_t;
#endif /* !__fpos64_t_defined */
#endif

/* Dos has different values for some of these.
 * Yet since they don't collide with each other, `setvbuf()' accepts either. */
#define __DOS_IOFBF 0x0000 /*< Fully buffered. */
#define __DOS_IOLBF 0x0040 /*< Line buffered. */
#define __DOS_IONBF 0x0004 /*< No buffering. */

/* The possibilities for the third argument to `setvbuf()'. */
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
#ifndef __std_FILE_defined
#define __std_FILE_defined 1
__NAMESPACE_STD_BEGIN
typedef __FILE FILE;
__NAMESPACE_STD_END
#endif /* !__std_FILE_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __FILE_defined
#define __FILE_defined 1
__NAMESPACE_STD_USING(FILE)
#endif /* !__FILE_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#endif /* !__KERNEL__ */

#ifndef __KERNEL__

/* Standard streams. */
#ifndef __stdstreams_defined
#define __stdstreams_defined 1
#undef stdin
#undef stdout
#undef stderr
#ifdef __CYG_COMPAT__
#   define stdin  (__CYG_REENT->__cyg_stdin)
#   define stdout (__CYG_REENT->__cyg_stdout)
#   define stderr (__CYG_REENT->__cyg_stderr)
#elif defined(__DOS_COMPAT__)
#ifdef __USE_DOS_LINKOBJECTS
__LIBC FILE _iob[];
#   define stdin  (_iob+0)
#   define stdout (_iob+1)
#   define stderr (_iob+2)
#else /* __USE_DOS_LINKOBJECTS */
__LIBC FILE *(__LIBCCALL __iob_func)(void);
#   define stdin  (__iob_func()+0)
#   define stdout (__iob_func()+1)
#   define stderr (__iob_func()+2)
#endif /* !__USE_DOS_LINKOBJECTS */
#else /* __DOS_COMPAT__ */
__LIBC __FILE *stdin;
__LIBC __FILE *stdout;
__LIBC __FILE *stderr;
#   define stdin  stdin
#   define stdout stdout
#   define stderr stderr
#endif /* !__DOS_COMPAT__ */
#endif /* !__stdstreams_defined */

__NAMESPACE_STD_BEGIN
#ifndef __std_remove_defined
#define __std_remove_defined 1
__REDIRECT_UFS(__LIBC,__NONNULL((1)),int,__LIBCCALL,remove,(char const *__file),remove,(__file))
#endif /* !__std_remove_defined */
#ifndef __std_rename_defined
#define __std_rename_defined 1
__REDIRECT_UFS(__LIBC,__NONNULL((1)),int,__LIBCCALL,rename,(char const *__old, char const *__new),rename,(__old,__new))
#endif /* !__std_rename_defined */
#ifdef __USE_KOS
__LIBC __WUNUSED char *(__LIBCCALL tmpnam)(char __buf[L_tmpnam]) __UFS_FUNC(tmpnam);
#else /* __USE_KOS */
__LIBC __WUNUSED char *(__LIBCCALL tmpnam)(char *__buf) __UFS_FUNC(tmpnam);
#endif /* !__USE_KOS */
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

#ifdef __USE_KOS_STDEXT
#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
__REDIRECT(__LIBC,__WUNUSED,char *,__LIBCCALL,fgets,(char *__restrict __buf, size_t __n, __FILE *__restrict __stream),fgets_sz,(__buf,__n,__stream))
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */
__LIBC __ssize_t (__LIBCCALL fputs)(char const *__restrict __buf, __FILE *__restrict __stream);
__LIBC __ssize_t (__LIBCCALL puts)(char const *__restrict __str);
#else /* __USE_KOS_STDEXT */
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict __buf, int __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fputs)(char const *__restrict __str, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL puts)(char const *__restrict __str);
#endif /* !__USE_KOS_STDEXT */
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
__REDIRECT_FS_FUNC(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,tmpfile,(void),tmpfile,())
__REDIRECT_UFS_FUNCn(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,fopen,(char const *__restrict __filename, char const *__restrict __modes),fopen,(__filename,__modes))
__REDIRECT_UFS_FUNCn(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,freopen,(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream),freopen,(__filename,__modes,__stream))
__REDIRECT_FS_FUNC(__LIBC,,int,__LIBCCALL,fgetpos,(__FILE *__restrict __stream, fpos_t *__restrict __pos),fgetpos,(__stream,__pos))
__REDIRECT_FS_FUNC(__LIBC,,int,__LIBCCALL,fsetpos,(__FILE *__restrict __stream, fpos_t const *__restrict __pos),fsetpos,(__stream,__pos))
#ifdef __USE_KOS_STDEXT
__LIBC __ATTR_LIBC_PRINTF(1,2) __ssize_t (__ATTR_CDECL printf)(char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL fprintf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(1,0) __ssize_t (__LIBCCALL vprintf)(char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF(2,0) __ssize_t (__LIBCCALL vfprintf)(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_SCANF(2,0) __WUNUSED __size_t (__ATTR_CDECL fscanf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_SCANF(1,2) __WUNUSED __size_t (__ATTR_CDECL scanf)(char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC __ATTR_LIBC_SCANF(2,0) __WUNUSED __size_t (__LIBCCALL vfscanf)(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_SCANF(1,0) __WUNUSED __size_t (__LIBCCALL vscanf)(char const *__restrict __format, __builtin_va_list __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#else /* __USE_KOS_STDEXT */
__LIBC __ATTR_LIBC_PRINTF(1,2) int (__ATTR_CDECL printf)(char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(2,3) int (__ATTR_CDECL fprintf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(1,0) int (__LIBCCALL vprintf)(char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF(2,0) int (__LIBCCALL vfprintf)(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_SCANF(2,3) __WUNUSED int (__ATTR_CDECL fscanf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_SCANF(1,2) __WUNUSED int (__ATTR_CDECL scanf)(char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC __ATTR_LIBC_SCANF(2,0) __WUNUSED int (__LIBCCALL vfscanf)(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_SCANF(1,0) __WUNUSED int (__LIBCCALL vscanf)(char const *__restrict __format, __builtin_va_list __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#endif /* !__USE_KOS_STDEXT */
#if !defined(__USE_ISOC11) || \
    (defined(__cplusplus) && __cplusplus <= 201103L)
__LIBC __WUNUSED __ATTR_DEPRECATED("No buffer size checks") char *(__LIBCCALL gets)(char *__restrict __buf);
#endif
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
#ifndef __remove_defined
#define __remove_defined 1
__NAMESPACE_STD_USING(remove)
#endif /* !__remove_defined */
#ifndef __rename_defined
#define __rename_defined 1
__NAMESPACE_STD_USING(rename)
#endif /* !__rename_defined */
#ifndef __perror_defined
#define __perror_defined 1
__NAMESPACE_STD_USING(perror)
#endif /* !__perror_defined */
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
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __USE_KOS
/* Reopen the given file stream using a provided file descriptor.
 * @param: MODE: Set of `FDREOPEN_*'
 * @return: STREAM: Successfully re-opened the given file.
 * @return: NULL:   Failed to re-open the file (see `errno' for details) */
__LIBC __PORT_KOSONLY __WUNUSED __FILE *(__LIBCCALL fdreopen)(int __fd, char const *__restrict __modes,
                                                              __FILE *__restrict __stream, int __mode);
#define FDREOPEN_DUP            0x0 /*< Duplicate the given descriptor, creating a private copy for the stream. */
#define FDREOPEN_INHERIT        0x1 /*< Inherit the given `fd' on success, using that same number for the stream. */
#define FDREOPEN_CLOSE_ON_ERROR 0x2 /*< Close `FD' if an error occurred during the attempt at re-opening it. */
#endif /* __USE_KOS */

#ifdef __CRT_GLC
#ifdef __USE_XOPEN2K8
#ifdef __USE_KOS
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_NODOS_ALT(fdopen+vfprintf) __ssize_t (__LIBCCALL vdprintf)(int __fd, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS_ALT(fdopen+fprintf)  __ssize_t (__ATTR_CDECL dprintf)(int __fd, char const *__restrict __format, ...);
#else /* __USE_KOS */
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_NODOS_ALT(fdopen+vfprintf) int (__LIBCCALL vdprintf)(int __fd, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS_ALT(fdopen+fprintf)  int (__ATTR_CDECL dprintf)(int __fd, char const *__restrict __format, ...);
#endif /* !__USE_KOS */
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_KOS
__REDIRECT_UFS(__LIBC,__PORT_NODOS_ALT(remove),int,__LIBCCALL,removeat,
              (int __fd, char const *__filename),removeat,(__fd,__filename))
#endif /* __USE_KOS */
#ifdef __USE_ATFILE
__REDIRECT_UFS(__LIBC,__PORT_NODOS_ALT(rename),int,__LIBCCALL,renameat,
              (int __oldfd, char const *__old, int __newfd, char const *__new),
               renameat,(__oldfd,__old,__newfd,__new))
#ifdef __USE_KOS
__REDIRECT_UFS(__LIBC,__PORT_KOSONLY_ALT(renameat),int,__LIBCCALL,frenameat,
              (int __oldfd, char const *__old, int __newfd, char const *__new, int __flags),
               frenameat,(__oldfd,__old,__newfd,__new,__flags))
#endif /* __USE_KOS */
#endif /* __USE_ATFILE */
#endif /* __CRT_GLC */

#ifdef __USE_LARGEFILE64
__REDIRECT_IFDOS(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,tmpfile64,(void),tmpfile,())
__REDIRECT_IFDOS(__LIBC,,int,__LIBCCALL,fseeko64,(__FILE *__stream, __off64_t __off, int __whence),_fseeki64,(__stream,__off,__whence))
__REDIRECT_IFDOS(__LIBC,__WUNUSED,__off64_t,__LIBCCALL,ftello64,(__FILE *__stream),_ftelli64,(__stream))
#ifdef __DOS_COMPAT__
__REDIRECT_UFS_(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,fopen64,(char const *__restrict __filename, char const *__restrict __modes),fopen,(__filename,__modes))
__REDIRECT_UFS_(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,freopen64,(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream),freopen,(__filename,__modes,__stream))
__LOCAL int (__LIBCCALL fgetpos64)(__FILE *__restrict __stream, fpos64_t *__restrict __pos) { return (__pos && (__off64_t)(*__pos = (fpos64_t)ftello64(__stream)) >= 0) ? 0 : -1; }
__LOCAL int (__LIBCCALL fsetpos64)(__FILE *__stream, fpos64_t const *__pos) { return (__pos && fseeko64(__stream,(__off64_t)*__pos,SEEK_SET) >= 0) ? 0 : -1; }
#else /* __DOS_COMPAT__ */
__REDIRECT_UFS(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,fopen64,(char const *__restrict __filename, char const *__restrict __modes),fopen64,(__filename,__modes))
__REDIRECT_UFS(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,freopen64,(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream),freopen64,(__filename,__modes,__stream))
__LIBC int (__LIBCCALL fgetpos64)(__FILE *__restrict __stream, fpos64_t *__restrict __pos);
__LIBC int (__LIBCCALL fsetpos64)(__FILE *__stream, fpos64_t const *__pos);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_MISC
#ifdef __DOS_COMPAT__
__LOCAL __WUNUSED char *(__LIBCCALL tmpnam_r)(char *__buf) { return __buf ? tmpnam(__buf) : NULL; }
__LOCAL void (__LIBCCALL setbuffer)(__FILE *__restrict __stream, char *__restrict __buf, size_t __size) { setvbuf(__stream,__buf,__buf ? _IOFBF : _IONBF,__buf ? __size : (size_t)0); }
__LOCAL void (__LIBCCALL setlinebuf)(__FILE *__stream) { setvbuf(__stream,NULL,_IOLBF,0); }
#else /* __DOS_COMPAT__ */
__REDIRECT_UFS(__LIBC,__WUNUSED,char *,__LIBCCALL,tmpnam_r,(char *__buf),tmpnam_r,(__buf))
__LIBC void (__LIBCCALL setbuffer)(__FILE *__restrict __stream, char *__restrict __buf, size_t __size);
__LIBC void (__LIBCCALL setlinebuf)(__FILE *__stream);
#endif /* !__DOS_COMPAT__ */
__REDIRECT_IFDOS(__LIBC,,int,__LIBCCALL,fflush_unlocked,(__FILE *__stream),_fflush_nolock,(__stream))
__REDIRECT_IFDOS(__LIBC,,size_t,__LIBCCALL,fread_unlocked,(void *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream),_fread_nolock,(__buf,__size,__n,__stream))
__REDIRECT_IFDOS(__LIBC,,size_t,__LIBCCALL,fwrite_unlocked,(void const *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream),_fwrite_nolock,(__buf,__size,__n,__stream))
#ifdef __CRT_KOS
__REDIRECT2(__LIBC,__WUNUSED,int,__LIBCCALL,fileno_unlocked,(__FILE *__stream),fileno,_fileno,(__stream))
#else /* __CRT_KOS */
__REDIRECT_IFDOS(__LIBC,__WUNUSED,int,__LIBCCALL,fileno_unlocked,(__FILE *__stream),_fileno,(__stream))
#endif /* !__CRT_KOS */
__REDIRECT_IFDOS(__LIBC,__WUNUSED,int,__LIBCCALL,feof_unlocked,(__FILE *__restrict __stream),feof,(__stream))
__REDIRECT_IFDOS(__LIBC,__WUNUSED,int,__LIBCCALL,ferror_unlocked,(__FILE *__restrict __stream),ferror,(__stream))
__REDIRECT_IFDOS_VOID(__LIBC,,__LIBCCALL,clearerr_unlocked,(__FILE *__stream),clearerr,(__stream))
#ifdef __DOS_COMPAT__
#ifndef ____dos_flsbuf_defined
#define ____dos_flsbuf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_flsbuf,(int __ch, __FILE *__restrict __file),_flsbuf,(__ch,__file))
#endif /* !____dos_flsbuf_defined */
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_filbuf,(__FILE *__restrict __file),_filbuf,(__file))
#define fgetc_unlocked(stream)    (--(stream)->__f_cnt >= 0 ? 0xff & *(stream)->__f_ptr++ : __dos_filbuf(stream))
#define fputc_unlocked(c,stream)  (--(stream)->__f_cnt >= 0 ? 0xff & (*(stream)->__f_ptr++ = (char)(c)) :  __dos_flsbuf((c),(stream)))
#else /* __DOS_COMPAT__ */
__LIBC int (__LIBCCALL fgetc_unlocked)(__FILE *__stream);
__LIBC int (__LIBCCALL fputc_unlocked)(int __ch, __FILE *__stream);
#endif /* __DOS_COMPAT__ */
#endif /* __USE_MISC */
#if defined(__USE_MISC) || defined(__USE_XOPEN) || defined(__USE_DOS)
#ifdef __USE_DOSFS
__REDIRECT(__LIBC,__ATTR_MALLOC __WUNUSED,char *,__LIBCCALL,tempnam,(char const *__dir, char const *__pfx),_tempnam,(__dir,__pfx))
#else /* __USE_DOSFS */
__LIBC __ATTR_MALLOC __WUNUSED char *(__LIBCCALL tempnam)(char const *__dir, char const *__pfx);
#endif /* !__USE_DOSFS */
#endif /* __USE_MISC || __USE_XOPEN || __USE_DOS */
#if defined(__USE_POSIX) || defined(__USE_DOS)
__REDIRECT_IFDOS(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,fdopen,(int __fd, char const *__restrict __modes),_fdopen,(__fd,__modes))
__REDIRECT_IFDOS(__LIBC,__WUNUSED,int,__LIBCCALL,fileno,(__FILE *__stream),_fileno,(__stream))
#endif /* __USE_POSIX || __USE_DOS */
#ifdef __USE_XOPEN2K8
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __WUNUSED __FILE *(__LIBCCALL fmemopen)(void *__mem, size_t __len, char const *__modes);
__LIBC __PORT_NODOS __WUNUSED __FILE *(__LIBCCALL open_memstream)(char **__bufloc, size_t *__sizeloc);
__REDIRECT(__LIBC,__PORT_NODOS __WUNUSED,__ssize_t,__LIBCCALL,__getdelim,(char **__restrict __lineptr, size_t *__restrict __n, int __delimiter, __FILE *__restrict __stream),getdelim,(__lineptr,__n,__delimiter,__stream))
__LIBC __PORT_NODOS __WUNUSED __ssize_t (__LIBCCALL getdelim)(char **__restrict __lineptr, size_t *__restrict __n, int __delimiter, __FILE *__restrict __stream);
__LIBC __PORT_NODOS __WUNUSED __ssize_t (__LIBCCALL getline)(char **__restrict __lineptr, size_t *__restrict __n, __FILE *__restrict __stream);
#endif /* __CRT_GLC */
#endif /* __USE_XOPEN2K8 */
#ifdef __USE_POSIX
__REDIRECT_IFDOS(__LIBC,,int,__LIBCCALL,getc_unlocked,(__FILE *__stream),_getc_nolock,(__stream))
#ifdef __DOS_COMPAT__
#ifndef ____dos_flsbuf_defined
#define ____dos_flsbuf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_flsbuf,(int __ch, __FILE *__restrict __file),_flsbuf,(__ch,__file))
#endif /* !____dos_flsbuf_defined */
__LOCAL int (__LIBCCALL putc_unlocked)(int __ch, __FILE *__stream) { return --__stream->__f_cnt >= 0 ? 0xff & (*__stream->__f_ptr++ = (char)__ch) :  __dos_flsbuf(__ch,__stream); }
#else /* __DOS_COMPAT__ */
__LIBC int (__LIBCCALL putc_unlocked)(int __ch, __FILE *__stream);
#endif /* !__DOS_COMPAT__ */
#ifdef __DOS_COMPAT__
__LOCAL int (__LIBCCALL getchar_unlocked)(void) { return getc_unlocked(stdin); }
__LOCAL int (__LIBCCALL putchar_unlocked)(int __ch) { return putc_unlocked(__ch,stdout); }
#else /* __DOS_COMPAT__ */
__LIBC int (__LIBCCALL getchar_unlocked)(void);
__LIBC int (__LIBCCALL putchar_unlocked)(int __ch);
#endif /* !__DOS_COMPAT__ */
#ifdef __CRT_GLC
#ifndef __ctermid_defined
#define __ctermid_defined 1
__LIBC __PORT_NODOS char *(__LIBCCALL ctermid)(char *__buf);
#endif /* !__ctermid_defined */
__LIBC __PORT_NODOS __WUNUSED int (__LIBCCALL ftrylockfile)(__FILE *__stream);
#endif /* __CRT_GLC */
__REDIRECT_IFDOS_VOID(__LIBC,,__LIBCCALL,flockfile,(__FILE *__stream),_lock_file,(__stream))
__REDIRECT_IFDOS_VOID(__LIBC,,__LIBCCALL,funlockfile,(__FILE *__stream),_unlock_file,(__stream))
#endif /* __USE_POSIX */
#ifdef __USE_POSIX2
__REDIRECT_IFDOS(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,popen,(char const *__command, char const *__modes),_popen,(__command,__modes))
__REDIRECT_IFDOS(__LIBC,,int,__LIBCCALL,pclose,(__FILE *__stream),_pclose,(__stream))
#endif /* __USE_POSIX2 */
#if defined(__USE_MISC) || defined(__USE_DOS) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__REDIRECT_IFDOS(__LIBC,,int,__LIBCCALL,getw,(__FILE *__stream),_getw,(__stream))
__REDIRECT_IFDOS(__LIBC,,int,__LIBCCALL,putw,(int __w, __FILE *__stream),_putw,(__w,__stream))
#endif
#if defined(__USE_GNU) || defined(__USE_DOS)
__REDIRECT_IFDOS(__LIBC,,int,__LIBCCALL,fcloseall,(void),_fcloseall,())
#endif /* __USE_GNU || __USE_DOS */
#ifdef __USE_GNU
#ifdef __CRT_GLC
//__LIBC __PORT_NODOS __WUNUSED __FILE *(__LIBCCALL fopencookie)(void *__restrict __magic_cookie, char const *__restrict __modes, _IO_cookie_io_functions_t __io_funcs);
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC __PORT_NODOS_ALT(fgets) __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__REDIRECT(__LIBC,__PORT_NODOS_ALT(fgets) __WUNUSED,char *,__LIBCCALL,fgets_unlocked,
          (char *__restrict __buf, size_t __n, __FILE *__restrict __stream),fgets_unlocked_sz,(__buf,__n,__stream))
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
__LIBC __PORT_NODOS_ALT(fputs) __ssize_t (__LIBCCALL fputs_unlocked)(char const *__restrict __str, __FILE *__restrict __stream);
#else /* __USE_KOS */
__LIBC __PORT_NODOS_ALT(fgets) __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict __str, int __n, __FILE *__restrict __stream);
__LIBC __PORT_NODOS_ALT(fputs) int (__LIBCCALL fputs_unlocked)(char const *__restrict __str, __FILE *__restrict __stream);
#endif /* !__USE_KOS */
struct obstack;
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS int (__LIBCCALL obstack_printf)(struct obstack *__restrict __obstack, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_NODOS int (__LIBCCALL obstack_vprintf)(struct obstack *__restrict __obstack, char const *__restrict __format, __builtin_va_list __args);
#endif /* __CRT_GLC */
#endif /* __USE_GNU */

#if defined(__USE_LARGEFILE) || defined(__USE_XOPEN2K)
#ifdef __DOS_COMPAT__
#ifdef __USE_FILE_OFFSET64
__REDIRECT(__LIBC,,int,__LIBCCALL,fseeko,(__FILE *__stream, __FS_TYPE(off) __off, int __whence),_fseeki64,(__stream,__off,__whence))
__REDIRECT(__LIBC,__WUNUSED,__FS_TYPE(off),__LIBCCALL,ftello,(__FILE *__stream),_ftelli64,(__stream))
#else /* __USE_FILE_OFFSET64 */
__REDIRECT(__LIBC,,int,__LIBCCALL,fseeko,(__FILE *__stream, __FS_TYPE(off) __off, int __whence),fseek,(__stream,__off,__whence))
__REDIRECT(__LIBC,__WUNUSED,__FS_TYPE(off),__LIBCCALL,ftello,(__FILE *__stream),ftell,(__stream))
#endif /* !__USE_FILE_OFFSET64 */
#else /* __DOS_COMPAT__ */
__REDIRECT_FS_FUNC(__LIBC,,int,__LIBCCALL,fseeko,(__FILE *__stream, __FS_TYPE(off) __off, int __whence),fseeko,(__stream,__off,__whence))
__REDIRECT_FS_FUNC(__LIBC,__WUNUSED,__FS_TYPE(off),__LIBCCALL,ftello,(__FILE *__stream),ftello,(__stream))
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_LARGEFILE || __USE_XOPEN2K */
#ifdef __USE_XOPEN
#ifdef __CRT_GLC
__LIBC __PORT_NODOS char *(__LIBCCALL cuserid)(char *__buf);
#endif /* __CRT_GLC */
#endif /* __USE_XOPEN */
#ifdef __USE_KOS
/* For use with `format_printf()' and friends: Prints to a `FILE *' closure argument. */
#if !defined(__CRT_KOS) || (defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__))
__LOCAL __ssize_t (__LIBCCALL file_printer)(char const *__restrict __data,
                                            size_t __datalen, void *__closure) {
 return (__ssize_t)fwrite(__data,sizeof(char),__datalen,(FILE *)__closure);
}
#else
__LIBC __ssize_t (__LIBCCALL file_printer)(char const *__restrict __data,
                                           size_t __datalen, void *__closure);
#endif
#endif /* __USE_KOS */
#endif /* !__KERNEL__ */

#ifndef ____libc_vsnprintf_defined
#define ____libc_vsnprintf_defined 1
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_vsnprintf,(char *__restrict __buf, __size_t __buflen, char const *__restrict __format, __builtin_va_list __args),vsnprintf,(__buf,__buflen,__format,__args))
#ifndef ____dos_vscprintf_defined
#define ____dos_vscprintf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_vscprintf,(char const *__restrict __format, __builtin_va_list __args),_vscprintf,(__format,__args))
#endif /* !____dos_vsnprintf_defined */
__LOCAL int (__LIBCCALL __libc_vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) {
 /* Workaround for DOS's broken vsnprintf() implementation. */
 int __result = __dos_vsnprintf(__buf,__buflen,__format,__args);
 if (__result < 0) __result = __dos_vscprintf(__format,__args);
 return __result;
}
#else /* __DOS_COMPAT__ */
__REDIRECT(__LIBC,,int,__LIBCCALL,__libc_vsnprintf,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args),vsnprintf,(__buf,__buflen,__format,__args))
#endif /* !__DOS_COMPAT__ */
#endif /* !____libc_vsnprintf_defined */


__NAMESPACE_STD_BEGIN
#if defined(__USE_KOS) && ((defined(__CRT_KOS) && !defined(__GLC_COMPAT__) && !defined(__DOS_COMPAT__)) || __SIZEOF_SIZE_T__ <= __SIZEOF_INT__)
#ifndef __std_sprintf_defined
#define __std_sprintf_defined 1
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL sprintf)(char *__restrict __buf, char const *__restrict __format, ...);
#endif /* !__std_sprintf_defined */
__LIBC __ATTR_LIBC_PRINTF(2,0) __ssize_t (__LIBCCALL vsprintf)(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_SCANF(2,3) __size_t (__ATTR_CDECL sscanf)(char const *__restrict __buf, char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_UNIX98) || defined(__USE_DOS)
#ifdef __DOS_COMPAT__
__LOCAL __ATTR_LIBC_PRINTF(3,0) __ssize_t (__LIBCCALL vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { return (__ssize_t)__libc_vsnprintf(__buf,__buflen,__format,__args); }
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_LIBC_PRINTF(3,0) __ssize_t (__LIBCCALL vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_ISOC99 || __USE_UNIX98 || __USE_DOS */
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
#ifndef __std_snprintf_defined
#define __std_snprintf_defined 1
#ifdef __DOS_COMPAT__
__LOCAL __ATTR_LIBC_PRINTF(3,4) __ssize_t (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) {
 __ssize_t __result; __builtin_va_list __args; __builtin_va_start(__args,__format);
 __result = __libc_vsnprintf(__buf,__buflen,__format,__args);
 __builtin_va_end(__args);
 return __result;
}
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_LIBC_PRINTF(3,4) __ssize_t (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...);
#endif /* !__DOS_COMPAT__ */
#endif /* !__std_snprintf_defined */
#endif /* __USE_ISOC99 || __USE_UNIX98 */
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC __ATTR_LIBC_SCANF(2,0) __size_t (__LIBCCALL vsscanf)(char const *__restrict __buf, char const *__restrict __format, __builtin_va_list __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#else /* __USE_KOS */
#ifndef __std_sprintf_defined
#define __std_sprintf_defined 1
__LIBC __ATTR_LIBC_PRINTF(2,3) int (__ATTR_CDECL sprintf)(char *__restrict __buf, char const *__restrict __format, ...);
#endif /* !__std_sprintf_defined */
__LIBC __ATTR_LIBC_PRINTF(2,0) int (__LIBCCALL vsprintf)(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_SCANF(2,3) int (__ATTR_CDECL sscanf)(char const *__restrict __buf, char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_UNIX98) || defined(__USE_DOS)
#ifdef __DOS_COMPAT__
__LOCAL __ATTR_LIBC_PRINTF(3,0) int (__LIBCCALL vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { return __libc_vsnprintf(__buf,__buflen,__format,__args); }
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_LIBC_PRINTF(3,0) int (__LIBCCALL vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_ISOC99 || __USE_UNIX98 || __USE_DOS */
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
#ifndef __std_snprintf_defined
#define __std_snprintf_defined 1
#ifdef __DOS_COMPAT__
__LOCAL __ATTR_LIBC_PRINTF(3,4) int (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) {
 int __result; __builtin_va_list __args; __builtin_va_start(__args,__format);
 __result = __libc_vsnprintf(__buf,__buflen,__format,__args);
 __builtin_va_end(__args);
 return __result;
}
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_LIBC_PRINTF(3,4) int (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...);
#endif /* !__DOS_COMPAT__ */
#endif /* !__std_snprintf_defined */
#endif /* __USE_ISOC99 || __USE_UNIX98 */
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC __ATTR_LIBC_SCANF(2,0) int (__LIBCCALL vsscanf)(char const *__restrict __buf, char const *__restrict __format, __builtin_va_list __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#endif /* !__USE_KOS */
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
#ifndef __sprintf_defined
#define __sprintf_defined 1
__NAMESPACE_STD_USING(sprintf)
#endif /* !__sprintf_defined */
__NAMESPACE_STD_USING(vsprintf)
__NAMESPACE_STD_USING(sscanf)
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
#ifndef __snprintf_defined
#define __snprintf_defined 1
__NAMESPACE_STD_USING(snprintf)
#endif /* !__snprintf_defined */
#endif /* __USE_ISOC99 || __USE_UNIX98 */
#if defined(__USE_ISOC99) || defined(__USE_UNIX98) || defined(__USE_DOS)
__NAMESPACE_STD_USING(vsnprintf)
#endif /* __USE_ISOC99 || __USE_UNIX98 || __USE_DOS */
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__NAMESPACE_STD_USING(vsscanf)
#endif /* __USE_ISOC99 || __USE_DOS */
#endif /* !__CXX_SYSTEM_HEADER */


#ifdef __USE_GNU
#if !defined(__KERNEL__) && defined(__CRT_GLC)
#ifdef __USE_KOS
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS __WUNUSED __ssize_t (__ATTR_CDECL asprintf)(char **__restrict __pstr, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS __WUNUSED __ssize_t (__ATTR_CDECL __asprintf)(char **__restrict __pstr, char const *__restrict __format, ...) __ASMNAME("asprintf");
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_NODOS __WUNUSED __ssize_t (__LIBCCALL vasprintf)(char **__restrict __pstr, char const *__restrict __format, __builtin_va_list __args);
#else /* __USE_KOS */
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS __WUNUSED int (__ATTR_CDECL asprintf)(char **__restrict __pstr, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS __WUNUSED int (__ATTR_CDECL __asprintf)(char **__restrict __pstr, char const *__restrict __format, ...) __ASMNAME("asprintf");
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_NODOS __WUNUSED int (__LIBCCALL vasprintf)(char **__restrict __pstr, char const *__restrict __format, __builtin_va_list __args);
#endif /* !__USE_KOS */
#endif /* !__KERNEL__ && __CRT_GLC */
#endif /* __USE_GNU */

#if defined(__USE_XOPEN) && !defined(__USE_XOPEN2K) && \
   !defined(__USE_GNU) && defined(__CRT_GLC)
#include <getopt.h>
#endif

/* DOS Extensions */
#ifdef __USE_DOS
#ifndef _iobuf
#define _iobuf   __IO_FILE
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
#ifndef ___unlink_defined
#define ___unlink_defined 1
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,__NONNULL((1)),int,__LIBCCALL,_unlink,(char const *__name),unlink,(__name))
#endif /* !___unlink_defined */
#ifndef __unlink_defined
#define __unlink_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,unlink,(char const *__name),unlink,(__name))
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
__REDIRECT_IFKOS(__LIBC,,__FILE *,__LIBCCALL,_popen,(char const *__command, char const *__mode),popen,(__command,__mode))
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_pclose,(__FILE *__restrict __file),pclose,(__file))
__REDIRECT_IFKOS(__LIBC,,__FILE *,__LIBCCALL,_fdopen,(int __fd, char const *__restrict __mode),fdopen,(__fd,__modes))
#ifdef __LIBCCALL_CALLER_CLEANUP
__REDIRECT_IFKOS(__LIBC,,__FILE *,__LIBCCALL,_fsopen,(char const *__file, char const *__mode, int __shflag),fopen,(__file,__mode,__shflag))
#else /* __LIBCCALL_CALLER_CLEANUP */
__LOCAL __FILE *(__LIBCCALL _fsopen)(char const *__file, char const *__mode, int __UNUSED(__shflag)) { return __NAMESPACE_STD_SYM fopen(__file,__mode); }
#endif /* !__LIBCCALL_CALLER_CLEANUP */
#if defined(__GLC_COMPAT__) || !defined(__CRT_DOS)
__LOCAL int (__LIBCCALL _flushall)(void) { return fflush(NULL); }
#else /* __GLC_COMPAT__ || !__CRT_DOS */
__LIBC int (__LIBCCALL _flushall)(void);
#endif /* !__GLC_COMPAT__ && __CRT_DOS */
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_fcloseall,(void),fcloseall,())
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_fileno,(__FILE *__restrict __file),fileno,(__file))
__REDIRECT(__LIBC,,int,__LIBCCALL,_fgetchar,(void),getchar,())
__REDIRECT(__LIBC,,int,__LIBCCALL,_fputchar,(int __ch),putchar,(__ch))
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_getw,(__FILE *__restrict __file),getw,(__file))
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_putw,(int __w, __FILE *__restrict __file),putw,(__w,__file))
#ifdef __USE_DOSFS
__LIBC __ATTR_MALLOC char *(__LIBCCALL _tempnam)(char const *__dir, char const *__pfx);
#else /* __USE_DOSFS */
__REDIRECT(__LIBC,__ATTR_MALLOC,char *,__LIBCCALL,_tempnam,(char const *__dir, char const *__pfx),tempnam,(__dir,__pfx))
#endif /* !__USE_DOSFS */
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_fseeki64,(__FILE *__restrict __file, __INT64_TYPE__ __off, int __whence),fseeko64,(__file,__off,__whence))
__REDIRECT_IFKOS(__LIBC,,__INT64_TYPE__,__LIBCCALL,_ftelli64,(__FILE *__restrict __file),ftello64,(__file))

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY int (__LIBCCALL _rmtmp)(void);
__LIBC __PORT_DOSONLY int (__LIBCCALL _filbuf)(__FILE *__restrict __file);
__LIBC __PORT_DOSONLY int (__LIBCCALL _flsbuf)(int __ch, __FILE *__restrict __file);

__LIBC __PORT_DOSONLY int (__LIBCCALL _getmaxstdio)(void);
__LIBC __PORT_DOSONLY int (__LIBCCALL _setmaxstdio)(int __val);
__LIBC __PORT_DOSONLY int (__LIBCCALL _set_printf_count_output)(int __val);
__LIBC __PORT_DOSONLY int (__LIBCCALL _get_printf_count_output)(void);
__LIBC __PORT_DOSONLY __UINT32_TYPE__ (__LIBCCALL _set_output_format)(__UINT32_TYPE__ __format);
__LIBC __PORT_DOSONLY __UINT32_TYPE__ (__LIBCCALL _get_output_format)(void);

__LIBC int (__ATTR_CDECL _fscanf_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _scanf_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _sscanf_l)(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _snscanf)(char const *__restrict __src, size_t __buflen, char const *__restrict __format, ...);
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _snscanf_l)(char const *__restrict __src, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
#ifdef __CRT_KOS
__LIBC int (__ATTR_CDECL _fscanf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_fscanf_l");
__LIBC int (__ATTR_CDECL _scanf_s_l)(char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_scanf_l");
__LIBC int (__ATTR_CDECL _sscanf_s_l)(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_sscanf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _snscanf_s)(char const *__restrict __src, size_t __buflen, char const *__restrict __format, ...) __ASMNAME("_snscanf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _snscanf_s_l)(char const *__restrict __src, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_snscanf_l");
#else /* __CRT_KOS */
__LIBC int (__ATTR_CDECL _fscanf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _scanf_s_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _sscanf_s_l)(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _snscanf_s)(char const *__restrict __src, size_t __buflen, char const *__restrict __format, ...);
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _snscanf_s_l)(char const *__restrict __src, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
#endif /* !__CRT_KOS */

__LIBC int (__ATTR_CDECL _sprintf_l)(char *__restrict __buf, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsprintf_l)(char *__restrict __buf, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _sprintf_p)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vsprintf_p)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _sprintf_p_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsprintf_p_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale,  __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _sprintf_s_l)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsprintf_s_l)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);

#ifndef ___scprintf_defined
#define ___scprintf_defined 1
__LIBC int (__ATTR_CDECL _scprintf)(char const *__restrict __format, ...);
#endif /* !___scprintf_defined */
__LIBC int (__LIBCCALL _vscprintf)(char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _scprintf_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vscprintf_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#ifdef __CRT_KOS
__LIBC int (__ATTR_CDECL _scprintf_p)(char const *__restrict __format, ...) __ASMNAME("_scprintf");
__LIBC int (__LIBCCALL _vscprintf_p)(char const *__restrict __format, __builtin_va_list __args) __ASMNAME("_vscprintf");
__LIBC int (__ATTR_CDECL _scprintf_p_l)(char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_scprintf_l");
__LIBC int (__LIBCCALL _vscprintf_p_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME("_vscprintf_l");
#else /* __CRT_KOS */
__LIBC int (__ATTR_CDECL _scprintf_p)(char const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vscprintf_p)(char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _scprintf_p_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vscprintf_p_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#endif /* !__CRT_KOS */

/* The following 2 return an error, rather than the required size when the buffer is too small */
__LIBC int (__ATTR_CDECL _snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _snprintf_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsnprintf_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _snprintf_c_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsnprintf_c_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _snprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vsnprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _snprintf_s_l)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vsnprintf_s_l)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC int (__LIBCCALL _vsnprintf_c)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _snprintf_c)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...);

__LIBC int (__ATTR_CDECL _printf_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vprintf_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#ifdef __CRT_KOS
__LIBC int (__ATTR_CDECL _printf_p)(char const *__restrict __format, ...) __ASMNAME("printf");
__LIBC int (__LIBCCALL _vprintf_p)(char const *__restrict __format, __builtin_va_list __args) __ASMNAME("vprintf");
__LIBC int (__ATTR_CDECL _printf_p_l)(char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_printf_l");
__LIBC int (__LIBCCALL _vprintf_p_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME("_vprintf_l");
__LIBC int (__ATTR_CDECL _printf_s_l)(char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_printf_l");
__LIBC int (__LIBCCALL _vprintf_s_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME("_vprintf_l");
#else /* __CRT_KOS */
__LIBC int (__ATTR_CDECL _printf_p)(char const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vprintf_p)(char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _printf_p_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vprintf_p_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _printf_s_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vprintf_s_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#endif /* !__CRT_KOS */

__LIBC int (__ATTR_CDECL _fprintf_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vfprintf_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#ifdef __CRT_KOS
__LIBC int (__ATTR_CDECL _fprintf_p)(__FILE *__restrict __file, char const *__restrict __format, ...) __ASMNAME("fprintf");
__LIBC int (__LIBCCALL _vfprintf_p)(__FILE *__restrict __file, char const *__restrict __format, __builtin_va_list __args) __ASMNAME("vfprintf");
__LIBC int (__ATTR_CDECL _fprintf_p_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_fprintf_l");
__LIBC int (__LIBCCALL _vfprintf_p_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME("_vfprintf_l");
__LIBC int (__ATTR_CDECL _fprintf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) __ASMNAME("_fprintf_l");
__LIBC int (__LIBCCALL _vfprintf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME("_vfprintf_l");
#else /* __CRT_KOS */
__LIBC int (__ATTR_CDECL _fprintf_p)(__FILE *__restrict __file, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL _vfprintf_p)(__FILE *__restrict __file, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _fprintf_p_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vfprintf_p_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _fprintf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vfprintf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#endif /* !__CRT_KOS */
#else /* __CRT_DOS */
__LOCAL int (__ATTR_CDECL _fscanf_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _fscanf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _scanf_l)(char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vscanf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _scanf_s_l)(char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vscanf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _sscanf_l)(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vsscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _sscanf_s_l)(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vsscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__LIBCCALL _vsprintf_l)(char *__restrict __buf, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vsprintf(__buf,__format,__args); }
__LOCAL int (__LIBCCALL _vsprintf_p)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { return vsnprintf(__buf,__buflen,__format,__args); }
__LOCAL int (__LIBCCALL _vsprintf_p_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __UNUSED(__locale),  __builtin_va_list __args) { return __libc_vsnprintf(__buf,__buflen,__format,__args); }
__LOCAL int (__LIBCCALL _vsprintf_s_l)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return __libc_vsnprintf(__buf,__bufsize,__format,__args); }
__LOCAL int (__LIBCCALL _vscprintf)(char const *__restrict __format, __builtin_va_list __args) { return __libc_vsnprintf(NULL,0,__format,__args); }
__LOCAL int (__LIBCCALL _vscprintf_p)(char const *__restrict __format, __builtin_va_list __args) { return __libc_vsnprintf(NULL,0,__format,__args); }
__LOCAL int (__LIBCCALL _vscprintf_l)(char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return __libc_vsnprintf(NULL,0,__format,__args); }
__LOCAL int (__LIBCCALL _vscprintf_p_l)(char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return __libc_vsnprintf(NULL,0,__format,__args); }
__LOCAL int (__ATTR_CDECL _sprintf_l)(char *__restrict __buf, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = sprintf(__buf,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _sprintf_p)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = snprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _sprintf_p_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = snprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _sprintf_s_l)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = snprintf(__buf,__bufsize,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _scprintf)(char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vscprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _scprintf_p)(char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vscprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _scprintf_l)(char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vscprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _scprintf_p_l)(char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vscprintf(__format,__args); __builtin_va_end(__args); return __result; }

__LOCAL int (__LIBCCALL _vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { int __result = __libc_vsnprintf(__buf,__buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL int (__LIBCCALL _vsnprintf_c)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { int __result = __libc_vsnprintf(__buf,__buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL int (__LIBCCALL _vsnprintf_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { int __result = __libc_vsnprintf(__buf,__buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL int (__LIBCCALL _vsnprintf_c_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { int __result = __libc_vsnprintf(__buf,__buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL int (__LIBCCALL _vsnprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { int __result = __libc_vsnprintf(__buf,__bufsize < __buflen ? __bufsize : __buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL int (__LIBCCALL _vsnprintf_s_l)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { int __result = __libc_vsnprintf(__buf,__bufsize < __buflen ? __bufsize : __buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL int (__ATTR_CDECL _snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vsnprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _snprintf_c)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vsnprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _snprintf_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vsnprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _snprintf_c_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vsnprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _snprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vsnprintf_s(__buf,__bufsize,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _snprintf_s_l)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vsnprintf_s(__buf,__bufsize,__buflen,__format,__args); __builtin_va_end(__args); return __result; }

__LIBC int (__LIBCCALL _vprintf_p)(char const *__restrict __format, __builtin_va_list __args) { return vprintf(__format,__args); }
__LIBC int (__LIBCCALL _vprintf_l)(char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vprintf(__format,__args); }
__LIBC int (__LIBCCALL _vprintf_p_l)(char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vprintf(__format,__args); }
__LIBC int (__LIBCCALL _vprintf_s_l)(char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vprintf(__format,__args); }
__LIBC int (__ATTR_CDECL _printf_p)(char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LIBC int (__ATTR_CDECL _printf_l)(char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LIBC int (__ATTR_CDECL _printf_p_l)(char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LIBC int (__ATTR_CDECL _printf_s_l)(char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vprintf(__format,__args); __builtin_va_end(__args); return __result; }

__LIBC int (__LIBCCALL _vfprintf_p)(__FILE *__restrict __file, char const *__restrict __format, __builtin_va_list __args) { return vfprintf(__file,__format,__args); }
__LIBC int (__LIBCCALL _vfprintf_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vfprintf(__file,__format,__args); }
__LIBC int (__LIBCCALL _vfprintf_p_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vfprintf(__file,__format,__args); }
__LIBC int (__LIBCCALL _vfprintf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vfprintf(__file,__format,__args); }
__LIBC int (__ATTR_CDECL _fprintf_p)(__FILE *__restrict __file, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vfprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LIBC int (__ATTR_CDECL _fprintf_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LIBC int (__ATTR_CDECL _fprintf_p_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LIBC int (__ATTR_CDECL _fprintf_s_l)(__FILE *__restrict __file, char const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__CRT_DOS */

#ifdef __USE_DOS_SLIB
#ifdef __CRT_DOS
/* WARNING: `fopen_s' and `freopen_s' returns DOS error codes in DOS-FS mode! */
__REDIRECT_UFS(__LIBC,__PORT_DOSONLY_ALT(fopen),errno_t,__LIBCCALL,fopen_s,(__FILE **__pfile, char const *__file, char const *__mode),fopen_s,(__pfile,__file,__mode))
__REDIRECT_UFS(__LIBC,__PORT_DOSONLY_ALT(freopen),errno_t,__LIBCCALL,freopen_s,(__FILE **__pfile, char const *__file, char const *__mode, __FILE *__oldfile),freopen_s,(__pfile,__file,__mode,__oldfile))
__REDIRECT_UFS(__LIBC,__PORT_DOSONLY_ALT(tmpnam),errno_t,__LIBCCALL,tmpnam_s,(char *__restrict __buf, rsize_t __bufsize),tmpnam_s,(__buf,__bufsize))
__LIBC __PORT_DOSONLY_ALT(clearerr) errno_t (__LIBCCALL clearerr_s)(__FILE *__restrict __file);
__LIBC __PORT_DOSONLY_ALT(tmpfile) errno_t (__LIBCCALL tmpfile_s)(__FILE **__pfile);
__LIBC size_t (__LIBCCALL fread_s)(void *__buf, size_t __bufsize, size_t __elemsize, size_t __elemcount, __FILE *__restrict __file);
__LIBC char *(__LIBCCALL gets_s)(char *__restrict __buf, rsize_t __bufsize);

#ifdef __CRT_KOS
__LIBC int (__ATTR_CDECL printf_s)(char const *__restrict __format, ...) __ASMNAME("printf");
__LIBC int (__LIBCCALL vprintf_s)(char const *__restrict __format, __builtin_va_list __args) __ASMNAME("vprintf");
__LIBC int (__ATTR_CDECL fprintf_s)(__FILE *__restrict __file, char const *__restrict __format, ...) __ASMNAME("fprintf");
__LIBC int (__LIBCCALL vfprintf_s)(__FILE *__restrict __file, char const *__restrict __format, __builtin_va_list __args) __ASMNAME("vfprintf");
__LIBC int (__ATTR_CDECL sprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, ...) __ASMNAME("snprintf");
__LIBC int (__LIBCCALL vsprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __builtin_va_list __args) __ASMNAME("vsnprintf");
__LIBC int (__ATTR_CDECL fscanf_s)(__FILE *__restrict __file, char const *__restrict __format, ...) __ASMNAME("fscanf");
__LIBC int (__LIBCCALL vfscanf_s)(__FILE *__restrict __file, char const *__restrict __format, __builtin_va_list __args) __ASMNAME("vfscanf");
__LIBC int (__ATTR_CDECL scanf_s)(char const *__restrict __format, ...) __ASMNAME("scanf");
__LIBC int (__LIBCCALL vscanf_s)(char const *__restrict __format, __builtin_va_list __args) __ASMNAME("vscanf");
__LIBC int (__ATTR_CDECL sscanf_s)(char const *__restrict __src, char const *__restrict __format, ...) __ASMNAME("sscanf");
__LIBC int (__LIBCCALL vsscanf_s)(char const *__restrict __src, char const *__restrict __format, __builtin_va_list __args) __ASMNAME("vsscanf");
#else /* __CRT_KOS */
__LIBC int (__ATTR_CDECL printf_s)(char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vprintf_s)(char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL fprintf_s)(__FILE *__restrict __file, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vfprintf_s)(__FILE *__restrict __file, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL sprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vsprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL fscanf_s)(__FILE *__restrict __file, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vfscanf_s)(__FILE *__restrict __file, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL scanf_s)(char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vscanf_s)(char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL sscanf_s)(char const *__restrict __src, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vsscanf_s)(char const *__restrict __src, char const *__restrict __format, __builtin_va_list __args);
#endif /* !__CRT_KOS */
__LIBC int (__LIBCCALL vsnprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __builtin_va_list __args);
#else /* __CRT_DOS */
__LOCAL size_t (__LIBCCALL fread_s)(void *__buf, size_t __bufsize, size_t __elemsize, size_t __elemcount, __FILE *__restrict __file) { __bufsize /= __elemsize; return fread(__buf,__elemsize,__bufsize < __elemcount ? __bufsize : __elemcount,__file); }
__LOCAL char *(__LIBCCALL gets_s)(char *__restrict __buf, rsize_t __bufsize) { return fgets(__buf,__bufsize,stdin); }

__LOCAL int (__LIBCCALL vprintf_s)(char const *__restrict __format, __builtin_va_list __args) { return vprintf(__format,__args); }
__LOCAL int (__LIBCCALL vfprintf_s)(__FILE *__restrict __file, char const *__restrict __format, __builtin_va_list __args) { return vfprintf(__file,__format,__args); }
__LOCAL int (__LIBCCALL vsprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __builtin_va_list __args) { return __libc_vsnprintf(__buf,__bufsize,__format,__args); }
__LOCAL int (__LIBCCALL vsnprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { return __libc_vsnprintf(__buf,__bufsize < __buflen ? __bufsize : __buflen,__format,__args); }
__LOCAL int (__LIBCCALL vfscanf_s)(__FILE *__restrict __file, char const *__restrict __format, __builtin_va_list __args) { return vfscanf(__file,__format,__args); }
__LOCAL int (__LIBCCALL vscanf_s)(char const *__restrict __format, __builtin_va_list __args) { return vscanf(__format,__args); }
__LOCAL int (__LIBCCALL vsscanf_s)(char const *__restrict __src, char const *__restrict __format, __builtin_va_list __args) { return vsscanf(__src,__format,__args); }
__LOCAL int (__ATTR_CDECL printf_s)(char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL fprintf_s)(__FILE *__restrict __file, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vfprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL sprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = __libc_vsnprintf(__buf,__bufsize,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL fscanf_s)(__FILE *__restrict __file, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vfscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL scanf_s)(char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vscanf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL sscanf_s)(char const *__restrict __src, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vsscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__CRT_DOS */

#endif /* __USE_DOS_SLIB */

#ifdef __CRT_DOS
#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED 1
__REDIRECT_IFW32_VOID(__LIBC,__PORT_DOSONLY __ATTR_COLD,__LIBCCALL,_wperror,
                     (wchar_t const *__restrict __errmsg),wperror,(__errmsg))
#endif /* !_CRT_WPERROR_DEFINED */
#endif /* __CRT_DOS */

#ifndef _WSTDIO_DEFINED
#define _WSTDIO_DEFINED 1

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */

#ifndef WEOF
#if __SIZEOF_WCHAR_T__ == 4
#   define WEOF             0xffffffffu
#else
#   define WEOF    (wint_t)(0xffff)
#endif
#endif /* !WEOF */

#ifndef __std_wprintf_defined
#define __std_wprintf_defined 1
__NAMESPACE_STD_BEGIN
#if defined(__USE_KOS) && (__SIZEOF_SIZE_T__ <= __SIZEOF_INT__ || defined(__CRT_KOS))
__LIBC __NONNULL((1,2)) __ssize_t (__ATTR_CDECL fwprintf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __NONNULL((1)) __ssize_t (__ATTR_CDECL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC __NONNULL((1,2)) __ssize_t (__LIBCCALL vfwprintf)(__FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __NONNULL((1)) __ssize_t (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __NONNULL((1,2)) __ssize_t (__ATTR_CDECL fwscanf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __NONNULL((1)) __ssize_t (__ATTR_CDECL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC __NONNULL((1,2)) __ssize_t (__ATTR_CDECL swscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) __PE_ASMNAME("_swscanf");
__REDIRECT_IFW16(__LIBC,__NONNULL((1,3)),__ssize_t,__LIBCCALL,vswprintf,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args),_vswprintf_c,(__buf,__buflen,__format,__args))
#ifndef __NO_ASMNAME
#if __SIZEOF_WCHAR_T__ == 2 && defined(__CRT_DOS)
__LIBC __NONNULL((1,3)) __ssize_t (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME("_swprintf_c");
#else /* __SIZEOF_WCHAR_T__ == 2 && __CRT_DOS */
__LIBC __NONNULL((1,3)) __ssize_t (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
#endif /* __SIZEOF_WCHAR_T__ != 2 || !__CRT_DOS */
#else /* !__NO_ASMNAME */
__LOCAL __NONNULL((1,3)) __ssize_t (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { __builtin_va_list __args; __ssize_t __result; __builtin_va_start(__args,__format); __result = vswprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* __NO_ASMNAME */
#else /* __USE_KOS */
__LIBC __NONNULL((1,2)) int (__ATTR_CDECL fwprintf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __NONNULL((1)) int (__ATTR_CDECL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC __NONNULL((1,2)) int (__LIBCCALL vfwprintf)(__FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __NONNULL((1)) int (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __NONNULL((1,2)) int (__ATTR_CDECL fwscanf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __NONNULL((1)) int (__ATTR_CDECL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC __NONNULL((1,2)) int (__ATTR_CDECL swscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) __PE_ASMNAME("_swscanf");
__REDIRECT_IFW16(__LIBC,__NONNULL((1,3)),int,__LIBCCALL,vswprintf,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args),_vswprintf_c,(__buf,__buflen,__format,__args))
#ifndef __NO_ASMNAME
#if __SIZEOF_WCHAR_T__ == 2 && defined(__CRT_DOS)
__LIBC __NONNULL((1,3)) int (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME("_swprintf_c");
#else /* __SIZEOF_WCHAR_T__ == 2 && __CRT_DOS */
__LIBC __NONNULL((1,3)) int (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
#endif /* __SIZEOF_WCHAR_T__ != 2 || !__CRT_DOS */
#else /* !__NO_ASMNAME */
__LOCAL __NONNULL((1,3)) int (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { __builtin_va_list __args; int __result; __builtin_va_start(__args,__format); __result = vswprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* __NO_ASMNAME */
#endif /* !__USE_KOS */
__NAMESPACE_STD_END
#endif /* !__std_wprintf_defined */

#ifndef __std_vwscanf_defined
#define __std_vwscanf_defined 1
__NAMESPACE_STD_BEGIN
#ifdef __USE_KOS
__LIBC __NONNULL((1,2)) __ssize_t (__LIBCCALL vfwscanf)(__FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __NONNULL((1)) __ssize_t (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __NONNULL((1,2)) __ssize_t (__LIBCCALL vswscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __builtin_va_list __arg);
#else /* __USE_KOS */
__LIBC __NONNULL((1,2)) int (__LIBCCALL vfwscanf)(__FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __NONNULL((1)) int (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __NONNULL((1,2)) int (__LIBCCALL vswscanf)(wchar_t const *__restrict __rc, wchar_t const *__restrict __format, __builtin_va_list __arg);
#endif /* !__USE_KOS */
__NAMESPACE_STD_END
#endif /* !__std_vwscanf_defined */

#ifndef __CXX_SYSTEM_HEADER
#ifndef __wprintf_defined
#define __wprintf_defined 1
__NAMESPACE_STD_USING(fwprintf)
__NAMESPACE_STD_USING(wprintf)
__NAMESPACE_STD_USING(swprintf)
__NAMESPACE_STD_USING(vfwprintf)
__NAMESPACE_STD_USING(vwprintf)
__NAMESPACE_STD_USING(vswprintf)
__NAMESPACE_STD_USING(fwscanf)
__NAMESPACE_STD_USING(wscanf)
__NAMESPACE_STD_USING(swscanf)
#endif /* !__wprintf_defined */
#ifndef __vwscanf_defined
#define __vwscanf_defined 1
__NAMESPACE_STD_USING(vfwscanf)
__NAMESPACE_STD_USING(vwscanf)
__NAMESPACE_STD_USING(vswscanf)
#endif /* !__vwscanf_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __CRT_DOS
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,_wfsopen,(wchar_t const *__file, wchar_t const *__mode, int __shflag),_wfsopen,(__file,__mode,__shflag))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,_wfopen,(wchar_t const *__file, wchar_t const *__mode),_wfopen,(__file,__mode))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,_wfreopen,(wchar_t const *__file, wchar_t const *__mode, FILE *__oldfile),_wfreopen,(__file,__mode,__oldfile))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wfopen_s,(FILE **__pfile, wchar_t const *__file, wchar_t const *__mode),_wfopen_s,(__pfile,__file,__mode))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wfreopen_s,(FILE **__pfile, wchar_t const *__file, wchar_t const *__mode, FILE *__oldfile),_wfreopen_s,(__pfile,__file,__mode,__oldfile))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,_wfdopen,(int __fd, wchar_t const *__mode),wfdopen,(__fd,__mode))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,_wpopen,(wchar_t const *__command, wchar_t const *__mode),wpopen,(__command,__mode))
#endif /* __CRT_DOS */

/* Get wide character functions */
__REDIRECT(__LIBC,,wint_t,__LIBCCALL,_fgetwchar,(void),getwchar,())
__REDIRECT_IFKOS(__LIBC,__NONNULL((1)),wint_t,__LIBCCALL,_fgetwc_nolock,(FILE *__restrict __file),fgetwc_unlocked,(__file))
#ifndef __std_getwchar_defined
#define __std_getwchar_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wint_t (__LIBCCALL getwchar)(void);
__LIBC __NONNULL((1)) wint_t (__LIBCCALL fgetwc)(FILE *__restrict __file);
__LIBC __NONNULL((1)) wint_t (__LIBCCALL getwc)(FILE *__restrict __file);
__NAMESPACE_STD_END
#endif /* !__std_getwchar_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __getwchar_defined
#define __getwchar_defined 1
__NAMESPACE_STD_USING(getwchar)
__NAMESPACE_STD_USING(fgetwc)
__NAMESPACE_STD_USING(getwc)
#endif /* !__getwchar_defined */
#endif /* !__CXX_SYSTEM_HEADER */

/* Put wide character functions */
__REDIRECT(__LIBC,,wint_t,__LIBCCALL,_fputwchar,(wchar_t __ch),putwchar,(__ch))
__REDIRECT_IFKOS(__LIBC,,wint_t,__LIBCCALL,_fputwc_nolock,(wchar_t __ch, FILE *__restrict __file),fputwc_unlocked,(__ch,__file))
#ifndef __std_putwchar_defined
#define __std_putwchar_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wint_t (__LIBCCALL putwchar)(wchar_t __ch);
__LIBC __NONNULL((2)) wint_t (__LIBCCALL fputwc)(wchar_t __ch, FILE *__restrict __file);
__LIBC __NONNULL((2)) wint_t (__LIBCCALL putwc)(wchar_t __ch, FILE *__restrict __file);
__NAMESPACE_STD_END
#endif /* !__std_putwchar_defined */
#ifndef __putwchar_defined
#define __putwchar_defined 1
__NAMESPACE_STD_USING(putwchar)
__NAMESPACE_STD_USING(fputwc)
__NAMESPACE_STD_USING(putwc)
#endif /* !__putwchar_defined */

/* Unget character functions */
__REDIRECT_IFKOS(__LIBC,,wint_t,__LIBCCALL,_ungetwc_nolock,(wint_t __ch, FILE *__restrict __file),ungetwc_unlocked,(__ch,__file))
#ifndef __std_ungetwc_defined
#define __std_ungetwc_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wint_t (__LIBCCALL ungetwc)(wint_t __ch, FILE *__restrict __file);
__NAMESPACE_STD_END
#endif /* !__std_ungetwc_defined */
#ifndef __ungetwc_defined
#define __ungetwc_defined 1
__NAMESPACE_STD_USING(ungetwc)
#endif /* !__ungetwc_defined */

/* Get wide string functions */
#ifdef __CRT_DOS
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_getws,(wchar_t *__restrict __buf),getws,(__buf))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_getws_s,(wchar_t *__restrict __buf, size_t __buflen),getws_s,(__buf,__buflen))
#endif /* __CRT_DOS */

#ifndef __std_fgetws_defined
#define __std_fgetws_defined 1
__NAMESPACE_STD_BEGIN
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
/* In PE-mode, we don't export the size_t version */
#if defined(__PE__) || !defined(__CRT_KOS) || \
   (defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__))
#ifdef __NO_ASMNAME
__LIBC __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream);
#define fgetws(buf,n,stream) fgetws(buf,(int)(n),stream)
#else /* __NO_ASMNAME */
__REDIRECT(__LIBC,,wchar_t *,__LIBCCALL,__pe_fgetws,(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream),fgetws,(__buf,__n,__stream))
__LOCAL __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) { return __pe_fgetws(__ws,(int)__n,__stream); }
#endif /* !__NO_ASMNAME */
#else /* ... */
__REDIRECT(__LIBC,__NONNULL((1,3)),wchar_t *,__LIBCCALL,fgetws,(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream),fgetws_sz,(__buf,__n,__stream))
#endif /* !... */
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__LIBC __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream);
#endif /* !__USE_KOS */
__NAMESPACE_STD_END
#endif /* !__std_fgetws_defined */
#ifndef __fgetws_defined
#define __fgetws_defined 1
__NAMESPACE_STD_USING(fgetws)
#endif /* !__fgetws_defined */

/* Put wide string functions */
__REDIRECT_IFKOS(__LIBC,__NONNULL((1)),int,__LIBCCALL,_putws,(wchar_t const *__restrict __str),putws,(__str))
#ifndef __std_fputws_defined
#define __std_fputws_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __NONNULL((1,2)) int (__LIBCCALL fputws)(wchar_t const *__restrict __str, FILE *__restrict __file);
__NAMESPACE_STD_END
#endif /* !__std_fputws_defined */
#ifndef __fputws_defined
#define __fputws_defined 1
__NAMESPACE_STD_USING(fputws)
#endif /* !__fputws_defined */

__LIBC __PORT_DOSONLY int (__ATTR_CDECL _scwprintf)(wchar_t const *__restrict __format, ...) __ASMNAME_IFKOS("scwprintf");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vscwprintf)(wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME_IFKOS("vscwprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _swprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME_IFKOS("swprintf");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vswprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME_IFKOS("vswprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _fwprintf_p)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwprintf");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vfwprintf_p)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vfwprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _wprintf_p)(wchar_t const *__restrict __format, ...) __ASMNAME("wprintf");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vwprintf_p)(wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vwprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _swprintf_p)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME2("swprintf","_swprintf_c");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vswprintf_p)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME2("vswprintf","_vswprintf_c");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _scwprintf_p)(wchar_t const *__restrict __format, ...) __ASMNAME2("scwprintf","_scwprintf");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vscwprintf_p)(wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME2("vscwprintf","_vscwprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _wprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("wprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME_IFKOS("vwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _wprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("wprintf_l","_wprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME2("vwprintf_l","_vwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _wprintf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("wprintf_l","_wprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vwprintf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME2("vwprintf_l","_vwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _fwprintf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("fwprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vfwprintf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME_IFKOS("vfwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _fwprintf_p_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("fwprintf_l","_fwprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vfwprintf_p_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME2("vfwprintf_l","_vfwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _fwprintf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("fwprintf_l","_fwprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vfwprintf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME2("vfwprintf_l","_vfwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _swprintf_c_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("swprintf_c_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vswprintf_c_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME_IFKOS("vswprintf_c_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _swprintf_p_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swprintf_c_l","_swprintf_c_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vswprintf_p_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME2("vswprintf_c_l","_vswprintf_c_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _swprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swprintf_c_l","_swprintf_c_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vswprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME2("vswprintf_c_l","_vswprintf_c_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _scwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("scwprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vscwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME_IFKOS("vscwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _scwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("scwprintf_l","_scwprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vscwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME2("vscwprintf_l","_vscwprintf_l");
#ifdef __PE__
/* The following return an error, rather than the required size when the buffer is too small */
__LIBC __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, ...);
__LIBC __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#else /* __PE__ */
/* Outside of DOS-mode, libc doesn't export DOS's broken wide-string printer functions, so we emulate them here. */
__LOCAL __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args) { size_t __result = vswprintf(__buf,__buflen,__format,__args); return __result < __buflen ? (int)__result : -1; }
__LOCAL __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return _vsnwprintf(__buf,__buflen,__format,__args); }
__LOCAL __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __builtin_va_list __args) { return _vsnwprintf(__buf,__buflen < __maxlen ? __buflen : __maxlen,__format,__args); }
__LOCAL __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return _vsnwprintf_s(__buf,__buflen,__maxlen,__format,__args); }
__LOCAL __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vsnwprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vsnwprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vsnwprintf_s(__buf,__buflen,__maxlen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vsnwprintf_s(__buf,__buflen,__maxlen,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__PE__ */


/* NOTE: ~safe~ functions are re-directed to the regular versions. (For the reason, see below) */
#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, ...) __ASMNAME_IFKOS("snwscanf"); /* No varargs version. */
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf_l)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("snwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _fwscanf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("fwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _swscanf_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("swscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _wscanf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("wscanf_l"); /* No varargs version. */
#ifdef __USE_DOS
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf_s)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, ...) __ASMNAME_IFKOS("snwscanf"); /* No varargs version. */
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf_s_l)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("snwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _fwscanf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("fwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _swscanf_s_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("swscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _wscanf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("wscanf_l"); /* No varargs version. */
#else /* __USE_DOS */
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf_s)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, ...) __ASMNAME2("snwscanf","_snwscanf"); /* No varargs version. */
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf_s_l)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("snwscanf_l","_snwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _fwscanf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("fwscanf_l","_fwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _swscanf_s_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swscanf_l","_swscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _wscanf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("wscanf_l","_wscanf_l"); /* No varargs version. */
#endif /* !__USE_DOS */
#else /* __CRT_DOS */
__LOCAL int (__ATTR_CDECL _fwscanf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _fwscanf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _swscanf_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vswscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _swscanf_s_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vswscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _wscanf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vwscanf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _wscanf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vwscanf(__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__CRT_DOS */

#ifdef __USE_DOS_SLIB
/* Simply redirect these so-called ~safe~ functions to the regular version.
 * In KOS, they're already ~safe~ to begin with, because unknown format strings are always handled.
 * NOTE: For binary compatibility, assembly names such as `fwprintf_s' are exported as alias,
 *       but should never be used by newly compiled applications. */
#ifndef __NO_ASMNAME
__LIBC int (__ATTR_CDECL fwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwprintf");
__LIBC int (__LIBCCALL vfwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vfwprintf");
__LIBC int (__ATTR_CDECL wprintf_s)(wchar_t const *__restrict __format, ...) __ASMNAME("wprintf");
__LIBC int (__LIBCCALL vwprintf_s)(wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vwprintf");
__LIBC int (__ATTR_CDECL fwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwscanf");
__LIBC int (__LIBCCALL vfwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vfwscanf");
__LIBC int (__ATTR_CDECL wscanf_s)(wchar_t const *__restrict __format, ...) __ASMNAME("wscanf");
__LIBC int (__LIBCCALL vwscanf_s)(wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vwscanf");
__LIBC int (__ATTR_CDECL swprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME2("swprintf","_swprintf_c");
__LIBC int (__LIBCCALL vswprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME2("vswprintf","_vswprintf_c");
__LIBC int (__ATTR_CDECL swscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) __ASMNAME("swscanf");
__LIBC int (__LIBCCALL vswscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vswscanf");
#else /* !__NO_ASMNAME */
__LOCAL int (__LIBCCALL vfwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args) { return vfwprintf(__file,__format,__args); }
__LOCAL int (__LIBCCALL vwprintf_s)(wchar_t const *__restrict __format, __builtin_va_list __args) { return vwprintf(__format,__args); }
__LOCAL int (__LIBCCALL vfwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args) { return vfwscanf(__file,__format,__args); }
__LOCAL int (__LIBCCALL vwscanf_s)(wchar_t const *__restrict __format, __builtin_va_list __args) { return vwscanf(__format,__args); }
__LOCAL int (__LIBCCALL vswprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args) { return vswprintf(__buf,__buflen,__format,__args); }
__LOCAL int (__LIBCCALL vswscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __builtin_va_list __args) { return vswscanf(__src,__format,__args); }
__LOCAL int (__ATTR_CDECL fwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vfwprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL wprintf_s)(wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vwprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL fwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vfwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL wscanf_s)(wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL swprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vswprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL swscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vswscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* __NO_ASMNAME */
#endif /* __USE_DOS_SLIB */

#ifdef __CRT_DOS
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wtmpnam,(wchar_t *__restrict __buf),_wtmpnam,(__buf))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wtmpnam_s,(wchar_t *__restrict __buf, size_t __buflen),_wtmpnam_s,(__buf,__buflen))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wtempnam,(wchar_t const *__dir, wchar_t const *__pfx),_wtempnam,(__dir,__pfx))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wremove,(wchar_t const *__restrict __file),_wremove,(__file))
#endif /* __CRT_DOS */

#if defined(__PE__) && defined(__CRT_DOS)
/* Versions lacking the C standard mandated BUFLEN argument...
 * NOTE: Internally, these functions will link against `.dos._swprintf' and `.dos._vswprintf' */
__LIBC int (__LIBCCALL _vswprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _swprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, ...);
#else /* __PE__ && __CRT_DOS */
/* Outside of PE-mode, wchar_t is 32 bits wide and `.dos.' isn't inserted before symbol names. */
/* libc doesn't export these superfluous and confusion version of swprintf.
 * (They're lacking the BUFLEN argument mandated by the C standard).
 * So instead, they're implemented as a hack. */
__LOCAL int (__LIBCCALL _vswprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __builtin_va_list __args) { return vswprintf(__buf,(size_t)-1,__format,__args); }
__LOCAL int (__ATTR_CDECL _swprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, ...) { __builtin_va_list __args; int __result; __builtin_va_start(__args,__format); __result = _vswprintf(__buf,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__PE__ || !__CRT_DOS */

#if defined(__PE__) && defined(__CRT_DOS) /* Unlimited locale wide-string printers (Only defined for DOS mode) */
__LIBC int (__LIBCCALL __vswprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL __swprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __locale, ...);
#else /* In KOS mode, we emulate these. */
__LOCAL int (__LIBCCALL __vswprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return _vswprintf(__buf,__format,__args); }
__LOCAL int (__ATTR_CDECL __swprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vswprintf(__buf,__format,__args); __builtin_va_end(__args); return __result; }
#endif

#if !defined(__NO_ASMNAME) && defined(__CRT_DOS)
__LIBC int (__ATTR_CDECL _swprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swprintf_c_l","_swprintf_c_l");
__LIBC int (__LIBCCALL _vswprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args) __ASMNAME2("vswprintf_c_l","_vswprintf_c_l");
#else /* !__NO_ASMNAME && __CRT_DOS */
__LOCAL int (__ATTR_CDECL _swprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) { __builtin_va_list __args; int __result; __builtin_va_start(__args,__locale); __result = __NAMESPACE_STD_SYM vswprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__LIBCCALL _vswprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return __NAMESPACE_STD_SYM vswprintf(__buf,__buflen,__format,__args); }
#endif /* __NO_ASMNAME || !__CRT_DOS */

#define getwchar()            fgetwc(stdin)
#define putwchar(c)           fputwc((c),stdout)
#define getwc(file)           fgetwc(file)
#define putwc(c,file)         fputwc(c,file)
#define _putwc_nolock(c,file) _fputwc_nolock(c,file)
#define _getwc_nolock(file)   _fgetwc_nolock(file)
#endif  /* _WSTDIO_DEFINED */
#endif  /* _STDIO_DEFINED */

#ifdef __CRT_DOS
#define _fgetc_nolock(stream)    (--(stream)->__f_cnt >= 0 ? 0xff & *(stream)->__f_ptr++ : _filbuf(stream))
#define _fputc_nolock(c,stream)  (--(stream)->__f_cnt >= 0 ? 0xff & (*(stream)->__f_ptr++ = (char)(c)) :  _flsbuf((c),(stream)))
#else /* __CRT_DOS */
#define _fgetc_nolock(stream)    fgetc_unlocked(stream)
#define _fputc_nolock(c,stream)  fputc_unlocked(c,stream)
#endif /* !__CRT_DOS */
#define _getc_nolock(stream)     _fgetc_nolock(stream)
#define _putc_nolock(c,stream)   _fputc_nolock(c, stream)
#define _getchar_nolock()        _getc_nolock(stdin)
#define _putchar_nolock(c)       _putc_nolock((c),stdout)
#define _getwchar_nolock()       _getwc_nolock(stdin)
#define _putwchar_nolock(c)      _putwc_nolock((c),stdout)

__REDIRECT_IFKOS_VOID(__LIBC,,__LIBCCALL,_lock_file,(__FILE *__restrict __file),flockfile,(__file))
__REDIRECT_IFKOS_VOID(__LIBC,,__LIBCCALL,_unlock_file,(__FILE *__restrict __file),funlockfile,(__file))
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_fclose_nolock,(__FILE *__restrict __file),fclose,(__file))
#ifdef __CRT_KOS
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_fflush_nolock,(__FILE *__restrict __file),fflush_unlocked,(__file))
#else /* __CRT_KOS */
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_fflush_nolock,(__FILE *__restrict __file),fflush,(__file))
#endif /* !__CRT_KOS */
__REDIRECT_IFKOS(__LIBC,,size_t,__LIBCCALL,_fread_nolock,(void *__restrict __buf, size_t __elemsize, size_t __elemcount, __FILE *__restrict __file),fread_unlocked,(__buf,__elemsize,__elemcount,__file))
__REDIRECT_IFKOS(__LIBC,,size_t,__LIBCCALL,_fwrite_nolock,(void const *__restrict __buf, size_t __elemsize, size_t __elemcount, __FILE *__restrict __file),fwrite_unlocked,(__buf,__elemsize,__elemcount,__file))

#ifdef __CRT_DOS
__LIBC size_t (__LIBCCALL _fread_nolock_s)(void *__restrict __buf, size_t __bufsize, size_t __elemsize, size_t __elemcount, __FILE *__restrict __file);
__LIBC int (__LIBCCALL _fseek_nolock)(__FILE *__restrict __file, __LONG32_TYPE__ __off, int __whence);
__LIBC __LONG32_TYPE__ (__LIBCCALL _ftell_nolock)(__FILE *__restrict __file);
__LIBC int (__LIBCCALL _fseeki64_nolock)(__FILE *__restrict __file, __INT64_TYPE__ __off, int __whence);
__LIBC __INT64_TYPE__ (__LIBCCALL _ftelli64_nolock)(__FILE *__restrict __file);
__LIBC int (__LIBCCALL _ungetc_nolock)(int __ch, __FILE *__restrict __file);
#else
__REDIRECT(__LIBC,,int,__LIBCCALL,_ungetc_nolock,(int __ch, __FILE *__restrict __file),ungetc,(__ch,__file))
__REDIRECT2(__LIBC,,size_t,__LIBCCALL,__do_fread_unlocked,(void *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream),fread_unlocked,_fread_nolock,(__buf,__size,__n,__stream))
__LOCAL size_t (__LIBCCALL _fread_nolock_s)(void *__restrict __buf, size_t __bufsize, size_t __elemsize, size_t __elemcount, __FILE *__restrict __file) { __bufsize /= __elemsize; return __do_fread_unlocked(__buf,__elemsize,__bufsize < __elemcount ? __bufsize : __elemcount,__file); }
#ifdef __CRT_GLC
__REDIRECT(__LIBC,,int,__LIBCCALL,_fseek_nolock,(__FILE *__restrict __file, __LONG32_TYPE__ __off, int __whence),fseeko,(__file,__off,__whence))
__REDIRECT(__LIBC,,__LONG32_TYPE__,__LIBCCALL,_ftell_nolock,(__FILE *__restrict __file),ftello,(__file))
__REDIRECT(__LIBC,,int,__LIBCCALL,_fseeki64_nolock,(__FILE *__restrict __file, __INT64_TYPE__ __off, int __whence),fseeko64,(__file,__off,__whence))
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_ftelli64_nolock,(__FILE *__restrict __file),ftello64,(__file))
#elif __SIZEOF_LONG__ == 4
__REDIRECT(__LIBC,,int,__LIBCCALL,_fseek_nolock,(__FILE *__restrict __file, __LONG32_TYPE__ __off, int __whence),fseek,(__file,__off,__whence))
__REDIRECT(__LIBC,,__LONG32_TYPE__,__LIBCCALL,_ftell_nolock,(__FILE *__restrict __file),ftell,(__file))
__LOCAL int (__LIBCCALL _fseeki64_nolock)(__FILE *__restrict __file, __INT64_TYPE__ __off, int __whence) { return fseek(__file,(long int)__off,__whence); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _ftelli64_nolock)(__FILE *__restrict __file) { return (__INT64_TYPE__)ftell(__file); }
#elif __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,,int,__LIBCCALL,_fseeki64_nolock,(__FILE *__restrict __file, __INT64_TYPE__ __off, int __whence),fseek,(__file,__off,__whence))
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_ftelli64_nolock,(__FILE *__restrict __file),ftell,(__file))
__LOCAL int (__LIBCCALL _fseek_nolock)(__FILE *__restrict __file, __LONG32_TYPE__ __off, int __whence) { return fseek(__file,(long int)__off,__whence); }
__LOCAL __LONG32_TYPE__ (__LIBCCALL _ftell_nolock)(__FILE *__restrict __file) { return (__LONG32_TYPE__)ftell(__file); }
#else /* ... */
__LOCAL int (__LIBCCALL _fseek_nolock)(__FILE *__restrict __file, __LONG32_TYPE__ __off, int __whence) { return fseek(__file,(long int)__off,__whence); }
__LOCAL __LONG32_TYPE__ (__LIBCCALL _ftell_nolock)(__FILE *__restrict __file) { return (__LONG32_TYPE__)ftell(__file); }
__LOCAL int (__LIBCCALL _fseeki64_nolock)(__FILE *__restrict __file, __INT64_TYPE__ __off, int __whence) { return fseek(__file,(long int)__off,__whence); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _ftelli64_nolock)(__FILE *__restrict __file) { return (__INT64_TYPE__)ftell(__file); }
#endif /* !... */
#endif

#define SYS_OPEN  _SYS_OPEN
__REDIRECT(__LIBC,,int,__LIBCCALL,fgetchar,(void),getchar,())
__REDIRECT(__LIBC,,int,__LIBCCALL,fputchar,(int __ch),putchar,(__ch))
__REDIRECT(__LIBC,,int,__LIBCCALL,flushall,(void),_flushall,())
__REDIRECT(__LIBC,,int,__LIBCCALL,rmtmp,(void),_rmtmp,())
#endif /* __USE_DOS */

__SYSDECL_END

#endif /* !_STDIO_H */
