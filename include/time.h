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

__SYSDECL_BEGIN

__NAMESPACE_STD_BEGIN
#ifndef __std_size_t_defined
#define __std_size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__std_size_t_defined */
#ifndef __std_clock_t_defined
#define __std_clock_t_defined 1
typedef __typedef_clock_t clock_t;
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

#ifndef NULL
#define NULL __NULLPTR
#endif

#ifdef __USE_XOPEN2K
#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif
#endif
#if defined(__USE_DOS) || \
  (!defined(__USE_XOPEN2K) && !defined(__STRICT_ANSI__))
#ifndef CLK_TCK
#   define CLK_TCK CLOCKS_PER_SEC
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
#if !defined(__DOS_COMPAT__) && defined(__CRT_GLC)
#if defined(__USE_MISC) && !defined(__USE_DOS)
    long int    tm_gmtoff;   /*< Seconds east of UTC. */
    char const *tm_zone;     /*< Timezone abbreviation. */
#else
    long int    __tm_gmtoff; /*< Seconds east of UTC. */
    char const *__tm_zone;   /*< Timezone abbreviation. */
#endif
#endif /* !... */
};
__NAMESPACE_STD_END
#endif /* !__std_tm_defined */

#ifndef __CXX_SYSTEM_HEADER
#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */
#endif /* !__CXX_SYSTEM_HEADER */

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

#define __isleap(year) ((year)%4 == 0 && ((year)%100 != 0 || (year)%400 == 0))
#define __daystoyears(n_days)  ((400*((n_days)+1))/146097)
#define __yearstodays(n_years) (((146097*(n_years))/400)/*-1*/) /* rounding error? */

#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL,char **,__LIBCCALL,__dos_tzname,(void),__tzname,())
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL,int *,__LIBCCALL,__dos_daylight,(void),__daylight,())
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL,long *,__LIBCCALL,__dos_timezone,(void),__timezone,())
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL,long *,__LIBCCALL,__dos_dstbias,(void),__dstbias,())
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_gmtime_s,(struct tm *__restrict __tp, time_t const *__restrict __timer),_gmtime64_s,(__tp,__timer))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_localtime_s,(struct tm *__restrict __tp, time_t const *__restrict __timer),_localtime64_s,(__tp,__timer))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_ctime_s,(char *__buf, size_t __bufsize, time_t const *__restrict __timer),_ctime64_s,(__buf,__bufsize,__timer))
#define __dos_gmtime64_s(tp,timer)         __dos_gmtime_s(tp,timer)
#define __dos_localtime64_s(tp,timer)      __dos_localtime_s(tp,timer)
#define __dos_ctime64_s(buf,bufsize,timer) __dos_ctime_s(buf,bufsize,timer)
#else /* __USE_TIME_BITS64 */
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_gmtime_s,(struct tm *__restrict __tp, time_t const *__restrict __timer),_gmtime32_s,(__tp,__timer))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_localtime_s,(struct tm *__restrict __tp, time_t const *__restrict __timer),_localtime32_s,(__tp,__timer))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_ctime_s,(char *__buf, size_t __bufsize, time_t const *__restrict __timer),_ctime32_s,(__buf,__bufsize,__timer))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_gmtime64_s,(struct tm *__restrict __tp, time64_t const *__restrict __timer),_gmtime64_s,(__tp,__timer))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_localtime64_s,(struct tm *__restrict __tp, time64_t const *__restrict __timer),_localtime64_s,(__tp,__timer))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_ctime64_s,(char *__buf, size_t __bufsize, __time64_t const *__restrict __timer),_ctime64_s,(__buf,__bufsize,__timer))
#endif /* !__USE_TIME_BITS64 */
#endif /* __DOS_COMPAT__ */

#ifdef __USE_KOS
#define __FIXED_CONST const
#else
#define __FIXED_CONST /* Nothing */
#endif

#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__REDIRECT_DOS_FUNC(__LIBC,__WUNUSED,clock_t,__LIBCCALL,clock,(void),clock,())
#if (defined(__PE__) || defined(__DOS_COMPAT__)) && defined(__CRT_DOS)
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,,time_t,__LIBCCALL,time,(time_t *__timer),_time64,(__timer))
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST,double,__LIBCCALL,difftime,(time_t __time1, time_t __time0),_difftime64,(__time1,__time0))
__REDIRECT(__LIBC,__WUNUSED ,time_t,__LIBCCALL,mktime,(struct tm __FIXED_CONST *__tp),_mktime64,(__tp))
__REDIRECT(__LIBC,__WUNUSED,char *,__LIBCCALL,ctime,(time_t const *__timer),_ctime64,(__timer))
__REDIRECT(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,gmtime,(time_t const *__timer),_gmtime64,(__timer))
__REDIRECT(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,localtime,(time_t const *__timer),_localtime64,(__timer))
#else /* __USE_TIME_BITS64 */
__REDIRECT(__LIBC,,time_t,__LIBCCALL,time,(time_t *__timer),_time32,(__timer))
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST,double,__LIBCCALL,difftime,(time_t __time1, time_t __time0),_difftime32,(__time1,__time0))
__REDIRECT(__LIBC,__WUNUSED ,time_t,__LIBCCALL,mktime,(struct tm __FIXED_CONST *__tp),_mktime32,(__tp))
__REDIRECT(__LIBC,__WUNUSED,char *,__LIBCCALL,ctime,(time_t const *__timer),_ctime32,(__timer))
__REDIRECT(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,gmtime,(time_t const *__timer),_gmtime32,(__timer))
__REDIRECT(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,localtime,(time_t const *__timer),_localtime32,(__timer))
#endif  /* !__USE_TIME_BITS64 */
#elif (defined(__GLC_COMPAT__) || !defined(__CRT_KOS)) && \
       defined(__USE_TIME_BITS64)
