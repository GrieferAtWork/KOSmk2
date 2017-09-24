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
#ifndef GUARD_LIBS_LIBC_ENVIRON_H
#define GUARD_LIBS_LIBC_ENVIRON_H 1

#include <hybrid/compiler.h>

DECL_BEGIN

struct envdata;

INTDEF char *LIBCCALL libc_getenv(char const *name);
INTDEF int LIBCCALL libc_clearenv(void);
INTDEF int LIBCCALL libc_setenv(char const *name, char const *value, int replace);
INTDEF int LIBCCALL libc_unsetenv(char const *name);
INTDEF int LIBCCALL libc_putenv(char *string);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
/* Same as 'libc_getenv()', but automatically transform
 * paths and separators in variables such as 'PATH' or 'HOME' */
INTDEF char *LIBCCALL libc_dos_getenv(char const *name);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_ENVIRON_H */
