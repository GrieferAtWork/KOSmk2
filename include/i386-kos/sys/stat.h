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
#endif /* __USE_XOPEN || __USE_XOPEN2K */

#ifdef __USE_ATFILE
#include <hybrid/timespec.h>
#endif /* __USE_ATFILE */

__SYSDECL_BEGIN

#ifdef __INTELLISENSE__
struct stat;
#ifdef __USE_LARGEFILE64
struct stat64;
#endif /* __USE_LARGEFILE64 */
#endif /* __INTELLISENSE__ */

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K)
#ifndef __dev_t_defined
#define __dev_t_defined 1
typedef __typedef_dev_t dev_t;
#endif /* __dev_t_defined */

#ifndef __gid_t_defined
#define __gid_t_defined 1
typedef __gid_t gid_t;
#endif /* __gid_t_defined */

#ifndef __ino_t_defined
#define __ino_t_defined 1
typedef __typedef_ino_t ino_t;
#endif /* __ino_t_defined */

#ifndef __mode_t_defined
#define __mode_t_defined 1
typedef __mode_t mode_t;
#endif /* __mode_t_defined */

#ifndef __nlink_t_defined
#define __nlink_t_defined 1
typedef __nlink_t nlink_t;
#endif /* __nlink_t_defined */

#ifndef __off_t_defined
#define __off_t_defined 1
typedef __typedef_off_t off_t;
#endif /* __off_t_defined */

#ifndef __uid_t_defined
#define __uid_t_defined 1
typedef __uid_t uid_t;
#endif /* __uid_t_defined */
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
#define S_IRWXUGO (S_IRWXU|S_IRWXG|S_IRWXO)
#define S_IALLUGO (S_ISUID|S_ISGID|S_ISVTX|S_IRWXUGO)
#define S_IRUGO   (S_IRUSR|S_IRGRP|S_IROTH)
#define S_IWUGO   (S_IWUSR|S_IWGRP|S_IWOTH)
#define S_IXUGO   (S_IXUSR|S_IXGRP|S_IXOTH)
#endif /* __USE_KOS */

#ifdef __USE_MISC
#define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO) /* 0777 */
#define ALLPERMS    (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)/* 07777 */
#define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)/* 0666*/
#define S_BLKSIZE    512 /* Block size for `st_blocks'. */
#endif /* __USE_MISC */

#ifdef __USE_DOS
#define _S_IFMT   __S_IFMT
#define _S_IFDIR  __S_IFDIR
#define _S_IFCHR  __S_IFCHR
#define _S_IFIFO  __S_IFIFO
#define _S_IFREG  __S_IFREG
#define _S_IREAD  __S_IREAD
#define _S_IWRITE __S_IWRITE
#define _S_IEXEC  __S_IEXEC
#endif /* __USE_DOS */

