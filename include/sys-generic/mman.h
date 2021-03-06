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
#ifndef _SYS_GENERIC_MMAN_H
#define _SYS_GENERIC_MMAN_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>

#ifndef __CRT_GLC
#error "<sys/mman.h> is not supported by the linked libc"
#endif /* !__CRT_GLC */

__SYSDECL_BEGIN

#ifndef __off_t_defined
#define __off_t_defined 1
typedef __typedef_off_t off_t;
#endif /* !__off_t_defined */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __size_t size_t;
#endif /* !__size_t_defined */

#ifndef __mode_t_defined
#define __mode_t_defined 1
typedef __mode_t mode_t;
#endif /* !__mode_t_defined */

#ifndef PROT_NONE
#define PROT_NONE     0x00 /*< Data cannot be accessed. */
#endif /* !PROT_NONE */
#ifndef PROT_EXEC
#define PROT_EXEC     0x01 /*< Data can be executed. */
#endif /* !PROT_EXEC */
#ifndef PROT_WRITE
#define PROT_WRITE    0x02 /*< Data can be written. */
#endif /* !PROT_WRITE */
#ifndef PROT_READ
#define PROT_READ     0x04 /*< Data can be read. */
#endif /* !PROT_READ */
#ifndef PROT_SEM
#define PROT_SEM      0x08
#endif /* !PROT_SEM */

#ifdef __USE_KOS
#ifndef PROT_LOOSE
#define PROT_LOOSE    0x10 /*< Unmap the region within the when cloning a VM (`CLONE_VM').
                            *  NOTE: Implicitly set for all system-allocated user stacks,
                            *        except for that of the calling thread. */
#endif /* !PROT_LOOSE */
#ifndef PROT_SHARED
#define PROT_SHARED   0x20 /*< Changes are shared, even after the VM was cloned (`CLONE_VM').
                            *  NOTE: Same as the `MAP_SHARED' map flag, but
                            *        can be set like any other protection. */
#endif /* !PROT_SHARED */
#ifdef __KERNEL__
#ifndef PROT_NOUSER
#define PROT_NOUSER   0x40 /*< Map memory as inaccessible to user-space.
                            *  WARNING: Not fully enforced for addresses within user-memory. */
#endif /* !PROT_NOUSER */
#ifndef PROT_CLEAN
#define PROT_CLEAN    0x80 /*< Unset whenever user-space re-maps a page as writable. - Cannot be removed.
                            *  NOTE: This flag is used to prevent rootfork() from working
                            *        when called from an otherwise read-only module after
                            *        some portion of the section containing the root-fork
                            *        system call has been mapped as writable.
                            *     >> rootfork() fails when any page in the calling section (.text)
                            *        isn't part of a root-module, isn't fully mapped, was
                            *        re-mapped somewhere else, or been made writable at any point. */
#endif /* !PROT_CLEAN */
#endif /* __KERNEL__ */
#endif /* __USE_KOS */

#ifdef __KERNEL__
#ifndef PROT_MASK
#define PROT_MASK     0x3f /*< Mask of flags accessible from user-space. */
#endif /* !PROT_MASK */
#endif


#ifdef __USE_KOS
#ifndef MAP_AUTOMATIC
#define MAP_AUTOMATIC 0x00000000 /* Use sharing behavior specified by `PROT_SHARED'. */
#endif /* !MAP_AUTOMATIC */
#endif

#ifndef MAP_SHARED
#define MAP_SHARED        0x00000001 /* Share changes. */
#endif /* !MAP_SHARED */
#ifndef MAP_PRIVATE
#define MAP_PRIVATE	      0x00000002 /* Changes are private. */
#endif /* !MAP_PRIVATE */
#ifndef MAP_FIXED
#define MAP_FIXED         0x00000010 /* Interpret addr exactly. */
#endif /* !MAP_FIXED */
#ifdef __USE_MISC
#ifndef MAP_TYPE
#define MAP_TYPE          0x0000000f /* Mask for type of mapping. */
#endif /* !MAP_TYPE */
#ifndef MAP_FILE
#define MAP_FILE          0x00000000 /* Do use a file. */
#endif /* !MAP_FILE */
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS     0x00000020 /* Don't use a file. */
#endif /* !MAP_ANONYMOUS */
#endif
#ifndef MAP_ANON
#define MAP_ANON          MAP_ANONYMOUS
#endif /* !MAP_ANON */
                            
