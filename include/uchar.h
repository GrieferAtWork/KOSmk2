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
#include <bits/mbstate.h>
#ifdef __USE_UTF
#include <xlocale.h>
#include <__malldefs.h>
#endif /* __USE_UTF */

__SYSDECL_BEGIN

/* Define `size_t' */
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

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */


/* Libc uses utf16/utf32 to encode/decode char16_t and char32_t */
#define __STD_UTF_16__ 1
#define __STD_UTF_32__ 1

#ifndef __KERNEL__
#ifdef __CRT_GLC
__LIBC __PORT_NODOS size_t __NOTHROW((__LIBCCALL mbrtoc16)(char16_t *__restrict __pc16, char const *__restrict __s, size_t __n, mbstate_t *__restrict __p));
__LIBC __PORT_NODOS size_t __NOTHROW((__LIBCCALL mbrtoc32)(char32_t *__restrict __pc32, char const *__restrict __s, size_t __n, mbstate_t *__restrict __p));
__LIBC __PORT_NODOS size_t __NOTHROW((__LIBCCALL c16rtomb)(char *__restrict __s, char16_t __c16, mbstate_t *__restrict __ps));
__LIBC __PORT_NODOS size_t __NOTHROW((__LIBCCALL c32rtomb)(char *__restrict __s, char32_t __c32, mbstate_t *__restrict __ps));
#endif /* __CRT_GLC */

/* UTF-16/32 string functions. */
#if defined(__USE_UTF) && defined(__CRT_KOS)

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

