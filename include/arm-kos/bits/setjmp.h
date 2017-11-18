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
#ifndef _ARM_KOS_BITS_SETJMP_H
#define _ARM_KOS_BITS_SETJMP_H 1
#define _BITS_SETJMP_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>


#define __JMP_BUF_ALIGN 8
#define __JMP_BUF_SIZE  256

#ifdef __CC__
__SYSDECL_BEGIN
__ATTR_ALIGNED(8) struct __jmp_buf { __INT32_TYPE__ __data[64]; };

__SYSDECL_END
#endif

#endif /* !_ARM_KOS_BITS_SETJMP_H */
