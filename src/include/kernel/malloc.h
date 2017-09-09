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

#include <stddef.h>
#include <stdbool.h>
#include <hybrid/compiler.h>
#include <features.h>
#include <__malldefs.h>
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
 * ever returned by 'kmalloc_usable_size'.
 * NOTE: This value can be anything!
 */
#ifndef HEAP_ALIGNMENT
#define HEAP_ALIGNMENT  __MALL_MIN_ALIGNMENT
#endif /* !HEAP_ALIGNMENT */

#ifdef __CC__
typedef __UINT16_TYPE__ gfp_t;

/* Truncate unused memory, pre-allocated memory from
 * all heaps, at most releasing 'max_free' bytes of mapped
 * memory back to the system (physical memory allocator).
 * NOTE: This function is called during the early phases of 'mman_swapmem' in order
 *       to clean up small portions of memory oftenly available after dynamic memory
 *       allocation has been used for some time.
 * NOTE: Unused heap memory is always freed once a single continuous block of data
 *       exceeds 'M_TRIM_THRESHOLD', effectively meaning that this function releases
 *       all memory as though 'M_TRIM_THRESHOLD' had always been set to 'PAGESIZE'
 *      (Its minimum effective value).
 * @return: * : The total amount of released bytes of memory. */
FUNDEF SAFE size_t (KCALL kmalloc_trim)(size_t max_free);

/* Allocate kernel memory.
 * @param: flags: A set of 'GFP_*' */
FUNDEF SAFE WUNUSED __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((1))
ATTR_MALLOC void *(KCALL __kmalloc)(size_t size, gfp_t flags) ASMNAME("kmalloc");
FUNDEF SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __kmemalign)(size_t alignment, size_t size, gfp_t flags) ASMNAME("kmemalign");

/* Reallocate existing memory.
 * @param: flags: A set of 'GFP_*' matching 'GFP_MASK_FLAGS'
 *          NOTE: When 'ptr' is NULL, 'flags' may also include any other 'GFP_*' heap-designator flag(s).
 *                Otherwise, heap designators are overwritten by those originally used to allocate 'ptr'. */
FUNDEF SAFE WUNUSED NONNULL((1)) __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __krealloc)(void *ptr, size_t size, gfp_t flags) ASMNAME("krealloc");
FUNDEF SAFE WUNUSED NONNULL((1)) ATTR_ALLOC_ALIGN(2) ATTR_ALLOC_SIZE((3))
ATTR_MALLOC void *(KCALL __krealign)(void *ptr, size_t alignment, size_t size, gfp_t flags) ASMNAME("krealign");

/* Free a given kernel-allocated pointer 'ptr'.
 * NOTE: When passing GFP_CALLOC to 'kffree', all usable memory
 *      ('ptr...+=kmalloc_usable_size(ptr)') must be cleared
 *      ('memset(ptr,0,kmalloc_usable_size(ptr))')
 *       Failing to ensure this causes undefined behavior.
 *       If you are not sure if memory is cleared, don't pass
 *       'GFP_CALLOC', or simply use 'kfree' */
FUNDEF SAFE NONNULL((1)) void (KCALL __kfree)(void *ptr) ASMNAME("kfree");
FUNDEF SAFE NONNULL((1)) void (KCALL __kffree)(void *ptr, gfp_t flags) ASMNAME("kffree");

/* Return information about a given pointer. */
FUNDEF WUNUSED NONNULL((1)) size_t (KCALL __kmalloc_usable_size)(void *ptr) ASMNAME("kmalloc_usable_size");
FUNDEF WUNUSED NONNULL((1)) gfp_t (KCALL __kmalloc_flags)(void *ptr) ASMNAME("kmalloc_flags");


/* Allocation helper functions. */
FUNDEF SAFE WUNUSED NONNULL((1)) __MALL_DEFAULT_ALIGNED ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __kmemdup)(void const *__restrict ptr, size_t size,
                                    gfp_t flags) ASMNAME("kmemdup");
FUNDEF SAFE WUNUSED NONNULL((1)) ATTR_ALLOC_ALIGN(2) ATTR_ALLOC_SIZE((3))
ATTR_MALLOC void *(KCALL __kmemadup)(void const *__restrict ptr, size_t alignment,
                                     size_t size, gfp_t flags) ASMNAME("kmemadup");

/* Define system-wide malloc-options:
 *  - M_TRIM_THRESHOLD: Threshold of the size of blocks, before they are returned to the system.
 *  - M_GRANULARITY:    Amount to overallocate by when allocating memory from lower-level APIs. */
