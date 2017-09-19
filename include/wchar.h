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

#ifndef __KERNEL__
#include <features.h>
#include <bits/wchar.h>
#include <hybrid/typecore.h>
#include <hybrid/limitcore.h>
#include <__malldefs.h>

#if defined(__USE_XOPEN2K8) || defined(__USE_GNU)
#   include <xlocale.h>
#endif /* __USE_XOPEN2K8 || __USE_GNU */
#if defined(__USE_XOPEN) && !defined(__USE_UNIX98)
#   include <wctype.h>
#elif defined(__USE_UNIX98) && !defined(__USE_GNU)
#   define __need_iswxxx
#   include <wctype.h>
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

#ifndef __mbstate_t_defined
#define __mbstate_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __mbstate_t mbstate_t;
__NAMESPACE_STD_END
#endif

#if defined(__USE_GNU) || defined(__USE_XOPEN2K8)
__NAMESPACE_STD_USING(mbstate_t)
#endif

#ifndef WCHAR_MIN
#define WCHAR_MIN __WCHAR_MIN__
#define WCHAR_MAX __WCHAR_MAX__
#endif

#ifndef WEOF
#define WEOF     (0xffffffffu)
#endif

__NAMESPACE_STD_BEGIN
struct tm;
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
__LIBC size_t (__LIBCCALL mbrtowc)(wchar_t *__restrict __pwc, const char *__restrict __s, size_t __n, mbstate_t *__restrict __p);
__LIBC size_t (__LIBCCALL wcrtomb)(char *__restrict __s, wchar_t __wc, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL __mbrlen)(const char *__restrict __s, size_t __n, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL mbrlen)(const char *__restrict __s, size_t __n, mbstate_t *__restrict __ps);
__LIBC size_t (__LIBCCALL mbsrtowcs)(wchar_t *__restrict __dst, const char **__restrict __src, size_t __len, mbstate_t *__restrict __ps);
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
__NAMESPACE_STD_END

__NAMESPACE_STD_USING(tm)
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
__LIBC size_t (__LIBCCALL mbsnrtowcs)(wchar_t *__restrict __dst, const char **__restrict __src, size_t __nmc, size_t __len, mbstate_t *__restrict __ps);
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

__DECL_END
#endif /* !__KERNEL__ */

#endif /* !_WCHAR_H */
