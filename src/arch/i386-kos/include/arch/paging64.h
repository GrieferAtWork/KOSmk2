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
#ifndef GUARD_ARCH_I386_KOS_INCLUDE_ARCH_PAGING64_H
#define GUARD_ARCH_I386_KOS_INCLUDE_ARCH_PAGING64_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>

#ifdef __x86_64__
#include <hybrid/types.h>
#include <hybrid/limits.h>
#include <hybrid/typecore.h>
#include <kernel/memory.h>
#include <arch/hints.h>
#include <assert.h>

DECL_BEGIN

#define PHYS_END        __UINT64_C(0x0000800000000000) /* The end of the physical identity-mapping in the kernel page directory. */
#define KERNEL_BASE     __UINT64_C(0xffff800000000000)


/* Mask of all address bits that can actually be used.
 * NOTE: On x86_64, this is 48 bits. */
#define VIRT_MASK       __UINT64_C(0x0000ffffffffffff)
#define PDIR_SIGN_BIT   __UINT64_C(0x0000800000000000)
#define PDIR_SIGN_EXT   __UINT64_C(0xffff000000000000)

/* Pagesizes of different page directory levels. */
#define PDIR_E1_SIZE     __UINT64_C(0x0000000000001000) /* 4 KiB (Same as `PAGESIZE') */
#define PDIR_E2_SIZE     __UINT64_C(0x0000000000200000) /* 2 MiB */
#define PDIR_E3_SIZE     __UINT64_C(0x0000000040000000) /* 1 GiB */
#define PDIR_E4_SIZE     __UINT64_C(0x0000008000000000) /* 512 GiB */
#define ASM_PDIR_E1_SIZE            0x0000000000001000  /* 4 KiB (Same as `PAGESIZE') */
#define ASM_PDIR_E2_SIZE            0x0000000000200000  /* 2 MiB */
#define ASM_PDIR_E3_SIZE            0x0000000040000000  /* 1 GiB */
#define ASM_PDIR_E4_SIZE            0x0000008000000000  /* 512 GiB */

/* Physical address masks of different page directory levels. */
#define PDIR_E1_MASK     __UINT64_C(0x0000fffffffff000) /* == (~(PDIR_E1_SIZE-1) & VIRT_MASK) */
#define PDIR_E2_MASK     __UINT64_C(0x0000ffffffe00000) /* == (~(PDIR_E2_SIZE-1) & VIRT_MASK) */
#define PDIR_E3_MASK     __UINT64_C(0x0000ffffc0000000) /* == (~(PDIR_E3_SIZE-1) & VIRT_MASK) */
#define PDIR_E4_MASK     __UINT64_C(0x0000ff8000000000) /* == (~(PDIR_E4_SIZE-1) & VIRT_MASK) */

/* The amount of sub-level entries contained within any given level. */
#define PDIR_E1_COUNT    512 /* Amount of level #0 entries (pages). */
#define PDIR_E2_COUNT    512 /* Amount of level #1 entries. */
#define PDIR_E3_COUNT    512 /* Amount of level #2 entries. */
#define PDIR_E4_COUNT    512 /* Amount of level #3 entries. */

/* Total amount of representable addresses of individual levels. */
#define PDIR_E1_TOTALSIZE  __UINT64_C(0x0000000000200000) /* 2 MiB */
#define PDIR_E2_TOTALSIZE  __UINT64_C(0x0000000040000000) /* 1 GiB */
#define PDIR_E3_TOTALSIZE  __UINT64_C(0x0000008000000000) /* 512 GiB */
#define PDIR_E4_TOTALSIZE  __UINT64_C(0x0001000000000000) /* 256 TiB */

/* Page directory level indices.
 * NOTE: As you can see, bits 48...63 cannot be addressed. (48 == 39+ffs(0x1ff+1)) */
#define PDIR_E4_INDEX(v_ptr)  (((u64)(v_ptr) >> 39) & 0x1ff)
#define PDIR_E3_INDEX(v_ptr)  (((u64)(v_ptr) >> 30) & 0x1ff)
#define PDIR_E2_INDEX(v_ptr)  (((u64)(v_ptr) >> 21) & 0x1ff)
#define PDIR_E1_INDEX(v_ptr)  (((u64)(v_ptr) >> 12) & 0x1ff)

/* Page directory address offsets (Added to the mapped address when `PDIR_E?_ISADDR(...)' is true). */
#define PDIR_E4_OFFSET(v_ptr)  ((u64)(v_ptr) & (PDIR_E4_SIZE-1))
#define PDIR_E3_OFFSET(v_ptr)  ((u64)(v_ptr) & (PDIR_E3_SIZE-1))
#define PDIR_E2_OFFSET(v_ptr)  ((u64)(v_ptr) & (PDIR_E2_SIZE-1))
#define PDIR_E1_OFFSET(v_ptr)  ((u64)(v_ptr) & (PDIR_E1_SIZE-1))



