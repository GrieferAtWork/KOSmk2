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
#ifndef GUARD_KERNEL_MMAN_PART_C
#define GUARD_KERNEL_MMAN_PART_C 1
#define _KOS_SOURCE 2

#include "intern.h"
#include <assert.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/malloc.h>
#include <kernel/mman.h>
#include <sys/syslog.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <string.h>

DECL_BEGIN

#define HAS_PREV    (ppart != &self->mr_parts)
#define HAS_NEXT    (part->mt_chain.le_next != NULL)
#define PREV         container_of(ppart,struct mregion_part,mt_chain.le_next)
#define NEXT         part->mt_chain.le_next

/* Split the given region part at 'split_addr', updating
 * 'part' to point to the returned part that contains its
 * contents starting at 'split_addr'
 * @return: NULL: Not enough available memory. */
INTERN struct mregion_part *
mregion_part_split_lo(struct mregion_part *__restrict part,
                      raddr_t split_addr) {
 struct mregion_part *result;
 CHECK_HOST_DOBJ(part);
 assert(split_addr > part->mt_start);
 assert(!HAS_NEXT || split_addr < NEXT->mt_start);
 assert(addr_isvirt(part) || PDIR_ISKPD());
 /* Mirror physical/virtual allocation behavior from the existing part. */
 result = (struct mregion_part *)kmalloc(sizeof(struct mregion_part),
                                         addr_isvirt(part) ? GFP_SHARED|GFP_NOFREQ
                                                           : GFP_MEMORY|GFP_NOFREQ);
 if (!addr_isvirt(part)) (void)_mall_untrack(result);
 if (result) {
  /* Copy shared data. */
  memcpy(&result->mt_refcnt,&part->mt_refcnt,
         offsetafter(struct mregion_part,mt_locked)-
         offsetof(struct mregion_part,mt_refcnt));
  if (part->mt_state == MPART_STATE_INCORE) {
   if (!mscatter_split_lo(&result->mt_memory,
                          &part->mt_memory,
                           split_addr-part->mt_start))
        goto fail;
  } else if (part->mt_state == MPART_STATE_INSWAP) {
   if (!mswap_ticket_split_lo(&result->mt_stick,
                              &part->mt_stick,
                               split_addr-part->mt_start))
        goto fail;
  }
  result->mt_start = split_addr;
  /* Insert the new part after the previous. */
  LIST_INSERT_AFTER(part,result,mt_chain);
 }
 return result;
fail: free(result);
 return NULL;
}

/* Try to merge the given region part with its successor.
 * @return: true:  The parts got merged.
 * @return: false: The parts were not merged. */
INTERN bool KCALL
mregion_part_merge(struct mregion_part *__restrict part) {
 struct mregion_part *next;
 if ((next = part->mt_chain.le_next) != NULL &&
      next->mt_refcnt == part->mt_refcnt &&
      next->mt_state  == part->mt_state &&
      next->mt_flags  == part->mt_flags) {
  if (part->mt_state == MPART_STATE_INSWAP) {
   if (!mscatter_cat(&part->mt_memory,&next->mt_memory)) return false;
  } else {
   if (!mswap_ticket_cat(&part->mt_stick,&next->mt_stick)) return false;
  }
  /* Remove the next-entry from the chain. */
  LIST_REMOVE(next,mt_chain);
  free(next);
 }
 return false;
}


typedef void (KCALL *part_action)(struct mregion *__restrict region,
                                  struct mregion_part *__restrict part);
INTERN bool KCALL
mregion_part_action(struct mregion *__restrict self,
                    raddr_t start, rsize_t n_bytes,
                    part_action action, part_action undo) {
 struct mregion_part **ppart,*part;
 raddr_t orig_start = start;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_TEXT(action,1);
 if unlikely(!n_bytes) goto end;
 assert(rwlock_writing(&self->mr_plock));
 assert(start+n_bytes >  start);
 assert(start+n_bytes <= self->mr_size);
 MASSERT_ALIGN(IS_ALIGNED(start,PAGESIZE));
 MASSERT_ALIGN(IS_ALIGNED(n_bytes,PAGESIZE));
 assert(self->mr_parts != NULL);
 for (ppart = &self->mr_parts; (part = *ppart) != NULL;
      ppart = &part->mt_chain.le_next)
again_thisone: {
  bool recheck_thisone;
  raddr_t part_begin = part->mt_start;
  raddr_t part_end   = HAS_NEXT ? NEXT->mt_start : self->mr_size;
  rsize_t part_size,update_size;
  assert(part_begin < part_end);
  if (part_end <= start) continue;
  part_size   = part_end-part_begin;
  update_size = (start+n_bytes)-part_begin;
  if (update_size > part_size)
      update_size = part_size;
  if (update_size > n_bytes)
      update_size = n_bytes; /* This can happen when 'start != part_begin' */
  //assert(update_size <= n_bytes);
  /* Check again to ensure that the given range is inside this part. */
  assert(start             >= part_begin);
  assert(start+update_size <= part_end);
  if (start != part_begin) {
   /* Split the part near its base. */
   if (!mregion_part_split_lo(part,start)) goto fail;
   continue;
  }
  recheck_thisone = false;
  if (start+update_size == part_end) {
   /* Both bounds match! - This is great! */
   (*action)(self,part);
   if (mregion_part_merge(part)) recheck_thisone = true;
   if (HAS_PREV && mregion_part_merge(PREV)) recheck_thisone = true;
  } else {
   struct mregion_part *high_part;
   /* Must split the part half-way. */
   high_part = mregion_part_split_lo(part,start+update_size);
   if unlikely(!high_part) goto fail;
   (*action)(self,part);
  }
  if (n_bytes == update_size) break;
  n_bytes -= update_size;
  start   += update_size;
  if (recheck_thisone) goto again_thisone;
 }
end: return true;
fail:
 if (undo) {
  /* Drop all references that were already created. */
  mregion_part_action(self,orig_start,
                      start-orig_start,undo,NULL);
  /* TODO: Log failure? */
 }
 return false;
}


