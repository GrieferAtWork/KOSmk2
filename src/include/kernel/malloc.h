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
#ifndef GUARD_INCLUDE_KERNEL_MALLOC_H
#define GUARD_INCLUDE_KERNEL_MALLOC_H 1

/* Kernel memory management:
 * THIS: malloc.h: Heap memory
 *       memory.h: Physical memory
 *       mman.h:   Virtual memory
 */

#include <stddef.h>
#include <stdbool.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <features.h>
#include <__malldefs.h>
#include <bits/endian.h>
#ifdef __USE_DEBUG
#include <hybrid/debuginfo.h>
#endif
#ifdef __OPTIMIZE__
#include <string.h>
#endif

DECL_BEGIN

/* The default & minimal alignment required/used for kmalloc-allocated memory.
 * In addition, this also describes the size of a minimal-sized malloc-chunk
 * (including internal overhead), as well as the alignment of any value
 * ever returned by `kmalloc_usable_size'.
 * NOTE: This value can be anything!
 */
#ifndef HEAP_ALIGNMENT
#define HEAP_ALIGNMENT  __MALL_MIN_ALIGNMENT
#endif /* !HEAP_ALIGNMENT */

#ifdef __CC__
typedef __UINT16_TYPE__ gfp_t;

/* Truncate unused memory, pre-allocated memory from
 * all heaps, at most releasing `max_free' bytes of mapped
 * memory back to the system (physical memory allocator).
 * NOTE: This function is called during the early phases of `mman_swapmem' in order
 *       to clean up small portions of memory oftenly available after dynamic memory
 *       allocation has been used for some time.
 * NOTE: Unused heap memory is always freed once a single continuous block of data
 *       exceeds `M_TRIM_THRESHOLD', effectively meaning that this function releases
 *       all memory as though `M_TRIM_THRESHOLD' had always been set to `PAGESIZE'
 *      (Its minimum effective value).
 * @return: * : The total amount of released bytes of memory. */
FUNDEF SAFE size_t (KCALL kmalloc_trim)(size_t max_free);

/* Allocate kernel memory.
 * @param: flags: A set of `GFP_*' */
FUNDEF SAFE WUNUSED __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((1))
ATTR_MALLOC void *(KCALL __kmalloc)(size_t size, gfp_t flags) ASMNAME("kmalloc");
FUNDEF SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __kmemalign)(size_t alignment, size_t size, gfp_t flags) ASMNAME("kmemalign");

/* Reallocate existing memory.
 * @param: flags: A set of `GFP_*' matching `GFP_MASK_FLAGS'
 *          NOTE: When `ptr' is NULL, `flags' may also include any other `GFP_*' heap-designator flag(s).
 *                Otherwise, heap designators are overwritten by those originally used to allocate `ptr'. */
FUNDEF SAFE WUNUSED NONNULL((1)) __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __krealloc)(void *ptr, size_t size, gfp_t flags) ASMNAME("krealloc");
FUNDEF SAFE WUNUSED NONNULL((1)) ATTR_ALLOC_ALIGN(2) ATTR_ALLOC_SIZE((3))
ATTR_MALLOC void *(KCALL __krealign)(void *ptr, size_t alignment, size_t size, gfp_t flags) ASMNAME("krealign");

/* Free a given kernel-allocated pointer `ptr'.
 * NOTE: When passing GFP_CALLOC to `kffree', all usable memory
 *      (`ptr...+=kmalloc_usable_size(ptr)') must be cleared
 *      (`memset(ptr,0,kmalloc_usable_size(ptr))')
 *       Failing to ensure this causes undefined behavior.
 *       If you are not sure if memory is cleared, don't pass
 *       `GFP_CALLOC', or simply use `kfree' */
FUNDEF SAFE NONNULL((1)) void (KCALL __kfree)(void *ptr) ASMNAME("kfree");
FUNDEF SAFE NONNULL((1)) void (KCALL __kffree)(void *ptr, gfp_t flags) ASMNAME("kffree");

/* Return information about a given pointer. */
FUNDEF WUNUSED NONNULL((1)) size_t (KCALL __kmalloc_usable_size)(void *ptr) ASMNAME("kmalloc_usable_size");
FUNDEF WUNUSED NONNULL((1)) gfp_t (KCALL __kmalloc_flags)(void *ptr) ASMNAME("kmalloc_flags");


/* Allocation helper functions. */
FUNDEF SAFE WUNUSED NONNULL((1)) __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __kmemdup)(void const *__restrict ptr, size_t n_bytes,
                                    gfp_t flags) ASMNAME("kmemdup");
