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
#ifndef _LOCALE_H
#define _LOCALE_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/locale.h>
#ifdef __USE_XOPEN2K8
#include <xlocale.h>
#endif /* __USE_XOPEN2K8 */

/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

#ifndef NULL
#ifdef __INTELLISENSE__
#   define NULL nullptr
#elif defined(__cplusplus) || defined(__LINKER__)
#   define NULL          0
#else
#   define NULL ((void *)0)
#endif
#endif

#define __DOS_LC_ALL      0
#define __DOS_LC_COLLATE  1
#define __DOS_LC_CTYPE    2
#define __DOS_LC_MONETARY 3
#define __DOS_LC_NUMERIC  4
#define __DOS_LC_TIME     5
#define __DOS_LC_MIN      0
#define __DOS_LC_MAX      5

#ifdef __USE_DOS
#   define LC_ALL            __DOS_LC_ALL
#   define LC_COLLATE        __DOS_LC_COLLATE
#   define LC_CTYPE          __DOS_LC_CTYPE
#   define LC_MONETARY       __DOS_LC_MONETARY
#   define LC_NUMERIC        __DOS_LC_NUMERIC
#   define LC_TIME           __DOS_LC_TIME
#   define LC_MIN            __DOS_LC_MIN
#   define LC_MAX            __DOS_LC_MAX
#else /* __USE_DOS */
#   define LC_CTYPE          __LC_CTYPE
#   define LC_NUMERIC        __LC_NUMERIC
#   define LC_TIME           __LC_TIME
#   define LC_COLLATE        __LC_COLLATE
#   define LC_MONETARY       __LC_MONETARY
#   define LC_MESSAGES       __LC_MESSAGES
#   define LC_ALL            __LC_ALL
#   define LC_PAPER          __LC_PAPER
#   define LC_NAME           __LC_NAME
#   define LC_ADDRESS        __LC_ADDRESS
#   define LC_TELEPHONE      __LC_TELEPHONE
#   define LC_MEASUREMENT    __LC_MEASUREMENT
#   define LC_IDENTIFICATION __LC_IDENTIFICATION
#endif /* !__USE_DOS */

__NAMESPACE_STD_BEGIN

/* Structure giving information about numeric and monetary notation. */
struct lconv {
  /* Numeric (non-monetary) information. */
  char *decimal_point;        /* Decimal point character. */
  char *thousands_sep;        /* Thousands separator. */
  /* Each element is the number of digits in each group;
   * elements with higher indices are farther left.
   * An element with value CHAR_MAX means that no further grouping is done.
   * An element with value 0 means that the previous element is used
   * for all groups farther left. */
  char *grouping;

  /* Monetary information. */

  /* First three chars are a currency symbol from ISO 4217.
     Fourth char is the separator.  Fifth char is '\0'. */
  char *int_curr_symbol;
  char *currency_symbol;   /*< Local currency symbol. */
  char *mon_decimal_point; /*< Decimal point character. */
  char *mon_thousands_sep; /*< Thousands separator. */
  char *mon_grouping;      /*< Like `grouping' element (above). */
  char *positive_sign;     /*< Sign for positive values. */
  char *negative_sign;     /*< Sign for negative values. */
  char int_frac_digits;    /*< Int'l fractional digits. */
  char frac_digits;        /*< Local fractional digits. */
  char p_cs_precedes;      /*< 1 if currency_symbol precedes a positive value, 0 if succeeds. */
  char p_sep_by_space;     /*< 1 iff a space separates currency_symbol from a positive value. */
  char n_cs_precedes;      /*< 1 if currency_symbol precedes a negative value, 0 if succeeds. */
  char n_sep_by_space;     /*< 1 iff a space separates currency_symbol from a negative value. */
  /* Positive and negative sign positions:
   * 0 Parentheses surround the quantity and currency_symbol.
   * 1 The sign string precedes the quantity and currency_symbol.
   * 2 The sign string follows the quantity and currency_symbol.
   * 3 The sign string immediately precedes the currency_symbol.
   * 4 The sign string immediately follows the currency_symbol. */
  char p_sign_posn;
  char n_sign_posn;
#if defined(__PE__) && defined(__USE_DOS)
  wchar_t *_W_decimal_point;
  wchar_t *_W_thousands_sep;
  wchar_t *_W_int_curr_symbol;
  wchar_t *_W_currency_symbol;
  wchar_t *_W_mon_decimal_point;
  wchar_t *_W_mon_thousands_sep;
  wchar_t *_W_positive_sign;
  wchar_t *_W_negative_sign;
#elif defined(__USE_ISOC99)
  char int_p_cs_precedes;  /* 1 if int_curr_symbol precedes a positive value, 0 if succeeds. */
  char int_p_sep_by_space; /* 1 iff a space separates int_curr_symbol from a positive value. */
  char int_n_cs_precedes;  /* 1 if int_curr_symbol precedes a negative value, 0 if succeeds. */
  char int_n_sep_by_space; /* 1 iff a space separates int_curr_symbol from a negative value. */
  /* Positive and negative sign positions:
   * 0 Parentheses surround the quantity and int_curr_symbol.
   * 1 The sign string precedes the quantity and int_curr_symbol.
   * 2 The sign string follows the quantity and int_curr_symbol.
   * 3 The sign string immediately precedes the int_curr_symbol.
   * 4 The sign string immediately follows the int_curr_symbol. */
  char int_p_sign_posn;
  char int_n_sign_posn;
#else
  char __int_p_cs_precedes;
  char __int_p_sep_by_space;
  char __int_n_cs_precedes;
  char __int_n_sep_by_space;
  char __int_p_sign_posn;
  char __int_n_sign_posn;
#endif
#if defined(__USE_DOS) && !defined(__PE__)
  wchar_t *_W_decimal_point;
  wchar_t *_W_thousands_sep;
  wchar_t *_W_int_curr_symbol;
  wchar_t *_W_currency_symbol;
  wchar_t *_W_mon_decimal_point;
  wchar_t *_W_mon_thousands_sep;
  wchar_t *_W_positive_sign;
  wchar_t *_W_negative_sign;
#endif
};

