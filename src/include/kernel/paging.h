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
#ifndef GUARD_INCLUDE_KERNEL_PAGING_H
#define GUARD_INCLUDE_KERNEL_PAGING_H 1

#include <hybrid/compiler.h>

#ifdef __CC__
#include <errno.h>
#include <hybrid/types.h>
#include <kernel/memory.h>
#include <format-printer.h>
#endif /* __CC__ */

DECL_BEGIN

/* The virtual base address of the kernel itself!
 * WARNING: Changing this values requires additional changes in:
 * >> /src/kernel/core/paging.c:  pdir_kernel
 * >> /src/kernel/include/mman.h: ZONE_* (Comments)
 * WARNING: The kernel may never map addresses lower than this to itself!
 * All addresses lower than this are usually unmapped, with the exception
 * of special memory regions that encompass physical memory, which
 * maps 1 on 1 to kernel virtual addresses.
 * As a consequence of this, KOS is (by default) limited to only using
 * 3GB of physically available memory, as anything higher cannot actually
 * be used (as the associated address space is reserved by the kernel).
 */
#define KERNEL_BASE     0xc0000000

#define phys_to_virt(ptr)  ((VIRT void *)((uintptr_t)(ptr)+KERNEL_BASE))
#define virt_to_phys(ptr)  ((PHYS void *)((uintptr_t)(ptr)-KERNEL_BASE))
#define addr_isphys(ptr)   ((uintptr_t)(ptr) <  KERNEL_BASE)
#define addr_isvirt(ptr)   ((uintptr_t)(ptr) >= KERNEL_BASE)



#define PDIR_ATTR_MASK     0x0fffu
#define PDIR_ATTR_GLOBAL   0x0100u /* Set to optimize mappings that appear at the same location in all
                                    * directories it appears inside of (aka: Kernel-allocated stack/memory). */
#define PDIR_ATTR_DIRTY    0x0040u /* The page was written to. */
#define PDIR_ATTR_ACCESSED 0x0020u /* The page was read from, or written to. */
#define PDIR_ATTR_USER     0x0004u /* User-space may access this page (read, or write). */
#define PDIR_ATTR_WRITE    0x0002u /* The page is writable. */
#define PDIR_ATTR_PRESENT  0x0001u /* The page is present (When not set, cause a PAGEFAULT that may be used for allocate/load-on-read). */
#ifdef __CC__
typedef u16 pdir_attr_t; /* Set of 'PDIR_ATTR_*|PDIR_FLAG_*' */
#endif /* __CC__ */

/* Page directory action flags (accepted by 'pdir_mprotect', 'pdir_mmap', 'pdir_mremap' and 'pdir_munmap') */
#define PDIR_FLAG_NOFLUSH  0x8000u /* Don't sync the page directory entry - instead, the caller must invalidate it. */


/* Internal/arch-specific attributes. */
#define PDIR_ATTR_4MIB     0x0080u /* Only used by 'pd_table': Directly map a physical address.
                                    * NOTE: Use of this requires the 'CR4_PSE' bit to be set. */

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
                                *  [owned_if((self-pd_directory)*0x400000 < KERNEL_BASE)] // NOTE: '0x400000 == (1 << 32) / PD_TABLE_ENTRY_COUNT' (Aka: The first kernel-page)
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
                                *   NOTE: Also, using 'PDIR_ATTR_PRESENT', indicates the presence of a . */
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
 /* The page directory itself (this vector is your 'CR3')
  * NOTE: Index == PDIR_DINDEX(v_ptr). */
 union pd_table pd_directory[PDIR_TABLE_COUNT];
};

/* The global kernel page directory.
 * NOTE: All data above KERNEL_BASE is mirrored in all user-space directories. */
DATDEF PHYS pdir_t pdir_kernel;
DATDEF VIRT pdir_t pdir_kernel_v;

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Do some minor initialization of paging utility functions
 * (relocate functions based on what the CPU can do).
 * NOTE: Also sets up read-only kernel pages.
 * WARNING: This function is an init-call and must not
 *          be called once free-data has been released!
 */
INTDEF void KCALL pdir_initialize(void);
#endif
#endif /* __CC__ */

#define PDENTRY_REPRSIZE    PAGESIZE /* 1 << 12 */
#define PDTABLE_REPRSIZE    0x400000 /* 1 << 22 */

#define PTTABLE_ARRAYSIZE  (PDTABLE_REPRSIZE/PDENTRY_REPRSIZE)
#define PDTABLE_ALLOCSIZE  (PTTABLE_ARRAYSIZE*PD_ENTRY_SIZE)

/* Return the directory/table index of a given virtual pointer 'v_ptr'. */
#define PDIR_DINDEX(v_ptr)  ((u32)(v_ptr) >> 22)
#define PDIR_TINDEX(v_ptr) (((u32)(v_ptr) >> 12) & 0x3ff)

/* Return the page-local offset of of a given virtual pointer 'v_ptr'. */
#define PDIR_OFFSET(v_ptr)  ((u32)(v_ptr) & 0xfff)
#define PDIR_MOFFSET(v_ptr) ((u32)(v_ptr) & 0x3fffff)

#ifdef __CC__
/* Raw, unchecked pointer translation.
 * NOTE: The caller must ensure that no PAGEFAULT
 *       would occur when hardware would be used. */
#define PDIR_TRANSLATE(dir,v_ptr) pdir_translate(dir,(VIRT void *)(v_ptr))
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

