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
#ifndef _GETOPT_H
#define _GETOPT_H 1

#include <__stdinc.h>
#include <features.h>

#ifndef __CRT_GLC
#error "<getopt.h> is not supported by the linked libc"
#endif /* !__CRT_GLC */

#ifndef __KERNEL__
/* DISCLAIMER: Documentation comments are derived from those found in "/usr/include/getopt.h".
 *          >> The following is the original copyright notice found in that file. */

/* Declarations for getopt.
   Copyright (C) 1989-2016 Free Software Foundation, Inc.
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

__SYSDECL_BEGIN

/* For communication from `getopt' to the caller.
 * When `getopt' finds an option that takes an argument,
 * the argument value is returned here.
 * Also, when `ordering' is RETURN_IN_ORDER,
 * each non-option ARGV-element is returned here. */
__LIBC char *(optarg);

/* Index in ARGV of the next element to be scanned.
 * - This is used for communication to and from the caller
 *   and for communication between successive calls to `getopt'.
 * - On entry to `getopt', zero means this is the first call; initialize.
 * - When `getopt' returns -1, this is the index of the first of
 *   the non-option elements that the caller should itself scan.
 * - Otherwise, `optind' communicates from one call to
 *   the next how much of ARGV has been scanned so far. */
__LIBC int (optind);

/* Callers store zero here to inhibit the error message
 * `getopt' prints for unrecognized options. */
__LIBC int (opterr);

/* Set to an option character which was unrecognized. */
__LIBC int (optopt);

/* Describe the long-named options requested by the application.
 * The LONG_OPTIONS argument to getopt_long or getopt_long_only is a vector
 * of `struct option' terminated by an element containing a name which is zero.
 * - The field `has_arg' is:
 *     - no_argument       (or 0) if the option does not take an argument,
 *     - required_argument (or 1) if the option requires an argument,
 *     - optional_argument (or 2) if the option takes an optional argument.
 * - If the field `flag' is not NULL, it points to a variable that is set
 *   to the value given in the field `val' when the option is found, but
 *   left unchanged if the option is not found.
 * - To have a long-named option do something other than set an `int' to a
 *   compiled-in constant, such as set a value from `optarg', set the option
 *   's `flag' field to zero and its `val' field to a nonzero value
 *  (the equivalent single-letter option character, if there is one).
 *   For long options that have a zero `flag' field, `getopt' returns
 *   the contents of the `val' field. */
struct option {
    char const *name;
    int         has_arg;
    int        *flag;
    int         val;
};

/* Names for the values of the `has_arg' field of `struct option'. */
#define no_argument       0
#define required_argument 1
#define optional_argument 2

/* Return the option character from OPTS just read.  Return -1 when
 * there are no more options.  For unrecognized options, or options
 * missing arguments, `optopt' is set to the option letter, and '?' is
 * returned.
 * - The OPTS string is a list of characters which are recognized option
 *   letters, optionally followed by colons, specifying that that letter
 *   takes an argument, to be placed in `optarg'.
 * - If a letter in OPTS is followed by two colons, its argument is
 *   optional.  This behavior is specific to the GNU `getopt'.
 * - The argument `--' causes premature termination of argument
 *   scanning, explicitly telling `getopt' that there are no more
 *   options.
 * - If OPTS begins with `--', then non-option arguments are treated as
 *   arguments to the option '\0'.  This behavior is specific to the GNU
 *   `getopt'. */
#ifndef __getopt_defined
#define __getopt_defined 1
__LIBC int (__LIBCCALL getopt)(int ___argc, char *const *___argv, char const *__shortopts);
#endif /* !__getopt_defined */
__LIBC int (__LIBCCALL getopt_long)(int ___argc, char *const *___argv, char const *__shortopts, const struct option *__longopts, int *__longind);
__LIBC int (__LIBCCALL getopt_long_only)(int ___argc, char *const *___argv, char const *__shortopts, const struct option *__longopts, int *__longind);

__SYSDECL_END
#endif /* !__KERNEL__ */

#endif /* !_GETOPT_H */
