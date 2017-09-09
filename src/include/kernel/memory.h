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
#ifndef GUARD_INCLUDE_KERNEL_MEMORY_H
#define GUARD_INCLUDE_KERNEL_MEMORY_H 1

#include <format-printer.h>
#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <hybrid/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

#define PAGE_ERROR        COMPILER_UNIPOINTER(-1)

#ifdef __CC__
typedef u32 pgattr_t; /* Page attributes (Set of 'PAGEATTR_*') */
#endif /* __CC__ */
#define PAGEATTR_NONE     0x00000000
#define PAGEATTR_ZERO     0x00000001 /* Page memory (except for the free block) is zero-initialized. */

/* Attributes returned by 'page_query()' */
#define PAGEATTR_FREE     0x00000100 /* The page is currently marked as free. */
#define PAGEATTR_ALLSHIFT 16
#define PAGEATTR_ALLZERO (PAGEATTR_ZERO << PAGEATTR_ALLSHIFT)
#define PAGEATTR_ALLFREE (PAGEATTR_FREE << PAGEATTR_ALLSHIFT)

#ifdef __CC__
/* NOTE: Memory pages are _always_ physical, meaning that the user
 *       must have the kernel page directory set before allocating any. */
typedef union page        page_t;
typedef PHYS PAGE_ALIGNED page_t *ppage_t;
#endif /* __CC__ */


#ifdef __CC__
union page {
 /* NOTE: Memory pages from different zones are usually not connected. */
               byte_t   p_data[PAGESIZE]; /* Allocated page data. */
struct{
          PHYS ppage_t  p_next; /*< [0..1|null(PAGE_ERROR)][>= (ppage_t)((uintptr_t)this+this->p_free.p_size)]
                                 *   Next free page-range at a greater physical memory address. */
          PHYS ppage_t *p_self; /*< [1..1] Pointer to field containing a self-pointer. */
  PAGE_ALIGNED size_t   p_size; /*< [!0] Size of this free page-range (in bytes). */
               pgattr_t p_attr; /*< Set of 'PAGEATTR_*'. */
}                       p_free; /*< Free page controller. */
};
#define PAGE_END(p) ((ppage_t)((uintptr_t)(p)+(p)->p_free.p_size))


STATIC_ASSERT(sizeof(((ppage_t)0)->p_free) <=
              sizeof(((ppage_t)0)->p_data));


/* Query page attributes, returning a set of 'PAGEATTR_*' */
FUNDEF KPD pgattr_t KCALL page_query(PHYS void *start, size_t n_bytes);
#endif /* __CC__ */


/* --- Physical memory zones ---
 * NOTE: All zones implicitly include other zones with lower IDs,
 *       but allocation will usually try to refrain allocating
 *       pointers from a zone other than the explicitly requested.
 * NOTE: In addition to this, any zone with a greater address than
 *       another starts after the other, meaning that zones can be
 *       searched linearly to figure out the origin of any given pointer.
 */
#define MZONE_1MB     0x00 /* [0x00000000..0x0009ffff] Physical, dynamic memory within the first 1Mb of memory.
                            *  HINT: This is where 16-bit/realmode code running in ring#0 must be loaded to. */
#define MZONE_DEV     0x01 /* [0x000a0000..0x000fffff] Device memory mappings. This zone is always empty, as it cannot be used for dynamic allocations. */
#define MZONE_SHARE   0x02 /* [0x00100000..0x3fffffff] The physical address space where the kernel itself is located within. */
#define MZONE_NOSHARE 0x03 /* [0x40000000..0xbfffffff] Physical memory that can never be apart of the static kernel image. */
#define MZONE_INVALID 0x04 /* Invalid memory zone. */
#define MZONE_ANY     MZONE_NOSHARE

#define MZONE_COUNT   0x04 /* The real amount of zones there are. */
#ifdef __CC__
typedef unsigned int mzone_t;
#endif /* __CC__ */

