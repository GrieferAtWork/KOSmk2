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
#ifndef _LIBC_CTYPE_H
#define _LIBC_CTYPE_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>
#ifdef __CRT_DOS
#include <bits/dos-ctype.h>
#endif /* __CRT_DOS */
#ifdef __CRT_GLC
#include <bits/endian.h>
#include <hybrid/byteorder.h>
#endif /* __CRT_DOS */

__SYSDECL_BEGIN

#ifdef __CRT_CYG
__LIBC const char *(__LIBCCALL __locale_ctype_ptr)(void);
__LIBC const char *(__LIBCCALL __locale_ctype_ptr_l)(__locale_t __locale);
#define __ctype_lookup(c)         ((__locale_ctype_ptr()+1)[(int)(c)])
#define __ctype_lookup_l(c,l)     ((__locale_ctype_ptr_l(l)+1)[(int)(c)])
#   define __CYG_U 01
#   define __CYG_L 02
#   define __CYG_N 04
#   define __CYG_S 010
#   define __CYG_P 020
#   define __CYG_C 040
#   define __CYG_X 0100
#   define __CYG_B 0200
#ifdef __USE_CYG
#   define _U __CYG_U
#   define _L __CYG_L
#   define _N __CYG_N
#   define _S __CYG_S
#   define _P __CYG_P
#   define _C __CYG_C
#   define _X __CYG_X
#   define _B __CYG_B
#endif /* __USE_CYG */
#endif /* __CRT_CYG */

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
#if defined(__CRT_KOS) && !defined(__GLC_COMPAT__)
__NAMESPACE_INT_BEGIN __LIBC __UINT16_TYPE__ const __chattr[256]; __NAMESPACE_INT_END
#define __glc_isctype(c,type) (__NAMESPACE_INT_SYM __chattr[(__UINT8_TYPE__)(c)]&(__UINT16_TYPE__)type)
#else
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__glc_isctype,(int __c, int __mask),isctype,(__c,__mask))
#endif
#ifdef __USE_GNU
__LIBC __WUNUSED int (__LIBCCALL isctype)(int __c, int __mask);
#if !defined(__cplusplus) && !defined(__OPTIMIZE_SIZE__)
#define isctype(c,mask) __glc_isctype(c,mask)
#endif
#endif /* __USE_GNU */
#endif /* __CRT_GLC */

#ifdef __CRT_DOS
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__dos_isctype,(int __c, int __mask),_isctype,(__c,__mask))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__dos_isctype_l,(int __c, int __mask, __locale_t __locale),_isctype_l,(__c,__mask))
#endif /* __CRT_DOS */

