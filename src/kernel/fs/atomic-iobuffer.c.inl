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
#ifndef GUARD_KERNEL_FS_ATOMIC_IOBUFFER_C_INL
#define GUARD_KERNEL_FS_ATOMIC_IOBUFFER_C_INL 1
#define _KOS_SOURCE 1

#include <fs/atomic-iobuffer.h>
#include <hybrid/atomic.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <sched/cpu.h>
#include <string.h>

DECL_BEGIN

PUBLIC ssize_t KCALL
atomic_iobuffer_read(struct atomic_iobuffer *__restrict self,
                     USER void *buf, size_t bufsiz, bool blocking) {
 size_t result = 0;
#ifdef CONFIG_SMP
 bool has_write_lock = false;
#endif
 CHECK_HOST_DOBJ(self);
 CHECK_USER_DATA(buf,bufsiz);
#ifdef CONFIG_SMP
 atomic_rwlock_read(&self->aib_using);
#endif
 while (bufsiz) {
  pflag_t was; size_t bufavail,high_copy;
  byte_t *old_rpos,*old_apos;
  byte_t *new_rpos; errno_t error;
  do {
read_again:
   old_rpos = ATOMIC_READ(self->aib_rpos);
   old_apos = ATOMIC_READ(self->aib_apos);
   /* Check how much can be read. */
   bufavail = ATOMIC_IOBUFFER_MAXREAD(self,old_rpos,old_apos);
got_bufavail:
   if (bufavail > bufsiz)
       bufavail = bufsiz;
   if unlikely(!bufavail) {
    /* If at least ~something~ was read, or if we're
     * not supposed to block, stop prematurely. */
    if (result || !blocking) goto end;
#ifdef CONFIG_SMP
    if (has_write_lock) {
     has_write_lock = false;
     atomic_rwlock_downgrade(&self->aib_using);
    }
#endif
    /* Special case: Try to wait for the signal.
     * NOTE: Must disable preemption now, so-as to prevent
     *       an interrupt from dead-locking when it tries to
     *       write data while we're locking the avail-signal. */
    was = PREEMPTION_PUSH();
    sig_write(&self->aib_avail);
    old_rpos = ATOMIC_READ(self->aib_rpos);
    old_apos = ATOMIC_READ(self->aib_apos);
    bufavail = ATOMIC_IOBUFFER_MAXREAD(self,old_rpos,old_apos);
    if (bufavail) {
     sig_endwrite(&self->aib_avail);
     PREEMPTION_POP(was);
     goto got_bufavail;
    }
    error = sig_recv_endwrite(&self->aib_avail);
    PREEMPTION_POP(was);
    if (E_ISERR(error)) { result = (size_t)error; goto end; }
    goto read_again;
   }
   high_copy = (self->aib_buffer+ATOMIC_IOBUFFER_SIZE(self))-old_rpos;
   if (high_copy >= bufavail) {
    if (copy_to_user(buf,old_rpos,bufavail))
    { result = (size_t)-EFAULT; goto end; }
    new_rpos = old_rpos+bufavail;
   } else {
    size_t low_copy = bufavail-high_copy;
    if (copy_to_user(buf,old_rpos,high_copy) || (low_copy &&
        copy_to_user((void *)((uintptr_t)buf+high_copy),self->aib_buffer,low_copy)))
    { result = (size_t)-EFAULT; goto end; }
    new_rpos = self->aib_buffer+low_copy;
   }
   assert(new_rpos != self->aib_buffer);

   if (new_rpos == old_apos) {
    /* Special case: The buffer has become empty. */
    was = PREEMPTION_PUSH();
#ifdef CONFIG_SMP
    if (!has_write_lock) {
     has_write_lock = true;
     if (!atomic_rwlock_upgrade(&self->aib_using))
          goto read_again;
    }
#endif
    if (!ATOMIC_CMPXCH(self->aib_apos,old_apos,self->aib_buffer) ||
        !ATOMIC_CMPXCH(self->aib_wpos,old_apos,self->aib_buffer) ||
        !ATOMIC_CMPXCH(self->aib_rpos,old_rpos,
                       self->aib_buffer+ATOMIC_IOBUFFER_SIZE(self))) {
#ifdef CONFIG_SMP
     atomic_rwlock_downgrade(&self->aib_using);
     has_write_lock = false;
#endif
     PREEMPTION_POP(was);
     goto read_again;
    }
#ifdef CONFIG_SMP
    atomic_rwlock_downgrade(&self->aib_using);
    has_write_lock = false;
#endif
    PREEMPTION_POP(was);
    break;
   }
   /* Ensure that every piece of data is only read once by
    * atomically updating the read-pointer and re-reading data
    * in the event that another thread also read something. */
  } while (!ATOMIC_CMPXCH(self->aib_rpos,old_rpos,new_rpos));
  result += bufavail;
  bufsiz -= bufavail;
  *(uintptr_t *)&buf += bufavail;
 }
end:
#ifdef CONFIG_SMP
 if unlikely(has_write_lock)
      atomic_rwlock_endwrite(&self->aib_using);
 else atomic_rwlock_endread (&self->aib_using);
#endif
 return result;
}

