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
#ifndef _SYS_GENERIC_TIMEB_H
#define _SYS_GENERIC_TIMEB_H 1
#define _SYS_TIMEB_H 1

#include <features.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("time")
#pragma push_macro("millitm")
#pragma push_macro("timezone")
#pragma push_macro("dstflag")
#endif

#undef time
#undef millitm
#undef timezone
#undef dstflag

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
    time_t          time;     /*< Seconds since epoch, as from `time'. */
    __UINT16_TYPE__ millitm;  /*< Additional milliseconds. */
    __INT16_TYPE__  timezone; /*< Minutes west of GMT. */
    __INT16_TYPE__  dstflag;  /*< Nonzero if Daylight Savings Time used. */
    __INT16_TYPE__  __pad0;   /*< ... */
};

#ifdef __USE_TIME64
#ifndef __time64_t_defined
#define __time64_t_defined 1
typedef __time64_t time64_t;
#endif /* !__time64_t_defined */

struct timeb64 {
    time64_t        time;     /*< Seconds since epoch, as from `time'. */
    __UINT16_TYPE__ millitm;  /*< Additional milliseconds. */
    __INT16_TYPE__  timezone; /*< Minutes west of GMT. */
    __INT16_TYPE__  dstflag;  /*< Nonzero if Daylight Savings Time used. */
    __INT16_TYPE__  __pad0;   /*< ... */
};
#endif

#ifdef __USE_DOS
#ifndef _TIMEB_DEFINED
#define _TIMEB_DEFINED 1
struct __timeb32 {
    __time32_t      time;     /*< Seconds since epoch, as from `time'. */
    __UINT16_TYPE__ millitm;  /*< Additional milliseconds. */
    __INT16_TYPE__  timezone; /*< Minutes west of GMT. */
    __INT16_TYPE__  dstflag;  /*< Nonzero if Daylight Savings Time used. */
    __INT16_TYPE__  __pad0;   /*< ... */
};

struct __timeb64 {
    __time64_t      time;     /*< Seconds since epoch, as from `time'. */
    __UINT16_TYPE__ millitm;  /*< Additional milliseconds. */
    __INT16_TYPE__  timezone; /*< Minutes west of GMT. */
    __INT16_TYPE__  dstflag;  /*< Nonzero if Daylight Savings Time used. */
    __INT16_TYPE__  __pad0;   /*< ... */
};

#ifdef __USE_TIME_BITS64
#define _timeb      __timeb32
#define _ftime      _ftime32
#define _ftime_s    _ftime32_s
#else /* __USE_TIME_BITS64 */
#define _timeb      __timeb64
#define _ftime      _ftime64
#define _ftime_s    _ftime64_s
#endif /* !__USE_TIME_BITS64 */

#endif /* !_TIMEB_DEFINED */
#endif /* __USE_DOS */

#ifndef __KERNEL__
/* NOTE: This switch is only here to reduce binary dependency. */
#ifdef __DOS_COMPAT__
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,,int,__LIBCCALL,ftime,(struct timeb *__timebuf),_ftime64_s,(__timebuf))
#else /* __USE_TIME_BITS64 */
__REDIRECT(__LIBC,,int,__LIBCCALL,ftime,(struct timeb *__timebuf),_ftime32_s,(__timebuf))
#endif /* !__USE_TIME_BITS64 */
#ifdef __USE_TIME64
__REDIRECT(__LIBC,,int,__LIBCCALL,ftime64,(struct timeb64 *__timebuf),_ftime64_s,(__timebuf))
#endif /* __USE_TIME64 */
#elif defined(__GLC_COMPAT__) && defined(__USE_TIME_BITS64)
__REDIRECT(__LIBC,,int,__LIBCCALL,__ftime32,(struct __timeb32 *__timebuf),ftime,(__timebuf))
__LOCAL int (__LIBCCALL ftime)(struct timeb *__timebuf) {
 struct __timeb32 __buf; int __result = ftime(&__buf);
 __timebuf->time     = (__time64_t)__buf.time;
 __timebuf->millitm  = __buf.millitm;
 __timebuf->timezone = __buf.timezone;
 __timebuf->dstflag  = __buf.dstflag;
 return __result;
}
#ifdef __USE_TIME64
__LOCAL int (__LIBCCALL ftime64)(struct timeb64 *__timebuf) { return ftime((struct timeb *)__timebuf); }
#endif /* __USE_TIME64 */
#else /* Compat... */
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,ftime,(struct timeb *__timebuf),ftime,(__timebuf))
#ifdef __USE_TIME64
#ifdef __GLC_COMPAT__
__LOCAL int (__LIBCCALL ftime64)(struct timeb64 *__timebuf) {
 struct __timeb32 __buf; int __result = ftime(&__buf);
 __timebuf->time     = (__time64_t)__buf.time;
 __timebuf->millitm  = __buf.millitm;
 __timebuf->timezone = __buf.timezone;
 __timebuf->dstflag  = __buf.dstflag;
 return __result;
}
#else /* __GLC_COMPAT__ */
__LIBC int (__LIBCCALL ftime64)(struct timeb64 *__timebuf);
#endif /* !__GLC_COMPAT__ */
#endif /* __USE_TIME64 */
#endif /* Builtin... */

