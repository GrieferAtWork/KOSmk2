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

#define __expect(x,y) (x)
#define __NO_expect    1
#define __likely      /* Nothing */
#define __unlikely    /* Nothing */

#if defined(_MSC_EXTENSIONS) || _MSC_VER >= 1400
#   define __COMPILER_HAVE_LONGLONG 1
#endif
#define __COMPILER_HAVE_LONGDOUBLE 1
#define __COMPILER_HAVE_TRANSPARENT_STRUCT 1
#define __COMPILER_HAVE_TRANSPARENT_UNION 1
#define __COMPILER_HAVE_PRAGMA_PUSHMACRO 1

#if __has_feature(cxx_auto_type) || \
   (defined(__cplusplus) && _MSC_VER >= 1600)
#   define __auto_type              auto
#   define __COMPILER_HAVE_AUTOTYPE 1
#endif

#if __has_feature(cxx_static_assert) || _MSC_VER >= 1600
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
#define __NO_ASMNAME             1
#define __ASMNAME(x)             /* Nothing */
#define __extension__            /* Nothing */
#define __ATTR_NOINLINE          __declspec(noinline)
#define __ATTR_NORETURN          __declspec(noreturn)
#define __ATTR_FASTCALL          __fastcall
#define __ATTR_STDCALL           __stdcall
#define __ATTR_CDECL             __cdecl
#define __ATTR_PURE              __declspec(noalias)
#define __ATTR_CONST             __declspec(noalias)
#define __NO_ATTR_MALLOC         1
#define __ATTR_MALLOC            /* Nothing */
#define __NO_ATTR_HOT            1
#define __ATTR_HOT               /* Nothing */
#define __NO_ATTR_COLD           1
#define __ATTR_COLD              /* Nothing */
#define __NO_ATTR_ALLOC_SIZE     1
#define __ATTR_ALLOC_SIZE(ppars) /* Nothing */
#define __NO_ATTR_ALLOC_ALIGN    1
#define __ATTR_ALLOC_ALIGN(pari) /* Nothing */
#define __NO_ATTR_ASSUME_ALIGNED 1
#define __ATTR_ASSUME_ALIGNED(n) /* Nothing */
#define __NO_ATTR_NOCLONE        1
#define __ATTR_NOCLONE           /* Nothing */
#define __NO_ATTR_USED           1
#define __ATTR_USED              /* Nothing */
#define __NO_ATTR_UNUSED         1
#define __ATTR_UNUSED            /* Nothing */
#define __NO_ATTR_SENTINEL       1
#define __ATTR_SENTINEL          /* Nothing */
#if _MSC_VER >= 1700
#   define __ATTR_THREAD         __declspec(thread)
#else
#   define __NO_ATTR_THREAD      1
#   define __ATTR_THREAD         /* Nothing */
#endif
#if _MSC_VER >= 1200
#   define __ATTR_DEPRECATED_    __declspec(deprecated)
#   define __ATTR_DEPRECATED(text) __declspec(deprecated(text))
#else
#   define __NO_ATTR_DEPRECATED  1
#   define __ATTR_DEPRECATED(text) /* Nothing */
#endif
#define __NO_ATTR_WARNING        1
#define __ATTR_ERROR(text)       /* Nothing */
#define __NO_ATTR_WARNING        1
#define __ATTR_ERROR(text)       /* Nothing */
#define __NO_ATTR_TSECTION       1
#define __ATTR_SECTION(name)     /* Nothing */
#ifdef __cplusplus
#define __ATTR_NOTHROW           __declspec(nothrow)
#else
#define __NO_ATTR_NOTHROW        1
#define __ATTR_NOTHROW           /* Nothing */
#endif
#define __NO_ATTR_NOTHROW_SUFFIX 1
#define __NO_ATTR_RETNONNULL     1
#define __ATTR_RETNONNULL        /* Nothing */
#define __NO_ATTR_PACKED         1
#define __ATTR_PACKED            /* Nothing */
#define __NO_ATTR_ALIAS          1
#define __ATTR_ALIAS(name)       /* Nothing */
#if _MSC_VER >= 1300
#   define __ATTR_ALIGNED(n)     __declspec(align(n))
#else
#   define __NO_ATTR_ALIGNED     1
#   define __ATTR_ALIGNED(n)     /* Nothing */
#endif
#define __ATTR_WEAK              __declspec(selectany) /* For all that we care, it's basically the same. */
#define __NO_ATTR_VISIBILITY     1
#define __ATTR_VISIBILITY(vis)   /* Nothing */
#define __ATTR_DLLIMPORT         __declspec(dllimport)
#define __ATTR_DLLEXPORT         __declspec(dllexport)
#define __NO_NONNULL             1
#define __NONNULL(ppars)         /* Nothing */
#define __NO_WUNUSED             1
#define __WUNUSED                /* Nothing */
#define __NO_XBLOCK              1
#define __XBLOCK(...)            do __VA_ARGS__ while(0)
#define __XRETURN                /* Nothing */
#define __builtin_assume(x)      __assume(x)
#define __builtin_unreachable()  __assume(0)
#define __COMPILER_ALIGNOF       __alignof
#define __COMPILER_OFFSETOF(s,m) ((__SIZE_TYPE__)&((s *)0)->m)
#define __FORCELOCAL             static __forceinline
#define __LOCAL                  static __inline
#define __LONGLONG               long long
#define __ULONGLONG              unsigned long long
#define __NO_builtin_constant_p  1
#define __builtin_constant_p(x)  0
#define __restrict_arr           __restrict

