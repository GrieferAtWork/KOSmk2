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
#ifndef GUARD_INCLUDE_KERNEL_MMAN_H
#define GUARD_INCLUDE_KERNEL_MMAN_H 1

/* Kernel memory management:
 *       malloc.h: Heap memory
 *       memory.h: Physical memory
 * THIS: mman.h:   Virtual memory
 */

#include <hybrid/compiler.h>
#include <hybrid/list/atree.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/sync/atomic-rwptr.h>
#include <kernel/memory.h>
#include <kernel/mswap.h>
#include <kernel/paging.h>
#include <kernel/malloc.h>
#include <sched/types.h>
#include <sched/percpu.h>
#include <hybrid/byteorder.h>
#include <malloc.h>
#include <stdalign.h>
#include <sync/rwlock.h>
#include <sync/owner-rwlock.h>
#include <sync/sig.h>

DECL_BEGIN

/* Paging-based memory manager, fully capable of load-on-read
 * and copy-on-write, as well as making use of SWAP memory. */

#ifndef __raddr_t_defined
#define __raddr_t_defined 1
typedef PAGE_ALIGNED uintptr_t raddr_t; /* Region address. */
#endif /* !__raddr_t_defined */
typedef PAGE_ALIGNED uintptr_t rsize_t; /* Region size. */

struct file;
struct instance;
struct mbranch;
struct mfutex;
struct mman;
struct mman_maps;
struct module;
struct mregion;
struct mregion_part;
struct mscatter;
struct stack;
struct task;
union  mregion_cinit;

ATTR_ALIGNED(ATOMIC_RWPTR_ALIGN)
struct mfutex {
 atomic_rwptr_t   *f_pself;  /*< [TYPE(struct mfutex)][0..1][lock(f_next.fp_lock)] Self-pointer (or NULL for unlinked futex objects; aka. when the region was deleted). */
 atomic_rwptr_t    f_next;   /*< [TYPE(struct mfutex)][sort(ASCENDING(f_addr))] Pointer to the next futex. */
 ATOMIC_DATA ref_t f_refcnt; /*< Reference counter for this futex. */
 raddr_t           f_addr;   /*< In-region address of this futex. */
 struct sig        f_sig;    /*< Signal used to implement scheduling with this futex. */
};

/* Locking helpers for futex objects. */
#define mfutex_reading(x)     sig_reading(&(x)->f_sig)
#define mfutex_writing(x)     sig_writing(&(x)->f_sig)
#define mfutex_tryread(x)     sig_tryread(&(x)->f_sig)
#define mfutex_trywrite(x)    sig_trywrite(&(x)->f_sig)
#define mfutex_tryupgrade(x)  sig_tryupgrade(&(x)->f_sig)
#define mfutex_read(x)        sig_read(&(x)->f_sig)
#define mfutex_write(x)       sig_write(&(x)->f_sig)
#define mfutex_upgrade(x)     sig_upgrade(&(x)->f_sig)
#define mfutex_downgrade(x)   sig_downgrade(&(x)->f_sig)
#define mfutex_endread(x)     sig_endread(&(x)->f_sig)
#define mfutex_endwrite(x)    sig_endwrite(&(x)->f_sig)

/* NOTE: Futex objects are always allocated in shared memory, so-as to ensure
 *       that shared access is possible without switching page directories.
 *       This is always possible, as no code used to allocate/map memory
 *       makes use of them. */
#define mfutex_new()            ((struct mfutex *)kmemalign(ATOMIC_RWPTR_ALIGN,sizeof(struct mfutex),GFP_SHARED))
#define MFUTEX_TRYINCREF(self)    ATOMIC_INCIFNONZERO((self)->f_refcnt)
#define MFUTEX_INCREF(self)    (void)(ATOMIC_FETCHINC((self)->f_refcnt))
#define MFUTEX_DECREF(self)    (void)(ATOMIC_DECFETCH((self)->f_refcnt) || (mfutex_destroy(self),0))
FUNDEF SAFE void KCALL mfutex_destroy(struct mfutex *__restrict self);

/* Get/Allocate futex objects.
 * @return: * :   A new reference to the futex associated with `addr'.
 * @return: NULL: Failed to find/allocate a futex associated with the given `addr'. */
FUNDEF SAFE REF struct mfutex *KCALL mfutexptr_get(atomic_rwptr_t *__restrict self, raddr_t addr);
FUNDEF SAFE REF struct mfutex *KCALL mfutexptr_new(atomic_rwptr_t *__restrict self, raddr_t addr);



