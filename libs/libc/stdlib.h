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
#ifndef GUARD_LIBS_LIBC_STDLIB_H
#define GUARD_LIBS_LIBC_STDLIB_H 1

#include "libc.h"
#include <stdlib.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/list/list.h>
#include <xlocale.h>

DECL_BEGIN

#ifndef __COMPAR_FN_T
#define __COMPAR_FN_T
typedef int (__LIBCCALL *__compar_fn_t)(void const *__a, void const *__b);
typedef __compar_fn_t comparison_fn_t;
#endif /* __COMPAR_FN_T */
#ifndef __compar_d_fn_t_defined
#define __compar_d_fn_t_defined 1
typedef int (__LIBCCALL *__compar_d_fn_t)(void const *__a, void const *__b, void *__arg);
#endif /* !__compar_d_fn_t_defined */

#ifndef __KERNEL__
typedef void (LIBCCALL *exitfunc)(int status, void *arg);
struct exitcall {
 SLIST_NODE(struct exitcall) ec_next; /*< [0..1] Next callback to-be executed. */
 exitfunc                    ec_func; /*< [1..1] Function to call on exit. */
 void                       *ec_arg;  /*< [?..?] Argument passed to 'ec_func'. */
};
INTDEF ATTR_NORETURN void LIBCCALL libc__exit(int status);
INTDEF ATTR_NORETURN void LIBCCALL libc_abort(void);
INTDEF ATTR_NORETURN void LIBCCALL libc_exit(int status);
INTDEF ATTR_NORETURN void LIBCCALL libc_quick_exit(int status);
INTDEF int LIBCCALL libc_on_exit(void (LIBCCALL *func)(int status, void *arg), void *arg);
INTDEF int LIBCCALL libc_atexit(void (LIBCCALL *func)(void));
INTDEF int LIBCCALL libc_at_quick_exit(void (LIBCCALL *func)(void));
#endif

#ifdef __KERNEL__
#define RAND_TYPE   __UINT32_TYPE__
#define RAND_SEED   __UINT32_TYPE__
#define RAND_SEED_R __UINT32_TYPE__
#else
#define RAND_TYPE   int
#define RAND_SEED   long
#define RAND_SEED_R unsigned int
#endif

INTDEF void LIBCCALL libc_srand(RAND_SEED seed);
#ifndef __KERNEL__
#if __SIZEOF_LONG__ != __SIZEOF_INT__
INTDEF void LIBCCALL libc_srandom(unsigned int seed);
#endif
#endif

#ifdef __KERNEL__
#define LOCAL_RAND_TYPE RAND_TYPE
INTDEF LOCAL_RAND_TYPE LIBCCALL libc_rand(void);
#else /* __KERNEL__ */
#if __SIZEOF_LONG__ == __SIZEOF_INT__
#define LOCAL_RAND_TYPE RAND_TYPE
INTDEF LOCAL_RAND_TYPE LIBCCALL libc_rand(void);
#else
#define LOCAL_RAND_TYPE long
INTDEF RAND_TYPE LIBCCALL libc_rand(void);
INTDEF long LIBCCALL libc_random(void);
#endif
#endif /* !__KERNEL__ */
#undef LOCAL_RAND_TYPE

INTDEF RAND_TYPE LIBCCALL libc_rand_r(RAND_SEED_R *__restrict pseed);

#define INTERFACE_ONLY
#if __SIZEOF_LONG__ == 4
#define STRTOINT_32  libc_strtol
#define STRTOUINT_32 libc_strtoul
#define STRTOINT     libc_strtol
#define STRTOUINT    libc_strtoul
#define TYPE         long
#define UTYPE        unsigned long
#define DECL         INTDEF
#include "templates/strtouint.code"
#elif __SIZEOF_LONG_LONG__ == 4
#define STRTOINT_32  libc_strtoll
#define STRTOUINT_32 libc_strtoull
#define STRTOINT     libc_strtoll
#define STRTOUINT    libc_strtoull
#define TYPE         __LONGLONG
#define UTYPE        __ULONGLONG
#define DECL         INTDEF
#include "templates/strtouint.code"
#endif

#if __SIZEOF_LONG__ == 8
#define STRTOINT_64  libc_strtol
#define STRTOUINT_64 libc_strtoul
#define STRTOINT     libc_strtol
#define STRTOUINT    libc_strtoul
#define TYPE         long
#define UTYPE        unsigned long
#define DECL         INTDEF
#include "templates/strtouint.code"
#elif __SIZEOF_LONG_LONG__ == 8
#define STRTOINT_64  libc_strtoll
#define STRTOUINT_64 libc_strtoull
#define STRTOINT     libc_strtoll
#define STRTOUINT    libc_strtoull
#define TYPE         __LONGLONG
#define UTYPE        __ULONGLONG
#define DECL         INTDEF
#include "templates/strtouint.code"
#endif
#undef INTERFACE_ONLY

