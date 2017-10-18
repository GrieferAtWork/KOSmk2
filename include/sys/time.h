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
#ifndef _SYS_TIME_H
#define _SYS_TIME_H	1

#include <features.h>
#include <bits/types.h>
#include <sys/select.h>
#include <hybrid/timeval.h>

__SYSDECL_BEGIN

#ifndef __time_t_defined
#define __time_t_defined  1
typedef __TM_TYPE(time) time_t;
#endif /* !__time_t_defined */

#ifndef __suseconds_t_defined
#define __suseconds_t_defined
typedef __suseconds_t suseconds_t;
#endif

#ifdef __USE_GNU
#define TIMEVAL_TO_TIMESPEC(tv,ts) \
{   (ts)->tv_sec = (tv)->tv_sec; \
    (ts)->tv_nsec = (tv)->tv_usec * 1000; }
#define TIMESPEC_TO_TIMEVAL(tv,ts) \
{   (tv)->tv_sec = (ts)->tv_sec; \
    (tv)->tv_usec = (ts)->tv_nsec / 1000; }
#endif /* __USE_GNU */

#ifdef __USE_MISC
struct timezone {
    int tz_minuteswest; /*< Minutes west of GMT. */
    int tz_dsttime;     /*< Nonzero if DST is ever in effect. */
};
typedef struct timezone *__restrict __timezone_ptr_t;
#else /* __USE_MISC */
typedef void *__restrict __timezone_ptr_t;
#endif /* !__USE_MISC */

enum __itimer_which {
    ITIMER_REAL    = 0,
    ITIMER_VIRTUAL = 1,
    ITIMER_PROF    = 2
};
#define ITIMER_REAL    0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF    2

struct itimerval {
    struct timeval it_interval; /*< Value to put into `it_value' when the timer expires. */
    struct timeval it_value;    /*< Time to the next timer expiration. */
};

#ifdef __USE_KOS
#ifndef __USE_TIME_BITS64
#define itimerval32 itimerval
#else /* __USE_TIME_BITS64 */
struct itimerval32 {
    struct timeval32 it_interval; /*< Value to put into `it_value' when the timer expires. */
    struct timeval32 it_value;    /*< Time to the next timer expiration. */
};
#endif /* !__USE_TIME_BITS64 */
#endif /* __USE_KOS */
#ifdef __USE_TIME64
#ifdef __USE_TIME_BITS64
#define itimerval64 itimerval
#else /* __USE_TIME_BITS64 */
struct itimerval64 {
    struct timeval64 it_interval; /*< Value to put into `it_value' when the timer expires. */
    struct timeval64 it_value;    /*< Time to the next timer expiration. */
};
#endif /* !__USE_TIME_BITS64 */
#endif /* __USE_TIME64 */

#if defined(__USE_GNU) && !defined(__cplusplus)
typedef enum __itimer_which __itimer_which_t;
#else
typedef int __itimer_which_t;
#endif

#ifndef __KERNEL__
/* TODO: GLibc compatibility mode. */
__REDIRECT_TM_FUNC(__LIBC,__NONNULL((1)),int,__LIBCCALL,gettimeofday,(struct timeval *__restrict __tv, __timezone_ptr_t __tz),gettimeofday,(__tv,__tz))
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,getitimer,(__itimer_which_t __which, struct itimerval *__value),getitimer,(__which,__value))
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,setitimer,(__itimer_which_t __which, struct itimerval const *__restrict __new, struct itimerval *__restrict __old),setitimer,(__which,__new,__old))
__REDIRECT_TM_FUNC(__LIBC,__NONNULL((1)),int,__LIBCCALL,utimes,(char const *__file, struct timeval const __tvp[2]),utimes,(__file,__tvp))
#ifdef __USE_GNU
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,futimesat,(int __fd, char const *__file, struct timeval const __tvp[2]),futimesat,(__fd,__file,__tvp))
#endif /* __USE_GNU */
#ifdef __USE_MISC
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,settimeofday,(struct timeval const *__tv, struct timezone const *__tz),settimeofday,(__tv,__tz))
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,adjtime,(struct timeval const *__delta, struct timeval *__olddelta),adjtime,(__delta,__olddelta))
__REDIRECT_TM_FUNC(__LIBC,__NONNULL((1)),int,__LIBCCALL,lutimes,(char const *__file, struct timeval const __tvp[2]),lutimes,(__file,__tvp))
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,futimes,(int __fd, struct timeval const __tvp[2]),futimes,(__fd,__tvp))
#endif /* __USE_MISC */

#ifdef __USE_TIME64
__LIBC __NONNULL((1)) int (__LIBCCALL gettimeofday64)(struct timeval64 *__restrict __tv, __timezone_ptr_t __tz);
__LIBC int (__LIBCCALL getitimer64)(__itimer_which_t __which, struct itimerval64 *__value);
__LIBC int (__LIBCCALL setitimer64)(__itimer_which_t __which, const struct itimerval64 *__restrict __new, struct itimerval64 *__restrict __old);
__LIBC __NONNULL((1)) int (__LIBCCALL utimes64)(char const *__file, struct timeval64 const __tvp[2]);
#ifdef __USE_MISC
__LIBC int (__LIBCCALL settimeofday64)(struct timeval64 const *__tv, struct timezone const *__tz);
__LIBC int (__LIBCCALL adjtime64)(struct timeval64 const *__delta, struct timeval64 *__olddelta);
__LIBC __NONNULL((1)) int (__LIBCCALL lutimes64)(char const *__file, struct timeval64 const __tvp[2]);
__LIBC int (__LIBCCALL futimes64)(int __fd, struct timeval64 const __tvp[2]);
#endif /* __USE_MISC */
#ifdef __USE_GNU
__LIBC int (__LIBCCALL futimesat64)(int __fd, char const *__file, struct timeval64 const __tvp[2]);
#endif /* __USE_GNU */
#endif /* __USE_TIME64 */

#endif /* !__KERNEL__ */

#ifdef __USE_MISC
#define timerisset(tvp)    ((tvp)->tv_sec || (tvp)->tv_usec)
#define timerclear(tvp)    ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#define timercmp(a,b,CMP) \
  (((a)->tv_sec == (b)->tv_sec) \
 ? ((a)->tv_usec CMP (b)->tv_usec) \
 : ((a)->tv_sec  CMP (b)->tv_sec))
# define timeradd(a,b,result) \
do{ (result)->tv_sec  = (a)->tv_sec  + (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
    if ((result)->tv_usec >= 1000000) { \
      ++(result)->tv_sec; \
      (result)->tv_usec -= 1000000; \
    } \
}__WHILE0
# define timersub(a,b,result) \
do{ (result)->tv_sec  = (a)->tv_sec  - (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((result)->tv_usec < 0) { \
      --(result)->tv_sec; \
      (result)->tv_usec += 1000000; \
    } \
}__WHILE0
#endif /* __USE_MISC */

__SYSDECL_END

#endif /* !_SYS_TIME_H */
