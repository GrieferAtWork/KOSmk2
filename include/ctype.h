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
#ifndef _CTYPE_H
#define _CTYPE_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/byteorder.h>
#include <hybrid/typecore.h>

__DECL_BEGIN

#if __BYTE_ORDER == __BIG_ENDIAN
#   define _ISbit(bit) (1 << (bit))
#else
#   define _ISbit(bit) ((bit) < 8 ? ((1 << (bit)) << 8) : ((1 << (bit)) >> 8))
#endif


#define _ISupper  _ISbit(0)  /*< UPPERCASE. */
#define _ISlower  _ISbit(1)  /*< lowercase. */
#define _ISalpha  _ISbit(2)  /*< Alphabetic. */
#define _ISdigit  _ISbit(3)  /*< Numeric. */
#define _ISxdigit _ISbit(4)  /*< Hexadecimal numeric. */
#define _ISspace  _ISbit(5)  /*< Whitespace. */
#define _ISprint  _ISbit(6)  /*< Printing. */
#define _ISgraph  _ISbit(7)  /*< Graphical. */
#define _ISblank  _ISbit(8)  /*< Blank (usually SPC and TAB). */
#define _IScntrl  _ISbit(9)  /*< Control character. */
#define _ISpunct  _ISbit(10) /*< Punctuation. */
#define _ISalnum  _ISbit(11) /*< Alphanumeric. */

#define	__isascii(c) (!((c)&0x80)) /* If C is a 7 bit value. */
#define	__toascii(c)   ((c)&0x7f)  /* Mask off high bits. */

#ifdef __cplusplus
__NAMESPACE_STD_BEGIN
__LIBC __UINT16_TYPE__ const __chattr[256];
__FORCELOCAL bool __isctype(char __c, __UINT16_TYPE__ __type) {
 return __chattr[(__UINT8_TYPE__)__c] & __type;
}
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(__chattr)
__NAMESPACE_STD_USING(__isctype)
#else
__LIBC __UINT16_TYPE__ const __chattr[256];
#define __isctype(c,type) (__chattr[(__UINT8_TYPE__)(c)]&(__UINT16_TYPE__)type)
#endif

__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED int (__LIBCCALL isalpha)(int __c);
__LIBC __WUNUSED int (__LIBCCALL isupper)(int __c);
__LIBC __WUNUSED int (__LIBCCALL islower)(int __c);
__LIBC __WUNUSED int (__LIBCCALL isdigit)(int __c);
__LIBC __WUNUSED int (__LIBCCALL isxdigit)(int __c);
__LIBC __WUNUSED int (__LIBCCALL isspace)(int __c);
__LIBC __WUNUSED int (__LIBCCALL ispunct)(int __c);
__LIBC __WUNUSED int (__LIBCCALL isalnum)(int __c);
__LIBC __WUNUSED int (__LIBCCALL isprint)(int __c);
__LIBC __WUNUSED int (__LIBCCALL isgraph)(int __c);
__LIBC __WUNUSED int (__LIBCCALL iscntrl)(int __c);
__LIBC __WUNUSED int (__LIBCCALL toupper)(int __c);
__LIBC __WUNUSED int (__LIBCCALL tolower)(int __c);
#ifdef __USE_ISOC99
__LIBC __WUNUSED int (__LIBCCALL isblank)(int __c);
#endif /* __USE_ISOC99 */
__NAMESPACE_STD_END

__NAMESPACE_STD_USING(isalpha)
__NAMESPACE_STD_USING(isupper)
__NAMESPACE_STD_USING(islower)
__NAMESPACE_STD_USING(isdigit)
__NAMESPACE_STD_USING(isxdigit)
__NAMESPACE_STD_USING(isspace)
__NAMESPACE_STD_USING(ispunct)
__NAMESPACE_STD_USING(isalnum)
__NAMESPACE_STD_USING(isprint)
__NAMESPACE_STD_USING(isgraph)
__NAMESPACE_STD_USING(iscntrl)
__NAMESPACE_STD_USING(toupper)
__NAMESPACE_STD_USING(tolower)
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(isblank)
#endif /* __USE_ISOC99 */
#if defined(__USE_KOS) || defined(__USE_DOS)
__LIBC int __NOTHROW((__LIBCCALL isascii)(int __c));
#endif /* __USE_KOS || __USE_DOS */

#define isalnum(c)  __isctype((c),_ISalnum)
#define isalpha(c)  __isctype((c),_ISalpha)
#define iscntrl(c)  __isctype((c),_IScntrl)
#define isdigit(c)  __isctype((c),_ISdigit)
#define islower(c)  __isctype((c),_ISlower)
#define isgraph(c)  __isctype((c),_ISgraph)
#define isprint(c)  __isctype((c),_ISprint)
#define ispunct(c)  __isctype((c),_ISpunct)
#define isspace(c)  __isctype((c),_ISspace)
#define isupper(c)  __isctype((c),_ISupper)
#define isxdigit(c) __isctype((c),_ISxdigit)
#ifdef __USE_ISOC99
#define isblank(c)  __isctype((c),_ISblank)
#endif /* __USE_ISOC99 */
#if defined(__USE_KOS) || defined(__USE_DOS)
#define isascii(c)  ((__UINT8_TYPE__)(c) <= 0x7f)
#endif /* __USE_KOS || __USE_DOS */

#ifdef __USE_GNU
__LIBC __WUNUSED int (__LIBCCALL isctype)(int __c, int __mask);
#ifndef __CXX_SYSTEM_HEADER
#define isctype(c,mask) __isctype((c),(mask))
#endif /* !__CXX_SYSTEM_HEADER */
#endif /* __USE_GNU */


__DECL_END

#endif /* !_CTYPE_H */
