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
#ifndef GUARD_LIBS_LIBC_SYSTEM_C
#define GUARD_LIBS_LIBC_SYSTEM_C 1
#define _KOS_SOURCE         1
#define _LARGEFILE64_SOURCE 1

#include "libc.h"
#include "system.h"
#include "file.h"
#include "misc.h"
#include "format-printer.h"
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "signal.h"

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <hybrid/asm.h>
#include <sys/syslog.h>
#include <sys/mman.h>

DECL_BEGIN

PRIVATE int syslog_options = 0;
PRIVATE int syslog_facility = 0;
PRIVATE int syslog_mask = -1;
INTDEF void LIBCCALL libc_closelog(void) {}
INTDEF void LIBCCALL libc_openlog(char const *UNUSED(ident), int option, int facility) {
 syslog_options  = option;
 syslog_facility = facility;
}
INTDEF int LIBCCALL libc_setlogmask(int mask) { return ATOMIC_XCH(syslog_mask,mask); }
INTDEF ssize_t LIBCCALL
libc_syslog_printer(char const *__restrict data,
                    size_t datalen, void *closure) {
 /* Check if the specified priority should be ignored. */
 if (!(syslog_mask&LOG_MASK(LOG_FAC((int)(uintptr_t)closure))))
     return 0;
 /* Also log to stderr if requested to. */
 if (syslog_options&LOG_PERROR)
     libc_fwrite(data,sizeof(char),datalen,stderr);
 return sys_xsyslog((int)(uintptr_t)closure,data,datalen);
}
INTDEF void LIBCCALL libc_vsyslog(int level, char const *format, va_list args) {
#if 1
 /* NOTE: Print the given message unbuffered to prevent syslog calls from
  *       other functions that may either cause a deadlock, or an infinite
  *       loop being called recursively. */
 libc_format_vprintf(&libc_syslog_printer,
                      SYSLOG_PRINTER_CLOSURE(level),
                      format,args);
#else
 libc_format_vbprintf(&libc_syslog_printer,
                       SYSLOG_PRINTER_CLOSURE(level),
                       format,args);
#endif
}
INTDEF void ATTR_CDECL
libc_syslog(int level, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_vsyslog(level,format,args);
 va_end(args);
}


INTDEF int LIBCCALL
libc_munmap(void *addr, size_t len) {
 ssize_t result = sys_munmap(addr,len);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return -1; }
 return 0;
}
INTDEF void *LIBCCALL
libc_xmmap1(struct mmap_info const *data) {
 void *result;
 result = sys_xmmap(MMAP_INFO_CURRENT,data);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return MAP_FAILED; }
 return result;
}
INTDEF ssize_t LIBCCALL
libc_xmunmap(void *addr, size_t len, int flags, void *tag) {
 ssize_t result = sys_xmunmap(addr,len,flags,tag);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return -1; }
 return result;
}

INTDEF void *LIBCCALL libc_xsharesym(char const *name) {
 void *result = sys_xsharesym(name);
 if (!result) SET_ERRNO(EINVAL);
 else if (E_ISERR(result)) {
  SET_ERRNO(-E_GTERR(result));
  result = NULL;
 }
 return result;
}
INTDEF void *LIBCCALL libc_mmap(void *addr, size_t len, int prot,
                                int flags, int fd, off_t offset) {
 void *result = sys_mmap(addr,len,prot,flags,fd,offset);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return MAP_FAILED; }
 return result;
}
INTDEF void *LIBCCALL libc_mmap64(void *addr, size_t len, int prot,
                                  int flags, int fd, off64_t offset) {
#if __SIZEOF_SYSCALL_LONG__ >= 8
 void *rresult = sys_mmap(addr,len,prot,flags,fd,offset);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return MAP_FAILED; }
 return result;
