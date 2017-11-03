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
#ifndef _ASM_GENERIC_STRING_H
#define _ASM_GENERIC_STRING_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>
#include <asm/string.h>
#include <libc/string.h>

__SYSDECL_BEGIN

#if !defined(__NO_ATTR_FORCEINLINE)

__FORCELOCAL __NONNULL((1,2)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memcpy)(void *__restrict __dst,
                                     void const *__restrict __src,
                                     __SIZE_TYPE__ __n_bytes) {
 if (__n_bytes < __SIZEOF_BUSINT__*8) {
  __BUSINT_TYPE__ *__pdst = (__BUSINT_TYPE__ *)__dst;
  __BUSINT_TYPE__ const *__psrc = (__BUSINT_TYPE__ const *)__src;
  /* Use direct copy. */
  switch (__n_bytes >> 2) {
  case 7: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 6: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 5: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 4: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 3: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 2: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 1: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 0: break;
  }
  if (__n_bytes & 2) *(*(__UINT16_TYPE__ **)&__pdst)++ = *(*(__UINT16_TYPE__ const **)&__psrc)++;
  if (__n_bytes & 1) *(__UINT8_TYPE__ *)__pdst = *(__UINT8_TYPE__ const *)__psrc;
  return __dst;
 }
 if (!(__n_bytes & 3)) return __libc_memcpyl(__dst,__src,__n_bytes >> 2);
 if (__n_bytes > __SIZEOF_BUSINT__*128) {
  /* Use 2-step copy for very large, unaligned operations. */
  __libc_memcpyl(__dst,__src,__n_bytes >> 2);
  if (__n_bytes & 2) ((__UINT16_TYPE__ *)__dst)[(__n_bytes >> 1)-1] = ((__UINT16_TYPE__ const *)__src)[(__n_bytes >> 1)-1];
  if (__n_bytes & 1) ((__UINT8_TYPE__ *)__dst)[__n_bytes-1] = ((__UINT8_TYPE__ const *)__src)[__n_bytes-1];
  return __dst;
 }
 if (!(__n_bytes & 1)) return __libc_memcpyw(__dst,__src,__n_bytes >> 1);
 return __libc_memcpy(__dst,__src,__n_bytes);
}
__FORCELOCAL __NONNULL((1,2)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_mempcpy)(void *__restrict __dst,
                                      void const *__restrict __src,
                                      __SIZE_TYPE__ __n_bytes) {
 if (__n_bytes < __SIZEOF_BUSINT__*8) {
  __BUSINT_TYPE__ *__pdst = (__BUSINT_TYPE__ *)__dst;
  __BUSINT_TYPE__ const *__psrc = (__BUSINT_TYPE__ const *)__src;
  /* Use direct copy. */
  switch (__n_bytes >> 2) {
  case 7: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 6: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 5: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 4: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 3: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 2: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 1: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 0: break;
  }
  if (__n_bytes & 2) *(*(__UINT16_TYPE__ **)&__pdst)++ = *(*(__UINT16_TYPE__ const **)&__psrc)++;
  if (__n_bytes & 1) *(__UINT8_TYPE__ *)__pdst = *(__UINT8_TYPE__ const *)__psrc;
  return (__BYTE_TYPE__ *)__dst+__n_bytes;
 }
 if (!(__n_bytes & 3)) return __libc_mempcpyl(__dst,__src,__n_bytes >> 2);
 if (__n_bytes > __SIZEOF_BUSINT__*128) {
  /* Use 2-step copy for very large, unaligned operations. */
  __libc_memcpyl(__dst,__src,__n_bytes >> 2);
  if (__n_bytes & 2) ((__UINT16_TYPE__ *)__dst)[(__n_bytes >> 1)-1] = ((__UINT16_TYPE__ const *)__src)[(__n_bytes >> 1)-1];
  if (__n_bytes & 1) ((__UINT8_TYPE__ *)__dst)[__n_bytes-1] = ((__UINT8_TYPE__ const *)__src)[__n_bytes-1];
  return (__BYTE_TYPE__ *)__dst+__n_bytes;
 }
 if (!(__n_bytes & 1)) return __libc_mempcpyw(__dst,__src,__n_bytes >> 1);
 return __libc_mempcpy(__dst,__src,__n_bytes);
}
__FORCELOCAL __NONNULL((1,2)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memmove)(void *__dst, void const *__src, __SIZE_TYPE__ __n_bytes) {
 switch (__n_bytes) {
#ifdef __UINT64_TYPE__
 case 8: *(__UINT64_TYPE__ *)__dst = *(__UINT64_TYPE__ const *)__src; return __dst;
#endif
 case 4: *(__UINT32_TYPE__ *)__dst = *(__UINT32_TYPE__ const *)__src; return __dst;
 case 2: *(__UINT16_TYPE__ *)__dst = *(__UINT16_TYPE__ const *)__src; return __dst;
 case 1: *(__UINT8_TYPE__ *)__dst = *(__UINT8_TYPE__ const *)__src; __ATTR_FALLTHROUGH
 case 0: return __dst;
 default: break;
 }
 if (!(__n_bytes & 2)) return __libc_memmovel(__dst,__src,__n_bytes >> 2);
 if (!(__n_bytes & 1)) return __libc_memmovew(__dst,__src,__n_bytes >> 1);
 return __libc_memmove(__dst,__src,__n_bytes);
}
__FORCELOCAL __NONNULL((1)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords) {
 if (__n_dwords < 4) {
  __UINT32_TYPE__ *__pdst = (__UINT32_TYPE__ *)__dst;
  switch (__n_dwords) {
  case 3: *__pdst++ = __dword; __ATTR_FALLTHROUGH
  case 2: *__pdst++ = __dword; __ATTR_FALLTHROUGH
  case 1: *__pdst++ = __dword; __ATTR_FALLTHROUGH
  case 0: break;
  }
  return __dst;
 }
 return __libc_memsetl(__dst,__dword,__n_dwords);
}
__FORCELOCAL __NONNULL((1)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, __SIZE_TYPE__ __n_words) {
 if (__builtin_constant_p(__word) || __n_words > 256 ||
    (__n_words < 16 && __n_words > 4) || (__n_words > 64 && !(__n_words & 1))) {
  __constant_memsetl(__dst,(__UINT32_TYPE__)(0x00010001*__word),__n_words >> 1);
  if (__n_words & 1) ((__UINT16_TYPE__ *)__dst)[__n_words-1] = __word;
  return __dst;
 }
 switch (__n_words) {
 case 4: ((__UINT16_TYPE__ *)__dst)[0] = __word;
         ((__UINT16_TYPE__ *)__dst)[1] = __word;
         ((__UINT16_TYPE__ *)__dst)[2] = __word;
         ((__UINT16_TYPE__ *)__dst)[3] = __word;
         return __dst;
 case 3: ((__UINT16_TYPE__ *)__dst)[0] = __word;
         ((__UINT16_TYPE__ *)__dst)[1] = __word;
         ((__UINT16_TYPE__ *)__dst)[2] = __word;
         return __dst;
 case 2: ((__UINT16_TYPE__ *)__dst)[0] = __word;
         ((__UINT16_TYPE__ *)__dst)[1] = __word;
         return __dst;
 case 1: *(__UINT16_TYPE__ *)__dst = __word;
 case 0: return __dst;
 }
 return __libc_memsetw(__dst,__word,__n_words);
}
__FORCELOCAL __NONNULL((1)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memset)(void *__restrict __dst, int __byte, __SIZE_TYPE__ __n_bytes) {
 if (__builtin_constant_p(__byte) || __n_bytes > 256 ||
    (__n_bytes < 16 && __n_bytes > 4) || (__n_bytes > 64 && !(__n_bytes & 1))) {
  __constant_memsetw(__dst,(__UINT16_TYPE__)(0x0101*(__byte & 0xff)),__n_bytes >> 1);
  if (__n_bytes & 1) ((__UINT8_TYPE__ *)__dst)[__n_bytes-1] = (__UINT8_TYPE__)__byte;
  return __dst;
 }
 switch (__n_bytes) {
 case 4: ((__UINT8_TYPE__ *)__dst)[0] = (__UINT8_TYPE__)__byte;
         ((__UINT8_TYPE__ *)__dst)[1] = (__UINT8_TYPE__)__byte;
         ((__UINT8_TYPE__ *)__dst)[2] = (__UINT8_TYPE__)__byte;
         ((__UINT8_TYPE__ *)__dst)[3] = (__UINT8_TYPE__)__byte;
         return __dst;
 case 3: ((__UINT8_TYPE__ *)__dst)[0] = (__UINT8_TYPE__)__byte;
         ((__UINT8_TYPE__ *)__dst)[1] = (__UINT8_TYPE__)__byte;
         ((__UINT8_TYPE__ *)__dst)[2] = (__UINT8_TYPE__)__byte;
         return __dst;
 case 2: ((__UINT8_TYPE__ *)__dst)[0] = (__UINT8_TYPE__)__byte;
         ((__UINT8_TYPE__ *)__dst)[1] = (__UINT8_TYPE__)__byte;
         return __dst;
 case 1: *(__UINT8_TYPE__ *)__dst = (__UINT8_TYPE__)__byte;
 case 0: return __dst;
 }
 return __libc_memset(__dst,__byte,__n_bytes);
}

