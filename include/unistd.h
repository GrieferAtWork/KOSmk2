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
#include <asm/unistd.h>
#include <features.h>
#include <bits/types.h>
#include <bits/confname.h>
#include <bits/posix_opt.h>
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K)
#include <bits/environments.h>
#endif

__DECL_BEGIN

/* Disclaimer: Code below is based off of /usr/include/unistd.h */

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
#endif

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

#ifndef __KERNEL__

#undef __environ
__LIBC char **__environ __ASMNAME2("environ","_environ");

__LIBC __NONNULL((1,2)) __ATTR_SENTINEL int (__ATTR_CDECL execl)(char const *__path, char const *__args, ...) __UFS_FUNC_OLDPEA(execl);
__LIBC __NONNULL((1,2)) __ATTR_SENTINEL int (__ATTR_CDECL execle)(char const *__path, char const *__args, ...) __UFS_FUNC_OLDPEA(execle);
__LIBC __NONNULL((1,2)) __ATTR_SENTINEL int (__ATTR_CDECL execlp)(char const *__file, char const *__args, ...) __PE_FUNC_OLDPEA(execlp);
__LIBC __NONNULL((1,2)) int (__LIBCCALL execv)(char const *__path, char *const __argv[]) __UFS_FUNC_OLDPEA(execv);
__LIBC __NONNULL((1,2)) int (__LIBCCALL execve)(char const *__path, char *const __argv[], char *const __envp[]) __UFS_FUNC_OLDPEA(execve);
__LIBC __NONNULL((1,2)) int (__LIBCCALL execvp)(char const *__file, char *const __argv[]) __PE_FUNC_OLDPEA(execvp);
__LIBC __pid_t (__LIBCCALL getpid)(void) __PE_FUNC_OLDPEA(getpid);
__LIBC __pid_t (__LIBCCALL getppid)(void);
__LIBC __pid_t (__LIBCCALL getpgrp)(void);
__LIBC __pid_t (__LIBCCALL __getpgid)(__pid_t __pid);
__LIBC int (__LIBCCALL setpgid)(__pid_t __pid, __pid_t __pgid);
__LIBC __pid_t (__LIBCCALL setsid)(void);
__LIBC __uid_t (__LIBCCALL getuid)(void);
__LIBC __uid_t (__LIBCCALL geteuid)(void);
__LIBC __gid_t (__LIBCCALL getgid)(void);
__LIBC __gid_t (__LIBCCALL getegid)(void);
__LIBC int (__LIBCCALL getgroups)(int __size, __gid_t __list[]);
__LIBC int (__LIBCCALL setuid)(__uid_t __uid);
__LIBC int (__LIBCCALL setgid)(__gid_t __gid);
__LIBC __pid_t (__LIBCCALL fork)(void);
__LIBC int (__LIBCCALL pipe)(int __pipedes[2]);
__LIBC unsigned int (__LIBCCALL alarm)(unsigned int __seconds);
#ifndef __sleep_defined
#define __sleep_defined 1
__LIBC unsigned int (__LIBCCALL sleep)(unsigned int __seconds) __PE_ASMNAME("_sleep");
#endif /* !__sleep_defined */
__LIBC int (__LIBCCALL pause)(void);
__LIBC __NONNULL((1)) int (__LIBCCALL chown)(char const *__file, __uid_t __owner, __gid_t __group) __UFS_FUNC(chown);
__LIBC __NONNULL((1)) long int (__LIBCCALL pathconf)(char const *__path, int __name) __UFS_FUNC(pathconf);
__LIBC long int (__LIBCCALL fpathconf)(int __fd, int __name);
__LIBC __WUNUSED char *(__LIBCCALL ttyname)(int __fd);
__LIBC __NONNULL((2)) int (__LIBCCALL ttyname_r)(int __fd, char *__buf, size_t __buflen);
__LIBC __NONNULL((1,2)) int (__LIBCCALL link)(char const *__from, char const *__to) __UFS_FUNC(link);
__LIBC __pid_t (__LIBCCALL tcgetpgrp)(int __fd);
__LIBC int (__LIBCCALL tcsetpgrp)(int __fd, __pid_t __pgrp_id);
__LIBC __WUNUSED char *(__LIBCCALL getlogin)(void);
__LIBC int (__LIBCCALL fsync)(int __fd);

