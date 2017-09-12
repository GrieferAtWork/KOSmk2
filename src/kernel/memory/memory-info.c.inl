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
#ifndef GUARD_KERNEL_MEMORY_MEMORY_INFO_C_INL
#define GUARD_KERNEL_MEMORY_MEMORY_INFO_C_INL 1
#define _KOS_SOURCE 2

#include <assert.h>
#include <hybrid/align.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <sys/syslog.h>
#include <stddef.h>
#include <string.h>

DECL_BEGIN

PUBLIC PHYS struct meminfo const *
_mem_info[MZONE_COUNT] __ASMNAME("mem_info") = {
    [0 ... MZONE_COUNT-1] = NULL,
};

#define PAGECORE_DATA_MAXSLOTS  \
 (PAGESIZE-(sizeof(void *)+sizeof(size_t)))/\
  sizeof(struct meminfo)

struct meminfo_data {
 struct meminfo_data *cp_next; /*< [0..1|null(PAGE_ERROR)] Next core page. */
 size_t               cp_used;
 struct meminfo       cp_data[PAGECORE_DATA_MAXSLOTS];
};

PRIVATE ATTR_FREEDATA struct meminfo_data *core_alloc = PAGE_ERROR;
PRIVATE ATTR_FREETEXT SAFE KPD struct meminfo *KCALL meminfo_alloc(void) {
 if (core_alloc == PAGE_ERROR ||
     core_alloc->cp_used == PAGECORE_DATA_MAXSLOTS) {
  struct meminfo_data *new_slot;
  new_slot = (struct meminfo_data *)page_malloc(sizeof(struct meminfo_data),
                                                PAGEATTR_NONE,MZONE_ANY);
  if unlikely(new_slot == PAGE_ERROR) return NULL;
  new_slot->cp_used = 0;
  new_slot->cp_next = core_alloc;
  core_alloc = new_slot;
 }
 return &core_alloc->cp_data[core_alloc->cp_used++];
}

/* Add the given memory range as a RAM-range within meminfo. */
PRIVATE ATTR_FREETEXT SAFE KPD void KCALL
memory_register_info(ppage_t start, size_t n_bytes) {
 struct meminfo *entry; ppage_t free_end;
 struct meminfo *iter,**piter;
 mzone_t zone_id;
 if unlikely(!n_bytes) return;
 zone_id = mzone_of(start);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED(n_bytes,PAGESIZE));
 assert(addr_isphys(start));
 assertf((uintptr_t)start+(n_bytes-1) >= (uintptr_t)start,
         FREESTR("Pointer overflow while freeing pages: %p + %Id(%Ix) overflows into %p"),
         (uintptr_t)start,n_bytes,n_bytes,(uintptr_t)start+(n_bytes-1));
 free_end = (ppage_t)((uintptr_t)start+n_bytes);
 if ((uintptr_t)free_end-1 > MZONE_MAX(zone_id)) {
  /* Split the free requested when it overlaps into a different zone. */
  size_t zone_offset = (uintptr_t)MZONE_MIN(zone_id+1)-(uintptr_t)start;
  assertf(zone_offset,FREESTR("But the we've determined the wrong zone above..."));
  assertf(zone_offset < n_bytes,FREESTR("Zone %d %p...%p\n"),zone_id,start,(uintptr_t)start+n_bytes-1);
  assert(IS_ALIGNED(zone_offset,PAGESIZE));
  /* Recursively free memory above the zone limits. */
  memory_register_info((ppage_t)((uintptr_t)start+zone_offset),n_bytes-zone_offset);
  /* Update the free pointers. */
  n_bytes  = zone_offset;
  free_end = (ppage_t)((uintptr_t)start+zone_offset);
 }
 assert((uintptr_t)free_end-1 <= MZONE_MAX(zone_id));
 assert(n_bytes);
 assert((uintptr_t)start+n_bytes == (uintptr_t)free_end);

 /* Special case: Ignore free requested for the DEV memory zone. */
 if (zone_id == MZONE_DEV) return;

#if 0
 syslog(LOG_DEBUG,"Adding meminfo: %p...%p (Zone %d)\n",
         start,(uintptr_t)free_end-1,zone_id);
