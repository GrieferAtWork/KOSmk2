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
#include <bits/mbstate.h>
#include <hybrid/typecore.h>
#include <hybrid/limitcore.h>
#include <bits/types.h>
#include <__malldefs.h>
#ifdef __USE_DOS
#include <bits/stat.h>
#include <bits/io-file.h>
#endif /* __USE_DOS */

#if defined(__USE_XOPEN2K8) || defined(__USE_GNU)
#   include <xlocale.h>
#endif /* __USE_XOPEN2K8 || __USE_GNU */
#if defined(__USE_XOPEN) && !defined(__USE_UNIX98)
#   include <wctype.h>
#elif (defined(__USE_UNIX98) && !defined(__USE_GNU)) || \
       defined(__USE_DOS)
#   include <bits/wctype.h>
#endif

__SYSDECL_BEGIN

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
#define NULL __NULLPTR
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
__LIBC __WUNUSED wint_t (__LIBCCALL btowc)(int __c);
__LIBC __WUNUSED int (__LIBCCALL wctob)(wint_t __c);
#ifdef __DOS_COMPAT__
__LOCAL __ATTR_PURE int (__LIBCCALL mbsinit)(mbstate_t const *__ps) { return !__ps || !__ps->__val; }
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_PURE int (__LIBCCALL mbsinit)(mbstate_t const *__ps);
#endif /* !__DOS_COMPAT__ */
__LIBC size_t (__LIBCCALL mbrtowc)(wchar_t *__restrict __pwc, char const *__restrict __s, size_t __n, mbstate_t *__restrict __p);
__LIBC size_t (__LIBCCALL wcrtomb)(char *__restrict __s, wchar_t __wc, mbstate_t *__restrict __ps);
__REDIRECT(__LIBC,__WUNUSED,size_t,__LIBCCALL,__mbrlen,(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps),mbrlen,(__s,__n,__ps))
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
#ifndef __getwchar_defined
__LIBC wint_t (__LIBCCALL getwchar)(void);
__LIBC wint_t (__LIBCCALL fgetwc)(__FILE *__stream);
__LIBC wint_t (__LIBCCALL getwc)(__FILE *__stream);
#endif /* !__getwchar_defined */
#ifndef __putwchar_defined
__LIBC wint_t (__LIBCCALL putwchar)(wchar_t __wc);
__LIBC wint_t (__LIBCCALL fputwc)(wchar_t __wc, __FILE *__stream);
__LIBC wint_t (__LIBCCALL putwc)(wchar_t __wc, __FILE *__stream);
#endif /* !__putwchar_defined */
#ifndef __fgetws_defined
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#if defined(__PE__) || !defined(__CRT_KOS) /* In PE-mode, we don't export the size_t version */
__REDIRECT(__LIBC,,wchar_t *,__LIBCCALL,__fgetws_int,(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream),fgetws,(__buf,__n,__stream))
__LOCAL wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) { return __fgetws_int(__ws,(int)__n,__stream); }
#else /* ... */
__REDIRECT(__LIBC,,wchar_t *,__LIBCCALL,fgetws,(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream),fgetws_sz,(__buf,__n,__stream))
#endif /* !... */
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__LIBC wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream);
#endif /* !__USE_KOS */
#endif /* !__fgetws_defined */
#ifndef __fputws_defined
__LIBC int (__LIBCCALL fputws)(wchar_t const *__restrict __str, __FILE *__restrict __stream);
#endif /* !__fputws_defined */
#ifndef __ungetwc_defined
__LIBC wint_t (__LIBCCALL ungetwc)(wint_t __wc, __FILE *__stream);
#endif /* !__ungetwc_defined */
#ifndef __wcsftime_defined
__LIBC size_t (__LIBCCALL wcsftime)(wchar_t *__restrict __s, size_t __maxsize, wchar_t const *__restrict __format, struct tm const *__restrict __tp);
#endif /* !__wcsftime_defined */
#ifndef __wcstok_defined
#if defined(__USE_DOS) && !defined(__USE_ISOC95)
/* Define wcstok() incorrect the same way DOS does. */
#ifdef __CRT_DOS
__REDIRECT_IFKOS(__LIBC,,wchar_t *,__LIBCCALL,wcstok,(wchar_t *__restrict __s, wchar_t const *__restrict __delim),__wcstok_f,(__s,__delim))
#else /* __CRT_DOS */
__REDIRECT(__LIBC,,wchar_t *,__LIBCCALL,__wcstok_impl,(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr),wcstok,(__s,__delim,__ptr))
__INTERN __ATTR_WEAK __ATTR_UNUSED wchar_t *__wcstok_safe = 0;
__LOCAL wchar_t *(__LIBCCALL wcstok)(wchar_t *__restrict __s, wchar_t const *__restrict __delim) { return __wcstok_impl(__s,__delim,&__wcstok_safe); }
#endif /* !__CRT_DOS */
#elif defined(__CRT_DOS) && __SIZEOF_WCHAR_T__ == 2
__REDIRECT(__LIBC,,wchar_t *,__LIBCCALL,wcstok,(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr),wcstok_s,(__s,__delim,__ptr))
#else
__LIBC wchar_t *(__LIBCCALL wcstok)(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr);
#endif
#endif /* !__wcstok_defined */
#ifndef __wcslen_defined
__LIBC size_t (__LIBCCALL wcslen)(wchar_t const *__s);
#endif /* !__wcslen_defined */
#ifndef __wcsspn_defined
__LIBC __ATTR_PURE size_t (__LIBCCALL wcsspn)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __ATTR_PURE size_t (__LIBCCALL wcscspn)(wchar_t const *__haystack, wchar_t const *__reject);
#endif /* !__wcsspn_defined */
#ifdef __DOS_COMPAT__
__NAMESPACE_STD_END __DECL_END
#include <hybrid/string.h>
__DECL_BEGIN __NAMESPACE_STD_BEGIN
__LOCAL __ATTR_PURE int (__LIBCCALL wmemcmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n) { return __hybrid_memcmp(__s1,__s2,__n*sizeof(wchar_t)); }
__LOCAL wchar_t *(__LIBCCALL wmemcpy)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n) { return (wchar_t *)__hybrid_memcpy(__s1,__s2,__n*sizeof(wchar_t)); }
__LOCAL wchar_t *(__LIBCCALL wmemmove)(wchar_t *__s1, wchar_t const *__s2, size_t __n) { return (wchar_t *)__hybrid_memmove(__s1,__s2,__n*sizeof(wchar_t)); }
#if __SIZEOF_WCHAR_T__ == 4
__LOCAL wchar_t *(__LIBCCALL wmemset)(wchar_t *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memsetl(__s,(__UINT32_TYPE__)__c,__n); }
#elif __SIZEOF_WCHAR_T__ == 2
__LOCAL wchar_t *(__LIBCCALL wmemset)(wchar_t *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memsetw(__s,(__UINT16_TYPE__)__c,__n); }
#else
__LOCAL wchar_t *(__LIBCCALL wmemset)(wchar_t *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memset(__s,(int)__c,__n*sizeof(wchar_t)); }
#endif
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_PURE int (__LIBCCALL wmemcmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n);
__LIBC wchar_t *(__LIBCCALL wmemcpy)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n);
__LIBC wchar_t *(__LIBCCALL wmemmove)(wchar_t *__s1, wchar_t const *__s2, size_t __n);
__LIBC wchar_t *(__LIBCCALL wmemset)(wchar_t *__s, wchar_t __c, size_t __n);
#endif /* !__DOS_COMPAT__ */
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
#ifdef __DOS_COMPAT__
#if __SIZEOF_WCHAR_T__ == 4
__LOCAL __ATTR_PURE wchar_t *(__LIBCCALL wmemchr)(wchar_t *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memchrl(__s,(__UINT32_TYPE__)__c,__n); }
__LOCAL __ATTR_PURE wchar_t const *(__LIBCCALL wmemchr)(wchar_t const *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memchrl(__s,(__UINT32_TYPE__)__c,__n); }
#elif __SIZEOF_WCHAR_T__ == 2
__LOCAL __ATTR_PURE wchar_t *(__LIBCCALL wmemchr)(wchar_t *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memchrl(__s,(__UINT16_TYPE__)__c,__n); }
__LOCAL __ATTR_PURE wchar_t const *(__LIBCCALL wmemchr)(wchar_t const *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memchrl(__s,(__UINT16_TYPE__)__c,__n); }
#else /* ... */
__LOCAL __ATTR_PURE wchar_t *(__LIBCCALL wmemchr)(wchar_t *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memchr(__s,(int)__c,__n*sizeof(wchar_t)); }
__LOCAL __ATTR_PURE wchar_t const *(__LIBCCALL wmemchr)(wchar_t const *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memchr(__s,(int)__c,__n*sizeof(wchar_t)); }
#endif /* !... */
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wmemchr)(wchar_t *__s, wchar_t __c, size_t __n) __ASMNAME("wmemchr");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wmemchr)(wchar_t const *__s, wchar_t __c, size_t __n) __ASMNAME("wmemchr");
#endif /* !__DOS_COMPAT__ */
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
#ifdef __DOS_COMPAT__
#if __SIZEOF_WCHAR_T__ == 4
__LOCAL __ATTR_PURE wchar_t *(__LIBCCALL wmemchr)(wchar_t const *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memchrl(__s,(__UINT32_TYPE__)__c,__n); }
#elif __SIZEOF_WCHAR_T__ == 2
__LOCAL __ATTR_PURE wchar_t *(__LIBCCALL wmemchr)(wchar_t const *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memchrl(__s,(__UINT16_TYPE__)__c,__n); }
#else /* ... */
__LOCAL __ATTR_PURE wchar_t *(__LIBCCALL wmemchr)(wchar_t const *__s, wchar_t __c, size_t __n) { return (wchar_t *)__libc_memchr(__s,(int)__c,__n*sizeof(wchar_t)); }
#endif /* !... */
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wmemchr)(wchar_t const *__s, wchar_t __c, size_t __n);
#endif /* !__DOS_COMPAT__ */
#endif
#if defined(__USE_ISOC95) || defined(__USE_UNIX98)
#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__LIBC int (__LIBCCALL fwide)(__FILE *__fp, int __mode);
#else /* __CRT_GLC && !__DOS_COMPAT__ */
__LOCAL int (__LIBCCALL fwide)(__FILE *__UNUSED(__fp), int __UNUSED(__mode)) { return 0; }
#endif /* !__CRT_GLC || __DOS_COMPAT__ */
#endif /* __USE_ISOC95 || __USE_UNIX98 */
#if defined(__USE_ISOC95) || defined(__USE_UNIX98) || defined(__USE_DOS)
#ifndef __wprintf_defined
#if defined(__USE_KOS) && (__SIZEOF_SIZE_T__ <= __SIZEOF_INT__ || defined(__CRT_KOS))
__LIBC __ssize_t (__ATTR_CDECL fwprintf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __ssize_t (__ATTR_CDECL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC __ssize_t (__LIBCCALL vfwprintf)(__FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC __ssize_t (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC __ssize_t (__ATTR_CDECL fwscanf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __ssize_t (__ATTR_CDECL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC __ssize_t (__ATTR_CDECL swscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) __PE_ASMNAME("_swscanf");
__REDIRECT_IFW16(__LIBC,,__ssize_t,__LIBCCALL,vswprintf,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args),_vswprintf_c,(__buf,__buflen,__format,__args))
#ifndef __NO_ASMNAME
#if __SIZEOF_WCHAR_T__ == 2 && defined(__CRT_DOS)
__LIBC __ssize_t (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME("_swprintf_c");
#else /* __SIZEOF_WCHAR_T__ == 2 && __CRT_DOS */
__LIBC __ssize_t (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
#endif /* __SIZEOF_WCHAR_T__ != 2 || !__CRT_DOS */
#else /* !__NO_ASMNAME */
__LOCAL __ssize_t (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { __VA_LIST __args; __ssize_t __result; __builtin_va_start(__args,__format); __result = vswprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* __NO_ASMNAME */
#else /* __USE_KOS */
__LIBC int (__ATTR_CDECL fwprintf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vfwprintf)(__FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC int (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC int (__ATTR_CDECL fwscanf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC int (__ATTR_CDECL swscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) __PE_ASMNAME("_swscanf");
__REDIRECT_IFW16(__LIBC,,int,__LIBCCALL,vswprintf,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args),_vswprintf_c,(__buf,__buflen,__format,__args))
#ifndef __NO_ASMNAME
#if __SIZEOF_WCHAR_T__ == 2 && defined(__CRT_DOS)
__LIBC int (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME("_swprintf_c");
#else /* __SIZEOF_WCHAR_T__ == 2 && __CRT_DOS */
__LIBC int (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
#endif /* __SIZEOF_WCHAR_T__ != 2 || !__CRT_DOS */
#else /* !__NO_ASMNAME */
__LOCAL int (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { __VA_LIST __args; int __result; __builtin_va_start(__args,__format); __result = vswprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* __NO_ASMNAME */
#endif /* !__USE_KOS */
#endif /* !__wprintf_defined */
#endif /* __USE_ISOC95 || __USE_UNIX98 || __USE_DOS */
#ifdef __USE_ISOC99
#ifndef __wcstof_defined
__LIBC float (__LIBCCALL wcstof)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
__LIBC long double (__LIBCCALL wcstold)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
#endif /* !__wcstof_defined */
#if (defined(__PE__) || defined(__DOS_COMPAT__)) && __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,,__LONGLONG,__LIBCCALL,wcstoll,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),_wcstoi64,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,,__ULONGLONG,__LIBCCALL,wcstoull,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),_wcstoui64,(__nptr,__endptr,__base))
#elif __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
__REDIRECT(__LIBC,,__LONGLONG,__LIBCCALL,wcstoll,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,,__ULONGLONG,__LIBCCALL,wcstoull,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),wcstoul,(__nptr,__endptr,__base))
#else
__LIBC __LONGLONG (__LIBCCALL wcstoll)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
__LIBC __ULONGLONG (__LIBCCALL wcstoull)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
#endif
#ifndef __wprintf_defined
#ifdef __USE_KOS
__LIBC __ssize_t (__LIBCCALL vfwscanf)(__FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC __ssize_t (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC __ssize_t (__LIBCCALL vswscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __VA_LIST __arg);
#else /* __USE_KOS */
__LIBC int (__LIBCCALL vfwscanf)(__FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC int (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __VA_LIST __arg);
__LIBC int (__LIBCCALL vswscanf)(wchar_t const *__restrict __rc, wchar_t const *__restrict __format, __VA_LIST __arg);
#endif /* !__USE_KOS */
#endif /* !__wprintf_defined */
#endif /* __USE_ISOC99 */
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END

#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */
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
#ifndef __getwchar_defined
#define __getwchar_defined 1
__NAMESPACE_STD_USING(getwchar)
__NAMESPACE_STD_USING(fgetwc)
__NAMESPACE_STD_USING(getwc)
#endif /* !__getwchar_defined */
#ifndef __putwchar_defined
#define __putwchar_defined 1
__NAMESPACE_STD_USING(putwchar)
__NAMESPACE_STD_USING(fputwc)
__NAMESPACE_STD_USING(putwc)
#endif /* !__putwchar_defined */
#ifndef __fgetws_defined
#define __fgetws_defined 1
__NAMESPACE_STD_USING(fgetws)
#endif /* !__fgetws_defined */
#ifndef __fputws_defined
#define __fputws_defined 1
__NAMESPACE_STD_USING(fputws)
#endif /* !__fputws_defined */
#ifndef __ungetwc_defined
#define __ungetwc_defined 1
__NAMESPACE_STD_USING(ungetwc)
#endif /* !__ungetwc_defined */
#ifndef __wcsftime_defined
#define __wcsftime_defined 1
__NAMESPACE_STD_USING(wcsftime)
#endif /* !__wcsftime_defined */
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
#endif /* __USE_ISOC95 || __USE_UNIX98 */
#if defined(__USE_ISOC95) || defined(__USE_UNIX98) || defined(__USE_DOS)
#ifndef __wprintf_defined
__NAMESPACE_STD_USING(fwprintf)
__NAMESPACE_STD_USING(wprintf)
__NAMESPACE_STD_USING(swprintf)
__NAMESPACE_STD_USING(vfwprintf)
__NAMESPACE_STD_USING(vwprintf)
__NAMESPACE_STD_USING(vswprintf)
__NAMESPACE_STD_USING(fwscanf)
__NAMESPACE_STD_USING(wscanf)
__NAMESPACE_STD_USING(swscanf)
#endif /* !__wprintf_defined */
#endif /* __USE_ISOC95 || __USE_UNIX98 || __USE_DOS */
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
#ifndef __wprintf_defined
__NAMESPACE_STD_USING(vfwscanf)
__NAMESPACE_STD_USING(vwscanf)
__NAMESPACE_STD_USING(vswscanf)
#endif /* !__wprintf_defined */
#endif /* __USE_ISOC99 */
#ifndef __wprintf_defined
#define __wprintf_defined 1
#endif /* !__wprintf_defined */
#endif /* !__KERNEL__ */

#ifndef __KERNEL__
#ifdef __USE_XOPEN2K8
__REDIRECT_IFDOS(__LIBC,__WUNUSED,int,__LIBCCALL,wcscasecmp,(wchar_t const *__s1, wchar_t const *__s2),_wcsicmp,(__s1,__s2))
__REDIRECT_IFDOS(__LIBC,__WUNUSED,int,__LIBCCALL,wcsncasecmp,(wchar_t const *__s1, wchar_t const *__s2, size_t __n),_wcsnicmp,(__s1,__s2,__n))
__REDIRECT_IFDOS(__LIBC,__WUNUSED,int,__LIBCCALL,wcscasecmp_l,(wchar_t const *__s1, wchar_t const *__s2, __locale_t __locale),_wcsicmp_l,(__s1,__s2,__locale))
__REDIRECT_IFDOS(__LIBC,__WUNUSED,int,__LIBCCALL,wcsncasecmp_l,(wchar_t const *__s1, wchar_t const *__s2, size_t __n, __locale_t __locale),_wcsnicmp_l,(__s1,__s2,__n,__locale))
__REDIRECT_IFDOS(__LIBC,__WUNUSED,int,__LIBCCALL,wcscoll_l,(wchar_t const *__s1, wchar_t const *__s2, __locale_t __locale),_wcscoll_l,(__s1,__s2,__locale))
__REDIRECT_IFDOS(__LIBC,,size_t,__LIBCCALL,wcsxfrm_l,(wchar_t *__s1, wchar_t const *__s2, size_t __n, __locale_t __locale),_wcsxfrm_l,(__s1,__s2,__n,__locale))
#ifdef __CRT_GLC
__LIBC wchar_t *(__LIBCCALL wcpcpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src);
__LIBC wchar_t *(__LIBCCALL wcpncpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __n);
/* TODO: Check if there really isn't any way of emulate the following two (because they're _very_ useful...) */
__LIBC __PORT_NODOS size_t (__LIBCCALL mbsnrtowcs)(wchar_t *__restrict __dst, char const **__restrict __psrc, size_t __nmc, size_t __len, mbstate_t *__restrict __ps);
__LIBC __PORT_NODOS size_t (__LIBCCALL wcsnrtombs)(char *__restrict __dst, wchar_t const **__restrict __psrc, size_t __nwc, size_t __len, mbstate_t *__restrict __ps);
__REDIRECT_IFW16(__LIBC,__PORT_NODOS,__FILE *,__LIBCCALL,open_wmemstream,(wchar_t **__bufloc, size_t *__sizeloc),_open_wmemstream,(__bufloc,__sizeloc))
#else /* __CRT_GLC */
__LOCAL wchar_t *(__LIBCCALL wcpcpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src) { return wcscpy(__dst,__src)+wcslen(__dst); }
__LOCAL wchar_t *(__LIBCCALL wcpncpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __n) { return wcsncpy(__dst,__src,__n)+wcslen(__dst); }
#endif /* !__CRT_GLC */
#endif /* __USE_XOPEN2K8 */

#if defined(__USE_XOPEN2K8) || defined(__USE_DOS)
#ifndef __wcsnlen_defined
#define __wcsnlen_defined 1
__LIBC __ATTR_PURE size_t (__LIBCCALL wcsnlen)(wchar_t const *__s, size_t __maxlen);
#endif /* !__wcsnlen_defined */
#ifndef __wcsdup_defined
#define __wcsdup_defined 1
__REDIRECT_IFDOS(__LIBC,__SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,
                wchar_t *,__LIBCCALL,wcsdup,(wchar_t const *__restrict __str),_wcsdup,(__str))
#endif /* !__wcsdup_defined */
#endif /* __USE_XOPEN2K8 || __USE_DOS */

#ifdef __USE_XOPEN
#ifdef __CRT_GLC
#if defined(__USE_KOS) && (__SIZEOF_SIZE_T__ <= __SIZEOF_INT__ || defined(__CRT_KOS))
__LIBC __PORT_NODOS __ssize_t (__LIBCCALL wcwidth)(wchar_t __c);
__LIBC __PORT_NODOS __ssize_t (__LIBCCALL wcswidth)(wchar_t const *__restrict __s, size_t __n);
#else /* __USE_KOS */
__LIBC __PORT_NODOS int (__LIBCCALL wcwidth)(wchar_t __c);
__LIBC __PORT_NODOS int (__LIBCCALL wcswidth)(wchar_t const *__restrict __s, size_t __n);
#endif /* !__USE_KOS */
#endif /* __CRT_GLC */
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
__REDIRECT(__LIBC,__ATTR_PURE,wchar_t *,__LIBCCALL,wcswcs,(wchar_t const *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
#endif
#endif /* !__wcswcs_defined */
#endif /* __USE_XOPEN || __USE_DOS */

#ifdef __USE_GNU
#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcschrnul)(wchar_t const *__haystack, wchar_t __needle);
__LIBC wchar_t *(__LIBCCALL wmempcpy)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n);
#else /* __CRT_GLC */
__LOCAL __ATTR_PURE wchar_t *(__LIBCCALL wcschrnul)(wchar_t const *__haystack, wchar_t __needle) { wchar_t *__iter = (wchar_t *)__haystack; for (; *__iter && *__iter != __needle; ++__iter); return __iter; }
__LOCAL wchar_t *(__LIBCCALL wmempcpy)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n) { return __NAMESPACE_STD_SYM wmemcpy(__s1,__s2,__n)+__n; }
#endif /* !__CRT_GLC */
__REDIRECT_IFDOS(__LIBC,,long int,__LIBCCALL,wcstol_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),_wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_IFDOS(__LIBC,,unsigned long int,__LIBCCALL,wcstoul_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),_wcstoul_l,(__nptr,__endptr,__base,__locale))
#if defined(__CRT_DOS) && __SIZEOF_LONG_LONG__ == 8 && \
  ((defined(__PE__) && !defined(__GLC_COMPAT__)) || defined(__DOS_COMPAT__))
__REDIRECT(__LIBC,,__LONGLONG,__LIBCCALL,wcstoq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),_wcstoi64,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,,__ULONGLONG,__LIBCCALL,wcstouq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),_wcstoui64,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,,__LONGLONG,__LIBCCALL,wcstoll_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),_wcstoi64_l,(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,,__ULONGLONG,__LIBCCALL,wcstoull_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),_wcstoui64_l,(__nptr,__endptr,__base,__locale))
#elif __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
__REDIRECT(__LIBC,,__LONGLONG,__LIBCCALL,wcstoq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,,__ULONGLONG,__LIBCCALL,wcstouq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT2(__LIBC,,__LONGLONG,__LIBCCALL,wcstoll_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),wcstol_l,_wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT2(__LIBC,,__ULONGLONG,__LIBCCALL,wcstoull_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),wcstoul_l,_wcstoul_l,(__nptr,__endptr,__base,__locale))
#else
__REDIRECT2(__LIBC,,__LONGLONG,__LIBCCALL,wcstoq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),wcstoll_l,_wcstoll_l,(__nptr,__endptr,__base))
__REDIRECT2(__LIBC,,__ULONGLONG,__LIBCCALL,wcstouq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),wcstoull_l,_wcstoull_l,(__nptr,__endptr,__base))
__REDIRECT_IFDOS(__LIBC,,__LONGLONG,__LIBCCALL,wcstoll_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),_wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_IFDOS(__LIBC,,__ULONGLONG,__LIBCCALL,wcstoull_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),_wcstoull_l,(__nptr,__endptr,__base,__locale))
#endif
__REDIRECT_IFDOS(__LIBC,,double,__LIBCCALL,wcstod_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __locale),_wcstod_l,(__nptr,__endptr,__locale))
__REDIRECT_IFDOS(__LIBC,,float,__LIBCCALL,wcstof_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __locale),_wcstof_l,(__nptr,__endptr,__locale))
__REDIRECT_IFDOS(__LIBC,,long double,__LIBCCALL,wcstold_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __locale),_wcstold_l,(__nptr,__endptr,__locale))
__REDIRECT_IFDOS(__LIBC,,wint_t,__LIBCCALL,getwchar_unlocked,(void),_getwchar_nolock,())
__REDIRECT2(__LIBC,,wint_t,__LIBCCALL,getwc_unlocked,(__FILE *__stream),fgetwc_unlocked,_fgetwc_nolock,(__stream))
__REDIRECT2(__LIBC,,wint_t,__LIBCCALL,putwc_unlocked,(wchar_t __wc, __FILE *__stream),fputwc_unlocked,_fputwc_nolock,(__wc,__stream))
__REDIRECT_IFDOS(__LIBC,,wint_t,__LIBCCALL,fgetwc_unlocked,(__FILE *__stream),_fgetwc_nolock,(__stream))
__REDIRECT_IFDOS(__LIBC,,wint_t,__LIBCCALL,putwchar_unlocked,(wchar_t __wc),_putwchar_nolock,(__wc))
__REDIRECT_IFDOS(__LIBC,,wint_t,__LIBCCALL,fputwc_unlocked,(wchar_t __wc, __FILE *__stream),_fputwc_nolock,(__wc,__stream))
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__REDIRECT_IFDOS(__LIBC,,wchar_t *,__LIBCCALL,fgetws_unlocked,(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream),_fgetws_nolock,(__buf,__n,__stream))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#if defined(__CRT_KOS) && !defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__)
__REDIRECT2(__LIBC,,wchar_t *,__LIBCCALL,fgetws_unlocked,(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream),fgetws_unlocked_sz,_fgetws_nolock_sz,(__buf,__n,__stream))
#else /* Builtin... */
__REDIRECT2(__LIBC,,wchar_t *,__LIBCCALL,__fgetws_int_unlocked,(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream),fgetws_unlocked,_fgetws_nolock,(__buf,__n,__stream))
__LOCAL wchar_t *(__LIBCCALL fgetws_unlocked)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) { return __fgetws_int_unlocked(__buf,(int)__n,__stream); }
#endif /* Compat... */
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__REDIRECT_IFDOS(__LIBC,,wchar_t *,__LIBCCALL,fgetws_unlocked,(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream),_fgetws_nolock,(__buf,__n,__stream))
#endif /* !__USE_KOS */
__REDIRECT_IFDOS(__LIBC,,int,__LIBCCALL,fputws_unlocked,(wchar_t const *__restrict __str, __FILE *__restrict __stream),_fputws_nolock,(__str,__stream))
__REDIRECT_IFDOS(__LIBC,,size_t,__LIBCCALL,wcsftime_l,(wchar_t *__restrict __buf, size_t __maxsize, wchar_t const *__restrict __format, struct tm const *__restrict __tp, __locale_t __locale),_wcsftime_l,(__buf,__maxsize,__format,__tp,__locale))
#endif /* __USE_GNU */

#ifdef __USE_KOS
#ifndef __wcsnlen_defined
__REDIRECT(__LIBC,__ATTR_PURE,size_t,__LIBCCALL,__libc_wcsnlen,(wchar_t const *__s, size_t __maxlen),wcsnlen,(__s,__maxlen))
#else /* !__wcsnlen_defined */
#define __libc_wcsnlen(s,maxlen) wcsnlen(s,maxlen)
#endif /* __wcsnlen_defined */
#if (defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__)) || !defined(__CRT_KOS)
#ifdef __cplusplus
extern "C++" {
__LOCAL wchar_t *(__LIBCCALL wcsend)(wchar_t *__restrict __s) { return __s+__NAMESPACE_STD_SYM wcslen(__s); }
__LOCAL wchar_t *(__LIBCCALL wcsnend)(wchar_t *__restrict __s, size_t __n) { return __s+__libc_wcsnlen(__s,__n); }
__LOCAL wchar_t const *(__LIBCCALL wcsend)(wchar_t const *__restrict __s) { return __s+__NAMESPACE_STD_SYM wcslen(__s); }
__LOCAL wchar_t const *(__LIBCCALL wcsnend)(wchar_t const *__restrict __s, size_t __n) { return __s+__libc_wcsnlen(__s,__n); }
}
#else /* C++ */
__LOCAL wchar_t *(__LIBCCALL wcsend)(wchar_t const *__restrict __s) { return (wchar_t *)__s+__NAMESPACE_STD_SYM wcslen(__s); }
__LOCAL wchar_t *(__LIBCCALL wcsnend)(wchar_t const *__restrict __s, size_t __n) { return (wchar_t *)__s+__libc_wcsnlen(__s,__n); }
#endif /* !C++ */
#else /* Compat... */
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC wchar_t *(__LIBCCALL wcsend)(wchar_t *__restrict __s) __ASMNAME("wcsend");
__LIBC wchar_t *(__LIBCCALL wcsnend)(wchar_t *__restrict __s, size_t __n) __ASMNAME("wcsnend");
__LIBC wchar_t const *(__LIBCCALL wcsend)(wchar_t const *__restrict __s) __ASMNAME("wcsend");
__LIBC wchar_t const *(__LIBCCALL wcsnend)(wchar_t const *__restrict __s, size_t __n) __ASMNAME("wcsnend");
}
#else /* C++ */
__LIBC wchar_t *(__LIBCCALL wcsend)(wchar_t const *__restrict __s);
__LIBC wchar_t *(__LIBCCALL wcsnend)(wchar_t const *__restrict __s, size_t __n);
#endif /* !C++ */
#endif /* Builtin... */
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
#ifdef __DOS_COMPAT__
#ifdef __USE_DOS_LINKOBJECTS
__LIBC FILE _iob[];
#define stdin  (_iob+0)
#define stdout (_iob+1)
#define stderr (_iob+2)
#else /* __USE_DOS_LINKOBJECTS */
__LIBC FILE *(__LIBCCALL __iob_func)(void);
#define stdin  (__iob_func()+0)
#define stdout (__iob_func()+1)
#define stderr (__iob_func()+2)
#endif /* !__USE_DOS_LINKOBJECTS */
#else /* __DOS_COMPAT__ */
__LIBC __FILE *stdin;
__LIBC __FILE *stdout;
__LIBC __FILE *stderr;
#define stdin  stdin
#define stdout stdout
#define stderr stderr
#endif /* !__DOS_COMPAT__ */
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
 __UINT32_TYPE__ __pad0;
 __time64_t      time_create;
 __time64_t      time_access;
 __time64_t      time_write;
 _fsize_t        size;
 wchar_t         name[260];
 __UINT32_TYPE__ __pad1;
};

struct _wfinddata64_t {
 __UINT32_TYPE__ attrib;
 __UINT32_TYPE__ __pad0;
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

#ifdef __CRT_DOS
#ifndef _WDIRECT_DEFINED
#define _WDIRECT_DEFINED 1
__LIBC __PORT_NODOS __NONNULL((1)) wchar_t *(__LIBCCALL _wgetcwd)(wchar_t *__dstbuf, int __dstlen);
__LIBC __PORT_NODOS __NONNULL((2)) wchar_t *(__LIBCCALL _wgetdcwd)(int __drive, wchar_t *__dstbuf, int __dstlen);
#define _wgetdcwd_nolock    _wgetdcwd
__REDIRECT_WFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,_wchdir,(wchar_t const *__path),_wchdir,(__path));
__REDIRECT_WFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,_wrmdir,(wchar_t const *__path),_wrmdir,(__path));
#ifdef __USE_DOSFS
__REDIRECT_WFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,_wmkdir,(wchar_t const *__path),_wmkdir,(__path));
#else /* __USE_DOSFS */
__REDIRECT_WFS_(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,__libc_wmkdir,(wchar_t const *__path, int __mode),_wmkdir,(__path,__mode));
__LOCAL __PORT_NODOS __NONNULL((1)) int (__LIBCCALL _wmkdir)(wchar_t const *__path) { return __libc_wmkdir(__path,0755); }
#endif /* !__USE_DOSFS */
#endif /* !_WDIRECT_DEFINED */

#ifndef _WIO_DEFINED
#define _WIO_DEFINED 1
struct _wfinddata32_t;
struct _wfinddata64_t;
struct _wfinddata32i64_t;
struct _wfinddata64i32_t;

__LIBC __PORT_DOSONLY int (__ATTR_CDECL _wopen)(wchar_t const *__file, int __oflag, ...) __WFS_FUNC(_wopen); /* TODO: Use redirection */
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _wsopen)(wchar_t const *__file, int __oflag, int __sflag, ...) __WFS_FUNC(_wsopen); /* TODO: Use redirection */
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wcreat,(wchar_t const *__file, int __pmode),_wcreat,(__file,__pmode))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wsopen_s,(int *__fd, wchar_t const *__file, int __oflag, int __sflag, int __pflags),_wsopen_s,(__fd,__file,__oflag,__sflag,__pflags))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_waccess,(wchar_t const *__file, int __type),_waccess,(__file,__type))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_waccess_s,(wchar_t const *__file, int __type),_waccess,(__file,__type))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wchmod,(wchar_t const *__file, int __mode),_wchmod,(__file,__mode))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wunlink,(wchar_t const *__file),_wunlink,(__file))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wrename,(wchar_t const *__oldname, wchar_t const *__newname),_wrename,(__oldname,__newname))
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _wmktemp_s)(wchar_t *__templatename, size_t __sizeinwords);
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,intptr_t,__LIBCCALL,_wfindfirst32,(wchar_t const *__file, struct _wfinddata32_t *__finddata),_wfindfirst32,(__file,__finddata))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,intptr_t,__LIBCCALL,_wfindfirst64,(wchar_t const *__file, struct _wfinddata64_t *__finddata),_wfindfirst64,(__file,__finddata))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,intptr_t,__LIBCCALL,_wfindfirst32i64,(wchar_t const *__file, struct _wfinddata32i64_t *__finddata),_wfindfirst32i64,(__file,__finddata))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,intptr_t,__LIBCCALL,_wfindfirst64i32,(wchar_t const *__file, struct _wfinddata64i32_t *__finddata),_wfindfirst64i32,(__file,__finddata))
__LIBC __PORT_DOSONLY int (__LIBCCALL _wfindnext32)(intptr_t __findfd, struct _wfinddata32_t *__finddata);
__LIBC __PORT_DOSONLY int (__LIBCCALL _wfindnext64)(intptr_t __findfd, struct _wfinddata64_t *__finddata);
__LIBC __PORT_DOSONLY int (__LIBCCALL _wfindnext32i64)(intptr_t __findfd, struct _wfinddata32i64_t *__finddata);
__LIBC __PORT_DOSONLY int (__LIBCCALL _wfindnext64i32)(intptr_t __findfd, struct _wfinddata64i32_t *__finddata);
#endif /* !_WIO_DEFINED */

