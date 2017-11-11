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
#include <malloc.h>
#include <proprietary/multiboot.h>
#include <proprietary/multiboot2.h>
#include <sched/paging.h>
#include <sched/task.h>
#include <string.h>
#include <sys/syslog.h>

DECL_BEGIN

#ifdef __x86_64__
#define LOG_PHYSICAL_ALLOCATIONS 1
#endif

/* Define to non-zero to add syslog entries for physical memory allocation. */
#ifndef LOG_PHYSICAL_ALLOCATIONS
#define LOG_PHYSICAL_ALLOCATIONS 0
#endif


#if __SIZEOF_POINTER__ == 8
#   define MEMSETX   memsetq
#   define MEMCPYX   memcpyq
#elif __SIZEOF_POINTER__ == 4
#   define MEMSETX   memsetl
#   define MEMCPYX   memcpyl
#elif __SIZEOF_POINTER__ == 2
#   define MEMSETX   memsetw
#   define MEMCPYX   memcpyw
#elif __SIZEOF_POINTER__ == 1
#   define MEMSETX   memsetb
#   define MEMCPYX   memcpyb
#else
#   error "Unsupported sizeof(void *)"
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
  MEMSETX(start,KERNEL_DEBUG_MEMPAT_PAGE_MALLOC,
          n_bytes/__SIZEOF_POINTER__);
 }
#endif
}


#else
#define VERIFY_MEMORY_M(start,n_bytes,attr) (void)0
#define VERIFY_MEMORY_F(start,n_bytes,attr) (void)0
#endif



INTERN ATTR_HOTDATA
struct mzone page_zones[MZONE_REAL_COUNT] = {
    /* Pre-initialize all zones as empty.
     * NOTE: All zones use physical pointers */
    [0 ... MZONE_REAL_COUNT-1] = {
        .z_lock  = ATOMIC_RWLOCK_INIT,
        .z_root  = PAGE_ERROR,
        .z_inuse = 0,
        .z_avail = 0,
    },
};

PUBLIC struct mzonespec const mzone_spec = {
#ifdef __x86_64__
    .ms_min = {
        [MZONE_1MB]                  = __UINTPTR_C(0x0000000000000000),
        [MZONE_STATIC]               = __UINTPTR_C(0x0000000000100000),
        [MZONE_32BIT]                = __UINTPTR_C(0x0000000080000000),
        [MZONE_HIMEM]                = __UINTPTR_C(0x0000000100000000),
        [MZONE_VIRTUAL|MZONE_1MB]    = __UINTPTR_C(0xffffffff80000000),
        [MZONE_VIRTUAL|MZONE_STATIC] = __UINTPTR_C(0xffffffff80100000),
        [MZONE_VIRTUAL|MZONE_32BIT]  = __UINTPTR_C(0xffffffffffffffff), /* Doesn't exist. */
        [MZONE_VIRTUAL|MZONE_HIMEM]  = __UINTPTR_C(0xffffffffffffffff), /* Doesn't exist. */
    },
    .ms_max = {
        [MZONE_1MB]                  = __UINTPTR_C(0x00000000000fffff),
        [MZONE_STATIC]               = __UINTPTR_C(0x000000007fffffff),
        [MZONE_32BIT]                = __UINTPTR_C(0x00000000ffffffff),
        [MZONE_HIMEM]                = __UINTPTR_C(0x00007fffffffffff),
        [MZONE_VIRTUAL|MZONE_1MB]    = __UINTPTR_C(0xffffffff8009ffff),
        [MZONE_VIRTUAL|MZONE_STATIC] = __UINTPTR_C(0xffffffffffffffff),
        [MZONE_VIRTUAL|MZONE_32BIT]  = __UINTPTR_C(0x0000000000000000), /* Doesn't exist. */
        [MZONE_VIRTUAL|MZONE_HIMEM]  = __UINTPTR_C(0x0000000000000000), /* Doesn't exist. */
    },
#else
    .ms_min = {
        [MZONE_1MB]   = __UINTPTR_C(0x00000000),
        [MZONE_HIMEM] = __UINTPTR_C(0x00100000),
    },
    .ms_max = {
        [MZONE_1MB]   = __UINTPTR_C(0x000fffff),
        [MZONE_HIMEM] = __UINTPTR_C(0xbfffffff),
    },
#endif
};


