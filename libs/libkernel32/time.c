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
#ifndef GUARD_LIBS_LIBKERNEL32_TIME_C
#define GUARD_LIBS_LIBKERNEL32_TIME_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1
#define _TIME64_SOURCE 1

#include "k32.h"
#include "time.h"

#include <hybrid/compiler.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <stdint.h>
#include <time.h>

DECL_BEGIN

__LIBC int LIBCCALL libc_beep(unsigned int freq, unsigned int duration) ASMNAME("_beep");


/* Time conversion functions. */
INTERN FILETIME WINAPI
K32_TimespecToFiletime(struct __timespec64 val) {
 FILETIME result;
 /* Seconds between 1.1.1601 and 1.1.1970 */
 *(u64 *)&result  = UINT64_C(11644473600)+val.tv_sec;
 *(u64 *)&result *= 10000000l;
 *(u64 *)&result += val.tv_nsec/(NSEC_PER_SEC/10000000l);
 return result;
}

INTERN struct __timespec64 WINAPI
K32_FiletimeToTimespec(FILETIME val) {
 struct __timespec64 result;
 /* Seconds between 1.1.1601 and 1.1.1970 */
 result.tv_sec  = (*(u64 *)&val/10000000l)-UINT64_C(11644473600);
 result.tv_nsec = (*(u64 *)&val % 10000000l)/(NSEC_PER_SEC/10000000l);
 return result;
}

INTERN FILETIME WINAPI
K32_TimevalToFiletime(struct __timeval64 val) {
 struct __timespec64 tmval;
 TIMEVAL_TO_TIMESPEC(&val,&tmval);
 return K32_TimespecToFiletime(tmval);
}
INTERN struct __timeval64 WINAPI
K32_FiletimeToTimeval(FILETIME val) {
 struct __timeval64 res;
 struct __timespec64 tmres = K32_FiletimeToTimespec(val);
 TIMESPEC_TO_TIMEVAL(&res,&tmres);
 return res;
}






/* Time utility functions. */
INTERN LONG WINAPI
K32_CompareFileTime(CONST FILETIME *lpFileTime1,
                    CONST FILETIME *lpFileTime2) {
 s64 a = lpFileTime1 ? *(s64 *)lpFileTime1 : 0;
 s64 b = lpFileTime2 ? *(s64 *)lpFileTime2 : 0;
 if (a < b) return -1;
 if (a > b) return 1;
 return 0;
}