#ifdef __USE_DOS
#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifdef __DOS_COMPAT__
__LIBC void (__LIBCCALL _ftime32)(struct __timeb32 *__timebuf);
__LIBC void (__LIBCCALL _ftime64)(struct __timeb64 *__timebuf);
__LIBC errno_t (__LIBCCALL _ftime32_s)(struct __timeb32 *__timebuf);
__LIBC errno_t (__LIBCCALL _ftime64_s)(struct __timeb64 *__timebuf);
#elif defined(__GLC_COMPAT__)
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,,int,__LIBCCALL,__ftime32,(struct __timeb32 *__timebuf),ftime,(__timebuf))
__LOCAL errno_t (__LIBCCALL _ftime32_s)(struct __timeb32 *__timebuf) { return __ftime32(__timebuf); }
__LOCAL errno_t (__LIBCCALL _ftime64_s)(struct __timeb64 *__timebuf) { return ftime(__timebuf); }
__LOCAL void (__LIBCCALL _ftime32)(struct __timeb32 *__timebuf) { __ftime32(__timebuf); }
__LOCAL void (__LIBCCALL _ftime64)(struct __timeb64 *__timebuf) { ftime(__timebuf); }
#else /* __USE_TIME_BITS64 */
__LOCAL errno_t (__LIBCCALL _ftime32_s)(struct __timeb32 *__timebuf) { return ftime(__timebuf); }
__LOCAL errno_t (__LIBCCALL _ftime64_s)(struct __timeb64 *__timebuf) {
    struct __timeb32 __buf; errno_t __result = _ftime32_s(&__buf);
    __timebuf->time     = (__time64_t)__buf.time;
    __timebuf->millitm  = __buf.millitm;
    __timebuf->timezone = __buf.timezone;
    __timebuf->dstflag  = __buf.dstflag;
    return __result;
}
__LOCAL void (__LIBCCALL _ftime32)(struct __timeb32 *__timebuf) { ftime(__timebuf); }
__LOCAL void (__LIBCCALL _ftime64)(struct __timeb64 *__timebuf) { _ftime64_s(__timebuf); }
#endif /* !__USE_TIME_BITS64 */
#else
__REDIRECT_VOID(__LIBC,,__LIBCCALL,_ftime32,(struct __timeb32 *__timebuf),ftime,(__timebuf))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,_ftime64,(struct __timeb64 *__timebuf),ftime64,(__timebuf))
__REDIRECT(__LIBC,,errno_t,__LIBCCALL,_ftime32_s,(struct __timeb32 *__timebuf),ftime,(__timebuf))
__REDIRECT(__LIBC,,errno_t,__LIBCCALL,_ftime64_s,(struct __timeb64 *__timebuf),ftime64,(__timebuf))
#endif
#endif /* __USE_DOS */

#endif /* !__KERNEL__ */

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("dstflag")
#pragma pop_macro("timezone")
#pragma pop_macro("millitm")
#pragma pop_macro("time")
#endif

__SYSDECL_END

#endif /* !_SYS_GENERIC_TIMEB_H */
