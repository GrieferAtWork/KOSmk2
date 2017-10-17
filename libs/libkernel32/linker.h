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
#ifndef GUARD_LIBS_LIBKERNEL32_LINKER_H
#define GUARD_LIBS_LIBKERNEL32_LINKER_H 1

#include "k32.h"
#include <hybrid/compiler.h>

DECL_BEGIN

/* DLL Directory access. */
INTDEF WINBOOL WINAPI K32_SetDllDirectoryA(LPCSTR lpPathName);
INTDEF WINBOOL WINAPI K32_SetDllDirectoryW(LPCWSTR lpPathName);
INTDEF DWORD   WINAPI K32_GetDllDirectoryA(DWORD nBufferLength, LPSTR lpBuffer);
INTDEF DWORD   WINAPI K32_GetDllDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer);

/* Library loading/unloading. */
INTDEF HMODULE WINAPI K32_LoadLibraryA(LPCSTR lpLibFileName);
INTDEF HMODULE WINAPI K32_LoadLibraryW(LPCWSTR lpLibFileName);
INTDEF HMODULE WINAPI K32_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
INTDEF HMODULE WINAPI K32_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
INTDEF WINBOOL WINAPI K32_FreeLibrary(HMODULE hLibModule);
INTDEF ATTR_NORETURN void WINAPI K32_FreeLibraryAndExitThread(HMODULE hLibModule, DWORD dwExitCode);
INTDEF WINBOOL WINAPI K32_DisableThreadLibraryCalls(HMODULE hLibModule);
INTDEF FARPROC WINAPI K32_GetProcAddress(HMODULE hModule, LPCSTR lpProcName);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_LINKER_H */
