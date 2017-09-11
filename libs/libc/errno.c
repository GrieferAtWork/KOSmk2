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

static int __ERRNO = 0; /* TODO: Thread-local. */
PUBLIC int *(LIBCCALL __errno)(void) { return &__ERRNO; }
PUBLIC int (LIBCCALL __get_errno)(void) {
 return __ERRNO;
}
PUBLIC void (LIBCCALL __set_errno)(int err) {
#if 0
 syslogf(LOG_DEBUG,"__set_errno(%[errno])\n",err);
 __asm__("int $3");
#endif
 __ERRNO = err;
}

#define ERROR_EXIT(code) _exit(code)

PUBLIC char *(LIBCCALL __libc_program_invocation_name)(void) {
 return appenv->e_argc ? appenv->e_argv[0] : "";
}
PUBLIC char *(LIBCCALL __libc_program_invocation_short_name)(void) {
 return basename(program_invocation_name);
}

PUBLIC void (LIBCCALL vwarn)(char const *format, va_list args) {
 fprintf(stderr,"%s: ",program_invocation_short_name);
 vfprintf(stderr,format,args);
 fprintf(stderr,": %[errno]",errno);
}
PUBLIC void (LIBCCALL vwarnx)(char const *format, va_list args) {
 fprintf(stderr,"%s: ",program_invocation_short_name);
 vfprintf(stderr,format,args);
}
PUBLIC void (LIBCCALL verr)(int status, char const *format, va_list args) {
 vwarn(format,args);
 ERROR_EXIT(status);
}
PUBLIC void (LIBCCALL verrx)(int status, char const *format, va_list args) {
 vwarnx(format,args);
 ERROR_EXIT(status);
}
PUBLIC void (LIBCCALL warn)(char const *format, ...) {
 va_list args;
 va_start(args,format);
 vwarn(format,args);
 va_end(args);
}
PUBLIC void (LIBCCALL warnx)(char const *format, ...) {
 va_list args;
 va_start(args,format);
 vwarnx(format,args);
 va_end(args);
}
PUBLIC void (LIBCCALL err)(int status, char const *format, ...) {
 va_list args;
 va_start(args,format);
 verr(status,format,args);
}
PUBLIC void (LIBCCALL errx)(int status, char const *format, ...) {
 va_list args;
 va_start(args,format);
 verrx(status,format,args);
}

PUBLIC ATTR_COLDDATA unsigned int error_message_count = 0;
PUBLIC ATTR_COLDDATA int error_one_per_line = 0;
PUBLIC ATTR_COLDDATA void (*error_print_progname)(void) = NULL;

PRIVATE ATTR_COLDTEXT void LIBCCALL error_prefix(void) {
 void (*print_name)(void) = error_print_progname;
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
 ++error_message_count;
 if (status) ERROR_EXIT(status);
}

PUBLIC ATTR_COLDTEXT void LIBCCALL
error(int status, int errnum, char const *format, ...) {
 va_list args;
 error_prefix();
 fwrite(": ",sizeof(char),2,stderr);
 va_start(args,format);
 vfprintf(stderr,format,args);
 va_end(args);
 error_suffix(status,errnum);
}
PUBLIC ATTR_COLDTEXT void LIBCCALL
error_at_line(int status, int errnum, char const *fname,
              unsigned int lineno, char const *format, ...) {
 va_list args;
 error_prefix();
 fprintf(stderr,":%s:%d: ",fname,lineno);
 va_start(args,format);
 vfprintf(stderr,format,args);
 va_end(args);
 error_suffix(status,errnum);
}



DECL_END

#endif /* !GUARD_LIBS_LIBC_ERRNO_C */
