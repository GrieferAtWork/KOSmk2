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
#ifndef GUARD_LIBS_LIBC_ASSERT_H
#define GUARD_LIBS_LIBC_ASSERT_H 1
#define _KOS_SOURCE 2

#include "libc.h"
#include <assert.h>
#include <hybrid/debuginfo.h>
#include <stdarg.h>

DECL_BEGIN

INTDEF void ATTR_CDECL libc_debug_printf(char const *__restrict format, ...);
INTDEF void LIBCCALL libc_debug_vprintf(char const *__restrict format, __builtin_va_list args);
INTDEF ssize_t LIBCCALL libc_debug_print(char const *__restrict data, size_t datalen, void *ignored_closure);
INTDEF void LIBCCALL libc_debug_tbprint2(void const *ebp, size_t n_skip);
INTDEF void LIBCCALL libc_debug_tbprintl(void const *ip, void const *frame, size_t tb_id);
INTDEF void LIBCCALL libc_debug_tbprint(void);

INTDEF ATTR_NORETURN ATTR_NOINLINE void LIBCCALL libc_assertion_corefail(char const *expr, DEBUGINFO, char const *format, va_list args);
INTDEF ATTR_NORETURN ATTR_NOINLINE void LIBCCALL libc___assertion_unreachable(void);
INTDEF ATTR_NORETURN ATTR_NOINLINE void LIBCCALL libc___assertion_failed(char const *expr, DEBUGINFO);
INTDEF ATTR_NORETURN ATTR_NOINLINE void ATTR_CDECL libc___assertion_failedf(char const *expr, DEBUGINFO, char const *format, ...);

DATDEF uintptr_t __stack_chk_guard;
FUNDEF ATTR_NORETURN ATTR_NOINLINE void __stack_chk_fail(void);
FUNDEF ATTR_NORETURN ATTR_NOINLINE void __stack_chk_fail_local(void);

DECL_END

#endif /* !GUARD_LIBS_LIBC_ASSERT_H */
