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
#ifndef __GUARD_HYBRID_STRING_H
#define __GUARD_HYBRID_STRING_H 1

#include <hybrid/typecore.h>
#include <bits/types.h>

#ifdef __CC__
__SYSDECL_BEGIN

__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,__libc_memcpy,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_bytes),memcpy,(__dst,__src,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,__libc_memmove,(void *__dst, void const *__src, __SIZE_TYPE__ __n_bytes),memmove,(__dst,__src,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,__libc_memset,(void *__dst, int __byte, __SIZE_TYPE__ __n_bytes),memset,(__dst,__byte,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,__libc_memcmp,(void const *__a, void const *__b, __SIZE_TYPE__ __n_bytes),memcmp,(__a,__b,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_strlen,(char const *__restrict __s),strlen,(__s))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,__libc_strcmp,(char const *__s1, char const *__s2),strcmp,(__s1,__s2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void *,__LIBCCALL,__libc_memchr,(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes),memchr,(__haystack,__needle,__n_bytes))

#if defined(__CRT_KOS) && (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memcpyw,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words),memcpyw,(__dst,__src,__n_words))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memcpyl,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords),memcpyl,(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memmovew,(void *__dst, void const *__src, __SIZE_TYPE__ __n_words),memmovew,(__dst,__src,__n_words))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memmovel,(void *__dst, void const *__src, __SIZE_TYPE__ __n_dwords),memmovel,(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int   ,__LIBCCALL,__libc_memcmpw,(void const *__a, void const *__b, __SIZE_TYPE__ __n_words),memcmpw,(__a,__b,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int   ,__LIBCCALL,__libc_memcmpl,(void const *__a, void const *__b, __SIZE_TYPE__ __n_dwords),memcmpl,(__a,__b,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1))  ,void *,__LIBCCALL,__libc_memsetw,(void *__restrict __dst, __UINT16_TYPE__ __word, __SIZE_TYPE__ __n_words),memsetw,(__dst,__word,__n_words))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1))  ,void *,__LIBCCALL,__libc_memsetl,(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords),memsetl,(__dst,__dword,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1))  ,void *,__LIBCCALL,__libc_memchrw,(void const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words),memchrw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1))  ,void *,__LIBCCALL,__libc_memchrl,(void const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords),memchrl,(__haystack,__needle,__n_dwords))
#else /* __CRT_KOS... */
/* (ab-)use wide-character memory functions. (NOTE: These are not exported by DOS) */
#if __SIZEOF_WCHAR_T__ == 4 && (defined(__CRT_GLC) && !defined(__DOS_COMPAT__))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memcpyl,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords),wmemcpy,(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memmovel,(void *__dst, void const *__src, __SIZE_TYPE__ __n_dwords),wmemmove,(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int   ,__LIBCCALL,__libc_memcmpl,(void const *__a, void const *__b, __SIZE_TYPE__ __n_dwords),wmemcmp,(__a,__b,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1))  ,void *,__LIBCCALL,__libc_memsetl,(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords),wmemset,(__dst,__dword,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1))  ,void *,__LIBCCALL,__libc_memchrl,(void const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords),wmemchr,(__haystack,__needle,__n_dwords))
#else /* Wide... */
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memcpyl)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords) { return __libc_memcpy(__dst,__src,__n_dwords*4); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memmovel)(void *__dst, void const *__src, __SIZE_TYPE__ __n_dwords) { return __libc_memmove(__dst,__src,__n_dwords*4); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int   (__LIBCCALL __libc_memcmpl)(void const *__a, void const *__b, __SIZE_TYPE__ __n_dwords) { return __libc_memcmp(__a,__b,__n_dwords*4); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL __libc_memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords) { __UINT32_TYPE__ *__iter = (__UINT32_TYPE__ *)__dst; while (__n_dwords--) *__iter++ = __dword; return __dst; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1))   void *(__LIBCCALL __libc_memchrl)(void const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) { __UINT32_TYPE__ *__iter,*__end; __end = (__iter = (__UINT32_TYPE__ *)__haystack)+__n_dwords; for (; __iter != __end; ++__iter) if (*__iter == __needle) return __iter; return 0; }
#endif /* !Wide... */
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memcpyw)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words) { return __libc_memcpy(__dst,__src,__n_words*2); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memmovew)(void *__dst, void const *__src, __SIZE_TYPE__ __n_words) { return __libc_memmove(__dst,__src,__n_words*2); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int   (__LIBCCALL __libc_memcmpw)(void const *__a, void const *__b, __SIZE_TYPE__ __n_words) { return __libc_memcmp(__a,__b,__n_words*2); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL __libc_memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, __SIZE_TYPE__ __n_words) { __UINT16_TYPE__ *__iter = (__UINT16_TYPE__ *)__dst; while (__n_words--) *__iter++ = __word; return __dst; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1))   void *(__LIBCCALL __libc_memchrw)(void const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_words) { __UINT16_TYPE__ *__iter,*__end; __end = (__iter = (__UINT16_TYPE__ *)__haystack)+__n_words; for (; __iter != __end; ++__iter) if (*__iter == __needle) return __iter; return 0; }
#endif /* !__CRT_KOS... */


