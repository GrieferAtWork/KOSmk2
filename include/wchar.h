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
#if !defined(__KERNEL__) && \
    (defined(__USE_UNIX98) || defined(__USE_XOPEN2K) || \
     defined(__USE_DOS))
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
#endif /* !__wchar_t_defined */

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
#if 0 /* TODO: For binary compatibility with dos, this must be a 32-bit integer type! */
typedef struct __mbstate { __INT32_TYPE____val; } __mbstate_t;
#else
typedef struct __mbstate {
 int            __count;
 union { wint_t __wch; char   __wchb[4]; } __value;
} __mbstate_t;
#endif
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

#ifdef __USE_KOS
#ifndef MBSTATE_INIT
#define MBSTATE_INIT     {0,{0}}
#endif /* !MBSTATE_INIT */
#endif /* __USE_KOS */


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
__LIBC size_t (__LIBCCALL __mbrlen)(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps) __ASMNAME("mbrlen");
__LIBC size_t (__LIBCCALL mbrlen)(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL mbsrtowcs)(wchar_t *__restrict __dst, char const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL wcsrtombs)(char *__restrict __dst, wchar_t const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps);
#ifndef __wcstod_defined
__LIBC double (__LIBCCALL wcstod)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
#endif /* !__wcstod_defined */
#ifndef __wcstol_defined
__LIBC long int (__LIBCCALL wcstol)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
__LIBC unsigned long int (__LIBCCALL wcstoul)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
#endif /* !__wcstol_defined */
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
#ifdef __USE_DOS
__LIBC wchar_t *(__LIBCCALL wcstok)(wchar_t *__restrict __s, wchar_t const *__restrict __delim) __PE_FUNC("wcstok");
#else /* __USE_DOS */
__LIBC wchar_t *(__LIBCCALL wcstok)(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr) __KOS_FUNC("wcstok");
#endif /* !__USE_DOS */
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
#ifndef __wcstof_defined
__LIBC float (__LIBCCALL wcstof)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
__LIBC long double (__LIBCCALL wcstold)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
#endif /* !__wcstof_defined */
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
#ifndef __wcstod_defined
#define __wcstod_defined 1
__NAMESPACE_STD_USING(wcstod)
#endif /* !__wcstod_defined */
#ifndef __wcstol_defined
#define __wcstol_defined 1
__NAMESPACE_STD_USING(wcstol)
__NAMESPACE_STD_USING(wcstoul)
#endif /* !__wcstol_defined */
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
#ifndef __wcstof_defined
#define __wcstof_defined 1
__NAMESPACE_STD_USING(wcstof)
__NAMESPACE_STD_USING(wcstold)
#endif /* !__wcstof_defined */
#ifndef __wcstoll_defined
#define __wcstoll_defined
__NAMESPACE_STD_USING(wcstoll)
__NAMESPACE_STD_USING(wcstoull)
#endif /* !__wcstoll_defined */
__NAMESPACE_STD_USING(vfwscanf)
__NAMESPACE_STD_USING(vwscanf)
__NAMESPACE_STD_USING(vswscanf)
#endif /* __USE_ISOC99 */
#endif /* !__KERNEL__ */

#ifndef __KERNEL__
#ifdef __USE_XOPEN2K8
__LIBC int (__LIBCCALL wcscasecmp)(wchar_t const *__s1, wchar_t const *__s2) __ASMNAME2("wcscasecmp","wcsicmp");
__LIBC int (__LIBCCALL wcsncasecmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n) __ASMNAME2("wcsncasecmp","wcsnicmp");
__LIBC int (__LIBCCALL wcscasecmp_l)(wchar_t const *__s1, wchar_t const *__s2, __locale_t __loc) __ASMNAME2("wcscasecmp_l","_wcsicmp_l");
__LIBC int (__LIBCCALL wcsncasecmp_l)(wchar_t const *__s1, wchar_t const *__s2, size_t __n, __locale_t __loc) __ASMNAME2("wcsncasecmp_l","_wcsnicmp_l");
__LIBC int (__LIBCCALL wcscoll_l)(wchar_t const *__s1, wchar_t const *__s2, __locale_t __loc) __ASMNAME2("wcscoll_l","_wcscoll_l");
__LIBC size_t (__LIBCCALL wcsxfrm_l)(wchar_t *__s1, wchar_t const *__s2, size_t __n, __locale_t __loc) __ASMNAME2("wcsxfrm_l","_wcsxfrm_l");
__LIBC size_t (__LIBCCALL mbsnrtowcs)(wchar_t *__restrict __dst, char const **__restrict __src, size_t __nmc, size_t __len, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL wcsnrtombs)(char *__restrict __dst, wchar_t const **__restrict __src, size_t __nwc, size_t __len, mbstate_t *__restrict __ps);
__LIBC wchar_t *(__LIBCCALL wcpcpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src);
__LIBC wchar_t *(__LIBCCALL wcpncpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __n);
__LIBC __FILE *(__LIBCCALL open_wmemstream)(wchar_t **__bufloc, size_t *__sizeloc);
#endif /* __USE_XOPEN2K8 */


