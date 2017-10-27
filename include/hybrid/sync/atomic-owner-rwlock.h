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
#ifndef __GUARD_HYBRID_SYNC_ATOMIC_OWNER_RWLOCK_H
#define __GUARD_HYBRID_SYNC_ATOMIC_OWNER_RWLOCK_H 1

#include <hybrid/compiler.h>
#include <stdbool.h>
#include <assert.h>
#include <hybrid/atomic.h>
#include <hybrid/types.h>
#include <hybrid/critical.h>
#include <hybrid/sched/yield.h>
#include <hybrid/sync/threadid.h>

DECL_BEGIN

#define ATOMIC_OWNER_RWLOCK_NMASK  0x7fffffff
#define ATOMIC_OWNER_RWLOCK_WFLAG  0x80000000 /*  */
#define ATOMIC_OWNER_RWLOCK_SIZE  (4+THREADID_SIZE)
typedef struct atomic_owner_rwlock {
 /* Similar to a regular rwlock, but allow for owner write-recursion: */
 u32        aorw_lock;  /*< The underlying synchronization atomic. */
 threadid_t aorw_owner; /*< [valid_if(ATOMIC_READ(self->aorw_lock)&ATOMIC_OWNER_RWLOCK_WFLAG)]
                         *  A unique identifier for the thread owning this lock. */
} atomic_owner_rwlock_t;

#define ATOMIC_OWNER_RWLOCK_INIT         {0,THREADID_INVALID}
#ifdef THREADID_INVALID_IS_ZERO
#define atomic_owner_rwlock_cinit(self)  (void)(assert((self)->aorw_lock == 0),assert((self)->aorw_owner == THREADID_INVALID))
#else
#define atomic_owner_rwlock_cinit(self)  (void)(assert((self)->aorw_lock == 0),(self)->aorw_owner = THREADID_INVALID)
#endif
#define atomic_owner_rwlock_init(self)   (void)((self)->aorw_lock = 0,(self)->aorw_owner = THREADID_INVALID)
#define DEFINE_ATOMIC_OWNER_RWLOCK(name)  atomic_owner_rwlock_t name = ATOMIC_OWNER_RWLOCK_INIT

#define atomic_owner_rwlock_reading(x)   (ATOMIC_READ((x)->aorw_lock) != 0)
#define atomic_owner_rwlock_writing(x)   (ATOMIC_READ((x)->aorw_lock)&ATOMIC_OWNER_RWLOCK_WFLAG && \
                                                      (x)->aorw_owner == THREADID_SELF())

/* Acquire an exclusive read/write lock. */
LOCAL SAFE bool KCALL atomic_owner_rwlock_tryread(atomic_owner_rwlock_t *__restrict self);
LOCAL SAFE bool KCALL atomic_owner_rwlock_trywrite(atomic_owner_rwlock_t *__restrict self);
LOCAL SAFE bool KCALL atomic_owner_rwlock_trywrite_r(atomic_owner_rwlock_t *__restrict self);
LOCAL SAFE void KCALL atomic_owner_rwlock_read(atomic_owner_rwlock_t *__restrict self);
LOCAL SAFE void KCALL atomic_owner_rwlock_write(atomic_owner_rwlock_t *__restrict self);

/* Try to upgrade a read-lock to a write-lock. Return `FALSE' upon failure. */
LOCAL SAFE bool KCALL atomic_owner_rwlock_tryupgrade(atomic_owner_rwlock_t *__restrict self);

/* NOTE: The lock is always upgraded, but when `FALSE' is returned, no lock
 *       may have been held temporarily, meaning that the caller should
 *       re-load local copies of affected resources. */
LOCAL SAFE bool KCALL atomic_owner_rwlock_upgrade(atomic_owner_rwlock_t *__restrict self);

/* Downgrade a write-lock to a read-lock (Always succeeds). */
LOCAL SAFE void KCALL atomic_owner_rwlock_downgrade(atomic_owner_rwlock_t *__restrict self);