__REDIRECT(__LIBC,,__time32_t,__LIBCCALL,__time32,(__time32_t *__timer),time,(__timer))
__REDIRECT(__LIBC,__ATTR_CONST,double,__LIBCCALL,__difftime32,(__time32_t __time1, __time32_t __time0),difftime,(__time1,__time0))
__REDIRECT(__LIBC,__WUNUSED ,__time32_t,__LIBCCALL,__mktime32,(struct tm __FIXED_CONST *__tp),mktime,(__tp))
__REDIRECT(__LIBC,__WUNUSED,char *,__LIBCCALL,__ctime32,(__time32_t const *__timer),ctime,(__timer))
__REDIRECT(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,__gmtime32,(time_t const *__timer),gmtime,(__timer))
__REDIRECT(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,__localtime32,(time_t const *__timer),localtime,(__timer))
__LOCAL time_t (__LIBCCALL time)(time_t *__timer) { __time32_t __t = __time32(0); if (__timer) *__timer = (time_t)__t; return (time_t)__t; }
__LOCAL __WUNUSED __ATTR_CONST double (__LIBCCALL difftime)(time_t __time1, time_t __time0) { return __difftime32((__time32_t)__time1,(__time32_t)__time0); }
__LOCAL __WUNUSED time_t (__LIBCCALL mktime)(struct tm __FIXED_CONST *__tp) { return (__time_t)__mktime32(__tp); }
__LOCAL __WUNUSED char *(__LIBCCALL ctime)(time_t const *__timer) { __time32_t __t = (__time32_t)*__timer; return __ctime32(&__t); }
__LOCAL __WUNUSED struct tm *(__LIBCCALL gmtime)(time_t const *__timer) { __time32_t __t = (__time32_t)*__timer; return __gmtime32(&__t); }
__LOCAL __WUNUSED struct tm *(__LIBCCALL localtime)(time_t const *__timer) { __time32_t __t = (__time32_t)*__timer; return __localtime32(&__t); }
#else /* Compat... */
__REDIRECT_TM_FUNC(__LIBC,,time_t,__LIBCCALL,time,(time_t *__timer),time,(__timer))
__REDIRECT_TM_FUNC(__LIBC,__WUNUSED __ATTR_CONST,double,__LIBCCALL,difftime,(time_t __time1, time_t __time0),difftime,(__time1,__time0))
__REDIRECT_TM_FUNC(__LIBC,__WUNUSED ,time_t,__LIBCCALL,mktime,(struct tm __FIXED_CONST *__tp),mktime,(__tp))
__REDIRECT_TM_FUNC(__LIBC,__WUNUSED,char *,__LIBCCALL,ctime,(time_t const *__timer),ctime,(__timer))
__REDIRECT_TM_FUNC(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,gmtime,(time_t const *__timer),gmtime,(__timer))
__REDIRECT_TM_FUNC(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,localtime,(time_t const *__timer),localtime,(__timer))
#endif /* Builtin... */
__LIBC size_t (__LIBCCALL strftime)(char *__restrict __s, size_t __maxsize, char const *__restrict __format, struct tm const *__restrict __tp);
__LIBC __WUNUSED char *(__LIBCCALL asctime)(struct tm const *__tp);
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(clock)
__NAMESPACE_STD_USING(time)
__NAMESPACE_STD_USING(difftime)
__NAMESPACE_STD_USING(mktime)
__NAMESPACE_STD_USING(strftime)
__NAMESPACE_STD_USING(asctime)
__NAMESPACE_STD_USING(ctime)
__NAMESPACE_STD_USING(gmtime)
__NAMESPACE_STD_USING(localtime)
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __USE_TIME64
#if (defined(__PE__) || defined(__DOS_COMPAT__)) && defined(__CRT_DOS)
__REDIRECT(__LIBC,,time64_t,__LIBCCALL,time64,(time64_t *__timer),_time64,(__timer))
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST,double,__LIBCCALL,difftime64,(time64_t __time1, time64_t __time0),_difftime64,(__time1,__time0))
__REDIRECT(__LIBC,__WUNUSED ,time64_t,__LIBCCALL,mktime64,(struct tm __FIXED_CONST *__tp),_mktime64,(__tp))
__REDIRECT(__LIBC,__WUNUSED,char *,__LIBCCALL,ctime64,(time64_t const *__timer),_ctime64,(__timer))
__REDIRECT(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,gmtime64,(time64_t const *__timer),_gmtime64,(__timer))
__REDIRECT(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,localtime64,(time64_t const *__timer),_localtime64,(__timer))
#elif defined(__GLC_COMPAT__) || !defined(__CRT_KOS)
#ifdef __USE_TIME_BITS64
__LOCAL time64_t (__LIBCCALL time64)(time64_t *__timer) { __time32_t __t = __NAMESPACE_STD_SYM __time32(0); if (__timer) *__timer = (time64_t)__t; return (time64_t)__t; }
__LOCAL __WUNUSED __ATTR_CONST double (__LIBCCALL difftime64)(time64_t __time1, time64_t __time0) { return __NAMESPACE_STD_SYM __difftime32((__time32_t)__time1,(__time32_t)__time0); }
__LOCAL __WUNUSED time64_t (__LIBCCALL mktime64)(struct tm __FIXED_CONST *__tp) { return (__time64_t)__NAMESPACE_STD_SYM __mktime32(__tp); }
__LOCAL __WUNUSED char *(__LIBCCALL ctime64)(time64_t const *__timer) { __time32_t __t = (__time32_t)*__timer; return __NAMESPACE_STD_SYM __ctime32(&__t); }
__LOCAL __WUNUSED struct tm *(__LIBCCALL gmtime64)(time64_t const *__timer) { __time32_t __t = (__time32_t)*__timer; return __NAMESPACE_STD_SYM __gmtime32(&__t); }
__LOCAL __WUNUSED struct tm *(__LIBCCALL localtime64)(time64_t const *__timer) { __time32_t __t = (__time32_t)*__timer; return __NAMESPACE_STD_SYM __localtime32(&__t); }
#else /* __USE_TIME_BITS64 */
__LOCAL time64_t (__LIBCCALL time64)(time64_t *__timer) { __time32_t __t = __NAMESPACE_STD_SYM time(0); if (__timer) *__timer = (time64_t)__t; return (time64_t)__t; }
__LOCAL __WUNUSED __ATTR_CONST double (__LIBCCALL difftime64)(time64_t __time1, time64_t __time0) { return __NAMESPACE_STD_SYM difftime((__time32_t)__time1,(__time32_t)__time0); }
__LOCAL __WUNUSED time64_t (__LIBCCALL mktime64)(struct tm __FIXED_CONST *__tp) { return (__time64_t)__NAMESPACE_STD_SYM mktime(__tp); }
__LOCAL __WUNUSED char *(__LIBCCALL ctime64)(time64_t const *__timer) { __time32_t __t = (__time32_t)*__timer; return __NAMESPACE_STD_SYM ctime(&__t); }
__LOCAL __WUNUSED struct tm *(__LIBCCALL gmtime64)(time64_t const *__timer) { __time32_t __t = (__time32_t)*__timer; return __NAMESPACE_STD_SYM gmtime(&__t); }
__LOCAL __WUNUSED struct tm *(__LIBCCALL localtime64)(time64_t const *__timer) { __time32_t __t = (__time32_t)*__timer; return __NAMESPACE_STD_SYM localtime(&__t); }
#endif /* !__USE_TIME_BITS64 */
#else /* Compat... */
__LIBC time64_t (__LIBCCALL time64)(time64_t *__timer);
__LIBC __WUNUSED __ATTR_CONST double (__LIBCCALL difftime64)(time64_t __time1, time64_t __time0);
__LIBC __WUNUSED time64_t (__LIBCCALL mktime64)(struct tm __FIXED_CONST *__tp);
__LIBC __WUNUSED char *(__LIBCCALL ctime64)(time64_t const *__timer);
__LIBC __WUNUSED struct tm *(__LIBCCALL gmtime64)(time64_t const *__timer);
__LIBC __WUNUSED struct tm *(__LIBCCALL localtime64)(time64_t const *__timer);
#endif /* Builtin... */
#endif /* __USE_TIME64 */


#ifdef __DOS_COMPAT__
#undef __tzname
#undef __daylight
#undef __timezone
#define __tzname     __dos_tzname()
#define __daylight (*__dos_daylight())
#define __timezone (*__dos_timezone())
#else /* __DOS_COMPAT__ */
#undef __tzname
#undef __daylight
#undef __timezone
#ifdef __NO_ASMNAME
#define __tzname     tzname
#define __daylight   daylight
#define __timezone   timezone
#else /* __NO_ASMNAME */
__LIBC char *(__tzname)[2] __ASMNAME("tzname");
__LIBC int (__daylight) __ASMNAME("daylight");
__LIBC long int (__timezone) __ASMNAME("timezone");
#endif /* !__NO_ASMNAME */
#endif /* !__DOS_COMPAT__ */