#else
 struct mmap_info info;
 info.mi_prot          = prot;
 info.mi_flags         = flags;
 info.mi_xflag         = XMAP_FINDAUTO;
 info.mi_addr          = addr;
 info.mi_size          = len;
 info.mi_align         = PAGESIZE;
 info.mi_gap           = PAGESIZE*16;
 info.mi_virt.mv_file  = fd;
 info.mi_virt.mv_off64 = offset; /* Use 64-bit file offsets. */
 info.mi_virt.mv_len   = len;
 info.mi_virt.mv_fill  = 0;
 info.mi_virt.mv_guard = PAGESIZE;
 info.mi_virt.mv_funds = MMAP_VIRT_MAXFUNDS;
 return libc_xmmap1(&info);
#endif
}

INTDEF void *ATTR_CDECL libc_mremap(void *addr, size_t old_len,
                                    size_t new_len, int flags, ...) {
 va_list args; void *result,*newaddr;
 va_start(args,flags);
 newaddr = va_arg(args,void *);
 result  = sys_mremap(addr,old_len,new_len,flags,newaddr);
 va_end(args);
 if (E_ISERR(result)) {
  SET_ERRNO(-E_GTERR(result));
  result = MAP_FAILED;
 }
 return result;
}

INTDEF int LIBCCALL libc_mprotect(void *addr, size_t len, int prot) {
 return FORWARD_SYSTEM_ERROR(sys_mprotect(addr,len,prot));
}

INTDEF void *LIBCCALL libc_xdlopen(char const *filename, int flags) {
 void *result = sys_xdlopen(filename,flags);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return NULL; }
 return result;
}
INTDEF void *LIBCCALL libc_xfdlopen(int fd, int flags) {
 void *result = sys_xfdlopen(fd,flags);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return NULL; }
 return result;
}
INTDEF void *LIBCCALL libc_xdlsym(void *handle, char const *symbol) {
 void *result = sys_xdlsym(handle,symbol);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return NULL; }
 return result;
}
INTDEF int LIBCCALL libc_xdlclose(void *handle) {
 return FORWARD_SYSTEM_ERROR(sys_xdlclose(handle));
}

INTERN ATTR_NORETURN
void LIBCCALL libc_internal_failure(void) {
 libc_fprintf(stderr,
              "An internal libc error caused the application to be terminated.\n"
              "For further details, consult the system log or coredump.");
 libc_fflush(stderr);

 /* Reset the signal handler for SIGQUIT (which defaults to generating a coredump). */
 libc_sysv_signal(SIGQUIT,SIG_DFL);
 /* Raise a SIGQUIT error. */
 libc_raise(SIGQUIT);
 /* We shouldn't get here, but if we do then simply abort. */
 libc_abort();
}


DEFINE_PUBLIC_ALIAS(closelog,libc_closelog);
DEFINE_PUBLIC_ALIAS(openlog,libc_openlog);
DEFINE_PUBLIC_ALIAS(setlogmask,libc_setlogmask);
DEFINE_PUBLIC_ALIAS(syslog_printer,libc_syslog_printer);
DEFINE_PUBLIC_ALIAS(vsyslog,libc_vsyslog);
DEFINE_PUBLIC_ALIAS(syslog,libc_syslog);
DEFINE_PUBLIC_ALIAS(munmap,libc_munmap);
DEFINE_PUBLIC_ALIAS(xmmap1,libc_xmmap1);
DEFINE_PUBLIC_ALIAS(xmunmap,libc_xmunmap);
DEFINE_PUBLIC_ALIAS(xsharesym,libc_xsharesym);
DEFINE_PUBLIC_ALIAS(mmap,libc_mmap);
DEFINE_PUBLIC_ALIAS(mmap64,libc_mmap64);
DEFINE_PUBLIC_ALIAS(mremap,libc_mremap);
DEFINE_PUBLIC_ALIAS(mprotect,libc_mprotect);
DEFINE_PUBLIC_ALIAS(xdlopen,libc_xdlopen);
DEFINE_PUBLIC_ALIAS(xfdlopen,libc_xfdlopen);
DEFINE_PUBLIC_ALIAS(xdlsym,libc_xdlsym);
DEFINE_PUBLIC_ALIAS(xdlclose,libc_xdlclose);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_getdllprocaddr,libc_xdlsym);
#else
INTERN void *LIBCCALL
libc_getdllprocaddr(intptr_t hnd, char *symname, intptr_t UNUSED(ord)) {
 return libc_xdlsym((void *)hnd,symname);
}
#endif
DEFINE_PUBLIC_ALIAS(_loaddll,libc_xdlopen);
DEFINE_PUBLIC_ALIAS(_unloaddll,libc_xdlclose);
DEFINE_PUBLIC_ALIAS(_getdllprocaddr,libc_getdllprocaddr);


