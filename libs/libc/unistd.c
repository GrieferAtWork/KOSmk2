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
#ifndef GUARD_LIBS_LIBC_UNISTD_C
#define GUARD_LIBS_LIBC_UNISTD_C 1
#define _GNU_SOURCE         1
#define _KOS_SOURCE         2
#define _TIME64_SOURCE      1
#define _ATFILE_SOURCE      1
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS   32

#include "libc.h"
#include "system.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <grp.h>
#include <hybrid/align.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/kdev_t.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <malloc.h>
#include <mntent.h>
#include <poll.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <unistd.h>

DECL_BEGIN

PUBLIC long int (ATTR_CDECL syscall)(long int sysno, ...);
GLOBAL_ASM(
L(.section .text                                                              )
L(PUBLIC_ENTRY(syscall)                                                       )
#ifdef __i386__
L(    /* Save callee-save registers */                                        )
L(    pushl %ebx                                                              )
L(    pushl %edi                                                              )
L(    pushl %esi                                                              )
L(    pushl %ebp                                                              )
L(                                                                            )
L(    /* Load arguments */                                                    )
#define    A  20 /* return+4*register (4+4*4) */
L(    movl A+ 0(%esp), %eax /* sysno */                                       )
L(    movl A+ 4(%esp), %ebx /* Arg #1 */                                      )
L(    movl A+ 8(%esp), %ecx /* Arg #2 */                                      )
L(    movl A+12(%esp), %edx /* Arg #3 */                                      )
L(    movl A+16(%esp), %esi /* Arg #4 */                                      )
L(    movl A+20(%esp), %edi /* Arg #5 */                                      )
L(    movl A+24(%esp), %ebp /* Arg #6 */                                      )
#undef A
L(                                                                            )
L(    int $0x80 /* Invoke the system call interrupt vector */                 )
L(                                                                            )
L(    /* Restore callee-save registers */                                     )
L(    popl %ebp                                                               )
L(    popl %esi                                                               )
L(    popl %edi                                                               )
L(    popl %ebx                                                               )
L(    ret                                                                     )
#else
#error FIXME
#endif
L(SYM_END(syscall)                                                            )
L(.previous                                                                   )
);


#undef major
#undef minor
#undef makedev
PUBLIC major_t (LIBCCALL gnu_dev_major)(dev_t dev) { return MAJOR(dev); }
PUBLIC minor_t (LIBCCALL gnu_dev_minor)(dev_t dev) { return MINOR(dev); }
PUBLIC dev_t (LIBCCALL gnu_dev_makedev)(major_t major, minor_t minor) { return MKDEV(major,minor); }

PUBLIC off32_t (LIBCCALL lseek)(int fd, off32_t offset, int whence) { return FORWARD_SYSTEM_VALUE(sys_lseek32(fd,offset,whence)); }
PUBLIC ssize_t (LIBCCALL read)(int fd, void *buf, size_t n_bytes) { return FORWARD_SYSTEM_VALUE(sys_read(fd,buf,n_bytes)); }
PUBLIC ssize_t (LIBCCALL write)(int fd, void const *buf, size_t n_bytes) { return FORWARD_SYSTEM_VALUE(sys_write(fd,buf,n_bytes)); }
PUBLIC off64_t (LIBCCALL lseek64)(int fd, off64_t offset, int whence) { return FORWARD_SYSTEM_VALUE(sys_lseek(fd,offset,whence)); }
PUBLIC ssize_t (LIBCCALL pread)(int fd, void *buf, size_t n_bytes, off32_t offset) { return FORWARD_SYSTEM_VALUE(sys_pread32(fd,buf,n_bytes,offset)); }
PUBLIC ssize_t (LIBCCALL pwrite)(int fd, void const *buf, size_t n_bytes, off32_t offset) { return FORWARD_SYSTEM_VALUE(sys_pwrite32(fd,buf,n_bytes,offset)); }
PUBLIC ssize_t (LIBCCALL pread64)(int fd, void *buf, size_t n_bytes, off64_t offset) { return FORWARD_SYSTEM_VALUE(sys_pread64(fd,buf,n_bytes,offset)); }
PUBLIC ssize_t (LIBCCALL pwrite64)(int fd, void const *buf, size_t n_bytes, off64_t offset) { return FORWARD_SYSTEM_VALUE(sys_pwrite64(fd,buf,n_bytes,offset)); }

