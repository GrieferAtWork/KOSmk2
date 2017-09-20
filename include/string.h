/* Copyright (__needle) 2017 Griefer@Work                                            *
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
#ifndef _STRING_H
#define _STRING_H 1

#include "__stdinc.h"
#include "__malldefs.h"
#include <features.h>
#include <hybrid/typecore.h>

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

#ifndef NULL
#ifdef __INTELLISENSE__
#   define NULL nullptr
#elif defined(__cplusplus) || defined(__LINKER__)
#   define NULL          0
#else
#   define NULL ((void *)0)
#endif
#endif

__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memmove)(void *__dst, void const *__src, size_t __n_bytes);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memset)(void *__dst, int __byte, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmp)(void const *__a, void const *__b, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strcmp)(char const *__s1, char const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strncmp)(char const *__s1, char const *__s2, size_t __n);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) char *(__LIBCCALL strstr)(char const *__haystack, char const *__needle);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(memcpy)
__NAMESPACE_STD_USING(memmove)
__NAMESPACE_STD_USING(memset)
__NAMESPACE_STD_USING(memcmp)
__NAMESPACE_STD_USING(strcmp)
__NAMESPACE_STD_USING(strncmp)
__NAMESPACE_STD_USING(strstr)

#ifdef __NAMESPACE_STD_EXISTS
#ifndef __std_strlen_defined
#define __std_strlen_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strlen)(char const *__s);
__NAMESPACE_STD_END
#endif /* !__std_strlen_defined */
#ifndef __strlen_defined
#define __strlen_defined 1
__NAMESPACE_STD_USING(strlen)
#endif /* !__strlen_defined */
#else /* __NAMESPACE_STD_EXISTS */
#ifndef __strlen_defined
#define __strlen_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strlen)(char const *__s);
#endif /* !__strlen_defined */
#endif /* !__NAMESPACE_STD_EXISTS */

#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strcpy)(char *__restrict __dst, char const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strncpy)(char *__restrict __dst, char const *__restrict __src, size_t __n);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strcat)(char *__restrict __dst, char const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strncat)(char *__restrict __dst, char const *__restrict __src, size_t __n);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strcoll)(char const *__s1, char const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL strxfrm)(char *__restrict __dst, char const *__restrict __src, size_t __n);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL strcspn)(char const *__s, char const *__reject);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL strspn)(char const *__s, char const *__accept);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) char *(__LIBCCALL strpbrk)(char const *__s, char const *__accept);
__LIBC __NONNULL((2)) char *(__LIBCCALL strtok)(char *__restrict __s, char const *__restrict __delim);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(strcpy)
__NAMESPACE_STD_USING(strncpy)
__NAMESPACE_STD_USING(strcat)
__NAMESPACE_STD_USING(strncat)
__NAMESPACE_STD_USING(strcoll)
__NAMESPACE_STD_USING(strxfrm)
__NAMESPACE_STD_USING(strcspn)
__NAMESPACE_STD_USING(strspn)
__NAMESPACE_STD_USING(strpbrk)
__NAMESPACE_STD_USING(strtok)


__LIBC __NONNULL((1)) void (__LIBCCALL __bzero)(void *__s, size_t __n) __ASMNAME("bzero");
__LIBC __NONNULL((1,2,3)) char *(__LIBCCALL __strtok_r)(char *__restrict __s, char const *__restrict __delim, char **__restrict __save_ptr) __ASMNAME("strtok_r");
#ifdef __USE_POSIX
__LIBC __NONNULL((2,3)) char *(__LIBCCALL strtok_r)(char *__restrict __s, char const *__restrict __delim, char **__restrict __save_ptr);
#endif /* __USE_POSIX */
#ifdef __USE_XOPEN2K
#ifndef __USE_GNU
__LIBC __NONNULL((2)) int (__LIBCCALL strerror_r)(int __errnum, char *__buf, size_t __buflen) __ASMNAME("__xpg_strerror_r");
#else /* !__USE_GNU */
__LIBC __WUNUSED __ATTR_RETNONNULL __NONNULL((2)) char *(__LIBCCALL strerror_r)(int __errnum, char *__buf, size_t __buflen);
#endif /* __USE_GNU */
#endif /* __USE_XOPEN2K */
#endif /* !__KERNEL__ */


__NAMESPACE_STD_BEGIN
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memchr)(void *__restrict __haystack, int __needle, size_t __n_bytes) __ASMNAME("memchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void const *(__LIBCCALL memchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes) __ASMNAME("memchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strchr)(char *__restrict __haystack, int __needle) __ASMNAME("strchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL strchr)(char const *__restrict __haystack, int __needle) __ASMNAME("strchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strrchr)(char *__restrict __haystack, int __needle) __ASMNAME("strrchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL strrchr)(char const *__restrict __haystack, int __needle) __ASMNAME("strrchr");
}
#else
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strchr)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strrchr)(char const *__restrict __haystack, int __needle);
#endif
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(memchr)
__NAMESPACE_STD_USING(strchr)
__NAMESPACE_STD_USING(strrchr)


#ifdef __USE_KOS
__LIBC __WUNUSED char const *(__LIBCCALL strerror_s)(int __errnum) ;
__LIBC __WUNUSED char const *(__LIBCCALL strerrorname_s)(int __errnum);
#endif /* __USE_KOS */
#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_RETNONNULL char *(__LIBCCALL strerror)(int __errnum);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(strerror)
#endif /* !__KERNEL__ */

