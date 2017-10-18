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
#ifndef GUARD_LIBS_LIBKERNEL32_TIME_H
#define GUARD_LIBS_LIBKERNEL32_TIME_H 1

#include "k32.h"
#include <hybrid/compiler.h>
#include <hybrid/timespec.h>
#include <hybrid/timeval.h>
#include <winapi/mmsystem.h>

DECL_BEGIN

/* Time conversion functions. */
INTDEF FILETIME WINAPI K32_TimespecToFiletime(struct __timespec64 val);
INTDEF FILETIME WINAPI K32_TimevalToFiletime(struct __timeval64 val);
INTDEF struct __timespec64 WINAPI K32_FiletimeToTimespec(FILETIME val);
INTDEF struct __timeval64 WINAPI K32_FiletimeToTimeval(FILETIME val);

/* Time utility functions. */
INTDEF LONG WINAPI K32_CompareFileTime(CONST FILETIME *lpFileTime1, CONST FILETIME *lpFileTime2);

/* Realtime get/set API. */
INTDEF void    WINAPI K32_GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime);
INTDEF void    WINAPI K32_GetSystemTime(LPSYSTEMTIME lpSystemTime);
INTDEF WINBOOL WINAPI K32_SetSystemTime(CONST SYSTEMTIME *lpSystemTime);
INTDEF void    WINAPI K32_GetLocalTime(LPSYSTEMTIME lpSystemTime);
INTDEF WINBOOL WINAPI K32_SetLocalTime(CONST SYSTEMTIME *lpSystemTime);
INTDEF WINBOOL WINAPI K32_GetSystemTimes(LPFILETIME lpIdleTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime);

/* High-performance tick-style query. */
INTDEF WINBOOL WINAPI K32_QueryPerformanceCounter(LARGE_INTEGER *ptick);
INTDEF WINBOOL WINAPI K32_QueryPerformanceFrequency(LARGE_INTEGER *pfreq);

/* Misc. */
INTDEF MMRESULT WINAPI K32_timeBeginPeriod(UINT uPeriod);
INTDEF MMRESULT WINAPI K32_timeEndPeriod(UINT uPeriod);
INTDEF MMRESULT WINAPI K32_timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
INTDEF MMRESULT WINAPI K32_timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);
INTDEF DWORD WINAPI K32_timeGetTime(void);


DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_TIME_H */
