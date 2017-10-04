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
#ifdef __USE_UTF
#include <xlocale.h>
#include <__malldefs.h>
#endif /* __USE_UTF */

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

#ifndef __MBSTATE_INIT
#define __MBSTATE_INIT     {0,{0}}
#endif /* !__MBSTATE_INIT */

#ifdef __USE_KOS
#ifndef MBSTATE_INIT
#define MBSTATE_INIT     __MBSTATE_INIT
#endif /* !MBSTATE_INIT */
#endif /* __USE_KOS */


#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */


/* Libc uses utf16/utf32 to encode/decode char16_t and char32_t */
#define __STD_UTF_16__ 1
#define __STD_UTF_32__ 1

#ifndef __KERNEL__
__LIBC size_t __NOTHROW((__LIBCCALL mbrtoc16)(char16_t *__restrict __pc16, char const *__restrict __s, size_t __n, mbstate_t *__restrict __p));
__LIBC size_t __NOTHROW((__LIBCCALL mbrtoc32)(char32_t *__restrict __pc32, char const *__restrict __s, size_t __n, mbstate_t *__restrict __p));
__LIBC size_t __NOTHROW((__LIBCCALL c16rtomb)(char *__restrict __s, char16_t __c16, mbstate_t *__restrict __ps));
__LIBC size_t __NOTHROW((__LIBCCALL c32rtomb)(char *__restrict __s, char32_t __c32, mbstate_t *__restrict __ps));

/* UTF-16/32 string functions. */
#ifdef __USE_UTF

#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_BEGIN struct tm;
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */

__LIBC size_t (__LIBCCALL mbr16len)(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps) __C16_DECL(mbrlen);
__LIBC size_t (__LIBCCALL mbr32len)(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps) __C32_DECL(mbrlen);
__LIBC __NONNULL((1,2)) char16_t *(__LIBCCALL c16cpy)(char16_t *__restrict __dst, char16_t const *__restrict __src) __C16_FUNC(cpy);
__LIBC __NONNULL((1,2)) char32_t *(__LIBCCALL c32cpy)(char32_t *__restrict __dst, char32_t const *__restrict __src) __C32_FUNC(cpy);
__LIBC __NONNULL((1,2)) char16_t *(__LIBCCALL c16ncpy)(char16_t *__restrict __dst, char16_t const *__restrict __src, size_t __n) __C16_FUNC(ncpy);
__LIBC __NONNULL((1,2)) char32_t *(__LIBCCALL c32ncpy)(char32_t *__restrict __dst, char32_t const *__restrict __src, size_t __n) __C32_FUNC(ncpy);
__LIBC __NONNULL((1,2)) char16_t *(__LIBCCALL c16cat)(char16_t *__restrict __dst, char16_t const *__restrict __src) __C16_FUNC(cat);
__LIBC __NONNULL((1,2)) char32_t *(__LIBCCALL c32cat)(char32_t *__restrict __dst, char32_t const *__restrict __src) __C32_FUNC(cat);
__LIBC __NONNULL((1,2)) char16_t *(__LIBCCALL c16ncat)(char16_t *__restrict __dst, char16_t const *__restrict __src, size_t __n) __C16_FUNC(ncat);
__LIBC __NONNULL((1,2)) char32_t *(__LIBCCALL c32ncat)(char32_t *__restrict __dst, char32_t const *__restrict __src, size_t __n) __C32_FUNC(ncat);
__LIBC __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL c16cmp)(char16_t const *__s1, char16_t const *__s2) __C16_FUNC(cmp);
__LIBC __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL c32cmp)(char32_t const *__s1, char32_t const *__s2) __C32_FUNC(cmp);
__LIBC __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL c16ncmp)(char16_t const *__s1, char16_t const *__s2, size_t __n) __C16_FUNC(ncmp);
__LIBC __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL c32ncmp)(char32_t const *__s1, char32_t const *__s2, size_t __n) __C32_FUNC(ncmp);
__LIBC int (__LIBCCALL c16coll)(char16_t const *__s1, char16_t const *__s2) __C16_FUNC(coll);
__LIBC int (__LIBCCALL c32coll)(char32_t const *__s1, char32_t const *__s2) __C32_FUNC(coll);
__LIBC size_t (__LIBCCALL c16xfrm)(char16_t *__restrict __s1, char16_t const *__restrict __s2, size_t __n) __C16_FUNC(xfrm);
__LIBC size_t (__LIBCCALL c32xfrm)(char32_t *__restrict __s1, char32_t const *__restrict __s2, size_t __n) __C32_FUNC(xfrm);
__LIBC size_t (__LIBCCALL mbsrtoc16)(char16_t *__restrict __dst, char const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps) __C16_DECL(mbsrtowcs);
__LIBC size_t (__LIBCCALL mbsrtoc32)(char32_t *__restrict __dst, char const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps) __C32_DECL(mbsrtowcs);
__LIBC size_t (__LIBCCALL c16rtombs)(char *__restrict __dst, char16_t const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps) __C16_FUNC(rtombs);
__LIBC size_t (__LIBCCALL c32rtombs)(char *__restrict __dst, char32_t const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps) __C32_FUNC(rtombs);
__LIBC double (__LIBCCALL c16tod)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr) __C16_FUNC(tod);
__LIBC double (__LIBCCALL c32tod)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr) __C32_FUNC(tod);
__LIBC long int (__LIBCCALL c16tol)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base) __C16_FUNC(tol);
__LIBC long int (__LIBCCALL c32tol)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base) __C32_FUNC(tol);
__LIBC unsigned long int (__LIBCCALL c16toul)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base) __C16_FUNC(toul);
__LIBC unsigned long int (__LIBCCALL c32toul)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base) __C32_FUNC(toul);
__LIBC wint_t (__LIBCCALL fgetc16)(__FILE *__stream) __C16_DECL(fgetwc);
__LIBC wint_t (__LIBCCALL fgetc32)(__FILE *__stream) __C32_DECL(fgetwc);
__LIBC wint_t (__LIBCCALL getc16)(__FILE *__stream) __C16_DECL(getwc);
__LIBC wint_t (__LIBCCALL getc32)(__FILE *__stream) __C32_DECL(getwc);
__LIBC wint_t (__LIBCCALL getc16char)(void) __C16_DECL(getwchar);
__LIBC wint_t (__LIBCCALL getc32char)(void) __C32_DECL(getwchar);
__LIBC wint_t (__LIBCCALL fputc16)(char16_t __wc, __FILE *__stream) __C16_DECL(fputwc);
__LIBC wint_t (__LIBCCALL fputc32)(char32_t __wc, __FILE *__stream) __C32_DECL(fputwc);
__LIBC wint_t (__LIBCCALL putc16)(char16_t __wc, __FILE *__stream) __C16_DECL(putwc);
__LIBC wint_t (__LIBCCALL putc32)(char32_t __wc, __FILE *__stream) __C32_DECL(putwc);
__LIBC wint_t (__LIBCCALL putc16char)(char16_t __wc) __C16_DECL(putwchar);
__LIBC wint_t (__LIBCCALL putc32char)(char32_t __wc) __C32_DECL(putwchar);
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC char16_t *(__LIBCCALL fgetc16s)(char16_t *__restrict __ws, size_t __n, __FILE *__restrict __stream) __C16_DECL(fgetws);
__LIBC char32_t *(__LIBCCALL fgetc32s)(char32_t *__restrict __ws, size_t __n, __FILE *__restrict __stream) __C32_DECL(fgetws);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__LIBC char16_t *(__LIBCCALL fgetc16s)(char16_t *__restrict __ws, size_t __n, __FILE *__restrict __stream) __ASMNAME("_fgetws_sz");
__LIBC char32_t *(__LIBCCALL fgetc32s)(char32_t *__restrict __ws, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgetws_sz");
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__LIBC char16_t *(__LIBCCALL fgetc16s)(char16_t *__restrict __ws, int __n, __FILE *__restrict __stream) __C16_DECL(fgetws);
__LIBC char32_t *(__LIBCCALL fgetc32s)(char32_t *__restrict __ws, int __n, __FILE *__restrict __stream) __C32_DECL(fgetws);
#endif /* !__USE_KOS */
__LIBC int (__LIBCCALL fputc16s)(char16_t const *__restrict __ws, __FILE *__restrict __stream) __C16_DECL(fputws);
__LIBC int (__LIBCCALL fputc32s)(char32_t const *__restrict __ws, __FILE *__restrict __stream) __C32_DECL(fputws);
__LIBC wint_t (__LIBCCALL ungetc16)(wint_t __wc, __FILE *__stream) __C16_DECL(ungetwc);
__LIBC wint_t (__LIBCCALL ungetc32)(wint_t __wc, __FILE *__stream) __C32_DECL(ungetwc);
__LIBC size_t (__LIBCCALL c16ftime)(char16_t *__restrict __s, size_t __maxsize, char16_t const *__restrict __format, struct tm const *__restrict __tp) __C16_FUNC(ftime);
__LIBC size_t (__LIBCCALL c32ftime)(char32_t *__restrict __s, size_t __maxsize, char32_t const *__restrict __format, struct tm const *__restrict __tp) __C32_FUNC(ftime);
__LIBC char16_t *(__LIBCCALL c16tok)(char16_t *__restrict __s, char16_t const *__restrict __delim, char16_t **__restrict __ptr) __ASMNAME("wcstok_s");
__LIBC char32_t *(__LIBCCALL c32tok)(char32_t *__restrict __s, char32_t const *__restrict __delim, char32_t **__restrict __ptr) __ASMNAME("wcstok");
__LIBC size_t (__LIBCCALL c16len)(char16_t const *__s) __C16_FUNC(len);
__LIBC __ATTR_PURE size_t (__LIBCCALL c16spn)(char16_t const *__haystack, char16_t const *__accept) __C16_FUNC(spn);
__LIBC __ATTR_PURE size_t (__LIBCCALL c16cspn)(char16_t const *__haystack, char16_t const *__reject) __C16_FUNC(cspn);
__LIBC __ATTR_PURE int (__LIBCCALL umemcmp)(char16_t const *__s1, char16_t const *__s2, size_t __n) __ASMNAME("memcmpw");
__LIBC __ATTR_PURE int (__LIBCCALL Umemcmp)(char32_t const *__s1, char32_t const *__s2, size_t __n) __ASMNAME("memcmpl");
__LIBC char16_t *(__LIBCCALL umemcpy)(char16_t *__restrict __s1, char16_t const *__restrict __s2, size_t __n) __ASMNAME("memcpyw");
__LIBC char32_t *(__LIBCCALL Umemcpy)(char32_t *__restrict __s1, char32_t const *__restrict __s2, size_t __n) __ASMNAME("memcpyw");
__LIBC char16_t *(__LIBCCALL umemmove)(char16_t *__s1, char16_t const *__s2, size_t __n) __ASMNAME("memmovew");
__LIBC char32_t *(__LIBCCALL Umemmove)(char32_t *__s1, char32_t const *__s2, size_t __n) __ASMNAME("memmovew");
__LIBC char16_t *(__LIBCCALL umemset)(char16_t *__s, char16_t __c, size_t __n) __ASMNAME("memsetw");
__LIBC char32_t *(__LIBCCALL Umemset)(char32_t *__s, char32_t __c, size_t __n) __ASMNAME("memsetw");

#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16chr)(char16_t *__haystack, char16_t __wc) __C16_FUNC(chr);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32chr)(char32_t *__haystack, char32_t __wc) __C32_FUNC(chr);
__LIBC __ATTR_PURE char16_t const *(__LIBCCALL c16chr)(char16_t const *__haystack, char16_t __wc) __C16_FUNC(chr);
__LIBC __ATTR_PURE char32_t const *(__LIBCCALL c32chr)(char32_t const *__haystack, char32_t __wc) __C32_FUNC(chr);
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16rchr)(char16_t *__haystack, char16_t __wc) __C16_FUNC(rchr);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32rchr)(char32_t *__haystack, char32_t __wc) __C32_FUNC(rchr);
__LIBC __ATTR_PURE char16_t const *(__LIBCCALL c16rchr)(char16_t const *__haystack, char16_t __wc) __C16_FUNC(rchr);
__LIBC __ATTR_PURE char32_t const *(__LIBCCALL c32rchr)(char32_t const *__haystack, char32_t __wc) __C32_FUNC(rchr);
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16pbrk)(char16_t *__haystack, char16_t const *__accept) __C16_FUNC(pbrk);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32pbrk)(char32_t *__haystack, char32_t const *__accept) __C32_FUNC(pbrk);
__LIBC __ATTR_PURE char16_t const *(__LIBCCALL c16pbrk)(char16_t const *__haystack, char16_t const *__accept) __C16_FUNC(pbrk);
__LIBC __ATTR_PURE char32_t const *(__LIBCCALL c32pbrk)(char32_t const *__haystack, char32_t const *__accept) __C32_FUNC(pbrk);
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16str)(char16_t *__haystack, char16_t const *__needle) __C16_FUNC(str);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32str)(char32_t *__haystack, char32_t const *__needle) __C32_FUNC(str);
__LIBC __ATTR_PURE char16_t const *(__LIBCCALL c16str)(char16_t const *__haystack, char16_t const *__needle) __C16_FUNC(str);
__LIBC __ATTR_PURE char32_t const *(__LIBCCALL c32str)(char32_t const *__haystack, char32_t const *__needle) __C32_FUNC(str);
__LIBC __ATTR_PURE char16_t *(__LIBCCALL umemchr)(char16_t *__s, char16_t __c, size_t __n) __ASMNAME("memchrw");
__LIBC __ATTR_PURE char32_t *(__LIBCCALL Umemchr)(char32_t *__s, char32_t __c, size_t __n) __ASMNAME("memchrl");
__LIBC __ATTR_PURE char16_t const *(__LIBCCALL umemchr)(char16_t const *__s, char16_t __c, size_t __n) __ASMNAME("memchrw");
__LIBC __ATTR_PURE char32_t const *(__LIBCCALL Umemchr)(char32_t const *__s, char32_t __c, size_t __n) __ASMNAME("memchrl");
}
#else
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16chr)(char16_t const *__haystack, char16_t __wc) __C16_FUNC(chr);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32chr)(char32_t const *__haystack, char32_t __wc) __C32_FUNC(chr);
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16rchr)(char16_t const *__haystack, char16_t __wc) __C16_FUNC(rchr);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32rchr)(char32_t const *__haystack, char32_t __wc) __C32_FUNC(rchr);
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16pbrk)(char16_t const *__haystack, char16_t const *__accept) __C16_FUNC(pbrk);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32pbrk)(char32_t const *__haystack, char32_t const *__accept) __C32_FUNC(pbrk);
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16str)(char16_t const *__haystack, char16_t const *__needle) __C16_FUNC(str);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32str)(char32_t const *__haystack, char32_t const *__needle) __C32_FUNC(str);
__LIBC __ATTR_PURE char16_t *(__LIBCCALL umemchr)(char16_t const *__s, char16_t __c, size_t __n) __ASMNAME("memchrw");
__LIBC __ATTR_PURE char32_t *(__LIBCCALL Umemchr)(char32_t const *__s, char32_t __c, size_t __n) __ASMNAME("memchrl");
#endif
#if defined(__USE_ISOC95) || defined(__USE_UNIX98)
#ifdef __USE_KOS
__LIBC ssize_t (__LIBCCALL fuprintf)(__FILE *__restrict __stream, char16_t const *__restrict __format, ...) __C16_DECL(fwprintf);
__LIBC ssize_t (__LIBCCALL fUprintf)(__FILE *__restrict __stream, char32_t const *__restrict __format, ...) __C32_DECL(fwprintf);
__LIBC ssize_t (__LIBCCALL uprintf)(char16_t const *__restrict __format, ...) __C16_DECL(wprintf);
__LIBC ssize_t (__LIBCCALL Uprintf)(char32_t const *__restrict __format, ...) __C32_DECL(wprintf);
__LIBC ssize_t (__LIBCCALL suprintf)(char16_t *__restrict __s, size_t __n, char16_t const *__restrict __format, ...) __ASMNAME("_swprintf_c");
__LIBC ssize_t (__LIBCCALL sUprintf)(char32_t *__restrict __s, size_t __n, char32_t const *__restrict __format, ...) __ASMNAME("swprintf");
__LIBC ssize_t (__LIBCCALL vfuprintf)(__FILE *__restrict __s, char16_t const *__restrict __format, __VA_LIST __arg) __C16_DECL(vfwprintf);
__LIBC ssize_t (__LIBCCALL vfUprintf)(__FILE *__restrict __s, char32_t const *__restrict __format, __VA_LIST __arg) __C32_DECL(vfwprintf);
__LIBC ssize_t (__LIBCCALL vuprintf)(char16_t const *__restrict __format, __VA_LIST __arg) __C16_DECL(vwprintf);
__LIBC ssize_t (__LIBCCALL vUprintf)(char32_t const *__restrict __format, __VA_LIST __arg) __C32_DECL(vwprintf);
__LIBC ssize_t (__LIBCCALL vsuprintf)(char16_t *__restrict __s, size_t __n, char16_t const *__restrict __format, __VA_LIST __arg) __ASMNAME("_vswprintf_c");
__LIBC ssize_t (__LIBCCALL vsUprintf)(char32_t *__restrict __s, size_t __n, char32_t const *__restrict __format, __VA_LIST __arg) __ASMNAME("vswprintf");
__LIBC ssize_t (__LIBCCALL fuscanf)(__FILE *__restrict __stream, char16_t const *__restrict __format, ...) __C16_DECL(fwscanf);
__LIBC ssize_t (__LIBCCALL fUscanf)(__FILE *__restrict __stream, char32_t const *__restrict __format, ...) __C32_DECL(fwscanf);
__LIBC ssize_t (__LIBCCALL uscanf)(char16_t const *__restrict __format, ...) __C16_DECL(wscanf);
__LIBC ssize_t (__LIBCCALL Uscanf)(char32_t const *__restrict __format, ...) __C32_DECL(wscanf);
__LIBC ssize_t (__LIBCCALL suscanf)(char16_t const *__restrict __s, char16_t const *__restrict __format, ...) __C16_DECL(swscanf);
__LIBC ssize_t (__LIBCCALL sUscanf)(char32_t const *__restrict __s, char32_t const *__restrict __format, ...) __C32_DECL(swscanf);
#else /* __USE_KOS */
__LIBC int (__LIBCCALL fuprintf)(__FILE *__restrict __stream, char16_t const *__restrict __format, ...) __C16_DECL(fwprintf);
__LIBC int (__LIBCCALL fUprintf)(__FILE *__restrict __stream, char32_t const *__restrict __format, ...) __C32_DECL(fwprintf);
__LIBC int (__LIBCCALL uprintf)(char16_t const *__restrict __format, ...) __C16_DECL(wprintf);
__LIBC int (__LIBCCALL Uprintf)(char32_t const *__restrict __format, ...) __C32_DECL(wprintf);
__LIBC int (__LIBCCALL suprintf)(char16_t *__restrict __s, size_t __n, char16_t const *__restrict __format, ...) __ASMNAME("_swprintf_c");
__LIBC int (__LIBCCALL sUprintf)(char32_t *__restrict __s, size_t __n, char32_t const *__restrict __format, ...) __ASMNAME("swprintf");
__LIBC int (__LIBCCALL vfuprintf)(__FILE *__restrict __s, char16_t const *__restrict __format, __VA_LIST __arg) __C16_DECL(vfwprintf);
__LIBC int (__LIBCCALL vfUprintf)(__FILE *__restrict __s, char32_t const *__restrict __format, __VA_LIST __arg) __C32_DECL(vfwprintf);
__LIBC int (__LIBCCALL vuprintf)(char16_t const *__restrict __format, __VA_LIST __arg) __C16_DECL(vwprintf);
__LIBC int (__LIBCCALL vUprintf)(char32_t const *__restrict __format, __VA_LIST __arg) __C32_DECL(vwprintf);
__LIBC int (__LIBCCALL vsuprintf)(char16_t *__restrict __s, size_t __n, char16_t const *__restrict __format, __VA_LIST __arg) __ASMNAME("_vswprintf_c");
__LIBC int (__LIBCCALL vsUprintf)(char32_t *__restrict __s, size_t __n, char32_t const *__restrict __format, __VA_LIST __arg) __ASMNAME("vswprintf");
__LIBC int (__LIBCCALL fuscanf)(__FILE *__restrict __stream, char16_t const *__restrict __format, ...) __C16_DECL(fwscanf);
__LIBC int (__LIBCCALL fUscanf)(__FILE *__restrict __stream, char32_t const *__restrict __format, ...) __C32_DECL(fwscanf);
__LIBC int (__LIBCCALL uscanf)(char16_t const *__restrict __format, ...) __C16_DECL(wscanf);
__LIBC int (__LIBCCALL Uscanf)(char32_t const *__restrict __format, ...) __C32_DECL(wscanf);
__LIBC int (__LIBCCALL suscanf)(char16_t const *__restrict __s, char16_t const *__restrict __format, ...) __C16_DECL(swscanf);
__LIBC int (__LIBCCALL sUscanf)(char32_t const *__restrict __s, char32_t const *__restrict __format, ...) __C32_DECL(swscanf);
#endif /* !__USE_KOS */
#endif /* __USE_ISOC95 || __USE_UNIX98 */
#ifdef __USE_ISOC99
__LIBC float (__LIBCCALL c16tof)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr) __C16_FUNC(tof);
__LIBC float (__LIBCCALL c32tof)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr) __C32_FUNC(tof);
#ifdef __PE__
__LIBC long double (__LIBCCALL c16told)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr) __ASMNAME("wcstod");
__LIBC long double (__LIBCCALL c32told)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr) __ASMNAME(".kos.wcstod");
#else /* __PE__ */
__LIBC long double (__LIBCCALL c16told)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr) __ASMNAME(".dos.wcstold96");
__LIBC long double (__LIBCCALL c32told)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr) __ASMNAME("wcstold");
#endif /* !__PE__ */
#if __SIZEOF_LONG_LONG__ == 8
__LIBC __LONGLONG (__LIBCCALL c16toll)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base) __ASMNAME("_wcstoi64");
__LIBC __ULONGLONG (__LIBCCALL c16toull)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base) __ASMNAME("_wcstoui64");
#else
__LIBC __LONGLONG (__LIBCCALL c16toll)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base) __C16_FUNC(toll);
__LIBC __ULONGLONG (__LIBCCALL c16toull)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base) __C16_FUNC(toull);
#endif
__LIBC __LONGLONG (__LIBCCALL c32toll)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base) __C32_FUNC(toll);
__LIBC __ULONGLONG (__LIBCCALL c32toull)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base) __C32_FUNC(toull);
#ifdef __USE_KOS
__LIBC ssize_t (__LIBCCALL vfuscanf)(__FILE *__restrict __s, char16_t const *__restrict __format, __VA_LIST __arg) __C16_DECL(vfwscanf);
__LIBC ssize_t (__LIBCCALL vfUscanf)(__FILE *__restrict __s, char32_t const *__restrict __format, __VA_LIST __arg) __C32_DECL(vfwscanf);
__LIBC ssize_t (__LIBCCALL vuscanf)(char16_t const *__restrict __format, __VA_LIST __arg) __C16_DECL(vwscanf);
__LIBC ssize_t (__LIBCCALL vUscanf)(char32_t const *__restrict __format, __VA_LIST __arg) __C32_DECL(vwscanf);
__LIBC ssize_t (__LIBCCALL vsuscanf)(char16_t const *__restrict __s, char16_t const *__restrict __format, __VA_LIST __arg) __C16_DECL(vswscanf);
__LIBC ssize_t (__LIBCCALL vsUscanf)(char32_t const *__restrict __s, char32_t const *__restrict __format, __VA_LIST __arg) __C32_DECL(vswscanf);
#else /* __USE_KOS */
__LIBC int (__LIBCCALL vfuscanf)(__FILE *__restrict __s, char16_t const *__restrict __format, __VA_LIST __arg) __C16_DECL(vfwscanf);
__LIBC int (__LIBCCALL vfUscanf)(__FILE *__restrict __s, char32_t const *__restrict __format, __VA_LIST __arg) __C32_DECL(vfwscanf);
__LIBC int (__LIBCCALL vuscanf)(char16_t const *__restrict __format, __VA_LIST __arg) __C16_DECL(vwscanf);
__LIBC int (__LIBCCALL vUscanf)(char32_t const *__restrict __format, __VA_LIST __arg) __C32_DECL(vwscanf);
__LIBC int (__LIBCCALL vsuscanf)(char16_t const *__restrict __s, char16_t const *__restrict __format, __VA_LIST __arg) __C16_DECL(vswscanf);
__LIBC int (__LIBCCALL vsUscanf)(char32_t const *__restrict __s, char32_t const *__restrict __format, __VA_LIST __arg) __C32_DECL(vswscanf);
#endif /* !__USE_KOS */
#endif /* __USE_ISOC99 */

