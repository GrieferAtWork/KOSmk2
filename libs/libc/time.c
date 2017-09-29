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
#ifndef GUARD_LIBS_LIBC_TIME_C
#define GUARD_LIBS_LIBC_TIME_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1
#define _TIME64_SOURCE 1

#include "libc.h"
#include "system.h"
#include "time.h"
#include "stdio.h"
#include "string.h"
#include "format-printer.h"
#include "unicode.h"
#include "malloc.h"

#include <time.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/section.h>
#include <limits.h>
#include <bits/fcntl-linux.h>
#include <sys/timeb.h>
#include <utime.h>
#include <bits/stat.h>
#include <sys/times.h>

DECL_BEGIN

#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(_tzname,tzname);
DEFINE_PUBLIC_ALIAS(_timezone,timezone);
DEFINE_PUBLIC_ALIAS(_tzset,libc_tzset);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
DEFINE_PUBLIC_ALIAS(__tzname,tzname);
DEFINE_PUBLIC_ALIAS(__daylight,daylight);
DEFINE_PUBLIC_ALIAS(__timezone,timezone);
PUBLIC int      getdate_err;
PUBLIC char    *tzname[2];
PUBLIC int      daylight;
PUBLIC long int timezone;

INTERN void LIBCCALL libc_tzset(void) {
 /* TODO */
 tzname[0] = "foo";
 tzname[1] = "bar";
 daylight = 1;
 timezone = 42;
}

INTDEF char const abbr_month_names[12][4];
INTDEF char const abbr_wday_names[7][4];
INTERN u16 const time_monthstart_yday[2][13] = {
 {0,31,59,90,120,151,181,212,243,273,304,334,365},
 {0,31,60,91,121,152,182,213,244,274,305,335,366}};

/* LIBC time-size independent functions. */

#define ASCTIME_BUFSIZE 26
INTERN char *LIBCCALL libc_asctime_r(struct tm const *__restrict tp,
                                     char *__restrict buf) {
 if unlikely(!tp) { SET_ERRNO(EINVAL); return NULL; }
 if unlikely(tp->tm_year > INT_MAX-1900) { SET_ERRNO(EOVERFLOW); return NULL; }
 libc_sprintf(buf,"%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
             ((unsigned int)tp->tm_wday >= 7 ? "???" : abbr_wday_names[tp->tm_wday]),
             ((unsigned int)tp->tm_mon >= 12 ? "???" : abbr_month_names[tp->tm_mon]),
              tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec,tp->tm_year+1900);
 return buf;
}
INTERN char *LIBCCALL libc_asctime(struct tm const *tp) {
 PRIVATE char buf[ASCTIME_BUFSIZE];
 return libc_asctime_r(tp,buf);
}

struct ftimebuf { char *iter,*end; };
PRIVATE ssize_t LIBCCALL
time_printer(char const *__restrict s, size_t n,
             struct ftimebuf *__restrict data) {
 char *new_iter;
 n = libc_strnlen(s,n);
 new_iter = data->iter+n;
 if (new_iter >= data->end) return -1;
 libc_memcpy(data->iter,s,n*sizeof(char));
 data->iter = new_iter;
 return 0;
}

#define LINUX_TIME_START_YEAR 1970
#define SECS_PER_DAY          86400

INTERN char *LIBCCALL
libc_strptime(char const *__restrict s,
              char const *__restrict format,
              struct tm *tp) {
 NOT_IMPLEMENTED();
 return NULL;
}

LOCAL int LIBCCALL tm_calc_isdst(struct tm const *self) {
 /* found here: "http://stackoverflow.com/questions/5590429/calculating-daylight-savings-time-from-only-date" */
 int previousSunday;
 //January, February, and December are out.
 if (self->tm_mon < 3 || self->tm_mon > 11) { return 0; }
 //April to October are in
 if (self->tm_mon > 3 && self->tm_mon < 11) { return 1; }
 previousSunday = self->tm_mday-self->tm_wday;
 //In march, we are DST if our previous Sunday was on or after the 8th.
 if (self->tm_mon == 3) { return previousSunday >= 8; }
 //In November we must be before the first Sunday to be dst.
 //That means the previous Sunday must be before the 1st.
 return previousSunday <= 0;
}

INTERN int LIBCCALL libc_dysize(int year) { return __isleap(year) ? 366 : 365; }
INTERN int LIBCCALL libc_getdate_r(char const *__restrict string,
                                   struct tm *__restrict resbufp) {
 NOT_IMPLEMENTED();
 return -1;
}
PRIVATE ATTR_RAREBSS struct tm getdate_buf;
INTERN struct tm *LIBCCALL libc_getdate(char const *string) {
 struct tm *result = &getdate_buf;
 return libc_getdate_r(string,result) ? NULL : result;
}
INTERN char *LIBCCALL
libc_strptime_l(char const *__restrict s, char const *__restrict format,
                struct tm *tp, locale_t loc) {
 NOT_IMPLEMENTED();
 return libc_strptime(s,format,tp);
}


/* LIBC proxy/independent time functions */
INTERN char *LIBCCALL libc_ctime64_r(time64_t const *__restrict timer, char *__restrict buf) { struct tm ltm; return libc_asctime_r(libc_localtime64_r(timer,&ltm),buf); }
INTERN char *LIBCCALL libc_ctime_r(time_t const *__restrict timer, char *__restrict buf) { struct tm ltm; return libc_asctime_r(libc_localtime_r(timer,&ltm),buf); }
PRIVATE char ctime_buf[ASCTIME_BUFSIZE]; 
INTERN char *LIBCCALL libc_ctime64(time64_t const *timer) { return libc_ctime64_r(timer,ctime_buf); }
INTERN char *LIBCCALL libc_ctime(time_t const *timer) { return libc_ctime_r(timer,ctime_buf); }

