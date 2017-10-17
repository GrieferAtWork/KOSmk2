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
#ifndef GUARD_LIBS_LIBKERNEL32_MISC_C
#define GUARD_LIBS_LIBKERNEL32_MISC_C 1

#define _KOS_SOURCE 1
#define _TIME64_SOURCE 1

#include <hybrid/compiler.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <stdint.h>
#include <time.h>
#include "misc.h"

DECL_BEGIN
DEFINE_INTERN_ALIAS(__stack_chk_fail_local,__stack_chk_fail);

INTERN DWORD WINAPI K32_GetCurrentThreadId(void) { return syscall(SYS_gettid); }
INTERN DWORD WINAPI K32_GetCurrentProcessId(void) { return getpid(); }

INTERN void WINAPI
K32_GetSystemTimeAsFileTime(LPFILETIME presult) {
 if (presult) {
  struct timeval64 val;   u64 result;
  gettimeofday64(&val,NULL);
  /* Seconds between 1.1.1601 and 1.1.1970 */
  result = UINT64_C(11644473600)+val.tv_sec;
  result *= USEC_PER_SEC;
  result += val.tv_usec;
  /* FILTIME has a resolution of 100 nanoseconds. */
  result *= (NSEC_PER_SEC/USEC_PER_SEC)*100;

  *(u64 *)presult = result;
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
INTERN WINBOOL WINAPI
K32_IsProcessorFeaturePresent(DWORD feature) {
 (void)feature; /* TODO */
 return FALSE;
}
INTERN WINBOOL WINAPI K32_IsDebuggerPresent(void) { return FALSE; }
INTERN PVOID WINAPI K32_EncodePointer(PVOID p) { return p; }
DEFINE_INTERN_ALIAS(K32_DecodePointer,K32_EncodePointer);
DEFINE_INTERN_ALIAS(K32_EncodeSystemPointer,K32_EncodePointer);
DEFINE_INTERN_ALIAS(K32_DecodeSystemPointer,K32_EncodePointer);


DEFINE_PUBLIC_ALIAS(GetCurrentThreadId,K32_GetCurrentThreadId);
DEFINE_PUBLIC_ALIAS(GetCurrentProcessId,K32_GetCurrentProcessId);
DEFINE_PUBLIC_ALIAS(GetSystemTimeAsFileTime,K32_GetSystemTimeAsFileTime);
DEFINE_PUBLIC_ALIAS(QueryPerformanceCounter,K32_QueryPerformanceCounter);
DEFINE_PUBLIC_ALIAS(QueryPerformanceFrequency,K32_QueryPerformanceFrequency);
DEFINE_PUBLIC_ALIAS(IsProcessorFeaturePresent,K32_IsProcessorFeaturePresent);
DEFINE_PUBLIC_ALIAS(IsDebuggerPresent,K32_IsDebuggerPresent);
DEFINE_PUBLIC_ALIAS(EncodePointer,K32_EncodePointer);
DEFINE_PUBLIC_ALIAS(DecodePointer,K32_DecodePointer);
DEFINE_PUBLIC_ALIAS(EncodeSystemPointer,K32_EncodeSystemPointer);
DEFINE_PUBLIC_ALIAS(DecodeSystemPointer,K32_DecodeSystemPointer);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_MISC_C */
