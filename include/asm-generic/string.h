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

#ifdef __CC__
__SYSDECL_BEGIN

#ifndef __asm_memcpy
#define __NO_asm_memcpy 1
#define __asm_memcpy(dst,src,n_bytes)            __libc_memcpy(dst,src,n_bytes)
#endif /* !__asm_memcpy */
#ifndef __asm_memcpyw
#define __NO_asm_memcpyw 1
#define __asm_memcpyw(dst,src,n_words)           __libc_memcpyw(dst,src,n_words)
#endif /* !__asm_memcpyw */
#ifndef __asm_memcpyl
#define __NO_asm_memcpyl 1
#define __asm_memcpyl(dst,src,n_dwords)          __libc_memcpyl(dst,src,n_dwords)
#endif /* !__asm_memcpyl */
#ifndef __asm_memcpyq
#define __NO_asm_memcpyq 1
#define __asm_memcpyq(dst,src,n_qwords)          __libc_memcpyq(dst,src,n_qwords)
#endif /* !__asm_memcpyq */
#ifndef __asm_mempcpy
#define __NO_asm_mempcpy 1
#define __asm_mempcpy(dst,src,n_bytes)           __libc_mempcpy(dst,src,n_bytes)
#endif /* !__asm_mempcpy */
#ifndef __asm_mempcpyw
#define __NO_asm_mempcpyw 1
#define __asm_mempcpyw(dst,src,n_words)          __libc_mempcpyw(dst,src,n_words)
#endif /* !__asm_mempcpyw */
#ifndef __asm_mempcpyl
#define __NO_asm_mempcpyl 1
#define __asm_mempcpyl(dst,src,n_dwords)         __libc_mempcpyl(dst,src,n_dwords)
#endif /* !__asm_mempcpyl */
#ifndef __asm_mempcpyq
#define __NO_asm_mempcpyq 1
#define __asm_mempcpyq(dst,src,n_qwords)         __libc_mempcpyq(dst,src,n_qwords)
#endif /* !__asm_mempcpyq */
#ifndef __asm_memmove
#define __NO_asm_memmove 1
#define __asm_memmove(dst,src,n_bytes)           __libc_memmove(dst,src,n_bytes)
#endif /* !__asm_memmove */
#ifndef __asm_memmovew
#define __NO_asm_memmovew 1
#define __asm_memmovew(dst,src,n_words)          __libc_memmovew(dst,src,n_words)
#endif /* !__asm_memmovew */
#ifndef __asm_memmovel
#define __NO_asm_memmovel 1
#define __asm_memmovel(dst,src,n_dwords)         __libc_memmovel(dst,src,n_dwords)
#endif /* !__asm_memmovel */
#ifndef __asm_memmoveq
#define __NO_asm_memmoveq 1
#define __asm_memmoveq(dst,src,n_qwords)         __libc_memmoveq(dst,src,n_qwords)
#endif /* !__asm_memmoveq */
#ifndef __asm_memset
#define __NO_asm_memset 1
#define __asm_memset(dst,byte,n_bytes)           __libc_memset(dst,byte,n_bytes)
#endif /* !__asm_memset */
#ifndef __asm_memsetw
#define __NO_asm_memsetw 1
#define __asm_memsetw(dst,word,n_words)          __libc_memsetw(dst,word,n_words)
#endif /* !__asm_memsetw */
#ifndef __asm_memsetl
#define __NO_asm_memsetl 1
#define __asm_memsetl(dst,dword,n_dwords)        __libc_memsetl(dst,dword,n_dwords)
#endif /* !__asm_memsetl */
#ifndef __asm_memsetq
#define __NO_asm_memsetq 1
#define __asm_memsetq(dst,qword,n_qwords)        __libc_memsetq(dst,qword,n_qwords)
#endif /* !__asm_memsetq */
#ifndef __asm_memcmp
#define __NO_asm_memcmp 1
#define __asm_memcmp(a,b,n_bytes)                __libc_memcmp(a,b,n_bytes)
#endif /* !__asm_memcmp */
#ifndef __asm_memcmpw
#define __NO_asm_memcmpw 1
#define __asm_memcmpw(a,b,n_words)               __libc_memcmpw(a,b,n_words)
#endif /* !__asm_memcmpw */
#ifndef __asm_memcmpl
#define __NO_asm_memcmpl 1
#define __asm_memcmpl(a,b,n_dwords)              __libc_memcmpl(a,b,n_dwords)
#endif /* !__asm_memcmpl */
#ifndef __asm_memcmpq
#define __NO_asm_memcmpq 1
#define __asm_memcmpq(a,b,n_qwords)              __libc_memcmpq(a,b,n_qwords)
#endif /* !__asm_memcmpq */
#ifndef __asm_memchr
#define __NO_asm_memchr 1
#define __asm_memchr(haystack,needle,n_bytes)    __libc_memchr(haystack,needle,n_bytes)
#endif /* !__asm_memchr */
#ifndef __asm_memchrw
#define __NO_asm_memchrw 1
#define __asm_memchrw(haystack,needle,n_words)   __libc_memchrw(haystack,needle,n_words)
#endif /* !__asm_memchrw */
#ifndef __asm_memchrl
#define __NO_asm_memchrl 1
#define __asm_memchrl(haystack,needle,n_dwords)  __libc_memchrl(haystack,needle,n_dwords)
#endif /* !__asm_memchrl */
#ifndef __asm_memchrq
#define __NO_asm_memchrq 1
#define __asm_memchrq(haystack,needle,n_qwords)  __libc_memchrq(haystack,needle,n_qwords)
#endif /* !__asm_memchrq */
#ifndef __asm_memrchr
#define __NO_asm_memrchr 1
#define __asm_memrchr(haystack,needle,n_bytes)   __libc_memrchr(haystack,needle,n_bytes)
#endif /* !__asm_memrchr */
#ifndef __asm_memrchrw
#define __NO_asm_memrchrw 1
#define __asm_memrchrw(haystack,needle,n_words)  __libc_memrchrw(haystack,needle,n_words)
#endif /* !__asm_memrchrw */
#ifndef __asm_memrchrl
#define __NO_asm_memrchrl 1
#define __asm_memrchrl(haystack,needle,n_dwords) __libc_memrchrl(haystack,needle,n_dwords)
#endif /* !__asm_memrchrl */
#ifndef __asm_memrchrq
#define __NO_asm_memrchrq 1
#define __asm_memrchrq(haystack,needle,n_qwords) __libc_memrchrq(haystack,needle,n_qwords)
#endif /* !__asm_memrchrq */
#ifndef __asm_memend
#define __NO_asm_memend 1
#define __asm_memend(haystack,needle,n_bytes)    __libc_memend(haystack,needle,n_bytes)
#endif /* !__asm_memend */
#ifndef __asm_memendw
#define __NO_asm_memendw 1
#define __asm_memendw(haystack,needle,n_words)   __libc_memendw(haystack,needle,n_words)
#endif /* !__asm_memendw */
#ifndef __asm_memendl
#define __NO_asm_memendl 1
#define __asm_memendl(haystack,needle,n_dwords)  __libc_memendl(haystack,needle,n_dwords)
#endif /* !__asm_memendl */
#ifndef __asm_memendq
#define __NO_asm_memendq 1
#define __asm_memendq(haystack,needle,n_qwords)  __libc_memendq(haystack,needle,n_qwords)
#endif /* !__asm_memendq */
#ifndef __asm_memrend
#define __NO_asm_memrend 1
#define __asm_memrend(haystack,needle,n_bytes)   __libc_memrend(haystack,needle,n_bytes)
#endif /* !__asm_memrend */
#ifndef __asm_memrendw
#define __NO_asm_memrendw 1
#define __asm_memrendw(haystack,needle,n_words)  __libc_memrendw(haystack,needle,n_words)
#endif /* !__asm_memrendw */
#ifndef __asm_memrendl
#define __NO_asm_memrendl 1
#define __asm_memrendl(haystack,needle,n_dwords) __libc_memrendl(haystack,needle,n_dwords)
#endif /* !__asm_memrendl */
#ifndef __asm_memrendq
#define __NO_asm_memrendq 1
#define __asm_memrendq(haystack,needle,n_qwords) __libc_memrendq(haystack,needle,n_qwords)
#endif /* !__asm_memrendq */
#ifndef __asm_rawmemchr
#define __NO_asm_rawmemchr 1
#define __asm_rawmemchr(haystack,needle)         __libc_rawmemchr(haystack,needle)
#endif /* !__asm_rawmemchr */
#ifndef __asm_rawmemchrw
#define __NO_asm_rawmemchrw 1
#define __asm_rawmemchrw(haystack,needle)        __libc_rawmemchrw(haystack,needle)
#endif /* !__asm_rawmemchrw */
#ifndef __asm_rawmemchrl
#define __NO_asm_rawmemchrl 1
#define __asm_rawmemchrl(haystack,needle)        __libc_rawmemchrl(haystack,needle)
#endif /* !__asm_rawmemchrl */
#ifndef __asm_rawmemchrq
#define __NO_asm_rawmemchrq 1
#define __asm_rawmemchrq(haystack,needle)        __libc_rawmemchrq(haystack,needle)
#endif /* !__asm_rawmemchrq */
#ifndef __asm_rawmemrchr
#define __NO_asm_rawmemrchr 1
#define __asm_rawmemrchr(haystack,needle)        __libc_rawmemrchr(haystack,needle)
#endif /* !__asm_rawmemrchr */
#ifndef __asm_rawmemrchrw
#define __NO_asm_rawmemrchrw 1
#define __asm_rawmemrchrw(haystack,needle)       __libc_rawmemrchrw(haystack,needle)
#endif /* !__asm_rawmemrchrw */
#ifndef __asm_rawmemrchrl
#define __NO_asm_rawmemrchrl 1
#define __asm_rawmemrchrl(haystack,needle)       __libc_rawmemrchrl(haystack,needle)
#endif /* !__asm_rawmemrchrl */
#ifndef __asm_rawmemrchrq
#define __NO_asm_rawmemrchrq 1
#define __asm_rawmemrchrq(haystack,needle)       __libc_rawmemrchrq(haystack,needle)
#endif /* !__asm_rawmemrchrq */
#ifndef __asm_memlen
#define __NO_asm_memlen 1
#define __asm_memlen(haystack,needle,n_bytes)    __libc_memlen(haystack,needle,n_bytes)
#endif /* !__asm_memlen */
#ifndef __asm_memlenw
#define __NO_asm_memlenw 1
#define __asm_memlenw(haystack,needle,n_words)   __libc_memlenw(haystack,needle,n_words)
#endif /* !__asm_memlenw */
#ifndef __asm_memlenl
#define __NO_asm_memlenl 1
#define __asm_memlenl(haystack,needle,n_dwords)  __libc_memlenl(haystack,needle,n_dwords)
#endif /* !__asm_memlenl */
#ifndef __asm_memlenq
#define __NO_asm_memlenq 1
#define __asm_memlenq(haystack,needle,n_qwords)  __libc_memlenq(haystack,needle,n_qwords)
#endif /* !__asm_memlenq */
#ifndef __asm_memrlen
#define __NO_asm_memrlen 1
#define __asm_memrlen(haystack,needle,n_bytes)   __libc_memrlen(haystack,needle,n_bytes)
#endif /* !__asm_memrlen */
#ifndef __asm_memrlenw
#define __NO_asm_memrlenw 1
#define __asm_memrlenw(haystack,needle,n_words)  __libc_memrlenw(haystack,needle,n_words)
#endif /* !__asm_memrlenw */
#ifndef __asm_memrlenl
#define __NO_asm_memrlenl 1
#define __asm_memrlenl(haystack,needle,n_dwords) __libc_memrlenl(haystack,needle,n_dwords)
#endif /* !__asm_memrlenl */
#ifndef __asm_memrlenq
#define __NO_asm_memrlenq 1
#define __asm_memrlenq(haystack,needle,n_qwords) __libc_memrlenq(haystack,needle,n_qwords)
#endif /* !__asm_memrlenq */
#ifndef __asm_rawmemlen
#define __NO_asm_rawmemlen 1
#define __asm_rawmemlen(haystack,needle)         __libc_rawmemlen(haystack,needle)
#endif /* !__asm_rawmemlen */
#ifndef __asm_rawmemlenw
#define __NO_asm_rawmemlenw 1
#define __asm_rawmemlenw(haystack,needle)        __libc_rawmemlenw(haystack,needle)
#endif /* !__asm_rawmemlenw */
#ifndef __asm_rawmemlenl
#define __NO_asm_rawmemlenl 1
#define __asm_rawmemlenl(haystack,needle)        __libc_rawmemlenl(haystack,needle)
#endif /* !__asm_rawmemlenl */
#ifndef __asm_rawmemlenq
#define __NO_asm_rawmemlenq 1
#define __asm_rawmemlenq(haystack,needle)        __libc_rawmemlenq(haystack,needle)
#endif /* !__asm_rawmemlenq */
#ifndef __asm_rawmemrlen
#define __NO_asm_rawmemrlen 1
#define __asm_rawmemrlen(haystack,needle)        __libc_rawmemrlen(haystack,needle)
#endif /* !__asm_rawmemrlen */
#ifndef __asm_rawmemrlenw
#define __NO_asm_rawmemrlenw 1
#define __asm_rawmemrlenw(haystack,needle)       __libc_rawmemrlenw(haystack,needle)
#endif /* !__asm_rawmemrlenw */
#ifndef __asm_rawmemrlenl
#define __NO_asm_rawmemrlenl 1
#define __asm_rawmemrlenl(haystack,needle)       __libc_rawmemrlenl(haystack,needle)
#endif /* !__asm_rawmemrlenl */
#ifndef __asm_rawmemrlenq
#define __NO_asm_rawmemrlenq 1
#define __asm_rawmemrlenq(haystack,needle)       __libc_rawmemrlenq(haystack,needle)
#endif /* !__asm_rawmemrlenq */

