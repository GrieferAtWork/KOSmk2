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
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "stdlib.h"

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kos/environ.h>
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

#define ERROR_EXIT(code) libc__exit(code)

INTERN char *LIBCCALL libc___libc_program_invocation_name(void) {
 return appenv->e_argc ? appenv->e_argv[0] : "";
}
INTERN char *LIBCCALL libc___libc_program_invocation_short_name(void) {
 return libc_basename(libc___libc_program_invocation_name());
}

INTERN void LIBCCALL libc_vwarn(char const *format, va_list args) {
 libc_fprintf(stderr,"%s: ",libc___libc_program_invocation_short_name());
 libc_vfprintf(stderr,format,args);
 libc_fprintf(stderr,": %[errno]",GET_ERRNO());
}
INTERN void LIBCCALL libc_vwarnx(char const *format, va_list args) {
 libc_fprintf(stderr,"%s: ",libc___libc_program_invocation_short_name());
 libc_vfprintf(stderr,format,args);
}
INTERN void LIBCCALL libc_verr(int status, char const *format, va_list args) {
 libc_vwarn(format,args);
 ERROR_EXIT(status);
}
INTERN void LIBCCALL libc_verrx(int status, char const *format, va_list args) {
 libc_vwarnx(format,args);
 ERROR_EXIT(status);
}
INTERN void LIBCCALL libc_warn(char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_vwarn(format,args);
 va_end(args);
}
INTERN void LIBCCALL libc_warnx(char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_vwarnx(format,args);
 va_end(args);
}
INTERN void LIBCCALL libc_err(int status, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_verr(status,format,args);
}
INTERN void LIBCCALL libc_errx(int status, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_verrx(status,format,args);
}

PUBLIC ATTR_COLDDATA unsigned int error_message_count = 0;
PUBLIC ATTR_COLDDATA int          error_one_per_line = 0;
PUBLIC ATTR_COLDDATA void       (*error_print_progname)(void) = NULL;
PRIVATE ATTR_COLDTEXT void LIBCCALL error_prefix(void) {
 void (*print_name)(void) = error_print_progname;
 libc_fflush(stdout);
 if (print_name) (*print_name)();
 else libc_fprintf(stderr,"%s",libc___libc_program_invocation_short_name());
}
PRIVATE ATTR_COLDTEXT void LIBCCALL
error_suffix(int status, int errnum) {
#if 1
 libc_fprintf(stderr,": %[errno]\n",errnum);
#else
 libc_fprintf(stderr,": %s\n",strerror(errnum));
#endif
 ++error_message_count;
 if (status) ERROR_EXIT(status);
}

INTERN ATTR_COLDTEXT void LIBCCALL
libc_error(int status, int errnum, char const *format, ...) {
 va_list args;
 error_prefix();
 libc_fwrite(": ",sizeof(char),2,stderr);
 va_start(args,format);
 libc_vfprintf(stderr,format,args);
 va_end(args);
 error_suffix(status,errnum);
}
INTERN ATTR_COLDTEXT void LIBCCALL
libc_error_at_line(int status, int errnum, char const *fname,
                   unsigned int lineno, char const *format, ...) {
 va_list args;
 error_prefix();
 libc_fprintf(stderr,":%s:%d: ",fname,lineno);
 va_start(args,format);
 libc_vfprintf(stderr,format,args);
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
DEFINE_PUBLIC_ALIAS(error,libc_error);
DEFINE_PUBLIC_ALIAS(error_at_line,libc_error_at_line);

DECL_END

#endif /* !GUARD_LIBS_LIBC_ERRNO_C */
