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
#ifndef GUARD_KERNEL_MMAN_MMAN_C
#define GUARD_KERNEL_MMAN_MMAN_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include "../debug-config.h"
#include "intern.h"

#include <assert.h>
#include <dev/blkdev.h>
#include <fs/dentry.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/list/atree.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/paging-util.h>
#include <kernel/stack.h>
#include <sys/syslog.h>
#include <limits.h>
#include <linker/module.h>
#include <malloc.h>
#include <sched/paging.h>
#include <sched/task.h>
#include <stdalign.h>
#include <string.h>
#include <sys/mman.h>

/* Define the ABI for the address tree used by mman. */
#define ATREE(x) mbranch_tree_##x
#define Tkey     VIRT uintptr_t
#define T        struct mbranch
#define path     mb_node
#include <hybrid/list/atree-abi.h>

DECL_BEGIN

/* Global list of known memory regions (tracked for swap functionality)
 * WARNING: This list may contain ZERO-refcnt entries, so
 *         'MREGION_TRYINCREF()' must be used when accessing elements.
 * NOTE: To prevent expensive switching to/from mman_kernel,
 *       this list is split into regions allocated in shared
 *       memory, and those allocated in physical memory. */
PRIVATE VIRT LIST_HEAD(struct mregion) mregion_chain_v = NULL;
PRIVATE PHYS LIST_HEAD(struct mregion) mregion_chain_p = NULL;
PRIVATE DEFINE_ATOMIC_RWLOCK(mregion_chain_lock);


PUBLIC ssize_t KCALL
mman_swapmem(size_t max_swap, gfp_t flags) {
 size_t result = 0;
 if ((result += kmalloc_trim(max_swap)) >= max_swap) goto end;
 if ((result += dentry_clearcache_freemem()) >= max_swap) goto end;


end:
 if (result > SSIZE_MAX)
     result = SSIZE_MAX;
 return result;
}



PUBLIC void KCALL
mregion_destroy(struct mregion *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assertf(addr_isvirt(self) || PDIR_ISKPD(),
         "Non-virtual memory region may only be operated upon within KPD");
 assert(self->mr_refcnt == 0);
 if (self->mr_type == MREGION_TYPE_MEM) {
  if (self->mr_global.le_pself) {
   atomic_rwlock_write(&mregion_chain_lock);
   LIST_REMOVE(self,mr_global);
   atomic_rwlock_endwrite(&mregion_chain_lock);
  }
  if (MREGION_INIT_TYPE(self->mr_init) ==
      MREGION_INIT_TYPE(MREGION_INIT_USER)) {
   CHECK_HOST_TEXT(self->mr_setup.mri_ufunc,1);
   (*self->mr_setup.mri_ufunc)(MREGION_INITFUN_MODE_FINI,
                               self->mr_setup.mri_uclosure,
                               self,NULL,0,self->mr_size);
  }
 }
 /* NOTE: Futex pointers may still exist, as they have their own
  *       reference counters. So we must manually drop all that remain. */
 mfutexptr_clear(&self->mr_futex);

 /* Free all region parts still allocated. */
 { struct mregion_part *iter,*next;
   iter = self->mr_parts;
   while (iter) {
    next = iter->mt_chain.le_next;
    assert(iter->mt_start < self->mr_size);
    assert(!next || iter->mt_start < next->mt_start);
    assertf(iter->mt_refcnt == 0,"%d",iter->mt_refcnt);
    if (self->mr_type == MREGION_TYPE_MEM) {
     /* The part may not be in-core if the region is being destroyed after
      * having been pre-allocated before a call to mmap() fails. */
     if unlikely(iter->mt_state == MPART_STATE_INCORE ||
                 iter->mt_state == MPART_STATE_INSWAP) {
      struct mman *omm;
      TASK_PDIR_KERNEL_BEGIN(omm);
      if likely(iter->mt_state == MPART_STATE_INCORE)
           page_free_scatter(&iter->mt_memory,PAGEATTR_NONE);
      else mswap_delete(&iter->mt_stick);
      TASK_PDIR_KERNEL_END(omm);
     }
    } else {
     assertf(self->mr_type == MREGION_TYPE_PHYSICAL ? (iter->mt_state == MPART_STATE_INCORE) :
             MREGION_TYPE_ISGUARD(self->mr_type)    ? (iter->mt_state == MPART_STATE_MISSING) :
             1,"%d",iter->mt_state);
    }
    assertf(!(iter->mt_flags&MPART_FLAG_CHNG),"%x",iter->mt_flags);
    if (iter != &self->mr_part0) kffree(iter,GFP_NOFREQ);
    iter = next;
   }
 }
 if (MREGION_INIT_ISFILE(self->mr_init)) {
  CHECK_HOST_DOBJ(self->mr_setup.mri_file);
  FILE_DECREF(self->mr_setup.mri_file);
 }
 kffree(self,GFP_NOFREQ);
}

PUBLIC REF struct mregion *KCALL
mregion_new_phys(gfp_t region_gfp, PHYS ppage_t addr, PAGE_ALIGNED size_t n_bytes) {
 REF struct mregion *region;
 assert(IS_ALIGNED((uintptr_t)addr,PAGESIZE));
 assert(IS_ALIGNED(n_bytes,PAGESIZE));
 region = mregion_new(region_gfp);
 if unlikely(!region) return E_PTR(-ENOMEM);
 region->mr_size                    = n_bytes;
 region->mr_type                    = MREGION_TYPE_PHYSICAL;
 region->mr_part0.mt_state          = MPART_STATE_INCORE;
 region->mr_part0.mt_memory.m_start = addr;
 region->mr_part0.mt_memory.m_size  = n_bytes;
 region->mr_part0.mt_locked         = 1;
 mregion_setup(region);
 return region;
}

PUBLIC REF struct mregion *KCALL
mregion_new_anon(gfp_t region_gfp, PAGE_ALIGNED size_t n_bytes, u32 mode) {
 REF struct mregion *region;
 assert(IS_ALIGNED(n_bytes,PAGESIZE));
 region = mregion_new(region_gfp);
 if unlikely(!region) return E_PTR(-ENOMEM);
 region->mr_size            = n_bytes;
 region->mr_type            = MREGION_TYPE_MEM;
 region->mr_part0.mt_locked = !!(mode&MREGION_ANON_LOCKED);
#if MREGION_ANON_CALLOC == MREGION_INIT_ZERO
 region->mr_init = (mode&MREGION_ANON_CALLOC);
#else
 if (mode&MREGION_ANON_CALLOC)
     region->mr_init = MREGION_INIT_ZERO;
#endif
 if (mode&MREGION_ANON_PREFAULT) {
  /* Pre-fault all region pages. */
  if (!page_malloc_scatter(&region->mr_part0.mt_memory,n_bytes,
                            PAGESIZE,
#if MREGION_ANON_CALLOC == PAGEATTR_ZERO
                            mode&MREGION_ANON_CALLOC,
#else
                            mode&MREGION_ANON_CALLOC ? PAGEATTR_ZERO : 0,
#endif
                            MZONE_ANY)) {
   /* Failed to pre-fault the region. */
   if (!(mode&MREGION_ANON_TRY_PREFAULT))
         goto fail;
  } else {
   region->mr_part0.mt_state = MPART_STATE_INCORE;
  }
 }
 mregion_setup(region);
 return region;
fail:
 kfree(region);
 return E_PTR(-ENOMEM);
}

PUBLIC struct mregion *KCALL
mregion_cinit(struct mregion *self) {
 if (self) {
  assert(self->mr_refcnt                  == 0);
  assert(self->mr_type                    == MREGION_TYPE_MEM);
  assert(self->mr_init                    == MREGION_INIT_RAND);
  assert(self->mr_gfunds                  == 0);
  assert(self->mr_setup.mri_byte          == 0);
  assert(self->mr_setup.mri_file          == NULL);
  assert(self->mr_setup.mri_begin         == 0);
  assert(self->mr_setup.mri_start         == 0);
  assert(self->mr_setup.mri_size          == 0);
  assert(self->mr_size                    == 0);
  assert(self->mr_parts                   == NULL);
  assert(self->mr_global.le_pself         == NULL);
  assert(self->mr_global.le_next          == NULL);
  assert(self->mr_part0.mt_chain.le_pself == NULL);
  assert(self->mr_part0.mt_chain.le_next  == NULL);
  assert(self->mr_part0.mt_start          == 0);
  assert(self->mr_part0.mt_refcnt         == 0);
  assert(self->mr_part0.mt_state          == MPART_STATE_MISSING);
  assert(self->mr_part0.mt_flags          == MPART_FLAG_NONE);
  assert(self->mr_part0.mt_locked         == 0);
  self->mr_refcnt = 1;
  atomic_rwptr_cinit(&self->mr_futex);
  rwlock_cinit(&self->mr_plock);
  self->mr_parts                   = &self->mr_part0;
  self->mr_part0.mt_chain.le_pself = &self->mr_parts;
 }
 return self;
}

PUBLIC void KCALL
mregion_setup(struct mregion *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assertf(self->mr_global.le_pself == NULL,
         "Region %p was already set up",self);
 assertf(self->mr_size != 0,"Regions cannot be empty");
 assert(addr_isvirt(self) || PDIR_ISKPD());
 if (self->mr_type == MREGION_TYPE_MEM) {
  atomic_rwlock_write(&mregion_chain_lock);
  /* Similarly to when regions are destroyed,
   * we must make sure to access non-shared pointers
   * in the context of the kernel page directory. */
  if (addr_isvirt(self))
       LIST_INSERT(mregion_chain_v,self,mr_global);
  else LIST_INSERT(mregion_chain_p,self,mr_global);
  atomic_rwlock_endwrite(&mregion_chain_lock);
 }
}

PRIVATE void KCALL
mbranch_delete(struct mbranch *__restrict self) {
 struct mbranch *next;
again:
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->mb_region);
 MBRANCH_DECREF(self);
 if (self->mb_node.a_min) {
  if (self->mb_node.a_max)
      mbranch_delete(self->mb_node.a_max);
  next = self->mb_node.a_min;
 } else {
  next = self->mb_node.a_max;
 }
 /* Drop region references.
  * NOTE: Make sure we're not interrupted doing this. */
 task_nointr();
 mregion_decref(self->mb_region,self->mb_start,
                MBRANCH_SIZE(self));
 task_endnointr();
 kffree(self,GFP_NOFREQ);
 if (next) { self = next; goto again; }
}













PUBLIC SAFE struct mman *KCALL mman_new(void) {
 VIRT struct mman *result,*old_mm;
 ssize_t lock_error;
 assert(alignof(struct mman) >= PDIR_ALIGN);
 result = (struct mman *)kmemalign(MMAN_ALIGN,
                                   sizeof(struct mman),
                                   GFP_SHARED|GFP_CALLOC);
 if unlikely(!result) return NULL;
#if PDIR_ALIGN != PAGESIZE
#error FIXME
#endif
 assert(IS_ALIGNED((uintptr_t)result,PAGESIZE));
 /* Lock the data for the page directory, and load it into the core.
  * NOTE: We don't use 'GFP_LOCKED' for this, because on the first
  *       page needs to be locked for the page directory itself. */
 task_nointr();
 TASK_PDIR_KERNEL_BEGIN(old_mm);
 lock_error = mman_mlock(&mman_kernel,(ppage_t)&result->m_pdir,sizeof(result->m_pdir),
                          MMAN_MLOCK_LOCK|MMAN_MLOCK_INCORE|MMAN_MLOCK_WRITE);
 assert(lock_error != -EINTR);
 task_endnointr();
 if (E_ISERR(lock_error) ||
    !pdir_init(&result->m_pdir)) {
  kffree(result,GFP_NOFREQ);
  result = NULL;
  goto end;
 }
 result->m_refcnt = 1;
 /* Figure out where the physical page directory is located at. */
 result->m_ppdir = (PHYS PAGE_ALIGNED pdir_t *)PDIR_TRANSLATE(&pdir_kernel_v,&result->m_pdir);
 assert(IS_ALIGNED((uintptr_t)result->m_ppdir,PDIR_ALIGN));
 assertf(!result->m_map,"%p: %p",result,result->m_map);
 owner_rwlock_cinit(&result->m_lock);
 result->m_uheap = MMAN_UHEAP_DEFAULT_ADDR;
 result->m_ustck = MMAN_USTCK_DEFAULT_ADDR;
 /* Start out with an empty LDT. */
 LDT_INCREF(&ldt_empty);
 result->m_ldt = &ldt_empty;
end:
 TASK_PDIR_KERNEL_END(old_mm);
 return result;
}