/* TODO: Redirect all functions in this file. */
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,size_t,__LIBCCALL,mbr16len,(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps),mbrlen,(__s,__n,__ps))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,size_t,__LIBCCALL,mbr32len,(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps),mbrlen,(__s,__n,__ps))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),char16_t *,__LIBCCALL,c16cpy,(char16_t *__restrict __dst, char16_t const *__restrict __src),wcscpy,(__dst,__src))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),char32_t *,__LIBCCALL,c32cpy,(char32_t *__restrict __dst, char32_t const *__restrict __src),wcscpy,(__dst,__src))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),char16_t *,__LIBCCALL,c16ncpy,(char16_t *__restrict __dst, char16_t const *__restrict __src, size_t __n),wcsncpy,(__dst,__src,__n))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),char32_t *,__LIBCCALL,c32ncpy,(char32_t *__restrict __dst, char32_t const *__restrict __src, size_t __n),wcsncpy,(__dst,__src,__n))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),char16_t *,__LIBCCALL,c16cat,(char16_t *__restrict __dst, char16_t const *__restrict __src),wcscat,(__dst,__src))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),char32_t *,__LIBCCALL,c32cat,(char32_t *__restrict __dst, char32_t const *__restrict __src),wcscat,(__dst,__src))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),char16_t *,__LIBCCALL,c16ncat,(char16_t *__restrict __dst, char16_t const *__restrict __src, size_t __n),wcsncat,(__dst,__src,__n))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),char32_t *,__LIBCCALL,c32ncat,(char32_t *__restrict __dst, char32_t const *__restrict __src, size_t __n),wcsncat,(__dst,__src,__n))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,c16cmp,(char16_t const *__s1, char16_t const *__s2),wcscmp,(__s1,__s2))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,c32cmp,(char32_t const *__s1, char32_t const *__s2),wcscmp,(__s1,__s2))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,c16ncmp,(char16_t const *__s1, char16_t const *__s2, size_t __n),wcsncmp,(__s1,__s2,__n))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,c32ncmp,(char32_t const *__s1, char32_t const *__s2, size_t __n),wcsncmp,(__s1,__s2,__n))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,c16coll,(char16_t const *__s1, char16_t const *__s2),wcscoll,(__s1,__s2))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,c32coll,(char32_t const *__s1, char32_t const *__s2),wcscoll,(__s1,__s2))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,size_t,__LIBCCALL,c16xfrm,(char16_t *__restrict __s1, char16_t const *__restrict __s2, size_t __n),wcsxfrm,(__s1,__s2,__n))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,size_t,__LIBCCALL,c32xfrm,(char32_t *__restrict __s1, char32_t const *__restrict __s2, size_t __n),wcsxfrm,(__s1,__s2,__n))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,size_t,__LIBCCALL,mbsrtoc16,(char16_t *__restrict __dst, char const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps),mbsrtowcs,(__dst,__psrc,__len,__ps))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,size_t,__LIBCCALL,mbsrtoc32,(char32_t *__restrict __dst, char const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps),mbsrtowcs,(__dst,__psrc,__len,__ps))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,size_t,__LIBCCALL,c16rtombs,(char *__restrict __dst, char16_t const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps),wcsrtombs,(__dst,__psrc,__len,__ps))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,size_t,__LIBCCALL,c32rtombs,(char *__restrict __dst, char32_t const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps),wcsrtombs,(__dst,__psrc,__len,__ps))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,double,__LIBCCALL,c16tod,(char16_t const *__restrict __nptr, char16_t **__restrict __endptr),wcstod,(__nptr,__endptr,__base))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,double,__LIBCCALL,c32tod,(char32_t const *__restrict __nptr, char32_t **__restrict __endptr),wcstod,(__nptr,__endptr,__base))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,long int,__LIBCCALL,c16tol,(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,long int,__LIBCCALL,c32tol,(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,unsigned long int,__LIBCCALL,c16toul,(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,unsigned long int,__LIBCCALL,c32toul,(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,fgetc16,(__FILE *__stream),fgetwc,(__stream))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,fgetc32,(__FILE *__stream),fgetwc,(__stream))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,getc16,(__FILE *__stream),getwc,(__stream))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,getc32,(__FILE *__stream),getwc,(__stream))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,getc16char,(void),getwchar,())
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,getc32char,(void),getwchar,())
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,fputc16,(char16_t __wc, __FILE *__stream),fputwc,(__wc,__stream))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,fputc32,(char32_t __wc, __FILE *__stream),fputwc,(__wc,__stream))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,putc16,(char16_t __wc, __FILE *__stream),putwc,(__wc,__stream))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,putc32,(char32_t __wc, __FILE *__stream),putwc,(__wc,__stream))
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,putc16char,(char16_t __wc),putwchar,(__wc))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,wint_t,__LIBCCALL,putc32char,(char32_t __wc),putwchar,(__wc))
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,char16_t *,__LIBCCALL,fgetc16s,(char16_t *__restrict __ws, size_t __n, __FILE *__restrict __stream),fgetws,(__ws,__n,__stream))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,char32_t *,__LIBCCALL,fgetc32s,(char32_t *__restrict __ws, size_t __n, __FILE *__restrict __stream),fgetws,(__ws,__n,__stream))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__REDIRECT(__LIBC,__PORT_KOSONLY,char16_t *,__LIBCCALL,fgetc16s,(char16_t *__restrict __ws, size_t __n, __FILE *__restrict __stream),_fgetws_sz,(__ws,__n,__stream))
__REDIRECT(__LIBC,__PORT_KOSONLY,char32_t *,__LIBCCALL,fgetc32s,(char32_t *__restrict __ws, size_t __n, __FILE *__restrict __stream),fgetws_sz,(__ws,__n,__stream))
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__REDIRECT_C16(__LIBC,__PORT_KOSONLY,char16_t *,__LIBCCALL,fgetc16s,(char16_t *__restrict __ws, int __n, __FILE *__restrict __stream),fgetws,(__ws,__n,__stream))
__REDIRECT_C32(__LIBC,__PORT_KOSONLY,char32_t *,__LIBCCALL,fgetc32s,(char32_t *__restrict __ws, int __n, __FILE *__restrict __stream),fgetws,(__ws,__n,__stream))
#endif /* !__USE_KOS */
__LIBC __PORT_KOSONLY int (__LIBCCALL fputc16s)(char16_t const *__restrict __ws, __FILE *__restrict __stream) __C16_DECL(fputws);
__LIBC __PORT_KOSONLY int (__LIBCCALL fputc32s)(char32_t const *__restrict __ws, __FILE *__restrict __stream) __C32_DECL(fputws);
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL ungetc16)(wint_t __wc, __FILE *__stream) __C16_DECL(ungetwc);
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL ungetc32)(wint_t __wc, __FILE *__stream) __C32_DECL(ungetwc);
__LIBC __PORT_KOSONLY size_t (__LIBCCALL c16ftime)(char16_t *__restrict __s, size_t __maxsize, char16_t const *__restrict __format, struct tm const *__restrict __tp) __C16_DECL(wcsftime);
__LIBC __PORT_KOSONLY size_t (__LIBCCALL c32ftime)(char32_t *__restrict __s, size_t __maxsize, char32_t const *__restrict __format, struct tm const *__restrict __tp) __C32_DECL(wcsftime);
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL c16tok)(char16_t *__restrict __s, char16_t const *__restrict __delim, char16_t **__restrict __ptr) __ASMNAME("wcstok_s");
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL c32tok)(char32_t *__restrict __s, char32_t const *__restrict __delim, char32_t **__restrict __ptr) __ASMNAME("wcstok");
__LIBC __PORT_KOSONLY size_t (__LIBCCALL c16len)(char16_t const *__s) __C16_DECL(wcslen);
__LIBC __PORT_KOSONLY size_t (__LIBCCALL c32len)(char32_t const *__s) __C32_DECL(wcslen);
__LIBC __PORT_KOSONLY __ATTR_PURE size_t (__LIBCCALL c16spn)(char16_t const *__haystack, char16_t const *__accept) __C16_DECL(wcsspn);
__LIBC __PORT_KOSONLY __ATTR_PURE size_t (__LIBCCALL c32spn)(char32_t const *__haystack, char16_t const *__accept) __C32_DECL(wcsspn);
__LIBC __PORT_KOSONLY __ATTR_PURE size_t (__LIBCCALL c16cspn)(char16_t const *__haystack, char16_t const *__reject) __C16_DECL(wcscspn);
__LIBC __PORT_KOSONLY __ATTR_PURE size_t (__LIBCCALL c32cspn)(char32_t const *__haystack, char16_t const *__reject) __C32_DECL(wcscspn);
__LIBC __PORT_KOSONLY __ATTR_PURE int (__LIBCCALL umemcmp)(char16_t const *__s1, char16_t const *__s2, size_t __n) __ASMNAME("memcmpw");
__LIBC __PORT_KOSONLY __ATTR_PURE int (__LIBCCALL Umemcmp)(char32_t const *__s1, char32_t const *__s2, size_t __n) __ASMNAME("memcmpl");
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL umemcpy)(char16_t *__restrict __s1, char16_t const *__restrict __s2, size_t __n) __ASMNAME("memcpyw");
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL Umemcpy)(char32_t *__restrict __s1, char32_t const *__restrict __s2, size_t __n) __ASMNAME("memcpyl");
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL umemmove)(char16_t *__s1, char16_t const *__s2, size_t __n) __ASMNAME("memmovew");
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL Umemmove)(char32_t *__s1, char32_t const *__s2, size_t __n) __ASMNAME("memmovel");
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL umemset)(char16_t *__s, char16_t __c, size_t __n) __ASMNAME("memsetw");
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL Umemset)(char32_t *__s, char32_t __c, size_t __n) __ASMNAME("memsetl");