PUBLIC int (LIBCCALL truncate)(char const *file, off32_t length) { return FORWARD_SYSTEM_ERROR(sys_truncate32(file,length)); }
PUBLIC int (LIBCCALL truncate64)(char const *file, off64_t length) { return FORWARD_SYSTEM_ERROR(sys_truncate(file,length)); }
PUBLIC int (LIBCCALL ftruncate)(int fd, off32_t length) { return FORWARD_SYSTEM_ERROR(sys_ftruncate32(fd,length)); }
PUBLIC int (LIBCCALL ftruncate64)(int fd, off64_t length) { return FORWARD_SYSTEM_ERROR(sys_ftruncate(fd,length)); }
PUBLIC int (LIBCCALL fsync)(int fd) { return FORWARD_SYSTEM_ERROR(sys_fsync(fd)); }
PUBLIC int (LIBCCALL syncfs)(int fd) { return FORWARD_SYSTEM_ERROR(sys_syncfs(fd)); }
PUBLIC void (LIBCCALL sync)(void) { sys_sync(); }
PUBLIC int (LIBCCALL fdatasync)(int fd) { return FORWARD_SYSTEM_ERROR(sys_fdatasync(fd)); }
PUBLIC int (LIBCCALL close)(int fd) { return FORWARD_SYSTEM_ERROR(sys_close(fd)); }
PUBLIC int (LIBCCALL chroot)(char const *path) { return FORWARD_SYSTEM_ERROR(sys_chroot(path)); }
PUBLIC int (LIBCCALL chdir)(char const *path) { return FORWARD_SYSTEM_ERROR(sys_chdir(path)); }
PUBLIC int (LIBCCALL fchdir)(int fd) { return FORWARD_SYSTEM_ERROR(sys_fchdir(fd)); }
PUBLIC int (LIBCCALL dup)(int fd) { return FORWARD_SYSTEM_VALUE(sys_dup(fd)); }
PUBLIC int (LIBCCALL dup3)(int fd, int fd2, int flags) { return FORWARD_SYSTEM_VALUE(sys_dup3(fd,fd2,flags)); }
PUBLIC int (LIBCCALL dup2)(int fd, int fd2) { return fd == fd2 ? 0 : dup3(fd,fd2,0); }

/* NOTE: xstat64 has binary compatibility with regular xstat().
 *    >> All 32-bit fields will still be filled, while 64-bit fields are all located at the end.
 *       The different between xstat() and xstat64() is that the associated struct
 *       is either commenting out the 32-bit or 64-bit fields, making it appear as
 *       though only one is available at any time (when in fact both always are). */
STATIC_ASSERT(sizeof(struct stat) == sizeof(struct stat64));
DEFINE_PUBLIC_ALIAS(kstat,kstat64);
DEFINE_PUBLIC_ALIAS(klstat,klstat64);
DEFINE_PUBLIC_ALIAS(kfstat,kfstat64);
DEFINE_PUBLIC_ALIAS(kfstatat,kfstatat64);
PUBLIC int (LIBCCALL kfstat64)(int fd, struct stat64 *buf) { return FORWARD_SYSTEM_ERROR(sys_fstat64(fd,buf)); }
PUBLIC int (LIBCCALL kfstatat64)(int fd, char const *__restrict file, struct stat64 *__restrict buf, int flag) { return FORWARD_SYSTEM_ERROR(sys_fstatat64(fd,file,buf,flag)); }
PUBLIC int (LIBCCALL kstat64)(char const *__restrict file, struct stat64 *__restrict buf) { return fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_FOLLOW); }
PUBLIC int (LIBCCALL klstat64)(char const *__restrict file, struct stat64 *__restrict buf) { return fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
PUBLIC int (LIBCCALL access)(char const *name, int type) { return faccessat(AT_FDCWD,name,type,AT_SYMLINK_FOLLOW); }
PUBLIC int (LIBCCALL faccessat)(int fd, char const *file, int type, int flag) { return FORWARD_SYSTEM_ERROR(sys_faccessat(fd,file,type,flag)); }
PUBLIC int (LIBCCALL chown)(char const *file, uid_t owner, gid_t group) { return fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_FOLLOW); }
PUBLIC int (LIBCCALL lchown)(char const *file, uid_t owner, gid_t group) { return fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW); }
PUBLIC int (LIBCCALL fchown)(int fd, uid_t owner, gid_t group) { return FORWARD_SYSTEM_ERROR(sys_fchown(fd,owner,group)); }
PUBLIC int (LIBCCALL chmod)(char const *file, mode_t mode) { return fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_FOLLOW); }
PUBLIC int (LIBCCALL lchmod)(char const *file, mode_t mode) { return fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW); }
PUBLIC int (LIBCCALL fchmod)(int fd, mode_t mode) { return FORWARD_SYSTEM_ERROR(sys_fchmod(fd,mode)); }
PUBLIC int (LIBCCALL fchownat)(int fd, char const *file, uid_t owner, gid_t group, int flag) { return FORWARD_SYSTEM_ERROR(sys_fchownat(fd,file,owner,group,flag)); }
PUBLIC int (LIBCCALL fchmodat)(int fd, char const *file, mode_t mode, int flag) {
 if (!(flag&(AT_SYMLINK_NOFOLLOW|AT_SYMLINK_FOLLOW))) { __set_errno(EINVAL); return -1; }
 return FORWARD_SYSTEM_ERROR(sys_fchmodat(fd,file,mode|((flag&AT_SYMLINK_NOFOLLOW) ? O_NOFOLLOW : 0)));
}


