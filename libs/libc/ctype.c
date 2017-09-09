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

#include <ctype.h>
#include <hybrid/byteorder.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdint.h>

DECL_BEGIN

#if __BYTE_ORDER == __BIG_ENDIAN
#   define F_SLOT(x)    x
#else
#   define F_SLOT(x)   (u16)(x << 8|x >> 8)
#endif

#define F_UPPER  0x0000 /*< UPPERCASE. */
#define F_LOWER  0x0001 /*< lowercase. */
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

PUBLIC u16 const __chattr[256] = {
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
    F_SLOT(0x04c0),F_SLOT(0x08d4),F_SLOT(0x08d4),F_SLOT(0x08d4),F_SLOT(0x08d4),F_SLOT(0x08d4),F_SLOT(0x08d4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),
    F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x08c4),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),
    F_SLOT(0x04c0),F_SLOT(0x08d5),F_SLOT(0x08d5),F_SLOT(0x08d5),F_SLOT(0x08d5),F_SLOT(0x08d5),F_SLOT(0x08d5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),
    F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x08c5),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x04c0),F_SLOT(0x0200),
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

PUBLIC int (LIBCCALL isalpha)(int ch)  { return isalpha(ch); }
PUBLIC int (LIBCCALL isupper)(int ch)  { return isupper(ch); }
PUBLIC int (LIBCCALL islower)(int ch)  { return islower(ch); }
PUBLIC int (LIBCCALL isdigit)(int ch)  { return isdigit(ch); }
PUBLIC int (LIBCCALL isxdigit)(int ch) { return isxdigit(ch); }
PUBLIC int (LIBCCALL isspace)(int ch)  { return isspace(ch); }
PUBLIC int (LIBCCALL ispunct)(int ch)  { return ispunct(ch); }
PUBLIC int (LIBCCALL isalnum)(int ch)  { return isalnum(ch); }
PUBLIC int (LIBCCALL isprint)(int ch)  { return isprint(ch); }
PUBLIC int (LIBCCALL isgraph)(int ch)  { return isgraph(ch); }
PUBLIC int (LIBCCALL iscntrl)(int ch)  { return iscntrl(ch); }
PUBLIC int (LIBCCALL isblank)(int ch)  { return isblank(ch); }
PUBLIC int (LIBCCALL toupper)(int ch)  { return ASCII_TOUPPER(ch); }
PUBLIC int (LIBCCALL tolower)(int ch)  { return ASCII_TOLOWER(ch); }
PUBLIC int (LIBCCALL isctype)(int ch, int mask) { return __chattr[(uint8_t)ch] & (__UINT16_TYPE__)mask; }


DECL_END

#endif /* !GUARD_LIBS_LIBC_CTYPE_C */
