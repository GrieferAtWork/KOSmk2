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

#ifndef __GNUC_MINOR__
#   define __GNUC_MINOR__ 0
#endif
#ifndef __GNUC_PATCH__
#ifdef __GNUC_PATCHLEVEL__
#   define __GNUC_PATCH__ __GNUC_PATCHLEVEL__
#else
#   define __GNUC_PATCH__ 0
#endif
#endif
#define __GCC_VERSION_NUM    (__GNUC__*10000+__GNUC_MINOR__*100+__GNUC_PATCH__)
#define __GCC_VERSION(a,b,c) (__GCC_VERSION_NUM >= ((a)*10000+(b)*100+(c)))

#if __has_builtin(__builtin_expect) || !defined(__clang__)
#   define __expect(x,y)  __builtin_expect((x),(y))
#   define __likely(x)   (__builtin_expect(!!(x),1))
#   define __unlikely(x) (__builtin_expect(!!(x),0))
#else
#   define __expect(x,y) (x)
#   define __NO_expect    1
#   define __likely      /* Nothing */
#   define __unlikely    /* Nothing */
#endif

#if defined(__clang__) || !defined(__DARWIN_NO_LONG_LONG)
#define __COMPILER_HAVE_LONGLONG 1
#endif
#define __COMPILER_HAVE_LONGDOUBLE 1
#define __COMPILER_HAVE_TRANSPARENT_STRUCT 1
#define __COMPILER_HAVE_TRANSPARENT_UNION 1
#define __COMPILER_HAVE_PRAGMA_PUSHMACRO 1
#if 1
/* XXX: When was this added in C? */
#   define __COMPILER_HAVE_AUTOTYPE 1
#elif __has_feature(cxx_auto_type) || \
     (defined(__cplusplus) && __GCC_VERSION(4,4,0))
#     define __auto_type              auto
#     define __COMPILER_HAVE_AUTOTYPE 1
#endif

#if __has_feature(cxx_static_assert) || \
   (__GCC_VERSION(4,3,0) && (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L))
