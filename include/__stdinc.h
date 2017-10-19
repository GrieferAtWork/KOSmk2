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
#ifndef ___STDINC_H
#define ___STDINC_H 1

/* ... */

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
#ifndef __has_feature
#define __has_feature(x) 0
#endif
#ifndef __has_extension
#define __has_extension  __has_feature
#endif
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif
#ifndef __has_declspec_attribute
#define __has_declspec_attribute(x) 0
#endif
#ifndef __has_cpp_attribute
#define __has_cpp_attribute(x) 0
#endif
#ifndef __has_include
#define __has_include(x) 0
#endif
#ifndef __has_include_next
#define __has_include_next(x) 0
#endif

#ifdef __cplusplus
#   define __DECL_BEGIN extern "C" {
#   define __DECL_END   }
#else
#   define __DECL_BEGIN /* Nothing */
#   define __DECL_END   /* Nothing */
#endif

#if defined(__cplusplus) || defined(__INTELLISENSE__) || \
  (!defined(__LINKER__) && !defined(__ASSEMBLY__) && \
   !defined(__ASSEMBLER__) && !defined(__DEEMON__))
#define __CC__ 1 /* C Compiler. */
#endif


#define __PP_PRIVATE_STR(x) #x
#define __PP_STR(x) __PP_PRIVATE_STR(x)
#define __PP_PRIVATE_CAT2(a,b)   a##b
#define __PP_PRIVATE_CAT3(a,b,c) a##b##c
#define __PP_CAT2(a,b)   __PP_PRIVATE_CAT2(a,b)
#define __PP_CAT3(a,b,c) __PP_PRIVATE_CAT3(a,b,c)
#define __PP_PRIVATE_MUL8_0 0
#define __PP_PRIVATE_MUL8_1 8
#define __PP_PRIVATE_MUL8_2 16
#define __PP_PRIVATE_MUL8_4 32
#define __PP_PRIVATE_MUL8_8 64
#define __PP_PRIVATE_MUL8(x) __PP_PRIVATE_MUL8_##x
#define __PP_MUL8(x) __PP_PRIVATE_MUL8(x)

#define __COMPILER_LENOF(arr)          (sizeof(arr)/sizeof(*(arr)))
#define __COMPILER_ENDOF(arr)   ((arr)+(sizeof(arr)/sizeof(*(arr))))
#define __COMPILER_STRLEN(str)         (sizeof(str)/sizeof(char)-1)
#define __COMPILER_STREND(str)  ((str)+(sizeof(str)/sizeof(char)-1))


#ifdef __GNUC__
#   include "__stdinc-gcc.h"
#elif defined(_MSC_VER)
#   include "__stdinc-msvc.h"
#else
#   include "__stdinc-generic.h"
#endif

#ifndef __SYSDECL_BEGIN
#define __SYSDECL_BEGIN __DECL_BEGIN
#define __SYSDECL_END   __DECL_END
#endif /* !__SYSDECL_BEGIN */

#if defined(__cplusplus) && (__has_feature(cxx_constexpr) || \
   (defined(__cpp_constexpr) && __cpp_constexpr >= 200704) || \
   (defined(__IBMCPP__) && defined(__IBMCPP_CONSTEXPR) && (__IBMCPP_CONSTEXPR+0)) || \
   (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5130) || \
   (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GCC_VERSION(4,6,0)) || \
   (defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023026))
#   define __COMPILER_HAVE_CXX11_CONSTEXPR 1
#   define __CXX11_CONSTEXPR          constexpr
#   define __CXX11_CONSTEXPR_OR_CONST constexpr
#else
#   define __CXX11_CONSTEXPR          /* nothing */
#   define __CXX11_CONSTEXPR_OR_CONST const
#endif
#if defined(__cplusplus) && (\
   (defined(__clang__) && !(!__has_feature(cxx_generic_lambdas) || \
                           !(__has_feature(cxx_relaxed_constexpr) || \
                             __has_extension(cxx_relaxed_constexpr)))) || \
   (defined(__cpp_constexpr) && __cpp_constexpr >= 201304 && !defined(__clang__)))
