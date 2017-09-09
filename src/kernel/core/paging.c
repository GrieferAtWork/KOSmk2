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
#ifndef GUARD_KERNEL_CORE_PAGING_C
#define GUARD_KERNEL_CORE_PAGING_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <hybrid/align.h>
#include <hybrid/arch/cpu.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <kernel/memory.h>
#include <kernel/mman.h>
#include <kernel/paging-util.h>
#include <kernel/paging.h>
#include <kos/syslog.h>
#include <sched/cpu.h>
#include <sched/task.h>
#include <stdalign.h>
#include <string.h>
#ifndef CONFIG_NO_LDT
#include <kernel/arch/task.h>
#endif

DECL_BEGIN

/* Memory zone used for dynamic allocation of page tables. */
#define PDIR_PAGEZONE  MZONE_ANY

STATIC_ASSERT(alignof(pdir_t) >= PAGESIZE);

typedef /*ALIGNED(PDTABLE_REPRSIZE)*/ ppage_t ptable_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
STATIC_ASSERT(PTTABLE_ARRAYSIZE                  == 1024);
STATIC_ASSERT(PTTABLE_ARRAYSIZE                  == PD_TABLE_ENTRY_COUNT);
STATIC_ASSERT(PDTABLE_ALLOCSIZE                  == PAGESIZE);
STATIC_ASSERT(PDENTRY_REPRSIZE                   == PAGESIZE);
STATIC_ASSERT(PTTABLE_ARRAYSIZE*PDENTRY_REPRSIZE == PDTABLE_REPRSIZE);
STATIC_ASSERT(PDIR_SIZE                        == PDIR_TABLE_COUNT*PD_TABLE_SIZE);
STATIC_ASSERT(PDIR_TABLE_COUNT                   == PD_TABLE_ENTRY_COUNT);
STATIC_ASSERT(IS_ALIGNED(KERNEL_BASE,PDTABLE_REPRSIZE));
STATIC_ASSERT(sizeof(union pd_entry) == 4);
STATIC_ASSERT(sizeof(union pd_table) == 4);

#ifdef CONFIG_PDIR_SELFMAP
STATIC_ASSERT((uintptr_t)(THIS_PDIR_BASE) == (uintptr_t)(0-(PDIR_TABLE_COUNT*PD_TABLE_ENTRY_COUNT*PD_ENTRY_SIZE)));
STATIC_ASSERT((uintptr_t)(THIS_PDIR_BASE) >= (uintptr_t)(KERNEL_GLOBAL_END));
STATIC_ASSERT((uintptr_t)(THIS_PDIR_BASE+THIS_PDIR_SIZE) == 0 ||
             ((uintptr_t)(THIS_PDIR_BASE+THIS_PDIR_SIZE) > (uintptr_t)(KERNEL_GLOBAL_END)));
#endif /* CONFIG_PDIR_SELFMAP */
#pragma GCC diagnostic pop


#ifdef CONFIG_PDIR_SELFMAP
/* Initialize the page-directory self-mappings for all addresses above 'KERNEL_BASE'.
 * NOTE: The caller is responsible for allocating the
 *       page table at 'THIS_PDIR_BASE/PDTABLE_REPRSIZE'. */
PRIVATE void KCALL
pdir_initialize_selfmap(pdir_t *__restrict self) {
 union pd_entry *selfmap_table;
 union pd_table *iter,*end;
 CHECK_HOST_DOBJ(self);
 assert(IS_ALIGNED(KERNEL_BASE,PDTABLE_REPRSIZE));
 assert(PDTABLE_ISALLOC(self->pd_directory[THIS_PDIR_BASE/PDTABLE_REPRSIZE]));
 assert(PDIR_TABLE_COUNT == PD_TABLE_ENTRY_COUNT);
 selfmap_table  = PDTABLE_GETPTEV(self->pd_directory[THIS_PDIR_BASE/PDTABLE_REPRSIZE]);
 selfmap_table += KERNEL_BASE/PDTABLE_REPRSIZE;
 iter           = self->pd_directory+(KERNEL_BASE/PDTABLE_REPRSIZE);
 end            = self->pd_directory+(PDIR_TABLE_COUNT);
 for (; iter != end; ++iter,++selfmap_table) {
  assertf(PDTABLE_ISALLOC(*iter),
          "All page tables above 'KERNEL_BASE' must be allocated, but %p...%p isn't",
         ((uintptr_t)((iter-self->pd_directory)  )*PDTABLE_REPRSIZE),
         ((uintptr_t)((iter-self->pd_directory)+1)*PDTABLE_REPRSIZE)-1);
  selfmap_table->pe_data  = (iter->pt_data & ~(PDIR_ATTR_MASK));
  selfmap_table->pe_data |= (PDIR_ATTR_DIRTY|PDIR_ATTR_ACCESSED|
                             PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT);
  /* NOTE: We enable the 'PDIR_ATTR_GLOBAL' flag, because all
   *       page directory tables (except for the last) above
   *      'KERNEL_BASE' are still global (the last is the self-map itself). */
  if likely(iter != end-1) selfmap_table->pe_data |= PDIR_ATTR_GLOBAL;
 }
}
#endif /* CONFIG_PDIR_SELFMAP */


PUBLIC bool KCALL pdir_init(pdir_t *__restrict self) {
 CHECK_HOST_DOBJ(self);
 /* Fill lower memory with unallocated pages. */
 memsetl(self->pd_directory,~PDIR_ATTR_MASK,KERNEL_BASE/PDTABLE_REPRSIZE);
 assert(IS_ALIGNED(KERNEL_GLOBAL_END,PDTABLE_REPRSIZE));

 /* Clone kernel pages into upper memory. */
 memcpy(&self->pd_directory[KERNEL_BASE/PDTABLE_REPRSIZE],
        &pdir_kernel_v.pd_directory[KERNEL_BASE/PDTABLE_REPRSIZE],
      ((KERNEL_GLOBAL_END-KERNEL_BASE)/PDTABLE_REPRSIZE)*sizeof(union pd_table));

#ifdef CONFIG_PDIR_SELFMAP
 /* Allocate the table for the page-directory self-map. */
 { union pd_entry *last_table;
   STATIC_ASSERT(THIS_PDIR_SIZE == PDTABLE_REPRSIZE);
   last_table = (union pd_entry *)page_malloc(PDTABLE_ALLOCSIZE,
                                              PAGEATTR_NONE,PDIR_PAGEZONE);
   if unlikely(last_table == PAGE_ERROR) return false;
   assert(IS_ALIGNED((uintptr_t)last_table,PAGESIZE));
   self->pd_directory[THIS_PDIR_BASE/PDTABLE_REPRSIZE].
         pt_data = (u32)last_table | (PDIR_ATTR_DIRTY|PDIR_ATTR_ACCESSED|
                                      PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE);
   memsetl(last_table,~PDIR_ATTR_MASK,KERNEL_BASE/PDTABLE_REPRSIZE);
   pdir_initialize_selfmap(self);
 }
#endif /* CONFIG_PDIR_SELFMAP */

#if 0
 syslogf(LOG_DEBUG,"%.?[hex]\n",
       ((KERNEL_GLOBAL_END-KERNEL_BASE)/PDTABLE_REPRSIZE)*sizeof(union pd_table),
         &self->pd_directory[KERNEL_BASE/PDTABLE_REPRSIZE]);
#endif
 return true;
}

