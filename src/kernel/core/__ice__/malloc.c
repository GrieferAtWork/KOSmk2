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
#ifndef GUARD_SRC_KERNEL_CORE_MALLOC_C
#define GUARD_SRC_KERNEL_CORE_MALLOC_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <string.h>
#include <hybrid/compiler.h>
#include <hybrid/debuginfo.h>
#include <hybrid/check.h>
#include <kernel/paging.h>
#include <kernel/malloc.h>
#include <kernel/memory.h>
#include <kernel/mman.h>
#include <hybrid/list/list.h>
#include <hybrid/list/atree.h>
#include <hybrid/align.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/syslog.h>
#include <hybrid/minmax.h>
#include <sched/task.h>
#include <sched/paging.h>
#include <sys/mman.h>
#include <format-printer.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define MALIGNED /* Annotation for an integral/pointer aligned by 'HEAP_ALIGNMENT' */

DECL_BEGIN

#if 0 /* Disable all debug functionality. */
#define CONFIG_MALLOC_NO_DEBUG_INIT
#undef CONFIG_TRACE_LEAKS
#define CONFIG_MALLOC_HEADSIZE 0
#define CONFIG_MALLOC_FOOTSIZE 0
#define CONFIG_MALLOC_TRACEBACK 0
#endif


//#define CONFIG_TRACE_LEAKS
//#define CONFIG_MALLOC_HEADSIZE  8
//#define CONFIG_MALLOC_FOOTSIZE  8
//TODO: #define CONFIG_MALLOC_CHKSUM 1
//#define CONFIG_MALLOC_TRACEBACK 1
//#define CONFIG_MALLOC_TRACEBACK_MINSIZE 8
//#define CONFIG_MALLOC_NO_DEBUG_INIT 0
//#define CONFIG_MALLOC_DEBUG_INIT 0xcc


#ifdef CONFIG_MALLOC_NO_DEBUG_INIT
#if (CONFIG_MALLOC_NO_DEBUG_INIT+0) == 0
#   undef CONFIG_MALLOC_NO_DEBUG_INIT
#endif
#endif
#ifndef CONFIG_MALLOC_DEBUG_INIT
#if !defined(CONFIG_MALLOC_NO_DEBUG_INIT) && \
     defined(CONFIG_DEBUG)
#   define CONFIG_MALLOC_DEBUG_INIT 0xcc
#endif
#elif defined(CONFIG_MALLOC_NO_DEBUG_INIT)
#   undef  CONFIG_MALLOC_DEBUG_INIT
#endif

/* The min amount of traceback entries to track for any allocation. */
#ifdef CONFIG_TRACE_LEAKS
#ifndef CONFIG_MALLOC_TRACEBACK
#   define CONFIG_MALLOC_TRACEBACK 1
#elif (CONFIG_MALLOC_TRACEBACK+0) == 0
#   undef CONFIG_MALLOC_TRACEBACK
#endif
#elif defined(CONFIG_MALLOC_TRACEBACK)
#   undef CONFIG_MALLOC_TRACEBACK
#endif

#ifdef CONFIG_MALLOC_TRACEBACK
#ifndef CONFIG_MALLOC_TRACEBACK_MINSIZE
#   define CONFIG_MALLOC_TRACEBACK_MINSIZE 4
#endif
#else
#   undef CONFIG_MALLOC_TRACEBACK_MINSIZE
#endif

#ifndef CONFIG_MALLOC_HEADSIZE
#ifdef CONFIG_DEBUG
#   define CONFIG_MALLOC_HEADSIZE HEAP_ALIGNMENT
#endif
#elif (CONFIG_MALLOC_HEADSIZE+0) == 0
#   undef  CONFIG_MALLOC_HEADSIZE
#endif
#ifndef CONFIG_MALLOC_FOOTSIZE
#ifdef CONFIG_DEBUG
#   define CONFIG_MALLOC_FOOTSIZE HEAP_ALIGNMENT
#endif
#elif (CONFIG_MALLOC_FOOTSIZE+0) == 0
#   undef  CONFIG_MALLOC_FOOTSIZE
#endif

/* Enable the debug-API when any functionality that
 * requirest additional tracking to-be done was activated. */
#if defined(CONFIG_TRACE_LEAKS) || \
    defined(CONFIG_MALLOC_HEADSIZE) || \
    defined(CONFIG_MALLOC_FOOTSIZE)
#define MALLOC_DEBUG_API
#endif


/* Memory zone used for physical allocations. */
#define MEMORY_PHYS_ZONE          MZONE_NOSHARE

/* Address ranges used by virtual kernel/shared memory allocations. */
#define KERNEL_VIRT_BEGIN         0x00000000  /* 0x00000000...0xbfffffff */
#define KERNEL_VIRT_END           KERNEL_BASE /* 0x00000000...0xbfffffff */
#define SHARED_VIRT_BEGIN         KERNEL_BASE /* 0xc0000000...0xffffffff */
#define SHARED_VIRT_END           0x00000000  /* 0xc0000000...0xffffffff */

#define BAD_ALLOC(n_bytes,flags) (void)0
#define M_ISNONNULL(p) likely((p) != NULL)

#define __GFP_NULL                      GFP_NORMAL /* GFP flags for pointers not matching 'M_ISNONNULL' */
#if 1 /* TODO: Use shared memory for malloc()! */
#define __GFP_MALLOC                    GFP_MEMORY /* GFP flags for malloc()-allocated memory. */
#else
#define __GFP_MALLOC                    GFP_SHARED /* GFP flags for malloc()-allocated memory. */
#endif


/* Define to non-zero to add syslog entries for managed memory allocation. */
#ifndef LOG_MANAGED_ALLOCATIONS
#define LOG_MANAGED_ALLOCATIONS 0
#endif



#ifdef MALLOC_DEBUG_API
struct dinfo {
 char const      *i_file;
 int              i_line;
 char const      *i_func;
 struct instance *i_inst;
};
#endif /* MALLOC_DEBUG_API */


#if defined(CONFIG_MALLOC_FOOTSIZE) || \
    defined(CONFIG_MALLOC_TRACEBACK)
#define MPTR_HAVE_TAIL
struct PACKED mptr_tail {
#ifdef CONFIG_MALLOC_FOOTSIZE
 byte_t       t_foot[CONFIG_MALLOC_FOOTSIZE];
#endif
#ifdef CONFIG_MALLOC_TRACEBACK
#define MPTR_TAIL_TB_EOF ((void *)-1)
 void        *t_tb[1]; /*< [0..1|null(MPTR_TAIL_TB_EOF)][EOF(MPTR_TAIL_TB_EOF)]
                        *  [0..MPTR_TRACEBACK_SIZE(^self)][const] Malloc traceback instruction pointers.
                        *   NOTE: May prematurely terminate upon hitting 'MPTR_TAIL_TB_EOF'. */
#endif
};
#endif

struct mptr {
 /* Trace header */
#ifdef CONFIG_TRACE_LEAKS
 LIST_NODE(struct mptr)
              m_chain;  /*< [lock(MALLTRACE_LOCK(self))] Chain of all other MALL headers. */
 u16          m_chksum; /*< [const] Checksum of mh_flag...:m_info + mt_tb. */
 u8           m_refcnt; /*< [lock(MPTR_TRACE_LOCK(self))] Reference counter (required for handling free() while enumerating). */
#define MPTRFLAG_NONE      0x00
#define MPTRFLAG_UNTRACKED 0x01 /*< The pointer was untracked. */
#define MPTRFLAG_NOFREE    0x02 /*< The pointer must not be freed or reallocated. */
 u8           m_flag;   /*< [lock(MALLTRACE_LOCK(self))] Mall flags (Set of 'MPTRFLAG_*'). */
 struct dinfo m_info;   /*< [const] Basic debug information for tracking. */
#define __1_MPTR_SIZEOF (5*__SIZEOF_POINTER__+4+__SIZEOF_INT__)
#else /* CONFIG_TRACE_LEAKS */
#define __1_MPTR_SIZEOF  0
#endif /* !CONFIG_TRACE_LEAKS */

 /* Tail pointer header */
#ifdef MPTR_HAVE_TAIL
 struct mptr_tail *m_tail; /*< [const][1..1][>= self] Pointer to the end of user-data. */
#define __2_MPTR_SIZEOF (__1_MPTR_SIZEOF+__SIZEOF_POINTER__)
#else
#define __2_MPTR_SIZEOF __1_MPTR_SIZEOF
#endif

 /* Underflow safe-area header. */
#ifdef CONFIG_MALLOC_HEADSIZE
#define __3_MPTR_SIZEOF (CONFIG_MALLOC_HEADSIZE+__2_MPTR_SIZEOF)
#else
#define __3_MPTR_SIZEOF  __2_MPTR_SIZEOF
#endif
#define MPTR_UNALIGNED_SIZE (__3_MPTR_SIZEOF+__SIZEOF_SIZE_T__)

#if (MPTR_UNALIGNED_SIZE % HEAP_ALIGNMENT) != 0
#define MPTR_HAVE_PAD
 byte_t   m_pad[MPTR_UNALIGNED_SIZE % HEAP_ALIGNMENT];
#endif
union{ size_t m_size; /*< [const][mask(~GFP_MASK_TYPE)][>= HEAP_MIN_MALLOC] Total memory size (including this header). */
       size_t m_type; /*< [const][mask(GFP_MASK_TYPE)][!= 3] Flags used to create the pointer. */
       size_t m_data; /*< [const] Malloc-pointer information. */
};
#ifdef CONFIG_MALLOC_HEADSIZE
 byte_t       m_head[CONFIG_MALLOC_HEADSIZE];
#endif
};


#define MPTR_TRACE_LOCK(self) ((self)->m_info.i_inst->i_kernel.k_tlock)

#ifdef MPTR_HAVE_TAIL
#ifdef CONFIG_MALLOC_TRACEBACK
#   define MPTR_TRACEBACK_ADDR(self) ((self)->m_tail->t_tb)
#   define MPTR_TRACEBACK_SIZE(self) ((MPTR_TAILSIZE(self)-offsetof(struct mptr_tail,t_tb))/sizeof(void *))
#endif
#   define MPTR_TAILADDR(self) ((self)->m_tail)
#   define MPTR_TAILSIZE(self) (((uintptr_t)(self)+MPTR_SIZE(self))-(uintptr_t)MPTR_TAILADDR(self))
#   define MPTR_USERADDR(self) ((struct mptr *)(self)+1)
#   define MPTR_USERSIZE(self) ((uintptr_t)MPTR_TAILADDR(self)-(uintptr_t)MPTR_USERADDR(self))
#else
#   define MPTR_USERADDR(self) ((struct mptr *)(self)+1)
#   define MPTR_USERSIZE(self) (MPTR_SIZE(self)-sizeof(struct mptr))
#endif

#if defined(CONFIG_MALLOC_FOOTSIZE) && CONFIG_MALLOC_TRACEBACK
#   define MPTR_SIZEOF(s)            (sizeof(struct mptr)+(s)+CONFIG_MALLOC_FOOTSIZE+CONFIG_MALLOC_TRACEBACK*sizeof(void *))
#elif defined(CONFIG_MALLOC_FOOTSIZE)
#   define MPTR_SIZEOF(s)            (sizeof(struct mptr)+(s)+CONFIG_MALLOC_FOOTSIZE)
#elif CONFIG_MALLOC_TRACEBACK
#   define MPTR_SIZEOF(s)            (sizeof(struct mptr)+(s)+CONFIG_MALLOC_TRACEBACK*sizeof(void *))
#else
#   define MPTR_SIZEOF(s)            (sizeof(struct mptr)+(s))
#endif

#define MPTR_OF(p)         ((struct mptr *)(p)-1)

#define MPTR_TYPE(p)           ((p)->m_type&GFP_MASK_TYPE)
#define MPTR_SIZE(p)           ((p)->m_size&~GFP_MASK_TYPE)
#define MPTR_HEAP(p)           (&mheap_kernel[MPTR_TYPE(p)])
#define MPTR_MKDATA(type,size) ((type)|(size))

#define MPTR_ISOK(p)     (MPTR_TYPE(p) != 3 && MPTR_SIZE(p) >= HEAP_MIN_MALLOC)
#define MPTR_VALIDATE(p)  assertf(MPTR_ISOK(p),"Invalid m-pointer %p",p)

#ifdef MALLOC_DEBUG_API
struct dsetup {
 struct dinfo s_info;  /*< Basic debug information for tracking. */
 void        *s_tbebp; /*< EBP used in tracebacks. */
};
PRIVATE struct mptr *KCALL mptr_safeload(struct dsetup *__restrict setup, void *__restrict p);
#define MPTR_GET(p)  mptr_safeload(setup,p)

/* Setup debug informations within a given mptr. */
PRIVATE void KCALL mptr_setup(struct mptr *__restrict self,
                              struct dsetup *__restrict setup,
                              size_t user_size);
#ifdef MPTR_HAVE_TAIL
PRIVATE void KCALL mptr_mvtail(struct mptr *__restrict self, size_t old_size, size_t new_size, gfp_t flags);
#else
#define mptr_mvtail(self,old_size,new_size) (void)0
#endif
#ifdef CONFIG_TRACE_LEAKS
PRIVATE u16 KCALL mptr_chksum(struct mptr *__restrict self);
PRIVATE void KCALL mptr_unlink(struct mptr *__restrict self);
PRIVATE void KCALL mptr_relink(struct mptr *__restrict self);
#else
#define mptr_unlink(self) (void)0
#define mptr_relink(self) (void)0
#endif
#else
#define MPTR_GET(p) (MPTR_VALIDATE(MPTR_OF(p)),MPTR_OF(p))
#endif


#ifdef CONFIG_MALLOC_HEADSIZE
PRIVATE byte_t mall_header_seed[4] = {0x65,0xB6,0xBD,0x5A};
#define MALL_HEADERBYTE(i) (mall_header_seed[(i) % 4]^(byte_t)((0xff >> (i) % 8)*(i))) /* Returns the i-th control byte for mall-headers. */
#endif

#ifdef CONFIG_MALLOC_FOOTSIZE
PRIVATE byte_t mall_footer_seed[4] = {0xCF,0x6A,0xB7,0x97};
#define MALL_FOOTERBYTE(i) (mall_footer_seed[(i) % 4]^(byte_t)((0xff >> (i) % 7)*((i)+1))) /* Returns the i-th control byte for mall-footers. */
#endif


STATIC_ASSERT(IS_ALIGNED(sizeof(struct mptr),HEAP_ALIGNMENT));
STATIC_ASSERT(GFP_GTPAGEATTR(GFP_CALLOC) == PAGEATTR_ZERO);










/* unsigned int FFS(size_t x); */
/* unsigned int CLZ(size_t x); */
#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#   define FFS(x)  ((unsigned int)__builtin_ffs((int)(x)))
#   define CLZ(x)  ((unsigned int)__builtin_clz((int)(x)))
#   define CTZ(x)  ((unsigned int)__builtin_ctz((int)(x)))
#elif __SIZEOF_SIZE_T__ == __SIZEOF_LONG__
#   define FFS(x)  ((unsigned int)__builtin_ffsl((long)(x)))
#   define CLZ(x)  ((unsigned int)__builtin_clzl((long)(x)))
#   define CTZ(x)  ((unsigned int)__builtin_ctzl((long)(x)))
#else
#   define FFS(x)  ((unsigned int)__builtin_ffsll((long long)(x)))
#   define CLZ(x)  ((unsigned int)__builtin_clzll((long long)(x)))
#   define CTZ(x)  ((unsigned int)__builtin_ctzll((long long)(x)))
#endif

/* Heap configuration:
 * Index offset for the first bucket that should be search for a given size. */
#if HEAP_ALIGNMENT == 8
#   define HEAP_BUCKET_OFFSET     4 /* FFS(HEAP_ALIGNMENT) */
#elif HEAP_ALIGNMENT == 16
#   define HEAP_BUCKET_OFFSET     5 /* FFS(HEAP_ALIGNMENT) */
#else
#   define HEAP_BUCKET_OFFSET     FFS(HEAP_ALIGNMENT)
#endif

#define HEAP_BUCKET_OF(size)   (((__SIZEOF_SIZE_T__*8)-CLZ(size))-HEAP_BUCKET_OFFSET)
#define HEAP_BUCKET_COUNT       ((__SIZEOF_SIZE_T__*8)-HEAP_BUCKET_OFFSET)

/* The min amount of bytes that can be allocated at once.
 * NOTE: Technically, this is the min amount that can be freed at once, but eh... */
#define HEAP_MIN_MALLOC  CEIL_ALIGN(sizeof(struct mfree),HEAP_ALIGNMENT)

#define MFREE_ATTRMASK   0x1
#define MFREE_SIZEMASK ~(MFREE_ATTRMASK)

