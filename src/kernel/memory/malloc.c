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
#ifndef GUARD_KERNEL_MEMORY_MALLOC_C
#define GUARD_KERNEL_MEMORY_MALLOC_C 1
#define _KOS_SOURCE 1

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#include "../debug-config.h"

#include <assert.h>
#include <format-printer.h>
#include <hybrid/asm.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/debuginfo.h>
#include <hybrid/list/atree.h>
#include <hybrid/list/list.h>
#include <hybrid/minmax.h>
#include <hybrid/debug.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-owner-rwlock.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/traceback.h>
#include <hybrid/typecore.h>
#include <kernel/interrupt.h>
#include <kernel/malloc.h>
#include <kernel/memory.h>
#include <kernel/mman.h>
#include <kernel/paging-util.h>
#include <kernel/paging.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/task.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <arch/hints.h>
#include <sys/mman.h>
#if defined(__i386__) || defined(__x86_64__)
#include <asm/instx.h>
#include <arch/hints.h>
#endif

#define MALIGNED /* Annotation for an integral/pointer aligned by `HEAP_ALIGNMENT' */

DECL_BEGIN

#if 0 /* Disable all debug functionality. */
#define CONFIG_MALLOC_NO_DEBUG_INIT
#undef CONFIG_TRACE_LEAKS
#define CONFIG_MALLOC_HEADSIZE 0
#define CONFIG_MALLOC_FOOTSIZE 0
#define CONFIG_MALLOC_TRACEBACK 0
#endif

/* MALLOC configuration options:
 *
 * >> #define CONFIG_DEBUG_HEAP <BOOL>
 *    Enable all debug functionality, as well as additional
 *    internal integrity checks for heap-based kernel allocations.
 *    This option is enabled by default when the kernel core is
 *    being compiled in debug mode (CONFIG_DEBUG).
 *    Define as 0 to disable in every context.
 * 
 * >> [GLOBAL] #define CONFIG_TRACE_LEAKS
 *    When defined, track all dynamically allocated pointers,
 *    based on which `struct instance' allocated them.
 *    This option must be defined in `.sources', as it affects
 *    the layout of data structures elsewhere, too.
 *    NOTE: In addition, this option enables validation of a
 *          16-bit checksum whenever an mall-pointer is loaded,
 *          essentially enabling additional safety constraints
 *          whenever operating on dynamically allocated memory.
 *    
 * >> #define CONFIG_MALLOC_HEADSIZE <SIZE>
 *    DEFAULT: IFDEF(CONFIG_DEBUG_HEAP): HEAP_ALIGNMENT
 *    DEFAULT: ELSE:                     0
 *    Set the size of a small area of memory to-be allocated
 *    before all heap pointers.
 *    During (re-)allocation, this area is filled with random,
 *    but consistent values that are later re-checked for
 *    validity, thus allowing for early detection of most
 *    buffer underflow attacks.
 *    
 * >> #define CONFIG_MALLOC_FOOTSIZE <SIZE>
 *    DEFAULT: IFDEF(CONFIG_DEBUG_HEAP): HEAP_ALIGNMENT
 *    DEFAULT: ELSE:                     0
 *    Similar to `CONFIG_MALLOC_HEADSIZE', but located at
 *    the other end, this option is useful for detecting
 *    buffer overflow attacks.
 *    WARNING: Enabling this option guaranties 1-byte alignment
 *             in `kmalloc_usable_size()', disallowing any
 *             access to trailing data during allocations.
 *
 * >> #define CONFIG_MALLOC_NO_DEBUG_INIT <BOOL>
 *    DEFAULT: != DEFINED(CONFIG_DEBUG_HEAP)
 * >> #define CONFIG_MALLOC_DEBUG_INIT   <BYTE>
 *    DEFAULT: IFNDEF(CONFIG_MALLOC_NO_DEBUG_INIT)
 *             IFDEF (CONFIG_DEBUG_HEAP): 0xc3
 *    Define the default initialization of heap-allocated memory
 *    to always allocate data pre-initialized to `CONFIG_DEBUG_HEAP'
 *    This option only affects memory not allocated with `GFP_CALLOC',
 *    and is internally implemented by making use of mregion
 *    initialization, meaning that enabling this option will not
 *    increase memory allocation overhead/waste when allocating
 *    virtual (GFP_KERNEL or GFP_SHARED/GFP_NORMAL) memory.
 *    NOTE: `0xc3' was chosen because of its unique binary layout: `0b11000011'
 * 
 * >> #define CONFIG_MALLOC_TRACEBACK <BOOL>
 *    DEFAULT:  IFDEF(CONFIG_TRACE_LEAKS): 1
 *    REQUIRES: IFDEF(CONFIG_TRACE_LEAKS)
 * >> #define CONFIG_MALLOC_TRACEBACK_MINSIZE <SIZE>
 *    DEFAULT:  IFDEF(CONFIG_MALLOC_TRACEBACK): 4
 *    Store a short traceback of at least `CONFIG_MALLOC_TRACEBACK_MINSIZE'
 *    entries within the trailing memory located after every allocated pointer.
 * 
 * >> #define CONFIG_MALLOC_NO_FREQUENCY <BOOL>
 *    DEFAULT: != DEFINED(CONFIG_DEBUG_HEAP)
 * >> #define CONFIG_MALLOC_FREQUENCY    <UNSIGNED INT>
 *    DEFAULT: IFNDEF(CONFIG_MALLOC_NO_FREQUENCY)
 *             IFDEF (CONFIG_DEBUG_HEAP): 1024
 *    The frequency on which the global state of all kernel heaps
 *    are checked for errors, such as access-after-free, among others.
 *    The frequency is automatically triggered once for every call to
 *    any allocating, or free-ing function.
 *    Note that is operation is very costly and the frequency (when available)
 *    can be re-configured using mallopt(M_MALL_CHECK_FREQUENCY) at runtime.
 */


#ifndef CONFIG_DEBUG_HEAP
#ifdef CONFIG_DEBUG
#   define CONFIG_DEBUG_HEAP 1
#endif
#elif (CONFIG_DEBUG_HEAP+0) == 0
#   undef CONFIG_DEBUG_HEAP
#endif
#ifdef CONFIG_MALLOC_NO_DEBUG_INIT
#if (CONFIG_MALLOC_NO_DEBUG_INIT+0) == 0
#   undef CONFIG_MALLOC_NO_DEBUG_INIT
#endif
#endif
#ifndef CONFIG_MALLOC_DEBUG_INIT
#if !defined(CONFIG_MALLOC_NO_DEBUG_INIT) && \
     defined(CONFIG_DEBUG_HEAP)
#   define CONFIG_MALLOC_DEBUG_INIT KERNEL_DEBUG_MEMPAT_KMALLOC
#endif
#elif defined(CONFIG_MALLOC_NO_DEBUG_INIT)
#   undef  CONFIG_MALLOC_DEBUG_INIT
#endif
#if !defined(CONFIG_MALLOC_DEBUG_INIT) && \
    !defined(CONFIG_MALLOC_NO_DEBUG_INIT)
#define CONFIG_MALLOC_NO_DEBUG_INIT
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
#ifdef CONFIG_DEBUG_HEAP
#   define CONFIG_MALLOC_HEADSIZE HEAP_ALIGNMENT
#endif
#elif (CONFIG_MALLOC_HEADSIZE+0) == 0
#   undef  CONFIG_MALLOC_HEADSIZE
#endif
#ifndef CONFIG_MALLOC_FOOTSIZE
#ifdef CONFIG_DEBUG_HEAP
#   define CONFIG_MALLOC_FOOTSIZE HEAP_ALIGNMENT
#endif
#elif (CONFIG_MALLOC_FOOTSIZE+0) == 0
#   undef  CONFIG_MALLOC_FOOTSIZE
#endif

#ifdef CONFIG_MALLOC_NO_FREQUENCY
#if (CONFIG_MALLOC_NO_FREQUENCY+0) == 0
#   undef CONFIG_MALLOC_NO_FREQUENCY
#endif
#endif
#ifndef CONFIG_MALLOC_FREQUENCY
#if !defined(CONFIG_MALLOC_NO_FREQUENCY) && \
     defined(CONFIG_DEBUG_HEAP)
#   define CONFIG_MALLOC_FREQUENCY 1024
#endif
#elif (CONFIG_MALLOC_FREQUENCY+0) == 0
#   undef CONFIG_MALLOC_FREQUENCY
#endif

/* Enable the debug-API when any functionality that
 * requires additional tracking to-be done was activated. */
#if defined(CONFIG_TRACE_LEAKS) || \
    defined(CONFIG_MALLOC_HEADSIZE) || \
    defined(CONFIG_MALLOC_FOOTSIZE)
#define MALLOC_DEBUG_API
#endif


/* Enable/Disable some of the ~fancy~ strdup-style function in kernel-space:
 *  - strdup
 *  - strndup
 *  - strdupf
 *  - vstrdupf
 *  - memcdup  // Not really part of the group, but never used since `memdup' also exists. */
#undef CONFIG_HAVE_STRDUP_IN_KERNEL
/* #define CONFIG_HAVE_STRDUP_IN_KERNEL 1 */


/* Memory zone used for physical allocations. */
#define MEMORY_PHYS_ZONE          MZONE_ANY

/* Address ranges used by virtual kernel/shared memory allocations. */
#ifdef CONFIG_HIGH_KERNEL
#define KERNEL_VIRT_BEGIN         __UINTPTR_C(0x00000000)  /* 0x00000000...0xbfffffff */
#define KERNEL_VIRT_END          (VM_USER_MAX+1)           /* 0x00000000...0xbfffffff */
#define SHARED_VIRT_BEGIN         VM_HOST_BASE             /* 0xc0000000...0xffffffff */
#if (__SIZEOF_POINTER__ == 4 && VM_HOST_MAX == __UINTPTR_C(0xffffffff)) || \
    (__SIZEOF_POINTER__ == 8 && VM_HOST_MAX == __UINTPTR_C(0xffffffffffffffff))
#define SHARED_VIRT_END           __UINTPTR_C(0x00000000)  /* 0xc0000000...0xffffffff */
#else
#define SHARED_VIRT_END          (VM_HOST_MAX+1)           /* 0xc0000000...0xffffffff */
#endif
#elif defined(CONFIG_LOW_KERNEL)
#define KERNEL_VIRT_BEGIN         VM_USER_BASE             /* 0x40000000...0xffffffff */
#if (__SIZEOF_POINTER__ == 4 && VM_USER_MAX == __UINTPTR_C(0xffffffff)) || \
    (__SIZEOF_POINTER__ == 8 && VM_USER_MAX == __UINTPTR_C(0xffffffffffffffff))
#define KERNEL_VIRT_END           __UINTPTR_C(0x00000000)  /* 0x40000000...0xffffffff */
#else
#define KERNEL_VIRT_END          (VM_USER_MAX+1)           /* 0x40000000...0xffffffff */
#endif
#if VM_HOST_BASE == 0
#define SHARED_VIRT_BEGIN         __UINTPTR_C(0x00000000)  /* 0x00000000...0x3fffffff */
#define SHARED_VIRT_END           VM_HOST_SIZE             /* 0x00000000...0x3fffffff */
#else
#define SHARED_VIRT_BEGIN         VM_HOST_BASE             /* 0x00000000...0x3fffffff */
#define SHARED_VIRT_END          (VM_HOST_MAX+1)           /* 0x00000000...0x3fffffff */
#endif
#endif

#define BAD_ALLOC(n_bytes,flags) (void)0
#define M_ISNONNULL(p) likely((p) != NULL)

#define __GFP_NULL                GFP_NORMAL /* GFP flags for pointers not matching `M_ISNONNULL' */
#if 1 /* Use shared memory for malloc()! */
#define __GFP_MALLOC              GFP_SHARED /* GFP flags for malloc()-allocated memory. */
#else
#define __GFP_MALLOC              GFP_MEMORY /* GFP flags for malloc()-allocated memory. */
#endif


/* Define to non-zero to add syslog entries for managed memory allocation. */
#ifndef LOG_MANAGED_ALLOCATIONS
#define LOG_MANAGED_ALLOCATIONS 0
#endif


#ifdef MALLOC_DEBUG_API
struct PACKED dinfo {
 char const      *i_file;
 char const      *i_func;
 struct instance *i_inst;
 int              i_line;
};
struct PACKED stored_dinfo {
 char const      *i_file;
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
                        *   NOTE: May prematurely terminate upon hitting `MPTR_TAIL_TB_EOF'. */
#endif
};
#endif

struct PACKED mptr {
 /* Trace header */
#ifdef CONFIG_TRACE_LEAKS
#define MPTR_FILE(x) ((x)->m_info.i_file)
#define MPTR_FUNC(x) ((x)->m_info.i_func)
#define MPTR_INST(x) ((x)->m_info.i_inst)
#define MPTR_LINE(x) ((x)->m_line)
 LIST_NODE(struct mptr)
                     m_chain;  /*< [lock(MALLTRACE_LOCK(self))][null(KINSTANCE_TRACE_NULL)]
                                *   Chain of all other MALL headers. */
 struct stored_dinfo m_info;   /*< [const] Basic debug information for tracking. */
 s32                 m_line;   /*< [const] Source line where this pointer was allocated at. */
 u16                 m_chksum; /*< [const] Checksum of m_flag...:m_data + mt_tb. */
 u8                  m_refcnt; /*< [lock(MPTR_TRACE_LOCK(self))] Reference counter (required for handling free() while enumerating). */
#define MPTRFLAG_NONE      0x00
#define MPTRFLAG_UNTRACKED 0x01 /*< The pointer was untracked. */
#define MPTRFLAG_NOFREE    0x02 /*< The pointer must not be freed or reallocated. */
#define MPTRFLAG_GLOBAL    0x04 /*< The pointer is intended for global usage. */
#define MPTRFLAG_MASK      0x07 /*< Mask of recognized flags. */
 u8                  m_flag;   /*< [lock(MALLTRACE_LOCK(self))] Mall flags (Set of `MPTRFLAG_*'). */
#define __1_MPTR_SIZEOF (5*__SIZEOF_POINTER__+8)
#else /* CONFIG_TRACE_LEAKS */
#define __1_MPTR_SIZEOF  0
#endif /* !CONFIG_TRACE_LEAKS */

 /* Tail pointer header */
#ifdef MPTR_HAVE_TAIL
 struct mptr_tail *m_tail; /*< [const][1..1][>= self] Pointer to the end of user-data. */
#define __2_MPTR_SIZEOF (__1_MPTR_SIZEOF+__SIZEOF_POINTER__)
#else
#define __2_MPTR_SIZEOF  __1_MPTR_SIZEOF
#endif

 /* Underflow safe-area header. */
#ifdef CONFIG_MALLOC_HEADSIZE
#define __3_MPTR_SIZEOF (CONFIG_MALLOC_HEADSIZE+__2_MPTR_SIZEOF)
#else
#define __3_MPTR_SIZEOF  __2_MPTR_SIZEOF
#endif

#if (HEAP_ALIGNMENT & GFP_MASK_MPTR) != 0
 /* Use a unique field for the pointer type. */
#define MPTR_HAVE_TYPE
#if GFP_MASK_MPTR > 0xff
#   define __4_MPTR_SIZEOF  (2+__3_MPTR_SIZEOF)
 u16          m_type;
#else
#   define __4_MPTR_SIZEOF  (1+__3_MPTR_SIZEOF)
 u8           m_type;
#endif
#else
#define __4_MPTR_SIZEOF     __3_MPTR_SIZEOF
#endif

#define MPTR_UNALIGNED_SIZE (__4_MPTR_SIZEOF+__SIZEOF_SIZE_T__)
#if (MPTR_UNALIGNED_SIZE % HEAP_ALIGNMENT) != 0
#define MPTR_HAVE_PAD
 byte_t       m_pad[HEAP_ALIGNMENT-(MPTR_UNALIGNED_SIZE % HEAP_ALIGNMENT)];
#endif
#ifdef MPTR_HAVE_TYPE
 size_t       m_size; /*< [const][mask(~GFP_MASK_MPTR)][>= HEAP_MIN_MALLOC] Total memory size (including this header). */
#else
union{ size_t m_size; /*< [const][mask(~GFP_MASK_MPTR)][>= HEAP_MIN_MALLOC] Total memory size (including this header). */
       size_t m_type; /*< [const][mask(GFP_MASK_MPTR)][!= 3] Flags used to create the pointer. */
       size_t m_data; /*< [const] Malloc-pointer information. */
};
#endif
#ifdef CONFIG_MALLOC_HEADSIZE
 byte_t       m_head[CONFIG_MALLOC_HEADSIZE];
#endif
};


#ifdef CONFIG_TRACE_LEAKS
/* Lock that must be held when reading/writing the `m_info.i_inst' field of any mptr. */
PRIVATE DEFINE_ATOMIC_RWLOCK(mptr_inst_lock);
#define MPTR_TRACE_LOCK(self) ((self)->m_info.i_inst->i_driver.k_tlock)
#endif

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
#   define MPTR_MINTAILSIZE          (CONFIG_MALLOC_FOOTSIZE+CONFIG_MALLOC_TRACEBACK_MINSIZE*sizeof(void *))
#   define MPTR_SIZEOF(s)            (sizeof(struct mptr)+(s)+CONFIG_MALLOC_FOOTSIZE+CONFIG_MALLOC_TRACEBACK_MINSIZE*sizeof(void *))
#elif defined(CONFIG_MALLOC_FOOTSIZE)
#   define MPTR_MINTAILSIZE          (CONFIG_MALLOC_FOOTSIZE)
#   define MPTR_SIZEOF(s)            (sizeof(struct mptr)+(s)+CONFIG_MALLOC_FOOTSIZE)
#elif CONFIG_MALLOC_TRACEBACK
#   define MPTR_MINTAILSIZE          (CONFIG_MALLOC_TRACEBACK_MINSIZE*sizeof(void *))
#   define MPTR_SIZEOF(s)            (sizeof(struct mptr)+(s)+CONFIG_MALLOC_TRACEBACK_MINSIZE*sizeof(void *))
#else
#   define MPTR_SIZEOF(s)            (sizeof(struct mptr)+(s))
#endif

#define MPTR_OF(p)         ((struct mptr *)(p)-1)

#ifdef MPTR_HAVE_TYPE
#   define MPTR_FLAGS(p)        (p)->m_type
#   define MPTR_TYPE(p)        ((p)->m_type&GFP_MASK_TYPE)
#   define MPTR_SIZE(p)         (p)->m_size
#   define MPTR_SETUP(self,type,size) (void)((self)->m_type = (type),(self)->m_size = (size))
#else
#   define MPTR_FLAGS(p)       ((p)->m_type&GFP_MASK_MPTR)
#   define MPTR_TYPE(p)        ((p)->m_type&GFP_MASK_TYPE)
#   define MPTR_SIZE(p)        ((p)->m_size&~GFP_MASK_MPTR)
#   define MPTR_SETUP(self,type,size) (void)((self)->m_data = (type)|(size))
#endif
#define MPTR_HEAP(p)           (&mheaps[MPTR_TYPE(p)])

#define MPTR_ISOK(p)     (MPTR_TYPE(p) < __GFP_HEAPCOUNT && MPTR_SIZE(p) >= HEAP_MIN_MALLOC)
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
PRIVATE void KCALL mptr_mvtail(struct mptr *__restrict self,
                               size_t old_size, size_t old_user_size,
                               size_t new_size, size_t new_user_size,
                               gfp_t flags);
