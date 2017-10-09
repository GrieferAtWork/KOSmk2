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
#ifndef __GUARD_HYBRID_DEBUG_H
#define __GUARD_HYBRID_DEBUG_H 1

#include <__stdinc.h>

__SYSDECL_BEGIN

__LIBC __SSIZE_TYPE__ (__LIBCCALL debug_print)(char const *__restrict __data, __SIZE_TYPE__ __datalen, void *__ignored_closure);
__LIBC void (__ATTR_CDECL debug_printf)(char const *__restrict __format, ...);
__LIBC void (__LIBCCALL debug_vprintf)(char const *__restrict __format, __VA_LIST args);

__SYSDECL_END

#endif /* !__GUARD_HYBRID_DEBUG_H */
