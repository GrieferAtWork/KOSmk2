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
#ifndef __GUARD_HYBRID_SYNC_ATOMIC_RWLOCK_H
#define __GUARD_HYBRID_SYNC_ATOMIC_RWLOCK_H 1

#include <hybrid/compiler.h>
#include <stdbool.h>
#include <assert.h>
#include <hybrid/atomic.h>
#include <hybrid/types.h>
#include <hybrid/critical.h>
#include <hybrid/sched/yield.h>

DECL_BEGIN

#define ATOMIC_RWLOCK_RMASK 0x7fffffff
#define ATOMIC_RWLOCK_WFLAG 0x80000000
#define ATOMIC_RWLOCK_SIZE  4

#ifdef __CC__
typedef struct atomic_rwlock {
 u32 arw_lock;
} atomic_rwlock_t;

#define ATOMIC_RWLOCK_INIT         {0}
#define atomic_rwlock_cinit(self)  (void)assert((self)->arw_lock == 0)
#define atomic_rwlock_init(self)   (void)((self)->arw_lock = 0)
#define DEFINE_ATOMIC_RWLOCK(name)  atomic_rwlock_t name = ATOMIC_RWLOCK_INIT

#define atomic_rwlock_reading(x)   (ATOMIC_READ((x)->arw_lock) != 0)
#define atomic_rwlock_writing(x)   (ATOMIC_READ((x)->arw_lock)&ATOMIC_RWLOCK_WFLAG)

/* Acquire an exclusive read/write lock. */
LOCAL SAFE bool KCALL atomic_rwlock_tryread(atomic_rwlock_t *__restrict self);
LOCAL SAFE bool KCALL atomic_rwlock_trywrite(atomic_rwlock_t *__restrict self);
LOCAL SAFE void KCALL atomic_rwlock_read(atomic_rwlock_t *__restrict self);
LOCAL SAFE void KCALL atomic_rwlock_write(atomic_rwlock_t *__restrict self);

/* Try to upgrade a read-lock to a write-lock. Return 'FALSE' upon failure. */
LOCAL SAFE bool KCALL atomic_rwlock_tryupgrade(atomic_rwlock_t *__restrict self);

/* NOTE: The lock is always upgraded, but when 'FALSE' is returned, no lock
 *       may have been held temporarily, meaning that the caller should
 *       re-load local copies of affected resources. */
LOCAL SAFE bool KCALL atomic_rwlock_upgrade(atomic_rwlock_t *__restrict self);

/* Downgrade a write-lock to a read-lock (Always succeeds). */
LOCAL SAFE void KCALL atomic_rwlock_downgrade(atomic_rwlock_t *__restrict self);

/* End reading/writing/either.
 * @return: true:  The lock has become free.
 * @return: false: The lock is still held by something. */
LOCAL SAFE void KCALL atomic_rwlock_endwrite(atomic_rwlock_t *__restrict self);
LOCAL SAFE bool KCALL atomic_rwlock_endread(atomic_rwlock_t *__restrict self);
LOCAL SAFE bool KCALL atomic_rwlock_end(atomic_rwlock_t *__restrict self);





LOCAL SAFE void (KCALL atomic_rwlock_endwrite)(atomic_rwlock_t *__restrict self) {
#ifdef NDEBUG
 assert(TASK_ISSAFE());
 ATOMIC_STORE(self->arw_lock,0);
#else
 u32 f;
 assert(TASK_ISSAFE());
 do {
  f = ATOMIC_READ(self->arw_lock);
  __assertf(f&ATOMIC_RWLOCK_WFLAG,"Lock isn't in write-mode (%x)",f);
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,f,0));
#endif
}
LOCAL SAFE bool (KCALL atomic_rwlock_endread)(atomic_rwlock_t *__restrict self) {
#ifdef NDEBUG
 assert(TASK_ISSAFE());
 return ATOMIC_DECFETCH(self->arw_lock) == 0;
#else
 u32 f;
 assert(TASK_ISSAFE());
 do {
  f = ATOMIC_READ(self->arw_lock);
  __assertf(!(f&ATOMIC_RWLOCK_WFLAG),"Lock is in write-mode (%x)",f);
  __assertf(f != 0,"Lock isn't held by anyone");
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,f,f-1));
 return f == 1;