__LIBC char *(__LIBCCALL setlocale)(int __category, char const *__locale)  __DOS_FUNC(setlocale);
#if defined(__PE__) && !defined(__USE_DOS)
__LIBC struct lconv *(__LIBCCALL localeconv)(void) __ASMNAME(".kos.localeconv");
#else
__LIBC struct lconv *(__LIBCCALL localeconv)(void);
#endif

__NAMESPACE_STD_END
__NAMESPACE_STD_USING(lconv)
__NAMESPACE_STD_USING(setlocale)
__NAMESPACE_STD_USING(localeconv)


#ifdef __USE_XOPEN2K8

__LIBC __locale_t (__LIBCCALL newlocale)(int __category_mask, char const *__locale, __locale_t __base);

/* These are the bits that can be set in the CATEGORY_MASK
 * argument to `newlocale'. In the GNU implementation, LC_FOO_MASK
 * has the value of (1 << LC_FOO), but this is not a part
 * of the interface that callers can assume will be true. */
#define LC_CTYPE_MASK          (1 << __LC_CTYPE)
#define LC_NUMERIC_MASK        (1 << __LC_NUMERIC)
#define LC_TIME_MASK           (1 << __LC_TIME)
#define LC_COLLATE_MASK        (1 << __LC_COLLATE)
#define LC_MONETARY_MASK       (1 << __LC_MONETARY)
#define LC_MESSAGES_MASK       (1 << __LC_MESSAGES)
#define LC_PAPER_MASK          (1 << __LC_PAPER)
#define LC_NAME_MASK           (1 << __LC_NAME)
#define LC_ADDRESS_MASK        (1 << __LC_ADDRESS)
#define LC_TELEPHONE_MASK      (1 << __LC_TELEPHONE)
#define LC_MEASUREMENT_MASK    (1 << __LC_MEASUREMENT)
#define LC_IDENTIFICATION_MASK (1 << __LC_IDENTIFICATION)
#define LC_ALL_MASK            (LC_CTYPE_MASK|LC_NUMERIC_MASK|LC_TIME_MASK|LC_COLLATE_MASK \
                               |LC_MONETARY_MASK|LC_MESSAGES_MASK|LC_PAPER_MASK|LC_NAME_MASK \
                               |LC_ADDRESS_MASK|LC_TELEPHONE_MASK|LC_MEASUREMENT_MASK \
                               |LC_IDENTIFICATION_MASK)

__LIBC __locale_t (__LIBCCALL duplocale)(__locale_t __dataset);
__LIBC void (__LIBCCALL freelocale)(__locale_t __dataset);
__LIBC __locale_t (__LIBCCALL uselocale)(__locale_t __dataset);
#define LC_GLOBAL_LOCALE    ((__locale_t)-1L)
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_DOS
#ifndef _CONFIG_LOCALE_SWT
#define _CONFIG_LOCALE_SWT 1
#define _ENABLE_PER_THREAD_LOCALE         0x001
#define _DISABLE_PER_THREAD_LOCALE        0x002
#define _ENABLE_PER_THREAD_LOCALE_GLOBAL  0x010
#define _DISABLE_PER_THREAD_LOCALE_GLOBAL 0x020
#define _ENABLE_PER_THREAD_LOCALE_NEW     0x100
#define _DISABLE_PER_THREAD_LOCALE_NEW    0x200
#endif /* !_CONFIG_LOCALE_SWT */

__LIBC int (__LIBCCALL _configthreadlocale)(int __flag);
__LIBC __locale_t (__LIBCCALL _get_current_locale)(void);
__LIBC __locale_t (__LIBCCALL __get_current_locale)(void) __ASMNAME("_get_current_locale");
__LIBC __locale_t (__LIBCCALL _create_locale)(int __dos_category, char const *__locale);
__LIBC __locale_t (__LIBCCALL __create_locale)(int __dos_category, char const *__locale) __ASMNAME("_create_locale");
__LIBC void (__LIBCCALL __free_locale)(__locale_t __locale) __ASMNAME("freelocale");
__LIBC void (__LIBCCALL _free_locale)(__locale_t __locale) __ASMNAME("freelocale");

#ifndef _WLOCALE_DEFINED
#define _WLOCALE_DEFINED 1
__LIBC wchar_t *(__LIBCCALL _wsetlocale)(int __category, wchar_t const *__locale);
__LIBC __locale_t (__LIBCCALL _wcreate_locale)(int __category, wchar_t const *__locale);
#endif /* !_WLOCALE_DEFINED */
#endif /* __USE_DOS */

__DECL_END

#endif /* !_LOCALE_H */