#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16chr)(char16_t *__haystack, char16_t __wc) __C16_DECL(wcschr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32chr)(char32_t *__haystack, char32_t __wc) __C32_DECL(wcschr);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t const *(__LIBCCALL c16chr)(char16_t const *__haystack, char16_t __wc) __C16_DECL(wcschr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t const *(__LIBCCALL c32chr)(char32_t const *__haystack, char32_t __wc) __C32_DECL(wcschr);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16rchr)(char16_t *__haystack, char16_t __wc) __C16_DECL(wcsrchr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32rchr)(char32_t *__haystack, char32_t __wc) __C32_DECL(wcsrchr);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t const *(__LIBCCALL c16rchr)(char16_t const *__haystack, char16_t __wc) __C16_DECL(wcsrchr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t const *(__LIBCCALL c32rchr)(char32_t const *__haystack, char32_t __wc) __C32_DECL(wcsrchr);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16pbrk)(char16_t *__haystack, char16_t const *__accept) __C16_DECL(wcspbrk);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32pbrk)(char32_t *__haystack, char32_t const *__accept) __C32_DECL(wcspbrk);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t const *(__LIBCCALL c16pbrk)(char16_t const *__haystack, char16_t const *__accept) __C16_DECL(wcspbrk);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t const *(__LIBCCALL c32pbrk)(char32_t const *__haystack, char32_t const *__accept) __C32_DECL(wcspbrk);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16str)(char16_t *__haystack, char16_t const *__needle) __C16_DECL(wcsstr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32str)(char32_t *__haystack, char32_t const *__needle) __C32_DECL(wcsstr);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t const *(__LIBCCALL c16str)(char16_t const *__haystack, char16_t const *__needle) __C16_DECL(wcsstr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t const *(__LIBCCALL c32str)(char32_t const *__haystack, char32_t const *__needle) __C32_DECL(wcsstr);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL umemchr)(char16_t *__s, char16_t __c, size_t __n) __ASMNAME("memchrw");
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL Umemchr)(char32_t *__s, char32_t __c, size_t __n) __ASMNAME("memchrl");
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t const *(__LIBCCALL umemchr)(char16_t const *__s, char16_t __c, size_t __n) __ASMNAME("memchrw");
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t const *(__LIBCCALL Umemchr)(char32_t const *__s, char32_t __c, size_t __n) __ASMNAME("memchrl");
}
#else
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16chr)(char16_t const *__haystack, char16_t __wc) __C16_DECL(wcschr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32chr)(char32_t const *__haystack, char32_t __wc) __C32_DECL(wcschr);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16rchr)(char16_t const *__haystack, char16_t __wc) __C16_DECL(wcsrchr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32rchr)(char32_t const *__haystack, char32_t __wc) __C32_DECL(wcsrchr);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16pbrk)(char16_t const *__haystack, char16_t const *__accept) __C16_DECL(wcspbrk);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32pbrk)(char32_t const *__haystack, char32_t const *__accept) __C32_DECL(wcspbrk);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16str)(char16_t const *__haystack, char16_t const *__needle) __C16_DECL(wcsstr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32str)(char32_t const *__haystack, char32_t const *__needle) __C32_DECL(wcsstr);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL umemchr)(char16_t const *__s, char16_t __c, size_t __n) __ASMNAME("memchrw");
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL Umemchr)(char32_t const *__s, char32_t __c, size_t __n) __ASMNAME("memchrl");
#endif
#if defined(__USE_ISOC95) || defined(__USE_UNIX98)
#ifdef __USE_KOS
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL fuprintf)(__FILE *__restrict __stream, char16_t const *__restrict __format, ...) __C16_DECL(fwprintf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL fUprintf)(__FILE *__restrict __stream, char32_t const *__restrict __format, ...) __C32_DECL(fwprintf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL uprintf)(char16_t const *__restrict __format, ...) __C16_DECL(wprintf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL Uprintf)(char32_t const *__restrict __format, ...) __C32_DECL(wprintf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL suprintf)(char16_t *__restrict __s, size_t __n, char16_t const *__restrict __format, ...) __ASMNAME("_swprintf_c");
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL sUprintf)(char32_t *__restrict __s, size_t __n, char32_t const *__restrict __format, ...) __ASMNAME("swprintf");
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vfuprintf)(__FILE *__restrict __s, char16_t const *__restrict __format, __builtin_va_list __arg) __C16_DECL(vfwprintf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vfUprintf)(__FILE *__restrict __s, char32_t const *__restrict __format, __builtin_va_list __arg) __C32_DECL(vfwprintf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vuprintf)(char16_t const *__restrict __format, __builtin_va_list __arg) __C16_DECL(vwprintf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vUprintf)(char32_t const *__restrict __format, __builtin_va_list __arg) __C32_DECL(vwprintf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vsuprintf)(char16_t *__restrict __s, size_t __n, char16_t const *__restrict __format, __builtin_va_list __arg) __ASMNAME("_vswprintf_c");
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vsUprintf)(char32_t *__restrict __s, size_t __n, char32_t const *__restrict __format, __builtin_va_list __arg) __ASMNAME("vswprintf");
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL fuscanf)(__FILE *__restrict __stream, char16_t const *__restrict __format, ...) __C16_DECL(fwscanf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL fUscanf)(__FILE *__restrict __stream, char32_t const *__restrict __format, ...) __C32_DECL(fwscanf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL uscanf)(char16_t const *__restrict __format, ...) __C16_DECL(wscanf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL Uscanf)(char32_t const *__restrict __format, ...) __C32_DECL(wscanf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL suscanf)(char16_t const *__restrict __s, char16_t const *__restrict __format, ...) __C16_DECL(swscanf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL sUscanf)(char32_t const *__restrict __s, char32_t const *__restrict __format, ...) __C32_DECL(swscanf);
#else /* __USE_KOS */
__LIBC __PORT_KOSONLY int (__LIBCCALL fuprintf)(__FILE *__restrict __stream, char16_t const *__restrict __format, ...) __C16_DECL(fwprintf);
__LIBC __PORT_KOSONLY int (__LIBCCALL fUprintf)(__FILE *__restrict __stream, char32_t const *__restrict __format, ...) __C32_DECL(fwprintf);
__LIBC __PORT_KOSONLY int (__LIBCCALL uprintf)(char16_t const *__restrict __format, ...) __C16_DECL(wprintf);
__LIBC __PORT_KOSONLY int (__LIBCCALL Uprintf)(char32_t const *__restrict __format, ...) __C32_DECL(wprintf);
__LIBC __PORT_KOSONLY int (__LIBCCALL suprintf)(char16_t *__restrict __s, size_t __n, char16_t const *__restrict __format, ...) __ASMNAME("_swprintf_c");
__LIBC __PORT_KOSONLY int (__LIBCCALL sUprintf)(char32_t *__restrict __s, size_t __n, char32_t const *__restrict __format, ...) __ASMNAME("swprintf");
__LIBC __PORT_KOSONLY int (__LIBCCALL vfuprintf)(__FILE *__restrict __s, char16_t const *__restrict __format, __builtin_va_list __arg) __C16_DECL(vfwprintf);
__LIBC __PORT_KOSONLY int (__LIBCCALL vfUprintf)(__FILE *__restrict __s, char32_t const *__restrict __format, __builtin_va_list __arg) __C32_DECL(vfwprintf);
__LIBC __PORT_KOSONLY int (__LIBCCALL vuprintf)(char16_t const *__restrict __format, __builtin_va_list __arg) __C16_DECL(vwprintf);
__LIBC __PORT_KOSONLY int (__LIBCCALL vUprintf)(char32_t const *__restrict __format, __builtin_va_list __arg) __C32_DECL(vwprintf);
__LIBC __PORT_KOSONLY int (__LIBCCALL vsuprintf)(char16_t *__restrict __s, size_t __n, char16_t const *__restrict __format, __builtin_va_list __arg) __ASMNAME("_vswprintf_c");
__LIBC __PORT_KOSONLY int (__LIBCCALL vsUprintf)(char32_t *__restrict __s, size_t __n, char32_t const *__restrict __format, __builtin_va_list __arg) __ASMNAME("vswprintf");
__LIBC __PORT_KOSONLY int (__LIBCCALL fuscanf)(__FILE *__restrict __stream, char16_t const *__restrict __format, ...) __C16_DECL(fwscanf);
__LIBC __PORT_KOSONLY int (__LIBCCALL fUscanf)(__FILE *__restrict __stream, char32_t const *__restrict __format, ...) __C32_DECL(fwscanf);
__LIBC __PORT_KOSONLY int (__LIBCCALL uscanf)(char16_t const *__restrict __format, ...) __C16_DECL(wscanf);
__LIBC __PORT_KOSONLY int (__LIBCCALL Uscanf)(char32_t const *__restrict __format, ...) __C32_DECL(wscanf);
__LIBC __PORT_KOSONLY int (__LIBCCALL suscanf)(char16_t const *__restrict __s, char16_t const *__restrict __format, ...) __C16_DECL(swscanf);
__LIBC __PORT_KOSONLY int (__LIBCCALL sUscanf)(char32_t const *__restrict __s, char32_t const *__restrict __format, ...) __C32_DECL(swscanf);
#endif /* !__USE_KOS */
#endif /* __USE_ISOC95 || __USE_UNIX98 */
#ifdef __USE_ISOC99
__LIBC __PORT_KOSONLY float (__LIBCCALL c16tof)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr) __C16_DECL(wcstof);
__LIBC __PORT_KOSONLY float (__LIBCCALL c32tof)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr) __C32_DECL(wcstof);
#ifdef __PE__
__LIBC __PORT_KOSONLY long double (__LIBCCALL c16told)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr) __ASMNAME("wcstod");
__LIBC __PORT_KOSONLY long double (__LIBCCALL c32told)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr) __ASMNAME(".kos.wcstod");
#else /* __PE__ */
__LIBC __PORT_KOSONLY long double (__LIBCCALL c16told)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr) __ASMNAME(".dos.wcstold96");
__LIBC __PORT_KOSONLY long double (__LIBCCALL c32told)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr) __ASMNAME("wcstold");
#endif /* !__PE__ */
#if __SIZEOF_LONG_LONG__ == 8
__LIBC __PORT_KOSONLY __LONGLONG (__LIBCCALL c16toll)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base) __ASMNAME("_wcstoi64");
__LIBC __PORT_KOSONLY __ULONGLONG (__LIBCCALL c16toull)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base) __ASMNAME("_wcstoui64");
#else
__LIBC __PORT_KOSONLY __LONGLONG (__LIBCCALL c16toll)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base) __C16_DECL(wcstoll);
__LIBC __PORT_KOSONLY __ULONGLONG (__LIBCCALL c16toull)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base) __C16_DECL(wcstoull);
#endif
__LIBC __PORT_KOSONLY __LONGLONG (__LIBCCALL c32toll)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base) __C32_DECL(wcstoll);
__LIBC __PORT_KOSONLY __ULONGLONG (__LIBCCALL c32toull)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base) __C32_DECL(wcstoull);
#ifdef __USE_KOS
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vfuscanf)(__FILE *__restrict __s, char16_t const *__restrict __format, __builtin_va_list __arg) __C16_DECL(vfwscanf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vfUscanf)(__FILE *__restrict __s, char32_t const *__restrict __format, __builtin_va_list __arg) __C32_DECL(vfwscanf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vuscanf)(char16_t const *__restrict __format, __builtin_va_list __arg) __C16_DECL(vwscanf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vUscanf)(char32_t const *__restrict __format, __builtin_va_list __arg) __C32_DECL(vwscanf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vsuscanf)(char16_t const *__restrict __s, char16_t const *__restrict __format, __builtin_va_list __arg) __C16_DECL(vswscanf);
__LIBC __PORT_KOSONLY ssize_t (__LIBCCALL vsUscanf)(char32_t const *__restrict __s, char32_t const *__restrict __format, __builtin_va_list __arg) __C32_DECL(vswscanf);
#else /* __USE_KOS */
__LIBC __PORT_KOSONLY int (__LIBCCALL vfuscanf)(__FILE *__restrict __s, char16_t const *__restrict __format, __builtin_va_list __arg) __C16_DECL(vfwscanf);
__LIBC __PORT_KOSONLY int (__LIBCCALL vfUscanf)(__FILE *__restrict __s, char32_t const *__restrict __format, __builtin_va_list __arg) __C32_DECL(vfwscanf);
__LIBC __PORT_KOSONLY int (__LIBCCALL vuscanf)(char16_t const *__restrict __format, __builtin_va_list __arg) __C16_DECL(vwscanf);
__LIBC __PORT_KOSONLY int (__LIBCCALL vUscanf)(char32_t const *__restrict __format, __builtin_va_list __arg) __C32_DECL(vwscanf);
__LIBC __PORT_KOSONLY int (__LIBCCALL vsuscanf)(char16_t const *__restrict __s, char16_t const *__restrict __format, __builtin_va_list __arg) __C16_DECL(vswscanf);
__LIBC __PORT_KOSONLY int (__LIBCCALL vsUscanf)(char32_t const *__restrict __s, char32_t const *__restrict __format, __builtin_va_list __arg) __C32_DECL(vswscanf);
#endif /* !__USE_KOS */
#endif /* __USE_ISOC99 */