#ifdef __USE_XOPEN2K8
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnlen)(char const *__string, size_t __maxlen);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL __stpcpy)(char *__restrict __dst, char const *__restrict __src) __ASMNAME("stpcpy");
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL stpcpy)(char *__restrict __dst, char const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL __stpncpy)(char *__restrict __dst, char const *__restrict __src, size_t __n) __ASMNAME("stpncpy");
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL stpncpy)(char *__restrict __dst, char const *__restrict __src, size_t __n);
#ifndef __KERNEL__
#include <xlocale.h>
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2,3)) int (__LIBCCALL strcoll_l)(char const *__s1, char const *__s2, __locale_t __l);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((2,4)) size_t (__LIBCCALL strxfrm_l)(char *__dst, char const *__src, size_t __n, __locale_t __l);
__LIBC __WUNUSED __NONNULL((2)) char *(__LIBCCALL strerror_l)(int __errnum, __locale_t __l);
__LIBC __WUNUSED __ATTR_RETNONNULL char *(__LIBCCALL strsignal)(int __sig);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL strndup)(char const *__restrict __str, size_t __max_chars);
#endif /* !__KERNEL__ */
#endif /* __USE_XOPEN2K8 */

#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL strdup)(char const *__restrict __str);
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#endif /* !__KERNEL__ */

#ifdef __USE_GNU
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL rawmemchr)(void *__restrict __haystack, int __needle) __ASMNAME("rawmemchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void const *(__LIBCCALL rawmemchr)(void const *__restrict __haystack, int __needle) __ASMNAME("rawmemchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memrchr)(void *__restrict __haystack, int __needle, size_t __n_bytes) __ASMNAME("memrchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void const *(__LIBCCALL memrchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes) __ASMNAME("memrchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strchrnul)(char *__restrict __haystack, int __needle) __ASMNAME("strchrnul");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL strchrnul)(char const *__restrict __haystack, int __needle) __ASMNAME("strchrnul");
#ifndef basename
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL basename)(char *__restrict __filename) __ASMNAME("basename");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL basename)(char const *__restrict __filename) __ASMNAME("basename");
#endif /* !basename */
}
#else
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL rawmemchr)(void const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memrchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strchrnul)(char const *__restrict __haystack, int __needle);
#ifndef basename
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL basename)(char const *__restrict __filename);
#endif /* !basename */
#endif
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) char *(__LIBCCALL strcasestr)(char const *__haystack, char const *__needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,3)) void *(__LIBCCALL memmem)(void const *__haystack, size_t __haystacklen, void const *__needle, size_t __needlelen);
__LIBC __NONNULL((1,2)) void *(__LIBCCALL __mempcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n) __ASMNAME("mempcpy");
__LIBC __NONNULL((1,2)) void *(__LIBCCALL mempcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strverscmp)(char const *__s1, char const *__s2);
#ifndef __KERNEL__
__LIBC __NONNULL((1)) char *(__LIBCCALL strfry)(char *__string);
__LIBC __NONNULL((1)) void *(__LIBCCALL memfrob)(void *__s, size_t __n);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2,3)) int (__LIBCCALL strcasecmp_l)(char const *__s1, char const *__s2, __locale_t __loc);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2,4)) int (__LIBCCALL strncasecmp_l)(char const *__s1, char const *__s2, size_t __n, __locale_t __loc);
#endif /* !__KERNEL__ */
#endif /* __USE_GNU */

#ifdef __USE_MISC
__LIBC __NONNULL((1,2)) char *(__LIBCCALL strsep)(char **__restrict __stringp, char const *__restrict __delim);
#ifndef __bstring_defined
#define __bstring_defined 1
#ifndef __KERNEL__
__LIBC __NONNULL((1,2)) void (__LIBCCALL bcopy)(void const *__src, void *__dst, size_t __n);
__LIBC __NONNULL((1)) void (__LIBCCALL bzero)(void *__s, size_t __n);
#else
#define bcopy(src,dst,n) (void)memcpy(dst,src,n)
#define bzero(s,n)       (void)memset(s,0,n)
#endif
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL bcmp)(void const *__s1, void const *__s2, size_t __n) __ASMNAME("memcmp");
#endif /* !__bstring_defined */
#ifndef __KERNEL__
#ifndef __index_defined
#define __index_defined 1
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL index)(char *__haystack, int __needle) __ASMNAME("index");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL rindex)(char const *__restrict __haystack, int __needle) __ASMNAME("rindex");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL index)(char *__haystack, int __needle) __ASMNAME("index");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL rindex)(char const *__restrict __haystack, int __needle) __ASMNAME("rindex");
}
#else
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL index)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL rindex)(char const *__restrict __haystack, int __needle);
#endif
#endif /* !__index_defined */
#endif /* !__KERNEL__ */
#ifdef __USE_GNU
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffsl)(long __i)
        __ASMNAME("__ffs" __PP_STR(__PP_MUL8(__SIZEOF_LONG__)));
#ifdef __COMPILER_HAVE_LONGLONG
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffsll)(long long __i)
         __ASMNAME("__ffs" __PP_STR(__PP_MUL8(__SIZEOF_LONG_LONG__)));
