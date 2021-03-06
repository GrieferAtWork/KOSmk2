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
#include "features.h"
#include "hybrid/typecore.h"
#include "libc/string.h"
#include "xlocale.h"
#ifdef __OPTIMIZE_LIBC__
#include <asm-generic/string.h>
#endif

/* Memory functions (An optional `[b|w|l]' suffix is a KOS extension):
 *   [STD] memset[b|w|l]     - Fill memory with the given byte/word/dword
 *   [STD] memcpy[b|w|l]     - Copy memory between non-overlapping memory blocks.
 *   [GLC] mempcpy[b|w|l]    - Same as `memcpy[b|w|l]', but return `DST+N_(BYTES|WORDS|DWORDS)', rather than `DST'
 *   [STD] memmove[b|w|l]    - Move memory between potentially overlapping memory blocks.
 *   [STD] memchr[b|w|l]     - Ascendingly search for `NEEDLE', starting at `HAYSTACK'. - Return `NULL' if `NEEDLE' wasn't found.
 *   [GLC] memrchr[b|w|l]    - Descendingly search for `NEEDLE', starting at `HAYSTACK+N_(BYTES|WORDS|DWORDS)'. - Return `NULL' if `NEEDLE' wasn't found.
 *   [GLC] rawmemchr[b|w|l]  - Same as `memchr[b|w|l]' with a search limit of `(size_t)-1/sizeof(T)'
 *   [KOS] rawmemrchr[b|w|l] - Same as `memrchr[b|w|l]' without a search limit, starting at `HAYSTACK-sizeof(T)'
 *   [KOS] memend[b|w|l]     - Same as `memchr[b|w|l]', but return `HAYSTACK+N_(BYTES|WORDS|DWORDS)', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] memrend[b|w|l]    - Same as `memrchr[b|w|l]', but return `HAYSTACK-1', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] memlen[b|w|l]     - Same as `memend[b|w|l]', but return the offset from `HAYSTACK', rather than the actual address.
 *   [KOS] memrlen[b|w|l]    - Same as `memrend[b|w|l]', but return the offset from `HAYSTACK', rather than the actual address.
 *   [KOS] rawmemlen[b|w|l]  - Same as `rawmemchr[b|w|l]', but return the offset from `HAYSTACK', rather than the actual address.
 *   [KOS] rawmemrlen[b|w|l] - Same as `rawmemrchr[b|w|l]', but return the offset from `HAYSTACK', rather than the actual address.
 *   [KOS] mempat[b|w|l]     - Same as `memset', but repeat a multi-byte pattern on aligned addresses.
 * String functions:
 *   [STD] strlen            - Return the length of the string in characters (Same as `rawmemlen[...](STR,'\0')�)
 *   [STD] strnlen           - Same as `strlen', but don't exceed `MAX_CHARS' characters (Same as `memlen[...](STR,'\0',MAX_CHARS)�)
 *   [KOS] strend            - Same as `STR+strlen(STR)'
 *   [KOS] strnend           - Same as `STR+strnlen(STR,MAX_CHARS)'
 *   [STD] strchr            - Return the pointer of the first instance of `NEEDLE', or `NULL' if `NEEDLE' wasn't found.
 *   [STD] strrchr           - Return the pointer of the last instance of `NEEDLE', or `NULL' if `NEEDLE' wasn't found.
 *   [KOS] strnchr           - Same as `strchr', but don't exceed `MAX_CHARS' characters.
 *   [KOS] strnrchr          - Same as `strrchr', but don't exceed `MAX_CHARS' characters.
 *   [GLC] strchrnul         - Same as `strchr', but return `strend(STR)', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] strrchrnul        - Same as `strrchr', but return `STR-1', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] strnchrnul        - Same as `strnchr', but return `strnend(STR,MAX_CHARS)', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] strnrchrnul       - Same as `strnrchr', but return `STR-1', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] stroff            - Same as `strchrnul', but return the offset from `STR', rather than the actual address.
 *   [KOS] strroff           - Same as `strrchrnul', but return the offset from `STR', rather than the actual address.
 *   [KOS] strnoff           - Same as `strnchrnul', but return the offset from `STR', rather than the actual address.
 *   [KOS] strnroff          - Same as `strnrchrnul', but return the offset from `STR', rather than the actual address.
 *   [STD] strcpy            - Same as `memcpy(DST,SRC,(strlen(SRC)+1)*sizeof(char))�
 *   [STD] strcat            - Same as `memcpy(strend(DST),SRC,(strlen(SRC)+1)*sizeof(char))'
 *   [STD] strncpy           - Similar to `strcpy', but always write `DSTSIZE' characters, copying from `SRC' and filling the rest with padding ZEROes.
 *   [STD] strncat           - Same as Copy `strnlen(SRC,MAX_CHARS)' characters to `strned(DST)', then append a NUL-character thereafter. - Return `DST'.
 *   [GLC] stpcpy            - Same as `mempcpy(DST,SRC,(strlen(SRC)+1)*sizeof(char))-1�
 *   [GLC] stpncpy           - Same as `strncpy(DST,SRC,DSTSIZE)+strnlen(SRC,DSTSIZE)' (Returns a pointer to the end of `DST', or to the first NUL-character)
 */

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

#ifndef NULL
#define NULL __NULLPTR
#endif


#ifdef __OPTIMIZE_LIBC__
#define __local_memset(dst,byte,n_bytes)                              __opt_memset(dst,byte,n_bytes)
#define __local_memsetw(dst,word,n_words)                             __opt_memsetw(dst,word,n_words)
#define __local_memsetl(dst,dword,n_dwords)                           __opt_memsetl(dst,dword,n_dwords)
#define __local_memsetq(dst,qword,n_qwords)                           __opt_memsetq(dst,qword,n_qwords)
#define __local_memcmp(a,b,n_bytes)                                   __opt_memcmp(a,b,n_bytes)
#define __local_memcmpw(a,b,n_words)                                  __opt_memcmpw(a,b,n_words)
#define __local_memcmpl(a,b,n_dwords)                                 __opt_memcmpl(a,b,n_dwords)
#define __local_memcmpq(a,b,n_qwords)                                 __opt_memcmpq(a,b,n_qwords)
#define __local_memcpy(dst,src,n_bytes)                               __opt_memcpy(dst,src,n_bytes)
#define __local_memcpyw(dst,src,n_words)                              __opt_memcpyw(dst,src,n_words)
#define __local_memcpyl(dst,src,n_dwords)                             __opt_memcpyl(dst,src,n_dwords)
#define __local_memcpyq(dst,src,n_qwords)                             __opt_memcpyq(dst,src,n_qwords)
#define __local_memmove(dst,src,n_bytes)                              __opt_memmove(dst,src,n_bytes)
#define __local_memmovew(dst,src,n_words)                             __opt_memmovew(dst,src,n_words)
#define __local_memmovel(dst,src,n_dwords)                            __opt_memmovel(dst,src,n_dwords)
#define __local_memmoveq(dst,src,n_qwords)                            __opt_memmoveq(dst,src,n_qwords)
#define __local_mempcpy(dst,src,n_bytes)                              __opt_mempcpy(dst,src,n_bytes)
#define __local_mempcpyw(dst,src,n_words)                             __opt_mempcpyw(dst,src,n_words)
#define __local_mempcpyl(dst,src,n_dwords)                            __opt_mempcpyl(dst,src,n_dwords)
#define __local_mempcpyq(dst,src,n_qwords)                            __opt_mempcpyq(dst,src,n_qwords)
#define __local_memchr(haystack,needle,n_bytes)                       __opt_memchr(haystack,needle,n_bytes)
#define __local_memchrw(haystack,needle,n_words)                      __opt_memchrw(haystack,needle,n_words)
#define __local_memchrl(haystack,needle,n_dwords)                     __opt_memchrl(haystack,needle,n_dwords)
#define __local_memchrq(haystack,needle,n_qwords)                     __opt_memchrq(haystack,needle,n_qwords)
#define __local_memrchr(haystack,needle,n_bytes)                      __opt_memrchr(haystack,needle,n_bytes)
#define __local_memrchrw(haystack,needle,n_words)                     __opt_memrchrw(haystack,needle,n_words)
#define __local_memrchrl(haystack,needle,n_dwords)                    __opt_memrchrl(haystack,needle,n_dwords)
#define __local_memrchrq(haystack,needle,n_qwords)                    __opt_memrchrq(haystack,needle,n_qwords)
#define __local_memend(haystack,needle,n_bytes)                       __opt_memend(haystack,needle,n_bytes)
#define __local_memendw(haystack,needle,n_words)                      __opt_memendw(haystack,needle,n_words)
#define __local_memendl(haystack,needle,n_dwords)                     __opt_memendl(haystack,needle,n_dwords)
#define __local_memendq(haystack,needle,n_qwords)                     __opt_memendq(haystack,needle,n_qwords)
#define __local_memrend(haystack,needle,n_bytes)                      __opt_memrend(haystack,needle,n_bytes)
#define __local_memrendw(haystack,needle,n_words)                     __opt_memrendw(haystack,needle,n_words)
#define __local_memrendl(haystack,needle,n_dwords)                    __opt_memrendl(haystack,needle,n_dwords)
#define __local_memrendq(haystack,needle,n_qwords)                    __opt_memrendq(haystack,needle,n_qwords)
#define __local_rawmemchr(haystack,needle)                            __opt_rawmemchr(haystack,needle)
#define __local_rawmemchrw(haystack,needle)                           __opt_rawmemchrw(haystack,needle)
#define __local_rawmemchrl(haystack,needle)                           __opt_rawmemchrl(haystack,needle)
#define __local_rawmemchrq(haystack,needle)                           __opt_rawmemchrq(haystack,needle)
#define __local_rawmemrchr(haystack,needle)                           __opt_rawmemrchr(haystack,needle)
#define __local_rawmemrchrw(haystack,needle)                          __opt_rawmemrchrw(haystack,needle)
#define __local_rawmemrchrl(haystack,needle)                          __opt_rawmemrchrl(haystack,needle)
#define __local_rawmemrchrq(haystack,needle)                          __opt_rawmemrchrq(haystack,needle)
#define __local_memlen(haystack,needle,n_bytes)                       __opt_memlen(haystack,needle,n_bytes)
#define __local_memlenw(haystack,needle,n_words)                      __opt_memlenw(haystack,needle,n_words)
#define __local_memlenl(haystack,needle,n_dwords)                     __opt_memlenl(haystack,needle,n_dwords)
#define __local_memlenq(haystack,needle,n_qwords)                     __opt_memlenq(haystack,needle,n_qwords)
#define __local_memrlen(haystack,needle,n_bytes)                      __opt_memrlen(haystack,needle,n_bytes)
#define __local_memrlenw(haystack,needle,n_words)                     __opt_memrlenw(haystack,needle,n_words)
#define __local_memrlenl(haystack,needle,n_dwords)                    __opt_memrlenl(haystack,needle,n_dwords)
#define __local_memrlenq(haystack,needle,n_qwords)                    __opt_memrlenq(haystack,needle,n_qwords)
#define __local_rawmemlen(haystack,needle)                            __opt_rawmemlen(haystack,needle)
#define __local_rawmemlenw(haystack,needle)                           __opt_rawmemlenw(haystack,needle)
#define __local_rawmemlenl(haystack,needle)                           __opt_rawmemlenl(haystack,needle)
#define __local_rawmemlenq(haystack,needle)                           __opt_rawmemlenq(haystack,needle)
#define __local_rawmemrlen(haystack,needle)                           __opt_rawmemrlen(haystack,needle)
#define __local_rawmemrlenw(haystack,needle)                          __opt_rawmemrlenw(haystack,needle)
#define __local_rawmemrlenl(haystack,needle)                          __opt_rawmemrlenl(haystack,needle)
#define __local_rawmemrlenq(haystack,needle)                          __opt_rawmemrlenq(haystack,needle)
#else /* __OPTIMIZE_LIBC__ */
#define __local_memset(dst,byte,n_bytes)                              __libc_memset(dst,byte,n_bytes)
#define __local_memsetw(dst,word,n_words)                             __libc_memsetw(dst,word,n_words)
#define __local_memsetl(dst,dword,n_dwords)                           __libc_memsetl(dst,dword,n_dwords)
#define __local_memsetq(dst,qword,n_qwords)                           __libc_memsetq(dst,qword,n_qwords)
#define __local_memcmp(a,b,n_bytes)                                   __libc_memcmp(a,b,n_bytes)
#define __local_memcmpw(a,b,n_words)                                  __libc_memcmpw(a,b,n_words)
#define __local_memcmpl(a,b,n_dwords)                                 __libc_memcmpl(a,b,n_dwords)
#define __local_memcmpq(a,b,n_qwords)                                 __libc_memcmpq(a,b,n_qwords)
#define __local_memcpy(dst,src,n_bytes)                               __libc_memcpy(dst,src,n_bytes)
#define __local_memcpyw(dst,src,n_words)                              __libc_memcpyw(dst,src,n_words)
#define __local_memcpyl(dst,src,n_dwords)                             __libc_memcpyl(dst,src,n_dwords)
#define __local_memcpyq(dst,src,n_qwords)                             __libc_memcpyq(dst,src,n_qwords)
#define __local_memmove(dst,src,n_bytes)                              __libc_memmove(dst,src,n_bytes)
#define __local_memmovew(dst,src,n_words)                             __libc_memmovew(dst,src,n_words)
#define __local_memmovel(dst,src,n_dwords)                            __libc_memmovel(dst,src,n_dwords)
#define __local_memmoveq(dst,src,n_qwords)                            __libc_memmoveq(dst,src,n_qwords)
#define __local_mempcpy(dst,src,n_bytes)                              __libc_mempcpy(dst,src,n_bytes)
#define __local_mempcpyw(dst,src,n_words)                             __libc_mempcpyw(dst,src,n_words)
#define __local_mempcpyl(dst,src,n_dwords)                            __libc_mempcpyl(dst,src,n_dwords)
#define __local_mempcpyq(dst,src,n_qwords)                            __libc_mempcpyq(dst,src,n_qwords)
#define __local_memchr(haystack,needle,n_bytes)                       __libc_memchr(haystack,needle,n_bytes)
#define __local_memchrw(haystack,needle,n_words)                      __libc_memchrw(haystack,needle,n_words)
#define __local_memchrl(haystack,needle,n_dwords)                     __libc_memchrl(haystack,needle,n_dwords)
#define __local_memchrq(haystack,needle,n_qwords)                     __libc_memchrq(haystack,needle,n_qwords)
#define __local_memrchr(haystack,needle,n_bytes)                      __libc_memrchr(haystack,needle,n_bytes)
#define __local_memrchrw(haystack,needle,n_words)                     __libc_memrchrw(haystack,needle,n_words)
#define __local_memrchrl(haystack,needle,n_dwords)                    __libc_memrchrl(haystack,needle,n_dwords)
#define __local_memrchrq(haystack,needle,n_qwords)                    __libc_memrchrq(haystack,needle,n_qwords)
#define __local_memend(haystack,needle,n_bytes)                       __libc_memend(haystack,needle,n_bytes)
#define __local_memendw(haystack,needle,n_words)                      __libc_memendw(haystack,needle,n_words)
#define __local_memendl(haystack,needle,n_dwords)                     __libc_memendl(haystack,needle,n_dwords)
#define __local_memendq(haystack,needle,n_qwords)                     __libc_memendq(haystack,needle,n_qwords)
#define __local_memrend(haystack,needle,n_bytes)                      __libc_memrend(haystack,needle,n_bytes)
#define __local_memrendw(haystack,needle,n_words)                     __libc_memrendw(haystack,needle,n_words)
#define __local_memrendl(haystack,needle,n_dwords)                    __libc_memrendl(haystack,needle,n_dwords)
#define __local_memrendq(haystack,needle,n_qwords)                    __libc_memrendq(haystack,needle,n_qwords)
#define __local_rawmemchr(haystack,needle)                            __libc_rawmemchr(haystack,needle)
#define __local_rawmemchrw(haystack,needle)                           __libc_rawmemchrw(haystack,needle)
#define __local_rawmemchrl(haystack,needle)                           __libc_rawmemchrl(haystack,needle)
#define __local_rawmemchrq(haystack,needle)                           __libc_rawmemchrq(haystack,needle)
#define __local_rawmemrchr(haystack,needle)                           __libc_rawmemrchr(haystack,needle)
#define __local_rawmemrchrw(haystack,needle)                          __libc_rawmemrchrw(haystack,needle)
#define __local_rawmemrchrl(haystack,needle)                          __libc_rawmemrchrl(haystack,needle)
#define __local_rawmemrchrq(haystack,needle)                          __libc_rawmemrchrq(haystack,needle)
#define __local_memlen(haystack,needle,n_bytes)                       __libc_memlen(haystack,needle,n_bytes)
#define __local_memlenw(haystack,needle,n_words)                      __libc_memlenw(haystack,needle,n_words)
#define __local_memlenl(haystack,needle,n_dwords)                     __libc_memlenl(haystack,needle,n_dwords)
#define __local_memlenq(haystack,needle,n_qwords)                     __libc_memlenq(haystack,needle,n_qwords)
#define __local_memrlen(haystack,needle,n_bytes)                      __libc_memrlen(haystack,needle,n_bytes)
#define __local_memrlenw(haystack,needle,n_words)                     __libc_memrlenw(haystack,needle,n_words)
#define __local_memrlenl(haystack,needle,n_dwords)                    __libc_memrlenl(haystack,needle,n_dwords)
#define __local_memrlenq(haystack,needle,n_qwords)                    __libc_memrlenq(haystack,needle,n_qwords)
#define __local_rawmemlen(haystack,needle)                            __libc_rawmemlen(haystack,needle)
#define __local_rawmemlenw(haystack,needle)                           __libc_rawmemlenw(haystack,needle)
#define __local_rawmemlenl(haystack,needle)                           __libc_rawmemlenl(haystack,needle)
#define __local_rawmemlenq(haystack,needle)                           __libc_rawmemlenq(haystack,needle)
#define __local_rawmemrlen(haystack,needle)                           __libc_rawmemrlen(haystack,needle)
#define __local_rawmemrlenw(haystack,needle)                          __libc_rawmemrlenw(haystack,needle)
#define __local_rawmemrlenl(haystack,needle)                          __libc_rawmemrlenl(haystack,needle)
#define __local_rawmemrlenq(haystack,needle)                          __libc_rawmemrlenq(haystack,needle)
#endif /* !__OPTIMIZE_LIBC__ */