PUBLIC int (LIBCCALL mknodat)(int fd, char const *path, mode_t mode, dev_t dev) { return FORWARD_SYSTEM_ERROR(sys_mknodat(fd,path,mode,dev)); }
PUBLIC int (LIBCCALL mkdirat)(int fd, char const *path, mode_t mode) { return FORWARD_SYSTEM_ERROR(sys_mkdirat(fd,path,mode)); }
PUBLIC int (LIBCCALL mkfifoat)(int fd, char const *path, mode_t mode) { return mknodat(fd,path,S_IFIFO|mode,0); }
PUBLIC int (LIBCCALL linkat)(int fromfd, char const *from, int tofd, char const *to, int flags) { return FORWARD_SYSTEM_ERROR(sys_linkat(fromfd,from,tofd,to,flags)); }
PUBLIC int (LIBCCALL symlinkat)(char const *from, int tofd, char const *to) { return FORWARD_SYSTEM_ERROR(sys_symlinkat(from,tofd,to)); }
PUBLIC int (LIBCCALL unlinkat)(int fd, char const *name, int flag) { return FORWARD_SYSTEM_ERROR(sys_unlinkat(fd,name,flag)); }
PUBLIC int (LIBCCALL renameat)(int oldfd, char const *old, int newfd, char const *new_) { return FORWARD_SYSTEM_ERROR(sys_renameat(oldfd,old,newfd,new_)); }
PUBLIC ssize_t (LIBCCALL readlinkat)(int fd, char const *__restrict path, char *__restrict buf, size_t len) { return FORWARD_SYSTEM_VALUE(sys_readlinkat(fd,path,buf,len)); }
PUBLIC int (LIBCCALL removeat)(int fd, char const *filename) { return unlinkat(fd,filename,AT_SYMLINK_NOFOLLOW|AT_REMOVEREG|AT_REMOVEDIR); }
PUBLIC int (LIBCCALL remove)(char const *filename) { return removeat(AT_FDCWD,filename); }
PUBLIC int (LIBCCALL rename)(char const *old, char const *new_) { return renameat(AT_FDCWD,old,AT_FDCWD,new_); }
PUBLIC pid_t (LIBCCALL fork)(void) { return FORWARD_SYSTEM_VALUE(sys_fork()); }
PUBLIC int (LIBCCALL execve)(char const *path, char *const argv[], char *const envp[]) { return SET_SYSTEM_ERROR(sys_execve(path,(char const *const *)argv,(char const *const *)envp)); }

#if defined(__i386__) && 0 /* TODO: Doesn't seem to work? */
GLOBAL_ASM(
L(.section .text                                              )
L(PUBLIC_ENTRY(execl)                                         )
L(    leal  8(%esp), %eax                                     )
L(    pushl %eax     /* argv */                               )
L(    pushl -4(%eax) /* path */                               )
L(    call  PLT_SYM(execv)                                    )
L(    addl  $8, %esp                                          )
L(    ret                                                     )
L(SYM_END(execl)                                              )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text                                              )
L(PUBLIC_ENTRY(execlp)                                        )
L(    leal 8(%esp), %eax                                      )
L(    pushl %eax     /* argv */                               )
L(    pushl -4(%esp) /* path */                               )
L(    call PLT_SYM(execvp)                                    )
#ifndef __KERNEL__ /* #if LIBCCALL != ATTR_STDCALL */
L(    addl $8, %esp                                           )
#endif
L(    ret                                                     )
L(SYM_END(execlp)                                             )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text                                              )
L(PUBLIC_ENTRY(execle)                                        )
L(    leal   8(%esp), %eax                                    )
L(    /* Scan ahead until the first NULL-pointer */           )
L(1:  addl  $4,       %eax                                    )
L(    testl $-1,   -4(%eax)                                   )
L(    jnz 1b                                                  )
L(    /* The value after the first NULL-pointer is envp */    )
L(    pushl   (%eax) /* envp */                               )
L(    leal   8(%esp), %eax                                    )
L(    pushl    %eax  /* argv */                               )
L(    pushl 12(%esp) /* path */                               )
L(    call PLT_SYM(execve)                                    )
#ifndef __KERNEL__ /* #if LIBCCALL != ATTR_STDCALL */
L(    addl $12, %esp                                          )
#endif
L(    ret                                                     )
L(SYM_END(execle)                                             )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text                                              )
L(PUBLIC_ENTRY(execlpe)                                       )
L(    leal   8(%esp), %eax                                    )
L(    /* Scan ahead until the first NULL-pointer */           )
L(1:  addl  $4,       %eax                                    )
L(    testl $-1,   -4(%eax)                                   )
L(    jnz 1b                                                  )
L(    /* The value after the first NULL-pointer is envp */    )
L(    pushl   (%eax) /* envp */                               )
L(    leal   8(%esp), %eax                                    )
L(    pushl    %eax  /* argv */                               )
L(    pushl 12(%esp) /* path */                               )
L(    call PLT_SYM(execvpe)                                   )
#ifndef __KERNEL__ /* #if LIBCCALL != ATTR_STDCALL */
L(    addl $12, %esp                                          )
#endif
L(    ret                                                     )
L(SYM_END(execlpe)                                            )
L(.previous                                                   )
);
#else

