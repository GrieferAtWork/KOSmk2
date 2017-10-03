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
#ifndef _STDLIB_H
#define _STDLIB_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <__malldefs.h>

#ifdef __USE_MISC
#include <alloca.h>
#endif /* __USE_MISC */
#ifdef __USE_DOS
#include <xlocale.h>
#include <bits/byteswap.h>
#endif /* __USE_DOS */

#if defined(__USE_DEBUG) && __USE_DEBUG != 0
#include <hybrid/debuginfo.h>
#endif

__DECL_BEGIN

#ifdef __NAMESPACE_STD_EXISTS
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
#else /* __NAMESPACE_STD_EXISTS */
#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */
#endif /* !__NAMESPACE_STD_EXISTS */

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif

#ifndef NULL
#ifdef __INTELLISENSE__
#   define NULL nullptr
#elif defined(__cplusplus) || defined(__LINKER__)
#   define NULL          0
#else
#   define NULL ((void *)0)
#endif
#endif



#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#ifndef __WAIT_MACROS_DEFINED
#define __WAIT_MACROS_DEFINED 1
#include <bits/waitflags.h>
#include <bits/waitstatus.h>

#ifdef __USE_MISC
#if defined(__GNUC__) && !defined __cplusplus
#   define __WAIT_INT(status) (__extension__(((union{ __typeof__(status) __in; int __i; }) { .__in = (status) }).__i))
#else
#   define __WAIT_INT(status) (*(int *)&(status))
#endif
#if !defined(__GNUC__) || __GNUC__ < 2 || defined(__cplusplus)
#   define __WAIT_STATUS      void *
#   define __WAIT_STATUS_DEFN void *
#else
typedef union {
 union wait *__uptr;
 int        *__iptr;
} __WAIT_STATUS __attribute__((__transparent_union__));
#   define __WAIT_STATUS_DEFN int *
#endif
#else /* __USE_MISC */
#   define __WAIT_INT(status)  (status)
#   define __WAIT_STATUS        int *
#   define __WAIT_STATUS_DEFN   int *
#endif /* !__USE_MISC */
#   define WEXITSTATUS(status)  __WEXITSTATUS(__WAIT_INT(status))
#   define WTERMSIG(status)     __WTERMSIG(__WAIT_INT(status))
#   define WSTOPSIG(status)     __WSTOPSIG(__WAIT_INT(status))
#   define WIFEXITED(status)    __WIFEXITED(__WAIT_INT(status))
#   define WIFSIGNALED(status)  __WIFSIGNALED(__WAIT_INT(status))
#   define WIFSTOPPED(status)   __WIFSTOPPED(__WAIT_INT(status))
#ifdef __WIFCONTINUED
#   define WIFCONTINUED(status) __WIFCONTINUED(__WAIT_INT(status))
#endif
#endif /* !__WAIT_MACROS_DEFINED */
#endif /* __USE_XOPEN || __USE_XOPEN2K8 */

#ifdef __NAMESPACE_STD_EXISTS
__NAMESPACE_STD_BEGIN
#ifndef __std_div_t_defined
#define __std_div_t_defined 1
typedef struct { int quot,rem; } div_t;
#endif /* !__std_div_t_defined */
#ifndef __std_ldiv_t_defined
#define __std_ldiv_t_defined 1
typedef struct { long quot,rem; } ldiv_t;
#endif /* !__std_ldiv_t_defined */
#ifdef __USE_ISOC99
#ifndef __std_lldiv_t_defined
#define __std_lldiv_t_defined 1
typedef struct { __LONGLONG quot,rem; } lldiv_t;
#endif /* !__std_lldiv_t_defined */
#endif /* __USE_ISOC99 */
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
#ifndef __div_t_defined
#define __div_t_defined 1
__NAMESPACE_STD_USING(div_t)
#endif /* !__div_t_defined */
#ifndef __ldiv_t_defined
#define __ldiv_t_defined 1
__NAMESPACE_STD_USING(ldiv_t)
#endif /* !__ldiv_t_defined */
#ifdef __USE_ISOC99
#ifndef __lldiv_t_defined
#define __lldiv_t_defined 1
__NAMESPACE_STD_USING(lldiv_t)
#endif /* !__lldiv_t_defined */
#endif /* __USE_ISOC99 */
#endif /* !__CXX_SYSTEM_HEADER */
#else /* __NAMESPACE_STD_EXISTS */
#ifndef __div_t_defined
#define __div_t_defined 1
typedef struct { int quot,rem; } div_t;
#endif /* !__div_t_defined */
#ifndef __ldiv_t_defined
#define __ldiv_t_defined 1
typedef struct { long quot,rem; } ldiv_t;
#endif /* !__ldiv_t_defined */
#ifdef __USE_ISOC99
#ifndef __lldiv_t_defined
#define __lldiv_t_defined 1
typedef struct { __LONGLONG quot,rem; } lldiv_t;
#endif /* !__lldiv_t_defined */
#endif /* __USE_ISOC99 */
#endif /* !__NAMESPACE_STD_EXISTS */


#ifdef __KERNEL__
#define RAND_MAX 0xffffffffu
#else
#define RAND_MAX 0x7fffffff
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


#ifndef __COMPAR_FN_T
#define __COMPAR_FN_T
typedef int (__LIBCCALL *__compar_fn_t)(void const *__a, void const *__b);
#ifdef __USE_GNU
typedef __compar_fn_t comparison_fn_t;
#endif /* __USE_GNU */
#endif /* __COMPAR_FN_T */

#ifdef __USE_GNU
#ifndef __compar_d_fn_t_defined
#define __compar_d_fn_t_defined 1
typedef int (__LIBCCALL *__compar_d_fn_t)(void const *__a, void const *__b, void *__arg);
#endif /* !__compar_d_fn_t_defined */
__LIBC __NONNULL((1,4)) void (__LIBCCALL qsort_r)(void *__base, size_t __nmemb, size_t __size, __compar_d_fn_t __compar, void *__arg);
#endif /* __USE_GNU */

__NAMESPACE_STD_BEGIN
__LIBC __NONNULL((1,2,5)) __WUNUSED void *(__LIBCCALL bsearch)(void const *__key, void const *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar);
__LIBC __NONNULL((1,4)) void (__LIBCCALL qsort)(void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar);
#ifdef __KERNEL__
__LOCAL __ATTR_CONST __WUNUSED int (__LIBCCALL abs)(int __x) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED long (__LIBCCALL labs)(long __x) { return __x < 0 ? -__x : __x; }
#ifdef __USE_ISOC99
__LOCAL __ATTR_CONST __WUNUSED __LONGLONG (__LIBCCALL llabs)(__LONGLONG __x) { return __x < 0 ? -__x : __x; }
#endif /* __USE_ISOC99 */

#ifdef __GNUC__
__LOCAL __ATTR_CONST __WUNUSED div_t (__LIBCCALL div)(int __numer, int __denom) { return (div_t){ __numer/__denom,__numer%__denom }; }
__LOCAL __ATTR_CONST __WUNUSED ldiv_t (__LIBCCALL ldiv)(long __numer, long __denom) { return (ldiv_t){ __numer/__denom,__numer%__denom }; }
#ifdef __USE_ISOC99
__LOCAL __ATTR_CONST __WUNUSED lldiv_t (__LIBCCALL lldiv)(long long __numer, long long __denom) { return (lldiv_t){ __numer/__denom,__numer%__denom }; }
#endif /* __USE_ISOC99 */
#else
__LOCAL __ATTR_CONST __WUNUSED div_t (__LIBCCALL div)(int __numer, int __denom) { div_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
__LOCAL __ATTR_CONST __WUNUSED ldiv_t (__LIBCCALL ldiv)(long __numer, long __denom) { ldiv_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
#ifdef __USE_ISOC99
__LOCAL __ATTR_CONST __WUNUSED lldiv_t (__LIBCCALL lldiv)(__LONGLONG __numer, __LONGLONG __denom) { lldiv_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
#endif /* __USE_ISOC99 */
#endif
#else /* __KERNEL__ */
#if defined(__cplusplus) && (defined(__USE_KOS) || defined(__USE_DOS))
extern "C++" {
__LIBC __ATTR_CONST __WUNUSED int (__LIBCCALL abs)(int __x) __ASMNAME("abs");
__LIBC __ATTR_CONST __WUNUSED long (__LIBCCALL abs)(long __x) __ASMNAME("labs");
__LIBC __ATTR_CONST __WUNUSED __LONGLONG (__LIBCCALL abs)(__LONGLONG __x) __ASMNAME("llabs");
__LIBC __ATTR_CONST __WUNUSED div_t (__LIBCCALL div)(int __numer, int __denom) __ASMNAME("div");
__LIBC __ATTR_CONST __WUNUSED ldiv_t (__LIBCCALL div)(long __numer, long __denom) __ASMNAME("ldiv");
__LIBC __ATTR_CONST __WUNUSED lldiv_t (__LIBCCALL div)(__LONGLONG __numer, __LONGLONG __denom) __ASMNAME("lldiv");
}
#else
__LIBC __ATTR_CONST __WUNUSED int (__LIBCCALL abs)(int __x);
__LIBC __ATTR_CONST __WUNUSED div_t (__LIBCCALL div)(int __numer, int __denom);
#endif
__LIBC __ATTR_CONST __WUNUSED long (__LIBCCALL labs)(long __x);
__LIBC __ATTR_CONST __WUNUSED ldiv_t (__LIBCCALL ldiv)(long __numer, long __denom);
#ifdef __USE_ISOC99
__LIBC __ATTR_CONST __WUNUSED __LONGLONG (__LIBCCALL llabs)(__LONGLONG __x);
__LIBC __ATTR_CONST __WUNUSED lldiv_t (__LIBCCALL lldiv)(__LONGLONG __numer, __LONGLONG __denom);
#endif /* __USE_ISOC99 */
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(bsearch)
__NAMESPACE_STD_USING(qsort)
__NAMESPACE_STD_USING(abs)
__NAMESPACE_STD_USING(labs)
__NAMESPACE_STD_USING(div)
__NAMESPACE_STD_USING(ldiv)
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(llabs)
__NAMESPACE_STD_USING(lldiv)
#endif /* __USE_ISOC99 */

#if defined(__USE_MISC) || defined(__USE_DOS) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8))
__LIBC __WUNUSED __NONNULL((3)) char *(__LIBCCALL gcvt)(double __val, int __ndigit, char *__buf) __PE_ASMNAME("_gcvt");
#endif

#ifdef __USE_MISC
__LIBC __WUNUSED __NONNULL((3)) char *(__LIBCCALL qgcvt)(long double __val, int __ndigit, char *__buf);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL ecvt_r)(double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign, char *__restrict __buf, size_t __len);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL fcvt_r)(double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign, char *__restrict __buf, size_t __len);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL qecvt_r)(long double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign, char *__restrict __buf, size_t __len);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL qfcvt_r)(long double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign, char *__restrict __buf, size_t __len);
__LIBC __NONNULL((1)) __LONGLONG (__LIBCCALL strtoq)(char const *__restrict __nptr, char **__restrict __endptr, int __base) __ASMNAME("strtoll");
__LIBC __NONNULL((1)) __ULONGLONG (__LIBCCALL strtouq)(char const *__restrict __nptr, char **__restrict __endptr, int __base) __ASMNAME("strtoull");
#endif /* __USE_MISC */

