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
#ifndef _LANGINFO_H
#define _LANGINFO_H 1

#include <__stdinc.h>
#include <nl_types.h>
#include <bits/locale.h> /* __LC_* */
#ifdef __USE_XOPEN2K
#include <xlocale.h>
#endif /* __USE_XOPEN2K */

__SYSDECL_BEGIN

/* Access to locale-dependent parameters.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
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


#define _NL_ITEM(category,index)    (((category) << 16)|(index))
#define _NL_ITEM_CATEGORY(item)     ((int)(item) >> 16)
#define _NL_ITEM_INDEX(item)        ((int)(item) & 0xffff)

/* LC_TIME category: date and time formatting. */
/* Abbreviated days of the week. */
#define ABDAY_1            _NL_ITEM(__LC_TIME,0) /* Sun */
#define ABDAY_2            _NL_ITEM(__LC_TIME,1)
#define ABDAY_3            _NL_ITEM(__LC_TIME,1)
#define ABDAY_4            _NL_ITEM(__LC_TIME,2)
#define ABDAY_5            _NL_ITEM(__LC_TIME,3)
#define ABDAY_6            _NL_ITEM(__LC_TIME,4)
#define ABDAY_7            _NL_ITEM(__LC_TIME,5)

/* Long-named days of the week. */
#define DAY_1              _NL_ITEM(__LC_TIME,6) /* Sunday */
#define DAY_2              _NL_ITEM(__LC_TIME,7) /* Monday */
#define DAY_3              _NL_ITEM(__LC_TIME,8) /* Tuesday */
#define DAY_4              _NL_ITEM(__LC_TIME,9) /* Wednesday */
#define DAY_5              _NL_ITEM(__LC_TIME,10) /* Thursday */
#define DAY_6              _NL_ITEM(__LC_TIME,11) /* Friday */
#define DAY_7              _NL_ITEM(__LC_TIME,12) /* Saturday */

/* Abbreviated month names. */
#define ABMON_1            _NL_ITEM(__LC_TIME,13) /* Jan */
#define ABMON_2            _NL_ITEM(__LC_TIME,14)
#define ABMON_3            _NL_ITEM(__LC_TIME,15)
#define ABMON_4            _NL_ITEM(__LC_TIME,16)
#define ABMON_5            _NL_ITEM(__LC_TIME,17)
#define ABMON_6            _NL_ITEM(__LC_TIME,18)
#define ABMON_7            _NL_ITEM(__LC_TIME,19)
#define ABMON_8            _NL_ITEM(__LC_TIME,20)
#define ABMON_9            _NL_ITEM(__LC_TIME,21)
#define ABMON_10           _NL_ITEM(__LC_TIME,22)
#define ABMON_11           _NL_ITEM(__LC_TIME,23)
#define ABMON_12           _NL_ITEM(__LC_TIME,24)

/* Long month names. */
#define MON_1              _NL_ITEM(__LC_TIME,25) /* January */
#define MON_2              _NL_ITEM(__LC_TIME,26)
#define MON_3              _NL_ITEM(__LC_TIME,27)
#define MON_4              _NL_ITEM(__LC_TIME,28)
#define MON_5              _NL_ITEM(__LC_TIME,29)
#define MON_6              _NL_ITEM(__LC_TIME,30)
#define MON_7              _NL_ITEM(__LC_TIME,31)
#define MON_8              _NL_ITEM(__LC_TIME,32)
#define MON_9              _NL_ITEM(__LC_TIME,33)
#define MON_10             _NL_ITEM(__LC_TIME,34)
#define MON_11             _NL_ITEM(__LC_TIME,35)
#define MON_12             _NL_ITEM(__LC_TIME,36)

#define AM_STR             _NL_ITEM(__LC_TIME,37) /* Ante meridiem string. */
#define PM_STR             _NL_ITEM(__LC_TIME,38) /* Post meridiem string. */

#define D_T_FMT            _NL_ITEM(__LC_TIME,39) /* Date and time format for strftime. */
#define D_FMT              _NL_ITEM(__LC_TIME,40) /* Date format for strftime. */
#define T_FMT              _NL_ITEM(__LC_TIME,41) /* Time format for strftime. */
#define T_FMT_AMPM         _NL_ITEM(__LC_TIME,42) /* 12-hour time format for strftime. */

#define ERA                _NL_ITEM(__LC_TIME,43) /* Alternate era. */
#define __ERA_YEAR         _NL_ITEM(__LC_TIME,44) /* Year in alternate era format. */
#ifdef __USE_GNU
#define ERA_YEAR           __ERA_YEAR
#endif
#define ERA_D_FMT          _NL_ITEM(__LC_TIME,45) /* Date in alternate era format. */
#define ALT_DIGITS         _NL_ITEM(__LC_TIME,46) /* Alternate symbols for digits. */
#define ERA_D_T_FMT        _NL_ITEM(__LC_TIME,47) /* Date and time in alternate era format. */
#define ERA_T_FMT          _NL_ITEM(__LC_TIME,48) /* Time in alternate era format. */