#ifdef __CC__
/* The logical min/max addresses of memory zones. */
struct mzonespec {
#ifdef __INTELLISENSE__
      uintptr_t ms_min[MZONE_COUNT]; /* Lowest virtual part of a given memory zone. */
      uintptr_t ms_max[MZONE_COUNT]; /* Greatest virtual part of a given memory zone. */
#else
 PHYS uintptr_t ms_min[MZONE_COUNT]; /* Lowest virtual part of a given memory zone. */
 PHYS uintptr_t ms_max[MZONE_COUNT]; /* Greatest virtual part of a given memory zone. */
#endif
};
DATDEF struct mzonespec const mzone_spec;
#define MZONE_MIN(zone) mzone_spec.ms_min[zone]
#define MZONE_MAX(zone) mzone_spec.ms_max[zone]

LOCAL mzone_t KCALL mzone_of(PHYS void *ptr) {
 mzone_t result = MZONE_COUNT; do --result;
 while (ptr < (PHYS void *)MZONE_MIN(result));
 return result;
}



struct meminfo {
 PHYS struct meminfo const *mi_next;  /*< [0..1][->mi_start > mi_start+mi_size][const] Next info link. */
#ifdef __INTELLISENSE__
      ppage_t               mi_start; /*< [const] First associated page. */
              size_t        mi_size;  /*< [const] Amount of bytes part of this region. */
#else
 PHYS ppage_t               mi_start; /*< [const] First associated page. */
 PAGE_ALIGNED size_t        mi_size;  /*< [const] Amount of bytes part of this region. */
#endif
};
/* Per-zone information about memory available for dynamic allocation.
 * NOTE: This information is allocated during bootup and is never modified afterwards.
 * WARNING: This structure is allocated in 'MZONE_NOSHARE' using physical
 *          addresses, meaning that access requires the kernel page directory! */
DATDEF PHYS struct meminfo const *const mem_info[MZONE_COUNT];
#define MEMINFO_FOREACH(iter,zone) \
 for ((iter) = mem_info[zone]; \
      (iter) != NULL; (iter) = (iter)->mi_next)



/* Dynamically allocate page memory, returning a pointer to the allocated block.
 * NOTE: Unaligned allocation requests are ceil-aligned.
 * NOTE: Passing ZERO(0) for 'n_bytes' will allocate an empty page
 *      (that is a page that may be aliasing another pointer and
 *       doesn't have to be freed, or can be freed through 'page_free(p,0)')
 *       Though that page is still guarantied to be page-aligned and not be
 *       equal to 'PAGE_ERROR', unless no dynamic memory what-so-ever is
 *       available for use, in which case 'PAGE_ERROR' may still be returned.
 * @param: n_bytes:     The min. amount of continuous bytes to-be returned.
 * @param: attr:        Attributes that the returned memory must conform to.
 * @param: zone:        The greatest memory that the function is allowed to allocate from.
 * @return: *:          Address to a block of dynamic memory of at least 'n_bytes' bytes.
 * @return: PAGE_ERROR: Failed to allocate a continuous memory block of sufficient size.
 *                      In addition to this, swapping memory failed, too. */
FUNDEF SAFE KPD ppage_t KCALL page_malloc(size_t n_bytes, pgattr_t attr, mzone_t zone);

/* Similar to 'page_malloc', but only allocate dynamic
 * memory at the given address 'start', returning 'start' upon success,
 * or 'PAGE_ERROR' if the range is already in use, or is not available.
 * WARNING: The caller is responsible never to pass '0' for 'n_bytes'.
 * NOTE: Unaligned allocation requests are ceil-aligned.
 * NOTE: Passing ZERO(0) for 'n_bytes' will return 'start' if the
 *       associated page is available for dynamic allocation, but
 *       will return 'PAGE_ERROR' if the page is already allocated,
 *       or not a dynamic memory page at all.
 * NOTE: Unlike 'page_malloc', this function won't actually attempt to swap memory.
 * @return: start:      Successfully allocated the given address range.
 * @return: PAGE_ERROR: At least one page in the address range
 *                      start...+=n_bytes is already in use. */
