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
#ifndef _SYS_GENERIC_UN_H
#define _SYS_GENERIC_UN_H 1
#define _SYS_UN_H 1

#include <__stdinc.h>
#include <bits/sockaddr.h>

__SYSDECL_BEGIN

#ifdef __CC__
#ifndef __sockaddr_un_defined
#define __sockaddr_un_defined 1
struct sockaddr_un {
    __SOCKADDR_COMMON(sun_);
    char sun_path[108]; /*< Path name. */
};
#endif /* !__sockaddr_un_defined */

#ifdef __USE_MISC
#ifndef __std_strlen_defined
#define __std_strlen_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strlen)(char const *__s);
__NAMESPACE_STD_END
#endif /* !__std_strlen_defined */
#ifndef __strlen_defined
#define __strlen_defined 1
__NAMESPACE_STD_USING(strlen)
#endif /* !__strlen_defined */
#define SUN_LEN(ptr)   ((size_t)(((struct sockaddr_un *)0)->sun_path)+__NAMESPACE_STD_SYM strlen((ptr)->sun_path))
#endif /* __USE_MISC */
#endif /* __CC__ */

__SYSDECL_END

#endif /* !_SYS_GENERIC_UN_H */
