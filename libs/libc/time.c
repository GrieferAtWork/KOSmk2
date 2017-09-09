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

#include <hybrid/compiler.h>
#include <sys/time.h>
#include <time.h>
#include <hybrid/types.h>
#include <string.h>
#include <format-printer.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <hybrid/section.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/stat.h>

DECL_BEGIN

PUBLIC int getdate_err;
PUBLIC char *tzname[2];
PUBLIC int daylight;
PUBLIC long int timezone;
PUBLIC void (LIBCCALL tzset)(void) {
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
PUBLIC char *(LIBCCALL asctime_r)(struct tm const *__restrict tp,
                                  char *__restrict buf) {
 if __unlikely(!tp) { __set_errno(EINVAL); return NULL; }
 if __unlikely(tp->tm_year > INT_MAX-1900) { __set_errno(EOVERFLOW); return NULL; }
 sprintf(buf,"%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
        (tp->tm_wday < 0 || tp->tm_wday >= 7 ? "???" : abbr_wday_names[tp->tm_wday]),
        (tp->tm_mon < 0 || tp->tm_mon >= 12 ? "???" : abbr_month_names[tp->tm_mon]),
         tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec,tp->tm_year+1900);
 return buf;
}
PUBLIC char *(LIBCCALL asctime)(struct tm const *tp) {
 PRIVATE char buf[ASCTIME_BUFSIZE];
 return asctime_r(tp,buf);
}

struct ftimebuf { char *iter,*end; };
PRIVATE ssize_t LIBCCALL
time_printer(char const *__restrict s, size_t n,
             struct ftimebuf *__restrict data) {
 char *new_iter;
 n = strnlen(s,n);
 new_iter = data->iter+n;
 if (new_iter >= data->end) return -1;
 memcpy(data->iter,s,n*sizeof(char));
 data->iter = new_iter;
 return 0;
}

#define LINUX_TIME_START_YEAR 1970
#define SECS_PER_DAY          86400


PUBLIC size_t (LIBCCALL strftime)(char *__restrict s, size_t maxsize,
                                  char const *__restrict format,
                                  struct tm const *__restrict tp) {
 struct ftimebuf buf; buf.end = (buf.iter = s)+maxsize;
 if (format_strftime((pformatprinter)&time_printer,&buf,format,tp)) return 0;
 if (buf.iter == buf.end) return 0;
 *buf.iter = '\0';
 return (size_t)(buf.iter-s);
}
PUBLIC char *(LIBCCALL strptime)(char const *__restrict s,
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

PUBLIC int (LIBCCALL dysize)(int year) { return __isleap(year) ? 366 : 365; }
PUBLIC int (LIBCCALL getdate_r)(char const *__restrict string,
                                struct tm *__restrict resbufp) {
 NOT_IMPLEMENTED();
 return -1;
}
PRIVATE ATTR_RAREBSS struct tm getdate_buf;
PUBLIC struct tm *(LIBCCALL getdate)(char const *string) {
 struct tm *result = &getdate_buf;
 return getdate_r(string,result) ? NULL : result;
}

PUBLIC size_t (LIBCCALL strftime_l)(char *__restrict s, size_t maxsize,
                                    char const *__restrict format,
                                    struct tm const *__restrict tp,
                                    locale_t loc) {
 NOT_IMPLEMENTED();
 return strftime(s,maxsize,format,tp);
}
PUBLIC char *(LIBCCALL strptime_l)(char const *__restrict s, char const *__restrict format,
                                   struct tm *tp, locale_t loc) {
 NOT_IMPLEMENTED();
 return strptime(s,format,tp);
}


/* LIBC proxy/independent time functions */
PUBLIC char *(LIBCCALL ctime64_r)(time64_t const *__restrict timer, char *__restrict buf) { struct tm ltm; return asctime_r(localtime64_r(timer,&ltm),buf); }
PUBLIC char *(LIBCCALL ctime_r)(time_t const *__restrict timer, char *__restrict buf) { struct tm ltm; return asctime_r(localtime_r(timer,&ltm),buf); }
PRIVATE char ctime_buf[ASCTIME_BUFSIZE]; 
PUBLIC char *(LIBCCALL ctime64)(time64_t const *timer) { return ctime64_r(timer,ctime_buf); }
PUBLIC char *(LIBCCALL ctime)(time_t const *timer) { return ctime_r(timer,ctime_buf); }

PUBLIC time64_t (LIBCCALL mktime64)(struct tm *tp) {
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
PUBLIC struct tm *(LIBCCALL gmtime64_r)
(time64_t const *__restrict timer, struct tm *__restrict tp) {
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
PUBLIC time_t (LIBCCALL mktime)(struct tm *tp) { return (time_t)mktime64(tp); }
PUBLIC struct tm *(LIBCCALL gmtime_r)(time_t const *__restrict timer, struct tm *__restrict tp) { time64_t t = (time64_t)*timer; return gmtime64_r(&t,tp); }
PRIVATE struct tm gmtime_buf,localtime_buf;
PUBLIC struct tm *(LIBCCALL localtime_r)(time_t const *__restrict timer, struct tm *__restrict tp) { return gmtime_r(timer,tp); /* TODO: Timezones 'n $hit. */ }
PUBLIC struct tm *(LIBCCALL localtime64_r)(time64_t const *__restrict timer, struct tm *__restrict tp) { return gmtime64_r(timer,tp); /* TODO: Timezones 'n $hit. */ }
PUBLIC struct tm *(LIBCCALL gmtime)(time_t const *timer) { return gmtime_r(timer,&gmtime_buf); }
PUBLIC struct tm *(LIBCCALL gmtime64)(time64_t const *timer) { return gmtime64_r(timer,&gmtime_buf); }
PUBLIC struct tm *(LIBCCALL localtime)(time_t const *timer) { return localtime_r(timer,&localtime_buf); }
PUBLIC struct tm *(LIBCCALL localtime64)(time64_t const *timer) { return localtime64_r(timer,&localtime_buf); }
PUBLIC double (LIBCCALL difftime)(time_t time1, time_t time0) { return time1 > time0 ? time1-time0 : time0-time1; }
PUBLIC double (LIBCCALL difftime64)(time64_t time1, time64_t time0) { return time1 > time0 ? time1-time0 : time0-time1; }

/* Kernel-time independent system-call functions */
PUBLIC clock_t (LIBCCALL clock)(void) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL timer_create)(clockid_t clock_id, struct sigevent *__restrict evp, timer_t *__restrict timerid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL timer_delete)(timer_t timerid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL timer_getoverrun)(timer_t timerid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL clock_getcpuclockid)(pid_t pid, clockid_t *clock_id) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL timespec_get)(struct timespec *ts, int base) { NOT_IMPLEMENTED(); return -1; }


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
PUBLIC atime_t (LIBCCALL A(time))(atime_t *timer) {
 struct atimeval now;
 if (A(gettimeofday)(&now,NULL)) return -1;
 if (timer) *timer = now.tv_sec;
 return now.tv_sec;
}
PUBLIC int (LIBCCALL A(stime))(atime_t const *when) {
 struct atimeval now;
 now.tv_sec = *when;
 now.tv_usec = 0;
 return A(settimeofday)(&now,NULL);
}
PUBLIC int (LIBCCALL A(gettimeofday))(struct atimeval *__restrict tv, __timezone_ptr_t tz) { return FORWARD_SYSTEM_ERROR(sys_gettimeofday(tv,tz)); }
PUBLIC int (LIBCCALL A(settimeofday))(struct atimeval const *tv, struct timezone const *tz) { return FORWARD_SYSTEM_ERROR(sys_settimeofday(tv,tz)); }
PUBLIC atime_t (LIBCCALL A(timelocal))(struct tm *tp) { return mktime(tp); /* XXX: Timezones */ }
PUBLIC int (LIBCCALL A(adjtime))(struct atimeval const *delta, struct atimeval *olddelta) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL A(getitimer))(__itimer_which_t which, struct aitimerval *value) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL A(setitimer))(__itimer_which_t which, const struct aitimerval *__restrict new_, struct aitimerval *__restrict old) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL A(nanosleep))(struct atimespec const *requested_time, struct atimespec *remaining) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL A(clock_getres))(clockid_t clock_id, struct atimespec *res) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL A(clock_gettime))(clockid_t clock_id, struct atimespec *tp) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL A(clock_settime))(clockid_t clock_id, struct atimespec const *tp) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL A(timer_settime))(timer_t timerid, int flags, struct aitimerspec const *__restrict value, struct aitimerspec *__restrict ovalue) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL A(timer_gettime))(timer_t timerid, struct aitimerspec *value) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL A(clock_nanosleep))(clockid_t clock_id, int flags, struct atimespec const *req, struct atimespec *rem) { NOT_IMPLEMENTED(); return -1; }

