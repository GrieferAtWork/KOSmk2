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

DECL_BEGIN

PRIVATE ATTR_FREETEXT size_t KCALL
page_addmemory(mzone_t zone_id, ppage_t start,
               PAGE_ALIGNED size_t n_bytes) {
#if 1
 ppage_t *piter,iter,free_end;
 struct mzone *zone;
#ifdef CONFIG_USE_NEW_MEMINFO
 if unlikely((uintptr_t)start >= KERNEL_BASE) return 0;
 if unlikely((uintptr_t)start+n_bytes > KERNEL_BASE)
    n_bytes = KERNEL_BASE-(uintptr_t)start;
#endif
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

#ifdef CONFIG_USE_NEW_MEMINFO
#ifdef __x86_64__
#define STATIC_MEMINFO_MAX ((HOST_IDLE_STCKSIZE- \
                            (sizeof(struct cpustate)))/ \
                             sizeof(struct meminfo))
#else
#define STATIC_MEMINFO_MAX ((HOST_IDLE_STCKSIZE- \
                            (sizeof(struct cpustate_host)))/ \
                             sizeof(struct meminfo))
#endif
#else
#define STATIC_MEMINFO_MAX ((HOST_IDLE_STCKSIZE- \
                            (sizeof(struct cpustate_host)+sizeof(size_t)))/ \
                             sizeof(struct meminfo))
#endif

INTDEF struct PACKED {
 struct meminfo s_kerninfo[STATIC_MEMINFO_MAX]; /*< Vector of statically allocated meminfo slots. */
} __bootidlestack;

#ifdef CONFIG_USE_NEW_MEMINFO
/* NOTE: 'STATIC_MEMINFO_MAX' currently evaluates to '2041', which should be
 *        more than enough distinct memory region for any kind of host system. */
enum{_STATIC_MEMINFO_MAX = STATIC_MEMINFO_MAX};
#define MEMINFO_MAXCOUNT  _STATIC_MEMINFO_MAX

PUBLIC size_t _mem_info_c ASMNAME("mem_info_c") = 6;
PUBLIC struct meminfo const *_mem_info_v ASMNAME("mem_info_v") = __bootidlestack.s_kerninfo;
PUBLIC struct meminfo const *_mem_info_last ASMNAME("mem_info_last") = __bootidlestack.s_kerninfo+5;

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

#else

/* The vector base address and max amount of statically allocated memory info slots. */
#define STATIC_MEMINFO_VEC (__bootidlestack.s_kerninfo)
#define STATIC_MEMINFO_CNT (__bootidlestack.s_kernused)
#define STATIC_MEMINFO_ONLY (STATIC_MEMINFO_CNT < STATIC_MEMINFO_MAX)

/* HINT: `STATIC_MEMINFO_MAX' currently equates to 510, meaning it's highly unlikely to be insufficient. */
enum{_STATIC_MEMINFO_MAX = STATIC_MEMINFO_MAX};
#undef STATIC_MEMINFO_MAX
#define STATIC_MEMINFO_MAX  _STATIC_MEMINFO_MAX

PUBLIC PHYS struct meminfo *mem_info_[MZONE_REAL_COUNT] ASMNAME("mem_info") = {
    [MZONE_1MB]   = MEMINFO_EARLY_NULL,
    [MZONE_HIMEM] = &__bootidlestack.s_kerninfo[0],
};

#define INFOPAGE_MAX  ((PAGESIZE/sizeof(struct meminfo))-1)
struct infopage {
 PAGE_ALIGNED
 struct infopage *ip_next;   /*< [0..1|null(PAGE_ERROR)] Next allocated info page. */
union{size_t      ip_used;   /*< [<= INFOPAGE_MAX] Amouunt of used meminfo slots. */
 uintptr_t      __ip_pad0;}; /*< ... */
 u8             __ip_pad1[sizeof(struct meminfo)-2*sizeof(void *)];
 struct meminfo   ip_data[INFOPAGE_MAX]; /*< Meminfo slot vector. */
};
STATIC_ASSERT(sizeof(struct infopage) <= PAGESIZE);

/* Chain of additional physical memory allocations
 * used to extend upon memory information slots. */
