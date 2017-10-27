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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_APIC_H
#define GUARD_INCLUDE_KERNEL_ARCH_APIC_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/arch/apicdef.h>
#include <sync/rwlock.h>
#include <sys/io.h>

DECL_BEGIN

/* From linux: "/arch/x86/include/asm/apic.h" */
#define TRAMPOLINE_PHYS_LOW  0x467
#define TRAMPOLINE_PHYS_HIGH 0x469
/* end... */

DATDEF VIRT uintptr_t const apic_base;   /*< Memory-mapped address of local APICs. */
DATDEF PHYS uintptr_t const apic_base_p; /*< Physical counterpart to `apic_base'. */
DATDEF rwlock_t             apic_lock;   /*< Lock held while reading from/writing to the LAPIC. */

/* TODO: I/O APIC mappings. */

#define APIC_SUPPORTED()   (__bootcpu.c_arch.ac_flags&CPUFLAG_LAPIC)

#define apic_read(reg)     readl(apic_base+(reg))
#define apic_write(reg,v) writel(apic_base+(reg),v)

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_APIC_H */