#ifndef __NO_ATTR_INLINE
__LOCAL __BOOL (__LIBCCALL __ascii_iscntrl)(int __ch)  { return (__UINT8_TYPE__)__ch <= 0x1f || (__UINT8_TYPE__)__ch == 0x7f; }
__LOCAL __BOOL (__LIBCCALL __ascii_isblank)(int __ch)  { return (__UINT8_TYPE__)__ch == 0x09 || (__UINT8_TYPE__)__ch == 0x20; }
__LOCAL __BOOL (__LIBCCALL __ascii_isspace)(int __ch)  { return ((__UINT8_TYPE__)__ch >= 0x09 && (__UINT8_TYPE__)__ch <= 0x0d) || (__UINT8_TYPE__)__ch == 0x20; }
__LOCAL __BOOL (__LIBCCALL __ascii_isupper)(int __ch)  { return (__UINT8_TYPE__)__ch >= 0x41 && (__UINT8_TYPE__)__ch <= 0x5a; }
__LOCAL __BOOL (__LIBCCALL __ascii_islower)(int __ch)  { return (__UINT8_TYPE__)__ch >= 0x61 && (__UINT8_TYPE__)__ch <= 0x7a; }
__LOCAL __BOOL (__LIBCCALL __ascii_isalpha)(int __ch)  { return __ascii_isupper(__ch) || __ascii_islower(__ch); }
__LOCAL __BOOL (__LIBCCALL __ascii_isdigit)(int __ch)  { return (__UINT8_TYPE__)__ch >= 0x30 && (__UINT8_TYPE__)__ch <= 0x39; }
__LOCAL __BOOL (__LIBCCALL __ascii_isxdigit)(int __ch) { return __ascii_isdigit(__ch) || ((__UINT8_TYPE__)__ch >= 0x41 && (__UINT8_TYPE__)__ch <= 0x46) || ((__UINT8_TYPE__)__ch >= 0x61 && (__UINT8_TYPE__)__ch <= 0x66); }
__LOCAL __BOOL (__LIBCCALL __ascii_isalnum)(int __ch)  { return __ascii_isupper(__ch) || __ascii_islower(__ch) || __ascii_isdigit(__ch); }
__LOCAL __BOOL (__LIBCCALL __ascii_ispunct)(int __ch)  { return ((__UINT8_TYPE__)__ch >= 0x21 && (__UINT8_TYPE__)__ch <= 0x2f) || ((__UINT8_TYPE__)__ch >= 0x3a && (__UINT8_TYPE__)__ch <= 0x40) || ((__UINT8_TYPE__)__ch >= 0x5b && (__UINT8_TYPE__)__ch <= 0x60) || ((__UINT8_TYPE__)__ch >= 0x7b && (__UINT8_TYPE__)__ch <= 0x7e); }
__LOCAL __BOOL (__LIBCCALL __ascii_isgraph)(int __ch)  { return (__UINT8_TYPE__)__ch >= 0x21 && (__UINT8_TYPE__)__ch <= 0x7e; }
__LOCAL __BOOL (__LIBCCALL __ascii_isprint)(int __ch)  { return (__UINT8_TYPE__)__ch >= 0x20 && (__UINT8_TYPE__)__ch <= 0x7e; }
__LOCAL __BOOL (__LIBCCALL __ascii_tolower)(int __ch)  { return __ascii_isupper(__ch) ? ((__UINT8_TYPE__)__ch+0x20) : __ch; }
__LOCAL __BOOL (__LIBCCALL __ascii_toupper)(int __ch)  { return __ascii_islower(__ch) ? ((__UINT8_TYPE__)__ch-0x20) : __ch; }
#elif !defined(__NO_XBLOCK)
#define __ascii_iscntrl(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ac_ch <= 0x1f || __ac_ch == 0x7f; })
#define __ascii_isblank(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ac_ch == 0x09 || __ac_ch == 0x20; })
#define __ascii_isspace(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN (__ac_ch >= 0x09 && __ac_ch <= 0x0d) || __ac_ch == 0x20; })
#define __ascii_isupper(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ac_ch >= 0x41 && __ac_ch <= 0x5a; })
#define __ascii_islower(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ac_ch >= 0x61 && __ac_ch <= 0x7a; })
#define __ascii_isalpha(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ascii_isupper(__ac_ch) || __ascii_islower(__ac_ch); })
#define __ascii_isdigit(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ac_ch >= 0x30 && __ac_ch <= 0x39; })
#define __ascii_isxdigit(ch) __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ascii_isdigit(__ac_ch) || (__ac_ch >= 0x41 && __ac_ch <= 0x46) || (__ac_ch >= 0x61 && __ac_ch <= 0x66); })
#define __ascii_isalnum(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ascii_isupper(__ac_ch) || __ascii_islower(__ac_ch) || __ascii_isdigit(__ac_ch); })
#define __ascii_ispunct(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN (__ac_ch >= 0x21 && __ac_ch <= 0x2f) || (__ac_ch >= 0x3a && __ac_ch <= 0x40) || (__ac_ch >= 0x5b && __ac_ch <= 0x60) || (__ac_ch >= 0x7b && __ac_ch <= 0x7e); })
#define __ascii_isgraph(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ac_ch >= 0x21 && __ac_ch <= 0x7e; })
#define __ascii_isprint(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ac_ch >= 0x20 && __ac_ch <= 0x7e; })
#define __ascii_tolower(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ascii_isupper(ch) ? (__ac_ch+0x20) : __ac_ch; })
#define __ascii_toupper(ch)  __XBLOCK({ __UINT8_TYPE__ const __ac_ch = (__UINT8_TYPE__)(ch); __XRETURN __ascii_islower(ch) ? (__ac_ch-0x20) : __ac_ch; })
#else
#define __ascii_iscntrl(ch)  ((__UINT8_TYPE__)(ch) <= 0x1f || (__UINT8_TYPE__)(ch) == 0x7f)
#define __ascii_isblank(ch)  ((__UINT8_TYPE__)(ch) == 0x09 || (__UINT8_TYPE__)(ch) == 0x20)
#define __ascii_isspace(ch) (((__UINT8_TYPE__)(ch) >= 0x09 && (__UINT8_TYPE__)(ch) <= 0x0d) || \
                              (__UINT8_TYPE__)(ch) == 0x20)
