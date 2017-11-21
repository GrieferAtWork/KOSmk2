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
#ifndef GUARD_KERNEL_MEMORY_INSTALL_C
#define GUARD_KERNEL_MEMORY_INSTALL_C 1
#define _KOS_SOURCE 2 /* 'assertf' */

#include "../debug-config.h"

/* TODO: New memory information system that is similar to an mregion,
 *       in that all physical memory always has a descriptor assigned.
 *    >> That way, memory information can easily be changed retroactively,
 *       and information splitting can be implemented as its own function.
 */

#include <hybrid/align.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/panic.h>
#include <hybrid/section.h>
#include <kernel/malloc.h>
#include <kernel/memory.h>
#include <malloc.h>
#include <proprietary/multiboot.h>
#include <proprietary/multiboot2.h>
#include <sched/task.h>
#include <string.h>
#include <sys/syslog.h>
#include <arch/cpustate.h>
#include <kernel/paging.h>
#include <arch/hints.h>
#include <kernel/export.h>
#include <arch/memory.h>

DECL_BEGIN

PRIVATE ATTR_FREETEXT size_t KCALL
page_addmemory(mzone_t zone_id, ppage_t start,
               PAGE_ALIGNED size_t n_bytes) {
#if 1
 ppage_t *piter,iter,free_end;
 struct mzone *zone;
 if unlikely((uintptr_t)start >= KERNEL_BASE) return 0;
 if unlikely((uintptr_t)start+n_bytes > KERNEL_BASE)
    n_bytes = KERNEL_BASE-(uintptr_t)start;
 if unlikely(!n_bytes) return 0;
 syslog(LOG_DEBUG,FREESTR("[MEM] Using dynamic memory range %p...%p\n"),
       (uintptr_t)start,(uintptr_t)start+n_bytes-1);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)n_bytes,PAGESIZE));
 assert(addr_isphys(start));
 assert(mzone_of((void *)start) == zone_id);
 assertf(mzone_of((void *)((uintptr_t)start+n_bytes-1)) == zone_id,
         "Different zone at end of %p...%p (Isn't %d)",
         start,(uintptr_t)start+n_bytes-1,zone_id);
 assertf((uintptr_t)start+(n_bytes-1) >= (uintptr_t)start,
         FREESTR("Pointer overflow while freeing pages: %p + %Id(%Ix) overflows into %p"),
        (uintptr_t)start,n_bytes,n_bytes,(uintptr_t)start+(n_bytes-1));
 free_end = (ppage_t)((uintptr_t)start+n_bytes);
 /* Load the zone this pointer is apart of. */
 zone = PAGEZONE(zone_id);
 piter = &zone->z_root;
 while ((iter = *piter) != PAGE_ERROR &&
         PAGE_END(iter) <= start) {
  assert(iter < start);
  piter = &iter->p_free.p_next;
 }
 assertf(iter == PAGE_ERROR || (uintptr_t)iter >= (uintptr_t)start+n_bytes,
         FREESTR("At least part of address range %p...%p was already marked as free by %p...%p"),
         start,(uintptr_t)free_end-1,iter,(uintptr_t)PAGE_END(iter)-1);
 /* Insert after `piter' / before `iter' */

 /* Check for extending the previous range. */
 if (piter != &zone->z_root) {
  ppage_t prev_page = container_of(piter,union page,p_free.p_next);
  if (PAGE_END(prev_page) == start) {
   /* Extend the previous range. */
   prev_page->p_free.p_size += n_bytes;
   prev_page->p_free.p_attr  = PAGEATTR_NONE;
   assert(prev_page->p_free.p_next == iter);
   assert(PAGE_END(prev_page) <= iter);
   if unlikely(PAGE_END(prev_page) == iter) {
    /* Extending the previous range causes it to touch the next. - Merge the two. */
    prev_page->p_free.p_attr &= iter->p_free.p_attr;
    prev_page->p_free.p_size += iter->p_free.p_size;
    prev_page->p_free.p_next  = iter->p_free.p_next;
    if (prev_page->p_free.p_next != PAGE_ERROR)
        prev_page->p_free.p_next->p_free.p_self = &prev_page->p_free.p_next;
   }
   goto done;
  }
 }

 /* Check for merging with the next range. */
 if (iter != PAGE_ERROR &&
    (uintptr_t)iter == (uintptr_t)start+n_bytes) {
  start->p_free         = iter->p_free;
  start->p_free.p_size += n_bytes;
  start->p_free.p_attr &= PAGEATTR_NONE;
  if (start->p_free.p_next != PAGE_ERROR)
      start->p_free.p_next->p_free.p_self = &start->p_free.p_next;
  *start->p_free.p_self = start;
  goto done;
 }

 /* Fallback: Create & insert a new free-range. */
 start->p_free.p_size = n_bytes;
 start->p_free.p_self = piter;
 start->p_free.p_next = iter;
 start->p_free.p_attr = PAGEATTR_NONE;
 *piter               = start;
 if (iter != PAGE_ERROR)
     iter->p_free.p_self = &start->p_free.p_next;

