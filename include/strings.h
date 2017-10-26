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
#if defined(__KERNEL__) || defined(__DOS_COMPAT__)
#include <hybrid/string.h>
#endif /* __KERNEL__ || __DOS_COMPAT__ */
#if !defined(__KERNEL__) && defined(__USE_XOPEN2K8)
#include <xlocale.h>
#endif /* !__KERNEL__ && __USE_XOPEN2K8 */

__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#if defined(__USE_MISC) || !defined(__USE_XOPEN2K8)

#ifndef __bstring_defined
#define __bstring_defined 1
#ifndef __KERNEL__
#ifdef __DOS_COMPAT__
__LOCAL __NONNULL((1,2)) void (__LIBCCALL bcopy)(void const *__src, void *__dst, size_t __n) { __hybrid_memmove(__dst,__src,__n); }
__LOCAL __NONNULL((1)) void (__LIBCCALL bzero)(void *__s, size_t __n) { __hybrid_memset(__s,0,__n); }
#else /* __DOS_COMPAT__ */
__LIBC __NONNULL((1,2)) void (__LIBCCALL bcopy)(void const *__src, void *__dst, size_t __n);
__LIBC __NONNULL((1)) void (__LIBCCALL bzero)(void *__s, size_t __n);
#endif /* !__DOS_COMPAT__ */
#else
#define bcopy(src,dst,n) (void)__hybrid_memmove(dst,src,n)
#define bzero(s,n)       (void)__hybrid_memset(s,0,n)
#endif
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,bcmp,
          (void const *__s1, void const *__s2, size_t __n),memcmp,(__s1,__s2,__n))
#endif /* !__bstring_defined */

#ifndef __KERNEL__
#ifndef __index_defined
#define __index_defined 1
#ifdef __CORRECT_ISO_CPP_STRINGS_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,index,(char *__restrict __haystack, int __needle),strchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,index,(char const *__restrict __haystack, int __needle),strchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,rindex,(char *__restrict __haystack, int __needle),strrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,rindex,(char const *__restrict __haystack, int __needle),strrchr,(__haystack,__needle))
}
#else /* __CORRECT_ISO_CPP_STRINGS_H_PROTO */
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,index,(char const *__restrict __haystack, int __needle),strchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,rindex,(char const *__restrict __haystack, int __needle),strrchr,(__haystack,__needle))
#endif /* !__CORRECT_ISO_CPP_STRINGS_H_PROTO */
#endif /* !__index_defined */
#endif /* !__KERNEL__ */


#if defined(__USE_MISC) || !defined(__USE_XOPEN2K8) || defined(__USE_XOPEN2K8XSI)
#ifndef __ffs_defined
#define __ffs_defined 1
#ifdef __DOS_COMPAT__
#define ____IMPL_DO_FFS(i) \
{ int __result; \
  if (!i) return 0; \
  for (__result = 1; !(i&1); ++__result) i >>= 1; \
  return __result; \
}
#ifdef __USE_KOS
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs8)(__INT8_TYPE__ __i) ____IMPL_DO_FFS(__i)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs16)(__INT16_TYPE__ __i) ____IMPL_DO_FFS(__i)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs32)(__INT32_TYPE__ __i) ____IMPL_DO_FFS(__i)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs64)(__INT64_TYPE__ __i) ____IMPL_DO_FFS(__i)
#define ffs(i) (sizeof(i) == 4 ? __ffs32((__INT32_TYPE__)(i)) : sizeof(i) == 8 ? __ffs64((__INT64_TYPE__)(i)) : \
                sizeof(i) == 2 ? __ffs16((__INT16_TYPE__)(i)) : __ffs8((__INT8_TYPE__)(i)))
#else /* __USE_KOS */
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL ffs)(int __i) ____IMPL_DO_FFS(__i)
#endif /* !__USE_KOS */
#undef ____IMPL_DO_FFS
#else /* __DOS_COMPAT__ */
#ifdef __USE_KOS
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs8)(__INT8_TYPE__ __i);
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs16)(__INT16_TYPE__ __i);
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs32)(__INT32_TYPE__ __i);
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs64)(__INT64_TYPE__ __i);
#define ffs(i) (sizeof(i) == 4 ? __ffs32((__INT32_TYPE__)(i)) : sizeof(i) == 8 ? __ffs64((__INT64_TYPE__)(i)) : \
                sizeof(i) == 2 ? __ffs16((__INT16_TYPE__)(i)) : __ffs8((__INT8_TYPE__)(i)))
#else /* __USE_KOS */
#if defined(__CRT_KOS) && !defined(__GLC_COMPAT__)
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffs)(int __i)
         __ASMNAME("__ffs" __PP_STR(__PP_MUL8(__SIZEOF_INT__)));
#else
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffs)(int __i);
#endif
#endif /* !__USE_KOS */
#endif /* !__DOS_COMPAT__ */
#endif /* !__ffs_defined */
#endif /* __USE_MISC || !__USE_XOPEN2K8 || __USE_XOPEN2K8XSI */

#ifndef __strcasecmp_defined
#define __strcasecmp_defined 1
__REDIRECT_IFDOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strcasecmp,(char const *__s1, char const *__s2),_stricmp,(__s1,__s2))
__REDIRECT_IFDOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strncasecmp,(char const *__s1, char const *__s2, size_t __n),_strnicmp,(__s1,__s2,__n))
#endif /* !__strcasecmp_defined */
#endif /* __USE_MISC || !__USE_XOPEN2K8 */

#if !defined(__KERNEL__) && defined(__USE_XOPEN2K8)
__REDIRECT_IFDOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2,3)),int,__LIBCCALL,strcasecmp_l,(char const *__s1, char const *__s2, __locale_t __locale),_stricmp_l,(__s1,__s2,__locale))
__REDIRECT_IFDOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2,4)),int,__LIBCCALL,strncasecmp_l,(char const *__s1, char const *__s2, size_t __n, __locale_t __locale),_strnicmp_l,(__s1,__s2,__n,__locale))
#endif /* !__KERNEL__ && __USE_XOPEN2K8 */

__SYSDECL_END


#endif /* !_STRINGS_H */