/* These functions are used to lock/unlock some internal guards within DOS. */
INTERN ATTR_DOSTEXT void LIBCCALL libc_lock(int lockno) { NOT_IMPLEMENTED(); }
INTERN ATTR_DOSTEXT void LIBCCALL libc_unlock(int lockno) { NOT_IMPLEMENTED(); }
INTERN ATTR_DOSTEXT void LIBCCALL
libc_invalid_parameter(char16_t const *expr, char16_t const *func,
                       char16_t const *file, unsigned int lineno,
                       uintptr_t UNUSED(unused)) {
 libc_syslog(LOG_WARN,"%U16s(%d) : %U16s : Invalid parameter %U16s\n",
             file,lineno,func,expr);
}
INTERN ATTR_DOSTEXT ATTR_NORETURN void LIBCCALL 
libc_invoke_watson(char16_t const *expr, char16_t const *func,
                   char16_t const *file, unsigned int lineno,
                   uintptr_t UNUSED(unused)) {
 libc_syslog(LOG_WARN,"%U16s(%d) : %U16s : INVOKE_WATSON %U16s\n",
             file,lineno,func,expr);
 libc_internal_failure();
}
INTERN ATTR_DOSTEXT void LIBCCALL
libc_invalid_parameter_noinfo(void) {
 libc_invalid_parameter(NULL,NULL,NULL,0,0);
}
INTERN ATTR_DOSTEXT void LIBCCALL
libc_invalid_parameter_noinfo_noreturn(void) {
 libc_invalid_parameter(NULL,NULL,NULL,0,0);
 libc_invoke_watson(NULL,NULL,NULL,0,0);
}

DEFINE_PUBLIC_ALIAS(_lock,libc_lock);
DEFINE_PUBLIC_ALIAS(_unlock,libc_unlock);
DEFINE_PUBLIC_ALIAS(_invoke_watson,libc_invoke_watson);
DEFINE_PUBLIC_ALIAS(_invalid_parameter,libc_invalid_parameter);
DEFINE_PUBLIC_ALIAS(_invalid_parameter_noinfo,libc_invalid_parameter_noinfo);
DEFINE_PUBLIC_ALIAS(_invalid_parameter_noinfo_noreturn,libc_invalid_parameter_noinfo_noreturn);

