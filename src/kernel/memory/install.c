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
#include <kernel/arch/cpustate.h>
#include <kernel/paging.h>

#ifdef CONFIG_NEW_MEMINFO
DECL_BEGIN

PUBLIC char const memtype_names[MEMTYPE_COUNT][8] = {
   [MEMTYPE_RAM   ] = "ram",
   [MEMTYPE_NVS   ] = "nvs",
   [MEMTYPE_DEVICE] = "device",
   [MEMTYPE_KERNEL] = "kernel",
   [MEMTYPE_KFREE ] = "kfree",
   [MEMTYPE_BADRAM] = "badram",
};


/* The vector base address and max amount of statically allocated memory info slots. */
#define STATIC_MEMINFO_VEC (__bootidlestack.s_kerninfo)
#define STATIC_MEMINFO_CNT (__bootidlestack.s_kernused)
#define STATIC_MEMINFO_MAX ((TASK_HOSTSTACK_IDLESIZE- \
                            (sizeof(struct host_cpustate)+sizeof(size_t)))/ \
                             sizeof(struct meminfo))
#define STATIC_MEMINFO_ONLY (STATIC_MEMINFO_CNT < STATIC_MEMINFO_MAX)


/* HINT: 'STATIC_MEMINFO_MAX' currently equates to 510, meaning it's highly unlikely to be insufficient. */
enum{_STATIC_MEMINFO_MAX = STATIC_MEMINFO_MAX};
#undef STATIC_MEMINFO_MAX
#define STATIC_MEMINFO_MAX  _STATIC_MEMINFO_MAX

INTDEF struct PACKED {
 size_t         s_kernused; /*< Total amount of allocated memory information slots.
                             *  HINT: While '< STATIC_MEMINFO_ONLY', memory info is _ONLY_ stored in 's_kerninfo'! */
 struct meminfo s_kerninfo[STATIC_MEMINFO_MAX]; /*< Vector of statically allocated meminfo slots. */
} __bootidlestack;
PUBLIC PHYS struct meminfo *mem_info_[MZONE_COUNT] ASMNAME("mem_info") = {
    [MZONE_1MB]     = MEMINFO_EARLY_NULL,
    [MZONE_DEV]     = MEMINFO_EARLY_NULL,
    [MZONE_SHARE]   = &__bootidlestack.s_kerninfo[0],
    [MZONE_NOSHARE] = MEMINFO_EARLY_NULL,
};

LOCAL ATTR_FREETEXT void KCALL
meminfo_load_part_full(struct meminfo *__restrict self) {
 uintptr_t base = (uintptr_t)self->mi_addr;
 size_t num_bytes = self->mi_size;
 self->mi_part_addr = (ppage_t)FLOOR_ALIGN(base,PAGESIZE);
 self->mi_part_size =          CEIL_ALIGN(num_bytes+(base-(uintptr_t)self->mi_part_addr),PAGESIZE);
 self->mi_full_addr = (ppage_t)CEIL_ALIGN(base,PAGESIZE);
 self->mi_full_size =          CEIL_ALIGN(base+num_bytes,PAGESIZE)-(uintptr_t)self->mi_full_addr;
}

PRIVATE ATTR_FREETEXT size_t KCALL
page_addmemory(mzone_t zone_id, ppage_t start,
               PAGE_ALIGNED size_t n_bytes) {
#if 1
 ppage_t *piter,iter,free_end;
 struct mzone *zone;
 if unlikely(!n_bytes) return 0;
 syslog(LOG_DEBUG,"[MEM] Using dynamic memory range %p...%p\n",
       (uintptr_t)start,(uintptr_t)start+n_bytes-1);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)n_bytes,PAGESIZE));
 assert(addr_isphys(start));
 assert(mzone_of((void *)start) == zone_id);
 assert(mzone_of((void *)((uintptr_t)start+n_bytes-1)) == zone_id);
 assertf((uintptr_t)start+(n_bytes-1) >= (uintptr_t)start,
         FREESTR("Pointer overflow while freeing pages: %p + %Id(%Ix) overflows into %p"),
        (uintptr_t)start,n_bytes,n_bytes,(uintptr_t)start+(n_bytes-1));
 free_end = (ppage_t)((uintptr_t)start+n_bytes);
 if (zone_id == MZONE_DEV) {
  /* Special case: Ignore free requested for the DEV memory zone. */
  syslog(LOG_MEM|LOG_WARN,
         FREESTR("[MEM] Ignoring RAM %p..%p apart of the device memory zone\n"),
        (uintptr_t)start,(uintptr_t)free_end-1);
  return 0;
 }
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
 /* Insert after 'piter' / before 'iter' */

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
 syslog(LOG_DEBUG,"[MEM] Using dynamic memory range %p...%p\n",
       (uintptr_t)start,(uintptr_t)start+n_bytes-1);
 page_free(start,n_bytes);
