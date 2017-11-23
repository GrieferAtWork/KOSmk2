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
#include <kos/thread.h>

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include <bits/dos-errno.h>
#include <winapi/winerror.h>
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_BEGIN

#define ERRNO_FMT  offsetof(struct tlb,tl_errno_fmt)
#define ERRNO_VAL  offsetof(struct tlb,tl_errno_val)
#define NTERRNO2   offsetof(struct tib,it_error_code)

#ifdef CONFIG_LIBC_NO_DOS_LIBC
#ifdef TLB_ADDR
INTERN errno_t *LIBCCALL libc_errno(void) { return (errno_t *)TLB_ADDR(ERRNO_VAL); }
INTERN errno_t LIBCCALL libc_get_errno(void) { return (errno_t)TLB_PEEKL(ERRNO_VAL); }
INTERN errno_t LIBCCALL libc_set_errno(errno_t err) { TLB_POKEL(ERRNO_VAL,(u32)err); return EOK; }
#else
PRIVATE errno_t current_errno;
INTERN errno_t *LIBCCALL libc_errno(void) { return &current_errno; }
INTERN errno_t LIBCCALL libc_get_errno(void) { return current_errno; }
INTERN errno_t LIBCCALL libc_set_errno(errno_t err) { current_errno = err; return EOK; }
#endif
#else

INTERN errno_t *LIBCCALL libc_errno(void) {
 u32 *result = (u32 *)TLB_ADDR(ERRNO_VAL);
 switch (result[-1]) {
 case TLB_ERRNO_DOS: *result = (u32)libc_errno_dos2kos((errno_t)*result);
                     goto setfmt;
 case TLB_ERRNO_NT:  TIB_POKEL(NTERRNO2,*result);
                     *result = (u32)libc_errno_nt2kos(*result);
                     goto setfmt;
 default: break;
 }
 return (errno_t *)result;
setfmt:
 result[-1] = TLB_ERRNO_KOS;
 return (errno_t *)result;
}
INTERN errno_t LIBCCALL libc_get_errno(void) {
 if (TLB_PEEKL(ERRNO_FMT) != TLB_ERRNO_KOS)
     return *libc_errno();
 return TLB_PEEKL(ERRNO_VAL);
}
INTERN errno_t LIBCCALL libc_set_errno(errno_t err) {
 TLB_POKEL(ERRNO_FMT,TLB_ERRNO_KOS);
 TLB_POKEL(ERRNO_VAL,(u32)err);
 return EOK;
}

