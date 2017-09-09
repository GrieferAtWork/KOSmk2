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


DECL_END

#endif /* !GUARD_KERNEL_MMAN_FUTEX_C */
