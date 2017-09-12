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
#ifndef GUARD_KERNEL_FS_IOBUFFER_C
#define GUARD_KERNEL_FS_IOBUFFER_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <fs/iobuffer.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/types.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <sched/task.h>
#include <string.h>
#include <sync/rwlock.h>
#include <sync/sig.h>

DECL_BEGIN

PUBLIC SAFE void KCALL
iobuffer_fini(struct iobuffer *__restrict self) {
 free(self->ib_buffer);
}

PUBLIC SAFE ssize_t KCALL
iobuffer_discard(struct iobuffer *__restrict self) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 result = (ssize_t)rwlock_write(&self->ib_rwlock);
 if (E_ISERR(result)) goto end;
 result = (ssize_t)IOBUFFER_MAXREAD(self,self->ib_rpos);
 self->ib_wpos = self->ib_buffer;
 self->ib_rpos = self->ib_buffer+self->ib_size;
 rwlock_endwrite(&self->ib_rwlock);
end:
 return result;
}

PUBLIC SAFE ssize_t KCALL
iobuffer_get_read_size(struct iobuffer const *__restrict self) {
 ssize_t result; byte_t *rpos;
 CHECK_HOST_DOBJ(self);
 result = (ssize_t)rwlock_read((rwlock_t *)&self->ib_rwlock);
 if (E_ISERR(result)) goto end;
 rpos = ATOMIC_READ(self->ib_rpos);
 result = IOBUFFER_MAXREAD(self,rpos);
 rwlock_endread((rwlock_t *)&self->ib_rwlock);
end:
 return result;
}
PUBLIC SAFE ssize_t KCALL
iobuffer_get_write_size(struct iobuffer const *__restrict self) {
 ssize_t result; byte_t *rpos;
 CHECK_HOST_DOBJ(self);
 result = (ssize_t)rwlock_read((rwlock_t *)&self->ib_rwlock);
 if (E_ISERR(result)) goto end;
 rpos = ATOMIC_READ(self->ib_rpos);
 result = IOBUFFER_MAXWRITE(self,rpos);
 rwlock_endread((rwlock_t *)&self->ib_rwlock);
end:
 return result;
}

PUBLIC SAFE ssize_t KCALL
iobuffer_reserve(struct iobuffer *__restrict self,
                 size_t write_size) {
 ssize_t result; size_t new_total_size;
 byte_t *new_buffer;
 CHECK_HOST_DOBJ(self);
 result = rwlock_write(&self->ib_rwlock);
 if (E_ISERR(result)) goto end;
 assert((self->ib_size != 0)    == (self->ib_buffer != NULL));
 assert((self->ib_rpos != NULL) == (self->ib_buffer != NULL));
 assert((self->ib_wpos != NULL) == (self->ib_buffer != NULL));
 new_total_size = self->ib_size+write_size;
 /* Clamp the new total size to what is actually allowed. */
 new_total_size = MIN(new_total_size,iobuffer_get_maxsize(self));
 assert(new_total_size >= self->ib_size);
 result = 0;
 if (new_total_size != self->ib_size) {
  size_t rpos,wpos;
  assert(new_total_size);
  /* Try to allocate a new buffer of sufficient size. */
  new_buffer = (byte_t *)krealloc(self->ib_buffer,new_total_size,
                                  GFP_SHARED|GFP_CALLOC);
  if __unlikely(!new_buffer) goto end2;
  result = new_total_size-self->ib_size;
  /* Install the new buffer */
  rpos = self->ib_rpos-self->ib_buffer;
  wpos = self->ib_wpos-self->ib_buffer;
  self->ib_buffer = new_buffer;
  self->ib_rpos   = new_buffer+rpos;
  self->ib_wpos   = new_buffer+wpos;
  self->ib_size   = new_total_size;
 }
end2:
 rwlock_endwrite(&self->ib_rwlock);
end:
 return result;
}

PUBLIC SAFE errno_t KCALL
iobuffer_interrupt(struct iobuffer *__restrict self) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 error = rwlock_write(&self->ib_rwlock);
 if (E_ISOK(error)) {
  /* Set the interrupt flag to stop the next empty read call. */
  self->ib_mode |= IOBUFFER_INTR_BLOCKFIRST;
  COMPILER_WRITE_BARRIER();
  sig_broadcast(&self->ib_avail);
  rwlock_endwrite(&self->ib_rwlock);
 }
 return error;
}

