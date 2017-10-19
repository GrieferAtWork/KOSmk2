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
#ifndef _FORMAT_PRINTER_H
#define _FORMAT_PRINTER_H 1

#include "__stdinc.h"
#include <bits/types.h>
#include <hybrid/typecore.h>
#include <features.h>
#ifndef __KERNEL__
#include <bits/mbstate.h>
#endif

#if (!defined(__CRT_KOS) || \
     (defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__))) && \
     !defined(__BUILDING_LIBC)
#define __USE_PRIVATE_FORMAT_PRINTER
#if !defined(__NO_ATTR_WEAK) && \
    !defined(__ATTR_WEAK_IS_SELECTANY)
#define __FORMAT_PRINTER_DECL __INTDEF __ATTR_WEAK
#else /* !__NO_ATTR_WEAK */
#define __FORMAT_PRINTER_DECL __PRIVATE
#endif /* __NO_ATTR_WEAK */
#else
#define __FORMAT_PRINTER_DECL __LIBC
#endif

__SYSDECL_BEGIN

#ifdef __CC__
#ifndef __pformatprinter_defined
#define __pformatprinter_defined 1
/* Callback functions prototypes provided to format functions.
 * NOTE: 'pformatprinter' usually returns the number of characters printed, but isn't required to.
 * @param: DATA:    The base address of a DATALEN bytes long character vector that should be printed.
 * @param: DATALEN: The amount of characters that should be printed, starting at 'data'.
 *                  Note that this is an exact value, meaning that a NUL-character appearing
 *                  before then should not terminate printing prematurely, but be printed as well.
 * @param: CLOSURE: The user-defined closure parameter passed alongside this function pointer.
 * @return: < 0:    An error occurred and the calling function shall return with this same value.
 * @return: >= 0:   The print was successful.
 *                  Usually, the return value is added to a sum of values which is then
 *                  returned by the calling function upon success, also meaning that the
 *                  usual return value used to indicate success in 'DATALEN'. */
typedef __ssize_t (__LIBCCALL *pformatprinter)(char const *__restrict __data,
                                               __size_t __datalen, void *__closure);
typedef __ssize_t (__LIBCCALL *pformatgetc)(void *__closure);
typedef __ssize_t (__LIBCCALL *pformatungetc)(int __ch, void *__closure);
#endif /* !__pformatprinter_defined */

/* Generic printf implementation
 * Taking a regular printf-style format string and arguments, these
 * functions will call the given 'PRINTER' callback with various strings
 * that, when put together, result in the desired formated text.
 *  - 'PRINTER' obviously is called with the text parts in their correct order
 *  - If 'PRINTER' returns '< 0', the function returns immediately,
 *    yielding that same value. Otherwise, format_printf() returns
 *    the sum of all return values from 'PRINTER'.
 *  - The strings passed to 'PRINTER' may not necessarily be zero-terminated, and
 *    a second argument is passed that indicates the absolute length in characters.
 * Supported extensions:
 *  - '%q'-format mode: Semantics equivalent to '%s', this modifier escapes the string using
 *                        'format_quote' with flags set of 'FORMAT_QUOTE_FLAG_NONE', or
 *                        'PRINTF_FLAG_PREFIX' when the '#' flag was used (e.g.: '%#q').
 *  - '%~s'    [KERNEL-ONLY] Print a string from a user-space pointer (may be combined to something like '%~.?s')
 *  - '%.*s'   Instead of reading an 'int' and dealing with undefined behavior when negative, an 'unsigned int' is read.
 *  - '%.?s'   Similar to '%.*s', but takes an 'size_t' from the argument list instead of an 'unsigned int', as well as define
 *             a fixed-length buffer size for string/quote formats (thus allowing you to print '\0' characters after quoting)
 *  - '%$s'    Same as '$.?s'
 *  - '%Qs'    Same as '%q'
 *  - '%Qc'    Print an escaped character.
 *  - '%I'     length modifier: Integral length equivalent to sizeof(size_t).
 *  - '%I8'    length modifier: Integral length equivalent to sizeof(int8_t).
 *  - '%I16'   length modifier: Integral length equivalent to sizeof(int16_t).
 *  - '%I32'   length modifier: Integral length equivalent to sizeof(int32_t).
 *  - '%I64'   length modifier: Integral length equivalent to sizeof(int64_t).
 *  - '%[...]' Extended formating options, allowing for additional formating options:
 *             - '%[errno]': printf("%[errno]",EAGAIN); // Print human-readable information about the error 'EAGAIN'
 * >>> Possible (and actual) uses:
 *  - printf:           Unbuffered output into any kind of stream/file.
 *  - sprintf/snprintf: Unsafe/Counted string formatting into a user-supplied buffer.
 *  - strdupf:          Output into dynamically allocated heap memory,
 *                      increasing the buffer when it gets filled completely.
 *  - k_syslogf:        Unbuffered system-log output.
 *  - ...               There are a _lot_ more... */