#define PDIR_ADDR_MASK     __UINT64_C(0xfffffffffffff000)
#define PDIR_LINK_MASK     __UINT64_C(0xfffffffffffff000)
#define PDIR_ATTR_MASK     __UINT64_C(0x0000000000000fff)
#define PDIR_ATTR_GLOBAL              0x0100 /* Set to optimize mappings that appear at the same location in all
                                              * directories it appears inside of (aka: Kernel-allocated stack/memory). */
//#define PDIR_ATTR_DIRTY             0x0040 /* The page was written to. XXX: Aparently this doesn't exist in x86_64's page structure? */
#define PDIR_ATTR_ACCESSED            0x0020 /* The page was read from, or written to. */
#define PDIR_ATTR_USER                0x0004 /* User-space may access this page (read, or write). */
#define PDIR_ATTR_WRITE               0x0002 /* The page is writable. */
#define PDIR_ATTR_PRESENT             0x0001 /* The page is present (When not set, cause a PAGEFAULT that may be used for allocate/load-on-read). */
/* Page directory action flags (accepted by `pdir_mprotect', `pdir_mmap', `pdir_mremap' and `pdir_munmap') */
#define PDIR_FLAG_NOFLUSH             0x8000 /* Don't sync the page directory entry - instead, the caller must invalidate it. */
/* Internal/arch-specific attributes. */
#define PDIR_ATTR_2MIB                0x0080 /* The entry describes an address endpoint, rather than a link. */
//#define PDIR_ATTR_NXE      __UINT64_C(0x8000000000000000) /* No-execute. */





#define PDIR_OFFSETOF_DIRECTORY 0
#define PDIR_SIZE               4096
#define PDIR_ALIGN              4096
#define PDIR_TABLE_COUNT        PDIR_E4_COUNT
#ifdef __CC__
typedef u64 pdir_attr_t; /* Set of `PDIR_ATTR_*|PDIR_FLAG_*' */

union PACKED pdir_e1 { /* Level #1 (PT) entry. */
      u64   e1_data;
 PHYS void *e1_addr;
      u64   e1_attr;
};
union PACKED pdir_e2 { /* Level #2 (PD) entry. */
      u64                  e2_data;
union{PHYS union pdir_e1 (*e2_link)[PDIR_E1_COUNT];
      PHYS void           *e2_addr; };
      u64                  e2_attr;
};
union PACKED pdir_e3 { /* Level #3 (PDP) entry. */
      u64                  e3_data;
union{PHYS union pdir_e2 (*e2_link)[PDIR_E2_COUNT];
      PHYS void           *e2_addr; };
      u64                  e3_attr;
};
union PACKED pdir_e4 { /* Level #4 (PML4) entry. */
      u64                  e4_data;
union{PHYS union pdir_e3 (*e4_link)[PDIR_E3_COUNT];
      PHYS void           *e4_addr; };
      u64                  e4_attr;
};
union PACKED pdir_e {
  union pdir_e1 e1;
  union pdir_e2 e2;
  union pdir_e3 e3;
  union pdir_e4 e4;
};

/* Entry accessors. */
#define PDIR_E1_RDADDR(x)                   ((x).e1_data&PDIR_ADDR_MASK)
#define PDIR_E1_ISADDR(x)                   ((x).e1_data&PDIR_ATTR_PRESENT)

#define PDIR_E2_RDLINK(x) ((union pdir_e1 *)((x).e2_data&PDIR_LINK_MASK))
#define PDIR_E2_RDADDR(x)                   ((x).e2_data&PDIR_ADDR_MASK)
#define PDIR_E2_ISLINK(x)                  (((x).e2_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_2MIB)) == (PDIR_ATTR_PRESENT))
#define PDIR_E2_ISADDR(x)                  (((x).e2_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_2MIB)) == (PDIR_ATTR_PRESENT|PDIR_ATTR_2MIB))
#define PDIR_E2_ISALLOC(x)                 (((x).e2_data&PDIR_LINK_MASK) != PDIR_LINK_MASK && !((x).e2_attr&PDIR_ATTR_2MIB))

#define PDIR_E3_RDLINK(x) ((union pdir_e2 *)((x).e3_data&PDIR_LINK_MASK))
#define PDIR_E3_ISLINK(x)                   ((x).e3_attr&PDIR_ATTR_PRESENT)
#define PDIR_E3_ISALLOC(x)                 (((x).e3_data&PDIR_LINK_MASK) != PDIR_LINK_MASK)

