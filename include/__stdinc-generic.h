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
#define __GCC_VERSION(a,b,c) 0

#if __has_builtin(__builtin_expect)
#   define __expect(x,y)  __builtin_expect((x),(y))
#   define __likely(x)   (__builtin_expect(!!(x),1))
#   define __unlikely(x) (__builtin_expect(!!(x),0))
#else
#   define __expect(x,y) (x)
#   define __NO_expect    1
#   define __likely      /* Nothing */
#   define __unlikely    /* Nothing */
#endif

#if (defined(__BORLANDC__) && __BORLANDC__ >= 0x561 && !defined(__NO_LONG_LONG)) || \
     defined(__TINYC__) || defined(__DCC_VERSION__)
#   define __COMPILER_HAVE_LONGLONG 1
#endif
#define __COMPILER_HAVE_LONGDOUBLE 1
#define __COMPILER_HAVE_TRANSPARENT_STRUCT 1
#define __COMPILER_HAVE_TRANSPARENT_UNION 1
#if __has_feature(__tpp_pragma_push_macro__) || \
   (defined(__TPP_VERSION__) && __TPP_VERSION__ == 103)
#define __COMPILER_HAVE_PRAGMA_PUSHMACRO 1
#endif

#if defined(__DCC_VERSION__)
#   define __COMPILER_HAVE_AUTOTYPE 1
#elif __has_feature(cxx_auto_type)
#   define __auto_type              auto
#   define __COMPILER_HAVE_AUTOTYPE 1
#endif

#if defined(__BORLANDC__) && __BORLANDC__ >= 0x599
#pragma defineonoption __CODEGEAR_0X_SUPPORT__ -Ax
#endif

#if __has_feature(cxx_static_assert) || \
   (defined(__IBMCPP_STATIC_ASSERT) && __IBMCPP_STATIC_ASSERT) || \
   (defined(__BORLANDC__) && defined(__CODEGEAR_0X_SUPPORT__) && __BORLANDC__ >= 0x610)
#   define __STATIC_ASSERT(expr) static_assert(expr,#expr)
#elif __has_feature(c_static_assert)
#   define __STATIC_ASSERT(expr) _Static_assert(expr,#expr)
#elif defined(__TPP_COUNTER)
#   define __STATIC_ASSERT(expr) typedef int __PP_CAT2(__static_assert_,__TPP_COUNTER(__static_assert))[(expr)?1:-1]
#elif defined(__COUNTER__)
#   define __STATIC_ASSERT(expr) typedef int __PP_CAT2(__static_assert_,__COUNTER__)[(expr)?1:-1]
#else
#   define __STATIC_ASSERT(expr) typedef int __PP_CAT2(__static_assert_,__LINE__)[(expr)?1:-1]
#endif

