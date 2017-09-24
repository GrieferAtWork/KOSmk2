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
#ifndef GUARD_LIBS_LIBC_UNICODE_C
#define GUARD_LIBS_LIBC_UNICODE_C 1

#include "libc.h"
#include "unicode.h"
#include "malloc.h"
#include "errno.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <uchar.h>
#include <stdbool.h>

DECL_BEGIN

PRIVATE u8 const uni_bytemarks[7] = {0x00,0x00,0xC0,0xE0,0xF0,0xF8,0xFC};
PRIVATE u8 const utf8_trailing_bytes[256] = {
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5};
PRIVATE u32 const utf8_offsets[6] = {0x00000000,0x00003080,0x000E2080,
                                     0x03C82080,0xFA082080,0x82082080};
#define UNI_HALF_BASE            0x0010000
#define UNI_HALF_MASK            0x3FF
#define UNI_HALF_SHIFT           10
#define UNI_MAX_BMP              0x0000FFFF
#define UNI_MAX_LEGAL_UTF32      0x0010FFFF
#define UNI_MAX_UTF16            0x0010FFFF
#define UNI_MAX_UTF32            0x7FFFFFFF
#define UNI_SURROGATE_HIGH_END   0xDBFF
#define UNI_SURROGATE_HIGH_BEGIN 0xD800
#define UNI_SURROGATE_LOW_END    0xDFFF
#define UNI_SURROGATE_LOW_BEGIN  0xDC00

LOCAL bool LIBCCALL
libc_utf8_check(char const *__restrict utf8, size_t utf8chars) {
 u8 ch; char const *end = utf8+utf8chars;
 switch (utf8chars) {
 case 4: if ((ch = (u8)*--end) < 0x80 || ch > 0xBF) return false;
 case 3: if ((ch = (u8)*--end) < 0x80 || ch > 0xBF) return false;
 case 2: if ((ch = (u8)*--end) < 0x80 || ch > 0xBF) return false;
  switch ((u8)*utf8) {
  case 0xE0: if (ch < 0xA0) return false; break;
  case 0xED: if (ch > 0x9F) return false; break;
  case 0xF0: if (ch < 0x90) return false; break;
  case 0xF4: if (ch > 0x8F) return false; break;
  default:   if (ch < 0x80) return false; break;
  }
 case 1:
  if ((u8)*utf8 >= 0x80 && (u8)*utf8 < 0xC2)
      return false;
  break;
 default:
  return false;
 }
 if ((u8)*utf8 > 0xF4)
     return false;
 return true;
}


#define MBSTATE_ISEMPTY(x)    ((x)->__count == 0)
#define MBSTATE_GETMISSING(x) ((x)->__count & 0xff)
#define MBSTATE_GETSRCSIZE(x) (((x)->__count >> 16) & 0xff)
#define MBSTATE_SETCOUNT(x,missing,srcsize_minus_one) \
  ((x)->__count = (u32)(missing) | ((u32)(srcsize_minus_one) << 16))

#define GOTO_DONE() do{ if ((mode&(UNICODE_F_ALWAYSZEROTERM|UNICODE_F_NOZEROTERM)) == UNICODE_F_ALWAYSZEROTERM) goto done2; else goto done; }while(0)

