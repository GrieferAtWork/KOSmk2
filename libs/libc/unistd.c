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
#include "errno.h"
#include "unistd.h"
#include "stdio.h"
#include "malloc.h"
#include "string.h"
#include "sysconf.h"
#include "misc.h"
#include "environ.h"
#include "unicode.h"
#include "fcntl.h"

#include <hybrid/asm.h>
#include <hybrid/section.h>
#include <bits/stat.h>
#include <bits/confname.h>
#include <bits/fcntl-linux.h>
#include <io.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <hybrid/atomic.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <sys/utsname.h>
#include <hybrid/align.h>
#include <sys/mman.h>
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include <direct.h>
#include <stdlib.h>
#endif

DECL_BEGIN

GLOBAL_ASM(
L(.section .text                                                              )
L(INTERN_ENTRY(libc_syscall)                                                  )
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
L(SYM_END(libc_syscall)                                                       )
L(.previous                                                                   )
);


#undef major
#undef minor
#undef makedev
INTERN major_t LIBCCALL libc_gnu_dev_major(dev_t dev) { return MAJOR(dev); }
INTERN minor_t LIBCCALL libc_gnu_dev_minor(dev_t dev) { return MINOR(dev); }
INTERN dev_t LIBCCALL libc_gnu_dev_makedev(major_t major, minor_t minor) { return MKDEV(major,minor); }

INTERN off32_t LIBCCALL libc_lseek(int fd, off32_t offset, int whence) { return FORWARD_SYSTEM_VALUE(sys_lseek32(fd,offset,whence)); }
INTERN ssize_t LIBCCALL libc_read(int fd, void *buf, size_t n_bytes) { return FORWARD_SYSTEM_VALUE(sys_read(fd,buf,n_bytes)); }
INTERN ssize_t LIBCCALL libc_write(int fd, void const *buf, size_t n_bytes) { return FORWARD_SYSTEM_VALUE(sys_write(fd,buf,n_bytes)); }
INTERN off64_t LIBCCALL libc_lseek64(int fd, off64_t offset, int whence) { return FORWARD_SYSTEM_VALUE(sys_lseek(fd,offset,whence)); }
INTERN ssize_t LIBCCALL libc_pread(int fd, void *buf, size_t n_bytes, off32_t offset) { return FORWARD_SYSTEM_VALUE(sys_pread32(fd,buf,n_bytes,offset)); }
INTERN ssize_t LIBCCALL libc_pwrite(int fd, void const *buf, size_t n_bytes, off32_t offset) { return FORWARD_SYSTEM_VALUE(sys_pwrite32(fd,buf,n_bytes,offset)); }
INTERN ssize_t LIBCCALL libc_pread64(int fd, void *buf, size_t n_bytes, off64_t offset) { return FORWARD_SYSTEM_VALUE(sys_pread64(fd,buf,n_bytes,offset)); }
INTERN ssize_t LIBCCALL libc_pwrite64(int fd, void const *buf, size_t n_bytes, off64_t offset) { return FORWARD_SYSTEM_VALUE(sys_pwrite64(fd,buf,n_bytes,offset)); }

INTERN int LIBCCALL libc_truncate(char const *file, off32_t length) { return FORWARD_SYSTEM_ERROR(sys_truncate32(file,length)); }
INTERN int LIBCCALL libc_truncate64(char const *file, off64_t length) { return FORWARD_SYSTEM_ERROR(sys_truncate(file,length)); }
INTERN int LIBCCALL libc_ftruncate(int fd, off32_t length) { return FORWARD_SYSTEM_ERROR(sys_ftruncate32(fd,length)); }
INTERN int LIBCCALL libc_ftruncate64(int fd, off64_t length) { return FORWARD_SYSTEM_ERROR(sys_ftruncate(fd,length)); }
INTERN int LIBCCALL libc_fsync(int fd) { return FORWARD_SYSTEM_ERROR(sys_fsync(fd)); }
INTERN int LIBCCALL libc_syncfs(int fd) { return FORWARD_SYSTEM_ERROR(sys_syncfs(fd)); }
INTERN void LIBCCALL libc_sync(void) { sys_sync(); }
INTERN int LIBCCALL libc_fdatasync(int fd) { return FORWARD_SYSTEM_ERROR(sys_fdatasync(fd)); }
INTERN int LIBCCALL libc_close(int fd) { return FORWARD_SYSTEM_ERROR(sys_close(fd)); }
INTERN int LIBCCALL libc_chroot(char const *path) { return FORWARD_SYSTEM_ERROR(sys_chroot(path)); }
INTERN int LIBCCALL libc_chdir(char const *path) { return FORWARD_SYSTEM_ERROR(sys_chdir(path)); }
INTERN int LIBCCALL libc_fchdir(int fd) { return FORWARD_SYSTEM_ERROR(sys_fchdir(fd)); }
INTERN int LIBCCALL libc_fchdirat(int dfd, char const *path, int flags) { return FORWARD_SYSTEM_ERROR(sys_xfchdirat(dfd,path,flags)); }
INTERN int LIBCCALL libc_dup(int fd) { return FORWARD_SYSTEM_VALUE(sys_dup(fd)); }
INTERN int LIBCCALL libc_dup3(int fd, int fd2, int flags) { return FORWARD_SYSTEM_VALUE(sys_dup3(fd,fd2,flags)); }
INTERN int LIBCCALL libc_dup2(int fd, int fd2) { return fd == fd2 ? fd2 : libc_dup3(fd,fd2,0); }

/* NOTE: xstat64 has binary compatibility with regular xstat().
 *    >> All 32-bit fields will still be filled, while 64-bit fields are all located at the end.
 *       The different between xstat() and xstat64() is that the associated struct
 *       is either commenting out the 32-bit or 64-bit fields, making it appear as
 *       though only one is available at any time (when in fact both always are). */
STATIC_ASSERT(sizeof(struct stat) == sizeof(struct stat64));
INTERN int LIBCCALL libc_kfstat64(int fd, struct stat64 *buf) { return FORWARD_SYSTEM_ERROR(sys_fstat64(fd,buf)); }
INTERN int LIBCCALL libc_kfstatat64(int fd, char const *__restrict file, struct stat64 *__restrict buf, int flags) { return FORWARD_SYSTEM_ERROR(sys_fstatat64(fd,file,buf,flags)); }
INTERN int LIBCCALL libc_kstat64(char const *__restrict file, struct stat64 *__restrict buf) { return libc_kfstatat64(AT_FDCWD,file,buf,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_klstat64(char const *__restrict file, struct stat64 *__restrict buf) { return libc_kfstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
INTERN int LIBCCALL libc_access(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_eaccess(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,AT_EACCESS|AT_SYMLINK_NOFOLLOW); }
INTERN int LIBCCALL libc_faccessat(int fd, char const *file, int type, int flags) { return FORWARD_SYSTEM_ERROR(sys_faccessat(fd,file,type,flags)); }
INTERN int LIBCCALL libc_chown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_lchown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW); }
INTERN int LIBCCALL libc_fchown(int fd, uid_t owner, gid_t group) { return FORWARD_SYSTEM_ERROR(sys_fchown(fd,owner,group)); }
INTERN int LIBCCALL libc_chmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_lchmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW); }
INTERN int LIBCCALL libc_fchmod(int fd, mode_t mode) { return FORWARD_SYSTEM_ERROR(sys_fchmod(fd,mode)); }
INTERN int LIBCCALL libc_fchownat(int fd, char const *file, uid_t owner, gid_t group, int flags) { return FORWARD_SYSTEM_ERROR(sys_fchownat(fd,file,owner,group,flags)); }
INTERN int LIBCCALL libc_fchmodat(int fd, char const *file, mode_t mode, int flags) {
 if (!(flags&(AT_SYMLINK_NOFOLLOW|AT_SYMLINK_FOLLOW))) { SET_ERRNO(EINVAL); return -1; }
 return FORWARD_SYSTEM_ERROR(sys_fchmodat(fd,file,mode|((flags&AT_SYMLINK_NOFOLLOW) ? O_NOFOLLOW : 0)));
}


INTERN int LIBCCALL libc_mknodat(int fd, char const *path, mode_t mode, dev_t dev) { return FORWARD_SYSTEM_ERROR(sys_mknodat(fd,path,mode,dev)); }
INTERN int LIBCCALL libc_mkdirat(int fd, char const *path, mode_t mode) { return FORWARD_SYSTEM_ERROR(sys_mkdirat(fd,path,mode)); }
INTERN int LIBCCALL libc_mkfifoat(int fd, char const *path, mode_t mode) { return libc_mknodat(fd,path,S_IFIFO|mode,0); }
INTERN int LIBCCALL libc_linkat(int fromfd, char const *from, int tofd, char const *to, int flags) { return FORWARD_SYSTEM_ERROR(sys_linkat(fromfd,from,tofd,to,flags)); }
INTERN int LIBCCALL libc_symlinkat(char const *from, int tofd, char const *to) { return FORWARD_SYSTEM_ERROR(sys_symlinkat(from,tofd,to)); }
INTERN int LIBCCALL libc_unlinkat(int fd, char const *name, int flags) {
 libc_printf("unlinkat(%d,%q,%x)\n",fd,name,flags);
 return FORWARD_SYSTEM_ERROR(sys_unlinkat(fd,name,flags));
}
INTERN int LIBCCALL libc_renameat(int oldfd, char const *old, int newfd, char const *new_) { return FORWARD_SYSTEM_ERROR(sys_renameat(oldfd,old,newfd,new_)); }
INTERN int LIBCCALL libc_frenameat(int oldfd, char const *old, int newfd, char const *new_, int flags) { return FORWARD_SYSTEM_ERROR(sys_xrenameat(oldfd,old,newfd,new_,flags)); }
INTERN ssize_t LIBCCALL libc_readlinkat(int fd, char const *__restrict path, char *__restrict buf, size_t len) {
 /* XXX: KOS has different (admittedly better) semantics for readlink().
  *   >> POSIX readlink() does not append a \0-character.
  *   >> POSIX readlink() does not return the amount of required bytes, but the amount written. */
 return FORWARD_SYSTEM_VALUE(sys_readlinkat(fd,path,buf,len));
}
INTERN int LIBCCALL libc_removeat(int fd, char const *filename) { return libc_unlinkat(fd,filename,AT_SYMLINK_NOFOLLOW|AT_REMOVEREG|AT_REMOVEDIR); }
INTERN int LIBCCALL libc_remove(char const *filename) { return libc_removeat(AT_FDCWD,filename); }
INTERN int LIBCCALL libc_rename(char const *old, char const *new_) { return libc_renameat(AT_FDCWD,old,AT_FDCWD,new_); }
INTERN pid_t LIBCCALL libc_fork(void) { return FORWARD_SYSTEM_VALUE(sys_fork()); }
INTERN int LIBCCALL libc_execve(char const *path, char *const argv[], char *const envp[]) { return SET_SYSTEM_ERROR(sys_execve(path,(char const *const *)argv,(char const *const *)envp)); }
INTERN int LIBCCALL libc_fexecve(int fd, char *const argv[], char *const envp[]) { return SET_SYSTEM_ERROR(sys_xfexecve(fd,(char const *const *)argv,(char const *const *)envp)); }

