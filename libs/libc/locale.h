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
#ifndef GUARD_LIBS_LIBC_LOCALE_H
#define GUARD_LIBS_LIBC_LOCALE_H 1

#include "libc.h"
#include <hybrid/compiler.h>
#include <xlocale.h>
#include <langinfo.h>
#include <uchar.h>

DECL_BEGIN

INTDEF char *LIBCCALL libc_setlocale(int category, char const *locale);
INTDEF struct lconv *LIBCCALL libc_localeconv(void);
INTDEF locale_t LIBCCALL libc_newlocale(int category_mask, char const *locale, locale_t base);
INTDEF locale_t LIBCCALL libc_duplocale(locale_t dataset);
INTDEF void LIBCCALL libc_freelocale(locale_t dataset);
INTDEF locale_t LIBCCALL libc_uselocale(locale_t dataset);
INTDEF char *LIBCCALL libc_nl_langinfo(nl_item item);
INTDEF char *LIBCCALL libc_nl_langinfo_l(nl_item item, locale_t l);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF struct lconv *LIBCCALL libc_dos_localeconv(void);
INTDEF char16_t *LIBCCALL libc_dos_16wsetlocale(int dos_category, char16_t const *locale);
INTDEF char32_t *LIBCCALL libc_dos_32wsetlocale(int dos_category, char32_t const *locale);
INTDEF locale_t LIBCCALL libc_dos_16wcreate_locale(int dos_category, char16_t const *locale);
INTDEF locale_t LIBCCALL libc_dos_32wcreate_locale(int dos_category, char32_t const *locale);
INTDEF int LIBCCALL libc_dos_configthreadlocale(int flag);
INTDEF locale_t LIBCCALL libc_dos_get_current_locale(void);
INTDEF locale_t LIBCCALL libc_dos_create_locale(int dos_category, char const *locale);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_LOCALE_H */