PUBLIC SAFE size_t KCALL
iobuffer_flush(struct iobuffer *__restrict self) {
 byte_t *newbuf;
 size_t newsize,result = 0;
 CHECK_HOST_DOBJ(self);
 task_nointr();
 rwlock_write(&self->ib_rwlock);
 if ((newsize = IOBUFFER_MAXREAD(self,self->ib_rpos)) != self->ib_size) {
  assert(newsize < self->ib_size);
  if (!newsize) {
   free(self->ib_buffer);
   self->ib_buffer = NULL;
   self->ib_rpos   = NULL;
   self->ib_wpos   = NULL;
  } else {
   size_t rindex = self->ib_rpos-self->ib_buffer;
   newbuf = (byte_t *)realloc(self->ib_buffer,newsize);
   if unlikely(!newbuf) goto end;
   /* Shift unread memory towards the start */
   memmove(newbuf,newbuf+rindex,newsize-rindex);
   self->ib_rpos   = newbuf;
   self->ib_wpos   = newbuf+newsize;
   self->ib_buffer = newbuf;
  }
  result        = self->ib_size-newsize;
  self->ib_size = newsize;
 }
end:
 rwlock_endwrite(&self->ib_rwlock);
 task_endnointr();
 return result;
}

#if defined(CONFIG_DEBUG) && 0
#define DEBUG_TRACE(x) (void)x
#else
#define DEBUG_TRACE(x) (void)0
#endif