#if defined(__USE_XOPEN2K8) || defined(__USE_DOS)
#ifndef __wcsnlen_defined
#define __wcsnlen_defined 1
__LIBC __ATTR_PURE size_t (__LIBCCALL wcsnlen)(wchar_t const *__s, size_t __maxlen);
#endif /* !__wcsnlen_defined */
#ifndef __wcsdup_defined
#define __wcsdup_defined 1
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC wchar_t *(__LIBCCALL wcsdup)(wchar_t const *__restrict __s);
#endif /* !__wcsdup_defined */
#endif /* __USE_XOPEN2K8 || __USE_DOS */

#ifdef __USE_XOPEN
#ifdef __USE_KOS
__LIBC __ssize_t (__LIBCCALL wcwidth)(wchar_t __c);
__LIBC __ssize_t (__LIBCCALL wcswidth)(wchar_t const *__restrict __s, size_t __n);
#else /* __USE_KOS */
__LIBC int (__LIBCCALL wcwidth)(wchar_t __c);
__LIBC int (__LIBCCALL wcswidth)(wchar_t const *__restrict __s, size_t __n);
#endif /* !__USE_KOS */
#endif /* __USE_XOPEN */

#if defined(__USE_XOPEN) || defined(__USE_DOS)
#ifndef __wcswcs_defined
#define __wcswcs_defined 1
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcswcs)(wchar_t *__haystack, wchar_t const *__needle) __ASMNAME("wcsstr");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wcswcs)(wchar_t const *__haystack, wchar_t const *__needle) __ASMNAME("wcsstr");
}
#else
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcswcs)(wchar_t const *__haystack, wchar_t const *__needle) __ASMNAME("wcsstr");
#endif
#endif /* !__wcswcs_defined */
#endif /* __USE_XOPEN || __USE_DOS */