FUNDEF SAFE WUNUSED NONNULL((1)) ATTR_ALLOC_ALIGN(2) ATTR_ALLOC_SIZE((3))
ATTR_MALLOC void *(KCALL __kmemadup)(void const *__restrict ptr, size_t alignment,
                                     size_t n_bytes, gfp_t flags) ASMNAME("kmemadup");

/* Define system-wide malloc-options:
 *  - M_TRIM_THRESHOLD: Threshold of the size of blocks, before they are returned to the system.
 *  - M_GRANULARITY:    Amount to overallocate by when allocating memory from lower-level APIs. */
FUNDEF int (KCALL __kmallopt)(int parameter_number, int parameter_value, gfp_t flags) ASMNAME("kmallopt");

// /* Allocate full pages of dynamic memory.
//  * These functions basically act as a high-level re-implementation
//  * of the physical memory allocator, in that they always allocate in
//  * multiples of `PAGESIZE', CEIL_ALIGN()-ing the given `n_bytes' if necessary.
//  * @return: * :         Physical/virtual base address of the newly allocated block of memory.
//  * @return: PAGE_ERROR: Failed to allocate memory. */
// FUNDEF SAFE WUNUSED ppage_t (KCALL kpage_malloc)(size_t n_bytes, gfp_t flags);
// FUNDEF SAFE WUNUSED ppage_t (KCALL kpage_memalign)(size_t alignment, size_t n_bytes, gfp_t flags);
// FUNDEF SAFE WUNUSED ppage_t (KCALL kpage_malloc_in)(ppage_t min, ppage_t max, size_t n_bytes, gfp_t flags);
// FUNDEF SAFE WUNUSED ppage_t (KCALL kpage_realloc)(ppage_t addr, size_t old_n_bytes, size_t new_n_bytes, gfp_t flags);
// FUNDEF SAFE WUNUSED ppage_t (KCALL kpage_realign)(ppage_t addr, size_t alignment, size_t old_n_bytes, size_t new_n_bytes, gfp_t flags);
// FUNDEF SAFE void (KCALL kpage_free)(ppage_t addr, size_t n_bytes, gfp_t flags);


/* Heap pointer definitions. */
#if __SIZEOF_POINTER__ == 8
#ifdef __SIZEOF_INT128__
typedef unsigned __int128 hptr_t;
#if __BYTE_ORDER == __LITTLE_ENDIAN
#   define HPTR(addr,size) ((hptr_t)(uintptr_t)(addr)|((hptr_t)(size) << 64))
#   define HPTR_ADDR(p)    ((HOST void *)(uintptr_t)(p))
#   define HPTR_SIZE(p)    ((size_t)((p) >> 64))
#else
#   define HPTR(addr,size) (((hptr_t)(uintptr_t)(addr) << 64)|(hptr_t)(size))
#   define HPTR_ADDR(p)    ((HOST void *)((uintptr_t)(p) >> 64))
#   define HPTR_SIZE(p)    ((size_t)(p))
#endif
#else /* __SIZEOF_INT128__ */
typedef struct { HOST void *__hp_base; size_t __hp_size; } hptr_t;
#ifdef __GNUC__
#define HPTR(addr,size) ((hptr_t){addr,size})
#else /* __GNUC__ */
FORCELOCAL hptr_t (KCALL __make_hptr)(HOST void *addr, size_t size) { hptr_t result = {addr,size}; return result; }
#define HPTR(addr,size) __make_hptr(addr,size)
#endif /* !__GNUC__ */
#define HPTR_ADDR(p)    ((p).__hp_base)
#define HPTR_SIZE(p)    ((p).__hp_size)
#endif /* !__SIZEOF_INT128__ */
#elif __SIZEOF_POINTER__ == 4
typedef u64 hptr_t;
#if __BYTE_ORDER == __LITTLE_ENDIAN
#   define HPTR(addr,size) ((hptr_t)(uintptr_t)(addr) | ((hptr_t)(size) << 32))
#   define HPTR_ADDR(p)    ((HOST void *)(uintptr_t)(p))
#   define HPTR_SIZE(p)    ((size_t)((p) >> 32))
#else
#   define HPTR(addr,size) (((hptr_t)(uintptr_t)(addr) << 32) | (hptr_t)(size))
#   define HPTR_ADDR(p)    ((HOST void *)(uintptr_t)((p) >> 32))
#   define HPTR_SIZE(p)    ((size_t)(p))
#endif
#else
#error "Unsupported sizeof(void *)"
#endif

