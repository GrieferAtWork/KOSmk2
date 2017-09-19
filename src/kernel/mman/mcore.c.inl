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
#ifndef GUARD_KERNEL_MMAN_MCORE_C_INL
#define GUARD_KERNEL_MMAN_MCORE_C_INL 1
#define _KOS_SOURCE 2

#include "intern.h"
#include <fs/dentry.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kernel/malloc.h>
#include <kernel/memory.h>
#include <kernel/mman.h>
#include <sys/syslog.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef __INTELLISENSE__
#include "mman.c"
#endif

DECL_BEGIN

PRIVATE errno_t KCALL
mscatter_kpread(struct mscatter *__restrict scatter,
                struct file *__restrict fp, pos_t pos,
                size_t max_read, size_t fill_before,
                u32 filler_dword) {
 ssize_t temp; size_t part;
#if 0
 /* Log a debug message describing how we're going to load this scatter tab. */
 syslog(LOG_DEBUG,"LOAD: %[file] FILL(%.4IX,%IX), READ(%.4IX,%IX,%I64X), FILL(%.4IX,...)\n",
        fp,(void *)0,fill_before,
       (void *)fill_before,max_read,pos,
       (void *)(fill_before+max_read));
#endif
 for (; scatter; scatter = scatter->m_next) {
  uintptr_t start = (uintptr_t)scatter->m_start;
  size_t    size  = scatter->m_size;
  assert(size);
  if (fill_before) {
   part = size;
   if (part > fill_before)
       part = fill_before;
   assert(part);
   memsetl((void *)start,filler_dword,part/4);
   start += part;
   size  -= part;
   if (!size) continue;
  }
  if (max_read) {
   part = size;
   if (part > max_read)
       part = max_read;
   assert(part);
   temp = file_kpread(fp,(void *)start,part,pos);
   if (E_ISERR(temp)) return (errno_t)temp;
   if unlikely(!temp) max_read = 0; /* Handle EOF */
   pos      += (size_t)temp;
   start    += (size_t)temp;
   max_read -= (size_t)temp;
   size     -= (size_t)temp;
   if (!size) continue;
  }
  /* Fill the remainder using the filler byte. */
  memsetl((void *)start,filler_dword,size/4);
 }
 return -EOK;
}

PRIVATE void KCALL
mscatter_memset(struct mscatter *__restrict scatter,
                u32 filler_dword) {
 while (scatter) {
  memsetl(scatter->m_start,filler_dword,
          scatter->m_size/4);
  scatter = scatter->m_next;
 }
}

LOCAL SAFE KPD bool KCALL
page_malloc_scatter_with_hint(struct mscatter *__restrict scatter,
                              size_t n_bytes, size_t min_scatter,
                              pgattr_t attr, mzone_t zone, ppage_t hint) {
 CHECK_HOST_DOBJ(scatter);
 /* Try to allocate at the hinted address to improve
  * physical/virtual coherency & cache locality. */
 scatter->m_start = page_malloc_at(hint,n_bytes,attr);
 if (scatter->m_start != PAGE_ERROR) {
  /* Awesome! */
  scatter->m_size = n_bytes;
  scatter->m_next = NULL;
  return true;
 }
 /* Ok... Lets try it the normal way... */
 return page_malloc_scatter(scatter,n_bytes,min_scatter,attr,zone);
}