#ifdef __USE_XOPEN2K8
__REDIRECT_IFDOS(__LIBC,,size_t,__LIBCCALL,strftime_l,
                (char *__restrict __s, size_t __maxsize, char const *__restrict __format, struct tm const *__restrict __tp, __locale_t __loc),
                 _strftime_l,(__s,__maxsize,__format,__tp,__loc))
#endif /* __USE_XOPEN2K8 */

#ifdef __CRT_GLC
#ifdef __USE_XOPEN
__LIBC __PORT_NODOS char *(__LIBCCALL strptime)(char const *__restrict __s, char const *__restrict __format, struct tm *__tp);
#endif /* __USE_XOPEN */
#ifdef __USE_GNU
__LIBC __PORT_NODOS char *(__LIBCCALL strptime_l)(char const *__restrict __s, char const *__restrict __format, struct tm *__tp, __locale_t __loc);
__LIBC __PORT_NODOS int (__LIBCCALL getdate_r)(char const *__restrict __string, struct tm *__restrict __resbufp);
#endif /* __USE_GNU */
#endif /* __CRT_GLC */

#ifdef __USE_POSIX
#ifdef __DOS_COMPAT__
__LOCAL struct tm *(__LIBCCALL gmtime_r)(time_t const *__restrict __timer, struct tm *__restrict __tp) { return __dos_gmtime_s(__tp,__timer) ? 0 : __tp; }
__LOCAL struct tm *(__LIBCCALL localtime_r)(time_t const *__restrict __timer, struct tm *__restrict __tp) { return __dos_localtime_s(__tp,__timer) ? 0 : __tp; }
__LOCAL char *(__LIBCCALL ctime_r)(time_t const *__restrict __timer, char *__restrict __buf) { return __dos_ctime_s(__buf,26,__timer) ? 0 : __buf; }
#elif defined(__GLC_COMPAT__) && defined(__USE_TIME_BITS64)
#ifndef ____ctime32_r_defined
#define ____ctime32_r_defined 1
__REDIRECT(__LIBC,,struct tm *,__LIBCCALL,__gmtime32_r,(__time32_t const *__restrict __timer, struct tm *__restrict __tp),gmtime_r,(__timer,__tp))
__REDIRECT(__LIBC,,struct tm *,__LIBCCALL,__localtime32_r,(__time32_t const *__restrict __timer, struct tm *__restrict __tp),localtime_r,(__timer,__tp))
__REDIRECT(__LIBC,,char *,__LIBCCALL,__ctime32_r,(__time32_t const *__restrict __timer, char *__restrict __buf),ctime_r,(__timer,__buf))
#endif /* !____ctime32_r_defined */
__LOCAL struct tm *(__LIBCCALL gmtime_r)(time_t const *__restrict __timer, struct tm *__restrict __tp) { __time32_t __t = (__time32_t)*__timer; return __gmtime32_r(&__t,__tp); }
__LOCAL struct tm *(__LIBCCALL localtime_r)(time_t const *__restrict __timer, struct tm *__restrict __tp) { __time32_t __t = (__time32_t)*__timer; return __localtime32_r(&__t,__tp); }
__LOCAL char *(__LIBCCALL ctime_r)(time_t const *__restrict __timer, char *__restrict __buf) { __time32_t __t = (__time32_t)*__timer; return __ctime32_r(&__t,__buf); }
#else /* Compat... */
__REDIRECT_TM_FUNC_R(__LIBC,,struct tm *,__LIBCCALL,gmtime_r,(time_t const *__restrict __timer, struct tm *__restrict __tp),gmtime,(__timer,__tp))
__REDIRECT_TM_FUNC_R(__LIBC,,struct tm *,__LIBCCALL,localtime_r,(time_t const *__restrict __timer, struct tm *__restrict __tp),localtime,(__timer,__tp))
__REDIRECT_TM_FUNC_R(__LIBC,,char *,__LIBCCALL,ctime_r,(time_t const *__restrict __timer, char *__restrict __buf),ctime,(__timer,__buf))
#endif /* Builtin... */

#ifdef __USE_TIME64
#ifdef __DOS_COMPAT__
__LOCAL struct tm *(__LIBCCALL gmtime64_r)(time_t const *__restrict __timer, struct tm *__restrict __tp) { return __dos_gmtime64_s(__tp,__timer) ? 0 : __tp; }
__LOCAL struct tm *(__LIBCCALL localtime64_r)(time_t const *__restrict __timer, struct tm *__restrict __tp) { return __dos_localtime64_s(__tp,__timer) ? 0 : __tp; }
__LOCAL char *(__LIBCCALL ctime64_r)(time64_t const *__restrict __timer, char *__restrict __buf) { return __dos_ctime64_s(__buf,26,__timer) ? 0 : __buf; }
#elif defined(__GLC_COMPAT__)
#ifdef __USE_TIME_BITS64
__LOCAL struct tm *(__LIBCCALL gmtime64_r)(time64_t const *__restrict __timer, struct tm *__restrict __tp) { return gmtime_r(__timer,__tp); }
__LOCAL struct tm *(__LIBCCALL localtime64_r)(time64_t const *__restrict __timer, struct tm *__restrict __tp) { return localtime_r(__timer,__tp); }
__LOCAL char *(__LIBCCALL ctime64_r)(time64_t const *__restrict __timer, char *__restrict __buf) { return ctime_r(__timer,__buf); }
#else /* __USE_TIME_BITS64 */
__LOCAL struct tm *(__LIBCCALL gmtime64_r)(time64_t const *__restrict __timer, struct tm *__restrict __tp) { __time32_t __t = (__time32_t)*__timer_t; return gmtime_r(&__t,__tp); }
__LOCAL struct tm *(__LIBCCALL localtime64_r)(time64_t const *__restrict __timer, struct tm *__restrict __tp) { __time32_t __t = (__time32_t)*__timer_t; return localtime_r(&__t,__tp); }
__LOCAL char *(__LIBCCALL ctime64_r)(time64_t const *__restrict __timer, char *__restrict __buf) { __time32_t __t = (__time32_t)*__timer_t; return ctime_r(&__t,__buf); }
#endif /* !__USE_TIME_BITS64 */
#else /* Compat... */
__LIBC struct tm *(__LIBCCALL gmtime64_r)(time64_t const *__restrict __timer, struct tm *__restrict __tp);
__LIBC struct tm *(__LIBCCALL localtime64_r)(time64_t const *__restrict __timer, struct tm *__restrict __tp);
__LIBC char *(__LIBCCALL ctime64_r)(time64_t const *__restrict __timer, char *__restrict __buf);
#endif /* Builtin... */
#endif /* __USE_TIME64 */

#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_asctime_s,(char *__buf, size_t __bufsize, struct tm const *__restrict __tp),asctime_s,(__buf,__bufsize,__tp))
__LOCAL char *(__LIBCCALL asctime_r)(struct tm const *__restrict __tp, char *__restrict __buf) { return __dos_asctime_s(__buf,26,__tp) ? 0 : __buf; }
#else /* __DOS_COMPAT__ */
__LIBC char *(__LIBCCALL asctime_r)(struct tm const *__restrict __tp, char *__restrict __buf);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_POSIX */

#if defined(__USE_POSIX) || defined(__USE_DOS)
#undef tzname
__REDIRECT_IFDOS_VOID(__LIBC,,__LIBCCALL,tzset,(void),_tzset,())
#endif /* __USE_POSIX || __USE_DOS */

#if defined(__USE_POSIX) || defined(__USE_DOS) || \
  (!defined(__DOS_COMPAT__) && defined(__NO_ASMNAME))
