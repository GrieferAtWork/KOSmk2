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
#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H 1

#include <__stdinc.h>
#include <features.h>
#include <sys/ioctl.h>

/* Header file for mounting/unmount Linux filesystems.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
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

__SYSDECL_BEGIN

#define BLOCK_SIZE      1024
#define BLOCK_SIZE_BITS 10

#define MS_RDONLY      0x00000001 /*< Mount read-only. */
#define MS_NOSUID      0x00000002 /*< Ignore suid and sgid bits. */
#define MS_NODEV       0x00000004 /*< Disallow access to device special files. */
#define MS_NOEXEC      0x00000008 /*< Disallow program execution. */
#define MS_SYNCHRONOUS 0x00000010 /*< Writes are synced at once. */
#define MS_REMOUNT     0x00000020 /*< Alter flags of a mounted FS. */
#define MS_MANDLOCK    0x00000040 /*< Allow mandatory locks on an FS. */
#define MS_DIRSYNC     0x00000080 /*< Directory modifications are synchronous. */
#define MS_NOATIME     0x00000400 /*< Do not update access times. */
#define MS_NODIRATIME  0x00000800 /*< Do not update directory access times. */
#define MS_BIND        0x00001000 /*< Bind directory at different place. */
#define MS_MOVE        0x00002000
#define MS_REC         0x00004000
#define MS_SILENT      0x00008000

#define MS_POSIXACL    0x00010000 /*< VFS does not apply the umask. */
#define MS_UNBINDABLE  0x00020000 /*< Change to unbindable. */
#define MS_PRIVATE     0x00040000 /*< Change to private. */
#define MS_SLAVE       0x00080000 /*< Change to slave. */
#define MS_SHARED      0x00100000 /*< Change to shared. */
#define MS_RELATIME    0x00200000 /*< Update atime relative to mtime/ctime. */
#define MS_KERNMOUNT   0x00400000 /*< This is a kern_mount call. */
#define MS_I_VERSION   0x00800000 /*< Update inode I_version field. */
#define MS_STRICTATIME 0x01000000 /*< Always perform atime updates. */
#define MS_LAZYTIME    0x02000000 /*< Update the on-disk [acm]times lazily. */
#define MS_ACTIVE      0x40000000
#define MS_NOUSER      0x80000000

/* Flags that can be altered by MS_REMOUNT  */
#define MS_RMT_MASK   (MS_RDONLY|MS_SYNCHRONOUS|MS_MANDLOCK|MS_I_VERSION|MS_LAZYTIME)

/* Magic mount flag number. Has to be or-ed to the flag values. */
#define MS_MGC_VAL     0xc0ed0000 /*< Magic flag number to indicate "new" flags. */
#define MS_MGC_MSK     0xffff0000 /*< Magic flag number mask. */

/* The read-only stuff doesn't really belong here, but any other place
 * is probably as bad and I don't want to create yet another include file.*/
#define BLKROSET     _IO(0x12,93)  /*< Set device read-only (0 = read-write). */
#define BLKROGET     _IO(0x12,94)  /*< Get read-only status (0 = read_write). */
#define BLKRRPART    _IO(0x12,95)  /*< Re-read partition table. */
#define BLKGETSIZE   _IO(0x12,96)  /*< Return device size. */
#define BLKFLSBUF    _IO(0x12,97)  /*< Flush buffer cache. */
#define BLKRASET     _IO(0x12,98)  /*< Set read ahead for block device. */
#define BLKRAGET     _IO(0x12,99)  /*< Get current read ahead setting. */
#define BLKFRASET    _IO(0x12,100) /*< Set filesystem read-ahead. */
#define BLKFRAGET    _IO(0x12,101) /*< Get filesystem read-ahead. */
#define BLKSECTSET   _IO(0x12,102) /*< Set max sectors per request. */
#define BLKSECTGET   _IO(0x12,103) /*< Get max sectors per request. */
#define BLKSSZGET    _IO(0x12,104) /*< Get block device sector size. */
#define BLKBSZGET    _IOR(0x12,112,size_t)
#define BLKBSZSET    _IOW(0x12,113,size_t)
#define BLKGETSIZE64 _IOR(0x12,114,size_t) /* return device size. */

/* Possible value for FLAGS parameter of `umount2'. */
#define MNT_FORCE       1 /*< Force unmounting. */
#define MNT_DETACH      2 /*< Just detach from the tree. */
#define MNT_EXPIRE      4 /*< Mark for expiry. */
#define UMOUNT_NOFOLLOW 8 /*< Don't follow symlink on umount. */

#ifndef __KERNEL__
/* Mount a filesystem. */
__LIBC __WARN_NODOSFS int (__LIBCCALL mount)(char const *__special_file, char const *__dir,
                                             char const *__fstype, unsigned long int __rwflag,
                                             void const *__data);
__LIBC __WARN_NODOSFS int (__LIBCCALL umount)(char const *__special_file);
__LIBC __WARN_NODOSFS int (__LIBCCALL umount2)(char const *__special_file, int __flags);
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_SYS_MOUNT_H */