PUBLIC ssize_t KCALL
iobuffer_read(struct iobuffer *__restrict self,
              USER void *buf, size_t bufsize,
              iomode_t mode) {
 size_t result,max_read_linear,max_read,destsize;
 ssize_t error; byte_t *bufend,*rpos,*start_rpos;
 byte_t *destbuf;
 CHECK_HOST_DOBJ(self);
again_full:
 destbuf  = (byte_t *)buf;
 destsize = bufsize;
 result   = 0;
again:
 error = rwlock_read(&self->ib_rwlock);
/*again_locked:*/
 if (E_ISERR(error)) goto end_always;
 CHECK_HOST_DATA(self->ib_buffer,self->ib_size);
 start_rpos = rpos = ATOMIC_READ(self->ib_rpos);
 assert(rpos >= self->ib_buffer && rpos <= IOBUFFER_BUFEND(self));
 assert(self->ib_wpos >= self->ib_buffer && self->ib_wpos <= IOBUFFER_BUFEND(self));
 bufend = IOBUFFER_BUFEND(self);
 max_read = IOBUFFER_MAXREAD(self,rpos);
 if (!max_read) {
buffer_is_empty:
  if (mode&IO_BLOCKFIRST) {
   /* Don't wait if we've already read something and are
    * only supposed to block for the first chunk of data. */
   if ((mode&(IO_BLOCKFIRST|IO_BLOCKALL)) == IO_BLOCKFIRST) {
    if (result != 0) goto end_rpos;
    if (IOBUFFER_CONSUME_INTR(self)) {
handle_intr:
     rwlock_endread(&self->ib_rwlock);
handle_intr2:
     error = 0;
     goto end_always2;
    }
   }
   /* Wait until at there is at least something to read
    * NOTE: The following like essentially performs what a
    *       condition variable calls its wait-operation. */
   //DEBUG_TRACE(syslog(LOG_DEBUG,"WAIT FOR DATA\n"));
   sig_write(&self->ib_avail);
   rwlock_endread(&self->ib_rwlock);
   if (IOBUFFER_CONSUME_INTR(self)) {
    sig_endwrite(&self->ib_avail);
    goto handle_intr2;
   }
   error = sig_recv_endwrite(&self->ib_avail);
   DEBUG_TRACE(syslog(LOG_DEBUG,"DATA AVAILABLE %Iu %p %p %p %p\n",
                       IOBUFFER_MAXREAD(self,self->ib_rpos),
                       self->ib_rpos,self->ib_wpos,
                       self->ib_buffer,self->ib_buffer+self->ib_size));
   if (E_ISERR(error)) goto end_always;
   goto again;
  } else goto end_rpos;
 }
 /* Check for an I/O interrupt in block-first mode. */
 if ((mode&(IO_BLOCKFIRST|IO_BLOCKALL)) == IO_BLOCKFIRST &&
      IOBUFFER_CONSUME_INTR(self))
      goto handle_intr;
 /* Read upper-half memory */
 assert(max_read != 0);
 assert(max_read == IOBUFFER_MAXREAD(self,rpos));
 assert(bufend   == IOBUFFER_BUFEND(self));
 max_read_linear = MIN(max_read,(size_t)(bufend-rpos));
 max_read_linear = MIN(max_read_linear,destsize);
 if (!(mode&IO_SKIP) && copy_to_user(destbuf,rpos,max_read_linear))
     goto err_fault;
 rpos   += max_read_linear;
 result += max_read_linear;
 if (destsize == max_read_linear ||
    (rpos == self->ib_wpos && max_read_linear == max_read)) {
  assertf(result != 0 || bufsize == 0,
          "begin    = %p\n"
          "rpos     = %p\n"
          "wpos     = %p\n"
          "end      = %p\n"
          "max_read = %Iu\n",
          self->ib_buffer,
          rpos,self->ib_wpos,
          bufend,max_read);
  goto end_rpos;
 }
 assertf(rpos == bufend,
         "rpos   = %p\n"
         "wpos   = %p\n"
         "buffer = %p\n"
         "bufend = %p\n",
         rpos,self->ib_wpos,
         self->ib_buffer,bufend);
 rpos      = self->ib_buffer;
 destsize -= max_read_linear;
 destbuf  += max_read_linear;
 /* Read lower-half memory */
 max_read_linear = MIN((size_t)(self->ib_wpos-rpos),destsize);
 if (!(mode&IO_SKIP) && copy_to_user(destbuf,rpos,max_read_linear))
     goto err_fault;
 rpos   += max_read_linear;
 result += max_read_linear;
 if (destsize == max_read_linear) {
  assert(result != 0 || bufsize == 0);
  goto end_rpos;
 }
 /* All available memory was read */
 assertf(rpos == self->ib_wpos,
         "rpos   = %p\n"
         "wpos   = %p\n"
         "buffer = %p\n"
         "bufend = %p\n",
         rpos,self->ib_wpos,
         self->ib_buffer,bufend);
 destsize -= max_read_linear;
 destbuf += max_read_linear;
 assert(result);
 goto buffer_is_empty;
end_rpos:
 //(void)(bufend = bufend); /* It is always initialized! */
 assert(result != 0 || bufsize == 0 || !(mode&IO_BLOCKFIRST));
 assert(bufend == self->ib_buffer+self->ib_size);
 assert(start_rpos >= self->ib_buffer && start_rpos <= bufend);
 assert(rpos >= self->ib_buffer && rpos <= bufend);
 if (!(mode&IO_PEEK)) {
  int buffer_was_full;
  buffer_was_full = (start_rpos == self->ib_wpos ||
                    (start_rpos == self->ib_buffer &&
                     self->ib_wpos == bufend));
  if (rpos == self->ib_wpos) {
   /* Special case: The buffer is now empty, but if we
    * would simply set the r-pointer like usual, the buffer
    * would look like it was full.
    * >> Instead, we must upgrade our lock to get write-access,
    *    and then continue to setup the buffer as follow:
    * BEFORE:               v r/w-pos
    *   ====================|======
    *
    * AFTER: r-pos (out-of-bounds) v
    *   |==========================|
    *   ^ w-pos
    */
   error = rwlock_upgrade(&self->ib_rwlock);
   if (E_ISERR(error) && error != -ERELOAD) goto end_always;
   COMPILER_BARRIER();
   /* Make sure our original start r-pos is still valid.
    * Also make sure no other was performed a write, thus making this just a regular case. */
   DEBUG_TRACE(syslog(LOG_DEBUG,"BUFFER BECAME EMPTY\n"));
   /* NOTE: the second check might fail if some other task quickly performed a read/write. */
   if __unlikely(error == -ERELOAD &&
                (self->ib_rpos != start_rpos || rpos != self->ib_wpos)) {
    /* Some other task already performed a read. */
    rwlock_endwrite(&self->ib_rwlock);
    DEBUG_TRACE(syslog(LOG_DEBUG,"Some other task already performed a read.\n"));
    goto again_full;
   }
   assert(bufend == self->ib_buffer+self->ib_size);
   /* If the buffer was full, we'll have to wake writers later! */

   /* Finally... */
   self->ib_wpos = self->ib_buffer;
   self->ib_rpos = bufend;
   COMPILER_WRITE_BARRIER();

   if (buffer_was_full) {
    DEBUG_TRACE(syslog(LOG_DEBUG,"FULL BUFFER BECAME EMPTY\n"));
    /* Wake writers if the buffer used to be full */
    rwlock_downgrade(&self->ib_rwlock);
    goto wake_writers;
   }
   rwlock_endwrite(&self->ib_rwlock);
   error = (ssize_t)result;
   goto end_always;
  }
  /* Overwrite the read-position, if we're the fastest in doing so.
   * NOTE: Due to the fact that were only holding a read-lock,
   *       some other task may have been faster than us and
   *       already read the same data. */
  if (!ATOMIC_CMPXCH(self->ib_rpos,start_rpos,rpos) &&
      !(mode&IO_QUICKMOVE)) goto again_full;
  /* Wake writers if the buffer used to be full */
  if (buffer_was_full) {
wake_writers:
   DEBUG_TRACE(syslog(LOG_DEBUG,"Waking writers..."));
   sig_broadcast(&self->ib_nfull);
   DEBUG_TRACE(syslog(LOG_DEBUG," (OK)\n"));
  }
 }
 error = (ssize_t)result;
end_read:   rwlock_endread(&self->ib_rwlock);
end_always: assert(error != 0 || bufsize == 0 || !(mode&IO_BLOCKFIRST));
end_always2:return error;
err_fault:  error = -EFAULT; goto end_read;
}


