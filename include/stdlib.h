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

#if defined(__USE_DEBUG) && __USE_DEBUG != 0 && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
#include <hybrid/debuginfo.h>
#endif

__SYSDECL_BEGIN

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

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif

#ifndef NULL
#define NULL __NULLPTR
#endif

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#ifndef __WAIT_MACROS_DEFINED
#define __WAIT_MACROS_DEFINED 1
#include <bits/waitflags.h>
#include <bits/waitstatus.h>

#ifdef __USE_MISC
#if defined(__GNUC__) && !defined(__cplusplus)
#   define __WAIT_INT(status) (__extension__(((union{ __typeof__(status) __in; int __i; }) { .__in = (status) }).__i))
#else
#   define __WAIT_INT(status) (*(int *)&(status))
#endif
#ifdef __NO_ATTR_TRANSPARENT_UNION
#   define __WAIT_STATUS      void *
#   define __WAIT_STATUS_DEFN void *
#else
typedef union {
    union wait *__uptr;
    int        *__iptr;
} __WAIT_STATUS __ATTR_TRANSPARENT_UNION;
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
#if defined(__CORRECT_ISO_CPP_STDLIB_H_PROTO) && 0
extern "C++" {
__LOCAL __ATTR_CONST __WUNUSED int __NOTHROW((__LIBCCALL abs)(int __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED long __NOTHROW((__LIBCCALL abs)(long __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED __LONGLONG __NOTHROW((__LIBCCALL abs)(__LONGLONG __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED div_t __NOTHROW((__LIBCCALL div)(int __numer, int __denom)) { div_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
__LOCAL __ATTR_CONST __WUNUSED ldiv_t __NOTHROW((__LIBCCALL div)(long __numer, long __denom)) { ldiv_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
__LOCAL __ATTR_CONST __WUNUSED lldiv_t __NOTHROW((__LIBCCALL div)(__LONGLONG __numer, __LONGLONG __denom)) { lldiv_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
}
#else /* __CORRECT_ISO_CPP_STDLIB_H_PROTO */
__LOCAL __ATTR_CONST __WUNUSED int __NOTHROW((__LIBCCALL abs)(int __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED div_t __NOTHROW((__LIBCCALL div)(int __numer, int __denom)) { div_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
#endif /* !__CORRECT_ISO_CPP_STDLIB_H_PROTO */
__LOCAL __ATTR_CONST __WUNUSED long __NOTHROW((__LIBCCALL labs)(long __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED ldiv_t __NOTHROW((__LIBCCALL ldiv)(long __numer, long __denom)) { ldiv_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
#ifdef __USE_ISOC99
__LOCAL __ATTR_CONST __WUNUSED __LONGLONG __NOTHROW((__LIBCCALL llabs)(__LONGLONG __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED lldiv_t __NOTHROW((__LIBCCALL lldiv)(__LONGLONG __numer, __LONGLONG __denom)) { lldiv_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
#endif /* __USE_ISOC99 */
#else /* __KERNEL__ */
#if defined(__CORRECT_ISO_CPP_STDLIB_H_PROTO) && 0
extern "C++" {
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST __WUNUSED,int,__LIBCCALL,abs,(int __x),abs,(__x))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST __WUNUSED,long,__LIBCCALL,abs,(long __x),labs,(__x))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST __WUNUSED,__LONGLONG,__LIBCCALL,abs,(__LONGLONG __x),llabs,(__x))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST __WUNUSED,div_t,__LIBCCALL,div,(int __numer, int __denom),div,(__numer,__denom))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST __WUNUSED,ldiv_t,__LIBCCALL,div,(long __numer, long __denom),ldiv,(__numer,__denom))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST __WUNUSED,lldiv_t,__LIBCCALL,div,(__LONGLONG __numer, __LONGLONG __denom),lldiv,(__numer,__denom))
}
#else /* __CORRECT_ISO_CPP_STDLIB_H_PROTO */
__LIBC __ATTR_CONST __WUNUSED int __NOTHROW((__LIBCCALL abs)(int __x));
__LIBC __ATTR_CONST __WUNUSED div_t __NOTHROW((__LIBCCALL div)(int __numer, int __denom));
#endif /* !__CORRECT_ISO_CPP_STDLIB_H_PROTO */
__LIBC __ATTR_CONST __WUNUSED long __NOTHROW((__LIBCCALL labs)(long __x));
__LIBC __ATTR_CONST __WUNUSED ldiv_t __NOTHROW((__LIBCCALL ldiv)(long __numer, long __denom));
#ifdef __USE_ISOC99
__LIBC __ATTR_CONST __WUNUSED __LONGLONG __NOTHROW((__LIBCCALL llabs)(__LONGLONG __x));
__LIBC __ATTR_CONST __WUNUSED lldiv_t __NOTHROW((__LIBCCALL lldiv)(__LONGLONG __numer, __LONGLONG __denom));
#endif /* __USE_ISOC99 */
#ifdef _MSC_VER
#pragma intrinsic(abs)
#pragma intrinsic(labs)
#endif
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
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
#endif /* !__CXX_SYSTEM_HEADER */

#if defined(__USE_MISC) || defined(__USE_DOS) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8))
__REDIRECT_IFDOS(__LIBC,__WUNUSED __NONNULL((3)),char *,__LIBCCALL,gcvt,(double __val, int __ndigit, char *__buf),_gcvt,(__val,__ndigit,__buf))
#endif
#ifdef __USE_MISC
__REDIRECT_IFDOS(__LIBC,__WUNUSED __NONNULL((3)),char *,__LIBCCALL,qgcvt,(long double __val, int __ndigit, char *__buf),_gcvt,(__val,__ndigit,__buf))
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_ecvt_s,(char *__buf, size_t __buflen, double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),_ecvt_s,(__buf,__buflen,__val,__ndigit,__decptr,__sign))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_fcvt_s,(char *__buf, size_t __buflen, double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),_fcvt_s,(__buf,__buflen,__val,__ndigit,__decptr,__sign))
__LOCAL __NONNULL((3,4,5)) int (__LIBCCALL ecvt_r)(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len) { return __dos_ecvt_s(__buf,__len,__val,__ndigit,__decptr,__sign); }
__LOCAL __NONNULL((3,4,5)) int (__LIBCCALL fcvt_r)(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len) { return __dos_fcvt_s(__buf,__len,__val,__ndigit,__decptr,__sign); }
__LOCAL __NONNULL((3,4,5)) int (__LIBCCALL qecvt_r)(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len) { return __dos_ecvt_s(__buf,__len,(double)__val,__ndigit,__decptr,__sign); }
__LOCAL __NONNULL((3,4,5)) int (__LIBCCALL qfcvt_r)(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len) { return __dos_fcvt_s(__buf,__len,(double)__val,__ndigit,__decptr,__sign); }
#else /* __DOS_COMPAT__ */
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL ecvt_r)(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL fcvt_r)(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL qecvt_r)(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL qfcvt_r)(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len);
#endif /* !__DOS_COMPAT__ */
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,strtoq,(char const *__restrict __nptr, char **__restrict __endptr, int __base),strtoll,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,strtouq,(char const *__restrict __nptr, char **__restrict __endptr, int __base),strtoull,(__nptr,__endptr,__base))
#endif /* __USE_MISC */

__NAMESPACE_STD_BEGIN
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) double (__LIBCCALL atof)(char const *__restrict __nptr);
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) int (__LIBCCALL atoi)(char const *__restrict __nptr);
#if __SIZEOF_LONG__ == __SIZEOF_INT__
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1)),long,__LIBCCALL,atol,(char const *__restrict __nptr),atoi,(__nptr))
#else /* __SIZEOF_LONG__ == __SIZEOF_INT__ */
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) long (__LIBCCALL atol)(char const *__restrict __nptr);
#endif /* __SIZEOF_LONG__ != __SIZEOF_INT__ */
__LIBC __NONNULL((1)) double (__LIBCCALL strtod)(char const *__restrict __nptr, char **__restrict __endptr);
__LIBC __NONNULL((1)) long (__LIBCCALL strtol)(char const *__restrict __nptr, char **__restrict __endptr, int __base);
__LIBC __NONNULL((1)) unsigned long (__LIBCCALL strtoul)(char const *__restrict __nptr, char **__restrict __endptr, int __base);
#ifdef __USE_ISOC99
__LIBC __NONNULL((1)) float (__LIBCCALL strtof)(char const *__restrict __nptr, char **__restrict __endptr);
__LIBC __NONNULL((1)) long double (__LIBCCALL strtold)(char const *__restrict __nptr, char **__restrict __endptr);
#if __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1)),__LONGLONG,__LIBCCALL,atoll,(char const *__restrict __nptr),atol,(__nptr))
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,strtoll,(char const *__restrict __nptr, char **__restrict __endptr, int __base),strtol,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,strtoull,(char const *__restrict __nptr, char **__restrict __endptr, int __base),strtoul,(__nptr,__endptr,__base))
#else /* __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__ */
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) __LONGLONG (__LIBCCALL atoll)(char const *__restrict __nptr);
__LIBC __NONNULL((1)) __LONGLONG (__LIBCCALL strtoll)(char const *__restrict __nptr, char **__restrict __endptr, int __base);
__LIBC __NONNULL((1)) __ULONGLONG (__LIBCCALL strtoull)(char const *__restrict __nptr, char **__restrict __endptr, int __base);
#endif /* __SIZEOF_LONG_LONG__ != __SIZEOF_LONG__ */
#endif /* __USE_ISOC99 */
#ifdef __KERNEL__
__LIBC __UINT32_TYPE__ (__LIBCCALL rand)(void);
__LIBC void (__LIBCCALL srand)(__UINT32_TYPE__ __seed);
#else /* __KERNEL__ */
__LIBC int (__LIBCCALL rand)(void);
__LIBC void (__LIBCCALL srand)(long __seed);
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
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
#endif /* !__NAMESPACE_STD_USING */

#ifdef __USE_POSIX
#ifdef __CRT_GLC
#ifdef __KERNEL__
__LIBC __PORT_NODOS_ALT(rand) __NONNULL((1)) __UINT32_TYPE__ (__LIBCCALL rand_r)(__UINT32_TYPE__ *__restrict __seed);
#else /* __KERNEL__ */
__LIBC __PORT_NODOS_ALT(rand) __NONNULL((1)) int (__LIBCCALL rand_r)(unsigned int *__restrict __seed);
#endif /* !__KERNEL__ */
#endif /* __CRT_GLC */
#endif /* __USE_POSIX */

