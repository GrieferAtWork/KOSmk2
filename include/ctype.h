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
#ifdef __USE_DOS
#include <xlocale.h>
#endif /* __USE_DOS */

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
__FORCELOCAL bool __NOTHROW(__isctype(char __c, __UINT16_TYPE__ __type));
__FORCELOCAL bool __isctype(char __c, __UINT16_TYPE__ __type) { return __chattr[(__UINT8_TYPE__)__c] & __type; }
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(__chattr)
__NAMESPACE_STD_USING(__isctype)
#else
__LIBC __UINT16_TYPE__ const __chattr[256];
#define __isctype(c,type) (__chattr[(__UINT8_TYPE__)(c)]&(__UINT16_TYPE__)type)
#endif

__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isalpha)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isupper)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL islower)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isdigit)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isxdigit)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isspace)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL ispunct)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isalnum)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isprint)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isgraph)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL iscntrl)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL toupper)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL tolower)(int __c));
#ifdef __USE_ISOC99
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isblank)(int __c));
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
__LIBC int __NOTHROW((__LIBCCALL isascii)(int __c)) __ASMNAME("__isascii");
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
__LIBC __WUNUSED int (__LIBCCALL isctype)(int __c, int __mask) __PE_ASMNAME("_isctype");
#ifndef __CXX_SYSTEM_HEADER
#define isctype(c,mask) __isctype((c),(mask))
#endif /* !__CXX_SYSTEM_HEADER */
#endif /* __USE_GNU */

#define __tolower(c) ((c)+0x20)
#define __toupper(c) ((c)-0x20)

#ifdef __USE_KOS
/* Same as the regular tolower()/toupper() functions,
 * but only returns the correct value when the character
 * was of a reverse casing beforehand. */
#ifndef ___tolower_defined
#define ___tolower_defined 1
#ifndef __KERNEL__
__LIBC int __NOTHROW((__LIBCCALL _tolower)(int __c));
__LIBC int __NOTHROW((__LIBCCALL _toupper)(int __c));
#endif /* !__KERNEL__ */
#define _tolower(c) __tolower(c)
#define _toupper(c) __toupper(c)
#endif /* !___tolower_defined */
#endif /* __USE_KOS */

/* Dos character type bits. */
#ifndef __DOS_UPPER
#define __DOS_UPPER    0x0001
#define __DOS_LOWER    0x0002
#define __DOS_DIGIT    0x0004
#define __DOS_SPACE    0x0008
#define __DOS_PUNCT    0x0010
#define __DOS_CONTROL  0x0020
#define __DOS_BLANK    0x0040
#define __DOS_HEX      0x0080
#define __DOS_LEADBYTE 0x8000
#define __DOS_ALPHA    0x0103

#ifdef __USE_DOS
#   define _UPPER    __DOS_UPPER
#   define _LOWER    __DOS_LOWER
#   define _DIGIT    __DOS_DIGIT
#   define _SPACE    __DOS_SPACE
#   define _PUNCT    __DOS_PUNCT
#   define _CONTROL  __DOS_CONTROL
#   define _BLANK    __DOS_BLANK
#   define _HEX      __DOS_HEX
#   define _LEADBYTE __DOS_LEADBYTE
#   define _ALPHA    __DOS_ALPHA
#endif /* __USE_DOS */
#endif /* !__DOS_UPPER */

#ifdef __USE_DOS
#ifndef _CTYPE_DEFINED
#define _CTYPE_DEFINED 1
__LIBC int (__LIBCCALL _isctype)(int __c, int __mask) __KOS_ASMNAME("isctype");
__LIBC int (__LIBCCALL _isctype_l)(int __c, int __mask, __locale_t __locale);
__LIBC int (__LIBCCALL _isalpha_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _isupper_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _islower_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _isdigit_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _isxdigit_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _isspace_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _ispunct_l)(int __c, __locale_t __locale) __ASMNAME("ispunct");
__LIBC int (__LIBCCALL _isblank_l)(int __c, __locale_t __locale) __ASMNAME("isblank");
__LIBC int (__LIBCCALL _isalnum_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _isprint_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _isgraph_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _iscntrl_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _tolower_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL _toupper_l)(int __c, __locale_t __locale);
__LIBC int (__LIBCCALL __isascii)(int __c);
__LIBC int (__LIBCCALL __toascii)(int __c);
__LIBC int (__LIBCCALL __iscsymf)(int __c);
__LIBC int (__LIBCCALL __iscsym)(int __c);