__NAMESPACE_STD_BEGIN
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) double (__LIBCCALL atof)(char const *__restrict __nptr);
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) int (__LIBCCALL atoi)(char const *__restrict __nptr);
#if __SIZEOF_LONG__ == __SIZEOF_INT__
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) long (__LIBCCALL atol)(char const *__restrict __nptr) __ASMNAME("atoi");
#else
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) long (__LIBCCALL atol)(char const *__restrict __nptr);
#endif
__LIBC __NONNULL((1)) double (__LIBCCALL strtod)(char const *__restrict __nptr, char **__endptr);
__LIBC __NONNULL((1)) long (__LIBCCALL strtol)(char const *__restrict __nptr, char **__endptr, int __base);
__LIBC __NONNULL((1)) unsigned long (__LIBCCALL strtoul)(char const *__restrict __nptr, char **__restrict __endptr, int __base);
#ifdef __USE_ISOC99
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) __LONGLONG (__LIBCCALL atoll)(char const *__nptr)
#if __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__
    __ASMNAME("atol")
#endif
;
__LIBC __NONNULL((1)) float (__LIBCCALL strtof)(char const *__restrict __nptr, char **__restrict __endptr) __PE_ASMNAME("strtod");
__LIBC __NONNULL((1)) long double (__LIBCCALL strtold)(char const *__restrict __nptr, char **__restrict __endptr) __PE_ASMNAME("strtod");
__LIBC __NONNULL((1)) __LONGLONG (__LIBCCALL strtoll)(char const *__restrict __nptr, char **__restrict __endptr, int __base)
#if __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__
    __ASMNAME("strtol")
#endif
;
__LIBC __NONNULL((1)) __ULONGLONG (__LIBCCALL strtoull)(char const *__restrict __nptr, char **__restrict __endptr, int __base)
#if __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__
    __ASMNAME("strtoul")
#endif
;
#endif /* __USE_ISOC99 */
#ifdef __KERNEL__
__LIBC __UINT32_TYPE__ (__LIBCCALL rand)(void);
__LIBC void (__LIBCCALL srand)(__UINT32_TYPE__ __seed);
#else /* __KERNEL__ */
__LIBC int (__LIBCCALL rand)(void);
__LIBC void (__LIBCCALL srand)(long __seed);
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(atof)
__NAMESPACE_STD_USING(atoi)
__NAMESPACE_STD_USING(atol)
__NAMESPACE_STD_USING(strtod)
__NAMESPACE_STD_USING(strtol)
__NAMESPACE_STD_USING(strtoul)
__NAMESPACE_STD_USING(rand)
__NAMESPACE_STD_USING(srand)
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(atoll)
__NAMESPACE_STD_USING(strtof)
__NAMESPACE_STD_USING(strtold)
__NAMESPACE_STD_USING(strtoll)
__NAMESPACE_STD_USING(strtoull)
#endif /* __USE_ISOC99 */

#ifdef __USE_POSIX
#ifdef __KERNEL__
__LIBC __NONNULL((1)) __UINT32_TYPE__ (__LIBCCALL rand_r)(__UINT32_TYPE__ *__restrict __seed);
#else /* __KERNEL__ */
__LIBC __NONNULL((1)) int rand_r (unsigned int *__restrict __seed);
#endif /* !__KERNEL__ */
#endif /* __USE_POSIX */

#ifdef __NAMESPACE_STD_EXISTS
#ifndef __std_malloc_calloc_defined
#define __std_malloc_calloc_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL malloc)(size_t __n_bytes);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC void *(__LIBCCALL calloc)(size_t __count, size_t __n_bytes);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL realloc)(void *__restrict __mallptr, size_t __n_bytes);
#ifdef __KERNEL__
__LIBC __SAFE void (__LIBCCALL free)(void *__restrict __mallptr) __ASMNAME("kfree");
#else
__LIBC __SAFE void (__LIBCCALL free)(void *__restrict __mallptr);
#endif
__NAMESPACE_STD_END
#endif /* !__malloc_calloc_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __malloc_calloc_defined
#define __malloc_calloc_defined 1
__NAMESPACE_STD_USING(malloc)
__NAMESPACE_STD_USING(calloc)
__NAMESPACE_STD_USING(realloc)
__NAMESPACE_STD_USING(free)
#endif /* !__malloc_calloc_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#else /* __NAMESPACE_STD_EXISTS */
#ifndef __malloc_calloc_defined
#define __malloc_calloc_defined 1
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL malloc)(size_t __n_bytes);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC void *(__LIBCCALL calloc)(size_t __count, size_t __n_bytes);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL realloc)(void *__restrict __mallptr, size_t __n_bytes);
#ifdef __KERNEL__
__LIBC __SAFE void (__LIBCCALL free)(void *__restrict __mallptr) __ASMNAME("kfree");
#else
__LIBC __SAFE void (__LIBCCALL free)(void *__restrict __mallptr);
#endif
#endif /* !__malloc_calloc_defined */
#endif /* !__NAMESPACE_STD_EXISTS */


#ifdef __USE_MISC
#ifndef __cfree_defined
#define __cfree_defined 1
#ifdef __KERNEL__
__LIBC __SAFE void (__LIBCCALL cfree)(void *__restrict __mallptr) __ASMNAME("kfree");
#else /* __KERNEL__ */
__LIBC __SAFE void (__LIBCCALL cfree)(void *__restrict __mallptr) __ASMNAME("free");
#endif /* !__KERNEL__ */
#endif /* !__cfree_defined */
__LIBC int (__LIBCCALL getloadavg)(double __loadavg[], int __nelem);
#endif /* __USE_MISC */

#ifdef __USE_KOS
/* KOS extensions for accessing the kernel linker without needing to link against -ldl.
 * NOTE: Since KOS integrates the user-space linker in kernel-space, so as to speed up
 *       load times and especially cache characteristics significantly, there is no need
 *       for libraries to be loaded by a custom binary that would otherwise always sit
 *       in user-space, clobbering mapped memory just for the sake of it...
 *    >> Don't believe me? - Take a look at any dynamic binary's /proc/PID/map_files and
 *       tell me what you see. I'll tell you: '/lib/i386-linux-gnu/ld-2.23.so'
 *       That's right! That one's always linked in just to serve dynamic linking.
 *       Now that's not necessarily a bad thing, but when it comes to KOS, I chose
 *       to go a different route by hiding general purpose linking from userspace,
 *       keeping redundancy low and eliminating dependency on a second library probably
 *       even more important than libc itself.
 * NOTE: These functions follow usual LIBC semantics, returning -1/NULL and setting errno on error.
 *    >> the 'dlerror()' function you can find in libdl.so is literally just a swapper
 *       around 'strerror()' with the error number internally saved by libdl, so as not
 *       to clobber libc's thread-local 'errno' variable. */
__LIBC void *(__LIBCCALL xdlopen)(char const *__filename, int __flags);
__LIBC void *(__LIBCCALL xfdlopen)(int __fd, int __flags);
__LIBC void *(__LIBCCALL xdlsym)(void *__handle, char const *__symbol);
__LIBC int (__LIBCCALL xdlclose)(void *__handle);
#endif /* __USE_KOS */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K))
#ifndef __valloc_defined
#define __valloc_defined 1
__LIBC __SAFE __WUNUSED __MALL_ATTR_PAGEALIGNED
__ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL valloc)(size_t __n_bytes);
#endif /* !__valloc_defined */
#endif

#ifdef __USE_XOPEN2K
#ifndef __posix_memalign_defined
#define __posix_memalign_defined 1
__LIBC __SAFE __NONNULL((1)) int (__LIBCCALL posix_memalign)(void **__restrict __pp, size_t __alignment, size_t __n_bytes);
#endif /* !__posix_memalign_defined */
#endif /* __USE_XOPEN2K */

#ifdef __USE_ISOC11
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC
void *(__LIBCCALL aligned_alloc)(size_t __alignment, size_t __n_bytes) __ASMNAME("memalign");
#endif /* __USE_ISOC11 */