/* Descriptor for a free portion of memory. */
struct mfree {
#ifdef __INTELLISENSE__
 struct { struct mfree *le_next,**le_pself; }
                           mf_lsize; /*< [lock(:mh_lock)][sort(ASCENDING(mf_size))] List of free entries ordered by size. */
#else
 LIST_NODE(struct mfree)   mf_lsize; /*< [lock(:mh_lock)][sort(ASCENDING(mf_size))] List of free entries ordered by size. */
#endif
 ATREE_XNODE(struct mfree) mf_laddr; /*< [lock(:mh_lock)][sort(ASCENDING(self))] List of free entries ordered by address. */
union{
 size_t                    mf_info;
 size_t                    mf_size;  /*< [lock(:mh_lock)][mask(MFREE_SIZEMASK)][>= HEAP_MIN_MALLOC]
                                      *   Size of this free range in bytes (Including this header). */
 pgattr_t                  mf_attr;  /*< [lock(:mh_lock)][mask(MFREE_ATTRMASK)]
                                      *   Page-attributes for data within this free region.
                                      *   NOTE: When 'CONFIG_MALLOC_DEBUG_INIT' is defined,
                                      *         and the 'PAGEATTR_ZERO' flag isn't set, the
                                      *         memory described within this free range is
                                      *         fully initialized to 'CONFIG_MALLOC_DEBUG_INIT'
                                      *        (Excluding this header of course)
                                      *   >> This behavior mirrors that of 'PAGEATTR_ZERO'-initialized
                                      *      memory, in that free data is known to be in a specific state.
                                      */
};
};
#define MFREE_MIN(self)         ((uintptr_t)(self))
#define MFREE_MAX(self)         ((uintptr_t)(self)+((self)->mf_size&MFREE_SIZEMASK)-1)
#define MFREE_BEGIN(self)       ((uintptr_t)(self))
#define MFREE_END(self)         ((uintptr_t)(self)+((self)->mf_size&MFREE_SIZEMASK))
#define MFREE_SIZE(self)                   ((self)->mf_size&MFREE_SIZEMASK)
#define MFREE_ATTR(self)                   ((self)->mf_attr&MFREE_ATTRMASK)
#define MFREE_MKINFO(size,pg_attr)        (assert(IS_ALIGNED(size,HEAP_ALIGNMENT)),\
                                           assert((size) >= HEAP_MIN_MALLOC),\
                                           assert(((pg_attr)&MFREE_ATTRMASK) == (pg_attr)),\
                                          (size)|(pg_attr))


struct mheap {
 atomic_rwlock_t          mh_lock;      /*< Lock for this heap. */
 ATREE_HEAD(struct mfree) mh_addr;      /*< [lock(mh_lock)] Heap sorted by address. */
 LIST_HEAD(struct mfree)  mh_size[HEAP_BUCKET_COUNT];
                                        /*< [lock(mh_lock)][0..1|null(PAGE_ERROR)][*] Heap sorted by free range size. */
 size_t                   mh_overalloc; /*< [lock(mh_lock)] Amount (in bytes) by which to over-allocate memory in heaps.
                                         *   NOTE: Set to ZERO(0) to disable overallocation. */
 size_t                   mh_freethresh;/*< [lock(mh_lock)] Threshold that must be reached before any continuous block
                                         *   of free data is split to free memory. (Should always be '>= PAGESIZE') */
};

PRIVATE MALIGNED void *KCALL mheap_acquire(struct mheap *__restrict self, MALIGNED size_t n_bytes, MALIGNED size_t *__restrict alloc_bytes, gfp_t flags);
PRIVATE MALIGNED void *KCALL mheap_acquire_al(struct mheap *__restrict self, MALIGNED size_t alignment, MALIGNED size_t offset, MALIGNED size_t n_bytes, MALIGNED size_t *__restrict alloc_bytes, gfp_t flags);
PRIVATE MALIGNED void *KCALL mheap_acquire_at(struct mheap *__restrict self, MALIGNED void *p, MALIGNED size_t n_bytes, MALIGNED size_t *__restrict alloc_bytes, gfp_t flags);
PRIVATE bool KCALL mheap_release(struct mheap *__restrict self, MALIGNED void *p, MALIGNED size_t n_bytes, gfp_t flags);
PRIVATE void KCALL mheap_release_nomerge(struct mheap *__restrict self, MALIGNED void *p, MALIGNED size_t n_bytes, gfp_t flags);
PRIVATE void  KCALL mheap_unmapfree(struct mheap *__restrict self, struct mfree **__restrict pslot, ATREE_SEMI_T(uintptr_t) addr_semi, ATREE_LEVEL_T addr_level, gfp_t flags);

PRIVATE struct mptr *KCALL mheap_malloc(struct mheap *__restrict self, size_t size, gfp_t flags);
PRIVATE struct mptr *KCALL mheap_memalign(struct mheap *__restrict self, size_t alignment, size_t size, gfp_t flags);
PRIVATE void         KCALL mheap_free(struct mheap *__restrict self, struct mptr *ptr, gfp_t flags);
PRIVATE struct mptr *KCALL mheap_realloc(struct mheap *__restrict self, struct mptr *ptr, size_t new_size, gfp_t flags);
PRIVATE struct mptr *KCALL mheap_realign(struct mheap *__restrict self, struct mptr *ptr, size_t alignment, size_t new_size, gfp_t flags);


#define MHEAP_INIT(o,f) \
{ \
    .mh_lock = ATOMIC_RWLOCK_INIT, \
    .mh_addr = PAGE_ERROR, \
    .mh_size = { \
        [0 ... HEAP_BUCKET_COUNT-1] = PAGE_ERROR, \
    }, \
    .mh_overalloc  = (o),\
    .mh_freethresh = (f),\
}

/* The different memory heaps used by the kernel. */
#define MHEAP_COUNT 3
PRIVATE struct mheap mheap_kernel[MHEAP_COUNT] = {
    [GFP_SHARED] = MHEAP_INIT(PAGESIZE*16,PAGESIZE*16),
    [GFP_KERNEL] = MHEAP_INIT(PAGESIZE*16,PAGESIZE*16),
    [GFP_MEMORY] = MHEAP_INIT(0,PAGESIZE),
};
/* [mheap_memory]: Don't overallocate, or wait before releasing physical memory.
 *              >> Since physical memory is the most valuable resouces, as all
 *                 other types of memory make use of it to implement actual
 *                 in-core data, don't overallocate, and don't wait before
 *                 releasing allocated memory.
 *           NOTE: Doing do does not really degrade performance, as physical
 *                 memory allocation is fairly fast to begin with, as well
 *                 as being designed to behave nearly atomic. */
#undef MHEAP_INIT

DECL_END

/* Define the ABI for the address tree used by mman. */
#define ATREE(x)            mfree_tree_##x
#define ATREE_NULL          PAGE_ERROR
#define ATREE_NODE_MIN      MFREE_MIN
#define ATREE_NODE_MAX      MFREE_MAX
#define Tkey                uintptr_t
#define T                   struct mfree
#define path                mf_laddr
#include <hybrid/list/atree-abi.h>

/* Mall public interface implementation. */
DECL_BEGIN



/* Page-level physical/virtual memory allocators. */
LOCAL PAGE_ALIGNED void *KCALL core_page_alloc(size_t n_bytes, gfp_t flags);
LOCAL PAGE_ALIGNED void *KCALL core_page_allocat(PAGE_ALIGNED void *start, size_t n_bytes, gfp_t flags);
LOCAL void KCALL core_page_free(PAGE_ALIGNED void *ptr, size_t n_bytes, gfp_t flags);

/* Kernel-level virtual mmap()/munmap() for anonymous memory. */
PRIVATE KPD VIRT void *KCALL
kernel_mmap_anon(VIRT PAGE_ALIGNED void *start,
                 PAGE_ALIGNED size_t n_bytes,
                 gfp_t flags) {
 PHYS struct mregion *region;
 errno_t error;
 assert(PDIR_ISKPD());
 assert(n_bytes != 0);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED(n_bytes,PAGESIZE));
 assert((uintptr_t)start+n_bytes > (uintptr_t)start);
again:
 region = (PHYS struct mregion *)kcalloc(sizeof(struct mregion),
                                         GFP_MEMORY);
 if unlikely(!region) return PAGE_ERROR;
 region->mr_refcnt = 1;
 region->mr_size   = n_bytes;
 if (flags&GFP_CALLOC)
     region->mr_init = MREGION_INIT_ZERO;
#ifdef CONFIG_MALLOC_DEBUG_INIT
 else {
  /* Pre-initialize virtual kernel heap memory with a debug filler byte.
   * >> Doing so here allows for pre-initialization of data. */
#if CONFIG_MALLOC_DEBUG_INIT == 0
  region->mr_init           = MREGION_INIT_ZERO;
#else
  region->mr_init           = MREGION_INIT_BYTE;
  region->mr_setup.mri_byte = 0x01010101u*(CONFIG_MALLOC_DEBUG_INIT&0xff);
#endif
 }
#endif
 rwlock_cinit(&region->mr_plock);
 region->mr_parts = &region->mr_part0;
 region->mr_part0.mt_chain.le_pself = &region->mr_parts;
 if (flags&GFP_INCORE) {
  /* TODO: Preallocate core memory for the region. */
 }
 mregion_setup(region);
 error = mman_mmap_unlocked(&mman_kernel,(ppage_t)start,n_bytes,0,region,
                            PROT_READ|PROT_WRITE,NULL,NULL);
 MREGION_DECREF(region);
 if (E_ISERR(error)) {
  /* Try to swap out memory. */
  if (MMAN_SWAPOK(mman_swapmem(n_bytes,flags))) goto again;
  BAD_ALLOC(n_bytes,flags);
  return PAGE_ERROR;
 }
 return start;
}
LOCAL KPD VIRT void KCALL
kernel_munmap(VIRT PAGE_ALIGNED void *start,
              PAGE_ALIGNED size_t n_bytes,
              pgattr_t flags) {
 assert(PDIR_ISKPD());
 assert(n_bytes != 0);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED(n_bytes,PAGESIZE));
 assert((uintptr_t)start+n_bytes > (uintptr_t)start);
 (void)flags; /* TODO: flags&PAGEATTR_ZERO --> Free as cleared memory. */
 mman_munmap_unlocked(&mman_kernel,(ppage_t)start,n_bytes);
}



LOCAL CRIT PAGE_ALIGNED void *KCALL
core_page_alloc(size_t n_bytes, gfp_t flags) {
 void *result;
 if (flags&GFP_MEMORY) {
  do result = page_malloc(n_bytes,MEMORY_PHYS_ZONE,GFP_GTPAGEATTR(flags));
  while (result == PAGE_ERROR && MMAN_SWAPOK(mman_swapmem(n_bytes,flags)));
#ifdef CONFIG_MALLOC_DEBUG_INIT
  if (result != PAGE_ERROR && !(flags&GFP_CALLOC))
      memsetl(result,0x01010101u*(CONFIG_MALLOC_DEBUG_INIT&0xff),PAGESIZE/4);
#endif
 } else {
  PHYS struct mman *old_mman; bool has_write_lock = false;
  static VIRT uintptr_t heap_end[(GFP_SHARED|GFP_KERNEL)+1] = {
      [GFP_SHARED] = SHARED_VIRT_BEGIN+0x10000000,
      [GFP_KERNEL] = KERNEL_VIRT_BEGIN+0x10000000,
  };
#define HEAP_KERNEL heap_end[GFP_KERNEL]
#define HEAP_SHARED heap_end[GFP_SHARED]
  task_nointr();
  if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_BEGIN(old_mman);
  assert(PDIR_ISKPD());
  /* Only need a read-lock to search for free space. */
  if (!(flags&GFP_ATOMIC)) rwlock_read(&mman_kernel.m_lock);
  else if (rwlock_tryread(&mman_kernel.m_lock) == -EAGAIN) {
   result = PAGE_ERROR;
   goto end;
  }
check_again:
  result = mman_findspace_unlocked(&mman_kernel,
                                  (ppage_t)heap_end[flags&(GFP_SHARED|GFP_KERNEL)],
                                   n_bytes,PAGESIZE,0,false);
  if (result == PAGE_ERROR) goto end2;
  /* Upgrade to a write-lock before mapping the space we've discovered. */
  if (!has_write_lock) {
   has_write_lock = true;
   if (!(flags&GFP_ATOMIC)) {
    if (rwlock_upgrade(&mman_kernel.m_lock) == -ERELOAD) goto check_again;
   } else if (rwlock_tryupgrade(&mman_kernel.m_lock) == -EAGAIN) {
    result = PAGE_ERROR;
    goto end_read;
   }
  }
  result = kernel_mmap_anon(result,n_bytes,flags);
  if (result == PAGE_ERROR) goto end2;
  if unlikely(flags&GFP_KERNEL) {
   HEAP_KERNEL = (uintptr_t)result+n_bytes;
   if unlikely(HEAP_KERNEL == KERNEL_VIRT_END)
      HEAP_KERNEL = KERNEL_VIRT_BEGIN;
   assert(HEAP_KERNEL < KERNEL_VIRT_END);
  } else {
   HEAP_SHARED = (uintptr_t)result+n_bytes;
   if unlikely(HEAP_SHARED == SHARED_VIRT_END)
      HEAP_SHARED = SHARED_VIRT_BEGIN;
   assert(HEAP_SHARED >= SHARED_VIRT_BEGIN);
  }
#undef HEAP_SHARED
#undef HEAP_KERNEL
end2: if (has_write_lock) rwlock_endwrite(&mman_kernel.m_lock);
      else end_read:      rwlock_endread(&mman_kernel.m_lock);
end:  if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_END(old_mman);
  task_endnointr();
 }
 if (result == PAGE_ERROR) BAD_ALLOC(n_bytes,flags);
 return result;
}


LOCAL PAGE_ALIGNED void *KCALL
core_page_allocat(PAGE_ALIGNED void *start,
                  size_t n_bytes, gfp_t flags) {
 void *result;
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 if (flags&GFP_MEMORY) {
  result = page_mallocat((ppage_t)start,n_bytes,GFP_GTPAGEATTR(flags));
#ifdef CONFIG_MALLOC_DEBUG_INIT
  if (result != PAGE_ERROR && !(flags&GFP_CALLOC))
      memsetl(result,0x01010101u*(CONFIG_MALLOC_DEBUG_INIT&0xff),PAGESIZE/4);
#endif
 } else {
  bool has_write_lock = false;
  PHYS struct mman *old_mman;
  assert(n_bytes != 0);
  assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
  assert(IS_ALIGNED(n_bytes,PAGESIZE));
  if unlikely((uintptr_t)start+n_bytes < (uintptr_t)start) return PAGE_ERROR;
  if (flags&GFP_KERNEL) {
   if unlikely((uintptr_t)start+n_bytes > KERNEL_VIRT_END) return PAGE_ERROR;
  } else {
   if unlikely((uintptr_t)start < SHARED_VIRT_BEGIN) return PAGE_ERROR;
  }
  task_nointr();
  if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_BEGIN(old_mman);
  if (!(flags&GFP_ATOMIC)) rwlock_read(&mman_kernel.m_lock);
  else if (rwlock_tryread(&mman_kernel.m_lock) == -EAGAIN) {
   result = PAGE_ERROR;
   goto end;
  }
check_again:
  if unlikely(mman_inuse_unlocked(&mman_kernel,(ppage_t)start,n_bytes)) { result = PAGE_ERROR; goto end; }
  /* Upgrade to a write-lock before mapping the space. */
  if (!has_write_lock) {
   has_write_lock = true;
   if (!(flags&GFP_ATOMIC)) {
    if (rwlock_upgrade(&mman_kernel.m_lock) == -ERELOAD) goto check_again;
   } else if (rwlock_tryupgrade(&mman_kernel.m_lock) == -EAGAIN) {
    result = PAGE_ERROR;
    goto end_read;
   }
  }
  result = kernel_mmap_anon(start,n_bytes,flags);
  if (has_write_lock) rwlock_endwrite(&mman_kernel.m_lock);
  else end_read:      rwlock_endread(&mman_kernel.m_lock);
end: if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_END(old_mman);
  task_endnointr();
 }
 return result;
}
LOCAL void KCALL
core_page_free(PAGE_ALIGNED void *ptr,
               size_t n_bytes, gfp_t flags) {
 PHYS struct mman *old_mman;
 assert(IS_ALIGNED((uintptr_t)ptr,PAGESIZE));
 assert((uintptr_t)ptr+CEIL_ALIGN(n_bytes,PAGESIZE) >= (uintptr_t)ptr);
 if (flags&GFP_MEMORY) {
  page_free_((ppage_t)ptr,n_bytes,GFP_GTPAGEATTR(flags));
 } else {
  assert((flags&GFP_KERNEL)
         ? ((uintptr_t)ptr+CEIL_ALIGN(n_bytes,PAGESIZE) <= KERNEL_VIRT_END)
         : ((uintptr_t)ptr >= SHARED_VIRT_BEGIN));
  task_nointr();
  if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_BEGIN(old_mman);
  rwlock_write(&mman_kernel.m_lock);
  kernel_munmap(ptr,n_bytes,flags);
  rwlock_endwrite(&mman_kernel.m_lock);
  if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_END(old_mman);
  task_endnointr();
 }
}





