#ifdef __USE_GNU
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcschrnul)(wchar_t const *__s, wchar_t __wc);
__LIBC wchar_t *(__LIBCCALL wmempcpy)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n);
__LIBC __LONGLONG (__LIBCCALL wcstoq)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base) __ASMNAME("wcstoll");
__LIBC __ULONGLONG (__LIBCCALL wcstouq)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base) __ASMNAME("wcstoull");
__LIBC long int (__LIBCCALL wcstol_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME2("wcstol_l","_wcstol_l");
__LIBC unsigned long int (__LIBCCALL wcstoul_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME2("wcstoul_l","_wcstoul_l");
__LIBC __LONGLONG (__LIBCCALL wcstoll_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME2("wcstoll_l","_wcstoll_l");
__LIBC __ULONGLONG (__LIBCCALL wcstoull_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME2("wcstoull_l","_wcstoull_l");
__LIBC double (__LIBCCALL wcstod_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __loc) __ASMNAME2("wcstod_l","_wcstod_l");
__LIBC float (__LIBCCALL wcstof_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __loc) __ASMNAME2("wcstof_l","_wcstof_l");
__LIBC long double (__LIBCCALL wcstold_l)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __loc) __ASMNAME2("wcstold_l","_wcstold_l");
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

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

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

#ifndef __rsize_t_defined
#define __rsize_t_defined 1
typedef size_t rsize_t;
#endif /* !__rsize_t_defined */

#ifndef __va_list_defined
#define __va_list_defined 1
__NAMESPACE_STD_BEGIN
typedef __VA_LIST va_list;
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(va_list)
#endif /* !__va_list_defined */

typedef wchar_t _Wint_t;

#ifndef _iobuf
#define _iobuf   _IO_FILE
#endif /* !_iobuf */

#ifndef __stdstreams_defined
#define __stdstreams_defined 1
#undef stdin
#undef stdout
#undef stderr
__LIBC FILE *(stdin);
__LIBC FILE *(stdout);
__LIBC FILE *(stderr);
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
__LIBC __NONNULL((1)) wchar_t *(__LIBCCALL _wgetcwd)(wchar_t *__dstbuf, int __dstlen);
__LIBC __NONNULL((1)) wchar_t *(__LIBCCALL _wgetdcwd)(int __drive, wchar_t *__dstbuf, int __dstlen);
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
/* TODO: Should also appear in <process.h> */
__LIBC int (__LIBCCALL _wsystem)(wchar_t const *__cmd);
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
#endif /* !_WSTAT_DEFINED */

#ifndef _WCONIO_DEFINED
#define _WCONIO_DEFINED 1
/* Since KOS doesn't differentiate for these, they just redirect to stdin/stdout.
 * >> If you really want to operate on a console, open your terminal slave under '/dev/'. */
__LIBC errno_t (__LIBCCALL _cgetws_s)(wchar_t *__buffer, size_t __buflen, size_t *__sizeok) __ASMNAME("_getws_s");
__LIBC wchar_t *(__LIBCCALL _cgetws)(wchar_t *__buffer) __ASMNAME("_getws");
__LIBC wint_t (__LIBCCALL _getwch)(void) __ASMNAME("getwchar");
__LIBC wint_t (__LIBCCALL _getwch_nolock)(void) __ASMNAME("getwchar_unlocked");
__LIBC wint_t (__LIBCCALL _getwche)(void) __ASMNAME("getwchar");
__LIBC wint_t (__LIBCCALL _getwche_nolock)(void) __ASMNAME("getwchar_unlocked");
__LIBC wint_t (__LIBCCALL _putwch)(wchar_t __wc) __ASMNAME("putwchar");
__LIBC wint_t (__LIBCCALL _putwch_nolock)(wchar_t __wc) __ASMNAME("putwchar_unlocked");
__LIBC wint_t (__LIBCCALL _ungetwch)(wint_t __wc);
__LIBC wint_t (__LIBCCALL _ungetwch_nolock)(wint_t __wc);
__LIBC int (__LIBCCALL _cputws)(wchar_t const *__string) __ASMNAME("_putws");
__LIBC int (__ATTR_CDECL _cwprintf)(wchar_t const *__format, ...) __ASMNAME("wprintf");
__LIBC int (__ATTR_CDECL _cwprintf_p)(wchar_t const *__format, ...) __ASMNAME("wprintf");
__LIBC int (__ATTR_CDECL _cwprintf_s)(wchar_t const *__format, ...) __ASMNAME("wprintf");
__LIBC int (__LIBCCALL _vcwprintf)(wchar_t const *__format, __VA_LIST __args) __ASMNAME("vwprintf");
__LIBC int (__LIBCCALL _vcwprintf_p)(wchar_t const*__format, __VA_LIST __args) __ASMNAME("vwprintf");
__LIBC int (__LIBCCALL _vcwprintf_s)(wchar_t const *__format, __VA_LIST __args) __ASMNAME("vwprintf");
__LIBC int (__ATTR_CDECL _cwprintf_l)(wchar_t const *__format, __locale_t __locale, ...) __ASMNAME("_wprintf_l");
__LIBC int (__ATTR_CDECL _cwprintf_p_l)(wchar_t const *__format, __locale_t __locale, ...) __ASMNAME("_wprintf_l");
__LIBC int (__ATTR_CDECL _cwprintf_s_l)(wchar_t const *__format, __locale_t __locale, ...) __ASMNAME("_wprintf_l");
__LIBC int (__LIBCCALL _vcwprintf_l)(wchar_t const *__format, __locale_t __locale, __VA_LIST __args) __ASMNAME("_vwprintf_l");
__LIBC int (__LIBCCALL _vcwprintf_p_l)(wchar_t const *__format, __locale_t __locale, __VA_LIST __args) __ASMNAME("_vwprintf_l");
__LIBC int (__LIBCCALL _vcwprintf_s_l)(wchar_t const *__format, __locale_t __locale, __VA_LIST __args) __ASMNAME("_vwprintf_l");
__LIBC int (__ATTR_CDECL _cwscanf)(wchar_t const *__format, ...) __ASMNAME("wscanf");
__LIBC int (__ATTR_CDECL _cwscanf_s)(wchar_t const *__format, ...) __ASMNAME("wscanf");
__LIBC int (__ATTR_CDECL _cwscanf_l)(wchar_t const *__format, __locale_t __locale, ...) __ASMNAME("_wscanf_l");
__LIBC int (__ATTR_CDECL _cwscanf_s_l)(wchar_t const *__format, __locale_t __locale, ...) __ASMNAME("_wscanf_l");
#endif /* !_WCONIO_DEFINED */

#ifndef _WSTDIO_DEFINED
#define _WSTDIO_DEFINED 1
__LIBC FILE *(__LIBCCALL _wfdopen)(int __fd, wchar_t const *__restrict __mode);
__LIBC FILE *(__LIBCCALL _wfopen)(wchar_t const *__file, wchar_t const *__mode);
__LIBC FILE *(__LIBCCALL _wfreopen)(wchar_t const *__file, wchar_t const *__mode, FILE *__fp);
__LIBC FILE *(__LIBCCALL _wfsopen)(wchar_t const *__file, wchar_t const *__mode, int __sflag);
__LIBC FILE *(__LIBCCALL _wpopen)(wchar_t const *__cmd, wchar_t const *__mode);
__LIBC errno_t (__LIBCCALL _wfopen_s)(FILE **__restrict __pfp, wchar_t const *__file, wchar_t const *__mode);
__LIBC errno_t (__LIBCCALL _wfreopen_s)(FILE **__restrict __pfp, wchar_t const *__file, wchar_t const *__mode, FILE *__restrict __fp);

__LIBC wint_t (__LIBCCALL getwchar)(void);
__LIBC wint_t (__LIBCCALL putwchar)(wchar_t __wc);
__LIBC wint_t (__LIBCCALL _fgetwchar)(void) __ASMNAME("getwchar");
__LIBC wint_t (__LIBCCALL _fputwchar)(wchar_t __wc) __ASMNAME("putwchar");

__LIBC wint_t (__LIBCCALL fgetwc)(FILE *__restrict __fp);
__LIBC wint_t (__LIBCCALL fputwc)(wchar_t __wc, FILE *__restrict __fp);
__LIBC wint_t (__LIBCCALL getwc)(FILE *__restrict __fp) __ASMNAME("fgetwc");
__LIBC wint_t (__LIBCCALL putwc)(wchar_t __wc, FILE *__restrict __fp) __ASMNAME("fputwc");
__LIBC wint_t (__LIBCCALL ungetwc)(wint_t __wc, FILE *__restrict __fp);
__LIBC wchar_t *(__LIBCCALL _getws_s)(wchar_t *__restrict __s, size_t __dstlen);
__LIBC wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, int __dstlen, FILE *__restrict __fp);
__LIBC int (__LIBCCALL _putws)(wchar_t const *__restrict __s);
__LIBC int (__LIBCCALL fputws)(wchar_t const *__restrict __s, FILE *__restrict __fp);
__LIBC errno_t (__LIBCCALL _wtmpnam_s)(wchar_t *__restrict __buf, size_t __buflen);
__LIBC wchar_t *(__LIBCCALL _wtempnam)(wchar_t const *__restrict __dir, wchar_t const *__restrict __prefix);
__LIBC wchar_t *(__LIBCCALL _wtmpnam)(wchar_t *__restrict __buf);
__LIBC wint_t (__LIBCCALL _fgetwc_nolock)(FILE *__restrict __fp) __ASMNAME("fgetwc_unlocked");
__LIBC wint_t (__LIBCCALL _fputwc_nolock)(wchar_t __wc, FILE *__restrict __fp) __ASMNAME("fputwc_unlocked");
__LIBC wint_t (__LIBCCALL _ungetwc_nolock)(wint_t __wc, FILE *__restrict __fp);

__LIBC int (__ATTR_CDECL _fwprintf_l)(FILE *__restrict __fp, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _fwprintf_p)(FILE *__restrict __fp, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _fwprintf_p_l)(FILE *__restrict __fp, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _fwprintf_s_l)(FILE *__restrict __fp, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _fwscanf_l)(FILE *__restrict __fp, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _fwscanf_s_l)(FILE *__restrict __fp, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _scwprintf)(wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _scwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _scwprintf_p)(wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _scwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _snwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _snwprintf_l)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _snwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _snwprintf_s_l)(wchar_t *__restrict __buf, size_t __dstsize, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _snwscanf)(wchar_t const *__restrict __src, size_t __maxlen, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _snwscanf_l)(wchar_t const *__restrict __src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _snwscanf_s)(wchar_t const *__restrict __src, size_t __maxlen, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _snwscanf_s_l)(wchar_t const *__restrict __src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _swprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _swprintf_c_l)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _swprintf_p)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _swprintf_p_l)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _swprintf_s_l)(wchar_t *__restrict __buf, size_t __dstsize, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _swscanf_l)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _swscanf_s_l)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _wprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _wprintf_p)(wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL _wprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _wprintf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _wscanf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL _wscanf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__ATTR_CDECL fwprintf)(FILE *__restrict __fp, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL fwscanf)(FILE *__restrict __fp, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __count, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL swscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL _swprintf_l)(wchar_t *__restrict __buf, size_t __count, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC int (__LIBCCALL _vfwprintf_l)(FILE *__restrict __fp, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vfwprintf_p)(FILE *__restrict __fp, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL _vfwprintf_p_l)(FILE *__restrict __fp, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vfwprintf_s_l)(FILE *__restrict __fp, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vscwprintf)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL _vscwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vscwprintf_p)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL _vscwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vsnwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST _Args);
__LIBC int (__LIBCCALL _vsnwprintf_l)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vsnwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL _vsnwprintf_s_l)(wchar_t *__restrict __buf, size_t __dstsize, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vswprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL _vswprintf_c_l)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vswprintf_l)(wchar_t *__restrict __buf, size_t __count, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vswprintf_p)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL _vswprintf_p_l)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vswprintf_s_l)(wchar_t *__restrict __buf, size_t __dstsize, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vwprintf_p)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL _vwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _vwprintf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__LIBCCALL _wremove)(wchar_t const *__restrict __file);
__LIBC int (__LIBCCALL vfwprintf)(FILE *__restrict __fp, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vfwscanf)(FILE *__restrict __fp, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vswprintf)(wchar_t *__restrict __buf, size_t __count, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vswscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __VA_LIST __args);

