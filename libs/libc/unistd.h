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

DECL_BEGIN

struct tms;
struct utsname;

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
INTDEF int LIBCCALL libc_dup(int fd);
INTDEF int LIBCCALL libc_dup3(int fd, int fd2, int flags);
INTDEF int LIBCCALL libc_dup2(int fd, int fd2);
INTDEF int LIBCCALL libc_kfstat64(int fd, struct stat64 *buf);
INTDEF int LIBCCALL libc_kfstatat64(int fd, char const *__restrict file, struct stat64 *__restrict buf, int flag);
INTDEF int LIBCCALL libc_kstat64(char const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_klstat64(char const *__restrict file, struct stat64 *__restrict buf);
INTDEF int LIBCCALL libc_access(char const *name, int type);
INTDEF int LIBCCALL libc_faccessat(int fd, char const *file, int type, int flag);
INTDEF int LIBCCALL libc_chown(char const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_lchown(char const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_fchown(int fd, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_chmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_lchmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_fchmod(int fd, mode_t mode);
INTDEF int LIBCCALL libc_fchownat(int fd, char const *file, uid_t owner, gid_t group, int flag);
INTDEF int LIBCCALL libc_fchmodat(int fd, char const *file, mode_t mode, int flag);
INTDEF int LIBCCALL libc_mknodat(int fd, char const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_mkdirat(int fd, char const *path, mode_t mode);
INTDEF int LIBCCALL libc_mkfifoat(int fd, char const *path, mode_t mode);
INTDEF int LIBCCALL libc_linkat(int fromfd, char const *from, int tofd, char const *to, int flags);
INTDEF int LIBCCALL libc_symlinkat(char const *from, int tofd, char const *to);
INTDEF int LIBCCALL libc_unlinkat(int fd, char const *name, int flag);
INTDEF int LIBCCALL libc_renameat(int oldfd, char const *old, int newfd, char const *new_);
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
INTDEF int LIBCCALL libc_eaccess(char const *name, int type);
INTDEF int LIBCCALL libc_mkdir(char const *path, mode_t mode);
INTDEF int LIBCCALL libc_mkfifo(char const *path, mode_t mode);
INTDEF int LIBCCALL libc_mknod(char const *path, mode_t mode, dev_t dev);
INTDEF pid_t LIBCCALL libc_wait(__WAIT_STATUS stat_loc);
INTDEF pid_t LIBCCALL libc_waitpid(pid_t pid, __WAIT_STATUS stat_loc, int options);
INTDEF pid_t LIBCCALL libc_wait3(__WAIT_STATUS stat_loc, int options, struct rusage *usage);
INTDEF int LIBCCALL libc_waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
INTDEF pid_t LIBCCALL libc_wait4(pid_t pid, __WAIT_STATUS stat_loc, int options, struct rusage *usage);
INTDEF int LIBCCALL libc_getgroups(int size, gid_t list[]);
INTDEF int LIBCCALL libc_setuid(uid_t uid);
INTDEF int LIBCCALL libc_setgid(gid_t gid);
INTDEF int LIBCCALL libc_umount(char const *special_file);
INTDEF int LIBCCALL libc_mount(char const *special_file, char const *dir, char const *fstype, unsigned long int rwflag, void const *data);
INTDEF int LIBCCALL libc_umount2(char const *special_file, int flags);
INTDEF int LIBCCALL libc_swapon(const char *path, int flags);
INTDEF int LIBCCALL libc_swapoff(const char *path);
INTDEF int LIBCCALL libc_pipe(int pipedes[2]);
INTDEF int LIBCCALL libc_pipe2(int pipedes[2], int flags);
INTDEF mode_t LIBCCALL libc_umask(mode_t mask);
INTDEF mode_t LIBCCALL libc_getumask(void);
INTDEF pid_t LIBCCALL libc_getpid(void);
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
INTDEF clock_t LIBCCALL libc_times(struct tms *buffer);
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
INTDEF int LIBCCALL libc_glibc_fstatat(int fd, char const *filename, struct glibc_stat *statbuf, int flag);
INTDEF int LIBCCALL libc___fxstat(int ver, int fd, struct glibc_stat *statbuf);
INTDEF int LIBCCALL libc___xstat(int ver, char const *filename, struct glibc_stat *statbuf);
INTDEF int LIBCCALL libc___lxstat(int ver, char const *filename, struct glibc_stat *statbuf);
INTDEF int LIBCCALL libc___fxstatat(int ver, int fd, char const *filename, struct glibc_stat *statbuf, int flag);
INTDEF int LIBCCALL libc_uname(struct utsname *name);
INTDEF int LIBCCALL libc_brk(void *addr);
INTDEF void *LIBCCALL libc_sbrk(intptr_t increment);

DECL_END

#endif /* !GUARD_LIBS_LIBC_UNISTD_H */