done:
#ifdef CONFIG_DEBUG
 PAGE_FOREACH(iter,zone) {}
#endif
 zone->z_avail += n_bytes;
#else
 if unlikely(!n_bytes) return 0;
 syslog(LOG_DEBUG,FREESTR("[MEM] Using dynamic memory range %p...%p\n"),
       (uintptr_t)start,(uintptr_t)start+n_bytes-1);
 page_free(start,n_bytes);
#endif
 return n_bytes;
}

PUBLIC char const memtype_names[MEMTYPE_COUNT][8] = {
    [MEMTYPE_NDEF]      = "-",
    [MEMTYPE_PRESERVE]  = "presrv",
    [MEMTYPE_RAM]       = "ram",
#ifdef MEMTYPE_ALLOCATED
    [MEMTYPE_ALLOCATED] = "alloc",
#endif
    [MEMTYPE_KFREE]     = "kfree",
    [MEMTYPE_KERNEL]    = "kernel",
    [MEMTYPE_NVS]       = "nvs",
    [MEMTYPE_DEVICE]    = "device",
    [MEMTYPE_BADRAM]    = "badram",
};

#ifdef __i386__
#define STATIC_MEMINFO_MAX ((HOST_IDLE_STCKSIZE- \
                            (sizeof(struct cpustate_host)))/ \
                             sizeof(struct meminfo))
#else
#define STATIC_MEMINFO_MAX ((HOST_IDLE_STCKSIZE- \
                            (sizeof(struct cpustate)))/ \
                             sizeof(struct meminfo))
#endif

INTDEF struct PACKED {
 struct meminfo s_kerninfo[STATIC_MEMINFO_MAX]; /*< Vector of statically allocated meminfo slots. */
} __bootidlestack;

/* NOTE: 'STATIC_MEMINFO_MAX' currently evaluates to '2041', which should be
 *        more than enough distinct memory region for any kind of host system. */
enum{_STATIC_MEMINFO_MAX = STATIC_MEMINFO_MAX};
#define MEMINFO_MAXCOUNT  _STATIC_MEMINFO_MAX

PUBLIC size_t _mem_info_c ASMNAME("mem_info_c") = MEMORY_PREDEF_COUNT;
PUBLIC struct meminfo const *_mem_info_v ASMNAME("mem_info_v") = __bootidlestack.s_kerninfo;
PUBLIC struct meminfo const *_mem_info_last ASMNAME("mem_info_last") = __bootidlestack.s_kerninfo+(MEMORY_PREDEF_COUNT-1);

/* During early memory initialization, we can directly address the 's_kerninfo' symbol
 * to get rid of an additional indirection that would occurr if '_mem_info_v' was used. */
#define MEMINFO_V  (__bootidlestack.s_kerninfo)
#define MEMINFO_C   _mem_info_c
#define SET_MEMINFO_C(x) (_mem_info_last = MEMINFO_V+(_mem_info_c = (x))-1)
#define INC_MEMINFO_C()  (++_mem_info_last,++_mem_info_c)
#define DEC_MEMINFO_C()  (--_mem_info_last,--_mem_info_c)
#define MEMINFO_HASPREV(x) ((x) != MEMINFO_V)
#define MEMINFO_HASNEXT(x) ((x) != _mem_info_last)
#define MEMINFO_PREV(x)    ((x)-1)
#define MEMINFO_NEXT(x)    ((x)+1)
#undef MEMINFO_FOREACH
#define MEMINFO_FOREACH(iter) \
 for ((iter) = MEMINFO_V; (iter) <= _mem_info_last; ++(iter))