#ifndef __KERNEL__
/* Recognized & known stat() structures and assembly names.
 * 
 * __CRT_KOS: (Used by default)
 * >> int CDECL kfstat(int fd, struct __kos_stat *buf);
 * >> int CDECL kfstat64(int fd, struct __kos_stat64 *buf);
 * >> int CDECL kstat(char const *kos_file, struct __kos_stat *buf);
 * >> int CDECL kstat64(char const *kos_file, struct __kos_stat64 *buf);
 * >> int CDECL kfstatat(int fd, char const *kos_file, struct __kos_stat *buf, int flag);
 * >> int CDECL kfstatat64(int fd, char const *kos_file, struct __kos_stat64 *buf, int flag);
 * >> int CDECL klstat(char const *kos_file, struct __kos_stat *buf);
 * >> int CDECL klstat64(char const *kos_file, struct __kos_stat64 *buf);
 * >> int CDECL .dos.kstat(char const *dos_file, struct __kos_stat *buf);
 * >> int CDECL .dos.kstat64(char const *dos_file, struct __kos_stat64 *buf);
 * >> int CDECL .dos.kfstatat(int fd, char const *dos_file, struct __kos_stat *buf, int flag);
 * >> int CDECL .dos.kfstatat64(int fd, char const *dos_file, struct __kos_stat64 *buf, int flag);
 * >> int CDECL .dos.klstat(char const *dos_file, struct __kos_stat *buf);
 * >> int CDECL .dos.klstat64(char const *dos_file, struct __kos_stat64 *buf);
 *
 * __CRT_GLC: (__GLC_COMPAT__)
 * >> int CDECL fstat(int fd, struct __glc_stat *buf);
 * >> int CDECL fstat64(int fd, struct __glc_stat64 *buf);
 * >> int CDECL stat(char const *kos_file, struct __glc_stat *buf);
 * >> int CDECL stat64(char const *kos_file, struct __glc_stat64 *buf);
 * >> int CDECL fstatat(int fd, char const *kos_file, struct __glc_stat *buf, int flag);
 * >> int CDECL fstatat64(int fd, char const *kos_file, struct __glc_stat64 *buf, int flag);
 * >> int CDECL lstat(char const *kos_file, struct __glc_stat *buf);
 * >> int CDECL lstat64(char const *kos_file, struct __glc_stat64 *buf);
 * 
 * __CRT_DOS: (__DOS_COMPAT__)
 * >> int CDECL [OLD: _fstat,    NEW:_fstat32] (int fd, struct __dos_stat32 *buf);
 * >> int CDECL [OLD: _fstati64, NEW:_fstat32i64] (int fd, struct __dos_stat32i64 *buf);
 * >> int CDECL [OLD: _stat,     NEW:_stat32] (char const *dos_file, struct __dos_stat32 *buf);
 * >> int CDECL [OLD: _stati64,  NEW:_stat32i64] (char const *dos_file, struct __dos_stat32i64 *buf);
 * >> int CDECL _fstat64(int fd, struct __dos_stat64 *buf);
 * >> int CDECL _stat64(char const *dos_file, struct __dos_stat64 *buf);
 * >> int CDECL [NEW: _fstat64i32](int fd, struct __dos_stat64i32 *buf) = _fstat64;
 * >> int CDECL [NEW: _stat64i32](char const *dos_file, struct __dos_stat64i32 *buf) = _stat64;
 *
 * NOTE: Since KOS uses a different 'stat' buffer than glibc, but still wants to
 *       maintain binary compatibility, the 'stat()' function provided internally
 *       accepts a glibc-compatible stat buffer, while the functions making use
 *       of what we (and the kernel) defines as its stat()-buffer are actually
 *       named 'kstat()' */

#undef stat
#undef stat64
#undef lstat64
#ifdef __DOS_COMPAT__
/* 'struct stat' is:
 *    [                                       ] = struct __dos_stat32
 *    [__USE_TIME_BITS64                      ] = struct __dos_stat64i32 == struct __dos_stat64
 *    [__USE_FILE_OFFSET64                    ] = struct __dos_stat32i64
 *    [__USE_TIME_BITS64 + __USE_FILE_OFFSET64] = struct __dos_stat64
 * 'struct stat64' is:
 *    [                 ] = struct __dos_stat32i64
 *    [__USE_TIME_BITS64] = struct __dos_stat64 */