PUBLIC WUNUSED bool KCALL
pdir_load_copy(pdir_t *__restrict self, pdir_t const *__restrict existing) {
#ifdef CONFIG_PDIR_SELFMAP
 union pd_entry *dst_selfmap;
#endif
 union pd_table *dst;
 union pd_table const *src,*end;
 dst = &self->pd_directory[0];
 src = &existing->pd_directory[0];
 end = src+(KERNEL_BASE/PDTABLE_REPRSIZE);
#ifdef CONFIG_PDIR_SELFMAP
 assert(PDTABLE_ISALLOC(self->pd_directory[THIS_PDIR_BASE/PDTABLE_REPRSIZE]));
 dst_selfmap = PDTABLE_GETPTEV(self->pd_directory[THIS_PDIR_BASE/PDTABLE_REPRSIZE]);
#endif
 for (; src != end; ++src,++dst) {
  if (PDTABLE_ISVALID(*src)) {
   /* Must copy the page table. */
   PHYS union pd_entry *table;
   table = (PHYS union pd_entry *)page_malloc(PDTABLE_ALLOCSIZE,PAGEATTR_NONE,MZONE_ANY);
   if unlikely(table == PAGE_ERROR) goto err_nomem;
   memcpy(table,PDTABLE_GETPTEV(*src),PDTABLE_ALLOCSIZE);
   dst->pt_data = (u32)table | (src->pt_data & PDIR_ATTR_MASK);
#ifdef CONFIG_PDIR_SELFMAP
   assert(IS_ALIGNED((uintptr_t)table,PDIR_ATTR_MASK+1));
   dst_selfmap->pe_data = (u32)table | (PDIR_ATTR_DIRTY|PDIR_ATTR_ACCESSED|
                                        PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT);
#endif
  } else {
   /* Simply alias data + attributes. */
   dst->pt_data = src->pt_data;
  }
#ifdef CONFIG_PDIR_SELFMAP
  ++dst_selfmap;
#endif
 }
 return true;
err_nomem:
 /* Free all page tables that we had already allocated. */
 while (src-- != &existing->pd_directory[0]) {
  --dst;
  if (PDTABLE_ISVALID(*dst))
      page_free((ppage_t)PDTABLE_GETPTEV(*dst),PDTABLE_ALLOCSIZE);
 }
 return false;
}

PUBLIC void KCALL pdir_fini(pdir_t *__restrict self) {
 union pd_table *iter,*end;
 PHYS union pd_entry *entry;
 CHECK_HOST_DOBJ(self);
 end = (iter = self->pd_directory)+(KERNEL_BASE/PDTABLE_REPRSIZE);
 for (; iter != end; ++iter) {
  entry = PDTABLE_GETPTEV(*iter);
  if (entry != (PHYS union pd_entry *)(~PDIR_ATTR_MASK) &&
    !(iter->pt_attr&PDIR_ATTR_4MIB))
      page_free((ppage_t)entry,PDTABLE_ALLOCSIZE);
 }
}

PRIVATE size_t KCALL pdir_reqbytes_for_change(pdir_t *__restrict self, VIRT ppage_t start, PAGE_ALIGNED size_t n_bytes);


#ifdef CONFIG_DEBUG
/* Check if a given 'page' is mapped with the specified page directory 'self' */
PRIVATE bool KCALL pdir_ismapped(pdir_t *__restrict self, VIRT ppage_t page) {
 union pd_table table;
 union pd_entry entry;
 table = self->pd_directory[PDIR_DINDEX(page)];
 if (PDTABLE_ISMAP(table)) return true;
 if (!PDTABLE_ISALLOC(table)) return false;
 entry = PDTABLE_GETPTEV(table)[PDIR_TINDEX(page)];
 return PDENTRY_ISPRESENT(entry);
}
#endif /* CONFIG_DEBUG */


#ifdef CONFIG_PDIR_SELFMAP
PRIVATE void KCALL
pdir_table_changed(pdir_t *__restrict self,
                   uintptr_t table_index) {
 union pd_table  new_table;
 union pd_entry *self_entry;
 CHECK_HOST_DOBJ(self);
 assertf(table_index < PDIR_TABLE_COUNT,"Invalid table index: %Iu",table_index);
 assertf(table_index != THIS_PDIR_BASE/PDTABLE_REPRSIZE,
         "The page-directory self-map table must never be changed!");
 assert(PDTABLE_ISALLOC(self->pd_directory[THIS_PDIR_BASE/PDTABLE_REPRSIZE]));
 new_table  = self->pd_directory[table_index];
 self_entry = &PDTABLE_GETPTEV(self->pd_directory[THIS_PDIR_BASE/PDTABLE_REPRSIZE])[table_index];
 if (PDTABLE_ISALLOC(new_table))
  self_entry->pe_data = (u32)PDTABLE_GETPTEV(new_table) | (PDIR_ATTR_DIRTY|PDIR_ATTR_ACCESSED|
                                                           PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT);
 else {
  self_entry->pe_data = ~PDIR_ATTR_MASK;
 }

 /* NOTE: No need to flush. - This shouldn't be our page directory. */
#if 1 /* TODO: Remove this. */
 pdir_flush(self_entry,sizeof(union pd_entry));
#endif
}
#else /* CONFIG_PDIR_SELFMAP */
#define pdir_table_changed(self,table_index) (void)0
#endif /* !CONFIG_PDIR_SELFMAP */

/* Convert a mapping to a table in 'self', using 'dyn_page' as dynamic memory. */
PRIVATE void KCALL
pd_table_map2tbl(union pd_table *self, PHYS ppage_t dyn_page) {
 union pd_entry *iter,*end; ppage_t phys_addr;
 pdir_attr_t attr;
 assertf(IS_ALIGNED((uintptr_t)dyn_page,PAGESIZE),"%p",dyn_page);
 attr = self->pt_data & (PDIR_ATTR_MASK & ~(PDIR_ATTR_4MIB));
 phys_addr = PDTABLE_GETMAP(*self);
 end = (iter = (union pd_entry *)dyn_page)+PTTABLE_ARRAYSIZE;
 for (; iter != end; ++iter) {
  iter->pe_data = (u32)phys_addr | attr;
  ++phys_addr;
 }
 self->pt_data = (u32)dyn_page | attr;
 assert(PDTABLE_ISALLOC(*self));
}

/* Check if, and so so when, a table can be converted to a map, as is possible
 * when all virtual memory shares the same properties and is mapped linearly. */
PRIVATE void KCALL
pd_table_tbl2map(union pd_table *__restrict self) {
 assert(PDTABLE_ISVALID(*self));

 /* TODO */
}

