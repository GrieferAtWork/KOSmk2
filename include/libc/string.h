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
#ifndef _LIBC_STRING_H
#define _LIBC_STRING_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef __LIBC_FUNC
#ifdef __BUILDING_LIBC
#define __LIBC_FUNC(x) libc_##x
#else
#define __LIBC_FUNC(x) x
#endif
#endif

__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,__libc_memcpy,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memcpy),(__dst,__src,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,__libc_mempcpy,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(mempcpy),(__dst,__src,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,__libc_memmove,(void *__dst, void const *__src, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memmove),(__dst,__src,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,__libc_memset,(void *__dst, int __byte, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memset),(__dst,__byte,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,__libc_memcmp,(void const *__a, void const *__b, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memcmp),(__a,__b,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_strlen,(char const *__restrict __s),__LIBC_FUNC(strlen),(__s))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_strnlen,(char const *__restrict __s, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(strnlen),(__s,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,__libc_strcmp,(char const *__s1, char const *__s2),__LIBC_FUNC(strcmp),(__s1,__s2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void *,__LIBCCALL,__libc_memchr,(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memchr),(__haystack,__needle,__n_bytes))

#if defined(__CRT_KOS) && (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memcpyw,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words),__LIBC_FUNC(memcpyw),(__dst,__src,__n_words))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memcpyl,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(memcpyl),(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_mempcpyw,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words),__LIBC_FUNC(mempcpyw),(__dst,__src,__n_words))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_mempcpyl,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(mempcpyl),(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memmovew,(void *__dst, void const *__src, __SIZE_TYPE__ __n_words),__LIBC_FUNC(memmovew),(__dst,__src,__n_words))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memmovel,(void *__dst, void const *__src, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(memmovel),(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),__INT16_TYPE__,__LIBCCALL,__libc_memcmpw,(void const *__a, void const *__b, __SIZE_TYPE__ __n_words),__LIBC_FUNC(memcmpw),(__a,__b,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),__INT32_TYPE__,__LIBCCALL,__libc_memcmpl,(void const *__a, void const *__b, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(memcmpl),(__a,__b,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1))  ,void *,__LIBCCALL,__libc_memsetw,(void *__restrict __dst, __UINT16_TYPE__ __word, __SIZE_TYPE__ __n_words),__LIBC_FUNC(memsetw),(__dst,__word,__n_words))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1))  ,void *,__LIBCCALL,__libc_memsetl,(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(memsetl),(__dst,__dword,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1))  ,__UINT16_TYPE__ *,__LIBCCALL,__libc_memchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words),__LIBC_FUNC(memchrw),(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1))  ,__UINT32_TYPE__ *,__LIBCCALL,__libc_memchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(memchrl),(__haystack,__needle,__n_dwords))
#else /* __CRT_KOS... */
/* (ab-)use wide-character memory functions. (NOTE: These are not exported by DOS) */
#if __SIZEOF_WCHAR_T__ == 4 && (defined(__CRT_GLC) && !defined(__DOS_COMPAT__))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memcpyl,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords),wmemcpy,(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_mempcpyl,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords),wmempcpy,(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memmovel,(void *__dst, void const *__src, __SIZE_TYPE__ __n_dwords),wmemmove,(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),__INT32_TYPE__,__LIBCCALL,__libc_memcmpl,(void const *__a, void const *__b, __SIZE_TYPE__ __n_dwords),wmemcmp,(__a,__b,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1))  ,void *,__LIBCCALL,__libc_memsetl,(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords),wmemset,(__dst,__dword,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1))  ,__UINT32_TYPE__ *,__LIBCCALL,__libc_memchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords),wmemchr,(__haystack,__needle,__n_dwords))
#else /* Wide... */
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memcpyl)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords) { return __libc_memcpy(__dst,__src,__n_dwords*4); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_mempcpyl)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords) { return __libc_mempcpy(__dst,__src,__n_dwords*4); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memmovel)(void *__dst, void const *__src, __SIZE_TYPE__ __n_dwords) { return __libc_memmove(__dst,__src,__n_dwords*4); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) __INT32_TYPE__ (__LIBCCALL __libc_memcmpl)(void const *__a, void const *__b, __SIZE_TYPE__ __n_dwords) { __INT32_TYPE__ __res; __INT32_TYPE__ const *__pa = (__INT32_TYPE__ const *)__a,*__pb = (__INT32_TYPE__ const *)__b; while (__n_dwords--) { if ((__res = (*__pa++)-(*__pb++)) != 0) return __res; } return 0; }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL __libc_memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords) { __UINT32_TYPE__ *__iter = (__UINT32_TYPE__ *)__dst; while (__n_dwords--) *__iter++ = __dword; return __dst; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1))   __UINT32_TYPE__ *(__LIBCCALL __libc_memchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) { __UINT32_TYPE__ *__iter,*__end; __end = (__iter = (__UINT32_TYPE__ *)__haystack)+__n_dwords; for (; __iter != __end; ++__iter) if (*__iter == __needle) return __iter; return 0; }
#endif /* !Wide... */
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memcpyw)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words) { return __libc_memcpy(__dst,__src,__n_words*2); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_mempcpyw)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words) { return __libc_mempcpy(__dst,__src,__n_words*2); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memmovew)(void *__dst, void const *__src, __SIZE_TYPE__ __n_words) { return __libc_memmove(__dst,__src,__n_words*2); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) __INT16_TYPE__ (__LIBCCALL __libc_memcmpw)(void const *__a, void const *__b, __SIZE_TYPE__ __n_words) { __INT16_TYPE__ __res; __INT16_TYPE__ const *__pa = (__INT16_TYPE__ const *)__a,*__pb = (__INT16_TYPE__ const *)__b; while (__n_words--) { if ((__res = (*__pa++)-(*__pb++)) != 0) return __res; } return 0; }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL __libc_memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, __SIZE_TYPE__ __n_words) { __UINT16_TYPE__ *__iter = (__UINT16_TYPE__ *)__dst; while (__n_words--) *__iter++ = __word; return __dst; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1))   __UINT16_TYPE__ *(__LIBCCALL __libc_memchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_words) { __UINT16_TYPE__ *__iter,*__end; __end = (__iter = (__UINT16_TYPE__ *)__haystack)+__n_words; for (; __iter != __end; ++__iter) if (*__iter == __needle) return __iter; return 0; }
#endif /* !__CRT_KOS... */
#undef __LIBC_FUNC

__SYSDECL_END

#endif /* !_LIBC_STRING_H */