/* Kernel heap implementation. */
PRIVATE MALIGNED void *KCALL
mheap_acquire(struct mheap *__restrict self, MALIGNED size_t n_bytes,
              MALIGNED size_t *__restrict alloc_bytes, gfp_t flags) {
 MALIGNED void *result = PAGE_ERROR;
 MALIGNED struct mfree **iter,**end;
 MALIGNED struct mfree *chain;
 size_t page_bytes;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(alloc_bytes);
 assert(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
 assert(atomic_rwlock_writing(&self->mh_lock));
 if unlikely(n_bytes < HEAP_MIN_MALLOC)
             n_bytes = HEAP_MIN_MALLOC;
 iter = &self->mh_size[HEAP_BUCKET_OF(n_bytes)];
 end  =  COMPILER_ENDOF(self->mh_size);
 for (; iter != end; ++iter) {
  chain = *iter,assert(chain);
  while (chain != PAGE_ERROR && MFREE_SIZE(chain) < n_bytes)
         chain = chain->mf_lsize.le_next,assert(chain);
  if (chain != PAGE_ERROR) {
   size_t unused_size;
   pgattr_t chain_attr = MFREE_ATTR(chain);
   result = (void *)chain;
#ifdef CONFIG_DEBUG
   {
    struct mfree *del_entry;
    del_entry = mfree_tree_remove(&self->mh_addr,MFREE_BEGIN(chain));
    assertf(del_entry == chain,
            "Invalid tracking for %p...%p (%p != %p)",
            MFREE_MIN(chain),MFREE_MAX(chain),
            MFREE_BEGIN(del_entry),MFREE_BEGIN(chain));
   }
#else
   mfree_tree_remove(&self->mh_addr,MFREE_BEGIN(chain));
#endif
   LIST_REMOVE_EX(chain,mf_lsize,PAGE_ERROR);
   unused_size  = MFREE_SIZE(chain)-n_bytes;
   if (unused_size < HEAP_MIN_MALLOC) {
    /* Remainder is too small. - Allocate it as well. */
    n_bytes += unused_size;
   } else {
    MALIGNED void *unused_begin = (void *)((uintptr_t)chain+n_bytes);
    /* Align the remainder. */
    assert(IS_ALIGNED((uintptr_t)unused_begin,HEAP_ALIGNMENT));
    assert(IS_ALIGNED(unused_size,HEAP_ALIGNMENT));
    mheap_release_nomerge(self,unused_begin,unused_size,chain_attr);
   }
   /* Initialize the result memory. */
   if (flags&GFP_CALLOC) {
    if (chain_attr&PAGEATTR_ZERO)
         memset(result,0,MIN(sizeof(struct mfree),n_bytes));
    else memset(result,0,n_bytes);
   }
#ifdef CONFIG_MALLOC_DEBUG_INIT
   else if (chain_attr&PAGEATTR_ZERO) {
    memset(result,CONFIG_MALLOC_DEBUG_INIT&0xff,n_bytes);
   }
#endif
   *alloc_bytes = n_bytes;
   assert(IS_ALIGNED((uintptr_t)result,HEAP_ALIGNMENT));
   goto end;
  }
 }
 /* Allocate whole pages. */
 page_bytes  = CEIL_ALIGN(n_bytes,PAGESIZE);
 page_bytes += self->mh_overalloc; /* Overallocate a bit. */
core_again:
 result = core_page_alloc(page_bytes,flags);
 if (result == PAGE_ERROR) {
  if (page_bytes == CEIL_ALIGN(n_bytes,PAGESIZE)) goto end;
  /* Try again without overallocation. */
  page_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
  goto core_again;
 }
 if (page_bytes != n_bytes) {
  /* Release all unused memory. */
  if (!mheap_release(self,(void *)((uintptr_t)result+n_bytes),
                     page_bytes-n_bytes,flags))
       n_bytes = page_bytes;
 }
 *alloc_bytes = n_bytes;
end:
#if LOG_MANAGED_ALLOCATIONS
 if (result != PAGE_ERROR) {
  syslogf(LOG_MEM|LOG_ERROR,"ALLOC(%p...%p) (%Iu/%Iu)\n",
          result,(uintptr_t)result+*alloc_bytes-1,*alloc_bytes,n_bytes);
 }
#endif
 return result;
}
PRIVATE MALIGNED void *KCALL
mheap_acquire_at(struct mheap *__restrict self, MALIGNED void *p,
                 MALIGNED size_t n_bytes, MALIGNED size_t *__restrict alloc_bytes,
                 gfp_t flags) {
 struct mfree **pslot,*slot;
 MALIGNED void *result = p;
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level;
 size_t slot_avail;
 size_t unused_before; gfp_t slot_flags;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(alloc_bytes);
 assert(IS_ALIGNED((uintptr_t)p,HEAP_ALIGNMENT));
 assert(IS_ALIGNED((uintptr_t)n_bytes,HEAP_ALIGNMENT));
 assert(atomic_rwlock_writing(&self->mh_lock));
 if unlikely(!n_bytes) return p;
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 /* Search for a free slot at the given address. */
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)p,
                               &addr_semi,&addr_level);
 if unlikely(pslot == PAGE_ERROR) {
  /* Easy enough: the slot doesn't exist, so allocate memory here. */
  PAGE_ALIGNED void *ptr_page = (void *)FLOOR_ALIGN((uintptr_t)p,PAGESIZE);
  PAGE_ALIGNED size_t ptr_size = CEIL_ALIGN(((uintptr_t)ptr_page-(uintptr_t)p)+n_bytes,PAGESIZE);
  slot = (struct mfree *)core_page_allocat(ptr_page,ptr_size,flags);
  if unlikely(slot == PAGE_ERROR) return PAGE_ERROR;
  slot_avail = MFREE_SIZE(slot)-((uintptr_t)p-MFREE_BEGIN(slot));
  slot_flags = flags;
 } else {
  pgattr_t slot_attr;
  slot = *pslot;
  CHECK_HOST_DOBJ(slot);
  assert((uintptr_t)p >= MFREE_MIN(slot));
  assert((uintptr_t)p <= MFREE_MAX(slot));
  assert(MFREE_SIZE(slot) >= HEAP_MIN_MALLOC);
  assert(MFREE_SIZE(slot) > ((uintptr_t)p-MFREE_BEGIN(slot)));
  assert(IS_ALIGNED(MFREE_BEGIN(slot),HEAP_ALIGNMENT));
  slot_avail = MFREE_SIZE(slot)-((uintptr_t)p-MFREE_BEGIN(slot));
  slot_attr  = MFREE_ATTR(slot);
  assert(IS_ALIGNED(slot_avail,HEAP_ALIGNMENT));
  if (slot_avail < n_bytes) {
   MALIGNED void  *missing_addr;
   MALIGNED size_t missing_size;
   MALIGNED size_t missing_alloc;
   /* The slot is too small. - allocate more memory afterwards. */
   missing_addr = (void *)MFREE_END(slot);
   missing_size = n_bytes-slot_avail;
   assert(IS_ALIGNED((uintptr_t)missing_addr,HEAP_ALIGNMENT));
   assert(IS_ALIGNED(missing_size,HEAP_ALIGNMENT));
   if (mheap_acquire_at(self,missing_addr,missing_size,&missing_alloc,flags) == PAGE_ERROR)
       return PAGE_ERROR;
   /* Find the slot again, now that the heap has changed. */
   addr_semi  = ATREE_SEMI0(uintptr_t);
   addr_level = ATREE_LEVEL0(uintptr_t);
   /* Search for a free slot at the given address. */
   pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)p,
                                 &addr_semi,&addr_level);
   assertf(pslot != NULL,"But we know it must exist! (%p...%p)");
   assert(*pslot == slot);
   assert(slot_attr == MFREE_ATTR(*pslot));
   slot_avail += missing_size;
  }
  /* Remove the slot from the address-tree & size-chain. */
  mfree_tree_pop_at(pslot,addr_semi,addr_level);
  LIST_REMOVE_EX(slot,mf_lsize,PAGE_ERROR);
  /* Clear out the slot header. */
  if (slot_attr&PAGEATTR_ZERO)
       memset(slot,0,sizeof(struct mfree));
#ifdef CONFIG_MALLOC_DEBUG_INIT
  else memset(slot,CONFIG_MALLOC_DEBUG_INIT&0xff,sizeof(struct mfree));