FUNDEF int (KCALL __kmallopt)(int parameter_number, int parameter_value, gfp_t flags) ASMNAME("kmallopt");
#endif /* __CC__ */


/* Kernel memory allocation heaps
 * With the exception of 'GFP_LOCKED', which should be
 * used as a flag, always specify exactly _ONE_ of these!. */
#define GFP_NORMAL 0x0000 /*< Normal allocation of memory only accessible from
                           *  ring#0, but shared between all page directories.
                           * (In fact, this is the same as 'GFP_SHARED') */
#define GFP_SHARED 0x0000 /*< Allocate memory shared between all page directories. (_ALWAYS_ ZERO(0))
                           *  NOTE: All memory allocated by this is located above 'KERNEL_BASE'.
                           *  NOTE: Dispite being mapped in all page directories,
                           *        only the kernel can access this type of memory.
                           *  WARNING: Unless stated otherwise, _ALL_ dynamically allocated
                           *           structures must be allocated as shared (that is they
                           *           are accessible for all page directories) */
#define GFP_LOCKED 0x0001 /*< Allocate in-core-locked virtual memory.
                           *  NOTE: May only or'd together with 'GFP_KERNEL' and 'GFP_SHARED'
                           *  HINT: You may or' this heap name with 'GFP_INCORE' to  */
#define GFP_KERNEL 0x0002 /*< [KPD] Allocate virtual memory only visible to the kernel
                           *  NOTE: All memory allocate by this heap is located below 'KERNEL_BASE'. */
#define GFP_MEMORY 0x0004 /*< [KPD] Directly allocate physical memory.
                           *  NOTE: This flag implies 'GFP_INCORE' behavior, but does
                           *        not create mman mappings, meaning it must also
                           *        be used to allocate mman parts/regions/branches. */
#define __GFP_HEAPCOUNT 5 /*< Amount of different heaps used by the kernel (Use the above macros to address them). */
/*      GFP_...... 0x0008  */

/* Heap name aliases. */
#define GFP_VIRT   GFP_SHARED
#define GFP_PHYS   GFP_MEMORY

#ifdef GUARD_INCLUDE_KERNEL_PAGING_H
/* Flag for allocating memory locally
 * (visibly in at least the current page directory) */
#if 1
#define GFP_LOCAL  (PDIR_ISKPD() ? GFP_KERNEL : GFP_SHARED)
#else
#define GFP_LOCAL                              (GFP_SHARED)
#endif
#endif /* GUARD_INCLUDE_KERNEL_PAGING_H */




/* Additional flags for allocating dynamic memory within the kernel. */
#define GFP_CALLOC 0x0010 /*< Zero-initialize newly allocated memory. */
/*      GFP_...... 0x00e0  */
#define GFP_NOTRIM 0x0100 /*< Don't trim kernel heaps to free up more memory.
                           *  NOTE: Any heap currently locked will not be trimmed! */
#define GFP_NOSWAP 0x0200 /*< Don't initialize any kind of I/O during swap.
                           *  NOTE: 'GFP_NOFS' only prevents write-mapped files from
                           *         being flushed, while this flag is required to
                           *         prevents any use of a potential swap partition. */
#define GFP_NOFS   0x0400 /*< Don't sync + unload write-mapped files to free up core memory. */
#define GFP_INCORE 0x0800 /*< Allocate all memory directly. - Don't use allocate-on-access.
                           *  WARNING: Unless the caller is either read, or write-locking
                           *          'mman_kernel.m_lock', memory may have already been
                           *           swapped by the time 'kmalloc' and friends return. */
#ifdef CONFIG_MALLOC_NO_FREQUENCY
#define GFP_NOFREQ 0x0000 /*< Ignored... */
#else
#define GFP_NOFREQ 0x1000 /*< Don't spin the malloc frequency edge trigger. - Must be used by functions that may called during a page fault. */
#endif
#define GFP_ATOMIC 0x2000 /*< Don't preempt, or sleep when waiting for locks. - Spin atomic locks, and fail for all others. */
#define GFP_RQUSER 0x4000 /*< Allocate on behalf of the user (fail when exceeding their allocation quota). */
#define GFP_NOMOVE 0x8000 /*< For 'krealloc' & 'krealign': Use 'realloc_in_place()' semantics. */

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
#   define _kmalloc_d(size,flags,...)                              __kmalloc(size,flags)
#   define _kmemalign_d(alignment,size,flags,...)                  __kmemalign(alignment,size,flags)
#   define _krealloc_d(ptr,size,flags,...)                         __krealloc(ptr,size,flags)
#   define _krealign_d(ptr,alignment,size,flags,...)               __krealign(ptr,alignment,size,flags)
#   define _kfree_d(ptr,...)                                       __kfree(ptr)
#   define _kffree_d(ptr,flags,...)                                __kffree(ptr,flags)
#   define _kmalloc_usable_size_d(ptr,...)                         __kmalloc_usable_size(ptr)
#   define _kmalloc_flags_d(ptr,...)                               __kmalloc_flags(ptr)
#   define _kmemdup_d(ptr,size,flags,...)                          __kmemdup(ptr,size,flags)
#   define _kmemadup_d(ptr,alignment,size,flags,...)               __kmemadup(ptr,alignment,size,flags)
#   define _kmallopt_d(parameter_number,parameter_value,flags,...) __kmallopt(parameter_number,parameter_value,flags)
#endif
#endif /* __USE_DEBUG */