#endif
 return n_bytes;
}


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
PRIVATE ATTR_FREEDATA PAGE_ALIGNED struct infopage *early_extension = PAGE_ERROR;

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


/* Resolve a potential 'mi_part_addr...+=mi_part_size' overlap between 'prev' and 'next' */
LOCAL void KCALL
mem_resolve_part_overlap(struct meminfo *__restrict prev,
                         struct meminfo *__restrict next) {
 assert((uintptr_t)prev->mi_full_addr+prev->mi_full_size <=
        (uintptr_t)next->mi_full_addr);
 if ((uintptr_t)prev->mi_part_addr+prev->mi_part_size >
     (uintptr_t)next->mi_part_addr) {
  assert((uintptr_t)next->mi_part_addr == (uintptr_t)next->mi_full_addr-PAGESIZE);
  assert((uintptr_t)next->mi_part_addr == (uintptr_t)prev->mi_full_addr+prev->mi_full_size-PAGESIZE);
  /* Figure out which of the regions to truncate. */
  if (next->mi_type == MEMTYPE_RAM) {
   /* Truncate 'next' */
   next->mi_size     -= (size_t)((uintptr_t)next->mi_full_addr-
                                 (uintptr_t)next->mi_addr);
   next->mi_addr      = next->mi_full_addr;
   next->mi_part_addr = next->mi_full_addr;
   next->mi_part_size = next->mi_full_size;
  } else {
   /* Truncate 'prev' */
   assert(prev->mi_size > (uintptr_t)next->mi_part_addr-
                          (uintptr_t)prev->mi_addr);
   prev->mi_size = (uintptr_t)next->mi_part_addr-
                   (uintptr_t)prev->mi_addr;
   meminfo_load_part_full(prev);
   assert((uintptr_t)prev->mi_part_addr+prev->mi_part_size <=
          (uintptr_t)next->mi_part_addr);
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
 syslog(LOG_MEM|LOG_WARN,
        FREESTR("[MEM] Overlapping <%s> address range with <%s> at %p...%p\n"),
        memtype_names[old_type],memtype_names[new_type],base,base+num_bytes-1);
}




INTERN ATTR_FREETEXT PAGE_ALIGNED size_t KCALL
mem_install_zone(PHYS uintptr_t base, size_t num_bytes,
                 memtype_t type, mzone_t zone) {
 struct meminfo *iter,**piter,*new_info,*prev;
 size_t result = 0; uintptr_t addr_end;
 assert(num_bytes);
 piter = &mem_info_[zone];
 /* Figure out the memory info record before which to insert this zone. */
 while ((iter = *piter) != MEMINFO_EARLY_NULL &&
        (PHYS uintptr_t)iter->mi_addr < base) {
  assertf(piter == &mem_info_[zone] ||
         (uintptr_t)container_of(piter,struct meminfo,mi_next)->mi_addr+
                    container_of(piter,struct meminfo,mi_next)->mi_size <=
         (uintptr_t)iter->mi_addr,
          "Illegal overlap: %p...%p %p...%p",
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
   /* Map memory above the overlap. */
   if (addr_end > iter_end)
       result += mem_install_zone(iter_end,addr_end-iter_end,type,zone);
   mem_overlapping_address_range((uintptr_t)iter->mi_addr,
                                  MIN(addr_end,iter_end)-base,
                                  iter->mi_type,type);
   addr_end  = (uintptr_t)iter->mi_addr;
   num_bytes =  addr_end-base;
   if unlikely(!num_bytes) return result; /* Full overlap */
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
   if (piter != &mem_info_[zone]) {
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
    }
   }

   /* Make the memory available if it is RAM. */
   if (MEMTYPE_ISUSE(type)) {
    result += page_addmemory(zone,new_full_addr,
                            (uintptr_t)old_full_addr-
                            (uintptr_t)new_full_addr);
   }
   return result;
  }
 }

 if (piter != &mem_info_[zone]) {
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
   if (MEMTYPE_ISUSE(type)) {
    uintptr_t aligned_base = CEIL_ALIGN(base,PAGESIZE);
    uintptr_t aligned_end  = CEIL_ALIGN(base+num_bytes,PAGESIZE)-aligned_base;
    result += page_addmemory(zone,(ppage_t)aligned_base,
                             aligned_base-aligned_end);
   }
   return result;
  }
 }


 /* Create and insert a new info record. */
 assert(num_bytes);
 new_info = meminfo_early_alloc();
 new_info->mi_next = iter;
 new_info->mi_type = type;
 new_info->mi_addr = (PHYS void *)base;
 new_info->mi_size = num_bytes;
 *piter            = new_info;
 meminfo_load_part_full(new_info);

 /* Fix overlaps. */
 if (type != MEMTYPE_PRESERVE) {
  if (piter != &mem_info_[zone])
      mem_resolve_part_overlap(container_of(piter,struct meminfo,mi_next),new_info);
  if (iter != MEMINFO_EARLY_NULL)
      mem_resolve_part_overlap(new_info,iter);
 }

 /* Make the memory available for use by the physical memory allocator. */
 if (MEMTYPE_ISUSE(type)) {
  result += page_addmemory(zone,
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
  mem_unusable_address_range(KERNEL_BASE,KERNEL_BASE-(base+num_bytes),type);
  num_bytes = KERNEL_BASE-base;
 }

 /* Step #5: Enumerate all zones that the range is apart of and install it in each. */
 for (zone = 0; zone < MZONE_COUNT; ++zone) {
  if (base >= MZONE_MIN(zone) &&
      base <= MZONE_MAX(zone)) {
   size_t zone_bytes = (MZONE_MAX(zone)+1)-base;
   if (zone_bytes > num_bytes)
       zone_bytes = num_bytes;
   result += mem_install_zone(base,zone_bytes,type,zone);
   num_bytes -= zone_bytes;
   if (!num_bytes) break;
  }
 }
 return result;
}



INTERN ATTR_FREETEXT
PAGE_ALIGNED size_t KCALL mem_unpreserve(void) {
 size_t result = 0; mzone_t zone;
 struct meminfo *iter,*prev,*next;
 for (zone = 0; zone < MZONE_COUNT; ++zone) {
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
 struct meminfo *new_info_vec[MZONE_COUNT];
 /* Allocate a chunk of continuous memory within
  * which we're going to re-build memory information. */
 new_info = (struct meminfo *)kmalloc(STATIC_MEMINFO_CNT*sizeof(struct meminfo),GFP_SHARED);
 if unlikely(!new_info) PANIC(FREESTR("Failed to allocate %Iu bytes for relocated meminfo"),
                              STATIC_MEMINFO_CNT*sizeof(struct meminfo));
 _mall_nofree(new_info); /* This one's never supposed to be freed. */

 for (zone = 0; zone < MZONE_COUNT; ++zone) {
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
        MZONE_COUNT*sizeof(struct meminfo *));

 /* Free any additional physical pages that were allocated for memory information. */
 meminfo_early_freeall();

#ifdef CONFIG_DEBUG
 /* Restore debug pre-initialization of the BOOTCPU IDLE stack. */
 memsetl(&__bootidlestack.s_kernused,KERNEL_DEBUG_MEMPAT_HOSTSTACK,
        (sizeof(size_t)+(STATIC_MEMINFO_CNT*sizeof(struct meminfo)))/4);
#endif /* CONFIG_DEBUG */
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

DECL_END
#endif

#endif /* !GUARD_KERNEL_MEMORY_INSTALL_C */