#endif
  slot_flags = (flags&~GFP_CALLOC)|GFP_STPAGEATTR(slot_attr);
 }
 unused_before = (uintptr_t)result-MFREE_BEGIN(slot);
 assert(IS_ALIGNED(unused_before,HEAP_ALIGNMENT));
 if (unused_before) {
  /* Make sure that the sub-pre-range isn't too small. */
  if (unused_before < HEAP_MIN_MALLOC) {
   MALIGNED void *slot_before_addr;
   size_t slot_before_size,slot_before_alloc;
   slot_before_size = HEAP_MIN_MALLOC-unused_before;
   assert(IS_ALIGNED(slot_before_size,HEAP_ALIGNMENT));
   assert(slot_before_size != 0);
   slot_before_addr = (void *)((uintptr_t)slot-slot_before_size);
   if (mheap_acquire_at(self,slot_before_addr,slot_before_size,
                       &slot_before_alloc,slot_flags) == PAGE_ERROR) {
    asserte(mheap_release(self,slot,slot_avail+
                        ((uintptr_t)p-MFREE_BEGIN(slot)),
                          slot_flags));
    return PAGE_ERROR;
   }
   /* Got some memory before the slot! */
   assert(IS_ALIGNED(slot_before_alloc,HEAP_ALIGNMENT));
   assert(slot_before_alloc == slot_before_size);
   slot           = (struct mfree *)slot_before_addr;
   unused_before += slot_before_alloc;
  }
  /* Free the unused memory before the slot. */
  mheap_release_nomerge(self,slot,unused_before,slot_flags);
 }
 /* At this point we've allocated 'slot_avail' bytes at 'p'
  * >> Now we must simply try to free as much of the difference as possible. */
 assert(IS_ALIGNED(slot_avail,HEAP_ALIGNMENT));
 assert(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
 assert(slot_avail >= n_bytes);
 {
  size_t unused_size = slot_avail-n_bytes;
  if (unused_size && mheap_release(self,p,unused_size,slot_flags))
      slot_avail -= unused_size;
 }
 /* Do final initialization of memory. */
 if (flags&GFP_CALLOC) {
  if (!(slot_flags&GFP_CALLOC))
        memset(p,0,slot_avail);
 }
#ifdef CONFIG_MALLOC_DEBUG_INIT
 else if (slot_flags&GFP_CALLOC) {
  memset(p,CONFIG_MALLOC_DEBUG_INIT&0xff,slot_avail);
 }
#endif
 *alloc_bytes = slot_avail;
 return p;
}
PRIVATE void KCALL
mheap_release_nomerge(struct mheap *__restrict self, MALIGNED void *p,
                      MALIGNED size_t n_bytes, gfp_t flags) {
 struct mfree **piter,*iter;
 CHECK_HOST_DATA(p,n_bytes);
 assert(IS_ALIGNED((uintptr_t)p,HEAP_ALIGNMENT));
 assert(IS_ALIGNED((uintptr_t)n_bytes,HEAP_ALIGNMENT));
 assert(n_bytes >= HEAP_MIN_MALLOC);
#define NEW_SLOT  ((struct mfree *)p)
#ifdef CONFIG_RESERVE_NULL_PAGE
 assert((uintptr_t)p >= PAGESIZE);
#endif
 assert(atomic_rwlock_writing(&self->mh_lock));
 assert(!(n_bytes&MFREE_ATTRMASK));
 assert((GFP_GTPAGEATTR(flags)&MFREE_ATTRMASK) == GFP_GTPAGEATTR(flags));
 NEW_SLOT->mf_info = n_bytes|GFP_GTPAGEATTR(flags);
 assert(MFREE_SIZE(NEW_SLOT) >= HEAP_MIN_MALLOC);
 mfree_tree_insert(&self->mh_addr,NEW_SLOT);
 /* Figure out where the free-slot should go in the chain of free ranges. */
 piter = &self->mh_size[HEAP_BUCKET_OF(n_bytes)];
 while ((iter = *piter) != PAGE_ERROR &&
         MFREE_SIZE(iter) < n_bytes)
         piter = &iter->mf_lsize.le_next;
 NEW_SLOT->mf_lsize.le_pself = piter;
 NEW_SLOT->mf_lsize.le_next  = iter;
 if (iter != PAGE_ERROR) iter->mf_lsize.le_pself = &NEW_SLOT->mf_lsize.le_next;
 *piter = NEW_SLOT;
 assert(NEW_SLOT->mf_lsize.le_next);
#undef NEW_SLOT
}
PRIVATE bool KCALL
mheap_release(struct mheap *__restrict self, MALIGNED void *p,
              MALIGNED size_t n_bytes, gfp_t flags) {
 struct mfree **pslot,*slot,*iter,*next;
 struct mfree *free_slot;
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level;
 CHECK_HOST_DATA(p,n_bytes);
 assert(atomic_rwlock_writing(&self->mh_lock));
 assert(IS_ALIGNED((uintptr_t)p,HEAP_ALIGNMENT));
 assert(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
 if unlikely(!n_bytes) return true;
 assert(!(GFP_GTPAGEATTR(flags)&MFREE_SIZEMASK));
#if LOG_MANAGED_ALLOCATIONS
 syslogf(LOG_MEM|LOG_ERROR,"FREE(%p...%p)\n",
         p,(uintptr_t)p+n_bytes-1);
#endif
 /* Check for extending a free range above. */
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)p-1,
                               &addr_semi,&addr_level);
 if (pslot != PAGE_ERROR) {
  /* Extend this slot above. */
  slot = *pslot;
  CHECK_HOST_DOBJ(slot);
  assert(MFREE_END(slot) == (uintptr_t)p);
  mfree_tree_pop_at(pslot,addr_semi,addr_level);
#ifdef CONFIG_MALLOC_DEBUG_INIT
  if (slot->mf_attr&PAGEATTR_ZERO && !(flags&GFP_CALLOC))
      memset(slot+1,CONFIG_MALLOC_DEBUG_INIT & 0xff,
             MFREE_SIZE(slot)-sizeof(struct mfree));
#endif
  slot->mf_info = MFREE_MKINFO(MFREE_SIZE(slot)+n_bytes,
                               GFP_GTPAGEATTR(flags));
  addr_semi  = ATREE_SEMI0(uintptr_t);
  addr_level = ATREE_LEVEL0(uintptr_t);
  pslot      = mfree_tree_pinsert_at(&self->mh_addr,slot,&addr_semi,&addr_level);
  iter       = slot->mf_lsize.le_next;
  if (iter != PAGE_ERROR && MFREE_SIZE(iter) < MFREE_SIZE(slot)) {
   /* Fix the size position. */
   LIST_REMOVE_EX(slot,mf_lsize,PAGE_ERROR);
   while ((next = iter->mf_lsize.le_next) != PAGE_ERROR &&
           MFREE_SIZE(next) < MFREE_SIZE(slot)) iter = next;
   /* Re-insert the slot. */
   LIST_INSERT_BEFORE(iter,slot,mf_lsize);
  }
  mheap_unmapfree(self,pslot,addr_semi,addr_level,flags);
  return true;
 }
 /* Check for extending a free range below. */
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)p+n_bytes,
                               &addr_semi,&addr_level);
 if (pslot != PAGE_ERROR) {
  /* Extend this slot below. */
  slot = *pslot;
  CHECK_HOST_DOBJ(slot);
  assert(MFREE_BEGIN(slot) == (uintptr_t)p+n_bytes);
  assertf(MFREE_SIZE(slot) >= HEAP_MIN_MALLOC,
          "MFREE_SIZE(slot) = %Iu\n",MFREE_SIZE(slot));
  mfree_tree_pop_at(pslot,addr_semi,addr_level);
  free_slot = (struct mfree *)p;
  *free_slot          = *slot;
#ifdef CONFIG_MALLOC_DEBUG_INIT
  if (free_slot->mf_attr&PAGEATTR_ZERO && !(flags&GFP_CALLOC))
      memset(slot+1,CONFIG_MALLOC_DEBUG_INIT & 0xff,
             MFREE_SIZE(slot)-sizeof(struct mfree));
#endif
  free_slot->mf_info = MFREE_MKINFO(MFREE_SIZE(free_slot)+n_bytes,
                                    GFP_GTPAGEATTR(flags));
  *free_slot->mf_lsize.le_pself = free_slot;
  if (free_slot->mf_lsize.le_next != PAGE_ERROR)
      free_slot->mf_lsize.le_next->mf_lsize.le_pself = &free_slot->mf_lsize.le_next;
  assert(IS_ALIGNED(MFREE_SIZE(free_slot),HEAP_ALIGNMENT));
  if (free_slot->mf_attr&PAGEATTR_ZERO) {
   if (n_bytes < sizeof(struct mfree))
        memset((void *)((uintptr_t)slot+(sizeof(struct mfree)-n_bytes)),0,n_bytes);
   else memset(slot,0,sizeof(struct mfree));
  }
#ifdef CONFIG_MALLOC_DEBUG_INIT
  else {
   if (n_bytes < sizeof(struct mfree))
        memset((void *)((uintptr_t)slot+(sizeof(struct mfree)-n_bytes)),
                CONFIG_MALLOC_DEBUG_INIT&0xff,n_bytes);
   else memset(slot,CONFIG_MALLOC_DEBUG_INIT&0xff,sizeof(struct mfree));
  }
#endif
  assert(IS_ALIGNED(MFREE_SIZE(free_slot),HEAP_ALIGNMENT));
  addr_semi  = ATREE_SEMI0(uintptr_t);
  addr_level = ATREE_LEVEL0(uintptr_t);
  pslot      = mfree_tree_pinsert_at(&self->mh_addr,free_slot,&addr_semi,&addr_level);
  iter       = free_slot->mf_lsize.le_next;
  if (iter != PAGE_ERROR && MFREE_SIZE(iter) < MFREE_SIZE(free_slot)) {
   /* Fix the size position. */
   LIST_REMOVE_EX(free_slot,mf_lsize,PAGE_ERROR);
   while ((next = iter->mf_lsize.le_next) != PAGE_ERROR &&
           MFREE_SIZE(next) < MFREE_SIZE(free_slot)) iter = next;
   /* Re-insert the slot. */
   LIST_INSERT_BEFORE(iter,free_slot,mf_lsize);
  }
  mheap_unmapfree(self,pslot,addr_semi,addr_level,flags);
  return true;
 }
 if (n_bytes < HEAP_MIN_MALLOC) return false;
 mheap_release_nomerge(self,p,n_bytes,flags);
 return true;
}
PRIVATE void KCALL
mheap_unmapfree(struct mheap *__restrict self,
                struct mfree **__restrict pslot,
                ATREE_SEMI_T(uintptr_t) addr_semi,
                ATREE_LEVEL_T addr_level, gfp_t flags) {
 struct mfree *slot;
 CHECK_HOST_DOBJ(pslot);
 slot = *pslot;
 CHECK_HOST_DOBJ(slot);
 assert(IS_ALIGNED(MFREE_BEGIN(slot),HEAP_ALIGNMENT));
 assert(IS_ALIGNED(MFREE_SIZE(slot),HEAP_ALIGNMENT));
 if (MFREE_SIZE(slot) >= self->mh_freethresh) {
  /* Release full pages. */
  PAGE_ALIGNED void *page_begin = (PAGE_ALIGNED void *)CEIL_ALIGN((uintptr_t)MFREE_BEGIN(slot),PAGESIZE);
  PAGE_ALIGNED void *page_end   = (PAGE_ALIGNED void *)FLOOR_ALIGN((uintptr_t)MFREE_END(slot),PAGESIZE);
  assert(page_begin <= page_end);
  assert((uintptr_t)page_begin >= MFREE_BEGIN(slot));
  assert((uintptr_t)page_end   <= MFREE_END(slot));
  if (page_begin != page_end) {
   /* Unlink the free portion. */
   struct mfree *hi_slot;
   gfp_t page_flags;
   MALIGNED size_t lo_size = (uintptr_t)page_begin-MFREE_BEGIN(slot);
   MALIGNED size_t hi_size = MFREE_END(slot)-(uintptr_t)page_end;
   assertf(IS_ALIGNED(lo_size,HEAP_ALIGNMENT),"lo_size = %Iu\n",lo_size);
   assertf(IS_ALIGNED(hi_size,HEAP_ALIGNMENT),"hi_size = %Iu\n",hi_size);
   hi_slot = (struct mfree *)page_end;
   if (lo_size && lo_size < HEAP_MIN_MALLOC) {
    lo_size += PAGESIZE;
    *(uintptr_t *)&page_begin += PAGESIZE;
    if (page_begin == page_end) return;
   }
   if (hi_size && hi_size < HEAP_MIN_MALLOC) {
    hi_size += PAGESIZE;
    *(uintptr_t *)&page_end -= PAGESIZE;
    *(uintptr_t *)&hi_slot -= PAGESIZE;
    if (page_begin == page_end) return;
   }
   assert(page_begin <= page_end);
   page_flags = GFP_STPAGEATTR(MFREE_ATTR(slot))|(flags&~GFP_CALLOC);

   /* Remove the old slot from the heap. */
   LIST_REMOVE_EX(slot,mf_lsize,PAGE_ERROR);
   mfree_tree_pop_at(pslot,addr_semi,addr_level);

   /* Clear the small amount of memory filled with the free-controller. */
   if (page_flags&GFP_CALLOC)
       memset(slot,0,sizeof(struct mfree));

   /* Create the low & high parts. */
   if (lo_size) mheap_release_nomerge(self,slot,lo_size,page_flags);
   if (hi_size) mheap_release_nomerge(self,hi_slot,hi_size,page_flags);

   /* Release heap data back to the system. */
   core_page_free(page_begin,
                 (PAGE_ALIGNED uintptr_t)page_end-
                 (PAGE_ALIGNED uintptr_t)page_begin,
                  page_flags);
  }
 }
}
PRIVATE MALIGNED void *KCALL
mheap_acquire_al(struct mheap *__restrict self,
                 MALIGNED size_t alignment, MALIGNED size_t offset, MALIGNED size_t n_bytes,
                 MALIGNED size_t *__restrict alloc_bytes, gfp_t flags) {
 void *alloc_base; size_t alloc_size,nouse_size,minalloc_offset;
 void *user_base; struct mptr *result;
 assert(alignment != 0);
 assert((alignment&(alignment-1)) == 0);
 assert(IS_ALIGNED(alignment,HEAP_ALIGNMENT));
 assert(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
 assert(IS_ALIGNED(offset,HEAP_ALIGNMENT));
 if unlikely(n_bytes < HEAP_MIN_MALLOC)
             n_bytes = HEAP_MIN_MALLOC;
 /* Must overallocate by at least 'HEAP_MIN_MALLOC',
  * so we can _always_ free unused lower memory. */
 if (alignment < HEAP_MIN_MALLOC)
      minalloc_offset = HEAP_MIN_MALLOC-alignment;
 else minalloc_offset = 0;
 assert(IS_ALIGNED(minalloc_offset,HEAP_ALIGNMENT));
 alloc_base = mheap_acquire(self,n_bytes+alignment+minalloc_offset,&alloc_size,flags);
 if unlikely(alloc_base == PAGE_ERROR) return PAGE_ERROR;
 user_base = (void *)CEIL_ALIGN((uintptr_t)alloc_base+minalloc_offset+offset,alignment);
 result = (struct mptr *)((uintptr_t)user_base-offset);
 assert((uintptr_t)user_base+n_bytes <=
        (uintptr_t)alloc_base+alloc_size);
 nouse_size = (uintptr_t)result-(uintptr_t)alloc_base;
 assert(nouse_size+n_bytes <= alloc_size);
 assert(nouse_size >= HEAP_MIN_MALLOC);
 /* Release lower memory. */
 asserte(mheap_release(self,alloc_base,nouse_size,flags));
 alloc_size -= nouse_size;

 /* Try to release upper memory. */
 assert(alloc_size >= n_bytes);
 nouse_size = alloc_size-n_bytes;
 if (mheap_release(self,(void *)((uintptr_t)result+n_bytes),nouse_size,flags))
     alloc_size -= nouse_size;
 *alloc_bytes = alloc_size;
 assert(IS_ALIGNED((uintptr_t)result+offset,alignment));
 return result;
}

#ifdef CONFIG_MALLOC_DEBUG_INIT
/* Reset debug pre-initialization of the given memory. */
PRIVATE void KCALL mheap_resetdebug(MALIGNED void *__restrict begin,
                                    MALIGNED size_t n_bytes, gfp_t flags) {
 if (flags&GFP_CALLOC) return;
 /* TODO: if (!(flags&GFP_MEMORY)) { ONLY_MODIFY_ALLOCATED_PAGES(); } */
 while (n_bytes && (uintptr_t)begin & 3) {
  *(*(byte_t **)&begin)++ = CONFIG_MALLOC_DEBUG_INIT&0xff;
  --n_bytes;
 }
 memsetl(begin,0x01010101u*(CONFIG_MALLOC_DEBUG_INIT&0xff),n_bytes/4);
 n_bytes %= 4;
 while (n_bytes) {
  *(*(byte_t **)&begin)++ = CONFIG_MALLOC_DEBUG_INIT&0xff;
  --n_bytes;
 }
}
#else
#define mheap_resetdebug(begin,n_bytes,flags) (void)0
#endif


#if 0
PRIVATE void KCALL
mheap_putmem_nomerge(struct mheap *__restrict self, MALIGNED void *begin,
                     MALIGNED size_t size, pgattr_t flags) {
 struct mfree **piter,*iter;
 CHECK_HOST_DATA(begin,size);
#define NEW_SLOT  ((struct mfree *)begin)
#ifdef CONFIG_RESERVE_NULL_PAGE
 assert((uintptr_t)begin >= PAGESIZE);
#endif
 assert(atomic_rwlock_writing(&self->mh_lock));
 assert(size >= HEAP_MIN_MALLOC);
 assert(IS_ALIGNED((uintptr_t)begin,HEAP_ALIGNMENT));
 assertf(IS_ALIGNED(size,HEAP_ALIGNMENT),"size = %Iu",size);
 assert((flags&MFREE_ATTRMASK) == flags);
 NEW_SLOT->mf_info = size|flags;
 mfree_tree_insert(&self->mh_addr,NEW_SLOT);
 /* Figure out where the free-slot should go in the chain of free ranges. */
 piter = &self->mh_size[HEAP_BUCKET_OF(size)];
 while ((iter = *piter) != PAGE_ERROR &&
         MFREE_SIZE(iter) < size)
         piter = &iter->mf_lsize.le_next;
 NEW_SLOT->mf_lsize.le_pself = piter;
 NEW_SLOT->mf_lsize.le_next  = iter;
 if (iter != PAGE_ERROR) iter->mf_lsize.le_pself = &NEW_SLOT->mf_lsize.le_next;
 *piter = NEW_SLOT;
 assert(NEW_SLOT->mf_lsize.le_next);
#undef NEW_SLOT
}
PRIVATE void KCALL
mheap_delmem(struct mheap *__restrict self, struct mfree **__restrict pslot,
             ATREE_SEMI_T(uintptr_t) addr_semi, ATREE_LEVEL_T addr_level, gfp_t flags) {
 struct mfree *slot;
 CHECK_HOST_DOBJ(pslot);
 slot = *pslot;
 CHECK_HOST_DOBJ(slot);
 assertf(IS_ALIGNED(MFREE_SIZE(slot),HEAP_ALIGNMENT),"%p",MFREE_SIZE(slot));
 if (MFREE_SIZE(slot) >= self->mh_freethresh) {
  /* Release full pages. */
  PAGE_ALIGNED void *page_begin = (PAGE_ALIGNED void *)CEIL_ALIGN((uintptr_t)MFREE_BEGIN(slot),PAGESIZE);
  PAGE_ALIGNED void *page_end   = (PAGE_ALIGNED void *)FLOOR_ALIGN((uintptr_t)MFREE_END(slot),PAGESIZE);
  assert(page_begin <= page_end);
  assert((uintptr_t)page_begin >= MFREE_BEGIN(slot));
  assert((uintptr_t)page_end   <= MFREE_END(slot));
  if (page_begin != page_end) {
   /* Unlink the free portion. */
   struct mfree *hi_slot;
   pgattr_t page_attr;
   MALIGNED size_t lo_size = (uintptr_t)page_begin-MFREE_BEGIN(slot);
   MALIGNED size_t hi_size = MFREE_END(slot)-(uintptr_t)page_end;
   assertf(IS_ALIGNED(lo_size,HEAP_ALIGNMENT),"lo_size = %Iu\n",lo_size);
   assertf(IS_ALIGNED(hi_size,HEAP_ALIGNMENT),"hi_size = %Iu\n",hi_size);
   hi_slot = (struct mfree *)page_end;
   if (lo_size && lo_size < HEAP_MIN_MALLOC) {
    lo_size += PAGESIZE;
    *(uintptr_t *)&page_begin += PAGESIZE;
    if (page_begin == page_end) return;
   }
   if (hi_size && hi_size < HEAP_MIN_MALLOC) {
    hi_size += PAGESIZE;
    *(uintptr_t *)&page_end -= PAGESIZE;
    *(uintptr_t *)&hi_slot -= PAGESIZE;
    if (page_begin == page_end) return;
   }
   assert(page_begin <= page_end);
   page_attr = MFREE_ATTR(slot);

   /* Remove the old slot from the heap. */
   LIST_REMOVE_EX(slot,mf_lsize,PAGE_ERROR);
   mfree_tree_pop_at(pslot,addr_semi,addr_level);

   /* Clear the small amount of memory filled with the free-controller. */
   if (page_attr&PAGEATTR_ZERO)
       memset(slot,0,sizeof(struct mfree));

   /* Create the low & high parts. */
   if (lo_size) mheap_putmem_nomerge(self,slot,lo_size,page_attr);
   if (hi_size) mheap_putmem_nomerge(self,hi_slot,hi_size,page_attr);

   /* Release heap data back to the system. */
   core_page_free(page_begin,
                 (PAGE_ALIGNED uintptr_t)page_end-
                 (PAGE_ALIGNED uintptr_t)page_begin,
                  flags|GFP_STPAGEATTR(page_attr));
  }
 }
}
PRIVATE bool KCALL
mheap_putmem(struct mheap *__restrict self, MALIGNED void *begin,
             MALIGNED size_t n_bytes, gfp_t flags) {
 struct mfree **pslot,*slot,*iter,*next;
 struct mfree *free_slot;
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level;
 CHECK_HOST_DATA(begin,n_bytes);
 assert(atomic_rwlock_writing(&self->mh_lock));
 assert(IS_ALIGNED((uintptr_t)begin,HEAP_ALIGNMENT));
 assertf(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT),"n_bytes = %Iu\n",n_bytes);
 assert(n_bytes != 0);
 assert(!(n_bytes&MFREE_ATTRMASK));
 assert(!(GFP_GTPAGEATTR(flags)&MFREE_SIZEMASK));
#if LOG_MANAGED_ALLOCATIONS
 syslogf(LOG_MEM|LOG_ERROR,"FREE(%p...%p)\n",
         begin,(uintptr_t)begin+n_bytes-1);
#endif
 /* Check for extending a free range above. */
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)begin-1,
                               &addr_semi,&addr_level);
 if (pslot != PAGE_ERROR) {
  /* Extend this slot above. */
  slot = *pslot;
  CHECK_HOST_DOBJ(slot);
  assert(MFREE_END(slot) == (uintptr_t)begin);
  mfree_tree_pop_at(pslot,addr_semi,addr_level);
  slot->mf_attr &= GFP_GTPAGEATTR(flags);
  slot->mf_size += n_bytes;
  addr_semi  = ATREE_SEMI0(uintptr_t);
  addr_level = ATREE_LEVEL0(uintptr_t);
  pslot = mfree_tree_pinsert_at(&self->mh_addr,slot,&addr_semi,&addr_level);
  iter = slot->mf_lsize.le_next;
  if (iter != PAGE_ERROR && MFREE_SIZE(iter) < MFREE_SIZE(slot)) {
   /* Fix the size position. */
   LIST_REMOVE_EX(slot,mf_lsize,PAGE_ERROR);
   while ((next = iter->mf_lsize.le_next) != PAGE_ERROR &&
           MFREE_SIZE(next) < MFREE_SIZE(slot)) iter = next;
   /* Re-insert the slot. */
   LIST_INSERT_BEFORE(iter,slot,mf_lsize);
  }
  mheap_delmem(self,pslot,addr_semi,addr_level,flags);
  return true;
 }
 /* Check for extending a free range below. */
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)begin+n_bytes,
                               &addr_semi,&addr_level);
 if (pslot != PAGE_ERROR) {
  /* Extend this slot below. */
  slot = *pslot;
  CHECK_HOST_DOBJ(slot);
  assertf(MFREE_BEGIN(slot) == (uintptr_t)begin+n_bytes,
          "MFREE_BEGIN(slot)        = %p\n"
          "(uintptr_t)begin         = %p\n"
          "(uintptr_t)begin+n_bytes = %p\n",
          MFREE_BEGIN(slot),(uintptr_t)begin,
          (uintptr_t)begin+n_bytes);
  assert(MFREE_SIZE(slot) >= HEAP_MIN_MALLOC);
  mfree_tree_pop_at(pslot,addr_semi,addr_level);
  free_slot = (struct mfree *)begin;
  *free_slot                    = *slot;
  free_slot->mf_attr           &= GFP_GTPAGEATTR(flags);
  free_slot->mf_size           += n_bytes;
  *free_slot->mf_lsize.le_pself = free_slot;
  if (free_slot->mf_lsize.le_next != PAGE_ERROR)
      free_slot->mf_lsize.le_next->mf_lsize.le_pself = &free_slot->mf_lsize.le_next;
  assertf(IS_ALIGNED(MFREE_SIZE(free_slot),HEAP_ALIGNMENT),"%p",MFREE_SIZE(free_slot));
  if (free_slot->mf_attr&PAGEATTR_ZERO) {
   if (n_bytes < sizeof(struct mfree))
        memset((void *)((uintptr_t)slot+(sizeof(struct mfree)-n_bytes)),0,n_bytes);
   else memset(slot,0,sizeof(struct mfree));
  }
#ifdef CONFIG_MALLOC_DEBUG_INIT
  else {
   if (n_bytes < sizeof(struct mfree))
        memset((void *)((uintptr_t)slot+(sizeof(struct mfree)-n_bytes)),
                CONFIG_MALLOC_DEBUG_INIT&0xff,n_bytes);
   else memset(slot,CONFIG_MALLOC_DEBUG_INIT&0xff,sizeof(struct mfree));
  }
#endif
  assertf(IS_ALIGNED(MFREE_SIZE(free_slot),HEAP_ALIGNMENT),
          "size      = %p\n"
          "free_slot = %p\n"
          "slot      = %p\n",
          MFREE_SIZE(free_slot),
          free_slot,slot);
  addr_semi  = ATREE_SEMI0(uintptr_t);
  addr_level = ATREE_LEVEL0(uintptr_t);
  pslot      = mfree_tree_pinsert_at(&self->mh_addr,free_slot,&addr_semi,&addr_level);
  iter       = free_slot->mf_lsize.le_next;
  if (iter != PAGE_ERROR && MFREE_SIZE(iter) < MFREE_SIZE(free_slot)) {
   /* Fix the size position. */
   LIST_REMOVE_EX(free_slot,mf_lsize,PAGE_ERROR);
   while ((next = iter->mf_lsize.le_next) != PAGE_ERROR &&
           MFREE_SIZE(next) < MFREE_SIZE(free_slot)) iter = next;
   /* Re-insert the slot. */
   LIST_INSERT_BEFORE(iter,free_slot,mf_lsize);
  }
  mheap_delmem(self,pslot,addr_semi,addr_level,flags);
  return true;
 }
 if (n_bytes < HEAP_MIN_MALLOC) {
#if 0
  syslogf(LOG_MEM|LOG_ERROR,"Cannot free small memory chunk %p...%p\n",
          begin,(uintptr_t)begin+n_bytes-1);
#endif
  return false;
 }
 mheap_putmem_nomerge(self,begin,n_bytes,GFP_GTPAGEATTR(flags));
 return true;
}
PRIVATE void *KCALL
mheap_getmem(struct mheap *__restrict self, size_t n_bytes,
             size_t *__restrict alloc_bytes, gfp_t flags) {
 void *result = PAGE_ERROR;
 struct mfree **iter,**end;
 struct mfree *chain;
 size_t page_bytes;
 CHECK_HOST_DOBJ(alloc_bytes);
 assert(atomic_rwlock_writing(&self->mh_lock));
 assert(n_bytes != 0);
 assert(n_bytes >= HEAP_ALIGNMENT);
 if (n_bytes < HEAP_MIN_MALLOC)
     n_bytes = HEAP_MIN_MALLOC;
 iter = &self->mh_size[HEAP_BUCKET_OF(n_bytes)];
 end  =  COMPILER_ENDOF(self->mh_size);
 for (; iter != end; ++iter) {
  chain = *iter,assert(chain);
  while (chain != PAGE_ERROR && MFREE_SIZE(chain) < n_bytes)
         chain = chain->mf_lsize.le_next,assert(chain);
  if (chain != PAGE_ERROR) {
   size_t unused_size;
   pgattr_t chain_attr = MFREE_ATTR(chain);
   result = (void *)chain;
#ifdef CONFIG_DEBUG
   {
    struct mfree *del_entry;
    del_entry = mfree_tree_remove(&self->mh_addr,MFREE_BEGIN(chain));
    assertf(del_entry == chain,
            "Invalid tracking for %p...%p (%p != %p)",
            MFREE_MIN(chain),MFREE_MAX(chain),
            MFREE_BEGIN(del_entry),MFREE_BEGIN(chain));
   }
#else
   mfree_tree_remove(&self->mh_addr,MFREE_BEGIN(chain));
#endif
   LIST_REMOVE_EX(chain,mf_lsize,PAGE_ERROR);
   unused_size  = MFREE_SIZE(chain)-n_bytes;
   if (unused_size < HEAP_MIN_MALLOC) {
    /* Remainder is too small. - Allocate it as well. */
use_remainder:
    n_bytes += unused_size;
#if 0
    syslogf(LOG_MEM|LOG_DEBUG,"Allocating remainder of %Iu bytes\n",unused_size);
#endif
   } else {
    MALIGNED uintptr_t aligned_begin; size_t align_unused;
    uintptr_t unused_begin = (uintptr_t)chain+n_bytes;
    /* Align the remainder. */
    aligned_begin = CEIL_ALIGN(unused_begin,HEAP_ALIGNMENT);
    align_unused  = aligned_begin-unused_begin;
    if (align_unused >= unused_size) goto use_remainder;
    unused_size -= align_unused;
    n_bytes     += align_unused;
    /* Re-insert the remainder. */
    assert(IS_ALIGNED(aligned_begin,HEAP_ALIGNMENT));
    assert(unused_size);
    mheap_putmem_nomerge(self,(void *)aligned_begin,
                         unused_size,GFP_GTPAGEATTR(chain_attr));
   }
   if (flags&GFP_CALLOC) {
    if (chain_attr&PAGEATTR_ZERO) {
     memset(result,0,MIN(sizeof(struct mfree),n_bytes));
    } else {
     memset(result,0,n_bytes);
    }
   }
#ifdef CONFIG_MALLOC_DEBUG_INIT
   else if (chain_attr&PAGEATTR_ZERO) {
    memset(result,CONFIG_MALLOC_DEBUG_INIT&0xff,n_bytes);
   }
#endif
   *alloc_bytes = n_bytes;
   assert(IS_ALIGNED((uintptr_t)result,HEAP_ALIGNMENT));
   goto end;
  }
 }
 n_bytes     = CEIL_ALIGN(n_bytes,HEAP_ALIGNMENT);
 page_bytes  = CEIL_ALIGN(n_bytes,PAGESIZE);
 page_bytes += self->mh_overalloc; /* Overallocate a bit. */
core_again:
 result = core_page_alloc(page_bytes,flags);
 if (result == PAGE_ERROR) {
  if (page_bytes == CEIL_ALIGN(n_bytes,PAGESIZE)) goto end;
  /* Try again without overallocation. */
  page_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
  goto core_again;
 }
 if (page_bytes != n_bytes) {
  /* Release all unused memory. */
  if (!mheap_putmem(self,(void *)((uintptr_t)result+n_bytes),
                    page_bytes-n_bytes,flags))
       n_bytes = page_bytes;
 }
 *alloc_bytes = n_bytes;
end:
#if LOG_MANAGED_ALLOCATIONS
 if (result != PAGE_ERROR) {
  syslogf(LOG_MEM|LOG_ERROR,"ALLOC(%p...%p) (%Iu/%Iu)\n",
          result,(uintptr_t)result+*alloc_bytes-1,*alloc_bytes,n_bytes);
 }
#endif
 return result;
}
PRIVATE void *KCALL
mheap_getmemat(struct mheap *__restrict self, void *addr, size_t n_bytes,
               size_t *__restrict alloc_bytes, gfp_t flags) {
 struct mfree **pslot,*slot; size_t more_n_bytes = 0;
 pgattr_t slot_attr; void *result = addr;
 size_t page_bytes,sub_size,slot_avail;
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level;
 assert(atomic_rwlock_writing(&self->mh_lock));
 assert(n_bytes != 0);
again_slot:
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)addr,
                               &addr_semi,&addr_level);
 if unlikely(pslot == PAGE_ERROR) goto fail;
 slot = *pslot;
 CHECK_HOST_DOBJ(slot);
 assert((uintptr_t)addr >= MFREE_MIN(slot));
 assert((uintptr_t)addr <= MFREE_MAX(slot));
 assert(MFREE_SIZE(slot) > ((uintptr_t)addr-MFREE_BEGIN(slot)));
 slot_avail = MFREE_SIZE(slot)-((uintptr_t)addr-MFREE_BEGIN(slot));
 slot_attr = MFREE_ATTR(slot);
 if (slot_avail >= n_bytes) {
  /* Simple case: The slot is large enough. */
  mfree_tree_pop_at(pslot,addr_semi,addr_level);
  LIST_REMOVE_EX(slot,mf_lsize,PAGE_ERROR);
  if (slot_attr&PAGEATTR_ZERO)
       memset(slot,0,sizeof(struct mfree));
#ifdef CONFIG_MALLOC_DEBUG_INIT
  else memset(slot,CONFIG_MALLOC_DEBUG_INIT&0xff,sizeof(struct mfree));
#endif
  sub_size = (uintptr_t)result-MFREE_BEGIN(slot);
  assert(IS_ALIGNED(sub_size,HEAP_ALIGNMENT));
  if (sub_size) {
   /* Cannot create a sub-free-range this small. */
   if (sub_size < HEAP_MIN_MALLOC) {
    if (more_n_bytes) {
     asserte(mheap_putmem(self,(void *)((uintptr_t)addr+n_bytes),more_n_bytes,flags));
    }
    return PAGE_ERROR;
   }
   mheap_putmem_nomerge(self,slot,sub_size,GFP_GTPAGEATTR(slot_attr));
  }
  /* Figure out memory that can be freed above. */
  n_bytes = CEIL_ALIGN(n_bytes,HEAP_ALIGNMENT);
  sub_size = (slot_avail-n_bytes);
  assert(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
  assert(IS_ALIGNED(slot_avail,HEAP_ALIGNMENT));
  assert(IS_ALIGNED(sub_size,HEAP_ALIGNMENT));
  if (sub_size) {
   if (sub_size < HEAP_MIN_MALLOC) n_bytes += sub_size;
   else {
    mheap_putmem_nomerge(self,(void *)((uintptr_t)result+n_bytes),
                         sub_size,GFP_GTPAGEATTR(slot_attr));
   }
  }
  if (flags&GFP_CALLOC && !(slot_attr&PAGEATTR_ZERO))
      memset(result,0,n_bytes);
#ifdef CONFIG_MALLOC_DEBUG_INIT
  else if (!(flags&GFP_CALLOC) && slot_attr&PAGEATTR_ZERO)
      memset(result,CONFIG_MALLOC_DEBUG_INIT&0xff,n_bytes);
#endif
  goto end;
 }
 /* The slot is too small. - Try to allocate memory after it. */
 assert(more_n_bytes == 0);
 slot_avail   = MFREE_SIZE(slot);
 if (mheap_getmemat(self,(void *)MFREE_END(slot),
                            n_bytes-slot_avail,
                           &more_n_bytes,flags) == PAGE_ERROR)
     return PAGE_ERROR;
 /* OK! We've managed to allocate all the required upper memory!
  * Now go back and allocate the lower slot, of which we already
  * know that is must exist. */
 n_bytes = slot_avail;
 goto again_slot;
end:
 *alloc_bytes = n_bytes+more_n_bytes;
 return result;
fail:
 assert(more_n_bytes == 0);
 if (!IS_ALIGNED((uintptr_t)addr,PAGESIZE)) return PAGE_ERROR;
 /* For page-aligned addresses, try to HEAP_ALLOC_AT the data. */
 page_bytes  = CEIL_ALIGN(n_bytes,PAGESIZE);
 page_bytes += self->mh_overalloc; /* Overallocate a bit. */
core_again:
 result = core_page_allocat(addr,page_bytes,flags);
 if (result == PAGE_ERROR) {
  /* Try again without overallocation. */
  if (page_bytes == CEIL_ALIGN(n_bytes,PAGESIZE)) goto end;
  page_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
  goto core_again;
 }
 assert(page_bytes >= n_bytes);
 if (page_bytes != n_bytes) {
  sub_size = page_bytes-n_bytes;
  if (sub_size < HEAP_MIN_MALLOC) n_bytes += sub_size;
  else {
   /* Release all unused memory. */
   if (!mheap_putmem(self,(void *)((uintptr_t)result+n_bytes),sub_size,flags))
        n_bytes += sub_size;
  }
 }
 goto end;
}
PRIVATE void *KCALL
mheap_getmemal(struct mheap *__restrict self, size_t alignment,
               ssize_t offset, size_t n_bytes, size_t *__restrict alloc_bytes,
               gfp_t flags) {
 void *alloc_base; size_t alloc_size,nouse_size,align_offset;
 void *user_base; struct mptr *result; uintptr_t user_end;
 assert(alignment != 0);
 assert((alignment&(alignment-1)) == 0);
 assert(alignment > HEAP_ALIGNMENT);
 assert(n_bytes != 0);
 alloc_base = mheap_getmem(self,n_bytes+alignment,&alloc_size,flags);
 if unlikely(alloc_base == PAGE_ERROR) return PAGE_ERROR;
 user_base = (void *)CEIL_ALIGN((uintptr_t)alloc_base+offset,alignment);
 result = (struct mptr *)((uintptr_t)user_base-offset);
 assert((uintptr_t)user_base+n_bytes <= (uintptr_t)alloc_base+alloc_size);
 nouse_size = (uintptr_t)result-(uintptr_t)alloc_base;
 assert(nouse_size+n_bytes <= alloc_size);
 if (nouse_size) {
  asserte(mheap_putmem(self,alloc_base,nouse_size,flags));
  alloc_size -= nouse_size;
 }
 /* Try to release upper memory. */
 n_bytes         = CEIL_ALIGN(n_bytes,HEAP_ALIGNMENT);
 assert(alloc_size >= n_bytes);
 nouse_size   = alloc_size-n_bytes;
 user_end     = (uintptr_t)result+n_bytes;
 alloc_base   = (void *)CEIL_ALIGN(user_end,HEAP_ALIGNMENT);
 align_offset = (uintptr_t)alloc_base-user_end;
 if (nouse_size > align_offset) {
  nouse_size -= align_offset;
  if (mheap_putmem(self,alloc_base,nouse_size,flags))
      alloc_size -= nouse_size;
 }
 *alloc_bytes = alloc_size;
 return result;
}
#endif