#define getwchar()           fgetwc(stdin)
#define putwchar(c)          fputwc(c,stdout)
#define getwc(fp)            fgetwc(fp)
#define putwc(c,fp)          fputwc(c,fp)
#define _putwc_nolock(c,fp) _fputwc_nolock(c,fp)
#define _getwc_nolock(c)    _fgetwc_nolock(c)

#ifdef __USE_DOS_SLIB
__LIBC int (__ATTR_CDECL wprintf_s)(wchar_t const *__restrict __restrict __format, ...);
__LIBC int (__ATTR_CDECL swprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL fwprintf_s)(FILE *__restrict __fp, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL wscanf_s)(wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL swscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL fwscanf_s)(FILE *__restrict __fp, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vwprintf_s)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vswprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vfwprintf_s)(FILE *__restrict __fp, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vwscanf_s)(wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vswscanf_s)(wchar_t const *__restrict __dst, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__LIBCCALL vfwscanf_s)(FILE *__restrict __fp, wchar_t const *__restrict __format, __VA_LIST __args);
#endif /* __USE_DOS_SLIB */
#endif /* !_WSTDIO_DEFINED */

#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED 1
/* TODO: Should also appear in <stdio.h> */
__LIBC void (__LIBCCALL _wperror)(wchar_t const *__restrict __errmsg);
#endif /* !_CRT_WPERROR_DEFINED */

