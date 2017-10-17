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
#ifndef _FCNTL_H
#define _FCNTL_H 1

#include "__stdinc.h"
#include "features.h"
#include <bits/fcntl.h>
#include <bits/types.h>
#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#include <hybrid/timespec.h>
#include <bits/stat.h>
#endif

__SYSDECL_BEGIN

#ifdef __O_TMPFILE
#   define __OPEN_NEEDS_MODE(oflag) (((oflag)&O_CREAT) || ((oflag)&__O_TMPFILE) == __O_TMPFILE)
#else
#   define __OPEN_NEEDS_MODE(oflag)  ((oflag)&O_CREAT)
#endif

#ifndef __mode_t_defined
#define __mode_t_defined 1
typedef __mode_t mode_t;
#endif /* !__mode_t_defined */
#ifndef __off_t_defined
#define __off_t_defined 1
typedef __typedef_off_t off_t;
#endif /* !__off_t_defined */
#ifdef __USE_LARGEFILE64
#ifndef __off64_t_defined
#define __off64_t_defined 1
typedef __off64_t off64_t;
#endif /* !__off64_t_defined */
#endif /* __USE_LARGEFILE64 */

#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif

/* For XPG all symbols from <sys/stat.h> should also be available. */
#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#   define S_IFMT     __S_IFMT
#   define S_IFDIR    __S_IFDIR
#   define S_IFCHR    __S_IFCHR
#   define S_IFBLK    __S_IFBLK
#   define S_IFREG    __S_IFREG
#ifdef __S_IFIFO
#   define S_IFIFO __S_IFIFO
#endif
#ifdef __S_IFLNK
#   define S_IFLNK __S_IFLNK
#endif
#if (defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)) && \
     defined(__S_IFSOCK)
#   define S_IFSOCK __S_IFSOCK
#endif
#   define S_ISUID __S_ISUID /* Set user ID on execution. */
#   define S_ISGID __S_ISGID /* Set group ID on execution. */
#if defined(__USE_MISC) || defined(__USE_XOPEN)
#   define S_ISVTX __S_ISVTX
#endif
#   define S_IRUSR  __S_IREAD  /*< Read by owner. */
#   define S_IWUSR  __S_IWRITE /*< Write by owner. */
#   define S_IXUSR  __S_IEXEC  /*< Execute by owner. */
#   define S_IRWXU (__S_IREAD|__S_IWRITE|__S_IEXEC)
#   define S_IRGRP (S_IRUSR >> 3) /* Read by group. */
#   define S_IWGRP (S_IWUSR >> 3) /* Write by group. */
#   define S_IXGRP (S_IXUSR >> 3) /* Execute by group. */
#   define S_IRWXG (S_IRWXU >> 3)
#   define S_IROTH (S_IRGRP >> 3) /* Read by others. */
#   define S_IWOTH (S_IWGRP >> 3) /* Write by others. */
#   define S_IXOTH (S_IXGRP >> 3) /* Execute by others. */
#   define S_IRWXO (S_IRWXG >> 3)
#endif

#ifdef __USE_MISC
#ifndef R_OK
#   define R_OK 4 /*< Test for read permission. */
#   define W_OK 2 /*< Test for write permission. */
#   define X_OK 1 /*< Test for execute permission. */
#   define F_OK 0 /*< Test for existence. */
#endif
#endif /* __USE_MISC */

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#   define SEEK_SET 0 /*< Seek from beginning of file. */
#   define SEEK_CUR 1 /*< Seek from current position. */
#   define SEEK_END 2 /*< Seek from end of file. */
#endif

#ifndef __KERNEL__

#ifdef __CRT_GLC
#ifdef __USE_KOS
__LIBC __PORT_NODOS __ssize_t (__ATTR_CDECL fcntl)(int __fd, int __cmd, ...);
#else /* __USE_KOS */
__LIBC __PORT_NODOS int (__ATTR_CDECL fcntl)(int __fd, int __cmd, ...);
#endif /* !__USE_KOS */
#endif /* __CRT_GLC */

#ifndef __open_defined
#define __open_defined 1 /* TODO: Use redirection */
__LIBC __WUNUSED __NONNULL((1)) int (__ATTR_CDECL open)(char const *__file, int __oflag, ...) __UFS_FUNCn_OLDPEA(open);
#endif /* !__open_defined */

#ifndef __creat_defined
#define __creat_defined 1
__REDIRECT_UFS_FUNCn_OLDPEA(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,creat,(char const *__file, mode_t __mode),creat,(__file,__mode))
#endif /* !__creat_defined */

#ifdef __USE_LARGEFILE64
/* TODO: Use redirection */
#ifdef __PE__
__LIBC __WUNUSED __NONNULL((1)) int (__ATTR_CDECL open64)(char const *__file, int __oflag, ...) __UFS_FUNC_(_open);
__REDIRECT_UFS_(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,creat64,
               (char const *__file, mode_t __mode),_creat,(__file,__mode))