#define _NL_TIME_ERA_NUM_ENTRIES _NL_ITEM(__LC_TIME,49) /* Number entries in the era arrays. */
#define _NL_TIME_ERA_ENTRIES     _NL_ITEM(__LC_TIME,50) /* Structure with era entries in usable form.*/

#define _NL_WABDAY_1       _NL_ITEM(__LC_TIME,51) /* Sun */
#define _NL_WABDAY_2       _NL_ITEM(__LC_TIME,52)
#define _NL_WABDAY_3       _NL_ITEM(__LC_TIME,53)
#define _NL_WABDAY_4       _NL_ITEM(__LC_TIME,54)
#define _NL_WABDAY_5       _NL_ITEM(__LC_TIME,55)
#define _NL_WABDAY_6       _NL_ITEM(__LC_TIME,56)
#define _NL_WABDAY_7       _NL_ITEM(__LC_TIME,57)

/* Long-named days of the week. */
#define _NL_WDAY_1         _NL_ITEM(__LC_TIME,58) /* Sunday */
#define _NL_WDAY_2         _NL_ITEM(__LC_TIME,59) /* Monday */
#define _NL_WDAY_3         _NL_ITEM(__LC_TIME,60) /* Tuesday */
#define _NL_WDAY_4         _NL_ITEM(__LC_TIME,61) /* Wednesday */
#define _NL_WDAY_5         _NL_ITEM(__LC_TIME,62) /* Thursday */
#define _NL_WDAY_6         _NL_ITEM(__LC_TIME,63) /* Friday */
#define _NL_WDAY_7         _NL_ITEM(__LC_TIME,64) /* Saturday */

/* Abbreviated month names. */
#define _NL_WABMON_1       _NL_ITEM(__LC_TIME,65) /* Jan */
#define _NL_WABMON_2       _NL_ITEM(__LC_TIME,66)
#define _NL_WABMON_3       _NL_ITEM(__LC_TIME,67)
#define _NL_WABMON_4       _NL_ITEM(__LC_TIME,68)
#define _NL_WABMON_5       _NL_ITEM(__LC_TIME,69)
#define _NL_WABMON_6       _NL_ITEM(__LC_TIME,70)
#define _NL_WABMON_7       _NL_ITEM(__LC_TIME,71)
#define _NL_WABMON_8       _NL_ITEM(__LC_TIME,72)
#define _NL_WABMON_9       _NL_ITEM(__LC_TIME,73)
#define _NL_WABMON_10      _NL_ITEM(__LC_TIME,74)
#define _NL_WABMON_11      _NL_ITEM(__LC_TIME,75)
#define _NL_WABMON_12      _NL_ITEM(__LC_TIME,76)

/* Long month names. */
#define _NL_WMON_1         _NL_ITEM(__LC_TIME,77) /* January */
#define _NL_WMON_2         _NL_ITEM(__LC_TIME,78)
#define _NL_WMON_3         _NL_ITEM(__LC_TIME,79)
#define _NL_WMON_4         _NL_ITEM(__LC_TIME,80)
#define _NL_WMON_5         _NL_ITEM(__LC_TIME,81)
#define _NL_WMON_6         _NL_ITEM(__LC_TIME,82)
#define _NL_WMON_7         _NL_ITEM(__LC_TIME,83)
#define _NL_WMON_8         _NL_ITEM(__LC_TIME,84)
#define _NL_WMON_9         _NL_ITEM(__LC_TIME,85)
#define _NL_WMON_10        _NL_ITEM(__LC_TIME,86)
#define _NL_WMON_11        _NL_ITEM(__LC_TIME,87)
#define _NL_WMON_12        _NL_ITEM(__LC_TIME,88)

#define _NL_WAM_STR        _NL_ITEM(__LC_TIME,89) /* Ante meridiem string. */
#define _NL_WPM_STR        _NL_ITEM(__LC_TIME,90) /* Post meridiem string. */

#define _NL_WD_T_FMT       _NL_ITEM(__LC_TIME,91) /* Date and time format for strftime. */
#define _NL_WD_FMT         _NL_ITEM(__LC_TIME,92) /* Date format for strftime. */
#define _NL_WT_FMT         _NL_ITEM(__LC_TIME,93) /* Time format for strftime. */
#define _NL_WT_FMT_AMPM    _NL_ITEM(__LC_TIME,94) /* 12-hour time format for strftime. */

#define _NL_WERA_YEAR      _NL_ITEM(__LC_TIME,95) /* Year in alternate era format. */
#define _NL_WERA_D_FMT     _NL_ITEM(__LC_TIME,96) /* Date in alternate era format. */
#define _NL_WALT_DIGITS    _NL_ITEM(__LC_TIME,97) /* Alternate symbols for digits. */
#define _NL_WERA_D_T_FMT   _NL_ITEM(__LC_TIME,98) /* Date and time in alternate era format. */
#define _NL_WERA_T_FMT     _NL_ITEM(__LC_TIME,99) /* Time in alternate era format. */

