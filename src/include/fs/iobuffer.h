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
#ifndef GUARD_INCLUDE_FS_IOBUFFER_H
#define GUARD_INCLUDE_FS_IOBUFFER_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <sync/rwlock.h>
#include <sync/sig.h>

DECL_BEGIN

#define IOBUFFER_INTR_BLOCKFIRST 0x00000001 /*< Interrupt a block-first read operation, causing it to return ZERO(0) to indicate EOF. */

/* An I/O Buffer for one-directional read/write.
 * >> Specially designed to be written to from
 *    something like an interrupt handler, while
 *    being read from normal code.
 * >> Features a runtime-configurable max-size */
struct iobuffer {
 struct sig           ib_avail;   /*< Signal send when data becomes available (Used as a condition variable). */
 struct sig           ib_nfull;   /*< Signal send when memory becomes available in a previously full buffer. */
 rwlock_t             ib_rwlock;  /*< Read-write lock for all members below. */
 ATOMIC_DATA size_t   ib_maxsize; /*< Max size, the buffer is allowed to grow to, before data is dropped. */
 size_t               ib_size;    /*< [lock(ib_rwlock)] Allocated buffer size. */
 byte_t              *ib_buffer;  /*< [lock(ib_rwlock)][0..ib_size][owned] Base address of the R/W buffer. */
 ATOMIC_DATA byte_t  *ib_rpos;    /*< [lock(ib_rwlock)][0..1][in(ib_buffer+=ib_size)][cyclic:<=ib_wpos] Read pointer within the cyclic R/W buffer (Note atomic-write is allowed when holding a read-lock). */
 byte_t              *ib_wpos;    /*< [lock(ib_rwlock)][0..1][in(ib_buffer+=ib_size)][cyclic:>=ib_rpos] Write pointer within the cyclic R/W buffer. */
 ATOMIC_DATA u32      ib_mode;    /*< I/O Buffer mode (Set of 'IOBUFFER_*') */
};

#define IOBUFFER_DEFAULT_MAX_SIZE 0x200 /* 512 */

#define IOBUFFER_BUFEND(self)        ((self)->ib_buffer+(self)->ib_size)
#define IOBUFFER_MAXREAD(self,rpos) \
 ((rpos) < (self)->ib_wpos ? (size_t)((self)->ib_wpos-(rpos)) : \
 ((self)->ib_size-(size_t)((rpos)-(self)->ib_wpos)))
#define IOBUFFER_MAXWRITE(self,rpos) \
 ((rpos) >= (self)->ib_wpos ? (size_t)((rpos)-(self)->ib_wpos) : \
 ((self)->ib_size-(size_t)((self)->ib_wpos-(rpos))))

#define IOBUFFER_INIT_EX(max_size)  {SIG_INIT,SIG_INIT,RWLOCK_INIT,max_size,0,NULL,NULL,NULL,0}
#define IOBUFFER_INIT                IOBUFFER_INIT_EX(IOBUFFER_DEFAULT_MAX_SIZE)
#define IOBUFFER_CONSUME_INTR(self) (ATOMIC_FETCHAND((self)->ib_mode,~IOBUFFER_INTR_BLOCKFIRST)&IOBUFFER_INTR_BLOCKFIRST)

#define iobuffer_init_ex(self,maxsize) \
 (void)(sig_init(&(self)->ib_avail), \
        sig_init(&(self)->ib_nfull), \
        rwlock_init(&(self)->ib_rwlock), \
       (self)->ib_maxsize = (maxsize), \
       (self)->ib_size = 0, \
       (self)->ib_buffer = (self)->ib_rpos = \
       (self)->ib_wpos = NULL,(self)->ib_mode = 0)
#define iobuffer_cinit_ex(self,maxsize) \
 (void)(sig_cinit(&(self)->ib_avail), \
        sig_cinit(&(self)->ib_nfull), \
        rwlock_cinit(&(self)->ib_rwlock), \
       (self)->ib_maxsize = (maxsize), \
        assert((self)->ib_size == 0), \
        assert((self)->ib_buffer == NULL), \
        assert((self)->ib_rpos == NULL), \
        assert((self)->ib_wpos == NULL), \
        assert((self)->ib_mode == 0))
