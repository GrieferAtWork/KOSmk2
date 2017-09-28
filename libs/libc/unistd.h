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
#ifndef GUARD_LIBS_LIBC_UNISTD_H
#define GUARD_LIBS_LIBC_UNISTD_H 1

#include "libc.h"
#include "system.h"
#include "errno.h"

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/kdev_t.h>
#include <sys/resource.h>
#include <uchar.h>
#include <stdbool.h>

DECL_BEGIN

struct utsname;
struct rlimit64;

INTDEF long int ATTR_CDECL libc_syscall(long int sysno, ...);
INTDEF major_t LIBCCALL libc_gnu_dev_major(dev_t dev);
INTDEF minor_t LIBCCALL libc_gnu_dev_minor(dev_t dev);
INTDEF dev_t LIBCCALL libc_gnu_dev_makedev(major_t major, minor_t minor);
INTDEF off32_t LIBCCALL libc_lseek(int fd, off32_t offset, int whence);
INTDEF ssize_t LIBCCALL libc_read(int fd, void *buf, size_t n_bytes);
INTDEF ssize_t LIBCCALL libc_write(int fd, void const *buf, size_t n_bytes);
INTDEF off64_t LIBCCALL libc_lseek64(int fd, off64_t offset, int whence);
INTDEF ssize_t LIBCCALL libc_pread(int fd, void *buf, size_t n_bytes, off32_t offset);
INTDEF ssize_t LIBCCALL libc_pwrite(int fd, void const *buf, size_t n_bytes, off32_t offset);
INTDEF ssize_t LIBCCALL libc_pread64(int fd, void *buf, size_t n_bytes, off64_t offset);
INTDEF ssize_t LIBCCALL libc_pwrite64(int fd, void const *buf, size_t n_bytes, off64_t offset);
#if __SIZEOF_POINTER__ == 8
#define libc_lseekI  libc_lseek64
#define libc_preadI  libc_pread64
#define libc_pwriteI libc_pwrite64
#else
#define libc_lseekI  libc_lseek
#define libc_preadI  libc_pread
#define libc_pwriteI libc_pwrite
#endif
INTDEF int LIBCCALL libc_truncate(char const *file, off32_t length);
INTDEF int LIBCCALL libc_truncate64(char const *file, off64_t length);
INTDEF int LIBCCALL libc_ftruncate(int fd, off32_t length);
INTDEF int LIBCCALL libc_ftruncate64(int fd, off64_t length);
INTDEF int LIBCCALL libc_fsync(int fd);
INTDEF int LIBCCALL libc_syncfs(int fd);
INTDEF void LIBCCALL libc_sync(void);
INTDEF int LIBCCALL libc_fdatasync(int fd);
INTDEF int LIBCCALL libc_close(int fd);
INTDEF int LIBCCALL libc_chroot(char const *path);
INTDEF int LIBCCALL libc_chdir(char const *path);
INTDEF int LIBCCALL libc_fchdir(int fd);
INTDEF int LIBCCALL libc_fchdirat(int dfd, char const *path, int flags);
INTDEF int LIBCCALL libc_dup(int fd);
INTDEF int LIBCCALL libc_dup3(int fd, int fd2, int flags);
INTDEF int LIBCCALL libc_dup2(int fd, int fd2);
INTDEF int LIBCCALL libc_fstat64(int fd, struct stat64 *buf);
INTDEF int LIBCCALL libc_fstatat64(int fd, char const *__restrict file, struct stat64 *__restrict buf, int flags);
INTDEF int LIBCCALL libc_stat64(char const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_lstat64(char const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_access(char const *name, int type);
INTDEF int LIBCCALL libc_eaccess(char const *name, int type);
INTDEF int LIBCCALL libc_faccessat(int fd, char const *file, int type, int flags);
INTDEF int LIBCCALL libc_chown(char const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_lchown(char const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_fchown(int fd, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_fchownat(int fd, char const *file, uid_t owner, gid_t group, int flags);
INTDEF int LIBCCALL libc_chmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_lchmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_fchmod(int fd, mode_t mode);
INTDEF int LIBCCALL libc_fchmodat(int fd, char const *file, mode_t mode, int flags);
INTDEF int LIBCCALL libc_mknodat(int fd, char const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_mkdirat(int fd, char const *path, mode_t mode);
INTDEF int LIBCCALL libc_mkfifoat(int fd, char const *path, mode_t mode);
INTDEF int LIBCCALL libc_linkat(int fromfd, char const *from, int tofd, char const *to, int flags);
INTDEF int LIBCCALL libc_symlinkat(char const *from, int tofd, char const *to);
INTDEF int LIBCCALL libc_unlinkat(int fd, char const *name, int flags);
INTDEF int LIBCCALL libc_renameat(int oldfd, char const *old, int newfd, char const *new_);
INTDEF int LIBCCALL libc_frenameat(int oldfd, char const *old, int newfd, char const *new_, int flags);
INTDEF ssize_t LIBCCALL libc_readlinkat(int fd, char const *__restrict path, char *__restrict buf, size_t len);
INTDEF int LIBCCALL libc_removeat(int fd, char const *filename);
INTDEF int LIBCCALL libc_remove(char const *filename);
INTDEF int LIBCCALL libc_rename(char const *old, char const *new_);
INTDEF pid_t LIBCCALL libc_fork(void);
INTDEF int LIBCCALL libc_execve(char const *path, char *const argv[], char *const envp[]);
INTDEF int LIBCCALL libc_fexecve(int fd, char *const argv[], char *const envp[]);
INTDEF int ATTR_CDECL libc_execl(char const *path, char const *arg, ...);
INTDEF int ATTR_CDECL libc_execle(char const *path, char const *arg, ...);
INTDEF int ATTR_CDECL libc_execlp(char const *file, char const *arg, ...);
INTDEF int ATTR_CDECL libc_execlpe(char const *file, char const *arg, ...);
INTDEF int ATTR_CDECL libc_fexecl(int fd, char const *arg, ...);
INTDEF int ATTR_CDECL libc_fexecle(int fd, char const *arg, ...);
INTDEF int LIBCCALL libc_fexecv(int fd, char *const argv[]);
INTDEF int LIBCCALL libc_execvpe(char const *file, char *const argv[], char *const envp[]);
INTDEF int LIBCCALL libc_execv(char const *path, char *const argv[]);
INTDEF int LIBCCALL libc_execvp(char const *file, char *const argv[]);
INTDEF int LIBCCALL libc_link(char const *from, char const *to);
INTDEF int LIBCCALL libc_symlink(char const *from, char const *to);
INTDEF ssize_t LIBCCALL libc_readlink(char const *__restrict path, char *__restrict buf, size_t len);
INTDEF int LIBCCALL libc_unlink(char const *name);
INTDEF int LIBCCALL libc_rmdir(char const *path);
INTDEF int LIBCCALL libc_mkdir(char const *path, mode_t mode);
INTDEF int LIBCCALL libc_mkfifo(char const *path, mode_t mode);
INTDEF int LIBCCALL libc_mknod(char const *path, mode_t mode, dev_t dev);
INTDEF pid_t LIBCCALL libc_wait(int *stat_loc);
INTDEF pid_t LIBCCALL libc_waitpid(pid_t pid, int *stat_loc, int options);
INTDEF pid_t LIBCCALL libc_wait3(int *stat_loc, int options, struct rusage *usage);
INTDEF int LIBCCALL libc_waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
INTDEF pid_t LIBCCALL libc_wait4(pid_t pid, int *stat_loc, int options, struct rusage *usage);
INTDEF int LIBCCALL libc_getgroups(int size, gid_t list[]);
INTDEF int LIBCCALL libc_setuid(uid_t uid);
INTDEF int LIBCCALL libc_setgid(gid_t gid);
INTDEF int LIBCCALL libc_umount(char const *special_file);
INTDEF int LIBCCALL libc_mount(char const *special_file, char const *dir, char const *fstype, unsigned long int rwflag, void const *data);
INTDEF int LIBCCALL libc_umount2(char const *special_file, int flags);
INTDEF int LIBCCALL libc_swapon(char const *path, int flags);
INTDEF int LIBCCALL libc_swapoff(char const *path);
INTDEF int LIBCCALL libc_pipe(int pipedes[2]);
INTDEF int LIBCCALL libc_pipe2(int pipedes[2], int flags);
INTDEF mode_t LIBCCALL libc_umask(mode_t mask);
INTDEF mode_t LIBCCALL libc_getumask(void);
INTDEF pid_t LIBCCALL libc_getpid(void);
INTDEF pid_t LIBCCALL libc_gettid(void);
INTDEF pid_t LIBCCALL libc_getppid(void);
INTDEF pid_t LIBCCALL libc_getpgid(pid_t pid);
INTDEF pid_t LIBCCALL libc_getpgrp(void);
INTDEF int LIBCCALL libc_setpgid(pid_t pid, pid_t pgid);
INTDEF pid_t LIBCCALL libc_getsid(pid_t pid);
INTDEF pid_t LIBCCALL libc_setsid(void);
INTDEF uid_t LIBCCALL libc_getuid(void);
INTDEF gid_t LIBCCALL libc_getgid(void);
INTDEF uid_t LIBCCALL libc_geteuid(void);
INTDEF gid_t LIBCCALL libc_getegid(void);
INTDEF long int LIBCCALL libc_pathconf(char const *path, int name);
INTDEF long int LIBCCALL libc_fpathconf(int fd, int name);
INTDEF char *LIBCCALL libc_getlogin(void);
INTDEF int LIBCCALL libc_group_member(gid_t gid);
INTDEF int LIBCCALL libc_getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
INTDEF int LIBCCALL libc_getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid);
INTDEF int LIBCCALL libc_setresuid(uid_t ruid, uid_t euid, uid_t suid);
INTDEF int LIBCCALL libc_setresgid(gid_t rgid, gid_t egid, gid_t sgid);
INTDEF int LIBCCALL libc_setreuid(uid_t ruid, uid_t euid);
INTDEF int LIBCCALL libc_setregid(gid_t rgid, gid_t egid);
INTDEF pid_t LIBCCALL libc_vfork(void);
INTDEF int LIBCCALL libc_nice(int inc);
INTDEF size_t LIBCCALL libc_confstr(int name, char *buf, size_t len);
INTDEF int LIBCCALL libc_setpgrp(void);
INTDEF long int LIBCCALL libc_gethostid(void);
INTDEF int LIBCCALL libc_getpagesize(void);
INTDEF int LIBCCALL libc_getdtablesize(void);
INTDEF int LIBCCALL libc_seteuid(uid_t uid);
INTDEF int LIBCCALL libc_setegid(gid_t gid);
INTDEF int LIBCCALL libc_ttyslot(void);
INTDEF int LIBCCALL libc_getlogin_r(char *name, size_t name_len);
INTDEF int LIBCCALL libc_gethostname(char *name, size_t len);
INTDEF int LIBCCALL libc_setlogin(char const *name);
INTDEF int LIBCCALL libc_sethostname(char const *name, size_t len);
INTDEF int LIBCCALL libc_sethostid(long int id);
INTDEF int LIBCCALL libc_getdomainname(char *name, size_t len);
INTDEF int LIBCCALL libc_setdomainname(char const *name, size_t len);
INTDEF int LIBCCALL libc_vhangup(void);
INTDEF int LIBCCALL libc_revoke(char const *file);
INTDEF int LIBCCALL libc_profil(unsigned short int *sample_buffer, size_t size, size_t offset, unsigned int scale);
INTDEF int LIBCCALL libc_acct(char const *name);
INTDEF char *LIBCCALL libc_getusershell(void);
INTDEF void LIBCCALL libc_endusershell(void);
INTDEF void LIBCCALL libc_setusershell(void);
INTDEF int LIBCCALL libc_daemon(int nochdir, int noclose);
INTDEF char *LIBCCALL libc_getpass(char const *prompt);
INTDEF char *LIBCCALL libc_crypt(char const *key, char const *salt);
INTDEF void LIBCCALL libc_encrypt(char *glibc_block, int edflag);
INTDEF void LIBCCALL libc_swab(void const *__restrict from, void *__restrict to, ssize_t n_bytes);
INTDEF char *LIBCCALL libc_ctermid(char *s);
INTDEF char *LIBCCALL libc_cuserid(char *s);
INTDEF int LIBCCALL libc_lockf(int fd, int cmd, off32_t len);
INTDEF int LIBCCALL libc_lockf64(int fd, int cmd, off64_t len);
INTDEF struct passwd *LIBCCALL libc_getpwuid(uid_t uid);
INTDEF struct passwd *LIBCCALL libc_getpwnam(char const *name);
INTDEF void LIBCCALL libc_setpwent(void);
INTDEF void LIBCCALL libc_endpwent(void);
INTDEF struct passwd *LIBCCALL libc_getpwent(void);
INTDEF struct passwd *LIBCCALL libc_fgetpwent(__FILE *stream);
INTDEF int LIBCCALL libc_putpwent(struct passwd const *__restrict p, __FILE *__restrict f);
INTDEF int LIBCCALL libc_getpwuid_r(uid_t uid, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result);
INTDEF int LIBCCALL libc_getpwnam_r(char const *__restrict name, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result);
INTDEF int LIBCCALL libc_getpwent_r(struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result);
INTDEF int LIBCCALL libc_fgetpwent_r(__FILE *__restrict stream, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result);
INTDEF int LIBCCALL libc_getpw(uid_t uid, char *buffer);
INTDEF struct group *LIBCCALL libc_getgrgid(gid_t gid);
INTDEF struct group *LIBCCALL libc_getgrnam(char const *name);
INTDEF void LIBCCALL libc_setgrent(void);
INTDEF void LIBCCALL libc_endgrent(void);
INTDEF struct group *LIBCCALL libc_getgrent(void);
INTDEF int LIBCCALL libc_putgrent(struct group const *__restrict p, __FILE *__restrict f);
INTDEF int LIBCCALL libc_getgrgid_r(gid_t gid, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result);
INTDEF int LIBCCALL libc_getgrnam_r(char const *__restrict name, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result);
INTDEF int LIBCCALL libc_getgrent_r(struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result);
INTDEF int LIBCCALL libc_fgetgrent_r(__FILE *__restrict stream, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result);
INTDEF struct group *LIBCCALL libc_fgetgrent(__FILE *stream);
INTDEF int LIBCCALL libc_setgroups(size_t __n, const gid_t *groups);
INTDEF int LIBCCALL libc_getgrouplist(char const *user, gid_t group, gid_t *groups, int *ngroups);
INTDEF int LIBCCALL libc_initgroups(char const *user, gid_t group);
INTDEF __FILE *LIBCCALL libc_setmntent(char const *file, char const *mode);
INTDEF struct mntent *LIBCCALL libc_getmntent(__FILE *stream);
INTDEF struct mntent *LIBCCALL libc_getmntent_r(__FILE *__restrict stream, struct mntent *__restrict result, char *__restrict buffer, int bufsize);
INTDEF int LIBCCALL libc_addmntent(__FILE *__restrict stream, struct mntent const *__restrict mnt);
INTDEF int LIBCCALL libc_endmntent(__FILE *stream);
INTDEF char *LIBCCALL libc_hasmntopt(struct mntent const *mnt, char const *opt);
INTDEF int LIBCCALL libc_fnmatch(char const *pattern, char const *name, int flags);
INTDEF int LIBCCALL libc_getrlimit(__rlimit_resource_t resource, struct rlimit *rlimits);
INTDEF int LIBCCALL libc_setrlimit(__rlimit_resource_t resource, struct rlimit const *rlimits);
INTDEF int LIBCCALL libc_getrlimit64(__rlimit_resource_t resource, struct rlimit64 *rlimits);
INTDEF int LIBCCALL libc_setrlimit64(__rlimit_resource_t resource, struct rlimit64 const *rlimits);
INTDEF int LIBCCALL libc_getpriority(__priority_which_t which, id_t who);
INTDEF int LIBCCALL libc_setpriority(__priority_which_t which, id_t who, int prio);
INTDEF int LIBCCALL libc_prlimit(pid_t pid, enum __rlimit_resource resource, struct rlimit const *new_limit, struct rlimit *old_limit);
INTDEF int LIBCCALL libc_prlimit64(pid_t pid, enum __rlimit_resource resource, struct rlimit64 const *new_limit, struct rlimit64 *old_limit);
INTDEF int LIBCCALL libc_getrusage(__rusage_who_t who, struct rusage *usage);

/* Binary compatibility with glibc's stat() buffer. */
typedef u64             glibc_dev_t;
typedef syscall_ulong_t glibc_ino_t;
typedef u64             glibc_ino64_t;
typedef u32             glibc_mode_t;
typedef syscall_ulong_t glibc_nlink_t;
typedef u32             glibc_uid_t;
typedef u32             glibc_gid_t;
typedef s64             glibc_off64_t;
typedef u64             glibc_blkcnt64_t;
typedef syscall_slong_t glibc_blksize_t;
struct glibc_stat {
    glibc_dev_t   st_dev;
#ifdef __x86_64__
    glibc_ino_t   st_ino;
    glibc_nlink_t st_nlink;
    glibc_mode_t  st_mode;
#else
    unsigned short int __pad1;
    glibc_ino_t   st_ino32;
    glibc_mode_t  st_mode;
    glibc_nlink_t st_nlink;
#endif
    glibc_uid_t   st_uid;
    glibc_gid_t   st_gid;
#ifdef __x86_64__
    int         __pad0;
#endif
    glibc_dev_t   st_rdev;
#ifndef __x86_64__
    unsigned short int __pad2;
#endif
    glibc_off64_t st_size;
    glibc_blksize_t st_blksize;
    glibc_blkcnt64_t st_blocks;
    struct __timespec32 st_atim;
    struct __timespec32 st_mtim;
    struct __timespec32 st_ctim;
#ifdef __x86_64__
    __syscall_slong_t __glibc_reserved[3];
#else
    glibc_ino64_t st_ino64;
#endif
};
INTDEF int LIBCCALL libc_glibc_stat(char const *path, struct glibc_stat *statbuf);
INTDEF int LIBCCALL libc_glibc_lstat(char const *path, struct glibc_stat *statbuf);
INTDEF int LIBCCALL libc_glibc_fstat(int fd, struct glibc_stat *statbuf);
INTDEF int LIBCCALL libc_glibc_fstatat(int fd, char const *filename, struct glibc_stat *statbuf, int flags);
INTDEF int LIBCCALL libc___fxstat(int ver, int fd, struct glibc_stat *statbuf);
INTDEF int LIBCCALL libc___xstat(int ver, char const *filename, struct glibc_stat *statbuf);
INTDEF int LIBCCALL libc___lxstat(int ver, char const *filename, struct glibc_stat *statbuf);
INTDEF int LIBCCALL libc___fxstatat(int ver, int fd, char const *filename, struct glibc_stat *statbuf, int flags);
INTDEF int LIBCCALL libc_uname(struct utsname *name);
INTDEF int LIBCCALL libc_brk(void *addr);
INTDEF void *LIBCCALL libc_sbrk(intptr_t increment);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
struct _diskfree_t;
INTDEF int LIBCCALL libc_chdrive(int drive);
INTDEF int LIBCCALL libc_getdrive(void);
INTDEF unsigned long LIBCCALL libc_getdrives(void);
INTDEF unsigned LIBCCALL libc_getdiskfree(unsigned drive, struct _diskfree_t *diskfree);

INTDEF int LIBCCALL libc_dos_truncate(char const *file, off32_t length);
INTDEF int LIBCCALL libc_dos_truncate64(char const *file, off64_t length);
INTDEF int LIBCCALL libc_dos_chroot(char const *path);
INTDEF int LIBCCALL libc_dos_chdir(char const *path);
INTDEF int LIBCCALL libc_dos_fchdirat(int dfd, char const *path, int flags);
INTDEF int LIBCCALL libc_dos_fstatat64(int fd, char const *__restrict file, struct stat64 *__restrict buf, int flags);
INTDEF int LIBCCALL libc_dos_stat64(char const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_lstat64(char const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_access(char const *name, int type);
INTDEF int LIBCCALL libc_dos_eaccess(char const *name, int type);
INTDEF int LIBCCALL libc_dos_faccessat(int fd, char const *file, int type, int flags);
INTDEF int LIBCCALL libc_dos_chown(char const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_dos_lchown(char const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_dos_fchownat(int fd, char const *file, uid_t owner, gid_t group, int flags);
INTDEF int LIBCCALL libc_dos_chmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_lchmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_fchmodat(int fd, char const *file, mode_t mode, int flags);
INTDEF int LIBCCALL libc_dos_mknodat(int fd, char const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_dos_mkdirat(int fd, char const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_mkfifoat(int fd, char const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_linkat(int fromfd, char const *from, int tofd, char const *to, int flags);
INTDEF int LIBCCALL libc_dos_symlinkat(char const *from, int tofd, char const *to);
INTDEF int LIBCCALL libc_dos_unlinkat(int fd, char const *name, int flags);
INTDEF int LIBCCALL libc_dos_renameat(int oldfd, char const *old, int newfd, char const *new_);
INTDEF int LIBCCALL libc_dos_removeat(int fd, char const *filename);
INTDEF int LIBCCALL libc_dos_remove(char const *filename);
INTDEF int LIBCCALL libc_dos_rename(char const *old, char const *new_);
INTDEF int LIBCCALL libc_dos_execve(char const *path, char *const argv[], char *const envp[]);
INTDEF int ATTR_CDECL libc_dos_execl(char const *path, char const *arg, ...);
INTDEF int ATTR_CDECL libc_dos_execle(char const *path, char const *arg, ...);
INTDEF int ATTR_CDECL libc_dos_execlp(char const *file, char const *arg, ...);
INTDEF int ATTR_CDECL libc_dos_execlpe(char const *file, char const *arg, ...);
INTDEF int LIBCCALL libc_dos_execvpe(char const *file, char *const argv[], char *const envp[]);
INTDEF int LIBCCALL libc_dos_execv(char const *path, char *const argv[]);
INTDEF int LIBCCALL libc_dos_execvp(char const *file, char *const argv[]);
INTDEF int LIBCCALL libc_dos_link(char const *from, char const *to);
INTDEF int LIBCCALL libc_dos_symlink(char const *from, char const *to);
INTDEF int LIBCCALL libc_dos_unlink(char const *name);
INTDEF int LIBCCALL libc_dos_rmdir(char const *path);
INTDEF int LIBCCALL libc_dos_mkdir(char const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_mkfifo(char const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_mknod(char const *path, mode_t mode, dev_t dev);
INTDEF long int LIBCCALL libc_dos_pathconf(char const *path, int name);
INTDEF int LIBCCALL libc_dos_revoke(char const *file);
INTDEF int LIBCCALL libc_dos_acct(char const *name);
INTDEF __FILE *LIBCCALL libc_dos_setmntent(char const *file, char const *mode);
INTDEF int LIBCCALL libc_execvpe_impl(char const *file, char *const argv[], char *const envp[], bool dosmode);

/* Functions that are not supported in dos-fs mode. */
// INTDEF int LIBCCALL libc_dos_umount(char const *special_file);
// INTDEF int LIBCCALL libc_dos_mount(char const *special_file, char const *dir, char const *fstype, unsigned long int rwflag, void const *data);
// INTDEF int LIBCCALL libc_dos_umount2(char const *special_file, int flags);
// INTDEF int LIBCCALL libc_dos_swapon(char const *path, int flags);
// INTDEF int LIBCCALL libc_dos_swapoff(char const *path);


INTDEF int LIBCCALL libc_16wchdir(char16_t const *path);
INTDEF int LIBCCALL libc_16wrmdir(char16_t const *path);
INTDEF int LIBCCALL libc_16wmkdir(char16_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_32wchdir(char32_t const *path);
INTDEF int LIBCCALL libc_32wrmdir(char32_t const *path);
INTDEF int LIBCCALL libc_32wmkdir(char32_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_mkdir(char const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_mkdir2(char const *path);
INTDEF int LIBCCALL libc_dos_16wmkdir(char16_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_32wmkdir(char32_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_16wmkdir2(char16_t const *path);
INTDEF int LIBCCALL libc_dos_32wmkdir2(char32_t const *path);

INTDEF int LIBCCALL libc_16wfaccessat(int dfd, char16_t const *file, int type, int flags);
INTDEF int LIBCCALL libc_32wfaccessat(int dfd, char32_t const *file, int type, int flags);
INTDEF int LIBCCALL libc_16wfchmodat(int dfd, char16_t const *file, mode_t mode, int flags);
INTDEF int LIBCCALL libc_32wfchmodat(int dfd, char32_t const *file, mode_t mode, int flags);
INTDEF int LIBCCALL libc_16wunlinkat(int dfd, char16_t const *file, int flags);
INTDEF int LIBCCALL libc_32wunlinkat(int dfd, char32_t const *file, int flags);
INTDEF int LIBCCALL libc_16wfrenameat(int oldfd, char16_t const *oldname, int newfd, char16_t const *newname, int flags);
INTDEF int LIBCCALL libc_32wfrenameat(int oldfd, char32_t const *oldname, int newfd, char32_t const *newname, int flags);

INTDEF int LIBCCALL libc_16waccess(char16_t const *file, int type);
INTDEF int LIBCCALL libc_32waccess(char32_t const *file, int type);
INTDEF int LIBCCALL libc_dos_16waccess(char16_t const *file, int type);
INTDEF int LIBCCALL libc_dos_32waccess(char32_t const *file, int type);
INTDEF __errno_t LIBCCALL libc_16waccess_s(char16_t const *file, int type);
INTDEF __errno_t LIBCCALL libc_32waccess_s(char32_t const *file, int type);
INTDEF __errno_t LIBCCALL libc_dos_16waccess_s(char16_t const *file, int type);
INTDEF __errno_t LIBCCALL libc_dos_32waccess_s(char32_t const *file, int type);
INTDEF int LIBCCALL libc_16wchmod(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_32wchmod(char32_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_16wunlink(char16_t const *file);
INTDEF int LIBCCALL libc_32wunlink(char32_t const *file);
INTDEF int LIBCCALL libc_16wrename(char16_t const *oldname, char16_t const *newname);
INTDEF int LIBCCALL libc_32wrename(char32_t const *oldname, char32_t const *newname);
INTDEF int LIBCCALL libc_dos_16wchmod(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_32wchmod(char32_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_16wunlink(char16_t const *file);
INTDEF int LIBCCALL libc_dos_32wunlink(char32_t const *file);
INTDEF int LIBCCALL libc_dos_16wrename(char16_t const *oldname, char16_t const *newname);
INTDEF int LIBCCALL libc_dos_32wrename(char32_t const *oldname, char32_t const *newname);

INTDEF __errno_t LIBCCALL libc_access_s(char const *file, int type);
INTDEF __errno_t LIBCCALL libc_dos_access_s(char const *file, int type);
INTDEF int LIBCCALL libc_dos_rename(char const *old, char const *new_);
INTDEF int LIBCCALL libc_dos_chdir(char const *path);
INTDEF int LIBCCALL libc_dos_rmdir(char const *path);
INTDEF int LIBCCALL libc_dos_unlink(char const *name);
INTDEF int LIBCCALL libc_dos_remove(char const *name);
INTDEF int LIBCCALL libc_dos_16wchdir(char16_t const *path);
INTDEF int LIBCCALL libc_dos_16wrmdir(char16_t const *path);
INTDEF int LIBCCALL libc_dos_32wchdir(char32_t const *path);
INTDEF int LIBCCALL libc_dos_32wrmdir(char32_t const *path);
INTDEF int LIBCCALL libc_dos_chmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_eof(int fd);
INTDEF off_t LIBCCALL libc_fdsize(int fd);
INTDEF off_t LIBCCALL libc_fdtell(int fd);
INTDEF off64_t LIBCCALL libc_fdsize64(int fd);
INTDEF off64_t LIBCCALL libc_fdtell64(int fd);
INTDEF int LIBCCALL libc_dos_pipe(int pipedes[2], u32 pipesize, int textmode);
INTDEF mode_t LIBCCALL libc_setmode(int fd, mode_t mode);
INTDEF __errno_t LIBCCALL libc_umask_s(mode_t new_mode, mode_t *old_mode);
INTDEF int  LIBCCALL libc__lock_fhandle(int fd);
INTDEF void LIBCCALL libc_unlock_fhandle(int fd);
INTDEF intptr_t LIBCCALL libc_get_osfhandle(int fd);
INTDEF int LIBCCALL libc_open_osfhandle(intptr_t osfd, int flags);

#define WEXEC_F_NORMAL  0x0
#define WEXEC_F_DOSMODE 0x1
#define WEXEC_F_PATH    0x2

INTDEF int LIBCCALL libc_impl_16wexecve(int mode, char16_t const *path, char16_t const *const *argv, char16_t const *const *envp);
INTDEF int LIBCCALL libc_impl_32wexecve(int mode, char32_t const *path, char32_t const *const *argv, char32_t const *const *envp);
INTDEF int LIBCCALL libc_16wexecv(char16_t const *path, char16_t const *const *argv);
INTDEF int LIBCCALL libc_16wexecve(char16_t const *path, char16_t const *const *argv, char16_t const *const *envp);
INTDEF int LIBCCALL libc_16wexecvp(char16_t const *file, char16_t const *const *argv);
INTDEF int LIBCCALL libc_16wexecvpe(char16_t const *file, char16_t const *const *argv, char16_t const *const *envp);
INTDEF int LIBCCALL libc_32wexecv(char32_t const *path, char32_t const *const *argv);
INTDEF int LIBCCALL libc_32wexecve(char32_t const *path, char32_t const *const *argv, char32_t const *const *envp);
INTDEF int LIBCCALL libc_32wexecvp(char32_t const *file, char32_t const *const *argv);
INTDEF int LIBCCALL libc_32wexecvpe(char32_t const *file, char32_t const *const *argv, char32_t const *const *envp);
INTDEF int LIBCCALL libc_dos_16wexecv(char16_t const *path, char16_t const *const *argv);
INTDEF int LIBCCALL libc_dos_16wexecve(char16_t const *path, char16_t const *const *argv, char16_t const *const *envp);
INTDEF int LIBCCALL libc_dos_16wexecvp(char16_t const *file, char16_t const *const *argv);
INTDEF int LIBCCALL libc_dos_16wexecvpe(char16_t const *file, char16_t const *const *argv, char16_t const *const *envp);
INTDEF int LIBCCALL libc_dos_32wexecv(char32_t const *path, char32_t const *const *argv);
INTDEF int LIBCCALL libc_dos_32wexecve(char32_t const *path, char32_t const *const *argv, char32_t const *const *envp);
INTDEF int LIBCCALL libc_dos_32wexecvp(char32_t const *file, char32_t const *const *argv);
INTDEF int LIBCCALL libc_dos_32wexecvpe(char32_t const *file, char32_t const *const *argv, char32_t const *const *envp);
INTDEF int ATTR_CDECL libc_16wexecl(char16_t const *path, char16_t const *argv, ...);
INTDEF int ATTR_CDECL libc_16wexecle(char16_t const *path, char16_t const *argv, ...);
INTDEF int ATTR_CDECL libc_16wexeclp(char16_t const *file, char16_t const *argv, ...);
INTDEF int ATTR_CDECL libc_16wexeclpe(char16_t const *file, char16_t const *argv, ...);
INTDEF int ATTR_CDECL libc_32wexecl(char32_t const *path, char32_t const *argv, ...);
INTDEF int ATTR_CDECL libc_32wexecle(char32_t const *path, char32_t const *argv, ...);
INTDEF int ATTR_CDECL libc_32wexeclp(char32_t const *file, char32_t const *argv, ...);
INTDEF int ATTR_CDECL libc_32wexeclpe(char32_t const *file, char32_t const *argv, ...);
INTDEF int ATTR_CDECL libc_dos_16wexecl(char16_t const *path, char16_t const *argv, ...);
INTDEF int ATTR_CDECL libc_dos_16wexecle(char16_t const *path, char16_t const *argv, ...);
INTDEF int ATTR_CDECL libc_dos_16wexeclp(char16_t const *file, char16_t const *argv, ...);
INTDEF int ATTR_CDECL libc_dos_16wexeclpe(char16_t const *file, char16_t const *argv, ...);
INTDEF int ATTR_CDECL libc_dos_32wexecl(char32_t const *path, char32_t const *argv, ...);
INTDEF int ATTR_CDECL libc_dos_32wexecle(char32_t const *path, char32_t const *argv, ...);
INTDEF int ATTR_CDECL libc_dos_32wexeclp(char32_t const *file, char32_t const *argv, ...);
INTDEF int ATTR_CDECL libc_dos_32wexeclpe(char32_t const *file, char32_t const *argv, ...);


/* Unicode lstat() and fstatat() are currently not used in header files.
 * >> Don't export them until they actually appear somewhere. */
#undef CONFIG_LIBC_HAVE_UNICODE_LSTAT
#undef CONFIG_LIBC_HAVE_UNICODE_FSTATAT
//#define CONFIG_LIBC_HAVE_UNICODE_LSTAT
//#define CONFIG_LIBC_HAVE_UNICODE_FSTATAT

/* KOS stat functions for different character encodings. */
INTDEF int LIBCCALL libc_w16stat64(char16_t const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_w32stat64(char32_t const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_w16stat64(char16_t const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_w32stat64(char32_t const *__restrict file, struct stat64 *__restrict buf);
#ifdef CONFIG_LIBC_HAVE_UNICODE_FSTATAT
INTDEF int LIBCCALL libc_w16fstatat64(int fd, char16_t const *__restrict file, struct stat64 *__restrict buf, int flags);
INTDEF int LIBCCALL libc_w32fstatat64(int fd, char32_t const *__restrict file, struct stat64 *__restrict buf, int flags);
INTDEF int LIBCCALL libc_dos_w16fstatat64(int fd, char16_t const *__restrict file, struct stat64 *__restrict buf, int flags);
INTDEF int LIBCCALL libc_dos_w32fstatat64(int fd, char32_t const *__restrict file, struct stat64 *__restrict buf, int flags);
#endif
#ifdef CONFIG_LIBC_HAVE_UNICODE_LSTAT
INTDEF int LIBCCALL libc_w16lstat64(char16_t const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_w32lstat64(char32_t const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_w16lstat64(char16_t const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_w32lstat64(char32_t const *__restrict file, struct stat64 *__restrict buf);
#endif /* CONFIG_LIBC_HAVE_UNICODE_LSTAT */

struct __dos_stat32;
struct __dos_stat32i64;
struct __dos_stat64i32;
struct __dos_stat64;

/* DOS-FS mode + DOS-binary-compatible stat functions.
 * NOTE: These are only used when user-apps are configures as '__PE__' + '__USE_DOS'.
 *       Otherwise, KOS's stat buffer data layout is used instead.  */
INTDEF void LIBCCALL libc_fill_dos_stat32(struct stat64 const *__restrict src, struct __dos_stat32 *__restrict buf);
INTDEF void LIBCCALL libc_fill_dos_stat64(struct stat64 const *__restrict src, struct __dos_stat64 *__restrict buf);
INTDEF void LIBCCALL libc_fill_dos_stat32i64(struct stat64 const *__restrict src, struct __dos_stat32i64 *__restrict buf);
INTDEF void LIBCCALL libc_fill_dos_stat64i32(struct stat64 const *__restrict src, struct __dos_stat64i32 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_stat32(char const *__restrict file, struct __dos_stat32 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_stat64(char const *__restrict file, struct __dos_stat64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_stat32i64(char const *__restrict file, struct __dos_stat32i64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_stat64i32(char const *__restrict file, struct __dos_stat64i32 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_fstat32(int fd, struct __dos_stat32 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_fstat64(int fd, struct __dos_stat64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_fstat32i64(int fd, struct __dos_stat32i64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_fstat64i32(int fd, struct __dos_stat64i32 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_16wstat32(char16_t const *__restrict file, struct __dos_stat32 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_16wstat64(char16_t const *__restrict file, struct __dos_stat64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_16wstat32i64(char16_t const *__restrict file, struct __dos_stat32i64 *__restrict buf);
INTDEF int LIBCCALL libc_dos_local_16wstat64i32(char16_t const *__restrict file, struct __dos_stat64i32 *__restrict buf);
/* NOTE: There are no 'libc_dos_32wstat32' functions, because PE only has 16-bit wide
 *       characters, and the functions above are only ever used when building in PE-mode. */


/* TODO: Spawn functions. */
// __LIBC intptr_t (ATTR_CDECL _wspawnl)(int __mode, wchar_t const *__path, wchar_t const *__argv, ...) __UFS_FUNC(_wspawnl);
// __LIBC intptr_t (ATTR_CDECL _wspawnle)(int __mode, wchar_t const *__path, wchar_t const *__argv, ...) __UFS_FUNC(_wspawnle);
// __LIBC intptr_t (ATTR_CDECL _wspawnlp)(int __mode, wchar_t const *__file, wchar_t const *__argv, ...) __UFS_FUNC(_wspawnlp);
// __LIBC intptr_t (ATTR_CDECL _wspawnlpe)(int __mode, wchar_t const *__file, wchar_t const *__argv, ...) __UFS_FUNC(_wspawnlpe);
// __LIBC intptr_t (__LIBCCALL _wspawnv)(int __mode, wchar_t const *__path, wchar_t const *const *__argv) __UFS_FUNC(_wspawnv);
// __LIBC intptr_t (__LIBCCALL _wspawnve)(int __mode, wchar_t const *__path, wchar_t const *const *__argv, wchar_t const *const *__envp) __UFS_FUNC(_wspawnve);
// __LIBC intptr_t (__LIBCCALL _wspawnvp)(int __mode, wchar_t const *__file, wchar_t const *const *__argv) __UFS_FUNC(_wspawnvp);
// __LIBC intptr_t (__LIBCCALL _wspawnvpe)(int __mode, wchar_t const *__file, wchar_t const *const *__argv, wchar_t const *const *__envp) __UFS_FUNC(_wspawnvpe);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_UNISTD_H */
