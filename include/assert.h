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

#ifndef __assertion_failed_defined
#define __assertion_failed_defined 1
#ifdef __CC__
#include <features.h>
#include <hybrid/debuginfo.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#if defined(__NO_ATTR_COLD) && !defined(__NO_builtin_expect)
/* Without `__attribute__((cold))' to hist branch prediction,
 * use `__builtin_expect' itself to coax the compiler into
 * prediction that the assertion will not fail. */
#define __ASSERT_LIKELY __likely
#else
/* Since assertions appear as often as they do (especially in the kernel),
 * try to keep the generated preprocessor output smaller by only making use
 * of `__attribute__((cold))' branch prediction if available. */
#define __ASSERT_LIKELY
#endif

#if defined(__CRT_KOS) && (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__NAMESPACE_INT_BEGIN
__LIBC __ATTR_NORETURN __ATTR_COLD void (__LIBCCALL __afail)(char const *__expr, __DEBUGINFO);
__LIBC __ATTR_NORETURN __ATTR_COLD void (           __afailf)(char const *__expr, __DEBUGINFO, char const *__format, ...);
__NAMESPACE_INT_END
#   define __yes_assert(sexpr,expr)         (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afail(sexpr,__DEBUGINFO_GEN),0))
#   define __yes_assertf(sexpr,expr,...)    (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afailf(sexpr,__DEBUGINFO_GEN,__VA_ARGS__),0))
#   define __yes_asserte(sexpr,expr)        (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afail(sexpr,__DEBUGINFO_GEN),0))
#   define __yes_assertef(sexpr,expr,...)   (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afailf(sexpr,__DEBUGINFO_GEN,__VA_ARGS__),0))
#   define __yes_assert_d(sexpr,expr,...)   (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afail(sexpr,__VA_ARGS__),0))
#   define __yes_assertf_d(sexpr,expr,...)  (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afailf(sexpr,__VA_ARGS__),0))
#   define __yes_asserte_d(sexpr,expr,...)  (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afail(sexpr,__VA_ARGS__),0))
#   define __yes_assertef_d(sexpr,expr,...) (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afailf(sexpr,__VA_ARGS__),0))
#elif defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__LIBC __ATTR_NORETURN __ATTR_COLD void (__LIBCCALL __assert_fail)(const char *__assertion, const char *__file, unsigned int __line, const char *__function);
#   define __yes_assert(sexpr,expr)         (void)(__ASSERT_LIKELY(expr) || (__assert_fail(sexpr,__FILE__,__LINE__,__FUNCTION__),0))
#   define __yes_asserte(sexpr,expr)        (void)(__ASSERT_LIKELY(expr) || (__assert_fail(sexpr,__FILE__,__LINE__,__FUNCTION__),0))
#   define __yes_assertf(sexpr,expr,...)    (void)(__ASSERT_LIKELY(expr) || (__assert_fail(sexpr,__FILE__,__LINE__,__FUNCTION__),0))
#   define __yes_assertef(sexpr,expr,...)   (void)(__ASSERT_LIKELY(expr) || (__assert_fail(sexpr,__FILE__,__LINE__,__FUNCTION__),0))
#   define __IMPL2_yes_assert_d(sexpr,expr,file,line,func,...) (void)(__ASSERT_LIKELY(expr) || (__assert_fail(sexpr,file,line,func),0))
#   define __IMPL_yes_assert_d(args)        __IMPL2_yes_assert_d args
#   define __yes_assert_d(sexpr,expr,...)   __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#   define __yes_assertf_d(sexpr,expr,...)  __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#   define __yes_asserte_d(sexpr,expr,...)  __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#   define __yes_assertef_d(sexpr,expr,...) __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#elif defined(__CRT_DOS)
__LIBC __ATTR_NORETURN __ATTR_COLD void (__LIBCCALL _assert)(const char *__assertion, const char *__file, int __line);
#   define __yes_assert(sexpr,expr)         (void)(__ASSERT_LIKELY(expr) || (_assert(sexpr,__FILE__,__LINE__),0))
#   define __yes_asserte(sexpr,expr)        (void)(__ASSERT_LIKELY(expr) || (_assert(sexpr,__FILE__,__LINE__),0))
#   define __yes_assertf(sexpr,expr,...)    (void)(__ASSERT_LIKELY(expr) || (_assert(sexpr,__FILE__,__LINE__),0))
#   define __yes_assertef(sexpr,expr,...)   (void)(__ASSERT_LIKELY(expr) || (_assert(sexpr,__FILE__,__LINE__),0))
#   define __IMPL2_yes_assert_d(sexpr,expr,file,line,func,...) (void)(__ASSERT_LIKELY(expr) || (_assert(sexpr,file,line,func),0))
#   define __IMPL_yes_assert_d(args)        __IMPL2_yes_assert_d args
#   define __yes_assert_d(sexpr,expr,...)   __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#   define __yes_assertf_d(sexpr,expr,...)  __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#   define __yes_asserte_d(sexpr,expr,...)  __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#   define __yes_assertef_d(sexpr,expr,...) __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#elif defined(__CRT_CYG)
__LIBC __ATTR_NORETURN __ATTR_COLD void (__LIBCCALL __assert_func)(const char *__file, int __line, const char *__func, const char *__expr);
#ifdef __NO_FUNCTION__
__LIBC __ATTR_NORETURN __ATTR_COLD void (__LIBCCALL __assert)(const char *__file, int __line, const char *__expr);
#   define __yes_assert(sexpr,expr)         (void)(__ASSERT_LIKELY(expr) || (__assert(__FILE__,__LINE__,sexpr),0))
#   define __yes_asserte(sexpr,expr)        (void)(__ASSERT_LIKELY(expr) || (__assert(__FILE__,__LINE__,sexpr),0))
#   define __yes_assertf(sexpr,expr,...)    (void)(__ASSERT_LIKELY(expr) || (__assert(__FILE__,__LINE__,sexpr),0))
#   define __yes_assertef(sexpr,expr,...)   (void)(__ASSERT_LIKELY(expr) || (__assert(__FILE__,__LINE__,sexpr),0))
#   define __IMPL2_yes_assert_d(sexpr,expr,file,line,func,...) (void)(__ASSERT_LIKELY(expr) || (__assert_func(file,line,func,sexpr),0))
#else /* __NO_FUNCTION__ */
#   define __yes_assert(sexpr,expr)         (void)(__ASSERT_LIKELY(expr) || (__assert_func(__FILE__,__LINE__,__FUNCTION__,sexpr),0))
#   define __yes_asserte(sexpr,expr)        (void)(__ASSERT_LIKELY(expr) || (__assert_func(__FILE__,__LINE__,__FUNCTION__,sexpr),0))
#   define __yes_assertf(sexpr,expr,...)    (void)(__ASSERT_LIKELY(expr) || (__assert_func(__FILE__,__LINE__,__FUNCTION__,sexpr),0))
#   define __yes_assertef(sexpr,expr,...)   (void)(__ASSERT_LIKELY(expr) || (__assert_func(__FILE__,__LINE__,__FUNCTION__,sexpr),0))
#   define __IMPL2_yes_assert_d(sexpr,expr,file,line,func,...) (void)(__ASSERT_LIKELY(expr) || (__assert_func(file,line,func,sexpr),0))
#endif /* !__NO_FUNCTION__ */
#   define __IMPL_yes_assert_d(args)        __IMPL2_yes_assert_d args
#   define __yes_assert_d(sexpr,expr,...)   __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#   define __yes_assertf_d(sexpr,expr,...)  __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#   define __yes_asserte_d(sexpr,expr,...)  __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#   define __yes_assertef_d(sexpr,expr,...) __IMPL_yes_assert_d((sexpr,expr,__VA_ARGS__))
#else
#   error "Not way of implementing assert()"
#endif