#define _NL_TIME_WEEK_NDAYS    _NL_ITEM(__LC_TIME,100)
#define _NL_TIME_WEEK_1STDAY   _NL_ITEM(__LC_TIME,101)
#define _NL_TIME_WEEK_1STWEEK  _NL_ITEM(__LC_TIME,102)
#define _NL_TIME_FIRST_WEEKDAY _NL_ITEM(__LC_TIME,103)
#define _NL_TIME_FIRST_WORKDAY _NL_ITEM(__LC_TIME,104)
#define _NL_TIME_CAL_DIRECTION _NL_ITEM(__LC_TIME,105)
#define _NL_TIME_TIMEZONE      _NL_ITEM(__LC_TIME,106)

#define _DATE_FMT          _NL_ITEM(__LC_TIME,107) /* strftime format for date. */
#define _NL_W_DATE_FMT     _NL_ITEM(__LC_TIME,108)

#define _NL_TIME_CODESET   _NL_ITEM(__LC_TIME,109)

#define _NL_NUM_LC_TIME    _NL_ITEM(__LC_TIME,110) /* Number of indices in LC_TIME category. */

/* LC_COLLATE category: text sorting.
 * This information is accessed by the strcoll and strxfrm functions.
 * These `nl_langinfo' names are used only internally. */
#define _NL_COLLATE_NRULES           _NL_ITEM(__LC_COLLATE,0)
#define _NL_COLLATE_RULESETS         _NL_ITEM(__LC_COLLATE,1)
#define _NL_COLLATE_TABLEMB          _NL_ITEM(__LC_COLLATE,2)
#define _NL_COLLATE_WEIGHTMB         _NL_ITEM(__LC_COLLATE,3)
#define _NL_COLLATE_EXTRAMB          _NL_ITEM(__LC_COLLATE,4)
#define _NL_COLLATE_INDIRECTMB       _NL_ITEM(__LC_COLLATE,5)
#define _NL_COLLATE_GAP1             _NL_ITEM(__LC_COLLATE,6)
#define _NL_COLLATE_GAP2             _NL_ITEM(__LC_COLLATE,7)
#define _NL_COLLATE_GAP3             _NL_ITEM(__LC_COLLATE,8)
#define _NL_COLLATE_TABLEWC          _NL_ITEM(__LC_COLLATE,9)
#define _NL_COLLATE_WEIGHTWC         _NL_ITEM(__LC_COLLATE,10)
#define _NL_COLLATE_EXTRAWC          _NL_ITEM(__LC_COLLATE,11)
#define _NL_COLLATE_INDIRECTWC       _NL_ITEM(__LC_COLLATE,12)
#define _NL_COLLATE_SYMB_HASH_SIZEMB _NL_ITEM(__LC_COLLATE,13)
#define _NL_COLLATE_SYMB_TABLEMB     _NL_ITEM(__LC_COLLATE,14)
#define _NL_COLLATE_SYMB_EXTRAMB     _NL_ITEM(__LC_COLLATE,15)
#define _NL_COLLATE_COLLSEQMB        _NL_ITEM(__LC_COLLATE,16)
#define _NL_COLLATE_COLLSEQWC        _NL_ITEM(__LC_COLLATE,17)
#define _NL_COLLATE_CODESET          _NL_ITEM(__LC_COLLATE,18)
#define _NL_NUM_LC_COLLATE           _NL_ITEM(__LC_COLLATE,19)

/* LC_CTYPE category: character classification.
 * This information is accessed by the functions in <ctype.h>.
 * These `nl_langinfo' names are used only internally. */
