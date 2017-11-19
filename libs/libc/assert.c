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
#ifndef GUARD_LIBS_LIBC_ASSERT_C
#define GUARD_LIBS_LIBC_ASSERT_C 1
#define _KOS_SOURCE 2

#include "libc.h"
#include "format-printer.h"
#include "misc.h"
#include "assert.h"
#include "string.h"
#include <hybrid/host.h>
#include <hybrid/section.h>
#include <hybrid/atomic.h>

#ifndef __KERNEL__
#include "unicode.h"
#endif /* !__KERNEL__ */

DECL_BEGIN

#if defined(__i386__) || defined(__x86_64__)
#   include "arch/i386-kos/assert.c.inl"
#elif defined(__arm__)
#   include "arch/arm-kos/assert.c.inl"
#endif


/* Stub/fallback implementations. */
#ifndef HAVE_LIBC_DEBUG_PRINT
#define HAVE_LIBC_DEBUG_PRINT 1
#undef libc_debug_print
INTERN ATTR_COLDTEXT ssize_t LIBCCALL
libc_debug_print(char const *__restrict data, size_t datalen,
                 void *UNUSED(ignored_closure)) {
#ifdef __KERNEL__
 return syslog_printer(data,datalen,SYSLOG_PRINTER_CLOSURE(LOG_EMERG));
#else
 return libc_syslog_printer(data,datalen,SYSLOG_PRINTER_CLOSURE(LOG_EMERG));
#endif
}
#endif /* !HAVE_LIBC_DEBUG_PRINT */


#ifndef HAVE_LIBC_DEBUG_VPRINTF
#define HAVE_LIBC_DEBUG_VPRINTF 1
#undef libc_debug_vprintf
INTERN ATTR_COLDTEXT void LIBCCALL
libc_debug_vprintf(char const *__restrict format, __builtin_va_list args) {
 libc_format_vprintf(&libc_debug_print,NULL,format,args);
}
#endif /* !HAVE_LIBC_DEBUG_VPRINTF */


#ifndef HAVE_LIBC_DEBUG_PRINTF
#define HAVE_LIBC_DEBUG_PRINTF 1
#undef libc_debug_printf
INTERN void ATTR_CDECL
libc_debug_printf(char const *__restrict format, ...) {
 va_list args;
 va_start(args,format);
 debug_vprintf(format,args);
 va_end(args);
}
#endif /* !HAVE_LIBC_DEBUG_PRINTF */


#ifndef HAVE_LIBC_DEBUG_TBPRINT
#define HAVE_LIBC_DEBUG_TBPRINT 1
#undef libc_debug_tbprint
INTERN ATTR_COLDTEXT void LIBCCALL libc_debug_tbprint(void) {}
#endif /* !HAVE_LIBC_DEBUG_TBPRINT */


#ifndef HAVE_LIBC_DEBUG_TBPRINT2
#define HAVE_LIBC_DEBUG_TBPRINT2 1
#undef libc_debug_tbprint2
INTERN void LIBCCALL libc_debug_tbprint2(void const *UNUSED(ebp), size_t UNUSED(n_skip)) {}
#endif /* !HAVE_LIBC_DEBUG_TBPRINT2 */


#ifndef HAVE_LIBC_DEBUG_TBPRINTL
#define HAVE_LIBC_DEBUG_TBPRINTL 1
#undef libc_debug_tbprintl
INTERN ATTR_COLDTEXT void LIBCCALL
libc_debug_tbprintl(void const *xip, void const *frame, size_t tb_id) {
#ifdef CONFIG_USE_EXTERNAL_ADDR2LINE
 if (frame) {
  debug_printf("#!$ addr2line(%Ix) '{file}({line}) : {func} : [%Ix] : %p : %p'\n",
                    (uintptr_t)xip-1,tb_id,xip,frame);
 } else {
  debug_printf("#!$ addr2line(%Ix) '{file}({line}) : {func} : [%Ix] : %p'\n",
              (uintptr_t)xip-1,tb_id,xip);
 }
#else /* CONFIG_USE_EXTERNAL_ADDR2LINE */
 if (frame) {
  debug_printf("%[vinfo] : [%Ix] : %p : %p\n",(uintptr_t)xip-1,tb_id,xip,frame);
 } else {
  debug_printf("%[vinfo] : [%Ix] : %p\n",(uintptr_t)xip-1,tb_id,xip);
 }
#endif /* !CONFIG_USE_EXTERNAL_ADDR2LINE */
}
#endif /* !HAVE_LIBC_DEBUG_TBPRINTL */