PUBLIC SAFE void KCALL
mman_destroy(struct mman *__restrict self) {
 struct mman *old_mman;
 CHECK_HOST_DOBJ(self);
 assert(self != THIS_MMAN);
 assert(self->m_refcnt == 0);
 assertf(addr_isvirt(self),
         "Memory managers must be allocated as shared memory");
 assertf(self != &mman_kernel,
         "The kernel mman must never be destroyed!");
 TASK_PDIR_KERNEL_BEGIN(old_mman);
 assertf(self != old_mman,
         "You can't destroy your own memory manager! "
         "Switch to another first");
 if (self->m_map) mbranch_delete(self->m_map);
 pdir_fini(&self->m_pdir);
#if PDIR_ALIGN != PAGESIZE || (PDIR_SIZE % PAGESIZE) != 0
#error FIXME
#endif
 /* Unlock all memory associated with the page directory. */
 { ssize_t error;
   task_nointr();
   mman_write(&mman_kernel);
   error = mman_mlock_unlocked(&mman_kernel,(ppage_t)&self->m_pdir,
                                sizeof(self->m_pdir),MMAN_MLOCK_UNLOCK);
   mman_endwrite(&mman_kernel);
   task_endnointr();
   if (E_ISERR(error)) {
    syslog(LOG_MEM|LOG_ERROR,
           "[MMAN] Failed to unlock virtual page directory %p...%p: %[errno]\n",
          (ppage_t)&self->m_pdir,sizeof(self->m_pdir),(errno_t)-error);
   }
 }
 LDT_DECREF(self->m_ldt);
 kfree(self);
 TASK_PDIR_KERNEL_END(old_mman);
}

#ifdef CONFIG_DEBUG
/* Assert the internal consistency of the memory manager. */
PUBLIC SAFE void KCALL
mman_assert_branch(struct mman *__restrict self,
                   struct mbranch *__restrict branch) {
again:
 CHECK_HOST_DOBJ(branch);
 CHECK_HOST_DOBJ(branch->mb_region);
 assert(branch->mb_region->mr_refcnt >= 1);
 assert(MBRANCH_MIN(branch) < MBRANCH_MAX(branch));
 assert(branch->mb_start < branch->mb_region->mr_size);
 assert(IS_ALIGNED(MBRANCH_BEGIN(branch),PAGESIZE));
 assert(IS_ALIGNED(MBRANCH_SIZE(branch),PAGESIZE));
 /* XXX: Validate more stuff? */
 if (branch->mb_notify) {
  (*branch->mb_notify)(MNOTIFY_ASSERT,branch->mb_closure,self,
                      (ppage_t)MBRANCH_BEGIN(branch),MBRANCH_SIZE(branch));
 }
 if (branch->mb_node.a_min) {
  if (branch->mb_node.a_max)
      mman_assert_branch(self,branch->mb_node.a_max);
  branch = branch->mb_node.a_min;
  goto again;
 }
 if (branch->mb_node.a_max) {
  branch = branch->mb_node.a_max;
  goto again;
 }
}
PUBLIC SAFE void KCALL
mman_assert_unlocked(struct mman *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(mman_reading(self));
 assertf(addr_isvirt(self),"Memory managers must exist in virtual memory");
 assertf((self->m_map != NULL) == (self->m_order != NULL),
         "Broken map <---> order associativity");
 if (self->m_map) mman_assert_branch(self,self->m_map);
}
PUBLIC void KCALL
mman_assert(struct mman *__restrict self) {
 task_crit();
 mman_read(self);
 mman_assert_unlocked(self);
 mman_endread(self);
 task_endcrit();
}
#endif



PUBLIC struct instance *KCALL
mman_instance_at_unlocked(struct mman const *__restrict self,
                          USER void *addr) {
 struct mbranch *branch;
 CHECK_HOST_DOBJ(self);
 assert(mman_reading(self));
 branch = mbranch_tree_locate(self->m_map,(uintptr_t)addr);
 return branch && MBRANCH_ISINSTANCE(branch)
      ? MBRANCH_GETINSTANCE(branch) : NULL;
}

PUBLIC struct instance *KCALL
mman_instance_of_unlocked(struct mman const *__restrict self,
                          struct module *__restrict mod) {
 struct instance *inst;
 CHECK_HOST_DOBJ(self);
 assert(mman_reading(self));
 MMAN_FOREACH_INST(inst,self) {
  if (inst->i_module == mod &&
      INSTANCE_INCREF(inst))
      return inst;
 }
 return NULL;
}



PUBLIC void KCALL
mman_insbranch_unlocked(struct mman *__restrict self,
                        struct mbranch *__restrict branch) {
 struct mbranch **pinsert,*insert;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(branch);
 CHECK_HOST_DOBJ(branch->mb_region);
 CHECK_HOST_DOBJ(branch->mb_region->mr_parts);
 assert(branch->mb_region->mr_parts->mt_chain.le_pself ==
       &branch->mb_region->mr_parts);
 assertf(IS_ALIGNED(MBRANCH_BEGIN(branch),PAGESIZE),
         "Miss-aligned branch begin %p\n",MBRANCH_BEGIN(branch));
 assertf(IS_ALIGNED(MBRANCH_END(branch),PAGESIZE),
         "Miss-aligned branch end %p\n",MBRANCH_END(branch));
 assertf(MBRANCH_MIN(branch) < MBRANCH_MAX(branch),
         "Unordered branch: MIN(%p) >= MAX(%p)",
         MBRANCH_MIN(branch),MBRANCH_MAX(branch));
 assertf(!mman_inuse_unlocked(self,
                             (VIRT void *)MBRANCH_BEGIN(branch),
                              MBRANCH_SIZE(branch)),
         "Some part of the branch's memory range %p...%p is already in use",
         MBRANCH_MIN(branch),MBRANCH_MAX(branch));
 assert(branch->mb_start < branch->mb_region->mr_size);
#ifdef CONFIG_DEBUG
 if (branch->mb_notify) CHECK_HOST_TEXT(branch->mb_notify,1);
#endif
 mbranch_tree_insert(&self->m_map,branch);
 /* Figure out where we need to insert the branch. */
 pinsert = &self->m_order;
 while ((insert = *pinsert) != NULL) {
  if (insert->mb_node.a_vmin > branch->mb_node.a_vmax) break;
  pinsert = &insert->mb_order.le_next;
 }
 /* Insert the branch before 'insert' at 'pinsert' */
 branch->mb_order.le_pself = pinsert;
 branch->mb_order.le_next  = insert;
 if (insert) insert->mb_order.le_pself = &branch->mb_order.le_next;
 *pinsert = branch;
}

PUBLIC errno_t KCALL
mman_insbranch_map_unlocked(struct mman *__restrict self,
                            struct mbranch *__restrict branch) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(branch);
 error = mbranch_remap(branch,&self->m_pdir,false);
 if (E_ISOK(error)) mman_insbranch_unlocked(self,branch);
 return error;
}

PUBLIC struct mbranch *KCALL
mman_getbranch_unlocked(struct mman const *__restrict self, VIRT void *addr) {
 CHECK_HOST_DOBJ(self);
 return mbranch_tree_locate(self->m_map,(uintptr_t)addr);
}



PUBLIC u8 KCALL
mman_getstate_unlocked(struct mman const *__restrict self,
                       VIRT void *addr) {
 struct mbranch *branch; raddr_t region_addr;
 struct mregion *region; struct mregion_part *part;
 u8 result;
 CHECK_HOST_DOBJ(self);
 assert(mman_reading(self));
 branch = mbranch_tree_locate(self->m_map,
                             (VIRT uintptr_t)addr);
 if unlikely(!branch) return MPART_STATE_MISSING;
 CHECK_HOST_DOBJ(branch);
 region = branch->mb_region;
 CHECK_HOST_DOBJ(region);
 assert((uintptr_t)addr >= MBRANCH_MIN(branch));
 assert((uintptr_t)addr <= MBRANCH_MAX(branch));
 region_addr = ((uintptr_t)addr-MBRANCH_BEGIN(branch))+branch->mb_start;
 assert(region_addr <= region->mr_size);
 task_nointr();
 rwlock_read(&region->mr_plock);
 /* Find the part responsible for the given address. */
 part = region->mr_parts;
 for (;; part = part->mt_chain.le_next) {
  raddr_t part_end;
  assert(part);
  CHECK_HOST_DOBJ(part);
  part_end = MREGION_PART_END(part,region);
  if (part_end > region_addr) break;
 }
 result = part->mt_state;
 rwlock_endread(&region->mr_plock);
 task_endnointr();
 return result;
}

PUBLIC REF struct mfutex *KCALL
mman_getfutex_unlocked(struct mman *__restrict self, VIRT void *addr) {
 struct mbranch *branch;
 CHECK_HOST_DOBJ(self);
 assert(mman_writing(self));
 branch = mbranch_tree_locate(self->m_map,
                             (VIRT uintptr_t)addr);
 if unlikely(!branch) return E_PTR(-EFAULT);
 CHECK_HOST_DOBJ(branch->mb_region);
 return mfutexptr_get(&branch->mb_region->mr_futex,
                       MBRANCH_RADDR(branch,addr));
}
PUBLIC REF struct mfutex *KCALL
mman_newfutex_unlocked(struct mman *__restrict self, VIRT void *addr) {
 struct mbranch *branch;
 CHECK_HOST_DOBJ(self);
 assert(mman_writing(self));
 branch = mbranch_tree_locate(self->m_map,
                             (VIRT uintptr_t)addr);
 if unlikely(!branch) return E_PTR(-EFAULT);
 CHECK_HOST_DOBJ(branch->mb_region);
 return mfutexptr_new(&branch->mb_region->mr_futex,
                       MBRANCH_RADDR(branch,addr));
}