#define page_malloc_at(start,n_bytes,attr)  page_malloc_in(start,start,n_bytes,attr)

/* Similar to 'page_malloc_at()', but dynamically allocate an
 * available free-range between 'min...(max+n_bytes)'
 * (that is: The greatest valid address to return is 'max')
 * NOTE: When 'max < min', the function always fails.
 * NOTE: When 'min == max', the call is equal to 'page_malloc_at(min,n_bytes,attr)'
 * @param: min:         The lowest address in which to search for an available free range.
 * @param: max:         The greatest address in which to search for an available free range.
 * @return: * :        [>= min && <= max] The base address of 'n_bytes'
 *                      ceil-aligned by pages physical bytes of memory.
 * @return: PAGE_ERROR: No available physical memory region of at least 'n_bytes' available. */
FUNDEF SAFE KPD ppage_t KCALL page_malloc_in(ppage_t min, ppage_t max,
                                             size_t n_bytes, pgattr_t attr);

/* Allocate a page at least 'min_size' bytes large, and at most 'max_size' bytes.
 * Upon success, the real allocated size is stored in '*res_size'.
 * NOTE: 'min_size' is ceil-aligned to PAGESIZE, and 'max_size' is floor-aligned to PAGESIZE.
 * NOTE: When the aligned 'max_size' <= the aligned 'min_size',
 *       'min_size' is used for 'max_size' instead.
 * NOTE: This function is implemented by searching for the smallest
 *       free memory region still greater than 'min_size' bytes and
 *       extracting as most 'max_size' bytes that are then returned.
 *       -> So even though this function is allowed to, it will not
 *          blindly allocate 'min_size' bytes at all times.
 * NOTE: Passing ZERO(0) for 'min_size' is rounded up to 'PAGESIZE'
 * NOTE: Passing ZERO(0) for 'max_size' will allocate an empty page
 *      (that is a page that may be aliasing another pointer and
 *       doesn't have to be freed, or can be freed through 'page_free(p,0)')
 *       Though that page is still guarantied to be page-aligned and not be equal to 'PAGE_ERROR'.
 *       NOTE: If no dynamic memory what-so-ever is available for use, 'PAGE_ERROR' may still be returned.
 * @param: min_size:    The lowest size (in bytes) that the returned region may be of.
 * @param: max_size:    The greatest size (in bytes) that the returned region may be of.
 * @param: res_size:    Upon success, this pointer will be filled with the actual region size.
 * @param: attr:        Memory attributes required from the region to-be allocated.
 * @param: zone:        The memory zone to allocated from.
 * @return: * :         Base address of the allocated memory block.
 * @return: PAGE_ERROR: Failed to allocate a memory region of at least 'min_size' bytes. */
FUNDEF SAFE KPD ppage_t KCALL page_malloc_part(size_t min_size, size_t max_size,
                                               size_t *__restrict res_size,
                                               pgattr_t attr, mzone_t zone);


struct mscatter {
 /* Memory scatter controller. */
 PHYS struct mscatter *m_next;  /*< [0..1][owned] The next scatter link.
                                 *   NOTE: The sum of 'm_size' from all links is always
                                 *         >= the 'n_bytes' passed to 'page_malloc_scatter' */
 ppage_t               m_start; /*< [1..1][owned(page_free)] Starting address to 'm_size' available bytes. */
 PAGE_ALIGNED size_t   m_size;  /*< Amount of bytes allocated in 'm_start' (page-aligned)
                                 *  NOTE: This value is always >= the 'min_scatter' passed to 'page_malloc_scatter' */
};

/* Take away one from the given memory scatter.
 * NOTE: Upon success, the caller takes ownership of
 *      'PAGESIZE' bytes, starting at the returned pointer.
 * @return: * :         Base address of the scattered memory page.
 * @return: PAGE_ERROR: The given memory scatter is empty. */
LOCAL SAFE KPD ppage_t KCALL mscatter_takeone(struct mscatter *__restrict self);