LOCAL SAFE KPD bool KCALL
page_malloc_scatter_in_region_part(struct mscatter *__restrict scatter,
                                   size_t n_bytes, size_t min_scatter,
                                   pgattr_t attr, mzone_t zone,
                                   struct mregion *__restrict region,
                                   struct mregion_part *__restrict part,
                                   bool hint_unique_parts) {
 struct mregion_part *nearby_part;
 uintptr_t malloc_hint;
 rsize_t malloc_hint_distance;
 CHECK_HOST_DOBJ(part);

 malloc_hint          = (uintptr_t)(void *)PAGE_ERROR;
 malloc_hint_distance = (rsize_t)-1;

again:
  /* Try to allocate physical memory close to other physical allocations nearby. */
 nearby_part = part;
 while (nearby_part != region->mr_parts) {
  assert(*nearby_part->mt_chain.le_pself == nearby_part);
  nearby_part = container_of(nearby_part->mt_chain.le_pself,
                           struct mregion_part,
                           mt_chain.le_next);
  CHECK_HOST_DOBJ(nearby_part);
  if (nearby_part->mt_state == MPART_STATE_INCORE &&
     (!hint_unique_parts || nearby_part->mt_refcnt == 1)) {
   struct mscatter *part_end;
   assert(part->mt_start > nearby_part->mt_start);
   /* Use the last physically allocated part. */
   part_end = &nearby_part->mt_memory;
   while ((assert(part_end->m_size),
           part_end->m_next != NULL))
           part_end = part_end->m_next;
   /* Using the physical end address of the previous region's last
    * part, prefer allocating more physical pages directly above. */
   malloc_hint_distance = (part->mt_start-nearby_part->mt_chain.le_next->mt_start);
   malloc_hint          = (uintptr_t)part_end->m_start+part_end->m_size;
   malloc_hint         +=  malloc_hint_distance;
   break;
  }
 }
 if (!malloc_hint_distance) goto perfect_part;

 /* Use upper pages for the same effect. */
 nearby_part = part;
 while ((nearby_part->mt_start-part->mt_start) <
         malloc_hint_distance) {
  nearby_part = nearby_part->mt_chain.le_next;
  if (!nearby_part) break;
  CHECK_HOST_DOBJ(nearby_part);
  if (nearby_part->mt_state == MPART_STATE_INCORE &&
     (!hint_unique_parts || nearby_part->mt_refcnt == 1)) {
   /* Using the physical address address of the first part,
    * subtract the nearby part's offset  */
   assert(nearby_part->mt_memory.m_size);
   malloc_hint  = (uintptr_t)nearby_part->mt_memory.m_start;
   malloc_hint -= (nearby_part->mt_start-part->mt_start);
   break;
  }
 }
perfect_part:
 if (hint_unique_parts &&
     malloc_hint == (uintptr_t)(void *)PAGE_ERROR) {
  assert(malloc_hint_distance == (size_t)-1);
  /* Try again without requiring unique parts. */
  hint_unique_parts = false;
  goto again;
 }

 assert(malloc_hint == (uintptr_t)(void *)PAGE_ERROR ||
        IS_ALIGNED(malloc_hint,PAGESIZE));
 return (malloc_hint != (uintptr_t)(void *)PAGE_ERROR)
  ? page_malloc_scatter_with_hint(scatter,n_bytes,min_scatter,attr,zone,(ppage_t)malloc_hint)
  : page_malloc_scatter          (scatter,n_bytes,min_scatter,attr,zone);
}


/* Allocate+initialize memory from the given region range.
 * @return: * :         The amount of newly allocate core memory.
 * @return: -ENOMEM:    Not enough available core memory.
 * @return: E_ISERR(*): Failed to allocate+initialize memory for some reason.
 * NOTE: Upon error 'E_ISERR(*)', a write-lock to 'self->mr_plock' will be released.
 */
