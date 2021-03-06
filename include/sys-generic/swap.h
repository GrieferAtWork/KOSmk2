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
#ifndef _SYS_GENERIC_SWAP_H
#define _SYS_GENERIC_SWAP_H 1
#define _SYS_SWAP_H 1

#include <__stdinc.h>
#include <features.h>

/* Calls to enable and disable swapping on specified locations.  Linux version.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef __CRT_GLC
#error "<sys/swap.h> is not supported by the linked libc"
#endif /* !__CRT_GLC */

__SYSDECL_BEGIN

/* The swap priority is encoded as:
 * >> (prio << SWAP_FLAG_PRIO_SHIFT) & SWAP_FLAG_PRIO_MASK */
#ifndef SWAP_FLAG_PREFER
#define SWAP_FLAG_PREFER     0x08000 /*< Set if swap priority is specified. */
#endif /* !SWAP_FLAG_PREFER */
#ifndef SWAP_FLAG_PRIO_MASK
#define SWAP_FLAG_PRIO_MASK  0x07fff
#endif /* !SWAP_FLAG_PRIO_MASK */
#ifndef SWAP_FLAG_PRIO_SHIFT
#define SWAP_FLAG_PRIO_SHIFT       0
#endif /* !SWAP_FLAG_PRIO_SHIFT */
#ifndef SWAP_FLAG_DISCARD
#define SWAP_FLAG_DISCARD    0x10000 /*< Discard swap cluster after use. */
#endif /* !SWAP_FLAG_DISCARD */

#ifndef __KERNEL__
__LIBC int (__LIBCCALL swapon)(char const *__path, int __flags);
__LIBC int (__LIBCCALL swapoff)(char const *__path);
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* _SYS_GENERIC_SWAP_H */