INTERN ATTR_DOSTEXT errno_t *LIBCCALL libc_doserrno(void) {
 u32 *result = (u32 *)TLB_ADDR(ERRNO_VAL);
 switch (result[-1]) {
 case TLB_ERRNO_KOS: *result = (u32)libc_errno_kos2dos((errno_t)*result);
                     goto setfmt;
 case TLB_ERRNO_NT:  TIB_POKEL(NTERRNO2,*result);
                     *result = (u32)libc_errno_nt2dos(*result);
                     goto setfmt;
 default: break;
 }
 return (errno_t *)result;
setfmt:
 result[-1] = TLB_ERRNO_KOS;
 return (errno_t *)result;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_get_doserrno(void) {
 if (TLB_PEEKL(ERRNO_FMT) != TLB_ERRNO_DOS)
     return *libc_doserrno();
 return TLB_PEEKL(ERRNO_VAL);
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_set_doserrno(errno_t err) {
 TLB_POKEL(ERRNO_FMT,TLB_ERRNO_DOS);
 TLB_POKEL(ERRNO_VAL,err);
 return EOK;
}


INTERN ATTR_DOSTEXT u32 *LIBCCALL libc_nterrno(void) {
 u32 *result = (u32 *)TLB_ADDR(ERRNO_VAL);
 switch (result[-1]) {
 case TLB_ERRNO_DOS:
  *result = (u32)libc_errno_dos2kos(*result);
  /* fallthrough */
 case TLB_ERRNO_KOS:
  *result = (u32)libc_errno_kos2nt((errno_t)*result);
  TIB_POKEL(NTERRNO2,*result); /* Update the redundant copy. */
  result[-1] = TLB_ERRNO_KOS;
  break;
 default: break;
 }
 return result;
}
INTERN ATTR_DOSTEXT u32 LIBCCALL libc_get_nterrno(void) {
 if (TLB_PEEKL(ERRNO_FMT) != TLB_ERRNO_NT)
     return *libc_nterrno();
 return TLB_PEEKL(ERRNO_VAL);
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_set_nterrno(u32 err) {
 TLB_POKEL(ERRNO_FMT,TLB_ERRNO_NT);
 TLB_POKEL(ERRNO_VAL,err);
 TIB_POKEL(NTERRNO2,err);
 return EOK;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_get_doserrno2(errno_t *perr) {
 if (perr) *perr = libc_get_doserrno();
 return EOK;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_get_nterrno2(u32 *perr) {
 if (perr) *perr = libc_get_nterrno();
 return EOK;
}


DEFINE_PUBLIC_ALIAS(_errno,libc_doserrno);             /* DOS */
DEFINE_PUBLIC_ALIAS(__get_doserrno,libc_get_doserrno); /* DOS */
DEFINE_PUBLIC_ALIAS(_set_errno,libc_set_doserrno);     /* DOS */
DEFINE_PUBLIC_ALIAS(_get_errno,libc_get_doserrno2);    /* DOS */
DEFINE_PUBLIC_ALIAS(__doserrno,libc_nterrno);          /* NT */
DEFINE_PUBLIC_ALIAS(_set_doserrno,libc_set_nterrno);   /* NT */
DEFINE_PUBLIC_ALIAS(_get_doserrno,libc_get_nterrno2);  /* NT */
DEFINE_PUBLIC_ALIAS(__get_nterrno,libc_get_nterrno);   /* NT */
#endif
DEFINE_PUBLIC_ALIAS(__errno,libc_errno);               /* Cygwin */
DEFINE_PUBLIC_ALIAS(__errno_location,libc_errno);      /* GLibC */
DEFINE_PUBLIC_ALIAS(__get_errno,libc_get_errno);       /* KOS */
DEFINE_PUBLIC_ALIAS(__set_errno,libc_set_errno);       /* KOS */
/* NOTE: There is no KOS-version that stores the error in a pointer. */


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

/* Error code transformations. */
#define DOS_ERRNO_NOT_SUPPORTED __DOS_ENOTSUP
#define KOS_ERRNO_NOT_SUPPORTED EINVAL
#define NT_ERRNO_NOT_SUPPORTED  ERROR_FUNCTION_FAILED

PRIVATE ATTR_DOSRODATA u8 const vec_errno_dos2kos[__DOS_EMAX+1] = {
    [0 ... __DOS_EMAX] = KOS_ERRNO_NOT_SUPPORTED,
#define PAIR(kos,dos,nt) [dos] = kos,
#include "templates/errno-dospair.code"
};
PRIVATE ATTR_DOSRODATA u8 const vec_errno_kos2dos[__EBASEMAX+1] = {
    [0 ... __EBASEMAX] = DOS_ERRNO_NOT_SUPPORTED,
#define PAIR(kos,dos,nt) [kos] = dos,
#include "templates/errno-dospair.code"
};
PRIVATE ATTR_DOSRODATA u16 const vec_errno_kos2nt[__EBASEMAX+1] = {
    [0 ... __EBASEMAX] = NT_ERRNO_NOT_SUPPORTED,
#define PAIR(kos,dos,nt) [kos] = nt,
#include "templates/errno-dospair.code"
};
#pragma GCC diagnostic pop

INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_errno_dos2kos(errno_t eno) { return (unsigned int)eno < COMPILER_LENOF(vec_errno_dos2kos) ? vec_errno_dos2kos[eno] : KOS_ERRNO_NOT_SUPPORTED; }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_errno_kos2dos(errno_t eno) { return (unsigned int)eno < COMPILER_LENOF(vec_errno_kos2dos) ? vec_errno_kos2dos[eno] : DOS_ERRNO_NOT_SUPPORTED; }
INTERN ATTR_DOSTEXT u32 LIBCCALL libc_errno_kos2nt(errno_t eno) { return (unsigned int)eno < COMPILER_LENOF(vec_errno_kos2nt) ? vec_errno_kos2nt[eno] : NT_ERRNO_NOT_SUPPORTED; }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_errno_nt2kos(u32 eno) {
 u16 const *iter;
 errno_t result = KOS_ERRNO_NOT_SUPPORTED;
 /* Translate an NT error code to KOS. */
 for (iter = vec_errno_kos2nt;
      iter != COMPILER_ENDOF(vec_errno_kos2nt); ++iter) {
  if ((u32)*iter == eno) {
   result = (errno_t)(iter-vec_errno_kos2nt);
   break;
  }
 }
 switch (eno) {
#define PAIR(kos,dos,nt)
#define NT_ALIAS(nt,kos,dos) case nt: result = kos; break;
#include "templates/errno-dospair.code"
 default: break;
 }
 return result;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_errno_nt2dos(u32 eno) {
 u16 const *iter;
 errno_t result = KOS_ERRNO_NOT_SUPPORTED;
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

DEFINE_PUBLIC_ALIAS(errno_dos2kos,libc_errno_dos2kos);
DEFINE_PUBLIC_ALIAS(errno_kos2dos,libc_errno_kos2dos);
DEFINE_PUBLIC_ALIAS(errno_kos2nt,libc_errno_kos2nt);
DEFINE_PUBLIC_ALIAS(errno_nt2dos,libc_errno_nt2kos);
DEFINE_PUBLIC_ALIAS(_dosmaperr,libc_errno_nt2dos);


PRIVATE ATTR_DOSRODATA char const *const empty_errlist[] = {NULL};
PRIVATE ATTR_DOSRODATA int const empty_errlist_sz = 0;
INTERN ATTR_DOSTEXT char **LIBCCALL libc_sys_errlist(void) { return (char **)empty_errlist; }
INTERN ATTR_DOSTEXT int *LIBCCALL libc_sys_nerr(void) { return (int *)&empty_errlist_sz; }
DEFINE_PUBLIC_ALIAS(__sys_errlist,libc_sys_errlist);
DEFINE_PUBLIC_ALIAS(__sys_nerr,libc_sys_nerr);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_ERRNO_C */