__NAMESPACE_STD_BEGIN
#ifndef __std_memcpy_defined
#define __std_memcpy_defined 1
#ifndef __OPTIMIZE_LIBC__
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes);
#else
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes) { return __local_memcpy(__dst,__src,__n_bytes); }
#endif
#endif /* !__std_memcpy_defined */
#ifndef __std_memmove_defined
#define __std_memmove_defined 1
#ifndef __OPTIMIZE_LIBC__
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memmove)(void *__dst, void const *__src, size_t __n_bytes);
#else
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memmove)(void *__dst, void const *__src, size_t __n_bytes) { return __local_memmove(__dst,__src,__n_bytes); }
#endif
#endif /* !__std_memmove_defined */
#ifndef __std_strlen_defined
#define __std_strlen_defined 1
#if defined(_MSC_VER) || (defined(__NO_opt_strlen) || !defined(__OPTIMIZE_LIBC__))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strlen)(char const *__restrict __s);
#ifdef _MSC_VER
#pragma intrinsic(strlen)
#endif
#else /* Builtin... */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strlen)(char const *__restrict __s) { return __opt_strlen(__s); }
#ifdef __opt_strlen_needs_macro
#define strlen(s)  __opt_strlen(s)
#endif /* __opt_strlen_needs_macro */
#endif /* Optimized... */
#endif /* !__std_strlen_defined */
#if defined(__OPTIMIZE_LIBC__) && !defined(_MSC_VER)
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memset)(void *__dst, int __byte, size_t __n_bytes) { return __local_memset(__dst,__byte,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmp)(void const *__a, void const *__b, size_t __n_bytes) { return __local_memcmp(__a,__b,__n_bytes); }
#else /* __OPTIMIZE_LIBC__ */
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memset)(void *__dst, int __byte, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmp)(void const *__a, void const *__b, size_t __n_bytes);
#ifdef _MSC_VER
#pragma intrinsic(memset)
#pragma intrinsic(memcmp)
#endif
#endif /* !__OPTIMIZE_LIBC__ */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strcmp)(char const *__s1, char const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strncmp)(char const *__s1, char const *__s2, size_t __n);
#ifdef _MSC_VER
#pragma intrinsic(strcmp)
#endif
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) char *(__LIBCCALL strstr)(char const *__haystack, char const *__needle);
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
#ifndef __memcpy_defined
#define __memcpy_defined 1
__NAMESPACE_STD_USING(memcpy)
#endif /* !__memcpy_defined */
#ifndef __memmove_defined
#define __memmove_defined 1
__NAMESPACE_STD_USING(memmove)
#endif /* !__memmove_defined */
#ifndef __strlen_defined
#define __strlen_defined 1
__NAMESPACE_STD_USING(strlen)
#endif /* !__strlen_defined */
__NAMESPACE_STD_USING(memset)
__NAMESPACE_STD_USING(memcmp)
__NAMESPACE_STD_USING(strcmp)
__NAMESPACE_STD_USING(strncmp)
__NAMESPACE_STD_USING(strstr)
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strcpy)(char *__restrict __dst, char const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strncpy)(char *__restrict __dst, char const *__restrict __src, size_t __dstsize);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strcat)(char *__restrict __dst, char const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strncat)(char *__restrict __dst, char const *__restrict __src, size_t __max_chars);
__LIBC __WUNUSED __NONNULL((1,2)) int (__LIBCCALL strcoll)(char const *__s1, char const *__s2);
__LIBC __NONNULL((2)) size_t (__LIBCCALL strxfrm)(char *__dst, char const *__restrict __src, size_t __n);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL strcspn)(char const *__s, char const *__reject);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL strspn)(char const *__s, char const *__accept);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) char *(__LIBCCALL strpbrk)(char const *__s, char const *__accept);
__LIBC __NONNULL((2)) char *(__LIBCCALL strtok)(char *__restrict __s, char const *__restrict __delim);
#ifdef _MSC_VER
#pragma intrinsic(strcpy)
#pragma intrinsic(strcat)
#endif
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
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
#endif /* !__CXX_SYSTEM_HEADER */

#if defined(__DOS_COMPAT__) || defined(__OPTIMIZE_LIBC__) || defined(__KERNEL__)
__OPT_LOCAL __NONNULL((1)) void (__LIBCCALL __bzero)(void *__s, size_t __n) { __local_memset(__s,0,__n); }
#else /* __DOS_COMPAT__ */
__REDIRECT_VOID(__LIBC,__NONNULL((1)),__LIBCCALL,__bzero,(void *__s, size_t __n),bzero,(__s,__n))
#endif /* !__DOS_COMPAT__ */
__REDIRECT2(__LIBC,__NONNULL((1,2,3)),char *,__LIBCCALL,__strtok_r,(char *__restrict __s, char const *__restrict __delim, char **__restrict __save_ptr),strtok_r,strtok_s,(__s,__delim,__save_ptr))
#ifdef __USE_POSIX
__REDIRECT_IFDOS(__LIBC,__NONNULL((1,2,3)),char *,__LIBCCALL,strtok_r,(char *__restrict __s, char const *__restrict __delim, char **__restrict __save_ptr),strtok_s,(__s,__delim,__save_ptr))
#endif /* __USE_POSIX */
#ifdef __USE_XOPEN2K
#ifndef __USE_GNU
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,strerror_r,(int __errnum, char *__buf, size_t __buflen),__xpg_strerror_r,(__errnum,__buf,__buflen))
#else /* !__USE_GNU */
#ifdef __CRT_GLC
__LIBC __PORT_NODOS_ALT(strerror) __ATTR_RETNONNULL __NONNULL((2)) char *(__LIBCCALL strerror_r)(int __errnum, char *__buf, size_t __buflen);
#endif /* __CRT_GLC */
#endif /* __USE_GNU */
#endif /* __USE_XOPEN2K */
#endif /* !__KERNEL__ */

__NAMESPACE_STD_BEGIN
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
#ifndef __OPTIMIZE_LIBC__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void *,__LIBCCALL,memchr,(void *__restrict __haystack, int __needle, size_t __n_bytes),memchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void const *,__LIBCCALL,memchr,(void const *__restrict __haystack, int __needle, size_t __n_bytes),memchr,(__haystack,__needle,__n_bytes))
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memchr)(void *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void const *(__LIBCCALL memchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memchr(__haystack,__needle,__n_bytes); }
#endif
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,strchr,(char *__restrict __haystack, int __needle),strchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,strchr,(char const *__restrict __haystack, int __needle),strchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,strrchr,(char *__restrict __haystack, int __needle),strrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,strrchr,(char const *__restrict __haystack, int __needle),strrchr,(__haystack,__needle))
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
#ifdef __OPTIMIZE_LIBC__
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memchr(__haystack,__needle,__n_bytes); }
#endif
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strchr)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strrchr)(char const *__restrict __haystack, int __needle);
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(memchr)
__NAMESPACE_STD_USING(strchr)
__NAMESPACE_STD_USING(strrchr)
#endif /* !__CXX_SYSTEM_HEADER */


#ifdef __USE_KOS
#if !defined(__CRT_KOS) && \
    (defined(__CYG_COMPAT__) || defined(__GLC_COMPAT__))