LOCAL ssize_t KCALL
mregion_load_core(struct mregion *__restrict self,
                  PAGE_ALIGNED raddr_t start,
                  PAGE_ALIGNED rsize_t size, u32 mode,
                  bool *__restrict holds_region_ref,
                  struct mman **__restrict pold_mman) {
 struct mscatter load_scatter; errno_t error;
 struct mregion_part **piter,*iter;
 size_t part_number,result = 0;
 CHECK_HOST_DOBJ(self);
 assert(IS_ALIGNED(start,PAGESIZE));
 assert(IS_ALIGNED(size,PAGESIZE));
 assert(start+size >  start);
 assert(start+size <= self->mr_size);
again:
 part_number = 0;
 for (piter = &self->mr_parts;
     (iter = *piter) != NULL;
      piter = &iter->mt_chain.le_next,
      ++part_number) {
  raddr_t iter_end; rsize_t load_size;
  assert(iter->mt_chain.le_next != iter);
  if (iter->mt_state == MPART_STATE_INCORE ||
      iter->mt_state == MPART_STATE_UNKNOWN)
      continue;
  /* Skip unlocked parts when we're not supposed to load them! */
  if (mode&MMAN_MCORE_LOCKED && !MREGION_PART_ISLOCKED(iter)) continue;

  assertf(self->mr_type != MREGION_TYPE_PHYSICAL,
          "Physical regions must always be in-core");
  if (iter->mt_start >= start+size) break;
  iter_end = iter->mt_chain.le_next ? iter->mt_chain.le_next->mt_start
                                    : self->mr_size;
  assert(iter_end > iter->mt_start);
  if (iter_end <= start) continue;
  /* At least some portion of this part must be loaded in some way. */
  assert(start >= iter->mt_start);
  if (start != iter->mt_start) {
   assert(start < iter_end);
   piter = &iter->mt_chain.le_next;
   iter = mregion_part_split_lo(iter,start);
   if (!iter) goto err_nomem;
   assert(*piter == iter);
   ++part_number;
  }
  if (start+size < iter_end) {
   if (!mregion_part_split_lo(iter,start+size)) goto err_nomem;
   assert(iter->mt_chain.le_next);
   assert(iter->mt_chain.le_next->mt_start == start+size);
   iter_end = start+size;
  }
  assertf(iter_end > iter->mt_start,
          "iter_end       = %p\n"
          "iter->mt_start = %p\n",
          iter_end,iter->mt_start);
  load_size = iter_end-iter->mt_start;
  assert(load_size <= size);
  assert(iter->mt_start == start);
  /*  */

  /* Make sure to switch to the kernel page directory when necessary. */
  if (!*pold_mman) TASK_PDIR_KERNEL_BEGIN(*pold_mman);
  /* Allocate physical memory for core data. */
  if (!page_malloc_scatter_in_region_part(&load_scatter,load_size,PAGESIZE,
                                          self->mr_init == MREGION_INIT_ZERO ? PAGEATTR_ZERO : PAGEATTR_NONE,
                                          MZONE_ANY,self,iter,false)) error = -ENOMEM;
  else /* Huzza! We've got the memory! - Now to initialize it. */
  if (iter->mt_state == MPART_STATE_INSWAP) {
load_swap:
   /* Load data out of swap. */
   error = mswap_reload(&iter->mt_stick,&load_scatter);
  } else {
   switch (MREGION_INIT_TYPE(self->mr_init)) {

   case MREGION_INIT_TYPE(MREGION_INIT_BYTE):
    /* Simple, constant-byte initialization. */
    mscatter_memset(&load_scatter,self->mr_setup.mri_byte);
    error = -EOK;
    break;

   {
    struct mscatter *miter;
   case MREGION_INIT_TYPE(MREGION_INIT_REREAND):
    miter = &load_scatter;
    /* Use actual random data to initialize this memory. */
    for (; miter; miter = miter->m_next) {
     u32 *piter,*pend;
     assert(IS_ALIGNED((uintptr_t)miter->m_start,PAGESIZE));
     assert(IS_ALIGNED(miter->m_size,PAGESIZE));
     piter = (u32 *)miter->m_start;
     pend  = (u32 *)((uintptr_t)miter->m_start+miter->m_size);
     for (; piter < pend; ++piter) *piter = rand();
    }
    error = -EOK;
   } break;

   { /* Custom/potentially volatile initializations. */
    size_t check_i; raddr_t part_begin;
    struct mregion_part **pcheck,*check;
   case MREGION_INIT_TYPE(MREGION_INIT_FILE):
   case MREGION_INIT_TYPE(MREGION_INIT_USER):
    part_begin = iter->mt_start;
    /* Keep a reference to the region to prevent it from dying while we're working. */
    if (!*holds_region_ref) { MREGION_INCREF(self); *holds_region_ref = true; }
    rwlock_endwrite(&self->mr_plock);
    if (MREGION_INIT_ISFILE(self->mr_init)) {
     size_t max_read = self->mr_setup.mri_size;
     /* Figure out how much should be read from the file. */
     if (max_read > iter_end) max_read = iter_end;
     if (max_read < part_begin) max_read  = 0;
     else max_read -= part_begin;
#if 0
     syslog(LOG_DEBUG,"SCATTER (%[file]:%I64X - %IX bytes; offset %IX)\n",
            self->mr_setup.mri_file,
            self->mr_setup.mri_start+part_begin,
            max_read,part_begin);
#endif
     /* Read the file data into the allocated scatter. */
     error = mscatter_kpread(&load_scatter,self->mr_setup.mri_file,
                              self->mr_setup.mri_start+part_begin,
                              max_read,self->mr_setup.mri_begin,
                              self->mr_setup.mri_byte);
    } else {
     /* User-defined initialization. */
     CHECK_HOST_TEXT(self->mr_setup.mri_ufunc,1);
     error = (*self->mr_setup.mri_ufunc)(MREGION_INITFUN_MODE_LOAD,
                                         self->mr_setup.mri_uclosure,
                                         self,&load_scatter,
                                         part_begin,load_size);
    }
    /* Check if an error code was returned. */
    if (E_ISERR(error)) {
err_load_scatter:
     page_free_scatter(&load_scatter,PAGEATTR_NONE);
     goto return_error;
    }
    /* re-lock the region. */
    error = rwlock_write(&self->mr_plock);
    if (E_ISERR(error)) goto err_load_scatter;

    /* Validate that no parts changed in the mean time.
     * >> If they did change, we must discard everything and start over!
     * XXX: Keep in mind that this must not result in an infinite loop... */
    check_i = part_number;
    pcheck = &self->mr_parts;
    for (;;) {
     check = *pcheck;
     if (!check_i--) break;
     if unlikely(!check) goto restart_scatter;
     pcheck = &check->mt_chain.le_next;
    }
    if unlikely(pcheck != piter || check != iter) goto restart_scatter;
    if unlikely(iter->mt_start != part_begin) goto restart_scatter;
    if unlikely(iter_end != (iter->mt_chain.le_next
                           ? iter->mt_chain.le_next->mt_start
                           : self->mr_size)) goto restart_scatter;
    /* One more thing: Is the part still not loaded? */
    if unlikely(iter->mt_state != MPART_STATE_MISSING) {
     /* $h1t! - Now we have to deal with this. */
     if (iter->mt_state == MPART_STATE_INCORE) {
      /* Simple enough:
       * > Don't overwrite existing core data and act
       *   like we didn't do anything (which we hadn't yet) */
      page_free_scatter(&load_scatter,PAGEATTR_NONE);
      goto next_part;
     } else {
      assertf(iter->mt_state == MPART_STATE_INSWAP,
              "Invalid state: %d",iter->mt_state);
      /* Fair enough: - We've already got the memory, so now
       *                we just have to load the swap data. */
      goto load_swap;
     }
    }
   } break;

   default:
    /* Default: keep data initialized as random. */
    error = -EOK;
    break;
   }
  }
  /* -- END OF MEMORY INITIALIZATION -- */
  if (E_ISERR(error)) {
error_init:
   page_free_scatter(&load_scatter,PAGEATTR_NONE);
   if (error == -ERELOAD) goto again;
   goto err;
  }
  /* Now simply install the new scatter. */
  assert(self->mr_type == MREGION_TYPE_MEM);
  iter->mt_state  = MPART_STATE_INCORE;
  iter->mt_memory = load_scatter;

  /* Finally, move on to the next part! */
  result += load_size;
next_part:;
 }
end:
 return (ssize_t)result;
err_nomem: error = -ENOMEM;
err: rwlock_endwrite(&self->mr_plock);
return_error: result = (size_t)error;
 goto end;
restart_scatter: error = -ERELOAD; goto error_init;
}