#define MPART_STATE_MISSING 0 /*< Memory hasn't been allocated/initialized (NOTE: Mandatory for guard regions). */
#define MPART_STATE_INCORE  1 /*< Memory is loaded into the core (NOTE: Mandatory for physical regions). */
#define MPART_STATE_INSWAP  2 /*< Memory has been off-loaded into swap. */
#define MPART_STATE_UNKNOWN 3 /*< The memory state isn't managed by the mman, or not relevant (usually used alongside `MREGION_TYPE_RESERVED').
                               *  WARNING: This state may be overwritten when a region type other than `MREGION_TYPE_RESERVED' is used! */
/*      MPART_STATE_...     4  */

#define MPART_FLAG_NONE  0x00 /* ... */
#define MPART_FLAG_CHNG  0x01 /*< The part is known to have changed
                               *  That is: some mman unmapped the part while the associated
                               *           page directory entry was marked as dirty.
                               *  NOTE: This flag is set when a page is copied for COW.
                               *  NOTE: This flag is used to implement 'msync()'
                               *  WARNING: Only trust this flag when 'MREGION_INIT_WRITETHROUGH' is used. */
#define MPART_FLAG_KEEP  0x02 /*< Do not free this part once all mappings to it disappear.
                               *  Instead, keep it around until the surrounding region is destroyed.
                               *  This flag is set for parts 
                               */
/*      MPART_FLAG_...   0x04  */

struct mregion_part {
 /* Reference counting/state tracking of memory region parts.
  * NOTES:
  *  - Adjacent parts with compatible settings are merged.
  *  - A part ends where the next starts, or when no
  *    successor is present, where the region ends.
  *  - All parts of any region are _ALWAYS_ present.
  *  - Region parts are never empty.
  */
 LIST_NODE(struct mregion_part)
                           mt_chain;  /*< [lock(:mr_plock)][sort(ASCENDING(mt_start))] Chain of more region part. */
 raddr_t                   mt_start;  /*< [lock(:mr_plock)] Starting address of this part. */
 ref_t                     mt_refcnt; /*< [lock(:mr_plock)] Amount of branches that map this part.
                                       *   NOTE: This field may be ZERO(0), in which case this part isn't
                                       *         mapped by anyone. Though this does not mean that it has no
                                       *         memory or stick associated, but simply that any associated data
                                       *         may be freed at any point in time (Unless 'MPART_FLAG_KEEP' is set).
                                       *         aka.: 'mt_state == MPART_STATE_MISSING'. */
 u8                        mt_state;  /*< [lock(:mr_plock)] The current state of this region part.
                                       *   NOTE: Always 'MPART_STATE_INCORE' for 'MREGION_TYPE_PHYSICAL' regions.
                                       *   NOTE: Usually 'MPART_STATE_MISSING' when 'mt_refcnt' is ZERO(0). */
 u8                        mt_flags;  /*< [lock(:mr_plock)] Flags associated with the current region part. */
 s16                       mt_locked; /*< [lock(:mr_plock)] Recursion counter for locking/unlocking memory for in-core usage (mlock() / munlock()).
                                       *   NOTE: Locking is enabled when this field is '> 0', otherwise it is disabled.
                                       *   NOTE: This field is ignored and considered equal to '1' for 'MREGION_TYPE_PHYSICAL' typed regions.
                                       *   NOTE: This field being non-zero does _NOT_ require mt_state to be 'MPART_STATE_INCORE'!
                                       *         It merely means that once loaded, the part will stay in memory until it is unlocked at a later time. */
union{ struct mscatter     mt_memory; /*< [lock(:mr_plock)][valid_if(mt_state == MPART_STATE_INCORE)] Physical memory scatter chain for in-core data. */
       struct mswap_ticket mt_stick;  /*< [lock(:mr_plock)][valid_if(mt_state == MPART_STATE_INSWAP)] Swap ticket describing memory data. */};
};
#define MREGION_PART_BEGIN(self)       ((self)->mt_start)
#define MREGION_PART_END(self,region)  ((self)->mt_chain.le_next ? (self)->mt_chain.le_next->mt_start : (region)->mr_size)
#define MREGION_PART_SIZE(self,region) (MREGION_PART_END(self,region)-MREGION_PART_BEGIN(self))
#define MREGION_PART_ISLOCKED(self)    ((self)->mt_locked > 0)


/* Memory region types (used to implement guard regions) */
#define MREGION_TYPE_MEM      0 /*< Default region type used for mapping memory (NOTE: _MUST_ be ZERO(0)). */
#define MREGION_TYPE_PHYSICAL 1 /*< A special kind of memory region that maps an explicit physical memory region.
                                 *  NOTE: This kind of region cannot be locked, as it is always loaded, with
                                 *        the addition that all parts are always locked + 'MPART_STATE_INCORE',
                                 *        and associated memory scatter chains are not freed.
                                 *  NOTE: This is also the mapping used to describe the kernel core.
                                 *  >> This kind of region is meant for safely mapping device memory in userspace.
                                 *  WARNING: Guard ranges longer than 1 page may signal 'MNOTIFY_GUARD_INUSE'
                                 *           incorrectly when attempting to relocate them self after being
                                 *           split (1 page long regions cannot be split). */
#define MREGION_TYPE_LOGUARD  2 /*< A guard region that will be replaced with a 'MREGION_TYPE_MEM'
                                 *  copy containing the same memory mappings, before re-mapping itself
                                 *  'mr_size' below its previous position, so long as that address
                                 *  doesn't underflow and sufficient funds remain available.
                                 *  NOTE: If the region is set-up to use 'MREGION_INIT_FILE' initialization,
                                 *        the 'mri_start' will be decremented by 'mr_size' in the new mapping.
                                 *        In the even that decrementing 'mri_start' underflows, the region's
                                 *        type is changed to 'MREGION_INIT_BYTE'. */
#define MREGION_TYPE_HIGUARD  3 /*< Same as 'MREGION_TYPE_LOGUARD', but remap 'mr_size' above and increment 'mri_start'. */
#define MREGION_TYPE_RESERVED 4 /*< Used internally: A reserved memory region that may or may not contain data, yet may not be un-mapped,
                                 *  or re-mapped in the associated page directory (it may still be deleted/moved in the mman).
                                 *  Internally, this region type is used to reserve memory for virtual address space used
                                 *  for page directory self-mappings. */
/*      MREGION_TYPE_...      6  */
/*      MREGION_TYPE_...      8  */
#define MREGION_TYPE_ISGUARD(x) ((x)&2)



#define MREGION_INIT_ISFILE(x)       (MREGION_INIT_TYPE(x) == MREGION_INIT_TYPE(MREGION_INIT_FILE))
#define MREGION_INIT_ISUSER(x)       (MREGION_INIT_TYPE(x) == MREGION_INIT_TYPE(MREGION_INIT_USER))
#define MREGION_INIT_ISUSER_ASYNC(x) (MREGION_INIT_TYPE(x) == MREGION_INIT_TYPE(MREGION_INIT_USER_ASYNC))
#define MREGION_INIT_TYPE(x)         ((x)&MREGION_INIT_TYPEMASK)
#define MREGION_INIT_FLAG(x)         ((x)&MREGION_INIT_FLAGMASK)
#define MREGION_INIT_TYPEMASK     0x0f /*< Mask for mregion init-type. */
#define MREGION_INIT_FLAGMASK     0xf0 /*< Mask for mregion init-flags. */

#define MREGION_INIT_RAND         0x00 /*< Don't initialize memory mappings, keeping original memory contents. (NOTE: _MUST_ be ZERO(0)). */
#define MREGION_INIT_ZERO         0x01 /*< Allocate memory using the 'PAGEATTR_ZERO' attribute. */
#define MREGION_INIT_BYTE         0x02 /*< Initialize memory with a fixed byte (don't use for '0x00'; use 'MINIT_TYPE_ZERO' instead). */
#define MREGION_INIT_FILE         0x43 /*< Initialize with data read from a file. */
#define MREGION_INIT_WFILE        0x83 /*< Same as 'MREGION_INIT_FILE', but write changes back to the file before freeing data. */
#define MREGION_INIT_USER         0x44 /*< Execute a user-defined callback for initialization.
                                        *  WARNING: This callback must be locked in-core within the kernel, and must not
                                        *           be reference any memory not loaded within the kernel mman either! */
#define MREGION_INIT_WUSER        0x84 /*< Same as 'MREGION_INIT_USER', but enable write-through semantics.
                                        *  When this is set, the 'mregion' should be kept around until data is no longer needed. */
#define MREGION_INIT_REREAND      0x05 /*< Initialize memory with random values, generated by rand(). */
/*      MREGION_INIT_...          0x07  */
/*      MREGION_INIT_...          0x0f  */
/*      MREGION_INIT_...          0x10  */
#define MREGION_INIT_READTHROUGH  0x40 /*< The region must ensure that changes to initialized data are re-loaded when accessed a second time.
                                        *  Required to ensure that a single mregion can be cached for any file-mapping, and when mapped
                                        *  a second time (even after parts had already changed), all parts that weren't changed will
                                        *  remain mapped the same will be shared, while all parts that were changed will be re-loaded
                                        *  using region initialization upon first access.
                                        *  NOTE: Read-through is always disabled when regions are mapped
                                        *        as 'PROT_SHARED'. - But sadly this brings alongside a
                                        *        minor consequence when looking at how the kernel implements
                                        *        its linker:
                                        *     >> Any kind of code running in KOS could mprotect() parts
                                        *        of its executable image as 'PROT_WRITE|PROT_SHARED', at
                                        *        which point any changes to data will be mirrored _EVERYWHERE_.
                                        *        With that in mind, any other running instance will be
                                        *        affected _immediatly_, and changes will remain until the
                                        *        linker decides to delete the executable from its cache.
                                        *       (Which happens at an undefined, random point in time)
                                        */
#define MREGION_INIT_WRITETHROUGH 0x80 /*< The region must track when parts have changed.
                                        *  When used with user-callbacks, 'MREGION_INITFUN_MODE_SAVE' is signaled. */


#define MREGION_GFUNDS_INFINITE ((u16)-1)



/* Custom mregion initialization.
 * NOTE: This user-defined init functions are executed when the 'mr_parts'
 *       lock of the associated region isn't held, meaning called code
 *       must neither be present within the core, nor is required not to
 *       access data that would cause additional page-faults!
 * HINT: 'region_size == SUM_OF(data->[m_next->...]m_size)'
 * WARNING: The implementor of this function must implement
 *          their own synchronization, as this callback may
 *          be executed asynchronously, and at any time.
 * WARNING: Due to race conditions, this function may be called more than
 *          once for the same area of memory, with no way of determining
 *          which call will be the last (although you should still optimize
 *          under the assumption that you're function will only be called once
 *          for any given region described by 'region_addr...+=region_size')
 * @return: -EOK:       Successfully initialized memory.
 * @return: -ENODATA:   Cannot initialize memory (internally interpreted by skipped the associated part)
 * @return: E_ISERR(*): Failed to initialize memory for some reason (Error is propagated & returned by 'mman_mcore_unlocked')
 */
typedef errno_t (KCALL *mregion_initfun)(u32 mode, void *closure,
                                         struct mregion *__restrict region,
                                         struct mscatter *__restrict data,
                                         PAGE_ALIGNED raddr_t region_addr,
                                         PAGE_ALIGNED rsize_t region_size);
#define MREGION_INITFUN_MODE_LOAD 0 /*< Default behavior: Initialize the given 'data' however you please. */
#define MREGION_INITFUN_MODE_SAVE 1 /*< Only called when 'MREGION_INIT_WRITETHROUGH' is enabled: May be implemented to write changes. */
#define MREGION_INITFUN_MODE_FINI 2 /*< Called just before the region is destroyed.
                                     *  WARNING: The return value of this message is ignored.
                                     *  WARNING: Despite its '__restrict' tag, the 'data' argument for this specific message is NULL.
                                     *  HINT: 'region_addr == 0' and 'region_size == region->mr_size'
                                     *  The main purpose for this notification is to implement reference
                                     *  counting within using a custom structure allocated in 'closure'. */

union mregion_cinit {struct{
 mregion_initfun                mri_ufunc; /*< [const][1..1][valid_if(MREGION_INIT_ISUSER(:mr_init))] User-defined memory initialization callback. */
 void                          *mri_uclosure; /*< [const][?..?] Closure parameter used when calling 'mri_ufunc' */
 byte_t                         mri_udata[((sizeof(u32)+sizeof(pos_t)+sizeof(size_t))-
                                            sizeof(mregion_initfun))]; /* Padding user-data. */
};struct{
 /* Initialization info for memory regions. */
 u32                            mri_byte;  /*< [const][valid_if(:mr_init&MREGION_INIT_TYPEMASK != MREGION_INIT_RAND &&
                                            *                   :mr_init&MREGION_INIT_TYPEMASK != MREGION_INIT_ZERO)]
                                            *       Initialization DWORD used to fill newly allocated memory.
                                            * NOTE: set using 'mi_byte = byte*0x01010101;'
                                            * NOTE: This byte is also used for initialization of file data past its end. */
 REF struct file               *mri_file;  /*< [const][valid_if(MREGION_INIT_ISFILE(:mr_init))][1..1] Underlying initializer file that is read from during load-on-read. */
 /* NOTE: File mappings are initialized as follows:
  *                 
  *  FILE: abcdefghijklmnopqrtuvwxyzABCDEFGHIJKLMNOPQRTUVWXYZ...
  *        |         |              |
  *        0 mri_start              mri_start+mri_size
  *                  |              |
  *                  +-+            +-+
  *                    |              |
  *  MEM:  XXXXXXXXXXXXklmnopqrtuvwxyzXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  *        |           |              |                                   |
  *        0   mri_begin              mri_begin+mri_size            mr_size
  * X: Initialized using 'mri_byte' */
 raddr_t                        mri_begin; /*< [const][valid_if(MREGION_INIT_ISFILE(:mr_init))][<= :mr_size] Offset into the region, where data from the file starts (memory before then is initialized using 'mri_byte'). */
 pos_t                          mri_start; /*< [const][valid_if(MREGION_INIT_ISFILE(:mr_init))] Offset within 'mri_file' to the start of initializer data. */
 size_t                         mri_size;  /*< [const][valid_if(MREGION_INIT_ISFILE(:mr_init))][<= :mr_size] Max amount of bytes to read/write to/from 'mri_file'. */
};};

struct mregion {
 /* Memory region. - Can be split, merged and shared between multiple processes. */
 ATOMIC_DATA ref_t              mr_refcnt; /*< Reference counter (NOTE: May only be set to ZERO(0) when no more pages are in use).
                                            *  NOTE: No part reference counter may be larger that this! */
 u8                             mr_type;   /*< [const] The type of region (One of 'MREGION_TYPE_*') */
 u8                             mr_init;   /*< [const] The type of initialization (One of 'MREGION_INIT_**') */
 u16                            mr_gfunds; /*< [lock(mr_plock)][valid_if(MREGION_TYPE_ISGUARD(mr_type))] Guard funding.
                                            *   Whenever a guard region is replaced, the old region is updated
                                            *   with 'mr_gfunds = mr_gfunds-1'. When 'mr_gfunds == 0' prior to
                                            *   creation of a new region, the new region isn't created, and
                                            *  (if set) 'mb_notify' of the associated branch is executed
                                            *   with 'MNOTIFY_GUARD_END'
                                            *   NOTE: Set to 'MREGION_GFUNDS_INFINITE' to provide infinite funding. */
 union mregion_cinit            mr_setup;  /*< Setup/teardown special-handling information. */
 PAGE_ALIGNED rsize_t           mr_size;   /*< [const] The size of this region (in bytes). */
 atomic_rwptr_t                 mr_futex;  /*< [TYPE(struct mfutex)] Known futex objects within this region. */
 rwlock_t                       mr_plock;  /*< Lock used for accessing region parts. */
 LIST_HEAD(struct mregion_part) mr_parts;  /*< [lock(mr_plock)][1..1] Chain of region parts containing information about different sub-portions of this region. */
 struct mregion_part            mr_part0;  /*< [lock(mr_plock)] Initially allocated region part (may later be re-used for any part). */
 LIST_NODE(struct mregion)      mr_global; /*< [lock(INTERNAL(::mregion_chain_lock))][valid_if(mr_type == MREGION_TYPE_MEM)]
                                            *  Global chain of memory regions (tracked to move pages to swap when 'mman_swapmem()' is called) */
};
#define MREGION_FOREACH_PART(part,self) \
  LIST_FOREACH(part,(self)->mr_parts,mt_chain)


#define MREGION_TRYINCREF(self) ATOMIC_INCIFNONZERO((self)->mr_refcnt)
#define MREGION_INCREF(self) (void)(ATOMIC_FETCHINC((self)->mr_refcnt))
#define MREGION_DECREF(self) (void)(ATOMIC_DECFETCH((self)->mr_refcnt) || (mregion_destroy(self),0))
FUNDEF void KCALL mregion_destroy(struct mregion *__restrict self);

/* Create a new, fully initialized memory region given specific arguments.
 * @param: mode: [mregion_new_anon]: Set of 'MREGION_ANON_*'
 * @return: * :         A reference to the allocated memory region.
 * @return: E_ISERR(*): Failed to create the new region for some reason. */
FUNDEF REF struct mregion *KCALL mregion_new_phys(gfp_t region_gfp, PHYS ppage_t addr, PAGE_ALIGNED size_t n_bytes);
FUNDEF REF struct mregion *KCALL mregion_new_anon(gfp_t region_gfp, PAGE_ALIGNED size_t n_bytes, u32 mode);
#define MREGION_ANON_DYNAMIC      0x0000 /*< Allocate dynamic memory. */
#define MREGION_ANON_CALLOC       0x0001 /*< Use zero-initialized memory for the region (Unless set, use random initialization instead). */
#define MREGION_ANON_LOCKED       0x0002 /*< Initialize allocated parts as locked in-core (that is: Keep loaded once allocated during the first fault). */
#define MREGION_ANON_PREFAULT     0x0004 /*< Pre-allocate physical memory for the entirety of the region (Unless 'MREGION_ANON_LOCKED' is set, memory may be deallocated at any point). */
#define MREGION_ANON_TRY_PREFAULT 0x8000 /*< Used alongside 'MREGION_ANON_PREFAULT': Don't fail (returning NULL) if memory cannot be pre-faulted. - Leave as unallocated instead. */

/* Allocate and pre-initialize a new memory region.
 * NOTE: For the pre-initialization ensured by this, look at 'mregion_setup()'
 * @return: * :   The initial reference to a newly allocated region.
 * @return: NULL: Not enough available memory. */
#define mregion_new(gfp) mregion_cinit((struct mregion *)kcalloc(sizeof(struct mregion),gfp))
FUNDEF struct mregion *KCALL mregion_cinit(struct mregion *self);

/* Perform final setup on the given memory region,
 * publishing it to the global list of regions and
 * thereby enabling swap control.
 * WARNING: 'Preset' relies on the fact that the pointer passed
 *          to 'mregion_cinit()' is already zero-initialized.
 * The caller must initialize:
 *  - mr_refcnt   (Preset to ONE(1) by mregion_cinit)
 *  - mr_type     (Preset by 'MREGION_TYPE_MEM' by mregion_cinit)
 *  - mr_init     (Preset to 'MREGION_INIT_RAND' by mregion_cinit)
 *  - mr_gfunds   (Preset to ZERO(0) by mregion_cinit)
 *  - mr_setup    (Preset to all ZEROes by mregion_cinit)
 *  - mr_size
 *  - mr_futex    (Initialized by mregion_cinit)
 *  - mr_plock    (Initialized by mregion_cinit)
 *  - mr_parts    (Initialized by mregion_cinit) */
FUNDEF void KCALL mregion_setup(struct mregion *__restrict self);


/* Read data from a given memory-region, at the specified offset `addr'.
 * NOTES:
 *   - Attempting to read out-of-bounds will return ZERO(0).
 *   - Attempting to read past the end will return the max possible read.
 *   - Attempting to read unmapped/unallocated memory will
 *     simply use that initializer for load the given buffer
 *    (such as for fixed-integer initialization, or a file-mapping)
 * @return: * :         The actual amount of bytes read.
 * @return: -EFAULT:    The given user-space buffer is faulty.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): Failed to read region data for some reason
 *                     (can happen for non-loaded file-mappings) */