#define iobuffer_init(self)  iobuffer_init_ex(self,IOBUFFER_DEFAULT_MAX_SIZE)
#define iobuffer_cinit(self) iobuffer_cinit_ex(self,IOBUFFER_DEFAULT_MAX_SIZE)
FUNDEF SAFE void KCALL iobuffer_fini(struct iobuffer *__restrict self);

/* Discards all unread data from the given I/O.
 * buffer, returning the amount of bytes removed.
 * @return: 0 :     No unread data available.
 * @return: * :     Amount of bytes successfully discarded (Written latest)
 * @return: -EINTR: The calling thread was interrupted. */
FUNDEF SAFE ssize_t KCALL iobuffer_discard(struct iobuffer *__restrict self);


/* Get/Set the maximum allowed buffer size for a given I/O-buffer.
 * NOTE: Lowering the maximum size does not retroactively shrink an already allocated buffer.
 * @return: * : The old/current max-size of the I/O-buffer. */
#define iobuffer_get_maxsize(self)       ATOMIC_READ((self)->ib_maxsize)
#define iobuffer_set_maxsize(self,value) ATOMIC_XCH((self)->ib_maxsize,value)

/* Get the current maximum read/write buffer sizes.
 * WARNING: When used, these values must only be considered as
 *          meaningless hints, as the actual read/write size
 *          could already be something completely different at
 *          the moment this function returns.
 * @return: * :     The available read/write buffer size.
 * @return: -EINTR: The calling thread was interrupted. */
FUNDEF SAFE ssize_t KCALL iobuffer_get_read_size(struct iobuffer const *__restrict self);
FUNDEF SAFE ssize_t KCALL iobuffer_get_write_size(struct iobuffer const *__restrict self);


/* Try to reserve (preallocate) memory for write
 * operations with a total of 'write_size' bytes.
 * This function respects a set max-size, and returns
 * ZERO(0) if the buffer is at its limit, or when
 * it failed to allocate a bigger buffer.
 * @return: 0 :     - Failed to reallocate memory.
 *                  - The buffer is already at its limit
 *                    and not allowed to grow anymore.
 *                  - The given 'write_size' was ZERO(0)
 * @return: * :     Amount of bytes that were reserved.
 * @return: -EINTR: The calling thread was interrupted. */
FUNDEF SAFE ssize_t KCALL iobuffer_reserve(struct iobuffer *__restrict self, size_t write_size);

/* Try to release memory by flushing unused buffer data.
 * @return: * : The amount of flushed (freed) bytes. */
FUNDEF SAFE size_t KCALL iobuffer_flush(struct iobuffer *__restrict self);




typedef u8 iomode_t;
/* I/O read/write mode flags.
 * When used, always use exactly one 'IO_BLOCK*' value,
 * that may be or'ed together with any of the other flags. */
#define IO_BLOCKNONE  0x00 /*< Don't block if no (more) data can be read/written. */
#define IO_BLOCKFIRST 0x01 /*< Only block if no data existed when reading started/buffer was full when writing started. */
#define IO_BLOCKALL   0x03 /*< Block until the entirety of the provided buffer is filled/written (implies behavior of 'IO_BLOCKFIRST'). */
#define IO_NONE       0x00 /*< This flag does nothing... */
#define IO_PEEK       0x10 /*< Don't advance the read/write-pointer. */
#define IO_QUICKMOVE  0x20 /*< Don't restart the read/write process if another task operated on the same data (May lead to data being read/written more than once).
                            *  NOTE: This flag may not have any effect in locking I/O interfaces incapable of performing true asynchronous reads/writes. */
#define IO_SKIP       0x40 /*< Don't read data. - Instead discard I/O buffer memory. */

/* Read/Write generic memory to/from a given I/O buffer.
 * @param: mode:        I/O read/write configuration to-be used.
 * @param: rsize|wsize: Amount of bytes transferred after a success call.
 * @return: 0 :        [iobuffer_read][IO_BLOCKFIRST] The I/O buffer was interrupted using 'iobuffer_interrupt'.
 * @return: * :         The amount of bytes transferred (<= 'bufsize')
 * @return: -EFAULT:    A given pointer was faulty.
 * @return: -EINTR:     The calling thread was interrupted.
 * NOTE: [iobuffer_write] Neither 'IO_PEEK', 'IO_QUICKMOVE' nor 'IO_SKIP' are supported. */