LOCAL void KCALL
mov_pages_lo(void *dst, void const *src, size_t n_bytes) {
 /* NOTE: Intentionally don't check for writable. */
 assert(dst < src);
 CHECK_HOST_TEXT(dst,n_bytes);
 CHECK_HOST_TEXT(src,n_bytes);
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 while (n_bytes) {
  MEMCPYX(dst,src,PAGESIZE/__SIZEOF_POINTER__);
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
  MEMCPYX(dst,src,PAGESIZE/__SIZEOF_POINTER__);
  n_bytes            -= PAGESIZE;
 }
}

PUBLIC void KCALL
page_stat(struct mstat *__restrict info) {
 struct mzone *zone;
 struct mzstat *zinfo;
 struct mman *omm;
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
 TASK_PDIR_KERNEL_BEGIN(omm);
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
 TASK_PDIR_KERNEL_END(omm);
 if unlikely(info->m_total.mz_freemin == (size_t)-1) info->m_total.mz_freemin = 0;
 if unlikely(info->m_total.mz_zeromin == (size_t)-1) info->m_total.mz_zeromin = 0;
}

PRIVATE size_t KCALL
page_do_available(mzone_t zoneid, ppage_t start, size_t n_bytes) {
 struct mzone *zone; ppage_t iter;
 size_t result = 0; ppage_t check_end;
 zone = &page_zones[zoneid];
 check_end = (ppage_t)((uintptr_t)start+n_bytes);
 task_crit();
 atomic_rwlock_read(&zone->z_lock);
 PAGE_FOREACH(iter,zone) {
  ppage_t page_end = PAGE_END(iter);
  if (page_end <= start) continue;
  if (iter >= check_end) break;
  result += MIN((uintptr_t)page_end,(uintptr_t)check_end)-
            MAX((uintptr_t)iter,(uintptr_t)start);
 }
 atomic_rwlock_endread(&zone->z_lock);
 task_endcrit();
 return result;
}

PUBLIC size_t KCALL
page_available(ppage_t start, PAGE_ALIGNED size_t n_bytes) {
 mzone_t zone; size_t result = 0;
 struct mman *omm = NULL;
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED(n_bytes,PAGESIZE));
 for (zone = 0; zone < MZONE_REAL_COUNT; ++zone) {
  if ((uintptr_t)start >= MZONE_MIN(zone) &&
      (uintptr_t)start <= MZONE_MAX(zone)) {
   size_t zone_bytes = (MZONE_MAX(zone)+1)-(uintptr_t)start;
   if (zone_bytes > n_bytes)
       zone_bytes = n_bytes;
   if (!omm) TASK_PDIR_KERNEL_BEGIN(omm);
   result += page_do_available(zone,start,zone_bytes);
   n_bytes -= zone_bytes;
   if (!n_bytes) break;
   *(uintptr_t *)&start += zone_bytes;
  }
 }
 if (omm) TASK_PDIR_KERNEL_END(omm);
 return result;
}