PRIVATE struct mptr *KCALL
mheap_malloc(struct mheap *__restrict self, size_t size, gfp_t flags) {
 struct mptr *result;
 MALIGNED size_t alloc_size;
 atomic_rwlock_write(&self->mh_lock);
 result = (struct mptr *)mheap_acquire(self,CEIL_ALIGN(MPTR_SIZEOF(size),HEAP_ALIGNMENT),
                                      &alloc_size,flags);
 atomic_rwlock_endwrite(&self->mh_lock);
 if unlikely(result == PAGE_ERROR) return MPTR_OF(NULL);
 assert(alloc_size >= size);
 result->m_data = MPTR_MKDATA(flags&GFP_MASK_TYPE,alloc_size);
 MPTR_VALIDATE(result);
 return result;
}
PRIVATE struct mptr *KCALL
mheap_memalign(struct mheap *__restrict self,
               size_t alignment, size_t size,
               gfp_t flags) {
 struct mptr *result;
 MALIGNED size_t alloc_size;
 atomic_rwlock_write(&self->mh_lock);
 result = (struct mptr *)mheap_acquire_al(self,alignment,sizeof(struct mptr),
                                          CEIL_ALIGN(MPTR_SIZEOF(size),HEAP_ALIGNMENT),
                                         &alloc_size,flags);
 atomic_rwlock_endwrite(&self->mh_lock);
 if (result == PAGE_ERROR) return MPTR_OF(NULL);
 assert(IS_ALIGNED((uintptr_t)result+sizeof(struct mptr),alignment));
 result->m_data = MPTR_MKDATA(flags&GFP_MASK_TYPE,alloc_size);
 MPTR_VALIDATE(result);
 return result;
}
PRIVATE void KCALL
mheap_free(struct mheap *__restrict self, struct mptr *ptr, gfp_t flags) {
 MALIGNED size_t alloc_size = MPTR_SIZE(ptr);
 assertf(alloc_size >= MPTR_SIZEOF(0),
         "alloc_size = %Iu\n",alloc_size);
 if (flags&GFP_CALLOC) {
  /* Clear the header + tail. */
#ifdef MPTR_HAVE_TAIL
  memset(MPTR_TAILADDR(ptr),0,MPTR_TAILSIZE(ptr));
#endif
  memset(ptr,0,sizeof(struct mptr));
 }
#ifdef CONFIG_MALLOC_DEBUG_INIT
 else {
  mheap_resetdebug(ptr,alloc_size,flags);
 }
#endif
 atomic_rwlock_write(&self->mh_lock);
 asserte(mheap_release(self,ptr,alloc_size,flags));
 atomic_rwlock_endwrite(&self->mh_lock);
}