INTERN size_t LIBCCALL
libc_utf8to32(char const *__restrict utf8, size_t utf8len,
              char32_t *__restrict utf32, size_t buflen32,
              mbstate_t *__restrict state, u32 mode) {
 char const *iter,*end; u8 src_size; u32 ch;
 u32 *dst = (u32 *)utf32,*dst_end = (u32 *)utf32+buflen32;
 if (mode&UNICODE_F_UPDATESRC) {
  iter = *(char const **)utf8;
  end = iter+(*(size_t *)utf8len);
 } else {
  end = (iter = utf8)+utf8len;
 }
 if (!MBSTATE_ISEMPTY(state)) {
  /* Non-empty input state. */
  if ((size_t)(end-iter) < MBSTATE_GETMISSING(state)) GOTO_DONE(); /* We need more! */
  src_size = MBSTATE_GETSRCSIZE(state);
  /* Read the remaining, missing characters. */
  ch = state->__value.__wch;
  switch (MBSTATE_GETMISSING(state)) {
  case 5: ch += (u8)*iter++; ch <<= 6;
  case 4: ch += (u8)*iter++; ch <<= 6;
  case 3: ch += (u8)*iter++; ch <<= 6;
  case 2: ch += (u8)*iter++; ch <<= 6;
  case 1: ch += (u8)*iter++;
  default: break;
  }
  state->__count = 0;
  goto got_char;
 }
 while (iter != end) {
  size_t avail = (size_t)(end-iter);
  if (mode&UNICODE_F_STOPONNUL && !*iter) break;
  src_size = utf8_trailing_bytes[(u8)*iter];
  ch = 0;
  if unlikely((size_t)src_size+1 > avail) {
   /* Not enough available input characters. - Use the mbstate. */
   /* Store the amount of missing bytes and total bytes. */
   MBSTATE_SETCOUNT(state,(src_size+1)-(u8)avail,src_size);
   assert(avail <= 5);
   assert(avail != 0);
   switch (avail) {
   case 5: ch += (u8)*iter++; ch <<= 6;
   case 4: ch += (u8)*iter++; ch <<= 6;
   case 3: ch += (u8)*iter++; ch <<= 6;
   case 2: ch += (u8)*iter++; ch <<= 6;
   case 1: ch += (u8)*iter++; break;
   }
   state->__value.__wch = ch;
   GOTO_DONE();
  }
  if unlikely(!libc_utf8_check(iter,src_size+1))
     goto err;
  switch (src_size) {
  case 5: ch += (u8)*iter++; ch <<= 6;
  case 4: ch += (u8)*iter++; ch <<= 6;
  case 3: ch += (u8)*iter++; ch <<= 6;
  case 2: ch += (u8)*iter++; ch <<= 6;
  case 1: ch += (u8)*iter++; ch <<= 6;
  case 0: ch += (u8)*iter++; break;
  }
got_char:
  ch -= utf8_offsets[src_size];
  if unlikely(ch > UNI_MAX_LEGAL_UTF32)
     goto err;
  if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
              ch <= UNI_SURROGATE_LOW_END)
     goto err;
  if (dst < dst_end) *dst = ch;
  ++dst;
  if (mode&UNICODE_F_DOSINGLE) break;
 }
 if (!(mode&UNICODE_F_NOZEROTERM)) {
done2:
  if (dst < dst_end) *dst = 0;
  ++dst;
 }
done:
 if (mode&UNICODE_F_UPDATESRC) {
  *(char const **)utf8 = iter;
  *(size_t *)utf8len = (size_t)(end-iter);
 }
 return (size_t)(dst-utf32);
err:
 if (mode&UNICODE_F_SETERRNO)
     SET_ERRNO(EILSEQ);
 return UNICODE_ERROR;
}



