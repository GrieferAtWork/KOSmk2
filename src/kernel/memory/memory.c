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
#ifndef GUARD_KERNEL_MEMORY_MEMORY_C
#define GUARD_KERNEL_MEMORY_MEMORY_C 1
#define _KOS_SOURCE 2 /* 'assertf' */

#include "../debug-config.h"

#include <assert.h>
#include <hybrid/align.h>
#include <hybrid/byteorder.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/traceback.h>
#include <kernel/boot.h>
#include <kernel/export.h>
#include <kernel/memory.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <proprietary/multiboot.h>
#include <proprietary/multiboot2.h>
#include <sched/task.h>
#include <string.h>

DECL_BEGIN

/* Define to non-zero to add syslog entries for physical memory allocation. */
#ifndef LOG_PHYSICAL_ALLOCATIONS
#define LOG_PHYSICAL_ALLOCATIONS 0
#endif

#if defined(CONFIG_DEBUG) && 1
#define HAVE_VERIFY_MEMORY_M
#define HAVE_VERIFY_MEMORY_F
#define VERIFY_MEMORY_M(start,n_bytes,attr) debug_verify_memory(start,n_bytes,attr,true)
#define VERIFY_MEMORY_F(start,n_bytes,attr) debug_verify_memory(start,n_bytes,attr,false)
PRIVATE void KCALL
debug_verify_memory(ppage_t start, PAGE_ALIGNED size_t n_bytes,
                    pgattr_t attr, bool after_malloc) {
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)n_bytes,PAGESIZE));
 (void)after_malloc;
 if (attr&PAGEATTR_ZERO) {
  u32 *iter,*end;
  end = (iter = (u32 *)start)+(n_bytes/4);
  for (; iter != end; ++iter) if (*iter != 0) {
   u8 *addr = (u8 *)iter;
   while (!*addr) ++addr;
   assertf(!*iter,"%p (in %p...%p; offset %Id) should be 0x00, but is %#.2I8x\n"
                  "%.?[hex]",
           addr,start,(uintptr_t)start+n_bytes-1,(uintptr_t)addr-(uintptr_t)start,*addr,
           MIN((uintptr_t)end-(uintptr_t)addr,64),addr);
  }
 }
#if 1
 else if (after_malloc) {
  memsetl(start,KERNEL_DEBUG_MEMPAT_PAGE_MALLOC,n_bytes/4);
 }
#endif
}


#else
#define VERIFY_MEMORY_M(start,n_bytes,attr) (void)0
#define VERIFY_MEMORY_F(start,n_bytes,attr) (void)0
#endif



INTERN ATTR_HOTDATA
struct mzone page_zones[MZONE_COUNT] = {
    /* Pre-initialize all zones as empty.
     * NOTE: All zones use physical pointers */
    [0 ... MZONE_COUNT-1] = {
        .z_lock  = ATOMIC_RWLOCK_INIT,
        .z_root  = PAGE_ERROR,
        .z_inuse = 0,
        .z_avail = 0,
    },
};

#define DEVICE_BEGIN  0x000a0000
#define DEVICE_END    0x00100000

PUBLIC struct mzonespec const mzone_spec = {
    .ms_min = {
        [MZONE_1MB]     = (uintptr_t)(0x00000000),
        [MZONE_DEV]     = (uintptr_t)(DEVICE_BEGIN),
        [MZONE_SHARE]   = (uintptr_t)(DEVICE_END),
        [MZONE_NOSHARE] = (uintptr_t)(0-KERNEL_BASE),
    },
    .ms_max = {
        [MZONE_1MB]     = (uintptr_t)(DEVICE_BEGIN-1),
        [MZONE_DEV]     = (uintptr_t)(DEVICE_END-1),
        [MZONE_SHARE]   = (uintptr_t)((0-KERNEL_BASE)-1),
        [MZONE_NOSHARE] = (uintptr_t)(KERNEL_BASE-1),
    },
};


LOCAL void KCALL
mov_pages_lo(void *dst, void const *src, size_t n_bytes) {
 /* NOTE: Intentionally don't check for writable. */
 assert(dst < src);
 CHECK_HOST_TEXT(dst,n_bytes);
 CHECK_HOST_TEXT(src,n_bytes);
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 while (n_bytes) {
  memcpyl(dst,src,PAGESIZE/4);
  n_bytes            -= PAGESIZE;
  *(uintptr_t *)&dst += PAGESIZE;
  *(uintptr_t *)&src += PAGESIZE;
 }
}
LOCAL void KCALL
mov_pages_hi(void *dst, void const *src, size_t n_bytes) {
 /* NOTE: Intentionally don't check for writable. */
 assert(dst > src);
 CHECK_HOST_TEXT(dst,n_bytes);
 CHECK_HOST_TEXT(src,n_bytes);
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 *(uintptr_t *)&dst += n_bytes;
 *(uintptr_t *)&src += n_bytes;
 while (n_bytes) {
  *(uintptr_t *)&dst -= PAGESIZE;
  *(uintptr_t *)&src -= PAGESIZE;
  memcpyl(dst,src,PAGESIZE/4);
  n_bytes            -= PAGESIZE;
 }
}