#ifdef __OPTIMIZE_ASM__
#define __nonconst_memcpy(dst,src,n_bytes)            __asm_memcpy(dst,src,n_bytes)
#define __nonconst_memcpyw(dst,src,n_words)           __asm_memcpyw(dst,src,n_words)
#define __nonconst_memcpyl(dst,src,n_dwords)          __asm_memcpyl(dst,src,n_dwords)
#define __nonconst_memcpyq(dst,src,n_qwords)          __asm_memcpyq(dst,src,n_qwords)
#define __nonconst_mempcpy(dst,src,n_bytes)           __asm_mempcpy(dst,src,n_bytes)
#define __nonconst_mempcpyw(dst,src,n_words)          __asm_mempcpyw(dst,src,n_words)
#define __nonconst_mempcpyl(dst,src,n_dwords)         __asm_mempcpyl(dst,src,n_dwords)
#define __nonconst_mempcpyq(dst,src,n_qwords)         __asm_mempcpyq(dst,src,n_qwords)
#define __nonconst_memmove(dst,src,n_bytes)           __asm_memmove(dst,src,n_bytes)
#define __nonconst_memmovew(dst,src,n_words)          __asm_memmovew(dst,src,n_words)
#define __nonconst_memmovel(dst,src,n_dwords)         __asm_memmovel(dst,src,n_dwords)
#define __nonconst_memmoveq(dst,src,n_qwords)         __asm_memmoveq(dst,src,n_qwords)
#define __nonconst_memset(dst,byte,n_bytes)           __asm_memset(dst,byte,n_bytes)
#define __nonconst_memsetw(dst,word,n_words)          __asm_memsetw(dst,word,n_words)
#define __nonconst_memsetl(dst,dword,n_dwords)        __asm_memsetl(dst,dword,n_dwords)
#define __nonconst_memsetq(dst,qword,n_qwords)        __asm_memsetq(dst,qword,n_qwords)
#define __nonconst_memcmp(a,b,n_bytes)                __asm_memcmp(a,b,n_bytes)
#define __nonconst_memcmpw(a,b,n_words)               __asm_memcmpw(a,b,n_words)
#define __nonconst_memcmpl(a,b,n_dwords)              __asm_memcmpl(a,b,n_dwords)
#define __nonconst_memcmpq(a,b,n_qwords)              __asm_memcmpq(a,b,n_qwords)
#define __nonconst_memchr(haystack,needle,n_bytes)    __asm_memchr(haystack,needle,n_bytes)
#define __nonconst_memchrw(haystack,needle,n_words)   __asm_memchrw(haystack,needle,n_words)
#define __nonconst_memchrl(haystack,needle,n_dwords)  __asm_memchrl(haystack,needle,n_dwords)
#define __nonconst_memchrq(haystack,needle,n_qwords)  __asm_memchrq(haystack,needle,n_qwords)
#define __nonconst_memrchr(haystack,needle,n_bytes)   __asm_memrchr(haystack,needle,n_bytes)
#define __nonconst_memrchrw(haystack,needle,n_words)  __asm_memrchrw(haystack,needle,n_words)
#define __nonconst_memrchrl(haystack,needle,n_dwords) __asm_memrchrl(haystack,needle,n_dwords)
#define __nonconst_memrchrq(haystack,needle,n_qwords) __asm_memrchrq(haystack,needle,n_qwords)
#define __nonconst_memend(haystack,needle,n_bytes)    __asm_memend(haystack,needle,n_bytes)
#define __nonconst_memendw(haystack,needle,n_words)   __asm_memendw(haystack,needle,n_words)
#define __nonconst_memendl(haystack,needle,n_dwords)  __asm_memendl(haystack,needle,n_dwords)
#define __nonconst_memendq(haystack,needle,n_qwords)  __asm_memendq(haystack,needle,n_qwords)
#define __nonconst_memrend(haystack,needle,n_bytes)   __asm_memrend(haystack,needle,n_bytes)
#define __nonconst_memrendw(haystack,needle,n_words)  __asm_memrendw(haystack,needle,n_words)
#define __nonconst_memrendl(haystack,needle,n_dwords) __asm_memrendl(haystack,needle,n_dwords)
#define __nonconst_memrendq(haystack,needle,n_qwords) __asm_memrendq(haystack,needle,n_qwords)
#define __nonconst_rawmemchr(haystack,needle)         __asm_rawmemchr(haystack,needle)
#define __nonconst_rawmemchrw(haystack,needle)        __asm_rawmemchrw(haystack,needle)
#define __nonconst_rawmemchrl(haystack,needle)        __asm_rawmemchrl(haystack,needle)
#define __nonconst_rawmemchrq(haystack,needle)        __asm_rawmemchrq(haystack,needle)
#define __nonconst_rawmemrchr(haystack,needle)        __asm_rawmemrchr(haystack,needle)
#define __nonconst_rawmemrchrw(haystack,needle)       __asm_rawmemrchrw(haystack,needle)
#define __nonconst_rawmemrchrl(haystack,needle)       __asm_rawmemrchrl(haystack,needle)
#define __nonconst_rawmemrchrq(haystack,needle)       __asm_rawmemrchrq(haystack,needle)
#define __nonconst_memlen(haystack,needle,n_bytes)    __asm_memlen(haystack,needle,n_bytes)
#define __nonconst_memlenw(haystack,needle,n_words)   __asm_memlenw(haystack,needle,n_words)
#define __nonconst_memlenl(haystack,needle,n_dwords)  __asm_memlenl(haystack,needle,n_dwords)
#define __nonconst_memlenq(haystack,needle,n_qwords)  __asm_memlenq(haystack,needle,n_qwords)
#define __nonconst_memrlen(haystack,needle,n_bytes)   __asm_memrlen(haystack,needle,n_bytes)
#define __nonconst_memrlenw(haystack,needle,n_words)  __asm_memrlenw(haystack,needle,n_words)
#define __nonconst_memrlenl(haystack,needle,n_dwords) __asm_memrlenl(haystack,needle,n_dwords)
#define __nonconst_memrlenq(haystack,needle,n_qwords) __asm_memrlenq(haystack,needle,n_qwords)
#define __nonconst_rawmemlen(haystack,needle)         __asm_rawmemlen(haystack,needle)
#define __nonconst_rawmemlenw(haystack,needle)        __asm_rawmemlenw(haystack,needle)
#define __nonconst_rawmemlenl(haystack,needle)        __asm_rawmemlenl(haystack,needle)
#define __nonconst_rawmemlenq(haystack,needle)        __asm_rawmemlenq(haystack,needle)
#define __nonconst_rawmemrlen(haystack,needle)        __asm_rawmemrlen(haystack,needle)
#define __nonconst_rawmemrlenw(haystack,needle)       __asm_rawmemrlenw(haystack,needle)
#define __nonconst_rawmemrlenl(haystack,needle)       __asm_rawmemrlenl(haystack,needle)
#define __nonconst_rawmemrlenq(haystack,needle)       __asm_rawmemrlenq(haystack,needle)
#else /* __OPTIMIZE_ASM__ */
#define __nonconst_memcpy(dst,src,n_bytes)            __libc_memcpy(dst,src,n_bytes)
#define __nonconst_memcpyw(dst,src,n_words)           __libc_memcpyw(dst,src,n_words)
#define __nonconst_memcpyl(dst,src,n_dwords)          __libc_memcpyl(dst,src,n_dwords)
#define __nonconst_memcpyq(dst,src,n_qwords)          __libc_memcpyq(dst,src,n_qwords)
#define __nonconst_mempcpy(dst,src,n_bytes)           __libc_mempcpy(dst,src,n_bytes)
#define __nonconst_mempcpyw(dst,src,n_words)          __libc_mempcpyw(dst,src,n_words)
#define __nonconst_mempcpyl(dst,src,n_dwords)         __libc_mempcpyl(dst,src,n_dwords)
#define __nonconst_mempcpyq(dst,src,n_qwords)         __libc_mempcpyq(dst,src,n_qwords)
#define __nonconst_memmove(dst,src,n_bytes)           __libc_memmove(dst,src,n_bytes)
#define __nonconst_memmovew(dst,src,n_words)          __libc_memmovew(dst,src,n_words)
#define __nonconst_memmovel(dst,src,n_dwords)         __libc_memmovel(dst,src,n_dwords)
#define __nonconst_memmoveq(dst,src,n_qwords)         __libc_memmoveq(dst,src,n_qwords)
#define __nonconst_memset(dst,byte,n_bytes)           __libc_memset(dst,byte,n_bytes)
#define __nonconst_memsetw(dst,word,n_words)          __libc_memsetw(dst,word,n_words)
#define __nonconst_memsetl(dst,dword,n_dwords)        __libc_memsetl(dst,dword,n_dwords)
#define __nonconst_memsetq(dst,qword,n_qwords)        __libc_memsetq(dst,qword,n_qwords)
#define __nonconst_memcmp(a,b,n_bytes)                __libc_memcmp(a,b,n_bytes)
#define __nonconst_memcmpw(a,b,n_words)               __libc_memcmpw(a,b,n_words)
#define __nonconst_memcmpl(a,b,n_dwords)              __libc_memcmpl(a,b,n_dwords)
#define __nonconst_memcmpq(a,b,n_qwords)              __libc_memcmpq(a,b,n_qwords)
#define __nonconst_memchr(haystack,needle,n_bytes)    __libc_memchr(haystack,needle,n_bytes)
#define __nonconst_memchrw(haystack,needle,n_words)   __libc_memchrw(haystack,needle,n_words)
#define __nonconst_memchrl(haystack,needle,n_dwords)  __libc_memchrl(haystack,needle,n_dwords)
#define __nonconst_memchrq(haystack,needle,n_qwords)  __libc_memchrq(haystack,needle,n_qwords)
#define __nonconst_memrchr(haystack,needle,n_bytes)   __libc_memrchr(haystack,needle,n_bytes)
#define __nonconst_memrchrw(haystack,needle,n_words)  __libc_memrchrw(haystack,needle,n_words)
#define __nonconst_memrchrl(haystack,needle,n_dwords) __libc_memrchrl(haystack,needle,n_dwords)
#define __nonconst_memrchrq(haystack,needle,n_qwords) __libc_memrchrq(haystack,needle,n_qwords)
#define __nonconst_memend(haystack,needle,n_bytes)    __libc_memend(haystack,needle,n_bytes)
#define __nonconst_memendw(haystack,needle,n_words)   __libc_memendw(haystack,needle,n_words)
#define __nonconst_memendl(haystack,needle,n_dwords)  __libc_memendl(haystack,needle,n_dwords)
#define __nonconst_memendq(haystack,needle,n_qwords)  __libc_memendq(haystack,needle,n_qwords)
#define __nonconst_memrend(haystack,needle,n_bytes)   __libc_memrend(haystack,needle,n_bytes)
#define __nonconst_memrendw(haystack,needle,n_words)  __libc_memrendw(haystack,needle,n_words)
#define __nonconst_memrendl(haystack,needle,n_dwords) __libc_memrendl(haystack,needle,n_dwords)
#define __nonconst_memrendq(haystack,needle,n_qwords) __libc_memrendq(haystack,needle,n_qwords)
#define __nonconst_rawmemchr(haystack,needle)         __libc_rawmemchr(haystack,needle)
#define __nonconst_rawmemchrw(haystack,needle)        __libc_rawmemchrw(haystack,needle)
#define __nonconst_rawmemchrl(haystack,needle)        __libc_rawmemchrl(haystack,needle)
#define __nonconst_rawmemchrq(haystack,needle)        __libc_rawmemchrq(haystack,needle)
#define __nonconst_rawmemrchr(haystack,needle)        __libc_rawmemrchr(haystack,needle)
#define __nonconst_rawmemrchrw(haystack,needle)       __libc_rawmemrchrw(haystack,needle)
#define __nonconst_rawmemrchrl(haystack,needle)       __libc_rawmemrchrl(haystack,needle)
#define __nonconst_rawmemrchrq(haystack,needle)       __libc_rawmemrchrq(haystack,needle)
#define __nonconst_memlen(haystack,needle,n_bytes)    __libc_memlen(haystack,needle,n_bytes)
#define __nonconst_memlenw(haystack,needle,n_words)   __libc_memlenw(haystack,needle,n_words)
#define __nonconst_memlenl(haystack,needle,n_dwords)  __libc_memlenl(haystack,needle,n_dwords)
#define __nonconst_memlenq(haystack,needle,n_qwords)  __libc_memlenq(haystack,needle,n_qwords)
#define __nonconst_memrlen(haystack,needle,n_bytes)   __libc_memrlen(haystack,needle,n_bytes)
#define __nonconst_memrlenw(haystack,needle,n_words)  __libc_memrlenw(haystack,needle,n_words)
#define __nonconst_memrlenl(haystack,needle,n_dwords) __libc_memrlenl(haystack,needle,n_dwords)
#define __nonconst_memrlenq(haystack,needle,n_qwords) __libc_memrlenq(haystack,needle,n_qwords)
#define __nonconst_rawmemlen(haystack,needle)         __libc_rawmemlen(haystack,needle)
#define __nonconst_rawmemlenw(haystack,needle)        __libc_rawmemlenw(haystack,needle)
#define __nonconst_rawmemlenl(haystack,needle)        __libc_rawmemlenl(haystack,needle)
#define __nonconst_rawmemlenq(haystack,needle)        __libc_rawmemlenq(haystack,needle)
#define __nonconst_rawmemrlen(haystack,needle)        __libc_rawmemrlen(haystack,needle)
#define __nonconst_rawmemrlenw(haystack,needle)       __libc_rawmemrlenw(haystack,needle)
#define __nonconst_rawmemrlenl(haystack,needle)       __libc_rawmemrlenl(haystack,needle)
#define __nonconst_rawmemrlenq(haystack,needle)       __libc_rawmemrlenq(haystack,needle)
#endif /* !__OPTIMIZE_ASM__ */


