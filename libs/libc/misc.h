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
#ifndef GUARD_LIBS_LIBC_MISC_H
#define GUARD_LIBS_LIBC_MISC_H 1

#include "libc.h"
#include "system.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

INTDEF void LIBCCALL libc_closelog(void);
INTDEF void LIBCCALL libc_openlog(char const *ident, int option, int facility);
INTDEF int LIBCCALL libc_setlogmask(int mask);
INTDEF ssize_t LIBCCALL libc_syslog_printer(char const *__restrict data, size_t datalen, void *closure);
INTDEF void LIBCCALL libc_vsyslog(int level, char const *format, va_list args);
#ifndef __libc_syslog_defined
#define __libc_syslog_defined 1
INTDEF void ATTR_CDECL libc_syslog(int level, char const *format, ...);
#endif /* !__libc_syslog_defined */
INTDEF int LIBCCALL libc_munmap(void *addr, size_t len);
INTDEF void *LIBCCALL libc_xmmap1(struct mmap_info const *data);
INTDEF ssize_t LIBCCALL libc_xmunmap(void *addr, size_t len, int flags, void *tag);
INTDEF void *LIBCCALL libc_xsharesym(char const *name);
INTDEF void *LIBCCALL libc_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
INTDEF void *LIBCCALL libc_mmap64(void *addr, size_t len, int prot, int flags, int fd, off64_t offset);
INTDEF void *ATTR_CDECL libc_mremap(void *addr, size_t old_len, size_t new_len, int flags, ...);
INTDEF int LIBCCALL libc_mprotect(void *addr, size_t len, int prot);
INTDEF void *LIBCCALL libc_xdlopen(char const *filename, int flags);
INTDEF void *LIBCCALL libc_xfdlopen(int fd, int flags);
INTDEF void *LIBCCALL libc_xdlsym(void *handle, char const *symbol);
INTDEF int LIBCCALL libc_xdlclose(void *handle);

DECL_END

#endif /* !GUARD_LIBS_LIBC_MISC_H */