#endif
}
LOCAL SAFE bool (KCALL atomic_rwlock_end)(atomic_rwlock_t *__restrict self) {
 u32 temp,newval;
 assert(TASK_ISSAFE());
 do {
  temp = ATOMIC_READ(self->arw_lock);
  if (temp&ATOMIC_RWLOCK_WFLAG) {
   assert(!(temp&ATOMIC_RWLOCK_RMASK));
   newval = 0;
  } else {
   __assertf(temp != 0,"No remaining read-locks");
   newval = temp-1;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,temp,newval));
 return newval == 0;
}
LOCAL SAFE bool (KCALL atomic_rwlock_tryread)(atomic_rwlock_t *__restrict self) {
 u32 temp;
 assert(TASK_ISSAFE());
 do {
  temp = ATOMIC_READ(self->arw_lock);
  if (temp&ATOMIC_RWLOCK_WFLAG) return false;
  assert((temp&ATOMIC_RWLOCK_RMASK) != ATOMIC_RWLOCK_RMASK);
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,temp,temp+1));
 return true;
}
LOCAL SAFE bool (KCALL atomic_rwlock_trywrite)(atomic_rwlock_t *__restrict self) {
 assert(TASK_ISSAFE());
 return ATOMIC_CMPXCH(self->arw_lock,0,ATOMIC_RWLOCK_WFLAG);
}
LOCAL SAFE void (KCALL atomic_rwlock_read)(atomic_rwlock_t *__restrict self) {
 while (!atomic_rwlock_tryread(self)) SCHED_YIELD();
}
LOCAL SAFE void (KCALL atomic_rwlock_write)(atomic_rwlock_t *__restrict self) {
 while (!atomic_rwlock_trywrite(self)) SCHED_YIELD();
}
LOCAL SAFE bool (KCALL atomic_rwlock_tryupgrade)(atomic_rwlock_t *__restrict self) {
 u32 temp;
 assert(TASK_ISSAFE());
 do {
  temp = ATOMIC_READ(self->arw_lock);
  if (temp != 1) return false;
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,temp,ATOMIC_RWLOCK_WFLAG));
 return true;
}
LOCAL SAFE bool (KCALL atomic_rwlock_upgrade)(atomic_rwlock_t *__restrict self) {
 if (atomic_rwlock_tryupgrade(self)) return true;
 atomic_rwlock_endread(self);
 atomic_rwlock_write(self);
 return false;
}
LOCAL SAFE void (KCALL atomic_rwlock_downgrade)(atomic_rwlock_t *__restrict self) {
#ifdef NDEBUG
 assert(TASK_ISSAFE());
 ATOMIC_WRITE(self->arw_lock,1);
#else
 u32 f;
 assert(TASK_ISSAFE());
 do {
  f = ATOMIC_READ(self->arw_lock);
  __assertf(f == ATOMIC_RWLOCK_WFLAG,"Lock not in write-mode (%x)",f);
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,f,1));
#endif
}
#endif /* __CC__ */

#if !defined(__INTELLISENSE__) && !defined(__NO_expect)
#define atomic_rwlock_tryread(self)    __expect(atomic_rwlock_tryread(self),true)
#define atomic_rwlock_trywrite(self)   __expect(atomic_rwlock_trywrite(self),true)
#define atomic_rwlock_tryupgrade(self) __expect(atomic_rwlock_tryupgrade(self),true)
#define atomic_rwlock_upgrade(self)    __expect(atomic_rwlock_upgrade(self),true)
#endif


DECL_END

#endif /* !__GUARD_HYBRID_SYNC_ATOMIC_RWLOCK_H */
