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
#ifndef _ASM_STRING_H
#define _ASM_STRING_H 1

#include <__stdinc.h>
#include <libc/string.h>

__SYSDECL_BEGIN

#define __asm_memcpy(dst,src,n_bytes)           __libc_memcpy(dst,src,n_bytes)
#define __asm_memcpyw(dst,src,n_words)          __libc_memcpyw(dst,src,n_words)
#define __asm_memcpyl(dst,src,n_dwords)         __libc_memcpyl(dst,src,n_dwords)
#define __asm_mempcpy(dst,src,n_bytes)          __libc_mempcpy(dst,src,n_bytes)
#define __asm_mempcpyw(dst,src,n_words)         __libc_mempcpyw(dst,src,n_words)
#define __asm_mempcpyl(dst,src,n_dwords)        __libc_mempcpyl(dst,src,n_dwords)
#define __asm_memmove(dst,src,n_bytes)          __libc_memmove(dst,src,n_bytes)
#define __asm_memmovew(dst,src,n_words)         __libc_memmovew(dst,src,n_words)
#define __asm_memmovel(dst,src,n_dwords)        __libc_memmovel(dst,src,n_dwords)
#define __asm_memset(dst,byte,n_bytes)          __libc_memset(dst,byte,n_bytes)
#define __asm_memsetw(dst,word,n_words)         __libc_memsetw(dst,word,n_words)
#define __asm_memsetl(dst,dword,n_dwords)       __libc_memsetl(dst,dword,n_dwords)
#define __asm_memcmp(a,b,n_bytes)               __libc_memcmp(a,b,n_bytes)
#define __asm_memcmpw(a,b,n_words)              __libc_memcmpw(a,b,n_words)
#define __asm_memcmpl(a,b,n_dwords)             __libc_memcmpl(a,b,n_dwords)
#define __asm_memchr(haystack,needle,n_bytes)   __libc_memchr(haystack,needle,n_bytes)
#define __asm_memchrw(haystack,needle,n_words)  __libc_memchrw(haystack,needle,n_words)
#define __asm_memchrl(haystack,needle,n_dwords) __libc_memchrl(haystack,needle,n_dwords)

__SYSDECL_END

#endif /* !_ASM_STRING_H */
