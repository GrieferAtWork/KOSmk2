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
#ifndef GUARD_HYBRID_ALLOCA_H
#define GUARD_HYBRID_ALLOCA_H 1

#include <__stdinc.h>
#include <features.h>

#if defined(__USE_DEBUG_HOOK) && !defined(__GNUC__)
#include "typecore.h"
__DECL_BEGIN
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_SIZE((1))
void *(__LIBCCALL alloca)(__SIZE_TYPE__ __n_bytes);
__DECL_END
#   define __ALLOCA(s)  alloca((s))
#else /* __USE_DEBUG_HOOK */
#   define __ALLOCA(s)  __builtin_alloca((s))
#endif /* !__USE_DEBUG_HOOK */

#endif /* !GUARD_HYBRID_ALLOCA_H */
