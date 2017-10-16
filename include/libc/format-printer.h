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
#ifndef _LIBC_FORMAT_PRINTER_H
#define _LIBC_FORMAT_PRINTER_H 1

#include "__stdinc.h"
#include <assert.h>
#include <features.h>
#include <bits/types.h>
#include <hybrid/typecore.h>
#include <hybrid/string.h>
#include <hybrid/malloc.h>

#ifdef __INTELLISENSE__
#ifndef _FORMAT_PRINTER_H
#   include <format-printer.h>
#endif /* !_FORMAT_PRINTER_H */
#elif __LIBC_FORMAT_PRINTER_INCLUDE_MAGIC != 0xdeadbeef
#   error "Don't include this header directly. - Include <format-printer.h> instead"
#endif

#if !defined(__NO_ATTR_WEAK) && \
    !defined(__ATTR_WEAK_IS_SELECTANY)
#define __FORMAT_PRINTER_IMPL __INTERN __ATTR_WEAK
#else /* !__NO_ATTR_WEAK */
#define __FORMAT_PRINTER_IMPL __PRIVATE
#endif /* __NO_ATTR_WEAK */

#ifdef _MSC_VER
#pragma warning(disable: 4505) /* Unused local function was removed. */
#endif

__SYSDECL_BEGIN

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#endif

#define __PRINTF_FLAG_NONE     0x0000
#define __PRINTF_FLAG_PREFIX   0x0001 /*< '%#'. */
#define __PRINTF_FLAG_LJUST    0x0002 /*< '%-'. */
#define __PRINTF_FLAG_SIGN     0x0004 /*< '%+'. */
#define __PRINTF_FLAG_SPACE    0x0008 /*< '% '. */
#define __PRINTF_FLAG_PADZERO  0x0010 /*< '%0'. */
#define __PRINTF_FLAG_HASWIDTH 0x0020 /*< '%123'. */
#define __PRINTF_FLAG_HASPREC  0x0040 /*< '%.123'. */
#define __PRINTF_FLAG_UPPER    0x0080 /*< Print upper-case hex-characters. */
#define __PRINTF_FLAG_SIGNED   0x0100
#define __PRINTF_FLAG_FIXBUF   0x0200
#define __PRINTF_FLAG_QUOTE    0x0400
#define __PRINTF_FLAG_CHQUOTE  0x0800 /*< Quote as character. */

#define __PRINTF_VA_SIZE  __SIZEOF_INT__

#if __PRINTF_VA_SIZE == 8
#define __PRINTF_LEN_I64 0
#define __PRINTF_LEN_I32 __PRINTF_LEN_I64
#define __PRINTF_LEN_I16 __PRINTF_LEN_I64
#define __PRINTF_LEN_I8  __PRINTF_LEN_I64
#define __PRINTF_FINT_LOAD(arg,length,flags,args) \
  { arg.__u_64 = __builtin_va_arg(args,__UINT64_TYPE__); }
#elif __PRINTF_VA_SIZE == 4
#define __PRINTF_LEN_I32 0
#define __PRINTF_LEN_I64 1
#define __PRINTF_LEN_I16 __PRINTF_LEN_I32
#define __PRINTF_LEN_I8  __PRINTF_LEN_I32
#define __PRINTF_FINT_LOAD(arg,length,flags,args) \
  { arg.__u_64 = 0; \
    if ((length) == __PRINTF_LEN_I64) arg.__u_64 = __builtin_va_arg(args,__UINT64_TYPE__); \
    else { arg.__u_32 = __builtin_va_arg(args,__UINT32_TYPE__); \
    if (flags&__PRINTF_FLAG_SIGNED) arg.__s_64 = (__INT64_TYPE__)arg.__s_32; } \
  }