#ifndef HAVE_LIBC___ASSERTION_UNREACHABLE
#define HAVE_LIBC___ASSERTION_UNREACHABLE 1
#undef libc___assertion_unreachable
INTERN ATTR_COLDTEXT ATTR_NORETURN ATTR_NOINLINE void LIBCCALL
libc___assertion_unreachable(void) {
 va_list empty; libc_memset(&empty,0,sizeof(empty));
 libc_assertion_corefail("__builtin_unreachable()",DEBUGINFO_NUL,NULL,empty);
}
#endif /* !HAVE_LIBC___ASSERTION_UNREACHABLE */


#ifndef HAVE_LIBC___ASSERTION_FAILED
#define HAVE_LIBC___ASSERTION_FAILED 1
#undef libc___assertion_failed
INTERN ATTR_COLDTEXT ATTR_NORETURN ATTR_NOINLINE void LIBCCALL
libc___assertion_failed(char const *expr, DEBUGINFO) {
 va_list empty; libc_memset(&empty,0,sizeof(empty));
 libc_assertion_corefail(expr,DEBUGINFO_FWD,NULL,empty);
}
#endif /* !HAVE_LIBC___ASSERTION_FAILED */


#ifndef HAVE_LIBC___ASSERTION_FAILEDF
#define HAVE_LIBC___ASSERTION_FAILEDF 1
#undef libc___assertion_failedf
INTERN ATTR_COLDTEXT ATTR_NORETURN ATTR_NOINLINE void ATTR_CDECL
libc___assertion_failedf(char const *expr, DEBUGINFO, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_assertion_corefail(expr,DEBUGINFO_FWD,format,args);
 __builtin_unreachable();
}
#endif /* !HAVE_LIBC___ASSERTION_FAILEDF */


#ifndef HAVE___STACK_CHK_GUARD
#define HAVE___STACK_CHK_GUARD 1
#undef __stack_chk_guard
#if __SIZEOF_POINTER__ == 4
PUBLIC ATTR_COLDDATA uintptr_t __stack_chk_guard = 0xe2dee396;
#else
PUBLIC ATTR_COLDDATA uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;
#endif
#endif /* !HAVE___STACK_CHK_GUARD */


#ifndef HAVE___STACK_CHK_FAIL
#define HAVE___STACK_CHK_FAIL 1
#undef __stack_chk_fail
PUBLIC ATTR_COLDTEXT ATTR_NORETURN ATTR_NOINLINE void __stack_chk_fail(void) {
 va_list empty; libc_memset(&empty,0,sizeof(empty));
 libc_assertion_corefail("STACK VIOLATION",DEBUGINFO_NUL,NULL,empty);
}
#endif /* !HAVE___STACK_CHK_FAIL */





