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
#ifndef GUARD_INCLUDE_FS_MEMFILE_H
#define GUARD_INCLUDE_FS_MEMFILE_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <fs/file.h>
#include <fs/inode.h>
#include <sync/rwlock.h>

DECL_BEGIN

struct inode;
struct dentry;
struct mman;

struct memfile {
 /* memory-file, allowing unfiltered access to some address range in any page-directory.
  * >> This type of file is used to implement a lot of different things in the filesystem:
  *  - /dev/kmem
  *  - /proc/kcore
  *  - /proc/cmdline
  *  - /proc/PID/mem
  *  - /proc/PID/cmdline
  * NOTE: Attempting to read from/write to unmapped memory is a
  *       no-op, returning ZERO(0) and thus indicating that no
  *       memory can be accessed. */
 struct file      m_file; /*< Underlying file stream. */
 REF struct mman *m_mman; /*< [1..1][const] The memory manager referenced. */
 VIRT uintptr_t   m_min;  /*< [const][<= m_max] The lowest virtual address to which data may be written.
                           *   NOTE: seek(0,SEEK_SET) will navigate to this location. */
 VIRT uintptr_t   m_max;  /*< [const][>= m_min] The greatest virtual address to which data may be written. */
 VIRT uintptr_t   m_pos;  /*< [lock(m_file.f_lock)] The current R/W position (may be located out-of-bounds). */
};

/* Create and return a new memory file.
 * The value returned by this function can be directly returned from 'ino_fopen' operators.
 * @return: * :      A reference to the newly created (and set-up) file.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF REF struct file *KCALL
make_memfile(struct inode *__restrict node,
             struct dentry *__restrict dent, oflag_t oflags,
             struct mman *__restrict mm,
             VIRT uintptr_t min, VIRT uintptr_t max);


/* Flags to-be assed to the 'f_flags' field of inodeops using memfiles. */
#define MEMFILE_FLAGS  INODE_FILE_NORMAL
/* Operators to-be used when creating an INode type that refers to a memfile. */
FUNDEF ssize_t KCALL memfile_read(struct file *__restrict fp, USER void *buf, size_t bufsize);
FUNDEF ssize_t KCALL memfile_write(struct file *__restrict fp, USER void const *buf, size_t bufsize);
FUNDEF ssize_t KCALL memfile_pread(struct file *__restrict fp, USER void *buf, size_t bufsize, pos_t pos);
FUNDEF ssize_t KCALL memfile_pwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize, pos_t pos);
FUNDEF off_t KCALL memfile_seek(struct file *__restrict fp, off_t off, int whence);
FUNDEF void KCALL memfile_fclose(struct inode *__restrict ino, struct file *__restrict fp);
#define MEMFILE_OPS_INIT \
    .f_flags    = MEMFILE_FLAGS, \
    .ino_fclose = &memfile_fclose, \
    .f_read     = &memfile_read, \
    .f_write    = &memfile_write, \
    .f_pread    = &memfile_pread, \
    .f_pwrite   = &memfile_pwrite, \
    .f_seek     = &memfile_seek, \

DECL_END

#endif /* !GUARD_INCLUDE_FS_MEMFILE_H */