/* Other flags. */
#ifdef __USE_MISC
#ifndef MAP_32BIT
#define MAP_32BIT         0x00000040 /* Only give out 32-bit addresses. */
#endif /* !MAP_32BIT */
#endif

/* These are Linux-specific (But also supported by KOS). */
#ifdef __USE_MISC
#ifndef MAP_GROWSDOWN
#define MAP_GROWSDOWN     0x00000100 /* Stack-like segment. */
#endif /* !MAP_GROWSDOWN */
#ifndef MAP_GROWSUP
#define MAP_GROWSUP       0x00000200 /* Stack-like segment growing upwards. */
#endif /* !MAP_GROWSUP */
#ifndef MAP_DENYWRITE
#define MAP_DENYWRITE     0x00000800 /* Ignored. */
#endif /* !MAP_DENYWRITE */
#ifndef MAP_EXECUTABLE
#define MAP_EXECUTABLE    0x00001000 /* Ignored. */
#endif /* !MAP_EXECUTABLE */
#ifndef MAP_LOCKED
#define MAP_LOCKED        0x00002000 /* Lock the mapping. */
#endif /* !MAP_LOCKED */
#ifndef MAP_NORESERVE
#define MAP_NORESERVE     0x00004000 /* Don't check for reservations. */
#endif /* !MAP_NORESERVE */
#ifndef MAP_POPULATE
#define MAP_POPULATE      0x00008000 /* Populate (prefault) pagetables. */
#endif /* !MAP_POPULATE */
#ifndef MAP_NONBLOCK
#define MAP_NONBLOCK      0x00010000 /* Do not block on IO. */
#endif /* !MAP_NONBLOCK */
#ifndef MAP_STACK
#define MAP_STACK         0x00020000 /* Allocation is for a stack.
                                      * NOTE: KOS uses this flag to determine where
                                      *       automatic memory mappings are allocated at. */
#endif /* !MAP_STACK */
#ifndef MAP_HUGETLB
#define MAP_HUGETLB       0x00040000 /* Create huge page mapping. */
#endif /* !MAP_HUGETLB */
#ifndef MAP_HUGE_SHIFT
#define MAP_HUGE_SHIFT    26
#endif /* !MAP_HUGE_SHIFT */
#ifndef MAP_HUGE_MASK
#define MAP_HUGE_MASK     0x3f
#endif /* !MAP_HUGE_MASK */
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0x04000000 /* For anonymous mmap, memory could be uninitialized.
                                      * NOTE: Implied for physical mappings.
                                      * NOTE: The kernel may initialize memory randomly in sandboxed threads. */
#endif /* !MAP_UNINITIALIZED */
#endif /* __USE_MISC */

#ifdef __USE_KOS
/* Highly versatile descriptor for mapping memory while providing a
 * large assortment of options to customize locating free memory,
 * as well as how memory should be initialized, and where it should be. */

