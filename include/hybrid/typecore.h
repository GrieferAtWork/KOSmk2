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
#ifndef __GUARD_HYBRID_TYPECORE_H
#define __GUARD_HYBRID_TYPECORE_H 1

#include <__stdinc.h>

/* Autocomplete gcc-style compiler intrinsic predefined macros. */
#ifndef __SIZEOF_CHAR__
#define __SIZEOF_CHAR__  1
#endif
#ifndef __SIZEOF_SHORT__
#define __SIZEOF_SHORT__ 2
#endif
#ifndef __SIZEOF_INT__
#define __SIZEOF_INT__   4
#endif
#ifndef __SIZEOF_LONG__
#if defined(_WIN16) || defined(WIN16) || \
    defined(_WIN32) || defined(WIN32) || \
    defined(_WIN64) || defined(WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(_WIN32_WCE) || defined(WIN32_WCE)
#   define __SIZEOF_LONG__  4
#else
#   define __SIZEOF_LONG__  __SIZEOF_POINTER__
#endif
#endif
#ifndef __SIZEOF_LONG_LONG__
#ifdef __COMPILER_HAVE_LONGLONG
#   define __SIZEOF_LONG_LONG__  8
#endif
#endif

#ifndef __INT8_C
#if defined(_MSC_VER) || __has_extension(tpp_msvc_integer_suffix)
#   define __INT8_C(c)    c##i8
#   define __INT16_C(c)   c##i16
#   define __INT32_C(c)   c##i32
#   define __INT64_C(c)   c##i64
#   define __UINT8_C(c)   c##ui8
#   define __UINT16_C(c)  c##ui16
#   define __UINT32_C(c)  c##ui32
#   define __UINT64_C(c)  c##ui64
#if defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128
#   define __INT128_C(c)  c##i128
#   define __UINT128_C(c) c##ui128
#endif
#else /* _MSC_VER */
#   define __INT8_C(c)    c
#   define __INT16_C(c)   c
#if __SIZEOF_INT__ >= 4
#   define __INT32_C(c)   c
#elif __SIZEOF_LONG__ >= 4
#   define __INT32_C(c)   c##l
#else
#   error FIXME
#endif
#   define __INT64_C(c)   c##ll
#   define __UINT8_C(c)   c##u
#   define __UINT16_C(c)  c##u
#   define __UINT32_C(c)  c##u
#   define __UINT64_C(c)  c##ull
#endif /* ... */
#endif /* !__INT8_C */

#ifndef __CC__
/* Don't append anything when not hosting by a
 * C-compiler (e.g.: A linker script, or assembler) */
#undef __INT8_C
#undef __INT16_C
#undef __INT32_C
#undef __INT64_C
#undef __UINT8_C
#undef __UINT16_C
#undef __UINT32_C
#undef __UINT64_C
#define __INT8_C(c)    c
#define __INT16_C(c)   c
#define __INT32_C(c)   c
#define __INT64_C(c)   c
#define __UINT8_C(c)   c
#define __UINT16_C(c)  c
#define __UINT32_C(c)  c
#define __UINT64_C(c)  c
#endif

#define __PRIVATE_MIN_S1  (-__INT8_C(127)-__INT8_C(1))
#define __PRIVATE_MAX_S1    __INT8_C(127)
#define __PRIVATE_MIN_U1    __UINT8_C(0)
#define __PRIVATE_MAX_U1    __UINT8_C(0xff)
#define __PRIVATE_MIN_S2  (-__INT16_C(32767)-__INT16_C(1))
#define __PRIVATE_MAX_S2    __INT16_C(32767)
#define __PRIVATE_MIN_U2    __UINT16_C(0)
#define __PRIVATE_MAX_U2    __UINT16_C(0xffff)
#define __PRIVATE_MIN_S4  (-__INT32_C(2147483647)-__INT32_C(1))
#define __PRIVATE_MAX_S4    __INT32_C(2147483647)
#define __PRIVATE_MIN_U4    __UINT32_C(0)
#define __PRIVATE_MAX_U4    __UINT32_C(0xffffffff)
#define __PRIVATE_MIN_S8  (-__INT64_C(9223372036854775807)-__INT64_C(1))
#define __PRIVATE_MAX_S8    __INT64_C(9223372036854775807)
#define __PRIVATE_MIN_U8    __UINT64_C(0)
#define __PRIVATE_MAX_U8    __UINT64_C(0xffffffffffffffff)
#ifdef __INT128_C
#define __PRIVATE_MIN_S16 (-__INT128_C(170141183460469231731687303715884105727)-__INT128_C(1))
#define __PRIVATE_MAX_S16   __INT128_C(170141183460469231731687303715884105727)
#define __PRIVATE_MIN_U16   __UINT128_C(0)
#define __PRIVATE_MAX_U16   __UINT128_C(0xffffffffffffffffffffffffffffffff)
#endif

#ifndef __INTMAX_C
#define __INTMAX_C(c)  __INT64_C(c)
#define __UINTMAX_C(c) __UINT64_C(c)
#endif /* !__INTMAX_C */

#ifndef __SIZEOF_POINTER__
#   include <hybrid/host.h>
#if defined(__x86_64__) || defined(__aarch64__)
#   define __SIZEOF_POINTER__ 8
#elif defined(__i386__) || defined(__arm__)
#   define __SIZEOF_POINTER__ 4
#elif defined(__SIZEOF_SIZE_T__)
#   define __SIZEOF_POINTER__ __SIZEOF_SIZE_T__
#elif defined(__SIZEOF_PTRDIFF_T__)
#   define __SIZEOF_POINTER__ __SIZEOF_PTRDIFF_T__
#else
#ifdef __UINTPTR_MAX__
#if __UINTPTR_MAX__ == __PRIVATE_MAX_U4
#   define __SIZEOF_POINTER__ 4
#elif __UINTPTR_MAX__ == __PRIVATE_MAX_U8
#   define __SIZEOF_POINTER__ 8
#elif __UINTPTR_MAX__ == __PRIVATE_MAX_U2
#   define __SIZEOF_POINTER__ 2
#elif __UINTPTR_MAX__ == __PRIVATE_MAX_U1
#   define __SIZEOF_POINTER__ 1
#endif
#endif /* __UINTPTR_MAX__ */
#ifndef __SIZEOF_POINTER__
#ifdef __INTPTR_MAX__
#if __INTPTR_MAX__ == __PRIVATE_MAX_S4
#   define __SIZEOF_POINTER__ 4
#elif __INTPTR_MAX__ == __PRIVATE_MAX_S8
#   define __SIZEOF_POINTER__ 8
#elif __INTPTR_MAX__ == __PRIVATE_MAX_S2
#   define __SIZEOF_POINTER__ 2
#elif __INTPTR_MAX__ == __PRIVATE_MAX_S1
#   define __SIZEOF_POINTER__ 1
#endif
#endif /* __INTPTR_MAX__ */
#ifndef __SIZEOF_POINTER__
#   error FIXME
#endif /* !__SIZEOF_POINTER__ */
#endif /* !__SIZEOF_POINTER__ */
#endif /* ... */
#endif /* !__SIZEOF_POINTER__ */
#ifndef __SIZEOF_PTRDIFF_T__
#define __SIZEOF_PTRDIFF_T__ __SIZEOF_POINTER__
#endif
#ifndef __SIZEOF_SIZE_T__
#define __SIZEOF_SIZE_T__ __SIZEOF_POINTER__
#endif


#if __SIZEOF_POINTER__ == 8
#   define __INTPTR_C   __INT64_C
#   define __UINTPTR_C  __UINT64_C
#elif __SIZEOF_POINTER__ == 4
#   define __INTPTR_C   __INT32_C
#   define __UINTPTR_C  __UINT32_C
#elif __SIZEOF_POINTER__ == 2
#   define __INTPTR_C   __INT16_C
#   define __UINTPTR_C  __UINT16_C
#elif __SIZEOF_POINTER__ == 1
#   define __INTPTR_C   __INT8_C
#   define __UINTPTR_C  __UINT8_C
#else
#   error "Unsupported sizeof(void *)"
#endif
/* XXX: Assume that `__SIZEOF_SIZE_T__ == __SIZEOF_POINTER__' */
#define __SIZE_C    __UINTPTR_C
#define __SSIZE_C   __INTPTR_C



#ifndef __SIZEOF_INTMAX_T__
#define __SIZEOF_INTMAX_T__   8
#endif
#ifndef __SIZEOF_INT_LEAST8_T__
#define __SIZEOF_INT_LEAST8_T__  1
#endif /* !__SIZEOF_INT_LEAST8_T__ */
#ifndef __SIZEOF_INT_LEAST16_T__
#define __SIZEOF_INT_LEAST16_T__ 2
#endif /* !__SIZEOF_INT_LEAST16_T__ */
#ifndef __SIZEOF_INT_LEAST32_T__
#define __SIZEOF_INT_LEAST32_T__ 4
#endif /* !__SIZEOF_INT_LEAST32_T__ */
#ifndef __SIZEOF_INT_LEAST64_T__
#define __SIZEOF_INT_LEAST64_T__ 8
#endif /* !__SIZEOF_INT_LEAST64_T__ */

#ifndef __SIZEOF_INT_FAST8_T__
#ifdef __INT_FAST8_MAX__
#if __INT_FAST8_MAX__ == __PRIVATE_MAX_S1
#   define __SIZEOF_INT_FAST8_T__   1
#elif __INT_FAST8_MAX__ == __PRIVATE_MAX_S2
#   define __SIZEOF_INT_FAST8_T__   2
#elif __INT_FAST8_MAX__ == __PRIVATE_MAX_S4
#   define __SIZEOF_INT_FAST8_T__   4
#elif __INT_FAST8_MAX__ == __PRIVATE_MAX_S8
#   define __SIZEOF_INT_FAST8_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST8_T__
#ifdef __UINT_FAST8_MAX__
#if __UINT_FAST8_MAX__ == __PRIVATE_MAX_U1
#   define __SIZEOF_INT_FAST8_T__   1
#elif __UINT_FAST8_MAX__ == __PRIVATE_MAX_U2
#   define __SIZEOF_INT_FAST8_T__   2
#elif __UINT_FAST8_MAX__ == __PRIVATE_MAX_U4
#   define __SIZEOF_INT_FAST8_T__   4
#elif __UINT_FAST8_MAX__ == __PRIVATE_MAX_U8
#   define __SIZEOF_INT_FAST8_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST8_T__
#ifdef __INT_FAST8_MIN__
#if __INT_FAST8_MIN__ == __PRIVATE_MIN_S1
#   define __SIZEOF_INT_FAST8_T__   1
#elif __INT_FAST8_MIN__ == __PRIVATE_MIN_S2
#   define __SIZEOF_INT_FAST8_T__   2
#elif __INT_FAST8_MIN__ == __PRIVATE_MIN_S4
#   define __SIZEOF_INT_FAST8_T__   4
#elif __INT_FAST8_MIN__ == __PRIVATE_MIN_S8
#   define __SIZEOF_INT_FAST8_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST8_T__
#   define __SIZEOF_INT_FAST8_T__   1
#endif /* !__SIZEOF_INT_FAST8_T__ */
#endif /* !__SIZEOF_INT_FAST8_T__ */
#endif /* !__SIZEOF_INT_FAST8_T__ */
#endif /* !__SIZEOF_INT_FAST8_T__ */

#ifndef __SIZEOF_INT_FAST16_T__
#ifdef __INT_FAST16_MAX__
#if __INT_FAST16_MAX__ == __PRIVATE_MAX_S1
#   define __SIZEOF_INT_FAST16_T__   1
#elif __INT_FAST16_MAX__ == __PRIVATE_MAX_S2
#   define __SIZEOF_INT_FAST16_T__   2
#elif __INT_FAST16_MAX__ == __PRIVATE_MAX_S4
#   define __SIZEOF_INT_FAST16_T__   4
#elif __INT_FAST16_MAX__ == __PRIVATE_MAX_S8
#   define __SIZEOF_INT_FAST16_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST16_T__
#ifdef __UINT_FAST16_MAX__
#if __UINT_FAST16_MAX__ == __PRIVATE_MAX_U1
#   define __SIZEOF_INT_FAST16_T__   1
#elif __UINT_FAST16_MAX__ == __PRIVATE_MAX_U2
#   define __SIZEOF_INT_FAST16_T__   2
#elif __UINT_FAST16_MAX__ == __PRIVATE_MAX_U4
#   define __SIZEOF_INT_FAST16_T__   4
#elif __UINT_FAST16_MAX__ == __PRIVATE_MAX_U8
#   define __SIZEOF_INT_FAST16_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST16_T__
#ifdef __INT_FAST16_MIN__
#if __INT_FAST16_MIN__ == __PRIVATE_MIN_S1
#   define __SIZEOF_INT_FAST16_T__   1
#elif __INT_FAST16_MIN__ == __PRIVATE_MIN_S2
#   define __SIZEOF_INT_FAST16_T__   2
#elif __INT_FAST16_MIN__ == __PRIVATE_MIN_S4
#   define __SIZEOF_INT_FAST16_T__   4
#elif __INT_FAST16_MIN__ == __PRIVATE_MIN_S8
#   define __SIZEOF_INT_FAST16_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST16_T__
#   define __SIZEOF_INT_FAST16_T__   1
#endif /* !__SIZEOF_INT_FAST16_T__ */
#endif /* !__SIZEOF_INT_FAST16_T__ */
#endif /* !__SIZEOF_INT_FAST16_T__ */
#endif /* !__SIZEOF_INT_FAST16_T__ */

#ifndef __SIZEOF_INT_FAST32_T__
#ifdef __INT_FAST32_MAX__
#if __INT_FAST32_MAX__ == __PRIVATE_MAX_S1
#   define __SIZEOF_INT_FAST32_T__   1
#elif __INT_FAST32_MAX__ == __PRIVATE_MAX_S2
#   define __SIZEOF_INT_FAST32_T__   2
#elif __INT_FAST32_MAX__ == __PRIVATE_MAX_S4
#   define __SIZEOF_INT_FAST32_T__   4
#elif __INT_FAST32_MAX__ == __PRIVATE_MAX_S8
#   define __SIZEOF_INT_FAST32_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST32_T__
#ifdef __UINT_FAST32_MAX__
#if __UINT_FAST32_MAX__ == __PRIVATE_MAX_U1
#   define __SIZEOF_INT_FAST32_T__   1
#elif __UINT_FAST32_MAX__ == __PRIVATE_MAX_U2
#   define __SIZEOF_INT_FAST32_T__   2
#elif __UINT_FAST32_MAX__ == __PRIVATE_MAX_U4
#   define __SIZEOF_INT_FAST32_T__   4
#elif __UINT_FAST32_MAX__ == __PRIVATE_MAX_U8
#   define __SIZEOF_INT_FAST32_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST32_T__
#ifdef __INT_FAST32_MIN__
#if __INT_FAST32_MIN__ == __PRIVATE_MIN_S1
#   define __SIZEOF_INT_FAST32_T__   1
#elif __INT_FAST32_MIN__ == __PRIVATE_MIN_S2
#   define __SIZEOF_INT_FAST32_T__   2
#elif __INT_FAST32_MIN__ == __PRIVATE_MIN_S4
#   define __SIZEOF_INT_FAST32_T__   4
#elif __INT_FAST32_MIN__ == __PRIVATE_MIN_S8
#   define __SIZEOF_INT_FAST32_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST32_T__
#   define __SIZEOF_INT_FAST32_T__   1
#endif /* !__SIZEOF_INT_FAST32_T__ */
#endif /* !__SIZEOF_INT_FAST32_T__ */
#endif /* !__SIZEOF_INT_FAST32_T__ */
#endif /* !__SIZEOF_INT_FAST32_T__ */

#ifndef __SIZEOF_INT_FAST64_T__
#ifdef __INT_FAST64_MAX__
#if __INT_FAST64_MAX__ == __PRIVATE_MAX_S1
#   define __SIZEOF_INT_FAST64_T__   1
#elif __INT_FAST64_MAX__ == __PRIVATE_MAX_S2
#   define __SIZEOF_INT_FAST64_T__   2
#elif __INT_FAST64_MAX__ == __PRIVATE_MAX_S4
#   define __SIZEOF_INT_FAST64_T__   4
#elif __INT_FAST64_MAX__ == __PRIVATE_MAX_S8
#   define __SIZEOF_INT_FAST64_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST64_T__
#ifdef __UINT_FAST64_MAX__
#if __UINT_FAST64_MAX__ == __PRIVATE_MAX_U1
#   define __SIZEOF_INT_FAST64_T__   1
#elif __UINT_FAST64_MAX__ == __PRIVATE_MAX_U2
#   define __SIZEOF_INT_FAST64_T__   2
#elif __UINT_FAST64_MAX__ == __PRIVATE_MAX_U4
#   define __SIZEOF_INT_FAST64_T__   4
#elif __UINT_FAST64_MAX__ == __PRIVATE_MAX_U8
#   define __SIZEOF_INT_FAST64_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST64_T__
#ifdef __INT_FAST64_MIN__
#if __INT_FAST64_MIN__ == __PRIVATE_MIN_S1
#   define __SIZEOF_INT_FAST64_T__   1
#elif __INT_FAST64_MIN__ == __PRIVATE_MIN_S2
#   define __SIZEOF_INT_FAST64_T__   2
#elif __INT_FAST64_MIN__ == __PRIVATE_MIN_S4
#   define __SIZEOF_INT_FAST64_T__   4
#elif __INT_FAST64_MIN__ == __PRIVATE_MIN_S8
#   define __SIZEOF_INT_FAST64_T__   8
#endif
#endif
#ifndef __SIZEOF_INT_FAST64_T__
#   define __SIZEOF_INT_FAST64_T__   1
#endif /* !__SIZEOF_INT_FAST64_T__ */
#endif /* !__SIZEOF_INT_FAST64_T__ */
#endif /* !__SIZEOF_INT_FAST64_T__ */
#endif /* !__SIZEOF_INT_FAST64_T__ */

#ifndef __SIZEOF_SIG_ATOMIC_T__
#ifdef __SIG_ATOMIC_MIN__
#if __SIG_ATOMIC_MIN__ == __PRIVATE_MIN_S4
#   define __SIZEOF_SIG_ATOMIC_T__ 4
#elif __SIG_ATOMIC_MIN__ == __PRIVATE_MIN_S8
#   define __SIZEOF_SIG_ATOMIC_T__ 8
#elif __SIG_ATOMIC_MIN__ == __PRIVATE_MIN_S2
#   define __SIZEOF_SIG_ATOMIC_T__ 2
#elif __SIG_ATOMIC_MIN__ == __PRIVATE_MIN_S1
#   define __SIZEOF_SIG_ATOMIC_T__ 1
#elif __SIG_ATOMIC_MIN__ == __PRIVATE_MIN_U4
#   define __SIG_ATOMIC_UNSIGNED__ 1
#   define __SIZEOF_SIG_ATOMIC_T__ 4
#elif __SIG_ATOMIC_MIN__ == __PRIVATE_MIN_U8
#   define __SIG_ATOMIC_UNSIGNED__ 1
#   define __SIZEOF_SIG_ATOMIC_T__ 8
#elif __SIG_ATOMIC_MIN__ == __PRIVATE_MIN_U2
#   define __SIG_ATOMIC_UNSIGNED__ 1
#   define __SIZEOF_SIG_ATOMIC_T__ 2
#elif __SIG_ATOMIC_MIN__ == __PRIVATE_MIN_U1
#   define __SIG_ATOMIC_UNSIGNED__ 1
#   define __SIZEOF_SIG_ATOMIC_T__ 1
#endif
#endif /* __SIG_ATOMIC_MIN__ */
#ifndef __SIZEOF_SIG_ATOMIC_T__
#ifdef __SIG_ATOMIC_MAX__
#if __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_S4
#   define __SIZEOF_SIG_ATOMIC_T__ 4
#elif __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_S8
#   define __SIZEOF_SIG_ATOMIC_T__ 8
#elif __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_S2
#   define __SIZEOF_SIG_ATOMIC_T__ 2
#elif __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_S1
#   define __SIZEOF_SIG_ATOMIC_T__ 1
#elif __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_U4
#   define __SIG_ATOMIC_UNSIGNED__ 1
#   define __SIZEOF_SIG_ATOMIC_T__ 4
#elif __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_U8
#   define __SIG_ATOMIC_UNSIGNED__ 1
#   define __SIZEOF_SIG_ATOMIC_T__ 8
#elif __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_U2
#   define __SIG_ATOMIC_UNSIGNED__ 1
#   define __SIZEOF_SIG_ATOMIC_T__ 2
#elif __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_U1
#   define __SIG_ATOMIC_UNSIGNED__ 1
#   define __SIZEOF_SIG_ATOMIC_T__ 1
#endif
#endif /* __SIG_ATOMIC_MAX__ */
#ifndef __SIZEOF_SIG_ATOMIC_T__
#define __SIZEOF_SIG_ATOMIC_T__ __SIZEOF_INT__
#endif /* !__SIZEOF_SIG_ATOMIC_T__ */
#endif /* !__SIZEOF_SIG_ATOMIC_T__ */
#else /* !__SIZEOF_SIG_ATOMIC_T__ */
#if (defined(__SIG_ATOMIC_MIN__) && (__SIG_ATOMIC_MIN__ == 0)) || \
    (defined(__SIG_ATOMIC_MAX__) && (__SIG_ATOMIC_MAX__ == __PRIVATE_MAX_U4 || \
                                     __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_U8 || \
                                     __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_U2 || \
                                     __SIG_ATOMIC_MAX__ == __PRIVATE_MAX_U1))
#   define __SIG_ATOMIC_UNSIGNED__ 1
#endif /* ... */
#endif /* __SIZEOF_SIG_ATOMIC_T__ */

#ifndef __SIZEOF_WINT_T__
#ifdef __WINT_MIN__
#if __WINT_MIN__ == __PRIVATE_MIN_S4
#   define __SIZEOF_WINT_T__ 4
#elif __WINT_MIN__ == __PRIVATE_MIN_S2
#   define __SIZEOF_WINT_T__ 2
#elif __WINT_MIN__ == __PRIVATE_MIN_S8
#   define __SIZEOF_WINT_T__ 8
#elif __WINT_MIN__ == __PRIVATE_MIN_S1
#   define __SIZEOF_WINT_T__ 1
#elif __WINT_MIN__ == __PRIVATE_MIN_U4
#   undef  __WINT_UNSIGNED__
#   define __WINT_UNSIGNED__ 1
#   define __SIZEOF_WINT_T__ 4
#elif __WINT_MIN__ == __PRIVATE_MIN_U2
#   undef  __WINT_UNSIGNED__
#   define __WINT_UNSIGNED__ 1
#   define __SIZEOF_WINT_T__ 2
#elif __WINT_MIN__ == __PRIVATE_MIN_U8
#   undef  __WINT_UNSIGNED__
#   define __WINT_UNSIGNED__ 1
#   define __SIZEOF_WINT_T__ 8
#elif __WINT_MIN__ == __PRIVATE_MIN_U1
#   undef  __WINT_UNSIGNED__
#   define __WINT_UNSIGNED__ 1
#   define __SIZEOF_WINT_T__ 1
#endif
#endif /* __WINT_MIN__ */
#ifndef __SIZEOF_WINT_T__
#ifdef __WINT_MAX__
#if __WINT_MAX__ == __PRIVATE_MAX_S4
#   define __SIZEOF_WINT_T__ 4
#elif __WINT_MAX__ == __PRIVATE_MAX_S2
#   define __SIZEOF_WINT_T__ 2
#elif __WINT_MAX__ == __PRIVATE_MAX_S8
#   define __SIZEOF_WINT_T__ 8
#elif __WINT_MAX__ == __PRIVATE_MAX_S1
#   define __SIZEOF_WINT_T__ 1
#elif __WINT_MAX__ == __PRIVATE_MAX_U4
#   undef  __WINT_UNSIGNED__
#   define __WINT_UNSIGNED__ 1
#   define __SIZEOF_WINT_T__ 4
#elif __WINT_MAX__ == __PRIVATE_MAX_U2
#   undef  __WINT_UNSIGNED__
#   define __WINT_UNSIGNED__ 1
#   define __SIZEOF_WINT_T__ 2
#elif __WINT_MAX__ == __PRIVATE_MAX_U8
#   undef  __WINT_UNSIGNED__
#   define __WINT_UNSIGNED__ 1
#   define __SIZEOF_WINT_T__ 8
#elif __WINT_MAX__ == __PRIVATE_MAX_U1
#   undef  __WINT_UNSIGNED__
#   define __WINT_UNSIGNED__ 1
#   define __SIZEOF_WINT_T__ 1
#endif
#endif /* __WINT_MAX__ */
#ifndef __SIZEOF_WINT_T__
#if defined(__CYGWIN__) || defined(__CYGWIN32__) || \
    defined(__MINGW32__) || defined(WIN32) || defined(_WIN32) || \
    defined(WIN64) || defined(_WIN64)
#   define __SIZEOF_WINT_T__ 2
#   define __WINT_UNSIGNED__ 1
#else
#   define __SIZEOF_WINT_T__ __SIZEOF_INT__
#endif
#endif /* !__SIZEOF_WINT_T__ */
#endif /* !__SIZEOF_WINT_T__ */
#else /* !__SIZEOF_WINT_T__ */
#ifndef __WINT_UNSIGNED__
#if (defined(__WINT_MIN__) && (__WINT_MIN__ == 0)) || \
    (defined(__WINT_MAX__) && (__WINT_MAX__ == __PRIVATE_MAX_U4 || \
                               __WINT_MAX__ == __PRIVATE_MAX_U2 || \
                               __WINT_MAX__ == __PRIVATE_MAX_U1 || \
                               __WINT_MAX__ == __PRIVATE_MAX_U8))
#   define __WINT_UNSIGNED__ 1
#endif /* ... */
#endif /* !__WINT_UNSIGNED__ */
#endif /* __SIZEOF_WINT_T__ */


#ifndef __SIZEOF_WCHAR_T__
#ifdef __WCHAR_MIN__
#if __WCHAR_MIN__ == __PRIVATE_MIN_S4
#   define __SIZEOF_WCHAR_T__ 4
#elif __WCHAR_MIN__ == __PRIVATE_MIN_S2
#   define __SIZEOF_WCHAR_T__ 2
#elif __WCHAR_MIN__ == __PRIVATE_MIN_S8
#   define __SIZEOF_WCHAR_T__ 8
#elif __WCHAR_MIN__ == __PRIVATE_MIN_S1
#   define __SIZEOF_WCHAR_T__ 1
#elif __WCHAR_MIN__ == __PRIVATE_MIN_U4
#   undef  __WCHAR_UNSIGNED__
#   define __WCHAR_UNSIGNED__ 1
#   define __SIZEOF_WCHAR_T__ 4
#elif __WCHAR_MIN__ == __PRIVATE_MIN_U2
#   undef  __WCHAR_UNSIGNED__
#   define __WCHAR_UNSIGNED__ 1
#   define __SIZEOF_WCHAR_T__ 2
#elif __WCHAR_MIN__ == __PRIVATE_MIN_U8
#   undef  __WCHAR_UNSIGNED__
#   define __WCHAR_UNSIGNED__ 1
#   define __SIZEOF_WCHAR_T__ 8
#elif __WCHAR_MIN__ == __PRIVATE_MIN_U1
#   undef  __WCHAR_UNSIGNED__
#   define __WCHAR_UNSIGNED__ 1
#   define __SIZEOF_WCHAR_T__ 1
#endif
#endif /* __WCHAR_MIN__ */
#ifndef __SIZEOF_WCHAR_T__
#ifdef __WCHAR_MAX__
#if __WCHAR_MAX__ == __PRIVATE_MAX_S4
#   define __SIZEOF_WCHAR_T__ 4
#elif __WCHAR_MAX__ == __PRIVATE_MAX_S2
#   define __SIZEOF_WCHAR_T__ 2
#elif __WCHAR_MAX__ == __PRIVATE_MAX_S8
#   define __SIZEOF_WCHAR_T__ 8
#elif __WCHAR_MAX__ == __PRIVATE_MAX_S1
#   define __SIZEOF_WCHAR_T__ 1
#elif __WCHAR_MAX__ == __PRIVATE_MAX_U4
#   undef  __WCHAR_UNSIGNED__
#   define __WCHAR_UNSIGNED__ 1
#   define __SIZEOF_WCHAR_T__ 4
#elif __WCHAR_MAX__ == __PRIVATE_MAX_U2
#   undef  __WCHAR_UNSIGNED__
#   define __WCHAR_UNSIGNED__ 1
#   define __SIZEOF_WCHAR_T__ 2
#elif __WCHAR_MAX__ == __PRIVATE_MAX_U8
#   undef  __WCHAR_UNSIGNED__
#   define __WCHAR_UNSIGNED__ 1
#   define __SIZEOF_WCHAR_T__ 8
#elif __WCHAR_MAX__ == __PRIVATE_MAX_U1
#   undef  __WCHAR_UNSIGNED__
#   define __WCHAR_UNSIGNED__ 1
#   define __SIZEOF_WCHAR_T__ 1
#endif
#endif /* __WCHAR_MAX__ */
#ifndef __SIZEOF_WCHAR_T__
#if defined(__PE__)
#   define __SIZEOF_WCHAR_T__ 2
#else
#   define __SIZEOF_WCHAR_T__ 4
#endif
#endif /* !__SIZEOF_WCHAR_T__ */
#endif /* !__SIZEOF_WCHAR_T__ */
#else /* !__SIZEOF_WCHAR_T__ */
#ifndef __WCHAR_UNSIGNED__
#if (defined(__WCHAR_MIN__) && (__WCHAR_MIN__ == 0)) || \
    (defined(__WCHAR_MAX__) && (__WCHAR_MAX__ == __PRIVATE_MAX_U4 || \
                                __WCHAR_MAX__ == __PRIVATE_MAX_U2 || \
                                __WCHAR_MAX__ == __PRIVATE_MAX_U1 || \
                                __WCHAR_MAX__ == __PRIVATE_MAX_U8))
#   define __WCHAR_UNSIGNED__ 1
#endif /* ... */
#endif /* !__WCHAR_UNSIGNED__ */
#endif /* __SIZEOF_WCHAR_T__ */


#if defined(__INT8_TYPE__) && defined(__UINT8_TYPE__)
#   define __TYPEFOR_INTIB1  __INT8_TYPE__
#   define __TYPEFOR_UINTIB1 __UINT8_TYPE__
#elif __SIZEOF_CHAR__ == 1
#   define __TYPEFOR_INTIB1  signed char
#   define __TYPEFOR_UINTIB1 unsigned char
#elif __SIZEOF_SHORT__ == 1
#   define __TYPEFOR_INTIB1  signed short
#   define __TYPEFOR_UINTIB1 unsigned short
#elif __SIZEOF_INT__ == 1
#   define __TYPEFOR_INTIB1  signed int
#   define __TYPEFOR_UINTIB1 unsigned int
#elif __SIZEOF_LONG__ == 1
#   define __TYPEFOR_INTIB1  signed long
#   define __TYPEFOR_UINTIB1 unsigned long
#elif defined(__SIZEOF_LONG_LONG__) && __SIZEOF_LONG_LONG__ == 1
#   define __TYPEFOR_INTIB1  signed long long
#   define __TYPEFOR_UINTIB1 unsigned long long
#else
#   error FIXME
#endif
#if defined(__INT16_TYPE__) && defined(__UINT16_TYPE__)
#   define __TYPEFOR_INTIB2  __INT16_TYPE__
#   define __TYPEFOR_UINTIB2 __UINT16_TYPE__
#elif __SIZEOF_SHORT__ == 2
#   define __TYPEFOR_INTIB2  signed short
#   define __TYPEFOR_UINTIB2 unsigned short
#elif __SIZEOF_INT__ == 2
#   define __TYPEFOR_INTIB2  signed int
#   define __TYPEFOR_UINTIB2 unsigned int
#elif __SIZEOF_LONG__ == 2
#   define __TYPEFOR_INTIB2  signed long
#   define __TYPEFOR_UINTIB2 unsigned long
#elif defined(__SIZEOF_LONG_LONG__) && __SIZEOF_LONG_LONG__ == 2
#   define __TYPEFOR_INTIB2  signed long long
#   define __TYPEFOR_UINTIB2 unsigned long long
#elif __SIZEOF_CHAR__ == 2
#   define __TYPEFOR_INTIB2  signed char
#   define __TYPEFOR_UINTIB2 unsigned char
#else
#   error FIXME
#endif
#if defined(__INT32_TYPE__) && defined(__UINT32_TYPE__)
#   define __TYPEFOR_INTIB4  __INT32_TYPE__
#   define __TYPEFOR_UINTIB4 __UINT32_TYPE__
#elif __SIZEOF_INT__ == 4
#   define __TYPEFOR_INTIB4  signed int
#   define __TYPEFOR_UINTIB4 unsigned int
#elif __SIZEOF_LONG__ == 4
#   define __TYPEFOR_INTIB4  signed long
#   define __TYPEFOR_UINTIB4 unsigned long
#elif defined(__SIZEOF_LONG_LONG__) && __SIZEOF_LONG_LONG__ == 4
#   define __TYPEFOR_INTIB4  signed long long
#   define __TYPEFOR_UINTIB4 unsigned long long
#elif __SIZEOF_SHORT__ == 4
#   define __TYPEFOR_INTIB4  signed short
#   define __TYPEFOR_UINTIB4 unsigned short
#elif __SIZEOF_CHAR__ == 4
#   define __TYPEFOR_INTIB4  signed char
#   define __TYPEFOR_UINTIB4 unsigned char
#else
#   error FIXME
#endif
#if defined(__INT64_TYPE__) && defined(__UINT64_TYPE__)
#   define __TYPEFOR_INTIB8  __INT64_TYPE__
#   define __TYPEFOR_UINTIB8 __UINT64_TYPE__
#elif defined(__SIZEOF_LONG_LONG__) && __SIZEOF_LONG_LONG__ == 8
#   define __TYPEFOR_INTIB8  signed long long
#   define __TYPEFOR_UINTIB8 unsigned long long
#elif __SIZEOF_LONG__ == 8
#   define __TYPEFOR_INTIB8  signed long
#   define __TYPEFOR_UINTIB8 unsigned long
#elif __SIZEOF_INT__ == 8
#   define __TYPEFOR_INTIB8  signed int
#   define __TYPEFOR_UINTIB8 unsigned int
#elif __SIZEOF_SHORT__ == 8
#   define __TYPEFOR_INTIB8  signed short
#   define __TYPEFOR_UINTIB8 unsigned short
#elif __SIZEOF_CHAR__ == 8
#   define __TYPEFOR_INTIB8  signed char
#   define __TYPEFOR_UINTIB8 unsigned char
#endif

#define __PRIVATE_TYPEFOR_INTIB(n)  __TYPEFOR_INTIB##n
#define __PRIVATE_TYPEFOR_UINTIB(n) __TYPEFOR_UINTIB##n
#define __TYPEFOR_INTIB(n)  __PRIVATE_TYPEFOR_INTIB(n)
#define __TYPEFOR_UINTIB(n) __PRIVATE_TYPEFOR_UINTIB(n)

#ifndef __CHAR8_TYPE__
#if __SIZEOF_CHAR__ == 1
#   define __CHAR8_TYPE__ char
#else
#   define __CHAR8_TYPE__ __INT8_TYPE__
#endif
#endif
#ifndef __INT8_TYPE__
#define __INT8_TYPE__   __TYPEFOR_INTIB1
#endif
#ifndef __INT16_TYPE__
#define __INT16_TYPE__  __TYPEFOR_INTIB2
#endif
#ifndef __INT32_TYPE__
#define __INT32_TYPE__  __TYPEFOR_INTIB4
#endif
#ifndef __UINT8_TYPE__
#define __UINT8_TYPE__  __TYPEFOR_UINTIB1
#endif
#ifndef __UINT16_TYPE__
#define __UINT16_TYPE__ __TYPEFOR_UINTIB2
#endif
#ifndef __UINT32_TYPE__
#define __UINT32_TYPE__ __TYPEFOR_UINTIB4
#endif
#ifndef __INT64_TYPE__
#ifdef __TYPEFOR_INTIB8
#define __INT64_TYPE__  __TYPEFOR_INTIB8
#endif /* __TYPEFOR_INTIB8 */
#endif /* !__INT64_TYPE__ */
#ifndef __UINT64_TYPE__
#ifdef __TYPEFOR_UINTIB8
#define __UINT64_TYPE__ __TYPEFOR_UINTIB8
#endif /* __TYPEFOR_UINTIB8 */
#endif /* !__UINT64_TYPE__ */

#ifndef __INT_LEAST8_TYPE__
#define __INT_LEAST8_TYPE__   __TYPEFOR_INTIB(__SIZEOF_INT_LEAST8_T__)
#endif /* !__INT_LEAST8_TYPE__ */
#ifndef __INT_LEAST16_TYPE__
#define __INT_LEAST16_TYPE__  __TYPEFOR_INTIB(__SIZEOF_INT_LEAST16_T__)
#endif /* !__INT_LEAST16_TYPE__ */
#ifndef __INT_LEAST32_TYPE__
#define __INT_LEAST32_TYPE__  __TYPEFOR_INTIB(__SIZEOF_INT_LEAST32_T__)
#endif /* !__INT_LEAST32_TYPE__ */
#ifndef __INT_LEAST64_TYPE__
#define __INT_LEAST64_TYPE__  __TYPEFOR_INTIB(__SIZEOF_INT_LEAST64_T__)
#endif /* !__INT_LEAST64_TYPE__ */
#ifndef __UINT_LEAST8_TYPE__
#define __UINT_LEAST8_TYPE__  __TYPEFOR_UINTIB(__SIZEOF_INT_LEAST8_T__)
#endif /* !__UINT_LEAST8_TYPE__ */
#ifndef __UINT_LEAST16_TYPE__
#define __UINT_LEAST16_TYPE__ __TYPEFOR_UINTIB(__SIZEOF_INT_LEAST16_T__)
#endif /* !__UINT_LEAST16_TYPE__ */
#ifndef __UINT_LEAST32_TYPE__
#define __UINT_LEAST32_TYPE__ __TYPEFOR_UINTIB(__SIZEOF_INT_LEAST32_T__)
#endif /* !__UINT_LEAST32_TYPE__ */
#ifndef __UINT_LEAST64_TYPE__
#define __UINT_LEAST64_TYPE__ __TYPEFOR_UINTIB(__SIZEOF_INT_LEAST64_T__)
#endif /* !__UINT_LEAST64_TYPE__ */
#ifndef __INT_FAST8_TYPE__
#define __INT_FAST8_TYPE__    __TYPEFOR_INTIB(__SIZEOF_INT_FAST8_T__)
#endif /* !__INT_FAST8_TYPE__ */
#ifndef __INT_FAST16_TYPE__
#define __INT_FAST16_TYPE__   __TYPEFOR_INTIB(__SIZEOF_INT_FAST16_T__)
#endif /* !__INT_FAST16_TYPE__ */
#ifndef __INT_FAST32_TYPE__
#define __INT_FAST32_TYPE__   __TYPEFOR_INTIB(__SIZEOF_INT_FAST32_T__)
#endif /* !__INT_FAST32_TYPE__ */
#ifndef __INT_FAST64_TYPE__
#define __INT_FAST64_TYPE__   __TYPEFOR_INTIB(__SIZEOF_INT_FAST64_T__)
#endif /* !__INT_FAST64_TYPE__ */
#ifndef __UINT_FAST8_TYPE__
#define __UINT_FAST8_TYPE__   __TYPEFOR_UINTIB(__SIZEOF_INT_FAST8_T__)
#endif /* !__UINT_FAST8_TYPE__ */
#ifndef __UINT_FAST16_TYPE__
#define __UINT_FAST16_TYPE__  __TYPEFOR_UINTIB(__SIZEOF_INT_FAST16_T__)
#endif /* !__UINT_FAST16_TYPE__ */
#ifndef __UINT_FAST32_TYPE__
#define __UINT_FAST32_TYPE__  __TYPEFOR_UINTIB(__SIZEOF_INT_FAST32_T__)
#endif /* !__UINT_FAST32_TYPE__ */
#ifndef __UINT_FAST64_TYPE__
#define __UINT_FAST64_TYPE__  __TYPEFOR_UINTIB(__SIZEOF_INT_FAST64_T__)
#endif /* !__UINT_FAST64_TYPE__ */


#ifndef __INTPTR_TYPE__
#define __INTPTR_TYPE__  __TYPEFOR_INTIB(__SIZEOF_POINTER__)
#endif
#ifndef __UINTPTR_TYPE__
#define __UINTPTR_TYPE__ __TYPEFOR_UINTIB(__SIZEOF_POINTER__)
#endif
#ifndef __PTRDIFF_TYPE__
#define __PTRDIFF_TYPE__ __TYPEFOR_INTIB(__SIZEOF_PTRDIFF_T__)
#endif
#ifndef __SIZE_TYPE__
#define __SIZE_TYPE__    __TYPEFOR_UINTIB(__SIZEOF_SIZE_T__)
#endif
#ifndef __INTMAX_TYPE__
#define __INTMAX_TYPE__  __INT64_TYPE__
#endif
#ifndef __UINTMAX_TYPE__
#define __UINTMAX_TYPE__ __UINT64_TYPE__
#endif

#if (__SIZEOF_LONG__ == __SIZEOF_POINTER__) && \
    (!defined(__x86_64__) || !defined(__INTELLISENSE__))
#define __LONGPTR_TYPE__  signed long
#define __ULONGPTR_TYPE__ unsigned long
#else
#define __LONGPTR_TYPE__  __INTPTR_TYPE__
#define __ULONGPTR_TYPE__ __UINTPTR_TYPE__
#endif


#ifndef __SSIZE_TYPE__
#define __SSIZE_TYPE__   __TYPEFOR_INTIB(__SIZEOF_SIZE_T__)
#endif
#ifndef __BYTE_TYPE__
#define __SBYTE_TYPE__   __TYPEFOR_INTIB1
#define __BYTE_TYPE__    __TYPEFOR_UINTIB1
#endif
#ifndef __WCHAR_TYPE__
#if defined(__native_wchar_t_defined)
#   define __WCHAR_TYPE__ wchar_t
#elif defined(__WCHAR_UNSIGNED__)
#   define __WCHAR_TYPE__ __TYPEFOR_UINTIB(__SIZEOF_WCHAR_T__)
#else
#   define __WCHAR_TYPE__ __TYPEFOR_INTIB(__SIZEOF_WCHAR_T__)
#endif
#endif
#ifndef __WINT_TYPE__
#ifdef __WINT_UNSIGNED__
#   define __WINT_TYPE__  __TYPEFOR_UINTIB(__SIZEOF_WINT_T__)
#else
#   define __WINT_TYPE__  __TYPEFOR_INTIB(__SIZEOF_WINT_T__)
#endif
#endif
#ifndef __WCTYPE_TYPE__
#if defined(__CYGWIN__) || defined(__CYGWIN32__) || \
    defined(__MINGW32__) || defined(WIN32) || defined(_WIN32) || \
    defined(WIN64) || defined(_WIN64)
#   define __WCTYPE_TYPE__      __UINT16_TYPE__
#   define __SIZEOF_WCTYPE_T__  2
#else
#   define __WCTYPE_TYPE__      __ULONGPTR_TYPE__
#   define __SIZEOF_WCTYPE_T__  __SIZEOF_POINTER__
#endif
#endif /* !__WCTYPE_TYPE__ */
#ifndef __CHAR16_TYPE__
#define __CHAR16_TYPE__ __UINT16_TYPE__
#endif
#ifndef __CHAR32_TYPE__
#define __CHAR32_TYPE__ __UINT32_TYPE__
#endif
#ifndef __SIG_ATOMIC_TYPE__
#define __SIG_ATOMIC_TYPE__ int
#endif

#if __SIZEOF_LONG__ == __SIZEOF_SIZE_T__
#   define __LONGSIZE_TYPE__  unsigned long int
#   define __LONGSSIZE_TYPE__ long int
#else
#   define __LONGSIZE_TYPE__  __SIZE_TYPE__
#   define __LONGSSIZE_TYPE__ __SSIZE_TYPE__
#endif

#if __SIZEOF_LONG__ == 4
#define __LONG32_TYPE__  long int
#define __ULONG32_TYPE__ unsigned long int
#else
#define __LONG32_TYPE__  __INT32_TYPE__
#define __ULONG32_TYPE__ __UINT32_TYPE__
#endif

#if __SIZEOF_LONG__ == 8
#define __LONG64_TYPE__  long int
#define __ULONG64_TYPE__ unsigned long int
#else
#define __LONG64_TYPE__  __INT64_TYPE__
#define __ULONG64_TYPE__ __UINT64_TYPE__
#endif

/* The max integral type that the target's main address bus is optimized for.
 * Currently, that is equal to `sizeof(void *)' for all targets. */
#ifndef __SIZEOF_BUSINT__
#define __SIZEOF_BUSINT__ __SIZEOF_POINTER__
#define __BUSINT_TYPE__   __INTPTR_TYPE__
#define __UBUSIINT_TYPE__ __UINTPTR_TYPE__
#endif

/* General-purpose register type.
 * Currently, that is equal to `sizeof(void *)' for all targets. */
#ifndef __SIZEOF_REGISTER__
#define __SIZEOF_REGISTER__  __SIZEOF_POINTER__
#define __REGISTER_TYPE__    __ULONGPTR_TYPE__
#define __SREGISTER_TYPE__   __LONGPTR_TYPE__
#endif


#if defined(_NATIVE_CHAR16_T_DEFINED) || \
   (defined(__cpp_unicode_characters) && __cpp_unicode_characters+0 >= 200704) || \
   (defined(_HAS_CHAR16_T_LANGUAGE_SUPPORT) && _HAS_CHAR16_T_LANGUAGE_SUPPORT+0) || \
   (defined(__cplusplus) && (defined(__CODEGEARC__) || (defined(_MSC_VER) && _MSC_VER+0 >= 1900) || \
   (defined(__clang__) && !defined(_MSC_VER) && (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L)) || \
   (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GCC_VERSION(4,4,0)) || \
   (defined(__BORLANDC__) && defined(__CODEGEAR_0X_SUPPORT__) && __BORLANDC__+0 >= 0x610) || \
   (defined(__IBMCPP_UTF_LITERAL__) && __IBMCPP_UTF_LITERAL__)))
/* The compiler is pre-defining the `char16_t' / `char32_t' types. */
#undef __CHAR16_TYPE__
#undef __CHAR32_TYPE__
#define __CHAR16_TYPE__ char16_t
#define __CHAR32_TYPE__ char32_t
#define __char16_t_defined        1
#define __native_char16_t_defined 1
#endif


#endif /* !__GUARD_HYBRID_TYPECORE_H */
