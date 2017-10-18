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
#ifndef GUARD_LIBS_LIBKERNEL32_STRING_C
#define GUARD_LIBS_LIBKERNEL32_STRING_C 1
#define _KOS_SOURCE 1
#define _UTF_SOURCE 1

#include "k32.h"
#include "string.h"
#include <string.h>
#include <uchar.h>
#include <unicode.h>

#include <hybrid/compiler.h>
#include <hybrid/minmax.h>

DECL_BEGIN

INTERN WCHAR const K32_EmptyString[1] = {0};

INTERN int WINAPI K32_lstrcmpA(LPCSTR lpString1, LPCSTR lpString2) { return strcmp(K32_StringOrEmptyA(lpString1),K32_StringOrEmptyA(lpString2)); }
INTERN int WINAPI K32_lstrcmpW(LPCWSTR lpString1, LPCWSTR lpString2) { return c16cmp(K32_StringOrEmptyW(lpString1),K32_StringOrEmptyW(lpString2)); }
INTERN int WINAPI K32_lstrcmpiA(LPCSTR lpString1, LPCSTR lpString2) { return strcasecmp(K32_StringOrEmptyA(lpString1),K32_StringOrEmptyA(lpString2)); }
INTERN int WINAPI K32_lstrcmpiW(LPCWSTR lpString1, LPCWSTR lpString2) { return c16casecmp(K32_StringOrEmptyW(lpString1),K32_StringOrEmptyW(lpString2)); }
INTERN int WINAPI K32_lstrlenA(LPCSTR lpString) { return lpString ? strlen(lpString) : 0; }
INTERN int WINAPI K32_lstrlenW(LPCWSTR lpString) { return lpString ? c16len(lpString) : 0; }
INTERN LPSTR  WINAPI K32_lstrcpynA(LPSTR lpString1, LPCSTR lpString2, int iMaxLength) { return lpString1 ? strncpy(lpString1,K32_StringOrEmptyA(lpString2),(size_t)iMaxLength) : NULL; }
INTERN LPWSTR WINAPI K32_lstrcpynW(LPWSTR lpString1, LPCWSTR lpString2, int iMaxLength) { return lpString1 ? c16ncpy(lpString1,K32_StringOrEmptyW(lpString2),(size_t)iMaxLength) : NULL; }
INTERN LPSTR  WINAPI K32_lstrcpyA(LPSTR lpString1, LPCSTR lpString2) { return lpString1 ? strcpy(lpString1,K32_StringOrEmptyA(lpString2)) : NULL; }
INTERN LPWSTR WINAPI K32_lstrcpyW(LPWSTR lpString1, LPCWSTR lpString2) { return lpString1 ? c16cpy(lpString1,K32_StringOrEmptyW(lpString2)) : NULL; }
INTERN LPSTR  WINAPI K32_lstrcatA(LPSTR lpString1, LPCSTR lpString2) { return lpString1 ? strcat(lpString1,K32_StringOrEmptyA(lpString2)) : NULL; }
INTERN LPWSTR WINAPI K32_lstrcatW(LPWSTR lpString1, LPCWSTR lpString2) { return lpString1 ? c16cat(lpString1,K32_StringOrEmptyW(lpString2)) : NULL; }

INTERN int WINAPI
K32_CompareStringA(LCID UNUSED(Locale), DWORD UNUSED(dwCmpFlags),
                   LPCSTR lpString1, int cchCount1,
                   LPCSTR lpString2, int cchCount2) {
 return strncmp(lpString1,lpString2,
                MIN((unsigned int)cchCount1,
                    (unsigned int)cchCount2));
}

INTERN int WINAPI
K32_CompareStringW(LCID UNUSED(Locale), DWORD UNUSED(dwCmpFlags),
                   LPCWSTR lpString1, int cchCount1,
                   LPCWSTR lpString2, int cchCount2) {
 return c16ncmp(lpString1,lpString2,
                MIN((unsigned int)cchCount1,
                    (unsigned int)cchCount2));

}