FUNDEF errno_t KCALL
mman_mmap_instance_unlocked(struct mman *__restrict self,
                            struct instance *__restrict inst,
                            u32 prot_amask, u32 prot_omask) {
 struct module *mod;
 struct modseg *iter,*end;
 uintptr_t base_addr;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(inst);
 assert(mman_writing(self));
 /* Create the instance reference used below. */
 if (!INSTANCE_INCREF(inst))
      return -EPERM;
 mod = inst->i_module;
 CHECK_HOST_DOBJ(mod);
 assert(inst->i_refcnt >= 1);
 assert(mod->m_refcnt);
 assertf(!inst->i_branch,"The given instance has already been mapped.");
 assertf(!mod->m_size || mod->m_segc,"Module %.?q with a size of %Iu, but not segments?",
         mod->m_name->dn_size,mod->m_name->dn_name,mod->m_size);
 base_addr = (uintptr_t)inst->i_base;
 assert(base_addr+mod->m_size >= base_addr);
 assertf(IS_ALIGNED(base_addr,PAGESIZE),"Invalid base: %p",base_addr);
 /* Start mapping! */
 end = (iter = mod->m_segv)+mod->m_segc;
#ifdef CONFIG_DEBUG
 /* Prevent 'MNOTIFY_INCREF' from causing an
  * assertion failure because 'i_branch' was zero.
  * >> This is the one time it's allowed to be! */
 inst->i_branch = 1;
#endif
 for (; iter != end; ++iter) {
  errno_t error;
  assert(iter->ms_msize);
  assert(iter->ms_region);
  assert(iter->ms_region->mr_size >= iter->ms_msize);
  error = mman_mmap_unlocked(self,
                            (ppage_t)((uintptr_t)base_addr+FLOOR_ALIGN(iter->ms_paddr,PAGESIZE)),
                             iter->ms_msize,0,iter->ms_region,
                            (iter->ms_prot&prot_amask)|prot_omask,
                            &instance_mnotify,inst);
  if (E_ISERR(error)) {
   while (iter-- != mod->m_segv) {
    mman_munmap_unlocked(self,
                        (ppage_t)((uintptr_t)base_addr+FLOOR_ALIGN(iter->ms_paddr,PAGESIZE)),
                         iter->ms_msize,MMAN_MUNMAP_TAG,inst);
   }
#ifdef CONFIG_DEBUG
   assert(inst->i_branch == 1);
   inst->i_branch = 0;
#endif
   INSTANCE_DECREF(inst);
   return error;
  }
 }
#ifdef CONFIG_DEBUG
 --inst->i_branch;
#endif
 /* NOTE: There may be less branches if some mapping were overlapping. */
 assertf(inst->i_branch <= mod->m_segc,
         "inst->i_branch = %Iu\n"
         "mod->m_segc    = %Iu\n",
         inst->i_branch,mod->m_segc);
 if (inst->i_branch) {
  struct instance **piter,*iter;
  /* Insert the instance into the sorted chain of loaded modules. */
  piter = &self->m_inst;
  while ((iter = *piter) != NULL &&
         (uintptr_t)iter->i_base < base_addr)
          piter = &iter->i_chain.le_next;
  assert(!iter || iter->i_chain.le_pself == piter);
  inst->i_chain.le_pself = piter;
  inst->i_chain.le_next  = iter;
  if (iter) iter->i_chain.le_pself = &inst->i_chain.le_next;
  *piter = inst; /* Inherit reference (create at the start of this function) */
 }
 return -EOK;
}


PUBLIC errno_t KCALL
mman_mmap_stack_unlocked(struct mman *__restrict self,
                         struct stack *__restrict stck,
                         u32 prot, u8 type, u16 funds,
                         size_t guard_size,
                         struct mregion *stack_region) {
 size_t stack_size; errno_t error;
 ppage_t stack_addr;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(stck);
 assert(mman_writing(self));
 assert(stck->s_refcnt >= 1);
 stack_addr = stck->s_begin;
 assert(IS_ALIGNED((uintptr_t)stack_addr,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)stck->s_end,PAGESIZE));
 assert(stack_addr <= stck->s_end);
 stack_size = ((uintptr_t)stck->s_end-(uintptr_t)stack_addr);
 /* Check for special case: Empty stack. */
 if unlikely(!stack_size) return -EOK;
 assert(stack_size >= PAGESIZE);
 /* Fix + clamp the size of the guard. */
 guard_size = CEIL_ALIGN(guard_size,PAGESIZE);
 if (guard_size > stack_size-PAGESIZE)
     guard_size = stack_size-PAGESIZE;
 /* Force an empty guard size when  */
 if (!MREGION_TYPE_ISGUARD(type)) guard_size = 0;
 stack_size -= guard_size;
 assert(stack_size >= PAGESIZE);

 if (stack_region) {
  CHECK_HOST_DOBJ(stack_region);
  assert(stack_region->mr_size == stack_size);
  MREGION_INCREF(stack_region);
 } else {
  /* Create a new descriptor for the stack region. */
  stack_region = mregion_new(MMAN_DATAGFP(self));
  if unlikely(!stack_region) return -ENOMEM;
  stack_region->mr_size           = stack_size;
#if KERNEL_DEBUG_MEMPAT_USERSTACK != 0
  stack_region->mr_init           = MREGION_INIT_BYTE;
  stack_region->mr_setup.mri_byte = KERNEL_DEBUG_MEMPAT_USERSTACK;
#else
  stack_region->mr_init           = MREGION_INIT_ZERO;
#endif
  mregion_setup(stack_region);
 }
#define GUARD_ADDR  (type == MREGION_TYPE_LOGUARD ? stck->s_begin : \
                    (ppage_t)((uintptr_t)stck->s_end-guard_size))

 if (!stck->s_branch) STACK_INCREF(stck);
#ifdef CONFIG_DEBUG
 ++stck->s_branch;
#endif

 if (guard_size) {
  /* Create + map the guard region. */
  struct mregion *guard_region;
  guard_region = mregion_new(GFP_NOFREQ|MMAN_DATAGFP(self));
  if unlikely(!guard_region) {
   error = -ENOMEM;
guard_error:
#ifdef CONFIG_DEBUG
   assert(stck->s_branch != 0);
   --stck->s_branch;
#endif
   MREGION_DECREF(stack_region);
   goto err_stck;
  }
  /* Initialize the guard region. */
  guard_region->mr_size   = stack_size;
  guard_region->mr_type   = type;
  guard_region->mr_gfunds = funds;
  if (stack_region->mr_init == MREGION_INIT_BYTE ||
      MREGION_INIT_ISFILE(stack_region->mr_init)) {
   guard_region->mr_init           = MREGION_INIT_BYTE;
   guard_region->mr_setup.mri_byte = stack_region->mr_setup.mri_byte;
  } else if (stack_region->mr_init == MREGION_INIT_RAND) {
   guard_region->mr_init = MREGION_INIT_RAND;
  } else {
   stack_region->mr_init = MREGION_INIT_ZERO;
  }
  mregion_setup(guard_region);
  error = mman_mmap_unlocked(self,GUARD_ADDR,guard_size,0,guard_region,
                             prot,&stack_mnotify,stck);
  MREGION_DECREF(guard_region);
  if (E_ISERR(error)) goto guard_error;

  /* Make sure to update the proper stack
   * address to point after the guard portion. */
  if (type == MREGION_TYPE_LOGUARD)
    *(uintptr_t *)&stack_addr += guard_size;
 }

 /* Map the stack itself. */
 error = mman_mmap_unlocked(self,stack_addr,stack_size,0,stack_region,
                            prot,&stack_mnotify,stck);
 MREGION_DECREF(stack_region);
#ifdef CONFIG_DEBUG
 assert(stck->s_branch != 0);
 --stck->s_branch;
#endif
 assert(E_ISERR(error) || stck->s_branch != 0);
 if (E_ISERR(error)) {
  /* Unmap the guard again. */
  if (guard_size)
      mman_munmap_unlocked(self,GUARD_ADDR,guard_size,
                           MMAN_MUNMAP_TAG,stck);
err_stck:
  if (!stck->s_branch) asserte(ATOMIC_FETCHDEC(stck->s_refcnt) >= 2);
 }
 return error;
}




PRIVATE ssize_t KCALL
mman_mprotect_impl(struct mman *__restrict self,
                   struct mbranch *__restrict branch,
                   PAGE_ALIGNED VIRT uintptr_t addr_min,
                                VIRT uintptr_t addr_max,
                   ATREE_SEMI_T(VIRT uintptr_t) addr_semi,
                   ATREE_LEVEL_T addr_level,
                   u32 prot_amask, u32 prot_omask) {
 ssize_t temp,result = 0;
again:
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(branch);
 assert(IS_ALIGNED(addr_min,PAGESIZE));
 assert(IS_ALIGNED(addr_max+1,PAGESIZE));
 assert(addr_min <= addr_max);
 if (addr_min <= branch->mb_node.a_vmax &&
     addr_max >= branch->mb_node.a_vmin) {
  /* Found a matching entry!
   * NOTE: Since the caller already split branches
   *       near borders, we are allowed to simply
   *       update this entire branch! */
  branch->mb_prot &= prot_amask;
  branch->mb_prot |= prot_omask;
  temp = mbranch_remap(branch,&self->m_pdir,true);
  if (E_ISERR(temp)) return temp;
  result += MBRANCH_SIZE(branch);
 }

 { bool walk_min,walk_max;
   walk_min = addr_min <  addr_semi && branch->mb_node.a_min;
   walk_max = addr_max >= addr_semi && branch->mb_node.a_max;
   if (walk_min) {
    /* Recursively continue searching left. */
    if (walk_max) {
     temp = mman_mprotect_impl(self,branch->mb_node.a_max,addr_min,addr_max,
                               ATREE_NEXTMAX(VIRT uintptr_t,addr_semi,addr_level),
                               ATREE_NEXTLEVEL(addr_level),prot_amask,prot_omask);
     if (E_ISERR(temp)) return temp;
     result += temp;
    }
    ATREE_WALKMIN(VIRT uintptr_t,addr_semi,addr_level);
    branch = branch->mb_node.a_min;
    goto again;
   } else if (walk_max) {
    /* Recursively continue searching right. */
    ATREE_WALKMAX(VIRT uintptr_t,addr_semi,addr_level);
    branch = branch->mb_node.a_max;
    goto again;
   }
 }
 return result;
}


PUBLIC ssize_t KCALL
mman_mprotect_unlocked(struct mman *__restrict self, VIRT ppage_t addr,
                       size_t n_bytes, u32 prot_amask, u32 prot_omask) {
 errno_t error; ssize_t result;
 CHECK_HOST_DOBJ(self);
 if unlikely(!n_bytes) return 0;
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 if ((error = mman_split_branch_unlocked(self,(VIRT uintptr_t)addr),E_ISERR(error)) ||
     (error = mman_split_branch_unlocked(self,(VIRT uintptr_t)addr+n_bytes),E_ISERR(error)))
      return error;
 if (!self->m_map) return 0;
 /* Recursively update protection mappings. */
 result = mman_mprotect_impl(self,self->m_map,
                            (VIRT uintptr_t)addr,
                            (VIRT uintptr_t)addr+n_bytes-1,
                             ATREE_SEMI0(VIRT uintptr_t),
                             ATREE_LEVEL0(VIRT uintptr_t),
                             prot_amask,prot_omask);
 if (E_ISOK(result)) {
  /* Try to merge adjacent leafs. */
  mman_merge_branch_unlocked(self,(uintptr_t)addr);
  mman_merge_branch_unlocked(self,(uintptr_t)addr+n_bytes);
 }
 return result;
}


struct branch_info {
 VIRT uintptr_t  bi_addr_min;
 VIRT uintptr_t  bi_addr_max;
 struct mbranch *bi_min;     /*< [0..1] Lowest branch. */
 struct mbranch *bi_max;     /*< [0..1] Greatest branch. */
 uintptr_t       bi_min_min; /*< == MBRANCH_MIN(bi_min). */
 uintptr_t       bi_max_max; /*< == MBRANCH_MAX(bi_max). */
};