#ifndef _WLOCALE_DEFINED
#define _WLOCALE_DEFINED 1
__LIBC __PORT_DOSONLY wchar_t *(__LIBCCALL _wsetlocale)(int __category, wchar_t const *__locale);
__LIBC __PORT_DOSONLY __locale_t (__LIBCCALL _wcreate_locale)(int __category, wchar_t const *__locale);
#endif /* !_WLOCALE_DEFINED */

#ifndef __TWARGV
#ifdef __USE_DOS
#   define __TWARGV wchar_t const *const *___argv
#   define __TWENVP wchar_t const *const *___envp
#else
#   define __TWARGV wchar_t *const ___argv[]
#   define __TWENVP wchar_t *const ___envp[]
#endif
#endif /* !__TARGV */

#ifndef _WPROCESS_DEFINED
#define _WPROCESS_DEFINED 1
__LIBC __PORT_DOSONLY intptr_t (__ATTR_CDECL _wexecl)(wchar_t const *__path, wchar_t const *__args, ...) __WFS_FUNC(_wexecl); /* TODO: Redirect. */
__LIBC __PORT_DOSONLY intptr_t (__ATTR_CDECL _wexecle)(wchar_t const *__path, wchar_t const *__args, ...) __WFS_FUNC(_wexecle);
__LIBC __PORT_DOSONLY intptr_t (__ATTR_CDECL _wexeclp)(wchar_t const *__file, wchar_t const *__args, ...) __WFS_FUNC(_wexeclp);
__LIBC __PORT_DOSONLY intptr_t (__ATTR_CDECL _wexeclpe)(wchar_t const *__file, wchar_t const *__args, ...) __WFS_FUNC(_wexeclpe);
__LIBC __PORT_DOSONLY intptr_t (__ATTR_CDECL _wspawnl)(int __mode, wchar_t const *__path, wchar_t const *__args, ...) __WFS_FUNC(_wspawnl);
__LIBC __PORT_DOSONLY intptr_t (__ATTR_CDECL _wspawnle)(int __mode, wchar_t const *__path, wchar_t const *__args, ...) __WFS_FUNC(_wspawnle);
__LIBC __PORT_DOSONLY intptr_t (__ATTR_CDECL _wspawnlp)(int __mode, wchar_t const *__file, wchar_t const *__args, ...) __WFS_FUNC(_wspawnlp);
__LIBC __PORT_DOSONLY intptr_t (__ATTR_CDECL _wspawnlpe)(int __mode, wchar_t const *__file, wchar_t const *__args, ...) __WFS_FUNC(_wspawnlpe);
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,,intptr_t,__LIBCCALL,_wexecv,(wchar_t const *__path, __TWARGV),wexecv,(__path,___argv))
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,,intptr_t,__LIBCCALL,_wexecve,(wchar_t const *__path, __TWARGV, __TWENVP),wexecve,(__path,___argv,___envp))
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,,intptr_t,__LIBCCALL,_wexecvp,(wchar_t const *__file, __TWARGV),wexecvp,(__file,___argv));
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,,intptr_t,__LIBCCALL,_wexecvpe,(wchar_t const *__file, __TWARGV, __TWENVP),wexecvpe,(__file,___argv,___envp));
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,,intptr_t,__LIBCCALL,_wspawnv,(int __mode, wchar_t const *__path, __TWARGV),wspawnv,(__mode,__path,___argv))
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,,intptr_t,__LIBCCALL,_wspawnve,(int __mode, wchar_t const *__path, __TWARGV, __TWENVP),wspawnve,(__mode,__path,___argv,___envp))
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,,intptr_t,__LIBCCALL,_wspawnvp,(int __mode, wchar_t const *__file, __TWARGV),wspawnvp,(__mode,__file,___argv));
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,,intptr_t,__LIBCCALL,_wspawnvpe,(int __mode, wchar_t const *__file, __TWARGV, __TWENVP),wspawnvpe,(__mode,__file,___argv,___envp));
#endif /* !_WPROCESS_DEFINED */