PUBLIC KPD void KCALL
page_stat(struct mstat *__restrict info) {
 struct mzone *zone;
 struct mzstat *zinfo;
 assert(PDIR_ISKPD());
 CHECK_HOST_DOBJ(info);
 info->m_total.mz_avail   = 0;
 info->m_total.mz_inuse   = 0;
 info->m_total.mz_freemin = (size_t)-1;
 info->m_total.mz_freemax = 0;
 info->m_total.mz_freeblk = 0;
 info->m_total.mz_zeromin = (size_t)-1;
 info->m_total.mz_zeromax = 0;
 info->m_total.mz_zeroblk = 0;
 zinfo = info->m_zones;
 PAGEZONES_FOREACH(zone) {
  ppage_t iter;
  task_crit();
  atomic_rwlock_read(&zone->z_lock);
  zinfo->mz_avail = zone->z_avail;
  zinfo->mz_inuse = zone->z_inuse;
  PAGE_FOREACH(iter,zone) {
   if (zinfo->mz_freemin > iter->p_free.p_size)
       zinfo->mz_freemin = iter->p_free.p_size;
   if (zinfo->mz_freemax < iter->p_free.p_size)
       zinfo->mz_freemax = iter->p_free.p_size;
   ++zinfo->mz_freeblk;
   if (iter->p_free.p_attr&PAGEATTR_ZERO) {
    if (zinfo->mz_zeromin > iter->p_free.p_size)
        zinfo->mz_zeromin = iter->p_free.p_size;
    if (zinfo->mz_zeromax < iter->p_free.p_size)
        zinfo->mz_zeromax = iter->p_free.p_size;
    ++zinfo->mz_zeroblk;
   }
  }
  atomic_rwlock_endread(&zone->z_lock);
  task_endcrit();
  info->m_total.mz_avail   += zinfo->mz_avail;
  info->m_total.mz_inuse   += zinfo->mz_inuse;
  info->m_total.mz_freeblk += zinfo->mz_freeblk;
  info->m_total.mz_zeroblk += zinfo->mz_zeroblk;
  if (info->m_total.mz_freemin > zinfo->mz_freemin)
      info->m_total.mz_freemin = zinfo->mz_freemin;
  if (info->m_total.mz_zeromin > zinfo->mz_zeromin)
      info->m_total.mz_zeromin = zinfo->mz_zeromin;
  if (info->m_total.mz_freemax < zinfo->mz_freemax)
      info->m_total.mz_freemax = zinfo->mz_freemax;
  if (info->m_total.mz_zeromax < zinfo->mz_zeromax)
      info->m_total.mz_zeromax = zinfo->mz_zeromax;
  if unlikely(zinfo->mz_freemin == (size_t)-1) zinfo->mz_freemin = 0;
  if unlikely(zinfo->mz_zeromin == (size_t)-1) zinfo->mz_zeromin = 0;
  ++zinfo;
 }
 if unlikely(info->m_total.mz_freemin == (size_t)-1) info->m_total.mz_freemin = 0;
 if unlikely(info->m_total.mz_zeromin == (size_t)-1) info->m_total.mz_zeromin = 0;
}



/* Query page attributes, returning a set of 'PAGEATTR_*' */
PUBLIC KPD pgattr_t KCALL
page_query(PHYS void *start, size_t n_bytes) {
 ppage_t iter; size_t found = 0;
 pgattr_t result = PAGEATTR_ALLZERO|PAGEATTR_ALLFREE;
 void *addr_end = (void *)((uintptr_t)start+n_bytes);
 struct mzone *z_iter,*z_end;
 assert(PDIR_ISKPD());
 z_iter = PAGEZONE(mzone_of(start));
 z_end  = PAGEZONE(mzone_of((void *)((uintptr_t)addr_end-1))+1);
 if unlikely(!n_bytes) goto end;
 for (; z_iter != z_end; ++z_iter) {
  task_crit();
  atomic_rwlock_read(&z_iter->z_lock);
  PAGE_FOREACH(iter,z_iter) {
   if ((uintptr_t)addr_end >= (uintptr_t)iter &&
       (uintptr_t)start    <  (uintptr_t)iter+iter->p_free.p_size) {
    result |= (iter->p_free.p_attr);
    result &= (iter->p_free.p_attr << PAGEATTR_ALLSHIFT);
    if (found && start != iter)
        result &= ~(PAGEATTR_ALLFREE);
   }
  }
  atomic_rwlock_endread(&z_iter->z_lock);
  task_endcrit();
 }
end:
 if (!found) result = PAGEATTR_NONE;
 return result;
}