PRIVATE void KCALL
mman_findbranches(struct branch_info *__restrict info,
                  struct mbranch *__restrict branch,
                  ATREE_SEMI_T(VIRT uintptr_t) addr_semi,
                  ATREE_LEVEL_T addr_level) {
again:
 CHECK_HOST_DOBJ(branch);
 if (info->bi_addr_min <= branch->mb_node.a_vmax &&
     info->bi_addr_max >= branch->mb_node.a_vmin) {
  /* Found a matching entry!
   * NOTE: Since the caller already split branches
   *       near borders, we are allowed to simply
   *       update this entire branch! */
  if (!info->bi_min || branch->mb_node.a_vmin < info->bi_min_min) {
   info->bi_min     = branch;
   info->bi_min_min = branch->mb_node.a_vmin;
  }
  if (!info->bi_max || branch->mb_node.a_vmax < info->bi_max_max) {
   info->bi_max     = branch;
   info->bi_max_max = branch->mb_node.a_vmax;
  }
 }

 { bool walk_min,walk_max;
   walk_min = info->bi_addr_min <  addr_semi && branch->mb_node.a_min;
   walk_max = info->bi_addr_max >= addr_semi && branch->mb_node.a_max;
   if (walk_min) {
    /* Recursively continue searching left. */
    if (walk_max) {
     mman_findbranches(info,branch->mb_node.a_max,
                       ATREE_NEXTMAX(VIRT uintptr_t,addr_semi,addr_level),
                       ATREE_NEXTLEVEL(addr_level));
    }
    ATREE_WALKMIN(VIRT uintptr_t,addr_semi,addr_level);
    branch = branch->mb_node.a_min;
    goto again;
   } else if (walk_max) {
    /* Recursively continue searching right. */
    ATREE_WALKMAX(VIRT uintptr_t,addr_semi,addr_level);
    branch = branch->mb_node.a_max;
    goto again;
   }
 }
}


PUBLIC VIRT ppage_t KCALL
mman_findspace_unlocked(struct mman *__restrict self,
                        VIRT ppage_t hint, size_t n_bytes,
                        size_t alignment, size_t pad_size,
                        u32 mode) {
 uintptr_t candidate;
 struct branch_info info;
 CHECK_HOST_DOBJ(self);
 assert(mman_reading(self));
 assertf(IS_ALIGNED((uintptr_t)hint,PAGESIZE),"Invalid hint: %p",hint);
 assertf(!(alignment&(alignment-1)),"Invalid alignment: %p",alignment);

 /* Check for special cases: Empty range/empty map. */
 if unlikely(!n_bytes || !self->m_map) return hint;
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
again:
 /* Step #1: Find the branch (if any) for the given hint.
  *          >> If no such branch exists, we can simply
  *             return the hint for perfect behavior! */
 info.bi_addr_min = (uintptr_t)hint;
 info.bi_addr_max = (uintptr_t)hint+n_bytes-1;
 if unlikely(info.bi_addr_max < info.bi_addr_min) {
  /* Special case: Fix an overflowing hint. */
  hint = (ppage_t)(0-n_bytes);
  goto again;
 }
 info.bi_min      = NULL;
 info.bi_max      = NULL;
 mman_findbranches(&info,self->m_map,
                   ATREE_SEMI0(VIRT uintptr_t),
                   ATREE_LEVEL0(VIRT uintptr_t));
 assert((info.bi_min != NULL) ==
        (info.bi_max != NULL));
 /* Simple case: No branch exists. */
 if (!info.bi_min) return hint;
 assert(info.bi_min_min < ((uintptr_t)hint+n_bytes-1));
 assert(info.bi_max_max >  (uintptr_t)hint);
 /* Step #2: based on 'mode&MMAN_FINDSPACE_BELOW', search from info.bi_min/info.bi_max
  *          upwards/downwards until a free memory range of sufficient
  *          size was found, using the ordered branch chain. */
 if (mode&MMAN_FINDSPACE_BELOW) {
  struct mbranch *iter;
  iter = info.bi_min;
  for (;;) {
   uintptr_t prev_end = 0;
   bool has_prev;
   CHECK_HOST_DOBJ(iter);
   has_prev = MBRANCH_HASPREV(iter,self);
   if (has_prev) {
    struct mbranch *prev_branch = MBRANCH_PREV(iter);
    prev_end = MBRANCH_END(prev_branch);
    /* Enforce padding before the next branch. */
    if (mode&MMAN_FINDSPACE_FORCEGAP ||
        prev_branch->mb_region->mr_type == MREGION_TYPE_HIGUARD)
        prev_end += pad_size;
   }
   candidate = MBRANCH_BEGIN(iter);
   if (mode&MMAN_FINDSPACE_FORCEGAP ||
       iter->mb_region->mr_type == MREGION_TYPE_HIGUARD)
       candidate -= pad_size;
   if (candidate > MBRANCH_BEGIN(iter)) break; /* Stop on overflow. */
   if ((candidate -= n_bytes) > MBRANCH_BEGIN(iter)) break; /* Stop on overflow. */
   candidate = FLOOR_ALIGN(candidate,alignment);
   if (candidate >= prev_end) goto winner;
   if (!has_prev) break;
   iter = MBRANCH_PREV(iter);
  }
 } else {
  struct mbranch *iter;
  iter = info.bi_max;
  for (;;) {
   uintptr_t next_begin = 0;
   bool has_next;
   CHECK_HOST_DOBJ(iter);
   has_next = MBRANCH_HASNEXT(iter,self);
   if (has_next) {
    struct mbranch *next_branch = MBRANCH_NEXT(iter);
    next_begin = MBRANCH_BEGIN(next_branch);
    /* Enforce padding before the next branch. */
    if (mode&MMAN_FINDSPACE_FORCEGAP ||
        next_branch->mb_region->mr_type == MREGION_TYPE_LOGUARD)
        next_begin -= pad_size;
   }
   candidate = MBRANCH_END(iter);
   if (mode&MMAN_FINDSPACE_FORCEGAP ||
       iter->mb_region->mr_type == MREGION_TYPE_HIGUARD)
       candidate += pad_size;
   candidate = CEIL_ALIGN(candidate,alignment);
   next_begin -= candidate;
   if (n_bytes <= next_begin) goto winner;
   if (!has_next) break;
   iter = MBRANCH_NEXT(iter);
  }
 }
 return PAGE_ERROR;
winner:
 return (ppage_t)candidate;
}


PUBLIC errno_t KCALL
mman_mmap_unlocked(struct mman *__restrict self, VIRT ppage_t addr,
                   size_t n_bytes, raddr_t start,
                   struct mregion *__restrict region, u32 prot,
                   mbranch_notity notify, void *closure) {
 ssize_t error;
 struct mbranch *newleaf;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(region);
 CHECK_HOST_DOBJ(region->mr_parts);
 assert(ATOMIC_READ(region->mr_refcnt));
 assert(IS_ALIGNED((VIRT uintptr_t)start,PAGESIZE));
 assertf(MMAN_DATAOK(self,region),"%p",region);
 assert(region->mr_parts->mt_chain.le_pself == &region->mr_parts);
 if unlikely(!n_bytes) return -EOK;
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 assert(region->mr_size);
 assert(start+n_bytes > start);
 assertf(start+n_bytes <= region->mr_size,
         "Too large mapping (%p > %p)",
         start+n_bytes,region->mr_size);
 newleaf = (struct mbranch *)kmalloc(sizeof(struct mbranch),
                                     GFP_NOFREQ|MMAN_DATAGFP(self));
 if (self == &mman_kernel) (void)_mall_untrack(newleaf);
 if unlikely(!newleaf) goto enomem;
 assert(MMAN_DATAOK(self,newleaf));
 /* Initialize the new leaf. */
 newleaf->mb_node.a_vmin = (VIRT uintptr_t)addr;
 newleaf->mb_node.a_vmax = (VIRT uintptr_t)addr+n_bytes-1;
 newleaf->mb_prot        = prot;
 newleaf->mb_start       = start;
 newleaf->mb_region      = region;
 newleaf->mb_notify      = notify;
 newleaf->mb_closure     = closure;
 error = mregion_incref(region,start,n_bytes);
 if (E_ISERR(error)) goto err1;
 /* Signal the INCREF to the given notifier. */
 if (notify &&
    (error = (errno_t)(*notify)(MNOTIFY_INCREF,closure,self,0,0),
     E_ISERR(error))) goto err2;
 /* Make sure that no branch exists for the mapping area. */
 error = mman_munmap_unlocked(self,addr,n_bytes,MMAN_MUNMAP_ALL,NULL);
 if (E_ISERR(error)) goto err3;
 /* Actually map the new leaf. */
 error = mbranch_remap(newleaf,&self->m_pdir,false);
 if (E_ISERR(error)) goto err3;
 /* Insert the new leaf. */
 mman_insbranch_unlocked(self,newleaf);
 /* Try to merge adjacent leafs. */
 mman_merge_branch_unlocked(self,(uintptr_t)addr);
 mman_merge_branch_unlocked(self,(uintptr_t)addr+n_bytes);
 return error;
err3: if (notify) (*notify)(MNOTIFY_DECREF,closure,NULL,0,0);
err2: mregion_decref(region,start,n_bytes);
err1: kffree(newleaf,GFP_NOFREQ);
err0: return error;
enomem: error = -ENOMEM; goto err0;
}



PRIVATE PHYS struct mbranch *KCALL
mman_findany(PHYS struct mbranch *__restrict branch,
             PAGE_ALIGNED uintptr_t addr_min,
                          uintptr_t addr_max,
             ATREE_SEMI_T(VIRT uintptr_t) addr_semi,
             ATREE_LEVEL_T addr_level) {
 PHYS struct mbranch *result;
again:
 CHECK_HOST_DOBJ(branch);
 assert(branch != branch->mb_node.a_min);
 assert(branch != branch->mb_node.a_max);
 if (addr_min <= branch->mb_node.a_vmax &&
     addr_max >= branch->mb_node.a_vmin)
     return branch;

 { bool walk_min,walk_max;
   walk_min = addr_min <  addr_semi && branch->mb_node.a_min;
   walk_max = addr_max >= addr_semi && branch->mb_node.a_max;
   if (walk_min) {
    /* Recursively continue searching left. */
    if (walk_max) {
     result = mman_findany(branch->mb_node.a_max,addr_min,addr_max,
                           ATREE_NEXTMAX(VIRT uintptr_t,addr_semi,addr_level),
                           ATREE_NEXTLEVEL(addr_level));
     if (result) return result;
    }
    ATREE_WALKMIN(VIRT uintptr_t,addr_semi,addr_level);
    branch = branch->mb_node.a_min;
    goto again;
   } else if (walk_max) {
    /* Recursively continue searching right. */
    ATREE_WALKMAX(VIRT uintptr_t,addr_semi,addr_level);
    branch = branch->mb_node.a_max;
    goto again;
   }
 }
 return NULL;
}

PUBLIC bool KCALL
mman_inuse_unlocked(struct mman const *__restrict self,
                    VIRT void *start, size_t n_bytes) {
 CHECK_HOST_DOBJ(self);
 assert(mman_reading(self));
 if unlikely(!self->m_map) return false;
 return mman_findany(self->m_map,(uintptr_t)start,(uintptr_t)start+n_bytes-1,
                     ATREE_SEMI0(VIRT uintptr_t),
                     ATREE_LEVEL0(VIRT uintptr_t)) != NULL;
}
PUBLIC bool KCALL
mman_valid_unlocked(struct mman const *__restrict self,
                    VIRT void *start, size_t n_bytes,
                    u32 mask, u32 prot) {
 struct mbranch *iter,*next;
 uintptr_t valid_end;
 CHECK_HOST_DOBJ(self);
 assert(mman_reading(self));
 if unlikely(!self->m_map) return false;
 /* Check for overflow and n_bytes == 0 */
 valid_end = (uintptr_t)start+n_bytes;
 if unlikely((uintptr_t)valid_end <=
             (uintptr_t)start) return false;
 --valid_end;
 iter = mman_getbranch_unlocked(self,start);
 for (;;) {
  if (!iter) return false;
  if ((iter->mb_prot&mask) != prot) return false;
  if (MBRANCH_MAX(iter) >= valid_end) break;
  /* Check for continuous mappings. */
  next = iter->mb_order.le_next;
  if (!next || MBRANCH_BEGIN(next) != MBRANCH_END(iter)) return false;
  iter = next;
 }
 return true;
}