FUNDEF SAFE ssize_t KCALL
mregion_read(struct mregion *__restrict self,
             USER void *buf, size_t bufsize, raddr_t addr);
/* Same as 'mregion_read', but the caller must be holding a write-(Yes a write)lock on 'self'. */
FUNDEF SAFE ssize_t KCALL
mregion_read_unlocked(struct mregion *__restrict self,
                      USER void *buf, size_t bufsize, raddr_t addr);

/* Write data to a given memory region, at a specified offset `addr'.
 * NOTES:
 *   - Attempting to write out-of-bounds will return ZERO(0).
 *   - Attempting to write unmapped memory ('mt_refcnt == 0') will
 *     allocate affected parts at set the 'MPART_FLAG_KEEP' flag.
 *   - Attempting to write past the end will return the max possible write.
 *   - Attempting to write unallocated memory will cause that memory
 *     to become allocated, as well as cause data portions not defined
 *     by the specified user-space buffer to be initialized.
 * @return: * :         The actual amount of bytes written.
 * @return: -EFAULT:    The given user-space buffer is faulty.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): Failed to write region data for some reason
 *                     (can happen for non-loaded file-mappings, or custom initializers) */
FUNDEF SAFE ssize_t KCALL
mregion_write(struct mregion *__restrict self,
              USER void const *buf, size_t bufsize, raddr_t addr);
/* Same as 'mregion_write', but the caller must be holding a write-lock on 'self'. */
FUNDEF SAFE ssize_t KCALL
mregion_write_unlocked(struct mregion *__restrict self,
                       USER void const *buf, size_t bufsize, raddr_t addr);
/* Unload all unmapped, but still allocated parts with the given address range.
 * Such parts may have been created through calls to 'mregion_write_unlocked',
 * when the associated part was not mapped in any real VM.
 * @return: * :     The amount of bytes unloaded (Sum of physical and swap memory).
 * @return: -EINTR: The calling thread was interrupted. */
FUNDEF SAFE ssize_t KCALL
mregion_unload(struct mregion *__restrict self,
               raddr_t addr, rsize_t size);
/* Same as 'mregion_unload', but the caller must be holding a write-lock on 'self'. */
FUNDEF SAFE ssize_t KCALL
mregion_unload_unlocked(struct mregion *__restrict self,
                        raddr_t addr, rsize_t size);












/* A user-defined notification function executed upon
 * special events related to per-mman mappings.
 * @param: type:    One of 'MNOTIFY_*'
 * @param: closure: The user-defined closure pointer.
 * @param: mm:      The memory manager associated with the address range.
 *                  NOTE: Only valid for notifications marked as [M]
 * @param: addr:    The virtual address the branch. (Only defined for callbacks marked as [A])
 * @param: size:    The size (in bytes) of the branch in question. (Only defined for callbacks marked as [A])
 * @return: * : An error code, or -EOK.
 * NOTE: Any notification that is not handled should return '-EOK' (aka. ZERO(0))
 * NOTE: Return values are ignores for all events not marked as [E]
 */
typedef ssize_t (KCALL *mbranch_notity)(unsigned int type, void *__restrict closure,
                                        struct mman *mm, ppage_t addr, size_t size);
#define MNOTIFY_HASE(x) ((((x)&0770) != 0 || (x) == MNOTIFY_INCREF) && \
                          ((x)&0770) != 0020 && (x) != MNOTIFY_DELETE)
#define MNOTIFY_HASA(x) ((x) > MNOTIFY_DECREF)
#define MNOTIFY_HASM(x) ((x) > MNOTIFY_DECREF && (x) != MNOTIFY_DELETE)

#define MNOTIFY_INCREF         0000 /*< [E] The notification+closure pair exists at an additional location. */
#define MNOTIFY_DECREF         0001 /*<     The reverse of 'MNOTIFY_INCREF', called when a notification branch is deleted. */
#ifdef CONFIG_DEBUG
#define MNOTIFY_ASSERT         0002 /*< [M][A] Optionally assert consistency of the branch. */
#endif
#define MNOTIFY_LOADED         0010 /*< [M][A][E] Executed after a given address range has been loaded into the core.
                                     *   HINT: This notifier can be used to implement on-load relocations. */
#define MNOTIFY_GUARD_ALLOCATE 0020 /*< [M][A] A new guard region was allocated.
                                     *         For stacks, this notification should update the associated stack limits. */
#define MNOTIFY_GUARD_INUSE    0021 /*< [M][A] The guard region was allocated, but a new guard could not be
                                     *         created because the virtual address space is already in use.
                                     *   For stacks, this notification should trigger a STACK_OVERFLOW/SIGSEGV.
                                     *   NOTE: After this notification has been sent, the guard page will disappear. */
#define MNOTIFY_GUARD_END      0022 /*< [M][A] No new guard page was allocated because the page ran out of funds (aka. grew too large). */
/* WARNING: Remember that branches are often split into multiple parts, meaning
 *          that the following events may be called multiple times for non-overlapping
 *          parts of the same original address range, or out-of-order! */
#define MNOTIFY_EXTRACT        0030 /*< [M][A][E] The branch was extracted from 'addr...+=size' using 'mman_mextract_unlocked'. (Eventually followed by 'MNOTIFY_RESTORE' or 'MNOTIFY_DELETE') */
#define MNOTIFY_CLONE          0031 /*< [M][A][E] Signaled instead of 'MNOTIFY_EXTRACT' when 'mman_mextract_unlocked' was instructed to copy branches.
                                     *   NOTE: This callback can instruct the memory manager to exclude a notifier from the copy by returning 'MNOTIFY_CLONE_WITHOUT_CALLBACK'. */