PRIVATE void KCALL
pdir_mprotect_one(struct mscatter *dynmem, pdir_t *__restrict self,
                  VIRT ppage_t addr, pdir_attr_t flags) {
 union pd_table *table;
 union pd_entry *entry;
 if (addr_isvirt(addr)) flags |= PDIR_ATTR_GLOBAL;
 assert(!(flags&~PDIR_ATTR_MASK));
 table = &self->pd_directory[PDIR_DINDEX(addr)];
 if (PDTABLE_ISMAP(*table)) {
  pd_table_map2tbl(table,mscatter_takeone(dynmem));
  pdir_table_changed(self,PDIR_DINDEX(addr));
 } else if (!PDTABLE_ISALLOC(*table)) return;
 entry = &PDTABLE_GETPTEV(*table)[PDIR_TINDEX(addr)];
 entry->pe_attr &= ~PDIR_ATTR_MASK;
 entry->pe_attr |= flags;
 table->pt_attr |= flags;
}
PRIVATE void KCALL
pdir_mprotect_tbl(struct mscatter *dynmem, pdir_t *__restrict self,
                  VIRT ptable_t addr, pdir_attr_t flags) {
 union pd_table *table;
 if (addr_isvirt(addr)) flags |= PDIR_ATTR_GLOBAL;
 assert(!(flags&~PDIR_ATTR_MASK));
 table = &self->pd_directory[PDIR_DINDEX(addr)];
 if (PDTABLE_ISMAP(*table)) {
  table->pt_attr &= ~PDIR_ATTR_MASK;
  table->pt_attr |= flags;
 } else if (PDTABLE_ISALLOC(*table)) {
  union pd_entry *entry,*end;
  end = (entry = PDTABLE_GETPTEV(*table))+PTTABLE_ARRAYSIZE;
  for (; entry != end; ++entry) {
   entry->pe_data &= ~PDIR_ATTR_MASK;
   entry->pe_attr |= flags;
  }
  if (PDIR_ATTR_ALLOW_4MIB(addr))
      pd_table_tbl2map(table);
  table->pt_attr |= flags;
 }
}

PRIVATE size_t
pdir_reqbytes_for_mprotect(pdir_t *__restrict self, VIRT ppage_t start,
                           PAGE_ALIGNED size_t n_bytes, pdir_attr_t flags) {
 unsigned int table_min,table_max; size_t result = 0;
 union pd_table table;
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)n_bytes,PAGESIZE));
 if unlikely(!n_bytes) return 0;
 table_min = PDIR_DINDEX(FLOOR_ALIGN((uintptr_t)start,PDTABLE_REPRSIZE));
 table_max = PDIR_DINDEX(FLOOR_ALIGN((uintptr_t)start+(n_bytes-1),PDTABLE_REPRSIZE));
 assert(table_min <= table_max);
 /* NOTE: mprotect() only needs to allocate tables
  *       if the new protection differs from the old. */
 if (table_min != table_max) {
  table = self->pd_directory[table_max];
  if (!PDTABLE_ISALLOC(table) &&
     (table.pt_attr&PDIR_ATTR_MASK) != flags)
      result += PDTABLE_ALLOCSIZE;
 }
 table = self->pd_directory[table_min];
 if (!PDTABLE_ISALLOC(table) &&
    (table.pt_attr&PDIR_ATTR_MASK) != flags)
     result += PDTABLE_ALLOCSIZE;
 return result;
}

#define PDIR_ISKERNEL(self) ((self) == &pdir_kernel || (self) == &pdir_kernel_v)

FUNDEF ssize_t KCALL
pdir_mprotect(pdir_t *__restrict self, VIRT ppage_t start,
              size_t n_bytes, pdir_attr_t flags) {
 ssize_t result; size_t reqmem;
 struct mscatter dynmem;
 ppage_t base_start = start;
 pdir_attr_t prot_flags;
 CHECK_HOST_DOBJ(self);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 result = n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 assertf(PDIR_ISKERNEL(self) || !n_bytes || !addr_isvirt((uintptr_t)start+n_bytes-1),
         "Virtual addresses may only be mapped within the kernel page directory (%p...%p)",
        (uintptr_t)start,(uintptr_t)start+n_bytes-1);
 assert((uintptr_t)start+n_bytes == 0 ||
        (uintptr_t)start+n_bytes >= (uintptr_t)start);
#ifdef CONFIG_DEBUG
 { ppage_t iter,end;
   iter = start,end = (ppage_t)((uintptr_t)start+n_bytes);
   for (; iter != end; ++iter) assertf(pdir_ismapped(self,iter),"Unmapped page at %p",iter);
 }
#endif
 prot_flags = (flags&PDIR_ATTR_MASK);
 reqmem = pdir_reqbytes_for_mprotect(self,start,n_bytes,prot_flags);
 if (!page_malloc_scatter(&dynmem,reqmem,PAGESIZE,PAGEATTR_NONE,PDIR_PAGEZONE))
      return -ENOMEM;
 while (n_bytes && !IS_ALIGNED((uintptr_t)start,PDTABLE_REPRSIZE)) {
  pdir_mprotect_one(&dynmem,self,start,prot_flags);
  ++start,n_bytes -= PAGESIZE;
 }
 while (n_bytes >= PDTABLE_REPRSIZE) {
  pdir_mprotect_tbl(&dynmem,self,start,prot_flags);
  *(uintptr_t *)&start += PDTABLE_REPRSIZE;
  n_bytes -= PDTABLE_REPRSIZE;
 }
 while (n_bytes) {
  pdir_mprotect_one(&dynmem,self,start,prot_flags);
  ++start,n_bytes -= PAGESIZE;
 }
 assertf(!dynmem.m_next && !dynmem.m_size,
         "Not all dynamic pages were used!");
 /* Flush modified TLB entries. */
 if (!(flags&PDIR_FLAG_NOFLUSH))
       pdir_flush(base_start,result);
 return (size_t)result;
}

PUBLIC bool KCALL
pdir_maccess(pdir_t const *__restrict self,
             VIRT void const *addr, size_t n_bytes,
             pdir_attr_t flags) {
 union pd_table table;
 union pd_entry entry;
 /* Align to full pages. */
 n_bytes += (uintptr_t)addr & (PAGESIZE-1);
 *(uintptr_t *)&addr &= PAGESIZE-1;
 while (n_bytes) {
  table = self->pd_directory[PDIR_DINDEX(addr)];
  if ((table.pt_attr&flags) != flags) return false;
  if (!PDTABLE_ISVALID(table)) {
   /* Check entire page table. */
   n_bytes += (uintptr_t)addr & (PDTABLE_REPRSIZE-1);
   if (n_bytes <= PDTABLE_REPRSIZE) break;
   n_bytes -= PDTABLE_REPRSIZE;
   *(uintptr_t *)&addr &= PDTABLE_REPRSIZE-1;
   *(uintptr_t *)&addr += PDTABLE_REPRSIZE;
   continue;
  }
next_page:
  entry = PDTABLE_GETPTEV(table)[PDIR_TINDEX(addr)];
  if ((entry.pe_attr&flags) != flags) return false;
  if (n_bytes <= PAGESIZE) break;
  n_bytes -= PAGESIZE;
  *(uintptr_t *)&addr += PAGESIZE;
#define TABLEPAGE_MASK  ((PDTABLE_REPRSIZE-1)&~(PAGESIZE-1))
  /* Don't re-load the table when we havn't entered another. */
  if ((uintptr_t)addr&TABLEPAGE_MASK) goto next_page;
 }
 return true;
}

PUBLIC bool KCALL
pdir_maccess_addr(pdir_t *__restrict self,
                  VIRT void const *addr, pdir_attr_t flags) {
 union pd_table table;
 union pd_entry entry;
 table = self->pd_directory[PDIR_DINDEX(addr)];
 if ((table.pt_attr&flags) != flags) return false;
 if (PDTABLE_ISMAP(table)) return true;
 if (!PDTABLE_ISVALID(table)) return true;
 entry = PDTABLE_GETPTEV(table)[PDIR_TINDEX(addr)];
 return (entry.pe_attr&flags) == flags;
}


