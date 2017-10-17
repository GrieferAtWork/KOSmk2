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
#define _UTF_SOURCE 1

#include "k32.h"
#include "string.h"
#include <string.h>
#include <uchar.h>

#include <hybrid/compiler.h>
#include <hybrid/minmax.h>

DECL_BEGIN

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

DEFINE_PUBLIC_ALIAS(CompareStringA,K32_CompareStringA);
DEFINE_PUBLIC_ALIAS(CompareStringW,K32_CompareStringW);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_STRING_C */
