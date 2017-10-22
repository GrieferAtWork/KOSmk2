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
#ifndef __GUARD_HYBRID_ARCH_CPU_H
#define __GUARD_HYBRID_ARCH_CPU_H 1

#include <hybrid/compiler.h>
#include <hybrid/arch/eflags.h>
#include <stdbool.h>

DECL_BEGIN

#ifdef __i386__
LOCAL bool KCALL cpu_is_486(void) {
 /* NOTE: This variable _must_ be aligned, because the AC flag's
  *       intended purpose (other than checking for 486), is to
  *       trigger faults when working with unaligned pointers. */
 register ATTR_ALIGNED(4) __UINT32_TYPE__ f1,f2;
 __asm__ __volatile__("pushf\n" /* Store original eflags. */
                      "pushl (%%esp)\n" /* Create working copy of eflags. */
                      "movl (%%esp), %0\n"
                      "xorl $0x00040000, (%%esp)\n" /* Flip EFLAGS_AC */
                      "popf\n" /* Try to enable the AC-bit. */
                      "pushf\n" /* See if it was enabled. */
                      "popl %1\n" /* Store flags with AC potentially enabled. */
                      "popf\n" /* Restore original flags. */
                      : "=r" (f1), "=r" (f2));
 /* If we've managed to activate AC-mode, we're running on a 486, or above. */
 return (f1&EFLAGS_AC) != (f2&EFLAGS_AC);
}

#endif


DECL_END

#endif /* !__GUARD_HYBRID_ARCH_CPU_H */
