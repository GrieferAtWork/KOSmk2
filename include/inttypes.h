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
#ifndef _INTTYPES_H
#define _INTTYPES_H	1

#include <features.h>
#include <stdint.h>
#include <hybrid/typecore.h>
#ifdef __USE_DOS
#include <xlocale.h>
#endif /* __USE_DOS */

__SYSDECL_BEGIN

#ifndef ____gwchar_t_defined
#define ____gwchar_t_defined 1
#define __gwchar_t __WCHAR_TYPE__
#endif

#if __SIZEOF_LONG__ == 8
#   define __PRI64_PREFIX  "l"
#   define __PRIPTR_PREFIX "l"
#else
#   define __PRI64_PREFIX  "ll"
#   define __PRIPTR_PREFIX
#endif

#define PRId8          "d"
#define PRId16         "d"
#define PRId32         "d"
#define PRId64         __PRI64_PREFIX "d"
#define PRIdLEAST8     "d"
#define PRIdLEAST16    "d"
#define PRIdLEAST32    "d"
#define PRIdLEAST64    __PRI64_PREFIX "d"
#define PRIdFAST8      "d"
#define PRIdFAST16     __PRIPTR_PREFIX "d"
#define PRIdFAST32     __PRIPTR_PREFIX "d"
#define PRIdFAST64     __PRI64_PREFIX "d"
#define PRIi8          "i"
#define PRIi16         "i"
#define PRIi32         "i"
#define PRIi64         __PRI64_PREFIX "i"
#define PRIiLEAST8     "i"
#define PRIiLEAST16    "i"
#define PRIiLEAST32    "i"
#define PRIiLEAST64    __PRI64_PREFIX "i"
#define PRIiFAST8      "i"
#define PRIiFAST16     __PRIPTR_PREFIX "i"
#define PRIiFAST32     __PRIPTR_PREFIX "i"
#define PRIiFAST64     __PRI64_PREFIX "i"
#define PRIo8          "o"
#define PRIo16         "o"
#define PRIo32         "o"
#define PRIo64         __PRI64_PREFIX "o"
#define PRIoLEAST8     "o"
#define PRIoLEAST16    "o"
#define PRIoLEAST32    "o"
#define PRIoLEAST64    __PRI64_PREFIX "o"
#define PRIoFAST8      "o"
#define PRIoFAST16     __PRIPTR_PREFIX "o"
#define PRIoFAST32     __PRIPTR_PREFIX "o"
#define PRIoFAST64     __PRI64_PREFIX "o"
#define PRIu8          "u"
#define PRIu16         "u"
#define PRIu32         "u"
#define PRIu64         __PRI64_PREFIX "u"
#define PRIuLEAST8     "u"
#define PRIuLEAST16    "u"
#define PRIuLEAST32    "u"
#define PRIuLEAST64    __PRI64_PREFIX "u"
#define PRIuFAST8      "u"
#define PRIuFAST16     __PRIPTR_PREFIX "u"
#define PRIuFAST32     __PRIPTR_PREFIX "u"
#define PRIuFAST64     __PRI64_PREFIX "u"
#define PRIx8          "x"
#define PRIx16         "x"
#define PRIx32         "x"
#define PRIx64         __PRI64_PREFIX "x"
#define PRIxLEAST8     "x"
#define PRIxLEAST16    "x"
#define PRIxLEAST32    "x"
#define PRIxLEAST64    __PRI64_PREFIX "x"
#define PRIxFAST8      "x"
#define PRIxFAST16     __PRIPTR_PREFIX "x"
#define PRIxFAST32     __PRIPTR_PREFIX "x"
#define PRIxFAST64     __PRI64_PREFIX "x"
#define PRIX8          "X"
#define PRIX16         "X"
#define PRIX32         "X"
#define PRIX64         __PRI64_PREFIX "X"
#define PRIXLEAST8     "X"
#define PRIXLEAST16    "X"
#define PRIXLEAST32    "X"
#define PRIXLEAST64    __PRI64_PREFIX "X"
#define PRIXFAST8      "X"
#define PRIXFAST16     __PRIPTR_PREFIX "X"
#define PRIXFAST32     __PRIPTR_PREFIX "X"
#define PRIXFAST64     __PRI64_PREFIX "X"
#define PRIdMAX        __PRI64_PREFIX "d"
#define PRIiMAX        __PRI64_PREFIX "i"
#define PRIoMAX        __PRI64_PREFIX "o"
#define PRIuMAX        __PRI64_PREFIX "u"
#define PRIxMAX        __PRI64_PREFIX "x"
#define PRIXMAX        __PRI64_PREFIX "X"
#define PRIdPTR        __PRIPTR_PREFIX "d"
#define PRIiPTR        __PRIPTR_PREFIX "i"
#define PRIoPTR        __PRIPTR_PREFIX "o"
#define PRIuPTR        __PRIPTR_PREFIX "u"
#define PRIxPTR        __PRIPTR_PREFIX "x"
#define PRIXPTR        __PRIPTR_PREFIX "X"

