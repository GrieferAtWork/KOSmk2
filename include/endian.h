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
#ifndef _ENDIAN_H
#define _ENDIAN_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/endian.h>

#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__ 1234
#endif
#ifndef __ORDER_BIG_ENDIAN__
#define __ORDER_BIG_ENDIAN__    4321
#endif
#ifndef __ORDER_PDP_ENDIAN__
#define __ORDER_PDP_ENDIAN__    3412
#endif

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define __BIG_ENDIAN    __ORDER_BIG_ENDIAN__
#define __PDP_ENDIAN    __ORDER_PDP_ENDIAN__
#endif /* !__LITTLE_ENDIAN */

#ifndef __FLOAT_WORD_ORDER
#define __FLOAT_WORD_ORDER __BYTE_ORDER
#endif

#ifdef __USE_MISC
#   define LITTLE_ENDIAN __LITTLE_ENDIAN
#   define BIG_ENDIAN    __BIG_ENDIAN
#   define PDP_ENDIAN    __PDP_ENDIAN
#ifndef BYTE_ORDER
#   define BYTE_ORDER    __BYTE_ORDER
#endif
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#   define __LONG_LONG_PAIR(hi,lo) lo,hi
#elif __BYTE_ORDER == __BIG_ENDIAN
#   define __LONG_LONG_PAIR(hi,lo) hi,lo
#endif

#if defined(__USE_MISC) && defined(__CC__)
#include <bits/byteswap.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#   define htobe16(x)  __bswap_16(x)
#   define htole16(x) (x)
#   define be16toh(x)  __bswap_16(x)
#   define le16toh(x) (x)
#   define htobe32(x)  __bswap_32(x)
#   define htole32(x) (x)
#   define be32toh(x)  __bswap_32(x)
#   define le32toh(x) (x)
#   define htobe64(x)  __bswap_64(x)
#   define htole64(x) (x)
#   define be64toh(x)  __bswap_64(x)
#   define le64toh(x) (x)
#elif __BYTE_ORDER == __BIG_ENDIAN
#   define htobe16(x) (x)
#   define htole16(x)  __bswap_16(x)
#   define be16toh(x) (x)
#   define le16toh(x)  __bswap_16(x)
#   define htobe32(x) (x)
#   define htole32(x)  __bswap_32(x)
#   define be32toh(x) (x)
#   define le32toh(x)  __bswap_32(x)
#   define htobe64(x) (x)
#   define htole64(x)  __bswap_64(x)
#   define be64toh(x) (x)
#   define le64toh(x)  __bswap_64(x)
#endif
#endif /* __USE_MISC && __CC__ */

#endif /* !_ENDIAN_H */