/* Generic C implementation. */
PRIVATE char **LIBCCALL generic_capture_argv(char *first, va_list *pargs) {
 size_t argc = 1,arga = 2;
 char *arg,**new_result,**result = tmalloc(char *,arga+1);
 if unlikely(!result) return NULL;
 result[0] = first;
 while ((arg = va_arg(*pargs,char *)) != NULL) {
  if (argc == arga) {
   new_result = (arga *= 2,trealloc(char *,result,arga+1));
   if unlikely(!new_result) { free(result); return NULL; }
   result = new_result;
  }
  result[argc++] = arg;
 }
 result[argc] = NULL; /* Terminate with NULL-pointer. */
 return result;
}

PUBLIC int (ATTR_CDECL execl)(char const *path, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = execv(path,argv);
 free(argv);
 return result;
}
PUBLIC int (ATTR_CDECL execle)(char const *path, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = execve(path,argv,va_arg(args,char **));
 free(argv);
 return result;
}
PUBLIC int (ATTR_CDECL execlp)(char const *file, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = execvp(file,argv);
 free(argv);
 return result;
}
PUBLIC int (ATTR_CDECL execlpe)(char const *file, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = execvpe(file,argv,va_arg(args,char **));
 free(argv);
 return result;
}
#endif

PUBLIC int (ATTR_CDECL fexecl)(int fd, char const *arg, ...);
PUBLIC int (ATTR_CDECL fexecle)(int fd, char const *arg, ...);
PUBLIC int (LIBCCALL fexecve)(int fd, char *const argv[], char *const envp[]);
PUBLIC int (LIBCCALL fexecv)(int fd, char *const argv[]);

PRIVATE void (LIBCCALL execvpe_inside)(char const *path, char const *file,
                                       char *const argv[], char *const envp[]) {
 char *buf;
 size_t pathlen = strlen(path);
 size_t filelen = strlen(file);
 buf = (char *)amalloc((pathlen+filelen+2)*sizeof(char));
 memcpy(buf,path,pathlen*sizeof(char));
 buf[pathlen] = '/';
 memcpy(buf+pathlen+1,file,filelen*sizeof(char));
 buf[pathlen+1+filelen] = '\0';
 /* Try to execute the joined path. */
 execve(buf,argv,envp);
 afree(buf);
}
PUBLIC int (LIBCCALL execvpe)(char const *file, char *const argv[], char *const envp[]) {
 char *iter,*part,*next,*path;
 if unlikely((path = getenv("PATH")) == NULL) { __set_errno(ENOENT); return -1; }
 if unlikely((path = strdupma(path)) == NULL) return -1;
 part = path;
 for (;;) {
  bool last = *(next = strchrnul(part,':')) != '\0';
  for (iter = next; iter[-1] == '/'; --iter);
  *iter = '\0';
  execvpe_inside(part,file,argv,envp);
  if (last) break;
  part = next+1;
 }
 afree(path);
 return -1;
}
PUBLIC int (LIBCCALL execv)(char const *path, char *const argv[]) { return execve(path,argv,environ); }
PUBLIC int (LIBCCALL execvp)(char const *file, char *const argv[]) { return execvpe(file,argv,environ); }
PUBLIC int (LIBCCALL link)(char const *from, char const *to) { return linkat(AT_FDCWD,from,AT_FDCWD,to,AT_SYMLINK_FOLLOW); }
PUBLIC int (LIBCCALL symlink)(char const *from, char const *to) { return symlinkat(from,AT_FDCWD,to); }
PUBLIC ssize_t (LIBCCALL readlink)(char const *__restrict path, char *__restrict buf, size_t len) { return readlinkat(AT_FDCWD,path,buf,len); }
PUBLIC int (LIBCCALL unlink)(char const *name) { return unlinkat(AT_FDCWD,name,AT_SYMLINK_NOFOLLOW|AT_REMOVEREG); }
PUBLIC int (LIBCCALL rmdir)(char const *path) { return unlinkat(AT_FDCWD,path,AT_SYMLINK_FOLLOW|AT_REMOVEDIR); }
PUBLIC int (LIBCCALL eaccess)(char const *name, int type) { return faccessat(AT_FDCWD,name,type,AT_EACCESS); }
DEFINE_PUBLIC_ALIAS(eaccess,euidaccess);
PUBLIC int (LIBCCALL mkdir)(char const *path, mode_t mode) { return mkdirat(AT_FDCWD,path,mode); }
PUBLIC int (LIBCCALL mkfifo)(char const *path, mode_t mode) { return mkfifoat(AT_FDCWD,path,mode); }
PUBLIC int (LIBCCALL mknod)(char const *path, mode_t mode, dev_t dev) { return mknodat(AT_FDCWD,path,mode,dev); }
PUBLIC pid_t (LIBCCALL wait)(__WAIT_STATUS stat_loc) { return wait4(-1,stat_loc,0,NULL); }
PUBLIC pid_t (LIBCCALL waitpid)(pid_t pid, __WAIT_STATUS stat_loc, int options) { return wait4(pid,stat_loc,options,NULL); }
PUBLIC pid_t (LIBCCALL wait3)(__WAIT_STATUS stat_loc, int options, struct rusage *usage) { return wait4(-1,stat_loc,options,usage); }
PUBLIC int (LIBCCALL waitid)(idtype_t idtype, id_t id, siginfo_t *infop, int options) { return FORWARD_SYSTEM_ERROR(sys_waitid(idtype,id,infop,options,NULL)); }
PUBLIC pid_t (LIBCCALL wait4)(pid_t pid, __WAIT_STATUS stat_loc, int options, struct rusage *usage) { return FORWARD_SYSTEM_VALUE(sys_wait4(pid,stat_loc,options,usage)); }
PUBLIC int (LIBCCALL getgroups)(int size, gid_t list[]) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setuid)(uid_t uid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setgid)(gid_t gid) { NOT_IMPLEMENTED(); return -1; }

