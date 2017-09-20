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
#ifndef _NL_TYPES_H
#define _NL_TYPES_H 1

#include <__stdinc.h>
#include <features.h>

/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

__DECL_BEGIN

#define NL_SETD       1 /*< The default message set used by the gencat program. */
#define NL_CAT_LOCALE 1 /*< Value for FLAG parameter of `catgets' to say we want XPG4 compliance. */

typedef void *nl_catd; /*< Message catalog descriptor type. */
typedef int   nl_item; /*< Type used by `nl_langinfo'. */

#ifndef __KERNEL__
__LIBC __NONNULL((1)) nl_catd (__LIBCCALL catopen)(char const *__cat_name, int __flag);
__LIBC __NONNULL((1)) char *(__LIBCCALL catgets)(nl_catd __catalog, int __set, int __number, char const *__string);
__LIBC __NONNULL((1)) int (__LIBCCALL catclose)(nl_catd __catalog);
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_NL_TYPES_H */
