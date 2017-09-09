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

PUBLIC SAFE KPD ppage_t KCALL
page_malloc_at(ppage_t start, size_t n_bytes, pgattr_t attr) {
 ppage_t iter,result = PAGE_ERROR;
 bool must_clear = false,has_write_lock = false;
 mzone_t zone_id = mzone_of(start);
 struct mzone *zone = PAGEZONE(zone_id);
 assert(PDIR_ISKPD());
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(addr_isphys(start));
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 atomic_rwlock_read(&zone->z_lock);
again:
 PAGE_FOREACH(iter,zone) {
  if ((uintptr_t)iter > (uintptr_t)start) break;
  if ((uintptr_t)iter+iter->p_free.p_size <= (uintptr_t)start) continue;
  /* We can split this range. */
  if (!has_write_lock) {
   has_write_lock = true;
   if (!atomic_rwlock_upgrade(&zone->z_lock)) goto again;
  }
  assert((uintptr_t)iter+iter->p_free.p_size >=
         (uintptr_t)start+n_bytes);
  zone->z_inuse += n_bytes;
  zone->z_avail -= n_bytes;
  must_clear = (attr&PAGEATTR_ZERO) && !(iter->p_free.p_attr&PAGEATTR_ZERO);
  result = start;
  if (iter == start) {
   assert(iter->p_free.p_size >= n_bytes);
   if (iter->p_free.p_size == n_bytes) {
    /* Allocate whole region. */
    *iter->p_free.p_self = iter->p_free.p_next;
    if (iter->p_free.p_next != PAGE_ERROR)
        iter->p_free.p_next->p_free.p_self = iter->p_free.p_self;
   } else {
    /* Split at the front. */
    ppage_t next_page         = (ppage_t)((uintptr_t)iter+n_bytes);
    *iter->p_free.p_self      = next_page;
    next_page->p_free         = iter->p_free;
    next_page->p_free.p_size -= n_bytes;
    assert(next_page->p_free.p_size);
   }
   if (!must_clear && (attr&PAGEATTR_ZERO)) {
    /* Make sure to clear up controller data
     * if it wouldn't be cleared later. */
    iter->p_free.p_next = 0;
    iter->p_free.p_self = 0;
    iter->p_free.p_size = 0;
    iter->p_free.p_attr = 0;
   }
  } else if ((uintptr_t)iter+iter->p_free.p_size ==
             (uintptr_t)start+n_bytes) {
   /* Split at the back. */
   iter->p_free.p_size -= n_bytes;
  } else if (n_bytes) {
   /* Split within. */
   ppage_t next_page;
   next_page = (ppage_t)((uintptr_t)start+n_bytes);
   next_page->p_free.p_size = ((uintptr_t)start+n_bytes)-
                              ((uintptr_t)iter+iter->p_free.p_size);
   next_page->p_free.p_attr =  iter->p_free.p_attr;
   next_page->p_free.p_next =  iter->p_free.p_next;
   next_page->p_free.p_self = &iter->p_free.p_next;
   iter->p_free.p_next      =  next_page;
  }
  break;
 }
 /* Track successful allocations. */
 if (result != PAGE_ERROR) ++zone->z_alloc;
 if (has_write_lock)
      atomic_rwlock_endwrite(&zone->z_lock);
 else atomic_rwlock_endread(&zone->z_lock);
 if (must_clear) memsetl(result,0,n_bytes/4);
#if LOG_PHYSICAL_ALLOCATIONS
 if (result != PAGE_ERROR) {
  syslogf(LOG_MEM|LOG_DEBUG,
          "[MEM] Allocated memory %p...%p from zone #%d\n",
          result,(uintptr_t)result+(n_bytes-1),
         (int)(zone-page_zones));
 }
#endif /* LOG_PHYSICAL_ALLOCATIONS */
 return result;
}