PUBLIC btime_t (LIBCCALL B(timelocal))(struct tm *tp) { return (btime_t)A(timelocal)(tp); }
PUBLIC btime_t (LIBCCALL B(time))(btime_t *timer) { btime_t result = (btime_t)A(time)(NULL); if (timer) *timer = result; return result; }
PUBLIC int (LIBCCALL B(stime))(btime_t const *when) { atime_t awhen = (atime_t)*when; return A(stime)(&awhen); }
PUBLIC int (LIBCCALL B(gettimeofday))(struct btimeval *__restrict tv, __timezone_ptr_t tz) { struct atimeval atv; int result = A(gettimeofday)(&atv,tz); if (!result) tv->tv_sec = (btime_t)atv.tv_sec,tv->tv_usec = atv.tv_usec; return result; }
PUBLIC int (LIBCCALL B(settimeofday))(struct btimeval const *tv, struct timezone const *tz) { struct atimeval atv = {(atime_t)tv->tv_sec,tv->tv_usec}; return A(settimeofday)(&atv,tz); }
PUBLIC int (LIBCCALL B(adjtime))(struct btimeval const *delta, struct btimeval *olddelta) {
 struct atimeval atv,oatv; int result;
 if (delta)
     atv.tv_sec  = (atime_t)delta->tv_sec,
     atv.tv_usec = delta->tv_usec;
 result = A(adjtime)(delta ? &atv : NULL,olddelta ? &oatv : NULL);
 if (!result && olddelta)
     olddelta->tv_sec = (btime_t)oatv.tv_sec,
     olddelta->tv_usec = oatv.tv_usec;
 return result;
}
PUBLIC int (LIBCCALL B(getitimer))(__itimer_which_t which, struct bitimerval *value) {
 struct aitimerval aval; int result;
 result = A(getitimer)(which,&aval);
 if (!result) {
  value->it_interval.tv_sec  = (btime_t)aval.it_interval.tv_sec;
  value->it_interval.tv_usec = aval.it_interval.tv_usec;
  value->it_value.tv_sec     = (btime_t)aval.it_value.tv_sec;
  value->it_value.tv_usec    = aval.it_value.tv_usec;
 }
 return result;
}
PUBLIC int (LIBCCALL B(setitimer))(__itimer_which_t which,
                                   struct bitimerval const *__restrict new_,
                                   struct bitimerval *__restrict old) {
 struct aitimerval anew,aold; int result;
 if (new_) {
  anew.it_interval.tv_sec  = (atime_t)new_->it_interval.tv_sec;
  anew.it_interval.tv_usec = new_->it_interval.tv_usec;
  anew.it_value.tv_sec     = (atime_t)new_->it_value.tv_sec;
  anew.it_value.tv_usec    = new_->it_value.tv_usec;
 }
 result = A(setitimer)(which,new_ ? &anew : NULL,old ? &aold : NULL);
 if (!result && old) {
  old->it_interval.tv_sec  = (btime_t)aold.it_interval.tv_sec;
  old->it_interval.tv_usec = aold.it_interval.tv_usec;
  old->it_value.tv_sec     = (btime_t)aold.it_value.tv_sec;
  old->it_value.tv_usec    = aold.it_value.tv_usec;
 }
 return result;
}
PUBLIC int (LIBCCALL B(nanosleep))(struct btimespec const *requested_time,
                                   struct btimespec *remaining) {
 struct atimespec areq,arem; int result;
 areq.tv_sec  = (atime_t)requested_time->tv_sec;
 areq.tv_nsec = requested_time->tv_nsec;
 result = A(nanosleep)(&areq,remaining ? &arem : NULL);
 if (!result && remaining) {
  remaining->tv_sec  = (btime_t)arem.tv_sec;
  remaining->tv_nsec = arem.tv_nsec;
 }
 return result;
}
PUBLIC int (LIBCCALL B(clock_getres))(clockid_t clock_id, struct btimespec *res) {
 struct atimespec ares; int result;
 result = A(clock_getres)(clock_id,&ares);
 if (!result) {
  res->tv_sec  = (btime_t)ares.tv_sec;
  res->tv_nsec = ares.tv_nsec;
 }
 return result;
}
PUBLIC int (LIBCCALL B(clock_gettime))(clockid_t clock_id, struct btimespec *tp) {
 struct atimespec ares; int result;
 result = A(clock_gettime)(clock_id,&ares);
 if (!result) {
  tp->tv_sec  = (btime_t)ares.tv_sec;
  tp->tv_nsec = ares.tv_nsec;
 }
 return result;
}
PUBLIC int (LIBCCALL B(clock_settime))(clockid_t clock_id, struct btimespec const *tp) {
 struct atimespec ares;
 ares.tv_sec  = (btime_t)tp->tv_sec;
 ares.tv_nsec = tp->tv_nsec;
 return A(clock_settime)(clock_id,&ares);
}
PUBLIC int (LIBCCALL B(timer_settime))(timer_t timerid, int flags,
                                       struct bitimerspec const *__restrict value,
                                       struct bitimerspec *__restrict ovalue) {
 struct aitimerspec anew,aold; int result;
 if (value) {
  anew.it_interval.tv_sec  = (atime_t)value->it_interval.tv_sec;
  anew.it_interval.tv_nsec = value->it_interval.tv_nsec;
  anew.it_value.tv_sec     = (atime_t)value->it_value.tv_sec;
  anew.it_value.tv_nsec    = value->it_value.tv_nsec;
 }
 result = A(timer_settime)(timerid,flags,value ? &anew : NULL,ovalue ? &aold : NULL);
 if (!result && ovalue) {
  ovalue->it_interval.tv_sec  = (btime_t)aold.it_interval.tv_sec;
  ovalue->it_interval.tv_nsec = aold.it_interval.tv_nsec;
  ovalue->it_value.tv_sec     = (btime_t)aold.it_value.tv_sec;
  ovalue->it_value.tv_nsec    = aold.it_value.tv_nsec;
 }
 return result;
}
PUBLIC int (LIBCCALL B(timer_gettime))(timer_t timerid, struct bitimerspec *value) {
 struct aitimerspec ares; int result;
 result = A(timer_gettime)(timerid,&ares);
 if (!result) {
  value->it_interval.tv_sec  = (btime_t)ares.it_interval.tv_sec;
  value->it_interval.tv_nsec = ares.it_interval.tv_nsec;
  value->it_value.tv_sec     = (btime_t)ares.it_value.tv_sec;
  value->it_value.tv_nsec    = ares.it_value.tv_nsec;
 }
 return result;
}
PUBLIC int (LIBCCALL B(clock_nanosleep))(clockid_t clock_id, int flags,
                                         struct btimespec const *req,
                                         struct btimespec *rem) {
 struct atimespec areq,arem; int result;
 areq.tv_sec  = (atime_t)req->tv_sec;
 areq.tv_nsec = req->tv_nsec;
 result = A(nanosleep)(&areq,rem ? &arem : NULL);
 if (!result && rem) {
  rem->tv_sec  = (btime_t)arem.tv_sec;
  rem->tv_nsec = arem.tv_nsec;
 }
 return result;
}