#define PDIR_E4_RDLINK(x) ((union pdir_e3 *)((x).e4_data&PDIR_LINK_MASK))
#define PDIR_E4_ISLINK(x)                   ((x).e4_attr&PDIR_ATTR_PRESENT)
#define PDIR_E4_ISALLOC(x)                 (((x).e4_data&PDIR_LINK_MASK) != PDIR_LINK_MASK)


typedef struct _pdir pdir_t;
struct _pdir { /* Controller structure for a page directory. */
 /* The page directory itself (this vector is your `CR3') */
#ifndef __INTELLISENSE__
 ATTR_ALIGNED(4096)
#endif
 union pdir_e4 pd_directory[PDIR_E4_COUNT];
};
#define PDIR_KERNELBASE_STARTINDEX \
     (((VM_HOST_BASE&(PDIR_E4_TOTALSIZE-1))*PDIR_E4_COUNT)/PDIR_E4_TOTALSIZE)
#define PDIR_ROOTENTRY_REPRSIZE   PDIR_E4_TOTALSIZE


LOCAL KPD PHYS void *KCALL pdir_translate(pdir_t *__restrict self, VIRT void *ptr) {
 union pdir_e e;
 e.e4 = self->pd_directory[PDIR_E4_INDEX(ptr)];
 __assertf(e.e4.e4_attr&PDIR_ATTR_PRESENT,"Faulty address %p (%p)",ptr,e.e4.e4_attr);
 e.e3 = PDIR_E4_RDLINK(e.e4)[PDIR_E3_INDEX(ptr)];
 __assertf(e.e3.e3_attr&PDIR_ATTR_PRESENT,"Faulty address %p (%p)",ptr,e.e3.e3_attr);
 e.e2 = PDIR_E3_RDLINK(e.e3)[PDIR_E2_INDEX(ptr)];
 __assertf(e.e2.e2_attr&PDIR_ATTR_PRESENT,"Faulty address %p (%p)",ptr,e.e2.e2_attr);
 if (e.e2.e2_attr&PDIR_ATTR_2MIB)
     return (PHYS void *)(PDIR_E2_RDADDR(e.e2)+PDIR_E2_OFFSET(ptr));
 e.e1 = PDIR_E2_RDLINK(e.e2)[PDIR_E1_INDEX(ptr)];
 __assertf(PDIR_E1_ISADDR(e.e1),"Faulty address %p (%p)",ptr,e.e1.e1_attr);
 return (PHYS void *)(PDIR_E1_RDADDR(e.e1)+PDIR_E1_OFFSET(ptr));
}
LOCAL KPD PHYS int KCALL pdir_test_readable(pdir_t *__restrict self, VIRT void *ptr) {
 union pdir_e e;
 e.e4 = self->pd_directory[PDIR_E4_INDEX(ptr)];
 if (!(e.e4.e4_attr&PDIR_ATTR_PRESENT)) return 0;
 e.e3 = PDIR_E4_RDLINK(e.e4)[PDIR_E3_INDEX(ptr)];
 if (!(e.e3.e3_attr&PDIR_ATTR_PRESENT)) return 0;
 e.e2 = PDIR_E3_RDLINK(e.e3)[PDIR_E2_INDEX(ptr)];
 if (e.e2.e2_attr&PDIR_ATTR_2MIB) return e.e2.e2_attr&PDIR_ATTR_PRESENT;
 e.e1 = PDIR_E2_RDLINK(e.e2)[PDIR_E1_INDEX(ptr)];
 return e.e1.e1_attr&PDIR_ATTR_PRESENT;
}
LOCAL KPD PHYS int KCALL pdir_test_writable(pdir_t *__restrict self, VIRT void *ptr) {
 union pdir_e e;
 e.e4 = self->pd_directory[PDIR_E4_INDEX(ptr)];
 if (!(e.e4.e4_attr&PDIR_ATTR_PRESENT)) return 0;
 e.e3 = PDIR_E4_RDLINK(e.e4)[PDIR_E3_INDEX(ptr)];
 if (!(e.e3.e3_attr&PDIR_ATTR_PRESENT)) return 0;
 e.e2 = PDIR_E3_RDLINK(e.e3)[PDIR_E2_INDEX(ptr)];
 if ((e.e2.e2_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_2MIB)) != PDIR_ATTR_PRESENT)
      return (e.e2.e2_attr&(PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT)) == (PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT);
 e.e1 = PDIR_E2_RDLINK(e.e2)[PDIR_E1_INDEX(ptr)];
 return (e.e1.e1_attr&(PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT)) == (PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT);
}

LOCAL void FCALL pdir_flushall(void) {
 register register_t temp;
 __asm__ __volatile__("movq %%cr3, %0\n"
                      "movq %0, %%cr3\n"
                      : "=&r" (temp)/* : : "memory"*/);
}

