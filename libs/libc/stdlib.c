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
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include "libc.h"
#include "stdlib.h"
#include "malloc.h"
#ifndef __KERNEL__
#include "stdio.h"
#include "file.h"
#include "system.h"
#include "unistd.h"
#include "unicode.h"
#include "string.h"
#include "errno.h"
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include <bits/dos-errno.h>
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */
#include <assert.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdbool.h>
#include <hybrid/traceback.h>
#include <hybrid/minmax.h>
#include <hybrid/host.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwptr.h>
#include <hybrid/section.h>
#include <hybrid/atomic.h>
#include <sys/wait.h>

DECL_BEGIN

#ifndef __KERNEL__
PRIVATE ATTR_COLDBSS atomic_rwptr_t onexit_n = ATOMIC_RWPTR_INIT(NULL);
PRIVATE ATTR_COLDBSS atomic_rwptr_t onexit_q = ATOMIC_RWPTR_INIT(NULL);

PRIVATE void LIBCCALL
run_onexit(struct exitcall *chain, int status) {
 for (; chain; chain = chain->ec_next.le_next)
     (*chain->ec_func)(status,chain->ec_arg);
}
PRIVATE int LIBCCALL
add_onexit(atomic_rwptr_t *pchain, exitfunc func, void *arg) {
 struct exitcall *entry;
 entry = (struct exitcall *)libc_memalign(ATOMIC_RWPTR_ALIGN,
                                          sizeof(struct exitcall));
 if unlikely(!entry) return -1;
 entry->ec_func = func;
 entry->ec_arg  = arg;
 (void)libc__mall_untrack(entry);
 atomic_rwptr_write(pchain);
 entry->ec_next.le_next = (struct exitcall *)ATOMIC_RWPTR_GET(*pchain);
 ATOMIC_WRITE(pchain->ap_ptr,entry);
 return 0;
}

PRIVATE ATTR_COLDTEXT
void LIBCCALL libc_run_common_atexit(void) {
 /* Flush all file streams. */
 libc_flushall();
}
INTERN ATTR_COLDTEXT void LIBCCALL libc_run_atexit(void) {
 atomic_rwptr_read(&onexit_n);
 run_onexit((struct exitcall *)ATOMIC_RWPTR_GET(onexit_n),0);
 atomic_rwptr_endread(&onexit_n);
 libc_run_common_atexit();
}
INTERN ATTR_COLDTEXT void LIBCCALL libc_run_at_quick_exit(void) {
 atomic_rwptr_read(&onexit_q);
 run_onexit((struct exitcall *)ATOMIC_RWPTR_GET(onexit_q),0);
 atomic_rwptr_endread(&onexit_q);
 libc_run_common_atexit();
}
INTERN ATTR_NORETURN ATTR_COLDTEXT void LIBCCALL libc__exit(int status) { sys_exit_group(status); }
INTERN ATTR_NORETURN ATTR_COLDTEXT void LIBCCALL libc_abort(void) { libc__exit(EXIT_FAILURE); }
INTERN ATTR_NORETURN ATTR_COLDTEXT void LIBCCALL libc_exit(int status) {
 atomic_rwptr_read(&onexit_n);
 run_onexit((struct exitcall *)ATOMIC_RWPTR_GET(onexit_n),status);
 libc__exit(status);
}
INTERN ATTR_NORETURN ATTR_COLDTEXT void LIBCCALL libc_quick_exit(int status) {
 atomic_rwptr_read(&onexit_q);
 run_onexit((struct exitcall *)ATOMIC_RWPTR_GET(onexit_q),status);
 libc__exit(status);
}

PRIVATE void LIBCCALL callarg(int status, void *arg) { (*(void (LIBCCALL *)(void))arg)(); }
INTERN int LIBCCALL libc_on_exit(void (LIBCCALL *func)(int status, void *arg), void *arg) { return add_onexit(&onexit_n,func,arg); }
INTERN int LIBCCALL libc_atexit(void (LIBCCALL *func)(void)) { return add_onexit(&onexit_n,&callarg,(void *)func); }
INTERN int LIBCCALL libc_at_quick_exit(void (LIBCCALL *func)(void)) { return add_onexit(&onexit_q,&callarg,(void *)func); }