/* GLibC/Cygwin compatibility (NOTE: Doesn't work in KOS) */
__LOCAL __WUNUSED __ATTR_CONST char const *(__LIBCCALL strerror_s)(int __errnum) {
#ifndef ___sys_errlist_defined
 __LIBC char const *const _sys_errlist[]; __LIBC int _sys_nerr;
#endif
 return (unsigned int)__errnum < (unsigned int)_sys_nerr ? _sys_errlist[__errnum] : __NULLPTR;
}
#elif !defined(__CRT_KOS) && defined(__DOS_COMPAT__)
/* DOS/MSVcrt compatibility (NOTE: Doesn't work in KOS) */
#ifndef __int___sys_errlist_defined
#define __int___sys_errlist_defined 1
__NAMESPACE_INT_BEGIN
__LIBC __WUNUSED __ATTR_CONST char **(__LIBCCALL __sys_errlist)(void);
__LIBC __WUNUSED __ATTR_CONST int *(__LIBCCALL __sys_nerr)(void);
__NAMESPACE_INT_END
#endif /* !__int___sys_errlist_defined */
__LOCAL __WUNUSED __ATTR_CONST char const *(__LIBCCALL strerror_s)(int __errnum) {
 return (unsigned int)__errnum < (unsigned int)*(__NAMESPACE_INT_SYM __sys_nerr)() ?
        (__NAMESPACE_INT_SYM __sys_errlist)()[__errnum] : __NULLPTR;
}
#else
/* KOS direct function implementation. */
__LIBC __WUNUSED __ATTR_CONST char const *(__LIBCCALL strerror_s)(int __errnum);
#endif
#if defined(__CRT_KOS) && !defined(__CYG_COMPAT__) && \
   !defined(__GLC_COMPAT__) && !defined(__DOS_COMPAT__)
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_CONST char const *(__LIBCCALL strerrorname_s)(int __errnum);
#endif /* ... */
#endif /* __USE_KOS */

#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_RETNONNULL char *(__LIBCCALL strerror)(int __errnum);
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(strerror)
#endif /* !__CXX_SYSTEM_HEADER */
#endif /* !__KERNEL__ */

#ifdef __USE_XOPEN2K8
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnlen)(char const *__str, size_t __max_chars);
#ifndef __DOS_COMPAT__
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),char *,__LIBCCALL,__stpcpy,(char *__restrict __dst, char const *__restrict __src),stpcpy,(__dst,__src))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),char *,__LIBCCALL,__stpncpy,(char *__restrict __dst, char const *__restrict __src, size_t __dstsize),stpncpy,(__dst,__src,__dstsize))
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL stpcpy)(char *__restrict __dst, char const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL stpncpy)(char *__restrict __dst, char const *__restrict __src, size_t __dstsize);
#else /* !__DOS_COMPAT__ */
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL stpcpy)(char *__restrict __dst, char const *__restrict __src) { return __NAMESPACE_STD_SYM strcpy(__dst,__src)+__NAMESPACE_STD_SYM strlen(__dst); }
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL stpncpy)(char *__restrict __dst, char const *__restrict __src, size_t __dstsize) { return __NAMESPACE_STD_SYM strncpy(__dst,__src,__dstsize)+__NAMESPACE_STD_SYM strlen(__dst); }
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL __stpcpy)(char *__restrict __dst, char const *__restrict __src) { return stpcpy(__dst,__src); }
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL __stpncpy)(char *__restrict __dst, char const *__restrict __src, size_t __dstsize) { return stpncpy(__dst,__src,__dstsize); }
#endif /* __DOS_COMPAT__ */
#ifndef __KERNEL__
__REDIRECT_IFDOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2,3)),int,__LIBCCALL,strcoll_l,(char const *__s1, char const *__s2, __locale_t __locale),_strcoll_l,(__s1,__s2,__locale))
__REDIRECT_IFDOS(__LIBC,__NONNULL((2)),size_t,__LIBCCALL,strxfrm_l,(char *__dst, char const *__restrict __src, size_t __n, __locale_t __locale),_strxfrm_l,(__dst,__src,__n,__locale))
#ifndef __DOS_COMPAT__
__LIBC __WUNUSED __NONNULL((2)) char *(__LIBCCALL strerror_l)(int __errnum, __locale_t __locale);
__LIBC __PORT_NODOS __WUNUSED __ATTR_RETNONNULL char *(__LIBCCALL strsignal)(int __sig);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL strndup)(char const *__restrict __str, size_t __max_chars);
#else /* !__DOS_COMPAT__ */
__SYSDECL_END
#include "hybrid/malloc.h"
__SYSDECL_BEGIN
__LOCAL __WUNUSED __NONNULL((2)) char *(__LIBCCALL strerror_l)(int __errnum, __locale_t __UNUSED(__locale)) { return strerror(__errnum); }
__LOCAL __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED
__ATTR_MALLOC char *(__LIBCCALL strndup)(char const *__restrict __str, size_t __max_chars) {
    size_t __resultlen = strnlen(__str,__max_chars);
    char *__result = (char *)__hybrid_malloc((__resultlen+1)*sizeof(char));
    if (__result) {
        __NAMESPACE_STD_SYM memcpy(__result,__str,__resultlen*sizeof(char));
        __result[__resultlen] = '\0';
    }
    return __result;
}
#endif /* __DOS_COMPAT__ */
#endif /* !__KERNEL__ */
#endif /* __USE_XOPEN2K8 */

#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8) || defined(__USE_DOS)
__REDIRECT_IFDOS(__LIBC,__SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC __NONNULL((1)),
                 char *,__LIBCCALL,strdup,(char const *__restrict __str),_strdup,(__str))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 || __USE_DOS */
#endif /* !__KERNEL__ */

#ifdef __USE_GNU
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
#if !defined(__DOS_COMPAT__) && !defined(__OPTIMIZE_LIBC__)
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void *,__LIBCCALL,memrchr,(void *__restrict __haystack, int __needle, size_t __n_bytes),memrchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void const *,__LIBCCALL,memrchr,(void const *__restrict __haystack, int __needle, size_t __n_bytes),memrchr,(__haystack,__needle,__n_bytes))
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memrchr)(void *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void const *(__LIBCCALL memrchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrchr(__haystack,__needle,__n_bytes); }
#endif
#ifndef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)),void *,__LIBCCALL,rawmemchr,(void *__restrict __haystack, int __needle),rawmemchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)),void const *,__LIBCCALL,rawmemchr,(void const *__restrict __haystack, int __needle),rawmemchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,strchrnul,(char *__restrict __haystack, int __needle),strchrnul,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,strchrnul,(char const *__restrict __haystack, int __needle),strchrnul,(__haystack,__needle))
#else /* !__DOS_COMPAT__ */
__OPT_LOCAL __WUNUSED __ATTR_RETNONNULL  __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL rawmemchr)(void *__restrict __haystack, int __needle) { return __local_rawmemchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_RETNONNULL  __ATTR_PURE __NONNULL((1)) void const *(__LIBCCALL rawmemchr)(void const *__restrict __haystack, int __needle) { return __local_rawmemchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strchrnul)(char *__restrict __haystack, int __needle) { return __libc_strchrnul(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL strchrnul)(char const *__restrict __haystack, int __needle) { return __libc_strchrnul(__haystack,__needle); }
#endif /* __DOS_COMPAT__ */
#if !defined(basename) && defined(__CRT_GLC)
__REDIRECT(__LIBC,__PORT_NODOS __WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,basename,(char *__restrict __filename),basename,(__filename))
__REDIRECT(__LIBC,__PORT_NODOS __WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,basename,(char const *__restrict __filename),basename,(__filename))
#endif /* !basename && __CRT_GLC */
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
#if !defined(__DOS_COMPAT__) && !defined(__OPTIMIZE_LIBC__)
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memrchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memrchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrchr(__haystack,__needle,__n_bytes); }
#endif
#if !defined(__DOS_COMPAT__) && !defined(__OPTIMIZE_LIBC__)
__LIBC __WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL rawmemchr)(void const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strchrnul)(char const *__restrict __haystack, int __needle);
#else /* !__DOS_COMPAT__ */
__OPT_LOCAL __WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL rawmemchr)(void const *__restrict __haystack, int __needle) { return __local_rawmemchr(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strchrnul)(char const *__restrict __haystack, int __needle) { return __libc_strchrnul(__haystack,__needle); }
#endif /* __DOS_COMPAT__ */
#if !defined(basename) && defined(__CRT_GLC)
__LIBC __PORT_NODOS __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL basename)(char const *__restrict __filename);
#endif /* !basename && __CRT_GLC */
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */

#ifdef __DOS_COMPAT__
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
char *(__LIBCCALL strcasestr)(char const *__haystack, char const *__needle) {
    for (; *__haystack; ++__haystack) {
        if (__libc_strcasecmp(__haystack,__needle) == 0)
            return (char *)__haystack;
    }
    return __NULLPTR;
}
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,3))
void *(__LIBCCALL memmem)(void const *__haystack, size_t __haystacklen,
                          void const *__needle, size_t __needlelen) {
    __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__haystack;
    while (__haystacklen >= __needlelen) {
        if (__NAMESPACE_STD_SYM memcmp(__iter,__needle,__needlelen) == 0)
            return (void *)__iter;
        ++__iter;
    }
    return __NULLPTR;
}
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strverscmp)(char const *__s1, char const *__s2) { return __NAMESPACE_STD_SYM strcmp(__s1,__s2); /* TODO. */ }
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) char *(__LIBCCALL strcasestr)(char const *__haystack, char const *__needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,3)) void *(__LIBCCALL memmem)(void const *__haystack, size_t __haystacklen, void const *__needle, size_t __needlelen);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strverscmp)(char const *__s1, char const *__s2);
#endif /* !__DOS_COMPAT__ */


#if defined(__OPTIMIZE_LIBC__) || defined(__DOS_COMPAT__)
__OPT_LOCAL __NONNULL((1,2)) void *(__LIBCCALL __mempcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n) { return __local_mempcpy(__dst,__src,__n); }
__OPT_LOCAL __NONNULL((1,2)) void *(__LIBCCALL mempcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n) { return __local_mempcpy(__dst,__src,__n); }
#else /* __DOS_COMPAT__ */
__REDIRECT(__LIBC,__NONNULL((1,2)),void *,__LIBCCALL,__mempcpy,(void *__restrict __dst, void const *__restrict __src, size_t __n),mempcpy,(__dst,__src,__n))
__LIBC __NONNULL((1,2)) void *(__LIBCCALL mempcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n);
#endif /* !__DOS_COMPAT__ */

#ifndef __KERNEL__
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __NONNULL((1)) char *(__LIBCCALL strfry)(char *__str);
__LIBC __PORT_NODOS __NONNULL((1)) void *(__LIBCCALL memfrob)(void *__s, size_t __n);
#endif /* __CRT_GLC */
__REDIRECT_IFDOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2,3)),int,__LIBCCALL,strcasecmp_l,(char const *__s1, char const *__s2, __locale_t __locale),_stricmp_l,(__s1,__s2,__locale))
__REDIRECT_IFDOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2,4)),int,__LIBCCALL,strncasecmp_l,(char const *__s1, char const *__s2, size_t __n, __locale_t __locale),_strnicmp_l,(__s1,__s2,__n,__locale))
#endif /* !__KERNEL__ */
#endif /* __USE_GNU */

#ifdef __USE_MISC
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __NONNULL((1,2)) char *(__LIBCCALL strsep)(char **__restrict __stringp, char const *__restrict __delim);
#endif /* __CRT_GLC */
#ifndef __bstring_defined
#define __bstring_defined 1
#if defined(__DOS_COMPAT__) || defined(__KERNEL__)
__OPT_LOCAL __NONNULL((1,2)) void (__LIBCCALL bcopy)(void const *__src, void *__dst, size_t __n) { __local_memmove(__dst,__src,__n); }
__OPT_LOCAL __NONNULL((1)) void (__LIBCCALL bzero)(void *__s, size_t __n) { __local_memset(__s,0,__n); }
#else /* __DOS_COMPAT__ */
__LIBC __NONNULL((1,2)) void (__LIBCCALL bcopy)(void const *__src, void *__dst, size_t __n);
__LIBC __NONNULL((1)) void (__LIBCCALL bzero)(void *__s, size_t __n);
#endif /* !__DOS_COMPAT__ */
#ifndef __OPTIMIZE_LIBC__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,bcmp,
          (void const *__s1, void const *__s2, size_t __n),memcmp,(__s1,__s2,__n))
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL bcmp)(void const *__s1, void const *__s2, size_t __n) { return __local_memcmp(__s1,__s2,__n); }
#endif
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

#ifdef __DOS_COMPAT__
#define ____IMPL_DO_FFS(i) \
{ int __result; \
  if (!i) return 0; \
  for (__result = 1; !(i&1); ++__result) i >>= 1; \
  return __result; \
}
#endif /* __DOS_COMPAT__ */

#ifdef __USE_GNU
#ifdef __DOS_COMPAT__
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL ffsl)(long __i) ____IMPL_DO_FFS(__i)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL ffsll)(__LONGLONG __i) ____IMPL_DO_FFS(__i)
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffsl)(long __i);
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffsll)(__LONGLONG __i);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_GNU */

#ifndef __ffs_defined
#define __ffs_defined 1
#ifdef __DOS_COMPAT__
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
#else /* __DOS_COMPAT__ */
#ifdef __USE_KOS
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs8)(__INT8_TYPE__ __i);
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs16)(__INT16_TYPE__ __i);
#if __SIZEOF_INT__ == 4
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST,int,__LIBCCALL,__ffs32,(__INT32_TYPE__ __i),ffs,(__i))
#else /* __SIZEOF_INT__ == 4 */
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs32)(__INT32_TYPE__ __i);
#endif /* __SIZEOF_INT__ != 4 */
#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST,int,__LIBCCALL,__ffs64,(__INT64_TYPE__ __i),ffsl,(__i))
#elif __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST,int,__LIBCCALL,__ffs64,(__INT64_TYPE__ __i),ffsll,(__i))
#else /* ... */
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL __ffs64)(__INT64_TYPE__ __i);
#endif /* !... */
#define ffs(i) (sizeof(i) == 4 ? __ffs32((__INT32_TYPE__)(i)) : sizeof(i) == 8 ? __ffs64((__INT64_TYPE__)(i)) : \
                sizeof(i) == 2 ? __ffs16((__INT16_TYPE__)(i)) : __ffs8((__INT8_TYPE__)(i)))