/* Special heap pointer values. */
#define HPTR_ERROR     HPTR(0,0)
#define HPTR_ISOK(x)  (HPTR_SIZE(x) != 0)
#define HPTR_ISERR(x) (HPTR_SIZE(x) == 0)


/* Low-level, heap memory alloc/free.
 * NOTE: Heap memory allocation is special, in that every allocation
 *       needs to be at least some internal number of bytes large,
 *       meaning that attempting to allocate less memory produces
 *       unused padding that must still be freed again later.
 * HINT: Input size arguments are ceil-aligned by `HEAP_ALIGNMENT'.
 * WARNING: Do not attempt to free heap-allocated memory using regular free functions.
 *          The specialty of heap memory is that there is no control block that
 *          tracks the allocated size, allowing for much more efficient allocation
 *          of special memory that must be (e.g.) page-aligned.
 * @assume_on_success(IS_ALIGNED(HPTR_SIZE(return)));
 * @return: * :            Base pointer and associated size.
 * @return: HPTR_ISERR(*): Failed to allocate memory. */
FUNDEF SAFE WUNUSED hptr_t KCALL heap_malloc(size_t n_bytes, gfp_t flags);
FUNDEF SAFE WUNUSED hptr_t KCALL heap_malloc_at(HOST void *ptr, size_t n_bytes, gfp_t flags);
FUNDEF SAFE WUNUSED hptr_t KCALL heap_memalign(size_t alignment, size_t offset, size_t n_bytes, gfp_t flags);

/* Free heap memory previously allocated using `heap_*' functions.
 * NOTE: The caller is responsible for passing the same heap
 *       ID as was specified when `heap_malloc()' was called.
 * @assume(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
 * @return: true:  Successfully freed the given heap range.
 * @return: false: Failed to free memory, because `n_bytes' is too small. */
FUNDEF SAFE bool KCALL heap_ffree(hptr_t ptr, gfp_t flags);

#endif /* __CC__ */


/* Kernel memory allocation heaps
 * With the exception of `GFP_LOCKED', which should be
 * used as a flag, always specify exactly _ONE_ of these!. */
