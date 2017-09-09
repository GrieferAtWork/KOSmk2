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
#ifndef _FNMATCH_H
#define _FNMATCH_H 1

#include <__stdinc.h>

__DECL_BEGIN

#undef FNM_PATHNAME
#undef FNM_NOESCAPE
#undef FNM_PERIOD

#define FNM_PATHNAME (1 << 0) /*< No wildcard can ever match '/'. */
#define FNM_NOESCAPE (1 << 1) /*< Backslashes don't quote special chars. */
#define FNM_PERIOD   (1 << 2) /*< Leading '.' is matched only explicitly. */
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 2 || defined(_GNU_SOURCE)
#   define FNM_FILE_NAME   FNM_PATHNAME /*< Preferred GNU name. */
#   define FNM_LEADING_DIR (1 << 3)     /*< Ignore '/...' after a match. */
#   define FNM_CASEFOLD    (1 << 4)     /*< Compare without regard to case. */
#   define FNM_EXTMATCH    (1 << 5)     /*< Use ksh-like extended matching. */
#endif
#define FNM_NOMATCH    1 /*< Value returned by 'fnmatch' if STRING does not match PATTERN. */
#ifdef _XOPEN_SOURCE
#   define FNM_NOSYS (-1)
#endif

#ifndef __KERNEL__
__LIBC int (__LIBCCALL fnmatch)(char const *__pattern, char const *__name, int __flags);
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_FNMATCH_H */
