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

#include "libc.h"
#include <hybrid/compiler.h>
#include <uchar.h>

DECL_BEGIN

struct envdata;

INTDEF char *LIBCCALL libc_getenv(char const *name);
INTDEF int LIBCCALL libc_clearenv(void);
INTDEF int LIBCCALL libc_setenv(char const *name, char const *value, int replace);
INTDEF int LIBCCALL libc_unsetenv(char const *name);
INTDEF int LIBCCALL libc_putenv(char *string);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
/* Same as `libc_getenv()', but automatically
 * transforms separators in variables such as 'PATH':
 * >> if (name == "PATH")
 * >>     return getenv("PATH").replace(":",";");
 * >>
 * >> [...]
 * >>
 * >> return getenv("PATH");
 */
INTDEF char *LIBCCALL libc_dos_getenv(char const *name);

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

INTDEF int *LIBCCALL libc_p_argc(void);
INTDEF char ***LIBCCALL libc_p_argv(void);
INTDEF char ***LIBCCALL libc_p_environ(void);
INTDEF char **LIBCCALL libc_p_pgmptr(void);
INTDEF void LIBCCALL libc_argvfree(void **argv);
INTDEF void LIBCCALL libc_environ_changed(void);
INTDEF size_t LIBCCALL libc_countpointer(void **pvec);
INTDEF char16_t **LIBCCALL libc_argv8to16_ex(size_t argc, char **__restrict argv);
INTDEF char32_t **LIBCCALL libc_argv8to32_ex(size_t argc, char **__restrict argv);
INTDEF char16_t **LIBCCALL libc_argv8to16(char **__restrict argv);
INTDEF char32_t **LIBCCALL libc_argv8to32(char **__restrict argv);
INTDEF char16_t ***LIBCCALL libc_p_16wargv(void);
INTDEF char16_t ***LIBCCALL libc_p_16wenviron(void);
INTDEF char16_t **LIBCCALL libc_p_16wpgmptr(void);
INTDEF char32_t ***LIBCCALL libc_p_32wargv(void);
INTDEF char32_t ***LIBCCALL libc_p_32wenviron(void);
INTDEF char32_t **LIBCCALL libc_p_32wpgmptr(void);
INTDEF char ***LIBCCALL libc_p_initenviron(void);
INTDEF char16_t ***LIBCCALL libc_p_16winitenviron(void);
INTDEF char32_t ***LIBCCALL libc_p_32winitenviron(void);
INTDEF errno_t LIBCCALL libc_get_pgmptr(char **pres);
INTDEF errno_t LIBCCALL libc_get_wpgmptr(char16_t **pres);
INTDEF errno_t LIBCCALL libc_dos_getenv_s(size_t *psize, char *buf, size_t bufsize, char const *name);
INTDEF errno_t LIBCCALL libc_dos_dupenv_s(char **pbuf, size_t *pbuflen, char const *name);
INTDEF errno_t LIBCCALL libc_dos_putenv_s(char const *name, char const *value);


INTDEF char16_t **libc_16wargv;    /* Internal storage for lazily allocated UTF-16 copy of argv from `appenv'. */
INTDEF char32_t **libc_32wargv;    /* Internal storage for lazily allocated UTF-32 copy of argv from `appenv'. */
INTDEF char16_t **libc_16wenviron; /* Internal storage for lazily allocated & updated UTF-16 copy of `environ'. */
INTDEF char32_t **libc_32wenviron; /* Internal storage for lazily allocated & updated UTF-32 copy of `environ'. */
INTDEF char16_t **libc_16winitenv; /* Internal storage for lazily allocated UTF-16 copy of env from `appenv'. */
INTDEF char32_t **libc_32winitenv; /* Internal storage for lazily allocated UTF-32 copy of env from `appenv'. */

INTDEF char     **libc_initenv;    /* Unused in KOS/ELF-mode (For DOS compatibility only; use `appenv' instead) */
INTDEF char      *libc_pgmptr;     /* Unused in KOS/ELF-mode (For DOS compatibility only; use `appenv' instead) */
INTDEF char16_t  *libc_16wpgmptr;  /* Unused in KOS/ELF-mode (For DOS compatibility only; use '*libc_p_16wpgmptr()' instead) */
INTDEF int        libc_argc;       /* Unused in KOS/ELF-mode (For DOS compatibility only; use `appenv' instead) */
INTDEF char     **libc_argv;       /* Unused in KOS/ELF-mode (For DOS compatibility only; use `appenv' instead) */


typedef struct { int newmode; } dos_startupinfo_t;
/* These are implemented kind-of as the LIBC initializers for DOS-mode.
 * In addition to doing what `_entry' would, they also
 * initialize variables marked as 'Unused in KOS' above.
 * NOTE: I have no idea what 'do_wildcard' is about, so it's just ignored. */
INTDEF int LIBCCALL libc_getmainargs(int *pargc, char ***pargv, char ***penvp, int do_wildcard, dos_startupinfo_t *info);
INTDEF int LIBCCALL libc_16wgetmainargs(int *pargc, char16_t ***pargv, char16_t ***penvp, int do_wildcard, dos_startupinfo_t *info);

#else /* !CONFIG_LIBC_NO_DOS_LIBC */
#define libc_environ_changed() (void)0
#endif /* CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_ENVIRON_H */
