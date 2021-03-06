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
#ifndef __GUARD_HYBRID_MINMAX_H
#define __GUARD_HYBRID_MINMAX_H 1

#include <__stdinc.h>

__SYSDECL_BEGIN

#if defined(__DCC_VERSION__) || \
   (__has_builtin(__builtin_min) && __has_builtin(__builtin_max))
#   define MIN(a,b) __builtin_min(a,b)
#   define MAX(a,b) __builtin_max(a,b)
#elif defined(__COMPILER_HAVE_TYPEOF) && !defined(__NO_XBLOCK)
#   define MIN(a,b) __XBLOCK({ __typeof__(a) _a = (a),_b = (b); __XRETURN _a < _b ? _a : _b; })
#   define MAX(a,b) __XBLOCK({ __typeof__(a) _a = (a),_b = (b); __XRETURN _b < _a ? _a : _b; })
#else
#   define MIN(a,b) ((a) < (b) ? (a) : (b))
#   define MAX(a,b) ((b) < (a) ? (a) : (b))
#endif

__SYSDECL_END

#endif /* !__GUARD_HYBRID_MINMAX_H */