#ifndef HAVE_LIBC_ASSERTION_COREFAIL
#define HAVE_LIBC_ASSERTION_COREFAIL 1
#undef libc_assertion_corefail
INTERN ATTR_COLDTEXT ATTR_NORETURN ATTR_NOINLINE void LIBCCALL
libc_assertion_corefail(char const *expr, DEBUGINFO_MUNUSED,
                        char const *format, va_list args) {
 PRIVATE ATTR_COLDBSS unsigned int in_assertion_core = 0;
 register unsigned int level = ATOMIC_FETCHINC(in_assertion_core);
 if (level == 0) {
  libc_debug_printf("\n\n%s(%d) : %q : Assertion failed : %q\n",
                    __file,__line,__func,expr);
  if (format) {
   libc_debug_vprintf(format,args);
   libc_debug_print("\n",1,NULL);
  }
  libc_debug_tbprint();
 } else if (level == 1) {
  char buffer[__SIZEOF_POINTER__*2],*iter;
  uintptr_t return_address;
  libc_debug_print("\n\nASSERTION CORE RECURSION\n",27,NULL);
  return_address = (uintptr_t)__builtin_return_address(0);
  iter = COMPILER_ENDOF(buffer);
  while (iter-- != buffer) {
   uintptr_t temp;
   temp = return_address % 16;
   return_address /= 16;
   if (temp >= 10) *iter = (char)('A'+(temp-10));
   else            *iter = (char)('0'+temp);
  }
  ++iter;
  libc_debug_print("IP = ",6,NULL);
  libc_debug_print(iter,COMPILER_ENDOF(buffer)-iter,NULL);
  libc_debug_print("\n",1,NULL);
 }
 for (;;) {}
}
#endif /* !HAVE_LIBC_ASSERTION_COREFAIL */




#undef __stack_chk_fail_local
#undef __assertion_unreachable
#undef __afail
#undef __afailf
#undef debug_print
#undef debug_printf
#undef debug_vprintf
#undef debug_tbprintl
#undef debug_tbprint2
#undef debug_tbprint
DEFINE_PUBLIC_ALIAS(__stack_chk_fail_local,__stack_chk_fail);
DEFINE_PUBLIC_ALIAS(__assertion_unreachable,libc___assertion_unreachable);
DEFINE_PUBLIC_ALIAS(debug_print,libc_debug_print);
DEFINE_PUBLIC_ALIAS(debug_printf,libc_debug_printf);
DEFINE_PUBLIC_ALIAS(debug_vprintf,libc_debug_vprintf);
DEFINE_PUBLIC_ALIAS(__afail,libc___assertion_failed);
DEFINE_PUBLIC_ALIAS(__afailf,libc___assertion_failedf);
DEFINE_PUBLIC_ALIAS(debug_tbprintl,libc_debug_tbprintl);
DEFINE_PUBLIC_ALIAS(debug_tbprint2,libc_debug_tbprint2);
DEFINE_PUBLIC_ALIAS(debug_tbprint,libc_debug_tbprint);

/*
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(__assertion_failed,libc___assertion_failed);
DEFINE_PUBLIC_ALIAS(__assertion_failedf,libc___assertion_failed);
#endif
*/

#ifndef __KERNEL__
INTERN ATTR_COLDTEXT ATTR_NORETURN ATTR_NOINLINE void LIBCCALL
libc_assert(char const *msg, char const *file, unsigned int line) {
 va_list empty; libc_memset(&empty,0,sizeof(empty));
 libc_assertion_corefail(msg,file,(int)line,NULL,NULL,empty);
}
INTERN ATTR_COLDTEXT ATTR_NORETURN ATTR_NOINLINE void LIBCCALL
libc_assert_perror_fail(int errnum, const char *file,
                        unsigned int line, const char *function) {
 va_list empty; libc_memset(&empty,0,sizeof(empty));
 libc_assertion_corefail("TODO",file,(int)line,function,NULL,empty);
}
DEFINE_PUBLIC_ALIAS(__assert_perror_fail,libc_assert_perror_fail);
DEFINE_PUBLIC_ALIAS(__assert,libc_assert);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(_assert,libc_assert);
INTERN ATTR_DOSTEXT ATTR_NORETURN ATTR_NOINLINE
void LIBCCALL libc_16wassert(char16_t const *msg, char16_t const *file, u32 line) {
 va_list empty; libc_memset(&empty,0,sizeof(empty));
 libc_assertion_corefail(libc_utf16to8m(msg),libc_utf16to8m(file),
                        (int)line,NULL,NULL,empty);
}

/* NOTE: We're only exporting 16-bit '_wassert', because
 *       this is only for binary compatibility with DOS. */
DEFINE_PUBLIC_ALIAS(_wassert,libc_16wassert);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_ASSERT_C */