PUBLIC ssize_t KCALL
iobuffer_write(struct iobuffer *__restrict self,
               USER void const *buf, size_t bufsize,
               iomode_t mode) {
 size_t result,max_size;
 size_t max_write;
 ssize_t error; byte_t *bufend;
 CHECK_HOST_DOBJ(self);
 result = 0;
again:
 error = rwlock_write(&self->ib_rwlock);
 if (E_ISERR(error)) goto end_always;
 assert(self->ib_rpos >= self->ib_buffer && self->ib_rpos <= IOBUFFER_BUFEND(self));
 assert(self->ib_wpos >= self->ib_buffer && self->ib_wpos <= IOBUFFER_BUFEND(self));
 max_size  = iobuffer_get_maxsize(self);
 max_write = IOBUFFER_MAXWRITE(self,self->ib_rpos);
 if (!max_write) {
buffer_is_full:
  if ((mode&IO_BLOCKFIRST) && self->ib_size >= max_size) {
   /* Don't wait if we've already read something and are
    * only supposed to block for the first chunk of data. */
   if (result && (mode&(IO_BLOCKFIRST|IO_BLOCKALL)) == IO_BLOCKFIRST) goto end;
   /* Wake readers if we did write something before having to start waiting. */
   if (result) sig_broadcast(&self->ib_avail);
   //printf("WAIT ON FULL BUFFER (r:%p w:%p b:%p e:%p)\n",
   //       self->ib_rpos,self->ib_wpos,self->ib_buffer,
   //       self->ib_buffer+self->ib_size);
   /* Wait until at there is at least something to read
    * NOTE: The following code essentially performs what a
    *       condition variable calls its wait-operation. */
   sig_write(&self->ib_nfull);
   assert(!IOBUFFER_MAXWRITE(self,self->ib_rpos));
   rwlock_endwrite(&self->ib_rwlock);
   error = sig_recv_endwrite(&self->ib_nfull);
   DEBUG_TRACE(syslog(LOG_DEBUG,"Writer awoke\n"));
   if (E_ISERR(error)) goto end_always;
   goto again;
  } /*else goto end;*/
 }
 bufend = IOBUFFER_BUFEND(self);
 if (max_write < bufsize) {
  byte_t *new_buffer;
  size_t read_pos,write_pos,new_buffer_size;
  /* Difficult case: Check if we can reallocate the buffer to be large
   *                 enough, while respecting the buffer's 'max_size'. */
  read_pos  = (size_t)(self->ib_rpos-self->ib_buffer);
  write_pos = (size_t)(self->ib_wpos-self->ib_buffer);
  new_buffer_size = self->ib_size+(bufsize-max_write);
  new_buffer_size = MIN(new_buffer_size,max_size);
  //syslog(LOG_DEBUG,"[IOBUF] Realloc for %Iu bytes (%Iu; %Iu)\n",bufsize,new_buffer_size,max_size);
  assert(new_buffer_size >= self->ib_size);
  assert(new_buffer_size <= max_size);
  if (new_buffer_size != self->ib_size) {
   new_buffer = (byte_t *)krealloc(self->ib_buffer,new_buffer_size,
                                   GFP_SHARED|GFP_CALLOC);
   if __unlikely(!new_buffer) { error = -ENOMEM; goto end_write; }
   self->ib_buffer = new_buffer;
   /* Be careful to preserve the an empty-buffer state.
    * NOTE: The check against the old 'self->ib_size' is required to
    *       prevent an incorrect empty-buffer state from being detected
    *       when this is the first time the I/O-buffer is written to. */
   self->ib_rpos = new_buffer+((self->ib_rpos == bufend && __likely(self->ib_size)) ? new_buffer_size : read_pos);
   self->ib_wpos = new_buffer+write_pos;
   self->ib_size = new_buffer_size;
   bufend = new_buffer+new_buffer_size;
  } else if (!max_write) {
   goto end;
  }
 }
 assert(!bufsize || self->ib_buffer != bufend);
 /* Copy into the upper portion */
 max_write = (size_t)(bufend-self->ib_wpos);
 if (self->ib_rpos > self->ib_wpos) {
  /* Make sure not to overwrite unread buffer space.
   * This check is especially necessary if the reader is slow. */
  max_write = MIN(max_write,(size_t)(self->ib_rpos-self->ib_wpos));
 }
 max_write = MIN(max_write,bufsize);
 //printf("Upper copy: %Iu\n",max_write);
 if (copy_from_user(self->ib_wpos,buf,max_write))
     goto err_fault;
 self->ib_wpos += max_write;
 result += max_write;
 assert(max_write <= bufsize);
 if (bufsize == max_write) {
  //if (self->ib_wpos == bufend)
  // self->ib_wpos = self->ib_buffer;
  goto end;
 }
 *(uintptr_t *)&buf += max_write;
 bufsize -= max_write;
 assert(bufsize);
 assert((self->ib_wpos == bufend) ||
        (self->ib_wpos == self->ib_rpos));
 if (self->ib_wpos != bufend) {
  max_write = 0;
  goto buffer_is_full;
 }
 /* Copy into the lower portion */
 max_write = (size_t)(self->ib_rpos-self->ib_buffer);
 if (!max_write) {
  /* Special case: We can't wrap the buffer because
   *               that would indicate an empty ring. */
  goto buffer_is_full;
 }
 max_write = MIN(max_write,bufsize);
 if (copy_from_user(self->ib_buffer,buf,max_write))
     goto err_fault;
 self->ib_wpos = self->ib_buffer+max_write;
 result += max_write;
 assert(max_write <= bufsize);
 assert(self->ib_wpos <= self->ib_rpos);
 assert(self->ib_wpos < bufend ||
       (self->ib_wpos == bufend &&
        self->ib_rpos == bufend));
 if (bufsize == max_write) goto end;
 assert(self->ib_wpos == self->ib_rpos);
 *(uintptr_t *)&buf += max_write;
 bufsize -= max_write;
 /* Unwritten buffer data must still be available */
 assert(bufsize);
 goto buffer_is_full;
end:
 error = (ssize_t)result;
 if (result) {
  assert(bufend == IOBUFFER_BUFEND(self));
  assert(self->ib_rpos <= bufend);
  if (self->ib_rpos == bufend
#if 0
      /* Technically, we'd only need to set the r-pointer if
       * the following was true, but since there is no harm in
       * always wrapping it when it is out-of-bounds, we do so anyways.
       * NOTE: The condition would assert the w-ptr at start condition,
       *       as defined as part of the empty-buffer state, in the
       *       description in 'iobuffer_read'. */
   && self->ib_wpos == self->ib_buffer
#endif
   ) {
   /* Prevent the buffer from appearing as though it was empty */
   self->ib_rpos = self->ib_buffer;
   DEBUG_TRACE(syslog(LOG_DEBUG,"FIX READ POINTER\n"));
  }
  DEBUG_TRACE(syslog(LOG_DEBUG,"BROADCAST_DATA_AVAILABLE\n"));
  /* Signal that data has become available */
  sig_broadcast(&self->ib_avail);
 }
end_write:  rwlock_endwrite(&self->ib_rwlock);
end_always: return error;
err_fault:  error = -EFAULT; goto end_write;
}