PRIVATE void KCALL
pdir_mmap_one(struct mscatter *dynmem, pdir_t *__restrict self,
              VIRT ppage_t addr, PHYS ppage_t target, pdir_attr_t flags) {
 union pd_table *table;
 union pd_entry *entry;
 if (addr_isvirt(addr)) flags |= PDIR_ATTR_GLOBAL;
 assert(!(flags&~PDIR_ATTR_MASK));
 table = &self->pd_directory[PDIR_DINDEX(addr)];
 if (PDTABLE_ISMAP(*table)) {
  pd_table_map2tbl(table,mscatter_takeone(dynmem));
  pdir_table_changed(self,PDIR_DINDEX(addr));
 } else if (!PDTABLE_ISALLOC(*table)) {
  PHYS ppage_t map_page = mscatter_takeone(dynmem);
  assert(!addr_isvirt(addr));
  memsetl(map_page,~PDIR_ATTR_MASK,PTTABLE_ARRAYSIZE);
  table->pt_data = (u32)map_page | flags;
  pdir_table_changed(self,PDIR_DINDEX(addr));
 }
 assert(PDTABLE_ISALLOC(*table));
 entry = &PDTABLE_GETPTEV(*table)[PDIR_TINDEX(addr)];
 entry->pe_data  = (uintptr_t)target|flags;
 entry->pe_attr |= flags;
 table->pt_attr |= flags;
}
PRIVATE void KCALL
pdir_mmap_tbl(pdir_t *__restrict self, VIRT ppage_t addr,
              PHYS ppage_t target, pdir_attr_t flags) {
 union pd_table *table;
 assert(!(flags&~PDIR_ATTR_MASK));
 table = &self->pd_directory[PDIR_DINDEX(addr)];
 if (PDIR_ATTR_ALLOW_4MIB(addr)) {
  bool was_allocated;
  /* Free a previously allocated table. */
  was_allocated = PDTABLE_ISALLOC(*table);
  if (was_allocated)
      page_free((ppage_t)PDTABLE_GETPTEV(*table),PDTABLE_ALLOCSIZE);
  table->pt_data = (u32)target|flags|PDIR_ATTR_4MIB;
  pdir_table_changed(self,PDIR_DINDEX(addr));
 } else {
  union pd_entry *iter,*end;
  assert(PDTABLE_ISALLOC(*table));
  assert(addr_isvirt(addr));
  assert(!(flags&PDIR_ATTR_4MIB));
  flags |= PDIR_ATTR_GLOBAL;
  /* Map each individual entry. */
  end = (iter = PDTABLE_GETPTEV(*table))+PTTABLE_ARRAYSIZE;
  for (; iter != end; ++iter) { iter->pe_data = (u32)target|flags; ++target; }
 }
}

PUBLIC errno_t KCALL
pdir_mmap(pdir_t *__restrict self, VIRT ppage_t start,
          size_t n_bytes, PHYS ppage_t target, pdir_attr_t flags) {
 /*ppage_t iter,end;*/ size_t reqmem;
 VIRT ppage_t virt_iter = start;
 struct mscatter dynmem;
 pdir_attr_t prot_flags = flags&PDIR_ATTR_MASK;
 CHECK_HOST_DOBJ(self);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)target,PAGESIZE));
 if unlikely(!n_bytes) return -EOK;
 assertf(PDIR_ISKERNEL(self) || !addr_isvirt((uintptr_t)start+n_bytes-1),
         "Virtual addresses may only be mapped within the kernel page directory");
 reqmem = pdir_reqbytes_for_change(self,start,n_bytes);
 if (!page_malloc_scatter(&dynmem,reqmem,PAGESIZE,PAGEATTR_NONE,PDIR_PAGEZONE))
      return -ENOMEM;
 while (n_bytes && !IS_ALIGNED((uintptr_t)virt_iter,PDTABLE_REPRSIZE)) {
  pdir_mmap_one(&dynmem,self,virt_iter,target,prot_flags);
  ++virt_iter,++target,n_bytes -= PAGESIZE;
 }
 while (n_bytes >= PDTABLE_REPRSIZE) {
  pdir_mmap_tbl(self,virt_iter,target,prot_flags);
  *(uintptr_t *)&virt_iter += PDTABLE_REPRSIZE;
  *(uintptr_t *)&target += PDTABLE_REPRSIZE;
  n_bytes -= PDTABLE_REPRSIZE;
 }
 while (n_bytes) {
  pdir_mmap_one(&dynmem,self,virt_iter,target,prot_flags);
  ++virt_iter,++target,n_bytes -= PAGESIZE;
 }
 assertf(!dynmem.m_next && !dynmem.m_size,
         "Not all dynamic pages were used!");
 /* Try to convert the start/end to a table. */
 if (PDIR_ATTR_ALLOW_4MIB(start)) {
  union PACKED pd_table *table;
  table = &self->pd_directory[PDIR_DINDEX((uintptr_t)start)];
  if (PDTABLE_ISVALID(*table)) pd_table_tbl2map(table);
 }
 { uintptr_t max_addr = (uintptr_t)start+n_bytes-1;
   if (PDIR_ATTR_ALLOW_4MIB(max_addr) &&
       /* Don't re-try if the last table is the same as the first. */
       PDIR_DINDEX(start) != PDIR_DINDEX(max_addr)) {
    union PACKED pd_table *table;
    table = &self->pd_directory[PDIR_DINDEX(max_addr)];
    if (PDTABLE_ISVALID(*table)) pd_table_tbl2map(table);
   }
 }

 /* Flush modified TLB entries. */
 if (!(flags&PDIR_FLAG_NOFLUSH))
       pdir_flush(start,n_bytes);
 return -EOK;
}

