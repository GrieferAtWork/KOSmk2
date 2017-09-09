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
#ifndef GUARD_INCLUDE_KERNEL_EXPORT_H
#define GUARD_INCLUDE_KERNEL_EXPORT_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

struct module;

#ifdef CONFIG_BUILDING_KERNEL_CORE

INTDEF byte_t (PAGE_ALIGNED VIRT __kernel_start)[];
INTDEF byte_t (PAGE_ALIGNED VIRT __kernel_ro_end)[];
INTDEF byte_t (PAGE_ALIGNED VIRT __kernel_rw_start)[];
INTDEF byte_t (PAGE_ALIGNED VIRT __kernel_end)[];

INTDEF byte_t (PAGE_ALIGNED PHYS __kernel_phys_start)[];
INTDEF byte_t (PAGE_ALIGNED PHYS __kernel_phys_end)[];

INTDEF byte_t (PAGE_ALIGNED PHYS __kernel_user_start)[];
INTDEF byte_t (PAGE_ALIGNED PHYS __kernel_user_end)[];

INTDEF byte_t (PAGE_ALIGNED VIRT __kernel_free_start)[];
INTDEF byte_t (PAGE_ALIGNED VIRT __kernel_free_end)[];

INTDEF byte_t (PAGE_ALIGNED __kernel_size)[]; /* __kernel_end - __kernel_start */
INTDEF byte_t (PAGE_ALIGNED __kernel_ro_size)[]; /* __kernel_ro_end - __kernel_start */
INTDEF byte_t (PAGE_ALIGNED __kernel_rw_size)[]; /* __kernel_end - __kernel_rw_start */
INTDEF byte_t (PAGE_ALIGNED __kernel_phys_size)[]; /* __kernel_phys_end - __kernel_phys_start */
INTDEF byte_t (PAGE_ALIGNED __kernel_user_size)[]; /* __kernel_user_end - __kernel_user_start */
INTDEF byte_t (PAGE_ALIGNED __kernel_free_size)[]; /* __kernel_free_end - __kernel_free_start */
INTDEF byte_t (PAGE_ALIGNED __kernel_after_end)[]; /* KERNEL_GLOBAL_END - __kernel_end */

/* Helper macros for kernel layout information.
 * NOTE: All addresses are virtual (>= KERNEL_BASE) */
#define KERNEL_BEGIN      (PAGE_ALIGNED VIRT uintptr_t)__kernel_start
#define KERNEL_END        (PAGE_ALIGNED VIRT uintptr_t)__kernel_end
#define KERNEL_RO_BEGIN   (PAGE_ALIGNED VIRT uintptr_t)__kernel_start
#define KERNEL_RO_END     (PAGE_ALIGNED VIRT uintptr_t)__kernel_ro_end
#define KERNEL_RW_BEGIN   (PAGE_ALIGNED VIRT uintptr_t)__kernel_rw_start
#define KERNEL_RW_END     (PAGE_ALIGNED VIRT uintptr_t)__kernel_end

/* Begin/end of the physical .data section.
 * NOTE: This is where pdir_kernel (the physical version) are stored. */
#define KERNEL_PHYS_BEGIN (PAGE_ALIGNED PHYS uintptr_t)__kernel_phys_start
#define KERNEL_PHYS_END   (PAGE_ALIGNED PHYS uintptr_t)__kernel_phys_end

#define KERNEL_SIZE       (PAGE_ALIGNED size_t)__kernel_size
#define KERNEL_RO_SIZE    (PAGE_ALIGNED size_t)__kernel_ro_size
#define KERNEL_RW_SIZE    (PAGE_ALIGNED size_t)__kernel_rw_size
#define KERNEL_PHYS_SIZE  (PAGE_ALIGNED size_t)__kernel_phys_size
#define KERNEL_FREE_SIZE  (PAGE_ALIGNED size_t)__kernel_free_size
#define KERNEL_AFTER_END  (PAGE_ALIGNED size_t)__kernel_after_end


INTDEF void (*__init_array_start[])(void);
INTDEF void (*__init_array_end[])(void);
INTDEF void (*__fini_array_start[])(void);
INTDEF void (*__fini_array_end[])(void);

#define KERNEL_RUN_CONSTRUCTORS() \
do{ void (**iter)(void),(**end)(void); \
    iter = __init_array_start; \
    end  = __init_array_end; \
    for (; iter != end; ++iter) (**iter)(); \
}while(0)
#define KERNEL_RUN_DESTRUCTORS() \
do{ void (**begin)(void),(**iter)(void); \
    begin = __fini_array_start; \
    iter  = __fini_array_end; \
    while (iter-- != begin) (**iter)(); \
}while(0)
#endif

#define MODULE_INIT  __attribute__((__section__(".text.free"),__used__,__constructor__))
#define MODULE_FINI  __attribute__((__section__(".text.cold"),__used__,__destructor__))

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_EXPORT_H */