#define __ascii_isupper(ch)  ((__UINT8_TYPE__)(ch) >= 0x41 && (__UINT8_TYPE__)(ch) <= 0x5a)
#define __ascii_islower(ch)  ((__UINT8_TYPE__)(ch) >= 0x61 && (__UINT8_TYPE__)(ch) <= 0x7a)
#define __ascii_isalpha(ch)  (__ascii_isupper(ch) || __ascii_islower(ch))
#define __ascii_isdigit(ch)  ((__UINT8_TYPE__)(ch) >= 0x30 && (__UINT8_TYPE__)(ch) <= 0x39)
#define __ascii_isxdigit(ch)  (__ascii_isdigit(ch) || \
                             ((__UINT8_TYPE__)(ch) >= 0x41 && (__UINT8_TYPE__)(ch) <= 0x46) || \
                             ((__UINT8_TYPE__)(ch) >= 0x61 && (__UINT8_TYPE__)(ch) <= 0x66))
#define __ascii_isalnum(ch)   (__ascii_isupper(ch) || __ascii_islower(ch) || __ascii_isdigit(ch))
#define __ascii_ispunct(ch) (((__UINT8_TYPE__)(ch) >= 0x21 && (__UINT8_TYPE__)(ch) <= 0x2f) || \
                             ((__UINT8_TYPE__)(ch) >= 0x3a && (__UINT8_TYPE__)(ch) <= 0x40) || \
                             ((__UINT8_TYPE__)(ch) >= 0x5b && (__UINT8_TYPE__)(ch) <= 0x60) || \
                             ((__UINT8_TYPE__)(ch) >= 0x7b && (__UINT8_TYPE__)(ch) <= 0x7e))
#define __ascii_isgraph(ch)  ((__UINT8_TYPE__)(ch) >= 0x21 && (__UINT8_TYPE__)(ch) <= 0x7e)
#define __ascii_isprint(ch)  ((__UINT8_TYPE__)(ch) >= 0x20 && (__UINT8_TYPE__)(ch) <= 0x7e)
#define __ascii_tolower(ch)  (__ascii_isupper(ch) ? ((ch)+0x20) : (ch))
#define __ascii_toupper(ch)  (__ascii_islower(ch) ? ((ch)-0x20) : (ch))
#endif

#define	__inline_isascii(c) (!((__UINT8_TYPE__)(c)&0x80)) /* If C is a 7 bit value. */
#define	__inline_toascii(c)   ((__UINT8_TYPE__)(c)&0x7f)  /* Mask off high bits. */