#ifdef CONFIG_BUILDING_KERNEL_CORE
INTERN ATTR_FREETEXT errno_t KCALL
pdir_mmap_early(pdir_t *__restrict self, VIRT ppage_t start,
                size_t n_bytes, PHYS ppage_t target, pdir_attr_t flags) {
 /*ppage_t iter,end;*/ size_t reqmem;
 VIRT ppage_t virt_iter = start;
 struct mscatter dynmem;
 pdir_attr_t prot_flags = flags&PDIR_ATTR_MASK;
 CHECK_HOST_DOBJ(self);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)target,PAGESIZE));
 if unlikely(!n_bytes) return -EOK;
 assertf(PDIR_ISKERNEL(self) || !addr_isvirt((uintptr_t)start+n_bytes-1),
         FREESTR("Virtual addresses may only be mapped within the kernel page directory"));
 reqmem = pdir_reqbytes_for_change(self,start,n_bytes);
 if (!page_malloc_scatter(&dynmem,reqmem,PAGESIZE,PAGEATTR_NONE,PDIR_PAGEZONE))
      return -ENOMEM;
 while (n_bytes && !IS_ALIGNED((uintptr_t)virt_iter,PDTABLE_REPRSIZE)) {
  pdir_mmap_one(&dynmem,self,virt_iter,target,prot_flags);
  ++virt_iter,++target,n_bytes -= PAGESIZE;
 }
 while (n_bytes >= PDTABLE_REPRSIZE) {
  union pd_table *table; bool was_allocated;
  table = &self->pd_directory[PDIR_DINDEX(virt_iter)];
  /* Free a previously allocated table. */
  was_allocated = PDTABLE_ISALLOC(*table);
  if (was_allocated)
      page_free((ppage_t)PDTABLE_GETPTEV(*table),PDTABLE_ALLOCSIZE);
  table->pt_data = (u32)target|prot_flags|PDIR_ATTR_4MIB;
  pdir_table_changed(self,PDIR_DINDEX(virt_iter));
  *(uintptr_t *)&virt_iter += PDTABLE_REPRSIZE;
  *(uintptr_t *)&target += PDTABLE_REPRSIZE;
  n_bytes -= PDTABLE_REPRSIZE;
 }
 while (n_bytes) {
  pdir_mmap_one(&dynmem,self,virt_iter,target,prot_flags);
  ++virt_iter,++target,n_bytes -= PAGESIZE;
 }
 assertf(!dynmem.m_next && !dynmem.m_size,
         FREESTR("Not all dynamic pages were used!"));
 /* Try to convert the start/end to a table. */
 { union PACKED pd_table *table;
   table = &self->pd_directory[PDIR_DINDEX((uintptr_t)start)];
   if (PDTABLE_ISVALID(*table)) pd_table_tbl2map(table);
 }
 { uintptr_t max_addr = (uintptr_t)start+n_bytes-1;
   if (PDIR_DINDEX(start) != PDIR_DINDEX(max_addr)) {
    union PACKED pd_table *table;
    table = &self->pd_directory[PDIR_DINDEX(max_addr)];
    if (PDTABLE_ISVALID(*table)) pd_table_tbl2map(table);
   }
 }

 /* Flush modified TLB entries. */
 if (!(flags&PDIR_FLAG_NOFLUSH))
       pdir_flush(start,n_bytes);
 return -EOK;
}
#endif


PRIVATE size_t KCALL
pdir_reqbytes_for_change(pdir_t *__restrict self, VIRT ppage_t start,
                         PAGE_ALIGNED size_t n_bytes) {
 unsigned int table_min,table_max; size_t result = 0;
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)n_bytes,PAGESIZE));
 if unlikely(!n_bytes) return 0;
 table_min = PDIR_DINDEX(FLOOR_ALIGN((uintptr_t)start,PDTABLE_REPRSIZE));
 table_max = PDIR_DINDEX(FLOOR_ALIGN((uintptr_t)start+(n_bytes-1),PDTABLE_REPRSIZE));
 assert(table_min <= table_max);
 if ( table_min != table_max &&
     !PDTABLE_ISALLOC(self->pd_directory[table_min]) &&
     !IS_ALIGNED((uintptr_t)start,PDTABLE_REPRSIZE))
      result += PDTABLE_ALLOCSIZE;
 if (!PDTABLE_ISALLOC(self->pd_directory[table_max]) &&
    (!IS_ALIGNED((uintptr_t)start+n_bytes,PDTABLE_REPRSIZE) ||
    (!IS_ALIGNED((uintptr_t)start,PDTABLE_REPRSIZE) && table_min == table_max)))
      result += PDTABLE_ALLOCSIZE;
 return result;
}
PRIVATE void KCALL
pdir_munmap_one(struct mscatter *dynmem, pdir_t *__restrict self,
                VIRT ppage_t addr) {
 union pd_table *table;
 union pd_entry *entry;
 table = &self->pd_directory[PDIR_DINDEX(addr)];
 if (PDTABLE_ISMAP(*table)) {
  pd_table_map2tbl(table,mscatter_takeone(dynmem));
  pdir_table_changed(self,PDIR_DINDEX(addr));
 } else if (!PDTABLE_ISALLOC(*table)) return;
 entry = &PDTABLE_GETPTEV(*table)[PDIR_TINDEX(addr)];
 entry->pe_data = ~PDIR_ATTR_MASK;
}
PRIVATE void KCALL
pdir_munmap_tbl(struct mscatter *dynmem, pdir_t *__restrict self, VIRT ptable_t addr) {
 union pd_table *table; bool was_allocated;
 table = &self->pd_directory[PDIR_DINDEX(addr)];
 /* Only mark kernel tables as non-present. - Don't actually free them! */
 if (!PDIR_ATTR_ALLOW_4MIB(addr)) {
  union pd_entry *iter,*end;
  assert(PDTABLE_ISALLOC(*table));
  /* Unmap each individual entry. */
  end = (iter = PDTABLE_GETPTEV(*table))+PTTABLE_ARRAYSIZE;
  for (; iter != end; ++iter) iter->pe_data = ~PDIR_ATTR_MASK;
  return;
 }
 /* Free an allocate page table. */
 was_allocated = PDTABLE_ISALLOC(*table);
 if (was_allocated)
     page_free((ppage_t)PDTABLE_GETPTEV(*table),PDTABLE_ALLOCSIZE);
 table->pt_data = ~PDIR_ATTR_MASK;
 if (was_allocated) pdir_table_changed(self,PDIR_DINDEX(addr));
}

PUBLIC errno_t KCALL
pdir_munmap(pdir_t *__restrict self, VIRT ppage_t start,
            size_t n_bytes, pdir_attr_t flags) {
 size_t reqmem; struct mscatter dynmem;
 CHECK_HOST_DOBJ(self);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 if unlikely(!n_bytes) return 0;
 assertf(PDIR_ISKERNEL(self) || !addr_isvirt((uintptr_t)start+n_bytes-1),
         "Virtual addresses may only be mapped within the kernel page directory");
 assertf((uintptr_t)start+n_bytes == 0 ||
         (uintptr_t)start+n_bytes > (uintptr_t)start,
          "start   = %p\n"
          "n_bytes = %p\n",
          start,n_bytes);
 reqmem = pdir_reqbytes_for_change(self,start,n_bytes);
 if (!page_malloc_scatter(&dynmem,reqmem,PAGESIZE,PAGEATTR_NONE,PDIR_PAGEZONE))
      return -ENOMEM;
 while (n_bytes && !IS_ALIGNED((uintptr_t)start,PDTABLE_REPRSIZE)) {
  assert(IS_ALIGNED(n_bytes,PAGESIZE));
  pdir_munmap_one(&dynmem,self,start);
  ++start,n_bytes -= PAGESIZE;
 }
 while (n_bytes >= PDTABLE_REPRSIZE) {
  pdir_munmap_tbl(&dynmem,self,start);
  *(uintptr_t *)&start += PDTABLE_REPRSIZE;
  n_bytes -= PDTABLE_REPRSIZE;
 }
 while (n_bytes) {
  assert(IS_ALIGNED(n_bytes,PAGESIZE));
  pdir_munmap_one(&dynmem,self,start);
  ++start,n_bytes -= PAGESIZE;
 }
 assertf(!dynmem.m_next && !dynmem.m_size,
         "Not all dynamic pages were used (%Iu)!",dynmem.m_size);
 /* Flush modified TLB entries. */
 if (!(flags&PDIR_FLAG_NOFLUSH))
       pdir_flush(start,n_bytes);
/*end:*/
 return -EOK;
}

struct pdr_print_pck {
 pformatprinter printer;
 void          *closure;
};

