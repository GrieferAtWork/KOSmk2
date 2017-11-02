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
#ifndef _UNISTD_H
#define _UNISTD_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>
#include <bits/confname.h>
#include <bits/posix_opt.h>
#ifdef __CRT_GLC
#include <asm/unistd.h>
#endif /* __CRT_GLC */
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K)
#include <bits/environments.h>
#endif

__SYSDECL_BEGIN

/* Disclaimer: Code below is based off of /usr/include/unistd.h, yet has been _heavily_ modified. */

/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef __USE_XOPEN2K8
#   define _POSIX_VERSION    200809L
#elif defined __USE_XOPEN2K
#   define _POSIX_VERSION    200112L
#elif defined __USE_POSIX199506
#   define _POSIX_VERSION    199506L
#elif defined __USE_POSIX199309
#   define _POSIX_VERSION    199309L
#else
#   define _POSIX_VERSION    199009L
#endif
#ifdef __USE_XOPEN2K8
#   define __POSIX2_THIS_VERSION    200809L
#elif defined __USE_XOPEN2K
#   define __POSIX2_THIS_VERSION    200112L
#elif defined __USE_POSIX199506
#   define __POSIX2_THIS_VERSION    199506L
#else
#   define __POSIX2_THIS_VERSION    199209L
#endif

#define _POSIX2_VERSION   __POSIX2_THIS_VERSION
#define _POSIX2_C_VERSION __POSIX2_THIS_VERSION
#define _POSIX2_C_BIND    __POSIX2_THIS_VERSION
#define _POSIX2_C_DEV     __POSIX2_THIS_VERSION
#define _POSIX2_SW_DEV    __POSIX2_THIS_VERSION
#define _POSIX2_LOCALEDEF __POSIX2_THIS_VERSION

#ifdef __USE_XOPEN2K8
#   define _XOPEN_VERSION    700
#elif defined __USE_XOPEN2K
#   define _XOPEN_VERSION    600
#elif defined __USE_UNIX98
#   define _XOPEN_VERSION    500
#else
#   define _XOPEN_VERSION    4
#endif
#define _XOPEN_XCU_VERSION   4
#define _XOPEN_XPG2          1
#define _XOPEN_XPG3          1
#define _XOPEN_XPG4          1
#define _XOPEN_UNIX          1
#define _XOPEN_CRYPT         1
#define _XOPEN_ENH_I18N      1
#define _XOPEN_LEGACY        1

#define STDIN_FILENO  0 /* Standard input.  */
#define STDOUT_FILENO 1 /* Standard output.  */
#define STDERR_FILENO 2 /* Standard error output.  */

#ifndef __ssize_t_defined
#define __ssize_t_defined 1
typedef __ssize_t ssize_t;
#endif /* !__ssize_t_defined */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef NULL
#define NULL __NULLPTR
#endif /* !NULL */

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K)
#ifndef __gid_t_defined
#define __gid_t_defined 1
typedef __gid_t gid_t;
#endif /* !__gid_t_defined */
#ifndef __uid_t_defined
#define __uid_t_defined 1
typedef __uid_t uid_t;
#endif /* !__uid_t_defined */
#ifndef __off_t_defined
#define __off_t_defined
typedef __typedef_off_t off_t;
#endif /* !__off_t_defined */
#ifndef __useconds_t_defined
#define __useconds_t_defined 1
typedef __useconds_t useconds_t;
#endif /* !__useconds_t_defined */
#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif /* !__pid_t_defined */
#ifdef __USE_LARGEFILE64
#ifndef __off64_t_defined
#define __off64_t_defined 1
typedef __off64_t off64_t;
#endif /* !__off64_t_defined */
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN || __USE_XOPEN2K */

#if defined(__USE_XOPEN_EXTENDED) || \
    defined(__USE_XOPEN2K)
#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t intptr_t;
#endif /* !__intptr_t_defined */
#endif

#if defined(__USE_MISC) || defined(__USE_XOPEN)
#ifndef __socklen_t_defined
#define __socklen_t_defined 1
typedef __socklen_t socklen_t;
#endif /* !__socklen_t_defined */
#endif

#define F_OK 0 /* Test for existence.  */
#define X_OK 1 /* Test for execute permission.  */
#define W_OK 2 /* Test for write permission.  */
#define R_OK 4 /* Test for read permission.  */

#ifndef SEEK_SET
#   define SEEK_SET  0 /* Seek from beginning of file.  */
#   define SEEK_CUR  1 /* Seek from current position.  */
#   define SEEK_END  2 /* Seek from end of file.  */
#ifdef __USE_GNU
#   define SEEK_DATA 3 /* Seek to next data.  */
#   define SEEK_HOLE 4 /* Seek to next hole.  */
#endif /* __USE_GNU */
#endif

#ifdef __USE_MISC
#ifndef L_SET
#   define L_SET  SEEK_SET
#   define L_INCR SEEK_CUR
#   define L_XTND SEEK_END
#endif /* !L_SET */
#endif


/* Argument types used by exec() and spawn() functions. */
#ifndef __TARGV
#ifdef __USE_DOS
#   define __TARGV  char const *const *___argv
#   define __TENVP  char const *const *___envp
#else
#   define __TARGV  char *const ___argv[]
#   define __TENVP  char *const ___envp[]
#endif
#endif /* !__TARGV */


#ifndef __KERNEL__