#ifdef __DOS_COMPAT__
#define tzname     __dos_tzname()
#else /* __DOS_COMPAT__ */
__LIBC char *tzname[2];
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_POSIX || __USE_DOS */

#if defined(__USE_MISC) || defined(__USE_XOPEN) || defined(__USE_DOS) || \
  (!defined(__DOS_COMPAT__) && defined(__NO_ASMNAME))
#undef daylight
#undef timezone
#ifdef __DOS_COMPAT__
#define daylight   (*__dos_daylight())
#define timezone   (*__dos_timezone())
#else /* __DOS_COMPAT__ */
__LIBC int daylight;
__LIBC long int timezone;
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_MISC || __USE_XOPEN || __USE_DOS */


#ifdef __USE_MISC
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,,unsigned int,__LIBCCALL,__dos_setsystime,(struct tm __FIXED_CONST *__tp, unsigned int __msec),_setsystime,(__tp,__msec))
__LOCAL int (__LIBCCALL stime)(time_t const *__when) { struct tm __temp; return (__dos_localtime_s(&__temp,__when) == 0 && __dos_setsystime(&__temp,0) == 0) ? 0 : -1; }
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,__WUNUSED,time_t,__LIBCCALL,timegm,(struct tm __FIXED_CONST *__tp),_mkgmtime64,(__tp))
__REDIRECT(__LIBC,__WUNUSED,time_t,__LIBCCALL,timelocal,(struct tm __FIXED_CONST *__tp),_mktime64,(__tp))
#else /* __USE_TIME_BITS64 */
__REDIRECT(__LIBC,__WUNUSED,time_t,__LIBCCALL,timegm,(struct tm __FIXED_CONST *__tp),_mkgmtime32,(__tp))
__REDIRECT(__LIBC,__WUNUSED,time_t,__LIBCCALL,timelocal,(struct tm __FIXED_CONST *__tp),_mktime32,(__tp))
#endif /* !__USE_TIME_BITS64 */
#elif defined(__GLC_COMPAT__) && defined(__USE_TIME_BITS64)
__REDIRECT(__LIBC,,int,__LIBCCALL,__stime32,(__time32_t const *__when),stime,(__when))
__REDIRECT(__LIBC,__WUNUSED,time_t,__LIBCCALL,__timegm32,(struct tm __FIXED_CONST *__tp),timegm,(__tp))
__REDIRECT(__LIBC,__WUNUSED,time_t,__LIBCCALL,__timelocal32,(struct tm __FIXED_CONST *__tp),timelocal,(__tp))
__LOCAL int (__LIBCCALL stime)(time_t const *__when) { __time32_t __t = (__time32_t)*__when; return __stime32(&__t); }
__LOCAL __WUNUSED time_t (__LIBCCALL timegm)(struct tm __FIXED_CONST *__tp) { return (time_t)__timegm32(__tp); }
__LOCAL __WUNUSED time_t (__LIBCCALL timelocal)(struct tm __FIXED_CONST *__tp) { return (time_t)__timelocal32(__tp); }
#else /* Compat... */
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,stime,(time_t const *__when),stime,(__when))
__REDIRECT_TM_FUNC(__LIBC,__WUNUSED,time_t,__LIBCCALL,timegm,(struct tm __FIXED_CONST *__tp),timegm,(__tp))
__REDIRECT_TM_FUNC(__LIBC,__WUNUSED,time_t,__LIBCCALL,timelocal,(struct tm __FIXED_CONST *__tp),timelocal,(__tp))
#endif /* Builtin... */
#ifdef __USE_TIME64
#ifdef __DOS_COMPAT__
__LOCAL int (__LIBCCALL stime64)(time64_t const *__when) { struct tm __temp; return (__dos_localtime64_s(&__temp,__when) == 0 && __dos_setsystime(&__temp,0) == 0) ? 0 : -1; }
__REDIRECT(__LIBC,__WUNUSED,time64_t,__LIBCCALL,timegm64,(struct tm __FIXED_CONST *__tp),_mkgmtime64,(__tp))
__REDIRECT(__LIBC,__WUNUSED,time64_t,__LIBCCALL,timelocal64,(struct tm __FIXED_CONST *__tp),_mktime64,(__tp))
#elif defined(__GLC_COMPAT__)
#ifdef __USE_TIME_BITS64
__LOCAL int (__LIBCCALL stime64)(time64_t const *__when) { return stime(__when); }
#else /* __USE_TIME_BITS64 */
__LOCAL int (__LIBCCALL stime64)(time64_t const *__when) { __time32_t __t = (__time32_t)*__when; return stime(&__t); }
#endif /* !__USE_TIME_BITS64 */
__LOCAL time64_t (__LIBCCALL timegm64)(struct tm *__tp) { return (time64_t)timegm(__tp); }
__LOCAL time64_t (__LIBCCALL timelocal64)(struct tm *__tp) { return (time64_t)timelocal(__tp); }
#else /* Compat... */
__LIBC int (__LIBCCALL stime64)(time64_t const *__when);
__LIBC time64_t (__LIBCCALL timegm64)(struct tm __FIXED_CONST *__tp);
__LIBC time64_t (__LIBCCALL timelocal64)(struct tm __FIXED_CONST *__tp);
#endif /* Builtin... */
#endif /* __USE_TIME64 */

#ifdef __DOS_COMPAT__
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL dysize)(int __year) { return __isleap(__year) ? 366 : 365; }
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL dysize)(int __year);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_MISC */

