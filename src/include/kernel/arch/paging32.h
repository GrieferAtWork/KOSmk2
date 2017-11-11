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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_PAGING32_H
#define GUARD_INCLUDE_KERNEL_ARCH_PAGING32_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#ifndef __x86_64__
#include <hybrid/types.h>
#include <hybrid/limits.h>
#include <hybrid/typecore.h>
#include <kernel/memory.h>
#include <assert.h>

DECL_BEGIN

/* The virtual base address of the kernel itself!
 * WARNING: Changing this values requires additional changes in:
 * >> /src/kernel/core/paging.c:  pdir_kernel
 * >> /src/kernel/include/mman.h: ZONE_* (Comments)
 * WARNING: The kernel may never map addresses lower than this to itself!
 * All addresses lower than this are usually unmapped, with the exception
 * of special memory regions that encompass physical memory, which
 * map 1 on 1 to kernel virtual addresses.
 * As a consequence of this, KOS is (by default) limited to only using
 * 3GB of physically available memory, as anything higher cannot actually
 * be used (due to the associated address space being reserved by the kernel).
 * TODO: Add a config that still allows use of physical memory above 3Gb,
 *       adding a new memory zone who's memory cannot be accessed directly,
 *       and is therefor managed by special code that must be deleted during
 *       boot when it is detected that no physical memory above 3Gb exists,
 *       as well as minor changes to mman ALLOA to make use of such memory
 *       for userspace applications/virtual kernel memory.
 */
#define ASM_USER_MAX               0xbfffffff
#define ASM_USER_END               0xc0000000
#define ASM_KERNEL_BASE            0xc0000000
#define ASM_CORE_BASE              0xc0000000
#define USER_MAX        __UINT32_C(0xbfffffff)
#define USER_END        __UINT32_C(0xc0000000)
#define KERNEL_BASE     __UINT32_C(0xc0000000)
#define CORE_BASE       __UINT32_C(0xc0000000)

/* Mask of all address bits that can actually be used.
 * NOTE: On i386, that simply is every bit there is (All 32). */
#define VIRT_MASK       __UINT32_C(0xffffffff)


#define PDIR_ATTR_MASK     0x0fffu
#define PDIR_ATTR_GLOBAL   0x0100u /* Set to optimize mappings that appear at the same location in all
                                    * directories it appears inside of (aka: Kernel-allocated stack/memory). */
#define PDIR_ATTR_DIRTY    0x0040u /* The page was written to. */
#define PDIR_ATTR_ACCESSED 0x0020u /* The page was read from, or written to. */
#define PDIR_ATTR_USER     0x0004u /* User-space may access this page (read, or write). */
#define PDIR_ATTR_WRITE    0x0002u /* The page is writable. */
#define PDIR_ATTR_PRESENT  0x0001u /* The page is present (When not set, cause a PAGEFAULT that may be used for allocate/load-on-read). */
#ifdef __CC__
typedef u16 pdir_attr_t; /* Set of `PDIR_ATTR_*|PDIR_FLAG_*' */
#endif /* __CC__ */

/* Page directory action flags (accepted by `pdir_mprotect', `pdir_mmap', `pdir_mremap' and `pdir_munmap') */
#define PDIR_FLAG_NOFLUSH  0x8000u /* Don't sync the page directory entry - instead, the caller must invalidate it. */


