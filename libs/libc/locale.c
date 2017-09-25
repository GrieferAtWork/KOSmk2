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
#include "errno.h"
#include "unicode.h"
#include "string.h"
#include "malloc.h"

#include <locale.h>
#include <langinfo.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
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
libc_setlocale(int category, char const *locale) {
 libc_syslog(LOG_DEBUG,"LIBC: Set locale: %q\n",locale);
 /*NOT_IMPLEMENTED();*/
 return (char *)locale;
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
libc_newlocale(int category_mask, char const *locale, locale_t base) {
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
 libc_syslog(LOG_DEBUG,"LIBC: Lookup langinfo: %x (%d)\n",item,item);
 switch (item) {
 case CODESET: return "UTF-8";
 default: break;
 }
 /*NOT_IMPLEMENTED();*/
 return (char *)"";
}
INTERN char *LIBCCALL libc_nl_langinfo_l(nl_item item, locale_t l) {
 NOT_IMPLEMENTED();
 return libc_nl_langinfo(item);
}

DEFINE_PUBLIC_ALIAS(setlocale,libc_setlocale);
DEFINE_PUBLIC_ALIAS(localeconv,libc_localeconv);
DEFINE_PUBLIC_ALIAS(newlocale,libc_newlocale);
DEFINE_PUBLIC_ALIAS(duplocale,libc_duplocale);
DEFINE_PUBLIC_ALIAS(freelocale,libc_freelocale);
DEFINE_PUBLIC_ALIAS(uselocale,libc_uselocale);
DEFINE_PUBLIC_ALIAS(nl_langinfo,libc_nl_langinfo);
DEFINE_PUBLIC_ALIAS(nl_langinfo_l,libc_nl_langinfo_l);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
PRIVATE ATTR_DOSRODATA u8 const dos_localcategory[] = {
    [__DOS_LC_ALL]      = __LC_ALL,
    [__DOS_LC_COLLATE]  = __LC_COLLATE,
    [__DOS_LC_CTYPE]    = __LC_CTYPE,
    [__DOS_LC_MONETARY] = __LC_MONETARY,
    [__DOS_LC_NUMERIC]  = __LC_NUMERIC,
    [__DOS_LC_TIME]     = __LC_TIME,
};
PRIVATE ATTR_DOSRODATA int const dos_localmask[] = {
    [__DOS_LC_ALL]      = LC_ALL_MASK,
    [__DOS_LC_COLLATE]  = LC_COLLATE_MASK,
    [__DOS_LC_CTYPE]    = LC_CTYPE_MASK,
    [__DOS_LC_MONETARY] = LC_MONETARY_MASK,
    [__DOS_LC_NUMERIC]  = LC_NUMERIC_MASK,
    [__DOS_LC_TIME]     = LC_TIME_MASK,
};
INTDEF struct lconv *LIBCCALL libc_dos_localeconv(void) {
 /* TODO: The binary layout of 'struct lconv' is different in DOS. */
 return libc_localeconv();
}

INTERN char16_t *LIBCCALL libc_dos_16wsetlocale(int dos_category, char16_t const *locale) {
 char *locale_name; char16_t *result = (char16_t *)locale;
 if unlikely((unsigned int)dos_category >= COMPILER_LENOF(dos_localcategory)) { SET_ERRNO(EINVAL); return NULL; }
 locale_name = libc_utf16to8m(locale,libc_16wcslen(locale));
 if unlikely(!locale_name) return NULL;
 if (!libc_setlocale(dos_localcategory[dos_category],locale_name))
      result = NULL;
 libc_free(locale_name);
 return result;
}
INTERN char32_t *LIBCCALL libc_dos_32wsetlocale(int dos_category, char32_t const *locale) {
 char *locale_name; char32_t *result = (char32_t *)locale;
 if unlikely((unsigned int)dos_category >= COMPILER_LENOF(dos_localcategory)) { SET_ERRNO(EINVAL); return NULL; }
 locale_name = libc_utf32to8m(locale,libc_32wcslen(locale));
 if unlikely(!locale_name) return NULL;
 if (!libc_setlocale(dos_localcategory[dos_category],locale_name))
      result = NULL;
 libc_free(locale_name);
 return result;
}
INTDEF locale_t LIBCCALL libc_dos_create_locale(int dos_category, char const *locale) {
 if unlikely((unsigned int)dos_category >= COMPILER_LENOF(dos_localmask)) { SET_ERRNO(EINVAL); return NULL; }
 return libc_newlocale(dos_localmask[dos_category],locale,NULL);
}
INTERN locale_t LIBCCALL libc_dos_16wcreate_locale(int dos_category, char16_t const *locale) {
 locale_t result; char *locale_name;
 if unlikely((unsigned int)dos_category >= COMPILER_LENOF(dos_localmask)) { SET_ERRNO(EINVAL); return NULL; }
 locale_name = libc_utf16to8m(locale,libc_16wcslen(locale));
 if unlikely(!locale_name) return NULL;
 result = libc_newlocale(dos_localmask[dos_category],locale_name,NULL);
 libc_free(locale_name);
 return result;
}
INTERN locale_t LIBCCALL libc_dos_32wcreate_locale(int dos_category, char32_t const *locale) {
 locale_t result; char *locale_name;
 if unlikely((unsigned int)dos_category >= COMPILER_LENOF(dos_localmask)) { SET_ERRNO(EINVAL); return NULL; }
 locale_name = libc_utf32to8m(locale,libc_32wcslen(locale));
 if unlikely(!locale_name) return NULL;
 result = libc_newlocale(dos_localmask[dos_category],locale_name,NULL);
 libc_free(locale_name);
 return result;
}
INTDEF int LIBCCALL libc_dos_configthreadlocale(int flag) { NOT_IMPLEMENTED(); return 0; }
INTDEF locale_t LIBCCALL libc_dos_get_current_locale(void) { NOT_IMPLEMENTED(); return NULL; }

DEFINE_PUBLIC_ALIAS(_wsetlocale,libc_dos_32wsetlocale);
DEFINE_PUBLIC_ALIAS(_wcreate_locale,libc_dos_32wcreate_locale);
DEFINE_PUBLIC_ALIAS(__DSYM(localeconv),libc_dos_localeconv);
DEFINE_PUBLIC_ALIAS(__DSYM(_wsetlocale),libc_dos_16wsetlocale);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcreate_locale),libc_dos_16wcreate_locale);
DEFINE_PUBLIC_ALIAS(_configthreadlocale,libc_dos_configthreadlocale);
DEFINE_PUBLIC_ALIAS(_get_current_locale,libc_dos_get_current_locale);
DEFINE_PUBLIC_ALIAS(_create_locale,libc_dos_create_locale);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_LOCALE_C */