FUNDEF ssize_t KCALL iobuffer_read(struct iobuffer *__restrict self, USER void *buf, size_t bufsize, iomode_t mode);
FUNDEF ssize_t KCALL iobuffer_write(struct iobuffer *__restrict self, USER void const *buf, size_t bufsize, iomode_t mode);
#define iobuffer_skip(self,max_skip,mode) iobuffer_read(self,NULL,max_skip,IO_SKIP|(mode))

/* Interrupts the next, or current blocking read operation
 * specifying 'IO_BLOCKFIRST' as its blocking argument.
 * Contrary to the documentation of 'IO_BLOCKFIRST', the
 * read operation will then return ZERO(0).
 * @return: -EOK:   The I/O interrupt was performed.
 * @return: -EINTR: The calling thread was interrupted. */
FUNDEF SAFE errno_t KCALL iobuffer_interrupt(struct iobuffer *__restrict self);


/* Attempt to recover previously read data, but not yet overwritten data.
 * WARNING: Due to internal data management and automatic GC cleanup,
 *          this function _NEVER_ guaranties being able to recover _ANYTHING_.
 * WARNING: It is impossible for an I/O buffer to differentiate between
 *          memory that is being re-used, and memory that was newly allocated
 *          for use in a buffer. For that reason, it is possible to ~recover~
 *          memory that didn't actually contain previously read data, but
 *          instead contain ZERO-initialized memory, as per convention
 *          to prevent random, free data (potentially containing unencrypted
 *          passwords or the like) from being leaked to user space.
 *       >> Keep in mind that you can unread more than what was actually read!
 * @param: max_unread: Max amount of bytes to unread.
 * @return: * :        Amount of bytes unwritten after a success call.
 * @return: -EINTR:    The calling thread was interrupted. */
FUNDEF ssize_t KCALL iobuffer_unread(struct iobuffer *__restrict self, size_t max_unread);


/* Take back previously written, but not yet read bytes.
 * WARNING: Due to race conditions, as well as cache optimizations,
 *          this function _NEVER_ guaranties being able to undo _ANYTHING_.
 * @param: max_unwrite: Max amount of bytes to unwrite.
 * @return: * :         Amount of bytes unwritten after a success call.
 * @return: -EINTR:     The calling thread was interrupted. */
FUNDEF ssize_t KCALL iobuffer_unwrite(struct iobuffer *__restrict self, size_t max_unwrite);


/* Seek the r/w pointer within validated restrictions, internally
 * calling 'iobuffer_read+IO_SKIP', 'iobuffer_unread' and 'iobuffer_unwrite'.
 * >> These functions can be used to implement a SEEK_CUR-style seek() callback
 *    for pipes and PTY devices, as well as other I/O-buffer-based objects.
 * @param: off: [kiobuf_rseek][off < 0]: Call 'iobuffer_unread' to move the r-pointer backwards (return the amount of unread bytes).
 *              [kiobuf_rseek][off > 0]: Call 'iobuffer_read+IO_SKIP' to move the r-pointer forwards (uses 'IO_BLOCKNONE'; return the amount of skipped bytes).
 *              [kiobuf_wseek][off < 0]: Call 'iobuffer_unwrite' to move the w-pointer backwards (return the amount of unwritten bytes).
 *              [kiobuf_wseek][off > 0]: return '0'.
 * @return: * :     See above.
 * @return: -EINTR: The calling thread was interrupted. */
#ifdef __INTELLISENSE__
FUNDEF ssize_t KCALL iobuffer_seek_read(struct iobuffer *__restrict self, ssize_t off);
FUNDEF ssize_t KCALL iobuffer_seek_write(struct iobuffer *__restrict self, ssize_t off);
#else
#define iobuffer_seek_read(self,off) \
   ((off) < 0 ? iobuffer_unread((self),(size_t)-(off)) \
              : iobuffer_skip((self),(size_t)(off),IO_BLOCKNONE))
#define iobuffer_seek_write(self,off) \
   ((off) < 0 ? iobuffer_unwrite((self),(size_t)-(off)) : 0)
#endif

DECL_END

#endif /* !GUARD_INCLUDE_FS_IOBUFFER_H */