PUBLIC int (LIBCCALL pipe)(int pipedes[2]) { return pipe2(pipedes,0); }
PUBLIC int (LIBCCALL pipe2)(int pipedes[2], int flags) {
#if 0
 return FORWARD_SYSTEM_ERROR(sys_pipe2(pipedes,flags));
#else
 union pipefd {
    s64     data;
 struct{union{
    errno_t error;
    int     fd_reader;
 }; int     fd_writer; };
 } pfd;
 pfd.data = sys_xpipe(flags);
 if (E_ISERR(pfd.error)) {
  __set_errno(-pfd.error);
  return -1;
 }
 pipedes[0] = pfd.fd_reader;
 pipedes[1] = pfd.fd_writer;
 return 0;
#endif
}
PUBLIC mode_t (LIBCCALL umask)(mode_t mask) { /* TODO */ return 0022; }
PUBLIC mode_t (LIBCCALL getumask)(void) { /* TODO */ return 0022; }

DEFINE_PUBLIC_ALIAS(__getpgid,getpgid);
PUBLIC pid_t (LIBCCALL getpid)(void) { return sys_getpid(); }
PUBLIC pid_t (LIBCCALL getppid)(void) { return sys_getppid(); }
PUBLIC pid_t (LIBCCALL getpgid)(pid_t pid) { return FORWARD_SYSTEM_VALUE(sys_getpgid(pid)); }
PUBLIC pid_t (LIBCCALL getpgrp)(void) { return getpgid(0); }
PUBLIC int (LIBCCALL setpgid)(pid_t pid, pid_t pgid) { return FORWARD_SYSTEM_ERROR(sys_setpgid(pid,pgid)); }
PUBLIC pid_t (LIBCCALL getsid)(pid_t pid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC pid_t (LIBCCALL setsid)(void) { NOT_IMPLEMENTED(); return -1; }
PUBLIC uid_t (LIBCCALL getuid)(void) { NOT_IMPLEMENTED(); return -1; }
PUBLIC gid_t (LIBCCALL getgid)(void) { NOT_IMPLEMENTED(); return -1; }
PUBLIC uid_t (LIBCCALL geteuid)(void) { NOT_IMPLEMENTED(); return -1; }
PUBLIC gid_t (LIBCCALL getegid)(void) { NOT_IMPLEMENTED(); return -1; }

PUBLIC unsigned int (LIBCCALL alarm)(unsigned int seconds) { NOT_IMPLEMENTED(); return seconds; }
PUBLIC int (LIBCCALL pause)(void) { NOT_IMPLEMENTED(); return -1; }

PUBLIC long int (LIBCCALL pathconf)(char const *path, int name) { NOT_IMPLEMENTED(); return -1; }
PUBLIC long int (LIBCCALL fpathconf)(int fd, int name) { NOT_IMPLEMENTED(); return -1; }
PUBLIC char *(LIBCCALL getlogin)(void) { NOT_IMPLEMENTED(); return NULL; }

PUBLIC int (LIBCCALL group_member)(gid_t gid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getresuid)(uid_t *ruid, uid_t *euid, uid_t *suid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getresgid)(gid_t *rgid, gid_t *egid, gid_t *sgid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setresuid)(uid_t ruid, uid_t euid, uid_t suid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setresgid)(gid_t rgid, gid_t egid, gid_t sgid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setreuid)(uid_t ruid, uid_t euid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setregid)(gid_t rgid, gid_t egid) { NOT_IMPLEMENTED(); return -1; }

PUBLIC useconds_t (LIBCCALL ualarm)(useconds_t value, useconds_t interval) { NOT_IMPLEMENTED(); return -1; }
PUBLIC pid_t (LIBCCALL vfork)(void) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL nice)(int inc) { NOT_IMPLEMENTED(); return -1; }
PUBLIC size_t (LIBCCALL confstr)(int name, char *buf, size_t len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setpgrp)(void) { NOT_IMPLEMENTED(); return -1; }
PUBLIC long int (LIBCCALL gethostid)(void) { NOT_IMPLEMENTED(); return -1; }
#ifdef PAGESIZE
PUBLIC int (LIBCCALL getpagesize)(void) { return PAGESIZE; }
#elif defined(_SC_PAGE_SIZE)
PUBLIC int (LIBCCALL getpagesize)(void) { return sysconf(_SC_PAGE_SIZE); }
#else
PUBLIC int (LIBCCALL getpagesize)(void) { return sysconf(_SC_PAGESIZE); }
#endif
PUBLIC int (LIBCCALL getdtablesize)(void) { return sysconf(_SC_OPEN_MAX); }
PUBLIC int (LIBCCALL seteuid)(uid_t uid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setegid)(gid_t gid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL ttyslot)(void) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getlogin_r)(char *name, size_t name_len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL gethostname)(char *name, size_t len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setlogin)(char const *name) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sethostname)(char const *name, size_t len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sethostid)(long int id) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getdomainname)(char *name, size_t len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setdomainname)(char const *name, size_t len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL vhangup)(void) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL revoke)(char const *file) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL profil)(unsigned short int *sample_buffer, size_t size, size_t offset, unsigned int scale) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL acct)(char const *name) { NOT_IMPLEMENTED(); return -1; }
PUBLIC char *(LIBCCALL getusershell)(void) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC void (LIBCCALL endusershell)(void) { NOT_IMPLEMENTED(); }
PUBLIC void (LIBCCALL setusershell)(void) { NOT_IMPLEMENTED(); }
PUBLIC int (LIBCCALL daemon)(int nochdir, int noclose) { NOT_IMPLEMENTED(); return -1; }
PUBLIC char *(LIBCCALL getpass)(char const *prompt) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC char *(LIBCCALL crypt)(char const *key, char const *salt) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC void (LIBCCALL encrypt)(char *glibc_block, int edflag) { NOT_IMPLEMENTED(); }
PUBLIC void (LIBCCALL swab)(void const *__restrict from, void *__restrict to, ssize_t n_bytes) { NOT_IMPLEMENTED(); }
PUBLIC char *(LIBCCALL ctermid)(char *s) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC char *(LIBCCALL cuserid)(char *s) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL lockf)(int fd, int cmd, off32_t len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL lockf64)(int fd, int cmd, off64_t len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC struct passwd *(LIBCCALL getpwuid)(uid_t uid) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct passwd *(LIBCCALL getpwnam)(char const *name) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC void (LIBCCALL setpwent)(void) { NOT_IMPLEMENTED(); }
PUBLIC void (LIBCCALL endpwent)(void) { NOT_IMPLEMENTED(); }
PUBLIC struct passwd *(LIBCCALL getpwent)(void) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct passwd *(LIBCCALL fgetpwent)(FILE *stream) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL putpwent)(struct passwd const *__restrict p, FILE *__restrict f) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getpwuid_r)(uid_t uid, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getpwnam_r)(char const *__restrict name, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getpwent_r)(struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL fgetpwent_r)(FILE *__restrict stream, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getpw)(uid_t uid, char *buffer) { NOT_IMPLEMENTED(); return -1; }
PUBLIC struct group *(LIBCCALL getgrgid)(gid_t gid) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct group *(LIBCCALL getgrnam)(char const *name) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC void (LIBCCALL setgrent)(void) { NOT_IMPLEMENTED(); }
PUBLIC void (LIBCCALL endgrent)(void) { NOT_IMPLEMENTED(); }
PUBLIC struct group *(LIBCCALL getgrent)(void) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL putgrent)(struct group const *__restrict p, FILE *__restrict f) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getgrgid_r)(gid_t gid, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getgrnam_r)(char const *__restrict name, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getgrent_r)(struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL fgetgrent_r)(FILE *__restrict stream, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { NOT_IMPLEMENTED(); return -1; }
PUBLIC struct group *(LIBCCALL fgetgrent)(FILE *stream) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL setgroups)(size_t __n, const gid_t *groups) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getgrouplist)(char const *user, gid_t group, gid_t *groups, int *ngroups) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL initgroups)(char const *user, gid_t group) { NOT_IMPLEMENTED(); return -1; }
PUBLIC FILE *(LIBCCALL setmntent)(char const *file, char const *mode) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct mntent *(LIBCCALL getmntent)(FILE *stream) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct mntent *(LIBCCALL getmntent_r)(FILE *__restrict stream, struct mntent *__restrict result, char *__restrict buffer, int bufsize) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL addmntent)(FILE *__restrict stream, const struct mntent *__restrict mnt) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL endmntent)(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
PUBLIC char *(LIBCCALL hasmntopt)(const struct mntent *mnt, char const *opt) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL fnmatch)(char const *pattern, char const *name, int flags) { return FNM_NOSYS; } /* TODO */
PUBLIC clock_t (LIBCCALL times)(struct tms *buffer) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getrlimit)(__rlimit_resource_t resource, struct rlimit *rlimits) { return prlimit(getpid(),(enum __rlimit_resource)resource,NULL,rlimits); }
PUBLIC int (LIBCCALL setrlimit)(__rlimit_resource_t resource, struct rlimit const *rlimits) { return prlimit(getpid(),(enum __rlimit_resource)resource,rlimits,NULL); }
PUBLIC int (LIBCCALL getrlimit64)(__rlimit_resource_t resource, struct rlimit64 *rlimits) { return prlimit64(getpid(),(enum __rlimit_resource)resource,NULL,rlimits); }
PUBLIC int (LIBCCALL setrlimit64)(__rlimit_resource_t resource, struct rlimit64 const *rlimits) { return prlimit64(getpid(),(enum __rlimit_resource)resource,rlimits,NULL); }
PUBLIC int (LIBCCALL getpriority)(__priority_which_t which, id_t who) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setpriority)(__priority_which_t which, id_t who, int prio) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL prlimit)(pid_t pid, enum __rlimit_resource resource, struct rlimit const *new_limit, struct rlimit *old_limit) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL prlimit64)(pid_t pid, enum __rlimit_resource resource, struct rlimit64 const *new_limit, struct rlimit64 *old_limit) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL umount)(char const *special_file) { return umount2(special_file,0); }
PUBLIC int (LIBCCALL mount)(char const *special_file, char const *dir, char const *fstype, unsigned long int rwflag, void const *data) { return FORWARD_SYSTEM_ERROR(sys_mount(special_file,dir,fstype,rwflag,data)); }
PUBLIC int (LIBCCALL umount2)(char const *special_file, int flags) { return FORWARD_SYSTEM_ERROR(sys_umount2(special_file,flags)); }
PUBLIC int (LIBCCALL getrusage)(__rusage_who_t who, struct rusage *usage) { memset(usage,0,sizeof(struct rusage)); return 0; } /* xxx? */



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
PUBLIC int (LIBCCALL glibc_stat)(char const *path, struct glibc_stat *statbuf) __ASMNAME("stat");
PUBLIC int (LIBCCALL glibc_lstat)(char const *path, struct glibc_stat *statbuf) __ASMNAME("lstat");
PUBLIC int (LIBCCALL glibc_fstat)(int fd, struct glibc_stat *statbuf) __ASMNAME("fstat");
PUBLIC int (LIBCCALL glibc_fstatat)(int fd, char const *filename, struct glibc_stat *statbuf, int flag) __ASMNAME("fstatat");