PUBLIC SAFE KPD ppage_t KCALL
page_malloc(size_t n_bytes, pgattr_t attr, mzone_t zone_id) {
 struct mzone *zone;
 zone = PAGEZONE(zone_id);
 do {
  bool has_write_lock = false;
  ppage_t iter,best_page = PAGE_ERROR;
  size_t best_size = (size_t)-1;
  atomic_rwlock_read(&zone->z_lock);
search_zone:
  /* Search for the most suitable free range. */
  PAGE_FOREACH(iter,zone) {
#if 0
   syslog(LOG_DEBUG,"zone #%d: %p...%p\n",
          (int)(zone-page_zones),iter,(uintptr_t)PAGE_END(iter)-1);
#endif
   assertf(iter->p_free.p_next == PAGE_ERROR ||
           iter->p_free.p_next >  PAGE_END(iter),
           "Miss-ordered memory zone: %p...%p overlaps with %p...%p",
           iter,(uintptr_t)PAGE_END(iter)-1,iter->p_free.p_next,
          (uintptr_t)PAGE_END(iter->p_free.p_next)-1);
   if (iter->p_free.p_size < n_bytes) continue;
   if (iter->p_free.p_size > best_size) continue;
   if (iter->p_free.p_size == best_size) {
    /* Still prefer same-size zones if their attributes match better. */
    if ((best_page->p_free.p_attr&PAGEATTR_ZERO) ==
        (attr&PAGEATTR_ZERO)) continue;
   }
   best_page = iter;
   best_size = iter->p_free.p_size;
  }

  /* Try to allocate from the best fitting page
   * if we've managed to located any at all. */
  if (best_page != PAGE_ERROR) {
   ppage_t result; bool must_clear = false;
   assert(best_size >= n_bytes);
   n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
   assert(best_size >= n_bytes);
   if (!has_write_lock) {
    has_write_lock = true;
    if (!atomic_rwlock_upgrade(&zone->z_lock))
         goto search_zone;
   }
   assert(best_size == best_page->p_free.p_size);
   result = (ppage_t)((uintptr_t)best_page+(best_size-n_bytes));
   assert(IS_ALIGNED((uintptr_t)best_page,PAGESIZE));
   assert(IS_ALIGNED((uintptr_t)result,PAGESIZE));
   if (result == best_page) {
    /* Allocate this whole part. */
    *best_page->p_free.p_self = best_page->p_free.p_next;
    if (best_page->p_free.p_next != PAGE_ERROR)
        best_page->p_free.p_next->p_free.p_self = best_page->p_free.p_self;
   } else {
    /* Trim the part near its upper end. */
    best_page->p_free.p_size -= n_bytes;
   }

   if (attr&PAGEATTR_ZERO) {
    if (!(best_page->p_free.p_attr&PAGEATTR_ZERO)) {
     /* Clear the entirety of the generated region. */
     must_clear = true;
    } else if (result == best_page) {
     /* Clear the page controller. */
     result->p_free.p_next = 0;
     result->p_free.p_self = 0;
     result->p_free.p_size = 0;
     result->p_free.p_attr = 0;
    }
   }

   /* Update zone statistics. */
   zone->z_inuse += n_bytes;
   zone->z_free  -= n_bytes;
   ++zone->z_alloc;
   atomic_rwlock_endwrite(&zone->z_lock);

#if LOG_PHYSICAL_ALLOCATIONS
   syslog(LOG_MEM|LOG_DEBUG,
           "[MEM] Allocated memory %p...%p from zone #%d (#%d)\n",
           result,(uintptr_t)result+(n_bytes-1),
          (int)(zone-page_zones),zone_id);
   if (zone_id == 0) __assertion_tbprint(0);
#endif /* LOG_PHYSICAL_ALLOCATIONS */

   if (must_clear) memsetl(result,0,n_bytes/4);
   VERIFY_MEMORY_M(result,n_bytes,attr);
   return result;
  }
  if (has_write_lock)
       atomic_rwlock_endwrite(&zone->z_lock);
  else atomic_rwlock_endread (&zone->z_lock);
 } while (zone-- != page_zones);
 return PAGE_ERROR;
}