__FORMAT_PRINTER_DECL __NONNULL((1,3))
__ssize_t (__ATTR_CDECL format_printf)(pformatprinter __printer, void *__closure,
                                       char const *__restrict __format, ...);
__FORMAT_PRINTER_DECL __NONNULL((1,3))
__ssize_t (__LIBCCALL format_vprintf)(pformatprinter __printer, void *__closure,
                                      char const *__restrict __format, __VA_LIST __args);


#ifdef __CRT_KOS
/* Generic scanf implementation
 * Taking a regular scanf-style format string and argument, these
 * functions will call the given 'SCANNER' function which in
 * return should successively yield a character at a time from
 * some kind of input source.
 *  - If 'SCANNER' returns < 0, scanning aborts and that value is returned.
 *    Otherwise, the function returns the amount of successfully parsed arguments.
 *  - The user may use 'SCANNER' to track the last read character to get
 *    additional information about what character caused the scan to fail.
 *  - The given 'SCANNER' should also indicate EOF by returning 'NUL'
 *  - This implementation supports the following extensions:
 *    - '%[A-Z]'   -- Character ranges in scan patterns
 *    - '%[^abc]'  -- Inversion of a scan pattern
 *    - '\n'       -- Skip any kind of linefeed ('\n', '\r', '\r\n')
 *    - '%$s'      -- '$'-modifier, available for any format outputting a string.
 *                    This modifier reads a 'size_t' from the argument list,
 *                    that specifies the size of the following string buffer:
 *                 >> char buffer[64];
 *                 >> sscanf(data,"My name is %.?s\n",sizeof(buffer),buffer);
 * format -> %[*|?][width][length]specifier
 * @return: * : The total number of successfully scanned arguments. */