#ifdef __OPTIMIZE__
#define _kmalloc(size,flags)                __kmalloc(size,flags)
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL _kmemalign)(size_t alignment, size_t size, gfp_t flags) {
 if (__builtin_constant_p(alignment)) {
  if (alignment <= HEAP_ALIGNMENT)
      return _kmalloc(size,flags);
 } 
 return __kmemalign(alignment,size,flags);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL _krealloc)(void *ptr, size_t size, gfp_t flags) {
 if (__builtin_constant_p(ptr)) {
  if (!ptr) return _kmalloc(size,flags);
 }
 return __krealloc(ptr,size,flags);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL _krealign)(void *ptr, size_t alignment,
                                    size_t size, gfp_t flags) {
 if (__builtin_constant_p(ptr)) {
  if (!ptr) return _kmemalign(alignment,size,flags);
 }
 if (__builtin_constant_p(alignment)) {
  if (alignment <= HEAP_ALIGNMENT)
      return _krealloc(ptr,size,flags);
 }
 return __krealign(ptr,alignment,size,flags);
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
ATTR_MALLOC void *(KCALL _kmemdup)(void const *__restrict ptr, size_t size,
                                   gfp_t flags) {
 if (__builtin_constant_p(flags) ||
     __builtin_constant_p(size)) {
  register void *result = _kmalloc(size,flags);
  if (result) memcpy(result,ptr,size);
  return result;
 }
 return __kmemdup(ptr,size,flags);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL _kmemadup)(void const *__restrict ptr, size_t alignment,
                                    size_t size, gfp_t flags) {
 if (__builtin_constant_p(flags) ||
     __builtin_constant_p(size)) {
  register void *result = _kmemalign(alignment,size,flags);
  if (result) memcpy(result,ptr,size);
  return result;
 }
 return __kmemadup(ptr,alignment,size,flags);
}
#define _kmallopt(parameter_number,parameter_value,flags) \
       __kmallopt(parameter_number,parameter_value,flags)
#ifdef __USE_DEBUG
#if __USE_DEBUG != 0
#define __kmallopt_d(parameter_number,parameter_value,flags,...) \
         _kmallopt_d(parameter_number,parameter_value,flags,__VA_ARGS__)
#define __kmalloc_d(size,flags,...) _kmalloc_d(size,flags,__VA_ARGS__)
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __kmemalign_d)(size_t alignment, size_t size, gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(alignment)) {
  if (alignment <= HEAP_ALIGNMENT)
      return _kmalloc_d(size,flags,DEBUGINFO_FWD);
 } 
 return _kmemalign_d(alignment,size,flags,DEBUGINFO_FWD);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __krealloc_d)(void *ptr, size_t size, gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(ptr)) {
  if (!ptr) return _kmalloc_d(size,flags,DEBUGINFO_FWD);
 }
 return _krealloc_d(ptr,size,flags,DEBUGINFO_FWD);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __krealign_d)(void *ptr, size_t alignment,
                                       size_t size, gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(ptr)) {
  if (!ptr) return _kmemalign_d(alignment,size,flags,DEBUGINFO_FWD);
 }
 if (__builtin_constant_p(alignment)) {
  if (alignment <= HEAP_ALIGNMENT)
      return _krealloc_d(ptr,size,flags,DEBUGINFO_FWD);
 }
 return _krealign_d(ptr,alignment,size,flags,DEBUGINFO_FWD);
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
ATTR_MALLOC void *(KCALL __kmemdup_d)(void const *__restrict ptr, size_t size,
                                      gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(flags) ||
     __builtin_constant_p(size)) {
  register void *result = _kmalloc_d(size,flags,DEBUGINFO_FWD);
  if (result) memcpy(result,ptr,size);
  return result;
 }
 return _kmemdup_d(ptr,size,flags,DEBUGINFO_FWD);
}
FORCELOCAL SAFE WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2))
ATTR_MALLOC void *(KCALL __kmemadup_d)(void const *__restrict ptr, size_t alignment,
                                       size_t size, gfp_t flags, DEBUGINFO) {
 if (__builtin_constant_p(flags) ||
     __builtin_constant_p(size)) {
  register void *result = _kmemalign_d(alignment,size,flags,DEBUGINFO_FWD);
  if (result) memcpy(result,ptr,size);
  return result;
 }
 return _kmemadup_d(ptr,alignment,size,flags,DEBUGINFO_FWD);
}
#else /* __USE_DEBUG != 0 */
#   define __kmalloc_d(size,flags,...)                _kmalloc(size,flags)
#   define __kmemalign_d(alignment,size,flags,...)    _kmemalign(alignment,size,flags)
#   define __krealloc_d(ptr,size,flags,...)           _krealloc(ptr,size,flags)
#   define __krealign_d(ptr,alignment,size,flags,...) _krealign(ptr,alignment,size,flags)
#   define __kfree_d(ptr,...)                         _kfree(ptr)
#   define __kffree_d(ptr,flags,...)                  _kffree(ptr,flags)
#   define __kmalloc_usable_size_d(ptr,...)           _kmalloc_usable_size(ptr)
#   define __kmalloc_flags_d(ptr,...)                 _kmalloc_flags(ptr)
#   define __kmemdup_d(ptr,size,flags,...)            _kmemdup(ptr,size,flags)
#   define __kmemadup_d(ptr,alignment,size,flags,...) _kmemadup(ptr,alignment,size,flags)
#   define __kmallopt_d(parameter_number,parameter_value,flags,...) \
            _kmallopt(parameter_number,parameter_value,flags)
