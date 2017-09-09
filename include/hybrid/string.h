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
__DECL_BEGIN

#ifdef __OPTIMIZE__
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memcpyb)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_bytes) __ASMNAME("memcpy");
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memcpyw)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words) __ASMNAME("memcpyw");
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memcpyl)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords) __ASMNAME("memcpyl");
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL __libc_memsetb)(void *__restrict __dst, int __byte, __SIZE_TYPE__ __n_bytes) __ASMNAME("memset");
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL __libc_memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, __SIZE_TYPE__ __n_words) __ASMNAME("memsetw");
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL __libc_memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords) __ASMNAME("memsetl");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int   (__LIBCCALL __libc_memcmpb)(void const *__a, void const *__b, __SIZE_TYPE__ __n_bytes) __ASMNAME("memcmp");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int   (__LIBCCALL __libc_memcmpw)(void const *__a, void const *__b, __SIZE_TYPE__ __n_words) __ASMNAME("memcmpw");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int   (__LIBCCALL __libc_memcmpl)(void const *__a, void const *__b, __SIZE_TYPE__ __n_dwords) __ASMNAME("memcmpl");

__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *
(__LIBCCALL __hybrid_memcpy2)(void *__restrict __dst, void const *__restrict __src,
                              __SIZE_TYPE__ __n_bytes) {
 if (__builtin_constant_p(__n_bytes)) {
  if ((__n_bytes % 4) == 0) return __libc_memcpyl(__dst,__src,__n_bytes/4);
  if ((__n_bytes % 2) == 0) return __libc_memcpyw(__dst,__src,__n_bytes/2);
 }
 return __libc_memcpyb(__dst,__src,__n_bytes);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) void *
(__LIBCCALL __hybrid_memset2)(void *__restrict __dst, int __byte,
                              __SIZE_TYPE__ __n_bytes) {
 if (__builtin_constant_p(__byte) &&
     __builtin_constant_p(__n_bytes)) {
  if ((__n_bytes % 4) == 0) return __libc_memsetl(__dst,(__UINT32_TYPE__)(0x01010101u*__byte),__n_bytes/4);
  if ((__n_bytes % 2) == 0) return __libc_memsetw(__dst,(__UINT16_TYPE__)(0x0101u*__byte),__n_bytes/4);
 }
 return __libc_memsetb(__dst,__byte,__n_bytes);
}
__FORCELOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int
(__LIBCCALL __hybrid_memcmp2)(void const *__a, void const *__b,
                              __SIZE_TYPE__ __n_bytes) {
 if (__builtin_constant_p(__a == __b) && (__a == __b)) return 0;
 if (__builtin_constant_p(__n_bytes)) {
  if ((__n_bytes % 4) == 0) return __libc_memcmpl(__a,__b,__n_bytes/4);
  if ((__n_bytes % 2) == 0) return __libc_memcmpw(__a,__b,__n_bytes/2);
 }
 return __libc_memcmpb(__a,__b,__n_bytes);
}

#define __hybrid_memcpy(dst,src,n_bytes)  __libc_memcpyb(dst,byte,n_bytes)
#define __hybrid_memset(dst,byte,n_bytes) __libc_memsetb(dst,byte,n_bytes)
#define __hybrid_memcmp(a,b,n_bytes)      __libc_memcmpb(dst,byte,n_bytes)
#else

__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memcpyb)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_bytes) __ASMNAME("memcpy");
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL __libc_memsetb)(void *__restrict __dst, int __byte, __SIZE_TYPE__ __n_bytes) __ASMNAME("memset");
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int   (__LIBCCALL __libc_memcmpb)(void const *__a, void const *__b, __SIZE_TYPE__ __n_bytes) __ASMNAME("memcmp");
#define __hybrid_memcpy(dst,src,n_bytes)  __libc_memcpyb(dst,byte,n_bytes)
#define __hybrid_memset(dst,byte,n_bytes) __libc_memsetb(dst,byte,n_bytes)
#define __hybrid_memcmp(a,b,n_bytes)      __libc_memcmpb(dst,byte,n_bytes)

#endif

__DECL_END
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_STRING_H */