#elif __PRINTF_VA_SIZE == 2
#define __PRINTF_LEN_I16 0
#define __PRINTF_LEN_I32 1
#define __PRINTF_LEN_I64 2
#define __PRINTF_LEN_I8  __PRINTF_LEN_I16
#define __PRINTF_FINT_LOAD(arg,length,flags,args) \
  {  arg.__u_64 = 0; \
         if ((length) == __PRINTF_LEN_I64) arg.__u_64 = __builtin_va_arg(args,__UINT64_TYPE__); \
    else if ((length) == __PRINTF_LEN_I32) { arg.__u_32 = __builtin_va_arg(args,__UINT32_TYPE__); if (flags&__PRINTF_FLAG_SIGNED) arg.__s_64 = (__INT64_TYPE__)arg.__s_32; } \
    else { arg.__u_16 = __builtin_va_arg(args,__UINT16_TYPE__); if (flags&__PRINTF_FLAG_SIGNED) arg.__s_64 = (__INT64_TYPE__)arg.__s_16; } \
  }
#elif __PRINTF_VA_SIZE == 1
#define __PRINTF_LEN_I8  0
#define __PRINTF_LEN_I16 1
#define __PRINTF_LEN_I32 2
#define __PRINTF_LEN_I64 3
#define __PRINTF_FINT_LOAD(arg,length,flags,args) \
  { arg.__u_64 = 0; \
         if ((length) == __PRINTF_LEN_I64) arg.__u_64 = __builtin_va_arg(args,__UINT64_TYPE__); \
    else if ((length) == __PRINTF_LEN_I32) { arg.__u_32 = __builtin_va_arg(args,__UINT32_TYPE__); if (flags&__PRINTF_FLAG_SIGNED) arg.__s_64 = (__INT64_TYPE__)arg.__s_32; } \
    else if ((length) == __PRINTF_LEN_I16) { arg.__u_16 = __builtin_va_arg(args,__UINT16_TYPE__); if (flags&__PRINTF_FLAG_SIGNED) arg.__s_64 = (__INT64_TYPE__)arg.__s_16; } \
    else { arg.__u_8 = __builtin_va_arg(args,__UINT8_TYPE__); if (flags&__PRINTF_FLAG_SIGNED) arg.__s_64 = (__INT64_TYPE__)arg.__s_8; } \
  }
#else
#   error "Unsupported printf() va-size"
#endif
#define __PRINTF_LEN_PTR __PP_CAT2(__PRINTF_LEN_I,__PP_MUL8(__SIZEOF_POINTER__))
#define __PRINTF_LEN_hh  __PP_CAT2(__PRINTF_LEN_I,__PP_MUL8(__SIZEOF_CHAR__))
#define __PRINTF_LEN_h   __PP_CAT2(__PRINTF_LEN_I,__PP_MUL8(__SIZEOF_SHORT__))
#define __PRINTF_LEN_l   __PP_CAT2(__PRINTF_LEN_I,__PP_MUL8(__SIZEOF_LONG__))
#define __PRINTF_LEN_ll  __PP_CAT2(__PRINTF_LEN_I,__PP_MUL8(__SIZEOF_LONG_LONG__))
#define __PRINTF_LEN_j   __PRINTF_LEN_I64 /* intmax_t */