/* Copy all data from scatter allocation 'src' to 'dst'.
 * NOTE: Both scatter chains must have the same length. */
FUNDEF KPD void KCALL mscatter_memcpy(struct mscatter const *__restrict dst,
                                      struct mscatter const *__restrict src);

/* Split the given memory scatter 'src' at 'offset_from_src', storing
 * the higher half in 'dst' and updating 'src' to contain the lower half.
 * @return: true:  Successfully split the given scatter.
 * @return: false: Not enough available memory to (re-)allocate control structures. */
FUNDEF SAFE KPD bool KCALL mscatter_split_lo(struct mscatter *__restrict dst,
                                             struct mscatter *__restrict src,
                                             uintptr_t offset_from_src);
/* Append the given memory scatter 'src' at the end of 'dst'.
 * @return: true:  Successfully appended the scatter chains.
 * @return: false: Not enough available memory to (re-)allocate control structures. */
LOCAL SAFE KPD bool KCALL mscatter_cat(struct mscatter *__restrict dst,
                                       struct mscatter const *__restrict src);

/* Dynamically allocate physical memory scattered across the address space.
 * During this process, page are allocated in such a way that is meant to
 * counteract address space fragmentation, by preferring to allocate memory
 * from smaller free page clusters, thus hopefully reducing their numbers.
 * NOTE: 'n_bytes' and 'min_scatter' are ceil-aligned to PAGESIZE.
 * @param: scatter:     A pointer to a 'mscatter' data-structure to-be filled
 *                      with information about the allocated memory upon success.
 * @param: min_scatter: The smallest number of bytes that may be apart of any scatter entry.
 *             WARNING: When this value '>= n_bytes', 'n_bytes' is used instead!
 * @param: n_bytes:     The total number of bytes that should be allocated.
 *                      When this value is ZERO(0), 'scatter->m_start' is
 *                      in an undefined state, 'm_next' is set to NULL,
 *                      'm_size' is set to ZERO(0), and 'true' is returned.
 * @return: true:       Successfully allocated the region (s.a.: 'page_free_scatter{_list}')
 * @return: false:      Failed to allocate the scatter region (*scatter is in an undefined state) */
FUNDEF SAFE KPD bool KCALL page_malloc_scatter(struct mscatter *__restrict scatter,
                                               size_t n_bytes, size_t min_scatter,
                                               pgattr_t attr, mzone_t zone);

/* Allocate all existing free ranges within 'begin...+=n_bytes', storing them
 * as a potentially disconnected chain of scatter entries within 'scatter'.
 * >> Unlike other page_malloc()-functions, this one does not require that
 *    the specified amount of bytes are all allocated, but rather is used
 *    to allocate all dynamic memory with a given address range.
 * @return: * :      The total amount of bytes allocated, and now tracked by 'scatter'
 * @return: -ENOMEM: Failed to allocate a sufficient number of mscatter entries.
 *                   All memory regions allocated before this error occurred are
 *                   released again, meaning that 'scatter' is in an invalid state. */
FUNDEF SAFE KPD ssize_t KCALL
page_malloc_all(struct mscatter *__restrict scatter, ppage_t begin,
                size_t n_bytes, pgattr_t attr);


/* Free a given scatter list and release associated memory. */
LOCAL SAFE KPD void KCALL page_free_scatter(struct mscatter *__restrict scatter, pgattr_t attr);

/* Free the link chain used by scatter lists, but not the contained pages them self.
 * >> Useful when trying to free the scatter list after having used up its contents. */
LOCAL SAFE KPD void KCALL page_free_scatter_list(struct mscatter *__restrict scatter);


FUNDEF SAFE KPD void KCALL page_free_(ppage_t start, size_t n_bytes, pgattr_t attr);

#ifdef __INTELLISENSE__
/* Free a given memory range previous allocated with 'page_(m|c)alloc{at}'.
 * NOTE: No-op when '0' is passed for 'n_bytes'.
 * NOTE: Unaligned allocation requests are ceil-aligned. */
