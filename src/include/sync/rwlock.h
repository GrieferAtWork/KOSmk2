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
#ifndef GUARD_INCLUDE_SYNC_RWLOCK_H
#define GUARD_INCLUDE_SYNC_RWLOCK_H 1

#include <assert.h>
#include <errno.h>
#include <hybrid/atomic.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/sched/yield.h>
#include <hybrid/types.h>
#include <sync/sig.h>
#include <stdbool.h>

#ifdef __CC__
DECL_BEGIN

typedef struct rwlock {
 atomic_rwlock_t rw_lock;
 struct sig      rw_sig;
} rwlock_t;

#define RWLOCK_INIT          {ATOMIC_RWLOCK_INIT,SIG_INIT}
#define rwlock_cinit(self)   (atomic_rwlock_cinit(&(self)->rw_lock),sig_cinit(&(self)->rw_sig))
#define rwlock_init(self)    (atomic_rwlock_init(&(self)->rw_lock),sig_init(&(self)->rw_sig))
#define DEFINE_RWLOCK(name)   rwlock_t name = RWLOCK_INIT

#define rwlock_reading(x)    atomic_rwlock_reading(&(x)->rw_lock)
#define rwlock_writing(x)    atomic_rwlock_writing(&(x)->rw_lock)
#define RWLOCK_TRYREAD(x)    atomic_rwlock_tryread(&(x)->rw_lock)
#define RWLOCK_TRYWRITE(x)   atomic_rwlock_trywrite(&(x)->rw_lock)
#define RWLOCK_TRYUPGRADE(x) atomic_rwlock_tryupgrade(&(x)->rw_lock)


/* Acquire an exclusive read/write lock.
 * @return: -EOK:       Successfully upgraded the R/W-lock.
 * @return: -EAGAIN:    [rwlock_try*] Failed to acquire a lock immediately.
 * @return: -EINTR:     [rwlock_(read|write)] The calling thread was interrupted.
 * @return: -ETIMEDOUT: [rwlock_timed(read|write)] The given abstime has expired. */
LOCAL errno_t KCALL rwlock_timedread(rwlock_t *__restrict self, struct timespec const *abstime);
LOCAL errno_t KCALL rwlock_timedwrite(rwlock_t *__restrict self, struct timespec const *abstime);
#ifdef __INTELLISENSE__
LOCAL errno_t KCALL rwlock_tryread(rwlock_t *__restrict self);
LOCAL errno_t KCALL rwlock_trywrite(rwlock_t *__restrict self);
LOCAL errno_t KCALL rwlock_read(rwlock_t *__restrict self);
LOCAL errno_t KCALL rwlock_write(rwlock_t *__restrict self);
#else
#define rwlock_tryread(self)  (atomic_rwlock_tryread(&(self)->rw_lock) ? -EOK : -EAGAIN)
#define rwlock_trywrite(self) (atomic_rwlock_trywrite(&(self)->rw_lock) ? -EOK : -EAGAIN)
#define rwlock_read(self)      rwlock_timedread(self,NULL)
#define rwlock_write(self)     rwlock_timedwrite(self,NULL)
#endif

/* Try to upgrade a read-lock to a write-lock.
 * @return: -EOK:    Successfully upgraded the R/W-lock.
 * @return: -EAGAIN: Failed to acquire a lock immediately. */
#ifdef __INTELLISENSE__
LOCAL errno_t KCALL rwlock_tryupgrade(rwlock_t *__restrict self);
#else
#define rwlock_tryupgrade(self) (atomic_rwlock_tryupgrade(&(self)->rw_lock) ? -EOK : -EAGAIN)
#endif

/* Upgrade a read-lock to a write-lock.
 * @return: -EOK:     Successfully upgraded the R/W-lock.
 * @return: -EINTR:   The calling thread was interrupted (WARNING: The read-lock was lost).
 * @return: -ERELOAD: No lock may have been held temporarily,
 *                    meaning that the caller should reload
 *                    local copies of affected resources.
 * @return: -ETIMEDOUT: [rwlock_timedupgrade] The given abstime has expired (WARNING: The read-lock was lost).
 */