PRIVATE ssize_t KCALL
pdir_println(VIRT ppage_t v_addr, PHYS ppage_t p_addr,
             size_t n_bytes, pdir_attr_t attr, void *data) {
 if (!(attr&PDIR_ATTR_PRESENT) && (uintptr_t)p_addr == ~PDIR_ATTR_MASK) return 0;
 if ((uintptr_t)p_addr+n_bytes < (uintptr_t)p_addr)
      n_bytes = 0-(uintptr_t)p_addr;
 return format_printf(((struct pdr_print_pck *)data)->printer,
                      ((struct pdr_print_pck *)data)->closure,
                      "%p...%p --> %p...%p [%c%c%c] (%.3Ix)\n",
                      v_addr,(uintptr_t)v_addr+n_bytes-1,
                      p_addr,(uintptr_t)p_addr+n_bytes-1,
                      attr&PDIR_ATTR_USER ? 'U' : '-',
                      attr&PDIR_ATTR_WRITE ? 'W' : '-',
                      attr&PDIR_ATTR_PRESENT ? 'P' : '-',
                      attr);
}

PUBLIC ssize_t KCALL
pdir_print(pdir_t *__restrict self,
           pformatprinter printer,
           void *closure) {
 struct pdr_print_pck pck;
 pck.printer = printer;
 pck.closure = closure;
 return pdir_enum(self,&pdir_println,&pck);
}

#define PDIR_ENUM_MASK  (PDIR_ATTR_MASK&~(PDIR_ATTR_DIRTY|PDIR_ATTR_ACCESSED))

PUBLIC ssize_t KCALL
pdir_enum(pdir_t *__restrict self,
          pdirwalker walker, void *closure) {
 union pd_table *iter,*end;
 ssize_t temp,result = 0;
 VIRT ppage_t prev_vaddr = NULL;
 PHYS ppage_t prev_paddr = NULL;
 PHYS ppage_t next_paddr = NULL;
 pdir_attr_t  prev_attr  = 0;
 PHYS ppage_t curr_paddr;
 VIRT ppage_t next_vaddr;
 pdir_attr_t  curr_attr;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_TEXT(walker,1);
 end = (iter = self->pd_directory)+COMPILER_LENOF(self->pd_directory);
 for (; iter != end; ++iter) {
  if (!(iter->pt_attr&PDIR_ATTR_PRESENT) || PDTABLE_ISMAP(*iter)) {
   curr_attr  = iter->pt_attr & PDIR_ENUM_MASK;
   curr_paddr = PDTABLE_GETMAP(*iter);
   if (curr_paddr != next_paddr || prev_attr != curr_attr) {
    next_vaddr = (ppage_t)((iter-self->pd_directory)*PDTABLE_REPRSIZE);
    if (next_vaddr != prev_vaddr) {
     temp = (*walker)(prev_vaddr,prev_paddr,
                     (uintptr_t)next_vaddr-(uintptr_t)prev_vaddr,
                      prev_attr,closure);
     if (temp < 0) return temp;
     result += temp;
    }
    prev_attr  = curr_attr;
    prev_vaddr = next_vaddr;
    prev_paddr = curr_paddr;
   }
   next_paddr = (ppage_t)((uintptr_t)curr_paddr+PDTABLE_REPRSIZE);
  } else {
   union pd_entry *table,*table_end;
   assert(PDTABLE_ISALLOC(*iter));
   table = (union pd_entry *)PDTABLE_GETPTEV(*iter);
   table_end = table+PTTABLE_ARRAYSIZE;
   for (; table != table_end; ++table) {
    curr_attr  = table->pe_attr & PDIR_ENUM_MASK;
    curr_paddr = PDENTRY_GETMAP(*table);
    if (curr_paddr != next_paddr || prev_attr != curr_attr) {
     next_vaddr = (ppage_t)(((iter-self->pd_directory)*PDTABLE_REPRSIZE)+
                            ((table-PDTABLE_GETPTEV(*iter))*PDENTRY_REPRSIZE));
     if (next_vaddr != prev_vaddr) {
      temp = (*walker)(prev_vaddr,prev_paddr,
                      (uintptr_t)next_vaddr-(uintptr_t)prev_vaddr,
                       prev_attr,closure);
      if (temp < 0) return temp;
      result += temp;
     }
     prev_attr  = curr_attr;
     prev_vaddr = next_vaddr;
     prev_paddr = curr_paddr;
    }
    next_paddr = (ppage_t)((uintptr_t)curr_paddr+PDENTRY_REPRSIZE);
   }
  }
 }
 temp = (*walker)(prev_vaddr,prev_paddr,
                 (uintptr_t)0-(uintptr_t)prev_vaddr,
                  prev_attr,closure);
 if (temp < 0) return temp;
 result += temp;
 return result;
}


INTDEF byte_t pdir_flush_386[];
INTDEF byte_t pdir_flush_386_end[];

GLOBAL_ASM(
L(.section .text.free                     )
L(PRIVATE_ENTRY(pdir_flush_386)           )
L(    movl %cr3, %eax                     )
L(    movl %eax, %cr3                     )
L(    ret                                 )
L(SYM_END(pdir_flush_386)                 )
L(pdir_flush_386_end:                     )
L(.previous                               )
);

GLOBAL_ASM(
L(.section .text                          )
L(PUBLIC_ENTRY(pdir_flush)                )
/* NOTE: When the host is a 386, we'll copy data from 'pdir_flush_386' here. */
/* HINT: start   == %ecx */
/* HINT: n_bytes == %edx */
L(    cmp $(PAGESIZE*64), %edx            )
L(    jae 1f                              )
L(    invlpg (%ecx)                       )
L(    subl $(PAGESIZE), %edx              )
L(    jo 2f                               ) /* Stop if n_bytes underflowed. */
L(    addl $(PAGESIZE), %ecx              )
L(    jmp pdir_flush                      )
L(1:  movl %cr3, %eax                     ) /* Do a regular, full flush for larger quantities. */
L(    movl %eax, %cr3                     )
L(2:  ret                                 )
L(SYM_END(pdir_flush)                     )
L(.previous                               )
);

INTERN ATTR_SECTION(".data.phys")
       ATTR_ALIGNED(PAGESIZE)
