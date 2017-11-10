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
#include <hybrid/host.h>

#ifdef __x86_64__
#include "arch/paging64.h"
#elif defined(__i386__)
#include "arch/paging32.h"
#else
#error "Unsupported arch"
#endif

#ifdef __CC__
#include <errno.h>
#include <hybrid/types.h>
#include <kernel/memory.h>
#include <format-printer.h>
#endif /* __CC__ */

DECL_BEGIN

#ifdef __CC__
#define phys_to_virt(ptr)  ((VIRT void *)((uintptr_t)(ptr)+CORE_BASE))
#define virt_to_phys(ptr)  ((PHYS void *)((uintptr_t)(ptr)-CORE_BASE))
#define addr_isphys(ptr)   ((uintptr_t)(ptr) <  CORE_BASE)
#define addr_isvirt(ptr)   ((uintptr_t)(ptr) >= CORE_BASE)
#else
#define phys_to_virt(ptr)  ((ptr)+CORE_BASE)
#define virt_to_phys(ptr)  ((ptr)-CORE_BASE)
#define addr_isphys(ptr)   ((ptr) <  CORE_BASE)
#define addr_isvirt(ptr)   ((ptr) >= CORE_BASE)
#endif

#ifdef __CC__
/* The global kernel page directory.
 * NOTE: All data above `KERNEL_BASE' is mirrored in all user-space directories. */
DATDEF PHYS pdir_t pdir_kernel;
DATDEF VIRT pdir_t pdir_kernel_v;

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Do some minor initialization of paging utility functions
 * (relocate functions based on what the CPU can do).
 * NOTE: Also sets up read-only kernel pages. */
INTDEF INITCALL void KCALL pdir_initialize(void);
#endif
#endif /* __CC__ */

#ifdef __CC__
/* Raw, unchecked pointer translation.
 * NOTE: The caller must ensure that no PAGEFAULT
 *       would occur when hardware would be used. */
#define PDIR_TRANSLATE(dir,v_ptr) pdir_translate(dir,(VIRT void *)(v_ptr))

/* Initialize/finalize a given page directory.
 * @return: true:  Successfully initialized the page directory.
 * @return: false: Not enough available memory. */
FUNDEF WUNUSED bool KCALL pdir_init(pdir_t *__restrict self);
FUNDEF void KCALL pdir_fini(pdir_t *__restrict self);

/* Load `self' as a copy of `existing' (NOTE: Called _AFTER_ `pdir_init(self)' succeeded!) */
FUNDEF WUNUSED bool KCALL pdir_load_copy(pdir_t *__restrict self, pdir_t const *__restrict existing);

/* Flush `n_bytes' starting at `addr' in the currently set page directory. */
FUNDEF void FCALL pdir_flush(VIRT void *start, size_t n_bytes);

/* Check if the current page directory matches the `KPD' annotation. */
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


/* Changes the protection/attributes of page mappings within `start..+=n_bytes'.
 * NOTE: The given argument `n_bytes' is ceil-aligned by pages.
 * @return: * :      The amount of bytes modified, following `start' (always a multiple of PAGESIZE)
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF ssize_t KCALL pdir_mprotect(pdir_t *__restrict self, ppage_t start,
                                   size_t n_bytes, pdir_attr_t flags);

/* Returns true if all pages within `addr...+=n_bytes' have all flags from `flags' set. */
FUNDEF bool KCALL pdir_maccess(pdir_t const *__restrict self,
                               VIRT void const *addr, size_t n_bytes,
                               pdir_attr_t flags);
/* Same as `pdir_maccess', but optimized for a single address. */
FUNDEF bool KCALL pdir_maccess_addr(pdir_t const *__restrict self, VIRT void const *addr, pdir_attr_t flags);

/* Create a physical mapping for `target' to `start..+=n_bytes'
 * NOTE: The given argument `n_bytes' is ceil-aligned by pages.
 * NOTE: Any existing mappings are replaced.
 * @param: start: The starting address where memory mappings should begin.
 * @param: flags: Mapping flags.
 * @return: -EOK:    Successfully created a mapping.
 * @return: -ENOMEM: Not enough available memory, or address space. */
FUNDEF errno_t KCALL pdir_mmap(pdir_t *__restrict self, VIRT ppage_t start,
                               size_t n_bytes, PHYS ppage_t target, pdir_attr_t flags);

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Used for mapping virtual memory during early booting (before paging is initialized). */
INTDEF INITCALL errno_t KCALL
pdir_mmap_early(pdir_t *__restrict self, VIRT ppage_t start,
                size_t n_bytes, PHYS ppage_t target, pdir_attr_t flags);
#endif /* CONFIG_BUILDING_KERNEL_CORE */

/* Unmap a virtual memory mapping.
 * NOTE: No-op if no mapping exists within the given range.
 * @param: flags:    Only used for `PDIR_FLAG_NOFLUSH'
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

#ifndef early_map_identity
/* The entirety of the physical address space is already identity-mapped. */
#define early_map_identity(addr,n_bytes) (void)0
#endif

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_PAGING_H */