INTERN int WINAPI
K32_MultiByteToWideChar(UINT UNUSED(CodePage), DWORD dwFlags,
                        LPCSTR lpMultiByteStr, int cbMultiByte,
                        LPWSTR lpWideCharStr, int cchWideChar) {
 mbstate_t state = MBSTATE_INIT; size_t result;
 size_t count = (size_t)cbMultiByte;
 if (!lpWideCharStr || !cchWideChar) goto erange;
 if (!lpMultiByteStr && count) goto einval;
 result = uni_utf8to16((char *)&lpMultiByteStr,
                       (size_t)&count,lpWideCharStr,cchWideChar,
                      &state,(dwFlags&MB_ERR_INVALID_CHARS) ?
                       UNICODE_F_SETERRNO|UNICODE_F_UPDATESRC :
                       UNICODE_F_NOFAIL|UNICODE_F_UPDATESRC);
 if (result == UNICODE_ERROR) return 0;
 if (count) goto erange;
 return result;
einval: SET_ERRNO(EINVAL); return 0;
erange: SET_ERRNO(ERANGE); return 0;
}
INTERN int WINAPI
K32_WideCharToMultiByte(UINT UNUSED(CodePage), DWORD dwFlags,
                        LPCWSTR lpWideCharStr, int cchWideChar,
                        LPSTR lpMultiByteStr, int cbMultiByte,
                        LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar) {
 mbstate_t state = MBSTATE_INIT; size_t result;
 size_t count = (size_t)cchWideChar;
 if (!lpWideCharStr && count) goto einval;
 if (!lpMultiByteStr || !cbMultiByte) goto erange;
 result = uni_utf16to8((char16_t *)&lpWideCharStr,
                       (size_t)&count,lpMultiByteStr,count,
                      &state,(dwFlags&MB_ERR_INVALID_CHARS) ?
                       UNICODE_F_SETERRNO|UNICODE_F_UPDATESRC :
                       UNICODE_F_NOFAIL|UNICODE_F_UPDATESRC);
 if (result == UNICODE_ERROR) return 0;
 if (count) goto erange;
 return result;
einval: SET_ERRNO(EINVAL); return 0;
erange: SET_ERRNO(ERANGE); return 0;
}


#undef lstrcmp
#undef lstrcmpi
#undef lstrlen
#undef lstrcpyn
#undef lstrcpy
#undef lstrcat
DEFINE_PUBLIC_ALIAS(lstrcmp,K32_lstrcmpA);
DEFINE_PUBLIC_ALIAS(lstrcmpi,K32_lstrcmpiA);
DEFINE_PUBLIC_ALIAS(lstrlen,K32_lstrlenA);
DEFINE_PUBLIC_ALIAS(lstrcpyn,K32_lstrcpynA);
DEFINE_PUBLIC_ALIAS(lstrcpy,K32_lstrcpyA);
DEFINE_PUBLIC_ALIAS(lstrcat,K32_lstrcatA);

DEFINE_PUBLIC_ALIAS(lstrcmpA,K32_lstrcmpA);
DEFINE_PUBLIC_ALIAS(lstrcmpW,K32_lstrcmpW);
DEFINE_PUBLIC_ALIAS(lstrcmpiA,K32_lstrcmpiA);
DEFINE_PUBLIC_ALIAS(lstrcmpiW,K32_lstrcmpiW);
DEFINE_PUBLIC_ALIAS(lstrlenA,K32_lstrlenA);
DEFINE_PUBLIC_ALIAS(lstrlenW,K32_lstrlenW);
DEFINE_PUBLIC_ALIAS(lstrcpynA,K32_lstrcpynA);
DEFINE_PUBLIC_ALIAS(lstrcpynW,K32_lstrcpynW);
DEFINE_PUBLIC_ALIAS(lstrcpyA,K32_lstrcpyA);
DEFINE_PUBLIC_ALIAS(lstrcpyW,K32_lstrcpyW);
DEFINE_PUBLIC_ALIAS(lstrcatA,K32_lstrcatA);
DEFINE_PUBLIC_ALIAS(lstrcatW,K32_lstrcatW);
DEFINE_PUBLIC_ALIAS(CompareStringA,K32_CompareStringA);
DEFINE_PUBLIC_ALIAS(CompareStringW,K32_CompareStringW);

DEFINE_PUBLIC_ALIAS(MultiByteToWideChar,K32_MultiByteToWideChar);
DEFINE_PUBLIC_ALIAS(WideCharToMultiByte,K32_WideCharToMultiByte);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_STRING_C */