#ifdef __USE_XOPEN2K8
__LIBC __PORT_KOSONLY int (__LIBCCALL c16casecmp)(char16_t const *__s1, char16_t const *__s2) __ASMNAME("_wcsicmp");
__LIBC __PORT_KOSONLY int (__LIBCCALL c32casecmp)(char32_t const *__s1, char32_t const *__s2) __ASMNAME("wcscasecmp");
__LIBC __PORT_KOSONLY int (__LIBCCALL c16ncasecmp)(char16_t const *__s1, char16_t const *__s2, size_t __n) __ASMNAME("_wcsnicmp");
__LIBC __PORT_KOSONLY int (__LIBCCALL c32ncasecmp)(char32_t const *__s1, char32_t const *__s2, size_t __n) __ASMNAME("wcsncasecmp");
__LIBC __PORT_KOSONLY int (__LIBCCALL c16casecmp_l)(char16_t const *__s1, char16_t const *__s2, __locale_t __loc) __ASMNAME("_wcsicmp_l");
__LIBC __PORT_KOSONLY int (__LIBCCALL c32casecmp_l)(char32_t const *__s1, char32_t const *__s2, __locale_t __loc) __ASMNAME("wcscasecmp_l");
__LIBC __PORT_KOSONLY int (__LIBCCALL c16ncasecmp_l)(char16_t const *__s1, char16_t const *__s2, size_t __n, __locale_t __loc) __ASMNAME("_wcsnicmp_l");
__LIBC __PORT_KOSONLY int (__LIBCCALL c32ncasecmp_l)(char32_t const *__s1, char32_t const *__s2, size_t __n, __locale_t __loc) __ASMNAME("wcsncasecmp_l");
__LIBC __PORT_KOSONLY int (__LIBCCALL c16coll_l)(char16_t const *__s1, char16_t const *__s2, __locale_t __loc) __ASMNAME("_wcscoll_l");
__LIBC __PORT_KOSONLY int (__LIBCCALL c32coll_l)(char32_t const *__s1, char32_t const *__s2, __locale_t __loc) __ASMNAME("wcscoll_l");
__LIBC __PORT_KOSONLY size_t (__LIBCCALL c16sxfrm_l)(char16_t *__s1, char16_t const *__s2, size_t __n, __locale_t __loc) __ASMNAME("_wcsxfrm_l");
__LIBC __PORT_KOSONLY size_t (__LIBCCALL c32xfrm_l)(char32_t *__s1, char32_t const *__s2, size_t __n, __locale_t __loc) __ASMNAME("wcsxfrm_l");
__LIBC __PORT_KOSONLY size_t (__LIBCCALL mbsnrtoc16)(char16_t *__restrict __dst, char const **__restrict __src, size_t __nmc, size_t __len, mbstate_t *__restrict __ps) __C16_DECL(mbsnrtowcs);
__LIBC __PORT_KOSONLY size_t (__LIBCCALL mbsnrtoc32)(char32_t *__restrict __dst, char const **__restrict __src, size_t __nmc, size_t __len, mbstate_t *__restrict __ps) __C32_DECL(mbsnrtowcs);
__LIBC __PORT_KOSONLY size_t (__LIBCCALL c16nrtombs)(char *__restrict __dst, char16_t const **__restrict __src, size_t __nwc, size_t __len, mbstate_t *__restrict __ps) __C16_DECL(wcsnrtombs);
__LIBC __PORT_KOSONLY size_t (__LIBCCALL c32nrtombs)(char *__restrict __dst, char32_t const **__restrict __src, size_t __nwc, size_t __len, mbstate_t *__restrict __ps) __C32_DECL(wcsnrtombs);
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL c16pcpy)(char16_t *__restrict __dst, char16_t const *__restrict __src) __C16_DECL(wcpcpy);
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL c32pcpy)(char32_t *__restrict __dst, char32_t const *__restrict __src) __C32_DECL(wcpcpy);
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL c16pncpy)(char16_t *__restrict __dst, char16_t const *__restrict __src, size_t __n) __C16_DECL(wcpncpy);
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL c32pncpy)(char32_t *__restrict __dst, char32_t const *__restrict __src, size_t __n) __C32_DECL(wcpncpy);
__LIBC __PORT_KOSONLY __FILE *(__LIBCCALL open_umemstream)(char16_t **__bufloc, size_t *__sizeloc) __ASMNAME("_open_wmemstream");
__LIBC __PORT_KOSONLY __FILE *(__LIBCCALL open_Umemstream)(char32_t **__bufloc, size_t *__sizeloc) __ASMNAME("open_wmemstream");
#endif /* __USE_XOPEN2K8 */

