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
#ifndef GUARD_INCLUDE_SYNC_OWNER_OWNER_RWLOCK_H
#define GUARD_INCLUDE_SYNC_OWNER_OWNER_RWLOCK_H 1

#include <assert.h>
#include <errno.h>
#include <hybrid/atomic.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-owner-rwlock.h>
#include <hybrid/sched/yield.h>
#include <hybrid/types.h>
#include <sync/sig.h>
#include <sched/percpu.h>
#include <stdbool.h>

DECL_BEGIN

typedef struct owner_rwlock {
 /* Similar to a regular rwlock, but allow for owner write-recursion: */
 atomic_owner_rwlock_t orw_lock;  /*< The underlying owner-lock used for implemented this. */
 struct sig            orw_sig;   /*< Signal used for synchronization. */
} owner_rwlock_t;

#define OWNER_RWLOCK_INIT          {ATOMIC_OWNER_RWLOCK_INIT,SIG_INIT}
#define owner_rwlock_cinit(self)   (atomic_owner_rwlock_cinit(&(self)->orw_lock),sig_cinit(&(self)->orw_sig))
#define owner_rwlock_init(self)    (atomic_owner_rwlock_init(&(self)->orw_lock),sig_init(&(self)->orw_sig))
#define DEFINE_OWNER_RWLOCK(name)   owner_rwlock_t name = OWNER_RWLOCK_INIT

#define owner_rwlock_reading(x)    atomic_owner_rwlock_reading(&(x)->orw_lock)
#define owner_rwlock_writing(x)    atomic_owner_rwlock_writing(&(x)->orw_lock)
#define OWNER_RWLOCK_TRYREAD(x)    atomic_owner_rwlock_tryread(&(x)->orw_lock)
#define OWNER_RWLOCK_TRYWRITE(x)   atomic_owner_rwlock_trywrite(&(x)->orw_lock)
#define OWNER_RWLOCK_TRYWRITE_R(x) atomic_owner_rwlock_trywrite_r(&(x)->orw_lock)
#define OWNER_RWLOCK_TRYUPGRADE(x) atomic_owner_rwlock_tryupgrade(&(x)->orw_lock)


/* Acquire an exclusive read/write lock.
 * @return: -EOK:       Successfully upgraded the R/W-lock.
 * @return: -EAGAIN:    [owner_rwlock_try*] Failed to acquire a lock immediately.
 * @return: -EINTR:     [owner_rwlock_(read|write)] The calling thread was interrupted.
 * @return: -ETIMEDOUT: [owner_rwlock_timed(read|write)] The given abstime has expired.
 */
LOCAL errno_t KCALL owner_rwlock_timedread(owner_rwlock_t *__restrict self, struct timespec const *abstime);
LOCAL errno_t KCALL owner_rwlock_timedwrite(owner_rwlock_t *__restrict self, struct timespec const *abstime);
#ifdef __INTELLISENSE__
LOCAL errno_t KCALL owner_rwlock_tryread(owner_rwlock_t *__restrict self);
LOCAL errno_t KCALL owner_rwlock_trywrite(owner_rwlock_t *__restrict self);
LOCAL errno_t KCALL owner_rwlock_read(owner_rwlock_t *__restrict self);
LOCAL errno_t KCALL owner_rwlock_write(owner_rwlock_t *__restrict self);
#else
#define owner_rwlock_tryread(self)  (atomic_owner_rwlock_tryread(&(self)->orw_lock) ? -EOK : -EAGAIN)
#define owner_rwlock_trywrite(self) (atomic_owner_rwlock_trywrite(&(self)->orw_lock) ? -EOK : -EAGAIN)
#define owner_rwlock_read(self)      owner_rwlock_timedread(self,NULL)
#define owner_rwlock_write(self)     owner_rwlock_timedwrite(self,NULL)
#endif

/* Try to upgrade a read-lock to a write-lock.
 * @return: -EOK:    Successfully upgraded the R/W-lock.
 * @return: -EAGAIN: Failed to acquire a lock immediately. */
#ifdef __INTELLISENSE__
LOCAL errno_t KCALL owner_rwlock_tryupgrade(owner_rwlock_t *__restrict self);
#else
#define owner_rwlock_tryupgrade(self) (atomic_owner_rwlock_tryupgrade(&(self)->orw_lock) ? -EOK : -EAGAIN)
#endif

/* Upgrade a read-lock to a write-lock.
 * @return: -EOK:     Successfully upgraded the R/W-lock.
 * @return: -EINTR:   The calling thread was interrupted (WARNING: The read-lock was lost).
 * @return: -ERELOAD: No lock may have been held temporarily,
 *                    meaning that the caller should reload
 *                    local copies of affected resources.
 * @return: -ETIMEDOUT: [owner_rwlock_timedupgrade] The given abstime has expired (WARNING: The read-lock was lost).
 */
LOCAL errno_t KCALL owner_rwlock_timedupgrade(owner_rwlock_t *__restrict self, struct timespec const *abstime);
#ifdef __INTELLISENSE__
LOCAL errno_t KCALL owner_rwlock_upgrade(owner_rwlock_t *__restrict self);
#else
#define owner_rwlock_upgrade(self) owner_rwlock_timedupgrade(self,NULL)
#endif