#endif /* __COMPILER_HAVE_LONGLONG */
#endif /* __USE_GNU */
#ifndef __ffs_defined
#define __ffs_defined 1
#ifdef __USE_KOS
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs8)(__INT8_TYPE__ __i);
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs16)(__INT16_TYPE__ __i);
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs32)(__INT32_TYPE__ __i);
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs64)(__INT64_TYPE__ __i);
#define ffs(i) (sizeof(i) == 4 ? __ffs32((__INT32_TYPE__)(i)) : sizeof(i) == 8 ? __ffs64((__INT64_TYPE__)(i)) : \
                sizeof(i) == 2 ? __ffs16((__INT16_TYPE__)(i)) : __ffs8((__INT8_TYPE__)(i)))
#else /* __USE_KOS */
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffs)(int __i)
         __ASMNAME("__ffs" __PP_STR(__PP_MUL8(__SIZEOF_INT__)));
#endif /* !__USE_KOS */
#endif /* !__ffs_defined */

#ifndef __strcasecmp_defined
#define __strcasecmp_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strcasecmp)(char const *__s1, char const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strncasecmp)(char const *__s1, char const *__s2, size_t __n);
#endif /* !__strcasecmp_defined */
#endif /* __USE_MISC */

#ifndef __KERNEL__
#if defined(__USE_MISC) || defined(__USE_XOPEN)
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memccpy)(void *__restrict __dst, void const *__restrict __src, int __c, size_t __n);
#endif /* __USE_MISC || __USE_XOPEN */
#endif /* !__KERNEL__ */

#ifdef __USE_KOS
/* KOS String extensions. */

#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memend)(void *__restrict __haystack, int __needle, size_t __n_bytes) __ASMNAME("memend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void const *(__LIBCCALL memend)(void const *__restrict __haystack, int __needle, size_t __n_bytes) __ASMNAME("memend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrend)(void *__restrict __haystack, int __needle, size_t __n_bytes) __ASMNAME("memrend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void const *(__LIBCCALL memrend)(void const *__restrict __haystack, int __needle, size_t __n_bytes) __ASMNAME("memrend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strend)(char *__str) __ASMNAME("strend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strend)(char const *__str) __ASMNAME("strend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnend)(char *__str, size_t __max_chars) __ASMNAME("strnend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strnend)(char const *__str, size_t __max_chars) __ASMNAME("strnend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL rawmemrchr)(void *__haystack, int __needle) __ASMNAME("rawmemrchr");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void const *(__LIBCCALL rawmemrchr)(void const *__haystack, int __needle) __ASMNAME("rawmemrchr");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strrchrnul)(char *__haystack, int __needle) __ASMNAME("strrchrnul");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strrchrnul)(char const *__restrict __haystack, int __needle) __ASMNAME("strrchrnul");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnchr)(char *__haystack, int __needle, size_t __max_chars) __ASMNAME("strnchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL strnchr)(char const *__restrict __haystack, int __needle, size_t __max_chars) __ASMNAME("strnchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnrchr)(char *__haystack, int __needle, size_t __max_chars) __ASMNAME("strnrchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL strnrchr)(char const *__restrict __haystack, int __needle, size_t __max_chars) __ASMNAME("strnrchr");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnchrnul)(char *__haystack, int __needle, size_t __max_chars) __ASMNAME("strnchrnul");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strnchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars) __ASMNAME("strnchrnul");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnrchrnul)(char *__haystack, int __needle, size_t __max_chars) __ASMNAME("strnrchrnul");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strnrchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars) __ASMNAME("strnrchrnul");
}
#else
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memend)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrend)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strend)(char const *__str);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnend)(char const *__str, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL rawmemrchr)(void const *__haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strrchrnul)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnchr)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnrchr)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnrchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars);
#endif
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlen)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlen)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlen)(void const *__haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlen)(void const *__haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL stroff)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strroff)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnoff)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnroff)(char const *__restrict __haystack, int __needle, size_t __max_chars);

#ifndef __KERNEL__
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__ATTR_CDECL strdupf)(char const *__restrict __format, ...);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL vstrdupf)(char const *__restrict __format, __VA_LIST __args);
#endif /* !__KERNEL__ */

/* byte/word/dword-wise string operations. */
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyb)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes) __ASMNAME("memcpy");
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyw)(void *__restrict __dst, void const *__restrict __src, size_t __n_words);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyl)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetb)(void *__restrict __dst, int __byte, size_t __n_bytes) __ASMNAME("memset");
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, size_t __n_words);
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int   (__LIBCCALL memcmpb)(void const *__a, void const *__b, size_t __n_bytes) __ASMNAME("memcmp");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) __INT16_TYPE__ (__LIBCCALL memcmpw)(void const *__a, void const *__b, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) __INT32_TYPE__ (__LIBCCALL memcmpl)(void const *__a, void const *__b, size_t __n_dwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmoveb)(void *__dst, void const *__src, size_t __n_bytes) __ASMNAME("memmove");
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmovew)(void *__dst, void const *__src, size_t __n_words);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmovel)(void *__dst, void const *__src, size_t __n_dwords);


