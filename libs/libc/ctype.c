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
#ifndef GUARD_LIBS_LIBC_CTYPE_C
#define GUARD_LIBS_LIBC_CTYPE_C 1
#define _ISOC99_SOURCE 1

#include "libc.h"
#include "ctype.h"
#include "string.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdint.h>
#include <xlocale.h>
#include <wctype.h>

DECL_BEGIN

INTERN u16 const libc___chattr[256] = {
/*[[[deemon
function attrof(ch) {
    local result = 0;
    if (ASCII_ISCNTRL(ch))  result |= F_CNTRL;
    if (ASCII_ISBLANK(ch))  result |= F_BLANK;
    if (ASCII_ISSPACE(ch))  result |= F_SPACE;
    if (ASCII_ISUPPER(ch))  result |= F_UPPER;
    if (ASCII_ISLOWER(ch))  result |= F_LOWER;
    if (ASCII_ISALPHA(ch))  result |= F_ALPHA;
    if (ASCII_ISDIGIT(ch))  result |= F_DIGIT;
    if (ASCII_ISXDIGIT(ch)) result |= F_XDIGIT;
    if (ASCII_ISALNUM(ch))  result |= F_ALNUM;
    if (ASCII_ISPUNCT(ch))  result |= F_PUNCT;
    if (ASCII_ISGRAPH(ch))  result |= F_GRAPH;
    if (ASCII_ISPRINT(ch))  result |= F_PRINT;
    return result;
}
for (local c = 0; c < 256; ++c) {
    if (!(c % 16)) print c ? "\n    " : "    ",;
    print "F_SLOT(%#.4x)," % attrof(c),;
}
]]]*/
    F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0320),F_SLOT(0x0220),F_SLOT(0x0220),F_SLOT(0x0220),F_SLOT(0x0220),F_SLOT(0x0200),F_SLOT(0x0200),
    F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),F_SLOT(0x0200),
    F_SLOT(0x0160),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),
    F_SLOT(0x08d8),F_SLOT(0x08d8),F_SLOT(0x08d8),F_SLOT(0x08d8),F_SLOT(0x08d8),F_SLOT(0x08d8),F_SLOT(0x08d8),F_SLOT(0x08d8),F_SLOT(0x08d8),F_SLOT(0x08d8),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),
    F_SLOT(0x04c0),F_SLOT(0x08d5),F_SLOT(0x08d5),F_SLOT(0x08d5),F_SLOT(0x08d5),F_SLOT(0x08d5),F_SLOT(0x08d5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),
    F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),
    F_SLOT(0x04c0),F_SLOT(0x08d6),F_SLOT(0x08d6),F_SLOT(0x08d6),F_SLOT(0x08d6),F_SLOT(0x08d6),F_SLOT(0x08d6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),
    F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x08c6),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x0200),
    F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),
    F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),
    F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),
    F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),
    F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),
    F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),
    F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),
    F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),F_SLOT(0x0000),
//[[[end]]]
};

INTERN int (LIBCCALL libc_isalpha)(int ch)  { return libc_isalpha(ch); }
INTERN int (LIBCCALL libc_isupper)(int ch)  { return libc_isupper(ch); }
INTERN int (LIBCCALL libc_islower)(int ch)  { return libc_islower(ch); }
INTERN int (LIBCCALL libc_isdigit)(int ch)  { return libc_isdigit(ch); }
INTERN int (LIBCCALL libc_isxdigit)(int ch) { return libc_isxdigit(ch); }
INTERN int (LIBCCALL libc_isspace)(int ch)  { return libc_isspace(ch); }
INTERN int (LIBCCALL libc_ispunct)(int ch)  { return libc_ispunct(ch); }
INTERN int (LIBCCALL libc_isalnum)(int ch)  { return libc_isalnum(ch); }
INTERN int (LIBCCALL libc_isprint)(int ch)  { return libc_isprint(ch); }
INTERN int (LIBCCALL libc_isgraph)(int ch)  { return libc_isgraph(ch); }
INTERN int (LIBCCALL libc_iscntrl)(int ch)  { return libc_iscntrl(ch); }
INTERN int (LIBCCALL libc_isblank)(int ch)  { return libc_isblank(ch); }
INTERN int (LIBCCALL libc_isascii)(int ch)  { return libc_isascii(ch); }
INTERN int (LIBCCALL libc_toupper)(int ch)  { return ASCII_TOUPPER(ch); }
INTERN int (LIBCCALL libc_tolower)(int ch)  { return ASCII_TOLOWER(ch); }
INTERN int (LIBCCALL libc_isctype)(int ch, int mask) { return libc___isctype(ch,mask); }

