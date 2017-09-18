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
#ifndef GUARD_LIBS_LIBC_ERRNO_C
#define GUARD_LIBS_LIBC_ERRNO_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include "libc.h"

#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <kos/environ.h>
#include <err.h>
#include <stdarg.h>

DECL_BEGIN

PRIVATE int __ERRNO = 0; /* TODO: Thread-local. */
INTERN int *LIBCCALL libc__errno(void) { return &__ERRNO; }
INTERN int LIBCCALL libc___get_errno(void) {
 return __ERRNO;
}
INTERN void LIBCCALL libc___set_errno(int err) {
#if 0
 syslog(LOG_DEBUG,"SET_ERRNO(%[errno])\n",err);
 __asm__("int $3");
#endif
 __ERRNO = err;
}

#define ERROR_EXIT(code) _exit(code)

INTERN char *LIBCCALL libc___libc_program_invocation_name(void) {
 return appenv->e_argc ? appenv->e_argv[0] : "";
}
INTERN char *LIBCCALL libc___libc_program_invocation_short_name(void) {
 return basename(program_invocation_name);
}

INTERN void LIBCCALL libc_vwarn(char const *format, va_list args) {
 fprintf(stderr,"%s: ",program_invocation_short_name);
 vfprintf(stderr,format,args);
 fprintf(stderr,": %[errno]",errno);
}
INTERN void LIBCCALL libc_vwarnx(char const *format, va_list args) {
 fprintf(stderr,"%s: ",program_invocation_short_name);
 vfprintf(stderr,format,args);
}
INTERN void LIBCCALL libc_verr(int status, char const *format, va_list args) {
 vwarn(format,args);
 ERROR_EXIT(status);
}
INTERN void LIBCCALL libc_verrx(int status, char const *format, va_list args) {
 vwarnx(format,args);
 ERROR_EXIT(status);
}
INTERN void LIBCCALL libc_warn(char const *format, ...) {
 va_list args;
 va_start(args,format);
 vwarn(format,args);
 va_end(args);
}
INTERN void LIBCCALL libc_warnx(char const *format, ...) {
 va_list args;
 va_start(args,format);
 vwarnx(format,args);
 va_end(args);
}
INTERN void LIBCCALL libc_err(int status, char const *format, ...) {
 va_list args;
 va_start(args,format);
 verr(status,format,args);
}
INTERN void LIBCCALL libc_errx(int status, char const *format, ...) {
 va_list args;
 va_start(args,format);
 verrx(status,format,args);
}

INTERN ATTR_COLDDATA unsigned int libc_error_message_count = 0;
INTERN ATTR_COLDDATA int libc_error_one_per_line = 0;
INTERN ATTR_COLDDATA void (*libc_error_print_progname)(void) = NULL;
PRIVATE ATTR_COLDTEXT void LIBCCALL error_prefix(void) {
 void (*print_name)(void) = libc_error_print_progname;
 fflush(stdout);
 if (print_name) (*print_name)();
 else fprintf(stderr,"%s",program_invocation_short_name);
}
PRIVATE ATTR_COLDTEXT void LIBCCALL
error_suffix(int status, int errnum) {
#if 1
 fprintf(stderr,": %[errno]\n",errnum);
#else
 fprintf(stderr,": %s\n",strerror(errnum));
#endif
 ++libc_error_message_count;
 if (status) ERROR_EXIT(status);
}

INTERN ATTR_COLDTEXT void LIBCCALL
libc_error(int status, int errnum, char const *format, ...) {
 va_list args;
 error_prefix();
 fwrite(": ",sizeof(char),2,stderr);
 va_start(args,format);
 vfprintf(stderr,format,args);
 va_end(args);
 error_suffix(status,errnum);
}
INTERN ATTR_COLDTEXT void LIBCCALL
libc_error_at_line(int status, int errnum, char const *fname,
                   unsigned int lineno, char const *format, ...) {
 va_list args;
 error_prefix();
 fprintf(stderr,":%s:%d: ",fname,lineno);
 va_start(args,format);
 vfprintf(stderr,format,args);
 va_end(args);
 error_suffix(status,errnum);
}


DEFINE_PUBLIC_ALIAS(_errno,libc__errno);
DEFINE_PUBLIC_ALIAS(__get_errno,libc___get_errno);
DEFINE_PUBLIC_ALIAS(__set_errno,libc___set_errno);
DEFINE_PUBLIC_ALIAS(__libc_program_invocation_name,libc___libc_program_invocation_name);
DEFINE_PUBLIC_ALIAS(__libc_program_invocation_short_name,libc___libc_program_invocation_short_name);
DEFINE_PUBLIC_ALIAS(vwarn,libc_vwarn);
DEFINE_PUBLIC_ALIAS(vwarnx,libc_vwarnx);
DEFINE_PUBLIC_ALIAS(verr,libc_verr);
DEFINE_PUBLIC_ALIAS(verrx,libc_verrx);
DEFINE_PUBLIC_ALIAS(warn,libc_warn);
DEFINE_PUBLIC_ALIAS(warnx,libc_warnx);
DEFINE_PUBLIC_ALIAS(err,libc_err);
DEFINE_PUBLIC_ALIAS(errx,libc_errx);
DEFINE_PUBLIC_ALIAS(error_message_count ,libc_error_message_count);
DEFINE_PUBLIC_ALIAS(error_one_per_line ,libc_error_one_per_line);
DEFINE_PUBLIC_ALIAS(error_print_progname,libc_error_print_progname);
DEFINE_PUBLIC_ALIAS(error,libc_error);
DEFINE_PUBLIC_ALIAS(error_at_line,libc_error_at_line);

DECL_END

#endif /* !GUARD_LIBS_LIBC_ERRNO_C */