#if defined(__OPTIMIZE__) && !defined(__NO_builtin_constant_p)

__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *
(__LIBCCALL __hybrid_memcpy2)(void *__restrict __dst, void const *__restrict __src,
                              __SIZE_TYPE__ __n_bytes) {
 if (__builtin_constant_p(__n_bytes)) {
  if ((__n_bytes % 4) == 0) return __libc_memcpyl(__dst,__src,__n_bytes/4);
  if ((__n_bytes % 2) == 0) return __libc_memcpyw(__dst,__src,__n_bytes/2);
 }
 return __libc_memcpy(__dst,__src,__n_bytes);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *
(__LIBCCALL __hybrid_memmove2)(void *__restrict __dst, void const *__restrict __src,
                               __SIZE_TYPE__ __n_bytes) {
 if (__builtin_constant_p(__n_bytes)) {
  if ((__n_bytes % 4) == 0) return __libc_memmovel(__dst,__src,__n_bytes/4);
  if ((__n_bytes % 2) == 0) return __libc_memmovew(__dst,__src,__n_bytes/2);
 }
 return __libc_memmove(__dst,__src,__n_bytes);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) void *
(__LIBCCALL __hybrid_memset2)(void *__restrict __dst, int __byte,
                              __SIZE_TYPE__ __n_bytes) {
 if (__builtin_constant_p(__byte) &&
     __builtin_constant_p(__n_bytes)) {
  if ((__n_bytes % 4) == 0) return __libc_memsetl(__dst,(__UINT32_TYPE__)(0x01010101u*__byte),__n_bytes/4);
  if ((__n_bytes % 2) == 0) return __libc_memsetw(__dst,(__UINT16_TYPE__)(0x0101u*__byte),__n_bytes/4);
 }
 return __libc_memset(__dst,__byte,__n_bytes);
}
__FORCELOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int
(__LIBCCALL __hybrid_memcmp2)(void const *__a, void const *__b,
                              __SIZE_TYPE__ __n_bytes) {
 if (__builtin_constant_p(__a == __b) && (__a == __b)) return 0;
 if (__builtin_constant_p(__n_bytes)) {
  if ((__n_bytes % 4) == 0) return __libc_memcmpl(__a,__b,__n_bytes/4);
  if ((__n_bytes % 2) == 0) return __libc_memcmpw(__a,__b,__n_bytes/2);
 }
 return __libc_memcmp(__a,__b,__n_bytes);
}

#define __hybrid_memcpy(dst,src,n_bytes)    __hybrid_memcpy2(dst,byte,n_bytes)
#define __hybrid_memmove(dst,src,n_bytes)   __hybrid_memmove2(dst,src,n_bytes)
#define __hybrid_memset(dst,byte,n_bytes)   __hybrid_memset2(dst,byte,n_bytes)
#define __hybrid_memcmp(a,b,n_bytes)        __hybrid_memcmp2(dst,byte,n_bytes)
#else
#define __hybrid_memcpy(dst,src,n_bytes)    __libc_memcpy(dst,src,n_bytes)
#define __hybrid_memmove(dst,src,n_bytes)   __libc_memmove(dst,src,n_bytes)
#define __hybrid_memset(dst,byte,n_bytes)   __libc_memset(dst,byte,n_bytes)
#define __hybrid_memcmp(a,b,n_bytes)        __libc_memcmp(a,b,n_bytes)
#endif
#define __hybrid_strlen(s)                  __libc_strlen(s)
#define __hybrid_strcmp(s1,s2)              __libc_strcmp(s1,s2)
#define __hybrid_memchr(ptr,needle,n_bytes) __libc_memchr(ptr,needle,n_bytes)

__SYSDECL_END
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_STRING_H */