PUBLIC SAFE KPD ppage_t KCALL
page_malloc_in(ppage_t min, ppage_t max,
              size_t n_bytes, pgattr_t attr) {
 struct mzone *zone,*zone_min;
 bool must_clear = false;
 ppage_t iter,result = PAGE_ERROR;
 assert(IS_ALIGNED((uintptr_t)min,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)max,PAGESIZE));
 n_bytes  = CEIL_ALIGN(n_bytes,PAGESIZE);
 /* Figure out what zones we're allocated to allocate from. */
 zone     = PAGEZONE(mzone_of(max));
 zone_min = PAGEZONE(mzone_of(min));
 for (; zone >= zone_min; --zone) {
  bool has_write_lock = false;
  ppage_t best_page;
  size_t  best_size;
  atomic_rwlock_read(&zone->z_lock);
scan_zone:
  best_page = PAGE_ERROR;
  best_size = (size_t)-1;
  PAGE_FOREACH(iter,zone) {
   ppage_t iter_end; size_t page_avail;
   if (iter > max) break;
   iter_end = (ppage_t)((uintptr_t)iter+iter->p_free.p_size);
   if (iter_end <= min) continue;
   if ((uintptr_t)iter_end > (uintptr_t)max+n_bytes)
        iter_end = (ppage_t)((uintptr_t)max+n_bytes);
   page_avail = (uintptr_t)iter_end-(uintptr_t)min;
   if (page_avail > iter->p_free.p_size)
       page_avail = iter->p_free.p_size;
   if (page_avail < n_bytes) continue;
   /* We can allocate from this page!
    * >> If it's a better fit that the next smaller, use this one. */
   if (page_avail < best_size) {
    best_page = iter;
    best_size = page_avail;
    /* It can't get better than 'n_bytes' itself! */
    if (page_avail == n_bytes) break;
   }
  }
  if (best_page != PAGE_ERROR) {
   pgattr_t part_attr;
   has_write_lock = true;
   if (!atomic_rwlock_upgrade(&zone->z_lock))
        goto scan_zone;
   /* Use the candidate. */
   assert(best_page->p_free.p_size >= n_bytes);
   /* Take away from the free range's top. */
   part_attr = best_page->p_free.p_attr;
   result    = (ppage_t)((uintptr_t)best_page+(best_page->p_free.p_size-n_bytes));
   if (result > max) {
    /* This can happen when 'best_page' includes 'max' */
    assert((uintptr_t)max >= (uintptr_t)best_page);
    assert((uintptr_t)max <= (uintptr_t)best_page+best_page->p_free.p_size);
    result = max;
   }
   assert((uintptr_t)result         >= (uintptr_t)min);
   assert((uintptr_t)result         <= (uintptr_t)max);
   assert((uintptr_t)result         >= (uintptr_t)best_page);
   assert((uintptr_t)result+n_bytes <= (uintptr_t)best_page+best_page->p_free.p_size);
   if ((uintptr_t)result == (uintptr_t)best_page) {
    if (n_bytes == best_page->p_free.p_size) {
     /* The allocation consumes the entirety of this free range. */
     assert(n_bytes == best_size);
     /* Unlink the resulting page part. */
     if (best_page->p_free.p_next != PAGE_ERROR)
         best_page->p_free.p_next->p_free.p_self = best_page->p_free.p_self;
     *best_page->p_free.p_self = best_page->p_free.p_next;
    } else {
     ppage_t new_base;
     /* Allocate the requested portion at the page's bottom. */
     assert(n_bytes < best_page->p_free.p_size);
     new_base = (ppage_t)((uintptr_t)best_page+n_bytes);
     new_base->p_free         = best_page->p_free;
     new_base->p_free.p_size -= n_bytes;
     if (new_base->p_free.p_next != PAGE_ERROR)
         new_base->p_free.p_next->p_free.p_self = &new_base->p_free.p_next;
     *new_base->p_free.p_self = new_base;
    }
    /* Must clear the free-data controller if it is allocated. */
    if ((part_attr&PAGEATTR_ZERO) &&
        (attr&PAGEATTR_ZERO)) {
     best_page->p_free.p_next = 0;
     best_page->p_free.p_self = 0;
     best_page->p_free.p_size = 0;
     best_page->p_free.p_attr = 0;
    }
   } else {
    if ((uintptr_t)result+n_bytes !=
        (uintptr_t)best_page+best_page->p_free.p_size) {
     /* Allocate portion in-between, creating two splits. */
     ppage_t hi_split = (ppage_t)((uintptr_t)result+n_bytes);
     assert((uintptr_t)hi_split < (uintptr_t)best_page+best_page->p_free.p_size);
     hi_split->p_free = best_page->p_free;
     if (hi_split->p_free.p_next != PAGE_ERROR)
         hi_split->p_free.p_next->p_free.p_self = &hi_split->p_free.p_next;
     hi_split->p_free.p_self   = &best_page->p_free.p_next;
     hi_split->p_free.p_size  -= (size_t)((uintptr_t)hi_split-(uintptr_t)best_page);
     best_page->p_free.p_next  = hi_split;
     best_page->p_free.p_size -= n_bytes;
    }
    /* Now just truncate the free region near its end. */
    best_page->p_free.p_size = (size_t)((uintptr_t)result-(uintptr_t)best_page);
   }

   /* Force zero-initialization if the page we've decided
    * to use isn't already, but was requested to be. */
   if (!(part_attr&PAGEATTR_ZERO) &&
        (attr&PAGEATTR_ZERO))
         must_clear = true;
   assert(has_write_lock);
   ++zone->z_alloc;
   zone->z_inuse += n_bytes;
   zone->z_avail -= n_bytes;
  }
  /* Release our zone lock. */
  if (has_write_lock)
       atomic_rwlock_endwrite(&zone->z_lock);
  else atomic_rwlock_endread (&zone->z_lock);
  if (result != PAGE_ERROR) break;
 }
 assert(result == PAGE_ERROR ||
       (result >= min && result <= max));
 if (must_clear) memsetl(result,0,n_bytes/4);
#ifdef HAVE_VERIFY_MEMORY_M
 if (result != PAGE_ERROR)
     VERIFY_MEMORY_M(result,n_bytes,attr);
#endif
#if 0
 { extern void *mall_check_ptr;
   if (mall_check_ptr) COMPILER_UNUSED(malloc_usable_size(mall_check_ptr));
 }
#endif
 return result;
}


PUBLIC SAFE KPD ppage_t KCALL
page_malloc_part(size_t min_size, size_t max_size,
                 size_t *__restrict res_size,
                 pgattr_t attr, mzone_t zone) {
 /* TODO: Do this for real! (The specs are all there!) */
 *res_size = min_size;
 return page_malloc(min_size,attr,zone);
}


PUBLIC SAFE KPD bool KCALL
mscatter_split_lo(struct mscatter *__restrict dst,
                  struct mscatter *__restrict src,
                  uintptr_t offset_from_src) {
 assert(PDIR_ISKPD());
 CHECK_HOST_DOBJ(dst);
 CHECK_HOST_DOBJ(src);
 /* TODO */
 return false;
}

PUBLIC KPD void KCALL
mscatter_memcpy(struct mscatter const *__restrict dst,
                struct mscatter const *__restrict src) {
 PAGE_ALIGNED uintptr_t src_iter; size_t src_size;
 PAGE_ALIGNED uintptr_t dst_iter; size_t dst_size;
 assert(PDIR_ISKPD());
 CHECK_HOST_DOBJ(src);
 CHECK_HOST_DOBJ(dst);
 assert((dst->m_size != 0) ==
        (src->m_size != 0));
 src_size = src->m_size;
 if (!src_size) return;
 src_iter = (PAGE_ALIGNED uintptr_t)src->m_start;
 dst_iter = (PAGE_ALIGNED uintptr_t)dst->m_start;
 dst_size = dst->m_size;
 for (;;) {
  size_t cpy_size = MIN(src_size,dst_size);
  memcpyl((void *)dst_iter,(void *)src_iter,cpy_size/4);
  if ((dst_size -= cpy_size) == 0) {
   if ((dst = dst->m_next) == 0) break;
   dst_iter = (PAGE_ALIGNED uintptr_t)dst->m_start;
   dst_size = dst->m_size;
  } else dst_iter += cpy_size;
  if ((src_size -= cpy_size) == 0) {
   if ((src = src->m_next) == 0) break;
   src_iter = (PAGE_ALIGNED uintptr_t)src->m_start;
   src_size = src->m_size;
  } else src_iter += cpy_size;
 }
}



