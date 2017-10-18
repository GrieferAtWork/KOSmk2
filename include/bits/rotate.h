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
#ifndef _BITS_ROTATE_H
#define _BITS_ROTATE_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#define __constant_rol_8(x,shift)  (((x)>>(0x8-((shift)&0x07)))|((x)<<((shift)&0x07)))
#define __constant_ror_8(x,shift)  (((x)<<(0x8-((shift)&0x07)))|((x)>>((shift)&0x07)))
#define __constant_rol_16(x,shift) (((x)>>(0x10-((shift)&0x0f)))|((x)<<((shift)&0x0f)))
#define __constant_ror_16(x,shift) (((x)<<(0x10-((shift)&0x0f)))|((x)>>((shift)&0x0f)))
#define __constant_rol_32(x,shift) (((x)>>(0x20-((shift)&0x1f)))|((x)<<((shift)&0x1f)))
#define __constant_ror_32(x,shift) (((x)<<(0x20-((shift)&0x1f)))|((x)>>((shift)&0x1f)))
#define __constant_rol_64(x,shift) (((x)>>(0x40-((shift)&0x3f)))|((x)<<((shift)&0x3f)))
#define __constant_ror_64(x,shift) (((x)<<(0x40-((shift)&0x3f)))|((x)>>((shift)&0x3f)))

/* TODO: Use rol/ror instructions */
#define __rol_8(x,shift)  __constant_rol_8(x,shift)
#define __rol_16(x,shift) __constant_rol_16(x,shift)
#define __rol_32(x,shift) __constant_rol_32(x,shift)
#define __rol_64(x,shift) __constant_rol_64(x,shift)
#define __ror_8(x,shift)  __constant_ror_8(x,shift)
#define __ror_16(x,shift) __constant_ror_16(x,shift)
#define __ror_32(x,shift) __constant_ror_32(x,shift)
#define __ror_64(x,shift) __constant_ror_64(x,shift)

__SYSDECL_END

#endif /* _BITS_ROTATE_H */