#ifndef CONFIG_LIBC_NO_DOS_LIBC
#if defined(__i386__) || defined(__x86_64__) || defined(__ia64__)
DEFINE_INTERN_ALIAS(libc_crt_terminate_process,libc__exit);
DEFINE_PUBLIC_ALIAS(__crtTerminateProcess,libc_crt_terminate_process);
#endif
DEFINE_PUBLIC_ALIAS(_cexit,libc_run_atexit);
DEFINE_PUBLIC_ALIAS(_c_exit,libc_run_at_quick_exit);

INTDEF ATTR_DOSTEXT void (LIBCCALL *LIBCCALL libc_get_terminate(void))(void) { return &libc_abort; }
DEFINE_PUBLIC_ALIAS(_get_terminate,libc_get_terminate); /* I think??? */
DEFINE_PUBLIC_ALIAS(_get_unexpected,libc_get_terminate); /* I think??? */

DEFINE_PUBLIC_ALIAS(_amsg_exit,libc_amsg_exit);
INTERN ATTR_NORETURN ATTR_DOSTEXT
void LIBCCALL libc_amsg_exit(int errnum) {
 SET_ERRNO(errnum);
 libc_perror(DOSSTR("_amsg_exit() called"));
 libc__exit(255);
}

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */

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

INTERN void LIBCCALL libc_srand(RAND_SEED seed) { rand_seed = (u32)seed; }
#if !defined(__KERNEL__) && __SIZEOF_LONG__ != __SIZEOF_INT__
INTERN void LIBCCALL libc_srandom(unsigned int seed) { rand_seed = (u32)seed; }
#endif


#ifdef __KERNEL__
#define LOCAL_RAND_TYPE RAND_TYPE
INTERN LOCAL_RAND_TYPE LIBCCALL libc_rand(void)
#else /* __KERNEL__ */
#if __SIZEOF_LONG__ == __SIZEOF_INT__
#define LOCAL_RAND_TYPE RAND_TYPE
INTERN LOCAL_RAND_TYPE LIBCCALL libc_rand(void)
#else
#define LOCAL_RAND_TYPE long
INTERN RAND_TYPE LIBCCALL libc_rand(void) {
 return (RAND_TYPE)libc_random();
}
INTERN long LIBCCALL libc_random(void)
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