__LIBC __PORT_KOSONLY __NONNULL((1,4))
__ssize_t (__ATTR_CDECL format_scanf)(pformatgetc __pgetc, pformatungetc __pungetc,
                                      void *__closure, char const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __NONNULL((1,4))
__ssize_t (__LIBCCALL format_vscanf)(pformatgetc __pgetc, pformatungetc __pungetc,
                                     void *__closure, char const *__restrict __format,
                                     __VA_LIST __args);

#ifndef __KERNEL__
#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_BEGIN struct tm;
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */
/* The format-generic version of strftime-style formatting.
 * NOTE: Besides supported the standard, name extensions, as already supported
 *       by deemon are supported as well (Documentation taken from deemon):
 * WARNING: Not all extensions from deemon are implemented (no milliseconds, and no *s (years, mouths, etc.) are supported)
 * NOTE: Since I really think that the format modifiers for strftime are just total garbage,
 *       and only designed to be short, but not readable, I've added a new, extended modifier
 *       that is meant to fix this:
 *       >> format_strftime(...,"%[S:WD], the %[n:D] of %[S:M] %[Y], %[2:H]:%[2:MI]:%[2:S]",time(NULL));
 *       -> Tuesday, the 8th of November 2016, 17:45:38
 *       attribute ::= ('year'|'month'|'mweek'|'yweek'|'wday'|'mday'|
 *                      'yday'|'hour'|'minute'|'second'|'msecond'|
 *                      'Y'|'M'|'MW'|'YW'|'WD'|'MD'|'D'|'YD'|'H'|'MI'|'I'|'S'|'MS');
 *       exftime ::= '%[' ## [('S'|'s'|(['n'|' '] ## ('0'..'9')##...)] ## ':')] ## attribute ## ']';
 *       - The format modifier always starts with '%[' and ends with ']', with the actual text in-between
 *       - Optional representation prefix (before ':'):
 *         - 's': Short representation of the attribute (only allowed for 'wday' and 'month')
 *         - 'S': Long representation of the attribute (only allowed for 'wday' and 'month')
 *         - 'n': nth representation of the attribute (can be prefixed infront of width modifier; (1st, 2nd, 3rd, 4th, 5th, ...))
 *         - ' ': Fill empty space created by a large width modifier with ' ' instead of '0'
 *       - Optional width prefix (before ':'):
 *         - Only allowed without a representation prefix, the 'n' or ' ' prefix.
 *       - Attribute name:
 *         - One of the names listed above, this part describes which attribute to refer to.
 *         - The attributes match the member names of the time object, with the following aliases provided:
 *           - 'Y'      --> 'year'
 *           - 'M'      --> 'month'
 *           - 'WD'     --> 'wday'
 *           - 'MD'|'D' --> 'mday'
 *           - 'YD'     --> 'yday'
 *           - 'H'      --> 'hour'
 *           - 'MI'|'I' --> 'minute'
 *           - 'S'      --> 'second' */
__LIBC __PORT_KOSONLY __NONNULL((1,4))
__ssize_t (__LIBCCALL format_strftime)(pformatprinter __printer, void *__closure,
                                       char const *__restrict __format, struct tm const *__tm);
#endif /* !__KERNEL__ */
#endif /* __CRT_KOS */

/* Do C-style quotation on the given text, printing
 * all of its escaped portions to the given printer.
 * Input:
 * >> Hello "World" W
 * >> hat a great day.
 * Output #1: >> \"Hello \"World\" W\nhat a great day.\"
 * Output #2: >> Hello \"World\" W\nhat a great day.
 * NOTE: Output #2 is generated if the 'FORMAT_QUOTE_FLAG_PRINTRAW' is set.
 * This function escapes all control and non-ascii characters,
 * preferring octal encoding for control characters and hex-encoding
 * for other non-ascii characters, a behavior that may be modified
 * with the 'FORMAT_QUOTE_FLAG_FORCE*' flags.
 * @param: PRINTER: A function called for all quoted portions of the text.
 * @param: MAXTEXT: strnlen-style maxlen for the given TEXT,
 *                  unless the 'FORMAT_QUOTE_FLAG_QUOTEALL' flag is
 *                  set, in which case it is the exact amount of characters
 *                  to quote from 'TEXT', including '\0' characters. */
__FORMAT_PRINTER_DECL __NONNULL((1,3)) __ssize_t
(__LIBCCALL format_quote)(pformatprinter __printer, void *__closure,
                          char const *__restrict __text, __size_t __textlen,
                          __UINT32_TYPE__ __flags);
#endif /* __CC__ */
#define FORMAT_QUOTE_FLAG_NONE     0x00000000
#define FORMAT_QUOTE_FLAG_PRINTRAW 0x00000001 /*< Don't surround the quoted text with "..."; */
#define FORMAT_QUOTE_FLAG_FORCEHEX 0x00000002 /*< Force hex encoding of all control characters without special strings ('\n', etc.). */
#define FORMAT_QUOTE_FLAG_FORCEOCT 0x00000004 /*< Force octal encoding of all non-ascii characters. */
#define FORMAT_QUOTE_FLAG_NOCTRL   0x00000008 /*< Disable special encoding strings such as '\r', '\n' or '\e' */
#define FORMAT_QUOTE_FLAG_NOASCII  0x00000010 /*< Disable regular ascii-characters and print everything using special encodings. */
#define FORMAT_QUOTE_FLAG_UPPERHEX 0x00000020 /*< Use uppercase characters for hex (e.g.: '\xAB'). */



#ifdef __CRT_KOS
#ifdef __CC__
/* Print a hex dump of the given data using the provided format printer.
 * @param: PRINTER:  A function called for all quoted portions of the text.
 * @param: DATA:     A pointer to the data that should be dumped.
 * @param: SIZE:     The amount of bytes read starting at DATA.
 * @param: LINESIZE: The max amount of bytes to include per-line.
 *                   HINT: Pass ZERO(0) to use a default size (16)
 * @param: FLAGS:    A set of 'FORMAT_HEXDUMP_FLAG_*'.
 * @return: 0: The given data was successfully hex-dumped.
 * @return: *: The first non-ZERO(0) return value of PRINTER. */
__LIBC __PORT_KOSONLY __NONNULL((1,3)) __ssize_t
(__LIBCCALL format_hexdump)(pformatprinter __printer, void *__closure,
                            void const *__restrict __data, __size_t __size,
                            __size_t __linesize, __UINT32_TYPE__ __flags);
#endif /* __CC__ */
#define FORMAT_HEXDUMP_FLAG_NONE     0x00000000
#define FORMAT_HEXDUMP_FLAG_ADDRESS  0x00000001 /*< Include the absolute address at the start of every line. */
#define FORMAT_HEXDUMP_FLAG_OFFSETS  0x00000002 /*< Include offsets from the base address at the start of every line (after the address when also shown). */
#define FORMAT_HEXDUMP_FLAG_NOHEX    0x00000004 /*< Don't print the actual hex dump (hex data representation). */
#define FORMAT_HEXDUMP_FLAG_NOASCII  0x00000008 /*< Don't print ascii representation of printable characters at the end of lines. */
#define FORMAT_HEXDUMP_FLAG_HEXLOWER 0x00000010 /*< Print hex text of the dump in lowercase (does not affect address/offset). */
#endif /* __CRT_KOS */


#ifdef __CC__
struct stringprinter {
#if defined(__BUILDING_LIBC) || defined(__KERNEL__)
 char   *sp_bufpos; /*< [1..1][>= sp_buffer][<= sp_bufend] . */
 char   *sp_buffer; /*< [1..1] Allocate buffer base pointer. */
 char   *sp_bufend; /*< [1..1] Buffer end (Pointer to currently allocated '\0'-character). */
#else /* __BUILDING_LIBC || __KERNEL__ */
 char *__sp_bufpos; /*< [1..1][>= __sp_buffer][<= __sp_bufend] . */
 char *__sp_buffer; /*< [1..1] Allocate buffer base pointer. */
 char *__sp_bufend; /*< [1..1] Buffer end (Pointer to currently allocated '\0'-character). */
#endif /* !__BUILDING_LIBC && !__KERNEL__ */
};

/* Helper functions for using any pformatprinter-style
 * function to print into a dynamically allocated string.
 * >> struct stringprinter printer; char *text;
 * >> if (stringprinter_init(&printer,0)) return handle_error();
 * >> if (format_printf(&stringprinter_print,&printer,"Hello %s","dynamic world")) {
 * >>   stringprinter_fini(&printer);
 * >>   return handle_error();
 * >> } else {
 * >>   text = stringprinter_pack(&printer,NULL);
 * >>   //stringprinter_fini(&printer); // No-op after pack has been called
 * >> }
 * >> ...
 * >> free(text);
 * @param: HINT: A hint as to how big the initial buffer should
 *               be allocated as (Pass ZERO if unknown).
 * @return:  0: Successfully printed to/initialized the given string printer.
 * @return: -1: Failed to initialize/print the given text ('errno' is set to ENOMEM) */
__FORMAT_PRINTER_DECL __NONNULL((1)) int  (__LIBCCALL stringprinter_init)(struct stringprinter *__restrict __self, __size_t __hint);
__FORMAT_PRINTER_DECL __NONNULL((1)) void (__LIBCCALL stringprinter_fini)(struct stringprinter *__restrict __self);
__FORMAT_PRINTER_DECL __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL stringprinter_pack)(struct stringprinter *__restrict __self, __size_t *__length);
__FORMAT_PRINTER_DECL __ssize_t (__LIBCCALL stringprinter_print)(char const *__restrict __data, __size_t __datalen, void *__closure);


