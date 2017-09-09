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
#ifndef ___MALLDEFS_H
#define ___MALLDEFS_H 1

#include "__stdinc.h"
#include <hybrid/limits.h>
#ifndef __SIZEOF_POINTER__
#include <hybrid/typecore.h>
#endif

#ifndef __MALL_MIN_ALIGNMENT
#if __SIZEOF_POINTER__ == 4
#   define __MALL_MIN_ALIGNMENT  8
#elif __SIZEOF_POINTER__ == 8
#   define __MALL_MIN_ALIGNMENT  16
#else
#   define __MALL_MIN_ALIGNMENT (2*__SIZEOF_POINTER__)
#endif
#endif


#   define __MALL_DEFAULT_ALIGNED  __ATTR_ASSUME_ALIGNED(__MALL_MIN_ALIGNMENT)
#ifdef __PAGESIZE
#   define __MALL_ATTR_PAGEALIGNED __ATTR_ASSUME_ALIGNED(__PAGESIZE)
#else
#   define __MALL_ATTR_PAGEALIGNED /* Nothing */
#endif

#endif /* !___MALLDEFS_H */