/* Query page attributes, returning a set of `PAGEATTR_*' */
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
#endif /* LOG_PHYSICAL_ALLOCATIONS */

   if (must_clear) MEMSETX(result,0,n_bytes/__SIZEOF_POINTER__);
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
page_memalign(size_t alignment, size_t n_bytes,
              pgattr_t attr, mzone_t zone_id) {
 ppage_t result,aligned_result,result_end;
 size_t total_size;
 /* Use regular malloc() for sub-page alignments. */
 if (alignment <= PAGESIZE)
     return page_malloc(n_bytes,attr,zone_id);
 /* Make sure to align by multiples of pages.
  * HINT: Also handles the case of illegal non-power-of-2
  *       alignments as weak undefined behavior. */
 alignment = CEIL_ALIGN(alignment,PAGESIZE);
 n_bytes   = CEIL_ALIGN(n_bytes,PAGESIZE);
 /* Make sure that the alignment doesn't overflow when added to `n_bytes' */
 if (__builtin_add_overflow(alignment,n_bytes,&total_size))
     return PAGE_ERROR;

 /* Overallocate + free unused portion. */
 result = page_malloc(total_size,attr,zone_id);
 if likely(result != PAGE_ERROR) {
  aligned_result = (ppage_t)CEIL_ALIGN((uintptr_t)result,alignment);
  result_end     = (ppage_t)((uintptr_t)aligned_result+n_bytes);
  /* Free alignment memory below and above. */
  page_ffree(result,(uintptr_t)aligned_result-(uintptr_t)result,attr);
  page_ffree(result_end,((uintptr_t)result+total_size)-(uintptr_t)result_end,attr);
  /* Return the aligned pointer. */
  result = aligned_result;
 }
 return result;
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
    /* It can't get better than `n_bytes' itself! */
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
    /* This can happen when `best_page' includes `max' */
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
 if (must_clear) MEMSETX(result,0,n_bytes/__SIZEOF_POINTER__);
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
                 pgattr_t attr, mzone_t zone_id) {
 CHECK_HOST_DOBJ(res_size);
#if 0
 /* Stub implementation. */
 if (max_size <= min_size) {
  *res_size = min_size;
  return page_malloc(min_size,attr,zone_id);
 }
#else
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
   assertf(iter->p_free.p_next == PAGE_ERROR ||
           iter->p_free.p_next >  PAGE_END(iter),
           "Miss-ordered memory zone: %p...%p overlaps with %p...%p",
           iter,(uintptr_t)PAGE_END(iter)-1,iter->p_free.p_next,
          (uintptr_t)PAGE_END(iter->p_free.p_next)-1);
   if (iter->p_free.p_size < min_size) continue;
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
   size_t alloc_size;
   assert(best_size >= min_size);
   min_size = CEIL_ALIGN(min_size,PAGESIZE);
   assert(best_size >= min_size);
   if (!has_write_lock) {
    has_write_lock = true;
    if (!atomic_rwlock_upgrade(&zone->z_lock))
         goto search_zone;
   }
   assert(best_size == best_page->p_free.p_size);
   alloc_size = MIN(best_size,FLOOR_ALIGN(max_size,PAGESIZE));
   *res_size = alloc_size;
   result = (ppage_t)((uintptr_t)best_page+(best_size-alloc_size));
#if 0
   syslog(LOG_MEM|LOG_DEBUG,"[MEM] Allocating MAX(%Iu) >= GOT(%Iu) >= MIN(%Iu) bytes at %p...%p\n",
          FLOOR_ALIGN(max_size,PAGESIZE),alloc_size,min_size,result,(uintptr_t)result+alloc_size-1);
#endif
   assert(IS_ALIGNED((uintptr_t)best_page,PAGESIZE));
   assert(IS_ALIGNED((uintptr_t)result,PAGESIZE));
   if (result == best_page) {
    /* Allocate this whole part. */
    *best_page->p_free.p_self = best_page->p_free.p_next;
    if (best_page->p_free.p_next != PAGE_ERROR)
        best_page->p_free.p_next->p_free.p_self = best_page->p_free.p_self;
   } else {
    /* Trim the part near its upper end. */
    best_page->p_free.p_size -= alloc_size;
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
   zone->z_inuse += alloc_size;
   zone->z_free  -= alloc_size;
   ++zone->z_alloc;
   atomic_rwlock_endwrite(&zone->z_lock);

#if LOG_PHYSICAL_ALLOCATIONS
   syslog(LOG_MEM|LOG_DEBUG,
          "[MEM] Allocated memory %p...%p from zone #%d (#%d)\n",
          result,(uintptr_t)result+(alloc_size-1),
         (int)(zone-page_zones),zone_id);
#endif /* LOG_PHYSICAL_ALLOCATIONS */

   if (must_clear) MEMSETX(result,0,alloc_size/__SIZEOF_POINTER__);
   VERIFY_MEMORY_M(result,alloc_size,attr);
   return result;
  }
  if (has_write_lock)
       atomic_rwlock_endwrite(&zone->z_lock);
  else atomic_rwlock_endread (&zone->z_lock);
 } while (zone-- != page_zones);
 return PAGE_ERROR;
#endif
}