#define MNOTIFY_RESTORE        0032 /*< [M][A][E] Following an 'MNOTIFY_EXTRACT'-event, the branch was re-inserted at 'addr...+=size' */
#define MNOTIFY_DELETE         0033 /*<    [A] Following an 'MNOTIFY_EXTRACT'-event, the branch was deleted when 'mman_maps_fini' was called. */
#define MNOTIFY_UNMAP          0034 /*< [M][A][E] Called when the branch is unmapped by normal means of 'munmap()' */
#define MNOTIFY_UNMAP_UNDO     0035 /*< [M][A] Following a successful call to 'MNOTIFY_UNMAP', some other error caused the unmap operation to be
                                     *         aborted, in which case this notification is send to undo whatever 'MNOTIFY_UNMAP' may have done.
                                     *         NOTE: Obviously not sent when 'MNOTIFY_UNMAP' itself already returned an error code. */
#define MNOTIFY_UNSHARE_DROP   0036 /*< [M][A][E] Called when the memory manger is unshared to determine if a mapping should be dropped.
                                     *   WARNING: Triggering this notification is skipped as though it returned '1' when the branch was mapped as 'PROT_LOOSE'.
                                     *   When implemented, the notification should return '> 0' to indicate that that the mapping should be dropped.
                                     *   When not implemented, ZERO(0) is returned, or the branch was mapped as 'PROT_LOOSE',
                                     *   the notifier is duplicated by sending the following events in order:
                                     *  'MNOTIFY_INCREF(new_mm)', 'MNOTIFY_CLONE(old_mm)', 'MNOTIFY_RESTORE(new_mm)'. */
/*      MNOTIFY_...            0040  */

#define MBRANCH_INCREF(self,mm) ((self)->mb_notify ? (*(self)->mb_notify)(MNOTIFY_INCREF,(self)->mb_closure,mm,0,0) : -EOK)
#define MBRANCH_DECREF(self)    ((self)->mb_notify ? (*(self)->mb_notify)(MNOTIFY_DECREF,(self)->mb_closure,NULL,0,0) : -EOK)

/* Special return value of 'MNOTIFY_CLONE'. When returned, still clone
 * the mapping, but do so without the notifier, essentially disconnecting
 * it from the memory block.
 * NOTE: The associated closure is still kept as memory tag though,
 *       meaning that a cloned mapping can still be addressed by that tag. */
#define MNOTIFY_CLONE_WITHOUT_CALLBACK 0xf000CA11


struct mbranch {
 ATREE_NODE(struct mbranch,uintptr_t)
                     mb_node;   /*< [lock(:m_lock)] This branch's node. */
 LIST_NODE(struct mbranch)
                     mb_order;  /*< [lock(:m_lock)][->mb_node.a_vmin > mb_node.a_vmax)] Ordered list of per-manager branches. */
 u32                 mb_prot;   /*< [lock(:m_lock)] Branch protection (Set of 'PROT_*') */
 raddr_t             mb_start;  /*< [lock(:m_lock)] Offset within 'mb_region' to the first page mapped by this branch. */
 REF struct mregion *mb_region; /*< [lock(:m_lock)][1..1] The region descriptor associated with this branch.
                                 *   NOTE: When writing to shared data, the relevant portion of data is
                                 *         extracted before a new region is inserted into the branch.
                                 *   NOTE: This pointer holds a reference to '->mr_refcnt', as well as a ranged
                                 *         reference to all parts between 'mb_start...+=MBRANCH_SIZE(self)'. */
 REF mbranch_notity  mb_notify; /*< [const][0..1] When non-NULL, a user-defined function executed upon special events. */
 void               *mb_closure;/*< [const][?..?] The closure argument passed to 'mb_notify' / Memory ~tag~ available in user-space. */
};
#define MBRANCH_MIN(self)          ((self)->mb_node.a_vmin)
#define MBRANCH_MAX(self)          ((self)->mb_node.a_vmax)
#define MBRANCH_BEGIN(self)        ((self)->mb_node.a_vmin)
#define MBRANCH_END(self)          ((self)->mb_node.a_vmax+1)
#define MBRANCH_SIZE(self)        (((self)->mb_node.a_vmax-(self)->mb_node.a_vmin)+1)
#define MBRANCH_HASNEXT(self,mman) ((self)->mb_order.le_next)
#define MBRANCH_HASPREV(self,mman) ((self)->mb_order.le_pself != &(mman)->m_order)
#define MBRANCH_NEXT(self)         ((self)->mb_order.le_next)
#define MBRANCH_PREV(self)           __COMPILER_CONTAINER_OF((self)->mb_order.le_pself,struct mbranch,mb_order.le_next)
#define MBRANCH_RADDR(self,vptr)   ((self)->mb_start+((uintptr_t)(vptr)-(self)->mb_node.a_vmin))

/* mbranch helpers related to builtin special memory mappings.
 * WARNING: Use of these may require additional headers to be #include'ed */
#define MBRANCH_ISINSTANCE(self)  ((self)->mb_notify == &instance_mnotify)
#define MBRANCH_ISSTACK(self)     ((self)->mb_notify == &stack_mnotify)
#define MBRANCH_GETINSTANCE(self) ((struct instance *)(self)->mb_closure)
#define MBRANCH_GETSTACK(self)    ((struct stack *)(self)->mb_closure)


#define MMAN_OFFSETOF_PDIR  0
#define MMAN_OFFSETOF_PPDIR PDIR_SIZE
#define MMAN_ALIGN          PDIR_ALIGN
struct mman {
 /* The memory-manager core controller. */
 pdir_t            m_pdir;   /*< [lock(m_lock)] Page directory implementing all the memory mapping through arch-specific means.
                              *   NOTE: Try to keep this member at offset ZERO(0), because it might require a
                              *         _HUGE_ alignment that would otherwise waste a whole bunch of space. */
 PHYS PAGE_ALIGNED pdir_t
                  *m_ppdir;  /*< [1..1|null(~)][const][== PHYSICAL(&m_pdir)] The physical address of 'm_pdir' */
 ATOMIC_DATA ref_t m_refcnt; /*< Memory manager reference counter. (Amount of threads using this manager; aka: Amount of relevant uses of 'm_pdir') */
 owner_rwlock_t    m_lock;   /*< Lock for accessing anything mman-related.
                              *  NOTE: Use an r/w lock with write-recursion here,
                              *        as an easy way of supporting load-on-read
                              *        recursion when loading a region of memory
                              *        requires loading some other missing piece
                              *        of memory.
                              */
 /* NOTE: Need to suspend all tasks using this memory manager and
  *       running on different CPUs while modifying 'm_pdir'!
  *       Otherwise, undefined behavior will be caused when
  *       another CPU executes code using said directory. */
 atomic_rwlock_t   m_tasks_lock;
 LIST_HEAD(struct task) m_tasks; /*< [lock(m_tasks_lock)] List of tasks using this memory manager. */
 ATREE_HEAD(struct mbranch)
                   m_map;    /*< [lock(m_lock)][0..1] Memory mappings (not including kernel mappings).
                              *   NOTE: When instructed from userspace, the caller must ensure never
                              *         to map memory within kernel-space (aka. above 'KERNEL_BASE'). */
 LIST_HEAD(struct mbranch)
                   m_order;  /*< [lock(m_lock)][0..1] Ordered list of branches. */
 LIST_HEAD(struct instance)
                   m_inst;   /*< [lock(m_lock)][0..1][sort(ASCENDING(i_base))]
                              *   Linked list of instances loaded into the address space. */
#ifndef CONFIG_NO_LDT
 REF struct ldt   *m_ldt;    /*< [lock(m_lock)][1..1] The LDT descriptor table used by tasks using this memory manager. */
#endif
 VIRT ppage_t      m_uheap;  /*< [lock(m_lock)] End address of the user-space heap (Used as hint when auto-allocating user-space heap memory). */
 VIRT ppage_t      m_ustck;  /*< [lock(m_lock)] Start address of the user-space heap (Used as hint when auto-allocating user-space stack memory). */
#ifndef CONFIG_NO_VM_EXE
 REF struct instance *m_exe; /*< [lock(m_lock)][0..1] The main executable within this VM (aka. the first instance ever mapped; used by '/proc/PID/exe') */
#endif /* !CONFIG_NO_VM_EXE */
 PAGE_ALIGNED
 VIRT struct envdata *m_environ; /*< [lock(m_lock)][?..?] Address at which environment data was mapped by the kernel. */
 PAGE_ALIGNED size_t  m_envsize; /*< [lock(m_lock)] Size of the environment mapping. (When non-zero, an environment block exists) */
 size_t               m_envargc; /*< [lock(m_lock)][WEAK(== m_environ->e_argc)] Size of the argument vector (in elements). */
 size_t               m_envenvc; /*< [lock(m_lock)][WEAK(== m_environ->e_envc)] Size of the environment vector (in elements). */
 size_t               m_envetxt; /*< [lock(m_lock)][WEAK(== ((uintptr_t)m_environ->e_argv -
                                  *                          (uintptr_t)m_environ->e_envp)-self->m_envenvc)]
                                  *   Total size of the environment text (in bytes). */
 size_t               m_envatxt; /*< [lock(m_lock)][WEAK(== offsetafter(m_environ,__e_targ) -
                                  *                            offsetof(m_environ,__e_targ))]
                                  *   Total size of the argument vector text (in bytes). */
 /* XXX: Tracking information about the total number of mapped bytes? */
};
#define MMAN_UHEAP_DEFAULT_ADDR  ((ppage_t)0x40000000) /* Initial value for 'm_uheap' */
#define MMAN_USTCK_DEFAULT_ADDR  ((ppage_t)0x80000000) /* Initial value for 'm_ustck' */

/* Access to user-space environment argument vectors/text buffers. */
#define MMAN_ENVIRON_ARGC(self) ((self)->m_envargc)
#define MMAN_ENVIRON_ARGP(self) ((USER char **)((uintptr_t)(self)->m_environ->__e_envv+ \
                                     (sizeof(USER char *)*((self)->m_envenvc+1))+ \
                                                           (self)->m_envetxt))
