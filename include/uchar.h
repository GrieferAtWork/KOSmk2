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
#ifndef _UCHAR_H
#define _UCHAR_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>

__DECL_BEGIN

/* Define 'size_t' */
#ifdef __NAMESPACE_STD_EXISTS
#ifndef __std_size_t_defined
#define __std_size_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __SIZE_TYPE__ size_t;
__NAMESPACE_STD_END
#endif /* !__std_size_t_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __size_t_defined
#define __size_t_defined 1
__NAMESPACE_STD_USING(size_t)
#endif /* !__size_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#else /* STD-namespace */
#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */
#endif /* !STD-namespace */

#ifndef ____mbstate_t_defined
#define ____mbstate_t_defined 1
typedef struct __mbstate {
 int                   __count;
 union { __WINT_TYPE__ __wch; char __wchb[4]; } __value;
} __mbstate_t;
#endif /* !____mbstate_t_defined */

#ifndef __std_mbstate_t_defined
#define __std_mbstate_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __mbstate_t mbstate_t;
__NAMESPACE_STD_END
#endif /* !__std_mbstate_t_defined */

#ifndef __mbstate_t_defined
#define __mbstate_t_defined 1
__NAMESPACE_STD_USING(mbstate_t)
#endif /* !__mbstate_t_defined */

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifndef __MBSTATE_INIT
#define __MBSTATE_INIT     {0,{0}}
#endif /* !__MBSTATE_INIT */

#ifdef __USE_KOS
#ifndef MBSTATE_INIT
#define MBSTATE_INIT     __MBSTATE_INIT
#endif /* !MBSTATE_INIT */
#endif /* __USE_KOS */

/* Libc uses utf16/utf32 to encode/decode char16_t and char32_t */
#define __STD_UTF_16__ 1
#define __STD_UTF_32__ 1

#ifndef __KERNEL__
__LIBC size_t __NOTHROW((__LIBCCALL mbrtoc16)(char16_t *__restrict __pc16, char const *__restrict __s, size_t __n, mbstate_t *__restrict __p));
__LIBC size_t __NOTHROW((__LIBCCALL mbrtoc32)(char32_t *__restrict __pc32, char const *__restrict __s, size_t __n, mbstate_t *__restrict __p));
__LIBC size_t __NOTHROW((__LIBCCALL c16rtomb)(char *__restrict __s, char16_t __c16, mbstate_t *__restrict __ps));
__LIBC size_t __NOTHROW((__LIBCCALL c32rtomb)(char *__restrict __s, char32_t __c32, mbstate_t *__restrict __ps));
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_UCHAR_H */