PHYS struct mman __mman_kernel_p = {
    /* After boot, memory is mapped like this:
     * >> 0x00000000..0xbfffffff  -->  0x00000000..0xbfffffff
     * >> 0xc0000000..0xffffffff  -->  0x00000000..0x3fffffff
     * Aka: Physical memory pointers is mirrored in upper memory.
     * Once paging has been fully initialized, the lower
     * mappings from 0x00000000 to 0xbfffffff are deleted.
     * HINT: 0x83 == PDIR_ATTR_4MIB|PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE */
    .m_pdir = {
        .pd_directory = {
#define PD(phys) \
           {phys+0x00000083},{phys+0x00400083},{phys+0x00800083},{phys+0x00c00083}, \
           {phys+0x01000083},{phys+0x01400083},{phys+0x01800083},{phys+0x01c00083}, \
           {phys+0x02000083},{phys+0x02400083},{phys+0x02800083},{phys+0x02c00083}, \
           {phys+0x03000083},{phys+0x03400083},{phys+0x03800083},{phys+0x03c00083}, \
           {phys+0x04000083},{phys+0x04400083},{phys+0x04800083},{phys+0x04c00083}, \
           {phys+0x05000083},{phys+0x05400083},{phys+0x05800083},{phys+0x05c00083}, \
           {phys+0x06000083},{phys+0x06400083},{phys+0x06800083},{phys+0x06c00083}, \
           {phys+0x07000083},{phys+0x07400083},{phys+0x07800083},{phys+0x07c00083},
            PD(0x00000000) PD(0x08000000) PD(0x10000000) PD(0x18000000) /* 0x18000000 */
            PD(0x20000000) PD(0x28000000) PD(0x30000000) PD(0x38000000) /* 0x38000000 */
            PD(0x40000000) PD(0x48000000) PD(0x50000000) PD(0x58000000) /* 0x58000000 */
            PD(0x60000000) PD(0x68000000) PD(0x70000000) PD(0x78000000) /* 0x78000000 */
            PD(0x80000000) PD(0x88000000) PD(0x90000000) PD(0x98000000) /* 0x98000000 */
            PD(0xa0000000) PD(0xa8000000) PD(0xb0000000) PD(0xb8000000) /* 0xb8000000 */
            /* Map lower memory a second time. (NOTE: 0x100 is 'PDIR_ATTR_GLOBAL') */
            PD(0x00000100) PD(0x08000100) PD(0x10000100) PD(0x18000100) /* 0xd8000000 */
            PD(0x20000100) PD(0x28000100) PD(0x30000100) PD(0x38000100) /* 0xf8000000 */
#if KERNEL_BASE != 0xc0000000
#error "FIXME: Fix the bootstrap page table above"
#endif
#undef PD
        },
    },
    .m_ppdir  = &pdir_kernel,
#ifdef CONFIG_DEBUG
    .m_refcnt = 1,
#else
    .m_refcnt = 0x80000001,
#endif
    .m_lock   = OWNER_RWLOCK_INIT,
    .m_tasks_lock = ATOMIC_RWLOCK_INIT,
    .m_tasks  = &inittask,
    .m_inst   = THIS_INSTANCE,
#ifndef CONFIG_NO_LDT
    .m_ldt    = &ldt_empty,
#endif
    .m_uheap  = MMAN_UHEAP_DEFAULT_ADDR,
    .m_ustck  = MMAN_USTCK_DEFAULT_ADDR,
};

GLOBAL_ASM(
/* Define physical/virtual versions of the kernel mman/pdir. */
L(.global mman_kernel, pdir_kernel, pdir_kernel_v               )
L(mman_kernel   = __mman_kernel_p+KERNEL_BASE                   )
L(pdir_kernel   = __mman_kernel_p+MMAN_OFFSETOF_PDIR            )
L(pdir_kernel_v = __mman_kernel_p+KERNEL_BASE+MMAN_OFFSETOF_PDIR)
);




INTERN ATTR_FREETEXT void KCALL
pdir_kernel_transform_tables(void) {
 union pd_table *iter; uintptr_t addr_iter;
 /* until it overflows! */
#if 1
 iter = &pdir_kernel.pd_directory[PDIR_TABLE_COUNT-1];
 for (addr_iter = (0-KERNEL_BASE)-PDTABLE_REPRSIZE;
      addr_iter < (0-KERNEL_BASE);
      addr_iter -= PDTABLE_REPRSIZE,--iter)
#else
 iter = &pdir_kernel.pd_directory[KERNEL_BASE/PDTABLE_REPRSIZE];
 for (addr_iter  = 0; addr_iter < (0-KERNEL_BASE);
      addr_iter += PDTABLE_REPRSIZE,++iter)
#endif
 {
  union pd_entry *table,*table_iter,*table_end;
  uintptr_t local_addr_iter = addr_iter;
#if 1
  if unlikely(PDTABLE_ISALLOC(*iter)) {
   /* Still allocate a new page to improve cache locality. */
   table = (union pd_entry *)page_malloc(PDTABLE_ALLOCSIZE,
                                         PAGEATTR_NONE,PDIR_PAGEZONE);
   if (table == PAGE_ERROR) table = PDTABLE_GETPTEV(*iter);
   else page_free((ppage_t)PDTABLE_GETPTEV(*iter),PDTABLE_ALLOCSIZE);
  } else
#else
  if unlikely(PDTABLE_ISALLOC(*iter)) {
   /* Steal this table. */
   table = PDTABLE_GETPTEV(*iter);
  } else
#endif
  {
   table = (union pd_entry *)page_malloc(PDTABLE_ALLOCSIZE,
                                         PAGEATTR_NONE,PDIR_PAGEZONE);
  }
  if unlikely(table == PAGE_ERROR) {
   syslogf(LOG_MEM|LOG_ERROR,
           FREESTR("[PD] Failed to transform 4Mib kernel page-table for %p...%p: %[errno]\n"),
           local_addr_iter,local_addr_iter+PDTABLE_REPRSIZE-1,ENOMEM);
  } else {
   table_end = (table_iter = table)+PTTABLE_ARRAYSIZE;
   for (; table_iter != table_end; ++table_iter) {
    table_iter->pe_data = local_addr_iter|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT|PDIR_ATTR_GLOBAL;
    local_addr_iter += PAGESIZE;
   }
   /* We use assembly for this assignment, because
    * it actually matters how this data is set.
    * Essentially, we must move a register containing the
    * full generated value directly into the page table. */
   __asm__ __volatile__("movl %1, %0\n"
                        : "=m" (iter->pt_data)
                        : "r" ((uintptr_t)table|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT|
                                                /* NOTE: Mark all shared address tables as dirty+accessed,
                                                 *       so that the CPU won't need to set the bits, and
                                                 *       to prevent redundant and potentially dangerous
                                                 *       data from being copied into user page directories. */
                                                PDIR_ATTR_DIRTY|PDIR_ATTR_ACCESSED|
                                                /* Must set the global bit for all high pages! */
                                                PDIR_ATTR_GLOBAL)
                        : "memory");
  }
 }
}



INTERN ATTR_FREETEXT void KCALL
pdir_kernel_do_unmap(VIRT uintptr_t begin, size_t n_bytes) {
 errno_t error;
#if 0
 syslogf(LOG_DEBUG,FREESTR("[PD] UNMAP(%p...%p)\n"),begin,begin+n_bytes-1);
#endif
 error = pdir_munmap(&pdir_kernel,
                    (ppage_t)begin,n_bytes,
                     PDIR_FLAG_NOFLUSH);
 if (E_ISERR(error)) {
  syslogf(LOG_MEM|LOG_ERROR,
          FREESTR("[PD] Failed to unmap unused kernel mapping %p...%p: %[errno]\n"),
          begin,(uintptr_t)begin+n_bytes-1,-error);
 }
}


/* These mappings must be kept to ensure proper  */
#define PDIR_KERNEL_KEEP_VIRTUAL_PHYSMAPPINGS 1
#define PDIR_KERNEL_PHYS_BEGIN   KERNEL_PHYS_BEGIN
#define PDIR_KERNEL_PHYS_END     KERNEL_PHYS_END

