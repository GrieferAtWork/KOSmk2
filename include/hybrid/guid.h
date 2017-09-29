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
#ifndef __GUARD_HYBRID_GUID_H
#define __GUARD_HYBRID_GUID_H 1

#include "compiler.h"

#ifdef __CC__
DECL_BEGIN

typedef union PACKED guid_struct {
  u8   g_guid[16]; /*< GUID. */
struct PACKED {
  be32 g_a;
  be16 g_b;
  be16 g_c;
  be16 g_d;
  be32 g_e_1;
  be16 g_e_2;
}      g_data;
} guid_t;

#define GUID_A(x)        BSWAP_BE2H32((x).g_data.g_a)
#define GUID_B(x)        BSWAP_BE2H16((x).g_data.g_b)
#define GUID_C(x)        BSWAP_BE2H16((x).g_data.g_c)
#define GUID_D(x)        BSWAP_BE2H16((x).g_data.g_d)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define GUID_E(x) (((u64)BSWAP_BE2H32((x).g_data.g_e_1) << 16) | (u64)BSWAP_BE2H16((x).g_data.g_e_2))
#else
#define GUID_E(x)  ((u64)BSWAP_BE2H32((x).g_data.g_e_1) | ((u64)BSWAP_BE2H16((x).g_data.g_e_2) << 32))
#endif

/* Printf helpers for GUID printing. */
#define GUID_PRINTF_FMT    "%.8I32X-%.4I16X-%.4I16X-%.4I16X-%.12I64X"
#define GUID_PRINTF_ARG(x) GUID_A(*(x)),GUID_B(*(x)),GUID_C(*(x)),GUID_D(*(x)),GUID_E(*(x))

#define INITIALIZE_GUID(a,b,c,d,e) \
{{ \
   (u8)(((0x##a##u)&0xff000000) >> 24), \
   (u8)(((0x##a##u)&0x00ff0000) >> 16), \
   (u8)(((0x##a##u)&0x0000ff00) >> 8), \
   (u8)(((0x##a##u)&0x000000ff)), \
   (u8)(((0x##b##u)&0xff00) >> 8), \
   (u8)(((0x##b##u)&0x00ff)), \
   (u8)(((0x##c##u)&0xff00) >> 8), \
   (u8)(((0x##c##u)&0x00ff)), \
   (u8)(((0x##d##u)&0xff00) >> 8), \
   (u8)(((0x##d##u)&0x00ff)), \
   (u8)(((0x##e##ull)&0xff0000000000ull) >> 40), \
   (u8)(((0x##e##ull)&0x00ff00000000ull) >> 32), \
   (u8)(((0x##e##ull)&0x0000ff000000ull) >> 24), \
   (u8)(((0x##e##ull)&0x000000ff0000ull) >> 16), \
   (u8)(((0x##e##ull)&0x00000000ff00ull) >> 8), \
   (u8)(((0x##e##ull)&0x0000000000ffull)), \
}}

DECL_END
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_GUID_H */
