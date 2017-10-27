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
#ifndef _ERROR_H
#define _ERROR_H 1

#include <features.h>

#ifndef __CRT_GLC
#error "<error.h> is not supported by the linked libc"
#endif /* !__CRT_GLC */

__SYSDECL_BEGIN

/* Print an error message using 'fprintf(stderr,FORMAT,...)';
 * @param: ERRNUM: When nonzero, printf(": %s",strerror(ERRNUM)).
 * @param: STATUS: When nonzero, exit(STATUS) and never return.  */
#ifndef __NO_ASMNAME
__NAMESPACE_INT_BEGIN
__LIBC __ATTR_COLD void (__LIBCCALL __libc_error)(int __status, int __errnum, char const *__format, ...) __ASMNAME("error");
__LIBC __ATTR_COLD void (__LIBCCALL __libc_error_at_line)(int __status, int __errnum, char const *__fname, unsigned int __lineno, char const *__format, ...) __ASMNAME("error_at_line");
__LIBC __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL __nret_error)(int __status, int __errnum, char const *__format, ...) __ASMNAME("error");
__LIBC __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL __nret_error_at_line)(int __status, int __errnum, char const *__fname, unsigned int __lineno, char const *__format, ...) __ASMNAME("error_at_line");
__NAMESPACE_INT_END

/* Inform the compiler when error() won't return. */
#define error(status,errnum,...) \
       (__builtin_constant_p(status) && (status) != 0 \
      ? __NAMESPACE_INT_SYM __nret_error(status,errnum,__VA_ARGS__) \
      : __NAMESPACE_INT_SYM __libc_error(status,errnum,__VA_ARGS__))
#define error_at_line(status,errnum,fname,lineno,...) \
       (__builtin_constant_p(status) && (status) != 0 \
      ? __NAMESPACE_INT_SYM __nret_error_at_line(status,errnum,fname,lineno,__VA_ARGS__) \
      : __NAMESPACE_INT_SYM __libc_error_at_line(status,errnum,fname,lineno,__VA_ARGS__))
#else
__LIBC __ATTR_COLD void (__LIBCCALL error)(int __status, int __errnum, char const *__format, ...);
__LIBC __ATTR_COLD void (__LIBCCALL error_at_line)(int __status, int __errnum, char const *__fname, unsigned int __lineno, char const *__format, ...);
#endif


#ifdef __CRT_GLC
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("error_print_progname")
#pragma push_macro("error_message_count")
#pragma push_macro("error_one_per_line")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#undef error_print_progname
#undef error_message_count
#undef error_one_per_line

/* Optional user-callback that may override the following default:
 * >> fflush(stdout);
 * >> fprintf(stderr,"%s: ",argv[0]); */
__LIBC void (*error_print_progname)(void);

__LIBC unsigned int error_message_count; /* Incremented each time error() is called. */
__LIBC int error_one_per_line;

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("error_one_per_line")
#pragma pop_macro("error_message_count")
#pragma pop_macro("error_print_progname")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif /* !__CRT_GLC */

__SYSDECL_END

#endif /* !_ERROR_H */