#ifndef _CRT_WSYSTEM_DEFINED
#define _CRT_WSYSTEM_DEFINED 1
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wsystem,(wchar_t const *__restrict __cmd),wsystem,(__cmd))
#endif /* !_CRT_WSYSTEM_DEFINED */

#ifndef _WSTAT_DEFINED
#define _WSTAT_DEFINED 1
#if defined(__PE__) || defined(__DOS_COMPAT__)
__LIBC __PORT_DOSONLY __WARN_NOKOSFS int (__LIBCCALL _wstat64)(wchar_t const *__name, struct _stat64 *__buf);
__LIBC __PORT_DOSONLY __WARN_NOKOSFS int (__LIBCCALL _wstat32i64)(wchar_t const *__name, struct _stat32i64 *__buf);
__REDIRECT(__LIBC,__PORT_DOSONLY __WARN_NOKOSFS,int,__LIBCCALL,_wstat32,(wchar_t const *__name, struct _stat32 *__buf),_wstat,(__name,__buf))
__REDIRECT(__LIBC,__PORT_DOSONLY __WARN_NOKOSFS,int,__LIBCCALL,_wstat64i32,(wchar_t const *__name, struct _stat64i32 *__buf),_wstat64,(__name,__buf))
#else /* __PE__ || __DOS_COMPAT__ */
__REDIRECT_WFS_(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wstat32,(wchar_t const *__name, struct _stat32 *__buf),wstat,(__name,__buf))
__REDIRECT_WFS_(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wstat64,(wchar_t const *__name, struct _stat64 *__buf),wstat,(__name,__buf))
__REDIRECT_WFS_(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wstat32i64,(wchar_t const *__name, struct _stat32i64 *__buf),wstat,(__name,__buf))
__REDIRECT_WFS_(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wstat64i32,(wchar_t const *__name, struct _stat64i32 *__buf),wstat,(__name,__buf))
#endif /* !__PE__ && !__DOS_COMPAT__ */
#endif /* !_WSTAT_DEFINED */

