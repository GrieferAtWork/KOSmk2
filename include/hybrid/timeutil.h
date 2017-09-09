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
#ifndef GUARD_HYBRID_TIMEUTIL_H
#define GUARD_HYBRID_TIMEUTIL_H 1

#include "compiler.h"

DECL_BEGIN

#define LINUX_TIME_START_YEAR  1970

#define SECONDS_PER_DAY        86400

#define DAYS2YEARS(n_days)   ((400*((n_days)+1))/146097)
#define YEARS2DAYS(n_years)  (((146097*(n_years))/400)/*-1*/) // rounding error?
#define ISLEAPYEAR(year) \
 (__builtin_constant_p(year)\
  ? ((year)%400 == 0 || ((year)%100 != 0 && (year)%4 == 0))\
  : XBLOCK({ __typeof__(year) const _year = (year);\
             XRETURN _year%400 == 0 || (_year%100 != 0 && _year%4 == 0);\
  }))

#define __DEFINE_MONTH_STARTING_DAY_OF_YEAR \
 time_t const __time_monthstart_yday[2][13] = { \
   {0,31,59,90,120,151,181,212,243,273,304,334,365}, \
   {0,31,60,91,121,152,182,213,244,274,305,335,366}}; 

#define DEFINE_MONTH_STARTING_DAY_OF_YEAR \
 PRIVATE __DEFINE_MONTH_STARTING_DAY_OF_YEAR

#define MONTH_STARTING_DAY_OF_YEAR(leap_year,month) \
 __time_monthstart_yday[!!(leap_year)][month]


DECL_END

#endif /* !GUARD_HYBRID_TIMEUTIL_H */
