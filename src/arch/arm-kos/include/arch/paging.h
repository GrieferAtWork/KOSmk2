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
#ifndef GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_PAGING_H
#define GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_PAGING_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

#define ASM_USER_MAX               0x7fffffff
#define ASM_USER_END               0x80000000
#define ASM_KERNEL_BASE            0x80000000
#define ASM_CORE_BASE              0x80000000
#define USER_MAX        __UINT32_C(0x7fffffff)
#define USER_END        __UINT32_C(0x80000000)
#define KERNEL_BASE     __UINT32_C(0x80000000)
#define CORE_BASE       __UINT32_C(0x80000000)

/* Mask of all address bits that can actually be used.
 * NOTE: On i386, that simply is every bit there is (All 32). */
#define VIRT_MASK       __UINT32_C(0xffffffff)



#define PDIR_ATTR_PRESENT 0
#define PDIR_ATTR_WRITE   0
#ifdef __CC__
typedef u32 pdir_attr_t;
typedef struct _pdir pdir_t;
#endif /* __CC__ */


/* TODO */

#define PDIR_SIZE  8
#define PDIR_ALIGN 8
#ifdef __CC__
struct _pdir {
 int pd_directory[2];
};
#endif /* __CC__ */

#define PDIR_KERNELBASE_STARTINDEX  0
#define PDIR_ROOTENTRY_REPRSIZE     0x80000000

#ifdef __CC__
LOCAL KPD PHYS void *KCALL pdir_translate(pdir_t *__restrict self, VIRT void *ptr) { return ptr; }
LOCAL KPD PHYS int KCALL pdir_test_readable(pdir_t *__restrict self, VIRT void *ptr) { return 1; }
LOCAL KPD PHYS int KCALL pdir_test_writable(pdir_t *__restrict self, VIRT void *ptr) { return 1; }
LOCAL void FCALL pdir_flushall(void) { }
#define PDIR_GTCURR()  (&pdir_kernel)
#define PDIR_STCURR(v) (void)0
#endif /* __CC__ */


DECL_END

#endif /* !GUARD_ARCH_I386_KOS_INCLUDE_ARCH_PAGING_H */