/* Similar to memset(), but fill memory using the given pattern:
 * >> mempatb(addr,0x12,7):
 *    Same as regular memset().
 * >> mempatw(addr,0x12fd,7):
 *    addr&1 == 0: 12fd12fd12fd12
 *    addr&1 == 1:   fd12fd12fd1212
 *    >> '*byte = (__pattern >> 8*((uintptr_t)byte & 0x2)) & 0xff;'
 * >> mempatl(addr,0x12345678,11):
 *    addr&3 == 0: 12345678123
 *    addr&3 == 1:   34567812312
 *    addr&3 == 2:     56781231234
 *    addr&3 == 3:       78123123456
 *    >> '*byte = (__pattern >> 8*((uintptr_t)byte & 0x3)) & 0xff;'
 * WARNING: PATTERN is encoded in host endian, meaning that
 *          byte-order is reversed on little-endian machines. */
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL mempatb)(void *__restrict __dst, int __pattern, size_t __n_bytes) __ASMNAME("memset");
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL mempatw)(void *__restrict __dst, __UINT16_TYPE__ __pattern, size_t __n_bytes);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL mempatl)(void *__restrict __dst, __UINT32_TYPE__ __pattern, size_t __n_bytes);

__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) __ASMNAME("memlen");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) __ASMNAME("memrlen");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenb)(__UINT8_TYPE__ const *__haystack, int __needle) __ASMNAME("rawmemlen");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenb)(__UINT8_TYPE__ const *__haystack, int __needle) __ASMNAME("rawmemrlen");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle);

#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memchrb)(__UINT8_TYPE__ *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memchrb)(__UINT8_TYPE__ const *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memchrw)(__UINT16_TYPE__ *__haystack, __UINT16_TYPE__ __needle, size_t __n_words) __ASMNAME("memchrw");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memchrw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle, size_t __n_words) __ASMNAME("memchrw");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memchrl)(__UINT32_TYPE__ *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) __ASMNAME("memchrl");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memchrl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) __ASMNAME("memchrl");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrchrb)(__UINT8_TYPE__ *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memrchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memrchrb)(__UINT8_TYPE__ const *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memrchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrchrw)(__UINT16_TYPE__ *__haystack, __UINT16_TYPE__ __needle, size_t __n_words) __ASMNAME("memrchrw");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memrchrw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle, size_t __n_words) __ASMNAME("memrchrw");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrchrl)(__UINT32_TYPE__ *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) __ASMNAME("memrchrl");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memrchrl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) __ASMNAME("memrchrl");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memendb)(__UINT8_TYPE__ *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memendb)(__UINT8_TYPE__ const *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memendw)(__UINT16_TYPE__ *__haystack, __UINT16_TYPE__ __needle, size_t __n_words) __ASMNAME("memendw");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memendw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle, size_t __n_words) __ASMNAME("memendw");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memendl)(__UINT32_TYPE__ *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) __ASMNAME("memendl");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memendl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) __ASMNAME("memendl");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrendb)(__UINT8_TYPE__ *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memrend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memrendb)(__UINT8_TYPE__ const *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memrend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrendw)(__UINT16_TYPE__ *__haystack, __UINT16_TYPE__ __needle, size_t __n_words) __ASMNAME("memrendw");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memrendw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle, size_t __n_words) __ASMNAME("memrendw");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrendl)(__UINT32_TYPE__ *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) __ASMNAME("memrendl");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memrendl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) __ASMNAME("memrendl");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemchrb)(__UINT8_TYPE__ *__haystack, int __needle) __ASMNAME("rawmemchr");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL rawmemchrb)(__UINT8_TYPE__ const *__haystack, int __needle) __ASMNAME("rawmemchr");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ *__haystack, __UINT16_TYPE__ __needle) __ASMNAME("rawmemchrw");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle) __ASMNAME("rawmemchrw");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ *__haystack, __UINT32_TYPE__ __needle) __ASMNAME("rawmemchrl");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle) __ASMNAME("rawmemchrl");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemrchrb)(__UINT8_TYPE__ *__haystack, int __needle) __ASMNAME("rawmemrchr");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL rawmemrchrb)(__UINT8_TYPE__ const *__haystack, int __needle) __ASMNAME("rawmemrchr");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ *__haystack, __UINT16_TYPE__ __needle) __ASMNAME("rawmemrchrw");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle) __ASMNAME("rawmemrchrw");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ *__haystack, __UINT32_TYPE__ __needle) __ASMNAME("rawmemrchrl");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle) __ASMNAME("rawmemrchrl");
}
#else
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memchrb)(__UINT8_TYPE__ const *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memchrw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memchrl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrchrb)(__UINT8_TYPE__ const *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memrchr");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrchrw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrchrl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memendb)(__UINT8_TYPE__ const *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memendw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memendl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrendb)(__UINT8_TYPE__ const *__haystack, int __needle, size_t __n_bytes) __ASMNAME("memrend");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrendw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrendl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemchrb)(__UINT8_TYPE__ const *__haystack, int __needle) __ASMNAME("rawmemchr");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemrchrb)(__UINT8_TYPE__ const *__haystack, int __needle) __ASMNAME("rawmemrchr");
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle);
#endif

/* Fuzzy string compare extensions.
 *  - Lower return values indicate more closely matching data.
 *  - ZERO(0) indicates perfectly matching data. */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_strcmp)(char const *__a, char const *__b);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcmp)(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_strncmp)(char const *__a, size_t __max_a_chars, char const *__b, size_t __max_b_chars);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_strcasecmp)(char const *__a, char const *__b);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcasecmp)(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_strncasecmp)(char const *__a, size_t __max_a_chars, char const *__b, size_t __max_b_chars);


