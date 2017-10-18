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

DECL_BEGIN

/* Atomic operations. */
INTDEF u32 ATTR_CDECL K32_InterlockedCompareExchange(u32 volatile *Dest, u32 Exchange, u32 Comp);
INTDEF u64 ATTR_CDECL K32_InterlockedCompareExchange64(u64 volatile *Dest, u64 Exchange, u64 Comp);
INTDEF u32 ATTR_CDECL K32_InterlockedIncrement(u32 volatile *Dest);
INTDEF u32 ATTR_CDECL K32_InterlockedDecrement(u32 volatile *Dest);
INTDEF u32 ATTR_CDECL K32_InterlockedExchange(u32 volatile *Dest, u32 Exchange);
INTDEF u32 ATTR_CDECL K32_InterlockedExchangeAdd(u32 volatile *Dest, u32 Value);

// INTDEF PSLIST_ENTRY WINAPI   K32_InterlockedFlushSList(PSLIST_HEADER ListHead);
// INTDEF PSLIST_ENTRY WINAPI   K32_InterlockedPopEntrySList(PSLIST_HEADER ListHead);
// INTDEF PSLIST_ENTRY WINAPI   K32_InterlockedPushEntrySList(PSLIST_HEADER ListHead, PSLIST_ENTRY ListEntry);
// INTDEF PSLIST_ENTRY FASTCALL K32_InterlockedPushListSList(PSLIST_HEADER ListHead, PSLIST_ENTRY List, PSLIST_ENTRY ListEnd, ULONG Count);
// INTDEF PSLIST_ENTRY WINAPI   K32_InterlockedPushListSListEx(PSLIST_HEADER ListHead, PSLIST_ENTRY List, PSLIST_ENTRY ListEnd, ULONG Count);


/* Misc. */
INTDEF BOOL WINAPI K32_Beep(DWORD freq, DWORD duration);
INTDEF int WINAPI K32_MulDiv(int nNumber, int nNumerator, int nDenominator);


DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_MISC_H */