/* Wide-string API */
#ifndef __KERNEL__
/* XXX: Locales? */
#if __SIZEOF_WINT_T__ != __SIZEOF_INT__
INTERN int LIBCCALL libc_iswalnum(wint_t wc) { return libc_isalnum((int)wc); }
INTERN int LIBCCALL libc_iswalpha(wint_t wc) { return libc_isalpha((int)wc); }
INTERN int LIBCCALL libc_iswcntrl(wint_t wc) { return libc_iscntrl((int)wc); }
INTERN int LIBCCALL libc_iswdigit(wint_t wc) { return libc_isdigit((int)wc); }
INTERN int LIBCCALL libc_iswgraph(wint_t wc) { return libc_isgraph((int)wc); }
INTERN int LIBCCALL libc_iswlower(wint_t wc) { return libc_islower((int)wc); }
INTERN int LIBCCALL libc_iswprint(wint_t wc) { return libc_isprint((int)wc); }
INTERN int LIBCCALL libc_iswpunct(wint_t wc) { return libc_ispunct((int)wc); }
INTERN int LIBCCALL libc_iswspace(wint_t wc) { return libc_isspace((int)wc); }
INTERN int LIBCCALL libc_iswupper(wint_t wc) { return libc_isupper((int)wc); }
INTERN int LIBCCALL libc_iswxdigit(wint_t wc) { return libc_isxdigit((int)wc); }
INTERN int LIBCCALL libc_iswblank(wint_t wc) { return libc_isblank((int)wc); }
INTERN int LIBCCALL libc_iswascii(wint_t wc) { return libc_isascii((int)wc); }
#if __SIZEOF_WINT_T__ > __SIZEOF_INT__
INTERN wint_t LIBCCALL libc_towlower(wint_t wc) { return wc < 256 ? (wint_t)libc_tolower((int)wc) : wc; }
INTERN wint_t LIBCCALL libc_towupper(wint_t wc) { return wc < 256 ? (wint_t)libc_toupper((int)wc) : wc; }
#else /* __SIZEOF_WINT_T__ > __SIZEOF_INT__ */
INTERN wint_t LIBCCALL libc_towlower(wint_t wc) { return (wint_t)libc_tolower((int)wc); }
INTERN wint_t LIBCCALL libc_towupper(wint_t wc) { return (wint_t)libc_toupper((int)wc); }
#endif /* __SIZEOF_WINT_T__ <= __SIZEOF_INT__ */
#endif /* __SIZEOF_WINT_T__ != __SIZEOF_INT__ */

#undef libc_isalpha
#undef libc_isupper
#undef libc_islower
#undef libc_isdigit
#undef libc_isxdigit
#undef libc_isspace
#undef libc_ispunct
#undef libc_isalnum
#undef libc_isprint
#undef libc_isgraph
#undef libc_iscntrl
#undef libc_isblank
#undef libc_isascii