#define MMAN_ENVIRON_ENVC(self) ((self)->m_envenvc)
#define MMAN_ENVIRON_ENVP(self) ((self)->m_environ->__e_envv)
#define MMAN_ENVIRON_ARGSIZ(self) ((self)->m_envatxt)
#define MMAN_ENVIRON_ARGTXT(self) ((USER char *)((uintptr_t)(self)->m_environ->__e_envv+ \
                                      (sizeof(USER char *)*((self)->m_envenvc+1))+ \
                                      (sizeof(USER char *)*((self)->m_envargc+1))+ \
                                                            (self)->m_envetxt))
#define MMAN_ENVIRON_ENVSIZ(self) ((self)->m_envetxt)
#define MMAN_ENVIRON_ENVTXT(self) ((USER char *)((uintptr_t)(self)->m_environ->__e_envv+ \
                                      (sizeof(USER char *)*((self)->m_envenvc+1))))


#define mman_reading(self)              owner_rwlock_reading(&(self)->m_lock)
#define mman_writing(self)              owner_rwlock_writing(&(self)->m_lock)
#define mman_timedread(self,abstime)    owner_rwlock_timedread(&(self)->m_lock,abstime)
#define mman_timedwrite(self,abstime)   owner_rwlock_timedwrite(&(self)->m_lock,abstime)
#define mman_tryread(self)              owner_rwlock_tryread(&(self)->m_lock)
#define mman_trywrite(self)             owner_rwlock_trywrite(&(self)->m_lock)
#define mman_read(self)                 owner_rwlock_read(&(self)->m_lock)
#define mman_write(self)                owner_rwlock_write(&(self)->m_lock)
#define mman_tryupgrade(self)           owner_rwlock_tryupgrade(&(self)->m_lock)
#define mman_timedupgrade(self,abstime) owner_rwlock_timedupgrade(&(self)->m_lock,abstime)
#define mman_upgrade(self)              owner_rwlock_upgrade(&(self)->m_lock)
#define mman_downgrade(self)            owner_rwlock_downgrade(&(self)->m_lock)
#define mman_endwrite(self)             owner_rwlock_endwrite(&(self)->m_lock)
#define mman_endread(self)              owner_rwlock_endread(&(self)->m_lock)
#define mman_end(self)                  owner_rwlock_end(&(self)->m_lock)

#define MMAN_FOREACH(branch,self) LIST_FOREACH(branch,(self)->m_order,mb_order)
#define MMAN_FOREACH_INST(inst,self) LIST_FOREACH(inst,(self)->m_inst,i_chain)

/* The storage class that must be used when allocating
 * branch/region controller structures for the given memory manager.
 * NOTE: The distinction between physical memory for the kernel mman,
 *       and shared memory for all others is required to prevent the
 *       need to switch between page directories whenever wanting to
 *       access associated data structures, such as a futex.
 * NOTE: When conforming to these requirements, 'MMAN_DATAOK' will always succeed.
 * WARNING: Be careful when mapping regions/branches that are allocated
 *          using 'GFP_SHARED' within the kernel. - In doing so, you
 *          essentially create mappings who can only remain in a consistent
 *          state while some other mappings exist, meaning that attempting
 *          to unmap those regions might work, but will break the memory manager,
 *          which is eventually going to access unallocated memory.
 */
#define MMAN_DATAGFP(self)  ((self) == &mman_kernel ? GFP_MEMORY : GFP_SHARED)

/* A universal storage class that can be used in all memory manager.
 * Note though, that allocating memory of this class may invoke further
 * mman-related calls on the kernel's own memory manager, meaning it is
 * the caller's responsibility not to use this class when they may be
 * called for reasons of allocating more virtual memory of any kind.
 * NOTE: This class is used e.g. when allocating linker regions.
 */
#define MMAN_UNIGFP           GFP_SHARED

/* Check if a given region/branch address can be used with the specified memory manager. */
#define MMAN_DATAOK(self,p) ((self) == &mman_kernel || addr_isvirt(p))

/* The memory manager associated with the kernel (also contains 'pdir_kernel_v')
 * NOTE: Remember that anything mapped in here above 4Mib will
 *       automatically become available in _ALL_ other page
 *       directories! */
DATDEF VIRT struct mman mman_kernel;

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Allocate kernel control mappings within 'mman_kernel',
 * which in return will describe the bootstrap mappings
 * that can be found within 'pdir_kernel_v' at this point. */
INTDEF INITCALL void KCALL mman_initialize(void);
#endif

/* Go through all existing memory regions and transfer
 * core data into swap when memory isn't locked.
 * NOTE: As the 'mr_parts' lock of regions must be acquired
 *       for this, this function generally tries to be safe
 *       and will only try to acquire read/write access,
 *       skipping regions that cannot be immediately, therefor
 *       not running the risk of deadlocking when called
 *       while some region has been locked manually.
 * NOTE: Due to the size of the ABI surrounding this function, synchronization
 *       between threads calling this function is somewhat unique:
 *        - When the calling thread is already inside this
 *          function, it will return '-EALREADY' immediately.
 *        - When another thread is already swapping memory,
 *          wait until it has finished and  return '-EAGAIN'.
 * WARNING: This function is called by 'page_malloc()' as a last resort
 *          when physical memory allocation fails due to lack thereof.
 *          With that in mind, remember that any call to any kind of
 *          malloc()-style function may reach the point at which
 *          this function may be called.
 *          For that, it has been specially designed to fail, rather
 *          than cause deadlocks when called at an inopportune time.
 *          -> So there shouldn't really be any reason why this
 *             function should not be called at whatever point.
 * @param: max_swap: A hint to the kernel for how much (non-continuous) bytes should be swapped.
 *                   This argument is only hint in the sense that once at least 'max_swap' bytes
 *                   have been freed (result >= max_swap)
 * @return: *:         The amount of bytes that were freed
 *            WARNING: Don't test for this case using '>= 0'. - Use 'E_ISOK()' instead.
 * @return: -ENOMEM:   Failed to transfer data to swap:
 *                     Either the swap has filled up, or no swap has been installed.
 *                     This error should not occur due to small amount of memory
 *                     required for tracking swap tickets, as an internal buffer
 *                     is used for allocating them when in the context of 'mman_swapmem'
 * @return: -EAGAIN:   Another thread was swapping memory. - The caller was suspended
 *                     until that other thread was finished, yet it is unknown
 *                     how much data has become available.
 *                     The caller should handle this error by trying to allocate
 *                     memory again, hoping that enough will be available then.
 * @return: -EALREADY: The calling thread is already in the process of swapping memory.
 *                     In this case 'mman_swapmem' returns immediately to prevent a
 *                     deadlock that could otherwise be caused when memory becomes sparse.
 *                     In that context, this error should be handled the same as
 *                     -ENOMEM, yet with the addition of knowing that memory ~may~
 *                     become available again soon. */
FUNDEF ssize_t KCALL mman_swapmem(size_t max_swap, gfp_t flags);
#define MMAN_SWAPMEM_ALL    GFP_NORMAL /*< Default behavior: do anything to free up memory. */
#define MMAN_SWAPMEM_NOTRIM GFP_NOTRIM /*< Don't trim kernel heaps to make more memory available. */
#define MMAN_SWAPMEM_NOSWAP GFP_NOSWAP /*< Don't initialize any kind of I/O during swap.
                                        *  NOTE: 'MMAN_SWAPMEM_NOFS' only prevents write-mapped files
                                        *         from being synched, while this flag is required
                                        *         to prevents any use of a potential swap partition. */
#define MMAN_SWAPMEM_NOFS   GFP_NOFS   /*< Don't sync + unload write-mapped files to free up core memory. */


/* Helper macro for checking if allocation should be re-attempted after a call to 'mman_swapmem'.
 * >>again:
 * >>    result = kmalloc(n_bytes,GFP_MEMORY|GFP_NOSWAP);
 * >>    if (!result) {
 * >>        if (MMAN_SWAPOK(mman_swapmem(n_bytes,MMAN_SWAPMEM_ALL))) goto again;
 * >>    }
 * >>    syslog(LOG_DEBUG,"[MEM] Allocated: %p\n",result);
 * >>    kfree(result); */
#define MMAN_SWAPOK(e) XBLOCK({ ssize_t const _e = (e); XRETURN E_ISOK(_e) || _e == -EAGAIN; })

/* Create a new, empty memory manager.
 * NOTE: The memory manager's page directory will be pre-initialized
 *       to include shared memory that must never be touched, leaving
 *       all other memory (below 3GB) unused.
 * NOTE: Shared memory isn't mapped using managed branching, but only
 *       weakly aliased to always mirror the kernel page directory.
 * HINT: The caller is not required to fill in any additional fields
 *       after using this function to create a new memory manager.
 * @return: * :   The initial reference to a newly allocated memory manager.
 * @return: NULL: Not enough available memory. */
FUNDEF WUNUSED SAFE REF struct mman *KCALL mman_new(void);

#define MMAN_INCREF(self) (void)(ATOMIC_FETCHINC((self)->m_refcnt))
#define MMAN_DECREF(self) (void)(ATOMIC_DECFETCH((self)->m_refcnt) || (mman_destroy(self),0))
FUNDEF SAFE void KCALL mman_destroy(struct mman *__restrict self);


#ifdef CONFIG_DEBUG
/* Assert the internal consistency of the memory manager.
 * NOTE: The 'mman_assert_unlocked()' function only requires a read-lock to be held. */
FUNDEF      void KCALL mman_assert(struct mman *__restrict self);
FUNDEF SAFE void KCALL mman_assert_unlocked(struct mman *__restrict self);
#else
#define mman_assert(self)          (void)0
#define mman_assert_unlocked(self) (void)0
#endif