/* End reading/writing/either. */
LOCAL SAFE void KCALL atomic_owner_rwlock_endwrite(atomic_owner_rwlock_t *__restrict self);
LOCAL SAFE void KCALL atomic_owner_rwlock_endread(atomic_owner_rwlock_t *__restrict self);
LOCAL SAFE void KCALL atomic_owner_rwlock_end(atomic_owner_rwlock_t *__restrict self);





LOCAL SAFE void KCALL atomic_owner_rwlock_endwrite(atomic_owner_rwlock_t *__restrict self) {
 u32 f;
 assert(TASK_ISSAFE());
 __assertf(self->aorw_owner == THREADID_SELF(),"You're not the owner!");
 do {
  f = ATOMIC_READ(self->aorw_lock);
  __assertf(f&ATOMIC_OWNER_RWLOCK_WFLAG,"Lock isn't in write-mode (%x)",f);
#ifndef NDEBUG
  if (!(f&ATOMIC_OWNER_RWLOCK_NMASK))
        self->aorw_owner = THREADID_INVALID;
#endif
 } while (!ATOMIC_CMPXCH_WEAK(self->aorw_lock,f,f&ATOMIC_OWNER_RWLOCK_NMASK ? f-1 : 0));
}
LOCAL SAFE void KCALL atomic_owner_rwlock_endread(atomic_owner_rwlock_t *__restrict self) {
#ifdef NDEBUG
 assert(TASK_ISSAFE());
 ATOMIC_FETCHDEC(self->aorw_lock);
#else
 u32 f;
 assert(TASK_ISSAFE());
 do {
  f = ATOMIC_READ(self->aorw_lock);
  __assertf(f != 0,"Lock isn't held by anyone");
  if (f&ATOMIC_OWNER_RWLOCK_WFLAG) {
   __assertf(f != ATOMIC_OWNER_RWLOCK_WFLAG,
             "No more read-locks available. - You probably meant to close a write-lock");
   __assertf(self->aorw_owner == THREADID_SELF(),
             "You're not the owner!");
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->aorw_lock,f,f-1));
#endif
}
LOCAL SAFE void KCALL atomic_owner_rwlock_end(atomic_owner_rwlock_t *__restrict self) {
 u32 f,newval;
 assert(TASK_ISSAFE());
 do {
  f = ATOMIC_READ(self->aorw_lock);
  __assertf(f != 0,"Lock isn't held by anyone");
  newval = f-1;
  if (f&ATOMIC_OWNER_RWLOCK_WFLAG) {
#ifndef NDEBUG
   __assertf(self->aorw_owner == THREADID_SELF() ||
             self->aorw_owner == THREADID_INVALID,
             "Lock is in write-mode (%x)",f);
#else
   __assertf(self->aorw_owner == THREADID_SELF(),
             "Lock is in write-mode (%x)",f);
#endif
   if (!(f&ATOMIC_OWNER_RWLOCK_NMASK)) {
#ifndef NDEBUG
    self->aorw_owner = THREADID_INVALID;
#endif
    newval = 0;
   }
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->aorw_lock,f,newval));
}
LOCAL SAFE bool KCALL atomic_owner_rwlock_tryread(atomic_owner_rwlock_t *__restrict self) {
 u32 temp;
 assert(TASK_ISSAFE());
 do {
  temp = ATOMIC_READ(self->aorw_lock);
  if (temp&ATOMIC_OWNER_RWLOCK_WFLAG &&
      self->aorw_owner != THREADID_SELF()) return false;
  assert((temp&ATOMIC_OWNER_RWLOCK_NMASK) != ATOMIC_OWNER_RWLOCK_NMASK);
 } while (!ATOMIC_CMPXCH_WEAK(self->aorw_lock,temp,temp+1));
 return true;
}
LOCAL SAFE bool KCALL atomic_owner_rwlock_trywrite(atomic_owner_rwlock_t *__restrict self) {
 u32 temp,newval;
 threadid_t tself = THREADID_SELF();
 assert(TASK_ISSAFE());
 do {
  temp = ATOMIC_READ(self->aorw_lock);
  if (temp&ATOMIC_OWNER_RWLOCK_WFLAG) {
   if (self->aorw_owner != tself) return false;
   newval = temp+1;
  } else {
   if (temp) return false;
   newval = ATOMIC_OWNER_RWLOCK_WFLAG;
  }
  assert(newval&ATOMIC_OWNER_RWLOCK_WFLAG);
 } while (!ATOMIC_CMPXCH_WEAK(self->aorw_lock,temp,newval));
 if (newval == ATOMIC_OWNER_RWLOCK_WFLAG)
     self->aorw_owner = tself;
 return true;
}
LOCAL SAFE bool KCALL atomic_owner_rwlock_trywrite_r(atomic_owner_rwlock_t *__restrict self) {
 u32 temp;
 assert(TASK_ISSAFE());
 do {
  temp = ATOMIC_READ(self->aorw_lock);
  assert(temp&ATOMIC_OWNER_RWLOCK_WFLAG ?
         self->aorw_owner != THREADID_SELF() : 1);
  if (temp) return false;
 } while (!ATOMIC_CMPXCH_WEAK(self->aorw_lock,temp,ATOMIC_OWNER_RWLOCK_WFLAG));
 self->aorw_owner = THREADID_SELF();
 return true;
}
LOCAL SAFE void KCALL atomic_owner_rwlock_read(atomic_owner_rwlock_t *__restrict self) {
 while (!atomic_owner_rwlock_tryread(self)) SCHED_YIELD();
}
LOCAL SAFE void KCALL atomic_owner_rwlock_write(atomic_owner_rwlock_t *__restrict self) {
 while (!atomic_owner_rwlock_trywrite(self)) SCHED_YIELD();
}
LOCAL SAFE bool KCALL atomic_owner_rwlock_tryupgrade(atomic_owner_rwlock_t *__restrict self) {
 u32 temp;
 assert(TASK_ISSAFE());
again:
 do {
  temp = ATOMIC_READ(self->aorw_lock);
  if (temp != 1) {
   if (temp&ATOMIC_OWNER_RWLOCK_WFLAG &&
       self->aorw_owner == THREADID_SELF()) {
    /* Do nothing when upgrading a read-lock ontop of a write-lock. */
    if (ATOMIC_CMPXCH_WEAK(self->aorw_lock,temp,temp)) return true;
    goto again;
   }
   return false;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->aorw_lock,temp,ATOMIC_OWNER_RWLOCK_WFLAG));
 self->aorw_owner = THREADID_SELF();
 return true;
}
LOCAL SAFE bool KCALL atomic_owner_rwlock_upgrade(atomic_owner_rwlock_t *__restrict self) {
 if (atomic_owner_rwlock_tryupgrade(self)) return true;
 atomic_owner_rwlock_endread(self);
 atomic_owner_rwlock_write(self);
 return false;
}
LOCAL SAFE void KCALL atomic_owner_rwlock_downgrade(atomic_owner_rwlock_t *__restrict self) {
#ifdef NDEBUG
 assert(TASK_ISSAFE());
 ATOMIC_WRITE(self->aorw_lock,1);
#else
 u32 f;
 assert(TASK_ISSAFE());
 __assertf(self->aorw_owner == THREADID_SELF(),"You're not the owner!");
 do {
  f = ATOMIC_READ(self->aorw_lock);
  __assertf(f&ATOMIC_OWNER_RWLOCK_WFLAG,"Lock not in write-mode (%x)",f);
  __assertf(!(f&ATOMIC_OWNER_RWLOCK_NMASK),"Lock has more than one writer (%x)",f);
 } while (!ATOMIC_CMPXCH_WEAK(self->aorw_lock,f,1));
#endif
}

DECL_END

#endif /* !__GUARD_HYBRID_SYNC_ATOMIC_OWNER_RWLOCK_H */