#ifndef __KERNEL__
/* >> char *strdupaf(char const *__restrict format, ...);
 * Similar to strdupf, but allocates memory of the stack, instead of the heap.
 * While this function is _very_ useful, be warned that due to the way variadic
 * arguments are managed by cdecl (the only calling convention possible to use
 * for them on most platforms) it is nearly impossible not to waste the stack
 * space that was originally allocated for the arguments (Because in cdecl, the
 * callee is responsible for argument cleanup).
 * String duplicate as fu$k!
 * ANYWAYS: Since its the stack, it shouldn't really matter, but please be advised
 *          that use of these functions fall under the same restrictions as all
 *          other alloca-style functions.
 * >> int open_file_in_folder(char const *folder, char const *file) {
 * >>   return open(strdupaf("%s/%s",folder,file),O_RDONLY);
 * >> }
 */
__LIBC __WUNUSED __ATTR_MALLOC char *(__ATTR_CDECL strdupaf)(char const *__restrict __format, ...);
__LIBC __WUNUSED __ATTR_MALLOC char *(__LIBCCALL vstrdupaf)(char const *__restrict __format, __VA_LIST __args);

#ifdef __INTELLISENSE__
#elif defined(__GNUC__)
/* Dear GCC devs: WHY IS THERE NO '__attribute__((alloca))'?
 * Or better yet! Add something like: '__attribute__((clobber("%esp")))'
 *
 * Here's what the hacky code below does:
 * We must use '__builtin_alloca' to inform the compiler that the stack pointer
 * contract has been broken, meaning that %ESP can (no longer) be used for offsets.
 * NOTE: If you don't believe me that this is required, and think this is just me
 *       ranting about missing GCC functionality, try the following code yourself:
 * >> printf("path = '%s'\n",strdupaf("%s/%s","/usr","lib")); // OK (Also try cloning this line a bunch of times)
 * >> #undef strdupaf
 * >> printf("path = '%s'\n",strdupaf("%s/%s","/usr","lib")); // Breaks
 *
 * NOTE: We also can't do __builtin_alloca(0) because that's optimized away too early
 *       and the compiler will (correctly) not mark %ESP as clobbered internally.
 *       So we're left with no choice but to waste another bit of
 *       stack memory, and more importantly: instructions!
 * Oh and by-the-way: Unlike with str(n)dupa It's only possible to implement
 *                    this as a dedicated function, when wanting to ensure
 *                    one-time evaluation of the variadic arguments.
 *                 -> So we can't just implement the whole thing as a macro.
 *                   (OK: '_vstrdupaf' could be, but when are you even going to use any to begin with...)
 * HINT: A standard-compliant, but double-evaluating version would look something like this:
 * >> #define strdupaf(...) \
 * >>   ({ char *result; size_t s;\
 * >>      s = (snprintf(NULL,0,__VA_ARGS__)+1)*sizeof(char);\
 * >>      result = (char *)__builtin_alloca(s);\
 * >>      snprintf(result,s,__VA_ARGS__);\
 * >>      result;\
 * >>   })
 */
#define strdupaf(...) \
 __XBLOCK({ char *const __sdares = strdupaf(__VA_ARGS__);\
            (void)__builtin_alloca(1);\
            __XRETURN __sdares;\
 })
#define vstrdupaf(fmt,args) \
 __XBLOCK({ char *__sdares = _vstrdupaf(fmt,args);\
            (void)__builtin_alloca(1);\
            __XRETURN __sdares;\
 })
#else
/* This might not work, because the compiler has no
 * idea these functions are violating the stack layout. */
#endif
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */

#ifdef __USE_DEBUG
#include <hybrid/debuginfo.h>
#if __USE_DEBUG != 0
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpy_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes, __DEBUGINFO);
#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL _strdup_d)(char const *__restrict __str, __DEBUGINFO);
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_XOPEN2K8
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL _strndup_d)(char const *__restrict __str, size_t __max_chars, __DEBUGINFO);
#endif /* __USE_XOPEN2K8 */
#endif /* !__KERNEL__ */
#ifdef __USE_KOS
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpyb_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes, __DEBUGINFO) __ASMNAME("_memcpy_d");
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpyw_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_words, __DEBUGINFO);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpyl_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords, __DEBUGINFO);
#ifndef __KERNEL__
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__ATTR_CDECL _strdupf_d)(__DEBUGINFO, char const *__restrict __format, ...);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL _vstrdupf_d)(char const *__restrict __format, __VA_LIST __args, __DEBUGINFO);
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */
#else /* __USE_DEBUG != 0 */
#   define _memcpy_d(dst,src,n_bytes,...) memcpy(dst,src,n_bytes)
#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#   define _strdup_d(str,...)             strdup(str)
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_XOPEN2K8
#   define _strndup_d(str,max_chars,...)  strndup(str,max_chars)
#endif /* __USE_XOPEN2K8 */
#endif /* !__KERNEL__ */
#ifdef __USE_KOS
#   define _memcpyb_d(dst,src,n_bytes,...)  memcpyb(dst,src,n_bytes)
#   define _memcpyw_d(dst,src,n_words,...)  memcpyw(dst,src,n_words)
#   define _memcpyl_d(dst,src,n_dwords,...) memcpyl(dst,src,n_dwords)
#ifndef __KERNEL__
#   define _strdupf_d(...,...)              strdupf(__VA_ARGS__)
#   define _vstrdupf_d(format,args,...)     vstrdupf(format,args)
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */
#endif /* __USE_DEBUG == 0 */
#ifdef __USE_DEBUG_HOOK
#   define memcpy(dst,src,n_bytes) _memcpy_d(dst,src,n_bytes,__DEBUGINFO_GEN)
#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#   define strdup(str)             _strdup_d(str,__DEBUGINFO_GEN)
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_XOPEN2K8
#   define strndup(str,max_chars)  _strndup_d(str,max_chars,__DEBUGINFO_GEN)
#endif /* __USE_XOPEN2K8 */
#endif /* !__KERNEL__ */
#ifdef __USE_KOS
#   define memcpyb(dst,src,n_bytes)  _memcpyb_d(dst,src,n_bytes,__DEBUGINFO_GEN)
#   define memcpyw(dst,src,n_words)  _memcpyw_d(dst,src,n_words,__DEBUGINFO_GEN)
#   define memcpyl(dst,src,n_dwords) _memcpyl_d(dst,src,n_dwords,__DEBUGINFO_GEN)
#ifndef __KERNEL__
#   define strdupf(...)              _strdupf_d(__DEBUGINFO_GEN,__VA_ARGS__)
#   define vstrdupf(format,args)     _vstrdupf_d(format,args,__DEBUGINFO_GEN)
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */
#endif /* __USE_DEBUG_HOOK */
#endif /* __USE_DEBUG */