#ifdef __USE_XOPEN2K8
__LIBC int (__LIBCCALL c16casecmp)(char16_t const *__s1, char16_t const *__s2) __ASMNAME("_wcsicmp");
__LIBC int (__LIBCCALL c32casecmp)(char32_t const *__s1, char32_t const *__s2) __ASMNAME("wcscasecmp");
__LIBC int (__LIBCCALL c16ncasecmp)(char16_t const *__s1, char16_t const *__s2, size_t __n) __ASMNAME("_wcsnicmp");
__LIBC int (__LIBCCALL c32ncasecmp)(char32_t const *__s1, char32_t const *__s2, size_t __n) __ASMNAME("wcsncasecmp");
__LIBC int (__LIBCCALL c16casecmp_l)(char16_t const *__s1, char16_t const *__s2, __locale_t __loc) __ASMNAME("_wcsicmp_l");
__LIBC int (__LIBCCALL c32casecmp_l)(char32_t const *__s1, char32_t const *__s2, __locale_t __loc) __ASMNAME("wcscasecmp_l");
__LIBC int (__LIBCCALL c16ncasecmp_l)(char16_t const *__s1, char16_t const *__s2, size_t __n, __locale_t __loc) __ASMNAME("_wcsnicmp_l");
__LIBC int (__LIBCCALL c32ncasecmp_l)(char32_t const *__s1, char32_t const *__s2, size_t __n, __locale_t __loc) __ASMNAME("wcsncasecmp_l");
__LIBC int (__LIBCCALL c16coll_l)(char16_t const *__s1, char16_t const *__s2, __locale_t __loc) __ASMNAME("_wcscoll_l");
__LIBC int (__LIBCCALL c32coll_l)(char32_t const *__s1, char32_t const *__s2, __locale_t __loc) __ASMNAME("wcscoll_l");
__LIBC size_t (__LIBCCALL c16sxfrm_l)(char16_t *__s1, char16_t const *__s2, size_t __n, __locale_t __loc) __ASMNAME("_wcsxfrm_l");
__LIBC size_t (__LIBCCALL c32xfrm_l)(char32_t *__s1, char32_t const *__s2, size_t __n, __locale_t __loc) __ASMNAME("wcsxfrm_l");
__LIBC size_t (__LIBCCALL mbsnrtoc16)(char16_t *__restrict __dst, char const **__restrict __src, size_t __nmc, size_t __len, mbstate_t *__restrict __ps) __C16_DECL(mbsnrtowcs);
__LIBC size_t (__LIBCCALL mbsnrtoc32)(char32_t *__restrict __dst, char const **__restrict __src, size_t __nmc, size_t __len, mbstate_t *__restrict __ps) __C32_DECL(mbsnrtowcs);
__LIBC size_t (__LIBCCALL c16nrtombs)(char *__restrict __dst, char16_t const **__restrict __src, size_t __nwc, size_t __len, mbstate_t *__restrict __ps) __C16_DECL(wcsnrtombs);
__LIBC size_t (__LIBCCALL c32nrtombs)(char *__restrict __dst, char32_t const **__restrict __src, size_t __nwc, size_t __len, mbstate_t *__restrict __ps) __C32_DECL(wcsnrtombs);
__LIBC char16_t *(__LIBCCALL c16pcpy)(char16_t *__restrict __dst, char16_t const *__restrict __src) __C16_DECL(wcpcpy);
__LIBC char32_t *(__LIBCCALL c32pcpy)(char32_t *__restrict __dst, char32_t const *__restrict __src) __C32_DECL(wcpcpy);
__LIBC char16_t *(__LIBCCALL c16pncpy)(char16_t *__restrict __dst, char16_t const *__restrict __src, size_t __n) __C16_DECL(wcpncpy);
__LIBC char32_t *(__LIBCCALL c32pncpy)(char32_t *__restrict __dst, char32_t const *__restrict __src, size_t __n) __C32_DECL(wcpncpy);
__LIBC __FILE *(__LIBCCALL open_umemstream)(char16_t **__bufloc, size_t *__sizeloc) __ASMNAME("_open_wmemstream");
__LIBC __FILE *(__LIBCCALL open_Umemstream)(char32_t **__bufloc, size_t *__sizeloc) __ASMNAME("open_wmemstream");
#endif /* __USE_XOPEN2K8 */

