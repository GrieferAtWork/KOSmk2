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
#ifndef _BITS_SYSCALL_H
#define _BITS_SYSCALL_H 1

/* Alias all system call numbers using the 'SYS_*' notation in favor of '__NR_*' */

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
#define SYS_dup __NR_dup
#define SYS_dup3 __NR_dup3
#define SYS_fcntl __NR_fcntl
#define SYS_ioctl __NR_ioctl
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
#define SYS_lseek __NR_lseek
#define SYS_read __NR_read
#define SYS_write __NR_write
#define SYS_pread64 __NR_pread64
#define SYS_pwrite64 __NR_pwrite64
#define SYS_exit __NR_exit
#define SYS_unshare __NR_unshare
#define SYS_munmap __NR_munmap
#define SYS_mremap __NR_mremap
#define SYS_execve __NR_execve
#define SYS_mmap __NR_mmap
#define SYS_fork __NR_fork
#define SYS_xsysprint __NR_xsysprint
#define SYS_xmmap __NR_xmmap
#define SYS_xmunmap __NR_xmunmap
#define SYS_xsharesym __NR_xsharesym
//[[[end]]]

#endif /* !_BITS_SYSCALL_H */