#ifdef __ATTR_WEAK_IS_SELECTANY
__INTERN __ATTR_WEAK
#else
__FORMAT_PRINTER_IMPL
#endif
char const __printf_decimals[2][17] = {
 {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','x'},
 {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','X'},
};

#define __print(s,n) \
do{ if ((__temp = (*__printer)(s,n,__closure)) < 0) return __temp; \
    __result += __temp; \
}while(0)
#define __printf(...) \
do{ if ((__temp = format_printf(__printer,__closure,__VA_ARGS__)) < 0) return __temp; \
    __result += __temp; \
}while(0)
#define __quote(s,n,flags) \
do{ if ((__temp = format_quote(__printer,__closure,s,n,flags)) < 0) return __temp; \
    __result += __temp; \
}while(0)

__FORMAT_PRINTER_IMPL
__ssize_t (__LIBCCALL format_vprintf)(pformatprinter __printer, void *__closure,
                                      char const *__restrict __format, __VA_LIST __args) {
 typedef union {
  __INT8_TYPE__  __s_8; __INT16_TYPE__  __s_16; __INT32_TYPE__  __s_32; __INT64_TYPE__  __s_64;
  __UINT8_TYPE__ __u_8; __UINT16_TYPE__ __u_16; __UINT32_TYPE__ __u_32; __UINT64_TYPE__ __u_64;
 } __fint_t;
 __ssize_t __result = 0,__temp; char __ch;
 char const *__flush_start = __format;
 for (;;) {
__next_normal:
  __ch = *__format++;
  if __unlikely(!__ch) break;
  if (__ch == '%') {
   int __length; __uint16_t __flags;
   __size_t __width,__precision,__print_width;
   if (__format-1 != __flush_start)
       __print(__flush_start,(__size_t)((__format-1)-__flush_start));
   __flush_start = __format;
   __length      = 0;
   __flags       = 0;
   __width       = 0;
   __precision   = 0;
   __print_width = (__size_t)__result;
__nextmod:
   __ch     = *__format++;
__nextmodc:
   switch (__ch) {
   case '\0': goto end;
   case '-': __flags |= __PRINTF_FLAG_LJUST;   goto __nextmod;
   case '+': __flags |= __PRINTF_FLAG_SIGN;    goto __nextmod;
   case ' ': __flags |= __PRINTF_FLAG_SPACE;   goto __nextmod;
   case '#': __flags |= __PRINTF_FLAG_PREFIX;  goto __nextmod;
   case '0': __flags |= __PRINTF_FLAG_PADZERO; goto __nextmod;
   case 'Q': __flags |= __PRINTF_FLAG_QUOTE;   goto __nextmod;
   case '?':
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
    __width = __builtin_va_arg(__args,__size_t);
    goto __have_width;
#endif
   case '*':
    __width = (__size_t)__builtin_va_arg(__args,unsigned int); /* Technically int, but come on... */
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
__have_width:
#endif
    __flags |= __PRINTF_FLAG_HASWIDTH;
    goto __nextmod;

    /* Precision */
   case ':': __flags |= __PRINTF_FLAG_FIXBUF;
   case '.': __ch = *__format++;
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
    if (__ch == '?') { case '$': __width = __builtin_va_arg(__args,__size_t); goto __have_precision; }
#else
    if (__ch == '?') { case '$': __flags |= __PRINTF_FLAG_FIXBUF; goto __use_precision; }
#endif
    else if (__ch == '*') {
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__use_precision:
#endif
     __precision = (__size_t)__builtin_va_arg(__args,unsigned int); /* Technically int, but come on... */
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
__have_precision:
#endif
    } else if (__ch >= '0' && __ch <= '9') {
     for (;;) {
      __precision *= 10;
      __precision += (__size_t)(__ch-'0');
      __ch = *__format++;
      if (__ch < '0' || __ch > '9') break;
     }
     --__format;
    } else goto __broken_format;
    __flags |= __PRINTF_FLAG_HASPREC;
    goto __nextmod;

    /* Length modifiers */
   case 'h':
    if (*__format != 'h') __length = __PRINTF_LEN_h;
    else { ++__format; __length = __PRINTF_LEN_hh; }
    goto __nextmod;

   case 'l':
    if (*__format != 'l') __length = __PRINTF_LEN_l;
    else { ++__format; __length = __PRINTF_LEN_ll; }
    goto __nextmod;

   case 'j':
    __length = __PRINTF_LEN_j;
    goto __nextmod;

   case 'z': case 't': case 'L':
    __length = __ch;
    goto __nextmod;


   case 'I':
    __ch = *__format++;
    /* */if (__ch == '8') __length = __PRINTF_LEN_I8;
    else if (__ch == '1' && *__format == '6') ++__format,__length = __PRINTF_LEN_I16;
    else if (__ch == '3' && *__format == '2') ++__format,__length = __PRINTF_LEN_I32;
    else if (__ch == '6' && *__format == '4') ++__format,__length = __PRINTF_LEN_I64;
    else { __length = __PRINTF_LEN_PTR; goto __nextmodc; }
    goto __nextmod;

   {
    int __numsys; /* Integer formating. */
    char __buf[32],*__iter;
    char const *dec;
    __size_t bufsize;
    __fint_t __arg;
    if (0) { case 'B': __flags |= __PRINTF_FLAG_UPPER; case 'b': __numsys = 2; }
    if (0) { case 'o': __numsys = 8; }
    if (0) { case 'u': __numsys = 10; if __unlikely(__length == 't') __flags |= __PRINTF_FLAG_SIGNED; }
    if (0) { case 'd': case 'i': __numsys = 10; if __likely(__length != 'z') __flags |= __PRINTF_FLAG_SIGNED; }
    if (0) { case 'p': if (!(__flags&__PRINTF_FLAG_HASPREC)) { __precision = sizeof(void *)*2; __flags |= __PRINTF_FLAG_HASPREC; }
#if __SIZEOF_POINTER__ > __PRINTF_VA_SIZE
                       if (!__length) __length = __PRINTF_LEN_PTR;
#endif
             case 'X': __flags |= __PRINTF_FLAG_UPPER;
             case 'x': __numsys = 16; if __unlikely(__length == 't') __flags |= __PRINTF_FLAG_SIGNED; }
    __PRINTF_FINT_LOAD(__arg,__length,__flags,__args);
    __iter = __buf;
    if (__flags&__PRINTF_FLAG_SIGNED) {
     if (__arg.__s_64 < 0) {
      *__iter++ = '-';
      __arg.__s_64 = -__arg.__s_64;
     } else if (__flags&__PRINTF_FLAG_SPACE) {
      *__iter++ = ' ';
     }
    }
    dec = __printf_decimals[!!(__flags&__PRINTF_FLAG_UPPER)];
    if (__flags&__PRINTF_FLAG_PREFIX && __numsys != 10) {
     *__iter++ = '0';
          if (__numsys == 16) *__iter++ = dec[16]; /* X/x */
     else if (__numsys == 2)  *__iter++ = dec[11]; /* B/b */
    }
    if (__iter != __buf) __print(__buf,(__size_t)(__iter-__buf));
    __iter = __COMPILER_ENDOF(__buf);
    __assertf(__numsys <= 16,"%d",__numsys);
    do *--__iter = dec[__arg.__u_64 % __numsys];
    while ((__arg.__u_64 /= __numsys) != 0);
    for (;;) {
     bufsize = (__size_t)(__COMPILER_ENDOF(__buf)-__iter);
     if ((__flags&__PRINTF_FLAG_HASPREC) &&
          __precision > bufsize) {
      __size_t __precbufsize = __COMPILER_LENOF(__buf)-bufsize;
      __precision -= bufsize;
      if (__precbufsize > __precision)
          __precbufsize = __precision;
      assert(__precbufsize);
      bufsize += __precbufsize;
      __iter    -= __precbufsize;
      __hybrid_memset(__iter,'0',__precbufsize);
      assert(__precbufsize <= __precision);
      __precision -= __precbufsize;
     }
     __print(__iter,bufsize);
     if (__precision <= bufsize) break;
     assert(__flags&__PRINTF_FLAG_HASPREC);
     __iter = __COMPILER_ENDOF(__buf);
    }
   } break;

   {
    char *__s;
   {
    char __given_char[1];
   case 'c':
    __given_char[0] = (char)__builtin_va_arg(__args,int);
    if (__flags&__PRINTF_FLAG_QUOTE) {
     __flags    |= __PRINTF_FLAG_CHQUOTE;
     __s         = __given_char;
     __precision = 1;
     goto __quote_string;
    } else {
     __print(__given_char,1);
    }
   } break;

   case 'q': __flags |= __PRINTF_FLAG_QUOTE;
   case '__s':
    __s = __builtin_va_arg(__args,char *);
    if (!__s) __s = "(null)";
    if (!(__flags&__PRINTF_FLAG_HASPREC)) __precision = (__size_t)-1;
    if (!(__flags&__PRINTF_FLAG_FIXBUF))
          __precision = __hybrid_strnlen(__s,__precision);
    if (__flags&__PRINTF_FLAG_QUOTE) {
__quote_string:
     /* Quote string. */
     if (__flags&__PRINTF_FLAG_CHQUOTE) {
      if (__flags&__PRINTF_FLAG_PREFIX)
           __flags &= ~(__PRINTF_FLAG_CHQUOTE);
      else __flags |=   __PRINTF_FLAG_PREFIX;
      __print("\'",1);
     }
#if __PRINTF_FLAG_PREFIX == FORMAT_QUOTE_FLAG_PRINTRAW
     __quote(__s,__precision,(__UINT32_TYPE__)(__flags&__PRINTF_FLAG_PREFIX));
#else
     __quote(__s,__precision,(__UINT32_TYPE__)(__flags&__PRINTF_FLAG_PREFIX
                                             ? FORMAT_QUOTE_FLAG_PRINTRAW
                                             : FORMAT_QUOTE_FLAG_NONE));
#endif
     if (__flags&__PRINTF_FLAG_CHQUOTE) __print("\'",1);
    } else {
     __temp = (*__printer)(__s,__precision,__closure);
     if (__temp < 0) return __temp;
     __result += __temp;
    }
   } break;

#if 0
   {
   case 'f':
   case 'g':
    /* TODO: Floating point printing. */
   } break;
#endif

   case '%':
    __flush_start = __format-1;
    goto __next_normal;
   default:
    if (__ch >= '0' && __ch <= '9') {
     for (;;) {
      __width *= 10;
      __width += (__size_t)(__ch-'0');
      __ch = *__format++;
      if (__ch < '0' || __ch > '9') break;
     }
     __flags |= __PRINTF_FLAG_HASWIDTH;
     goto __nextmodc;
    }
__broken_format:
    /*__printf("<INVALID_FORMAT_OPTION:'%c'>",__ch);*/
    __format = __flush_start;
    goto __next_normal;
   }
/*__done_fmt:*/
   if (__flags&__PRINTF_FLAG_HASWIDTH) {
    __print_width = (__size_t)__result-__print_width;
    if (__print_width < __width) {
     __size_t partsize;
     char __buf[32];
     __print_width = __width-__print_width;
     partsize    = __print_width;
     if (partsize > __COMPILER_LENOF(__buf))
         partsize = __COMPILER_LENOF(__buf);
     __hybrid_memset(__buf,' ',partsize);
     while (__print_width > __COMPILER_LENOF(__buf)) {
      __print(__buf,__COMPILER_LENOF(__buf));
      __print_width -= __COMPILER_LENOF(__buf);
     }
     __print(__buf,partsize);
    }
   }
   __flush_start = __format;
  }
 }
end:
 --__format;
 assert(!*__format);
 if (__flush_start != __format)
     __print(__flush_start,(__size_t)(__format-__flush_start));
 return __result;
}

__FORMAT_PRINTER_IMPL
__ssize_t (__ATTR_CDECL format_printf)(pformatprinter __printer, void *__closure,
                                       char const *__restrict __format, ...) {
 __VA_LIST __args; __ssize_t __result;
 __builtin_va_start(__args,__format);
 __result = format_vprintf(__printer,__closure,__format,__args);
 __builtin_va_end(__args);
 return __result;
}

#define __tooct(c) ('0'+(c))
__FORMAT_PRINTER_IMPL __NONNULL((1,3)) __ssize_t
(__LIBCCALL format_quote)(pformatprinter __printer, void *__closure,
                          char const *__restrict __text, __size_t __textlen,
                          __UINT32_TYPE__ __flags) {
 char __encoded_text[4];
 __size_t __encoded_text_size;
 ssize_t __result = 0,__temp; unsigned char __ch;
 char const *__iter,*__end,*__flush_start,*__c_hex;
 __end = (__iter = __flush_start = __text)+__textlen;
 __c_hex = __printf_decimals[!(__flags&FORMAT_QUOTE_FLAG_UPPERHEX)];
 __encoded_text[0] = '\\';
 if (!(__flags&FORMAT_QUOTE_FLAG_PRINTRAW)) __print("\"",1);
 while (__iter != __end) {
  __ch = *(unsigned char *)__iter;
  if (__ch < 32    || __ch >= 127  || __ch == '\'' ||
      __ch == '\"' || __ch == '\\' ||
     (__flags&FORMAT_QUOTE_FLAG_NOASCII)) {
   /* Character requires special encoding. */
#if 0
   if (!ch && !(__flags&FORMAT_QUOTE_FLAG_QUOTEALL)) goto done;
#endif
   /* Flush unprinted text. */
   if (__iter != __flush_start)
       __print(__flush_start,(__size_t)(__iter-__flush_start));
   __flush_start = __iter+1;
   if (__ch < 32) {
#if 0
    goto __encode_hex;
#endif
    /* Control character. */
    if (__flags&FORMAT_QUOTE_FLAG_NOCTRL) {
__default_ctrl:
     if (__flags&FORMAT_QUOTE_FLAG_FORCEHEX) goto __encode_hex;
__encode_oct:
     if (__ch <= 0x07) {
      __encoded_text[1] = __tooct((__ch & 0x07));
      __encoded_text_size = 2;
     } else if (__ch <= 0x38) {
      __encoded_text[1] = __tooct((__ch & 0x38) >> 3);
      __encoded_text[2] = __tooct((__ch & 0x07));
      __encoded_text_size = 3;
     } else {
      __encoded_text[1] = __tooct((__ch & 0xC0) >> 6);
      __encoded_text[2] = __tooct((__ch & 0x38) >> 3);
      __encoded_text[3] = __tooct((__ch & 0x07));
      __encoded_text_size = 4;
     }
     goto __print_encoded;
    }
__special_control:
    switch (__ch) {
    case '\a':   __ch = 'a'; break;
    case '\b':   __ch = 'b'; break;
    case '\f':   __ch = 'f'; break;
    case '\n':   __ch = 'n'; break;
    case '\r':   __ch = 'r'; break;
    case '\t':   __ch = 't'; break;
    case '\v':   __ch = 'v'; break;
    case '\033': __ch = 'e'; break;
    case '\\': case '\'': case '\"': break;
    default: goto __default_ctrl;
    }
    __encoded_text[1] = (char)__ch;
    __encoded_text_size = 2;
    goto __print_encoded;
   } else if ((__ch == '\\' || __ch == '\'' || __ch == '\"') &&
             !(__flags&FORMAT_QUOTE_FLAG_NOCTRL)) {
    goto __special_control;
   } else {
    /* Non-ascii character. */
/*__default_nonascii:*/
    if (__flags&FORMAT_QUOTE_FLAG_FORCEOCT) goto __encode_oct;
__encode_hex:
    __encoded_text[1] = 'x';
    if (__ch <= 0xf) {
     __encoded_text[2] = __c_hex[__ch];
     __encoded_text_size = 3;
    } else {
     __encoded_text[2] = __c_hex[(__ch & 0xf0) >> 4];
     __encoded_text[3] = __c_hex[__ch&0xf];
     __encoded_text_size = 4;
    }
__print_encoded:
    __print(__encoded_text,__encoded_text_size);
    goto __next;
   }
  }
__next:
  ++__iter;
 }
/*done:*/
 if (__iter != __flush_start)
     __print(__flush_start,(__size_t)(__iter-__flush_start));
 if (!(__flags&FORMAT_QUOTE_FLAG_PRINTRAW))
     __print("\"",1);
 return __result;
}
#undef __tooct

__FORMAT_PRINTER_IMPL int
(__LIBCCALL stringprinter_init)(struct stringprinter *__restrict __self, __size_t __hint) {
 if (!__hint) __hint = 4*sizeof(void *);
 __self->__sp_buffer = (char *)__hybrid_malloc((__hint+1)*sizeof(char));
 if __unlikely(!__self->__sp_buffer) {
  __self->__sp_bufpos = __NULLPTR;
  __self->__sp_buffer = __NULLPTR;
  return -1;
 }
 __self->__sp_bufpos = __self->__sp_buffer;
 __self->__sp_bufend = __self->__sp_buffer+__hint;
 __self->__sp_bufend[0] = '\0';
 return 0;
}
__FORMAT_PRINTER_IMPL char *
(__LIBCCALL stringprinter_pack)(struct stringprinter *__restrict __self, __size_t *__length) {
 char *__result; __size_t __result_size;
 assert(__self->__sp_bufpos >= __self->__sp_buffer);
 assert(__self->__sp_bufpos <= __self->__sp_bufend);
 __result_size = (__size_t)(__self->__sp_bufpos-__self->__sp_buffer);
 if (__self->__sp_bufpos != __self->__sp_bufend) {
  __result = (char *)__hybrid_realloc(__self->__sp_buffer,(__result_size+1)*sizeof(char));
  if __unlikely(!__result) __result = __self->__sp_buffer;
 } else {
  __result = __self->__sp_buffer;
 }
 __result[__result_size] = '\0';
 __self->__sp_buffer = __NULLPTR;
 if (__length) *__length = __result_size;
 return __result;
}
__FORMAT_PRINTER_IMPL void
(__LIBCCALL stringprinter_fini)(struct stringprinter *__restrict __self) {
 __hybrid_free(__self->__sp_buffer);
}
__FORMAT_PRINTER_IMPL __ssize_t
(__LIBCCALL stringprinter_print)(char const *__restrict __data,
                                 __size_t __datalen, void *__closure) {
 struct stringprinter *__self = (struct stringprinter *)__closure;
 __size_t __size_avail,__newsize,__reqsize;
 char *__new_buffer;
 assert(__self->__sp_bufpos >= __self->__sp_buffer);
 assert(__self->__sp_bufpos <= __self->__sp_bufend);
 __size_avail = (__size_t)(__self->__sp_bufend-__self->__sp_bufpos);
 if unlikely(__size_avail < __datalen) {
  /* Must allocate more memory. */
  __newsize = (__size_t)(__self->__sp_bufend-__self->__sp_buffer);
  assert(__newsize);
  __reqsize = __newsize+(__datalen-__size_avail);
  /* Double the buffer size until it is of sufficient length. */
  do __newsize *= 2; while (__newsize < __reqsize);
  /* Reallocate the buffer (But include 1 character for the terminating '\0') */
  __new_buffer = (char *)__hybrid_realloc(__self->__sp_buffer,(__newsize+1)*sizeof(char));
  if unlikely(!__new_buffer) return -1;
  __self->__sp_bufpos = __new_buffer+(__self->__sp_bufpos-__self->__sp_buffer);
  __self->__sp_bufend = __new_buffer+__newsize;
  __self->__sp_buffer = __new_buffer;
 }
 __hybrid_memcpy(__self->__sp_bufpos,__data,__datalen*sizeof(char));
 __self->__sp_bufpos += __datalen;
 assert(__self->__sp_bufpos <= __self->__sp_bufend);
 return 0;
}



#undef __PRINTF_FLAG_NONE
#undef __PRINTF_FLAG_PREFIX
#undef __PRINTF_FLAG_LJUST
#undef __PRINTF_FLAG_SIGN
#undef __PRINTF_FLAG_SPACE
#undef __PRINTF_FLAG_PADZERO
#undef __PRINTF_FLAG_HASWIDTH
#undef __PRINTF_FLAG_HASPREC
#undef __PRINTF_FLAG_UPPER
#undef __PRINTF_FLAG_SIGNED
#undef __PRINTF_FLAG_FIXBUF
#undef __PRINTF_FLAG_QUOTE
#undef __PRINTF_FLAG_CHQUOTE
#undef __PRINTF_VA_SIZE
#undef __PRINTF_LEN_I64
#undef __PRINTF_LEN_I32
#undef __PRINTF_LEN_I16
#undef __PRINTF_LEN_I8
#undef __PRINTF_FINT_LOAD
#undef __PRINTF_LEN_PTR
#undef __PRINTF_LEN_hh
#undef __PRINTF_LEN_h
#undef __PRINTF_LEN_l
#undef __PRINTF_LEN_ll
#undef __PRINTF_LEN_j
#undef __print
#undef __printf
#undef __quote
#undef __FORMAT_PRINTER_IMPL

#ifdef _MSC_VER
#pragma warning(pop)
#endif

__SYSDECL_END

#endif /* !_LIBC_FORMAT_PRINTER_H */
