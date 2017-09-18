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
#ifdef __INTELLISENSE__
#include "memory.c"
#endif

DECL_BEGIN

#ifdef MMAN_REGISTER
#define MY_FREESTR FREESTR
PRIVATE ATTR_FREETEXT SAFE KPD void KCALL
memory_register(PAGE_ALIGNED ppage_t start,
                PAGE_ALIGNED size_t n_bytes,
                bool without_info)
#define attr PAGEATTR_NONE
#else
#define MY_FREESTR /* Nothing */
PUBLIC SAFE KPD void KCALL
page_free_(ppage_t start, size_t n_bytes, pgattr_t attr)
#endif
{
#ifdef MMAN_REGISTER
 uintptr_t skip_end,after_skip_bytes = 0;
#endif
 ppage_t *piter,iter,free_end;
 struct mzone *zone;
 mzone_t zone_id;
 assert(PDIR_ISKPD());
 /* NOTE: Intentionally don't check for writable. */
 CHECK_HOST_TEXT(start,n_bytes);
 if unlikely(!n_bytes) return;
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(addr_isphys(start));
#ifdef MMAN_REGISTER
 assert(IS_ALIGNED(n_bytes,PAGESIZE));
#else
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
#endif
 zone_id = mzone_of(start);

 /* TODO: if (attr&PAGEATTR_ZERO) assert(is_zero_initialized(start,n_bytes)); */
 assertf((uintptr_t)start+(n_bytes-1) >= (uintptr_t)start,
         MY_FREESTR("Pointer overflow while freeing pages: %p + %Id(%Ix) overflows into %p"),
         (uintptr_t)start,n_bytes,n_bytes,(uintptr_t)start+(n_bytes-1));
 free_end = (ppage_t)((uintptr_t)start+n_bytes);
 if ((uintptr_t)free_end-1 > MZONE_MAX(zone_id)) {
  /* Split the free requested when it overlaps into a different zone. */
  size_t zone_offset = (uintptr_t)(MZONE_MAX(zone_id)+1)-(uintptr_t)start;
  assertf(zone_offset,MY_FREESTR("But the we've determined the wrong zone above..."));
  assertf(zone_offset < n_bytes,
          MY_FREESTR("Zone %d %p...%p\n"
                     "zone_offset = %Iu\n"
                     "n_bytes     = %Iu\n"),
          zone_id,start,(uintptr_t)start+n_bytes-1,
          zone_offset,n_bytes);
  assert(IS_ALIGNED(zone_offset,PAGESIZE));
  /* Recursively free memory above the zone limits. */
#ifdef MMAN_REGISTER
  if (addr_isphys((uintptr_t)start+zone_offset)) {
   memory_register((ppage_t)((uintptr_t)start+zone_offset),
                    n_bytes-zone_offset,without_info);
  } else {
   syslog(LOG_MEM|LOG_WARN,MY_FREESTR("[MEM] Cannot use RAM %p...%p above 3Gb\n"),
         (uintptr_t)start+zone_offset,
         (uintptr_t)start+n_bytes-1);
  }
#else
  page_free_((ppage_t)((uintptr_t)start+zone_offset),n_bytes-zone_offset,attr);
#endif
  /* Update the free pointers. */
  n_bytes  = zone_offset;
  free_end = (ppage_t)((uintptr_t)start+zone_offset);
 }
 assert((uintptr_t)free_end-1 <= MZONE_MAX(zone_id));

 assert(n_bytes);
#ifdef MMAN_REGISTER
 if (zone_id == MZONE_DEV) {
  /* Special case: Ignore free requested for the DEV memory zone. */
  syslog(LOG_MEM|LOG_INFO,
         MY_FREESTR("[MEM] Ignoring free memory range %p..%p apart of the device memory zone\n"),
         start,(uintptr_t)start+(n_bytes-1));
  return;
 }
#else
 assert(zone_id != MZONE_DEV);
#endif
#if LOG_PHYSICAL_ALLOCATIONS
#ifndef MMAN_REGISTER
 syslog(LOG_MEM|LOG_DEBUG,
        MY_FREESTR("[MEM] Feeing memory %p...%p from zone #%d\n"),
        start,(uintptr_t)start+(n_bytes-1),zone_id);
#else
 syslog(LOG_MEM|LOG_DEBUG,
        MY_FREESTR("[MEM] Registering memory %p...%p in zone #%d\n"),
        start,(uintptr_t)start+(n_bytes-1),zone_id);
#endif /* !MMAN_REGISTER */
#endif /* LOG_PHYSICAL_ALLOCATIONS */
 VERIFY_MEMORY_F(start,n_bytes,attr);

 /* Load the zone this pointer is apart of. */
 zone = PAGEZONE(zone_id);
#ifndef MMAN_REGISTER
 atomic_rwlock_write(&zone->z_lock);
#endif /* !MMAN_REGISTER */
 piter = &zone->z_root;
 while ((iter = *piter) != PAGE_ERROR &&
         PAGE_END(iter) <= start) {
#ifdef MMAN_REGISTER
  assert(iter <= start);
#else
  assert(iter < start);
#endif
  piter = &iter->p_free.p_next;
 }
#ifdef MMAN_REGISTER
 if (iter != PAGE_ERROR
#ifdef CONFIG_DEBUG
     /* When no information is being registered,
      * the free range must not exist already! */
     && !without_info
#endif
     ) {
  if ((uintptr_t)iter < (uintptr_t)start+n_bytes) {
   /* Skip overlapping range: iter...MIN(start+n_bytes,PAGE_END(iter)) */
   skip_end         = MIN((uintptr_t)start+n_bytes,
                          (uintptr_t)PAGE_END(iter));
   after_skip_bytes = (uintptr_t)start+n_bytes;
   if (skip_end > after_skip_bytes)
        after_skip_bytes -= skip_end;
   else after_skip_bytes  = 0;
   assert(skip_end > (uintptr_t)start);
   assert(skip_end > (uintptr_t)iter);
   syslog(LOG_MEM|LOG_WARN,
          MY_FREESTR("[MEM] Skipping overlapping free-memory range at %p...%p already within %p...%p\n"),
          start,skip_end-1,iter,(uintptr_t)PAGE_END(iter)-1);
   /* Free memory below the skipped range. */
   if ((uintptr_t)iter <= (uintptr_t)start) goto free_after_skip;
   n_bytes = (uintptr_t)iter-(uintptr_t)start;
   assert(n_bytes);
   assert(IS_ALIGNED(n_bytes,PAGESIZE));
  }
 }
#endif
 assertf(iter == PAGE_ERROR || (uintptr_t)iter >= (uintptr_t)start+n_bytes,
         MY_FREESTR("At least part of address range %p...%p was already marked as free by %p...%p"),
         start,(uintptr_t)free_end-1,iter,(uintptr_t)PAGE_END(iter)-1);

 /* Insert after 'piter' / before 'iter' */
 
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
#ifndef MMAN_REGISTER
 zone->z_inuse -= n_bytes;
#endif
 zone->z_avail += n_bytes;
 /* Track free-calls. */
 ++zone->z_free;
#ifndef MMAN_REGISTER
 atomic_rwlock_endwrite(&zone->z_lock);
#endif /* !MMAN_REGISTER */
#ifdef MMAN_REGISTER
 if (!without_info)
      memory_register_info(start,n_bytes);
free_after_skip:
 if (after_skip_bytes)
     memory_register((ppage_t)skip_end,after_skip_bytes,without_info);

#endif
}

#ifdef attr
#undef attr
#endif
#undef MY_FREESTR
#ifdef MMAN_REGISTER
#undef MMAN_REGISTER
#endif

DECL_END