#ifdef __USE_POSIX199309
#ifdef __DOS_COMPAT__
/* Hidden function exported by DOS that allows for millisecond precision. */
#ifndef ____dos_sleep_mili_defined
#define ____dos_sleep_mili_defined 1
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__dos_sleep_mili,(__UINT32_TYPE__ __msecs),__crtSleep,(__msecs))
#endif /* !____dos_sleep_mili_defined */
__LOCAL int (__LIBCCALL nanosleep)(struct timespec const *__requested_time, struct timespec *__remaining) {
 if (__requested_time) { __dos_sleep_mili((__UINT32_TYPE__)(__requested_time->tv_sec*1000l+__requested_time->tv_nsec/1000000l)); }
 if (__remaining) { __remaining->tv_sec = 0,__remaining->tv_nsec = 0; }
 return 0;
}
#ifdef __USE_TIME64
__LOCAL int (__LIBCCALL nanosleep64)(struct timespec64 const *__requested_time,
                                     struct timespec64 *__remaining) {
#ifdef __USE_TIME_BITS64
 return nanosleep(__requested_time,__remaining);
#else /* __USE_TIME_BITS64 */
 if (__requested_time) { __dos_sleep_mili((__UINT32_TYPE__)(__requested_time->tv_sec*1000l+__requested_time->tv_nsec/1000000l)); }
 if (__remaining) { __remaining->tv_sec = 0,__remaining->tv_nsec = 0; }
 return 0;
#endif /* !__USE_TIME_BITS64 */
}
#endif /* __USE_TIME64 */
#elif defined(__GLC_COMPAT__) && defined(__USE_TIME_BITS64)
__REDIRECT(__LIBC,,int,__LIBCCALL,__nanosleep32,(struct __timespec32 const *__requested_time, struct __timespec32 *__remaining),(__requested_time,__remaining))
__LOCAL int (__LIBCCALL nanosleep)(struct timespec const *__requested_time, struct timespec *__remaining) {
 struct __timespec32 __req,__rem; int __result;
 if (__requested_time) __req.tv_sec = (__time32_t)__requested_time->tv_sec,
                       __req.tv_nsec = __requested_time->tv_nsec;
 __result = __nanosleep32(__requested_time ? &__req : 0,__remaining ? &__rem : 0);
 if (__remaining) __remaining->tv_sec = (__time64_t)rem.tv_sec,
                  __remaining->tv_nsec = rem.tv_nsec;
 return __result;
}
#ifdef __USE_TIME64
__LOCAL int (__LIBCCALL nanosleep64)(struct timespec64 const *__requested_time, struct timespec64 *__remaining) { return nanosleep(__requested_time,__remaining); }
#endif /* __USE_TIME64 */
#else /* Compat... */
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,nanosleep,(struct timespec const *__requested_time, struct timespec *__remaining),nanosleep,(__requested_time,__remaining))
#ifdef __USE_TIME64
#ifdef __GLC_COMPAT__
__LOCAL int (__LIBCCALL nanosleep64)(struct timespec64 const *__requested_time,
                                     struct timespec64 *__remaining) {
 struct __timespec32 __req,__rem; int __result;
 if (__requested_time) __req.tv_sec = (__time32_t)__requested_time->tv_sec,
                       __req.tv_nsec = __requested_time->tv_nsec;
#ifdef __USE_TIME_BITS64
 __result = __nanosleep32(__requested_time ? &__req : 0,__remaining ? &__rem : 0);
#else /* __USE_TIME_BITS64 */
 __result = nanosleep(__requested_time ? &__req : 0,__remaining ? &__rem : 0);
#endif /* !__USE_TIME_BITS64 */
 if (__remaining) __remaining->tv_sec = (__time64_t)rem.tv_sec,
                  __remaining->tv_nsec = rem.tv_nsec;
 return __result;
}
#else /* __GLC_COMPAT__ */
__LIBC int (__LIBCCALL nanosleep64)(struct timespec64 const *__requested_time, struct timespec64 *__remaining);
#endif /* !__GLC_COMPAT__ */
#endif /* __USE_TIME64 */
#endif /* Builtin... */

#ifdef __CRT_GLC
struct sigevent;
#if defined(__GLC_COMPAT__) && defined(__USE_TIME_BITS64)
#ifdef __USE_KOS
#define __itimerspec32  itimerspec32
#else /* __USE_KOS */
struct __itimerspec32 {
    struct __timespec32 it_interval;
    struct __timespec32 it_value;
};
#endif /* !__USE_KOS */
__REDIRECT(__LIBC,__PORT_NODOS,int,__LIBCCALL,__clock_getres32,(clockid_t __clock_id, struct timespec *__res),clock_getres,(__clock_id,__res))
__REDIRECT(__LIBC,__PORT_NODOS,int,__LIBCCALL,__clock_gettime32,(clockid_t __clock_id, struct timespec *__tp),clock_gettime,(__clock_id,__tp))
__REDIRECT(__LIBC,__PORT_NODOS,int,__LIBCCALL,__clock_settime32,(clockid_t __clock_id, struct timespec const *__tp),clock_settime,(__clock_id,__tp))
__REDIRECT(__LIBC,__PORT_NODOS,int,__LIBCCALL,__timer_settime32,(timer_t __timerid, int __flags, struct __itimerspec32 const *__restrict __value, struct __itimerspec32 *__restrict __ovalue),timer_settime,(__timerid,__flags,__value,__ovalue))
__REDIRECT(__LIBC,__PORT_NODOS,int,__LIBCCALL,__timer_gettime32,(timer_t __timerid, struct __itimerspec32 *__value),timer_gettime,(__timerid,__value))
__LOCAL __PORT_NODOS int (__LIBCCALL clock_getres)(clockid_t __clock_id, struct timespec *__res) {
 struct __timespec32 __res32; int __result;
 __result = __clock_getres32(__clock_id,__res ? &__res32 : 0);
 if (__res) __res->tv_sec = (__time64_t)__res32.tv_sec,
            __res->tv_nsec = __res32.tv_nsec;
 return __result;
}
__LOCAL __PORT_NODOS int (__LIBCCALL clock_gettime)(clockid_t __clock_id, struct timespec *__tp) {
 struct __timespec32 __res32; int __result;
 __result = __clock_gettime32(__clock_id,__tp ? &__res32 : 0);
 if (__tp) __tp->tv_sec = (__time64_t)__res32.tv_sec,
           __tp->tv_nsec = __res32.tv_nsec;
 return __result;
}
__LOCAL __PORT_NODOS int (__LIBCCALL clock_settime)(clockid_t __clock_id, struct timespec const *__tp) {
 struct __timespec32 __res32;
 if (__tp) __res32.tv_sec = (__time32_t)__tp->tv_sec,
           __res32.tv_nsec = __tp->tv_nsec;
 return __clock_settime32(__clock_id,__tp ? &__res32 : 0);
}
__LOCAL __PORT_NODOS int (__LIBCCALL timer_settime)(timer_t __timerid, int __flags, struct itimerspec const *__restrict __value, struct itimerspec *__restrict __ovalue) {
 struct __itimerspec32 __val,__oval; int __result;
 if (__value) __val.it_interval.tv_sec = (__time32_t)__value->it_interval.tv_sec,
              __val.it_interval.tv_nsec = __value->it_interval.tv_nsec,
              __val.it_value.tv_sec = (__time32_t)__value->it_value.tv_sec,
              __val.it_value.tv_nsec = __value->it_value.tv_nsec;
 __result = __timer_settime32(__timerid,__flags,__value ? &__val : 0,__ovalue ? &__oval : 0);
 if (__ovalue) __ovalue->it_interval.tv_sec = (__time64_t)__oval.it_interval.tv_sec,
               __ovalue->it_interval.tv_nsec = __oval.it_interval.tv_nsec,
               __ovalue->it_value.tv_sec = (__time64_t)__oval.it_value.tv_sec,
               __ovalue->it_value.tv_nsec = __oval.it_value.tv_nsec;
 return __result;
}
__LOCAL __PORT_NODOS int (__LIBCCALL timer_gettime)(timer_t __timerid, struct itimerspec *__value) {
 struct __itimerspec32 __val;
 if (__value) __val.it_interval.tv_sec = (__time32_t)__value->it_interval.tv_sec,
              __val.it_interval.tv_nsec = __value->it_interval.tv_nsec,
              __val.it_value.tv_sec = (__time32_t)__value->it_value.tv_sec,
              __val.it_value.tv_nsec = __value->it_value.tv_nsec;
 return __timer_gettime32(__timerid,__value ? &__val : 0);
}
#ifdef __USE_XOPEN2K
__REDIRECT(__LIBC,__PORT_NODOS,int,__LIBCCALL,__clock_nanosleep32,(clockid_t __clock_id, int __flags, struct timespec const *__req, struct timespec *__rem),clock_nanosleep,(__clock_id,__flags,__req,__rem))
__LOCAL __PORT_NODOS int (__LIBCCALL clock_nanosleep)(clockid_t __clock_id, int __flags, struct timespec const *__req, struct timespec *__rem) {
 struct __timespec32 __req,__rem; int __result;
 if (__requested_time) __req.tv_sec = (__time32_t)__requested_time->tv_sec,
                       __req.tv_nsec = __requested_time->tv_nsec;
 __result = __clock_nanosleep32(__clock_id,__flags,__requested_time ? &__req : 0,__remaining ? &__rem : 0);
 if (__remaining) __remaining->tv_sec = (__time64_t)rem.tv_sec,
                  __remaining->tv_nsec = rem.tv_nsec;
 return __result;
}
#endif /* __USE_XOPEN2K */
#else /* Compat... */
__REDIRECT_TM_FUNC(__LIBC,__PORT_NODOS,int,__LIBCCALL,clock_getres,(clockid_t __clock_id, struct timespec *__res),clock_getres,(__clock_id,__res))
__REDIRECT_TM_FUNC(__LIBC,__PORT_NODOS,int,__LIBCCALL,clock_gettime,(clockid_t __clock_id, struct timespec *__tp),clock_gettime,(__clock_id,__tp))
__REDIRECT_TM_FUNC(__LIBC,__PORT_NODOS,int,__LIBCCALL,clock_settime,(clockid_t __clock_id, struct timespec const *__tp),clock_settime,(__clock_id,__tp))
__REDIRECT_TM_FUNC(__LIBC,__PORT_NODOS,int,__LIBCCALL,timer_settime,(timer_t __timerid, int __flags, struct itimerspec const *__restrict __value, struct itimerspec *__restrict __ovalue),timer_settime,(__timerid,__flags,__value,__ovalue))
__REDIRECT_TM_FUNC(__LIBC,__PORT_NODOS,int,__LIBCCALL,timer_gettime,(timer_t __timerid, struct itimerspec *__value),timer_gettime,(__timerid,__value))
#ifdef __USE_XOPEN2K
__REDIRECT_TM_FUNC(__LIBC,__PORT_NODOS,int,__LIBCCALL,clock_nanosleep,(clockid_t __clock_id, int __flags, struct timespec const *__req, struct timespec *__rem),clock_nanosleep,(__clock_id,__flags,__req,__rem))
#endif /* __USE_XOPEN2K */
#endif /* Builtin... */