#ifndef _WCONIO_DEFINED
#define _WCONIO_DEFINED 1
/* Since KOS doesn't differentiate for these, they just redirect to stdin/stdout.
 * >> If you really want to operate on a console, open your terminal slave under '/dev/'. */
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_cgetws_s,(wchar_t *__buffer, size_t __buflen, size_t *__sizeok),_getws_s,())
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_cgetws,(wchar_t *__buffer),_getws,(__buffer))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wint_t,__LIBCCALL,_getwch,(void),getwchar,())
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wint_t,__LIBCCALL,_getwch_nolock,(void),getwchar_unlocked,())
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wint_t,__LIBCCALL,_getwche,(void),getwchar,())
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wint_t,__LIBCCALL,_getwche_nolock,(void),getwchar_unlocked,())
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wint_t,__LIBCCALL,_putwch,(wchar_t __wc),putwchar,(__wc))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wint_t,__LIBCCALL,_putwch_nolock,(wchar_t __wc),putwchar_unlocked,(__wc))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,wint_t,__LIBCCALL,_ungetwch,(wint_t __wc),ungetwch,(__wc))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,wint_t,__LIBCCALL,_ungetwch_nolock,(wint_t __wc),ungetwch_nolock,(__wc))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_cputws,(wchar_t const *__string),_putws,(__string))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_vcwprintf,(wchar_t const *__format, __VA_LIST __args),vwprintf,(__format,__args))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_vcwprintf_p,(wchar_t const*__format, __VA_LIST __args),vwprintf,(__format,__args))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_vcwprintf_s,(wchar_t const *__format, __VA_LIST __args),vwprintf,(__format,__args))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_vcwprintf_l,(wchar_t const *__format, __locale_t __locale, __VA_LIST __args),_vwprintf_l,(__format,__locale,__args))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_vcwprintf_p_l,(wchar_t const *__format, __locale_t __locale, __VA_LIST __args),_vwprintf_l,(__format,__locale,__args))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_vcwprintf_s_l,(wchar_t const *__format, __locale_t __locale, __VA_LIST __args),_vwprintf_l,(__format,__locale,__args))
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _cwprintf)(wchar_t const *__format, ...) __ASMNAME("wprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _cwprintf_p)(wchar_t const *__format, ...) __ASMNAME("wprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _cwprintf_s)(wchar_t const *__format, ...) __ASMNAME("wprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _cwprintf_l)(wchar_t const *__format, __locale_t __locale, ...) __ASMNAME("_wprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _cwprintf_p_l)(wchar_t const *__format, __locale_t __locale, ...) __ASMNAME("_wprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _cwprintf_s_l)(wchar_t const *__format, __locale_t __locale, ...) __ASMNAME("_wprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _cwscanf)(wchar_t const *__format, ...) __ASMNAME("wscanf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _cwscanf_s)(wchar_t const *__format, ...) __ASMNAME("wscanf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _cwscanf_l)(wchar_t const *__format, __locale_t __locale, ...) __ASMNAME("_wscanf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _cwscanf_s_l)(wchar_t const *__format, __locale_t __locale, ...) __ASMNAME("_wscanf_l");
#endif /* !_WCONIO_DEFINED */

#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED 1
__REDIRECT_IFW32_VOID(__LIBC,__PORT_DOSONLY __ATTR_COLD,__LIBCCALL,_wperror,
                     (wchar_t const *__restrict __errmsg),wperror,(__errmsg))
#endif /* !_CRT_WPERROR_DEFINED */
#endif /* __CRT_DOS */

#ifndef _WSTDIO_DEFINED
#define _WSTDIO_DEFINED 1

#ifdef __CRT_DOS
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,_wfsopen,(wchar_t const *__file, wchar_t const *__mode, int __shflag),_wfsopen,(__file,__mode,__shflag))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,_wfopen,(wchar_t const *__file, wchar_t const *__mode),_wfopen,(__file,__mode))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,_wfreopen,(wchar_t const *__file, wchar_t const *__mode, FILE *__oldfile),_wfreopen,(__file,__mode,__oldfile))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wfopen_s,(FILE **__pfile, wchar_t const *__file, wchar_t const *__mode),_wfopen_s,(__pfile,__file,__mode))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wfreopen_s,(FILE **__pfile, wchar_t const *__file, wchar_t const *__mode, FILE *__oldfile),_wfreopen_s,(__pfile,__file,__mode,__oldfile))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,_wfdopen,(int __fd, wchar_t const *__mode),wfdopen,(__fd,__mode))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,_wpopen,(wchar_t const *__command, wchar_t const *__mode),wpopen,(__command,__mode))
#endif /* __CRT_DOS */

