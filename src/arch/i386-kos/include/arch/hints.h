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
#ifndef GUARD_INCLUDE_ARCH_HINTS_H
#define GUARD_INCLUDE_ARCH_HINTS_H 1

#include <hybrid/compiler.h>
#include <hybrid/typecore.h>
#include <hybrid/limits.h>
#include <hybrid/host.h>

DECL_BEGIN

#ifdef __x86_64__

#define HOST_STCK_ALIGN                                 __SIZE_C(32) /* Alignment hint that should be respected by all host-stack allocators. */
#define HOST_STCK_SIZE                              __SIZE_C(0x8000) /* Default, generic size for host stacks. */
#define HOST_BOOT_STCKSIZE                          __SIZE_C(0x8000) /* For reference: The size of the boot stack. */
#define HOST_IDLE_STCKSIZE                          __SIZE_C(0x8000) /* For reference: The size of IDLE-thread stacks. */
#ifndef CONFIG_NO_JOBS
#define HOST_WOKER_STCKSIZE                         __SIZE_C(0x8000) /* For reference: The size of per-cpu WORK-thread stacks. */
#endif /* !CONFIG_NO_JOBS */

#define USER_MODULE_STATIC_ADDRHINT  __UINTPTR_C(0x0000000008000000) /* Default base address of user-space applications. (Grows up) */
#define USER_MODULE_DYNAMIC_ADDRHINT_GROWS_DOWN 1
#define USER_MODULE_DYNAMIC_ADDRHINT __UINTPTR_C(0x0000000100000000) /* Address hint for mapping relocatable user-space modules. (Grows down) */
#define USER_HEAP_ADDRHINT           __UINTPTR_C(0x0000100000000000) /* Initial value for `struct man::m_uheap' (Grows up) */
#define USER_STCK_ADDRHINT           __UINTPTR_C(0x00007ff000000000) /* Initial value for `struct man::m_ustck' (Grows down) */
#define USER_TASK_TLB_ADDRHINT       __UINTPTR_C(0x00007fff00000000) /* Address hint for the per-thread TLB/TIB block. (Grows down) */
#define USER_ENVIRON_ADDRHINT       (__UINTPTR_C(0x0000800000000000)-(64*PAGESIZE)) /* Address hint when attempting to place the appenv data block in user-space. (Grows down) */
#define USER_STCK_BASESIZE                          __SIZE_C(0x8000) /* Default, generic size for user stacks. */
#define USER_STCK_ALIGN                                 __SIZE_C(32) /* Alignment hint that should be respected by all user-stack allocators. */
#define USER_STCK_ADDRGAP                              (64*PAGESIZE) /* Allocation gap size when initially reserving space for a user-space stack.
                                                                      * NOTE: If no free memory region can be found that fits this gap, the search
                                                                      *       is attempted a second time while ignoring this hint. */
#define USER_STCK_FUNDS                                 __SIZE_C(16) /* Default amount of funding for guard-pages in user-space stacks. */
#define USER_STCK_GUARDSIZE                                PAGESIZE  /* Default user-space stack guard size. */
#define USER_REDZONE_SIZE                              __SIZE_C(128) /* Size of the so-called `red' zone (Basically, a memory gap left on user-stacks when a signal handler is executed)
                                                                      * The 128 bytes specified here are mandated by the SysV ABI standard (Which KOS tries to be compatible with). */

#define HOST_HEAPEND_KERNEL_LOCKED   __UINTPTR_C(0x0000500000000000) /* Start address of the GFP_KERNEL|GFP_LOCKED heap. (Grows up) */
#define HOST_HEAPEND_KERNEL          __UINTPTR_C(0x0000510000000000) /* Start address of the GFP_KERNEL heap. (Grows up) */
#define HOST_USHARE_WRITE_ADDRHINT   __UINTPTR_C(0x0000800000000000) /* Address hint for mapping the writable copy of the user-share segment. (Grows down) */
#define HOST_DEVICE_ADDRHINT         __UINTPTR_C(0xffffa00000000000) /* Address hint for device memory mappings. (Grows up) */
#define HOST_MEMORY_ADDRHINT         __UINTPTR_C(0xffffb00000000000) /* Address hint for misc. memory mappings. (Grows up) */
#define HOST_HEAPEND_SHARED_LOCKED   __UINTPTR_C(0xffffc00000000000) /* Start address of the GFP_SHARED|GFP_LOCKED heap. KERNEL_BASE+0x0000500000000000 (Grows up) */
#define HOST_HEAPEND_SHARED          __UINTPTR_C(0xffffc10000000000) /* Start address of the GFP_SHARED heap. KERNEL_BASE+0x0000510000000000 (Grows up) */
#define HOST_STCK_ADDRHINT           __UINTPTR_C(0xfffff00000000000) /* Address hint for allocating kernel stacks. (HINT: Corresponds with the start of the page-directory self-mapping) (Grows down) */
#define HOST_DRIVER_ADDRHINT         __UINTPTR_C(0xfffff00000000000) /* Address hint for mapping drivers in kernel-space. (Grows up) */
/* TODO: Once pdir self-mappings are re-implemented, update `HOST_STCK_ADDRHINT' */

#else /* __x86_64__ */