/* Downgrade a write-lock to a read-lock (Always succeeds). */
#ifdef __INTELLISENSE__
LOCAL void KCALL owner_rwlock_downgrade(owner_rwlock_t *__restrict self);
#else
#define owner_rwlock_downgrade(self) atomic_owner_rwlock_downgrade(&(self)->orw_lock)
#endif

/* End reading/writing/either. */
LOCAL void KCALL owner_rwlock_endwrite(owner_rwlock_t *__restrict self);
LOCAL void KCALL owner_rwlock_endread(owner_rwlock_t *__restrict self);
LOCAL void KCALL owner_rwlock_end(owner_rwlock_t *__restrict self);





LOCAL void KCALL owner_rwlock_endwrite(owner_rwlock_t *__restrict self) {
 u32 f,newval;
 assert(TASK_ISSAFE());
 __assertf(self->orw_lock.aorw_owner == THREADID_SELF(),"You're not the owner!");
 do {
  f = ATOMIC_READ(self->orw_lock.aorw_lock);
  __assertf(f&ATOMIC_OWNER_RWLOCK_WFLAG,"Lock isn't in write-mode (%x)",f);
  newval = f-1;
  if (!(f&ATOMIC_OWNER_RWLOCK_NMASK)) {
#ifndef NDEBUG
   self->orw_lock.aorw_owner = THREADID_INVALID;
#endif
   newval = 0;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->orw_lock.aorw_lock,f,newval));
 if (!newval) sig_broadcast(&self->orw_sig);
}
LOCAL void KCALL owner_rwlock_endread(owner_rwlock_t *__restrict self) {
#ifdef NDEBUG
 assert(TASK_ISSAFE());
 if (ATOMIC_FETCHDEC(self->orw_lock.aorw_lock) == 1)
     sig_broadcast(&self->orw_sig);
#else
 u32 f;
 assert(TASK_ISSAFE());
 do {
  f = ATOMIC_READ(self->orw_lock.aorw_lock);
  __assertf(f != 0,"Lock isn't held by anyone");
  if (f&ATOMIC_OWNER_RWLOCK_WFLAG) {
   __assertf(self->orw_lock.aorw_owner == THREADID_SELF(),
             "You're not the owner!");
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->orw_lock.aorw_lock,f,f-1));
 if (f == 1) sig_broadcast(&self->orw_sig);
#endif
}
LOCAL void KCALL owner_rwlock_end(owner_rwlock_t *__restrict self) {
 u32 f,newval;
 assert(TASK_ISSAFE());
 do {
  f = ATOMIC_READ(self->orw_lock.aorw_lock);
  __assertf(f != 0,"Lock isn't held by anyone");
  newval = f-1;
  if (f&ATOMIC_OWNER_RWLOCK_WFLAG) {
#ifndef NDEBUG
   __assertf(self->orw_lock.aorw_owner == THREADID_SELF() ||
             self->orw_lock.aorw_owner == THREADID_INVALID,
             "Lock is in write-mode (%x)",f);
#else
   __assertf(self->orw_lock.aorw_owner == THREADID_SELF(),
             "Lock is in write-mode (%x)",f);
#endif
   if (!(f&ATOMIC_OWNER_RWLOCK_NMASK)) {
#ifndef NDEBUG
    self->orw_lock.aorw_owner = THREADID_INVALID;
#endif
    newval = 0;
   }
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->orw_lock.aorw_lock,f,newval));
 if (!newval) sig_broadcast(&self->orw_sig);
}

LOCAL errno_t KCALL
owner_rwlock_timedread(owner_rwlock_t *__restrict self,
                       struct timespec const *abstime) {
 errno_t error = -EOK;
 while (!OWNER_RWLOCK_TRYREAD(self)) {
  sig_read(&self->orw_sig);
  if (OWNER_RWLOCK_TRYREAD(self)) { sig_endread(&self->orw_sig); break; }
  if (!sig_upgrade(&self->orw_sig) && OWNER_RWLOCK_TRYREAD(self)) { sig_endwrite(&self->orw_sig); break; }
  error = sig_timedrecv_endwrite(&self->orw_sig,abstime);
  if (E_ISERR(error)) break;
 }
 return error;
}
LOCAL errno_t KCALL
owner_rwlock_timedwrite(owner_rwlock_t *__restrict self,
                        struct timespec const *abstime) {
 errno_t error = -EOK;
 while (!OWNER_RWLOCK_TRYWRITE(self)) {
  sig_read(&self->orw_sig);
  if (OWNER_RWLOCK_TRYWRITE_R(self)) { sig_endread(&self->orw_sig); break; }
  if (!sig_upgrade(&self->orw_sig) && OWNER_RWLOCK_TRYWRITE_R(self)) { sig_endwrite(&self->orw_sig); break; }
  error = sig_timedrecv_endwrite(&self->orw_sig,abstime);
  if (E_ISERR(error)) break;
 }
 return error;
}
LOCAL errno_t KCALL
owner_rwlock_timedupgrade(owner_rwlock_t *__restrict self,
                          struct timespec const *abstime) {
 errno_t error = -EOK;
 if (!OWNER_RWLOCK_TRYUPGRADE(self)) {
  owner_rwlock_endread(self);
  error = owner_rwlock_timedwrite(self,abstime);
  if (E_ISOK(error)) error = -ERELOAD;
 }
 return error;
}

DECL_END

#endif /* !GUARD_INCLUDE_SYNC_OWNER_OWNER_RWLOCK_H */