#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__) && !defined(__CYG_COMPAT__)
#define __inline_isalpha(c)         __glc_isctype(__c,_ISalpha)
#define __inline_isupper(c)         __glc_isctype(__c,_ISupper)
#define __inline_islower(c)         __glc_isctype(__c,_ISlower)
#define __inline_isdigit(c)         __glc_isctype(__c,_ISdigit)
#define __inline_isxdigit(c)        __glc_isctype(__c,_ISxdigit)
#define __inline_isspace(c)         __glc_isctype(__c,_ISspace)
#define __inline_ispunct(c)         __glc_isctype(__c,_ISpunct)
#define __inline_isalnum(c)         __glc_isctype(__c,_ISalnum)
#define __inline_isprint(c)         __glc_isctype(__c,_ISprint)
#define __inline_isgraph(c)         __glc_isctype(__c,_ISgraph)
#define __inline_iscntrl(c)         __glc_isctype(__c,_IScntrl)
#define __inline_isblank(c)         __glc_isctype(__c,_ISblank)
#elif defined(__CRT_CYG) && !defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__)
#define __inline_isalnum(c)        (__ctype_lookup(c)&(__CYG_U|__CYG_L|__CYG_N))
#define __inline_isalpha(c)        (__ctype_lookup(c)&(__CYG_U|__CYG_L))
#define __inline_iscntrl(c)        (__ctype_lookup(c)&__CYG_C)
#define __inline_isdigit(c)        (__ctype_lookup(c)&__CYG_N)
#define __inline_isgraph(c)        (__ctype_lookup(c)&(__CYG_P|__CYG_U|__CYG_L|__CYG_N))
#define __inline_islower(c)       ((__ctype_lookup(c)&(__CYG_U|__CYG_L))==__CYG_L)
#define __inline_isprint(c)        (__ctype_lookup(c)&(__CYG_P|__CYG_U|__CYG_L|__CYG_N|__CYG_B))
#define __inline_ispunct(c)        (__ctype_lookup(c)&__CYG_P)
#define __inline_isspace(c)        (__ctype_lookup(c)&__CYG_S)
#define __inline_isupper(c)       ((__ctype_lookup(c)&(__CYG_U|__CYG_L))==__CYG_U)
#define __inline_isxdigit(c)       (__ctype_lookup(c)&(__CYG_X|__CYG_N))
#define __inline_isalnum_l(c,l)    (__ctype_lookup_l(c,l)&(_U|_L|_N))
#define __inline_isalpha_l(c,l)    (__ctype_lookup_l(c,l)&(_U|_L))
#define __inline_iscntrl_l(c,l)    (__ctype_lookup_l(c,l)&_C)
#define __inline_isdigit_l(c,l)    (__ctype_lookup_l(c,l)&_N)
#define __inline_isgraph_l(c,l)    (__ctype_lookup_l(c,l)&(_P|_U|_L|_N))
#define __inline_islower_l(c,l)   ((__ctype_lookup_l(c,l)&(_U|_L))==_L)
#define __inline_isprint_l(c,l)    (__ctype_lookup_l(c,l)&(_P|_U|_L|_N|_B))
#define __inline_ispunct_l(c,l)    (__ctype_lookup_l(c,l)&_P)
#define __inline_isspace_l(c,l)    (__ctype_lookup_l(c,l)&_S)
#define __inline_isupper_l(c,l)   ((__ctype_lookup_l(c,l)&(_U|_L))==_U)
#define __inline_isxdigit_l(c,l)   (__ctype_lookup_l(c,l)&(_X|_N))
#elif defined(__CRT_DOS) && !defined(__CYG_COMPAT__) && !defined(__GLC_COMPAT__)
#define __inline_isalnum(c)       __dos_isctype((c),__DOS_ALPHA|__DOS_DIGIT)
#define __inline_isalpha(c)       __dos_isctype((c),__DOS_ALPHA)
#define __inline_isupper(c)       __dos_isctype((c),__DOS_UPPER)
#define __inline_islower(c)       __dos_isctype((c),__DOS_LOWER)
#define __inline_isdigit(c)       __dos_isctype((c),__DOS_DIGIT)
#define __inline_isxdigit(c)      __dos_isctype((c),__DOS_HEX)
#define __inline_isspace(c)       __dos_isctype((c),__DOS_SPACE)
#define __inline_ispunct(c)       __dos_isctype((c),__DOS_PUNCT)
#define __inline_isprint(c)       __dos_isctype((c),__DOS_BLANK|__DOS_PUNCT|__DOS_ALPHA|__DOS_DIGIT)
#define __inline_isgraph(c)       __dos_isctype((c),__DOS_PUNCT|__DOS_ALPHA|__DOS_DIGIT)
#define __inline_iscntrl(c)       __dos_isctype((c),__DOS_CONTROL)
#define __inline_isalnum_l(c,l)   __dos_isctype_l((c),__DOS_ALPHA|__DOS_DIGIT,(l))
#define __inline_isalpha_l(c,l)   __dos_isctype_l((c),__DOS_ALPHA,(l))
#define __inline_isupper_l(c,l)   __dos_isctype_l((c),__DOS_UPPER,(l))
#define __inline_islower_l(c,l)   __dos_isctype_l((c),__DOS_LOWER,(l))
#define __inline_isdigit_l(c,l)   __dos_isctype_l((c),__DOS_DIGIT,(l))
#define __inline_isxdigit_l(c,l)  __dos_isctype_l((c),__DOS_HEX,(l))
#define __inline_isspace_l(c,l)   __dos_isctype_l((c),__DOS_SPACE,(l))
#define __inline_ispunct_l(c,l)   __dos_isctype_l((c),__DOS_PUNCT,(l))
#define __inline_isprint_l(c,l)   __dos_isctype_l((c),__DOS_BLANK|__DOS_PUNCT|__DOS_ALPHA|__DOS_DIGIT,(l))
#define __inline_isgraph_l(c,l)   __dos_isctype_l((c),__DOS_PUNCT|__DOS_ALPHA|__DOS_DIGIT,(l))
#define __inline_iscntrl_l(c,l)   __dos_isctype_l((c),__DOS_CONTROL,(l))
#ifndef __NO_ATTR_INLINE
__LOCAL __BOOL (__LIBCCALL __inline_isblank)(int __c) { return __c == '\t' || __dos_isctype(__c,__DOS_BLANK); }
__LOCAL __BOOL (__LIBCCALL __inline_isblank_l)(int __c, __locale_t __l) { return __c == '\t' || __dos_isctype_l(__c,__DOS_BLANK,__l); }
#elif !defined(__NO_XBLOCK)
#define __inline_isblank(c)     __XBLOCK({ int const __in_ch = (c); __XRETURN __in_ch == '\t' || __dos_isctype(__in_ch,__DOS_BLANK); }
#define __inline_isblank_l(c,l) __XBLOCK({ int const __in_ch = (c); __XRETURN __in_ch == '\t' || __dos_isctype_l(__in_ch,__DOS_BLANK,(l)); }
#else
#define __inline_isblank(c)     ((c) == '\t' || __dos_isctype((c),__DOS_BLANK))
#define __inline_isblank_l(c,l) ((c) == '\t' || __dos_isctype_l((c),__DOS_BLANK,(l)))
#endif
#else /* ... */
#define __inline_isalpha(c)       __ascii_isalpha(c)
#define __inline_isupper(c)       __ascii_isupper(c)
#define __inline_islower(c)       __ascii_islower(c)
#define __inline_isdigit(c)       __ascii_isdigit(c)
#define __inline_isxdigit(c)      __ascii_isxdigit(c)
#define __inline_isspace(c)       __ascii_isspace(c)
#define __inline_ispunct(c)       __ascii_ispunct(c)
#define __inline_isalnum(c)       __ascii_isalnum(c)
#define __inline_isprint(c)       __ascii_isprint(c)
#define __inline_isgraph(c)       __ascii_isgraph(c)
#define __inline_iscntrl(c)       __ascii_iscntrl(c)
#endif /* !... */