#   define __STATIC_ASSERT(expr) static_assert(expr,#expr)
#elif __has_feature(c_static_assert)
#   define __STATIC_ASSERT(expr) _Static_assert(expr,#expr)
#elif defined(__TPP_COUNTER)
#   define __STATIC_ASSERT(expr) extern __attribute__((__unused__)) int __PP_CAT2(__static_assert_,__TPP_COUNTER(__static_assert))[(expr)?1:-1]
#elif defined(__COUNTER__)
#   define __STATIC_ASSERT(expr) extern __attribute__((__unused__)) int __PP_CAT2(__static_assert_,__COUNTER__)[(expr)?1:-1]
#else
#   define __STATIC_ASSERT(expr) extern __attribute__((__unused__)) int __PP_CAT2(__static_assert_,__LINE__)[(expr)?1:-1]
#endif
#ifdef __INTELLISENSE__
#define __ASMNAME(x)   /* Nothing */
#else
#define __ASMNAME(x)   __asm__(x)
#endif
//#define __NO_ASMNAME 1 /* TODO: Remove me */
#if !__GCC_VERSION(2,8,0)
#define __extension__
#endif
#if !__GCC_VERSION(2,92,0)
#define __restrict     /* Nothing */
#define __restrict__   /* Nothing */
#endif
#define __VA_LIST      __builtin_va_list
#define __COMPILER_HAVE_TYPEOF 1
#if __GCC_VERSION(3,1,0)
#   define __ATTR_NOINLINE         __attribute__((__noinline__))
#else
#   define __NO_ATTR_NOINLINE      1
#   define __ATTR_NOINLINE         /* Nothing */
#endif
#define __ATTR_NORETURN            __attribute__((__noreturn__))
#define __ATTR_FASTCALL            __attribute__((__fastcall__))
#define __ATTR_STDCALL             __attribute__((__stdcall__))
#define __ATTR_CDECL               __attribute__((__cdecl__))
#if __GCC_VERSION(2,96,0)
#   define __ATTR_PURE             __attribute__((__pure__))
#else
#   define __NO_ATTR_PURE          1
#   define __ATTR_PURE             /* Nothing */
#endif
#if __GCC_VERSION(2,5,0) /* __GCC_VERSION(2,95,0) */
#   define __ATTR_CONST            __attribute__ ((__const__))
#else
#   define __NO_ATTR_CONST         1
#   define __ATTR_CONST            /* Nothing */
#endif
#if __GCC_VERSION(2,96,0)
#   define __ATTR_MALLOC           __attribute__((__malloc__))
#else
#   define __NO_ATTR_MALLOC        1
#   define __ATTR_MALLOC           /* Nothing */
#endif
#if __GCC_VERSION(4,3,0)
#   define __ATTR_ALLOC_SIZE(ppars) __attribute__((__alloc_size__ ppars))
#else
#   define __NO_ATTR_ALLOC_SIZE     1
#   define __ATTR_ALLOC_SIZE(ppars) /* Nothing */
#endif
#if __GCC_VERSION(2,7,0) /* __GCC_VERSION(3,1,0)? __GCC_VERSION(3,3,0)? */
#   define __ATTR_USED             __attribute__((__used__))
#   define __ATTR_UNUSED           __attribute__((__unused__))
#else
#   define __NO_ATTR_USED          1
#   define __ATTR_USED             /* Nothing */
#   define __NO_ATTR_UNUSED        1
#   define __ATTR_UNUSED           /* Nothing */
#endif
#ifdef __INTELLISENSE__
#   define __ATTR_DEPRECATED_      __declspec(deprecated)
#   define __ATTR_DEPRECATED(text) __declspec(deprecated(text))
#elif __GCC_VERSION(3,2,0) /* __GCC_VERSION(3,5,0) */
#   define __ATTR_DEPRECATED_      __attribute__((__deprecated__))
#if __GCC_VERSION(4,5,0)
#   define __ATTR_DEPRECATED(text) __attribute__((__deprecated__(text)))
#else
#   define __ATTR_DEPRECATED(text) __attribute__((__deprecated__))
#endif
#else
#   define __NO_ATTR_DEPRECATED    1
#   define __ATTR_DEPRECATED_      /* Nothing */
#   define __ATTR_DEPRECATED(text) /* Nothing */
#endif
#if __GCC_VERSION(3,5,0)
#   define __ATTR_SENTINEL         __attribute__((__sentinel__))
#else
#   define __NO_ATTR_SENTINEL      1
#   define __ATTR_SENTINEL         /* Nothing */
#endif
#if __GCC_VERSION(4,3,0)
#   define __ATTR_HOT              __attribute__((__hot__))
#   define __ATTR_COLD             __attribute__((__cold__))
#else
#   define __NO_ATTR_HOT           1
#   define __ATTR_HOT              /* Nothing */
#   define __NO_ATTR_COLD          1
#   define __ATTR_COLD             /* Nothing */
#endif
#if __GCC_VERSION(4,5,0)
#   define __ATTR_NOCLONE          __attribute__((__noclone__))
#else
#   define __NO_ATTR_NOCLONE       1
#   define __ATTR_NOCLONE          /* Nothing */
#endif
#if __GCC_VERSION(4,8,0)
#   define __ATTR_THREAD           __thread
#else
#   define __NO_ATTR_THREAD        1
#   define __ATTR_THREAD           /* Nothing */
#endif
#if __GCC_VERSION(4,9,0)
#   define __ATTR_ASSUME_ALIGNED(n) __attribute__((__assume_aligned__(n)))
#else
#   define __NO_ATTR_ASSUME_ALIGNED 1
#   define __ATTR_ASSUME_ALIGNED(n) /* Nothing */
#endif
#if __GCC_VERSION(5,4,0)
#   define __ATTR_ALLOC_ALIGN(pari) __attribute__((__alloc_align__(pari)))
#else
#   define __NO_ATTR_ALLOC_ALIGN   1
#   define __ATTR_ALLOC_ALIGN(pari) /* Nothing */
#endif
#define __ATTR_WARNING(text)     __attribute__((__warning__(text)))
#define __ATTR_ERROR(text)       __attribute__((__error__(text)))
#define __ATTR_SECTION(name)     __attribute__((__section__(name)))
#define __ATTR_NOTHROW           __attribute__((__nothrow__))
#define __ATTR_RETNONNULL        __attribute__((__returns_nonnull__))
#define __ATTR_PACKED            __attribute__((__packed__))
#define __ATTR_ALIAS(name)       __attribute__((__alias__(name)))
#define __ATTR_ALIGNED(n)        __attribute__((__aligned__(n)))
#define __ATTR_WEAK              __attribute__((__weak__))
#define __ATTR_VISIBILITY(vis)   __attribute__((__visibility__(vis)))
#if defined(__PE__) || defined(_WIN32)
#   define __ATTR_DLLIMPORT      __attribute__((__dllimport__))
#   define __ATTR_DLLEXPORT      __attribute__((__dllexport__))
#else
#   define __NO_ATTR_DLLIMPORT   1
#   define __ATTR_DLLIMPORT      /* Nothing */
#   define __NO_ATTR_DLLEXPORT   1
#   define __ATTR_DLLEXPORT      /* Nothing */
#endif
#define __NONNULL(ppars)         __attribute__((__nonnull__ ppars))
#if __GCC_VERSION(3,4,0)
#   define __WUNUSED             __attribute__((__warn_unused_result__))
#else
#   define __NO_WUNUSED          1
#   define __WUNUSED             /* Nothing */
#endif
#ifdef __INTELLISENSE__
#   define __XBLOCK(...)      (([&]__VA_ARGS__)())
#   define __XRETURN             return
#   define __builtin_assume(x)   __assume(x)
#else
#   define __XBLOCK              __extension__
#   define __XRETURN             /* Nothing */
#if !__has_builtin(__builtin_unreachable)
#   define __NO_builtin_assume   1
#   define __builtin_assume(x)  (void)0
#endif
#endif
#if __GCC_VERSION(4,3,0) && (!defined(__GCCXML__) && \
   !defined(__clang__) && !defined(unix) && \
   !defined(__unix__)) || defined(__LP64__)