#ifndef ____environ_defined
#define ____environ_defined 1
#ifndef __NO_ASMNAME
#undef __environ
#ifdef __DOS_COMPAT__
__LIBC char **__environ __ASMNAME("_environ");
#else /* __DOS_COMPAT__ */
__LIBC char **__environ __ASMNAME("environ");
#endif /* !__DOS_COMPAT__ */
#else /* __NO_ASMNAME */
#ifdef __DOS_COMPAT__
#undef _environ
__LIBC char **_environ;
#define __environ _environ
#else /* __DOS_COMPAT__ */
#undef environ
__LIBC char **environ;
#define __environ environ
#endif /* !__DOS_COMPAT__ */
#endif /* !__NO_ASMNAME */
#endif /* !____environ_defined */

#ifndef __execl_defined
#define __execl_defined 1
__LIBC __NONNULL((1)) __ATTR_SENTINEL int (__ATTR_CDECL execl)(char const *__restrict __path, char const *__args, ...) __UFS_FUNC_OLDPEA(execl);
__LIBC __NONNULL((1)) __ATTR_SENTINEL_O(1) int (__ATTR_CDECL execle)(char const *__restrict __path, char const *__args, ...) __UFS_FUNC_OLDPEA(execle);
__LIBC __NONNULL((1)) __ATTR_SENTINEL int (__ATTR_CDECL execlp)(char const *__restrict __file, char const *__args, ...) __PE_FUNC_OLDPEA(execlp);
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,execv,(char const *__restrict __path, __TARGV),execv,(__path,___argv))
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1,2,3)),int,__LIBCCALL,execve,(char const *__restrict __path, __TARGV, __TENVP),execve,(__path,___argv,___envp))
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,execvp,(char const *__restrict __file, __TARGV),execvp,(__path,___argv))
#endif /* !__execl_defined */

__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__WUNUSED,__pid_t,__LIBCCALL,getpid,(void),getpid,())

#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,__dos_pipe,(int __pipedes[2], __UINT32_TYPE__ __pipesize, int __textmode),_pipe,(__pipedes,__pipesize,__textmode))
__LOCAL __WUNUSED_SUGGESTED __NONNULL((1)) int (__LIBCCALL pipe)(int __pipedes[2]) { return __dos_pipe(__pipedes,4096,0x8000); /*O_BINARY*/ }
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED_SUGGESTED __NONNULL((1)) int (__LIBCCALL pipe)(int __pipedes[2]);
#endif /* !__DOS_COMPAT__ */

#ifndef __sleep_defined
#define __sleep_defined 1
#ifdef __DOS_COMPAT__
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__dos_sleep,(__UINT32_TYPE__ __duration),_sleep,(__duration))
__LOCAL unsigned int (__LIBCCALL sleep)(unsigned int __seconds) { __dos_sleep(__seconds); return 0; }
#else /* __DOS_COMPAT__ */
__LIBC unsigned int (__LIBCCALL sleep)(unsigned int __seconds);
#endif /* !__DOS_COMPAT__ */
#endif /* !__sleep_defined */

#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,,int,__LIBCCALL,fsync,(int __fd),_commit,(__fd))
#else /* __DOS_COMPAT__ */
__LIBC int (__LIBCCALL fsync)(int __fd);
#endif /* !__DOS_COMPAT__ */

#ifdef __CRT_GLC
__LIBC __PORT_NODOS __WUNUSED __pid_t (__LIBCCALL getppid)(void);
__LIBC __PORT_NODOS __WUNUSED __pid_t (__LIBCCALL getpgrp)(void);
__LIBC __PORT_NODOS __WUNUSED __pid_t (__LIBCCALL __getpgid)(__pid_t __pid);
__LIBC __PORT_NODOS int (__LIBCCALL setpgid)(__pid_t __pid, __pid_t __pgid);
__LIBC __PORT_NODOS __pid_t (__LIBCCALL setsid)(void);
__LIBC __PORT_NODOS __WUNUSED __uid_t (__LIBCCALL getuid)(void);
__LIBC __PORT_NODOS __WUNUSED __uid_t (__LIBCCALL geteuid)(void);
__LIBC __PORT_NODOS __WUNUSED __gid_t (__LIBCCALL getgid)(void);
__LIBC __PORT_NODOS __WUNUSED __gid_t (__LIBCCALL getegid)(void);
__LIBC __PORT_NODOS int (__LIBCCALL getgroups)(int __size, __gid_t __list[]);
__LIBC __PORT_NODOS int (__LIBCCALL setuid)(__uid_t __uid);
__LIBC __PORT_NODOS int (__LIBCCALL setgid)(__gid_t __gid);
__LIBC __PORT_NODOS __WUNUSED __pid_t (__LIBCCALL fork)(void);
__LIBC __PORT_NODOS unsigned int (__LIBCCALL alarm)(unsigned int __seconds);
__LIBC __PORT_NODOS int (__LIBCCALL pause)(void);
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,chown,(char const *__file, __uid_t __owner, __gid_t __group),chown,(__file,__owner,__group))
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1)),long int,__LIBCCALL,pathconf,(char const *__path, int __name),pathconf,(__path,__name))
__LIBC __PORT_NODOS __WUNUSED long int (__LIBCCALL fpathconf)(int __fd, int __name);
__LIBC __PORT_NODOS __WUNUSED char *(__LIBCCALL ttyname)(int __fd);
__LIBC __PORT_NODOS __NONNULL((2)) int (__LIBCCALL ttyname_r)(int __fd, char *__buf, size_t __buflen);
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1,2)),int,__LIBCCALL,link,(char const *__from, char const *__to),link,(__from,__to))
__LIBC __PORT_NODOS __WUNUSED __pid_t (__LIBCCALL tcgetpgrp)(int __fd);
__LIBC __PORT_NODOS int (__LIBCCALL tcsetpgrp)(int __fd, __pid_t __pgrp_id);
__LIBC __PORT_NODOS __WUNUSED char *(__LIBCCALL getlogin)(void);
#endif /* __CRT_GLC */