PUBLIC size_t KCALL
atomic_iobuffer_kwrite(struct atomic_iobuffer *__restrict self,
                       HOST void const *__restrict buf, size_t bufsiz) {
 size_t result = 0;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_TEXT(buf,bufsiz);
#ifdef CONFIG_SMP
 atomic_rwlock_read(&self->aib_using);
#endif
 while (bufsiz) {
  byte_t *old_wpos,*old_rpos;
  byte_t *old_apos,*new_wpos;
  size_t bufavail,high_copy;
  bool must_signal = false;
  do {
   old_rpos = ATOMIC_READ(self->aib_rpos);
   old_wpos = ATOMIC_READ(self->aib_wpos);

   /* Check how much can be written. */
   bufavail = ATOMIC_IOBUFFER_MAXWRITE(self,old_rpos,old_wpos);
   if (!bufavail) goto done;
   if (bufavail > bufsiz)
       bufavail = bufsiz;
   high_copy = (size_t)((self->aib_buffer+ATOMIC_IOBUFFER_SIZE(self))-old_wpos);
   if (high_copy > bufavail)
       high_copy = bufavail;
   new_wpos = old_wpos+high_copy;
   /* Wrap the write-pointer around in case low-memory is written as well. */
   if (new_wpos > self->aib_buffer+ATOMIC_IOBUFFER_SIZE(self))
       new_wpos -= ATOMIC_IOBUFFER_SIZE(self);
   assert(new_wpos >= self->aib_buffer);
   assert(new_wpos <= self->aib_buffer+ATOMIC_IOBUFFER_SIZE(self));

   /* This cmpxch ensures that multiple writers write the correct
    * amount of data and tries to prevent data corruption.
    * >> Data corruption can still occurr due to race conditions,
    *    but the chance of this diminishes as buffer size increases. */
  } while (!ATOMIC_CMPXCH(self->aib_wpos,old_wpos,new_wpos));

  /* Actually write data. */
  result += bufavail;
  bufsiz -= bufavail;
  memcpy(old_wpos,buf,high_copy); /* Copy high memory. */
  *(uintptr_t *)&buf += high_copy;
  bufavail -= high_copy;
  memcpy(self->aib_buffer,buf,bufavail); /* Copy low memory. */
  *(uintptr_t *)&buf += bufavail;


  /* Lazily update the avail-pointer. */
  do {
   size_t old_avail;
   old_rpos  = ATOMIC_READ(self->aib_rpos);
   old_apos  = ATOMIC_READ(self->aib_apos);
   old_avail = ATOMIC_IOBUFFER_MAXREAD(self,old_rpos,old_apos);
   /* If the buffer was empty before, we must signal any potential readers waiting for data. */
   if (old_avail == 0) must_signal = true;
#if 0
   /* Don't update the avail-pointer if doing so would reduce the effectively available size.
    * >> In the event that it would, this probably means that some form of data curruption
    *    has occurred, but in the spirit of trying to keep data size as consistent as possible,
    *    we try not to corrupt things even more than they already are. */
   if (ATOMIC_IOBUFFER_MAXREAD(self,old_rpos,new_wpos) < old_avail)
       break;
#endif
  } while (!ATOMIC_CMPXCH(self->aib_apos,old_apos,new_wpos));

  /* Signal readers that data has become available. */
  if (must_signal)
      sig_broadcast(&self->aib_avail);
 }
done:
#ifdef CONFIG_SMP
 atomic_rwlock_endread(&self->aib_using);
#endif
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_FS_ATOMIC_IOBUFFER_C_INL */