PUBLIC SAFE KPD bool KCALL
page_malloc_scatter(struct mscatter *__restrict scatter,
                    size_t n_bytes, size_t min_scatter,
                    pgattr_t attr, mzone_t zone) {
 struct mscatter *s_next;
 CHECK_HOST_DOBJ(scatter);
#if 1
 if (!n_bytes) {
  scatter->m_next = NULL;
  scatter->m_size = 0;
  return true;
 }
#endif
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 min_scatter = CEIL_ALIGN(min_scatter,PAGESIZE);
 if (min_scatter > n_bytes)
     min_scatter = n_bytes;
 if (!min_scatter) min_scatter = PAGESIZE;
 /* Terminate the chain of scatter entries. */
 scatter->m_next = NULL;
 for (;;) {
  /* Allocate a portion of what we're supposed to load. */
  assert(n_bytes);
  scatter->m_start = page_malloc_part(min_scatter,n_bytes,&scatter->m_size,attr,zone);
  if unlikely(scatter->m_start == PAGE_ERROR) { scatter->m_size = 0; goto nomem; }
  assert(n_bytes >= scatter->m_size);
  assert(IS_ALIGNED(scatter->m_size,PAGESIZE));
  assert(scatter->m_size != 0);
  if (n_bytes == scatter->m_size) break;
  /* Allocate another scatter link. */
  n_bytes -= scatter->m_size;
  s_next   = (struct mscatter *)kmalloc(sizeof(struct mscatter),
                                        GFP_SHARED|GFP_NOFREQ);
  if unlikely(!s_next) goto nomem;
  *s_next  = *scatter;
  scatter->m_next = s_next;
 }
 return true;
nomem:
 /* Free everything that was already allocated. */
 { struct mscatter *s_iter = scatter;
   do {
    s_next = s_iter->m_next;
    page_free_(s_iter->m_start,s_iter->m_size,attr);
    if (s_iter != scatter) kffree(s_iter,GFP_NOFREQ);
   } while ((s_iter = s_next) != NULL);
 }
 return false;
}

PUBLIC SAFE KPD ssize_t KCALL
page_malloc_all(struct mscatter *__restrict scatter, ppage_t begin,
                size_t n_bytes, pgattr_t attr) {
 CHECK_HOST_DOBJ(scatter);
 (void)scatter;
 (void)begin;
 (void)n_bytes;
 (void)attr;
 /* TODO */
 return -ENOMEM;
}



PUBLIC SAFE KPD ppage_t KCALL
page_realloc_inplace(ppage_t start, size_t old_bytes,
                     size_t new_bytes, pgattr_t attr) {
 if unlikely(!old_bytes) return PAGE_ERROR;
 old_bytes = CEIL_ALIGN(old_bytes,PAGESIZE);
 new_bytes = CEIL_ALIGN(new_bytes,PAGESIZE);
 if (old_bytes > new_bytes) {
  page_free((ppage_t)((uintptr_t)start+new_bytes),old_bytes-new_bytes);
 } else if (old_bytes < new_bytes) {
  size_t diff = new_bytes-old_bytes;
  /* Try to allocate directly above. */
  if (page_malloc_at((ppage_t)((uintptr_t)start+old_bytes),diff,attr) == PAGE_ERROR)
      return PAGE_ERROR;
 }
 return start;
}

PUBLIC SAFE KPD ppage_t KCALL
page_realloc(ppage_t start, size_t old_bytes,
             size_t new_bytes, pgattr_t attr,
             mzone_t zone) {
 if (!old_bytes) return page_malloc(new_bytes,attr,zone);
 old_bytes = CEIL_ALIGN(old_bytes,PAGESIZE);
 new_bytes = CEIL_ALIGN(new_bytes,PAGESIZE);
 if (old_bytes > new_bytes) {
  page_free((ppage_t)((uintptr_t)start+new_bytes),old_bytes-new_bytes);
 } else if (old_bytes < new_bytes) {
  ppage_t result; size_t diff = new_bytes-old_bytes;
  assert(IS_ALIGNED(diff,PAGESIZE));

  /* Try to allocate directly above. */
  if (page_malloc_at((ppage_t)((uintptr_t)start+old_bytes),diff,attr) != PAGE_ERROR) goto end;
  /* Try to allocate directly below. */
  result = page_malloc_at((ppage_t)((uintptr_t)start-diff),diff,
                           attr&~(PAGEATTR_ZERO));
  if (result != PAGE_ERROR) {
   mov_pages_lo(result,start,old_bytes);
   if (attr&PAGEATTR_ZERO) memsetl((void *)((uintptr_t)result+old_bytes),0,diff/4);
   return result;
  }
  /* Allocate a whole new region. */
  if (attr&PAGEATTR_ZERO && diff < old_bytes) {
   result = page_malloc(new_bytes,attr&~(PAGEATTR_ZERO),zone);
   if unlikely(result == PAGE_ERROR) return PAGE_ERROR;
   memsetl((void *)((uintptr_t)result+old_bytes),0,diff/4);
  } else {
   result = page_malloc(new_bytes,attr,zone);
   if unlikely(result == PAGE_ERROR) return PAGE_ERROR;
  }
  assert(IS_ALIGNED(old_bytes,PAGESIZE));
  memcpyl(result,start,old_bytes/4);
  page_free(start,old_bytes);
  return result;
 }
end:
 return start;
}