PUBLIC int (LIBCCALL A(sigtimedwait))(sigset_t const *__restrict set, siginfo_t *__restrict info, struct atimespec const *timeout) { return FORWARD_SYSTEM_ERROR(sys_sigtimedwait(set,info,timeout,sizeof(siginfo_t))); }
PUBLIC int (LIBCCALL A(utimensat))(int fd, char const *path, struct atimespec const times[2], int flags) { return FORWARD_SYSTEM_ERROR(sys_utimensat(fd,path,times,flags)); }
PUBLIC int (LIBCCALL A(ppoll))(struct pollfd *fds, nfds_t nfds, struct atimespec const *timeout, sigset_t const *ss) { return FORWARD_SYSTEM_VALUE((int)sys_ppoll(fds,nfds,timeout,ss,sizeof(sigset_t))); }
PUBLIC int (LIBCCALL A(select))(int nfds, fd_set *__restrict readfds,
                                fd_set *__restrict writefds,
                                fd_set *__restrict exceptfds,
                                struct atimeval *__restrict timeout) {
 struct atimespec tmo;
 if (!timeout) return A(pselect)(nfds,readfds,writefds,exceptfds,NULL,NULL);
 TIMEVAL_TO_TIMESPEC(timeout,&tmo);
 return A(pselect)(nfds,readfds,writefds,exceptfds,&tmo,NULL);
}
PUBLIC int (LIBCCALL A(pselect))(int nfds, fd_set *__restrict readfds,
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
 if (E_ISERR(error)) { __set_errno(-error); return -1; }
 return error;
}