INTERN RAND_TYPE LIBCCALL libc_rand_r(RAND_SEED_R *__restrict pseed) {
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


INTERN void *LIBCCALL
libc_bsearch(void const *key, void const *base, size_t nmemb,
             size_t size, comparison_fn_t compar) {
 while (nmemb--) {
  if ((*compar)(key,base) == 0) return (void *)base;
  *(uintptr_t *)&base += size;
 }
 return NULL;
}

INTERN int LIBCCALL libc_getloadavg(double loadavg[], int nelem) {
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

INTERN void LIBCCALL
libc_qsort_r(void *pbase, size_t total_elems, size_t size,
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

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_qsort,libc_qsort_r);
#else
PRIVATE int LIBCCALL
call_comp(void const *a, void const *b, void *arg) {
 return (*(comparison_fn_t)arg)(a,b);
}
INTERN void LIBCCALL
libc_qsort(void *base, size_t nmemb, size_t size,
           comparison_fn_t compar) {
 libc_qsort_r(base,nmemb,size,&call_comp,compar);
}
#endif



#ifndef __KERNEL__
INTERN int LIBCCALL
libc_getsubopt(char **__restrict optionp,
               char *const *__restrict tokens,
               char **__restrict valuep) {
 NOT_IMPLEMENTED();
 return 0;
}

INTERN int LIBCCALL libc_abs(int x) { return x < 0 ? -x : x; }
INTERN div_t LIBCCALL libc_div(int numer, int denom) {
 return (div_t){ numer/denom, numer%denom };
}
#if __SIZEOF_LONG__ == __SIZEOF_INT__
DEFINE_INTERN_ALIAS(libc_ldiv,libc_div);
DEFINE_INTERN_ALIAS(libc_labs,libc_abs);
#else
INTERN long LIBCCALL libc_labs(long x) { return x < 0 ? -x : x; }
INTERN ldiv_t LIBCCALL libc_ldiv(long numer, long denom) {
 return (ldiv_t){ numer/denom, numer%denom };
}
#endif
#if __SIZEOF_LONG_LONG__ == __SIZEOF_INT__
DEFINE_INTERN_ALIAS(libc_lldiv,libc_div);
DEFINE_INTERN_ALIAS(libc_llabs,libc_abs);
#elif __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__
DEFINE_INTERN_ALIAS(libc_lldiv,libc_ldiv);
DEFINE_INTERN_ALIAS(libc_llabs,libc_labs);
#else
INTERN __LONGLONG LIBCCALL libc_llabs(__LONGLONG x) { return x < 0 ? -x : x; }
INTERN lldiv_t LIBCCALL libc_lldiv(long long numer, long long denom) {
 return (lldiv_t){ numer/denom, numer%denom };
}
#endif
#if __SIZEOF_INTMAX_T__ == __SIZEOF_INT__
DEFINE_INTERN_ALIAS(libc_imaxdiv,libc_div);
DEFINE_INTERN_ALIAS(libc_imaxabs,libc_abs);
#elif __SIZEOF_INTMAX_T__ == __SIZEOF_LONG__
DEFINE_INTERN_ALIAS(libc_imaxdiv,libc_ldiv);
DEFINE_INTERN_ALIAS(libc_imaxabs,libc_labs);
#elif __SIZEOF_INTMAX_T__ == __SIZEOF_LONG_LONG__
DEFINE_INTERN_ALIAS(libc_imaxdiv,libc_lldiv);
DEFINE_INTERN_ALIAS(libc_imaxabs,libc_llabs);
#else
INTERN intmax_t LIBCCALL libc_imaxabs(intmax_t x) { return x < 0 ? -x : x; }
INTERN imaxdiv_t LIBCCALL libc_imaxdiv(intmax_t numer, intmax_t denom) {
 return (imaxdiv_t){ numer/denom, numer%denom };
}
#endif
INTERN int LIBCCALL libc_system(char const *__restrict command) {
 pid_t child,error; int status;
 if ((child = libc_fork()) < 0) return -1; /* XXX: Use vfork()? */
 if (child == 0) {
  libc_execl("/bin/sh","sh","-c",command,NULL);
  libc_execl("/bin/busybox","sh","-c",command,NULL);
  /* NOTE: system() must return ZERO(0) if no command processor is available. */
  libc__exit(command ? 127 : 0);
 }
 for (;;) {
  error = libc_waitpid(child,&status,WEXITED);
  if (error == child) break;
  if (error >= 0) continue;
  if (GET_ERRNO() != EINTR) return -1;
 }
 return status;
}

INTERN double LIBCCALL libc_drand48(void) { NOT_IMPLEMENTED(); return 0; }
INTERN long LIBCCALL libc_lrand48(void) { NOT_IMPLEMENTED(); return 0; }
INTERN long LIBCCALL libc_mrand48(void) { NOT_IMPLEMENTED(); return 0; }
INTERN double LIBCCALL libc_erand48(unsigned short xsubi[3]) { NOT_IMPLEMENTED(); return 0; }
INTERN long LIBCCALL libc_nrand48(unsigned short xsubi[3]) { NOT_IMPLEMENTED(); return 0; }
INTERN long LIBCCALL libc_jrand48(unsigned short xsubi[3]) { NOT_IMPLEMENTED(); return 0; }
INTERN void LIBCCALL libc_srand48(long seedval) { NOT_IMPLEMENTED(); }
INTERN unsigned short *LIBCCALL libc_seed48(unsigned short seed16v[3]) { NOT_IMPLEMENTED(); return seed16v; }
INTERN void LIBCCALL libc_lcong48(unsigned short param[7]) { NOT_IMPLEMENTED(); }
INTERN char *LIBCCALL libc_initstate(unsigned int seed, char *__restrict statebuf, size_t statelen) { NOT_IMPLEMENTED(); return statebuf; }
INTERN char *LIBCCALL libc_setstate(char *__restrict statebuf) { NOT_IMPLEMENTED(); return statebuf; }
INTERN char *LIBCCALL libc_l64a(long n) { NOT_IMPLEMENTED(); return NULL; }
INTERN long LIBCCALL libc_a64l(char const *__restrict s) { NOT_IMPLEMENTED(); return 0; }
INTERN char *LIBCCALL libc_realpath(char const *__restrict name, char *__restrict resolved) { NOT_IMPLEMENTED(); return resolved; }
INTERN int LIBCCALL libc_drand48_r(struct drand48_data *__restrict buffer, double *__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_erand48_r(unsigned short xsubi[3], struct drand48_data *__restrict buffer, double *__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_lrand48_r(struct drand48_data *__restrict buffer, long *__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_nrand48_r(unsigned short xsubi[3], struct drand48_data *__restrict buffer, long *__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_mrand48_r(struct drand48_data *__restrict buffer, long *__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_jrand48_r(unsigned short xsubi[3], struct drand48_data *__restrict buffer, long *__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_srand48_r(long seedval, struct drand48_data *buffer) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_seed48_r(unsigned short seed16v[3], struct drand48_data *buffer) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_lcong48_r(unsigned short param[7], struct drand48_data *buffer) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_random_r(struct random_data *__restrict buf, s32 *__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_srandom_r(unsigned int seed, struct random_data *buf) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_initstate_r(unsigned int seed, char *__restrict statebuf, size_t statelen, struct random_data *__restrict buf) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_setstate_r(char *__restrict statebuf, struct random_data *__restrict buf) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_mkstemps(char *__restrict template_, int suffixlen) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_rpmatch(char const *__restrict response) { NOT_IMPLEMENTED(); return 0; }
INTERN char *LIBCCALL libc_canonicalize_file_name(char const *__restrict name) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_ptsname_r(int fd, char *__restrict buf, size_t buflen) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_getpt(void) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_mkostemp(char *__restrict template_, int flags) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_mkostemps(char *__restrict template_, int suffixlen, int flags) { NOT_IMPLEMENTED(); return 0; }
INTERN char *LIBCCALL libc_mktemp(char *__restrict template_) { NOT_IMPLEMENTED(); return template_; }
INTERN int LIBCCALL libc_mkstemp(char *__restrict template_) { NOT_IMPLEMENTED(); return 0; }
INTERN char *LIBCCALL libc_mkdtemp(char *__restrict template_) { NOT_IMPLEMENTED(); return NULL; }
INTERN void LIBCCALL libc_setkey(char const *__restrict key) { NOT_IMPLEMENTED(); }
INTERN int LIBCCALL libc_grantpt(int fd) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_unlockpt(int fd) { NOT_IMPLEMENTED(); return 0; }
INTERN char *LIBCCALL libc_ptsname(int fd) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_posix_openpt(int oflag) { NOT_IMPLEMENTED(); return 0; }
#endif /* !__KERNEL__ */



#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(_exit,libc__exit);
DEFINE_PUBLIC_ALIAS(abort,libc_abort);
DEFINE_PUBLIC_ALIAS(exit,libc_exit);
DEFINE_PUBLIC_ALIAS(quick_exit,libc_quick_exit);
DEFINE_PUBLIC_ALIAS(on_exit,libc_on_exit);
DEFINE_PUBLIC_ALIAS(atexit,libc_atexit);
DEFINE_PUBLIC_ALIAS(at_quick_exit,libc_at_quick_exit);
DEFINE_PUBLIC_ALIAS(__cxa_atexit,libc_atexit);
#if __SIZEOF_LONG__ != __SIZEOF_INT__
DEFINE_PUBLIC_ALIAS(srandom,libc_srandom);
DEFINE_PUBLIC_ALIAS(random,libc_random);
#endif /* __SIZEOF_LONG__ != __SIZEOF_INT__ */
#endif /* !__KERNEL__ */

DEFINE_PUBLIC_ALIAS(srand,libc_srand);
DEFINE_PUBLIC_ALIAS(rand,libc_rand);
DEFINE_PUBLIC_ALIAS(rand_r,libc_rand_r);
DEFINE_PUBLIC_ALIAS(bsearch,libc_bsearch);
DEFINE_PUBLIC_ALIAS(qsort,libc_qsort);
DEFINE_PUBLIC_ALIAS(getloadavg,libc_getloadavg);
DEFINE_PUBLIC_ALIAS(qsort_r,libc_qsort_r);
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(getsubopt,libc_getsubopt);
DEFINE_PUBLIC_ALIAS(abs,libc_abs);
DEFINE_PUBLIC_ALIAS(labs,libc_labs);
DEFINE_PUBLIC_ALIAS(llabs,libc_llabs);
DEFINE_PUBLIC_ALIAS(imaxabs,libc_imaxabs);
DEFINE_PUBLIC_ALIAS(div,libc_div);
DEFINE_PUBLIC_ALIAS(ldiv,libc_ldiv);
DEFINE_PUBLIC_ALIAS(lldiv,libc_lldiv);
DEFINE_PUBLIC_ALIAS(imaxdiv,libc_imaxdiv);
DEFINE_PUBLIC_ALIAS(system,libc_system);
DEFINE_PUBLIC_ALIAS(drand48,libc_drand48);
DEFINE_PUBLIC_ALIAS(lrand48,libc_lrand48);
DEFINE_PUBLIC_ALIAS(mrand48,libc_mrand48);
DEFINE_PUBLIC_ALIAS(erand48,libc_erand48);
DEFINE_PUBLIC_ALIAS(nrand48,libc_nrand48);
DEFINE_PUBLIC_ALIAS(jrand48,libc_jrand48);
DEFINE_PUBLIC_ALIAS(srand48,libc_srand48);
DEFINE_PUBLIC_ALIAS(seed48,libc_seed48);
DEFINE_PUBLIC_ALIAS(lcong48,libc_lcong48);
DEFINE_PUBLIC_ALIAS(initstate,libc_initstate);
DEFINE_PUBLIC_ALIAS(setstate,libc_setstate);
DEFINE_PUBLIC_ALIAS(l64a,libc_l64a);
DEFINE_PUBLIC_ALIAS(a64l,libc_a64l);
DEFINE_PUBLIC_ALIAS(realpath,libc_realpath);
DEFINE_PUBLIC_ALIAS(drand48_r,libc_drand48_r);
DEFINE_PUBLIC_ALIAS(erand48_r,libc_erand48_r);
DEFINE_PUBLIC_ALIAS(lrand48_r,libc_lrand48_r);
DEFINE_PUBLIC_ALIAS(nrand48_r,libc_nrand48_r);
DEFINE_PUBLIC_ALIAS(mrand48_r,libc_mrand48_r);
DEFINE_PUBLIC_ALIAS(jrand48_r,libc_jrand48_r);
DEFINE_PUBLIC_ALIAS(srand48_r,libc_srand48_r);
DEFINE_PUBLIC_ALIAS(seed48_r,libc_seed48_r);
DEFINE_PUBLIC_ALIAS(lcong48_r,libc_lcong48_r);
DEFINE_PUBLIC_ALIAS(random_r,libc_random_r);
DEFINE_PUBLIC_ALIAS(srandom_r,libc_srandom_r);
DEFINE_PUBLIC_ALIAS(initstate_r,libc_initstate_r);
DEFINE_PUBLIC_ALIAS(setstate_r,libc_setstate_r);
DEFINE_PUBLIC_ALIAS(mkstemps,libc_mkstemps);
DEFINE_PUBLIC_ALIAS(rpmatch,libc_rpmatch);
DEFINE_PUBLIC_ALIAS(canonicalize_file_name,libc_canonicalize_file_name);
DEFINE_PUBLIC_ALIAS(ptsname_r,libc_ptsname_r);
DEFINE_PUBLIC_ALIAS(getpt,libc_getpt);
DEFINE_PUBLIC_ALIAS(mkostemp,libc_mkostemp);
DEFINE_PUBLIC_ALIAS(mkostemps,libc_mkostemps);
DEFINE_PUBLIC_ALIAS(mktemp,libc_mktemp);
DEFINE_PUBLIC_ALIAS(mkstemp,libc_mkstemp);
DEFINE_PUBLIC_ALIAS(mkdtemp,libc_mkdtemp);
DEFINE_PUBLIC_ALIAS(setkey,libc_setkey);
DEFINE_PUBLIC_ALIAS(grantpt,libc_grantpt);
DEFINE_PUBLIC_ALIAS(unlockpt,libc_unlockpt);
DEFINE_PUBLIC_ALIAS(ptsname,libc_ptsname);
DEFINE_PUBLIC_ALIAS(posix_openpt,libc_posix_openpt);
#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS("?terminate%%YAXXZ",libc_abort);
DEFINE_PUBLIC_ALIAS(_mktemp,libc_mktemp);
DEFINE_PUBLIC_ALIAS(_onexit,libc_atexit);
DEFINE_PUBLIC_ALIAS(__dllonexit,libc_atexit);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */

#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(_Exit,libc__exit);
#if __SIZEOF_LONG__ == __SIZEOF_INT__
DEFINE_PUBLIC_ALIAS(srandom,libc_srand);
DEFINE_PUBLIC_ALIAS(random,libc_rand);
#endif
#if __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
DEFINE_PUBLIC_ALIAS(strtoll,libc_strtol)
DEFINE_PUBLIC_ALIAS(strtoull,libc_strtoul)
#endif
DEFINE_PUBLIC_ALIAS(mkostemp64,libc_mkostemp);
DEFINE_PUBLIC_ALIAS(mkostemps64,libc_mkostemps);
DEFINE_PUBLIC_ALIAS(mkstemp64,libc_mkstemp);
DEFINE_PUBLIC_ALIAS(mkstemps64,libc_mkstemps);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#if __SIZEOF_LONG_LONG__ == 8
DEFINE_PUBLIC_ALIAS(_abs64,libc_llabs);
#elif __SIZEOF_LONG__ == 8
DEFINE_PUBLIC_ALIAS(_abs64,libc_labs);
#elif __SIZEOF_INTMAX_T__ == 8
DEFINE_PUBLIC_ALIAS(_abs64,libc_imaxabs);
#else
#error FIXME
#endif

INTERN ATTR_DOSTEXT void *LIBCCALL
libc_bsearch_s(void const *key, void const *base, size_t nmemb, size_t size,
               int (LIBCCALL *compar)(void *arg, const void *a, const void *b),
               void *arg) {
 while (nmemb--) {
  if ((*compar)(arg,key,base) == 0) return (void *)base;
  *(uintptr_t *)&base += size;
 }
 return NULL;
}
struct dos_qsort_compar_d {
 int (LIBCCALL *cmp)(void *arg, const void *a, const void *b);
 void          *arg;
};
PRIVATE ATTR_DOSTEXT int LIBCCALL
dos_qsort_compar_f(void const *a, void const *b, void *arg) {
 return (*((struct dos_qsort_compar_d *)arg)->cmp)
        ( ((struct dos_qsort_compar_d *)arg)->arg,a,b);
}

INTERN ATTR_DOSTEXT void LIBCCALL
libc_qsort_s(void *base, size_t nmemb, size_t size,
             int (LIBCCALL *compar)(void *arg, const void *a, const void *b),
             void *arg) {
 struct dos_qsort_compar_d d;
 d.cmp = compar,d.arg = arg;
 libc_qsort_r(base,nmemb,size,&dos_qsort_compar_f,&d);
}

INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_mktemp_s(char *templatename, size_t size) {
 return templatename && size && libc_mktemp(templatename) ? EOK : EINVAL;
}

INTERN ATTR_DOSTEXT int LIBCCALL libc_16wsystem(char16_t const *__restrict command) {
 int result = -1; char *utf8_command = libc_utf16to8m(command);
 if (utf8_command) result = libc_system(utf8_command),libc_free(utf8_command);
 return result;
}
INTERN ATTR_DOSTEXT int LIBCCALL libc_32wsystem(char32_t const *__restrict command) {
 int result = -1; char *utf8_command = libc_utf32to8m(command);
 if (utf8_command) result = libc_system(utf8_command),libc_free(utf8_command);
 return result;
}
INTDEF ATTR_DOSTEXT errno_t LIBCCALL
libc_rand_s(unsigned int *__restrict randval) {
 if (!randval) return __DOS_EINVAL;
 *randval = libc_rand();
 return EOK;
}

DEFINE_PUBLIC_ALIAS(bsearch_s,libc_bsearch_s);
DEFINE_PUBLIC_ALIAS(qsort_s,libc_qsort_s);
DEFINE_PUBLIC_ALIAS(_mktemp_s,libc_mktemp_s);
DEFINE_PUBLIC_ALIAS(wsystem,libc_32wsystem);
DEFINE_PUBLIC_ALIAS(_wsystem,libc_16wsystem);
DEFINE_PUBLIC_ALIAS(rand_s,libc_rand_s);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

#endif /* !__KERNEL__ */

DECL_END

#ifndef __INTELLISENSE__
#include "typecore.c.inl"
#endif

#endif /* !GUARD_LIBS_LIBC_STDLIB_C */