#else /* __USE_KOS */
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffs)(int __i);
#endif /* !__USE_KOS */
#endif /* !__DOS_COMPAT__ */
#endif /* !__ffs_defined */
#undef ____IMPL_DO_FFS

#ifndef __strcasecmp_defined
#define __strcasecmp_defined 1
__REDIRECT_IFDOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strcasecmp,(char const *__s1, char const *__s2),_stricmp,(__s1,__s2))
__REDIRECT_IFDOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strncasecmp,(char const *__s1, char const *__s2, size_t __n),_strnicmp,(__s1,__s2,__n))
#endif /* !__strcasecmp_defined */
#endif /* __USE_MISC */

#ifndef __KERNEL__
#if defined(__USE_MISC) || defined(__USE_XOPEN)
__REDIRECT_IFDOS(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,memccpy,(void *__restrict __dst, void const *__restrict __src, int __c, size_t __n),_memccpy,(__dst,__src,__c,__n))
#endif /* __USE_MISC || __USE_XOPEN */
#endif /* !__KERNEL__ */

#ifdef __USE_KOS
/* KOS String extensions. */
#if defined(__OPTIMIZE_LIBC__) || !defined(__CRT_KOS) || \
   (defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__))
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlen)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memlen(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlen)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrlen(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlen)(void const *__restrict __haystack, int __needle) { return __local_rawmemlen(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlen)(void const *__restrict __haystack, int __needle) { return __local_rawmemrlen(__haystack,__needle); }
#else
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlen)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlen)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlen)(void const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlen)(void const *__restrict __haystack, int __needle);
#endif

#if !defined(__CRT_KOS) || defined(__DOS_COMPAT__) || \
     defined(__GLC_COMPAT__) || defined(__CYG_COMPAT__)
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL stroff)(char const *__restrict __haystack, int __needle) { return __libc_stroff(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strroff)(char const *__restrict __haystack, int __needle) { return __libc_strroff(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnoff)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnoff(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnroff)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnroff(__haystack,__needle,__max_chars); }

#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strend)(char *__restrict __str) { return __libc_strend(__str); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strend)(char const *__restrict __str) { return __libc_strend(__str); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnend)(char *__restrict __str, size_t __max_chars) { return __libc_strnend(__str,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strnend)(char const *__restrict __str, size_t __max_chars) { return __libc_strnend(__str,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strrchrnul)(char *__restrict __haystack, int __needle) { return __libc_strrchrnul(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strrchrnul)(char const *__restrict __haystack, int __needle) { return __libc_strrchrnul(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnchr)(char *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL strnchr)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnrchr)(char *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL strnrchr)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnchrnul)(char *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strnchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnrchrnul)(char *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strnrchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchrnul(__haystack,__needle,__max_chars); }
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strend)(char const *__str) { return __libc_strend(__str); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnend)(char const *__str, size_t __max_chars) { return __libc_strnend(__str,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strrchrnul)(char const *__restrict __haystack, int __needle) { return __libc_strrchrnul(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnchr)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnrchr)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnrchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchrnul(__haystack,__needle,__max_chars); }
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
#else /* Emulate extensions... */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL stroff)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strroff)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnoff)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnroff)(char const *__restrict __haystack, int __needle, size_t __max_chars);
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strend,(char *__restrict __str),strend,(__str))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strend,(char const *__restrict __str),strend,(__str))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strnend,(char *__restrict __str, size_t __max_chars),strnend,(__str,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strnend,(char const *__restrict __str, size_t __max_chars),strnend,(__str,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strrchrnul,(char *__restrict __haystack, int __needle),strrchrnul,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strrchrnul,(char const *__restrict __haystack, int __needle),strrchrnul,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,strnchr,(char *__restrict __haystack, int __needle, size_t __max_chars),strnchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,strnchr,(char const *__restrict __haystack, int __needle, size_t __max_chars),strnchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,strnrchr,(char *__restrict __haystack, int __needle, size_t __max_chars),strnrchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,strnrchr,(char const *__restrict __haystack, int __needle, size_t __max_chars),strnrchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strnchrnul,(char *__restrict __haystack, int __needle, size_t __max_chars),strnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strnchrnul,(char const *__restrict __haystack, int __needle, size_t __max_chars),strnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strnrchrnul,(char *__restrict __haystack, int __needle, size_t __max_chars),strnrchrnul,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strnrchrnul,(char const *__restrict __haystack, int __needle, size_t __max_chars),strnrchrnul,(__haystack,__needle,__max_chars))
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strend)(char const *__restrict __str);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnend)(char const *__restrict __str, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strrchrnul)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnchr)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnrchr)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnrchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars);
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
#endif /* !Emulate extensions... */

#if defined(__OPTIMIZE_LIBC__) || !defined(__CRT_KOS) || \
    defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__)
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL rawmemrchr)(void *__restrict __haystack, int __needle) { return __local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void const *(__LIBCCALL rawmemrchr)(void const *__restrict __haystack, int __needle) { return __local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memend)(void *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void const *(__LIBCCALL memend)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrend)(void *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void const *(__LIBCCALL memrend)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrend(__haystack,__needle,__n_bytes); }
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL rawmemrchr)(void const *__restrict __haystack, int __needle) { return __local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memend)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrend)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrend(__haystack,__needle,__n_bytes); }
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
#else /* Emulate extensions... */
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,rawmemrchr,(void *__restrict __haystack, int __needle),rawmemrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void const *,__LIBCCALL,rawmemrchr,(void const *__restrict __haystack, int __needle),rawmemrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,memend,(void *__restrict __haystack, int __needle, size_t __n_bytes),memend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void const *,__LIBCCALL,memend,(void const *__restrict __haystack, int __needle, size_t __n_bytes),memend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,memrend,(void *__restrict __haystack, int __needle, size_t __n_bytes),memrend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void const *,__LIBCCALL,memrend,(void const *__restrict __haystack, int __needle, size_t __n_bytes),memrend,(__haystack,__needle,__n_bytes))
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL rawmemrchr)(void const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memend)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrend)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
#endif /* !Emulate extensions... */

#ifndef __KERNEL__
#if defined(__CRT_GLC) && defined(__GLC_COMPAT__)
/* Implement using `asprintf' */
__REDIRECT(__LIBC,,__ssize_t,__LIBCCALL,__libc_vasprintf,(char **__restrict __pstr, char const *__restrict __format, __builtin_va_list __args),vasprintf,(__ptr,__format,__args))
__LOCAL __ATTR_LIBC_PRINTF(1,0) __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC
char *(__LIBCCALL vstrdupf)(char const *__restrict __format, __builtin_va_list __args) {
    char *__result;
    return __libc_vasprintf(&__result,__format,__args) >= 0 ? __result : 0;
}
__LOCAL __ATTR_LIBC_PRINTF(1,2) __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC
char *(__ATTR_CDECL strdupf)(char const *__restrict __format, ...) {
    char *__result; __builtin_va_list __args; __builtin_va_start(__args,__format);
    __result = vstrdupf(__format,__args);
    __builtin_va_end(__args);
    return __result;
}
#elif defined(__CRT_DOS) && defined(__DOS_COMPAT__)
/* Implement using scprintf()+malloc()+sprintf() */
__SYSDECL_END
#include "hybrid/malloc.h"
__SYSDECL_BEGIN
#ifndef ____dos_vscprintf_defined
#define ____dos_vscprintf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_vscprintf,(char const *__restrict __format, __builtin_va_list __args),_vscprintf,(__format,__args))
#endif /* !____dos_vsnprintf_defined */
#ifndef ____libc_vsprintf_defined
#define ____libc_vsprintf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__libc_vsprintf,(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args),vsprintf,(__buf,__format,__args))
#endif /* !____libc_vsprintf_defined */
__LOCAL __ATTR_LIBC_PRINTF(1,0) __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC
char *(__LIBCCALL vstrdupf)(char const *__restrict __format, __builtin_va_list __args) {
    int __resultlen = __dos_vscprintf(__format,__args);
    char *__result = __resultlen >= 0 ? (char *)__hybrid_malloc((__resultlen+1)*sizeof(char)) : 0;
    if (__result) __libc_vsprintf(__result,__format,__args);
    return __result;
}
__LOCAL __ATTR_LIBC_PRINTF(1,2) __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC
char *(__ATTR_CDECL strdupf)(char const *__restrict __format, ...) {
    char *__result; __builtin_va_list __args; __builtin_va_start(__args,__format);
    __result = vstrdupf(__format,__args);
    __builtin_va_end(__args);
    return __result;
}
#else /* ... */
/* Use actual functions exported from libc. */
__LIBC __ATTR_LIBC_PRINTF(1,2) __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__ATTR_CDECL strdupf)(char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(1,0) __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL vstrdupf)(char const *__restrict __format, __builtin_va_list __args);
#endif /* !... */
#endif /* !__KERNEL__ */

/* byte/word/dword-wise string operations. */
#ifndef __OPTIMIZE_LIBC__
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,memcpyb,(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes),memcpy,(__dst,__src,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1))  ,void *,__LIBCCALL,memsetb,(void *__restrict __dst, int __byte, size_t __n_bytes),memset,(__dst,__byte,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int   ,__LIBCCALL,memcmpb,(void const *__a, void const *__b, size_t __n_bytes),memcmp,(__a,__b,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,memmoveb,(void *__dst, void const *__src, size_t __n_bytes),memmove,(__dst,__src,__n_bytes))
#else
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyb)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes) { return __local_memcpy(__dst,__src,__n_bytes); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetb)(void *__restrict __dst, int __byte, size_t __n_bytes) { return __local_memset(__dst,__byte,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int   (__LIBCCALL memcmpb)(void const *__a, void const *__b, size_t __n_bytes) { return __local_memcmp(__a,__b,__n_bytes); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmoveb)(void *__dst, void const *__src, size_t __n_bytes) { return __local_memmove(__dst,__src,__n_bytes); }
#endif
#if defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyw)(void *__restrict __dst, void const *__restrict __src, size_t __n_words);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyl)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyq)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, size_t __n_words);
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, size_t __n_dwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetq)(void *__restrict __dst, __UINT64_TYPE__ __qword, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpw)(void const *__a, void const *__b, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpl)(void const *__a, void const *__b, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpq)(void const *__a, void const *__b, size_t __n_qwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmovew)(void *__dst, void const *__src, size_t __n_words);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmovel)(void *__dst, void const *__src, size_t __n_dwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmoveq)(void *__dst, void const *__src, size_t __n_qwords);
#ifdef __USE_GNU
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,mempcpyb,(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes),mempcpy,(__dst,__src,__n_bytes))
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyw)(void *__restrict __dst, void const *__restrict __src, size_t __n_words);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyl)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyq)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords);
#endif /* __USE_GNU */
#else /* Builtin... */
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyw)(void *__restrict __dst, void const *__restrict __src, size_t __n_words) { return __local_memcpyw(__dst,__src,__n_words); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyl)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords) { return __local_memcpyl(__dst,__src,__n_dwords); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyq)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords) { return __local_memcpyq(__dst,__src,__n_qwords); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, size_t __n_words) { return __local_memsetw(__dst,__word,__n_words); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, size_t __n_dwords) { return __local_memsetl(__dst,__dword,__n_dwords); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetq)(void *__restrict __dst, __UINT64_TYPE__ __qword, size_t __n_qwords) { return __local_memsetq(__dst,__qword,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpw)(void const *__a, void const *__b, size_t __n_words) { return __local_memcmpw(__a,__b,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpl)(void const *__a, void const *__b, size_t __n_dwords) { return __local_memcmpl(__a,__b,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpq)(void const *__a, void const *__b, size_t __n_qwords) { return __local_memcmpq(__a,__b,__n_qwords); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmovew)(void *__dst, void const *__src, size_t __n_words) { return __local_memmovew(__dst,__src,__n_words); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmovel)(void *__dst, void const *__src, size_t __n_dwords) { return __local_memmovel(__dst,__src,__n_dwords); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmoveq)(void *__dst, void const *__src, size_t __n_qwords) { return __local_memmoveq(__dst,__src,__n_qwords); }
#ifdef __USE_GNU
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyb)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes) { return __local_mempcpy(__dst,__src,__n_bytes); }
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyw)(void *__restrict __dst, void const *__restrict __src, size_t __n_words) { return __local_mempcpyw(__dst,__src,__n_words); }
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyl)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords) { return __local_mempcpyl(__dst,__src,__n_dwords); }
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyq)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords) { return __local_mempcpyq(__dst,__src,__n_qwords); }
#endif /* __USE_GNU */
#endif /* Compat... */


