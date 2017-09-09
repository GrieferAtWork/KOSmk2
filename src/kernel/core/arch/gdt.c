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
#ifndef GUARD_KERNEL_CORE_ARCH_GDT_C
#define GUARD_KERNEL_CORE_ARCH_GDT_C 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/arch/tss.h>
#include <kernel/arch/gdt.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <assert.h>
#include <stdint.h>

DECL_BEGIN

#ifndef __INTELLISENSE__
STATIC_ASSERT(sizeof(struct segment) == 8);
#endif

/* Allocate/Free/Update a (new) descriptor index within global descriptor table.
 * These are mainly used to implement the higher-level LDT table and its functions.
 * @return: KSEG_NULL: Failed to allocate a new segment. */
PUBLIC SAFE segid_t KCALL
gdt_alloc(struct segment const *seg) {
 /* TODO */
 return SEG_NULL;
}
PUBLIC SAFE void KCALL
gdt_free(segid_t id) {
 /* TODO */
}
PUBLIC SAFE void KCALL
gdt_update(segid_t id, struct segment const *seg) {
 /* TODO */
}

INTDEF byte_t __kernel_seg_cputss_lo[];
INTDEF byte_t __kernel_seg_cputss_hi[];
INTDEF byte_t __kernel_seg_cpuself_lo[];
INTDEF byte_t __kernel_seg_cpuself_hi[];
  
INTERN CPU_DATA struct segment cpu_gdt[SEG_BUILTIN] = {
    [SEG_NULL]           = SEGMENT_INIT(0,0,0), /* NULL segment */
    [SEG_KERNEL_CODE]    = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_CODE_PL0), /* Kernel code segment */
    [SEG_KERNEL_DATA]    = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_DATA_PL0), /* Kernel data segment */
    [SEG_KERNEL_CODE_16] = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_CODE_PL0_16), /* 16-bit kernel code segment. */
    [SEG_KERNEL_DATA_16] = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_DATA_PL0_16), /* 16-bit kernel data segment. */
    [SEG_KERNELLDT]      = SEGMENT_INIT(0,0,SEG_LDT), /* Kernel LDT table. */
    [SEG_CPUTSS]         = {{{(u32)__kernel_seg_cputss_lo,(u32)__kernel_seg_cputss_hi}}}, /* CPU TSS */
    [SEG_CPUSELF]        = {{{(u32)__kernel_seg_cpuself_lo,(u32)__kernel_seg_cpuself_hi}}}, /* CPU-self */
#ifdef SEG_USER_CODE
    [SEG_USER_CODE]      = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_CODE_PL3), /* User code */
#endif
#ifdef SEG_USER_DATA
    [SEG_USER_DATA]      = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_DATA_PL3), /* User data */
#endif
};

DECL_END

#endif /* !GUARD_KERNEL_CORE_ARCH_GDT_C */