#ifndef __std_malloc_calloc_defined
#define __std_malloc_calloc_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL malloc)(size_t __n_bytes);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC void *(__LIBCCALL calloc)(size_t __count, size_t __n_bytes);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL realloc)(void *__restrict __mallptr, size_t __n_bytes);
#ifdef __KERNEL__
__REDIRECT_VOID(__LIBC,__SAFE,__LIBCCALL,free,(void *__restrict __mallptr),kfree,(__mallptr))
#else
__LIBC __SAFE void (__LIBCCALL free)(void *__restrict __mallptr);
#endif
__NAMESPACE_STD_END
#endif /* !__std_malloc_calloc_defined */

#ifndef __CXX_SYSTEM_HEADER
#ifndef __malloc_calloc_defined
#define __malloc_calloc_defined 1
__NAMESPACE_STD_USING(malloc)
__NAMESPACE_STD_USING(calloc)
__NAMESPACE_STD_USING(realloc)
__NAMESPACE_STD_USING(free)
#endif /* !__malloc_calloc_defined */
#endif /* !__CXX_SYSTEM_HEADER */


#ifdef __USE_MISC
#ifndef __cfree_defined
#define __cfree_defined 1
#ifdef __KERNEL__
__REDIRECT_VOID(__LIBC,__SAFE,__LIBCCALL,cfree,(void *__restrict __mallptr),kfree,(__mallptr))
#else /* __KERNEL__ */
__REDIRECT_VOID(__LIBC,__SAFE,__LIBCCALL,cfree,(void *__restrict __mallptr),free,(__mallptr))
#endif /* !__KERNEL__ */
#endif /* !__cfree_defined */
#ifdef __CRT_GLC
__LIBC __PORT_NODOS int (__LIBCCALL getloadavg)(double __loadavg[], int __nelem);
#endif /* __CRT_GLC */
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
 *    >> the `dlerror()' function you can find in libdl.so is literally just a swapper
 *       around `strerror()' with the error number internally saved by libdl, so as not
 *       to clobber libc's thread-local `errno' variable. */
#ifdef __CRT_KOS
__LIBC __PORT_KOSONLY_ALT(dlopen) void *(__LIBCCALL xdlopen)(char const *__filename, int __flags);
__LIBC __PORT_KOSONLY_ALT(dlopen) void *(__LIBCCALL xfdlopen)(int __fd, int __flags);
__LIBC __PORT_KOSONLY_ALT(dlsym) void *(__LIBCCALL xdlsym)(void *__handle, char const *__symbol);
__LIBC __PORT_KOSONLY_ALT(dlclose) int (__LIBCCALL xdlclose)(void *__handle);
#endif /* __CRT_KOS */
#endif /* __USE_KOS */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K))
#ifdef __CRT_GLC
#ifndef __valloc_defined
#define __valloc_defined 1
__LIBC __PORT_NODOS __SAFE __WUNUSED __MALL_ATTR_PAGEALIGNED
__ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL valloc)(size_t __n_bytes);
#endif /* !__valloc_defined */
#endif /* __CRT_GLC */
#endif

#ifdef __USE_XOPEN2K
#ifdef __CRT_GLC
#ifndef __posix_memalign_defined
#define __posix_memalign_defined 1
__LIBC __PORT_NODOS __SAFE __NONNULL((1)) int (__LIBCCALL posix_memalign)(void **__restrict __pp, size_t __alignment, size_t __n_bytes);
#endif /* !__posix_memalign_defined */
#endif /* __CRT_GLC */
#endif /* __USE_XOPEN2K */

#ifdef __USE_ISOC11
#ifdef __CRT_GLC /* XXX: DOS may add this one some time in the future. */
__REDIRECT(__LIBC,__SAFE __PORT_NODOS __WUNUSED __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC,
           void *,__LIBCCALL,aligned_alloc,(size_t __alignment, size_t __n_bytes),memalign,(__alignment,__n_bytes))
#endif /* __CRT_GLC */
#endif /* __USE_ISOC11 */


/* Debug hooks for malloc() and friends. */
#ifdef __USE_DEBUG
#if __USE_DEBUG != 0 && defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
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
 * >> void *p = std::malloc(42); // Expands to `std::_malloc_d(42,...)'
 */
__NAMESPACE_STD_BEGIN
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _malloc_d)(size_t __n_bytes, __DEBUGINFO);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC void *(__LIBCCALL _calloc_d)(size_t __count, size_t __n_bytes, __DEBUGINFO);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL _realloc_d)(void *__restrict __mallptr, size_t __n_bytes, __DEBUGINFO);
#ifdef __KERNEL__
__REDIRECT_VOID(__LIBC,__SAFE,__LIBCCALL,_free_d,(void *__restrict __mallptr, __DEBUGINFO),_kfree_d,(__mallptr,__DEBUGINFO_FWD))
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

#ifdef __USE_MISC
#ifndef __cfree_d_defined
#define __cfree_d_defined 1
#ifdef __KERNEL__
__REDIRECT_VOID(__LIBC,__SAFE,__LIBCCALL,_cfree_d,(void *__restrict __mallptr, __DEBUGINFO),_kfree_d,(__mallptr,__DEBUGINFO_FWD))
#else /* __KERNEL__ */
__REDIRECT_VOID(__LIBC,__SAFE,__LIBCCALL,_cfree_d,(void *__restrict __mallptr, __DEBUGINFO),_free_d,(__mallptr,__DEBUGINFO_FWD))
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
__REDIRECT(__LIBC,__SAFE __WUNUSED __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC,void *,__LIBCCALL,
           _aligned_alloc_d,(size_t __alignment, size_t __n_bytes, __DEBUGINFO),_memalign_d,(__alignment,__n_bytes,__DEBUGINFO_FWD))
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
#ifdef __CRT_GLC
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
#endif /* __CRT_GLC */
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
__REDIRECT(__LIBC,,long,__LIBCCALL,random,(void),rand,())
#endif /* __SIZEOF_LONG__ == 4 */
#if __SIZEOF_INT__ == 4
__REDIRECT_VOID(__LIBC,,__LIBCCALL,srandom,(unsigned int __seed),srand,(__seed))
#endif /* __SIZEOF_INT__ == 4 */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED  */

#else /* __KERNEL__ */

#define MB_CUR_MAX                 (__ctype_get_mb_cur_max())
__LIBC __WUNUSED size_t (__LIBCCALL __ctype_get_mb_cur_max)(void);

__NAMESPACE_STD_BEGIN
__REDIRECT_UFS(__LIBC,__WUNUSED __NONNULL((1)),char *,__LIBCCALL,getenv,(char const *__name),getenv,(__name))
__LIBC int (__LIBCCALL mblen)(char const *__s, size_t __n);
__LIBC int (__LIBCCALL mbtowc)(wchar_t *__restrict __pwc, char const *__restrict __s, size_t __n);
__LIBC int (__LIBCCALL wctomb)(char *__s, wchar_t __wchar);
__LIBC size_t (__LIBCCALL mbstowcs)(wchar_t *__restrict __pwcs, char const *__restrict __s, size_t __n);
__LIBC size_t (__LIBCCALL wcstombs)(char *__restrict __s, wchar_t const *__restrict __pwcs, size_t __n);
#ifndef __std_system_defined
#define __std_system_defined 1
__LIBC int (__LIBCCALL system)(char const *__command);
#endif /* !__std_system_defined */
#ifndef __std_abort_defined
#define __std_abort_defined 1
__LIBC __ATTR_NORETURN void (__LIBCCALL abort)(void);
#endif /* !__std_abort_defined */
#ifndef __std_exit_defined
#define __std_exit_defined 1
__LIBC __ATTR_NORETURN void (__LIBCCALL exit)(int __status);
#endif /* !__std_exit_defined */
__LIBC int (__LIBCCALL atexit)(void (*__LIBCCALL __func)(void));
#if defined(__USE_ISOC11) || defined(__USE_ISOCXX11)
#ifdef __DOS_COMPAT__
__REDIRECT_VOID(__LIBC,__ATTR_NORETURN,__LIBCCALL,quick_exit,(int __status),exit,(__status))
#ifdef __cplusplus
extern "C++" { __REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,at_quick_exit,(void (*__LIBCCALL __func)(void)),at_quick_exit,(__func)) }
#else /* __cplusplus */
__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,at_quick_exit,(void (*__LIBCCALL __func)(void)),atexit,(__func))
#endif /* !__cplusplus */
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_NORETURN void (__LIBCCALL quick_exit)(int __status);
#ifdef __cplusplus
extern "C++" { __REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,at_quick_exit,(void (*__LIBCCALL __func)(void)),at_quick_exit,(__func)) }
#else /* __cplusplus */
__LIBC __NONNULL((1)) int (__LIBCCALL at_quick_exit)(void (*__LIBCCALL __func)(void));
#endif /* !__cplusplus */
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_ISOC11 || __USE_ISOCXX11 */
#ifdef __USE_ISOC99
__REDIRECT_VOID(__LIBC,__ATTR_NORETURN,__LIBCCALL,_Exit,(int __status),_exit,(__status))
#endif /* __USE_ISOC99 */
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
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
#endif /* !__CXX_SYSTEM_HEADER */

#if defined(__USE_MISC) || defined(__USE_XOPEN)
#ifdef __CRT_GLC
__LIBC __PORT_NODOS double (__LIBCCALL drand48)(void);
__LIBC __PORT_NODOS long (__LIBCCALL lrand48)(void);
__LIBC __PORT_NODOS long (__LIBCCALL mrand48)(void);
__LIBC __PORT_NODOS __NONNULL((1)) double (__LIBCCALL erand48)(unsigned short __xsubi[3]);
__LIBC __PORT_NODOS __NONNULL((1)) long (__LIBCCALL nrand48)(unsigned short __xsubi[3]);
__LIBC __PORT_NODOS __NONNULL((1)) long (__LIBCCALL jrand48)(unsigned short __xsubi[3]);
__LIBC __PORT_NODOS void (__LIBCCALL srand48)(long __seedval);
__LIBC __PORT_NODOS __NONNULL((1)) unsigned short *(__LIBCCALL seed48)(unsigned short __seed16v[3]);
__LIBC __PORT_NODOS __NONNULL((1)) void (__LIBCCALL lcong48)(unsigned short __param[7]);
#endif /* __CRT_GLC */
#endif /* __USE_MISC || __USE_XOPEN */

#if defined(__USE_MISC) || defined(__USE_XOPEN) || defined(__USE_DOS)
__REDIRECT_IFDOS(__LIBC,__NONNULL((1)),int,__LIBCCALL,putenv,(char *__string),_putenv,(__string))
#endif /* __USE_MISC || __USE_XOPEN || __USE_DOS */