#if defined(__USE_XOPEN2K8) || defined(__USE_DOS)
__LIBC __PORT_KOSONLY __ATTR_PURE size_t (__LIBCCALL c16nlen)(char16_t const *__s, size_t __maxlen) __C16_DECL(wcsnlen);
__LIBC __PORT_KOSONLY __ATTR_PURE size_t (__LIBCCALL c32nlen)(char32_t const *__s, size_t __maxlen) __C32_DECL(wcsnlen);
__LIBC __PORT_KOSONLY __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char16_t *(__LIBCCALL c16dup)(char16_t const *__restrict __s) __ASMNAME("_wcsdup");
__LIBC __PORT_KOSONLY __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char32_t *(__LIBCCALL c32dup)(char32_t const *__restrict __s) __ASMNAME("wcsdup");
#endif /* __USE_XOPEN2K8 || __USE_DOS */

#ifdef __USE_XOPEN
#ifdef __USE_KOS
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL c16width)(char16_t __c) __C16_DECL(wcwidth);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL c32width)(char32_t __c) __C32_DECL(wcwidth);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL c16swidth)(char16_t const *__restrict __s, size_t __n) __C16_DECL(wcswidth);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL c32swidth)(char32_t const *__restrict __s, size_t __n) __C32_DECL(wcswidth);
#else /* __USE_KOS */
__LIBC __PORT_KOSONLY int (__LIBCCALL c16width)(char16_t __c) __C16_DECL(wcwidth);
__LIBC __PORT_KOSONLY int (__LIBCCALL c32width)(char32_t __c) __C32_DECL(wcwidth);
__LIBC __PORT_KOSONLY int (__LIBCCALL c16swidth)(char16_t const *__restrict __s, size_t __n) __C16_DECL(wcswidth);
__LIBC __PORT_KOSONLY int (__LIBCCALL c32swidth)(char32_t const *__restrict __s, size_t __n) __C32_DECL(wcswidth);
#endif /* !__USE_KOS */
#endif /* __USE_XOPEN */

