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
#ifndef GUARD_LIBS_LIBKERNEL32_THREAD_C
#define GUARD_LIBS_LIBKERNEL32_THREAD_C 1
#define _KOS_SOURCE 1

#include "k32.h"
#include "thread.h"

#include <hybrid/compiler.h>
#include <unistd.h>
#include <sys/syscall.h>

DECL_BEGIN

INTERN DWORD WINAPI K32_GetCurrentThreadId(void) { return syscall(SYS_gettid); }
INTERN DWORD WINAPI K32_GetCurrentProcessId(void) { return getpid(); }
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
DEFINE_PUBLIC_ALIAS(IsProcessorFeaturePresent,K32_IsProcessorFeaturePresent);
DEFINE_PUBLIC_ALIAS(IsDebuggerPresent,K32_IsDebuggerPresent);
DEFINE_PUBLIC_ALIAS(EncodePointer,K32_EncodePointer);
DEFINE_PUBLIC_ALIAS(DecodePointer,K32_DecodePointer);
DEFINE_PUBLIC_ALIAS(EncodeSystemPointer,K32_EncodeSystemPointer);
DEFINE_PUBLIC_ALIAS(DecodeSystemPointer,K32_DecodeSystemPointer);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_THREAD_C */