#if __SIZEOF_WINT_T__ == __SIZEOF_INT__
DEFINE_INTERN_ALIAS(libc_iswalnum,libc_isalnum);
DEFINE_INTERN_ALIAS(libc_iswalpha,libc_isalpha);
DEFINE_INTERN_ALIAS(libc_iswcntrl,libc_iscntrl);
DEFINE_INTERN_ALIAS(libc_iswdigit,libc_isdigit);
DEFINE_INTERN_ALIAS(libc_iswgraph,libc_isgraph);
DEFINE_INTERN_ALIAS(libc_iswlower,libc_islower);
DEFINE_INTERN_ALIAS(libc_iswprint,libc_isprint);
DEFINE_INTERN_ALIAS(libc_iswpunct,libc_ispunct);
DEFINE_INTERN_ALIAS(libc_iswspace,libc_isspace);
DEFINE_INTERN_ALIAS(libc_iswupper,libc_isupper);
DEFINE_INTERN_ALIAS(libc_iswxdigit,libc_isxdigit);
DEFINE_INTERN_ALIAS(libc_iswblank,libc_isblank);
DEFINE_INTERN_ALIAS(libc_iswascii,libc_isascii);
DEFINE_INTERN_ALIAS(libc_towlower,libc_tolower);
DEFINE_INTERN_ALIAS(libc_towupper,libc_toupper);
#endif /* __SIZEOF_WINT_T__ == __SIZEOF_INT__ */


struct prop { char name[8]; };
PRIVATE struct prop const prop_names[12] = {
    [__ISwupper ] = { .name = "upper"  }, /*< UPPERCASE. */
    [__ISwlower ] = { .name = "lower"  }, /*< lowercase. */
    [__ISwalpha ] = { .name = "alpha"  }, /*< Alphabetic. */
    [__ISwdigit ] = { .name = "digit"  }, /*< Numeric. */
    [__ISwxdigit] = { .name = "xdigit" }, /*< Hexadecimal numeric. */
    [__ISwspace ] = { .name = "space"  }, /*< Whitespace. */
    [__ISwprint ] = { .name = "print"  }, /*< Printing. */
    [__ISwgraph ] = { .name = "graph"  }, /*< Graphical. */
    [__ISwblank ] = { .name = "blank"  }, /*< Blank (usually SPC and TAB). */
    [__ISwcntrl ] = { .name = "cntrl"  }, /*< Control character. */
    [__ISwpunct ] = { .name = "punct"  }, /*< Punctuation. */
    [__ISwalnum ] = { .name = "alnum"  }, /*< Alphanumeric. */
};
INTERN wctype_t LIBCCALL libc_wctype(char const *prop) {
 struct prop const *iter = prop_names;
 for (; iter != COMPILER_ENDOF(prop_names); ++iter) {
  if (!libc_strcmp(iter->name,prop)) {
   int bit = (int)(iter-prop_names);
   return (wctype_t)_ISwbit(bit);
  }
 }
 return 0;
}
INTERN int LIBCCALL libc_iswctype(wint_t wc, wctype_t desc) {
 return wc < 256 && (libc___chattr[(u8)wc] & (u16)desc);
}

struct trans { char name[8]; };
#define TRANS_LOWER 0
#define TRANS_UPPER 1
PRIVATE struct trans const trans_names[2] = {
    [TRANS_LOWER] = { .name = "tolower"  },
    [TRANS_UPPER] = { .name = "toupper"  },
};
INTERN wctrans_t LIBCCALL libc_wctrans(char const *prop) {
 struct trans const *iter = trans_names;
 for (; iter != COMPILER_ENDOF(trans_names); ++iter) {
  if (!libc_strcmp(iter->name,prop)) {
   return (wctrans_t)((uintptr_t)(iter-trans_names));
  }
 }
 return 0;
}
INTERN wint_t LIBCCALL libc_towctrans(wint_t wc, wctrans_t desc) {
 switch ((uintptr_t)desc) {
 case TRANS_LOWER: return libc_towlower(wc);
 case TRANS_UPPER: return libc_towupper(wc);
 default: break;
 }
 return wc;
}

