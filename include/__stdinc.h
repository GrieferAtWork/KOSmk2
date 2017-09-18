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

#ifdef __cplusplus
#   define __NOTHROW(prot) prot throw()
#elif defined(__NO_ATTR_NOTHROW)
#   define __NOTHROW(prot) prot
#elif defined(__NO_ATTR_NOTHROW_SUFFIX)
#   define __NOTHROW(prot) __ATTR_NOTHROW prot
#else
#   define __NOTHROW(prot) prot __ATTR_NOTHROW
#endif

#ifdef __cplusplus
#   define __NAMESPACE_STD_EXISTS     1
#   define __NAMESPACE_STD_BEGIN      namespace std {
#   define __NAMESPACE_STD_END        }
#   define __NAMESPACE_STD_USING_(x)
#   define __NAMESPACE_STD_USING_1(x)
#   define __NAMESPACE_STD_USING___CXX_SYSTEM_HEADER(x) using std::x;
#   define __NAMESPACE_STD_USINGX2(d) __NAMESPACE_STD_USING_##d
#   define __NAMESPACE_STD_USINGX1(d) __NAMESPACE_STD_USINGX2(d)
#   define __NAMESPACE_STD_USING(x)   __NAMESPACE_STD_USINGX1(__CXX_SYSTEM_HEADER)(x)
#else
#   define __NAMESPACE_STD_BEGIN    /* nothing */
#   define __NAMESPACE_STD_END      /* nothing */
#   define __NAMESPACE_STD_USING(x) /* nothing */
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
#   define __DEFINE_PRIVATE_ALIAS(new,old) __asm__(".local " __DEFINE_ALIAS_STR(new) "\n" __DEFINE_ALIAS_STR(new) " = " __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_PUBLIC_ALIAS(new,old)  __asm__(".global " __DEFINE_ALIAS_STR(new) "\n" __DEFINE_ALIAS_STR(new) " = " __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_INTERN_ALIAS(new,old)  __asm__(".global " __DEFINE_ALIAS_STR(new) "\n.hidden " __DEFINE_ALIAS_STR(new) "\n" __DEFINE_ALIAS_STR(new) " = " __DEFINE_ALIAS_STR(old) "\n")
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


#if defined(__cplusplus) || defined(__DEEMON__)
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

#ifdef __cplusplus
extern "C++" {
class __any_ptr {
 void *__p;
public:
 constexpr inline __any_ptr(void *__p) throw(): __p(__p) {}
 template<class T> constexpr inline operator T *(void) throw() { return (T *)this->__p; }
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
#elif defined(__KOS__)
#   define __FILE     struct _IO_FILE
/* NOTE: '__DSYM()' generate an assembly name for a symbol that
 *        the kernel should prefer to link against when the module
 *        being patched was linked using PE, rather than ELF.
 *     >> Used to provide special symbols for use by windows
 *        emulation to work around stuff like 'wchar_t' being
 *        16 bits on kos-pe, but 32 bits on kos-elf. */
#   define __DSYM(x) .dos.x
#endif

#ifndef __LIBCCALL
#ifdef __KERNEL__
#   define __LIBCCALL __KCALL
#else
#   define __LIBCCALL /* Nothing */
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
