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
#ifndef _ASM_GENERIC_TYPES_H
#define _ASM_GENERIC_TYPES_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>

#ifndef __ASSEMBLY__

#ifndef ____suX_defined
#define ____suX_defined 1
__SYSDECL_BEGIN
typedef __INT8_TYPE__   __s8;
typedef __INT16_TYPE__  __s16;
typedef __INT32_TYPE__  __s32;
typedef __INT64_TYPE__  __s64;
typedef __UINT8_TYPE__  __u8;
typedef __UINT16_TYPE__ __u16;
typedef __UINT32_TYPE__ __u32;
typedef __UINT64_TYPE__ __u64;
__SYSDECL_END
#endif /* !____suX_defined */

#endif /* __ASSEMBLY__ */

#endif /* !_ASM_GENERIC_TYPES_H */