#if 1 /* Ignore locale in character-trait functions. */
#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_iswalnum_l,libc_iswalnum);
DEFINE_INTERN_ALIAS(libc_iswalpha_l,libc_iswalpha);
DEFINE_INTERN_ALIAS(libc_iswcntrl_l,libc_iswcntrl);
DEFINE_INTERN_ALIAS(libc_iswdigit_l,libc_iswdigit);
DEFINE_INTERN_ALIAS(libc_iswgraph_l,libc_iswgraph);
DEFINE_INTERN_ALIAS(libc_iswlower_l,libc_iswlower);
DEFINE_INTERN_ALIAS(libc_iswprint_l,libc_iswprint);
DEFINE_INTERN_ALIAS(libc_iswpunct_l,libc_iswpunct);
DEFINE_INTERN_ALIAS(libc_iswspace_l,libc_iswspace);
DEFINE_INTERN_ALIAS(libc_iswupper_l,libc_iswupper);
DEFINE_INTERN_ALIAS(libc_iswxdigit_l,libc_iswxdigit);
DEFINE_INTERN_ALIAS(libc_iswblank_l,libc_iswblank);
DEFINE_INTERN_ALIAS(libc_wctype_l,libc_wctype);
DEFINE_INTERN_ALIAS(libc_iswctype_l,libc_iswctype);
DEFINE_INTERN_ALIAS(libc_towlower_l,libc_towlower);
DEFINE_INTERN_ALIAS(libc_towupper_l,libc_towupper);
DEFINE_INTERN_ALIAS(libc_wctrans_l,libc_wctrans);
DEFINE_INTERN_ALIAS(libc_towctrans_l,libc_towctrans);
#else
INTERN int LIBCCALL libc_iswalnum_l(wint_t wc, locale_t locale) { return libc_iswalnum(wc); }
INTERN int LIBCCALL libc_iswalpha_l(wint_t wc, locale_t locale) { return libc_iswalpha(wc); }
INTERN int LIBCCALL libc_iswcntrl_l(wint_t wc, locale_t locale) { return libc_iswcntrl(wc); }
INTERN int LIBCCALL libc_iswdigit_l(wint_t wc, locale_t locale) { return libc_iswdigit(wc); }
INTERN int LIBCCALL libc_iswgraph_l(wint_t wc, locale_t locale) { return libc_iswgraph(wc); }
INTERN int LIBCCALL libc_iswlower_l(wint_t wc, locale_t locale) { return libc_iswlower(wc); }
INTERN int LIBCCALL libc_iswprint_l(wint_t wc, locale_t locale) { return libc_iswprint(wc); }
INTERN int LIBCCALL libc_iswpunct_l(wint_t wc, locale_t locale) { return libc_iswpunct(wc); }
INTERN int LIBCCALL libc_iswspace_l(wint_t wc, locale_t locale) { return libc_iswspace(wc); }
INTERN int LIBCCALL libc_iswupper_l(wint_t wc, locale_t locale) { return libc_iswupper(wc); }
INTERN int LIBCCALL libc_iswxdigit_l(wint_t wc, locale_t locale) { return libc_iswxdigit(wc); }
INTERN int LIBCCALL libc_iswblank_l(wint_t wc, locale_t locale) { return libc_iswblank(wc); }
INTERN wctype_t LIBCCALL libc_wctype_l(char const *prop, locale_t locale) { return libc_wctype(prop); }
INTERN int LIBCCALL libc_iswctype_l(wint_t wc, wctype_t desc, locale_t locale) { return libc_iswctype(wc,desc); }
INTERN wint_t LIBCCALL libc_towlower_l(wint_t wc, locale_t locale) { return libc_towlower(wc); }
INTERN wint_t LIBCCALL libc_towupper_l(wint_t wc, locale_t locale) { return libc_towupper(wc); }
INTERN wctrans_t LIBCCALL libc_wctrans_l(char const *prop, locale_t locale) { return libc_wctrans(prop); }
INTERN wint_t LIBCCALL libc_towctrans_l(wint_t wc, wctrans_t desc, locale_t locale) { return libc_towctrans(wc,desc); }
#endif
#else
INTERN int LIBCCALL libc_iswalnum_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswalnum(wc); }
INTERN int LIBCCALL libc_iswalpha_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswalpha(wc); }
INTERN int LIBCCALL libc_iswcntrl_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswcntrl(wc); }
INTERN int LIBCCALL libc_iswdigit_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswdigit(wc); }
INTERN int LIBCCALL libc_iswgraph_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswgraph(wc); }
INTERN int LIBCCALL libc_iswlower_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswlower(wc); }
INTERN int LIBCCALL libc_iswprint_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswprint(wc); }
INTERN int LIBCCALL libc_iswpunct_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswpunct(wc); }
INTERN int LIBCCALL libc_iswspace_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswspace(wc); }
INTERN int LIBCCALL libc_iswupper_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswupper(wc); }
INTERN int LIBCCALL libc_iswxdigit_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswxdigit(wc); }
INTERN int LIBCCALL libc_iswblank_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswblank(wc); }
INTERN wctype_t LIBCCALL libc_wctype_l(char const *prop, locale_t locale) { NOT_IMPLEMENTED(); return libc_wctype(prop); }
INTERN int LIBCCALL libc_iswctype_l(wint_t wc, wctype_t desc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswctype(wc,desc); }
INTERN wint_t LIBCCALL libc_towlower_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_towlower(wc); }
INTERN wint_t LIBCCALL libc_towupper_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_towupper(wc); }
INTERN wctrans_t LIBCCALL libc_wctrans_l(char const *prop, locale_t locale) { NOT_IMPLEMENTED(); return libc_wctrans(prop); }
INTERN wint_t LIBCCALL libc_towctrans_l(wint_t wc, wctrans_t desc, locale_t locale) { NOT_IMPLEMENTED(); return libc_towctrans(wc,desc); }
#endif