#ifndef __INTELLISENSE__
DECL_END
#define HEAP_REALIGN
#include "malloc-realign.c.inl"
#include "malloc-realign.c.inl"
DECL_BEGIN
#endif


PRIVATE int (KCALL core_mallopt)(int parameter_number,
                                 int parameter_value,
                                 gfp_t flags) {
 struct mheap *heap;
 assert((flags&GFP_MASK_TYPE) < MHEAP_COUNT);
 heap = &mheap_kernel[flags&GFP_MASK_TYPE];
 CHECK_HOST_DOBJ(heap);
 switch (parameter_number) {
 case M_TRIM_THRESHOLD:
  if (parameter_value < PAGESIZE) return 0;
  heap->mh_freethresh = parameter_value;
  break;
 case M_GRANULARITY:
  heap->mh_overalloc = parameter_value;
  break;
 default: return 0;
 }
 return 1;
}


/* General purpose memory API. */
#define MPTR_KMALLOC(size,flags)          mheap_malloc(&mheap_kernel[(flags)&GFP_MASK_TYPE],size,flags)
#define MPTR_KMEMALIGN(alignment,size,flags) \
 (unlikely(alignment <= HEAP_ALIGNMENT) ? MPTR_KMALLOC(size,flags) : \
  mheap_memalign(&mheap_kernel[(flags)&GFP_MASK_TYPE],alignment,size,flags))
#define MPTR_KFREE(ptr)                   mheap_free(MPTR_HEAP(ptr),ptr,MPTR_TYPE(ptr))
#define MPTR_KZFREE(ptr)                  mheap_free(MPTR_HEAP(ptr),ptr,MPTR_TYPE(ptr)|GFP_CALLOC)
#define MPTR_KFREE2(ptr,flags)            mheap_free(MPTR_HEAP(ptr),ptr,MPTR_TYPE(ptr)|((flags)&~GFP_MASK_TYPE))
#define MPTR_KMALLOC_USABLE_SIZE(ptr)     MPTR_USERSIZE(ptr)
#define MPTR_KMALLOC_FLAGS(ptr)           MPTR_TYPE(ptr)
#define MPTR_KREALLOC(ptr,new_size,flags) mheap_realloc(MPTR_HEAP(ptr),ptr,new_size,MPTR_TYPE(ptr)|((flags)&~GFP_MASK_TYPE))
#define MPTR_KREALIGN(ptr,alignment,new_size,flags) \
 (likely(alignment > HEAP_ALIGNMENT) \
  ? mheap_realign(MPTR_HEAP(ptr),ptr,alignment,new_size,MPTR_TYPE(ptr)|((flags)&~GFP_MASK_TYPE))\
  : mheap_realloc(MPTR_HEAP(ptr),ptr,new_size,MPTR_TYPE(ptr)|((flags)&~GFP_MASK_TYPE)))


#define KMALLOC(size,flags)                    MPTR_USERADDR(MPTR_KMALLOC(size,flags))
#define KMEMALIGN(alignment,size,flags)        MPTR_USERADDR(MPTR_KMEMALIGN(alignment,size,flags))
#define KFREE(ptr)                            (M_ISNONNULL(ptr) ? MPTR_KFREE(MPTR_GET(ptr)) : (void)0)
#define KZFREE(ptr)                           (M_ISNONNULL(ptr) ? MPTR_KZFREE(MPTR_GET(ptr)) : (void)0)
#define KFREE2(ptr,flags)                     (M_ISNONNULL(ptr) ? MPTR_KFREE2(MPTR_GET(ptr),flags) : (void)0)
#define KMALLOC_USABLE_SIZE(ptr)              (M_ISNONNULL(ptr) ? MPTR_KMALLOC_USABLE_SIZE(MPTR_GET(ptr)) : 0)
#define KMALLOC_FLAGS(ptr)                    (M_ISNONNULL(ptr) ? MPTR_KMALLOC_FLAGS(MPTR_GET(ptr)) : GFP_NORMAL)
#define KREALLOC(ptr,size,flags)               MPTR_USERADDR(M_ISNONNULL(ptr) ? MPTR_KREALLOC(MPTR_GET(ptr),size,flags) : MPTR_KMALLOC(size,flags))
#define KREALIGN(ptr,alignment,size,flags)     MPTR_USERADDR(M_ISNONNULL(ptr) ? MPTR_KREALIGN(MPTR_GET(ptr),alignment,size,flags) : MPTR_KMEMALIGN(alignment,size,flags))

/* Public malloc/kmalloc/mall Api implementations. */
#ifndef MALLOC_DEBUG_API
PUBLIC void  *(KCALL kmalloc)(size_t size, gfp_t flags) { return KMALLOC(size,flags); }
PUBLIC void  *(KCALL krealloc)(void *ptr, size_t size, gfp_t flags) { return KREALLOC(ptr,size,flags); }
PUBLIC void  *(KCALL kmemalign)(size_t alignment, size_t size, gfp_t flags) { return KMEMALIGN(alignment,size,flags); }
PUBLIC void  *(KCALL krealign)(void *ptr, size_t alignment, size_t size, gfp_t flags) { return KREALIGN(ptr,alignment,size,flags); }
PUBLIC void  *(KCALL kmemdup)(void const *__restrict ptr, size_t size, gfp_t flags) { register void *result = kmalloc(size,flags); if (result) memcpy(result,ptr,size); return result; }
PUBLIC void  *(KCALL kmemadup)(void const *__restrict ptr, size_t alignment, size_t size, gfp_t flags) { register void *result = kmemalign(alignment,size,flags); if (result) memcpy(result,ptr,size); return result; }
PUBLIC void   (KCALL kfree)(void *ptr) { KFREE(ptr); }
PUBLIC void   (KCALL kzfree)(void *ptr) { KZFREE(ptr); }
PUBLIC size_t (KCALL kmalloc_usable_size)(void *ptr) { return KMALLOC_USABLE_SIZE(ptr); }
PUBLIC gfp_t  (KCALL kmalloc_flags)(void *ptr) { return KMALLOC_FLAGS(ptr); }
PUBLIC int    (KCALL kmallopt)(int parameter_number, int parameter_value, gfp_t flags) { return core_mallopt(parameter_number,parameter_value,flags); }
PUBLIC void  *(KCALL _kmalloc_d)(size_t size, gfp_t flags, DEBUGINFO_UNUSED) { return KMALLOC(size,flags); }
PUBLIC void  *(KCALL _krealloc_d)(void *ptr, size_t size, gfp_t flags, DEBUGINFO_UNUSED) { return KREALLOC(ptr,size,flags); }
PUBLIC void  *(KCALL _kmemalign_d)(size_t alignment, size_t size, gfp_t flags, DEBUGINFO_UNUSED) { return KMEMALIGN(alignment,size,flags); }
PUBLIC void  *(KCALL _krealign_d)(void *ptr, size_t alignment, size_t size, gfp_t flags, DEBUGINFO_UNUSED) { return KREALIGN(ptr,alignment,size,flags); }
PUBLIC void  *(KCALL _kmemdup_d)(void const *__restrict ptr, size_t size, gfp_t flags, DEBUGINFO_UNUSED) { return kmemdup(ptr,size,flags); }
PUBLIC void  *(KCALL _kmemadup_d)(void const *__restrict ptr, size_t alignment, size_t size, gfp_t flags, DEBUGINFO_UNUSED) { return kmemadup(ptr,alignment,size,flags); }
PUBLIC void   (KCALL _kfree_d)(void *ptr, DEBUGINFO_UNUSED) { KFREE(ptr); }
PUBLIC void   (KCALL _kzfree_d)(void *ptr, DEBUGINFO_UNUSED) { KZFREE(ptr); }
PUBLIC size_t (KCALL _kmalloc_usable_size_d)(void *ptr, DEBUGINFO_UNUSED) { return KMALLOC_USABLE_SIZE(ptr); }
PUBLIC gfp_t  (KCALL _kmalloc_flags_d)(void *ptr, DEBUGINFO_UNUSED) { return KMALLOC_FLAGS(ptr); }
PUBLIC int    (KCALL _kmallopt_d)(int parameter_number, int parameter_value, gfp_t flags, DEBUGINFO_UNUSED) { return core_mallopt(parameter_number,parameter_value,flags); }

/* MALL Api stubs. */
PUBLIC void   *(KCALL _mall_getattrib)(void *__restrict UNUSED(mallptr), int UNUSED(attrib)) { return NULL; }
PUBLIC ssize_t (KCALL _mall_traceback)(void *__restrict UNUSED(mallptr), __ptbwalker UNUSED(callback), void *UNUSED(closure)) { return 0; }
PUBLIC ssize_t (KCALL _mall_enum)(struct module *UNUSED(mod), void *UNUSED(checkpoint), ssize_t (*callback)(void *__restrict mallptr, void *closure), void *UNUSED(closure)) { (void)callback; return 0; }
PUBLIC void    (KCALL _mall_printleaks)(struct module *UNUSED(mod)) {}
PUBLIC void    (KCALL _mall_validate)(struct module *UNUSED(mod)) {}
PUBLIC void   *(KCALL _mall_untrack)(void *mallptr) { return mallptr; }
PUBLIC void   *(KCALL _mall_nofree)(void *mallptr) { return mallptr; }
PUBLIC void   *(KCALL _mall_getattrib_d)(void *__restrict UNUSED(mallptr), int UNUSED(attrib), DEBUGINFO_UNUSED) { return NULL; }
PUBLIC ssize_t (KCALL _mall_traceback_d)(void *__restrict UNUSED(mallptr), __ptbwalker UNUSED(callback), void *UNUSED(closure), DEBUGINFO_UNUSED) { return 0; }
PUBLIC ssize_t (KCALL _mall_enum_d)(struct module *UNUSED(mod), void *UNUSED(checkpoint), ssize_t (*callback)(void *__restrict mallptr, void *closure), void *UNUSED(closure), DEBUGINFO_UNUSED) { (void)callback; return 0; }
PUBLIC void    (KCALL _mall_printleaks_d)(struct module *UNUSED(mod), DEBUGINFO_UNUSED) {}
PUBLIC void    (KCALL _mall_validate_d)(struct module *UNUSED(mod), DEBUGINFO_UNUSED) {}
PUBLIC void   *(KCALL _mall_untrack_d)(void *mallptr, DEBUGINFO_UNUSED) { return mallptr; }
PUBLIC void   *(KCALL _mall_nofree_d)(void *mallptr, DEBUGINFO_UNUSED) { return mallptr; }

/* Public malloc Api implementation. */
PUBLIC void *(KCALL malloc)(size_t n_bytes) { return KMALLOC(n_bytes,__GFP_MALLOC); }
PUBLIC void *(KCALL calloc)(size_t count, size_t n_bytes) { return KMALLOC(count*n_bytes,__GFP_MALLOC|GFP_CALLOC); }
PUBLIC void *(KCALL memalign)(size_t alignment, size_t n_bytes) { return KMEMALIGN(alignment,n_bytes,__GFP_MALLOC); }
PUBLIC void *(KCALL realloc)(void *__restrict mallptr, size_t n_bytes) { return KREALLOC(mallptr,n_bytes,__GFP_MALLOC); }
PUBLIC void *(KCALL realloc_in_place)(void *__restrict mallptr, size_t n_bytes) { return KREALLOC(mallptr,n_bytes,__GFP_MALLOC|GFP_NOMOVE); }
PUBLIC void *(KCALL valloc)(size_t n_bytes) { return KMEMALIGN(PAGESIZE,n_bytes,__GFP_MALLOC); }
PUBLIC void *(KCALL pvalloc)(size_t n_bytes) { return KMEMALIGN(PAGESIZE,(n_bytes+PAGESIZE-1)&~(PAGESIZE-1),__GFP_MALLOC); }
PUBLIC int   (KCALL mallopt)(int parameter_number, int parameter_value) { return core_mallopt(parameter_number,parameter_value,__GFP_MALLOC); }
PUBLIC int   (KCALL _mallopt_d)(int parameter_number, int parameter_value, DEBUGINFO_UNUSED) { return core_mallopt(parameter_number,parameter_value,__GFP_MALLOC); }
PUBLIC void *(KCALL _malloc_d)(size_t n_bytes, DEBUGINFO_UNUSED) { return KMALLOC(n_bytes,__GFP_MALLOC); }
PUBLIC void *(KCALL _calloc_d)(size_t count, size_t n_bytes, DEBUGINFO_UNUSED) { return KMALLOC(count*n_bytes,__GFP_MALLOC|GFP_CALLOC); }
PUBLIC void *(KCALL _memalign_d)(size_t alignment, size_t n_bytes, DEBUGINFO_UNUSED) { return KMEMALIGN(alignment,n_bytes,__GFP_MALLOC); }
PUBLIC void *(KCALL _realloc_d)(void *__restrict mallptr, size_t n_bytes, DEBUGINFO_UNUSED) { return KREALLOC(mallptr,n_bytes,__GFP_MALLOC); }
PUBLIC void *(KCALL _realloc_in_place_d)(void *__restrict mallptr, size_t n_bytes, DEBUGINFO_UNUSED) { return KREALLOC(mallptr,n_bytes,__GFP_MALLOC|GFP_NOMOVE); }
PUBLIC void *(KCALL _valloc_d)(size_t n_bytes, DEBUGINFO_UNUSED) { return KMEMALIGN(PAGESIZE,n_bytes,__GFP_MALLOC); }
PUBLIC void *(KCALL _pvalloc_d)(size_t n_bytes, DEBUGINFO_UNUSED) { return KMEMALIGN(PAGESIZE,(n_bytes+PAGESIZE-1)&~(PAGESIZE-1),__GFP_MALLOC); }
PUBLIC int   (KCALL _posix_memalign_d)(void **__restrict pp, size_t alignment, size_t n_bytes, DEBUGINFO_UNUSED) { return (posix_memalign)(pp,alignment,n_bytes); }
PUBLIC char *(KCALL _strdup_d)(char const *__restrict str, DEBUGINFO_UNUSED) { return (strdup)(str); }
PUBLIC char *(KCALL _strndup_d)(char const *__restrict str, size_t max_chars, DEBUGINFO_UNUSED) { return (strndup)(str,max_chars); }
PUBLIC char *(      _strdupf_d)(DEBUGINFO_UNUSED, char const *__restrict format, ...) { char *result; va_list args; va_start(args,format); result = (vstrdupf)(format,args); va_end(args); return result; }
PUBLIC char *(KCALL _vstrdupf_d)(char const *__restrict format, va_list args, DEBUGINFO_UNUSED) { return (vstrdupf)(format,args); }
PUBLIC void *(KCALL _memdup_d)(void const *__restrict ptr, size_t n_bytes, DEBUGINFO_UNUSED) { return (memdup)(ptr,n_bytes); }
PUBLIC void *(KCALL _memcdup_d)(void const *__restrict ptr, int needle, size_t n_bytes, DEBUGINFO_UNUSED) { return (memcdup)(ptr,needle,n_bytes); }