#ifdef __USE_TIME_BITS64
#undef _stat64
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,stat,(char const *__restrict __file, struct stat *__restrict __buf),_stat64,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,fstat,(int __fd, struct stat *__buf),_fstat64,(__fd,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,lstat,(char const *__restrict __file, struct stat *__restrict __buf),_stat64,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#ifdef __USE_LARGEFILE64
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,stat64,(char const *__restrict __file, struct stat64 *__restrict __buf),_stat64,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,fstat64,(int __fd, struct stat64 *__buf),_fstat64,(__fd,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,lstat64,(char const *__restrict __file, struct stat64 *__restrict __buf),_stat64,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#endif /* __USE_LARGEFILE64 */
#else /* __USE_TIME_BITS64 */
#undef _stat32
#undef _stat32i64
#undef _fstat32
#undef _fstat32i64
#ifdef __USE_DOS_LINKOLDFINDSTAT
#   undef _stat
#   undef _stati64
#   undef _fstat
#   undef _fstati64
#endif /* __USE_DOS_LINKOLDFINDSTAT */
#ifdef __USE_FILE_OFFSET64
#ifdef __USE_DOS_LINKOLDFINDSTAT
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,stat,(char const *__restrict __file, struct stat *__restrict __buf),_stati64,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,fstat,(int __fd, struct stat *__buf),_fstati64,(__fd,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,lstat,(char const *__restrict __file, struct stat *__restrict __buf),_stati64,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#else /* __USE_DOS_LINKOLDFINDSTAT */
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,stat,(char const *__restrict __file, struct stat *__restrict __buf),_stat32i64,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,fstat,(int __fd, struct stat *__buf),_fstat32i64,(__fd,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,lstat,(char const *__restrict __file, struct stat *__restrict __buf),_stat32i64,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#endif /* !__USE_DOS_LINKOLDFINDSTAT */
#else /* __USE_FILE_OFFSET64 */
#ifdef __USE_DOS_LINKOLDFINDSTAT
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,stat,(char const *__restrict __file, struct stat *__restrict __buf),_stat,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,fstat,(int __fd, struct stat *__buf),_fstat,(__fd,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,lstat,(char const *__restrict __file, struct stat *__restrict __buf),_stat,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#else /* __USE_DOS_LINKOLDFINDSTAT */
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,stat,(char const *__restrict __file, struct stat *__restrict __buf),_stat32,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,fstat,(int __fd, struct stat *__buf),_fstat32,(__fd,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,lstat,(char const *__restrict __file, struct stat *__restrict __buf),_stat32,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#endif /* !__USE_DOS_LINKOLDFINDSTAT */
#endif /* !__USE_FILE_OFFSET64 */
#ifdef __USE_LARGEFILE64
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,stat64,(char const *__restrict __file, struct stat64 *__restrict __buf),_stat32i64,(__fd,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,fstat64,(int __fd, struct stat64 *__buf),_fstat32i64,(__fd,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,lstat64,(char const *__restrict __file, struct stat64 *__restrict __buf),_stat32i64,(__fd,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#endif /* __USE_LARGEFILE64 */
#endif /* !__USE_TIME_BITS64 */

#elif defined(__GLC_COMPAT__)

__REDIRECT_FS_FUNC(__LIBC,__WARN_NODOSFS __NONNULL((1,2)),int,__LIBCCALL,stat,(char const *__restrict __file, struct stat *__restrict __buf),stat,(__file,__buf))
__REDIRECT_FS_FUNC(__LIBC,__NONNULL((2)),int,__LIBCCALL,fstat,(int __fd, struct stat *__buf),fstat,(__fd,__buf))
#ifdef __USE_LARGEFILE64
__LIBC __WARN_NODOSFS __NONNULL((1,2)) int (__LIBCCALL stat64)(char const *__restrict __file, struct stat64 *__restrict __buf);
__LIBC __NONNULL((2)) int (__LIBCCALL fstat64)(int __fd, struct stat64 *__buf);
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_ATFILE
__REDIRECT_UFS_FUNCn(__LIBC,__PORT_NODOS_ALT(stat) __WARN_NODOSFS __NONNULL((2,3)),int,__LIBCCALL,fstatat,
                    (int __fd, char const *__restrict __file, struct stat *__restrict __buf, int __flag),
                     fstatat,(__fd,__file,__buf,__flag))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS_ALT(stat64) __WARN_NODOSFS __NONNULL((2,3))
int (__LIBCCALL fstatat64)(int __fd, char const *__restrict __file,
                           struct stat64 *__restrict __buf, int __flag);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_ATFILE */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT_FS_FUNC(__LIBC,__WARN_NODOSFS __NONNULL((1,2)),int,__LIBCCALL,lstat,
                  (char const *__restrict __file, struct stat *__restrict __buf),lstat,(__file,__buf))
#ifdef __USE_LARGEFILE64
__LIBC __WARN_NODOSFS __NONNULL((1,2)) int (__LIBCCALL lstat64)(char const *__restrict __file, struct stat64 *__restrict __buf);
#endif /* __USE_LARGEFILE64*/
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#else /* Compat... */

__REDIRECT_UFS_FUNCn_(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,stat,(char const *__restrict __file, struct stat *__restrict __buf),kstat,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,fstat,(int __fd, struct stat *__buf),kfstat,(__fd,__buf))
#ifdef __USE_LARGEFILE64
__REDIRECT_UFS_FUNCn_(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,stat64,(char const *__restrict __file, struct stat64 *__restrict __buf),kstat64,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,fstat64,(int __fd, struct stat64 *__buf),kfstat64,(__fd,__buf))
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_ATFILE
__REDIRECT_UFS_FUNCn_(__LIBC,__PORT_NODOS_ALT(stat) __NONNULL((2,3)),int,__LIBCCALL,fstatat,
                     (int __fd, char const *__restrict __file, struct stat *__restrict __buf, int __flag),
                      kfstatat,(__fd,__file,__buf,__flag))
#ifdef __USE_LARGEFILE64
__REDIRECT_UFS_(__LIBC,__PORT_NODOS_ALT(stat) __NONNULL((2,3)),int,__LIBCCALL,fstatat64,
               (int __fd, char const *__restrict __file, struct stat64 *__restrict __buf, int __flag),
                kfstatat64,(__fd,__file,__buf,__flag))
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_ATFILE */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT_UFS_FUNCn_(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,lstat,(char const *__restrict __file, struct stat *__restrict __buf),klstat,(__file,__buf))
#ifdef __USE_LARGEFILE64
__REDIRECT_UFS_FUNCn_(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,lstat64,(char const *__restrict __file, struct stat64 *__restrict __buf),klstat64,(__file,__buf))
#endif /* __USE_LARGEFILE64*/
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#endif /* Builtin... */



#ifndef __mkdir_defined
#define __mkdir_defined 1
#ifdef __USE_DOSFS
/* NOTE: 'mkdir()' with 1 argument is defined in <direct.h> */
__REDIRECT_UFS_(__LIBC,__NONNULL((1)),int,__LIBCCALL,mkdir,(char const *__path, __mode_t __mode),mkdir_m,(__path,__mode))
#elif defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__REDIRECT_UFS(__LIBC,__NONNULL((1)),int,__LIBCCALL,mkdir,(char const *__path, __mode_t __mode),mkdir,(__path,__mode))
#else
__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,__dos_mkdir,(char const *__path),_mkdir,(__path))
__LOCAL __WARN_NOKOSFS __NONNULL((1)) int (__LIBCCALL mkdir)(char const *__path, __mode_t __UNUSED(__mode)) { return __dos_mkdir(__path); }
#endif /* !__USE_DOSFS */
#endif /* !__mkdir_defined */

#ifndef __chmod_defined
#define __chmod_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,chmod,(char const *__file, int __mode),chmod,(__file,__mode))
#endif /* !__chmod_defined */

#ifdef __USE_MISC
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,lchmod,(char const *__file, __mode_t __mode),_chmod,(__file,__mode))
#else /* __DOS_COMPAT__ */
__REDIRECT_UFS(__LIBC,__NONNULL((1)),int,__LIBCCALL,lchmod,(char const *__file, __mode_t __mode),lchmod,(__file,__mode))
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_MISC */

#ifndef __umask_defined
#define __umask_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,,__mode_t,__LIBCCALL,umask,(__mode_t __mode),uname,(__mode))
#endif /* !__umask_defined */

#ifdef __USE_GNU
#ifdef __DOS_COMPAT__
__LOCAL __WUNUSED __mode_t (__LIBCCALL getumask)(void) { __mode_t __result = umask(0); return (umask(__result),__result); }
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __mode_t (__LIBCCALL getumask)(void);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_GNU */

#ifdef __CRT_GLC
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,mkfifo,(char const *__path, __mode_t __mode),mkfifo,(__path,__mode))
#ifdef __USE_ATFILE
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((2)),int,__LIBCCALL,mknodat,(int __fd, char const *__path, __mode_t __mode, __dev_t __dev),mknodat,(__fd,__path,__mode,__dev))
__REDIRECT_UFS(__LIBC,__PORT_NODOS_ALT(chmod) __NONNULL((2)),int,__LIBCCALL,fchmodat,(int __fd, char const *__file, __mode_t __mode, int __flag),fchmodat,(__fd,__file,__mode,__flag))
__REDIRECT_UFS(__LIBC,__PORT_NODOS_ALT(mkdir) __NONNULL((2)),int,__LIBCCALL,mkdirat,(int __fd, char const *__path, __mode_t __mode),mkdirat,(__fd,__path,__mode))
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((2)),int,__LIBCCALL,mkfifoat,(int __fd, char const *__path, __mode_t __mode),mkfifoat,(__fd,__path,__mode))
#endif /* __USE_ATFILE */
#ifdef __USE_POSIX
__LIBC __PORT_NODOS_ALT(chmod) int (__LIBCCALL fchmod)(int __fd, __mode_t __mode);
#endif /* __USE_POSIX */
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,mknod,(char const *__path, __mode_t __mode, __dev_t __dev),mknod,(__path,__mode,__dev))
#ifdef __USE_ATFILE
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((2)),int,__LIBCCALL,mknodat,(int __fd, char const *__path, __mode_t __mode, __dev_t __dev),mknod,(__fd,__path,__mode,__dev))
#endif /* __USE_ATFILE */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __USE_ATFILE
#ifdef __GLC_COMPAT__
/* TODO: GLibc compatibility. */
__REDIRECT_UFS_FUNCt(__LIBC,__PORT_NODOS_ALT(utime) __NONNULL((2)),int,__LIBCCALL,utimensat,(int __fd, char const *__path, struct timespec const __times[2], int __flags),utimensat,(__fd,__path,__times,__flags))
#ifdef __USE_TIME64
__REDIRECT_UFS(__LIBC,__PORT_NODOS_ALT(utime64) __NONNULL((2)),int,__LIBCCALL,utimensat64,(int __fd, char const *__path, struct __timespec64 const __times[2], int __flags),utimensat64,(__fd,__path,__times,__flags))
#endif /* __USE_TIME64 */
#else /* __GLC_COMPAT__ */
__REDIRECT_UFS_FUNCt(__LIBC,__PORT_NODOS_ALT(utime) __NONNULL((2)),int,__LIBCCALL,utimensat,(int __fd, char const *__path, struct timespec const __times[2], int __flags),utimensat,(__fd,__path,__times,__flags))
#ifdef __USE_TIME64
__REDIRECT_UFS(__LIBC,__PORT_NODOS_ALT(utime64) __NONNULL((2)),int,__LIBCCALL,utimensat64,(int __fd, char const *__path, struct __timespec64 const __times[2], int __flags),utimensat64,(__fd,__path,__times,__flags))
#endif /* __USE_TIME64 */
#endif /* !__GLC_COMPAT__ */
#endif /* __USE_ATFILE */
#endif /* __CRT_GLC */

#ifdef __USE_XOPEN2K8
#ifdef __GLC_COMPAT__
/* TODO: GLibc compatibility. */
__REDIRECT_TM_FUNC(__LIBC,__NONNULL((2)),int,__LIBCCALL,futimens,(int __fd, struct timespec const __times[2]),futimens,(__fd,__times))
#ifdef __USE_TIME64
__LIBC __NONNULL((2)) int (__LIBCCALL futimens64)(int __fd, struct __timespec64 const __times[2]);
#endif /* __USE_TIME64 */
#else /* __GLC_COMPAT__ */
__REDIRECT_TM_FUNC(__LIBC,__NONNULL((2)),int,__LIBCCALL,futimens,(int __fd, struct timespec const __times[2]),futimens,(__fd,__times))
#ifdef __USE_TIME64
__LIBC __NONNULL((2)) int (__LIBCCALL futimens64)(int __fd, struct __timespec64 const __times[2]);
#endif /* __USE_TIME64 */
#endif /* !__GLC_COMPAT__ */
#endif /* __USE_XOPEN2K8 */


/* Define DOS's redundant stat() functions. */
#ifdef __USE_DOS
#ifdef __DOS_COMPAT__
/* Link stat functions with binary compatibility to DOS's stat buffer types. */
#ifdef __USE_DOS_LINKOLDFINDSTAT
#undef _stat
#undef _stati64
#undef _fstat
#undef _fstati64
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat32,(char const *__restrict __file, struct _stat32 *__restrict __buf),_stat,(__file,__buf))
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat32i64,(char const *__restrict __file, struct _stat32i64 *__restrict __buf),_stati64,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat32,(int __fd, struct _stat32 *__restrict __buf),_fstat,(__fd,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat32i64,(int __fd, struct _stat32i64 *__restrict __buf),_fstati64,(__fd,__buf))
#else /* __USE_DOS_LINKOLDFINDSTAT */
__LIBC __WARN_NOKOSFS __NONNULL((1,2)) int (__LIBCCALL _stat32)(char const *__restrict __file, struct _stat32 *__restrict __buf);
__LIBC __WARN_NOKOSFS __NONNULL((1,2)) int (__LIBCCALL _stat32i64)(char const *__restrict __file, struct _stat32i64 *__restrict __buf);
__LIBC __NONNULL((2)) int (__LIBCCALL _fstat32)(int __fd, struct _stat32 *__restrict __buf);
__LIBC __NONNULL((2)) int (__LIBCCALL _fstat32i64)(int __fd, struct _stat32i64 *__restrict __buf);
#endif /* !__USE_DOS_LINKOLDFINDSTAT  */

__LIBC __WARN_NOKOSFS __NONNULL((1,2)) int (__LIBCCALL _stat64)(char const *__restrict __file, struct _stat64 *__restrict __buf);
__LIBC __NONNULL((2)) int (__LIBCCALL _fstat64)(int __fd, struct _stat64 *__restrict __buf);
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat64i32,(char const *__restrict __file, struct _stat64i32 *__restrict __buf),_stat64,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat64i32,(int __fd, struct _stat64i32 *__restrict __buf),_fstat64,(__fd,__buf))
#elif defined(__GLC_COMPAT__)
/* Redirect DOS's stat functions to the GLC equivalents. */
#define _stat32    stat   /* These must be defined as macros because they're also structures. */
#define _stat64    stat64
#define _stat32i64 stat64
#define _stat64i32 stat64
//__REDIRECT(__LIBC,__WARN_NODOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat32,(char const *__restrict __file, struct _stat32 *__restrict __buf),stat,(__file,__buf))
//__REDIRECT(__LIBC,__WARN_NODOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat64,(char const *__restrict __file, struct _stat64 *__restrict __buf),stat64,(__file,__buf))
//__REDIRECT(__LIBC,__WARN_NODOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat32i64,(char const *__restrict __file, struct _stat32i64 *__restrict __buf),stat64,(__file,__buf))
//__REDIRECT(__LIBC,__WARN_NODOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat64i32,(char const *__restrict __file, struct _stat64i32 *__restrict __buf),stat64,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat32,(int __fd, struct _stat32 *__restrict __buf),fstat,(__fd,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat64,(int __fd, struct _stat64 *__restrict __buf),fstat64,(__fd,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat32i64,(int __fd, struct _stat32i64 *__restrict __buf),fstat64,(__fd,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat64i32,(int __fd, struct _stat64i32 *__restrict __buf),fstat64,(__fd,__buf))
#else /* Compat... */
/* Redirect DOS's stat functions to the KOS equivalents. */
#define _stat32    stat   /* These must be defined as macros because they're also structures. */
#define _stat64    stat64
#define _stat32i64 stat64
#define _stat64i32 stat64
//__REDIRECT(__LIBC,__WARN_NODOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat32,(char const *__restrict __file, struct _stat32 *__restrict __buf),kstat,(__file,__buf))
//__REDIRECT(__LIBC,__WARN_NODOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat64,(char const *__restrict __file, struct _stat64 *__restrict __buf),kstat64,(__file,__buf))
//__REDIRECT(__LIBC,__WARN_NODOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat32i64,(char const *__restrict __file, struct _stat32i64 *__restrict __buf),kstat64,(__file,__buf))
//__REDIRECT(__LIBC,__WARN_NODOSFS __NONNULL((1,2)),int,__LIBCCALL,_stat64i32,(char const *__restrict __file, struct _stat64i32 *__restrict __buf),kstat64,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat32,(int __fd, struct _stat32 *__restrict __buf),kfstat,(__fd,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat64,(int __fd, struct _stat64 *__restrict __buf),kfstat64,(__fd,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat32i64,(int __fd, struct _stat32i64 *__restrict __buf),kfstat64,(__fd,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,_fstat64i32,(int __fd, struct _stat64i32 *__restrict __buf),kfstat64,(__fd,__buf))
#endif /* Builtin... */
#endif /* __USE_DOS */

#ifdef __DOS_COMPAT__
/* Redefinite stat functions. */
#if defined(__USE_TIME_BITS64) || 1
#define _stat64          stat64
#else /* __USE_TIME_BITS64 */
#define _stat32i64       stat64
#endif /* !__USE_TIME_BITS64 */
#endif

#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_SYS_STAT_H */
