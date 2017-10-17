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
#include "file.h"
#include "errno.h"
#include "string.h"
#include "stdlib.h"

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kos/environ.h>
#include <stdarg.h>
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include <bits/dos-errno.h>
#include <winapi/winerror.h>
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_BEGIN

#ifndef CONFIG_LIBC_NO_DOS_LIBC
PRIVATE bool libc_errno_isdos = false;
PRIVATE errno_t libc_errnoval = EOK; /* TODO: Thread-local. */
INTERN errno_t *LIBCCALL libc_errno(void) {
 if (libc_errno_isdos) {
  libc_errnoval = libc_errno_dos2kos(libc_errnoval);
  libc_errno_isdos = false;
 }
 return &libc_errnoval;
}
INTERN errno_t LIBCCALL libc_get_errno(void) {
 if (libc_errno_isdos) {
  libc_errnoval = libc_errno_dos2kos(libc_errnoval);
  libc_errno_isdos = false;
 }
 return libc_errnoval;
}
INTERN errno_t LIBCCALL libc_set_errno(errno_t err) {
#if 0
 syslog(LOG_DEBUG,"SET_ERRNO(%[errno])\n",err);
 __asm__("int $3");
#endif
 libc_errnoval = err;
 libc_errno_isdos = false;
 return EOK;
}
#else
PRIVATE errno_t libc_errnoval = 0; /* TODO: Thread-local. */
INTERN errno_t *LIBCCALL libc_errno(void) { return &libc_errnoval; }
INTERN errno_t LIBCCALL libc_get_errno(void) { return libc_errnoval; }
INTERN errno_t LIBCCALL libc_set_errno(errno_t err) {
#if 0
 syslog(LOG_DEBUG,"SET_ERRNO(%[errno])\n",err);
 __asm__("int $3");
#endif
 libc_errnoval = err;
 return EOK;
}
#endif

#define ERROR_EXIT(code) libc__exit(code)

INTERN char *LIBCCALL libc_program_invocation_name(void) {
 return appenv->e_argc ? appenv->e_argv[0] : "";
}
INTERN char *LIBCCALL libc_program_invocation_short_name(void) {
 return libc_basename(libc_program_invocation_name());
}

