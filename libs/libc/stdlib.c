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
#ifndef GUARD_LIBS_LIBC_STDLIB_C
#define GUARD_LIBS_LIBC_STDLIB_C 1
#define _GNU_SOURCE 1

#include "libc.h"
#ifndef __KERNEL__
#include "system.h"
#endif
#include <assert.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <hybrid/traceback.h>
#include <hybrid/minmax.h>

DECL_BEGIN

#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(_Exit,_exit);
PUBLIC ATTR_NORETURN void (LIBCCALL _exit)(int status) {
//#undef __assertion_tbprint
// __assertion_tbprint();
 sys_exit(status);
}
PUBLIC ATTR_NORETURN void (LIBCCALL abort)(void) { _exit(EXIT_FAILURE); }
PUBLIC ATTR_NORETURN void (LIBCCALL exit)(int status) {
 /* TODO: Run atexit() */
 _exit(status);
}
PUBLIC ATTR_NORETURN void (LIBCCALL quick_exit)(int status) {
 /* TODO: Run at_quick_exit() */
 _exit(status);
}
PUBLIC int (LIBCCALL atexit)(void (*LIBCCALL func)(void)) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL at_quick_exit)(void (*LIBCCALL func) (void)) { NOT_IMPLEMENTED(); return 0; }
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

PRIVATE u32 rand_seed = 0;
PRIVATE u32 const rand_map[16] = {
    0x11e0ebcc,0xa914eba6,0xe400e438,0xa6c4a4df,
    0x0da46171,0x4b9a27d1,0x201910ae,0x95e213cb,
    0xd5ce0943,0x00005fdc,0x0319257d,0x09280b06,
    0x1148c0a6,0x07a24139,0x021214a6,0x03221af8,
};

PUBLIC void (LIBCCALL srand)(RAND_SEED seed) { rand_seed = (u32)seed; }
#ifndef __KERNEL__
#if __SIZEOF_LONG__ == __SIZEOF_INT__
DEFINE_PUBLIC_ALIAS(srandom,srand);
#else
PUBLIC void (LIBCCALL srandom)(unsigned int seed) { srand((RAND_SEED)seed); }
#endif
#endif


#ifdef __KERNEL__
#define LOCAL_RAND_TYPE RAND_TYPE
PUBLIC LOCAL_RAND_TYPE (LIBCCALL rand)(void)
#else /* __KERNEL__ */
#if __SIZEOF_LONG__ == __SIZEOF_INT__
#define LOCAL_RAND_TYPE RAND_TYPE
DEFINE_PUBLIC_ALIAS(random,rand);
PUBLIC LOCAL_RAND_TYPE (LIBCCALL rand)(void)
#else
#define LOCAL_RAND_TYPE long
PUBLIC RAND_TYPE (LIBCCALL rand)(void) {
 return (RAND_TYPE)random();
}
PUBLIC long (LIBCCALL random)(void)
#endif
#endif /* !__KERNEL__ */
{
 rand_seed  = (((rand_seed+7) << 1)/3);
 rand_seed ^= rand_map[(rand_seed >> (rand_seed&7)) % 16];
#ifdef __KERNEL__
 return (LOCAL_RAND_TYPE)rand_seed;
#else
 return (LOCAL_RAND_TYPE)(rand_seed & 0x7fffffff);
#endif
}
#undef LOCAL_RAND_TYPE

PUBLIC RAND_TYPE (LIBCCALL rand_r)(RAND_SEED_R *__restrict pseed) {
 RAND_SEED seed = (RAND_SEED)*pseed;
 seed  = (((seed+7) << 1)/3);
 seed ^= rand_map[(seed >> (seed&7)) % 16];
 *pseed = (RAND_SEED_R)seed;
#ifdef __KERNEL__
 return (RAND_TYPE)seed;
#else
 return (RAND_TYPE)(seed & 0x7fffffff);
#endif
}