#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
#if (defined(__CRT_KOS) || !defined(__CRT_GLC))
#if __SIZEOF_LONG__ == __SIZEOF_INT__
__REDIRECT(__LIBC,,long,__LIBCCALL,random,(void),rand,())
__REDIRECT_VOID(__LIBC,,__LIBCCALL,srandom,(unsigned int __seed),srand,(__seed))
#else /* __SIZEOF_LONG__ == __SIZEOF_INT__ */
__LOCAL long (__LIBCCALL random)(void) { return (long)rand(); }
__LOCAL void (__LIBCCALL srandom)(unsigned int __seed) { srand(__seed); }
#endif /* __SIZEOF_LONG__ != __SIZEOF_INT__ */
#else /* __CRT_KOS || !__CRT_GLC */
__LIBC long (__LIBCCALL random)(void);
__LIBC void (__LIBCCALL srandom)(unsigned int __seed);
#endif /* !__CRT_KOS && __CRT_GLC */
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __NONNULL((2)) char *(__LIBCCALL initstate)(unsigned int __seed, char *__statebuf, size_t __statelen);
__LIBC __PORT_NODOS __NONNULL((1)) char *(__LIBCCALL setstate)(char *__statebuf);
__LIBC __PORT_NODOS __WUNUSED char *(__LIBCCALL l64a)(long __n);
__LIBC __PORT_NODOS __WUNUSED __ATTR_PURE __NONNULL((1)) long (__LIBCCALL a64l)(char const *__s);
__LIBC __PORT_NODOS __WUNUSED char *(__LIBCCALL realpath)(char const *__restrict __name, char *__restrict __resolved);
#endif /* __CRT_GLC */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED  */

#ifdef __USE_MISC
#ifdef __CRT_GLC
struct drand48_data {
    unsigned short __x[3];
    unsigned short __old_x[3];
    unsigned short __c;
    unsigned short __init;
    __ULONGLONG    __a;
};

__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL drand48_r)(struct drand48_data *__restrict __buffer, double *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL erand48_r)(unsigned short __xsubi[3], struct drand48_data *__restrict __buffer, double *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL lrand48_r)(struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL nrand48_r)(unsigned short __xsubi[3], struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL mrand48_r)(struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL jrand48_r)(unsigned short __xsubi[3], struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((2))   int (__LIBCCALL srand48_r)(long __seedval, struct drand48_data *__buffer);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL seed48_r)(unsigned short __seed16v[3], struct drand48_data *__buffer);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL lcong48_r)(unsigned short __param[7], struct drand48_data *__buffer);

struct random_data {
    __INT32_TYPE__ *fptr;
    __INT32_TYPE__ *rptr;
    __INT32_TYPE__ *state;
    int             rand_type;
    int             rand_deg;
    int             rand_sep;
    __INT32_TYPE__ *end_ptr;
};
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL random_r)(struct random_data *__restrict __buf, __INT32_TYPE__ *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((2)) int (__LIBCCALL srandom_r)(unsigned int __seed, struct random_data *__buf);
__LIBC __PORT_NODOS __NONNULL((2,4)) int (__LIBCCALL initstate_r)(unsigned int __seed, char *__restrict __statebuf, size_t __statelen, struct random_data *__restrict __buf);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL setstate_r)(char *__restrict __statebuf, struct random_data *__restrict __buf);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL on_exit)(void (__LIBCCALL *__func)(int __status, void *__arg), void *__arg);
__LIBC __PORT_NODOS int (__LIBCCALL clearenv)(void);
__REDIRECT_FS_FUNC(__LIBC,__PORT_NODOS __WUNUSED __NONNULL((1)),int,__LIBCCALL,mkstemps,(char *__template, int __suffixlen),mkstemps,(__template,__suffixlen))
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) int (__LIBCCALL rpmatch)(char const *__response);
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemps64)(char *__template, int __suffixlen);
#endif /* __USE_LARGEFILE64 */
#endif /* __CRT_GLC */
__REDIRECT_IFDOS(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,qecvt,(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),_ecvt,(__val,__ndigit,__decptr,__sign))
__REDIRECT_IFDOS(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,qfcvt,(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),_fcvt,(__val,__ndigit,__decptr,__sign))
#endif /* __USE_MISC */

#ifdef __USE_GNU
#include <xlocale.h>
#if defined(__CRT_DOS) && __SIZEOF_LONG__ == 4
__REDIRECT(__LIBC,__NONNULL((1,4)),long,__LIBCCALL,strtol_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),_strtol_l,(__nptr,__endptr,__base,__loc))
__REDIRECT(__LIBC,__NONNULL((1,4)),unsigned long,__LIBCCALL,strtoul_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),_strtoul_l,(__nptr,__endptr,__base,__loc))
#elif defined(__CRT_DOS) && __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__NONNULL((1,4)),long,__LIBCCALL,strtol_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),_strtoi64_l,(__nptr,__endptr,__base,__loc))
__REDIRECT(__LIBC,__NONNULL((1,4)),unsigned long,__LIBCCALL,strtoul_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),_strtoui64_l,(__nptr,__endptr,__base,__loc))
#else
__LIBC __NONNULL((1,4)) long (__LIBCCALL strtol_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc);
__LIBC __NONNULL((1,4)) unsigned long (__LIBCCALL strtoul_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc);
#endif
#if defined(__CRT_DOS) && __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,__NONNULL((1,4)),__LONGLONG,__LIBCCALL,strtoll_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),_strtoi64_l,(__nptr,__endptr,__base,__loc))
__REDIRECT(__LIBC,__NONNULL((1,4)),__ULONGLONG,__LIBCCALL,strtoull_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),_strtoui64_l,(__nptr,__endptr,__base,__loc))
#else
__LIBC __NONNULL((1,4)) __LONGLONG (__LIBCCALL strtoll_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc);
__LIBC __NONNULL((1,4)) __ULONGLONG (__LIBCCALL strtoull_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc);
#endif
__REDIRECT_IFDOS(__LIBC,__NONNULL((1,3)),double,__LIBCCALL,strtod_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __loc),_strtod_l,(__nptr,__endptr,__loc))
__REDIRECT_IFDOS(__LIBC,__NONNULL((1,3)),float,__LIBCCALL,strtof_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __loc),_strtof_l,(__nptr,__endptr,__loc))
__REDIRECT_IFDOS(__LIBC,__NONNULL((1,3)),long double,__LIBCCALL,strtold_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __loc),_strtold_l,(__nptr,__endptr,__loc))
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __NONNULL((1)),char *,__LIBCCALL,secure_getenv,(char const *__name),getenv,(__name))
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __NONNULL((1)) char *(__LIBCCALL secure_getenv)(char const *__name);
#endif /* !__DOS_COMPAT__ */

#ifdef __CRT_GLC
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) char *(__LIBCCALL canonicalize_file_name)(char const *__name);
__LIBC __PORT_NODOS __NONNULL((2)) int (__LIBCCALL ptsname_r)(int __fd, char *__buf, size_t __buflen);
__LIBC __PORT_NODOS int (__LIBCCALL getpt)(void);
__REDIRECT_FS_FUNC(__LIBC,__PORT_NODOS __WUNUSED __NONNULL((1)),int,__LIBCCALL,mkostemp,(char *__template, int __flags),mkostemp,(__template,__flags))
__REDIRECT_FS_FUNC(__LIBC,__PORT_NODOS __WUNUSED __NONNULL((1)),int,__LIBCCALL,mkostemps,(char *__template, int __suffixlen, int __flags),mkostemps,(__template,__suffixlen,__flags))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) int (__LIBCCALL mkostemp64)(char *__template, int __flags);
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) int (__LIBCCALL mkostemps64)(char *__template, int __suffixlen, int __flags);
#endif /* __USE_LARGEFILE64 */
#endif /* __CRT_GLC */
#endif /* __USE_GNU */

#ifdef __USE_XOPEN2K
#ifdef __DOS_COMPAT__
__SYSDECL_END
#include <hybrid/malloc.h>
#include <hybrid/string.h>
__SYSDECL_BEGIN
__LOCAL __NONNULL((2)) int (__LIBCCALL setenv)(char const *__name, char const *__val, int __replace);
__LOCAL __NONNULL((1)) int (__LIBCCALL unsetenv)(char const *__name);

__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,__dos_putenv,(char *__string),_putenv,(__string))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_setenv,(char const *__name, char const *__val),_putenv_s,(__name,__val))
__LOCAL __NONNULL((2)) int (__LIBCCALL setenv)(char const *__name, char const *__val, int __replace) {
    if (!__replace && __NAMESPACE_STD_SYM getenv(__name)) return 0;
    return __dos_setenv(__name,__val);
}
__LOCAL __NONNULL((1)) int (__LIBCCALL unsetenv)(char const *__name) {
    char *__copy; size_t __namelen; int __result; if (!__name) return -1;
    __namelen = __hybrid_strlen(__name);
    __copy = (char *)__hybrid_malloc((__namelen+2)*sizeof(char));
    if __unlikely(!__copy) return -1;
    __hybrid_memcpy(__copy,__name,__namelen*sizeof(char));
    __copy[__namelen] = '=';
    __copy[__namelen+1] = '\0';
    __result = __dos_putenv(__copy);
    __hybrid_free(__copy);
    return __result;
}
#else /* __DOS_COMPAT__ */
__LIBC __NONNULL((2)) int (__LIBCCALL setenv)(char const *__name, char const *__val, int __replace);
__LIBC __NONNULL((1)) int (__LIBCCALL unsetenv)(char const *__name);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_XOPEN2K */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8))
#ifndef __mktemp_defined
#define __mktemp_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,__NONNULL((1)),char *,__LIBCCALL,mktemp,(char *__template),mktemp,(__template))
#endif /* !__mktemp_defined */
#endif

#if defined(__USE_MISC) || defined(__USE_DOS) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8))
__REDIRECT_IFDOS(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,ecvt,(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),_ecvt,(__val,__ndigit,__decptr,__sign))
__REDIRECT_IFDOS(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,fcvt,(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),_fcvt,(__val,__ndigit,__decptr,__sign))
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1,2,3)) int (__LIBCCALL getsubopt)(char **__restrict __optionp, char *const *__restrict __tokens, char **__restrict __valuep);
#endif /* __CRT_GLC */