#endif

 piter = (struct meminfo **)&_mem_info[zone_id];
 assert(*piter != PAGE_ERROR);
 while ((iter = *piter) != NULL &&
        (uintptr_t)iter->mi_start+iter->mi_size <=
        (uintptr_t)start) {
  assert(iter != PAGE_ERROR);
  assert(iter->mi_next != PAGE_ERROR);
  assert(iter->mi_start < start);
  piter = (struct meminfo **)&iter->mi_next;
 }
 assertf(iter == NULL || (uintptr_t)iter->mi_start >= (uintptr_t)start+n_bytes,
         FREESTR("At least part of address range %p...%p was already marked as free by %p...%p"),
         start,(uintptr_t)start+n_bytes-1,iter->mi_start,(uintptr_t)iter->mi_start+iter->mi_size-1);

 /* Insert after 'piter' / before 'iter' */
 
 /* Check for extending the previous range. */
 if (piter != (struct meminfo **)&_mem_info[zone_id]) {
  struct meminfo *prev_info = container_of(piter,struct meminfo,mi_next);
  if ((uintptr_t)prev_info->mi_start+prev_info->mi_size ==
      (uintptr_t)start) {
   /* Extend the previous range. */
   prev_info->mi_size += n_bytes;
   assert(prev_info->mi_next == iter);
   assert((uintptr_t)prev_info->mi_start+prev_info->mi_size <=
          (uintptr_t)iter->mi_start);
   if unlikely((uintptr_t)prev_info->mi_start+prev_info->mi_size ==
               (uintptr_t)iter->mi_start) {
    /* Extending the previous range causes it to touch the next. - Merge the two. */
    assert(iter->mi_next != PAGE_ERROR);
    prev_info->mi_size += iter->mi_size;
    prev_info->mi_next  = iter->mi_next;
    /*meminfo_free(iter);*/
   }
   goto done;
  }
 }

 /* Check for merging with the next range. */
 if (iter != NULL &&
    (uintptr_t)iter->mi_start == (uintptr_t)start+n_bytes) {
  *(uintptr_t *)&iter->mi_start -= n_bytes;
  iter->mi_size                 += n_bytes;
  assert(iter->mi_start == start);
  assert(*piter == iter);
  assert(piter == (struct meminfo **)&_mem_info[zone_id] ||
        ((uintptr_t)container_of(piter,struct meminfo,mi_next)->mi_start+
                    container_of(piter,struct meminfo,mi_next)->mi_size) <
         (uintptr_t)iter->mi_start);
  goto done;
 }

 /* Fallback: Create & insert a new free-range. */
 if unlikely((entry = meminfo_alloc()) == NULL) {
  syslog(LOG_MEM|LOG_DEBUG,
          FREESTR("[MEM] Failed to allocate core-memory information page: %[errno]\n"),
          ENOMEM);
 } else {
  assert(iter  != PAGE_ERROR);
  assert(entry != PAGE_ERROR);
  entry->mi_next  = iter;
  entry->mi_start = start;
  entry->mi_size  = n_bytes;
  *piter = entry;
 }
done:;
}


#define IS_1MB_ZONE(p) \
  ((uintptr_t)(p) < (1024*1024))

/* If possible, relocate any core information allocated within the 'MZONE_V_1MB' zone. */
PRIVATE ATTR_FREETEXT SAFE KPD void KCALL
meminfo_relocate_data(struct meminfo_data *__restrict dst,
                      struct meminfo_data *__restrict src,
                      uintptr_t offset) {
 struct meminfo *iter,**piter; mzone_t id;
 assert(IS_1MB_ZONE(src));
 assert(!IS_1MB_ZONE(dst));
 assert(IS_ALIGNED((uintptr_t)dst,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)src,PAGESIZE));
 assert(offset == (uintptr_t)dst-(uintptr_t)src);
 for (id = 0; id < MZONE_COUNT; ++id) {
  piter = (struct meminfo **)&_mem_info[id];
  while ((assert(iter != PAGE_ERROR),
         (iter = *piter) != NULL)) {
   if (iter >= src->cp_data &&
       iter <  COMPILER_ENDOF(src->cp_data)) {
    *(uintptr_t *)piter += offset;
    *(uintptr_t *)&iter += offset;
   }
   assert(iter->mi_next != PAGE_ERROR);
   piter = (struct meminfo **)&iter->mi_next;
  }
 }
}

INTERN ATTR_FREETEXT SAFE KPD void KCALL memory_relocate_info(void) {
 struct meminfo_data *iter,**piter = &core_alloc;
 while ((iter = *piter) != PAGE_ERROR) {
  if (IS_1MB_ZONE(iter)) {
   struct meminfo_data *copy;
   copy = (struct meminfo_data *)page_malloc(sizeof(struct meminfo_data),
                                             PAGEATTR_NONE,MZONE_ANY);
   if likely(copy != PAGE_ERROR) {
    if unlikely(IS_1MB_ZONE(copy)) {
     page_free((ppage_t)copy,sizeof(struct meminfo_data));
    } else {
     memcpy(copy,iter,sizeof(struct meminfo_data));
     syslog(LOG_MEM|LOG_DEBUG,FREESTR("[MEM] Relocating meminfo table %p...%p to %p...%p\n"),
            (uintptr_t)iter,(uintptr_t)iter+sizeof(struct meminfo_data)-1,
            (uintptr_t)copy,(uintptr_t)copy+sizeof(struct meminfo_data)-1);
     /* Relocate data. */
     meminfo_relocate_data(copy,iter,(uintptr_t)copy-(uintptr_t)iter);
     /* Free the pages previously used for meminfo. */
     page_free((ppage_t)iter,sizeof(struct meminfo_data));
     *piter = copy;
     iter = copy;
    }
   }
  }
  piter = &iter->cp_next;
 }
}

DECL_END

#endif /* !GUARD_KERNEL_MEMORY_MEMORY_INFO_C_INL */