PUBLIC SAFE KPD ssize_t KCALL
page_print(mzone_t zone_id,
           pformatprinter printer,
           void *closure) {
 struct mzone *zone; ppage_t iter;
 ssize_t result = 0,temp;
 assert(PDIR_ISKPD());
 zone = PAGEZONE(zone_id);
 atomic_rwlock_read(&zone->z_lock);
 PAGE_FOREACH(iter,zone) {
  temp = format_printf(printer,closure,"Free memory %p...%p\n",
                       iter,(uintptr_t)iter+iter->p_free.p_size-1);
  if (temp < 0) { result = temp; break; }
  result += temp;
 }
 atomic_rwlock_endread(&zone->z_lock);
 return result;
}



#define page_free_initial(start,n_bytes,attr) \
 (ATOMIC_FETCHADD(page_inuse,n_bytes),page_free_(start,n_bytes,attr))

#undef KERNEL_BEGIN
#undef KERNEL_END

/* We must work with the physical variants below! */
#define KERNEL_BEGIN ((uintptr_t)__kernel_start-KERNEL_BASE)
#define KERNEL_END   ((uintptr_t)__kernel_end-KERNEL_BASE)


/* Add the given memory range as a RAM-range within meminfo. */
PRIVATE SAFE KPD void KCALL memory_register(PAGE_ALIGNED ppage_t start, PAGE_ALIGNED size_t n_bytes, bool without_info);
PRIVATE SAFE KPD void KCALL memory_register_info(PAGE_ALIGNED ppage_t start, PAGE_ALIGNED size_t n_bytes);

#define MEMORY_INSTALL_MODE_NORMAL   0
#define MEMORY_INSTALL_MODE_NOINFO   1
#define MEMORY_INSTALL_MODE_ONLYINFO 2
PRIVATE ATTR_FREETEXT SAFE KPD size_t KCALL
memory_do_install(PHYS PAGE_ALIGNED uintptr_t start,
                       PAGE_ALIGNED size_t size,
                       int mode) {
 assert(IS_ALIGNED(start,PAGESIZE));
 assert(IS_ALIGNED(size,PAGESIZE));
 /* Align the given memory region by whole pages. */
 assert(IS_ALIGNED(KERNEL_BEGIN,PAGESIZE));
 assert(IS_ALIGNED(KERNEL_END,PAGESIZE));
 if (start+size < start) size = 0-start;
 /* NOTE: We must not allow physical memory within the kernel's virtual address space.
  *       All addresses '>= KERNEL_BASE' are mapped to the same places in _all_
  *       page directories, and must therefor not describe physical addresses. */
 if (start      >= KERNEL_BASE) return 0;
 if (start+size >= KERNEL_BASE) size = KERNEL_BASE-(start+size);

 /* Make sure not to allocate memory across the kernel. */
 if (start < KERNEL_BEGIN) {
  /* Free region is located before the kernel. */
  uintptr_t max_size = KERNEL_BEGIN-start;
  if (start+size > KERNEL_END)
      memory_do_install(KERNEL_END,(start+size)-KERNEL_END,mode);
  if (size > max_size)
      size = max_size;
 } else if (start < KERNEL_END) {
  /* Free region begins inside the kernel. */
  uintptr_t reserved_size = KERNEL_END-start;
  start += reserved_size;
  if (size < reserved_size) size = 0;
  else size -= reserved_size;
 }

#ifdef CONFIG_RESERVE_NULL_PAGE
 /* Don't consider memory at NULL */
 if (start == (uintptr_t)NULL && size) {
     start += PAGESIZE;
     size  -= PAGESIZE;
 }
#endif

 /* Mark the region as available. */
 if (size) {
  if (mode != MEMORY_INSTALL_MODE_ONLYINFO) {
   syslog(LOG_MEM|LOG_INFO,
           FREESTR("[MEM] Using dynamic memory %p..%p\n"),
           start,start+size-1);
   memory_register((ppage_t)start,size,mode == MEMORY_INSTALL_MODE_NOINFO);
  } else {
   memory_register_info((ppage_t)start,size);
  }
 }
 return size;
}


struct memrange {
 PAGE_ALIGNED uintptr_t mr_begin; /*< Start address of this no-touch range (first address protected) */
 PAGE_ALIGNED uintptr_t mr_end;   /*< End address of this no-touch range (first address no longer protected). */
};

PRIVATE ATTR_FREEDATA
struct memrange nt[MEMORY_NOTOUCH_MAXCOUNT] = {
    [0 ... MEMORY_NOTOUCH_MAXCOUNT-1] = {
        .mr_begin = 0,
        .mr_end   = 0,
    },
};
PRIVATE ATTR_FREEDATA
struct memrange fl[MEMORY_FREELATER_MAXCOUNT] = {
    [0 ... MEMORY_FREELATER_MAXCOUNT-1] = {
        .mr_begin = 0,
        .mr_end   = 0,
    },
};

