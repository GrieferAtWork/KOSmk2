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
#ifndef _WCTYPE_H
#define _WCTYPE_H 1

#include <features.h>
#include <bits/types.h>
#include <hybrid/typecore.h>
#include <hybrid/byteorder.h>
#ifdef __USE_XOPEN2K8
#include <xlocale.h>
#endif

/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

__DECL_BEGIN

#ifdef __NAMESPACE_STD_EXISTS
__NAMESPACE_STD_BEGIN
#ifndef __std_wint_t_defined
#define __std_wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__std_wint_t_defined */
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
#ifndef __wint_t_defined
#define __wint_t_defined 1
__NAMESPACE_STD_USING(wint_t)
#endif /* !__wint_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#else /* STD-namespace */
#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */
#endif /* !STD-namespace */

#ifndef WEOF
#define WEOF (0xffffffffu)
#endif

#ifndef __wisxxx_defined
#define __wisxxx_defined 1

#ifndef __wctype_t_defined
#define __wctype_t_defined 1
__NAMESPACE_STD_BEGIN
typedef unsigned long int wctype_t;
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wctype_t)
#endif /* !__wctype_t_defined */

#if __BYTE_ORDER == __BIG_ENDIAN
#   define _ISwbit(bit) (1 << (bit))
#else /* __BYTE_ORDER == __BIG_ENDIAN */
#   define _ISwbit(bit) \
    ((bit) < 8  ? (int)((1UL << (bit)) << 24) : \
    ((bit) < 16 ? (int)((1UL << (bit)) << 8) : \
    ((bit) < 24 ? (int)((1UL << (bit)) >> 8) : \
                  (int)((1UL << (bit)) >> 24))))
#endif /* __BYTE_ORDER != __BIG_ENDIAN */

#define __ISwupper  0  /*< UPPERCASE. */
#define __ISwlower  1  /*< lowercase. */
#define __ISwalpha  2  /*< Alphabetic. */
#define __ISwdigit  3  /*< Numeric. */
#define __ISwxdigit 4  /*< Hexadecimal numeric. */
#define __ISwspace  5  /*< Whitespace. */
#define __ISwprint  6  /*< Printing. */
#define __ISwgraph  7  /*< Graphical. */
#define __ISwblank  8  /*< Blank (usually SPC and TAB). */
#define __ISwcntrl  9  /*< Control character. */
#define __ISwpunct  10 /*< Punctuation. */
#define __ISwalnum  11 /*< Alphanumeric. */
#define _ISwupper   _ISwbit(__ISwupper)  /*< UPPERCASE. */
#define _ISwlower   _ISwbit(__ISwlower)  /*< lowercase. */
#define _ISwalpha   _ISwbit(__ISwalpha)  /*< Alphabetic. */
#define _ISwdigit   _ISwbit(__ISwdigit)  /*< Numeric. */
#define _ISwxdigit  _ISwbit(__ISwxdigit) /*< Hexadecimal numeric. */
#define _ISwspace   _ISwbit(__ISwspace)  /*< Whitespace. */
#define _ISwprint   _ISwbit(__ISwprint)  /*< Printing. */
#define _ISwgraph   _ISwbit(__ISwgraph)  /*< Graphical. */
#define _ISwblank   _ISwbit(__ISwblank)  /*< Blank (usually SPC and TAB). */
#define _ISwcntrl   _ISwbit(__ISwcntrl)  /*< Control character. */
#define _ISwpunct   _ISwbit(__ISwpunct)  /*< Punctuation. */
#define _ISwalnum   _ISwbit(__ISwalnum)  /*< Alphanumeric. */


#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC int __NOTHROW((__LIBCCALL iswalnum)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswalpha)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswcntrl)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswdigit)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswgraph)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswlower)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswprint)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswpunct)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswspace)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswupper)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswxdigit)(wint_t __wc));
__LIBC wctype_t __NOTHROW((__LIBCCALL wctype)(char const *__property));
__LIBC int __NOTHROW((__LIBCCALL iswctype)(wint_t __wc, wctype_t __desc));
#ifdef __USE_ISOC99
__LIBC int __NOTHROW((__LIBCCALL iswblank)(wint_t __wc));
#endif /* __USE_ISOC99 */
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(iswalnum)
__NAMESPACE_STD_USING(iswalpha)
__NAMESPACE_STD_USING(iswcntrl)
__NAMESPACE_STD_USING(iswdigit)
__NAMESPACE_STD_USING(iswgraph)
__NAMESPACE_STD_USING(iswlower)
__NAMESPACE_STD_USING(iswprint)
__NAMESPACE_STD_USING(iswpunct)
__NAMESPACE_STD_USING(iswspace)
__NAMESPACE_STD_USING(iswupper)
__NAMESPACE_STD_USING(iswxdigit)
__NAMESPACE_STD_USING(wctype)
__NAMESPACE_STD_USING(iswctype)
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(iswblank)
#endif /* __USE_ISOC99 */
#endif /* !__KERNEL__ */

__NAMESPACE_STD_BEGIN
#if defined(__USE_ISOC99) || defined(__USE_GNU)
typedef __int32_t const *wctrans_t;
#endif /* __USE_ISOC99 || __USE_GNU */
#ifndef __KERNEL__
__LIBC wint_t __NOTHROW((__LIBCCALL towlower)(wint_t __wc));
__LIBC wint_t __NOTHROW((__LIBCCALL towupper)(wint_t __wc));
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END
#if defined(__USE_ISOC99) || defined(__USE_GNU)
#ifndef __wctrans_t_defined
#define __wctrans_t_defined 1
__NAMESPACE_STD_USING(wctrans_t)
#endif /* !__wctrans_t_defined */
#endif /* __USE_ISOC99 || __USE_GNU */
#ifndef __KERNEL__
__NAMESPACE_STD_USING(towlower)
__NAMESPACE_STD_USING(towupper)
#endif /* !__KERNEL__ */
#endif /* !__wisxxx_defined */


#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC wctrans_t __NOTHROW((__LIBCCALL wctrans)(char const *__property));
__LIBC wint_t __NOTHROW((__LIBCCALL towctrans)(wint_t __wc, wctrans_t __desc));
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wctrans)
__NAMESPACE_STD_USING(towctrans)

#ifdef __USE_XOPEN2K8
__LIBC int __NOTHROW((__LIBCCALL iswalnum_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswalpha_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswcntrl_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswdigit_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswgraph_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswlower_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswprint_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswpunct_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswspace_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswupper_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswxdigit_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswblank_l)(wint_t __wc, __locale_t __locale));
__LIBC wctype_t __NOTHROW((__LIBCCALL wctype_l)(char const *__property, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL iswctype_l)(wint_t __wc, wctype_t __desc, __locale_t __locale));
__LIBC wint_t __NOTHROW((__LIBCCALL towlower_l)(wint_t __wc, __locale_t __locale));
__LIBC wint_t __NOTHROW((__LIBCCALL towupper_l)(wint_t __wc, __locale_t __locale));
__LIBC wctrans_t __NOTHROW((__LIBCCALL wctrans_l)(char const *__property, __locale_t __locale));
__LIBC wint_t __NOTHROW((__LIBCCALL towctrans_l)(wint_t __wc, wctrans_t __desc, __locale_t __locale));
#endif /* __USE_XOPEN2K8 */
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_WCTYPE_H */