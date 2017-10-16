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
#include "malloc.h"

#include <alloca.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
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
#include <hybrid/section.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef __KERNEL__
#include <fs/fs.h>
#include <fs/file.h>
#include <fs/dentry.h>
#else
#include "unistd.h"
#include "unicode.h"
#include "errno.h"
#include <stdio.h>
#include <kos/fcntl.h>
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
#define PRINTF_EXTENSION_UNICODE     1        /*< Add 'U16' and 'U32' length modifiers for utf16/32 respectively. */
#ifndef CONFIG_DEBUG
#undef PRINTF_EXTENSION_ERRORMSG
#define PRINTF_EXTENSION_ERRORMSG    0
#endif

#ifndef __KERNEL__
#undef PRINTF_EXTENSION_VIRTUALPTR
#define PRINTF_EXTENSION_VIRTUALPTR  0
#else
#undef PRINTF_EXTENSION_UNICODE
#define PRINTF_EXTENSION_UNICODE     0
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
#define __len_is64(x) ((x) == len_I64)
#define len_is64 __len_is64
#elif VA_SIZE == 2
 len_I16  = 0,
 len_I32  = 1,
 len_I64  = 2,
 len_I8   = len_I16,
#define __len_is64(x) ((x) == len_I64)
#define __len_is32(x) ((x) == len_I32)
#define len_is64 __len_is64
#define len_is32 __len_is32
#elif VA_SIZE == 1
 len_I8   = 0,
 len_I16  = 1,
 len_I32  = 2,
 len_I64  = 3,
#define __len_is64(x) ((x) == len_I64)
#define __len_is32(x) ((x) == len_I32)
#define __len_is16(x) ((x) == len_I16)
#define len_is64 __len_is64
#define len_is32 __len_is32
#define len_is16 __len_is16
#else
#   error FIXME
#endif
 len_L    = 'L',len_z = 'z',len_t = 't',
 len_I    = PP_CAT2(len_I,PP_MUL8(__SIZEOF_POINTER__)),
 len_hh   = PP_CAT2(len_I,PP_MUL8(__SIZEOF_CHAR__)),
 len_h    = PP_CAT2(len_I,PP_MUL8(__SIZEOF_SHORT__)),
#if PRINTF_EXTENSION_UNICODE
 len_U16  = 'u',
 len_U32  = 'U',
#endif
#ifdef __KERNEL__
 len_l    = PP_CAT2(len_I,PP_MUL8(__SIZEOF_LONG__)),
#else
 /* Need to separate 'l' for wide-string support. */
 len_l    = 'l',
#endif
 len_ll   = PP_CAT2(len_I,PP_MUL8(__SIZEOF_LONG_LONG__)),
 len_j    = len_I64, /* intmax_t */
};
#ifndef __KERNEL__
#if __SIZEOF_LONG__ == 1
#ifdef __len_is8
#   undef len_is8
#   define len_is8(x)  (__len_is8(x) || (x) == len_l)
#endif
#elif __SIZEOF_LONG__ == 2
#ifdef __len_is16
#   undef len_is16
#   define len_is16(x) (__len_is16(x) || (x) == len_l)
#endif
#elif __SIZEOF_LONG__ == 4
#ifdef __len_is32
#   undef len_is32
#   define len_is32(x) (__len_is32(x) || (x) == len_l)
#endif
#elif __SIZEOF_LONG__ == 8
#ifdef __len_is64
#   undef len_is64
#   define len_is64(x) (__len_is64(x) || (x) == len_l)
#endif
#endif
#endif /* !__KERNEL__ */

typedef union {
 s8 s_8; s16 s_16; s32 s_32; s64 s_64; s32 s_32_64[2];
 u8 u_8; u16 u_16; u32 u_32; u64 u_64; u32 u_32_64[2];
} fint_t;

#if VA_SIZE == 8
#define FINT_LOAD(arg,length,flags,args) \
  { arg.u_64 = va_arg(args,u64); }
#elif VA_SIZE == 4
#define FINT_LOAD(arg,length,flags,args) \
  { arg.u_64 = 0; \
    if (len_is64(length)) arg.u_64 = va_arg(args,u64); \
    else { arg.u_32 = va_arg(args,u32); \
    if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_32; } \
  }
#elif VA_SIZE == 2
#define FINT_LOAD(arg,length,flags,args) \
  {  arg.u_64 = 0; \
         if (len_is64(length)) arg.u_64 = va_arg(args,u64); \
    else if (len_is32(length)) { arg.u_32 = va_arg(args,u32); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_32; } \
    else { arg.u_16 = va_arg(args,u16); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_16; } \
  }
#else
#define FINT_LOAD(arg,length,flags,args) \
  { arg.u_64 = 0; \
         if (len_is64(length)) arg.u_64 = va_arg(args,u64); \
    else if (len_is32(length)) { arg.u_32 = va_arg(args,u32); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_32; } \
    else if (len_is16(length)) { arg.u_16 = va_arg(args,u16); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_16; } \
    else { arg.u_8 = va_arg(args,u8); if (flags&PRINTF_FLAG_SIGNED) arg.s_64 = (s64)arg.s_8; } \
  }
#endif


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

#if defined(__KERNEL__) || 1
#define HAVE_LONGPRINTER_MAC   /*< AA:BB:CC:DD:EE:FF */
#define HAVE_LONGPRINTER_IP    /*< "127.0.0.1" */
#endif
#define HAVE_LONGPRINTER_UNIT /*< 53+10*1024 == "~10Kb" */
#define HAVE_LONGPRINTER_ND    /*< 0 == "0"; 1 == "1st"; 2 == "2nd"; 3 == "3rd"; 4 == "4th" ... */



LONG_PRINTER(errno_printer);
LONG_PRINTER(dev_t_printer);
LONG_PRINTER(hex_printer);
#ifdef __KERNEL__
LONG_PRINTER(dentrypath_printer);
LONG_PRINTER(filepath_printer);
#else
LONG_PRINTER(fdpath_printer);
#endif


#ifdef HAVE_LONGPRINTER_MAC
LONG_PRINTER(mac_printer) {
 u8 *bytes = va_arg(*args,u8 *);
 char buffer[64],*iter = buffer; size_t n = 6;
 while (n--) iter += libc_sprintf(iter,"%.2I8X:",*bytes++);
 return (*printer)(buffer,(size_t)(iter-buffer)-1,closure);
}
#endif
#ifdef HAVE_LONGPRINTER_IP
LONG_PRINTER(ip_printer) {
 u8 *bytes = va_arg(*args,u8 *);
 char buffer[64],*iter = buffer; size_t n = 4;
 while (n--) iter += libc_sprintf(iter,"%I8d.",*bytes++);
 return (*printer)(buffer,(size_t)(iter-buffer)-1,closure);
}
#endif
#ifdef HAVE_LONGPRINTER_UNIT
LONG_PRINTER(unit_printer) {
 fint_t val; FINT_LOAD(val,length,flags,*args);
#if __BYTE_ORDER == __LITTLE_ENDIAN
 if (val.u_32_64[1])
#else
 if (val.u_32_64[0])
#endif
 {
  /* Value is larger than 32 bits. */
  u64 gb = val.u_64/(1024*1024*1024);
  return gb >= 8*1024 
   ? libc_format_printf(printer,closure,"%I64uTb",(u64)(gb/1024))
   : libc_format_printf(printer,closure,"%I64uGb",(u64)gb);
 }
 if (val.u_32 > 1024*1024*4)
     return libc_format_printf(printer,closure,"%I64uMb",(u64)(val.u_32/(1024*1024)));
 if (val.u_32 > 1024*2)
     return libc_format_printf(printer,closure,"%I64uKb",(u64)(val.u_32/1024));
 return libc_format_printf(printer,closure,"%I64uB",(u64)val.u_32);
}
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
#ifdef HAVE_LONGPRINTER_MAC
 {"mac",&mac_printer},
#endif
#ifdef HAVE_LONGPRINTER_IP
 {"ip",&ip_printer},
#endif
#ifdef HAVE_LONGPRINTER_UNIT
 {"unit",&unit_printer},
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

#if defined(CONFIG_LIBC_NO_DOS_LIBC) || defined(__KERNEL__)
INTERN ssize_t LIBCCALL
libc_format_vprintf(pformatprinter printer, void *closure,
                    char const *__restrict format, va_list args)