PUBLIC int (LIBCCALL B(sigtimedwait))(sigset_t const *__restrict set,
                                      siginfo_t *__restrict info,
                                      struct btimespec const *timeout) {
 struct atimespec tma;
 if (!timeout) return A(sigtimedwait)(set,info,NULL);
 tma.tv_sec  = (time64_t)timeout->tv_sec;
 tma.tv_nsec = timeout->tv_nsec;
 return A(sigtimedwait)(set,info,&tma);
}
PUBLIC int (LIBCCALL B(utimensat))(int fd, char const *path,
                                   struct btimespec const times[2],
                                   int flags) {
 struct atimespec atime[2];
 if (!times) return A(utimensat)(fd,path,NULL,flags);
 atime[0].tv_sec  = (atime_t)times[0].tv_sec;
 atime[0].tv_nsec = times[0].tv_nsec;
 atime[1].tv_sec  = (atime_t)times[1].tv_sec;
 atime[1].tv_nsec = times[1].tv_nsec;
 return A(utimensat)(fd,path,atime,flags);
}
PUBLIC int (LIBCCALL B(ppoll))(struct pollfd *fds, nfds_t nfds,
                               struct timespec const *timeout,
                               sigset_t const *ss) {
 struct atimespec tmo;
 if (!timeout) return A(ppoll)(fds,nfds,NULL,ss);
 tmo.tv_sec  = (atime_t)timeout->tv_sec;
 tmo.tv_nsec = timeout->tv_nsec;
 return A(ppoll)(fds,nfds,&tmo,ss);
}
PUBLIC int (LIBCCALL B(select))(int nfds, fd_set *__restrict readfds,
                                fd_set *__restrict writefds,
                                fd_set *__restrict exceptfds,
                                struct btimeval *__restrict timeout) {
 struct atimespec tmo;
 if (!timeout) return A(pselect)(nfds,readfds,writefds,exceptfds,NULL,NULL);
 TIMEVAL_TO_TIMESPEC(timeout,&tmo);
 return A(pselect)(nfds,readfds,writefds,exceptfds,&tmo,NULL);
}