/* Initialize/finalize a given page directory.
 * @return: true:  Successfully initialized the page directory.
 * @return: false: Not enough available memory. */
FUNDEF WUNUSED bool KCALL pdir_init(pdir_t *__restrict self);
FUNDEF void KCALL pdir_fini(pdir_t *__restrict self);

/* Load 'self' as a copy of 'existing' (NOTE: Called _AFTER_ 'pdir_init(self)' succeeded!) */
FUNDEF WUNUSED bool KCALL pdir_load_copy(pdir_t *__restrict self, pdir_t const *__restrict existing);

/* Flush 'n_bytes' starting at 'addr' in the currently set page directory. */
FUNDEF void FCALL pdir_flush(VIRT void *start, size_t n_bytes);
LOCAL void FCALL pdir_flushall(void) {
 register u32 temp;
 __asm__ __volatile__("movl %%cr0, %0\n"
                      "movl %0, %%cr0\n"
                      : "=r" (temp)
                      : 
                      : "memory");
}

/* Get/Set the currently active page directory.
 * NOTE: These functions work with PHYS pointers! */
#define PDIR_GTCURR()  XBLOCK({ register PHYS pdir_t *_r; __asm__ __volatile__("movl %%cr3, %0" : "=r" (_r)); XRETURN _r; })
#define PDIR_STCURR(v) XBLOCK({ __asm__ __volatile__("movl %0, %%cr3" : : "r" (v)); (void)0; })
#if PDIR_OFFSETOF_DIRECTORY != 0
#error "Fix the above macros"
#endif

/* Check if the current page directory matches the 'KPD' annotation. */
#define PDIR_ISKPD()  (PDIR_GTCURR() == &pdir_kernel)

#ifdef GUARD_INCLUDE_KERNEL_MALLOC_H
/* Flag for allocating memory locally
 * (visibly in at least the current page directory) */
#if 1
#   define GFP_LOCAL  (PDIR_ISKPD() ? GFP_KERNEL : GFP_SHARED)
#else
#   define GFP_LOCAL                              (GFP_SHARED)
#endif
#endif /* GUARD_INCLUDE_KERNEL_MALLOC_H */


/* TODO: Currently all page directory function require 'PDIR_ISKPD()'
 *       This wouldn't be necessary if we'd create page-directory self-mappings...
 */

/* Changes the protection/attributes of page mappings within 'start..+=n_bytes'.
 * NOTE: The given argument 'n_bytes' is ceil-aligned by pages.
 * @return: * :      The amount of bytes modified, following 'start' (always a multiple of PAGESIZE)
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF ssize_t KCALL pdir_mprotect(pdir_t *__restrict self, ppage_t start,
                                   size_t n_bytes, pdir_attr_t flags);

/* Returns true if all pages within 'addr...+=n_bytes' have all flags from 'flags' set. */
FUNDEF bool KCALL pdir_maccess(pdir_t const *__restrict self,
                               VIRT void const *addr, size_t n_bytes,
                               pdir_attr_t flags);
/* Same as 'pdir_maccess', but optimized for a single address. */
FUNDEF bool KCALL pdir_maccess_addr(pdir_t *__restrict self, VIRT void const *addr, pdir_attr_t flags);

/* Create a physical mapping for 'target' to 'start..+=n_bytes'
 * NOTE: The given argument 'n_bytes' is ceil-aligned by pages.
 * NOTE: Any existing mappings are replaced.
 * @param: start: The starting address where memory mappings should begin.
 * @param: flags: Mapping flags.
 * @return: -EOK:    Successfully created a mapping.
 * @return: -ENOMEM: Not enough available memory, or address space. */
FUNDEF errno_t KCALL pdir_mmap(pdir_t *__restrict self, VIRT ppage_t start,
                               size_t n_bytes, PHYS ppage_t target, pdir_attr_t flags);

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Used for mapping virtual memory during early booting (before paging is initialized).
 * WARNING: This function is an init-call and must not
 *          be called once free-data has been released! */
INTDEF errno_t KCALL pdir_mmap_early(pdir_t *__restrict self, VIRT ppage_t start,
                                     size_t n_bytes, PHYS ppage_t target, pdir_attr_t flags);
#endif /* CONFIG_BUILDING_KERNEL_CORE */

/* Unmap a virtual memory mapping.
 * NOTE: No-op if no mapping exists within the given range.
 * @param: flags:    Only used for 'PDIR_FLAG_NOFLUSH'
 * @return: -EOK:    No more mappings exist within the given range.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF errno_t KCALL pdir_munmap(pdir_t *__restrict self, VIRT ppage_t start,
                                 size_t n_bytes, pdir_attr_t flags);

/* Print a human-readable representation of the given page directory. */
FUNDEF ssize_t KCALL pdir_print(pdir_t *__restrict self,
                                pformatprinter printer, void *closure);


typedef ssize_t (KCALL *pdirwalker)(VIRT ppage_t v_addr, PHYS ppage_t p_addr,
                                    size_t n_bytes, pdir_attr_t attr,
                                    void *closure);

/* Enumerate all consecutive portions of the given page directory,
 * executing the given walker() for each and eventually returning
 * the sum of all its return values, or when a negative value
 * is returned, that value before finishing. */
FUNDEF ssize_t KCALL pdir_enum(pdir_t *__restrict self,
                               pdirwalker walker, void *closure);

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_PAGING_H */
