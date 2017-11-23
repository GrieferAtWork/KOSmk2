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
#include <libc/ctype.h>
#ifdef __USE_DOS
#include <bits/wctype.h>
#endif /* __USE_DOS */

__SYSDECL_BEGIN

__NAMESPACE_STD_BEGIN
#if defined(__OPTIMIZE__) && !defined(__OPTIMIZE_SIZE__)
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL isalpha)(int __c))  { return __inline_isalpha(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL isupper)(int __c))  { return __inline_isupper(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL islower)(int __c))  { return __inline_islower(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL isdigit)(int __c))  { return __inline_isdigit(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL isxdigit)(int __c)) { return __inline_isxdigit(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL isspace)(int __c))  { return __inline_isspace(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL ispunct)(int __c))  { return __inline_ispunct(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL isalnum)(int __c))  { return __inline_isalnum(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL isprint)(int __c))  { return __inline_isprint(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL isgraph)(int __c))  { return __inline_isgraph(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL iscntrl)(int __c))  { return __inline_iscntrl(__c); }
#ifdef __USE_ISOC99
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isblank)(int __c)) { return __inline_isblank(__c); }
#endif /* __USE_ISOC99 */
#ifndef __cplusplus
#define isalnum(c)  __inline_isalnum(c)
#define isalpha(c)  __inline_isalpha(c)
#define iscntrl(c)  __inline_iscntrl(c)
#define isdigit(c)  __inline_isdigit(c)
#define islower(c)  __inline_islower(c)
#define isgraph(c)  __inline_isgraph(c)
#define isprint(c)  __inline_isprint(c)
#define ispunct(c)  __inline_ispunct(c)
#define isspace(c)  __inline_isspace(c)
#define isupper(c)  __inline_isupper(c)
#define isxdigit(c) __inline_isxdigit(c)
#ifdef __USE_ISOC99
#define isblank(c)  __inline_isblank(c)
#endif /* __USE_ISOC99 */
#endif /* !__cplusplus */
#else /* Inline */
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
#ifdef __USE_ISOC99
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL isblank)(int __c));
#endif /* __USE_ISOC99 */
#endif /* !Inline */
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL toupper)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL tolower)(int __c));
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
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
#endif /* !__CXX_SYSTEM_HEADER */
#if defined(__USE_KOS) || defined(__USE_DOS)
#ifdef __CYG_COMPAT__
__LIBC __WUNUSED int (__LIBCCALL isascii)(int __c);
#else
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,isascii,(int __c),__isascii,(__c))
#endif
#define isascii(c)  __inline_isascii(c)
#endif /* __USE_KOS || __USE_DOS */

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
#ifdef __DOS_COMPAT__
__LIBC __WUNUSED int (__LIBCCALL _isctype_l)(int __c, int __mask, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isalpha_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isupper_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _islower_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isdigit_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isxdigit_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isspace_l)(int __c, __locale_t __locale);
#if defined(__CRT_DOS) && __CRT_DOS < 2
#ifdef __LIBCCALL_CALLER_CLEANUP
__REDIRECT(__LIBC,__WUNUSED,int,__LIBCCALL,_ispunct_l,(int __c, __locale_t __locale),ispunct,(__c,__locale))
__REDIRECT(__LIBC,__WUNUSED,int,__LIBCCALL,_isblank_l,(int __c, __locale_t __locale),isblank,(__c,__locale))
#else /* __LIBCCALL_CALLER_CLEANUP */
__LOCAL __WUNUSED int (__LIBCCALL _ispunct_l)(int __c, __locale_t __UNUSED(__locale)) { return __NAMESPACE_STD_SYM ispunct(__c); }
__LOCAL __WUNUSED int (__LIBCCALL _isblank_l)(int __c, __locale_t __UNUSED(__locale)) { return __NAMESPACE_STD_SYM isblank(__c); }
#endif /* !__LIBCCALL_CALLER_CLEANUP */
#else
__LIBC __WUNUSED int (__LIBCCALL _ispunct_l)(int __c, __locale_t __locale);
__LIBC __WUNUSED int (__LIBCCALL _isblank_l)(int __c, __locale_t __locale);
#endif
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
#else /* ... */
#ifdef __isctype
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL _isctype_l)(int __c, int __mask, __locale_t __UNUSED(__locale))) { return __isctype(__c,__mask); }
#endif /* __isctype */
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
#endif /* !... */

#ifdef __DOS_COMPAT__
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL __isascii)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL __toascii)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL __iscsymf)(int __c));
__LIBC __WUNUSED int __NOTHROW((__LIBCCALL __iscsym)(int __c));
#else /* __DOS_COMPAT__ */
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL __isascii)(int __c)) { return __isascii(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL __toascii)(int __c)) { return __toascii(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL __iscsymf)(int __c)) { return __NAMESPACE_STD_SYM isalpha(__c) || __c == '_'; }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL __iscsym)(int __c)) { return __NAMESPACE_STD_SYM isalnum(__c) || __c == '_'; }
#endif /* !__DOS_COMPAT__ */

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

#ifdef __DOS_COMPAT__
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,toascii,(int __c),__toascii,(__c))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iscsymf,(int __c),__iscsymf,(__c))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iscsym,(int __c),__iscsym,(__c))
#else /* __DOS_COMPAT__ */
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL toascii)(int __c)) { return __toascii(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL iscsymf)(int __c)) { return __iscsymf(__c); }
__LOCAL __WUNUSED int __NOTHROW((__LIBCCALL iscsym)(int __c)) { return __iscsym(__c); }
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_DOS */

__SYSDECL_END

#endif /* !_CTYPE_H */