#if !defined(__KERNEL__) && defined(__CRT_KOS)
#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

/* Generic unicode/wide-string to utf8 conversion, using a format-printer as target.
 * NOTE: The given '(C(16|32)|WCS)LEN' is the absolute amount of encoded characters,
 *       meaning that any NUL-characters before then are printed as well.
 *       To use strnlen-style semantics, use 'format_*sntomb' instead.
 * NOTE: Upon encoding error, errno is set to 'EILSEQ' and '-1' is returned.
 * @param: MODE: Set of 'UNICODE_F_*' from '<unicode.h>' (Only 'UNICODE_F_NOFAIL' changes the behavior)
 * HINT: These functions are also used to implement '%ls'. */
__REDIRECT_TOPE_ (__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,format_c16sztomb,(pformatprinter __printer, void *__closure, char16_t const *__restrict __c16, __size_t __c16len, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),format_wcsztomb,(__printer,__closure,__c16,__c16len,__ps,__mode))
__REDIRECT_TOKOS_(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,format_c32sztomb,(pformatprinter __printer, void *__closure, char32_t const *__restrict __c32, __size_t __c32len, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),format_wcsztomb,(__printer,__closure,__c32,__c32len,__ps,__mode))
__REDIRECT_TOPE_ (__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,format_c16sntomb,(pformatprinter __printer, void *__closure, char16_t const *__restrict __c16, __size_t __c16max, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),format_wcsntomb,(__printer,__closure,__c16,__c16max,__ps,__mode))
__REDIRECT_TOKOS_(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,format_c32sntomb,(pformatprinter __printer, void *__closure, char32_t const *__restrict __c32, __size_t __c32max, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),format_wcsntomb,(__printer,__closure,__c32,__c32max,__ps,__mode))
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL format_wcsztomb)(pformatprinter __printer, void *__closure, wchar_t const *__restrict __wcs, __size_t __wcslen, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL format_wcsntomb)(pformatprinter __printer, void *__closure, wchar_t const *__restrict __wcs, __size_t __wcsmax, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode);