/* Similar to memset(), but fill memory using the given pattern:
 * >> mempatb(addr,0x12,7):
 *    Same as regular memset().
 * >> mempatw(addr,0x12fd,7):
 *    addr&1 == 0: 12fd12fd12fd12
 *    addr&1 == 1:   fd12fd12fd1212
 *    >> `*byte = (__pattern >> 8*((uintptr_t)byte & 0x2)) & 0xff;'
 * >> mempatl(addr,0x12345678,11):
 *    addr&3 == 0: 12345678123
 *    addr&3 == 1:   34567812312
 *    addr&3 == 2:     56781231234
 *    addr&3 == 3:       78123123456
 *    >> `*byte = (__pattern >> 8*((uintptr_t)byte & 0x3)) & 0xff;'
 * WARNING: PATTERN is encoded in host endian, meaning that
 *          byte-order is reversed on little-endian machines. */
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,mempatb,(void *__restrict __dst, int __pattern, size_t __n_bytes),memset,(__dst,__pattern,__n_bytes))
#if defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL mempatw)(void *__restrict __dst, __UINT16_TYPE__ __pattern, size_t __n_bytes);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL mempatl)(void *__restrict __dst, __UINT32_TYPE__ __pattern, size_t __n_bytes);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL mempatq)(void *__restrict __dst, __UINT64_TYPE__ __pattern, size_t __n_bytes);
#else /* Builtin... */
__LOCAL __ATTR_RETNONNULL __NONNULL((1))
void *(__LIBCCALL mempatw)(void *__restrict __dst, __UINT16_TYPE__ __pattern, size_t __n_bytes) {
    __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__dst;
    if (__n_bytes && (__UINTPTR_TYPE__)__iter & 1) {
        *__iter = ((__UINT8_TYPE__ *)&__pattern)[1];
        ++__iter,--__n_bytes;
    }
    memsetw(__iter,__pattern,__n_bytes/2);
    __iter += __n_bytes,__n_bytes &= 1;
    if (__n_bytes) *__iter = ((__UINT8_TYPE__ *)&__pattern)[0];
    return __dst;
}
__LOCAL __ATTR_RETNONNULL __NONNULL((1))
void *(__LIBCCALL mempatl)(void *__restrict __dst, __UINT32_TYPE__ __pattern, size_t __n_bytes) {
    __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__dst;
    while (__n_bytes && (__UINTPTR_TYPE__)__iter & 3) {
        *__iter = ((__UINT8_TYPE__ *)&__pattern)[(__UINTPTR_TYPE__)__iter & 3];
        ++__iter,--__n_bytes;
    }
    memsetl(__iter,__pattern,__n_bytes/4);
    __iter += __n_bytes,__n_bytes &= 3;
    while (__n_bytes) {
        *__iter = ((__UINT8_TYPE__ *)&__pattern)[(__UINTPTR_TYPE__)__iter & 3];
        ++__iter,--__n_bytes;
    }
    return __dst;
}
__LOCAL __ATTR_RETNONNULL __NONNULL((1))
void *(__LIBCCALL mempatq)(void *__restrict __dst, __UINT64_TYPE__ __pattern, size_t __n_bytes) {
    __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__dst;
    while (__n_bytes && (__UINTPTR_TYPE__)__iter & 7) {
        *__iter = ((__UINT8_TYPE__ *)&__pattern)[(__UINTPTR_TYPE__)__iter & 7];
        ++__iter,--__n_bytes;
    }
    memsetq(__iter,__pattern,__n_bytes/8);
    __iter += __n_bytes,__n_bytes &= 7;
    while (__n_bytes) {
        *__iter = ((__UINT8_TYPE__ *)&__pattern)[(__UINTPTR_TYPE__)__iter & 7];
        ++__iter,--__n_bytes;
    }
    return __dst;
}
#endif /* Compat... */

#if !defined(__OPTIMIZE_LIBC__) && defined(__CRT_KOS) && \
   (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,memlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memlen,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,memrlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memrlen,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,rawmemlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemlen,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,rawmemrlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemrlen,(__haystack,__needle))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle);
#else /* Builtin... */
/* Compatibility/optimized multibyte memory functions. */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memlen(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrlen(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return __local_rawmemlen(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return __local_rawmemrlen(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memlenw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memlenl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memlenq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrlenw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrlenl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrlenq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemlenw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemlenl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemlenq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemrlenw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemrlenl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemrlenq(__haystack,__needle); }
#endif /* Compat... */

#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
#ifndef __OPTIMIZE_LIBC__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memchrb,(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes),memchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memchr,(__haystack,__needle,__n_bytes))
#else /* !__OPTIMIZE_LIBC__ */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memchrb)(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memchr(__haystack,__needle,__n_bytes); }
#endif /* __OPTIMIZE_LIBC__ */
#if !defined(__OPTIMIZE_LIBC__) && defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrchrb,(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes),memrchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memrchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memrchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemchrb,(__UINT8_TYPE__ *__restrict __haystack, int __needle),rawmemchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,rawmemchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemchr,(__haystack,__needle))
#else /* GLC... */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrchrb)(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memrchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemchrb)(__UINT8_TYPE__ *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL rawmemchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemchr(__haystack,__needle); }
#endif /* !GLC... */
#if !defined(__OPTIMIZE_LIBC__) && defined(__CRT_KOS) && \
   (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memchrw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memchrw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memchrl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memchrl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_dwords),memchrq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_dwords),memchrq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memrchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memrchrw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memrchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memrchrw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memrchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memrchrl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memrchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memrchrl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memrchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memrchrq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memrchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memrchrq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,rawmemchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle),rawmemchrw,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,rawmemchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle),rawmemchrw,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,rawmemchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle),rawmemchrl,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,rawmemchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle),rawmemchrl,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,rawmemchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle),rawmemchrq,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,rawmemchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle),rawmemchrq,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemrchrb,(__UINT8_TYPE__ *__restrict __haystack, int __needle),rawmemrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,rawmemrchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,rawmemrchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle),rawmemrchrw,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,rawmemrchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle),rawmemrchrw,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,rawmemrchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle),rawmemrchrl,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,rawmemrchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle),rawmemrchrl,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,rawmemrchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle),rawmemrchrq,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,rawmemrchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle),rawmemrchrq,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memendb,(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes),memend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memendb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memendw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memendw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memendw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memendw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memendl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memendl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memendl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memendl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memendq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memendq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memendq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memendq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrendb,(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes),memrend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memrendb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memrend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memrendw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memrendw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memrendw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memrendw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memrendl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memrendl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memrendl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memrendl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memrendq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memrendq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memrendq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memrendq,(__haystack,__needle,__n_qwords))
#else /* KOS... */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memchrw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memchrl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrchrw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrchrl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemrchrb)(__UINT8_TYPE__ *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL rawmemrchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemrchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemrchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemrchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemrchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memendb)(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memendb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrendb)(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memrendb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memendw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memendl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrendw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memrendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrendl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memrendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memchrq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL memchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrchrq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL memrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemchrq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL rawmemchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemrchrq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemrchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL rawmemrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemrchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memendq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memendq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL memendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memendq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrendq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrendq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL memrendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrendq(__haystack,__needle,__n_qwords); }
#endif /* !KOS... */
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
#ifndef __OPTIMIZE_LIBC__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memchr,(__haystack,__needle,__n_bytes))
#else /* !__OPTIMIZE_LIBC__ */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memchr(__haystack,__needle,__n_bytes); }
#endif /* __OPTIMIZE_LIBC__ */
#if !defined(__OPTIMIZE_LIBC__) && defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memrchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemchr,(__haystack,__needle))
#else /* GLC... */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemchr(__haystack,__needle); }
#endif /* !GLC... */
#if !defined(__OPTIMIZE_LIBC__) && defined(__CRT_KOS) && \
   (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemrchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemrchr,(__haystack,__needle))
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memendb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrendb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memrend,(__haystack,__needle,__n_bytes))
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemrchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemrchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemrchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemrchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memendb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrendb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memendq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrendq(__haystack,__needle,__n_qwords); }
#endif
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */


#ifdef __CRT_KOS

/* Fuzzy string compare extensions.
 *  - Lower return values indicate more closely matching data.
 *  - ZERO(0) indicates perfectly matching data. */
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_strcmp)(char const *__a, char const *__b);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcmp)(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_strncmp)(char const *__a, size_t __max_a_chars, char const *__b, size_t __max_b_chars);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_strcasecmp)(char const *__a, char const *__b);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcasecmp)(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_strncasecmp)(char const *__a, size_t __max_a_chars, char const *__b, size_t __max_b_chars);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_strcasecmp_l)(char const *__a, char const *__b, __locale_t __locale);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcasecmp_l)(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes, __locale_t __locale);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_strncasecmp_l)(char const *__a, size_t __max_a_chars, char const *__b, size_t __max_b_chars, __locale_t __locale);
/* Perform a wildcard string comparison, returning ZERO(0) upon match, or non-zero when not. */
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wildstrcmp)(char const *__pattern, char const *__string);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wildstrcasecmp)(char const *__pattern, char const *__string);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wildstrcasecmp_l)(char const *__pattern, char const *__string, __locale_t __locale);
#endif /* __CRT_KOS */

#if !defined(__GLC_COMPAT__) && !defined(__CYG_COMPAT__)
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,memcasecmp,
          (void const *__a, void const *__b, size_t __n_bytes),_memicmp,(__a,__b,__n_bytes))
#ifndef __KERNEL__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,memcasecmp_l,
          (void const *__a, void const *__b, size_t __n_bytes, __locale_t __locale),_memicmp_l,(__a,__b,__n_bytes,__locale))
#endif /* !__KERNEL__ */
#else /* !__GLC_COMPAT__ */
#ifndef ____libc_tolower_defined
#define ____libc_tolower_defined 1
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_tolower,(int __c),tolower,(__c))
#endif /* !____libc_tolower_defined */
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
int (__LIBCCALL memcasecmp)(void const *__a, void const *__b, size_t __n_bytes) {
    __BYTE_TYPE__ *__ai = (__BYTE_TYPE__ *)__a,*__bi = (__BYTE_TYPE__ *)__b; int __temp;
    while (__n_bytes--) if ((__temp = __libc_tolower(*__ai++) - __libc_tolower(*__bi++)) != 0) return __temp;
    return 0;
}
#ifndef __KERNEL__
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
int (__LIBCCALL memcasecmp_l)(void const *__a, void const *__b, size_t __n_bytes,
                              __locale_t __UNUSED(__locale)) {
    return memcasecmp(__a,__b,__n_bytes);
}
#endif /* !__KERNEL__ */
#endif /* __GLC_COMPAT__ */

#ifndef __KERNEL__

#ifdef __DOS_COMPAT__
__SYSDECL_END
#include <__amalloc.h>
__SYSDECL_BEGIN
#ifndef ____dos_vscprintf_defined
#define ____dos_vscprintf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_vscprintf,(char const *__restrict __format, __builtin_va_list __args),_vscprintf,(__format,__args))
#endif /* !____dos_vsnprintf_defined */
#ifndef ____libc_vsprintf_defined
#define ____libc_vsprintf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__libc_vsprintf,(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args),vsprintf,(__buf,__format,__args))
#endif /* !____libc_vsprintf_defined */

#ifndef __NO_ASMNAME
__LIBC __ATTR_LIBC_PRINTF(1,2) int (__ATTR_CDECL __dos_scprintf)(char const *__restrict __format, ...) __ASMNAME("_scprintf");
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL __libc_sprintf)(char *__restrict __buf, char const *__restrict __format, ...) __ASMNAME("sprintf");
#else /* !__NO_ASMNAME */
#define __dos_scprintf(...) _scprintf(__VA_ARGS__)
#define __libc_sprintf(...) __NAMESPACE_STD_SYM sprintf(__VA_ARGS__)
#ifndef ___scprintf_defined
#define ___scprintf_defined 1
__LIBC __ATTR_LIBC_PRINTF(1,2) int (__ATTR_CDECL _scprintf)(char const *__restrict __format, ...);
#endif /* !___scprintf_defined */
#ifndef __std_sprintf_defined
#define __std_sprintf_defined
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL sprintf)(char *__restrict __buf, char const *__restrict __format, ...);
__NAMESPACE_STD_END
#endif /* !__std_sprintf_defined */
#ifndef __sprintf_defined
#define __sprintf_defined
__NAMESPACE_STD_USING(sprintf)
#endif /* !__sprintf_defined */
#endif /* __NO_ASMNAME */
__LOCAL __ATTR_LIBC_PRINTF(2,3) char *(__ATTR_CDECL __forward_sprintf)(char *__restrict __buf, char const *__restrict __format, ...) { __builtin_va_list __args; __builtin_va_start(__args,__format); __libc_vsprintf(__buf,__format,__args); __builtin_va_end(__args); return __buf; }
__LOCAL __ATTR_LIBC_PRINTF(2,0) char *(__LIBCCALL __forward_vsprintf)(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args) { __libc_vsprintf(__buf,__format,__args); return __buf; }

/* Without dedicated Libc functionality, double-evaluation can't be prevented. */
#define strdupaf(...) \
      __forward_sprintf((char *)__ALLOCA(((size_t)__dos_scprintf(__VA_ARGS__)+1)*sizeof(char)),__VA_ARGS__)
#ifdef __NO_XBLOCK
#define vstrdupaf(format,args) \
      __forward_vsprintf((char *)__ALLOCA(((size_t)__dos_vscprintf(format,args)+1)*sizeof(char)),format,args)