#ifndef ___exit_defined
#define ___exit_defined 1
__LIBC __ATTR_NORETURN void (__LIBCCALL _exit)(int __status);
#endif /* !___exit_defined */
#ifndef __read_defined
#define __read_defined 1
#if (defined(__DOS_COMPAT__) || defined(__PE__)) && __SIZEOF_SIZE_T__ == 4
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __NONNULL((2)),ssize_t,__LIBCCALL,read,(int __fd, void *__buf, size_t __n_bytes),_write,(__fd,__buf,__n_bytes))
#elif defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __NONNULL((2)),__INT32_TYPE__,__LIBCCALL,__read32,(int __fd, void *__buf, __UINT32_TYPE__ __n_bytes),_read,(__fd,__buf,__n_bytes))
__LOCAL __WUNUSED_SUGGESTED __NONNULL((2)) ssize_t (__LIBCCALL read)(int __fd, void *__buf, size_t __n_bytes) { return __read32(__fd,__buf,__n_bytes > (size_t)(__UINT32_TYPE__)-1 ? (__UINT32_TYPE__)-1 : (__UINT32_TYPE__)__n_bytes); }
#else
__LIBC __WUNUSED_SUGGESTED __NONNULL((2)) ssize_t (__LIBCCALL read)(int __fd, void *__buf, size_t __n_bytes);
#endif
#endif /* !__read_defined */
#ifndef __write_defined
#define __write_defined 1
#if (defined(__DOS_COMPAT__) || defined(__PE__)) && __SIZEOF_SIZE_T__ == 4
__REDIRECT(__LIBC,__NONNULL((2)),ssize_t,__LIBCCALL,write,(int __fd, void const *__buf, size_t __n_bytes),_write,(__fd,__buf,__n_bytes))
#elif defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__NONNULL((2)),__INT32_TYPE__,__LIBCCALL,__write32,(int __fd, void const *__buf, __UINT32_TYPE__ __n_bytes),_write,(__fd,__buf,__n_bytes))
__LOCAL __NONNULL((2)) ssize_t (__LIBCCALL write)(int __fd, void const *__buf, size_t __n_bytes) { return __write32(__fd,__buf,__n_bytes > (size_t)(__UINT32_TYPE__)-1 ? (__UINT32_TYPE__)-1 : (__UINT32_TYPE__)__n_bytes); }
#else
__LIBC __NONNULL((2)) ssize_t (__LIBCCALL write)(int __fd, void const *__buf, size_t __n_bytes);
#endif
#endif /* !__write_defined */
#ifndef __lseek_defined
#define __lseek_defined 1
#if defined(__DOS_COMPAT__) || defined(__PE__)
#ifdef __USE_FILE_OFFSET64
__REDIRECT(__LIBC,,__FS_TYPE(off),__LIBCCALL,lseek,(int __fd, __FS_TYPE(off) __offset, int __whence),_lseeki64,(__fd,__offset,__whence))
#else /* __USE_FILE_OFFSET64 */
__REDIRECT(__LIBC,,__FS_TYPE(off),__LIBCCALL,lseek,(int __fd, __FS_TYPE(off) __offset, int __whence),_lseek,(__fd,__offset,__whence))
#endif /* !__USE_FILE_OFFSET64 */
#else /* __DOS_COMPAT__ || __PE__ */
__REDIRECT_FS_FUNC(__LIBC,,__FS_TYPE(off),__LIBCCALL,lseek,(int __fd, __FS_TYPE(off) __offset, int __whence),lseek,(__fd,__offset,__whence))
#endif /* !__DOS_COMPAT__ && !__PE__ */
#endif /* !__lseek_defined */
#ifndef __isatty_defined
#define __isatty_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,__WUNUSED,int,__LIBCCALL,isatty,(int __fd),isatty,(__fd))
#endif /* !__isatty_defined */
#ifndef __dup2_defined
#define __dup2_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,,int,__LIBCCALL,dup2,(int __ofd, int __nfd),dup2,(__ofd,__nfd))
#endif /* !__dup2_defined */
#ifndef __dup_defined
#define __dup_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,__WUNUSED,int,__LIBCCALL,dup,(int __fd),dup,(__fd))
#endif /* !__dup_defined */
#ifndef __close_defined
#define __close_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,,int,__LIBCCALL,close,(int __fd),close,(__fd))
#endif /* !__close_defined */
#ifndef __access_defined
#define __access_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,access,(char const *__name, int __type),access,(__name,__type))
#endif /* !__access_defined */
#ifndef __chdir_defined
#define __chdir_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,chdir,(char const *__path),chdir,(__path))
#endif /* !__chdir_defined */
#ifndef __getcwd_defined
#define __getcwd_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__WUNUSED_SUGGESTED,char *,__LIBCCALL,getcwd,(char *__buf, size_t __size),_getcwd,(__buf,__size))
#endif /* !__getcwd_defined */
#ifndef __unlink_defined
#define __unlink_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,unlink,(char const *__name),unlink,(__name))
#endif /* !__unlink_defined */
#ifndef __rmdir_defined
#define __rmdir_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,rmdir,(char const *__path),rmdir,(__path))
#endif /* !__rmdir_defined */

#if defined(__USE_KOS) && defined(__CRT_KOS)
__REDIRECT_UFS(__LIBC,__PORT_KOSONLY_ALT(chdir) __NONNULL((2)),int,__LIBCCALL,fchdirat,
              (int __dfd, char const *__path, int __flags),fchdirat,(__dfd,__path,__flags))
#endif /* __USE_KOS && __CRT_KOS */