/* Printer-style multi-byte string to utf16/32 or wide-char conversion.
 * >> ssize_t LIBCCALL
 * >> my_wprinter(wchar_t const *__restrict data,
 * >>             size_t datalen, void *closure) {
 * >>     return printf("{WSTR:%$ls}",datalen,data);
 * >> }
 * >> 
 * >> void foo(void) {
 * >>     struct wprinter p;
 * >>     wprinter_init(&p,&my_wprinter,NULL);
 * >>     format_printf(&wprinter_print,&p,"This string %s\n",
 * >>                   "is converted to wide encoding");
 * >>     wprinter_fini(&p);
 * >> }
 */
typedef __ssize_t (__LIBCCALL *pwformatprinter)(wchar_t const *__restrict __data, __size_t __datalen, void *__closure);
typedef __ssize_t (__LIBCCALL *pc16formatprinter)(char16_t const *__restrict __data, __size_t __datalen, void *__closure);
typedef __ssize_t (__LIBCCALL *pc32formatprinter)(char32_t const *__restrict __data, __size_t __datalen, void *__closure);
#ifdef __BUILDING_LIBC
#define __DEFINE_PRINTER(T,Tpfp) { Tpfp p_printer; void *p_closure; T *p_buffer; __size_t p_buflen; mbstate_t p_mbstate; void *p_padding; }
#else
#define __DEFINE_PRINTER(T,Tpfp) { Tpfp __p_printer; void *__p_closure; T *__p_buffer; __size_t __p_buflen; mbstate_t __p_mbstate; void *__p_padding; }
#endif
struct wprinter   __DEFINE_PRINTER(wchar_t,pwformatprinter);
#ifdef __USE_UTF
struct c16printer __DEFINE_PRINTER(char16_t,pc16formatprinter);
struct c32printer __DEFINE_PRINTER(char32_t,pc32formatprinter);
#endif /* __USE_UTF */
#undef __DEFINE_PRINTER