/* Get wide character functions */
__REDIRECT(__LIBC,,wint_t,__LIBCCALL,_fgetwchar,(void),getwchar,())
__REDIRECT_IFKOS(__LIBC,,wint_t,__LIBCCALL,_fgetwc_nolock,(FILE *__restrict __file),fgetwc_unlocked,(__file))

/* Put wide character functions */
__REDIRECT(__LIBC,,wint_t,__LIBCCALL,_fputwchar,(wchar_t __ch),putwchar,(__ch))
__REDIRECT_IFKOS(__LIBC,,wint_t,__LIBCCALL,_fputwc_nolock,(wchar_t __ch, FILE *__restrict __file),fputwc_unlocked,(__ch,__file))

/* Unget character functions */
__REDIRECT_IFKOS(__LIBC,,wint_t,__LIBCCALL,_ungetwc_nolock,(wint_t __ch, FILE *__restrict __file),ungetwc_unlocked,(__ch,__file))

/* Get wide string functions */
#ifdef __CRT_DOS
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_getws,(wchar_t *__restrict __buf),getws,(__buf))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_getws_s,(wchar_t *__restrict __buf, size_t __buflen),getws_s,(__buf,__buflen))
#endif /* __CRT_DOS */

/* Put wide string functions */
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_putws,(wchar_t const *__restrict __str),putws,(__str))

__LIBC __PORT_DOSONLY int (__ATTR_CDECL _scwprintf)(wchar_t const *__restrict __format, ...) __ASMNAME_IFKOS("scwprintf");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vscwprintf)(wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME_IFKOS("vscwprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _swprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME_IFKOS("swprintf");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vswprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME_IFKOS("vswprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _fwprintf_p)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwprintf");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vfwprintf_p)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vfwprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _wprintf_p)(wchar_t const *__restrict __format, ...) __ASMNAME("wprintf");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vwprintf_p)(wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vwprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _swprintf_p)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME2("swprintf","_swprintf_c");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vswprintf_p)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME2("vswprintf","_vswprintf_c");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _scwprintf_p)(wchar_t const *__restrict __format, ...) __ASMNAME2("scwprintf","_scwprintf");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vscwprintf_p)(wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME2("vscwprintf","_vscwprintf");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _wprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("wprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME_IFKOS("vwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _wprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("wprintf_l","_wprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vwprintf_l","_vwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _wprintf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("wprintf_l","_wprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vwprintf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vwprintf_l","_vwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _fwprintf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("fwprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vfwprintf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME_IFKOS("vfwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _fwprintf_p_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("fwprintf_l","_fwprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vfwprintf_p_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vfwprintf_l","_vfwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _fwprintf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("fwprintf_l","_fwprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vfwprintf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vfwprintf_l","_vfwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _swprintf_c_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("swprintf_c_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vswprintf_c_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME_IFKOS("vswprintf_c_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _swprintf_p_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swprintf_c_l","_swprintf_c_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vswprintf_p_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vswprintf_c_l","_vswprintf_c_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _swprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swprintf_c_l","_swprintf_c_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vswprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vswprintf_c_l","_vswprintf_c_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _scwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("scwprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vscwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME_IFKOS("vscwprintf_l");
__LIBC __PORT_DOSONLY int (__ATTR_CDECL _scwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("scwprintf_l","_scwprintf_l");
__LIBC __PORT_DOSONLY int (__LIBCCALL _vscwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vscwprintf_l","_vscwprintf_l");
#ifdef __PE__
/* The following return an error, rather than the required size when the buffer is too small */
__LIBC __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, ...);
__LIBC __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
#else /* __PE__ */
/* Outside of DOS-mode, libc doesn't export DOS's broken wide-string printer functions, so we emulate them here. */
__LOCAL __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) { size_t __result = vswprintf(__buf,__buflen,__format,__args); return __result < __buflen ? (int)__result : -1; }
__LOCAL __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __UNUSED(__locale), __VA_LIST __args) { return _vsnwprintf(__buf,__buflen,__format,__args); }
__LOCAL __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __VA_LIST __args) { return _vsnwprintf(__buf,__buflen < __maxlen ? __buflen : __maxlen,__format,__args); }
__LOCAL __WARN_NONSTD(vsnwprintf) int (__LIBCCALL _vsnwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __UNUSED(__locale), __VA_LIST __args) { return _vsnwprintf_s(__buf,__buflen,__maxlen,__format,__args); }
__LOCAL __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__format); __result = _vsnwprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__locale); __result = _vsnwprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__format); __result = _vsnwprintf_s(__buf,__buflen,__maxlen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __WARN_NONSTD(snwprintf) int (__ATTR_CDECL _snwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__locale); __result = _vsnwprintf_s(__buf,__buflen,__maxlen,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__PE__ */


/* NOTE: ~safe~ functions are re-directed to the regular versions. (For the reason, see below) */
#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, ...) __ASMNAME_IFKOS("snwscanf"); /* No varargs version. */
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf_l)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("snwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _fwscanf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("fwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _swscanf_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("swscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _wscanf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("wscanf_l"); /* No varargs version. */
#ifdef __USE_DOS
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf_s)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, ...) __ASMNAME_IFKOS("snwscanf"); /* No varargs version. */
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf_s_l)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("snwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _fwscanf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("fwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _swscanf_s_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("swscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _wscanf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME_IFKOS("wscanf_l"); /* No varargs version. */
#else /* __USE_DOS */
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf_s)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, ...) __ASMNAME2("snwscanf","_snwscanf"); /* No varargs version. */
__LIBC __PORT_DOSONLY_ALT(swscanf) int (__ATTR_CDECL _snwscanf_s_l)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("snwscanf_l","_snwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _fwscanf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("fwscanf_l","_fwscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _swscanf_s_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swscanf_l","_swscanf_l"); /* No varargs version. */
__LIBC int (__ATTR_CDECL _wscanf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("wscanf_l","_wscanf_l"); /* No varargs version. */
#endif /* !__USE_DOS */
#else /* __CRT_DOS */
__LOCAL int (__ATTR_CDECL _fwscanf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__locale); __result = vfwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _fwscanf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__locale); __result = vfwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _swscanf_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__locale); __result = vswscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _swscanf_s_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__locale); __result = vswscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _wscanf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__locale); __result = vwscanf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL _wscanf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__locale); __result = vwscanf(__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__CRT_DOS */

#ifdef __USE_DOS_SLIB
/* Simply redirect these so-called ~safe~ functions to the regular version.
 * In KOS, they're already ~safe~ to begin with, because unknown format strings are always handled.
 * NOTE: For binary compatibility, assembly names such as 'fwprintf_s' are exported as alias,
 *       but should never be used by newly compiled applications. */
#ifndef __NO_ASMNAME
__LIBC int (__ATTR_CDECL fwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwprintf");
__LIBC int (__LIBCCALL vfwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vfwprintf");
__LIBC int (__ATTR_CDECL wprintf_s)(wchar_t const *__restrict __format, ...) __ASMNAME("wprintf");
__LIBC int (__LIBCCALL vwprintf_s)(wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vwprintf");
__LIBC int (__ATTR_CDECL fwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwscanf");
__LIBC int (__LIBCCALL vfwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vfwscanf");
__LIBC int (__ATTR_CDECL wscanf_s)(wchar_t const *__restrict __format, ...) __ASMNAME("wscanf");
__LIBC int (__LIBCCALL vwscanf_s)(wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vwscanf");
__LIBC int (__ATTR_CDECL swprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME2("swprintf","_swprintf_c");
__LIBC int (__LIBCCALL vswprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME2("vswprintf","_vswprintf_c");
__LIBC int (__ATTR_CDECL swscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) __ASMNAME("swscanf");
__LIBC int (__LIBCCALL vswscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __VA_LIST __args) __ASMNAME("vswscanf");
#else /* !__NO_ASMNAME */
__LOCAL int (__LIBCCALL vfwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args) { return vfwprintf(__file,__format,__args); }
__LOCAL int (__LIBCCALL vwprintf_s)(wchar_t const *__restrict __format, __VA_LIST __args) { return vwprintf(__format,__args); }
__LOCAL int (__LIBCCALL vfwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __VA_LIST __args) { return vfwscanf(__file,__format,__args); }
__LOCAL int (__LIBCCALL vwscanf_s)(wchar_t const *__restrict __format, __VA_LIST __args) { return vwscanf(__format,__args); }
__LOCAL int (__LIBCCALL vswprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args) { return vswprintf(__buf,__buflen,__format,__args); }
__LOCAL int (__LIBCCALL vswscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __VA_LIST __args) { return vswscanf(__src,__format,__args); }
__LOCAL int (__ATTR_CDECL fwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__format); __result = vfwprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL wprintf_s)(wchar_t const *__restrict __format, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__format); __result = vwprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL fwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__format); __result = vfwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL wscanf_s)(wchar_t const *__restrict __format, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__format); __result = vwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL swprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__format); __result = vswprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL swscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__format); __result = vswscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* __NO_ASMNAME */
#endif /* __USE_DOS_SLIB */

#ifdef __CRT_DOS
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wtmpnam,(wchar_t *__restrict __buf),_wtmpnam,(__buf))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wtmpnam_s,(wchar_t *__restrict __buf, size_t __buflen),_wtmpnam_s,(__buf,__buflen))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wtempnam,(wchar_t const *__dir, wchar_t const *__pfx),_wtempnam,(__dir,__pfx))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wremove,(wchar_t const *__restrict __file),_wremove,(__file))
#endif /* __CRT_DOS */