FUNDEF SAFE KPD void KCALL page_free(ppage_t start, size_t n_bytes);

/* Same as 'page_free', but may only be used for
 * freeing memory that is in a zero-initialized state.
 * NOTE: The caller is responsible to ensure that 'start...+=n_bytes' is zero-initialized.
 *       If this cannot be guarantied, 'page_free' should be called instead. */
FUNDEF SAFE KPD void KCALL page_cfree(ppage_t start, size_t n_bytes);
#else
#define page_free(start,n_bytes)  page_free_(start,n_bytes,PAGEATTR_NONE)
#define page_cfree(start,n_bytes) page_free_(start,n_bytes,PAGEATTR_ZERO)
#endif


/* Reallocate a memory block a 'start', spanning
 * 'old_bytes' bytes to fit 'new_bytes' afterwards.
 * - In the event that 'new_bytes <= old_bytes', 'start' is always
 *   returned, and the overflow is freed using 'page_free'.
 * - Otherwise, attempt to allocate overflowing in-place after 'start+old_bytes'
 *   using 'page_malloc_at', as well as at 'start-(new_bytes-old_bytes)'.
 *   If both fail, try to allocate a completely new region and copy all data
 *   inside.
 * @return: PAGE_ERROR: Failed to allocate a new region of sufficient size.
 *                     [page_realloc_inplace] ZERO(0) was passed for 'old_bytes'
 * @return: start:      Re-allocation could be performed in-place.
 * @return: *:          A pointer to a new memory block spanning 'new_bytes' bytes.
 * HINT: Passing 'PAGEATTR_ZERO' in 'attr' will zero-initialize newly
 *       allocated memory in the even that 'new_bytes > old_bytes'.
 * HINT: When 'old_bytes' is ZERO(0), 'start' is ignored and the
 *       function behaves identical to 'page_(m|c)alloc'.
 * NOTE: Unaligned allocation requests are ceil-aligned. */
FUNDEF SAFE KPD ppage_t KCALL page_realloc(ppage_t start, size_t old_bytes,
                                           size_t new_bytes, pgattr_t attr,
                                           mzone_t zone);
FUNDEF SAFE KPD ppage_t KCALL page_realloc_inplace(ppage_t start, size_t old_bytes,
                                                   size_t new_bytes, pgattr_t attr);


struct mzstat {
 PAGE_ALIGNED size_t mz_avail;   /* Total amount of available, non-continuous bytes */
 PAGE_ALIGNED size_t mz_inuse;   /* Total amount of non-continuous bytes currently in use */
 PAGE_ALIGNED size_t mz_freemin; /*< Size of the smallest free page block (in bytes). */
 PAGE_ALIGNED size_t mz_freemax; /*< Size of the largest free page block (in bytes). */
 PAGE_ALIGNED size_t mz_zeromin; /*< Size of the smallest zero-initialized page block (in bytes). */
 PAGE_ALIGNED size_t mz_zeromax; /*< Size of the largest zero-initialized page block (in bytes). */
              size_t mz_freeblk; /*< Amount of non-continuous free page blocks. */
              size_t mz_zeroblk; /*< [<= ps_freeblk] Amount of non-continuous, zero-initialized page blocks. */
};

struct mstat {
 struct mzstat m_total;
 struct mzstat m_zones[MZONE_COUNT];
};

/* Query various page statistics and store the information in '*info'. */
FUNDEF KPD void KCALL page_stat(struct mstat *__restrict info);

/* Print human-readable debug information about free memory from the given zone. */
FUNDEF SAFE KPD ssize_t KCALL page_print(mzone_t zone, pformatprinter printer, void *closure);


#ifdef CONFIG_BUILDING_KERNEL_CORE
struct mb_mmap_entry;
struct mb2_tag_mmap;
struct mb2_tag_basic_meminfo;

/* Initialize the kernel memory manager from multiboot bootloader information.
 * WARNING: This function is an init-call and may not
 *          be called once init memory has been freed. */