#ifndef _WSTDLIB_DEFINED
#define _WSTDLIB_DEFINED 1
__LIBC wchar_t *(__LIBCCALL _itow)(int __val, wchar_t *__restrict __dst, int __radix);
__LIBC wchar_t *(__LIBCCALL _ultow)(unsigned long __val,  wchar_t *__restrict __dst, int __radix);
__LIBC wchar_t *(__LIBCCALL _i64tow)(__INT64_TYPE__ __val, wchar_t *__restrict __dst, int __radix);
__LIBC wchar_t *(__LIBCCALL _ui64tow)(__UINT64_TYPE__ __val, wchar_t *__restrict __dst, int __radix);
__LIBC errno_t (__LIBCCALL _itow_s)(int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix);
__LIBC errno_t (__LIBCCALL _ultow_s)(unsigned long __val, wchar_t *__dst, size_t __maxlen, int __radix);
__LIBC errno_t (__LIBCCALL _i64tow_s)(__INT64_TYPE__ __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix);
__LIBC errno_t (__LIBCCALL _ui64tow_s)(__UINT64_TYPE__ __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix);
__LIBC double (__LIBCCALL _wtof)(wchar_t const *__restrict __s);
__LIBC double (__LIBCCALL _wtof_l)(wchar_t const *__restrict __s, __locale_t __locale);
__LIBC int (__LIBCCALL _wtoi)(wchar_t const *__restrict __s);
__LIBC int (__LIBCCALL _wtoi_l)(wchar_t const *__restrict __s, __locale_t __locale);
__LIBC __LONGLONG (__LIBCCALL _wtoll)(wchar_t const *__restrict __s);
__LIBC __LONGLONG (__LIBCCALL _wtoll_l)(wchar_t const *__restrict __s, __locale_t __locale);
#if __SIZEOF_INT__ == __SIZEOF_LONG__
__LIBC long int (__LIBCCALL _wtol)(wchar_t const *__restrict __s) __ASMNAME("_wtoi");
__LIBC long int (__LIBCCALL _wtol_l)(wchar_t const *__restrict __s, __locale_t __locale) __ASMNAME("_wtoi_l");
__LIBC wchar_t *(__LIBCCALL _ltow)(long __val, wchar_t *__restrict __dst, int __radix) __ASMNAME("_itow");
__LIBC errno_t (__LIBCCALL _ltow_s)(long __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix) __ASMNAME("_itow_s");
#else /* __SIZEOF_INT__ == __SIZEOF_LONG__ */
__LIBC long int (__LIBCCALL _wtol)(wchar_t const *__restrict __s);
__LIBC long int (__LIBCCALL _wtol_l)(wchar_t const *__restrict __s, __locale_t __locale);
__LIBC wchar_t *(__LIBCCALL _ltow)(long __val, wchar_t *__restrict __dst, int __radix);
__LIBC errno_t (__LIBCCALL _ltow_s)(long __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix);
#endif /* __SIZEOF_INT__ != __SIZEOF_LONG__ */

__LIBC long int (__LIBCCALL _wcstol_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME2("wcstol_l","_wcstol_l");
__LIBC unsigned long int (__LIBCCALL _wcstoul_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME2("wcstoul_l","_wcstoul_l");
__LIBC __LONGLONG (__LIBCCALL _wcstoll_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME2("wcstoll_l","_wcstoll_l");
__LIBC __ULONGLONG (__LIBCCALL _wcstoull_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME2("wcstoull_l","_wcstoull_l");
__LIBC float (__LIBCCALL _wcstof_l)(wchar_t const *__restrict __s, wchar_t **__restrict __pend, __locale_t __locale) __ASMNAME2("wcstof_l","_wcstof_l");
__LIBC double (__LIBCCALL _wcstod_l)(wchar_t const *__restrict __s, wchar_t **__pend, __locale_t __locale) __ASMNAME2("wcstod_l","_wcstod_l");
__LIBC long double (__LIBCCALL _wcstold_l)(wchar_t const *__restrict __s, wchar_t **__pend, __locale_t __locale) __ASMNAME2("wcstold_l","_wcstold_l");

__LIBC wchar_t *(__LIBCCALL _wgetenv)(wchar_t const *__restrict __varname);
__LIBC errno_t (__LIBCCALL _wgetenv_s)(size_t *__restrict __psize, wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __varname);
__LIBC errno_t (__LIBCCALL _wdupenv_s)(wchar_t **__restrict __pbuf, size_t *__restrict __pbuflen, wchar_t const *__restrict __varname);

#if __SIZEOF_LONG_LONG__ == 8
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) __ASMNAME("wcstoll");
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME2("wcstoll_l","_wcstoll_l");
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) __ASMNAME("wcstoull");
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME2("wcstoull_l","_wcstoull_l");
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) __ASMNAME("_wtoll");
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) __ASMNAME("_wtoll_l");
#elif __SIZEOF_LONG__ == 8
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) __ASMNAME("wcstol");
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME2("wcstol_l","_wcstol_l");
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) __ASMNAME("wcstoul");
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME2("wcstoul_l","_wcstoul_l");
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) __ASMNAME("_wtol");
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) __ASMNAME("_wtol_l");
#else
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale);
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale);
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s);
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale);
#endif

#if 1 /* TODO: Should only appear in <stdlib.h> */
#ifndef _CRT_WSYSTEM_DEFINED
#define _CRT_WSYSTEM_DEFINED 1
__LIBC int (__LIBCCALL _wsystem)(wchar_t const *__restrict __cmd);
#endif /* !_CRT_WSYSTEM_DEFINED */
#ifndef __wcstol_defined
#define __wcstol_defined 1
__NAMESPACE_STD_BEGIN
__LIBC long int (__LIBCCALL wcstol)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC unsigned long int (__LIBCCALL wcstoul)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcstol)
__NAMESPACE_STD_USING(wcstoul)
#endif /* !__wcstol_defined */