#ifdef __DOS_COMPAT__
#ifndef __mktemp_defined
__REDIRECT(__LIBC,__NONNULL((1)),char *,__LIBCCALL,__dos_mktemp,(char *__template),_mktemp,(__template))
#else
#define __dos_mktemp(template) mktemp(template)
#endif /* !__mktemp_defined */
__LOCAL __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemp)(char *__template) { return __dos_mktemp(__template) ? 0 : -1; }
#ifdef __USE_LARGEFILE64
__LOCAL __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemp64)(char *__template) { return __dos_mktemp(__template) ? 0 : -1; }
#endif /* __USE_LARGEFILE64 */
#else /* __DOS_COMPAT__ */
__REDIRECT_FS_FUNC(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,mkstemp,(char *__template),mkstemp,(__template))
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemp64)(char *__template);
#endif /* __USE_LARGEFILE64 */
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */

#ifdef __USE_XOPEN2K8
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __NONNULL((1)),char *,__LIBCCALL,mkdtemp,(char *__template),_mktemp,(__template))
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __NONNULL((1)) char *(__LIBCCALL mkdtemp)(char *__template);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_XOPEN2K8 */

#ifdef __CRT_GLC
#ifdef __USE_XOPEN
__LIBC __PORT_NODOS __NONNULL((1)) void (__LIBCCALL setkey)(char const *__key);
__LIBC __PORT_NODOS int (__LIBCCALL grantpt)(int __fd);
__LIBC __PORT_NODOS int (__LIBCCALL unlockpt)(int __fd);
__LIBC __PORT_NODOS __WUNUSED char *(__LIBCCALL ptsname)(int __fd);
#endif /* __USE_XOPEN */

#ifdef __USE_XOPEN2KXSI
__LIBC __PORT_NODOS __WUNUSED int (__LIBCCALL posix_openpt)(int __oflag);
#endif /* __USE_XOPEN2KXSI */
#endif /* __CRT_GLC */

#define __DOS_MAX_PATH         260
#define __DOS_MAX_DRIVE        3
#define __DOS_MAX_DIR          256
#define __DOS_MAX_FNAME        256
#define __DOS_MAX_EXT          256
#define __DOS_OUT_TO_DEFAULT   0
#define __DOS_OUT_TO_STDERR    1
#define __DOS_OUT_TO_MSGBOX    2
#define __DOS_REPORT_ERRMODE   3
#define __DOS_WRITE_ABORT_MSG  0x1
#define __DOS_CALL_REPORTFAULT 0x2
#define __DOS_MAX_ENV          0x7fff

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

#define _MAX_PATH         __DOS_MAX_PATH
#define _MAX_DRIVE        __DOS_MAX_DRIVE
#define _MAX_DIR          __DOS_MAX_DIR
#define _MAX_FNAME        __DOS_MAX_FNAME
#define _MAX_EXT          __DOS_MAX_EXT
#define _OUT_TO_DEFAULT   __DOS_OUT_TO_DEFAULT
#define _OUT_TO_STDERR    __DOS_OUT_TO_STDERR
#define _OUT_TO_MSGBOX    __DOS_OUT_TO_MSGBOX
#define _REPORT_ERRMODE   __DOS_REPORT_ERRMODE
#define _WRITE_ABORT_MSG  __DOS_WRITE_ABORT_MSG
#define _CALL_REPORTFAULT __DOS_CALL_REPORTFAULT
#define _MAX_ENV          __DOS_MAX_ENV

#ifndef _CRT_ERRNO_DEFINED
#define _CRT_ERRNO_DEFINED 1
#define errno                                            (*__errno())
__REDIRECT_DOS_FUNC_NOTHROW_(__LIBC,,errno_t *,__LIBCCALL,__errno,(void),_errno,())
__REDIRECT_DOS_FUNC_NOTHROW_(__LIBC,,errno_t,__LIBCCALL,__get_errno,(void),_get_errno,())
__REDIRECT_DOS_FUNC_NOTHROW_(__LIBC,,errno_t,__LIBCCALL,__set_errno,(errno_t __err),_set_errno,(__err))
#endif /* !_CRT_ERRNO_DEFINED */

#define _doserrno     (*__doserrno())
__LIBC __UINT32_TYPE__ *__NOTHROW((__LIBCCALL __doserrno)(void));
__LIBC errno_t __NOTHROW((__LIBCCALL _get_doserrno)(__UINT32_TYPE__ *__perr));
__LIBC errno_t __NOTHROW((__LIBCCALL _set_doserrno)(__UINT32_TYPE__ __err));

__LIBC char **__NOTHROW((__LIBCCALL __sys_errlist)(void));
__LIBC int *__NOTHROW((__LIBCCALL __sys_nerr)(void));
#define _sys_errlist (__sys_errlist())
#define _sys_nerr    (*__sys_nerr())

#undef _environ
#ifdef __DOS_COMPAT__
__LIBC char ***__NOTHROW((__LIBCCALL __p__environ)(void));
#define _environ  (*__p__environ())
#else /* __DOS_COMPAT__ */
#ifndef ____environ_defined
#define ____environ_defined 1
#ifndef __NO_ASMNAME
#undef __environ
__LIBC char **__environ __ASMNAME("environ");
#else /* __NO_ASMNAME */
#undef environ
__LIBC char **environ;
#define __environ environ
#endif /* !__NO_ASMNAME */
#endif /* !____environ_defined */
#define _environ  __environ
#endif /* !__DOS_COMPAT__ */

__LIBC __PORT_DOSONLY int *__NOTHROW((__LIBCCALL __p___argc)(void));
__LIBC __PORT_DOSONLY char ***__NOTHROW((__LIBCCALL __p___argv)(void));
__LIBC __PORT_DOSONLY char **__NOTHROW((__LIBCCALL __p__pgmptr)(void));
__REDIRECT_IFKOS_NOTHROW(__LIBC,__PORT_DOSONLY,wchar_t ***,__LIBCCALL,__p___wargv,(void),wgetargv,())
__REDIRECT_IFKOS_NOTHROW(__LIBC,__PORT_DOSONLY,wchar_t ***,__LIBCCALL,__p__wenviron,(void),wgetenviron,())
__REDIRECT_IFKOS_NOTHROW(__LIBC,__PORT_DOSONLY,wchar_t **,__LIBCCALL,__p__wpgmptr,(void),wgetpgmptr,())
#define __argc    (*__p___argc())
#define __argv    (*__p___argv())
#define __wargv   (*__p___wargv())
#define _wenviron (*__p__wenviron())
#define _pgmptr   (*__p__pgmptr())
#define _wpgmptr  (*__p__wpgmptr())

#ifdef __USE_KOS
/* Access to the initial environment block. */
__LIBC char ***__NOTHROW((__LIBCCALL __p___initenv)(void));
__REDIRECT_IFKOS_NOTHROW(__LIBC,__PORT_DOSONLY,wchar_t ***,__LIBCCALL,__p___winitenv,(void),wgetinitenv,())
#define _initenv  (*__p___initenv())
#define _winitenv (*__p___winitenv())
#endif /* __USE_KOS */

#if defined(__USE_UTF) && defined(__CRT_KOS)
#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */
__REDIRECT_NOTHROW(__LIBC,__PORT_DOSONLY,char16_t ***,__LIBCCALL,__p___uinitenv,(void),__p___winitenv,())
__REDIRECT_NOTHROW(__LIBC,__PORT_DOSONLY,char32_t ***,__LIBCCALL,__p___Uinitenv,(void),wgetinitenv,())
#define _uinitenv  (*__p___uinitenv())
#define _Uinitenv  (*__p___Uinitenv())
#endif /* __USE_UTF */

#ifndef _countof
#define _countof(a) __COMPILER_LENOF(a)
#endif /* !_countof */

#if __SIZEOF_LONG_LONG__ == 8
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,__INT64_TYPE__,__LIBCCALL,_abs64,(__INT64_TYPE__ __x),llabs,(__x))
#elif __SIZEOF_LONG__ == 8
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,__INT64_TYPE__,__LIBCCALL,_abs64,(__INT64_TYPE__ __x),labs,(__x))
#elif __SIZEOF_INTMAX_T__ == 8
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,__INT64_TYPE__,__LIBCCALL,_abs64,(__INT64_TYPE__ __x),imaxabs,(__x))
#else
__LIBC __ATTR_CONST __INT64_TYPE__ (__LIBCCALL _abs64)(__INT64_TYPE__ __x);
#endif

__REDIRECT2(__LIBC,__NONNULL((1,4)),long,__LIBCCALL,__libc_strtol_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),strtol_l,_strtol_l,(__nptr,__endptr,__base,__loc))
__REDIRECT2(__LIBC,__NONNULL((1,4)),__LONGLONG,__LIBCCALL,__libc_strtoll_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),strtoll_l,_strtoi64_l,(__nptr,__endptr,__base,__loc))
#ifdef __DOS_COMPAT__
__REDIRECT2(__LIBC,__NONNULL((1,3)),double,__LIBCCALL,__libc_strtod_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __loc),strtod_l,_strtod_l,(__nptr,__endptr,__loc))
__LOCAL double (__LIBCCALL _atof_l)(char const *__restrict __nptr, __locale_t __locale) { return __libc_strtod_l(__nptr,0,__locale); }
__LOCAL int (__LIBCCALL _atoi_l)(char const *__restrict __nptr, __locale_t __locale) { return (int)__libc_strtol_l(__nptr,0,10,__locale); }
__LOCAL long int (__LIBCCALL _atol_l)(char const *__restrict __nptr, __locale_t __locale) { return __libc_strtol_l(__nptr,0,10,__locale); }
__LOCAL __LONGLONG (__LIBCCALL _atoll_l)(char const *__restrict __nptr, __locale_t __locale) { return __libc_strtoll_l(__nptr,0,10,__locale); }
#else /* __DOS_COMPAT__ */
__LIBC double (__LIBCCALL _atof_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC int (__LIBCCALL _atoi_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC long int (__LIBCCALL _atol_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC __LONGLONG (__LIBCCALL _atoll_l)(char const *__restrict __nptr, __locale_t __locale);
#endif /* !__DOS_COMPAT__ */

#if defined(__CRT_DOS) && (defined(_MSC_VER) || defined(__OPTIMIZE_SIZE__))
#ifndef ___byteswap_ushort_defined
#define ___byteswap_ushort_defined 1
__LIBC __ATTR_CONST __UINT16_TYPE__ __NOTHROW((__LIBCCALL _byteswap_ushort)(__UINT16_TYPE__ x));
#ifdef _MSC_VER
#pragma intrinsic(_byteswap_ushort)
#endif /* _MSC_VER */
#endif /* !___byteswap_ushort_defined */
#ifndef ___byteswap_ulong_defined
#define ___byteswap_ulong_defined 1
__LIBC __ATTR_CONST __UINT32_TYPE__ __NOTHROW((__LIBCCALL _byteswap_ulong)(__UINT32_TYPE__ x));
#ifdef _MSC_VER
#pragma intrinsic(_byteswap_ulong)
#endif /* _MSC_VER */
#endif /* !___byteswap_ulong_defined */
#ifndef ___byteswap_uint64_defined
#define ___byteswap_uint64_defined 1
__LIBC __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _byteswap_uint64)(__UINT64_TYPE__ x));
#ifdef _MSC_VER
#pragma intrinsic(_byteswap_uint64)
#endif /* _MSC_VER */
#endif /* !___byteswap_uint64_defined */
#else /* defined(__CRT_DOS) && defined(__OPTIMIZE_SIZE__) */
__LOCAL __ATTR_CONST __UINT16_TYPE__ __NOTHROW((__LIBCCALL _byteswap_ushort)(__UINT16_TYPE__ __val)) { return __bswap_16(__val); }
__LOCAL __ATTR_CONST __UINT32_TYPE__ __NOTHROW((__LIBCCALL _byteswap_ulong)(__UINT32_TYPE__ __val)) { return __bswap_32(__val); }
__LOCAL __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _byteswap_uint64)(__UINT64_TYPE__ __val)) { return __bswap_64(__val); }
#endif /* __CRT_DOS && __OPTIMIZE_SIZE__ */