/* Debug hooks for malloc() and friends. */
#ifdef __USE_DEBUG
#if __USE_DEBUG != 0
#ifdef __NAMESPACE_STD_EXISTS
#ifndef __std_malloc_calloc_d_defined
#define __std_malloc_calloc_d_defined 1
/* Must also define these within the std:: namespace to allow the debug macro overrides to work:
 * >> namespace std {
 * >>    void *malloc(size_t);
 * >>    void *_malloc_d(size_t, __DEBUGINFO);
 * >> }
 * >> using std::malloc;
 * >> using std::_malloc_d;
 * >> 
 * >> #define malloc(s) _malloc_d(s,__DEBUGINFO_GEN)
 * >>
 * >> void *p = std::malloc(42); // Expands to 'std::_malloc_d(42,...)'
 */
__NAMESPACE_STD_BEGIN
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _malloc_d)(size_t __n_bytes, __DEBUGINFO);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC void *(__LIBCCALL _calloc_d)(size_t __count, size_t __n_bytes, __DEBUGINFO);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL _realloc_d)(void *__restrict __mallptr, size_t __n_bytes, __DEBUGINFO);
#ifdef __KERNEL__
__LIBC __SAFE void (__LIBCCALL _free_d)(void *__restrict __mallptr, __DEBUGINFO) __ASMNAME("_kfree_d");
#else /* __KERNEL__ */
__LIBC __SAFE void (__LIBCCALL _free_d)(void *__restrict __mallptr, __DEBUGINFO);
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END
#endif /* !__std_malloc_calloc_d_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __malloc_calloc_d_defined
#define __malloc_calloc_d_defined 1
__NAMESPACE_STD_USING(_malloc_d)
__NAMESPACE_STD_USING(_calloc_d)
__NAMESPACE_STD_USING(_realloc_d)
__NAMESPACE_STD_USING(_free_d)
#endif /* !__malloc_calloc_d_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#else /* __NAMESPACE_STD_EXISTS */
#ifndef __malloc_calloc_d_defined
#define __malloc_calloc_d_defined 1
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _malloc_d)(size_t __n_bytes, __DEBUGINFO);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC void *(__LIBCCALL _calloc_d)(size_t __count, size_t __n_bytes, __DEBUGINFO);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL _realloc_d)(void *__restrict __mallptr, size_t __n_bytes, __DEBUGINFO);
#ifdef __KERNEL__
__LIBC __SAFE void (__LIBCCALL _free_d)(void *__restrict __mallptr, __DEBUGINFO) __ASMNAME("_kfree_d");
#else /* __KERNEL__ */
__LIBC __SAFE void (__LIBCCALL _free_d)(void *__restrict __mallptr, __DEBUGINFO);
#endif /* !__KERNEL__ */
#endif /* !__malloc_calloc_d_defined */
#endif /* !__NAMESPACE_STD_EXISTS */

#ifdef __USE_MISC
#ifndef __cfree_d_defined
#define __cfree_d_defined 1
#ifdef __KERNEL__
__LIBC __SAFE void (__LIBCCALL _cfree_d)(void *__restrict __mallptr, __DEBUGINFO) __ASMNAME("_kfree_d");
#else /* __KERNEL__ */
__LIBC __SAFE void (__LIBCCALL _cfree_d)(void *__restrict __mallptr, __DEBUGINFO) __ASMNAME("_free_d");
#endif /* !__KERNEL__ */
#endif /* !__cfree_d_defined */
#endif /* __USE_MISC */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K))
#ifndef __valloc_d_defined
#define __valloc_d_defined 1
__LIBC __SAFE __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL _valloc_d)(size_t __n_bytes, __DEBUGINFO);
#endif /* !__valloc_d_defined */
#endif

#ifdef __USE_XOPEN2K
#ifndef __posix_memalign_d_defined
#define __posix_memalign_d_defined 1
__LIBC __SAFE __NONNULL((1)) int (__LIBCCALL _posix_memalign_d)(void **__restrict __pp, size_t __alignment, size_t __n_bytes, __DEBUGINFO);
#endif /* !__posix_memalign_d_defined */
#endif /* __USE_XOPEN2K */

#ifdef __USE_ISOC11
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC
void *(__LIBCCALL _aligned_alloc_d)(size_t __alignment, size_t __n_bytes) __ASMNAME("_memalign_d");
#endif /* __USE_ISOC11 */
#else /* __USE_DEBUG != 0 */
#ifndef __malloc_calloc_d_defined
#define __malloc_calloc_d_defined 1
#   define _malloc_d(n_bytes,...)                      malloc(n_bytes)
#   define _calloc_d(count,n_bytes,...)                calloc(count,n_bytes)
#   define _realloc_d(ptr,n_bytes,...)                 realloc(ptr,n_bytes)
#   define _free_d(ptr,...)                            free(ptr)
#endif /* !__malloc_calloc_d_defined */
#ifdef __USE_MISC
#ifndef __cfree_d_defined
#define __cfree_d_defined 1
#   define _cfree_d(ptr,...)                           cfree(ptr)
#endif /* !__cfree_d_defined */
#endif /* __USE_MISC */
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K))
#ifndef __valloc_d_defined
#define __valloc_d_defined 1
#   define _valloc_d(n_bytes,...)                      valloc(n_bytes)
#endif /* !__valloc_d_defined */
#endif
#ifdef __USE_XOPEN2K
#ifndef __posix_memalign_d_defined
#define __posix_memalign_d_defined 1
#   define _posix_memalign_d(pp,alignment,n_bytes,...) posix_memalign(pp,alignment,n_bytes)
#endif /* !__posix_memalign_d_defined */
#endif /* __USE_XOPEN2K */
#ifdef __USE_ISOC11
#   define _aligned_alloc_d(alignment,n_bytes,...)     aligned_alloc(alignment,n_bytes)
#endif /* __USE_ISOC11 */
#endif /* __USE_DEBUG == 0 */
#endif /* __USE_DEBUG */

#ifdef __USE_DEBUG_HOOK
#   undef  malloc
#   undef  calloc
#   undef  realloc
#   undef  free
#   define malloc(n_bytes)                           _malloc_d(n_bytes,__DEBUGINFO_GEN)
#   define calloc(count,n_bytes)                     _calloc_d(count,n_bytes,__DEBUGINFO_GEN)
#   define realloc(ptr,n_bytes)                      _realloc_d(ptr,n_bytes,__DEBUGINFO_GEN)
#   define free(ptr)                                 _free_d(ptr,__DEBUGINFO_GEN)
#ifdef __USE_MISC
#   undef  cfree
#   define cfree(ptr)                                _cfree_d(ptr,__DEBUGINFO_GEN)
#endif /* __USE_MISC */
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K))
#   undef  valloc
#   define valloc(n_bytes)                           _valloc_d(n_bytes,__DEBUGINFO_GEN)
#endif
#ifdef __USE_XOPEN2K
#   undef  posix_memalign
#   define posix_memalign(pp,alignment,n_bytes)      _posix_memalign_d(pp,alignment,n_bytes,__DEBUGINFO_GEN)
#endif /* __USE_XOPEN2K */
#ifdef __USE_ISOC11
#   undef  aligned_alloc
#   define aligned_alloc(alignment,n_bytes)          _aligned_alloc_d(alignment,n_bytes,__DEBUGINFO_GEN)
#endif /* __USE_ISOC11 */
#endif /* __USE_DEBUG_HOOK */


#ifdef __KERNEL__
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
#if __SIZEOF_LONG__ == 4
__LIBC long (__LIBCCALL random)(void) __ASMNAME("rand");
#endif /* __SIZEOF_LONG__ == 4 */
#if __SIZEOF_INT__ == 4
__LIBC void (__LIBCCALL srandom)(unsigned int __seed) __ASMNAME("srand");
#endif /* __SIZEOF_INT__ == 4 */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED  */

#else /* __KERNEL__ */

#define MB_CUR_MAX                 (__ctype_get_mb_cur_max())
__LIBC __WUNUSED size_t (__LIBCCALL __ctype_get_mb_cur_max)(void);

