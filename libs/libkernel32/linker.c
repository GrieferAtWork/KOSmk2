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
#include "thread.h"
#include <hybrid/compiler.h>
#include <stdlib.h> /* xdlopen(), etc. */
#include <malloc.h>
#include <unicode.h>

#include "linker.h"

DECL_BEGIN

/* DLL Directory access. */
INTERN WINBOOL WINAPI K32_SetDllDirectoryA(LPCSTR lpPathName) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_SetDllDirectoryW(LPCWSTR lpPathName) { NOT_IMPLEMENTED(); return FALSE; }
INTERN DWORD WINAPI K32_GetDllDirectoryA(DWORD nBufferLength, LPSTR lpBuffer) { NOT_IMPLEMENTED(); return 0; }
INTERN DWORD WINAPI K32_GetDllDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer) { NOT_IMPLEMENTED(); return 0; }




/* Library loading/unloading. */
INTERN HMODULE WINAPI
K32_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE UNUSED(hFile), DWORD dwFlags) {
 (void)dwFlags; /* TODO: Handle 'dwFlags' */
 return (HMODULE)xdlopen(lpLibFileName,0);
}
INTERN HMODULE WINAPI K32_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
 HMODULE result; char *path;
 if unlikely(!lpLibFileName) { SET_ERRNO(EINVAL); return NULL; }
 path = uni_utf16to8m(lpLibFileName);
 if unlikely(!path) return NULL;
 result = K32_LoadLibraryExA(path,hFile,dwFlags);
 free(path);
 return result;
}
INTERN HMODULE WINAPI K32_LoadLibraryA(LPCSTR lpLibFileName) { return K32_LoadLibraryExA(lpLibFileName,NULL,0); }
INTERN HMODULE WINAPI K32_LoadLibraryW(LPCWSTR lpLibFileName) { return K32_LoadLibraryExW(lpLibFileName,NULL,0); }
INTERN WINBOOL WINAPI K32_FreeLibrary(HMODULE hLibModule) { return !xdlclose(hLibModule); }
INTERN ATTR_NORETURN void WINAPI K32_FreeLibraryAndExitThread(HMODULE hLibModule, DWORD dwExitCode) { K32_FreeLibrary(hLibModule); K32_ExitThread(dwExitCode); }
INTERN WINBOOL WINAPI K32_DisableThreadLibraryCalls(HMODULE hLibModule) { NOT_IMPLEMENTED(); return FALSE; }
INTERN FARPROC WINAPI K32_GetProcAddress(HMODULE hModule, LPCSTR lpProcName) { return (FARPROC)xdlsym((void *)hModule,lpProcName); }





/* Command execution. */
INTERN UINT WINAPI K32_WinExec(LPCSTR lpCmdLine, UINT UNUSED(uCmdShow)) { return system(lpCmdLine); }




/* DLL Directory access. */
DEFINE_PUBLIC_ALIAS(SetDllDirectoryA,K32_SetDllDirectoryA);
DEFINE_PUBLIC_ALIAS(SetDllDirectoryW,K32_SetDllDirectoryW);
DEFINE_PUBLIC_ALIAS(GetDllDirectoryA,K32_GetDllDirectoryA);
DEFINE_PUBLIC_ALIAS(GetDllDirectoryW,K32_GetDllDirectoryW);

/* Library loading/unloading. */
DEFINE_PUBLIC_ALIAS(LoadLibraryA,K32_LoadLibraryA);
DEFINE_PUBLIC_ALIAS(LoadLibraryW,K32_LoadLibraryW);
DEFINE_PUBLIC_ALIAS(LoadLibraryExA,K32_LoadLibraryExA);
DEFINE_PUBLIC_ALIAS(LoadLibraryExW,K32_LoadLibraryExW);
DEFINE_PUBLIC_ALIAS(FreeLibrary,K32_FreeLibrary);
DEFINE_PUBLIC_ALIAS(FreeLibraryAndExitThread,K32_FreeLibraryAndExitThread);
DEFINE_PUBLIC_ALIAS(DisableThreadLibraryCalls,K32_DisableThreadLibraryCalls);
DEFINE_PUBLIC_ALIAS(GetProcAddress,K32_GetProcAddress);

/* Command execution. */
DEFINE_PUBLIC_ALIAS(WinExec,K32_WinExec);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_LINKER_C */