#ifdef __USE_GNU
#ifdef __DOS_COMPAT__
__REDIRECT_UFS_(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,euidaccess,(char const *__name, int __type),_access,(__name,__type))
__REDIRECT_UFS_(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,eaccess,(char const *__name, int __type),_access,(__name,__type))
#else /* __DOS_COMPAT__ */
__REDIRECT_UFS_(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,euidaccess,(char const *__name, int __type),eaccess,(__name,__type))
__REDIRECT_UFS(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,eaccess,(char const *__name, int __type),eaccess,(__name,__type))
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_GNU */

#ifdef __USE_ATFILE
#ifdef __CRT_GLC
__REDIRECT_UFS(__LIBC,__PORT_NODOS_ALT(access) __NONNULL((2)),int,__LIBCCALL,faccessat,(int __fd, char const *__file, int __type, int __flag),faccessat,(__fd,__file,__type,__flag))
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((2)),int,__LIBCCALL,fchownat,(int __fd, char const *__file, __uid_t __owner, __gid_t __group, int __flag),fchownat,(__fd,__file,__owner,__group))
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((2,4)),int,__LIBCCALL,linkat,(int __fromfd, char const *__from, int __tofd, char const *__to, int __flags),linkat,(__fromfd,__from,__tofd,__to,__flags))
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1,3)),int,__LIBCCALL,symlinkat,(char const *__from, int __tofd, char const *__to),symlinkat,(__from,__tofd,__to))
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((2,3)),ssize_t,__LIBCCALL,readlinkat,(int __fd, char const *__restrict __path, char *__restrict __buf, size_t __buflen),readlinkat,(__fd,__path,__buf,__buflen))
__REDIRECT_UFS(__LIBC,__PORT_NODOS_ALT(unlink) __NONNULL((2)),int,__LIBCCALL,unlinkat,(int __fd, char const *__name, int __flag),unlinkat,(__fd,__name,__flag))
#endif /* __CRT_GLC */
#endif /* __USE_ATFILE */

#ifdef __USE_LARGEFILE64
__REDIRECT_IFDOS(__LIBC,,__off64_t,__LIBCCALL,lseek64,(int __fd, __off64_t __offset, int __whence),_lseeki64,(__fd,__offset,__whence))
#endif /* __USE_LARGEFILE64 */

#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
#ifdef __DOS_COMPAT__
/* It may not be quick, and it may not be SMP-safe, but it'll still do the job! */
#define __DEFINE_PREADWRITE(off_t,lseek,readwrite) \
{ ssize_t __result; off_t __oldpos; \
  if ((__oldpos = lseek(__fd,0,SEEK_CUR)) < 0) return -1; \
  if (lseek(__fd,__offset,SEEK_SET) < 0) return -1; \
  __result = readwrite(__fd,__buf,__n_bytes); \
  lseek(__fd,__oldpos,SEEK_SET); \
  return __result; \
}
__LOCAL __WUNUSED_SUGGESTED __WUNUSED ssize_t (__LIBCCALL pread)(int __fd, void *__buf, size_t __n_bytes, __FS_TYPE(off) __offset) __DEFINE_PREADWRITE(__FS_TYPE(off),lseek,read)
__LOCAL __WUNUSED ssize_t (__LIBCCALL pwrite)(int __fd, void const *__buf, size_t __n_bytes, __FS_TYPE(off) __offset) __DEFINE_PREADWRITE(__FS_TYPE(off),lseek,write)
#ifdef __USE_LARGEFILE64
__LOCAL __WUNUSED_SUGGESTED __WUNUSED ssize_t (__LIBCCALL pread64)(int __fd, void *__buf, size_t __n_bytes, __off64_t __offset) __DEFINE_PREADWRITE(__off64_t,lseek64,read)
__LOCAL __WUNUSED ssize_t (__LIBCCALL pwrite64)(int __fd, void const *__buf, size_t __n_bytes, __off64_t __offset) __DEFINE_PREADWRITE(__off64_t,lseek64,write)
#endif /* __USE_LARGEFILE64 */
#undef __DEFINE_PREADWRITE
#else /* __DOS_COMPAT__ */
__REDIRECT_FS_FUNC(__LIBC,__WUNUSED_SUGGESTED __WUNUSED,ssize_t,__LIBCCALL,pread,(int __fd, void *__buf, size_t __n_bytes, __FS_TYPE(off) __offset),pread,(__fd,__buf,__n_bytes,__offset))
__REDIRECT_FS_FUNC(__LIBC,__WUNUSED,ssize_t,__LIBCCALL,pwrite,(int __fd, void const *__buf, size_t __n_bytes, __FS_TYPE(off) __offset),pwrite,(__fd,__buf,__n_bytes,__offset))
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED_SUGGESTED __WUNUSED ssize_t (__LIBCCALL pread64)(int __fd, void *__buf, size_t __n_bytes, __off64_t __offset);
__LIBC __WUNUSED ssize_t (__LIBCCALL pwrite64)(int __fd, void const *__buf, size_t __n_bytes, __off64_t __offset);
#endif /* __USE_LARGEFILE64 */
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */

#ifdef __USE_GNU
#ifndef __environ_defined
#define __environ_defined 1
#undef environ
#ifdef __DOS_COMPAT__
/* Try to maintain binary compatibility with DOS.
 * Note though, that LIBC exports `environ' and `__p__environ' in DOS and KOS mode. */
__LIBC __WUNUSED char ***__NOTHROW((__LIBCCALL __p__environ)(void));
#define environ  (*__p__environ())
#else /* __DOS_COMPAT__ */
#ifdef __PE__
__LIBC char **environ __ASMNAME("_environ");
#else /* __PE__ */
__LIBC char **environ;
#endif /* !__PE__ */
#if defined(__PE__) || defined(__USE_KOS)
/* Always defining `environ' as a macro is a KOS extension.
 * Otherwise, we mimic PE behavior which always defines `environ' as a macro. */
