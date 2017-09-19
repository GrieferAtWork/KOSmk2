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
#ifndef GUARD_LIBS_LIBC_FORMAT_PRINTER_C
#define GUARD_LIBS_LIBC_FORMAT_PRINTER_C 1
#define _KOS_SOURCE 1

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#include "libc.h"
#include "format-printer.h"
#include "string.h"
#include "fcntl.h"

#include <alloca.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <format-printer.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/kdev_t.h>
#include <hybrid/minmax.h>
#include <hybrid/typecore.h>
#include <hybrid/types.h>
#include <hybrid/types.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef __KERNEL__
#include <fs/fs.h>
#include <fs/file.h>
#include <fs/dentry.h>
#else
#include <kos/fcntl.h>
#include "malloc.h"
#endif

DECL_BEGIN

#define print(s,n) \
do{ if ((temp = (*printer)(s,n,closure)) < 0) return temp; \
    result += temp; \
}while(0)
#define printf(...) \
do{ if ((temp = libc_format_printf(printer,closure,__VA_ARGS__)) < 0) return temp; \
    result += temp; \
}while(0)
#define quote(s,n,flags) \
do{ if ((temp = libc_format_quote(printer,closure,s,n,flags)) < 0) return temp; \
    result += temp; \
}while(0)
#define hexdump(data,size,linesize,flags) \
do{ if ((temp = libc_format_hexdump(printer,closure,data,size,linesize,flags)) < 0) return temp; \
    result += temp; \
}while(0)

#define VA_SIZE  __SIZEOF_INT__

#define PRINTF_EXTENSION_POINTERSIZE 1        /*< Allow pointer-size length modifiers 'I' */
#define PRINTF_EXTENSION_FIXLENGTH   1        /*< Allow fixed length modifiers 'I8|I16|I32|I64' */
#define PRINTF_EXTENSION_DOTQUESTION 1        /*< Allow '.?' in place of '.*' to read size_t from the argument list instead of 'unsigned int', as well as force fixed-length buffers. */
#define PRINTF_EXTENSION_QUOTESTRING 1        /*< Allow '%q' in place of '%s' for quoting a string (escaping all special characters). */
#define PRINTF_EXTENSION_QUOTEFLAG   1        /*< Allow '%Q' to enable quoting mode, allowing for '%Qc' and '%Qs' (same as '%q'). */
#define PRINTF_EXTENSION_NULLSTRING  "(null)" /*< Replace NULL-arguments to %s and %q with this string (don't define to cause undefined behavior) */
#define STRFTIME_EXTENSION_LONGNAMES 1        /*< Allow %[...] for long attribute names. */
#define PRINTF_EXTENSION_VIRTUALPTR  1        /*< Allow '%~' to describe virtual pointers. */
#define PRINTF_EXTENSION_LONGDESCR   1        /*< Allow '%[...]' for a set of special printf descriptors. */
#define PRINTF_EXTENSION_ERRORMSG    1        /*< Emit error messages for illegal printf() formats. */
#ifndef CONFIG_DEBUG
#undef PRINTF_EXTENSION_ERRORMSG
#define PRINTF_EXTENSION_ERRORMSG    0
#endif

#ifndef __KERNEL__
#undef PRINTF_EXTENSION_VIRTUALPTR
#define PRINTF_EXTENSION_VIRTUALPTR  0
#endif


enum printf_length {
#if VA_SIZE == 8
 len_I64  = 0,
 len_I32  = len_I64,
 len_I16  = len_I64,
 len_I8   = len_I64,
#elif VA_SIZE == 4
 len_I32  = 0,
 len_I64  = 1,
 len_I16  = len_I32,
 len_I8   = len_I32,
#elif VA_SIZE == 2
 len_I16  = 0,
 len_I32  = 1,
 len_I64  = 2,
 len_I8   = len_I16,
#elif VA_SIZE == 1
 len_I8   = 0,
 len_I16  = 1,
 len_I32  = 2,
 len_I64  = 3,
#else
#   error FIXME
#endif
 len_L    = 'L',len_z = 'z',len_t = 't',
 len_I    = PP_CAT2(len_I,PP_MUL8(__SIZEOF_POINTER__)),
 len_hh   = PP_CAT2(len_I,PP_MUL8(__SIZEOF_CHAR__)),
 len_h    = PP_CAT2(len_I,PP_MUL8(__SIZEOF_SHORT__)),
 len_l    = PP_CAT2(len_I,PP_MUL8(__SIZEOF_LONG__)),
 len_ll   = PP_CAT2(len_I,PP_MUL8(__SIZEOF_LONG_LONG__)),
 len_j    = len_I64, /* intmax_t */
};

#define PRINTF_FLAG_NONE     0x0000
#define PRINTF_FLAG_PREFIX   0x0001 /*< '%#'. */
#define PRINTF_FLAG_LJUST    0x0002 /*< '%-'. */
#define PRINTF_FLAG_SIGN     0x0004 /*< '%+'. */
#define PRINTF_FLAG_SPACE    0x0008 /*< '% '. */
#define PRINTF_FLAG_PADZERO  0x0010 /*< '%0'. */
#define PRINTF_FLAG_HASWIDTH 0x0020 /*< '%123'. */
#define PRINTF_FLAG_HASPREC  0x0040 /*< '%.123'. */
#define PRINTF_FLAG_UPPER    0x0080 /*< Print upper-case hex-characters. */
#define PRINTF_FLAG_SIGNED   0x0100
#if PRINTF_EXTENSION_DOTQUESTION
#define PRINTF_FLAG_FIXBUF   0x0200
#endif
#if PRINTF_EXTENSION_QUOTEFLAG
#define PRINTF_FLAG_QUOTE    0x0400
#define PRINTF_FLAG_CHQUOTE  0x0800 /*< Quote as character. */
#endif
#if PRINTF_EXTENSION_VIRTUALPTR
#define PRINTF_FLAG_VIRTUAL  0x1000 /*< '%~' (Modifies %s: the pointer referrs to a user-space address). */
#endif