#if defined(__USE_XOPEN2K8) || defined(__USE_DOS)
__LIBC __ATTR_PURE size_t (__LIBCCALL c16nlen)(char16_t const *__s, size_t __maxlen) __C16_FUNC(nlen);
__LIBC __ATTR_PURE size_t (__LIBCCALL c32nlen)(char32_t const *__s, size_t __maxlen) __C32_FUNC(nlen);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char16_t *(__LIBCCALL c16dup)(char16_t const *__restrict __s) __ASMNAME("_wcsdup");
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char32_t *(__LIBCCALL c32dup)(char32_t const *__restrict __s) __ASMNAME("wcsdup");
#endif /* __USE_XOPEN2K8 || __USE_DOS */

#ifdef __USE_XOPEN
#ifdef __USE_KOS
__LIBC __ssize_t (__LIBCCALL c16width)(char16_t __c) __C16_DECL(wcwidth);
__LIBC __ssize_t (__LIBCCALL c32width)(char32_t __c) __C32_DECL(wcwidth);
__LIBC __ssize_t (__LIBCCALL c16swidth)(char16_t const *__restrict __s, size_t __n) __C16_DECL(wcswidth);
__LIBC __ssize_t (__LIBCCALL c32swidth)(char32_t const *__restrict __s, size_t __n) __C32_DECL(wcswidth);
#else /* __USE_KOS */
__LIBC int (__LIBCCALL c16width)(char16_t __c) __C16_DECL(wcwidth);
__LIBC int (__LIBCCALL c32width)(char32_t __c) __C32_DECL(wcwidth);
__LIBC int (__LIBCCALL c16swidth)(char16_t const *__restrict __s, size_t __n) __C16_DECL(wcswidth);
__LIBC int (__LIBCCALL c32swidth)(char32_t const *__restrict __s, size_t __n) __C32_DECL(wcswidth);
#endif /* !__USE_KOS */
#endif /* __USE_XOPEN */

