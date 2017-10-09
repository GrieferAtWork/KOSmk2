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
#ifndef GUARD_KERNEL_SCHED_SIGHAND_C_INL
#define GUARD_KERNEL_SCHED_SIGHAND_C_INL 1
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#include <sched/signal.h>
#include <malloc.h>
#include <string.h>
#include <hybrid/check.h>

DECL_BEGIN

PUBLIC struct sigshare sigshare_kernel = {
    /* Reference counter explaination:
     *   - sigshare_kernel
     *   - inittask.t_sigshare
     *   - __bootcpu.c_idle.t_sigshare
     */
#ifdef CONFIG_DEBUG
    .ss_refcnt = 3,
#else
    .ss_refcnt = 0x80000003,
#endif
    .ss_pending = SIGPENDING_INIT,
};

PUBLIC REF struct sigshare *KCALL sigshare_new(void) {
 REF struct sigshare *result;
 result = ocalloc(REF struct sigshare);
 if unlikely(!result) return NULL;
 result->ss_refcnt = 1;
 sigpending_cinit(&result->ss_pending);
 return result;
}

PUBLIC void KCALL
sigshare_destroy(struct sigshare *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(!self->ss_refcnt);
 sigpending_fini(&self->ss_pending);
 free(self);
}
PUBLIC void KCALL
sigpending_fini(struct sigpending *__restrict self) {
 struct sigqueue *iter,*next;
 CHECK_HOST_DOBJ(self);
 iter = self->sp_queue;
 while (iter) {
  next = iter->sq_chain.le_next;
  free(iter);
  iter = next;
 }
}

PUBLIC errno_t KCALL
sigpending_enqueue_unlocked(struct sigpending *__restrict self,
                            siginfo_t const *__restrict signal_info) {
 struct sigqueue *entry;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(signal_info);
 assert(sigpending_writing(self));
 assert(signal_info->si_signo > 0 &&
        signal_info->si_signo < _NSIG);
 entry = omalloc(struct sigqueue);
 if unlikely(!entry) return -ENOMEM;
 /* Fill in the new queue entry and add it to the chain. */
 memcpy(&entry->sq_info,signal_info,sizeof(siginfo_t));
 SLIST_INSERT(self->sp_queue,entry,sq_chain);
 /* Mark the signal as pending in this controller. */
 __sigaddset(&self->sp_mask,signal_info->si_signo);
 return -EOK;
}
PUBLIC errno_t KCALL
sigpending_enqueue(struct sigpending *__restrict self,
                   siginfo_t const *__restrict signal_info) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 sigpending_write(self);
 error = sigpending_enqueue_unlocked(self,signal_info);
 /* Broadcast upon success. */
 if (E_ISOK(error)) sig_broadcast_unlocked(&self->sp_newsig);
 sigpending_endwrite(self);
 return error;
}

PUBLIC size_t KCALL
sigpending_discard(struct sigpending *__restrict self, int signo) {
 bool has_write_lock = false;
 size_t result = 0;
 struct sigqueue **piter,*iter;
 CHECK_HOST_DOBJ(self);
 sigpending_read(self);
 assert(signo > 0 && signo < _NSIG);
check_again:
 if (!sigismember(&self->sp_mask,signo)) goto end;
 if (!has_write_lock) {
  has_write_lock = true;
  if (!sigpending_upgrade(self))
       goto check_again;
 }
 /* Unset the signal in the pending mask. */
 sigdelset(&self->sp_mask,signo);
 piter = &self->sp_queue;
 while ((iter = *piter) != NULL) {
  if (iter->sq_info.si_signo == signo) {
   /* Found one! */
   *piter = iter->sq_chain.le_next;
   free(iter); /* Simply delete this entry. */
   ++result;
  } else {
   piter = &iter->sq_chain.le_next;
  }
 }
 /* Make sure to consider the possibility of an undefined signal. */
 if (!result) result = 1;
end:
 if (has_write_lock)
      sigpending_endwrite(self);
 else sigpending_endread(self);
 return result;
}