LOCAL void LIBCCALL
glibc_load_stat_buffer(struct glibc_stat *__restrict dst,
                       struct stat64 const *__restrict src) {
 dst->st_dev = (glibc_dev_t)src->st_dev;
#ifdef __x86_64__
 dst->st_ino   = (glibc_ino_t)src->st_ino64;
#else
 dst->st_ino32 = src->st_ino32;
 dst->st_ino64 = src->st_ino64;
#endif
 dst->st_nlink   = (glibc_nlink_t)src->st_nlink;
 dst->st_mode    = (glibc_mode_t)src->st_mode;
 dst->st_uid     = (glibc_uid_t)src->st_uid;
 dst->st_gid     = (glibc_gid_t)src->st_gid;
 dst->st_rdev    = (glibc_dev_t)src->st_rdev;
 dst->st_size    = (glibc_off64_t)src->st_size;
 dst->st_blksize = (glibc_blksize_t)src->st_blksize;
 dst->st_blocks  = (glibc_blkcnt64_t)src->st_blocks;
 dst->st_atim    = src->st_atim32;
 dst->st_mtim    = src->st_mtim32;
 dst->st_ctim    = src->st_ctim32;
}

PUBLIC int (LIBCCALL glibc_fstat)(int fd, struct glibc_stat *statbuf) {
 struct stat64 buf; int result;
 result = kfstat64(fd,&buf);
 if (!result) glibc_load_stat_buffer(statbuf,&buf);
 return result;
}
PUBLIC int (LIBCCALL glibc_fstatat)(int fd, char const *filename, struct glibc_stat *statbuf, int flag) {
 struct stat64 buf; int result;
 result = kfstatat64(fd,filename,&buf,flag);
 if (!result) glibc_load_stat_buffer(statbuf,&buf);
 return result;
}
PUBLIC int (LIBCCALL glibc_stat)(char const *path, struct glibc_stat *statbuf) { return glibc_fstatat(AT_FDCWD,path,statbuf,AT_SYMLINK_FOLLOW); }
PUBLIC int (LIBCCALL glibc_lstat)(char const *path, struct glibc_stat *statbuf) { return glibc_fstatat(AT_FDCWD,path,statbuf,AT_SYMLINK_NOFOLLOW); }
#define VERCHK { if (ver != 0) { __set_errno(-EINVAL); return -1; } }
PUBLIC int (LIBCCALL __fxstat)(int ver, int fd, struct glibc_stat *statbuf) { VERCHK return glibc_fstat(fd,statbuf); }
PUBLIC int (LIBCCALL __xstat)(int ver, char const *filename, struct glibc_stat *statbuf) { VERCHK return glibc_stat(filename,statbuf); }
PUBLIC int (LIBCCALL __lxstat)(int ver, char const *filename, struct glibc_stat *statbuf) { VERCHK return glibc_lstat(filename,statbuf); }
PUBLIC int (LIBCCALL __fxstatat)(int ver, int fd, char const *filename, struct glibc_stat *statbuf, int flag) { VERCHK return glibc_fstatat(fd,filename,statbuf,flag); }
#undef VERCHK
/* GLIBC does the same trick where the 32-bit stat
 * buffer has binary compatibility with the 64-bit one! */