#if __SIZEOF_LONG__ == 4
#define STRTOINT_32  strtol
#define STRTOUINT_32 strtoul
#define STRTOINT     strtol
#define STRTOUINT    strtoul
#define TYPE         long
#define UTYPE        unsigned long
#define DECL         PUBLIC
#include "templates/strtouint.code"
#if !defined(__KERNEL__) && __SIZEOF_LONG_LONG__ == 4
DEFINE_PUBLIC_ALIAS(strtoll,strtol)
DEFINE_PUBLIC_ALIAS(strtoull,strtoul)
#endif
#elif __SIZEOF_LONG_LONG__ == 4
#define STRTOINT_32  strtoll
#define STRTOUINT_32 strtoull
#define STRTOINT     strtoll
#define STRTOUINT    strtoull
#define TYPE         __LONGLONG
#define UTYPE        __ULONGLONG
#define DECL         PUBLIC
#include "templates/strtouint.code"
#else
#define STRTOINT_32  strtoint_32
#define STRTOUINT_32 strtouint_32
#define STRTOINT     strtoint_32
#define STRTOUINT    strtouint_32
#define TYPE         s64
#define UTYPE        u64
#define DECL         LOCAL
#include "templates/strtouint.code"
#endif

#if __SIZEOF_LONG__ == 8
#define STRTOINT_64  strtol
#define STRTOUINT_64 strtoul
#define STRTOINT     strtol
#define STRTOUINT    strtoul
#define TYPE         long
#define UTYPE        unsigned long
#define DECL         PUBLIC
#include "templates/strtouint.code"
#if !defined(__KERNEL__) && __SIZEOF_LONG_LONG__ == 4
DEFINE_PUBLIC_ALIAS(strtoll,strtol)
DEFINE_PUBLIC_ALIAS(strtoull,strtoul)
#endif
#elif __SIZEOF_LONG_LONG__ == 8
#define STRTOINT_64  strtoll
#define STRTOUINT_64 strtoull
#define STRTOINT     strtoll
#define STRTOUINT    strtoull
#define TYPE         __LONGLONG
#define UTYPE        __ULONGLONG
#define DECL         PUBLIC
#include "templates/strtouint.code"
#else
#define STRTOINT_64  strtoint_64
#define STRTOUINT_64 strtouint_64
#define STRTOINT     strtoint_64
#define STRTOUINT    strtouint_64
#define TYPE         s64
#define UTYPE        u64
#define DECL         LOCAL
#include "templates/strtouint.code"
#endif

PUBLIC int (LIBCCALL atoi)(char const *__restrict nptr) {
#if __SIZEOF_INT__ == 8
 return (int)STRTOINT_64(nptr,NULL,10);
#else
 return (int)STRTOINT_32(nptr,NULL,10);
#endif
}

#if __SIZEOF_LONG__ == __SIZEOF_INT__
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(atol,atoi);
#endif /* !__KERNEL__ */
#else
PUBLIC long (LIBCCALL atol)(char const *__restrict nptr) {
 return strtol(nptr,NULL,10);
}
#endif

#if __SIZEOF_LONG_LONG__ == __SIZEOF_INT__
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(atoll,atoi);
#endif /* !__KERNEL__ */
#elif __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(atoll,atol);
#endif /* !__KERNEL__ */
#else
PUBLIC __LONGLONG (LIBCCALL atoll)(char const *__restrict nptr) {
 return strtoll(nptr,NULL,10);
}
#endif


PUBLIC void *(LIBCCALL bsearch)(void const *key, void const *base, size_t nmemb,
                                size_t size, comparison_fn_t compar) {
 while (nmemb--) {
  if ((*compar)(key,base) == 0) return (void *)base;
  *(uintptr_t *)&base += size;
 }
 return NULL;
}

PRIVATE int (LIBCCALL call_comp)(void const *a, void const *b, void *arg) {
 return (*(comparison_fn_t)arg)(a,b);
}
PUBLIC void (LIBCCALL qsort)(void *base, size_t nmemb, size_t size,
                             comparison_fn_t compar) {
 qsort_r(base,nmemb,size,&call_comp,compar);
}