PRIVATE void KCALL
action_incref(struct mregion *__restrict UNUSED(region),
              struct mregion_part *__restrict self) {
 ++self->mt_refcnt;
}
LOCAL void KCALL
action_do_decref(struct mregion *__restrict region,
                 struct mregion_part *__restrict self,
                 pgattr_t memory_attrib) {
 assertf(self->mt_refcnt != 0,
         "Missing references to part at region %p...%p",
         MREGION_PART_BEGIN(self),MREGION_PART_END(self,region));
 if (!--self->mt_refcnt) {
  /* Change the self state to not-allocated, freeing any allocated data. */
  if (self->mt_flags&MPART_FLAG_CHNG &&
      region->mr_init == MREGION_INIT_WFILE) {
   /* TODO: Signal 'MREGION_INITFUN_MODE_SAVE' to user-defined initializers. */
   /* TODO: Unlock region. */
   /* Write modified pages back to the file/user-callback. */
   errno_t error = -EOK;
   pos_t file_pos = region->mr_setup.mri_start+self->mt_start;
   pos_t file_end = region->mr_setup.mri_start+region->mr_setup.mri_size;
   if (self->mt_state == MPART_STATE_INCORE) {
    struct mscatter *iter = &self->mt_memory;
    size_t skip_before = 0;
    /* If the part is located below the mapping start, make sure to shift offsets. */
    if (self->mt_start < region->mr_setup.mri_begin) {
     size_t part_size = MREGION_PART_END(self,region)-MREGION_PART_BEGIN(self);
     skip_before = region->mr_setup.mri_begin-self->mt_start;
     if (skip_before >= part_size) goto done_write;
    }
    do {
     size_t max_write;
     uintptr_t start = (uintptr_t)iter->m_start;
     size_t    size  = iter->m_size;
     if (file_pos >= file_end) break;
     max_write = (size_t)(file_end-file_pos);
     if (max_write > size)
         max_write = size;
     /* Make sure to skip leading data */
     if (skip_before) {
      if (skip_before >= max_write) {
       skip_before -= max_write;
       goto next;
      }
      start     += skip_before;
      max_write -= skip_before;
      skip_before = 0;
     }
     error = file_kpwriteall(region->mr_setup.mri_file,
                            (void *)start,max_write,file_pos);
     if (E_ISERR(error)) break;
next:
     file_pos += max_write;
    } while ((iter = iter->m_next) != NULL);
   }
done_write:
   if (E_ISERR(error)) {
    syslog(LOG_MEM|LOG_ERROR,
            "[MEM] Failed to sync file mapping in %[file] at %I64u\n",
            file_pos);
   }
  }
  if (region->mr_type == MREGION_TYPE_MEM) {
   if (self->mt_state == MPART_STATE_INCORE) {
    struct mman *old_mm;
    TASK_PDIR_KERNEL_BEGIN(old_mm);
    page_free_scatter(&self->mt_memory,memory_attrib);
    TASK_PDIR_KERNEL_END(old_mm);
   } else if (self->mt_state == MPART_STATE_INSWAP) {
    mswap_delete(&self->mt_stick);
   }
   /* Fix the part state to mirror what's new. */
   self->mt_state  = MPART_STATE_MISSING;
   self->mt_flags &= ~(MPART_FLAG_CHNG);
  } else {
   assertf(region->mr_type == MREGION_TYPE_PHYSICAL ? (self->mt_state == MPART_STATE_INCORE) :
           MREGION_TYPE_ISGUARD(region->mr_type)    ? (self->mt_state == MPART_STATE_MISSING) :
           1,"%d",self->mt_state);
  }
 }
}
PRIVATE void KCALL
action_decref(struct mregion *__restrict region,
              struct mregion_part *__restrict self) {
 action_do_decref(region,self,PAGEATTR_NONE);
}
PRIVATE void KCALL
action_decref_clr(struct mregion *__restrict region,
                  struct mregion_part *__restrict self) {
 action_do_decref(region,self,PAGEATTR_ZERO);
}
PRIVATE void KCALL
action_inclck(struct mregion *__restrict region,
              struct mregion_part *__restrict self) {
 ++self->mt_locked;
}
PRIVATE void KCALL
action_declck(struct mregion *__restrict region,
              struct mregion_part *__restrict self) {
 --self->mt_locked;
}