#if defined(__DCC_VERSION__)
#   define __ASMNAME(x)   __asm__(x)
#else
#   define __NO_ASMNAME   1
#   define __ASMNAME(x)   /* Nothing */
#endif
#ifndef __DCC_VERSION__
#   define __extension__
#endif
#if __has_builtin(__builtin_va_list)
#   define __VA_LIST      __builtin_va_list
#endif
#if !defined(__DCC_VERSION__) && !defined(__FUNCTION__)
#if defined(__TINYC__) || 1
#   define __FUNCTION__ __func__
#elif __has_builtin(__builtin_FUNCTION)
#   define __FUNCTION__ __builtin_FUNCTION()
#else
#   define __NO_FUNCTION__ 1
#   define __FUNCTION__ (char *)0
#endif
#endif
#if defined(__DCC_VERSION__) || defined(__TINYC__)
#define __COMPILER_HAVE_TYPEOF   1
#endif
#if __has_attribute(__noinline__)
#   define __ATTR_NOINLINE       __attribute__((__noinline__))
#elif __has_declspec_attribute(noinline)
#   define __ATTR_NOINLINE       __declspec(noinline)
#else
#   define __NO_ATTR_NOINLINE    1
#   define __ATTR_NOINLINE       /* Nothing */
#endif
#if __has_attribute(__noreturn__)
#   define __ATTR_NORETURN       __attribute__((__noreturn__))
#elif __has_declspec_attribute(noreturn)
#   define __ATTR_NORETURN       __declspec(noreturn)
#else
#   define __NO_ATTR_NORETURN    1
#   define __ATTR_NORETURN       /* Nothing */
#endif
#if __has_attribute(__fastcall__)
#   define __ATTR_FASTCALL       __attribute__((__fastcall__))
#else
#   define __NO_ATTR_FASTCALL    1
#   define __ATTR_FASTCALL       /* Nothing */
#endif
#if __has_attribute(__stdcall__)
#   define __ATTR_STDCALL        __attribute__((__stdcall__))
#else
#   define __NO_ATTR_STDCALL     1
#   define __ATTR_STDCALL        /* Nothing */
#endif
#if __has_attribute(__cdecl__)
#   define __ATTR_CDECL          __attribute__((__cdecl__))
#else
#   define __NO_ATTR_CDECL       1
#   define __ATTR_CDECL          /* Nothing */
#endif
#if __has_attribute(__pure__)
#   define __ATTR_PURE           __attribute__((__pure__))
#elif __has_declspec_attribute(noalias)
#   define __ATTR_PURE           __declspec(noalias)
#else
#   define __NO_ATTR_PURE        1
#   define __ATTR_PURE           /* Nothing */
#endif
#if __has_attribute(__const__)
#   define __ATTR_CONST          __attribute__((__const__))
#elif __has_declspec_attribute(noalias)
#   define __ATTR_CONST          __declspec(noalias)
#else
#   define __NO_ATTR_CONST       1
#   define __ATTR_CONST          /* Nothing */
#endif
#if __has_attribute(__malloc__)
#   define __ATTR_MALLOC         __attribute__((__malloc__))
#else
#   define __NO_ATTR_MALLOC      1
#   define __ATTR_MALLOC         /* Nothing */
#endif
#if __has_attribute(__hot__)
#   define __ATTR_HOT         __attribute__((__hot__))
#else
#   define __NO_ATTR_HOT         1
#   define __ATTR_HOT            /* Nothing */
#endif
#if __has_attribute(__cold__)
#   define __ATTR_COLD           __attribute__((__cold__))
#else
#   define __NO_ATTR_COLD        1
#   define __ATTR_COLD           /* Nothing */
#endif
#if __has_attribute(__alloc_size__)
#   define __ATTR_ALLOC_SIZE(ppars) __attribute__((__alloc_size__ ppars))
#else
#   define __NO_ATTR_ALLOC_SIZE  1
#   define __ATTR_ALLOC_SIZE(ppars) /* Nothing */
#endif
#if __has_attribute(__alloc_align__)
#   define __ATTR_ALLOC_ALIGN(pari) __attribute__((__alloc_align__(pari)))
#else
#   define __NO_ATTR_ALLOC_ALIGN 1
#   define __ATTR_ALLOC_ALIGN(pari) /* Nothing */
#endif
#if __has_attribute(__assume_aligned__)
#   define __ATTR_ASSUME_ALIGNED(n) __attribute__((__assume_aligned__(n)))
#else
#   define __NO_ATTR_ASSUME_ALIGNED 1
#   define __ATTR_ASSUME_ALIGNED(n) /* Nothing */
#endif
#if __has_attribute(__noclone__)
#   define __ATTR_NOCLONE        __attribute__((__noclone__))
#else
#   define __NO_ATTR_NOCLONE     1
#   define __ATTR_NOCLONE        /* Nothing */
#endif
#if __has_attribute(__used__)
#   define __ATTR_USED           __attribute__((__used__))
#else
#   define __NO_ATTR_USED        1
#   define __ATTR_USED           /* Nothing */
#endif
#if __has_attribute(__unused__)
#   define __ATTR_UNUSED         __attribute__((__unused__))
#else
#   define __NO_ATTR_UNUSED      1
#   define __ATTR_UNUSED         /* Nothing */
#endif
#if __has_attribute(__sentinel__)
#   define __ATTR_SENTINEL       __attribute__((__sentinel__))
#else
#   define __NO_ATTR_SENTINEL    1
#   define __ATTR_SENTINEL       /* Nothing */
#endif
#if __has_feature(cxx_thread_local) || (defined(__cplusplus) && \
   (defined(__SUNPRO_CC) || defined(__IBMC__) || defined(__IBMCPP__)))
#   define __ATTR_THREAD         thread_local
#elif __has_feature(c_thread_local) || \
     (!defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ > 201000L)