#else /* __NO_XBLOCK */
#define vstrdupaf(format,args) \
  __XBLOCK({ char const *const __format = (format); \
             __builtin_va_list __args = (args); \
             __XRETURN __forward_vsprintf((char *)__ALLOCA(((size_t) \
                       __dos_vscprintf(__format,__args)+1)*sizeof(char)),__format,__args); \
  })
#endif /* !__NO_XBLOCK */

#elif defined(__GLC_COMPAT__) || defined(__CYG_COMPAT__)
__SYSDECL_END
#include <hybrid/alloca.h>
__SYSDECL_BEGIN

#ifndef ____libc_vsprintf_defined
#define ____libc_vsprintf_defined 1
__REDIRECT(__LIBC,__ATTR_LIBC_PRINTF(2,0),int,__LIBCCALL,__libc_vsprintf,(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args),vsprintf,(__buf,__format,__args))
#endif /* !____libc_vsprintf_defined */

#ifndef ____libc_vsnprintf_defined
#define ____libc_vsnprintf_defined 1
__REDIRECT(__LIBC,__ATTR_LIBC_PRINTF(3,0),int,__LIBCCALL,__libc_vsnprintf,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args),vsnprintf,(__buf,__buflen,__format,__args))
#endif /* !____libc_vsnprintf_defined */

#ifndef __NO_ASMNAME
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL __libc_sprintf)(char *__restrict __buf, char const *__restrict __format, ...) __ASMNAME("sprintf");
__LIBC __ATTR_LIBC_PRINTF(3,0) __ssize_t (__ATTR_CDECL __libc_snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) __ASMNAME("snprintf");
#else /* !__NO_ASMNAME */
#define __libc_sprintf(...)   __NAMESPACE_STD_SYM sprintf(__VA_ARGS__)
#define __libc_snprintf(...)  __NAMESPACE_STD_SYM snprintf(__VA_ARGS__)
#ifndef __std_sprintf_defined
#define __std_sprintf_defined
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL sprintf)(char *__restrict __buf, char const *__restrict __format, ...);
__NAMESPACE_STD_END
#endif /* !__std_sprintf_defined */
#ifndef __std_snprintf_defined
#define __std_snprintf_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_LIBC_PRINTF(3,4) __ssize_t (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...);
__NAMESPACE_STD_END
#endif /* !__std_snprintf_defined */
#ifndef __sprintf_defined
#define __sprintf_defined
__NAMESPACE_STD_USING(sprintf)
#endif /* !__sprintf_defined */
#ifndef __snprintf_defined
#define __snprintf_defined 1
__NAMESPACE_STD_USING(snprintf)
#endif /* !__snprintf_defined */
#endif /* __NO_ASMNAME */

__LOCAL __ATTR_LIBC_PRINTF(2,3) char *(__ATTR_CDECL __forward_sprintf)(char *__restrict __buf, char const *__restrict __format, ...) { __builtin_va_list __args; __builtin_va_start(__args,__format); __libc_vsprintf(__buf,__format,__args); __builtin_va_end(__args); return __buf; }
__LOCAL __ATTR_LIBC_PRINTF(2,0) char *(__LIBCCALL __forward_vsprintf)(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args) { __libc_vsprintf(__buf,__format,__args); return __buf; }

/* Without dedicated Libc functionality, double-evaluation can't be prevented. */
#define strdupaf(...) \
      __forward_sprintf((char *)__ALLOCA(((size_t)__libc_snprintf(NULL,0,__VA_ARGS__)+1)*sizeof(char)),__VA_ARGS__)
#ifdef __NO_XBLOCK
#define vstrdupaf(format,args) \
      __forward_vsprintf((char *)__ALLOCA(((size_t)__libc_snprintf(NULL,0,format,args)+1)*sizeof(char)),format,args)
#else /* __NO_XBLOCK */
#define vstrdupaf(format,args) \
  __XBLOCK({ char const *const __format = (format); \
             __builtin_va_list __args = (args); \
             __XRETURN __forward_vsprintf((char *)__ALLOCA(((size_t) \
                       __libc_snprintf(NULL,0,__format,__args)+1)*sizeof(char)),__format,__args); \
  })
#endif /* !__NO_XBLOCK */

#else /* Compat... */
/* >> char *strdupaf(char const *__restrict format, ...);
 * String duplicate as fu$k!
 * Similar to strdupf, but allocates memory of the stack, instead of the heap.
 * While this function is _very_ useful, be warned that due to the way variadic
 * arguments are managed by cdecl (the only calling convention possible to use
 * for them on most platforms) it is nearly impossible not to waste the stack
 * space that was originally allocated for the arguments (Because in cdecl, the
 * callee is responsible for argument cleanup).
 * ANYWAYS: Since its the stack, it shouldn't really matter, but please be advised
 *          that use of these functions fall under the same restrictions as all
 *          other alloca-style functions.
 * >> int open_file_in_folder(char const *folder, char const *file) {
 * >>   return open(strdupaf("%s/%s",folder,file),O_RDONLY);
 * >> }
 */
__LIBC __ATTR_LIBC_PRINTF(1,2) __WUNUSED __ATTR_MALLOC char *(__ATTR_CDECL strdupaf)(char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(1,0) __WUNUSED __ATTR_MALLOC char *(__LIBCCALL vstrdupaf)(char const *__restrict __format, __builtin_va_list __args);

#ifdef __INTELLISENSE__
#elif defined(__GNUC__)
/* Dear GCC devs: WHY IS THERE NO `__attribute__((alloca))'?
 * Or better yet! Add something like: `__attribute__((clobber("%esp")))'
 *
 * Here's what the hacky code below does:
 * We must use `__builtin_alloca' to inform the compiler that the stack pointer
 * contract has been broken, meaning that %ESP can (no longer) be used for offsets.
 * NOTE: If you don't believe me that this is required, and think this is just me
 *       ranting about missing GCC functionality, try the following code yourself:
 * >> printf("path = `%s'\n",strdupaf("%s/%s","/usr","lib")); // OK (Also try cloning this line a bunch of times)
 * >> #undef strdupaf
 * >> printf("path = `%s'\n",strdupaf("%s/%s","/usr","lib")); // Breaks
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
#endif /* Builtin... */
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */

#ifdef __USE_DEBUG
#include "hybrid/debuginfo.h"
#if __USE_DEBUG != 0 && defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpy_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes, __DEBUGINFO);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(_memcpy_d)
#ifdef __USE_GNU
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _mempcpy_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes, __DEBUGINFO);
#endif /* __USE_GNU */
#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL _strdup_d)(char const *__restrict __str, __DEBUGINFO);
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_XOPEN2K8
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL _strndup_d)(char const *__restrict __str, size_t __max_chars, __DEBUGINFO);
#endif /* __USE_XOPEN2K8 */
#endif /* !__KERNEL__ */
#ifdef __USE_KOS
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,_memcpyb_d,(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes, __DEBUGINFO),_memcpy_d,(__dst,__src,__n_bytes,__DEBUGINFO_FWD))
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpyw_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_words, __DEBUGINFO);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpyl_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords, __DEBUGINFO);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpyq_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords, __DEBUGINFO);
#ifdef __USE_GNU
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,_mempcpyb_d,(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes, __DEBUGINFO),_mempcpy_d,(__dst,__src,__n_bytes,__DEBUGINFO_FWD))
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _mempcpyw_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_words, __DEBUGINFO);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _mempcpyl_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords, __DEBUGINFO);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _mempcpyq_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords, __DEBUGINFO);
#endif /* __USE_GNU */
#ifndef __KERNEL__
__LIBC __ATTR_LIBC_PRINTF(4,5) __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__ATTR_CDECL _strdupf_d)(__DEBUGINFO, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(1,0) __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL _vstrdupf_d)(char const *__restrict __format, __builtin_va_list __args, __DEBUGINFO);
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */
#else /* __USE_DEBUG != 0 */
#   define _memcpy_d(dst,src,n_bytes,...) memcpy(dst,src,n_bytes)
#ifdef __USE_GNU
#   define _mempcpy_d(dst,src,n_bytes,...) mempcpy(dst,src,n_bytes)
#endif /* __USE_GNU */
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
#   define _memcpyq_d(dst,src,n_qwords,...) memcpyq(dst,src,n_qwords)
#ifdef __USE_GNU
#   define _mempcpyb_d(dst,src,n_bytes,...)  mempcpyb(dst,src,n_bytes)
#   define _mempcpyw_d(dst,src,n_words,...)  mempcpyw(dst,src,n_words)
#   define _mempcpyl_d(dst,src,n_dwords,...) mempcpyl(dst,src,n_dwords)
#   define _mempcpyq_d(dst,src,n_dwords,...) mempcpyq(dst,src,n_qwords)
#endif /* __USE_GNU */
#ifndef __KERNEL__
#   define _strdupf_d(file,line,func,...)   strdupf(__VA_ARGS__)
#   define _vstrdupf_d(format,args,...)     vstrdupf(format,args)
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */
#endif /* __USE_DEBUG == 0 */
#ifdef __USE_DEBUG_HOOK
#ifndef __OPTIMIZE_LIBC__
#   define memcpy(dst,src,n_bytes) _memcpy_d(dst,src,n_bytes,__DEBUGINFO_GEN)
#ifdef __USE_GNU
#   define mempcpy(dst,src,n_bytes) _mempcpy_d(dst,src,n_bytes,__DEBUGINFO_GEN)
#endif /* __USE_GNU */
#endif /* !__OPTIMIZE_LIBC__ */
#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#   define strdup(str)             _strdup_d(str,__DEBUGINFO_GEN)
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_XOPEN2K8
#   define strndup(str,max_chars)  _strndup_d(str,max_chars,__DEBUGINFO_GEN)
#endif /* __USE_XOPEN2K8 */
#endif /* !__KERNEL__ */
#ifdef __USE_KOS
#ifndef __OPTIMIZE_LIBC__
#   define memcpyb(dst,src,n_bytes)   _memcpyb_d(dst,src,n_bytes,__DEBUGINFO_GEN)
#   define memcpyw(dst,src,n_words)   _memcpyw_d(dst,src,n_words,__DEBUGINFO_GEN)
#   define memcpyl(dst,src,n_dwords)  _memcpyl_d(dst,src,n_dwords,__DEBUGINFO_GEN)
#   define memcpyq(dst,src,n_qwords)  _memcpyq_d(dst,src,n_qwords,__DEBUGINFO_GEN)
#ifdef __USE_GNU
#   define mempcpyb(dst,src,n_bytes)  _mempcpyb_d(dst,src,n_bytes,__DEBUGINFO_GEN)
#   define mempcpyw(dst,src,n_words)  _mempcpyw_d(dst,src,n_words,__DEBUGINFO_GEN)
#   define mempcpyl(dst,src,n_dwords) _mempcpyl_d(dst,src,n_dwords,__DEBUGINFO_GEN)
#   define mempcpyq(dst,src,n_qwords) _mempcpyq_d(dst,src,n_qwords,__DEBUGINFO_GEN)
#endif /* __USE_GNU */
#endif /* !__OPTIMIZE_LIBC__ */
#ifndef __KERNEL__
#ifdef _strdupf_d
#   define strdupf(...)               _strdupf_d(__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__)
#else /* _strdupf_d */
#   define strdupf(...)               _strdupf_d(__DEBUGINFO_GEN,__VA_ARGS__)
#endif /* !_strdupf_d */
#   define vstrdupf(format,args)      _vstrdupf_d(format,args,__DEBUGINFO_GEN)
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */
#endif /* __USE_DEBUG_HOOK */
#endif /* __USE_DEBUG */



#ifndef __KERNEL__
#if defined(__USE_KOS) && defined(__CRT_DOS)
__REDIRECT(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strlwr_l,(char *__restrict __str, __locale_t __locale),_strlwr_l,(__str,__locale))
__REDIRECT(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strupr_l,(char *__restrict __str, __locale_t __locale),_strupr_l,(__str,__locale))
__REDIRECT(__LIBC,__ATTR_PURE __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strcasecoll,(char const *__str1, char const *__str2),_stricoll,(__str1,__str2))
__REDIRECT(__LIBC,__ATTR_PURE __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strcasecoll_l,(char const *__str1, char const *__str2, __locale_t __locale),_stricoll_l,(__str1,__str2,__locale))
__REDIRECT(__LIBC,__ATTR_PURE __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strncoll,(char const *__str1, char const *__str2, size_t __max_chars),_strncoll,(__str1,__str2,__max_chars))
__REDIRECT(__LIBC,__ATTR_PURE __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strncoll_l,(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale),_strncoll_l,(__str1,__str2,__max_chars,__locale))
__REDIRECT(__LIBC,__ATTR_PURE __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strncasecoll,(char const *__str1, char const *__str2, size_t __max_chars),_strnicoll,(__str1,__str2,__max_chars))
__REDIRECT(__LIBC,__ATTR_PURE __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strncasecoll_l,(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale),_strnicoll_l,(__str1,__str2,__max_chars,__locale))
#endif /* __USE_KOS && __CRT_DOS */
#if (defined(__USE_KOS) || defined(__USE_DOS)) && defined(__CRT_DOS)
__REDIRECT(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strlwr,(char *__restrict __str),_strlwr,(__str))
__REDIRECT(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strnset,(char *__restrict __str, int __char, size_t __max_chars),_strnset,(__str,__char,__max_chars))
__REDIRECT(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strrev,(char *__restrict __str),_strrev,(__str))
#ifdef _MSC_VER
#define ___strset_defined 1
__LIBC __PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL _strset)(char *__restrict __str, int __char);
#pragma intrinsic(_strset)
#define strset  _strset
#else
__REDIRECT(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strset,(char *__restrict __str, int __char),_strset,(__str,__char))
#endif
__REDIRECT(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strupr,(char *__restrict __str),_strupr,(__str))
#endif /* (__USE_KOS || __USE_DOS) && __CRT_DOS */