#ifdef __USE_DOS

#ifndef __KERNEL__
__LIBC char *(__LIBCCALL _strdup)(char const *__src) __ASMNAME("strdup");
__LIBC char *(__LIBCCALL _strlwr)(char *__str) __ASMNAME("strlwr");
__LIBC char *(__LIBCCALL _strlwr_l)(char *__str, __locale_t __lc) __ASMNAME("strlwr_l");
__LIBC char *(__LIBCCALL _strnset)(char *__str, int __char, size_t __max_chars) __ASMNAME("strnset");
__LIBC char *(__LIBCCALL _strrev)(char *__str) __ASMNAME("strrev");
__LIBC char *(__LIBCCALL _strset)(char *__str, int __char) __ASMNAME("strset");
__LIBC char *(__LIBCCALL _strupr)(char *__str) __ASMNAME("strupr");
__LIBC char *(__LIBCCALL _strupr_l)(char *__str, __locale_t __lc) __ASMNAME("strupr_l");
__LIBC char *(__LIBCCALL strdup)(char const *__src);
__LIBC char *(__LIBCCALL strlwr)(char *__str);
__LIBC char *(__LIBCCALL strnset)(char *__str, int __char, size_t __max_chars);
__LIBC char *(__LIBCCALL strrev)(char *__str);
__LIBC char *(__LIBCCALL strset)(char *__str, int __char);
__LIBC char *(__LIBCCALL strupr)(char *__str);
__LIBC int (__LIBCCALL _memicmp)(void const *__a, void const *__b, size_t __n_bytes) __ASMNAME("memcasecmp");
__LIBC int (__LIBCCALL _memicmp_l)(void const *__a, void const *__b, size_t __n_bytes, __locale_t __lc) __ASMNAME("memcasecmp_l");
__LIBC int (__LIBCCALL _strcmpi)(char const *__str1, char const *__str2) __ASMNAME("strcasecmp");
__LIBC int (__LIBCCALL _strcoll_l)(char const *__str1, char const *__str2, __locale_t __lc) __ASMNAME("strcoll_l");
__LIBC int (__LIBCCALL _stricmp)(char const *__str1, char const *__str2) __ASMNAME("strcasecmp");
__LIBC int (__LIBCCALL _stricmp_l)(char const *__str1, char const *__str2, __locale_t __lc) __ASMNAME("strcasecmp_l");
__LIBC int (__LIBCCALL _stricoll)(char const *__str1, char const *__str2) __ASMNAME("strcasecoll");
__LIBC int (__LIBCCALL _stricoll_l)(char const *__str1, char const *__str2, __locale_t __lc) __ASMNAME("strcasecoll_l");
__LIBC int (__LIBCCALL _strncoll)(char const *__str1, char const *__str2, size_t __max_chars) __ASMNAME("strncoll");
__LIBC int (__LIBCCALL _strncoll_l)(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __lc) __ASMNAME("strncoll_l");
__LIBC int (__LIBCCALL _strnicmp)(char const *__str1, char const *__str2, size_t __max_chars) __ASMNAME("strncasecmp");
__LIBC int (__LIBCCALL _strnicmp_l)(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __lc) __ASMNAME("strncasecmp_l");
__LIBC int (__LIBCCALL _strnicoll)(char const *__str1, char const *__str2, size_t __max_chars) __ASMNAME("strncasecoll");
__LIBC int (__LIBCCALL _strnicoll_l)(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __lc) __ASMNAME("strncasecoll_l");
__LIBC int (__LIBCCALL memicmp)(void const *__a, void const *__b, size_t __n_bytes) __ASMNAME("memcasecmp");
__LIBC int (__LIBCCALL strcmpi)(char const *__str1, char const *__str2) __ASMNAME("strcasecmp");
__LIBC int (__LIBCCALL stricmp)(char const *__str1, char const *__str2) __ASMNAME("strcasecmp");
__LIBC int (__LIBCCALL strnicmp)(char const *__str1, char const *__str, size_t __max_chars) __ASMNAME("strncasecmp");
__LIBC size_t (__LIBCCALL _strxfrm_l)(char *__dst, char const *__src, size_t __max_chars, __locale_t __lc) __ASMNAME("strxfrm_l");
__LIBC void *(__LIBCCALL _memccpy)(void *__dst, void const *__src, int __needle, size_t __max_chars) __ASMNAME("memccpy");

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif

__LIBC int (__LIBCCALL _wcscoll_l)(wchar_t const *__str1, wchar_t const *__str2, __locale_t __lc) __ASMNAME("wcscoll_l");
__LIBC int (__LIBCCALL _wcsicmp)(wchar_t const *__str1, wchar_t const *__str2) __ASMNAME2("wcscasecmp","wcsicmp");
__LIBC int (__LIBCCALL _wcsicmp_l)(wchar_t const *__str1, wchar_t const *__str2, __locale_t __lc) __ASMNAME2("wcscasecmp_l","wcsicmp_l");
__LIBC int (__LIBCCALL _wcsicoll)(wchar_t const *__str1, wchar_t const *__str2) __ASMNAME2("wcscasecoll","wcsicoll");
__LIBC int (__LIBCCALL _wcsicoll_l)(wchar_t const *__str1, wchar_t const *__str2, __locale_t __lc) __ASMNAME2("wcscasecoll_l","wcsicoll_l");
__LIBC int (__LIBCCALL _wcsncoll)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars) __ASMNAME("wcsncoll");
__LIBC int (__LIBCCALL _wcsncoll_l)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __lc) __ASMNAME("wcsncoll_l");
__LIBC int (__LIBCCALL _wcsnicmp)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars) __ASMNAME2("wcsncasecmp","wcsnicmp");
__LIBC int (__LIBCCALL _wcsnicmp_l)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __lc) __ASMNAME2("wcsncasecmp_l","wcsnicmp_l");
__LIBC int (__LIBCCALL _wcsnicoll)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars) __ASMNAME2("wcsncasecoll","wcsnicoll");
__LIBC int (__LIBCCALL _wcsnicoll_l)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __lc) __ASMNAME2("wcsncasecoll_l","wcsnicoll_l");
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC wchar_t *(__LIBCCALL _wcsdup)(wchar_t const *__restrict __str) __ASMNAME("wcsdup");
__LIBC int (__LIBCCALL wcsicmp)(wchar_t const *__str1, wchar_t const *__str2) __ASMNAME2("wcscasecmp","wcsicmp");
__LIBC int (__LIBCCALL wcsicoll)(wchar_t const *__str1, wchar_t const *__str2) __ASMNAME2("wcscasecoll","wcsicoll");
__LIBC int (__LIBCCALL wcsnicmp)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars) __ASMNAME2("wcsncasecmp","wcsnicmp");
__LIBC size_t (__LIBCCALL _wcsxfrm_l)(wchar_t *__dst, wchar_t const *__src, size_t __max_chars, __locale_t __lc) __ASMNAME("wcsxfrm_l");
__LIBC wchar_t *(__LIBCCALL _wcslwr)(wchar_t *__str) __ASMNAME("wcslwr");
__LIBC wchar_t *(__LIBCCALL _wcslwr_l)(wchar_t *__str, __locale_t __lc) __ASMNAME("wcslwr_l");
__LIBC wchar_t *(__LIBCCALL _wcsnset)(wchar_t *__str, wchar_t __char, size_t __max_chars) __ASMNAME("wcsnset");
__LIBC wchar_t *(__LIBCCALL _wcsrev)(wchar_t *__str) __ASMNAME("wcsrev");
__LIBC wchar_t *(__LIBCCALL _wcsset)(wchar_t *__str, wchar_t __char) __ASMNAME("wcsset");
__LIBC wchar_t *(__LIBCCALL _wcsupr)(wchar_t *__str) __ASMNAME("wcsupr");
__LIBC wchar_t *(__LIBCCALL _wcsupr_l)(wchar_t *__str, __locale_t __lc) __ASMNAME("wcsupr_l");
__LIBC wchar_t *(__LIBCCALL wcslwr)(wchar_t *__str);
__LIBC wchar_t *(__LIBCCALL wcsnset)(wchar_t *__str, wchar_t __needle, size_t __max_chars);
__LIBC wchar_t *(__LIBCCALL wcsrev)(wchar_t *__str);
__LIBC wchar_t *(__LIBCCALL wcsset)(wchar_t *__str, wchar_t __needle);
__LIBC wchar_t *(__LIBCCALL wcsupr)(wchar_t *__str);

#ifndef __wcscmp_defined
#define __wcscmp_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_PURE int (__LIBCCALL wcscmp)(wchar_t const *__str1, wchar_t const *__str2);
__LIBC __ATTR_PURE int (__LIBCCALL wcsncmp)(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars);
__LIBC int (__LIBCCALL wcscoll)(wchar_t const *__str1, wchar_t const *__str2);
__LIBC size_t (__LIBCCALL wcsxfrm)(wchar_t *__dst, wchar_t const *__src, size_t __max_chars);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcscmp)
__NAMESPACE_STD_USING(wcsncmp)
__NAMESPACE_STD_USING(wcscoll)
__NAMESPACE_STD_USING(wcsxfrm)
#endif /* !__wcscmp_defined */

#ifndef __wcsspn_defined
#define __wcsspn_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_PURE size_t (__LIBCCALL wcsspn)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __ATTR_PURE size_t (__LIBCCALL wcscspn)(wchar_t const *__haystack, wchar_t const *__reject);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcsspn)
__NAMESPACE_STD_USING(wcscspn)
#endif /* !__wcsspn_defined */