/* Create/Update the environment parameter block.
 * NOTE: The caller is responsible to ensure that
 *      'self' is the active memory manager.
 * NOTE: The caller must be holding a write-lock to on 'self'
 * NOTE: To better support linker extensions such as SHEBANG,
 *       the caller may specify two vectors of kernel-strings
 *       that should either be pre-pended, or appended to the
 *       actual user-space argument vector.
 * @return: * :      A pointer to the new allocated environment block.
 *             NOTE: The caller should then fill in additional information such as 'e_root'
 * @return: -EFAULT: A faulty pointer was given.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF SAFE VIRT struct envdata *KCALL
mman_setenviron_unlocked(struct mman *__restrict self,
                         VIRT char *VIRT *argv,
                         VIRT char *VIRT *envp,
                         HOST char **head_argv, size_t head_argc,
                         HOST char **tail_argv, size_t tail_argc);


#define THIS_MMAN   THIS_TASK->t_mman

/* Insert the given branch into the specified memory manager.
 * WARNING: For the most part, this is an internal function, meaning
 *          you shouldn't expose it to userspace, or call it without
 *          ensuring that the branch doesn't overlap with another
 *          beforehand!
 * NOTE: The caller must initialize 'branch->mb_node.a_min' and 'branch->mb_node.a_max',
 *       as well as all node/order-unrelated memory, but 'branch->mb_node.a_vmin' and
 *       'branch->mb_node.a_vmax' may be in an undefined state.
 * @param: branch: [GFP_ONLY(GFP_MEMORY)] The branch to insert into 'self'. */
FUNDEF void KCALL
mman_insbranch_unlocked(struct mman *__restrict self,
                        struct mbranch *__restrict branch);

/* Find the instance located at a given address.
 * NOTE: The caller must hold a read-lock on 'self'
 * >> Used for detecting module-specific privileges in user-space.
 * NOTE: Returns NULL if no instance exists at the given address. */
FUNDEF struct instance *KCALL
mman_instance_at_unlocked(struct mman const *__restrict self,
                          USER void *addr);

/* Find the first instance of the given module
 * 'mod', or return NULL if it doesn't exist.
 * NOTE: The caller must hold a read-lock on 'self'
 * HINT: This function returns a reference, thus ensuing that
 *       the instance can actually be used and isn't just dead. */
FUNDEF REF struct instance *KCALL
mman_instance_of_unlocked(struct mman const *__restrict self,
                          struct module *__restrict mod);

/* Same as 'mman_insbranch_unlocked', but also map
 * the branch within the associated page directory.
 * NOTE: During mapping, a read-lock is temporarily acquired on 'branch->mb_region->mr_plock'
 * @return: -EOK:    Successfully mapped & inserted the branch.
 * @return: -EINTR:  The calling thread was interrupted.
 * @return: -ENOMEM: Failed to map the branch (not enough available memory).
 */
FUNDEF errno_t KCALL
mman_insbranch_map_unlocked(struct mman *__restrict self,
                            struct mbranch *__restrict branch);

/* Return the branch at a given virtual address `addr', or NULL if none exists. */
FUNDEF struct mbranch *KCALL
mman_getbranch_unlocked(struct mman const *__restrict self, VIRT void *addr);

/* Return the part-state of memory located at the given `addr'.
 * NOTE: The caller must hold a read-lock on 'self'
 * @return: * :                  One of 'MPART_STATE_*'
 * @return: MPART_STATE_MISSING: The part is either not loaded, or the address is faulty. */
FUNDEF u8 KCALL mman_getstate_unlocked(struct mman const *__restrict self, VIRT void *addr);

/* Print a human-readable representation of the given memory manager.
 * NOTE: The caller is responsible for holding a read-lock to 'self->m_lock'
 * HINT: The representation generated by this may be used to implement '/proc/[pid]/maps' */
FUNDEF ssize_t KCALL mman_print_unlocked(struct mman *__restrict self,
                                         pformatprinter printer, void *closure);

/* Get/Allocate-missing futex objects associated with the given `addr'.
 * NOTE: [mman_getfutex_unlocked] The caller is responsible for holding a read-lock to 'self->m_lock'
 * NOTE: [mman_newfutex_unlocked] The caller is responsible for holding a write-lock to 'self->m_lock'
 * @return: * :      A new reference to a futex located at `addr'.
 * @return: -EFAULT: An invalid address was given.
 * @return: -ENOMEM: [mman_newfutex_unlocked] Failed to allocate a new futex. */
FUNDEF REF struct mfutex *KCALL mman_getfutex_unlocked(struct mman *__restrict self, VIRT void *addr);
FUNDEF REF struct mfutex *KCALL mman_newfutex_unlocked(struct mman *__restrict self, VIRT void *addr);

/* Load all branches required for the given 'inst' into 'self',
 * and register 'inst' as part of the the memory manager's
 * list of active module, so-long as at least one byte must be mapped.
 *  - Instances without any memory-branch (aka: those of an empty module)
 *    are silently ignored and not linked into the chain of modules.
 *    This coincides with the requirement that 'struct instance::i_chain'
 *    is only considered valid when 'struct instance::i_branch' is non-zero!
 *  - This function makes use of 'mman_mmap_unlocked', meaning
 *    that existing mappings at 'load_addr' may be overwritten.
 *  - The caller is responsible for mapping the module to its
 *    proper address in the event that it isn't relocatable.
 *  - The caller must be holding a write-lock on 'self'.
 *  - The caller is responsible to ensure that all regions from
 *    'inst->i_module' have previously been allocated (aka. 'module_mkregions()' was called)
 *  - Created branches are to hold a reference to the given instance through use of 'instance_mnotify'
 * HINT: When called on the kernel page directory, this function
 *       will not allocate any virtual memory, meaning that any
 *       free range discovered with 'mman_findspace_unlocked()'
 *       is still going to be available when using this function
 *       to load drivers.
 * HINT: You can easily use 'prot_amask' and 'prot_omask' to create the
 *       initial mapping as writable in the event that relocations must
 *       be performed, too.
 * @param: inst:       The instance that should be mapped.
 * @param: prot_amask: A bit-wise and-modifier for module segment protections.
 * @param: prot_omask: A bit-wise or-modifier for module segment protections.
 * @return: -EOK:      Successfully created all instance mappings.
 * @return: -EPERM:    The given 'inst' doesn't permit new references to be created.
 * @return: -ENOMEM:   Not enough available memory. */
FUNDEF errno_t KCALL
mman_mmap_instance_unlocked(struct mman *__restrict self,
                            struct instance *__restrict inst,
                            u32 prot_amask, u32 prot_omask);


/* Allocate mappings associated with the given stack.
 * NOTE: When creating guard mappings ('MREGION_TYPE_ISGUARD(type)'), the given
 *      'stck' will be updated whenever one of them is accessed and thereby
 *       allocated, meaning that any access must be protected by 'self->m_lock'.
 * @param: self:         The memory manager to map the stack inside of.
 * @param: stck:         The stack that should be mapped (Using the 's_begin' and 's_end' fields as mapping targets)
 *              WARNING: The stack must be in a consistent state, as this function
 *                       create references to the stack's 's_branch' counter, as
 *                       well as one additional reference to 's_refcnt', so long as
 *                       at least one branch was created.
 * @param: prot:         Memory protection for stack data (Usually 'PROT_READ|PROT_WRITE')
 * @param: type:         One of 'MREGION_TYPE_*' (Including guard mappings!)
 * @param: funds:        Guard funds used when allocating guard pages (Only used by guard mappings).
 * @param: guard_size:   The stack portion (in bytes) that should be reserved as a guard (Only used by guard mappings).
 *                 NOTE: Usually, this value is simply set to 'PAGESIZE'.
 *                       If this argument is ZERO(0) and 'type' is a guard region type,
 *                       no guard region is created and 'type' will be ignored.
 *                 NOTE: When larger, this value is clamped to '((uintptr_t)stck->s_end-(uintptr_t)stck->s_begin)-PAGESIZE',
 *                 NOTE: When 'type' isn't a 'MREGION_TYPE_ISGUARD()', this value is forced to ZERO(0)
 * @param: stack_region: [0..1]
 *                       When non-null, a region of exactly
 *                       '((uintptr_t)stck->s_end-(uintptr_t)stck->s_begin)-
 *                          CEIL_ALIGN(guard_size,PAGESIZE)' ('guard_size' was already fixed)
 *                       bytes that will be mapped for the initially valid portion of the stack.
 *                       Using this, one can specify custom memory initialization, including
 *                       file mappings, or some debug filler-byte.
 *                       When NULL, a new region is created internally that fits the
 *                       proper size and uses random initialization of anonymous memory.
 *                       WARNING: When provided a guard mapping must be created,
 *                                this region must be of type 'MREGION_TYPE_MEM'.
 * @return: -EOK:        Successfully mapped the stack.
 * @return: -ENOMEM:     Not enough available memory to allocate control structures.
 */
FUNDEF errno_t KCALL
mman_mmap_stack_unlocked(struct mman *__restrict self, struct stack *__restrict stck,
                         u32 prot, u8 type, u16 funds, size_t guard_size,
                         struct mregion *stack_region);

/* Helper macro for safely unmapping a stack. */
#define mman_munmap_stack_unlocked(self,stck) \
        mman_munmap_unlocked(self,(stck)->s_begin, \
                            (uintptr_t)(stck)->s_end- \
                            (uintptr_t)(stck)->s_begin,\
                             MMAN_MUNMAP_TAG,stck)


/* Change the protection for all pages within the range defined by 'addr...+=n_bytes'
 * NOTE: `addr' must be page-aligned, but 'n_bytes' will be ceil-aligned by PAGESIZE.
 * NOTE: The caller is responsible for holding a write-lock to 'self->m_lock'
 * WARNING: Some pages may have been updated, even when the function fails!
 * @return: * :         The actual number of bytes who's protection got changed.
 * @return: -ENOMEM:    Not enough available memory for splitting ranges.
 * @return: E_ISERR(*): Failed to incref() a memory branch for some reason. */
FUNDEF ssize_t KCALL mman_mprotect_unlocked(struct mman *__restrict self, VIRT ppage_t addr,
                                            size_t n_bytes, u32 prot_amask, u32 prot_omask);

