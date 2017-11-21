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
#ifndef GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_MEMORY_H
#define GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_MEMORY_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/memory.h>
#include <kernel/export.h>
#include <arch/paging.h>

DECL_BEGIN

/* Predefined memory regions. */
#define MEMORY_PREDEF_COUNT  4
#define MEMORY_PREDEF_LIST \
  { .mi_type = MEMTYPE_NDEF,   .mi_addr = (void *)0, }, \
  { .mi_type = MEMTYPE_KERNEL, .mi_addr = (void *)KERNEL_START, }, \
  { .mi_type = MEMTYPE_KFREE,  .mi_addr = (void *)KERNEL_FREE_START, }, \
  { .mi_type = MEMTYPE_NDEF,   .mi_addr = (void *)KERNEL_FREE_END, },


DECL_END

#endif /* !GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_MEMORY_H */