DEFINE_PUBLIC_ALIAS(__fxstat64,__fxstat);
DEFINE_PUBLIC_ALIAS(__xstat64,__xstat);
DEFINE_PUBLIC_ALIAS(__lxstat64,__lxstat);
DEFINE_PUBLIC_ALIAS(__fxstatat64,__fxstatat);
DEFINE_PUBLIC_ALIAS(stat64,stat);
DEFINE_PUBLIC_ALIAS(lstat64,lstat);
DEFINE_PUBLIC_ALIAS(fstat64,fstat);
DEFINE_PUBLIC_ALIAS(fstatat64,fstatat);




PRIVATE struct utsname const *kernel_utsname = NULL;
PUBLIC int (LIBCCALL uname)(struct utsname *name) {
 struct utsname const *data;
 /* Lazily load uname information from the kernel. */
 if ((data = kernel_utsname) == NULL) {
  data = (struct utsname const *)sys_xsharesym("uname");
  if unlikely(!data) { __set_errno(ENOSYS); return -1; }
  ATOMIC_CMPXCH(kernel_utsname,NULL,data);
 }
 memcpy(name,data,sizeof(struct utsname));
 return 0;
}


extern byte_t _end[]; /*< Automatically defined by the linker (end of '.bss'). */
PRIVATE byte_t *brk_curr = NULL;
PRIVATE DEFINE_ATOMIC_RWLOCK(brk_lock);