#   define __COMPILER_HAVE_CXX14_CONSTEXPR 1
#   define __CXX14_CONSTEXPR          constexpr
#   define __CXX14_CONSTEXPR_OR_CONST constexpr
#else
#   define __CXX14_CONSTEXPR          /* nothing */
#   define __CXX14_CONSTEXPR_OR_CONST const
#endif
#if defined(__cplusplus) && (__has_feature(cxx_noexcept) || \
   (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GCC_VERSION(4,6,0)) || \
   (defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190021730))
#   define __COMPILER_HAVE_CXX11_NOEXCEPT 1
#   define __CXX_NOEXCEPT noexcept
#elif defined(__cplusplus)
#   define __CXX_NOEXCEPT throw()
#else
#   define __CXX_NOEXCEPT /* nothing */
#endif


#ifdef __INTELLISENSE__
#   define __NOTHROW       /* nothing */
#elif defined(__cplusplus)
#   define __NOTHROW(prot) prot __CXX_NOEXCEPT
#elif defined(__NO_ATTR_NOTHROW)
#   define __NOTHROW(prot) prot
#elif defined(__NO_ATTR_NOTHROW_SUFFIX)
#   define __NOTHROW(prot) __ATTR_NOTHROW prot
#else
#   define __NOTHROW(prot) __ATTR_NOTHROW prot
#endif

#if defined(__cplusplus) && !defined(__INTELLISENSE__)
#define __COMPILER_PREFERR_ENUMS 1
#endif

#ifdef __cplusplus
/* Mark the wchar_t type as already being defined when pre-defined by the compiler */
#if !defined(__wchar_t_defined) && \
    (defined(_NATIVE_WCHAR_T_DEFINED) || defined(__GNUC__))
#   define __wchar_t_defined 1
#endif
#   define __NAMESPACE_STD_EXISTS     1
#   define __NAMESPACE_STD_BEGIN      namespace std {
#   define __NAMESPACE_STD_END        }
#   define __NAMESPACE_STD_SYM        ::std::
#   define __NAMESPACE_STD_USING_(x)
#   define __NAMESPACE_STD_USING_1(x)
#   define __NAMESPACE_STD_USING___CXX_SYSTEM_HEADER(x) using std::x;
#   define __NAMESPACE_STD_USINGX2(d) __NAMESPACE_STD_USING_##d
#   define __NAMESPACE_STD_USINGX1(d) __NAMESPACE_STD_USINGX2(d)
#   define __NAMESPACE_STD_USING(x)   __NAMESPACE_STD_USINGX1(__CXX_SYSTEM_HEADER)(x)
#   define __NAMESPACE_INT_BEGIN      namespace __int {
#   define __NAMESPACE_INT_END        }
#   define __NAMESPACE_INT_SYM        ::__int::
#else
#   define __NAMESPACE_STD_BEGIN    /* nothing */
#   define __NAMESPACE_STD_END      /* nothing */
#   define __NAMESPACE_STD_SYM      /* nothing */
#   define __NAMESPACE_STD_USING(x) /* nothing */
#   define __NAMESPACE_INT_BEGIN    /* nothing */
#   define __NAMESPACE_INT_END      /* nothing */
#   define __NAMESPACE_INT_SYM      /* nothing */
#endif

#define __FCALL                  __ATTR_FASTCALL
#define __KCALL                  __ATTR_STDCALL

#if defined(__COMPILER_HAVE_AUTOTYPE) && !defined(__NO_XBLOCK)
#   define __COMPILER_UNUSED(expr) __XBLOCK({ __auto_type __expr = (expr); (void)__expr; })
#elif defined(__COMPILER_HAVE_TYPEOF) && !defined(__NO_XBLOCK)
#   define __COMPILER_UNUSED(expr) __XBLOCK({ __typeof__(expr) __expr = (expr); (void)__expr; })
#else
#   define __COMPILER_UNUSED(expr) (void)(expr)
#endif

