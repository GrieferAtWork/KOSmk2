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
#ifndef _TIME_H
#define _TIME_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/time.h>
#include <bits/types.h>
#ifdef __USE_POSIX199309
#include <hybrid/timespec.h>
#endif /* __USE_POSIX199309 */
#ifdef __USE_XOPEN2K8
#include <xlocale.h>
#endif

__DECL_BEGIN

#ifdef __NAMESPACE_STD_EXISTS

__NAMESPACE_STD_BEGIN

#ifndef __std_size_t_defined
#define __std_size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__std_size_t_defined */

#ifndef __std_clock_t_defined
#define __std_clock_t_defined 1
typedef __clock_t clock_t;
#endif /* !__std_clock_t_defined */

#ifndef __std_time_t_defined
#define __std_time_t_defined 1
typedef __TM_TYPE(time) time_t;
#endif /* !__std_time_t_defined */

__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER

#ifndef __size_t_defined
#define __size_t_defined 1
__NAMESPACE_STD_USING(size_t)
#endif /* !__size_t_defined */

#ifndef __clock_t_defined
#define __clock_t_defined 1
__NAMESPACE_STD_USING(clock_t)
#endif /* !__clock_t_defined */

#ifndef __time_t_defined
#define __time_t_defined 1
__NAMESPACE_STD_USING(time_t)
#endif /* !__time_t_defined */

#endif /* !__CXX_SYSTEM_HEADER */

#else /* __NAMESPACE_STD_EXISTS */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef __clock_t_defined
#define __clock_t_defined 1
typedef __clock_t clock_t;
#endif /* !__clock_t_defined */

#ifndef __time_t_defined
#define __time_t_defined 1
typedef __TM_TYPE(time) time_t;
#endif /* !__time_t_defined */

#endif /* !__NAMESPACE_STD_EXISTS */

#ifndef NULL
#ifdef __INTELLISENSE__
#   define NULL nullptr
#elif defined(__cplusplus) || defined(__LINKER__)
#   define NULL          0
#else
#   define NULL ((void *)0)
#endif
#endif

#ifdef __USE_XOPEN2K
#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif
#else
#ifndef __STRICT_ANSI__
#ifndef CLK_TCK
#   define CLK_TCK CLOCKS_PER_SEC
#endif
#endif
#endif

#ifdef __USE_TIME64
#ifndef __time64_t_defined
#define __time64_t_defined 1
typedef __time64_t time64_t;
#endif /* !__time64_t_defined */
#endif /* __USE_TIME64 */

#ifdef __USE_POSIX199309
#ifndef __clockid_t_defined
#define __clockid_t_defined 1
typedef __clockid_t clockid_t;
#endif /* !__clockid_t_defined */
#ifndef __timer_t_defined
#define __timer_t_defined 1
typedef __timer_t timer_t;
#endif /* !__timer_t_defined */
#endif /* __USE_POSIX199309 */

/* Used by other time functions.  */
#ifndef __std_tm_defined
#define __std_tm_defined 1
__NAMESPACE_STD_BEGIN
struct tm {
 int         tm_sec;      /*< seconds [0,61]. */
 int         tm_min;      /*< minutes [0,59]. */
 int         tm_hour;     /*< hour [0,23]. */
 int         tm_mday;     /*< day of month [1,31]. */
 int         tm_mon;      /*< month of year [0,11]. */
 int         tm_year;     /*< years since 1900. */
 int         tm_wday;     /*< day of week [0,6] (Sunday = 0). */
 int         tm_yday;     /*< day of year [0,365]. */
 int         tm_isdst;    /*< daylight savings flag. */
#ifndef __USE_DOS
#ifdef __USE_MISC
 long int    tm_gmtoff;   /* Seconds east of UTC.  */
 char const *tm_zone;     /* Timezone abbreviation.  */
#else
 long int    __tm_gmtoff; /* Seconds east of UTC.  */
 char const *__tm_zone;   /* Timezone abbreviation.  */
#endif
#endif /* !__USE_DOS */
};
__NAMESPACE_STD_END
#endif /* !__std_tm_defined */
#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */

#ifdef __USE_POSIX199309
struct itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};
#ifdef __USE_KOS
#ifndef __USE_TIME_BITS64
#define itimerspec32 itimerspec
#else /* __USE_TIME_BITS64 */
struct itimerspec32 {
    struct __timespec32 it_interval;
    struct __timespec32 it_value;
};
#endif /* !__USE_TIME_BITS64 */
#endif /* __USE_KOS */
#ifdef __USE_TIME64
#ifdef __USE_TIME_BITS64
#define itimerspec64 itimerspec
#else /* __USE_TIME_BITS64 */
struct itimerspec64 {
    struct __timespec64 it_interval;
    struct __timespec64 it_value;
};
#endif /* !__USE_TIME_BITS64 */
#endif /* __USE_TIME64 */
#endif /* __USE_POSIX199309 */

#ifdef __USE_KOS
#   define MSEC_PER_SEC    1000l
#   define USEC_PER_MSEC   1000l
#   define NSEC_PER_USEC   1000l
#   define NSEC_PER_MSEC   1000000l
#   define USEC_PER_SEC    1000000l
#   define NSEC_PER_SEC    1000000000l
#   define FSEC_PER_SEC    1000000000000000ll
#endif

#ifdef __USE_ISOC11
#define TIME_UTC 1
#endif

#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC clock_t (__LIBCCALL clock)(void) __DOS_FUNC(clock);
#ifdef __PE__
#ifdef __USE_TIME_BITS64
__LIBC time_t (__LIBCCALL time)(time_t *__timer) __ASMNAME("_time64");
#else /* __USE_TIME_BITS64 */
__LIBC time_t (__LIBCCALL time)(time_t *__timer) __ASMNAME("_time32");
#endif  /* !__USE_TIME_BITS64 */
#else /* __PE__ */
__LIBC time_t (__LIBCCALL time)(time_t *__timer) __TM_FUNC(time);
#endif /* !__PE__ */
__LIBC __ATTR_CONST double (__LIBCCALL difftime)(time_t __time1, time_t __time0) __TM_FUNC(difftime);
__LIBC time_t (__LIBCCALL mktime)(struct tm *__tp) __TM_FUNC(mktime);
__LIBC size_t (__LIBCCALL strftime)(char *__restrict __s, size_t __maxsize, char const *__restrict __format, struct tm const *__restrict __tp);
__LIBC char *(__LIBCCALL asctime)(struct tm const *__tp);
__LIBC char *(__LIBCCALL ctime)(time_t const *__timer) __TM_FUNC(ctime);
#ifdef __USE_TIME64
#ifdef __PE__
__LIBC time64_t (__LIBCCALL time64)(time64_t *__timer) __ASMNAME("_time64");
#else /* __PE__ */
__LIBC time64_t (__LIBCCALL time64)(time64_t *__timer);
#endif /* !__PE__ */
__LIBC __ATTR_CONST double (__LIBCCALL difftime64)(time64_t __time1, time64_t __time0);
__LIBC time64_t (__LIBCCALL mktime64)(struct tm *__tp);
__LIBC char *(__LIBCCALL ctime64)(time64_t const *__timer);
#endif /* __USE_TIME64 */
__NAMESPACE_STD_END

__NAMESPACE_STD_USING(clock)
__NAMESPACE_STD_USING(time)
__NAMESPACE_STD_USING(difftime)
__NAMESPACE_STD_USING(mktime)
__NAMESPACE_STD_USING(strftime)
__NAMESPACE_STD_USING(asctime)
__NAMESPACE_STD_USING(ctime)
#ifdef __USE_TIME64
__NAMESPACE_STD_USING(time64)
__NAMESPACE_STD_USING(difftime64)
__NAMESPACE_STD_USING(mktime64)
__NAMESPACE_STD_USING(ctime64)
#endif /* __USE_TIME64 */