#if defined(__i386__)
GLOBAL_ASM(
L(.section .text                                              )
L(INTERN_ENTRY(libc_execl)                                    )
L(    leal  8(%esp), %eax                                     )
L(    pushl %eax     /* argv */                               )
L(    pushl -4(%eax) /* path */                               )
L(    call  libc_execv                                        )
L(    addl  $8, %esp                                          )
L(    ret                                                     )
L(SYM_END(libc_execl)                                         )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text                                              )
L(INTERN_ENTRY(libc_execlp)                                   )
L(    leal 8(%esp), %eax                                      )
L(    pushl %eax     /* argv */                               )
L(    pushl -4(%esp) /* path */                               )
L(    call libc_execvp                                        )
#ifndef __KERNEL__ /* #if LIBCCALL != ATTR_STDCALL */
L(    addl $8, %esp                                           )
#endif
L(    ret                                                     )
L(SYM_END(libc_execlp)                                        )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text                                              )
L(INTERN_ENTRY(libc_execle)                                   )
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
L(    call libc_execve                                        )
#ifndef __KERNEL__ /* #if LIBCCALL != ATTR_STDCALL */
L(    addl $12, %esp                                          )
#endif
L(    ret                                                     )
L(SYM_END(libc_execle)                                        )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text                                              )
L(INTERN_ENTRY(libc_execlpe)                                  )
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
L(    call libc_execvpe                                       )
#ifndef __KERNEL__ /* #if LIBCCALL != ATTR_STDCALL */
L(    addl $12, %esp                                          )
#endif
L(    ret                                                     )
L(SYM_END(libc_execlpe)                                       )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text                                              )
L(INTERN_ENTRY(libc_fexecl)                                   )
L(    leal  8(%esp), %eax                                     )
L(    pushl %eax     /* argv */                               )
L(    pushl -4(%eax) /* fd */                                 )
L(    call  libc_fexecv                                       )
L(    addl  $8, %esp                                          )
L(    ret                                                     )
L(SYM_END(libc_fexecl)                                        )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text                                              )
L(INTERN_ENTRY(libc_fexecle)                                  )
L(    leal   8(%esp), %eax                                    )
L(    /* Scan ahead until the first NULL-pointer */           )
L(1:  addl  $4,       %eax                                    )
L(    testl $-1,   -4(%eax)                                   )
L(    jnz 1b                                                  )
L(    /* The value after the first NULL-pointer is envp */    )
L(    pushl   (%eax) /* envp */                               )
L(    leal   8(%esp), %eax                                    )
L(    pushl    %eax  /* argv */                               )
L(    pushl 12(%esp) /* fd */                                 )
L(    call libc_fexecve                                       )
#ifndef __KERNEL__ /* #if LIBCCALL != ATTR_STDCALL */
L(    addl $12, %esp                                          )
#endif
L(    ret                                                     )
L(SYM_END(libc_fexecle)                                       )
L(.previous                                                   )
);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
GLOBAL_ASM(
L(.section .text.dos                                          )
L(INTERN_ENTRY(libc_dos_execl)                                )
L(    leal  8(%esp), %eax                                     )
L(    pushl %eax     /* argv */                               )
L(    pushl -4(%eax) /* path */                               )
L(    call  libc_dos_execv                                    )
L(    addl  $8, %esp                                          )
L(    ret                                                     )
L(SYM_END(libc_dos_execl)                                     )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text.dos                                          )
L(INTERN_ENTRY(libc_dos_execlp)                               )
L(    leal 8(%esp), %eax                                      )
L(    pushl %eax     /* argv */                               )
L(    pushl -4(%esp) /* path */                               )
L(    call libc_dos_execvp                                    )
#ifndef __KERNEL__ /* #if LIBCCALL != ATTR_STDCALL */
L(    addl $8, %esp                                           )
#endif
L(    ret                                                     )
L(SYM_END(libc_dos_execlp)                                    )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text.dos                                          )
L(INTERN_ENTRY(libc_dos_execle)                               )
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
L(    call libc_dos_execve                                    )
#ifndef __KERNEL__ /* #if LIBCCALL != ATTR_STDCALL */
L(    addl $12, %esp                                          )
#endif
L(    ret                                                     )
L(SYM_END(libc_dos_execle)                                    )
L(.previous                                                   )
);

GLOBAL_ASM(
L(.section .text.dos                                          )
L(INTERN_ENTRY(libc_dos_execlpe)                              )
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
L(    call libc_dos_execvpe                                   )
#ifndef __KERNEL__ /* #if LIBCCALL != ATTR_STDCALL */
L(    addl $12, %esp                                          )
#endif
L(    ret                                                     )
L(SYM_END(libc_dos_execlpe)                                   )
L(.previous                                                   )
);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

#else

/* Generic C implementation. */
PRIVATE char **LIBCCALL
generic_capture_argv(char *first, va_list *pargs) {
 size_t argc = 1,arga = 2;
 char *arg,**new_result,**result;
 result = (char **)libc_malloc((arga+1)*sizeof(char *));
 if unlikely(!result) return NULL;
 result[0] = first;
 while ((arg = va_arg(*pargs,char *)) != NULL) {
  if (argc == arga) {
   new_result = (arga *= 2,(char **)libc_realloc(result,(arga+1)*sizeof(char *)));
   if unlikely(!new_result) { libc_free(result); return NULL; }
   result = new_result;
  }
  result[argc++] = arg;
 }
 result[argc] = NULL; /* Terminate with NULL-pointer. */
 return result;
}

INTERN int ATTR_CDECL libc_execl(char const *path, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = libc_execv(path,argv);
 libc_free(argv);
 return result;
}
INTERN int ATTR_CDECL libc_execle(char const *path, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = libc_execve(path,argv,va_arg(args,char **));
 libc_free(argv);
 return result;
}
INTERN int ATTR_CDECL libc_execlp(char const *file, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = libc_execvp(file,argv);
 libc_free(argv);
 return result;
}
INTERN int ATTR_CDECL libc_execlpe(char const *file, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = libc_execvpe(file,argv,va_arg(args,char **));
 libc_free(argv);
 return result;
}
INTERN int ATTR_CDECL libc_fexecl(int fd, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = libc_fexecv(fd,argv);
 libc_free(argv);
 return result;
}
INTERN int ATTR_CDECL libc_fexecle(int fd, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = libc_fexecve(fd,argv,va_arg(args,char **));
 libc_free(argv);
 return result;
}
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN int ATTR_CDECL libc_dos_execl(char const *path, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = libc_dos_execv(path,argv);
 libc_free(argv);
 return result;
}
INTERN int ATTR_CDECL libc_dos_execle(char const *path, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = libc_dos_execve(path,argv,va_arg(args,char **));
 libc_free(argv);
 return result;
}
INTERN int ATTR_CDECL libc_dos_execlp(char const *file, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = libc_dos_execvp(file,argv);
 libc_free(argv);
 return result;
}
INTERN int ATTR_CDECL libc_dos_execlpe(char const *file, char const *arg, ...) {
 va_list args; char **argv; int result;
 va_start(args,arg);
 argv = generic_capture_argv((char *)arg,&args);
 va_end(args);
 result = libc_dos_execvpe(file,argv,va_arg(args,char **));
 libc_free(argv);
 return result;
}
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

#endif

DATDEF char **environ;
INTERN int LIBCCALL libc_fexecv(int fd, char *const argv[]) { return libc_fexecve(fd,argv,environ); }
#ifdef CONFIG_LIBC_NO_DOS_LIBC
PRIVATE void LIBCCALL execvpe_inside(char const *path, char const *file,
                                     char *const argv[], char *const envp[])
#else
PRIVATE void LIBCCALL execvpe_inside(char const *path, char const *file,
                                     char *const argv[], char *const envp[],
                                     bool dosmode)
#endif
{
 char *buf; bool use_malloc;
 size_t pathlen = libc_strlen(path);
 size_t filelen = libc_strlen(file);
 size_t bufsize = (pathlen+filelen+2)*sizeof(char);
 use_malloc = (bufsize > __AMALLOC_MAX);
 if (use_malloc) {
  buf = (char *)libc_malloc(bufsize);
  if unlikely(!buf) return;
 } else {
  buf = (char *)alloca(bufsize);
 }
 libc_memcpy(buf,path,pathlen*sizeof(char));
 buf[pathlen] = '/';
 libc_memcpy(buf+pathlen+1,file,filelen*sizeof(char));
 buf[pathlen+1+filelen] = '\0';
 /* Try to execute the joined path. */
#ifndef CONFIG_LIBC_NO_DOS_LIBC
 if (dosmode) {
  libc_dos_execve(buf,argv,envp);
 } else
#endif
 {
  libc_execve(buf,argv,envp);
 }

 if (use_malloc) libc_free(buf);
}
#ifdef CONFIG_LIBC_NO_DOS_LIBC
INTERN int LIBCCALL libc_execvpe(char const *file, char *const argv[], char *const envp[])
#else
INTERN int LIBCCALL libc_execvpe_impl(char const *file, char *const argv[],
                                      char *const envp[], bool dosmode);
INTERN int LIBCCALL libc_execvpe(char const *file, char *const argv[], char *const envp[]) { return libc_execvpe_impl(file,argv,envp,false); }
INTERN int LIBCCALL libc_dos_execvpe(char const *file, char *const argv[], char *const envp[]) { return libc_execvpe_impl(file,argv,envp,true); }
INTERN int LIBCCALL libc_execvpe_impl(char const *file, char *const argv[],
                                      char *const envp[], bool dosmode)