#define _NL_CTYPE_CLASS                        _NL_ITEM(__LC_CTYPE,0)
#define _NL_CTYPE_TOUPPER                      _NL_ITEM(__LC_CTYPE,1)
#define _NL_CTYPE_GAP1                         _NL_ITEM(__LC_CTYPE,2)
#define _NL_CTYPE_TOLOWER                      _NL_ITEM(__LC_CTYPE,3)
#define _NL_CTYPE_GAP2                         _NL_ITEM(__LC_CTYPE,4)
#define _NL_CTYPE_CLASS32                      _NL_ITEM(__LC_CTYPE,5)
#define _NL_CTYPE_GAP3                         _NL_ITEM(__LC_CTYPE,6)
#define _NL_CTYPE_GAP4                         _NL_ITEM(__LC_CTYPE,7)
#define _NL_CTYPE_GAP5                         _NL_ITEM(__LC_CTYPE,8)
#define _NL_CTYPE_GAP6                         _NL_ITEM(__LC_CTYPE,9)
#define _NL_CTYPE_CLASS_NAMES                  _NL_ITEM(__LC_CTYPE,10)
#define _NL_CTYPE_MAP_NAMES                    _NL_ITEM(__LC_CTYPE,11)
#define _NL_CTYPE_WIDTH                        _NL_ITEM(__LC_CTYPE,12)
#define _NL_CTYPE_MB_CUR_MAX                   _NL_ITEM(__LC_CTYPE,13)
#define _NL_CTYPE_CODESET_NAME                 _NL_ITEM(__LC_CTYPE,14)
#define CODESET                                _NL_CTYPE_CODESET_NAME
#define _NL_CTYPE_TOUPPER32                    _NL_ITEM(__LC_CTYPE,15)
#define _NL_CTYPE_TOLOWER32                    _NL_ITEM(__LC_CTYPE,16)
#define _NL_CTYPE_CLASS_OFFSET                 _NL_ITEM(__LC_CTYPE,17)
#define _NL_CTYPE_MAP_OFFSET                   _NL_ITEM(__LC_CTYPE,18)
#define _NL_CTYPE_INDIGITS_MB_LEN              _NL_ITEM(__LC_CTYPE,19)
#define _NL_CTYPE_INDIGITS0_MB                 _NL_ITEM(__LC_CTYPE,20)
#define _NL_CTYPE_INDIGITS1_MB                 _NL_ITEM(__LC_CTYPE,21)
#define _NL_CTYPE_INDIGITS2_MB                 _NL_ITEM(__LC_CTYPE,22)
#define _NL_CTYPE_INDIGITS3_MB                 _NL_ITEM(__LC_CTYPE,23)
#define _NL_CTYPE_INDIGITS4_MB                 _NL_ITEM(__LC_CTYPE,24)
#define _NL_CTYPE_INDIGITS5_MB                 _NL_ITEM(__LC_CTYPE,25)
#define _NL_CTYPE_INDIGITS6_MB                 _NL_ITEM(__LC_CTYPE,26)
#define _NL_CTYPE_INDIGITS7_MB                 _NL_ITEM(__LC_CTYPE,27)
#define _NL_CTYPE_INDIGITS8_MB                 _NL_ITEM(__LC_CTYPE,28)
#define _NL_CTYPE_INDIGITS9_MB                 _NL_ITEM(__LC_CTYPE,29)
#define _NL_CTYPE_INDIGITS_WC_LEN              _NL_ITEM(__LC_CTYPE,30)
#define _NL_CTYPE_INDIGITS0_WC                 _NL_ITEM(__LC_CTYPE,31)
#define _NL_CTYPE_INDIGITS1_WC                 _NL_ITEM(__LC_CTYPE,32)
#define _NL_CTYPE_INDIGITS2_WC                 _NL_ITEM(__LC_CTYPE,33)
#define _NL_CTYPE_INDIGITS3_WC                 _NL_ITEM(__LC_CTYPE,34)
#define _NL_CTYPE_INDIGITS4_WC                 _NL_ITEM(__LC_CTYPE,35)
#define _NL_CTYPE_INDIGITS5_WC                 _NL_ITEM(__LC_CTYPE,36)
#define _NL_CTYPE_INDIGITS6_WC                 _NL_ITEM(__LC_CTYPE,37)
#define _NL_CTYPE_INDIGITS7_WC                 _NL_ITEM(__LC_CTYPE,38)
#define _NL_CTYPE_INDIGITS8_WC                 _NL_ITEM(__LC_CTYPE,39)
#define _NL_CTYPE_INDIGITS9_WC                 _NL_ITEM(__LC_CTYPE,40)
#define _NL_CTYPE_OUTDIGIT0_MB                 _NL_ITEM(__LC_CTYPE,41)
#define _NL_CTYPE_OUTDIGIT1_MB                 _NL_ITEM(__LC_CTYPE,42)
#define _NL_CTYPE_OUTDIGIT2_MB                 _NL_ITEM(__LC_CTYPE,43)
#define _NL_CTYPE_OUTDIGIT3_MB                 _NL_ITEM(__LC_CTYPE,44)
#define _NL_CTYPE_OUTDIGIT4_MB                 _NL_ITEM(__LC_CTYPE,45)
#define _NL_CTYPE_OUTDIGIT5_MB                 _NL_ITEM(__LC_CTYPE,46)
#define _NL_CTYPE_OUTDIGIT6_MB                 _NL_ITEM(__LC_CTYPE,47)
#define _NL_CTYPE_OUTDIGIT7_MB                 _NL_ITEM(__LC_CTYPE,48)
#define _NL_CTYPE_OUTDIGIT8_MB                 _NL_ITEM(__LC_CTYPE,49)
#define _NL_CTYPE_OUTDIGIT9_MB                 _NL_ITEM(__LC_CTYPE,50)
#define _NL_CTYPE_OUTDIGIT0_WC                 _NL_ITEM(__LC_CTYPE,51)
#define _NL_CTYPE_OUTDIGIT1_WC                 _NL_ITEM(__LC_CTYPE,52)
#define _NL_CTYPE_OUTDIGIT2_WC                 _NL_ITEM(__LC_CTYPE,53)
#define _NL_CTYPE_OUTDIGIT3_WC                 _NL_ITEM(__LC_CTYPE,54)
#define _NL_CTYPE_OUTDIGIT4_WC                 _NL_ITEM(__LC_CTYPE,55)
#define _NL_CTYPE_OUTDIGIT5_WC                 _NL_ITEM(__LC_CTYPE,56)
#define _NL_CTYPE_OUTDIGIT6_WC                 _NL_ITEM(__LC_CTYPE,57)
#define _NL_CTYPE_OUTDIGIT7_WC                 _NL_ITEM(__LC_CTYPE,58)
#define _NL_CTYPE_OUTDIGIT8_WC                 _NL_ITEM(__LC_CTYPE,59)
#define _NL_CTYPE_OUTDIGIT9_WC                 _NL_ITEM(__LC_CTYPE,60)
#define _NL_CTYPE_TRANSLIT_TAB_SIZE            _NL_ITEM(__LC_CTYPE,61)
#define _NL_CTYPE_TRANSLIT_FROM_IDX            _NL_ITEM(__LC_CTYPE,62)
#define _NL_CTYPE_TRANSLIT_FROM_TBL            _NL_ITEM(__LC_CTYPE,63)
#define _NL_CTYPE_TRANSLIT_TO_IDX              _NL_ITEM(__LC_CTYPE,64)
#define _NL_CTYPE_TRANSLIT_TO_TBL              _NL_ITEM(__LC_CTYPE,65)
#define _NL_CTYPE_TRANSLIT_DEFAULT_MISSING_LEN _NL_ITEM(__LC_CTYPE,66)
#define _NL_CTYPE_TRANSLIT_DEFAULT_MISSING     _NL_ITEM(__LC_CTYPE,67)
#define _NL_CTYPE_TRANSLIT_IGNORE_LEN          _NL_ITEM(__LC_CTYPE,68)
#define _NL_CTYPE_TRANSLIT_IGNORE              _NL_ITEM(__LC_CTYPE,69)
#define _NL_CTYPE_MAP_TO_NONASCII              _NL_ITEM(__LC_CTYPE,70)
#define _NL_CTYPE_NONASCII_CASE                _NL_ITEM(__LC_CTYPE,71)
#define _NL_CTYPE_EXTRA_MAP_1                  _NL_ITEM(__LC_CTYPE,72)
#define _NL_CTYPE_EXTRA_MAP_2                  _NL_ITEM(__LC_CTYPE,73)
#define _NL_CTYPE_EXTRA_MAP_3                  _NL_ITEM(__LC_CTYPE,74)
#define _NL_CTYPE_EXTRA_MAP_4                  _NL_ITEM(__LC_CTYPE,75)
#define _NL_CTYPE_EXTRA_MAP_5                  _NL_ITEM(__LC_CTYPE,76)
#define _NL_CTYPE_EXTRA_MAP_6                  _NL_ITEM(__LC_CTYPE,77)
#define _NL_CTYPE_EXTRA_MAP_7                  _NL_ITEM(__LC_CTYPE,78)
#define _NL_CTYPE_EXTRA_MAP_8                  _NL_ITEM(__LC_CTYPE,79)
#define _NL_CTYPE_EXTRA_MAP_9                  _NL_ITEM(__LC_CTYPE,80)
#define _NL_CTYPE_EXTRA_MAP_10                 _NL_ITEM(__LC_CTYPE,81)
#define _NL_CTYPE_EXTRA_MAP_11                 _NL_ITEM(__LC_CTYPE,82)
#define _NL_CTYPE_EXTRA_MAP_12                 _NL_ITEM(__LC_CTYPE,83)
#define _NL_CTYPE_EXTRA_MAP_13                 _NL_ITEM(__LC_CTYPE,84)
#define _NL_CTYPE_EXTRA_MAP_14                 _NL_ITEM(__LC_CTYPE,85)
#define _NL_NUM_LC_CTYPE                       _NL_ITEM(__LC_CTYPE,86)

