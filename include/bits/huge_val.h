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
#ifndef _BITS_HUGE_VAL_H
#define _BITS_HUGE_VAL_H 1

#include <__stdinc.h>

/* `HUGE_VAL' constant for IEEE 754 machines (where it is infinity).
   Used by <stdlib.h> and <math.h> functions for overflow.
   Copyright (C) 1992-2016 Free Software Foundation, Inc.
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

__DECL_BEGIN

#if __GCC_VERSION(3,3,0)
#   define HUGE_VAL    (__builtin_huge_val())
#elif __GCC_VERSION(2,96,0)
#   define HUGE_VAL    (__extension__ 0x1.0p2047)
#elif defined(__GNUC__)
#   define HUGE_VAL \
  (__extension__((union { unsigned __l __attribute__((__mode__(__DI__))); \
                          double __d; }){ __l: 0x7ff0000000000000ULL }).__d)
#else /* ... */
#include <endian.h>
typedef union { unsigned char __c[8]; double __d; } __huge_val_t;
#if __BYTE_ORDER == __BIG_ENDIAN
#   define __HUGE_VAL_bytes    {0x7f,0xf0,0,0,0,0,0,0}
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#   define __HUGE_VAL_bytes    {0,0,0,0,0,0,0xf0,0x7f}
#endif
__PRIVATE __huge_val_t const __huge_val = {__HUGE_VAL_bytes};
#define HUGE_VAL    (__huge_val.__d)
#endif /* !... */

__DECL_END

#endif /* !_BITS_HUGE_VAL_H */