PRIVATE ATTR_FREETEXT SAFE KPD bool KCALL
memory_install_free_later(PHYS PAGE_ALIGNED uintptr_t start,
                               PAGE_ALIGNED uintptr_t mend) {
 struct memrange *iter;
 assert(IS_ALIGNED(start,PAGESIZE));
 assert(IS_ALIGNED(mend,PAGESIZE));
 assert(start <= mend);
 if unlikely(start == mend) return true;
 for (iter = fl; iter < COMPILER_ENDOF(fl); ++iter) {
  /* Extend an overlapping no-touch region. */
  if (start <= iter->mr_end && mend >= iter->mr_begin) {
   if (iter->mr_begin > start) iter->mr_begin = start;
   if (iter->mr_end   < mend)  iter->mr_end   = mend;
   return true;
  }
  /* Create a new region in the first free slot. */
  if (iter->mr_begin == iter->mr_end) {
   iter->mr_begin = start;
   iter->mr_end   = mend;
   return true;
  }
 }
 return false;
}

INTERN ATTR_FREETEXT SAFE KPD size_t KCALL
memory_install(PHYS uintptr_t start, size_t size) {
 struct memrange *iter; uintptr_t mend,temp;
 /* Fix the given memory range potentially not being page-aligned. */
 temp  = CEIL_ALIGN(start,PAGESIZE);
 size -= temp-start,start = temp;
 size  = FLOOR_ALIGN(size,PAGESIZE);
 mend  = start+size;
 /* Fix overflow. */
 if unlikely(mend < start) {
#define END_OFFSET 1
  size = (END_OFFSET-start);
  mend = (uintptr_t)-END_OFFSET;
  assert(mend == start+size);
#undef END_OFFSET
 }
 for (iter = nt; iter < COMPILER_ENDOF(nt); ++iter) {
  if (mend <= iter->mr_begin || start >= iter->mr_end) continue;
  size_t result = 0;
  assert(mend > iter->mr_begin && start < iter->mr_end);
  /* Install memory before & after this no-touch guard. */
  if (mend  > iter->mr_end)   result += memory_install(iter->mr_end,mend-iter->mr_end),mend = iter->mr_end;
  if (start < iter->mr_begin) result += memory_install(start,iter->mr_begin-start),start = iter->mr_begin;
  /* Mark this portion of the no-touch guard as free-later. */
  assert(start < mend);
  if (!memory_install_free_later(start,mend)) {
   syslog(LOG_MEM|LOG_WARN,
           FREESTR("[MEM] Insufficient free-later-ranges to mark %p...%p\n"),
           start,mend-1);
  }
  return result;
 }
 return memory_do_install(start,size,MEMORY_INSTALL_MODE_NORMAL);
}

INTERN ATTR_FREETEXT SAFE KPD bool KCALL
memory_notouch(PHYS uintptr_t start, size_t size) {
 struct memrange *iter;
 uintptr_t mend = start+size;
 /* Fix the given memory range potentially not being page-aligned. */
 size  += start & (PAGESIZE-1);
 start &= ~(PAGESIZE-1);
 size   = CEIL_ALIGN(size,PAGESIZE);
 mend   = start+size;
 if unlikely(!size) return true;
 for (iter = nt; iter < COMPILER_ENDOF(nt); ++iter) {
  /* Extend an overlapping no-touch region. */
  if (start <= iter->mr_end && mend >= iter->mr_begin) {
   if (iter->mr_begin > start) iter->mr_begin = start;
   if (iter->mr_end   < mend)  iter->mr_end   = mend;
   return true;
  }
  /* Create a new region in the first free slot. */
  if (iter->mr_begin == iter->mr_end) {
   iter->mr_begin = start;
   iter->mr_end   = mend;
   return true;
  }
 }
 return false;
}
INTERN ATTR_FREETEXT SAFE KPD size_t KCALL
memory_done_install(void) {
 struct memrange *iter; size_t result = 0;
 for (iter = fl; iter < COMPILER_ENDOF(fl); ++iter) {
  result += memory_do_install(iter->mr_begin,
                             (size_t)(iter->mr_end-iter->mr_begin),
                              MEMORY_INSTALL_MODE_NOINFO);
 }
#ifdef CONFIG_DEBUG
 /* Enumerate all zones to check for proper linkage. */
 { ppage_t iter;
   PAGE_FOREACH(iter,&page_zones[0]) {}
   PAGE_FOREACH(iter,&page_zones[1]) {}
   PAGE_FOREACH(iter,&page_zones[2]) {}
   PAGE_FOREACH(iter,&page_zones[3]) {}
 }
#endif
 return result;
}
INTERN ATTR_FREETEXT SAFE KPD void KCALL
memory_mirror_freelater_info(void) {
 struct memrange *iter; size_t result = 0;
 for (iter = fl; iter < COMPILER_ENDOF(fl); ++iter) {
  result += memory_do_install(iter->mr_begin,
                             (size_t)(iter->mr_end-iter->mr_begin),
                              MEMORY_INSTALL_MODE_ONLYINFO);
 }
}

#if __SIZEOF_POINTER__ < 8
PRIVATE ATTR_FREETEXT SAFE KPD size_t KCALL
memory_install64(u64 begin, u64 size) {
 uintptr_t used_begin; size_t used_size;
 used_begin = (uintptr_t)begin;
 used_size  = (uintptr_t)size;
 if ((u64)used_begin < begin) return 0;
 if ((u64)used_size < size) used_size = (size_t)-1;
 return memory_install(used_begin,used_size);
}
#else
#define memory_install64   memory_install
#endif

