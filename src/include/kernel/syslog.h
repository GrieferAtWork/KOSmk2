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
#ifndef GUARD_INCLUDE_KERNEL_SYSLOG_H
#define GUARD_INCLUDE_KERNEL_SYSLOG_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <sys/syslog.h>
#include <format-printer.h>

DECL_BEGIN



/* Set a new global system-log printer.
 * HINT: The closure argument for the given `printer' is generated using `SYSLOG_PRINTER_CLOSURE()' */
FUNDEF pformatprinter KCALL syslog_set_printer(pformatprinter printer);

/* Builtin system log printers. */
FUNDEF ssize_t KCALL syslog_print_tty(char const *__restrict data, size_t datalen, void *closure);
FUNDEF ssize_t KCALL syslog_print_serio(char const *__restrict data, size_t datalen, void *closure);

/* The default system log printer initially set-up during boot. */
#define syslog_print_default   syslog_print_tty

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_SYSLOG_H */