/* Internal/arch-specific attributes. */
#define PDIR_ATTR_4MIB     0x0080u /* Only used by `pd_table': Directly map a physical address.
                                    * NOTE: Use of this requires the `CR4_PSE' bit to be set. */

/* Check if 4Mib pages are allowed for the given address.
 * NOTE: They are not allowed for kernel pages, so-as to
 *       keep the indirection required for sharing pages. */
#define PDIR_ATTR_ALLOW_4MIB(addr) ((uintptr_t)(addr) < KERNEL_BASE)


#define PD_ENTRY_SIZE        4
#define PD_TABLE_SIZE        4
#define PD_TABLE_ENTRY_COUNT 1024

#ifdef __CC__
typedef struct _pdir pdir_t;

union pd_entry {
      u32   pe_data; /*< Entry data. */
#ifdef __INTELLISENSE__
      void *pe_map;  /*< [MASK(~PDIR_ATTR_MASK)] The mapped physical pointer (page-aligned). */
      u32   pe_attr; /*< [MASK(PDIR_ATTR_MASK)] Mapping attributes. */
#else
 PHYS void *pe_map;  /*< [MASK(~PDIR_ATTR_MASK)] The mapped physical pointer (page-aligned). */
      u32   pe_attr; /*< [MASK(PDIR_ATTR_MASK)] Mapping attributes. */
#endif
};
#define PDENTRY_GETMAP(self)    ((PHYS ppage_t)((self).pe_data&~PDIR_ATTR_MASK))
#define PDENTRY_ISPRESENT(self) ((self).pe_attr&PDIR_ATTR_PRESENT)

union PACKED pd_table {
      u32             pt_data; /*< Table data. */
union{
 PHYS union pd_entry *pt_ptev; /*< [0..1=PD_TABLE_ENTRY_COUNT][MASK(~PDIR_ATTR_MASK)]
                                *  [owned_if((self-pd_directory)*0x400000 < KERNEL_BASE)] // NOTE: `0x400000 == (1 << 32) / PD_TABLE_ENTRY_COUNT' (Aka: The first kernel-page)
                                *  [valid_if(PDTABLE_ISVALID(self))]
                                *  [alloc_if(PDTABLE_ISALLOC(self))]
                                *   The associated page table entry-vector (Always contains PD_TABLE_ENTRY_COUNT elements).
                                *   NOTE: Index == PDIR_TINDEX(v_ptr). */
 PHYS void           *pt_map;  /*< [MASK(~PDIR_ATTR_MASK)]
                                *  [valid_if(pt_attr&PDIR_ATTR_4MIB)]
                                *   The mapped physical pointer (page-aligned). */
};
      u32             pt_attr; /*< [0..1][MASK(PDIR_ATTR_MASK)]
                                *   Page directory entry attributes (or'd together attributes of all table entries).
                                *   NOTE: Also, using `PDIR_ATTR_PRESENT', indicates the presence of a . */
};
#define PDTABLE_GETPTEV(self)   ((PHYS union pd_entry *)((self).pt_data&~PDIR_ATTR_MASK))
#define PDTABLE_ISMAP(self)     ((self).pt_attr&PDIR_ATTR_4MIB)
#define PDTABLE_ISTABLE(self) (!((self).pt_attr&PDIR_ATTR_4MIB))
#define PDTABLE_ISALLOC(self) ((((self).pt_data&~PDIR_ATTR_MASK) != ~PDIR_ATTR_MASK) && PDTABLE_ISTABLE(self))
#define PDTABLE_ISVALID(self) ((((self).pt_data&~PDIR_ATTR_MASK) != ~PDIR_ATTR_MASK) && (((self).pt_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_4MIB)) == PDIR_ATTR_PRESENT))
#define PDTABLE_ISPRESENT(self) ((self).pt_attr&PDIR_ATTR_PRESENT)
#define PDTABLE_GETMAP(self)    ((PHYS ppage_t)((self).pt_data&~PDIR_ATTR_MASK))
#endif /* __CC__ */

#define PDIR_OFFSETOF_DIRECTORY 0
#define PDIR_SIZE               4096
#define PDIR_ALIGN              PAGESIZE
#define PDIR_TABLE_COUNT        1024

#ifdef __CC__
struct _pdir {
 /* [lock(this == PDIR_GTCURR())] Controller structure for a page directory.
  *  Note, that any thread is only ever allowed to modify its, currently active directory. */
#ifndef __INTELLISENSE__
 ATTR_ALIGNED(PDIR_ALIGN)
#endif
 /* The page directory itself (this vector is your `CR3')
  * NOTE: Index == PDIR_DINDEX(v_ptr). */
 union pd_table pd_directory[PDIR_TABLE_COUNT];
};
#endif /* __CC__ */

