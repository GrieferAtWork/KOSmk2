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
#ifndef GUARD_LIBS_LIBKERNEL32_THREAD_H
#define GUARD_LIBS_LIBKERNEL32_THREAD_H 1

#include "k32.h"
#include <hybrid/compiler.h>

DECL_BEGIN

INTDEF DWORD WINAPI K32_GetCurrentThreadId(void);
INTDEF DWORD WINAPI K32_GetCurrentProcessId(void);
INTDEF WINBOOL WINAPI K32_IsProcessorFeaturePresent(DWORD feature);
INTDEF WINBOOL WINAPI K32_IsDebuggerPresent(void);
INTDEF PVOID WINAPI K32_EncodePointer(PVOID p);
INTDEF PVOID WINAPI K32_DecodePointer(PVOID p);
INTDEF PVOID WINAPI K32_EncodeSystemPointer(PVOID p);
INTDEF PVOID WINAPI K32_DecodeSystemPointer(PVOID p);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_THREAD_H */