PUBLIC ssize_t KCALL
iobuffer_unread(struct iobuffer *__restrict self,
                size_t max_unread) {
 /* Since this function could be used to access random, freed kernel memory,
  * I/O buffers will ZERO out new buffer memory after allocation. */
 size_t max_writesize; ssize_t result;
 CHECK_HOST_DOBJ(self);
 result = rwlock_write(&self->ib_rwlock);
 if unlikely(E_ISERR(result)) goto end;
 max_writesize = IOBUFFER_MAXWRITE(self,self->ib_rpos);
 /* Clamp the unread size to what is actually possible. */
 max_unread = MIN(max_unread,max_writesize);
 if ((result = (ssize_t)max_unread) != 0) {
  assert(self->ib_rpos != self->ib_wpos);
  if (self->ib_rpos > self->ib_wpos) {
   /* Unread upper memory. */
   self->ib_rpos -= max_unread;
   assert(self->ib_rpos >= self->ib_wpos);
  } else {
   /* Figure out how much buffer is available between begin & wpos */
   size_t partsize = (size_t)(self->ib_rpos-self->ib_buffer);
   if (partsize <= max_unread) {
    /* The entire shift takes place in lower memory. */
    self->ib_rpos -= max_unread;
    assert(self->ib_rpos <= self->ib_wpos);
   } else {
    /* The shift is destined to put the r-pointer
     * above the write-position in upper memory. */
    max_unread -= partsize;
    self->ib_rpos = IOBUFFER_BUFEND(self)-max_unread;
    assert(self->ib_rpos >= self->ib_wpos);
   }
  }
  assert(self->ib_rpos >= self->ib_buffer);
  /* Sanity: If all was unread, the read pointer
   *         must match the write pointer. */
  assert((max_unread == max_writesize) ==
         (self->ib_rpos == self->ib_wpos));
 }
end:
 return result;
}