#define PDIR_KERNELBASE_STARTINDEX  (KERNEL_BASE/PDTABLE_REPRSIZE)
#define PDIR_ROOTENTRY_REPRSIZE       PDTABLE_REPRSIZE

#define PDENTRY_REPRSIZE    PAGESIZE /* 1 << 12 */
#define PDTABLE_REPRSIZE    0x400000 /* 1 << 22 */

#define PTTABLE_ARRAYSIZE  (PDTABLE_REPRSIZE/PDENTRY_REPRSIZE)
#define PDTABLE_ALLOCSIZE  (PTTABLE_ARRAYSIZE*PD_ENTRY_SIZE)

/* Return the directory/table index of a given virtual pointer `v_ptr'. */
#define PDIR_DINDEX(v_ptr)  ((u32)(v_ptr) >> 22)
#define PDIR_TINDEX(v_ptr) (((u32)(v_ptr) >> 12) & 0x3ff)

/* Return the page-local offset of of a given virtual pointer `v_ptr'. */
#define PDIR_OFFSET(v_ptr)  ((u32)(v_ptr) & 0xfff)
#define PDIR_MOFFSET(v_ptr) ((u32)(v_ptr) & 0x3fffff)

#ifdef __CC__
LOCAL KPD PHYS void *KCALL pdir_translate(pdir_t *__restrict self, VIRT void *ptr) {
 union pd_entry entry;
 union pd_table table = self->pd_directory[PDIR_DINDEX(ptr)];
 if (PDTABLE_ISMAP(table))
     return (void *)((uintptr_t)PDTABLE_GETMAP(table)+PDIR_MOFFSET(ptr));
 assert(PDTABLE_ISVALID(table));
 entry = PDTABLE_GETPTEV(table)[PDIR_TINDEX(ptr)];
 assert(entry.pe_attr&PDIR_ATTR_PRESENT);
 return (void *)((uintptr_t)PDENTRY_GETMAP(entry)+PDIR_OFFSET(ptr));
}
LOCAL KPD PHYS int KCALL pdir_test_writable(pdir_t *__restrict self, VIRT void *ptr) {
 union pd_entry entry;
 union pd_table table = self->pd_directory[PDIR_DINDEX(ptr)];
 if (PDTABLE_ISMAP(table)) return (table.pt_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE)) ==
                                                 (PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE);
 entry = PDTABLE_GETPTEV(table)[PDIR_TINDEX(ptr)];
 return (entry.pe_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE)) ==
                       (PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE);
}


LOCAL void FCALL pdir_flushall(void) {
 register u32 temp;
 __asm__ __volatile__("movl %%cr3, %0\n"
                      "movl %0, %%cr3\n"
                      : "=&r" (temp)/* : : "memory"*/);
}

/* Get/Set the currently active page directory.
 * NOTE: These functions work with PHYS pointers! */
#define PDIR_GTCURR()  XBLOCK({ register PHYS pdir_t *_r; __asm__ __volatile__("movl %%cr3, %0" : "=r" (_r)); XRETURN _r; })
#define PDIR_STCURR(v) XBLOCK({ __asm__ __volatile__("movl %0, %%cr3" : : "r" (v)/* : "memory"*/); (void)0; })
#if PDIR_OFFSETOF_DIRECTORY != 0
#error "Fix the above macros"
#endif

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Used for mapping virtual memory during early booting (before paging is initialized).
 * WARNING: Only available in 32-bit mode, use `early_map_identity' in 64-bit mode. */
INTDEF INITCALL errno_t KCALL
pdir_mmap_early(pdir_t *__restrict self, VIRT ppage_t start,
                size_t n_bytes, PHYS ppage_t target, pdir_attr_t flags);
#endif /* CONFIG_BUILDING_KERNEL_CORE */

#endif /* __CC__ */


DECL_END
#endif /* !__x86_64__ */

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_PAGING32_H */