#define WPRINTER_INIT(printer,closure)   {printer,closure,NULL,0,__MBSTATE_INIT,NULL}
__LIBC __PORT_KOSONLY void (__LIBCCALL wprinter_init)(struct wprinter *__restrict wp, pwformatprinter printer, void *__closure);
__LIBC __PORT_KOSONLY void (__LIBCCALL wprinter_fini)(struct wprinter *__restrict wp);
/* NOTE: Wide-character printers forward the return value of the underlying printer,
 *       or -1 if a format error occurred, alongside setting errno to EILSEQ. */
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL wprinter_print)(char const *__restrict __data, __size_t __datalen, void *__closure);

#ifdef __USE_UTF
#define C16PRINTER_INIT(printer,closure) {printer,closure,NULL,0,__MBSTATE_INIT,NULL}
#define C32PRINTER_INIT(printer,closure) {printer,closure,NULL,0,__MBSTATE_INIT,NULL}
__REDIRECT_TOPE_VOID_ (__LIBC,__PORT_KOSONLY,__LIBCCALL,c16printer_init,(struct c16printer *__restrict wp, pc16formatprinter printer, void *__closure),wprinter_init,(wp,printer,__closure))
__REDIRECT_TOKOS_VOID_(__LIBC,__PORT_KOSONLY,__LIBCCALL,c32printer_init,(struct c32printer *__restrict wp, pc32formatprinter printer, void *__closure),wprinter_init,(wp,printer,__closure))
__REDIRECT_TOPE_VOID_ (__LIBC,__PORT_KOSONLY,__LIBCCALL,c16printer_fini,(struct c16printer *__restrict wp),wprinter_fini,(wp))
__REDIRECT_TOKOS_VOID_(__LIBC,__PORT_KOSONLY,__LIBCCALL,c32printer_fini,(struct c32printer *__restrict wp),wprinter_fini,(wp))
__REDIRECT_TOPE_ (__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,c16printer_print,(char const *__restrict __data, __size_t __datalen, void *__closure),wprinter_print,(__data,__datalen,__closure))
__REDIRECT_TOKOS_(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,c32printer_print,(char const *__restrict __data, __size_t __datalen, void *__closure),wprinter_print,(__data,__datalen,__closure))
#endif /* __USE_UTF */



/* Buffered format printing.
 * >> Since format printing is used quite often thoughout the user-space and the kernel,
 *    many less-than optimized print generators are often chained together with fairly
 *    slow print receivers.
 *    To speed up performance by bunching together a whole lot of data, a buffer
 *    can be used to automatically collect data until it is flushed, or deleted:
 * HINT: If the buffer fails to allocate memory, it will try to flush itself and
 *       attempt to allocate memory again. If this still fails, print commands
 *       are passed through directly, meaning that the buffer is still going to
 *       generated the desired output, be it with less efficient throughput.
 *
 * >> struct buffer *log_buffer;
 * >> log_buffer = buffer_new(&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_WARNING));
 * >> // 'format_printf' is unbuffered, meaning that normally each component would call
 * >> // 'syslog_printer()', resulting in a total to 7 calls: "a" "foo" ",b" "bar" ",c" "foobar" "\n"
 * >> // Using a buffer, this function returning.
 * >> format_printf(&buffer_print,log_buffer,"a%s,b%s,c%s\n","foo","bar","foobar");
 * 
 * WARNING: Buffers are themself not thread-safe. They are intended for local
 *          use, or require the caller to perform their own synchronization.
 */