#if PRINTF_EXTENSION_LONGDESCR
#define LONGPRINT_NAMEMAX 8
struct longprint {
 char const         lp_name[LONGPRINT_NAMEMAX];
 ssize_t (LIBCCALL *lp_func)(pformatprinter printer, void *closure,
                             enum printf_length length, u16 flags,
                             size_t precision, va_list *args);
};
#define LONG_PRINTER(name) \
 PRIVATE ssize_t LIBCCALL name(pformatprinter printer, void *closure, \
                               enum printf_length length, u16 flags, \
                               size_t precision, va_list *args)

LONG_PRINTER(errno_printer);
LONG_PRINTER(dev_t_printer);
LONG_PRINTER(hex_printer);
#ifdef __KERNEL__
LONG_PRINTER(dentrypath_printer);
LONG_PRINTER(filepath_printer);
#else
LONG_PRINTER(fdpath_printer);
#endif

PRIVATE struct longprint const ext_printers[] = {
 {"errno",&errno_printer},
 {"dev_t",&dev_t_printer},
 {"hex",&hex_printer},
#ifdef __KERNEL__
 {"dentry",&dentrypath_printer},
 {"file",&filepath_printer},
#else
 {"fd",&fdpath_printer},
 {"dentry",&fdpath_printer},
 {"file",&fdpath_printer},
#endif
 {"",NULL},
};

LONG_PRINTER(errno_printer) {
 errno_t error = va_arg(*args,errno_t);
 char const *msg = libc_strerror_s(error);
 (void)length,(void)precision,(void)flags;
 if (!msg) return libc_format_printf(printer,closure,"Unknown error %d",error);
 return libc_format_printf(printer,closure,"%s(%s)",libc_strerrorname_s(error),msg);
}
LONG_PRINTER(dev_t_printer) {
 dev_t dev = va_arg(*args,dev_t);
 (void)length,(void)precision,(void)flags;
 return libc_format_printf(printer,closure,"%.2x:%.2x",
                           MAJOR(dev),MINOR(dev));
}
LONG_PRINTER(hex_printer) {
 void *p = va_arg(*args,void *);
 (void)length;
 if (!(flags&PRINTF_FLAG_FIXBUF))
       precision = libc_strnlen((char *)p,precision);
 return libc_format_hexdump(printer,closure,p,precision,0,
                            FORMAT_HEXDUMP_FLAG_ADDRESS);
}
#ifdef __KERNEL__
PRIVATE ssize_t LIBCCALL
print_dentry_path_r(pformatprinter printer, void *closure,
                    struct dentry const *__restrict entry,
                    struct dentry const *__restrict fs_root) {
 ssize_t result = 0,temp;
 if (entry == fs_root) goto end;
 CHECK_HOST_DOBJ(entry->d_parent);
 temp = print_dentry_path_r(printer,closure,
                            entry->d_parent,
                            fs_root);
 if unlikely(temp < 0) return temp;
 result += temp;
 temp = (*printer)("/",1,closure);
 if unlikely(temp < 0) return temp;
 result += temp;
 temp = (*printer)(entry->d_name.dn_name,
                   entry->d_name.dn_size,
                   closure);
 if unlikely(temp < 0) return temp;
 result += temp;
end:
 return result;
}
PRIVATE ssize_t LIBCCALL
print_dentry_path(pformatprinter printer, void *closure,
                  struct dentry const *__restrict entry) {
 /* TODO: Use chroot() filesystem root. */
 struct dentry const *root = &fs_root;
 if (entry == root) return (*printer)("/",1,closure);
 return print_dentry_path_r(printer,closure,entry,root);
}
LONG_PRINTER(dentrypath_printer) {
 struct dentry *p = va_arg(*args,struct dentry *);
 (void)length,(void)precision,(void)flags;
#ifdef PRINTF_EXTENSION_NULLSTRING
 if (!p) return (*printer)(PRINTF_EXTENSION_NULLSTRING,
                           COMPILER_STRLEN(PRINTF_EXTENSION_NULLSTRING),
                           closure);
#endif
 return print_dentry_path(printer,closure,p);
}
LONG_PRINTER(filepath_printer) {
 struct file *p = va_arg(*args,struct file *);
 (void)length,(void)precision,(void)flags;
#ifdef PRINTF_EXTENSION_NULLSTRING
 if (!p) return (*printer)(PRINTF_EXTENSION_NULLSTRING,
                           COMPILER_STRLEN(PRINTF_EXTENSION_NULLSTRING),
                           closure);
#endif
 return print_dentry_path(printer,closure,p->f_dent);
}
#else /* __KERNEL__ */
LONG_PRINTER(fdpath_printer) {
 char buf[256],*bufp; ssize_t result;
 int fd = va_arg(*args,int);
 (void)length,(void)precision,(void)flags;
 bufp = libc_xfdname(fd,FDNAME_PATH,buf,sizeof(buf));
 if (!bufp) bufp = libc_xfdname(fd,FDNAME_PATH,NULL,0);
 if (!bufp) return -1;
 result = (*printer)(bufp,libc_strlen(bufp),closure);
 if (buf != bufp) libc_free(bufp);
 return result;
}
#endif /* !__KERNEL__ */

#endif /* PRINTF_EXTENSION_LONGDESCR */


