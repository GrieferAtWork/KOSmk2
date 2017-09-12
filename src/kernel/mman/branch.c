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
#ifndef GUARD_KERNEL_MMAN_BRANCH_C
#define GUARD_KERNEL_MMAN_BRANCH_C 1
#define _KOS_SOURCE 1

#include "intern.h"
#include <assert.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kernel/mman.h>
#include <sys/syslog.h>
#include <sched/paging.h>
#include <sys/mman.h>

DECL_BEGIN

#if 1
#define MMAN_LOADONREAD_ADDR ((ppage_t)0)
#else
#define MMAN_LOADONREAD_ADDR ((ppage_t)(~PDIR_ATTR_MASK))
#endif

INTERN errno_t KCALL
mbranch_remap(struct mbranch const *__restrict self,
              pdir_t *__restrict pdir, bool update_prot) {
 struct mregion *region; errno_t error;
 CHECK_HOST_DOBJ(self);
 region = self->mb_region;
 error  = rwlock_read(&region->mr_plock);
 if (E_ISERR(error)) return error;
 error = mbranch_remap_unlocked(self,pdir,update_prot);
 rwlock_endread(&region->mr_plock);
 return error;
}
INTERN errno_t KCALL
mbranch_remap_unlocked(struct mbranch const *__restrict self,
                       pdir_t *__restrict pdir, bool update_prot) {
 struct mregion_part *part; struct mman *old_mman = NULL;
 struct mregion *region; errno_t error = -EOK;
 pdir_attr_t page_prot = PDIR_FLAG_NOFLUSH;
 raddr_t region_begin,region_end;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(pdir);
 CHECK_HOST_DOBJ(self->mb_region);
 region       = self->mb_region;
 assert(rwlock_reading(&region->mr_plock));
 region_begin = self->mb_start;
 region_end   = region_begin+MBRANCH_SIZE(self);
 assert(region_begin <  region_end);
 assertf(region_end  <= region->mr_size,
         "region_begin    = %p\n"
         "region_end      = %p\n"
         "region->mr_size = %p\n",
         region_begin,region_end,region->mr_size);
 assert(IS_ALIGNED(region_begin,PAGESIZE));
 assert(IS_ALIGNED(region_end,PAGESIZE));
 /* Don't touch page-directory mappings of reserved regions. */
 if unlikely(region->mr_type == MREGION_TYPE_RESERVED)
       goto end;
 if (!(self->mb_prot&PROT_NOUSER))
       page_prot |= PDIR_ATTR_USER;
 if (!(self->mb_prot&(PROT_WRITE|PROT_READ)))
       goto end; /* No access at all... */
 /* Must map for copy-on-write when write access is given, but sharing isn't allowed. */
 MREGION_FOREACH_PART(part,region) {
  raddr_t part_begin = part->mt_start,part_end;
  raddr_t using_begin,using_end;
  pdir_attr_t part_prot = 0;
  VIRT ppage_t part_vpage; size_t part_size;
  assert(IS_ALIGNED(part_begin,PAGESIZE));
  part_end = part->mt_chain.le_next ? part->mt_chain.le_next->mt_start : region->mr_size;
  assertf(part_end > part_begin,
          "part_begin = %Iu\n"
          "part_end   = %Iu\n",
          part_begin,part_end);
  assertf(IS_ALIGNED(part_begin,PAGESIZE),"Miss-aligned part begin %p",part_begin);
  assertf(IS_ALIGNED(part_end,PAGESIZE),"Miss-aligned part end %p",part_end);
  if (part_end <= region_begin) continue; /* Ignore parts below the mapped range. */
  if (part_begin >= region_end) break;    /* Stop when the part is above the mapped range. */
  /* NOTE: The part may not begin/end where the branch does,
   *       simple because while the branch does require
   *       its own unique part references, the parts may
   *       have been merged again later, when neighboring
   *       reference counters matched up. */
  using_begin = MAX(part_begin,region_begin)-region_begin;
  using_end   = MIN(part_end,region_end)-region_begin;
  assertf(part->mt_refcnt != 0,"How can this be zero? - We're using it!");
  assertf(using_end > using_begin,
          "part_begin   = %Iu\n"
          "part_end     = %Iu\n"
          "region_begin = %Iu\n"
          "region_end   = %Iu\n"
          "using_begin  = %Iu\n"
          "using_end    = %Iu\n",
          part_begin,part_end,
          region_begin,region_end,
          using_begin,using_end);
  part_prot  = page_prot;
  part_size  = (size_t)(using_end-using_begin);
  part_vpage = (VIRT ppage_t)(self->mb_node.a_vmin+
                              using_begin);
  if (!old_mman) TASK_PDIR_KERNEL_BEGIN(old_mman);
  if (part->mt_state == MPART_STATE_INCORE) {
   /* Only map in-core parts as present. */
   part_prot |= PDIR_ATTR_PRESENT;
   /* Map with write-access if that protection is required,
    * or when COW isn't used when 'PROT_SHARED' is set, or
    * when the part is only used by us! */
   if ((self->mb_prot&PROT_WRITE) &&
        /* real-only file-mappings are loaded into memory regions upon first write-access.
         * NOTE: This doesn't apply to regions mapped as shared, as those are not meant to be copied! */
      (!(region->mr_init&MREGION_INIT_READTHROUGH) || (self->mb_prot&PROT_SHARED)) &&
        /* For write-through regions, we use the first write-access to set the CHNG
         * flag, thus tracking which parts of memory-mapped files have changed. */
      (!(region->mr_init&MREGION_INIT_WRITETHROUGH) || (part->mt_flags&MPART_FLAG_CHNG)) &&
        /* Finally, only shared, or uniquely owned parts can be mapped as writable. */
      ((self->mb_prot&PROT_SHARED) || part->mt_refcnt == 1))
        part_prot |= PDIR_ATTR_WRITE;
#if 0 /* Due to lazy binding, this can't really be determined? */
   if (update_prot) {
    /* Simple case: Only needing to update permissions, we
     *              can use one call to 'pdir_mprotect'
     *              since we're allowed to assume that the
     *              mapping already exists, as well as being
     *              mapped correctly. */
    error = pdir_mprotect(pdir,part_vpage,part_size,part_prot);
   } else
#endif
   {
    struct mscatter *iter;
    /* Because the part itself may be splintered all
     * across the physical address space, we'll have
     * to iterate all the different scatter entries,
     * mapping each one individually to its correct
     * location. */
    iter = &part->mt_memory;
    do {
     assert(IS_ALIGNED((uintptr_t)iter->m_start,PAGESIZE));
     assert(IS_ALIGNED(iter->m_size,PAGESIZE));
     assertf(iter->m_size <= ((uintptr_t)(self->mb_node.a_vmin+using_end)-(uintptr_t)part_vpage),
             "Scatter entry too large (%p > %p)",iter->m_size,
            ((uintptr_t)(self->mb_node.a_vmin+using_end)-(uintptr_t)part_vpage));
     error = pdir_mmap(pdir,part_vpage,iter->m_size,
                       iter->m_start,part_prot);
     if (E_ISERR(error)) goto end; /* TODO: Delete completed mappings? */
     *(uintptr_t *)&part_vpage += iter->m_size;
    } while ((iter = iter->m_next) != NULL);
    assertf(part_vpage == (VIRT ppage_t)(self->mb_node.a_vmin+using_end),
            "Incorrect scatter size (%p != %p)\n",
            part_vpage,(VIRT ppage_t)(self->mb_node.a_vmin+using_end));
   }
  } else {
   /* Map the part as non-present to cause a PAGEFAULT
    * at the next access, which in turn will cause the
    * page to be allocated & filled with data.
    * That PAGEFAULT will then have to decide what to do:
    *   - Allocate missing parts (always)
    *   - Move memory from swap into the core (potentially).
    *   - Initialize newly allocated memory from fill/file (potentially).
    */
   assert(!(part_prot&PDIR_ATTR_PRESENT));
   error = pdir_mmap(pdir,part_vpage,part_size,
                     MMAN_LOADONREAD_ADDR,page_prot);
  }
  if (E_ISERR(error)) goto end;
 }
end:
 if (old_mman) TASK_PDIR_KERNEL_END(old_mman);
 return error;
}

INTERN errno_t KCALL
mbranch_unmap(struct mbranch const *__restrict self,
              pdir_t *__restrict pdir) {
 errno_t error;
 struct mman *old_mman;
 /* Don't touch page-directory mappings of reserved regions. */
 if unlikely(self->mb_region->mr_type == MREGION_TYPE_RESERVED)
    return -EOK;
 TASK_PDIR_KERNEL_BEGIN(old_mman);
 error = pdir_munmap(pdir,(ppage_t)self->mb_node.a_vmin,
                     MBRANCH_SIZE(self),PDIR_FLAG_NOFLUSH);
 TASK_PDIR_KERNEL_END(old_mman);
 return error;
}


DECL_END

#endif /* !GUARD_KERNEL_MMAN_BRANCH_C */
