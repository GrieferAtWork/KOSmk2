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

#if !defined(__ASSERT_ENABLED) || \
     defined(NDEBUG) == __ASSERT_ENABLED
#undef __ASSERT_ENABLED
#ifdef NDEBUG
#   define __ASSERT_ENABLED 0
#else
#   define __ASSERT_ENABLED 1
#endif

#include "__stdinc.h"

#ifdef __CC__
#include <features.h>
#include <hybrid/debuginfo.h>
#include <hybrid/typecore.h>

#ifndef __assertion_failed_defined
#define __assertion_failed_defined 1
__LIBC                   __SSIZE_TYPE__ (__LIBCCALL __assertion_print)(char const *__data, __SIZE_TYPE__ __datalen, void *__ignored_closure);
__LIBC                             void (           __assertion_printf)(char const *__format, ...);
__LIBC                             void (__LIBCCALL __assertion_vprintf)(char const *__format, __VA_LIST args);
__LIBC __ATTR_NORETURN __ATTR_COLD void (__LIBCCALL __assertion_failed)(char const *__expr, __DEBUGINFO);
__LIBC __ATTR_NORETURN __ATTR_COLD void (           __assertion_failedf)(char const *__expr, __DEBUGINFO, char const *__format, ...);
#   define __yes_assert(sexpr,expr)         (void)(__likely(expr) || (__assertion_failed(sexpr,__DEBUGINFO_GEN),0))
#   define __yes_assertf(sexpr,expr,...)    (void)(__likely(expr) || (__assertion_failedf(sexpr,__DEBUGINFO_GEN,__VA_ARGS__),0))
#   define __yes_asserte(sexpr,expr)        (void)(__likely(expr) || (__assertion_failed(sexpr,__DEBUGINFO_GEN),0))
#   define __yes_assertef(sexpr,expr,...)   (void)(__likely(expr) || (__assertion_failedf(sexpr,__DEBUGINFO_GEN,__VA_ARGS__),0))
#   define __yes_assert_d(sexpr,expr,...)   (void)(__likely(expr) || (__assertion_failed(sexpr,__VA_ARGS__),0))
#   define __yes_assertf_d(sexpr,expr,...)  (void)(__likely(expr) || (__assertion_failedf(sexpr,__VA_ARGS__),0))
#   define __yes_asserte_d(sexpr,expr,...)  (void)(__likely(expr) || (__assertion_failed(sexpr,__VA_ARGS__),0))
#   define __yes_assertef_d(sexpr,expr,...) (void)(__likely(expr) || (__assertion_failedf(sexpr,__VA_ARGS__),0))
#ifdef __OPTIMIZE__
#   define __no_assert(sexpr,expr)           __builtin_assume(expr)
#   define __no_assertf(sexpr,expr,...)      __builtin_assume(expr)
#   define __no_assert_d(sexpr,expr,...)     __builtin_assume(expr)
#   define __no_assertf_d(sexpr,expr,...)    __builtin_assume(expr)
#else
#   define __no_assert(sexpr,expr)          (void)0
#   define __no_assertf(sexpr,expr,...)     (void)0
#   define __no_assert_d(sexpr,expr,...)    (void)0      
#   define __no_assertf_d(sexpr,expr,...)   (void)0      
#endif
#   define __no_asserte(sexpr,expr)         (void)(__likely(expr) || (__assertion_failed(sexpr,__DEBUGINFO_GEN),0))
#   define __no_assertef(sexpr,expr,...)    (void)(__likely(expr) || (__assertion_failedf(sexpr,__DEBUGINFO_GEN,__VA_ARGS__),0))
#   define __no_asserte_d(sexpr,expr,...)   (void)(__likely(expr) || (__assertion_failed(sexpr,__VA_ARGS__),0))
#   define __no_assertef_d(sexpr,expr,...)  (void)(__likely(expr) || (__assertion_failedf(sexpr,__VA_ARGS__),0))
#ifdef __USE_KOS
#   define assertf     __assertf
#   define asserte     __asserte
#   define assertef    __assertef
#endif
#ifdef __USE_KXS
#   define assert_d    __assert_d
#   define assertf_d   __assertf_d
#   define asserte_d   __asserte_d
#   define assertef_d  __assertef_d
#endif
#endif /* __assertion_failed_defined */

#undef assert
#undef __assertf
#undef __assert_d
#undef __assertf_d
#undef __asserte
#undef __assertef
#undef __asserte_d
#undef __assertef_d

#if __ASSERT_ENABLED
#   define assert(expr)           __yes_assert(#expr,expr)
#   define __assertf(expr,...)    __yes_assertf(#expr,expr,__VA_ARGS__)
#   define __assert_d(expr,...)   __yes_assert_d(#expr,expr,__VA_ARGS__)
#   define __assertf_d(expr,...)  __yes_assertf_d(#expr,expr,__VA_ARGS__)
#   define __asserte(expr)        __yes_asserte(#expr,expr)
#   define __assertef(expr,...)   __yes_assertef(#expr,expr,__VA_ARGS__)
#   define __asserte_d(expr,...)  __yes_asserte_d(#expr,expr,__VA_ARGS__)
#   define __assertef_d(expr,...) __yes_assertef_d(#expr,expr,__VA_ARGS__)
#else
#   define assert(expr)           __no_assert(#expr,expr)
#   define __assertf(expr,...)    __no_assertf(#expr,expr,__VA_ARGS__)
#   define __assert_d(expr,...)   __no_assert_d(#expr,expr,__VA_ARGS__)
#   define __assertf_d(expr,...)  __no_assertf_d(#expr,expr,__VA_ARGS__)
#   define __asserte(expr)        __no_asserte(#expr,expr)
#   define __assertef(expr,...)   __no_assertef(#expr,expr,__VA_ARGS__)
#   define __asserte_d(expr,...)  __no_asserte_d(#expr,expr,__VA_ARGS__)
#   define __assertef_d(expr,...) __no_assertef_d(#expr,expr,__VA_ARGS__)
#endif
#endif /* __CC__ */

#endif /* Changed... */