INTERN time64_t LIBCCALL libc_mktime64(struct tm *tp) {
 time64_t result;
 result = __yearstodays(tp->tm_year) -
          __yearstodays(LINUX_TIME_START_YEAR);
 result += tp->tm_yday;
 result *= SECS_PER_DAY;
 result += tp->tm_hour*60*60;
 result += tp->tm_min*60;
 result += tp->tm_sec;
 return result;
}
INTERN struct tm *LIBCCALL
libc_gmtime64_r(time64_t const *__restrict timer,
                struct tm *__restrict tp) {
 time64_t t; int i; u16 const *monthvec;
 t = *timer;
 tp->tm_sec  = (int)(t % 60);
 tp->tm_min  = (int)((t/60) % 60);
 tp->tm_hour = (int)((t/(60*60)) % 24);
 t /= SECS_PER_DAY;
 t += __yearstodays(LINUX_TIME_START_YEAR);
 tp->tm_wday = (int)(t % 7);
 tp->tm_year = (int)__daystoyears(t);
 t -= __yearstodays(tp->tm_year);
 tp->tm_yday = (int)t;
 monthvec = time_monthstart_yday[__isleap(tp->tm_year)];
 for (i = 1; i < 12; ++i) if (monthvec[i] >= t) break;
 tp->tm_mon  = i-1;
 t -= monthvec[i-1];
 tp->tm_mday = t+1;
 tp->tm_isdst = tm_calc_isdst(tp);
 tp->tm_year -= 1900;
 return tp;
}
INTERN time_t LIBCCALL libc_mktime(struct tm *tp) { return (time_t)libc_mktime64(tp); }
INTERN struct tm *LIBCCALL libc_gmtime_r(time_t const *__restrict timer, struct tm *__restrict tp) { time64_t t = (time64_t)*timer; return libc_gmtime64_r(&t,tp); }
PRIVATE struct tm gmtime_buf,localtime_buf;
INTERN struct tm *LIBCCALL libc_localtime_r(time_t const *__restrict timer, struct tm *__restrict tp) { return libc_gmtime_r(timer,tp); /* TODO: Timezones 'n $hit. */ }
INTERN struct tm *LIBCCALL libc_localtime64_r(time64_t const *__restrict timer, struct tm *__restrict tp) { return libc_gmtime64_r(timer,tp); /* TODO: Timezones 'n $hit. */ }
INTERN struct tm *LIBCCALL libc_gmtime(time_t const *timer) { return libc_gmtime_r(timer,&gmtime_buf); }
INTERN struct tm *LIBCCALL libc_gmtime64(time64_t const *timer) { return libc_gmtime64_r(timer,&gmtime_buf); }
INTERN struct tm *LIBCCALL libc_localtime(time_t const *timer) { return libc_localtime_r(timer,&localtime_buf); }
INTERN struct tm *LIBCCALL libc_localtime64(time64_t const *timer) { return libc_localtime64_r(timer,&localtime_buf); }
INTERN double LIBCCALL libc_difftime(time_t time1, time_t time0) { return time1 > time0 ? time1-time0 : time0-time1; }
INTERN double LIBCCALL libc_difftime64(time64_t time1, time64_t time0) { return time1 > time0 ? time1-time0 : time0-time1; }