#ifndef ___exit_defined
#define ___exit_defined 1
__LIBC __ATTR_NORETURN void (__LIBCCALL _exit)(int __status);
#endif /* !___exit_defined */
#ifndef __read_defined
#define __read_defined 1
#if defined(__PE__) && __SIZEOF_SIZE_T__ == 4
__LIBC ssize_t (__LIBCCALL read)(int __fd, void *__buf, size_t __n_bytes) __ASMNAME("_read");
#else
__LIBC ssize_t (__LIBCCALL read)(int __fd, void *__buf, size_t __n_bytes);
#endif
#endif /* !__read_defined */
#ifndef __write_defined
#define __write_defined 1
#if defined(__PE__) && __SIZEOF_SIZE_T__ == 4
__LIBC ssize_t (__LIBCCALL write)(int __fd, void const *__buf, size_t __n_bytes) __ASMNAME("_write");
#else
__LIBC ssize_t (__LIBCCALL write)(int __fd, void const *__buf, size_t __n_bytes);
#endif
#endif /* !__write_defined */
#ifndef __lseek_defined
#define __lseek_defined 1
#ifdef __PE__
#ifdef __USE_FILE_OFFSET64
__LIBC __FS_TYPE(off) (__LIBCCALL lseek)(int __fd, __FS_TYPE(off) __offset, int __whence) __ASMNAME("_lseeki64");
#else /* __USE_FILE_OFFSET64 */
__LIBC __FS_TYPE(off) (__LIBCCALL lseek)(int __fd, __FS_TYPE(off) __offset, int __whence) __ASMNAME("_lseek");
#endif /* !__USE_FILE_OFFSET64 */
#else /* __PE__ */
__LIBC __FS_TYPE(off) (__LIBCCALL lseek)(int __fd, __FS_TYPE(off) __offset, int __whence) __FS_FUNC(lseek);
#endif /* !__PE__ */
#endif /* !__lseek_defined */
#ifndef __isatty_defined
#define __isatty_defined 1
__LIBC __WUNUSED int (__LIBCCALL isatty)(int __fd) __PE_FUNC_OLDPEA(isatty);
#endif /* !__isatty_defined */
#ifndef __dup2_defined
#define __dup2_defined 1
__LIBC int (__LIBCCALL dup2)(int __ofd, int __nfd) __PE_FUNC_OLDPEA(dup2);
#endif /* !__dup2_defined */
#ifndef __dup_defined
#define __dup_defined 1
__LIBC __WUNUSED int (__LIBCCALL dup)(int __fd) __PE_FUNC_OLDPEA(dup);
#endif /* !__dup_defined */
#ifndef __close_defined
#define __close_defined 1
__LIBC int (__LIBCCALL close)(int __fd) __PE_FUNC_OLDPEA(close);
#endif /* !__close_defined */
#ifndef __access_defined
#define __access_defined 1
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL access)(char const *__name, int __type) __UFS_FUNC_OLDPEA(access);
#endif /* !__access_defined */
#ifndef __chdir_defined
#define __chdir_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL chdir)(char const *__path) __UFS_FUNC_OLDPEA(chdir);
#endif /* !__chdir_defined */
#ifndef __getcwd_defined
#define __getcwd_defined 1
__LIBC char *(__LIBCCALL getcwd)(char *__buf, size_t __size) __PE_FUNC_OLDPEA(getcwd);
#endif /* !__getcwd_defined */
#ifndef __unlink_defined
#define __unlink_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL unlink)(char const *__name) __UFS_FUNC_OLDPEA(unlink);
#endif /* !__unlink_defined */
#ifndef __rmdir_defined
#define __rmdir_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL rmdir)(char const *__path) __UFS_FUNC_OLDPEA(rmdir);
#endif /* !__rmdir_defined */

#ifdef __USE_KOS
__LIBC __NONNULL((2)) int (__LIBCCALL fchdirat)(int __dfd, char const *__path, int __flags) __UFS_FUNC(fchdirat);
#endif /* __USE_KOS */

#ifdef __USE_GNU
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL euidaccess)(char const *__name, int __type) __UFS_FUNC_(eaccess);
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL eaccess)(char const *__name, int __type) __UFS_FUNC(eaccess);
#endif /* __USE_GNU */