#elif defined(CONFIG_TRACE_LEAKS)
#define mptr_mvtail(self,...) (void)(self->m_chksum = mptr_chksum(self))
#else
#define mptr_mvtail(self,...) (void)0
#endif
#ifdef CONFIG_TRACE_LEAKS
PRIVATE u16 KCALL mptr_chksum(struct mptr *__restrict self);
PRIVATE void KCALL mptr_unlink(struct dsetup *setup, struct mptr *__restrict self);
PRIVATE void KCALL mptr_relink(struct dsetup *setup, struct mptr *__restrict self);
#else
#define mptr_unlink(...) (void)0
#define mptr_relink(...) (void)0
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

STATIC_ASSERT(IS_ALIGNED(MPTR_SIZEOF(0),HEAP_ALIGNMENT));
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
#define HEAP_BUCKET_MINSIZE(i)   (1 << ((i)+HEAP_BUCKET_OFFSET-1))
#define HEAP_BUCKET_COUNT       ((__SIZEOF_SIZE_T__*8)-HEAP_BUCKET_OFFSET-1)

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
#if (HEAP_ALIGNMENT & MFREE_ATTRMASK) != 0
#define MFREE_HAVE_ATTR
#undef MFREE_ATTRMASK
#undef MFREE_SIZEMASK
 MALIGNED size_t           mf_size;  /*< [lock(:mh_lock)][mask(MFREE_SIZEMASK)][>= HEAP_MIN_MALLOC]
                                      *   Size of this free range in bytes (Including this header). */
 pgattr_t                  mf_attr;  /*< [lock(:mh_lock)]
                                      *   Page-attributes for data within this free region.
                                      *   NOTE: When `CONFIG_MALLOC_DEBUG_INIT' is defined,
                                      *         and the `PAGEATTR_ZERO' flag isn't set, the
                                      *         memory described within this free range is
                                      *         fully initialized to `CONFIG_MALLOC_DEBUG_INIT'
                                      *        (Excluding this header of course)
                                      *   >> This behavior mirrors that of `PAGEATTR_ZERO'-initialized
                                      *      memory, in that free data is known to be in a specific state.
                                      */
#else
union{
 size_t                    mf_info;
 MALIGNED size_t           mf_size;  /*< [lock(:mh_lock)][mask(MFREE_SIZEMASK)][>= HEAP_MIN_MALLOC]
                                      *   Size of this free range in bytes (Including this header). */
 pgattr_t                  mf_attr;  /*< [lock(:mh_lock)][mask(MFREE_ATTRMASK)]
                                      *   Page-attributes for data within this free region.
                                      *   NOTE: When `CONFIG_MALLOC_DEBUG_INIT' is defined,
                                      *         and the `PAGEATTR_ZERO' flag isn't set, the
                                      *         memory described within this free range is
                                      *         fully initialized to `CONFIG_MALLOC_DEBUG_INIT'
                                      *        (Excluding this header of course)
                                      *   >> This behavior mirrors that of `PAGEATTR_ZERO'-initialized
                                      *      memory, in that free data is known to be in a specific state.
                                      */
};
#endif
};


#define MFREE_MIN(self)         ((uintptr_t)(self))
#define MFREE_BEGIN(self)       ((uintptr_t)(self))
#ifdef MFREE_HAVE_ATTR
#define MFREE_MAX(self)         ((uintptr_t)(self)+(self)->mf_size-1)
#define MFREE_END(self)         ((uintptr_t)(self)+(self)->mf_size)
#define MFREE_SIZE(self)                    (self)->mf_size
#define MFREE_ATTR(self)                    (self)->mf_attr
#define MFREE_SETUP(self,size,pg_attr)    (assert(IS_ALIGNED(size,HEAP_ALIGNMENT)),\
                                           assert((size) >= HEAP_MIN_MALLOC),\
                                          (self)->mf_attr = (pg_attr),\
                                          (self)->mf_size = (size))
#else
#define MFREE_MAX(self)         ((uintptr_t)(self)+((self)->mf_size&MFREE_SIZEMASK)-1)
#define MFREE_END(self)         ((uintptr_t)(self)+((self)->mf_size&MFREE_SIZEMASK))
#define MFREE_SIZE(self)                   ((self)->mf_size&MFREE_SIZEMASK)
#define MFREE_ATTR(self)                   ((self)->mf_attr&MFREE_ATTRMASK)
#define MFREE_SETUP(self,size,pg_attr)    (assert(IS_ALIGNED(size,HEAP_ALIGNMENT)),\
                                           assert((size) >= HEAP_MIN_MALLOC),\
                                           assert(((pg_attr)&MFREE_ATTRMASK) == (pg_attr)),\
                                          (self)->mf_info = (size)|(pg_attr))
#endif


/* TO-DO: Disable me. NOTE: When disabled, replace start with `TO-DO' */
#define MHEAP_USING_INTERNAL_VALIDATION 0

#if MHEAP_USING_INTERNAL_VALIDATION
#define MHEAP_RECURSIVE_LOCK              1
#define MHEAP_INTERNAL_VALIDATE(flags) (((flags)&GFP_NOFREQ) ? (void)0 : _mall_validate(NULL))
#else
#define MHEAP_RECURSIVE_LOCK              0
#define MHEAP_INTERNAL_VALIDATE(flags)   (void)0
#endif


/* Memory heaps must always use recursive locks to prevent deadlocks such as the following:
../src/kernel/sched/task.c(650) : task_yield : [0] : C0169AD3
../include/hybrid/sync/atomic-rwlock.h(138) : atomic_rwlock_write : [0] : C01387DB : EFFFB898
../src/kernel/memory/malloc.c(1612) : mheap_free : [1] : C013DDF5 : EFFFB8A8
../src/kernel/memory/malloc.c(2256) : debug_free : [2] : C014032B : EFFFB8E8
../src/kernel/memory/malloc.c(3087) : _kfree_d : [3] : C01435EC : EFFFB938
../src/kernel/memory/memory.c(608) : mscatter_split_lo : [4] : C0146245 : EFFFB988
../src/kernel/mman/part.c(73) : mregion_part_split_lo : [5] : C0158ABB : EFFFB9C8
../src/kernel/mman/part.c(162) : mregion_part_action : [6] : C0159077 : EFFFB9F8
../src/kernel/mman/part.c(317) : mregion_part_decref : [7] : C0159692 : EFFFBA48
../src/kernel/mman/part.c(389) : mregion_decref : [8] : C01599D2 : EFFFBA78
../src/kernel/mman/mman.c(1473) : mman_munmap_impl : [9] : C0150297 : EFFFBAA8
../src/kernel/mman/mman.c(1532) : mman_munmap_unlocked : [a] : C01505AE : EFFFBAD8
../src/kernel/memory/malloc.c(786) : kernel_munmap : [b] : C013AC07 : EFFFBB18
../src/kernel/memory/malloc.c(970) : core_page_free : [c] : C013B7BC : EFFFBB48
../src/kernel/memory/malloc.c(1448) : mheap_unmapfree : [d] : C013D41F : EFFFBB88
../src/kernel/memory/malloc.c(1383) : mheap_release : [e] : C013D04A : EFFFBBD8
../src/kernel/memory/malloc.c(1613) : mheap_free : [f] : C013DE0B : EFFFBC38
../src/kernel/memory/malloc.c(2256) : debug_free : [10] : C014032B : EFFFBC78
../src/kernel/memory/malloc.c(3087) : _kfree_d : [11] : C01435EC : EFFFBCC8
../src/kernel/mman/mman.c(370) : mman_destroy : [12] : C014CA94 : EFFFBD18
../src/kernel/sched/task.c(633) : task_destroy : [13] : C0169819 : EFFFBD58
../src/kernel/sched/task.c(1982) : __task_destroy2 : [14] : C016D98F : EFFFBD88
../src/kernel/sched/task.c(2073) : task_terminate_self_unlock_cpu : [15] : C016DDBD : EFFFBDA8
../src/kernel/sched/task-util.c(594) : sig_vtimedrecv_endwrite : [16] : C01661DE : EFFFBE40
../src/kernel/sched/task-util.c(575) : sig_timedrecv_endwrite : [17] : C0166147 : EFFFBE70
../src/kernel/fs/iobuffer.c(222) : iobuffer_read : [18] : C0127D87 : EFFFBE90
../src/kernel/fs/pty.c(409) : master_read : [19] : C012CAD9 : EFFFBF20
../src/kernel/fs/file.c(148) : file_read : [1a] : C011DD7C : EFFFBF50
../src/kernel/fs/fd.c(815) : SYSC_read : [1b] : C0119577 : EFFFBF80
../syscall.c() : sys_ccall : [1c] : C01010D9 : EFFFBFC0
../??(0) : ?? : [1d] : 0000001B : EFFFBFDC

>> Basically: `free()' might call `munmap()', which could call another `free()'
               of the same magnitude when the kernel memory manager inherited
               a region, or a region part allocated as `MMAN_UNIGFP', or for
               another manager, meaning that free()-ing that branch as the
               result of unmapping unused memory will in turn call free() again.
*/
#undef MHEAP_RECURSIVE_LOCK
#define MHEAP_RECURSIVE_LOCK        1


struct mheap {
#if MHEAP_RECURSIVE_LOCK
#define __MHEAP_LOCK_INIT ATOMIC_OWNER_RWLOCK_INIT
 atomic_owner_rwlock_t    mh_lock;      /*< Lock for this heap. */
#else
#define __MHEAP_LOCK_INIT ATOMIC_RWLOCK_INIT
 atomic_rwlock_t          mh_lock;      /*< Lock for this heap. */
#endif
 ATREE_HEAD(struct mfree) mh_addr;      /*< [lock(mh_lock)][0..1|null(PAGE_ERROR)] Heap sorted by address. */
 LIST_HEAD(struct mfree)  mh_size[HEAP_BUCKET_COUNT];
                                        /*< [lock(mh_lock)][0..1|null(PAGE_ERROR)][*] Heap sorted by free range size. */
 size_t                   mh_overalloc; /*< [lock(mh_lock)] Amount (in bytes) by which to over-allocate memory in heaps.
                                         *   NOTE: Set to ZERO(0) to disable overallocation. */
 size_t                   mh_freethresh;/*< [lock(mh_lock)] Threshold that must be reached before any continuous block
                                         *   of free data is split to free memory. (Should always be `>= PAGESIZE') */
};

#if MHEAP_RECURSIVE_LOCK
#define mheap_reading(x)     atomic_owner_rwlock_reading(&(x)->mh_lock)
#define mheap_writing(x)     atomic_owner_rwlock_writing(&(x)->mh_lock)
#define mheap_tryread(x)     atomic_owner_rwlock_tryread(&(x)->mh_lock)
#define mheap_trywrite(x)    atomic_owner_rwlock_trywrite(&(x)->mh_lock)
#define mheap_tryupgrade(x)  atomic_owner_rwlock_tryupgrade(&(x)->mh_lock)
#define mheap_read(x)        atomic_owner_rwlock_read(&(x)->mh_lock)
#define mheap_write(x)       atomic_owner_rwlock_write(&(x)->mh_lock)
#define mheap_upgrade(x)     atomic_owner_rwlock_upgrade(&(x)->mh_lock)
#define mheap_downgrade(x)   atomic_owner_rwlock_downgrade(&(x)->mh_lock)
#define mheap_endread(x)     atomic_owner_rwlock_endread(&(x)->mh_lock)
#define mheap_endwrite(x)    atomic_owner_rwlock_endwrite(&(x)->mh_lock)
#else
#define mheap_reading(x)     atomic_rwlock_reading(&(x)->mh_lock)
#define mheap_writing(x)     atomic_rwlock_writing(&(x)->mh_lock)
#define mheap_tryread(x)     atomic_rwlock_tryread(&(x)->mh_lock)
#define mheap_trywrite(x)    atomic_rwlock_trywrite(&(x)->mh_lock)
#define mheap_tryupgrade(x)  atomic_rwlock_tryupgrade(&(x)->mh_lock)
#define mheap_read(x)        atomic_rwlock_read(&(x)->mh_lock)
#define mheap_write(x)       atomic_rwlock_write(&(x)->mh_lock)
#define mheap_upgrade(x)     atomic_rwlock_upgrade(&(x)->mh_lock)
#define mheap_downgrade(x)   atomic_rwlock_downgrade(&(x)->mh_lock)
#define mheap_endread(x)     atomic_rwlock_endread(&(x)->mh_lock)
#define mheap_endwrite(x)    atomic_rwlock_endwrite(&(x)->mh_lock)
#endif

#ifdef MALLOC_DEBUG_API
PRIVATE SAFE MALIGNED bool KCALL mheap_isfree_l(struct mheap *__restrict self, void *p);
#endif /* MALLOC_DEBUG_API */
PRIVATE SAFE MALIGNED void *KCALL mheap_acquire(struct mheap *__restrict self, MALIGNED size_t n_bytes, MALIGNED size_t *__restrict alloc_bytes, gfp_t flags, bool unlock_heap);
PRIVATE SAFE MALIGNED void *KCALL mheap_acquire_al(struct mheap *__restrict self, MALIGNED size_t alignment, size_t offset, MALIGNED size_t n_bytes, MALIGNED size_t *__restrict alloc_bytes, gfp_t flags, bool unlock_heap);
PRIVATE SAFE MALIGNED void *KCALL mheap_acquire_at(struct mheap *__restrict self, MALIGNED void *p, MALIGNED size_t n_bytes, MALIGNED size_t *__restrict alloc_bytes, gfp_t flags);
PRIVATE SAFE bool KCALL mheap_release(struct mheap *__restrict self, MALIGNED void *p, MALIGNED size_t n_bytes, gfp_t flags, bool unlock_heap);
PRIVATE SAFE void KCALL mheap_release_nomerge(struct mheap *__restrict self, MALIGNED void *p, MALIGNED size_t n_bytes, gfp_t flags);
PRIVATE SAFE void KCALL mheap_unmapfree(struct mheap *__restrict self, struct mfree **__restrict pslot, ATREE_SEMI_T(uintptr_t) addr_semi, ATREE_LEVEL_T addr_level, gfp_t flags, bool unlock_heap);
#ifdef CONFIG_DEBUG_HEAP
PRIVATE SAFE void KCALL mheap_validate(struct dsetup *setup, struct mheap *__restrict self);
#endif
#if defined(CONFIG_DEBUG_HEAP) || defined(MALLOC_DEBUG_API)
PRIVATE SAFE ATTR_NORETURN void KCALL malloc_panic(struct dsetup *setup, struct mptr *info_header, char const *__restrict format, ...);
#endif

PRIVATE SAFE struct mptr *KCALL mheap_malloc(struct mheap *__restrict self, size_t size, gfp_t flags);
PRIVATE SAFE struct mptr *KCALL mheap_memalign(struct mheap *__restrict self, size_t alignment, size_t size, gfp_t flags);
PRIVATE SAFE void         KCALL mheap_free(struct mheap *__restrict self, struct mptr *ptr, gfp_t flags);
#ifdef CONFIG_TRACE_LEAKS
PRIVATE SAFE struct mptr *KCALL mheap_realloc(struct dsetup *setup, struct mheap *__restrict self, struct mptr *ptr, size_t new_size, gfp_t flags);
PRIVATE SAFE struct mptr *KCALL mheap_realign(struct dsetup *setup, struct mheap *__restrict self, struct mptr *ptr, size_t alignment, size_t new_size, gfp_t flags);
#else
PRIVATE SAFE struct mptr *KCALL mheap_realloc(struct mheap *__restrict self, struct mptr *ptr, size_t new_size, gfp_t flags);
PRIVATE SAFE struct mptr *KCALL mheap_realign(struct mheap *__restrict self, struct mptr *ptr, size_t alignment, size_t new_size, gfp_t flags);
#endif

#if __SIZEOF_POINTER__ == 8
#   define MEMSETX   memsetq
#   define MEMCPYX   memcpyq
#   define MEMPATX   mempatq
#elif __SIZEOF_POINTER__ == 4
#   define MEMSETX   memsetl
#   define MEMCPYX   memcpyl
#   define MEMPATX   mempatl
#elif __SIZEOF_POINTER__ == 2
#   define MEMSETX   memsetw
#   define MEMCPYX   memcpyw
#   define MEMPATX   mempatw
#elif __SIZEOF_POINTER__ == 1
#   define MEMSETX   memsetb
#   define MEMCPYX   memcpyb
#   define MEMPATX   mempatb
#else
#   error "Unsupported sizeof(void *)"
#endif


#define MHEAP_INIT(name) \
{ \
    .mh_lock = __MHEAP_LOCK_INIT, \
    .mh_addr = PAGE_ERROR, \
    .mh_size = { \
        [0 ... HEAP_BUCKET_COUNT-1] = PAGE_ERROR, \
    }, \
    .mh_overalloc  = HEAP_DEFAULT_OVERALLOC(name), \
    .mh_freethresh = HEAP_DEFAULT_FREETHRESH(name), \
}

/* The different memory heaps used by the kernel. */
PRIVATE struct mheap mheaps[__GFP_HEAPCOUNT] = {
    [GFP_SHARED]            = MHEAP_INIT(GFP_SHARED),
    [GFP_SHARED|GFP_LOCKED] = MHEAP_INIT(GFP_SHARED|GFP_LOCKED),
    [GFP_KERNEL]            = MHEAP_INIT(GFP_KERNEL),
    [GFP_KERNEL|GFP_LOCKED] = MHEAP_INIT(GFP_KERNEL|GFP_LOCKED),
    [GFP_MEMORY]            = MHEAP_INIT(GFP_MEMORY),
#ifdef MZONE_FAST
    /* TODO: Make use the permanent mapping of 0-2Gib at -2Gib.
     *    >> Allocating physical memory from `MZONE_FAST'
     *       doesn't require memory to be re-mapped! */
#endif
};

#define MHEAP_GET(flags)  \
  (assertf(((flags)&GFP_MASK_TYPE) < __GFP_HEAPCOUNT,\
             "Invalid heap id #%d",(int)((flags)&GFP_MASK_TYPE)),\
    &mheaps[(flags)&GFP_MASK_TYPE])

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
LOCAL SAFE PAGE_ALIGNED void *KCALL core_page_alloc(size_t n_bytes, gfp_t flags);
LOCAL SAFE PAGE_ALIGNED void *KCALL core_page_allocat(PAGE_ALIGNED void *start, size_t n_bytes, gfp_t flags);
LOCAL SAFE void KCALL core_page_free(PAGE_ALIGNED void *ptr, size_t n_bytes, gfp_t flags);

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
 region = mregion_new(GFP_NOFREQ|GFP_MEMORY);
 if unlikely(!_mall_untrack(region)) return PAGE_ERROR;
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
  region->mr_setup.mri_byte = CONFIG_MALLOC_DEBUG_INIT;
#endif
 }
#endif
#if GFP_LOCKED == 1
 region->mr_part0.mt_locked = flags&GFP_LOCKED;
#else
 region->mr_part0.mt_locked = !!(flags&GFP_LOCKED);