INTERN bool KCALL
mregion_part_incref(struct mregion *__restrict self,
                    raddr_t start, rsize_t n_bytes) {
 return mregion_part_action(self,start,n_bytes,&action_incref,&action_decref);
}
PRIVATE ATTR_COLDTEXT void KCALL
mregion_part_decref_failed(struct mregion *__restrict self,
                           raddr_t start, rsize_t n_bytes) {
 syslog(LOG_MEM|LOG_ERROR,"[MEM] Failed to decref %p...%p of region %p...%p: %[errno]\n",
         start,start+n_bytes-1,0,self->mr_size-1,ENOMEM);
}
INTERN void KCALL
mregion_part_decref(struct mregion *__restrict self,
                    raddr_t start, rsize_t n_bytes) {
 if unlikely(!mregion_part_action(self,start,n_bytes,&action_decref,&action_incref))
              mregion_part_decref_failed(self,start,n_bytes);
}
INTERN void KCALL
mregion_part_decref_clr(struct mregion *__restrict self,
                        raddr_t start, rsize_t n_bytes) {
 if unlikely(!mregion_part_action(self,start,n_bytes,&action_decref_clr,&action_incref))
              mregion_part_decref_failed(self,start,n_bytes);
}
INTERN bool KCALL
mregion_part_inclock(struct mregion *__restrict self,
                     raddr_t start, rsize_t n_bytes) {
 CHECK_HOST_DOBJ(self);
 assert(rwlock_writing(&self->mr_plock));
 assert(self->mr_type != MREGION_TYPE_PHYSICAL);
 return mregion_part_action(self,start,n_bytes,&action_inclck,&action_declck);
}
INTERN bool KCALL
mregion_part_declock(struct mregion *__restrict self,
                     raddr_t start, rsize_t n_bytes) {
 CHECK_HOST_DOBJ(self);
 assert(rwlock_writing(&self->mr_plock));
 assert(self->mr_type != MREGION_TYPE_PHYSICAL);
 return mregion_part_action(self,start,n_bytes,&action_declck,&action_inclck);
#if 0
 syslog(LOG_MEM|LOG_ERROR,"[MEM] Failed to declck %p...%p of region %p...%p: %[errno]\n",
         start,start+n_bytes-1,0,self->mr_size-1,ENOMEM);
#endif
}
PRIVATE void KCALL
action_stchng(struct mregion *__restrict UNUSED(region),
              struct mregion_part *__restrict self) {
 self->mt_flags |= MPART_FLAG_CHNG;
}
INTDEF bool KCALL
mregion_part_setchng(struct mregion *__restrict self,
                     raddr_t start, rsize_t n_bytes) {
 CHECK_HOST_DOBJ(self);
 assert(rwlock_writing(&self->mr_plock));
 assert(self->mr_type != MREGION_TYPE_PHYSICAL);
 return mregion_part_action(self,start,n_bytes,&action_stchng,NULL);
}




INTERN errno_t KCALL
mregion_incref(struct mregion *__restrict self,
               raddr_t start, rsize_t n_bytes) {
 errno_t error;
 MREGION_INCREF(self);
 error = rwlock_write(&self->mr_plock);
 if (E_ISERR(error)) return error;
 error = mregion_part_incref(self,start,n_bytes);
 rwlock_endwrite(&self->mr_plock);
 if (!error) {
#ifdef CONFIG_DEBUG
  { ref_t newref = ATOMIC_DECFETCH(self->mr_refcnt);
    assertf(newref != 0,"The caller did not safely access this region!");
  }
#else
  ATOMIC_FETCHDEC(self->mr_refcnt);
#endif
  return -ENOMEM;
 }
 return -EOK;
}
INTERN errno_t KCALL
mregion_decref(struct mregion *__restrict self,
               raddr_t start, rsize_t n_bytes) {
 errno_t error = rwlock_write(&self->mr_plock);
 if (E_ISERR(error)) return error;
 mregion_part_decref(self,start,n_bytes);
 rwlock_endwrite(&self->mr_plock);
 MREGION_DECREF(self);
 return error;
}
INTERN errno_t KCALL
mregion_decref_clr(struct mregion *__restrict self,
                   raddr_t start, rsize_t n_bytes) {
 errno_t error = rwlock_write(&self->mr_plock);
 if (E_ISERR(error)) return error;
 mregion_part_decref(self,start,n_bytes);
 rwlock_endwrite(&self->mr_plock);
 MREGION_DECREF(self);
 return error;
}


#undef NEXT
#undef PREV
#undef HAS_NEXT
#undef HAS_PREV

DECL_END

#endif /* !GUARD_KERNEL_MMAN_PART_C */
