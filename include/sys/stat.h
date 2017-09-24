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
#ifndef _SYS_STAT_H
#define _SYS_STAT_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>
#include <bits/stat.h>

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K)
#include <time.h>
#endif
#ifdef __USE_ATFILE
#include <hybrid/timespec.h>
#endif

__DECL_BEGIN

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K)
#ifndef __dev_t_defined
#define __dev_t_defined 1
typedef __dev_t dev_t;
#endif
#ifndef __gid_t_defined
#define __gid_t_defined 1
typedef __gid_t gid_t;
#endif
#ifndef __ino_t_defined
#define __ino_t_defined 1
typedef __FS_TYPE(ino) ino_t;
#endif

#ifndef __mode_t_defined
#define __mode_t_defined 1
typedef __mode_t mode_t;
#endif

#ifndef __nlink_t_defined
#define __nlink_t_defined 1
typedef __nlink_t nlink_t;
#endif

#ifndef __off_t_defined
#define __off_t_defined 1
typedef __FS_TYPE(off)   off_t;
#endif

#ifndef __uid_t_defined
#define __uid_t_defined 1
typedef __uid_t uid_t;
#endif
#endif /* __USE_XOPEN || __USE_XOPEN2K */

#ifdef __USE_UNIX98
#ifndef __blkcnt_t_defined
#define __blkcnt_t_defined 1
typedef __FS_TYPE(blkcnt)   blkcnt_t;
#endif /* __blkcnt_t_defined */
#ifndef __blksize_t_defined
#define __blksize_t_defined 1
typedef __blksize_t blksize_t;
#endif /* __blksize_t_defined */
#endif /* __USE_UNIX98 */

#if defined(__USE_MISC) || defined(__USE_XOPEN)
#   define S_IFMT     __S_IFMT
#   define S_IFDIR    __S_IFDIR
#   define S_IFCHR    __S_IFCHR
#   define S_IFBLK    __S_IFBLK
#   define S_IFREG    __S_IFREG
#ifdef __S_IFIFO
#   define S_IFIFO    __S_IFIFO
#endif
#ifdef __S_IFLNK
#   define S_IFLNK    __S_IFLNK
#endif
#if (defined(__USE_MISC) || defined(__USE_UNIX98)) && \
     defined(__S_IFSOCK)
#   define S_IFSOCK   __S_IFSOCK
#endif
#endif

#   define __S_ISTYPE(mode,mask)   (((mode)&__S_IFMT)==(mask))
#   define S_ISDIR(mode)  __S_ISTYPE((mode),__S_IFDIR)
#   define S_ISCHR(mode)  __S_ISTYPE((mode),__S_IFCHR)
#   define S_ISBLK(mode)  __S_ISTYPE((mode),__S_IFBLK)
#   define S_ISREG(mode)  __S_ISTYPE((mode),__S_IFREG)
#ifdef __USE_KOS
#   define S_ISDEV(mode)    __S_ISDEV(mode)
#endif /* __USE_KOS */
#ifdef __S_IFIFO
#   define S_ISFIFO(mode) __S_ISTYPE((mode),__S_IFIFO)
#endif
#ifdef __S_IFLNK
#   define S_ISLNK(mode)  __S_ISTYPE((mode),__S_IFLNK)
#endif

#if defined(__USE_MISC) && !defined(__S_IFLNK)
#   define S_ISLNK(mode)  0
#endif

#if (defined(__USE_UNIX98) || defined(__USE_XOPEN2K)) && \
     defined(__S_IFSOCK)
#   define S_ISSOCK(mode) __S_ISTYPE((mode),__S_IFSOCK)
#elif defined __USE_XOPEN2K
#   define S_ISSOCK(mode) 0
#endif

#ifdef __USE_POSIX199309
#   define S_TYPEISMQ(buf)  __S_TYPEISMQ(buf)
#   define S_TYPEISSEM(buf) __S_TYPEISSEM(buf)
#   define S_TYPEISSHM(buf) __S_TYPEISSHM(buf)
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
#ifdef __USE_MISC
#   define S_IREAD  S_IRUSR
#   define S_IWRITE S_IWUSR
#   define S_IEXEC  S_IXUSR
#endif

#define S_IRGRP (S_IRUSR >> 3) /* Read by group. */
#define S_IWGRP (S_IWUSR >> 3) /* Write by group. */
#define S_IXGRP (S_IXUSR >> 3) /* Execute by group. */
#define S_IRWXG (S_IRWXU >> 3)
#define S_IROTH (S_IRGRP >> 3) /* Read by others. */
#define S_IWOTH (S_IWGRP >> 3) /* Write by others. */
#define S_IXOTH (S_IXGRP >> 3) /* Execute by others. */
#define S_IRWXO (S_IRWXG >> 3)


#ifdef __USE_KOS
/* As also seen in the linux kernel headers. */
#   define S_IRWXUGO (S_IRWXU|S_IRWXG|S_IRWXO)
#   define S_IALLUGO (S_ISUID|S_ISGID|S_ISVTX|S_IRWXUGO)
#   define S_IRUGO   (S_IRUSR|S_IRGRP|S_IROTH)
#   define S_IWUGO   (S_IWUSR|S_IWGRP|S_IWOTH)
#   define S_IXUGO   (S_IXUSR|S_IXGRP|S_IXOTH)
#endif /* __USE_KOS */

#ifdef __USE_MISC
#   define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO) /* 0777 */
#   define ALLPERMS    (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)/* 07777 */
#   define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)/* 0666*/
#   define S_BLKSIZE    512 /* Block size for `st_blocks'. */
#endif /* __USE_MISC */