PRIVATE ATTR_FREEDATA PAGE_ALIGNED
struct infopage *early_extension = PAGE_ERROR;

/* Allocate a new memory information record, for used during early boot. */
PRIVATE ATTR_FREETEXT
struct meminfo *KCALL meminfo_early_alloc(void) {
 /* Simple case: Allocate from the static buffer located
  * on the IDLE-task stack, which is currently unused. */
 if likely(STATIC_MEMINFO_CNT < STATIC_MEMINFO_MAX)
    return &STATIC_MEMINFO_VEC[STATIC_MEMINFO_CNT++];
 if (early_extension == PAGE_ERROR ||
     early_extension->ip_used >= INFOPAGE_MAX) {
  struct infopage *newpage;
  /* Must allocate a new physical data page to extend static meminfo data. */
  newpage = (struct infopage *)page_malloc(sizeof(struct infopage),
                                           PAGEATTR_NONE,MZONE_ANY);
  if unlikely(newpage == PAGE_ERROR)
     PANIC(FREESTR("Failed to allocate an extend memory information page"));
  /* Link the new page. */
  newpage->ip_used = 0;
  newpage->ip_next = early_extension;
  early_extension = newpage;
 }
 /* Return an entry from the extension buffer. */
 return &early_extension->ip_data[early_extension->ip_used++];
}

/* Free all extended memory allocated for early memory info. */
PRIVATE ATTR_FREETEXT void KCALL meminfo_early_freeall(void) {
 while unlikely(early_extension != PAGE_ERROR) {
  struct infopage *next = early_extension->ip_next;
  page_free((ppage_t)early_extension,sizeof(struct infopage));
  early_extension = next;
 }
}


LOCAL ATTR_FREETEXT void KCALL
meminfo_load_part_full(struct meminfo *__restrict self) {
 uintptr_t base = (uintptr_t)self->mi_addr;
 size_t num_bytes = self->mi_size;
 self->mi_part_addr = (ppage_t)FLOOR_ALIGN(base,PAGESIZE);
 self->mi_part_size =          CEIL_ALIGN(num_bytes+(base-(uintptr_t)self->mi_part_addr),PAGESIZE);
 self->mi_full_addr = (ppage_t)CEIL_ALIGN(base,PAGESIZE);
 assertf(CEIL_ALIGN(base+num_bytes,PAGESIZE) >= (uintptr_t)self->mi_full_addr,
         "base      = %p\n"
         "num_bytes = %p\n",base,num_bytes);
 self->mi_full_size =          CEIL_ALIGN(base+num_bytes,PAGESIZE)-(uintptr_t)self->mi_full_addr;
}

/* Resolve a potential `mi_part_addr...+=mi_part_size' overlap between `prev' and `next' */
LOCAL void KCALL
mem_resolve_part_overlap(struct meminfo *__restrict prev,
                         struct meminfo *__restrict next) {
 assert((uintptr_t)prev->mi_full_addr+prev->mi_full_size <=
        (uintptr_t)next->mi_full_addr);
 if ((uintptr_t)prev->mi_part_addr+prev->mi_part_size >
     (uintptr_t)next->mi_part_addr) {
  size_t size_diff;
  assert((uintptr_t)next->mi_part_addr == (uintptr_t)next->mi_full_addr-PAGESIZE);
  assert((uintptr_t)next->mi_part_addr == (uintptr_t)prev->mi_full_addr+prev->mi_full_size-PAGESIZE);
  /* Figure out which of the regions to truncate. */
  if (next->mi_type == MEMTYPE_RAM) {
   assert((uintptr_t)next->mi_full_addr >= (uintptr_t)next->mi_addr);
   /* Truncate `next' */
   size_diff = (size_t)((uintptr_t)next->mi_full_addr-
                        (uintptr_t)next->mi_addr);
   if (size_diff > next->mi_size)
       size_diff = next->mi_size;
   next->mi_size     -= size_diff;
   next->mi_addr      = next->mi_full_addr;
   next->mi_part_addr = next->mi_full_addr;
   next->mi_part_size = CEIL_ALIGN(next->mi_size,PAGESIZE);
  } else {
   /* Truncate `prev' */
   size_diff = ((uintptr_t)next->mi_part_addr-
                (uintptr_t)prev->mi_addr);
   assertf(prev->mi_size > size_diff,
           "%p...%p  %p...%p",
           prev->mi_addr,(uintptr_t)prev->mi_addr+prev->mi_size-1,
           next->mi_addr,(uintptr_t)next->mi_addr+next->mi_size-1);
   prev->mi_size = size_diff;
   meminfo_load_part_full(prev);
   assert((uintptr_t)prev->mi_part_addr+prev->mi_part_size <= (uintptr_t)next->mi_part_addr);
   assert((uintptr_t)prev->mi_part_addr+prev->mi_part_size >= (uintptr_t)prev->mi_part_addr);
   assert((uintptr_t)prev->mi_full_addr+prev->mi_full_size >= (uintptr_t)prev->mi_full_addr);
  }
 }
 assert((uintptr_t)prev->mi_part_addr+prev->mi_part_size <=
        (uintptr_t)next->mi_part_addr);
}