#   define __ATTR_THREAD         _Thread_local
#elif __has_declspec_attribute(thread) || defined(__BORLANDC__) || defined(__DMC__)
#   define __ATTR_THREAD         __declspec(thread)
#elif (defined(__INTEL_COMPILER) || defined(__ICC) || defined(__ICL) || defined(__ECC))
#if defined(_WIN32) || defined(WIN32)
#   define __ATTR_THREAD         __thread
#else
#   define __ATTR_THREAD         __declspec(thread)
#endif
#else
#   define __NO_ATTR_THREAD      1
#   define __ATTR_THREAD         /* Nothing */
#endif
#if __has_attribute(__deprecated__)
#   define __ATTR_DEPRECATED_      __attribute__((__deprecated__))
#   define __ATTR_DEPRECATED(text) __attribute__((__deprecated__(text)))
#elif __has_declspec_attribute(deprecated)
#   define __ATTR_DEPRECATED_      __declspec(deprecated)
#   define __ATTR_DEPRECATED(text) __declspec(deprecated(text))
#elif __has_cpp_attribute(deprecated) >= 201309
#   define __ATTR_DEPRECATED_      [[deprecated]]
#   define __ATTR_DEPRECATED(text) [[deprecated(text)]]
#else
#   define __NO_ATTR_DEPRECATED    1
#   define __ATTR_DEPRECATED(text) /* Nothing */
#endif
#if __has_attribute(__warning__)
#   define __ATTR_WARNING(text)  __attribute__((__warning__(text)))
#else
#   define __NO_ATTR_WARNING     1
#   define __ATTR_WARNING(text)  /* Nothing */
#endif
#if __has_attribute(__error__)
#   define __ATTR_ERROR(text)    __attribute__((__error__(text)))
#else
#   define __NO_ATTR_ERROR       1
#   define __ATTR_ERROR(text)    /* Nothing */
#endif
#if __has_attribute(__section__)
#   define __ATTR_SECTION(name)  __attribute__((__section__(name)))
#else
#   define __NO_ATTR_TSECTION    1
#   define __ATTR_SECTION(name)  /* Nothing */
#endif
#if __has_attribute(__nothrow__)
#   define __ATTR_NOTHROW        __attribute__((__nothrow__))
#else
#   define __NO_ATTR_NOTHROW     1
#   define __ATTR_NOTHROW        /* Nothing */
#endif
#if __has_attribute(__returns_nonnull__)
#   define __ATTR_RETNONNULL     __attribute__((__returns_nonnull__))
#else
#   define __NO_ATTR_RETNONNULL  1
#   define __ATTR_RETNONNULL     /* Nothing */
#endif
#if __has_attribute(__packed__)
#   define __ATTR_PACKED         __attribute__((__packed__))
#else
#   define __NO_ATTR_PACKED      1
#   define __ATTR_PACKED         /* Nothing */
#endif
#if __has_attribute(__alias__)
#   define __ATTR_ALIAS(name)    __attribute__((__alias__(name)))
#else
#   define __NO_ATTR_ALIAS       1
#   define __ATTR_ALIAS(name)    /* Nothing */
#endif
#if __has_attribute(__aligned__)
#   define __ATTR_ALIGNED(n)     __attribute__((__aligned__(n)))
#elif __has_declspec_attribute(align)
#   define __ATTR_ALIGNED(n)     __declspec(align(n))
#elif __has_feature(cxx_alignas) || __has_extension(cxx_alignas)
#   define __ATTR_ALIGNED(n)     alignas(n)
#else
#   define __NO_ATTR_ALIGNED     1
#   define __ATTR_ALIGNED(n)     /* Nothing */
#endif
#if __has_attribute(__weak__) || \
   (defined(__ELF__) || defined(__TINYC__))
#   define __ATTR_WEAK           __attribute__((__weak__))
#else
#   define __NO_ATTR_WEAK        1
#   define __ATTR_WEAK           /* Nothing */
#endif
#if __has_attribute(__visibility__) || \
   (defined(__ELF__) || defined(__TINYC__))
