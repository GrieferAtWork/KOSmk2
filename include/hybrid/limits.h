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
#ifndef GUARD_HYBRID_LIMITS_H
#define GUARD_HYBRID_LIMITS_H 1

#include <hybrid/host.h>

#if defined(__i386__) || defined(__x86_64__)
#   define __PAGESIZE        4096
#else
#   error FIXME
#endif
#if defined(__i386__) || defined(__x86_64__)
#   define __CACHELINE       64
#else
#   define __CACHELINE       64 /* Just guess... */
#endif

#ifdef GUARD_HYBRID_COMPILER_H
#ifndef PAGESIZE
#   define PAGESIZE          __PAGESIZE
#endif /* !PAGESIZE */
#   define CACHELINE         __CACHELINE
#   define CACHELINE_ALIGNED ATTR_ALIGNED(CACHELINE)
#endif

#endif /* !GUARD_HYBRID_LIMITS_H */