#define GFP_NORMAL 0x0000 /*< Normal allocation of memory only accessible from
                           *  ring#0, but shared between all page directories.
                           * (In fact, this is the same as `GFP_SHARED') */
#define GFP_SHARED 0x0000 /*< Allocate memory shared between all page directories. (_ALWAYS_ ZERO(0))
                           *  NOTE: All memory allocated by this is located above `KERNEL_BASE'.
                           *  NOTE: Dispite being mapped in all page directories,
                           *        only the kernel can access this type of memory.
                           *  WARNING: Unless stated otherwise, _ALL_ dynamically allocated
                           *           structures must be allocated as shared (that is they
                           *           are accessible for all page directories) */
#define GFP_LOCKED 0x0001 /*< Allocate in-core-locked virtual memory.
                           *  NOTE: May only or'd together with `GFP_KERNEL' and `GFP_SHARED'
                           *  HINT: You may or- this heap name with `GFP_INCORE' to  */
#define GFP_KERNEL 0x0002 /*< [KPD] Allocate virtual memory only visible to the kernel
                           *  NOTE: All memory allocate by this heap is located below `KERNEL_BASE'. */
#define GFP_MEMORY 0x0004 /*< [KPD] Directly allocate physical memory.
                           *  NOTE: This flag implies `GFP_INCORE' behavior, but does
                           *        not create mman mappings, meaning it must be
                           *        used to allocate mman parts/regions/branches.
                           *  WARNING: `GFP_LOCKED' cannot be used with this heap!
                           *  Instead, memory allocated by this heap features a one-on-one
                           *  mapping of virtual to physical memory (at least in `mman_kernel') */
/*      GFP_...... 0x0008  */
#define __GFP_HEAPCOUNT 5 /*< Amount of different heaps used by the kernel (Use the above macros to address them). */

/* Heap name aliases. */
#define GFP_VIRT   GFP_SHARED
#define GFP_PHYS   GFP_MEMORY

#ifdef GUARD_INCLUDE_KERNEL_PAGING_H
/* Flag for allocating memory locally
 * (visibly in at least the current page directory) */
#if 1
#   define GFP_LOCAL  (PDIR_ISKPD() ? GFP_KERNEL : GFP_SHARED)
#else
#   define GFP_LOCAL                              (GFP_SHARED)
#endif
#endif /* GUARD_INCLUDE_KERNEL_PAGING_H */




/* Additional flags for allocating dynamic memory within the kernel. */
#define GFP_CALLOC 0x0010 /*< Zero-initialize newly allocated memory. */
/*      GFP_...... 0x00e0  *  Unused memory attribute flags. */
#define GFP_NOTRIM 0x0100 /*< Don't trim kernel heaps to free up more memory.
                           *  NOTE: Any heap currently locked will not be trimmed regardless! */
#define GFP_NOSWAP 0x0200 /*< Don't initialize any kind of I/O during swap.
                           *  NOTE: `GFP_NOFS' only prevents write-mapped files from
                           *         being synched, while this flag is required to
                           *         prevents any use of a potential swap partition. */
#define GFP_NOFS   0x0400 /*< Don't sync + unload write-mapped files to free up core memory. */
#define GFP_INCORE 0x0800 /*< Allocate all memory directly. - Don't use allocate-on-access.
                           *  WARNING: Unless the caller is either read, or write-locking
                           *          `mman_kernel.m_lock', memory may have already been
                           *           swapped by the time `kmalloc' and friends return. */
#ifdef CONFIG_MALLOC_NO_FREQUENCY
#define GFP_NOFREQ 0x0000 /*< Ignored... */
#else
#define GFP_NOFREQ 0x1000 /*< Don't spin the malloc frequency edge trigger. - Must be used by functions that may be called during a page fault. */
#endif
#define GFP_ATOMIC 0x2000 /*< Don't preempt, or sleep when waiting for locks. - Spin atomic locks, and fail for all others. */
#define GFP_RQUSER 0x4000 /*< Allocate on behalf of the user (fail when exceeding their allocation quota). */
#define GFP_NOMOVE 0x8000 /*< For `krealloc' & `krealign': Use `realloc_in_place()' semantics. */

#define GFP_NOIO              (GFP_NOSWAP|GFP_NOFS) /*< Don't make use of any kind of I/O when trying to free up available memory. */
#define GFP_NOFREE (GFP_NOTRIM|GFP_NOSWAP|GFP_NOFS) /*< Don't take any actions to free up available memory. */

#define GFP_MASK_FLAGS 0xff10 /*< Mask for memory allocation flags. */
#define GFP_MASK_MPTR  0x0007 /*< Mask for memory flags stored in mptr headers. */
#define GFP_MASK_TYPE  0x0007 /*< Mask for memory type. */
#define GFP_MASK_FREE  0x00f0 /*< Mask for flags stored in free memory ranges.
                               *  NOTE: This is also the mask of page attribute flags (PAGEATTR_*). */

#define GFP_ISVIRT(f)     (((f)&GFP_MASK_TYPE) != GFP_MEMORY)
#define GFP_ISPHYS(f)     (((f)&GFP_MASK_TYPE) == GFP_MEMORY)
#define GFP_GTPAGEATTR(f) (((f)&GFP_MASK_FREE) >> 4)
#define GFP_STPAGEATTR(f)  ((f) << 4)


#ifdef __CC__

#ifdef __USE_DEBUG
#if __USE_DEBUG != 0
FUNDEF SAFE WUNUSED __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((1)) ATTR_MALLOC void *(KCALL _kmalloc_d)(size_t size, gfp_t flags, DEBUGINFO);
FUNDEF SAFE WUNUSED __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((2)) NONNULL((1)) ATTR_MALLOC void *(KCALL _krealloc_d)(void *ptr, size_t size, gfp_t flags, DEBUGINFO);
FUNDEF SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) ATTR_MALLOC void *(KCALL _kmemalign_d)(size_t alignment, size_t size, gfp_t flags, DEBUGINFO);
FUNDEF SAFE WUNUSED ATTR_ALLOC_ALIGN(2) ATTR_ALLOC_SIZE((3)) NONNULL((1)) ATTR_MALLOC void *(KCALL _krealign_d)(void *ptr, size_t alignment, size_t size, gfp_t flags, DEBUGINFO);
FUNDEF SAFE NONNULL((1)) void (KCALL _kfree_d)(void *ptr, DEBUGINFO);
FUNDEF SAFE NONNULL((1)) void (KCALL _kffree_d)(void *ptr, gfp_t flags, DEBUGINFO);
FUNDEF WUNUSED NONNULL((1)) size_t (KCALL _kmalloc_usable_size_d)(void *ptr, DEBUGINFO);
FUNDEF WUNUSED NONNULL((1)) gfp_t (KCALL _kmalloc_flags_d)(void *ptr, DEBUGINFO);
FUNDEF SAFE WUNUSED __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((2)) NONNULL((1)) ATTR_MALLOC void *(KCALL _kmemdup_d)(void const *__restrict ptr, size_t size, gfp_t flags, DEBUGINFO);
FUNDEF SAFE WUNUSED ATTR_ALLOC_ALIGN(2) ATTR_ALLOC_SIZE((3)) NONNULL((1)) ATTR_MALLOC void *(KCALL _kmemadup_d)(void const *__restrict ptr, size_t alignment, size_t size, gfp_t flags, DEBUGINFO);
FUNDEF int (KCALL _kmallopt_d)(int parameter_number, int parameter_value, gfp_t flags, DEBUGINFO);
#else
#   define _kmalloc_d(n_bytes,flags,...)                           __kmalloc(n_bytes,flags)
#   define _kmemalign_d(alignment,n_bytes,flags,...)               __kmemalign(alignment,n_bytes,flags)
#   define _krealloc_d(ptr,n_bytes,flags,...)                      __krealloc(ptr,n_bytes,flags)
#   define _krealign_d(ptr,alignment,n_bytes,flags,...)            __krealign(ptr,alignment,n_bytes,flags)
#   define _kfree_d(ptr,...)                                       __kfree(ptr)
#   define _kffree_d(ptr,flags,...)                                __kffree(ptr,flags)
#   define _kmalloc_usable_size_d(ptr,...)                         __kmalloc_usable_size(ptr)
#   define _kmalloc_flags_d(ptr,...)                               __kmalloc_flags(ptr)
#   define _kmemdup_d(ptr,n_bytes,flags,...)                       __kmemdup(ptr,n_bytes,flags)
#   define _kmemadup_d(ptr,alignment,n_bytes,flags,...)            __kmemadup(ptr,alignment,n_bytes,flags)
#   define _kmallopt_d(parameter_number,parameter_value,flags,...) __kmallopt(parameter_number,parameter_value,flags)
#endif
#endif /* __USE_DEBUG */

#ifdef __OPTIMIZE__
#define _kmalloc(n_bytes,flags)                __kmalloc(n_bytes,flags)
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL _kmemalign)(size_t alignment, size_t n_bytes, gfp_t flags) {
 if (__builtin_constant_p(alignment)) {
  if (alignment <= HEAP_ALIGNMENT)
      return _kmalloc(n_bytes,flags);
 } 
 return __kmemalign(alignment,n_bytes,flags);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL _krealloc)(void *ptr, size_t n_bytes, gfp_t flags) {
 if (__builtin_constant_p(ptr)) {
  if (!ptr) return _kmalloc(n_bytes,flags);
 }
 return __krealloc(ptr,n_bytes,flags);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL _krealign)(void *ptr, size_t alignment,
                                    size_t n_bytes, gfp_t flags) {
 if (__builtin_constant_p(ptr)) {
  if (!ptr) return _kmemalign(alignment,n_bytes,flags);
 }
 if (__builtin_constant_p(alignment)) {
  if (alignment <= HEAP_ALIGNMENT)
      return _krealloc(ptr,n_bytes,flags);
 }
 return __krealign(ptr,alignment,n_bytes,flags);
}
FORCELOCAL SAFE void (KCALL _kfree)(void *ptr) {
 if (__builtin_constant_p(ptr)) { if (!ptr) return; }
 return __kfree(ptr);
}
FORCELOCAL SAFE void (KCALL _kffree)(void *ptr, gfp_t flags) {
 if (__builtin_constant_p(ptr)) { if (!ptr) return; }
 if (__builtin_constant_p(flags)) {
  if (!(flags&(GFP_NOFREQ|GFP_CALLOC))) { __kfree(ptr); return; }
 }
 __kffree(ptr,flags);
}
FORCELOCAL WUNUSED size_t (KCALL _kmalloc_usable_size)(void *ptr) {
 if (__builtin_constant_p(ptr)) { if (!ptr) return 0; }
 return __kmalloc_usable_size(ptr);
}
FORCELOCAL WUNUSED gfp_t (KCALL _kmalloc_flags)(void *ptr) {
 if (__builtin_constant_p(ptr)) { if (!ptr) return 0; }
 return __kmalloc_flags(ptr);
}

FORCELOCAL SAFE WUNUSED __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((1))
ATTR_MALLOC void *(KCALL _kmemdup)(void const *__restrict ptr, size_t n_bytes,
                                   gfp_t flags) {
 if (__builtin_constant_p(flags) ||
     __builtin_constant_p(n_bytes)) {
  register void *result = _kmalloc(n_bytes,flags);
  if (result) memcpy(result,ptr,n_bytes);
  return result;
 }
 return __kmemdup(ptr,n_bytes,flags);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL _kmemadup)(void const *__restrict ptr, size_t alignment,
                                    size_t n_bytes, gfp_t flags) {
 if (__builtin_constant_p(flags) ||
     __builtin_constant_p(n_bytes)) {
  register void *result = _kmemalign(alignment,n_bytes,flags);
  if (result) memcpy(result,ptr,n_bytes);
  return result;
 }
 return __kmemadup(ptr,alignment,n_bytes,flags);
}
#define _kmallopt(parameter_number,parameter_value,flags) \
       __kmallopt(parameter_number,parameter_value,flags)
#ifdef __USE_DEBUG
#if __USE_DEBUG != 0
#define __kmallopt_d(parameter_number,parameter_value,flags,...) \
         _kmallopt_d(parameter_number,parameter_value,flags,__VA_ARGS__)
#define __kmalloc_d(n_bytes,flags,...) _kmalloc_d(n_bytes,flags,__VA_ARGS__)
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __kmemalign_d)(size_t alignment, size_t n_bytes, gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(alignment)) {
  if (alignment <= HEAP_ALIGNMENT)
      return _kmalloc_d(n_bytes,flags,DEBUGINFO_FWD);
 } 
 return _kmemalign_d(alignment,n_bytes,flags,DEBUGINFO_FWD);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __krealloc_d)(void *ptr, size_t n_bytes, gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(ptr)) {
  if (!ptr) return _kmalloc_d(n_bytes,flags,DEBUGINFO_FWD);
 }
 return _krealloc_d(ptr,n_bytes,flags,DEBUGINFO_FWD);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __krealign_d)(void *ptr, size_t alignment,
                                       size_t n_bytes, gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(ptr)) {
  if (!ptr) return _kmemalign_d(alignment,n_bytes,flags,DEBUGINFO_FWD);
 }
 if (__builtin_constant_p(alignment)) {
  if (alignment <= HEAP_ALIGNMENT)
      return _krealloc_d(ptr,n_bytes,flags,DEBUGINFO_FWD);
 }
 return _krealign_d(ptr,alignment,n_bytes,flags,DEBUGINFO_FWD);
}
FORCELOCAL SAFE void (KCALL __kfree_d)(void *ptr, DEBUGINFO) {
 if (__builtin_constant_p(ptr)) { if (!ptr) return; }
 return _kfree_d(ptr,DEBUGINFO_FWD);
}
FORCELOCAL SAFE void (KCALL __kffree_d)(void *ptr, gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(ptr)) { if (!ptr) return; }
 if (__builtin_constant_p(flags)) {
  if (!(flags&(GFP_NOFREQ|GFP_CALLOC))) { _kfree_d(ptr,DEBUGINFO_FWD); return; }
 }
 _kffree_d(ptr,flags,DEBUGINFO_FWD);
}
FORCELOCAL WUNUSED size_t (KCALL __kmalloc_usable_size_d)(void *ptr, DEBUGINFO) {
 if (__builtin_constant_p(ptr)) { if (!ptr) return 0; }
 return _kmalloc_usable_size_d(ptr,DEBUGINFO_FWD);
}
FORCELOCAL WUNUSED gfp_t (KCALL __kmalloc_flags_d)(void *ptr, DEBUGINFO) {
 if (__builtin_constant_p(ptr)) { if (!ptr) return 0; }
 return _kmalloc_flags_d(ptr,DEBUGINFO_FWD);
}

FORCELOCAL SAFE WUNUSED __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((1))
ATTR_MALLOC void *(KCALL __kmemdup_d)(void const *__restrict ptr, size_t n_bytes,
                                      gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(flags) ||
     __builtin_constant_p(n_bytes)) {
  register void *result = _kmalloc_d(n_bytes,flags,DEBUGINFO_FWD);
  if (result) memcpy(result,ptr,n_bytes);
  return result;
 }
 return _kmemdup_d(ptr,n_bytes,flags,DEBUGINFO_FWD);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __kmemadup_d)(void const *__restrict ptr, size_t alignment,
                                       size_t n_bytes, gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(flags) ||
     __builtin_constant_p(n_bytes)) {
  register void *result = _kmemalign_d(alignment,n_bytes,flags,DEBUGINFO_FWD);
  if (result) memcpy(result,ptr,n_bytes);
  return result;
 }
 return _kmemadup_d(ptr,alignment,n_bytes,flags,DEBUGINFO_FWD);
}
#else /* __USE_DEBUG != 0 */
#   define __kmalloc_d(n_bytes,flags,...)                _kmalloc(n_bytes,flags)
#   define __kmemalign_d(alignment,n_bytes,flags,...)    _kmemalign(alignment,n_bytes,flags)
#   define __krealloc_d(ptr,n_bytes,flags,...)           _krealloc(ptr,n_bytes,flags)
#   define __krealign_d(ptr,alignment,n_bytes,flags,...) _krealign(ptr,alignment,n_bytes,flags)
#   define __kfree_d(ptr,...)                            _kfree(ptr)
#   define __kffree_d(ptr,flags,...)                     _kffree(ptr,flags)
#   define __kmalloc_usable_size_d(ptr,...)              _kmalloc_usable_size(ptr)
#   define __kmalloc_flags_d(ptr,...)                    _kmalloc_flags(ptr)
#   define __kmemdup_d(ptr,n_bytes,flags,...)            _kmemdup(ptr,n_bytes,flags)
#   define __kmemadup_d(ptr,alignment,n_bytes,flags,...) _kmemadup(ptr,alignment,n_bytes,flags)
#   define __kmallopt_d(parameter_number,parameter_value,flags,...) \
            _kmallopt(parameter_number,parameter_value,flags)
#endif /* __USE_DEBUG == 0 */
#endif /* __USE_DEBUG */
#else /* __OPTIMIZE__ */
#   define _kmalloc(n_bytes,flags)                __kmalloc(n_bytes,flags)
#   define _kmemalign(alignment,n_bytes,flags)    __kmemalign(alignment,n_bytes,flags)
#   define _krealloc(ptr,n_bytes,flags)           __krealloc(ptr,n_bytes,flags)
#   define _krealign(ptr,alignment,n_bytes,flags) __krealign(ptr,alignment,n_bytes,flags)
#   define _kfree(ptr)                            __kfree(ptr)
#   define _kffree(ptr,flags)                     __kffree(ptr,flags)
#   define _kmalloc_usable_size(ptr)              __kmalloc_usable_size(ptr)
#   define _kmalloc_flags(ptr)                    __kmalloc_flags(ptr)
#   define _kmemdup(ptr,n_bytes,flags)            __kmemdup(ptr,n_bytes,flags)
#   define _kmemadup(ptr,alignment,n_bytes,flags) __kmemadup(ptr,alignment,n_bytes,flags)
#   define _kmallopt(parameter_number,parameter_value,flags) \
          __kmallopt(parameter_number,parameter_value,flags)
#ifdef __USE_DEBUG
#if __USE_DEBUG != 0
#   define __kmalloc_d(n_bytes,flags,...)                _kmalloc_d(n_bytes,flags,__VA_ARGS__)
#   define __kmemalign_d(alignment,n_bytes,flags,...)    _kmemalign_d(alignment,n_bytes,flags,__VA_ARGS__)
#   define __krealloc_d(ptr,n_bytes,flags,...)           _krealloc_d(ptr,n_bytes,flags,__VA_ARGS__)
#   define __krealign_d(ptr,alignment,n_bytes,flags,...) _krealign_d(ptr,alignment,n_bytes,flags,__VA_ARGS__)
#   define __kfree_d(ptr,...)                            _kfree_d(ptr,__VA_ARGS__)
#   define __kffree_d(ptr,flags,...)                     _kffree_d(ptr,flags,__VA_ARGS__)
#   define __kmalloc_usable_size_d(ptr,...)              _kmalloc_usable_size_d(ptr,__VA_ARGS__)
#   define __kmalloc_flags_d(ptr,...)                    _kmalloc_flags_d(ptr,__VA_ARGS__)
#   define __kmemdup_d(ptr,n_bytes,flags,...)            _kmemdup_d(ptr,n_bytes,flags,__VA_ARGS__)
#   define __kmemadup_d(ptr,alignment,n_bytes,flags,...) _kmemadup_d(ptr,alignment,n_bytes,flags,__VA_ARGS__)
#   define __kmallopt_d(parameter_number,parameter_value,flags,...) \
            _kmallopt_d(parameter_number,parameter_value,flags,__VA_ARGS__)
#else /* __USE_DEBUG != 0 */
#   define __kmalloc_d(n_bytes,flags,...)                __kmalloc(n_bytes,flags)
#   define __kmemalign_d(alignment,n_bytes,flags,...)    __kmemalign(alignment,n_bytes,flags)
#   define __krealloc_d(ptr,n_bytes,flags,...)           __krealloc(ptr,n_bytes,flags)
#   define __krealign_d(ptr,alignment,n_bytes,flags,...) __krealign(ptr,alignment,n_bytes,flags)
#   define __kfree_d(ptr,...)                            __kfree(ptr)
#   define __kffree_d(ptr,flags,...)                     __kffree(ptr,flags)
#   define __kmalloc_usable_size_d(ptr,...)              __kmalloc_usable_size(ptr)
#   define __kmalloc_flags_d(ptr,...)                    __kmalloc_flags(ptr)
#   define __kmemdup_d(ptr,n_bytes,flags,...)            __kmemdup(ptr,n_bytes,flags)
#   define __kmemadup_d(ptr,alignment,n_bytes,flags,...) __kmemadup(ptr,alignment,n_bytes,flags)
#   define __kmallopt_d(parameter_number,parameter_value,flags,...) \
           __kmallopt(parameter_number,parameter_value,flags)
#endif /* __USE_DEBUG == 0 */
#endif /* __USE_DEBUG */
#endif /* !__OPTIMIZE__ */

#ifdef __USE_DEBUG_HOOK
#   define kmalloc(n_bytes,flags)                __kmalloc_d(n_bytes,flags,DEBUGINFO_GEN)
#   define kmemalign(alignment,n_bytes,flags)    __kmemalign_d(alignment,n_bytes,flags,DEBUGINFO_GEN)
#   define krealloc(ptr,n_bytes,flags)           __krealloc_d(ptr,n_bytes,flags,DEBUGINFO_GEN)
#   define krealign(ptr,alignment,n_bytes,flags) __krealign_d(ptr,alignment,n_bytes,flags,DEBUGINFO_GEN)
#   define kfree(ptr)                            __kfree_d(ptr,DEBUGINFO_GEN)
#   define kffree(ptr,flags)                     __kffree_d(ptr,flags,DEBUGINFO_GEN)
#   define kmalloc_usable_size(ptr)              __kmalloc_usable_size_d(ptr,DEBUGINFO_GEN)
#   define kmalloc_flags(ptr)                    __kmalloc_flags_d(ptr,DEBUGINFO_GEN)
#   define kmemdup(ptr,n_bytes,flags)            __kmemdup_d(ptr,n_bytes,flags,DEBUGINFO_GEN)
#   define kmemadup(ptr,alignment,n_bytes,flags) __kmemadup_d(ptr,alignment,n_bytes,flags,DEBUGINFO_GEN)
#   define kmallopt(parameter_number,parameter_value,flags) \
       __kmallopt_d(parameter_number,parameter_value,flags,DEBUGINFO_GEN)
#else
#   define kmalloc(n_bytes,flags)                _kmalloc(n_bytes,flags)
#   define kmemalign(alignment,n_bytes,flags)    _kmemalign(alignment,n_bytes,flags)
#   define krealloc(ptr,n_bytes,flags)           _krealloc(ptr,n_bytes,flags)
#   define krealign(ptr,alignment,n_bytes,flags) _krealign(ptr,alignment,n_bytes,flags)
#   define kfree(ptr)                            _kfree(ptr)
#   define kffree(ptr,flags)                     _kffree(ptr,flags)
#   define kmalloc_usable_size(ptr)              _kmalloc_usable_size(ptr)
#   define kmalloc_flags(ptr)                    _kmalloc_flags(ptr)
#   define kmemdup(ptr,n_bytes,flags)            _kmemdup(ptr,n_bytes,flags)
#   define kmemadup(ptr,alignment,n_bytes,flags) _kmemadup(ptr,alignment,n_bytes,flags)
#   define kmallopt(parameter_number,parameter_value,flags) \
          _kmallopt(parameter_number,parameter_value,flags)
#endif

/* Helper macros for intrinsic functionality. */
#define kcalloc(n_bytes,flags)                 kmalloc((n_bytes),(flags)|GFP_CALLOC)
#define krecalign(ptr,alignment,n_bytes,flags) krealign(ptr,alignment,n_bytes,(flags)|GFP_CALLOC)
#define krecalloc(ptr,n_bytes,flags)           krealloc(ptr,n_bytes,(flags)|GFP_CALLOC)
#define krealloc_in_place(ptr,n_bytes,flags)   krealloc(ptr,n_bytes,(flags)|GFP_NOMOVE)
#define krecalloc_in_place(ptr,n_bytes,flags)  krealloc(ptr,n_bytes,(flags)|GFP_NOMOVE|GFP_CALLOC)

#endif /* __CC__ */


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_MALLOC_H */