INTDEF int LIBCCALL libc_atoi(char const *__restrict nptr);
#if __SIZEOF_LONG__ != __SIZEOF_INT__
INTDEF long LIBCCALL libc_atol(char const *__restrict nptr);
#endif
#if __SIZEOF_LONG_LONG__ != __SIZEOF_INT__ && \
    __SIZEOF_LONG_LONG__ != __SIZEOF_LONG__
INTDEF __LONGLONG LIBCCALL libc_atoll(char const *__restrict nptr);
#endif

INTDEF void *LIBCCALL libc_bsearch(void const *key, void const *base, size_t nmemb, size_t size, __compar_fn_t compar);
INTDEF void LIBCCALL libc_qsort(void *base, size_t nmemb, size_t size, __compar_fn_t compar);
INTDEF char *LIBCCALL libc_gcvt(double value, int ndigit, char *buf);
INTDEF char *LIBCCALL libc_qgcvt(long double value, int ndigit, char *buf);
INTDEF int LIBCCALL libc_ecvt_r(double value, int ndigit, int *__restrict decpt, int *__restrict sign, char *__restrict buf, size_t len);
INTDEF int LIBCCALL libc_fcvt_r(double value, int ndigit, int *__restrict decpt, int *__restrict sign, char *__restrict buf, size_t len);
INTDEF int LIBCCALL libc_qecvt_r(long double value, int ndigit, int *__restrict decpt, int *__restrict sign, char *__restrict buf, size_t len);
INTDEF int LIBCCALL libc_qfcvt_r(long double value, int ndigit, int *__restrict decpt, int *__restrict sign, char *__restrict buf, size_t len);
INTDEF double LIBCCALL libc_atof(char const *__restrict nptr);
INTDEF double LIBCCALL libc_strtod(char const *__restrict nptr, char **endptr);
INTDEF float LIBCCALL libc_strtof(char const *__restrict nptr, char **__restrict endptr);
INTDEF long double LIBCCALL libc_strtold(char const *__restrict nptr, char **__restrict endptr);
INTDEF int LIBCCALL libc_getloadavg(double loadavg[], int nelem);
INTDEF void LIBCCALL libc_qsort_r(void *pbase, size_t total_elems, size_t size, __compar_d_fn_t cmp, void *arg);
#ifndef __KERNEL__
INTDEF int LIBCCALL libc_getsubopt(char **__restrict optionp, char *const *__restrict tokens, char **__restrict valuep);
INTDEF int LIBCCALL libc_abs(int x);
INTDEF long LIBCCALL libc_labs(long x);
INTDEF __LONGLONG LIBCCALL libc_llabs(__LONGLONG x);
INTDEF div_t LIBCCALL libc_div(int numer, int denom);
INTDEF ldiv_t LIBCCALL libc_ldiv(long numer, long denom);
INTDEF lldiv_t LIBCCALL libc_lldiv(long long numer, long long denom);
INTDEF int LIBCCALL libc_system(char const *command);
INTDEF size_t LIBCCALL libc___ctype_get_mb_cur_max(void);
INTDEF int LIBCCALL libc_mblen(char const *s, size_t n);
INTDEF int LIBCCALL libc_mbtowc(wchar_t *__restrict pwc, char const *__restrict s, size_t n);
INTDEF int LIBCCALL libc_wctomb(char *s, wchar_t wchar);
INTDEF size_t LIBCCALL libc_mbstowcs(wchar_t *__restrict pwcs, char const *__restrict s, size_t n);
INTDEF size_t LIBCCALL libc_wcstombs(char *__restrict s, const wchar_t *__restrict pwcs, size_t n);
INTDEF double LIBCCALL libc_drand48(void);
INTDEF long LIBCCALL libc_lrand48(void);
INTDEF long LIBCCALL libc_mrand48(void);
INTDEF double LIBCCALL libc_erand48(unsigned short xsubi[3]);
INTDEF long LIBCCALL libc_nrand48(unsigned short xsubi[3]);
INTDEF long LIBCCALL libc_jrand48(unsigned short xsubi[3]);
INTDEF void LIBCCALL libc_srand48(long seedval);
INTDEF unsigned short *LIBCCALL libc_seed48(unsigned short seed16v[3]);
INTDEF void LIBCCALL libc_lcong48(unsigned short param[7]);
INTDEF char *LIBCCALL libc_initstate(unsigned int seed, char *statebuf, size_t statelen);
INTDEF char *LIBCCALL libc_setstate(char *statebuf);
INTDEF char *LIBCCALL libc_l64a(long n);
INTDEF long LIBCCALL libc_a64l(char const *s);
INTDEF char *LIBCCALL libc_realpath(char const *__restrict name, char *__restrict resolved);
INTDEF int LIBCCALL libc_drand48_r(struct drand48_data *__restrict buffer, double *__restrict result);
INTDEF int LIBCCALL libc_erand48_r(unsigned short xsubi[3], struct drand48_data *__restrict buffer, double *__restrict result);
INTDEF int LIBCCALL libc_lrand48_r(struct drand48_data *__restrict buffer, long *__restrict result);
INTDEF int LIBCCALL libc_nrand48_r(unsigned short xsubi[3], struct drand48_data *__restrict buffer, long *__restrict result);
INTDEF int LIBCCALL libc_mrand48_r(struct drand48_data *__restrict buffer, long *__restrict result);
INTDEF int LIBCCALL libc_jrand48_r(unsigned short xsubi[3], struct drand48_data *__restrict buffer, long *__restrict result);
INTDEF int LIBCCALL libc_srand48_r(long seedval, struct drand48_data *buffer);
INTDEF int LIBCCALL libc_seed48_r(unsigned short seed16v[3], struct drand48_data *buffer);
INTDEF int LIBCCALL libc_lcong48_r(unsigned short param[7], struct drand48_data *buffer);
INTDEF int LIBCCALL libc_random_r(struct random_data *__restrict buf, s32 *__restrict result);
INTDEF int LIBCCALL libc_srandom_r(unsigned int seed, struct random_data *buf);
INTDEF int LIBCCALL libc_initstate_r(unsigned int seed, char *__restrict statebuf, size_t statelen, struct random_data *__restrict buf);
INTDEF int LIBCCALL libc_setstate_r(char *__restrict statebuf, struct random_data *__restrict buf);
INTDEF int LIBCCALL libc_mkstemps(char *template_, int suffixlen);
INTDEF int LIBCCALL libc_rpmatch(char const *response);
INTDEF char *LIBCCALL libc_qecvt(long double value, int ndigit, int *__restrict decpt, int *__restrict sign);
INTDEF char *LIBCCALL libc_qfcvt(long double value, int ndigit, int *__restrict decpt, int *__restrict sign);
INTDEF char *LIBCCALL libc_ecvt(double value, int ndigit, int *__restrict decpt, int *__restrict sign);
INTDEF char *LIBCCALL libc_fcvt(double value, int ndigit, int *__restrict decpt, int *__restrict sign);
INTDEF long LIBCCALL libc_strtol_l(char const *__restrict nptr, char **__restrict endptr, int base, locale_t loc);
INTDEF unsigned long LIBCCALL libc_strtoul_l(char const *__restrict nptr, char **__restrict endptr, int base, locale_t loc);
INTDEF __LONGLONG LIBCCALL libc_strtoll_l(char const *__restrict nptr, char **__restrict endptr, int base, locale_t loc);
INTDEF __ULONGLONG LIBCCALL libc_strtoull_l(char const *__restrict nptr, char **__restrict endptr, int base, locale_t loc);
INTDEF double LIBCCALL libc_strtod_l(char const *__restrict nptr, char **__restrict endptr, locale_t loc);
INTDEF float LIBCCALL libc_strtof_l(char const *__restrict nptr, char **__restrict endptr, locale_t loc);
INTDEF long double LIBCCALL libc_strtold_l(char const *__restrict nptr, char **__restrict endptr, locale_t loc);
INTDEF char *LIBCCALL libc_canonicalize_file_name(char const *name);
INTDEF int LIBCCALL libc_ptsname_r(int fd, char *buf, size_t buflen);
INTDEF int LIBCCALL libc_getpt(void);
INTDEF int LIBCCALL libc_mkostemp(char *template_, int flags);
INTDEF int LIBCCALL libc_mkostemps(char *template_, int suffixlen, int flags);
INTDEF char *LIBCCALL libc_mktemp(char *template_);
INTDEF int LIBCCALL libc_mkstemp(char *template_);
INTDEF char *LIBCCALL libc_mkdtemp(char *template_);
INTDEF void LIBCCALL libc_setkey(char const *key);
INTDEF int LIBCCALL libc_grantpt(int fd);
INTDEF int LIBCCALL libc_unlockpt(int fd);
INTDEF char *LIBCCALL libc_ptsname(int fd);
INTDEF int LIBCCALL libc_posix_openpt(int oflag);
#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_STDLIB_H */
