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
#ifndef _ARM_KOS_BITS_SYSCALL_H
#define _ARM_KOS_BITS_SYSCALL_H 1
#define _BITS_SYSCALL_H 1

/* Alias all system call numbers using the `SYS_*' notation in favor of `__NR_*' */

/*[[[deemon
#include <file>
#include <fs>
fs::chdir(fs::path::head(__FILE__));
local fp = file.open("../asm/syscallno.ci");
for (local line: fp) {
    local name;
    try name = line.scanf(" # define __NR_%s ")...;
    catch (...) continue;
    if (name.endswith("syscall_min") ||
        name.endswith("syscall_max"))
        continue;
    print "#define SYS_"+name,"__NR_"+name;
}
]]]*/
#define SYS_getcwd __NR_getcwd
#define SYS_dup __NR_dup
#define SYS_dup3 __NR_dup3
#define SYS_fcntl __NR_fcntl
#define SYS_ioctl __NR_ioctl
#define SYS_mknodat __NR_mknodat
#define SYS_mkdirat __NR_mkdirat
#define SYS_unlinkat __NR_unlinkat
#define SYS_symlinkat __NR_symlinkat
#define SYS_linkat __NR_linkat
#define SYS_renameat __NR_renameat
#define SYS_umount2 __NR_umount2
#define SYS_mount __NR_mount
#define SYS_truncate __NR_truncate
#define SYS_ftruncate __NR_ftruncate
#define SYS_fallocate __NR_fallocate
#define SYS_faccessat __NR_faccessat
#define SYS_chdir __NR_chdir
#define SYS_fchdir __NR_fchdir
#define SYS_chroot __NR_chroot
#define SYS_fchmod __NR_fchmod
#define SYS_fchmodat __NR_fchmodat
#define SYS_fchownat __NR_fchownat
#define SYS_fchown __NR_fchown
#define SYS_openat __NR_openat
#define SYS_close __NR_close
#define SYS_pipe2 __NR_pipe2
#define SYS_lseek __NR_lseek
#define SYS_read __NR_read
#define SYS_write __NR_write
#define SYS_pread64 __NR_pread64
#define SYS_pwrite64 __NR_pwrite64
#define SYS_pselect6 __NR_pselect6
#define SYS_ppoll __NR_ppoll
#define SYS_readlinkat __NR_readlinkat
#define SYS_fstatat64 __NR_fstatat64
#define SYS_fstat64 __NR_fstat64
#define SYS_sync __NR_sync
#define SYS_fsync __NR_fsync
#define SYS_fdatasync __NR_fdatasync
#define SYS_utimensat __NR_utimensat
#define SYS_exit __NR_exit
#define SYS_exit_group __NR_exit_group
#define SYS_waitid __NR_waitid
#define SYS_unshare __NR_unshare
#define SYS_futex __NR_futex
#define SYS_nanosleep __NR_nanosleep
#define SYS_sched_yield __NR_sched_yield
#define SYS_kill __NR_kill
#define SYS_tkill __NR_tkill
#define SYS_tgkill __NR_tgkill
#define SYS_sigsuspend __NR_sigsuspend
#define SYS_sigaction __NR_sigaction
#define SYS_sigprocmask __NR_sigprocmask
#define SYS_sigpending __NR_sigpending
#define SYS_sigtimedwait __NR_sigtimedwait
#define SYS_sigreturn __NR_sigreturn
#define SYS_setpgid __NR_setpgid
#define SYS_getpgid __NR_getpgid
#define SYS_umask __NR_umask
#define SYS_gettimeofday __NR_gettimeofday
#define SYS_settimeofday __NR_settimeofday
#define SYS_getpid __NR_getpid
#define SYS_getppid __NR_getppid
#define SYS_gettid __NR_gettid
#define SYS_sysinfo __NR_sysinfo
#define SYS_munmap __NR_munmap
#define SYS_mremap __NR_mremap
#define SYS_clone __NR_clone
#define SYS_execve __NR_execve
#define SYS_mmap __NR_mmap
#define SYS_swapon __NR_swapon
#define SYS_swapoff __NR_swapoff
#define SYS_mprotect __NR_mprotect
#define SYS_wait4 __NR_wait4
#define SYS_syncfs __NR_syncfs
#define SYS_fork __NR_fork
#define SYS_xsyslog __NR_xsyslog
#define SYS_xmmap __NR_xmmap
#define SYS_xmunmap __NR_xmunmap
#define SYS_xsharesym __NR_xsharesym
#define SYS_xreaddir __NR_xreaddir
#define SYS_xopenpty __NR_xopenpty
#define SYS_xfdname __NR_xfdname
#define SYS_xpipe __NR_xpipe
#define SYS_xfexecve __NR_xfexecve
#define SYS_xfchdirat __NR_xfchdirat
#define SYS_xrenameat __NR_xrenameat
#define SYS_xfsymlinkat __NR_xfsymlinkat
#define SYS_xfreadlinkat __NR_xfreadlinkat
#define SYS_xdlopen __NR_xdlopen
#define SYS_xfdlopen __NR_xfdlopen
#define SYS_xdlclose __NR_xdlclose
#define SYS_xdlsym __NR_xdlsym
#define SYS_xpaused __NR_xpaused
//[[[end]]]

#endif /* !_ARM_KOS_BITS_SYSCALL_H */
