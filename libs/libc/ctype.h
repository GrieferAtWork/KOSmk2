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
#ifndef GUARD_LIBS_LIBC_CTYPE_H
#define GUARD_LIBS_LIBC_CTYPE_H 1
#define _ISOC99_SOURCE 1

#include "libc.h"
#include <hybrid/byteorder.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <xlocale.h>

DECL_BEGIN

#if __BYTE_ORDER == __BIG_ENDIAN
#   define F_SLOT(x)    x
#else
#   define F_SLOT(x)   (u16)(x << 8|x >> 8)
#endif

#define F_UPPER  0x0001 /*< UPPERCASE. */
#define F_LOWER  0x0002 /*< lowercase. */
#define F_ALPHA  0x0004 /*< Alphabetic. */
#define F_DIGIT  0x0008 /*< Numeric. */
#define F_XDIGIT 0x0010 /*< Hexadecimal numeric. */
#define F_SPACE  0x0020 /*< Whitespace. */
#define F_PRINT  0x0040 /*< Printing. */
#define F_GRAPH  0x0080 /*< Graphical. */
#define F_BLANK  0x0100 /*< Blank (usually SPC and TAB). */
#define F_CNTRL  0x0200 /*< Control character. */
#define F_PUNCT  0x0400 /*< Punctuation. */
#define F_ALNUM  0x0800 /*< Alphanumeric. */

#define ASCII_ISCNTRL(ch)  ((ch) <= 0x1f || (ch) == 0x7f)
#define ASCII_ISBLANK(ch)  ((ch) == 0x09 || (ch) == 0x20)
#define ASCII_ISSPACE(ch)  (((ch) >= 0x09 && (ch) <= 0x0d) || (ch) == 0x20)
#define ASCII_ISUPPER(ch)  ((ch) >= 0x41 && (ch) <= 0x5a)
#define ASCII_ISLOWER(ch)  ((ch) >= 0x61 && (ch) <= 0x7a)
#define ASCII_ISALPHA(ch)  (ASCII_ISUPPER(ch) || ASCII_ISLOWER(ch))
#define ASCII_ISDIGIT(ch)  ((ch) >= 0x30 && (ch) <= 0x39)
#define ASCII_ISXDIGIT(ch) (ASCII_ISDIGIT(ch) || \
                           ((ch) >= 0x41 && (ch) <= 0x46) || \
                           ((ch) >= 0x61 && (ch) <= 0x66))
#define ASCII_ISALNUM(ch)  (ASCII_ISUPPER(ch) || ASCII_ISLOWER(ch) || ASCII_ISDIGIT(ch))
#define ASCII_ISPUNCT(ch)  (((ch) >= 0x21 && (ch) <= 0x2f) || \
                            ((ch) >= 0x3a && (ch) <= 0x40) || \
                            ((ch) >= 0x5b && (ch) <= 0x60) || \
                            ((ch) >= 0x7b && (ch) <= 0x7e))
#define ASCII_ISGRAPH(ch)  ((ch) >= 0x21 && (ch) <= 0x7e)
#define ASCII_ISPRINT(ch)  ((ch) >= 0x20 && (ch) <= 0x7e)
#define ASCII_TOLOWER(ch)  (ASCII_ISUPPER(ch) ? ((ch)+0x20) : (ch))
#define ASCII_TOUPPER(ch)  (ASCII_ISLOWER(ch) ? ((ch)-0x20) : (ch))

INTDEF u16 const libc___chattr[256];
INTDEF int (LIBCCALL libc_isalpha)(int ch);
INTDEF int (LIBCCALL libc_isupper)(int ch);
INTDEF int (LIBCCALL libc_islower)(int ch);
INTDEF int (LIBCCALL libc_isdigit)(int ch);
INTDEF int (LIBCCALL libc_isxdigit)(int ch);
INTDEF int (LIBCCALL libc_isspace)(int ch);
INTDEF int (LIBCCALL libc_ispunct)(int ch);
INTDEF int (LIBCCALL libc_isalnum)(int ch);
INTDEF int (LIBCCALL libc_isprint)(int ch);
INTDEF int (LIBCCALL libc_isgraph)(int ch);
INTDEF int (LIBCCALL libc_iscntrl)(int ch);
INTDEF int (LIBCCALL libc_isblank)(int ch);
INTDEF int (LIBCCALL libc_toupper)(int ch);
INTDEF int (LIBCCALL libc_tolower)(int ch);
INTDEF int (LIBCCALL libc_isctype)(int ch, int mask);

#ifndef __KERNEL__
INTDEF int (LIBCCALL libc__toupper)(int ch);
INTDEF int (LIBCCALL libc__tolower)(int ch);
#endif /* !__KERNEL__ */