#undef __tzname
#undef __daylight
#undef __timezone
__LIBC char *(__tzname)[2] __ASMNAME("tzname");
__LIBC int (__daylight) __ASMNAME("daylight");
__LIBC long int (__timezone) __ASMNAME("timezone");


#ifdef __USE_XOPEN
__LIBC char *(__LIBCCALL strptime)(char const *__restrict __s, char const *__restrict __format, struct tm *__tp);;
#endif /* __USE_XOPEN */

#ifdef __USE_XOPEN2K8
__LIBC size_t (__LIBCCALL strftime_l)(char *__restrict __s, size_t __maxsize, char const *__restrict __format, struct tm const *__restrict __tp, __locale_t __loc);
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_GNU
__LIBC char *(__LIBCCALL strptime_l)(char const *__restrict __s, char const *__restrict __format, struct tm *__tp, __locale_t __loc);
__LIBC int (__LIBCCALL getdate_r)(char const *__restrict __string, struct tm *__restrict __resbufp);
#endif /* __USE_GNU */

__NAMESPACE_STD_BEGIN
__LIBC struct tm *(__LIBCCALL gmtime)(time_t const *__timer) __TM_FUNC(gmtime);
__LIBC struct tm *(__LIBCCALL localtime)(time_t const *__timer) __TM_FUNC(localtime);
#ifdef __USE_TIME64
__LIBC struct tm *(__LIBCCALL gmtime64)(time64_t const *__timer);
__LIBC struct tm *(__LIBCCALL localtime64)(time64_t const *__timer);
#endif /* __USE_TIME64 */
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(gmtime)
__NAMESPACE_STD_USING(localtime)
#ifdef __USE_TIME64
__NAMESPACE_STD_USING(gmtime64)
__NAMESPACE_STD_USING(localtime64)
#endif /* __USE_TIME64 */

#ifdef __USE_POSIX
__LIBC struct tm *(__LIBCCALL gmtime_r)(time_t const *__restrict __timer, struct tm *__restrict __tp) __TM_FUNC_R(gmtime);
__LIBC struct tm *(__LIBCCALL localtime_r)(time_t const *__restrict __timer, struct tm *__restrict __tp) __TM_FUNC_R(localtime);
__LIBC char *(__LIBCCALL ctime_r)(time_t const *__restrict __timer, char *__restrict __buf) __TM_FUNC_R(ctime);
__LIBC char *(__LIBCCALL asctime_r)(struct tm const *__restrict __tp, char *__restrict __buf);
#ifdef __USE_TIME64
__LIBC struct tm *(__LIBCCALL gmtime64_r)(time64_t const *__restrict __timer, struct tm *__restrict __tp);
__LIBC struct tm *(__LIBCCALL localtime64_r)(time64_t const *__restrict __timer, struct tm *__restrict __tp);
__LIBC char *(__LIBCCALL ctime64_r)(time64_t const *__restrict __timer, char *__restrict __buf);
#endif /* __USE_TIME64 */

#undef tzname
__LIBC char *(tzname)[2];
__LIBC void (__LIBCCALL tzset)(void);
#endif /* __USE_POSIX */

#if defined(__USE_MISC) || defined(__USE_XOPEN)
#undef daylight
#undef timezone
__LIBC int (daylight);
__LIBC long int (timezone);
#endif /* __USE_MISC || __USE_XOPEN */

#ifdef __USE_MISC
__LIBC int (__LIBCCALL stime)(time_t const *__when) __TM_FUNC(stime);
__LIBC time_t (__LIBCCALL timegm)(struct tm *__tp) __TM_FUNC(timegm);
__LIBC time_t (__LIBCCALL timelocal)(struct tm *__tp) __TM_FUNC(timelocal);
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL stime64)(time64_t const *__when);
__LIBC time64_t (__LIBCCALL timegm64)(struct tm *__tp);
__LIBC time64_t (__LIBCCALL timelocal64)(struct tm *__tp);
#endif /* __USE_TIME64 */
__LIBC __ATTR_CONST int (__LIBCCALL dysize)(int __year);
#endif /* __USE_MISC */