#define environ  environ
#endif /* __PE__ || __USE_KOS */
#endif /* !__DOS_COMPAT__ */
#endif /* !__environ_defined */
#ifndef __execvpe_defined
#define __execvpe_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1,2,3)),int,__LIBCCALL,execvpe,(char const *__file, __TARGV, __TENVP),execvpe,(__file,___argv,___envp))
#endif /* !__execvpe_defined */
#ifdef __DOS_COMPAT__
__LOCAL int (__LIBCCALL pipe2)(int __pipedes[2], int __flags) { return __dos_pipe(__pipedes,4096,0x8000|__flags); }
__LOCAL int (__LIBCCALL dup3)(int __fd, int __fd2, int __UNUSED(__flags)) { return __fd != __fd2 ? dup2(__fd,__fd2) : -1; }
__LOCAL __WUNUSED char *(__LIBCCALL get_current_dir_name)(void) { return getcwd(NULL,0); }
__LOCAL int (__LIBCCALL syncfs)(int __UNUSED(__fd)) {}
#else /* __DOS_COMPAT__ */
__LIBC int (__LIBCCALL pipe2)(int __pipedes[2], int __flags);
__LIBC int (__LIBCCALL dup3)(int __fd, int __fd2, int __flags);
__LIBC __WUNUSED char *(__LIBCCALL get_current_dir_name)(void);
__LIBC int (__LIBCCALL syncfs)(int __fd);
#endif /* !__DOS_COMPAT__ */
#ifdef __CRT_GLC
__LIBC __PORT_NODOS int (__LIBCCALL group_member)(__gid_t __gid);
__LIBC __PORT_NODOS int (__LIBCCALL getresuid)(__uid_t *__ruid, __uid_t *__euid, __uid_t *__suid);
__LIBC __PORT_NODOS int (__LIBCCALL getresgid)(__gid_t *__rgid, __gid_t *__egid, __gid_t *__sgid);
__LIBC __PORT_NODOS __WUNUSED int (__LIBCCALL setresuid)(__uid_t __ruid, __uid_t __euid, __uid_t __suid);
__LIBC __PORT_NODOS __WUNUSED int (__LIBCCALL setresgid)(__gid_t __rgid, __gid_t __egid, __gid_t __sgid);
#endif /* __CRT_GLC */
#endif /* __USE_GNU */

#if (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8)) || \
     defined(__USE_MISC)
#ifdef __DOS_COMPAT__
/* Hidden function exported by DOS that allows for millisecond precision. */
#ifndef ____dos_sleep_mili_defined
#define ____dos_sleep_mili_defined 1
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__dos_sleep_mili,(__UINT32_TYPE__ __msecs),__crtSleep,(__msecs))
#endif /* !____dos_sleep_mili_defined */
__LOCAL int (__LIBCCALL usleep)(__useconds_t __useconds) { return __dos_sleep_mili(__useconds/1000l /*USEC_PER_MSEC*/); }
__LOCAL __NONNULL((1)) __ATTR_DEPRECATED("Use getcwd()") char *(__LIBCCALL getwd)(char *__buf) { return getcwd(__buf,(size_t)-1); }
#else /* __DOS_COMPAT__ */
__LIBC int (__LIBCCALL usleep)(__useconds_t __useconds);
__LIBC __NONNULL((1)) __ATTR_DEPRECATED("Use getcwd()") char *(__LIBCCALL getwd)(char *__buf);
#endif /* !__DOS_COMPAT__ */
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __useconds_t (__LIBCCALL ualarm)(__useconds_t __value, __useconds_t __interval);
__LIBC __PORT_NODOS __WUNUSED __pid_t (__LIBCCALL vfork)(void);
#endif /* __CRT_GLC */
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#ifdef __CRT_GLC
__LIBC __PORT_NODOS int (__LIBCCALL fchown)(int __fd, __uid_t __owner, __gid_t __group);
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,lchown,
              (char const *__file, __uid_t __owner, __gid_t __group),lchown,(__file,__owner,__group))
__LIBC __PORT_NODOS_ALT(chdir) int (__LIBCCALL fchdir)(int __fd);
__LIBC __PORT_NODOS __WUNUSED __pid_t (__LIBCCALL getpgid)(__pid_t __pid);
__LIBC __PORT_NODOS __WUNUSED __pid_t (__LIBCCALL getsid)(__pid_t __pid);
__REDIRECT_UFS_FUNCn(__LIBC,__NONNULL((1)),int,__LIBCCALL,truncate,
                    (char const *__file, __FS_TYPE(off) __length),truncate,(__file,__length))
#ifdef __USE_LARGEFILE64
__REDIRECT_UFS(__LIBC,__NONNULL((1)),int,__LIBCCALL,truncate64,
              (char const *__file, __off64_t __length),truncate64,(__file,__length))
#endif /* __USE_LARGEFILE64 */
#else /* __CRT_GLC */
__LOCAL __NONNULL((1)) int (__LIBCCALL truncate)(char const *__file, __FS_TYPE(off) __length) { return 0; /* TODO: open()+ftruncate(); */ }
#ifdef __USE_LARGEFILE64
__LOCAL __NONNULL((1)) int (__LIBCCALL truncate64)(char const *__file, __off64_t __length) { return 0; /* TODO: open()+ftruncate64(); */ }
#endif /* __USE_LARGEFILE64 */
#endif /* !__CRT_GLC */
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */

#ifdef __USE_XOPEN2K8
#ifdef __CRT_GLC
__LIBC __PORT_NODOS_ALT(execve) __NONNULL((2)) int (__LIBCCALL fexecve)(int __fd, __TARGV, __TENVP);
#endif /* __CRT_GLC */
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_KOS
#ifndef __execlpe_defined
#define __execlpe_defined 1
#ifdef __GLC_COMPAT__
__LOCAL __NONNULL((1)) __ATTR_SENTINEL
int (__ATTR_CDECL execlpe)(char const *__file, char const *__args, ...) {
 /* TODO: Not exported by GLibc. - Must forward call to `execvpe()' */
 return 0;
}
#else /* __GLC_COMPAT__ */
/* TODO: Use redirection. */
__LIBC __NONNULL((1)) __ATTR_SENTINEL_O(1) int (__LIBCCALL execlpe)(char const *__restrict __file, char const *__args, ...) __UFS_FUNC_OLDPEA(execlpe);
#endif /* !__GLC_COMPAT__ */
#endif /* !__execlpe_defined */

#ifdef __CRT_KOS
/* TODO: When linking against GLibc, we could redirect these against 'fexecve' */
__LIBC __PORT_KOSONLY_ALT(execl) __NONNULL((2)) __ATTR_SENTINEL int (__ATTR_CDECL fexecl)(int __fd, char const *__args, ...);
__LIBC __PORT_KOSONLY_ALT(execle) __NONNULL((2)) __ATTR_SENTINEL int (__ATTR_CDECL fexecle)(int __fd, char const *__args, ...);
__LIBC __PORT_KOSONLY_ALT(execv) __NONNULL((2)) int (__LIBCCALL fexecv)(int __fd, __TARGV);
#endif /* __CRT_KOS */
#endif /* __USE_KOS */

#if defined(__CRT_KOS) && defined(__USE_KOS)
#ifdef __USE_ATFILE
/* At-file style exec functions. */
__LIBC __PORT_KOSONLY_ALT(execl) __ATTR_SENTINEL_O(1) int (__ATTR_CDECL fexeclat)(int __dfd, char const *__path, char const *__args, ... /*, int __flags*/);
__LIBC __PORT_KOSONLY_ALT(execle) __ATTR_SENTINEL_O(2) int (__ATTR_CDECL fexecleat)(int __dfd, char const *__path, char const *__args, ... /*, char *const ___envp[], int __flags*/);
__LIBC __PORT_KOSONLY_ALT(execlp) __NONNULL((1)) __ATTR_SENTINEL_O(1) int (__ATTR_CDECL fexeclpat)(char const *__restrict __file, char const *__args, ... /*, int __flags*/);
__LIBC __PORT_KOSONLY_ALT(execlpe) __NONNULL((1)) __ATTR_SENTINEL_O(2) int (__ATTR_CDECL fexeclpeat)(char const *__restrict __file, char const *__args, ... /*, char *const ___envp[], int __flags*/);
__LIBC __PORT_KOSONLY_ALT(execv) __NONNULL((3)) int (__LIBCCALL fexecvat)(int __dfd, char const *__path, __TARGV, int __flags);
__LIBC __PORT_KOSONLY_ALT(execve) __NONNULL((3)) int (__LIBCCALL fexecveat)(int __dfd, char const *__path, __TARGV, __TENVP, int __flags);
__LIBC __PORT_KOSONLY_ALT(execvp) __NONNULL((1,2)) int (__LIBCCALL fexecvpat)(char const *__restrict __file, __TARGV, int __flags);
__LIBC __PORT_KOSONLY_ALT(execvpe) __NONNULL((1,2)) int (__LIBCCALL fexecvpeat)(char const *__restrict __file, __TARGV, __TENVP, int __flags);
#endif /* __USE_ATFILE */

/* The filesystem mode provided by the KOS kernel is used to hard-configure `AT_*' filesystem flags.
 * When set, any filesystem-related system call will make use of the filesystem mask/mode to change
 * its behavior in accordance to what is requested.
 * The new filesystem mode is calculated as follows:
 *   >> new_mode = (given_mode & fm_mask) | fm_mode;
 * With that in mind, the default state is:
 *   >> fm_mask == -1
 *   >> fm_mode == 0
 * Note that not all bits can be configured.
 *   The masks of immutable bits can be reviewed within the kernel sources
 *   /src/include/fs/fd.h: FDMAN_FSMASK_ALWAYS1 / FDMAN_FSMODE_ALWAYS0
 * EXAMPLE:
 *   >> Force-enable DOS semantics for all file operations,
 *      regardless of `AT_DOSPATH' or linked libc function:
 *      fm_mask = -1;
 *      fm_mode = AT_DOSPATH;
 *   >> Force-disable DOS semantics for all file operations,
 *      regardless of `AT_DOSPATH' or linked libc function:
 *      fm_mask = ~(AT_DOSPATH);
 *      fm_mode = 0;
 * NOTE: This function never fails and always returns the old mask.
 * HINT: When prefixed before a command, the utility 'dosfs' will
 *       set the fsmask to '-1,AT_DOSPATH' before executing the
 *       remainder of the commandline as another command. */
struct fsmask {
 int fm_mask; /*< Filesystem mode mask. (Set of `AT_*') */
 int fm_mode; /*< Filesystem mode. (Set of `AT_*') */
};
__LIBC __PORT_KOSONLY struct fsmask (__LIBCCALL fsmode)(struct fsmask new_mode);
#endif /* __CRT_KOS && _USE_KOS */

#if defined(__USE_MISC) || defined(__USE_XOPEN)
#ifdef __DOS_COMPAT__
/* It should be sufficient to emulate this is a no-op. */
__LOCAL __WUNUSED int (__LIBCCALL nice)(int __UNUSED(__inc)) { return 0; }
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED int (__LIBCCALL nice)(int __inc);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_MISC || __USE_XOPEN */