#   define __ATTR_VISIBILITY(vis) __attribute__((__visibility__(vis)))
#else
#   define __NO_ATTR_VISIBILITY  1
#   define __ATTR_VISIBILITY(vis) /* Nothing */
#endif
#if __has_attribute(__dllimport__)
#   define __ATTR_DLLIMPORT      __attribute__((__dllimport__))
#   define __ATTR_DLLEXPORT      __attribute__((__dllexport__))
#elif __has_declspec_attribute(dllimport)
#   define __ATTR_DLLIMPORT      __declspec(dllimport)
#   define __ATTR_DLLEXPORT      __declspec(dllexport)
#elif defined(__PE__) || defined(_WIN32)
#   define __ATTR_DLLIMPORT      __declspec(dllimport)
#   define __ATTR_DLLEXPORT      __declspec(dllexport)
#else
#   define __NO_ATTR_DLLIMPORT   1
#   define __ATTR_DLLIMPORT      /* Nothing */
#   define __NO_ATTR_DLLEXPORT   1
#   define __ATTR_DLLEXPORT      /* Nothing */
#endif
#if __has_attribute(__nonnull__)
#   define __NONNULL(ppars)      __attribute__((__nonnull__ ppars))
#else
#   define __NO_NONNULL          1
#   define __NONNULL(ppars)      /* Nothing */
#endif
#if __has_attribute(__warn_unused_result__)
#   define __WUNUSED             __attribute__((__warn_unused_result__))
#else
#   define __NO_WUNUSED          1
#   define __WUNUSED             /* Nothing */
#endif
#if defined(__DCC_VERSION__) || defined(__TINYC__)
#   define __XBLOCK              __extension__
#   define __XRETURN             /* Nothing */
#else
#   define __NO_XBLOCK           1
#   define __XBLOCK(...)         do __VA_ARGS__ while(0)
#   define __XRETURN             /* Nothing */
#endif
#if !__has_builtin(__builtin_assume)
#   define __NO_builtin_assume   1
#   define __builtin_assume(x)  (void)0
#endif
#if !__has_builtin(__builtin_unreachable) && !defined(__TINYC__)
#   define __NO_builtin_unreachable 1
#   define __builtin_unreachable() do;while(1)
#endif
#if !__has_builtin(__builtin_constant_p)
#   define __NO_builtin_constant_p 1
#   define __builtin_constant_p(x) 0
#endif
#if __has_feature(cxx_alignof) || (defined(__cplusplus) && \
   (defined(__CODEGEARC__)  || (defined(__BORLANDC__) && \
    defined(__CODEGEAR_0X_SUPPORT__) && __BORLANDC__ >= 0x610)))
#   define __COMPILER_ALIGNOF alignof
#elif (defined(__ghs__) && (__GHS_VERSION_NUMBER >= 600)) || \
      (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5130) || \
       defined(__DCC_VERSION__) || defined(__TINYC__)
#   define __COMPILER_ALIGNOF __alignof__
#elif defined(__cplusplus)
template<class T> struct __compiler_alignof { char __x; T __y; };
#   define __COMPILER_ALIGNOF(T) (sizeof(__compiler_alignof< T >)-sizeof(T))
#else
#   define __COMPILER_ALIGNOF(T) ((__SIZE_TYPE__)&((struct{ char __x; T __y; } *)0)->__y)
#endif
#if __has_builtin(__builtin_offsetof)
#   define __COMPILER_OFFSETOF       __builtin_offsetof
#else
#   define __COMPILER_OFFSETOF(s,m) ((__SIZE_TYPE__)&((s *)0)->m)
#endif
#if defined(__cplusplus)
#   define __LOCAL      static inline
#elif defined(__BORLANDC__) || defined(__DMC__) || \
      defined(__SC__) || defined(__WATCOMC__) || \
      defined(__LCC__) || defined(__DECC)
#   define __LOCAL      static __inline
#elif __has_attribute(__always_inline__) || \
      defined(__DCC_VERSION__) || defined(__TINYC__)
#   define __LOCAL      static __inline__
#else
#   define __LOCAL      static
#endif
#if __has_attribute(__always_inline__)
#   define __FORCELOCAL __LOCAL __attribute__((__always_inline__))
#else
#   define __FORCELOCAL __LOCAL
#endif
#   define __LONGLONG   long long
#   define __ULONGLONG  unsigned long long
#if !__has_builtin(__builtin_prefetch)
#   define __NO_builtin_prefetch    1
#   define __builtin_prefetch(...) (void)0
#endif
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#   define __restrict_arr restrict
#else
#   define __restrict_arr /* Not supported.  */
#endif

#define __COMPILER_BARRIER()       (void)0 /* ??? */
#define __COMPILER_READ_BARRIER()  (void)0 /* ??? */
#define __COMPILER_WRITE_BARRIER() (void)0 /* ??? */

#ifdef __cplusplus
#ifdef __INTELLISENSE__
#   define __NULLPTR    nullptr
#else
#   define __NULLPTR          0
#endif
#else
#   define __NULLPTR ((void *)0)
#endif


/* Define varargs macros expected by system headers. */
#if __has_builtin(__builtin_va_list) || \
    __has_builtin(__builtin_va_start)
#define __VA_LIST                  __builtin_va_list
#else
/* Just guess some generic implementation... */
#define __VA_LIST                  char *
#define __VA_ADDROF(v)            &(v)
#define __VA_SIZEOF(n)            ((sizeof(n)+3)&~3)
#define __builtin_va_start(ap,v)  (ap = (va_list)__VA_ADDROF(v)+__VA_SIZEOF(v))
#define __builtin_va_arg(ap,T)    (*(T *)((ap += __VA_SIZEOF(T))-__VA_SIZEOF(T)))
#define __builtin_va_end(ap)      (void)0
#endif


