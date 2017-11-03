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

#ifdef __BUILDING_LIBC
#define __LIBC_FUNC(x) libc_##x
#else
#define __LIBC_FUNC(x) x
#endif

__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,__libc_memcpy,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memcpy),(__dst,__src,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,__libc_mempcpy,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(mempcpy),(__dst,__src,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,__libc_memmove,(void *__dst, void const *__src, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memmove),(__dst,__src,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,__libc_memset,(void *__dst, int __byte, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memset),(__dst,__byte,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,__libc_memcmp,(void const *__a, void const *__b, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memcmp),(__a,__b,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_strlen,(char const *__restrict __s),__LIBC_FUNC(strlen),(__s))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_wcslen,(__WCHAR_TYPE__ const *__restrict __s),__LIBC_FUNC(wcslen),(__s))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_strnlen,(char const *__restrict __s, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(strnlen),(__s,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_wcsnlen,(__WCHAR_TYPE__ const *__restrict __s, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(wcsnlen),(__s,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,__libc_strcmp,(char const *__s1, char const *__s2),__LIBC_FUNC(strcmp),(__s1,__s2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void *,__LIBCCALL,__libc_memchr,(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memchr),(__haystack,__needle,__n_bytes))
__REDIRECT2(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,__libc_strcasecmp,(char const *__s1, char const *__s2),__LIBC_FUNC(strcasecmp),_stricmp,(__s1,__s2))

#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void *,__LIBCCALL,__libc_memrchr,(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memrchr),(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void *,__LIBCCALL,__libc_rawmemchr,(void const *__restrict __haystack, int __needle),__LIBC_FUNC(rawmemchr),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,__libc_strchrnul,(char const *__restrict __haystack, int __needle),__LIBC_FUNC(strchrnul),(__haystack,__needle))
#else /* __CRT_GLC... */
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL __libc_memrchr)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) { __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__haystack+__n_bytes; while (__iter != (__BYTE_TYPE__ *)__haystack) if (*--__iter == (__BYTE_TYPE__)__needle) return (void *)__iter; return __NULLPTR; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL __libc_rawmemchr)(void const *__restrict __haystack, int __needle) { __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__haystack; while (*__iter != (__BYTE_TYPE__)__needle) ++__iter; return __iter; }
__LOCAL __WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL __libc_strchrnul)(char const *__restrict __haystack, int __needle) { char *__iter = (char *)__haystack; for (; *__iter && *__iter != __needle; ++__iter); return __iter; }
#endif /* !__CRT_GLC... */
#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__) && !defined(__KERNEL__)
__REDIRECT_VOID(__LIBC,__NONNULL((1)),__LIBCCALL,__libc_bzero,(void *__s, __SIZE_TYPE__ __n),__LIBC_FUNC(bzero),(__s,__n))
#else
#define __libc_bzero(s,n)  __libc_memset(s,0,n)
#endif


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
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1))  ,__UINT16_TYPE__ *,__LIBCCALL,__libc_memrchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words),__LIBC_FUNC(memrchrw),(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1))  ,__UINT32_TYPE__ *,__LIBCCALL,__libc_memrchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(memrchrl),(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,__libc_memend,(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memend),(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,__libc_memrend,(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memrend),(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,__libc_strend,(char const *__restrict __str),__LIBC_FUNC(strend),(__str))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,__libc_strnend,(char const *__restrict __str, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(strnend),(__str,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__WCHAR_TYPE__ *,__LIBCCALL,__libc_wcsend,(__WCHAR_TYPE__ const *__restrict __s),__LIBC_FUNC(wcsend),(__s))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__WCHAR_TYPE__ *,__LIBCCALL,__libc_wcsnend,(__WCHAR_TYPE__ const *__restrict __s, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(wcsnend),(__s,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,__libc_rawmemrchr,(void const *__restrict __haystack, int __needle),__LIBC_FUNC(rawmemrchr),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,__libc_strrchrnul,(char const *__restrict __haystack, int __needle),__LIBC_FUNC(strrchrnul),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,__libc_strnchr,(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(strnchr),(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,__libc_strnrchr,(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(strnrchr),(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,__libc_strnchrnul,(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(strnchrnul),(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,__libc_strnrchrnul,(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(strnrchrnul),(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_memlen,(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memlen),(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_memrlen,(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes),__LIBC_FUNC(memrlen),(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_rawmemlen,(void const *__restrict __haystack, int __needle),__LIBC_FUNC(rawmemlen),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_rawmemrlen,(void const *__restrict __haystack, int __needle),__LIBC_FUNC(rawmemrlen),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_stroff,(char const *__restrict __haystack, int __needle),__LIBC_FUNC(stroff),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_strroff,(char const *__restrict __haystack, int __needle),__LIBC_FUNC(strroff),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_strnoff,(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(strnoff),(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_strnroff,(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars),__LIBC_FUNC(strnroff),(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,__libc_memendw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words),__LIBC_FUNC(memendw),(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,__libc_memendl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(memendl),(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,__libc_memrendw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words),__LIBC_FUNC(memrendw),(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,__libc_memrendl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(memrendl),(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,__libc_rawmemchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle),__LIBC_FUNC(rawmemchrw),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,__libc_rawmemchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle),__LIBC_FUNC(rawmemchrl),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,__libc_rawmemrchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle),__LIBC_FUNC(rawmemrchrw),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,__libc_rawmemrchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle),__LIBC_FUNC(rawmemrchrl),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_memlenw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words),__LIBC_FUNC(memlenw),(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_memlenl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(memlenl),(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_memrlenw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words),__LIBC_FUNC(memrlenw),(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_memrlenl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(memrlenl),(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_rawmemlenw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle),__LIBC_FUNC(rawmemlenw),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_rawmemlenl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle),__LIBC_FUNC(rawmemlenl),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_rawmemrlenw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle),__LIBC_FUNC(rawmemrlenw),(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__SIZE_TYPE__,__LIBCCALL,__libc_rawmemrlenl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle),__LIBC_FUNC(rawmemrlenl),(__haystack,__needle))
#else /* __CRT_KOS... */
/* (ab-)use wide-character memory functions. (NOTE: These are not exported by DOS) */
#if __SIZEOF_WCHAR_T__ == 4 && (defined(__CRT_GLC) && !defined(__DOS_COMPAT__))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memcpyl,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(wmemcpy),(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_mempcpyl,(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(wmempcpy),(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,__libc_memmovel,(void *__dst, void const *__src, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(wmemmove),(__dst,__src,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),__INT32_TYPE__,__LIBCCALL,__libc_memcmpl,(void const *__a, void const *__b, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(wmemcmp),(__a,__b,__n_dwords))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1))  ,void *,__LIBCCALL,__libc_memsetl,(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(wmemset),(__dst,__dword,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1))  ,__UINT32_TYPE__ *,__LIBCCALL,__libc_memchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords),__LIBC_FUNC(wmemchr),(__haystack,__needle,__n_dwords))
#else /* Wide... */
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memcpyl)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords) { return __libc_memcpy(__dst,__src,__n_dwords*4); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_mempcpyl)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords) { return __libc_mempcpy(__dst,__src,__n_dwords*4); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memmovel)(void *__dst, void const *__src, __SIZE_TYPE__ __n_dwords) { return __libc_memmove(__dst,__src,__n_dwords*4); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) __INT32_TYPE__ (__LIBCCALL __libc_memcmpl)(void const *__a, void const *__b, __SIZE_TYPE__ __n_dwords) { __INT32_TYPE__ __res; __INT32_TYPE__ const *__pa = (__INT32_TYPE__ const *)__a,*__pb = (__INT32_TYPE__ const *)__b; while (__n_dwords--) { if ((__res = (*__pa++)-(*__pb++)) != 0) return __res; } return 0; }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL __libc_memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords) { __UINT32_TYPE__ *__iter = (__UINT32_TYPE__ *)__dst; while (__n_dwords--) *__iter++ = __dword; return __dst; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1))   __UINT32_TYPE__ *(__LIBCCALL __libc_memchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) { __UINT32_TYPE__ *__iter,*__end; __end = (__iter = (__UINT32_TYPE__ *)__haystack)+__n_dwords; for (; __iter != __end; ++__iter) if (*__iter == __needle) return __iter; return __NULLPTR; }
#endif /* !Wide... */
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memcpyw)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words) { return __libc_memcpy(__dst,__src,__n_words*2); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_mempcpyw)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words) { return __libc_mempcpy(__dst,__src,__n_words*2); }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL __libc_memmovew)(void *__dst, void const *__src, __SIZE_TYPE__ __n_words) { return __libc_memmove(__dst,__src,__n_words*2); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) __INT16_TYPE__ (__LIBCCALL __libc_memcmpw)(void const *__a, void const *__b, __SIZE_TYPE__ __n_words) { __INT16_TYPE__ __res; __INT16_TYPE__ const *__pa = (__INT16_TYPE__ const *)__a,*__pb = (__INT16_TYPE__ const *)__b; while (__n_words--) { if ((__res = (*__pa++)-(*__pb++)) != 0) return __res; } return 0; }
__LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL __libc_memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, __SIZE_TYPE__ __n_words) { __UINT16_TYPE__ *__iter = (__UINT16_TYPE__ *)__dst; while (__n_words--) *__iter++ = __word; return __dst; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1))   __UINT16_TYPE__ *(__LIBCCALL __libc_memchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_words) { __UINT16_TYPE__ *__iter,*__end; __end = (__iter = (__UINT16_TYPE__ *)__haystack)+__n_words; for (; __iter != __end; ++__iter) if (*__iter == __needle) return __iter; return __NULLPTR; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1))   __UINT16_TYPE__ *(__LIBCCALL __libc_memrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) { __UINT16_TYPE__ *__iter = (__UINT16_TYPE__ *)__haystack+__n_words; while (__iter != (__UINT16_TYPE__ *)__haystack) if (*--__iter == __needle) return __iter; return __NULLPTR; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1))   __UINT32_TYPE__ *(__LIBCCALL __libc_memrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) { __UINT32_TYPE__ *__iter = (__UINT32_TYPE__ *)__haystack+__n_dwords; while (__iter != (__UINT32_TYPE__ *)__haystack) if (*--__iter == __needle) return __iter; return __NULLPTR; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL __libc_memend)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) { __BYTE_TYPE__ *__iter,*__end; __end = (__iter = (__BYTE_TYPE__ *)__haystack)+__n_bytes; for (; __iter != __end; ++__iter) if (*__iter == __needle) break; return (void *)__iter; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL __libc_memrend)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) { __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__haystack+__n_bytes; while (__iter != __haystack) if (*--__iter == __needle) break; return (void *)__iter; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL __libc_strend)(char const *__restrict __str) { return (char *)__str+__libc_strlen(__str); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL __libc_strnend)(char const *__restrict __str, __SIZE_TYPE__ __max_chars) { return (char *)__str+__libc_strnlen(__str,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __WCHAR_TYPE__ *(__LIBCCALL __libc_wcsend)(__WCHAR_TYPE__ const *__restrict __str) { return (__WCHAR_TYPE__ *)__str+__libc_wcslen(__str); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __WCHAR_TYPE__ *(__LIBCCALL __libc_wcsnend)(__WCHAR_TYPE__ const *__restrict __str, __SIZE_TYPE__ __max_chars) { return (__WCHAR_TYPE__ *)__str+__libc_wcsnlen(__str,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL __libc_rawmemrchr)(void const *__restrict __haystack, int __needle) { __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__haystack; while (*--__iter != __needle); return __iter; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL __libc_strrchrnul)(char const *__restrict __haystack, int __needle) { char *__result = (char *)__haystack-1,*__iter = (char *)__haystack; for (; *__iter; ++__iter) if (*__iter == __needle) __result = __iter; return __result; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL __libc_strnchr)(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars) { char *__iter = (char *)__haystack,*__end = __iter+__max_chars; for (; __iter != __end && *__iter; ++__iter) if (*__iter == __needle) return __iter; return __NULLPTR; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL __libc_strnrchr)(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars) { char *__iter = (char *)__haystack,*__end = __iter+__max_chars,*__result = 0; for (; __iter != __end && *__iter; ++__iter) if (*__iter == __needle) __result = __iter; return __result; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL __libc_strnchrnul)(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars) { char *__iter = (char *)__haystack,*__end = __iter+__max_chars; for (; __iter != __end && *__iter && *__iter != __needle; ++__iter); return __iter; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL __libc_strnrchrnul)(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars) { char *__iter = (char *)__haystack,*__end = __iter+__max_chars,*__result = (char *)__haystack-1; for (; __iter != __end && *__iter; ++__iter) if (*__iter == __needle) __result = __iter; return __result; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_memlen)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) { return (__UINTPTR_TYPE__)__libc_memend(__haystack,__needle,__n_bytes) - (__UINTPTR_TYPE__)__haystack; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_memrlen)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) { return (__UINTPTR_TYPE__)__libc_memrend(__haystack,__needle,__n_bytes) - (__UINTPTR_TYPE__)__haystack; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_rawmemlen)(void const *__restrict __haystack, int __needle) { return (__UINTPTR_TYPE__)__libc_rawmemchr(__haystack,__needle) - (__UINTPTR_TYPE__)__haystack; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_rawmemrlen)(void const *__restrict __haystack, int __needle) { return (__UINTPTR_TYPE__)__libc_rawmemrchr(__haystack,__needle) - (__UINTPTR_TYPE__)__haystack; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_stroff)(char const *__restrict __haystack, int __needle) { return (__SIZE_TYPE__)(__libc_strchrnul(__haystack,__needle)-__haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_strroff)(char const *__restrict __haystack, int __needle) { return (__SIZE_TYPE__)(__libc_strrchrnul(__haystack,__needle)-__haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_strnoff)(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars) { return (__SIZE_TYPE__)(__libc_strnchrnul(__haystack,__needle,__max_chars)-__haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_strnroff)(char const *__restrict __haystack, int __needle, __SIZE_TYPE__ __max_chars) { return (__SIZE_TYPE__)(__libc_strnrchrnul(__haystack,__needle,__max_chars)-__haystack); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL __libc_memendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) { __UINT16_TYPE__ *__iter,*__end; __end = (__iter = (__UINT16_TYPE__ *)__haystack)+__n_words; for (; __iter != __end; ++__iter) if (*__iter == __needle) break; return __iter; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL __libc_memendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) { __UINT32_TYPE__ *__iter,*__end; __end = (__iter = (__UINT32_TYPE__ *)__haystack)+__n_dwords; for (; __iter != __end; ++__iter) if (*__iter == __needle) break; return __iter; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL __libc_memrendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) { __UINT16_TYPE__ *__iter = (__UINT16_TYPE__ *)__haystack+__n_words; while (__iter != __haystack) if (*--__iter == __needle) break; return __iter; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL __libc_memrendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) { __UINT32_TYPE__ *__iter = (__UINT32_TYPE__ *)__haystack+__n_dwords; while (__iter != __haystack) if (*--__iter == __needle) break; return __iter; }
__LOCAL __WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL __libc_rawmemchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { __UINT16_TYPE__ *__iter = (__UINT16_TYPE__ *)__haystack; while (*__iter != __needle) ++__iter; return __iter; }
__LOCAL __WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL __libc_rawmemchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { __UINT32_TYPE__ *__iter = (__UINT32_TYPE__ *)__haystack; while (*__iter != __needle) ++__iter; return __iter; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL __libc_rawmemrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { __UINT16_TYPE__ *__iter = (__UINT16_TYPE__ *)__haystack; while (*--__iter != __needle); return __iter; }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL __libc_rawmemrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { __UINT32_TYPE__ *__iter = (__UINT32_TYPE__ *)__haystack; while (*--__iter != __needle); return __iter; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_memlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) { return (__SIZE_TYPE__)(__libc_memendw(__haystack,__needle,__n_words) - __haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_memlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) { return (__SIZE_TYPE__)(__libc_memendl(__haystack,__needle,__n_dwords) - __haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_memrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) { return (__SIZE_TYPE__)(__libc_memrendw(__haystack,__needle,__n_words) - __haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_memrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) { return (__SIZE_TYPE__)(__libc_memrendl(__haystack,__needle,__n_dwords) - __haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_rawmemlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return (__SIZE_TYPE__)(__libc_rawmemchrw(__haystack,__needle) - __haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_rawmemlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return (__SIZE_TYPE__)(__libc_rawmemchrl(__haystack,__needle) - __haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_rawmemrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return (__SIZE_TYPE__)(__libc_rawmemrchrw(__haystack,__needle) - __haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __SIZE_TYPE__ (__LIBCCALL __libc_rawmemrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return (__SIZE_TYPE__)(__libc_rawmemrchrl(__haystack,__needle) - __haystack); }
#endif /* !__CRT_KOS... */
#undef __LIBC_FUNC

__SYSDECL_END

#endif /* !_LIBC_STRING_H */