#endif
{
 char *iter,*part,*next,*path;
 bool use_malloc; size_t path_size;
 if unlikely((path = libc_getenv("PATH")) == NULL) { SET_ERRNO(ENOENT); return -1; }
 path_size = (libc_strlen(path)+1)*sizeof(char);
 use_malloc = path_size > __AMALLOC_MAX;
 if (use_malloc) {
  path = (char *)libc_memdup(path,path_size);
  if unlikely(!path) return -1;
 } else {
  char *path_copy;
  path_copy = (char *)alloca(path_size);
  libc_memcpy(path_copy,path,path_size);
  path = path_copy;
 }
 part = path;
 for (;;) {
  bool last = *(next = libc_strchrnul(part,':')) != '\0';
  for (iter = next; iter[-1] == '/'; --iter);
  *iter = '\0';
#ifdef CONFIG_LIBC_NO_DOS_LIBC
  execvpe_inside(part,file,argv,envp);
#else
  execvpe_inside(part,file,argv,envp,dosmode);
#endif
  if (last) break;
  part = next+1;
 }
 if (use_malloc) libc_free(path);
 return -1;
}
INTERN int LIBCCALL libc_execv(char const *path, char *const argv[]) { return libc_execve(path,argv,environ); }
INTERN int LIBCCALL libc_execvp(char const *file, char *const argv[]) { return libc_execvpe(file,argv,environ); }
INTERN int LIBCCALL libc_link(char const *from, char const *to) { return libc_linkat(AT_FDCWD,from,AT_FDCWD,to,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_symlink(char const *from, char const *to) { return libc_symlinkat(from,AT_FDCWD,to); }
INTERN ssize_t LIBCCALL libc_readlink(char const *__restrict path, char *__restrict buf, size_t len) { return libc_readlinkat(AT_FDCWD,path,buf,len); }
INTERN int LIBCCALL libc_unlink(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_SYMLINK_NOFOLLOW|AT_REMOVEREG); }
INTERN int LIBCCALL libc_rmdir(char const *path) { return libc_unlinkat(AT_FDCWD,path,AT_SYMLINK_FOLLOW|AT_REMOVEDIR); }
INTERN int LIBCCALL libc_mkdir(char const *path, mode_t mode) { return libc_mkdirat(AT_FDCWD,path,mode); }
INTERN int LIBCCALL libc_mkfifo(char const *path, mode_t mode) { return libc_mkfifoat(AT_FDCWD,path,mode); }
INTERN int LIBCCALL libc_mknod(char const *path, mode_t mode, dev_t dev) { return libc_mknodat(AT_FDCWD,path,mode,dev); }
INTERN pid_t LIBCCALL libc_wait(__WAIT_STATUS stat_loc) { return libc_wait4(-1,stat_loc,0,NULL); }
INTERN pid_t LIBCCALL libc_waitpid(pid_t pid, __WAIT_STATUS stat_loc, int options) { return libc_wait4(pid,stat_loc,options,NULL); }
INTERN pid_t LIBCCALL libc_wait3(__WAIT_STATUS stat_loc, int options, struct rusage *usage) { return libc_wait4(-1,stat_loc,options,usage); }
INTERN int LIBCCALL libc_waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options) { return FORWARD_SYSTEM_ERROR(sys_waitid(idtype,id,infop,options,NULL)); }
INTERN pid_t LIBCCALL libc_wait4(pid_t pid, __WAIT_STATUS stat_loc, int options, struct rusage *usage) { return FORWARD_SYSTEM_VALUE(sys_wait4(pid,stat_loc,options,usage)); }
INTERN int LIBCCALL libc_getgroups(int size, gid_t list[]) { NOT_IMPLEMENTED(); return -1; }
INTERN ATTR_COLDTEXT int LIBCCALL libc_setuid(uid_t uid) { NOT_IMPLEMENTED(); return -1; }
INTERN ATTR_COLDTEXT int LIBCCALL libc_setgid(gid_t gid) { NOT_IMPLEMENTED(); return -1; }
INTERN ATTR_COLDTEXT int LIBCCALL libc_umount(char const *special_file) { return libc_umount2(special_file,0); }
INTERN ATTR_COLDTEXT int LIBCCALL libc_mount(char const *special_file, char const *dir, char const *fstype, unsigned long int rwflag, void const *data) { return FORWARD_SYSTEM_ERROR(sys_mount(special_file,dir,fstype,rwflag,data)); }
INTERN ATTR_COLDTEXT int LIBCCALL libc_umount2(char const *special_file, int flags) { return FORWARD_SYSTEM_ERROR(sys_umount2(special_file,flags)); }
INTERN ATTR_COLDTEXT int LIBCCALL libc_swapon(char const *path, int flags) { return FORWARD_SYSTEM_ERROR(sys_swapon(path,flags)); }
INTERN ATTR_COLDTEXT int LIBCCALL libc_swapoff(char const *path) { return FORWARD_SYSTEM_ERROR(sys_swapoff(path)); }
INTERN int LIBCCALL libc_pipe(int pipedes[2]) { return libc_pipe2(pipedes,0); }
INTERN int LIBCCALL libc_pipe2(int pipedes[2], int flags) {
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
  SET_ERRNO(-pfd.error);
  return -1;
 }
 pipedes[0] = pfd.fd_reader;
 pipedes[1] = pfd.fd_writer;
 return 0;
#endif
}
INTERN mode_t LIBCCALL libc_umask(mode_t mask) {
 return sys_umask(mask);
}
INTERN mode_t LIBCCALL libc_getumask(void) {
 mode_t result = libc_umask(0);
 return (libc_umask(result),result);
}
INTERN pid_t LIBCCALL libc_getpid(void) { return sys_getpid(); }
INTERN pid_t LIBCCALL libc_getppid(void) { return sys_getppid(); }
INTERN pid_t LIBCCALL libc_getpgid(pid_t pid) { return FORWARD_SYSTEM_VALUE(sys_getpgid(pid)); }
INTERN pid_t LIBCCALL libc_getpgrp(void) { return libc_getpgid(0); }
INTERN int LIBCCALL libc_setpgid(pid_t pid, pid_t pgid) { return FORWARD_SYSTEM_ERROR(sys_setpgid(pid,pgid)); }

INTERN pid_t LIBCCALL libc_getsid(pid_t pid) { NOT_IMPLEMENTED(); return -1; }
INTERN pid_t LIBCCALL libc_setsid(void) { NOT_IMPLEMENTED(); return -1; }
INTERN uid_t LIBCCALL libc_getuid(void) { NOT_IMPLEMENTED(); return -1; }
INTERN gid_t LIBCCALL libc_getgid(void) { NOT_IMPLEMENTED(); return -1; }
INTERN uid_t LIBCCALL libc_geteuid(void) { NOT_IMPLEMENTED(); return -1; }
INTERN gid_t LIBCCALL libc_getegid(void) { NOT_IMPLEMENTED(); return -1; }

INTERN long int LIBCCALL libc_pathconf(char const *path, int name) { NOT_IMPLEMENTED(); return -1; }
INTERN long int LIBCCALL libc_fpathconf(int fd, int name) { NOT_IMPLEMENTED(); return -1; }
INTERN char *LIBCCALL libc_getlogin(void) { NOT_IMPLEMENTED(); return NULL; }