#undef iswalnum
#undef iswalpha
#undef iswcntrl
#undef iswdigit
#undef iswgraph
#undef iswlower
#undef iswprint
#undef iswpunct
#undef iswspace
#undef iswupper
#undef iswxdigit
#undef iswblank
#undef iswascii
#undef iswalnum_l
#undef iswalpha_l
#undef iswcntrl_l
#undef iswdigit_l
#undef iswgraph_l
#undef iswlower_l
#undef iswprint_l
#undef iswpunct_l
#undef iswspace_l
#undef iswupper_l
#undef iswxdigit_l
#undef iswblank_l

DEFINE_PUBLIC_ALIAS(iswalnum,libc_iswalnum);
DEFINE_PUBLIC_ALIAS(iswalpha,libc_iswalpha);
DEFINE_PUBLIC_ALIAS(iswcntrl,libc_iswcntrl);
DEFINE_PUBLIC_ALIAS(iswdigit,libc_iswdigit);
DEFINE_PUBLIC_ALIAS(iswgraph,libc_iswgraph);
DEFINE_PUBLIC_ALIAS(iswlower,libc_iswlower);
DEFINE_PUBLIC_ALIAS(iswprint,libc_iswprint);
DEFINE_PUBLIC_ALIAS(iswpunct,libc_iswpunct);
DEFINE_PUBLIC_ALIAS(iswspace,libc_iswspace);
DEFINE_PUBLIC_ALIAS(iswupper,libc_iswupper);
DEFINE_PUBLIC_ALIAS(iswxdigit,libc_iswxdigit);
DEFINE_PUBLIC_ALIAS(iswblank,libc_iswblank);
DEFINE_PUBLIC_ALIAS(iswascii,libc_iswascii);
DEFINE_PUBLIC_ALIAS(towlower,libc_towlower);
DEFINE_PUBLIC_ALIAS(towupper,libc_towupper);
DEFINE_PUBLIC_ALIAS(wctype,libc_wctype);
DEFINE_PUBLIC_ALIAS(iswctype,libc_iswctype);
DEFINE_PUBLIC_ALIAS(wctrans,libc_wctrans);
DEFINE_PUBLIC_ALIAS(towctrans,libc_towctrans);
DEFINE_PUBLIC_ALIAS(iswalnum_l,libc_iswalnum_l);
DEFINE_PUBLIC_ALIAS(iswalpha_l,libc_iswalpha_l);
DEFINE_PUBLIC_ALIAS(iswcntrl_l,libc_iswcntrl_l);
DEFINE_PUBLIC_ALIAS(iswdigit_l,libc_iswdigit_l);
DEFINE_PUBLIC_ALIAS(iswgraph_l,libc_iswgraph_l);
DEFINE_PUBLIC_ALIAS(iswlower_l,libc_iswlower_l);
DEFINE_PUBLIC_ALIAS(iswprint_l,libc_iswprint_l);
DEFINE_PUBLIC_ALIAS(iswpunct_l,libc_iswpunct_l);
DEFINE_PUBLIC_ALIAS(iswspace_l,libc_iswspace_l);
DEFINE_PUBLIC_ALIAS(iswupper_l,libc_iswupper_l);
DEFINE_PUBLIC_ALIAS(iswxdigit_l,libc_iswxdigit_l);
DEFINE_PUBLIC_ALIAS(iswblank_l,libc_iswblank_l);
DEFINE_PUBLIC_ALIAS(wctype_l,libc_wctype_l);
DEFINE_PUBLIC_ALIAS(iswctype_l,libc_iswctype_l);
DEFINE_PUBLIC_ALIAS(towlower_l,libc_towlower_l);
DEFINE_PUBLIC_ALIAS(towupper_l,libc_towupper_l);
DEFINE_PUBLIC_ALIAS(wctrans_l,libc_wctrans_l);
DEFINE_PUBLIC_ALIAS(towctrans_l,libc_towctrans_l);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(_iswalnum_l,libc_iswalnum_l);
DEFINE_PUBLIC_ALIAS(_iswalpha_l,libc_iswalpha_l);
DEFINE_PUBLIC_ALIAS(_iswcntrl_l,libc_iswcntrl_l);
DEFINE_PUBLIC_ALIAS(_iswdigit_l,libc_iswdigit_l);
DEFINE_PUBLIC_ALIAS(_iswgraph_l,libc_iswgraph_l);
DEFINE_PUBLIC_ALIAS(_iswlower_l,libc_iswlower_l);
DEFINE_PUBLIC_ALIAS(_iswprint_l,libc_iswprint_l);
DEFINE_PUBLIC_ALIAS(_iswpunct_l,libc_iswpunct_l);
DEFINE_PUBLIC_ALIAS(_iswspace_l,libc_iswspace_l);
DEFINE_PUBLIC_ALIAS(_iswupper_l,libc_iswupper_l);
DEFINE_PUBLIC_ALIAS(_iswxdigit_l,libc_iswxdigit_l);
DEFINE_PUBLIC_ALIAS(_iswblank_l,libc_iswblank_l);
DEFINE_PUBLIC_ALIAS(_wctype_l,libc_wctype_l);
DEFINE_PUBLIC_ALIAS(_wctrans_l,libc_wctrans_l);
DEFINE_PUBLIC_ALIAS(_towctrans_l,libc_towctrans_l);
#endif