PUBLIC ssize_t KCALL
mman_mextract_unlocked(struct mman *__restrict self,
                       struct mman_maps *__restrict maps,
                       VIRT ppage_t addr, size_t n_bytes,
                       bool unmap_branches) {
 PHYS struct mbranch *iter,*chain;
 VIRT uintptr_t addr_max;
 ssize_t result; errno_t error;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(maps);
 assert(unmap_branches ? mman_writing(self)
                       : mman_reading(self));
 assert(IS_ALIGNED((uintptr_t)addr,PAGESIZE));
 n_bytes  = CEIL_ALIGN(n_bytes,PAGESIZE);
 addr_max = (VIRT uintptr_t)addr+n_bytes-1;
 /* Split branches around the start+end to ensure that we're not extracting too much. */
 if ((error = mman_split_branch_unlocked(self,(VIRT uintptr_t)addr),E_ISERR(error)) ||
     (error = mman_split_branch_unlocked(self,addr_max+1),E_ISERR(error)))
      return error;

 /* Find the first branch within the given address range. */
 iter = mman_findany(self->m_map,(VIRT uintptr_t)addr,addr_max,
                     ATREE_SEMI0(VIRT uintptr_t),
                     ATREE_LEVEL0(VIRT uintptr_t));
 if unlikely(!iter) {
  /* No branches exist within the given range. */
  maps->mm_maps = NULL;
  return 0;
 }
 /* Find the highest branch still apart of the given address range. */
 while (MBRANCH_HASNEXT(iter,self) &&
        MBRANCH_NEXT(iter)->mb_node.a_vmin < addr_max)
        iter = MBRANCH_NEXT(iter);

 result = 0;
 chain  = NULL;
 /* Enumerate all branches within the given address range. */
 while (iter->mb_node.a_vmax > (VIRT uintptr_t)addr) {
  struct mbranch *branch_copy;
  if (unmap_branches) {
   error = mbranch_unmap(iter,&self->m_pdir);
   if (E_ISERR(error)) goto err;
   branch_copy = iter;
   if (branch_copy->mb_notify) {
    error = (*branch_copy->mb_notify)(MNOTIFY_EXTRACT,branch_copy->mb_closure,self,
                                     (ppage_t)((uintptr_t)branch_copy->mb_node.a_vmin-(uintptr_t)addr),
                                      MBRANCH_SIZE(branch_copy));
    if (E_ISERR(error)) { mbranch_remap(iter,&self->m_pdir,false); goto err; }
   }
   /* Unmap the branch. */
   /* Inherit the branch. */
   asserte(branch_copy == mbranch_tree_remove(&self->m_map,iter->mb_node.a_vmin));
   LIST_REMOVE(branch_copy,mb_order);
  } else {
   /* Create a copy of the branch. */
   branch_copy = (struct mbranch *)kmalloc(sizeof(struct mbranch),
                                           GFP_NOFREQ|MMAN_DATAGFP(self));
   if (self == &mman_kernel) (void)_mall_untrack(branch_copy);
   if unlikely(!branch_copy) goto enomem;
   assert(MMAN_DATAOK(self,branch_copy));
   *branch_copy = *iter;
   /* Create a new reference to the region's data. */
   error = mregion_incref(branch_copy->mb_region,
                          branch_copy->mb_start,
                          MBRANCH_SIZE(branch_copy));
   if (E_ISERR(error)) { err2: kffree(branch_copy,GFP_NOFREQ); goto err; }
   if (branch_copy->mb_notify) {
    /* ............ */ error = (errno_t)(*branch_copy->mb_notify)(MNOTIFY_INCREF,branch_copy->mb_closure,self,0,0);
    if (E_ISOK(error)) {
     error = (errno_t)(*branch_copy->mb_notify)(MNOTIFY_CLONE,branch_copy->mb_closure,self,
                                               (ppage_t)((uintptr_t)branch_copy->mb_node.a_vmin-(uintptr_t)addr),
                                                MBRANCH_SIZE(branch_copy));
     if (error == (errno_t)MNOTIFY_CLONE_WITHOUT_CALLBACK) {
      /* Delete the notification callback, but keep the close as a tag. */
      (*branch_copy->mb_notify)(MNOTIFY_DECREF,branch_copy->mb_closure,self,0,0);
      branch_copy->mb_notify = NULL;
      error = -EOK;
     }
    }
    if (E_ISERR(error)) {
     task_nointr();
     mregion_decref(branch_copy->mb_region,
                    branch_copy->mb_start,
                    MBRANCH_SIZE(branch_copy));
     task_endnointr();
     goto err2;
    }
   }
  }
  branch_copy->mb_node.a_vmin  -= (VIRT uintptr_t)addr;
  branch_copy->mb_node.a_vmax  -= (VIRT uintptr_t)addr;
  branch_copy->mb_order.le_next = chain;
  if (chain) chain->mb_order.le_pself = &branch_copy->mb_order.le_next;
  chain = branch_copy;
  if (!MBRANCH_HASPREV(iter,self)) break;
  iter = MBRANCH_PREV(iter);
 }
 if (chain) chain->mb_order.le_pself = &maps->mm_maps;
 maps->mm_maps = chain;

 if (!unmap_branches) {
  /* Try to re-merge branches if we didn't unmap them. */
  mman_merge_branch_unlocked(self,(VIRT uintptr_t)addr);
  mman_merge_branch_unlocked(self,addr_max+1);
 }
 return result;
enomem: error = -ENOMEM;
err:
 /* Cleanup */
 while (chain) {
  iter = chain->mb_order.le_next;
  if (unmap_branches) {
   /* Re-map the branch. */
   chain->mb_node.a_vmin += (VIRT uintptr_t)addr;
   chain->mb_node.a_vmax += (VIRT uintptr_t)addr;
   if (E_ISERR(mbranch_remap(chain,&self->m_pdir,false)))
       goto del_branch; /* We can only delete the branch now... */
   mman_insbranch_unlocked(self,chain);
  } else {
del_branch:
   if (chain->mb_notify) {
    (*chain->mb_notify)(MNOTIFY_DELETE,chain->mb_closure,self,
                       (ppage_t)MBRANCH_BEGIN(chain),MBRANCH_SIZE(chain));
    (*chain->mb_notify)(MNOTIFY_DECREF,chain->mb_closure,self,0,0);
   }
   task_nointr();
   mregion_decref(chain->mb_region,
                  chain->mb_start,
                  MBRANCH_SIZE(chain));
   task_endnointr();
   kffree(chain,GFP_NOFREQ);
  }
  chain = iter;
 }
 return error;
}


PRIVATE errno_t KCALL
mman_do_mrestore(struct mman *__restrict self,
                 struct mbranch *__restrict branch,
                 VIRT ppage_t addr, bool reuse_branches) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(branch);
 assert(IS_ALIGNED((uintptr_t)addr,PAGESIZE));
 /* NOTE: Use teardown recursion to keep the branch
  *       tree in a consistent state after each step. */
#if 1
 if (branch->mb_order.le_next) {
  error = mman_do_mrestore(self,branch->mb_order.le_next,
                           addr,reuse_branches);
  if (E_ISERR(error)) return error;
  branch->mb_order.le_next = NULL;
 }
#else
 if (branch->mb_node.a_min) {
  error = mman_do_mrestore(self,branch->mb_node.a_min,addr,reuse_branches);
  if (E_ISERR(error)) return error;
  branch->mb_node.a_min = NULL;
 }
 if (branch->mb_node.a_max) {
  error = mman_do_mrestore(self,branch->mb_node.a_max,addr,reuse_branches);
  if (E_ISERR(error)) return error;
  branch->mb_node.a_max = NULL;
 }
#endif
 /* Copy the branch if we're not supposed to reuse it. */
 if (!reuse_branches) {
  struct mbranch *branch_copy;
  branch_copy = (struct mbranch *)memdup(branch,sizeof(struct mbranch));
  if unlikely(!branch_copy) return -ENOMEM;
  error = mregion_incref(branch_copy->mb_region,
                         branch_copy->mb_start,
                         MBRANCH_SIZE(branch_copy));
  if (E_ISERR(error)) { err_branch_copy: free(branch_copy); return error; }
  /* Signal the incref() event */
  if (branch_copy->mb_notify) {
   error = (errno_t)(*branch_copy->mb_notify)(MNOTIFY_INCREF,
                                              branch_copy->mb_closure,
                                              NULL,0,0);
   if (E_ISERR(error)) {
    mregion_decref(branch_copy->mb_region,
                   branch_copy->mb_start,
                   MBRANCH_SIZE(branch_copy));
    goto err_branch_copy;
   }
  }
  branch = branch_copy; /* Use our new copy below. */
 }
 /* Add the given addr as offset to the branch. */
 branch->mb_node.a_vmin += (uintptr_t)addr;
 branch->mb_node.a_vmax += (uintptr_t)addr;
 if (branch->mb_notify) {
  error = (errno_t)(*branch->mb_notify)(MNOTIFY_RESTORE,branch->mb_closure,self,
                                       (ppage_t)MBRANCH_BEGIN(branch),MBRANCH_SIZE(branch));
  if (E_ISERR(error)) goto err;
 }


 /* Delete existing mappings. */
 mman_munmap_unlocked(self,(ppage_t)MBRANCH_BEGIN(branch),
                                    MBRANCH_SIZE(branch),
                      MMAN_MUNMAP_ALL,NULL);

 mman_insbranch_unlocked(self,branch);
 error = mbranch_remap(branch,&self->m_pdir,false);
 if (E_ISERR(error)) {
  mman_munmap_unlocked(self,(ppage_t)MBRANCH_BEGIN(branch),
                                     MBRANCH_SIZE(branch),
                       MMAN_MUNMAP_ALL,NULL);
 } else {
  uintptr_t merge1 = MBRANCH_BEGIN(branch);
  uintptr_t merge2 = MBRANCH_END(branch);
  mman_merge_branch_unlocked(self,merge1);
  mman_merge_branch_unlocked(self,merge2);
 }
 return error;
err:
 if (!reuse_branches) {
  if (branch->mb_notify) {
   (*branch->mb_notify)(MNOTIFY_DELETE,branch->mb_closure,NULL,
                       (ppage_t)MBRANCH_BEGIN(branch),MBRANCH_SIZE(branch));
   (*branch->mb_notify)(MNOTIFY_DECREF,branch->mb_closure,NULL,0,0);
  }
  mregion_decref(branch->mb_region,
                 branch->mb_start,
                 MBRANCH_SIZE(branch));
  free(branch);
 }
 return error;
}
PUBLIC errno_t KCALL
mman_mrestore_unlocked(struct mman *__restrict self,
                       struct mman_maps *__restrict maps,
                       VIRT ppage_t addr, bool reuse_branches) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(maps);
 assert(IS_ALIGNED((uintptr_t)addr,PAGESIZE));
 assert(mman_writing(self));
 if (!maps->mm_maps) return -EOK; /* nothing to do here... */
 error = mman_do_mrestore(self,maps->mm_maps,addr,reuse_branches);
 if (E_ISOK(error) && reuse_branches)
     maps->mm_maps = NULL;
 return error;
}

PUBLIC void KCALL
mman_maps_fini(struct mman_maps *__restrict self) {
 struct mbranch *iter,*next;
 CHECK_HOST_DOBJ(self);
 iter = self->mm_maps;
 while (iter) {
  next = iter->mb_order.le_next;
  assert(!next || MBRANCH_MIN(next) > MBRANCH_MAX(iter));
  if (iter->mb_notify) {
   (*iter->mb_notify)(MNOTIFY_DELETE,iter->mb_closure,NULL,
                     (ppage_t)MBRANCH_BEGIN(iter),MBRANCH_SIZE(iter));
   (*iter->mb_notify)(MNOTIFY_DECREF,iter->mb_closure,NULL,0,0);
  }
  /* Drop region references.
   * NOTE: Make sure we're not interrupted doing this. */
  task_nointr();
  mregion_decref(iter->mb_region,iter->mb_start,
                 MBRANCH_SIZE(iter));
  task_endnointr();
  kffree(iter,GFP_NOFREQ);
  iter = next;
 }
}