#if defined(__PE__) && defined(__CRT_DOS)
/* Versions lacking the C standard mandated BUFLEN argument...
 * NOTE: Internally, these functions will link against '.dos._swprintf' and '.dos._vswprintf' */
__LIBC int (__LIBCCALL _vswprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __VA_LIST __args);
__LIBC int (__ATTR_CDECL _swprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, ...);
#else /* __PE__ && __CRT_DOS */
/* Outside of PE-mode, wchar_t is 32 bits wide and '.dos.' isn't inserted before symbol names. */
/* libc doesn't export these superfluous and confusion version of swprintf.
 * (They're lacking the BUFLEN argument mandated by the C standard).
 * So instead, they're implemented as a hack. */
__LOCAL int (__LIBCCALL _vswprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __VA_LIST __args) { return vswprintf(__buf,(size_t)-1,__format,__args); }
__LOCAL int (__ATTR_CDECL _swprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, ...) { __VA_LIST __args; int __result; __builtin_va_start(__args,__format); __result = _vswprintf(__buf,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__PE__ || !__CRT_DOS */

#if defined(__PE__) && defined(__CRT_DOS) /* Unlimited locale wide-string printers (Only defined for DOS mode) */
__LIBC int (__LIBCCALL __vswprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args);
__LIBC int (__ATTR_CDECL __swprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __locale, ...);
#else /* In KOS mode, we emulate these. */
__LOCAL int (__LIBCCALL __vswprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __UNUSED(__locale), __VA_LIST __args) { return _vswprintf(__buf,__format,__args); }
__LOCAL int (__ATTR_CDECL __swprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __locale, ...) { int __result; __VA_LIST __args; __builtin_va_start(__args,__locale); __result = _vswprintf(__buf,__format,__args); __builtin_va_end(__args); return __result; }
#endif

#if !defined(__NO_ASMNAME) && defined(__CRT_DOS)
__LIBC int (__ATTR_CDECL _swprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) __ASMNAME2("swprintf_c_l","_swprintf_c_l");
__LIBC int (__LIBCCALL _vswprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __VA_LIST __args) __ASMNAME2("vswprintf_c_l","_vswprintf_c_l");
#else /* !__NO_ASMNAME && __CRT_DOS */
__REDIRECT2(__LIBC,,int,__LIBCCALL,__dos_vswprintf_c,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __VA_LIST __args),vswprintf,_vswprintf_c,(__buf,__buflen,__format,__args))
__LOCAL int (__ATTR_CDECL _swprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) { __VA_LIST __args; int __result; __builtin_va_start(__args,__locale); __result = __dos_vswprintf_c(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__LIBCCALL _vswprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __UNUSED(__locale), __VA_LIST __args) { return __dos_vswprintf_c(__buf,__buflen,__format,__args); }
#endif /* __NO_ASMNAME || !__CRT_DOS */

#define getwchar()            fgetwc(stdin)
#define putwchar(c)           fputwc((c),stdout)
#define getwc(file)           fgetwc(file)
#define putwc(c,file)         fputwc(c,file)
#define _putwc_nolock(c,file) _fputwc_nolock(c,file)
#define _getwc_nolock(file)   _fgetwc_nolock(file)
#endif  /* _WSTDIO_DEFINED */

#ifndef _WSTDLIB_DEFINED
#define _WSTDLIB_DEFINED 1
#ifdef __CRT_DOS
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_i64tow,(__INT64_TYPE__ __val, wchar_t *__restrict __dst, int __radix),i64tow,(__val,__dst,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ui64tow,(__UINT64_TYPE__ __val, wchar_t *__restrict __dst, int __radix),ui64tow,(__val,__dst,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_i64tow_s,(__INT64_TYPE__ __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix),i64tow_s,(__val,__dst,__maxlen,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ui64tow_s,(__UINT64_TYPE__ __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix),ui64tow_s,(__val,__dst,__maxlen,__radix))
#if __SIZEOF_LONG__ == 8
__REDIRECT2(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ultow,(unsigned long int __val, wchar_t *__restrict __dst, int __radix),ui64tow,_ui64tow,(__val,__dst,__radix))
__REDIRECT2(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ltow,(long int __val, wchar_t *__restrict __dst, int __radix),i64tow,_i64tow,(__val,__dst,__radix))
__REDIRECT2(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ultow_s,(unsigned long int __val, wchar_t *__dst, size_t __maxlen, int __radix),ui64tow_s,_ui64tow_s,(__val,__dst,__maxlen,__radix))
__REDIRECT2(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ltow_s,(long int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix),i64tow_s,_i64tow_s,(__val,__dst,__maxlen,__radix))
#elif __SIZEOF_LONG__ == 4
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ultow,(unsigned long int __val, wchar_t *__restrict __dst, int __radix),ultow,(__val,__dst,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ltow,(long int __val, wchar_t *__restrict __dst, int __radix),ltow,(__val,__dst,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ultow_s,(unsigned long int __val, wchar_t *__dst, size_t __maxlen, int __radix),ultow_s,(__val,__dst,__maxlen,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ltow_s,(long int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix),ltow_s,(__val,__dst,__maxlen,__radix))
#else
__LIBC __PORT_DOSONLY wchar_t *(__LIBCCALL _ultow)(unsigned long int __val, wchar_t *__restrict __dst, int __radix);
__LIBC __PORT_DOSONLY wchar_t *(__LIBCCALL _ltow)(long int __val, wchar_t *__restrict __dst, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ultow_s)(unsigned long int __val, wchar_t *__dst, size_t __maxlen, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ltow_s)(long int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix);
#endif /* __SIZEOF_LONG__ != 8 */
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_itow,(int __val, wchar_t *__restrict __dst, int __radix),itow,(__val,__dst,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_itow_s,(int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix),itow_s,(__val,__dst,__maxlen,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wgetenv,(wchar_t const *__restrict __varname),wgetenv,(__varname))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wgetenv_s,(size_t *__restrict __psize, wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __varname),wgetenv_s,(__psize,__buf,__buflen,__varname))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wdupenv_s,(wchar_t **__restrict __pbuf, size_t *__restrict __pbuflen, wchar_t const *__restrict __varname),wdupenv_s,(__pbuf,__pbuflen,__varname))
#endif /* __CRT_DOS */

__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,long int,__LIBCCALL,_wcstol_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstol_l,(__s,__pend,__radix,__locale))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,unsigned long int,__LIBCCALL,_wcstoul_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoul_l,(__s,__pend,__radix,__locale))
#if defined(__PE__) && defined(__CRT_DOS) && __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,,__LONGLONG,__LIBCCALL,_wcstoll_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),_wcstoi64_l,(__s,__pend,__radix,__locale))
__REDIRECT(__LIBC,,__ULONGLONG,__LIBCCALL,_wcstoull_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoui64_l,(__s,__pend,__radix,__locale))
#else
__REDIRECT_IFKOS(__LIBC,,__LONGLONG,__LIBCCALL,_wcstoll_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoll_l,(__s,__pend,__radix,__locale))
__REDIRECT_IFKOS(__LIBC,,__ULONGLONG,__LIBCCALL,_wcstoull_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoull_l,(__s,__pend,__radix,__locale))
#endif
__REDIRECT_IFKOS(__LIBC,,float,__LIBCCALL,_wcstof_l,(wchar_t const *__restrict __s, wchar_t **__restrict __pend, __locale_t __locale),wcstof_l,(__s,__pend,__locale))
__REDIRECT_IFKOS(__LIBC,,double,__LIBCCALL,_wcstod_l,(wchar_t const *__restrict __s, wchar_t **__pend, __locale_t __locale),wcstod_l,(__s,__pend,__locale))
__REDIRECT_IFKOS(__LIBC,,long double,__LIBCCALL,_wcstold_l,(wchar_t const *__restrict __s, wchar_t **__pend, __locale_t __locale),wcstold_l,(__s,__pend,__locale))

#ifdef __CRT_DOS
__REDIRECT_IFKOS(__LIBC,,long int,__LIBCCALL,_wtol,(wchar_t const *__restrict __s),wtol,(__s));
__REDIRECT_IFKOS(__LIBC,,long int,__LIBCCALL,_wtol_l,(wchar_t const *__restrict __s, __locale_t __locale),wtol_l,(__s,__locale));
__REDIRECT2(__LIBC,,__LONGLONG,__LIBCCALL,_wtoll,(wchar_t const *__restrict __s),wtoi64,_wtoi64,(__s));
__REDIRECT2(__LIBC,,__LONGLONG,__LIBCCALL,_wtoll_l,(wchar_t const *__restrict __s, __locale_t __locale),wtoi64_l,_wtoi64_l,(__s,__locale));
#if __SIZEOF_INT__ == __SIZEOF_LONG__
__REDIRECT2(__LIBC,,int,__LIBCCALL,_wtoi,(wchar_t const *__restrict __s),wtol,_wtol,(__s))
__REDIRECT2(__LIBC,,int,__LIBCCALL,_wtoi_l,(wchar_t const *__restrict __s, __locale_t __locale),wtol_l,_wtol_l,(__s,__locale))
#else /* __SIZEOF_INT__ == __SIZEOF_LONG__ */
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_wtoi,(wchar_t const *__restrict __s),wtoi,(__s))
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_wtoi_l,(wchar_t const *__restrict __s, __locale_t __locale),wtoi_l,(__s,__locale))
#endif /* __SIZEOF_INT__ != __SIZEOF_LONG__ */
__REDIRECT_IFKOS(__LIBC,,double,__LIBCCALL,_wtof,(wchar_t const *__restrict __s),wtof,(__s))
__REDIRECT_IFKOS(__LIBC,,double,__LIBCCALL,_wtof_l,(wchar_t const *__restrict __s, __locale_t __locale),wtof_l,(__s,__locale))
#else /* __CRT_DOS */
__LOCAL long int (__LIBCCALL _wtol)(wchar_t const *__restrict __s) { return __NAMESPACE_STD_SYM wcstol(__s,0,10); }
__LOCAL long int (__LIBCCALL _wtol_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstol_l(__s,0,10,__locale); }
__LOCAL __LONGLONG (__LIBCCALL _wtoll)(wchar_t const *__restrict __s) { return __NAMESPACE_STD_SYM wcstoll(__s,0,10); }
__LOCAL __LONGLONG (__LIBCCALL _wtoll_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoll_l(__s,0,10,__locale); }
__LOCAL int (__LIBCCALL _wtoi)(wchar_t const *__restrict __s) { return (int)__NAMESPACE_STD_SYM wcstol(__s,0,10); }
__LOCAL int (__LIBCCALL _wtoi_l)(wchar_t const *__restrict __s, __locale_t __locale) { return (int)_wcstol_l(__s,0,10,__locale); }
__LOCAL double (__LIBCCALL _wtof)(wchar_t const *__restrict __s) { return __NAMESPACE_STD_SYM wcstod(__s,0); }
__LOCAL double (__LIBCCALL _wtof_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstod_l(__s,0,__locale); }
#endif /* !__CRT_DOS */

#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstol,(__s,__pend,__radix))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstoul,(__s,__pend,__radix))
__REDIRECT2(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstol_l,_wcstol_l,(__s,__pend,__radix,__locale))
__REDIRECT2(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64_l,(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale),wcstoul_l,_wcstoul_l,(__s,__pend,__radix,__locale))
#ifdef __CRT_DOS
__REDIRECT2(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wtoi64,(wchar_t const *__restrict __s),wtol,_wtol,(__s))
__REDIRECT2(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wtoi64_l,(wchar_t const *__restrict __s, __locale_t __locale),wtol_l,_wtol_l,(__s))
#else /* __CRT_DOS */
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) { return _wcstoi64(__s,0,10); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoi64_l(__s,0,10,__locale); }
#endif /* !__CRT_DOS */
#elif __SIZEOF_LONG_LONG__ == 8
__REDIRECT_IFKOS(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstoll,(__s,__pend,__radix))
__REDIRECT_IFKOS(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstoull,(__s,__pend,__radix))
__REDIRECT_IFKOS(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoll_l,(__s,__pend,__radix,__locale))
__REDIRECT_IFKOS(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64_l,(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale),wcstoull_l,(__s,__pend,__radix,__locale))
#ifdef __CRT_DOS
__REDIRECT_IFW32(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wtoi64,(wchar_t const *__restrict __s),wtoi64,(__s))
__REDIRECT_IFW32(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wtoi64_l,(wchar_t const *__restrict __s, __locale_t __locale),wtoi64_l,(__s,__locale))
#else /* __CRT_DOS */
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) { return _wcstoi64(__s,0,10); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoi64_l(__s,0,10,__locale); }
#endif /* !__CRT_DOS */
#elif defined(__CRT_DOS)
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale);
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale);
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s);
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale);
#else
__LOCAL __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) { return (__INT64_TYPE__)__NAMESPACE_STD_SYM wcstoll(__s,__pend,__radix); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) { return (__INT64_TYPE__)_wcstoll_l(__s,__pend,__radix,__locale); }
__LOCAL __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) { return (__UINT64_TYPE__)__NAMESPACE_STD_SYM wcstoull(__s,__pend,__radix); }
__LOCAL __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale) { return (__UINT64_TYPE__)_wcstoull_l(__s,__pend,__radix,__locale); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) { return _wcstoi64(__s,0,10); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoi64_l(__s,0,10,__locale); }
#endif
#endif /* !_WSTDLIB_DEFINED */

