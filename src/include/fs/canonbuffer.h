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
#ifndef GUARD_INCLUDE_FS_CANONBUFFER_H
#define GUARD_INCLUDE_FS_CANONBUFFER_H 1

#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>
#include <malloc.h>

DECL_BEGIN

/* An I/O Buffer designed for being written to, before all written
 * data is captured at some point, at which point the buffer is cleared.
 * HINT: In the old KOS, this was called a `krawbuf' */
struct canonbuffer {
 atomic_rwlock_t      cb_lock;    /*< Read-write lock for all members below. */
 ATOMIC_DATA size_t   cb_maxsize; /*< Max size, the buffer is allowed to grow to, before data is dropped. */
 byte_t              *cb_buffer;  /*< [lock(cb_lock)][0..1][owned] Allocated buffer. */
 byte_t              *cb_end;     /*< [lock(cb_lock)][0..1] End of the allocated buffer. */
 ATOMIC_DATA byte_t  *cb_pos;     /*< [lock(cb_lock)][in(cb_buffer...cb_end)] Current buffer position.
                                   *   NOTE: Writes to this field must either be performed as
                                   *         atomic-cmpxch, or while holding a write-lock to `cb_lock'.  */
};

#define CANONBUFFER_INIT(maxsize) {ATOMIC_RWLOCK_INIT,maxsize,NULL,NULL,NULL}
#define canonbuffer_init(self,maxsize) \
  (void)(atomic_rwlock_init(&(self)->cb_lock), \
        (self)->cb_maxsize = (maxsize), \
        (self)->cb_buffer = (self)->cb_end = (self)->cb_pos = NULL)
#define canonbuffer_cinit(self,maxsize) \
  (void)(atomic_rwlock_cinit(&(self)->cb_lock), \
        (self)->cb_maxsize = (maxsize), \
         assert((self)->cb_buffer == NULL), \
         assert((self)->cb_end == NULL), \
         assert((self)->cb_pos == NULL))
#define canonbuffer_fini(self) free((self)->cb_buffer)

/* Get/Set the max buffer size of a canon buffer.
 * NOTE: Setting this lower than its actual size will not
 *       deallocate already allocated buffer memory retroactively. */
#define canonbuffer_getmaxsize(self)    ATOMIC_READ((self)->cb_maxsize)
#define canonbuffer_setmaxsize(self,v) ATOMIC_WRITE((self)->cb_maxsize,v)


/* Write the given user-data to the canon buffer.
 * @return: 0 :      Either `bufsize' was ZERO(0), or the canon buffer is full.
 * @return: * :      The actual amount of written bytes (<= `bufsize').
 * @return: -EFAULT: A faulty pointer was given. (NOTE: Faulty data may have still been output)
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF ssize_t KCALL canonbuffer_write(struct canonbuffer *__restrict self,
                                       USER void const *buf, size_t bufsize);

/* Clear the buffer and return the amount of discarded bytes. */
FUNDEF size_t KCALL canonbuffer_clear(struct canonbuffer *__restrict self);
/* Erase up to `max' to the latest written bytes, returning the exact amount deleted. */
FUNDEF size_t KCALL canonbuffer_erase(struct canonbuffer *__restrict self, size_t max);



struct canon {
 void   *c_data; /*< [0..c_size][owned] Canon data buffer. */
 size_t  c_size; /*< Canon data size. */
 byte_t *c_end;  /*< [0..1] The allocated canon buffer end. */
};

/* Capture/Release canon data.
 * These functions are used to extract written data, giving the caller exclusive
 * access to all of its contents, before `canonbuffer_release()' should be called
 * in order to return canon data to the buffer, allowing it to re-use the contained
 * data in the (likely) event that no new buffer had to be allowed in the meantime.
 * NOTE: These functions never fail and `canonbuffer_capture()' will simply return
 *       an empty canon in the event that no data was written.
 * WARNING: Do not attempt to free() `c_data' of the canon
 *          after `canonbuffer_release()' was called!
 *       >> The function's purpose already is to re-use an old canon buffer,
 *          and in the event that a new buffer was already allocated, _IT_ will
 *          free() the out-dated data pointer passed! */
FUNDEF struct canon KCALL canonbuffer_capture(struct canonbuffer *__restrict self);
FUNDEF void KCALL canonbuffer_release(struct canonbuffer *__restrict self, struct canon data);


DECL_END

#endif /* !GUARD_INCLUDE_FS_CANONBUFFER_H */