/* LC_MONETARY category: formatting of monetary quantities.
 * These items each correspond to a member of `struct lconv',
 * defined in <locale.h>. */
#define __INT_CURR_SYMBOL                   _NL_ITEM(__LC_MONETARY,0)
#define __CURRENCY_SYMBOL                   _NL_ITEM(__LC_MONETARY,1)
#define __MON_DECIMAL_POINT                 _NL_ITEM(__LC_MONETARY,2)
#define __MON_THOUSANDS_SEP                 _NL_ITEM(__LC_MONETARY,3)
#define __MON_GROUPING                      _NL_ITEM(__LC_MONETARY,4)
#define __POSITIVE_SIGN                     _NL_ITEM(__LC_MONETARY,5)
#define __NEGATIVE_SIGN                     _NL_ITEM(__LC_MONETARY,6)
#define __INT_FRAC_DIGITS                   _NL_ITEM(__LC_MONETARY,7)
#define __FRAC_DIGITS                       _NL_ITEM(__LC_MONETARY,8)
#define __P_CS_PRECEDES                     _NL_ITEM(__LC_MONETARY,9)
#define __P_SEP_BY_SPACE                    _NL_ITEM(__LC_MONETARY,10)
#define __N_CS_PRECEDES                     _NL_ITEM(__LC_MONETARY,11)
#define __N_SEP_BY_SPACE                    _NL_ITEM(__LC_MONETARY,12)
#define __P_SIGN_POSN                       _NL_ITEM(__LC_MONETARY,13)
#define __N_SIGN_POSN                       _NL_ITEM(__LC_MONETARY,14)
#define _NL_MONETARY_CRNCYSTR               _NL_ITEM(__LC_MONETARY,15)
#define __INT_P_CS_PRECEDES                 _NL_ITEM(__LC_MONETARY,16)
#define __INT_P_SEP_BY_SPACE                _NL_ITEM(__LC_MONETARY,17)
#define __INT_N_CS_PRECEDES                 _NL_ITEM(__LC_MONETARY,18)
#define __INT_N_SEP_BY_SPACE                _NL_ITEM(__LC_MONETARY,19)
#define __INT_P_SIGN_POSN                   _NL_ITEM(__LC_MONETARY,20)
#define __INT_N_SIGN_POSN                   _NL_ITEM(__LC_MONETARY,21)
#define _NL_MONETARY_DUO_INT_CURR_SYMBOL    _NL_ITEM(__LC_MONETARY,22)
#define _NL_MONETARY_DUO_CURRENCY_SYMBOL    _NL_ITEM(__LC_MONETARY,23)
#define _NL_MONETARY_DUO_INT_FRAC_DIGITS    _NL_ITEM(__LC_MONETARY,24)
#define _NL_MONETARY_DUO_FRAC_DIGITS        _NL_ITEM(__LC_MONETARY,25)
#define _NL_MONETARY_DUO_P_CS_PRECEDES      _NL_ITEM(__LC_MONETARY,26)
#define _NL_MONETARY_DUO_P_SEP_BY_SPACE     _NL_ITEM(__LC_MONETARY,27)
#define _NL_MONETARY_DUO_N_CS_PRECEDES      _NL_ITEM(__LC_MONETARY,28)
#define _NL_MONETARY_DUO_N_SEP_BY_SPACE     _NL_ITEM(__LC_MONETARY,29)
#define _NL_MONETARY_DUO_INT_P_CS_PRECEDES  _NL_ITEM(__LC_MONETARY,30)
#define _NL_MONETARY_DUO_INT_P_SEP_BY_SPACE _NL_ITEM(__LC_MONETARY,31)
#define _NL_MONETARY_DUO_INT_N_CS_PRECEDES  _NL_ITEM(__LC_MONETARY,32)
#define _NL_MONETARY_DUO_INT_N_SEP_BY_SPACE _NL_ITEM(__LC_MONETARY,33)
#define _NL_MONETARY_DUO_P_SIGN_POSN        _NL_ITEM(__LC_MONETARY,34)
#define _NL_MONETARY_DUO_N_SIGN_POSN        _NL_ITEM(__LC_MONETARY,35)
#define _NL_MONETARY_DUO_INT_P_SIGN_POSN    _NL_ITEM(__LC_MONETARY,36)
#define _NL_MONETARY_DUO_INT_N_SIGN_POSN    _NL_ITEM(__LC_MONETARY,37)
#define _NL_MONETARY_UNO_VALID_FROM         _NL_ITEM(__LC_MONETARY,38)
#define _NL_MONETARY_UNO_VALID_TO           _NL_ITEM(__LC_MONETARY,39)
#define _NL_MONETARY_DUO_VALID_FROM         _NL_ITEM(__LC_MONETARY,40)
#define _NL_MONETARY_DUO_VALID_TO           _NL_ITEM(__LC_MONETARY,41)
#define _NL_MONETARY_CONVERSION_RATE        _NL_ITEM(__LC_MONETARY,42)
#define _NL_MONETARY_DECIMAL_POINT_WC       _NL_ITEM(__LC_MONETARY,43)
#define _NL_MONETARY_THOUSANDS_SEP_WC       _NL_ITEM(__LC_MONETARY,44)
#define _NL_MONETARY_CODESET                _NL_ITEM(__LC_MONETARY,45)
#define _NL_NUM_LC_MONETARY                 _NL_ITEM(__LC_MONETARY,46)
#define CRNCYSTR                            _NL_MONETARY_CRNCYSTR
#ifdef __USE_GNU
#   define INT_CURR_SYMBOL                  __INT_CURR_SYMBOL
#   define CURRENCY_SYMBOL                  __CURRENCY_SYMBOL
#   define MON_DECIMAL_POINT                __MON_DECIMAL_POINT
#   define MON_THOUSANDS_SEP                __MON_THOUSANDS_SEP
#   define MON_GROUPING                     __MON_GROUPING
#   define POSITIVE_SIGN                    __POSITIVE_SIGN
#   define NEGATIVE_SIGN                    __NEGATIVE_SIGN
#   define INT_FRAC_DIGITS                  __INT_FRAC_DIGITS
#   define FRAC_DIGITS                      __FRAC_DIGITS
#   define P_CS_PRECEDES                    __P_CS_PRECEDES
#   define P_SEP_BY_SPACE                   __P_SEP_BY_SPACE
#   define N_CS_PRECEDES                    __N_CS_PRECEDES
#   define N_SEP_BY_SPACE                   __N_SEP_BY_SPACE
#   define P_SIGN_POSN                      __P_SIGN_POSN
#   define N_SIGN_POSN                      __N_SIGN_POSN
#   define INT_P_CS_PRECEDES                __INT_P_CS_PRECEDES
#   define INT_P_SEP_BY_SPACE               __INT_P_SEP_BY_SPACE
#   define INT_N_CS_PRECEDES                __INT_N_CS_PRECEDES
#   define INT_N_SEP_BY_SPACE               __INT_N_SEP_BY_SPACE
#   define INT_P_SIGN_POSN                  __INT_P_SIGN_POSN
#   define INT_N_SIGN_POSN                  __INT_N_SIGN_POSN
#endif