#ifdef __CRT_DOS
#ifndef _WSTDLIBP_DEFINED
#define _WSTDLIBP_DEFINED 1
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wfullpath,(wchar_t *__restrict __abspath, wchar_t const *__restrict __path, size_t __maxlen),wfullpath,(__abspath,__path,__maxlen))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wputenv,(wchar_t const *__restrict __envstr),wputenv,(__envstr))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,void,__LIBCCALL,_wmakepath,(wchar_t *__restrict __dst, wchar_t const *__drive, wchar_t const *__dir, wchar_t const *__file, wchar_t const *__ext),wmakepath,(__dst,__drive,__dir,__file,__ext))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,void,__LIBCCALL,_wsearchenv,(wchar_t const *__file, wchar_t const *__varname, wchar_t *__restrict __dst),wsearchenv,(__file,__varname,__dst))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,void,__LIBCCALL,_wsplitpath,(wchar_t const *__restrict __abspath, wchar_t *__drive, wchar_t *__dir, wchar_t *__file, wchar_t *__ext),wsplitpath,(__abspath,__drive,__dir,__file,__ext))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wmakepath_s,(wchar_t *__restrict __dst, size_t __maxlen, wchar_t const *__drive, wchar_t const *__dir, wchar_t const *__file, wchar_t const *__ext),wmakepath_s,(__dst,__maxlen,__drive,__dir,__file,__ext))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wputenv_s,(wchar_t const *__name, wchar_t const *__val),wputenv_s,(__name,__val))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wsearchenv_s,(wchar_t const *__file, wchar_t const *__varname, wchar_t *__restrict __dst, size_t __maxlen),wsearchenv_s,(__file,__varname,__dst,__maxlen))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wsplitpath_s,(wchar_t const *__restrict __abspath, wchar_t *__drive, size_t __drivelen, wchar_t *__dir, size_t __dirlen, wchar_t *__file, size_t __filelen, wchar_t *__ext, size_t __extlen),wsplitpath_s,(__abspath,__drive,__drivelen,__dir,__dirlen,__file,__filelen,__ext,__extlen))
#endif /* !_WSTDLIBP_DEFINED */
#endif /* __CRT_DOS */

#ifndef _WSTRING_DEFINED
#define _WSTRING_DEFINED 1
#ifndef ___wcsdup_defined
#define ___wcsdup_defined 1
__REDIRECT_IFKOS(__LIBC,__SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,
                 wchar_t *,__LIBCCALL,_wcsdup,(wchar_t const *__restrict __str),wcsdup,(__str))
#endif /* !___wcsdup_defined */
#ifndef __wcstok_s_defined
#define __wcstok_s_defined 1
__REDIRECT_IFKOS(__LIBC,,wchar_t *,__LIBCCALL,wcstok_s,
                (wchar_t *__restrict __str, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr),
                 wcstok,(__str,__delim,__ptr))
#endif /* !__wcstok_s_defined */

#ifdef __CRT_DOS
#ifndef __wcserror_defined
#define __wcserror_defined 1
__LIBC __PORT_DOSONLY_ALT(strerror) wchar_t *(__LIBCCALL _wcserror)(int __errnum);
__LIBC __PORT_DOSONLY_ALT(strerror) wchar_t *(__LIBCCALL __wcserror)(wchar_t const *__restrict __str);
__LIBC __PORT_DOSONLY_ALT(strerror) errno_t (__LIBCCALL _wcserror_s)(wchar_t *__restrict __buf, size_t __max_chars, int __errnum);
__LIBC __PORT_DOSONLY_ALT(strerror) errno_t (__LIBCCALL __wcserror_s)(wchar_t *__restrict __buf, size_t __max_chars, wchar_t const *__restrict __errmsg);
#endif /* !__wcserror_defined */