__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __NONNULL((1)) char *(__LIBCCALL getenv)(char const *__name) __UFS_FUNC(getenv);
__LIBC int (__LIBCCALL mblen)(char const *__s, size_t __n);
__LIBC int (__LIBCCALL mbtowc)(wchar_t *__restrict __pwc, char const *__restrict __s, size_t __n);
__LIBC int (__LIBCCALL wctomb)(char *__s, wchar_t __wchar);
__LIBC size_t (__LIBCCALL mbstowcs)(wchar_t *__restrict __pwcs, char const *__restrict __s, size_t __n);
__LIBC size_t (__LIBCCALL wcstombs)(char *__restrict __s, wchar_t const *__restrict __pwcs, size_t __n);
#ifndef __system_defined
__LIBC int (__LIBCCALL system)(char const *__command);
#endif /* !__system_defined */
#ifndef __abort_defined
__LIBC __ATTR_NORETURN void (__LIBCCALL abort)(void);
#endif /* !__abort_defined */
#ifndef __exit_defined
__LIBC __ATTR_NORETURN void (__LIBCCALL exit)(int __status);
#endif /* !__exit_defined */
__LIBC int (__LIBCCALL atexit)(void (*__LIBCCALL __func)(void));
#if defined(__USE_ISOC11) || defined(__USE_ISOCXX11)
__LIBC __ATTR_NORETURN void (__LIBCCALL quick_exit)(int __status);
#ifdef __cplusplus
extern "C++" { __LIBC __NONNULL((1)) int (__LIBCCALL at_quick_exit)(void (*__LIBCCALL __func) (void)) __ASMNAME("at_quick_exit"); }
#else /* __cplusplus */
__LIBC __NONNULL((1)) int (__LIBCCALL at_quick_exit)(void (*__LIBCCALL __func) (void));
#endif /* !__cplusplus */
#endif /* __USE_ISOC11 || __USE_ISOCXX11 */
#ifdef __USE_ISOC99
__LIBC __ATTR_NORETURN void (__LIBCCALL _Exit)(int __status) __ASMNAME("_exit");
#endif /* __USE_ISOC99 */
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(getenv)
__NAMESPACE_STD_USING(mblen)
__NAMESPACE_STD_USING(mbtowc)
__NAMESPACE_STD_USING(wctomb)
__NAMESPACE_STD_USING(mbstowcs)
__NAMESPACE_STD_USING(wcstombs)
#ifndef __system_defined
#define __system_defined 1
__NAMESPACE_STD_USING(system)
#endif /* !__system_defined */
#ifndef __abort_defined
#define __abort_defined 1
__NAMESPACE_STD_USING(abort)
#endif /* !__abort_defined */
#ifndef __exit_defined
#define __exit_defined 1
__NAMESPACE_STD_USING(exit)
#endif /* !__exit_defined */
__NAMESPACE_STD_USING(atexit)
#if defined(__USE_ISOC11) || defined(__USE_ISOCXX11)
__NAMESPACE_STD_USING(quick_exit)
__NAMESPACE_STD_USING(at_quick_exit)
#endif /* __USE_ISOC11 || __USE_ISOCXX11 */
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(_Exit)
#endif /* __USE_ISOC99 */

#if defined(__USE_MISC) || defined(__USE_XOPEN)
__LIBC double (__LIBCCALL drand48)(void);
__LIBC long (__LIBCCALL lrand48)(void);
__LIBC long (__LIBCCALL mrand48)(void);
__LIBC __NONNULL((1)) double (__LIBCCALL erand48)(unsigned short __xsubi[3]);
__LIBC __NONNULL((1)) long (__LIBCCALL nrand48)(unsigned short __xsubi[3]);
__LIBC __NONNULL((1)) long (__LIBCCALL jrand48)(unsigned short __xsubi[3]);
__LIBC void (__LIBCCALL srand48)(long __seedval);
__LIBC __NONNULL((1)) unsigned short *(__LIBCCALL seed48)(unsigned short __seed16v[3]);
__LIBC __NONNULL((1)) void (__LIBCCALL lcong48)(unsigned short __param[7]);
#endif /* __USE_MISC || __USE_XOPEN */

#if defined(__USE_MISC) || defined(__USE_XOPEN) || defined(__USE_DOS)
__LIBC __NONNULL((1)) int (__LIBCCALL putenv)(char *__string) __PE_ASMNAME("_putenv");
#endif /* __USE_MISC || __USE_XOPEN || __USE_DOS */

#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC long (__LIBCCALL random)(void);
__LIBC void (__LIBCCALL srandom)(unsigned int __seed);
__LIBC __NONNULL((2)) char *(__LIBCCALL initstate)(unsigned int __seed, char *__statebuf, size_t __statelen);
__LIBC __NONNULL((1)) char *(__LIBCCALL setstate)(char *__statebuf);
__LIBC __WUNUSED char *(__LIBCCALL l64a)(long __n);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) long (__LIBCCALL a64l)(char const *__s);
__LIBC __WUNUSED char *(__LIBCCALL realpath)(char const *__restrict __name, char *__restrict __resolved);
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED  */

#ifdef __USE_MISC
struct drand48_data {
 unsigned short __x[3];
 unsigned short __old_x[3];
 unsigned short __c;
 unsigned short __init;
 __ULONGLONG    __a;
};

