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


INTERN void WINAPI
K32_GetSystemTimeAsFileTime(LPFILETIME presult) {
 if (presult) {
  struct timeval64 val;
  gettimeofday64(&val,NULL);
  *presult = K32_TimevalToFiletime(val);
 }
}

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

__LIBC int LIBCCALL libc_beep(unsigned int freq, unsigned int duration) ASMNAME("_beep");
INTERN BOOL WINAPI K32_Beep(DWORD freq, DWORD duration) { return libc_beep((unsigned int)freq,(unsigned int)duration) ? FALSE : TRUE; }


DEFINE_PUBLIC_ALIAS(GetSystemTimeAsFileTime,K32_GetSystemTimeAsFileTime);
DEFINE_PUBLIC_ALIAS(QueryPerformanceCounter,K32_QueryPerformanceCounter);
DEFINE_PUBLIC_ALIAS(QueryPerformanceFrequency,K32_QueryPerformanceFrequency);
DEFINE_PUBLIC_ALIAS(Beep,K32_Beep);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_TIME_C */