INTDEF SAFE KPD size_t KCALL memory_load_mb_mmap(struct mb_mmap_entry *__restrict info, u32 info_len);
INTDEF SAFE KPD size_t KCALL memory_load_mb_lower_upper(u32 mem_lower, u32 mem_upper);
INTDEF SAFE KPD size_t KCALL memory_load_mb2_mmap(struct mb2_tag_mmap *__restrict info);

/* Try various different ways of detecting memory.
 * WARNING: This function is an init-call and may not
 *          be called once init memory has been freed. */
INTDEF SAFE KPD void KCALL memory_load_detect(void);

/* Register physical memory for use by the physical memory allocator
 * NOTE: Physical memory used by the kernel image itself is
 *       ignored, and memory marked using 'memory_notouch()'
 *       will be updated to only become available once
 *      'memory_done_install()' has been called.
 * WARNING: This function is an init-call and may not
 *          be called once init memory has been freed.
 * @return: * : The actual amount of usable bytes now installed. */
INTDEF SAFE KPD size_t KCALL memory_install(PHYS uintptr_t start, size_t size);

/* Helper functions for defining special regions of memory that
 * must not be touched when installing available memory.
 * >> Veryuseful when the memory information source itself
 *    is located in the dynamic memory it is describing.
 *    In such an event, this function may be used to prevent
 *    any data within 'start...+=size' from be corrupted
 *    before having been copied into a safe location.
 * >> To commit all free-memory mappings overlapping with
 *    the given range, 'memory_done_install()' must be called
 *    at a later time to install all previously protected
 *    memory, thereby allowing the system to start using it.
 *    NOTE: Only memory parts that were attempted to be installed
 *          through calls to 'memory_install()' will be registered
 *          following a call to 'memory_notouch()'. */
INTDEF SAFE KPD bool KCALL memory_notouch(PHYS uintptr_t start, size_t size);
INTDEF SAFE KPD size_t KCALL memory_done_install(void);
#define MEMORY_NOTOUCH_MAXCOUNT   8
#define MEMORY_FREELATER_MAXCOUNT 8

/* Mirror all touched free-later pages within memory information.
 * NOTE: This function must be called after all dynamic memory
 *       has been detected, but before paging is initialized. */
INTDEF SAFE KPD void KCALL memory_mirror_freelater_info(void);

/* If possible, relocate any core information allocated within the 'MZONE_V_1MB' zone. */
INTDEF SAFE KPD void KCALL memory_relocate_info(void);

#endif
#endif /* __CC__ */

DECL_END

#ifdef CONFIG_BUILDING_KERNEL_CORE
#ifdef __CC__
#include <hybrid/list/atree.h>
#include <hybrid/sync/atomic-rwlock.h>

DECL_BEGIN

struct mzone {
 atomic_rwlock_t z_lock;  /*< Lock for this zone. */
 ppage_t         z_root;  /*< [lock(z_lock)][0..1|null(PAGE_ERROR)]
                           *   Root page of this zone (Must always be 'PAGE_ERROR', or apart of it). */
 size_t          z_inuse; /*< [lock(z_lock)] Amount of non-continuous bytes from this zone, currently in use. */
 size_t          z_avail; /*< [lock(z_lock)] Amount of non-continuous, free bytes. */
 ref_t           z_alloc; /*< [lock(z_lock)] The amount of successful allocations encompassing at least 1 page.
                           *   WARNING: As this number is never decrementing, any code
                           *            using it should be prepared for it overflowing. */
 ref_t           z_free;  /*< [lock(z_lock)] The amount of free-calls encompassing at least 1 page.
                           *   WARNING: Same restrictions apply as for 'z_alloc'. */
};

/* Master controller for dynamic allocation of physical memory. */
INTDEF struct mzone  page_zones[MZONE_COUNT];
#define PAGEZONE(x) (page_zones+(x))

#define PAGEZONES_FOREACH(p) \
 for ((p) = page_zones; \
      (p) != COMPILER_ENDOF(page_zones); \
    ++(p))

