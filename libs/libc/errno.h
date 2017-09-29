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
#include <hybrid/types.h>
#include <stdarg.h>
#include <stddef.h>

DECL_BEGIN

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

INTDEF errno_t *LIBCCALL libc___errno(void);
INTDEF errno_t LIBCCALL libc___get_errno(void);
INTDEF errno_t LIBCCALL libc___set_errno(errno_t err);
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
INTDEF ATTR_COLDTEXT void LIBCCALL libc_error(int status, errno_t errnum, char const *format, ...);
INTDEF ATTR_COLDTEXT void LIBCCALL libc_error_at_line(int status, errno_t errnum, char const *fname, unsigned int lineno, char const *format, ...);

#define GET_ERRNO()        libc___get_errno()
#define SET_ERRNO(err)     libc___set_errno(err)

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#define GET_DOS_ERRNO()    libc_dos___get_errno()
#define SET_DOS_ERRNO(err) libc_dos___set_errno(err)
INTDEF errno_t *LIBCCALL libc_dos___errno(void);
INTDEF errno_t  LIBCCALL libc_dos___get_errno(void);
INTDEF errno_t  LIBCCALL libc_dos___set_errno(errno_t err);
INTDEF errno_t  LIBCCALL libc_errno_dos2kos(errno_t eno);
INTDEF errno_t  LIBCCALL libc_errno_kos2dos(errno_t eno);
INTDEF u32 LIBCCALL libc_errno_kos2nt(errno_t eno);

INTDEF u32    *LIBCCALL libc_dos___doserrno(void);
INTDEF errno_t LIBCCALL libc_dos___get_doserrno(u32 *__restrict perr);
INTDEF errno_t LIBCCALL libc_dos___set_doserrno(u32 err);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_ERRNO_H */