#ifdef __USE_ATFILE
__LIBC __NONNULL((2)) int (__LIBCCALL faccessat)(int __fd, char const *__file, int __type, int __flag) __UFS_FUNC(faccessat);
__LIBC __NONNULL((2)) int (__LIBCCALL fchownat)(int __fd, char const *__file, __uid_t __owner, __gid_t __group, int __flag) __UFS_FUNC(fchownat);
__LIBC __NONNULL((2,4)) int (__LIBCCALL linkat)(int __fromfd, char const *__from, int __tofd, char const *__to, int __flags) __UFS_FUNC(linkat);
__LIBC __NONNULL((1,3)) int (__LIBCCALL symlinkat)(char const *__from, int __tofd, char const *__to) __UFS_FUNC(symlinkat);
__LIBC __NONNULL((2,3)) ssize_t (__LIBCCALL readlinkat)(int __fd, char const *__restrict __path, char *__restrict __buf, size_t __len) __UFS_FUNC(readlinkat);
__LIBC __NONNULL((2)) int (__LIBCCALL unlinkat)(int __fd, char const *__name, int __flag) __UFS_FUNC(unlinkat);
#endif /* __USE_ATFILE */

#ifdef __USE_LARGEFILE64
__LIBC __off64_t (__LIBCCALL lseek64)(int __fd, __off64_t __offset, int __whence) __PE_ASMNAME("_lseeki64");
#endif /* __USE_LARGEFILE64 */

#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
__LIBC __WUNUSED ssize_t (__LIBCCALL pread)(int __fd, void *__buf, size_t __n_bytes, __FS_TYPE(off) __offset) __FS_FUNC(pread);
__LIBC __WUNUSED ssize_t (__LIBCCALL pwrite)(int __fd, void const *__buf, size_t __n_bytes, __FS_TYPE(off) __offset) __FS_FUNC(pwrite);
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED ssize_t (__LIBCCALL pread64)(int __fd, void *__buf, size_t __n_bytes, __off64_t __offset);
__LIBC __WUNUSED ssize_t (__LIBCCALL pwrite64)(int __fd, void const *__buf, size_t __n_bytes, __off64_t __offset);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */

#ifdef __USE_GNU
#undef environ
__LIBC char **environ __PE_ASMNAME("_environ");
__LIBC __NONNULL((1,2)) int (__LIBCCALL execvpe)(char const *__file, char *const __argv[], char *const __envp[]) __UFS_FUNC_OLDPEA(execvpe);
__LIBC int (__LIBCCALL pipe2)(int __pipedes[2], int __flags);
__LIBC __WUNUSED char *(__LIBCCALL get_current_dir_name)(void);
__LIBC int (__LIBCCALL dup3)(int __fd, int __fd2, int __flags);
__LIBC int (__LIBCCALL group_member)(__gid_t __gid);
__LIBC int (__LIBCCALL getresuid)(__uid_t *__ruid, __uid_t *__euid, __uid_t *__suid);
__LIBC int (__LIBCCALL getresgid)(__gid_t *__rgid, __gid_t *__egid, __gid_t *__sgid);
__LIBC __WUNUSED int (__LIBCCALL setresuid)(__uid_t __ruid, __uid_t __euid, __uid_t __suid);
__LIBC __WUNUSED int (__LIBCCALL setresgid)(__gid_t __rgid, __gid_t __egid, __gid_t __sgid);
__LIBC int (__LIBCCALL syncfs)(int __fd);
#endif /* __USE_GNU */

#if (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8)) || \
     defined(__USE_MISC)
__LIBC __useconds_t (__LIBCCALL ualarm)(__useconds_t __value, __useconds_t __interval);
__LIBC int (__LIBCCALL usleep)(__useconds_t __useconds);
__LIBC __NONNULL((1)) __ATTR_DEPRECATED("Use getcwd()") char *(__LIBCCALL getwd)(char *__buf);
__LIBC __pid_t (__LIBCCALL vfork)(void);
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
__LIBC int (__LIBCCALL fchown)(int __fd, __uid_t __owner, __gid_t __group);
__LIBC __NONNULL((1)) int (__LIBCCALL lchown)(char const *__file, __uid_t __owner, __gid_t __group) __UFS_FUNC(lchown);
__LIBC int (__LIBCCALL fchdir)(int __fd);
__LIBC __pid_t (__LIBCCALL getpgid)(__pid_t __pid);
__LIBC __pid_t (__LIBCCALL getsid)(__pid_t __pid);
__LIBC __NONNULL((1)) int (__LIBCCALL truncate)(char const *__file, __FS_TYPE(off) __length) __UFS_FUNCn(truncate);
#ifdef __USE_LARGEFILE64
__LIBC __NONNULL((1)) int (__LIBCCALL truncate64)(char const *__file, __off64_t __length) __UFS_FUNC(truncate64);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */

#ifdef __USE_XOPEN2K8
__LIBC __NONNULL((2)) int (__LIBCCALL fexecve)(int __fd, char *const __argv[], char *const __envp[]);
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_KOS
__LIBC __NONNULL((1,2)) __ATTR_SENTINEL int (__ATTR_CDECL execlpe)(char const *__file, char const *__args, ...);
__LIBC __NONNULL((2)) __ATTR_SENTINEL int (__ATTR_CDECL fexecl)(int __fd, char const *__args, ...);
__LIBC __NONNULL((2)) __ATTR_SENTINEL int (__ATTR_CDECL fexecle)(int __fd, char const *__args, ...);
__LIBC __NONNULL((2)) int (__LIBCCALL fexecv)(int __fd, char *const __argv[]);
#endif /* __USE_KOS */

#if defined(__USE_MISC) || defined(__USE_XOPEN)
__LIBC __WUNUSED int (__LIBCCALL nice)(int __inc);
#endif /* __USE_MISC || __USE_XOPEN */

#ifdef __USE_POSIX2
__LIBC size_t (__LIBCCALL confstr)(int __name, char *__buf, size_t __len);

#undef optarg
#undef optind
#undef opterr
#undef optopt
__LIBC char *(optarg);
__LIBC int (optind);
__LIBC int (opterr);
__LIBC int (optopt);

#ifndef __getopt_defined
#define __getopt_defined 1
__LIBC int (__LIBCCALL getopt)(int ___argc, char *const *___argv, char const *__shortopts)
#if defined(__USE_POSIX2) && !defined(__USE_POSIX_IMPLICITLY) && !defined(__USE_GNU)
    __ASMNAME("__posix_getopt")
#endif
;
#endif /* !__getopt_defined */
#endif /* __USE_POSIX2 */

#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC int (__LIBCCALL setpgrp)(void);
__LIBC int (__LIBCCALL setreuid)(__uid_t __ruid, __uid_t __euid);
__LIBC long int (__LIBCCALL gethostid)(void);
__LIBC void (__LIBCCALL sync)(void);
__LIBC int (__LIBCCALL setregid)(__gid_t __rgid, __gid_t __egid);
#if defined(__USE_MISC) || !defined(__USE_XOPEN2K)
__LIBC __ATTR_CONST int (__LIBCCALL getpagesize)(void) ;
__LIBC int (__LIBCCALL getdtablesize)(void);
#endif /* __USE_MISC || !__USE_XOPEN2K */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __USE_XOPEN2K
__LIBC int (__LIBCCALL seteuid)(__uid_t __uid);
__LIBC int (__LIBCCALL setegid)(__gid_t __gid);
#endif /* __USE_XOPEN2K */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_UNIX98))
__LIBC int (__LIBCCALL ttyslot)(void);
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__LIBC __NONNULL((1,2)) int (__LIBCCALL symlink)(char const *__from, char const *__to) __UFS_FUNC(symlink);
__LIBC __NONNULL((1,2)) ssize_t (__LIBCCALL readlink)(char const *__restrict __path, char *__restrict __buf, size_t __len) __UFS_FUNC(readlink);
#endif

#if defined(__USE_REENTRANT) || defined(__USE_POSIX199506)
__LIBC __NONNULL((1)) int (__LIBCCALL getlogin_r)(char *__name, size_t __name_len);
#endif /* __USE_REENTRANT || __USE_POSIX199506 */

#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K)
__LIBC __NONNULL((1)) int (__LIBCCALL gethostname)(char *__name, size_t __len);
#endif /* __USE_UNIX98 || __USE_XOPEN2K */