#define SCNd8          "hhd"
#define SCNd16         "hd"
#define SCNd32         "d"
#define SCNd64         __PRI64_PREFIX "d"
#define SCNdLEAST8     "hhd"
#define SCNdLEAST16    "hd"
#define SCNdLEAST32    "d"
#define SCNdLEAST64    __PRI64_PREFIX "d"
#define SCNdFAST8      "hhd"
#define SCNdFAST16     __PRIPTR_PREFIX "d"
#define SCNdFAST32     __PRIPTR_PREFIX "d"
#define SCNdFAST64     __PRI64_PREFIX "d"
#define SCNi8          "hhi"
#define SCNi16         "hi"
#define SCNi32         "i"
#define SCNi64         __PRI64_PREFIX "i"
#define SCNiLEAST8     "hhi"
#define SCNiLEAST16    "hi"
#define SCNiLEAST32    "i"
#define SCNiLEAST64    __PRI64_PREFIX "i"
#define SCNiFAST8      "hhi"
#define SCNiFAST16     __PRIPTR_PREFIX "i"
#define SCNiFAST32     __PRIPTR_PREFIX "i"
#define SCNiFAST64     __PRI64_PREFIX "i"
#define SCNu8          "hhu"
#define SCNu16         "hu"
#define SCNu32         "u"
#define SCNu64         __PRI64_PREFIX "u"
#define SCNuLEAST8     "hhu"
#define SCNuLEAST16    "hu"
#define SCNuLEAST32    "u"
#define SCNuLEAST64    __PRI64_PREFIX "u"
#define SCNuFAST8      "hhu"
#define SCNuFAST16     __PRIPTR_PREFIX "u"
#define SCNuFAST32     __PRIPTR_PREFIX "u"
#define SCNuFAST64     __PRI64_PREFIX "u"
#define SCNo8          "hho"
#define SCNo16         "ho"
#define SCNo32         "o"
#define SCNo64         __PRI64_PREFIX "o"
#define SCNoLEAST8     "hho"
#define SCNoLEAST16    "ho"
#define SCNoLEAST32    "o"
#define SCNoLEAST64    __PRI64_PREFIX "o"
#define SCNoFAST8      "hho"
#define SCNoFAST16     __PRIPTR_PREFIX "o"
#define SCNoFAST32     __PRIPTR_PREFIX "o"
#define SCNoFAST64     __PRI64_PREFIX "o"
#define SCNx8          "hhx"
#define SCNx16         "hx"
#define SCNx32         "x"
#define SCNx64         __PRI64_PREFIX "x"
#define SCNxLEAST8     "hhx"
#define SCNxLEAST16    "hx"
#define SCNxLEAST32    "x"
#define SCNxLEAST64    __PRI64_PREFIX "x"
#define SCNxFAST8      "hhx"
#define SCNxFAST16     __PRIPTR_PREFIX "x"
#define SCNxFAST32     __PRIPTR_PREFIX "x"
#define SCNxFAST64     __PRI64_PREFIX "x"
#define SCNdMAX        __PRI64_PREFIX "d"
#define SCNiMAX        __PRI64_PREFIX "i"
#define SCNoMAX        __PRI64_PREFIX "o"
#define SCNuMAX        __PRI64_PREFIX "u"
#define SCNxMAX        __PRI64_PREFIX "x"
#define SCNdPTR        __PRIPTR_PREFIX "d"
#define SCNiPTR        __PRIPTR_PREFIX "i"
#define SCNoPTR        __PRIPTR_PREFIX "o"
#define SCNuPTR        __PRIPTR_PREFIX "u"
#define SCNxPTR        __PRIPTR_PREFIX "x"