/* Kernel-time independent system-call functions */
INTERN int LIBCCALL libc_timer_create(clockid_t clock_id, struct sigevent *__restrict evp, timer_t *__restrict timerid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_timer_delete(timer_t timerid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_timer_getoverrun(timer_t timerid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_clock_getcpuclockid(pid_t pid, clockid_t *clock_id) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_timespec_get(struct timespec *ts, int base) { NOT_IMPLEMENTED(); return -1; }


#ifdef CONFIG_32BIT_TIME
#define A(x)   x
#define A_T(x) x##_t
#define A_R(x) x##_r
#define B(x)   x##64
#define B_T(x) x##64_t
#define B_R(x) x##64_r
#else
#define A(x)   x##64
#define A_T(x) x##64_t
#define A_R(x) x##64_r
#define B(x)   x
#define B_T(x) x##_t
#define B_R(x) x##_r
#endif

#define atime_t     A_T(time)
#define btime_t     B_T(time)
#define atimespec   A(timespec)
#define btimespec   B(timespec)
#define atimeval    A(timeval)
#define btimeval    B(timeval)
#define aitimerval  A(itimerval)
#define bitimerval  B(itimerval)
#define aitimerspec A(itimerspec)
#define bitimerspec B(itimerspec)


/* Kernel-time dependent system-call functions */
INTERN atime_t LIBCCALL A(libc_time)(atime_t *timer) {
 struct atimeval now;
 if (A(libc_gettimeofday)(&now,NULL)) return -1;
 if (timer) *timer = now.tv_sec;
 return now.tv_sec;
}
INTERN int LIBCCALL A(libc_stime)(atime_t const *when) {
 struct atimeval now;
 now.tv_sec = *when;
 now.tv_usec = 0;
 return A(libc_settimeofday)(&now,NULL);
}
INTERN int LIBCCALL A(libc_gettimeofday)(struct atimeval *__restrict tv, __timezone_ptr_t tz) { return FORWARD_SYSTEM_ERROR(sys_gettimeofday(tv,tz)); }
INTERN int LIBCCALL A(libc_settimeofday)(struct atimeval const *tv, struct timezone const *tz) { return FORWARD_SYSTEM_ERROR(sys_settimeofday(tv,tz)); }
INTERN int LIBCCALL A(libc_nanosleep)(struct atimespec const *requested_time, struct atimespec *remaining) { return FORWARD_SYSTEM_ERROR(sys_nanosleep(requested_time,remaining)); }
INTERN atime_t LIBCCALL A(libc_timelocal)(struct tm *tp) { return A(libc_mktime)(tp); /* XXX: Timezones */ }
INTERN int LIBCCALL A(libc_adjtime)(struct atimeval const *delta, struct atimeval *olddelta) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL A(libc_getitimer)(__itimer_which_t which, struct aitimerval *value) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL A(libc_setitimer)(__itimer_which_t which, const struct aitimerval *__restrict new_, struct aitimerval *__restrict old) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL A(libc_clock_getres)(clockid_t clock_id, struct atimespec *res) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL A(libc_clock_gettime)(clockid_t clock_id, struct atimespec *tp) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL A(libc_clock_settime)(clockid_t clock_id, struct atimespec const *tp) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL A(libc_timer_settime)(timer_t timerid, int flags, struct aitimerspec const *__restrict value, struct aitimerspec *__restrict ovalue) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL A(libc_timer_gettime)(timer_t timerid, struct aitimerspec *value) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL A(libc_clock_nanosleep)(clockid_t clock_id, int flags, struct atimespec const *req, struct atimespec *rem) { NOT_IMPLEMENTED(); return A(libc_nanosleep)(req,rem); }
INTERN btime_t LIBCCALL B(libc_timelocal)(struct tm *tp) { return (btime_t)A(libc_timelocal)(tp); }
INTERN btime_t LIBCCALL B(libc_time)(btime_t *timer) { btime_t result = (btime_t)A(libc_time)(NULL); if (timer) *timer = result; return result; }
INTERN int LIBCCALL B(libc_stime)(btime_t const *when) { atime_t awhen = (atime_t)*when; return A(libc_stime)(&awhen); }
INTERN int LIBCCALL B(libc_gettimeofday)(struct btimeval *__restrict tv, __timezone_ptr_t tz) { struct atimeval atv; int result = A(libc_gettimeofday)(&atv,tz); if (!result) tv->tv_sec = (btime_t)atv.tv_sec,tv->tv_usec = atv.tv_usec; return result; }
INTERN int LIBCCALL B(libc_settimeofday)(struct btimeval const *tv, struct timezone const *tz) { struct atimeval atv = {(atime_t)tv->tv_sec,tv->tv_usec}; return A(libc_settimeofday)(&atv,tz); }
INTERN int LIBCCALL B(libc_adjtime)(struct btimeval const *delta, struct btimeval *olddelta) {
 struct atimeval atv,oatv; int result;
 if (delta)
     atv.tv_sec  = (atime_t)delta->tv_sec,
     atv.tv_usec = delta->tv_usec;
 result = A(libc_adjtime)(delta ? &atv : NULL,olddelta ? &oatv : NULL);
 if (!result && olddelta)
     olddelta->tv_sec = (btime_t)oatv.tv_sec,
     olddelta->tv_usec = oatv.tv_usec;
 return result;
}
INTERN int LIBCCALL B(libc_getitimer)(__itimer_which_t which, struct bitimerval *value) {
 struct aitimerval aval; int result;
 result = A(libc_getitimer)(which,&aval);
 if (!result) {
  value->it_interval.tv_sec  = (btime_t)aval.it_interval.tv_sec;
  value->it_interval.tv_usec = aval.it_interval.tv_usec;
  value->it_value.tv_sec     = (btime_t)aval.it_value.tv_sec;
  value->it_value.tv_usec    = aval.it_value.tv_usec;
 }
 return result;
}
INTERN int LIBCCALL B(libc_setitimer)(__itimer_which_t which,
                                      struct bitimerval const *__restrict new_,
                                      struct bitimerval *__restrict old) {
 struct aitimerval anew,aold; int result;
 if (new_) {
  anew.it_interval.tv_sec  = (atime_t)new_->it_interval.tv_sec;
  anew.it_interval.tv_usec = new_->it_interval.tv_usec;
  anew.it_value.tv_sec     = (atime_t)new_->it_value.tv_sec;
  anew.it_value.tv_usec    = new_->it_value.tv_usec;
 }
 result = A(libc_setitimer)(which,new_ ? &anew : NULL,old ? &aold : NULL);
 if (!result && old) {
  old->it_interval.tv_sec  = (btime_t)aold.it_interval.tv_sec;
  old->it_interval.tv_usec = aold.it_interval.tv_usec;
  old->it_value.tv_sec     = (btime_t)aold.it_value.tv_sec;
  old->it_value.tv_usec    = aold.it_value.tv_usec;
 }
 return result;
}
INTERN int LIBCCALL B(libc_nanosleep)(struct btimespec const *requested_time,
                                      struct btimespec *remaining) {
 struct atimespec areq,arem; int result;
 areq.tv_sec  = (atime_t)requested_time->tv_sec;
 areq.tv_nsec = requested_time->tv_nsec;
 result = A(libc_nanosleep)(&areq,remaining ? &arem : NULL);
 if (!result && remaining) {
  remaining->tv_sec  = (btime_t)arem.tv_sec;
  remaining->tv_nsec = arem.tv_nsec;
 }
 return result;
}
INTERN int LIBCCALL B(libc_clock_getres)(clockid_t clock_id, struct btimespec *res) {
 struct atimespec ares; int result;
 result = A(libc_clock_getres)(clock_id,&ares);
 if (!result) {
  res->tv_sec  = (btime_t)ares.tv_sec;
  res->tv_nsec = ares.tv_nsec;
 }
 return result;
}
INTERN int LIBCCALL B(libc_clock_gettime)(clockid_t clock_id, struct btimespec *tp) {
 struct atimespec ares; int result;
 result = A(libc_clock_gettime)(clock_id,&ares);
 if (!result) {
  tp->tv_sec  = (btime_t)ares.tv_sec;
  tp->tv_nsec = ares.tv_nsec;
 }
 return result;
}
INTERN int LIBCCALL B(libc_clock_settime)(clockid_t clock_id, struct btimespec const *tp) {
 struct atimespec ares;
 ares.tv_sec  = (btime_t)tp->tv_sec;
 ares.tv_nsec = tp->tv_nsec;
 return A(libc_clock_settime)(clock_id,&ares);
}
INTERN int LIBCCALL B(libc_timer_settime)(timer_t timerid, int flags,
                                          struct bitimerspec const *__restrict value,
                                          struct bitimerspec *__restrict ovalue) {
 struct aitimerspec anew,aold; int result;
 if (value) {
  anew.it_interval.tv_sec  = (atime_t)value->it_interval.tv_sec;
  anew.it_interval.tv_nsec = value->it_interval.tv_nsec;
  anew.it_value.tv_sec     = (atime_t)value->it_value.tv_sec;
  anew.it_value.tv_nsec    = value->it_value.tv_nsec;
 }
 result = A(libc_timer_settime)(timerid,flags,value ? &anew : NULL,ovalue ? &aold : NULL);
 if (!result && ovalue) {
  ovalue->it_interval.tv_sec  = (btime_t)aold.it_interval.tv_sec;
  ovalue->it_interval.tv_nsec = aold.it_interval.tv_nsec;
  ovalue->it_value.tv_sec     = (btime_t)aold.it_value.tv_sec;
  ovalue->it_value.tv_nsec    = aold.it_value.tv_nsec;
 }
 return result;
}
INTERN int LIBCCALL B(libc_timer_gettime)(timer_t timerid, struct bitimerspec *value) {
 struct aitimerspec ares; int result;
 result = A(libc_timer_gettime)(timerid,&ares);
 if (!result) {
  value->it_interval.tv_sec  = (btime_t)ares.it_interval.tv_sec;
  value->it_interval.tv_nsec = ares.it_interval.tv_nsec;
  value->it_value.tv_sec     = (btime_t)ares.it_value.tv_sec;
  value->it_value.tv_nsec    = ares.it_value.tv_nsec;
 }
 return result;
}
INTERN int LIBCCALL B(libc_clock_nanosleep)(clockid_t clock_id, int flags,
                                            struct btimespec const *req,
                                            struct btimespec *rem) {
 struct atimespec areq,arem; int result;
 areq.tv_sec  = (atime_t)req->tv_sec;
 areq.tv_nsec = req->tv_nsec;
 result = A(libc_nanosleep)(&areq,rem ? &arem : NULL);
 if (!result && rem) {
  rem->tv_sec  = (btime_t)arem.tv_sec;
  rem->tv_nsec = arem.tv_nsec;
 }
 return result;
}

INTERN int LIBCCALL A(libc_sigtimedwait)(sigset_t const *__restrict set, siginfo_t *__restrict info, struct atimespec const *timeout) { return FORWARD_SYSTEM_ERROR(sys_sigtimedwait(set,info,timeout,sizeof(siginfo_t))); }
INTERN int LIBCCALL A(libc_ppoll)(struct pollfd *fds, nfds_t nfds, struct atimespec const *timeout, sigset_t const *ss) { return FORWARD_SYSTEM_VALUE((int)sys_ppoll(fds,nfds,timeout,ss,sizeof(sigset_t))); }
INTERN int LIBCCALL A(libc_select)(int nfds, fd_set *__restrict readfds,
                                   fd_set *__restrict writefds,
                                   fd_set *__restrict exceptfds,
                                   struct atimeval *__restrict timeout) {
 struct atimespec tmo;
 if (!timeout) return A(libc_pselect)(nfds,readfds,writefds,exceptfds,NULL,NULL);
 TIMEVAL_TO_TIMESPEC(timeout,&tmo);
 return A(libc_pselect)(nfds,readfds,writefds,exceptfds,&tmo,NULL);
}
INTERN int LIBCCALL A(libc_pselect)(int nfds, fd_set *__restrict readfds,
                                    fd_set *__restrict writefds,
                                    fd_set *__restrict exceptfds,
                                    struct atimespec const *__restrict timeout,
                                    sigset_t const *__restrict sigmask) {
 int error;
 if (sigmask) {
  struct { sigset_t const *p; size_t s; } sgm = {sigmask,sizeof(sigset_t)};
  error = (int)sys_pselect6(nfds,readfds,writefds,exceptfds,timeout,&sgm);
 } else {
  error = (int)sys_pselect6(nfds,readfds,writefds,exceptfds,timeout,NULL);
 }
 if (E_ISERR(error)) { SET_ERRNO(-error); return -1; }
 return error;
}



INTERN int LIBCCALL B(libc_sigtimedwait)(sigset_t const *__restrict set,
                                         siginfo_t *__restrict info,
                                         struct btimespec const *timeout) {
 struct atimespec tma;
 if (!timeout) return A(libc_sigtimedwait)(set,info,NULL);
 tma.tv_sec  = (time64_t)timeout->tv_sec;
 tma.tv_nsec = timeout->tv_nsec;
 return A(libc_sigtimedwait)(set,info,&tma);
}
INTERN int LIBCCALL A(libc_utimensat)(int fd, char const *path,
                                      struct atimespec const times[2],
                                      int flags) {
 return FORWARD_SYSTEM_ERROR(sys_utimensat(fd,path,times,flags));
}
INTERN int LIBCCALL B(libc_utimensat)(int fd, char const *path,
                                      struct btimespec const times[2],
                                      int flags) {
 struct atimespec atime[2];
 if (!times) return A(libc_utimensat)(fd,path,NULL,flags);
 atime[0].tv_sec  = (atime_t)times[0].tv_sec;
 atime[0].tv_nsec = times[0].tv_nsec;
 atime[1].tv_sec  = (atime_t)times[1].tv_sec;
 atime[1].tv_nsec = times[1].tv_nsec;
 return A(libc_utimensat)(fd,path,atime,flags);
}

INTERN int LIBCCALL B(libc_ppoll)(struct pollfd *fds, nfds_t nfds,
                                  struct timespec const *timeout,
                                  sigset_t const *ss) {
 struct atimespec tmo;
 if (!timeout) return A(libc_ppoll)(fds,nfds,NULL,ss);
 tmo.tv_sec  = (atime_t)timeout->tv_sec;
 tmo.tv_nsec = timeout->tv_nsec;
 return A(libc_ppoll)(fds,nfds,&tmo,ss);
}
INTERN int LIBCCALL B(libc_select)(int nfds, fd_set *__restrict readfds,
                                   fd_set *__restrict writefds,
                                   fd_set *__restrict exceptfds,
                                   struct btimeval *__restrict timeout) {
 struct atimespec tmo;
 if (!timeout) return A(libc_pselect)(nfds,readfds,writefds,exceptfds,NULL,NULL);
 TIMEVAL_TO_TIMESPEC(timeout,&tmo);
 return A(libc_pselect)(nfds,readfds,writefds,exceptfds,&tmo,NULL);
}

INTERN int LIBCCALL B(libc_pselect)(int nfds, fd_set *__restrict readfds,
                                    fd_set *__restrict writefds,
                                    fd_set *__restrict exceptfds,
                                    struct btimespec const *__restrict timeout,
                                    sigset_t const *__restrict sigmask) {
 struct atimespec tmo;
 if (!timeout) return A(libc_pselect)(nfds,readfds,writefds,exceptfds,NULL,sigmask);
 tmo.tv_sec  = (atime_t)timeout->tv_sec;
 tmo.tv_nsec = timeout->tv_nsec;
 return A(libc_pselect)(nfds,readfds,writefds,exceptfds,&tmo,sigmask);
}

PRIVATE int LIBCCALL A(impl_futimesat)(int fd, char const *file,
                                       struct atimeval const tvp[2],
                                       int flags) {
 struct atimespec times[2];
 TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
 TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 return A(libc_utimensat)(fd,file,times,flags);
}
PRIVATE int LIBCCALL B(impl_futimesat)(int fd, char const *file,
                                       struct btimeval const tvp[2],
                                       int flags) {
 struct atimespec times[2];
 TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
 TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 return A(libc_utimensat)(fd,file,times,flags);
}
INTERN int LIBCCALL A(libc_futimes)(int fd, struct atimeval const tvp[2]) {
 struct atimespec times[2];
 TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
 TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 return A(libc_futimens)(fd,times);
}
INTERN int LIBCCALL B(libc_futimes)(int fd, struct btimeval const tvp[2]) {
 struct atimespec times[2];
 TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
 TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 return A(libc_futimens)(fd,times);
}
INTERN int LIBCCALL A(libc_futimens)(int fd, struct atimespec const times[2]) { return A(libc_utimensat)(fd,NULL,times,0); }
INTERN int LIBCCALL B(libc_futimens)(int fd, struct btimespec const times[2]) { return B(libc_utimensat)(fd,NULL,times,0); }
INTERN int LIBCCALL A(libc_utimes)(char const *file, struct atimeval const tvp[2]) { return A(impl_futimesat)(AT_FDCWD,file,tvp,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL A(libc_lutimes)(char const *file, struct atimeval const tvp[2]) { return A(impl_futimesat)(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
INTERN int LIBCCALL A(libc_futimesat)(int fd, char const *file, struct atimeval const tvp[2]) { return A(impl_futimesat)(fd,file,tvp,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL B(libc_utimes)(char const *file, struct btimeval const tvp[2]) { return B(impl_futimesat)(AT_FDCWD,file,tvp,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL B(libc_lutimes)(char const *file, struct btimeval const tvp[2]) { return B(impl_futimesat)(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
INTERN int LIBCCALL B(libc_futimesat)(int fd, char const *file, struct btimeval const tvp[2]) { return B(impl_futimesat)(fd,file,tvp,AT_SYMLINK_FOLLOW); }

INTERN int LIBCCALL A(libc_futimeat)(int dfd, char const *file, struct A(utimbuf) const *file_times, int flags) {
 struct atimespec times[2];
 if (file_times) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  times[0].tv_sec  = (atime_t)file_times->actime;
  times[1].tv_sec  = (atime_t)file_times->modtime;
  times[0].tv_nsec = 0;
  times[1].tv_nsec = 0;
 }
 return A(libc_utimensat)(dfd,file,times,flags);
}
INTERN int LIBCCALL B(libc_futimeat)(int dfd, char const *file, struct B(utimbuf) const *file_times, int flags) {
 struct atimespec times[2];
 if (file_times) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  times[0].tv_sec  = (atime_t)file_times->actime;
  times[1].tv_sec  = (atime_t)file_times->modtime;
  times[0].tv_nsec = 0;
  times[1].tv_nsec = 0;
 }
 return A(libc_utimensat)(dfd,file,times,flags);
}

INTERN int LIBCCALL A(libc_utime)(char const *file, struct A(utimbuf) const *file_times) { return A(libc_futimeat)(AT_FDCWD,file,file_times,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL B(libc_utime)(char const *file, struct B(utimbuf) const *file_times) { return B(libc_futimeat)(AT_FDCWD,file,file_times,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL A(libc_futime)(int fd, struct A(utimbuf) const *file_times) { return A(libc_futimeat)(fd,NULL,file_times,0); }
INTERN int LIBCCALL B(libc_futime)(int fd, struct B(utimbuf) const *file_times) { return B(libc_futimeat)(fd,NULL,file_times,0); }
INTERN useconds_t LIBCCALL libc_ualarm(useconds_t value, useconds_t interval) { NOT_IMPLEMENTED(); return -1; }
INTERN unsigned int LIBCCALL libc_alarm(unsigned int seconds) { NOT_IMPLEMENTED(); return seconds; }
INTERN int LIBCCALL libc_pause(void) { return A(libc_select)(0,NULL,NULL,NULL,NULL); }

INTERN int LIBCCALL libc_poll(struct pollfd *fds, nfds_t nfds, int timeout) {
 struct atimespec tmo;
 /* NOTE: A negative value means infinite timeout! */
 if (timeout < 0)
     return A(libc_ppoll)(fds,nfds,NULL,NULL);
 /* NOTE: A timeout of ZERO(0) means try once; stop immediately. */
 if (!timeout) {
  tmo.tv_sec  = 0;
  tmo.tv_nsec = 0;
 } else {
  tmo.tv_sec  = timeout/MSEC_PER_SEC;
  tmo.tv_nsec = NSEC_PER_MSEC*(timeout%MSEC_PER_SEC);
 }
 return A(libc_ppoll)(fds,nfds,&tmo,NULL);
}
INTERN int LIBCCALL libc_sigwaitinfo(sigset_t const *__restrict set,
                                     siginfo_t *__restrict info) {
 return A(libc_sigtimedwait)(set,info,NULL);
}
INTERN int LIBCCALL libc_sigwait(sigset_t const *__restrict set,
                                 int *__restrict sig) {
 siginfo_t info;
 int error = A(libc_sigtimedwait)(set,&info,NULL);
 if (!error) *sig = info.si_signo;
 return error;
}
INTERN unsigned int LIBCCALL libc_sleep(unsigned int seconds) {
 struct atimespec req,rem;
 req.tv_sec  = seconds;
 req.tv_nsec = 0;
 A(libc_nanosleep)(&req,&rem);
 return rem.tv_sec;
}
INTERN int LIBCCALL libc_usleep(useconds_t useconds) {
 struct atimespec req,rem; int error;
 req.tv_sec  =  useconds/USEC_PER_SEC;
 req.tv_nsec = (useconds%USEC_PER_SEC)*NSEC_PER_USEC;
 while ((error = A(libc_nanosleep)(&req,&rem)) != 0 &&
         GET_ERRNO() == EINTR) req = rem;
 return error;
}

INTERN int LIBCCALL A(libc_ftime)(struct A(timeb) *timebuf) {
 struct atimeval tv; struct timezone tz;
 int result = A(libc_gettimeofday)(&tv,&tz);
 if (!result) {
  timebuf->time     = tv.tv_sec;
  timebuf->millitm  = tv.tv_usec / USEC_PER_MSEC;
  timebuf->timezone = tz.tz_minuteswest;
  timebuf->dstflag  = tz.tz_dsttime;
 }
 return result;
}
INTERN int LIBCCALL B(libc_ftime)(struct B(timeb) *timebuf) {
 struct atimeval tv; struct timezone tz;
 int result = A(libc_gettimeofday)(&tv,&tz);
 if (!result) {
  timebuf->time     = (btime_t)tv.tv_sec;
  timebuf->millitm  = tv.tv_usec / USEC_PER_MSEC;
  timebuf->timezone = tz.tz_minuteswest;
  timebuf->dstflag  = tz.tz_dsttime;
 }
 return result;
}

PRIVATE clock_t clock_start = -1;
INTERN clock_t LIBCCALL libc_clock(void) {
 struct atimeval now; clock_t result;
 /* Really hacky implementation... */
 if (A(libc_gettimeofday)(&now,NULL)) return -1;
 result = (now.tv_usec*(CLOCKS_PER_SEC/USEC_PER_SEC)+
           now.tv_sec*CLOCKS_PER_SEC);
 if (clock_start < 0) clock_start = result;
 return result-clock_start;
}
INTERN clock_t LIBCCALL
libc_times(struct tms *__restrict buffer) {
 /* This isn't right either... */
 clock_t clock_now = libc_clock();
 buffer->tms_stime = clock_now/2;
 buffer->tms_utime = clock_now/2;
 buffer->tms_cutime = 0;
 buffer->tms_cstime = 0;
 return clock_now;
}
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN clock_t LIBCCALL libc_dos_clock(void) {
 return libc_clock()/(CLOCKS_PER_SEC/__DOS_CLOCKS_PER_SEC);
}
INTERN clock_t LIBCCALL
libc_dos_times(struct tms *__restrict buffer) {
 /* This isn't right either... */
 clock_t clock_now = libc_dos_clock();
 buffer->tms_stime = clock_now/2;
 buffer->tms_utime = clock_now/2;
 buffer->tms_cutime = 0;
 buffer->tms_cstime = 0;
 return clock_now;
}
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


DEFINE_PUBLIC_ALIAS(tzset,libc_tzset);
DEFINE_PUBLIC_ALIAS(asctime_r,libc_asctime_r);
DEFINE_PUBLIC_ALIAS(asctime,libc_asctime);
DEFINE_PUBLIC_ALIAS(strftime,libc_strftime);
DEFINE_PUBLIC_ALIAS(strptime,libc_strptime);
DEFINE_PUBLIC_ALIAS(dysize,libc_dysize);
DEFINE_PUBLIC_ALIAS(getdate_r,libc_getdate_r);
DEFINE_PUBLIC_ALIAS(getdate,libc_getdate);
DEFINE_PUBLIC_ALIAS(strftime_l,libc_strftime_l);
DEFINE_PUBLIC_ALIAS(strptime_l,libc_strptime_l);
DEFINE_PUBLIC_ALIAS(ctime64_r,libc_ctime64_r);
DEFINE_PUBLIC_ALIAS(ctime_r,libc_ctime_r);
DEFINE_PUBLIC_ALIAS(ctime64,libc_ctime64);
DEFINE_PUBLIC_ALIAS(ctime,libc_ctime);
DEFINE_PUBLIC_ALIAS(mktime64,libc_mktime64);
DEFINE_PUBLIC_ALIAS(gmtime64_r,libc_gmtime64_r);
DEFINE_PUBLIC_ALIAS(mktime,libc_mktime);
DEFINE_PUBLIC_ALIAS(gmtime_r,libc_gmtime_r);
DEFINE_PUBLIC_ALIAS(localtime_r,libc_localtime_r);
DEFINE_PUBLIC_ALIAS(localtime64_r,libc_localtime64_r);
DEFINE_PUBLIC_ALIAS(gmtime,libc_gmtime);
DEFINE_PUBLIC_ALIAS(gmtime64,libc_gmtime64);
DEFINE_PUBLIC_ALIAS(localtime,libc_localtime);
DEFINE_PUBLIC_ALIAS(localtime64,libc_localtime64);
DEFINE_PUBLIC_ALIAS(difftime,libc_difftime);
DEFINE_PUBLIC_ALIAS(difftime64,libc_difftime64);
DEFINE_PUBLIC_ALIAS(clock,libc_clock);
DEFINE_PUBLIC_ALIAS(timer_create,libc_timer_create);
DEFINE_PUBLIC_ALIAS(timer_delete,libc_timer_delete);
DEFINE_PUBLIC_ALIAS(timer_getoverrun,libc_timer_getoverrun);
DEFINE_PUBLIC_ALIAS(clock_getcpuclockid,libc_clock_getcpuclockid);
DEFINE_PUBLIC_ALIAS(timespec_get,libc_timespec_get);
DEFINE_PUBLIC_ALIAS(time64,libc_time64);
DEFINE_PUBLIC_ALIAS(stime64,libc_stime64);
DEFINE_PUBLIC_ALIAS(gettimeofday64,libc_gettimeofday64);
DEFINE_PUBLIC_ALIAS(settimeofday64,libc_settimeofday64);
DEFINE_PUBLIC_ALIAS(nanosleep64,libc_nanosleep64);
DEFINE_PUBLIC_ALIAS(timelocal64,libc_timelocal64);
DEFINE_PUBLIC_ALIAS(adjtime64,libc_adjtime64);
DEFINE_PUBLIC_ALIAS(getitimer64,libc_getitimer64);
DEFINE_PUBLIC_ALIAS(setitimer64,libc_setitimer64);
DEFINE_PUBLIC_ALIAS(clock_getres64,libc_clock_getres64);
DEFINE_PUBLIC_ALIAS(clock_gettime64,libc_clock_gettime64);
DEFINE_PUBLIC_ALIAS(clock_settime64,libc_clock_settime64);
DEFINE_PUBLIC_ALIAS(timer_settime64,libc_timer_settime64);
DEFINE_PUBLIC_ALIAS(timer_gettime64,libc_timer_gettime64);
DEFINE_PUBLIC_ALIAS(clock_nanosleep64,libc_clock_nanosleep64);
DEFINE_PUBLIC_ALIAS(sigtimedwait64,libc_sigtimedwait64);
DEFINE_PUBLIC_ALIAS(utimensat64,libc_utimensat64);
DEFINE_PUBLIC_ALIAS(ppoll64,libc_ppoll64);
DEFINE_PUBLIC_ALIAS(select64,libc_select64);
DEFINE_PUBLIC_ALIAS(pselect64,libc_pselect64);
DEFINE_PUBLIC_ALIAS(futimes64,libc_futimes64);
DEFINE_PUBLIC_ALIAS(futimens64,libc_futimens64);
DEFINE_PUBLIC_ALIAS(utime64,libc_utime64);
DEFINE_PUBLIC_ALIAS(utimes64,libc_utimes64);
DEFINE_PUBLIC_ALIAS(lutimes64,libc_lutimes64);
DEFINE_PUBLIC_ALIAS(futimesat64,libc_futimesat64);
DEFINE_PUBLIC_ALIAS(ftime64,libc_ftime64);
DEFINE_PUBLIC_ALIAS(timelocal,libc_timelocal);
DEFINE_PUBLIC_ALIAS(time,libc_time);
DEFINE_PUBLIC_ALIAS(stime,libc_stime);
DEFINE_PUBLIC_ALIAS(gettimeofday,libc_gettimeofday);
DEFINE_PUBLIC_ALIAS(settimeofday,libc_settimeofday);
DEFINE_PUBLIC_ALIAS(adjtime,libc_adjtime);
DEFINE_PUBLIC_ALIAS(getitimer,libc_getitimer);
DEFINE_PUBLIC_ALIAS(setitimer,libc_setitimer);
DEFINE_PUBLIC_ALIAS(nanosleep,libc_nanosleep);
DEFINE_PUBLIC_ALIAS(clock_getres,libc_clock_getres);
DEFINE_PUBLIC_ALIAS(clock_gettime,libc_clock_gettime);
DEFINE_PUBLIC_ALIAS(clock_settime,libc_clock_settime);
DEFINE_PUBLIC_ALIAS(timer_settime,libc_timer_settime);
DEFINE_PUBLIC_ALIAS(timer_gettime,libc_timer_gettime);
DEFINE_PUBLIC_ALIAS(clock_nanosleep,libc_clock_nanosleep);
DEFINE_PUBLIC_ALIAS(sigtimedwait,libc_sigtimedwait);
DEFINE_PUBLIC_ALIAS(utimensat,libc_utimensat);
DEFINE_PUBLIC_ALIAS(ppoll,libc_ppoll);
DEFINE_PUBLIC_ALIAS(select,libc_select);
DEFINE_PUBLIC_ALIAS(pselect,libc_pselect);
DEFINE_PUBLIC_ALIAS(futimes,libc_futimes);
DEFINE_PUBLIC_ALIAS(futimens,libc_futimens);
DEFINE_PUBLIC_ALIAS(utime,libc_utime);
DEFINE_PUBLIC_ALIAS(utimes,libc_utimes);
DEFINE_PUBLIC_ALIAS(lutimes,libc_lutimes);
DEFINE_PUBLIC_ALIAS(futimesat,libc_futimesat);
DEFINE_PUBLIC_ALIAS(ftime,libc_ftime);
DEFINE_PUBLIC_ALIAS(ualarm,libc_ualarm);
DEFINE_PUBLIC_ALIAS(alarm,libc_alarm);
DEFINE_PUBLIC_ALIAS(pause,libc_pause);
DEFINE_PUBLIC_ALIAS(poll,libc_poll);
DEFINE_PUBLIC_ALIAS(sleep,libc_sleep);
DEFINE_PUBLIC_ALIAS(usleep,libc_usleep);
DEFINE_PUBLIC_ALIAS(times,libc_times);

DEFINE_PUBLIC_ALIAS(timegm,libc_mktime); /* ??? */
DEFINE_PUBLIC_ALIAS(timegm64,libc_mktime64); /* ??? */

#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(_mktime32,libc_mktime);
DEFINE_PUBLIC_ALIAS(_mktime64,libc_mktime64);
DEFINE_PUBLIC_ALIAS(_sleep,libc_sleep);
DEFINE_PUBLIC_ALIAS(_time32,libc_time);
DEFINE_PUBLIC_ALIAS(_time64,libc_time64);
DEFINE_PUBLIC_ALIAS(__DSYM(clock),libc_dos_clock);
DEFINE_PUBLIC_ALIAS(__DSYM(times),libc_dos_times);
DEFINE_PUBLIC_ALIAS(_ftime,libc_ftime); /* This is not an error. - DOS defines this name, too. */
DEFINE_PUBLIC_ALIAS(_ftime32,libc_ftime);
DEFINE_PUBLIC_ALIAS(_ftime64,libc_ftime64);
DEFINE_PUBLIC_ALIAS(_ftime32_s,libc_ftime);
DEFINE_PUBLIC_ALIAS(_ftime64_s,libc_ftime64);

/* Define DOS-mode time functions. */
INTERN ATTR_DOSTEXT int LIBCCALL A(libc_dos_utimensat)(int fd, char const *path, struct atimespec const times[2], int flags) { return A(libc_utimensat)(fd,path,times,AT_DOSPATH|flags); }
INTERN ATTR_DOSTEXT int LIBCCALL B(libc_dos_utimensat)(int fd, char const *path, struct btimespec const times[2], int flags) { return B(libc_utimensat)(fd,path,times,AT_DOSPATH|flags); }
INTERN ATTR_DOSTEXT int LIBCCALL A(libc_dos_utimes)(char const *file, struct atimeval const tvp[2]) { return A(impl_futimesat)(AT_FDCWD,file,tvp,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL B(libc_dos_utimes)(char const *file, struct btimeval const tvp[2]) { return B(impl_futimesat)(AT_FDCWD,file,tvp,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL A(libc_dos_lutimes)(char const *file, struct atimeval const tvp[2]) { return A(impl_futimesat)(AT_FDCWD,file,tvp,AT_DOSPATH|AT_SYMLINK_NOFOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL B(libc_dos_lutimes)(char const *file, struct btimeval const tvp[2]) { return B(impl_futimesat)(AT_FDCWD,file,tvp,AT_DOSPATH|AT_SYMLINK_NOFOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL A(libc_dos_futimesat)(int fd, char const *file, struct atimeval const tvp[2]) { return A(impl_futimesat)(fd,file,tvp,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL B(libc_dos_futimesat)(int fd, char const *file, struct btimeval const tvp[2]) { return B(impl_futimesat)(fd,file,tvp,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL A(libc_dos_utime)(char const *file, struct A(utimbuf) const *file_times) { return A(libc_futimeat)(AT_FDCWD,file,file_times,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN ATTR_DOSTEXT int LIBCCALL B(libc_dos_utime)(char const *file, struct B(utimbuf) const *file_times) { return B(libc_futimeat)(AT_FDCWD,file,file_times,AT_DOSPATH|AT_SYMLINK_FOLLOW); }

INTERN int LIBCCALL libc_16wutime(char16_t const *file, struct utimbuf const *file_times);
INTERN int LIBCCALL libc_32wutime(char32_t const *file, struct utimbuf const *file_times);
INTERN int LIBCCALL libc_dos_16wutime(char16_t const *file, struct utimbuf const *file_times);
INTERN int LIBCCALL libc_dos_32wutime(char32_t const *file, struct utimbuf const *file_times);
INTERN int LIBCCALL libc_16wutime64(char16_t const *file, struct utimbuf64 const *file_times);
INTERN int LIBCCALL libc_32wutime64(char32_t const *file, struct utimbuf64 const *file_times);
INTERN int LIBCCALL libc_dos_16wutime64(char16_t const *file, struct utimbuf64 const *file_times);
INTERN int LIBCCALL libc_dos_32wutime64(char32_t const *file, struct utimbuf64 const *file_times);

DEFINE_PUBLIC_ALIAS(__DSYM(utimensat),libc_dos_utimensat);
DEFINE_PUBLIC_ALIAS(__DSYM(utimensat64),libc_dos_utimensat64);
DEFINE_PUBLIC_ALIAS(__DSYM(utimes),libc_dos_utimes);
DEFINE_PUBLIC_ALIAS(__DSYM(utimes64),libc_dos_utimes64);
DEFINE_PUBLIC_ALIAS(__DSYM(lutimes),libc_dos_lutimes);
DEFINE_PUBLIC_ALIAS(__DSYM(lutimes64),libc_dos_lutimes64);
DEFINE_PUBLIC_ALIAS(__DSYM(futimesat),libc_dos_futimesat);
DEFINE_PUBLIC_ALIAS(__DSYM(futimesat64),libc_dos_futimesat64);

/* NOTE: These are named weirdly thanks to DOS */
DEFINE_PUBLIC_ALIAS(_futime,libc_futime); /* This is not an error. - DOS defines this name, too. */
DEFINE_PUBLIC_ALIAS(_futime32,libc_futime);
DEFINE_PUBLIC_ALIAS(_futime64,libc_futime64);
DEFINE_PUBLIC_ALIAS(_utime,libc_dos_utime); /* This is not an error. - DOS defines this name, too. */
DEFINE_PUBLIC_ALIAS(_utime32,libc_dos_utime);
DEFINE_PUBLIC_ALIAS(_utime64,libc_dos_utime64);

/* Wide-character version of utime(). */
INTERN int LIBCCALL
A(libc_w16futimeat)(int dfd, char16_t const *file,
                    struct A(utimbuf) const *file_times, int flags) {
 char *utf8file; int result;
 if unlikely(!file)
  result = A(libc_futimeat)(dfd,NULL,file_times,flags);
 else {
  result = -1;
  utf8file = libc_utf16to8m(file,libc_16wcslen(file));
  if (utf8file) result = A(libc_futimeat)(dfd,NULL,file_times,flags),
                libc_free(utf8file);
 }
 return result;
}
INTERN int LIBCCALL
A(libc_w32futimeat)(int dfd, char32_t const *file,
                    struct A(utimbuf) const *file_times, int flags) {
 char *utf8file; int result;
 if unlikely(!file)
  result = A(libc_futimeat)(dfd,NULL,file_times,flags);
 else {
  result = -1;
  utf8file = libc_utf32to8m(file,libc_32wcslen(file));
  if (utf8file) result = A(libc_futimeat)(dfd,NULL,file_times,flags),
                libc_free(utf8file);
 }
 return result;
}

INTERN int LIBCCALL
B(libc_w16futimeat)(int dfd, char16_t const *file,
                    struct B(utimbuf) const *file_times, int flags) {
 struct A(utimbuf) abuf;
 if (file_times) {
  abuf.actime  = (atime_t)file_times->actime;
  abuf.modtime = (atime_t)file_times->modtime;
 }
 return A(libc_w16futimeat)(dfd,file,file_times ? &abuf : NULL,flags);
}
INTERN int LIBCCALL
B(libc_w32futimeat)(int dfd, char32_t const *file,
                    struct B(utimbuf) const *file_times, int flags) {
 struct A(utimbuf) abuf;
 if (file_times) {
  abuf.actime  = (atime_t)file_times->actime;
  abuf.modtime = (atime_t)file_times->modtime;
 }
 return A(libc_w32futimeat)(dfd,file,file_times ? &abuf : NULL,flags);
}

INTERN int LIBCCALL libc_16wutime(char16_t const *file, struct utimbuf const *file_times) { return libc_w16futimeat(AT_FDCWD,file,file_times,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_16wutime64(char16_t const *file, struct utimbuf64 const *file_times) { return libc_w16futimeat64(AT_FDCWD,file,file_times,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_32wutime(char32_t const *file, struct utimbuf const *file_times) { return libc_w32futimeat(AT_FDCWD,file,file_times,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_32wutime64(char32_t const *file, struct utimbuf64 const *file_times) { return libc_w32futimeat64(AT_FDCWD,file,file_times,AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_dos_16wutime(char16_t const *file, struct utimbuf const *file_times) { return libc_w16futimeat(AT_FDCWD,file,file_times,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_dos_16wutime64(char16_t const *file, struct utimbuf64 const *file_times) { return libc_w16futimeat64(AT_FDCWD,file,file_times,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_dos_32wutime(char32_t const *file, struct utimbuf const *file_times) { return libc_w32futimeat(AT_FDCWD,file,file_times,AT_DOSPATH|AT_SYMLINK_FOLLOW); }
INTERN int LIBCCALL libc_dos_32wutime64(char32_t const *file, struct utimbuf64 const *file_times) { return libc_w32futimeat64(AT_FDCWD,file,file_times,AT_DOSPATH|AT_SYMLINK_FOLLOW); }

DEFINE_PUBLIC_ALIAS(_wutime,libc_dos_16wutime); /* Alias also defined by DOS. */
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wutime32),libc_16wutime);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wutime32),libc_32wutime);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wutime32),libc_dos_16wutime);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wutime32),libc_dos_32wutime);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wutime64),libc_16wutime64);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wutime64),libc_32wutime64);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wutime64),libc_dos_16wutime64);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wutime64),libc_dos_32wutime64);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

INTERN size_t LIBCCALL
libc_strftime(char *__restrict s, size_t maxsize,
              char const *__restrict format,
              struct tm const *__restrict tp) {
 struct ftimebuf buf; buf.end = (buf.iter = s)+maxsize;
 if (libc_format_strftime((pformatprinter)&time_printer,&buf,format,tp)) return 0;
 if (buf.iter == buf.end) return 0;
 *buf.iter = '\0';
 return (size_t)(buf.iter-s);
}
INTERN size_t LIBCCALL
libc_32wcsftime(char32_t *__restrict s, size_t maxsize,
                char32_t const *__restrict format,
                struct tm const *__restrict tp) {
 NOT_IMPLEMENTED(); /* TODO */
 return 0;
}

DEFINE_PUBLIC_ALIAS(wcsftime,libc_32wcsftime);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(wcsftime_l,libc_32wcsftime_l);
INTERN size_t LIBCCALL
libc_16wcsftime(char16_t *__restrict s, size_t maxsize,
                char16_t const *__restrict format,
                struct tm const *__restrict tp) {
 NOT_IMPLEMENTED(); /* TODO */
 return 0;
}
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */



INTERN size_t LIBCCALL
libc_strftime_l(char *__restrict s, size_t maxsize,
                char const *__restrict format,
                struct tm const *__restrict tp,
                locale_t UNUSED(loc)) {
 return libc_strftime(s,maxsize,format,tp);
}
INTERN size_t LIBCCALL
libc_32wcsftime_l(char32_t *__restrict s, size_t maxsize,
                  char32_t const *__restrict format,
                  struct tm const *__restrict tp,
                  locale_t UNUSED(loc)) {
 return libc_32wcsftime(s,maxsize,format,tp);
}
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN size_t LIBCCALL
libc_16wcsftime_l(char16_t *__restrict s, size_t maxsize,
                  char16_t const *__restrict format,
                  struct tm const *__restrict tp,
                  locale_t UNUSED(loc)) {
 return libc_16wcsftime(s,maxsize,format,tp);
}
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


DECL_END

#endif /* !GUARD_LIBS_LIBC_TIME_C */