#define ASSERT_FREEPAGE(x) \
 (assert((x)->p_free.p_size),\
  assert(IS_ALIGNED((uintptr_t)(x),PAGESIZE)),\
  __assertf(IS_ALIGNED((x)->p_free.p_size,PAGESIZE),"x = %p\nsize = %Ix\n",(x),(x)->p_free.p_size),\
  assert(*(x)->p_free.p_self == (x)),\
  assertf((x)->p_free.p_next == PAGE_ERROR || \
          (x)->p_free.p_next->p_free.p_self == &(x)->p_free.p_next,\
          "Invalid link between %p...%p and %p...%p", \
          (x),(uintptr_t)PAGE_END(x)-1, \
          (x)->p_free.p_next,(uintptr_t)PAGE_END((x)->p_free.p_next)-1))
#define PAGE_FOREACH(x,zone) \
 for ((x) = (zone)->z_root; (x) != PAGE_ERROR; (x) = (x)->p_free.p_next) \
 if ((ASSERT_FREEPAGE(x),0)); else



DECL_END
#endif /* __CC__ */
#endif /* CONFIG_BUILDING_KERNEL_CORE */


#ifdef __CC__
#ifndef __INTELLISENSE__
#include <assert.h>
#include <hybrid/check.h>
#include <hybrid/align.h>
#include <kernel/malloc.h>

DECL_BEGIN

LOCAL SAFE KPD void KCALL
page_free_scatter(struct mscatter *__restrict self, pgattr_t attr) {
 struct mscatter entry = *self;
 CHECK_HOST_DOBJ(self);
 for (;;) {
  page_free_(entry.m_start,entry.m_size,attr);
  if ((self = entry.m_next) == NULL) break;
  entry = *self;
  kffree(self,GFP_NOFREQ);
 }
}
LOCAL SAFE KPD void KCALL
page_free_scatter_list(struct mscatter *__restrict self) {
 struct mscatter entry = *self;
 CHECK_HOST_DOBJ(self);
 for (;;) {
  if ((self = entry.m_next) == NULL) break;
  entry = *self;
  kffree(self,GFP_NOFREQ);
 }
}
LOCAL SAFE KPD ppage_t KCALL
mscatter_takeone(struct mscatter *__restrict self) {
 ppage_t result;
 CHECK_HOST_DOBJ(self);
 if unlikely(!self->m_size) return PAGE_ERROR;
 __assertf(IS_ALIGNED(self->m_size,PAGESIZE),
           "self->m_size = %p",self->m_size);
 result = self->m_start++;
 self->m_size -= PAGESIZE;
 if (!self->m_size) {
  /* Load the next scatter entry. */
  struct mscatter *free_next = self->m_next;
  if (free_next) {
   *self = *free_next;
   kffree(free_next,GFP_NOFREQ);
  }
 }
 return result;
}
LOCAL SAFE KPD bool KCALL
mscatter_cat(struct mscatter *__restrict dst,
             struct mscatter const *__restrict src) {
 struct mscatter *src_copy;
 struct mscatter *next,**pnext = &dst->m_next;
 while ((next = *pnext) != NULL) pnext = &next->m_next;
 next = __COMPILER_CONTAINER_OF(pnext,struct mscatter,m_next);
 if ((uintptr_t)next->m_start+next->m_size == (uintptr_t)src->m_start) {
  /* Inplace-extend is possible! */
  next->m_next  = src->m_next;
  next->m_size += src->m_size;
 } else {
  src_copy = (struct mscatter *)kmalloc(sizeof(struct mscatter),
                                        GFP_SHARED|GFP_NOFREQ);
  if unlikely(!src_copy) return false;
  *src_copy = *src;
  *pnext = src_copy;
 }
 return true;
}


DECL_END
#endif /* __INTELLISENSE__ */
#endif /* __CC__ */

#endif /* !GUARD_INCLUDE_KERNEL_MEMORY_H */