struct mmap_virt {
 int        mv_file;  /*< [valid_if(!MAP_ANONYMOUS)] File descriptor of the file to map.
                       *   WARNING: The kernel cannot guaranty that file mappings
                       *            remain identical, especially when being modified
                       *            after being mapped.
                       *            In addition, mapping a file descriptor opened for
                       *            writing using `PROT_WRITE' will allow you to modify
                       *            mapped data to find the changes written back to
                       *            disk at some point.
                       *            The KOS kernel however does not make any attempts
                       *            at synchronizing such actions and write-backs may
                       *            be performed out of order, or become otherwise
                       *            corrupted.
                       *         >> Basically, the system can only work perfectly when
                       *            everyone is just reading (as is the case most of the time),
                       *            or when only 1 process is writing (at which point
                       *            changes will appear on-disk at a random point in time).
                       *   In my defense though, I chose to go this route because I've
                       *   never encountered a good reason why write-back file mappings
                       *   are something actually useful: read-only mappings make total
                       *   sense, allowing for lazy loading of data from executables
                       *   only when data is actually being accessed.
                       *   But writing? - I guess it would be cool to create persistent
                       *   application memory, such as a program that saves its configuration
                       *   within its own binary. But other than that: It just seems outdated and lazy.
                       *   And in addition, this change allowed me to remove the annoying restriction
                       *   of (mv_off & (PAGESIZE-1) == 0) unix imposes on file mappings.
                       *   And besides: As long as only once instance is running, write-mappings do fully work!
                       */
 /* NOTE: File mappings are initialized as follows:
  *                 
  *  FILE: abcdefghijklmnopqrtuvwxyzABCDEFGHIJKLMNOPQRTUVWXYZ...
  *        |         |              |
  *        0    mv_off              mv_off+mv_len
  *                  |              |
  *                  +-+            +-+
  *                    |              |
  *  MEM:  XXXXXXXXXXXXklmnopqrtuvwxyzXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  *        |           |              |                                   |
  *        0    mv_begin              mv_begin+mv_len              :mi_size
  * X: Initialized using 'mv_fill' */
 __uintptr_t mv_begin; /*< [valid_if(!MAP_ANONYMOUS)] The address within the region where the file mapping starts. */
union{
 __FS_TYPE(pos)
            mv_off;   /*< [valid_if(!MAP_ANONYMOUS)] Offset into 'mv_file' where mapping starts. */
#ifdef __USE_LARGEFILE64
 __pos64_t  mv_off64; /*< [valid_if(!MAP_ANONYMOUS)] Offset into 'mv_file' where mapping starts (64-bit field). */
#endif
 __pos64_t  __mv_off; /*< Force 64-bit alignment. */
};
 size_t     mv_len;   /*< [valid_if(!MAP_ANONYMOUS)] Max amount of bytes to read from 'mv_file', starting at 'mv_off/mv_off64'.
                       *   Clamped to `mi_size' when greater; when lower, difference is
                       *   _always_ initialized with `mi_fill' (ignoring `MAP_UNINITIALIZED').
                       *   >> Very useful when mapping program headers with a smaller memory-size than file-size. */
 __uintptr_t mv_fill; /*< [valid_if(MAP_ANONYMOUS ? !MAP_UNINITIALIZED : mv_len < mi_size)]
                       *   Filler double/quad-word for any undefined memory.
                       *   On some platforms, a multi-byte pattern described by this may be
                       *   used to initialize memory, but KOS only guaranties that the lowest byte
                       *  (that is `mi_fill & 0xff') will always be used, meaning that a consistent
                       *   initialization of memory is only guarantied when `mi_fill == [0x0101010101010101|0x01010101]*(mi_fill & 0xff)'. */
 size_t     mv_guard; /*< [valid_if(MAP_GROWSDOWN||MAP_GROWSUP)] Size of the guard region in bytes.
                       *   NOTE: When ZERO(0), both the `MAP_GROWSDOWN' and `MAP_GROWSUP' flags are ignored.
                       *   NOTE: Clamped to `mi_size-PAGESIZE' when greater.
                       *   HINT: mmap() has this field set to `PAGESIZE'.
                       *   HINT: When a guard region that uses file initialization is copied,
                       *         both 'mv_off' and 'mv_len' are updated accordingly!
                       */
#define MMAP_VIRT_MAXFUNDS ((__uint16_t)-1)
 __uint16_t mv_funds; /*< [valid_if(MAP_GROWSDOWN||MAP_GROWSUP)] Max amount of times the guard can replicate itself.
                       *   HINT: mmap() has this field set to 'MMAP_VIRT_MAXFUNDS' (to emulate linux behavior)
                       *   >> The total max amount of bytes that can be allocated by the guard is equal to 'mv_funds*mv_guard'. */
 /* XXX: Add some way of receiving signals from guards?
  *     (define a signal number to-be called when guard events occur) */
};

struct mmap_phys {
 __PHYS void *mp_addr;  /*< [valid_if(XMAP_PHYSICAL)] Physical address at which memory should be allocated.
                         *  [REQUIRES(!MAP_FIXED || mp_addr & (PAGESIZE-1) == mi_addr & (PAGESIZE-1))]
                         *   NOTE: When mapping memory at a non-fixed address, sub-page alignments are made to match. */
};


#ifndef XMAP_VIRTUAL
#define XMAP_VIRTUAL   0x00000000 /*< Allocate virtual memory, allowing for custom initializers or file mappings. */
#endif /* !XMAP_VIRTUAL */
#ifndef XMAP_PHYSICAL
#define XMAP_PHYSICAL  0x00000001 /*< Allocate physical memory at a fixed hardware address.
                                   *  NOTE: Using this flag requires the 'CAP_SYS_ADMIN' capability.
                                   *  NOTE: When this mode is used, no memory pre-initialization can be performed!
                                   *        This is pure, low-level hardware access and only meant for ring#3
                                   *        drivers that need access to memory-mapped device buffers or I/O. */
