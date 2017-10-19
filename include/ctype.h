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
#include <hybrid/typecore.h>
#include <hybrid/byteorder.h>
#include <bits/endian.h>
#include <bits/dos-ctype.h>
#ifdef __USE_DOS
#include <bits/wctype.h>
#include <xlocale.h>
#endif /* __USE_DOS */

__SYSDECL_BEGIN

#ifdef __CRT_GLC
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
#endif /* __CRT_GLC */

#define	__isascii(c) (!((c)&0x80)) /* If C is a 7 bit value. */
#define	__toascii(c)   ((c)&0x7f)  /* Mask off high bits. */

#ifdef __cplusplus
__NAMESPACE_STD_BEGIN
__LIBC __UINT16_TYPE__ const __chattr[256];
__LOCAL bool __NOTHROW((__LIBCCALL __isctype)(char __c, __UINT16_TYPE__ __type)) { return __chattr[(__UINT8_TYPE__)__c] & __type; }
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(__chattr)
__NAMESPACE_STD_USING(__isctype)
#else /* __cplusplus */
__LIBC __UINT16_TYPE__ const __chattr[256];
#define __isctype(c,type) (__chattr[(__UINT8_TYPE__)(c)]&(__UINT16_TYPE__)type)
#endif /* !__cplusplus */

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
#if defined(__CRT_DOS) && !defined(__NO_ASMNAME)
__LIBC int __NOTHROW((__LIBCCALL isascii)(int __c)) __ASMNAME("__isascii");
#else /* __CRT_DOS */
__LOCAL int __NOTHROW((__LIBCCALL isascii)(int __c)) { return (__UINT8_TYPE__)__c <= 0x7f; }
#endif /* !__CRT_DOS */
#endif /* __USE_KOS || __USE_DOS */

#ifdef __CRT_GLC
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
#ifndef __cplusplus /* libstdc++ doesn't undef this one properly... */
#define isblank(c)  __isctype((c),_ISblank)
#endif
#endif /* __USE_ISOC99 */
#endif /* __CRT_GLC */

#if defined(__USE_KOS) || defined(__USE_DOS)
#define isascii(c)  ((__UINT8_TYPE__)(c) <= 0x7f)
#endif /* __USE_KOS || __USE_DOS */

#ifdef __USE_GNU
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,isctype,(int __c, int __mask),_isctype,(__c,__mask))
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
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL _tolower)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL _toupper)(int __c));
#endif /* !__KERNEL__ */
#define _tolower(c) __tolower(c)
#define _toupper(c) __toupper(c)
#endif /* !___tolower_defined */
#endif /* __USE_KOS */