#if defined(__USE_XOPEN) || defined(__USE_DOS)
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16c16)(char16_t *__haystack, char16_t const *__needle) __C16_DECL(wcsstr);
__LIBC __ATTR_PURE char16_t const *(__LIBCCALL c16c16)(char16_t const *__haystack, char16_t const *__needle) __C16_DECL(wcsstr);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32c32)(char32_t *__haystack, char32_t const *__needle) __C32_DECL(wcsstr);
__LIBC __ATTR_PURE char32_t const *(__LIBCCALL c32c32)(char32_t const *__haystack, char32_t const *__needle) __C32_DECL(wcsstr);
}
#else
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16c16)(char16_t const *__haystack, char16_t const *__needle) __C16_DECL(wcsstr);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32c32)(char32_t const *__haystack, char32_t const *__needle) __C32_DECL(wcsstr);
#endif
#endif /* __USE_XOPEN || __USE_DOS */

#ifdef __USE_GNU
__LIBC __ATTR_PURE char16_t *(__LIBCCALL c16chrnul)(char16_t const *__s, char16_t __wc) __C16_FUNC(chrnul);
__LIBC __ATTR_PURE char32_t *(__LIBCCALL c32chrnul)(char32_t const *__s, char32_t __wc) __C32_FUNC(chrnul);
__LIBC char16_t *(__LIBCCALL umempcpy)(char16_t *__restrict __s1, char16_t const *__restrict __s2, size_t __n) __ASMNAME("mempcpyw");
__LIBC char32_t *(__LIBCCALL Umempcpy)(char32_t *__restrict __s1, char32_t const *__restrict __s2, size_t __n) __ASMNAME("mempcpyl");
#if __SIZEOF_LONG_LONG__ == 8
__LIBC __LONGLONG (__LIBCCALL c16toll_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_wcstoi64_l");
__LIBC __ULONGLONG (__LIBCCALL c16toull_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_wcstoui64_l");
#else
__LIBC __LONGLONG (__LIBCCALL c16toll_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __C16_FUNC(toll_l);
__LIBC __ULONGLONG (__LIBCCALL c16toull_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __C16_FUNC(toull_l);
#endif
__LIBC __LONGLONG (__LIBCCALL c32toll_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base, __locale_t __loc) __C32_FUNC(toll_l);
__LIBC __ULONGLONG (__LIBCCALL c32toull_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base, __locale_t __loc) __C32_FUNC(toull_l);
__LIBC long int (__LIBCCALL c16tol_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_wcstol_l");
__LIBC long int (__LIBCCALL c32tol_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("wcstol_l");
__LIBC unsigned long int (__LIBCCALL c16toul_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_wcstoul_l");
__LIBC unsigned long int (__LIBCCALL c32toul_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("wcstoul_l");
__LIBC double (__LIBCCALL c16tod_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, __locale_t __loc) __ASMNAME("_wcstod_l");
__LIBC double (__LIBCCALL c32tod_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, __locale_t __loc) __ASMNAME("wcstod_l");
__LIBC float (__LIBCCALL c16tof_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, __locale_t __loc) __ASMNAME("_wcstof_l");
__LIBC float (__LIBCCALL c32tof_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, __locale_t __loc) __ASMNAME("wcstof_l");
#ifdef __PE__
__LIBC long double (__LIBCCALL c16told_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, __locale_t __loc) __ASMNAME("_wcstod_l");
__LIBC long double (__LIBCCALL c32told_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, __locale_t __loc) __ASMNAME("wcstod_l");
#else /* __PE__ */
__LIBC long double (__LIBCCALL c16told_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, __locale_t __loc) __ASMNAME("_wcstold96_l");
__LIBC long double (__LIBCCALL c32told_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, __locale_t __loc) __ASMNAME("wcstold_l");
#endif /* !__PE__ */
__LIBC wint_t (__LIBCCALL getc16_unlocked)(__FILE *__stream) __ASMNAME("_fgetwc_nolock");
__LIBC wint_t (__LIBCCALL getc32_unlocked)(__FILE *__stream) __ASMNAME("fgetwc_unlocked");
__LIBC wint_t (__LIBCCALL fgetc16_unlocked)(__FILE *__stream) __ASMNAME("_fgetwc_nolock");
__LIBC wint_t (__LIBCCALL fgetc32_unlocked)(__FILE *__stream) __ASMNAME("fgetwc_unlocked");
__LIBC wint_t (__LIBCCALL getc16char_unlocked)(void) __ASMNAME("_getwchar_nolock");
__LIBC wint_t (__LIBCCALL getc32char_unlocked)(void) __ASMNAME("getwchar_unlocked");
__LIBC wint_t (__LIBCCALL putc16_unlocked)(char16_t __wc, __FILE *__stream) __ASMNAME("_fputwc_nolock");
__LIBC wint_t (__LIBCCALL putc32_unlocked)(char32_t __wc, __FILE *__stream) __ASMNAME("fputwc_unlocked");
__LIBC wint_t (__LIBCCALL fputc16_unlocked)(char16_t __wc, __FILE *__stream) __ASMNAME("_fputwc_nolock");
__LIBC wint_t (__LIBCCALL fputc32_unlocked)(char32_t __wc, __FILE *__stream) __ASMNAME("fputwc_unlocked");
__LIBC wint_t (__LIBCCALL putc16char_unlocked)(char16_t __wc) __ASMNAME("_putwchar_nolock");
__LIBC wint_t (__LIBCCALL putc32char_unlocked)(char32_t __wc) __ASMNAME("putwchar_unlocked");
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC char16_t *(__LIBCCALL fgetc16s_unlocked)(char16_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("_fgetws_nolock");
__LIBC char32_t *(__LIBCCALL fgetc32s_unlocked)(char32_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgetws_unlocked");
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__LIBC char16_t *(__LIBCCALL fgetc16s_unlocked)(char16_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("_fgetws_nolock_sz");
__LIBC char32_t *(__LIBCCALL fgetc32s_unlocked)(char32_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgetws_unlocked_sz");
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__LIBC char16_t *(__LIBCCALL fgetc16s_unlocked)(char16_t *__restrict __buf, int __n, __FILE *__restrict __stream) __ASMNAME("_fgetws_nolock");
__LIBC char32_t *(__LIBCCALL fgetc32s_unlocked)(char32_t *__restrict __buf, int __n, __FILE *__restrict __stream) __ASMNAME("fgetws_unlocked");
#endif /* !__USE_KOS */
__LIBC int (__LIBCCALL fputc16s_unlocked)(char16_t const *__restrict __str, __FILE *__restrict __stream) __ASMNAME("_fputws_nolock");
__LIBC int (__LIBCCALL fputc32s_unlocked)(char32_t const *__restrict __str, __FILE *__restrict __stream) __ASMNAME("fputws_unlocked");
__LIBC size_t (__LIBCCALL c16ftime_l)(char16_t *__restrict __buf, size_t __maxsize, char16_t const *__restrict __format, struct tm const *__restrict __tp, __locale_t __loc) __C16_FUNC(ftime_l);
__LIBC size_t (__LIBCCALL c32ftime_l)(char32_t *__restrict __buf, size_t __maxsize, char32_t const *__restrict __format, struct tm const *__restrict __tp, __locale_t __loc) __C32_FUNC(ftime_l);
#endif /* __USE_GNU */

#ifdef __USE_KOS
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC char16_t *(__LIBCCALL c16end)(char16_t *__restrict __s) __C16_FUNC(end);
__LIBC char32_t *(__LIBCCALL c32end)(char32_t *__restrict __s) __C32_FUNC(end);
__LIBC char16_t *(__LIBCCALL c16nend)(char16_t *__restrict __s, size_t __n) __C16_FUNC(nend);
__LIBC char32_t *(__LIBCCALL c32nend)(char32_t *__restrict __s, size_t __n) __C32_FUNC(nend);
__LIBC char16_t const *(__LIBCCALL c16end)(char16_t const *__restrict __s) __C16_FUNC(end);
__LIBC char32_t const *(__LIBCCALL c32end)(char32_t const *__restrict __s) __C32_FUNC(end);
__LIBC char16_t const *(__LIBCCALL c16nend)(char16_t const *__restrict __s, size_t __n) __C16_FUNC(nend);
__LIBC char32_t const *(__LIBCCALL c32nend)(char32_t const *__restrict __s, size_t __n) __C32_FUNC(nend);
}
#else
__LIBC char16_t *(__LIBCCALL c16end)(char16_t const *__restrict __s) __C16_FUNC(end);
__LIBC char32_t *(__LIBCCALL c32end)(char32_t const *__restrict __s) __C32_FUNC(end);
__LIBC char16_t *(__LIBCCALL c16nend)(char16_t const *__restrict __s, size_t __n) __C16_FUNC(nend);
__LIBC char32_t *(__LIBCCALL c32nend)(char32_t const *__restrict __s, size_t __n) __C32_FUNC(nend);
#endif
#endif /* __USE_KOS */

#endif /* __USE_UTF */
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_UCHAR_H */