PUBLIC ssize_t KCALL
iobuffer_unwrite(struct iobuffer *__restrict self,
                 size_t max_unwrite) {
 ssize_t result; size_t max_readsize;
 CHECK_HOST_DOBJ(self);
 result = rwlock_write(&self->ib_rwlock);
 if (E_ISERR(result)) goto end_always;
 max_readsize = IOBUFFER_MAXREAD(self,self->ib_rpos);
 /* Clamp the unwrite size to what is actually possible. */
 max_unwrite = MIN(max_unwrite,max_readsize);
 if ((result = (ssize_t)max_unwrite) != 0) {
  if (max_unwrite == max_readsize) {
   /* Special case: Discard everything. */
   self->ib_wpos = self->ib_buffer;
   self->ib_rpos = self->ib_buffer+self->ib_size;
  } else {
   /* Normal case: Discard part of the written buffer. */
   if (self->ib_rpos < self->ib_wpos) {
    self->ib_wpos -= max_unwrite;
    assert(self->ib_wpos > self->ib_rpos);
   } else {
    /* Figure out how much buffer is available between begin & wpos */
    size_t partsize = (size_t)(self->ib_wpos-self->ib_buffer);
    if (partsize <= max_unwrite) {
     /* The entire shift takes place in lower memory. */
     self->ib_wpos -= max_unwrite;
     assert(self->ib_wpos < self->ib_rpos);
    } else {
     /* The shift is destined to put the w-pointer
      * above the read-position in upper memory. */
     max_unwrite -= partsize;
     assert(max_unwrite);
     self->ib_wpos = IOBUFFER_BUFEND(self)-max_unwrite;
     assert(self->ib_wpos > self->ib_rpos);
    }
    assert(self->ib_wpos >= self->ib_buffer);
   }
  }
 }
 rwlock_endwrite(&self->ib_rwlock);
end_always:
 return result;
}

DECL_END

#ifndef __INTELLISENSE__
#include "canonbuffer.c.inl"
#include "atomic-iobuffer.c.inl"
#endif

#endif /* !GUARD_KERNEL_FS_IOBUFFER_C */