#ifdef __USE_TIME64
#ifdef __GLC_COMPAT__
__LOCAL __PORT_NODOS int (__LIBCCALL clock_getres64)(clockid_t __clock_id, struct timespec64 *__res) {
 struct __timespec32 __res32; int __result;
#ifdef __USE_TIME_BITS64
 __result = __clock_getres32(__clock_id,__res ? &__res32 : 0);
#else /* __USE_TIME_BITS64 */
 __result = clock_getres(__clock_id,__res ? &__res32 : 0);
#endif /* !__USE_TIME_BITS64 */
 if (__res) __res->tv_sec = (__time64_t)__res32.tv_sec,
            __res->tv_nsec = __res32.tv_nsec;
 return __result;
}
__LOCAL __PORT_NODOS int (__LIBCCALL clock_gettime64)(clockid_t __clock_id, struct timespec64 *__tp) {
 struct __timespec32 __res32; int __result;
#ifdef __USE_TIME_BITS64
 __result = __clock_gettime32(__clock_id,__tp ? &__res32 : 0);
#else /* __USE_TIME_BITS64 */
 __result = clock_gettime(__clock_id,__tp ? &__res32 : 0);
#endif /* !__USE_TIME_BITS64 */
 if (__tp) __tp->tv_sec = (__time64_t)__res32.tv_sec,
           __tp->tv_nsec = __res32.tv_nsec;
 return __result;
}
__LOCAL __PORT_NODOS int (__LIBCCALL clock_settime64)(clockid_t __clock_id, struct timespec64 const *__tp) {
 struct __timespec32 __res32;
 if (__tp) __res32.tv_sec = (__time32_t)__tp->tv_sec,
           __res32.tv_nsec = __tp->tv_nsec;
#ifdef __USE_TIME_BITS64
 return __clock_settime32(__clock_id,__tp ? &__res32 : 0);
#else /* __USE_TIME_BITS64 */
 return clock_settime(__clock_id,__tp ? &__res32 : 0);
#endif /* !__USE_TIME_BITS64 */
}
__LOCAL __PORT_NODOS int (__LIBCCALL timer_settime64)(timer_t __timerid, int __flags, struct itimerspec64 const *__restrict __value, struct itimerspec64 *__restrict __ovalue) {
 struct __itimerspec32 __val,__oval; int __result;
 if (__value) __val.it_interval.tv_sec = (__time32_t)__value->it_interval.tv_sec,
              __val.it_interval.tv_nsec = __value->it_interval.tv_nsec,
              __val.it_value.tv_sec = (__time32_t)__value->it_value.tv_sec,
              __val.it_value.tv_nsec = __value->it_value.tv_nsec;
#ifdef __USE_TIME_BITS64
 __result = __timer_settime32(__timerid,__flags,__value ? &__val : 0,__ovalue ? &__oval : 0);
#else /* __USE_TIME_BITS64 */
 __result = timer_settime(__timerid,__flags,__value ? &__val : 0,__ovalue ? &__oval : 0);
#endif /* !__USE_TIME_BITS64 */
 if (__ovalue) __ovalue->it_interval.tv_sec = (__time64_t)__oval.it_interval.tv_sec,
               __ovalue->it_interval.tv_nsec = __oval.it_interval.tv_nsec,
               __ovalue->it_value.tv_sec = (__time64_t)__oval.it_value.tv_sec,
               __ovalue->it_value.tv_nsec = __oval.it_value.tv_nsec;
 return __result;
}
__LOCAL __PORT_NODOS int (__LIBCCALL timer_gettime64)(timer_t __timerid, struct itimerspec64 *__value) {
 struct __itimerspec32 __val;
 if (__value) __val.it_interval.tv_sec = (__time32_t)__value->it_interval.tv_sec,
              __val.it_interval.tv_nsec = __value->it_interval.tv_nsec,
              __val.it_value.tv_sec = (__time32_t)__value->it_value.tv_sec,
              __val.it_value.tv_nsec = __value->it_value.tv_nsec;
#ifdef __USE_TIME_BITS64
 return __timer_gettime32(__timerid,__value ? &__val : 0);
#else /* __USE_TIME_BITS64 */
 return timer_gettime(__timerid,__value ? &__val : 0);
#endif /* !__USE_TIME_BITS64 */
}
#ifdef __USE_XOPEN2K
__LOCAL __PORT_NODOS int (__LIBCCALL clock_nanosleep64)(clockid_t __clock_id, int __flags, struct timespec64 const *__req, struct timespec64 *__rem) {
 struct __timespec32 __req,__rem; int __result;
 if (__requested_time) __req.tv_sec = (__time32_t)__requested_time->tv_sec,
                       __req.tv_nsec = __requested_time->tv_nsec;
#ifdef __USE_TIME_BITS64
 __result = __clock_nanosleep32(__clock_id,__flags,__requested_time ? &__req : 0,__remaining ? &__rem : 0);
#else /* __USE_TIME_BITS64 */
 __result = clock_nanosleep(__clock_id,__flags,__requested_time ? &__req : 0,__remaining ? &__rem : 0);
#endif /* !__USE_TIME_BITS64 */
 if (__remaining) __remaining->tv_sec = (__time64_t)rem.tv_sec,
                  __remaining->tv_nsec = rem.tv_nsec;
 return __result;
}
#endif /* __USE_XOPEN2K */
#else /* __GLC_COMPAT__ */
__LIBC __PORT_NODOS int (__LIBCCALL clock_getres64)(clockid_t __clock_id, struct timespec64 *__res);
__LIBC __PORT_NODOS int (__LIBCCALL clock_gettime64)(clockid_t __clock_id, struct timespec64 *__tp);
__LIBC __PORT_NODOS int (__LIBCCALL clock_settime64)(clockid_t __clock_id, struct timespec64 const *__tp);
__LIBC __PORT_NODOS int (__LIBCCALL timer_settime64)(timer_t __timerid, int __flags, struct itimerspec64 const *__restrict __value, struct itimerspec64 *__restrict __ovalue);
__LIBC __PORT_NODOS int (__LIBCCALL timer_gettime64)(timer_t __timerid, struct itimerspec64 *__value);
#ifdef __USE_XOPEN2K
__LIBC __PORT_NODOS int (__LIBCCALL clock_nanosleep64)(clockid_t __clock_id, int __flags, struct timespec64 const *__req, struct timespec64 *__rem);
#endif /* __USE_XOPEN2K */
#endif /* !__GLC_COMPAT__ */
#endif /* __USE_TIME64 */