#else /* CONFIG_LIBC_NO_DOS_LIBC || __KERNEL__ */
INTERN ssize_t ATTR_CDECL
libc_dos_format_printf(pformatprinter printer, void *closure,
                       char const *__restrict format, ...) {
 va_list args; ssize_t result;
 va_start(args,format);
 result = libc_xformat_vprintf(true,printer,closure,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t LIBCCALL libc_dos_format_vprintf(pformatprinter printer, void *closure,
                                                char const *__restrict format, va_list args) {
 return libc_xformat_vprintf(true,printer,closure,format,args);
}
INTERN ssize_t LIBCCALL libc_format_vprintf(pformatprinter printer, void *closure,
                                            char const *__restrict format, va_list args) {
 return libc_xformat_vprintf(false,printer,closure,format,args);
}
INTERN ssize_t LIBCCALL
libc_xformat_vprintf(bool wch16, pformatprinter printer, void *closure,
                     char const *__restrict format, va_list args)
#endif /* !CONFIG_LIBC_NO_DOS_LIBC && !__KERNEL__ */
{
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
   enum printf_length length; u16 flags;
   size_t width,precision,print_width;
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
#if PRINTF_EXTENSION_UNICODE
   case 'U':
    ch = *format++;
    /* */if (ch == '1' && *format == '6') ++format,length = len_U16;
    else if (ch == '3' && *format == '2') ++format,length = len_U32;
    else { --format; goto broken_format; }
    goto nextmod;
#endif


#if PRINTF_EXTENSION_POINTERSIZE || PRINTF_EXTENSION_FIXLENGTH
   case 'I':
#if PRINTF_EXTENSION_FIXLENGTH
    ch = *format++;
    /* */if (ch == '8') length = len_I8;
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
    size_t bufsize;
    fint_t arg;
    if (0) { case 'B': flags |= PRINTF_FLAG_UPPER; case 'b': numsys = 2; }
    if (0) { case 'o': numsys = 8; }
    if (0) { case 'u': numsys = 10; if unlikely(length == len_t) flags |= PRINTF_FLAG_SIGNED; }
    if (0) { case 'd': case 'i': numsys = 10; if likely(length != len_z) flags |= PRINTF_FLAG_SIGNED; }
    if (0) { case 'p': if (!(flags&PRINTF_FLAG_HASPREC)) { precision = sizeof(void *)*2; flags |= PRINTF_FLAG_HASPREC; }
#if __SIZEOF_POINTER__ > VA_SIZE
                       if (!length) length = len_I;
#endif
             case 'X': flags |= PRINTF_FLAG_UPPER;
             case 'x': numsys = 16; if unlikely(length == len_t) flags |= PRINTF_FLAG_SIGNED; }
    FINT_LOAD(arg,length,flags,args);
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
    //libc_memset(buf,0xcc,sizeof(buf));
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
    /* XXX: This is wrong for different encodings... */
    if (!s) s = PRINTF_EXTENSION_NULLSTRING;
#endif
    if (!(flags&PRINTF_FLAG_HASPREC)) precision = (size_t)-1;
#ifdef PRINTF_FLAG_FIXBUF
    if (!(flags&PRINTF_FLAG_FIXBUF))
#endif /* PRINTF_FLAG_FIXBUF */
    {
#ifndef __KERNEL__
     if (length == len_l) {
#ifndef CONFIG_LIBC_NO_DOS_LIBC
      precision = unlikely(wch16) ? libc_16wcsnlen((char16_t *)s,precision)
                                  : libc_32wcsnlen((char32_t *)s,precision);
#else
      precision = libc_32wcsnlen((char32_t *)s,precision);
#endif
     } else
#endif
#if PRINTF_EXTENSION_UNICODE
     if (length == len_U16)
      precision = libc_16wcsnlen((char16_t *)s,precision);
     else if (length == len_U32)
      precision = libc_32wcsnlen((char32_t *)s,precision);
     else
#endif
     {
      
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
#ifndef __KERNEL__
     if (length == len_l) {
      mbstate_t state = MBSTATE_INIT;
      temp = unlikely(wch16) ? libc_format_16wsztomb(printer,closure,(char16_t *)s,precision,&state)
                             : libc_format_32wsztomb(printer,closure,(char32_t *)s,precision,&state);
     } else
#endif
#if PRINTF_EXTENSION_UNICODE
     if (length == len_U16) {
      mbstate_t state = MBSTATE_INIT;
      temp = libc_format_16wsztomb(printer,closure,(char16_t *)s,precision,&state);
     } else if (length == len_U32) {
      mbstate_t state = MBSTATE_INIT;
      temp = libc_format_32wsztomb(printer,closure,(char32_t *)s,precision,&state);
     } else
#endif
     { temp = (*printer)(s,precision,closure); }
     if (temp < 0) return temp;
     result += temp;
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

   case '%':
    flush_start = format-1;
    goto next_normal;
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
#if PRINTF_EXTENSION_LONGDESCR
broken_format2:
#endif /* PRINTF_EXTENSION_LONGDESCR */
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
    encoded_text[1] = (char)ch;
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








INTERN int LIBCCALL
libc_stringprinter_init(struct stringprinter *__restrict self,
                        size_t hint) {
 CHECK_HOST_DOBJ(self);
 if (!hint) hint = 4*sizeof(void *);
 self->sp_buffer = (char *)libc_malloc((hint+1)*sizeof(char));
 if unlikely(!self->sp_buffer) {
  self->sp_bufpos = NULL;
  self->sp_buffer = NULL;
  return -1;
 }
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
 libc_memcpy(self->sp_bufpos,data,datalen*sizeof(char));
 self->sp_bufpos += datalen;
 assert(self->sp_bufpos <= self->sp_bufend);
 return 0;
}














#ifndef __KERNEL__
INTERN ssize_t ATTR_CDECL
libc_format_scanf(pformatgetc pgetc, pformatungetc pungetc,
                  void *closure, char const *__restrict format, ...) {
 va_list args; ssize_t result;
 va_start(args,format);
 result = libc_format_vscanf(pgetc,pungetc,closure,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t LIBCCALL
libc_format_vscanf(pformatgetc pgetc, pformatungetc pungetc,
                   void *closure, char const *__restrict format,
                   va_list args) {
 CHECK_HOST_TEXT(pgetc,1);
 CHECK_HOST_TEXT(pungetc,1);
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



#define BUFFER_FLUSHTRUNC 256
#define BUFFER_PRINTLIMIT 4096
#define BUFFER_SIZEALIGN  64

#define BUFFER_USED(x)   (size_t)((x)->b_bufpos-(x)->b_buffer)
#define BUFFER_SIZE(x)   (size_t)((x)->b_bufend-(x)->b_buffer)
#define BUFFER_UNUSED(x) (size_t)((x)->b_bufend-(x)->b_bufpos)
#define BUFFER_REMAIN(x) (size_t)(BUFFER_PRINTLIMIT-BUFFER_SIZE(x))

/* Process a given printer return value into the state of the given buffer. */
#define BUFFER_ADDSTATE(x,y) \
  (unlikely((y) < 0) ? (x)->b_state = (y) : \
   unlikely(__builtin_add_overflow((x)->b_state,(y),&(x)->b_state)) \
 ? (x)->b_state = SSIZE_MAX : 0)

INTERN void LIBCCALL
libc_buffer_init(struct buffer *__restrict self,
                 pformatprinter printer, void *closure) {
 self->b_printer = printer;
 self->b_closure = closure;
 self->b_state   = 0;
 self->b_buffer  = NULL;
 self->b_bufpos  = NULL;
 self->b_bufend  = NULL;
}
INTERN ssize_t LIBCCALL
libc_buffer_fini(struct buffer *__restrict buf) {
 ssize_t result = BUFFER_USED(buf);
 /* print remaining data if there is some and no error occurred. */
 if (result && buf->b_state >= 0) {
  result = (*buf->b_printer)(buf->b_buffer,(size_t)result,
                             buf->b_closure);
  if likely(result >= 0) result += buf->b_state;
 }
 /* Free the allocate data buffer. */
 libc_free(buf->b_buffer);
 return result;
}
INTERN ssize_t LIBCCALL
libc_buffer_flush(struct buffer *__restrict buf) {
 ssize_t result;
 /* Check for an error. */
 if (buf->b_state < 0)
     return buf->b_state;
 /* Check if unprinted data is available. */
 result = BUFFER_USED(buf);
 if (result) {
  result = (*buf->b_printer)(buf->b_buffer,(size_t)result,
                             buf->b_closure);
  BUFFER_ADDSTATE(buf,result);
 }
 /* Reset the buffer position. */
 buf->b_bufpos = buf->b_buffer;
 /* If the buffer was quite large, delete it.
  * Otherwise, keep it around so it can be re-used the next time print is called.
  * >> This way, we keep a small memory footprint, and optimize ourself
  *    for the intended use-case of the user printing large amounts of
  *    data through small chunks. */
 if (BUFFER_SIZE(buf) > BUFFER_FLUSHTRUNC) {
  libc_free(buf->b_buffer);
  buf->b_buffer = NULL;
  buf->b_bufpos = NULL;
  buf->b_bufend = NULL;
 }
 return result;
}

INTERN ssize_t LIBCCALL
libc_buffer_print(char const *__restrict data,
                  size_t datalen, void *closure) {
 /* The heart of the buffer: Its printer callback. */
 struct buffer *buf = (struct buffer *)closure;
 ssize_t result,temp; size_t copy_size;
 assert(buf);
 assert(buf->b_bufpos >= buf->b_buffer);
 assert(buf->b_bufpos <= buf->b_bufend);
 assert(buf->b_buffer <= buf->b_bufend);
 CHECK_HOST_TEXT(data,datalen);
 /* Return the number of written bytes by default. */
 result = (ssize_t)datalen;
 /* Unlikely: Handle large amounts of data. */
 if unlikely(datalen >= BUFFER_PRINTLIMIT) {
  /* Make sure to flush existing data to preserve print order. */
  temp = libc_buffer_flush(buf);
  if unlikely(temp < 0) return temp;
  /* Now print this huge block of data. */
  temp = (*buf->b_printer)(data,datalen,buf->b_closure);
  /* Add the state to what has already been printed. */
  BUFFER_ADDSTATE(buf,temp);
  if (temp < 0) return temp;
  goto end;
 }

again:
 /* Fill unused data. */
 copy_size = MIN(BUFFER_UNUSED(buf),datalen);
 /*if (copy_size)*/
 {
  libc_memcpy(buf->b_bufpos,data,copy_size*sizeof(char));
  data          += copy_size;
  datalen       -= copy_size;
  buf->b_bufpos += copy_size;
 }

 /* Allocate more memory. */
 copy_size = MIN(BUFFER_REMAIN(buf),datalen);
 if (copy_size) {
  size_t new_size; char *new_buffer;
  new_size = BUFFER_SIZE(buf)+copy_size;
  assert(new_size <= BUFFER_PRINTLIMIT);
  new_size = CEIL_ALIGN(new_size,BUFFER_SIZEALIGN);
  assert(new_size <= BUFFER_PRINTLIMIT);
  new_buffer = (char *)libc_realloc(buf->b_buffer,new_size*sizeof(char));
  if unlikely(!new_buffer) {
   /* Special case: Flush the existing buffer to get more unused data. */
   temp = libc_buffer_flush(buf);
   if (temp < 0) return temp;
   assert(buf->b_bufpos == buf->b_buffer);
   if (datalen < BUFFER_UNUSED(buf)) {
    /* Simple case: We can copy the remainder into the currently empty buffer. */
    libc_memcpy(buf->b_bufpos,data,datalen*sizeof(char));
    buf->b_bufpos += datalen;
    goto end;
   }
   goto again;
  }
  buf->b_bufpos = new_buffer+(buf->b_bufpos-buf->b_buffer);
  buf->b_bufend = new_buffer+new_size;
  buf->b_buffer = new_buffer;
  /* Copy the data we've just allocated memory for. */
  libc_memcpy(buf->b_bufpos,data,copy_size*sizeof(char));
  data          += copy_size;
  datalen       -= copy_size;
  buf->b_bufpos += copy_size;
 }

 /* At this point, we know that we're either done, or that the buffer is full. */
 assert(datalen < BUFFER_PRINTLIMIT); /* Already assured at the start. */
 if (datalen) {
  /* NOTE: Before calling the printer, make sure we're not in an error state. */
  if (buf->b_state < 0) return buf->b_state;
  assert(BUFFER_USED(buf) == BUFFER_PRINTLIMIT);
  assert(BUFFER_SIZE(buf) == BUFFER_PRINTLIMIT);
  /* If the buffer is full, print it. */
  temp = (*buf->b_printer)(buf->b_buffer,BUFFER_PRINTLIMIT,
                           buf->b_closure);
  BUFFER_ADDSTATE(buf,temp);
  if (temp < 0) return temp;
  /* Copy all the unprinted data into the (now) empty buffer. */
  libc_memcpy(buf->b_buffer,data,datalen*sizeof(char));
  buf->b_bufpos = buf->b_bufpos+datalen;
 }
end:
 return result;
}

INTERN ssize_t LIBCCALL
libc_format_vbprintf(pformatprinter printer, void *closure,
                     char const *__restrict format, va_list args) {
 struct buffer buf;
 libc_buffer_init(&buf,printer,closure);
 libc_format_vprintf(&libc_buffer_print,&buf,format,args);
 return libc_buffer_fini(&buf);
}
INTERN ssize_t ATTR_CDECL
libc_format_bprintf(pformatprinter printer, void *closure,
                    char const *__restrict format, ...) {
 va_list args; ssize_t result;
 va_start(args,format);
 result = libc_format_vbprintf(printer,closure,format,args);
 va_end(args);
 return result;
}


INTERN void LIBCCALL
libc_16wprinter_fini(struct c16printer *__restrict wp) {
 libc_free(wp->p_buffer);
}
INTERN void LIBCCALL
libc_16wprinter_init(struct c16printer *__restrict wp,
                     pc16formatprinter printer, void *closure) {
 wp->p_printer = printer;
 wp->p_closure = closure;
 wp->p_buffer  = NULL;
 wp->p_buflen  = 0;
 mbstate_reset(&wp->p_mbstate);
}
#if 1
DEFINE_INTERN_ALIAS(libc_32wprinter_fini,libc_16wprinter_fini);
DEFINE_INTERN_ALIAS(libc_32wprinter_init,libc_16wprinter_init);
#else
INTERN void LIBCCALL
libc_32wprinter_fini(struct c32printer *__restrict wp) {
 libc_free(wp->p_buffer);
}
INTERN void LIBCCALL
libc_32wprinter_init(struct c32printer *__restrict wp,
                     pc32formatprinter printer, void *closure) {
 wp->p_printer = printer;
 wp->p_closure = closure;
 wp->p_buffer  = NULL;
 wp->p_buflen  = 0;
 mbstate_reset(&wp->p_mbstate);
}
#endif

#define PRINTER_MAXBUF16  (4096/2)
#define PRINTER_MAXBUF32  (4096/4)

INTERN ssize_t LIBCCALL
libc_16wprinter_print(char const *__restrict data,
                      size_t datalen, void *closure) {
 char16_t minbuf[1]; ssize_t result = 0,temp;
 struct c16printer *wp = (struct c16printer *)closure;
 char16_t *buf = wp->p_buffer;
 for (;;) {
  temp = MIN(datalen,PRINTER_MAXBUF16);
  if (!temp) break;
  /* Allocate a buffer that should always be of sufficient size. */
  if ((size_t)temp > wp->p_buflen) {
   char16_t *new_buffer; size_t newsize = CEIL_ALIGN((size_t)temp,16);
   new_buffer = (char16_t *)libc_realloc(wp->p_buffer,newsize*sizeof(char16_t));
   if unlikely(!new_buffer) {
    buf  = minbuf;
    temp = COMPILER_LENOF(minbuf);
   } else {
    wp->p_buffer = buf = new_buffer;
    wp->p_buflen = newsize;
   }
  }
  assert(datalen);
  /* Convert text to utf16. */
  temp = (ssize_t)libc_utf8to16((char const *)&data,(size_t)&datalen,
                                 buf,wp->p_buflen,&wp->p_mbstate,
                                 UNICODE_F_NOZEROTERM|UNICODE_F_UPDATESRC|
                                 UNICODE_F_SETERRNO);
  if unlikely((size_t)temp == UNICODE_ERROR) return temp;
  /* Call the underlying pointer. */
  temp = (*wp->p_printer)(buf,(size_t)temp,wp->p_closure);
  /* Forward print errors. */
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}
INTERN ssize_t LIBCCALL
libc_32wprinter_print(char const *__restrict data,
                      size_t datalen, void *closure) {
 char32_t minbuf[1]; ssize_t result = 0,temp;
 struct c32printer *wp = (struct c32printer *)closure;
 char32_t *buf = wp->p_buffer;

 for (;;) {
  temp = MIN(datalen,PRINTER_MAXBUF32);
  if (!temp) break;
  /* Allocate a buffer that should always be of sufficient size. */
  if ((size_t)temp > wp->p_buflen) {
   char32_t *new_buffer; size_t newsize = CEIL_ALIGN((size_t)temp,16);
   new_buffer = (char32_t *)libc_realloc(wp->p_buffer,newsize*sizeof(char32_t));
   if unlikely(!new_buffer) {
    buf  = minbuf;
    temp = COMPILER_LENOF(minbuf);
   } else {
    wp->p_buffer = buf = new_buffer;
    wp->p_buflen = newsize;
   }
  }
  assert(datalen);
  /* Convert text to utf16. */
  temp = (ssize_t)libc_utf8to32((char const *)&data,(size_t)&datalen,
                                 buf,wp->p_buflen,&wp->p_mbstate,
                                 UNICODE_F_NOZEROTERM|UNICODE_F_UPDATESRC|
                                 UNICODE_F_SETERRNO);
  if unlikely((size_t)temp == UNICODE_ERROR) return temp;
  /* Call the underlying pointer. */
  temp = (*wp->p_printer)(buf,(size_t)temp,wp->p_closure);
  /* Forward print errors. */
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}
#endif /* !__KERNEL__ */


DEFINE_PUBLIC_ALIAS(format_vprintf,libc_format_vprintf);
DEFINE_PUBLIC_ALIAS(format_printf,libc_format_printf);
DEFINE_PUBLIC_ALIAS(format_quote,libc_format_quote);
DEFINE_PUBLIC_ALIAS(format_hexdump,libc_format_hexdump);
DEFINE_PUBLIC_ALIAS(stringprinter_init,libc_stringprinter_init);
DEFINE_PUBLIC_ALIAS(stringprinter_pack,libc_stringprinter_pack);
DEFINE_PUBLIC_ALIAS(stringprinter_fini,libc_stringprinter_fini);
DEFINE_PUBLIC_ALIAS(stringprinter_print,libc_stringprinter_print);
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(format_scanf,libc_format_scanf);
DEFINE_PUBLIC_ALIAS(format_vscanf,libc_format_vscanf);
DEFINE_PUBLIC_ALIAS(format_strftime,libc_format_strftime);
#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(__DSYM(format_printf),libc_dos_format_printf);
DEFINE_PUBLIC_ALIAS(__DSYM(format_vprintf),libc_dos_format_vprintf);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
DEFINE_PUBLIC_ALIAS(buffer_init,libc_buffer_init);
DEFINE_PUBLIC_ALIAS(buffer_fini,libc_buffer_fini);
DEFINE_PUBLIC_ALIAS(buffer_flush,libc_buffer_flush);
DEFINE_PUBLIC_ALIAS(buffer_print,libc_buffer_print);
#endif /* !__KERNEL__ */

#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(format_bprintf,libc_format_bprintf);
DEFINE_PUBLIC_ALIAS(format_vbprintf,libc_format_vbprintf);
DEFINE_PUBLIC_ALIAS(format_wcsztomb,libc_format_32wsztomb);
DEFINE_PUBLIC_ALIAS(format_wcsntomb,libc_format_32wsntomb);
DEFINE_PUBLIC_ALIAS(__DSYM(format_wcsztomb),libc_format_16wsztomb);
DEFINE_PUBLIC_ALIAS(__DSYM(format_wcsntomb),libc_format_16wsntomb);
#endif



PRIVATE ssize_t LIBCCALL
libc_sprintf_callback(char const *__restrict data, size_t datalen,
                      void *buffer) {
 libc_memcpy(*(char **)buffer,data,datalen*sizeof(char));
 *(char **)buffer += datalen;
 return datalen;
}

INTERN size_t LIBCCALL
libc_vsprintf(char *__restrict buf, char const *__restrict format, va_list args) {
 size_t result = (size_t)libc_format_vprintf(&libc_sprintf_callback,
                                             (void *)&buf,format,args);
 /* Random fact: Forgetting to terminate the string here
  *              breaks tab-completion in busybox's ash... */
 return (*buf = '\0',result);
}

struct snprintf_data { char *bufpos,*bufend; };
PRIVATE ssize_t LIBCCALL
snprintf_callback(char const *__restrict data, size_t datalen, void *closure) {
#define BUFFER ((struct snprintf_data *)closure)
 /* Don't exceed the buffer end */
 if (BUFFER->bufpos < BUFFER->bufend) {
  size_t maxwrite = (size_t)(BUFFER->bufend-BUFFER->bufpos);
  libc_memcpy(BUFFER->bufpos,data,
              MIN(maxwrite,datalen)*sizeof(char));
 }
 /* Still seek past the end, as to
  * calculate the required buffersize. */
 BUFFER->bufpos += datalen;
 return datalen;
#undef BUFFER
}

INTERN size_t LIBCCALL
libc_vsnprintf(char *__restrict buf, size_t buflen,
               char const *__restrict format, va_list args) {
 struct snprintf_data data; data.bufpos = buf;
 if (__builtin_add_overflow((uintptr_t)buf,buflen,(uintptr_t *)&data.bufend))
     data.bufend = (char *)(uintptr_t)-1;
 libc_format_vprintf(&snprintf_callback,&data,format,args);
 if likely(data.bufpos < data.bufend) *data.bufpos = '\0';
 return (size_t)(data.bufpos-buf);
}

INTERN size_t ATTR_CDECL
libc_sprintf(char *__restrict buf, char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 result = libc_vsprintf(buf,format,args);
 va_end(args);
 return result;
}
INTERN size_t ATTR_CDECL
libc_snprintf(char *__restrict buf, size_t maxlen, char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 result = libc_vsnprintf(buf,maxlen,format,args);
 va_end(args);
 return result;
}

/* Always export this set of printer functions (meaning they're also available within the kernel) */
DEFINE_PUBLIC_ALIAS(vsprintf,libc_vsprintf);
DEFINE_PUBLIC_ALIAS(vsnprintf,libc_vsnprintf);
DEFINE_PUBLIC_ALIAS(sprintf,libc_sprintf);
DEFINE_PUBLIC_ALIAS(snprintf,libc_snprintf);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN size_t LIBCCALL
libc_dos_vsprintf(char *__restrict buf, char const *__restrict format, va_list args) {
 size_t result = (size_t)libc_dos_format_vprintf(&libc_sprintf_callback,
                                                (void *)&buf,format,args);
 return (*buf = '\0',result);
}
INTERN size_t LIBCCALL
libc_dos_vsnprintf(char *__restrict buf, size_t buflen,
                   char const *__restrict format, va_list args) {
 struct snprintf_data data; data.bufpos = buf;
 if (__builtin_add_overflow((uintptr_t)buf,buflen,(uintptr_t *)&data.bufend))
     data.bufend = (char *)(uintptr_t)-1;
 libc_dos_format_vprintf(&snprintf_callback,&data,format,args);
 if likely(data.bufpos < data.bufend) *data.bufpos = '\0';
 return (size_t)(data.bufpos-buf);
}
INTERN size_t ATTR_CDECL
libc_dos_sprintf(char *__restrict buf, char const *__restrict format, ...) {
 va_list args; size_t result; va_start(args,format);
 result = libc_dos_vsprintf(buf,format,args);
 va_end(args);
 return result;
}
INTERN size_t ATTR_CDECL
libc_dos_snprintf(char *__restrict buf, size_t buflen,
                  char const *__restrict format, ...) {
 va_list args; size_t result; va_start(args,format);
 result = libc_dos_vsnprintf(buf,buflen,format,args);
 va_end(args);
 return result;
}
DEFINE_PUBLIC_ALIAS(__DSYM(vsprintf),libc_vsprintf);
DEFINE_PUBLIC_ALIAS(__DSYM(vsnprintf),libc_vsnprintf);
DEFINE_PUBLIC_ALIAS(__DSYM(sprintf),libc_sprintf);
DEFINE_PUBLIC_ALIAS(__DSYM(snprintf),libc_snprintf);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


#ifndef __KERNEL__
PRIVATE ssize_t LIBCCALL libc_sscanf_getc(void *data) { return *(*(char **)data)++; }
PRIVATE ssize_t LIBCCALL libc_sscanf_ungetc(int UNUSED(c), void *data) { --*(char **)data; return 0; }
INTERN size_t LIBCCALL
libc_vsscanf(char const *__restrict src,
             char const *__restrict format,
             va_list args) {
 return libc_format_vscanf(&libc_sscanf_getc,
                           &libc_sscanf_ungetc,
                          (void *)&src,format,args);
}
INTERN size_t ATTR_CDECL
libc_sscanf(char const *__restrict src, char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 result = libc_vsscanf(src,format,args);
 va_end(args);
 return result;
}

PRIVATE ssize_t LIBCCALL
vdprintf_callback(char const *__restrict data, size_t datalen, void *fd) {
 return libc_write((int)(uintptr_t)fd,data,datalen);
}
INTERN ssize_t LIBCCALL libc_vdprintf(int fd, char const *__restrict format, va_list args) {
#if 1
 /* Use buffered format printing, because this function
  * interfaces directly with the underlying system call. */
 return libc_format_vbprintf(&vdprintf_callback,(void *)(uintptr_t)fd,format,args);
#else
 return libc_format_vprintf(&vdprintf_callback,(void *)(uintptr_t)fd,format,args);
#endif
}
INTERN ssize_t ATTR_CDECL libc_dprintf(int fd, char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vdprintf(fd,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_RARETEXT ssize_t
LIBCCALL libc_vasprintf(char **__restrict ptr,
                        char const *__restrict format,
                        va_list args) {
 *ptr = libc_vstrdupf(format,args);
 return *ptr ? (ssize_t)libc_strlen(*ptr) : (ssize_t)-1;
}
INTERN ATTR_RARETEXT ssize_t ATTR_CDECL
libc_asprintf(char **__restrict ptr, char const *__restrict format, ...) {
 va_list args; ssize_t result;
 va_start(args,format);
 result = libc_vasprintf(ptr,format,args);
 va_end(args);
 return result;
}

DEFINE_PUBLIC_ALIAS(vsscanf,libc_vsscanf);
DEFINE_PUBLIC_ALIAS(sscanf,libc_sscanf);
DEFINE_PUBLIC_ALIAS(vdprintf,libc_vdprintf);
DEFINE_PUBLIC_ALIAS(dprintf,libc_dprintf);
DEFINE_PUBLIC_ALIAS(vasprintf,libc_vasprintf);
DEFINE_PUBLIC_ALIAS(asprintf,libc_asprintf);
DEFINE_PUBLIC_ALIAS(__asprintf,libc_asprintf);


/* UTF-32 Wide string printing/scanning */
struct sw32printf_data { char32_t *bufpos,*bufend; };
PRIVATE ATTR_RARETEXT ssize_t LIBCCALL
libc_sw32printf_callback(char32_t const *__restrict data,
                         size_t datalen, void *closure) {
#define BUFFER ((struct sw32printf_data *)closure)
 /* Don't exceed the buffer end */
 if (BUFFER->bufpos < BUFFER->bufend) {
  size_t maxwrite = (size_t)(BUFFER->bufend-BUFFER->bufpos);
  libc_memcpy(BUFFER->bufpos,data,MIN(maxwrite,datalen)*sizeof(char32_t));
 }
 /* Still seek past the end, as to
  * calculate the required buffersize. */
 BUFFER->bufpos += datalen;
 return datalen;
#undef BUFFER
}

INTERN ATTR_RARETEXT ssize_t LIBCCALL
libc_32vswprintf(char32_t *__restrict buf, size_t buflen,
                 char32_t const *__restrict format, va_list args) {
 ssize_t result = -1; char *utf8_format;
 /* Convert 'format' to utf8. */
 utf8_format = libc_utf32to8m(format);
 if likely(utf8_format) {
  struct c32printer printer;
  struct sw32printf_data data; data.bufpos = buf;
  if (__builtin_add_overflow((uintptr_t)buf,buflen,(uintptr_t *)&data.bufend))
      data.bufend = (char32_t *)(uintptr_t)-1;
  /* Use a 'c32printer' to convert format_printf() output to utf-32. */
  libc_32wprinter_init(&printer,&libc_sw32printf_callback,&data);
  /* Invoke the printer chain we're just created. */
  result = libc_format_vprintf(&libc_32wprinter_print,&printer,utf8_format,args);
  libc_32wprinter_fini(&printer);
  /* Ensure ZERO-termination. */
  if (result >= 0) {
   if likely(data.bufpos < data.bufend)
            *data.bufpos = (char32_t)'\0';
   result = (ssize_t)(data.bufpos-buf);
  }
  libc_free(utf8_format);
 }
 return result;
}

PRIVATE ATTR_RARETEXT ssize_t LIBCCALL libc_32sscanf_getc(void *data) { return (char)*(*(char32_t **)data)++; }
PRIVATE ATTR_RARETEXT ssize_t LIBCCALL libc_32sscanf_ungetc(int UNUSED(c), void *data) { --*(char32_t **)data; return 0; }
INTERN ATTR_RARETEXT ssize_t LIBCCALL
libc_32vswscanf(char32_t const *__restrict src,
                char32_t const *__restrict format, va_list args) {
 ssize_t result = -1; char *utf8_format;
 if ((utf8_format = libc_utf32to8m(format)) != NULL) {
  result = libc_format_vscanf(&libc_32sscanf_getc,
                              &libc_32sscanf_ungetc,
                             (void *)&src,utf8_format,args);
  libc_free(utf8_format);
 }
 return result;
}
INTERN ATTR_RARETEXT ssize_t ATTR_CDECL
libc_32swprintf(char32_t *__restrict buf, size_t buflen,
                char32_t const *__restrict format, ...) {
 ssize_t result; va_list args; va_start(args,format);
 result = libc_32vswprintf(buf,buflen,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_RARETEXT ssize_t ATTR_CDECL
libc_32swscanf(char32_t const *__restrict src,
               char32_t const *__restrict format, ...) {
 ssize_t result; va_list args; va_start(args,format);
 result = libc_32vswscanf(src,format,args);
 va_end(args);
 return result;
}

DEFINE_PUBLIC_ALIAS(vswprintf,libc_32vswprintf);
DEFINE_PUBLIC_ALIAS(swprintf,libc_32swprintf);
DEFINE_PUBLIC_ALIAS(vswscanf,libc_32vswscanf);
DEFINE_PUBLIC_ALIAS(swscanf,libc_32swscanf);


#ifndef CONFIG_LIBC_NO_DOS_LIBC
/* UTF-16 Wide string printing/scanning */
struct sw16printf_data { char16_t *bufpos,*bufend; };
PRIVATE ATTR_DOSTEXT ssize_t LIBCCALL
libc_sw16printf_callback(char16_t const *__restrict data,
                         size_t datalen, void *closure) {
#define BUFFER ((struct sw16printf_data *)closure)
 /* Don't exceed the buffer end */
 if (BUFFER->bufpos < BUFFER->bufend) {
  size_t maxwrite = (size_t)(BUFFER->bufend-BUFFER->bufpos);
  libc_memcpy(BUFFER->bufpos,data,MIN(maxwrite,datalen)*sizeof(char16_t));
 }
 /* Still seek past the end, as to
  * calculate the required buffersize. */
 BUFFER->bufpos += datalen;
 return datalen;
#undef BUFFER
}

INTERN ATTR_DOSTEXT ssize_t LIBCCALL
libc_dos_16vswprintf(char16_t *__restrict buf, size_t buflen,
                     char16_t const *__restrict format, va_list args) {
 ssize_t result = -1; char *utf8_format;
 /* Convert 'format' to utf8. */
 utf8_format = libc_utf16to8m(format);
 if likely(utf8_format) {
  struct c16printer printer;
  struct sw16printf_data data; data.bufpos = buf;
  if (__builtin_add_overflow((uintptr_t)buf,buflen,(uintptr_t *)&data.bufend))
      data.bufend = (char16_t *)(uintptr_t)-1;
  /* Use a 'c16printer' to convert format_printf() output to utf-16. */
  libc_16wprinter_init(&printer,&libc_sw16printf_callback,&data);
  /* Invoke the printer chain we're just created. */
  result = libc_dos_format_vprintf(&libc_16wprinter_print,&printer,utf8_format,args);
  libc_16wprinter_fini(&printer);
  /* Ensure ZERO-termination. */
  if (result >= 0) {
   if likely(data.bufpos < data.bufend)
            *data.bufpos = (char16_t)'\0';
   result = (ssize_t)(data.bufpos-buf);
  }
  libc_free(utf8_format);
 }
 return result;
}

PRIVATE ATTR_DOSTEXT ssize_t LIBCCALL libc_16sscanf_getc(void *data) { return (char)*(*(char16_t **)data)++; }
PRIVATE ATTR_DOSTEXT ssize_t LIBCCALL libc_16sscanf_ungetc(int UNUSED(c), void *data) { --*(char16_t **)data; return 0; }
INTERN ATTR_DOSTEXT ssize_t LIBCCALL
libc_16vswscanf(char16_t const *__restrict src,
                char16_t const *__restrict format, va_list args) {
 ssize_t result = -1; char *utf8_format;
 if ((utf8_format = libc_utf16to8m(format)) != NULL) {
  result = libc_format_vscanf(&libc_16sscanf_getc,
                              &libc_16sscanf_ungetc,
                             (void *)&src,utf8_format,args);
  libc_free(utf8_format);
 }
 return result;
}
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL
libc_dos_16swprintf(char16_t *__restrict buf, size_t buflen,
                    char16_t const *__restrict format, ...) {
 ssize_t result; va_list args; va_start(args,format);
 result = libc_dos_16vswprintf(buf,buflen,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL
libc_16swscanf(char16_t const *__restrict src,
               char16_t const *__restrict format, ...) {
 ssize_t result; va_list args; va_start(args,format);
 result = libc_16vswscanf(src,format,args);
 va_end(args);
 return result;
}

DEFINE_PUBLIC_ALIAS(_vswprintf_c,libc_dos_16vswprintf);
DEFINE_PUBLIC_ALIAS(_swprintf_c,libc_dos_16swprintf);
DEFINE_PUBLIC_ALIAS(__DSYM(vswscanf),libc_16vswscanf);
DEFINE_PUBLIC_ALIAS(__DSYM(swscanf),libc_16swscanf);

INTERN ATTR_DOSTEXT ssize_t LIBCCALL
libc_dos_16vswprintf_unlimited(char16_t *__restrict buf,
                               char16_t const *__restrict format,
                               va_list args) {
 return libc_dos_16vswprintf(buf,(size_t)-1,format,args);
}
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL
libc_dos_16swprintf_unlimited(char16_t *__restrict buf, char16_t const *__restrict format, ...) {
 va_list args; ssize_t result; va_start(args,format);
 result = libc_dos_16vswprintf_unlimited(buf,format,args);
 va_end(args);
 return result;
}
/* We only export these two symbols for binary compatibility with DOS. */
DEFINE_PUBLIC_ALIAS(_vswprintf,libc_dos_16vswprintf_unlimited);
DEFINE_PUBLIC_ALIAS(_swprintf,libc_dos_16swprintf_unlimited);


INTERN ATTR_DOSTEXT size_t ATTR_CDECL
libc_sscanf_l(char const *__restrict src,
              char const *__restrict format,
              locale_t locale, ...) {
 va_list args; size_t result; va_start(args,locale);
 result = libc_vsscanf(src,format,args);
 va_end(args);
 return result;
}

struct snscanf_data { char const *iter,*end; };
PRIVATE ATTR_RARETEXT ssize_t LIBCCALL libc_snscanf_getc(void *data) {
 if (((struct snscanf_data *)data)->iter ==
     ((struct snscanf_data *)data)->end) return 0;
 return *((struct snscanf_data *)data)->iter++;
}
PRIVATE ATTR_RARETEXT ssize_t LIBCCALL
libc_snscanf_ungetc(int UNUSED(c), void *data) {
 --((struct snscanf_data *)data)->iter;
 return 0;
}
INTERN ATTR_DOSTEXT size_t LIBCCALL
libc_vsnscanf(char const *__restrict src, size_t srclen,
              char const *__restrict format, va_list args) {
 struct snscanf_data data;
 data.end = (data.iter = src)+srclen;
 return libc_format_vscanf(&libc_snscanf_getc,
                           &libc_snscanf_ungetc,
                           &data,format,args);
}

DEFINE_INTERN_ALIAS(libc_sscanf_s_l,libc_sscanf_l);
DEFINE_INTERN_ALIAS(libc_snscanf_s,libc_snscanf);
DEFINE_INTERN_ALIAS(libc_snscanf_s_l,libc_snscanf_l);
INTERN ATTR_DOSTEXT size_t ATTR_CDECL
libc_snscanf(char const *__restrict src, size_t srclen,
             char const *__restrict format, ...) {
 va_list args; size_t result; va_start(args,format);
 result = libc_vsnscanf(src,srclen,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_DOSTEXT size_t ATTR_CDECL
libc_snscanf_l(char const *__restrict src, size_t srclen,
               char const *__restrict format, locale_t locale, ...) {
 va_list args; size_t result; va_start(args,locale);
 result = libc_vsnscanf(src,srclen,format,args);
 va_end(args);
 return result;
}
DEFINE_PUBLIC_ALIAS(_sscanf_l,libc_sscanf_l);
DEFINE_PUBLIC_ALIAS(_sscanf_s_l,libc_sscanf_s_l);
DEFINE_PUBLIC_ALIAS(_snscanf,libc_snscanf);
DEFINE_PUBLIC_ALIAS(_snscanf_s,libc_snscanf_s);
DEFINE_PUBLIC_ALIAS(_snscanf_l,libc_snscanf_l);
DEFINE_PUBLIC_ALIAS(_snscanf_s_l,libc_snscanf_s_l);

INTERN ATTR_DOSTEXT size_t LIBCCALL libc_dos_vsprintf_l(char *__restrict buf, char const *__restrict format, locale_t UNUSED(locale), va_list args) { return libc_vsprintf(buf,format,args); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_dos_vsprintf_p_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t UNUSED(locale),  va_list args) { return libc_vsnprintf(buf,buflen,format,args); }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_dos_sprintf_l(char *__restrict buf, char const *__restrict format, locale_t locale, ...) { va_list args; size_t result; va_start(args,locale); result = libc_vsprintf(buf,format,args); va_end(args); return result; }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_dos_sprintf_p_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, ...) { va_list args; size_t result; va_start(args,locale); result = libc_vsnprintf(buf,buflen,format,args); va_end(args); return result; }
DEFINE_INTERN_ALIAS(libc_dos_sprintf_p,libc_dos_snprintf);
DEFINE_INTERN_ALIAS(libc_dos_vsprintf_p,libc_dos_vsnprintf);
DEFINE_INTERN_ALIAS(libc_dos_sprintf_s_l,libc_dos_sprintf_p_l);
DEFINE_INTERN_ALIAS(libc_dos_vsprintf_s_l,libc_dos_vsprintf_p_l);
DEFINE_PUBLIC_ALIAS(_sprintf_l,libc_dos_sprintf_l);
DEFINE_PUBLIC_ALIAS(_vsprintf_l,libc_dos_vsprintf_l);
DEFINE_PUBLIC_ALIAS(_sprintf_p,libc_dos_sprintf_p);
DEFINE_PUBLIC_ALIAS(_vsprintf_p,libc_dos_vsprintf_p);
DEFINE_PUBLIC_ALIAS(_sprintf_p_l,libc_dos_sprintf_p_l);
DEFINE_PUBLIC_ALIAS(_vsprintf_p_l,libc_dos_vsprintf_p_l);
DEFINE_PUBLIC_ALIAS(_sprintf_s_l,libc_dos_sprintf_s_l);
DEFINE_PUBLIC_ALIAS(_vsprintf_s_l,libc_dos_vsprintf_s_l);

PRIVATE ssize_t LIBCCALL scprintf_callback(char const *__restrict UNUSED(data), size_t datalen, void *UNUSED(closure)) { return (ssize_t)datalen; }
INTERN ATTR_DOSTEXT size_t LIBCCALL   libc_dos_vscprintf(char const *__restrict format, va_list args) { return libc_dos_format_vprintf(&scprintf_callback,NULL,format,args); }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_dos_scprintf(char const *__restrict format, ...) { va_list args; size_t result; va_start(args,format); result = libc_dos_vscprintf(format,args); va_end(args); return result; }
INTERN ATTR_DOSTEXT size_t LIBCCALL   libc_dos_vscprintf_l(char const *__restrict format, locale_t UNUSED(locale), va_list args) { return libc_dos_vscprintf(format,args); }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_dos_scprintf_l(char const *__restrict format, locale_t locale, ...) { va_list args; size_t result; va_start(args,locale); result = libc_dos_vscprintf(format,args); va_end(args); return result; }
DEFINE_INTERN_ALIAS(libc_dos_vscprintf_p,libc_dos_vscprintf);
DEFINE_INTERN_ALIAS(libc_dos_scprintf_p,libc_dos_scprintf);
DEFINE_INTERN_ALIAS(libc_dos_vscprintf_p_l,libc_dos_vscprintf_l);
DEFINE_INTERN_ALIAS(libc_dos_scprintf_p_l,libc_dos_scprintf_l);
DEFINE_PUBLIC_ALIAS(_vscprintf,libc_dos_vscprintf);
DEFINE_PUBLIC_ALIAS(_scprintf,libc_dos_scprintf);
DEFINE_PUBLIC_ALIAS(_vscprintf_p,libc_dos_vscprintf_p);
DEFINE_PUBLIC_ALIAS(_scprintf_p,libc_dos_scprintf_p);
DEFINE_PUBLIC_ALIAS(_vscprintf_l,libc_dos_vscprintf_l);
DEFINE_PUBLIC_ALIAS(_scprintf_l,libc_dos_scprintf_l);
DEFINE_PUBLIC_ALIAS(_vscprintf_p_l,libc_dos_vscprintf_p_l);
DEFINE_PUBLIC_ALIAS(_scprintf_p_l,libc_dos_scprintf_p_l);



DEFINE_INTERN_ALIAS(libc_dos_snprintf_c,libc_dos_snprintf);
DEFINE_INTERN_ALIAS(libc_dos_vsnprintf_c,libc_dos_vsnprintf);
DEFINE_INTERN_ALIAS(libc_dos_snprintf_c_l,libc_dos_sprintf_p_l);
DEFINE_INTERN_ALIAS(libc_dos_vsnprintf_c_l,libc_dos_vsprintf_p_l);
INTERN ATTR_DOSTEXT ssize_t LIBCCALL
libc_dos_vsnprintf_broken(char *__restrict buf, size_t buflen,
                          char const *__restrict format, va_list args) {
 size_t result = libc_dos_vsnprintf(buf,buflen,format,args);
 return result < buflen ? (ssize_t)result : -1;
}
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL
libc_dos_snprintf_broken(char *__restrict buf, size_t buflen, char const *__restrict format, ...) {
 ssize_t result; va_list args; va_start(args,format);
 result = libc_dos_vsnprintf_broken(buf,buflen,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_DOSTEXT ssize_t LIBCCALL
libc_dos_vsnprintf_l_broken(char *__restrict buf, size_t buflen,
                            char const *__restrict format,
                            locale_t UNUSED(locale), va_list args) {
 return libc_dos_vsnprintf_broken(buf,buflen,format,args);
}
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL
libc_dos_snprintf_l_broken(char *__restrict buf, size_t buflen,
                           char const *__restrict format,
                           locale_t locale, ...) {
 ssize_t result; va_list args; va_start(args,locale);
 result = libc_dos_vsnprintf_broken(buf,buflen,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_DOSTEXT ssize_t LIBCCALL
libc_dos_vsnprintf_s_broken(char *__restrict buf, size_t bufsize, size_t buflen,
                            char const *__restrict format, va_list args) {
 size_t result = libc_dos_vsnprintf(buf,MIN(bufsize,buflen),format,args);
 if (result > bufsize) { SET_ERRNO(ERANGE); return -1; } /* XXX: This is correct? */
 return (ssize_t)result;
}
INTERN ATTR_DOSTEXT ssize_t LIBCCALL
libc_dos_vsnprintf_s_l_broken(char *__restrict buf, size_t bufsize, size_t buflen,
                              char const *__restrict format,
                              locale_t UNUSED(locale), va_list args) {
 return libc_dos_vsnprintf_s_broken(buf,bufsize,buflen,format,args);
}
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL
libc_dos_snprintf_s_broken(char *__restrict buf, size_t bufsize,
                           size_t buflen, char const *__restrict format, ...) {
 ssize_t result; va_list args; va_start(args,format);
 result = libc_dos_vsnprintf_s_broken(buf,bufsize,buflen,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL
libc_dos_snprintf_s_l_broken(char *__restrict buf, size_t bufsize, size_t buflen,
                             char const *__restrict format, locale_t locale, ...) {
 ssize_t result; va_list args; va_start(args,locale);
 result = libc_dos_vsnprintf_s_broken(buf,bufsize,buflen,format,args);
 va_end(args);
 return result;
}
DEFINE_PUBLIC_ALIAS(_snprintf,libc_dos_snprintf_broken);
DEFINE_PUBLIC_ALIAS(_vsnprintf,libc_dos_vsnprintf_broken);
DEFINE_PUBLIC_ALIAS(_snprintf_c,libc_dos_snprintf_c);
DEFINE_PUBLIC_ALIAS(_vsnprintf_c,libc_dos_vsnprintf_c);
DEFINE_PUBLIC_ALIAS(_snprintf_l,libc_dos_snprintf_l_broken);
DEFINE_PUBLIC_ALIAS(_vsnprintf_l,libc_dos_vsnprintf_l_broken);
DEFINE_PUBLIC_ALIAS(_snprintf_c_l,libc_dos_snprintf_c_l);
DEFINE_PUBLIC_ALIAS(_vsnprintf_c_l,libc_dos_vsnprintf_c_l);
DEFINE_PUBLIC_ALIAS(_snprintf_s,libc_dos_snprintf_s_broken);
DEFINE_PUBLIC_ALIAS(_vsnprintf_s,libc_dos_vsnprintf_s_broken);
DEFINE_PUBLIC_ALIAS(_snprintf_s_l,libc_dos_snprintf_s_l_broken);
DEFINE_PUBLIC_ALIAS(_vsnprintf_s_l,libc_dos_vsnprintf_s_l_broken);


DEFINE_INTERN_ALIAS(libc_dos_sprintf_s,libc_dos_snprintf);
DEFINE_INTERN_ALIAS(libc_dos_vsprintf_s,libc_dos_vsprintf);
INTERN ATTR_DOSTEXT size_t LIBCCALL
libc_dos_vsnprintf_s(char *__restrict buf, size_t bufsize, size_t buflen,
                     char const *__restrict format, va_list args) {
 return libc_dos_vsnprintf(buf,MIN(bufsize,buflen),format,args);
}
DEFINE_INTERN_ALIAS(libc_sscanf_s,libc_sscanf);
DEFINE_INTERN_ALIAS(libc_vsscanf_s,libc_vsscanf);
DEFINE_PUBLIC_ALIAS(sprintf_s,libc_dos_sprintf_s);
DEFINE_PUBLIC_ALIAS(vsprintf_s,libc_dos_vsprintf_s);
DEFINE_PUBLIC_ALIAS(vsnprintf_s,libc_dos_vsnprintf_s);
DEFINE_PUBLIC_ALIAS(sscanf_s,libc_sscanf_s);
DEFINE_PUBLIC_ALIAS(vsscanf_s,libc_vsscanf_s);


INTERN ATTR_DOSTEXT ssize_t LIBCCALL
libc_dos_16vsnwprintf_broken(char16_t *__restrict buf, size_t buflen,
                             char16_t const *__restrict format, va_list args) {
 size_t result = libc_dos_16vswprintf(buf,buflen,format,args);
 return result < buflen ? (ssize_t)result : -1;
}
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL
libc_dos_16snwprintf_broken(char16_t *__restrict buf, size_t buflen,
                            char16_t const *__restrict format, ...) {
 va_list args; ssize_t result; va_start(args,format);
 result = libc_dos_16vsnwprintf_broken(buf,buflen,format,args);
 va_end(args);
 return result;
}
DEFINE_PUBLIC_ALIAS(_snwprintf,libc_dos_16snwprintf_broken);
DEFINE_PUBLIC_ALIAS(_vsnwprintf,libc_dos_16vsnwprintf_broken);


/* Dual-export wide string printer functions. */
DEFINE_INTERN_ALIAS(libc_dos_16vswprintf_c,libc_dos_16vswprintf);
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_dos_16vscwprintf(char16_t const *__restrict format, va_list args) { return (size_t)libc_dos_16vswprintf(NULL,0,format,args); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_dos_16vscwprintf_l(char16_t const *__restrict format, locale_t UNUSED(locale), va_list args) { return libc_dos_16vscwprintf(format,args); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_dos_16vswprintf_c_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t UNUSED(locale), va_list args) { return libc_dos_16vswprintf_c(buf,buflen,format,args); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_32vscwprintf(char32_t const *__restrict format, va_list args) { return (size_t)libc_32vswprintf(NULL,0,format,args); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_32vscwprintf_l(char32_t const *__restrict format, locale_t UNUSED(locale), va_list args) { return libc_32vscwprintf(format,args); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_32vswprintf_c_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t UNUSED(locale), va_list args) { return libc_32vswprintf_c(buf,buflen,format,args); }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_dos_16scwprintf(char16_t const *__restrict format, ...) { size_t result; va_list args; va_start(args,format); result = libc_dos_16vscwprintf(format,args); va_end(args); return result; }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_dos_16scwprintf_l(char16_t const *__restrict format, locale_t locale, ...) { size_t result; va_list args; va_start(args,locale); result = libc_dos_16vscwprintf(format,args); va_end(args); return result; }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_dos_16swprintf_c(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, ...) { size_t result; va_list args; va_start(args,format); result = libc_dos_16vswprintf_c(buf,buflen,format,args); va_end(args); return result; }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_dos_16swprintf_c_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...) { size_t result; va_list args; va_start(args,locale); result = libc_dos_16vswprintf_c(buf,buflen,format,args); va_end(args); return result; }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_32scwprintf(char32_t const *__restrict format, ...) { size_t result; va_list args; va_start(args,format); result = libc_32vscwprintf(format,args); va_end(args); return result; }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_32scwprintf_l(char32_t const *__restrict format, locale_t locale, ...) { size_t result; va_list args; va_start(args,locale); result = libc_32vscwprintf(format,args); va_end(args); return result; }
INTERN ATTR_DOSTEXT size_t ATTR_CDECL libc_32swprintf_c_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, ...) { size_t result; va_list args; va_start(args,locale); result = libc_32vswprintf_c(buf,buflen,format,args); va_end(args); return result; }
DEFINE_PUBLIC_ALIAS(libc_32swprintf_c,libc_32swprintf);
DEFINE_PUBLIC_ALIAS(libc_32vswprintf_c,libc_32vswprintf);
INTERN ATTR_DOSTEXT ssize_t LIBCCALL   libc_dos_16vsnwprintf_l_broken(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args) { return libc_dos_16vsnwprintf_broken(buf,buflen,format,args); }
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL libc_dos_16snwprintf_l_broken(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...) { ssize_t result; va_list args; va_start(args,locale); result = libc_dos_16vsnwprintf_broken(buf,buflen,format,args); va_end(args); return result; }
INTERN ATTR_DOSTEXT ssize_t LIBCCALL   libc_dos_16vsnwprintf_s_broken(char16_t *__restrict buf, size_t buflen, size_t maxlen, char16_t const *__restrict format, va_list args) { return libc_dos_16vsnwprintf_broken(buf,MIN(buflen,maxlen),format,args); }
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL libc_dos_16snwprintf_s_broken(char16_t *__restrict buf, size_t buflen, size_t maxlen, char16_t const *__restrict format, ...) { ssize_t result; va_list args; va_start(args,format); result = libc_dos_16vsnwprintf_broken(buf,MIN(maxlen,buflen),format,args); va_end(args); return result; }
INTERN ATTR_DOSTEXT ssize_t LIBCCALL   libc_dos_16vsnwprintf_s_l_broken(char16_t *__restrict buf, size_t buflen, size_t maxlen, char16_t const *__restrict format, locale_t UNUSED(locale), va_list args) { return libc_dos_16vsnwprintf_broken(buf,MIN(buflen,maxlen),format,args); }
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL libc_dos_16snwprintf_s_l_broken(char16_t *__restrict buf, size_t buflen, size_t maxlen, char16_t const *__restrict format, locale_t locale, ...) { ssize_t result; va_list args; va_start(args,locale); result = libc_dos_16vsnwprintf_broken(buf,MIN(maxlen,buflen),format,args); va_end(args); return result; }

DEFINE_PUBLIC_ALIAS(_scwprintf,libc_dos_16scwprintf);
DEFINE_PUBLIC_ALIAS(_vscwprintf,libc_dos_16vscwprintf);
DEFINE_PUBLIC_ALIAS(_scwprintf_l,libc_dos_16scwprintf_l);
DEFINE_PUBLIC_ALIAS(_vscwprintf_l,libc_dos_16vscwprintf_l);
DEFINE_PUBLIC_ALIAS(_swprintf_c,libc_dos_16swprintf_c);
DEFINE_PUBLIC_ALIAS(_vswprintf_c,libc_dos_16vswprintf_c);
DEFINE_PUBLIC_ALIAS(_swprintf_c_l,libc_dos_16swprintf_c_l);
DEFINE_PUBLIC_ALIAS(_vswprintf_c_l,libc_dos_16vswprintf_c_l);
DEFINE_PUBLIC_ALIAS(_snwprintf_l,libc_dos_16snwprintf_l_broken);
DEFINE_PUBLIC_ALIAS(_vsnwprintf_l,libc_dos_16vsnwprintf_l_broken);
DEFINE_PUBLIC_ALIAS(_snwprintf_s,libc_dos_16snwprintf_s_broken);
DEFINE_PUBLIC_ALIAS(_vsnwprintf_s,libc_dos_16vsnwprintf_s_broken);
DEFINE_PUBLIC_ALIAS(_snwprintf_s_l,libc_dos_16snwprintf_s_l_broken);
DEFINE_PUBLIC_ALIAS(_vsnwprintf_s_l,libc_dos_16vsnwprintf_s_l_broken);
DEFINE_PUBLIC_ALIAS(scwprintf,libc_32scwprintf);
DEFINE_PUBLIC_ALIAS(vscwprintf,libc_32vscwprintf);
DEFINE_PUBLIC_ALIAS(scwprintf_l,libc_32scwprintf_l);
DEFINE_PUBLIC_ALIAS(vscwprintf_l,libc_32vscwprintf_l);
//DEFINE_PUBLIC_ALIAS(swprintf_c,libc_32swprintf_c);
//DEFINE_PUBLIC_ALIAS(vswprintf_c,libc_32vswprintf_c);
DEFINE_PUBLIC_ALIAS(swprintf_c_l,libc_32swprintf_c_l);
DEFINE_PUBLIC_ALIAS(vswprintf_c_l,libc_32vswprintf_c_l);

DEFINE_INTERN_ALIAS(libc_dos_16swprintf_s,libc_dos_16swprintf);
DEFINE_INTERN_ALIAS(libc_dos_16vswprintf_s,libc_dos_16vswprintf);
DEFINE_INTERN_ALIAS(libc_16swscanf_s,libc_16swscanf);
DEFINE_INTERN_ALIAS(libc_16vswscanf_s,libc_16vswscanf);
DEFINE_INTERN_ALIAS(libc_32swprintf_s,libc_32swprintf);
DEFINE_INTERN_ALIAS(libc_32vswprintf_s,libc_32vswprintf);
DEFINE_INTERN_ALIAS(libc_32swscanf_s,libc_32swscanf);
DEFINE_INTERN_ALIAS(libc_32vswscanf_s,libc_32vswscanf);
/* NOTE: Only exported for DOS-mode. */
DEFINE_PUBLIC_ALIAS(swprintf_s,libc_dos_16swprintf_s);
DEFINE_PUBLIC_ALIAS(vswprintf_s,libc_dos_16vswprintf_s);
DEFINE_PUBLIC_ALIAS(swscanf_s,libc_16swscanf_s);
DEFINE_PUBLIC_ALIAS(vswscanf_s,libc_16vswscanf_s);



DEFINE_INTERN_ALIAS(libc_dos_16scwprintf_p,libc_dos_16scwprintf);
DEFINE_INTERN_ALIAS(libc_dos_16vscwprintf_p,libc_dos_16vscwprintf);
DEFINE_INTERN_ALIAS(libc_dos_16scwprintf_p_l,libc_dos_16scwprintf_l);
DEFINE_INTERN_ALIAS(libc_dos_16vscwprintf_p_l,libc_dos_16vscwprintf_l);
DEFINE_INTERN_ALIAS(libc_dos_16swprintf_p,libc_dos_16swprintf);
DEFINE_INTERN_ALIAS(libc_dos_16vswprintf_p,libc_dos_16vswprintf);
DEFINE_INTERN_ALIAS(libc_dos_16swprintf_p_l,libc_dos_16swprintf_c_l);
DEFINE_INTERN_ALIAS(libc_dos_16vswprintf_p_l,libc_dos_16vswprintf_c_l);
DEFINE_INTERN_ALIAS(libc_dos_16swprintf_s_l,libc_dos_16swprintf_c_l);
DEFINE_INTERN_ALIAS(libc_dos_16vswprintf_s_l,libc_dos_16vswprintf_c_l);
DEFINE_INTERN_ALIAS(libc_32scwprintf_p,libc_32scwprintf);
DEFINE_INTERN_ALIAS(libc_32vscwprintf_p,libc_32vscwprintf);
DEFINE_INTERN_ALIAS(libc_32scwprintf_p_l,libc_32scwprintf_l);
DEFINE_INTERN_ALIAS(libc_32vscwprintf_p_l,libc_32vscwprintf_l);
DEFINE_INTERN_ALIAS(libc_32swprintf_p,libc_32swprintf);
DEFINE_INTERN_ALIAS(libc_32vswprintf_p,libc_32vswprintf);
DEFINE_INTERN_ALIAS(libc_32swprintf_p_l,libc_32swprintf_c_l);
DEFINE_INTERN_ALIAS(libc_32vswprintf_p_l,libc_32vswprintf_c_l);
DEFINE_INTERN_ALIAS(libc_32swprintf_s_l,libc_32swprintf_c_l);
DEFINE_INTERN_ALIAS(libc_32vswprintf_s_l,libc_32vswprintf_c_l);
/* None of these are exported outside of DOS mode. */
DEFINE_PUBLIC_ALIAS(_scwprintf_p,libc_dos_16scwprintf_p);
DEFINE_PUBLIC_ALIAS(_vscwprintf_p,libc_dos_16vscwprintf_p);
DEFINE_PUBLIC_ALIAS(_scwprintf_p_l,libc_dos_16scwprintf_p_l);
DEFINE_PUBLIC_ALIAS(_vscwprintf_p_l,libc_dos_16scwprintf_p_l);
DEFINE_PUBLIC_ALIAS(_swprintf_p,libc_dos_16swprintf_p);
DEFINE_PUBLIC_ALIAS(_vswprintf_p,libc_dos_16vswprintf_p);
DEFINE_PUBLIC_ALIAS(_swprintf_p_l,libc_dos_16swprintf_p_l);
DEFINE_PUBLIC_ALIAS(_vswprintf_p_l,libc_dos_16vswprintf_p_l);
DEFINE_PUBLIC_ALIAS(_swprintf_s_l,libc_dos_16swprintf_s_l);
DEFINE_PUBLIC_ALIAS(_vswprintf_s_l,libc_dos_16vswprintf_s_l);
//DEFINE_PUBLIC_ALIAS(scwprintf_p,libc_32scwprintf_p);
//DEFINE_PUBLIC_ALIAS(vscwprintf_p,libc_32vscwprintf_p);
//DEFINE_PUBLIC_ALIAS(scwprintf_p_l,libc_32scwprintf_p_l);
//DEFINE_PUBLIC_ALIAS(vscwprintf_p_l,libc_32scwprintf_p_l);
//DEFINE_PUBLIC_ALIAS(swprintf_p,libc_32swprintf_p);
//DEFINE_PUBLIC_ALIAS(vswprintf_p,libc_32vswprintf_p);
//DEFINE_PUBLIC_ALIAS(swprintf_p_l,libc_32swprintf_p_l);
//DEFINE_PUBLIC_ALIAS(vswprintf_p_l,libc_32vswprintf_p_l);
//DEFINE_PUBLIC_ALIAS(swprintf_s_l,libc_32swprintf_s_l);
//DEFINE_PUBLIC_ALIAS(vswprintf_s_l,libc_32vswprintf_s_l);


INTERN ATTR_DOSTEXT size_t ATTR_CDECL
libc_16swscanf_l(char16_t const *src,
                 char16_t const *__restrict format,
                 locale_t locale, ...) {
 size_t result; va_list args; va_start(args,locale);
 result = libc_16vswscanf(src,format,args);
 va_end(args);
 return result;
}
struct c16_snscanf_data { char16_t const *iter,*end; };
PRIVATE ATTR_RARETEXT ssize_t LIBCCALL libc_16snwscanf_getc(void *data) {
 if (((struct c16_snscanf_data *)data)->iter ==
     ((struct c16_snscanf_data *)data)->end) return 0;
 return (ssize_t)(char)*((struct c16_snscanf_data *)data)->iter++;
}
PRIVATE ATTR_RARETEXT ssize_t LIBCCALL
libc_16snwscanf_ungetc(int UNUSED(c), void *data) {
 --((struct c16_snscanf_data *)data)->iter;
 return 0;
}
INTERN ATTR_DOSTEXT size_t LIBCCALL
libc_16vsnwscanf(char16_t const *src, size_t srclen,
                 char16_t const *__restrict format,
                 va_list args) {
 ssize_t result = -1; char *utf8_format;
 if ((utf8_format = libc_utf16to8m(format)) != NULL) {
  struct c16_snscanf_data data;
  data.end = (data.iter = src)+srclen;
  result = libc_format_vscanf(&libc_16snwscanf_getc,
                              &libc_16snwscanf_ungetc,
                             (void *)&data,utf8_format,args);
  libc_free(utf8_format);
 }
 return result;
}
INTERN ATTR_DOSTEXT size_t ATTR_CDECL
libc_16snwscanf(char16_t const *src, size_t srclen,
                char16_t const *__restrict format, ...) {
 size_t result; va_list args; va_start(args,format);
 result = libc_16vsnwscanf(src,srclen,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_DOSTEXT size_t ATTR_CDECL
libc_16snwscanf_l(char16_t const *src, size_t srclen,
                  char16_t const *__restrict format,
                  locale_t locale, ...) {
 size_t result; va_list args; va_start(args,locale);
 result = libc_16vsnwscanf(src,srclen,format,args);
 va_end(args);
 return result;
}
DEFINE_INTERN_ALIAS(libc_16swscanf_s_l,libc_16swscanf_l);
DEFINE_INTERN_ALIAS(libc_16snwscanf_s,libc_16snwscanf);
DEFINE_INTERN_ALIAS(libc_16snwscanf_s_l,libc_16snwscanf_l);
INTERN ATTR_DOSTEXT size_t ATTR_CDECL
libc_32swscanf_l(char32_t const *src,
                 char32_t const *__restrict format,
                 locale_t locale, ...) {
 size_t result; va_list args; va_start(args,locale);
 result = libc_32vswscanf(src,format,args);
 va_end(args);
 return result;
}
struct c32_snscanf_data { char32_t const *iter,*end; };
PRIVATE ATTR_RARETEXT ssize_t LIBCCALL libc_32snwscanf_getc(void *data) {
 if (((struct c32_snscanf_data *)data)->iter ==
     ((struct c32_snscanf_data *)data)->end) return 0;
 return (ssize_t)(char)*((struct c32_snscanf_data *)data)->iter++;
}
PRIVATE ATTR_RARETEXT ssize_t LIBCCALL
libc_32snwscanf_ungetc(int UNUSED(c), void *data) {
 --((struct c32_snscanf_data *)data)->iter;
 return 0;
}
INTERN ATTR_DOSTEXT size_t LIBCCALL
libc_32vsnwscanf(char32_t const *src, size_t srclen,
                 char32_t const *__restrict format,
                 va_list args) {
 ssize_t result = -1; char *utf8_format;
 if ((utf8_format = libc_utf32to8m(format)) != NULL) {
  struct c32_snscanf_data data;
  data.end = (data.iter = src)+srclen;
  result = libc_format_vscanf(&libc_32snwscanf_getc,
                              &libc_32snwscanf_ungetc,
                             (void *)&data,utf8_format,args);
  libc_free(utf8_format);
 }
 return result;
}
INTERN ATTR_DOSTEXT size_t ATTR_CDECL
libc_32snwscanf(char32_t const *src, size_t srclen,
                char32_t const *__restrict format, ...) {
 size_t result; va_list args; va_start(args,format);
 result = libc_32vsnwscanf(src,srclen,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_DOSTEXT size_t ATTR_CDECL
libc_32snwscanf_l(char32_t const *src, size_t srclen,
                  char32_t const *__restrict format,
                  locale_t locale, ...) {
 size_t result; va_list args; va_start(args,locale);
 result = libc_32vsnwscanf(src,srclen,format,args);
 va_end(args);
 return result;
}
DEFINE_INTERN_ALIAS(libc_32swscanf_s_l,libc_32swscanf_l);
DEFINE_INTERN_ALIAS(libc_32snwscanf_s,libc_32snwscanf);
DEFINE_INTERN_ALIAS(libc_32snwscanf_s_l,libc_32snwscanf_l);

DEFINE_PUBLIC_ALIAS(_swscanf_l,libc_16swscanf_l);
DEFINE_PUBLIC_ALIAS(swscanf_l,libc_32swscanf_l);
DEFINE_PUBLIC_ALIAS(_swscanf_s_l,libc_16swscanf_s_l);
//DEFINE_PUBLIC_ALIAS(swscanf_s_l,libc_32swscanf_s_l); /* Linked against 'swscanf_l' in headers. */
DEFINE_PUBLIC_ALIAS(_snwscanf,libc_16snwscanf);
DEFINE_PUBLIC_ALIAS(snwscanf,libc_32snwscanf);
DEFINE_PUBLIC_ALIAS(_snwscanf_s,libc_16snwscanf_s);
//DEFINE_PUBLIC_ALIAS(snwscanf_s,libc_32snwscanf_s); /* Linked against 'snwscanf' in headers. */
DEFINE_PUBLIC_ALIAS(_snwscanf_l,libc_16snwscanf_l);
DEFINE_PUBLIC_ALIAS(snwscanf_l,libc_32snwscanf_l);
DEFINE_PUBLIC_ALIAS(_snwscanf_s_l,libc_16snwscanf_s_l);
//DEFINE_PUBLIC_ALIAS(snwscanf_s_l,libc_32snwscanf_s_l); /* Linked against 'snwscanf_l' in headers. */


/* Define some DOS alises. */
INTERN ATTR_DOSTEXT ssize_t LIBCCALL
libc_dos_16vswprintf_l_unlimited(char16_t *__restrict buf,
                                 char16_t const *__restrict format,
                                 locale_t UNUSED(locale), va_list args) {
 return libc_dos_16vswprintf_unlimited(buf,format,args);
}
INTERN ATTR_DOSTEXT ssize_t ATTR_CDECL
libc_dos_16swprintf_l_unlimited(char16_t *__restrict buf,
                                char16_t const *__restrict format,
                                locale_t locale, ...) {
 ssize_t result; va_list args; va_start(args,locale);
 result = libc_dos_16vswprintf_unlimited(buf,format,args);
 va_end(args);
 return result;
}
DEFINE_PUBLIC_ALIAS(__vswprintf_l,libc_dos_16vswprintf_l_unlimited);
DEFINE_PUBLIC_ALIAS(__swprintf_l,libc_dos_16swprintf_l_unlimited);
DEFINE_PUBLIC_ALIAS(_swprintf_l,libc_dos_16swprintf_c_l);
DEFINE_PUBLIC_ALIAS(_vswprintf_l,libc_dos_16vswprintf_c_l);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */

DECL_END

#pragma GCC diagnostic pop

#endif /* !GUARD_LIBS_LIBC_FORMAT_PRINTER_C */