#define MEMINFO_DELETE(info) \
 memmove((info),(info)+1,\
        ((MEMINFO_V+DEC_MEMINFO_C())-(info))*sizeof(struct meminfo))
#define MEMINFO_DUP(info) \
 (memmove((info)+1,(info),\
         ((MEMINFO_V+MEMINFO_C)-(info))*sizeof(struct meminfo)),\
  INC_MEMINFO_C())


PRIVATE ATTR_FREETEXT void KCALL
page_do_addmemory(PAGE_ALIGNED uintptr_t base, PAGE_ALIGNED size_t n_bytes) {
 mzone_t zone;
 for (zone = 0; zone < MZONE_REAL_COUNT; ++zone) {
  if (base >= MZONE_MIN(zone) &&
      base <= MZONE_MAX(zone)) {
   size_t zone_bytes = MIN((MZONE_MAX(zone)+1)-base,n_bytes);
   early_map_identity((void *)base,zone_bytes);
#ifdef __x86_64__
   /* Make sure not to install memory already in use by `early_map_identity()'.
    * If we didn't check this, early identity mappings of physical memory would
    * break as pages already in use by the kernel page directory were registered
    * as available RAM despite not being available as such.
    * NOTE: During initialization of the kernel page directory, any memory
    *       that we didn't install here will get re-added as available RAM.
    * NOTE: Additional overlapping after memory has already been installed
    *       should be impossible because `early_page_malloc()' always tries
    *       to allocate memory using `page_malloc()' first, before falling back
    *       to blindly assuming available RAM past `KERNEL_END'
    */
#define EARLY_PAGE_BEGIN_P  (EARLY_PAGE_BEGIN-CORE_BASE)
#define EARLY_PAGE_END_P      (EARLY_PAGE_END-CORE_BASE)
   if (base < EARLY_PAGE_END_P &&
       base+zone_bytes > EARLY_PAGE_BEGIN_P) {
    /* Overlap! */
    syslog(LOG_DEBUG,"[MEM] Skipping early_page_malloc-overlap %p...%p\n",
           MAX(base,EARLY_PAGE_BEGIN_P),MIN(base+zone_bytes,EARLY_PAGE_END_P)-1);
    if (base < EARLY_PAGE_BEGIN_P)
        page_addmemory(zone,(ppage_t)base,EARLY_PAGE_BEGIN_P-base);
    if (base+zone_bytes > EARLY_PAGE_END_P)
        page_addmemory(zone,(ppage_t)EARLY_PAGE_END_P,
                      (base+zone_bytes)-EARLY_PAGE_END_P);
   } else {
    /* Just some regular, old memory. */
    page_addmemory(zone,(ppage_t)base,zone_bytes);
   }
#else
   page_addmemory(zone,(ppage_t)base,zone_bytes);
#endif
   n_bytes -= zone_bytes;
   if (!n_bytes) break;
   base += zone_bytes;
  }
 }
}
PRIVATE ATTR_FREETEXT bool KCALL
page_do_delmemory(PAGE_ALIGNED uintptr_t base, PAGE_ALIGNED size_t n_bytes) {
 /* TODO: This call should not count towards memory statistics! */
 return page_malloc_at((ppage_t)base,n_bytes,PAGEATTR_NONE) != PAGE_ERROR;
}

/* Split meminfo at 'addr' and return a pointer to
 * the upper block (the one starting at 'addr') */