#endif
 if (flags&GFP_INCORE) {
  /* Preallocate core memory for the region. */
  if (!page_malloc_scatter(&region->mr_part0.mt_memory,n_bytes,
                            PAGESIZE,GFP_GTPAGEATTR(flags),
                            MZONE_ANY,GFP_MEMORY)) {
   kfree(region);
   goto swapmem;
  }
  assert(region->mr_part0.mt_memory.m_size <= region->mr_size);
#ifdef CONFIG_MALLOC_DEBUG_INIT
  if (!(flags&GFP_CALLOC)) {
   struct mscatter *iter = &region->mr_part0.mt_memory;
   while (iter) {
    MEMPATX(iter->m_start,
            CONFIG_MALLOC_DEBUG_INIT,
            iter->m_size);
    iter = iter->m_next;
   }
  }
#endif
  region->mr_part0.mt_state = MPART_STATE_INCORE;
 }
 mregion_setup(region);
 error = mman_mmap_unlocked(&mman_kernel,(ppage_t)start,n_bytes,0,region,
                             PROT_READ|PROT_WRITE|PROT_NOUSER,NULL,NULL);
 MREGION_DECREF(region);
 if (E_ISERR(error)) {
swapmem:
  /* Try to swap out memory. */
  if (MMAN_SWAPOK(mman_swapmem(n_bytes,flags))) goto again;
  BAD_ALLOC(n_bytes,flags);
  return PAGE_ERROR;
 }
 MHEAP_INTERNAL_VALIDATE(flags);
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
 mman_munmap_unlocked(&mman_kernel,(ppage_t)start,n_bytes,
#if  PAGEATTR_ZERO == MMAN_MUNMAP_CLEAR
                      MMAN_MUNMAP_ALL|(flags&PAGEATTR_ZERO),
#else
                      MMAN_MUNMAP_ALL|(flags&PAGEATTR_ZERO ? MMAN_MUNMAP_CLEAR : 0),
#endif
                      NULL);
}


LOCAL SAFE PAGE_ALIGNED void *KCALL
core_page_alloc(size_t n_bytes, gfp_t flags) {
 void *result;
 if (flags&GFP_MEMORY) {
  do result = page_malloc(n_bytes,GFP_GTPAGEATTR(flags),MEMORY_PHYS_ZONE);
  while (result == PAGE_ERROR && MMAN_SWAPOK(mman_swapmem(n_bytes,flags)));
#ifdef CONFIG_MALLOC_DEBUG_INIT
  if (result != PAGE_ERROR && !(flags&GFP_CALLOC))
      MEMPATX(result,CONFIG_MALLOC_DEBUG_INIT,n_bytes);
#endif
 } else {
  PHYS struct mman *old_mman; bool has_write_lock = false;
#define HEAPEND_MASK  (GFP_SHARED|GFP_KERNEL|GFP_LOCKED)
  /* Make sure that default heap addresses are in valid ranges. */
  PRIVATE VIRT uintptr_t heap_end[HEAPEND_MASK+1] = {
      [GFP_SHARED]            = HOST_HEAPEND_SHARED,        /* 0xd4000000 / 0xffffd10000000000 */
      [GFP_SHARED|GFP_LOCKED] = HOST_HEAPEND_SHARED_LOCKED, /* 0xd0000000 / 0xffffd00000000000 */
      [GFP_KERNEL]            = HOST_HEAPEND_KERNEL,        /* 0x14000000 / 0x0000510000000000 */
      [GFP_KERNEL|GFP_LOCKED] = HOST_HEAPEND_KERNEL_LOCKED, /* 0x10000000 / 0x0000500000000000 */
  };
  task_nointr();
  if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_BEGIN(old_mman);
  assert(PDIR_ISKPD());
  /* Only need a read-lock to search for free space. */
  if (!(flags&GFP_ATOMIC)) mman_read(&mman_kernel);
  else if (mman_tryread(&mman_kernel) == -EAGAIN) {
   result = PAGE_ERROR;
   goto end;
  }
check_again:
  result = mman_findspace_unlocked(&mman_kernel,
                                  (ppage_t)heap_end[flags&HEAPEND_MASK],n_bytes,PAGESIZE,
                                   0,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_PRIVATE);
  /* Make sure that we're not allocating out-of-bounds. */
  if unlikely(flags&GFP_KERNEL) {
   if (result == PAGE_ERROR) {
    /* Try to re-scan the heap for a free gap. */
    result = mman_findspace_unlocked(&mman_kernel,(ppage_t)(flags&GFP_LOCKED ? HOST_HEAPEND_KERNEL_LOCKED
                                                                             : HOST_HEAPEND_KERNEL),
                                     n_bytes,PAGESIZE,0,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_PRIVATE);
    if unlikely(result == PAGE_ERROR) {
     /* Try to scan an extended address range. */
#if HOST_HEAPEND_KERNEL_LOCKED < HOST_HEAPEND_KERNEL
     if (!(flags&GFP_LOCKED))
           result = mman_findspace_unlocked(&mman_kernel,(ppage_t)HOST_HEAPEND_KERNEL_LOCKED,n_bytes,
                                             PAGESIZE,0,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_PRIVATE);
#elif HOST_HEAPEND_KERNEL < HOST_HEAPEND_KERNEL_LOCKED
     if (flags&GFP_LOCKED)
         result = mman_findspace_unlocked(&mman_kernel,(ppage_t)HOST_HEAPEND_KERNEL,n_bytes,
                                           PAGESIZE,0,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_PRIVATE);
#endif
     /* Try to scan the entire virtual address range. */
     if unlikely(result == PAGE_ERROR)
        result = mman_findspace_unlocked(&mman_kernel,(ppage_t)(KERNEL_VIRT_BEGIN),n_bytes,
                                          PAGESIZE,0,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_PRIVATE);
    }
    if unlikely(result == PAGE_ERROR) goto end2;
   }
  } else {
   /* Make sure not to allocate dynamic memory above what is reserved for error-codes.
    * >> Since the reserve is _always_ less than a page, it's impossible
    *    to encounter this problem in other allocators such as `page_malloc()'.
    *    But since `kmalloc()'s purpose is to get away from page-aligned
    *    allocations, we may actually run into the problem of allocating
    *    memory above `__ERRNO_THRESHOLD', which in turn may be interpreted
    *    incorrectly once allocated pointers are transformed, or passed
    *    through various different interfaces.
    * >> So to prevent any of those problems, simply don't allow virtual
    *    address mapping of the last virtual address page, thereby preventing
    *    any collisions that might otherwise arise. */
#if defined(CONFIG_NO_PDIR_SELFMAP) || (__ERRNO_THRESHOLD < THIS_PDIR_BASE)
#define PAGE_ISOK(x) ((x) != PAGE_ERROR && (uintptr_t)(x)+(n_bytes) < FLOOR_ALIGN(__ERRNO_THRESHOLD,PAGESIZE))
#else
#define PAGE_ISOK(x) ((x) != PAGE_ERROR)
#endif
   if unlikely(!PAGE_ISOK(result)) {
    /* Try to stay inside the designated region of memory as long as possible. */
    result = mman_findspace_unlocked(&mman_kernel,(ppage_t)(flags&GFP_LOCKED ? HOST_HEAPEND_SHARED_LOCKED
                                                                             : HOST_HEAPEND_SHARED),
                                     n_bytes,PAGESIZE,0,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_PRIVATE);
    if unlikely(!PAGE_ISOK(result)) {
#if HOST_HEAPEND_SHARED_LOCKED < HOST_HEAPEND_SHARED
     if (!(flags&GFP_LOCKED))
           result = mman_findspace_unlocked(&mman_kernel,(ppage_t)HOST_HEAPEND_SHARED_LOCKED,n_bytes,
                                             PAGESIZE,0,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_PRIVATE);
#elif HOST_HEAPEND_SHARED < HOST_HEAPEND_SHARED_LOCKED
     if (flags&GFP_LOCKED)
         result = mman_findspace_unlocked(&mman_kernel,(ppage_t)HOST_HEAPEND_SHARED,n_bytes,
                                           PAGESIZE,0,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_PRIVATE);
#endif
     /* re-scan the entire shared address space as a last resort. */
     if unlikely(!PAGE_ISOK(result))
        result = mman_findspace_unlocked(&mman_kernel,(ppage_t)SHARED_VIRT_BEGIN,n_bytes,
                                          PAGESIZE,0,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_PRIVATE);
    }
#if defined(CONFIG_NO_PDIR_SELFMAP) || (__ERRNO_THRESHOLD < THIS_PDIR_BASE)
#define NEED_END2_ERR
    if unlikely(result == PAGE_ERROR ||
               (uintptr_t)result >= FLOOR_ALIGN(__ERRNO_THRESHOLD,PAGESIZE))
                goto end2_err;
#else
    if unlikely(result == PAGE_ERROR)
                goto end2;
#endif
   }
#undef PAGE_ISOK
  }
  assert(result != PAGE_ERROR);

  /* Upgrade to a write-lock before mapping the space we've discovered. */
  if (!has_write_lock) {
   has_write_lock = true;
   if (!(flags&GFP_ATOMIC)) {
    if (mman_upgrade(&mman_kernel) == -ERELOAD)
        goto check_again;
   } else if (mman_tryupgrade(&mman_kernel) == -EAGAIN) {
    result = PAGE_ERROR;
    goto end_read;
   }
  }

  result = kernel_mmap_anon(result,n_bytes,flags);
  if (result == PAGE_ERROR) goto end2;
  assert(((uintptr_t)result+n_bytes) <= FLOOR_ALIGN(__ERRNO_THRESHOLD,PAGESIZE));
  heap_end[flags&HEAPEND_MASK] = (uintptr_t)result+n_bytes;
end2: if (has_write_lock) mman_endwrite(&mman_kernel);
      else end_read:      mman_endread(&mman_kernel);
end:  if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_END(old_mman);
  task_endnointr();
 }
 if (result == PAGE_ERROR) BAD_ALLOC(n_bytes,flags);
 return result;
#ifdef NEED_END2_ERR
#undef NEED_END2_ERR
end2_err: result = PAGE_ERROR; goto end2;
#endif
}


LOCAL PAGE_ALIGNED void *KCALL
core_page_allocat(PAGE_ALIGNED void *start,
                  size_t n_bytes, gfp_t flags) {
 void *result;
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 if (flags&GFP_MEMORY) {
  result = page_malloc_at((ppage_t)start,n_bytes,GFP_GTPAGEATTR(flags));
#ifdef CONFIG_MALLOC_DEBUG_INIT
  if (result != PAGE_ERROR && !(flags&GFP_CALLOC))
      MEMPATX(result,CONFIG_MALLOC_DEBUG_INIT,PAGESIZE);
#endif
 } else {
  bool has_write_lock = false;
  PHYS struct mman *old_mman;
  assert(n_bytes != 0);
  assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
  assert(IS_ALIGNED(n_bytes,PAGESIZE));
  if unlikely((uintptr_t)start+n_bytes < (uintptr_t)start) return PAGE_ERROR;
  if (flags&GFP_KERNEL) {
   if unlikely(!addr_ishost_r(start,n_bytes)) return PAGE_ERROR;
  } else {
   if unlikely(!addr_isuser_r(start,n_bytes)) return PAGE_ERROR;
  }
  task_nointr();
  if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_BEGIN(old_mman);
  if (!(flags&GFP_ATOMIC)) mman_read(&mman_kernel);
  else if (mman_tryread(&mman_kernel) == -EAGAIN) {
   result = PAGE_ERROR;
   goto end;
  }
check_again:
  if unlikely(mman_inuse_unlocked(&mman_kernel,(ppage_t)start,n_bytes)) { result = PAGE_ERROR; goto end2; }
  /* Upgrade to a write-lock before mapping the space. */
  if (!has_write_lock) {
   has_write_lock = true;
   if (!(flags&GFP_ATOMIC)) {
    if (mman_upgrade(&mman_kernel) == -ERELOAD) goto check_again;
   } else if (mman_tryupgrade(&mman_kernel) == -EAGAIN) {
    result = PAGE_ERROR;
    goto end_read;
   }
  }
  result = kernel_mmap_anon(start,n_bytes,flags);
end2:
  if (has_write_lock) mman_endwrite(&mman_kernel);
  else end_read:      mman_endread(&mman_kernel);
end: if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_END(old_mman);
  task_endnointr();
 }
 return result;
}


LOCAL void KCALL
core_page_free(PAGE_ALIGNED void *ptr,
               size_t n_bytes, gfp_t flags) {
 assert(IS_ALIGNED((uintptr_t)ptr,PAGESIZE));
 assert((uintptr_t)ptr+CEIL_ALIGN(n_bytes,PAGESIZE) >= (uintptr_t)ptr);
 if (flags&GFP_MEMORY) {
  page_ffree((ppage_t)ptr,n_bytes,GFP_GTPAGEATTR(flags));
 } else {
  PHYS struct mman *old_mman;
  assert((flags&GFP_KERNEL)
         ? addr_isuser_r(ptr,CEIL_ALIGN(n_bytes,PAGESIZE))
         : addr_ishost_r(ptr,CEIL_ALIGN(n_bytes,PAGESIZE)));
  task_nointr();
  if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_BEGIN(old_mman);
  mman_write(&mman_kernel);
  kernel_munmap(ptr,n_bytes,flags);
  mman_endwrite(&mman_kernel);
  if (!(flags&GFP_KERNEL)) TASK_PDIR_KERNEL_END(old_mman);
  task_endnointr();
 }
}




















/* Kernel heap implementation. */
#ifdef MALLOC_DEBUG_API
PRIVATE MALIGNED bool KCALL
mheap_isfree_l(struct mheap *__restrict self, void *p) {
 struct mfree *free_slot;
 CHECK_HOST_DOBJ(self);
#if !MHEAP_RECURSIVE_LOCK
 if (!mheap_tryread(self)) return false;
#else
 mheap_read(self);
#endif
 free_slot = mfree_tree_locate(self->mh_addr,(uintptr_t)p);
 mheap_endread(self);
 return free_slot != PAGE_ERROR;
}
#endif /* MALLOC_DEBUG_API */

PRIVATE MALIGNED void *KCALL
mheap_acquire(struct mheap *__restrict self, MALIGNED size_t n_bytes,
              MALIGNED size_t *__restrict alloc_bytes, gfp_t flags,
              bool unlock_heap) {
 MALIGNED void *result = PAGE_ERROR;
 MALIGNED struct mfree **iter,**end;
 MALIGNED struct mfree *chain;
 size_t page_bytes;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(alloc_bytes);
 assert(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
 assert(mheap_writing(self));
 if unlikely(n_bytes < HEAP_MIN_MALLOC)
             n_bytes = HEAP_MIN_MALLOC;
 iter = &self->mh_size[HEAP_BUCKET_OF(n_bytes)];
 end  =  COMPILER_ENDOF(self->mh_size);
 MHEAP_INTERNAL_VALIDATE(flags);
 for (; iter != end; ++iter) {
  chain = *iter,assert(chain);
  while (chain != PAGE_ERROR && MFREE_SIZE(chain) < n_bytes)
         chain = chain->mf_lsize.le_next,assert(chain);
  if (chain != PAGE_ERROR) {
   size_t unused_size;
   pgattr_t chain_attr = MFREE_ATTR(chain);
   result = (void *)chain;
#ifdef CONFIG_DEBUG_HEAP
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
   unused_size = MFREE_SIZE(chain)-n_bytes;
   if (unused_size < HEAP_MIN_MALLOC) {
    /* Remainder is too small. - Allocate it as well. */
    n_bytes += unused_size;
   } else {
    MALIGNED void *unused_begin = (void *)((uintptr_t)chain+n_bytes);
    /* Make sure the the remainder is properly aligned. */
    assert(IS_ALIGNED((uintptr_t)unused_begin,HEAP_ALIGNMENT));
    assert(IS_ALIGNED(unused_size,HEAP_ALIGNMENT));
    assert(unused_size < MFREE_SIZE(chain));
    mheap_release_nomerge(self,unused_begin,unused_size,
                         (flags&~(GFP_CALLOC))|
                          GFP_STPAGEATTR(chain_attr));
   }
   /* Initialize the result memory. */
   if (flags&GFP_CALLOC) {
    if (chain_attr&PAGEATTR_ZERO)
         memset(result,0,MIN(sizeof(struct mfree),n_bytes));
    else memset(result,0,n_bytes);
   }
#ifdef CONFIG_MALLOC_DEBUG_INIT
   else if (chain_attr&PAGEATTR_ZERO) {
    MEMPATX(result,CONFIG_MALLOC_DEBUG_INIT,n_bytes);
   }
#endif
   *alloc_bytes = n_bytes;
   assert(IS_ALIGNED((uintptr_t)result,HEAP_ALIGNMENT));
   goto done;
  }
 }
 MHEAP_INTERNAL_VALIDATE(flags);
 /* Allocate whole pages. */
 page_bytes  = CEIL_ALIGN(n_bytes,PAGESIZE);
 page_bytes += self->mh_overalloc; /* Overallocate a bit. */
core_again:
 result = core_page_alloc(page_bytes,flags);
 if (result == PAGE_ERROR) {
  if (page_bytes == CEIL_ALIGN(n_bytes,PAGESIZE)) goto done;
  /* Try again without overallocation. */
  page_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
  goto core_again;
 }
 if (page_bytes != n_bytes) {
  /* Release all unused memory. */
  if (!mheap_release(self,(void *)((uintptr_t)result+n_bytes),
                     page_bytes-n_bytes,flags,unlock_heap))
       n_bytes = page_bytes;
  unlock_heap = false;
 }
 *alloc_bytes = n_bytes;
done:
 MHEAP_INTERNAL_VALIDATE(flags);
#if LOG_MANAGED_ALLOCATIONS
 if (result != PAGE_ERROR) {
  syslog(LOG_MEM|LOG_ERROR,"[MEM] ALLOC(%p...%p) (%Iu/%Iu)\n",
         result,(uintptr_t)result+*alloc_bytes-1,*alloc_bytes,n_bytes);
 }
#endif
 if (unlock_heap)
     mheap_endwrite(self);
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
 assert(mheap_writing(self));
 if unlikely(!n_bytes) return p;
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 /* Search for a free slot at the given address. */
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)p,
                               &addr_semi,&addr_level);
 if unlikely(pslot == PAGE_ERROR) {
  /* Easy enough: the slot doesn't exist, so allocate memory here. */
  PAGE_ALIGNED void *ptr_page = (void *)FLOOR_ALIGN((uintptr_t)p,PAGESIZE);
  PAGE_ALIGNED size_t ptr_size = CEIL_ALIGN(((uintptr_t)p-(uintptr_t)ptr_page)+n_bytes,PAGESIZE);
  assertf(IS_ALIGNED(ptr_size,HEAP_ALIGNMENT),"ptr_size = %Iu",ptr_size);
  slot = (struct mfree *)core_page_allocat(ptr_page,ptr_size,flags);
  if unlikely(slot == PAGE_ERROR) return PAGE_ERROR;
  assert(IS_ALIGNED(MFREE_BEGIN(slot),HEAP_ALIGNMENT));
  slot_avail = ptr_size-((uintptr_t)p-(uintptr_t)slot);
  assert(IS_ALIGNED(slot_avail,HEAP_ALIGNMENT));
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
  else MEMPATX(slot,CONFIG_MALLOC_DEBUG_INIT,sizeof(struct mfree));
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
                          slot_flags,false));
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
 /* At this point we've allocated `slot_avail' bytes at `p'
  * >> Now we must simply try to free as much of the difference as possible. */
 assertf(IS_ALIGNED(slot_avail,HEAP_ALIGNMENT),"slot_avail = %Iu\n",slot_avail);
 assert(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
 assert(slot_avail >= n_bytes);
 {
  size_t unused_size = slot_avail-n_bytes;
  /* Try to release high memory. */
  if (unused_size && mheap_release(self,
                                  (void *)((uintptr_t)p+n_bytes),
                                   unused_size,slot_flags,false))
      slot_avail -= unused_size;
 }
 /* Do final initialization of memory. */
 if (flags&GFP_CALLOC) {
  if (!(slot_flags&GFP_CALLOC))
        memset(p,0,slot_avail);
 }
#ifdef CONFIG_MALLOC_DEBUG_INIT
 else if (slot_flags&GFP_CALLOC) {
  MEMPATX(p,CONFIG_MALLOC_DEBUG_INIT,slot_avail);
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
 assert(mheap_writing(self));
 MFREE_SETUP(NEW_SLOT,n_bytes,GFP_GTPAGEATTR(flags));
 assert(MFREE_SIZE(NEW_SLOT) >= HEAP_MIN_MALLOC);
 mfree_tree_insert(&self->mh_addr,NEW_SLOT);
 /* Figure out where the free-slot should go in the chain of free ranges. */
 piter = &self->mh_size[HEAP_BUCKET_OF(n_bytes)];
 while ((iter = *piter) != PAGE_ERROR &&
         MFREE_SIZE(iter) < n_bytes)
         piter = &iter->mf_lsize.le_next;
//  syslog(LOG_DEBUG,"HERE: %p...%p (%x)\n",p,(uintptr_t)p+n_bytes-1,flags);
 NEW_SLOT->mf_lsize.le_pself = piter;
 NEW_SLOT->mf_lsize.le_next  = iter;
 if (iter != PAGE_ERROR) iter->mf_lsize.le_pself = &NEW_SLOT->mf_lsize.le_next;
 *piter = NEW_SLOT;
 assert(NEW_SLOT->mf_lsize.le_next);
 MHEAP_INTERNAL_VALIDATE(flags);
#undef NEW_SLOT
}
PRIVATE bool KCALL
mheap_release(struct mheap *__restrict self, MALIGNED void *p,
              MALIGNED size_t n_bytes, gfp_t flags,
              bool unlock_heap) {
 struct mfree **pslot,*slot,*iter,*next;
 struct mfree *free_slot;
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level;
 CHECK_HOST_DATA(p,n_bytes);
 assert(mheap_writing(self));
 assert(IS_ALIGNED((uintptr_t)p,HEAP_ALIGNMENT));
 assert(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
 if unlikely(!n_bytes) {
  if (unlock_heap) mheap_endwrite(self);
  return true;
 }
#ifndef MFREE_HAVE_ATTR
 assert(!(GFP_GTPAGEATTR(flags)&MFREE_SIZEMASK));
#endif
#if LOG_MANAGED_ALLOCATIONS
 syslog(LOG_MEM|LOG_ERROR,"[MEM] FREE(%p...%p)\n",
        p,(uintptr_t)p+n_bytes-1);
#endif
 MHEAP_INTERNAL_VALIDATE(flags);

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
      MEMPATX(slot+1,CONFIG_MALLOC_DEBUG_INIT,
              MFREE_SIZE(slot)-sizeof(struct mfree));
  if (flags&GFP_CALLOC && !(slot->mf_attr&PAGEATTR_ZERO))
      MEMPATX(p,CONFIG_MALLOC_DEBUG_INIT,n_bytes);
#endif
  if (!(slot->mf_attr&PAGEATTR_ZERO)) flags &= ~(GFP_CALLOC);
#ifdef MFREE_HAVE_ATTR
  slot->mf_attr  = GFP_GTPAGEATTR(flags);
  slot->mf_size += n_bytes;
#else
  MFREE_SETUP(slot,MFREE_SIZE(slot)+n_bytes,GFP_GTPAGEATTR(flags));
#endif
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
   LIST_INSERT_AFTER_EX(iter,slot,mf_lsize,PAGE_ERROR);
  }
  mheap_unmapfree(self,pslot,addr_semi,addr_level,flags,unlock_heap);
  MHEAP_INTERNAL_VALIDATE(flags);
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
  *free_slot = *slot;
#ifdef CONFIG_MALLOC_DEBUG_INIT
  if (free_slot->mf_attr&PAGEATTR_ZERO && !(flags&GFP_CALLOC))
      MEMPATX(slot+1,CONFIG_MALLOC_DEBUG_INIT,
              MFREE_SIZE(slot)-sizeof(struct mfree));
  if (flags&GFP_CALLOC && !(free_slot->mf_attr&PAGEATTR_ZERO))
      MEMPATX(free_slot+1,CONFIG_MALLOC_DEBUG_INIT,n_bytes);
#endif
  if (!(free_slot->mf_attr&PAGEATTR_ZERO)) flags &= ~(GFP_CALLOC);
#ifdef MFREE_HAVE_ATTR
  free_slot->mf_attr  = GFP_GTPAGEATTR(flags);
  free_slot->mf_size += n_bytes;
#else
  MFREE_SETUP(free_slot,MFREE_SIZE(free_slot)+n_bytes,GFP_GTPAGEATTR(flags));
#endif
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
        MEMPATX((void *)((uintptr_t)slot+(sizeof(struct mfree)-n_bytes)),
                 CONFIG_MALLOC_DEBUG_INIT,n_bytes);
   else MEMPATX(slot,CONFIG_MALLOC_DEBUG_INIT,sizeof(struct mfree));
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
   LIST_INSERT_AFTER_EX(iter,free_slot,mf_lsize,PAGE_ERROR);
  }
  mheap_unmapfree(self,pslot,addr_semi,
                  addr_level,flags,unlock_heap);
  MHEAP_INTERNAL_VALIDATE(flags);
  return true;
 }
 MHEAP_INTERNAL_VALIDATE(flags);
 /* Make sure the heap part wouldn't shrink too small. */
 if (n_bytes < HEAP_MIN_MALLOC) goto too_small;
 mheap_release_nomerge(self,p,n_bytes,flags);
 if (unlock_heap) mheap_endwrite(self);
 MHEAP_INTERNAL_VALIDATE(flags);
 return true;