#if defined(__USE_XOPEN) || defined(__USE_DOS)
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16c16)(char16_t *__haystack, char16_t const *__needle) __C16_DECL(wcsstr);
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t const *(__LIBCCALL c16c16)(char16_t const *__haystack, char16_t const *__needle) __C16_DECL(wcsstr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32c32)(char32_t *__haystack, char32_t const *__needle) __C32_DECL(wcsstr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t const *(__LIBCCALL c32c32)(char32_t const *__haystack, char32_t const *__needle) __C32_DECL(wcsstr);
}
#else
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16c16)(char16_t const *__haystack, char16_t const *__needle) __C16_DECL(wcsstr);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32c32)(char32_t const *__haystack, char32_t const *__needle) __C32_DECL(wcsstr);
#endif
#endif /* __USE_XOPEN || __USE_DOS */

#ifdef __USE_GNU
__LIBC __PORT_KOSONLY __ATTR_PURE char16_t *(__LIBCCALL c16chrnul)(char16_t const *__s, char16_t __wc) __C16_DECL(wcschrnul);
__LIBC __PORT_KOSONLY __ATTR_PURE char32_t *(__LIBCCALL c32chrnul)(char32_t const *__s, char32_t __wc) __C32_DECL(wcschrnul);
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL umempcpy)(char16_t *__restrict __s1, char16_t const *__restrict __s2, size_t __n) __ASMNAME("mempcpyw");
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL Umempcpy)(char32_t *__restrict __s1, char32_t const *__restrict __s2, size_t __n) __ASMNAME("mempcpyl");
#if __SIZEOF_LONG_LONG__ == 8
__LIBC __PORT_KOSONLY __LONGLONG (__LIBCCALL c16toll_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_wcstoi64_l");
__LIBC __PORT_KOSONLY __ULONGLONG (__LIBCCALL c16toull_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_wcstoui64_l");
#else
__LIBC __PORT_KOSONLY __LONGLONG (__LIBCCALL c16toll_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __C16_DECL(wcstoll_l);
__LIBC __PORT_KOSONLY __ULONGLONG (__LIBCCALL c16toull_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __C16_DECL(wcstoull_l);
#endif
__LIBC __PORT_KOSONLY __LONGLONG (__LIBCCALL c32toll_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base, __locale_t __loc) __C32_DECL(wcstoll_l);
__LIBC __PORT_KOSONLY __ULONGLONG (__LIBCCALL c32toull_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base, __locale_t __loc) __C32_DECL(wcstoull_l);
__LIBC __PORT_KOSONLY long int (__LIBCCALL c16tol_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_wcstol_l");
__LIBC __PORT_KOSONLY long int (__LIBCCALL c32tol_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("wcstol_l");
__LIBC __PORT_KOSONLY unsigned long int (__LIBCCALL c16toul_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_wcstoul_l");
__LIBC __PORT_KOSONLY unsigned long int (__LIBCCALL c32toul_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("wcstoul_l");
__LIBC __PORT_KOSONLY double (__LIBCCALL c16tod_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, __locale_t __loc) __ASMNAME("_wcstod_l");
__LIBC __PORT_KOSONLY double (__LIBCCALL c32tod_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, __locale_t __loc) __ASMNAME("wcstod_l");
__LIBC __PORT_KOSONLY float (__LIBCCALL c16tof_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, __locale_t __loc) __ASMNAME("_wcstof_l");
__LIBC __PORT_KOSONLY float (__LIBCCALL c32tof_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, __locale_t __loc) __ASMNAME("wcstof_l");
#ifdef __PE__
__LIBC __PORT_KOSONLY long double (__LIBCCALL c16told_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, __locale_t __loc) __ASMNAME("_wcstod_l");
__LIBC __PORT_KOSONLY long double (__LIBCCALL c32told_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, __locale_t __loc) __ASMNAME("wcstod_l");
#else /* __PE__ */
__LIBC __PORT_KOSONLY long double (__LIBCCALL c16told_l)(char16_t const *__restrict __nptr, char16_t **__restrict __endptr, __locale_t __loc) __ASMNAME("_wcstold96_l");
__LIBC __PORT_KOSONLY long double (__LIBCCALL c32told_l)(char32_t const *__restrict __nptr, char32_t **__restrict __endptr, __locale_t __loc) __ASMNAME("wcstold_l");
#endif /* !__PE__ */
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL getc16_unlocked)(__FILE *__stream) __ASMNAME("_fgetwc_nolock");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL getc32_unlocked)(__FILE *__stream) __ASMNAME("fgetwc_unlocked");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL fgetc16_unlocked)(__FILE *__stream) __ASMNAME("_fgetwc_nolock");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL fgetc32_unlocked)(__FILE *__stream) __ASMNAME("fgetwc_unlocked");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL getc16char_unlocked)(void) __ASMNAME("_getwchar_nolock");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL getc32char_unlocked)(void) __ASMNAME("getwchar_unlocked");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL putc16_unlocked)(char16_t __wc, __FILE *__stream) __ASMNAME("_fputwc_nolock");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL putc32_unlocked)(char32_t __wc, __FILE *__stream) __ASMNAME("fputwc_unlocked");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL fputc16_unlocked)(char16_t __wc, __FILE *__stream) __ASMNAME("_fputwc_nolock");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL fputc32_unlocked)(char32_t __wc, __FILE *__stream) __ASMNAME("fputwc_unlocked");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL putc16char_unlocked)(char16_t __wc) __ASMNAME("_putwchar_nolock");
__LIBC __PORT_KOSONLY wint_t (__LIBCCALL putc32char_unlocked)(char32_t __wc) __ASMNAME("putwchar_unlocked");
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL fgetc16s_unlocked)(char16_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("_fgetws_nolock");
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL fgetc32s_unlocked)(char32_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgetws_unlocked");
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL fgetc16s_unlocked)(char16_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("_fgetws_nolock_sz");
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL fgetc32s_unlocked)(char32_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) __ASMNAME("fgetws_unlocked_sz");
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL fgetc16s_unlocked)(char16_t *__restrict __buf, int __n, __FILE *__restrict __stream) __ASMNAME("_fgetws_nolock");
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL fgetc32s_unlocked)(char32_t *__restrict __buf, int __n, __FILE *__restrict __stream) __ASMNAME("fgetws_unlocked");
#endif /* !__USE_KOS */
__LIBC __PORT_KOSONLY int (__LIBCCALL fputc16s_unlocked)(char16_t const *__restrict __str, __FILE *__restrict __stream) __ASMNAME("_fputws_nolock");
__LIBC __PORT_KOSONLY int (__LIBCCALL fputc32s_unlocked)(char32_t const *__restrict __str, __FILE *__restrict __stream) __ASMNAME("fputws_unlocked");
__LIBC __PORT_KOSONLY size_t (__LIBCCALL c16ftime_l)(char16_t *__restrict __buf, size_t __maxsize, char16_t const *__restrict __format, struct tm const *__restrict __tp, __locale_t __loc) __C16_DECL(wcsftime_l);
__LIBC __PORT_KOSONLY size_t (__LIBCCALL c32ftime_l)(char32_t *__restrict __buf, size_t __maxsize, char32_t const *__restrict __format, struct tm const *__restrict __tp, __locale_t __loc) __C32_DECL(wcsftime_l);
#endif /* __USE_GNU */