/* Get/Set the currently active page directory.
 * NOTE: These functions work with PHYS pointers! */
#define PDIR_GTCURR()  XBLOCK({ register PHYS pdir_t *_r; __asm__ __volatile__("movq %%cr3, %0\n" : "=r" (_r)); XRETURN _r; })
#define PDIR_STCURR(v) XBLOCK({ __asm__ __volatile__("movq %0, %%cr3\n" : : "r" (v)/* : "memory"*/); (void)0; })
#if PDIR_OFFSETOF_DIRECTORY != 0
#error "Fix the above macros"
#endif


#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Used during early booting to allocate page directory identity pages
 * required for accessing physical memory before we actually know what
 * RAM can be used for ~real~ physical memory management.
 * Technically, this isn't a very good system, because it basically
 * assumes that there is ~sufficient~ memory following the kernel core's
 * position in the physical address space, but before we can start figuring
 * out what the real RAM ranges are (For which we must access memory provided
 * by the bootloader, which may not yet be mapped at that point), we need
 * ~some~ mechanism for allocating pages we can use for mapping that memory.
 * NOTE: Once physical memory management is set up and we've moved on to
 *       initializing the kernel page directory, 
 * HINT: Concerning the multiboot information data structures, a special
 *       check is performed that will move this pointer to the back of that
 *       structure in the event that they should overlap, though normally
 *       multiboot will just place its information structures in low memory. */
INTDEF INITDATA VIRT ppage_t early_page_end;
/* Leave an unused gap in case the bootloader places setup data past the kernel.
 * XXX: At least QEMU's `-kernel ...' option places the commandline past the
 *      loaded kernel in memory (aka. Starting at `KERNEL_END'), even though doing
 *      so was probably just an oversight by the developers, as what we're doing
 *      here could easily be interpreted as a low-level implementation of `brk()/sbrk()'.
 *     (In that we're allocating heap memory behind the executable image), leave a gap of
 *      8 pages for bootloaders that may decide to put stuff there without considering
 *      that the kernel may claim that memory for use by a HEAP implementation.
 *      NOTE: 8 pages is probably too much in any case, but as long as we don't leave
 *            a gap of multiple gigabytes, it's always a game of luck to assume that
 *            memory is available, just as the bootloader must assume that physical
 *            RAM is available where we're try to load the core itself into. */
#define EARLY_PAGE_GAP    (8*PAGESIZE)
#define EARLY_PAGE_BEGIN  (KERNEL_END+EARLY_PAGE_GAP)
#define EARLY_PAGE_END    ((uintptr_t)early_page_end)

/* WARNING: The returned pointer may be either virtual, or physical.
 *          Use `addr_is_virt' / `addr_is_phys' to check its association. */
INTDEF INITCALL ppage_t KCALL early_page_malloc(void);

/* Ensure that the given address is identity-mapped in the
 * kernel page directory during _very_ early booting.
 * Since x86_64 has the absurdly humongous address space is has,
 * together with the fact that there is no way of using a single
 * page to map its entirety (as is possible by using the PSE
 * extension on i386), this acts as a work-around for accessing
 * any physical memory address before paging has been initialized,
 * or RAM has been detected.
 * Internally, the function does the following:
 * >> e4 = &pdir_kernel.pd_directory[PDIR_E4_INDEX(addr)];
 * >> if (!PDIR_E4_ISLINK(*e4)) // Ensure level #4 presence.
 * >>     *e4 = PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT|memset(early_page_malloc(),0,PAGESIZE);
 * >> e3 = &PDIR_E4_RDLINK(*e4)[PDIR_E3_INDEX(addr)];
 * >> if (!PDIR_E3_ISLINK(*e3)) {
 * >>    // Allocate a new level #3 entry.
 * >>    uintptr_t base; size_t i;
 * >>    union pdir_e2 *e2 = early_page_malloc();
 * >>    base = (addr & PDIR_E2_MASK) | PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT;
 * >>    for (i = 0; i < PDIR_E2_COUNT; ++i) {
 * >>         e2[i] = (base+i*PDIR_E2_SIZE);
 * >>    }
 * >>    *e3 = PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT|e2;
 * >> }
 * WARNING: Only available in 64-bit mode, use `pdir_mmap_early' in 32-bit mode. 
 */
INTDEF INITCALL void KCALL early_map_identity(PHYS void *addr, size_t n_bytes);
#define early_map_identity(addr,n_bytes)  early_map_identity(addr,n_bytes)
#endif /* CONFIG_BUILDING_KERNEL_CORE */

#endif /* __CC__ */

DECL_END
#endif /* __x86_64__ */

#endif /* !GUARD_ARCH_I386_KOS_INCLUDE_ARCH_PAGING64_H */