#else /* __PE__ */
__LIBC __WUNUSED __NONNULL((1)) int (__ATTR_CDECL open64)(char const *__file, int __oflag, ...) __UFS_FUNC(open64);
__REDIRECT_UFS(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,creat64,
              (char const *__file, mode_t __mode),creat64,(__file,__mode))
#endif /* !__PE__ */
#endif /* __USE_LARGEFILE64 */

#ifdef __CRT_GLC
#ifdef __USE_ATFILE /* TODO: Use redirection */
__LIBC __WUNUSED __PORT_NODOS_ALT(open) __NONNULL((2)) int (__ATTR_CDECL openat)(int __fd, char const *__file, int __oflag, ...) __UFS_FUNCn(openat);
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED __PORT_NODOS_ALT(open64) __NONNULL((2)) int (__ATTR_CDECL openat64)(int __fd, char const *__file, int __oflag, ...) __UFS_FUNC(openat64);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_ATFILE */
#ifdef __USE_XOPEN2K
__REDIRECT_FS_FUNC(__LIBC,,int,__LIBCCALL,posix_fadvise,(int __fd, __FS_TYPE(off) __offset, __FS_TYPE(off) __len, int __advise),posix_fadvise,(__fd,__offset,__len,__advise));
__REDIRECT_FS_FUNC(__LIBC,,int,__LIBCCALL,posix_fallocate,(int __fd, __FS_TYPE(off) __offset, __FS_TYPE(off) __len),posix_fallocate,(__fd,__offset,__len))
#ifdef __USE_LARGEFILE64
__LIBC int (__LIBCCALL posix_fadvise64)(int __fd, __off64_t __offset, __off64_t __len, int __advise);
__LIBC int (__LIBCCALL posix_fallocate64)(int __fd, __off64_t __offset, __off64_t __len);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN2K */
#else /* __CRT_GLC */
#ifdef __USE_XOPEN2K
__LOCAL int (__LIBCCALL posix_fadvise)(int __UNUSED(__fd), __FS_TYPE(off) __UNUSED(__offset), __FS_TYPE(off) __UNUSED(__len), int __UNUSED(__advise)) { return 0; }
__LOCAL int (__LIBCCALL posix_fallocate)(int __UNUSED(__fd), __FS_TYPE(off) __UNUSED(__offset), __FS_TYPE(off) __UNUSED(__len)) { return 0; }
#ifdef __USE_LARGEFILE64
__LOCAL int (__LIBCCALL posix_fadvise64)(int __UNUSED(__fd), __off64_t __UNUSED(__offset), __off64_t __UNUSED(__len), int __UNUSED(__advise)) { return 0; }
__LOCAL int (__LIBCCALL posix_fallocate64)(int __UNUSED(__fd), __off64_t __UNUSED(__offset), __off64_t __UNUSED(__len)) { return 0; }
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN2K */
#endif /* !__CRT_GLC */


#endif /* !__KERNEL__ */

#if !defined(F_LOCK) && (defined(__USE_MISC) || \
    (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_POSIX)))
#   define F_ULOCK 0 /*< Unlock a previously locked region. */
#   define F_LOCK  1 /*< Lock a region for exclusive use. */
#   define F_TLOCK 2 /*< Test and lock a region for exclusive use. */
#   define F_TEST  3 /*< Test a region for other processes locks. */
#ifndef __KERNEL__
#if defined(__DOS_COMPAT__) || (defined(__PE__) && !defined(__USE_FILE_OFFSET64))
#ifdef __USE_FILE_OFFSET64
__REDIRECT(__LIBC,,int,__LIBCCALL,__lockf32,(int __fd, int __cmd, __off32_t __len),_locking,(__fd,__cmd,__len))
__LOCAL int (__LIBCCALL lockf)(int __fd, int __cmd, __off64_t __len) { return __lockf32(__fd,__cmd,(__off32_t)__len); }
#else /* __USE_FILE_OFFSET64 */
__REDIRECT(__LIBC,,int,__LIBCCALL,lockf,(int __fd, int __cmd, __FS_TYPE(off) __len),_locking,(__fd,__cmd,__len))
#endif /* !__USE_FILE_OFFSET64 */
#else /* ... */
__REDIRECT_FS_FUNC(__LIBC,,int,__LIBCCALL,lockf,(int __fd, int __cmd, __FS_TYPE(off) __len),lockf,(__fd,__cmd,__len))
#endif /* !... */
#ifdef __USE_LARGEFILE64
#ifdef __CRT_GLC
__LIBC int (__LIBCCALL lockf64)(int __fd, int __cmd, __off64_t __len);
#else /* __CRT_GLC */
__LOCAL int (__LIBCCALL lockf64)(int __fd, int __cmd, __off64_t __len) { return lockf(__fd,__cmd,(__FS_TYPE(off))__len); }
#endif /* !__CRT_GLC */
#endif /* __USE_LARGEFILE64 */
#endif /* !__KERNEL__ */
#endif /* ... */

__SYSDECL_END


#endif /* !_FCNTL_H */