#if 0
/* TODO: Faster+smaller assembly version that uses self-modifying code. */
#else
PRIVATE ATTR_DOSRODATA char const libm_name[] = "libm.so";
PRIVATE ATTR_DOSBSS void *libm_handle = NULL;
PRIVATE ATTR_DOSTEXT void *LIBCCALL libc_getlibm_symbol(char const *name) {
 void *result;
 if (!libm_handle) {
  void *new_handle = libc_xdlopen(libm_name,0);
  if (!new_handle) {
   libc_syslog(LOG_ERROR,"[LIBC] Failed to open %q: %[errno]\n",
               libm_name,GET_ERRNO());
   libc_internal_failure();
  }
  if (ATOMIC_CMPXCH(libm_handle,NULL,new_handle))
      libc_xdlclose(new_handle);
 }
 result = libc_xdlsym(libm_handle,name);
 if (!result) {
  libc_syslog(LOG_ERROR,"[LIBC] Failed to import libm symbol %q: %[errno]\n",
              libm_name,GET_ERRNO());
  libc_internal_failure();
 }
 return result;
}
#define DEFINE_INTERN_LIBM_ALIAS(libc_name,libm_name,Treturn,param,args) \
PRIVATE ATTR_DOSRODATA char const str_libm_##libm_name[] = #libm_name; \
PRIVATE ATTR_DOSBSS Treturn (ATTR_CDECL *libm_##libm_name) param; \
INTERN ATTR_DOSTEXT Treturn (LIBCCALL libc_name) param { \
    if (!libm_##libm_name) \
      *(void **)&libm_##libm_name = libc_getlibm_symbol(str_libm_##libm_name); \
    return (*libm_##libm_name) args;\
}
#endif


#define REDIRECT_LIBM(public_name,libc_name,libm_name,Treturn,param,args) \
 DEFINE_INTERN_LIBM_ALIAS(libc_name,libm_name,Treturn,param,args); \
 DEFINE_PUBLIC_ALIAS(public_name,libc_name)

#define __PE_LDOUBLE_IS_DOUBLE

/* DOS exports math functions from its libc, but since
 * we follow unix guidelines, we export them from libm.
 * For binary compatibility though, we must alias  */
REDIRECT_LIBM(acos,libc_acos,acos,double,(double x),(x));
REDIRECT_LIBM(acosf,libc_acosf,acosf,float,(float x),(x));
REDIRECT_LIBM(asin,libc_asin,asin,double,(double x),(x));
REDIRECT_LIBM(asinf,libc_asinf,asinf,float,(float x),(x));
REDIRECT_LIBM(atan,libc_atan,atan,double,(double x),(x));
REDIRECT_LIBM(atanf,libc_atanf,atanf,float,(float x),(x));
REDIRECT_LIBM(atan2,libc_atan2,atan2,double,(double y, double x),(y,x));
REDIRECT_LIBM(atan2f,libc_atan2f,atan2f,double,(double y, double x),(y,x));
REDIRECT_LIBM(ceil,libc_ceil,ceil,double,(double x),(x));
REDIRECT_LIBM(ceilf,libc_ceilf,ceilf,float,(float x),(x));
REDIRECT_LIBM(cos,libc_cos,cos,double,(double x),(x));
REDIRECT_LIBM(cosf,libc_cosf,cosf,float,(float x),(x));
REDIRECT_LIBM(cosh,libc_cosh,cosh,double,(double x),(x));
REDIRECT_LIBM(coshf,libc_coshf,coshf,float,(float x),(x));
REDIRECT_LIBM(exp,libc_exp,exp,double,(double x),(x));
REDIRECT_LIBM(expf,libc_expf,expf,float,(float x),(x));
REDIRECT_LIBM(fabs,libc_fabs,fabs,double,(double x),(x));
REDIRECT_LIBM(floor,libc_floor,floor,double,(double x),(x));
REDIRECT_LIBM(floorf,libc_floorf,floorf,float,(float x),(x));
REDIRECT_LIBM(fmod,libc_fmod,fmod,double,(double x, double y),(x,y));
REDIRECT_LIBM(fmodf,libc_fmodf,fmodf,float,(float x, float y),(x,y));
REDIRECT_LIBM(ldexp,libc_ldexp,ldexp,double,(double x, int exponent),(x,exponent));
REDIRECT_LIBM(log,libc_log,log,double,(double x),(x));
REDIRECT_LIBM(logf,libc_logf,logf,float,(float x),(x));
REDIRECT_LIBM(log10,libc_log10,log10,double,(double x),(x));
REDIRECT_LIBM(log10f,libc_log10f,log10f,float,(float x),(x));
REDIRECT_LIBM(modf,libc_modf,modf,double,(double x, double *iptr),(x,iptr));
REDIRECT_LIBM(modff,libc_modff,modff,float,(float x, float *iptr),(x,iptr));
REDIRECT_LIBM(pow,libc_pow,pow,double,(double x, double y),(x,y));
REDIRECT_LIBM(powf,libc_powf,powf,float,(float x, float y),(x,y));
REDIRECT_LIBM(sin,libc_sin,sin,double,(double x),(x));
REDIRECT_LIBM(sinf,libc_sinf,sinf,float,(float x),(x));
REDIRECT_LIBM(sinh,libc_sinh,sinh,double,(double x),(x));
REDIRECT_LIBM(sinhf,libc_sinhf,sinhf,float,(float x),(x));
REDIRECT_LIBM(sqrt,libc_sqrt,sqrt,double,(double x),(x));
REDIRECT_LIBM(sqrtf,libc_sqrtf,sqrtf,float,(float x),(x));
REDIRECT_LIBM(tan,libc_tan,tan,double,(double x),(x));
REDIRECT_LIBM(tanf,libc_tanf,tanf,float,(float x),(x));
REDIRECT_LIBM(tanh,libc_tanh,tanh,double,(double x),(x));
REDIRECT_LIBM(tanhf,libc_tanhf,tanhf,float,(float x),(x));
REDIRECT_LIBM(trunc,libc_trunc,trunc,double,(double x),(x));
REDIRECT_LIBM(truncf,libc_truncf,truncf,float,(float x),(x));
REDIRECT_LIBM(tgamma,libc_tgamma,tgamma,double,(double x),(x));
REDIRECT_LIBM(tgammaf,libc_tgammaf,tgammaf,float,(float x),(x));
REDIRECT_LIBM(lgamma,libc_lgamma,lgamma,double,(double x),(x));
REDIRECT_LIBM(lgammaf,libc_lgammaf,lgammaf,float,(float x),(x));
REDIRECT_LIBM(scalbn,libc_scalbn,scalbn,double,(double x, int n),(x,n));
REDIRECT_LIBM(scalbnf,libc_scalbnf,scalbnf,float,(float x, int n),(x,n));
REDIRECT_LIBM(scalbln,libc_scalbln,scalbln,double,(double x, long int n),(x,n));
REDIRECT_LIBM(scalblnf,libc_scalblnf,scalblnf,float,(float x, long int n),(x,n));
REDIRECT_LIBM(round,libc_round,round,double,(double x),(x));
REDIRECT_LIBM(roundf,libc_roundf,roundf,float,(float x),(x));
REDIRECT_LIBM(lround,libc_lround,lround,long,(double x),(x));
REDIRECT_LIBM(lroundf,libc_lroundf,lroundf,long,(float x),(x));
REDIRECT_LIBM(llround,libc_llround,llround,long long,(double x),(x));
REDIRECT_LIBM(llroundf,libc_llroundf,llroundf,long long,(float x),(x));
REDIRECT_LIBM(rint,libc_rint,rint,double,(double x),(x));
REDIRECT_LIBM(rintf,libc_rintf,rintf,float,(float x),(x));
REDIRECT_LIBM(lrint,libc_lrint,lrint,long,(double x),(x));
REDIRECT_LIBM(lrintf,libc_lrintf,lrintf,long,(float x),(x));
REDIRECT_LIBM(llrint,libc_llrint,llrint,long long,(double x),(x));
REDIRECT_LIBM(llrintf,libc_llrintf,llrintf,long long,(float x),(x));
REDIRECT_LIBM(remquo,libc_remquo,remquo,double,(double x, double y, int *quo),(x,y,quo));
REDIRECT_LIBM(remquof,libc_remquof,remquof,float,(float x, float y, int *quo),(x,y,quo));
REDIRECT_LIBM(remainder,libc_remainder,remainder,double,(double x, double y),(x,y));
REDIRECT_LIBM(remainderf,libc_remainderf,remainderf,float,(float x, float y),(x,y));
REDIRECT_LIBM(nan,libc_nan,nan,double,(char const *tag),(tag));
REDIRECT_LIBM(nanf,libc_nanf,nanf,float,(char const *tag),(tag));
REDIRECT_LIBM(logb,libc_logb,logb,double,(double x),(x));
REDIRECT_LIBM(logbf,libc_logbf,logbf,float,(float x),(x));
REDIRECT_LIBM(ilogb,libc_ilogb,ilogb,int,(double x),(x));
REDIRECT_LIBM(ilogbf,libc_ilogbf,ilogbf,int,(float x),(x));
REDIRECT_LIBM(log2,libc_log2,log2,double,(double x),(x));
REDIRECT_LIBM(log2f,libc_log2f,log2f,float,(float x),(x));
REDIRECT_LIBM(log1p,libc_log1p,log1p,double,(double x),(x));
REDIRECT_LIBM(log1pf,libc_log1pf,log1pf,float,(float x),(x));
REDIRECT_LIBM(fmin,libc_fmin,fmin,double,(double x, double y),(x,y));
REDIRECT_LIBM(fminf,libc_fminf,fminf,float,(float x, float y),(x,y));
REDIRECT_LIBM(fmax,libc_fmax,fmax,double,(double x, double y),(x,y));
REDIRECT_LIBM(fmaxf,libc_fmaxf,fmaxf,float,(float x, float y),(x,y));
REDIRECT_LIBM(fdim,libc_fdim,fdim,double,(double x, double y),(x,y));
REDIRECT_LIBM(fdimf,libc_fdimf,fdimf,float,(float x, float y),(x,y));
REDIRECT_LIBM(frexp,libc_frexp,frexp,double,(double x, int *exp),(x,exp));
REDIRECT_LIBM(fma,libc_fma,fma,double,(double x, double y, double z),(x,y,z));
REDIRECT_LIBM(fmaf,libc_fmaf,fmaf,float,(float x, float y, float z),(x,y,z));
REDIRECT_LIBM(expm1,libc_expm1,expm1,double,(double x),(x));
REDIRECT_LIBM(expm1f,libc_expm1f,expm1f,float,(float x),(x));
REDIRECT_LIBM(exp2,libc_exp2,exp2,double,(double x),(x));
REDIRECT_LIBM(exp2f,libc_exp2f,exp2f,float,(float x),(x));
REDIRECT_LIBM(erf,libc_erf,erf,double,(double x),(x));
REDIRECT_LIBM(erff,libc_erff,erff,float,(float x),(x));
REDIRECT_LIBM(erfc,libc_erfc,erfc,double,(double x),(x));
REDIRECT_LIBM(erfcf,libc_erfcf,erfcf,float,(float x),(x));
REDIRECT_LIBM(cbrt,libc_cbrt,cbrt,double,(double x),(x));
REDIRECT_LIBM(cbrtf,libc_cbrtf,cbrtf,float,(float x),(x));
REDIRECT_LIBM(copysign,libc_copysign,copysign,double,(double x, double y),(x,y));
REDIRECT_LIBM(copysignf,libc_copysignf,copysignf,float,(float x, float y),(x,y));
REDIRECT_LIBM(atanh,libc_atanh,atanh,double,(double x),(x));
REDIRECT_LIBM(atanhf,libc_atanhf,atanhf,float,(float x),(x));
REDIRECT_LIBM(asinh,libc_asinh,asinh,double,(double x),(x));
REDIRECT_LIBM(asinhf,libc_asinhf,asinhf,float,(float x),(x));
REDIRECT_LIBM(acosh,libc_acosh,acosh,double,(double x),(x));
REDIRECT_LIBM(acoshf,libc_acoshf,acoshf,float,(float x),(x));
#ifdef __PE_LDOUBLE_IS_DOUBLE
DEFINE_PUBLIC_ALIAS(truncl,libc_trunc);
DEFINE_PUBLIC_ALIAS(tgammal,libc_tgamma);
DEFINE_PUBLIC_ALIAS(lgammal,libc_lgamma);
DEFINE_PUBLIC_ALIAS(scalbnl,libc_scalbn);
DEFINE_PUBLIC_ALIAS(scalblnl,libc_scalbln);
DEFINE_PUBLIC_ALIAS(roundl,libc_round);
DEFINE_PUBLIC_ALIAS(rintl,libc_rint);
DEFINE_PUBLIC_ALIAS(lrintl,libc_lrint);
DEFINE_PUBLIC_ALIAS(llrintl,libc_llrint);
DEFINE_PUBLIC_ALIAS(remquol,libc_remquo);
DEFINE_PUBLIC_ALIAS(remainderl,libc_remainder);
DEFINE_PUBLIC_ALIAS(nanl,libc_nan);
DEFINE_PUBLIC_ALIAS(lroundl,libc_lround);
DEFINE_PUBLIC_ALIAS(llroundl,libc_llround);
DEFINE_PUBLIC_ALIAS(logbl,libc_logb);
DEFINE_PUBLIC_ALIAS(ilogbl,libc_ilogb);
DEFINE_PUBLIC_ALIAS(log2l,libc_log2);
DEFINE_PUBLIC_ALIAS(log1pl,libc_log1p);
DEFINE_PUBLIC_ALIAS(fminl,libc_fmin);
DEFINE_PUBLIC_ALIAS(fmaxl,libc_fmax);
DEFINE_PUBLIC_ALIAS(fdiml,libc_fdim);
DEFINE_PUBLIC_ALIAS(fmal,libc_fma);
DEFINE_PUBLIC_ALIAS(expm1l,libc_expm1);
DEFINE_PUBLIC_ALIAS(exp2l,libc_exp2);
DEFINE_PUBLIC_ALIAS(erfl,libc_erf);
DEFINE_PUBLIC_ALIAS(erfcl,libc_erfc);
DEFINE_PUBLIC_ALIAS(cbrtl,libc_cbrt);
DEFINE_PUBLIC_ALIAS(copysignl,libc_copysign);
DEFINE_PUBLIC_ALIAS(atanhl,libc_atanh);
DEFINE_PUBLIC_ALIAS(asinhl,libc_asinh);
DEFINE_PUBLIC_ALIAS(acoshl,libc_acosh);
#else
REDIRECT_LIBM(truncl,libc_truncl,truncl,long double,(long double x),(x));
REDIRECT_LIBM(tgammal,libc_tgammal,tgammal,long double,(long double x),(x));
REDIRECT_LIBM(lgammal,libc_lgammal,lgammal,long double,(long double x),(x));
REDIRECT_LIBM(scalbnl,libc_scalbnl,scalbnl,long double,(long double x, int n),(x,n));
REDIRECT_LIBM(scalblnl,libc_scalblnl,scalblnl,long double,(long double x, long int n),(x,n));
REDIRECT_LIBM(roundl,libc_roundl,roundl,long double,(long double x),(x));
REDIRECT_LIBM(rintl,libc_rintl,rintl,long double,(long double x),(x));
REDIRECT_LIBM(lrintl,libc_lrintl,lrintl,long,(long double x),(x));
REDIRECT_LIBM(llrintl,libc_llrintl,llrintl,long long,(long double x),(x));
REDIRECT_LIBM(remquol,libc_remquol,remquol,long double,(long double x, long double y, int *quo),(x,y,quo));
REDIRECT_LIBM(remainderl,libc_remainderl,remainderl,long double,(long double x, long double y),(x,y));
REDIRECT_LIBM(nanl,libc_nanl,nanl,long double,(char const *tag),(tag));
REDIRECT_LIBM(lroundl,libc_lroundl,lroundl,long,(long double x),(x));
REDIRECT_LIBM(llroundl,libc_llroundl,llroundl,long long,(long double x),(x));
REDIRECT_LIBM(logbl,libc_logbl,logbl,long double,(long double x),(x));
REDIRECT_LIBM(ilogbl,libc_ilogbl,ilogbl,int,(long double x),(x));
REDIRECT_LIBM(logb2,libc_logb2,logb2,long double,(long double x),(x));
REDIRECT_LIBM(logb1p,libc_logb1p,logb1p,long double,(long double x),(x));
REDIRECT_LIBM(fminl,libc_fminl,fminl,long double,(long double x, long double y),(x,y));
REDIRECT_LIBM(fmaxl,libc_fmaxl,fmaxl,long double,(long double x, long double y),(x,y));
REDIRECT_LIBM(fdiml,libc_fdiml,fdiml,long double,(long double x, long double y),(x,y));
REDIRECT_LIBM(fmal,libc_fmal,fmal,long double,(long double x, long double y, long double z),(x,y,z));
REDIRECT_LIBM(expm1l,libc_expm1l,expm1l,long double,(long double x),(x));
REDIRECT_LIBM(exp2l,libc_exp2l,exp2l,long double,(long double x),(x));
REDIRECT_LIBM(erfl,libc_erfl,erfl,long double,(long double x),(x));
REDIRECT_LIBM(erfcl,libc_erfcl,erfcl,long double,(long double x),(x));
REDIRECT_LIBM(cbrtl,libc_cbrtl,cbrtl,long double,(long double x),(x));
REDIRECT_LIBM(copysignl,libc_copysignl,copysignl,long double,(long double x, long double y),(x,y));
REDIRECT_LIBM(atanhl,libc_atanhl,atanhl,long double,(long double x),(x));
REDIRECT_LIBM(asinhl,libc_asinhl,asinhl,long double,(long double x),(x));
REDIRECT_LIBM(acoshl,libc_acoshl,acoshl,long double,(long double x),(x));
#endif
DEFINE_PUBLIC_ALIAS(_logb,libc_logb);
DEFINE_PUBLIC_ALIAS(_copysign,libc_copysign);
REDIRECT_LIBM(_finite,libc_finite,finite,int,(double x),(x));
REDIRECT_LIBM(_isnan,libc_isnan,isnan,int,(double x),(x));
REDIRECT_LIBM(_fpclass,libc_fpclassifyd,__fpclassifyd,int,(double x),(x));
REDIRECT_LIBM(_nextafter,libc_nextafter,nextafter,double,(double x, double y),(x,y));
REDIRECT_LIBM(_scalb,libc_scalb,scalb,double,(double x, double n),(x,n));
REDIRECT_LIBM(_hypot,libc_hypot,hypot,double,(double x, double y),(x,y));
REDIRECT_LIBM(_j0,libc_j0,j0,double,(double x),(x));
REDIRECT_LIBM(_j1,libc_j1,j1,double,(double x),(x));
REDIRECT_LIBM(_jn,libc_jn,jn,double,(int n, double x),(n,x));
REDIRECT_LIBM(_y0,libc_y0,y0,double,(double x),(x));
REDIRECT_LIBM(_y1,libc_y1,y1,double,(double x),(x));
REDIRECT_LIBM(_yn,libc_yn,yn,double,(int n, double x),(n,x));
INTERN ATTR_DOSTEXT double LIBCCALL libc_chgsign(double x) { return -x; }
DEFINE_PUBLIC_ALIAS(_chgsign,libc_chgsign);


INTERN ATTR_DOSTEXT u32 LIBCCALL libc_clearfp(void) { NOT_IMPLEMENTED(); return 0; }
INTERN ATTR_DOSTEXT u32 LIBCCALL libc_controlfp(u32 newval, u32 mask) { NOT_IMPLEMENTED(); return 0; }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_controlfp_s(u32 *pcurrent, u32 newval, u32 mask) { NOT_IMPLEMENTED(); return 0; }
DEFINE_INTERN_ALIAS(libc_set_controlfp,libc_controlfp);
INTERN ATTR_DOSTEXT u32 LIBCCALL libc_statusfp(void) { NOT_IMPLEMENTED(); return 0; }
INTERN ATTR_DOSTEXT void LIBCCALL libc_fpreset(void) { NOT_IMPLEMENTED(); }
INTERN ATTR_DOSTEXT u32 LIBCCALL libc_control87(u32 newval, u32 mask) { NOT_IMPLEMENTED(); return 0; }
PRIVATE ATTR_DOSBSS int fpecode = 0; /* ??? */
INTERN ATTR_DOSTEXT int *LIBCCALL libc_fpecode(void) { return &fpecode; }

DEFINE_PUBLIC_ALIAS(_clearfp,libc_clearfp);
DEFINE_PUBLIC_ALIAS(_controlfp,libc_controlfp);
DEFINE_PUBLIC_ALIAS(_set_controlfp,libc_set_controlfp);
DEFINE_PUBLIC_ALIAS(_controlfp_s,libc_controlfp_s);
DEFINE_PUBLIC_ALIAS(_statusfp,libc_statusfp);
DEFINE_PUBLIC_ALIAS(_fpreset,libc_fpreset);
DEFINE_PUBLIC_ALIAS(_control87,libc_control87);
DEFINE_PUBLIC_ALIAS(__fpecode,libc_fpecode);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_SYSTEM_C */