#endif /* !XMAP_PHYSICAL */
#ifndef XMAP_TYPE
#define XMAP_TYPE      0x00000001 /*< Mask for known XMAP types. */
#endif /* !XMAP_TYPE */
#ifndef XMAP_FINDAUTO
#define XMAP_FINDAUTO  0x00000000 /*< [valid_if(!MAP_FIXED)] Find free memory the same way `mmap()' does:
                                   *   When `mi_addr' is NULL, find suitable memory within the thread's user heap/stack.
                                   *   Otherwise, do the same as if both 'XMAP_FINDBELOW' and 'XMAP_FINDABOVE' were set. */
#endif /* !XMAP_FINDAUTO */
#ifndef XMAP_FINDBELOW
#define XMAP_FINDBELOW 0x00000010 /*< [valid_if(!MAP_FIXED)] Use the nearest unused address range above (considering `mi_addr...+=mi_size' first and 'CEIL_ALIGN(mi_addr+mi_size,mi_align)...+=mi_size' next). */
#endif /* !XMAP_FINDBELOW */
#ifndef XMAP_FINDABOVE
#define XMAP_FINDABOVE 0x00000020 /*< [valid_if(!MAP_FIXED)] Use the nearest unused address range below (considering `mi_addr...+=mi_size' first and `FLOOR_ALIGN(mi_addr-mi_size,mi_align)...+=mi_size' next).
                                   *   NOTE: When both 'XMAP_FINDBELOW' and 'XMAP_FINDABOVE' are set, use whichever
                                   *         discovered range (if any) is located closer to `mi_addr...+=mi_size' */
#endif /* !XMAP_FINDABOVE */
#ifndef XMAP_NOTRYNGAP
#define XMAP_NOTRYNGAP 0x00000040 /*< [valid_if(!MAP_FIXED)] When the first attempt to find free memory fails, and
                                   *                         `mi_gap' was non-zero, don't try again without any gap. */
#endif /* !XMAP_NOTRYNGAP */
#ifndef XMAP_FORCEGAP
#define XMAP_FORCEGAP  0x00000080 /*< [valid_if(!MAP_FIXED)] Force a gap before & after all types of memory mappings.
                                   *   When not set, only try to keep a gap before `MAP_GROWSDOWN' and after `MAP_GROWSUP' */
#endif /* !XMAP_FORCEGAP */

/* mmap_info version control. */
#ifndef MMAP_INFO_V1
#define MMAP_INFO_V1      0
#endif /* !MMAP_INFO_V1 */
#ifndef MMAP_INFO_CURRENT
#define MMAP_INFO_CURRENT MMAP_INFO_V1
#endif /* !MMAP_INFO_CURRENT */

#if MMAP_INFO_CURRENT == MMAP_INFO_V1
#ifndef __mmap_info_v1_defined
#ifndef mmap_info_v1
#define mmap_info_v1      mmap_info
#endif /* !mmap_info_v1 */
#else /* !__mmap_info_v1_defined */
#ifndef mmap_info
#define mmap_info         mmap_info_v1
#endif /* !mmap_info */
#endif /* __mmap_info_v1_defined */
#endif /* MMAP_INFO_CURRENT == MMAP_INFO_V1 */

#ifndef __mmap_info_v1_defined
#define __mmap_info_v1_defined 1
struct mmap_info_v1 {
 __uint32_t       mi_prot;  /*< Set of `PROT_*' */
 __uint32_t       mi_flags; /*< Set of `MAP_*' */
 __uint32_t       mi_xflag; /*< Set of 'XMAP_*' */
 __VIRT void     *mi_addr;  /*< Base/hint address.
                             *  NOTE: When `MAP_FIXED' is given, this member must be page-aligned for virtual allocations.
                             *        This differs somewhat when allocating physical memory, in which case it is required
                             *        that the physical address `mi_phys.mp_addr' has the same sub-page alignment as `mi_addr'
                             *     -> Otherwise no such restriction is necessary.
                             *  WARNING: When `MAP_FIXED' is given, `mi_addr+mi_size' must not overflow, or point into kernel-space! */
 size_t           mi_size;  /*< Size of the memory mapping (in bytes)
                             *  NOTE: This member is always ceil-aligned by PAGESIZE internally. */
 size_t           mi_align; /*< [valid_if(!MAP_FIXED)] Minimum alignment required when searching for free ranges.
                             *   NOTE: This value must be a power-of-2 and is ignored when < PAGESIZE. */
 size_t           mi_gap;   /*< [valid_if(!MAP_FIXED)] When searching for suitable addresses and `mi_addr...+=mi_size'
                             *   was already in use, any address range considered there-after must not be closer to another existing range than `mi_gap' bytes.
                             *   HINT: This member is useful for discovering free memory while leaving a gap for guard mappings to expand into.
                             *   HINT: mmap() has sets this argument to 16*PAGESIZE, not setting 'XMAP_NOTRYNGAP'. */
 void            *mi_tag;   /*< A virtual memory mapping tag that can later be re-used to identify this mapping. */
union{
 struct mmap_virt mi_virt;  /*< [valid_if(mi_xflag&XMAP_TYPE == XMAP_VIRTUAL)] Virtual mapping data. */
 struct mmap_phys mi_phys;  /*< [valid_if(mi_xflag&XMAP_TYPE == XMAP_PHYSICAL)] Physical mapping data. */
};
};
#endif /* !__mmap_info_v1_defined */