/* Map the given memory region 'region' to `addr'.
 * NOTE: Any existing mappings are deleted.
 * NOTE: The caller is responsible for holding a write-lock to 'self->m_lock'
 * HINT: Before unlocking the memory manager, the caller should
 *       reduce the reference counter upon success, as it is used
 *       internally to determine if a region can be modified in-place.
 *       Assuming that the caller held a reference to 'region' before,
 *       upon success, its reference counter will always be '>= 2',
 *       meaning it can safely be reduced without a chance of calling
 *       the destructor, thus allowing you to safely hint to the
 *       memory manager to inherit the region.
 *       If you were to unlock the manager first, everything would work just as well,
 *       but there'd also be a chance that the manager work less efficient when
 *       a PAGEFAULT happens before you finally get around to decref-ing the region.
 * HINT: Even when no 'notify' is given ('notify == NULL'), 'closure' may still be
 *       used to tag mapped memory with a unique key (such as the address of some
 *       structure owned by the caller), which can then later be re-used to protect
 *       unrelated mappings from be deleted in 'mman_munmap_unlocked'.
 *       NOTE: Always use the address of some real object to prevent accidental
 *             overlap of closure tags that could start happening if only one
 *             person started using random, or fixed numbers!
 *          >> ALWAYS USE AN ADDRESS!
 * @param: addr:     The virtual address to which the region should be mapped.
 * @param: n_bytes:  The amount of bytes that should be mapped (usually 'region->mr_size') (will be ceil-aligned by PAGESIZE).
 * @param: start:    The first address in 'region' where mappings should start (usually ZERO(0)).
 * @param: region:  [GFP_ONLY(GFP_MEMORY)] The memory region that should be mapped.
 * @param: prot:     A set of 'PROT_*' (from <sys/mman.h>) describing protection properties.
 * @param: notify:   A callback that is executed upon certain memory events, or NULL if not used.
 * @param: closure:  The closure argument for 'notify'
 * @return: -EOK:    Successfully mapped the region. (it's reference counter was incremented)
 * @return: -ENOMEM: Not enough available memory for mapping. */
FUNDEF errno_t KCALL mman_mmap_unlocked(struct mman *__restrict self, VIRT ppage_t addr,
                                        size_t n_bytes, raddr_t start,
                                        PHYS struct mregion *__restrict region,
                                        u32 prot, mbranch_notity notify, void *closure);

/* Unmaps all memory regions between 'addr...+=size'
 * NOTE: `addr' must be page-aligned, but 'n_bytes' will be ceil-aligned by PAGESIZE.
 * NOTE: Regions only partially mapped at the corners will be split.
 * NOTE: The caller is responsible for holding a write-lock to 'self->m_lock'
 * @param: addr:        Start address where mappings should be deleted from.
 * @param: n_bytes:     Size of the range from which mappings should be deleted.
 * @param: mode:        Unmapping mode (s.a.: 'MMAN_MUNMAP_*').
 * @param: tag:         Tag to scan for when using 'MMAN_MUNMAP_TAG'
 * @return: * :         The amount of unmapped bytes.
 * @return: -ENOMEM:    Not enough available memory.
 * @return: E_ISERR(*): Failed to incref() a memory branch for some reason. */
FUNDEF ssize_t KCALL mman_munmap_unlocked(struct mman *__restrict self,
                                          VIRT ppage_t addr, size_t n_bytes,
                                          u32 mode, void *tag);
#define MMAN_MUNMAP_ALL      0x00000000 /*< Unmap all mappings. */
#define MMAN_MUNMAP_CLEAR    0x00000001 /*< Indicate that any allocate core memory uniquely owned by this mman is fully zero-initialized.
                                         *  WARNING: Passing this flag without truly knowing that this is true causes undefined behavior. */
#define MMAN_MUNMAP_TAG      0x00000010 /*< Only unmap branches who's 'mb_closure' argument matches the given 'tag' */

/* Capture (and potentially delete) all existing mappings within 'addr...+=n_bytes'
 * All captured mappings are stored within a chain of branches stored in '*maps'
 * NOTE: When 'unmap_branches' is false, the caller must only hold a read-lock!
 * WARNING: Branches within '*maps' may be allocated in 'MMAN_DATAGFP(self)',
 *          meaning that the caller should be aware of the fact that branches
 *          may not be compatible with any other memory manager.
 * @param: unmap_branches: When true, inherit existing branches by unmapping them.
 * @param: maps:           Initialized by the function, upon return this container
 *                         describes all mappings within 'addr...+=n_bytes' before
 *                         the function was called.
 * @return: * :         The total amount of bytes stored on 'maps'
 *                      NOTE: Only in this case, was '*maps' initialized properly.
 * @return: -ENOMEM:    Not enough available memory.
 * @return: E_ISERR(*): Failed to incref() a memory branch for some reason. */
FUNDEF ssize_t KCALL mman_mextract_unlocked(struct mman *__restrict self,
                                            struct mman_maps *__restrict maps,
                                            VIRT ppage_t addr, size_t n_bytes,
                                            bool unmap_branches);

/* The reverse of 'mman_mextract_unlocked', inserting a given
 * list of branches into the specified memory manager.
 * NOTE: After a successful call to this function with
 *      'reuse_branches == true', 'mman_maps_fini(maps)' is a no-op.
 * @param: addr:           The base address that should be added to all mappings
 *                         before they are re-inserted into the given memory manager.
 * @param: reuse_branches: When true, remove branches from 'maps' as they are used.
 * @param: maps:           A list of mappings previously created with 'mman_mextract_unlocked'.
 * @return: -EOK:    Successfully restored all branches.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF errno_t KCALL mman_mrestore_unlocked(struct mman *__restrict self,
                                            struct mman_maps *__restrict maps,
                                            VIRT ppage_t addr, bool reuse_branches);


struct mman_maps {
 LIST_HEAD(struct mbranch) mm_maps; /*< [0..1] Ordered list of branches.
                                     *   NOTE: These branches have been unlinked from any address tree,
                                     *         and their 'a_vmin' is offset from the original `addr'
                                     *         passed to 'mman_mextract_unlocked'.
                                     */
};

/* Called to finalize a list of mappings created using 'mman_mextract_unlocked'.
 * NOTE: This function must be called when 'mman_mrestore_unlocked' isn't called
 *       with the given list, or when it is called with 'reuse_branches' set to false.
 * WARNING: Causes undefined behavior when called on a maps-list not
 *          initialized with 'mman_mextract_unlocked'. */
FUNDEF void KCALL mman_maps_fini(struct mman_maps *__restrict self);

/* Search for a free address range of at least 'n_bytes' bytes,
 * using 'hint' as a hint towards nearby addresses.
 * NOTE: The caller is responsible for holding a read-lock to 'self->m_lock'
 * @return: * :         A page-aligned pointer to the virtual address of available memory.
 *             WARNING: As soon as the caller releases their read-lock, the
 *                      returned pointer must no longer be considered available.
 *                     (Remember the -ERELOAD case if you choose to upgrade 'self->m_lock'!)
 * @param: alignment:   The minimum alignment required for the page
 *                      address of the returned virtual page pointer.
 *                      NOTE: Must be a non-zero multiple of PAGESIZE.
 * @param: gap_size:    Used to provide sufficient space between the returned address
 *                      and the nearest lower/higher branch, if that branch exists.
 *                      This argument's main purpose is to provide the ability
 *                      of guarantying space for guard mappings to extend into.
 *                      NOTE: Unless 'MMAN_FINDSPACE_FORCEGAP' is set, 'gap_size' is only
 *                            used to ensure unused space before 'MREGION_TYPE_LOGUARD',
 *                            and after 'MREGION_TYPE_HIGUARD'
 *                      WARNING: When 'hint...+=n_bytes' wasn't in use, this argument
 *                               is ignored, even when a guard mapping exists less
 *                               than 'gap_size' bytes away.
 * @param: mode:        A set of 'MMAN_FINDSPACE_*'
 * @return: PAGE_ERROR: There is no more available address space to map 'n_bytes' continuous bytes. */
FUNDEF VIRT ppage_t KCALL mman_findspace_unlocked(struct mman *__restrict self,
                                                  VIRT ppage_t hint, size_t n_bytes,
                                                  size_t alignment, size_t gap_size,
                                                  u32 mode);
#define MMAN_FINDSPACE_ABOVE    0x00 /*< If 'hint...+=n_bytes' cannot be used, search for a greater, suitable address range. */
#define MMAN_FINDSPACE_BELOW    0x01 /*< If 'hint...+=n_bytes' cannot be used, search for a lower, suitable address range. */
#define MMAN_FINDSPACE_FORCEGAP 0x02 /*< Force a gap before and after any kind of memory mapping. */

/* Check if any address within the given range is currently in use.
 * NOTE: The caller is responsible for holding a read-lock to 'self->m_lock'
 * @return: true:  At least one byte from the given range is in use.
 * @return: false: Either 'n_bytes' was ZERO(0), or no byte within the given range is in use. */
FUNDEF WUNUSED bool KCALL mman_inuse_unlocked(struct mman const *__restrict self,
                                              VIRT void *start, size_t n_bytes);
/* Similar to 'mman_inuse_unlocked()', but only return true
 * when _ALL_ full pages part of the given range are mapped and carry all flags given by 'prot' and masked by 'mask'.
 * NOTE: Returns false when 'n_bytes' is ZERO(0) or overflows when added to 'start'. */
FUNDEF WUNUSED bool KCALL mman_valid_unlocked(struct mman const *__restrict self,
                                              VIRT void *start, size_t n_bytes,
                                              u32 mask, u32 prot);

/* Remaps all pages from 'old_addr...+=n_bytes' to 'new_addr'
 * Any existing mappings at 'new_addr' are deleted.
 * The protection of the new mappings is calculated as:
 * >> new_branch->mb_prot = (old_branch->mb_prot&prot_amask)|prot_omask;
 * Upon success, previous mappings from 'old_addr...+=n_bytes' are deleted.
 * NOTE: The caller is responsible for holding a write-lock to 'self->m_lock'
 * NOTE: 'n_bytes' is ceil-aligned by PAGESIZE.
 * @return: * :       The actual amount of bytes that were re-mapped.
 * @return: -ENOMEM: Not enough available memory for allocating controller structures. */
FUNDEF ssize_t KCALL mman_mremap_unlocked(struct mman *__restrict self,
                                          VIRT ppage_t new_addr, VIRT ppage_t old_addr,
                                          size_t n_bytes, u32 prot_amask, u32 prot_omask);