#define HOST_STCK_ALIGN                         __SIZE_C(16) /* Alignment hint that should be respected by all host-stack allocators. */
#define HOST_STCK_SIZE                      __SIZE_C(0x4000) /* Default, generic size for host stacks. */
#define HOST_BOOT_STCKSIZE                  __SIZE_C(0x4000) /* For reference: The size of the boot stack. */
#define HOST_IDLE_STCKSIZE                  __SIZE_C(0x4000) /* For reference: The size of IDLE-thread stacks. */
#ifndef CONFIG_NO_JOBS
#define HOST_WOKER_STCKSIZE                 __SIZE_C(0x4000) /* For reference: The size of per-cpu WORK-thread stacks. */
#endif /* !CONFIG_NO_JOBS */

#define USER_MODULE_STATIC_ADDRHINT  __UINTPTR_C(0x08000000) /* Default base address of user-space applications. */
#define USER_MODULE_DYNAMIC_ADDRHINT __UINTPTR_C(0x20000000) /* Address hint for mapping relocatable user-space modules. */
#define USER_HEAP_ADDRHINT           __UINTPTR_C(0x40000000) /* Initial value for `struct man::m_uheap' (Grows up) */
#define USER_STCK_ADDRHINT           __UINTPTR_C(0x70000000) /* Initial value for `struct man::m_ustck' (Grows down) */
#define USER_TASK_TLB_ADDRHINT       __UINTPTR_C(0xa0000000) /* Address hint for the per-thread TLB/TIB block. (Grows down) */
#define USER_ENVIRON_ADDRHINT       (__UINTPTR_C(0xc0000000)-(4*PAGESIZE)) /* Address hint when attempting to place the appenv data block in user-space. (Grows down) */
#define USER_STCK_BASESIZE                  __SIZE_C(0x4000) /* Default, generic size for user stacks. */
#define USER_STCK_ALIGN                         __SIZE_C(16) /* Alignment hint that should be respected by all user-stack allocators. */
#define USER_STCK_ADDRGAP                      (16*PAGESIZE) /* Allocation gap size when initially reserving space for a user-space stack.
                                                              * NOTE: If no free memory region can be found that fits this gap, the search
                                                              *       is attempted a second time while ignoring this hint. */
#define USER_STCK_FUNDS                          __SIZE_C(8) /* Default amount of funding for guard-pages in user-space stacks. */
#define USER_STCK_GUARDSIZE                        PAGESIZE  /* Default user-space stack guard size. */
#define USER_REDZONE_SIZE                        __SIZE_C(0) /* Size of the so-called `red' zone (Basically, a memory gap left on user-stacks when a signal handler is executed) */

#define HOST_HEAPEND_KERNEL_LOCKED   __UINTPTR_C(0x10000000) /* Start address of the GFP_KERNEL|GFP_LOCKED heap. (Grows up) */
#define HOST_HEAPEND_KERNEL          __UINTPTR_C(0x14000000) /* Start address of the GFP_KERNEL heap. (Grows up) */
#define HOST_USHARE_WRITE_ADDRHINT   __UINTPTR_C(0xc0000000) /* Address hint for mapping the writable copy of the user-share segment. (Grows down) */
#define HOST_DRIVER_ADDRHINT         __UINTPTR_C(0xc2000000) /* Address hint for mapping drivers in kernel-space. (Grows up) */
#define HOST_HEAPEND_SHARED_LOCKED   __UINTPTR_C(0xd0000000) /* Start address of the GFP_SHARED|GFP_LOCKED heap. KERNEL_BASE+0x10000000 (Grows up) */
#define HOST_HEAPEND_SHARED          __UINTPTR_C(0xd4000000) /* Start address of the GFP_SHARED heap. KERNEL_BASE+0x18000000 (Grows up) */
#define HOST_DEVICE_ADDRHINT         __UINTPTR_C(0xe2000000) /* Address hint for device memory mappings. (Grows up) */
#define HOST_MEMORY_ADDRHINT         __UINTPTR_C(0xe4000000) /* Address hint for misc. memory mappings. (Grows up) */
#define HOST_STCK_ADDRHINT           __UINTPTR_C(0xffc00000) /* Address hint for allocating kernel stacks. (HINT: Corresponds with the start of the page-directory self-mapping) (Grows down) */

#endif /* !__x86_64__ */

#define HOST_SMPCPU_ADDRHINT  KERNEL_END  /* Address hint for mapping per-cpu segments for additional cores. */

/* Heap overallocation/free-threshold hints. */
/* [GFP_MEMORY]: Don't overallocate, or wait before releasing physical memory.
 *            >> Since physical memory is the most valuable resouces, as all
 *               other types of memory make use of it to implement actual
 *               in-core data, don't overallocate, and don't wait before
 *               releasing allocated memory.
 *         NOTE: Doing so does not really degrade performance, as physical
 *               memory allocation is fairly fast to begin with, as well
 *               as being designed to be nearly atomic. */
#ifdef __x86_64__
#define HEAP_DEFAULT_OVERALLOC(name)    ((name) != GFP_MEMORY ? PAGESIZE*32 : 0)
#define HEAP_DEFAULT_FREETHRESH(name)   ((name) != GFP_MEMORY ? PAGESIZE*32 : PAGESIZE)
#else /* __x86_64__ */
#define HEAP_DEFAULT_OVERALLOC(name)    ((name) != GFP_MEMORY ? PAGESIZE*16 : 0)
#define HEAP_DEFAULT_FREETHRESH(name)   ((name) != GFP_MEMORY ? PAGESIZE*16 : PAGESIZE)
#endif /* !__x86_64__ */


/* Weakly enforced limit on modules provided by the bootloader. */
#define BOOTLOADER_MAX_MODULE_COUNT  1024


DECL_END

#endif /* !GUARD_INCLUDE_ARCH_HINTS_H */