#ifndef ___tolower_defined
#define ___tolower_defined 1
#ifndef __KERNEL__
__LIBC int (__LIBCCALL _tolower)(int __c);
__LIBC int (__LIBCCALL _toupper)(int __c);
#endif /* !__KERNEL__ */
#define _tolower(c) __tolower(c)
#define _toupper(c) __toupper(c)
#endif /* !___tolower_defined */
#endif  /* _CTYPE_DEFINED */

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */

#ifndef __wctype_t_defined
#define __wctype_t_defined 1
__NAMESPACE_STD_BEGIN
typedef unsigned long int wctype_t;
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(wctype_t)
#endif /* !__wctype_t_defined */

#ifndef _WCTYPE_DEFINED
#define _WCTYPE_DEFINED 1
#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC int __NOTHROW((__LIBCCALL iswalnum)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswalpha)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswcntrl)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswdigit)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswgraph)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswlower)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswprint)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswpunct)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswspace)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswupper)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL iswxdigit)(wint_t __wc));
__LIBC wctype_t __NOTHROW((__LIBCCALL wctype)(char const *__prop));
__LIBC int __NOTHROW((__LIBCCALL iswctype)(wint_t __wc, wctype_t __desc)) __DOS_FUNC(iswctype);
#ifdef __USE_ISOC99
__LIBC int __NOTHROW((__LIBCCALL iswblank)(wint_t __wc));
#endif /* __USE_ISOC99 */
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(iswalnum)
__NAMESPACE_STD_USING(iswalpha)
__NAMESPACE_STD_USING(iswcntrl)
__NAMESPACE_STD_USING(iswdigit)
__NAMESPACE_STD_USING(iswgraph)
__NAMESPACE_STD_USING(iswlower)
__NAMESPACE_STD_USING(iswprint)
__NAMESPACE_STD_USING(iswpunct)
__NAMESPACE_STD_USING(iswspace)
__NAMESPACE_STD_USING(iswupper)
__NAMESPACE_STD_USING(iswxdigit)
__NAMESPACE_STD_USING(wctype)
__NAMESPACE_STD_USING(iswctype)
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(iswblank)
#endif /* __USE_ISOC99 */
__LIBC int __NOTHROW((__LIBCCALL iswascii)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL _iswalpha_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswalpha_l");
__LIBC int __NOTHROW((__LIBCCALL _iswupper_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswupper_l");
__LIBC int __NOTHROW((__LIBCCALL _iswlower_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswlower_l");
__LIBC int __NOTHROW((__LIBCCALL _iswdigit_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswdigit_l");
__LIBC int __NOTHROW((__LIBCCALL _iswxdigit_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswxdigit_l");
__LIBC int __NOTHROW((__LIBCCALL _iswspace_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswspace_l");
__LIBC int __NOTHROW((__LIBCCALL _iswpunct_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswpunct_l");
__LIBC int __NOTHROW((__LIBCCALL _iswblank_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswblank_l");
__LIBC int __NOTHROW((__LIBCCALL _iswalnum_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswalnum_l");
__LIBC int __NOTHROW((__LIBCCALL _iswprint_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswprint_l");
__LIBC int __NOTHROW((__LIBCCALL _iswgraph_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswgraph_l");
__LIBC int __NOTHROW((__LIBCCALL _iswcntrl_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("iswcntrl_l");
__LIBC int __NOTHROW((__LIBCCALL isleadbyte)(int __wc));
__LIBC int __NOTHROW((__LIBCCALL _isleadbyte_l)(int __wc, __locale_t __locale));
__LIBC wint_t __NOTHROW((__LIBCCALL _towupper_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("towupper_l");
__LIBC wint_t __NOTHROW((__LIBCCALL _towlower_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("towlower_l");
__LIBC int __NOTHROW((__LIBCCALL _iswctype_l)(wint_t __wc, wctype_t __type, __locale_t __locale)) __KOS_ASMNAME("iswctype_l");
__LIBC int __NOTHROW((__LIBCCALL is_wctype)(wint_t __wc, wctype_t __desc)) __ASMNAME("iswctype");
__LIBC int __NOTHROW((__LIBCCALL __iswcsymf)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL __iswcsym)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL _iswcsymf_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL _iswcsym_l)(wint_t __wc, __locale_t __locale));
#endif /* !__KERNEL__ */

#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC wint_t __NOTHROW((__LIBCCALL towlower)(wint_t __wc));
__LIBC wint_t __NOTHROW((__LIBCCALL towupper)(wint_t __wc));
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(towlower)
__NAMESPACE_STD_USING(towupper)
__LIBC int __NOTHROW((__LIBCCALL isleadbyte)(int __wc));
__LIBC int __NOTHROW((__LIBCCALL _isleadbyte_l)(int __wc, __locale_t __locale));
__LIBC wint_t __NOTHROW((__LIBCCALL _towupper_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("towupper_l");
__LIBC wint_t __NOTHROW((__LIBCCALL _towlower_l)(wint_t __wc, __locale_t __locale)) __KOS_ASMNAME("towlower_l");
__LIBC int __NOTHROW((__LIBCCALL _iswctype_l)(wint_t __wc, wctype_t __type, __locale_t __locale)) __KOS_ASMNAME("iswctype_l");
__LIBC int __NOTHROW((__LIBCCALL is_wctype)(wint_t __wc, wctype_t __desc)) __ASMNAME("iswctype");
__LIBC int __NOTHROW((__LIBCCALL __iswcsymf)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL __iswcsym)(wint_t __wc));
__LIBC int __NOTHROW((__LIBCCALL _iswcsymf_l)(wint_t __wc, __locale_t __locale));
__LIBC int __NOTHROW((__LIBCCALL _iswcsym_l)(wint_t __wc, __locale_t __locale));
#endif /* !__KERNEL__ */
#endif /* !_WCTYPE_DEFINED */

#ifndef _WCTYPE_INLINE_DEFINED
#define _WCTYPE_INLINE_DEFINED 1
#ifndef ____kos_iswctype_defined
#define ____kos_iswctype_defined 1
__NAMESPACE_STD_BEGIN
__LIBC int __NOTHROW((__LIBCCALL __kos_iswctype)(wint_t __wc, wctype_t __desc)) __ASMNAME("iswctype");
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(__kos_iswctype)
#endif /* !____kos_iswctype_defined */
#define iswalnum(wc)  __kos_iswctype((wc),_ISwalnum)
#define iswalpha(wc)  __kos_iswctype((wc),_ISwalpha)
#define iswcntrl(wc)  __kos_iswctype((wc),_ISwcntrl)
#define iswdigit(wc)  __kos_iswctype((wc),_ISwdigit)
#define iswlower(wc)  __kos_iswctype((wc),_ISwlower)
#define iswgraph(wc)  __kos_iswctype((wc),_ISwgraph)
#define iswprint(wc)  __kos_iswctype((wc),_ISwprint)
#define iswpunct(wc)  __kos_iswctype((wc),_ISwpunct)
#define iswspace(wc)  __kos_iswctype((wc),_ISwspace)
#define iswupper(wc)  __kos_iswctype((wc),_ISwupper)
#define iswxdigit(wc) __kos_iswctype((wc),_ISwxdigit)
#ifdef __USE_ISOC99
#define iswblank(wc)  __kos_iswctype((wc),_ISwblank)
#endif /* __USE_ISOC99 */
#if __SIZEOF_WCHAR_T__ == 4
#   define iswascii(wc) ((__UINT32_TYPE__)(wc) <= 0x7f)
#else /* #elif __SIZEOF_WCHAR_T__ == 2 */
#   define iswascii(wc) ((__UINT16_TYPE__)(wc) <= 0x7f)
#endif
/* Dos doesn't actually evaluate the second parameter, but we do as an extension. */
#ifdef __USE_KOS
#define _iswalnum_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwalnum))
#define _iswalpha_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwalpha))
#define _iswcntrl_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwcntrl))
#define _iswdigit_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwdigit))
#define _iswlower_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwlower))
#define _iswgraph_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwgraph))
#define _iswprint_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwprint))
#define _iswpunct_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwpunct))
#define _iswspace_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwspace))
#define _iswupper_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwupper))
#define _iswxdigit_l(wc,lc)   __kos_iswctype((wc),(lc,_ISwxdigit))
#define _iswblank_l(wc,lc)    __kos_iswctype((wc),(lc,_ISwblank))
#else /* __USE_KOS */
#define _iswalnum_l(wc,lc)    __kos_iswctype((wc),_ISwalnum)
#define _iswalpha_l(wc,lc)    __kos_iswctype((wc),_ISwalpha)
#define _iswcntrl_l(wc,lc)    __kos_iswctype((wc),_ISwcntrl)
#define _iswdigit_l(wc,lc)    __kos_iswctype((wc),_ISwdigit)
#define _iswlower_l(wc,lc)    __kos_iswctype((wc),_ISwlower)
#define _iswgraph_l(wc,lc)    __kos_iswctype((wc),_ISwgraph)
#define _iswprint_l(wc,lc)    __kos_iswctype((wc),_ISwprint)
#define _iswpunct_l(wc,lc)    __kos_iswctype((wc),_ISwpunct)
#define _iswspace_l(wc,lc)    __kos_iswctype((wc),_ISwspace)
#define _iswupper_l(wc,lc)    __kos_iswctype((wc),_ISwupper)
#define _iswxdigit_l(wc,lc)   __kos_iswctype((wc),_ISwxdigit)
#define _iswblank_l(wc,lc)    __kos_iswctype((wc),_ISwblank)
#endif /* !__USE_KOS */
#endif /* !_WCTYPE_INLINE_DEFINED */

#define __iscsymf(c)       (__NAMESPACE_STD_SYM isalpha(c) || ((c) == '_'))
#define __iscsym(c)        (__NAMESPACE_STD_SYM isalnum(c) || ((c) == '_'))
#define __iswcsymf(c)      (__NAMESPACE_STD_SYM iswalpha(c) || ((c) == '_'))
#define __iswcsym(c)       (__NAMESPACE_STD_SYM iswalnum(c) || ((c) == '_'))
#define _iscsymf_l(c,lc)   (_isalpha_l(c,lc) || ((c) == '_'))
#define _iscsym_l(c,lc)    (_isalnum_l(c,lc) || ((c) == '_'))
#define _iswcsymf_l(c,lc)  (__NAMESPACE_STD_SYM iswalpha(c) || ((c) == '_'))
#define _iswcsym_l(c,lc)   (__NAMESPACE_STD_SYM iswalnum(c) || ((c) == '_'))

__LIBC int __NOTHROW((__LIBCCALL isascii)(int __c)) __ASMNAME("__isascii");
__LIBC int __NOTHROW((__LIBCCALL toascii)(int __c)) __ASMNAME("__toascii");
__LIBC int __NOTHROW((__LIBCCALL iscsymf)(int __c)) __ASMNAME("__iscsymf");
__LIBC int __NOTHROW((__LIBCCALL iscsym)(int __c)) __ASMNAME("__iscsym");

#endif /* __USE_DOS */

__DECL_END

#endif /* !_CTYPE_H */