__NAMESPACE_STD_BEGIN

typedef struct {
 __INTMAX_TYPE__ quot; /*< Quotient. */
 __INTMAX_TYPE__ rem;  /*< Remainder. */
} imaxdiv_t;

#ifdef __KERNEL__
__LOCAL __ATTR_CONST __INTMAX_TYPE__ (__LIBCCALL imaxabs)(__INTMAX_TYPE__ __n) { return __n < 0 ? -__n : __n; }
__LOCAL __ATTR_CONST imaxdiv_t (__LIBCCALL imaxdiv)(__INTMAX_TYPE__ __numer, __INTMAX_TYPE__ __denom) {
#ifdef __GNUC__
 return (imaxdiv_t){ __numer/__denom, __numer%__denom };
#else
 imaxdiv_t __res;
 __res.quot = __numer/__denom;
 __res.rem  = __numer%__denom;
 return __res;
#endif
}
#else
#if __SIZEOF_INTMAX_T__ == __SIZEOF_LONG__
__REDIRECT(__LIBC,__ATTR_CONST,__INTMAX_TYPE__,__LIBCCALL,imaxabs,(__INTMAX_TYPE__ __n),labs,(__n))
__REDIRECT(__LIBC,__ATTR_CONST,imaxdiv_t,__LIBCCALL,imaxdiv,(__INTMAX_TYPE__ __numer, __INTMAX_TYPE__ __denom),ldiv,(__numer,__denom))
__REDIRECT(__LIBC,,__INTMAX_TYPE__, __LIBCCALL,strtoimax,(char const *__restrict __nptr, char **__restrict __endptr, int __base),strtol,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,,__UINTMAX_TYPE__,__LIBCCALL,strtoumax,(char const *__restrict __nptr, char ** __restrict __endptr, int __base),strtoul,(__nptr,__endptr,__base))
__REDIRECT2(__LIBC,,__INTMAX_TYPE__, __LIBCCALL,wcstoimax,(__gwchar_t const *__restrict __nptr, __gwchar_t **__restrict __endptr, int __base),wcstol,_wcstol,(__nptr,__endptr,__base))
__REDIRECT2(__LIBC,,__UINTMAX_TYPE__,__LIBCCALL,wcstoumax,(__gwchar_t const *__restrict __nptr, __gwchar_t ** __restrict __endptr, int __base),wcstoul,_wcstoul,(__nptr,__endptr,__base))
#elif __SIZEOF_INTMAX_T__ == __SIZEOF_LONG_LONG__
__REDIRECT(__LIBC,__ATTR_CONST,__INTMAX_TYPE__,__LIBCCALL,imaxabs,(__INTMAX_TYPE__ __n),llabs,(__n))
__REDIRECT(__LIBC,__ATTR_CONST,imaxdiv_t,__LIBCCALL,imaxdiv,(__INTMAX_TYPE__ __numer, __INTMAX_TYPE__ __denom),lldiv,(__numer,__denom))
__REDIRECT(__LIBC,,__INTMAX_TYPE__, __LIBCCALL,strtoimax,(char const *__restrict __nptr, char **__restrict __endptr, int __base),strtoll,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,,__UINTMAX_TYPE__,__LIBCCALL,strtoumax,(char const *__restrict __nptr, char ** __restrict __endptr, int __base),strtoull,(__nptr,__endptr,__base))
__REDIRECT2(__LIBC,,__INTMAX_TYPE__, __LIBCCALL,wcstoimax,(__gwchar_t const *__restrict __nptr, __gwchar_t **__restrict __endptr, int __base),wcstoll,_wcstoll,(__nptr,__endptr,__base))
__REDIRECT2(__LIBC,,__UINTMAX_TYPE__,__LIBCCALL,wcstoumax,(__gwchar_t const *__restrict __nptr, __gwchar_t ** __restrict __endptr, int __base),wcstoull,_wcstoull,(__nptr,__endptr,__base))
#else
__LIBC __ATTR_CONST __INTMAX_TYPE__ (__LIBCCALL imaxabs)(__INTMAX_TYPE__ __n);
__LIBC __ATTR_CONST imaxdiv_t (__LIBCCALL imaxdiv)(__INTMAX_TYPE__ __numer, __INTMAX_TYPE__ __denom);
__LIBC __INTMAX_TYPE__ (__LIBCCALL strtoimax)(char const *__restrict __nptr, char **__restrict __endptr, int __base);
__LIBC __UINTMAX_TYPE__ (__LIBCCALL strtoumax)(char const *__restrict __nptr, char ** __restrict __endptr, int __base);
__LIBC __INTMAX_TYPE__ (__LIBCCALL wcstoimax)(__gwchar_t const *__restrict __nptr, __gwchar_t **__restrict __endptr, int __base);
__LIBC __UINTMAX_TYPE__ (__LIBCCALL wcstoumax)(__gwchar_t const *__restrict __nptr, __gwchar_t ** __restrict __endptr, int __base);
#endif
#endif /* !__KERNEL__ */