#ifndef __KERNEL__
#ifdef __CRT_GLC
__LIBC __ATTR_NORETURN __ATTR_COLD void (__LIBCCALL __assert_perror_fail)(int __errnum, const char *__file, unsigned int __line, const char *__function);
#endif /* __CRT_GLC */
__REDIRECT_IFDOS_VOID(__LIBC,__ATTR_NORETURN __ATTR_COLD,__LIBCCALL,__assert,(const char *__assertion, const char *__file, int __line),_assert,(__assertion,__file,__line))
#endif /* !__KERNEL__ */

#ifdef __CRT_GLC
#   define __yes_assert_perror(e_num) \
   (void)(!(e_num) || (__assert_perror_fail((e_num),__FILE__,__LINE__,__FUNCTION__),0))
#else
#   define __yes_assert_perror(e_num) (ASSERT_PERROR_FAIL_IS_NOT_DEFINED)
#endif

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
#   define __no_asserte(sexpr,expr)         (void)(expr)
#   define __no_assertef(sexpr,expr,...)    (void)(expr)
#   define __no_asserte_d(sexpr,expr,...)   (void)(expr)
#   define __no_assertef_d(sexpr,expr,...)  (void)(expr)
#   define __no_assert_perror(e_num)        (void)0
#ifdef __USE_KOS
#   define assertf     __assertf
#   define asserte     __asserte
#   define assertef    __assertef
#endif /* __USE_KOS */
#ifdef __USE_KXS
#   define assert_d    __assert_d
#   define assertf_d   __assertf_d
#   define asserte_d   __asserte_d
#   define assertef_d  __assertef_d
#endif /* __USE_KXS */
#ifdef	__USE_GNU
#   define assert_perror __assert_perror
#endif
__SYSDECL_END

#endif /* __CC__ */
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
#   define __assert_perror(e_num) __yes_assert_perror(e_num)
#else
#   define assert(expr)           __no_assert(#expr,expr)
#   define __assertf(expr,...)    __no_assertf(#expr,expr,__VA_ARGS__)
#   define __assert_d(expr,...)   __no_assert_d(#expr,expr,__VA_ARGS__)
#   define __assertf_d(expr,...)  __no_assertf_d(#expr,expr,__VA_ARGS__)
#   define __asserte(expr)        __no_asserte(#expr,expr)
#   define __assertef(expr,...)   __no_assertef(#expr,expr,__VA_ARGS__)
#   define __asserte_d(expr,...)  __no_asserte_d(#expr,expr,__VA_ARGS__)
#   define __assertef_d(expr,...) __no_assertef_d(#expr,expr,__VA_ARGS__)
#   define __assert_perror(e_num) __no_assert_perror(e_num)
#endif

#endif /* Changed... */