#ifdef __USE_DOS_SLIB
#ifndef __rsize_t_defined
#define __rsize_t_defined 1
typedef size_t rsize_t;
#endif /* !__rsize_t_defined */

#ifdef __CRT_DOS
#ifndef _CRT_ALGO_DEFINED
#define _CRT_ALGO_DEFINED 1
/* TODO: The following two could be emulated on linux... */
__LIBC __PORT_DOSONLY __NONNULL((1,2,5)) void *(__LIBCCALL bsearch_s)(void const *__key, void const *__base, size_t __nmemb, size_t __size, int (__LIBCCALL *__compar)(void *__arg, void const *__a, void const *__b), void *__arg);
__LIBC __PORT_DOSONLY __NONNULL((1,4)) void (__LIBCCALL qsort_s)(void *__base, size_t __nmemb, size_t __size, int (__LIBCCALL *__compar)(void *__arg, void const *__a, void const *__b), void *__arg);
#endif  /* _CRT_ALGO_DEFINED */
__LIBC __PORT_DOSONLY __WARN_NOKOSFS errno_t (__LIBCCALL getenv_s)(size_t *__psize, char *__buf, rsize_t __bufsize, char const *__name);
__LIBC __PORT_DOSONLY __WARN_NOKOSFS errno_t (__LIBCCALL _dupenv_s)(char **__restrict __pbuf, size_t *__pbuflen, char const *__name);
#endif /* __CRT_DOS */
#endif /* __USE_DOS_SLIB */

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY char *(__LIBCCALL _itoa)(int __val, char *__dst, int __radix);
__LIBC __PORT_DOSONLY char *(__LIBCCALL _i64toa)(__INT64_TYPE__ __val, char *__dst, int __radix);
__LIBC __PORT_DOSONLY char *(__LIBCCALL _ui64toa)(__UINT64_TYPE__ __val, char *__dst, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _itoa_s)(int __val, char *__dst, size_t __bufsize, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _i64toa_s)(__INT64_TYPE__ __val, char *__dst, size_t __bufsize, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ui64toa_s)(__UINT64_TYPE__ __val, char *__dst, size_t __bufsize, int __radix);
#endif /* __CRT_DOS */

#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_atoi64,(char const *__restrict __nptr),atol,(__nptr))
#ifdef __CRT_DOS
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_atoi64_l,(char const *__restrict __nptr, __locale_t __locale),_atol_l,(__nptr,__locale))
#else /* __CRT_DOS */
__LOCAL __INT64_TYPE__ (__LIBCCALL _atoi64_l)(char const *__restrict __nptr, __locale_t __locale) { return __libc_strtol_l(__nptr,0,__locale); }
#endif /* !__CRT_DOS */
__REDIRECT(__LIBC,,__INT64_TYPE__ ,__LIBCCALL,_strtoi64,(char const *__restrict __nptr, char **__restrict __endptr, int __radix),strtol,(__nptr,__endptr,__radix))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_strtoui64,(char const *__restrict __nptr, char **__restrict __endptr, int __radix),strtoul,(__nptr,__endptr,__radix))
__REDIRECT2(__LIBC,,__INT64_TYPE__ ,__LIBCCALL,_strtoi64_l, (char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),strtol_l,_strtol_l,(__nptr,__endptr,__radix))
__REDIRECT2(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_strtoui64_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),strtoul_l,_strtoul_l,(__nptr,__endptr,__radix))
#elif __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_atoi64,(char const *__restrict __nptr),atoll,(__nptr))
#ifdef __CRT_DOS
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_atoi64_l,(char const *__restrict __nptr, __locale_t __locale),_atoll_l,(__nptr,__locale))
#else /* __CRT_DOS */
__LOCAL __INT64_TYPE__ (__LIBCCALL _atoi64_l)(char const *__restrict __nptr, __locale_t __locale) { return __libc_strtoll_l(__nptr,0,__locale); }
#endif /* !__CRT_DOS */
__REDIRECT(__LIBC,,__INT64_TYPE__ ,__LIBCCALL,_strtoi64,(char const *__restrict __nptr, char **__restrict __endptr, int __radix),strtoll,(__nptr,__endptr,__radix))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_strtoui64,(char const *__restrict __nptr, char **__restrict __endptr, int __radix),strtoull,(__nptr,__endptr,__radix))
__REDIRECT2(__LIBC,,__INT64_TYPE__ ,__LIBCCALL,_strtoi64_l, (char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),strtoll_l,_strtoll_l,(__nptr,__endptr,__radix))
__REDIRECT2(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_strtoui64_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),strtoull_l,_strtoull_l,(__nptr,__endptr,__radix))
#else
__LIBC __INT64_TYPE__ (__LIBCCALL _atoi64)(char const *__restrict __nptr);
__LIBC __INT64_TYPE__ (__LIBCCALL _atoi64_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC __INT64_TYPE__ (__LIBCCALL _strtoi64)(char const *__restrict __nptr, char **__restrict __endptr, int __radix);
__LIBC __INT64_TYPE__ (__LIBCCALL _strtoi64_l)(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale);
__LIBC __UINT64_TYPE__ (__LIBCCALL _strtoui64)(char const *__restrict __nptr, char **__restrict __endptr, int __radix);
__LIBC __UINT64_TYPE__ (__LIBCCALL _strtoui64_l)(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale);
#endif

#ifdef __CRT_DOS
#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,_ltoa,(long __val,  char *__buf, int __radix),_i64toa,(__val,__buf,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ltoa_s,(long __val, char *__buf, size_t __buflen, int __radix),_i64toa_s,(__val,__buf,__buflen,__radix))
#elif __SIZEOF_LONG__ == __SIZEOF_INT__
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,_ltoa,(long __val,  char *__buf, int __radix),_itoa,(__val,__buf,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ltoa_s,(long __val, char *__buf, size_t __buflen, int __radix),_itoa_s,(__val,__buf,__buflen,__radix))
#else /* ... */
__LIBC __PORT_DOSONLY char *(__LIBCCALL _ltoa)(long __val,  char *__buf, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ltoa_s)(long __val, char *__buf, size_t __buflen, int __radix);
#endif /* !... */
__LIBC __PORT_DOSONLY size_t (__LIBCCALL _mbstrlen)(char const *__str);
__LIBC __PORT_DOSONLY size_t (__LIBCCALL _mbstrlen_l)(char const *__str, __locale_t __locale);
__LIBC __PORT_DOSONLY size_t (__LIBCCALL _mbstrnlen)(char const *__str, size_t __maxlen);
__LIBC __PORT_DOSONLY size_t (__LIBCCALL _mbstrnlen_l)(char const *__str, size_t __maxlen, __locale_t __locale);
#endif /* __CRT_DOS */

#ifdef __USE_KOS
__REDIRECT_IFKOS(__LIBC,,size_t,__LIBCCALL,_mblen_l,(char const *__str, size_t __maxlen, __locale_t __locale),mblen_l,(__str,__maxlen,__locale))
__REDIRECT_IFKOS(__LIBC,,size_t,__LIBCCALL,_mbtowc_l,(wchar_t *__dst, char const *__src, size_t __srclen, __locale_t __locale),mbtowc_l,(__str,__maxlen,__locale))
#else /* __USE_KOS */
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_mblen_l,(char const *__str, size_t __maxlen, __locale_t __locale),mblen_l,(__str,__maxlen,__locale))
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_mbtowc_l,(wchar_t *__dst, char const *__src, size_t __srclen, __locale_t __locale),mbtowc_l,(__str,__maxlen,__locale))
#endif /* !__USE_KOS */
__REDIRECT_IFKOS(__LIBC,,size_t,__LIBCCALL,_mbstowcs_l,(wchar_t *__buf, char const *__src, size_t __maxlen, __locale_t __locale),mbstowcs_l,(__buf,__src,__maxlen,__locale))

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL mbstowcs_s)(size_t *__presult, wchar_t *__buf, size_t __buflen, char const *__src, size_t __maxlen);
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_mbstowcs_s_l,(size_t *__presult, wchar_t *__buf, size_t __buflen, char const *__src, size_t __maxlen, __locale_t __locale),mbstowcs_s_l,(__presult,__buf,__buflen,__src,__maxlen,__locale))
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL rand_s)(unsigned int *__restrict __randval);
__LIBC __PORT_DOSONLY int (__LIBCCALL _set_error_mode)(int __mode);
#endif /* __CRT_DOS */

