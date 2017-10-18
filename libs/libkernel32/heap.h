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
#ifndef GUARD_LIBS_LIBKERNEL32_HEAP_H
#define GUARD_LIBS_LIBKERNEL32_HEAP_H 1

#include "k32.h"
#include <hybrid/compiler.h>

DECL_BEGIN

/* Heap API */
INTDEF HANDLE  WINAPI K32_HeapCreate(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize);
INTDEF WINBOOL WINAPI K32_HeapDestroy(HANDLE hHeap);
INTDEF LPVOID  WINAPI K32_HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
INTDEF LPVOID  WINAPI K32_HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes);
INTDEF WINBOOL WINAPI K32_HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
INTDEF SIZE_T  WINAPI K32_HeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem);
INTDEF WINBOOL WINAPI K32_HeapValidate(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem);
INTDEF SIZE_T  WINAPI K32_HeapCompact(HANDLE hHeap, DWORD dwFlags);
INTDEF HANDLE  WINAPI K32_GetProcessHeap(void);
INTDEF DWORD   WINAPI K32_GetProcessHeaps(DWORD NumberOfHeaps, PHANDLE ProcessHeaps);
INTDEF WINBOOL WINAPI K32_HeapLock(HANDLE hHeap);
INTDEF WINBOOL WINAPI K32_HeapUnlock(HANDLE hHeap);
INTDEF WINBOOL WINAPI K32_HeapWalk(HANDLE hHeap, LPPROCESS_HEAP_ENTRY lpEntry);
INTDEF WINBOOL WINAPI K32_HeapSetInformation(HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength);
INTDEF WINBOOL WINAPI K32_HeapQueryInformation(HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength, PSIZE_T ReturnLength);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_HEAP_H */