#ifdef __USE_DOS
__REDIRECT_IFKOS(__LIBC,__SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC __NONNULL((1)),
                 char *,__LIBCCALL,_strdup,(char const *__restrict __str),strdup,(__str))

#ifdef __GLC_COMPAT__
#ifndef ____libc_tolower_defined
#define ____libc_tolower_defined 1
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_tolower,(int __c),tolower,(__c))
#endif /* !____libc_tolower_defined */
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
int (__LIBCCALL _memicmp)(void const *__a, void const *__b, size_t __n_bytes) {
    __BYTE_TYPE__ *__ai = (__BYTE_TYPE__ *)__a,*__bi = (__BYTE_TYPE__ *)__b; int __temp;
    while (__n_bytes--) if ((__temp = __libc_tolower(*__ai++) - __libc_tolower(*__bi++)) != 0) return __temp;
    return 0;
}
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
int (__LIBCCALL _memicmp_l)(void const *__a, void const *__b,
                            size_t __n_bytes, __locale_t __UNUSED(__locale)) {
    return _memicmp(__a,__b,__n_bytes);
}
#elif defined(__CRT_DOS)
__LIBC __PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL _strlwr)(char *__restrict __str);
__LIBC __PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL _strnset)(char *__restrict __str, int __char, size_t __max_chars);
__LIBC __PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL _strrev)(char *__restrict __str);
#ifndef ___strset_defined
#define ___strset_defined 1
__LIBC __PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL _strset)(char *__restrict __str, int __char);
#ifdef _MSC_VER
#pragma intrinsic(_strset)
#endif /* _MSC_VER */
#endif /* !___strset_defined */
__LIBC __PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL _strupr)(char *__restrict __str);
__LIBC __PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL _strlwr_l)(char *__restrict __str, __locale_t __locale);
__LIBC __PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL _strupr_l)(char *__restrict __str, __locale_t __locale);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL _stricoll)(char const *__str1, char const *__str2);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL _stricoll_l)(char const *__str1, char const *__str2, __locale_t __locale);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL _strncoll)(char const *__str1, char const *__str2, size_t __max_chars);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL _strncoll_l)(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL _strnicoll)(char const *__str1, char const *__str2, size_t __max_chars);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL _strnicoll_l)(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL _memicmp)(void const *__a, void const *__b, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL _memicmp_l)(void const *__a, void const *__b, size_t __n_bytes, __locale_t __locale);
#endif /* Builtin... */

__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_strcmpi,(char const *__str1, char const *__str2),strcasecmp,(__str1,__str2))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_strcoll_l,(char const *__str1, char const *__str2, __locale_t __locale),strcoll_l,(__str1,__str2,__locale))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_stricmp,(char const *__str1, char const *__str2),strcasecmp,(__str1,__str2))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_stricmp_l,(char const *__str1, char const *__str2, __locale_t __locale),strcasecmp_l,(__str1,__str2,__locale))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_strnicmp,(char const *__str1, char const *__str2, size_t __max_chars),strncasecmp,(__str1,__str2,__max_chars))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_strnicmp_l,(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale),strncasecmp_l,(__str1,__str2,__max_chars,__locale))
__REDIRECT_IFKOS(__LIBC,__NONNULL((1,2)),size_t,__LIBCCALL,_strxfrm_l,(char *__dst, char const *__src, size_t __max_chars, __locale_t __locale),strxfrm_l,(__dst,__src,__max_chars,__locale))
__REDIRECT_IFKOS(__LIBC,__NONNULL((1,2)),void *,__LIBCCALL,_memccpy,(void *__dst, void const *__src, int __needle, size_t __max_chars),memccpy,(__dst,__src,__needle,__max_chars))
__REDIRECT2(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strcmpi,(char const *__str1, char const *__str2),strcasecmp,_stricmp,(__str1,__str2))
__REDIRECT2(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,stricmp,(char const *__str1, char const *__str2),strcasecmp,_stricmp,(__str1,__str2))
__REDIRECT2(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strnicmp,(char const *__str1, char const *__str2, size_t __max_chars),strncasecmp,_strnicmp,(__str1,__str2,__max_chars))
#ifdef __GLC_COMPAT__
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
int (__LIBCCALL memicmp)(void const *__a, void const *__b, size_t __n_bytes) {
    return _memicmp(__a,__b,__n_bytes);
}
#else /* __GLC_COMPAT__ */
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,memicmp,(void const *__a, void const *__b, size_t __n_bytes),_memicmp,(__a,__b,__n_bytes))
#endif /* !__GLC_COMPAT__ */


/* Fulfill DOS's need to put all the wide-string stuff in here as well... */
#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef ___wcsicmp_defined
#define ___wcsicmp_defined 1
__REDIRECT2(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsicmp,(wchar_t const *__str1, wchar_t const *__str2),wcscasecmp,_wcsicmp,(__str1,__str2))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsicmp,(wchar_t const *__str1, wchar_t const *__str2),wcscasecmp,(__str1,__str2))
__REDIRECT2(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsnicmp,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncasecmp,_wcsnicmp,(__str1,__str2,__max_chars))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsnicmp,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncasecmp,(__str1,__str2,__max_chars))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsicmp_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),wcscasecmp_l,(__str1,__str2,__locale))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsnicmp_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncasecmp_l,(__str1,__str2,__max_chars,__locale))
#endif /* !___wcsicmp_defined */

#ifndef __wcsicoll_defined
#define __wcsicoll_defined 1
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcscoll_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),wcscoll_l,(__str1,__str2,__locale))
__REDIRECT2(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsicoll,(wchar_t const *__str1, wchar_t const *__str2),wcscasecoll,_wcsicoll,(__str1,__str2))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsicoll,(wchar_t const *__str1, wchar_t const *__str2),wcscasecoll,(__str1,__str2))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsicoll_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),wcscasecoll_l,(__str1,__str2,__locale))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsncoll,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncoll,(__str1,__str2,__max_chars))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsncoll_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncoll_l,(__str1,__str2,__max_chars,__locale))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsnicoll,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncasecoll,(__str1,__str2,__max_chars))
__REDIRECT_IFKOS(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsnicoll_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncasecoll_l,(__str1,__str2,__max_chars,__locale))
#endif /* !__wcsicoll_defined */

#ifndef ___wcsxfrm_l_defined
#define ___wcsxfrm_l_defined 1
__REDIRECT_IFKOS(__LIBC,__NONNULL((1,2)),size_t,__LIBCCALL,_wcsxfrm_l,(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __max_chars, __locale_t __locale),wcsxfrm_l,(__dst,__src,__max_chars,__locale))
#endif /* !___wcsxfrm_l_defined */

#ifndef ___wcsdup_defined
#define ___wcsdup_defined 1
__REDIRECT_IFKOS(__LIBC,__SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC __NONNULL((1)),
                 wchar_t *,__LIBCCALL,_wcsdup,(wchar_t const *__restrict __str),wcsdup,(__str))
#endif /* !___wcsdup_defined */

#ifdef __CRT_DOS
#ifndef __wcsrev_defined
#define __wcsrev_defined 1
/* TODO: Add GLC compat implementations for these. */
__REDIRECT_IFDOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsrev,(wchar_t *__restrict __str),_wcsrev,(__str))
__REDIRECT_IFDOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsset,(wchar_t *__restrict __str, wchar_t __needle),_wcsset,(__str,__needle))
__REDIRECT_IFDOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsnset,(wchar_t *__restrict __str, wchar_t __needle, size_t __max_chars),_wcsnset,(__str,__needle,__max_chars))
__REDIRECT_IFDOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcslwr,(wchar_t *__restrict __str),_wcslwr,(__str))
__REDIRECT_IFDOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsupr,(wchar_t *__restrict __str),_wcsupr,(__str))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcsrev,(wchar_t *__restrict __str),wcsrev,(__str))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcsset,(wchar_t *__restrict __str, wchar_t __char),wcsset,(__str,__char))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcsnset,(wchar_t *__restrict __str, wchar_t __char, size_t __max_chars),wcsnset,(__str,__char,__max_chars))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcslwr,(wchar_t *__restrict __str),wcslwr,(__str))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcslwr_l,(wchar_t *__restrict __str, __locale_t __locale),wcslwr_l,(__str,__locale))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcsupr,(wchar_t *__restrict __str),wcsupr,(__str))
__REDIRECT_IFKOS(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,_wcsupr_l,(wchar_t *__restrict __str, __locale_t __locale),wcsupr_l,(__str,__locale))
#endif /* !__wcsrev_defined */
#endif /* __CRT_DOS */

#ifndef __std_wcscmp_defined
#define __std_wcscmp_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcscmp)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcsncmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n);
__LIBC __WUNUSED __NONNULL((1,2)) int (__LIBCCALL wcscoll)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC __NONNULL((1,2)) size_t (__LIBCCALL wcsxfrm)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n);
__NAMESPACE_STD_END
#endif /* !__std_wcscmp_defined */
#ifndef __wcscmp_defined
#define __wcscmp_defined 1
__NAMESPACE_STD_USING(wcscmp)
__NAMESPACE_STD_USING(wcsncmp)
__NAMESPACE_STD_USING(wcscoll)
__NAMESPACE_STD_USING(wcsxfrm)
#endif /* !__wcscmp_defined */

#ifndef __std_wcsspn_defined
#define __std_wcsspn_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL wcsspn)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL wcscspn)(wchar_t const *__haystack, wchar_t const *__reject);
__NAMESPACE_STD_END
#endif /* !__std_wcsspn_defined */
#ifndef __wcsspn_defined
#define __wcsspn_defined 1
__NAMESPACE_STD_USING(wcsspn)
__NAMESPACE_STD_USING(wcscspn)
#endif /* !__wcsspn_defined */

#ifndef __std_wcslen_defined
#define __std_wcslen_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcslen)(wchar_t const *__restrict __str);
__NAMESPACE_STD_END
#endif /* !__std_wcslen_defined */
#ifndef __wcslen_defined
#define __wcslen_defined 1
__NAMESPACE_STD_USING(wcslen)
#endif /* !__wcslen_defined */

#ifndef __wcsnlen_defined
#define __wcsnlen_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsnlen)(wchar_t const *__restrict __src, size_t __max_chars);
#endif /* !__wcsnlen_defined */

#ifndef __std_wcschr_defined
#define __std_wcschr_defined 1
__NAMESPACE_STD_BEGIN
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t *,__LIBCCALL,wcschr,(wchar_t *__restrict __str, wchar_t __needle),wcschr,(__str,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t *,__LIBCCALL,wcsrchr,(wchar_t *__restrict __str, wchar_t __needle),wcsrchr,(__str,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t const *,__LIBCCALL,wcschr,(wchar_t const *__restrict __str, wchar_t __needle),wcschr,(__str,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsrchr,(wchar_t const *__restrict __str, wchar_t __needle),wcsrchr,(__str,__needle))
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcschr)(wchar_t const *__restrict __str, wchar_t __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsrchr)(wchar_t const *__restrict __str, wchar_t __needle);
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
__NAMESPACE_STD_END
#endif /* !__std_wcschr_defined */
#ifndef __wcschr_defined
#define __wcschr_defined 1
__NAMESPACE_STD_USING(wcschr)
__NAMESPACE_STD_USING(wcsrchr)
#endif /* !__wcschr_defined */

#ifndef __std_wcscpy_defined
#define __std_wcscpy_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcscpy)(wchar_t *__dst, wchar_t const *__src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcscat)(wchar_t *__dst, wchar_t const *__src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsncat)(wchar_t *__dst, wchar_t const *__src, size_t __max_chars);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsncpy)(wchar_t *__dst, wchar_t const *__src, size_t __max_chars);
__NAMESPACE_STD_END
#endif /* !__std_wcscpy_defined */
#ifndef __wcscpy_defined
#define __wcscpy_defined 1
__NAMESPACE_STD_USING(wcscpy)
__NAMESPACE_STD_USING(wcscat)
__NAMESPACE_STD_USING(wcsncat)
__NAMESPACE_STD_USING(wcsncpy)
#endif /* !__wcscpy_defined */

#ifndef __wcsdup_defined
#define __wcsdup_defined 1
__REDIRECT_IFDOS(__LIBC,__SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC __NONNULL((1)),
                 wchar_t *,__LIBCCALL,wcsdup,(wchar_t const *__restrict __str),_wcsdup,(__str))
#endif /* !__wcsdup_defined */

#ifndef __std_wcsstr_defined
#define __std_wcsstr_defined 1
__NAMESPACE_STD_BEGIN
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t *,__LIBCCALL,wcspbrk,(wchar_t *__haystack, wchar_t const *__accept),wcspbrk,(__haystack,__accept))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t const *,__LIBCCALL,wcspbrk,(wchar_t const *__haystack, wchar_t const *__accept),wcspbrk,(__haystack,__accept))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t *,__LIBCCALL,wcsstr,(wchar_t *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t const *,__LIBCCALL,wcsstr,(wchar_t const *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) wchar_t *(__LIBCCALL wcspbrk)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsstr)(wchar_t const *__haystack, wchar_t const *__needle);
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
__NAMESPACE_STD_END
#endif /* !__std_wcsstr_defined */
#ifndef __wcsstr_defined
#define __wcsstr_defined 1
__NAMESPACE_STD_USING(wcspbrk)
__NAMESPACE_STD_USING(wcsstr)
#endif /* !__wcsstr_defined */

