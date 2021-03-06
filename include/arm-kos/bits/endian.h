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
#ifndef _ARM_KOS_BITS_ENDIAN_H
#define _ARM_KOS_BITS_ENDIAN_H 1
#define _BITS_ENDIAN_H 1

#include <__stdinc.h>

#ifndef __BYTE_ORDER
#ifdef __BYTE_ORDER__
#   define __BYTE_ORDER       __BYTE_ORDER__
#else
#   define __BYTE_ORDER       __LITTLE_ENDIAN
#endif
#endif /* !__BYTE_ORDER */

#endif /* !_ARM_KOS_BITS_ENDIAN_H */