PUBLIC SAFE KPD void KCALL
page_ffree(ppage_t start, size_t n_bytes, pgattr_t attr) {
 ppage_t *piter,iter,free_end;
 struct mzone *zone;
 mzone_t zone_id;
 assert(PDIR_ISKPD());
 /* NOTE: Intentionally don't check for writable. */
 CHECK_HOST_TEXT(start,n_bytes);
 if unlikely(!n_bytes) return;
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(addr_isphys(start));
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 zone_id = mzone_of(start);

 /* TODO: if (attr&PAGEATTR_ZERO) assert(is_zero_initialized(start,n_bytes)); */
 assertf((uintptr_t)start+(n_bytes-1) >= (uintptr_t)start,
         "Pointer overflow while freeing pages: %p + %Id(%Ix) overflows into %p",
         (uintptr_t)start,n_bytes,n_bytes,(uintptr_t)start+(n_bytes-1));
 free_end = (ppage_t)((uintptr_t)start+n_bytes);
 if ((uintptr_t)free_end-1 > MZONE_MAX(zone_id)) {
  /* Split the free requested when it overlaps into a different zone. */
  size_t zone_offset = (uintptr_t)(MZONE_MAX(zone_id)+1)-(uintptr_t)start;
  assertf(zone_offset,"But the we've determined the wrong zone above...");
  assertf(zone_offset < n_bytes,
          "Zone %d %p...%p\n"
          "zone_offset = %Iu\n"
          "n_bytes     = %Iu\n",
          zone_id,start,(uintptr_t)start+n_bytes-1,
          zone_offset,n_bytes);
  assert(IS_ALIGNED(zone_offset,PAGESIZE));
  /* Recursively free memory above the zone limits. */
  page_ffree((ppage_t)((uintptr_t)start+zone_offset),n_bytes-zone_offset,attr);
  /* Update the free pointers. */
  n_bytes  = zone_offset;
  free_end = (ppage_t)((uintptr_t)start+zone_offset);
 }
 assert((uintptr_t)free_end-1 <= MZONE_MAX(zone_id));

 assert(n_bytes);
#if LOG_PHYSICAL_ALLOCATIONS
 syslog(LOG_MEM|LOG_DEBUG,
        "[MEM] Feeing memory %p...%p from zone #%d\n",
        start,(uintptr_t)start+(n_bytes-1),zone_id);
#endif /* LOG_PHYSICAL_ALLOCATIONS */
 VERIFY_MEMORY_F(start,n_bytes,attr);

 /* Load the zone this pointer is apart of. */
 zone = PAGEZONE(zone_id);
 atomic_rwlock_write(&zone->z_lock);
 piter = &zone->z_root;
 while ((iter = *piter) != PAGE_ERROR &&
         PAGE_END(iter) <= start) {
  assert(iter <= start);
  piter = &iter->p_free.p_next;
 }
 assertf(iter == PAGE_ERROR || (uintptr_t)iter >= (uintptr_t)start+n_bytes,
         "At least part of address range %p...%p was already marked as free by %p...%p",
         start,(uintptr_t)free_end-1,iter,(uintptr_t)PAGE_END(iter)-1);

 /* Insert after `piter' / before `iter' */
 
 /* Check for extending the previous range. */
 if (piter != &zone->z_root) {
  ppage_t prev_page = container_of(piter,union page,p_free.p_next);
  if (PAGE_END(prev_page) == start) {
   /* Extend the previous range. */
   prev_page->p_free.p_size += n_bytes;
   prev_page->p_free.p_attr &= attr;
   assert(prev_page->p_free.p_next == iter);
   assert(PAGE_END(prev_page) <= iter);
   if unlikely(PAGE_END(prev_page) == iter) {
    /* Extending the previous range causes it to touch the next. - Merge the two. */
    prev_page->p_free.p_attr &= iter->p_free.p_attr;
    prev_page->p_free.p_size += iter->p_free.p_size;
    prev_page->p_free.p_next  = iter->p_free.p_next;
    if (prev_page->p_free.p_next != PAGE_ERROR)
        prev_page->p_free.p_next->p_free.p_self = &prev_page->p_free.p_next;
    /* Clear the controller of the next range if the merged range is zero-initialized. */
    if (prev_page->p_free.p_attr&PAGEATTR_ZERO) {
     iter->p_free.p_next = 0;
     iter->p_free.p_self = 0;
     iter->p_free.p_size = 0;
     iter->p_free.p_attr = 0;
    }
   }
   goto done;
  }
 }

 /* Check for merging with the next range. */
 if (iter != PAGE_ERROR &&
    (uintptr_t)iter == (uintptr_t)start+n_bytes) {
  start->p_free         = iter->p_free;
  start->p_free.p_size += n_bytes;
  start->p_free.p_attr &= attr;
  if (start->p_free.p_attr&PAGEATTR_ZERO) {
   iter->p_free.p_next = 0;
   iter->p_free.p_self = 0;
   iter->p_free.p_size = 0;
   iter->p_free.p_attr = 0;
  }
  if (start->p_free.p_next != PAGE_ERROR)
      start->p_free.p_next->p_free.p_self = &start->p_free.p_next;
  *start->p_free.p_self = start;
  goto done;
 }

 /* Fallback: Create & insert a new free-range. */
 start->p_free.p_size = n_bytes;
 start->p_free.p_self = piter;
 start->p_free.p_next = iter;
 start->p_free.p_attr = attr;
 *piter               = start;
 if (iter != PAGE_ERROR)
     iter->p_free.p_self = &start->p_free.p_next;

done:
#ifdef CONFIG_DEBUG
 PAGE_FOREACH(iter,zone) {}
#endif
 zone->z_inuse -= n_bytes;
 zone->z_avail += n_bytes;
 /* Track free-calls. */
 ++zone->z_free;
 atomic_rwlock_endwrite(&zone->z_lock);
}