PRIVATE ATTR_FREETEXT void KCALL
mem_unusable_address_range(PHYS uintptr_t base,
                           size_t num_bytes,
                           memtype_t type) {
 syslog(LOG_MEM|LOG_WARN,
        FREESTR("[MEM] Unusable <%s> address range %p...%p above 3Gb\n"),
        memtype_names[type],base,base+num_bytes-1);
}
PRIVATE ATTR_FREETEXT void KCALL
mem_overlapping_address_range(PHYS uintptr_t base, size_t num_bytes,
                              memtype_t old_type, memtype_t new_type) {
 /* Don't report overlaps in preserved regions. */
 if (old_type == MEMTYPE_PRESERVE) return;
 if (new_type == MEMTYPE_PRESERVE && old_type == MEMTYPE_RAM) {
  /* TODO: Change the old memory type to 'MEMTYPE_PRESERVE'. */
 }
 syslog(LOG_MEM|LOG_WARN,
        FREESTR("[MEM] Overlapping <%s> address range with <%s> at %p...%p\n"),
        memtype_names[old_type],memtype_names[new_type],base,base+num_bytes-1);
}


INTERN ATTR_FREETEXT PAGE_ALIGNED size_t KCALL
mem_install_zone(PHYS uintptr_t base, size_t num_bytes,
                 memtype_t type, mzone_t zone_id) {
 struct meminfo *iter,**piter,*new_info,*prev;
 size_t result = 0; uintptr_t addr_end;
 assert(num_bytes);
 assert((uintptr_t)base+num_bytes > (uintptr_t)base);
 assert((uintptr_t)base >= MZONE_MIN(zone_id));
 assert((uintptr_t)base+num_bytes-1 <= MZONE_MAX(zone_id));
 piter = &mem_info_[zone_id];
 /* Figure out the memory info record before which to insert this zone. */
 while ((iter = *piter) != MEMINFO_EARLY_NULL &&
        (PHYS uintptr_t)iter->mi_addr < base) {
  assertf(piter == &mem_info_[zone_id] ||
         (uintptr_t)container_of(piter,struct meminfo,mi_next)->mi_addr+
                    container_of(piter,struct meminfo,mi_next)->mi_size <=
         (uintptr_t)iter->mi_addr,
          FREESTR("Illegal overlap: %p...%p %p...%p"),
         (uintptr_t)container_of(piter,struct meminfo,mi_next)->mi_addr,
         (uintptr_t)container_of(piter,struct meminfo,mi_next)->mi_addr+
                    container_of(piter,struct meminfo,mi_next)->mi_size-1,
         (uintptr_t)iter->mi_addr,(uintptr_t)iter->mi_addr+iter->mi_size-1);
  piter = (struct meminfo **)&iter->mi_next;
 }

 /* Check the next range to handle overlap/extending below. */
 if (iter != MEMINFO_EARLY_NULL &&
    (addr_end = base+num_bytes) >= (uintptr_t)iter->mi_addr) {
  /* Check for overlap above. */
  if (addr_end != (uintptr_t)iter->mi_addr) {
   uintptr_t iter_end = (uintptr_t)iter->mi_addr+iter->mi_size;
   mem_overlapping_address_range((uintptr_t)iter->mi_addr,
                                  MIN(addr_end,iter_end)-(uintptr_t)iter->mi_addr,
                                  iter->mi_type,type);
   /* Map memory above the overlap. */
   if (addr_end > iter_end)
       result += mem_install_zone(iter_end,addr_end-iter_end,type,zone_id);
   addr_end = (uintptr_t)iter->mi_addr;
   if unlikely(base >= addr_end)
      return result; /* Full overlap */
   num_bytes = addr_end-base;
   assert(base+num_bytes > base);
  }

  if (iter->mi_type == type) {
   ppage_t old_full_addr;
   ppage_t new_full_addr;
   /* Extent the next region downwards. */
   iter->mi_addr  = (PHYS void *)base;
   iter->mi_size += num_bytes;
   old_full_addr  = iter->mi_full_addr;
   meminfo_load_part_full(iter);
   new_full_addr  = iter->mi_full_addr;

   /* Check for merging with the previous region. */
   if (piter != &mem_info_[zone_id]) {
    prev = container_of(piter,struct meminfo,mi_next);
    assert((uintptr_t)prev->mi_addr+prev->mi_size <=
           (uintptr_t)iter->mi_addr);
    /* Fix overlap with the previous range. */
    if (type != MEMTYPE_PRESERVE)
        mem_resolve_part_overlap(prev,iter);
    if ((uintptr_t)prev->mi_addr+prev->mi_size ==
        (uintptr_t)iter->mi_addr) {
     /* Merge the two regions. */
     prev->mi_next       = iter->mi_next;
     prev->mi_size      += iter->mi_size;
     prev->mi_part_size += iter->mi_part_size;
     prev->mi_full_size += iter->mi_full_size;
     assert(type == MEMTYPE_PRESERVE ||
            prev->mi_next == MEMINFO_EARLY_NULL ||
           (uintptr_t)prev->mi_part_addr+prev->mi_part_size <=
           (uintptr_t)prev->mi_next->mi_part_addr);
    }
   }

   /* Make the memory available if it is RAM. */
   if (MEMTYPE_ISUSE(type)) {
    assert(old_full_addr >= new_full_addr);
    result += page_addmemory(zone_id,new_full_addr,
                            (uintptr_t)old_full_addr-
                            (uintptr_t)new_full_addr);
   }
   return result;
  }
 }
 assert(base+num_bytes > base);

 if (piter != &mem_info_[zone_id]) {
  uintptr_t prev_end;
  /* Check for extending the with the previous region. */
  prev = container_of(piter,struct meminfo,mi_next);
  assert((uintptr_t)prev->mi_addr <= base);
  prev_end = (uintptr_t)prev->mi_addr+prev->mi_size;
  /* Check for overflow at the base. */
  if (prev_end > base) {
   /* Notify the system of the overlap. */
   mem_overlapping_address_range(base,MIN(prev_end,addr_end)-base,
                                 prev->mi_type,type);
   /* Figure out what is remaining. */
   if (prev_end >= addr_end)
       return result;
   num_bytes = addr_end-prev_end;
  }
  if (prev_end == base && prev->mi_type == type) {
   prev->mi_size += num_bytes;
   meminfo_load_part_full(prev);
   if (type != MEMTYPE_PRESERVE && prev->mi_next != MEMINFO_EARLY_NULL)
       mem_resolve_part_overlap(prev,(struct meminfo *)prev->mi_next);
   assertf(type != MEMTYPE_PRESERVE || prev->mi_next == MEMINFO_EARLY_NULL ||
         ((uintptr_t)prev->mi_addr+prev->mi_size <= (uintptr_t)prev->mi_next->mi_addr &&
          (uintptr_t)prev->mi_part_addr+prev->mi_part_size <= (uintptr_t)prev->mi_next->mi_part_addr),
           FREESTR("PREV %p...%p/%p...%p\n"
                   "NEXT %p...%p/%p...%p\n"
                   "ADD  %p...%p"),
          (uintptr_t)prev->mi_addr,
          (uintptr_t)prev->mi_addr+prev->mi_size-1,
          (uintptr_t)prev->mi_part_addr,
          (uintptr_t)prev->mi_part_addr+prev->mi_part_size-1,
          (uintptr_t)prev->mi_next->mi_addr,
          (uintptr_t)prev->mi_next->mi_addr+(uintptr_t)prev->mi_next->mi_size-1,
          (uintptr_t)prev->mi_next->mi_part_addr,
          (uintptr_t)prev->mi_next->mi_part_addr+(uintptr_t)prev->mi_next->mi_part_size-1,
           base,base+num_bytes-1);
   if (MEMTYPE_ISUSE(type)) {
    uintptr_t aligned_base = CEIL_ALIGN(base,PAGESIZE);
    uintptr_t aligned_end  = FLOOR_ALIGN(base+num_bytes,PAGESIZE);
    if (aligned_end > aligned_base)
        result += page_addmemory(zone_id,(ppage_t)aligned_base,
                                 aligned_end-aligned_base);
   }
   return result;
  }
 }
 assert(base+num_bytes > base);


 /* Create and insert a new info record. */
 assert(num_bytes);
 new_info = meminfo_early_alloc();
 new_info->mi_next = iter;
 new_info->mi_type = type;
 new_info->mi_addr = (PHYS void *)base;
 new_info->mi_size = num_bytes;
 meminfo_load_part_full(new_info);
 if (!new_info->mi_full_size) return result; /* TODO: Incorrect */
 *piter = new_info;

 /* Fix overlaps. */
 if (type != MEMTYPE_PRESERVE) {
  if (piter != &mem_info_[zone_id])
      mem_resolve_part_overlap(container_of(piter,struct meminfo,mi_next),new_info);
  if (iter != MEMINFO_EARLY_NULL)
      mem_resolve_part_overlap(new_info,iter);
 }

 /* Make the memory available for use by the physical memory allocator. */
 if (MEMTYPE_ISUSE(type)) {
  result += page_addmemory(zone_id,
                           new_info->mi_full_addr,
                           new_info->mi_full_size);
 }
 return result;
}