INTERN int LIBCCALL libc_group_member(gid_t gid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getresuid(uid_t *ruid, uid_t *euid, uid_t *suid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setresuid(uid_t ruid, uid_t euid, uid_t suid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setresgid(gid_t rgid, gid_t egid, gid_t sgid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setreuid(uid_t ruid, uid_t euid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setregid(gid_t rgid, gid_t egid) { NOT_IMPLEMENTED(); return -1; }

INTERN pid_t LIBCCALL libc_vfork(void) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_nice(int inc) { NOT_IMPLEMENTED(); return -1; }
INTERN size_t LIBCCALL libc_confstr(int name, char *buf, size_t len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setpgrp(void) { NOT_IMPLEMENTED(); return -1; }
INTERN long int LIBCCALL libc_gethostid(void) { NOT_IMPLEMENTED(); return -1; }
#ifdef PAGESIZE
INTERN int LIBCCALL libc_getpagesize(void) { return PAGESIZE; }
#elif defined(_SC_PAGE_SIZE)
INTERN int LIBCCALL libc_getpagesize(void) { return libc_sysconf(_SC_PAGE_SIZE); }
#else
INTERN int LIBCCALL libc_getpagesize(void) { return libc_sysconf(_SC_PAGESIZE); }
#endif
INTERN int LIBCCALL libc_getdtablesize(void) { return libc_sysconf(_SC_OPEN_MAX); }
INTERN int LIBCCALL libc_seteuid(uid_t uid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setegid(gid_t gid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_ttyslot(void) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getlogin_r(char *name, size_t name_len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_gethostname(char *name, size_t len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setlogin(char const *name) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sethostname(char const *name, size_t len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sethostid(long int id) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getdomainname(char *name, size_t len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setdomainname(char const *name, size_t len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_vhangup(void) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_revoke(char const *file) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_profil(unsigned short int *sample_buffer, size_t size, size_t offset, unsigned int scale) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_acct(char const *name) { NOT_IMPLEMENTED(); return -1; }
INTERN char *LIBCCALL libc_getusershell(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN void LIBCCALL libc_endusershell(void) { NOT_IMPLEMENTED(); }
INTERN void LIBCCALL libc_setusershell(void) { NOT_IMPLEMENTED(); }
INTERN int LIBCCALL libc_daemon(int nochdir, int noclose) { NOT_IMPLEMENTED(); return -1; }
INTERN char *LIBCCALL libc_getpass(char const *prompt) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_crypt(char const *key, char const *salt) { NOT_IMPLEMENTED(); return NULL; }
INTERN void LIBCCALL libc_encrypt(char *glibc_block, int edflag) { NOT_IMPLEMENTED(); }
INTERN void LIBCCALL libc_swab(void const *__restrict from, void *__restrict to, ssize_t n_bytes) { NOT_IMPLEMENTED(); }
INTERN char *LIBCCALL libc_ctermid(char *s) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_cuserid(char *s) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_lockf(int fd, int cmd, off32_t len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_lockf64(int fd, int cmd, off64_t len) { NOT_IMPLEMENTED(); return -1; }
#if 0
PRIVATE struct passwd test = {
   .pw_name   = "root",
   //.pw_passwd = "",
   .pw_uid    = 0,
   .pw_gid    = 0,
   //.pw_gecos  = "",
   .pw_dir    = "/",
   .pw_shell  = "/bin/sh",
};
INTERN struct passwd *LIBCCALL libc_getpwuid(uid_t uid) { NOT_IMPLEMENTED(); return &test; }
INTERN struct passwd *LIBCCALL libc_getpwnam(char const *name) { NOT_IMPLEMENTED(); return &test; }
#else
INTERN struct passwd *LIBCCALL libc_getpwuid(uid_t uid) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct passwd *LIBCCALL libc_getpwnam(char const *name) { NOT_IMPLEMENTED(); return NULL; }
#endif
INTERN void LIBCCALL libc_setpwent(void) { NOT_IMPLEMENTED(); }
INTERN void LIBCCALL libc_endpwent(void) { NOT_IMPLEMENTED(); }
INTERN struct passwd *LIBCCALL libc_getpwent(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct passwd *LIBCCALL libc_fgetpwent(FILE *stream) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_putpwent(struct passwd const *__restrict p, FILE *__restrict f) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getpwuid_r(uid_t uid, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getpwnam_r(char const *__restrict name, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getpwent_r(struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_fgetpwent_r(FILE *__restrict stream, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getpw(uid_t uid, char *buffer) { NOT_IMPLEMENTED(); return -1; }
INTERN struct group *LIBCCALL libc_getgrgid(gid_t gid) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct group *LIBCCALL libc_getgrnam(char const *name) { NOT_IMPLEMENTED(); return NULL; }
INTERN void LIBCCALL libc_setgrent(void) { NOT_IMPLEMENTED(); }
INTERN void LIBCCALL libc_endgrent(void) { NOT_IMPLEMENTED(); }
INTERN struct group *LIBCCALL libc_getgrent(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_putgrent(struct group const *__restrict p, FILE *__restrict f) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getgrgid_r(gid_t gid, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getgrnam_r(char const *__restrict name, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getgrent_r(struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_fgetgrent_r(FILE *__restrict stream, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { NOT_IMPLEMENTED(); return -1; }
INTERN struct group *LIBCCALL libc_fgetgrent(FILE *stream) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_setgroups(size_t __n, const gid_t *groups) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getgrouplist(char const *user, gid_t group, gid_t *groups, int *ngroups) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_initgroups(char const *user, gid_t group) { NOT_IMPLEMENTED(); return -1; }
INTERN FILE *LIBCCALL libc_setmntent(char const *file, char const *mode) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct mntent *LIBCCALL libc_getmntent(FILE *stream) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct mntent *LIBCCALL libc_getmntent_r(FILE *__restrict stream, struct mntent *__restrict result, char *__restrict buffer, int bufsize) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_addmntent(FILE *__restrict stream, struct mntent const *__restrict mnt) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_endmntent(FILE *stream) { NOT_IMPLEMENTED(); return -1; }
INTERN char *LIBCCALL libc_hasmntopt(struct mntent const *mnt, char const *opt) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_fnmatch(char const *pattern, char const *name, int flags) { return FNM_NOSYS; } /* TODO */
INTERN int LIBCCALL libc_getrlimit(__rlimit_resource_t resource, struct rlimit *rlimits) { return libc_prlimit(libc_getpid(),(enum __rlimit_resource)resource,NULL,rlimits); }
INTERN int LIBCCALL libc_setrlimit(__rlimit_resource_t resource, struct rlimit const *rlimits) { return libc_prlimit(libc_getpid(),(enum __rlimit_resource)resource,rlimits,NULL); }
INTERN int LIBCCALL libc_getrlimit64(__rlimit_resource_t resource, struct rlimit64 *rlimits) { return libc_prlimit64(libc_getpid(),(enum __rlimit_resource)resource,NULL,rlimits); }
INTERN int LIBCCALL libc_setrlimit64(__rlimit_resource_t resource, struct rlimit64 const *rlimits) { return libc_prlimit64(libc_getpid(),(enum __rlimit_resource)resource,rlimits,NULL); }
INTERN int LIBCCALL libc_getpriority(__priority_which_t which, id_t who) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setpriority(__priority_which_t which, id_t who, int prio) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_prlimit(pid_t pid, enum __rlimit_resource resource, struct rlimit const *new_limit, struct rlimit *old_limit) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_prlimit64(pid_t pid, enum __rlimit_resource resource, struct rlimit64 const *new_limit, struct rlimit64 *old_limit) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getrusage(__rusage_who_t who, struct rusage *usage) { libc_memset(usage,0,sizeof(struct rusage)); return 0; } /* xxx? */


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

INTERN int LIBCCALL libc_glibc_fstat(int fd, struct glibc_stat *statbuf) {
 struct stat64 buf; int result;
 result = libc_kfstat64(fd,&buf);
 if (!result) glibc_load_stat_buffer(statbuf,&buf);
 return result;
}
INTERN int LIBCCALL libc_glibc_fstatat(int fd, char const *filename, struct glibc_stat *statbuf, int flags) {
 struct stat64 buf; int result;
 result = libc_kfstatat64(fd,filename,&buf,flags);
 if (!result) glibc_load_stat_buffer(statbuf,&buf);
 return result;
}
INTERN int LIBCCALL libc_glibc_stat(char const *path, struct glibc_stat *statbuf) { return libc_glibc_fstatat(AT_FDCWD,path,statbuf,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_glibc_lstat(char const *path, struct glibc_stat *statbuf) { return libc_glibc_fstatat(AT_FDCWD,path,statbuf,AT_SYMLINK_NOFOLLOW); }
#define VERCHK { if (ver != 0) { SET_ERRNO(-EINVAL); return -1; } }
INTERN int LIBCCALL libc___fxstat(int ver, int fd, struct glibc_stat *statbuf) { VERCHK return libc_glibc_fstat(fd,statbuf); }
INTERN int LIBCCALL libc___xstat(int ver, char const *filename, struct glibc_stat *statbuf) { VERCHK return libc_glibc_stat(filename,statbuf); }
INTERN int LIBCCALL libc___lxstat(int ver, char const *filename, struct glibc_stat *statbuf) { VERCHK return libc_glibc_lstat(filename,statbuf); }
INTERN int LIBCCALL libc___fxstatat(int ver, int fd, char const *filename, struct glibc_stat *statbuf, int flags) { VERCHK return libc_glibc_fstatat(fd,filename,statbuf,flags); }
#undef VERCHK




PRIVATE struct utsname const *kernel_utsname = NULL;
INTERN int LIBCCALL libc_uname(struct utsname *name) {
 struct utsname const *data;
 /* Lazily load uname information from the kernel. */
 if ((data = kernel_utsname) == NULL) {
  data = (struct utsname const *)sys_xsharesym("uname");
  if unlikely(!data) { SET_ERRNO(ENOSYS); return -1; }
  ATOMIC_CMPXCH(kernel_utsname,NULL,data);
 }
 libc_memcpy(name,data,sizeof(struct utsname));
 return 0;
}


extern byte_t _end[]; /*< Automatically defined by the linker (end of '.bss'). */
PRIVATE byte_t *brk_curr = NULL;
PRIVATE DEFINE_ATOMIC_RWLOCK(brk_lock);

PRIVATE int LIBCCALL do_brk(void *addr) {
 byte_t *real_oldbrk,*real_newbrk,*oldbrk;
 oldbrk = brk_curr;
 if (!oldbrk) oldbrk = _end;
 real_oldbrk = (byte_t *)FLOOR_ALIGN((uintptr_t)oldbrk,4096);
 real_newbrk = (byte_t *)CEIL_ALIGN((uintptr_t)addr,4096);
 if (real_newbrk < real_oldbrk) {
  /* Release memory */
  if unlikely(libc_munmap(real_newbrk,real_oldbrk-real_newbrk) == -1)
     return -1;
 } else if (real_newbrk > real_oldbrk) {
  void *map_result;
  /* Allocate more memory */
  map_result = libc_mmap(real_oldbrk,real_newbrk-real_oldbrk,
                         PROT_READ|PROT_WRITE,MAP_FIXED|MAP_ANONYMOUS,-1,0);
  if unlikely(map_result == MAP_FAILED) return -1;
  assertf(map_result == real_oldbrk,"%p != %p",map_result,real_oldbrk);
 }
 brk_curr = (byte_t *)addr;
 return 0;
}

INTERN int LIBCCALL libc_brk(void *addr) {
 int result;
 atomic_rwlock_write(&brk_lock);
 result = do_brk(addr);
 atomic_rwlock_endwrite(&brk_lock);
 return result;
}

INTERN void *LIBCCALL libc_sbrk(intptr_t increment) {
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

         strptime
         strsep
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
         getuid
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

DEFINE_PUBLIC_ALIAS(syscall,libc_syscall);
DEFINE_PUBLIC_ALIAS(gnu_dev_major,libc_gnu_dev_major);
DEFINE_PUBLIC_ALIAS(gnu_dev_minor,libc_gnu_dev_minor);
DEFINE_PUBLIC_ALIAS(gnu_dev_makedev,libc_gnu_dev_makedev);
DEFINE_PUBLIC_ALIAS(lseek,libc_lseek);
DEFINE_PUBLIC_ALIAS(read,libc_read);
DEFINE_PUBLIC_ALIAS(write,libc_write);
DEFINE_PUBLIC_ALIAS(lseek64,libc_lseek64);
DEFINE_PUBLIC_ALIAS(pread,libc_pread);
DEFINE_PUBLIC_ALIAS(pwrite,libc_pwrite);
DEFINE_PUBLIC_ALIAS(pread64,libc_pread64);
DEFINE_PUBLIC_ALIAS(pwrite64,libc_pwrite64);
DEFINE_PUBLIC_ALIAS(truncate,libc_truncate);
DEFINE_PUBLIC_ALIAS(truncate64,libc_truncate64);
DEFINE_PUBLIC_ALIAS(ftruncate,libc_ftruncate);
DEFINE_PUBLIC_ALIAS(ftruncate64,libc_ftruncate64);
DEFINE_PUBLIC_ALIAS(fsync,libc_fsync);
DEFINE_PUBLIC_ALIAS(syncfs,libc_syncfs);
DEFINE_PUBLIC_ALIAS(sync,libc_sync);
DEFINE_PUBLIC_ALIAS(fdatasync,libc_fdatasync);
DEFINE_PUBLIC_ALIAS(close,libc_close);
DEFINE_PUBLIC_ALIAS(chroot,libc_chroot);
DEFINE_PUBLIC_ALIAS(chdir,libc_chdir);
DEFINE_PUBLIC_ALIAS(fchdir,libc_fchdir);
DEFINE_PUBLIC_ALIAS(dup,libc_dup);
DEFINE_PUBLIC_ALIAS(dup3,libc_dup3);
DEFINE_PUBLIC_ALIAS(dup2,libc_dup2);
DEFINE_PUBLIC_ALIAS(kfstat64,libc_kfstat64);
DEFINE_PUBLIC_ALIAS(kfstatat64,libc_kfstatat64);
DEFINE_PUBLIC_ALIAS(kstat64,libc_kstat64);
DEFINE_PUBLIC_ALIAS(klstat64,libc_klstat64);
DEFINE_PUBLIC_ALIAS(access,libc_access);
DEFINE_PUBLIC_ALIAS(eaccess,libc_eaccess);
DEFINE_PUBLIC_ALIAS(euidaccess,libc_eaccess);
DEFINE_PUBLIC_ALIAS(faccessat,libc_faccessat);
DEFINE_PUBLIC_ALIAS(chown,libc_chown);
DEFINE_PUBLIC_ALIAS(lchown,libc_lchown);
DEFINE_PUBLIC_ALIAS(fchown,libc_fchown);
DEFINE_PUBLIC_ALIAS(chmod,libc_chmod);
DEFINE_PUBLIC_ALIAS(lchmod,libc_lchmod);
DEFINE_PUBLIC_ALIAS(fchmod,libc_fchmod);
DEFINE_PUBLIC_ALIAS(fchownat,libc_fchownat);
DEFINE_PUBLIC_ALIAS(fchmodat,libc_fchmodat);
DEFINE_PUBLIC_ALIAS(mknodat,libc_mknodat);
DEFINE_PUBLIC_ALIAS(mkdirat,libc_mkdirat);
DEFINE_PUBLIC_ALIAS(mkfifoat,libc_mkfifoat);
DEFINE_PUBLIC_ALIAS(linkat,libc_linkat);
DEFINE_PUBLIC_ALIAS(symlinkat,libc_symlinkat);
DEFINE_PUBLIC_ALIAS(unlinkat,libc_unlinkat);
DEFINE_PUBLIC_ALIAS(renameat,libc_renameat);
DEFINE_PUBLIC_ALIAS(frenameat,libc_frenameat);
DEFINE_PUBLIC_ALIAS(readlinkat,libc_readlinkat);
DEFINE_PUBLIC_ALIAS(removeat,libc_removeat);
DEFINE_PUBLIC_ALIAS(remove,libc_remove);
DEFINE_PUBLIC_ALIAS(rename,libc_rename);
DEFINE_PUBLIC_ALIAS(fork,libc_fork);
DEFINE_PUBLIC_ALIAS(execve,libc_execve);
DEFINE_PUBLIC_ALIAS(fexecve,libc_fexecve);
DEFINE_PUBLIC_ALIAS(execl,libc_execl);
DEFINE_PUBLIC_ALIAS(execle,libc_execle);
DEFINE_PUBLIC_ALIAS(execlp,libc_execlp);
DEFINE_PUBLIC_ALIAS(execlpe,libc_execlpe);
DEFINE_PUBLIC_ALIAS(fexecl,libc_fexecl);
DEFINE_PUBLIC_ALIAS(fexecle,libc_fexecle);
DEFINE_PUBLIC_ALIAS(fexecv,libc_fexecv);
DEFINE_PUBLIC_ALIAS(execvpe,libc_execvpe);
DEFINE_PUBLIC_ALIAS(execv,libc_execv);
DEFINE_PUBLIC_ALIAS(execvp,libc_execvp);
DEFINE_PUBLIC_ALIAS(link,libc_link);
DEFINE_PUBLIC_ALIAS(symlink,libc_symlink);
DEFINE_PUBLIC_ALIAS(readlink,libc_readlink);
DEFINE_PUBLIC_ALIAS(unlink,libc_unlink);
DEFINE_PUBLIC_ALIAS(rmdir,libc_rmdir);
DEFINE_PUBLIC_ALIAS(mkdir,libc_mkdir);
DEFINE_PUBLIC_ALIAS(mkfifo,libc_mkfifo);
DEFINE_PUBLIC_ALIAS(mknod,libc_mknod);
DEFINE_PUBLIC_ALIAS(wait,libc_wait);
DEFINE_PUBLIC_ALIAS(waitpid,libc_waitpid);
DEFINE_PUBLIC_ALIAS(wait3,libc_wait3);
DEFINE_PUBLIC_ALIAS(waitid,libc_waitid);
DEFINE_PUBLIC_ALIAS(wait4,libc_wait4);
DEFINE_PUBLIC_ALIAS(getgroups,libc_getgroups);
DEFINE_PUBLIC_ALIAS(setuid,libc_setuid);
DEFINE_PUBLIC_ALIAS(setgid,libc_setgid);
DEFINE_PUBLIC_ALIAS(umount,libc_umount);
DEFINE_PUBLIC_ALIAS(mount,libc_mount);
DEFINE_PUBLIC_ALIAS(umount2,libc_umount2);
DEFINE_PUBLIC_ALIAS(swapon,libc_swapon);
DEFINE_PUBLIC_ALIAS(swapoff,libc_swapoff);
DEFINE_PUBLIC_ALIAS(pipe,libc_pipe);
DEFINE_PUBLIC_ALIAS(pipe2,libc_pipe2);
DEFINE_PUBLIC_ALIAS(umask,libc_umask);
DEFINE_PUBLIC_ALIAS(getumask,libc_getumask);
DEFINE_PUBLIC_ALIAS(getpid,libc_getpid);
DEFINE_PUBLIC_ALIAS(getppid,libc_getppid);
DEFINE_PUBLIC_ALIAS(getpgid,libc_getpgid);
DEFINE_PUBLIC_ALIAS(getpgrp,libc_getpgrp);
DEFINE_PUBLIC_ALIAS(setpgid,libc_setpgid);
DEFINE_PUBLIC_ALIAS(getsid,libc_getsid);
DEFINE_PUBLIC_ALIAS(setsid,libc_setsid);
DEFINE_PUBLIC_ALIAS(getuid,libc_getuid);
DEFINE_PUBLIC_ALIAS(getgid,libc_getgid);
DEFINE_PUBLIC_ALIAS(geteuid,libc_geteuid);
DEFINE_PUBLIC_ALIAS(getegid,libc_getegid);
DEFINE_PUBLIC_ALIAS(pathconf,libc_pathconf);
DEFINE_PUBLIC_ALIAS(fpathconf,libc_fpathconf);
DEFINE_PUBLIC_ALIAS(getlogin,libc_getlogin);
DEFINE_PUBLIC_ALIAS(group_member,libc_group_member);
DEFINE_PUBLIC_ALIAS(getresuid,libc_getresuid);
DEFINE_PUBLIC_ALIAS(getresgid,libc_getresgid);
DEFINE_PUBLIC_ALIAS(setresuid,libc_setresuid);
DEFINE_PUBLIC_ALIAS(setresgid,libc_setresgid);
DEFINE_PUBLIC_ALIAS(setreuid,libc_setreuid);
DEFINE_PUBLIC_ALIAS(setregid,libc_setregid);
DEFINE_PUBLIC_ALIAS(vfork,libc_vfork);
DEFINE_PUBLIC_ALIAS(nice,libc_nice);
DEFINE_PUBLIC_ALIAS(confstr,libc_confstr);
DEFINE_PUBLIC_ALIAS(setpgrp,libc_setpgrp);
DEFINE_PUBLIC_ALIAS(gethostid,libc_gethostid);
DEFINE_PUBLIC_ALIAS(getpagesize,libc_getpagesize);
DEFINE_PUBLIC_ALIAS(getdtablesize,libc_getdtablesize);
DEFINE_PUBLIC_ALIAS(seteuid,libc_seteuid);
DEFINE_PUBLIC_ALIAS(setegid,libc_setegid);
DEFINE_PUBLIC_ALIAS(ttyslot,libc_ttyslot);
DEFINE_PUBLIC_ALIAS(getlogin_r,libc_getlogin_r);
DEFINE_PUBLIC_ALIAS(gethostname,libc_gethostname);
DEFINE_PUBLIC_ALIAS(setlogin,libc_setlogin);
DEFINE_PUBLIC_ALIAS(sethostname,libc_sethostname);
DEFINE_PUBLIC_ALIAS(sethostid,libc_sethostid);
DEFINE_PUBLIC_ALIAS(getdomainname,libc_getdomainname);
DEFINE_PUBLIC_ALIAS(setdomainname,libc_setdomainname);
DEFINE_PUBLIC_ALIAS(vhangup,libc_vhangup);
DEFINE_PUBLIC_ALIAS(revoke,libc_revoke);
DEFINE_PUBLIC_ALIAS(profil,libc_profil);
DEFINE_PUBLIC_ALIAS(acct,libc_acct);
DEFINE_PUBLIC_ALIAS(getusershell,libc_getusershell);
DEFINE_PUBLIC_ALIAS(endusershell,libc_endusershell);
DEFINE_PUBLIC_ALIAS(setusershell,libc_setusershell);
DEFINE_PUBLIC_ALIAS(daemon,libc_daemon);
DEFINE_PUBLIC_ALIAS(getpass,libc_getpass);
DEFINE_PUBLIC_ALIAS(crypt,libc_crypt);
DEFINE_PUBLIC_ALIAS(encrypt,libc_encrypt);
DEFINE_PUBLIC_ALIAS(swab,libc_swab);
DEFINE_PUBLIC_ALIAS(ctermid,libc_ctermid);
DEFINE_PUBLIC_ALIAS(cuserid,libc_cuserid);
DEFINE_PUBLIC_ALIAS(lockf,libc_lockf);
DEFINE_PUBLIC_ALIAS(lockf64,libc_lockf64);
DEFINE_PUBLIC_ALIAS(getpwuid,libc_getpwuid);
DEFINE_PUBLIC_ALIAS(getpwnam,libc_getpwnam);
DEFINE_PUBLIC_ALIAS(setpwent,libc_setpwent);
DEFINE_PUBLIC_ALIAS(endpwent,libc_endpwent);
DEFINE_PUBLIC_ALIAS(getpwent,libc_getpwent);
DEFINE_PUBLIC_ALIAS(fgetpwent,libc_fgetpwent);
DEFINE_PUBLIC_ALIAS(putpwent,libc_putpwent);
DEFINE_PUBLIC_ALIAS(getpwuid_r,libc_getpwuid_r);
DEFINE_PUBLIC_ALIAS(getpwnam_r,libc_getpwnam_r);
DEFINE_PUBLIC_ALIAS(getpwent_r,libc_getpwent_r);
DEFINE_PUBLIC_ALIAS(fgetpwent_r,libc_fgetpwent_r);
DEFINE_PUBLIC_ALIAS(getpw,libc_getpw);
DEFINE_PUBLIC_ALIAS(getgrgid,libc_getgrgid);
DEFINE_PUBLIC_ALIAS(getgrnam,libc_getgrnam);
DEFINE_PUBLIC_ALIAS(setgrent,libc_setgrent);
DEFINE_PUBLIC_ALIAS(endgrent,libc_endgrent);
DEFINE_PUBLIC_ALIAS(getgrent,libc_getgrent);
DEFINE_PUBLIC_ALIAS(putgrent,libc_putgrent);
DEFINE_PUBLIC_ALIAS(getgrgid_r,libc_getgrgid_r);
DEFINE_PUBLIC_ALIAS(getgrnam_r,libc_getgrnam_r);
DEFINE_PUBLIC_ALIAS(getgrent_r,libc_getgrent_r);
DEFINE_PUBLIC_ALIAS(fgetgrent_r,libc_fgetgrent_r);
DEFINE_PUBLIC_ALIAS(fgetgrent,libc_fgetgrent);
DEFINE_PUBLIC_ALIAS(setgroups,libc_setgroups);
DEFINE_PUBLIC_ALIAS(getgrouplist,libc_getgrouplist);
DEFINE_PUBLIC_ALIAS(initgroups,libc_initgroups);
DEFINE_PUBLIC_ALIAS(setmntent,libc_setmntent);
DEFINE_PUBLIC_ALIAS(getmntent,libc_getmntent);
DEFINE_PUBLIC_ALIAS(getmntent_r,libc_getmntent_r);
DEFINE_PUBLIC_ALIAS(addmntent,libc_addmntent);
DEFINE_PUBLIC_ALIAS(endmntent,libc_endmntent);
DEFINE_PUBLIC_ALIAS(hasmntopt,libc_hasmntopt);
DEFINE_PUBLIC_ALIAS(fnmatch,libc_fnmatch);
DEFINE_PUBLIC_ALIAS(getrlimit,libc_getrlimit);
DEFINE_PUBLIC_ALIAS(setrlimit,libc_setrlimit);
DEFINE_PUBLIC_ALIAS(getrlimit64,libc_getrlimit64);
DEFINE_PUBLIC_ALIAS(setrlimit64,libc_setrlimit64);
DEFINE_PUBLIC_ALIAS(getpriority,libc_getpriority);
DEFINE_PUBLIC_ALIAS(setpriority,libc_setpriority);
DEFINE_PUBLIC_ALIAS(prlimit,libc_prlimit);
DEFINE_PUBLIC_ALIAS(prlimit64,libc_prlimit64);
DEFINE_PUBLIC_ALIAS(getrusage,libc_getrusage);
DEFINE_PUBLIC_ALIAS(__fxstat,libc___fxstat);
DEFINE_PUBLIC_ALIAS(__xstat,libc___xstat);
DEFINE_PUBLIC_ALIAS(__lxstat,libc___lxstat);
DEFINE_PUBLIC_ALIAS(__fxstatat,libc___fxstatat);
DEFINE_PUBLIC_ALIAS(uname,libc_uname);
DEFINE_PUBLIC_ALIAS(brk,libc_brk);
DEFINE_PUBLIC_ALIAS(sbrk,libc_sbrk);



DEFINE_PUBLIC_ALIAS(kstat,libc_kstat64);
DEFINE_PUBLIC_ALIAS(klstat,libc_klstat64);
DEFINE_PUBLIC_ALIAS(kfstat,libc_kfstat64);
DEFINE_PUBLIC_ALIAS(kfstatat,libc_kfstatat64);
DEFINE_PUBLIC_ALIAS(__getpgid,libc_getpgid);

/* GLIBC does the same trick where the 32-bit stat
 * buffer has binary compatibility with the 64-bit one! */
DEFINE_PUBLIC_ALIAS(__fxstat64,libc___fxstat);
DEFINE_PUBLIC_ALIAS(__xstat64,libc___xstat);
DEFINE_PUBLIC_ALIAS(__lxstat64,libc___lxstat);
DEFINE_PUBLIC_ALIAS(__fxstatat64,libc___fxstatat);
DEFINE_PUBLIC_ALIAS(stat,libc_glibc_stat);
DEFINE_PUBLIC_ALIAS(lstat,libc_glibc_lstat);
DEFINE_PUBLIC_ALIAS(fstat,libc_glibc_fstat);
DEFINE_PUBLIC_ALIAS(fstatat,libc_glibc_fstatat);
DEFINE_PUBLIC_ALIAS(stat64,libc_glibc_stat);
DEFINE_PUBLIC_ALIAS(lstat64,libc_glibc_lstat);
DEFINE_PUBLIC_ALIAS(fstat64,libc_glibc_fstat);
DEFINE_PUBLIC_ALIAS(fstatat64,libc_glibc_fstatat);

#ifndef CONFIG_LIBC_NO_DOS_LIBC

#ifdef CONFIG_32BIT_FILESYSTEM
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_truncate(char const *file, off32_t length) {
 int result = -1,fd = libc_dos_open(file,O_RDWR);
 if (fd > 0) result = libc_ftruncate(fd,length),sys_close(fd);
 return result;
}
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_truncate64(char const *file, off64_t length) {
 return libc_dos_truncate(file,(off32_t)length);
}
#else /* CONFIG_32BIT_FILESYSTEM */
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_truncate(char const *file, off32_t length) {
 return libc_dos_truncate64(file,(off64_t)length);
}
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_truncate64(char const *file, off64_t length) {
 int result = -1,fd = libc_dos_open(file,O_RDWR);
 if (fd > 0) result = libc_ftruncate64(fd,length),sys_close(fd);
 return result;
}
#endif /* !CONFIG_32BIT_FILESYSTEM */

INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_chroot(char const *path) {
 int result = -1,fd = libc_dos_open(path,O_RDONLY|O_DIRECTORY);
 if (fd > 0) result = libc_dup2(fd,AT_FDROOT),sys_close(fd);
 return result;
}
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_chdir(char const *path) { return libc_fchdirat(AT_FDCWD,path,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_fchdirat(int dfd, char const *path, int flags) { return libc_fchdirat(dfd,path,AT_DOSPATH|flags); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_kfstatat64(int fd, char const *__restrict file, struct stat64 *__restrict buf, int flags) { return libc_kfstatat64(fd,file,buf,AT_DOSPATH|flags); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_kstat64(char const *__restrict file, struct stat64 *__restrict buf) { return libc_dos_kfstatat64(AT_FDCWD,file,buf,AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_klstat64(char const *__restrict file, struct stat64 *__restrict buf) { return libc_dos_kfstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_access(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_eaccess(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,AT_DOSPATH|AT_EACCESS|AT_SYMLINK_NOFOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_faccessat(int fd, char const *file, int type, int flags) { return libc_faccessat(fd,file,type,AT_DOSPATH|flags); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_chown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_lchown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,AT_DOSPATH|AT_SYMLINK_NOFOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_fchownat(int fd, char const *file, uid_t owner, gid_t group, int flags) { return libc_fchownat(fd,file,owner,group,AT_DOSPATH|flags); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_chmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_lchmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,AT_DOSPATH|AT_SYMLINK_NOFOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_fchmodat(int fd, char const *file, mode_t mode, int flags) { return libc_fchmodat(fd,file,mode,AT_DOSPATH|flags); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_mknodat(int fd, char const *path, mode_t mode, dev_t dev) { return libc_mknodat(fd,path,O_DOSPATH|mode,dev); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_mkdirat(int fd, char const *path, mode_t mode) { return libc_mkdirat(fd,path,O_DOSPATH|mode); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_mkfifoat(int fd, char const *path, mode_t mode) { return libc_mkfifoat(fd,path,O_DOSPATH|mode); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_linkat(int fromfd, char const *from, int tofd, char const *to, int flags) { return libc_linkat(fromfd,from,tofd,to,AT_DOSPATH|flags); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_symlinkat(char const *from, int tofd, char const *to) { return FORWARD_SYSTEM_ERROR(sys_xsymlinkat(from,tofd,to,AT_DOSPATH)); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_unlinkat(int fd, char const *name, int flags) { return libc_unlinkat(fd,name,AT_DOSPATH|flags); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_renameat(int oldfd, char const *old, int newfd, char const *new_) { return FORWARD_SYSTEM_ERROR(sys_xrenameat(oldfd,old,newfd,new_,AT_DOSPATH|AT_SYMLINK_NOFOLLOW)); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_removeat(int fd, char const *filename) { return libc_unlinkat(fd,filename,AT_DOSPATH|AT_SYMLINK_NOFOLLOW|AT_REMOVEREG|AT_REMOVEDIR); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_remove(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_DOSPATH|AT_SYMLINK_NOFOLLOW|AT_REMOVEREG|AT_REMOVEDIR); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_rename(char const *old, char const *new_) { return libc_dos_renameat(AT_FDCWD,old,AT_FDCWD,new_); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_execv(char const *path, char *const argv[]) { return libc_dos_execve(path,argv,environ); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_execvp(char const *file, char *const argv[]) { return libc_dos_execvpe(file,argv,environ); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_link(char const *from, char const *to) { return libc_linkat(AT_FDCWD,from,AT_FDCWD,to,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_symlink(char const *from, char const *to) { return libc_dos_symlinkat(from,AT_FDCWD,to); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_unlink(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_DOSPATH|AT_SYMLINK_NOFOLLOW|AT_REMOVEREG); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_rmdir(char const *path) { return libc_unlinkat(AT_FDCWD,path,AT_DOSPATH|AT_SYMLINK_FOLLOW|AT_REMOVEDIR); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_mkdir(char const *path, mode_t mode) { return libc_mkdir(path,O_DOSPATH|mode); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_mkfifo(char const *path, mode_t mode) { return libc_mkfifo(path,O_DOSPATH|mode); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_mknod(char const *path, mode_t mode, dev_t dev) { return libc_mknod(path,O_DOSPATH|mode,dev); }
INTERN ATTR_DOSTEXT long int LIBCCALL libc_dos_pathconf(char const *path, int name) { NOT_IMPLEMENTED(); return -1; }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_revoke(char const *file) { NOT_IMPLEMENTED(); return -1; }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_acct(char const *name) { NOT_IMPLEMENTED(); return -1; }
INTERN ATTR_DOSTEXT FILE *LIBCCALL libc_dos_setmntent(char const *file, char const *mode) { NOT_IMPLEMENTED(); return NULL; }

INTERN ATTR_DOSTEXT int LIBCCALL
libc_ext_execve(char const *path, size_t pathlen,
                char const *ext, size_t extlen,
                char *const argv[], char *const envp[]) {
 char *fullpath; size_t fullsize;
 /* Concat the given path and extension. */
 fullsize = (pathlen+extlen+1)*sizeof(char);
 if (fullsize >= __AMALLOC_MAX) {
  fullpath = (char *)libc_malloc(fullsize);
  if unlikely(!fullpath) return -1;
 } else {
  fullpath = (char *)alloca(fullsize);
 }
 libc_memcpy(fullpath,path,pathlen*sizeof(char));
 libc_memcpy(fullpath+pathlen,ext,extlen*sizeof(char));
 fullpath[pathlen+extlen] = '\0';

 /* Try to execute the path. */
 libc_execve(fullpath,argv,envp);

 if (fullsize >= __AMALLOC_MAX)
     libc_free(fullpath);
 return -1;
}

INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_execve(char const *path, char *const argv[], char *const envp[]) {
 PRIVATE ATTR_DOSRODATA char const default_pathext[] = ".EXE;.CMD;.BAT;.COM";
 char *pathext,*temp,*ext,*next;
 size_t path_size1,path_size2;
 /* HINT: Remember that a successful call to execve() never returns. */
 libc_execve(path,argv,envp); /* Try the given path as-is. */
 /* Windows allows for various path-extensions, such as '.exe' or '.cmd'.
  * Those extensions are usually stored in an environment variable 'PATHEXT'. */
 pathext = libc_getenv(DOSSTR("PATHEXT"));
 if (!pathext) pathext = (char *)default_pathext;
 path_size2 = path_size1 = libc_strlen(path);
 next = (char *)libc_memrchr(path,'/',path_size2);
 temp = (char *)libc_memrchr(path,'\\',path_size2);
 if (temp > next) next = temp;
 temp = (char *)libc_memrchr(path,'.',path_size2);
 if (temp > next) next = temp;
 if (next) path_size1 = (size_t)(next-path); /* Override old extension. */

 for (ext = pathext;;) {
  bool last = *(next = libc_strchrnul(ext,':')) != '\0';
  size_t extlen = (size_t)(next-ext);
  /* Try when overriding the extension, and when appending it. */
  if (path_size1 != path_size2)
      libc_ext_execve(path,path_size1,ext,extlen,argv,envp);
  libc_ext_execve(path,path_size2,ext,extlen,argv,envp);
  if (last) break;
  ext = next+1;
 }
 return -1;
}


DEFINE_PUBLIC_ALIAS(__DSYM(truncate),libc_dos_truncate);
DEFINE_PUBLIC_ALIAS(__DSYM(truncate64),libc_dos_truncate64);
DEFINE_PUBLIC_ALIAS(__DSYM(chroot),libc_dos_chroot);
DEFINE_PUBLIC_ALIAS(__DSYM(chdir),libc_dos_chdir);
DEFINE_PUBLIC_ALIAS(__DSYM(fchdirat),libc_dos_fchdirat);
DEFINE_PUBLIC_ALIAS(__DSYM(kfstatat64),libc_dos_kfstatat64);
DEFINE_PUBLIC_ALIAS(__DSYM(kstat64),libc_dos_kstat64);
DEFINE_PUBLIC_ALIAS(__DSYM(klstat64),libc_dos_klstat64);
DEFINE_PUBLIC_ALIAS(__DSYM(access),libc_dos_access);
DEFINE_PUBLIC_ALIAS(__DSYM(eaccess),libc_dos_eaccess);
DEFINE_PUBLIC_ALIAS(__DSYM(faccessat),libc_dos_faccessat);
DEFINE_PUBLIC_ALIAS(__DSYM(chown),libc_dos_chown);
DEFINE_PUBLIC_ALIAS(__DSYM(lchown),libc_dos_lchown);
DEFINE_PUBLIC_ALIAS(__DSYM(fchownat),libc_dos_fchownat);
DEFINE_PUBLIC_ALIAS(__DSYM(chmod),libc_dos_chmod);
DEFINE_PUBLIC_ALIAS(__DSYM(lchmod),libc_dos_lchmod);
DEFINE_PUBLIC_ALIAS(__DSYM(fchmodat),libc_dos_fchmodat);
DEFINE_PUBLIC_ALIAS(__DSYM(mknodat),libc_dos_mknodat);
DEFINE_PUBLIC_ALIAS(__DSYM(mkdirat),libc_dos_mkdirat);
DEFINE_PUBLIC_ALIAS(__DSYM(mkfifoat),libc_dos_mkfifoat);
DEFINE_PUBLIC_ALIAS(__DSYM(linkat),libc_dos_linkat);
DEFINE_PUBLIC_ALIAS(__DSYM(symlinkat),libc_dos_symlinkat);
DEFINE_PUBLIC_ALIAS(__DSYM(unlinkat),libc_dos_unlinkat);
DEFINE_PUBLIC_ALIAS(__DSYM(renameat),libc_dos_renameat);
DEFINE_PUBLIC_ALIAS(__DSYM(removeat),libc_dos_removeat);
DEFINE_PUBLIC_ALIAS(__DSYM(remove),libc_dos_remove);
DEFINE_PUBLIC_ALIAS(__DSYM(rename),libc_dos_rename);
DEFINE_PUBLIC_ALIAS(__DSYM(execve),libc_dos_execve);
DEFINE_PUBLIC_ALIAS(__DSYM(execl),libc_dos_execl);
DEFINE_PUBLIC_ALIAS(__DSYM(execle),libc_dos_execle);
DEFINE_PUBLIC_ALIAS(__DSYM(execlp),libc_dos_execlp);
DEFINE_PUBLIC_ALIAS(__DSYM(execlpe),libc_dos_execlpe);
DEFINE_PUBLIC_ALIAS(__DSYM(execvpe),libc_dos_execvpe);
DEFINE_PUBLIC_ALIAS(__DSYM(execv),libc_dos_execv);
DEFINE_PUBLIC_ALIAS(__DSYM(execvp),libc_dos_execvp);
DEFINE_PUBLIC_ALIAS(__DSYM(link),libc_dos_link);
DEFINE_PUBLIC_ALIAS(__DSYM(symlink),libc_dos_symlink);
DEFINE_PUBLIC_ALIAS(__DSYM(unlink),libc_dos_unlink);
DEFINE_PUBLIC_ALIAS(__DSYM(rmdir),libc_dos_rmdir);
DEFINE_PUBLIC_ALIAS(__DSYM(mkdir),libc_dos_mkdir);
DEFINE_PUBLIC_ALIAS(__DSYM(mkfifo),libc_dos_mkfifo);
DEFINE_PUBLIC_ALIAS(__DSYM(mknod),libc_dos_mknod);
DEFINE_PUBLIC_ALIAS(__DSYM(pathconf),libc_dos_pathconf);
DEFINE_PUBLIC_ALIAS(__DSYM(revoke),libc_dos_revoke);
DEFINE_PUBLIC_ALIAS(__DSYM(acct),libc_dos_acct);
DEFINE_PUBLIC_ALIAS(__DSYM(setmntent),libc_dos_setmntent);


/* Unicode pathname support. */
#define WRAPPER(n,base) \
{ int result = -1; char *epath; \
  if ((epath = libc_utf##n##to8m(path,libc_##n##wcslen(path))) != NULL) { \
   result = base(epath); \
   libc_free(epath); \
  } \
  return result; \
}
#define libc_mkdir_temp(name)      libc_mkdir(name,mode)
#define libc_mkdir_temp2(name)     libc_mkdir(name,0755)
#define libc_dos_mkdir_temp(name)  libc_dos_mkdir(name,mode)
#define libc_dos_mkdir_temp2(name) libc_dos_mkdir(name,0755)
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_16wchdir(char16_t const *path) WRAPPER(16,libc_dos_chdir)
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_32wchdir(char32_t const *path) WRAPPER(32,libc_dos_chdir)
INTERN ATTR_DOSTEXT int LIBCCALL libc_16wchdir(char16_t const *path) WRAPPER(16,libc_chdir)
INTERN ATTR_DOSTEXT int LIBCCALL libc_32wchdir(char32_t const *path) WRAPPER(32,libc_chdir)
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_16wmkdir(char16_t const *path, mode_t mode) WRAPPER(16,libc_dos_mkdir_temp)
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_32wmkdir(char32_t const *path, mode_t mode) WRAPPER(32,libc_dos_mkdir_temp)
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_16wmkdir2(char16_t const *path) WRAPPER(16,libc_dos_mkdir_temp2)
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_32wmkdir2(char32_t const *path) WRAPPER(32,libc_dos_mkdir_temp2)
INTERN ATTR_DOSTEXT int LIBCCALL libc_16wmkdir(char16_t const *path, mode_t mode) WRAPPER(16,libc_mkdir_temp)
INTERN ATTR_DOSTEXT int LIBCCALL libc_32wmkdir(char32_t const *path, mode_t mode) WRAPPER(32,libc_mkdir_temp)
//INTERN ATTR_DOSTEXT int LIBCCALL libc_16wmkdir2(char16_t const *path) WRAPPER(16,libc_mkdir_temp2)
//INTERN ATTR_DOSTEXT int LIBCCALL libc_32wmkdir2(char32_t const *path) WRAPPER(32,libc_mkdir_temp2)
#undef libc_dos_mkdir_temp2
#undef libc_dos_mkdir_temp
#undef libc_mkdir_temp2
#undef libc_mkdir_temp
#undef WRAPPER

INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_mkdir2(char const *path) { return libc_dos_mkdir(path,0755); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_chdrive(int UNUSED(drive)) { return 0; }
INTERN ATTR_DOSTEXT int LIBCCALL libc_getdrive(void) { return 0; }
INTERN ATTR_DOSTEXT unsigned long LIBCCALL libc_getdrives(void) { return 1; }
INTERN ATTR_DOSTEXT unsigned LIBCCALL libc_getdiskfree(unsigned drive, struct _diskfree_t *diskfree) {
 /* Fill in something reasonable. (KOS doesn't track this stuff right now) */
 diskfree->total_clusters      = 64*1024;
 diskfree->avail_clusters      = 48*1024;
 diskfree->sectors_per_cluster = 4;
 diskfree->bytes_per_sector    = 512;
 return 0;
}

INTERN ATTR_DOSTEXT int LIBCCALL libc_eof(int fd) {
 off64_t oldpos,endpos;
 /* TODO: Add a system-call for this, that is can assure atomicity. */
 if ((oldpos = libc_lseek64(fd,0,SEEK_CUR)) < 0) return -1;
 if ((endpos = libc_lseek64(fd,0,SEEK_END)) < 0) return -1;
 libc_lseek64(fd,oldpos,SEEK_SET);
 return oldpos == endpos;
}

DEFINE_INTERN_ALIAS(libc_fsize,libc_fsize64);
INTERN ATTR_DOSTEXT off64_t LIBCCALL libc_fsize64(int fd) {
 off64_t oldpos,endpos;
 /* TODO: Add a system-call for this, that is can assure atomicity. */
 if ((oldpos = libc_lseek64(fd,0,SEEK_CUR)) < 0) return -1;
 if ((endpos = libc_lseek64(fd,0,SEEK_END)) < 0) return -1;
 libc_lseek64(fd,oldpos,SEEK_SET);
 return endpos;
}
DEFINE_INTERN_ALIAS(libc_ftell,libc_ftell64);
INTERN ATTR_DOSTEXT off64_t LIBCCALL libc_ftell64(int fd) {
 return libc_lseek64(fd,0,SEEK_CUR);
}
INTERN ATTR_DOSTEXT int LIBCCALL
libc_dos_pipe(int pipedes[2], u32 pipesize, int textmode) {
 int result = libc_pipe2(pipedes,textmode);
 if (result > 0) libc_fcntl(result,F_SETPIPE_SZ,(size_t)pipesize);
 return result;
}
INTERN ATTR_DOSTEXT mode_t LIBCCALL libc_setmode(int fd, mode_t mode) {
 return libc_fcntl(fd,F_SETFL_XCH,mode);
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_umask_s(mode_t new_mode, mode_t *old_mode) {
 errno_t error = sys_umask(new_mode);
 if (E_ISERR(error)) return -error;
 if (old_mode) *old_mode = (mode_t)error;
 return -EOK;
}
INTERN ATTR_DOSTEXT int LIBCCALL libc__lock_fhandle(int UNUSED(fd)) { return 0; }
INTERN ATTR_DOSTEXT void LIBCCALL libc_unlock_fhandle(int UNUSED(fd)) { }
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_get_osfhandle(int fd) { return (intptr_t)fd; }
INTERN ATTR_DOSTEXT int LIBCCALL libc_open_osfhandle(intptr_t osfd, int flags) { return flags&O_CLOEXEC ? libc_fcntl((int)osfd,F_DUPFD_CLOEXEC) : libc_dup(flags); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_access_s(char const *file, int type) { return -sys_faccessat(AT_FDCWD,file,type,AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_dos_access_s(char const *file, int type) { return -sys_faccessat(AT_FDCWD,file,type,AT_DOSPATH|AT_SYMLINK_FOLLOW); }

INTERN ATTR_DOSTEXT int LIBCCALL
libc_16wfaccessat(int dfd, char16_t const *file, int type, int flags) {
 int result = -1; char *utf8file = libc_utf16to8m(file,libc_16wcslen(file));
 if (utf8file) result = libc_faccessat(dfd,utf8file,type,flags),libc_free(utf8file);
 return result;
}
INTERN ATTR_DOSTEXT int LIBCCALL
libc_32wfaccessat(int dfd, char32_t const *file, int type, int flags) {
 int result = -1; char *utf8file = libc_utf32to8m(file,libc_32wcslen(file));
 if (utf8file) result = libc_faccessat(dfd,utf8file,type,flags),libc_free(utf8file);
 return result;
}

INTERN ATTR_DOSTEXT int LIBCCALL libc_16waccess(char16_t const *file, int type) { return libc_16wfaccessat(AT_FDCWD,file,type,AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_32waccess(char32_t const *file, int type) { return libc_32wfaccessat(AT_FDCWD,file,type,AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_16waccess(char16_t const *file, int type) { return libc_16wfaccessat(AT_FDCWD,file,type,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_32waccess(char32_t const *file, int type) { return libc_32wfaccessat(AT_FDCWD,file,type,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT __errno_t LIBCCALL libc_16waccess_s(char16_t const *file, int type) { return libc_16waccess(file,type) ? GET_ERRNO() : EOK; }
INTERN ATTR_DOSTEXT __errno_t LIBCCALL libc_32waccess_s(char32_t const *file, int type) { return libc_32waccess(file,type) ? GET_ERRNO() : EOK; }
INTERN ATTR_DOSTEXT __errno_t LIBCCALL libc_dos_16waccess_s(char16_t const *file, int type) { return libc_dos_16waccess(file,type) ? GET_ERRNO() : EOK; }
INTERN ATTR_DOSTEXT __errno_t LIBCCALL libc_dos_32waccess_s(char32_t const *file, int type) { return libc_dos_32waccess(file,type) ? GET_ERRNO() : EOK; }

INTERN ATTR_DOSTEXT int LIBCCALL libc_16wfchmodat(int dfd, char16_t const *file, mode_t mode, int flags) {
 int result = -1; char *utf8file = libc_utf16to8m(file,libc_16wcslen(file));
 if (utf8file) result = libc_fchmodat(dfd,utf8file,mode,flags),libc_free(utf8file);
 return result;
}
INTERN ATTR_DOSTEXT int LIBCCALL libc_32wfchmodat(int dfd, char32_t const *file, mode_t mode, int flags) {
 int result = -1; char *utf8file = libc_utf32to8m(file,libc_32wcslen(file));
 if (utf8file) result = libc_fchmodat(dfd,utf8file,mode,flags),libc_free(utf8file);
 return result;
}
INTERN ATTR_DOSTEXT int LIBCCALL libc_16wunlinkat(int dfd, char16_t const *file, int flags) {
 int result = -1; char *utf8file = libc_utf16to8m(file,libc_16wcslen(file));
 if (utf8file) result = libc_unlinkat(dfd,utf8file,flags),libc_free(utf8file);
 return result;
}
INTERN ATTR_DOSTEXT int LIBCCALL libc_32wunlinkat(int dfd, char32_t const *file, int flags) {
 int result = -1; char *utf8file = libc_utf32to8m(file,libc_32wcslen(file));
 if (utf8file) result = libc_unlinkat(dfd,utf8file,flags),libc_free(utf8file);
 return result;
}
INTERN ATTR_DOSTEXT int LIBCCALL
libc_16wfrenameat(int oldfd, char16_t const *oldname,
                  int newfd, char16_t const *newname, int flags) {
 int result = -1;
 char *utf8oldname = libc_utf16to8m(oldname,libc_16wcslen(oldname));
 char *utf8newname = libc_utf16to8m(newname,libc_16wcslen(newname));
 if (utf8oldname && utf8newname)
     result = libc_frenameat(oldfd,utf8oldname,newfd,utf8newname,flags);
 libc_free(utf8newname);
 libc_free(utf8oldname);
 return result;
}
INTERN ATTR_DOSTEXT int LIBCCALL
libc_32wfrenameat(int oldfd, char32_t const *oldname,
                  int newfd, char32_t const *newname, int flags) {
 int result = -1;
 char *utf8oldname = libc_utf32to8m(oldname,libc_32wcslen(oldname));
 char *utf8newname = libc_utf32to8m(newname,libc_32wcslen(newname));
 if (utf8oldname && utf8newname)
     result = libc_frenameat(oldfd,utf8oldname,newfd,utf8newname,flags);
 libc_free(utf8newname);
 libc_free(utf8oldname);
 return result;
}

INTERN ATTR_DOSTEXT int LIBCCALL libc_16wchmod(char16_t const *file, mode_t mode) { return libc_16wfchmodat(AT_FDCWD,file,mode,AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_32wchmod(char32_t const *file, mode_t mode) { return libc_32wfchmodat(AT_FDCWD,file,mode,AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_16wchmod(char16_t const *file, mode_t mode) { return libc_16wfchmodat(AT_FDCWD,file,mode,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_32wchmod(char32_t const *file, mode_t mode) { return libc_32wfchmodat(AT_FDCWD,file,mode,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_16wunlink(char16_t const *file) { return libc_16wunlinkat(AT_FDCWD,file,AT_SYMLINK_NOFOLLOW|AT_REMOVEREG); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_32wunlink(char32_t const *file) { return libc_32wunlinkat(AT_FDCWD,file,AT_SYMLINK_NOFOLLOW|AT_REMOVEREG); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_16wunlink(char16_t const *file) { return libc_16wunlinkat(AT_FDCWD,file,AT_DOSPATH|AT_SYMLINK_NOFOLLOW|AT_REMOVEREG); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_32wunlink(char32_t const *file) { return libc_32wunlinkat(AT_FDCWD,file,AT_DOSPATH|AT_SYMLINK_NOFOLLOW|AT_REMOVEREG); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_16wrmdir(char16_t const *path) { return libc_16wunlinkat(AT_FDCWD,path,AT_SYMLINK_NOFOLLOW|AT_REMOVEDIR); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_32wrmdir(char32_t const *path) { return libc_32wunlinkat(AT_FDCWD,path,AT_SYMLINK_NOFOLLOW|AT_REMOVEDIR); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_16wrmdir(char16_t const *path) { return libc_16wunlinkat(AT_FDCWD,path,AT_DOSPATH|AT_SYMLINK_NOFOLLOW|AT_REMOVEDIR); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_32wrmdir(char32_t const *path) { return libc_32wunlinkat(AT_FDCWD,path,AT_DOSPATH|AT_SYMLINK_NOFOLLOW|AT_REMOVEDIR); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_16wrename(char16_t const *oldname, char16_t const *newname) { return libc_16wfrenameat(AT_FDCWD,oldname,AT_FDCWD,newname,AT_SYMLINK_NOFOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_32wrename(char32_t const *oldname, char32_t const *newname) { return libc_32wfrenameat(AT_FDCWD,oldname,AT_FDCWD,newname,AT_SYMLINK_NOFOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_16wrename(char16_t const *oldname, char16_t const *newname) { return libc_16wfrenameat(AT_FDCWD,oldname,AT_FDCWD,newname,AT_DOSPATH|AT_SYMLINK_NOFOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_32wrename(char32_t const *oldname, char32_t const *newname) { return libc_32wfrenameat(AT_FDCWD,oldname,AT_FDCWD,newname,AT_DOSPATH|AT_SYMLINK_NOFOLLOW); }

DEFINE_PUBLIC_ALIAS(__KSYMw16(_waccess),libc_16waccess);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_waccess),libc_32waccess);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_waccess_s),libc_16waccess_s);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_waccess_s),libc_32waccess_s);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wchmod),libc_16wchmod);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wchmod),libc_32wchmod);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wunlink),libc_16wunlink);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wunlink),libc_32wunlink);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wrename),libc_16wrename);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wrename),libc_32wrename);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_waccess),libc_dos_16waccess);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_waccess),libc_dos_32waccess);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_waccess_s),libc_dos_16waccess_s);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_waccess_s),libc_dos_32waccess_s);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wchmod),libc_dos_16wchmod);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wchmod),libc_dos_32wchmod);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wunlink),libc_dos_16wunlink);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wunlink),libc_dos_32wunlink);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wrename),libc_dos_16wrename);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wrename),libc_dos_32wrename);

DEFINE_PUBLIC_ALIAS(_chdir,libc_chdir);
DEFINE_PUBLIC_ALIAS(_rmdir,libc_rmdir);
DEFINE_PUBLIC_ALIAS(_mkdir,libc_mkdir);
DEFINE_PUBLIC_ALIAS(_access,libc_access);
DEFINE_PUBLIC_ALIAS(_access_s,libc_access_s);
DEFINE_PUBLIC_ALIAS(_unlink,libc_unlink);
DEFINE_PUBLIC_ALIAS(_chmod,libc_chmod);
DEFINE_PUBLIC_ALIAS(__DSYM(_chdir),libc_dos_chdir);
DEFINE_PUBLIC_ALIAS(__DSYM(_rmdir),libc_dos_rmdir);
DEFINE_PUBLIC_ALIAS(__DSYM(_mkdir),libc_dos_mkdir);
DEFINE_PUBLIC_ALIAS(__DSYM(_access),libc_dos_access);
DEFINE_PUBLIC_ALIAS(__DSYM(_access_s),libc_dos_access_s);
DEFINE_PUBLIC_ALIAS(__DSYM(_unlink),libc_dos_unlink);
DEFINE_PUBLIC_ALIAS(__DSYM(_chmod),libc_dos_chmod);

DEFINE_PUBLIC_ALIAS(__KSYMw16(wchdir),libc_16wchdir);
DEFINE_PUBLIC_ALIAS(__KSYMw16(wmkdir),libc_16wmkdir);
DEFINE_PUBLIC_ALIAS(__KSYMw16(wrmdir),libc_16wrmdir);
DEFINE_PUBLIC_ALIAS(__KSYMw32(wchdir),libc_32wchdir);
DEFINE_PUBLIC_ALIAS(__KSYMw32(wmkdir),libc_32wmkdir);
DEFINE_PUBLIC_ALIAS(__KSYMw32(wrmdir),libc_32wrmdir);
DEFINE_PUBLIC_ALIAS(__DSYMw16(wchdir),libc_dos_16wchdir);
DEFINE_PUBLIC_ALIAS(__DSYMw16(wrmdir),libc_dos_16wrmdir);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wchdir),libc_dos_16wchdir);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wrmdir),libc_dos_16wrmdir);
DEFINE_PUBLIC_ALIAS(__DSYMw32(wchdir),libc_dos_32wchdir);
DEFINE_PUBLIC_ALIAS(__DSYMw32(wrmdir),libc_dos_32wrmdir);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wchdir),libc_dos_32wchdir);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wrmdir),libc_dos_32wrmdir);

/* Link the version without mode-argument by default in PE-mode. */
DEFINE_PUBLIC_ALIAS(__DSYMw16(wmkdir),libc_dos_16wmkdir2);
DEFINE_PUBLIC_ALIAS(__DSYMw32(wmkdir),libc_dos_32wmkdir2);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wmkdir),libc_dos_16wmkdir2);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wmkdir),libc_dos_32wmkdir2);
DEFINE_PUBLIC_ALIAS(__DSYMw16(wmkdir_m),libc_dos_16wmkdir);
DEFINE_PUBLIC_ALIAS(__DSYMw32(wmkdir_m),libc_dos_32wmkdir);
#if 1
DEFINE_PUBLIC_ALIAS(__KSYMw16(wmkdir),libc_16wmkdir);
DEFINE_PUBLIC_ALIAS(__KSYMw32(wmkdir),libc_32wmkdir);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wmkdir),libc_16wmkdir);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wmkdir),libc_32wmkdir);
#else
DEFINE_PUBLIC_ALIAS(__KSYMw16(wmkdir),libc_16wmkdir2);
DEFINE_PUBLIC_ALIAS(__KSYMw32(wmkdir),libc_32wmkdir2);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wmkdir),libc_16wmkdir2);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wmkdir),libc_32wmkdir2);
DEFINE_PUBLIC_ALIAS(__KSYMw16(wmkdir_m),libc_16wmkdir);
DEFINE_PUBLIC_ALIAS(__KSYMw32(wmkdir_m),libc_32wmkdir);
#endif

DEFINE_PUBLIC_ALIAS(__lock_fhandle,libc__lock_fhandle);
DEFINE_PUBLIC_ALIAS(_unlock_fhandle,libc_unlock_fhandle);
DEFINE_PUBLIC_ALIAS(_chdrive,libc_chdrive);
DEFINE_PUBLIC_ALIAS(_getdrive,libc_getdrive);
DEFINE_PUBLIC_ALIAS(_getdrives,libc_getdrives);
DEFINE_PUBLIC_ALIAS(chsize,libc_ftruncate);
DEFINE_PUBLIC_ALIAS(_chsize,libc_ftruncate);
DEFINE_PUBLIC_ALIAS(_chsize_s,libc_ftruncate64);
DEFINE_PUBLIC_ALIAS(_close,libc_close);
DEFINE_PUBLIC_ALIAS(_commit,libc_fdatasync);
DEFINE_PUBLIC_ALIAS(_dup,libc_dup);
DEFINE_PUBLIC_ALIAS(_dup2,libc_dup2);
DEFINE_PUBLIC_ALIAS(eof,libc_eof);
DEFINE_PUBLIC_ALIAS(_eof,libc_eof);
DEFINE_PUBLIC_ALIAS(filelength,libc_fsize);
DEFINE_PUBLIC_ALIAS(_filelength,libc_fsize);
DEFINE_PUBLIC_ALIAS(_filelengthi64,libc_fsize64);
DEFINE_PUBLIC_ALIAS(locking,libc_lockf);
DEFINE_PUBLIC_ALIAS(_locking,libc_lockf);
DEFINE_PUBLIC_ALIAS(_pipe,libc_dos_pipe);
DEFINE_PUBLIC_ALIAS(_lseek,libc_lseek);
DEFINE_PUBLIC_ALIAS(_lseeki64,libc_lseek64);
DEFINE_PUBLIC_ALIAS(tell,libc_ftell);
DEFINE_PUBLIC_ALIAS(_tell,libc_ftell);
DEFINE_PUBLIC_ALIAS(_telli64,libc_ftell64);
DEFINE_PUBLIC_ALIAS(setmode,libc_setmode);
DEFINE_PUBLIC_ALIAS(_setmode,libc_setmode);
DEFINE_PUBLIC_ALIAS(_umask,libc_umask);
DEFINE_PUBLIC_ALIAS(_umask_s,libc_umask_s);
DEFINE_PUBLIC_ALIAS(_get_osfhandle,libc_get_osfhandle);
DEFINE_PUBLIC_ALIAS(_open_osfhandle,libc_open_osfhandle);

#if __SIZEOF_SIZE_T__ != 4
/* Who knew NT is also the world leading in SCREWING UP SIMPLE FU$%#ING A$$ LOGIC?
 * And don't give me that backwards-compatibility bull$h1t, because it doesn't
 * apply to a new architecture that would have a different 'sizeof(size_t) == 8'.
 * Because at that point, what are you trying to be backwards-compatible towards? */
INTERN s32 (__LIBCCALL libc_dos_read)(int fd, void *buf, u32 bufsize) { return (s32)libc_read(fd,buf,(size_t)bufsize); }
INTERN s32 (__LIBCCALL libc_dos_write)(int fd, void const *buf, u32 bufsize) { return (s32)libc_write(fd,buf,(size_t)bufsize); }
DEFINE_PUBLIC_ALIAS(__DSYM(read),libc_dos_read);
DEFINE_PUBLIC_ALIAS(__DSYM(write),libc_dos_write);
DEFINE_PUBLIC_ALIAS(_read,libc_dos_read);
DEFINE_PUBLIC_ALIAS(_write,libc_dos_write);
#else /* __SIZEOF_SIZE_T__ != 4 */
DEFINE_PUBLIC_ALIAS(_read,libc_read);
DEFINE_PUBLIC_ALIAS(_write,libc_write);
#endif /* __SIZEOF_SIZE_T__ == 4 */

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


DECL_END

#endif /* !GUARD_LIBS_LIBC_UNISTD_C */