struct buffer {
 pformatprinter b_printer; /*< [1..1] The underlying printer. */
 void          *b_closure; /*< [?..?] The closure argument passed to 'b_printer' */
union{
 __uintptr_t  __b_align0;  /*< ... */
 __ssize_t      b_state;   /*< The current printer state (< 0: Last error code returned by 'b_printer'; >= 0: Sum of 'b_printer' callbacks). */
};
 char          *b_buffer;  /*< [0..1][owned] Base-pointer of the allocated buffer. */
 char          *b_bufpos;  /*< [0..1][>= b_buffer && <= b_bufend] The current buffer position (Pointer to the buffer byte written to next). */
 char          *b_bufend;  /*< [0..1] End of the allocated buffer (first byte no longer apart of the buffer). */
 void          *__padding; /*< ... (Forward-compatibility & align to '8*sizeof(void *)', which is quite the pretty number) */
};

/* Static initializer for buffers. */
#define BUFFER_INIT(printer,closure) {printer,closure,0,NULL,NULL,NULL,NULL}

/* Initialize the given buffer using the provided PRINTER and CLOSURE.
 * NOTE: Even though possible, it is unwise to chain
 *       multiple buffers, or a buffer and a 'stringprinter' */
__LIBC __PORT_KOSONLY void (__LIBCCALL buffer_init)(struct buffer *__restrict self,
                                                    pformatprinter __printer, void *__closure);

/* NOTE: 'buffer_fini()' will automatically flush the buffer one last time,
 *        and return the total sum of return values from all calls to PRINTER,
 *        or the first error-code generated.
 * NOTE:  This function _MUST_ be called to prevent memory leaks.
 * HINT:  With that in mind and the fact that 'buffer_print' is a no-op after an
 *        error occurred, printer return values can always be ignored safely,
 *        centralizing error handling to this call. */
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL buffer_fini)(struct buffer *__restrict __buf);

/* Flush will write any unwritten data and return the sum of PRINTER
 * return values that were required to perform the flush.
 * NOTE: When the buffer is in a error state, return the PRINTER error code
 *       that caused the error state and don't change the buffer's actual state.
 * WARNING: Make sure to flush data before assuming that is has been printed.
 *          Else, the same corner-cases as possible with fflush() could arise.
 * @return: <  0 : ERROR: The current PRINTER error code.
 * @return: >= 0 : The sum of PRINTER return values called when flushing data. */
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL buffer_flush)(struct buffer *__restrict __buf);

/* The main printer callback used for printing to a buffer.
 * Pass a pointer to the buffer for 'CLOSURE' */
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL buffer_print)(char const *__restrict __data,
                                                          __size_t __datalen, void *__closure);

/* Same as the original functions above, but use a temporary
 * buffer in-between to reduce potential printer overhead. */
__LIBC __PORT_KOSONLY __NONNULL((1,3)) __ssize_t (__ATTR_CDECL format_bprintf)(pformatprinter __printer, void *__closure, char const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __NONNULL((1,3)) __ssize_t (__LIBCCALL format_vbprintf)(pformatprinter __printer, void *__closure, char const *__restrict __format, __VA_LIST __args);
#endif /* !__KERNEL__ */

#endif /* __CC__ */

#undef __FORMAT_PRINTER_DECL

__SYSDECL_END

#ifdef __USE_PRIVATE_FORMAT_PRINTER
/* Pull in our own format-printer implementation. */
#define __LIBC_FORMAT_PRINTER_INCLUDE_MAGIC 0xdeadbeef
#include <libc/format-printer.h>
#undef __LIBC_FORMAT_PRINTER_INCLUDE_MAGIC
#endif /* __USE_PRIVATE_FORMAT_PRINTER */

#endif /* !_FORMAT_PRINTER_H */