#ifndef ___wcsset_s_defined
#define ___wcsset_s_defined 1
/* TODO: These could be implemented as inline */
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wcsset_s,(wchar_t *__restrict __str, size_t __maxlen, wchar_t __val),wcsset_s,(__str,__maxlen,__val))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wcsnset_s,(wchar_t *__restrict __str, size_t __buflen, wchar_t __val, size_t __maxlen),wcsnset_s,(__str,__buflen,__val,__maxlen))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wcslwr_s,(wchar_t *__restrict __str, size_t __maxlen),wcslwr_s,(__str,__maxlen))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wcsupr_s,(wchar_t *__restrict __str, size_t __maxlen),wcsupr_s,(__str,__maxlen))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wcslwr_s_l,(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale),wcslwr_s_l,(__str,__maxlen,__locale))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wcsupr_s_l,(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale),wcsupr_s_l,(__str,__maxlen,__locale))
#endif /* !___wcsset_s_defined */
#endif /* __CRT_DOS */

#ifndef ___wcsicmp_defined
#define ___wcsicmp_defined 1
__REDIRECT2(__LIBC,__WUNUSED,int,__LIBCCALL,wcsicmp,(wchar_t const *__str1, wchar_t const *__str2),wcscasecmp,_wcsicmp,(__str1,__str2))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcsicmp,(wchar_t const *__str1, wchar_t const *__str2),wcscasecmp,(__str1,__str2))
__REDIRECT2(__LIBC,__WUNUSED,int,__LIBCCALL,wcsnicmp,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncasecmp,_wcsnicmp,(__str1,__str2,__max_chars))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcsnicmp,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncasecmp,(__str1,__str2,__max_chars))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcsicmp_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),wcscasecmp_l,(__str1,__str2,__locale))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcsnicmp_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncasecmp_l,(__str1,__str2,__max_chars,__locale))
#endif /* !___wcsicmp_defined */

#ifndef __wcsrev_defined
#define __wcsrev_defined 1
__REDIRECT_IFDOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsrev,(wchar_t *__restrict __str),_wcsrev,(__str))
__REDIRECT_IFDOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsset,(wchar_t *__restrict __str, wchar_t __needle),_wcsset,(__str,__needle))
__REDIRECT_IFDOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsnset,(wchar_t *__restrict __str, wchar_t __needle, size_t __max_chars),_wcsnset,(__str,__needle,__max_chars))
__REDIRECT_IFDOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcslwr,(wchar_t *__restrict __str),_wcslwr,(__str))
__REDIRECT_IFDOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsupr,(wchar_t *__restrict __str),_wcsupr,(__str))
__REDIRECT_IFKOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcsrev,(wchar_t *__restrict __str),wcsrev,(__str))
__REDIRECT_IFKOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcsset,(wchar_t *__restrict __str, wchar_t __char),wcsset,(__str,__char))
__REDIRECT_IFKOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcsnset,(wchar_t *__restrict __str, wchar_t __char, size_t __max_chars),wcsnset,(__str,__char,__max_chars))
__REDIRECT_IFKOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcslwr,(wchar_t *__restrict __str),wcslwr,(__str))
__REDIRECT_IFKOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcslwr_l,(wchar_t *__restrict __str, __locale_t __locale),wcslwr_l,(__str,__locale))
__REDIRECT_IFKOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcsupr,(wchar_t *__restrict __str),wcsupr,(__str))
__REDIRECT_IFKOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcsupr_l,(wchar_t *__restrict __str, __locale_t __locale),wcsupr_l,(__str,__locale))
#endif /* !__wcsrev_defined */

#ifndef __wcsicoll_defined
#define __wcsicoll_defined 1
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcscoll_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),wcscoll_l,(__str1,__str2,__locale))
__REDIRECT2(__LIBC,__WUNUSED,int,__LIBCCALL,wcsicoll,(wchar_t const *__str1, wchar_t const *__str2),wcscasecoll,_wcsicoll,(__str1,__str2))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcsicoll,(wchar_t const *__str1, wchar_t const *__str2),wcscasecoll,(__str1,__str2))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcsicoll_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),wcscasecoll_l,(__str1,__str2,__locale))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcsncoll,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncoll,(__str1,__str2,__max_chars))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcsncoll_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncoll_l,(__str1,__str2,__max_chars,__locale))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcsnicoll,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncasecoll,(__str1,__str2,__max_chars))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,int,__LIBCCALL,_wcsnicoll_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncasecoll_l,(__str1,__str2,__max_chars,__locale))
#endif /* !__wcsicoll_defined */

#ifndef ___wcsxfrm_l_defined
#define ___wcsxfrm_l_defined 1
__REDIRECT_IFKOS(__LIBC,,size_t,__LIBCCALL,_wcsxfrm_l,(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __max_chars, __locale_t __locale),wcsxfrm_l,(__dst,__src,__max_chars,__locale))
#endif /* !___wcsxfrm_l_defined */

#ifdef __USE_DOS_SLIB
#ifdef __CRT_DOS
#ifndef __wcsncat_s_defined
#define __wcsncat_s_defined 1
/* TODO: These could be implemented as inline */
__LIBC errno_t (__LIBCCALL wcscat_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src);
__LIBC errno_t (__LIBCCALL wcscpy_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src);
__LIBC errno_t (__LIBCCALL wcsncat_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __maxlen);
__LIBC errno_t (__LIBCCALL wcsncpy_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __maxlen);
#endif /* !__wcsncat_s_defined */
#endif /* __CRT_DOS */

#ifndef __wcsnlen_s_defined
#define __wcsnlen_s_defined 1
__LOCAL size_t (__LIBCCALL wcsnlen_s)(wchar_t const *__restrict __src, size_t __max_chars) { return __src ? wcsnlen(__src,__max_chars) : 0; }
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
#if !defined(__DOS_COMPAT__) && defined(__CRT_GLC)
#if defined(__USE_MISC) && !defined(__USE_DOS)
    long int    tm_gmtoff;   /* Seconds east of UTC.  */
    char const *tm_zone;     /* Timezone abbreviation.  */
#else
    long int    __tm_gmtoff; /* Seconds east of UTC.  */
    char const *__tm_zone;   /* Timezone abbreviation.  */
#endif
#endif /* !... */
};
__NAMESPACE_STD_END
#endif /* !__std_tm_defined */

#ifndef _WTIME_DEFINED
#define _WTIME_DEFINED 1
#ifndef __time_t_defined
#define __time_t_defined 1
typedef __TM_TYPE(time) time_t;
#endif /* !__time_t_defined */

__REDIRECT_IFKOS(__LIBC,,size_t,__LIBCCALL,_wcsftime_l,(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, struct tm const *__restrict __ptm, __locale_t __locale),wcsftime_l,(__buf,__maxlen,__format,__ptm,__locale))
#ifdef __CRT_DOS
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wasctime_s,(wchar_t __buf[26], size_t __maxlen, struct tm const *__restrict __ptm),wasctime_s,(__buf,__maxlen,__ptm))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wstrdate_s,(wchar_t __buf[9], size_t __maxlen),wstrdate_s,(__buf,__maxlen))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wstrtime_s,(wchar_t __buf[9], size_t __maxlen),wstrtime_s,(__buf,__maxlen))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wasctime,(struct tm const *__restrict __ptm),wasctime,(__ptm))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wstrdate,(wchar_t *__restrict __buf),wstrdate,(__buf))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wstrtime,(wchar_t *__restrict __buf),wstrtime,(__buf))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wctime32,(__time32_t const *__restrict __timer),wctime32,(__timer))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wctime64,(__time64_t const *__restrict __timer),wctime64,(__timer))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctime32_s,(wchar_t __buf[26], size_t __maxlen, __time32_t const *__timer),wctime32_s,(__buf,__maxlen,__timer))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctime64_s,(wchar_t __buf[26], size_t __maxlen, __time64_t const *__timer),wctime64_s,(__buf,__maxlen,__timer))
#ifdef __USE_TIME_BITS64
__REDIRECT2(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wctime,(time_t const *__restrict __timer),wctime64,_wctime64,(__timer))
__REDIRECT2(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctime_s,(wchar_t *__restrict __buf, size_t __maxlen, time_t const *__restrict __timer),wctime64_s,_wctime64_s,(__buf,__maxlen,__timer))
#else /* __USE_TIME_BITS64 */
__REDIRECT2(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wctime,(time_t const *__restrict __timer),wctime32,_wctime32,(__timer))
__REDIRECT2(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctime_s,(wchar_t *__restrict __buf, size_t __maxlen, time_t const *__restrict __timer),wctime32_s,_wctime32_s,(__buf,__maxlen,__timer))
#endif /* !__USE_TIME_BITS64 */
#endif /* __CRT_DOS */

#ifndef __wcsftime_defined
#define __wcsftime_defined 1
__NAMESPACE_STD_BEGIN
__LIBC size_t (__LIBCCALL wcsftime)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, struct tm const *__restrict __ptm);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcsftime)
#endif /* !__wcsftime_defined */
#endif /* !_WTIME_DEFINED */

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL mbsrtowcs_s)(size_t *__result, wchar_t *__restrict __buf, size_t __buflen, char const **__restrict __psrc, size_t __srcsize, mbstate_t *__restrict __ps);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wcsrtombs_s)(size_t *__result, char *__restrict __buf, size_t __buflen, wchar_t const **__restrict __psrc, size_t __srcsize, mbstate_t *__restrict __ps);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wcrtomb_s)(size_t *__result, char *__restrict __buf, size_t __buflen, wchar_t __wc, mbstate_t *__restrict __ps);
#endif /* __CRT_DOS */

#ifndef __memcpy_defined
#define __memcpy_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(memcpy)
#endif /* !__memcpy_defined */
#ifndef __memmove_defined
#define __memmove_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memmove)(void *__dst, void const *__src, size_t __n_bytes);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(memmove)
#endif /* !__memmove_defined */

#ifdef __USE_DOS_SLIB
/* TODO: These could be implemented inline. */
#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL memcpy_s)(void *__restrict __dst, rsize_t __dstsize, void const *__restrict __src, rsize_t __srcsize);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL memmove_s)(void *__dst, rsize_t __dstsize, void const *__src, rsize_t __srcsize);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wmemcpy_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __srcsize);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wmemmove_s)(wchar_t *__dst, rsize_t __dstsize, wchar_t const *__src, rsize_t __srcsize);
#endif /* __CRT_DOS */
#endif /* __USE_DOS_SLIB */
#endif /* __USE_DOS */

__SYSDECL_END

#endif /* !_WCHAR_H */