#ifndef __wcstoll_defined
#define __wcstoll_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __LONGLONG (__LIBCCALL wcstoll)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC __ULONGLONG (__LIBCCALL wcstoull)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcstoll)
__NAMESPACE_STD_USING(wcstoull)
#endif /* !__wcstoll_defined */

#ifndef __wcstod_defined
#define __wcstod_defined 1
__NAMESPACE_STD_BEGIN
__LIBC double (__LIBCCALL wcstod)(wchar_t const *__restrict __s, wchar_t **__pend);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcstod)
#endif /* !__wcstod_defined */

#ifndef __wcstof_defined
#define __wcstof_defined 1
__NAMESPACE_STD_BEGIN
__LIBC float (__LIBCCALL wcstof)(wchar_t const *__restrict __s, wchar_t **__pend);
__LIBC long double (__LIBCCALL wcstold)(wchar_t const *__restrict __s, wchar_t **__pend);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcstof)
__NAMESPACE_STD_USING(wcstold)
#endif /* !__wcstof_defined */
#endif /* TODO: END */
#endif /* !_WSTDLIB_DEFINED */


#ifndef _WSTDLIBP_DEFINED
#define _WSTDLIBP_DEFINED 1
__LIBC wchar_t *(__LIBCCALL _wfullpath)(wchar_t *__restrict __abspath, wchar_t const *__restrict __path, size_t __maxlen);
__LIBC int (__LIBCCALL _wputenv)(wchar_t const *__restrict __envstr);
__LIBC void (__LIBCCALL _wmakepath)(wchar_t *__restrict __dst, wchar_t const *__restrict __drive, wchar_t const *__restrict __dir, wchar_t const *__restrict __file, wchar_t const *__restrict __ext);
__LIBC void (__LIBCCALL _wsearchenv)(wchar_t const *__restrict __file, wchar_t const *__restrict __varname,  wchar_t *__restrict __dst);
__LIBC void (__LIBCCALL _wsplitpath)(wchar_t const *__restrict __abspath, wchar_t *__restrict __drive, wchar_t *__restrict __dir, wchar_t *__restrict __file, wchar_t *__restrict __ext);
__LIBC errno_t (__LIBCCALL _wmakepath_s)(wchar_t *__restrict __dst, size_t __maxlen, wchar_t const *__restrict __drive, wchar_t const *__restrict __dir, wchar_t const *__restrict __file, wchar_t const *__restrict __ext);
__LIBC errno_t (__LIBCCALL _wputenv_s)(wchar_t const *__restrict __name, wchar_t const *__restrict __val);
__LIBC errno_t (__LIBCCALL _wsearchenv_s)(wchar_t const *__restrict __file, wchar_t const *__restrict __varname, wchar_t * __restrict __dst, size_t __maxlen);
__LIBC errno_t (__LIBCCALL _wsplitpath_s)(wchar_t const *__restrict __abspath, wchar_t *__restrict __drive, size_t __drivelen, wchar_t *__restrict __dir, size_t __dirlen, wchar_t *__restrict __file, size_t __filelen, wchar_t *__restrict __ext, size_t __extlen);
#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED 1
/* TODO: Delete me. - Should only appear in <stdlib.h> */
__LIBC void (__LIBCCALL _wperror)(wchar_t const *__restrict __errmsg);
#endif /* !_CRT_WPERROR_DEFINED */
#endif /* !_WSTDLIBP_DEFINED */

#ifndef _WSTRING_DEFINED
#define _WSTRING_DEFINED 1
#ifndef ___wcsdup_defined
#define ___wcsdup_defined 1
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC wchar_t *(__LIBCCALL _wcsdup)(wchar_t const *__restrict __str) __ASMNAME("wcsdup");
#endif /* !___wcsdup_defined */
#ifndef __wcstok_s_defined
#define __wcstok_s_defined 1
__LIBC wchar_t *(__LIBCCALL wcstok_s)(wchar_t *__restrict __str, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr) __KOS_FUNC_("wcstok");
#endif /* !__wcstok_s_defined */
#ifndef __wcserror_defined
#define __wcserror_defined 1
__LIBC wchar_t *(__LIBCCALL _wcserror)(int __errnum);
__LIBC wchar_t *(__LIBCCALL __wcserror)(wchar_t const *__restrict __str);
__LIBC errno_t (__LIBCCALL _wcserror_s)(wchar_t *__restrict __buf, size_t __maxlen, int __errnum);
__LIBC errno_t (__LIBCCALL __wcserror_s)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __errmsg);
#endif /* !__wcserror_defined */

#ifndef ___wcsset_s_defined
#define ___wcsset_s_defined 1
__LIBC errno_t (__LIBCCALL _wcsset_s)(wchar_t *__restrict __str, size_t __maxlen, wchar_t __val);
__LIBC errno_t (__LIBCCALL _wcsnset_s)(wchar_t *__restrict __str, size_t __buflen, wchar_t __val, size_t __maxlen);
__LIBC errno_t (__LIBCCALL _wcslwr_s)(wchar_t *__restrict __str, size_t __maxlen);
__LIBC errno_t (__LIBCCALL _wcsupr_s)(wchar_t *__restrict __str, size_t __maxlen);
__LIBC errno_t (__LIBCCALL _wcslwr_s_l)(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale);
__LIBC errno_t (__LIBCCALL _wcsupr_s_l)(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale);
#endif /* !___wcsset_s_defined */