#define __DEFINE_MEMCMP(name,base,n,Tr,T) \
__FORCELOCAL __NONNULL((1,2)) __WUNUSED __ATTR_PURE \
Tr (__LIBCCALL name)(void const *__a, void const *__b, __SIZE_TYPE__ n) { \
 if (n < 8) { \
  T __temp; \
  T const *__pa = (T const *)__a; \
  T const *__pb = (T const *)__b; \
  switch (n) { \
  case 7: if ((__temp = *__pa++-*__pb++) != 0) return __temp; __ATTR_FALLTHROUGH \
  case 6: if ((__temp = *__pa++-*__pb++) != 0) return __temp; __ATTR_FALLTHROUGH \
  case 5: if ((__temp = *__pa++-*__pb++) != 0) return __temp; __ATTR_FALLTHROUGH \
  case 4: if ((__temp = *__pa++-*__pb++) != 0) return __temp; __ATTR_FALLTHROUGH \
  case 3: if ((__temp = *__pa++-*__pb++) != 0) return __temp; __ATTR_FALLTHROUGH \
  case 2: if ((__temp = *__pa++-*__pb++) != 0) return __temp; __ATTR_FALLTHROUGH \
  case 1: return *__pa-*__pb; \
  case 0: return 0; \
  } \
 } \
 return base(__a,__b,n); \
}
__DEFINE_MEMCMP(__constant_memcmp,__libc_memcmp,__n_bytes,int,__INT8_TYPE__)
__DEFINE_MEMCMP(__constant_memcmpw,__libc_memcmpw,__n_words,__INT16_TYPE__,__INT16_TYPE__)
__DEFINE_MEMCMP(__constant_memcmpl,__libc_memcmpl,__n_dwords,__INT32_TYPE__,__INT32_TYPE__)
#undef __DEFINE_MEMCMP
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
void *(__LIBCCALL __constant_memchr)(void const *__haystack, int __needle, __SIZE_TYPE__ __n_bytes) {
 if (__n_bytes < 8) {
  if (__n_bytes >= 1 && ((__UINT8_TYPE__ *)__haystack)[0] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack;
  if (__n_bytes >= 2 && ((__UINT8_TYPE__ *)__haystack)[1] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+1;
  if (__n_bytes >= 3 && ((__UINT8_TYPE__ *)__haystack)[2] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+2;
  if (__n_bytes >= 4 && ((__UINT8_TYPE__ *)__haystack)[3] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+3;
  if (__n_bytes >= 5 && ((__UINT8_TYPE__ *)__haystack)[4] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+4;
  if (__n_bytes >= 6 && ((__UINT8_TYPE__ *)__haystack)[5] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+5;
  if (__n_bytes >= 7 && ((__UINT8_TYPE__ *)__haystack)[6] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+6;
  return __NULLPTR;
 }
 return __libc_memchr(__haystack,__needle,__n_bytes);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT16_TYPE__ *(__LIBCCALL __constant_memchrw)(__UINT16_TYPE__ const *__haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) {
 if (__n_words < 8) {
  if (__n_words >= 1 && ((__UINT16_TYPE__ *)__haystack)[0] == __needle) return (__UINT16_TYPE__ *)__haystack;
  if (__n_words >= 2 && ((__UINT16_TYPE__ *)__haystack)[1] == __needle) return (__UINT16_TYPE__ *)__haystack+1;
  if (__n_words >= 3 && ((__UINT16_TYPE__ *)__haystack)[2] == __needle) return (__UINT16_TYPE__ *)__haystack+2;
  if (__n_words >= 4 && ((__UINT16_TYPE__ *)__haystack)[3] == __needle) return (__UINT16_TYPE__ *)__haystack+3;
  if (__n_words >= 5 && ((__UINT16_TYPE__ *)__haystack)[4] == __needle) return (__UINT16_TYPE__ *)__haystack+4;
  if (__n_words >= 6 && ((__UINT16_TYPE__ *)__haystack)[5] == __needle) return (__UINT16_TYPE__ *)__haystack+5;
  if (__n_words >= 7 && ((__UINT16_TYPE__ *)__haystack)[6] == __needle) return (__UINT16_TYPE__ *)__haystack+6;
  return __NULLPTR;
 }
 return __libc_memchrw(__haystack,__needle,__n_words);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT32_TYPE__ *(__LIBCCALL __constant_memchrl)(__UINT32_TYPE__ const *__haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) {
 if (__n_dwords < 8) {
  if (__n_dwords >= 1 && ((__UINT32_TYPE__ *)__haystack)[0] == __needle) return (__UINT32_TYPE__ *)__haystack;
  if (__n_dwords >= 2 && ((__UINT32_TYPE__ *)__haystack)[1] == __needle) return (__UINT32_TYPE__ *)__haystack+1;
  if (__n_dwords >= 3 && ((__UINT32_TYPE__ *)__haystack)[2] == __needle) return (__UINT32_TYPE__ *)__haystack+2;
  if (__n_dwords >= 4 && ((__UINT32_TYPE__ *)__haystack)[3] == __needle) return (__UINT32_TYPE__ *)__haystack+3;
  if (__n_dwords >= 5 && ((__UINT32_TYPE__ *)__haystack)[4] == __needle) return (__UINT32_TYPE__ *)__haystack+4;
  if (__n_dwords >= 6 && ((__UINT32_TYPE__ *)__haystack)[5] == __needle) return (__UINT32_TYPE__ *)__haystack+5;
  if (__n_dwords >= 7 && ((__UINT32_TYPE__ *)__haystack)[6] == __needle) return (__UINT32_TYPE__ *)__haystack+6;
  return __NULLPTR;
 }
 return __libc_memchrl(__haystack,__needle,__n_dwords);
}
#define __constant_memcpyw(dst,src,n_words)   __constant_memcpy(dst,src,(n_words)*2)
#define __constant_memcpyl(dst,src,n_dwords)  __constant_memcpy(dst,src,(n_dwords)*4)
#define __constant_mempcpyw(dst,src,n_words)  __constant_mempcpy(dst,src,(n_words)*2)
#define __constant_mempcpyl(dst,src,n_dwords) __constant_mempcpy(dst,src,(n_dwords)*4)
#define __constant_memmovew(dst,src,n_words)  __constant_memmove(dst,src,(n_words)*2)
#define __constant_memmovel(dst,src,n_dwords) __constant_memmove(dst,src,(n_dwords)*4)
#else
#define __constant_memcpy(dst,src,n_bytes)           __libc_memcpy(dst,src,n_bytes)
#define __constant_memcpyw(dst,src,n_words)          __libc_memcpyw(dst,src,n_words)
#define __constant_memcpyl(dst,src,n_dwords)         __libc_memcpyl(dst,src,n_dwords)
#define __constant_mempcpy(dst,src,n_bytes)          __libc_mempcpy(dst,src,n_bytes)
#define __constant_mempcpyw(dst,src,n_words)         __libc_mempcpyw(dst,src,n_words)
#define __constant_mempcpyl(dst,src,n_dwords)        __libc_mempcpyl(dst,src,n_dwords)
#define __constant_memmove(dst,src,n_bytes)          __libc_memmove(dst,src,n_bytes)
#define __constant_memmovew(dst,src,n_words)         __libc_memmovew(dst,src,n_words)
#define __constant_memmovel(dst,src,n_dwords)        __libc_memmovel(dst,src,n_dwords)
#define __constant_memset(dst,byte,n_bytes)          __libc_memset(dst,byte,n_bytes)
#define __constant_memsetw(dst,word,n_words)         __libc_memsetw(dst,word,n_words)
#define __constant_memsetl(dst,dword,n_dwords)       __libc_memsetl(dst,dword,n_dwords)
#define __constant_memcmp(a,b,n_bytes)               __libc_memcmp(a,b,n_bytes)
#define __constant_memcmpw(a,b,n_words)              __libc_memcmpw(a,b,n_words)
#define __constant_memcmpl(a,b,n_dwords)             __libc_memcmpl(a,b,n_dwords)
#define __constant_memchr(haystack,needle,n_bytes)   __libc_memchr(haystack,needle,n_bytes)
#define __constant_memchrw(haystack,needle,n_words)  __libc_memchrw(haystack,needle,n_words)
#define __constant_memchrl(haystack,needle,n_dwords) __libc_memchrl(haystack,needle,n_dwords)
#endif

#ifndef __asm_memcpy
#define __asm_memcpy(dst,src,n_bytes)           __libc_memcpy(dst,src,n_bytes)
#endif /* !__asm_memcpy */
#ifndef __asm_memcpyw
#define __asm_memcpyw(dst,src,n_words)          __libc_memcpyw(dst,src,n_words)
#endif /* !__asm_memcpyw */
#ifndef __asm_memcpyl
#define __asm_memcpyl(dst,src,n_dwords)         __libc_memcpyl(dst,src,n_dwords)
#endif /* !__asm_memcpyl */
#ifndef __asm_mempcpy
#define __asm_mempcpy(dst,src,n_bytes)          __libc_mempcpy(dst,src,n_bytes)
#endif /* !__asm_mempcpy */
#ifndef __asm_mempcpyw
#define __asm_mempcpyw(dst,src,n_words)         __libc_mempcpyw(dst,src,n_words)
#endif /* !__asm_mempcpyw */
#ifndef __asm_mempcpyl
#define __asm_mempcpyl(dst,src,n_dwords)        __libc_mempcpyl(dst,src,n_dwords)
#endif /* !__asm_mempcpyl */
#ifndef __asm_memmove
#define __asm_memmove(dst,src,n_bytes)          __libc_memmove(dst,src,n_bytes)
#endif /* !__asm_memmove */
#ifndef __asm_memmovew
#define __asm_memmovew(dst,src,n_words)         __libc_memmovew(dst,src,n_words)
#endif /* !__asm_memmovew */
#ifndef __asm_memmovel
#define __asm_memmovel(dst,src,n_dwords)        __libc_memmovel(dst,src,n_dwords)
#endif /* !__asm_memmovel */
#ifndef __asm_memset
#define __asm_memset(dst,byte,n_bytes)          __libc_memset(dst,byte,n_bytes)
#endif /* !__asm_memset */
#ifndef __asm_memsetw
#define __asm_memsetw(dst,word,n_words)         __libc_memsetw(dst,word,n_words)
#endif /* !__asm_memsetw */
#ifndef __asm_memsetl
#define __asm_memsetl(dst,dword,n_dwords)       __libc_memsetl(dst,dword,n_dwords)
#endif /* !__asm_memsetl */
#ifndef __asm_memcmp
#define __asm_memcmp(a,b,n_bytes)               __libc_memcmp(a,b,n_bytes)
#endif /* !__asm_memcmp */
#ifndef __asm_memcmpw
#define __asm_memcmpw(a,b,n_words)              __libc_memcmpw(a,b,n_words)
#endif /* !__asm_memcmpw */
#ifndef __asm_memcmpl
#define __asm_memcmpl(a,b,n_dwords)             __libc_memcmpl(a,b,n_dwords)
#endif /* !__asm_memcmpl */
#ifndef __asm_memchr
#define __asm_memchr(haystack,needle,n_bytes)   __libc_memchr(haystack,needle,n_bytes)
#endif /* !__asm_memchr */
#ifndef __asm_memchrw
#define __asm_memchrw(haystack,needle,n_words)  __libc_memchrw(haystack,needle,n_words)
#endif /* !__asm_memchrw */
#ifndef __asm_memchrl
#define __asm_memchrl(haystack,needle,n_dwords) __libc_memchrl(haystack,needle,n_dwords)
#endif /* !__asm_memchrl */

#ifdef __OPTIMIZE_ASM__
#define __nonconst_memcpy(dst,src,n_bytes)           __asm_memcpy(dst,src,n_bytes)
#define __nonconst_memcpyw(dst,src,n_words)          __asm_memcpyw(dst,src,n_words)
#define __nonconst_memcpyl(dst,src,n_dwords)         __asm_memcpyl(dst,src,n_dwords)
#define __nonconst_mempcpy(dst,src,n_bytes)          __asm_mempcpy(dst,src,n_bytes)
#define __nonconst_mempcpyw(dst,src,n_words)         __asm_mempcpyw(dst,src,n_words)
#define __nonconst_mempcpyl(dst,src,n_dwords)        __asm_mempcpyl(dst,src,n_dwords)
#define __nonconst_memmove(dst,src,n_bytes)          __asm_memmove(dst,src,n_bytes)
#define __nonconst_memmovew(dst,src,n_words)         __asm_memmovew(dst,src,n_words)
#define __nonconst_memmovel(dst,src,n_dwords)        __asm_memmovel(dst,src,n_dwords)
#define __nonconst_memset(dst,byte,n_bytes)          __asm_memset(dst,byte,n_bytes)
#define __nonconst_memsetw(dst,word,n_words)         __asm_memsetw(dst,word,n_words)
#define __nonconst_memsetl(dst,dword,n_dwords)       __asm_memsetl(dst,dword,n_dwords)
#define __nonconst_memcmp(a,b,n_bytes)               __asm_memcmp(a,b,n_bytes)
#define __nonconst_memcmpw(a,b,n_words)              __asm_memcmpw(a,b,n_words)
#define __nonconst_memcmpl(a,b,n_dwords)             __asm_memcmpl(a,b,n_dwords)
#define __nonconst_memchr(haystack,needle,n_bytes)   __asm_memchr(haystack,needle,n_bytes)
#define __nonconst_memchrw(haystack,needle,n_words)  __asm_memchrw(haystack,needle,n_words)
#define __nonconst_memchrl(haystack,needle,n_dwords) __asm_memchrl(haystack,needle,n_dwords)
#else /* __OPTIMIZE_ASM__ */
#define __nonconst_memcpy(dst,src,n_bytes)           __libc_memcpy(dst,src,n_bytes)
#define __nonconst_memcpyw(dst,src,n_words)          __libc_memcpyw(dst,src,n_words)
#define __nonconst_memcpyl(dst,src,n_dwords)         __libc_memcpyl(dst,src,n_dwords)
#define __nonconst_mempcpy(dst,src,n_bytes)          __libc_mempcpy(dst,src,n_bytes)
#define __nonconst_mempcpyw(dst,src,n_words)         __libc_mempcpyw(dst,src,n_words)
#define __nonconst_mempcpyl(dst,src,n_dwords)        __libc_mempcpyl(dst,src,n_dwords)
#define __nonconst_memmove(dst,src,n_bytes)          __libc_memmove(dst,src,n_bytes)
#define __nonconst_memmovew(dst,src,n_words)         __libc_memmovew(dst,src,n_words)
#define __nonconst_memmovel(dst,src,n_dwords)        __libc_memmovel(dst,src,n_dwords)
#define __nonconst_memset(dst,byte,n_bytes)          __libc_memset(dst,byte,n_bytes)
#define __nonconst_memsetw(dst,word,n_words)         __libc_memsetw(dst,word,n_words)
#define __nonconst_memsetl(dst,dword,n_dwords)       __libc_memsetl(dst,dword,n_dwords)
#define __nonconst_memcmp(a,b,n_bytes)               __libc_memcmp(a,b,n_bytes)
#define __nonconst_memcmpw(a,b,n_words)              __libc_memcmpw(a,b,n_words)
#define __nonconst_memcmpl(a,b,n_dwords)             __libc_memcmpl(a,b,n_dwords)
#define __nonconst_memchr(haystack,needle,n_bytes)   __libc_memchr(haystack,needle,n_bytes)
#define __nonconst_memchrw(haystack,needle,n_words)  __libc_memchrw(haystack,needle,n_words)
#define __nonconst_memchrl(haystack,needle,n_dwords) __libc_memchrl(haystack,needle,n_dwords)
#endif /* !__OPTIMIZE_ASM__ */

#ifdef __OPTIMIZE_CONST__
#define __opt_memcpy(dst,src,n_bytes)           (__builtin_constant_p(n_bytes) ? __constant_memcpy(dst,src,n_bytes) : __nonconst_memcpy(dst,src,n_bytes))
#define __opt_memcpyw(dst,src,n_words)          (__builtin_constant_p(n_words) ? __constant_memcpyw(dst,src,n_words) : __nonconst_memcpyw(dst,src,n_words))
#define __opt_memcpyl(dst,src,n_dwords)         (__builtin_constant_p(n_dwords) ? __constant_memcpyl(dst,src,n_dwords) : __nonconst_memcpyl(dst,src,n_dwords))
#define __opt_mempcpy(dst,src,n_bytes)          (__builtin_constant_p(n_bytes) ? __constant_mempcpy(dst,src,n_bytes) : __nonconst_mempcpy(dst,src,n_bytes))
#define __opt_mempcpyw(dst,src,n_words)         (__builtin_constant_p(n_words) ? __constant_mempcpyw(dst,src,n_words) : __nonconst_mempcpyw(dst,src,n_words))
#define __opt_mempcpyl(dst,src,n_dwords)        (__builtin_constant_p(n_dwords) ? __constant_mempcpyl(dst,src,n_dwords) : __nonconst_mempcpyl(dst,src,n_dwords))
#define __opt_memmove(dst,src,n_bytes)          (__builtin_constant_p(n_bytes) ? __constant_memmove(dst,src,n_bytes) : __nonconst_memmove(dst,src,n_bytes))
#define __opt_memmovew(dst,src,n_words)         (__builtin_constant_p(n_words) ? __constant_memmovew(dst,src,n_words) : __nonconst_memmovew(dst,src,n_words))
#define __opt_memmovel(dst,src,n_dwords)        (__builtin_constant_p(n_dwords) ? __constant_memmovel(dst,src,n_dwords) : __nonconst_memmovel(dst,src,n_dwords))
#define __opt_memset(dst,byte,n_bytes)          (__builtin_constant_p(n_bytes) ? __constant_memset(dst,byte,n_bytes) : __nonconst_memset(dst,byte,n_bytes))
#define __opt_memsetw(dst,word,n_words)         (__builtin_constant_p(n_words) ? __constant_memsetw(dst,word,n_words) : __nonconst_memsetw(dst,word,n_words))
#define __opt_memsetl(dst,dword,n_dwords)       (__builtin_constant_p(n_dwords) ? __constant_memsetl(dst,dword,n_dwords) : __nonconst_memsetl(dst,dword,n_dwords))
#define __opt_memcmp(a,b,n_bytes)               (__builtin_constant_p(n_bytes) ? __constant_memcmp(a,b,n_bytes) : __nonconst_memcmp(a,b,n_bytes))
#define __opt_memcmpw(a,b,n_words)              (__builtin_constant_p(n_words) ? __constant_memcmpw(a,b,n_words) : __nonconst_memcmpw(a,b,n_words))
#define __opt_memcmpl(a,b,n_dwords)             (__builtin_constant_p(n_dwords) ? __constant_memcmpl(a,b,n_dwords) : __nonconst_memcmpl(a,b,n_dwords))
#define __opt_memchr(haystack,needle,n_bytes)   (__builtin_constant_p(n_bytes) ? __constant_memchr(haystack,needle,n_bytes) : __nonconst_memchr(haystack,needle,n_bytes))
#define __opt_memchrw(haystack,needle,n_words)  (__builtin_constant_p(n_words) ? __constant_memchrw(haystack,needle,n_words) : __nonconst_memchrw(haystack,needle,n_words))
#define __opt_memchrl(haystack,needle,n_dwords) (__builtin_constant_p(n_dwords) ? __constant_memchrl(haystack,needle,n_dwords) : __nonconst_memchrl(haystack,needle,n_dwords))
#else /* __OPTIMIZE_CONST__ */
#define __opt_memcpy(dst,src,n_bytes)            __nonconst_memcpy(dst,src,n_bytes)
#define __opt_memcpyw(dst,src,n_words)           __nonconst_memcpyw(dst,src,n_words)
#define __opt_memcpyl(dst,src,n_dwords)          __nonconst_memcpyl(dst,src,n_dwords)
#define __opt_mempcpy(dst,src,n_bytes)           __nonconst_mempcpy(dst,src,n_bytes)
#define __opt_mempcpyw(dst,src,n_words)          __nonconst_mempcpyw(dst,src,n_words)
#define __opt_mempcpyl(dst,src,n_dwords)         __nonconst_mempcpyl(dst,src,n_dwords)
#define __opt_memmove(dst,src,n_bytes)           __nonconst_memmove(dst,src,n_bytes)
#define __opt_memmovew(dst,src,n_words)          __nonconst_memmovew(dst,src,n_words)
#define __opt_memmovel(dst,src,n_dwords)         __nonconst_memmovel(dst,src,n_dwords)
#define __opt_memset(dst,byte,n_bytes)           __nonconst_memset(dst,byte,n_bytes)
#define __opt_memsetw(dst,word,n_words)          __nonconst_memsetw(dst,word,n_words)
#define __opt_memsetl(dst,dword,n_dwords)        __nonconst_memsetl(dst,dword,n_dwords)
#define __opt_memcmp(a,b,n_bytes)                __nonconst_memcmp(a,b,n_bytes)
#define __opt_memcmpw(a,b,n_words)               __nonconst_memcmpw(a,b,n_words)
#define __opt_memcmpl(a,b,n_dwords)              __nonconst_memcmpl(a,b,n_dwords)
#define __opt_memchr(haystack,needle,n_bytes)    __nonconst_memchr(haystack,needle,n_bytes)
#define __opt_memchrw(haystack,needle,n_words)   __nonconst_memchrw(haystack,needle,n_words)
#define __opt_memchrl(haystack,needle,n_dwords)  __nonconst_memchrl(haystack,needle,n_dwords)
#endif /* !__OPTIMIZE_CONST__ */

__SYSDECL_END

#endif /* !_ASM_GENERIC_STRING_H */