#endif /* __USE_DEBUG == 0 */
#endif /* __USE_DEBUG */
#else /* __OPTIMIZE__ */
#   define _kmalloc(size,flags)                __kmalloc(size,flags)
#   define _kmemalign(alignment,size,flags)    __kmemalign(alignment,size,flags)
#   define _krealloc(ptr,size,flags)           __krealloc(ptr,size,flags)
#   define _krealign(ptr,alignment,size,flags) __krealign(ptr,alignment,size,flags)
#   define _kfree(ptr)                         __kfree(ptr)
#   define _kffree(ptr,flags)                  __kffree(ptr,flags)
#   define _kmalloc_usable_size(ptr)           __kmalloc_usable_size(ptr)
#   define _kmalloc_flags(ptr)                 __kmalloc_flags(ptr)
#   define _kmemdup(ptr,size,flags)            __kmemdup(ptr,size,flags)
#   define _kmemadup(ptr,alignment,size,flags) __kmemadup(ptr,alignment,size,flags)
#   define _kmallopt(parameter_number,parameter_value,flags) \
          __kmallopt(parameter_number,parameter_value,flags)
#ifdef __USE_DEBUG
#if __USE_DEBUG != 0
#   define __kmalloc_d(size,flags,...)                _kmalloc_d(size,flags,__VA_ARGS__)
#   define __kmemalign_d(alignment,size,flags,...)    _kmemalign_d(alignment,size,flags,__VA_ARGS__)
#   define __krealloc_d(ptr,size,flags,...)           _krealloc_d(ptr,size,flags,__VA_ARGS__)
#   define __krealign_d(ptr,alignment,size,flags,...) _krealign_d(ptr,alignment,size,flags,__VA_ARGS__)
#   define __kfree_d(ptr,...)                         _kfree_d(ptr,__VA_ARGS__)
#   define __kffree_d(ptr,flags,...)                  _kffree_d(ptr,flags,__VA_ARGS__)
#   define __kmalloc_usable_size_d(ptr,...)           _kmalloc_usable_size_d(ptr,__VA_ARGS__)
#   define __kmalloc_flags_d(ptr,...)                 _kmalloc_flags_d(ptr,__VA_ARGS__)
#   define __kmemdup_d(ptr,size,flags,...)            _kmemdup_d(ptr,size,flags,__VA_ARGS__)
#   define __kmemadup_d(ptr,alignment,size,flags,...) _kmemadup_d(ptr,alignment,size,flags,__VA_ARGS__)
#   define __kmallopt_d(parameter_number,parameter_value,flags,...) \
            _kmallopt_d(parameter_number,parameter_value,flags,__VA_ARGS__)