PRIVATE ATTR_FREETEXT
struct meminfo *KCALL meminfo_split_at(PHYS uintptr_t addr) {
 struct meminfo *iter = MEMINFO_V;
 while ((uintptr_t)iter->mi_addr < addr) {
  if (++iter == MEMINFO_V+MEMINFO_C) {
   /* Append at the end. */
   if (MEMINFO_C == MEMINFO_MAXCOUNT)
       PANIC(FREESTR("No more available memory information slots\n"));
   INC_MEMINFO_C();
   iter->mi_type = MEMINFO_PREV(iter)->mi_type;
   goto setup_iter;
  }
 }
 if ((uintptr_t)iter->mi_addr != addr) {
  /* Must insert a split here. */
  if (MEMINFO_C == MEMINFO_MAXCOUNT)
      PANIC(FREESTR("No more available memory information slots\n"));
  MEMINFO_DUP(iter);
  iter->mi_type = MEMINFO_HASPREV(iter) ? MEMINFO_PREV(iter)->mi_type : MEMTYPE_NDEF;
setup_iter:
  iter->mi_addr = (PHYS void *)addr;
 }
 return iter;
}
#define meminfo_merge_with_next(info) \
        meminfo_merge_with_prev(MEMINFO_NEXT(info))
PRIVATE ATTR_FREETEXT void KCALL
meminfo_merge_with_prev(struct meminfo *__restrict info) {
 if (MEMINFO_HASPREV(info) &&
     MEMINFO_PREV(info)->mi_type == info->mi_type)
     MEMINFO_DELETE(info);
}

INTERN ATTR_FREETEXT PAGE_ALIGNED size_t KCALL
mem_install(PHYS uintptr_t base, size_t num_bytes, memtype_t type) {
 struct meminfo *start,*stop,*iter;
 size_t result = 0; uintptr_t addr_end;
 uintptr_t info_begin,info_end;
 if unlikely(!num_bytes) goto end;
 start = meminfo_split_at(base);
 assert((uintptr_t)start->mi_addr == base);
 if unlikely((addr_end = base+num_bytes) <= base) {
  /* Region extends until the end of the address space. */
  stop = MEMINFO_V+MEMINFO_C;
 } else {
  stop = meminfo_split_at(base+num_bytes);
  assert(start != stop);
  assert((uintptr_t)start->mi_addr == base);
  assert((uintptr_t)stop->mi_addr == addr_end);
 }
 assertf(start < stop,"%p...%p\n",base,base+num_bytes-1);
 /* Go over all information slots within
  * the given range and update them. */
 for (iter = start; iter < stop; ++iter) {
  if (type < iter->mi_type)
      continue; /* `type' is less significant. */
  if (type <= MEMTYPE_ALLOCATED) {
   info_begin = CEIL_ALIGN(MEMINFO_BEGIN(iter),PAGESIZE);
   info_end   = FLOOR_ALIGN(MEMINFO_END(iter),PAGESIZE);
   if (info_end > info_begin) {
    info_end -= info_begin;
    if (MEMTYPE_ISUSE(type)) {
     /* Make given memory available for use. */
     if (!MEMTYPE_ISUSE(iter->mi_type)) {
      page_do_addmemory(info_begin,info_end);
      result += info_end;
     }
    } else {
     /* Allocate the affected memory range again. */
     if (MEMTYPE_ISUSE(iter->mi_type)) {
      if (!page_do_delmemory(info_begin,info_end))
           goto iter_done;
      result += info_end;
     }
    }
   }
  } else {
   result += MEMINFO_SIZE(iter);
  }
iter_done:
  iter->mi_type = type;
  if (MEMINFO_HASPREV(iter) && MEMINFO_PREV(iter)->mi_type == MEMTYPE_RAM) { delete_iter: MEMINFO_DELETE(iter); --iter,--stop; }
  if (MEMINFO_HASNEXT(iter) && MEMINFO_NEXT(iter)->mi_type == MEMTYPE_RAM) { ++iter; goto delete_iter; }
 }

 /* Re-merge start and stop information slots. */
 if (stop != MEMINFO_V+MEMINFO_C)
     meminfo_merge_with_prev(stop);
 meminfo_merge_with_prev(start);
end:
 return result;
}

