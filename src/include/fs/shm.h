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
#ifndef GUARD_INCLUDE_FS_SHM_H
#define GUARD_INCLUDE_FS_SHM_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <fs/inode.h>
#include <fs/file.h>

DECL_BEGIN

struct mregion;


/* Shared memory file.
 * SHM Files are used to implement unix's 'shm_open()' family of functions found in <sys/mman.h>.
 * In KOS, they are implemented using the native lazy-allocation functionality of memory regions,
 * in that by default an SHM file is 3Gb large (Wow!), but you must remember that upon creation
 * exactly 0 bytes of that are already allocated.
 * To accomplish what they do, SHM INode/Files override the following operators:
 *    - struct mregion *INODE::f_mmap(struct file *fp, pos_t pos, size_t size, raddr_t *pregion_start):
 *       #1 Ensure that 'pos+size' are located in-bounds of the file (below 3Gb; aka KERNEL_BASE)
 *       #2 Fill '*pregion_start' with 'pos', thus causing the SHM region's mapping to start
 *          at the desired address, while still pointing into the same underlying memory region.
 *          This way, SHM regions are natively shared by means of using the same underlying
 *         `mregion' to communicate, natively enabling support for 'futex()', process-local COW,
 *          as well as lazy allocation of memory only when it is used for the first time.
 *       #3 Return 'shm_region' (See below)
 * Now that I think about it, that's pretty much all the magic there is to it...
 * Of course it also implements other file-operators to allow it to (p)read like a regular file,
 * but all of the magic simply uses regular, old features of any mregion-compatible mapping.
 * HINT: SHM nodes are used when creating a regular file in any tmpfs filesystem, such as '/dev',
 *       essentially meaning that linux's '/dev/shm' can even be a regular old folder that doesn't
 *       even need to be its own mounting point!
 * Additional operators implemented are:
 *    - ino_fini:
 *        Cleanup
 *    - ino_fopen:
 *        Implemented using 'inode_fopen_default'
 *    - f_allocate:
 *        Prefault memory of a given address range.
 *    - f_read/f_write/f_seek/f_pread/f_pwrite:
 *        Simulate regular file access (File pointer is an 'raddr_t')
 *    - ino_stat:
 *        Use `PAGESIZE' to for st_blksize.
 *        Calculate allocated pages for 'st_blocks'/'st_size'.
 *    - ino_setattr: 
 *        Replace the stored region with a new one when truncating to ZERO-size.
 * NOTE: Additional operators may be implemented in the future.
 *       Don't assume NULL-ops based on this!
 */
struct shm_node {
 struct inode        sh_node;   /*< Underlying Inode descriptor. */
 REF struct mregion *sh_region; /*< [1..1][owned][lock(sh_node.i_attr_lock)]
                                 *  Shared-memory region used to implement file mappings (See above.)
                                 *  NOTE: This member is exchanged for a new region
                                 *        when the node is truncated ('O_TRUNC'). */
};

/* INode operators for SHM nodes. */
DATDEF struct inodeops shm_ops;
#define INODE_ISSHM(self) ((self)->i_ops == &shm_ops)



struct shm_file {
 /* A simple wrapper for accessing SHM nodes using regular files. */
 struct file         sh_file;   /*< Underlying file descriptor. */
 raddr_t             sh_addr;   /*< [lock(shm_file.f_lock)] Current in-file R/W position. */
 REF struct mregion *sh_region; /*< [1..1][owned][const] */
};


DECL_END

#endif /* !GUARD_INCLUDE_FS_SHM_H */
