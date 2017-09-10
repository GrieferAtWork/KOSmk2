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

__DECL_BEGIN

#ifndef __KERNEL__
__LIBC void (__LIBCCALL warn)(const char *__format, ...);
__LIBC void (__LIBCCALL vwarn)(const char *__format, __VA_LIST __args);
__LIBC void (__LIBCCALL warnx)(const char *__format, ...);
__LIBC void (__LIBCCALL vwarnx)(const char *__format, __VA_LIST __args);
__LIBC __ATTR_NORETURN void (__LIBCCALL err)(int __status, const char *__format, ...);
__LIBC __ATTR_NORETURN void (__LIBCCALL verr)(int __status, const char *__format, __VA_LIST __args);
__LIBC __ATTR_NORETURN void (__LIBCCALL errx)(int __status, const char *__format, ...);
__LIBC __ATTR_NORETURN void (__LIBCCALL verrx)(int __status, const char *__format, __VA_LIST __args);
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_ERR_H */
