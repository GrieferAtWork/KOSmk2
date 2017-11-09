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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_PAGING64_H
#define GUARD_INCLUDE_KERNEL_ARCH_PAGING64_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>

#ifdef __x86_64__
#include <hybrid/types.h>
#include <hybrid/limits.h>
#include <hybrid/typecore.h>
#include <kernel/memory.h>
#include <assert.h>

DECL_BEGIN

/* Virtual starting address of the kernel-share segment.
 * This constant describes a 47-bit address space at the upper end of virtual memory.
 * Any address greater than, or equal to this value is mapped identically in all
 * existing page directories and is considered to be owned by the kernel, meaning
 * that user-space is not allowed to access that memory in any way (with the exception
 * of the user-share segment which is mapped with user-space read-only access permissions).
 * Similar to i386-mode, the kernel page directory maps all physical memory below
 * this address to that same address, meaning that on x86_64, usable RAM is limited
 * to a maximum of 2^47 bytes (or `65536' (`0x10000') Terrabyte, so we're good in that department)
 * Usage:
 * 0000000000000000 - 00007FFFFFFFFFFF  USERSPACE
 * --- Hole due to bit#48 -> 49..63 sign extension.
 * FFFF800000000000 - FFFFFFFFFFFFFFFF  KERNELSPACE (Heap, file mappings, modules, etc.)
 * During early boot, the following range is mapped to the first 512 of physical memory:
 * FFFF800000000000 - FFFF807FFFFFFFFF
 * This range contains the kernel core and is later truncated to end past the
 * core itself (`__kernel_end'), leaving a one-on-one physical mappings of the first
 * Mb of physical memory intact (Which is used by e.g.: The core's VGA-TTY driver)
 * HINT: In this configuration, user and kernel-space are of equal size.
 */
#define ASM_USER_MAX               0x00007fffffffffff
#define ASM_USER_END               0x0000800000000000
#define ASM_KERNEL_BASE            0xffff800000000000
#define USER_MAX        __UINT64_C(0x00007fffffffffff)
#define USER_END        __UINT64_C(0x0000800000000000)
#define KERNEL_BASE     __UINT64_C(0xffff800000000000)

/* Mask of all address bits that can actually be used.
 * NOTE: On x86_64, this is 48 bits. */
#define VIRT_MASK       __UINT64_C(0x0000ffffffffffff)

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
#define PDIR_E1_MASK     __UINT64_C(0x0000ffffffff1000)
#define PDIR_E2_MASK     __UINT64_C(0x0000ffffff200000)
#define PDIR_E3_MASK     __UINT64_C(0x0000ffff40000000)
#define PDIR_E4_MASK     __UINT64_C(0x0000ff8000000000)

/* The amount of sub-level entries contained within any given level. */
#define PDIR_E1_COUNT 512 /* Amount of level #0 entries (pages). */
#define PDIR_E2_COUNT 512 /* Amount of level #1 entries. */
#define PDIR_E3_COUNT 512 /* Amount of level #2 entries. */
#define PDIR_E4_COUNT 512 /* Amount of level #3 entries. */

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
#define PDIR_ATTR_DIRTY               0x0040 /* The page was written to. */
#define PDIR_ATTR_ACCESSED            0x0020 /* The page was read from, or written to. */
#define PDIR_ATTR_USER                0x0004 /* User-space may access this page (read, or write). */
#define PDIR_ATTR_WRITE               0x0002 /* The page is writable. */
#define PDIR_ATTR_PRESENT             0x0001 /* The page is present (When not set, cause a PAGEFAULT that may be used for allocate/load-on-read). */
/* Page directory action flags (accepted by `pdir_mprotect', `pdir_mmap', `pdir_mremap' and `pdir_munmap') */
#define PDIR_FLAG_NOFLUSH             0x8000 /* Don't sync the page directory entry - instead, the caller must invalidate it. */
/* Internal/arch-specific attributes. */
#define PDIR_ATTR_4MIB                0x0080 /* The entry describes an address endpoint, rather than a link. */

/* Check if 4Mib pages are allowed for the given address.
 * NOTE: They are not allowed for kernel pages, so-as to
 *       keep the indirection required for sharing pages.
 * NOTE: This check applies to levels #1, #2 and #3 */
