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
#ifndef GUARD_LIBS_LIBC_LOCALE_C
#define GUARD_LIBS_LIBC_LOCALE_C 1

#include "libc.h"
#include "locale.h"
#include <locale.h>
#include <hybrid/compiler.h>
#include <limits.h>
#include <stdbool.h>

DECL_BEGIN

PRIVATE struct lconv current_conv = {
    .int_frac_digits    = CHAR_MAX,
    .frac_digits        = CHAR_MAX,
    .p_cs_precedes      = CHAR_MAX,
    .p_sep_by_space     = CHAR_MAX,
    .n_cs_precedes      = CHAR_MAX,
    .n_sep_by_space     = CHAR_MAX,
    .p_sign_posn        = CHAR_MAX,
    .n_sign_posn        = CHAR_MAX,
    .int_p_cs_precedes  = CHAR_MAX,
    .int_p_sep_by_space = CHAR_MAX,
    .int_n_cs_precedes  = CHAR_MAX,
    .int_n_sep_by_space = CHAR_MAX,
    .int_p_sign_posn    = CHAR_MAX,
    .int_n_sign_posn    = CHAR_MAX,
};
PRIVATE bool is_initialized;

INTERN char *LIBCCALL
libc_setlocale(int category, const char *locale) {
 NOT_IMPLEMENTED(); return (char *)locale;
}
INTERN struct lconv *LIBCCALL libc_localeconv(void) {
 if (!is_initialized) {
  /* Return the equivalent of the "C" locale. */
  current_conv.decimal_point     = ".";
  current_conv.thousands_sep     = "";
  current_conv.grouping          = "";
  current_conv.int_curr_symbol   = "";
  current_conv.currency_symbol   = "";
  current_conv.mon_decimal_point = "";
  current_conv.mon_thousands_sep = "";
  current_conv.mon_grouping      = "";
  current_conv.positive_sign     = "";
  current_conv.negative_sign     = "";
  is_initialized = true;
 }
 return &current_conv;
}
INTERN locale_t LIBCCALL
libc_newlocale(int category_mask, const char *locale, locale_t base) {
 NOT_IMPLEMENTED();
 return base;
}
INTERN locale_t LIBCCALL libc_duplocale(locale_t dataset) {
 NOT_IMPLEMENTED();
 return dataset;
}
INTERN void LIBCCALL libc_freelocale(locale_t dataset) {
 NOT_IMPLEMENTED();
}
INTERN locale_t LIBCCALL libc_uselocale(locale_t dataset) {
 NOT_IMPLEMENTED();
 return dataset;
}
INTERN char *LIBCCALL libc_nl_langinfo(nl_item item) {
 NOT_IMPLEMENTED();
 return (char *)"";
}
INTERN char *LIBCCALL libc_nl_langinfo_l(nl_item item, locale_t l) {
 NOT_IMPLEMENTED();
 return (char *)"";
}


DEFINE_PUBLIC_ALIAS(setlocale,libc_setlocale);
DEFINE_PUBLIC_ALIAS(localeconv,libc_localeconv);
DEFINE_PUBLIC_ALIAS(newlocale,libc_newlocale);
DEFINE_PUBLIC_ALIAS(duplocale,libc_duplocale);
DEFINE_PUBLIC_ALIAS(freelocale,libc_freelocale);
DEFINE_PUBLIC_ALIAS(uselocale,libc_uselocale);
DEFINE_PUBLIC_ALIAS(nl_langinfo,libc_nl_langinfo);
DEFINE_PUBLIC_ALIAS(nl_langinfo_l,libc_nl_langinfo_l);

DECL_END

#endif /* !GUARD_LIBS_LIBC_LOCALE_C */