#ifndef __inline_isblank
#define __inline_isblank(c)        __ascii_isblank(c)
#endif
#ifndef __inline_isalpha_l
#define __inline_isalnum_l(c,l)   ((void)(l),__inline_isalnum(c))
#define __inline_isalpha_l(c,l)   ((void)(l),__inline_isalpha(c))
#define __inline_iscntrl_l(c,l)   ((void)(l),__inline_iscntrl(c))
#define __inline_isdigit_l(c,l)   ((void)(l),__inline_isdigit(c))
#define __inline_isgraph_l(c,l)   ((void)(l),__inline_isgraph(c))
#define __inline_islower_l(c,l)   ((void)(l),__inline_islower(c))
#define __inline_isprint_l(c,l)   ((void)(l),__inline_isprint(c))
#define __inline_ispunct_l(c,l)   ((void)(l),__inline_ispunct(c))
#define __inline_isspace_l(c,l)   ((void)(l),__inline_isspace(c))
#define __inline_isupper_l(c,l)   ((void)(l),__inline_isupper(c))
#define __inline_isxdigit_l(c,l)  ((void)(l),__inline_isxdigit(c))
#endif
#ifndef __inline_isblank_l
#define __inline_isblank_l(c,l)   ((void)(l),__inline_isblank(c))
#endif
#define __inline_iscsymf(c)       (__inline_isalpha(c) || ((c) == '_'))
#define __inline_iscsym(c)        (__inline_isalnum(c) || ((c) == '_'))
#define __inline_iscsymf_l(c,l)   (__inline_isalpha_l(c,l) || ((c) == '_'))
#define __inline_iscsym_l(c,l)    (__inline_isalnum_l(c,l) || ((c) == '_'))


__SYSDECL_END

#endif /* !_LIBC_CTYPE_H */