INTERN ssize_t KCALL
mbranch_mcore(struct mbranch *__restrict self,
              struct mman *__restrict mspace,
              raddr_t region_begin, rsize_t region_size,
              u32 mode, bool *__restrict did_remap) {
 ssize_t load_bytes; struct mregion *region; errno_t error;
 bool holds_region_ref = false; VIRT uintptr_t region_base;
 mbranch_notity self_notify; void *self_closure;
 bool holds_notify_ref = false;
 struct mman *old_mman = NULL;
 CHECK_HOST_DOBJ(self);
#if 0
 syslog(LOG_DEBUG|LOG_MEM,"[MEM] Loading branch %p...%p into the core\n",
        self->mb_node.a_vmin,self->mb_node.a_vmax);
#endif
 /* Simple check: Ignore no-user branches. */
 if ((mode&MMAN_MCORE_USER) &&
     (self->mb_prot&PROT_NOUSER)) return 0;
 region = self->mb_region;
 CHECK_HOST_DOBJ(region);

 /* Figure out the in-region core address. */
 assert(region_begin             >= self->mb_start);
 assert(region_begin+region_size <= self->mb_start+MBRANCH_SIZE(self));
 region_base = MBRANCH_BEGIN(self)-self->mb_start;
 assert(region_begin+region_size  > region_begin);
 assert(region_begin+region_size <= region->mr_size);

lock_again:
 assert(region == self->mb_region);
 load_bytes = (ssize_t)rwlock_write(&region->mr_plock);
 if (E_ISERR(load_bytes)) return load_bytes;

 /* Handle guard regions! */
 if (MREGION_TYPE_ISGUARD(region->mr_type)) {
#if 1
  REF struct mregion *mapregion;
  ppage_t remap_location,merge_location;
  bool must_relock_guard_region = false;
  size_t guard_size = MBRANCH_SIZE(self);
  assert(!holds_region_ref);
  assert(region->mr_parts);
  assert(region->mr_parts->mt_state == MPART_STATE_MISSING);

  /* Simple check: Ignore guard regions. */
  if (mode&MMAN_MCORE_NOGUARD) { load_bytes = 0; goto end; }
  if unlikely(region->mr_gfunds == 0) {
   /* No more funds. - The guard page can no longer be extended. */
   rwlock_endwrite(&region->mr_plock);
   /* Call the notifier to signal that the guard region has run out of funds. */
   if (self->mb_notify)
     (*self->mb_notify)(MNOTIFY_GUARD_END,self->mb_closure,mspace,
                       (ppage_t)MBRANCH_BEGIN(self),guard_size);
   load_bytes = 0;
   goto end2;
  }

  /* Figure out where the region should be remapped. */
  assert(self->mb_start+guard_size <= region->mr_size);
  if (region->mr_type == MREGION_TYPE_LOGUARD) {
   merge_location = (ppage_t)MBRANCH_BEGIN(self);
   remap_location = (ppage_t)((uintptr_t)merge_location-guard_size);
  } else {
   merge_location = (ppage_t)MBRANCH_END(self);
   remap_location = merge_location;
  }

  /* Allocate the region that will replace the guard in this context. */
  mapregion = mregion_new(GFP_NOFREQ|MMAN_DATAGFP(mspace));
  if unlikely(!mapregion) { load_bytes = -ENOMEM; goto end; }
  /* Copy size and initialization properties from the guard region. */
  assert(mapregion->mr_type == MREGION_TYPE_MEM);
  mapregion->mr_size = region->mr_size;
  mapregion->mr_init = region->mr_init;
  memcpy(&mapregion->mr_setup,&region->mr_setup,
         sizeof(union mregion_cinit));
  if (MREGION_INIT_ISFILE(mapregion->mr_init))
      FILE_INCREF(mapregion->mr_setup.mri_file);
  /* Consume guard funds from the existing region. */
  if (region->mr_gfunds != MREGION_GFUNDS_INFINITE) --region->mr_gfunds;
  syslog(LOG_DEBUG|LOG_MEM,
         "[MEM] Allocating GUARD region %p...%p (%Iu bytes; %I16u remaining funds)\n",
         MBRANCH_MIN(self),MBRANCH_MAX(self),guard_size,region->mr_gfunds);

  assert(region->mr_parts->mt_state == MPART_STATE_MISSING);
  assert(mapregion->mr_parts == &mapregion->mr_part0);
  assert(mapregion->mr_part0.mt_refcnt == 0);
  assert(mapregion->mr_refcnt == 1);
  mregion_setup(mapregion);

  /* Create the mapped reference held by the branch itself. */
  load_bytes = rwlock_trywrite(&mapregion->mr_plock);
  if (E_ISERR(load_bytes)) {
   if unlikely(load_bytes == -EAGAIN) {
    /* If we could acquire both locks at once, release the
     * lock to the old (guard) region and re-acquire it later.
     * >> This is required to prevent a potential deadlock
     *    when holding two region locks at the same time. */
    rwlock_endwrite(&region->mr_plock);
    must_relock_guard_region = true;
    load_bytes = rwlock_write(&mapregion->mr_plock);
   } else {
    goto err_mapregion;
   }
  }
  if (!mregion_part_incref(mapregion,self->mb_start,guard_size)) {
   load_bytes = -ENOMEM;
   rwlock_endwrite(&mapregion->mr_plock);
err_mapregion:
   MREGION_DECREF(mapregion);
   goto end;
  }
  rwlock_endwrite(&mapregion->mr_plock);
  /* At this point we've acquired new sub-region references within the guard region copy.
   * >> This had to be done before dropping references from the guard region,
   *    as the case of the incref() failing would otherwise leave us in an
   *    impossible-to-recover state where we'd no longer be holding any references. */

  /* Drop the map references from the real guard
   * (new references will be created when it is re-mapped) */
  if (must_relock_guard_region) {
   load_bytes = rwlock_write(&region->mr_plock);
   if (E_ISERR(load_bytes)) {
    task_nointr();
    rwlock_write(&mapregion->mr_plock);
    mregion_part_decref(mapregion,self->mb_start,guard_size);
    rwlock_endwrite(&mapregion->mr_plock);
    task_endnointr();
    goto err_mapregion;
   }
  }
  mregion_part_decref(region,self->mb_start,guard_size);
  rwlock_endwrite(&region->mr_plock);

  /* Simply replace the  */
  assert(region == self->mb_region);
  /* Inherit old reference into 'region' (dropped below) */
  /* Inherit new reference from 'mapregion' (Kept valid due to caller-held lock to 'mspace->m_lock') */
  self->mb_region = mapregion;

  /* NOTE: Since the guard region must not have any parts associated with
   *       it, we can assume that no page-directory mapping exists within.
   *       So with that in mind, alongside the fact that the guard replacement
   *      'mapregion' is also empty (as we've yet to allocate its contents
   *       through use of ALOA semantics), nothing was mapped, and will be
   *       mapped for the time being.
   *       Actual memory will be assigned later, once 'mregion_load_core()' gets called.
   * HINT: These facts are asserted above.
   */
  /* mbranch_remap_unlocked(...); */

  /* Emit a notification to inform the system that a guard region was allocated. */
  if (self->mb_notify)
    (*self->mb_notify)(MNOTIFY_GUARD_ALLOCATE,self->mb_closure,mspace,
                      (ppage_t)MBRANCH_BEGIN(self),guard_size);

  /* Check if we'll be re-mapping the guard region, or dropping it. */
  if (!mman_inuse_unlocked(mspace,remap_location,guard_size)) {
   /* Re-map the guard region at a lower address. */
   load_bytes = (ssize_t)mman_mmap_unlocked(mspace,remap_location,guard_size,0,region,
                                            self->mb_prot,self->mb_notify,self->mb_closure);
   if (E_ISERR(load_bytes)) { MREGION_DECREF(region); goto end2; }
   /* Notify the caller that we did a remap. */
   *did_remap = true;
  } else {
   /* Notify the branch callback that the we couldn't remap the guard region. */
   if (self->mb_notify)
     (*self->mb_notify)(MNOTIFY_GUARD_INUSE,self->mb_closure,mspace,
                       (ppage_t)MBRANCH_BEGIN(self),guard_size);
  }

  /* Continue working with the newly mapped
   * region, allocating data within it instead. */
  MREGION_DECREF(region);
  region = mapregion;
  assert(self->mb_region == mapregion);

  /* Now that we're no longer dealing with a
   * guard-region, re-lock the newly mapped region. */
  assert(!MREGION_TYPE_ISGUARD(region->mr_type));
  assert(!holds_region_ref);

  /* Try to merge the mapped guard region with neighbor. */
  if (mman_merge_branch_unlocked(mspace,(uintptr_t)merge_location)) {
   /* The branch layout has changed. and 'self' may no longer be valid.
    * >> Instead, we must opt out and tell the caller that the memory layout has
    *    changed, which will cause them to reload data before calling us again. */
   *did_remap = true;
   load_bytes = 0;
   goto end2;
  }

  goto lock_again;
#else
  load_bytes = 0;
  goto end;
#endif
 }

 /* First: Make sure that all parts are loaded. */
 load_bytes = mregion_load_core(region,region_begin,region_size,
                                mode,&holds_region_ref,&old_mman);
 if (E_ISERR(load_bytes)) {
  /* Translate no-data errors to nothing-loaded.
   * NOTE: '-ENODATA' is returned when a user-initializer has been deleted. */
  if (load_bytes == -ENODATA)
      load_bytes = 0;
  /* NOTE: When 'mregion_load_core' fails, it unlocks
   *       the r/w-lock of the associated region. */
  goto end2;
 }


 /* NOTE: Must always remap the branch, because even if
  *       nothing needed to be loaded, or allocated, that
  *       may have only been the case because another
  *       thread already allocated it before potentially
  *       dying or simply unmapping it, leaving us to
  *       be its sole owner. */
 error = mbranch_remap_unlocked(self,&mspace->m_pdir,false);
 if (E_ISERR(error)) goto err;

 self_notify   = self->mb_notify;
 self_closure  = self->mb_closure;

 if (self_notify) {
  /* TODO: Track the exact region offsets that were loaded by 'mregion_load_core' */
  //(*self_notify)(MNOTIFY_LOADED,self_closure,...);
 }

#if 1
 /* Copy-on-write handling. */
 if (mode&MMAN_MCORE_WRITE) {
  load_bytes = 0;
  if (!(self->mb_prot&PROT_WRITE));
  else if (self->mb_prot&PROT_SHARED) {
   /* Mark changes in shared regions. */
   if (!mregion_part_setchng(region,region_begin,region_size)) goto enomem;
  } else {
   /* Copy shared core pages. */
   struct mregion_part **piter,*iter;
   u32 mprot = self->mb_prot;
   /* #1: Find all parts within 'region_begin...+=region_size'
    *     that match 'mt_refcnt > 1', figuring out exactly
    *     how much core memory is required to copy all of them.
    *     All other parts (with 'mt_refcnt <= 1') are marked with 'MPART_FLAG_CHNG'
    * #2: Allocate scattered memory for all of them
    * #3: Figure out which parts are located adjacent to each other.
    * #4: Allocate a region + branches for every group of adjacent parts.
    * #5: Create new parts at the same locations within
    *     those regions, marking all as 'MPART_FLAG_CHNG'.
    * #6: Copy all data into the allocated scatter memory.
    * #7: (re-)map the new branches in the mman.
    * HINT: The number of new parts equals that of new regions. */
   for (piter = &region->mr_parts;
       (iter  = *piter) != NULL;
        piter = &iter->mt_chain.le_next) {
    raddr_t iter_end; rsize_t part_size;
    if (iter->mt_state != MPART_STATE_INCORE) continue;
    if (iter->mt_start >= region_begin+region_size) break;
    iter_end = iter->mt_chain.le_next ? iter->mt_chain.le_next->mt_start
                                      : region->mr_size;
    assert(iter_end > iter->mt_start);
    if (iter_end <= region_begin) continue;
    /* At least some portion of this part must be loaded in some way. */
    assert(region_begin >= iter->mt_start);
    if (region_begin != iter->mt_start) {
     assert(region_begin < iter_end);
     piter = &iter->mt_chain.le_next;
     iter = mregion_part_split_lo(iter,region_begin);
     if (!iter) goto enomem;
     assert(*piter == iter);
    }
    if (region_begin+region_size < iter_end) {
     if (!mregion_part_split_lo(iter,region_begin+region_size)) goto enomem;
     assert(iter->mt_chain.le_next);
     assert(iter->mt_chain.le_next->mt_start == region_begin+region_size);
     iter_end = region_begin+region_size;
    }
    assertf(iter_end > iter->mt_start,
            "iter_end       = %p\n"
            "iter->mt_start = %p\n",
            iter_end,iter->mt_start);
    part_size = iter_end-iter->mt_start;
    assert(part_size <= region_size);
    assert(iter->mt_start == region_begin);
    /* Check if the part can be inherited. */
    if (iter->mt_refcnt == 1) {
     /* Check for special case: When changing parts of file mappings,
      *                         those parts must be extracted.
      * NOTE: This behavior isn't performed when the region is mapped as shared. */
     if (!(mprot&PROT_SHARED) &&
          (region->mr_init&MREGION_INIT_READTHROUGH))
           goto copy_part;
     /* Mark the part as changed. */
     iter->mt_flags |= MPART_FLAG_CHNG;
    } else {
     struct mbranch *copy_branch;
     struct mregion *part_copy;
     bool inherited_data;
     uintptr_t copy_branch_min;
     uintptr_t copy_branch_max;
     ATREE_SEMI_T(VIRT uintptr_t) addr_semi;
     ATREE_LEVEL_T addr_level;
     struct mbranch **central_override,*central_branch;
copy_part:
     /* Create a new region by copying this part. */
     inherited_data = (iter->mt_refcnt == 1);
     part_copy = mregion_new(GFP_NOFREQ|MMAN_DATAGFP(mspace));
     if unlikely(!part_copy) goto enomem;
     assert(IS_ALIGNED(part_size,PAGESIZE));
     if (inherited_data) {
      /* Inherit data. */
      part_copy->mr_part0.mt_memory = iter->mt_memory;
     } else {
      if (!old_mman) TASK_PDIR_KERNEL_BEGIN(old_mman);
      if (!page_malloc_scatter_in_region_part(&part_copy->mr_part0.mt_memory,
                                               part_size,PAGESIZE,PAGEATTR_NONE,
                                               MZONE_ANY,region,iter,true)) {
       kffree(part_copy,GFP_NOFREQ);
       goto enomem;
      }
      mscatter_memcpy(&part_copy->mr_part0.mt_memory,
                      &iter->mt_memory);
     }
     part_copy->mr_part0.mt_refcnt = 1;
     part_copy->mr_part0.mt_state  = MPART_STATE_INCORE;
     part_copy->mr_part0.mt_flags  = MPART_FLAG_CHNG;
     part_copy->mr_part0.mt_locked = iter->mt_locked;
     part_copy->mr_size            = part_size;
     copy_branch_min = region_base+iter->mt_start;
     copy_branch_max = region_base+iter_end-1;
     if (!*did_remap) {
      /* Overwrite existing mappings with 'copy_branch' */
      assertf(copy_branch_min >= self->mb_node.a_vmin,"%p < %p",copy_branch_min,self->mb_node.a_vmin);
      assertf(copy_branch_max <= self->mb_node.a_vmax,"%p %p %p > %p",
              region_base,copy_branch_min,copy_branch_max,self->mb_node.a_vmax);
      if (copy_branch_min == self->mb_node.a_vmin &&
          copy_branch_max == self->mb_node.a_vmax) {
       /* Special case: Overwrite the entirety of the current branch. */
       assert(iter->mt_start == self->mb_start);
       assert(iter_end       == self->mb_start+MBRANCH_SIZE(self));
       assert(inherited_data == (iter->mt_refcnt == 1));
       if (!--iter->mt_refcnt) iter->mt_state = MPART_STATE_MISSING;
       rwlock_endwrite(&region->mr_plock);
       /* Due to the way we're still holding locks, we're the sole accessor for 'part_copy',
        * meaning we can cheat a bit when it comes to acquiring write-access to its lock. */
       ATOMIC_WRITE(part_copy->mr_plock.rw_lock.arw_lock,ATOMIC_RWLOCK_WFLAG);
       mregion_setup(part_copy);
       self->mb_region = part_copy; /* Inherit reference. */
       self->mb_start  = 0; /* Important: The branch now starts at the base of the copied sub-region. */
       error = mbranch_remap_unlocked(self,&mspace->m_pdir,false);
       rwlock_endwrite(&part_copy->mr_plock);

       MREGION_DECREF(region); /* Drop the old region reference inherited at 'self->mb_region = ...' */
       if (E_ISERR(error)) goto err;
       load_bytes = (ssize_t)part_size;
       goto end2;
      }
     }
     *did_remap = true;
     /* XXX: Below code may not be fully tested? */
     /* XXX (much later): It seems to work just fine. */

     /* Create a new branch for the copied region. */
     copy_branch = (struct mbranch *)kmalloc(sizeof(struct mbranch),
                                             GFP_NOFREQ|MMAN_DATAGFP(mspace));
     if unlikely(!copy_branch) {
/*err_part_copy_nomem:*/
      error = -ENOMEM;
err_part_copy_custom:
      CHECK_HOST_DOBJ(part_copy);
      assert(part_copy->mr_size);
      assert(part_copy->mr_init == MREGION_INIT_RAND);
      assert(part_copy->mr_parts == &part_copy->mr_part0);
      assert(!part_copy->mr_part0.mt_chain.le_next);
      assert(part_copy->mr_part0.mt_chain.le_pself == &part_copy->mr_parts);
      assert(part_copy->mr_part0.mt_start == 0);
      assert(part_copy->mr_part0.mt_state == MPART_STATE_INCORE);
      assert(part_copy->mr_part0.mt_refcnt == 1);
      assert(part_copy->mr_part0.mt_flags&MPART_FLAG_CHNG);
      if (!inherited_data) {
       assertf(old_mman,"Should have already been switched earlier");
       page_free_scatter(&part_copy->mr_part0.mt_memory,PAGEATTR_NONE);
      }
      kffree(part_copy,GFP_NOFREQ);
      goto err;
     }
     copy_branch->mb_node.a_vmin = copy_branch_min;
     copy_branch->mb_node.a_vmax = copy_branch_max;
     copy_branch->mb_prot        = mprot;
     copy_branch->mb_start       = 0;
     copy_branch->mb_notify      = self_notify;
     copy_branch->mb_closure     = self_closure;
     copy_branch->mb_region      = part_copy; /* Inherit reference. */

     /* Incref the notification. */
     if (!holds_notify_ref && self_notify) {
      error = (*self_notify)(MNOTIFY_INCREF,self_closure,mspace,0,0);
      if (E_ISERR(error)) { kffree(copy_branch,GFP_NOFREQ); goto err_part_copy_custom; }
      holds_notify_ref = true;
     }

     /* Split the original branch before and after 'copy_branch' */
     if ((error = mman_split_branch_unlocked(mspace,copy_branch_min),E_ISERR(error)) ||
         (error = mman_split_branch_unlocked(mspace,copy_branch_max+1),E_ISERR(error))) {
      if (self_notify) (*self_notify)(MNOTIFY_DECREF,self_closure,NULL,0,0);
      kffree(copy_branch,GFP_NOFREQ);
      goto err_part_copy_custom;
     }

     /* Delete the branch mapping between the two parts. */
     addr_semi = ATREE_SEMI0(VIRT uintptr_t);
     addr_level = ATREE_LEVEL0(VIRT uintptr_t);
     central_override = mbranch_tree_plocate_at(&mspace->m_map,copy_branch_min,
                                                &addr_semi,&addr_level);
     assertf(central_override,"Failed to find central-override branch at %p",copy_branch_min);
     central_branch = mbranch_tree_pop_at(central_override,addr_semi,addr_level);
     assert(central_branch);

     /* Replace 'central_branch' in the ordered chain with 'copy_branch' */
     LIST_INSERT_REPLACE(central_branch,copy_branch,mb_order);
     /* NOTE: There is a chance that 'central_branch == self', and because
      *       of that, we may no longer access 'self' in any way! */
     assertf(central_branch->mb_region == region,
             "Miss-matching regions at %p in %p...%p (%p != %p)",
             copy_branch_min,
             central_branch->mb_node.a_vmin,
             central_branch->mb_node.a_vmax,
             central_branch->mb_region,region);
     assert(central_branch->mb_node.a_vmin == copy_branch_min);
     assert(central_branch->mb_node.a_vmax == copy_branch_max);
     assertf(central_branch->mb_start == iter->mt_start,
             "region_base              = %p\n"
             "copy_branch_min          = %p\n"
             "copy_branch_max          = %p\n"
             "central_branch->mb_start = %p\n"
             "iter->mt_start           = %p\n",
             region_base,
             copy_branch_min,copy_branch_max,
             central_branch->mb_start,
             iter->mt_start);
     assert(central_branch->mb_start+MBRANCH_SIZE(central_branch) == iter_end);
     assert(MBRANCH_SIZE(central_branch) == part_size);
     /* Drop the reference previously held by this branch mapping. */
     if (holds_region_ref) {
      assert(region->mr_refcnt >= 2);
      ATOMIC_FETCHDEC(region->mr_refcnt);
     } else {
      /* Keep an explicit reference to the original
       * region in case we unmap + destroy it. */
      holds_region_ref = true;
     }
     assert(inherited_data == (iter->mt_refcnt == 1));

     /* Insert the new branch into the address tree. */
     mbranch_tree_insert(&mspace->m_map,copy_branch);
     /*holds_notify_ref = false;*/

     /* Drop a reference from the original part. */
     if (!--iter->mt_refcnt) iter->mt_state = MPART_STATE_MISSING;

     /* Due to the way we're still holding locks, we're the sole accessor for 'part_copy',
      * meaning we can cheat a bit when it comes to acquiring write-access to its lock. */
     ATOMIC_WRITE(part_copy->mr_plock.rw_lock.arw_lock,ATOMIC_RWLOCK_WFLAG);
     /* Setup the part copy for use by SWAP memory. */
     mregion_setup(part_copy);

     /* Remap the copied branch. */
     error = mbranch_remap_unlocked(copy_branch,&mspace->m_pdir,false);
     if (E_ISERR(error)) goto err;
     rwlock_endwrite(&part_copy->mr_plock);

     /* Delete the old central branch. */
     if (central_branch->mb_notify) {
      if (central_branch->mb_notify  != self_notify ||
          central_branch->mb_closure != self_closure) {
       if (self_notify) {
        error = (*self_notify)(MNOTIFY_INCREF,self_closure,mspace,0,0);
        if (E_ISERR(error)) goto err2;
       }
       MBRANCH_DECREF(central_branch);
      }
      /*holds_notify_ref = true;*/
     }
     kffree(central_branch,GFP_NOFREQ);

     load_bytes += part_size;
    }
   }
  }
 }
#endif
end:
 rwlock_endwrite(&region->mr_plock);
end2:
 if (holds_notify_ref) {
  assert(self_notify);
  (*self_notify)(MNOTIFY_DECREF,self_closure,NULL,0,0);
 }
 if (holds_region_ref) MREGION_DECREF(region);
 if (old_mman) TASK_PDIR_KERNEL_END(old_mman);
 return load_bytes;
err: load_bytes = error; goto end;
err2: load_bytes = error; goto end2;
enomem: load_bytes = -ENOMEM; goto end;
}

DECL_END

#endif /* !GUARD_KERNEL_MMAN_MCORE_C_INL */
