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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_BLOCK_FILE_H
#define GUARD_KERNEL_INCLUDE_KERNEL_BLOCK_FILE_H 1

#include <errno.h>
#include <hybrid/compiler.h>
#include <kernel/fs.h>
#include <kernel/types.h>

DECL_BEGIN

typedef struct {
 /* Abstract representation of a file chunk ID through two __u64-s.
  * NOTE: FAT file systems use 'fc_data' as the address of the current sector. */
 blockid_t fc_index; /*< Chunk index (0: Start of file) */
 u64       fc_data;  /*< Block-type-specific index data (usually an LBA address for this chunk). */
} chunkid_t;

struct blockfile;

struct blockfileops {
 struct inodeops fo_node; /*< Underlying INode operations. */
 /* Additional, block-file specific operations. */
 errno_t (*fo_loadchunk)(struct blockfile const *__restrict self, chunkid_t const *__restrict chunk, USER void *__restrict buf);
 errno_t (*fo_savechunk)(struct blockfile *__restrict self, chunkid_t const *__restrict chunk, USER void const *__restrict buf);
 errno_t (*fo_nextchunk)(struct blockfile *__restrict self, chunkid_t *__restrict chunk);
 errno_t (*fo_findchunk)(struct blockfile *__restrict self, chunkid_t *__restrict chunk);
 /* Release all chunks following the one at index 'count', including 'count' itself */
 errno_t (*fo_releasechunks)(struct blockfile *__restrict self, blockid_t count);
};

/* Additional flags for 'bf_file.f_flag' */
#define BLOCKFILE_FLAG_NONE     0x00000000
#define BLOCKFILE_FLAG_CHANGED  0x10000000 /*< [lock(bf_lock)] Set when the given buffer was modified and must be flushed once exchanged. */
#define BLOCKFILE_FLAG_BUFDATA  0x20000000 /*< [lock(bf_lock)] The buffer is filled with valid data. */
#define BLOCKFILE_FLAG_CHUNK    0x40000000 /*< [lock(bf_lock)] The current chunk is selected ('fc_data' is valid). */
#define BLOCKFILE_FLAG_WASPREV  0x80000000 /*< [lock(bf_lock)] Used for optimizations: The last chunks was the chunk previous to the current. */

struct blockfile {
 struct file bf_file;      /*< Underlying file. */
 size_t      bf_chunksize; /*< [const] Size of a single chunk (in bytes). */
 byte_t     *bf_buffer;    /*< [lock(bf_lock)][1..bf_chunksize][const][owned] Intermediate file buffer. */
 byte_t     *bf_bufpos;    /*< [lock(bf_lock)][<bf_buffer>] Current r/w position in the buffer. */
 byte_t     *bf_bufend;    /*< (usually equals 'bf_buffer+bf_chunksize', but the last chunk may be smaller). */
 byte_t     *bf_bufmax;    /*< [const] == bf_buffer+bf_chunksize. */
 chunkid_t   bf_currchunk; /*< [lock(bf_lock)] The chunk id represented by 'bf_buffer'. */
 pos_t       bf_filesaved; /*< [lock(bf_lock)] Saved file size. */
 pos_t       bf_filesize;  /*< [lock(bf_lock)] Actual file size (When swapping the current chunk, this value is written). */
};
#define BLOCKFILE_FLAGS(o)               ((o)->bf_file.f_flag)
#define BLOCKFILE_OPS(o)                 ((struct blockfileops *)(o)->bf_file.f_ops)
#define BLOCKFILE_LOADCHUNK(o,chunk,buf) (*BLOCKFILE_OPS(o)->fo_loadchunk)(o,chunk,buf)
#define BLOCKFILE_SAVECHUNK(o,chunk,buf) (*BLOCKFILE_OPS(o)->fo_savechunk)(o,chunk,buf)
#define BLOCKFILE_NEXTCHUNK(o,chunk)     (*BLOCKFILE_OPS(o)->fo_nextchunk)(o,chunk)
#define BLOCKFILE_FINDCHUNK(o,chunk)     (*BLOCKFILE_OPS(o)->fo_findchunk)(o,chunk)
#define BLOCKFILE_RELEASECHUNKS(o,chunk) (*BLOCKFILE_OPS(o)->fo_releasechunks)(o,chunk)

FUNDEF ssize_t KCALL blockfile_read(struct file *__restrict fp, USER void *buf, size_t bufsize);
FUNDEF ssize_t KCALL blockfile_write(struct file *__restrict fp, USER void const *buf, size_t bufsize);
FUNDEF ssize_t KCALL blockfile_pread(struct file *__restrict fp, pos_t pos, USER void *buf, size_t bufsize);
FUNDEF ssize_t KCALL blockfile_pwrite(struct file *__restrict fp, pos_t pos, USER void const *buf, size_t bufsize);
FUNDEF off_t   KCALL blockfile_seek(struct file *__restrict fp, off_t off, int whence);
FUNDEF errno_t KCALL blockfile_ioctl(struct file *__restrict fp, attr_t name, USER void *arg);
FUNDEF errno_t KCALL blockfile_flush(struct file *__restrict fp);
FUNDEF ssize_t KCALL blockfile_getuattr(struct file *__restrict fp, attr_t name, USER void *buf, size_t bufsize);
FUNDEF errno_t KCALL blockfile_setuattr(struct file *__restrict fp, attr_t name, USER void const *buf, size_t bufsize);


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_BLOCK_FILE_H */
