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
#ifndef _WCHAR_H
#define _WCHAR_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/wchar.h>
#include <hybrid/typecore.h>
#include <hybrid/limitcore.h>
#include <__malldefs.h>
#ifdef __USE_DOS
#include <bits/stat.h>
#endif /* __USE_DOS */

#if defined(__USE_XOPEN2K8) || defined(__USE_GNU)
#   include <xlocale.h>
#endif /* __USE_XOPEN2K8 || __USE_GNU */
#if defined(__USE_XOPEN) && !defined(__USE_UNIX98)
#   include <wctype.h>
#elif (defined(__USE_UNIX98) && !defined(__USE_GNU)) || \
       defined(__USE_DOS)
#ifndef __wisxxx_defined
#define __wisxxx_defined 1
#ifndef _WCTYPE_DEFINED
#define _WCTYPE_DEFINED 1
__DECL_BEGIN

#ifndef __wctype_t_defined
#define __wctype_t_defined 1
__NAMESPACE_STD_BEGIN
typedef unsigned long int wctype_t;
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wctype_t)
#endif /* !__wctype_t_defined */

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
__LIBC wctype_t __NOTHROW((__LIBCCALL wctype)(char const *__prop));
__LIBC int __NOTHROW((__LIBCCALL iswctype)(wint_t __wc, wctype_t __desc)) __DOS_FUNC(iswctype);
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
#if defined(__USE_KOS) || defined(__USE_DOS)
__LIBC int __NOTHROW((__LIBCCALL iswascii)(wint_t __wc));
#endif /* __USE_KOS || __USE_DOS */
#ifdef __USE_DOS
__LIBC int __NOTHROW((__LIBCCALL _iswalpha_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswalpha_l");
__LIBC int __NOTHROW((__LIBCCALL _iswupper_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswupper_l");
__LIBC int __NOTHROW((__LIBCCALL _iswlower_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswlower_l");
__LIBC int __NOTHROW((__LIBCCALL _iswdigit_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswdigit_l");
__LIBC int __NOTHROW((__LIBCCALL _iswxdigit_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswxdigit_l");
__LIBC int __NOTHROW((__LIBCCALL _iswspace_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswspace_l");
__LIBC int __NOTHROW((__LIBCCALL _iswpunct_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswpunct_l");
__LIBC int __NOTHROW((__LIBCCALL _iswblank_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswblank_l");
__LIBC int __NOTHROW((__LIBCCALL _iswalnum_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswalnum_l");
__LIBC int __NOTHROW((__LIBCCALL _iswprint_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswprint_l");
__LIBC int __NOTHROW((__LIBCCALL _iswgraph_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswgraph_l");
__LIBC int __NOTHROW((__LIBCCALL _iswcntrl_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("iswcntrl_l");
__LIBC int __NOTHROW((__LIBCCALL isleadbyte)(int __wc));
__LIBC int __NOTHROW((__LIBCCALL _isleadbyte_l)(int __wc, __locale_t __locale));
__LIBC wint_t __NOTHROW((__LIBCCALL _towupper_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("towupper_l");
__LIBC wint_t __NOTHROW((__LIBCCALL _towlower_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("towlower_l");
__LIBC int __NOTHROW((__LIBCCALL _iswctype_l)(wint_t __wc, wctype_t __type, __locale_t __locale)) __ASMNAME("iswctype_l");
__LIBC int __NOTHROW((__LIBCCALL is_wctype)(wint_t __wc, wctype_t __desc)) __ASMNAME("iswctype");
__LIBC int __NOTHROW((__LIBCCALL __iswcsymf)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL __iswcsym)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL _iswcsymf_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL _iswcsym_l)(wint_t __wc, __locale_t __locale));
#endif /* __USE_DOS */
#endif /* !__KERNEL__ */

__NAMESPACE_STD_BEGIN
#if defined(__USE_ISOC99) || defined(__USE_GNU)
typedef const __int32_t *wctrans_t;
#endif /* __USE_ISOC99 || __USE_GNU */
#ifndef __KERNEL__
__LIBC wint_t __NOTHROW((__LIBCCALL towlower)(wint_t __wc));
__LIBC wint_t __NOTHROW((__LIBCCALL towupper)(wint_t __wc));
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END
#if defined(__USE_ISOC99) || defined(__USE_GNU)
__NAMESPACE_STD_USING(wctrans_t)
#endif /* __USE_ISOC99 || __USE_GNU */
#ifndef __KERNEL__
__NAMESPACE_STD_USING(towlower)
__NAMESPACE_STD_USING(towupper)
#endif /* !__KERNEL__ */
#ifdef __USE_DOS
__LIBC int __NOTHROW((__LIBCCALL isleadbyte)(int __wc));
__LIBC int __NOTHROW((__LIBCCALL _isleadbyte_l)(int __wc, __locale_t __locale));
__LIBC wint_t __NOTHROW((__LIBCCALL _towupper_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("towupper_l");
__LIBC wint_t __NOTHROW((__LIBCCALL _towlower_l)(wint_t __wc, __locale_t __locale)) __ASMNAME("towlower_l");
__LIBC int __NOTHROW((__LIBCCALL _iswctype_l)(wint_t __wc, wctype_t __type, __locale_t __locale)) __ASMNAME("iswctype_l");
__LIBC int __NOTHROW((__LIBCCALL is_wctype)(wint_t __wc, wctype_t __desc)) __ASMNAME("iswctype");
__LIBC int __NOTHROW((__LIBCCALL __iswcsymf)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL __iswcsym)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL _iswcsymf_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL _iswcsym_l)(wint_t __wc, __locale_t __locale));
#endif /* __USE_DOS */

__DECL_END
#endif /* !_WCTYPE_DEFINED */
#endif /* !__wisxxx_defined */
#endif

__DECL_BEGIN

/* Define 'FILE' */
#if !defined(__KERNEL__) && (defined(__USE_UNIX98) || defined(__USE_XOPEN2K))
#ifdef __NAMESPACE_STD_EXISTS
#ifndef __std_FILE_defined
#define __std_FILE_defined 1
__NAMESPACE_STD_BEGIN
typedef __FILE FILE;
__NAMESPACE_STD_END
#endif /* !__std_FILE_defined */
#ifndef __FILE_defined
#define __FILE_defined 1
__NAMESPACE_STD_USING(FILE)
#endif /* !__FILE_defined */
#else /* __NAMESPACE_STD_EXISTS */
#ifndef __FILE_defined
#define __FILE_defined 1
typedef __FILE FILE;
#endif /* !__FILE_defined */
#endif /* !__NAMESPACE_STD_EXISTS */
#endif /* !__KERNEL__ && (__USE_UNIX98 || __USE_XOPEN2K) */

/* Define 'wchar_t' */
#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif

/* Define 'size_t' */
#ifdef __NAMESPACE_STD_EXISTS
__NAMESPACE_STD_BEGIN
#ifndef __std_size_t_defined
#define __std_size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__std_size_t_defined */
#ifndef __std_wint_t_defined
#define __std_wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__std_wint_t_defined */
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
#ifndef __size_t_defined
#define __size_t_defined 1
__NAMESPACE_STD_USING(size_t)
#endif /* !__size_t_defined */
#ifndef __wint_t_defined
#define __wint_t_defined 1
__NAMESPACE_STD_USING(wint_t)
#endif /* !__wint_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#else /* STD-namespace */
#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */
#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */
#endif /* !STD-namespace */

/* Define 'NULL' */
#ifndef NULL
#ifdef __INTELLISENSE__
#   define NULL nullptr
#elif defined(__cplusplus) || defined(__LINKER__)
#   define NULL          0
#else
#   define NULL ((void *)0)
#endif
#endif

#ifndef ____mbstate_t_defined
#define ____mbstate_t_defined 1
typedef struct __mbstate {
 int            __count;
 union { wint_t __wch; char   __wchb[4]; } __value;
} __mbstate_t;
#endif /* !____mbstate_t_defined */

#ifndef __std_mbstate_t_defined
#define __std_mbstate_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __mbstate_t mbstate_t;
__NAMESPACE_STD_END
#endif /* !__std_mbstate_t_defined */

#if defined(__USE_GNU) || defined(__USE_XOPEN2K8)
#ifndef __mbstate_t_defined
#define __mbstate_t_defined 1
__NAMESPACE_STD_USING(mbstate_t)
#endif /* !__mbstate_t_defined */
#endif

#ifndef WCHAR_MIN
#define WCHAR_MIN __WCHAR_MIN__
#define WCHAR_MAX __WCHAR_MAX__
#endif

#ifndef WEOF
#if __SIZEOF_WCHAR_T__ == 4
#define WEOF             0xffffffffu
#else
#define WEOF    (wint_t)(0xffff)
#endif
#endif

__NAMESPACE_STD_BEGIN
struct tm;
#ifndef __KERNEL__
#ifndef __wcscpy_defined
__LIBC __NONNULL((1,2)) wchar_t *(__LIBCCALL wcscpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src);
__LIBC __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsncpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __n);
__LIBC __NONNULL((1,2)) wchar_t *(__LIBCCALL wcscat)(wchar_t *__restrict __dst, wchar_t const *__restrict __src);
__LIBC __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsncat)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __n);
#endif /* !__wcscpy_defined */
#ifndef __wcscmp_defined
__LIBC __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcscmp)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcsncmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n);
__LIBC int (__LIBCCALL wcscoll)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC size_t (__LIBCCALL wcsxfrm)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n);
#endif /* !__wcscmp_defined */
__LIBC wint_t (__LIBCCALL btowc)(int __c);
__LIBC int (__LIBCCALL wctob)(wint_t __c);
__LIBC __ATTR_PURE int (__LIBCCALL mbsinit)(mbstate_t const *__ps);
__LIBC size_t (__LIBCCALL mbrtowc)(wchar_t *__restrict __pwc, char const *__restrict __s, size_t __n, mbstate_t *__restrict __p);
__LIBC size_t (__LIBCCALL wcrtomb)(char *__restrict __s, wchar_t __wc, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL __mbrlen)(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL mbrlen)(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL mbsrtowcs)(wchar_t *__restrict __dst, char const **__restrict __src, size_t __len, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL wcsrtombs)(char *__restrict __dst, wchar_t const **__restrict __src, size_t __len, mbstate_t *__restrict __ps);
__LIBC double (__LIBCCALL wcstod)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
__LIBC long int (__LIBCCALL wcstol)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
__LIBC unsigned long int (__LIBCCALL wcstoul)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
__LIBC wint_t (__LIBCCALL fgetwc)(__FILE *__stream);
__LIBC wint_t (__LIBCCALL getwc)(__FILE *__stream);
__LIBC wint_t (__LIBCCALL getwchar)(void);
__LIBC wint_t (__LIBCCALL fputwc)(wchar_t __wc, __FILE *__stream);
__LIBC wint_t (__LIBCCALL putwc)(wchar_t __wc, __FILE *__stream);
__LIBC wint_t (__LIBCCALL putwchar)(wchar_t __wc);
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __ws, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__LIBC wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __ws, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgetws_sz");
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__LIBC wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __ws, int __n, __FILE *__restrict __stream);
#endif /* !__USE_KOS */
__LIBC int (__LIBCCALL fputws)(wchar_t const *__restrict __ws, __FILE *__restrict __stream);
__LIBC wint_t (__LIBCCALL ungetwc)(wint_t __wc, __FILE *__stream);
__LIBC size_t (__LIBCCALL wcsftime)(wchar_t *__restrict __s, size_t __maxsize, wchar_t const *__restrict __format, struct tm const *__restrict __tp);
#ifndef __wcstok_defined
__LIBC wchar_t *(__LIBCCALL wcstok)(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr);
#endif /* !__wcstok_defined */
#ifndef __wcslen_defined
__LIBC size_t (__LIBCCALL wcslen)(wchar_t const *__s);
#endif /* !__wcslen_defined */
#ifndef __wcsspn_defined
__LIBC __ATTR_PURE size_t (__LIBCCALL wcsspn)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __ATTR_PURE size_t (__LIBCCALL wcscspn)(wchar_t const *__haystack, wchar_t const *__reject);
#endif /* !__wcsspn_defined */
__LIBC __ATTR_PURE int (__LIBCCALL wmemcmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n);
__LIBC wchar_t *(__LIBCCALL wmemcpy)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n);
__LIBC wchar_t *(__LIBCCALL wmemmove)(wchar_t *__s1, wchar_t const *__s2, size_t __n);
__LIBC wchar_t *(__LIBCCALL wmemset)(wchar_t *__s, wchar_t __c, size_t __n);
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
#ifndef __wcschr_defined
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcschr)(wchar_t *__haystack, wchar_t __wc) __ASMNAME("wcschr");
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcsrchr)(wchar_t *__haystack, wchar_t __wc) __ASMNAME("wcsrchr");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wcschr)(wchar_t const *__haystack, wchar_t __wc) __ASMNAME("wcschr");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wcsrchr)(wchar_t const *__haystack, wchar_t __wc) __ASMNAME("wcsrchr");
#endif /* !__wcschr_defined */
#ifndef __wcsstr_defined
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcspbrk)(wchar_t *__haystack, wchar_t const *__accept) __ASMNAME("wcspbrk");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wcspbrk)(wchar_t const *__haystack, wchar_t const *__accept) __ASMNAME("wcspbrk");
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcsstr)(wchar_t *__haystack, wchar_t const *__needle) __ASMNAME("wcsstr");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wcsstr)(wchar_t const *__haystack, wchar_t const *__needle) __ASMNAME("wcsstr");
#endif /* !__wcsstr_defined */
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wmemchr)(wchar_t *__s, wchar_t __c, size_t __n) __ASMNAME("wmemchr");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wmemchr)(wchar_t const *__s, wchar_t __c, size_t __n) __ASMNAME("wmemchr");
}
#else
#ifndef __wcschr_defined
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcschr)(wchar_t const *__haystack, wchar_t __wc);
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcsrchr)(wchar_t const *__haystack, wchar_t __wc);
#endif /* !__wcschr_defined */
#ifndef __wcsstr_defined
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcspbrk)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcsstr)(wchar_t const *__haystack, wchar_t const *__needle);
#endif /* !__wcsstr_defined */
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wmemchr)(wchar_t const *__s, wchar_t __c, size_t __n);
#endif
#if defined(__USE_ISOC95) || defined(__USE_UNIX98)
__LIBC int (__LIBCCALL fwide)(__FILE *__fp, int __mode);
#ifdef __USE_KOS
__LIBC ssize_t (__LIBCCALL fwprintf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL swprintf)(wchar_t *__restrict __s, size_t __n, wchar_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL vfwprintf)(__FILE *__restrict __s, wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC ssize_t (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC ssize_t (__LIBCCALL vswprintf)(wchar_t *__restrict __s, size_t __n, wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC ssize_t (__LIBCCALL fwscanf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL swscanf)(wchar_t const *__restrict __s, wchar_t const *__restrict __format, ...);
#else /* __USE_KOS */
__LIBC int (__LIBCCALL fwprintf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL swprintf)(wchar_t *__restrict __s, size_t __n, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vfwprintf)(__FILE *__restrict __s, wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC int (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC int (__LIBCCALL vswprintf)(wchar_t *__restrict __s, size_t __n, wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC int (__LIBCCALL fwscanf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL swscanf)(wchar_t const *__restrict __s, wchar_t const *__restrict __format, ...);
#endif /* !__USE_KOS */
#endif /* __USE_ISOC95 || __USE_UNIX98 */
#ifdef __USE_ISOC99
__LIBC float (__LIBCCALL wcstof)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
__LIBC long double (__LIBCCALL wcstold)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
__LIBC __LONGLONG (__LIBCCALL wcstoll)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
__LIBC __ULONGLONG (__LIBCCALL wcstoull)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
#ifdef __USE_KOS
__LIBC ssize_t (__LIBCCALL vfwscanf)(__FILE *__restrict __s, wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC ssize_t (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC ssize_t (__LIBCCALL vswscanf)(wchar_t const *__restrict __s, wchar_t const *__restrict __format, __VA_LIST __arg);
#else /* __USE_KOS */
__LIBC int (__LIBCCALL vfwscanf)(__FILE *__restrict __s, wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC int (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC int (__LIBCCALL vswscanf)(wchar_t const *__restrict __s, wchar_t const *__restrict __format, __VA_LIST __arg);
#endif /* !__USE_KOS */
#endif /* __USE_ISOC99 */
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END

__NAMESPACE_STD_USING(tm)
#ifndef __KERNEL__
#ifndef __wcscpy_defined
#define __wcscpy_defined 1
__NAMESPACE_STD_USING(wcscpy)
__NAMESPACE_STD_USING(wcsncpy)
__NAMESPACE_STD_USING(wcscat)
__NAMESPACE_STD_USING(wcsncat)
#endif /* !__wcscpy_defined */
#ifndef __wcscmp_defined
#define __wcscmp_defined 1
__NAMESPACE_STD_USING(wcscmp)
__NAMESPACE_STD_USING(wcsncmp)
#endif /* !__wcscmp_defined */
__NAMESPACE_STD_USING(wcscoll)
__NAMESPACE_STD_USING(wcsxfrm)
__NAMESPACE_STD_USING(btowc)
__NAMESPACE_STD_USING(wctob)
__NAMESPACE_STD_USING(mbsinit)
__NAMESPACE_STD_USING(mbrtowc)
__NAMESPACE_STD_USING(wcrtomb)
__NAMESPACE_STD_USING(__mbrlen)
__NAMESPACE_STD_USING(mbrlen)
__NAMESPACE_STD_USING(mbsrtowcs)
__NAMESPACE_STD_USING(wcsrtombs)
__NAMESPACE_STD_USING(wcstod)
__NAMESPACE_STD_USING(wcstol)
__NAMESPACE_STD_USING(wcstoul)
__NAMESPACE_STD_USING(fgetwc)
__NAMESPACE_STD_USING(getwc)
__NAMESPACE_STD_USING(getwchar)
__NAMESPACE_STD_USING(fputwc)
__NAMESPACE_STD_USING(putwc)
__NAMESPACE_STD_USING(putwchar)
__NAMESPACE_STD_USING(fgetws)
__NAMESPACE_STD_USING(fputws)
__NAMESPACE_STD_USING(ungetwc)
__NAMESPACE_STD_USING(wcsftime)
#ifndef __wcstok_defined
#define __wcstok_defined 1
__NAMESPACE_STD_USING(wcstok)
#endif /* !__wcstok_defined */
#ifndef __wcslen_defined
#define __wcslen_defined 1
__NAMESPACE_STD_USING(wcslen)
#endif /* !__wcslen_defined */
#ifndef __wcsspn_defined
#define __wcsspn_defined 1
__NAMESPACE_STD_USING(wcsspn)
__NAMESPACE_STD_USING(wcscspn)
#endif /* !__wcsspn_defined */
__NAMESPACE_STD_USING(wmemcmp)
__NAMESPACE_STD_USING(wmemcpy)
__NAMESPACE_STD_USING(wmemmove)
__NAMESPACE_STD_USING(wmemset)
#ifndef __wcschr_defined
#define __wcschr_defined 1
__NAMESPACE_STD_USING(wcschr)
__NAMESPACE_STD_USING(wcsrchr)
#endif /* !__wcschr_defined */
#ifndef __wcsstr_defined
#define __wcsstr_defined 1
__NAMESPACE_STD_USING(wcsstr)
__NAMESPACE_STD_USING(wcspbrk)
#endif /* !__wcsstr_defined */
__NAMESPACE_STD_USING(wmemchr)
#if defined(__USE_ISOC95) || defined(__USE_UNIX98)
__NAMESPACE_STD_USING(fwide)
__NAMESPACE_STD_USING(fwprintf)
__NAMESPACE_STD_USING(wprintf)
__NAMESPACE_STD_USING(swprintf)
__NAMESPACE_STD_USING(vfwprintf)
__NAMESPACE_STD_USING(vwprintf)
__NAMESPACE_STD_USING(vswprintf)
__NAMESPACE_STD_USING(fwscanf)
__NAMESPACE_STD_USING(wscanf)
__NAMESPACE_STD_USING(swscanf)
#endif /* __USE_ISOC95 || __USE_UNIX98 */
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(wcstof)
__NAMESPACE_STD_USING(wcstold)
__NAMESPACE_STD_USING(wcstoll)
__NAMESPACE_STD_USING(wcstoull)
__NAMESPACE_STD_USING(vfwscanf)
__NAMESPACE_STD_USING(vwscanf)
__NAMESPACE_STD_USING(vswscanf)
#endif /* __USE_ISOC99 */
#endif /* !__KERNEL__ */

#ifndef __KERNEL__
#ifdef __USE_XOPEN2K8
__LIBC int (__LIBCCALL wcscasecmp)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC int (__LIBCCALL wcsncasecmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n);
__LIBC int (__LIBCCALL wcscasecmp_l)(wchar_t const *__s1, wchar_t const *__s2, __locale_t __loc);
__LIBC int (__LIBCCALL wcsncasecmp_l)(wchar_t const *__s1, wchar_t const *__s2, size_t __n, __locale_t __loc);
__LIBC int (__LIBCCALL wcscoll_l)(wchar_t const *__s1, wchar_t const *__s2, __locale_t __loc);
__LIBC size_t (__LIBCCALL wcsxfrm_l)(wchar_t *__s1, wchar_t const *__s2, size_t __n, __locale_t __loc);
#ifndef __wcsdup_defined
#define __wcsdup_defined 1
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC wchar_t *(__LIBCCALL wcsdup)(wchar_t const *__restrict __str);
#endif /* !__wcsdup_defined */
#ifndef __wcsnlen_defined
#define __wcsnlen_defined 1
__LIBC __ATTR_PURE size_t (__LIBCCALL wcsnlen)(wchar_t const *__s, size_t __maxlen);
#endif /* !__wcsnlen_defined */
__LIBC size_t (__LIBCCALL mbsnrtowcs)(wchar_t *__restrict __dst, char const **__restrict __src, size_t __nmc, size_t __len, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL wcsnrtombs)(char *__restrict __dst, wchar_t const **__restrict __src, size_t __nwc, size_t __len, mbstate_t *__restrict __ps);
__LIBC wchar_t *(__LIBCCALL wcpcpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src);
__LIBC wchar_t *(__LIBCCALL wcpncpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __n);
__LIBC __FILE *(__LIBCCALL open_wmemstream)(wchar_t **__bufloc, size_t *__sizeloc);
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_XOPEN
#ifdef __USE_KOS
__LIBC __ssize_t (__LIBCCALL wcwidth)(wchar_t __c);
__LIBC __ssize_t (__LIBCCALL wcswidth)(wchar_t const *__s, size_t __n);
#else /* __USE_KOS */
__LIBC int (__LIBCCALL wcwidth)(wchar_t __c);
__LIBC int (__LIBCCALL wcswidth)(wchar_t const *__s, size_t __n);
#endif /* !__USE_KOS */
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcswcs)(wchar_t *__haystack, wchar_t const *__needle) __ASMNAME("wcswcs");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wcswcs)(wchar_t const *__haystack, wchar_t const *__needle) __ASMNAME("wcswcs");
}
#else
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcswcs)(wchar_t const *__haystack, wchar_t const *__needle);
#endif
#endif /* __USE_XOPEN */

#ifdef __USE_GNU
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcschrnul)(wchar_t const *__s, wchar_t __wc);
__LIBC wchar_t *(__LIBCCALL wmempcpy)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n);
__LIBC __LONGLONG (__LIBCCALL wcstoq)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
__LIBC __ULONGLONG (__LIBCCALL wcstouq)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
__LIBC long int (__LIBCCALL wcstol_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __loc);
__LIBC unsigned long int (__LIBCCALL wcstoul_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __loc);
__LIBC __LONGLONG (__LIBCCALL wcstoll_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __loc);
__LIBC __ULONGLONG (__LIBCCALL wcstoull_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __loc);
__LIBC double (__LIBCCALL wcstod_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __loc);
__LIBC float (__LIBCCALL wcstof_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __loc);
__LIBC long double (__LIBCCALL wcstold_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __loc);
__LIBC wint_t (__LIBCCALL getwc_unlocked)(__FILE *__stream);
__LIBC wint_t (__LIBCCALL getwchar_unlocked)(void);
__LIBC wint_t (__LIBCCALL fgetwc_unlocked)(__FILE *__stream);
__LIBC wint_t (__LIBCCALL fputwc_unlocked)(wchar_t __wc, __FILE *__stream);
__LIBC wint_t (__LIBCCALL putwc_unlocked)(wchar_t __wc, __FILE *__stream);
__LIBC wint_t (__LIBCCALL putwchar_unlocked)(wchar_t __wc);
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC wchar_t *(__LIBCCALL fgetws_unlocked)(wchar_t *__restrict __ws, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__LIBC wchar_t *(__LIBCCALL fgetws_unlocked)(wchar_t *__restrict __ws, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgetws_unlocked_sz");
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__LIBC wchar_t *(__LIBCCALL fgetws_unlocked)(wchar_t *__restrict __ws, int __n, __FILE *__restrict __stream);
#endif /* !__USE_KOS */
__LIBC int (__LIBCCALL fputws_unlocked)(wchar_t const *__restrict __ws, __FILE *__restrict __stream);
__LIBC size_t (__LIBCCALL wcsftime_l)(wchar_t *__restrict __s, size_t __maxsize, wchar_t const *__restrict __format, struct tm const *__restrict __tp, __locale_t __loc);
#endif /* __USE_GNU */

#ifdef __USE_KOS
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC wchar_t *(__LIBCCALL wcsend)(wchar_t *__restrict __s) __ASMNAME("wcsend");
__LIBC wchar_t *(__LIBCCALL wcsnend)(wchar_t *__restrict __s, size_t __n) __ASMNAME("wcsnend");
__LIBC wchar_t const *(__LIBCCALL wcsend)(wchar_t const *__restrict __s) __ASMNAME("wcsend");
__LIBC wchar_t const *(__LIBCCALL wcsnend)(wchar_t const *__restrict __s, size_t __n) __ASMNAME("wcsnend");
}
#else
__LIBC wchar_t *(__LIBCCALL wcsend)(wchar_t const *__restrict __s);
__LIBC wchar_t *(__LIBCCALL wcsnend)(wchar_t const *__restrict __s, size_t __n);
#endif
#endif /* __USE_KOS */
#endif /* !__KERNEL__ */

/* DOS extensions. */
#ifdef __USE_DOS

#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t intptr_t;
#endif /* !__intptr_t_defined */

#ifndef _iobuf
#define _iobuf   _IO_FILE
#endif /* !_iobuf */

#ifndef __stdstreams_defined
#define __stdstreams_defined 1
#undef stdin
#undef stdout
#undef stderr
__LIBC __FILE *(stdin);
__LIBC __FILE *(stdout);
__LIBC __FILE *(stderr);
#define stdin  stdin
#define stdout stdout
#define stderr stderr
#endif /* !__stdstreams_defined */

#ifndef _FSIZE_T_DEFINED
#define _FSIZE_T_DEFINED
typedef __UINT32_TYPE__ _fsize_t;
#endif /* _FSIZE_T_DEFINED */

#ifndef _WFINDDATA_T_DEFINED
#define _WFINDDATA_T_DEFINED 1
/* Safely first! */
#undef attrib
#undef time_create
#undef time_access
#undef time_write
#undef size
#undef name

struct _wfinddata32_t {
 __UINT32_TYPE__ attrib;
 __time32_t      time_create;
 __time32_t      time_access;
 __time32_t      time_write;
 _fsize_t        size;
 wchar_t         name[260];
};

struct _wfinddata32i64_t {
 __UINT32_TYPE__ attrib;
 __time32_t      time_create;
 __time32_t      time_access;
 __time32_t      time_write;
 __INT64_TYPE__  size;
 wchar_t         name[260];
};

struct _wfinddata64i32_t {
 __UINT32_TYPE__ attrib;
 __time64_t      time_create;
 __time64_t      time_access;
 __time64_t      time_write;
 _fsize_t        size;
 wchar_t         name[260];
};

struct _wfinddata64_t {
 __UINT32_TYPE__ attrib;
 __time64_t      time_create;
 __time64_t      time_access;
 __time64_t      time_write;
 __INT64_TYPE__  size;
 wchar_t         name[260];
};

#ifdef __USE_TIME_BITS64
#define _wfinddata_t    _wfinddata32_t
#define _wfinddatai64_t _wfinddata32i64_t
#define _wfindfirst     _wfindfirst32
#define _wfindnext      _wfindnext32
#define _wfindfirsti64  _wfindfirst32i64
#define _wfindnexti64   _wfindnext32i64
#else /* __USE_TIME_BITS64 */
#define _wfinddata_t    _wfinddata64i32_t
#define _wfinddatai64_t _wfinddata64_t
#define _wfindfirst     _wfindfirst64i32
#define _wfindnext      _wfindnext64i32
#define _wfindfirsti64  _wfindfirst64
#define _wfindnexti64   _wfindnext64
#endif /* !__USE_TIME_BITS64 */
#endif /* _WFINDDATA_T_DEFINED */

#ifndef _WDIRECT_DEFINED
#define _WDIRECT_DEFINED 1
__LIBC __NONNULL((1)) wchar_t *(__LIBCCALL _wgetcwd)(wchar_t *__dstbuf, int __sizeinwchars);
__LIBC __NONNULL((1)) wchar_t *(__LIBCCALL _wgetdcwd)(int __drive, wchar_t *__dstbuf, int __sizeinwchars);
#define _wgetdcwd_nolock    _wgetdcwd
__LIBC __NONNULL((1)) int (__LIBCCALL _wchdir)(wchar_t const *__path) __WFS_FUNC_(wchdir);
__LIBC __NONNULL((1)) int (__LIBCCALL _wmkdir)(wchar_t const *__path) __WFS_FUNC_(wmkdir);
__LIBC __NONNULL((1)) int (__LIBCCALL _wrmdir)(wchar_t const *__path) __WFS_FUNC_(wrmdir);
#endif /* !_WDIRECT_DEFINED */

#ifndef _WIO_DEFINED
#define _WIO_DEFINED 1
__LIBC int (__LIBCCALL _wcreat)(wchar_t const *__file, int __pmode) __WFS_FUNC(_wcreat);
__LIBC int (__ATTR_CDECL _wopen)(wchar_t const *__file, int __oflag, ...) __WFS_FUNC(_wopen);
__LIBC int (__ATTR_CDECL _wsopen)(wchar_t const *__file, int __oflag, int __sflag, ...) __WFS_FUNC(_wsopen);
__LIBC errno_t (__LIBCCALL _wsopen_s)(int *__fd, wchar_t const *__file, int __oflag, int __sflag, int __pflags) __WFS_FUNC(_wsopen_s);

__LIBC int (__LIBCCALL _waccess)(wchar_t const *__file, int __type) __WFS_FUNC(_waccess);
__LIBC errno_t (__LIBCCALL _waccess_s)(wchar_t const *__file, int __type) __WFS_FUNC(_waccess);

__LIBC int (__LIBCCALL _wchmod)(wchar_t const *__file, int __mode) __WFS_FUNC(_wchmod);
__LIBC int (__LIBCCALL _wunlink)(wchar_t const *__file) __WFS_FUNC(_wunlink);
__LIBC int (__LIBCCALL _wrename)(wchar_t const *__oldname, wchar_t const *__newname) __WFS_FUNC(_wrename);
__LIBC errno_t (__LIBCCALL _wmktemp_s)(wchar_t *__templatename, size_t __sizeinwords);

__LIBC intptr_t (__LIBCCALL _wfindfirst32)(wchar_t const *__file, struct _wfinddata32_t *__finddata) __WFS_FUNC(_wfindfirst32);
__LIBC intptr_t (__LIBCCALL _wfindfirst64)(wchar_t const *__file, struct _wfinddata64_t *__finddata) __WFS_FUNC(_wfindfirst64);
__LIBC intptr_t (__LIBCCALL _wfindfirst32i64)(wchar_t const *__file, struct _wfinddata32i64_t *__finddata) __WFS_FUNC(_wfindfirst32i64);
__LIBC intptr_t (__LIBCCALL _wfindfirst64i32)(wchar_t const *__file, struct _wfinddata64i32_t *__finddata) __WFS_FUNC(_wfindfirst64i32);
__LIBC int (__LIBCCALL _wfindnext32)(intptr_t __findfd, struct _wfinddata32_t *__finddata);
__LIBC int (__LIBCCALL _wfindnext64)(intptr_t __findfd, struct _wfinddata64_t *__finddata);
__LIBC int (__LIBCCALL _wfindnext32i64)(intptr_t __findfd, struct _wfinddata32i64_t *__finddata);
__LIBC int (__LIBCCALL _wfindnext64i32)(intptr_t __findfd, struct _wfinddata64i32_t *__finddata);
#endif /* !_WIO_DEFINED */

#ifndef _WLOCALE_DEFINED
#define _WLOCALE_DEFINED 1
__LIBC wchar_t *(__LIBCCALL _wsetlocale)(int __category, wchar_t const *__locale);
__LIBC __locale_t (__LIBCCALL _wcreate_locale)(int __category, wchar_t const *__locale);
#endif /* !_WLOCALE_DEFINED */

#ifndef _WPROCESS_DEFINED
#define _WPROCESS_DEFINED 1
__LIBC intptr_t (__ATTR_CDECL _wexecl)(wchar_t const *__path, wchar_t const *__argv, ...) __WFS_FUNC(_wexecl);
__LIBC intptr_t (__ATTR_CDECL _wexecle)(wchar_t const *__path, wchar_t const *__argv, ...) __WFS_FUNC(_wexecle);
__LIBC intptr_t (__ATTR_CDECL _wexeclp)(wchar_t const *__file, wchar_t const *__argv, ...) __WFS_FUNC(_wexeclp);
__LIBC intptr_t (__ATTR_CDECL _wexeclpe)(wchar_t const *__file, wchar_t const *__argv, ...) __WFS_FUNC(_wexeclpe);
__LIBC intptr_t (__LIBCCALL _wexecv)(wchar_t const *__path, wchar_t const *const *__argv) __WFS_FUNC(_wexecv);
__LIBC intptr_t (__LIBCCALL _wexecve)(wchar_t const *__path, wchar_t const *const *__argv, wchar_t const *const *__envp) __WFS_FUNC(_wexecve);
__LIBC intptr_t (__LIBCCALL _wexecvp)(wchar_t const *__file, wchar_t const *const *__argv) __WFS_FUNC(_wexecvp);
__LIBC intptr_t (__LIBCCALL _wexecvpe)(wchar_t const *__file, wchar_t const *const *__argv, wchar_t const *const *__envp) __WFS_FUNC(_wexecvpe);
__LIBC intptr_t (__ATTR_CDECL _wspawnl)(int __mode, wchar_t const *__path, wchar_t const *__argv, ...) __WFS_FUNC(_wspawnl);
__LIBC intptr_t (__ATTR_CDECL _wspawnle)(int __mode, wchar_t const *__path, wchar_t const *__argv, ...) __WFS_FUNC(_wspawnle);
__LIBC intptr_t (__ATTR_CDECL _wspawnlp)(int __mode, wchar_t const *__file, wchar_t const *__argv, ...) __WFS_FUNC(_wspawnlp);
__LIBC intptr_t (__ATTR_CDECL _wspawnlpe)(int __mode, wchar_t const *__file, wchar_t const *__argv, ...) __WFS_FUNC(_wspawnlpe);
__LIBC intptr_t (__LIBCCALL _wspawnv)(int __mode, wchar_t const *__path, wchar_t const *const *__argv) __WFS_FUNC(_wspawnv);
__LIBC intptr_t (__LIBCCALL _wspawnve)(int __mode, wchar_t const *__path, wchar_t const *const *__argv, wchar_t const *const *__envp) __WFS_FUNC(_wspawnve);
__LIBC intptr_t (__LIBCCALL _wspawnvp)(int __mode, wchar_t const *__file, wchar_t const *const *__argv) __WFS_FUNC(_wspawnvp);
__LIBC intptr_t (__LIBCCALL _wspawnvpe)(int __mode, wchar_t const *__file, wchar_t const *const *__argv, wchar_t const *const *__envp) __WFS_FUNC(_wspawnvpe);
#endif /* !_WPROCESS_DEFINED */

#ifndef _CRT_WSYSTEM_DEFINED
#define _CRT_WSYSTEM_DEFINED 1
__LIBC int (__LIBCCALL _wsystem)(wchar_t const *__command);
#endif /* !_CRT_WSYSTEM_DEFINED */

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
#if __SIZEOF_WCHAR_T__ == 4
#   define iswascii(wc) ((__UINT32_TYPE__)(wc) <= 0x7f)
#else /* #elif __SIZEOF_WCHAR_T__ == 2 */
#   define iswascii(wc) ((__UINT16_TYPE__)(wc) <= 0x7f)
#endif
#define _iswalnum_l(wc,lc)    __kos_iswctype((lc,wc),_ISwalnum)
#define _iswalpha_l(wc,lc)    __kos_iswctype((lc,wc),_ISwalpha)
#define _iswcntrl_l(wc,lc)    __kos_iswctype((lc,wc),_ISwcntrl)
#define _iswdigit_l(wc,lc)    __kos_iswctype((lc,wc),_ISwdigit)
#define _iswlower_l(wc,lc)    __kos_iswctype((lc,wc),_ISwlower)
#define _iswgraph_l(wc,lc)    __kos_iswctype((lc,wc),_ISwgraph)
#define _iswprint_l(wc,lc)    __kos_iswctype((lc,wc),_ISwprint)
#define _iswpunct_l(wc,lc)    __kos_iswctype((lc,wc),_ISwpunct)
#define _iswspace_l(wc,lc)    __kos_iswctype((lc,wc),_ISwspace)
#define _iswupper_l(wc,lc)    __kos_iswctype((lc,wc),_ISwupper)
#define _iswxdigit_l(wc,lc)   __kos_iswctype((lc,wc),_ISwxdigit)
#define _iswblank_l(wc,lc)    __kos_iswctype((lc,wc),_ISwblank)
#endif /* !_WCTYPE_INLINE_DEFINED */

#ifndef _INO_T_DEFINED
#define _INO_T_DEFINED 1
typedef __typedef_ino_t _ino_t;
#endif /* !_INO_T_DEFINED */

#ifndef _DEV_T_DEFINED
#define _DEV_T_DEFINED 1
typedef __typedef_dev_t _dev_t;
#endif /* !_DEV_T_DEFINED */

#ifndef _OFF_T_DEFINED
#define _OFF_T_DEFINED 1
typedef __typedef_off_t _off_t;
#endif /* !_OFF_T_DEFINED */

#ifndef __ino_t_defined
#define __ino_t_defined 1
typedef __typedef_ino_t ino_t;
#endif /* !__ino_t_defined */

#ifndef __dev_t_defined
#define __dev_t_defined 1
typedef __typedef_dev_t dev_t;
#endif /* !__dev_t_defined */

#ifndef __off_t_defined
#define __off_t_defined 1
typedef __typedef_off_t off_t;
#endif /* !__off_t_defined */

#ifndef _WSTAT_DEFINED
#define _WSTAT_DEFINED 1

#if defined(__PE__) && defined(__USE_DOS)
__LIBC __WARN_NOKOSFS int (__LIBCCALL _wstat32)(wchar_t const *__name, struct _stat32 *__buf);
__LIBC __WARN_NOKOSFS int (__LIBCCALL _wstat64)(wchar_t const *__name, struct _stat64 *__buf);
__LIBC __WARN_NOKOSFS int (__LIBCCALL _wstat32i64)(wchar_t const *__name, struct _stat32i64 *__buf);
__LIBC __WARN_NOKOSFS int (__LIBCCALL _wstat64i32)(wchar_t const *__name, struct _stat64i32 *__buf);
#else /* __PE__ && __USE_DOS */
__LIBC int (__LIBCCALL _wstat32)(wchar_t const *__name, struct _stat32 *__buf) __WFS_FUNC_(wstat);
__LIBC int (__LIBCCALL _wstat64)(wchar_t const *__name, struct _stat64 *__buf) __WFS_FUNC_(wstat);
__LIBC int (__LIBCCALL _wstat32i64)(wchar_t const *__name, struct _stat32i64 *__buf) __WFS_FUNC_(wstat);
__LIBC int (__LIBCCALL _wstat64i32)(wchar_t const *__name, struct _stat64i32 *__buf) __WFS_FUNC_(wstat);
#endif /* !__PE__ || !__USE_DOS */

#endif  /* _WSTAT_DEFINED */

#endif /* __USE_DOS */

__DECL_END

#endif /* !_WCHAR_H */