/* LC_NUMERIC category: formatting of numbers.
 * These also correspond to members of `struct lconv'; see <locale.h>. */
#define __DECIMAL_POINT                     _NL_ITEM(__LC_NUMERIC,0)
#define __THOUSANDS_SEP                     _NL_ITEM(__LC_NUMERIC,1)
#define __GROUPING                          _NL_ITEM(__LC_NUMERIC,2)
#define _NL_NUMERIC_DECIMAL_POINT_WC        _NL_ITEM(__LC_NUMERIC,3)
#define _NL_NUMERIC_THOUSANDS_SEP_WC        _NL_ITEM(__LC_NUMERIC,4)
#define _NL_NUMERIC_CODESET                 _NL_ITEM(__LC_NUMERIC,5)
#define _NL_NUM_LC_NUMERIC                  _NL_ITEM(__LC_NUMERIC,6)
#define THOUSEP                             __THOUSANDS_SEP
#define RADIXCHAR                           __DECIMAL_POINT
#ifdef __USE_GNU
#   define DECIMAL_POINT                    __DECIMAL_POINT
#   define THOUSANDS_SEP                    __THOUSANDS_SEP
#   define GROUPING                         __GROUPING
#endif

#define __YESEXPR                           _NL_ITEM(__LC_MESSAGES,0) /* Regex matching ``yes'' input. */
#define  __NOEXPR                           _NL_ITEM(__LC_MESSAGES,1) /* Regex matching ``no'' input. */
#define __YESSTR                            _NL_ITEM(__LC_MESSAGES,2) /* Output string for ``yes''. */
#define __NOSTR                             _NL_ITEM(__LC_MESSAGES,3) /* Output string for ``no''. */
#define _NL_MESSAGES_CODESET                _NL_ITEM(__LC_MESSAGES,4)
#define _NL_NUM_LC_MESSAGES                 _NL_ITEM(__LC_MESSAGES,5)
#define YESEXPR                             __YESEXPR
#define NOEXPR                              __NOEXPR
#if defined(__USE_GNU) || (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
#   define YESSTR                           __YESSTR
#   define NOSTR                            __NOSTR
#endif