INTERN size_t LIBCCALL
libc_utf8to16(char const *__restrict utf8, size_t utf8len,
              char16_t *__restrict utf16, size_t buflen16,
              mbstate_t *__restrict state, u32 mode) {
 char const *iter,*end; u8 src_size; u32 ch;
 u16 *dst = (u16 *)utf16,*dst_end = (u16 *)utf16+buflen16;
 if (mode&UNICODE_F_UPDATESRC) {
  iter = *(char const **)utf8;
  end  = iter+(*(size_t *)utf8len);
 } else {
  end = (iter = utf8)+utf8len;
 }
 if (!MBSTATE_ISEMPTY(state)) {
  /* Non-empty input state. */
  if ((end-iter) < MBSTATE_GETMISSING(state)) GOTO_DONE(); /* We need more! */
  src_size = MBSTATE_GETSRCSIZE(state);
  /* Read the remaining, missing characters. */
  ch = state->__value.__wch;
  switch (MBSTATE_GETMISSING(state)) {
  case 5: ch += (u8)*iter++; ch <<= 6;
  case 4: ch += (u8)*iter++; ch <<= 6;
  case 3: ch += (u8)*iter++; ch <<= 6;
  case 2: ch += (u8)*iter++; ch <<= 6;
  case 1: ch += (u8)*iter++;
  default: break;
  }
  state->__count = 0;
  goto got_char;
 }
 while (iter != end) {
  size_t avail = (size_t)(end-iter);
  if (mode&UNICODE_F_STOPONNUL && !*iter) break;
  src_size = utf8_trailing_bytes[(u8)*iter];
  ch = 0;
  if unlikely((size_t)src_size+1 > avail) {
   /* Not enough available input characters. - Use the mbstate. */
   /* Store the amount of missing bytes and total bytes. */
   MBSTATE_SETCOUNT(state,(src_size+1)-(u8)avail,src_size);
   assert(avail <= 5);
   assert(avail != 0);
   switch (avail) {
   case 5: ch += (u8)*iter++; ch <<= 6;
   case 4: ch += (u8)*iter++; ch <<= 6;
   case 3: ch += (u8)*iter++; ch <<= 6;
   case 2: ch += (u8)*iter++; ch <<= 6;
   case 1: ch += (u8)*iter++; break;
   }
   state->__value.__wch = ch;
   GOTO_DONE();
  }
  if unlikely(!libc_utf8_check(iter,src_size+1))
     goto err;
  switch (src_size) {
  case 5: ch += (u8)*iter++; ch <<= 6;
  case 4: ch += (u8)*iter++; ch <<= 6;
  case 3: ch += (u8)*iter++; ch <<= 6;
  case 2: ch += (u8)*iter++; ch <<= 6;
  case 1: ch += (u8)*iter++; ch <<= 6;
  case 0: ch += (u8)*iter++; break;
  }
got_char:
  ch -= utf8_offsets[src_size];
  if likely(ch <= UNI_MAX_BMP) {
   if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
               ch <= UNI_SURROGATE_LOW_END)
      goto err;
   if (dst < dst_end) *dst = (u16)ch;
   ++dst;
  } else if unlikely(ch > UNI_MAX_UTF16) {
   goto err;
  } else { /* Range: 0xFFFF - 0x10FFFF. */
   ch -= UNI_HALF_BASE;
   if (dst < dst_end) *dst = (char16_t)(u16)((ch >> UNI_HALF_SHIFT)+UNI_SURROGATE_HIGH_BEGIN);
   ++dst;
   if (dst < dst_end) *dst = (char16_t)(u16)((ch & UNI_HALF_MASK)+UNI_SURROGATE_LOW_BEGIN);
   else if (mode&UNICODE_F_UTF16HALF) {
    /* Use the shift state. */
    ch = ((ch & UNI_HALF_MASK)+UNI_HALF_BASE)+utf8_offsets[src_size];
    MBSTATE_SETCOUNT(state,0,src_size);
    state->__value.__wch = ch;
    if (!(mode&UNICODE_F_NOZEROTERM)) {
     if (dst < dst_end) *dst = 0;
     ++dst;
    }
    if (mode&UNICODE_F_UPDATESRC) {
     *(char const **)utf8 = iter;
     *(size_t *)utf8len = (size_t)(end-iter);
    }
    return UNICODE_UTF16HALF;
   }
   ++dst;
  }
  if (mode&UNICODE_F_DOSINGLE) break;
 }
 if (!(mode&UNICODE_F_NOZEROTERM)) {
done2:
  if (dst < dst_end) *dst = 0;
  ++dst;
 }
done:
 if (mode&UNICODE_F_UPDATESRC) {
  *(char const **)utf8 = iter;
  *(size_t *)utf8len = (size_t)(end-iter);
 }
 return (size_t)(dst-utf16);
err:
 if (mode&UNICODE_F_SETERRNO)
     SET_ERRNO(EILSEQ);
 return UNICODE_ERROR;
}



