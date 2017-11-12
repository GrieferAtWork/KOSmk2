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
#ifndef GUARD_KERNEL_CORE_ARCH_INTERRUPT_INTERN_H
#define GUARD_KERNEL_CORE_ARCH_INTERRUPT_INTERN_H 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <kernel/arch/interrupt.h>

DECL_BEGIN

struct interrupt;
struct entry {
 LIST_HEAD(struct interrupt) e_head; /*< [0..1] Chain of interrupt handlers in this entry. */
};
struct PACKED interrupt_table {
 struct PACKED idtentry it_idt[256]; /* Internal CPU Interrupt Descriptor Table. */
 struct entry           it_tab[256]; /* Per-cpu + per-vector descriptors. */
};


INTDEF PERCPU struct PACKED interrupt_table inttab;
INTDEF INITCALL void KCALL cpu_interrupt_initialize(struct cpu *__restrict c);

DECL_END

#endif /* !GUARD_KERNEL_CORE_ARCH_INTERRUPT_INTERN_H */