INTERN ATTR_FREETEXT
PAGE_ALIGNED size_t KCALL mem_unpreserve(void) {
 struct meminfo *iter;
 size_t result = 0;
 MEMINFO_FOREACH(iter) {
  uintptr_t info_begin,info_end;
  if (iter->mi_type != MEMTYPE_ALLOCATED &&
      iter->mi_type != MEMTYPE_PRESERVE)
      continue;
  if (iter->mi_type == MEMTYPE_PRESERVE) {
   /* Install previously preserved memory as general purpose RAM. */
   info_begin = CEIL_ALIGN(MEMINFO_BEGIN(iter),PAGESIZE);
   info_end   = FLOOR_ALIGN(MEMINFO_END(iter),PAGESIZE);
   if (info_end > info_begin) {
    info_end -= info_begin;
    page_do_addmemory(info_begin,info_end-info_begin);
    result += info_end;
   }
  }
  iter->mi_type = MEMTYPE_RAM;
  /* Check for merge with adjacent slots. */
  if (MEMINFO_HASPREV(iter) && MEMINFO_PREV(iter)->mi_type == MEMTYPE_RAM) { delete_iter: MEMINFO_DELETE(iter); --iter; }
  if (MEMINFO_HASNEXT(iter) && MEMINFO_NEXT(iter)->mi_type == MEMTYPE_RAM) { ++iter; goto delete_iter; }
 }
 return result;
}
INTERN ATTR_FREETEXT
void KCALL mem_relocate_info(void) {
 struct meminfo *new_info;
 new_info = (struct meminfo *)kmalloc(MEMINFO_C*
                                      sizeof(struct meminfo),
                                      GFP_SHARED);
 if unlikely(!new_info) PANIC(FREESTR("Failed to allocate %Iu bytes for relocated meminfo"),
                              MEMINFO_C*sizeof(struct meminfo));
 memcpy(new_info,MEMINFO_V,MEMINFO_C*sizeof(struct meminfo));
 _mem_info_v    = new_info;
 _mem_info_last = new_info+MEMINFO_C-1;
}




#if __SIZEOF_POINTER__ < 8
INTERN ATTR_FREETEXT PAGE_ALIGNED size_t KCALL
mem_install64(PHYS u64 base, u64 num_bytes, memtype_t type) {
 /* Check if the given range lies within the usable address space. */
 if unlikely(base > (uintptr_t)-1)
    return 0;
 /* Fix overflow wrapping around the entire address space. */
 if unlikely(base+num_bytes < base)
    num_bytes = 0-base;
 return mem_install((uintptr_t)base,
                    (uintptr_t)num_bytes,
                     type);
}
#endif


INTERN ATTR_FREERODATA u8 const memtype_bios_matrix[6] = {
    [0] = MEMTYPE_NDEF,   /* Undefined (Fallback). */
    [1] = MEMTYPE_RAM,    /* Available. */
    [2] = MEMTYPE_DEVICE, /* Reserved. */
    [3] = MEMTYPE_COUNT,  /* ACPI-Reclaimable. (Ignored) */
    [4] = MEMTYPE_NVS,    /* NVS. */
    [5] = MEMTYPE_BADRAM, /* Badram. */
};


INTERN ATTR_FREETEXT SAFE KPD size_t KCALL
memory_load_mb_mmap(struct mb_mmap_entry *__restrict iter, u32 info_len) {
 mb_memory_map_t *end; size_t result = 0;
 for (end  = (mb_memory_map_t *)((uintptr_t)iter+info_len); iter < end;
      iter = (mb_memory_map_t *)((uintptr_t)&iter->addr+iter->size)) {
  if (iter->type >= COMPILER_LENOF(memtype_bios_matrix)) iter->type = 0;
  if (memtype_bios_matrix[iter->type] >= MEMTYPE_COUNT) continue;
  result += mem_install64(iter->addr,iter->len,
                          memtype_bios_matrix[iter->type]);
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
  if (iter->type >= COMPILER_LENOF(memtype_bios_matrix)) iter->type = 0;
  if (memtype_bios_matrix[iter->type] >= MEMTYPE_COUNT) continue;
  result += mem_install64(iter->addr,iter->len,
                          memtype_bios_matrix[iter->type]);
 }
done:
 return result;
}

DECL_END

#endif /* !GUARD_KERNEL_MEMORY_INSTALL_C */