#define libc___isctype(c,type) (libc___chattr[(__UINT8_TYPE__)(c)]&(__UINT16_TYPE__)type)
#define libc_isalnum(c)  libc___isctype((c),F_SLOT(F_ALNUM))
#define libc_isalpha(c)  libc___isctype((c),F_SLOT(F_ALPHA))
#define libc_iscntrl(c)  libc___isctype((c),F_SLOT(F_CNTRL))
#define libc_isdigit(c)  libc___isctype((c),F_SLOT(F_DIGIT))
#define libc_islower(c)  libc___isctype((c),F_SLOT(F_LOWER))
#define libc_isgraph(c)  libc___isctype((c),F_SLOT(F_GRAPH))
#define libc_isprint(c)  libc___isctype((c),F_SLOT(F_PRINT))
#define libc_ispunct(c)  libc___isctype((c),F_SLOT(F_PUNCT))
#define libc_isspace(c)  libc___isctype((c),F_SLOT(F_SPACE))
#define libc_isupper(c)  libc___isctype((c),F_SLOT(F_UPPER))
#define libc_isxdigit(c) libc___isctype((c),F_SLOT(F_XDIGIT))
#define libc_isblank(c)  libc___isctype((c),F_SLOT(F_BLANK))
#define libc_isascii(c)  ((unsigned int)(c) <= 0x7f)

#ifndef __KERNEL__
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

#ifndef __wctrans_t_defined
#define __wctrans_t_defined 1
typedef __int32_t const *wctrans_t;
#endif /* !__wctrans_t_defined */

INTDEF int LIBCCALL libc_iswalnum(wint_t wc);
INTDEF int LIBCCALL libc_iswalpha(wint_t wc);
INTDEF int LIBCCALL libc_iswcntrl(wint_t wc);
INTDEF int LIBCCALL libc_iswdigit(wint_t wc);
INTDEF int LIBCCALL libc_iswgraph(wint_t wc);
INTDEF int LIBCCALL libc_iswlower(wint_t wc);
INTDEF int LIBCCALL libc_iswprint(wint_t wc);
INTDEF int LIBCCALL libc_iswpunct(wint_t wc);
INTDEF int LIBCCALL libc_iswspace(wint_t wc);
INTDEF int LIBCCALL libc_iswupper(wint_t wc);
INTDEF int LIBCCALL libc_iswxdigit(wint_t wc);
INTDEF int LIBCCALL libc_iswblank(wint_t wc);
INTDEF int LIBCCALL libc_iswascii(wint_t wc);
INTDEF wint_t LIBCCALL libc_towlower(wint_t wc);
INTDEF wint_t LIBCCALL libc_towupper(wint_t wc);
INTDEF wctype_t LIBCCALL libc_wctype(char const *prop);
INTDEF int LIBCCALL libc_iswctype(wint_t wc, wctype_t desc);
INTDEF wctrans_t LIBCCALL libc_wctrans(char const *prop);
INTDEF wint_t LIBCCALL libc_towctrans(wint_t wc, wctrans_t desc);
INTDEF int LIBCCALL libc_iswalnum_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswalpha_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswcntrl_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswdigit_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswgraph_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswlower_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswprint_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswpunct_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswspace_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswupper_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswxdigit_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswblank_l(wint_t wc, locale_t locale);
INTDEF wctype_t LIBCCALL libc_wctype_l(char const *prop, locale_t locale);
INTDEF int LIBCCALL libc_iswctype_l(wint_t wc, wctype_t desc, locale_t locale);
INTDEF wint_t LIBCCALL libc_towlower_l(wint_t wc, locale_t locale);
INTDEF wint_t LIBCCALL libc_towupper_l(wint_t wc, locale_t locale);
INTDEF wctrans_t LIBCCALL libc_wctrans_l(char const *prop, locale_t locale);
INTDEF wint_t LIBCCALL libc_towctrans_l(wint_t wc, wctrans_t desc, locale_t locale);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF int LIBCCALL libc_isleadbyte(int wc);
INTDEF int LIBCCALL libc_isleadbyte_l(int wc, locale_t locale);
INTDEF int LIBCCALL libc_iswcsym(wint_t wc);
INTDEF int LIBCCALL libc_iswcsymf(wint_t wc);
INTDEF int LIBCCALL libc_iswcsym_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswcsymf_l(wint_t wc, locale_t locale);

INTDEF int LIBCCALL libc_isalpha_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isupper_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_islower_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isdigit_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isxdigit_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isspace_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_ispunct_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isalnum_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isprint_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isgraph_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_iscntrl_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isblank_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_toupper_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_tolower_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isctype_l(int ch, int mask, locale_t locale);
INTDEF int LIBCCALL libc_dos_isctype(int ch, int mask);
INTDEF int LIBCCALL libc_toascii(int ch);
INTERN int LIBCCALL libc_iscsym(int ch);
INTERN int LIBCCALL libc_iscsymf(int ch);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_CTYPE_H */