#ifndef ___wcsicmp_defined
#define ___wcsicmp_defined 1
__LIBC int (__LIBCCALL wcsicmp)(wchar_t const *__str1, wchar_t const *__str2) __ASMNAME2("wcscasecmp","wcsicmp");
__LIBC int (__LIBCCALL _wcsicmp)(wchar_t const *__str1, wchar_t const *__str2) __ASMNAME2("wcscasecmp","wcsicmp");
__LIBC int (__LIBCCALL wcsnicmp)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars) __ASMNAME2("wcsncasecmp","wcsnicmp");
__LIBC int (__LIBCCALL _wcsnicmp)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars) __ASMNAME2("wcsncasecmp","wcsnicmp");
__LIBC int (__LIBCCALL _wcsicmp_l)(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale) __ASMNAME2("wcscasecmp_l","_wcsicmp_l");
__LIBC int (__LIBCCALL _wcsnicmp_l)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale) __ASMNAME2("wcsncasecmp_l","_wcsnicmp_l");
#endif /* !___wcsicmp_defined */

#ifndef __wcsrev_defined
#define __wcsrev_defined 1
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsrev)(wchar_t *__restrict __str);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL _wcsrev)(wchar_t *__restrict __str) __ASMNAME("wcsrev");
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsset)(wchar_t *__restrict __str, wchar_t __needle);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnset)(wchar_t *__restrict __str, wchar_t __needle, size_t __max_chars);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL _wcsset)(wchar_t *__restrict __str, wchar_t __char) __ASMNAME("wcsset");
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL _wcsnset)(wchar_t *__restrict __str, wchar_t __char, size_t __max_chars) __ASMNAME("wcsnset");
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcslwr)(wchar_t *__restrict __str);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL _wcslwr)(wchar_t *__restrict __str) __ASMNAME("wcslwr");
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL _wcslwr_l)(wchar_t *__restrict __str, __locale_t __locale);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsupr)(wchar_t *__restrict __str);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL _wcsupr)(wchar_t *__restrict __str) __ASMNAME("wcsupr");
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL _wcsupr_l)(wchar_t *__restrict __str, __locale_t __locale);
#endif /* !__wcsrev_defined */

#ifndef __wcsicoll_defined
#define __wcsicoll_defined 1
__LIBC int (__LIBCCALL _wcscoll_l)(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale) __ASMNAME2("wcscoll_l","_wcscoll_l");
__LIBC int (__LIBCCALL wcsicoll)(wchar_t const *__str1, wchar_t const *__str2);
__LIBC int (__LIBCCALL _wcsicoll)(wchar_t const *__str1, wchar_t const *__str2) __ASMNAME("wcsicoll");
__LIBC int (__LIBCCALL _wcsicoll_l)(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale);
__LIBC int (__LIBCCALL _wcsncoll)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars);
__LIBC int (__LIBCCALL _wcsncoll_l)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale);
__LIBC int (__LIBCCALL _wcsnicoll)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars);
__LIBC int (__LIBCCALL _wcsnicoll_l)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale);
#endif /* !__wcsicoll_defined */

#ifndef ___wcsxfrm_l_defined
#define ___wcsxfrm_l_defined 1
__LIBC size_t (__LIBCCALL _wcsxfrm_l)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __max_chars, __locale_t __locale) __ASMNAME("wcsxfrm_l");
#endif /* !___wcsxfrm_l_defined */

#ifdef __USE_DOS_SLIB
#ifndef __wcsnlen_s_defined
#define __wcsnlen_s_defined 1
__LIBC errno_t (__LIBCCALL wcscat_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src);
__LIBC errno_t (__LIBCCALL wcscpy_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src);
__LIBC errno_t (__LIBCCALL wcsncat_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __maxlen);
__LIBC errno_t (__LIBCCALL wcsncpy_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __maxlen);
__LOCAL size_t (__LIBCCALL wcsnlen_s)(wchar_t const *__restrict __src, size_t __maxlen) { return__src ? wcsnlen)(__src,__maxlen) : 0; }
#endif /* !__wcsnlen_s_defined */
#endif /* __USE_DOS_SLIB */
#endif /* !_WSTRING_DEFINED */

#ifndef __std_tm_defined
#define __std_tm_defined 1
__NAMESPACE_STD_BEGIN
struct tm {
 int tm_sec;   /*< seconds [0,61]. */
 int tm_min;   /*< minutes [0,59]. */
 int tm_hour;  /*< hour [0,23]. */
 int tm_mday;  /*< day of month [1,31]. */
 int tm_mon;   /*< month of year [0,11]. */
 int tm_year;  /*< years since 1900. */
 int tm_wday;  /*< day of week [0,6] )(Sunday = 0). */
 int tm_yday;  /*< day of year [0,365]. */
 int tm_isdst; /*< daylight savings flag. */
};
__NAMESPACE_STD_END
#endif /* !__std_tm_defined */

#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */

#ifndef _WTIME_DEFINED
#define _WTIME_DEFINED 1
__LIBC errno_t (__LIBCCALL _wasctime_s)(wchar_t __buf[26], size_t __maxlen, struct tm const *__restrict __ptm);
__LIBC errno_t (__LIBCCALL _wstrdate_s)(wchar_t __buf[9], size_t __maxlen);
__LIBC errno_t (__LIBCCALL _wstrtime_s)(wchar_t __buf[9], size_t __maxlen);
__LIBC size_t (__LIBCCALL _wcsftime_l)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, struct tm const *__restrict __ptm, __locale_t __locale);
__LIBC size_t (__LIBCCALL wcsftime)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format,  struct tm const *__restrict __ptm);
__LIBC wchar_t *(__LIBCCALL _wasctime)(struct tm const *__restrict __ptm);
__LIBC wchar_t *(__LIBCCALL _wstrdate)(wchar_t *__restrict __buf);
__LIBC wchar_t *(__LIBCCALL _wstrtime)(wchar_t *__restrict __buf);
__LIBC wchar_t *(__LIBCCALL _wctime32)(__time32_t const *__restrict __timer);
__LIBC wchar_t *(__LIBCCALL _wctime64)(__time64_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _wctime32_s)(wchar_t __buf[26], size_t __maxlen, __time32_t const *__timer);
__LIBC errno_t (__LIBCCALL _wctime64_s)(wchar_t __buf[26], size_t __maxlen, __time64_t const *__timer);

#ifdef __NO_ASMNAME
#ifdef __USE_TIME_BITS64
__LOCAL wchar_t *(__LIBCCALL _wctime)(const time_t *__restrict __timer) { return _wctime64(__timer); }
__LOCAL errno_t (__LIBCCALL _wctime_s)(wchar_t *__restrict __buf, size_t __maxlen, const time_t *__restrict __timer) { return _wctime64_s(__buf,__maxlen,__timer); }
#else /* __USE_TIME_BITS64 */
__LOCAL wchar_t *(__LIBCCALL _wctime)(const time_t *__restrict __timer) { return _wctime32(__timer); }
__LOCAL errno_t (__LIBCCALL _wctime_s)(wchar_t *__restrict __buf, size_t __maxlen, const time_t *__restrict __timer) { return _wctime32_s(__buf, __maxlen, __timer); }
#endif /* !__USE_TIME_BITS64 */
#else /* __NO_ASMNAME */
#ifdef __USE_TIME_BITS64
__LIBC wchar_t *(__LIBCCALL _wctime)(const time_t *__restrict __timer) __ASMNAME)("_wctime64");
__LIBC errno_t (__LIBCCALL _wctime_s)(wchar_t *__restrict __buf, size_t __maxlen, const time_t *__restrict __timer) __ASMNAME)("_wctime64_s");
#else /* __USE_TIME_BITS64 */
__LIBC wchar_t *(__LIBCCALL _wctime)(const time_t *__restrict __timer) __ASMNAME)("_wctime32");
__LIBC errno_t (__LIBCCALL _wctime_s)(wchar_t *__restrict __buf, size_t __maxlen, const time_t *__restrict __timer) __ASMNAME)("_wctime32_s");
#endif /* !__USE_TIME_BITS64 */
#endif /* !__NO_ASMNAME */
#endif  /* _WTIME_DEFINED */

__LIBC errno_t (__LIBCCALL mbsrtowcs_s)(size_t *__result, wchar_t *__restrict __buf, size_t __buflen, char const **__restrict __psrc, size_t __srcsize, mbstate_t *__restrict __ps);
__LIBC errno_t (__LIBCCALL wcsrtombs_s)(size_t *__result, char *__restrict __buf, size_t __buflen, wchar_t const **__restrict __psrc, size_t __srcsize, mbstate_t *__restrict __ps);
__LIBC errno_t (__LIBCCALL wcrtomb_s)(size_t *__result, char *__restrict __buf, size_t __buflen, wchar_t __wc, mbstate_t *__restrict __ps);

#ifndef __memcpy_defined
#define __memcpy_defined 1
#ifndef __std_memcpy_defined
#define __std_memcpy_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes);
__NAMESPACE_STD_END
#endif /* !__std_memcpy_defined */
__NAMESPACE_STD_USING(memcpy)
#endif /* !__memcpy_defined */

#ifndef __memmove_defined
#define __memmove_defined 1
#ifndef __std_memcpy_defined
#define __std_memcpy_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memmove)(void *__dst, void const *__src, size_t __n_bytes);
__NAMESPACE_STD_END
#endif /* !__std_memcpy_defined */
__NAMESPACE_STD_USING(memmove)
#endif /* !__memmove_defined */

#ifdef __USE_DOS_SLIB
__LIBC errno_t (__LIBCCALL memcpy_s)(void *__restrict __dst, rsize_t __dstsize, void const *__restrict __src, rsize_t __srcsize);
__LIBC errno_t (__LIBCCALL memmove_s)(void *__dst, rsize_t __dstsize, void const *__src, rsize_t __srcsize);
__LIBC errno_t (__LIBCCALL wmemcpy_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __srcsize);
__LIBC errno_t (__LIBCCALL wmemmove_s)(wchar_t *__dst, rsize_t __dstsize, wchar_t const *__src, rsize_t __srcsize);
#endif /* __USE_DOS_SLIB */
#endif /* __USE_DOS */

__DECL_END

#endif /* !_WCHAR_H */
