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
#ifndef GUARD_LIBS_LIBKERNEL32_STRING_H
#define GUARD_LIBS_LIBKERNEL32_STRING_H 1

#include "k32.h"
#include <hybrid/compiler.h>

DECL_BEGIN

INTDEF CHAR  const K32_EmptyStringA[1] ASMNAME("K32_EmptyString");
INTDEF WCHAR const K32_EmptyStringW[1] ASMNAME("K32_EmptyString");
#define K32_StringOrEmptyA(x) ((x) ? (x) : K32_EmptyStringA)
#define K32_StringOrEmptyW(x) ((x) ? (x) : K32_EmptyStringW)

INTDEF int    WINAPI K32_lstrcmpA(LPCSTR lpString1, LPCSTR lpString2);
INTDEF int    WINAPI K32_lstrcmpW(LPCWSTR lpString1, LPCWSTR lpString2);
INTDEF int    WINAPI K32_lstrcmpiA(LPCSTR lpString1, LPCSTR lpString2);
INTDEF int    WINAPI K32_lstrcmpiW(LPCWSTR lpString1, LPCWSTR lpString2);
INTDEF int    WINAPI K32_lstrlenA(LPCSTR lpString);
INTDEF int    WINAPI K32_lstrlenW(LPCWSTR lpString);
INTDEF LPSTR  WINAPI K32_lstrcpynA(LPSTR lpString1, LPCSTR lpString2, int iMaxLength);
INTDEF LPWSTR WINAPI K32_lstrcpynW(LPWSTR lpString1, LPCWSTR lpString2, int iMaxLength);
INTDEF LPSTR  WINAPI K32_lstrcpyA(LPSTR lpString1, LPCSTR lpString2);
INTDEF LPWSTR WINAPI K32_lstrcpyW(LPWSTR lpString1, LPCWSTR lpString2);
INTDEF LPSTR  WINAPI K32_lstrcatA(LPSTR lpString1, LPCSTR lpString2);
INTDEF LPWSTR WINAPI K32_lstrcatW(LPWSTR lpString1, LPCWSTR lpString2);

INTDEF int WINAPI K32_CompareStringA(LCID Locale, DWORD dwCmpFlags, LPCSTR lpString1, int cchCount1, LPCSTR lpString2, int cchCount2);
INTDEF int WINAPI K32_CompareStringW(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2);

INTDEF int WINAPI K32_MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
INTDEF int WINAPI K32_WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_STRING_H */