/* Ensure that all memory mappings from 'addr...+=n_bytes' are loaded into the core.
 * NOTE: The caller is responsible for holding a write-lock to 'self->m_lock'
 * NOTE: 'n_bytes' is ceil-aligned by PAGESIZE.
 * WARNING: Some pages may have been moved into the core, even when the function fails!
 *          NOTE: Although when n_bytes '<= PAGESIZE', this will not be the case.
 * @param: mode: [MMAN_MCORE_WRITE] When not set, core memory may still be shared with other
 *                                       processes, but if true, also perform copy-on-write semantics.
 *                                    >> This switch enables copy-on-write alongside load-on-read.
 *               [MMAN_MCORE_USER]  When true, only load branches not marked as 'PROT_NOUSER'.
 * @return: * :      The amount of bytes now moved into the core.
 * @return: 0 :      Nothing needed, or could to be moved into the core.
 *             NOTE: With that in mind, this is also returned when an
 *                   invalid address is given and is thereby the perfect
 *                   indicator for propagating a pagefault if the caller
 *                   has executed this function after one was received.
 * @return: -ENOMEM: Not enough available memory (any reason).
 *             NOTE: When this happens, the caller should try to
 *                   swap other memory managers before swapping
 *                   other regions within this one, intermittently
 *                   retrying to load the given range again and again.
 * @return: E_ISERR(*): Failed to initialize memory for some reason. */
FUNDEF ssize_t KCALL mman_mcore_unlocked(struct mman *__restrict self,
                                         VIRT ppage_t addr, size_t n_bytes,
                                         u32 mode);
#define MMAN_MCORE_READ    0x00 /*< Allocate missing pages. */
#define MMAN_MCORE_WRITE   0x02 /*< Copy pages to acquire write-access after COW. */
#define MMAN_MCORE_USER    0x04 /*< Only handle branches not marked as 'PROT_NOUSER'. */
#define MMAN_MCORE_NOGUARD 0x08 /*< Don't allocate guard pages when touching them. */
#define MMAN_MCORE_LOCKED  0x80 /*< Only load core memory that is also locked. */
/*      MMAN_MCORE_...     0xf8  */


/* Lock/Unlock all memory mappings within 'addr...+=n_bytes'
 * NOTE: The caller is responsible for holding a write-lock to 'self->m_lock'
 * NOTE: 'n_bytes' is ceil-aligned by PAGESIZE.
 * NOTE: locking/unlocking works recursively in both ways:
 *       locking memory twice required unlocking it twice.
 *       unlocking memory first requires you to lock it 2
 *       times before mlock()-semantics actually being to take space.
 * WARNING: Make sure to lock swap/storage-device drivers
 *          in memory, as they might get called when a
 *          pagefault occurs, meaning they must already
 *          be loaded at that point.
 * @param: mode: A set of 'MMAN_MLOCK_*'
 * @return: * :      The actual amount of bytes locked/unlocked.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF ssize_t KCALL mman_mlock_unlocked(struct mman *__restrict self,
                                         VIRT ppage_t addr, size_t n_bytes,
                                         u32 mode);
#define MMAN_MLOCK_UNLOCK  0x00 /*< Unlock previously locked memory. */
#define MMAN_MLOCK_LOCK    0x01 /*< When set, lock memory instead of unlocking it. */
#define MMAN_MLOCK_INCORE  MMAN_MCORE_LOCKED  /*< Make sure that all locked pages are also present in-core. */
#define MMAN_MLOCK_WRITE   MMAN_MCORE_WRITE   /*< [for:MMAN_MLOCK_INCORE] Copy pages to acquire write-access after COW. */
#define MMAN_MLOCK_USER    MMAN_MCORE_USER    /*< [for:MMAN_MLOCK_INCORE] Only handle branches not marked as 'PROT_NOUSER'. */
#define MMAN_MLOCK_NOGUARD MMAN_MCORE_NOGUARD /*< [for:MMAN_MLOCK_INCORE] Don't allocate guard pages when touching them. */



/* Auto-locking mman-functions for convenience.
 * WARNING: All of these can also return '-EINTR'. */
LOCAL SAFE REF struct mfutex *KCALL mman_getfutex(struct mman *__restrict self, VIRT void *addr);
LOCAL SAFE REF struct mfutex *KCALL mman_newfutex(struct mman *__restrict self, VIRT void *addr);
LOCAL SAFE bool KCALL mman_inuse(struct mman const *__restrict self, VIRT ppage_t start, size_t n_bytes);
LOCAL SAFE ssize_t KCALL mman_mprotect(struct mman *__restrict self, VIRT ppage_t addr, size_t n_bytes, u32 prot_amask, u32 prot_omask);
LOCAL SAFE errno_t KCALL mman_mmap(struct mman *__restrict self, VIRT ppage_t addr, size_t n_bytes, raddr_t start, struct mregion *__restrict region, u32 prot, mbranch_notity notify, void *closure);
LOCAL SAFE ssize_t KCALL mman_mremap(struct mman *__restrict self, VIRT ppage_t new_addr, VIRT ppage_t old_addr, size_t n_bytes, u32 prot_amask, u32 prot_omask);
LOCAL SAFE ssize_t KCALL mman_munmap(struct mman *__restrict self, VIRT ppage_t addr, size_t n_bytes, u32 mode, void *tag);
LOCAL SAFE ssize_t KCALL mman_mcore(struct mman *__restrict self, VIRT ppage_t addr, size_t n_bytes, u32 mode);
LOCAL SAFE ssize_t KCALL mman_mlock(struct mman *__restrict self, VIRT ppage_t addr, size_t n_bytes, u32 mode);


/* HOW-TO do cow+load-from-file:
 * >> mman_write(self);
 * >> // Ensure that our memory manager is the only use using
 * >> // its regions 'addr...+=size'. - This can be done through:
 * >> //    if (!(m_map->[in(addr...+=size)]->mb_prot&PROT_SHARED) && 
 * >> //          m_map->[in(addr...+=size)]->mb_region->mr_parts->[in(addr...+=size)]->mt_refcnt != 1)
 * >> //          AUTOMAGICALLY_CLONE_DUPLICATE_REGION_PARTS_INTO_SMALLER_REGIONS_WHILE_UPDATING_BRANCH_MAPPINGS();
 * >> ENSURE_UNIQUE_REGION(addr,size);
 * >> // Ensure that pages within the given address range are in-core.
 * >> // During this, the memory manager will also ensure that memory is initialized.
 * >> ENSURE_INCORE_REGION(addr,size);
 * >> mman_endwrite(self);
 * FAULTS:
 *   - Use 'size = PAGESIZE;' and 'addr = FLOOR_ALIGN(GET_REGISTER("%cr2"),PAGESIZE);'
 *   - READ/EXEC-FAULT: Only the second step is performed.
 *   - WRITE-FAULT:     Both steps are performed.
 */


#ifndef __INTELLISENSE__
LOCAL SAFE REF struct mfutex *KCALL
mman_getfutex(struct mman *__restrict self, VIRT void *addr) {
 REF struct mfutex *result;
 result = E_PTR(mman_read(self));
 if (E_ISOK(result)) {
  result = mman_getfutex_unlocked(self,addr);
  mman_endread(self);
 }
 return result;
}
LOCAL SAFE REF struct mfutex *KCALL
mman_newfutex(struct mman *__restrict self, VIRT void *addr) {
 REF struct mfutex *result;
 result = E_PTR(mman_write(self));
 if (E_ISOK(result)) {
  result = mman_newfutex_unlocked(self,addr);
  mman_endwrite(self);
 }
 return result;
}
LOCAL SAFE bool KCALL
mman_inuse(struct mman const *__restrict self,
           VIRT ppage_t start, size_t n_bytes) {
 ssize_t result = mman_read((struct mman *)self);
 if (E_ISOK(result)) {
  result = mman_inuse_unlocked(self,start,n_bytes);
  mman_endread((struct mman *)self);
 }
 return result;
}
LOCAL SAFE ssize_t KCALL
mman_mprotect(struct mman *__restrict self, VIRT ppage_t addr,
              size_t n_bytes, u32 prot_amask, u32 prot_omask) {
 ssize_t result = mman_write(self);
 if (E_ISOK(result)) {
  result = mman_mprotect_unlocked(self,addr,n_bytes,prot_amask,prot_omask);
  mman_endwrite(self);
 }
 return result;
}
LOCAL errno_t KCALL
mman_mmap(struct mman *__restrict self, VIRT ppage_t addr,
          size_t n_bytes, raddr_t start,
          struct mregion *__restrict region, u32 prot,
          mbranch_notity notify, void *closure) {
 errno_t result = mman_write(self);
 if (E_ISOK(result)) {
  result = mman_mmap_unlocked(self,addr,n_bytes,start,region,
                              prot,notify,closure);
  mman_endwrite(self);
 }
 return result;
}
LOCAL ssize_t KCALL
mman_mremap(struct mman *__restrict self, VIRT ppage_t new_addr,
            VIRT ppage_t old_addr, size_t n_bytes, u32 prot_amask, u32 prot_omask) {
 ssize_t result = mman_write(self);
 if (E_ISOK(result)) {
  result = mman_mremap_unlocked(self,new_addr,old_addr,
                                n_bytes,prot_amask,prot_omask);
  mman_endwrite(self);
 }
 return result;
}
LOCAL ssize_t KCALL
mman_munmap(struct mman *__restrict self, VIRT ppage_t addr,
            size_t n_bytes, u32 mode, void *tag) {
 ssize_t result = mman_write(self);
 if (E_ISOK(result)) {
  result = mman_munmap_unlocked(self,addr,n_bytes,mode,tag);
  mman_endwrite(self);
 }
 return result;
}
LOCAL ssize_t KCALL
mman_mcore(struct mman *__restrict self,
           VIRT ppage_t addr, size_t n_bytes, u32 mode) {
 ssize_t result = mman_write(self);
 if (E_ISOK(result)) {
  result = mman_mcore_unlocked(self,addr,n_bytes,mode);
  mman_endwrite(self);
 }
 return result;
}
LOCAL ssize_t KCALL
mman_mlock(struct mman *__restrict self,
           VIRT ppage_t addr, size_t n_bytes, u32 mode) {
 ssize_t result = mman_write(self);
 if (E_ISOK(result)) {
  result = mman_mlock_unlocked(self,addr,n_bytes,mode);
  mman_endwrite(self);
 }
 return result;
}
#endif

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_MMAN_H */