#define _NL_PAPER_HEIGHT                    _NL_ITEM(__LC_PAPER,0)
#define _NL_PAPER_WIDTH                     _NL_ITEM(__LC_PAPER,1)
#define _NL_PAPER_CODESET                   _NL_ITEM(__LC_PAPER,2)
#define _NL_NUM_LC_PAPER                    _NL_ITEM(__LC_PAPER,3)

#define _NL_NAME_NAME_FMT                   _NL_ITEM(__LC_NAME,0)
#define _NL_NAME_NAME_GEN                   _NL_ITEM(__LC_NAME,1)
#define _NL_NAME_NAME_MR                    _NL_ITEM(__LC_NAME,2)
#define _NL_NAME_NAME_MRS                   _NL_ITEM(__LC_NAME,3)
#define _NL_NAME_NAME_MISS                  _NL_ITEM(__LC_NAME,4)
#define _NL_NAME_NAME_MS                    _NL_ITEM(__LC_NAME,5)
#define _NL_NAME_CODESET                    _NL_ITEM(__LC_NAME,6)
#define _NL_NUM_LC_NAME                     _NL_ITEM(__LC_NAME,7)

#define _NL_ADDRESS_POSTAL_FMT              _NL_ITEM(__LC_ADDRESS,0)
#define _NL_ADDRESS_COUNTRY_NAME            _NL_ITEM(__LC_ADDRESS,1)
#define _NL_ADDRESS_COUNTRY_POST            _NL_ITEM(__LC_ADDRESS,2)
#define _NL_ADDRESS_COUNTRY_AB2             _NL_ITEM(__LC_ADDRESS,3)
#define _NL_ADDRESS_COUNTRY_AB3             _NL_ITEM(__LC_ADDRESS,4)
#define _NL_ADDRESS_COUNTRY_CAR             _NL_ITEM(__LC_ADDRESS,5)
#define _NL_ADDRESS_COUNTRY_NUM             _NL_ITEM(__LC_ADDRESS,6)
#define _NL_ADDRESS_COUNTRY_ISBN            _NL_ITEM(__LC_ADDRESS,7)
#define _NL_ADDRESS_LANG_NAME               _NL_ITEM(__LC_ADDRESS,8)
#define _NL_ADDRESS_LANG_AB                 _NL_ITEM(__LC_ADDRESS,9)
#define _NL_ADDRESS_LANG_TERM               _NL_ITEM(__LC_ADDRESS,10)
#define _NL_ADDRESS_LANG_LIB                _NL_ITEM(__LC_ADDRESS,11)
#define _NL_ADDRESS_CODESET                 _NL_ITEM(__LC_ADDRESS,12)
#define _NL_NUM_LC_ADDRESS                  _NL_ITEM(__LC_ADDRESS,13)

