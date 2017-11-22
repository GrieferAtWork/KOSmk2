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
#ifndef GUARD_ARCH_I386_KOS_INCLUDE_ARCH_HINTS_H
#define GUARD_ARCH_I386_KOS_INCLUDE_ARCH_HINTS_H 1

#include <hybrid/compiler.h>
#include <hybrid/typecore.h>
#include <hybrid/limits.h>
#include <hybrid/host.h>

DECL_BEGIN

#undef CONFIG_LOW_KERNEL
#undef CONFIG_HIGH_KERNEL
#ifdef __x86_64__
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
 * 0000000000000000 - 00007FFFFFFFFFFF  USER
 * --- Hole due to bit#48 -> 49..63 sign extension.
 * FFFF800000000000 - FFFFFFFF7FFFFFFF  HOST (Extended Heap, file mappings, modules, etc)
 * FFFFFFFF80000000 - FFFFFFFFFFFFFFFF  CORE (Core, Heap, drivers. Permanently mapped to the first 2Gb of physical memory)
 * This range contains the kernel core and is later truncated to end past the
 * core itself (`__kernel_end'), leaving a one-on-one physical mappings of the first
 * Mb of physical memory intact (Which is used by e.g.: The core's VGA-TTY driver)
 * HINT: In this configuration, user and kernel-space are of equal size.
 */
#define VM_USER_BASE_A              0x0000000000000000
#define VM_USER_SIZE_A              0x00007fffffffffff
#define VM_USER_MAX_A               0x00007fffffffffff
#define VM_HOST_BASE_A              0xffff800000000000
#define VM_HOST_SIZE_A              0x00007fffffffffff
#define VM_HOST_MAX_A               0xffffffffffffffff
#define VM_CORE_BASE_A              0xffffffff80000000 /* -2Gb */
#define VM_CORE_SIZE_A              0x0000000080000000
#define VM_CORE_MAX_A               0xffffffffffffffff
#define VM_USER_BASE    __UINTPTR_C(0x0000000000000000)
#define VM_USER_SIZE    __UINTPTR_C(0x00007fffffffffff)
#define VM_USER_MAX     __UINTPTR_C(0x00007fffffffffff)
#define VM_HOST_BASE    __UINTPTR_C(0xffff800000000000)
#define VM_HOST_SIZE    __UINTPTR_C(0x00007fffffffffff)
#define VM_HOST_MAX     __UINTPTR_C(0xffffffffffffffff)
#define VM_CORE_BASE    __UINTPTR_C(0xffffffff80000000)
#define VM_CORE_SIZE    __UINTPTR_C(0x0000000080000000)
#define VM_CORE_MAX     __UINTPTR_C(0xffffffffffffffff)
#else
/* The virtual base address of the kernel itself!
 * WARNING: Changing this values requires additional changes in:
 * >> /src/kernel/core/paging.c:  pdir_kernel
 * >> /src/kernel/include/mman.h: ZONE_* (Comments)
 * WARNING: The kernel may never map addresses lower than this to itself!
 * All addresses lower than this are usually unmapped, with the exception
 * of special memory regions that encompass physical memory, which
 * map 1 on 1 to kernel virtual addresses.
 * As a consequence of this, KOS is (by default) limited to only using
 * 3GB of physically available memory, as anything higher cannot actually
 * be used (due to the associated address space being reserved by the kernel).
 * TODO: Add a config that still allows use of physical memory above 3Gb,
 *       adding a new memory zone who's memory cannot be accessed directly,
 *       and is therefor managed by special code that must be deleted during
 *       boot when it is detected that no physical memory above 3Gb exists,
 *       as well as minor changes to mman ALLOA to make use of such memory
 *       for userspace applications/virtual kernel memory.
 */
#define VM_USER_BASE_A              0x00000000
#define VM_USER_SIZE_A              0xc0000000
#define VM_USER_MAX_A               0xbfffffff
#define VM_HOST_BASE_A              0xc0000000
#define VM_HOST_SIZE_A              0x40000000
#define VM_HOST_MAX_A               0xffffffff
#define VM_CORE_BASE_A              0xc0000000
#define VM_CORE_SIZE_A              0x40000000
#define VM_CORE_MAX_A               0xffffffff
#define VM_USER_BASE    __UINTPTR_C(0x00000000)
#define VM_USER_SIZE    __UINTPTR_C(0xc0000000)
#define VM_USER_MAX     __UINTPTR_C(0xbfffffff)
#define VM_HOST_BASE    __UINTPTR_C(0xc0000000)
#define VM_HOST_SIZE    __UINTPTR_C(0x40000000)
#define VM_HOST_MAX     __UINTPTR_C(0xffffffff)
#define VM_CORE_BASE    __UINTPTR_C(0xc0000000)
#define VM_CORE_SIZE    __UINTPTR_C(0x40000000)
#define VM_CORE_MAX     __UINTPTR_C(0xffffffff)
#endif
#define CONFIG_HIGH_KERNEL 1




#ifdef __x86_64__

#define HOST_STCK_ALIGN                                          32  /* Alignment hint that should be respected by all host-stack allocators. */
#define HOST_STCK_SIZE                                       0x8000  /* Default, generic size for host stacks. */
#define HOST_BOOT_STCKSIZE                                   0x8000  /* For reference: The size of the boot stack. */
#define HOST_IDLE_STCKSIZE                                   0x8000  /* For reference: The size of IDLE-thread stacks. */
#ifndef CONFIG_NO_JOBS
#define HOST_WOKER_STCKSIZE                                  0x8000  /* For reference: The size of per-cpu WORK-thread stacks. */
#endif /* !CONFIG_NO_JOBS */

#define USER_MODULE_STATIC_ADDRHINT  __UINTPTR_C(0x0000000008000000) /* Default base address of user-space applications. (Grows up) */
#define USER_MODULE_DYNAMIC_ADDRHINT_GROWS_DOWN 1
#define USER_MODULE_DYNAMIC_ADDRHINT __UINTPTR_C(0x0000000100000000) /* Address hint for mapping relocatable user-space modules. (Grows down) */
#define USER_HEAP_ADDRHINT           __UINTPTR_C(0x0000100000000000) /* Initial value for `struct man::m_uheap' (Grows up) */
#define USER_STCK_ADDRHINT           __UINTPTR_C(0x00007ff000000000) /* Initial value for `struct man::m_ustck' (Grows down) */
#define USER_TASK_TLB_ADDRHINT       __UINTPTR_C(0x00007fff00000000) /* Address hint for the per-thread TLB/TIB block. (Grows down) */
#define USER_ENVIRON_ADDRHINT       (__UINTPTR_C(0x0000800000000000)-(64*PAGESIZE)) /* Address hint when attempting to place the appenv data block in user-space. (Grows down) */
#define USER_STCK_BASESIZE                                   0x8000  /* Default, generic size for user stacks. */
#define USER_STCK_ALIGN                                          32  /* Alignment hint that should be respected by all user-stack allocators. */
#define USER_STCK_ADDRGAP                              (64*PAGESIZE) /* Allocation gap size when initially reserving space for a user-space stack.
                                                                      * NOTE: If no free memory region can be found that fits this gap, the search
                                                                      *       is attempted a second time while ignoring this hint. */
#define USER_STCK_FUNDS                                          16  /* Default amount of funding for guard-pages in user-space stacks. */
#define USER_STCK_GUARDSIZE                                PAGESIZE  /* Default user-space stack guard size. */
#define USER_REDZONE_SIZE                                       128  /* Size of the so-called `red' zone (Basically, a memory gap left on user-stacks when a signal handler is executed)
                                                                      * The 128 bytes specified here are mandated by the SysV ABI standard (Which KOS tries to be compatible with). */

#define HOST_HEAPEND_KERNEL_LOCKED   __UINTPTR_C(0x0000500000000000) /* Start address of the GFP_KERNEL|GFP_LOCKED heap. (Grows up) */
#define HOST_HEAPEND_KERNEL          __UINTPTR_C(0x0000510000000000) /* Start address of the GFP_KERNEL heap. (Grows up) */
#define HOST_USHARE_WRITE_ADDRHINT   __UINTPTR_C(0x0000800000000000) /* Address hint for mapping the writable copy of the user-share segment. (Grows down) */
#define HOST_DEVICE_ADDRHINT         __UINTPTR_C(0xffffa00000000000) /* Address hint for device memory mappings. (Grows up) */
#define HOST_MEMORY_ADDRHINT         __UINTPTR_C(0xffffb00000000000) /* Address hint for misc. memory mappings. (Grows up) */
#define HOST_HEAPEND_SHARED_LOCKED   __UINTPTR_C(0xffffc00000000000) /* Start address of the GFP_SHARED|GFP_LOCKED heap. VM_HOST_BASE+0x0000500000000000 (Grows up) */
#define HOST_HEAPEND_SHARED          __UINTPTR_C(0xffffc10000000000) /* Start address of the GFP_SHARED heap. VM_HOST_BASE+0x0000510000000000 (Grows up) */
#define HOST_STCK_ADDRHINT           __UINTPTR_C(0xfffff00000000000) /* Address hint for allocating kernel stacks. (HINT: Corresponds with the start of the page-directory self-mapping) (Grows down) */
#define HOST_DRIVER_ADDRHINT         __UINTPTR_C(0xfffff00000000000) /* Address hint for mapping drivers in kernel-space. (Grows up) */
/* TODO: Once pdir self-mappings are re-implemented, update `HOST_STCK_ADDRHINT' */

#else /* __x86_64__ */

#define HOST_STCK_ALIGN                                  16  /* Alignment hint that should be respected by all host-stack allocators. */
#define HOST_STCK_SIZE                               0x4000  /* Default, generic size for host stacks. */
#define HOST_BOOT_STCKSIZE                           0x4000  /* For reference: The size of the boot stack. */
#define HOST_IDLE_STCKSIZE                           0x4000  /* For reference: The size of IDLE-thread stacks. */
#ifndef CONFIG_NO_JOBS
#define HOST_WOKER_STCKSIZE                          0x4000  /* For reference: The size of per-cpu WORK-thread stacks. */
#endif /* !CONFIG_NO_JOBS */

#define USER_MODULE_STATIC_ADDRHINT  __UINTPTR_C(0x08000000) /* Default base address of user-space applications. */
#define USER_MODULE_DYNAMIC_ADDRHINT __UINTPTR_C(0x20000000) /* Address hint for mapping relocatable user-space modules. */
#define USER_HEAP_ADDRHINT           __UINTPTR_C(0x40000000) /* Initial value for `struct man::m_uheap' (Grows up) */
#define USER_STCK_ADDRHINT           __UINTPTR_C(0x90000000) /* Initial value for `struct man::m_ustck' (Grows down) */
#define USER_TASK_TLB_ADDRHINT       __UINTPTR_C(0xa0000000) /* Address hint for the per-thread TLB/TIB block. (Grows down) */
#define USER_ENVIRON_ADDRHINT       (__UINTPTR_C(0xc0000000)-(4*PAGESIZE)) /* Address hint when attempting to place the appenv data block in user-space. (Grows down) */
#define USER_STCK_BASESIZE                           0x4000  /* Default, generic size for user stacks. */
#define USER_STCK_ALIGN                                  16  /* Alignment hint that should be respected by all user-stack allocators. */
#define USER_STCK_ADDRGAP                      (16*PAGESIZE) /* Allocation gap size when initially reserving space for a user-space stack.
                                                              * NOTE: If no free memory region can be found that fits this gap, the search
                                                              *       is attempted a second time while ignoring this hint. */
#define USER_STCK_FUNDS                                   8  /* Default amount of funding for guard-pages in user-space stacks. */
#define USER_STCK_GUARDSIZE                        PAGESIZE  /* Default user-space stack guard size. */
#define USER_REDZONE_SIZE                                 0  /* Size of the so-called `red' zone (Basically, a memory gap left on user-stacks when a signal handler is executed) */

#define HOST_HEAPEND_KERNEL_LOCKED   __UINTPTR_C(0x10000000) /* Start address of the GFP_KERNEL|GFP_LOCKED heap. (Grows up) */
#define HOST_HEAPEND_KERNEL          __UINTPTR_C(0x14000000) /* Start address of the GFP_KERNEL heap. (Grows up) */
#define HOST_USHARE_WRITE_ADDRHINT   __UINTPTR_C(0xc0000000) /* Address hint for mapping the writable copy of the user-share segment. (Grows down) */
#define HOST_DRIVER_ADDRHINT         __UINTPTR_C(0xc2000000) /* Address hint for mapping drivers in kernel-space. (Grows up) */
#define HOST_HEAPEND_SHARED_LOCKED   __UINTPTR_C(0xd0000000) /* Start address of the GFP_SHARED|GFP_LOCKED heap. VM_HOST_BASE+0x10000000 (Grows up) */
#define HOST_HEAPEND_SHARED          __UINTPTR_C(0xd4000000) /* Start address of the GFP_SHARED heap. VM_HOST_BASE+0x18000000 (Grows up) */
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

#endif /* !GUARD_ARCH_I386_KOS_INCLUDE_ARCH_HINTS_H */