#ifdef __USE_POSIX2
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __WUNUSED_SUGGESTED
size_t (__LIBCCALL confstr)(int __name, char *__buf, size_t __buflen);

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("optarg")
#pragma push_macro("optind")
#pragma push_macro("opterr")
#pragma push_macro("optopt")
#endif

#undef optarg
#undef optind
#undef opterr
#undef optopt
__LIBC char *optarg;
__LIBC int optind;
__LIBC int opterr;
__LIBC int optopt;

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("optopt")
#pragma pop_macro("opterr")
#pragma pop_macro("optind")
#pragma pop_macro("optarg")
#endif

#ifndef __getopt_defined
#define __getopt_defined 1
#if defined(__USE_POSIX2) && !defined(__USE_POSIX_IMPLICITLY) && !defined(__USE_GNU)
__REDIRECT(__LIBC,__PORT_NODOS __WUNUSED,int,__LIBCCALL,getopt,
          (int ___argc, char *const *___argv, char const *__shortopts),
           __posix_getopt,(___argc,___argv,__shortopts))
#else /* ... */
__LIBC __PORT_NODOS int (__LIBCCALL getopt)(int ___argc, char *const *___argv, char const *__shortopts);
#endif /* !... */
#endif /* !__getopt_defined */
#endif /* __CRT_GLC */
#endif /* __USE_POSIX2 */

#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
#ifdef __DOS_COMPAT__
__LOCAL void (__LIBCCALL sync)(void) {}
#if defined(__USE_MISC) || !defined(__USE_XOPEN2K)
#if defined(__PAGESIZE)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL getpagesize)(void) { return __PAGESIZE; }
#elif defined(PAGE_SIZE)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL getpagesize)(void) { return PAGE_SIZE; }
#elif defined(PAGESIZE)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL getpagesize)(void) { return PAGESIZE; }
#elif defined(__i386__) || defined(__x86_64__)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL getpagesize)(void) { return 4096; }
#else
#error "ERROR: PAGESIZE not known for this arch"
#endif
#endif /* __USE_MISC || !__USE_XOPEN2K */
#else /* __DOS_COMPAT__ */
__LIBC void (__LIBCCALL sync)(void);
__LIBC __PORT_NODOS int (__LIBCCALL setpgrp)(void);
__LIBC __PORT_NODOS int (__LIBCCALL setreuid)(__uid_t __ruid, __uid_t __euid);
__LIBC __PORT_NODOS int (__LIBCCALL setregid)(__gid_t __rgid, __gid_t __egid);
__LIBC __PORT_NODOS __WUNUSED long int (__LIBCCALL gethostid)(void);
#if defined(__USE_MISC) || !defined(__USE_XOPEN2K)
__LIBC __ATTR_CONST __WUNUSED int (__LIBCCALL getpagesize)(void) ;
__LIBC __PORT_NODOS __WUNUSED int (__LIBCCALL getdtablesize)(void);
#endif /* __USE_MISC || !__USE_XOPEN2K */
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __CRT_GLC
#ifdef __USE_XOPEN2K
__LIBC __PORT_NODOS int (__LIBCCALL seteuid)(__uid_t __uid);
__LIBC __PORT_NODOS int (__LIBCCALL setegid)(__gid_t __gid);
#endif /* __USE_XOPEN2K */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_UNIX98))
__LIBC __PORT_NODOS __WUNUSED int (__LIBCCALL ttyslot)(void);
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1,2)),int,__LIBCCALL,symlink,(char const *__from, char const *__to),symlink,(__from,__to))
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1,2)),ssize_t,__LIBCCALL,readlink,(char const *__restrict __path, char *__restrict __buf, size_t __buflen),readlink,(__path,__buf,__buflen))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#ifdef __USE_KOS
__LIBC __PORT_KOSONLY_ALT(readlink) __NONNULL((2)) ssize_t (__LIBCCALL freadlink)(int __fd, char *__restrict __buf, size_t __buflen);
#endif /* __USE_KOS */

#if defined(__USE_REENTRANT) || defined(__USE_POSIX199506)
__LIBC __PORT_NODOS __WUNUSED_SUGGESTED __NONNULL((1)) int (__LIBCCALL getlogin_r)(char *__name, size_t __name_len);
#endif /* __USE_REENTRANT || __USE_POSIX199506 */

#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K)
__LIBC __PORT_NODOS __WUNUSED_SUGGESTED __NONNULL((1)) int (__LIBCCALL gethostname)(char *__name, size_t __buflen);
#endif /* __USE_UNIX98 || __USE_XOPEN2K */

