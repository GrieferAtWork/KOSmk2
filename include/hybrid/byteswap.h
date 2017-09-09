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
#ifndef GUARD_HYBRID_BYTESWAP_H
#define GUARD_HYBRID_BYTESWAP_H 1

#include "compiler.h"
#include "byteorder.h"
#include <bits/byteswap.h>

DECL_BEGIN

#define LITTLE_ENDIAN_ORDER __LITTLE_ENDIAN
#define BIG_ENDIAN_ORDER    __BIG_ENDIAN
#define PDP_ENDIAN_ORDER    __PDP_ENDIAN
#define BYTEORDER           __BYTE_ORDER

#define BSWAP16(x)    __bswap_16(x)
#define BSWAP32(x)    __bswap_32(x)
#define BSWAP64(x)    __bswap_64(x)
#define BSWAP16_C(x)  __bswap_constant_16(x)
#define BSWAP32_C(x)  __bswap_constant_32(x)
#define BSWAP64_C(x)  __bswap_constant_64(x)

#ifdef __KERNEL__
STATIC_ASSERT(BSWAP16_C(0xaabbu) == 0xbbaau);
STATIC_ASSERT(BSWAP32_C(0xaabbccddu) == 0xddccbbaau);
STATIC_ASSERT(BSWAP64_C(0x66778899aabbccddull) == 0xddccbbaa99887766ull);
#endif /* __KERNEL__ */

#if BYTEORDER == LITTLE_ENDIAN_ORDER
#   define BSWAP_H2LE16(x)   (x)
#   define BSWAP_H2LE32(x)   (x)
#   define BSWAP_H2LE64(x)   (x)
#   define BSWAP_H2BE16(x)    BSWAP16(x)
#   define BSWAP_H2BE32(x)    BSWAP32(x)
#   define BSWAP_H2BE64(x)    BSWAP64(x)
#   define BSWAP_LE2H16(x)   (x)
#   define BSWAP_LE2H32(x)   (x)
#   define BSWAP_LE2H64(x)   (x)
#   define BSWAP_BE2H16(x)    BSWAP16(x)
#   define BSWAP_BE2H32(x)    BSWAP32(x)
#   define BSWAP_BE2H64(x)    BSWAP64(x)
#   define BSWAP_H2LE16_C(x) (x)
#   define BSWAP_H2LE32_C(x) (x)
#   define BSWAP_H2LE64_C(x) (x)
#   define BSWAP_H2BE16_C(x)  BSWAP16_C(x)
#   define BSWAP_H2BE32_C(x)  BSWAP32_C(x)
#   define BSWAP_H2BE64_C(x)  BSWAP64_C(x)
#   define BSWAP_LE2H16_C(x) (x)
#   define BSWAP_LE2H32_C(x) (x)
#   define BSWAP_LE2H64_C(x) (x)
#   define BSWAP_BE2H16_C(x)  BSWAP16_C(x)
#   define BSWAP_BE2H32_C(x)  BSWAP32_C(x)
#   define BSWAP_BE2H64_C(x)  BSWAP64_C(x)
#elif BYTEORDER == BIG_ENDIAN_ORDER
#   define BSWAP_H2LE16(x)    BSWAP16(x)
#   define BSWAP_H2LE32(x)    BSWAP32(x)
#   define BSWAP_H2LE64(x)    BSWAP64(x)
#   define BSWAP_H2BE16(x)   (x)
#   define BSWAP_H2BE32(x)   (x)
#   define BSWAP_H2BE64(x)   (x)
#   define BSWAP_LE2H16(x)    BSWAP16(x)
#   define BSWAP_LE2H32(x)    BSWAP32(x)
#   define BSWAP_LE2H64(x)    BSWAP64(x)
#   define BSWAP_BE2H16(x)   (x)
#   define BSWAP_BE2H32(x)   (x)
#   define BSWAP_BE2H64(x)   (x)
#   define BSWAP_H2LE16_C(x)  BSWAP16_C(x)
#   define BSWAP_H2LE32_C(x)  BSWAP32_C(x)
#   define BSWAP_H2LE64_C(x)  BSWAP64_C(x)
#   define BSWAP_H2BE16_C(x) (x)
#   define BSWAP_H2BE32_C(x) (x)
#   define BSWAP_H2BE64_C(x) (x)
#   define BSWAP_LE2H16_C(x)  BSWAP16_C(x)
#   define BSWAP_LE2H32_C(x)  BSWAP32_C(x)
#   define BSWAP_LE2H64_C(x)  BSWAP64_C(x)
#   define BSWAP_BE2H16_C(x) (x)
#   define BSWAP_BE2H32_C(x) (x)
#   define BSWAP_BE2H64_C(x) (x)
#else
#   error FIXME
#endif

DECL_END

#endif /* !GUARD_HYBRID_BYTESWAP_H */