__LIBC __PORT_NODOS int (__LIBCCALL timer_create)(clockid_t __clock_id, struct sigevent *__restrict __evp, timer_t *__restrict __timerid);
__LIBC __PORT_NODOS int (__LIBCCALL timer_delete)(timer_t __timerid);
__LIBC __PORT_NODOS int (__LIBCCALL timer_getoverrun)(timer_t __timerid);
#ifdef __USE_XOPEN2K
__LIBC __PORT_NODOS int (__LIBCCALL clock_getcpuclockid)(pid_t __pid, clockid_t *__clock_id);
#endif /* __USE_XOPEN2K */
#endif /* __CRT_GLC */
#endif /* __USE_POSIX199309 */

#ifdef __CRT_GLC
#ifdef __USE_ISOC11
#ifdef __GLC_COMPAT__
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,__timespec_get32,(struct timespec *__ts, int __base),timespec_get,(__ts,__base))
__LOCAL __PORT_NODOS __NONNULL((1)) int (__LIBCCALL timespec_get)(struct timespec *__ts, int __base) {
 int __result; struct __timespec32 __res;
 __result = __timespec_get32(&__res,__base);
 if (__result) __ts->tv_sec = (__time64_t)__res.tv_sec,
               __ts->tv_nsec = __res.tv_nsec;
 return __result;
}
#else /* __USE_TIME_BITS64 */
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL timespec_get)(struct timespec *__ts, int __base);
#endif /* !__USE_TIME_BITS64 */
#ifdef __USE_TIME64
__LOCAL __PORT_NODOS __NONNULL((1)) int (__LIBCCALL timespec_get64)(struct timespec64 *__ts, int __base) {
#ifdef __USE_TIME_BITS64
 return timespec_get(__ts,__base);
#else /* __USE_TIME_BITS64 */
 int __result; struct __timespec32 __res;
 __result = timespec_get(&__res,__base);
 if (__result) __ts->tv_sec = (__time64_t)__res.tv_sec,
               __ts->tv_nsec = __res.tv_nsec;
 return __result;
#endif /* !__USE_TIME_BITS64 */
}
#endif /* __USE_TIME64 */
#else /* __GLC_COMPAT__ */
__REDIRECT_TM_FUNC(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,timespec_get,(struct timespec *__ts, int __base),timespec_get,(__ts,__base))
#ifdef __USE_TIME64
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL timespec_get64)(struct timespec64 *__ts, int __base);
#endif /* __USE_TIME64 */
#endif /* !__GLC_COMPAT__ */
#endif /* __USE_ISOC11 */

#ifdef __USE_XOPEN_EXTENDED
#undef getdate_err
__LIBC __PORT_NODOS int getdate_err;
__LIBC __WUNUSED __PORT_NODOS struct tm *(__LIBCCALL getdate)(char const *__string);
#endif /* __USE_XOPEN_EXTENDED */
#endif /* __CRT_GLC */

#else /* !__KERNEL__ */

#ifdef __USE_MISC
#define dysize(year) (__isleap(year) ? 366 : 365)
#endif /* __USE_MISC */

#endif /* __KERNEL__ */

/* DOS Extensions. */
#ifdef __USE_DOS
#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

__REDIRECT_IFKOS(__LIBC,,size_t,__LIBCCALL,_strftime_l,
                (char *__restrict __buf, size_t __bufsize, const char *__restrict __format,
                 struct tm const *__restrict __tp, __locale_t __locale),
                 strftime_l,(__buf,__bufsize,__format,__tp,__locale))
__REDIRECT_IFKOS_VOID(__LIBC,,__LIBCCALL,_tzset,(void),tzset,())

__REDIRECT_IFKOS(__LIBC,__WUNUSED,char *,__LIBCCALL,_ctime32,(__time32_t const *__restrict __timer),ctime,(__timer))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,double,__LIBCCALL,_difftime32,(__time32_t __time1, __time32_t __time0),difftime,(__time1,__time0))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,_gmtime32,(__time32_t const *__restrict __timer),gmtime,(__timer))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,_localtime32,(__time32_t const *__restrict __timer),localtime,(__timer))
__REDIRECT_IFKOS(__LIBC,,__time32_t,__LIBCCALL,_time32,(__time32_t *__timer),time,(__timer))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,__time32_t,__LIBCCALL,_mktime32,(struct tm __FIXED_CONST *__restrict __tp),mktime,(__tp))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,__time32_t,__LIBCCALL,_mkgmtime32,(struct tm __FIXED_CONST *__restrict __tp),timegm,(__tp))

#ifdef __GLC_COMPAT__
__LOCAL __WUNUSED double (__LIBCCALL _difftime64)(__time64_t __time1, __time64_t __time0) { return _difftime32((__time32_t)__time1,(__time32_t)__time0); }
__LOCAL __WUNUSED char *(__LIBCCALL _ctime64)(__time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _ctime32(&__tm); }
__LOCAL __WUNUSED struct tm *(__LIBCCALL _gmtime64)(__time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _gmtime32(&__tm); }
__LOCAL __WUNUSED struct tm *(__LIBCCALL _localtime64)(__time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _localtime32(&__tm); }
__LOCAL __WUNUSED __time64_t (__LIBCCALL _mktime64)(struct tm __FIXED_CONST *__restrict __tp) { return (__time64_t)_mktime32(__tp); }
__LOCAL __WUNUSED __time64_t (__LIBCCALL _mkgmtime64)(struct tm __FIXED_CONST *__restrict __tp) { return (__time64_t)_mkgmtime32(__tp); }
__LOCAL __time64_t (__LIBCCALL _time64)(__time64_t *__timer) { __time32_t __res = _time32(NULL); if (__timer) *__timer = __res; return __res; }
#else /* __GLC_COMPAT__ */
__REDIRECT_IFKOS(__LIBC,__WUNUSED,double,__LIBCCALL,_difftime64,(__time64_t __time1, __time64_t __time0),difftime64,(__time1,__time0))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,char *,__LIBCCALL,_ctime64,(__time64_t const *__restrict __timer),ctime64,(__timer))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,_gmtime64,(__time64_t const *__restrict __timer),gmtime64,(__timer))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,_localtime64,(__time64_t const *__restrict __timer),localtime64,(__timer))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,__time64_t,__LIBCCALL,_mktime64,(struct tm __FIXED_CONST *__restrict __tp),mktime64,(__tp))
__REDIRECT_IFKOS(__LIBC,__WUNUSED,__time64_t,__LIBCCALL,_mkgmtime64,(struct tm __FIXED_CONST *__restrict __tp),mkgmtime64,(__tp))
__REDIRECT_IFKOS(__LIBC,,__time64_t,__LIBCCALL,_time64,(__time64_t *__timer),time64,(__timer))
#endif /* !__GLC_COMPAT__ */

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY __UINT32_TYPE__ (__LIBCCALL _getsystime)(struct tm *__restrict __tp);
__LIBC __PORT_DOSONLY __UINT32_TYPE__ (__LIBCCALL _setsystime)(struct tm __FIXED_CONST *__restrict __tp, __UINT32_TYPE__ __msec);
__LIBC __PORT_DOSONLY char *(__LIBCCALL _strdate)(char __buf[9]);
__LIBC __PORT_DOSONLY char *(__LIBCCALL _strtime)(char __buf[9]);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _strdate_s)(char __buf[9], size_t __bufsize);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _strtime_s)(char __buf[9], size_t __bufsize);
#endif /* __CRT_DOS */

