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
#ifndef __GUARD_HYBRID_SWAP_H
#define __GUARD_HYBRID_SWAP_H 1

#include "compiler.h"

#if defined(__COMPILER_HAVE_AUTOTYPE)
#   define SWAP(a,b) XBLOCK({ __auto_type   _temp = (a); (a) = (b); (b) = _temp; (void)0; })
#elif defined(__COMPILER_HAVE_TYPEOF)
#   define SWAP(a,b) XBLOCK({ __typeof__(a) _temp = (a); (a) = (b); (b) = _temp; (void)0; })
#else
#   define SWAP(a,b) XBLOCK({ (a) ^= (b); (b) ^= (a); (a) ^= (b); (void)0; })
#endif
#define SORT(a,b) ((a) > (b) ? SWAP(a,b) : (void)0)

#endif /* !__GUARD_HYBRID_SWAP_H */