__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(imaxdiv_t)
#endif /* !__CXX_SYSTEM_HEADER */
#ifndef __KERNEL__
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(imaxabs)
__NAMESPACE_STD_USING(imaxdiv)
__NAMESPACE_STD_USING(strtoimax)
__NAMESPACE_STD_USING(strtoumax)
__NAMESPACE_STD_USING(wcstoimax)
__NAMESPACE_STD_USING(wcstoumax)
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __USE_DOS
#if __SIZEOF_INTMAX_T__ == __SIZEOF_LONG__
__REDIRECT2(__LIBC,,intmax_t, __LIBCCALL,_strtoimax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),strtoll_l,_strtoi64_l,(__nptr,__endptr,__radix,__locale))
__REDIRECT2(__LIBC,,uintmax_t,__LIBCCALL,_strtoumax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),strtoull_l,_strtoui64_l,(__nptr,__endptr,__radix,__locale))
__REDIRECT2(__LIBC,,intmax_t, __LIBCCALL,_wcstoimax_l,(__gwchar_t const *__restrict __nptr, __gwchar_t **__restrict __endptr, int __radix, __locale_t __locale),wcstoll_l,_wcstoi64_l,(__nptr,__endptr,__radix,__locale))
__REDIRECT2(__LIBC,,uintmax_t,__LIBCCALL,_wcstoumax_l,(__gwchar_t const *__restrict __nptr, __gwchar_t **__restrict __endptr, int __radix, __locale_t __locale),wcstoull_l,_wcstoui64_l,(__nptr,__endptr,__radix,__locale))
#elif __SIZEOF_INTMAX_T__ == __SIZEOF_LONG_LONG__
__REDIRECT2(__LIBC,,intmax_t, __LIBCCALL,_strtoimax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),strtol_l,_strtol_l,(__nptr,__endptr,__radix,__locale))
__REDIRECT2(__LIBC,,uintmax_t,__LIBCCALL,_strtoumax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),strtoul_l,_strtoul_l,(__nptr,__endptr,__radix,__locale))
__REDIRECT2(__LIBC,,intmax_t, __LIBCCALL,_wcstoimax_l,(__gwchar_t const *__restrict __nptr, __gwchar_t **__restrict __endptr, int __radix, __locale_t __locale),wcstol_l,_wcstol_l,(__nptr,__endptr,__radix,__locale))
__REDIRECT2(__LIBC,,uintmax_t,__LIBCCALL,_wcstoumax_l,(__gwchar_t const *__restrict __nptr, __gwchar_t **__restrict __endptr, int __radix, __locale_t __locale),wcstoul_l,_wcstoul_l,(__nptr,__endptr,__radix,__locale))
#else /* ... */
__LIBC intmax_t (__LIBCCALL _strtoimax_l)(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale);
__LIBC uintmax_t (__LIBCCALL _strtoumax_l)(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale);
__LIBC intmax_t (__LIBCCALL _wcstoimax_l)(__gwchar_t const *__restrict __nptr, __gwchar_t **__restrict __endptr, int __radix, __locale_t __locale);
__LIBC uintmax_t (__LIBCCALL _wcstoumax_l)(__gwchar_t const *__restrict __nptr, __gwchar_t **__restrict __endptr, int __radix, __locale_t __locale);
#endif /* !... */
#endif /* __USE_DOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_INTTYPES_H */
