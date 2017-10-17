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
#ifndef GUARD_LIBS_LIBKERNEL32_MISC_H
#define GUARD_LIBS_LIBKERNEL32_MISC_H 1

#include "k32.h"
#include <hybrid/compiler.h>
#include <hybrid/timespec.h>
#include <hybrid/timeval.h>

DECL_BEGIN

INTDEF FILETIME WINAPI K32_TimespecToFiletime(struct __timespec64 val);
INTDEF FILETIME WINAPI K32_TimevalToFiletime(struct __timeval64 val);
INTDEF struct __timespec64 WINAPI K32_FiletimeToTimespec(FILETIME val);
INTDEF struct __timeval64 WINAPI K32_FiletimeToTimeval(FILETIME val);

INTDEF void WINAPI K32_GetSystemTimeAsFileTime(LPFILETIME presult);
INTDEF WINBOOL WINAPI K32_QueryPerformanceCounter(LARGE_INTEGER *ptick);
INTDEF WINBOOL WINAPI K32_QueryPerformanceFrequency(LARGE_INTEGER *pfreq);
INTDEF BOOL WINAPI K32_Beep(DWORD freq, DWORD duration);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_MISC_H */