/* Additional mall functions that require longer code. */
PUBLIC char *(KCALL strdup)(char const *__restrict str) {
 char *result; size_t len = strlen(str)+1;
 result = (char *)KMALLOC(len*sizeof(char),__GFP_MALLOC);
 if (result) memcpy(result,str,len*sizeof(char));
 return result;
}
PUBLIC char *(KCALL strndup)(char const *__restrict str, size_t max_chars) {
 char *result;
 max_chars = strnlen(str,max_chars);
 result = (char *)KMALLOC((max_chars+1)*sizeof(char),__GFP_MALLOC);
 if (result) {
  memcpy(result,str,max_chars*sizeof(char));
  result[max_chars] = '\0';
 }
 return result;
}
PUBLIC void *(KCALL memdup)(void const *__restrict ptr, size_t n_bytes) {
 void *result = KMALLOC(n_bytes,__GFP_MALLOC);
 if (result) memcpy(result,ptr,n_bytes);
 return result;
}
PUBLIC void *(KCALL memcdup)(void const *__restrict ptr,
                             int needle, size_t n_bytes) {
 if (n_bytes) {
  void const *endaddr = memchr(ptr,needle,n_bytes-1);
  if (endaddr) n_bytes = ((uintptr_t)endaddr-(uintptr_t)ptr)+1;
 }
 return memdup(ptr,n_bytes);
}
PUBLIC int (KCALL posix_memalign)(void **__restrict pp,
                                  size_t alignment,
                                  size_t n_bytes) {
 void *result = NULL;
 CHECK_HOST_DOBJ(pp);
 if (alignment == HEAP_ALIGNMENT)
  result = KMEMALIGN(alignment,n_bytes,__GFP_MALLOC);
 else {
  size_t d = alignment / sizeof(void*);
  size_t r = alignment % sizeof(void*);
  if (r != 0 || d == 0 || (d & (d-1)) != 0)
      return EINVAL;
  result = KMEMALIGN(alignment,n_bytes,__GFP_MALLOC);
 }
 if (!result) return ENOMEM;
 *pp = result;
 return 0;
}
struct strdup_formatdata { char *start,*iter,*end; };
static NONNULL((1,3)) ssize_t
(KCALL strdupf_printer)(char const *__restrict data, size_t datalen,
                        struct strdup_formatdata *__restrict fmt) {
 char *newiter;
 newiter = fmt->iter+datalen;
 if (newiter > fmt->end) {
  size_t newsize = (size_t)(fmt->end-fmt->start);
  assert(newsize);
  do newsize *= 2;
  while (fmt->start+newsize < newiter);
  /* Realloc the strdup string */
  newiter = (char *)KREALLOC(fmt->start,
                            (newsize+1)*sizeof(char),
                             __GFP_MALLOC);
  if unlikely(!newiter) {
   /* If there isn't enough memory, retry
    * with a smaller buffer before giving up. */
   newsize = (fmt->end-fmt->start)+datalen;
   newiter = (char *)KREALLOC(fmt->start,
                             (newsize+1)*sizeof(char),
                              __GFP_MALLOC);
   if unlikely(!newiter) return -1; /* Nothing we can do (out of memory...) */
  }
  fmt->iter = newiter+(fmt->iter-fmt->start);
  fmt->start = newiter;
  fmt->end = newiter+newsize;
 }
 memcpy(fmt->iter,data,datalen);
 fmt->iter += datalen;
 return datalen;
}

PUBLIC char *(KCALL vstrdupf)(char const *__restrict format, va_list args) {
 struct strdup_formatdata data;
 /* Try to do a (admittedly very bad) prediction on the required size. */
 size_t format_length = (strlen(format)*3)/2;
 data.start = (char *)KMALLOC((format_length+1)*sizeof(char),__GFP_MALLOC);
 if unlikely(!data.start) {
  /* Failed to allocate initial buffer (try with a smaller one) */
  format_length = 1;
  data.start = (char *)KMALLOC(2*sizeof(char),__GFP_MALLOC);
  if unlikely(!data.start) return NULL;
 }
 data.end = data.start+format_length;
 data.iter = data.start;
 if unlikely(format_vprintf((pformatprinter)&strdupf_printer,
                            &data,format,args) != 0) {
  free(data.start); /* Out-of-memory */
  return NULL;
 }
 *data.iter = '\0';
 if likely(data.iter != data.end) {
  /* Try to realloc the string one last time to save up on memory */
  data.end = (char *)KREALLOC(data.start,
                            ((data.iter-data.start)+1)*sizeof(char),
                              __GFP_MALLOC);
  if likely(data.end) data.start = data.end;
 }
 return data.start;
}
PUBLIC char *(strdupf)(char const *__restrict format, ...) {
 char *result;
 va_list args;
 va_start(args,format);
 result = vstrdupf(format,args);
 va_end(args);
 return result;
}
#else /* !MALLOC_DEBUG_API */

PRIVATE void   *(KCALL debug_malloc)(struct dsetup *__restrict setup, size_t size, gfp_t flags);
PRIVATE void   *(KCALL debug_realloc)(struct dsetup *__restrict setup, void *ptr, size_t size, gfp_t flags);
PRIVATE void   *(KCALL debug_memalign)(struct dsetup *__restrict setup, size_t alignment, size_t size, gfp_t flags);
PRIVATE void   *(KCALL debug_realign)(struct dsetup *__restrict setup, void *ptr, size_t alignment, size_t size, gfp_t flags);
PRIVATE void    (KCALL debug_free)(struct dsetup *__restrict setup, void *ptr, gfp_t flags);
PRIVATE size_t  (KCALL debug_malloc_usable_size)(struct dsetup *__restrict setup, void *ptr);
PRIVATE gfp_t   (KCALL debug_malloc_flags)(struct dsetup *__restrict setup, void *ptr);
PRIVATE void   *(KCALL debug_memdup)(struct dsetup *__restrict setup, void const *__restrict ptr, size_t size, gfp_t flags);
PRIVATE void   *(KCALL debug_memcdup)(struct dsetup *__restrict setup, void const *__restrict ptr, int needle, size_t size, gfp_t flags);
PRIVATE int     (KCALL debug_mallopt)(struct dsetup *__restrict setup, int parameter_number, int parameter_value, gfp_t flags);
PRIVATE char   *(KCALL debug_strdup)(struct dsetup *__restrict setup, char const *__restrict str, gfp_t flags);
PRIVATE char   *(KCALL debug_strndup)(struct dsetup *__restrict setup, char const *__restrict str, size_t max_chars, gfp_t flags);
PRIVATE int     (KCALL debug_posix_memalign)(struct dsetup *__restrict setup, void **__restrict pp, size_t alignment, size_t n_bytes, gfp_t flags);
PRIVATE char   *(KCALL debug_vstrdupf)(struct dsetup *__restrict setup, char const *__restrict format, va_list args, gfp_t flags);
PRIVATE void   *(KCALL debug_getattrib)(struct dsetup *__restrict setup, void *__restrict mallptr, int attrib);
PRIVATE ssize_t (KCALL debug_traceback)(struct dsetup *__restrict setup, void *__restrict mallptr, __ptbwalker callback, void *closure);
PRIVATE ssize_t (KCALL debug_enum)(struct dsetup *__restrict setup, struct module *mod, void *checkpoint, ssize_t (*callback)(void *__restrict mallptr, void *closure), void *closure);
PRIVATE void    (KCALL debug_printleaks)(struct dsetup *__restrict setup, struct module *mod);
PRIVATE void    (KCALL debug_validate)(struct dsetup *__restrict setup, struct module *mod);
PRIVATE void   *(KCALL debug_untrack)(struct dsetup *__restrict setup, void *mallptr);
PRIVATE void   *(KCALL debug_nofree)(struct dsetup *__restrict setup, void *mallptr);


PRIVATE void *(KCALL debug_malloc)(struct dsetup *__restrict setup,
                                   size_t size, gfp_t flags) {
 struct mptr *p = MPTR_KMALLOC(size,flags);
 if (p != MPTR_OF(NULL)) mptr_setup(p,setup,size);
 return MPTR_USERADDR(p);
}
PRIVATE void *(KCALL debug_memalign)(struct dsetup *__restrict setup,
                                     size_t alignment, size_t size, gfp_t flags) {
 struct mptr *p = MPTR_KMEMALIGN(alignment,size,flags);
 if (p != MPTR_OF(NULL)) mptr_setup(p,setup,size);
 return MPTR_USERADDR(p);
}
PRIVATE void *(KCALL debug_realloc)(struct dsetup *__restrict setup,
                                    void *ptr, size_t size, gfp_t flags) {
 struct mptr *p;
 if (M_ISNONNULL(ptr))
     return MPTR_USERADDR(MPTR_KREALLOC(MPTR_GET(ptr),size,flags));
 if ((p = MPTR_KMALLOC(size,flags)) != MPTR_OF(NULL))
      mptr_setup(p,setup,size);
 return MPTR_USERADDR(p);
}
PRIVATE void *(KCALL debug_realign)(struct dsetup *__restrict setup,
                                    void *ptr, size_t alignment,
                                    size_t size, gfp_t flags) {
 struct mptr *p;
 if (M_ISNONNULL(ptr))
     return MPTR_KREALIGN(MPTR_GET(ptr),alignment,size,flags);
 if ((p = MPTR_KMEMALIGN(alignment,size,flags)) != MPTR_OF(NULL))
      mptr_setup(p,setup,size);
 return MPTR_USERADDR(p);
}

PRIVATE void (KCALL debug_free)(struct dsetup *__restrict setup, void *ptr, gfp_t flags) {
 if (M_ISNONNULL(ptr)) {
  struct mptr *p = MPTR_GET(ptr);
  mptr_unlink(p);
  MPTR_KFREE2(p,flags);
 }
}
PRIVATE size_t (KCALL debug_malloc_usable_size)(struct dsetup *__restrict setup, void *ptr) { return KMALLOC_USABLE_SIZE(ptr); }
PRIVATE gfp_t (KCALL debug_malloc_flags)(struct dsetup *__restrict setup, void *ptr) { return KMALLOC_FLAGS(ptr); }

PRIVATE void *(KCALL debug_memdup)(struct dsetup *__restrict setup,
                                   void const *__restrict ptr, size_t size,
                                   gfp_t flags) {
 void *result = debug_malloc(setup,size,flags);
 if (result) memcpy(result,ptr,size);
 return result;
}
PRIVATE void *(KCALL debug_memcdup)(struct dsetup *__restrict setup,
                                    void const *__restrict ptr, int needle,
                                    size_t size, gfp_t flags) {
 if (size) {
  void const *endaddr = memchr(ptr,needle,size-1);
  if (endaddr) size = ((uintptr_t)endaddr-(uintptr_t)ptr)+1;
 }
 return debug_memdup(setup,ptr,size,flags);
}
PRIVATE void *(KCALL debug_memadup)(struct dsetup *__restrict setup,
                                    void const *__restrict ptr,
                                    size_t alignment, size_t size,
                                    gfp_t flags) {
 void *result = debug_memalign(setup,alignment,size,flags);
 if (result) memcpy(result,ptr,size);
 return result;
}
PRIVATE int (KCALL debug_mallopt)(struct dsetup *__restrict setup,
                                  int parameter_number, int parameter_value,
                                  gfp_t flags) {
 (void)setup;
 /* TODO */
 return core_mallopt(parameter_number,parameter_value,flags);
}
PRIVATE char *(KCALL debug_strdup)(struct dsetup *__restrict setup,
                                   char const *__restrict str, gfp_t flags) {
 size_t size = (strlen(str)+1)*sizeof(char);
 void *result = debug_malloc(setup,size,flags);
 if (result) memcpy(result,str,size);
 return (char *)result;
}
PRIVATE char *(KCALL debug_strndup)(struct dsetup *__restrict setup,
                                    char const *__restrict str,
                                    size_t max_chars, gfp_t flags) {
 size_t size = strnlen(str,max_chars);
 char *result = (char *)debug_malloc(setup,(size+1)*sizeof(char),flags);
 if (result) memcpy(result,str,size*sizeof(char)),
             result[size] = '\0';
 return result;
}
PRIVATE int (KCALL debug_posix_memalign)(struct dsetup *__restrict setup,
                                         void **__restrict pp, size_t alignment,
                                         size_t n_bytes, gfp_t flags) {
 void *result = NULL;
 CHECK_HOST_DOBJ(pp);
 if (alignment == HEAP_ALIGNMENT)
  result = debug_memalign(setup,alignment,n_bytes,GFP_NORMAL);
 else {
  size_t d = alignment / sizeof(void*);
  size_t r = alignment % sizeof(void*);
  if (r != 0 || d == 0 || (d & (d-1)) != 0)
      return EINVAL;
  result = debug_memalign(setup,alignment,n_bytes,GFP_NORMAL);
 }
 if (!result) return ENOMEM;
 *pp = result;
 return 0;
}
PRIVATE char *(KCALL debug_vstrdupf)(struct dsetup *__restrict setup,
                                     char const *__restrict format,
                                     va_list args, gfp_t flags) {
 /* Minimal implementation (Not meant for speed) */
 va_list args_copy; size_t result_size; char *result;
 va_copy(args_copy,args);
 result_size = (vsnprintf(NULL,0,format,args_copy)+1)*sizeof(char);
 va_end(args_copy);
 result = (char *)debug_malloc(setup,result_size,flags);
 if (result) vsnprintf(result,result_size,format,args);
 return result;
}


PRIVATE struct mptr *KCALL
mptr_safeload(struct dsetup *__restrict setup,
              void *__restrict p) {
 struct mptr *result;
 CHECK_HOST_DOBJ(setup);
 result = MPTR_OF(p);
 if (!OK_HOST_DATA(result,sizeof(struct mptr))) {
  /* TODO */
 }
 MPTR_VALIDATE(result);
 /* TODO */
 return result;
}
PRIVATE void KCALL
mptr_setup(struct mptr *__restrict self,
           struct dsetup *__restrict setup,
           size_t user_size) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(setup);
 MPTR_VALIDATE(self);
 assert(MPTR_SIZEOF(user_size) <= MPTR_SIZE(self));
 (void)user_size;
#ifdef CONFIG_TRACE_LEAKS
 self->m_refcnt = 1;
 self->m_flag   = MPTRFLAG_NONE;
 self->m_info   = setup->s_info;
#endif /* !CONFIG_TRACE_LEAKS */
#ifdef MPTR_HAVE_PAD
 memset(self->m_pad,0xaa,sizeof(self->m_pad));
#endif
#ifdef CONFIG_MALLOC_HEADSIZE
 { size_t i;
   for (i = 0; i < CONFIG_MALLOC_HEADSIZE; ++i)
        self->m_head[i] = MALL_HEADERBYTE(i);
 }
#endif

 /* Initialize the tail. */
#ifdef MPTR_HAVE_TAIL
 { struct mptr_tail *tail; size_t tail_size;
   tail = (struct mptr_tail *)((uintptr_t)MPTR_USERADDR(self)+user_size);
   tail_size = ((uintptr_t)self+MPTR_SIZE(self))-(uintptr_t)tail;
#if defined(CONFIG_MALLOC_FOOTSIZE) && defined(CONFIG_MALLOC_TRACEBACK)
   assertf(tail_size >= CONFIG_MALLOC_FOOTSIZE+CONFIG_MALLOC_TRACEBACK*sizeof(void *),"%Iu",tail_size);
#elif defined(CONFIG_MALLOC_FOOTSIZE)
   assertf(tail_size >= CONFIG_MALLOC_FOOTSIZE,"%Iu",tail_size);
#else
   assertf(tail_size >= CONFIG_MALLOC_TRACEBACK*sizeof(void *),"%Iu",tail_size);
#endif
#ifdef CONFIG_MALLOC_FOOTSIZE
   { size_t i;
     for (i = 0; i < CONFIG_MALLOC_FOOTSIZE; ++i)
          tail->t_foot[i] = MALL_FOOTERBYTE(i);
   }
#ifdef CONFIG_MALLOC_TRACEBACK
   tail_size -= CONFIG_MALLOC_FOOTSIZE;
#endif
#endif
#ifdef CONFIG_MALLOC_TRACEBACK
   //tail->t_tb; /* TODO (Fill up to 'tail_size' bytes) */
#endif
   self->m_tail = tail;
 }
#endif
#ifdef CONFIG_TRACE_LEAKS
 self->m_chksum = mptr_chksum(self);
#endif
 mptr_relink(self);
}
#ifdef CONFIG_TRACE_LEAKS
PRIVATE u16 KCALL
mptr_chksum(struct mptr *__restrict self) {
 /* TODO */
 return 42;
}
#endif /* CONFIG_TRACE_LEAKS */

#ifdef MPTR_HAVE_TAIL
PRIVATE void KCALL
mptr_mvtail(struct mptr *__restrict self,
            size_t old_size, size_t new_size,
            gfp_t flags) {
 void *old_tail,*new_tail; size_t tail_size;
 CHECK_HOST_DOBJ(self);
 assert(MPTR_SIZE(self) == new_size);
 assert(new_size >= MPTR_SIZEOF(0));
 assert(IS_ALIGNED(old_size,HEAP_ALIGNMENT));
 assert(IS_ALIGNED(new_size,HEAP_ALIGNMENT));
 old_tail  = self->m_tail;
 new_tail  = (void *)(((uintptr_t)old_tail-old_size)+new_size);
 tail_size = ((uintptr_t)self+new_size)-(uintptr_t)new_tail;
 assertf(tail_size < new_size,
         "old_tail  = %p\n"
         "tail_size = %Iu\n"
         "old_size  = %Iu\n"
         "new_size  = %Iu\n",
         old_tail,tail_size,old_size,new_size);
 memmove(new_tail,old_tail,tail_size);
 self->m_tail = (struct mptr_tail *)new_tail;
#ifndef CONFIG_MALLOC_DEBUG_INIT
 if (flags&GFP_CALLOC)
#endif
 {
  void *nolap_begin;
  size_t nolap_size;
  if (old_tail < new_tail) {
   nolap_begin = old_tail;
   nolap_size  = (uintptr_t)new_tail-(uintptr_t)old_tail;
  } else {
   nolap_begin = (void *)((uintptr_t)new_tail+tail_size);
   nolap_size  = ((uintptr_t)old_tail+tail_size)-(uintptr_t)nolap_begin;
  }
#ifdef CONFIG_MALLOC_DEBUG_INIT
  if (flags&GFP_CALLOC)
       memset(nolap_begin,0,nolap_size);
  else memset(nolap_begin,CONFIG_MALLOC_DEBUG_INIT&0xff,nolap_size);
#else
  memset(nolap_begin,0,nolap_size);
#endif
 }
 /* Update the checksum */
 self->m_chksum = mptr_chksum(self);
}
#endif /* MPTR_HAVE_TAIL */
#ifdef CONFIG_TRACE_LEAKS
PRIVATE void KCALL
mptr_unlink(struct mptr *__restrict self) {
 CHECK_HOST_DOBJ(self);
 /* TODO */
}
PRIVATE void KCALL
mptr_relink(struct mptr *__restrict self) {
 CHECK_HOST_DOBJ(self);
 /* TODO */
}
#endif /* CONFIG_TRACE_LEAKS */

