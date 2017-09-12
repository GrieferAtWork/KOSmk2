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
#ifndef GUARD_KERNEL_FS_CANONBUFFER_C_INL
#define GUARD_KERNEL_FS_CANONBUFFER_C_INL 1

#include <fs/canonbuffer.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kernel/user.h>
#include <sys/syslog.h>

DECL_BEGIN

PUBLIC ssize_t KCALL
canonbuffer_write(struct canonbuffer *__restrict self,
                  USER void const *buf, size_t bufsize) {
 size_t result; byte_t *target,*newbuf;
 bool has_write_lock = false;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->cb_lock);
scan_again:
 do {
  target = ATOMIC_READ(self->cb_pos);
  assert(target <= self->cb_end);
  result = (size_t)(self->cb_end-target);
  if (result >= bufsize) result = bufsize;
  else {
   size_t oldsize,newsize,maxsize;
   /* Buffer is too small. - Check if we can allocate more. */
   oldsize = (size_t)(self->cb_end-self->cb_buffer);
   newsize = (size_t)(target-self->cb_buffer);
   if (__builtin_add_overflow(newsize,bufsize,&newsize))
       newsize = (size_t)-1;
   assert(newsize >= oldsize);
   maxsize = ATOMIC_READ(self->cb_maxsize);
   if (newsize >= maxsize) newsize = maxsize;
   if (newsize == oldsize) {
    /* The buffer is full, if 'bufsize' is ZERO(0). */
    if (has_write_lock)
         atomic_rwlock_endwrite(&self->cb_lock);
    else atomic_rwlock_endread (&self->cb_lock);
    return 0;
   }
   /* Upgrade to a write-lock, so we can allocate more memory. */
   if (!has_write_lock) {
    has_write_lock = true;
    if (!atomic_rwlock_upgrade(&self->cb_lock))
         goto scan_again;
    /* Make sure to use an up-to-date write header. */
    if (target != ATOMIC_READ(self->cb_pos))
        goto scan_again;
   }

   /* Overallocate a bit */
   newsize = MIN(CEIL_ALIGN(newsize,128),maxsize);
   newbuf = (byte_t *)realloc(self->cb_buffer,newsize);
   if unlikely(!newbuf) {
    atomic_rwlock_endwrite(&self->cb_lock);
    return -ENOMEM;
   }
   /* Calculate the new target. */
   target = newbuf+(target-self->cb_buffer);
   self->cb_buffer = newbuf;
   self->cb_end    = newbuf+newsize;
   /* Figure out how much memory can be written now. */
   result = (size_t)(self->cb_end-target);
   if (result > bufsize) result = bufsize;
   /* Fix-up the resulting canon-pointer. */
   self->cb_pos = target+result;

   /* Make sure everything above has been written, as we're
    * going to skip the cmpxch() below and need all changes to
    * have happened once we're going to downgrade our write lock. */
   COMPILER_WRITE_BARRIER();
   break;
  }
 } while (!ATOMIC_CMPXCH(self->cb_pos,target,target+result));
 if (has_write_lock) atomic_rwlock_downgrade(&self->cb_lock);
 /* Memory has been allocated. - Now to write to it. */
 if (copy_from_user(target,buf,result)) {
  /* Fill the memory with all ZEROes to prevent
   * kernel data from leaking to userspace. */
  if (!ATOMIC_CMPXCH(self->cb_pos,target+result,target))
       memset(target,0,result);
  result = (size_t)-EFAULT;
 }
 atomic_rwlock_endread(&self->cb_lock);
 return (ssize_t)result;
}

PUBLIC size_t KCALL
canonbuffer_clear(struct canonbuffer *__restrict self) {
 size_t result; byte_t *old_pos;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->cb_lock);
 old_pos = ATOMIC_XCH(self->cb_pos,self->cb_buffer);
 result = (size_t)(old_pos-self->cb_buffer);
 atomic_rwlock_endread(&self->cb_lock);
 return result;
}
FUNDEF size_t KCALL
canonbuffer_erase(struct canonbuffer *__restrict self, size_t max) {
 byte_t *old_pos,*new_pos;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->cb_lock);
 do {
  old_pos = ATOMIC_READ(self->cb_pos);
  new_pos = old_pos-max;
  if (new_pos > old_pos ||
      new_pos < self->cb_buffer)
      new_pos = self->cb_buffer;
 } while (!ATOMIC_CMPXCH(self->cb_pos,old_pos,new_pos));
 atomic_rwlock_endread(&self->cb_lock);
 return (size_t)(old_pos-new_pos);
}


PUBLIC struct canon KCALL
canonbuffer_capture(struct canonbuffer *__restrict self) {
 struct canon result;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_write(&self->cb_lock);
 /* Capture canon data. */
 result.c_data = self->cb_buffer;
 result.c_size = (size_t)(self->cb_pos-self->cb_buffer);
 result.c_end  = self->cb_end;
 self->cb_end  = self->cb_pos = self->cb_buffer;
 atomic_rwlock_endwrite(&self->cb_lock);
 return result;
}
PUBLIC void KCALL
canonbuffer_release(struct canonbuffer *__restrict self, struct canon data) {
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->cb_lock);
 if likely(self->cb_end == self->cb_buffer) {
  if likely(atomic_rwlock_upgrade(&self->cb_lock) ||
           (self->cb_end == self->cb_buffer)) {
   assert(data.c_end >= (byte_t *)data.c_data+data.c_size);
   self->cb_buffer = (byte_t *)data.c_data;
   self->cb_pos    = (byte_t *)data.c_data;
   self->cb_end    = (byte_t *)data.c_end;
   atomic_rwlock_endwrite(&self->cb_lock);
   return;
  }
  atomic_rwlock_endwrite(&self->cb_lock);
 } else {
  atomic_rwlock_endread(&self->cb_lock);
 }
 /* Another buffer was already allocated. - Must free the old one. */
 free(data.c_data);
}


DECL_END

#endif /* !GUARD_KERNEL_FS_CANONBUFFER_C_INL */