PUBLIC SAFE KPD bool KCALL
mscatter_split_lo(struct mscatter *__restrict dst,
                  struct mscatter *__restrict src,
                  PAGE_ALIGNED uintptr_t offset_from_src) {
 struct mscatter *split_part;
 assert(PDIR_ISKPD());
 CHECK_HOST_DOBJ(dst);
 CHECK_HOST_DOBJ(src);
 assert(IS_ALIGNED(offset_from_src,PAGESIZE));
 split_part = src;
 while ((assertf(split_part,"Split offset is out-of-bounds by %Iu(%Ix) bytes",
                 offset_from_src,offset_from_src),
         offset_from_src > split_part->m_size)) {
  offset_from_src -= split_part->m_size;
  split_part = split_part->m_next;
 }
 if (offset_from_src == split_part->m_size) {
  /* Simple case: The split occurs at the end of 'split_part'. */
  if (split_part->m_next) {
   memcpy(dst,split_part->m_next,sizeof(struct mscatter));
   free(split_part->m_next);
   split_part->m_next = NULL;
  } else {
   memset(dst,0,sizeof(struct mscatter));
  }
 } else {
  /* Split in the middle of the tab. */
  memcpy(dst,split_part,sizeof(struct mscatter));
  split_part->m_size           = offset_from_src;
  split_part->m_next           = NULL;
  dst->m_size                 -= offset_from_src;
  *(uintptr_t *)&dst->m_start += offset_from_src;
 }
 return true;
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
  MEMCPYX((void *)dst_iter,(void *)src_iter,
           cpy_size/__SIZEOF_POINTER__);
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


#undef page_malloc_scatter
PUBLIC SAFE KPD bool KCALL
page_malloc_scatter(struct mscatter *__restrict scatter,
                    size_t n_bytes, size_t min_scatter,
                    pgattr_t attr, mzone_t zone,
                    gfp_t scatter_extension) {
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
                                        scatter_extension|GFP_NOFREQ);
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
    page_ffree(s_iter->m_start,s_iter->m_size,attr);
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
   if (attr&PAGEATTR_ZERO)
       MEMSETX((void *)((uintptr_t)result+old_bytes),0,
                diff/__SIZEOF_POINTER__);
   return result;
  }
  /* Allocate a whole new region. */
  if (attr&PAGEATTR_ZERO && diff < old_bytes) {
   result = page_malloc(new_bytes,attr&~(PAGEATTR_ZERO),zone);
   if unlikely(result == PAGE_ERROR) return PAGE_ERROR;
   MEMSETX((void *)((uintptr_t)result+old_bytes),
            0,diff/__SIZEOF_POINTER__);
  } else {
   result = page_malloc(new_bytes,attr,zone);
   if unlikely(result == PAGE_ERROR) return PAGE_ERROR;
  }
  assert(IS_ALIGNED(old_bytes,PAGESIZE));
  MEMCPYX(result,start,old_bytes/__SIZEOF_POINTER__);
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



INTERN ATTR_FREETEXT SAFE KPD size_t KCALL
memory_load_mb_lower_upper(u32 mem_lower, u32 mem_upper) {
 syslog(LOG_MEM|LOG_INFO,
        "TODO: memory_load_mb_lower_upper: %I32u / %I32u\n",
        mem_lower,mem_upper);
 return 0;
}

DECL_END

#endif /* !GUARD_KERNEL_MEMORY_MEMORY_C */
