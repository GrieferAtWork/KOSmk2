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
#ifndef _ERR_H
#define _ERR_H 1

#include <__stdinc.h>
#include <features.h>

#if !defined(__CRT_GLC) && !defined(__CRT_CYG)
#error "<err.h> is not supported by the linked libc"
#endif /* !__CRT_GLC && !__CRT_CYG */

__SYSDECL_BEGIN

#ifndef __KERNEL__
__LIBC __ATTR_COLD void (__LIBCCALL warn)(char const *__format, ...);
__LIBC __ATTR_COLD void (__LIBCCALL vwarn)(char const *__format, __builtin_va_list __args);
__LIBC __ATTR_COLD void (__LIBCCALL warnx)(char const *__format, ...);
__LIBC __ATTR_COLD void (__LIBCCALL vwarnx)(char const *__format, __builtin_va_list __args);
__LIBC __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL err)(int __status, char const *__format, ...);
__LIBC __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL verr)(int __status, char const *__format, __builtin_va_list __args);
__LIBC __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL errx)(int __status, char const *__format, ...);
__LIBC __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL verrx)(int __status, char const *__format, __builtin_va_list __args);
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_ERR_H */