LOCAL errno_t KCALL rwlock_timedupgrade(rwlock_t *__restrict self, struct timespec const *abstime);
#ifdef __INTELLISENSE__
LOCAL errno_t KCALL rwlock_upgrade(rwlock_t *__restrict self);
#else
#define rwlock_upgrade(self) rwlock_timedupgrade(self,NULL)
#endif

/* Downgrade a write-lock to a read-lock (Always succeeds). */
#ifdef __INTELLISENSE__
LOCAL void KCALL rwlock_downgrade(rwlock_t *__restrict self);
#else
#define rwlock_downgrade(self) atomic_rwlock_downgrade(&(self)->rw_lock)
#endif

/* End reading/writing/either. */
LOCAL void KCALL rwlock_endwrite(rwlock_t *__restrict self);
LOCAL void KCALL rwlock_endread(rwlock_t *__restrict self);
LOCAL void KCALL rwlock_end(rwlock_t *__restrict self);





LOCAL void KCALL rwlock_endwrite(rwlock_t *__restrict self) {
 atomic_rwlock_endwrite(&self->rw_lock);
 sig_broadcast(&self->rw_sig);
}
LOCAL void KCALL rwlock_endread(rwlock_t *__restrict self) {
 u32 f;
#ifdef NDEBUG
 f = ATOMIC_FETCHDEC(self->rw_lock.arw_lock);
#else
 do {
  f = ATOMIC_READ(self->rw_lock.arw_lock);
  __assertf(!(f&ATOMIC_RWLOCK_WFLAG),"Lock is in write-mode (%x)",f);
  __assertf(f != 0,"Lock isn't held by anyone");
 } while (!ATOMIC_CMPXCH_WEAK(self->rw_lock.arw_lock,f,f-1));
#endif
 if (f == 1) sig_broadcast(&self->rw_sig);
}
LOCAL void KCALL rwlock_end(rwlock_t *__restrict self) {
 u32 temp,newval;
 do {
  temp = ATOMIC_READ(self->rw_lock.arw_lock);
  if (temp&ATOMIC_RWLOCK_WFLAG) {
   assert(!(temp&ATOMIC_RWLOCK_RMASK));
   newval = 0;
  } else {
   newval = temp-1;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->rw_lock.arw_lock,temp,newval));
 if (!newval) sig_broadcast(&self->rw_sig);
}

LOCAL errno_t KCALL
rwlock_timedread(rwlock_t *__restrict self,
                 struct timespec const *abstime) {
 errno_t error = -EOK;
 while (!RWLOCK_TRYREAD(self)) {
  sig_read(&self->rw_sig);
  if (RWLOCK_TRYREAD(self)) { sig_endread(&self->rw_sig); break; }
  if (!sig_upgrade(&self->rw_sig) && RWLOCK_TRYREAD(self)) { sig_endwrite(&self->rw_sig); break; }
  error = sig_timedrecv_endwrite(&self->rw_sig,abstime);
  if (E_ISERR(error)) break;
 }
 return error;
}
LOCAL errno_t KCALL
rwlock_timedwrite(rwlock_t *__restrict self,
                  struct timespec const *abstime) {
 errno_t error = -EOK;
 while (!RWLOCK_TRYWRITE(self)) {
  sig_read(&self->rw_sig);
  if (RWLOCK_TRYWRITE(self)) { sig_endread(&self->rw_sig); break; }
  if (!sig_upgrade(&self->rw_sig) && RWLOCK_TRYWRITE(self)) { sig_endwrite(&self->rw_sig); break; }
  error = sig_timedrecv_endwrite(&self->rw_sig,abstime);
  if (E_ISERR(error)) break;
 }
 return error;
}
LOCAL errno_t KCALL
rwlock_timedupgrade(rwlock_t *__restrict self,
                    struct timespec const *abstime) {
 errno_t error = -EOK;
 if (!RWLOCK_TRYUPGRADE(self)) {
  rwlock_endread(self);
  error = rwlock_timedwrite(self,abstime);
  if (E_ISOK(error)) error = -ERELOAD;
 }
 return error;
}

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_INCLUDE_SYNC_RWLOCK_H */