__LIBC __NONNULL((1,2)) int (__LIBCCALL drand48_r)(struct drand48_data *__restrict __buffer, double *__restrict __result);
__LIBC __NONNULL((1,2)) int (__LIBCCALL erand48_r)(unsigned short __xsubi[3], struct drand48_data *__restrict __buffer, double *__restrict __result);
__LIBC __NONNULL((1,2)) int (__LIBCCALL lrand48_r)(struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __NONNULL((1,2)) int (__LIBCCALL nrand48_r)(unsigned short __xsubi[3], struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __NONNULL((1,2)) int (__LIBCCALL mrand48_r)(struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __NONNULL((1,2)) int (__LIBCCALL jrand48_r)(unsigned short __xsubi[3], struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __NONNULL((2))   int (__LIBCCALL srand48_r)(long __seedval, struct drand48_data *__buffer);
__LIBC __NONNULL((1,2)) int (__LIBCCALL seed48_r)(unsigned short __seed16v[3], struct drand48_data *__buffer);
__LIBC __NONNULL((1,2)) int (__LIBCCALL lcong48_r)(unsigned short __param[7], struct drand48_data *__buffer);

struct random_data {
 __INT32_TYPE__ *fptr;
 __INT32_TYPE__ *rptr;
 __INT32_TYPE__ *state;
 int             rand_type;
 int             rand_deg;
 int             rand_sep;
 __INT32_TYPE__ *end_ptr;
};
__LIBC __NONNULL((1,2)) int (__LIBCCALL random_r)(struct random_data *__restrict __buf, __INT32_TYPE__ *__restrict __result);
__LIBC __NONNULL((2)) int (__LIBCCALL srandom_r)(unsigned int __seed, struct random_data *__buf);
__LIBC __NONNULL((2,4)) int (__LIBCCALL initstate_r)(unsigned int __seed, char *__restrict __statebuf, size_t __statelen, struct random_data *__restrict __buf);
__LIBC __NONNULL((1,2)) int (__LIBCCALL setstate_r)(char *__restrict __statebuf, struct random_data *__restrict __buf);

__LIBC __NONNULL((1)) int (__LIBCCALL on_exit)(void (__LIBCCALL *__func)(int __status, void *__arg), void *__arg);
__LIBC int (__LIBCCALL clearenv)(void);
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemps)(char *__template, int __suffixlen) __FS_FUNC(mkstemps);
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemps64)(char *__template, int __suffixlen);
#endif /* __USE_LARGEFILE64 */
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL rpmatch)(char const *__response);
__LIBC __WUNUSED __NONNULL((3,4)) char *(__LIBCCALL qecvt)(long double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign);
__LIBC __WUNUSED __NONNULL((3,4)) char *(__LIBCCALL qfcvt)(long double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign);
#endif /* __USE_MISC */

#ifdef __USE_GNU
#include <xlocale.h>
#if defined(__PE__) && __SIZEOF_LONG__ == 4
__LIBC __NONNULL((1,4)) long (__LIBCCALL strtol_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtol_l");
__LIBC __NONNULL((1,4)) unsigned long (__LIBCCALL strtoul_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtoul_l");
#elif defined(__PE__) && __SIZEOF_LONG__ == 8
__LIBC __NONNULL((1,4)) long (__LIBCCALL strtol_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtoi64_l");
__LIBC __NONNULL((1,4)) unsigned long (__LIBCCALL strtoul_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtoui64_l");
#else
__LIBC __NONNULL((1,4)) long (__LIBCCALL strtol_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc);
__LIBC __NONNULL((1,4)) unsigned long (__LIBCCALL strtoul_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc);
#endif
#if defined(__PE__) && __SIZEOF_LONG_LONG__ == 8
__LIBC __NONNULL((1,4)) __LONGLONG (__LIBCCALL strtoll_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtoi64_l");
__LIBC __NONNULL((1,4)) __ULONGLONG (__LIBCCALL strtoull_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtoui64_l");
#else
__LIBC __NONNULL((1,4)) __LONGLONG (__LIBCCALL strtoll_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc);
__LIBC __NONNULL((1,4)) __ULONGLONG (__LIBCCALL strtoull_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc);
#endif
__LIBC __NONNULL((1,3)) double (__LIBCCALL strtod_l)(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __loc) __PE_ASMNAME("_strtod_l");
__LIBC __NONNULL((1,3)) float (__LIBCCALL strtof_l)(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __loc) __PE_ASMNAME("_strtod_l");
__LIBC __NONNULL((1,3)) long double (__LIBCCALL strtold_l)(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __loc) __PE_ASMNAME("_strtod_l");
__LIBC __WUNUSED __NONNULL((1)) char *(__LIBCCALL secure_getenv)(char const *__name);
__LIBC __WUNUSED __NONNULL((1)) char *(__LIBCCALL canonicalize_file_name)(char const *__name);
__LIBC __NONNULL((2)) int (__LIBCCALL ptsname_r)(int __fd, char *__buf, size_t __buflen);
__LIBC int (__LIBCCALL getpt)(void);

__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL mkostemp)(char *__template, int __flags) __FS_FUNC(mkostemp);
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL mkostemps)(char *__template, int __suffixlen, int __flags) __FS_FUNC(mkostemps);
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL mkostemp64)(char *__template, int __flags);
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL mkostemps64)(char *__template, int __suffixlen, int __flags);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_GNU */

#ifdef __USE_XOPEN2K
__LIBC __NONNULL((2)) int (__LIBCCALL setenv)(char const *__name, char const *__val, int __replace);
__LIBC __NONNULL((1)) int (__LIBCCALL unsetenv)(char const *__name);
#endif /* __USE_XOPEN2K */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8))
#ifndef __mktemp_defined
#define __mktemp_defined 1
__LIBC __NONNULL((1)) char *(__LIBCCALL mktemp)(char *__template) __PE_FUNC_OLDPEA(mktemp);
#endif /* !__mktemp_defined */
#endif

#if defined(__USE_MISC) || defined(__USE_DOS) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8))
__LIBC __WUNUSED __NONNULL((3,4)) char *(__LIBCCALL ecvt)(double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign) __PE_ASMNAME("_ecvt");
__LIBC __WUNUSED __NONNULL((3,4)) char *(__LIBCCALL fcvt)(double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign) __PE_ASMNAME("_fcvt");
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
__LIBC __WUNUSED __NONNULL((1,2,3)) int (__LIBCCALL getsubopt)(char **__restrict __optionp, char *const *__restrict __tokens, char **__restrict __valuep);
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemp)(char *__template) __FS_FUNC(mkstemp);
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemp64)(char *__template);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */

#ifdef __USE_XOPEN2K8
__LIBC __WUNUSED __NONNULL((1)) char *(__LIBCCALL mkdtemp)(char *__template);
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_XOPEN
__LIBC __NONNULL((1)) void (__LIBCCALL setkey)(char const *__key);
__LIBC int (__LIBCCALL grantpt)(int __fd);
__LIBC int (__LIBCCALL unlockpt)(int __fd);
__LIBC __WUNUSED char *(__LIBCCALL ptsname)(int __fd);
#endif /* __USE_XOPEN */

#ifdef __USE_XOPEN2KXSI
__LIBC __WUNUSED int (__LIBCCALL posix_openpt)(int __oflag);
#endif /* __USE_XOPEN2KXSI */

/* DOS Extensions. */
#ifdef __USE_DOS
#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef _ONEXIT_T_DEFINED
#define _ONEXIT_T_DEFINED 1
typedef int (__LIBCCALL *_onexit_t)(void);
#define onexit_t         _onexit_t
#endif  /* _ONEXIT_T_DEFINED */

#if defined(__DCC_VERSION__) || \
   (__has_builtin(__builtin_min) && __has_builtin(__builtin_max))
#   define __min(a,b) __builtin_min(a,b)
#   define __max(a,b) __builtin_max(a,b)
#elif defined(__COMPILER_HAVE_TYPEOF) && !defined(__NO_XBLOCK)
#   define __min(a,b) __XBLOCK({ __typeof__(a) _a = (a),_b = (b); __XRETURN _a < _b ? _a : _b; })
#   define __max(a,b) __XBLOCK({ __typeof__(a) _a = (a),_b = (b); __XRETURN _b < _a ? _a : _b; })
#else
#   define __min(a,b) ((a) < (b) ? (a) : (b))
#   define __max(a,b) ((b) < (a) ? (a) : (b))
#endif

#define _MAX_PATH         260
#define _MAX_DRIVE        3
#define _MAX_DIR          256
#define _MAX_FNAME        256
#define _MAX_EXT          256
#define _OUT_TO_DEFAULT   0
#define _OUT_TO_STDERR    1
#define _OUT_TO_MSGBOX    2
#define _REPORT_ERRMODE   3
#define _WRITE_ABORT_MSG  0x1
#define _CALL_REPORTFAULT 0x2
#define _MAX_ENV          0x7fff

#ifndef _CRT_ERRNO_DEFINED
#define _CRT_ERRNO_DEFINED 1
__LIBC int    *__NOTHROW((__LIBCCALL __errno)(void)) __DOS_FUNC_(_errno);
__LIBC errno_t __NOTHROW((__LIBCCALL __get_errno)(void)) __DOS_FUNC_(_get_errno);
__LIBC errno_t __NOTHROW((__LIBCCALL __set_errno)(errno_t __err)) __DOS_FUNC_(_set_errno);
#define errno         (*__errno())
#endif /* !_CRT_ERRNO_DEFINED */

#define _doserrno    (*__doserrno())
__LIBC __UINT32_TYPE__ *__NOTHROW((__LIBCCALL __doserrno)(void));
__LIBC errno_t __NOTHROW((__LIBCCALL _get_doserrno)(__UINT32_TYPE__ *__perr));
__LIBC errno_t __NOTHROW((__LIBCCALL _set_doserrno)(__UINT32_TYPE__ __err));

__LIBC char **__NOTHROW((__LIBCCALL __sys_errlist)(void));
__LIBC int *__NOTHROW((__LIBCCALL __sys_nerr)(void));
#define _sys_errlist (__sys_errlist())
#define _sys_nerr    (*__sys_nerr())

#ifdef __PE__
__LIBC char ***__NOTHROW((__LIBCCALL __p__environ)(void));
#define _environ  (*__p__environ())
#else /* __PE__ */
#ifndef ____environ_defined
#define ____environ_defined 1
#undef __environ
__LIBC char **__environ __ASMNAME2("environ","_environ");
#endif /* !____environ_defined */
#define _environ    __environ
#endif /* !__PE__ */

__LIBC int *__NOTHROW((__LIBCCALL __p___argc)(void));
__LIBC char ***__NOTHROW((__LIBCCALL __p___argv)(void));
__LIBC char **__NOTHROW((__LIBCCALL __p__pgmptr)(void));
__LIBC wchar_t ***(__LIBCCALL __p___wargv)(void) __KOS_ASMNAME("wgetargv");
__LIBC wchar_t ***(__LIBCCALL __p__wenviron)(void) __KOS_ASMNAME("wgetenviron");
__LIBC wchar_t **(__LIBCCALL __p__wpgmptr)(void) __KOS_ASMNAME("wgetpgmptr");
#define __argc  (*__p___argc())
#define __argv  (*__p___argv())
#define __wargv   (*__p___wargv())
#define _wenviron (*__p__wenviron())
#define _pgmptr   (*__p__pgmptr())
#define _wpgmptr  (*__p__wpgmptr())

#ifdef __USE_KOS
/* Access to the initial environment block. */
__LIBC char ***(__LIBCCALL __p___initenv)(void);
__LIBC wchar_t ***(__LIBCCALL __p___winitenv)(void) __KOS_ASMNAME("wgetinitenv");
#define _initenv  (*__p___initenv())
#define _winitenv (*__p___winitenv())
#endif /* __USE_KOS */

#ifdef __USE_UTF
#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

__LIBC char16_t ***(__LIBCCALL __p___uinitenv)(void) __ASMNAME("__p___winitenv");
__LIBC char32_t ***(__LIBCCALL __p___Uinitenv)(void) __ASMNAME("wgetinitenv");
#define _uinitenv  (*__p___uinitenv())
#define _Uinitenv  (*__p___Uinitenv())
#endif /* __USE_UTF */

#ifndef _countof
#define _countof(a) __COMPILER_LENOF(a)
#endif /* !_countof */

#if __SIZEOF_LONG_LONG__ == 8
__LIBC __INT64_TYPE__ __NOTHROW((__LIBCCALL _abs64)(__INT64_TYPE__ __x)) __ASMNAME("llabs");
#elif __SIZEOF_LONG__ == 8
__LIBC __INT64_TYPE__ __NOTHROW((__LIBCCALL _abs64)(__INT64_TYPE__ __x)) __ASMNAME("labs");
#elif __SIZEOF_INTMAX_T__ == 8
__LIBC __INT64_TYPE__ __NOTHROW((__LIBCCALL _abs64)(__INT64_TYPE__ __x)) __ASMNAME("imaxabs");
#else
__LIBC __INT64_TYPE__ __NOTHROW((__LIBCCALL _abs64)(__INT64_TYPE__ __x));
#endif

__LIBC double (__LIBCCALL _atof_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC int (__LIBCCALL _atoi_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC long int (__LIBCCALL _atol_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC __LONGLONG (__LIBCCALL _atoll_l)(char const *__restrict __nptr, __locale_t __locale);
__LOCAL __UINT16_TYPE__ (__LIBCCALL _byteswap_ushort)(__UINT16_TYPE__ __val) { return __bswap_16(__val); }
__LOCAL __UINT32_TYPE__ (__LIBCCALL _byteswap_ulong)(__UINT32_TYPE__ __val) { return __bswap_32(__val); }
__LOCAL __UINT64_TYPE__ (__LIBCCALL _byteswap_uint64)(__UINT64_TYPE__ __val) { return __bswap_64(__val); }


#ifdef __USE_DOS_SLIB
#ifndef __rsize_t_defined
#define __rsize_t_defined 1
typedef size_t rsize_t;
#endif /* !__rsize_t_defined */

#ifndef _CRT_ALGO_DEFINED
#define _CRT_ALGO_DEFINED
__LIBC __NONNULL((1,2,5)) void *(__LIBCCALL bsearch_s)(void const *__key, void const *__base, size_t __nmemb, size_t __size, int (__LIBCCALL *__compar)(void *__arg, void const *__a, void const *__b), void *__arg);
__LIBC __NONNULL((1,4)) void (__LIBCCALL qsort_s)(void *__base, size_t __nmemb, size_t __size, int (__LIBCCALL *__compar)(void *__arg, void const *__a, void const *__b), void *__arg);
#endif  /* _CRT_ALGO_DEFINED */
__LIBC __WARN_NOKOSFS errno_t (__LIBCCALL getenv_s)(size_t *__psize, char *__buf, rsize_t __bufsize, char const *__name);
__LIBC __WARN_NOKOSFS errno_t (__LIBCCALL _dupenv_s)(char **__restrict __pbuf, size_t *__pbuflen, char const *__name);
#endif /* __USE_DOS_SLIB */

__LIBC char *(__LIBCCALL _itoa)(int __val, char *__dst, int __radix);
__LIBC char *(__LIBCCALL _i64toa)(__INT64_TYPE__ __val, char *__dst, int __radix);
__LIBC char *(__LIBCCALL _ui64toa)(__UINT64_TYPE__ __val, char *__dst, int __radix);
__LIBC errno_t (__LIBCCALL _itoa_s)(int __val, char *__dst, size_t __bufsize, int __radix);
__LIBC errno_t (__LIBCCALL _i64toa_s)(__INT64_TYPE__ __val, char *__dst, size_t __bufsize, int __radix);
__LIBC errno_t (__LIBCCALL _ui64toa_s)(__UINT64_TYPE__ __val, char *__dst, size_t __bufsize, int __radix);
__LIBC __INT64_TYPE__ (__LIBCCALL _atoi64)(char const * __nptr);
__LIBC __INT64_TYPE__ (__LIBCCALL _atoi64_l)(char const * __nptr, __locale_t __locale);
__LIBC __INT64_TYPE__ (__LIBCCALL _strtoi64)(char const * __nptr, char ** __endptr, int __radix);
__LIBC __INT64_TYPE__ (__LIBCCALL _strtoi64_l)(char const * __nptr, char ** __endptr, int __radix, __locale_t __locale);
__LIBC __UINT64_TYPE__ (__LIBCCALL _strtoui64)(char const * __nptr, char ** __endptr, int __radix);
__LIBC __UINT64_TYPE__ (__LIBCCALL _strtoui64_l)(char const * __nptr, char ** __endptr, int  __radix, __locale_t __locale);

#if __SIZEOF_LONG__ == 8
__LIBC char *(__LIBCCALL _ltoa)(long __val,  char *__buf, int __radix) __ASMNAME("_i64toa");
__LIBC errno_t (__LIBCCALL _ltoa_s)(long __val, char *__buf, size_t __buflen, int __radix) __ASMNAME("_i64toa_s");
#elif __SIZEOF_LONG__ == __SIZEOF_INT__
__LIBC char *(__LIBCCALL _ltoa)(long __val,  char *__buf, int __radix) __ASMNAME("_itoa");
__LIBC errno_t (__LIBCCALL _ltoa_s)(long __val, char *__buf, size_t __buflen, int __radix) __ASMNAME("_itoa_s");
#else
__LIBC char *(__LIBCCALL _ltoa)(long __val,  char *__buf, int __radix);
__LIBC errno_t (__LIBCCALL _ltoa_s)(long __val, char *__buf, size_t __buflen, int __radix);
#endif
__LIBC size_t (__LIBCCALL _mbstrlen)(char const *__str);
__LIBC size_t (__LIBCCALL _mbstrlen_l)(char const *__str, __locale_t __locale);
__LIBC size_t (__LIBCCALL _mbstrnlen)(char const *__str, size_t __maxlen);
__LIBC size_t (__LIBCCALL _mbstrnlen_l)(char const *__str, size_t __maxlen, __locale_t __locale);
#ifdef __USE_KOS
__LIBC size_t (__LIBCCALL _mblen_l)(char const *__str, size_t __maxlen, __locale_t __locale) __KOS_ASMNAME("mblen_l");
__LIBC size_t (__LIBCCALL _mbtowc_l)(wchar_t *__dst, char const *__src, size_t __srclen, __locale_t __locale) __KOS_ASMNAME("mbtowc_l");
#else /* __USE_KOS */
__LIBC int (__LIBCCALL _mblen_l)(char const *__str, size_t __maxlen, __locale_t __locale) __KOS_ASMNAME("mblen_l");
__LIBC int (__LIBCCALL _mbtowc_l)(wchar_t *__dst, char const *__src, size_t __srclen, __locale_t __locale) __KOS_ASMNAME("mbtowc_l");
#endif /* !__USE_KOS */
__LIBC errno_t (__LIBCCALL mbstowcs_s)(size_t *__presult, wchar_t *__buf, size_t __buflen, char const *__src, size_t __maxlen);
__LIBC errno_t (__LIBCCALL _mbstowcs_s_l)(size_t *__presult, wchar_t *__buf, size_t __buflen, char const *__src, size_t __maxlen, __locale_t __locale) __KOS_ASMNAME("mbstowcs_s_l");
__LIBC size_t (__LIBCCALL _mbstowcs_l)(wchar_t *__buf, char const *__src, size_t __maxlen, __locale_t __locale) __KOS_ASMNAME("mbstowcs_l");
__LIBC errno_t (__LIBCCALL rand_s)(unsigned int *__restrict __randval);
__LIBC int (__LIBCCALL _set_error_mode)(int __mode);
#if defined(__PE__) && __SIZEOF_LONG__ == 4
__LIBC __NONNULL((1,4)) long (__LIBCCALL _strtol_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtol_l");
__LIBC __NONNULL((1,4)) unsigned long (__LIBCCALL _strtoul_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtoul_l");
#elif defined(__PE__) && __SIZEOF_LONG__ == 8
__LIBC __NONNULL((1,4)) long (__LIBCCALL _strtol_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtoi64_l");
__LIBC __NONNULL((1,4)) unsigned long (__LIBCCALL _strtoul_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtoui64_l");
#else
__LIBC __NONNULL((1,4)) long (__LIBCCALL _strtol_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("strtol_l");
__LIBC __NONNULL((1,4)) unsigned long (__LIBCCALL _strtoul_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("strtoul_l");
#endif
#if defined(__PE__) && __SIZEOF_LONG_LONG__ == 8
__LIBC __NONNULL((1,4)) __LONGLONG (__LIBCCALL _strtoll_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtoi64_l");
__LIBC __NONNULL((1,4)) __ULONGLONG (__LIBCCALL _strtoull_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("_strtoui64_l");
#else
__LIBC __NONNULL((1,4)) __LONGLONG (__LIBCCALL _strtoll_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("strtoll_l");
__LIBC __NONNULL((1,4)) __ULONGLONG (__LIBCCALL _strtoull_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc) __ASMNAME("strtoull_l");
#endif
__LIBC float (__LIBCCALL _strtof_l)(char const *__nptr, char ** __restrict __endptr, __locale_t __locale) __ASMNAME2("strtof_l","_strtod_l");
__LIBC double (__LIBCCALL _strtod_l)(char const *__nptr, char ** __restrict __endptr, __locale_t __locale) __KOS_ASMNAME("strtod_l");
__LIBC long double (__LIBCCALL _strtold_l)(char const *__nptr, char ** __restrict __endptr, __locale_t __locale) __ASMNAME2("strtof_l","_strtod_l");
#ifndef _CRT_SYSTEM_DEFINED
#define _CRT_SYSTEM_DEFINED 1
#endif /* !_CRT_SYSTEM_DEFINED */

__LIBC errno_t (__LIBCCALL _ultoa_s)(unsigned long int __val, char *__buf, size_t __bufsize, int __radix);
__LIBC char *(__LIBCCALL _ultoa)(unsigned long int __val, char *__buf, int __radix);
#ifdef __USE_KOS
__LIBC size_t (__LIBCCALL _wctomb_l)(char *__buf, wchar_t __wc, __locale_t __locale) __KOS_ASMNAME("wctomb_l");
#else /* __USE_KOS */
__LIBC int (__LIBCCALL _wctomb_l)(char *__buf, wchar_t __wc, __locale_t __locale) __KOS_ASMNAME("wctomb_l");
#endif /* !__USE_KOS */
#ifdef __USE_DOS_SLIB
__LIBC errno_t (__LIBCCALL wctomb_s)(int *__presult, char *__buf, rsize_t __buflen, wchar_t __wc);
#endif /* __USE_DOS_SLIB */
__LIBC errno_t (__LIBCCALL _wctomb_s_l)(int *__presult, char *__buf, size_t __buflen, wchar_t __wc, __locale_t __locale) __KOS_ASMNAME("wctomb_s_l");
__LIBC errno_t (__LIBCCALL wcstombs_s)(size_t *__presult, char *__buf, size_t __buflen, wchar_t const *__src, size_t __maxlen);
__LIBC errno_t (__LIBCCALL _wcstombs_s_l)(size_t *__presult, char *__buf, size_t __buflen, wchar_t const *__src, size_t __maxlen, __locale_t __locale) __KOS_ASMNAME("wcstombs_s_l");
__LIBC size_t (__LIBCCALL _wcstombs_l)(char *__dst, wchar_t const *__src, size_t __maxlen, __locale_t __locale) __KOS_ASMNAME("wcstombs_l");

/* DOS malloc extensions. */
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2,3)) void *(__LIBCCALL _recalloc)(void *__mptr, size_t __count, size_t __size);
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_ALIGN(2) __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _aligned_malloc)(size_t __size, size_t __align);
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _aligned_offset_malloc)(size_t __size, size_t __align, size_t __off);
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_ALIGN(3) __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL _aligned_realloc)(void *__mptr, size_t __newsize, size_t __align);
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_ALIGN(4) __ATTR_ALLOC_SIZE((2,3)) void *(__LIBCCALL _aligned_recalloc)(void *__mptr, size_t __count, size_t __size, size_t __align);
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL _aligned_offset_realloc)(void *__mptr, size_t __newsize, size_t __align, size_t __off);
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_SIZE((1,2)) void *(__LIBCCALL _aligned_offset_recalloc)(void *__mptr, size_t __count, size_t __size, size_t __align, size_t __off);
__LIBC __SAFE __WUNUSED __NONNULL((1)) size_t (__LIBCCALL _aligned_msize)(void *__mptr, size_t __align, size_t __off);
__LIBC __SAFE void  (__LIBCCALL _aligned_free)(void *__mptr);

#ifndef _WSTDLIB_DEFINED
#define _WSTDLIB_DEFINED 1
__LIBC wchar_t *(__LIBCCALL _i64tow)(__INT64_TYPE__ __val, wchar_t *__restrict __dst, int __radix) __KOS_ASMNAME("i64tow");
__LIBC wchar_t *(__LIBCCALL _ui64tow)(__UINT64_TYPE__ __val, wchar_t *__restrict __dst, int __radix) __KOS_ASMNAME("ui64tow");
__LIBC errno_t (__LIBCCALL _i64tow_s)(__INT64_TYPE__ __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix) __KOS_ASMNAME("i64tow_s");
__LIBC errno_t (__LIBCCALL _ui64tow_s)(__UINT64_TYPE__ __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix) __KOS_ASMNAME("ui64tow_s");

#if __SIZEOF_LONG__ == 8
__LIBC wchar_t *(__LIBCCALL _ultow)(unsigned long int __val,  wchar_t *__restrict __dst, int __radix) __ASMNAME2("ui64tow","_ui64tow");
__LIBC wchar_t *(__LIBCCALL _ltow)(long int __val, wchar_t *__restrict __dst, int __radix) __ASMNAME2("i64tow","_i64tow");
__LIBC errno_t (__LIBCCALL _ultow_s)(unsigned long int __val, wchar_t *__dst, size_t __maxlen, int __radix) __ASMNAME2("ui64tow_s","_ui64tow_s");
__LIBC errno_t (__LIBCCALL _ltow_s)(long int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix) __ASMNAME2("i64tow_s","_i64tow_s");
#elif __SIZEOF_LONG__ == 4
__LIBC wchar_t *(__LIBCCALL _ultow)(unsigned long int __val,  wchar_t *__restrict __dst, int __radix) __KOS_ASMNAME("ultow");
__LIBC wchar_t *(__LIBCCALL _ltow)(long int __val, wchar_t *__restrict __dst, int __radix) __KOS_ASMNAME("ltow");
__LIBC errno_t (__LIBCCALL _ultow_s)(unsigned long int __val, wchar_t *__dst, size_t __maxlen, int __radix) __KOS_ASMNAME("ultow_s");
__LIBC errno_t (__LIBCCALL _ltow_s)(long int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix) __KOS_ASMNAME("ltow_s");
#else
#error "ERROR: Unsupported 'sizeof(long) != 4 && sizeof(long) != 8'"
#endif /* __SIZEOF_LONG__ != 8 */

__LIBC long int (__LIBCCALL _wtol)(wchar_t const *__restrict __s) __KOS_ASMNAME("wtol");
__LIBC long int (__LIBCCALL _wtol_l)(wchar_t const *__restrict __s, __locale_t __locale) __KOS_ASMNAME("wtol_l");
__LIBC __LONGLONG (__LIBCCALL _wtoll)(wchar_t const *__restrict __s) __ASMNAME2("wtoi64","_wtoi64");
__LIBC __LONGLONG (__LIBCCALL _wtoll_l)(wchar_t const *__restrict __s, __locale_t __locale) __ASMNAME2("wtoi64_l","_wtoi64_l");

#if __SIZEOF_INT__ == __SIZEOF_LONG__
__LIBC int (__LIBCCALL _wtoi)(wchar_t const *__restrict __s) __ASMNAME2("wtol","_wtol");
__LIBC int (__LIBCCALL _wtoi_l)(wchar_t const *__restrict __s, __locale_t __locale) __ASMNAME2("wtol_l","_wtol_l");
#else /* __SIZEOF_INT__ == __SIZEOF_LONG__ */
__LIBC int (__LIBCCALL _wtoi)(wchar_t const *__restrict __s) __KOS_ASMNAME("wtoi");
__LIBC int (__LIBCCALL _wtoi_l)(wchar_t const *__restrict __s, __locale_t __locale) __KOS_ASMNAME("wtoi_l");
#endif /* __SIZEOF_INT__ != __SIZEOF_LONG__ */

__LIBC wchar_t *(__LIBCCALL _itow)(int __val, wchar_t *__restrict __dst, int __radix) __KOS_ASMNAME("itow");
__LIBC errno_t (__LIBCCALL _itow_s)(int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix) __KOS_ASMNAME("itow_s");
__LIBC double (__LIBCCALL _wtof)(wchar_t const *__restrict __s) __KOS_ASMNAME("wtof");
__LIBC double (__LIBCCALL _wtof_l)(wchar_t const *__restrict __s, __locale_t __locale) __KOS_ASMNAME("wtof_l");
__LIBC long int (__LIBCCALL _wcstol_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __KOS_ASMNAME("wcstol_l");
__LIBC unsigned long int (__LIBCCALL _wcstoul_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __KOS_ASMNAME("wcstoul_l");
#if defined(__PE__) && __SIZEOF_LONG_LONG__ == 8
__LIBC __LONGLONG (__LIBCCALL _wcstoll_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME("_wcstoi64_l");
__LIBC __ULONGLONG (__LIBCCALL _wcstoull_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME("_wcstoui64_l");
#else
__LIBC __LONGLONG (__LIBCCALL _wcstoll_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __KOS_ASMNAME("wcstoll_l");
__LIBC __ULONGLONG (__LIBCCALL _wcstoull_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __KOS_ASMNAME("wcstoull_l");
#endif
__LIBC float (__LIBCCALL _wcstof_l)(wchar_t const *__restrict __s, wchar_t **__restrict __pend, __locale_t __locale) __KOS_ASMNAME("wcstof_l");
__LIBC double (__LIBCCALL _wcstod_l)(wchar_t const *__restrict __s, wchar_t **__pend, __locale_t __locale) __KOS_ASMNAME("wcstod_l");
__LIBC long double (__LIBCCALL _wcstold_l)(wchar_t const *__restrict __s, wchar_t **__pend, __locale_t __locale) __KOS_ASMNAME("wcstold_l");
__LIBC wchar_t *(__LIBCCALL _wgetenv)(wchar_t const *__restrict __varname) __KOS_ASMNAME("wgetenv");
__LIBC errno_t (__LIBCCALL _wgetenv_s)(size_t *__restrict __psize, wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __varname) __KOS_ASMNAME("wgetenv_s");
__LIBC errno_t (__LIBCCALL _wdupenv_s)(wchar_t **__restrict __pbuf, size_t *__restrict __pbuflen, wchar_t const *__restrict __varname) __KOS_ASMNAME("wdupenv_s");

#if __SIZEOF_LONG__ == 8
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) __ASMNAME("wcstol");
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME2("wcstol_l","_wcstol_l");
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) __ASMNAME("wcstoul");
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale) __ASMNAME2("wcstoul_l","_wcstoul_l");
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) __ASMNAME("_wtol");
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) __ASMNAME("_wtol_l");
#elif __SIZEOF_LONG_LONG__ == 8 && !defined(__PE__)
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) __KOS_ASMNAME("wcstoll");
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __KOS_ASMNAME("wcstoll_l");
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) __KOS_ASMNAME("wcstoull");
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale) __KOS_ASMNAME("wcstoull_l");
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) __KOS_ASMNAME("wtoll");
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) __KOS_ASMNAME("wtoll_l");
#else
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) __KOS_ASMNAME("wcstoi64");
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) __KOS_ASMNAME("wcstoi64_l");
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) __KOS_ASMNAME("wcstoui64");
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale) __KOS_ASMNAME("wcstoui64_l");
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) __KOS_ASMNAME("wtoi64");
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) __KOS_ASMNAME("wtoi64_l");
#endif