INTERN ATTR_FREETEXT PAGE_ALIGNED size_t KCALL
mem_install(PHYS uintptr_t base, size_t num_bytes, memtype_t type) {
 mzone_t zone; size_t result = 0;
 /* Step #1: Fix overflow. */
 if unlikely(base+num_bytes < base)
    num_bytes = 0-base;
 /* Step #2: Ignore empty memory ranges. */
 if unlikely(!num_bytes) return 0;
 /* Step #3: Ignore unusable memory above KERNEL_BASE. */
 if unlikely(base >= KERNEL_BASE) {
  mem_unusable_address_range(base,num_bytes,type);
  return 0;
 }
 /* Step #4: Check for partial collision with KERNEL_BASE. */
 if unlikely(base+num_bytes > KERNEL_BASE) {
  mem_unusable_address_range(KERNEL_BASE,
                            (base+num_bytes)-KERNEL_BASE,
                             type);
  num_bytes = KERNEL_BASE-base;
 }

 /* Step #5: Enumerate all zones that the range is apart of and install it in each. */
 for (zone = 0; zone < MZONE_REAL_COUNT; ++zone) {
  if (base >= MZONE_MIN(zone) &&
      base <= MZONE_MAX(zone)) {
   size_t zone_bytes = MIN((MZONE_MAX(zone)+1)-base,num_bytes);
   result += mem_install_zone(base,zone_bytes,type,zone);
   num_bytes -= zone_bytes;
   if (!num_bytes) break;
   base += zone_bytes;
  }
 }
 return result;
}