#ifdef __USE_MISC
__LIBC __NONNULL((1)) int (__LIBCCALL setlogin)(char const *__name);
__LIBC __NONNULL((1)) int (__LIBCCALL sethostname)(char const *__name, size_t __len);
__LIBC int (__LIBCCALL sethostid)(long int __id);
__LIBC __NONNULL((1)) int (__LIBCCALL getdomainname)(char *__name, size_t __len);
__LIBC __NONNULL((1)) int (__LIBCCALL setdomainname)(char const *__name, size_t __len);
__LIBC int (__LIBCCALL vhangup)(void);
__LIBC __NONNULL((1)) int (__LIBCCALL revoke)(char const *__file) __UFS_FUNC(revoke);
__LIBC __NONNULL((1)) int (__LIBCCALL profil)(unsigned short int *__sample_buffer, size_t __size, size_t __offset, unsigned int __scale);
__LIBC int (__LIBCCALL acct)(char const *__name) __UFS_FUNC(acct);
__LIBC char *(__LIBCCALL getusershell)(void);
__LIBC void (__LIBCCALL endusershell)(void);
__LIBC void (__LIBCCALL setusershell)(void);
__LIBC int (__LIBCCALL daemon)(int __nochdir, int __noclose);
__LIBC long int (__ATTR_CDECL syscall)(long int __sysno, ...);
#ifdef __USE_KOS /* Execute a system call returning in both EAX and EDX */
__LIBC long long int (__ATTR_CDECL lsyscall)(long int __sysno, ...) __ASMNAME("syscall");
#endif /* __USE_KOS */
#endif /* __USE_MISC */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__LIBC __NONNULL((1)) int (__LIBCCALL chroot)(char const *__path) __UFS_FUNC(chroot);
__LIBC __NONNULL((1)) char *(__LIBCCALL getpass)(char const *__prompt);
#endif

#if defined(__USE_POSIX199309) || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
#ifdef __PE__
#ifdef __USE_FILE_OFFSET64
__LIBC int (__LIBCCALL ftruncate)(int __fd, __FS_TYPE(off) __length) __ASMNAME("_chsize_s");
#else /* __USE_FILE_OFFSET64 */
__LIBC int (__LIBCCALL ftruncate)(int __fd, __FS_TYPE(off) __length) __ASMNAME("_chsize");
#endif /* !__USE_FILE_OFFSET64 */
#else /* __PE__ */
__LIBC int (__LIBCCALL ftruncate)(int __fd, __FS_TYPE(off) __length) __FS_FUNC(ftruncate);
#endif /* !__PE__ */
#ifdef __USE_LARGEFILE64
__LIBC int (__LIBCCALL ftruncate64)(int __fd, __off64_t __length) __PE_ASMNAME("_chsize_s");
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_POSIX199309 || __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#if (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K)) || \
     defined(__USE_MISC)
__LIBC __WUNUSED int (__LIBCCALL brk)(void *__addr);
__LIBC __WUNUSED void *(__LIBCCALL sbrk)(intptr_t __delta);
#endif

#if defined(__USE_POSIX199309) || defined(__USE_UNIX98)
__LIBC int (__LIBCCALL fdatasync)(int __fildes) __PE_ASMNAME("_commit");
#endif /* __USE_POSIX199309 || __USE_UNIX98 */

#ifdef __USE_XOPEN
__LIBC __NONNULL((1,2)) char *(__LIBCCALL crypt)(char const *__key, char const *__salt);
__LIBC __NONNULL((1)) void (__LIBCCALL encrypt)(char *__glibc_block, int __edflag);
__LIBC __NONNULL((1,2)) void (__LIBCCALL swab)(void const *__restrict __from, void *__restrict __to, ssize_t __n_bytes);
#endif

#if defined(__USE_XOPEN) && !defined(__USE_XOPEN2K)
#ifndef __ctermid_defined
#define __ctermid_defined 1
__LIBC char *(__LIBCCALL ctermid)(char *__s);
#endif /* !__ctermid_defined */
#endif

__LIBC long int (__LIBCCALL sysconf)(int __name);
#endif /* !__KERNEL__ */


#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
#ifndef F_LOCK
#   define F_ULOCK 0 /*< Unlock a previously locked region. */
#   define F_LOCK  1 /*< Lock a region for exclusive use. */
#   define F_TLOCK 2 /*< Test and lock a region for exclusive use. */
#   define F_TEST  3 /*< Test a region for other processes locks. */
#ifndef __KERNEL__
__LIBC int (__LIBCCALL lockf)(int __fd, int __cmd, __FS_TYPE(off) __len) __FS_FUNC(lockf);
#ifdef __USE_LARGEFILE64
__LIBC int (__LIBCCALL lockf64)(int __fd, int __cmd, __off64_t __len);
#endif
#endif /* !__KERNEL__ */
#endif /* !F_LOCK */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __USE_GNU
#ifdef __COMPILER_HAVE_TYPEOF
#define TEMP_FAILURE_RETRY(expression) \
 __XBLOCK({ __typeof__(expression) __result; \
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

__DECL_END

#endif /* !_UNISTD_H */