#ifndef _CRT_WSYSTEM_DEFINED
#define _CRT_WSYSTEM_DEFINED 1
__LIBC int (__LIBCCALL _wsystem)(wchar_t const *__restrict __cmd);
#endif /* !_CRT_WSYSTEM_DEFINED */
#ifndef __wcstol_defined
#define __wcstol_defined 1
__NAMESPACE_STD_BEGIN
__LIBC long int (__LIBCCALL wcstol)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC unsigned long int (__LIBCCALL wcstoul)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcstol)
__NAMESPACE_STD_USING(wcstoul)
#endif /* !__wcstol_defined */

#ifndef __wcstoll_defined
#define __wcstoll_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __LONGLONG (__LIBCCALL wcstoll)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC __ULONGLONG (__LIBCCALL wcstoull)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcstoll)
__NAMESPACE_STD_USING(wcstoull)
#endif /* !__wcstoll_defined */

#ifndef __wcstod_defined
#define __wcstod_defined 1
__NAMESPACE_STD_BEGIN
__LIBC double (__LIBCCALL wcstod)(wchar_t const *__restrict __s, wchar_t **__pend);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcstod)
#endif /* !__wcstod_defined */

#ifndef __wcstof_defined
#define __wcstof_defined 1
__NAMESPACE_STD_BEGIN
__LIBC float (__LIBCCALL wcstof)(wchar_t const *__restrict __s, wchar_t **__pend);
__LIBC long double (__LIBCCALL wcstold)(wchar_t const *__restrict __s, wchar_t **__pend);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcstof)
__NAMESPACE_STD_USING(wcstold)
#endif /* !__wcstof_defined */
#endif /* !_WSTDLIB_DEFINED */

