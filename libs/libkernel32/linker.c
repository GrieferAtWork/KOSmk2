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
#ifndef GUARD_LIBS_LIBKERNEL32_LINKER_C
#define GUARD_LIBS_LIBKERNEL32_LINKER_C 1
#define _KOS_SOURCE 1 /* xdlopen(), etc. */

#include "k32.h"
#include <hybrid/compiler.h>
#include <stdlib.h> /* xdlopen(), etc. */

#include "linker.h"

DECL_BEGIN

/* PWD access. */
INTERN WINBOOL WINAPI K32_SetDllDirectoryA(LPCSTR lpPathName) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_SetDllDirectoryW(LPCWSTR lpPathName) { NOT_IMPLEMENTED(); return FALSE; }
INTERN DWORD WINAPI K32_GetDllDirectoryA(DWORD nBufferLength, LPSTR lpBuffer) { NOT_IMPLEMENTED(); return 0; }
INTERN DWORD WINAPI K32_GetDllDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer) { NOT_IMPLEMENTED(); return 0; }

DEFINE_PUBLIC_ALIAS(SetDllDirectoryA,K32_SetDllDirectoryA);
DEFINE_PUBLIC_ALIAS(SetDllDirectoryW,K32_SetDllDirectoryW);
DEFINE_PUBLIC_ALIAS(GetDllDirectoryA,K32_GetDllDirectoryA);
DEFINE_PUBLIC_ALIAS(GetDllDirectoryW,K32_GetDllDirectoryW);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_LINKER_C */
