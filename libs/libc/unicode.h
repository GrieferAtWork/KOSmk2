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
#ifndef GUARD_LIBS_LIBC_UNICODE_H
#define GUARD_LIBS_LIBC_UNICODE_H 1

#include "libc.h"
#include <hybrid/compiler.h>
#ifndef __KERNEL__
#include <hybrid/types.h>
#include <hybrid/section.h>
#include <bits/mbstate.h>
#include <uchar.h>

DECL_BEGIN

#define ATTR_UNITEXT   ATTR_RARETEXT
#define ATTR_UNIRODATA ATTR_RARERODATA
#define ATTR_UNIDATA   ATTR_RAREDATA
#define ATTR_UNIBSS    ATTR_RAREBSS

#define mbstate_reset(p)  libc_memset(p,0,sizeof(mbstate_t))

#define UNICODE_MB_MAX 6

/* Convert 'utf8chars' utf8-characters  to utf-32, storing up to 'bufchars32' characters
 * (that is 'bufchars32*4' bytes) in 'utf32' and returning the actual amount of stored
 * characters (including a terminating NUL-character that will automatically be
 * appended with the 'utf32'-buffer is of sufficient length to represent the
 * entirety of the provided 'utf8' buffer).
 * @param: mode: Set of 'UNICODE_F_*' found below (Unicode mode flags)
 * @return: UNICODE_ERROR: An encoding error occurred and 'state' and 'utf32' are left in an undefined state.
 * >> wchar_t buf[128]; size_t buflen;
 * >> mbstate_t state = MBSTATE_INIT;
 * >> char *text = "Encode this text in UTF-32";
 * >> buflen = libc_8to32n(text,(size_t)-1,buf,sizeof(buf),&state);
 * NOTE: Other functions behave the same, but for different encodings.
 */
INTDEF size_t LIBCCALL libc_utf8to32(char const *__restrict utf8, size_t utf8chars,
                                     char32_t *__restrict utf32, size_t bufchars32,
                                     mbstate_t *__restrict state, u32 mode);
INTDEF size_t LIBCCALL libc_utf8to16(char const *__restrict utf8, size_t utf8chars,
                                     char16_t *__restrict utf16, size_t bufchars16,
                                     mbstate_t *__restrict state, u32 mode);
INTDEF size_t LIBCCALL libc_utf32to8(char32_t const *__restrict utf32, size_t utf32chars,
                                     char *__restrict utf8, size_t bufchars8,
                                     mbstate_t *__restrict state, u32 mode);
INTDEF size_t LIBCCALL libc_utf16to8(char16_t const *__restrict utf16, size_t utf16chars,
                                     char *__restrict utf8, size_t bufchars8,
                                     mbstate_t *__restrict state, u32 mode);
#define UNICODE_ERROR           ((size_t)-1)
#define UNICODE_UTF16HALF       ((size_t)-3)
#define UNICODE_IS_STATEDEPENDENT 1 /* ??? (I think...) */

/* Unicode mode flags. */
#define UNICODE_F_NORMAL         0x0000 /*< Regular string conversion. */
#define UNICODE_F_NOZEROTERM     0x0001 /*< Don't terminate with a NUL-character that is included in the return value. */
#define UNICODE_F_ALWAYSZEROTERM 0x0002 /*< Always zero-terminate, even when an incomplete sequence is passed. */
#define UNICODE_F_STOPONNUL      0x0004 /*< Stop if a NUL-character is encountered in the input string. */
#define UNICODE_F_DOSINGLE       0x0008 /*< Stop after the first fully encoded character. */
#define UNICODE_F_UTF16HALF      0x0010 /*< For 'libc_utf8to16': When the target buffer is too small return 'UNICODE_UTF16HALF' and use the shift state and return the second half next call. */
#define UNICODE_F_UPDATESRC      0x0020 /*< Treat input arguments (the first two) as pointers and update them before returning upon success.
                                         *  NOTE: When set, the first two arguments are interpreted with one additional indirection. */
#define UNICODE_F_SETERRNO       0x0040 /*< Set 'errno' to 'EILSEQ' if a malformed sequence causes 'UNICODE_ERROR' to be returned. */


/* Helper functions that return a libc_malloc'ed string, or NULL upon error. */
INTDEF char16_t *LIBCCALL libc_utf8to16m(char const *__restrict utf8);
INTDEF char32_t *LIBCCALL libc_utf8to32m(char const *__restrict utf8);
INTDEF char *LIBCCALL libc_utf16to8m(char16_t const *__restrict utf16);
INTDEF char *LIBCCALL libc_utf32to8m(char32_t const *__restrict utf32);
INTDEF char16_t *LIBCCALL libc_utf8to16ms(char const *__restrict utf8, size_t utf8chars);
INTDEF char32_t *LIBCCALL libc_utf8to32ms(char const *__restrict utf8, size_t utf8chars);
INTDEF char *LIBCCALL libc_utf16to8ms(char16_t const *__restrict utf16, size_t utf16chars);
INTDEF char *LIBCCALL libc_utf32to8ms(char32_t const *__restrict utf32, size_t utf32chars);


#define libc_api_utf8to16(dst,dstlen,src,srclen) \
 XBLOCK({ mbstate_t state = MBSTATE_INIT; \
          size_t req = libc_utf8to16(src,srclen,dst,dstlen,&state,0); \
          XRETURN unlikely(req >= srclen) ? (SET_ERRNO(req == UNICODE_ERROR ? EINVAL : ERANGE),NULL) : dst; })
#define libc_api_utf8to32(dst,dstlen,src,srclen) \
 XBLOCK({ mbstate_t state = MBSTATE_INIT; \
          size_t req = libc_utf8to32(src,srclen,dst,dstlen,&state,0); \
          XRETURN unlikely(req >= srclen) ? (SET_ERRNO(req == UNICODE_ERROR ? EINVAL : ERANGE),NULL) : dst; })

/* Return the length (that is amount of indexable characters) of a
 * given utf-8 string containing 'utf8chars' characters.
 * >> char   utf8[] = "This is a UTF-8 string";
 * >> size_t utf8chars = (sizeof(u16_string)/sizeof(char16_t))-1; // Amount of indexable character units.
 * >> size_t utf8len   = libc_utf16len(u16_string);
 * >> assert(utf8len <= utf16chars);
 */
#define libc_utf8len16(utf8,utf8chars) XBLOCK({ mbstate_t state = MBSTATE_INIT; XRETURN libc_utf8to16(utf8,utf8chars,NULL,0,&state); })
#define libc_utf8len32(utf8,utf8chars) XBLOCK({ mbstate_t state = MBSTATE_INIT; XRETURN libc_utf8to32(utf8,utf8chars,NULL,0,&state); })


DECL_END
#endif /* !__KERNEL__ */

#endif /* !GUARD_LIBS_LIBC_UNICODE_H */
