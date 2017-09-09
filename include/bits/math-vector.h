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
#ifndef _BITS_MATH_VECTOR_H
#define _BITS_MATH_VECTOR_H 1

/* Platform-specific SIMD declarations of math functions.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

#include <bits/libm-simd-decl-stubs.h>

#if defined(__x86_64__) && defined(__FAST_MATH__)
#if defined(_OPENMP) && _OPENMP >= 201307
#   define __DECL_SIMD_x86_64 _Pragma ("omp declare simd notinbranch")
#elif __GNUC_PREREQ (6,0)
#   define __DECL_SIMD_x86_64 __attribute__ ((__simd__ ("notinbranch")))
#endif
#ifdef __DECL_SIMD_x86_64
#   undef __DECL_SIMD_cos
#   undef __DECL_SIMD_cosf
#   undef __DECL_SIMD_sin
#   undef __DECL_SIMD_sinf
#   undef __DECL_SIMD_sincos
#   undef __DECL_SIMD_sincosf
#   undef __DECL_SIMD_log
#   undef __DECL_SIMD_logf
#   undef __DECL_SIMD_exp
#   undef __DECL_SIMD_expf
#   undef __DECL_SIMD_pow
#   undef __DECL_SIMD_powf
#   define __DECL_SIMD_cos     __DECL_SIMD_x86_64
#   define __DECL_SIMD_cosf    __DECL_SIMD_x86_64
#   define __DECL_SIMD_sin     __DECL_SIMD_x86_64
#   define __DECL_SIMD_sinf    __DECL_SIMD_x86_64
#   define __DECL_SIMD_sincos  __DECL_SIMD_x86_64
#   define __DECL_SIMD_sincosf __DECL_SIMD_x86_64
#   define __DECL_SIMD_log     __DECL_SIMD_x86_64
#   define __DECL_SIMD_logf    __DECL_SIMD_x86_64
#   define __DECL_SIMD_exp     __DECL_SIMD_x86_64
#   define __DECL_SIMD_expf    __DECL_SIMD_x86_64
#   define __DECL_SIMD_pow     __DECL_SIMD_x86_64
#   define __DECL_SIMD_powf    __DECL_SIMD_x86_64
#endif
#endif

#endif /* !_BITS_MATH_VECTOR_H */