#ifdef __GLC_COMPAT__
#ifndef ____ctime32_r_defined
#define ____ctime32_r_defined 1
__REDIRECT(__LIBC,,struct tm *,__LIBCCALL,__gmtime32_r,(__time32_t const *__restrict __timer, struct tm *__restrict __tp),gmtime_r,(__timer,__tp))
__REDIRECT(__LIBC,,struct tm *,__LIBCCALL,__localtime32_r,(__time32_t const *__restrict __timer, struct tm *__restrict __tp),localtime_r,(__timer,__tp))
__REDIRECT(__LIBC,,char *,__LIBCCALL,__ctime32_r,(__time32_t const *__restrict __timer, char *__restrict __buf),ctime_r,(__timer,__buf))
#endif /* !____ctime32_r_defined */
__LOCAL errno_t (__LIBCCALL _ctime32_s)(char __buf[26], size_t __bufsize, __time32_t const *__restrict __timer) { return __bufsize >= 26 && __ctime32_r(__timer_t,__buf) ? 0 : 1; }
__LOCAL errno_t (__LIBCCALL _ctime64_s)(char __buf[26], size_t __bufsize, __time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _ctime32_s(__buf,__bufsize,&__tm); }
__LOCAL errno_t (__LIBCCALL _gmtime32_s)(struct tm *__restrict __tp, __time32_t const *__restrict __timer) { return __gmtime32_r(__timer,__tp) ? 0 : 1; }
__LOCAL errno_t (__LIBCCALL _localtime32_s)(struct tm *__restrict __tp, __time32_t const *__restrict __timer) { return __localtime32_r(__timer,__tp) ? 0 : 1; }
__LOCAL errno_t (__LIBCCALL _gmtime64_s)(struct tm *__restrict __tp, __time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _gmtime32_s(__tp,&__tm); }
__LOCAL errno_t (__LIBCCALL _localtime64_s)(struct tm *__restrict __tp, __time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _localtime32_s(__tp,&__tm); }
#ifdef __USE_DOS_SLIB
__REDIRECT(__LIBC,,char *,__LIBCCALL,__asctime_r,(struct tm const *__restrict __tp, char *__restrict __buf),asctime_r,(__tp,__buf))
__LOCAL errno_t (__LIBCCALL asctime_s)(char __buf[26], size_t __bufsize, struct tm const *__restrict __tp) { return __bufsize >= 26 && __asctime_r(__tp,__buf) ? 0 : 1; }
#endif /* __USE_DOS_SLIB */
#else /* __GLC_COMPAT__ */
/* WARNING: The following functions always return DOS error codes! */
__LIBC errno_t (__LIBCCALL _ctime32_s)(char __buf[26], size_t __bufsize, __time32_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _gmtime32_s)(struct tm *__restrict __tp, __time32_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _localtime32_s)(struct tm *__restrict __tp, __time32_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _ctime64_s)(char __buf[26], size_t __bufsize, __time64_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _gmtime64_s)(struct tm *__restrict __tp, __time64_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _localtime64_s)(struct tm *__restrict __tp, __time64_t const *__restrict __timer);
#ifdef __USE_DOS_SLIB
__LIBC errno_t (__LIBCCALL asctime_s)(char __buf[26], size_t __bufsize, struct tm const *__restrict __tp);
#endif /* __USE_DOS_SLIB */
#endif /* !__GLC_COMPAT__ */

#ifndef _WTIME_DEFINED
#define _WTIME_DEFINED 1
__REDIRECT_IFW32(__LIBC,,size_t,__LIBCCALL,_wcsftime_l,(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, struct tm const *__restrict __ptm, __locale_t __locale),wcsftime_l,(__buf,__maxlen,__format,__ptm,__locale))
#ifdef __CRT_DOS
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wasctime_s,(wchar_t __buf[26], size_t __maxlen, struct tm const *__restrict __ptm),wasctime_s,(__buf,__maxlen,__ptm))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wstrdate_s,(wchar_t __buf[9], size_t __maxlen),wstrdate_s,(__buf,__maxlen))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wstrtime_s,(wchar_t __buf[9], size_t __maxlen),wstrtime_s,(__buf,__maxlen))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wasctime,(struct tm const *__restrict __ptm),wasctime,(__ptm))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wstrdate,(wchar_t *__restrict __buf),wstrdate,(__buf))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wstrtime,(wchar_t *__restrict __buf),wstrtime,(__buf))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wctime32,(__time32_t const *__restrict __timer),wctime32,(__timer))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_wctime64,(__time64_t const *__restrict __timer),wctime64,(__timer))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctime32_s,(wchar_t __buf[26], size_t __maxlen, __time32_t const *__timer),wctime32_s,(__buf,__maxlen,__timer))
__REDIRECT_IFW32(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctime64_s,(wchar_t __buf[26], size_t __maxlen, __time64_t const *__timer),wctime64_s,(__buf,__maxlen,__timer))
#ifdef __USE_TIME_BITS64
__REDIRECT2(__LIBC,__WUNUSED __PORT_DOSONLY,wchar_t *,__LIBCCALL,_wctime,(time_t const *__restrict __timer),wctime64,_wctime64,(__timer))
__REDIRECT2(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctime_s,(wchar_t *__restrict __buf, size_t __maxlen, time_t const *__restrict __timer),wctime64_s,_wctime64_s,(__buf,__maxlen,__timer))
#else /* __USE_TIME_BITS64 */
__REDIRECT2(__LIBC,__WUNUSED __PORT_DOSONLY,wchar_t *,__LIBCCALL,_wctime,(time_t const *__restrict __timer),wctime32,_wctime32,(__timer))
__REDIRECT2(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctime_s,(wchar_t *__restrict __buf, size_t __maxlen, time_t const *__restrict __timer),wctime32_s,_wctime32_s,(__buf,__maxlen,__timer))
#endif /* !__USE_TIME_BITS64 */
#endif /* __CRT_DOS */

#ifndef __std_wcsftime_defined
#define __std_wcsftime_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __NONNULL((1,3,4)) size_t (__LIBCCALL wcsftime)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, struct tm const *__restrict __ptm);
__NAMESPACE_STD_END
#endif /* !__std_wcsftime_defined */
#ifndef __wcsftime_defined
#define __wcsftime_defined 1
__NAMESPACE_STD_USING(wcsftime)
#endif /* !__wcsftime_defined */
#endif /* !_WTIME_DEFINED */

#endif /* __USE_DOS */
#undef __FIXED_CONST

__SYSDECL_END

#endif /* !_TIME_H */