#ifdef __USE_DOS
#ifndef _CTYPE_DEFINED
#define _CTYPE_DEFINED 1
__REDIRECT_IFKOS_NOTHROW(__LIBC,,int,__LIBCCALL,_isctype,(int __c, int __mask),isctype,(__c,__mask))
#ifdef __CRT_DOS
__LIBC __WUNUSED int (__LIBCCALL _isctype_l)(int __c, int __mask, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isalpha_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isupper_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _islower_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isdigit_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isxdigit_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isspace_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _ispunct_l)(int __c, __locale_t __locale) /*__ASMNAME("ispunct")*/;
__LIBC __WUNUSED int (__LIBCCALL _isblank_l)(int __c, __locale_t __locale) /*__ASMNAME("isblank")*/;
__LIBC __WUNUSED int (__LIBCCALL _isalnum_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isprint_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isgraph_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _iscntrl_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _tolower_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _toupper_l)(int __c, __locale_t __locale);
#elif defined(__LIBCCALL_CALLER_CLEANUP)
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_isctype_l,(int __c, int __mask, __locale_t __locale),isctype,(__c,__mask,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_isalpha_l,(int __c, __locale_t __locale),isalpha,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_isupper_l,(int __c, __locale_t __locale),isupper,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_islower_l,(int __c, __locale_t __locale),islower,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_isdigit_l,(int __c, __locale_t __locale),isdigit,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_isxdigit_l,(int __c, __locale_t __locale),isxdigit,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_isspace_l,(int __c, __locale_t __locale),isspace,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_ispunct_l,(int __c, __locale_t __locale),ispunct,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_isalnum_l,(int __c, __locale_t __locale),isalnum,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_isprint_l,(int __c, __locale_t __locale),isprint,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_isgraph_l,(int __c, __locale_t __locale),isgraph,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_iscntrl_l,(int __c, __locale_t __locale),iscntrl,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_tolower_l,(int __c, __locale_t __locale),tolower,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_toupper_l,(int __c, __locale_t __locale),toupper,(__c,__locale))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,_isblank_l,(int __c, __locale_t __locale),isblank,(__c,__locale))
#else /* __CRT_DOS */
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isctype_l)(int __c, int __mask, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM __isctype(__c,__mask); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isalpha_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM isalpha(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isupper_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM isupper(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _islower_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM islower(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isdigit_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM isdigit(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isxdigit_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM isxdigit(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isspace_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM isspace(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _ispunct_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM ispunct(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isalnum_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM isalnum(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isprint_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM isprint(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isgraph_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM isgraph(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _iscntrl_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM iscntrl(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _tolower_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM tolower(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _toupper_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM toupper(__c); }
#ifdef __USE_ISOC99
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isblank_l)(int __c, __locale_t __UNUSED(__locale))) { return __NAMESPACE_STD_SYM isblank(__c); }
#else /* __USE_ISOC99 */
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_isblank,(int __c),isblank,(__c))
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isblank_l)(int __c, __locale_t __UNUSED(__locale))) { return __libc_isblank(__c); }
#endif /* !__USE_ISOC99 */
#endif /* !__CRT_DOS */

#ifdef __CRT_DOS
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL __isascii)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL __toascii)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL __iscsymf)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL __iscsym)(int __c));
#else /* __CRT_DOS */
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL __isascii)(int __c)) { return __isascii(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL __toascii)(int __c)) { return __toascii(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL __iscsymf)(int __c)) { return __NAMESPACE_STD_SYM isalpha(__c) || __c == '_'; }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL __iscsym)(int __c)) { return __NAMESPACE_STD_SYM isalnum(__c) || __c == '_'; }
#endif /* !__CRT_DOS */

#ifndef ___tolower_defined
#define ___tolower_defined 1
#ifndef __KERNEL__
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL _tolower)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL _toupper)(int __c));
#endif /* !__KERNEL__ */
#define _tolower(c) __tolower(c)
#define _toupper(c) __toupper(c)
#endif /* !___tolower_defined */
#endif  /* _CTYPE_DEFINED */

#define __iscsymf(c)       (__NAMESPACE_STD_SYM isalpha(c) || ((c) == '_'))
#define __iscsym(c)        (__NAMESPACE_STD_SYM isalnum(c) || ((c) == '_'))
#define __iswcsymf(c)      (__NAMESPACE_STD_SYM iswalpha(c) || ((c) == '_'))
#define __iswcsym(c)       (__NAMESPACE_STD_SYM iswalnum(c) || ((c) == '_'))
#define _iscsymf_l(c,lc)   (_isalpha_l(c,lc) || ((c) == '_'))
#define _iscsym_l(c,lc)    (_isalnum_l(c,lc) || ((c) == '_'))
#define _iswcsymf_l(c,lc)  (_iswalpha_l(c,lc) || ((c) == '_'))
#define _iswcsym_l(c,lc)   (_iswalnum_l(c,lc) || ((c) == '_'))

#ifdef __CRT_DOS
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,isascii,(int __c),__isascii,(__c))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,toascii,(int __c),__toascii,(__c))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iscsymf,(int __c),__iscsymf,(__c))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iscsym,(int __c),__iscsym,(__c))
#else /* __CRT_DOS */
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL isascii)(int __c)) { return __isascii(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL toascii)(int __c)) { return __toascii(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL iscsymf)(int __c)) { return __iscsymf(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL iscsym)(int __c)) { return __iscsym(__c); }
#endif /* !__CRT_DOS */
#endif /* __USE_DOS */

__SYSDECL_END

#endif /* !_CTYPE_H */