#endif /* !__KERNEL__ */

#undef __chattr
DEFINE_PUBLIC_ALIAS(__chattr,libc___chattr);
DEFINE_PUBLIC_ALIAS(isalpha,libc_isalpha);
DEFINE_PUBLIC_ALIAS(isupper,libc_isupper);
DEFINE_PUBLIC_ALIAS(islower,libc_islower);
DEFINE_PUBLIC_ALIAS(isdigit,libc_isdigit);
DEFINE_PUBLIC_ALIAS(isxdigit,libc_isxdigit);
DEFINE_PUBLIC_ALIAS(isspace,libc_isspace);
DEFINE_PUBLIC_ALIAS(ispunct,libc_ispunct);
DEFINE_PUBLIC_ALIAS(isalnum,libc_isalnum);
DEFINE_PUBLIC_ALIAS(isprint,libc_isprint);
DEFINE_PUBLIC_ALIAS(isgraph,libc_isgraph);
DEFINE_PUBLIC_ALIAS(iscntrl,libc_iscntrl);
DEFINE_PUBLIC_ALIAS(isblank,libc_isblank);
DEFINE_PUBLIC_ALIAS(toupper,libc_toupper);
DEFINE_PUBLIC_ALIAS(tolower,libc_tolower);
DEFINE_PUBLIC_ALIAS(isctype,libc_isctype);
DEFINE_PUBLIC_ALIAS(__isascii,libc_isascii);