too_small:
 if (unlock_heap) mheap_endwrite(self);
 return false;
}

PRIVATE void KCALL
mheap_unmapfree(struct mheap *__restrict self,
                struct mfree **__restrict pslot,
                ATREE_SEMI_T(uintptr_t) addr_semi,
                ATREE_LEVEL_T addr_level, gfp_t flags,
                bool unlock_heap) {
 struct mfree *slot;
 assert(mheap_writing(self));
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
    if (page_begin == page_end) goto end;
   }
   if (hi_size && hi_size < HEAP_MIN_MALLOC) {
    hi_size += PAGESIZE;
    *(uintptr_t *)&page_end -= PAGESIZE;
    *(uintptr_t *)&hi_slot -= PAGESIZE;
    if (page_begin == page_end) goto end;
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

   if (unlock_heap) mheap_endwrite(self);

   /* Release heap data back to the system. */
   core_page_free(page_begin,
                 (PAGE_ALIGNED uintptr_t)page_end-
                 (PAGE_ALIGNED uintptr_t)page_begin,
                  page_flags);
   return;
  }
 }
end:
 if (unlock_heap)
     mheap_endwrite(self);
}
PRIVATE MALIGNED void *KCALL
mheap_acquire_al(struct mheap *__restrict self,
                 MALIGNED size_t alignment, size_t offset, MALIGNED size_t n_bytes,
                 MALIGNED size_t *__restrict alloc_bytes, gfp_t flags, bool unlock_heap) {
 void *alloc_base; void *result;
 size_t alloc_size,nouse_size;
 assert(alignment != 0);
 assert((alignment&(alignment-1)) == 0);
 assert(IS_ALIGNED(alignment,HEAP_ALIGNMENT));
 assert(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
 if unlikely(n_bytes < HEAP_MIN_MALLOC)
             n_bytes = HEAP_MIN_MALLOC;
 /* Must overallocate by at least `HEAP_MIN_MALLOC',
  * so we can _always_ free unused lower memory. */
 alloc_base = mheap_acquire(self,n_bytes+alignment+HEAP_MIN_MALLOC,&alloc_size,flags,false);
 if unlikely(alloc_base == PAGE_ERROR) {
  if (unlock_heap) mheap_endwrite(self);
  return PAGE_ERROR;
 }
 assert(alloc_size >= n_bytes+alignment+HEAP_MIN_MALLOC);
 result = (void *)(CEIL_ALIGN((uintptr_t)alloc_base+HEAP_MIN_MALLOC+offset,alignment)-offset);
 assert((uintptr_t)result+n_bytes <=
        (uintptr_t)alloc_base+alloc_size);
 nouse_size = (uintptr_t)result-(uintptr_t)alloc_base;
 assert(nouse_size+n_bytes <= alloc_size);
 assertf(nouse_size >= HEAP_MIN_MALLOC,"nouse_size = %Iu",nouse_size);
 /* Release lower memory. */
 asserte(mheap_release(self,alloc_base,nouse_size,flags,false));
 alloc_size -= nouse_size;

 /* Try to release upper memory. */
 assert(alloc_size >= n_bytes);
 nouse_size = alloc_size-n_bytes;
 if (mheap_release(self,(void *)((uintptr_t)result+n_bytes),nouse_size,flags,unlock_heap))
     alloc_size -= nouse_size;
 assert(alloc_size >= n_bytes);
 *alloc_bytes = alloc_size;
 assert(IS_ALIGNED((uintptr_t)result+offset,alignment));
 return result;
}

#ifdef CONFIG_MALLOC_DEBUG_INIT
PRIVATE void KCALL
priv_resetpage(PAGE_ALIGNED void *start, uintptr_t xword, size_t n_bytes) {
 bool should_clear;
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(n_bytes != 0);
 assert(n_bytes <= PAGESIZE);
 assert((n_bytes%4) == 0);
 /* Check if the page at `start' is really allocated. */
 if (addr_isglob(start)) {
  task_nointr();
  mman_read(&mman_kernel);
  should_clear = thispdir_test_writable(&pdir_kernel_v,start);
  mman_endread(&mman_kernel);
  task_endnointr();
 } else {
  should_clear = true;
 }
 /* Only reset the page when it has been allocated. */
 if (should_clear) MEMSETX(start,xword,n_bytes/__SIZEOF_POINTER__);
}

/* Scan `n_bytes' of memory for any byte not matching `xword & 0xff',
 * assuming that `xword == 0x01010101 * (xword & 0xff)'
 * Return NULL when no such byte exists, or the non-matching byte if so.
 * NOTE: Special handling is done to ensure that no new memory after any non-aligned
 *       memory between `begin...CEIL_ALIGN(begin,PAGESIZE)' is allocated due to
 *       access, meaning that system memory isn't strained by accessing unallocated
 *       data, but instead assuming that that data is always equal to `xword'. */
PRIVATE void KCALL
priv_memsetx_noalloc(void *__restrict begin, uintptr_t xword, size_t n_bytes) {
 byte_t *iter,*end,*aligned;
 size_t fill_bytes;
 end = (iter = (byte_t *)begin)+n_bytes;
 assert(iter <= end);
 while (iter != end && (uintptr_t)iter&(__SIZEOF_POINTER__-1))
       *iter = ((u8 *)&xword)[(uintptr_t)iter&(__SIZEOF_POINTER__-1)];
 assert(((end-iter) % __SIZEOF_POINTER__) == 0);
 aligned    = (byte_t *)CEIL_ALIGN((uintptr_t)iter,PAGESIZE);
 fill_bytes = (size_t)(aligned-iter);
 n_bytes    = (size_t)(end-iter);
 if (fill_bytes > n_bytes)
     fill_bytes = n_bytes;
 if (fill_bytes) {
  /* Set unaligned data in the first (allocated) page. */
  MEMSETX(iter,xword,fill_bytes/__SIZEOF_POINTER__);
  n_bytes -= fill_bytes;
 }
 /* TODO: Tell the memory manager to unload all full pages,
  *       so that they will be re-allocated next time. */
 while (n_bytes) {
  fill_bytes = n_bytes;
  if (fill_bytes > PAGESIZE)
      fill_bytes = PAGESIZE;
  /* Set unaligned data in other (potentially unallocated) pages. */
  priv_resetpage(aligned,xword,fill_bytes);
  n_bytes -= fill_bytes;
  aligned += fill_bytes;
 }
}

/* Reset debug pre-initialization of the given memory. */
PRIVATE void KCALL mheap_resetdebug(MALIGNED void *__restrict begin,
                                    MALIGNED size_t n_bytes, gfp_t flags) {
 if (flags&GFP_CALLOC) return;
 priv_memsetx_noalloc(begin,CONFIG_MALLOC_DEBUG_INIT,n_bytes);
}
#else
#define mheap_resetdebug(begin,n_bytes,flags) (void)0
#endif

PRIVATE struct mptr *KCALL
mheap_malloc(struct mheap *__restrict self, size_t size, gfp_t flags) {
 struct mptr *result;
 MALIGNED size_t alloc_size;
 mheap_write(self);
 result = (struct mptr *)mheap_acquire(self,CEIL_ALIGN(MPTR_SIZEOF(size),HEAP_ALIGNMENT),
                                      &alloc_size,flags,true);
 /*mheap_endwrite(self);*/
 if unlikely(result == PAGE_ERROR) return MPTR_OF(NULL);
 assert(alloc_size >= size);
 MPTR_SETUP(result,flags&GFP_MASK_MPTR,alloc_size);
 MPTR_VALIDATE(result);
 return result;
}
PRIVATE struct mptr *KCALL
mheap_memalign(struct mheap *__restrict self,
               size_t alignment, size_t size,
               gfp_t flags) {
 struct mptr *result;
 MALIGNED size_t alloc_size;
 mheap_write(self);
 result = (struct mptr *)mheap_acquire_al(self,alignment,sizeof(struct mptr),
                                          CEIL_ALIGN(MPTR_SIZEOF(size),HEAP_ALIGNMENT),
                                         &alloc_size,flags,true);
 /*mheap_endwrite(self);*/
 if (result == PAGE_ERROR) return MPTR_OF(NULL);
 assert(IS_ALIGNED((uintptr_t)result+sizeof(struct mptr),alignment));
 MPTR_SETUP(result,flags&GFP_MASK_MPTR,alloc_size);
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
 mheap_write(self);
 asserte(mheap_release(self,ptr,alloc_size,flags,true));
 /*mheap_endwrite(self);*/
}

#ifndef __INTELLISENSE__
DECL_END
#define HEAP_REALIGN
#include "malloc-realign.c.inl"
#include "malloc-realign.c.inl"
DECL_BEGIN
#endif

#ifdef CONFIG_DEBUG_HEAP
PRIVATE void KCALL
mheap_validate_addr(struct dsetup *setup, struct mfree *node) {
 if (node == PAGE_ERROR) return;
 if (!OK_HOST_DATA(node,sizeof(struct mfree)))
      malloc_panic(setup,NULL,"Invalid heap address-space space node %p",node);
 if (!IS_ALIGNED(MFREE_BEGIN(node),HEAP_ALIGNMENT) ||
     !IS_ALIGNED(MFREE_END(node),HEAP_ALIGNMENT))
      malloc_panic(setup,NULL,"Miss-aligned heap address-space node %p...%p (size %p)\n%$[hex]",
                   MFREE_MIN(node),MFREE_MAX(node),MFREE_SIZE(node),sizeof(struct mfree),node);
 if (MFREE_BEGIN(node) >= MFREE_END(node))
     malloc_panic(setup,NULL,"Unordered heap address-space node %p...%p\n%$[hex]",
                  MFREE_MIN(node),MFREE_MAX(node),sizeof(struct mfree),node);
 if (MFREE_SIZE(node) < HEAP_MIN_MALLOC)
     malloc_panic(setup,NULL,"Heap address-space node %p...%p is too small (%Iu < %Iu)\n%$[hex]",
                  MFREE_MIN(node),MFREE_MAX(node),MFREE_SIZE(node),HEAP_MIN_MALLOC,sizeof(struct mfree),node);
 mheap_validate_addr(setup,node->mf_laddr.a_min);
 mheap_validate_addr(setup,node->mf_laddr.a_max);
}

PRIVATE void *KCALL
priv_scandata(void *start, uintptr_t xword, size_t n_bytes) {
#if defined(__i386__) || defined(__x86_64__)
 byte_t *iter,*end; bool is_ok;
 assert(n_bytes);
 end = (iter = (byte_t *)start)+n_bytes;
 __asm__ __volatile__(
#if __SIZEOF_POINTER__ >= 8
                      "repe; scasq\n"
#else
                      "repe; scasl\n"
#endif
                      "sete %1\n"
                      : "+D" (iter)
                      , "=g" (is_ok)
                      : "a" (xword)
                      , "c" ((end-iter)/__SIZEOF_POINTER__)
                      , "m" (*(struct { __extension__ byte_t val[n_bytes]; } *)start)
                      : "cc");
 if (!is_ok) {
  iter -= __SIZEOF_POINTER__;
  assert(*(uintptr_t *)iter != xword);
  while ((assert(iter < end),
         *iter == ((u8 *)&xword)[(uintptr_t)iter & (__SIZEOF_POINTER__-1)]))
         ++iter;
  return iter;
 }
 return NULL;
#else
 uintptr_t *iter,*end;
 while ((uintptr_t)start & (__SIZEOF_POINTER__-1)) {
  if (!n_bytes) return NULL;
  if unlikely(*(byte_t *)start != ((byte_t *)&xword)[(uintptr_t)start & (__SIZEOF_POINTER__-1)])
     return start;
  --n_bytes;
  ++*(byte_t **)&start;
 }
 end = (iter = (uintptr_t *)start)+(n_bytes/__SIZEOF_POINTER__);
 n_bytes %= __SIZEOF_POINTER__;
 for (; iter != end; ++iter) {
  if likely(*iter == xword) continue;
  while ((assert(iter < end),
         *iter == ((u8 *)&xword)[(uintptr_t)iter & (__SIZEOF_POINTER__-1)]))
         ++iter;
  return iter;
 }
 while (n_bytes) {
  if unlikely(*(byte_t *)iter != ((byte_t *)&xword)[(uintptr_t)iter & (__SIZEOF_POINTER__-1)])
     return iter;
  --n_bytes;
  ++*(byte_t **)&iter;
 }
 return NULL;
#endif
}

PRIVATE void *KCALL
priv_scanpage(PAGE_ALIGNED void *start, uintptr_t xword, size_t n_bytes) {
 void *result = NULL; bool should_scan;
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(n_bytes != 0);
 assert(n_bytes <= PAGESIZE);
 /* Check if the page at `start' is really allocated. */
 task_nointr();
 mman_read(&mman_kernel);
 should_scan = thispdir_test_writable(&pdir_kernel_v,start);
 mman_endread(&mman_kernel);
 task_endnointr();
 /* Only scan the page when it has been allocated. */
 if (should_scan) result = priv_scandata(start,xword,n_bytes);
 return result;
}

/* Scan `n_bytes' of memory for any byte not matching `xword'.
 * Return NULL when no such byte exists, or the non-matching byte if so.
 * NOTE: Special handling is done to ensure that no new memory after any non-aligned
 *       memory between `begin...CEIL_ALIGN(begin,PAGESIZE)' is allocated due to
 *       access, meaning that system memory isn't strained by accessing unallocated
 *       data, but instead assuming that that data is always equal to `xword'. */
PRIVATE void *KCALL
priv_memnchr_noalloc(void *__restrict begin, uintptr_t xword, size_t n_bytes) {
 byte_t *iter,*end,*aligned; void *result;
 size_t scan_bytes;
 end = (iter = (byte_t *)begin)+n_bytes;
 //syslog(LOG_MEM|LOG_ERROR,"CHECK: %p...%p\n",iter,end-1);
 assert(iter <= end);
 while (iter != end && (uintptr_t)iter&(__SIZEOF_POINTER__-1)) {
  if (*iter != ((u8 *)&xword)[(uintptr_t)iter&(__SIZEOF_POINTER__-1)]) return iter;
  ++iter;
 }
 assert(((end-iter) % __SIZEOF_POINTER__) == 0);
 aligned    = (byte_t *)CEIL_ALIGN((uintptr_t)iter,PAGESIZE);
 scan_bytes = (size_t)(aligned-iter);
 n_bytes    = (size_t)(end-iter);
 if (scan_bytes > n_bytes)
     scan_bytes = n_bytes;
 if (scan_bytes) {
  /* Scan unaligned data in the first (allocated) page. */
  result = priv_scandata(iter,xword,scan_bytes);
  if (result) return result;
  n_bytes -= scan_bytes;
 }
 while (n_bytes) {
  //syslog(LOG_DEBUG,"SCAN_PAGE: %p\n",aligned);
  scan_bytes = n_bytes;
  if (scan_bytes > PAGESIZE)
      scan_bytes = PAGESIZE;
  /* Scan unaligned data in other (potentially unallocated) pages. */
  result = priv_scanpage(aligned,xword,scan_bytes);
  if (result) return result;
  n_bytes -= scan_bytes;
  aligned += scan_bytes;
 }
 return NULL;
}

PRIVATE void KCALL
mheap_validate(struct dsetup *setup,
               struct mheap *__restrict self) {
 struct mfree **iter,**end,**pnode,*node;
#if !MHEAP_RECURSIVE_LOCK
 if (!mheap_tryread(self)) return;
#else
 mheap_read(self);
#endif
 mheap_validate_addr(setup,self->mh_addr);
 end = (iter = self->mh_size)+COMPILER_LENOF(self->mh_size);
 for (; iter != end; ++iter) {
  size_t min_size = HEAP_BUCKET_MINSIZE(iter-self->mh_size);
  pnode = iter;
  while ((node = *pnode) != PAGE_ERROR) {
   if (!OK_HOST_DATA(node,sizeof(struct mfree)))
        malloc_panic(setup,NULL,"Invalid heap size node %p",node);
   if (!IS_ALIGNED(MFREE_BEGIN(node),HEAP_ALIGNMENT) ||
       !IS_ALIGNED(MFREE_END(node),HEAP_ALIGNMENT))
        malloc_panic(setup,NULL,"Miss-aligned heap sized node %p...%p\n%$[hex]",
                     MFREE_MIN(node),MFREE_MAX(node),sizeof(struct mfree),node);
   if (MFREE_BEGIN(node) >= MFREE_END(node))
       malloc_panic(setup,NULL,"Unordered heap sized node %p...%p\n%$[hex]",
                    MFREE_MIN(node),MFREE_MAX(node),sizeof(struct mfree),node);
   if (MFREE_SIZE(node) < min_size)
       malloc_panic(setup,NULL,"Heap sized node %p...%p is too small (%Iu < %Iu)\n%$[hex]",
                    MFREE_MIN(node),MFREE_MAX(node),MFREE_SIZE(node),
                    min_size,sizeof(struct mfree),node);
   if (node->mf_lsize.le_pself != pnode)
       malloc_panic(setup,NULL,"Heap sized node %p...%p is incorrectly linked (%p != %p)\n%$[hex]",
                    MFREE_MIN(node),MFREE_MAX(node),node->mf_lsize.le_pself,
                    pnode,sizeof(struct mfree),node);
   pnode = &node->mf_lsize.le_next;
   if (*pnode != PAGE_ERROR && OK_HOST_DATA(*pnode,sizeof(struct mfree)) &&
        MFREE_SIZE(*pnode) < MFREE_SIZE(node))
       malloc_panic(setup,NULL,"Heap sized node %p...%p is incorrectly ordered (%Iu is larger than next node %p...%p with %Iu bytes)\n%$[hex]",
                    MFREE_MIN(node),MFREE_MAX(node),MFREE_SIZE(node),
                    MFREE_MIN(*pnode),MFREE_MAX(*pnode),MFREE_SIZE(*pnode),
                    sizeof(struct mfree),node);
#ifndef CONFIG_MALLOC_DEBUG_INIT
   if (MFREE_ATTR(node)&PAGEATTR_ZERO)
#endif
   {
    /* Make sure that node data is really zero/debug-initialized. */
    byte_t *begin,*error; size_t n_bytes;
#ifdef CONFIG_MALLOC_DEBUG_INIT
    uintptr_t init_xword = CONFIG_MALLOC_DEBUG_INIT;
    if (MFREE_ATTR(node)&PAGEATTR_ZERO) init_xword = 0;
#else
#define init_xword 0
#endif
#define HEX_OFFSET   32
#define HEX_SIZE    (16+HEX_OFFSET*2)
    begin   = (byte_t *)MFREE_BEGIN(node);
    n_bytes = MFREE_SIZE(node);
    assert(n_bytes >= sizeof(struct mfree));
    begin   += sizeof(struct mfree);
    n_bytes -= sizeof(struct mfree);
    if (n_bytes &&
       (error = (byte_t *)priv_memnchr_noalloc(begin,init_xword,n_bytes)) != NULL) {
     size_t hex_size; byte_t *hex_begin;
     hex_begin = error-HEX_OFFSET;
     if (hex_begin < begin)
         hex_begin = begin;
     hex_size  = (size_t)((begin+n_bytes)-hex_begin);
     if (hex_size > HEX_SIZE) hex_size = HEX_SIZE;
     malloc_panic(setup,NULL,
                  "Memory at %p modified after it was freed isn't %#.2I8x\n"
                  "> Offset %Id bytes into heap free range %p...%p\n"
                  "> Offset %Id bytes into heap data range %p...%p\n"
                  "%$[hex]",
                  error,((u8 *)&init_xword)[(uintptr_t)error & (__SIZEOF_POINTER__-1)],
                 (uintptr_t)error-MFREE_BEGIN(node),MFREE_MIN(node),MFREE_MAX(node),
                 (uintptr_t)error-(uintptr_t)begin,(uintptr_t)begin,MFREE_MAX(node),
                  hex_size,hex_begin);
    }
#undef init_xword
   }
  }
 }
 mheap_endread(self);
}
#endif

/* TODO: Use `hptr_t' internally! */

/* Public kernel heap API. */
PUBLIC SAFE hptr_t KCALL
heap_malloc(size_t n_bytes, gfp_t flags) {
 void *result; size_t heap_size = 0;
 struct mheap *heap = MHEAP_GET(flags);
 n_bytes = CEIL_ALIGN(n_bytes,HEAP_ALIGNMENT);
 mheap_write(heap);
 result = mheap_acquire(heap,n_bytes,&heap_size,flags,true);
 return HPTR(result,heap_size);
}
PUBLIC SAFE hptr_t KCALL
heap_malloc_at(void *ptr, size_t n_bytes, gfp_t flags) {
 void *result; size_t heap_size = 0;
 struct mheap *heap = MHEAP_GET(flags);
 n_bytes = CEIL_ALIGN(n_bytes,HEAP_ALIGNMENT);
 mheap_write(heap);
 result = mheap_acquire_at(heap,ptr,n_bytes,&heap_size,flags);
 mheap_endwrite(heap);
 return HPTR(result,heap_size);
}
PUBLIC SAFE hptr_t KCALL
heap_memalign(size_t alignment, size_t offset,
              size_t n_bytes, gfp_t flags) {
 void *result; size_t heap_size = 0;
 struct mheap *heap = MHEAP_GET(flags);
 if (alignment < HEAP_ALIGNMENT)
      alignment = HEAP_ALIGNMENT;
 else alignment = CEIL_ALIGN(alignment,HEAP_ALIGNMENT);
 n_bytes = CEIL_ALIGN(n_bytes,HEAP_ALIGNMENT);
 mheap_write(heap);
 result = mheap_acquire_al(heap,alignment,offset,n_bytes,
                           &heap_size,flags,true);
 return HPTR(result,heap_size);
}

/* Free heap memory previously allocated using `heap_*' functions. */
PUBLIC SAFE bool KCALL heap_ffree(hptr_t ptr, gfp_t flags) {
 struct mheap *heap = MHEAP_GET(flags);
 assert(IS_ALIGNED(HPTR_SIZE(ptr),HEAP_ALIGNMENT));
 assert(!HPTR_SIZE(ptr) || IS_ALIGNED((uintptr_t)HPTR_ADDR(ptr),HEAP_ALIGNMENT));
 /* Revert the contents of the given memory range to their defaults. */
 mheap_resetdebug(HPTR_ADDR(ptr),HPTR_SIZE(ptr),flags);
 mheap_write(heap);
 return mheap_release(heap,HPTR_ADDR(ptr),HPTR_SIZE(ptr),flags,true);
}


#if defined(CONFIG_DEBUG_HEAP) || defined(MALLOC_DEBUG_API)
PRIVATE ATTR_NORETURN void KCALL
malloc_panic(struct dsetup *setup,
             struct mptr *info_header,
             char const *__restrict format, ...) {
 va_list args;
 PREEMPTION_DISABLE();
 debug_printf("\n\n");
 va_start(args,format);
 debug_vprintf(format,args);
 debug_printf("\n");
 va_end(args);
 if (setup) {
  debug_printf("%s(%d) : %s : [%s] : See reference to caller location\n",
               setup->s_info.i_file,setup->s_info.i_line,
               setup->s_info.i_func,
               setup->s_info.i_inst->i_module->m_name->dn_name);
  debug_tbprint2(setup->s_tbebp,0);
 }
 if (info_header) {
  bool ok_module;
#ifdef CONFIG_TRACE_LEAKS
  atomic_rwlock_read(&mptr_inst_lock);
  ok_module = (OK_HOST_TEXT(MPTR_INST(info_header),sizeof(struct instance)) &&
               OK_HOST_TEXT(MPTR_INST(info_header)->i_module,sizeof(struct module)) &&
               OK_HOST_TEXT(MPTR_INST(info_header)->i_module->m_name,sizeof(struct dentryname)) &&
               OK_HOST_TEXT(MPTR_INST(info_header)->i_module->m_name->dn_name,
                            MPTR_INST(info_header)->i_module->m_name->dn_size));
  debug_printf("%s(%d) : %s : [%.?s] : See reference to associated allocation\n",
               OK_HOST_TEXT(MPTR_FILE(info_header),1) ? MPTR_FILE(info_header) : NULL,
               MPTR_LINE(info_header),
               OK_HOST_TEXT(MPTR_FUNC(info_header),1) ? MPTR_FUNC(info_header) : NULL,
               ok_module ? MPTR_INST(info_header)->i_module->m_name->dn_size : 0,
               ok_module ? MPTR_INST(info_header)->i_module->m_name->dn_name : NULL);
  atomic_rwlock_endread(&mptr_inst_lock);
#endif
#ifdef CONFIG_MALLOC_TRACEBACK
  {
   void **iter,**end; size_t pos;
   end = (iter = MPTR_TRACEBACK_ADDR(info_header))+
                 MPTR_TRACEBACK_SIZE(info_header);
   pos = 0;
   while (iter != end) {
    if (*iter == MPTR_TAIL_TB_EOF) break;
#ifdef CONFIG_USE_EXTERNAL_ADDR2LINE
    debug_printf("#!$ addr2line(%p) '{file}({line}) : {func} : [%Ix] : %p'\n",
                (uintptr_t)*iter-1,pos,*iter);
#else
    debug_printf("%[vinfo] : [%Ix] : %p\n",
                (uintptr_t)*iter-1,pos,*iter);
#endif
    ++iter,++pos;
   }
  }
#endif
 }

 debug_printf("\n");
#if 1
 __NAMESPACE_INT_SYM
 __afail("MALL PANIC",__DEBUGINFO_NUL);
#else
 PREEMPTION_FREEZE();
#endif
}
#endif



PRIVATE int (KCALL core_mallopt)(int parameter_number,
                                 int parameter_value,
                                 gfp_t flags) {
 struct mheap *heap = MHEAP_GET(flags);
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

PUBLIC SAFE size_t (KCALL kmalloc_trim)(size_t max_free) {
 size_t result = 0;
 /* TODO: Go through each of the kernel heaps and
  *       release any full pages kept for buffering. */
 return result;
}



/* General purpose memory API. */
#define MPTR_KMALLOC(size,flags)          mheap_malloc(MHEAP_GET(flags),size,flags)
#define MPTR_KMEMALIGN(alignment,size,flags) \
 (unlikely(alignment <= HEAP_ALIGNMENT) ? MPTR_KMALLOC(size,flags) : \
  mheap_memalign(MHEAP_GET(flags),alignment,size,flags))
#define MPTR_KFREE(ptr)                   mheap_free(MPTR_HEAP(ptr),ptr,MPTR_FLAGS(ptr))
#define MPTR_KZFREE(ptr)                  mheap_free(MPTR_HEAP(ptr),ptr,MPTR_FLAGS(ptr)|GFP_CALLOC)
#define MPTR_KFFREE(ptr,flags)            mheap_free(MPTR_HEAP(ptr),ptr,MPTR_FLAGS(ptr)|((flags)&~GFP_MASK_MPTR))
#define MPTR_KMALLOC_USABLE_SIZE(ptr)     MPTR_USERSIZE(ptr)
#define MPTR_KMALLOC_FLAGS(ptr)           MPTR_FLAGS(ptr)
#ifdef CONFIG_TRACE_LEAKS
#define MPTR_KREALLOC(ptr,new_size,flags) mheap_realloc(setup,MPTR_HEAP(ptr),ptr,new_size,MPTR_FLAGS(ptr)|((flags)&~GFP_MASK_MPTR))
#define MPTR_KREALIGN(ptr,alignment,new_size,flags) \
 (likely(alignment > HEAP_ALIGNMENT) \
  ? mheap_realign(setup,MPTR_HEAP(ptr),ptr,alignment,new_size,MPTR_FLAGS(ptr)|((flags)&~GFP_MASK_MPTR))\
  : mheap_realloc(setup,MPTR_HEAP(ptr),ptr,new_size,MPTR_FLAGS(ptr)|((flags)&~GFP_MASK_MPTR)))
#else
#define MPTR_KREALLOC(ptr,new_size,flags) mheap_realloc(MPTR_HEAP(ptr),ptr,new_size,MPTR_FLAGS(ptr)|((flags)&~GFP_MASK_MPTR))
#define MPTR_KREALIGN(ptr,alignment,new_size,flags) \
 (likely(alignment > HEAP_ALIGNMENT) \
  ? mheap_realign(MPTR_HEAP(ptr),ptr,alignment,new_size,MPTR_FLAGS(ptr)|((flags)&~GFP_MASK_MPTR))\
  : mheap_realloc(MPTR_HEAP(ptr),ptr,new_size,MPTR_FLAGS(ptr)|((flags)&~GFP_MASK_MPTR)))
#endif

#define KMALLOC(size,flags)                    MPTR_USERADDR(MPTR_KMALLOC(size,flags))
#define KMEMALIGN(alignment,size,flags)        MPTR_USERADDR(MPTR_KMEMALIGN(alignment,size,flags))
#define KFREE(ptr)                            (M_ISNONNULL(ptr) ? MPTR_KFREE(MPTR_GET(ptr)) : (void)0)
#define KFFREE(ptr,flags)                     (M_ISNONNULL(ptr) ? MPTR_KFFREE(MPTR_GET(ptr),flags) : (void)0)
#define KMALLOC_USABLE_SIZE(ptr)              (M_ISNONNULL(ptr) ? MPTR_KMALLOC_USABLE_SIZE(MPTR_GET(ptr)) : 0)
#define KMALLOC_FLAGS(ptr)                    (M_ISNONNULL(ptr) ? MPTR_KMALLOC_FLAGS(MPTR_GET(ptr)) : __GFP_NULL)
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
PUBLIC void   (KCALL kffree)(void *ptr, gfp_t flags) { KFFREE(ptr,flags); }
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
PUBLIC void   (KCALL _kffree_d)(void *ptr, gfp_t flags, DEBUGINFO_UNUSED) { KFFREE(ptr,flags); }
PUBLIC size_t (KCALL _kmalloc_usable_size_d)(void *ptr, DEBUGINFO_UNUSED) { return KMALLOC_USABLE_SIZE(ptr); }
PUBLIC gfp_t  (KCALL _kmalloc_flags_d)(void *ptr, DEBUGINFO_UNUSED) { return KMALLOC_FLAGS(ptr); }
PUBLIC int    (KCALL _kmallopt_d)(int parameter_number, int parameter_value, gfp_t flags, DEBUGINFO_UNUSED) { return core_mallopt(parameter_number,parameter_value,flags); }


#ifndef __ptbwalker_defined
#define __ptbwalker_defined 1
typedef __SSIZE_TYPE__ (__LIBCCALL *__ptbwalker)(void const *__restrict __instruction_pointer,
                                                 void const *__restrict __frame_address,
                                                 size_t __frame_index, void *__closure);
#endif

/* MALL Api stubs. */
PUBLIC void   *(KCALL _mall_getattrib)(void *__restrict UNUSED(mallptr), int UNUSED(attrib)) { return NULL; }
PUBLIC ssize_t (KCALL _mall_traceback)(void *__restrict UNUSED(mallptr), __ptbwalker UNUSED(callback), void *UNUSED(closure)) { return 0; }
PUBLIC ssize_t (KCALL _mall_enum)(struct instance *UNUSED(inst), void *UNUSED(checkpoint), ssize_t (*callback)(void *__restrict mallptr, void *closure), void *UNUSED(closure)) { (void)callback; return 0; }
PUBLIC void    (KCALL _mall_printleaks)(struct instance *UNUSED(inst)) {}
PUBLIC void    (KCALL _mall_validate)(struct instance *UNUSED(inst)) {}
PUBLIC void   *(KCALL _mall_untrack)(void *mallptr) { return mallptr; }
PUBLIC void   *(KCALL _mall_nofree)(void *mallptr) { return mallptr; }
PUBLIC void   *(KCALL _mall_getattrib_d)(void *__restrict UNUSED(mallptr), int UNUSED(attrib), DEBUGINFO_UNUSED) { return NULL; }
PUBLIC ssize_t (KCALL _mall_traceback_d)(void *__restrict UNUSED(mallptr), __ptbwalker UNUSED(callback), void *UNUSED(closure), DEBUGINFO_UNUSED) { return 0; }
PUBLIC ssize_t (KCALL _mall_enum_d)(struct instance *UNUSED(inst), void *UNUSED(checkpoint), ssize_t (*callback)(void *__restrict mallptr, void *closure), void *UNUSED(closure), DEBUGINFO_UNUSED) { (void)callback; return 0; }
PUBLIC void    (KCALL _mall_printleaks_d)(struct instance *UNUSED(inst), DEBUGINFO_UNUSED) {}
PUBLIC void    (KCALL _mall_validate_d)(struct instance *UNUSED(inst), DEBUGINFO_UNUSED) {}
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
PUBLIC void *(KCALL _memdup_d)(void const *__restrict ptr, size_t n_bytes, DEBUGINFO_UNUSED) { return (memdup)(ptr,n_bytes); }

PUBLIC void *(KCALL memdup)(void const *__restrict ptr, size_t n_bytes) {
 void *result = KMALLOC(n_bytes,__GFP_MALLOC);
 if (result) memcpy(result,ptr,n_bytes);
 return result;
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

#ifdef CONFIG_HAVE_STRDUP_IN_KERNEL
PUBLIC char *(KCALL _strdup_d)(char const *__restrict str, DEBUGINFO_UNUSED) { return (strdup)(str); }
PUBLIC char *(KCALL _strndup_d)(char const *__restrict str, size_t max_chars, DEBUGINFO_UNUSED) { return (strndup)(str,max_chars); }
PUBLIC char *(ATTR_CDECL _strdupf_d)(DEBUGINFO_UNUSED, char const *__restrict format, ...) { char *result; va_list args; va_start(args,format); result = (vstrdupf)(format,args); va_end(args); return result; }
PUBLIC char *(KCALL _vstrdupf_d)(char const *__restrict format, va_list args, DEBUGINFO_UNUSED) { return (vstrdupf)(format,args); }
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
PUBLIC void *(KCALL memcdup)(void const *__restrict ptr,
                             int needle, size_t n_bytes) {
 if (n_bytes) {
  void const *endaddr = memchr(ptr,needle,n_bytes-1);
  if (endaddr) n_bytes = ((uintptr_t)endaddr-(uintptr_t)ptr)+1;
 }
 return memdup(ptr,n_bytes);
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
PUBLIC char *(ATTR_CDECL strdupf)(char const *__restrict format, ...) {
 char *result;
 va_list args;
 va_start(args,format);
 result = vstrdupf(format,args);
 va_end(args);
 return result;
}
#endif /* CONFIG_HAVE_STRDUP_IN_KERNEL */
#else /* !MALLOC_DEBUG_API */

PRIVATE void   *(KCALL debug_malloc)(struct dsetup *__restrict setup, size_t size, gfp_t flags);
PRIVATE void   *(KCALL debug_realloc)(struct dsetup *__restrict setup, void *ptr, size_t size, gfp_t flags);
PRIVATE void   *(KCALL debug_memalign)(struct dsetup *__restrict setup, size_t alignment, size_t size, gfp_t flags);
PRIVATE void   *(KCALL debug_realign)(struct dsetup *__restrict setup, void *ptr, size_t alignment, size_t size, gfp_t flags);
PRIVATE void    (KCALL debug_free)(struct dsetup *__restrict setup, void *ptr, gfp_t flags);
PRIVATE size_t  (KCALL debug_malloc_usable_size)(struct dsetup *__restrict setup, void *ptr);
PRIVATE gfp_t   (KCALL debug_malloc_flags)(struct dsetup *__restrict setup, void *ptr);
PRIVATE void   *(KCALL debug_memdup)(struct dsetup *__restrict setup, void const *__restrict ptr, size_t size, gfp_t flags);
PRIVATE int     (KCALL debug_mallopt)(struct dsetup *__restrict setup, int parameter_number, int parameter_value, gfp_t flags);
PRIVATE int     (KCALL debug_posix_memalign)(struct dsetup *__restrict setup, void **__restrict pp, size_t alignment, size_t n_bytes, gfp_t flags);
PRIVATE void   *(KCALL debug_getattrib)(struct dsetup *__restrict setup, void *mallptr, int attrib);
PRIVATE ssize_t (KCALL debug_traceback)(struct dsetup *__restrict setup, void *mallptr, __ptbwalker callback, void *closure);
PRIVATE ssize_t (KCALL debug_enum)(struct dsetup *__restrict setup, struct instance *inst, void *checkpoint, ssize_t (KCALL *callback)(void *__restrict mallptr, void *closure), void *closure);
PRIVATE void    (KCALL debug_printleaks)(struct dsetup *__restrict setup, struct instance *inst);
PRIVATE void    (KCALL debug_validate)(struct dsetup *__restrict setup, struct instance *inst);
PRIVATE void   *(KCALL debug_setflag)(struct dsetup *__restrict setup, void *mallptr, u8 flags);
#ifdef CONFIG_HAVE_STRDUP_IN_KERNEL
PRIVATE void   *(KCALL debug_memcdup)(struct dsetup *__restrict setup, void const *__restrict ptr, int needle, size_t size, gfp_t flags);
PRIVATE char   *(KCALL debug_strdup)(struct dsetup *__restrict setup, char const *__restrict str, gfp_t flags);
PRIVATE char   *(KCALL debug_strndup)(struct dsetup *__restrict setup, char const *__restrict str, size_t max_chars, gfp_t flags);
PRIVATE char   *(KCALL debug_vstrdupf)(struct dsetup *__restrict setup, char const *__restrict format, va_list args, gfp_t flags);
#endif /* CONFIG_HAVE_STRDUP_IN_KERNEL */

#ifdef CONFIG_TRACE_LEAKS
/* Called for all tracked pointers when a driver is unloaded.
 * >> Using this, all pointers not previously tagged with `_mall_untrack()' or
 *    `_mall_nofree()' will be inherited by the kernel, as well as printed as leaks. */
INTDEF SAFE void (KCALL debug_add2core)(struct instance *__restrict inst);
#endif


#ifdef CONFIG_MALLOC_FREQUENCY
#if CONFIG_MALLOC_FREQUENCY > 0xffff || 1
typedef u32 mfreq_t;
#elif CONFIG_MALLOC_FREQUENCY > 0xff
typedef u16 mfreq_t;
#else
typedef u8 mfreq_t;
#endif
PRIVATE ATOMIC_DATA mfreq_t mall_check     = CONFIG_MALLOC_FREQUENCY; /*< Next time MALL is checked for inconsistencies. */
PRIVATE ATOMIC_DATA mfreq_t mall_checkfreq = CONFIG_MALLOC_FREQUENCY; /*< MALL consistency check frequency. */
PRIVATE ATTR_COLDTEXT void KCALL
mall_onfreq(struct dsetup *__restrict setup) {
#if 0
 syslog(LOG_MEM|LOG_DEBUG,"[MEM] Performing periodic memory validation\n");
#endif
 debug_validate(setup,NULL);
}
#define MALL_FREQ(setup,flags) \
do{ mfreq_t freq; \
    if (flags&GFP_NOFREQ) break; \
    do freq = ATOMIC_READ(mall_check); \
    while (!ATOMIC_CMPXCH_WEAK(mall_check,freq, \
            freq ? freq-1 : ATOMIC_READ(mall_checkfreq))); \
    if unlikely(!freq) mall_onfreq(setup); \
}while(0)

#else
#define MALL_FREQ(setup,flags) do{}while(0)
#endif

#if 1
#define MALL_FREQ_AFTER(setup,flags) \
        MALL_FREQ(setup,flags)
#else
#define MALL_FREQ_AFTER(setup,flags) do{}while(0)
#endif



PRIVATE void *(KCALL debug_malloc)(struct dsetup *__restrict setup,
                                   size_t size, gfp_t flags) {
 struct mptr *p;
 MALL_FREQ(setup,flags);
 p = MPTR_KMALLOC(size,flags);
 if (p != MPTR_OF(NULL)) mptr_setup(p,setup,size);
 MALL_FREQ_AFTER(setup,flags);
 return MPTR_USERADDR(p);
}
PRIVATE void *(KCALL debug_memalign)(struct dsetup *__restrict setup,
                                     size_t alignment, size_t size, gfp_t flags) {
 struct mptr *p;
 MALL_FREQ(setup,flags);
 p = MPTR_KMEMALIGN(alignment,size,flags);
 if (p != MPTR_OF(NULL)) mptr_setup(p,setup,size);
 MALL_FREQ_AFTER(setup,flags);
 return MPTR_USERADDR(p);
}
PRIVATE void *(KCALL debug_realloc)(struct dsetup *__restrict setup,
                                    void *ptr, size_t size, gfp_t flags) {
 struct mptr *p;
 MALL_FREQ(setup,flags);
 if (M_ISNONNULL(ptr))
     p = MPTR_KREALLOC(MPTR_GET(ptr),size,flags);
 else {
  if ((p = MPTR_KMALLOC(size,flags)) != MPTR_OF(NULL))
       mptr_setup(p,setup,size);
 }
 MALL_FREQ_AFTER(setup,flags);
 return MPTR_USERADDR(p);
}
PRIVATE void *(KCALL debug_realign)(struct dsetup *__restrict setup,
                                    void *ptr, size_t alignment,
                                    size_t size, gfp_t flags) {
 struct mptr *p;
 MALL_FREQ(setup,flags);
 if (M_ISNONNULL(ptr))
     p = MPTR_KREALIGN(MPTR_GET(ptr),alignment,size,flags);
 else {
  if ((p = MPTR_KMEMALIGN(alignment,size,flags)) != MPTR_OF(NULL))
       mptr_setup(p,setup,size);
 }
 MALL_FREQ_AFTER(setup,flags);
 return MPTR_USERADDR(p);
}

PRIVATE void (KCALL debug_free)(struct dsetup *__restrict setup, void *ptr, gfp_t flags) {
 if (M_ISNONNULL(ptr)) {
  struct mptr *p;
  MALL_FREQ(setup,flags);
  p = MPTR_GET(ptr);
  mptr_unlink(setup,p);
  MPTR_KFFREE(p,flags);
  MALL_FREQ_AFTER(setup,flags);
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
#ifdef CONFIG_MALLOC_FREQUENCY
 if (parameter_number == M_MALL_CHECK_FREQUENCY) {
  int result;
  if (--parameter_value < 0) parameter_value = (int)-1;
  result = (int)ATOMIC_XCH(mall_checkfreq,(u32)parameter_value);
  ATOMIC_WRITE(mall_check,(u32)parameter_value);
  if (result >= 0) ++result;
  return result;
 }
#endif
 return core_mallopt(parameter_number,parameter_value,flags);
}
PRIVATE int (KCALL debug_posix_memalign)(struct dsetup *__restrict setup,
                                         void **__restrict pp, size_t alignment,
                                         size_t n_bytes, gfp_t flags) {
 void *result = NULL;
 CHECK_HOST_DOBJ(pp);
 if (alignment == HEAP_ALIGNMENT)
  result = debug_memalign(setup,alignment,n_bytes,flags);
 else {
  size_t d = alignment / sizeof(void*);
  size_t r = alignment % sizeof(void*);
  if (r != 0 || d == 0 || (d & (d-1)) != 0)
      return EINVAL;
  result = debug_memalign(setup,alignment,n_bytes,flags);
 }
 if (!result) return ENOMEM;
 *pp = result;
 return 0;
}

#ifdef CONFIG_HAVE_STRDUP_IN_KERNEL
PRIVATE void *(KCALL debug_memcdup)(struct dsetup *__restrict setup,
                                    void const *__restrict ptr, int needle,
                                    size_t size, gfp_t flags) {
 if (size) {
  void const *endaddr = memchr(ptr,needle,size-1);
  if (endaddr) size = ((uintptr_t)endaddr-(uintptr_t)ptr)+1;
 }
 return debug_memdup(setup,ptr,size,flags);
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
#endif /* CONFIG_HAVE_STRDUP_IN_KERNEL */


PRIVATE SAFE struct mptr *KCALL
mptr_safeload(struct dsetup *__restrict setup,
              void *__restrict p) {
 ATTR_UNUSED size_t i;
 struct mptr *head;
 struct mman *m;
#ifdef MPTR_HAVE_TAIL
 struct mptr_tail *tail;
#endif
 assert(TASK_ISSAFE());
 CHECK_HOST_DOBJ(setup);
 head = MPTR_OF(p);
 /* Validate generic constraints. */
 if (!IS_ALIGNED((uintptr_t)head,HEAP_ALIGNMENT) ||
     !OK_HOST_DATA(head,sizeof(struct mptr)))
      goto invptr;
 /* Check if the given pointer is already apart of a free heap. */
 if (addr_isglob(head)) {
  bool is_mapped,is_free = false;
  TASK_PDIR_KERNEL_BEGIN(m);
  is_mapped = mman_inuse(&mman_kernel,
                        (ppage_t)FLOOR_ALIGN((uintptr_t)head,PAGESIZE),
                         PAGESIZE);
  if (is_mapped) is_free = (mheap_isfree_l(&mheaps[GFP_KERNEL],head) ||
                            mheap_isfree_l(&mheaps[GFP_KERNEL|GFP_LOCKED],head));
  TASK_PDIR_KERNEL_END(m);
  if (!is_mapped) goto invptr;
  if (is_free) goto freeptr;
  if (mheap_isfree_l(&mheaps[GFP_SHARED],head)) goto freeptr;
  if (mheap_isfree_l(&mheaps[GFP_SHARED|GFP_LOCKED],head)) goto freeptr;
 } else {
  if (!PDIR_ISKPD()) goto invptr;
  if (page_query(head,1) & PAGEATTR_FREE) goto freeptr;
  if (mheap_isfree_l(&mheaps[GFP_MEMORY],head)) goto freeptr;
 }

 if (MPTR_TYPE(head) >= __GFP_HEAPCOUNT)     malloc_panic(setup,NULL,"Invalid heap id #%d at %p, offset %Id in %p",MPTR_TYPE(head),&head->m_type,(intptr_t)&head->m_type-(intptr_t)p,p);
 if (MPTR_SIZE(head) < MPTR_SIZEOF(0))       malloc_panic(setup,NULL,"Invalid pointer size %Iu < %Iu at %p, offset %Id in %p",MPTR_SIZE(head),MPTR_SIZEOF(0),&head->m_size,(intptr_t)&head->m_size-(intptr_t)p,p);
 if (!ATOMIC_READ(head->m_refcnt))           malloc_panic(setup,NULL,"Invalid reference counter 0 at %p, offset %Id in %p",&head->m_refcnt,(intptr_t)&head->m_refcnt-(intptr_t)p,p);
#ifdef MPTR_HAVE_TAIL
 if ((void *)(tail = head->m_tail) < p)      malloc_panic(setup,NULL,"Invalid tail address %p < %p at %p, offset %Id in %p",tail,p,&head->m_tail,(intptr_t)&head->m_tail-(intptr_t)p,p);
 if (MPTR_TAILSIZE(head) < MPTR_MINTAILSIZE) malloc_panic(setup,NULL,"Tail %p of %p is too small (%Iu < %Iu) in %p",tail,head,(size_t)MPTR_TAILSIZE(head),(size_t)MPTR_MINTAILSIZE,p);
#endif
#ifdef CONFIG_MALLOC_HEADSIZE
 /* Validate header bytes */
 i = CONFIG_MALLOC_HEADSIZE; while (i--) {
  if (head->m_head[i] != MALL_HEADERBYTE(i)) {
   malloc_panic(setup,head,"Header corruption (%#.2I8x != %#.2I8x) at %p of %p (at offset %Id; header index %d)\n"
                           "%$[hex]",
                head->m_head[i],MALL_HEADERBYTE(i),&head->m_head[i],p,
               (intptr_t)&head->m_head[i]-(intptr_t)p,i,
                CONFIG_MALLOC_HEADSIZE,head->m_head);
  }
 }
#endif
#ifdef CONFIG_MALLOC_FOOTSIZE
 /* Validate footer bytes */
 for (i = 0; i < CONFIG_MALLOC_FOOTSIZE; ++i) {
  if (tail->t_foot[i] != MALL_FOOTERBYTE(i)) {
   malloc_panic(setup,head,"Footer corruption (%#.2I8x != %#.2I8x) at %p of %p (at offset %Id; footer index %d)\n"
                           "%$[hex]",
                tail->t_foot[i],MALL_FOOTERBYTE(i),&tail->t_foot[i],p,
               (intptr_t)&tail->t_foot[i]-(intptr_t)p,i,
                CONFIG_MALLOC_FOOTSIZE,tail->t_foot);
  }
 }
#endif
#ifdef CONFIG_TRACE_LEAKS
 { u16 chksum = mptr_chksum(head);
   if (chksum != head->m_chksum) {
    malloc_panic(setup,head,"Invalid checksum (expected %I32x, got %I32x) at %p, offset %Id in %p",
                 chksum,head->m_chksum,&head->m_chksum,
                (intptr_t)&head->m_chksum-(intptr_t)p,p);
   }
 }
 {
  REF struct instance *inst;
  /* Validate mall linkage */
  atomic_rwlock_read(&mptr_inst_lock);
  inst = head->m_info.i_inst;
  if (!OK_HOST_DATA(inst,sizeof(struct instance)) ||
      !OK_HOST_DATA(inst->i_module,sizeof(struct module)) ||
      !INSTANCE_INKERNEL(inst)) {
   atomic_rwlock_endread(&mptr_inst_lock);
   malloc_panic(setup,head,"Invalid associated instance %p at %p, offset %Id in %p",
                inst,&head->m_info.i_inst,(intptr_t)&head->m_info.i_inst-(intptr_t)p,p);
  }
  if (!INSTANCE_TRYINCREF(inst)) inst = NULL;
  atomic_rwlock_endread(&mptr_inst_lock);
  if (inst) {
   if (atomic_rwlock_tryread(&inst->i_driver.k_tlock)) {
    if (head->m_flag&~MPTRFLAG_MASK) {
     malloc_panic(setup,head,"Unknown flags %#.4I16x at %p, offset %Id in %p",
                  head->m_flag,&head->m_flag,(intptr_t)&head->m_flag-(intptr_t)p,p);
    }
    if (!(head->m_flag&MPTRFLAG_UNTRACKED)) {
     struct mptr *linked_self;
     struct mman *old_mm = NULL;
     if (addr_ispriv(head->m_chain.le_pself))
         TASK_PDIR_KERNEL_BEGIN(old_mm);
     if (!OK_HOST_DATA(head->m_chain.le_pself,sizeof(void *))) {
      malloc_panic(setup,head,"Broken chain self-pointer (%p at &p, offset %Id) in %p",
                   head->m_chain.le_pself,&head->m_chain.le_pself,
                  (intptr_t)&head->m_chain.le_pself-(intptr_t)p,p);
     }
     linked_self = *head->m_chain.le_pself;
     if (head != linked_self) {
      malloc_panic(setup,head,"Incorrect self-pointer (expected %p, got %p in at %p at offset %Id) in %p",
                   head,linked_self,head->m_chain.le_pself,
                  (intptr_t)&head->m_chain.le_pself-(intptr_t)p,p);
     }
     if (head->m_chain.le_next != KINSTANCE_TRACE_NULL) {
      struct mptr *next = head->m_chain.le_next;
      if (!old_mm && addr_ispriv(next)) TASK_PDIR_KERNEL_BEGIN(old_mm);
      if (!OK_HOST_DATA(next,sizeof(struct mptr))) {
       malloc_panic(setup,head,"Broken chain next-pointer (%p at %p, offset %Id) in %p",
                    next,&head->m_chain.le_next,
                   (intptr_t)&head->m_chain.le_next-(intptr_t)p,p);
      }
      if (&head->m_chain.le_next != next->m_chain.le_pself) {
       malloc_panic(setup,head,"Broken chain next-self-pointer (expected %p, got %p) following %p",
                   &head->m_chain.le_next,next->m_chain.le_pself,p);
      }
     }
     if (old_mm) TASK_PDIR_KERNEL_END(old_mm);
    }
    atomic_rwlock_endread(&inst->i_driver.k_tlock);
   }
   INSTANCE_DECREF(inst);
  }
 }
#endif
 return head;
invptr:  malloc_panic(setup,NULL,"Faulty malloc-pointer %p",p);
freeptr: malloc_panic(setup,NULL,"malloc-pointer %p has already been freed",p);
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
 self->m_refcnt      = 1;
 self->m_flag        = MPTRFLAG_NONE;
 self->m_line        = (s32)setup->s_info.i_line;
 self->m_info.i_file = setup->s_info.i_file;
 self->m_info.i_func = setup->s_info.i_func;
 self->m_info.i_inst = setup->s_info.i_inst;
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
   assertf(tail_size >= CONFIG_MALLOC_FOOTSIZE+CONFIG_MALLOC_TRACEBACK_MINSIZE*sizeof(void *),"%Iu",tail_size);
#elif defined(CONFIG_MALLOC_FOOTSIZE)
   assertf(tail_size >= CONFIG_MALLOC_FOOTSIZE,"%Iu",tail_size);
#else
   assertf(tail_size >= CONFIG_MALLOC_TRACEBACK_MINSIZE*sizeof(void *),"%Iu",tail_size);
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
   { void **iter; size_t tail_overflow;
     assert(tail_size >= CONFIG_MALLOC_TRACEBACK_MINSIZE*sizeof(void *));
     iter           = tail->t_tb;
     tail_overflow  = tail_size % sizeof(void *);
     tail_size     /= sizeof(void *);
     /* Fill the traceback portion. */
#if defined(__i386__) || defined(__x86_64__)
     if (tail_size) {
      register uintptr_t temp;
      /* Implement in assembly to take advantage of local interrupt handlers.
       * >> If a pagefault occurrs while accessing a frame pointer, stop creation
       *    of the traceback (it is either corrupted, was customized, or has terminated) */
      __asm__ __volatile__(/* BEGIN_EXCEPTION_HANDLER(EXC_PAGE_FAULT) */
                           L(    ipushx_sym(%[temp],3f)                                     )
                           L(    pushx $(EXC_PAGE_FAULT)                                    )
                           L(    pushx TASK_OFFSETOF_IC(%[task])                            )
                           L(    movx  %%xsp, TASK_OFFSETOF_IC(%[task])                     )
                           L(                                                               )
                           L(1:  testx %[frame], %[frame] /* >> if (!frame) break; */       )
                           L(    jz    2f                                                   )
                           L(    movx  XSZ(%[frame]), %[temp] /* >> *iter++ = frame->f_return; */)
                           L(    movx  %[temp], 0(%[iter])                                  )
                           L(    addx  $(XSZ), %[iter]                                      )
                           L(    subx  $1, %[size]            /* >> if (!--size) break; */  )
                           L(    jz    2f                                                   )
                           L(    movx  0(%[frame]), %[frame]  /* >> frame = frame->f_return; */)
                           L(    jmp   1b                     /* >> continue; */            )
                           /* END_EXCEPTION_HANDLER(EXC_PAGE_FAULT) */
                           L(2:  popx  TASK_OFFSETOF_IC(%[task])                            )
                           L(    addx  $(2*XSZ), %%xsp                                      )
                           L(3:  /* This is where execution jumps when something went wrong. */)
                           : [iter]  "+D" (iter)
                           , [size]  "+c" (tail_size)
                           , [temp]  "=a" (temp)
                           : [frame] "S" (setup->s_tbebp) /* struct frame */
                           , [task]  "d" (THIS_TASK)
                           : "memory", "cc");
     }
#else
#warning "FIXME: No local exception handler used for tracebacks."
     /* TODO: Use `call_user_worker' */
     { struct frame { struct frame *f_caller; void *f_return; };
       struct frame *frame = (struct frame *)setup->s_tbebp;
       for (; tail_size; --tail_size, ++iter) {
        if (!frame) break;
        if (!OK_HOST_DATA(frame,sizeof(struct frame))) break;
        *iter = frame->f_return;
        frame = frame->f_caller;
       }
     }
#endif
     /* XXX: Free unused trailing data? */

     /* Fill the remainder with EOF entries. */
     while (tail_size--) *iter++ = MPTR_TAIL_TB_EOF;
     /* Final trailing bytes. */
     while (tail_overflow--) *(*(byte_t **)&iter)++ = (byte_t)((uintptr_t)MPTR_TAIL_TB_EOF & 0xff);
   }
#endif
   self->m_tail = tail;
 }
#endif
#ifdef CONFIG_TRACE_LEAKS
 self->m_chksum = mptr_chksum(self);

 /* Link the pointer for the first time. */
 { struct instance *inst = setup->s_info.i_inst;
   struct mman *old_mm = NULL;
   assert(inst);
   assert(inst->i_module);
   atomic_rwlock_write(&inst->i_driver.k_tlock);
   if (addr_ispriv(inst->i_driver.k_trace) &&
       inst->i_driver.k_trace != KINSTANCE_TRACE_NULL)
       TASK_PDIR_KERNEL_BEGIN(old_mm);
   LIST_INSERT_EX(inst->i_driver.k_trace,self,m_chain,
                  KINSTANCE_TRACE_NULL);
   if (old_mm) TASK_PDIR_KERNEL_END(old_mm);
   atomic_rwlock_endwrite(&inst->i_driver.k_tlock);
 }
#endif
}
#ifdef CONFIG_TRACE_LEAKS
LOCAL u16 KCALL
gen_chksum(byte_t const *__restrict iter,
           size_t n_bytes, u16 sum) {
 while (n_bytes--) sum = (sum*9)+*iter++;
 return sum;
}
PRIVATE u16 KCALL
mptr_chksum(struct mptr *__restrict self) {
#ifdef MPTR_HAVE_TAIL
 struct mptr_tail *tail = self->m_tail;
#endif
 u16 result = 0xc13f;
 result = gen_chksum((byte_t *)&self->m_flag,
                      offsetafter(struct mptr,m_size)-
                      offsetof(struct mptr,m_flag),result);
#ifdef CONFIG_MALLOC_TRACEBACK
 result = gen_chksum((byte_t *)&tail->t_tb[0],
                      MPTR_TAILSIZE(self)
#ifdef CONFIG_MALLOC_FOOTSIZE
                     -CONFIG_MALLOC_FOOTSIZE
#endif
                     ,result);
#endif /* CONFIG_MALLOC_TRACEBACK */
 result ^= (uintptr_t)self & 0xffff;
 result ^= (uintptr_t)self >> 16;
#ifdef MPTR_HAVE_TAIL
 result ^= (uintptr_t)tail & 0xffff;
 result ^= (uintptr_t)tail >> 16;
#endif
 return result;
}
#endif /* CONFIG_TRACE_LEAKS */

#ifdef MPTR_HAVE_TAIL
PRIVATE void KCALL
mptr_mvtail(struct mptr *__restrict self,
            size_t old_size, size_t old_user_size,
            size_t new_size, size_t new_user_size,
            gfp_t flags) {
 void *old_tail,*new_tail;
 size_t tail_size,tail_unused;
 CHECK_HOST_DOBJ(self);
 assert(MPTR_SIZE(self) == new_size);
 assert(new_size >= MPTR_SIZEOF(0));
 assert(IS_ALIGNED(old_size,HEAP_ALIGNMENT));
 assert(IS_ALIGNED(new_size,HEAP_ALIGNMENT));
 assert(old_user_size <= old_size);
 assert(new_user_size <= new_size);
 old_tail    = self->m_tail;
 new_tail    = (void *)(((uintptr_t)old_tail-old_user_size)+new_user_size);
 tail_size   = ((uintptr_t)self+old_size)-(uintptr_t)old_tail;
 assert(((uintptr_t)self+old_size) >= (uintptr_t)old_tail);
 {
  uintptr_t new_end      = (uintptr_t)self+new_size;
  uintptr_t new_tail_end = (uintptr_t)new_tail+tail_size;
  if (new_end >= new_tail_end)
      tail_unused = new_end-new_tail_end;
  else {
   /* Truncate the tail. */
   assert(tail_size >= (new_tail_end-new_end));
   tail_size  -= (new_tail_end-new_end);
   tail_unused = 0;
  }
 }
 assert((uintptr_t)self+new_size >= (uintptr_t)new_tail+tail_size);

#if 0
 syslog(LOG_DEBUG,"MOVE_TAIL %Iu(%Iu) -> %Iu(%Iu) (%p -> %p)\n",
        old_size-MPTR_SIZEOF(0),old_user_size-MPTR_SIZEOF(0),
        new_size-MPTR_SIZEOF(0),new_user_size-MPTR_SIZEOF(0),
        old_tail,new_tail);
#endif
 assertf(tail_size < new_size,
         "old_tail      = %p\n"
         "tail_size     = %Iu\n"
         "old_size      = %Iu\n"
         "new_size      = %Iu\n"
         "old_user_size = %Iu\n"
         "new_user_size = %Iu\n",
         old_tail,tail_size,old_size,new_size,
         old_user_size,new_user_size);
 self->m_tail = (struct mptr_tail *)new_tail;
 memmove(new_tail,old_tail,tail_size);
 (void)tail_unused;
#ifdef CONFIG_MALLOC_TRACEBACK
 memset((void *)((uintptr_t)new_tail+tail_size),
        (byte_t)((uintptr_t)MPTR_TAIL_TB_EOF & 0xff),
         tail_unused);
#endif

#ifdef CONFIG_TRACE_LEAKS
 /* Update the associated checksum. */
 self->m_chksum = mptr_chksum(self);
#endif /* CONFIG_TRACE_LEAKS */

 if (old_size > new_size) {
  size_t release_size = old_size-new_size;
  void  *release_addr = (void *)((uintptr_t)self+new_size);
#ifdef CONFIG_MALLOC_DEBUG_INIT
  if (flags&GFP_CALLOC)
       memset(release_addr,0,release_size);
  else MEMPATX(release_addr,CONFIG_MALLOC_DEBUG_INIT,release_size);
#else
  if (flags&GFP_CALLOC)
      memset(release_addr,0,release_size);
#endif
 }
}
#endif /* MPTR_HAVE_TAIL */
#ifdef CONFIG_TRACE_LEAKS
PRIVATE void KCALL
mptr_unlink(struct dsetup *setup,
            struct mptr *__restrict self) {
 struct instance *inst;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&mptr_inst_lock);
 inst = self->m_info.i_inst;
 atomic_rwlock_write(&inst->i_driver.k_tlock);
 if (self->m_flag&MPTRFLAG_NOFREE) {
  malloc_panic(setup,self,
               "Cannot realloc/free pointer %p (%p...%p / %p...%p) marked as NOFREE",
               self,MPTR_USERADDR(self),(uintptr_t)MPTR_USERADDR(self)+MPTR_USERSIZE(self),
               self,(uintptr_t)self+MPTR_SIZE(self));
 }
 if (!(self->m_flag&MPTRFLAG_UNTRACKED)) {
  struct mman *old_mm = NULL;
  if (addr_ispriv(self->m_chain.le_pself) ||
     (self->m_chain.le_next != KINSTANCE_TRACE_NULL &&
      addr_ispriv(self->m_chain.le_next)))
      TASK_PDIR_KERNEL_BEGIN(old_mm);
  LIST_REMOVE_EX(self,m_chain,KINSTANCE_TRACE_NULL);
  if (old_mm) TASK_PDIR_KERNEL_END(old_mm);
 }
 atomic_rwlock_endwrite(&inst->i_driver.k_tlock);
 atomic_rwlock_endread(&mptr_inst_lock);
}
PRIVATE void KCALL
mptr_relink(struct dsetup *setup,
            struct mptr *__restrict self) {
 struct instance *inst;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&mptr_inst_lock);
 inst = self->m_info.i_inst;
 atomic_rwlock_write(&inst->i_driver.k_tlock);
 if (!(self->m_flag&MPTRFLAG_UNTRACKED)) {
  struct mman *old_mm = NULL;
  if (addr_ispriv(inst->i_driver.k_trace) &&
      inst->i_driver.k_trace != KINSTANCE_TRACE_NULL)
      TASK_PDIR_KERNEL_BEGIN(old_mm);
  LIST_INSERT_EX(inst->i_driver.k_trace,self,m_chain,KINSTANCE_TRACE_NULL);
  if (old_mm) TASK_PDIR_KERNEL_END(old_mm);
 }
 atomic_rwlock_endwrite(&inst->i_driver.k_tlock);
 atomic_rwlock_endread(&mptr_inst_lock);
}
#endif /* CONFIG_TRACE_LEAKS */

PRIVATE void *(KCALL debug_getattrib)(struct dsetup *__restrict setup,
                                      void *mallptr, int attrib) {
 struct mptr *p; void *result;
 if unlikely(!mallptr) return NULL;
 p = mptr_safeload(setup,mallptr);
 switch (attrib) {
#ifdef CONFIG_TRACE_LEAKS
 case __MALL_ATTRIB_FILE: result = (void *)MPTR_FILE(p); break;
 case __MALL_ATTRIB_LINE: result = (void *)(intptr_t)MPTR_LINE(p); break;
 case __MALL_ATTRIB_FUNC: result = (void *)MPTR_FUNC(p); break;
 case __MALL_ATTRIB_INST:
  atomic_rwlock_read(&mptr_inst_lock);
  result = (void *)MPTR_INST(p);
  if (!INSTANCE_TRYINCREF((struct instance *)result)) {
   assertef(INSTANCE_INCREF(THIS_INSTANCE),"But we're the core...");
   result = THIS_INSTANCE;
  }
  atomic_rwlock_endread(&mptr_inst_lock);

  break;
#else
 case __MALL_ATTRIB_LINE: result = (void *)(intptr_t)(int)-1; break;
 case __MALL_ATTRIB_FILE:
 case __MALL_ATTRIB_FUNC: result = (void *)"??" "?"; break;
 case __MALL_ATTRIB_INST:
  result = (void *)THIS_INSTANCE;
  if (!INSTANCE_INCREF(result)) result = NULL;
  break;
#endif
 case __MALL_ATTRIB_SIZE: result = (void *)(uintptr_t)MPTR_USERSIZE(p); break;
 default: result = NULL; break;
 }
 return result;
}
PRIVATE ssize_t (KCALL debug_traceback)(struct dsetup *__restrict setup,
                                        void *mallptr, __ptbwalker callback,
                                        void *closure) {
 if unlikely(!mallptr) return 0;
 (void)setup;
 (void)callback;
 (void)closure;
 /* TODO */
 return 0;
}
PRIVATE ssize_t (KCALL debug_enum)(struct dsetup *__restrict setup, struct instance *inst,
                                   void *checkpoint, ssize_t (KCALL *callback)(void *__restrict mallptr,
                                                                               void *closure),
                                   void *closure) {
#ifdef CONFIG_TRACE_LEAKS
 struct mptr *chain,**piter,*iter;
 struct mptr *next,*checkpoint_ptr;
 ssize_t temp,result = 0;
 struct mman *old_mm = NULL;
 if unlikely(!callback) return 0;
 if (!inst) {
  /* TODO: Validate all drivers + the core when `inst' is NULL. */
  inst = THIS_INSTANCE;
 }
 checkpoint_ptr = checkpoint ? mptr_safeload(setup,checkpoint) : KINSTANCE_TRACE_NULL;
 atomic_rwlock_write(&inst->i_driver.k_tlock);
 /* Pop the chain of traced pointers from the instance. */
 chain = inst->i_driver.k_trace;
 inst->i_driver.k_trace = KINSTANCE_TRACE_NULL;
 atomic_rwlock_endwrite(&inst->i_driver.k_tlock);
 if unlikely(chain == KINSTANCE_TRACE_NULL) return 0;

 TASK_PDIR_KERNEL_BEGIN(old_mm);
 assert(chain->m_chain.le_pself == &inst->i_driver.k_trace);
 chain->m_chain.le_pself = &chain;

 iter = chain;
 while (iter != KINSTANCE_TRACE_NULL) {
  if (checkpoint_ptr == iter) break;
  next = iter->m_chain.le_next;
  assert(iter != next);
  temp = (*callback)(MPTR_USERADDR(iter),closure);
  if (temp < 0) { result = temp; break; }
  result += temp;
  iter = next;
 }

 atomic_rwlock_write(&inst->i_driver.k_tlock);
 assert(chain->m_chain.le_pself == &chain);
 /* Re-append the chain to the instance. */
 piter = &inst->i_driver.k_trace;
 while ((iter = *piter) != KINSTANCE_TRACE_NULL)
         piter = &iter->m_chain.le_next;
 *piter = chain;
 chain->m_chain.le_pself = piter;
 atomic_rwlock_endwrite(&inst->i_driver.k_tlock);
 TASK_PDIR_KERNEL_END(old_mm);

 return result;
#else
 (void)setup;
 (void)inst;
 (void)checkpoint;
 (void)callback;
 (void)closure;
 return 0;
#endif
}

#ifdef CONFIG_TRACE_LEAKS
#define F_PRINTF(...) \
do{ if ((temp = format_printf(printer,closure,__VA_ARGS__)) < 0) return temp; \
    result += temp; \
}while(0)
PRIVATE ssize_t (KCALL mptr_printleak)(struct mptr *__restrict self,
                                       pformatprinter printer, void *closure) {
 ssize_t temp,result = 0;
 F_PRINTF("##################################################\n"
          "%s(%d) : %s : Leaked %Iu bytes at %p\n",
          MPTR_FILE(self),MPTR_LINE(self),
          MPTR_FUNC(self),MPTR_USERSIZE(self),
          MPTR_USERADDR(self));
#ifdef CONFIG_MALLOC_TRACEBACK
 { void **iter,**end; size_t pos = 0;
   end = (iter = MPTR_TRACEBACK_ADDR(self))+MPTR_TRACEBACK_SIZE(self);
   for (; iter != end; ++iter,++pos) {
#ifdef CONFIG_USE_EXTERNAL_ADDR2LINE
    F_PRINTF("#!$ addr2line(%p) '{file}({line}) : {func} : [%Ix] : %p'\n",
            (uintptr_t)*iter-1,pos,*iter);
#else
    F_PRINTF("%[vinfo] : [%Ix] : %p\n",
            (uintptr_t)*iter-1,pos,*iter);
#endif
   }
 }
#endif
 return result;
}
#undef F_PRINTF

INTERN SAFE void (KCALL debug_add2core)(struct instance *__restrict inst) {
 struct mptr *chain,*chain_end;
 struct mman *old_mm;
 TASK_PDIR_KERNEL_BEGIN(old_mm);
 atomic_rwlock_write(&mptr_inst_lock);

 /* Extract all pointers from this instance. */
 atomic_rwlock_write(&inst->i_driver.k_tlock);
 chain = inst->i_driver.k_trace;
 inst->i_driver.k_trace = KINSTANCE_TRACE_NULL;
 atomic_rwlock_endwrite(&inst->i_driver.k_tlock);

 /* At this point, nobody is really tracking the pointers from this instance.
  * Note tough, that someone may be attempting to access one of its pointers
  * right now, which is why we must keep on holding a lock to `mptr_inst_lock'
  * in order to prevent them from doing anything... */

 if (chain != KINSTANCE_TRACE_NULL) {
  chain_end = chain;
  for (;;) {
   /* Update instance pointers. */
   assert(chain_end->m_info.i_inst == inst);
   chain_end->m_info.i_inst = THIS_INSTANCE;
   /* Make sure to delete the file & function pointers.
    * (they _were_ part of the module now unloaded, meaning they are now dangling) */
   chain_end->m_info.i_file = NULL;
   chain_end->m_info.i_func = NULL;
#ifdef CONFIG_TRACE_LEAKS
   /* Update the associated checksum. */
   chain_end->m_chksum = mptr_chksum(chain_end);
#endif /* CONFIG_TRACE_LEAKS */
   assert(!(chain_end->m_flag&MPTRFLAG_UNTRACKED));
   if (!(chain_end->m_flag&(MPTRFLAG_NOFREE|MPTRFLAG_GLOBAL))) {
    mptr_printleak(chain_end,&syslog_printer,
                   SYSLOG_PRINTER_CLOSURE(LOG_MEM|LOG_DEBUG));
   }
   if (chain_end->m_chain.le_next == KINSTANCE_TRACE_NULL) break;
   chain_end = chain_end->m_chain.le_next;
  }

  /* Re-insert all pointers into the core module, so-as to keep tracking them! */
  atomic_rwlock_write(&THIS_INSTANCE->i_driver.k_tlock);
  chain_end->m_chain.le_next = THIS_INSTANCE->i_driver.k_trace;
  if (THIS_INSTANCE->i_driver.k_trace != KINSTANCE_TRACE_NULL)
      THIS_INSTANCE->i_driver.k_trace->m_chain.le_pself = &chain_end->m_chain.le_next;

  /* Update the chain heap pointer. */
  THIS_INSTANCE->i_driver.k_trace = chain;
  assert(chain->m_chain.le_pself == &inst->i_driver.k_trace);
  chain->m_chain.le_pself = &THIS_INSTANCE->i_driver.k_trace;

  atomic_rwlock_endwrite(&THIS_INSTANCE->i_driver.k_tlock);
 }
 atomic_rwlock_endwrite(&mptr_inst_lock);
 TASK_PDIR_KERNEL_END(old_mm);
}
#endif

PRIVATE ssize_t KCALL
debug_printleaks_callback(void *__restrict mallptr,
                          void *closure) {
 struct mptr *p = mptr_safeload((struct dsetup *)closure,mallptr);
#ifdef CONFIG_TRACE_LEAKS
 if (p->m_flag&MPTRFLAG_NOFREE) return 0;
#endif
 return mptr_printleak(p,&syslog_printer,
                       SYSLOG_PRINTER_CLOSURE(LOG_MEM|LOG_DEBUG));
}
PRIVATE ATTR_UNUSED ssize_t KCALL
debug_validate_callback(void *__restrict mallptr,
                        void *closure) {
 mptr_safeload((struct dsetup *)closure,mallptr);
 return 0;
}

PRIVATE void (KCALL debug_printleaks)(struct dsetup *__restrict setup, struct instance *inst) {
 task_crit();
 debug_enum(setup,inst,NULL,&debug_printleaks_callback,setup);
 task_endcrit();
}
PRIVATE void (KCALL debug_validate)(struct dsetup *__restrict setup, struct instance *inst) {
 struct mman *old_mm;
 task_crit();
 TASK_PDIR_KERNEL_BEGIN(old_mm);
#ifdef CONFIG_DEBUG_HEAP
 mheap_validate(setup,&mheaps[GFP_SHARED]);
 mheap_validate(setup,&mheaps[GFP_SHARED|GFP_LOCKED]);
 mheap_validate(setup,&mheaps[GFP_KERNEL]);
 mheap_validate(setup,&mheaps[GFP_KERNEL|GFP_LOCKED]);
 mheap_validate(setup,&mheaps[GFP_MEMORY]);
#endif
#if !MHEAP_USING_INTERNAL_VALIDATION
 debug_enum(setup,inst,NULL,&debug_validate_callback,setup);
#endif
 TASK_PDIR_KERNEL_END(old_mm);
 task_endcrit();
}
PRIVATE void *(KCALL debug_setflag)(struct dsetup *__restrict setup,
                                    void *mallptr, u8 flags) {
 struct instance *inst;
 struct mptr *p = mptr_safeload(setup,mallptr);
 atomic_rwlock_read(&mptr_inst_lock);
 inst = p->m_info.i_inst;
 atomic_rwlock_write(&inst->i_driver.k_tlock);
 if (flags&MPTRFLAG_UNTRACKED &&
   !(p->m_flag&MPTRFLAG_UNTRACKED)) {
  struct mman *old_mm = NULL;
  if (addr_ispriv(p->m_chain.le_pself) ||
     (p->m_chain.le_next != KINSTANCE_TRACE_NULL &&
      addr_ispriv(p->m_chain.le_next)))
      TASK_PDIR_KERNEL_BEGIN(old_mm);
  LIST_REMOVE_EX(p,m_chain,KINSTANCE_TRACE_NULL);
  if (old_mm) TASK_PDIR_KERNEL_END(old_mm);
 }
 p->m_flag |= flags;
 p->m_chksum = mptr_chksum(p);
 atomic_rwlock_endwrite(&inst->i_driver.k_tlock);
 atomic_rwlock_endread(&mptr_inst_lock);
 return mallptr;
}


#ifdef __x86_64__
#define GET_EBP(r) __asm__ __volatile__("movq %%rbp, %0" : "=g" (r))
#elif defined(__i386__)
#define GET_EBP(r) __asm__ __volatile__("movl %%ebp, %0" : "=g" (r))
#elif defined(__arm__)
#define GET_EBP(r) __asm__ __volatile__("mov %0, lr" : "=r" (r))
#else
#define GET_EBP(r) (void)((r) = NULL)
#endif

#define DEF_SETUP(name) \
  struct dsetup name; \
  memset(&name.s_info,0,sizeof(struct dinfo)); \
  name.s_info.i_inst = THIS_INSTANCE; \
  GET_EBP(name.s_tbebp)
PUBLIC ATTR_NOINLINE void *(KCALL kmalloc)(size_t size, gfp_t flags)                                                { DEF_SETUP(setup); return debug_malloc(&setup,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL krealloc)(void *ptr, size_t size, gfp_t flags)                                    { DEF_SETUP(setup); return debug_realloc(&setup,ptr,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL kmemalign)(size_t alignment, size_t size, gfp_t flags)                            { DEF_SETUP(setup); return debug_memalign(&setup,alignment,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL krealign)(void *ptr, size_t alignment, size_t size, gfp_t flags)                  { DEF_SETUP(setup); return debug_realign(&setup,ptr,alignment,size,flags); }
PUBLIC ATTR_NOINLINE void (KCALL kfree)(void *ptr)                                                                  { DEF_SETUP(setup);        debug_free(&setup,ptr,GFP_NORMAL); }
PUBLIC ATTR_NOINLINE void (KCALL kffree)(void *ptr, gfp_t flags)                                                    { DEF_SETUP(setup);        debug_free(&setup,ptr,flags); }
PUBLIC ATTR_NOINLINE size_t (KCALL kmalloc_usable_size)(void *ptr)                                                  { DEF_SETUP(setup); return debug_malloc_usable_size(&setup,ptr); }
PUBLIC ATTR_NOINLINE gfp_t (KCALL kmalloc_flags)(void *ptr)                                                         { DEF_SETUP(setup); return debug_malloc_flags(&setup,ptr); }
PUBLIC ATTR_NOINLINE void *(KCALL kmemdup)(void const *__restrict ptr, size_t size, gfp_t flags)                    { DEF_SETUP(setup); return debug_memdup(&setup,ptr,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL kmemadup)(void const *__restrict ptr, size_t alignment, size_t size, gfp_t flags) { DEF_SETUP(setup); return debug_memadup(&setup,ptr,alignment,size,flags); }
PUBLIC ATTR_NOINLINE int (KCALL kmallopt)(int parameter_number, int parameter_value, gfp_t flags)                   { DEF_SETUP(setup); return debug_mallopt(&setup,parameter_number,parameter_value,flags); }

PUBLIC ATTR_NOINLINE int (KCALL mallopt)(int parameter_number, int parameter_value)                     { DEF_SETUP(setup); return debug_mallopt(&setup,parameter_number,parameter_value,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL malloc)(size_t n_bytes)                                               { DEF_SETUP(setup); return debug_malloc(&setup,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL calloc)(size_t count, size_t n_bytes)                                 { DEF_SETUP(setup); return debug_malloc(&setup,count*n_bytes,__GFP_MALLOC|GFP_CALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL memalign)(size_t alignment, size_t n_bytes)                           { DEF_SETUP(setup); return debug_memalign(&setup,alignment,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL realloc)(void *__restrict mallptr, size_t n_bytes)                    { DEF_SETUP(setup); return debug_realloc(&setup,mallptr,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL realloc_in_place)(void *__restrict mallptr, size_t n_bytes)           { DEF_SETUP(setup); return debug_realloc(&setup,mallptr,n_bytes,__GFP_MALLOC|GFP_NOMOVE); }
PUBLIC ATTR_NOINLINE void *(KCALL valloc)(size_t n_bytes)                                               { DEF_SETUP(setup); return debug_memalign(&setup,PAGESIZE,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL pvalloc)(size_t n_bytes)                                              { DEF_SETUP(setup); return debug_memalign(&setup,PAGESIZE,(n_bytes+PAGESIZE-1)&~(PAGESIZE-1),__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL memdup)(void const *__restrict ptr, size_t n_bytes)                   { DEF_SETUP(setup); return debug_memdup(&setup,ptr,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE int (KCALL posix_memalign)(void **__restrict pp, size_t alignment, size_t n_bytes) { DEF_SETUP(setup); return debug_posix_memalign(&setup,pp,alignment,n_bytes,__GFP_MALLOC); }
#ifdef CONFIG_HAVE_STRDUP_IN_KERNEL
PUBLIC ATTR_NOINLINE char *(KCALL strdup)(char const *__restrict str)                                   { DEF_SETUP(setup); return debug_strdup(&setup,str,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(KCALL strndup)(char const *__restrict str, size_t max_chars)                { DEF_SETUP(setup); return debug_strndup(&setup,str,max_chars,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL memcdup)(void const *__restrict ptr, int needle, size_t n_bytes)      { DEF_SETUP(setup); return debug_memcdup(&setup,ptr,needle,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(KCALL vstrdupf)(char const *__restrict format, va_list args)                { DEF_SETUP(setup); return debug_vstrdupf(&setup,format,args,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(ATTR_CDECL strdupf)(char const *__restrict format, ...)                     { va_list args; char *result; DEF_SETUP(setup); va_start(args,format); result = debug_vstrdupf(&setup,format,args,__GFP_MALLOC); va_end(args); return result; }
#endif /* CONFIG_HAVE_STRDUP_IN_KERNEL */

PUBLIC ATTR_NOINLINE void *(KCALL _mall_getattrib)(void *mallptr, int attrib) { DEF_SETUP(setup); return debug_getattrib(&setup,mallptr,attrib); }
PUBLIC ATTR_NOINLINE ssize_t (KCALL _mall_traceback)(void *mallptr, __ptbwalker callback,
                                                     void *closure)           { DEF_SETUP(setup); return debug_traceback(&setup,mallptr,callback,closure); }
PUBLIC ATTR_NOINLINE ssize_t (KCALL _mall_enum)(struct instance *inst, void *checkpoint, ssize_t (KCALL *callback)(void *__restrict mallptr, void *closure),
                                                void *closure)                { DEF_SETUP(setup); return debug_enum(&setup,inst,checkpoint,callback,closure); }
PUBLIC ATTR_NOINLINE void (KCALL _mall_printleaks)(struct instance *inst)     { DEF_SETUP(setup); debug_printleaks(&setup,inst); }
PUBLIC ATTR_NOINLINE void (KCALL _mall_validate)(struct instance *inst)       { DEF_SETUP(setup); debug_validate(&setup,inst); }
PUBLIC ATTR_NOINLINE void *(KCALL _mall_untrack)(void *mallptr)               { DEF_SETUP(setup); return debug_setflag(&setup,mallptr,MPTRFLAG_UNTRACKED); }
PUBLIC ATTR_NOINLINE void *(KCALL _mall_nofree)(void *mallptr)                { DEF_SETUP(setup); return debug_setflag(&setup,mallptr,MPTRFLAG_NOFREE); }
PUBLIC ATTR_NOINLINE void *(KCALL _mall_global)(void *mallptr)                { DEF_SETUP(setup); return debug_setflag(&setup,mallptr,MPTRFLAG_GLOBAL); }
#undef DEF_SETUP

#define DEF_SETUP(name) \
  struct dsetup name; \
  name.s_info.i_file = __file; \
  name.s_info.i_line = __line; \
  name.s_info.i_func = __func; \
  name.s_info.i_inst = __inst; \
  GET_EBP(name.s_tbebp)

PUBLIC ATTR_NOINLINE void *(KCALL _kmalloc_d)(size_t size, gfp_t flags, DEBUGINFO)                                                { DEF_SETUP(setup); return debug_malloc(&setup,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL _krealloc_d)(void *ptr, size_t size, gfp_t flags, DEBUGINFO)                                    { DEF_SETUP(setup); return debug_realloc(&setup,ptr,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL _kmemalign_d)(size_t alignment, size_t size, gfp_t flags, DEBUGINFO)                            { DEF_SETUP(setup); return debug_memalign(&setup,alignment,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL _krealign_d)(void *ptr, size_t alignment, size_t size, gfp_t flags, DEBUGINFO)                  { DEF_SETUP(setup); return debug_realign(&setup,ptr,alignment,size,flags); }
PUBLIC ATTR_NOINLINE void (KCALL _kfree_d)(void *ptr, DEBUGINFO)                                                                  { DEF_SETUP(setup);        debug_free(&setup,ptr,GFP_NORMAL); }
PUBLIC ATTR_NOINLINE void (KCALL _kffree_d)(void *ptr, gfp_t flags, DEBUGINFO)                                                    { DEF_SETUP(setup);        debug_free(&setup,ptr,flags); }
PUBLIC ATTR_NOINLINE size_t (KCALL _kmalloc_usable_size_d)(void *ptr, DEBUGINFO)                                                  { DEF_SETUP(setup); return debug_malloc_usable_size(&setup,ptr); }
PUBLIC ATTR_NOINLINE gfp_t (KCALL _kmalloc_flags_d)(void *ptr, DEBUGINFO)                                                         { DEF_SETUP(setup); return debug_malloc_flags(&setup,ptr); }
PUBLIC ATTR_NOINLINE void *(KCALL _kmemdup_d)(void const *__restrict ptr, size_t size, gfp_t flags, DEBUGINFO)                    { DEF_SETUP(setup); return debug_memdup(&setup,ptr,size,flags); }
PUBLIC ATTR_NOINLINE void *(KCALL _kmemadup_d)(void const *__restrict ptr, size_t alignment, size_t size, gfp_t flags, DEBUGINFO) { DEF_SETUP(setup); return debug_memadup(&setup,ptr,alignment,size,flags); }
PUBLIC ATTR_NOINLINE int (KCALL _kmallopt_d)(int parameter_number, int parameter_value, gfp_t flags, DEBUGINFO)                   { DEF_SETUP(setup); return debug_mallopt(&setup,parameter_number,parameter_value,flags); }

PUBLIC ATTR_NOINLINE int (KCALL _mallopt_d)(int parameter_number, int parameter_value, DEBUGINFO)                     { DEF_SETUP(setup); return debug_mallopt(&setup,parameter_number,parameter_value,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _malloc_d)(size_t n_bytes, DEBUGINFO)                                               { DEF_SETUP(setup); return debug_malloc(&setup,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _calloc_d)(size_t count, size_t n_bytes, DEBUGINFO)                                 { DEF_SETUP(setup); return debug_malloc(&setup,count*n_bytes,__GFP_MALLOC|GFP_CALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _memalign_d)(size_t alignment, size_t n_bytes, DEBUGINFO)                           { DEF_SETUP(setup); return debug_memalign(&setup,alignment,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _realloc_d)(void *__restrict mallptr, size_t n_bytes, DEBUGINFO)                    { DEF_SETUP(setup); return debug_realloc(&setup,mallptr,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _realloc_in_place_d)(void *__restrict mallptr, size_t n_bytes, DEBUGINFO)           { DEF_SETUP(setup); return debug_realloc(&setup,mallptr,n_bytes,__GFP_MALLOC|GFP_NOMOVE); }
PUBLIC ATTR_NOINLINE void *(KCALL _valloc_d)(size_t n_bytes, DEBUGINFO)                                               { DEF_SETUP(setup); return debug_memalign(&setup,PAGESIZE,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _pvalloc_d)(size_t n_bytes, DEBUGINFO)                                              { DEF_SETUP(setup); return debug_memalign(&setup,PAGESIZE,(n_bytes+PAGESIZE-1)&~(PAGESIZE-1),__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _memdup_d)(void const *__restrict ptr, size_t n_bytes, DEBUGINFO)                   { DEF_SETUP(setup); return debug_memdup(&setup,ptr,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE int (KCALL _posix_memalign_d)(void **__restrict pp, size_t alignment, size_t n_bytes, DEBUGINFO) { DEF_SETUP(setup); return debug_posix_memalign(&setup,pp,alignment,n_bytes,__GFP_MALLOC); }
#ifdef CONFIG_HAVE_STRDUP_IN_KERNEL
PUBLIC ATTR_NOINLINE char *(KCALL _strdup_d)(char const *__restrict str, DEBUGINFO)                                   { DEF_SETUP(setup); return debug_strdup(&setup,str,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(KCALL _strndup_d)(char const *__restrict str, size_t max_chars, DEBUGINFO)                { DEF_SETUP(setup); return debug_strndup(&setup,str,max_chars,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE void *(KCALL _memcdup_d)(void const *__restrict ptr, int needle, size_t n_bytes, DEBUGINFO)      { DEF_SETUP(setup); return debug_memcdup(&setup,ptr,needle,n_bytes,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(KCALL _vstrdupf_d)(char const *__restrict format, va_list args, DEBUGINFO)                { DEF_SETUP(setup); return debug_vstrdupf(&setup,format,args,__GFP_MALLOC); }
PUBLIC ATTR_NOINLINE char *(ATTR_CDECL _strdupf_d)(DEBUGINFO, char const *__restrict format, ...)                     { va_list args; char *result; DEF_SETUP(setup); va_start(args,format); result = debug_vstrdupf(&setup,format,args,__GFP_MALLOC); va_end(args); return result; }
#endif /* CONFIG_HAVE_STRDUP_IN_KERNEL */

PUBLIC ATTR_NOINLINE void *(KCALL _mall_getattrib_d)(void *mallptr, int attrib, DEBUGINFO) { DEF_SETUP(setup); return debug_getattrib(&setup,mallptr,attrib); }
PUBLIC ATTR_NOINLINE ssize_t (KCALL _mall_traceback_d)(void *mallptr, __ptbwalker callback,
                                                       void *closure, DEBUGINFO)           { DEF_SETUP(setup); return debug_traceback(&setup,mallptr,callback,closure); }
PUBLIC ATTR_NOINLINE ssize_t (KCALL _mall_enum_d)(struct instance *inst, void *checkpoint, ssize_t (KCALL *callback)(void *__restrict mallptr, void *closure),
                                                  void *closure, DEBUGINFO)                { DEF_SETUP(setup); return debug_enum(&setup,inst,checkpoint,callback,closure); }
PUBLIC ATTR_NOINLINE void (KCALL _mall_printleaks_d)(struct instance *inst, DEBUGINFO)     { DEF_SETUP(setup); debug_printleaks(&setup,inst); }
PUBLIC ATTR_NOINLINE void (KCALL _mall_validate_d)(struct instance *inst, DEBUGINFO)       { DEF_SETUP(setup); debug_validate(&setup,inst); }
PUBLIC ATTR_NOINLINE void *(KCALL _mall_untrack_d)(void *mallptr, DEBUGINFO)               { DEF_SETUP(setup); return debug_setflag(&setup,mallptr,MPTRFLAG_UNTRACKED); }
PUBLIC ATTR_NOINLINE void *(KCALL _mall_nofree_d)(void *mallptr, DEBUGINFO)                { DEF_SETUP(setup); return debug_setflag(&setup,mallptr,MPTRFLAG_NOFREE); }
PUBLIC ATTR_NOINLINE void *(KCALL _mall_global_d)(void *mallptr, DEBUGINFO)                { DEF_SETUP(setup); return debug_setflag(&setup,mallptr,MPTRFLAG_GLOBAL); }
#undef DEF_SETUP
#endif /* MALLOC_DEBUG_API */

DECL_END

#pragma GCC diagnostic pop
#endif /* !GUARD_KERNEL_MEMORY_MALLOC_C */
