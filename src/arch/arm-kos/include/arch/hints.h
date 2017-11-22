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
#ifndef GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_HINTS_H
#define GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_HINTS_H 1

#include <hybrid/compiler.h>
#include <hybrid/typecore.h>
#include <hybrid/limits.h>

DECL_BEGIN

#undef CONFIG_LOW_KERNEL
#undef CONFIG_HIGH_KERNEL
/* Suitable for `-M integratorcp' */
#define VM_USER_BASE_A  0x40000000
#define VM_USER_SIZE_A  0xc0000000
#define VM_USER_MAX_A   0xffffffff
#define VM_HOST_BASE_A  0x00000000
#define VM_HOST_SIZE_A  0x40000000
#define VM_HOST_MAX_A   0x3fffffff
#define VM_CORE_BASE_A  0x00000000
#define VM_CORE_SIZE_A  0x40000000
#define VM_CORE_MAX_A   0x3fffffff
#define VM_USER_BASE    __UINTPTR_C(0x40000000) /*< 0x40000000. */
#define VM_USER_SIZE    __UINTPTR_C(0xc0000000) /*< 0xc0000000. */
#define VM_USER_MAX     __UINTPTR_C(0xffffffff) /*< 0xffffffff. */
#define VM_HOST_BASE    __UINTPTR_C(0x00000000) /*< 0x00000000. */
#define VM_HOST_SIZE    __UINTPTR_C(0x40000000) /*< 0x40000000. */
#define VM_HOST_MAX     __UINTPTR_C(0x3fffffff) /*< 0x3fffffff. */
#define VM_CORE_BASE    VM_HOST_BASE            /*< 0x00000000. */
#define VM_CORE_SIZE    VM_HOST_SIZE            /*< 0x40000000. */
#define VM_CORE_MAX     VM_HOST_MAX             /*< 0x3fffffff. */
#define CONFIG_LOW_KERNEL 1




#define HOST_STCK_ALIGN                                  16  /* Alignment hint that should be respected by all host-stack allocators. */
#define HOST_STCK_SIZE                               0x4000  /* Default, generic size for host stacks. */
#define HOST_BOOT_STCKSIZE                           0x4000  /* For reference: The size of the boot stack. */
#define HOST_IDLE_STCKSIZE                           0x4000  /* For reference: The size of IDLE-thread stacks. */
#ifndef CONFIG_NO_JOBS
#define HOST_WOKER_STCKSIZE                          0x4000  /* For reference: The size of per-cpu WORK-thread stacks. */
#endif /* !CONFIG_NO_JOBS */

#define USER_MODULE_STATIC_ADDRHINT  (VM_USER_BASE+__UINTPTR_C(0x08000000)) /* Default base address of user-space applications. */
#define USER_MODULE_DYNAMIC_ADDRHINT (VM_USER_BASE+__UINTPTR_C(0x20000000)) /* Address hint for mapping relocatable user-space modules. */
#define USER_HEAP_ADDRHINT           (VM_USER_BASE+__UINTPTR_C(0x40000000)) /* Initial value for `struct man::m_uheap' (Grows up) */
#define USER_STCK_ADDRHINT           (VM_USER_BASE+__UINTPTR_C(0x90000000)) /* Initial value for `struct man::m_ustck' (Grows down) */
#define USER_TASK_TLB_ADDRHINT       (VM_USER_BASE+__UINTPTR_C(0xa0000000)) /* Address hint for the per-thread TLB/TIB block. (Grows down) */
#define USER_ENVIRON_ADDRHINT        (VM_USER_MAX_A-((4*PAGESIZE)-1))       /* Address hint when attempting to place the appenv data block in user-space. (Grows down) */
#define USER_STCK_BASESIZE                                         0x4000   /* Default, generic size for user stacks. */
#define USER_STCK_ALIGN                                                16   /* Alignment hint that should be respected by all user-stack allocators. */
#define USER_STCK_ADDRGAP                                    (16*PAGESIZE)  /* Allocation gap size when initially reserving space for a user-space stack.
                                                                             * NOTE: If no free memory region can be found that fits this gap, the search
                                                                             *       is attempted a second time while ignoring this hint. */
#define USER_STCK_FUNDS                                                 8   /* Default amount of funding for guard-pages in user-space stacks. */
#define USER_STCK_GUARDSIZE                                      PAGESIZE   /* Default user-space stack guard size. */
#define USER_REDZONE_SIZE                                               0   /* Size of the so-called `red' zone (Basically, a memory gap left on user-stacks when a signal handler is executed) */

#define HOST_HEAPEND_KERNEL_LOCKED   (VM_HOST_BASE+__UINTPTR_C(0x10000000)) /* Start address of the GFP_KERNEL|GFP_LOCKED heap. (Grows up) */
#define HOST_HEAPEND_KERNEL          (VM_HOST_BASE+__UINTPTR_C(0x14000000)) /* Start address of the GFP_KERNEL heap. (Grows up) */
#define HOST_USHARE_WRITE_ADDRHINT   (VM_HOST_BASE+__UINTPTR_C(0x00000000)) /* Address hint for mapping the writable copy of the user-share segment. (Grows down) */
#define HOST_DRIVER_ADDRHINT         (VM_HOST_BASE+__UINTPTR_C(0x02000000)) /* Address hint for mapping drivers in kernel-space. (Grows up) */
#define HOST_HEAPEND_SHARED_LOCKED   (VM_HOST_BASE+__UINTPTR_C(0x10000000)) /* Start address of the GFP_SHARED|GFP_LOCKED heap. VM_HOST_BASE+0x10000000 (Grows up) */
#define HOST_HEAPEND_SHARED          (VM_HOST_BASE+__UINTPTR_C(0x14000000)) /* Start address of the GFP_SHARED heap. VM_HOST_BASE+0x18000000 (Grows up) */
#define HOST_DEVICE_ADDRHINT         (VM_HOST_BASE+__UINTPTR_C(0x22000000)) /* Address hint for device memory mappings. (Grows up) */
#define HOST_MEMORY_ADDRHINT         (VM_HOST_BASE+__UINTPTR_C(0x24000000)) /* Address hint for misc. memory mappings. (Grows up) */
#define HOST_STCK_ADDRHINT           (VM_HOST_BASE+__UINTPTR_C(0x3fc00000)) /* Address hint for allocating kernel stacks. (HINT: Corresponds with the start of the page-directory self-mapping) (Grows down) */

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
#define HEAP_DEFAULT_OVERALLOC(name)    ((name) != GFP_MEMORY ? PAGESIZE*16 : 0)
#define HEAP_DEFAULT_FREETHRESH(name)   ((name) != GFP_MEMORY ? PAGESIZE*16 : PAGESIZE)

/* Weakly enforced limit on modules provided by the bootloader. */
#define BOOTLOADER_MAX_MODULE_COUNT  1024

DECL_END

#endif /* !GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_HINTS_H */