#ifndef __KERNEL__
/* NOTE: Since KOS uses a different 'stat' buffer than glibc, but still wants to
 *       maintain binary compatibility, the 'stat()' function provided internally
 *       accepts a glibc-compatible stat buffer, while the functions making use
 *       of what we (and the kernel) defines as its stat()-buffer are actually
 *       named 'kstat()' */
__LIBC __NONNULL((1,2)) int (__LIBCCALL stat)(char const *__restrict __file, struct stat *__restrict __buf) __UFS_FUNCn_(kstat);
__LIBC __NONNULL((2)) int (__LIBCCALL fstat)(int __fd, struct stat *__buf) __UFS_FUNCn_(kfstat);
#ifdef __USE_LARGEFILE64
__LIBC __NONNULL((1,2)) int (__LIBCCALL stat64)(char const *__restrict __file, struct stat64 *__restrict __buf) __UFS_FUNC_(kstat64);
__LIBC __NONNULL((2)) int (__LIBCCALL fstat64)(int __fd, struct stat64 *__buf) __UFS_FUNC_(kfstat64);
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_ATFILE
__LIBC __NONNULL((2,3)) int (__LIBCCALL fstatat)(int __fd, char const *__restrict __file, struct stat *__restrict __buf, int __flag) __UFS_FUNCn_(kfstatat);
#ifdef __USE_LARGEFILE64
__LIBC __NONNULL((2,3)) int (__LIBCCALL fstatat64)(int __fd, char const *__restrict __file, struct stat64 *__restrict __buf, int __flag) __UFS_FUNC_(kfstatat64);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_ATFILE */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__LIBC __NONNULL((1,2)) int (__LIBCCALL lstat)(char const *__restrict __file, struct stat *__restrict __buf) __UFS_FUNCn_(klstat);
#ifdef __USE_LARGEFILE64
__LIBC __NONNULL((1,2)) int (__LIBCCALL lstat64)(char const *__restrict __file, struct stat64 *__restrict __buf) __UFS_FUNC_(klstat64);
#endif /* __USE_LARGEFILE64*/
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#ifndef __mkdir_defined
#define __mkdir_defined 1
#ifdef __USE_DOSFS
/* NOTE: 'mkdir()' with 1 argument is defined in <direct.h> */
__LIBC __NONNULL((1)) int (__LIBCCALL mkdir)(char const *__path, __mode_t __mode) __UFS_FUNC_(mkdir_m);
#else /* __PE__ */
__LIBC __NONNULL((1)) int (__LIBCCALL mkdir)(char const *__path, __mode_t __mode) __UFS_FUNC_(mkdir);
#endif /* !__PE__ */
#endif /* !__mkdir_defined */
#ifndef __chmod_defined
#define __chmod_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL chmod)(char const *__file, __mode_t __mode) __UFS_FUNC(chmod);
#endif /* !__chmod_defined */
#ifndef __umask_defined
#define __umask_defined 1
__LIBC __mode_t (__LIBCCALL umask)(__mode_t __mask);
#endif /* !__umask_defined */
__LIBC __NONNULL((1)) int (__LIBCCALL mkfifo)(char const *__path, __mode_t __mode) __UFS_FUNC(mkfifo);
#ifdef __USE_ATFILE
__LIBC __NONNULL((2)) int (__LIBCCALL mknodat)(int __fd, char const *__path, __mode_t __mode, __dev_t __dev) __UFS_FUNC(mknodat);
__LIBC __NONNULL((2)) int (__LIBCCALL fchmodat)(int __fd, char const *__file, __mode_t __mode, int __flag) __UFS_FUNC(fchmodat);
__LIBC __NONNULL((2)) int (__LIBCCALL mkdirat)(int __fd, char const *__path, __mode_t __mode) __UFS_FUNC(mkdirat);
__LIBC __NONNULL((2)) int (__LIBCCALL mkfifoat)(int __fd, char const *__path, __mode_t __mode) __UFS_FUNC(mkfifoat);
__LIBC __NONNULL((2)) int (__LIBCCALL utimensat)(int __fd, char const *__path, struct timespec const __times[2], int __flags) __UFS_FUNCt(utimensat);
#ifdef __USE_TIME64
__LIBC __NONNULL((2)) int (__LIBCCALL utimensat64)(int __fd, char const *__path, struct __timespec64 const __times[2], int __flags) __UFS_FUNC(utimensat64);
#endif /* __USE_TIME64 */
#endif /* __USE_ATFILE */
#ifdef __USE_MISC
__LIBC __NONNULL((1)) int (__LIBCCALL lchmod)(char const *__file, __mode_t __mode) __UFS_FUNC(lchmod);
#endif /* __USE_MISC */
#ifdef __USE_POSIX
__LIBC int (__LIBCCALL fchmod)(int __fd, __mode_t __mode);
#endif /* __USE_POSIX */
#ifdef __USE_GNU
__LIBC __WUNUSED __mode_t (__LIBCCALL getumask)(void);
#endif /* __USE_GNU */
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC __NONNULL((1)) int (__LIBCCALL mknod)(char const *__path, __mode_t __mode, __dev_t __dev) __UFS_FUNC(mknod);
#ifdef __USE_ATFILE
__LIBC __NONNULL((2)) int (__LIBCCALL mknodat)(int __fd, char const *__path, __mode_t __mode, __dev_t __dev) __UFS_FUNC(mknod);
#endif /* __USE_ATFILE */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_XOPEN2K8
__LIBC __NONNULL((2)) int (__LIBCCALL futimens)(int __fd, struct timespec const __times[2]) __TM_FUNC(futimens);
#ifdef __USE_TIME64
__LIBC __NONNULL((2)) int (__LIBCCALL futimens64)(int __fd, struct __timespec64 const __times[2]);
#endif /* __USE_TIME64 */
#endif /* __USE_XOPEN2K8 */
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_SYS_STAT_H */
