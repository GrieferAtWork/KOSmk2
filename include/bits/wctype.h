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
#ifndef _BITS_WCTYPE_H
#define _BITS_WCTYPE_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/dos-ctype.h>

__SYSDECL_BEGIN

#ifndef __std_wctype_t_defined
#define __std_wctype_t_defined 1
__NAMESPACE_STD_BEGIN
typedef unsigned long int wctype_t;
__NAMESPACE_STD_END
#endif /* !__std_wctype_t_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __wctype_t_defined
#define __wctype_t_defined 1
__NAMESPACE_STD_USING(wctype_t)
#endif /* !__wctype_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef __KERNEL__
#ifdef __CRT_GLC
#ifndef _ISwbit
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
#endif /* !_ISwbit */
#endif /* __CRT_GLC */

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */

#ifndef _WCTYPE_INLINE_DEFINED
#define _WCTYPE_INLINE_DEFINED 1
#ifdef __CRT_GLC
#ifndef ____kos_iswctype_defined
#define ____kos_iswctype_defined 1
__NAMESPACE_STD_BEGIN
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__kos_iswctype,
                  (wint_t __wc, wctype_t __desc),iswctype,(__wc,__desc))
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(__kos_iswctype)
#endif /* !__CXX_SYSTEM_HEADER */
#endif /* !____kos_iswctype_defined */
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

/* Dos doesn't actually evaluate the second parameter, but we do as an extension. */
#if defined(__USE_KOS) || !defined(__USE_DOS)
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
#else /* __USE_KOS || !__USE_DOS */
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
#endif /* !__USE_KOS && __USE_DOS */
#else /* __CRT_GLC */
/* TODO: Implement inline versions using DOS character bits. */
#endif /* __CRT_GLC */

#if __SIZEOF_WCHAR_T__ == 4
#   define iswascii(wc) ((__UINT32_TYPE__)(wc) <= 0x7f)
#else /* #elif __SIZEOF_WCHAR_T__ == 2 */
#   define iswascii(wc) ((__UINT16_TYPE__)(wc) <= 0x7f)
#endif
#endif /* !_WCTYPE_INLINE_DEFINED */

#ifndef _WCTYPE_DEFINED
#define _WCTYPE_DEFINED 1

__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswalnum)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswalpha)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswcntrl)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswdigit)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswgraph)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswlower)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswprint)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswpunct)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswspace)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswupper)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswxdigit)(wint_t __wc));
__LIBC __WUNUSED wctype_t __NOTHROW((__LIBCCALL wctype)(char const *__prop));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswctype)(wint_t __wc, wctype_t __desc)) __DOS_FUNC(iswctype);
#ifdef __USE_ISOC99
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iswblank)(wint_t __wc));
#endif /* __USE_ISOC99 */
__LIBC __WUNUSED wint_t __NOTHROW((__LIBCCALL towlower)(wint_t __wc));
__LIBC __WUNUSED wint_t __NOTHROW((__LIBCCALL towupper)(wint_t __wc));
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
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
__NAMESPACE_STD_USING(towlower)
__NAMESPACE_STD_USING(towupper)
#endif /* !__CXX_SYSTEM_HEADER */

#if defined(__USE_KOS) || defined(__USE_DOS)
__LIBC int __NOTHROW((__LIBCCALL iswascii)(wint_t __wc));
#endif /* __USE_KOS || __USE_DOS */

#ifdef __USE_DOS
#ifndef __wctype_t_defined
#define __wctype_t_defined 1
__NAMESPACE_STD_USING(wctype_t)
#endif /* !__wctype_t_defined */

__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswalnum_l,(wint_t __wc, __locale_t __locale),iswalnum_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswalpha_l,(wint_t __wc, __locale_t __locale),iswalpha_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswcntrl_l,(wint_t __wc, __locale_t __locale),iswcntrl_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswdigit_l,(wint_t __wc, __locale_t __locale),iswdigit_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswgraph_l,(wint_t __wc, __locale_t __locale),iswgraph_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswlower_l,(wint_t __wc, __locale_t __locale),iswlower_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswprint_l,(wint_t __wc, __locale_t __locale),iswprint_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswpunct_l,(wint_t __wc, __locale_t __locale),iswpunct_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswspace_l,(wint_t __wc, __locale_t __locale),iswspace_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswupper_l,(wint_t __wc, __locale_t __locale),iswupper_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswxdigit_l,(wint_t __wc, __locale_t __locale),iswxdigit_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswblank_l,(wint_t __wc, __locale_t __locale),iswblank_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,wint_t,__LIBCCALL,_towupper_l,(wint_t __wc, __locale_t __locale),towupper_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,wint_t,__LIBCCALL,_towlower_l,(wint_t __wc, __locale_t __locale),towlower_l,(__wc,__locale))
__REDIRECT_IFKOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iswctype_l,(wint_t __wc, wctype_t __type, __locale_t __locale),iswctype_l,(__wc,__type,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,is_wctype,(wint_t __wc, wctype_t __desc),iswctype,(__wc,__desc))
#if !defined(__GLC_COMPAT__) && defined(__CRT_DOS)
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isleadbyte)(int __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL _isleadbyte_l)(int __wc, __locale_t __locale));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL __iswcsymf)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL __iswcsym)(wint_t __wc));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL _iswcsymf_l)(wint_t __wc, __locale_t __locale));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL _iswcsym_l)(wint_t __wc, __locale_t __locale));
#else /* !__GLC_COMPAT__ && __CRT_DOS */
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL isleadbyte)(int __wc)) { return __wc >= 192 && __wc <= 255; }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isleadbyte_l)(int __wc, __locale_t __UNUSED(__locale))) { return __wc >= 192 && __wc <= 255; }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL __iswcsymf)(wint_t __wc)) { return __NAMESPACE_STD_SYM iswalpha(wc) || wc == '_' }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL __iswcsym)(wint_t __wc)) { return __NAMESPACE_STD_SYM iswalnum(wc) || wc == '_' }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _iswcsymf_l)(wint_t __wc, __locale_t __locale)) { return _iswalpha_l(wc,__locale) || wc == '_' }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _iswcsym_l)(wint_t __wc, __locale_t __locale)) { return _iswalnum_l(wc,__locale) || wc == '_' }
#endif /* __GLC_COMPAT__ || !__CRT_DOS */
#endif /* __USE_DOS */
#endif /* !_WCTYPE_DEFINED */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_BITS_WCTYPE_H */