PUBLIC /*inherit*/struct sigqueue *KCALL
sigpending_try_dequeue_unlocked(struct sigpending *__restrict self,
                                sigset_t const *__restrict deque_mask,
                                bool *__restrict has_write_lock) {
 u8 *iter,*end,*right; int i;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(deque_mask);
 CHECK_HOST_DOBJ(has_write_lock);
 assert(*has_write_lock ? sigpending_writing(self)
                        : sigpending_reading(self));
 end = (iter = (u8 *)&self->sp_mask)+sizeof(sigset_t);
 right = (u8 *)deque_mask;
 for (; iter != end; ++iter,++right) {
  if (*iter & *right) {
   /* Figure out where exactly the match occurred. */
   for (i = 0;; ++i) {
    assert(i < 8);
    if ((*iter  & (1 << i)) &&
        (*right & (1 << i)))
        break;
   }
   i += ((right-(u8 *)deque_mask)*8)+1;
   /* Now search for the associated pending entry.
    * NOTE: It may not exist if the signal was send without data. */
   {
    struct sigqueue **pfirst,**piter,*iter;
    pfirst = NULL;
    piter = &self->sp_queue;
    while ((iter = *piter) != NULL) {
     if (iter->sq_info.si_signo == i) {
      /* Found a match!
       * >> This means we'll have to try and remove it,
       *    also meaning that we need write-access. */
      if (!*has_write_lock) {
       *has_write_lock = true;
       if (!sigpending_upgrade(self))
            return E_PTR(-ERELOAD);
      }
      if (pfirst) goto remove_non_last;
      pfirst = piter;
     }
     piter = &iter->sq_chain.le_next;
    }
    /* Remove the signal set entry for a last/undefined match. */
    sigdelset(&self->sp_mask,i);

    /* With no associated entry, return an undefined queue entry. */
    if (!pfirst) return SIGPENDING_UNDEFINED(i);

    /* If we didn't find secondary entries, this was the last,
     * meaning we have to delete the  */

remove_non_last:
    CHECK_HOST_DOBJ(pfirst);
    CHECK_HOST_DOBJ(*pfirst);
    /* Unlink the first entry we've found. */
    iter = *pfirst;
    *pfirst = iter->sq_chain.le_next;
    return iter;
   }
  }
 }
 /* Nothing was found... */
 return NULL;
}












PUBLIC struct sighand sighand_kernel = {
    /* Reference counter explanation:
     *   - sighand_kernel
     *   - inittask.t_sighand
     *   - __bootcpu.c_idle.t_sighand
     */
#ifdef CONFIG_DEBUG
    .sh_refcnt = 3,
#else
    .sh_refcnt = 0x80000003,
#endif
    .sh_actions = {
    },
};

PUBLIC void KCALL
sighand_destroy(struct sighand *__restrict self) {
 CHECK_HOST_DOBJ(self);
 free(self);
}


PUBLIC REF struct sighand *KCALL sighand_new(void) {
 REF struct sighand *result;
 result = ocalloc(REF struct sighand);
 if unlikely(!result) return NULL;
 result->sh_refcnt = 1;
 /* NOTE: At this point, all signals have already
  *       been pre-initialized to 'SIG_DFL' */
 return result;
}
PUBLIC REF struct sighand *KCALL
sighand_copy(struct sighand *__restrict self) {
 REF struct sighand *result;
 CHECK_HOST_DOBJ(self);
 result = (REF struct sighand *)memdup(self,sizeof(struct sighand));
 if (result) result->sh_refcnt = 1;
 return result;
}
PUBLIC void KCALL
sighand_reset(struct sighand *__restrict self) {
 CHECK_HOST_DOBJ(self);
 /* As simple as that! */
 memset(&self->sh_actions,0,sizeof(self->sh_actions));
}

DECL_END

#endif /* !GUARD_KERNEL_SCHED_SIGHAND_C_INL */