INTERN size_t LIBCCALL
libc_utf32to8(char32_t const *__restrict utf32, size_t utf32len,
              char *__restrict utf8, size_t buflen8,
              mbstate_t *__restrict UNUSED(state), u32 mode) {
 char *dst = utf8,*dst_end = utf8+buflen8;
 u32 const *iter,*end;
 u32 ch; size_t dst_size;
 if (mode&UNICODE_F_UPDATESRC) {
  iter = *(u32 const **)utf32;
  end = iter+(*(size_t *)utf32len);
 } else {
  iter = (u32 const *)utf32;
  end = iter+utf32len;
 }
 while (iter != end) {
  ch = *iter++;
  if (!ch && mode&UNICODE_F_STOPONNUL) break;
  if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
              ch <= UNI_SURROGATE_LOW_END)
     goto err;
  if likely(ch < (u32)0x80) dst_size = 1;
  else if (ch < (u32)0x800) dst_size = 2;
  else if (ch < (u32)0x10000) dst_size = 3;
  else if likely(ch <= UNI_MAX_LEGAL_UTF32) dst_size = 4;
  else goto err;
  switch (dst_size) {
  case 4: if (dst < dst_end) { *dst++ = (char)((ch|0x80)&0xBF); } ++dst; ch >>= 6;
  case 3: if (dst < dst_end) { *dst++ = (char)((ch|0x80)&0xBF); } ++dst; ch >>= 6;
  case 2: if (dst < dst_end) { *dst++ = (char)((ch|0x80)&0xBF); } ++dst; ch >>= 6;
  case 1: if (dst < dst_end) { *dst   = (char) (ch|uni_bytemarks[dst_size]); } ++dst;
  }
  if (mode&UNICODE_F_DOSINGLE) break;
 }
 if (!(mode&UNICODE_F_NOZEROTERM)) {
/*done2:*/
  if (dst < dst_end) *dst = 0;
  ++dst;
 }
/*done:*/
 if (mode&UNICODE_F_UPDATESRC) {
  *(u32 const **)utf32 = iter;
  *(size_t *)utf32len = (size_t)(end-iter);
 }
 return (size_t)(dst-utf8);
err:
 if (mode&UNICODE_F_SETERRNO)
     SET_ERRNO(EILSEQ);
 return UNICODE_ERROR;
}

INTERN size_t LIBCCALL
libc_utf16to8(char16_t const *__restrict utf16, size_t utf16len,
              char *__restrict utf8, size_t buflen8,
              mbstate_t *__restrict state, u32 mode) {
 char *temp,*dst = utf8,*dst_end = utf8+buflen8;
 u16 const *iter,*end;
 u32 ch; size_t dst_size;
 if (mode&UNICODE_F_UPDATESRC) {
  iter = *(u16 const **)utf16;
  end = iter+(*(size_t *)utf16len);
 } else {
  iter = (u16 const *)utf16;
  end  = iter+utf16len;
 }
 if (state->__count) {
  /* Load an old mb-state. */
  if unlikely(iter == end) GOTO_DONE();
  state->__count = 0;
  ch = (u32)state->__value.__wch;
  goto second_char;
 }
 while (iter != end) {
  ch = (u32)(u16)*iter++;
  if (!ch && mode&UNICODE_F_STOPONNUL) break;
  /* Convert surrogate pair to Utf32 */
  if unlikely(ch < UNI_SURROGATE_HIGH_BEGIN ||
              ch > UNI_SURROGATE_HIGH_END)
     goto err;
  if likely(iter < end) {
   u16 ch2;
second_char:
   ch2 = (u16)*iter;
   if unlikely(ch2 < UNI_SURROGATE_LOW_BEGIN ||
               ch2 > UNI_SURROGATE_LOW_END)
      goto err;
   ch = ((u32)(ch-UNI_SURROGATE_HIGH_BEGIN) << UNI_HALF_SHIFT)+
         (u32)(ch2-UNI_SURROGATE_LOW_BEGIN)+UNI_HALF_BASE;
   ++iter;
  } else {
   /* Partial input string (store last character in mb-state). */
   state->__count = 2;
   state->__value.__wch = ch;
   GOTO_DONE();
  }

  if likely(ch < (u32)0x80) dst_size = 1;
  else if (ch < (u32)0x800) dst_size = 2;
  else if (ch < (u32)0x10000) dst_size = 3;
  else if likely(ch < (u32)0x110000) dst_size = 4;
  else goto err;
  temp = (dst += dst_size);
  switch (dst_size) {
   case 4: if (temp < dst_end) { temp[-1] = (char)(u8)((ch|0x80)&0xBF); } --temp,ch >>= 6;
   case 3: if (temp < dst_end) { temp[-1] = (char)(u8)((ch|0x80)&0xBF); } --temp,ch >>= 6;
   case 2: if (temp < dst_end) { temp[-1] = (char)(u8)((ch|0x80)&0xBF); } --temp,ch >>= 6;
   case 1: if (temp < dst_end) { temp[-1] = (char)(u8)(ch|uni_bytemarks[dst_size]); }
  }
  if (mode&UNICODE_F_DOSINGLE) break;
 }
 if (!(mode&UNICODE_F_NOZEROTERM)) {
done2:
  if (dst < dst_end) *dst = 0;
  ++dst;
 }
done:
 if (mode&UNICODE_F_UPDATESRC) {
  *(u16 const **)utf16 = iter;
  *(size_t *)utf16len = (size_t)(end-iter);
 }
 return (size_t)(dst-utf8);
err:
 if (mode&UNICODE_F_SETERRNO)
     SET_ERRNO(EILSEQ);
 return UNICODE_ERROR;
}