/* Define intrinsic barrier functions. */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#if defined(__i386__) || defined(__i386) || defined(i386) || \
    defined(__I86__) || defined(_M_IX86) || defined(__X86__) || \
    defined(_X86_) || defined(__THW_INTEL__) || defined(__INTEL__) || \
    defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || \
    defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64) || \
    defined(_WIN64) || defined(WIN64)
extern void (__cdecl _m_prefetch)(void *);
#define __builtin_prefetch(addr,...) ((_m_prefetch)(addr))
#pragma intrinsic(_m_prefetch)
#else
#define __NO_builtin_prefetch    1
#define __builtin_prefetch(...) (void)0
#endif
extern void (__cdecl _ReadBarrier)(void);
extern void (__cdecl _WriteBarrier)(void);
extern void (__cdecl _ReadWriteBarrier)(void);
#pragma intrinsic(_ReadBarrier)
#pragma intrinsic(_WriteBarrier)
#pragma intrinsic(_ReadWriteBarrier)
#define __COMPILER_BARRIER()       _ReadWriteBarrier()
#define __COMPILER_READ_BARRIER()  _ReadBarrier()
#define __COMPILER_WRITE_BARRIER() _WriteBarrier()
#ifdef __cplusplus
}
#endif /* __cplusplus */

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
#define __VA_LIST        char *
#define __VA_ADDROF(v)   &(v)
#if defined(__i386__) || defined(__i386) || defined(i386) || \
    defined(__I86__) || defined(_M_IX86) || defined(__X86__) || \
    defined(_X86_) || defined(__THW_INTEL__) || defined(__INTEL__)
#define __VA_SIZEOF(n)                 ((sizeof(n)+3)&~3)
#define __builtin_va_start(ap,last_arg) (void)(ap = (__VA_LIST)__VA_ADDROF(last_arg)+__VA_SIZEOF(last_arg))
#define __builtin_va_arg(ap,T)          (*(T *)((ap += __VA_SIZEOF(T))-__VA_SIZEOF(T)))
#define __builtin_va_end(ap)            (void)0
#elif defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || \
      defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64) || \
      defined(_WIN64) || defined(WIN64)
extern void (__cdecl __va_start)(__VA_LIST *, ...);
#define __builtin_va_start(ap,x) __va_start(&ap,x)
#define __builtin_va_arg(ap,T) \
    ((sizeof(T) > 8 || (sizeof(T)&(sizeof(T) - 1)) != 0) ? **(T **)((ap += 8)-8) : *(T *)((ap += 8)-8))
#define __builtin_va_end(ap)    (void)0
#else /* ... */
#define __VA_SIZEOF(n)            ((sizeof(n)+3)&~3)
#define __builtin_va_start(ap,v)  (ap = (va_list)__VA_ADDROF(v)+__VA_SIZEOF(v))
#define __builtin_va_arg(ap,T)    (*(T *)((ap += __VA_SIZEOF(T))-__VA_SIZEOF(T)))
#define __builtin_va_end(ap)      (void)0
#endif /* !... */

#pragma warning(disable: 4514) /* Unused inline function was removed. */
#pragma warning(disable: 4574) /* Nonsensical preprocessor warning. */
#pragma warning(disable: 4710) /* Function not inlined (Emit for local varargs functions...) */
#ifndef __cplusplus
/* Disable some warnings that are caused by function redirection in system headers. */
#define __REDIRECT_WSUPPRESS_BEGIN __pragma(warning(push)) \
                                   __pragma(warning(disable: 4210 4028 4142 4565))
#define __REDIRECT_WSUPPRESS_END   __pragma(warning(pop))
/* Suppress warnings caused by C-mode redirections in system headers. */
#define __SYSDECL_BEGIN __DECL_BEGIN __REDIRECT_WSUPPRESS_BEGIN
#define __SYSDECL_END   __REDIRECT_WSUPPRESS_END __DECL_END
#endif



