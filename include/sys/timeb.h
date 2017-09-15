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
#ifndef _SYS_TIMEB_H
#define _SYS_TIMEB_H 1

#include <features.h>
#include <bits/types.h>

__DECL_BEGIN

#ifndef __time_t_defined
#define __time_t_defined 1
typedef __TM_TYPE(time) time_t;
#endif /* !__time_t_defined */

/* Copyright (C) 1994-2016 Free Software Foundation, Inc.
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

struct timeb {
 time_t             time;     /*< Seconds since epoch, as from 'time'. */
 unsigned short int millitm;  /*< Additional milliseconds. */
 short int          timezone; /*< Minutes west of GMT. */
 short int          dstflag;  /*< Nonzero if Daylight Savings Time used. */
};

#ifdef __USE_TIME64
#ifndef __time64_t_defined
#define __time64_t_defined 1
typedef __time64_t time64_t;
#endif /* !__time64_t_defined */

struct timeb64 {
 time64_t           time;     /*< Seconds since epoch, as from 'time'. */
 unsigned short int millitm;  /*< Additional milliseconds. */
 short int          timezone; /*< Minutes west of GMT. */
 short int          dstflag;  /*< Nonzero if Daylight Savings Time used. */
};
#endif

#ifndef __KERNEL__
__LIBC int (__LIBCCALL ftime)(struct timeb *__timebuf) __TM_FUNC(ftime);
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL ftime64)(struct timeb64 *__timebuf);
#endif /* __USE_TIME64 */
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_SYS_TIMEB_H */