#if defined(__CRT_DOS) && __SIZEOF_LONG__ == 4
__LIBC __NONNULL((1,4)) long (__LIBCCALL _strtol_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc);
__LIBC __NONNULL((1,4)) unsigned long (__LIBCCALL _strtoul_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc);
#elif defined(__CRT_DOS) && __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__NONNULL((1,4)),long,__LIBCCALL,_strtol_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),_strtoi64_l,(__nptr,__endptr,__base,__loc))
__REDIRECT(__LIBC,__NONNULL((1,4)),unsigned long,__LIBCCALL,_strtoul_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),_strtoui64_l,(__nptr,__endptr,__base,__loc))
#else
__REDIRECT_IFKOS(__LIBC,__NONNULL((1,4)),long,__LIBCCALL,_strtol_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),strtol_l,(__nptr,__endptr,__base,__loc))
__REDIRECT_IFKOS(__LIBC,__NONNULL((1,4)),unsigned long,__LIBCCALL,_strtoul_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),strtoul_l,(__nptr,__endptr,__base,__loc))
#endif
#if defined(__CRT_DOS) && __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,__NONNULL((1,4)),__LONGLONG,__LIBCCALL,_strtoll_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),_strtoi64_l,(__nptr,__endptr,__base,__loc))
__REDIRECT(__LIBC,__NONNULL((1,4)),__ULONGLONG,__LIBCCALL,_strtoull_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),_strtoui64_l,(__nptr,__endptr,__base,__loc))
#else
__REDIRECT_IFKOS(__LIBC,__NONNULL((1,4)),__LONGLONG,__LIBCCALL,_strtoll_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),strtoll_l,(__nptr,__endptr,__base,__loc))
__REDIRECT_IFKOS(__LIBC,__NONNULL((1,4)),__ULONGLONG,__LIBCCALL,_strtoull_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __loc),strtoull_l,(__nptr,__endptr,__base,__loc))
#endif
__REDIRECT_IFDOS(__LIBC,__NONNULL((1,3)),double,__LIBCCALL,_strtod_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __loc),strtod_l,(__nptr,__endptr,__loc))
__REDIRECT_IFDOS(__LIBC,__NONNULL((1,3)),float,__LIBCCALL,_strtof_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __loc),strtof_l,(__nptr,__endptr,__loc))
__REDIRECT_IFDOS(__LIBC,__NONNULL((1,3)),long double,__LIBCCALL,_strtold_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __loc),strtold_l,(__nptr,__endptr,__loc))

#ifndef _CRT_SYSTEM_DEFINED
#define _CRT_SYSTEM_DEFINED 1
#endif /* !_CRT_SYSTEM_DEFINED */

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ultoa_s)(unsigned long int __val, char *__buf, size_t __bufsize, int __radix);
__LIBC __PORT_DOSONLY char *(__LIBCCALL _ultoa)(unsigned long int __val, char *__buf, int __radix);
#ifdef __USE_KOS
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,size_t,__LIBCCALL,_wctomb_l,(char *__buf, wchar_t __wc, __locale_t __locale),wctomb_l,(__buf,__wc,__locale))
#else /* __USE_KOS */
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wctomb_l,(char *__buf, wchar_t __wc, __locale_t __locale),wctomb_l,(__buf,__wc,__locale))
#endif /* !__USE_KOS */
#ifdef __USE_DOS_SLIB
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wctomb_s)(int *__presult, char *__buf, rsize_t __buflen, wchar_t __wc);
#endif /* __USE_DOS_SLIB */
__REDIRECT_IFW16(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctomb_s_l,(int *__presult, char *__buf, size_t __buflen, wchar_t __wc, __locale_t __locale),wctomb_s_l,(__presult,__buf,__buflen,__wc,__locale))
__REDIRECT_IFW16(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wcstombs_s_l,(size_t *__presult, char *__buf, size_t __buflen, wchar_t const *__src, size_t __maxlen, __locale_t __locale),wcstombs_s_l,(__presult,__buf,__buflen,__src,__maxlen,__locale))
__REDIRECT_IFW16(__LIBC,__PORT_DOSONLY,size_t,__LIBCCALL,_wcstombs_l,(char *__dst, wchar_t const *__src, size_t __maxlen, __locale_t __locale),wcstombs_l,(__dst,__src,__maxlen,__locale))
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wcstombs_s)(size_t *__presult, char *__buf, size_t __buflen, wchar_t const *__src, size_t __maxlen);

/* DOS malloc extensions. (TODO: With some work, these could be emulated) */
__LIBC __PORT_DOSONLY __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2,3)) void *(__LIBCCALL _recalloc)(void *__mptr, size_t __count, size_t __size);
__LIBC __PORT_DOSONLY __SAFE __WUNUSED __ATTR_ALLOC_ALIGN(2) __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _aligned_malloc)(size_t __size, size_t __align);
__LIBC __PORT_DOSONLY __SAFE __WUNUSED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _aligned_offset_malloc)(size_t __size, size_t __align, size_t __off);
__LIBC __PORT_DOSONLY __SAFE __WUNUSED __ATTR_ALLOC_ALIGN(3) __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL _aligned_realloc)(void *__mptr, size_t __newsize, size_t __align);
__LIBC __PORT_DOSONLY __SAFE __WUNUSED __ATTR_ALLOC_ALIGN(4) __ATTR_ALLOC_SIZE((2,3)) void *(__LIBCCALL _aligned_recalloc)(void *__mptr, size_t __count, size_t __size, size_t __align);
__LIBC __PORT_DOSONLY __SAFE __WUNUSED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL _aligned_offset_realloc)(void *__mptr, size_t __newsize, size_t __align, size_t __off);
__LIBC __PORT_DOSONLY __SAFE __WUNUSED __ATTR_ALLOC_SIZE((1,2)) void *(__LIBCCALL _aligned_offset_recalloc)(void *__mptr, size_t __count, size_t __size, size_t __align, size_t __off);
__LIBC __PORT_DOSONLY __SAFE __WUNUSED __NONNULL((1)) size_t (__LIBCCALL _aligned_msize)(void *__mptr, size_t __align, size_t __off);
__LIBC __PORT_DOSONLY __SAFE void  (__LIBCCALL _aligned_free)(void *__mptr);
#endif /* __CRT_DOS */

#ifndef _WSTDLIB_DEFINED
#define _WSTDLIB_DEFINED 1
#ifdef __CRT_DOS
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_i64tow,(__INT64_TYPE__ __val, wchar_t *__restrict __dst, int __radix),i64tow,(__val,__dst,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ui64tow,(__UINT64_TYPE__ __val, wchar_t *__restrict __dst, int __radix),ui64tow,(__val,__dst,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_i64tow_s,(__INT64_TYPE__ __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix),i64tow_s,(__val,__dst,__maxlen,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ui64tow_s,(__UINT64_TYPE__ __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix),ui64tow_s,(__val,__dst,__maxlen,__radix))
#if __SIZEOF_LONG__ == 8
__REDIRECT2(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ultow,(unsigned long int __val, wchar_t *__restrict __dst, int __radix),ui64tow,_ui64tow,(__val,__dst,__radix))
__REDIRECT2(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ltow,(long int __val, wchar_t *__restrict __dst, int __radix),i64tow,_i64tow,(__val,__dst,__radix))
__REDIRECT2(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ultow_s,(unsigned long int __val, wchar_t *__dst, size_t __maxlen, int __radix),ui64tow_s,_ui64tow_s,(__val,__dst,__maxlen,__radix))
__REDIRECT2(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ltow_s,(long int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix),i64tow_s,_i64tow_s,(__val,__dst,__maxlen,__radix))
#elif __SIZEOF_LONG__ == 4
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ultow,(unsigned long int __val, wchar_t *__restrict __dst, int __radix),ultow,(__val,__dst,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ltow,(long int __val, wchar_t *__restrict __dst, int __radix),ltow,(__val,__dst,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ultow_s,(unsigned long int __val, wchar_t *__dst, size_t __maxlen, int __radix),ultow_s,(__val,__dst,__maxlen,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ltow_s,(long int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix),ltow_s,(__val,__dst,__maxlen,__radix))
#else
__LIBC __PORT_DOSONLY wchar_t *(__LIBCCALL _ultow)(unsigned long int __val, wchar_t *__restrict __dst, int __radix);
__LIBC __PORT_DOSONLY wchar_t *(__LIBCCALL _ltow)(long int __val, wchar_t *__restrict __dst, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ultow_s)(unsigned long int __val, wchar_t *__dst, size_t __maxlen, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ltow_s)(long int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix);
#endif /* __SIZEOF_LONG__ != 8 */
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_itow,(int __val, wchar_t *__restrict __dst, int __radix),itow,(__val,__dst,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_itow_s,(int __val, wchar_t *__restrict __dst, size_t __maxlen, int __radix),itow_s,(__val,__dst,__maxlen,__radix))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wgetenv,(wchar_t const *__restrict __varname),wgetenv,(__varname))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wgetenv_s,(size_t *__restrict __psize, wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __varname),wgetenv_s,(__psize,__buf,__buflen,__varname))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wdupenv_s,(wchar_t **__restrict __pbuf, size_t *__restrict __pbuflen, wchar_t const *__restrict __varname),wdupenv_s,(__pbuf,__pbuflen,__varname))
#endif /* __CRT_DOS */

__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,long int,__LIBCCALL,_wcstol_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstol_l,(__s,__pend,__radix,__locale))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,unsigned long int,__LIBCCALL,_wcstoul_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoul_l,(__s,__pend,__radix,__locale))
#if defined(__PE__) && defined(__CRT_DOS) && __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,,__LONGLONG,__LIBCCALL,_wcstoll_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),_wcstoi64_l,(__s,__pend,__radix,__locale))
__REDIRECT(__LIBC,,__ULONGLONG,__LIBCCALL,_wcstoull_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoui64_l,(__s,__pend,__radix,__locale))
#else
__REDIRECT_IFKOS(__LIBC,,__LONGLONG,__LIBCCALL,_wcstoll_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoll_l,(__s,__pend,__radix,__locale))
__REDIRECT_IFKOS(__LIBC,,__ULONGLONG,__LIBCCALL,_wcstoull_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoull_l,(__s,__pend,__radix,__locale))
#endif
__REDIRECT_IFKOS(__LIBC,,float,__LIBCCALL,_wcstof_l,(wchar_t const *__restrict __s, wchar_t **__restrict __pend, __locale_t __locale),wcstof_l,(__s,__pend,__locale))
__REDIRECT_IFKOS(__LIBC,,double,__LIBCCALL,_wcstod_l,(wchar_t const *__restrict __s, wchar_t **__pend, __locale_t __locale),wcstod_l,(__s,__pend,__locale))
__REDIRECT_IFKOS(__LIBC,,long double,__LIBCCALL,_wcstold_l,(wchar_t const *__restrict __s, wchar_t **__pend, __locale_t __locale),wcstold_l,(__s,__pend,__locale))

