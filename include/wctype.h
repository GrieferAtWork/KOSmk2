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

/* Dos character type bits. */
#ifndef __DOS_UPPER
#define __DOS_UPPER    0x0001
#define __DOS_LOWER    0x0002
#define __DOS_DIGIT    0x0004
#define __DOS_SPACE    0x0008
#define __DOS_PUNCT    0x0010
#define __DOS_CONTROL  0x0020
#define __DOS_BLANK    0x0040
#define __DOS_HEX      0x0080
#define __DOS_LEADBYTE 0x8000
#define __DOS_ALPHA    0x0103

#ifdef __USE_DOS
#   define _UPPER    __DOS_UPPER
#   define _LOWER    __DOS_LOWER
#   define _DIGIT    __DOS_DIGIT
#   define _SPACE    __DOS_SPACE
#   define _PUNCT    __DOS_PUNCT
#   define _CONTROL  __DOS_CONTROL
#   define _BLANK    __DOS_BLANK
#   define _HEX      __DOS_HEX
#   define _LEADBYTE __DOS_LEADBYTE
#   define _ALPHA    __DOS_ALPHA
#endif /* __USE_DOS */
#endif /* !__DOS_UPPER */

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

#ifndef _WCTYPE_DEFINED
#define _WCTYPE_DEFINED 1
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
__LIBC wctype_t __NOTHROW((__LIBCCALL wctype)(char const *__prop));
#ifndef ____kos_iswctype_defined
__LIBC int __NOTHROW((__LIBCCALL __kos_iswctype)(wint_t __wc, wctype_t __desc)) __ASMNAME("iswctype");
#endif /* !____kos_iswctype_defined */
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
#ifndef ____kos_iswctype_defined
#define ____kos_iswctype_defined 1
__NAMESPACE_STD_USING(__kos_iswctype)
#endif /* !____kos_iswctype_defined */
__NAMESPACE_STD_USING(iswctype)
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(iswblank)
#endif /* __USE_ISOC99 */
#if defined(__USE_KOS) || defined(__USE_DOS)
__LIBC int __NOTHROW((__LIBCCALL iswascii)(wint_t __wc));
#endif /* __USE_KOS || __USE_DOS */
#ifdef __USE_DOS
__LIBC int __NOTHROW((__LIBCCALL _iswalpha_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswalpha_l");
__LIBC int __NOTHROW((__LIBCCALL _iswupper_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswupper_l");
__LIBC int __NOTHROW((__LIBCCALL _iswlower_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswlower_l");
__LIBC int __NOTHROW((__LIBCCALL _iswdigit_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswdigit_l");
__LIBC int __NOTHROW((__LIBCCALL _iswxdigit_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswxdigit_l");
__LIBC int __NOTHROW((__LIBCCALL _iswspace_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswspace_l");
__LIBC int __NOTHROW((__LIBCCALL _iswpunct_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswpunct_l");
__LIBC int __NOTHROW((__LIBCCALL _iswblank_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswblank_l");
__LIBC int __NOTHROW((__LIBCCALL _iswalnum_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswalnum_l");
__LIBC int __NOTHROW((__LIBCCALL _iswprint_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswprint_l");
__LIBC int __NOTHROW((__LIBCCALL _iswgraph_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswgraph_l");
__LIBC int __NOTHROW((__LIBCCALL _iswcntrl_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswcntrl_l");
__LIBC int __NOTHROW((__LIBCCALL isleadbyte)(int __wc));
__LIBC int __NOTHROW((__LIBCCALL _isleadbyte_l)(int __wc, __locale_t __locale));
__LIBC wint_t __NOTHROW((__LIBCCALL _towupper_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("towupper_l");
__LIBC wint_t __NOTHROW((__LIBCCALL _towlower_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("towlower_l");
__LIBC int __NOTHROW((__LIBCCALL _iswctype_l)(wint_t __wc, wctype_t __type, __locale_t __locale)) __KOS_ASMNAME("iswctype_l");
__LIBC int __NOTHROW((__LIBCCALL is_wctype)(wint_t __wc, wctype_t __desc)) __ASMNAME("iswctype");
__LIBC int __NOTHROW((__LIBCCALL __iswcsymf)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL __iswcsym)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL _iswcsymf_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL _iswcsym_l)(wint_t __wc, __locale_t __locale));
#endif /* __USE_DOS */
#endif /* !__KERNEL__ */

#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC wint_t __NOTHROW((__LIBCCALL towlower)(wint_t __wc));
__LIBC wint_t __NOTHROW((__LIBCCALL towupper)(wint_t __wc));
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(towlower)
__NAMESPACE_STD_USING(towupper)
#ifdef __USE_DOS
__LIBC int __NOTHROW((__LIBCCALL isleadbyte)(int __wc));
__LIBC int __NOTHROW((__LIBCCALL _isleadbyte_l)(int __wc, __locale_t __locale));
__LIBC wint_t __NOTHROW((__LIBCCALL _towupper_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("towupper_l");
__LIBC wint_t __NOTHROW((__LIBCCALL _towlower_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("towlower_l");
__LIBC int __NOTHROW((__LIBCCALL _iswctype_l)(wint_t __wc, wctype_t __type, __locale_t __locale)) __KOS_ASMNAME("iswctype_l");
__LIBC int __NOTHROW((__LIBCCALL is_wctype)(wint_t __wc, wctype_t __desc)) __ASMNAME("iswctype");
__LIBC int __NOTHROW((__LIBCCALL __iswcsymf)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL __iswcsym)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL _iswcsymf_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL _iswcsym_l)(wint_t __wc, __locale_t __locale));
#endif /* __USE_DOS */
#endif /* !__KERNEL__ */
#endif /* !_WCTYPE_DEFINED */
#endif /* !__wisxxx_defined */

#if defined(__USE_ISOC99) || defined(__USE_GNU)
__NAMESPACE_STD_BEGIN
typedef const __int32_t *wctrans_t;
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wctrans_t)
#endif /* __USE_ISOC99 || __USE_GNU */

#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC wctrans_t __NOTHROW((__LIBCCALL wctrans)(char const *__prop));
__LIBC wint_t __NOTHROW((__LIBCCALL towctrans)(wint_t __wc, wctrans_t __desc));
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wctrans)
__NAMESPACE_STD_USING(towctrans)

#ifdef __USE_XOPEN2K8
__LIBC int __NOTHROW((__LIBCCALL iswalnum_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswalnum_l");
__LIBC int __NOTHROW((__LIBCCALL iswalpha_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswalpha_l");
__LIBC int __NOTHROW((__LIBCCALL iswcntrl_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswcntrl_l");
__LIBC int __NOTHROW((__LIBCCALL iswdigit_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswdigit_l");
__LIBC int __NOTHROW((__LIBCCALL iswgraph_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswgraph_l");
__LIBC int __NOTHROW((__LIBCCALL iswlower_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswlower_l");
__LIBC int __NOTHROW((__LIBCCALL iswprint_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswprint_l");
__LIBC int __NOTHROW((__LIBCCALL iswpunct_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswpunct_l");
__LIBC int __NOTHROW((__LIBCCALL iswspace_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswspace_l");
__LIBC int __NOTHROW((__LIBCCALL iswupper_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswupper_l");
__LIBC int __NOTHROW((__LIBCCALL iswxdigit_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswxdigit_l");
__LIBC int __NOTHROW((__LIBCCALL iswblank_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_iswblank_l");
__LIBC int __NOTHROW((__LIBCCALL iswctype_l)(wint_t __wc, wctype_t __desc, __locale_t __locale)) __PE_ASMNAME("_iswctype_l");
__LIBC wint_t __NOTHROW((__LIBCCALL towlower_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_towlower_l");
__LIBC wint_t __NOTHROW((__LIBCCALL towupper_l)(wint_t __wc, __locale_t __locale)) __PE_ASMNAME("_towupper_l");
__LIBC wctype_t __NOTHROW((__LIBCCALL wctype_l)(char const *__prop, __locale_t __locale));
__LIBC wctrans_t __NOTHROW((__LIBCCALL wctrans_l)(char const *__prop, __locale_t __locale));
__LIBC wint_t __NOTHROW((__LIBCCALL towctrans_l)(wint_t __wc, wctrans_t __desc, __locale_t __locale));
#endif /* __USE_XOPEN2K8 */

#ifndef _WCTYPE_INLINE_DEFINED
#define _WCTYPE_INLINE_DEFINED 1
#define iswalnum(wc)  __kos_iswctype((wc),_ISwalnum)
#define iswalpha(wc)  __kos_iswctype((wc),_ISwalpha)
#define iswcntrl(wc)  __kos_iswctype((wc),_ISwcntrl)
#define iswdigit(wc)  __kos_iswctype((wc),_ISwdigit)
#define iswlower(wc)  __kos_iswctype((wc),_ISwlower)
#define iswgraph(wc)  __kos_iswctype((wc),_ISwgraph)
#define iswprint(wc)  __kos_iswctype((wc),_ISwprint)
#define iswpunct(wc)  __kos_iswctype((wc),_ISwpunct)
#define iswspace(wc)  __kos_iswctype((wc),_ISwspace)
#define iswupper(wc)  __kos_iswctype((wc),_ISwupper)
#define iswxdigit(wc) __kos_iswctype((wc),_ISwxdigit)
#ifdef __USE_ISOC99
#define iswblank(wc)  __kos_iswctype((wc),_ISwblank)
#endif /* __USE_ISOC99 */
#if defined(__USE_KOS) || defined(__USE_DOS)
#if __SIZEOF_WCHAR_T__ == 4
#   define iswascii(wc) ((__UINT32_TYPE__)(wc) <= 0x7f)
#else /* #elif __SIZEOF_WCHAR_T__ == 2 */
#   define iswascii(wc) ((__UINT16_TYPE__)(wc) <= 0x7f)
#endif
#endif /* __USE_KOS || __USE_DOS */
#ifdef __USE_DOS
/* Dos doesn't actually evaluate the second parameter, but we do as an extension. */
#ifdef __USE_KOS
#define _iswalnum_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwalnum))
#define _iswalpha_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwalpha))
#define _iswcntrl_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwcntrl))
#define _iswdigit_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwdigit))
#define _iswlower_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwlower))
#define _iswgraph_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwgraph))
#define _iswprint_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwprint))
#define _iswpunct_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwpunct))
#define _iswspace_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwspace))
#define _iswupper_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwupper))
#define _iswxdigit_l(wc,lc)   __kos_iswctype((wc),(lc,_ISwxdigit))
#define _iswblank_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwblank))
#else /* __USE_KOS */
#define _iswalnum_l(wc,lc)    __kos_iswctype((wc),_ISwalnum)
#define _iswalpha_l(wc,lc)    __kos_iswctype((wc),_ISwalpha)
#define _iswcntrl_l(wc,lc)    __kos_iswctype((wc),_ISwcntrl)
#define _iswdigit_l(wc,lc)    __kos_iswctype((wc),_ISwdigit)
#define _iswlower_l(wc,lc)    __kos_iswctype((wc),_ISwlower)
#define _iswgraph_l(wc,lc)    __kos_iswctype((wc),_ISwgraph)
#define _iswprint_l(wc,lc)    __kos_iswctype((wc),_ISwprint)
#define _iswpunct_l(wc,lc)    __kos_iswctype((wc),_ISwpunct)
#define _iswspace_l(wc,lc)    __kos_iswctype((wc),_ISwspace)
#define _iswupper_l(wc,lc)    __kos_iswctype((wc),_ISwupper)
#define _iswxdigit_l(wc,lc)   __kos_iswctype((wc),_ISwxdigit)
#define _iswblank_l(wc,lc)    __kos_iswctype((wc),_ISwblank)
#endif /* !__USE_KOS */
#endif /* __USE_DOS */
#endif /* !_WCTYPE_INLINE_DEFINED */

#ifdef __USE_XOPEN2K8
#ifndef iswalnum_l
#define iswalnum_l(wc,lc)    iswctype_l(wc,_ISwalnum,lc)
#define iswalpha_l(wc,lc)    iswctype_l(wc,_ISwalpha,lc)
#define iswcntrl_l(wc,lc)    iswctype_l(wc,_ISwcntrl,lc)
#define iswdigit_l(wc,lc)    iswctype_l(wc,_ISwdigit,lc)
#define iswlower_l(wc,lc)    iswctype_l(wc,_ISwlower,lc)
#define iswgraph_l(wc,lc)    iswctype_l(wc,_ISwgraph,lc)
#define iswprint_l(wc,lc)    iswctype_l(wc,_ISwprint,lc)
#define iswpunct_l(wc,lc)    iswctype_l(wc,_ISwpunct,lc)
#define iswspace_l(wc,lc)    iswctype_l(wc,_ISwspace,lc)
#define iswupper_l(wc,lc)    iswctype_l(wc,_ISwupper,lc)
#define iswxdigit_l(wc,lc)   iswctype_l(wc,_ISwxdigit,lc)
#define iswblank_l(wc,lc)    iswctype_l(wc,_ISwblank,lc)
#endif /* !iswalnum_l */
#endif /* __USE_XOPEN2K8 */
                        
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_WCTYPE_H */