PUBLIC ssize_t KCALL
mman_mremap_unlocked(struct mman *__restrict self,
                     VIRT ppage_t new_addr, VIRT ppage_t old_addr,
                     size_t n_bytes, u32 prot_amask, u32 prot_omask) {
 ssize_t result,error;
 struct mman_maps maps;
 CHECK_HOST_DOBJ(self);
 assert(mman_writing(self));
 assert(IS_ALIGNED((uintptr_t)old_addr,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)new_addr,PAGESIZE));
 /* Special case: Nothing to do here! */
 if (old_addr == new_addr) return 0;
 result = mman_mextract_unlocked(self,&maps,old_addr,n_bytes,true);
 if (E_ISERR(result)) return result;
 error = mman_munmap_unlocked(self,new_addr,n_bytes,MMAN_MUNMAP_ALL,NULL);
 if (E_ISERR(error)) goto maps_fail;
 /* ............. */ error = mman_mrestore_unlocked(self,&maps,new_addr,true);
 if (E_ISERR(error)) error = mman_mrestore_unlocked(self,&maps,old_addr,true);
 if (E_ISERR(error)) goto maps_fail;
 return -ENOMEM;
maps_fail:
 mman_maps_fini(&maps);
 return error;
}


PRIVATE ssize_t KCALL
mman_munmap_impl(struct mman *__restrict self,
                 struct mbranch **__restrict pbranch,
                 PAGE_ALIGNED VIRT uintptr_t addr_min,
                              VIRT uintptr_t addr_max,
                 ATREE_SEMI_T(VIRT uintptr_t) addr_semi,
                 ATREE_LEVEL_T addr_level,
                 u32 mode, void *tag) {
 errno_t temp; ssize_t result = 0;
 struct mbranch *branch;
again:
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(pbranch);
 branch = *pbranch;
 if (!branch) goto end;
 CHECK_HOST_DOBJ(branch);
 CHECK_HOST_DOBJ(branch->mb_region);
 assert(IS_ALIGNED(addr_min,PAGESIZE));
 assert(IS_ALIGNED(addr_max+1,PAGESIZE));
 assert(addr_min <= addr_max);
 if (addr_min <= branch->mb_node.a_vmax &&
     addr_max >= branch->mb_node.a_vmin) {
  /* Check the closure tag if requested, to. */
  if (mode&MMAN_MUNMAP_TAG &&
      branch->mb_closure != tag)
      goto next;
  /* Found a matching entry!
   * NOTE: Since the caller already split branches
   *       near borders, we are allowed to simply
   *       delete this entire branch! */
  temp = mbranch_unmap(branch,&self->m_pdir);
  if (E_ISERR(temp)) return temp;
  temp = (mode&MMAN_MUNMAP_CLEAR)
   ? mregion_decref_clr(branch->mb_region,branch->mb_start,MBRANCH_SIZE(branch))
   : mregion_decref    (branch->mb_region,branch->mb_start,MBRANCH_SIZE(branch));
  if (E_ISERR(temp)) return temp;
  /* Signal the DECREF. */

  if (branch->mb_notify) {
   (*branch->mb_notify)(MNOTIFY_UNMAP,branch->mb_closure,self,
                       (ppage_t)MBRANCH_BEGIN(branch),MBRANCH_SIZE(branch));
   (*branch->mb_notify)(MNOTIFY_DECREF,branch->mb_closure,NULL,0,0);
  }

  result += MBRANCH_SIZE(branch);
  mbranch_tree_pop_at(pbranch,addr_semi,addr_level);
  LIST_REMOVE(branch,mb_order);
  kffree(branch,GFP_NOFREQ);
  goto again;
 }

next:
 { bool walk_min,walk_max;
   walk_min = addr_min <  addr_semi && branch->mb_node.a_min;
   walk_max = addr_max >= addr_semi && branch->mb_node.a_max;
   if (walk_min) {
    /* Recursively continue searching left. */
    if (walk_max) {
     temp = mman_munmap_impl(self,&branch->mb_node.a_max,addr_min,addr_max,
                             ATREE_NEXTMAX(VIRT uintptr_t,addr_semi,addr_level),
                             ATREE_NEXTLEVEL(addr_level),mode,tag);
     if (E_ISERR(temp)) return temp;
     result += temp;
    }
    ATREE_WALKMIN(VIRT uintptr_t,addr_semi,addr_level);
    pbranch = &branch->mb_node.a_min;
    goto again;
   } else if (walk_max) {
    /* Recursively continue searching right. */
    ATREE_WALKMAX(VIRT uintptr_t,addr_semi,addr_level);
    pbranch = &branch->mb_node.a_max;
    goto again;
   }
 }
end:
 return result;
}

PUBLIC ssize_t KCALL
mman_munmap_unlocked(struct mman *__restrict self,
                     VIRT ppage_t addr, size_t n_bytes,
                     u32 mode, void *tag) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 assertf((uintptr_t)addr+n_bytes >= (uintptr_t)addr,
         "Overflow in address range %p...%p",
         addr,(uintptr_t)addr+n_bytes-1);
 if unlikely(!n_bytes) return 0;
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 if ((error = mman_split_branch_unlocked(self,(VIRT uintptr_t)addr),E_ISERR(error)) ||
     (error = mman_split_branch_unlocked(self,(VIRT uintptr_t)addr+n_bytes),E_ISERR(error)))
      return -ENOMEM;
 /* Recursively update protection mappings. */
 return mman_munmap_impl(self,&self->m_map,
                        (VIRT uintptr_t)addr,
                        (VIRT uintptr_t)addr+n_bytes-1,
                         ATREE_SEMI0(VIRT uintptr_t),
                         ATREE_LEVEL0(VIRT uintptr_t),
                         mode,tag);
}



PRIVATE ssize_t KCALL
mman_mlock_impl(struct mbranch *__restrict branch,
                struct mman *__restrict mspace,
                PAGE_ALIGNED VIRT uintptr_t addr_min,
                             VIRT uintptr_t addr_max,
                ATREE_SEMI_T(VIRT uintptr_t) addr_semi,
                ATREE_LEVEL_T addr_level, u32 mode) {
 ssize_t temp,result = 0;
 bool walk_min,walk_max;
 struct mbranch *min_branch;
 struct mbranch *max_branch;
again:
 CHECK_HOST_DOBJ(mspace);
 CHECK_HOST_DOBJ(branch);
 CHECK_HOST_DOBJ(branch->mb_region);
 assert(IS_ALIGNED(addr_min,PAGESIZE));
 assert(IS_ALIGNED(addr_max+1,PAGESIZE));
 assert(addr_min <= addr_max);
 min_branch = branch->mb_node.a_min;
 max_branch = branch->mb_node.a_max;
 walk_min = addr_min <  addr_semi && min_branch != NULL;
 walk_max = addr_max >= addr_semi && max_branch != NULL;

 if (addr_min <= branch->mb_node.a_vmax &&
     addr_max >= branch->mb_node.a_vmin) {
  raddr_t region_begin,region_end; rsize_t region_size;
  struct mregion *region;
  /* Found a matching entry!
   * >> Figure out how much of this must be loaded into the core. */
  assert(addr_min >= branch->mb_node.a_vmin);
  assert(addr_max <= branch->mb_node.a_vmax);
  region_begin = branch->mb_start+(addr_min-branch->mb_node.a_vmin);
  region_end   = branch->mb_start+((addr_max+1)-branch->mb_node.a_vmin);
  region_size  = region_end-region_begin;
  region = branch->mb_region;
  temp = rwlock_write(&region->mr_plock);
  if (E_ISERR(temp)) return temp;
  /* Create/Delete a lock for the region in question. */
  if ((mode&MMAN_MLOCK_LOCK)
      ? !mregion_part_inclock(region,region_begin,region_size)
      : !mregion_part_declock(region,region_begin,region_size))
       temp = -ENOMEM;
  else result += region_size;
  rwlock_endwrite(&region->mr_plock);
  if (E_ISERR(temp)) return temp;

  /* Load core memory if need be. */
  if (mode&MMAN_MLOCK_INCORE) {
   bool did_remap = false;
   /* Load core pages if requested to. */
   temp = mbranch_mcore(branch,mspace,region_begin,region_size,
                        mode&~(MMAN_MLOCK_UNLOCK|MMAN_MLOCK_LOCK),
                       &did_remap);
   if (E_ISERR(temp)) return temp;
   /* This check is required when 'mbranch_mcore' remaps branches in a
    * way that would otherwise make us skip a whole bunch of branches. */
   if (did_remap) {
    if (addr_min+PAGESIZE-1 == addr_max) goto end;
    /* Must start over to ensure all branches are loaded into the core. */
    branch     = mspace->m_map;
    addr_semi  = ATREE_SEMI0(VIRT uintptr_t);
    addr_level = ATREE_LEVEL0(VIRT uintptr_t);
    goto again;
   }
  }
 }

 if (walk_min) {
  /* Recursively continue searching left. */
  if (walk_max) {
   temp = mman_mlock_impl(max_branch,mspace,addr_min,addr_max,
                          ATREE_NEXTMAX(VIRT uintptr_t,addr_semi,addr_level),
                          ATREE_NEXTLEVEL(addr_level),mode);
   if (E_ISERR(temp)) return temp;
   result += temp;
  }
  ATREE_WALKMIN(VIRT uintptr_t,addr_semi,addr_level);
  branch = min_branch;
  goto again;
 } else if (walk_max) {
  /* Recursively continue searching right. */
  ATREE_WALKMAX(VIRT uintptr_t,addr_semi,addr_level);
  branch = max_branch;
  goto again;
 }
end:
 return result;
}

PUBLIC ssize_t KCALL
mman_mlock_unlocked(struct mman *__restrict self,
                    VIRT ppage_t addr, size_t n_bytes,
                    u32 mode) {
 CHECK_HOST_DOBJ(self);
 assert(IS_ALIGNED((uintptr_t)addr,PAGESIZE));
 if unlikely(!n_bytes) return 0;
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 /* Recursively scan branches within the given range. */
 return mman_mlock_impl(self->m_map,self,
                       (VIRT uintptr_t)addr,
                       (VIRT uintptr_t)addr+n_bytes-1,
                        ATREE_SEMI0(VIRT uintptr_t),
                        ATREE_LEVEL0(VIRT uintptr_t),
                        mode);
}