INTERN ATTR_FREETEXT
PAGE_ALIGNED size_t KCALL mem_unpreserve(void) {
 size_t result = 0; mzone_t zone;
 struct meminfo *iter,*prev,*next;
 for (zone = 0; zone < MZONE_REAL_COUNT; ++zone) {
  prev = MEMINFO_EARLY_NULL;
  for (iter = mem_info_[zone];
       iter != MEMINFO_EARLY_NULL;
       prev = iter,iter = (struct meminfo *)iter->mi_next) {
   assert(iter->mi_part_addr == (ppage_t)FLOOR_ALIGN((uintptr_t)iter->mi_addr,PAGESIZE));
   assert(iter->mi_part_size ==          CEIL_ALIGN(iter->mi_size+((uintptr_t)iter->mi_addr-(uintptr_t)iter->mi_part_addr),PAGESIZE));
   assert(iter->mi_full_addr == (ppage_t)CEIL_ALIGN((uintptr_t)iter->mi_addr,PAGESIZE));
   assert(iter->mi_full_size ==          CEIL_ALIGN((uintptr_t)iter->mi_addr+iter->mi_size,PAGESIZE)-(uintptr_t)iter->mi_full_addr);
   /* Fix potential part overlaps. */
   if (prev != MEMINFO_EARLY_NULL)
       mem_resolve_part_overlap(prev,iter);
   if (iter->mi_type == MEMTYPE_PRESERVE) {
    STATIC_ASSERT(MEMTYPE_ISUSE(MEMTYPE_RAM));
    /* Make this memory available. */
    result += page_addmemory(zone,iter->mi_full_addr,iter->mi_full_size);
    iter->mi_type = MEMTYPE_RAM;
    /* Check if we can merge these regions now. */
    if (prev->mi_type == MEMTYPE_RAM &&
       (uintptr_t)prev->mi_addr+prev->mi_size == iter->mi_size) {
     /* Yes, we can! */
     prev->mi_size      += iter->mi_size;
     prev->mi_full_size += iter->mi_full_size;
     prev->mi_part_size += iter->mi_part_size;
     prev->mi_next       = iter->mi_next;
     iter = prev;
    }
    /* Also check for merge with the successor. */
    next = (struct meminfo *)iter->mi_next;
    if (next != MEMINFO_EARLY_NULL &&
        next->mi_type == MEMTYPE_RAM &&
       (uintptr_t)next->mi_addr ==
       (uintptr_t)iter->mi_addr+iter->mi_size) {
     /* OK. Merge this one (too; potentially) */
     iter->mi_size      += next->mi_size;
     iter->mi_part_size += next->mi_part_size;
     iter->mi_full_size += next->mi_full_size;
     iter->mi_next       = next->mi_next;
    }
    assert(next == MEMINFO_EARLY_NULL ||
          (uintptr_t)iter->mi_part_addr+iter->mi_part_size <=
          (uintptr_t)next->mi_part_addr);
   }
  }
 }
 return result;
}



