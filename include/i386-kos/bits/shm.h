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
#ifndef _BITS_SHM_H
#define _BITS_SHM_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>
#include <hybrid/host.h>

__SYSDECL_BEGIN

/* Copyright (C) 1995-2016 Free Software Foundation, Inc.
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

/* Permission flag for shmget. */
#define SHM_R  0400 /*< or S_IRUGO from <linux/stat.h> */
#define SHM_W  0200 /*< or S_IWUGO from <linux/stat.h> */

/* Flags for `shmat'. */
#define SHM_RDONLY 0010000 /*< attach read-only else read-write. */
#define SHM_RND    0020000 /*< round attach address to SHMLBA. */
#define SHM_REMAP  0040000 /*< take-over region on attach. */
#define SHM_EXEC   0100000 /*< execution access. */

/* Commands for `shmctl'. */
#define SHM_LOCK   11 /*< lock segment (root only). */
#define SHM_UNLOCK 12 /*< unlock segment (root only). */

/* Segment low boundary address multiple.  */
#define SHMLBA        (__getpagesize())
#if defined(__KERNEL__) || \
  (!defined(__CRT_GLC) || defined(__DOS_COMPAT__))
__SYSDECL_END
#include <hybrid/limits.h>
__SYSDECL_BEGIN
#define __getpagesize()  __PAGESIZE
#else /* Hard-coded */
__LIBC __ATTR_CONST int __NOTHROW((__LIBCCALL __getpagesize)(void));
#endif /* Soft-coded */

/* Type to count number of attaches.  */
typedef __syscall_ulong_t shmatt_t;

/* Data structure describing a shared memory segment.  */
struct shmid_ds {
    struct ipc_perm     shm_perm;  /*< operation permission struct. */
    size_t              shm_segsz; /*< size of segment in bytes. */
    __time32_t          shm_atime; /*< time of last shmat(). */
#ifndef __x86_64__
    __UINT32_TYPE__   __glibc_reserved1;
#endif /* !__x86_64__ */
    __time32_t          shm_dtime; /*< time of last shmdt(). */
#ifndef __x86_64__
    __UINT32_TYPE__   __glibc_reserved2;
#endif /* !__x86_64__ */
    __time32_t          shm_ctime; /*< time of last change by shmctl(). */
#ifndef __x86_64__
    __UINT32_TYPE__   __glibc_reserved3;
#endif /* !__x86_64__ */
    __pid_t             shm_cpid;  /*< pid of creator. */
    __pid_t             shm_lpid;  /*< pid of last shmop. */
    shmatt_t            shm_nattch; /*< number of current attaches. */
    __syscall_ulong_t __glibc_reserved4;
    __syscall_ulong_t __glibc_reserved5;
};

#ifdef __USE_MISC

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("used_ids")
#pragma push_macro("swap_attempts")
#pragma push_macro("swap_successes")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef used_ids
#undef swap_attempts
#undef swap_successes

/* ipcs ctl commands */
#define SHM_STAT        13
#define SHM_INFO        14

/* shm_mode upper byte flags. */
#define SHM_DEST       01000 /*< segment will be destroyed on last detach. */
#define SHM_LOCKED     02000 /*< segment will not be swapped. */
#define SHM_HUGETLB    04000 /*< segment is mapped via hugetlb. */
#define SHM_NORESERVE 010000 /*< don't check for reservations. */

struct shminfo {
 __syscall_ulong_t shmmax;
 __syscall_ulong_t shmmin;
 __syscall_ulong_t shmmni;
 __syscall_ulong_t shmseg;
 __syscall_ulong_t shmall;
 __syscall_ulong_t __glibc_reserved1;
 __syscall_ulong_t __glibc_reserved2;
 __syscall_ulong_t __glibc_reserved3;
 __syscall_ulong_t __glibc_reserved4;
};

struct shm_info {
 int               used_ids;
 __syscall_ulong_t shm_tot; /*< total allocated shm. */
 __syscall_ulong_t shm_rss; /*< total resident shm. */
 __syscall_ulong_t shm_swp; /*< total swapped shm. */
 __syscall_ulong_t swap_attempts;
 __syscall_ulong_t swap_successes;
};

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("swap_successes")
#pragma pop_macro("swap_attempts")
#pragma pop_macro("used_ids")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#endif /* __USE_MISC */

__SYSDECL_END

#endif /* !_BITS_SHM_H */