#ifdef __USE_MISC
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL setlogin)(char const *__name);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sethostname)(char const *__name, size_t __len);
__LIBC __PORT_NODOS int (__LIBCCALL sethostid)(long int __id);
__LIBC __PORT_NODOS __WUNUSED_SUGGESTED __NONNULL((1)) int (__LIBCCALL getdomainname)(char *__name, size_t __buflen);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL setdomainname)(char const *__name, size_t __len);
__LIBC __PORT_NODOS int (__LIBCCALL vhangup)(void);
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,revoke,(char const *__file),revoke,(__file))
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL profil)(unsigned short int *__sample_buffer, size_t __size, size_t __offset, unsigned int __scale);
__REDIRECT_UFS(__LIBC,__PORT_NODOS,int,__LIBCCALL,acct,(char const *__name),acct,(__name))
__LIBC __PORT_NODOS __WUNUSED char *(__LIBCCALL getusershell)(void);
__LIBC __PORT_NODOS void (__LIBCCALL endusershell)(void);
__LIBC __PORT_NODOS void (__LIBCCALL setusershell)(void);
__LIBC __PORT_NODOS int (__LIBCCALL daemon)(int __nochdir, int __noclose);
#if defined(__USE_KOS) && defined(__CRT_KOS)
/* Execute a system call, returning in both EAX and EDX */
#ifdef __NO_ASMNAME
__LIBC __PORT_NODOS long long int (__ATTR_CDECL syscall)(long int __sysno, ...);
#define lsyscall(__VA_ARGS__)             (syscall)(__VA_ARGS__)
#define syscall(__VA_ARGS__)   ((long int)(syscall)(__VA_ARGS__))
#else /* __NO_ASMNAME */
__LIBC __PORT_NODOS long int (__ATTR_CDECL syscall)(long int __sysno, ...);
__LIBC __PORT_KOSONLY long long int (__ATTR_CDECL lsyscall)(long int __sysno, ...) __ASMNAME("syscall");
#endif /* !__NO_ASMNAME */
#else /* __USE_KOS && __CRT_KOS */
__LIBC __PORT_NODOS long int (__ATTR_CDECL syscall)(long int __sysno, ...);
#endif /* !__USE_KOS || !__CRT_KOS */
#endif /* __USE_MISC */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__REDIRECT_UFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,
               chroot,(char const *__restrict __path),chroot,(__path))
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) char *(__LIBCCALL getpass)(char const *__restrict __prompt);
#endif
#endif /* __CRT_GLC */

#if defined(__USE_POSIX199309) || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
#ifdef __DOS_COMPAT__
#ifdef __USE_FILE_OFFSET64
__REDIRECT(__LIBC,,int,__LIBCCALL,ftruncate,(int __fd, __FS_TYPE(off) __length),_chsize_s,(__fd,__length))
#else /* __USE_FILE_OFFSET64 */
__REDIRECT(__LIBC,,int,__LIBCCALL,ftruncate,(int __fd, __FS_TYPE(off) __length),_chsize,(__fd,__length))
#endif /* !__USE_FILE_OFFSET64 */
#else /* __DOS_COMPAT__ */
__REDIRECT_FS_FUNC(__LIBC,,int,__LIBCCALL,ftruncate,(int __fd, __FS_TYPE(off) __length),ftruncate,(__fd,__length))
#endif /* !__DOS_COMPAT__ */
#ifdef __USE_LARGEFILE64
__REDIRECT_IFDOS(__LIBC,,int,__LIBCCALL,ftruncate64,(int __fd, __off64_t __length),_chsize_s,(__fd,__length))
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_POSIX199309 || __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#ifdef __CRT_GLC
#if (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K)) || \
     defined(__USE_MISC)
__LIBC __PORT_NODOS_ALT(free) int (__LIBCCALL brk)(void *__addr);
__LIBC __PORT_NODOS_ALT(malloc) void *(__LIBCCALL sbrk)(intptr_t __delta);
#endif
#endif /* __CRT_GLC */

#if defined(__USE_POSIX199309) || defined(__USE_UNIX98)
__REDIRECT_IFDOS(__LIBC,,int,__LIBCCALL,fdatasync,(int __fd),_commit,(__fd))
#endif /* __USE_POSIX199309 || __USE_UNIX98 */

#ifdef __USE_XOPEN
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __NONNULL((1,2)) char *(__LIBCCALL crypt)(char const *__key, char const *__salt);
__LIBC __PORT_NODOS __NONNULL((1)) void (__LIBCCALL encrypt)(char *__glibc_block, int __edflag);
#endif /* __CRT_GLC */

#ifndef __swab_defined
#define __swab_defined 1
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__ && !defined(__DOS_COMPAT__)
__LIBC __NONNULL((1,2)) void (__LIBCCALL swab)(void const *__restrict __from, void *__restrict __to, int __n_bytes);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__REDIRECT_VOID(__LIBC,__NONNULL((1,2)),__LIBCCALL,swab,(void const *__restrict __from, void *__restrict __to, int __n_bytes),_swab,(__from,__to,__n_bytes))
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !__swab_defined */
#endif /* __USE_XOPEN */

#if defined(__USE_XOPEN) && !defined(__USE_XOPEN2K)
#ifdef __CRT_GLC
#ifndef __ctermid_defined
#define __ctermid_defined 1
__LIBC __PORT_NODOS __WUNUSED_SUGGESTED char *(__LIBCCALL ctermid)(char *__s);
#endif /* !__ctermid_defined */
#endif /* __USE_XOPEN && !__USE_XOPEN2K */
__LIBC __PORT_NODOS __WUNUSED long int (__LIBCCALL sysconf)(int __name);
#endif /* __CRT_GLC */
#endif /* !__KERNEL__ */


#if !defined(F_LOCK) && (defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED))
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

#ifdef __USE_GNU
#ifdef __COMPILER_HAVE_TYPEOF
#define TEMP_FAILURE_RETRY(expression) \
 __XBLOCK({ __typeof__(expression) __result; \
            do __result = (expression); \
            while (__result == -1L && errno == EINTR); \
            __XRETURN __result; }))
#elif defined(__COMPILER_HAVE_AUTOTYPE)
#define TEMP_FAILURE_RETRY(expression) \
 __XBLOCK({ __auto_type __result; \
            do __result = (expression); \
            while (__result == -1L && errno == EINTR); \
            __XRETURN __result; }))
#else
#define TEMP_FAILURE_RETRY(expression) \
 __XBLOCK({ long int __result; \
            do __result = (long int)(expression); \
            while (__result == -1L && errno == EINTR); \
            __XRETURN __result; }))
#endif
#endif

__SYSDECL_END

#endif /* !_UNISTD_H */
