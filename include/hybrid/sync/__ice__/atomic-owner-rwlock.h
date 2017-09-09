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
#ifndef GUARD_HYBRID_SYNC_ATOMIC_OWNER_RWLOCK_H
#define GUARD_HYBRID_SYNC_ATOMIC_OWNER_RWLOCK_H 1

#include <hybrid/compiler.h>
#include <stdbool.h>
#include <assert.h>
#include <hybrid/atomic.h>
#include <hybrid/types.h>
#include <hybrid/critical.h>
#include <hybrid/sched/yield.h>
#include <hybrid/sync/atomic-rwlock.h>

DECL_BEGIN

#if 0
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

/* Try to upgrade a read-lock to a write-lock. Return 'FALSE' upon failure. */
LOCAL SAFE bool KCALL atomic_owner_rwlock_tryupgrade(atomic_owner_rwlock_t *__restrict self);

/* NOTE: The lock is always upgraded, but when 'FALSE' is returned, no lock
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
  if (!(f&ATOMIC_OWNER_RWLOCK_NMASK))
        self->aorw_owner = THREADID_INVALID;
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
    self->aorw_owner = THREADID_INVALID;
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
#else


#define ATOMIC_OWNER_RWLOCK_TICKET_SIZE  8
typedef struct atomic_owner_rwlock_ticket {
 /* Atomic owner r/w lock ticket (must be allocated per-thread & per-owner-rw-lock) */
 u32 aorwt_read;  /*< [lock(PRIVATE(THIS_TASK))] The amount of read-locks held by this thread. */
 u32 aorwt_write; /*< [lock(PRIVATE(THIS_TASK))] The amount of write-locks held by this thread. */
} atomic_owner_rwlock_ticket_t;
#define ATOMIC_OWNER_RWLOCK_TICKET_INIT        {0,0}
#define atomic_owner_rwlock_ticket_cinit(self) (assert((self)->aorwt_read == 0),assert((self)->aorwt_write == 0))
#define atomic_owner_rwlock_ticket_init(self) (void)((self)->aorwt_read = (self)->aorwt_write = 0)

#define ATOMIC_OWNER_RWLOCK_SIZE  (ATOMIC_RWLOCK_SIZE+__SIZEOF_POINTER__)
typedef struct atomic_owner_rwlock {
 /* Similar to a regular rwlock, but allow for owner read/write-recursion. */
 atomic_rwlock_t               aorw_lock;  /*< The underlying synchronization lock. */
 atomic_owner_rwlock_ticket_t *aorw_owner; /*< [0..1] The ticket currently holding the lock. */
} atomic_owner_rwlock_t;

#define ATOMIC_OWNER_RWLOCK_INIT         {ATOMIC_RWLOCK_INIT,NULL}
#define atomic_owner_rwlock_cinit(self)  (void)(atomic_rwlock_cinit(&(self)->aorw_lock),assert((self)->aorw_owner == NULL))
#define atomic_owner_rwlock_init(self)   (void)(atomic_rwlock_init(&(self)->aorw_lock),(self)->aorw_owner = NULL)
#define DEFINE_ATOMIC_OWNER_RWLOCK(name)  atomic_owner_rwlock_t name = ATOMIC_OWNER_RWLOCK_INIT
#define atomic_owner_rwlock_reading(x)   (ATOMIC_READ((x)->aorw_lock) != 0)
#define atomic_owner_rwlock_writing(x)   (ATOMIC_READ((x)->aorw_lock)&ATOMIC_OWNER_RWLOCK_WFLAG)

/* Acquire an exclusive read/write lock. */
LOCAL SAFE bool KCALL atomic_owner_rwlock_tryread(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);
LOCAL SAFE bool KCALL atomic_owner_rwlock_trywrite(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);
LOCAL SAFE bool KCALL atomic_owner_rwlock_trywrite_r(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);
LOCAL SAFE void KCALL atomic_owner_rwlock_read(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);
LOCAL SAFE void KCALL atomic_owner_rwlock_write(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);

/* Try to upgrade a read-lock to a write-lock. Return 'FALSE' upon failure. */
LOCAL SAFE bool KCALL atomic_owner_rwlock_tryupgrade(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);