INTERN char16_t *LIBCCALL libc_utf8to16m(char const *__restrict utf8, size_t utf8len) {
 size_t reqlen,buflen = (utf8len+(utf8len/3)+1);
 char16_t *result = (char16_t *)libc_malloc(buflen*sizeof(char16_t));
 if unlikely(!result) return NULL;
again:
 { mbstate_t state = MBSTATE_INIT;
   reqlen = libc_utf8to16(utf8,utf8len,result,buflen,&state,0);
   if unlikely(reqlen == UNICODE_ERROR) goto err;
   if (reqlen != buflen) {
    char16_t *new_result = (char16_t *)libc_realloc(result,reqlen*sizeof(char16_t));
    if unlikely(!new_result) {err: libc_free(result); return NULL; }
    result = new_result;
    if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
   }
 }
 return result;
}
INTERN char32_t *LIBCCALL libc_utf8to32m(char const *__restrict utf8, size_t utf8len) {
 size_t reqlen,buflen = (utf8len+(utf8len/3)+1);
 char32_t *result = (char32_t *)libc_malloc(buflen*sizeof(char32_t));
 if unlikely(!result) return NULL;
again:
 { mbstate_t state = MBSTATE_INIT;
   reqlen = libc_utf8to32(utf8,utf8len,result,buflen,&state,0);
   if unlikely(reqlen == UNICODE_ERROR) goto err;
   if (reqlen != buflen) {
    char32_t *new_result = (char32_t *)libc_realloc(result,reqlen*sizeof(char32_t));
    if unlikely(!new_result) {err: libc_free(result); return NULL; }
    result = new_result;
    if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
   }
 }
 return result;
}
INTERN char *LIBCCALL libc_utf16to8m(char16_t const *__restrict utf16, size_t utf16len) {
 size_t reqlen,buflen = (utf16len+1);
 char *result = (char *)libc_malloc(buflen*sizeof(char));
 if unlikely(!result) return NULL;
again:
 { mbstate_t state = MBSTATE_INIT;
   reqlen = libc_utf16to8(utf16,utf16len,result,buflen,&state,0);
   if unlikely(reqlen == UNICODE_ERROR) goto err;
   if (reqlen != buflen) {
    char *new_result = (char *)libc_realloc(result,reqlen*sizeof(char));
    if unlikely(!new_result) {err: libc_free(result); return NULL; }
    result = new_result;
    if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
   }
 }
 return result;
}
INTERN char *LIBCCALL libc_utf32to8m(char32_t const *__restrict utf32, size_t utf32len) {
 size_t reqlen,buflen = (utf32len+1);
 char *result = (char *)libc_malloc(buflen*sizeof(char));
 if unlikely(!result) return NULL;
again:
 { mbstate_t state = MBSTATE_INIT;
   reqlen = libc_utf32to8(utf32,utf32len,result,buflen,&state,0);
   if unlikely(reqlen == UNICODE_ERROR) goto err;
   if (reqlen != buflen) {
    char *new_result = (char *)libc_realloc(result,reqlen*sizeof(char));
    if unlikely(!new_result) {err: libc_free(result); return NULL; }
    result = new_result;
    if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
   }
 }
 return result;
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_UNICODE_C */