#define _NL_TELEPHONE_TEL_INT_FMT           _NL_ITEM(__LC_TELEPHONE,0)
#define _NL_TELEPHONE_TEL_DOM_FMT           _NL_ITEM(__LC_TELEPHONE,1)
#define _NL_TELEPHONE_INT_SELECT            _NL_ITEM(__LC_TELEPHONE,2)
#define _NL_TELEPHONE_INT_PREFIX            _NL_ITEM(__LC_TELEPHONE,3)
#define _NL_TELEPHONE_CODESET               _NL_ITEM(__LC_TELEPHONE,4)
#define _NL_NUM_LC_TELEPHONE                _NL_ITEM(__LC_TELEPHONE,5)

#define _NL_MEASUREMENT_MEASUREMENT         _NL_ITEM(__LC_MEASUREMENT,0)
#define _NL_MEASUREMENT_CODESET             _NL_ITEM(__LC_MEASUREMENT,1)
#define _NL_NUM_LC_MEASUREMENT              _NL_ITEM(__LC_MEASUREMENT,2)

#define _NL_IDENTIFICATION_TITLE            _NL_ITEM(__LC_IDENTIFICATION,0)
#define _NL_IDENTIFICATION_SOURCE           _NL_ITEM(__LC_IDENTIFICATION,1)
#define _NL_IDENTIFICATION_ADDRESS          _NL_ITEM(__LC_IDENTIFICATION,2)
#define _NL_IDENTIFICATION_CONTACT          _NL_ITEM(__LC_IDENTIFICATION,3)
#define _NL_IDENTIFICATION_EMAIL            _NL_ITEM(__LC_IDENTIFICATION,4)
#define _NL_IDENTIFICATION_TEL              _NL_ITEM(__LC_IDENTIFICATION,5)
#define _NL_IDENTIFICATION_FAX              _NL_ITEM(__LC_IDENTIFICATION,6)
#define _NL_IDENTIFICATION_LANGUAGE         _NL_ITEM(__LC_IDENTIFICATION,7)
#define _NL_IDENTIFICATION_TERRITORY        _NL_ITEM(__LC_IDENTIFICATION,8)
#define _NL_IDENTIFICATION_AUDIENCE         _NL_ITEM(__LC_IDENTIFICATION,9)
#define _NL_IDENTIFICATION_APPLICATION      _NL_ITEM(__LC_IDENTIFICATION,10)
#define _NL_IDENTIFICATION_ABBREVIATION     _NL_ITEM(__LC_IDENTIFICATION,11)
#define _NL_IDENTIFICATION_REVISION         _NL_ITEM(__LC_IDENTIFICATION,12)
#define _NL_IDENTIFICATION_DATE             _NL_ITEM(__LC_IDENTIFICATION,13)
#define _NL_IDENTIFICATION_CATEGORY         _NL_ITEM(__LC_IDENTIFICATION,14)
#define _NL_IDENTIFICATION_CODESET          _NL_ITEM(__LC_IDENTIFICATION,15)
#define _NL_NUM_LC_IDENTIFICATION           _NL_ITEM(__LC_IDENTIFICATION,16)

/* This marks the highest value used. */
#define _NL_NUM                             _NL_ITEM(__LC_IDENTIFICATION,17)

/* This macro produces an item you can pass to `nl_langinfo' or
 * `nl_langinfo_l' to get the name of the locale in use for CATEGORY. */
#define _NL_LOCALE_NAME(category)    _NL_ITEM((category),_NL_ITEM_INDEX(-1))
#ifdef __USE_GNU
#   define NL_LOCALE_NAME(category)  _NL_LOCALE_NAME(category)
#endif

#ifndef __KERNEL__
__LIBC __PORT_NODOS char *(__LIBCCALL nl_langinfo)(nl_item __item);
#ifdef __USE_XOPEN2K
__LIBC __PORT_NODOS char *(__LIBCCALL nl_langinfo_l)(nl_item __item, __locale_t __l);
#endif /* __USE_XOPEN2K */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_LANGINFO_H */