#ifdef __USE_KOS
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL c16end)(char16_t *__restrict __s) __C16_DECL(wcsend);
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL c32end)(char32_t *__restrict __s) __C32_DECL(wcsend);
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL c16nend)(char16_t *__restrict __s, size_t __n) __C16_DECL(wcsnend);
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL c32nend)(char32_t *__restrict __s, size_t __n) __C32_DECL(wcsnend);
__LIBC __PORT_KOSONLY char16_t const *(__LIBCCALL c16end)(char16_t const *__restrict __s) __C16_DECL(wcsend);
__LIBC __PORT_KOSONLY char32_t const *(__LIBCCALL c32end)(char32_t const *__restrict __s) __C32_DECL(wcsend);
__LIBC __PORT_KOSONLY char16_t const *(__LIBCCALL c16nend)(char16_t const *__restrict __s, size_t __n) __C16_DECL(wcsnend);
__LIBC __PORT_KOSONLY char32_t const *(__LIBCCALL c32nend)(char32_t const *__restrict __s, size_t __n) __C32_DECL(wcsnend);
}
#else
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL c16end)(char16_t const *__restrict __s) __C16_DECL(wcsend);
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL c32end)(char32_t const *__restrict __s) __C32_DECL(wcsend);
__LIBC __PORT_KOSONLY char16_t *(__LIBCCALL c16nend)(char16_t const *__restrict __s, size_t __n) __C16_DECL(wcsnend);
__LIBC __PORT_KOSONLY char32_t *(__LIBCCALL c32nend)(char32_t const *__restrict __s, size_t __n) __C32_DECL(wcsnend);
#endif
#endif /* __USE_KOS */

#endif /* __USE_UTF && __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_UCHAR_H */
