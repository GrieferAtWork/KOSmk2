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
#ifndef GUARD_LIBS_LIBKERNEL32_K32_H
#define GUARD_LIBS_LIBKERNEL32_K32_H 1

#include <hybrid/compiler.h>
#include <winapi/windows.h>
#include <winapi/ntdef.h>
#include <syslog.h>
#include <errno.h>

DECL_BEGIN

DEFINE_PRIVATE_ALIAS(__stack_chk_fail_local,__stack_chk_fail);

#define NOT_IMPLEMENTED() \
  (SET_ERRNO(ENOSYS), \
   syslog(LOG_WARNING,"%s(%d) : KERNEL32::%s : NOT_IMPLEMENTED()\n",__FILE__,__LINE__,__FUNCTION__))


DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_K32_H */