#ifdef __CRT_DOS
__REDIRECT_IFKOS(__LIBC,,long int,__LIBCCALL,_wtol,(wchar_t const *__restrict __s),wtol,(__s))
__REDIRECT_IFKOS(__LIBC,,long int,__LIBCCALL,_wtol_l,(wchar_t const *__restrict __s, __locale_t __locale),wtol_l,(__s,__locale))
__REDIRECT2(__LIBC,,__LONGLONG,__LIBCCALL,_wtoll,(wchar_t const *__restrict __s),wtoi64,_wtoi64,(__s))
__REDIRECT2(__LIBC,,__LONGLONG,__LIBCCALL,_wtoll_l,(wchar_t const *__restrict __s, __locale_t __locale),wtoi64_l,_wtoi64_l,(__s,__locale))
#if __SIZEOF_INT__ == __SIZEOF_LONG__
__REDIRECT2(__LIBC,,int,__LIBCCALL,_wtoi,(wchar_t const *__restrict __s),wtol,_wtol,(__s))
__REDIRECT2(__LIBC,,int,__LIBCCALL,_wtoi_l,(wchar_t const *__restrict __s, __locale_t __locale),wtol_l,_wtol_l,(__s,__locale))
#else /* __SIZEOF_INT__ == __SIZEOF_LONG__ */
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_wtoi,(wchar_t const *__restrict __s),wtoi,(__s))
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_wtoi_l,(wchar_t const *__restrict __s, __locale_t __locale),wtoi_l,(__s,__locale))
#endif /* __SIZEOF_INT__ != __SIZEOF_LONG__ */
__REDIRECT_IFKOS(__LIBC,,double,__LIBCCALL,_wtof,(wchar_t const *__restrict __s),wtof,(__s))
__REDIRECT_IFKOS(__LIBC,,double,__LIBCCALL,_wtof_l,(wchar_t const *__restrict __s, __locale_t __locale),wtof_l,(__s,__locale))
#else /* __CRT_DOS */
__LOCAL long int (__LIBCCALL _wtol)(wchar_t const *__restrict __s) { return __NAMESPACE_STD_SYM wcstol(__s,0,10); }
__LOCAL long int (__LIBCCALL _wtol_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstol_l(__s,0,10,__locale); }
__LOCAL __LONGLONG (__LIBCCALL _wtoll)(wchar_t const *__restrict __s) { return __NAMESPACE_STD_SYM wcstoll(__s,0,10); }
__LOCAL __LONGLONG (__LIBCCALL _wtoll_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoll_l(__s,0,10,__locale); }
__LOCAL int (__LIBCCALL _wtoi)(wchar_t const *__restrict __s) { return (int)__NAMESPACE_STD_SYM wcstol(__s,0,10); }
__LOCAL int (__LIBCCALL _wtoi_l)(wchar_t const *__restrict __s, __locale_t __locale) { return (int)_wcstol_l(__s,0,10,__locale); }
__LOCAL double (__LIBCCALL _wtof)(wchar_t const *__restrict __s) { return __NAMESPACE_STD_SYM wcstod(__s,0); }
__LOCAL double (__LIBCCALL _wtof_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstod_l(__s,0,__locale); }
#endif /* !__CRT_DOS */

#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstol,(__s,__pend,__radix))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstoul,(__s,__pend,__radix))
__REDIRECT2(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstol_l,_wcstol_l,(__s,__pend,__radix,__locale))
__REDIRECT2(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64_l,(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale),wcstoul_l,_wcstoul_l,(__s,__pend,__radix,__locale))
#ifdef __CRT_DOS
__REDIRECT2(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wtoi64,(wchar_t const *__restrict __s),wtol,_wtol,(__s))
__REDIRECT2(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wtoi64_l,(wchar_t const *__restrict __s, __locale_t __locale),wtol_l,_wtol_l,(__s))
#else /* __CRT_DOS */
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) { return _wcstoi64(__s,0,10); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoi64_l(__s,0,10,__locale); }
#endif /* !__CRT_DOS */
#elif __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstoll,(__s,__pend,__radix))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstoull,(__s,__pend,__radix))
__REDIRECT2(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoll_l,_wcstoll_l,(__s,__pend,__radix,__locale))
__REDIRECT2(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64_l,(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale),wcstoull_l,_wcstoull_l,(__s,__pend,__radix,__locale))
#ifdef __CRT_DOS
__REDIRECT2(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wtoi64,(wchar_t const *__restrict __s),wtoll,_wtoll,(__s))
__REDIRECT2(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wtoi64_l,(wchar_t const *__restrict __s, __locale_t __locale),wtoll_l,_wtoll_l,(__s))
#else /* __CRT_DOS */
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) { return _wcstoi64(__s,0,10); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoi64_l(__s,0,10,__locale); }
#endif /* !__CRT_DOS */
#elif defined(__CRT_DOS)
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale);
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale);
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s);
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale);
#else
__LOCAL __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) { return (__INT64_TYPE__)__NAMESPACE_STD_SYM wcstoll(__s,__pend,__radix); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) { return (__INT64_TYPE__)_wcstoll_l(__s,__pend,__radix,__locale); }
__LOCAL __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) { return (__UINT64_TYPE__)__NAMESPACE_STD_SYM wcstoull(__s,__pend,__radix); }
__LOCAL __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale) { return (__UINT64_TYPE__)_wcstoull_l(__s,__pend,__radix,__locale); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) { return _wcstoi64(__s,0,10); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoi64_l(__s,0,10,__locale); }
#endif

#ifndef __std_wcstol_defined
#define __std_wcstol_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __NONNULL((1)) long int (__LIBCCALL wcstol)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC __NONNULL((1)) unsigned long int (__LIBCCALL wcstoul)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__NAMESPACE_STD_END
#endif /* !__std_wcstol_defined */
#ifndef __wcstol_defined
#define __wcstol_defined 1
__NAMESPACE_STD_USING(wcstol)
__NAMESPACE_STD_USING(wcstoul)
#endif /* !__wcstol_defined */

#ifndef __std_wcstoll_defined
#define __std_wcstoll_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __LONGLONG (__LIBCCALL wcstoll)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__LIBC __ULONGLONG (__LIBCCALL wcstoull)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix);
__NAMESPACE_STD_END
#endif /* !__std_wcstoll_defined */
#ifndef __wcstoll_defined
#define __wcstoll_defined 1
__NAMESPACE_STD_USING(wcstoll)
__NAMESPACE_STD_USING(wcstoull)
#endif /* !__wcstoll_defined */

#ifndef __std_wcstod_defined
#define __std_wcstod_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __NONNULL((1)) double (__LIBCCALL wcstod)(wchar_t const *__restrict __s, wchar_t **__pend);
__NAMESPACE_STD_END
#endif /* !__std_wcstod_defined */
#ifndef __wcstod_defined
#define __wcstod_defined 1
__NAMESPACE_STD_USING(wcstod)
#endif /* !__wcstod_defined */

#ifndef __std_wcstof_defined
#define __std_wcstof_defined 1
__NAMESPACE_STD_BEGIN
__LIBC float (__LIBCCALL wcstof)(wchar_t const *__restrict __s, wchar_t **__pend);
__LIBC long double (__LIBCCALL wcstold)(wchar_t const *__restrict __s, wchar_t **__pend);
__NAMESPACE_STD_END
#endif /* !__std_wcstof_defined */
#ifndef __wcstof_defined
#define __wcstof_defined 1
__NAMESPACE_STD_USING(wcstof)
__NAMESPACE_STD_USING(wcstold)
#endif /* !__wcstof_defined */
#endif /* !_WSTDLIB_DEFINED */

#ifdef __CRT_DOS
#ifndef _CRT_WSYSTEM_DEFINED
#define _CRT_WSYSTEM_DEFINED 1
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wsystem,(wchar_t const *__restrict __cmd),wsystem,(__cmd))
#endif /* !_CRT_WSYSTEM_DEFINED */

#define _CVTBUFSIZE   349
__LIBC __PORT_DOSONLY char *(__LIBCCALL _fullpath)(char *__buf, char const *__path, size_t __buflen);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ecvt_s)(char *__buf, size_t __buflen, double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _fcvt_s)(char *__buf, size_t __buflen, double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _gcvt_s)(char *__buf, size_t __buflen, double __val, int __ndigit);
#endif /* __CRT_DOS */
__REDIRECT_IFKOS(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,_ecvt,(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),ecvt,(__val,__ndigit,__decptr,__sign))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,_fcvt,(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),fcvt,(__val,__ndigit,__decptr,__sign))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __NONNULL((3)),char *,__LIBCCALL,_gcvt,(double __val, int __ndigit, char *__buf),gcvt,(__val,__ndigit,__buf))

#ifdef __USE_KOS
#define __FIXED_CONST const
#else
#define __FIXED_CONST /* Nothing */
#endif
#ifdef __CRT_DOS
__LIBC int (__LIBCCALL _atoflt)(float *__restrict __result, char const *__restrict __nptr);
__LIBC int (__LIBCCALL _atodbl)(double *__restrict __result, char __FIXED_CONST *__restrict __nptr);
__LIBC int (__LIBCCALL _atoldbl)(long double *__restrict __result, char __FIXED_CONST *__restrict __nptr);
__LIBC int (__LIBCCALL _atoflt_l)(float *__restrict __result, char const *__restrict __nptr, __locale_t __locale);
__LIBC int (__LIBCCALL _atodbl_l)(double *__restrict __result, char __FIXED_CONST *__restrict __nptr, __locale_t __locale);
__LIBC int (__LIBCCALL _atoldbl_l)(long double *__restrict __result, char __FIXED_CONST *__restrict __nptr, __locale_t __locale);
#else /* __CRT_DOS */
__LOCAL int (__LIBCCALL _atoflt)(float *__restrict __result, char const *__restrict __nptr) { *__result = __NAMESPACE_STD_SYM strtof(__nptr,NULL); return 0; }
__LOCAL int (__LIBCCALL _atodbl)(double *__restrict __result, char __FIXED_CONST *__restrict __nptr) { *__result = __NAMESPACE_STD_SYM strtod(__nptr,NULL); return 0; }
__LOCAL int (__LIBCCALL _atoldbl)(long double *__restrict __result, char __FIXED_CONST *__restrict __nptr) { *__result = __NAMESPACE_STD_SYM strtold(__nptr,NULL); return 0; }
__LOCAL int (__LIBCCALL _atoflt_l)(float *__restrict __result, char const *__restrict __nptr, __locale_t __locale) { *__result = _strtof_l(__nptr,NULL,__locale); return 0; }
__LOCAL int (__LIBCCALL _atodbl_l)(double *__restrict __result, char __FIXED_CONST *__restrict __nptr, __locale_t __locale) { *__result = _strtod_l(__nptr,NULL,__locale); return 0; }
__LOCAL int (__LIBCCALL _atoldbl_l)(long double *__restrict __result, char __FIXED_CONST *__restrict __nptr, __locale_t __locale) { *__result = _strtold_l(__nptr,NULL,__locale); return 0; }
#endif /* !__CRT_DOS */
#undef __FIXED_CONST