#define PDIR_ALLOW_4MIB(addr) ((u64)(addr) < KERNEL_BASE)





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
#define PDIR_E2_ISLINK(x)                  (((x).e2_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_4MIB)) == (PDIR_ATTR_PRESENT))
#define PDIR_E2_ISADDR(x)                  (((x).e2_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_4MIB)) == (PDIR_ATTR_PRESENT|PDIR_ATTR_4MIB))
#define PDIR_E3_RDLINK(x) ((union pdir_e2 *)((x).e3_data&PDIR_LINK_MASK))
#define PDIR_E3_ISLINK(x)                   ((x).e3_attr&PDIR_ATTR_PRESENT)
#define PDIR_E4_RDLINK(x) ((union pdir_e3 *)((x).e4_data&PDIR_LINK_MASK))
#define PDIR_E4_ISLINK(x)                   ((x).e4_attr&PDIR_ATTR_PRESENT)


typedef struct _pdir pdir_t;
struct _pdir { /* Controller structure for a page directory. */
 /* The page directory itself (this vector is your `CR3') */
#ifndef __INTELLISENSE__
 ATTR_ALIGNED(4096)
#endif
 union pdir_e4 pd_directory[PDIR_E4_COUNT];
};
#define PDIR_KERNELSHARE_STARTINDEX \
     (((KERNEL_BASE&(PDIR_E4_TOTALSIZE-1))*PDIR_E4_COUNT)/PDIR_E4_TOTALSIZE)
#define PDIR_ROOTENTRY_REPRSIZE   PDIR_E4_TOTALSIZE


LOCAL KPD PHYS void *KCALL pdir_translate(pdir_t *__restrict self, VIRT void *ptr) {
 union pdir_e e;
 e.e4 = self->pd_directory[PDIR_E4_INDEX(ptr)];
 __assertf(e.e4.e4_attr&PDIR_ATTR_PRESENT,"Faulty address %p",ptr);
 e.e3 = PDIR_E4_RDLINK(e.e4)[PDIR_E3_INDEX(ptr)];
 __assertf(e.e3.e3_attr&PDIR_ATTR_PRESENT,"Faulty address %p",ptr);
 e.e2 = PDIR_E3_RDLINK(e.e3)[PDIR_E2_INDEX(ptr)];
 __assertf(e.e2.e2_attr&PDIR_ATTR_PRESENT,"Faulty address %p",ptr);
 if (e.e2.e2_attr&PDIR_ATTR_4MIB)
     return (PHYS void *)(PDIR_E2_RDADDR(e.e2)+PDIR_E2_OFFSET(ptr));
 e.e1 = PDIR_E2_RDLINK(e.e2)[PDIR_E1_INDEX(ptr)];
 __assertf(PDIR_E1_ISADDR(e.e1),"Faulty address %p",ptr);
 return (PHYS void *)(PDIR_E1_RDADDR(e.e1)+PDIR_E1_OFFSET(ptr));
}
LOCAL KPD PHYS int KCALL pdir_test_writable(pdir_t *__restrict self, VIRT void *ptr) {
 union pdir_e e;
 e.e4 = self->pd_directory[PDIR_E4_INDEX(ptr)];
 if (!(e.e4.e4_attr&PDIR_ATTR_PRESENT)) return 0;
 e.e3 = PDIR_E4_RDLINK(e.e4)[PDIR_E3_INDEX(ptr)];
 if (!(e.e3.e3_attr&PDIR_ATTR_PRESENT)) return 0;
 e.e2 = PDIR_E3_RDLINK(e.e3)[PDIR_E2_INDEX(ptr)];
 if ((e.e2.e2_attr&(PDIR_ATTR_PRESENT|PDIR_ATTR_4MIB)) != PDIR_ATTR_PRESENT)
      return (e.e2.e2_attr&(PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT)) == PDIR_ATTR_WRITE;
 e.e1 = PDIR_E2_RDLINK(e.e2)[PDIR_E1_INDEX(ptr)];
 return (e.e1.e1_attr&(PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT)) == PDIR_ATTR_WRITE;
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

#endif /* __CC__ */

DECL_END
#endif /* __x86_64__ */

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_PAGING64_H */