#else /* __USE_DEBUG != 0 */
#   define __kmalloc_d(size,flags,...)                __kmalloc(size,flags)
#   define __kmemalign_d(alignment,size,flags,...)    __kmemalign(alignment,size,flags)
#   define __krealloc_d(ptr,size,flags,...)           __krealloc(ptr,size,flags)
#   define __krealign_d(ptr,alignment,size,flags,...) __krealign(ptr,alignment,size,flags)
#   define __kfree_d(ptr,...)                         __kfree(ptr)
#   define __kffree_d(ptr,flags,...)                  __kffree(ptr,flags)
#   define __kmalloc_usable_size_d(ptr,...)           __kmalloc_usable_size(ptr)
#   define __kmalloc_flags_d(ptr,...)                 __kmalloc_flags(ptr)
#   define __kmemdup_d(ptr,size,flags,...)            __kmemdup(ptr,size,flags)
#   define __kmemadup_d(ptr,alignment,size,flags,...) __kmemadup(ptr,alignment,size,flags)
#   define __kmallopt_d(parameter_number,parameter_value,flags,...) \
           __kmallopt(parameter_number,parameter_value,flags)
#endif /* __USE_DEBUG == 0 */
#endif /* __USE_DEBUG */
#endif /* !__OPTIMIZE__ */

#ifdef __USE_DEBUG_HOOK
#   define kmalloc(size,flags)                __kmalloc_d(size,flags,DEBUGINFO_GEN)
#   define kmemalign(alignment,size,flags)    __kmemalign_d(alignment,size,flags,DEBUGINFO_GEN)
#   define krealloc(ptr,size,flags)           __krealloc_d(ptr,size,flags,DEBUGINFO_GEN)
#   define krealign(ptr,alignment,size,flags) __krealign_d(ptr,alignment,size,flags,DEBUGINFO_GEN)
#   define kfree(ptr)                         __kfree_d(ptr,DEBUGINFO_GEN)
#   define kffree(ptr,flags)                  __kffree_d(ptr,flags,DEBUGINFO_GEN)
#   define kmalloc_usable_size(ptr)           __kmalloc_usable_size_d(ptr,DEBUGINFO_GEN)
#   define kmalloc_flags(ptr)                 __kmalloc_flags_d(ptr,DEBUGINFO_GEN)
#   define kmemdup(ptr,size,flags)            __kmemdup_d(ptr,size,flags,DEBUGINFO_GEN)
#   define kmemadup(ptr,alignment,size,flags) __kmemadup_d(ptr,alignment,size,flags,DEBUGINFO_GEN)
#   define kmallopt(parameter_number,parameter_value,flags) \
       __kmallopt_d(parameter_number,parameter_value,flags,DEBUGINFO_GEN)
#else
#   define kmalloc(size,flags)                _kmalloc(size,flags)
#   define kmemalign(alignment,size,flags)    _kmemalign(alignment,size,flags)
#   define krealloc(ptr,size,flags)           _krealloc(ptr,size,flags)
#   define krealign(ptr,alignment,size,flags) _krealign(ptr,alignment,size,flags)
#   define kfree(ptr)                         _kfree(ptr)
#   define kffree(ptr,flags)                  _kffree(ptr,flags)
#   define kmalloc_usable_size(ptr)           _kmalloc_usable_size(ptr)
#   define kmalloc_flags(ptr)                 _kmalloc_flags(ptr)
#   define kmemdup(ptr,size,flags)            _kmemdup(ptr,size,flags)
#   define kmemadup(ptr,alignment,size,flags) _kmemadup(ptr,alignment,size,flags)
#   define kmallopt(parameter_number,parameter_value,flags) \
          _kmallopt(parameter_number,parameter_value,flags)
#endif

/* Helper macros for intrinsic functionality. */
#define kcalloc(size,flags)                 kmalloc((size),(flags)|GFP_CALLOC)
#define krecalign(ptr,alignment,size,flags) krealign(ptr,alignment,size,(flags)|GFP_CALLOC)
#define krecalloc(ptr,size,flags)           krealloc(ptr,size,(flags)|GFP_CALLOC)
#define krealloc_in_place(ptr,size,flags)   krealloc(ptr,size,(flags)|GFP_NOMOVE)
#define krecalloc_in_place(ptr,size,flags)  krealloc(ptr,size,(flags)|GFP_NOMOVE|GFP_CALLOC)

#endif /* __CC__ */


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_MALLOC_H */