#ifndef XUNMAP_ALL
#define XUNMAP_ALL  0x00000000 /*< Unmap all mappings, ignoring the given tag. */
#endif /* !XUNMAP_ALL */
#ifndef XUNMAP_TAG
#define XUNMAP_TAG  0x00000010 /*< Only unmap mappings matching the given tag. */
#endif /* !XUNMAP_TAG */
#endif


/* Flags for 'mremap'. */
#ifdef __USE_GNU
#ifndef MREMAP_MAYMOVE
#   define MREMAP_MAYMOVE 1
#endif /* !MREMAP_MAYMOVE */
#ifndef MREMAP_FIXED
#   define MREMAP_FIXED   2
#endif /* !MREMAP_FIXED */
#endif

#ifndef MAP_FAILED
#define MAP_FAILED   ((void *)-1)
#endif /* !MAP_FAILED */

#if !defined(__KERNEL__) && defined(__CC__)
#ifdef __CRT_GLC
__REDIRECT_FS_FUNC(__LIBC,__WUNUSED,void *,__LIBCCALL,mmap,
                  (void *__addr, size_t __len, int __prot, int __flags, int __fd, __FS_TYPE(off) __offset),
                   mmap,(__addr,__len,__prot,__flags,__fd,__offset))
__LIBC int (__LIBCCALL munmap)(void *__addr, size_t __len);
__LIBC int (__LIBCCALL mprotect)(void *__addr, size_t __len, int __prot);
__LIBC int (__LIBCCALL msync)(void *__addr, size_t __len, int __flags);
__LIBC int (__LIBCCALL mlock)(void const *__addr, size_t __len);
__LIBC int (__LIBCCALL munlock)(void const *__addr, size_t __len);
__LIBC int (__LIBCCALL mlockall)(int __flags);
__LIBC int (__LIBCCALL munlockall)(void);
__LIBC int (__LIBCCALL shm_open)(char const *__name, int __oflag, mode_t __mode);
__LIBC int (__LIBCCALL shm_unlink)(char const *__name);
#ifdef __USE_MISC
__LIBC int (__LIBCCALL madvise)(void *__addr, size_t __len, int __advice);
__LIBC int (__LIBCCALL mincore)(void *__start, size_t __len, unsigned char *__vec);
#endif /* __USE_MISC */
#ifdef __USE_LARGEFILE64
__LIBC void *(__LIBCCALL mmap64)(void *__addr, size_t __len, int __prot, int __flags, int __fd, __off64_t __offset);
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_XOPEN2K
__LIBC int (__LIBCCALL posix_madvise)(void *__addr, size_t __len, int __advice);
#endif /* __USE_XOPEN2K */
#ifdef __USE_GNU
__LIBC void *(__ATTR_CDECL mremap)(void *__addr, size_t __old_len, size_t __new_len, int __flags, ...);
__LIBC int (__LIBCCALL remap_file_pages)(void *__start, size_t __size, int __prot, size_t __pgoff, int __flags);
#endif /* __USE_GNU */
#endif /* __CRT_GLC */

#if defined(__USE_KOS) && defined(__CRT_KOS)
__REDIRECT(__LIBC,__PORT_KOSONLY_ALT(mmap),void *,__LIBCCALL,xmmap,(struct mmap_info const *__data),xmmap1,(__data))
__LIBC __PORT_KOSONLY_ALT(munmap) ssize_t (__LIBCCALL xmunmap)(void *__addr, size_t __len, int __flags, void *__tag);
#endif /* __USE_KOS && __CRT_KOS */
#endif /* !__KERNEL__ && __CC__ */

__SYSDECL_END

#endif /* !_SYS_GENERIC_MMAN_H */
