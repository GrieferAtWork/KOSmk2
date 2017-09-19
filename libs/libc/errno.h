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
#ifndef GUARD_LIBS_LIBC_ERRNO_H
#define GUARD_LIBS_LIBC_ERRNO_H 1

#include "libc.h"
#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <stdarg.h>
#include <stddef.h>

DECL_BEGIN

INTDEF int *LIBCCALL libc__errno(void);
INTDEF int LIBCCALL libc___get_errno(void);
INTDEF void LIBCCALL libc___set_errno(int err);
INTDEF char *LIBCCALL libc___libc_program_invocation_name(void);
INTDEF char *LIBCCALL libc___libc_program_invocation_short_name(void);
INTDEF void LIBCCALL libc_vwarn(char const *format, va_list args);
INTDEF void LIBCCALL libc_vwarnx(char const *format, va_list args);
INTDEF void LIBCCALL libc_verr(int status, char const *format, va_list args);
INTDEF void LIBCCALL libc_verrx(int status, char const *format, va_list args);
INTDEF void LIBCCALL libc_warn(char const *format, ...);
INTDEF void LIBCCALL libc_warnx(char const *format, ...);
INTDEF void LIBCCALL libc_err(int status, char const *format, ...);
INTDEF void LIBCCALL libc_errx(int status, char const *format, ...);
INTDEF ATTR_COLDTEXT void LIBCCALL libc_error(int status, int errnum, char const *format, ...);
INTDEF ATTR_COLDTEXT void LIBCCALL libc_error_at_line(int status, int errnum, char const *fname, unsigned int lineno, char const *format, ...);

#define GET_ERRNO()    libc___get_errno()
#define SET_ERRNO(err) libc___set_errno(err)

DECL_END

#endif /* !GUARD_LIBS_LIBC_ERRNO_H */