INTERN void LIBCCALL libc_vwarn(char const *format, va_list args) {
 libc_fprintf(stderr,"%s: ",libc_program_invocation_short_name());
 libc_vfprintf(stderr,format,args);
 libc_fprintf(stderr,": %[errno]",GET_ERRNO());
}
INTERN void LIBCCALL libc_vwarnx(char const *format, va_list args) {
 libc_fprintf(stderr,"%s: ",libc_program_invocation_short_name());
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

PUBLIC ATTR_COLDDATA unsigned int error_message_count         = 0;
PUBLIC ATTR_COLDDATA int          error_one_per_line          = 0;
PUBLIC ATTR_COLDDATA void       (*error_print_progname)(void) = NULL;
PRIVATE ATTR_COLDTEXT void LIBCCALL error_prefix(void) {
 void (*print_name)(void) = error_print_progname;
 libc_fflush(stdout);
 if (print_name) (*print_name)();
 else libc_fprintf(stderr,"%s",libc_program_invocation_short_name());
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
libc_error(int status, errno_t errnum, char const *format, ...) {
 va_list args;
 error_prefix();
 libc_fwrite(": ",sizeof(char),2,stderr);
 va_start(args,format);
 libc_vfprintf(stderr,format,args);
 va_end(args);
 error_suffix(status,errnum);
}
INTERN ATTR_COLDTEXT void LIBCCALL
libc_error_at_line(int status, errno_t errnum, char const *fname,
                   unsigned int lineno, char const *format, ...) {
 va_list args;
 error_prefix();
 libc_fprintf(stderr,":%s:%d: ",fname,lineno);
 va_start(args,format);
 libc_vfprintf(stderr,format,args);
 va_end(args);
 error_suffix(status,errnum);
}


DEFINE_PUBLIC_ALIAS(_errno,libc_errno);
DEFINE_PUBLIC_ALIAS(__get_errno,libc_get_errno);
DEFINE_PUBLIC_ALIAS(_set_errno,libc_set_errno);
DEFINE_PUBLIC_ALIAS(__libc_program_invocation_name,libc_program_invocation_name);
DEFINE_PUBLIC_ALIAS(__libc_program_invocation_short_name,libc_program_invocation_short_name);
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

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"

#define DOS_ERRNO_NOT_SUPPORTED __DOS_ENOTSUP
#define KOS_ERRNO_NOT_SUPPORTED EINVAL
#define NT_ERRNO_NOT_SUPPORTED  ERROR_FUNCTION_FAILED

PRIVATE ATTR_DOSRODATA u8 const vec_errno_dos2kos[__DOS_EMAX+1] = {
    [0 ... __DOS_EMAX] = KOS_ERRNO_NOT_SUPPORTED,
#define PAIR(kos,dos,dos2) [dos] = kos,
#include "templates/errno-dospair.code"
};
PRIVATE ATTR_DOSRODATA u8 const vec_errno_kos2dos[__EBASEMAX+1] = {
    [0 ... __EBASEMAX] = DOS_ERRNO_NOT_SUPPORTED,
#define PAIR(kos,dos,dos2) [kos] = dos,
#include "templates/errno-dospair.code"
};
PRIVATE ATTR_DOSRODATA u16 const vec_errno_kos2nt[__EBASEMAX+1] = {
    [0 ... __EBASEMAX] = NT_ERRNO_NOT_SUPPORTED,
#define PAIR(kos,dos,dos2) [kos] = dos2,
#include "templates/errno-dospair.code"
};

#pragma GCC diagnostic pop

INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_get_errno2(errno_t *perr) { if (perr) *perr = libc_errnoval; return EOK; }
DEFINE_PUBLIC_ALIAS(_get_errno,libc_get_errno2);

INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_errno_dos2kos(errno_t eno) { return (unsigned int)eno < COMPILER_LENOF(vec_errno_dos2kos) ? vec_errno_dos2kos[eno] : KOS_ERRNO_NOT_SUPPORTED; }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_errno_kos2dos(errno_t eno) { return (unsigned int)eno < COMPILER_LENOF(vec_errno_kos2dos) ? vec_errno_kos2dos[eno] : DOS_ERRNO_NOT_SUPPORTED; }
INTERN ATTR_DOSTEXT u32 LIBCCALL libc_errno_kos2nt(errno_t eno) { return (unsigned int)eno < COMPILER_LENOF(vec_errno_kos2nt) ? vec_errno_kos2nt[eno] : NT_ERRNO_NOT_SUPPORTED; }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_errno_nt2kos(u32 eno) {
 u16 const *iter; errno_t result = EINVAL;
 /* Translate an NT error code to KOS. */
 for (iter = vec_errno_kos2nt;
      iter != COMPILER_ENDOF(vec_errno_kos2nt); ++iter) {
  if ((u32)*iter == eno) {
   result = (errno_t)(iter-vec_errno_kos2nt);
   break;
  }
 }
 return result;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_errno_nt2dos(u32 eno) {
 u16 const *iter; errno_t result = EINVAL;
 /* Translate an NT error code to KOS. */
 for (iter = vec_errno_kos2nt;
      iter != COMPILER_ENDOF(vec_errno_kos2nt); ++iter) {
  if ((u32)*iter == eno) {
   result = (errno_t)(iter-vec_errno_kos2nt);
   break;
  }
 }
 /* Translate a KOS error code to DOS. */
 return libc_errno_kos2dos(result);
}

INTERN ATTR_DOSTEXT
errno_t *LIBCCALL libc_dos___errno(void) {
 if (!libc_errno_isdos) {
  libc_errnoval = libc_errno_kos2dos(libc_errnoval);
  libc_errno_isdos = true;
 }
 return &libc_errnoval;
}
INTERN ATTR_DOSTEXT
errno_t LIBCCALL libc_dos___get_errno(void) {
 if (!libc_errno_isdos) {
  libc_errnoval = libc_errno_kos2dos(libc_errnoval);
  libc_errno_isdos = true;
 }
 return libc_errnoval;
}
INTERN ATTR_DOSTEXT
errno_t LIBCCALL libc_dos___set_errno(errno_t err) {
 libc_errnoval = err;
 libc_errno_isdos = true;
 return EOK;
}

DEFINE_PUBLIC_ALIAS(__DSYM(_errno),libc_dos___errno);
DEFINE_PUBLIC_ALIAS(__DSYM(_get_errno),libc_dos___get_errno);
DEFINE_PUBLIC_ALIAS(__DSYM(_set_errno),libc_dos___set_errno);
DEFINE_PUBLIC_ALIAS(errno_dos2kos,libc_errno_dos2kos);
DEFINE_PUBLIC_ALIAS(errno_kos2dos,libc_errno_kos2dos);
DEFINE_PUBLIC_ALIAS(errno_kos2nt,libc_errno_kos2nt);
DEFINE_PUBLIC_ALIAS(errno_nt2dos,libc_errno_nt2kos);
DEFINE_PUBLIC_ALIAS(_dosmaperr,libc_errno_nt2dos);


#if EOK != 0
PRIVATE ATTR_DOSDATA errno_t libc_doserrno_last = EOK; /* TODO: Thread-local. */
#else
PRIVATE ATTR_DOSBSS errno_t libc_doserrno_last = EOK; /* TODO: Thread-local. */
#endif
#if ERROR_SUCCESS != 0
PRIVATE ATTR_DOSDATA u32 libc_doserrno = ERROR_SUCCESS; /* TODO: Thread-local. */
#else
PRIVATE ATTR_DOSBSS u32 libc_doserrno = ERROR_SUCCESS; /* TODO: Thread-local. */
#endif
INTERN ATTR_DOSTEXT u32 *LIBCCALL libc_dos___doserrno(void) {
 if (libc_doserrno_last != libc_errnoval) {
  libc_doserrno_last = libc_errno_isdos ? libc_errno_dos2kos(libc_errnoval) : libc_errnoval;
  libc_doserrno = libc_errno_kos2nt(libc_doserrno_last);
 }
 return &libc_doserrno;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_dos___get_doserrno(u32 *__restrict perr) {
 if (libc_errno_isdos) {
  libc_errnoval = libc_errno_dos2kos(libc_errnoval);
  libc_errno_isdos = false;
 }
 if (libc_doserrno_last != libc_errnoval) {
  libc_doserrno_last = libc_errnoval;
  libc_doserrno = libc_errno_kos2nt(libc_doserrno_last);
 }
 *perr = libc_doserrno;
 return EOK;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_dos___set_doserrno(u32 err) {
 if (libc_errno_isdos) {
  libc_errnoval = libc_errno_dos2kos(libc_errnoval);
  libc_errno_isdos = false;
 }
 libc_doserrno = err;
 libc_doserrno_last = libc_errnoval;
 return EOK;
}

DEFINE_PUBLIC_ALIAS(__doserrno,libc_dos___doserrno);
DEFINE_PUBLIC_ALIAS(_get_doserrno,libc_dos___get_doserrno);
DEFINE_PUBLIC_ALIAS(_set_doserrno,libc_dos___set_doserrno);

PRIVATE ATTR_DOSRODATA char const *const empty_errlist[] = {NULL};
PRIVATE ATTR_DOSRODATA int const empty_errlist_sz = 0;
INTERN ATTR_DOSTEXT char **LIBCCALL libc_sys_errlist(void) { return (char **)empty_errlist; }
INTERN ATTR_DOSTEXT int *LIBCCALL libc_sys_nerr(void) { return (int *)&empty_errlist_sz; }
DEFINE_PUBLIC_ALIAS(__sys_errlist,libc_sys_errlist);
DEFINE_PUBLIC_ALIAS(__sys_nerr,libc_sys_nerr);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_ERRNO_C */