PRIVATE void *(KCALL debug_getattrib)(struct dsetup *__restrict setup,
                                      void *__restrict mallptr, int attrib) {
 (void)setup;
 (void)mallptr;
 (void)attrib;
 /* TODO */
 return NULL;
}
PRIVATE ssize_t (KCALL debug_traceback)(struct dsetup *__restrict setup,
                                        void *__restrict mallptr,
                                        __ptbwalker callback, void *closure) {
 (void)setup;
 (void)mallptr;
 (void)callback;
 (void)closure;
 /* TODO */
 return 0;
}
PRIVATE ssize_t (KCALL debug_enum)(struct dsetup *__restrict setup, struct module *mod,
                                   void *checkpoint, ssize_t (*callback)(void *__restrict mallptr,
                                                                         void *closure),
                                   void *closure) {
 (void)setup;
 (void)mod;
 (void)checkpoint;
 (void)callback;
 (void)closure;
 /* TODO */
 return 0;
}
PRIVATE void (KCALL debug_printleaks)(struct dsetup *__restrict setup, struct module *mod) {
 (void)setup;
 (void)mod;
 /* TODO */
}
PRIVATE void (KCALL debug_validate)(struct dsetup *__restrict setup, struct module *mod) {
 (void)setup;
 (void)mod;
 /* TODO */
}
PRIVATE void *(KCALL debug_untrack)(struct dsetup *__restrict setup, void *mallptr) {
 (void)setup;
 /* TODO */
 return mallptr;
}
PRIVATE void *(KCALL debug_nofree)(struct dsetup *__restrict setup, void *mallptr) {
 (void)setup;
 /* TODO */
 return mallptr;
}


#define GET_EBP(r) __asm__ __volatile__("movl %%ebp, %0" : "=g" (r))

#define DEFINE_SETUP(name) \
  struct dsetup name; \
  memset(&name.s_info,0,sizeof(struct dinfo)); \
  name.s_info.i_inst = THIS_INSTANCE; \
  GET_EBP(name.s_tbebp)
PUBLIC ATTR_NOINLINE void *(KCALL kmalloc)(size_t size, gfp_t flags)                                                { DEFINE_SETUP(setup); return debug_malloc(&setup,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL krealloc)(void *ptr, size_t size, gfp_t flags)                                    { DEFINE_SETUP(setup); return debug_realloc(&setup,ptr,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL kmemalign)(size_t alignment, size_t size, gfp_t flags)                            { DEFINE_SETUP(setup); return debug_memalign(&setup,alignment,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL krealign)(void *ptr, size_t alignment, size_t size, gfp_t flags)                  { DEFINE_SETUP(setup); return debug_realign(&setup,ptr,alignment,size,flags); }
PUBLIC ATTR_NOINLINE void (KCALL kfree)(void *ptr)                                                                  { DEFINE_SETUP(setup);        debug_free(&setup,ptr,GFP_NORMAL); }
PUBLIC ATTR_NOINLINE void (KCALL kzfree)(void *ptr)                                                                 { DEFINE_SETUP(setup);        debug_free(&setup,ptr,GFP_CALLOC); }
PUBLIC ATTR_NOINLINE size_t (KCALL kmalloc_usable_size)(void *ptr)                                                  { DEFINE_SETUP(setup); return debug_malloc_usable_size(&setup,ptr); }
PUBLIC ATTR_NOINLINE gfp_t (KCALL kmalloc_flags)(void *ptr)                                                         { DEFINE_SETUP(setup); return debug_malloc_flags(&setup,ptr); }
PUBLIC ATTR_NOINLINE void *(KCALL kmemdup)(void const *__restrict ptr, size_t size, gfp_t flags)                    { DEFINE_SETUP(setup); return debug_memdup(&setup,ptr,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL kmemadup)(void const *__restrict ptr, size_t alignment, size_t size, gfp_t flags) { DEFINE_SETUP(setup); return debug_memadup(&setup,ptr,alignment,size,flags); }
PUBLIC ATTR_NOINLINE int (KCALL kmallopt)(int parameter_number, int parameter_value, gfp_t flags)                   { DEFINE_SETUP(setup); return debug_mallopt(&setup,parameter_number,parameter_value,flags); }

PUBLIC ATTR_NOINLINE int (KCALL mallopt)(int parameter_number, int parameter_value)                     { DEFINE_SETUP(setup); return debug_mallopt(&setup,parameter_number,parameter_value,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL malloc)(size_t n_bytes)                                               { DEFINE_SETUP(setup); return debug_malloc(&setup,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL calloc)(size_t count, size_t n_bytes)                                 { DEFINE_SETUP(setup); return debug_malloc(&setup,count*n_bytes,__GFP_MALLOC|GFP_CALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL memalign)(size_t alignment, size_t n_bytes)                           { DEFINE_SETUP(setup); return debug_memalign(&setup,alignment,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL realloc)(void *__restrict mallptr, size_t n_bytes)                    { DEFINE_SETUP(setup); return debug_realloc(&setup,mallptr,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL realloc_in_place)(void *__restrict mallptr, size_t n_bytes)           { DEFINE_SETUP(setup); return debug_realloc(&setup,mallptr,n_bytes,__GFP_MALLOC|GFP_NOMOVE); }
PUBLIC ATTR_NOINLINE void *(KCALL valloc)(size_t n_bytes)                                               { DEFINE_SETUP(setup); return debug_memalign(&setup,PAGESIZE,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL pvalloc)(size_t n_bytes)                                              { DEFINE_SETUP(setup); return debug_memalign(&setup,PAGESIZE,(n_bytes+PAGESIZE-1)&~(PAGESIZE-1),__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(KCALL strdup)(char const *__restrict str)                                   { DEFINE_SETUP(setup); return debug_strdup(&setup,str,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(KCALL strndup)(char const *__restrict str, size_t max_chars)                { DEFINE_SETUP(setup); return debug_strndup(&setup,str,max_chars,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL memdup)(void const *__restrict ptr, size_t n_bytes)                   { DEFINE_SETUP(setup); return debug_memdup(&setup,ptr,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL memcdup)(void const *__restrict ptr, int needle, size_t n_bytes)      { DEFINE_SETUP(setup); return debug_memcdup(&setup,ptr,needle,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE int (KCALL posix_memalign)(void **__restrict pp, size_t alignment, size_t n_bytes) { DEFINE_SETUP(setup); return debug_posix_memalign(&setup,pp,alignment,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(KCALL vstrdupf)(char const *__restrict format, va_list args)                { DEFINE_SETUP(setup); return debug_vstrdupf(&setup,format,args,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(strdupf)(char const *__restrict format, ...)                                { va_list args; char *result; DEFINE_SETUP(setup); va_start(args,format); result = debug_vstrdupf(&setup,format,args,__GFP_MALLOC); va_end(args); return result; }

PUBLIC ATTR_NOINLINE void *(KCALL _mall_getattrib)(void *__restrict mallptr, int attrib) { DEFINE_SETUP(setup); return debug_getattrib(&setup,mallptr,attrib); }
PUBLIC ATTR_NOINLINE ssize_t (KCALL _mall_traceback)(void *__restrict mallptr, __ptbwalker callback,
                                                     void *closure)                      { DEFINE_SETUP(setup); return debug_traceback(&setup,mallptr,callback,closure); }
PUBLIC ATTR_NOINLINE ssize_t (KCALL _mall_enum)(struct module *mod, void *checkpoint, ssize_t (*callback)(void *__restrict mallptr, void *closure),
                                                void *closure)                           { DEFINE_SETUP(setup); return debug_enum(&setup,mod,checkpoint,callback,closure); }
PUBLIC ATTR_NOINLINE void (KCALL _mall_printleaks)(struct module *mod)                   { DEFINE_SETUP(setup); debug_printleaks(&setup,mod); }
PUBLIC ATTR_NOINLINE void (KCALL _mall_validate)(struct module *mod)                     { DEFINE_SETUP(setup); debug_validate(&setup,mod); }
PUBLIC ATTR_NOINLINE void *(KCALL _mall_untrack)(void *mallptr)                          { DEFINE_SETUP(setup); return debug_untrack(&setup,mallptr); }
PUBLIC ATTR_NOINLINE void *(KCALL _mall_nofree)(void *mallptr)                           { DEFINE_SETUP(setup); return debug_nofree(&setup,mallptr); }
#undef DEFINE_SETUP

#define DEFINE_SETUP(name) \
  struct dsetup name; \
  name.s_info.i_file = __file; \
  name.s_info.i_line = __line; \
  name.s_info.i_func = __func; \
  name.s_info.i_inst = __inst; \
  GET_EBP(name.s_tbebp)

PUBLIC ATTR_NOINLINE void *(KCALL _kmalloc_d)(size_t size, gfp_t flags, DEBUGINFO)                                                { DEFINE_SETUP(setup); return debug_malloc(&setup,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL _krealloc_d)(void *ptr, size_t size, gfp_t flags, DEBUGINFO)                                    { DEFINE_SETUP(setup); return debug_realloc(&setup,ptr,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL _kmemalign_d)(size_t alignment, size_t size, gfp_t flags, DEBUGINFO)                            { DEFINE_SETUP(setup); return debug_memalign(&setup,alignment,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL _krealign_d)(void *ptr, size_t alignment, size_t size, gfp_t flags, DEBUGINFO)                  { DEFINE_SETUP(setup); return debug_realign(&setup,ptr,alignment,size,flags); }
PUBLIC ATTR_NOINLINE void (KCALL _kfree_d)(void *ptr, DEBUGINFO)                                                                  { DEFINE_SETUP(setup);        debug_free(&setup,ptr,GFP_NORMAL); }
PUBLIC ATTR_NOINLINE void (KCALL _kzfree_d)(void *ptr, DEBUGINFO)                                                                 { DEFINE_SETUP(setup);        debug_free(&setup,ptr,GFP_CALLOC); }
PUBLIC ATTR_NOINLINE size_t (KCALL _kmalloc_usable_size_d)(void *ptr, DEBUGINFO)                                                  { DEFINE_SETUP(setup); return debug_malloc_usable_size(&setup,ptr); }
PUBLIC ATTR_NOINLINE gfp_t (KCALL _kmalloc_flags_d)(void *ptr, DEBUGINFO)                                                         { DEFINE_SETUP(setup); return debug_malloc_flags(&setup,ptr); }
PUBLIC ATTR_NOINLINE void *(KCALL _kmemdup_d)(void const *__restrict ptr, size_t size, gfp_t flags, DEBUGINFO)                    { DEFINE_SETUP(setup); return debug_memdup(&setup,ptr,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL _kmemadup_d)(void const *__restrict ptr, size_t alignment, size_t size, gfp_t flags, DEBUGINFO) { DEFINE_SETUP(setup); return debug_memadup(&setup,ptr,alignment,size,flags); }
PUBLIC ATTR_NOINLINE int (KCALL _kmallopt_d)(int parameter_number, int parameter_value, gfp_t flags, DEBUGINFO)                   { DEFINE_SETUP(setup); return debug_mallopt(&setup,parameter_number,parameter_value,flags); }

PUBLIC ATTR_NOINLINE int (KCALL _mallopt_d)(int parameter_number, int parameter_value, DEBUGINFO)                     { DEFINE_SETUP(setup); return debug_mallopt(&setup,parameter_number,parameter_value,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _malloc_d)(size_t n_bytes, DEBUGINFO)                                               { DEFINE_SETUP(setup); return debug_malloc(&setup,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _calloc_d)(size_t count, size_t n_bytes, DEBUGINFO)                                 { DEFINE_SETUP(setup); return debug_malloc(&setup,count*n_bytes,__GFP_MALLOC|GFP_CALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _memalign_d)(size_t alignment, size_t n_bytes, DEBUGINFO)                           { DEFINE_SETUP(setup); return debug_memalign(&setup,alignment,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _realloc_d)(void *__restrict mallptr, size_t n_bytes, DEBUGINFO)                    { DEFINE_SETUP(setup); return debug_realloc(&setup,mallptr,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _realloc_in_place_d)(void *__restrict mallptr, size_t n_bytes, DEBUGINFO)           { DEFINE_SETUP(setup); return debug_realloc(&setup,mallptr,n_bytes,__GFP_MALLOC|GFP_NOMOVE); }
PUBLIC ATTR_NOINLINE void *(KCALL _valloc_d)(size_t n_bytes, DEBUGINFO)                                               { DEFINE_SETUP(setup); return debug_memalign(&setup,PAGESIZE,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _pvalloc_d)(size_t n_bytes, DEBUGINFO)                                              { DEFINE_SETUP(setup); return debug_memalign(&setup,PAGESIZE,(n_bytes+PAGESIZE-1)&~(PAGESIZE-1),__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(KCALL _strdup_d)(char const *__restrict str, DEBUGINFO)                                   { DEFINE_SETUP(setup); return debug_strdup(&setup,str,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(KCALL _strndup_d)(char const *__restrict str, size_t max_chars, DEBUGINFO)                { DEFINE_SETUP(setup); return debug_strndup(&setup,str,max_chars,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _memdup_d)(void const *__restrict ptr, size_t n_bytes, DEBUGINFO)                   { DEFINE_SETUP(setup); return debug_memdup(&setup,ptr,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _memcdup_d)(void const *__restrict ptr, int needle, size_t n_bytes, DEBUGINFO)      { DEFINE_SETUP(setup); return debug_memcdup(&setup,ptr,needle,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE int (KCALL _posix_memalign_d)(void **__restrict pp, size_t alignment, size_t n_bytes, DEBUGINFO) { DEFINE_SETUP(setup); return debug_posix_memalign(&setup,pp,alignment,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(KCALL _vstrdupf_d)(char const *__restrict format, va_list args, DEBUGINFO)                { DEFINE_SETUP(setup); return debug_vstrdupf(&setup,format,args,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(_strdupf_d)(DEBUGINFO, char const *__restrict format, ...)                                { va_list args; char *result; DEFINE_SETUP(setup); va_start(args,format); result = debug_vstrdupf(&setup,format,args,__GFP_MALLOC); va_end(args); return result; }

PUBLIC ATTR_NOINLINE void *(KCALL _mall_getattrib_d)(void *__restrict mallptr, int attrib, DEBUGINFO) { DEFINE_SETUP(setup); return debug_getattrib(&setup,mallptr,attrib); }
PUBLIC ATTR_NOINLINE ssize_t (KCALL _mall_traceback_d)(void *__restrict mallptr, __ptbwalker callback,
                                                       void *closure, DEBUGINFO)                      { DEFINE_SETUP(setup); return debug_traceback(&setup,mallptr,callback,closure); }
PUBLIC ATTR_NOINLINE ssize_t (KCALL _mall_enum_d)(struct module *mod, void *checkpoint, ssize_t (*callback)(void *__restrict mallptr, void *closure),
                                                  void *closure, DEBUGINFO)                           { DEFINE_SETUP(setup); return debug_enum(&setup,mod,checkpoint,callback,closure); }
PUBLIC ATTR_NOINLINE void (KCALL _mall_printleaks_d)(struct module *mod, DEBUGINFO)                   { DEFINE_SETUP(setup); debug_printleaks(&setup,mod); }
PUBLIC ATTR_NOINLINE void (KCALL _mall_validate_d)(struct module *mod, DEBUGINFO)                     { DEFINE_SETUP(setup); debug_validate(&setup,mod); }
PUBLIC ATTR_NOINLINE void *(KCALL _mall_untrack_d)(void *mallptr, DEBUGINFO)                          { DEFINE_SETUP(setup); return debug_untrack(&setup,mallptr); }
PUBLIC ATTR_NOINLINE void *(KCALL _mall_nofree_d)(void *mallptr, DEBUGINFO)                           { DEFINE_SETUP(setup); return debug_nofree(&setup,mallptr); }
#undef DEFINE_SETUP
#endif /* MALLOC_DEBUG_API */

DECL_END

#endif /* !GUARD_SRC_KERNEL_CORE_MALLOC_C */
