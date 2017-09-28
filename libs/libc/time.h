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
#ifndef GUARD_LIBS_LIBC_TIME_H
#define GUARD_LIBS_LIBC_TIME_H 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1
#define _TIME64_SOURCE 1

#include "libc.h"
#include "system.h"

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <bits/siginfo.h>
#include <uchar.h>

DECL_BEGIN

struct tm;
struct itimerspec;
struct itimerspec64;
struct timezone;
struct sigevent;
struct timespec;
struct timespec64;
struct timeval;
struct timeval64;
struct pollfd;
struct timeb;
struct timeb64;
struct utimbuf;
struct utimbuf64;
struct tms;

INTDEF void LIBCCALL libc_tzset(void);
INTDEF char *LIBCCALL libc_asctime_r(struct tm const *__restrict tp, char *__restrict buf);
INTDEF char *LIBCCALL libc_asctime(struct tm const *tp);
INTDEF char *LIBCCALL libc_strptime(char const *__restrict s, char const *__restrict format, struct tm *tp);
INTDEF int LIBCCALL libc_dysize(int year);
INTDEF int LIBCCALL libc_getdate_r(char const *__restrict string, struct tm *__restrict resbufp);
INTDEF struct tm *LIBCCALL libc_getdate(char const *string);
INTDEF char *LIBCCALL libc_strptime_l(char const *__restrict s, char const *__restrict format, struct tm *tp, locale_t loc);
INTDEF char *LIBCCALL libc_ctime64_r(time64_t const *__restrict timer, char *__restrict buf);
INTDEF char *LIBCCALL libc_ctime_r(time_t const *__restrict timer, char *__restrict buf);
INTDEF char *LIBCCALL libc_ctime64(time64_t const *timer);
INTDEF char *LIBCCALL libc_ctime(time_t const *timer);
INTDEF time64_t LIBCCALL libc_mktime64(struct tm *tp);
INTDEF struct tm *LIBCCALL libc_gmtime64_r(time64_t const *__restrict timer, struct tm *__restrict tp);
INTDEF time_t LIBCCALL libc_mktime(struct tm *tp);
INTDEF struct tm *LIBCCALL libc_gmtime_r(time_t const *__restrict timer, struct tm *__restrict tp);
INTDEF struct tm *LIBCCALL libc_localtime_r(time_t const *__restrict timer, struct tm *__restrict tp);
INTDEF struct tm *LIBCCALL libc_localtime64_r(time64_t const *__restrict timer, struct tm *__restrict tp);
INTDEF struct tm *LIBCCALL libc_gmtime(time_t const *timer);
INTDEF struct tm *LIBCCALL libc_gmtime64(time64_t const *timer);
INTDEF struct tm *LIBCCALL libc_localtime(time_t const *timer);
INTDEF struct tm *LIBCCALL libc_localtime64(time64_t const *timer);
INTDEF double LIBCCALL libc_difftime(time_t time1, time_t time0);
INTDEF double LIBCCALL libc_difftime64(time64_t time1, time64_t time0);

/* Kernel-time independent system-call functions */
INTDEF clock_t LIBCCALL libc_clock(void);
INTDEF int LIBCCALL libc_timer_create(clockid_t clock_id, struct sigevent *__restrict evp, timer_t *__restrict timerid);
INTDEF int LIBCCALL libc_timer_delete(timer_t timerid);
INTDEF int LIBCCALL libc_timer_getoverrun(timer_t timerid);
INTDEF int LIBCCALL libc_clock_getcpuclockid(pid_t pid, clockid_t *clock_id);
INTDEF int LIBCCALL libc_timespec_get(struct timespec *ts, int base);

