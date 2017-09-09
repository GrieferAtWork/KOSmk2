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
#ifndef _KOS_SYSLOG_H
#define _KOS_SYSLOG_H 1

#include <__stdinc.h>
#include <bits/types.h>

__DECL_BEGIN

enum{
 LOG_DEFAULT = 0, /* Default logging category. */
 LOG_FS,
 LOG_MEM,
 LOG_HW,
 LOG_IRQ,
 LOG_EXEC,
 LOG_TEST,
 LOG_SCHED,
 LOG_BOOT,
 LOG_IO,
 LOG_CRITICAL = 0x00000000,
 LOG_ERROR    = 0x01000000,
 LOG_WARN     = 0x02000000,
 LOG_CONFIRM  = 0x03000000,
 LOG_MESSAGE  = 0x04000000,
 LOG_INFO     = 0x05000000,
 LOG_DEBUG    = 0x06000000,
 LOG_SEVERITY = 0xff000000,
};

__LIBC __ssize_t (__ATTR_CDECL syslogf)(__uint32_t __level, char const *__format, ...);
__LIBC __ssize_t (__LIBCCALL vsyslogf)(__uint32_t __level, char const *__format, __VA_LIST args);

/* Helper functions for printing to the system log. */
__LIBC __ssize_t (__LIBCCALL syslog_printer)(char const *__restrict __data,
                                             __size_t __datalen, void *__closure);
#define SYSLOG_PRINTER_CLOSURE(level) ((void *)(uintptr_t)(__uint32_t)(level))

__DECL_END

#endif /* !_KOS_SYSLOG_H */