PUBLIC int (LIBCCALL B(pselect))(int nfds, fd_set *__restrict readfds,
                                 fd_set *__restrict writefds,
                                 fd_set *__restrict exceptfds,
                                 struct btimespec const *__restrict timeout,
                                 sigset_t const *__restrict sigmask) {
 struct atimespec tmo;
 if (!timeout) return A(pselect)(nfds,readfds,writefds,exceptfds,NULL,sigmask);
 tmo.tv_sec  = (atime_t)timeout->tv_sec;
 tmo.tv_nsec = timeout->tv_nsec;
 return A(pselect)(nfds,readfds,writefds,exceptfds,&tmo,sigmask);
}

PRIVATE int (LIBCCALL A(libc_futimesat))(int fd, char const *file,
                                         struct atimeval const tvp[2],
                                         int flags) {
 struct atimespec times[2];
 TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
 TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 return A(utimensat)(fd,file,times,flags);
}
PRIVATE int (LIBCCALL B(libc_futimesat))(int fd, char const *file,
                                         struct btimeval const tvp[2],
                                         int flags) {
 struct atimespec times[2];
 TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
 TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 return A(utimensat)(fd,file,times,flags);
}
PUBLIC int (LIBCCALL A(futimes))(int fd, struct atimeval const tvp[2]) {
 struct atimespec times[2];
 TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
 TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 return A(futimens)(fd,times);
}
PUBLIC int (LIBCCALL B(futimes))(int fd, struct btimeval const tvp[2]) {
 struct atimespec times[2];
 TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
 TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 return A(futimens)(fd,times);
}
PUBLIC int (LIBCCALL A(futimens))(int fd, struct atimespec const times[2]) { return A(utimensat)(fd,NULL,times,0); }
PUBLIC int (LIBCCALL B(futimens))(int fd, struct btimespec const times[2]) { return B(utimensat)(fd,NULL,times,0); }
PUBLIC int (LIBCCALL A(utimes))(char const *file, struct atimeval const tvp[2]) { return A(libc_futimesat)(AT_FDCWD,file,tvp,AT_SYMLINK_FOLLOW); }
PUBLIC int (LIBCCALL A(lutimes))(char const *file, struct atimeval const tvp[2]) { return A(libc_futimesat)(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
PUBLIC int (LIBCCALL A(futimesat))(int fd, char const *file, struct atimeval const tvp[2]) { return A(libc_futimesat)(fd,file,tvp,AT_SYMLINK_FOLLOW); }
PUBLIC int (LIBCCALL B(utimes))(char const *file, struct btimeval const tvp[2]) { return B(libc_futimesat)(AT_FDCWD,file,tvp,AT_SYMLINK_FOLLOW); }
PUBLIC int (LIBCCALL B(lutimes))(char const *file, struct btimeval const tvp[2]) { return B(libc_futimesat)(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
PUBLIC int (LIBCCALL B(futimesat))(int fd, char const *file, struct btimeval const tvp[2]) { return B(libc_futimesat)(fd,file,tvp,AT_SYMLINK_FOLLOW); }




PUBLIC int (LIBCCALL poll)(struct pollfd *fds, nfds_t nfds, int timeout) {
 struct atimespec tmo;
 if (!timeout) return ppoll(fds,nfds,NULL,NULL);
 tmo.tv_sec  = timeout/MSEC_PER_SEC;
 tmo.tv_nsec = NSEC_PER_MSEC*(timeout%MSEC_PER_SEC);
 return A(ppoll)(fds,nfds,&tmo,NULL);
}



DEFINE_PUBLIC_ALIAS(timegm,mktime); /* ??? */
DEFINE_PUBLIC_ALIAS(timegm64,mktime64); /* ??? */

DECL_END

#endif /* !GUARD_LIBS_LIBC_TIME_C */