#if defined(__CRT_DOS) && defined(__OPTIMIZE_SIZE__)
__LIBC __ATTR_CONST unsigned int __NOTHROW((__LIBCCALL _rotl)(unsigned int __val, int __shift));
__LIBC __ATTR_CONST unsigned int __NOTHROW((__LIBCCALL _rotr)(unsigned int __val, int __shift));
__LIBC __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _rotl64)(__UINT64_TYPE__ __val, int __shift));
__LIBC __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _rotr64)(__UINT64_TYPE__ __val, int __shift));
#if __SIZEOF_LONG__ == __SIZEOF_INT__
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,unsigned long int,__LIBCCALL,_lrotl,(unsigned long int __val, int __shift),_rotl,(__val,__shift))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,unsigned long int,__LIBCCALL,_lrotr,(unsigned long int __val, int __shift),_rotr,(__val,__shift))
#elif __SIZEOF_LONG__ == 8
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,unsigned long int,__LIBCCALL,_lrotl,(unsigned long int __val, int __shift),_rotl64,(__val,__shift))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,unsigned long int,__LIBCCALL,_lrotr,(unsigned long int __val, int __shift),_rotr64,(__val,__shift))
#else
__LIBC __ATTR_CONST unsigned long int __NOTHROW((__LIBCCALL _lrotl)(unsigned long int __val, int __shift));
__LIBC __ATTR_CONST unsigned long int __NOTHROW((__LIBCCALL _lrotr)(unsigned long int __val, int __shift));
#endif
#else /* __CRT_DOS */
__SYSDECL_END
#include <bits/rotate.h>
__SYSDECL_BEGIN
__LOCAL __ATTR_CONST unsigned int __NOTHROW((__LIBCCALL _rotl)(unsigned int __val, int __shift)) { return __rol_32(__val,__shift); }
__LOCAL __ATTR_CONST unsigned int __NOTHROW((__LIBCCALL _rotr)(unsigned int __val, int __shift)) { return __ror_32(__val,__shift); }
__LOCAL __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _rotl64)(__UINT64_TYPE__ __val, int __shift)) { return __rol_64(__val,__shift); }
__LOCAL __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _rotr64)(__UINT64_TYPE__ __val, int __shift)) { return __ror_64(__val,__shift); }
__LOCAL __ATTR_CONST unsigned long int __NOTHROW((__LIBCCALL _lrotl)(unsigned long int __val, int __shift)) { return __rol_32(__val,__shift); }
__LOCAL __ATTR_CONST unsigned long int __NOTHROW((__LIBCCALL _lrotr)(unsigned long int __val, int __shift)) { return __ror_32(__val,__shift); }
#endif /* !__CRT_DOS */


#ifndef _CRT_PERROR_DEFINED
#define _CRT_PERROR_DEFINED 1
#ifndef __std_perror_defined
#define __std_perror_defined 1
__NAMESPACE_STD_BEGIN
__LIBC void (__LIBCCALL perror)(char const *__s);
__NAMESPACE_STD_END
#endif /* !__std_perror_defined */
#ifndef __perror_defined
#define __perror_defined 1
__NAMESPACE_STD_USING(perror)
#endif /* !__perror_defined */
#endif  /* _CRT_PERROR_DEFINED */


__REDIRECT_IFKOS(__LIBC,__NONNULL((1)),int,__LIBCCALL,_putenv,(char *__string),putenv,(__string))
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__ && !defined(__DOS_COMPAT__)
__REDIRECT_VOID(__LIBC,__NONNULL((1,2)),__LIBCCALL,_swab,(void const *__restrict __from, void *__restrict __to, int __n_bytes),swab,(__from,__to,__n_bytes))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ && !__DOS_COMPAT__ */
__LIBC __NONNULL((1,2)) void (__LIBCCALL _swab)(void const *__restrict __from, void *__restrict __to, int __n_bytes);
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ || __DOS_COMPAT__ */

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY_ALT(setenv) errno_t (__LIBCCALL _putenv_s)(char const *__name, char const *__val);
__LIBC __PORT_DOSONLY void (__LIBCCALL _makepath)(char *__restrict __buf, char const *__drive, char const *__dir, char const *__file, char const *__ext);
__LIBC __PORT_DOSONLY void (__LIBCCALL _searchenv)(char const *__file, char const *__envvar, char *__restrict __resultpath);
__LIBC __PORT_DOSONLY void (__LIBCCALL _splitpath)(char const *__restrict __abspath, char *__drive, char *__dir, char *__file, char *__ext);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _makepath_s)(char *__buf, size_t __buflen, char const *__drive, char const *__dir, char const *__file, char const *__ext);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _searchenv_s)(char const *__file, char const *__envvar, char *__restrict __resultpath, size_t __buflen);
__REDIRECT_IFKOSFS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_splitpath_s,(char const *__restrict __abspath, char *__drive, size_t __drivelen, char *__dir, size_t __dirlen, char *__file, size_t __filelen, char *__ext, size_t __extlen),splitpath_s,(__abspath,__drive,__drivelen,__dir,__dirlen,__file,__filelen,__ext,__extlen))
#ifndef _WSTDLIBP_DEFINED
#define _WSTDLIBP_DEFINED 1
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wfullpath,(wchar_t *__restrict __abspath, wchar_t const *__restrict __path, size_t __maxlen),wfullpath,(__abspath,__path,__maxlen))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wputenv,(wchar_t const *__restrict __envstr),wputenv,(__envstr))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,void,__LIBCCALL,_wmakepath,(wchar_t *__restrict __dst, wchar_t const *__drive, wchar_t const *__dir, wchar_t const *__file, wchar_t const *__ext),wmakepath,(__dst,__drive,__dir,__file,__ext))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,void,__LIBCCALL,_wsearchenv,(wchar_t const *__file, wchar_t const *__varname, wchar_t *__restrict __dst),wsearchenv,(__file,__varname,__dst))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,void,__LIBCCALL,_wsplitpath,(wchar_t const *__restrict __abspath, wchar_t *__drive, wchar_t *__dir, wchar_t *__file, wchar_t *__ext),wsplitpath,(__abspath,__drive,__dir,__file,__ext))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wmakepath_s,(wchar_t *__restrict __dst, size_t __maxlen, wchar_t const *__drive, wchar_t const *__dir, wchar_t const *__file, wchar_t const *__ext),wmakepath_s,(__dst,__maxlen,__drive,__dir,__file,__ext))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wputenv_s,(wchar_t const *__name, wchar_t const *__val),wputenv_s,(__name,__val))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wsearchenv_s,(wchar_t const *__file, wchar_t const *__varname, wchar_t *__restrict __dst, size_t __maxlen),wsearchenv_s,(__file,__varname,__dst,__maxlen))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wsplitpath_s,(wchar_t const *__restrict __abspath, wchar_t *__drive, size_t __drivelen, wchar_t *__dir, size_t __dirlen, wchar_t *__file, size_t __filelen, wchar_t *__ext, size_t __extlen),wsplitpath_s,(__abspath,__drive,__drivelen,__dir,__dirlen,__file,__filelen,__ext,__extlen))
#endif /* !_WSTDLIBP_DEFINED */
#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED 1
__REDIRECT_IFW32_VOID(__LIBC,__PORT_DOSONLY __ATTR_COLD,__LIBCCALL,_wperror,
                     (wchar_t const *__restrict __errmsg),wperror,(__errmsg))
#endif /* !_CRT_WPERROR_DEFINED */
__LIBC __PORT_DOSONLY void (__LIBCCALL _seterrormode)(int __mode);
__LIBC __PORT_DOSONLY void (__LIBCCALL _beep)(unsigned int __freq, unsigned int __duration);
#endif /* __CRT_DOS */

#if __SIZEOF_INT__ == 4 && !defined(__DOS_COMPAT__)
__REDIRECT_IFKOS_VOID(__LIBC,,__LIBCCALL,_sleep,(__UINT32_TYPE__ __duration),sleep,(__duration))
#else /* __SIZEOF_INT__ == 4 && !__DOS_COMPAT__ */
__LIBC void (__LIBCCALL _sleep)(__UINT32_TYPE__ __duration);
#endif /* __SIZEOF_INT__ != 4 || __DOS_COMPAT__ */

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

#ifndef __swab_defined
#define __swab_defined 1
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__ && !defined(__DOS_COMPAT__)
__LIBC __NONNULL((1,2)) void (__LIBCCALL swab)(void const *__restrict __from, void *__restrict __to, int __n_bytes);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__REDIRECT_VOID(__LIBC,__NONNULL((1,2)),__LIBCCALL,swab,(void const *__restrict __from, void *__restrict __to, int __n_bytes),_swab,(__from,__to,__n_bytes))
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !__swab_defined */

#ifdef __CRT_DOS
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,itoa,(int __val, char *__buf, int __radix),_itoa,(__val,__buf,__radix))
#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ltoa,(long __val, char *__buf, int __radix),_i64toa,(__val,__buf,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ultoa,(unsigned long __val, char *__buf, int __radix),_ui64toa,(__val,__buf,__radix))
#elif __SIZEOF_LONG__ == __SIZEOF_INT__
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ltoa,(long __val, char *__buf, int __radix),_itoa,(__val,__buf,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ultoa,(unsigned long __val, char *__buf, int __radix),_ultoa,(__val,__buf,__radix))
#else /* ... */
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ltoa,(long __val, char *__buf, int __radix),_ltoa,(__val,__buf,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ultoa,(unsigned long __val, char *__buf, int __radix),_ultoa,(__val,__buf,__radix))
#endif /* !... */
__LIBC __PORT_DOSONLY_ALT(atexit) onexit_t (__LIBCCALL _onexit)(onexit_t __func);
__REDIRECT(__LIBC,__PORT_DOSONLY_ALT(atexit),onexit_t,__LIBCCALL,onexit,(onexit_t __func),_onexit,(__func))
#endif /* __CRT_DOS */

#endif /* __USE_DOS */

#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_KOS
#ifdef _ALLOCA_H
#include "__amalloc.h"
#define amalloc(s) __amalloc(s)
#define acalloc(s) __acalloc(s)
#define afree(p)   __afree(p)
#endif
#endif /* __USE_KOS */

#endif /* !_STDLIB_H */