PRIVATE ssize_t KCALL
mman_mcore_impl(struct mbranch *__restrict branch,
                struct mman *__restrict mspace,
                PAGE_ALIGNED VIRT uintptr_t addr_min,
                             VIRT uintptr_t addr_max,
                ATREE_SEMI_T(VIRT uintptr_t) addr_semi,
                ATREE_LEVEL_T addr_level, u32 mode) {
 ssize_t temp,result = 0;
 bool walk_min,walk_max;
 struct mbranch *min_branch;
 struct mbranch *max_branch;
again:
 CHECK_HOST_DOBJ(mspace);
 CHECK_HOST_DOBJ(branch);
 CHECK_HOST_DOBJ(branch->mb_region);
 assert(IS_ALIGNED(addr_min,PAGESIZE));
 assert(IS_ALIGNED(addr_max+1,PAGESIZE));
 assert(addr_min <= addr_max);
 min_branch = branch->mb_node.a_min;
 max_branch = branch->mb_node.a_max;
 walk_min = addr_min <  addr_semi && min_branch != NULL;
 walk_max = addr_max >= addr_semi && max_branch != NULL;

 if (addr_min <= branch->mb_node.a_vmax &&
     addr_max >= branch->mb_node.a_vmin) {
  raddr_t region_begin,region_end; rsize_t region_size;
  bool did_remap = false;
  /* Found a matching entry!
   * >> Figure out how much of this must be loaded into the core. */
  assert(addr_min >= branch->mb_node.a_vmin);
  assert(addr_max <= branch->mb_node.a_vmax);
  region_begin = branch->mb_start+(addr_min-branch->mb_node.a_vmin);
  region_end   = branch->mb_start+((addr_max+1)-branch->mb_node.a_vmin);
  region_size  = region_end-region_begin;
  temp = mbranch_mcore(branch,mspace,region_begin,
                       region_size,mode,&did_remap);
  if (E_ISERR(temp)) return temp;
  result += temp;
  /* This check is required when 'mbranch_mcore' remaps branches in a
   * way that would otherwise make us skip a whole bunch of branches. */
  if (did_remap) {
   if (addr_min+PAGESIZE-1 == addr_max) goto end;
   /* Must start over to ensure all branches are loaded into the core. */
   branch     = mspace->m_map;
   addr_semi  = ATREE_SEMI0(VIRT uintptr_t);
   addr_level = ATREE_LEVEL0(VIRT uintptr_t);
   goto again;
  }
 }

 if (walk_min) {
  /* Recursively continue searching left. */
  if (walk_max) {
   temp = mman_mcore_impl(max_branch,mspace,addr_min,addr_max,
                          ATREE_NEXTMAX(VIRT uintptr_t,addr_semi,addr_level),
                          ATREE_NEXTLEVEL(addr_level),mode);
   if (E_ISERR(temp)) return temp;
   result += temp;
  }
  ATREE_WALKMIN(VIRT uintptr_t,addr_semi,addr_level);
  branch = min_branch;
  goto again;
 } else if (walk_max) {
  /* Recursively continue searching right. */
  ATREE_WALKMAX(VIRT uintptr_t,addr_semi,addr_level);
  branch = max_branch;
  goto again;
 }
end:
 return result;
}

PUBLIC ssize_t KCALL
mman_mcore_unlocked(struct mman *__restrict self,
                    VIRT ppage_t addr, size_t n_bytes,
                    u32 mode) {
 CHECK_HOST_DOBJ(self);
 assert(IS_ALIGNED((uintptr_t)addr,PAGESIZE));
 if unlikely(!n_bytes) return 0;
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 /* Recursively scan branches within the given range. */
 return mman_mcore_impl(self->m_map,self,
                       (VIRT uintptr_t)addr,
                       (VIRT uintptr_t)addr+n_bytes-1,
                        ATREE_SEMI0(VIRT uintptr_t),
                        ATREE_LEVEL0(VIRT uintptr_t),
                        mode);
}

PUBLIC ssize_t KCALL
mman_print_unlocked(struct mman *__restrict self,
                    pformatprinter printer, void *closure) {
 ssize_t result = 0,temp;
 struct mbranch *iter;
 assert(mman_reading(self));
 MMAN_FOREACH(iter,self) {
  struct mregion *region;
  temp = format_printf(printer,closure,"%p-%p %c%c%c%c ",
                       iter->mb_node.a_vmin,
#if 1
                       iter->mb_node.a_vmax,
#else
                       iter->mb_node.a_vmax+1,
#endif
                       iter->mb_prot&PROT_READ ? 'r' : '-',
                       iter->mb_prot&PROT_WRITE ? 'w' : '-',
                       iter->mb_prot&PROT_EXEC ? 'x' : '-',
                       iter->mb_prot&PROT_SHARED ? 's' : 'p');
  if unlikely(temp < 0) return temp;
  result += temp;
  region  = iter->mb_region;
  if (MREGION_INIT_ISFILE(region->mr_init)) {
   struct inode *node = region->mr_setup.mri_file->f_node;
   temp = format_printf(printer,closure,"%.8I64x %[dev_t] %10I32u %[file]\n",
                       (uint64_t)(region->mr_setup.mri_start+region->mr_setup.mri_begin)+iter->mb_start,
                       (dev_t)(node->i_super->sb_blkdev ? node->i_super->sb_blkdev->bd_device.d_id
                                                        : 0),
                       (uint32_t)node->i_ino,region->mr_setup.mri_file);
   if unlikely(temp < 0) return temp;
   result += temp;
  } else {
   temp = format_printf(printer,closure,"%.8Ix 00:00 0          \n",iter->mb_start);
   if unlikely(temp < 0) return temp;
   result += temp;
  }
 }
 return result;
}



#define CORE_RANGE_ROTEXT     0 /* Read-only kernel text region. */
#define CORE_RANGE_RWDATA     1 /* Read-only kernel data region. */
#define CORE_RANGE_MODULE_MAX 1 /* Greater core range index that mirrors module segments. */
#ifdef CONFIG_PDIR_SELFMAP
#define CORE_RANGE_THIS_PDIR  2 /* Reserved kernel region for the page-directory self-entry mappings. */
#endif


struct addrpair {
 uintptr_t ap_min;
 uintptr_t ap_max;
 u32       ap_prot;
};
PRIVATE ATTR_FREERODATA struct addrpair const kernel_core_ranges[] = {
    [CORE_RANGE_ROTEXT] = {KERNEL_RO_BEGIN,KERNEL_RO_END-1,PROT_READ|PROT_EXEC},
    [CORE_RANGE_RWDATA] = {KERNEL_RW_BEGIN,KERNEL_RW_END-1,PROT_READ|PROT_WRITE|PROT_EXEC},
#ifdef CONFIG_PDIR_SELFMAP
    [CORE_RANGE_THIS_PDIR] = {
        (uintptr_t)THIS_PDIR_BASE,
        (uintptr_t)THIS_PDIR_BASE+THIS_PDIR_SIZE-1,
        PROT_READ|PROT_WRITE,
    },
#endif
};

/* WARNING: These regions must contain at least mirror segments from
 *         '__core_segments' in '/kernel/linker/module.c' */
PRIVATE ATTR_COLDDATA struct mregion kernel_core_regions[] = {
    [CORE_RANGE_ROTEXT] = {
#ifdef CONFIG_DEBUG
        .mr_refcnt = 1,
#else
        .mr_refcnt = 0x80000001,
#endif
        .mr_type   = MREGION_TYPE_PHYSICAL,
        .mr_init   = MREGION_INIT_RAND, /* Actually 'MREGION_INIT_FILE', but we don't have the file... */
        .mr_size   = KERNEL_RO_SIZE,
        .mr_futex  = {{0}},
        .mr_plock  = RWLOCK_INIT,
        .mr_parts  = &kernel_core_regions[CORE_RANGE_ROTEXT].mr_part0,
        .mr_part0  = {
            .mt_chain = {
                .le_pself = &kernel_core_regions[CORE_RANGE_ROTEXT].mr_parts,
                .le_next  = NULL,
            },
            .mt_start  = 0,
            .mt_refcnt = 1,
            .mt_state  = MPART_STATE_INCORE,
            .mt_flags  = MPART_FLAG_NONE,
            .mt_locked = 1,
            .mt_memory = {
                .m_next  = NULL,
                .m_start = (ppage_t)virt_to_phys(KERNEL_RO_BEGIN),
                .m_size  = KERNEL_RO_SIZE,
            },
        },
        .mr_global = {NULL,NULL},
    },
    [CORE_RANGE_RWDATA] = {
#ifdef CONFIG_DEBUG
        .mr_refcnt = 1,
#else
        .mr_refcnt = 0x80000001,
#endif
        .mr_type   = MREGION_TYPE_PHYSICAL,
        .mr_init   = MREGION_INIT_RAND, /* Actually 'MREGION_INIT_FILE', but we don't have the file... */
        .mr_size   = KERNEL_RO_SIZE,
        .mr_futex  = {{0}},
        .mr_plock  = RWLOCK_INIT,
        .mr_parts  = &kernel_core_regions[CORE_RANGE_RWDATA].mr_part0,
        .mr_part0  = {
            .mt_chain = {
                .le_pself = &kernel_core_regions[CORE_RANGE_RWDATA].mr_parts,
                .le_next  = NULL,
            },
            .mt_start  = 0,
            .mt_refcnt = 1,
            .mt_state  = MPART_STATE_INCORE,
            .mt_flags  = MPART_FLAG_NONE,
            .mt_locked = 1,
            .mt_memory = {
                .m_next  = NULL,
                .m_start = (ppage_t)virt_to_phys(KERNEL_RW_BEGIN),
                .m_size  = KERNEL_RW_SIZE,
            },
        },
        .mr_global = {NULL,NULL},
    },
#ifdef CONFIG_PDIR_SELFMAP
    [CORE_RANGE_THIS_PDIR] = {
#ifdef CONFIG_DEBUG
        .mr_refcnt = 1,
#else
        .mr_refcnt = 0x80000001,
#endif
        .mr_type   = MREGION_TYPE_RESERVED,
        .mr_size   = THIS_PDIR_SIZE,
        .mr_futex  = {{0}},
        .mr_plock  = RWLOCK_INIT,
        .mr_parts  = &kernel_core_regions[CORE_RANGE_THIS_PDIR].mr_part0,
        .mr_part0  = {
            .mt_chain = {
                .le_pself = &kernel_core_regions[CORE_RANGE_THIS_PDIR].mr_parts,
                .le_next  = NULL,
            },
            .mt_start  = 0,
            .mt_refcnt = 1,
            .mt_state  = MPART_STATE_UNKNOWN,
            .mt_flags  = MPART_FLAG_NONE,
            .mt_locked = 1,
        },
        .mr_global = {NULL,NULL},
    },
#endif
};

STATIC_ASSERT(COMPILER_LENOF(kernel_core_ranges) ==
              COMPILER_LENOF(kernel_core_regions));



/* Create a memory branch within the kernel page directory
 * for a physical memory mapping within the given range. */
INTERN ATTR_FREETEXT void KCALL
mman_map_dynmem(PHYS ppage_t start, size_t n_bytes) {
 struct mregion *region;
 struct mbranch *branch;
 CHECK_HOST_DATA(start,n_bytes);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)n_bytes,PAGESIZE));
 assert(n_bytes);
 region = (struct mregion *)kcalloc(sizeof(struct mregion),GFP_MEMORY);
 branch = (struct mbranch *)kcalloc(sizeof(struct mbranch),GFP_MEMORY);
 if unlikely(!region || !branch) {
  syslog(LOG_ERROR|LOG_MEM,
         FREESTR("[MEM] Failed to create mman-controller for physical memory %p...%p: %[errno]\n"),
        (uintptr_t)start,(uintptr_t)start+n_bytes-1,ENOMEM);
  free(region);
  free(branch);
  return;
 }
 (void)_mall_nofree(region);
 (void)_mall_nofree(branch);
 region->mr_refcnt = 1;
#if MREGION_TYPE_PHYSICAL
 region->mr_type   = MREGION_TYPE_PHYSICAL;
#endif
#if MREGION_INIT_RAND
 region->mr_init   = MREGION_INIT_RAND;
#endif
 rwlock_cinit(&region->mr_plock);
 region->mr_size                    = n_bytes;
 region->mr_parts                   = &region->mr_part0;
 region->mr_part0.mt_chain.le_pself = &region->mr_parts;
 region->mr_part0.mt_refcnt         = 1;
 region->mr_part0.mt_locked         = 1; /* Makes sense: Physical memory cannot be swapped, so we simply lock it. */
 region->mr_part0.mt_state          = MPART_STATE_INCORE;
 region->mr_part0.mt_flags          = MPART_FLAG_CHNG; /* Shouldn't really make a difference... */
 region->mr_part0.mt_memory.m_start = start;
 region->mr_part0.mt_memory.m_size  = n_bytes;

 branch->mb_region      = region; /* Inherit reference. */
 /* Generic, dynamic memory shouldn't isn't considered executable by default. */
 branch->mb_prot        = PROT_READ|PROT_WRITE|PROT_NOUSER;
 branch->mb_node.a_vmin = (VIRT uintptr_t)start;
 branch->mb_node.a_vmax = (VIRT uintptr_t)start+n_bytes-1;

 /* Finally, insert the branch we've just generated. */
 mman_insbranch_unlocked(&mman_kernel,branch);
}


