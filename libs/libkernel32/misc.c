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

/*

<RING #3(USER) FAULT> Unhandled Exception 0x0e (14)
<#PF> - Page Fault - ECODE 0x4 (4)
CPU #0 (D8000860; GPID 8)
EAX 00400000  ECX 10000000  EDX 00416530  EBX 00000000  EIP 004120F0
ESP 0FFFFF74  EBP 0FFFFF80  ESI 00000000  EDI 00000000  ---
EFLAGS 00000216 (IOPL(0)+PF+AF+IF)
DS 0023 (GDT:04 RPL#3 - 00000000, FFFFFFFF DPL#3 PS--WA)
ES 0023 (GDT:04 RPL#3 - 00000000, FFFFFFFF DPL#3 PS--WA)
FS 0053 (GDT:10 RPL#3 - 9FFFF008, FFFFFFFF DPL#3 PS--WA)
GS 005B (GDT:11 RPL#3 - 9FFFF000, FFFFFFFF DPL#3 PS--WA)
CS 001B (GDT:03 RPL#3 - 00000000, FFFFFFFF DPL#3 PSX-W-)
SS 0023 (GDT:04 RPL#3 - 00000000, FFFFFFFF DPL#3 PS--WA)
CR0 8001001B (PE+MP+TS+ET+WP+PG)
CR2 00400000  CR3 07DB7000
CR4 00000210 (PSE+OSFXSR)
../??(0) : ?? : [0] : 004120F0 : 0FFFFF80
../??(0) : ?? : [0] : 00411CE8 : 0FFFFF80
../libs/libc/misc.c(644) : libc_initterm_e : [1] : 20033244 : 0FFFFF88
../??(0) : ?? : [2] : 00411EC4 : 0FFFFFA8
../??(0) : ?? : [3] : 0041219D : 0FFFFFF4
00401000-00410FFF rwxp 00000000 00:00 0          
00411000-00413FFF r-xp 00000400 0e:01 247712     /bin/dos_userapp.exe
00414000-00415FFF r--p 00003400 0e:01 247712     /bin/dos_userapp.exe
00416000-00416FFF rw-p 00000000 00:00 0          
00417000-00417FFF r--p 00000000 00:00 0          
00418000-00418FFF r--p 00005400 0e:01 247712     /bin/dos_userapp.exe
0FFFC000-0FFFCFFF rw-p 00000000 00:00 0          
0FFFD000-0FFFFFFF rw-p 00000000 00:00 0          
20000000-20069FFF r-xp 00000000 0e:01 321600     /lib/libc.so
2006A000-2006AFFF rw-p 00000000 00:00 0          
2007B000-2007BFFF r-xp 00000000 00:00 0          
2007C000-2007CFFF rw-p 00000000 00:00 0          
9FFFF000-9FFFFFFF rw-p 00000000 00:00 0          
BFFFB000-BFFFBFFF rw-p 00000000 00:00 0   
*/

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