INTDEF time64_t LIBCCALL libc_time64(time64_t *timer);
INTDEF int LIBCCALL libc_stime64(time64_t const *when);
INTDEF int LIBCCALL libc_gettimeofday64(struct timeval64 *__restrict tv, __timezone_ptr_t tz);
INTDEF int LIBCCALL libc_settimeofday64(struct timeval64 const *tv, struct timezone const *tz);
INTDEF int LIBCCALL libc_nanosleep64(struct timespec64 const *requested_time, struct timespec64 *remaining);
INTDEF time64_t LIBCCALL libc_timelocal64(struct tm *tp);
INTDEF int LIBCCALL libc_adjtime64(struct timeval64 const *delta, struct timeval64 *olddelta);
INTDEF int LIBCCALL libc_getitimer64(__itimer_which_t which, struct itimerval64 *value);
INTDEF int LIBCCALL libc_setitimer64(__itimer_which_t which, const struct itimerval64 *__restrict new_, struct itimerval64 *__restrict old);
INTDEF int LIBCCALL libc_clock_getres64(clockid_t clock_id, struct timespec64 *res);
INTDEF int LIBCCALL libc_clock_gettime64(clockid_t clock_id, struct timespec64 *tp);
INTDEF int LIBCCALL libc_clock_settime64(clockid_t clock_id, struct timespec64 const *tp);
INTDEF int LIBCCALL libc_timer_settime64(timer_t timerid, int flags, struct itimerspec64 const *__restrict value, struct itimerspec64 *__restrict ovalue);
INTDEF int LIBCCALL libc_timer_gettime64(timer_t timerid, struct itimerspec64 *value);
INTDEF int LIBCCALL libc_clock_nanosleep64(clockid_t clock_id, int flags, struct timespec64 const *req, struct timespec64 *rem);
INTDEF int LIBCCALL libc_sigtimedwait64(sigset_t const *__restrict set, siginfo_t *__restrict info, struct timespec64 const *timeout);
INTDEF int LIBCCALL libc_ppoll64(struct pollfd *fds, nfds_t nfds, struct timespec64 const *timeout, sigset_t const *ss);
INTDEF int LIBCCALL libc_select64(int nfds, fd_set *__restrict readfds, fd_set *__restrict writefds, fd_set *__restrict exceptfds, struct timeval64 *__restrict timeout);
INTDEF int LIBCCALL libc_pselect64(int nfds, fd_set *__restrict readfds, fd_set *__restrict writefds, fd_set *__restrict exceptfds, struct timespec64 const *__restrict timeout, sigset_t const *__restrict sigmask);
INTDEF int LIBCCALL libc_futimes64(int fd, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_futimens64(int fd, struct timespec64 const times[2]);
INTDEF int LIBCCALL libc_ftime64(struct timeb64 *timebuf);
INTDEF time_t LIBCCALL libc_timelocal(struct tm *tp);
INTDEF time_t LIBCCALL libc_time(time_t *timer);
INTDEF int LIBCCALL libc_stime(time_t const *when);
INTDEF int LIBCCALL libc_gettimeofday(struct timeval *__restrict tv, __timezone_ptr_t tz);
INTDEF int LIBCCALL libc_settimeofday(struct timeval const *tv, struct timezone const *tz);
INTDEF int LIBCCALL libc_adjtime(struct timeval const *delta, struct timeval *olddelta);
INTDEF int LIBCCALL libc_getitimer(__itimer_which_t which, struct itimerval *value);
INTDEF int LIBCCALL libc_setitimer(__itimer_which_t which, struct itimerval const *__restrict new_, struct itimerval *__restrict old);
INTDEF int LIBCCALL libc_nanosleep(struct timespec const *requested_time, struct timespec *remaining);
INTDEF int LIBCCALL libc_clock_getres(clockid_t clock_id, struct timespec *res);
INTDEF int LIBCCALL libc_clock_gettime(clockid_t clock_id, struct timespec *tp);
INTDEF int LIBCCALL libc_clock_settime(clockid_t clock_id, struct timespec const *tp);
INTDEF int LIBCCALL libc_timer_settime(timer_t timerid, int flags, struct itimerspec const *__restrict value, struct itimerspec *__restrict ovalue);
INTDEF int LIBCCALL libc_timer_gettime(timer_t timerid, struct itimerspec *value);
INTDEF int LIBCCALL libc_clock_nanosleep(clockid_t clock_id, int flags, struct timespec const *req, struct timespec *rem);
INTDEF int LIBCCALL libc_sigtimedwait(sigset_t const *__restrict set, siginfo_t *__restrict info, struct timespec const *timeout);
INTDEF int LIBCCALL libc_ppoll(struct pollfd *fds, nfds_t nfds, struct timespec const *timeout, sigset_t const *ss);
INTDEF int LIBCCALL libc_select(int nfds, fd_set *__restrict readfds, fd_set *__restrict writefds, fd_set *__restrict exceptfds, struct timeval *__restrict timeout);
INTDEF int LIBCCALL libc_pselect(int nfds, fd_set *__restrict readfds, fd_set *__restrict writefds, fd_set *__restrict exceptfds, struct timespec const *__restrict timeout, sigset_t const *__restrict sigmask);
INTDEF int LIBCCALL libc_futimes(int fd, struct timeval const tvp[2]);
INTDEF int LIBCCALL libc_futimens(int fd, struct timespec const times[2]);
INTDEF int LIBCCALL libc_ftime(struct timeb *timebuf);

INTDEF int LIBCCALL libc_futimeat64(int dfd, char const *file, struct utimbuf64 const *file_times, int flags);
INTDEF int LIBCCALL libc_utimensat64(int fd, char const *path, struct timespec64 const times[2], int flags);
INTDEF int LIBCCALL libc_utime64(char const *file, struct utimbuf64 const *file_times);
INTDEF int LIBCCALL libc_utimes64(char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_lutimes64(char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_futimesat64(int fd, char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_futimeat(int dfd, char const *file, struct utimbuf const *file_times, int flags);
INTDEF int LIBCCALL libc_utimensat(int fd, char const *path, struct timespec const times[2], int flags);
INTDEF int LIBCCALL libc_utime(char const *file, struct utimbuf const *file_times);
INTDEF int LIBCCALL libc_utimes(char const *file, struct timeval const tvp[2]);
INTDEF int LIBCCALL libc_lutimes(char const *file, struct timeval const tvp[2]);
INTDEF int LIBCCALL libc_futimesat(int fd, char const *file, struct timeval const tvp[2]);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF int LIBCCALL libc_dos_utimensat64(int fd, char const *path, struct timespec64 const times[2], int flags);
INTDEF int LIBCCALL libc_dos_utime64(char const *file, struct utimbuf64 const *file_times);
INTDEF int LIBCCALL libc_dos_utimes64(char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_lutimes64(char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_futimesat64(int fd, char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_utimensat(int fd, char const *path, struct timespec const times[2], int flags);
INTDEF int LIBCCALL libc_dos_utime(char const *file, struct utimbuf const *file_times);
INTDEF int LIBCCALL libc_dos_utimes(char const *file, struct timeval const tvp[2]);
INTDEF int LIBCCALL libc_dos_lutimes(char const *file, struct timeval const tvp[2]);
INTDEF int LIBCCALL libc_dos_futimesat(int fd, char const *file, struct timeval const tvp[2]);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

INTDEF useconds_t LIBCCALL libc_ualarm(useconds_t value, useconds_t interval);
INTDEF unsigned int LIBCCALL libc_alarm(unsigned int seconds);
INTDEF int LIBCCALL libc_pause(void);
INTDEF int LIBCCALL libc_poll(struct pollfd *fds, nfds_t nfds, int timeout);
INTDEF unsigned int LIBCCALL libc_sleep(unsigned int seconds);
INTDEF int LIBCCALL libc_usleep(useconds_t useconds);
INTDEF clock_t LIBCCALL libc_times(struct tms *buffer);

INTDEF size_t LIBCCALL libc_strftime(char *__restrict s, size_t maxsize, char const *__restrict format, struct tm const *__restrict tp);
INTDEF size_t LIBCCALL libc_strftime_l(char *__restrict s, size_t maxsize, char const *__restrict format, struct tm const *__restrict tp, locale_t loc);
INTDEF size_t LIBCCALL libc_32wcsftime(char32_t *__restrict s, size_t maxsize, char32_t const *__restrict format, struct tm const *__restrict tp);
INTDEF size_t LIBCCALL libc_32wcsftime_l(char32_t *__restrict s, size_t maxsize, char32_t const *__restrict format, struct tm const *__restrict tp, locale_t loc);
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF size_t LIBCCALL libc_16wcsftime(char16_t *__restrict s, size_t maxsize, char16_t const *__restrict format, struct tm const *__restrict tp);
INTDEF size_t LIBCCALL libc_16wcsftime_l(char16_t *__restrict s, size_t maxsize, char16_t const *__restrict format, struct tm const *__restrict tp, locale_t loc);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_TIME_H */
