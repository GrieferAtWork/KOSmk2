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
#ifndef GUARD_INCLUDE_KERNEL_PAGING_UTIL_H
#define GUARD_INCLUDE_KERNEL_PAGING_UTIL_H 1

#include <hybrid/compiler.h>
#include <kernel/paging.h>
#include <stdbool.h>

//#define CONFIG_NO_PDIR_SELFMAP

#ifndef CONFIG_NO_PDIR_SELFMAP
#define CONFIG_PDIR_SELFMAP 1
#else
#undef  CONFIG_PDIR_SELFMAP
#endif


#ifndef CONFIG_PDIR_SELFMAP
#include <sched/paging.h>
#else /* !CONFIG_PDIR_SELFMAP */
#include <hybrid/align.h>
#include <hybrid/limits.h>
#endif /* CONFIG_PDIR_SELFMAP */

DECL_BEGIN



#ifdef CONFIG_PDIR_SELFMAP

/* The last page directory table is reserved for
 * self page-directory & table self-mappings,
 * meaning that the current page directory always
 * includes a virtual mapping of itself at `__this_pdir',
 * as well as a mapping of all table entries at `__this_pdir_entries'  */
#define THIS_PDIR_BASE    0xffc00000
#define THIS_PDIR_SIZE    0x00400000

/* The first address past the end of global page directory data
 * (aka. data shared between all page directories) */
#define KERNEL_GLOBAL_END THIS_PDIR_BASE


#ifdef __CC__
struct pdir_entries { union pd_entry _e[PDIR_TABLE_COUNT][PD_TABLE_ENTRY_COUNT]; };
#define THIS_PDIR_ENTRIES                 ((struct pdir_entries *)THIS_PDIR_BASE)->_e
#define THIS_PDIR_ENTRY(d_index,t_index)    THIS_PDIR_ENTRIES[d_index][t_index]

LOCAL KPD PHYS bool KCALL
thispdir_test_writable(VIRT pdir_t *__restrict self, VIRT void *ptr) {
 union pd_table table = self->pd_directory[PDIR_DINDEX(ptr)];
 union pd_entry entry;
 if (PDTABLE_ISMAP(table)) return (table.pt_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE)) ==
                                                 (PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE);
 if (!PDTABLE_ISALLOC(table)) return false;
 entry = THIS_PDIR_ENTRY(PDIR_DINDEX(ptr),PDIR_TINDEX(ptr));
 return (entry.pe_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE)) ==
                       (PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE);
}

LOCAL KPD PHYS bool KCALL
thispdir_isdirty(VIRT pdir_t *__restrict self, VIRT void *ptr, size_t n_bytes) {
 union pd_table table;
 union pd_entry entry;
 for (;;) {
  table = self->pd_directory[PDIR_DINDEX(ptr)];
  if (PDTABLE_ISMAP(table)) {
   if (table.pt_attr&PDIR_ATTR_DIRTY) return true;
  } else if (PDTABLE_ISALLOC(table)) {
   entry = THIS_PDIR_ENTRY(PDIR_DINDEX(ptr),PDIR_TINDEX(ptr));
   if (entry.pe_attr&PDIR_ATTR_DIRTY) return true;
  }
  if (n_bytes <= PAGESIZE) break;
  n_bytes -= PAGESIZE;
  *(uintptr_t *)&ptr += PAGESIZE;
 }
 return false;
}

LOCAL KPD PHYS void KCALL
thispdir_undirty(VIRT pdir_t *__restrict self, VIRT void *ptr, size_t n_bytes) {
 union pd_table *ptable;
 union pd_entry *pentry;
 byte_t *iter = (byte_t *)ptr;
 size_t remaining = n_bytes;
 for (;;) {
  ptable = &self->pd_directory[PDIR_DINDEX(ptr)];
  if (PDTABLE_ISMAP(*ptable)) {
   ptable->pt_attr &= ~PDIR_ATTR_DIRTY;
  } else if (PDTABLE_ISALLOC(*ptable)) {
   pentry = &THIS_PDIR_ENTRY(PDIR_DINDEX(iter),PDIR_TINDEX(iter));
   pentry->pe_attr &= ~PDIR_ATTR_DIRTY;
  }
  if (remaining <= PAGESIZE) break;
  remaining -= PAGESIZE;
  iter += PAGESIZE;
 }
 COMPILER_WRITE_BARRIER();
 /* Flush changes to the page directory, thus ensuring
  * that the CPU will re-generate the dirty bit the next
  * time a write happens. */
 pdir_flush(ptr,n_bytes);
}


/* TODO: Optimized functions for changing the contents of one's own page directory. */
/* TODO: Go through all uses of `TASK_PDIR_KERNEL_BEGIN' and create
 *       custom callbacks that try to use page-directory self-mappings. */

#endif

#else /* CONFIG_PDIR_SELFMAP */
#define KERNEL_GLOBAL_END 0 /* 0xffffffff+1 */

#ifdef __CC__
LOCAL KPD PHYS bool KCALL
thispdir_test_writable(VIRT pdir_t *__restrict self, VIRT void *ptr) {
 bool result; struct mman *old_mman;
 TASK_PDIR_KERNEL_BEGIN(old_mman);
 result = pdir_test_writable(&pdir_kernel,ptr);
 TASK_PDIR_KERNEL_END(old_mman);
 return result;
}
#endif

#endif /* !CONFIG_PDIR_SELFMAP */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_PAGING_UTIL_H */