#define __COMPILER_OFFSETAFTER(s,m) ((__SIZE_TYPE__)(&((s *)0)->m+1))
#define __COMPILER_CONTAINER_OF(ptr,type,member) \
  ((type *)((__UINTPTR_TYPE__)(ptr)-__COMPILER_OFFSETOF(type,member)))
#   define __DEFINE_ALIAS_STR(x) #x
#   define __DEFINE_PRIVATE_ALIAS(new,old) __asm__(".local " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_PUBLIC_ALIAS(new,old)  __asm__(".global " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_INTERN_ALIAS(new,old)  __asm__(".global " __DEFINE_ALIAS_STR(new) "\n.hidden " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#ifdef __NO_ATTR_ALIAS
#   define __ALIAS_IMPL(old,args) { return old args; }
#   define __ALIAS_FUNC(decl_new,new,decl_old,old,param,args) \
           decl_old old param; \
           decl_new new param { return old args; }
#   define __ALIAS_SYMBOL      __DEFINE_PUBLIC_ALIAS
#else
#   define __ALIAS_IMPL(new,args) __ATTR_ALIAS(#new)
#   define __ALIAS_FUNC(decl_new,new,decl_old,old,param,args) \
           decl_old old param; \
           decl_new new param __ATTR_ALIAS(#old)
#   define __ALIAS_SYMBOL(new,old) \
           __typeof__(old) new __ATTR_ALIAS(#old)
#endif


#define __IFDEF_ARG_PLACEHOLDER_     ,
#define __IFDEF_ARG_PLACEHOLDER_1    ,
#define __IFDEF_TAKE_SECOND_ARG_IMPL(x,val,...) val
#define __IFDEF_TAKE_SECOND_ARG(x) __IFDEF_TAKE_SECOND_ARG_IMPL x
#define __IFDEF3(x) __IFDEF_TAKE_SECOND_ARG((x 1,0))
#define __IFDEF2(x) __IFDEF3(__IFDEF_ARG_PLACEHOLDER_##x)
#define __IFTHEN_0(...) /* nothing */
#define __IFTHEN_1(...) __VA_ARGS__
#define __IFTHEN3(x) __IFTHEN_##x
#define __IFTHEN2(x) __IFTHEN3(x)
#define __IFTHEN(x)  __IFTHEN2(__IFDEF2(x))
#define __IFELSE_0(...) __VA_ARGS__
#define __IFELSE_1(...) /* nothing */
#define __IFELSE3(x) __IFELSE_##x
#define __IFELSE2(x) __IFELSE3(x)
#define __IFELSE(x)  __IFELSE2(__IFDEF2(x))

#if !defined(__PE__) && !defined(__ELF__)
/* Try to determine current binary format using other platform
 * identifiers. (When KOS headers are used on other systems) */
#if defined(__linux__) || defined(__linux) || defined(linux) || \
    defined(__unix__) || defined(__unix) || defined(unix)
#   define __ELF__ 1
#elif defined(__CYGWIN__) || defined(__MINGW32__) || defined(__WINDOWS__) || \
      defined(_WIN16) || defined(WIN16) || defined(_WIN32) || defined(WIN32) || \
      defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
      defined(_WIN32_WCE) || defined(WIN32_WCE)
#   define __PE__  1
#else
#   warning "Target binary format not defined. - Assuming '__ELF__'"
#   define __ELF__ 1
#endif
#endif


#ifndef __VA_LIST
#define __VA_LIST  char *
#endif
#ifndef __BOOL
#ifdef __cplusplus
#   define __BOOL bool
#elif 1
#   define __BOOL _Bool
#else
#   define __BOOL unsigned char
#endif
#endif

#if defined(__PE__) || defined(_WIN32)
#   define __IMPDEF  extern __ATTR_DLLIMPORT
#   define __EXPDEF  extern __ATTR_DLLEXPORT
#   define __PUBDEF  extern
#   define __PUBLIC  extern __ATTR_DLLEXPORT
#   define __PRIVATE static
#   define __INTDEF  extern
#ifdef _MSC_VER
#   define __INTERN  extern
#else
#   define __INTERN  /* Nothing */
#endif
#else
#   define __IMPDEF  extern __ATTR_VISIBILITY("default")
#   define __EXPDEF  extern __ATTR_VISIBILITY("default")
#   define __PUBDEF  extern __ATTR_VISIBILITY("default")
#   define __PUBLIC         __ATTR_VISIBILITY("default")
#   define __PRIVATE static
#   define __INTDEF  extern __ATTR_VISIBILITY("hidden")
#   define __INTERN         __ATTR_VISIBILITY("hidden")
#endif


#ifdef __INTELLISENSE__
#   define __UNUSED         /* Nothing */
#elif defined(__cplusplus) || defined(__DEEMON__)
#   define __UNUSED(name)   /* Nothing */
#elif !defined(__NO_ATTR_UNUSED)
#   define __UNUSED(name)   name __ATTR_UNUSED
#elif defined(__LCLINT__)
#   define __UNUSED(name)   /*@unused@*/ name
#elif defined(_MSC_VER)
#   define __UNUSED(name)   name
#   pragma warning(disable: 4100)
#else
#   define __UNUSED(name)   name
#endif

#define __IGNORE_REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)
#define __IGNORE_REDIRECT_VOID(decl,attr,Treturn,cc,name,param,asmname,args)
#define __NOREDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn (cc name) param;
#define __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn __NOTHROW((cc name) param);
#define __NOREDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
    decl attr void (cc name) param;
#define __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
    decl attr void __NOTHROW((cc name) param);

/* General purpose redirection implementation. */
#ifndef __REDIRECT
#ifdef __INTELLISENSE__
/* Only declare the functions for intellisense to minimize IDE lag. */
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn (cc name) param;
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn __NOTHROW((cc name) param);
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
    decl attr void (cc name) param;
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
    decl attr void __NOTHROW((cc name) param);
#elif !defined(__NO_ASMNAME)
/* Use GCC families assembly name extension. */
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn (cc name) param __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn __NOTHROW((cc name) param) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
    decl attr void (cc name) param __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
    decl attr void __NOTHROW((cc name) param) __ASMNAME(__PP_PRIVATE_STR(asmname));
#elif defined(__cplusplus)
/* In C++, we can use use namespaces to prevent collisions with incompatible prototypes. */
#define __REDIRECT_UNIQUE  __PP_CAT2(__u,__LINE__)
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
namespace __int { namespace __REDIRECT_UNIQUE { extern "C" { decl Treturn (cc asmname) param; } } } \
__LOCAL attr Treturn (cc name) param { \
    return (__int::__REDIRECT_UNIQUE:: asmname) args; \
}
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
namespace __int { namespace __REDIRECT_UNIQUE { extern "C" { decl Treturn __NOTHROW((cc asmname) param); } } } \
__LOCAL attr Treturn __NOTHROW((cc name) param) { \
    return (__int::__REDIRECT_UNIQUE:: asmname) args; \
}
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
namespace __int { namespace __REDIRECT_UNIQUE { extern "C" { decl void (cc asmname) param; } } } \
__LOCAL attr void (cc name) param { \
    (__int::__REDIRECT_UNIQUE:: asmname) args; \
}
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
namespace __int { namespace __REDIRECT_UNIQUE { extern "C" { decl void __NOTHROW((cc asmname) param); } } } \
__LOCAL attr void __NOTHROW((cc name) param) { \
    (__int::__REDIRECT_UNIQUE:: asmname) args; \
}
#else
/* Fallback: Assume that the compiler supports scoped declarations,
 *           as well as deleting them once the scope ends.
 * NOTE: GCC actually doesn't support this one, somehow keeping
 *       track of the C declaration types even after the scope ends,
 *       causing it to fail fatal()-style with incompatible-prototype errors.
 * HINT: Function implementation does how ever work for MSVC when compiling for C.
 */
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
__LOCAL attr Treturn (cc name) param { \
    decl Treturn (cc asmname) param; \
    return (asmname) args; \
}
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
__LOCAL attr Treturn __NOTHROW((cc name) param) { \
    decl Treturn __NOTHROW((cc asmname) param); \
    return (asmname) args; \
}
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
__LOCAL attr void (cc name) param { \
    decl void (cc asmname) param; \
    (asmname) args; \
}
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
__LOCAL attr void __NOTHROW((cc name) param) { \
    decl void __NOTHROW((cc asmname) param); \
    (asmname) args; \
}
#endif
#endif /* !__REDIRECT */


#ifdef __cplusplus
extern "C++" {
class __any_ptr {
 void *__p;
public:
 __CXX11_CONSTEXPR inline __any_ptr(void *__p) throw(): __p(__p) {}
 template<class T> __CXX11_CONSTEXPR inline operator T *(void) throw() { return (T *)this->__p; }
};
}
#   define __COMPILER_UNIPOINTER(p) __any_ptr((void *)(__UINTPTR_TYPE__)(p))
#elif defined(__CC__)
#   define __COMPILER_UNIPOINTER(p)          ((void *)(__UINTPTR_TYPE__)(p))
#else
#   define __COMPILER_UNIPOINTER(p)                                     (p)
#endif

#ifdef __KERNEL__
#   undef NDEBUG
#ifndef CONFIG_DEBUG
#   define NDEBUG 1
#endif
#else
#ifdef __NAMESPACE_STD_EXISTS
#ifdef __CC__
__NAMESPACE_STD_BEGIN
struct _IO_FILE;
__NAMESPACE_STD_END
#endif
#   define __FILE     struct ::std::_IO_FILE
#else
#ifdef __CC__
struct _IO_FILE;
#endif
#   define __FILE     struct _IO_FILE
#endif
#endif

#ifndef __LIBCCALL
#ifdef __KERNEL__
#   define __LIBCCALL __KCALL
#else
#   define __LIBCCALL /* Nothing */
#   define __LIBCCALL_CALLER_CLEANUP 1
#endif
#endif

#ifndef __LIBC
#define __LIBC    __IMPDEF
#endif

/* Annotations */
#define __CLEARED      /* Annotation for allocators returning zero-initialized memory. */
#define __WEAK         /* Annotation for weakly referenced data. */
#define __REF          /* Annotation for reference holders. */
#define __ATOMIC_DATA  /* Annotation for atomic data. */
#define __PAGE_ALIGNED /* Annotation for page-aligned pointers. */
#define __USER         /* Annotation for user-space memory (default outside kernel). */
#define __HOST         /* Annotation for kernel-space memory (default within kernel). */
#define __VIRT         /* Annotation for virtual memory (default). */
#define __PHYS         /* Annotation for physical memory. */
#define __CRIT         /* Annotation for functions that require 'TASK_ISCRIT()' (When called from within the kernel). */
#define __SAFE         /* Annotation for functions that require 'TASK_ISSAFE()' (When called from within the kernel). */
#define __NOMP         /* Annotation for functions that are not thread-safe and require caller-synchronization. */
#define __PERCPU       /* Annotation for variables that must be accessed using the per-cpu API. */
#define __ASMCALL      /* Annotation for functions that are implemented in assembly and require a custom calling convention. */
#define __INITCALL     /* Annotation for functions that apart of .free sections, meaning that calling them is illegal after some specific point in time. */

#endif /* !___STDINC_H */