/* Realtime get/set API. */
INTERN void WINAPI
K32_GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime) {
 struct timeval64 val;
 if (gettimeofday64(&val,NULL))
     memset(&val,0,sizeof(struct timeval64));
 *lpSystemTimeAsFileTime = K32_TimevalToFiletime(val);
}
INTERN void WINAPI
K32_GetSystemTimeFromTm(LPSYSTEMTIME lpSystemTime,
                        struct tm const *__restrict tmValue) {
 lpSystemTime->wYear      = (WORD)tmValue->tm_year;
 lpSystemTime->wMonth     = (WORD)tmValue->tm_mon;
 lpSystemTime->wDayOfWeek = (WORD)tmValue->tm_wday;
 lpSystemTime->wDay       = (WORD)tmValue->tm_mday;
 lpSystemTime->wHour      = (WORD)tmValue->tm_hour;
 lpSystemTime->wMinute    = (WORD)tmValue->tm_min;
 lpSystemTime->wSecond    = (WORD)tmValue->tm_sec;
}
INTERN void WINAPI
K32_GetTmFromSystemTime(struct tm *__restrict tmValue,
                        CONST SYSTEMTIME *lpSystemTime) {
 tmValue->tm_year = (int)lpSystemTime->wYear;
 tmValue->tm_mon  = (int)lpSystemTime->wMonth;
 tmValue->tm_wday = (int)lpSystemTime->wDayOfWeek;
 tmValue->tm_mday = (int)lpSystemTime->wDay;
 tmValue->tm_hour = (int)lpSystemTime->wHour;
 tmValue->tm_min  = (int)lpSystemTime->wMinute;
 tmValue->tm_sec  = (int)lpSystemTime->wSecond;
}
INTERN void WINAPI K32_GetSystemTime(LPSYSTEMTIME lpSystemTime) {
 struct timeval64 tmv; struct tm tmm;
 if (gettimeofday64(&tmv,NULL))
     memset(&tmv,0,sizeof(struct timeval64));
 if (!gmtime64_r(&tmv.tv_sec,&tmm))
     memset(&tmm,0,sizeof(struct tm));
 K32_GetSystemTimeFromTm(lpSystemTime,&tmm);
 lpSystemTime->wMilliseconds = tmv.tv_usec/USEC_PER_MSEC;
}
INTERN void WINAPI K32_GetLocalTime(LPSYSTEMTIME lpSystemTime) {
 struct timeval64 tmv; struct tm tmm;
 if (gettimeofday64(&tmv,NULL))
     memset(&tmv,0,sizeof(struct timeval64));
 if (!localtime64_r(&tmv.tv_sec,&tmm))
     memset(&tmm,0,sizeof(struct tm));
 K32_GetSystemTimeFromTm(lpSystemTime,&tmm);
 lpSystemTime->wMilliseconds = tmv.tv_usec/USEC_PER_MSEC;
}
INTERN WINBOOL WINAPI
K32_SetSystemTime(CONST SYSTEMTIME *lpSystemTime) {
 struct timeval64 tmv; struct tm tmm;
 if unlikely(!lpSystemTime) { SET_ERRNO(EINVAL); return FALSE; }
 K32_GetTmFromSystemTime(&tmm,lpSystemTime);
 tmv.tv_sec  = timegm64(&tmm);
 tmv.tv_usec = lpSystemTime->wMilliseconds*USEC_PER_MSEC;
 return !settimeofday64(&tmv,NULL);
}
INTERN WINBOOL WINAPI
K32_SetLocalTime(CONST SYSTEMTIME *lpSystemTime) {
 struct timeval64 tmv; struct tm tmm;
 if unlikely(!lpSystemTime) { SET_ERRNO(EINVAL); return FALSE; }
 K32_GetTmFromSystemTime(&tmm,lpSystemTime);
 tmv.tv_sec  = timelocal64(&tmm);
 tmv.tv_usec = lpSystemTime->wMilliseconds*USEC_PER_MSEC;
 return !settimeofday64(&tmv,NULL);
}
INTERN WINBOOL WINAPI
K32_GetSystemTimes(LPFILETIME lpIdleTime,
                   LPFILETIME lpKernelTime,
                   LPFILETIME lpUserTime) {
 NOT_IMPLEMENTED();
 return FALSE;
}




/* High-performance tick-style query. */
INTERN WINBOOL WINAPI
K32_QueryPerformanceCounter(LARGE_INTEGER *ptick) {
 struct timeval64 now;
 if (gettimeofday64(&now,NULL)) return FALSE;
 if (ptick) ptick->QuadPart = (now.tv_sec*USEC_PER_SEC)+now.tv_usec;
 return TRUE;
}
INTERN WINBOOL WINAPI
K32_QueryPerformanceFrequency(LARGE_INTEGER *pfreq) {
 if (pfreq) pfreq->QuadPart = USEC_PER_SEC;
 return TRUE;
}




/* Misc. */
INTERN BOOL WINAPI K32_Beep(DWORD freq, DWORD duration) {
 return !libc_beep((unsigned int)freq,(unsigned int)duration);
}
INTERN int WINAPI
K32_MulDiv(int nNumber, int nNumerator, int nDenominator) {
 return (int)(((s64)nNumber*(s64)nNumerator)/nDenominator);
}





/* Time utility functions. */
DEFINE_PUBLIC_ALIAS(CompareFileTime,K32_CompareFileTime);

/* Realtime get/set API. */
DEFINE_PUBLIC_ALIAS(GetSystemTimeAsFileTime,K32_GetSystemTimeAsFileTime);
DEFINE_PUBLIC_ALIAS(GetSystemTime,K32_GetSystemTime);
DEFINE_PUBLIC_ALIAS(SetSystemTime,K32_SetSystemTime);
DEFINE_PUBLIC_ALIAS(GetLocalTime,K32_GetLocalTime);
DEFINE_PUBLIC_ALIAS(SetLocalTime,K32_SetLocalTime);
DEFINE_PUBLIC_ALIAS(GetSystemTimes,K32_GetSystemTimes);

/* High-performance tick-style query. */
DEFINE_PUBLIC_ALIAS(QueryPerformanceCounter,K32_QueryPerformanceCounter);
DEFINE_PUBLIC_ALIAS(QueryPerformanceFrequency,K32_QueryPerformanceFrequency);

/* Misc. */
DEFINE_PUBLIC_ALIAS(Beep,K32_Beep);
DEFINE_PUBLIC_ALIAS(MulDiv,K32_MulDiv);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_TIME_C */