PUBLIC char *(LIBCCALL gcvt)(double value, int ndigit, char *buf) {
 NOT_IMPLEMENTED();
 return NULL;
}
PUBLIC char *(LIBCCALL qgcvt)(long double value, int ndigit, char *buf) {
 NOT_IMPLEMENTED();
 return NULL;
}
PUBLIC int (LIBCCALL ecvt_r)(double value, int ndigit, int *__restrict decpt,
                             int *__restrict sign, char *__restrict buf, size_t len) {
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC int (LIBCCALL fcvt_r)(double value, int ndigit, int *__restrict decpt,
                             int *__restrict sign, char *__restrict buf, size_t len) {
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC int (LIBCCALL qecvt_r)(long double value, int ndigit, int *__restrict decpt,
                              int *__restrict sign, char *__restrict buf, size_t len) {
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC int (LIBCCALL qfcvt_r)(long double value, int ndigit, int *__restrict decpt,
                              int *__restrict sign, char *__restrict buf, size_t len) {
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC double (LIBCCALL atof)(char const *__restrict nptr) {
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC double (LIBCCALL strtod)(char const *__restrict nptr, char **endptr) {
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC float (LIBCCALL strtof)(char const *__restrict nptr, char **__restrict endptr) {
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC long double (LIBCCALL strtold)(char const *__restrict nptr, char **__restrict endptr) {
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC int (LIBCCALL getloadavg)(double loadavg[], int nelem) {
 NOT_IMPLEMENTED();
 return 0;
}




/* DISCALIMER: The qsort() implementation below has been taken directly
 *             from glibc ('/stdlib/qsort.c'), before being retuned and
 *             formatted to best work with KOS.
 *          >> For better source documentation, consult the original function!
 */
/* Copyright (C) 1991-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Douglas C. Schmidt (schmidt@ics.uci.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
#define SWAP(a,b,size) \
do{ size_t __size = (size); \
    char *__a = (a), *__b = (b); \
    do{ char __tmp = *__a; \
        *__a++ = *__b; \
        *__b++ = __tmp; \
    } while (--__size > 0); \
}while(0)

#define MAX_THRESH 4
typedef struct { char *lo,*hi; } stack_node;
#define STACK_SIZE      (8*sizeof(size_t))
#define PUSH(low,high)  ((void)((top->lo = (low)),(top->hi = (high)),++top))
#define POP(low,high)   ((void)(--top,(low = top->lo),(high = top->hi)))
#define STACK_NOT_EMPTY (stack < top)

PUBLIC void LIBCCALL
qsort_r(void *pbase, size_t total_elems, size_t size,
        __compar_d_fn_t cmp, void *arg) {
 return;
 char *base_ptr = (char *)pbase;
 const size_t max_thresh = MAX_THRESH * size;
 if (total_elems == 0) return;
 if (total_elems > MAX_THRESH) {
  char *lo = base_ptr;
  char *hi = &lo[size * (total_elems-1)];
  stack_node stack[STACK_SIZE];
  stack_node *top = stack;
  PUSH(NULL,NULL);
  while (STACK_NOT_EMPTY){
   char *left_ptr;
   char *right_ptr;
   char *mid = lo+size * ((hi-lo) / size >> 1);
   if ((*cmp)((void *)mid,(void *)lo,arg) < 0) SWAP(mid,lo,size);
   if ((*cmp)((void *)hi,(void *)mid,arg) < 0) SWAP(mid,hi,size);
   else goto jump_over;
   if ((*cmp) ((void *)mid,(void *)lo,arg) < 0) SWAP(mid,lo,size);
jump_over:
   left_ptr  = lo+size;
   right_ptr = hi-size;
   do {
    while ((*cmp)((void *)left_ptr,(void *)mid,arg) < 0) left_ptr += size;
    while ((*cmp)((void *)mid,(void *)right_ptr,arg) < 0) right_ptr -= size;
    if (left_ptr < right_ptr) {
     SWAP(left_ptr,right_ptr,size);
     if (mid == left_ptr) mid = right_ptr;
     else if (mid == right_ptr) mid = left_ptr;
     left_ptr += size;
     right_ptr -= size;
    } else if (left_ptr == right_ptr) {
     left_ptr += size;
     right_ptr -= size;
     break;
    }
   } while (left_ptr <= right_ptr);
   if ((size_t)(right_ptr-lo) <= max_thresh) {
    if ((size_t)(hi-left_ptr) <= max_thresh) POP(lo,hi);
    else lo = left_ptr;
   } else if ((size_t)(hi-left_ptr) <= max_thresh)
    hi = right_ptr;
   else if ((right_ptr-lo) > (hi - left_ptr)) {
    PUSH(lo,right_ptr);
    lo = left_ptr;
   } else {
    PUSH(left_ptr,hi);
    hi = right_ptr;
   }
  }
 }
 {
  char *const end_ptr = &base_ptr[size * (total_elems-1)];
  char *run_ptr,*tmp_ptr = base_ptr;
  char *thresh = MIN(end_ptr,base_ptr+max_thresh);
  for (run_ptr = tmp_ptr+size; run_ptr <= thresh; run_ptr += size) {
   if ((*cmp) ((void *)run_ptr,(void *)tmp_ptr,arg) < 0)
        tmp_ptr = run_ptr;
  }
  if (tmp_ptr != base_ptr)
      SWAP(tmp_ptr,base_ptr,size);
  run_ptr = base_ptr+size;
  while ((run_ptr += size) <= end_ptr) {
   tmp_ptr = run_ptr-size;
   while ((*cmp)((void *)run_ptr,(void *)tmp_ptr,arg) < 0)
           tmp_ptr -= size;
   tmp_ptr += size;
   if (tmp_ptr != run_ptr) {
    char *trav = run_ptr+size;
    while (--trav >= run_ptr) {
     char *hi,*lo,c = *trav;
     for (hi = lo = trav;
         (lo -= size) >= tmp_ptr;
          hi = lo) *hi = *lo;
     *hi = c;
    }
   }
  }
 }
}




#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(strtoq,strtoll);
DEFINE_PUBLIC_ALIAS(strtouq,strtoull);
DEFINE_PUBLIC_ALIAS(mkostemp64,mkostemp);
DEFINE_PUBLIC_ALIAS(mkostemps64,mkostemps);
DEFINE_PUBLIC_ALIAS(mkstemp64,mkstemp);
DEFINE_PUBLIC_ALIAS(mkstemps64,mkstemps);

PUBLIC int (LIBCCALL getsubopt)(char **__restrict optionp,
                                char *const *__restrict tokens,
                                char **__restrict valuep) {
 NOT_IMPLEMENTED();
 return 0;
}

PUBLIC int (LIBCCALL abs)(int x) { return x < 0 ? -x : x; }
PUBLIC long (LIBCCALL labs)(long x) { return x < 0 ? -x : x; }
PUBLIC __LONGLONG (LIBCCALL llabs)(__LONGLONG x) { return x < 0 ? -x : x; }
PUBLIC div_t (LIBCCALL div)(int numer, int denom) { return (div_t){ numer/denom,numer%denom }; }
PUBLIC ldiv_t (LIBCCALL ldiv)(long numer, long denom) { return (ldiv_t){ numer/denom,numer%denom }; }
PUBLIC lldiv_t (LIBCCALL lldiv)(long long numer, long long denom) { return (lldiv_t){ numer/denom,numer%denom }; }

PUBLIC size_t (LIBCCALL __ctype_get_mb_cur_max)(void) { NOT_IMPLEMENTED(); return 5; }
PUBLIC int (LIBCCALL system)(char const *command) { NOT_IMPLEMENTED(); return 5; }
PUBLIC int (LIBCCALL mblen)(char const *s, size_t n) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL mbtowc)(wchar_t *__restrict pwc, char const *__restrict s, size_t n) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL wctomb)(char *s, wchar_t wchar) { NOT_IMPLEMENTED(); return 0; }
PUBLIC size_t (LIBCCALL mbstowcs)(wchar_t *__restrict pwcs, char const *__restrict s, size_t n) { NOT_IMPLEMENTED(); return 0; }
PUBLIC size_t (LIBCCALL wcstombs)(char *__restrict s, const wchar_t *__restrict pwcs, size_t n) { NOT_IMPLEMENTED(); return 0; }
PUBLIC double (LIBCCALL drand48)(void) { NOT_IMPLEMENTED(); return 0; }
PUBLIC long (LIBCCALL lrand48)(void) { NOT_IMPLEMENTED(); return 0; }
PUBLIC long (LIBCCALL mrand48)(void) { NOT_IMPLEMENTED(); return 0; }
PUBLIC double (LIBCCALL erand48)(unsigned short xsubi[3]) { NOT_IMPLEMENTED(); return 0; }
PUBLIC long (LIBCCALL nrand48)(unsigned short xsubi[3]) { NOT_IMPLEMENTED(); return 0; }
PUBLIC long (LIBCCALL jrand48)(unsigned short xsubi[3]) { NOT_IMPLEMENTED(); return 0; }
PUBLIC void (LIBCCALL srand48)(long seedval) { NOT_IMPLEMENTED(); }
PUBLIC unsigned short *(LIBCCALL seed48)(unsigned short seed16v[3]) { NOT_IMPLEMENTED(); return seed16v; }
PUBLIC void (LIBCCALL lcong48)(unsigned short param[7]) { NOT_IMPLEMENTED(); }
PUBLIC char *(LIBCCALL initstate)(unsigned int seed, char *statebuf, size_t statelen) { NOT_IMPLEMENTED(); return statebuf; }
PUBLIC char *(LIBCCALL setstate)(char *statebuf) { NOT_IMPLEMENTED(); return statebuf; }
PUBLIC char *(LIBCCALL l64a)(long n) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC long (LIBCCALL a64l)(char const *s) { NOT_IMPLEMENTED(); return 0; }
PUBLIC char *(LIBCCALL realpath)(char const *__restrict name, char *__restrict resolved) { NOT_IMPLEMENTED(); return resolved; }
PUBLIC int (LIBCCALL drand48_r)(struct drand48_data *__restrict buffer, double *__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL erand48_r)(unsigned short xsubi[3], struct drand48_data *__restrict buffer, double *__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL lrand48_r)(struct drand48_data *__restrict buffer, long *__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL nrand48_r)(unsigned short xsubi[3], struct drand48_data *__restrict buffer, long *__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL mrand48_r)(struct drand48_data *__restrict buffer, long *__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL jrand48_r)(unsigned short xsubi[3], struct drand48_data *__restrict buffer, long *__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL srand48_r)(long seedval, struct drand48_data *buffer) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL seed48_r)(unsigned short seed16v[3], struct drand48_data *buffer) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL lcong48_r)(unsigned short param[7], struct drand48_data *buffer) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL random_r)(struct random_data *__restrict buf, s32 *__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL srandom_r)(unsigned int seed, struct random_data *buf) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL initstate_r)(unsigned int seed, char *__restrict statebuf, size_t statelen, struct random_data *__restrict buf) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL setstate_r)(char *__restrict statebuf, struct random_data *__restrict buf) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL on_exit)(void (LIBCCALL *func)(int status, void *arg), void *arg) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL mkstemps)(char *template_, int suffixlen) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL rpmatch)(char const *response) { NOT_IMPLEMENTED(); return 0; }
#define FLOAT_BUFFER_SIZE 64
PUBLIC char *(LIBCCALL qecvt)(long double value, int ndigit, int *__restrict decpt, int *__restrict sign) { PRIVATE char buffer[FLOAT_BUFFER_SIZE]; return qecvt_r(value,ndigit,decpt,sign,buffer,sizeof(buffer)) ? NULL : buffer; }
PUBLIC char *(LIBCCALL qfcvt)(long double value, int ndigit, int *__restrict decpt, int *__restrict sign) { PRIVATE char buffer[FLOAT_BUFFER_SIZE]; return qfcvt_r(value,ndigit,decpt,sign,buffer,sizeof(buffer)) ? NULL : buffer; }
PUBLIC char *(LIBCCALL ecvt)(double value, int ndigit, int *__restrict decpt, int *__restrict sign) { PRIVATE char buffer[FLOAT_BUFFER_SIZE]; return ecvt_r(value,ndigit,decpt,sign,buffer,sizeof(buffer)) ? NULL : buffer; }
PUBLIC char *(LIBCCALL fcvt)(double value, int ndigit, int *__restrict decpt, int *__restrict sign) { PRIVATE char buffer[FLOAT_BUFFER_SIZE]; return fcvt_r(value,ndigit,decpt,sign,buffer,sizeof(buffer)) ? NULL : buffer; }
#undef FLOAT_BUFFER_SIZE
PUBLIC long (LIBCCALL strtol_l)(char const *__restrict nptr, char **__restrict endptr, int base, locale_t UNUSED(loc)) { return strtol(nptr,endptr,base); }
PUBLIC unsigned long (LIBCCALL strtoul_l)(char const *__restrict nptr, char **__restrict endptr, int base, locale_t UNUSED(loc)) { return strtoul(nptr,endptr,base); }
PUBLIC __LONGLONG (LIBCCALL strtoll_l)(char const *__restrict nptr, char **__restrict endptr, int base, locale_t UNUSED(loc)) { return strtoll(nptr,endptr,base); }
PUBLIC __ULONGLONG (LIBCCALL strtoull_l)(char const *__restrict nptr, char **__restrict endptr, int base, locale_t UNUSED(loc)) { return strtoull(nptr,endptr,base); }
PUBLIC double (LIBCCALL strtod_l)(char const *__restrict nptr, char **__restrict endptr, locale_t UNUSED(loc)) { return strtod(nptr,endptr); }
PUBLIC float (LIBCCALL strtof_l)(char const *__restrict nptr, char **__restrict endptr, locale_t UNUSED(loc)) { return strtof(nptr,endptr); }
PUBLIC long double (LIBCCALL strtold_l)(char const *__restrict nptr, char **__restrict endptr, locale_t UNUSED(loc)) { return strtold(nptr,endptr); }
PUBLIC char *(LIBCCALL canonicalize_file_name)(char const *name) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL ptsname_r)(int fd, char *buf, size_t buflen) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL getpt)(void) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL mkostemp)(char *template_, int flags) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL mkostemps)(char *template_, int suffixlen, int flags) { NOT_IMPLEMENTED(); return 0; }
PUBLIC char *(LIBCCALL mktemp)(char *template_) { NOT_IMPLEMENTED(); return template_; }
PUBLIC int (LIBCCALL mkstemp)(char *template_) { NOT_IMPLEMENTED(); return 0; }
PUBLIC char *(LIBCCALL mkdtemp)(char *template_) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC void (LIBCCALL setkey)(char const *key) { NOT_IMPLEMENTED(); }
PUBLIC int (LIBCCALL grantpt)(int fd) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL unlockpt)(int fd) { NOT_IMPLEMENTED(); return 0; }
PUBLIC char *(LIBCCALL ptsname)(int fd) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL posix_openpt)(int oflag) { NOT_IMPLEMENTED(); return 0; }
#endif /* !__KERNEL__ */


DECL_END

#ifndef __INTELLISENSE__
#include "typecore.c.inl"
#endif

#endif /* !GUARD_LIBS_LIBC_STDLIB_C */