INTERN ATTR_FREETEXT SAFE KPD size_t KCALL
memory_load_mb_lower_upper(u32 mem_lower, u32 mem_upper) {
 syslog(LOG_MEM|LOG_INFO,
         "TODO: memory_load_mb_lower_upper: %I32u / %I32u\n",
         mem_lower,mem_upper);
 return 0;
}

INTERN ATTR_FREETEXT SAFE KPD size_t KCALL
memory_load_mb_mmap(struct mb_mmap_entry *__restrict iter, u32 info_len) {
 mb_memory_map_t *end; size_t result = 0;
 for (end  = (mb_memory_map_t *)((uintptr_t)iter+info_len); iter < end;
      iter = (mb_memory_map_t *)((uintptr_t)&iter->addr+iter->size)) {
  if (iter->type != MB_MEMORY_AVAILABLE) continue;
  result += memory_install64(iter->addr,iter->len);
 }
 return result;
}

INTERN ATTR_FREETEXT SAFE KPD size_t KCALL
memory_load_mb2_mmap(struct mb2_tag_mmap *__restrict info) {
 mb2_memory_map_t *iter,*end; size_t result = 0;
 iter = info->entries;
 end  = (mb2_memory_map_t *)((uintptr_t)info+info->size);
 if unlikely(!info->entry_size) goto done;
 for (; iter < end; *(uintptr_t *)&iter += info->entry_size) {
  if (iter->type != MB2_MEMORY_AVAILABLE) continue;
  result += memory_install64(iter->addr,iter->len);
 }
done:
 return result;
}


#if 0
INTERN ATTR_FREETEXT SAFE KPD void KCALL
memory_load_mbinfo(mb_info_t *__restrict info) {
 /* Address of the commandline memory (or '(void *)-1' when not available) */
 ppage_t cmd_page; char *cmd_addr = (char *)-1;
 size_t cmd_reserve,cmd_size = 0;
 if (info->flags&MB_INFO_CMDLINE) {
  cmd_addr = (char *)info->cmdline;
  if unlikely(!cmd_addr) cmd_addr = (char *)-1;
  else cmd_size = strlen(cmd_addr);
 }
 if (cmd_addr == (char *)-1 && cmd_size != 0) {
  cmd_page    = 0;
  cmd_reserve = 0;
 } else {
  cmd_page    = (ppage_t)FLOOR_ALIGN((uintptr_t)cmd_addr,PAGESIZE);
  cmd_reserve = CEIL_ALIGN(((uintptr_t)cmd_addr-(uintptr_t)cmd_page)+cmd_size,PAGESIZE);
 }

 if (info->flags&MB_INFO_MEMORY) {
  mb_memory_map_t *iter,*end;
  for (iter = (mb_memory_map_t *)info->mmap_addr,
       end  = (mb_memory_map_t *)((uintptr_t)iter+info->mmap_length); iter < end;
       iter = (mb_memory_map_t *)((uintptr_t)&iter->addr+iter->size)) {
   if (iter->type == 1) {
    u64 begin = iter->addr;
    u64 size  = iter->len;
#if __SIZEOF_POINTER__ < 8
    uintptr_t used_begin; size_t used_size;
#else
#define used_begin begin
#define used_size  size
#endif
    if (begin+size <= begin) continue;
    /* NOTE: We must not allow physical memory within the kernel's virtual address space.
     *       All addresses '>= KERNEL_BASE' are mapped to the same places in _all_
     *       page directories, and must therefor not describe physical addresses. */
    if (begin      >= KERNEL_BASE) continue;
    if (begin+size >= KERNEL_BASE) size = KERNEL_BASE-(begin+size);
    if (!size) continue;
#if __SIZEOF_POINTER__ < 8
    used_begin = (uintptr_t)begin;
    used_size  = (uintptr_t)size;
#endif
    /* Make sure to allocate all pages for the CMD prematurely. */
    if (used_begin < (uintptr_t)cmd_page) {
     /* Free region is located before the kernel. */
     uintptr_t max_size = (uintptr_t)cmd_page-used_begin;
     if (used_begin+used_size > (uintptr_t)cmd_page+cmd_reserve) {
      memory_install((uintptr_t)cmd_page+cmd_reserve,
                     (used_begin+used_size)-
                     (uintptr_t)cmd_page+cmd_reserve);
     }
     if (used_size > max_size)
         used_size = max_size;
    } else if (used_begin < (uintptr_t)cmd_page+cmd_reserve) {
     /* Free region begins inside the kernel. */
     uintptr_t reserved_size = ((uintptr_t)cmd_page+cmd_reserve)-
                                used_begin;
     used_size += reserved_size;
     if (used_size < reserved_size) used_size = 0;
     else used_size -= reserved_size;
    }
    memory_install(used_begin,
                   used_size);
#undef used_begin
#undef used_size
   }
  }
 } else {
  /* Search for memory ourself. */
  memory_load_detect();
 }

#ifdef CONFIG_DEBUG
 /* Enumerate all zones to check for proper linkage. */
 { ppage_t iter;
   PAGE_FOREACH(iter,&page_zones[0]) {}
   PAGE_FOREACH(iter,&page_zones[1]) {}
   PAGE_FOREACH(iter,&page_zones[2]) {}
   PAGE_FOREACH(iter,&page_zones[3]) {}
 }
#endif
}
#endif

DECL_END

#ifndef __INTELLISENSE__
#define MMAN_REGISTER
#include "memory-free.c.inl"
#include "memory-free.c.inl"
#include "memory-info.c.inl"
#endif

#endif /* !GUARD_KERNEL_MEMORY_MEMORY_C */
