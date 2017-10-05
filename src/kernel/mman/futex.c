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
#ifndef GUARD_KERNEL_MMAN_FUTEX_C
#define GUARD_KERNEL_MMAN_FUTEX_C 1
#define _KOS_SOURCE 2

#include "intern.h"
#include <assert.h>
#include <hybrid/atomic.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/sched/yield.h>
#include <kernel/mman.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <dev/rtc.h>
#include <linux/futex.h>
#include <sched/task.h>

DECL_BEGIN

INTDEF void KCALL
mfutexptr_clear(atomic_rwptr_t *__restrict self) {
 struct mfutex *futex;
again:
 atomic_rwptr_write(self);
 futex = (struct mfutex *)ATOMIC_RWPTR_GET(*self);
 if (futex) {
  if (!atomic_rwptr_trywrite(&futex->f_next)) {
   atomic_rwptr_endwrite(self);
   SCHED_YIELD();
   goto again;
  }
  /* Delete the self-pointer of the first futex. */
  futex->f_pself = NULL;
  atomic_rwptr_endwrite(&futex->f_next);
  /* Drop the reference we'll inherited from 'self->fp_next' below. */
  MFUTEX_DECREF(futex);
 }
 ATOMIC_WRITE(self->ap_data,0);
}

FUNDEF void KCALL
mfutex_destroy(struct mfutex *__restrict self) {
 struct mfutex *next_futex;
 CHECK_HOST_DOBJ(self);
 assert(!self->f_refcnt);
again:
 atomic_rwptr_write(&self->f_next); /* lock 'f_pself' from being modified. */
 if (self->f_pself) {
  /* Lock the self-pointer to we can update it to point to our successor. */
  if (!atomic_rwptr_trywrite(self->f_pself)) {
   /* This may fail due to race conditions. */
   atomic_rwptr_endwrite(&self->f_next);
   SCHED_YIELD();
   goto again;
  }
 }
 next_futex = (struct mfutex *)ATOMIC_RWPTR_GET(self->f_next);
 if (next_futex) {
  /* Must update the next->pself pointer. */
  if (!atomic_rwptr_trywrite(&next_futex->f_next)) {
   if (self->f_pself) atomic_rwptr_endwrite(self->f_pself);
   atomic_rwptr_endwrite(&self->f_next);
   goto again;
  }
  next_futex->f_pself = self->f_pself;
  /* Update the pself-pointer to point to the next futex, and unlock the self pointer. */
  if (self->f_pself) ATOMIC_STORE(self->f_pself->ap_data,(uintptr_t)next_futex);
  atomic_rwptr_endwrite(&next_futex->f_next);
 } else {
  /* Update the pself-pointer to point to the next futex, and unlock the self pointer. */
  if (self->f_pself) ATOMIC_STORE(self->f_pself->ap_data,(uintptr_t)next_futex);
 }
 /* 'self' is now fully unlinked. */
 assert(!SIG_GETTASK(&self->f_sig));
 free(self);
}

PUBLIC SAFE REF struct mfutex *KCALL
mfutexptr_get(atomic_rwptr_t *__restrict self, raddr_t addr) {
 struct mfutex *result;
 CHECK_HOST_DOBJ(self);
 atomic_rwptr_read(self);
 while ((result = (struct mfutex *)ATOMIC_RWPTR_GET(*self)) != NULL) {
  CHECK_HOST_DOBJ(result);
  if (result->f_addr >= addr) { /* Found it, or doesn't exist. */
   bool ref_ok;
   if (result->f_addr != addr) break; /* Doesn't exist. */
   ref_ok = MFUTEX_TRYINCREF(result);
   atomic_rwptr_endread(self);
   return ref_ok ? result : NULL;
  }
  atomic_rwptr_read(&result->f_next);
  atomic_rwptr_endread(self);
  self = &result->f_next;
 }
 atomic_rwptr_endread(self);
 return NULL;
}

PUBLIC SAFE REF struct mfutex *KCALL
mfutexptr_new(atomic_rwptr_t *__restrict self, raddr_t addr) {
 struct mfutex *result,*next_futex;
 atomic_rwptr_t *iter = self;
 CHECK_HOST_DOBJ(self);
again:
 atomic_rwptr_read(iter);
 while ((result = (struct mfutex *)ATOMIC_RWPTR_GET(*iter)) != NULL) {
  CHECK_HOST_DOBJ(result);
  if (result->f_addr >= addr) { /* Found it, or doesn't exist. */
   if (result->f_addr != addr) {
insert_futex_into_iter:
    /* Must insert a new futex here. */
    if unlikely((result = mfutex_new()) != NULL) goto err_nomem;
    /* Upgrade the read-lock into a write-lock. */
    if (!atomic_rwptr_upgrade(iter)) {
     atomic_rwptr_endread(iter);
     free(result);
     iter = self;
     goto again;
    }
    result->f_refcnt = 1;
    result->f_addr   = addr;
    sig_init(&result->f_sig);
    /* Link in the new futex. */
    result->f_pself = iter; /* Prepare the self-pointer for setting iter  */
    result->f_next.ap_data = iter->ap_data&ATOMIC_RWPTR_ADDR_MASK;
    if ((next_futex = (struct mfutex *)result->f_next.ap_ptr) != NULL) {
     CHECK_HOST_DOBJ(next_futex);
     /* Try to lock the successor (If this fails, we must unlock everything and start over!) */
     if unlikely(!atomic_rwptr_trywrite(&next_futex->f_next)) {
      atomic_rwptr_endwrite(iter);
      free(result);
      SCHED_YIELD();
      goto again;
     }
     next_futex->f_pself = &result->f_next;
     atomic_rwptr_endwrite(&next_futex->f_next);
    }
    ATOMIC_STORE(iter->ap_data,(uintptr_t)result); /* NOTE: This like also unlocks 'iter'. */
    return result;
   }
   if unlikely(!MFUTEX_TRYINCREF(result)) {
    /* This can happen due to a race-condition where an
     * existing futex at the same address is currently being
     * deallocated, and was currently trying to lock its
     * self-pointer when we were faster. */
    goto insert_futex_into_iter;
   }
   atomic_rwptr_endread(iter);
   return result;
  }
  atomic_rwptr_read(&result->f_next);
  atomic_rwptr_endread(iter);
  iter = &result->f_next;
 }
 goto insert_futex_into_iter;
err_nomem:
 atomic_rwptr_endread(iter);
 return NULL;
}

