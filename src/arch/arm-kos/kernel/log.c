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
#ifndef GUARD_KERNEL_ARCH_LOG_C
#define GUARD_KERNEL_ARCH_LOG_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>
#include <kernel/boot.h>
#include <kernel/export.h>
#include <kernel/paging.h>
#include <modules/tty.h>
#include <stdint.h>
#include <string.h>
#include <sys/io.h>

DECL_BEGIN

#define UART0_BASE 0x1c090000

PUBLIC vtty_char_t tty_color;
PUBLIC void KCALL tty_putc(char c) {
 volatile unsigned int *uart0 = (volatile unsigned int *)UART0_BASE;
 *uart0 = c;
}

PUBLIC void KCALL
tty_print(char const *__restrict str, size_t len) {
 char const *end = str+len;
 CHECK_HOST_TEXT(str,len);
 for (; str != end; ++str) tty_putc(*str);
}

PUBLIC ssize_t KCALL
tty_printer(char const *__restrict str, size_t len,
            void *UNUSED(closure)) {
 tty_print(str,len);
 return (ssize_t)len;
}

DECL_END

#endif /* !GUARD_KERNEL_ARCH_LOG_C */