#if !defined(__NO_ATTR_FORCEINLINE)

#define __constant_memcpyw(dst,src,n_words)     __constant_memcpy(dst,src,(n_words)*2)
#define __constant_memcpyl(dst,src,n_dwords)    __constant_memcpy(dst,src,(n_dwords)*4)
#define __constant_memcpyq(dst,src,n_qwords)    __constant_memcpy(dst,src,(n_qwords)*8)
__FORCELOCAL __NONNULL((1,2)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memcpy)(void *__restrict __dst,
                                     void const *__restrict __src,
                                     __SIZE_TYPE__ __n_bytes) {
 if (__n_bytes < __SIZEOF_BUSINT__*8) {
  __BUSINT_TYPE__ *__pdst = (__BUSINT_TYPE__ *)__dst;
  __BUSINT_TYPE__ const *__psrc = (__BUSINT_TYPE__ const *)__src;
  /* Use direct copy. */
  switch (__n_bytes/sizeof(__BUSINT_TYPE__)) {
  case 7: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 6: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 5: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 4: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 3: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 2: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 1: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 0: break;
  }
#if __SIZEOF_BUSINT__ > 4
  if (__n_bytes & 4) *(*(__UINT32_TYPE__ **)&__pdst)++ = *(*(__UINT32_TYPE__ const **)&__psrc)++;
#endif
  if (__n_bytes & 2) *(*(__UINT16_TYPE__ **)&__pdst)++ = *(*(__UINT16_TYPE__ const **)&__psrc)++;
  if (__n_bytes & 1) *(__UINT8_TYPE__ *)__pdst = *(__UINT8_TYPE__ const *)__psrc;
  return __dst;
 }
#if __SIZEOF_BUSINT__ >= 8
 if (!(__n_bytes & 7)) return __nonconst_memcpyq(__dst,__src,__n_bytes >> 3);
 if (__n_bytes > __SIZEOF_BUSINT__*256) {
  /* Use 2-step copy for very large, unaligned operations. */
  __nonconst_memcpyq(__dst,__src,__n_bytes >> 3);
  if (__n_bytes & 4) ((__UINT32_TYPE__ *)__dst)[(__n_bytes >> 2)-1] = ((__UINT32_TYPE__ const *)__src)[(__n_bytes >> 2)-1];
  if (__n_bytes & 2) ((__UINT16_TYPE__ *)__dst)[(__n_bytes >> 1)-1] = ((__UINT16_TYPE__ const *)__src)[(__n_bytes >> 1)-1];
  if (__n_bytes & 1) ((__UINT8_TYPE__ *)__dst)[__n_bytes-1] = ((__UINT8_TYPE__ const *)__src)[__n_bytes-1];
  return __dst;
 }
#endif
 if (!(__n_bytes & 3)) return __nonconst_memcpyl(__dst,__src,__n_bytes >> 2);
 if (__n_bytes > __SIZEOF_BUSINT__*128) {
  /* Use 2-step copy for very large, unaligned operations. */
  __nonconst_memcpyl(__dst,__src,__n_bytes >> 2);
  if (__n_bytes & 2) ((__UINT16_TYPE__ *)__dst)[(__n_bytes >> 1)-1] = ((__UINT16_TYPE__ const *)__src)[(__n_bytes >> 1)-1];
  if (__n_bytes & 1) ((__UINT8_TYPE__ *)__dst)[__n_bytes-1] = ((__UINT8_TYPE__ const *)__src)[__n_bytes-1];
  return __dst;
 }
 if (!(__n_bytes & 1)) return __nonconst_memcpyw(__dst,__src,__n_bytes >> 1);
 return __nonconst_memcpy(__dst,__src,__n_bytes);
}
#define __constant_mempcpyw(dst,src,n_words)    __constant_mempcpy(dst,src,(n_words)*2)
#define __constant_mempcpyl(dst,src,n_dwords)   __constant_mempcpy(dst,src,(n_dwords)*4)
#define __constant_mempcpyq(dst,src,n_qwords)   __constant_mempcpy(dst,src,(n_qwords)*8)
__FORCELOCAL __NONNULL((1,2)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_mempcpy)(void *__restrict __dst,
                                      void const *__restrict __src,
                                      __SIZE_TYPE__ __n_bytes) {
 if (__n_bytes < __SIZEOF_BUSINT__*8) {
  __BUSINT_TYPE__ *__pdst = (__BUSINT_TYPE__ *)__dst;
  __BUSINT_TYPE__ const *__psrc = (__BUSINT_TYPE__ const *)__src;
  /* Use direct copy. */
  switch (__n_bytes/sizeof(__SIZEOF_BUSINT__)) {
  case 7: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 6: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 5: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 4: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 3: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 2: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 1: *__pdst++ = *__psrc++; __ATTR_FALLTHROUGH
  case 0: break;
  }
#if __SIZEOF_BUSINT__ > 4
  if (__n_bytes & 4) *(*(__UINT32_TYPE__ **)&__pdst)++ = *(*(__UINT32_TYPE__ const **)&__psrc)++;
#endif
  if (__n_bytes & 2) *(*(__UINT16_TYPE__ **)&__pdst)++ = *(*(__UINT16_TYPE__ const **)&__psrc)++;
  if (__n_bytes & 1) *(*(__UINT8_TYPE__ **)&__pdst)++ = *(__UINT8_TYPE__ const *)__psrc;
  return (__BYTE_TYPE__ *)__pdst;
 }
#if __SIZEOF_BUSINT__ >= 8
 if (!(__n_bytes & 7)) return __nonconst_mempcpyl(__dst,__src,__n_bytes >> 3);
 if (__n_bytes > __SIZEOF_BUSINT__*256) {
  /* Use 2-step copy for very large, unaligned operations. */
  __nonconst_memcpyq(__dst,__src,__n_bytes >> 3);
  if (__n_bytes & 4) ((__UINT32_TYPE__ *)__dst)[(__n_bytes >> 2)-1] = ((__UINT32_TYPE__ const *)__src)[(__n_bytes >> 2)-1];
  if (__n_bytes & 2) ((__UINT16_TYPE__ *)__dst)[(__n_bytes >> 1)-1] = ((__UINT16_TYPE__ const *)__src)[(__n_bytes >> 1)-1];
  if (__n_bytes & 1) ((__UINT8_TYPE__ *)__dst)[__n_bytes-1] = ((__UINT8_TYPE__ const *)__src)[__n_bytes-1];
  return (__BYTE_TYPE__ *)__dst+__n_bytes;
 }
#endif
 if (!(__n_bytes & 3)) return __nonconst_mempcpyl(__dst,__src,__n_bytes >> 2);
 if (__n_bytes > __SIZEOF_BUSINT__*128) {
  /* Use 2-step copy for very large, unaligned operations. */
  __nonconst_memcpyl(__dst,__src,__n_bytes >> 2);
  if (__n_bytes & 2) ((__UINT16_TYPE__ *)__dst)[(__n_bytes >> 1)-1] = ((__UINT16_TYPE__ const *)__src)[(__n_bytes >> 1)-1];
  if (__n_bytes & 1) ((__UINT8_TYPE__ *)__dst)[__n_bytes-1] = ((__UINT8_TYPE__ const *)__src)[__n_bytes-1];
  return (__BYTE_TYPE__ *)__dst+__n_bytes;
 }
 if (!(__n_bytes & 1)) return __nonconst_mempcpyw(__dst,__src,__n_bytes >> 1);
 return __nonconst_mempcpy(__dst,__src,__n_bytes);
}
#define __constant_memmovew(dst,src,n_words)    __constant_memmove(dst,src,(n_words)*2)
#define __constant_memmovel(dst,src,n_dwords)   __constant_memmove(dst,src,(n_dwords)*4)
#define __constant_memmoveq(dst,src,n_qwords)   __constant_memmove(dst,src,(n_qwords)*8)
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
 if (!(__n_bytes & 3)) return __nonconst_memmovel(__dst,__src,__n_bytes >> 2);
 if (!(__n_bytes & 1)) return __nonconst_memmovew(__dst,__src,__n_bytes >> 1);
 return __nonconst_memmove(__dst,__src,__n_bytes);
}
#if __SIZEOF_BUSINT__ >= 8
__FORCELOCAL __NONNULL((1)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memsetq)(void *__restrict __dst, __UINT64_TYPE__ __qword, __SIZE_TYPE__ __n_qwords) {
 if (__n_qwords < 4) {
  __UINT64_TYPE__ *__pdst = (__UINT64_TYPE__ *)__dst;
  switch (__n_qwords) {
  case 3: *__pdst++ = __qword; __ATTR_FALLTHROUGH
  case 2: *__pdst++ = __qword; __ATTR_FALLTHROUGH
  case 1: *__pdst++ = __qword; __ATTR_FALLTHROUGH
  case 0: break;
  }
  return __dst;
 }
 return __nonconst_memsetq(__dst,__qword,__n_qwords);
}
#else
#define __constant_memsetq(dst,qword,n_qwords) \
        __nonconst_memsetq(dst,qword,n_qwords)