PRIVATE int (LIBCCALL do_brk)(void *addr) {
 byte_t *real_oldbrk,*real_newbrk,*oldbrk;
 oldbrk = brk_curr;
 if (!oldbrk) oldbrk = _end;
 real_oldbrk = (byte_t *)FLOOR_ALIGN((uintptr_t)oldbrk,4096);
 real_newbrk = (byte_t *)CEIL_ALIGN((uintptr_t)addr,4096);
 if (real_newbrk < real_oldbrk) {
  /* Release memory */
  if unlikely(munmap(real_newbrk,real_oldbrk-real_newbrk) == -1)
     return -1;
 } else if (real_newbrk > real_oldbrk) {
  void *map_result;
  /* Allocate more memory */
  map_result = mmap(real_oldbrk,real_newbrk-real_oldbrk,
                    PROT_READ|PROT_WRITE,MAP_FIXED|MAP_ANONYMOUS,-1,0);
  if unlikely(map_result == MAP_FAILED) return -1;
  assertf(map_result == real_oldbrk,"%p != %p",map_result,real_oldbrk);
 }
 brk_curr = (byte_t *)addr;
 return 0;
}

PUBLIC int (LIBCCALL brk)(void *addr) {
 int result;
 atomic_rwlock_write(&brk_lock);
 result = do_brk(addr);
 atomic_rwlock_endwrite(&brk_lock);
 return result;
}

PUBLIC void *(LIBCCALL sbrk)(intptr_t increment) {
 byte_t *result;
 atomic_rwlock_write(&brk_lock);
 result = brk_curr;
 if (do_brk(result+increment) != 0)
     result = (byte_t *)-1;
 atomic_rwlock_endwrite(&brk_lock);
 return result;
}

/*
PRIORITY IMPLEMENTATION (Easy)
         realpath

         sscanf

         strcspn
         strftime
         strpbrk
         strptime
         strsep
         strspn
         strtok
         strverscmp

PRIORITY IMPLEMENTATION (Hard)
         clearerr
         fclose
         fdopen
         fopen
         freopen
         setbuf

         mkdtemp
         mkstemp
         mktemp

         getegid
         geteuid
         getgid
         getgrgid
         getgrnam
         getgrouplist
         getgroups
         gethostid
         getlogin_r
         getpriority
         getpwnam
         getpwuid
         getrlimit
         gettimeofday
         getuid
         pipe
         sched_getaffinity
         setpriority
         setrlimit
         setsid
         sigsuspend
         stime
         strsignal
         sysconf
         time
         times
         umask
         vfork
*/

DECL_END

#endif /* !GUARD_LIBS_LIBC_UNISTD_C */