INTERN ATTR_FREETEXT void KCALL
pdir_kernel_unmap(VIRT uintptr_t begin, size_t n_bytes) {
#if 1
 /* Special behavior for physical kernel data, which is
  * unmapped in virtual memory, but kept in physical! */
 if (begin < PDIR_KERNEL_PHYS_BEGIN) {
  /* Free region is located before the kernel. */
  uintptr_t max_size = PDIR_KERNEL_PHYS_BEGIN-begin;
  if (n_bytes > max_size) {
   /* Unmap the virtual portion. */
#if !PDIR_KERNEL_KEEP_VIRTUAL_PHYSMAPPINGS
   pdir_kernel_do_unmap((uintptr_t)phys_to_virt(PDIR_KERNEL_PHYS_BEGIN),
                         MIN(KERNEL_PHYS_SIZE,n_bytes-max_size));
#endif
   n_bytes = max_size;
  }
 } else if (begin < PDIR_KERNEL_PHYS_END) {
  /* Free region begins inside the kernel. */
  uintptr_t reserved_size = PDIR_KERNEL_PHYS_END-begin;
#if !PDIR_KERNEL_KEEP_VIRTUAL_PHYSMAPPINGS
  pdir_kernel_do_unmap((uintptr_t)phys_to_virt(begin),
                        MIN(reserved_size,n_bytes));
#endif
  begin += reserved_size;
  if (n_bytes < reserved_size) n_bytes = 0;
  else n_bytes -= reserved_size;
 }
#endif
 pdir_kernel_do_unmap(begin,n_bytes);
}

INTERN ATTR_FREETEXT void KCALL
pdir_kernel_unmap_mzone(mzone_t zone_id) {
 /* Use memory information, as not to miss anything. */
 struct meminfo const *iter;
 PHYS uintptr_t del_begin,del_end;
 del_begin = MZONE_MIN(zone_id);
 iter      = mem_info[zone_id];
 for (;;) {
  if (iter == NULL)
      del_end = MZONE_MAX(zone_id)+1;
  else {
#if 0
   syslogf(LOG_MEM|LOG_DEBUG,FREESTR("[PD] CORE_RANGE(%p...%p) (zone #%d)\n"),
           iter->mi_start,(uintptr_t)iter->mi_start+iter->mi_size-1,zone_id);
#endif
   del_end = (uintptr_t)iter->mi_start;
  }
  assertf(!del_end || del_end >= del_begin,
          FREESTR("del_begin = %p\n"
                  "del_end   = %p"),
          del_begin,del_end);
  if (del_begin != del_end) {
   //if (zone_id < MZONE_SHARE) {
   // pdir_kernel_unmap((uintptr_t)phys_to_virt(del_begin),
   //                    del_end-del_begin);
   //}
   pdir_kernel_unmap(del_begin,del_end-del_begin);
  }
  if (iter == NULL) break;
  del_begin = (uintptr_t)iter->mi_start+iter->mi_size;
  iter      = iter->mi_next;
 }
}



INTERN ATTR_FREETEXT void KCALL pdir_initialize(void) {
 assert(addr_isphys(&pdir_kernel));
 assert(addr_isvirt(&pdir_kernel_v));
 assert(addr_isvirt(&mman_kernel));

 if (!cpu_is_486()) {
  /* Replace 'pdir_flush' with its fallback counterpart!
   * (The 'invlpg' instruction is only available starting at 486) */
  memcpy((void *)&pdir_flush,pdir_flush_386,
         (size_t)(pdir_flush_386_end-pdir_flush_386));
 }

 /* Transform kernel tables, ensuring level#1 indirection for
  * pages, which in return allow for page sharing with user-space. */
 pdir_kernel_transform_tables();

#ifdef CONFIG_PDIR_SELFMAP
 /* With all page-tables allocated, setup the
  * page-directory self-map for the kernel itself. */
 pdir_initialize_selfmap(&pdir_kernel);
#endif /* CONFIG_PDIR_SELFMAP */

 /* Time to clean up the kernel's own page directory! */
 pdir_kernel_unmap_mzone(MZONE_1MB);
 pdir_kernel_unmap_mzone(MZONE_SHARE);
 pdir_kernel_unmap_mzone(MZONE_NOSHARE);

 /* Unmap all virtual memory past the kernel itself. */
 pdir_kernel_unmap(KERNEL_END,KERNEL_AFTER_END);

 /* Unmap the virtual copy of physical memory before device data.
  * NOTE: Device memory itself must not be unmapped,
  *       as the TTY driver wouldn't work otherwise. */
 pdir_kernel_unmap(KERNEL_BASE,0x000a0000);

#ifdef CONFIG_DEBUG
 /* Make sure that all virtual kernel pages are still allocated! */
 { union pd_table *iter,*end;
   iter = &pdir_kernel.pd_directory[KERNEL_BASE/PDTABLE_REPRSIZE];
   end  = COMPILER_ENDOF(pdir_kernel.pd_directory);
   for (; iter != end; ++iter) {
    assertf(PDTABLE_ISALLOC(*iter),
            FREESTR("Page table at %p should be allocated"),
           (iter-pdir_kernel.pd_directory)*
            PDTABLE_REPRSIZE);
   }
 }
#endif

#if 1
 { ssize_t error;
   /* Change the protection of the kernel's text/rodata segment to read-only. */
   error = pdir_mprotect(&pdir_kernel,
                        (ppage_t)KERNEL_RO_BEGIN,KERNEL_RO_SIZE,
                         PDIR_ATTR_PRESENT|PDIR_ATTR_GLOBAL|PDIR_FLAG_NOFLUSH);
   if (E_ISERR(error)) {
    syslogf(LOG_MEM|LOG_ERROR,
            FREESTR("[PD] Failed to mark kernel text %p...%p as read-only: %[errno]\n"),
            KERNEL_RO_BEGIN,KERNEL_RO_END-1,-error);
   }
#if 1
   error = pdir_mprotect(&pdir_kernel,
                        (ppage_t)__kernel_user_start,(size_t)__kernel_user_size,
                         PDIR_ATTR_USER|PDIR_ATTR_GLOBAL|PDIR_ATTR_PRESENT|PDIR_FLAG_NOFLUSH);
   if (E_ISERR(error)) {
    syslogf(LOG_MEM|LOG_ERROR,
            FREESTR("[PD] Failed to mark user-data %p...%p as shared: %[errno]\n"),
           (uintptr_t)__kernel_user_start,(uintptr_t)__kernel_user_end-1,-error);
   }
#endif
 }
#endif

#if 0
 { union pd_table *iter; uintptr_t addr_iter;
   iter = &pdir_kernel.pd_directory[KERNEL_BASE/PDTABLE_REPRSIZE];
   for (addr_iter = 0; addr_iter < (0-KERNEL_BASE);
        addr_iter += PDTABLE_REPRSIZE,++iter) {
    syslogf(LOG_MEM|LOG_DEBUG,FREESTR("KERNEL_PD[%p] = %p\n"),
            addr_iter,PDTABLE_GETPTEV(*iter));
   }
 }
#endif

 /* Reload the kernel page directory completely. */
 pdir_flushall();

 assert(PDIR_TRANSLATE(&pdir_kernel_v,&pdir_kernel_v) == &pdir_kernel);
 assert(PDIR_TRANSLATE(&pdir_kernel  ,&pdir_kernel_v) == &pdir_kernel);
}

DECL_END

#endif /* !GUARD_KERNEL_CORE_PAGING_C */