/* NOTE: The lock is always upgraded, but when 'FALSE' is returned, no lock
 *       may have been held temporarily, meaning that the caller should
 *       re-load local copies of affected resources. */
LOCAL SAFE bool KCALL atomic_owner_rwlock_upgrade(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);

/* Downgrade a write-lock to a read-lock (Always succeeds). */
LOCAL SAFE void KCALL atomic_owner_rwlock_downgrade(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);

/* End reading/writing/either.
 * @return: true:  The lock has become free.
 * @return: false: The lock is still held by something. */
LOCAL SAFE bool KCALL atomic_owner_rwlock_endwrite(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);
LOCAL SAFE bool KCALL atomic_owner_rwlock_endread(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);
LOCAL SAFE bool KCALL atomic_owner_rwlock_end(atomic_owner_rwlock_t *__restrict self, atomic_owner_rwlock_ticket_t *__restrict ticket);





LOCAL SAFE bool KCALL
atomic_owner_rwlock_end(atomic_owner_rwlock_t        *__restrict self,
                        atomic_owner_rwlock_ticket_t *__restrict ticket) {
 assert(TASK_ISSAFE());
 if (ticket->aorwt_write) {
  __assertf(self->aorw_owner == ticket,
            "Invalid ticket %p used with r/w-lock %p",
            ticket,self);
  if (--ticket->aorwt_write != 0) return false;
  if (ticket->aorwt_read) {
   /* Must downgrade a write-lock to a read-lock. */
   self->aorw_owner = NULL;
   __COMPILER_WRITE_BARRIER();
   atomic_rwlock_downgrade(&self->aorw_lock);
  } else {
   /* Must release the lock completely. */
   atomic_rwlock_endwrite(&self->aorw_lock);
  }
  return true;
 } else {
  __assertf(ticket->aorwt_read,"You're not holding any locks");
  __assertf(self->aorw_owner == NULL,
            "Invalid ticket %p used with r/w-lock %p",
            ticket,self);
  if (--ticket->aorwt_read != 0) return false;
  return atomic_rwlock_endread(&self->aorw_lock);
 }
}
LOCAL SAFE bool KCALL
atomic_owner_rwlock_endwrite(atomic_owner_rwlock_t        *__restrict self,
                             atomic_owner_rwlock_ticket_t *__restrict ticket) {
 __assertf(ticket->aorwt_write != 0,"You're not holding any write-locks");
 return atomic_owner_rwlock_end(self,ticket);
}
LOCAL SAFE bool KCALL
atomic_owner_rwlock_endread(atomic_owner_rwlock_t        *__restrict self,
                            atomic_owner_rwlock_ticket_t *__restrict ticket) {
 return atomic_owner_rwlock_end(self,ticket);
}
LOCAL SAFE bool KCALL
atomic_owner_rwlock_tryread(atomic_owner_rwlock_t        *__restrict self,
                            atomic_owner_rwlock_ticket_t *__restrict ticket) {
 assert(TASK_ISSAFE());
 if (ATOMIC_READ(self->aorw_owner) == ticket) {
  __assertf(ticket->aorwt_write,"Owner ticket should be in write-mode");
  __assertf(atomic_rwlock_writing(&self->aorw_lock),"lock with owner should be in write-mode");
  ++ticket->aorwt_write;
  return true;
 }
 __assertf(!ticket->aorwt_write,"Invalid ticket %p used with r/w-lock %p",self,ticket);
 if (ticket->aorwt_read) {
  __assertf(atomic_rwlock_reading(&self->aorw_lock),
            "Invalid ticket %p used with r/w-lock %p",
            self,ticket);
  ++ticket->aorwt_read;
  return true;
 }
 if (!atomic_rwlock_tryread(&self->aorw_lock)) return false;
 ticket->aorwt_read = 1;
 return true;
}
LOCAL SAFE bool KCALL
atomic_owner_rwlock_trywrite_r(atomic_owner_rwlock_t        *__restrict self,
                               atomic_owner_rwlock_ticket_t *__restrict ticket) {
 assert(TASK_ISSAFE());
 __assertf(ATOMIC_READ(self->aorw_owner) != ticket,"You must use 'atomic_owner_rwlock_trywrite()' here!");
 __assertf(!ticket->aorwt_write,"Invalid ticket %p used with r/w-lock %p",self,ticket);
 if (ticket->aorwt_read) {
  __assertf(atomic_rwlock_reading(&self->aorw_lock),
            "Invalid ticket %p used with r/w-lock %p",self,ticket);
  /* Must upgrade an existing lock. */
  if (!atomic_rwlock_tryupgrade(&self->aorw_lock)) return false;
 } else {
  /* Begin a new write-lock. */
  if (!atomic_rwlock_trywrite(&self->aorw_lock)) return false;
 }
 assert(!self->aorw_owner);
 self->aorw_owner = ticket;
 __COMPILER_WRITE_BARRIER();
 return true;
}
LOCAL SAFE bool KCALL
atomic_owner_rwlock_trywrite(atomic_owner_rwlock_t        *__restrict self,
                             atomic_owner_rwlock_ticket_t *__restrict ticket) {
 assert(TASK_ISSAFE());
 if (ATOMIC_READ(self->aorw_owner) == ticket) {
  __assertf(ticket->aorwt_write,"Owner ticket should be in write-mode");
  __assertf(atomic_rwlock_writing(&self->aorw_lock),"lock with owner should be in write-mode");
  ++ticket->aorwt_write;
  return true;
 }
 return atomic_owner_rwlock_trywrite_r(self,ticket);
}
LOCAL SAFE void KCALL
atomic_owner_rwlock_read(atomic_owner_rwlock_t        *__restrict self,
                         atomic_owner_rwlock_ticket_t *__restrict ticket) {
 while (!atomic_owner_rwlock_tryread(self,ticket)) SCHED_YIELD();
}
LOCAL SAFE void KCALL
atomic_owner_rwlock_write(atomic_owner_rwlock_t        *__restrict self,
                          atomic_owner_rwlock_ticket_t *__restrict ticket) {
 /* XXX: What about the potential deadlock when two threads are holding a read-lock
  *      when both attempt to start writing, thus causing an infinite loop? */
 while (!atomic_owner_rwlock_trywrite(self,ticket)) SCHED_YIELD();
}
LOCAL SAFE bool KCALL
atomic_owner_rwlock_tryupgrade(atomic_owner_rwlock_t        *__restrict self,
                               atomic_owner_rwlock_ticket_t *__restrict ticket) {
 __assertf(ticket->aorwt_read,"You're not holding any read-locks");
 if (ticket->aorwt_write) {
  __assertf(ATOMIC_READ(self->aorw_owner) == ticket,
            "Invalid ticket %p used with r/w-lock %p",self,ticket);
  --ticket->aorwt_read;
  ++ticket->aorwt_write;
  return true;
 }
 assert(ATOMIC_READ(self->aorw_owner) != ticket);
 if (!atomic_rwlock_tryupgrade(&self->aorw_lock)) return false;
 assert(!self->aorw_owner);
 self->aorw_owner = ticket;
 --ticket->aorwt_read;
 ++ticket->aorwt_write;
 __COMPILER_WRITE_BARRIER();
 return true;
}
LOCAL SAFE void KCALL
atomic_owner_rwlock_downgrade(atomic_owner_rwlock_t        *__restrict self,
                              atomic_owner_rwlock_ticket_t *__restrict ticket) {
 __assertf(ticket->aorwt_write != 0,"You're not holding any write-locks");
 __assertf(self->aorw_owner == ticket,
           "Invalid ticket %p used with r/w-lock %p",self,ticket);
 ++ticket->aorwt_read;
 if (--ticket->aorwt_write != 0) return;
 self->aorw_owner = NULL;
 atomic_rwlock_downgrade(&self->aorw_lock);
}

LOCAL SAFE bool KCALL
atomic_owner_rwlock_upgrade(atomic_owner_rwlock_t        *__restrict self,
                            atomic_owner_rwlock_ticket_t *__restrict ticket) {
 if (atomic_owner_rwlock_tryupgrade(self,ticket)) return true;
 atomic_owner_rwlock_endread(self,ticket);
 atomic_owner_rwlock_write(self,ticket);
 return false;
}

#endif

DECL_END

#endif /* !GUARD_HYBRID_SYNC_ATOMIC_OWNER_RWLOCK_H */