#define _CVTBUFSIZE   349
__LIBC char *(__LIBCCALL _fullpath)(char *__buf, const char *__path, size_t __buflen);
__LIBC __WUNUSED __NONNULL((3,4)) char *(__LIBCCALL _ecvt)(double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign) __KOS_ASMNAME("ecvt");
__LIBC __WUNUSED __NONNULL((3,4)) char *(__LIBCCALL _fcvt)(double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign) __KOS_ASMNAME("fcvt");
__LIBC __WUNUSED __NONNULL((3)) char *(__LIBCCALL _gcvt)(double __val, int __ndigit, char *__buf) __KOS_ASMNAME("gcvt");
__LIBC errno_t (__LIBCCALL _ecvt_s)(char *__buf, size_t __buflen, double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign);
__LIBC errno_t (__LIBCCALL _fcvt_s)(char *__buf, size_t __buflen, double __val, int __ndigit, int *__restrict __decpt, int *__restrict __sign);
__LIBC errno_t (__LIBCCALL _gcvt_s)(char *__buf, size_t __buflen, double __val, int __ndigit);

#ifdef __USE_KOS
#define __FIXED_CONST const
#else
#define __FIXED_CONST /* nothing */
#endif
__LIBC int (__LIBCCALL _atoflt)(float *__restrict __result, char const *__restrict __nptr);
__LIBC int (__LIBCCALL _atodbl)(double *__restrict __result, char __FIXED_CONST *__restrict __nptr);
__LIBC int (__LIBCCALL _atoldbl)(long double *__restrict __result, char __FIXED_CONST *__restrict __nptr);
__LIBC int (__LIBCCALL _atoflt_l)(float *__restrict __result, char const *__restrict __nptr, __locale_t __locale);
__LIBC int (__LIBCCALL _atodbl_l)(double *__restrict __result, char __FIXED_CONST *__restrict __nptr, __locale_t __locale);
__LIBC int (__LIBCCALL _atoldbl_l)(long double *__restrict __result, char __FIXED_CONST *__restrict __nptr, __locale_t __locale);
#undef __FIXED_CONST
__LIBC unsigned int (__LIBCCALL _rotl)(unsigned int __val, int __shift);
__LIBC unsigned int (__LIBCCALL _rotr)(unsigned int __val, int __shift);
__LIBC __UINT64_TYPE__ (__LIBCCALL _rotl64)(__UINT64_TYPE__ __val, int __shift);
__LIBC __UINT64_TYPE__ (__LIBCCALL _rotr64)(__UINT64_TYPE__ __val, int __shift);
#if __SIZEOF_LONG__ == __SIZEOF_INT__
__LIBC unsigned long int (__LIBCCALL _lrotl)(unsigned long int __val, int __shift) __ASMNAME("_rotl");
__LIBC unsigned long int (__LIBCCALL _lrotr)(unsigned long int __val, int __shift) __ASMNAME("_rotr");
#elif __SIZEOF_LONG__ == 8
__LIBC unsigned long int (__LIBCCALL _lrotl)(unsigned long int __val, int __shift) __ASMNAME("_rotl64");
__LIBC unsigned long int (__LIBCCALL _lrotr)(unsigned long int __val, int __shift) __ASMNAME("_rotr64");
#else
__LIBC unsigned long int (__LIBCCALL _lrotl)(unsigned long int __val, int __shift);
__LIBC unsigned long int (__LIBCCALL _lrotr)(unsigned long int __val, int __shift);
#endif