#   define __COMPILER_ALIGNOF    __alignof__
#elif defined(__clang__)
#   define __COMPILER_ALIGNOF    __alignof
#elif defined(__cplusplus)
extern "C++" { template<class T> struct __compiler_alignof { char __x; T __y; }; }
#   define __COMPILER_ALIGNOF(T) (sizeof(__compiler_alignof< T >)-sizeof(T))
#else
#   define __COMPILER_ALIGNOF(T) ((__SIZE_TYPE__)&((struct{ char __x; T __y; } *)0)->__y)
#endif
#define __COMPILER_OFFSETOF            __builtin_offsetof
#define __FORCELOCAL static __inline__ __attribute__((__always_inline__))
#define __LOCAL      static __inline__
#ifdef __CC__
__extension__ typedef long long __longlong_t;
__extension__ typedef unsigned long long __ulonglong_t;
#define __LONGLONG   __longlong_t
#define __ULONGLONG  __ulonglong_t
#endif


#if __GCC_VERSION(3,1,0) && !defined(__GNUG__)
#   define __restrict_arr __restrict
#else
#   define __restrict_arr /* Nothing */
#endif

#if 1
#define __COMPILER_BARRIER()       __atomic_signal_fence(__ATOMIC_ACQ_REL)
#define __COMPILER_READ_BARRIER()  __atomic_signal_fence(__ATOMIC_ACQUIRE)
#define __COMPILER_WRITE_BARRIER() __atomic_signal_fence(__ATOMIC_RELEASE)
#elif 1
#define __COMPILER_BARRIER()       __XBLOCK({ __asm__ __volatile__("" : : : "memory"); (void)0; })
#define __COMPILER_READ_BARRIER()  __XBLOCK({ __asm__ __volatile__("" : : : "memory"); (void)0; })
#define __COMPILER_WRITE_BARRIER() __XBLOCK({ __asm__ __volatile__("" : : : "memory"); (void)0; })
#else
#define __COMPILER_BARRIER()       __sync_synchronize()
#define __COMPILER_READ_BARRIER()  __sync_synchronize()
#define __COMPILER_WRITE_BARRIER() __sync_synchronize()
#endif

#ifdef __cplusplus
#ifdef __INTELLISENSE__
#   define __NULLPTR    nullptr
#else
#   define __NULLPTR          0
#endif
#else
#   define __NULLPTR ((void *)0)
#endif