#ifndef __KERNEL__
INTERN int (LIBCCALL libc__tolower)(int ch) { return ch+0x20; }
INTERN int (LIBCCALL libc__toupper)(int ch) { return ch-0x20; }
DEFINE_PUBLIC_ALIAS(_tolower,libc__tolower);
DEFINE_PUBLIC_ALIAS(_toupper,libc__toupper);
#endif /* !__KERNEL__ */

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN int LIBCCALL libc_isleadbyte(int wc) { return wc >= 192 && wc <= 255; }
INTERN int LIBCCALL libc_iswcsym(wint_t wc) { return libc___isctype(wc,F_SLOT(F_ALPHA|F_DIGIT)) || wc == '_'; }
INTERN int LIBCCALL libc_iswcsymf(wint_t wc) { return libc___isctype(wc,F_SLOT(F_ALPHA)) || wc == '_'; }
INTERN int LIBCCALL libc_isleadbyte_l(int wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_isleadbyte(wc); }
INTERN int LIBCCALL libc_iswcsym_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswcsym(wc); }
INTERN int LIBCCALL libc_iswcsymf_l(wint_t wc, locale_t locale) { NOT_IMPLEMENTED(); return libc_iswcsymf(wc); }

DEFINE_PUBLIC_ALIAS(isleadbyte,libc_isleadbyte);
DEFINE_PUBLIC_ALIAS(_isleadbyte_l,libc_isleadbyte_l);
DEFINE_PUBLIC_ALIAS(_towupper_l,libc_towupper_l);
DEFINE_PUBLIC_ALIAS(_towlower_l,libc_towlower_l);
DEFINE_PUBLIC_ALIAS(_iswctype_l,libc_iswctype_l);
DEFINE_PUBLIC_ALIAS(is_wctype,libc_iswctype);
DEFINE_PUBLIC_ALIAS(__iswcsymf,libc_iswcsymf);
DEFINE_PUBLIC_ALIAS(__iswcsym,libc_iswcsym);
DEFINE_PUBLIC_ALIAS(_iswcsymf_l,libc_iswcsymf_l);
DEFINE_PUBLIC_ALIAS(_iswcsym_l,libc_iswcsym_l);

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_isalpha_l,libc_isalpha);
DEFINE_INTERN_ALIAS(libc_isupper_l,libc_isupper);
DEFINE_INTERN_ALIAS(libc_islower_l,libc_islower);
DEFINE_INTERN_ALIAS(libc_isdigit_l,libc_isdigit);
DEFINE_INTERN_ALIAS(libc_isxdigit_l,libc_isxdigit);
DEFINE_INTERN_ALIAS(libc_isspace_l,libc_isspace);
DEFINE_INTERN_ALIAS(libc_isalnum_l,libc_isalnum);
DEFINE_INTERN_ALIAS(libc_isprint_l,libc_isprint);
DEFINE_INTERN_ALIAS(libc_isgraph_l,libc_isgraph);
DEFINE_INTERN_ALIAS(libc_iscntrl_l,libc_iscntrl);
DEFINE_INTERN_ALIAS(libc_toupper_l,libc_toupper);
DEFINE_INTERN_ALIAS(libc_tolower_l,libc_tolower);
DEFINE_INTERN_ALIAS(libc_isctype_l,libc_isctype);
#else /* CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP */
INTERN ATTR_DOSTEXT int (LIBCCALL libc_isalpha_l)(int ch, locale_t UNUSED(loc))  { return libc_isalpha(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_isupper_l)(int ch, locale_t UNUSED(loc))  { return libc_isupper(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_islower_l)(int ch, locale_t UNUSED(loc))  { return libc_islower(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_isdigit_l)(int ch, locale_t UNUSED(loc))  { return libc_isdigit(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_isxdigit_l)(int ch, locale_t UNUSED(loc)) { return libc_isxdigit(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_isspace_l)(int ch, locale_t UNUSED(loc))  { return libc_isspace(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_isalnum_l)(int ch, locale_t UNUSED(loc))  { return libc_isalnum(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_isprint_l)(int ch, locale_t UNUSED(loc))  { return libc_isprint(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_isgraph_l)(int ch, locale_t UNUSED(loc))  { return libc_isgraph(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_iscntrl_l)(int ch, locale_t UNUSED(loc))  { return libc_iscntrl(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_toupper_l)(int ch, locale_t UNUSED(loc))  { return libc_toupper(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_tolower_l)(int ch, locale_t UNUSED(loc))  { return libc_tolower(ch); }
INTERN ATTR_DOSTEXT int (LIBCCALL libc_isctype_l)(int ch, int mask, locale_t UNUSED(loc)) { return libc_isctype(ch,mask); }
#endif /* !CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP */
INTERN ATTR_DOSTEXT int LIBCCALL libc_dos_isctype(int ch, int mask) {
 int check_mask = 0,result;
 if (mask&__DOS_UPPER) check_mask |= F_SLOT(F_UPPER);
 if (mask&__DOS_LOWER) check_mask |= F_SLOT(F_LOWER);
 if (mask&__DOS_DIGIT) check_mask |= F_SLOT(F_DIGIT);
 if (mask&__DOS_SPACE) check_mask |= F_SLOT(F_SPACE);
 if (mask&__DOS_PUNCT) check_mask |= F_SLOT(F_PUNCT);
 if (mask&__DOS_CONTROL) check_mask |= F_SLOT(F_CNTRL);
 if (mask&__DOS_BLANK) check_mask |= F_SLOT(F_BLANK);
 if (mask&__DOS_HEX) check_mask |= F_SLOT(F_XDIGIT);
 if (mask&__DOS_ALPHA) check_mask |= F_SLOT(F_XDIGIT);
 result = libc_isctype(ch,check_mask);
 if (mask&__DOS_LEADBYTE) result |= libc_isleadbyte(ch);
 return result;
}
INTDEF ATTR_DOSTEXT int LIBCCALL libc_toascii(int ch) { return ch&0x7f; }
INTERN ATTR_DOSTEXT int LIBCCALL libc_iscsym(int ch) { return libc___isctype(ch,F_SLOT(F_ALPHA|F_DIGIT)) || ch == '_'; }
INTERN ATTR_DOSTEXT int LIBCCALL libc_iscsymf(int ch) { return libc___isctype(ch,F_SLOT(F_ALPHA)) || ch == '_'; }

DEFINE_PUBLIC_ALIAS(_isalpha_l,libc_isalpha_l);
DEFINE_PUBLIC_ALIAS(_isupper_l,libc_isupper_l);
DEFINE_PUBLIC_ALIAS(_islower_l,libc_islower_l);
DEFINE_PUBLIC_ALIAS(_isdigit_l,libc_isdigit_l);
DEFINE_PUBLIC_ALIAS(_isxdigit_l,libc_isxdigit_l);
DEFINE_PUBLIC_ALIAS(_isspace_l,libc_isspace_l);
DEFINE_PUBLIC_ALIAS(_isalnum_l,libc_isalnum_l);
DEFINE_PUBLIC_ALIAS(_isprint_l,libc_isprint_l);
DEFINE_PUBLIC_ALIAS(_isgraph_l,libc_isgraph_l);
DEFINE_PUBLIC_ALIAS(_iscntrl_l,libc_iscntrl_l);
DEFINE_PUBLIC_ALIAS(_toupper_l,libc_toupper_l);
DEFINE_PUBLIC_ALIAS(_tolower_l,libc_tolower_l);
DEFINE_PUBLIC_ALIAS(_isctype_l,libc_isctype_l);
DEFINE_PUBLIC_ALIAS(_isctype,libc_dos_isctype);
DEFINE_PUBLIC_ALIAS(__toascii,libc_toascii);
DEFINE_PUBLIC_ALIAS(__iscsym,libc_iscsym);
DEFINE_PUBLIC_ALIAS(__iscsymf,libc_iscsymf);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_CTYPE_C */
