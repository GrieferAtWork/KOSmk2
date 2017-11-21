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
#ifndef GUARD_KERNEL_ARCH_BOOT_C
#define GUARD_KERNEL_ARCH_BOOT_C 1
#define _KOS_SOURCE 1

#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <arch/hints.h>
#include <sched/cpu.h>
#include <kernel/boot.h>

DECL_BEGIN

/* Statically allocate the initial boot stack. */
INTERN ATTR_FREEBSS ATTR_ALIGNED(HOST_STCK_ALIGN)
byte_t __bootstack[HOST_BOOT_STCKSIZE];

/* Hosting emulation information. */
PUBLIC u8  boot_emulation = BOOT_EMULATION_DEFAULT;
PUBLIC u16 boot_emulation_logport = (u16)0x80;

GLOBAL_ASM(
L(.syntax unified                                                             )
L(.section .text.entry                                                        )
L(INTERN_ENTRY(_start)                                                        )
L(    ldr   sp, 1f                                                            )
L(    bl    kernel_boot                                                       )
L(1:  .long __bootstack+HOST_BOOT_STCKSIZE                                    )
L(SYM_END(_start)                                                             )
L(.previous                                                                   )
);

PUBLIC void foo(void) {
 register byte_t *x asm("sp");
 x = __bootstack;
 *x = 42;
}

DECL_END

#endif /* !GUARD_KERNEL_ARCH_BOOT_C */