#endif
__FORCELOCAL __NONNULL((1)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords) {
#if __SIZEOF_BUSINT__ >= 8
 if (__builtin_constant_p(__dword) || __n_dwords > 256 ||
    (__n_dwords < 16 && __n_dwords > 4) || (__n_dwords > 64 && !(__n_dwords & 1))) {
  __constant_memsetq(__dst,(__UINT32_TYPE__)(__UINT64_C(0x0000000100000001)*__dword),__n_dwords >> 1);
  if (__n_dwords & 1) ((__UINT32_TYPE__ *)__dst)[__n_dwords-1] = __dword;
  return __dst;
 }
#endif
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
 return __nonconst_memsetl(__dst,__dword,__n_dwords);
}
__FORCELOCAL __NONNULL((1)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, __SIZE_TYPE__ __n_words) {
 if (__builtin_constant_p(__word) || __n_words > 256 ||
    (__n_words < 16 && __n_words > 4) || (__n_words > 64 && !(__n_words & 1))) {
  __constant_memsetl(__dst,(__UINT32_TYPE__)(__UINT32_C(0x00010001)*__word),__n_words >> 1);
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
 return __nonconst_memsetw(__dst,__word,__n_words);
}
__FORCELOCAL __NONNULL((1)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memset)(void *__restrict __dst, int __byte, __SIZE_TYPE__ __n_bytes) {
 if (__builtin_constant_p(__byte) || __n_bytes > 256 ||
    (__n_bytes < 16 && __n_bytes > 4) || (__n_bytes > 64 && !(__n_bytes & 1))) {
  __constant_memsetw(__dst,(__UINT16_TYPE__)(__UINT16_C(0x0101)*(__byte & 0xff)),__n_bytes >> 1);
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
#if !defined(__NO_builtin_constant_p) && !defined(__nonconst_bzero) && \
    (defined(__OPTIMIZE_SIZE__) || !defined(__OPTIMIZE_ASM__) || \
     defined(__NO_asm_memset) || defined(__NO_asm_memset_IF_BYTE_NONCONST))
 if (__builtin_constant_p(__byte) && __byte == 0) {
  __libc_bzero(__dst,__n_bytes);
  return __dst;
 }
#endif /* !__nonconst_bzero */
 return __nonconst_memset(__dst,__byte,__n_bytes);
}

#if !defined(__nonconst_bzero) && \
    (defined(__OPTIMIZE_SIZE__) || !defined(__OPTIMIZE_ASM__) || \
     defined(__NO_asm_memset) || defined(__NO_asm_memset_IF_BYTE_NONCONST))
#define __constant_memset2  __constant_memset2
__FORCELOCAL __NONNULL((1)) __ATTR_RETNONNULL
void *(__LIBCCALL __constant_memset2)(void *__restrict __dst, int __byte, __SIZE_TYPE__ __n_bytes) {
 if (__byte == 0) { __libc_bzero(__dst,__n_bytes); return __dst; }
 return __nonconst_memset(__dst,__byte,__n_bytes);
}
#endif /* !__nonconst_bzero */

#define __DEFINE_MEMCMP(name,base,n,T) \
__FORCELOCAL __NONNULL((1,2)) __WUNUSED __ATTR_PURE \
int (__LIBCCALL name)(void const *__a, void const *__b, __SIZE_TYPE__ n) { \
 if (n < 8) { \
  T __temp; \
  T const *__pa = (T const *)__a; \
  T const *__pb = (T const *)__b; \
  switch (n) { \
  case 7: if ((__temp = *__pa++-*__pb++) != 0) return (int)__temp; __ATTR_FALLTHROUGH \
  case 6: if ((__temp = *__pa++-*__pb++) != 0) return (int)__temp; __ATTR_FALLTHROUGH \
  case 5: if ((__temp = *__pa++-*__pb++) != 0) return (int)__temp; __ATTR_FALLTHROUGH \
  case 4: if ((__temp = *__pa++-*__pb++) != 0) return (int)__temp; __ATTR_FALLTHROUGH \
  case 3: if ((__temp = *__pa++-*__pb++) != 0) return (int)__temp; __ATTR_FALLTHROUGH \
  case 2: if ((__temp = *__pa++-*__pb++) != 0) return (int)__temp; __ATTR_FALLTHROUGH \
  case 1: return (int)(*__pa-*__pb); \
  case 0: return 0; \
  } \
 } \
 return base(__a,__b,n); \
}
__DEFINE_MEMCMP(__constant_memcmp,__nonconst_memcmp,__n_bytes,__INT8_TYPE__)
__DEFINE_MEMCMP(__constant_memcmpw,__nonconst_memcmpw,__n_words,__INT16_TYPE__)
__DEFINE_MEMCMP(__constant_memcmpl,__nonconst_memcmpl,__n_dwords,__INT32_TYPE__)
#if __SIZEOF_BUSINT__ >= 8
__DEFINE_MEMCMP(__constant_memcmpq,__nonconst_memcmpq,__n_qwords,__INT64_TYPE__)
#else
#define __constant_memcmpq(a,b,n_qwords) __nonconst_memcmpq(a,b,n_qwords)
#endif
#undef __DEFINE_MEMCMP
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
void *(__LIBCCALL __constant_memchr)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) {
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
 return __nonconst_memchr(__haystack,__needle,__n_bytes);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT16_TYPE__ *(__LIBCCALL __constant_memchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) {
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
 return __nonconst_memchrw(__haystack,__needle,__n_words);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT32_TYPE__ *(__LIBCCALL __constant_memchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) {
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
 return __nonconst_memchrl(__haystack,__needle,__n_dwords);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
void *(__LIBCCALL __constant_memrchr)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) {
 if (__n_bytes < 8) {
  switch (__n_bytes) {
  case 7:  if (((__UINT8_TYPE__ *)__haystack)[6] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+6;
  case 6:  if (((__UINT8_TYPE__ *)__haystack)[5] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+5;
  case 5:  if (((__UINT8_TYPE__ *)__haystack)[4] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+4;
  case 4:  if (((__UINT8_TYPE__ *)__haystack)[3] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+3;
  case 3:  if (((__UINT8_TYPE__ *)__haystack)[2] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+2;
  case 2:  if (((__UINT8_TYPE__ *)__haystack)[1] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+1;
  case 1:  if (((__UINT8_TYPE__ *)__haystack)[0] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack;
  default: break;
  }
  return __NULLPTR;
 }
 return __nonconst_memrchr(__haystack,__needle,__n_bytes);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT16_TYPE__ *(__LIBCCALL __constant_memrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) {
 if (__n_words < 8) {
  switch (__n_words) {
  case 7:  if (((__UINT16_TYPE__ *)__haystack)[6] == __needle) return (__UINT16_TYPE__ *)__haystack+6;
  case 6:  if (((__UINT16_TYPE__ *)__haystack)[5] == __needle) return (__UINT16_TYPE__ *)__haystack+5;
  case 5:  if (((__UINT16_TYPE__ *)__haystack)[4] == __needle) return (__UINT16_TYPE__ *)__haystack+4;
  case 4:  if (((__UINT16_TYPE__ *)__haystack)[3] == __needle) return (__UINT16_TYPE__ *)__haystack+3;
  case 3:  if (((__UINT16_TYPE__ *)__haystack)[2] == __needle) return (__UINT16_TYPE__ *)__haystack+2;
  case 2:  if (((__UINT16_TYPE__ *)__haystack)[1] == __needle) return (__UINT16_TYPE__ *)__haystack+1;
  case 1:  if (((__UINT16_TYPE__ *)__haystack)[0] == __needle) return (__UINT16_TYPE__ *)__haystack;
  default: break;
  }
  return __NULLPTR;
 }
 return __nonconst_memrchrw(__haystack,__needle,__n_words);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT32_TYPE__ *(__LIBCCALL __constant_memrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) {
 if (__n_dwords < 8) {
  switch (__n_dwords) {
  case 7:  if (((__UINT32_TYPE__ *)__haystack)[6] == __needle) return (__UINT32_TYPE__ *)__haystack+6;
  case 6:  if (((__UINT32_TYPE__ *)__haystack)[5] == __needle) return (__UINT32_TYPE__ *)__haystack+5;
  case 5:  if (((__UINT32_TYPE__ *)__haystack)[4] == __needle) return (__UINT32_TYPE__ *)__haystack+4;
  case 4:  if (((__UINT32_TYPE__ *)__haystack)[3] == __needle) return (__UINT32_TYPE__ *)__haystack+3;
  case 3:  if (((__UINT32_TYPE__ *)__haystack)[2] == __needle) return (__UINT32_TYPE__ *)__haystack+2;
  case 2:  if (((__UINT32_TYPE__ *)__haystack)[1] == __needle) return (__UINT32_TYPE__ *)__haystack+1;
  case 1:  if (((__UINT32_TYPE__ *)__haystack)[0] == __needle) return (__UINT32_TYPE__ *)__haystack;
  default: break;
  }
  return __NULLPTR;
 }
 return __nonconst_memrchrl(__haystack,__needle,__n_dwords);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
void *(__LIBCCALL __constant_memend)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) {
 if (__n_bytes < 8) {
  if (__n_bytes >= 1 && ((__UINT8_TYPE__ *)__haystack)[0] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack;
  if (__n_bytes >= 2 && ((__UINT8_TYPE__ *)__haystack)[1] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+1;
  if (__n_bytes >= 3 && ((__UINT8_TYPE__ *)__haystack)[2] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+2;
  if (__n_bytes >= 4 && ((__UINT8_TYPE__ *)__haystack)[3] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+3;
  if (__n_bytes >= 5 && ((__UINT8_TYPE__ *)__haystack)[4] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+4;
  if (__n_bytes >= 6 && ((__UINT8_TYPE__ *)__haystack)[5] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+5;
  if (__n_bytes >= 7 && ((__UINT8_TYPE__ *)__haystack)[6] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+6;
  return (__UINT8_TYPE__ *)__haystack+7;
 }
 return __nonconst_memend(__haystack,__needle,__n_bytes);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT16_TYPE__ *(__LIBCCALL __constant_memendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) {
 if (__n_words < 8) {
  if (__n_words >= 1 && ((__UINT16_TYPE__ *)__haystack)[0] == __needle) return (__UINT16_TYPE__ *)__haystack;
  if (__n_words >= 2 && ((__UINT16_TYPE__ *)__haystack)[1] == __needle) return (__UINT16_TYPE__ *)__haystack+1;
  if (__n_words >= 3 && ((__UINT16_TYPE__ *)__haystack)[2] == __needle) return (__UINT16_TYPE__ *)__haystack+2;
  if (__n_words >= 4 && ((__UINT16_TYPE__ *)__haystack)[3] == __needle) return (__UINT16_TYPE__ *)__haystack+3;
  if (__n_words >= 5 && ((__UINT16_TYPE__ *)__haystack)[4] == __needle) return (__UINT16_TYPE__ *)__haystack+4;
  if (__n_words >= 6 && ((__UINT16_TYPE__ *)__haystack)[5] == __needle) return (__UINT16_TYPE__ *)__haystack+5;
  if (__n_words >= 7 && ((__UINT16_TYPE__ *)__haystack)[6] == __needle) return (__UINT16_TYPE__ *)__haystack+6;
  return (__UINT16_TYPE__ *)__haystack+7;
 }
 return __nonconst_memendw(__haystack,__needle,__n_words);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT32_TYPE__ *(__LIBCCALL __constant_memendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) {
 if (__n_dwords < 8) {
  if (__n_dwords >= 1 && ((__UINT32_TYPE__ *)__haystack)[0] == __needle) return (__UINT32_TYPE__ *)__haystack;
  if (__n_dwords >= 2 && ((__UINT32_TYPE__ *)__haystack)[1] == __needle) return (__UINT32_TYPE__ *)__haystack+1;
  if (__n_dwords >= 3 && ((__UINT32_TYPE__ *)__haystack)[2] == __needle) return (__UINT32_TYPE__ *)__haystack+2;
  if (__n_dwords >= 4 && ((__UINT32_TYPE__ *)__haystack)[3] == __needle) return (__UINT32_TYPE__ *)__haystack+3;
  if (__n_dwords >= 5 && ((__UINT32_TYPE__ *)__haystack)[4] == __needle) return (__UINT32_TYPE__ *)__haystack+4;
  if (__n_dwords >= 6 && ((__UINT32_TYPE__ *)__haystack)[5] == __needle) return (__UINT32_TYPE__ *)__haystack+5;
  if (__n_dwords >= 7 && ((__UINT32_TYPE__ *)__haystack)[6] == __needle) return (__UINT32_TYPE__ *)__haystack+6;
  return (__UINT32_TYPE__ *)__haystack+7;
 }
 return __nonconst_memendl(__haystack,__needle,__n_dwords);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
void *(__LIBCCALL __constant_memrend)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) {
 if (__n_bytes < 8) {
  switch (__n_bytes) {
  case 7:  if (((__UINT8_TYPE__ *)__haystack)[6] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+6;
  case 6:  if (((__UINT8_TYPE__ *)__haystack)[5] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+5;
  case 5:  if (((__UINT8_TYPE__ *)__haystack)[4] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+4;
  case 4:  if (((__UINT8_TYPE__ *)__haystack)[3] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+3;
  case 3:  if (((__UINT8_TYPE__ *)__haystack)[2] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+2;
  case 2:  if (((__UINT8_TYPE__ *)__haystack)[1] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack+1;
  case 1:  if (((__UINT8_TYPE__ *)__haystack)[0] == (__UINT8_TYPE__)__needle) return (__UINT8_TYPE__ *)__haystack;
  default: break;
  }
  return (__UINT8_TYPE__ *)__haystack-1;
 }
 return __nonconst_memrend(__haystack,__needle,__n_bytes);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT16_TYPE__ *(__LIBCCALL __constant_memrendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) {
 if (__n_words < 8) {
  switch (__n_words) {
  case 7:  if (((__UINT16_TYPE__ *)__haystack)[6] == __needle) return (__UINT16_TYPE__ *)__haystack+6;
  case 6:  if (((__UINT16_TYPE__ *)__haystack)[5] == __needle) return (__UINT16_TYPE__ *)__haystack+5;
  case 5:  if (((__UINT16_TYPE__ *)__haystack)[4] == __needle) return (__UINT16_TYPE__ *)__haystack+4;
  case 4:  if (((__UINT16_TYPE__ *)__haystack)[3] == __needle) return (__UINT16_TYPE__ *)__haystack+3;
  case 3:  if (((__UINT16_TYPE__ *)__haystack)[2] == __needle) return (__UINT16_TYPE__ *)__haystack+2;
  case 2:  if (((__UINT16_TYPE__ *)__haystack)[1] == __needle) return (__UINT16_TYPE__ *)__haystack+1;
  case 1:  if (((__UINT16_TYPE__ *)__haystack)[0] == __needle) return (__UINT16_TYPE__ *)__haystack;
  default: break;
  }
  return (__UINT16_TYPE__ *)__haystack-1;
 }
 return __nonconst_memrendw(__haystack,__needle,__n_words);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT32_TYPE__ *(__LIBCCALL __constant_memrendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) {
 if (__n_dwords < 8) {
  switch (__n_dwords) {
  case 7:  if (((__UINT32_TYPE__ *)__haystack)[6] == __needle) return (__UINT32_TYPE__ *)__haystack+6;
  case 6:  if (((__UINT32_TYPE__ *)__haystack)[5] == __needle) return (__UINT32_TYPE__ *)__haystack+5;
  case 5:  if (((__UINT32_TYPE__ *)__haystack)[4] == __needle) return (__UINT32_TYPE__ *)__haystack+4;
  case 4:  if (((__UINT32_TYPE__ *)__haystack)[3] == __needle) return (__UINT32_TYPE__ *)__haystack+3;
  case 3:  if (((__UINT32_TYPE__ *)__haystack)[2] == __needle) return (__UINT32_TYPE__ *)__haystack+2;
  case 2:  if (((__UINT32_TYPE__ *)__haystack)[1] == __needle) return (__UINT32_TYPE__ *)__haystack+1;
  case 1:  if (((__UINT32_TYPE__ *)__haystack)[0] == __needle) return (__UINT32_TYPE__ *)__haystack;
  default: break;
  }
  return (__UINT32_TYPE__ *)__haystack-1;
 }
 return __nonconst_memrendl(__haystack,__needle,__n_dwords);
}

__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__ (__LIBCCALL __constant_memlen)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) {
 if (__n_bytes < 8) {
  if (__n_bytes >= 1 && ((__UINT8_TYPE__ *)__haystack)[0] == (__UINT8_TYPE__)__needle) return 0;
  if (__n_bytes >= 2 && ((__UINT8_TYPE__ *)__haystack)[1] == (__UINT8_TYPE__)__needle) return 1;
  if (__n_bytes >= 3 && ((__UINT8_TYPE__ *)__haystack)[2] == (__UINT8_TYPE__)__needle) return 2;
  if (__n_bytes >= 4 && ((__UINT8_TYPE__ *)__haystack)[3] == (__UINT8_TYPE__)__needle) return 3;
  if (__n_bytes >= 5 && ((__UINT8_TYPE__ *)__haystack)[4] == (__UINT8_TYPE__)__needle) return 4;
  if (__n_bytes >= 6 && ((__UINT8_TYPE__ *)__haystack)[5] == (__UINT8_TYPE__)__needle) return 5;
  if (__n_bytes >= 7 && ((__UINT8_TYPE__ *)__haystack)[6] == (__UINT8_TYPE__)__needle) return 6;
  return 7;
 }
 return __nonconst_memlen(__haystack,__needle,__n_bytes);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__ (__LIBCCALL __constant_memlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) {
 if (__n_words < 8) {
  if (__n_words >= 1 && ((__UINT16_TYPE__ *)__haystack)[0] == __needle) return 0;
  if (__n_words >= 2 && ((__UINT16_TYPE__ *)__haystack)[1] == __needle) return 1;
  if (__n_words >= 3 && ((__UINT16_TYPE__ *)__haystack)[2] == __needle) return 2;
  if (__n_words >= 4 && ((__UINT16_TYPE__ *)__haystack)[3] == __needle) return 3;
  if (__n_words >= 5 && ((__UINT16_TYPE__ *)__haystack)[4] == __needle) return 4;
  if (__n_words >= 6 && ((__UINT16_TYPE__ *)__haystack)[5] == __needle) return 5;
  if (__n_words >= 7 && ((__UINT16_TYPE__ *)__haystack)[6] == __needle) return 6;
  return 7;
 }
 return __nonconst_memlenw(__haystack,__needle,__n_words);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__ (__LIBCCALL __constant_memlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) {
 if (__n_dwords < 8) {
  if (__n_dwords >= 1 && ((__UINT32_TYPE__ *)__haystack)[0] == __needle) return 0;
  if (__n_dwords >= 2 && ((__UINT32_TYPE__ *)__haystack)[1] == __needle) return 1;
  if (__n_dwords >= 3 && ((__UINT32_TYPE__ *)__haystack)[2] == __needle) return 2;
  if (__n_dwords >= 4 && ((__UINT32_TYPE__ *)__haystack)[3] == __needle) return 3;
  if (__n_dwords >= 5 && ((__UINT32_TYPE__ *)__haystack)[4] == __needle) return 4;
  if (__n_dwords >= 6 && ((__UINT32_TYPE__ *)__haystack)[5] == __needle) return 5;
  if (__n_dwords >= 7 && ((__UINT32_TYPE__ *)__haystack)[6] == __needle) return 6;
  return 7;
 }
 return __nonconst_memlenl(__haystack,__needle,__n_dwords);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__ (__LIBCCALL __constant_memrlen)(void const *__restrict __haystack, int __needle, __SIZE_TYPE__ __n_bytes) {
 if (__n_bytes < 8) {
  switch (__n_bytes) {
  case 7:  if (((__UINT8_TYPE__ *)__haystack)[6] == (__UINT8_TYPE__)__needle) return 6;
  case 6:  if (((__UINT8_TYPE__ *)__haystack)[5] == (__UINT8_TYPE__)__needle) return 5;
  case 5:  if (((__UINT8_TYPE__ *)__haystack)[4] == (__UINT8_TYPE__)__needle) return 4;
  case 4:  if (((__UINT8_TYPE__ *)__haystack)[3] == (__UINT8_TYPE__)__needle) return 3;
  case 3:  if (((__UINT8_TYPE__ *)__haystack)[2] == (__UINT8_TYPE__)__needle) return 2;
  case 2:  if (((__UINT8_TYPE__ *)__haystack)[1] == (__UINT8_TYPE__)__needle) return 1;
  case 1:  if (((__UINT8_TYPE__ *)__haystack)[0] == (__UINT8_TYPE__)__needle) return 0;
  default: break;
  }
  return (__SIZE_TYPE__)-1;
 }
 return __nonconst_memrlen(__haystack,__needle,__n_bytes);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__ (__LIBCCALL __constant_memrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, __SIZE_TYPE__ __n_words) {
 if (__n_words < 8) {
  switch (__n_words) {
  case 7:  if (((__UINT16_TYPE__ *)__haystack)[6] == __needle) return 6;
  case 6:  if (((__UINT16_TYPE__ *)__haystack)[5] == __needle) return 5;
  case 5:  if (((__UINT16_TYPE__ *)__haystack)[4] == __needle) return 4;
  case 4:  if (((__UINT16_TYPE__ *)__haystack)[3] == __needle) return 3;
  case 3:  if (((__UINT16_TYPE__ *)__haystack)[2] == __needle) return 2;
  case 2:  if (((__UINT16_TYPE__ *)__haystack)[1] == __needle) return 1;
  case 1:  if (((__UINT16_TYPE__ *)__haystack)[0] == __needle) return 0;
  default: break;
  }
  return (__SIZE_TYPE__)-1;
 }
 return __nonconst_memrlenw(__haystack,__needle,__n_words);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__(__LIBCCALL __constant_memrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, __SIZE_TYPE__ __n_dwords) {
 if (__n_dwords < 8) {
  switch (__n_dwords) {
  case 7:  if (((__UINT32_TYPE__ *)__haystack)[6] == __needle) return 6;
  case 6:  if (((__UINT32_TYPE__ *)__haystack)[5] == __needle) return 5;
  case 5:  if (((__UINT32_TYPE__ *)__haystack)[4] == __needle) return 4;
  case 4:  if (((__UINT32_TYPE__ *)__haystack)[3] == __needle) return 3;
  case 3:  if (((__UINT32_TYPE__ *)__haystack)[2] == __needle) return 2;
  case 2:  if (((__UINT32_TYPE__ *)__haystack)[1] == __needle) return 1;
  case 1:  if (((__UINT32_TYPE__ *)__haystack)[0] == __needle) return 0;
  default: break;
  }
  return (__SIZE_TYPE__)-1;
 }
 return __nonconst_memrlenl(__haystack,__needle,__n_dwords);
}

#if __SIZEOF_BUSINT__ >= 8
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT64_TYPE__ *(__LIBCCALL __constant_memchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, __SIZE_TYPE__ __n_qwords) {
 if (__n_qwords < 8) {
  if (__n_qwords >= 1 && ((__UINT64_TYPE__ *)__haystack)[0] == __needle) return (__UINT64_TYPE__ *)__haystack;
  if (__n_qwords >= 2 && ((__UINT64_TYPE__ *)__haystack)[1] == __needle) return (__UINT64_TYPE__ *)__haystack+1;
  if (__n_qwords >= 3 && ((__UINT64_TYPE__ *)__haystack)[2] == __needle) return (__UINT64_TYPE__ *)__haystack+2;
  if (__n_qwords >= 4 && ((__UINT64_TYPE__ *)__haystack)[3] == __needle) return (__UINT64_TYPE__ *)__haystack+3;
  if (__n_qwords >= 5 && ((__UINT64_TYPE__ *)__haystack)[4] == __needle) return (__UINT64_TYPE__ *)__haystack+4;
  if (__n_qwords >= 6 && ((__UINT64_TYPE__ *)__haystack)[5] == __needle) return (__UINT64_TYPE__ *)__haystack+5;
  if (__n_qwords >= 7 && ((__UINT64_TYPE__ *)__haystack)[6] == __needle) return (__UINT64_TYPE__ *)__haystack+6;
  return __NULLPTR;
 }
 return __nonconst_memchrq(__haystack,__needle,__n_qwords);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT64_TYPE__ *(__LIBCCALL __constant_memrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, __SIZE_TYPE__ __n_qwords) {
 if (__n_qwords < 8) {
  switch (__n_qwords) {
  case 7:  if (((__UINT64_TYPE__ *)__haystack)[6] == __needle) return (__UINT64_TYPE__ *)__haystack+6;
  case 6:  if (((__UINT64_TYPE__ *)__haystack)[5] == __needle) return (__UINT64_TYPE__ *)__haystack+5;
  case 5:  if (((__UINT64_TYPE__ *)__haystack)[4] == __needle) return (__UINT64_TYPE__ *)__haystack+4;
  case 4:  if (((__UINT64_TYPE__ *)__haystack)[3] == __needle) return (__UINT64_TYPE__ *)__haystack+3;
  case 3:  if (((__UINT64_TYPE__ *)__haystack)[2] == __needle) return (__UINT64_TYPE__ *)__haystack+2;
  case 2:  if (((__UINT64_TYPE__ *)__haystack)[1] == __needle) return (__UINT64_TYPE__ *)__haystack+1;
  case 1:  if (((__UINT64_TYPE__ *)__haystack)[0] == __needle) return (__UINT64_TYPE__ *)__haystack;
  default: break;
  }
  return __NULLPTR;
 }
 return __nonconst_memrchrq(__haystack,__needle,__n_qwords);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT64_TYPE__ *(__LIBCCALL __constant_memendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, __SIZE_TYPE__ __n_qwords) {
 if (__n_qwords < 8) {
  if (__n_qwords >= 1 && ((__UINT64_TYPE__ *)__haystack)[0] == __needle) return (__UINT64_TYPE__ *)__haystack;
  if (__n_qwords >= 2 && ((__UINT64_TYPE__ *)__haystack)[1] == __needle) return (__UINT64_TYPE__ *)__haystack+1;
  if (__n_qwords >= 3 && ((__UINT64_TYPE__ *)__haystack)[2] == __needle) return (__UINT64_TYPE__ *)__haystack+2;
  if (__n_qwords >= 4 && ((__UINT64_TYPE__ *)__haystack)[3] == __needle) return (__UINT64_TYPE__ *)__haystack+3;
  if (__n_qwords >= 5 && ((__UINT64_TYPE__ *)__haystack)[4] == __needle) return (__UINT64_TYPE__ *)__haystack+4;
  if (__n_qwords >= 6 && ((__UINT64_TYPE__ *)__haystack)[5] == __needle) return (__UINT64_TYPE__ *)__haystack+5;
  if (__n_qwords >= 7 && ((__UINT64_TYPE__ *)__haystack)[6] == __needle) return (__UINT64_TYPE__ *)__haystack+6;
  return (__UINT64_TYPE__ *)__haystack+7;
 }
 return __nonconst_memendq(__haystack,__needle,__n_qwords);
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT64_TYPE__ *(__LIBCCALL __constant_memrendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, __SIZE_TYPE__ __n_qwords) {
 if (__n_qwords < 8) {
  switch (__n_qwords) {
  case 7:  if (((__UINT64_TYPE__ *)__haystack)[6] == __needle) return (__UINT64_TYPE__ *)__haystack+6;
  case 6:  if (((__UINT64_TYPE__ *)__haystack)[5] == __needle) return (__UINT64_TYPE__ *)__haystack+5;
  case 5:  if (((__UINT64_TYPE__ *)__haystack)[4] == __needle) return (__UINT64_TYPE__ *)__haystack+4;
  case 4:  if (((__UINT64_TYPE__ *)__haystack)[3] == __needle) return (__UINT64_TYPE__ *)__haystack+3;
  case 3:  if (((__UINT64_TYPE__ *)__haystack)[2] == __needle) return (__UINT64_TYPE__ *)__haystack+2;
  case 2:  if (((__UINT64_TYPE__ *)__haystack)[1] == __needle) return (__UINT64_TYPE__ *)__haystack+1;
  case 1:  if (((__UINT64_TYPE__ *)__haystack)[0] == __needle) return (__UINT64_TYPE__ *)__haystack;
  default: break;
  }
  return (__UINT64_TYPE__ *)__haystack-1;
 }
 return __nonconst_memrendq(__haystack,__needle,__n_qwords);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__ (__LIBCCALL __constant_memlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, __SIZE_TYPE__ __n_qwords) {
 if (__n_qwords < 8) {
  if (__n_qwords >= 1 && ((__UINT64_TYPE__ *)__haystack)[0] == __needle) return 0;
  if (__n_qwords >= 2 && ((__UINT64_TYPE__ *)__haystack)[1] == __needle) return 1;
  if (__n_qwords >= 3 && ((__UINT64_TYPE__ *)__haystack)[2] == __needle) return 2;
  if (__n_qwords >= 4 && ((__UINT64_TYPE__ *)__haystack)[3] == __needle) return 3;
  if (__n_qwords >= 5 && ((__UINT64_TYPE__ *)__haystack)[4] == __needle) return 4;
  if (__n_qwords >= 6 && ((__UINT64_TYPE__ *)__haystack)[5] == __needle) return 5;
  if (__n_qwords >= 7 && ((__UINT64_TYPE__ *)__haystack)[6] == __needle) return 6;
  return 7;
 }
 return __nonconst_memlenq(__haystack,__needle,__n_qwords);
}
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__(__LIBCCALL __constant_memrlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, __SIZE_TYPE__ __n_qwords) {
 if (__n_qwords < 8) {
  switch (__n_qwords) {
  case 7:  if (((__UINT64_TYPE__ *)__haystack)[6] == __needle) return 6;
  case 6:  if (((__UINT64_TYPE__ *)__haystack)[5] == __needle) return 5;
  case 5:  if (((__UINT64_TYPE__ *)__haystack)[4] == __needle) return 4;
  case 4:  if (((__UINT64_TYPE__ *)__haystack)[3] == __needle) return 3;
  case 3:  if (((__UINT64_TYPE__ *)__haystack)[2] == __needle) return 2;
  case 2:  if (((__UINT64_TYPE__ *)__haystack)[1] == __needle) return 1;
  case 1:  if (((__UINT64_TYPE__ *)__haystack)[0] == __needle) return 0;
  default: break;
  }
  return (__SIZE_TYPE__)-1;
 }
 return __nonconst_memrlenq(__haystack,__needle,__n_qwords);
}
#else
#define __constant_memchrq(haystack,needle,n_qwords) \
        __nonconst_memchrq(haystack,needle,n_qwords)
#define __constant_memrchrq(haystack,needle,n_qwords) \
        __nonconst_memrchrq(haystack,needle,n_qwords)
#define __constant_memendq(haystack,needle,n_qwords) \
        __nonconst_memendq(haystack,needle,n_qwords)
#define __constant_memrendq(haystack,needle,n_qwords) \
        __nonconst_memrendq(haystack,needle,n_qwords)
#define __constant_memlenq(haystack,needle,n_qwords) \
        __nonconst_memlenq(haystack,needle,n_qwords)
#define __constant_memrlenq(haystack,needle,n_qwords) \
        __nonconst_memrlenq(haystack,needle,n_qwords)
#endif



#if __SIZEOF_CHAR__ == 1 && \
  (defined(__OPTIMIZE_SIZE__) || !defined(__OPTIMIZE_ASM__) || defined(__NO_asm_rawmemchr))
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
void *(__LIBCCALL __constant_rawmemchr)(void const *__restrict __haystack, int __needle) {
 if (__needle == '\0') return __libc_strend((char const *)__haystack);
 return __nonconst_rawmemchr(__haystack,__needle);
}
#else
#define __constant_rawmemchr(haystack,needle)  __nonconst_rawmemchr(haystack,needle)
#endif
#if __SIZEOF_WCHAR_T__ == 2 && \
  (defined(__OPTIMIZE_SIZE__) || !defined(__OPTIMIZE_ASM__) || defined(__NO_asm_rawmemchrw))
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT16_TYPE__ *(__LIBCCALL __constant_rawmemchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) {
 if (!__needle) return (__UINT16_TYPE__ *)__libc_wcsend((__WCHAR_TYPE__ const *)__haystack);
 return __nonconst_rawmemchrw(__haystack,__needle);
}
#else
#define __constant_rawmemchrw(haystack,needle)  __nonconst_rawmemchrw(haystack,needle)
#endif
#if __SIZEOF_WCHAR_T__ == 4 && \
  (defined(__OPTIMIZE_SIZE__) || !defined(__OPTIMIZE_ASM__) || defined(__NO_asm_rawmemchrl))
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1)) __WUNUSED __ATTR_PURE
__UINT32_TYPE__ *(__LIBCCALL __constant_rawmemchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) {
 if (!__needle) return (__UINT32_TYPE__ *)__libc_wcsend((__WCHAR_TYPE__ const *)__haystack);
 return __nonconst_rawmemchrl(__haystack,__needle);
}
#else
#define __constant_rawmemchrl(haystack,needle)  __nonconst_rawmemchrl(haystack,needle)
#endif
#define __constant_rawmemchrq(haystack,needle)  __nonconst_rawmemchrq(haystack,needle)
#define __constant_rawmemrchr(haystack,needle)  __nonconst_rawmemrchr(haystack,needle)
#define __constant_rawmemrchrw(haystack,needle) __nonconst_rawmemrchrw(haystack,needle)
#define __constant_rawmemrchrl(haystack,needle) __nonconst_rawmemrchrl(haystack,needle)
#define __constant_rawmemrchrq(haystack,needle) __nonconst_rawmemrchrq(haystack,needle)

#if __SIZEOF_CHAR__ == 1 && \
  (defined(__OPTIMIZE_SIZE__) || !defined(__OPTIMIZE_ASM__) || defined(__NO_asm_rawmemlen))
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__ (__LIBCCALL __constant_rawmemlen)(void const *__restrict __haystack, int __needle) {
 if (__needle == '\0') return __libc_strlen((char const *)__haystack);
 return __nonconst_rawmemlen(__haystack,__needle);
}
#else
#define __constant_rawmemlen(haystack,needle)  __nonconst_rawmemlen(haystack,needle)
#endif
#if __SIZEOF_WCHAR_T__ == 2 && \
  (defined(__OPTIMIZE_SIZE__) || !defined(__OPTIMIZE_ASM__) || defined(__NO_asm_rawmemlenw))
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__ (__LIBCCALL __constant_rawmemlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) {
 if (!__needle) return __libc_wcslen((__WCHAR_TYPE__ const *)__haystack);
 return __nonconst_rawmemlenw(__haystack,__needle);
}
#else
#define __constant_rawmemlenw(haystack,needle)  __nonconst_rawmemlenw(haystack,needle)
#endif
#if __SIZEOF_WCHAR_T__ == 4 && \
  (defined(__OPTIMIZE_SIZE__) || !defined(__OPTIMIZE_ASM__) || defined(__NO_asm_rawmemlenl))
__FORCELOCAL __NONNULL((1)) __WUNUSED __ATTR_PURE
__SIZE_TYPE__ (__LIBCCALL __constant_rawmemlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) {
 if (!__needle) return __libc_wcslen((__WCHAR_TYPE__ const *)__haystack);
 return __nonconst_rawmemlenl(__haystack,__needle);
}
#else
#define __constant_rawmemlenl(haystack,needle)  __nonconst_rawmemlenl(haystack,needle)
#endif
#define __constant_rawmemlenq(haystack,needle)  __nonconst_rawmemlenq(haystack,needle)
#define __constant_rawmemrlen(haystack,needle)  __nonconst_rawmemrlen(haystack,needle)
#define __constant_rawmemrlenw(haystack,needle) __nonconst_rawmemrlenw(haystack,needle)
#define __constant_rawmemrlenl(haystack,needle) __nonconst_rawmemrlenl(haystack,needle)
#define __constant_rawmemrlenq(haystack,needle) __nonconst_rawmemrlenq(haystack,needle)

#else
#define __constant_memcpy(dst,src,n_bytes)            __nonconst_memcpy(dst,src,n_bytes)
#define __constant_memcpyw(dst,src,n_words)           __nonconst_memcpyw(dst,src,n_words)
#define __constant_memcpyl(dst,src,n_dwords)          __nonconst_memcpyl(dst,src,n_dwords)
#define __constant_memcpyq(dst,src,n_qwords)          __nonconst_memcpyq(dst,src,n_qwords)
#define __constant_mempcpy(dst,src,n_bytes)           __nonconst_mempcpy(dst,src,n_bytes)
#define __constant_mempcpyw(dst,src,n_words)          __nonconst_mempcpyw(dst,src,n_words)
#define __constant_mempcpyl(dst,src,n_dwords)         __nonconst_mempcpyl(dst,src,n_dwords)
#define __constant_mempcpyq(dst,src,n_qwords)         __nonconst_mempcpyq(dst,src,n_qwords)
#define __constant_memmove(dst,src,n_bytes)           __nonconst_memmove(dst,src,n_bytes)
#define __constant_memmovew(dst,src,n_words)          __nonconst_memmovew(dst,src,n_words)
#define __constant_memmovel(dst,src,n_dwords)         __nonconst_memmovel(dst,src,n_dwords)
#define __constant_memmoveq(dst,src,n_qwords)         __nonconst_memmoveq(dst,src,n_qwords)
#define __constant_memset(dst,byte,n_bytes)           __nonconst_memset(dst,byte,n_bytes)
#define __constant_memsetw(dst,word,n_words)          __nonconst_memsetw(dst,word,n_words)
#define __constant_memsetl(dst,dword,n_dwords)        __nonconst_memsetl(dst,dword,n_dwords)
#define __constant_memsetq(dst,qword,n_qwords)        __nonconst_memsetq(dst,qword,n_qwords)
#define __constant_memcmp(a,b,n_bytes)                __nonconst_memcmp(a,b,n_bytes)
#define __constant_memcmpw(a,b,n_words)               __nonconst_memcmpw(a,b,n_words)
#define __constant_memcmpl(a,b,n_dwords)              __nonconst_memcmpl(a,b,n_dwords)
#define __constant_memcmpq(a,b,n_qwords)              __nonconst_memcmpq(a,b,n_qwords)
#define __constant_memchr(haystack,needle,n_bytes)    __nonconst_memchr(haystack,needle,n_bytes)
#define __constant_memchrw(haystack,needle,n_words)   __nonconst_memchrw(haystack,needle,n_words)
#define __constant_memchrl(haystack,needle,n_dwords)  __nonconst_memchrl(haystack,needle,n_dwords)
#define __constant_memchrq(haystack,needle,n_qwords)  __nonconst_memchrq(haystack,needle,n_qwords)
#define __constant_memrchr(haystack,needle,n_bytes)   __nonconst_memrchr(haystack,needle,n_bytes)
#define __constant_memrchrw(haystack,needle,n_words)  __nonconst_memrchrw(haystack,needle,n_words)
#define __constant_memrchrl(haystack,needle,n_dwords) __nonconst_memrchrl(haystack,needle,n_dwords)
#define __constant_memrchrq(haystack,needle,n_qwords) __nonconst_memrchrq(haystack,needle,n_qwords)
#define __constant_memend(haystack,needle,n_bytes)    __nonconst_memend(haystack,needle,n_bytes)
#define __constant_memendw(haystack,needle,n_words)   __nonconst_memendw(haystack,needle,n_words)
#define __constant_memendl(haystack,needle,n_dwords)  __nonconst_memendl(haystack,needle,n_dwords)
#define __constant_memendq(haystack,needle,n_qwords)  __nonconst_memendq(haystack,needle,n_qwords)
#define __constant_memrend(haystack,needle,n_bytes)   __nonconst_memrend(haystack,needle,n_bytes)
#define __constant_memrendw(haystack,needle,n_words)  __nonconst_memrendw(haystack,needle,n_words)
#define __constant_memrendl(haystack,needle,n_dwords) __nonconst_memrendl(haystack,needle,n_dwords)
#define __constant_memrendq(haystack,needle,n_qwords) __nonconst_memrendq(haystack,needle,n_qwords)
#define __constant_rawmemchr(haystack,needle)         __nonconst_rawmemchr(haystack,needle)
#define __constant_rawmemchrw(haystack,needle)        __nonconst_rawmemchrw(haystack,needle)
#define __constant_rawmemchrl(haystack,needle)        __nonconst_rawmemchrl(haystack,needle)
#define __constant_rawmemchrq(haystack,needle)        __nonconst_rawmemchrq(haystack,needle)
#define __constant_rawmemrchr(haystack,needle)        __nonconst_rawmemrchr(haystack,needle)
#define __constant_rawmemrchrw(haystack,needle)       __nonconst_rawmemrchrw(haystack,needle)
#define __constant_rawmemrchrl(haystack,needle)       __nonconst_rawmemrchrl(haystack,needle)
#define __constant_rawmemrchrq(haystack,needle)       __nonconst_rawmemrchrq(haystack,needle)
#define __constant_memlen(haystack,needle,n_bytes)    __nonconst_memlen(haystack,needle,n_bytes)
#define __constant_memlenw(haystack,needle,n_words)   __nonconst_memlenw(haystack,needle,n_words)
#define __constant_memlenl(haystack,needle,n_dwords)  __nonconst_memlenl(haystack,needle,n_dwords)
#define __constant_memlenq(haystack,needle,n_qwords)  __nonconst_memlenq(haystack,needle,n_qwords)
#define __constant_memrlen(haystack,needle,n_bytes)   __nonconst_memrlen(haystack,needle,n_bytes)
#define __constant_memrlenw(haystack,needle,n_words)  __nonconst_memrlenw(haystack,needle,n_words)
#define __constant_memrlenl(haystack,needle,n_dwords) __nonconst_memrlenl(haystack,needle,n_dwords)
#define __constant_memrlenq(haystack,needle,n_qwords) __nonconst_memrlenq(haystack,needle,n_qwords)
#define __constant_rawmemlen(haystack,needle)         __nonconst_rawmemlen(haystack,needle)
#define __constant_rawmemlenw(haystack,needle)        __nonconst_rawmemlenw(haystack,needle)
#define __constant_rawmemlenl(haystack,needle)        __nonconst_rawmemlenl(haystack,needle)
#define __constant_rawmemlenq(haystack,needle)        __nonconst_rawmemlenq(haystack,needle)
#define __constant_rawmemrlen(haystack,needle)        __nonconst_rawmemrlen(haystack,needle)
#define __constant_rawmemrlenw(haystack,needle)       __nonconst_rawmemrlenw(haystack,needle)
#define __constant_rawmemrlenl(haystack,needle)       __nonconst_rawmemrlenl(haystack,needle)
#define __constant_rawmemrlenq(haystack,needle)       __nonconst_rawmemrlenq(haystack,needle)
#endif

#ifdef __OPTIMIZE_CONST__
#define __opt_memcpy(dst,src,n_bytes)            (__builtin_constant_p(n_bytes) ? __constant_memcpy(dst,src,n_bytes) : __nonconst_memcpy(dst,src,n_bytes))
#define __opt_memcpyw(dst,src,n_words)           (__builtin_constant_p(n_words) ? __constant_memcpyw(dst,src,n_words) : __nonconst_memcpyw(dst,src,n_words))
#define __opt_memcpyl(dst,src,n_dwords)          (__builtin_constant_p(n_dwords) ? __constant_memcpyl(dst,src,n_dwords) : __nonconst_memcpyl(dst,src,n_dwords))
#define __opt_memcpyq(dst,src,n_qwords)          (__builtin_constant_p(n_qwords) ? __constant_memcpyq(dst,src,n_qwords) : __nonconst_memcpyq(dst,src,n_qwords))
#define __opt_mempcpy(dst,src,n_bytes)           (__builtin_constant_p(n_bytes) ? __constant_mempcpy(dst,src,n_bytes) : __nonconst_mempcpy(dst,src,n_bytes))
#define __opt_mempcpyw(dst,src,n_words)          (__builtin_constant_p(n_words) ? __constant_mempcpyw(dst,src,n_words) : __nonconst_mempcpyw(dst,src,n_words))
#define __opt_mempcpyl(dst,src,n_dwords)         (__builtin_constant_p(n_dwords) ? __constant_mempcpyl(dst,src,n_dwords) : __nonconst_mempcpyl(dst,src,n_dwords))
#define __opt_mempcpyq(dst,src,n_qwords)         (__builtin_constant_p(n_qwords) ? __constant_mempcpyq(dst,src,n_qwords) : __nonconst_mempcpyq(dst,src,n_qwords))
#define __opt_memmove(dst,src,n_bytes)           (__builtin_constant_p(n_bytes) ? __constant_memmove(dst,src,n_bytes) : __nonconst_memmove(dst,src,n_bytes))
#define __opt_memmovew(dst,src,n_words)          (__builtin_constant_p(n_words) ? __constant_memmovew(dst,src,n_words) : __nonconst_memmovew(dst,src,n_words))
#define __opt_memmovel(dst,src,n_dwords)         (__builtin_constant_p(n_dwords) ? __constant_memmovel(dst,src,n_dwords) : __nonconst_memmovel(dst,src,n_dwords))
#define __opt_memmoveq(dst,src,n_qwords)         (__builtin_constant_p(n_qwords) ? __constant_memmoveq(dst,src,n_qwords) : __nonconst_memmoveq(dst,src,n_qwords))
#ifdef __constant_memset2 /* Additional optimization for 'memset(p,0,s)' -> 'bzero(p,s)' */
#define __opt_memset(dst,byte,n_bytes)           (__builtin_constant_p(n_bytes) ? __constant_memset(dst,byte,n_bytes) : __builtin_constant_p(byte) ? __constant_memset2(dst,byte,n_bytes) : __nonconst_memset(dst,byte,n_bytes))
#else
#define __opt_memset(dst,byte,n_bytes)           (__builtin_constant_p(n_bytes) ? __constant_memset(dst,byte,n_bytes) : __nonconst_memset(dst,byte,n_bytes))
#endif
#define __opt_memsetw(dst,word,n_words)          (__builtin_constant_p(n_words) ? __constant_memsetw(dst,word,n_words) : __nonconst_memsetw(dst,word,n_words))
#define __opt_memsetl(dst,dword,n_dwords)        (__builtin_constant_p(n_dwords) ? __constant_memsetl(dst,dword,n_dwords) : __nonconst_memsetl(dst,dword,n_dwords))
#define __opt_memsetq(dst,qword,n_qwords)        (__builtin_constant_p(n_qwords) ? __constant_memsetq(dst,qword,n_qwords) : __nonconst_memsetq(dst,qword,n_qwords))
#define __opt_memcmp(a,b,n_bytes)                (__builtin_constant_p(n_bytes) ? __constant_memcmp(a,b,n_bytes) : __nonconst_memcmp(a,b,n_bytes))
#define __opt_memcmpw(a,b,n_words)               (__builtin_constant_p(n_words) ? __constant_memcmpw(a,b,n_words) : __nonconst_memcmpw(a,b,n_words))
#define __opt_memcmpl(a,b,n_dwords)              (__builtin_constant_p(n_dwords) ? __constant_memcmpl(a,b,n_dwords) : __nonconst_memcmpl(a,b,n_dwords))
#define __opt_memcmpq(a,b,n_qwords)              (__builtin_constant_p(n_qwords) ? __constant_memcmpq(a,b,n_qwords) : __nonconst_memcmpq(a,b,n_qwords))
#define __opt_memchr(haystack,needle,n_bytes)    (__builtin_constant_p(n_bytes) ? __constant_memchr(haystack,needle,n_bytes) : __nonconst_memchr(haystack,needle,n_bytes))
#define __opt_memchrw(haystack,needle,n_words)   (__builtin_constant_p(n_words) ? __constant_memchrw(haystack,needle,n_words) : __nonconst_memchrw(haystack,needle,n_words))
#define __opt_memchrl(haystack,needle,n_dwords)  (__builtin_constant_p(n_dwords) ? __constant_memchrl(haystack,needle,n_dwords) : __nonconst_memchrl(haystack,needle,n_dwords))
#define __opt_memchrq(haystack,needle,n_qwords)  (__builtin_constant_p(n_qwords) ? __constant_memchrq(haystack,needle,n_qwords) : __nonconst_memchrq(haystack,needle,n_qwords))
#define __opt_memrchr(haystack,needle,n_bytes)   (__builtin_constant_p(n_bytes) ? __constant_memrchr(haystack,needle,n_bytes) : __nonconst_memrchr(haystack,needle,n_bytes))
#define __opt_memrchrw(haystack,needle,n_words)  (__builtin_constant_p(n_words) ? __constant_memrchrw(haystack,needle,n_words) : __nonconst_memrchrw(haystack,needle,n_words))
#define __opt_memrchrl(haystack,needle,n_dwords) (__builtin_constant_p(n_dwords) ? __constant_memrchrl(haystack,needle,n_dwords) : __nonconst_memrchrl(haystack,needle,n_dwords))
#define __opt_memrchrq(haystack,needle,n_qwords) (__builtin_constant_p(n_qwords) ? __constant_memrchrq(haystack,needle,n_qwords) : __nonconst_memrchrq(haystack,needle,n_qwords))
#define __opt_memend(haystack,needle,n_bytes)    (__builtin_constant_p(n_bytes) ? __constant_memend(haystack,needle,n_bytes) : __nonconst_memend(haystack,needle,n_bytes))
#define __opt_memendw(haystack,needle,n_words)   (__builtin_constant_p(n_words) ? __constant_memendw(haystack,needle,n_words) : __nonconst_memendw(haystack,needle,n_words))
#define __opt_memendl(haystack,needle,n_dwords)  (__builtin_constant_p(n_dwords) ? __constant_memendl(haystack,needle,n_dwords) : __nonconst_memendl(haystack,needle,n_dwords))
#define __opt_memendq(haystack,needle,n_qwords)  (__builtin_constant_p(n_qwords) ? __constant_memendq(haystack,needle,n_qwords) : __nonconst_memendq(haystack,needle,n_qwords))
#define __opt_memrend(haystack,needle,n_bytes)   (__builtin_constant_p(n_bytes) ? __constant_memrend(haystack,needle,n_bytes) : __nonconst_memrend(haystack,needle,n_bytes))
#define __opt_memrendw(haystack,needle,n_words)  (__builtin_constant_p(n_words) ? __constant_memrendw(haystack,needle,n_words) : __nonconst_memrendw(haystack,needle,n_words))
#define __opt_memrendl(haystack,needle,n_dwords) (__builtin_constant_p(n_dwords) ? __constant_memrendl(haystack,needle,n_dwords) : __nonconst_memrendl(haystack,needle,n_dwords))
#define __opt_memrendq(haystack,needle,n_qwords) (__builtin_constant_p(n_qwords) ? __constant_memrendq(haystack,needle,n_qwords) : __nonconst_memrendq(haystack,needle,n_qwords))
#define __opt_rawmemchr(haystack,needle)         (__builtin_constant_p(needle) ? __constant_rawmemchr(haystack,needle) : __nonconst_rawmemchr(haystack,needle))
#define __opt_rawmemchrw(haystack,needle)        (__builtin_constant_p(needle) ? __constant_rawmemchrw(haystack,needle) : __nonconst_rawmemchrw(haystack,needle))
#define __opt_rawmemchrl(haystack,needle)        (__builtin_constant_p(needle) ? __constant_rawmemchrl(haystack,needle) : __nonconst_rawmemchrl(haystack,needle))
#define __opt_rawmemchrq(haystack,needle)        (__builtin_constant_p(needle) ? __constant_rawmemchrq(haystack,needle) : __nonconst_rawmemchrq(haystack,needle))
#define __opt_rawmemrchr(haystack,needle)        (__builtin_constant_p(needle) ? __constant_rawmemrchr(haystack,needle) : __nonconst_rawmemrchr(haystack,needle))
#define __opt_rawmemrchrw(haystack,needle)       (__builtin_constant_p(needle) ? __constant_rawmemrchrw(haystack,needle) : __nonconst_rawmemrchrw(haystack,needle))
#define __opt_rawmemrchrl(haystack,needle)       (__builtin_constant_p(needle) ? __constant_rawmemrchrl(haystack,needle) : __nonconst_rawmemrchrl(haystack,needle))
#define __opt_rawmemrchrq(haystack,needle)       (__builtin_constant_p(needle) ? __constant_rawmemrchrq(haystack,needle) : __nonconst_rawmemrchrq(haystack,needle))
#define __opt_memlen(haystack,needle,n_bytes)    (__builtin_constant_p(n_bytes) ? __constant_memlen(haystack,needle,n_bytes) : __nonconst_memlen(haystack,needle,n_bytes))
#define __opt_memlenw(haystack,needle,n_words)   (__builtin_constant_p(n_words) ? __constant_memlenw(haystack,needle,n_words) : __nonconst_memlenw(haystack,needle,n_words))
#define __opt_memlenl(haystack,needle,n_dwords)  (__builtin_constant_p(n_dwords) ? __constant_memlenl(haystack,needle,n_dwords) : __nonconst_memlenl(haystack,needle,n_dwords))
#define __opt_memlenq(haystack,needle,n_qwords)  (__builtin_constant_p(n_qwords) ? __constant_memlenq(haystack,needle,n_qwords) : __nonconst_memlenq(haystack,needle,n_qwords))
#define __opt_memrlen(haystack,needle,n_bytes)   (__builtin_constant_p(n_bytes) ? __constant_memrlen(haystack,needle,n_bytes) : __nonconst_memrlen(haystack,needle,n_bytes))
#define __opt_memrlenw(haystack,needle,n_words)  (__builtin_constant_p(n_words) ? __constant_memrlenw(haystack,needle,n_words) : __nonconst_memrlenw(haystack,needle,n_words))
#define __opt_memrlenl(haystack,needle,n_dwords) (__builtin_constant_p(n_dwords) ? __constant_memrlenl(haystack,needle,n_dwords) : __nonconst_memrlenl(haystack,needle,n_dwords))
#define __opt_memrlenq(haystack,needle,n_qwords) (__builtin_constant_p(n_qwords) ? __constant_memrlenq(haystack,needle,n_qwords) : __nonconst_memrlenq(haystack,needle,n_qwords))
#define __opt_rawmemlen(haystack,needle)         (__builtin_constant_p(needle) ? __constant_rawmemlen(haystack,needle) : __nonconst_rawmemlen(haystack,needle))
#define __opt_rawmemlenw(haystack,needle)        (__builtin_constant_p(needle) ? __constant_rawmemlenw(haystack,needle) : __nonconst_rawmemlenw(haystack,needle))
#define __opt_rawmemlenl(haystack,needle)        (__builtin_constant_p(needle) ? __constant_rawmemlenl(haystack,needle) : __nonconst_rawmemlenl(haystack,needle))
#define __opt_rawmemlenq(haystack,needle)        (__builtin_constant_p(needle) ? __constant_rawmemlenq(haystack,needle) : __nonconst_rawmemlenq(haystack,needle))
#define __opt_rawmemrlen(haystack,needle)        (__builtin_constant_p(needle) ? __constant_rawmemrlen(haystack,needle) : __nonconst_rawmemrlen(haystack,needle))
#define __opt_rawmemrlenw(haystack,needle)       (__builtin_constant_p(needle) ? __constant_rawmemrlenw(haystack,needle) : __nonconst_rawmemrlenw(haystack,needle))
#define __opt_rawmemrlenl(haystack,needle)       (__builtin_constant_p(needle) ? __constant_rawmemrlenl(haystack,needle) : __nonconst_rawmemrlenl(haystack,needle))
#define __opt_rawmemrlenq(haystack,needle)       (__builtin_constant_p(needle) ? __constant_rawmemrlenq(haystack,needle) : __nonconst_rawmemrlenq(haystack,needle))

#if __has_builtin(__builtin_strlen) || defined(__GNUC__)
#define __opt_strlen_needs_macro 1
#define __opt_strlen(x) (__builtin_constant_p(x) ? __builtin_strlen(x) : __libc_strlen(x))
#endif
#else /* __OPTIMIZE_CONST__ */
#define __opt_memcpy(dst,src,n_bytes)             __nonconst_memcpy(dst,src,n_bytes)
#define __opt_memcpyw(dst,src,n_words)            __nonconst_memcpyw(dst,src,n_words)
#define __opt_memcpyl(dst,src,n_dwords)           __nonconst_memcpyl(dst,src,n_dwords)
#define __opt_memcpyq(dst,src,n_qwords)           __nonconst_memcpyq(dst,src,n_qwords)
#define __opt_mempcpy(dst,src,n_bytes)            __nonconst_mempcpy(dst,src,n_bytes)
#define __opt_mempcpyw(dst,src,n_words)           __nonconst_mempcpyw(dst,src,n_words)
#define __opt_mempcpyl(dst,src,n_dwords)          __nonconst_mempcpyl(dst,src,n_dwords)
#define __opt_mempcpyq(dst,src,n_qwords)          __nonconst_mempcpyq(dst,src,n_qwords)
#define __opt_memmove(dst,src,n_bytes)            __nonconst_memmove(dst,src,n_bytes)
#define __opt_memmovew(dst,src,n_words)           __nonconst_memmovew(dst,src,n_words)
#define __opt_memmovel(dst,src,n_dwords)          __nonconst_memmovel(dst,src,n_dwords)
#define __opt_memmoveq(dst,src,n_qwords)          __nonconst_memmoveq(dst,src,n_qwords)
#define __opt_memset(dst,byte,n_bytes)            __nonconst_memset(dst,byte,n_bytes)
#define __opt_memsetw(dst,word,n_words)           __nonconst_memsetw(dst,word,n_words)
#define __opt_memsetl(dst,dword,n_dwords)         __nonconst_memsetl(dst,dword,n_dwords)
#define __opt_memsetq(dst,qword,n_qwords)         __nonconst_memsetq(dst,qword,n_qwords)
#define __opt_memcmp(a,b,n_bytes)                 __nonconst_memcmp(a,b,n_bytes)
#define __opt_memcmpw(a,b,n_words)                __nonconst_memcmpw(a,b,n_words)
#define __opt_memcmpl(a,b,n_dwords)               __nonconst_memcmpl(a,b,n_dwords)
#define __opt_memcmpq(a,b,n_qwords)               __nonconst_memcmpq(a,b,n_qwords)
#define __opt_memchr(haystack,needle,n_bytes)     __nonconst_memchr(haystack,needle,n_bytes)
#define __opt_memchrw(haystack,needle,n_words)    __nonconst_memchrw(haystack,needle,n_words)
#define __opt_memchrl(haystack,needle,n_dwords)   __nonconst_memchrl(haystack,needle,n_dwords)
#define __opt_memchrq(haystack,needle,n_qwords)   __nonconst_memchrq(haystack,needle,n_qwords)
#define __opt_memrchr(haystack,needle,n_bytes)    __nonconst_memrchr(haystack,needle,n_bytes)
#define __opt_memrchrw(haystack,needle,n_words)   __nonconst_memrchrw(haystack,needle,n_words)
#define __opt_memrchrl(haystack,needle,n_dwords)  __nonconst_memrchrl(haystack,needle,n_dwords)
#define __opt_memrchrq(haystack,needle,n_qwords)  __nonconst_memrchrq(haystack,needle,n_qwords)
#define __opt_memend(haystack,needle,n_bytes)     __nonconst_memend(haystack,needle,n_bytes)
#define __opt_memendw(haystack,needle,n_words)    __nonconst_memendw(haystack,needle,n_words)
#define __opt_memendl(haystack,needle,n_dwords)   __nonconst_memendl(haystack,needle,n_dwords)
#define __opt_memendq(haystack,needle,n_qwords)   __nonconst_memendq(haystack,needle,n_qwords)
#define __opt_memrend(haystack,needle,n_bytes)    __nonconst_memrend(haystack,needle,n_bytes)
#define __opt_memrendw(haystack,needle,n_words)   __nonconst_memrendw(haystack,needle,n_words)
#define __opt_memrendl(haystack,needle,n_dwords)  __nonconst_memrendl(haystack,needle,n_dwords)
#define __opt_memrendq(haystack,needle,n_qwords)  __nonconst_memrendq(haystack,needle,n_qwords)
#define __opt_rawmemchr(haystack,needle)          __nonconst_rawmemchr(haystack,needle)
#define __opt_rawmemchrw(haystack,needle)         __nonconst_rawmemchrw(haystack,needle)
#define __opt_rawmemchrl(haystack,needle)         __nonconst_rawmemchrl(haystack,needle)
#define __opt_rawmemchrq(haystack,needle)         __nonconst_rawmemchrq(haystack,needle)
#define __opt_rawmemrchr(haystack,needle)         __nonconst_rawmemrchr(haystack,needle)
#define __opt_rawmemrchrw(haystack,needle)        __nonconst_rawmemrchrw(haystack,needle)
#define __opt_rawmemrchrl(haystack,needle)        __nonconst_rawmemrchrl(haystack,needle)
#define __opt_rawmemrchrq(haystack,needle)        __nonconst_rawmemrchrq(haystack,needle)
#define __opt_memlen(haystack,needle,n_bytes)     __nonconst_memlen(haystack,needle,n_bytes)
#define __opt_memlenw(haystack,needle,n_words)    __nonconst_memlenw(haystack,needle,n_words)
#define __opt_memlenl(haystack,needle,n_dwords)   __nonconst_memlenl(haystack,needle,n_dwords)
#define __opt_memlenq(haystack,needle,n_qwords)   __nonconst_memlenq(haystack,needle,n_qwords)
#define __opt_memrlen(haystack,needle,n_bytes)    __nonconst_memrlen(haystack,needle,n_bytes)
#define __opt_memrlenw(haystack,needle,n_words)   __nonconst_memrlenw(haystack,needle,n_words)
#define __opt_memrlenl(haystack,needle,n_dwords)  __nonconst_memrlenl(haystack,needle,n_dwords)
#define __opt_memrlenq(haystack,needle,n_qwords)  __nonconst_memrlenq(haystack,needle,n_qwords)
#define __opt_rawmemlen(haystack,needle)          __nonconst_rawmemlen(haystack,needle)
#define __opt_rawmemlenw(haystack,needle)         __nonconst_rawmemlenw(haystack,needle)
#define __opt_rawmemlenl(haystack,needle)         __nonconst_rawmemlenl(haystack,needle)
#define __opt_rawmemlenq(haystack,needle)         __nonconst_rawmemlenq(haystack,needle)
#define __opt_rawmemrlen(haystack,needle)         __nonconst_rawmemrlen(haystack,needle)
#define __opt_rawmemrlenw(haystack,needle)        __nonconst_rawmemrlenw(haystack,needle)
#define __opt_rawmemrlenl(haystack,needle)        __nonconst_rawmemrlenl(haystack,needle)
#define __opt_rawmemrlenq(haystack,needle)        __nonconst_rawmemrlenq(haystack,needle)
#endif /* !__OPTIMIZE_CONST__ */

#ifndef __opt_strlen
#define __NO_opt_strlen
#define __opt_strlen(x) __libc_strlen(x)
#endif

__SYSDECL_END
#endif /* __CC__ */

#endif /* !_ASM_GENERIC_STRING_H */