PRIVATE char const decimals[2][17] = {
 {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','x'},
 {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','X'},
};

INTERN ssize_t LIBCCALL
libc_format_vprintf(pformatprinter printer, void *closure,
                    char const *__restrict format, va_list args) {
 ssize_t result = 0,temp; char ch;
 char const *flush_start;
 CHECK_HOST_TEXT(printer,1);
#ifdef CONFIG_DEBUG
 /* Running strlen() on 'format' implicitly performs CHECK_HOST_TEXT-checks. */
 COMPILER_UNUSED(libc_strlen(format));
#endif
 flush_start = format;
 for (;;) {
next_normal:
  ch = *format++;
  if unlikely(!ch) break;
  if (ch == '%') {
   enum printf_length length;
   u16 flags; size_t width,precision,print_width;
   if (format-1 != flush_start)
       print(flush_start,(size_t)((format-1)-flush_start));
   flush_start = format;
   length      = (enum printf_length)0;
   flags       = 0;
   width       = 0;
   precision   = 0;
   print_width = (size_t)result;
nextmod:
   ch     = *format++;
nextmodc:
   switch (ch) {
   case '\0': goto end;
   case '-': flags |= PRINTF_FLAG_LJUST;   goto nextmod;
   case '+': flags |= PRINTF_FLAG_SIGN;    goto nextmod;
   case ' ': flags |= PRINTF_FLAG_SPACE;   goto nextmod;
   case '#': flags |= PRINTF_FLAG_PREFIX;  goto nextmod;
   case '0': flags |= PRINTF_FLAG_PADZERO; goto nextmod;
#ifdef PRINTF_FLAG_QUOTE
   case 'Q': flags |= PRINTF_FLAG_QUOTE;   goto nextmod;
#endif
#ifdef PRINTF_FLAG_VIRTUAL
   case '~': flags |= PRINTF_FLAG_VIRTUAL; goto nextmod;
#endif
#if PRINTF_EXTENSION_DOTQUESTION
   case '?':
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
    width = va_arg(args,size_t);
    goto have_width;
#endif
#endif
   case '*':
    width = (size_t)va_arg(args,unsigned int); /* Technically int, but come on... */
#if PRINTF_EXTENSION_DOTQUESTION && __SIZEOF_INT__ != __SIZEOF_SIZE_T__
have_width:
#endif
    flags |= PRINTF_FLAG_HASWIDTH;
    goto nextmod;

    /* Precision */
#ifdef PRINTF_FLAG_FIXBUF
   case ':':
    flags |= PRINTF_FLAG_FIXBUF;
#endif
   case '.':
    ch = *format++;
#if PRINTF_EXTENSION_DOTQUESTION
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
    if (ch == '?') { case '$': width = va_arg(args,size_t); goto have_precision; }
#else
    if (ch == '?') { case '$': flags |= PRINTF_FLAG_FIXBUF; goto use_precision; }
#endif
    else
#endif
    if (ch == '*')
    {
#if PRINTF_EXTENSION_DOTQUESTION && (__SIZEOF_INT__ == __SIZEOF_SIZE_T__)
use_precision:
#endif
     precision = (size_t)va_arg(args,unsigned int); /* Technically int, but come on... */
#if PRINTF_EXTENSION_DOTQUESTION && (__SIZEOF_INT__ != __SIZEOF_SIZE_T__)
have_precision:
#endif
    } else if (ch >= '0' && ch <= '9') {
     for (;;) {
      precision *= 10;
      precision += (size_t)(ch-'0');
      ch = *format++;
      if (ch < '0' || ch > '9') break;
     }
     --format;
    } else goto broken_format;
    flags |= PRINTF_FLAG_HASPREC;
    goto nextmod;

    /* Length modifiers */
   case 'h':
    if (*format != 'h') length = len_h;
    else { ++format; length = len_hh; }
    goto nextmod;

   case 'l':
    if (*format != 'l') length = len_l;
    else { ++format; length = len_ll; }
    goto nextmod;

   case 'j':
    length = len_j;
    goto nextmod;

   case 'z': case 't': case 'L':
    length = (enum printf_length)ch;
    goto nextmod;

#if PRINTF_EXTENSION_POINTERSIZE || PRINTF_EXTENSION_FIXLENGTH
   case 'I':
#if PRINTF_EXTENSION_FIXLENGTH
    ch = *format++;
    if (ch == '8') length = len_I8;
    else if (ch == '1' && *format == '6') ++format,length = len_I16;
    else if (ch == '3' && *format == '2') ++format,length = len_I32;
    else if (ch == '6' && *format == '4') ++format,length = len_I64;
#if PRINTF_EXTENSION_POINTERSIZE
    else { length = len_I; goto nextmodc; }
#else
    else { --format; goto broken_format; }
#endif
#else /* PRINTF_EXTENSION_FIXLENGTH */
    length = len_I;
#endif /* !PRINTF_EXTENSION_FIXLENGTH */
    goto nextmod;
#endif

   {
    int numsys; /* Integer formating. */
    char buf[32],*iter;
    char const *dec;
    union {
     s8 s_8; s16 s_16; s32 s_32; s64 s_64;
     u8 u_8; u16 u_16; u32 u_32; u64 u_64;
    } arg; size_t bufsize;
    if (0) { case 'B': flags |= PRINTF_FLAG_UPPER; case 'b': numsys = 2; }
    if (0) { case 'o': numsys = 8; }
    if (0) { case 'u': numsys = 10; if unlikely(length == len_t) flags |= PRINTF_FLAG_SIGNED; }
    if (0) { case 'd': case 'i': numsys = 10; if likely(length != len_z) flags |= PRINTF_FLAG_SIGNED; }
#if __SIZEOF_POINTER__ > VA_SIZE
#error "TODO: Explicit handling when 'sizeof(void *) > VA_SIZE' (Must fix 'length')"
#endif
    if (0) { case 'p': if (!(flags&PRINTF_FLAG_HASPREC)) { precision = sizeof(void *)*2; flags |= PRINTF_FLAG_HASPREC; }
             case 'X': flags |= PRINTF_FLAG_UPPER;
             case 'x': numsys = 16; if unlikely(length == len_t) flags |= PRINTF_FLAG_SIGNED; }
    arg.u_64 = 0;
#if VA_SIZE == 8
    arg.u_64 = va_arg(args,u64);
#elif VA_SIZE == 4
    if (length == len_I64) arg.u_64 = va_arg(args,u64);
    else { arg.u_32 = va_arg(args,u32); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_32; }
#elif VA_SIZE == 2
         if (length == len_I64) arg.u_64 = va_arg(args,u64);
    else if (length == len_I32) { arg.u_32 = va_arg(args,u32); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_32; }
    else { arg.u_16 = va_arg(args,u16); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_16; }
#else
         if (length == len_I64) arg.u_64 = va_arg(args,u64);
    else if (length == len_I32) { arg.u_32 = va_arg(args,u32); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_32; }
    else if (length == len_I16) { arg.u_16 = va_arg(args,u16); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_16; }
    else { arg.u_8 = va_arg(args,u8); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_8; }
#endif
    iter = buf;
    if (flags&PRINTF_FLAG_SIGNED) {
     if (arg.s_64 < 0) {
      *iter++ = '-';
      arg.s_64 = -arg.s_64;
     } else if (flags&PRINTF_FLAG_SPACE) {
      *iter++ = ' ';
     }
    }
    dec = decimals[!!(flags&PRINTF_FLAG_UPPER)];
    if (flags&PRINTF_FLAG_PREFIX && numsys != 10) {
     *iter++ = '0';
          if (numsys == 16) *iter++ = dec[16]; /* X/x */
     else if (numsys == 2)  *iter++ = dec[11]; /* B/b */
    }
    if (iter != buf) print(buf,(size_t)(iter-buf));
    libc_memset(buf,0xcc,sizeof(buf));
    iter = COMPILER_ENDOF(buf);
    assertf(numsys <= 16,"%d",numsys);
    do *--iter = dec[arg.u_64 % numsys];
    while ((arg.u_64 /= numsys) != 0);
    for (;;) {
     bufsize = (size_t)(COMPILER_ENDOF(buf)-iter);
     if ((flags&PRINTF_FLAG_HASPREC) &&
          precision > bufsize) {
      size_t precbufsize = COMPILER_LENOF(buf)-bufsize;
      precision -= bufsize;
      if (precbufsize > precision)
          precbufsize = precision;
      assert(precbufsize);
      bufsize += precbufsize;
      iter    -= precbufsize;
      libc_memset(iter,'0',precbufsize);
      assert(precbufsize <= precision);
      precision -= precbufsize;
     }
     print(iter,bufsize);
     if (precision <= bufsize) break;
     assert(flags&PRINTF_FLAG_HASPREC);
     iter = COMPILER_ENDOF(buf);
    }
   } break;

   {
    char *s;
   {
    char given_char[1];
   case 'c':
    given_char[0] = va_arg(args,int);
#ifdef PRINTF_FLAG_QUOTE
    if (flags&PRINTF_FLAG_QUOTE) {
     flags    |= PRINTF_FLAG_CHQUOTE;
     s         = given_char;
     precision = 1;
     goto quote_string;
    } else
#endif
    {
     print(given_char,1);
    }
   } break;

#if PRINTF_EXTENSION_QUOTESTRING
   case 'q':
#ifdef PRINTF_FLAG_QUOTE
    flags |= PRINTF_FLAG_QUOTE;
#endif
#endif /* PRINTF_EXTENSION_QUOTESTRING */
   case 's':
    s = va_arg(args,char *);
#ifdef PRINTF_EXTENSION_NULLSTRING
    if (!s) s = PRINTF_EXTENSION_NULLSTRING;
#endif
    if (!(flags&PRINTF_FLAG_HASPREC)) precision = (size_t)-1;
#ifdef PRINTF_FLAG_FIXBUF
    if (!(flags&PRINTF_FLAG_FIXBUF))
#endif /* PRINTF_FLAG_FIXBUF */
    {
     if (length == len_L) {
      u16 *end,*iter = (u16 *)s;
      end = (u16 *)((uintptr_t)s+(precision & ~1));
      while (iter != end && *iter) ++iter;
      precision = (uintptr_t)iter-(uintptr_t)s;
     } else {
      precision = libc_strnlen(s,precision);
     }
    }
#ifdef PRINTF_FLAG_QUOTE
    if (flags&PRINTF_FLAG_QUOTE)
#elif PRINTF_EXTENSION_QUOTESTRING
    if (ch == 'q')
#endif
#if PRINTF_EXTENSION_QUOTESTRING || PRINTF_EXTENSION_QUOTEFLAG
    {
#if PRINTF_EXTENSION_QUOTEFLAG
quote_string:
#endif
     /* Quote string. */
#ifdef PRINTF_FLAG_CHQUOTE
     if (flags&PRINTF_FLAG_CHQUOTE) {
      if (flags&PRINTF_FLAG_PREFIX)
           flags &= ~(PRINTF_FLAG_CHQUOTE);
      else flags |=   PRINTF_FLAG_PREFIX;
      print("\'",1);
     }
#endif
#if PRINTF_FLAG_PREFIX == FORMAT_QUOTE_FLAG_PRINTRAW
     quote(s,precision,flags&PRINTF_FLAG_PREFIX);
#else
     quote(s,precision,flags&PRINTF_FLAG_PREFIX
           ? FORMAT_QUOTE_FLAG_PRINTRAW
           : FORMAT_QUOTE_FLAG_NONE);
#endif
#ifdef PRINTF_FLAG_CHQUOTE
     if (flags&PRINTF_FLAG_CHQUOTE) print("\'",1);
#endif
    } else
#endif /* Quote... */
    {
     if (length == len_L) {
      char const *s_end = s+(precision&~1);
      for (; s != s_end; s += 2) print(s,1);
     } else {
      print(s,precision);
     }
    }
   } break;

#if 0
   {
   case 'f':
   case 'g':
    /* TODO: Floating point printing. */
   } break;
#endif

#if PRINTF_EXTENSION_LONGDESCR
   {
    size_t format_len;
    struct longprint const *printers;
   case '[':
    format_len = libc_stroff(format,']');
    if (format_len <= LONGPRINT_NAMEMAX) {
     for (printers = ext_printers;
          printers->lp_func; ++printers) {
      if (!libc_memcmp(printers->lp_name,format,format_len)) {
       temp = (*printers->lp_func)(printer,closure,length,
                                   flags,precision,&args);
       if (temp < 0) return temp;
       result += temp;
       format += format_len;
       if (*format) ++format;
       goto done_fmt;
      }
     }
    }
#if PRINTF_EXTENSION_ERRORMSG
    printf("<%s(%.?q)>",format[format_len]
           ? "UNKNOWN_LONG_PRINTER"
           : "MISSING_RBRACKED",
           format_len,format);
    goto broken_format2;
#else
    goto broken_format;
#endif
   } break;
#endif

   case '%': goto done_fmt;
   default:
    if (ch >= '0' && ch <= '9') {
     for (;;) {
      width *= 10;
      width += (size_t)(ch-'0');
      ch = *format++;
      if (ch < '0' || ch > '9') break;
     }
     flags |= PRINTF_FLAG_HASWIDTH;
     goto nextmodc;
    }
broken_format:
#if PRINTF_EXTENSION_ERRORMSG
    printf("<INVALID_FORMAT_OPTION:'%c'>",ch);
broken_format2: ATTR_UNUSED
#endif
    format = flush_start;
    goto next_normal;
   }
done_fmt:
   if (flags&PRINTF_FLAG_HASWIDTH) {
    print_width = (size_t)result-print_width;
    if (print_width < width) {
     size_t partsize;
     char buf[32];
     print_width = width-print_width;
     partsize    = print_width;
     if (partsize > COMPILER_LENOF(buf))
         partsize = COMPILER_LENOF(buf);
     libc_memset(buf,' ',partsize);
     while (print_width > COMPILER_LENOF(buf)) {
      print(buf,COMPILER_LENOF(buf));
      print_width -= COMPILER_LENOF(buf);
     }
     print(buf,partsize);
    }
   }
   flush_start = format;
  }
 }
end:
 --format;
 assert(!*format);
 if (flush_start != format)
     print(flush_start,(size_t)(format-flush_start));
 return result;
}

INTERN ssize_t ATTR_CDECL 
libc_format_printf(pformatprinter printer, void *closure,
                   char const *__restrict format, ...) {
 ssize_t result;
 va_list args;
 va_start(args,format);
 result = libc_format_vprintf(printer,closure,format,args);
 va_end(args);
 return result;
}



#define tooct(c) ('0'+(c))
INTERN ssize_t LIBCCALL
libc_format_quote(pformatprinter printer, void *closure,
                  char const *__restrict text, size_t textlen,
                  u32 flags) {
 char encoded_text[4];
 size_t encoded_text_size;
 ssize_t result = 0,temp; unsigned char ch;
 char const *iter,*end,*flush_start,*c_hex;
 end = (iter = flush_start = text)+textlen;
 c_hex = decimals[!(flags&FORMAT_QUOTE_FLAG_UPPERHEX)];
 encoded_text[0] = '\\';
 if (!(flags&FORMAT_QUOTE_FLAG_PRINTRAW)) print("\"",1);
 while (iter != end) {
  ch = *(unsigned char *)iter;
  if (ch < 32    || ch >= 127  || ch == '\'' ||
      ch == '\"' || ch == '\\' ||
     (flags&FORMAT_QUOTE_FLAG_NOASCII)) {
   /* Character requires special encoding. */
#if 0
   if (!ch && !(flags&FORMAT_QUOTE_FLAG_QUOTEALL)) goto done;
#endif
   /* Flush unprinted text. */
   if (iter != flush_start)
       print(flush_start,(size_t)(iter-flush_start));
   flush_start = iter+1;
   if (ch < 32) {
#if 0
    goto encode_hex;
#endif
    /* Control character. */
    if (flags&FORMAT_QUOTE_FLAG_NOCTRL) {
default_ctrl:
     if (flags&FORMAT_QUOTE_FLAG_FORCEHEX) goto encode_hex;
encode_oct:
     if (ch <= 0x07) {
      encoded_text[1] = tooct((ch & 0x07));
      encoded_text_size = 2;
     } else if (ch <= 0x38) {
      encoded_text[1] = tooct((ch & 0x38) >> 3);
      encoded_text[2] = tooct((ch & 0x07));
      encoded_text_size = 3;
     } else {
      encoded_text[1] = tooct((ch & 0xC0) >> 6);
      encoded_text[2] = tooct((ch & 0x38) >> 3);
      encoded_text[3] = tooct((ch & 0x07));
      encoded_text_size = 4;
     }
     goto print_encoded;
    }
special_control:
    switch (ch) {
    case '\a':   ch = 'a'; break;
    case '\b':   ch = 'b'; break;
    case '\f':   ch = 'f'; break;
    case '\n':   ch = 'n'; break;
    case '\r':   ch = 'r'; break;
    case '\t':   ch = 't'; break;
    case '\v':   ch = 'v'; break;
    case '\033': ch = 'e'; break;
    case '\\': case '\'': case '\"': break;
    default: goto default_ctrl;
    }
    encoded_text[1] = ch;
    encoded_text_size = 2;
    goto print_encoded;
   } else if ((ch == '\\' || ch == '\'' || ch == '\"') &&
             !(flags&FORMAT_QUOTE_FLAG_NOCTRL)) {
    goto special_control;
   } else {
    /* Non-ascii character. */
/*default_nonascii:*/
    if (flags&FORMAT_QUOTE_FLAG_FORCEOCT) goto encode_oct;
encode_hex:
    encoded_text[1] = 'x';
    if (ch <= 0xf) {
     encoded_text[2] = c_hex[ch];
     encoded_text_size = 3;
    } else {
     encoded_text[2] = c_hex[(ch & 0xf0) >> 4];
     encoded_text[3] = c_hex[ch&0xf];
     encoded_text_size = 4;
    }
print_encoded:
    print(encoded_text,encoded_text_size);
    goto next;
   }
  }
next:
  ++iter;
 }
/*done:*/
 if (iter != flush_start)
     print(flush_start,(size_t)(iter-flush_start));
 if (!(flags&FORMAT_QUOTE_FLAG_PRINTRAW))
     print("\"",1);
 return result;
}



#define MAX_SPACE_SIZE  64
#define MAX_ASCII_SIZE  64

PRIVATE ssize_t LIBCCALL
print_space(pformatprinter printer,
            void *closure, size_t count) {
 size_t used_size,bufsize;
 char *spacebuf; ssize_t result = 0,temp;
 bufsize = MIN(count,MAX_SPACE_SIZE);
 spacebuf = (char *)alloca(bufsize*sizeof(char));
 libc_memset(spacebuf,' ',bufsize*sizeof(char));
 for (;;) {
  used_size = MIN(count,bufsize);
  assert(spacebuf[0] == ' ');
  assert(spacebuf[used_size-1] == ' ');
  print(spacebuf,used_size/sizeof(char));
  if (used_size == count) break;
  count -= used_size;
 }
 return result;
}

INTERN ssize_t LIBCCALL
libc_format_hexdump(pformatprinter printer, void *closure,
                    void const *__restrict data, size_t size,
                    size_t linesize, u32 flags) {
 char hex_buf[3],*ascii_line;
 char const *hex_translate;
 byte_t const *line,*iter,*end; byte_t b;
 ssize_t result = 0,temp;
 size_t lineuse,overflow,ascii_size;
 unsigned int offset_size;
 if unlikely(!size) return 0;
 if unlikely(!linesize) linesize = 16;
 if (!(flags&FORMAT_HEXDUMP_FLAG_NOASCII)) {
  /* Allocate a small buffer we can overwrite for ascii text. */
  ascii_size = MIN(MAX_ASCII_SIZE,linesize);
  ascii_line = (char *)alloca(ascii_size);
 }
 if (flags&FORMAT_HEXDUMP_FLAG_OFFSETS) {
  /* Figure out how wide we should pad the address offset field. */
  size_t i = (size_t)1 << (offset_size = __SIZEOF_POINTER__*8-1);
  while (!(linesize&i)) --offset_size,i >>= 1;
  offset_size = CEILDIV(offset_size,4);
 }
 /* The last character of the hex buffer is always a space. */
 hex_buf[2] = ' ';
 /* Figure out the hex translation vector that should be used. */
 hex_translate = decimals[!!(flags&FORMAT_HEXDUMP_FLAG_HEXLOWER)];
 for (line = (byte_t const *)data;;) {
  if (linesize <= size) {
   lineuse  = linesize;
   overflow = 0;
  } else {
   lineuse  = size;
   overflow = linesize-size;
  }
  if (flags&(FORMAT_HEXDUMP_FLAG_ADDRESS|FORMAT_HEXDUMP_FLAG_OFFSETS)) {
   /* Must print some sort of prefix. */
   if (flags&FORMAT_HEXDUMP_FLAG_ADDRESS) printf("%p ",line);
   if (flags&FORMAT_HEXDUMP_FLAG_OFFSETS) printf("+%.*Ix ",offset_size,(uintptr_t)line-(uintptr_t)data);
  }
  end = line+lineuse;
  if (!(flags&FORMAT_HEXDUMP_FLAG_NOHEX)) {
   for (iter = line; iter != end; ++iter) {
    b = *iter;
    hex_buf[0] = hex_translate[(b&0xf0) >> 4];
    hex_buf[1] = hex_translate[b&0xf];
    print(hex_buf,COMPILER_LENOF(hex_buf));
   }
   if (overflow) {
    temp = print_space(printer,closure,overflow*
                       COMPILER_LENOF(hex_buf));
    if unlikely(temp < 0) return temp;
    result += temp;
   }
  }
  if (!(flags&FORMAT_HEXDUMP_FLAG_NOASCII)) {
   for (iter = line; iter != end;) {
    char *aiter,*aend;
    size_t textcount = MIN(ascii_size,(size_t)(end-iter));
    libc_memcpy(ascii_line,iter,textcount);
    /* Filter out non-printable characters, replacing them with '.' */
    aend = (aiter = ascii_line)+textcount;
    for (; aiter != aend; ++aiter) {
     if (!isprint(*aiter)) *aiter = '.';
    }
    /* Print our ascii text portion. */
    print(ascii_line,textcount);
    iter += textcount;
   }
   if (overflow) {
    /* Fill any overflow with space. */
    temp = print_space(printer,closure,overflow);
    if unlikely(temp < 0) return temp;
    result += temp;
   }
  }
  if (size == lineuse) break;
  line  = end;
  size -= lineuse;
  print("\n",1);
 }
 return result;
}








#ifndef __KERNEL__
INTERN ssize_t ATTR_CDECL
libc_format_scanf(pformatscanner scanner, pformatreturn returnch,
                  void *closure, char const *__restrict format, ...) {
 va_list args; ssize_t result;
 va_start(args,format);
 result = libc_format_vscanf(scanner,returnch,closure,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t LIBCCALL
libc_format_vscanf(pformatscanner scanner, pformatreturn returnch,
                   void *closure, char const *__restrict format, va_list args) {
 CHECK_HOST_TEXT(scanner,1);
 CHECK_HOST_TEXT(returnch,1);
#ifdef CONFIG_DEBUG
 /* Running strlen() on 'format' implicitly performs CHECK_HOST_TEXT-checks. */
 COMPILER_UNUSED(libc_strlen(format));
#endif

 /* TODO */
 return 0;
}





INTERN char const abbr_month_names[12][4] = {
 "Jan","Feb","Mar","Apr","May","Jun",
 "Jul","Aug","Sep","Oct","Nov","Dec"};
INTERN char const abbr_wday_names[7][4] = {
 "Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
INTERN char const full_month_names[12][10] = {
 "January","February","March","April","May","June",
 "July","August","September","October","November","December"};
INTERN char const full_wday_names[7][10] = {
 "Sunday","Monday","Tuesday","Wednesday",
 "Thursday","Friday","Saturday"};
INTERN char const am_pm[2][3] = {"AM","PM"};
INTERN char const am_pm_lower[2][3] = {"am","pm"};

struct time_attrib {
 char name[7];
 u8   offset;
};

PRIVATE struct time_attrib const time_attr[] = {
#define ATTRIB(name,field) {name,(u8)offsetof(struct tm,field)}
    ATTRIB("Y",tm_year),
    ATTRIB("M",tm_mon),
    ATTRIB("D",tm_mday),
    ATTRIB("H",tm_hour),
    ATTRIB("I",tm_min),
    ATTRIB("S",tm_sec),
    ATTRIB("MD",tm_mday),
    ATTRIB("MI",tm_min),
    ATTRIB("YD",tm_yday),
    ATTRIB("WD",tm_wday),
    ATTRIB("year",tm_year),
    ATTRIB("month",tm_mon),
    ATTRIB("wday",tm_wday),
    ATTRIB("mday",tm_mday),
    ATTRIB("yday",tm_yday),
    ATTRIB("hour",tm_hour),
    ATTRIB("minute",tm_min),
    ATTRIB("second",tm_sec),
#undef ATTRIB
    {"",0},
};
PRIVATE char const format_0dot2u[] = "%0.2u";

INTERN ssize_t LIBCCALL
libc_format_strftime(pformatprinter printer, void *closure,
                     char const *__restrict format, struct tm const *tm) {
 char const *format_begin,*iter; char ch;
 ssize_t temp,result = 0;
 CHECK_HOST_TEXT(printer,1);
 CHECK_HOST_DOBJ(tm);
#ifdef CONFIG_DEBUG
 /* Running strlen() on 'format' implicitly performs CHECK_HOST_TEXT-checks. */
 COMPILER_UNUSED(libc_strlen(format));
#endif
 iter = format_begin = format;
 for (;;) {
next:
  ch = *iter++;
  switch (ch) {
   case '\0': goto end;
   case '%': {
    if (format_begin != iter)
        print(format_begin,((size_t)(iter-format_begin))-1);
#define safe_elem(arr,i) ((arr)[MIN(i,COMPILER_LENOF(arr)-1)])

    switch ((ch = *iter++)) {
/* TODO: @begin locale_dependent */
     case 'a': print(safe_elem(abbr_wday_names,tm->tm_wday),(size_t)-1); break;
     case 'A': print(safe_elem(full_wday_names,tm->tm_wday),(size_t)-1); break;
     case 'h':
     case 'b': print(safe_elem(abbr_month_names,tm->tm_mon),(size_t)-1); break;
     case 'B': print(safe_elem(full_month_names,tm->tm_mon),(size_t)-1); break;
     case 'c': printf("%s %s %0.2u %0.2u:%0.2u:%0.2u %u",
                      abbr_wday_names[tm->tm_wday],
                      abbr_month_names[tm->tm_mon],
                      tm->tm_mday,tm->tm_hour,
                      tm->tm_min,tm->tm_sec,
                      tm->tm_year+1900);
               break;
     case 'x': printf("%0.2u/%0.2u/%0.2u",
                      tm->tm_mon+1,tm->tm_mday,
                      tm->tm_year+1900);
               break;
     case 'X': printf("%0.2u:%0.2u:%0.2u",
                      tm->tm_hour,tm->tm_min,
                      tm->tm_sec);
               break;
     case 'z': break; /* TODO: ISO 8601 offset from UTC in timezone (1 minute=1, 1 hour=100) | If timezone cannot be determined, no characters	+100 */
     case 'Z': break; /* TODO: Timezone name or abbreviation * | If timezone cannot be determined, no characters	CDT */
/* TODO: @end locale_dependent */
     case 'C': printf(format_0dot2u,((tm->tm_year+1900)/100)%100); break;
     case 'd': printf(format_0dot2u,tm->tm_mday); break;
     case 'D': printf("%0.2u/%0.2u/%0.2u",tm->tm_mon+1,tm->tm_mday,(tm->tm_year+1900)%100); break;
     case 'e': printf(format_0dot2u,tm->tm_mday); break;
     case 'F': printf("%0.4u-%0.2u-%0.2u",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday); break;
     case 'H': printf(format_0dot2u,tm->tm_hour); break;
     case 'I': printf(format_0dot2u,tm->tm_hour); break;
     case 'j': printf("%0.3u",tm->tm_yday+1); break;
     case 'm': printf(format_0dot2u,tm->tm_mon+1); break;
     case 'M': printf(format_0dot2u,tm->tm_min); break;
     case 'p': print(safe_elem(am_pm,tm->tm_hour/12),(size_t)-1); break;
     case 'r': printf("%0.2u:%0.2u:%0.2u %s",
                      tm->tm_hour%12,tm->tm_min,tm->tm_sec,
                      am_pm_lower[tm->tm_hour/12]);
               break;
     case 'R': printf("%0.2u:%0.2u",tm->tm_hour,tm->tm_min); break;
     case 'S': printf(format_0dot2u,tm->tm_sec); break;
     case 'T': printf("%0.2u:%0.2u:%0.2u",tm->tm_hour,tm->tm_min,tm->tm_sec); break;
     case 'u': printf(format_0dot2u,1+((tm->tm_wday+6)%7)); break;
     case 'w': printf(format_0dot2u,tm->tm_wday); break;
     case 'y': printf(format_0dot2u,(tm->tm_year+1900)%100); break;
     case 'Y': printf("%u",tm->tm_year+1900); break;

     /* I don't understand this week-based stuff.
      * I read the wikipedia article, but I still don't really get it.
      * >> So this might be supported in the future when I understand it...
      * %g	Week-based year, last two digits (00-99)	01
      * %G	Week-based year	2001
      * %U	Week number with the first Sunday as the first day of week one (00-53)	33
      * %V	ISO 8601 week number (00-53)	34
      * %W	Week number with the first Monday as the first day of week one (00-53)	34 */

     case 'n': print("\n",1); break;
     case 't': print("\t",1); break;
     case '%': case '\0': print("%",1);
      if (!ch) goto end;
      break;

#if STRFTIME_EXTENSION_LONGNAMES
     case '[': {
      char const *tag_begin,*tag_end,*mode_begin,*mode_end;
      unsigned int bracket_recursion = 1; unsigned int attribval;
      int repr_mode,width = 0; struct time_attrib const *attrib;
      size_t attribnam_len;
      /* Extended formatting */
      mode_end = mode_begin = tag_begin = iter;
      while (1) {
       ch = *iter++;
       if (ch == ']') { if (!--bracket_recursion) { tag_end = iter-1; break; } }
       else if (ch == '[') ++bracket_recursion;
       else if (ch == ':' && bracket_recursion == 1)
        mode_end = iter-1,tag_begin = iter;
       else if (!ch) { tag_end = iter; break; }
      }
      if (mode_begin != mode_end) {
       if (*mode_begin == 'n' || *mode_begin == 's' ||
           *mode_begin == 'S' || *mode_begin == ' ')
        repr_mode = *mode_begin++;
       else repr_mode = 0;
       /* Parse the width modifier */
       while (mode_begin != mode_end) {
        if (*mode_begin >= '0' && *mode_begin <= '9') {
         width = width*10+(*mode_begin-'0');
        } else goto format_end;
        ++mode_begin;
       }
      } else repr_mode = 0;
      attribnam_len = (size_t)(tag_end-tag_begin);
      for (attrib = time_attr;; ++attrib) {
       if (!attrib->name[0]) goto format_end;
       if (libc_memcmp(attrib->name,tag_begin,attribnam_len*sizeof(char)) == 0 &&
           attrib->name[attribnam_len] == '\0') break;
      }
      attribval = *(unsigned int *)((byte_t *)tm+attrib->offset);
      if (repr_mode == 's' || repr_mode == 'S') {
       char const *repr_value;
       if (attrib->offset == offsetof(struct tm,tm_mon)) {
        repr_value = repr_mode == 'S' ? safe_elem(full_month_names,attribval)
                                      : safe_elem(abbr_month_names,attribval);
       } else if (attrib->offset == offsetof(struct tm,tm_wday)) {
        repr_value = repr_mode == 'S' ? safe_elem(full_wday_names,attribval)
                                      : safe_elem(abbr_wday_names,attribval);
       } else {
        goto format_end;
       }
       print(repr_value,(size_t)-1);
      } else {
       static char const suffix_values[] = "st" "nd" "rd" "th";
       if (width) {
        if (repr_mode != ' ') printf("%0.*u",(unsigned int)width,attribval);
        else                  printf( "%.*u",(unsigned int)width,attribval);
       } else printf("%u",attribval);
       if (repr_mode == 'n') {
        unsigned int suffix_offset = (attribval >= 3 ? 3 : attribval)*2;
        print(suffix_values+suffix_offset,2);
       }
      }
     } break;
#endif /* STRFTIME_EXTENSION_LONGNAMES */

     default:
      format_begin = iter-2;
      goto next;
    }
format_end:
    format_begin = iter;
   } break;
   default: break;
  }
 }
end:
 if (format_begin != iter)
     print(format_begin,(size_t)(iter-format_begin));
#undef peekch
#undef readch
 return result;
}










INTERN int LIBCCALL
libc_stringprinter_init(struct stringprinter *__restrict self,
                        size_t hint) {
 CHECK_HOST_DOBJ(self);
 if (!hint) hint = 4*sizeof(void *);
 self->sp_buffer = (char *)libc_malloc((hint+1)*sizeof(char));
 if unlikely(!self->sp_buffer) return -1;
 self->sp_bufpos = self->sp_buffer;
 self->sp_bufend = self->sp_buffer+hint;
 self->sp_bufend[0] = '\0';
 return 0;
}
INTERN char *LIBCCALL
libc_stringprinter_pack(struct stringprinter *__restrict self,
                        size_t *length) {
 char *result; size_t result_size;
 CHECK_HOST_DOBJ(self);
 assert(self->sp_bufpos >= self->sp_buffer);
 assert(self->sp_bufpos <= self->sp_bufend);
 result_size = (size_t)(self->sp_bufpos-self->sp_buffer);
 if (self->sp_bufpos != self->sp_bufend) {
  result = (char *)libc_realloc(self->sp_buffer,(result_size+1)*sizeof(char));
  if unlikely(!result) result = self->sp_buffer;
 } else {
  result = self->sp_buffer;
 }
 result[result_size] = '\0';
 self->sp_buffer = NULL;
 if (length) *length = result_size;
 return result;
}
INTERN void LIBCCALL
libc_stringprinter_fini(struct stringprinter *__restrict self) {
 CHECK_HOST_DOBJ(self);
 libc_free(self->sp_buffer);
}
INTERN ssize_t LIBCCALL
libc_stringprinter_print(char const *__restrict data,
                         size_t datalen, void *closure) {
 struct stringprinter *self = (struct stringprinter *)closure;
 size_t size_avail,newsize,reqsize;
 char *new_buffer;
 CHECK_HOST_DOBJ(self);
 assert(self->sp_bufpos >= self->sp_buffer);
 assert(self->sp_bufpos <= self->sp_bufend);
 size_avail = (size_t)(self->sp_bufend-self->sp_bufpos);
 if unlikely(size_avail < datalen) {
  /* Must allocate more memory. */
  newsize = (size_t)(self->sp_bufend-self->sp_buffer);
  assert(newsize);
  reqsize = newsize+(datalen-size_avail);
  /* Double the buffer size until it is of sufficient length. */
  do newsize *= 2; while (newsize < reqsize);
  /* Reallocate the buffer (But include 1 character for the terminating '\0') */
  new_buffer = (char *)libc_realloc(self->sp_buffer,(newsize+1)*sizeof(char));
  if unlikely(!new_buffer) return -1;
  self->sp_bufpos = new_buffer+(self->sp_bufpos-self->sp_buffer);
  self->sp_bufend = new_buffer+newsize;
  self->sp_buffer = new_buffer;
 }
 libc_memcpy(self->sp_bufpos,data,datalen);
 self->sp_bufpos += datalen;
 assert(self->sp_bufpos <= self->sp_bufend);
 return 0;
}
#endif /* !__KERNEL__ */

DEFINE_PUBLIC_ALIAS(format_vprintf,libc_format_vprintf);
DEFINE_PUBLIC_ALIAS(format_printf,libc_format_printf);
DEFINE_PUBLIC_ALIAS(format_quote,libc_format_quote);
DEFINE_PUBLIC_ALIAS(format_hexdump,libc_format_hexdump);
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(format_scanf,libc_format_scanf);
DEFINE_PUBLIC_ALIAS(format_vscanf,libc_format_vscanf);
DEFINE_PUBLIC_ALIAS(format_strftime,libc_format_strftime);
DEFINE_PUBLIC_ALIAS(stringprinter_init,libc_stringprinter_init);
DEFINE_PUBLIC_ALIAS(stringprinter_pack,libc_stringprinter_pack);
DEFINE_PUBLIC_ALIAS(stringprinter_fini,libc_stringprinter_fini);
DEFINE_PUBLIC_ALIAS(stringprinter_print,libc_stringprinter_print);
#endif /* !__KERNEL__ */

DECL_END

#pragma GCC diagnostic pop

#endif /* !GUARD_LIBS_LIBC_FORMAT_PRINTER_C */
