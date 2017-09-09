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
#ifndef _STRINGS_H
#define _STRINGS_H 1

#include <__stdinc.h>
#include <features.h>

__DECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#if defined(__USE_MISC) || !defined(__USE_XOPEN2K8)

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


#if defined(__USE_MISC) || !defined(__USE_XOPEN2K8) || defined(__USE_XOPEN2K8XSI)
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
#endif

#ifndef __strcasecmp_defined
#define __strcasecmp_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strcasecmp)(char const *__s1, char const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strncasecmp)(char const *__s1, char const *__s2, size_t __n);
#endif /* !__strcasecmp_defined */

__DECL_END

#ifdef __USE_XOPEN2K8
#ifndef __KERNEL__
#include <xlocale.h>

__DECL_BEGIN
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2,3)) int (__LIBCCALL strcasecmp_l)(char const *__s1, char const *__s2, __locale_t __loc);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2,4)) int (__LIBCCALL strncasecmp_l)(char const *__s1, char const *__s2, size_t __n, __locale_t __loc);
__DECL_END

#endif /* !__KERNEL__ */
#endif /* __USE_XOPEN2K8 */


#endif /* !_STRINGS_H */