INTERN ATTR_FREETEXT void KCALL mem_relocate_info(void) {
 mzone_t zone;
 struct meminfo const *info;
 struct meminfo *new_info;
 struct meminfo **piter;
 struct meminfo *new_info_vec[MZONE_REAL_COUNT];
 /* Allocate a chunk of continuous memory within
  * which we're going to re-build memory information. */
 new_info = (struct meminfo *)kmalloc(STATIC_MEMINFO_CNT*sizeof(struct meminfo),GFP_SHARED);
 if unlikely(!new_info) PANIC(FREESTR("Failed to allocate %Iu bytes for relocated meminfo"),
                              STATIC_MEMINFO_CNT*sizeof(struct meminfo));
 _mall_nofree(new_info); /* This one's never supposed to be freed. */

 for (zone = 0; zone < MZONE_REAL_COUNT; ++zone) {
  piter = &new_info_vec[zone];
  MEMINFO_EARLY_FOREACH(info,zone) {
   *piter = new_info;
   /* Copy memory information. */
   memcpy(new_info,info,sizeof(struct meminfo));
   piter = (struct meminfo **)&new_info->mi_next;
   ++new_info;
  }
  /* Terminate the zone chain.
   * NOTE: Use NULL this time, as only EARLY meminfo is uses a different NULL-pointer,
   *       whereas this function is what marks the end of EARLY meminfo! */
  *piter = NULL;
 }

 /* Copy the new memory information vector into the global meminfo data vector. */
 memcpy(mem_info_,new_info_vec,
        MZONE_REAL_COUNT*sizeof(struct meminfo *));

 /* Free any additional physical pages that were allocated for memory information. */
 meminfo_early_freeall();

#ifdef CONFIG_DEBUG
 /* Restore debug pre-initialization of the BOOTCPU IDLE stack. */
 memsetl(&__bootidlestack.s_kernused,KERNEL_DEBUG_MEMPAT_HOSTSTACK,
        (sizeof(size_t)+(STATIC_MEMINFO_CNT*sizeof(struct meminfo)))/4);
#endif /* CONFIG_DEBUG */
}
#endif




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