/* The low-level assembly handler for PAGEFAULTs */
INTDEF void ASMCALL mman_asm_pf(void);
PRIVATE ATTR_FREERODATA isr_t const pf_isr = ISR_DEFAULT(EXC_PAGE_FAULT,&mman_asm_pf);

INTERN ATTR_FREETEXT void KCALL
mman_initialize(void) {
 unsigned int region_index;
 ATOMIC_WRITE(mman_kernel.m_lock.orw_lock.aorw_lock,
              ATOMIC_OWNER_RWLOCK_WFLAG);
 for (region_index = 0;
      region_index < COMPILER_LENOF(kernel_core_regions);
    ++region_index) {
  struct mbranch *b;
  b = (struct mbranch *)kcalloc(sizeof(struct mbranch),GFP_MEMORY);
  if unlikely(!b) {
   syslog(LOG_ERROR|LOG_MEM,
          FREESTR("[MEM] Failed to allocate mman kernel root branch #%d\n"),
          region_index);
   continue;
  }
  /* NOTE: Protect the mappings on the debug heap by marking them as NOFREE. */
  (void)_mall_nofree(b);
  b->mb_node.a_vmin = kernel_core_ranges[region_index].ap_min;
  b->mb_node.a_vmax = kernel_core_ranges[region_index].ap_max;
  b->mb_prot        = kernel_core_ranges[region_index].ap_prot;
  b->mb_region      = &kernel_core_regions[region_index];
  if (region_index <= CORE_RANGE_MODULE_MAX) {
   b->mb_notify  = &instance_mnotify;
   b->mb_closure = THIS_INSTANCE;
  }
  assert(MBRANCH_SIZE(b));
  /* Insert the two branches into the map. */
  mman_insbranch_unlocked(&mman_kernel,b);
 }
 assert(mman_kernel.m_map != NULL);


 /* With kernel core branches created, it is time to setup all the
  * additional branches for dynamic memory at its physical address.
  * NOTE: For this, we can simply re-use 'mem_info' the same
  *       way it got used during fixup initialization of the
  *       kernel page directory.
  * NOTE: Just like the mappings above, these mappings must remain forever! */
 { mzone_t id;
   for (id = 0; id != MZONE_COUNT; ++id) {
    PHYS struct meminfo const *iter = mem_info[id];
    for (; iter != MEMINFO_EARLY_NULL; iter = iter->mi_next) {
     if (!MEMTYPE_ISMAP(iter->mi_type)) continue;
     mman_map_dynmem(iter->mi_part_addr,iter->mi_part_size);
    }
   }
 }
 assert(mman_kernel.m_map != NULL);
 ATOMIC_WRITE(mman_kernel.m_lock.orw_lock.aorw_lock,0);

 /* Register an ISR service routine for pagefault handling. */
 irq_set(&pf_isr,NULL,IRQ_SET_RELOAD);
}











/* Internal utility functions. */
INTERN errno_t KCALL
mman_split_branch_unlocked(struct mman *__restrict self,
                           PAGE_ALIGNED VIRT uintptr_t start) {
 struct mbranch **plo_branch,*lo_branch;
 struct mbranch *hi_branch; errno_t error;
 ATREE_SEMI_T(VIRT uintptr_t) lo_branch_semi = ATREE_SEMI0(VIRT uintptr_t);
 ATREE_LEVEL_T lo_branch_level = ATREE_LEVEL0(VIRT uintptr_t);

 CHECK_HOST_DOBJ(self);
 assert(mman_writing(self));
 assertf(IS_ALIGNED(start,PAGESIZE),"Invalid start: %p",start);
 plo_branch = mbranch_tree_plocate_at(&self->m_map,start,
                                      &lo_branch_semi,&lo_branch_level);
 /* Simple case: No branch exists. */
 if unlikely(!plo_branch) return -EOK;
 lo_branch = *plo_branch;
 CHECK_HOST_DOBJ(lo_branch);

 /* Another simple case: The branch already starts here. */
 if (lo_branch->mb_node.a_vmin == start) return -EOK;

 hi_branch = (struct mbranch *)kmalloc(sizeof(struct mbranch),
                                       MMAN_DATAGFP(self)|GFP_NOFREQ);
 if (self == &mman_kernel) (void)_mall_untrack(hi_branch);
 if unlikely(!hi_branch) return -ENOMEM;

 /* Create a reference to the branch's notifier. */
 error = MBRANCH_INCREF(lo_branch,self);
 if (E_ISERR(error)) { kffree(hi_branch,GFP_NOFREQ); return error; }

 /* Pop the branch (for now) */
 mbranch_tree_pop_at(plo_branch,lo_branch_semi,lo_branch_level);

 /* Clone some easy fields. */
 hi_branch->mb_node.a_vmin = start;
 hi_branch->mb_node.a_vmax = lo_branch->mb_node.a_vmax;
 hi_branch->mb_prot        = lo_branch->mb_prot;
 hi_branch->mb_notify      = lo_branch->mb_notify; /* Inherit... */
 hi_branch->mb_closure     = lo_branch->mb_closure; /* ... Reference. */
 lo_branch->mb_node.a_vmax = start-1;

 /* No need to fiddle with part reference counters. - Those are all inherited! */
 hi_branch->mb_start  = lo_branch->mb_start;
 hi_branch->mb_start += start-lo_branch->mb_node.a_vmin;
 hi_branch->mb_region = lo_branch->mb_region;
 MREGION_INCREF(lo_branch->mb_region);

 /* Insert the split branch after the lower-ordered one. */
 LIST_INSERT_AFTER(lo_branch,hi_branch,mb_order);

 /* Now just re-insert both branches into the address tree. */
 mbranch_tree_insert(&self->m_map,lo_branch);
 mbranch_tree_insert(&self->m_map,hi_branch);

 return -EOK;
}


INTERN bool KCALL
mman_merge_branch_unlocked(struct mman *__restrict self,
                           PAGE_ALIGNED VIRT uintptr_t start) {
 struct mbranch **plo_branch,*lo_branch;
 struct mbranch **phi_branch,*hi_branch;
 struct mregion *lo_region,*hi_region;
 ATREE_SEMI_T(VIRT uintptr_t) lo_branch_semi = ATREE_SEMI0(VIRT uintptr_t);
 ATREE_SEMI_T(VIRT uintptr_t) hi_branch_semi = ATREE_SEMI0(VIRT uintptr_t);
 ATREE_LEVEL_T lo_branch_level = ATREE_LEVEL0(VIRT uintptr_t);
 ATREE_LEVEL_T hi_branch_level = ATREE_LEVEL0(VIRT uintptr_t);
 CHECK_HOST_DOBJ(self);
 assert(mman_writing(self));
 assert(IS_ALIGNED(start,PAGESIZE));

 /* Scan for two different branches at 'start' and 'start-1' */
 plo_branch = mbranch_tree_plocate_at(&self->m_map,start-1,&lo_branch_semi,&lo_branch_level);
 if (!plo_branch) return false;
 lo_branch = *plo_branch;
 CHECK_HOST_DOBJ(lo_branch);
 if (lo_branch->mb_node.a_vmax != start-1) return false;
 phi_branch = mbranch_tree_plocate_at(&self->m_map,start,&hi_branch_semi,&hi_branch_level);
 if (!phi_branch) return false;
 hi_branch = *phi_branch;
 CHECK_HOST_DOBJ(hi_branch);
 assert(lo_branch != hi_branch);
 assert(MBRANCH_END(lo_branch) == MBRANCH_BEGIN(hi_branch));
 /* Check if basic branch settings such as protection and callbacks match.
  * HINT: 'mb_closure' is the memory tag, meaning that this also checks for matching tags. */
 if (lo_branch->mb_prot != hi_branch->mb_prot ||
     lo_branch->mb_notify != hi_branch->mb_notify ||
     lo_branch->mb_closure != hi_branch->mb_closure)
     return false;
 lo_region = lo_branch->mb_region;
 hi_region = lo_branch->mb_region;
 if (lo_region == hi_region) {
  /* Merge the two branches into one. */
  if (lo_branch->mb_start+MBRANCH_SIZE(lo_branch) !=
      hi_branch->mb_start) return false;
  /* Simple case: Both branches use the same region.
   * To accomplish this, we simply delete the hi-branch and extend the lower one. */
  mbranch_tree_pop_at(phi_branch,hi_branch_semi,hi_branch_level);
  mbranch_tree_pop_at(plo_branch,lo_branch_semi,lo_branch_level);

  /* Simply update the lo-branch max-pointer to end where the hi-branch used to stop.
   * NOTE: In doing this, the lo-branch also inherits all parts references from the hi-branch. */
  lo_branch->mb_node.a_vmax = hi_branch->mb_node.a_vmax;

  syslog(LOG_MEM|LOG_DEBUG,
         "[MEM] Merging shared region of leafs at %p...%p and %p...%p\n",
         MBRANCH_MIN(lo_branch),MBRANCH_MAX(lo_branch),
         MBRANCH_MIN(hi_branch),MBRANCH_MAX(hi_branch));

  /* Unlink and delete the higher-order branch. */
  LIST_REMOVE(hi_branch,mb_order);
  kffree(hi_branch,GFP_NOFREQ);

  /* Since now only one branch exists that holds all the part-references,
   * drop the region-reference previously held by the hi-branch.
   * NOTE: This can never actually destroy the region, because the
   *       lo-branch must also be holding a reference. */
  assert(hi_region->mr_refcnt >= 2);
  asserte(ATOMIC_DECFETCH(hi_region->mr_refcnt) >= 1);

  /* NOTE: No need to re-map, or update the order list.
   *    >> The mapped data remains the same, and the
   *       lo-branch remains at the same location. */
  mbranch_tree_insert(&self->m_map,lo_branch);

  return false;
 }
 /* Check if the two adjacent regions can be merged into one. */

 /* Branches cannot be merged if the associated region
  * is special in that it is shared with another VM.
  * If it is, we must continue to use the old region, whether we like it or not,
  * as doing another else would break shared memory semantics.
  * >> Basically, we're going to create a new region that
  *    encompasses the parts of both the low and high regions,
  *    meaning that we're going to inherit data from either.
  * WARNING: This check, while required, introduces a race condition
  *          that occurs when this VM was sharing memory with another
  *          process at one point, causing this check to fail, but later
  *          then that other process died, leaving the first with two
  *          memory branches that could theoretically be merged, but
  *          weren't before due to data being shared.
  *       >> This may sound bad, but you've got to realize that
  *          merging memory branches in itself is completely optional
  *          and only serves to optimize performance, lookup time, as
  *          well as overhead.
  * The only thing it can affect, is the output of '/proc/PID/maps' */
 if (lo_region->mr_refcnt > 1 || hi_region->mr_refcnt > 1)
     return false;
 /* Make sure both regions are of the same type. */
 if (lo_region->mr_type != hi_region->mr_type)
     return false;
 /* TODO: Check matching initializers only if both regions
  *       contain unallocated portions within their respective
  *       mapped parts. */

 /* TODO */
 return false;
}

DECL_END

#ifndef __INTELLISENSE__
#include "environ.c.inl"
#include "mcore.c.inl"
#ifndef CONFIG_NO_LDT
#include "ldt.c.inl"
#endif /* !CONFIG_NO_LDT */
#endif

#endif /* !GUARD_KERNEL_MMAN_MMAN_C */
