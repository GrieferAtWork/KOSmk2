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
#ifndef GUARD_MODULES_NT_ERROR_C
#define GUARD_MODULES_NT_ERROR_C 1

#include <hybrid/compiler.h>
#include <errno.h>

#include "error.h"

#include <winapi/windows.h>
#include <winapi/windef.h>
#include <winapi/ntstatus.h>
#include <winapi/ntdef.h>

DECL_BEGIN

#define FALLBACK  STATUS_NOT_SUPPORTED

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"

PRIVATE LONG const errno_matrix[__EBASEMAX+1] = {
    [0 ... __EBASEMAX] = FALLBACK,
    [ENOMEM] = STATUS_NO_MEMORY,
    /* TODO */
};

#pragma GCC diagnostic pop

PUBLIC LONG KCALL errno_kos2nt(errno_t err) {
 if ((unsigned int)err >= COMPILER_LENOF(errno_matrix))
      return FALLBACK;
 return errno_matrix[(unsigned int)err];
}

PUBLIC LONG KCALL errno_kos2ntv(ssize_t err_or_value) {
 if (E_ISERR(err_or_value))
     return errno_kos2nt((errno_t)-err_or_value);
 return (LONG)err_or_value;
}

DECL_END

#endif /* !GUARD_MODULES_NT_ERROR_C */