#ifndef _CRT_PERROR_DEFINED
#define _CRT_PERROR_DEFINED 1
#ifndef __perror_defined
#define __perror_defined 1
__NAMESPACE_STD_BEGIN
__LIBC void (__LIBCCALL perror)(char const *__s);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(perror)
#endif /* !__perror_defined */
#endif  /* _CRT_PERROR_DEFINED */

__LIBC int (__LIBCCALL _putenv)(const char *__envstr) __KOS_ASMNAME("putenv");
__LIBC errno_t (__LIBCCALL _putenv_s)(const char *__name, const char *__val);

__LIBC void (__LIBCCALL _makepath)(char *__buf, const char *__drive, const char *__dir, const char *__file, const char *__ext);
__LIBC void (__LIBCCALL _searchenv)(const char *__file, const char *__envvar, char *__resultpath);
__LIBC void (__LIBCCALL _splitpath)(const char *__abspath, char *__drive, char *__dir, char *__file, char *__ext);
__LIBC errno_t (__LIBCCALL _makepath_s)(char *__buf, size_t __buflen, const char *__drive, const char *__dir, const char *__file, const char *__ext);
__LIBC errno_t (__LIBCCALL _searchenv_s)(const char *__file, const char *__envvar, char *__resultpath, size_t __buflen);
__LIBC errno_t (__LIBCCALL _splitpath_s)(const char *__abspath, char *__drive, size_t __drivelen, char *__dir, size_t __dirlen, char *__file, size_t __filelen, char *__ext, size_t __extlen);

#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__ && !defined(__PE__)
__LIBC __NONNULL((1,2)) void (__LIBCCALL _swab)(void const *__restrict __from, void *__restrict __to, int __n_bytes) __ASMNAME("swab");
#else
__LIBC __NONNULL((1,2)) void (__LIBCCALL _swab)(void const *__restrict __from, void *__restrict __to, int __n_bytes);
#endif

#ifndef _WSTDLIBP_DEFINED
#define _WSTDLIBP_DEFINED 1
__LIBC wchar_t *(__LIBCCALL _wfullpath)(wchar_t *__restrict __abspath, wchar_t const *__restrict __path, size_t __maxlen);
__LIBC int (__LIBCCALL _wputenv)(wchar_t const *__restrict __envstr);
__LIBC void (__LIBCCALL _wmakepath)(wchar_t *__restrict __dst, wchar_t const *__restrict __drive, wchar_t const *__restrict __dir, wchar_t const *__restrict __file, wchar_t const *__restrict __ext);
__LIBC void (__LIBCCALL _wsearchenv)(wchar_t const *__restrict __file, wchar_t const *__restrict __varname,  wchar_t *__restrict __dst);
__LIBC void (__LIBCCALL _wsplitpath)(wchar_t const *__restrict __abspath, wchar_t *__restrict __drive, wchar_t *__restrict __dir, wchar_t *__restrict __file, wchar_t *__restrict __ext);
__LIBC errno_t (__LIBCCALL _wmakepath_s)(wchar_t *__restrict __dst, size_t __maxlen, wchar_t const *__restrict __drive, wchar_t const *__restrict __dir, wchar_t const *__restrict __file, wchar_t const *__restrict __ext);
__LIBC errno_t (__LIBCCALL _wputenv_s)(wchar_t const *__restrict __name, wchar_t const *__restrict __val);
__LIBC errno_t (__LIBCCALL _wsearchenv_s)(wchar_t const *__restrict __file, wchar_t const *__restrict __varname, wchar_t * __restrict __dst, size_t __maxlen);
__LIBC errno_t (__LIBCCALL _wsplitpath_s)(wchar_t const *__restrict __abspath, wchar_t *__restrict __drive, size_t __drivelen, wchar_t *__restrict __dir, size_t __dirlen, wchar_t *__restrict __file, size_t __filelen, wchar_t *__restrict __ext, size_t __extlen);
#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED 1
__LIBC void (__LIBCCALL _wperror)(wchar_t const *__restrict __errmsg) __KOS_ASMNAME("wperror");
#endif /* !_CRT_WPERROR_DEFINED */
#endif /* !_WSTDLIBP_DEFINED */

__LIBC void (__LIBCCALL _seterrormode)(int __mode);
__LIBC void (__LIBCCALL _beep)(unsigned int __freq, unsigned int __duration);
#if __SIZEOF_INT__ == 4
__LIBC void (__LIBCCALL _sleep)(__UINT32_TYPE__ __duration) __KOS_ASMNAME("sleep");
#else
__LIBC void (__LIBCCALL _sleep)(__UINT32_TYPE__ __duration);
#endif

#ifndef __cplusplus
#define min(a,b)   __min(a,b)
#define max(a,b)   __max(a,b)
#endif /* !__cplusplus */

#define sys_errlist _sys_errlist
#define sys_nerr    _sys_nerr
#ifndef __environ_defined
#define __environ_defined 1
#define environ     _environ
#endif /* !__environ_defined */


__LIBC char *(__LIBCCALL itoa)(int __val, char *__buf, int __radix) __ASMNAME("_itoa");
#if __SIZEOF_LONG__ == 8
__LIBC char *(__LIBCCALL ltoa)(long __val, char *__buf, int __radix) __ASMNAME("_i64toa");
__LIBC char *(__LIBCCALL ultoa)(unsigned long __val, char *__buf, int __radix) __ASMNAME("_ui64toa");
#elif __SIZEOF_LONG__ == __SIZEOF_INT__
__LIBC char *(__LIBCCALL ltoa)(long __val, char *__buf, int __radix) __ASMNAME("_itoa");
__LIBC char *(__LIBCCALL ultoa)(unsigned long __val, char *__buf, int __radix) __ASMNAME("_ultoa");
#else /* ... */
__LIBC char *(__LIBCCALL ltoa)(long __val, char *__buf, int __radix) __ASMNAME("_ltoa");
__LIBC char *(__LIBCCALL ultoa)(unsigned long __val, char *__buf, int __radix) __ASMNAME("_ultoa");
#endif /* !... */
#ifndef __swab_defined
#define __swab_defined 1
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__ && !defined(__PE__)
__LIBC __NONNULL((1,2)) void (__LIBCCALL swab)(void const *__restrict __from, void *__restrict __to, int __n_bytes);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__LIBC __NONNULL((1,2)) void (__LIBCCALL swab)(void const *__restrict __from, void *__restrict __to, int __n_bytes) __ASMNAME("_swab");
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !__swab_defined */

__LIBC onexit_t (__LIBCCALL _onexit)(onexit_t __func);
__LIBC onexit_t (__LIBCCALL onexit)(onexit_t __func) __ASMNAME("_onexit");
#endif /* __USE_DOS */

#endif /* !__KERNEL__ */

__DECL_END

#ifdef __USE_KOS
#ifdef _ALLOCA_H
#include "__amalloc.h"
#define amalloc(s) __amalloc(s)
#define acalloc(s) __acalloc(s)
#define afree(p)   __afree(p)
#endif
#endif /* __USE_KOS */

#endif /* !_STDLIB_H */
