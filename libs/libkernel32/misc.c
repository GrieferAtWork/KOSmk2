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

#include "k32.h"
#include <hybrid/compiler.h>
#include <hybrid/atomic.h>

DECL_BEGIN

__LIBC int LIBCCALL libc_beep(unsigned int freq, unsigned int duration) ASMNAME("_beep");


/* Atomic operations. */
INTERN u32 ATTR_CDECL K32_InterlockedCompareExchange(u32 volatile *Dest, u32 Exchange, u32 Comp) { return ATOMIC_CMPXCH_VAL(*(u32 *)Dest,Exchange,Comp); }
INTERN u64 ATTR_CDECL K32_InterlockedCompareExchange64(u64 volatile *Dest, u64 Exchange, u64 Comp) { return ATOMIC_CMPXCH_VAL(*(u64 *)Dest,Exchange,Comp); }
INTERN u32 ATTR_CDECL K32_InterlockedIncrement(u32 volatile *Dest) { return ATOMIC_FETCHINC(*(u32 *)Dest); }
INTERN u32 ATTR_CDECL K32_InterlockedDecrement(u32 volatile *Dest) { return ATOMIC_FETCHDEC(*(u32 *)Dest); }
INTERN u32 ATTR_CDECL K32_InterlockedExchange(u32 volatile *Dest, u32 Exchange) { return ATOMIC_XCH(*(u32 *)Dest,Exchange); }
INTERN u32 ATTR_CDECL K32_InterlockedExchangeAdd(u32 volatile *Dest, u32 Value) { return ATOMIC_FETCHADD(*(u32 *)Dest,Value); }

// INTERN PSLIST_ENTRY WINAPI K32_InterlockedFlushSList(PSLIST_HEADER ListHead);
// INTERN PSLIST_ENTRY WINAPI K32_InterlockedPopEntrySList(PSLIST_HEADER ListHead);
// INTERN PSLIST_ENTRY WINAPI K32_InterlockedPushEntrySList(PSLIST_HEADER ListHead, PSLIST_ENTRY ListEntry);
// INTERN PSLIST_ENTRY FASTCALL K32_InterlockedPushListSList(PSLIST_HEADER ListHead, PSLIST_ENTRY List, PSLIST_ENTRY ListEnd, ULONG Count);
// INTERN PSLIST_ENTRY WINAPI K32_InterlockedPushListSListEx(PSLIST_HEADER ListHead, PSLIST_ENTRY List, PSLIST_ENTRY ListEnd, ULONG Count);




/* Misc. */
INTERN BOOL WINAPI K32_Beep(DWORD freq, DWORD duration) {
 return !libc_beep((unsigned int)freq,(unsigned int)duration);
}
INTERN int WINAPI
K32_MulDiv(int nNumber, int nNumerator, int nDenominator) {
 return (int)(((s64)nNumber*(s64)nNumerator)/nDenominator);
}




/* Atomic operations. */
DEFINE_PUBLIC_ALIAS(InterlockedCompareExchange,K32_InterlockedCompareExchange);
DEFINE_PUBLIC_ALIAS(InterlockedCompareExchange64,K32_InterlockedCompareExchange64);
DEFINE_PUBLIC_ALIAS(InterlockedIncrement,K32_InterlockedIncrement);
DEFINE_PUBLIC_ALIAS(InterlockedDecrement,K32_InterlockedDecrement);
DEFINE_PUBLIC_ALIAS(InterlockedExchange,K32_InterlockedExchange);
DEFINE_PUBLIC_ALIAS(InterlockedExchangeAdd,K32_InterlockedExchangeAdd);

/* Misc. */
DEFINE_PUBLIC_ALIAS(Beep,K32_Beep);
DEFINE_PUBLIC_ALIAS(MulDiv,K32_MulDiv);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_MISC_C */