SYSCALL_DEFINE6(futex,USER u32 *,uaddr,int,op,u32,val,
                USER struct timespec *,utime,USER u32 *,uaddr2,
                u32,val3) {
 /* futex() system call for linux compatibility. */
 struct timespec tmo,*ptmo = NULL; u32 val2 = 0;
 struct mman *mm = THIS_MMAN; ssize_t error;
 struct mfutex *lock = E_PTR(-ENOSYS);
 int cmd = op & FUTEX_CMD_MASK;
 if (utime &&
    (cmd == FUTEX_WAIT || cmd == FUTEX_LOCK_PI ||
     cmd == FUTEX_WAIT_BITSET || cmd == FUTEX_WAIT_REQUEUE_PI)) {
  /* Copy the given timeout if doing so is apart of the command. */
  if (copy_from_user(&tmo,utime,sizeof(struct timespec)))
      return -EFAULT;
  /* Add the current time to the timeout. */
  if (cmd == FUTEX_WAIT) {
   struct timespec now;
   sysrtc_get(&now);
   TIMESPEC_ADD(tmo,now);
  }
  ptmo = &tmo;
 }
 /* Interpret 'utime' as the second argument for specific commands. */
 if (cmd == FUTEX_REQUEUE || cmd == FUTEX_CMP_REQUEUE ||
     cmd == FUTEX_CMP_REQUEUE_PI || cmd == FUTEX_WAKE_OP)
     val2 = (u32)(uintptr_t)utime;

 (void)val2;

 task_crit();
 /* Now to execute the command. */
 switch (cmd) {

 case FUTEX_WAIT:
  val3 = FUTEX_BITSET_MATCH_ANY;
 case FUTEX_WAIT_BITSET:
  if unlikely(!val3) goto err_inval;
  lock = mman_newfutex(mm,uaddr);
  if (E_ISOK(lock)) {
   u32 user_value;
   bool has_write_lock = false;
   mfutex_read(lock);
wait_check_again:
   if (copy_from_user(&user_value,uaddr,4)) {
    if (has_write_lock)
         mfutex_endwrite(lock);
    else mfutex_endread(lock);
    MFUTEX_DECREF(lock);
    goto err_fault;
   }
   if ((user_value&val3) != val) {
    /* Different value. -> Don't wait. */
    if (has_write_lock)
         mfutex_endwrite(lock);
    else mfutex_endread(lock);
    MFUTEX_DECREF(lock);
    break;
   }
   if (!has_write_lock) {
    has_write_lock = true;
    if (!mfutex_upgrade(lock))
         goto wait_check_again;
   }
   /* The value is still the same. - Now wait for the associated signal. */
   error = sig_timedrecv_endwrite(&lock->f_sig,ptmo);
   MFUTEX_DECREF(lock);
   lock = E_PTR(error);
  }
  break;

 case FUTEX_WAKE:
  val3 = FUTEX_BITSET_MATCH_ANY;
 case FUTEX_WAKE_BITSET:
  //if unlikely(!val3) goto err_inval;
  /* TODO: Linux only wake threads who's wait bit-sets match 'val3' */
  lock = mman_getfutex(mm,uaddr);
  if (E_ISOK(lock)) {
   /* Wake at most 'val' threads waiting for the associated futex. */
   error = (ssize_t)sig_send(&lock->f_sig,(size_t)val);
   MFUTEX_DECREF(lock);
   lock = E_PTR(error);
  } else if (lock == E_PTR(-EFAULT)) {
   lock = E_PTR(0); /* Nothing was waiting for this address. */
  }
  break;

#if 0 /* TODO */
 case FUTEX_REQUEUE:
 case FUTEX_CMP_REQUEUE:
 case FUTEX_WAKE_OP:
 case FUTEX_LOCK_PI:
 case FUTEX_UNLOCK_PI:
 case FUTEX_TRYLOCK_PI:
 case FUTEX_WAIT_REQUEUE_PI:
 case FUTEX_CMP_REQUEUE_PI:
#endif
 default:
  /* Unknown command. */
  assert(lock == E_PTR(-ENOSYS));
  break;
 }
end:
 task_endcrit();
 return (syscall_slong_t)lock;
err_inval: lock = E_PTR(-EINVAL); goto end;
err_fault: lock = E_PTR(-EFAULT); goto end;
}

/* TODO: KOS-specific futex() system call that can be used to wait on multiple
 *       addresses, as well as allow for atomic wake()+wait() on the same address. */


DECL_END

#endif /* !GUARD_KERNEL_MMAN_FUTEX_C */
