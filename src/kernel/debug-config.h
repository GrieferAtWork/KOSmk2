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
#ifndef GUARD_KERNEL_DEBUG_CONFIG_H
#define GUARD_KERNEL_DEBUG_CONFIG_H 1

#include <hybrid/compiler.h>
#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>

DECL_BEGIN

/* Debug memory patterns indicating uninitialized data. */
#if __SIZEOF_POINTER__ == 8
#   define KERNEL_DEBUG_MEMPAT_PAGE_MALLOC  __UINTPTR_C(0xABAD9A6EABAD9A6E) /*< A BAD PAGE. */
#   define KERNEL_DEBUG_MEMPAT_KMALLOC      __UINTPTR_C(0xBAADF00DBAADF00D) /*< BAAD FOOD. */
#   define KERNEL_DEBUG_MEMPAT_HOSTSTACK    __UINTPTR_C(0xCCCCCCCCCCCCCCCC)
#   define KERNEL_DEBUG_MEMPAT_USERSTACK    __UINTPTR_C(0xCCCCCCCCCCCCCCCC)
#elif __SIZEOF_POINTER__ == 4
#   define KERNEL_DEBUG_MEMPAT_PAGE_MALLOC  __UINTPTR_C(0xABAD9A6E) /*< A BAD PAGE. */
#   define KERNEL_DEBUG_MEMPAT_KMALLOC      __UINTPTR_C(0xBAADF00D) /*< BAAD FOOD. */
#   define KERNEL_DEBUG_MEMPAT_HOSTSTACK    __UINTPTR_C(0xCCCCCCCC)
#   define KERNEL_DEBUG_MEMPAT_USERSTACK    __UINTPTR_C(0xCCCCCCCC)
#else
#error "Unsupported sizeof(void *)"
#endif

DECL_END

#endif /* !GUARD_KERNEL_DEBUG_CONFIG_H */