#ifndef __wcswcs_defined
#define __wcswcs_defined 1
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__ATTR_PURE,wchar_t *,__LIBCCALL,wcswcs,(wchar_t *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
__REDIRECT(__LIBC,__ATTR_PURE,wchar_t const *,__LIBCCALL,wcswcs,(wchar_t const *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
__LIBC __ATTR_PURE wchar_t *(__LIBCCALL wcswcs)(wchar_t const *__haystack, wchar_t const *__needle);
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
#endif /* !__wcswcs_defined */

#ifndef __std_wcstok_defined
#define __std_wcstok_defined 1
__NAMESPACE_STD_BEGIN
#if defined(__USE_DOS) && !defined(__USE_ISOC95)
/* Define wcstok() incorrectly, the same way DOS does. */
#ifdef __CRT_DOS
__REDIRECT_IFKOS(__LIBC,__NONNULL((1,2)),wchar_t *,__LIBCCALL,wcstok,(wchar_t *__restrict __s, wchar_t const *__restrict __delim),__wcstok_f,(__s,__delim))
#else /* __CRT_DOS */
__REDIRECT(__LIBC,,wchar_t *,__LIBCCALL,__wcstok_impl,(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr),wcstok,(__s,__delim,__ptr))
__INTERN __ATTR_WEAK __ATTR_UNUSED wchar_t *__wcstok_safe = 0;
__LOCAL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcstok)(wchar_t *__restrict __s, wchar_t const *__restrict __delim) { return __wcstok_impl(__s,__delim,&__wcstok_safe); }
#endif /* !__CRT_DOS */
#elif defined(__CRT_DOS) && __SIZEOF_WCHAR_T__ == 2
__REDIRECT(__LIBC,__NONNULL((1,2,3)),wchar_t *,__LIBCCALL,wcstok,(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr),wcstok_s,(__s,__delim,__ptr))
#else
__LIBC __NONNULL((1,2,3)) wchar_t *(__LIBCCALL wcstok)(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr);
#endif
__NAMESPACE_STD_END
#endif /* !__std_wcstok_defined */
#ifndef __wcstok_defined
#define __wcstok_defined 1
__NAMESPACE_STD_USING(wcstok)
#endif /* !__wcstok_defined */

#ifndef _WSTRING_DEFINED
#define _WSTRING_DEFINED 1
#ifndef __wcstok_s_defined
#define __wcstok_s_defined 1
__REDIRECT_IFKOS(__LIBC,__NONNULL((1,2,3)),wchar_t *,__LIBCCALL,wcstok_s,
                (wchar_t *__restrict __str, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr),
                 wcstok,(__str,__delim,__ptr))
#endif /* !__wcstok_s_defined */

#ifdef __CRT_DOS
#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __wcserror_defined
#define __wcserror_defined 1
__LIBC __WUNUSED __PORT_DOSONLY_ALT(strerror) wchar_t *(__LIBCCALL _wcserror)(int __errnum);
__LIBC __WUNUSED __PORT_DOSONLY_ALT(strerror) wchar_t *(__LIBCCALL __wcserror)(wchar_t const *__errmsg);
__LIBC __NONNULL((1)) __PORT_DOSONLY_ALT(strerror) errno_t (__LIBCCALL _wcserror_s)(wchar_t *__restrict __buf, size_t __max_chars, int __errnum);
__LIBC __NONNULL((1)) __PORT_DOSONLY_ALT(strerror) errno_t (__LIBCCALL __wcserror_s)(wchar_t *__restrict __buf, size_t __max_chars, wchar_t const *__errmsg);
#endif /* !__wcserror_defined */

#ifndef ___wcsset_s_defined
#define ___wcsset_s_defined 1
#ifdef __GLC_COMPAT__
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_towlower,(int __wc),towlower,(__wc))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_towupper,(int __wc),towupper,(__wc))
__REDIRECT2_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_towlower_l,(int __wc, __locale_t __locale),towlower_l,_towlower_l,(__wc))
__REDIRECT2_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_towupper_l,(int __wc, __locale_t __locale),towupper_l,_towupper_l,(__wc))
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcsset_s)(wchar_t *__restrict __str, size_t __maxlen, wchar_t __val) { if (wcsnlen(__str,__buflen) == __buflen) return 34; wcsset(__str,__val); return 0; }
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcsnset_s)(wchar_t *__restrict __str, size_t __buflen, wchar_t __val, size_t __maxlen) { if (__maxlen < __buflen) __buflen = __maxlen; while (__buflen-- && *__str) *__str++ = __val; return 0; }
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcslwr_s)(wchar_t *__restrict __str, size_t __maxlen) { while (__maxlen-- && *__str) *__str = __libc_towlower(*__str),++__str; return 0 }
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcsupr_s)(wchar_t *__restrict __str, size_t __maxlen) { while (__maxlen-- && *__str) *__str = __libc_towupper(*__str),++__str; return 0 }
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcslwr_s_l)(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale) { while (__maxlen-- && *__str) *__str = __libc_towlower_l(*__str,__locale),++__str; return 0 }
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcsupr_s_l)(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale) { while (__maxlen-- && *__str) *__str = __libc_towupper_l(*__str,__locale),++__str; return 0 }
#else /* __GLC_COMPAT__ */
__REDIRECT_IFKOS(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,_wcsset_s,(wchar_t *__restrict __str, size_t __maxlen, wchar_t __val),wcsset_s,(__str,__maxlen,__val))
__REDIRECT_IFKOS(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,_wcsnset_s,(wchar_t *__restrict __str, size_t __buflen, wchar_t __val, size_t __maxlen),wcsnset_s,(__str,__buflen,__val,__maxlen))
__REDIRECT_IFKOS(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,_wcslwr_s,(wchar_t *__restrict __str, size_t __maxlen),wcslwr_s,(__str,__maxlen))
__REDIRECT_IFKOS(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,_wcsupr_s,(wchar_t *__restrict __str, size_t __maxlen),wcsupr_s,(__str,__maxlen))
__REDIRECT_IFKOS(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,_wcslwr_s_l,(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale),wcslwr_s_l,(__str,__maxlen,__locale))
__REDIRECT_IFKOS(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,_wcsupr_s_l,(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale),wcsupr_s_l,(__str,__maxlen,__locale))
#endif /* !__GLC_COMPAT__ */
#endif /* !___wcsset_s_defined */

#ifdef __USE_DOS_SLIB
#ifndef __wcsncat_s_defined
#define __wcsncat_s_defined 1
#ifdef __GLC_COMPAT__
__LOCAL __NONNULL((1,3)) errno_t (__LIBCCALL wcscat_s)
(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src) {
    wchar_t *__dstend = (wchar_t *)__dst+wcsnlen(__dst,__dstsize);
    size_t __srclen = wcslen(__src);
    __dstsize -= (size_t)(__dstend-__dst);
    if (__srclen+1 > __dstsize) return 34;
    do *__dstend++ = *__src++; while (__srclen--);
    return 0;
}
__LOCAL __NONNULL((1,3)) errno_t (__LIBCCALL wcscpy_s)
(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src) {
    size_t __srclen = wcslen(__src);
    if (__srclen+1 > __dstsize) return 34;
    do *__dst++ = *__src++; while (__srclen--);
    return 0;
}
__LOCAL __NONNULL((1,3)) errno_t (__LIBCCALL wcsncat_s)
(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __max_chars) {
    wchar_t *__dstend = (wchar_t *)__dst+wcsnlen(__dst,__dstsize);
    size_t __srclen = wcsnlen(__src,__maxlen);
    __dstsize -= (size_t)(__dstend-__dst);
    if (__srclen+1 > __dstsize) return 34;
    while (__srclen--) *__dstend++ = *__src++;
    *__dstend = (wchar_t)'\0';
    return 0;
}
__LOCAL __NONNULL((1,3)) errno_t (__LIBCCALL wcsncpy_s)
(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __max_chars) {
    size_t __srclen = wcsnlen(__src,__maxlen);
    if (__srclen+1 > __dstsize) return 34;
    while (__srclen--) *__dst++ = *__src++;
    *__dst = (wchar_t)'\0';
    return 0;
}
#else
__LIBC __NONNULL((1,3)) errno_t (__LIBCCALL wcscat_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src);
__LIBC __NONNULL((1,3)) errno_t (__LIBCCALL wcscpy_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src);
__LIBC __NONNULL((1,3)) errno_t (__LIBCCALL wcsncat_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __max_chars);
__LIBC __NONNULL((1,3)) errno_t (__LIBCCALL wcsncpy_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __max_chars);
#endif
#endif /* !__wcsncat_s_defined */
#endif /* __USE_DOS_SLIB */
#endif /* __CRT_DOS */

#ifdef __USE_DOS_SLIB
#ifndef __wcsnlen_s_defined
#define __wcsnlen_s_defined 1
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsnlen_s)(wchar_t const *__restrict __src, size_t __max_chars) { return__src ? wcsnlen(__src,__max_chars) : 0; }
#endif /* !__wcsnlen_s_defined */
#endif /* __USE_DOS_SLIB */
#endif /* !_WSTRING_DEFINED */

#endif /* __USE_DOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_GNU
#include "hybrid/alloca.h"
#include "libc/string.h"
#ifndef __NO_XBLOCK
#define strdupa(s) \
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = __libc_strlen(__old)+1; \
   char *const __new = (char *)__ALLOCA(__len); \
   __XRETURN (char *)__NAMESPACE_STD_SYM memcpy(__new,__old,__len); \
 })
#define strndupa(s,n) \
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = __hybrid_strnlen(__old,(n)); \
   char *const __new = (char *)__ALLOCA(__len+1); \
   __new[__len] = '\0'; \
   __XRETURN (char *)__NAMESPACE_STD_SYM memcpy(__new,__old,__len); \
 })
#else /* !__NO_XBLOCK */
__NAMESPACE_INT_BEGIN
__LOCAL char *(__LIBCCALL __strndupa_init)(void *__restrict __buf, char const *__restrict __src, size_t __n) {
    __n = __libc_strnlen(__src,__n);
    __NAMESPACE_STD_SYM memcpy(__buf,__src,__n*sizeof(char));
    ((char *)__buf)[__n] = '\0';
    return (char *)__buf;
}
__NAMESPACE_INT_END
#define strdupa(s) \
  (__NAMESPACE_STD_SYM strcpy((char *)__ALLOCA((__libc_strlen(s)+1)*sizeof(char)),(s)))
#define strndupa(s,n) \
  (__NAMESPACE_INT_SYM __strndupa_init(__ALLOCA((__libc_strnlen((s),(n))+1)*sizeof(char)),(s),(n)))
#endif /* __NO_XBLOCK */
#ifdef __USE_KOS
#include "__amalloc.h"
#ifndef __NO_XBLOCK
#define strdupma(s)	\
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = __libc_strlen(__old)+1; \
   char *const __new = (char *)__amalloc(__len); \
   __XRETURN __new ? (char *)__NAMESPACE_STD_SYM memcpy(__new,__old,__len) : (char *)0; \
 })
#define strndupma(s,n) \
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = __hybrid_strnlen(__old,(n)); \
   char *const __new = (char *)__amalloc(__len+1); \
   __XRETURN __new ? (__new[__len] = '\0',(char *)__NAMESPACE_STD_SYM memcpy(__new,__old,__len)) : (char *)0; \
 })
#else /* !__NO_XBLOCK */
__NAMESPACE_INT_BEGIN
__LOCAL char *(__LIBCCALL __strdupma_init)(void *__buf, char const *__restrict __src) {
    if (__buf) __NAMESPACE_STD_SYM strcpy((char *)__buf,__src);
    return (char *)__buf;
}
__LOCAL char *(__LIBCCALL __strndupma_init)(void *__buf, char const *__restrict __src, size_t __n) {
    if (__buf) {
        __n = __libc_strnlen(__src,__n);
        __NAMESPACE_STD_SYM memcpy(__buf,__src,__n*sizeof(char));
        ((char *)__buf)[__n] = '\0';
    }
    return (char *)__buf;
}
__NAMESPACE_INT_END
#define strdupma(s) \
  (__NAMESPACE_INT_SYM __strdupma_init(__amalloc((__libc_strlen(s)+1)*sizeof(char)),(s)))
#define strndupma(s,n) \
  (__NAMESPACE_INT_SYM __strndupma_init(__amalloc((__libc_strnlen((s),(n))+1)*sizeof(char)),(s),(n)))
#endif /* __NO_XBLOCK */
#endif /* __USE_KOS */
#endif /* __USE_GNU */


#endif /* !_STRING_H */
