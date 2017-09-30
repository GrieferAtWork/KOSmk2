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
#include <errno.h>
#include <stdlib.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/list/list.h>
#include <uchar.h>
#include <inttypes.h>
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
INTDEF void *LIBCCALL libc_bsearch(void const *key, void const *base, size_t nmemb, size_t size, __compar_fn_t compar);
INTDEF void LIBCCALL libc_qsort(void *base, size_t nmemb, size_t size, __compar_fn_t compar);
INTDEF int LIBCCALL libc_getloadavg(double loadavg[], int nelem);
INTDEF void LIBCCALL libc_qsort_r(void *pbase, size_t total_elems, size_t size, __compar_d_fn_t cmp, void *arg);
#ifndef __KERNEL__
INTDEF int LIBCCALL libc_getsubopt(char **__restrict optionp, char *const *__restrict tokens, char **__restrict valuep);
INTDEF int LIBCCALL libc_abs(int x);
INTDEF long LIBCCALL libc_labs(long x);
INTDEF __LONGLONG LIBCCALL libc_llabs(__LONGLONG x);
INTDEF intmax_t LIBCCALL libc_imaxabs(intmax_t x);
INTDEF div_t LIBCCALL libc_div(int numer, int denom);
INTDEF ldiv_t LIBCCALL libc_ldiv(long numer, long denom);
INTDEF lldiv_t LIBCCALL libc_lldiv(long long numer, long long denom);
INTDEF imaxdiv_t LIBCCALL libc_imaxdiv(intmax_t numer, intmax_t denom);
INTDEF int LIBCCALL libc_system(char const *command);
INTDEF double LIBCCALL libc_drand48(void);
INTDEF long LIBCCALL libc_lrand48(void);
INTDEF long LIBCCALL libc_mrand48(void);
INTDEF double LIBCCALL libc_erand48(unsigned short xsubi[3]);
INTDEF long LIBCCALL libc_nrand48(unsigned short xsubi[3]);
INTDEF long LIBCCALL libc_jrand48(unsigned short xsubi[3]);
INTDEF void LIBCCALL libc_srand48(long seedval);
INTDEF unsigned short *LIBCCALL libc_seed48(unsigned short seed16v[3]);
INTDEF void LIBCCALL libc_lcong48(unsigned short param[7]);
INTDEF char *LIBCCALL libc_initstate(unsigned int seed, char *__restrict statebuf, size_t statelen);
INTDEF char *LIBCCALL libc_setstate(char *__restrict statebuf);
INTDEF char *LIBCCALL libc_l64a(long n);
INTDEF long LIBCCALL libc_a64l(char const *__restrict s);
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
INTDEF int LIBCCALL libc_mkstemps(char *__restrict template_, int suffixlen);
INTDEF int LIBCCALL libc_rpmatch(char const *__restrict response);
INTDEF char *LIBCCALL libc_canonicalize_file_name(char const *__restrict name);
INTDEF int LIBCCALL libc_ptsname_r(int fd, char *__restrict buf, size_t buflen);
INTDEF int LIBCCALL libc_getpt(void);
INTDEF int LIBCCALL libc_mkostemp(char *__restrict template_, int flags);
INTDEF int LIBCCALL libc_mkostemps(char *__restrict template_, int suffixlen, int flags);
INTDEF char *LIBCCALL libc_mktemp(char *__restrict template_);
INTDEF int LIBCCALL libc_mkstemp(char *__restrict template_);
INTDEF char *LIBCCALL libc_mkdtemp(char *__restrict template_);
INTDEF void LIBCCALL libc_setkey(char const *__restrict key);
INTDEF int LIBCCALL libc_grantpt(int fd);
INTDEF int LIBCCALL libc_unlockpt(int fd);
INTDEF char *LIBCCALL libc_ptsname(int fd);
INTDEF int LIBCCALL libc_posix_openpt(int oflag);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

INTDEF void *LIBCCALL libc_bsearch_s(void const *key, void const *base, size_t nmemb, size_t size, int (LIBCCALL *compar)(void *arg, const void *a, const void *b), void *arg);
INTDEF void LIBCCALL libc_qsort_s(void *base, size_t nmemb, size_t size, int (LIBCCALL *compar)(void *arg, const void *a, const void *b), void *arg);
INTDEF errno_t LIBCCALL libc_mktemp_s(char *__restrict templatename, size_t size);
INTDEF int LIBCCALL libc_16wsystem(char16_t const *__restrict command);
INTDEF int LIBCCALL libc_32wsystem(char32_t const *__restrict command);
INTDEF errno_t LIBCCALL libc_rand_s(unsigned int *__restrict randval);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_STDLIB_H */