#ifndef __wcslen_defined
#define __wcslen_defined 1
__NAMESPACE_STD_BEGIN
__LIBC size_t (__LIBCCALL wcslen)(wchar_t const *__str);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcslen)
#endif /* !__wcslen_defined */

#ifndef __wcsnlen_defined
#define __wcsnlen_defined 1
__LIBC size_t (__LIBCCALL wcsnlen)(wchar_t const *__src, size_t __max_chars);
#endif /* !__wcsnlen_defined */

#ifndef __wcschr_defined
#define __wcschr_defined 1
__NAMESPACE_STD_BEGIN
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC wchar_t *(__LIBCCALL wcschr)(wchar_t *__str, wchar_t __needle) __ASMNAME("wcschr");
__LIBC wchar_t *(__LIBCCALL wcsrchr)(wchar_t *__str, wchar_t __needle) __ASMNAME("wcsrchr");
__LIBC wchar_t const *(__LIBCCALL wcschr)(wchar_t const *__str, wchar_t __needle) __ASMNAME("wcschr");
__LIBC wchar_t const *(__LIBCCALL wcsrchr)(wchar_t const *__str, wchar_t __needle) __ASMNAME("wcsrchr");
}
#else
__LIBC wchar_t *(__LIBCCALL wcschr)(wchar_t const *__str, wchar_t __needle);
__LIBC wchar_t *(__LIBCCALL wcsrchr)(wchar_t const *__str, wchar_t __needle);
#endif
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcschr)
__NAMESPACE_STD_USING(wcsrchr)
#endif /* !__wcschr_defined */

#ifndef __wcscpy_defined
#define __wcscpy_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wchar_t *(__LIBCCALL wcscpy)(wchar_t *__dst, wchar_t const *__src);
__LIBC wchar_t *(__LIBCCALL wcscat)(wchar_t *__dst, wchar_t const *__src);
__LIBC wchar_t *(__LIBCCALL wcsncat)(wchar_t *__dst, wchar_t const *__src, size_t __max_chars);
__LIBC wchar_t *(__LIBCCALL wcsncpy)(wchar_t *__dst, wchar_t const *__src, size_t __max_chars);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcscpy)
__NAMESPACE_STD_USING(wcscat)
__NAMESPACE_STD_USING(wcsncat)
__NAMESPACE_STD_USING(wcsncpy)
#endif /* !__wcscpy_defined */

#ifndef __wcsdup_defined
#define __wcsdup_defined 1
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC wchar_t *(__LIBCCALL wcsdup)(wchar_t const *__restrict __str);
#endif /* !__wcsdup_defined */

#ifndef __wcsstr_defined
#define __wcsstr_defined 1
__NAMESPACE_STD_BEGIN
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcspbrk)(wchar_t *__haystack, wchar_t const *__accept) __ASMNAME("wcspbrk");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wcspbrk)(wchar_t const *__haystack, wchar_t const *__accept) __ASMNAME("wcspbrk");
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcsstr)(wchar_t *__haystack, wchar_t const *__needle) __ASMNAME("wcsstr");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wcsstr)(wchar_t const *__haystack, wchar_t const *__needle) __ASMNAME("wcsstr");
}
#else
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcspbrk)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcsstr)(wchar_t const *__haystack, wchar_t const *__needle);
#endif
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcspbrk)
__NAMESPACE_STD_USING(wcsstr)
#endif /* !__wcsstr_defined */

#ifndef __wcswcs_defined
#define __wcswcs_defined 1
#if defined(__cplusplus) && !defined(__NO_ASMNAME)
extern "C++" {
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcswcs)(wchar_t *__haystack, wchar_t const *__needle) __ASMNAME("wcsstr");
__LIBC __ATTR_PURE wchar_t const *(__LIBCCALL wcswcs)(wchar_t const *__haystack, wchar_t const *__needle) __ASMNAME("wcsstr");
}
#else
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcswcs)(wchar_t const *__haystack, wchar_t const *__needle);
#endif
#endif /* !__wcswcs_defined */

#ifndef __wcstok_defined
#define __wcstok_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wchar_t *(__LIBCCALL wcstok)(wchar_t *__str, wchar_t const *__delim);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wcstok)
#endif /* !__wcstok_defined */

#endif /* !__KERNEL__ */
#endif /* __USE_DOS */

__DECL_END

#ifdef __USE_GNU
#include <hybrid/alloca.h>
#define strdupa(s)	\
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = strlen(__old)+1; \
   char *const __new = (char *)__ALLOCA(__len); \
   __XRETURN (char *)memcpy(__new,__old,__len); \
 })
#define strndupa(s,n) \
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = strnlen(__old,(n)); \
   char *const __new = (char *)__ALLOCA(__len+1); \
   __new[__len] = '\0'; \
   __XRETURN (char *)memcpy(__new,__old,__len); \
 })
#ifdef __USE_KOS
#include "__amalloc.h"
#define strdupma(s)	\
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = strlen(__old)+1; \
   char *const __new = (char *)__amalloc(__len); \
   __XRETURN __new ? (char *)memcpy(__new,__old,__len) : (char *)0; \
 })
#define strndupma(s,n) \
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = strnlen(__old,(n)); \
   char *const __new = (char *)__amalloc(__len+1); \
   __XRETURN __new ? (__new[__len] = '\0',(char *)memcpy(__new,__old,__len)) : (char *)0; \
 })
#endif /* __USE_KOS */
#endif


#endif /* !_STRING_H */