#ifdef __USE_POSIX199309
__LIBC int (__LIBCCALL nanosleep)(struct timespec const *__requested_time, struct timespec *__remaining) __TM_FUNC(nanosleep);
__LIBC int (__LIBCCALL clock_getres)(clockid_t __clock_id, struct timespec *__res) __TM_FUNC(clock_getres);
__LIBC int (__LIBCCALL clock_gettime)(clockid_t __clock_id, struct timespec *__tp) __TM_FUNC(clock_gettime);
__LIBC int (__LIBCCALL clock_settime)(clockid_t __clock_id, struct timespec const *__tp) __TM_FUNC(clock_settime);
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL nanosleep64)(struct timespec64 const *__requested_time, struct timespec64 *__remaining);
__LIBC int (__LIBCCALL clock_getres64)(clockid_t __clock_id, struct timespec64 *__res);
__LIBC int (__LIBCCALL clock_gettime64)(clockid_t __clock_id, struct timespec64 *__tp);
__LIBC int (__LIBCCALL clock_settime64)(clockid_t __clock_id, struct timespec64 const *__tp);
#endif /* __USE_TIME64 */
struct sigevent;
__LIBC int (__LIBCCALL timer_create)(clockid_t __clock_id, struct sigevent *__restrict __evp, timer_t *__restrict __timerid);
__LIBC int (__LIBCCALL timer_delete)(timer_t __timerid);
__LIBC int (__LIBCCALL timer_settime)(timer_t __timerid, int __flags, struct itimerspec const *__restrict __value, struct itimerspec *__restrict __ovalue) __TM_FUNC(timer_settime);
__LIBC int (__LIBCCALL timer_gettime)(timer_t __timerid, struct itimerspec *__value) __TM_FUNC(timer_gettime);
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL timer_settime64)(timer_t __timerid, int __flags, struct itimerspec64 const *__restrict __value, struct itimerspec64 *__restrict __ovalue);
__LIBC int (__LIBCCALL timer_gettime64)(timer_t __timerid, struct itimerspec64 *__value);
#endif /* __USE_TIME64 */
__LIBC int (__LIBCCALL timer_getoverrun)(timer_t __timerid);
#ifdef __USE_XOPEN2K
__LIBC int (__LIBCCALL clock_nanosleep)(clockid_t __clock_id, int __flags, struct timespec const *__req, struct timespec *__rem) __TM_FUNC(clock_nanosleep);
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL clock_nanosleep64)(clockid_t __clock_id, int __flags, struct timespec64 const *__req, struct timespec64 *__rem);
#endif /* __USE_TIME64 */
__LIBC int (__LIBCCALL clock_getcpuclockid)(pid_t __pid, clockid_t *__clock_id);
#endif /* __USE_XOPEN2K */
#endif /* __USE_POSIX199309 */
#ifdef __USE_ISOC11
__LIBC __NONNULL((1)) int (__LIBCCALL timespec_get)(struct timespec *__ts, int __base) __TM_FUNC(timespec_get);
#ifdef __USE_TIME64
__LIBC __NONNULL((1)) int (__LIBCCALL timespec_get64)(struct timespec64 *__ts, int __base);
#endif /* __USE_TIME64 */
#endif /* __USE_ISOC11 */
#ifdef __USE_XOPEN_EXTENDED
#undef getdate_err
__LIBC int (getdate_err);
__LIBC struct tm *(__LIBCCALL getdate)(char const *__string);
#endif /* __USE_XOPEN_EXTENDED */

#else /* !__KERNEL__ */

#ifdef __USE_MISC
#define dysize(year) (__isleap(year) ? 366 : 365)
#endif /* __USE_MISC */

#endif /* __KERNEL__ */

#define __isleap(year) ((year)%4 == 0 && ((year)%100 != 0 || (year)%400 == 0))
#define __daystoyears(n_days)  ((400*((n_days)+1))/146097)
#define __yearstodays(n_years) (((146097*(n_years))/400)/*-1*/) /* rounding error? */


__DECL_END

#endif /* !_TIME_H */
