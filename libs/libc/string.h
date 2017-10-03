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
#ifndef GUARD_LIBS_LIBC_STRING_H
#define GUARD_LIBS_LIBC_STRING_H 1

#include "libc.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/debuginfo.h>
#include <hybrid/typecore.h>
#include <stdarg.h>

#ifndef __KERNEL__
#include <xlocale.h>
#include <uchar.h>
#endif /* !__KERNEL__ */

DECL_BEGIN

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_BEGIN
struct tm;
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */

INTDEF void *LIBCCALL libc_memcpy(void *__restrict dst, void const *__restrict src, size_t n);
INTDEF void *LIBCCALL libc__memcpy_d(void *__restrict dst, void const *__restrict src, size_t n, DEBUGINFO);
INTDEF void *LIBCCALL libc_mempcpy(void *__restrict dst, void const *__restrict src, size_t n);
INTDEF void *LIBCCALL libc__mempcpy_d(void *__restrict dst, void const *__restrict src, size_t n, DEBUGINFO);
INTDEF void *LIBCCALL libc_memmove(void *dst, void const *src, size_t n_bytes);
INTDEF void *LIBCCALL libc_memset(void *__restrict dst, int byte, size_t n);
INTDEF int LIBCCALL libc_memcmp(void const *a, void const *b, size_t n);
INTDEF void *LIBCCALL libc_memchr(void const *__restrict haystack, int needle, size_t n);
INTDEF void *LIBCCALL libc_memrchr(void const *__restrict haystack, int needle, size_t n);
INTDEF void *LIBCCALL libc_memend(void const *__restrict haystack, int needle, size_t n);
INTDEF void *LIBCCALL libc_memrend(void const *__restrict haystack, int needle, size_t n);
INTDEF size_t LIBCCALL libc_memlen(void const *__restrict haystack, int needle, size_t n);
INTDEF size_t LIBCCALL libc_memrlen(void const *__restrict haystack, int needle, size_t n);
INTDEF void *LIBCCALL libc_rawmemchr(void const *__restrict haystack, int needle);
INTDEF void *LIBCCALL libc_rawmemrchr(void const *__restrict haystack, int needle);
INTDEF size_t LIBCCALL libc_rawmemlen(void const *__restrict haystack, int needle);
INTDEF size_t LIBCCALL libc_rawmemrlen(void const *__restrict haystack, int needle);

INTDEF void *LIBCCALL libc_memcpyw(void *__restrict dst, void const *__restrict src, size_t n_words);
INTDEF void *LIBCCALL libc__memcpyw_d(void *__restrict dst, void const *__restrict src, size_t n_words, DEBUGINFO);
INTDEF void *LIBCCALL libc_mempcpyw(void *__restrict dst, void const *__restrict src, size_t n_words);
INTDEF void *LIBCCALL libc__mempcpyw_d(void *__restrict dst, void const *__restrict src, size_t n_words, DEBUGINFO);
INTDEF void *LIBCCALL libc_memmovew(void *dst, void const *src, size_t n_words);
INTDEF void *LIBCCALL libc_memsetw(void *__restrict dst, u16 word, size_t n_words);
INTDEF s16 LIBCCALL libc_memcmpw(void const *a, void const *b, size_t n_words);
INTDEF u16 *LIBCCALL libc_memchrw(u16 const *__restrict haystack, u16 needle, size_t n_words);
INTDEF u16 *LIBCCALL libc_memrchrw(u16 const *__restrict haystack, u16 needle, size_t n_words);
INTDEF u16 *LIBCCALL libc_memendw(u16 const *__restrict haystack, u16 needle, size_t n_words);
INTDEF u16 *LIBCCALL libc_memrendw(u16 const *__restrict haystack, u16 needle, size_t n_words);
INTDEF size_t LIBCCALL libc_memlenw(u16 const *__restrict haystack, u16 needle, size_t n_words);
INTDEF size_t LIBCCALL libc_memrlenw(u16 const *__restrict haystack, u16 needle, size_t n_words);
INTDEF u16 *LIBCCALL libc_rawmemchrw(u16 const *__restrict haystack, u16 needle);
INTDEF u16 *LIBCCALL libc_rawmemrchrw(u16 const *__restrict haystack, u16 needle);
INTDEF size_t LIBCCALL libc_rawmemlenw(u16 const *__restrict haystack, u16 needle);
INTDEF size_t LIBCCALL libc_rawmemrlenw(u16 const *__restrict haystack, u16 needle);

INTDEF void *LIBCCALL libc_memcpyl(void *__restrict dst, void const *__restrict src, size_t n_dwords);
INTDEF void *LIBCCALL libc__memcpyl_d(void *__restrict dst, void const *__restrict src, size_t n_dwords, DEBUGINFO);
INTDEF void *LIBCCALL libc_mempcpyl(void *__restrict dst, void const *__restrict src, size_t n_dwords);
INTDEF void *LIBCCALL libc__mempcpyl_d(void *__restrict dst, void const *__restrict src, size_t n_dwords, DEBUGINFO);
INTDEF void *LIBCCALL libc_memmovel(void *dst, void const *src, size_t n_dwords);
INTDEF void *LIBCCALL libc_memsetl(void *__restrict dst, u32 dword, size_t n_dwords);
INTDEF s32 LIBCCALL libc_memcmpl(void const *a, void const *b, size_t n_dwords);
INTDEF u32 *LIBCCALL libc_memchrl(u32 const *__restrict haystack, u32 needle, size_t n_dwords);
INTDEF u32 *LIBCCALL libc_memrchrl(u32 const *__restrict haystack, u32 needle, size_t n_dwords);
INTDEF u32 *LIBCCALL libc_memendl(u32 const *__restrict haystack, u32 needle, size_t n_dwords);
INTDEF u32 *LIBCCALL libc_memrendl(u32 const *__restrict haystack, u32 needle, size_t n_dwords);
INTDEF size_t LIBCCALL libc_memlenl(u32 const *__restrict haystack, u32 needle, size_t n_dwords);
INTDEF size_t LIBCCALL libc_memrlenl(u32 const *__restrict haystack, u32 needle, size_t n_dwords);
INTDEF u32 *LIBCCALL libc_rawmemchrl(u32 const *__restrict haystack, u32 needle);
INTDEF u32 *LIBCCALL libc_rawmemrchrl(u32 const *__restrict haystack, u32 needle);
INTDEF size_t LIBCCALL libc_rawmemlenl(u32 const *__restrict haystack, u32 needle);
INTDEF size_t LIBCCALL libc_rawmemrlenl(u32 const *__restrict haystack, u32 needle);

#ifdef CONFIG_64BIT_STRING
INTDEF void *LIBCCALL libc_memcpyq(void *__restrict dst, void const *__restrict src, size_t n_qwords);
INTDEF void *LIBCCALL libc__memcpyq_d(void *__restrict dst, void const *__restrict src, size_t n_qwords, DEBUGINFO);
INTDEF void *LIBCCALL libc_mempcpyq(void *__restrict dst, void const *__restrict src, size_t n_qwords);
INTDEF void *LIBCCALL libc__mempcpyq_d(void *__restrict dst, void const *__restrict src, size_t n_qwords, DEBUGINFO);
INTDEF void *LIBCCALL libc_memmoveq(void *dst, void const *src, size_t n_dwords);
INTDEF void *LIBCCALL libc_memsetq(void *__restrict dst, u64 qword, size_t n_qwords);
INTDEF s64 LIBCCALL libc_memcmpq(void const *a, void const *b, size_t n_qwords);
INTDEF u64 *LIBCCALL libc_memchrq(u64 const *__restrict haystack, u64 needle, size_t n_qwords);
INTDEF u64 *LIBCCALL libc_memrchrq(u64 const *__restrict haystack, u64 needle, size_t n_qwords);
INTDEF u64 *LIBCCALL libc_memendq(u64 const *__restrict haystack, u64 needle, size_t n_qwords);
INTDEF u64 *LIBCCALL libc_memrendq(u64 const *__restrict haystack, u64 needle, size_t n_qwords);
INTDEF size_t LIBCCALL libc_memlenq(u64 const *__restrict haystack, u64 needle, size_t n_qwords);
INTDEF size_t LIBCCALL libc_memrlenq(u64 const *__restrict haystack, u64 needle, size_t n_qwords);
INTDEF u64 *LIBCCALL libc_rawmemchrq(u64 const *__restrict haystack, u64 needle);
INTDEF u64 *LIBCCALL libc_rawmemrchrq(u64 const *__restrict haystack, u64 needle);
INTDEF size_t LIBCCALL libc_rawmemlenq(u64 const *__restrict haystack, u64 needle);
INTDEF size_t LIBCCALL libc_rawmemrlenq(u64 const *__restrict haystack, u64 needle);
#endif /* CONFIG_64BIT_STRING */

INTDEF void *LIBCCALL libc_mempatw(void *__restrict dst, u16 pattern, size_t n_bytes);
INTDEF void *LIBCCALL libc_mempatl(void *__restrict dst, u32 pattern, size_t n_bytes);

INTDEF char *LIBCCALL libc_strend(char const *__restrict str);
INTDEF char *LIBCCALL libc_strnend(char const *__restrict str, size_t maxlen);
INTDEF size_t LIBCCALL libc_strlen(char const *__restrict str);
INTDEF size_t LIBCCALL libc_strnlen(char const *__restrict str, size_t maxlen);
INTDEF char *LIBCCALL libc_strchrnul(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_strchr(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_strrchr(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_strrchrnul(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_strnchr(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF char *LIBCCALL libc_strnrchr(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF char *LIBCCALL libc_strnchrnul(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF char *LIBCCALL libc_strnrchrnul(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF size_t LIBCCALL libc_stroff(char const *__restrict haystack, int needle);
INTDEF size_t LIBCCALL libc_strroff(char const *__restrict haystack, int needle);
INTDEF size_t LIBCCALL libc_strnoff(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF size_t LIBCCALL libc_strnroff(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF char *LIBCCALL libc_stpcpy(char *__restrict dst, char const *__restrict src);
INTDEF char *LIBCCALL libc_stpncpy(char *__restrict dst, char const *__restrict src, size_t n);
INTDEF int LIBCCALL libc_strcmp(char const *s1, char const *s2);
INTDEF int LIBCCALL libc_strncmp(char const *s1, char const *s2, size_t n);
INTDEF int LIBCCALL libc_strcasecmp(char const *s1, char const *s2);
INTDEF int LIBCCALL libc_strncasecmp(char const *s1, char const *s2, size_t n);
INTDEF char *LIBCCALL libc_strstr(char const *haystack, char const *needle);
INTDEF char *LIBCCALL libc_strcasestr(char const *haystack, char const *needle);
INTDEF void *LIBCCALL libc_memmem(void const *haystack, size_t haystacklen, void const *needle, size_t needlelen);
INTDEF size_t LIBCCALL libc_fuzzy_memcasecmp(void const *a, size_t a_bytes, void const *b, size_t b_bytes);
INTDEF size_t LIBCCALL libc_fuzzy_memcmp(void const *a, size_t a_bytes, void const *b, size_t b_bytes);
INTDEF size_t LIBCCALL libc_fuzzy_strcmp(char const *a, char const *b);
INTDEF size_t LIBCCALL libc_fuzzy_strncmp(char const *a, size_t max_a_chars, char const *b, size_t max_b_chars);
INTDEF size_t LIBCCALL libc_fuzzy_strcasecmp(char const *a, char const *b);
INTDEF size_t LIBCCALL libc_fuzzy_strncasecmp(char const *a, size_t max_a_chars, char const *b, size_t max_b_chars);
INTDEF int LIBCCALL libc___ffs8(s8 i);
INTDEF int LIBCCALL libc___ffs16(s16 i);
INTDEF int LIBCCALL libc___ffs32(s32 i);
INTDEF int LIBCCALL libc___ffs64(s64 i);
INTDEF int LIBCCALL libc_strverscmp(char const *s1, char const *s2);
INTDEF char *LIBCCALL libc_strsep(char **__restrict stringp, char const *__restrict delim);
INTDEF int LIBCCALL libc_memcasecmp(void const *s1, void const *s2, size_t n_bytes);
#ifndef __KERNEL__
INTDEF void LIBCCALL libc_bcopy(void const *src, void *dst, size_t n);
INTDEF void LIBCCALL libc_bzero(void *__restrict s, size_t n);
INTDEF void LIBCCALL libc_swab(void const *__restrict from, void *__restrict to, size_t n_bytes);
INTDEF char *LIBCCALL libc_strcpy(char *__restrict dst, char const *__restrict src);
INTDEF char *LIBCCALL libc_strncpy(char *__restrict dst, char const *__restrict src, size_t n);
INTDEF char *LIBCCALL libc_index(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_rindex(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_dirname(char *__restrict path);
INTDEF char *LIBCCALL libc___xpg_basename(char *__restrict path);
INTDEF char *LIBCCALL libc_basename(char const *__restrict path);
INTDEF char *LIBCCALL libc_strcat(char *__restrict dst, char const *__restrict src);
INTDEF char *LIBCCALL libc_strncat(char *__restrict dst, char const *__restrict src, size_t n);
INTDEF size_t LIBCCALL libc_strcspn(char const *s, char const *reject);
INTDEF char *LIBCCALL libc_strpbrk(char const *s, char const *accept);
INTDEF char *LIBCCALL libc_strtok_r(char *__restrict s, char const *__restrict delim, char **__restrict save_ptr);
INTDEF char *LIBCCALL libc_strtok(char *__restrict s, char const *__restrict delim);
INTDEF size_t LIBCCALL libc_strspn(char const *s, char const *accept);
INTDEF void *LIBCCALL libc_memccpy(void *__restrict dst, void const *__restrict src, int c, size_t n);
INTDEF char const signal_names[][10];
INTDEF char *LIBCCALL libc_strsignal(int sig);
INTDEF int LIBCCALL libc_strcoll(char const *s1, char const *s2);
INTDEF size_t LIBCCALL libc_strxfrm(char *__restrict dst, char const *__restrict src, size_t n);
INTDEF char *LIBCCALL libc_strfry(char *__restrict string);
INTDEF void *LIBCCALL libc_memfrob(void *__restrict s, size_t n);
INTDEF int LIBCCALL libc_strcoll_l(char const *s1, char const *s2, locale_t l);
INTDEF size_t LIBCCALL libc_strxfrm_l(char *__restrict dst, char const *__restrict src, size_t n, locale_t l);
INTDEF char *LIBCCALL libc_strerror_l(int errnum, locale_t l);
INTDEF int LIBCCALL libc_strcasecmp_l(char const *s1, char const *s2, locale_t loc);
INTDEF int LIBCCALL libc_strncasecmp_l(char const *s1, char const *s2, size_t n, locale_t loc);
INTDEF char *ATTR_CDECL libc_strdupaf(char const *__restrict format, ...);
INTDEF char *LIBCCALL libc_vstrdupaf(char const *__restrict format, va_list args);
#endif /* !__KERNEL__ */

INTDEF char *LIBCCALL libc_gcvt(double value, int ndigit, char *buf);
INTDEF char *LIBCCALL libc_qgcvt(long double value, int ndigit, char *buf);
INTDEF int LIBCCALL libc_ecvt_r(double value, int ndigit, int *__restrict decpt, int *__restrict sign, char *__restrict buf, size_t len);
INTDEF int LIBCCALL libc_fcvt_r(double value, int ndigit, int *__restrict decpt, int *__restrict sign, char *__restrict buf, size_t len);
INTDEF int LIBCCALL libc_qecvt_r(long double value, int ndigit, int *__restrict decpt, int *__restrict sign, char *__restrict buf, size_t len);
INTDEF int LIBCCALL libc_qfcvt_r(long double value, int ndigit, int *__restrict decpt, int *__restrict sign, char *__restrict buf, size_t len);
INTDEF float LIBCCALL libc_strtof(char const *__restrict nptr, char **__restrict endptr);
INTDEF double LIBCCALL libc_strtod(char const *__restrict nptr, char **__restrict endptr);
INTDEF long double LIBCCALL libc_strtold(char const *__restrict nptr, char **__restrict endptr);
INTDEF s32 LIBCCALL libc_strto32(char const *__restrict nptr, char **__restrict endptr, int base);
INTDEF u32 LIBCCALL libc_strtou32(char const *__restrict nptr, char **__restrict endptr, int base);
INTDEF s64 LIBCCALL libc_strto64(char const *__restrict nptr, char **__restrict endptr, int base);
INTDEF u64 LIBCCALL libc_strtou64(char const *__restrict nptr, char **__restrict endptr, int base);
#ifndef __KERNEL__
INTDEF char *LIBCCALL libc_qecvt(long double value, int ndigit, int *__restrict decpt, int *__restrict sign);
INTDEF char *LIBCCALL libc_qfcvt(long double value, int ndigit, int *__restrict decpt, int *__restrict sign);
INTDEF char *LIBCCALL libc_ecvt(double value, int ndigit, int *__restrict decpt, int *__restrict sign);
INTDEF char *LIBCCALL libc_fcvt(double value, int ndigit, int *__restrict decpt, int *__restrict sign);
INTDEF float LIBCCALL libc_strtof_l(char const *__restrict nptr, char **__restrict endptr, locale_t loc);
INTDEF double LIBCCALL libc_strtod_l(char const *__restrict nptr, char **__restrict endptr, locale_t loc);
INTDEF long double LIBCCALL libc_strtold_l(char const *__restrict nptr, char **__restrict endptr, locale_t loc);
INTDEF double LIBCCALL libc_atof(char const *__restrict nptr);
INTDEF s32 LIBCCALL libc_ato32(char const *__restrict nptr);
INTDEF s64 LIBCCALL libc_ato64(char const *__restrict nptr);
INTDEF s32 LIBCCALL libc_strto32_l(char const *__restrict nptr, char **__restrict endptr, int base, locale_t loc);
INTDEF u32 LIBCCALL libc_strtou32_l(char const *__restrict nptr, char **__restrict endptr, int base, locale_t loc);
INTDEF s64 LIBCCALL libc_strto64_l(char const *__restrict nptr, char **__restrict endptr, int base, locale_t loc);
INTDEF u64 LIBCCALL libc_strtou64_l(char const *__restrict nptr, char **__restrict endptr, int base, locale_t loc);
#endif /* !__KERNEL__ */



#define STRERROR_VERSION 0
#define ERRNOSTR_NAME   offsetof(struct errnotext_entry,ete_name)
#define ERRNOSTR_TEXT   offsetof(struct errnotext_entry,ete_text)
struct errnotext_entry {
 u16       ete_name; /*< Offset into 'sed_strtab', to the name of the error. */
 u16       ete_text; /*< Offset into 'sed_strtab', to a string describing the error. */
};
struct errnotext_data {
 uintptr_t              etd_version; /*< == STRERROR_VERSION (Strerror-data version; will always be backwards-compatible). */
 uintptr_t              etd_strtab;  /*< Offset from 'strerror_data' to a string table. */
 uintptr_t              etd_enotab;  /*< Offset from 'strerror_data' to a vector of 'struct errnotext_entry'. */
 size_t                 etd_enocnt;  /*< Amount of entires in 'etd_enotab'. */
 size_t                 etd_enoent;  /*< Size of a single entry within 'etd_enotab'. */
};

#ifndef __KERNEL__
INTDEF struct errnotext_data const *p_errnotext;
#else
DATDEF struct errnotext_data const errnotext;
#endif
INTDEF char const *LIBCCALL strerror_get(uintptr_t kind, int no);
INTDEF char const *LIBCCALL libc_strerror_s(int errnum);
INTDEF char const *LIBCCALL libc_strerrorname_s(int errnum);
INTDEF char *LIBCCALL libc_strerror(int errnum);
INTDEF int LIBCCALL libc___xpg_strerror_r(int errnum, char *buf, size_t buflen);
INTDEF char *LIBCCALL libc_strerror_r(int errnum, char *buf, size_t buflen);

#ifndef __KERNEL__

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */
struct __mbstate;
                  
INTDEF char *LIBCCALL libc_strlwr(char *__restrict str);
INTDEF char *LIBCCALL libc_strupr(char *__restrict str);
INTDEF char *LIBCCALL libc_strset(char *__restrict str, int chr);
INTDEF char *LIBCCALL libc_strnset(char *__restrict str, int chr, size_t maxlen);
INTDEF char *LIBCCALL libc_strrev(char *__restrict str);
INTDEF int LIBCCALL libc_strcasecoll(char const *str1, char const *str2);
INTDEF int LIBCCALL libc_strncoll(char const *str1, char const *str2, size_t maxlen);
INTDEF int LIBCCALL libc_strncasecoll(char const *str1, char const *str2, size_t maxlen);
INTDEF char *LIBCCALL libc_strlwr_l(char *__restrict str, locale_t lc);
INTDEF char *LIBCCALL libc_strupr_l(char *__restrict str, locale_t lc);
INTDEF int LIBCCALL libc_strcasecoll_l(char const *str1, char const *str2, locale_t lc);
INTDEF int LIBCCALL libc_strncoll_l(char const *str1, char const *str2, size_t maxlen, locale_t lc);
INTDEF int LIBCCALL libc_strncasecoll_l(char const *str1, char const *str2, size_t maxlen, locale_t lc);
INTDEF int LIBCCALL libc_memcasecmp_l(void const *a, void const *b, size_t n_bytes, locale_t lc);

/* General-purpose wide-string API functions. */
INTDEF size_t LIBCCALL libc___ctype_get_mb_cur_max(void);
INTDEF wint_t LIBCCALL libc_btowc(int c);
INTDEF int LIBCCALL libc_wctob(wint_t c);
INTDEF int LIBCCALL libc_mbsinit(struct __mbstate const *ps);

/* General-purpose unicode API functions. */
INTDEF size_t LIBCCALL libc_mbrtoc16(char16_t *__restrict pc16, char const *__restrict s, size_t n, struct __mbstate *__restrict p);
INTDEF size_t LIBCCALL libc_mbrtoc32(char32_t *__restrict pc32, char const *__restrict s, size_t n, struct __mbstate *__restrict p);
INTDEF size_t LIBCCALL libc_c16rtomb(char *__restrict s, char16_t c16, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_c32rtomb(char *__restrict s, char32_t c32, struct __mbstate *__restrict ps);

INTDEF char32_t *LIBCCALL libc_32wcpcpy(char32_t *__restrict dst, char32_t const *__restrict src);
INTDEF char32_t *LIBCCALL libc_32wcpncpy(char32_t *__restrict dst, char32_t const *__restrict src, size_t n);
INTDEF char32_t *LIBCCALL libc_32wcscat(char32_t *__restrict dst, char32_t const *__restrict src);
INTDEF char32_t *LIBCCALL libc_32wcschr(char32_t const *__restrict haystack, char32_t wc);
INTDEF char32_t *LIBCCALL libc_32wcschrnul(char32_t const *__restrict s, char32_t wc);
INTDEF char32_t *LIBCCALL libc_32wcscpy(char32_t *__restrict dst, char32_t const *__restrict src);
INTDEF char32_t *LIBCCALL libc_32wcsdup(char32_t const *__restrict str);
INTDEF char32_t *LIBCCALL libc_32wcsend(char32_t const *__restrict s);
INTDEF char32_t *LIBCCALL libc_32wcsncat(char32_t *__restrict dst, char32_t const *__restrict src, size_t n);
INTDEF char32_t *LIBCCALL libc_32wcsncpy(char32_t *__restrict dst, char32_t const *__restrict src, size_t n);
INTDEF char32_t *LIBCCALL libc_32wcsnend(char32_t const *__restrict s, size_t maxlen);
INTDEF char32_t *LIBCCALL libc_32wcspbrk(char32_t const *haystack, char32_t const *accept);
INTDEF char32_t *LIBCCALL libc_32wcsrchr(char32_t const *__restrict haystack, char32_t wc);
INTDEF char32_t *LIBCCALL libc_32wcsstr(char32_t const *haystack, char32_t const *needle);
INTDEF char32_t *LIBCCALL libc_32wcstok(char32_t *__restrict s, char32_t const *__restrict delim);
INTDEF char32_t *LIBCCALL libc_32wcstok_r(char32_t *__restrict s, char32_t const *__restrict delim, char32_t **__restrict ptr);
INTDEF char32_t *LIBCCALL libc_32wmemchr(char32_t const *__restrict s, char32_t c, size_t n);
INTDEF char32_t *LIBCCALL libc_32wmemcpy(char32_t *__restrict s1, char32_t const *__restrict s2, size_t n);
INTDEF char32_t *LIBCCALL libc_32wmemmove(char32_t *s1, char32_t const *s2, size_t n);
INTDEF char32_t *LIBCCALL libc_32wmempcpy(char32_t *__restrict s1, char32_t const *__restrict s2, size_t n);
INTDEF char32_t *LIBCCALL libc_32wmemset(char32_t *__restrict s, char32_t c, size_t n);
INTDEF int LIBCCALL libc_32wcscasecmp(char32_t const *s1, char32_t const *s2);
INTDEF int LIBCCALL libc_32wcscasecmp_l(char32_t const *s1, char32_t const *s2, locale_t loc);
INTDEF int LIBCCALL libc_32wcscmp(char32_t const *s1, char32_t const *s2);
INTDEF int LIBCCALL libc_32wcscoll(char32_t const *s1, char32_t const *s2);
INTDEF int LIBCCALL libc_32wcscoll_l(char32_t const *s1, char32_t const *s2, locale_t loc);
INTDEF int LIBCCALL libc_32wcsncasecmp(char32_t const *s1, char32_t const *s2, size_t n);
INTDEF int LIBCCALL libc_32wcsncasecmp_l(char32_t const *s1, char32_t const *s2, size_t n, locale_t loc);
INTDEF int LIBCCALL libc_32wcsncmp(char32_t const *s1, char32_t const *s2, size_t n);
INTDEF int LIBCCALL libc_32wmemcmp(char32_t const *s1, char32_t const *s2, size_t n);
INTDEF size_t LIBCCALL libc_32mblen(char const *__restrict s, size_t n);
INTDEF size_t LIBCCALL libc_32mbrlen(char const *__restrict s, size_t n, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_32mbrtowc(char32_t *__restrict pwc, char const *__restrict s, size_t n, struct __mbstate *__restrict p);
INTDEF size_t LIBCCALL libc_32mbsnrtowcs(char32_t *__restrict dst, char const **__restrict src, size_t nmc, size_t len, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_32mbsrtowcs(char32_t *__restrict dst, char const **__restrict src, size_t len, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_32mbstowcs(char32_t *__restrict pwcs, char const *__restrict s, size_t n);
INTDEF size_t LIBCCALL libc_32mbtowc(char32_t *__restrict pwc, char const *__restrict s, size_t n);
INTDEF size_t LIBCCALL libc_32wcrtomb(char *__restrict s, char32_t wc, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_32wcscspn(char32_t const *haystack, char32_t const *reject);
INTDEF size_t LIBCCALL libc_32wcslen(char32_t const *__restrict s);
INTDEF size_t LIBCCALL libc_32wcsnlen(char32_t const *__restrict s, size_t maxlen);
INTDEF size_t LIBCCALL libc_32wcsnrtombs(char *__restrict dst, char32_t const **__restrict src, size_t nwc, size_t len, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_32wcsrtombs(char *__restrict dst, char32_t const **__restrict src, size_t len, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_32wcsspn(char32_t const *haystack, char32_t const *accept);
INTDEF size_t LIBCCALL libc_32wcstombs(char *__restrict dst, char32_t const *__restrict pwcs, size_t n);
INTDEF size_t LIBCCALL libc_32wcswidth(char32_t const *__restrict s, size_t n);
INTDEF size_t LIBCCALL libc_32wcsxfrm(char32_t *__restrict s1, char32_t const *__restrict s2, size_t n);
INTDEF size_t LIBCCALL libc_32wcsxfrm_l(char32_t *__restrict s1, char32_t const *__restrict s2, size_t n, locale_t loc);
INTDEF size_t LIBCCALL libc_32wctomb(char *__restrict s, char32_t wchar);
INTDEF size_t LIBCCALL libc_32wcwidth(char32_t c);

INTDEF s32 LIBCCALL libc_32wcsto32(char32_t const *__restrict nptr, char32_t **__restrict endptr, int base);
INTDEF s32 LIBCCALL libc_32wcsto32_l(char32_t const *__restrict nptr, char32_t **__restrict endptr, int base, locale_t loc);
INTDEF u32 LIBCCALL libc_32wcstou32(char32_t const *__restrict nptr, char32_t **__restrict endptr, int base);
INTDEF u32 LIBCCALL libc_32wcstou32_l(char32_t const *__restrict nptr, char32_t **__restrict endptr, int base, locale_t loc);
INTDEF s64 LIBCCALL libc_32wcsto64(char32_t const *__restrict nptr, char32_t **__restrict endptr, int base);
INTDEF s64 LIBCCALL libc_32wcsto64_l(char32_t const *__restrict nptr, char32_t **__restrict endptr, int base, locale_t loc);
INTDEF u64 LIBCCALL libc_32wcstou64(char32_t const *__restrict nptr, char32_t **__restrict endptr, int base);
INTDEF u64 LIBCCALL libc_32wcstou64_l(char32_t const *__restrict nptr, char32_t **__restrict endptr, int base, locale_t loc);

INTDEF float LIBCCALL libc_32wcstof(char32_t const *__restrict nptr, char32_t **__restrict endptr);
INTDEF float LIBCCALL libc_32wcstof_l(char32_t const *__restrict nptr, char32_t **__restrict endptr, locale_t loc);
INTDEF double LIBCCALL libc_32wcstod(char32_t const *__restrict nptr, char32_t **__restrict endptr);
INTDEF double LIBCCALL libc_32wcstod_l(char32_t const *__restrict nptr, char32_t **__restrict endptr, locale_t loc);
INTDEF long double LIBCCALL libc_32wcstold(char32_t const *__restrict nptr, char32_t **__restrict endptr);
INTDEF long double LIBCCALL libc_32wcstold_l(char32_t const *__restrict nptr, char32_t **__restrict endptr, locale_t loc);

#ifndef CONFIG_LIBC_NO_DOS_LIBC

/* Define misc. functions found in DOS's <stdlib.h> header. */
INTDEF size_t LIBCCALL libc_mbstrlen(char const *str);
INTDEF size_t LIBCCALL libc_mbstrnlen(char const *str, size_t maxlen);
INTDEF size_t LIBCCALL libc_mbstrlen_l(char const *str, locale_t locale);
INTDEF size_t LIBCCALL libc_mbstrnlen_l(char const *str, size_t maxlen, locale_t locale);
INTDEF size_t LIBCCALL libc_16mbtowc_l(char16_t *dst, char const *src, size_t srclen, locale_t locale);
INTDEF errno_t LIBCCALL libc_16mbstowcs_s(size_t *presult, char16_t *buf, size_t buflen, char const *src, size_t maxlen);
INTDEF errno_t LIBCCALL libc_16mbstowcs_s_l(size_t *presult, char16_t *buf, size_t buflen, char const *src, size_t maxlen, locale_t locale);
INTDEF size_t LIBCCALL libc_16mbstowcs_l(char16_t *buf, char const *src, size_t maxlen, locale_t locale);
INTDEF size_t LIBCCALL libc_32mbtowc_l(char32_t *dst, char const *src, size_t srclen, locale_t locale);
INTDEF errno_t LIBCCALL libc_32mbstowcs_s(size_t *presult, char32_t * buf, size_t buflen, char const *src, size_t maxlen);
INTDEF errno_t LIBCCALL libc_32mbstowcs_s_l(size_t *presult, char32_t *buf, size_t buflen, char const *src, size_t maxlen, locale_t locale);
INTDEF size_t LIBCCALL libc_32mbstowcs_l(char32_t *buf, char const *src, size_t maxlen, locale_t locale);

/* ... */
INTDEF size_t LIBCCALL libc_16wctomb_l(char *buf, char16_t wc, locale_t locale);
INTDEF errno_t LIBCCALL libc_16wctomb_s(int *presult, char *buf, size_t buflen, char16_t wc);
INTDEF errno_t LIBCCALL libc_16wctomb_s_l(int *presult, char *buf, size_t buflen, char16_t wc, locale_t locale);
INTDEF errno_t LIBCCALL libc_16wcstombs_s(size_t *presult, char *buf, size_t buflen, char16_t const *src, size_t maxlen);
INTDEF errno_t LIBCCALL libc_16wcstombs_s_l(size_t *presult, char *buf, size_t buflen, char16_t const *src, size_t maxlen, locale_t locale);
INTDEF size_t LIBCCALL libc_16wcstombs_l(char *dst, char16_t const *src, size_t maxlen, locale_t locale);
INTDEF size_t LIBCCALL libc_32wctomb_l(char *buf, char32_t wc, locale_t locale);
INTDEF errno_t LIBCCALL libc_32wctomb_s(int *presult, char *buf, size_t buflen, char32_t wc);
INTDEF errno_t LIBCCALL libc_32wctomb_s_l(int *presult, char *buf, size_t buflen, char32_t wc, locale_t locale);
INTDEF errno_t LIBCCALL libc_32wcstombs_s(size_t *presult, char *buf, size_t buflen, char32_t const *src, size_t maxlen);
INTDEF errno_t LIBCCALL libc_32wcstombs_s_l(size_t *presult, char *buf, size_t buflen, char32_t const *src, size_t maxlen, locale_t locale);
INTDEF size_t LIBCCALL libc_32wcstombs_l(char *dst, char32_t const *src, size_t maxlen, locale_t locale);

INTDEF double LIBCCALL libc_atof_l(const char *__restrict nptr, locale_t locale);
INTDEF s32 LIBCCALL libc_ato32_l(const char *__restrict nptr, locale_t locale);
INTDEF s64 LIBCCALL libc_ato64_l(const char *__restrict nptr, locale_t locale);

INTDEF errno_t LIBCCALL libc_strlwr_s(char *__restrict str, size_t buflen);
INTDEF errno_t LIBCCALL libc_strlwr_s_l(char *__restrict str, size_t buflen, locale_t locale);
INTDEF errno_t LIBCCALL libc_strupr_s(char *__restrict str, size_t buflen);
INTDEF errno_t LIBCCALL libc_strupr_s_l(char *__restrict str, size_t buflen, locale_t locale);
INTDEF errno_t LIBCCALL libc_strnset_s(char *__restrict str, size_t buflen, char val, size_t maxlen);
INTDEF errno_t LIBCCALL libc_strset_s(char *__restrict str, size_t buflen, char val);


/* DOS takes an integer as argument to 'swab()' */
INTDEF void LIBCCALL libc_swab_int(void const *__restrict from, void *__restrict to, int n_bytes);

INTDEF char *LIBCCALL libc_s32toa(s32 val, char *dst, int radix);
INTDEF char *LIBCCALL libc_s64toa(s64 val, char *dst, int radix);
INTDEF char *LIBCCALL libc_u32toa(u32 val, char *dst, int radix);
INTDEF char *LIBCCALL libc_u64toa(u64 val, char *dst, int radix);
INTDEF errno_t LIBCCALL libc_s32toa_s(s32 val, char *dst, size_t bufsize, int radix);
INTDEF errno_t LIBCCALL libc_s64toa_s(s64 val, char *dst, size_t bufsize, int radix);
INTDEF errno_t LIBCCALL libc_u32toa_s(u32 val, char *dst, size_t bufsize, int radix);
INTDEF errno_t LIBCCALL libc_u64toa_s(u64 val, char *dst, size_t bufsize, int radix);
INTDEF char16_t *LIBCCALL libc_16s32tow(s32 val, char16_t *dst, int radix);
INTDEF char16_t *LIBCCALL libc_16s64tow(s64 val, char16_t *dst, int radix);
INTDEF char16_t *LIBCCALL libc_16u32tow(u32 val, char16_t *dst, int radix);
INTDEF char16_t *LIBCCALL libc_16u64tow(u64 val, char16_t *dst, int radix);
INTDEF errno_t LIBCCALL libc_16s32tow_s(s32 val, char16_t *dst, size_t bufsize, int radix);
INTDEF errno_t LIBCCALL libc_16s64tow_s(s64 val, char16_t *dst, size_t bufsize, int radix);
INTDEF errno_t LIBCCALL libc_16u32tow_s(u32 val, char16_t *dst, size_t bufsize, int radix);
INTDEF errno_t LIBCCALL libc_16u64tow_s(u64 val, char16_t *dst, size_t bufsize, int radix);
INTDEF char32_t *LIBCCALL libc_32s32tow(s32 val, char32_t *dst, int radix);
INTDEF char32_t *LIBCCALL libc_32s64tow(s64 val, char32_t *dst, int radix);
INTDEF char32_t *LIBCCALL libc_32u32tow(u32 val, char32_t *dst, int radix);
INTDEF char32_t *LIBCCALL libc_32u64tow(u64 val, char32_t *dst, int radix);
INTDEF errno_t LIBCCALL libc_32s32tow_s(s32 val, char32_t *dst, size_t bufsize, int radix);
INTDEF errno_t LIBCCALL libc_32s64tow_s(s64 val, char32_t *dst, size_t bufsize, int radix);
INTDEF errno_t LIBCCALL libc_32u32tow_s(u32 val, char32_t *dst, size_t bufsize, int radix);
INTDEF errno_t LIBCCALL libc_32u64tow_s(u64 val, char32_t *dst, size_t bufsize, int radix);

/* Additional DOS 32-bit string functions */
INTDEF char32_t *LIBCCALL libc_32wcslwr(char32_t *__restrict str);
INTDEF char32_t *LIBCCALL libc_32wcslwr_l(char32_t *__restrict str, locale_t locale);
INTDEF char32_t *LIBCCALL libc_32wcsnset(char32_t *__restrict str, char32_t needle, size_t maxlen);
INTDEF char32_t *LIBCCALL libc_32wcsrev(char32_t *__restrict str);
INTDEF char32_t *LIBCCALL libc_32wcsset(char32_t *__restrict str, char32_t needle);
INTDEF char32_t *LIBCCALL libc_32wcsupr(char32_t *__restrict str);
INTDEF char32_t *LIBCCALL libc_32wcsupr_l(char32_t *__restrict str, locale_t locale);
INTDEF int LIBCCALL libc_32wcscasecoll(char32_t const *str1, char32_t const *str2);
INTDEF int LIBCCALL libc_32wcscasecoll_l(char32_t const *str1, char32_t const *str2, locale_t locale);
INTDEF int LIBCCALL libc_32wcsncasecoll(char32_t const *str1, char32_t const *str2, size_t maxlen);
INTDEF int LIBCCALL libc_32wcsncasecoll_l(char32_t const *str1, char32_t const *str2, size_t maxlen, locale_t locale);
INTDEF int LIBCCALL libc_32wcsncoll(char32_t const *str1, char32_t const *str2, size_t maxlen);
INTDEF int LIBCCALL libc_32wcsncoll_l(char32_t const *str1, char32_t const *str2, size_t maxlen, locale_t locale);

INTDEF char16_t *LIBCCALL libc_16wcpcpy(char16_t *__restrict dst, char16_t const *__restrict src);
INTDEF char16_t *LIBCCALL libc_16wcpncpy(char16_t *__restrict dst, char16_t const *__restrict src, size_t n);
INTDEF char16_t *LIBCCALL libc_16wcscat(char16_t *__restrict dst, char16_t const *__restrict src);
INTDEF char16_t *LIBCCALL libc_16wcschr(char16_t const *__restrict str, char16_t needle);
INTDEF char16_t *LIBCCALL libc_16wcschrnul(char16_t const *__restrict s, char16_t wc);
INTDEF char16_t *LIBCCALL libc_16wcscpy(char16_t *__restrict dst, char16_t const *__restrict src);
INTDEF char16_t *LIBCCALL libc_16wcsdup(char16_t const *__restrict str);
INTDEF char16_t *LIBCCALL libc_16wcsend(char16_t const *__restrict s);
INTDEF char16_t *LIBCCALL libc_16wcslwr(char16_t *__restrict str);
INTDEF char16_t *LIBCCALL libc_16wcslwr_l(char16_t *__restrict str, locale_t locale);
INTDEF char16_t *LIBCCALL libc_16wcsncat(char16_t *__restrict dst, char16_t const *__restrict src, size_t maxlen);
INTDEF char16_t *LIBCCALL libc_16wcsncpy(char16_t *__restrict dst, char16_t const *__restrict src, size_t maxlen);
INTDEF char16_t *LIBCCALL libc_16wcsnend(char16_t const *__restrict s, size_t maxlen);
INTDEF char16_t *LIBCCALL libc_16wcsnset(char16_t *__restrict str, char16_t needle, size_t maxlen);
INTDEF char16_t *LIBCCALL libc_16wcspbrk(char16_t const *__restrict str, char16_t const *reject);
INTDEF char16_t *LIBCCALL libc_16wcsrchr(char16_t const *__restrict str, char16_t needle);
INTDEF char16_t *LIBCCALL libc_16wcsrev(char16_t *__restrict str);
INTDEF char16_t *LIBCCALL libc_16wcsset(char16_t *__restrict str, char16_t needle);
INTDEF char16_t *LIBCCALL libc_16wcsstr(char16_t const *haystack, char16_t const *needle);
INTDEF char16_t *LIBCCALL libc_16wcstok(char16_t *__restrict str, char16_t const *__restrict delim);
INTDEF char16_t *LIBCCALL libc_16wcstok_r(char16_t *__restrict str, char16_t const *__restrict delim, char16_t **__restrict ptr);
INTDEF char16_t *LIBCCALL libc_16wcsupr(char16_t *__restrict str);
INTDEF char16_t *LIBCCALL libc_16wcsupr_l(char16_t *__restrict str, locale_t locale);
INTDEF char16_t *LIBCCALL libc_16wmemchr(char16_t const *__restrict s, char16_t c, size_t n);
INTDEF char16_t *LIBCCALL libc_16wmemcpy(char16_t *__restrict s1, char16_t const *__restrict s2, size_t n);
INTDEF char16_t *LIBCCALL libc_16wmemmove(char16_t *s1, char16_t const *s2, size_t n);
INTDEF char16_t *LIBCCALL libc_16wmempcpy(char16_t *__restrict s1, char16_t const *__restrict s2, size_t n);
INTDEF char16_t *LIBCCALL libc_16wmemset(char16_t *__restrict s, char16_t c, size_t n);
INTDEF int LIBCCALL libc_16wcscasecmp(char16_t const *str1, char16_t const *str2);
INTDEF int LIBCCALL libc_16wcscasecmp_l(char16_t const *str1, char16_t const *str2, locale_t lc);
INTDEF int LIBCCALL libc_16wcscasecoll(char16_t const *str1, char16_t const *str2);
INTDEF int LIBCCALL libc_16wcscasecoll_l(char16_t const *str1, char16_t const *str2, locale_t locale);
INTDEF int LIBCCALL libc_16wcscmp(char16_t const *str1, char16_t const *str2);
INTDEF int LIBCCALL libc_16wcscoll(char16_t const *str1, char16_t const *str2);
INTDEF int LIBCCALL libc_16wcscoll_l(char16_t const *str1, char16_t const *str2, locale_t locale);
INTDEF int LIBCCALL libc_16wcsncasecmp(char16_t const *str1, char16_t const *str2, size_t maxlen);
INTDEF int LIBCCALL libc_16wcsncasecmp_l(char16_t const *str1, char16_t const *str2, size_t maxlen, locale_t lc);
INTDEF int LIBCCALL libc_16wcsncasecoll(char16_t const *str1, char16_t const *str2, size_t maxlen);
INTDEF int LIBCCALL libc_16wcsncasecoll_l(char16_t const *str1, char16_t const *str2, size_t maxlen, locale_t locale);
INTDEF int LIBCCALL libc_16wcsncmp(char16_t const *str1, char16_t const *str2, size_t maxlen);
INTDEF int LIBCCALL libc_16wcsncoll(char16_t const *str1, char16_t const *str2, size_t maxlen);
INTDEF int LIBCCALL libc_16wcsncoll_l(char16_t const *str1, char16_t const *str2, size_t maxlen, locale_t locale);
INTDEF int LIBCCALL libc_16wmemcmp(char16_t const *s1, char16_t const *s2, size_t n);
INTDEF size_t LIBCCALL libc_16mblen(char const *__restrict s, size_t n);
INTDEF size_t LIBCCALL libc_16mbrlen(char const *__restrict s, size_t n, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_16mbrtowc(char16_t *__restrict pwc, char const *__restrict s, size_t n, struct __mbstate *__restrict p);
INTDEF size_t LIBCCALL libc_16mbsnrtowcs(char16_t *__restrict dst, char const **__restrict src, size_t nmc, size_t len, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_16mbsrtowcs(char16_t *__restrict dst, char const **__restrict src, size_t len, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_16mbstowcs(char16_t *__restrict pwcs, char const *__restrict s, size_t n);
INTDEF size_t LIBCCALL libc_16mbtowc(char16_t *__restrict pwc, char const *__restrict s, size_t n);
INTDEF size_t LIBCCALL libc_16wcrtomb(char *__restrict s, char16_t wc, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_16wcscspn(char16_t const *str, char16_t const *reject);
INTDEF size_t LIBCCALL libc_16wcslen(char16_t const *__restrict str);
INTDEF size_t LIBCCALL libc_16wcsnlen(char16_t const *__restrict src, size_t maxlen);
INTDEF size_t LIBCCALL libc_16wcsnrtombs(char *__restrict dst, char16_t const **__restrict src, size_t nwc, size_t len, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_16wcsrtombs(char *__restrict dst, char16_t const **__restrict src, size_t len, struct __mbstate *__restrict ps);
INTDEF size_t LIBCCALL libc_16wcsspn(char16_t const *str, char16_t const *reject);
INTDEF size_t LIBCCALL libc_16wcstombs(char *__restrict dst, char16_t const *__restrict pwcs, size_t n);
INTDEF size_t LIBCCALL libc_16wcswidth(char16_t const *__restrict s, size_t n);
INTDEF size_t LIBCCALL libc_16wcsxfrm(char16_t *__restrict dst, char16_t const *__restrict src, size_t maxlen);
INTDEF size_t LIBCCALL libc_16wcsxfrm_l(char16_t *__restrict dst, char16_t const *__restrict src, size_t maxlen, locale_t locale);
INTDEF size_t LIBCCALL libc_16wctomb(char *__restrict s, char16_t wchar);
INTDEF size_t LIBCCALL libc_16wcwidth(char16_t c);

INTDEF size_t LIBCCALL libc_32mblen_l(char const *__restrict s, size_t n, locale_t loc);
INTDEF size_t LIBCCALL libc_16mblen_l(char const *__restrict s, size_t n, locale_t loc);

/* DOS-SLIB functions. */
INTDEF errno_t LIBCCALL libc_16mbsrtowcs_s(size_t *result, char16_t *__restrict buf, size_t buflen, char const **__restrict psrc, size_t srcsize, mbstate_t *__restrict ps);
INTDEF errno_t LIBCCALL libc_16memcpy_s(void *__restrict dst, size_t dstsize, void const *__restrict src, size_t srcsize);
INTDEF errno_t LIBCCALL libc_16memmove_s(void *dst, size_t dstsize, void const *src, size_t srcsize);
INTDEF errno_t LIBCCALL libc_16wcrtomb_s(size_t *result, char *__restrict buf, size_t buflen, char16_t wc, mbstate_t *__restrict ps);
INTDEF errno_t LIBCCALL libc_16wcscat_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src);
INTDEF errno_t LIBCCALL libc_16wcscpy_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src);
INTDEF errno_t LIBCCALL libc_16wcslwr_s(char16_t *__restrict str, size_t buflen);
INTDEF errno_t LIBCCALL libc_16wcslwr_s_l(char16_t *__restrict str, size_t buflen, locale_t locale);
INTDEF errno_t LIBCCALL libc_16wcsupr_s(char16_t *__restrict str, size_t buflen);
INTDEF errno_t LIBCCALL libc_16wcsupr_s_l(char16_t *__restrict str, size_t buflen, locale_t locale);
INTDEF errno_t LIBCCALL libc_16wcsncat_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src, size_t maxlen);
INTDEF errno_t LIBCCALL libc_16wcsncpy_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src, size_t maxlen);
INTDEF errno_t LIBCCALL libc_16wcsnset_s(char16_t *__restrict str, size_t buflen, char16_t val, size_t maxlen);
INTDEF errno_t LIBCCALL libc_16wcsrtombs_s(size_t *result, char *__restrict buf, size_t buflen, char16_t const **__restrict psrc, size_t srcsize, mbstate_t *__restrict ps);
INTDEF errno_t LIBCCALL libc_16wcsset_s(char16_t *__restrict str, size_t buflen, char16_t val);
INTDEF errno_t LIBCCALL libc_16wmemcpy_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src, size_t srcsize);
INTDEF errno_t LIBCCALL libc_16wmemmove_s(char16_t *dst, size_t dstsize, char16_t const *src, size_t srcsize);
INTDEF errno_t LIBCCALL libc_32mbsrtowcs_s(size_t *result, char32_t *__restrict buf, size_t buflen, char const **__restrict psrc, size_t srcsize, mbstate_t *__restrict ps);
INTDEF errno_t LIBCCALL libc_32memcpy_s(void *__restrict dst, size_t dstsize, void const *__restrict src, size_t srcsize);
INTDEF errno_t LIBCCALL libc_32memmove_s(void *dst, size_t dstsize, void const *src, size_t srcsize);
INTDEF errno_t LIBCCALL libc_32wcrtomb_s(size_t *result, char *__restrict buf, size_t buflen, char32_t wc, mbstate_t *__restrict ps);
INTDEF errno_t LIBCCALL libc_32wcscat_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src);
INTDEF errno_t LIBCCALL libc_32wcscpy_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src);
INTDEF errno_t LIBCCALL libc_32wcslwr_s(char32_t *__restrict str, size_t buflen);
INTDEF errno_t LIBCCALL libc_32wcslwr_s_l(char32_t *__restrict str, size_t buflen, locale_t locale);
INTDEF errno_t LIBCCALL libc_32wcsupr_s(char32_t *__restrict str, size_t buflen);
INTDEF errno_t LIBCCALL libc_32wcsupr_s_l(char32_t *__restrict str, size_t buflen, locale_t locale);
INTDEF errno_t LIBCCALL libc_32wcsncat_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src, size_t maxlen);
INTDEF errno_t LIBCCALL libc_32wcsncpy_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src, size_t maxlen);
INTDEF errno_t LIBCCALL libc_32wcsnset_s(char32_t *__restrict str, size_t buflen, char32_t val, size_t maxlen);
INTDEF errno_t LIBCCALL libc_32wcsrtombs_s(size_t *result, char *__restrict buf, size_t buflen, char32_t const **__restrict psrc, size_t srcsize, mbstate_t *__restrict ps);
INTDEF errno_t LIBCCALL libc_32wcsset_s(char32_t *__restrict str, size_t buflen, char32_t val);
INTDEF errno_t LIBCCALL libc_32wmemcpy_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src, size_t srcsize);
INTDEF errno_t LIBCCALL libc_32wmemmove_s(char32_t *dst, size_t dstsize, char32_t const *src, size_t srcsize);

INTDEF s32 LIBCCALL libc_32wto32(char32_t const *__restrict nptr);
INTDEF s32 LIBCCALL libc_32wto32_l(char32_t const *__restrict nptr, locale_t locale);
INTDEF s64 LIBCCALL libc_32wto64(char32_t const *__restrict nptr);
INTDEF s64 LIBCCALL libc_32wto64_l(char32_t const *__restrict nptr, locale_t locale);
INTDEF s32 LIBCCALL libc_16wto32(char16_t const *__restrict nptr);
INTDEF s32 LIBCCALL libc_16wto32_l(char16_t const *__restrict nptr, locale_t locale);
INTDEF s64 LIBCCALL libc_16wto64(char16_t const *__restrict nptr);
INTDEF s64 LIBCCALL libc_16wto64_l(char16_t const *__restrict nptr, locale_t locale);

INTDEF s32 LIBCCALL libc_16wcsto32(char16_t const *__restrict nptr, char16_t **__restrict endptr, int base);
INTDEF s32 LIBCCALL libc_16wcsto32_l(char16_t const *__restrict nptr, char16_t **__restrict endptr, int base, locale_t loc);
INTDEF u32 LIBCCALL libc_16wcstou32(char16_t const *__restrict nptr, char16_t **__restrict endptr, int base);
INTDEF u32 LIBCCALL libc_16wcstou32_l(char16_t const *__restrict nptr, char16_t **__restrict endptr, int base, locale_t loc);
INTDEF s64 LIBCCALL libc_16wcsto64(char16_t const *__restrict nptr, char16_t **__restrict endptr, int base);
INTDEF s64 LIBCCALL libc_16wcsto64_l(char16_t const *__restrict nptr, char16_t **__restrict endptr, int base, locale_t loc);
INTDEF u64 LIBCCALL libc_16wcstou64(char16_t const *__restrict nptr, char16_t **__restrict endptr, int base);
INTDEF u64 LIBCCALL libc_16wcstou64_l(char16_t const *__restrict nptr, char16_t **__restrict endptr, int base, locale_t loc);
INTDEF double LIBCCALL libc_16wcstod(char16_t const *__restrict nptr, char16_t **__restrict endptr);
INTDEF double LIBCCALL libc_16wcstod_l(char16_t const *__restrict nptr, char16_t **__restrict endptr, locale_t loc);
INTDEF float LIBCCALL libc_16wcstof(char16_t const *__restrict nptr, char16_t **__restrict endptr);
INTDEF float LIBCCALL libc_16wcstof_l(char16_t const *__restrict nptr, char16_t **__restrict endptr, locale_t loc);
INTDEF long double LIBCCALL libc_16wcstold(char16_t const *__restrict nptr, char16_t **__restrict endptr);
INTDEF long double LIBCCALL libc_16wcstold_l(char16_t const *__restrict nptr, char16_t **__restrict endptr, locale_t loc);
INTDEF double LIBCCALL libc_16wtof(char16_t const *__restrict s);
INTDEF double LIBCCALL libc_32wtof(char32_t const *__restrict s);
INTDEF double LIBCCALL libc_16wtof_l(char16_t const *__restrict s, locale_t locale);
INTDEF double LIBCCALL libc_32wtof_l(char32_t const *__restrict s, locale_t locale);

INTDEF void LIBCCALL libc_makepath(char *buf, const char *drive, const char *dir, const char *file, const char *ext);
INTDEF void LIBCCALL libc16_wmakepath(char16_t *__restrict buf, char16_t const *__restrict drive, char16_t const *__restrict dir, char16_t const *__restrict file, char16_t const *__restrict ext);
INTDEF void LIBCCALL libc32_wmakepath(char32_t *__restrict buf, char32_t const *__restrict drive, char32_t const *__restrict dir, char32_t const *__restrict file, char32_t const *__restrict ext);
INTDEF errno_t LIBCCALL libc_makepath_s(char *buf, size_t buflen, const char *drive, const char *dir, const char *file, const char *ext);
INTDEF errno_t LIBCCALL libc16_wmakepath_s(char16_t *__restrict buf, size_t maxlen, char16_t const *drive, char16_t const *__restrict dir, char16_t const *__restrict file, char16_t const *__restrict ext);
INTDEF errno_t LIBCCALL libc32_wmakepath_s(char32_t *__restrict buf, size_t maxlen, char32_t const *drive, char32_t const *__restrict dir, char32_t const *__restrict file, char32_t const *__restrict ext);

INTDEF void LIBCCALL _searchenv(const char *file, const char *__envvar, char *__resultpath);
INTDEF void LIBCCALL _splitpath(const char *__abspath, char *drive, char *dir, char *file, char *ext);
INTDEF errno_t LIBCCALL _searchenv_s(const char *file, const char *__envvar, char *__resultpath, size_t buflen);
INTDEF errno_t LIBCCALL _splitpath_s(const char *__abspath, char *drive, size_t __drivelen, char *dir, size_t __dirlen, char *file, size_t __filelen, char *ext, size_t __extlen);
INTDEF void LIBCCALL _wsearchenv(char16_t const *__restrict file, char16_t const *__restrict __varname,  char16_t *__restrict dst);
INTDEF void LIBCCALL _wsplitpath(char16_t const *__restrict __abspath, char16_t *__restrict drive, char16_t *__restrict dir, char16_t *__restrict file, char16_t *__restrict ext);
INTDEF errno_t LIBCCALL _wsearchenv_s(char16_t const *__restrict file, char16_t const *__restrict __varname, char16_t *__restrict dst, size_t maxlen);
INTDEF errno_t LIBCCALL _wsplitpath_s(char16_t const *__restrict __abspath, char16_t *__restrict drive, size_t __drivelen, char16_t *__restrict dir, size_t __dirlen, char16_t *__restrict file, size_t __filelen, char16_t *__restrict ext, size_t __extlen);



#if 0 /* TODO */
__LIBC wint_t (__LIBCCALL libc_16fgetwc)(__FILE *__stream);
__LIBC wint_t (__LIBCCALL libc_16getwc)(__FILE *__stream);
__LIBC wint_t (__LIBCCALL libc_16getwchar)(void);
__LIBC wint_t (__LIBCCALL libc_16fputwc)(char16_t wc, __FILE *__stream);
__LIBC wint_t (__LIBCCALL libc_16putwc)(char16_t wc, __FILE *__stream);
__LIBC wint_t (__LIBCCALL libc_16putwchar)(char16_t wc);
__LIBC char16_t *(__LIBCCALL libc_16fgetws)(char16_t *__restrict ws, int __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL libc_16fputws)(char16_t const *__restrict ws, __FILE *__restrict __stream);
__LIBC wint_t (__LIBCCALL libc_16ungetwc)(wint_t wc, __FILE *__stream);
__LIBC int (__LIBCCALL libc_16fwide)(__FILE *__fp, int __mode);
__LIBC ssize_t (__LIBCCALL libc_16fwprintf)(__FILE *__restrict __stream, char16_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL libc_16wprintf)(char16_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL libc_16swprintf)(char16_t *__restrict __s, size_t __n, char16_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL libc_16vfwprintf)(__FILE *__restrict __s, char16_t const *__restrict __format, __VA_LIST __arg);
__LIBC ssize_t (__LIBCCALL libc_16vwprintf)(char16_t const *__restrict __format, __VA_LIST __arg);
__LIBC ssize_t (__LIBCCALL libc_16vswprintf)(char16_t *__restrict __s, size_t __n, char16_t const *__restrict __format, __VA_LIST __arg);
__LIBC ssize_t (__LIBCCALL libc_16fwscanf)(__FILE *__restrict __stream, char16_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL libc_16wscanf)(char16_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL libc_16swscanf)(char16_t const *__restrict __s, char16_t const *__restrict __format, ...);
__LIBC ssize_t (__LIBCCALL libc_16vfwscanf)(__FILE *__restrict __s, char16_t const *__restrict __format, __VA_LIST __arg);
__LIBC ssize_t (__LIBCCALL libc_16vwscanf)(char16_t const *__restrict __format, __VA_LIST __arg);
__LIBC ssize_t (__LIBCCALL libc_16vswscanf)(char16_t const *__restrict __s, char16_t const *__restrict __format, __VA_LIST __arg);
__LIBC __FILE *(__LIBCCALL libc_16open_wmemstream)(char16_t **__bufloc, size_t *__sizeloc);
__LIBC wint_t (__LIBCCALL libc_16getwc_unlocked)(__FILE *__stream);
__LIBC wint_t (__LIBCCALL libc_16getwchar_unlocked)(void);
__LIBC wint_t (__LIBCCALL libc_16fgetwc_unlocked)(__FILE *__stream);
__LIBC wint_t (__LIBCCALL libc_16fputwc_unlocked)(char16_t wc, __FILE *__stream);
__LIBC wint_t (__LIBCCALL libc_16putwc_unlocked)(char16_t wc, __FILE *__stream);
__LIBC wint_t (__LIBCCALL libc_16putwchar_unlocked)(char16_t wc);
__LIBC char16_t *(__LIBCCALL libc_16fgetws_unlocked)(char16_t *__restrict ws, int __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL libc_16fputws_unlocked)(char16_t const *__restrict ws, __FILE *__restrict __stream);
#endif

#if 0 /* TODO */
__LIBC errno_t LIBCCALL _cgetws_s(char16_t *__buffer, size_t buflen, size_t *__sizeok) __ASMNAME("_getws_s");
__LIBC char16_t *LIBCCALL _cgetws(char16_t *__buffer) __ASMNAME("_getws");
__LIBC wint_t LIBCCALL _getwch(void) __ASMNAME("getwchar");
__LIBC wint_t LIBCCALL _getwch_nolock(void) __ASMNAME("getwchar_unlocked");
__LIBC wint_t LIBCCALL _getwche(void) __ASMNAME("getwchar");
__LIBC wint_t LIBCCALL _getwche_nolock(void) __ASMNAME("getwchar_unlocked");
__LIBC wint_t LIBCCALL _putwch(char16_t wc) __ASMNAME("putwchar");
__LIBC wint_t LIBCCALL _putwch_nolock(char16_t wc) __ASMNAME("putwchar_unlocked");
__LIBC wint_t LIBCCALL _ungetwch(wint_t wc);
__LIBC wint_t LIBCCALL _ungetwch_nolock(wint_t wc);
__LIBC int LIBCCALL _cputws(char16_t const *__string) __ASMNAME("_putws");
__LIBC int ATTR_CDECL _cwprintf(char16_t const *__format, ...) __ASMNAME("wprintf");
__LIBC int ATTR_CDECL _cwprintf_p(char16_t const *__format, ...) __ASMNAME("wprintf");
__LIBC int ATTR_CDECL _cwprintf_s(char16_t const *__format, ...) __ASMNAME("wprintf");
__LIBC int LIBCCALL _vcwprintf(char16_t const *__format, __VA_LIST __args) __ASMNAME("vwprintf");
__LIBC int LIBCCALL _vcwprintf_p(char16_t const*__format, __VA_LIST __args) __ASMNAME("vwprintf");
__LIBC int LIBCCALL _vcwprintf_s(char16_t const *__format, __VA_LIST __args) __ASMNAME("vwprintf");
__LIBC int ATTR_CDECL _cwprintf_l(char16_t const *__format, locale_t locale, ...) __ASMNAME("_wprintf_l");
__LIBC int ATTR_CDECL _cwprintf_p_l(char16_t const *__format, locale_t locale, ...) __ASMNAME("_wprintf_l");
__LIBC int ATTR_CDECL _cwprintf_s_l(char16_t const *__format, locale_t locale, ...) __ASMNAME("_wprintf_l");
__LIBC int LIBCCALL _vcwprintf_l(char16_t const *__format, locale_t locale, __VA_LIST __args) __ASMNAME("_vwprintf_l");
__LIBC int LIBCCALL _vcwprintf_p_l(char16_t const *__format, locale_t locale, __VA_LIST __args) __ASMNAME("_vwprintf_l");
__LIBC int LIBCCALL _vcwprintf_s_l(char16_t const *__format, locale_t locale, __VA_LIST __args) __ASMNAME("_vwprintf_l");
__LIBC int ATTR_CDECL _cwscanf(char16_t const *__format, ...) __ASMNAME("wscanf");
__LIBC int ATTR_CDECL _cwscanf_s(char16_t const *__format, ...) __ASMNAME("wscanf");
__LIBC int ATTR_CDECL _cwscanf_l(char16_t const *__format, locale_t locale, ...) __ASMNAME("_wscanf_l");
__LIBC int ATTR_CDECL _cwscanf_s_l(char16_t const *__format, locale_t locale, ...) __ASMNAME("_wscanf_l");
__LIBC FILE *LIBCCALL _wfdopen(int __fd, char16_t const *__restrict __mode);
__LIBC FILE *LIBCCALL _wfopen(char16_t const *file, char16_t const *__mode);
__LIBC FILE *LIBCCALL _wfreopen(char16_t const *file, char16_t const *__mode, FILE *__fp);
__LIBC FILE *LIBCCALL _wfsopen(char16_t const *file, char16_t const *__mode, int __sflag);
__LIBC FILE *LIBCCALL _wpopen(char16_t const *__cmd, char16_t const *__mode);
__LIBC errno_t LIBCCALL _wfopen_s(FILE **__restrict __pfp, char16_t const *file, char16_t const *__mode);
__LIBC errno_t LIBCCALL _wfreopen_s(FILE **__restrict __pfp, char16_t const *file, char16_t const *__mode, FILE *__restrict __fp);
__LIBC wint_t LIBCCALL getwchar(void);
__LIBC wint_t LIBCCALL putwchar(char16_t wc);
__LIBC wint_t LIBCCALL _fgetwchar(void) __ASMNAME("getwchar");
__LIBC wint_t LIBCCALL _fputwchar(char16_t wc) __ASMNAME("putwchar");
__LIBC wint_t LIBCCALL fgetwc(FILE *__restrict __fp);
__LIBC wint_t LIBCCALL fputwc(char16_t wc, FILE *__restrict __fp);
__LIBC wint_t LIBCCALL getwc(FILE *__restrict __fp) __ASMNAME("fgetwc");
__LIBC wint_t LIBCCALL putwc(char16_t wc, FILE *__restrict __fp) __ASMNAME("fputwc");
__LIBC wint_t LIBCCALL ungetwc(wint_t wc, FILE *__restrict __fp);
__LIBC char16_t *LIBCCALL _getws_s(char16_t *__restrict __s, size_t __dstlen);
__LIBC char16_t *LIBCCALL fgetws(char16_t *__restrict buf, int __dstlen, FILE *__restrict __fp);
__LIBC int LIBCCALL _putws(char16_t const *__restrict __s);
__LIBC int LIBCCALL fputws(char16_t const *__restrict __s, FILE *__restrict __fp);
__LIBC errno_t LIBCCALL _wtmpnam_s(char16_t *__restrict buf, size_t buflen);
__LIBC char16_t *LIBCCALL _wtempnam(char16_t const *__restrict dir, char16_t const *__restrict __prefix);
__LIBC char16_t *LIBCCALL _wtmpnam(char16_t *__restrict buf);
__LIBC wint_t LIBCCALL _fgetwc_nolock(FILE *__restrict __fp) __ASMNAME("fgetwc_unlocked");
__LIBC wint_t LIBCCALL _fputwc_nolock(char16_t wc, FILE *__restrict __fp) __ASMNAME("fputwc_unlocked");
__LIBC wint_t LIBCCALL _ungetwc_nolock(wint_t wc, FILE *__restrict __fp);
__LIBC int ATTR_CDECL _fwprintf_l(FILE *__restrict __fp, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _fwprintf_p(FILE *__restrict __fp, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL _fwprintf_p_l(FILE *__restrict __fp, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _fwprintf_s_l(FILE *__restrict __fp, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _fwscanf_l(FILE *__restrict __fp, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _fwscanf_s_l(FILE *__restrict __fp, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _scwprintf(char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL _scwprintf_l(char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _scwprintf_p(char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL _scwprintf_p_l(char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _snwprintf(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL _snwprintf_l(char16_t *__restrict buf, size_t maxlen, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _snwprintf_s(char16_t *__restrict buf, size_t buflen, size_t maxlen, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL _snwprintf_s_l(char16_t *__restrict buf, size_t dstsize, size_t maxlen, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _snwscanf(char16_t const *__restrict src, size_t maxlen, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL _snwscanf_l(char16_t const *__restrict src, size_t maxlen, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _snwscanf_s(char16_t const *__restrict src, size_t maxlen, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL _snwscanf_s_l(char16_t const *__restrict src, size_t maxlen, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _swprintf_c(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL _swprintf_c_l(char16_t *__restrict buf, size_t maxlen, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _swprintf_p(char16_t *__restrict buf, size_t maxlen, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL _swprintf_p_l(char16_t *__restrict buf, size_t maxlen, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _swprintf_s_l(char16_t *__restrict buf, size_t dstsize, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _swscanf_l(char16_t const *__restrict src, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _swscanf_s_l(char16_t const *__restrict src, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _wprintf_l(char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _wprintf_p(char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL _wprintf_p_l(char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _wprintf_s_l(char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _wscanf_l(char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL _wscanf_s_l(char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int ATTR_CDECL fwprintf(FILE *__restrict __fp, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL fwscanf(FILE *__restrict __fp, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL swprintf(char16_t *__restrict buf, size_t __count, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL swscanf(char16_t const *__restrict src, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL wprintf(char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL wscanf(char16_t const *__restrict __format, ...);
__LIBC int LIBCCALL _swprintf_l(char16_t *__restrict buf, size_t __count, char16_t const *__restrict __format, locale_t locale, ...);
__LIBC int LIBCCALL _vfwprintf_l(FILE *__restrict __fp, char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vfwprintf_p(FILE *__restrict __fp, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL _vfwprintf_p_l(FILE *__restrict __fp, char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vfwprintf_s_l(FILE *__restrict __fp, char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vscwprintf(char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL _vscwprintf_l(char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vscwprintf_p(char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL _vscwprintf_p_l(char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vsnwprintf(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict __format, __VA_LIST _Args);
__LIBC int LIBCCALL _vsnwprintf_l(char16_t *__restrict buf, size_t maxlen, char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vsnwprintf_s(char16_t *__restrict buf, size_t buflen, size_t maxlen, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL _vsnwprintf_s_l(char16_t *__restrict buf, size_t dstsize, size_t maxlen, char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vswprintf_c(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL _vswprintf_c_l(char16_t *__restrict buf, size_t maxlen, char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vswprintf_l(char16_t *__restrict buf, size_t __count, char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vswprintf_p(char16_t *__restrict buf, size_t maxlen, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL _vswprintf_p_l(char16_t *__restrict buf, size_t maxlen, char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vswprintf_s_l(char16_t *__restrict buf, size_t dstsize, char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vwprintf_l(char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vwprintf_p(char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL _vwprintf_p_l(char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _vwprintf_s_l(char16_t const *__restrict __format, locale_t locale, __VA_LIST __args);
__LIBC int LIBCCALL _wremove(char16_t const *__restrict file);
__LIBC int LIBCCALL vfwprintf(FILE *__restrict __fp, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL vfwscanf(FILE *__restrict __fp, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL vswprintf(char16_t *__restrict buf, size_t __count, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL vswscanf(char16_t const *__restrict src, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL vwprintf(char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL vwscanf(char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int ATTR_CDECL wprintf_s(char16_t const *__restrict __restrict __format, ...);
__LIBC int ATTR_CDECL swprintf_s(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL fwprintf_s(FILE *__restrict __fp, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL wscanf_s(char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL swscanf_s(char16_t const *__restrict src, char16_t const *__restrict __format, ...);
__LIBC int ATTR_CDECL fwscanf_s(FILE *__restrict __fp, char16_t const *__restrict __format, ...);
__LIBC int LIBCCALL vwprintf_s(char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL vswprintf_s(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL vfwprintf_s(FILE *__restrict __fp, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL vwscanf_s(char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL vswscanf_s(char16_t const *__restrict dst, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC int LIBCCALL vfwscanf_s(FILE *__restrict __fp, char16_t const *__restrict __format, __VA_LIST __args);
__LIBC char16_t *LIBCCALL _itow(int val, char16_t *__restrict dst, int __radix);
__LIBC char16_t *LIBCCALL _ultow(unsigned long val,  char16_t *__restrict dst, int __radix);
__LIBC char16_t *LIBCCALL _i64tow(__INT64_TYPE__ val, char16_t *__restrict dst, int __radix);
__LIBC char16_t *LIBCCALL _ui64tow(__UINT64_TYPE__ val, char16_t *__restrict dst, int __radix);
__LIBC errno_t LIBCCALL _itow_s(int val, char16_t *__restrict dst, size_t maxlen, int __radix);
__LIBC errno_t LIBCCALL _ultow_s(unsigned long val, char16_t *dst, size_t maxlen, int __radix);
__LIBC errno_t LIBCCALL _i64tow_s(__INT64_TYPE__ val, char16_t *__restrict dst, size_t maxlen, int __radix);
__LIBC errno_t LIBCCALL _ui64tow_s(__UINT64_TYPE__ val, char16_t *__restrict dst, size_t maxlen, int __radix);
__LIBC double LIBCCALL _wtof(char16_t const *__restrict __s);
__LIBC double LIBCCALL _wtof_l(char16_t const *__restrict __s, locale_t locale);
__LIBC char16_t *LIBCCALL _ltow(long val, char16_t *__restrict dst, int __radix) __ASMNAME("_itow");
__LIBC errno_t LIBCCALL _ltow_s(long val, char16_t *__restrict dst, size_t maxlen, int __radix) __ASMNAME("_itow_s");
__LIBC float LIBCCALL _wcstof_l(char16_t const *__restrict __s, char16_t **__restrict __pend, locale_t locale) __ASMNAME("wcstof_l");
__LIBC double LIBCCALL _wcstod_l(char16_t const *__restrict __s, char16_t **__pend, locale_t locale) __ASMNAME("wcstod_l");
__LIBC long double LIBCCALL _wcstold_l(char16_t const *__restrict __s, char16_t **__pend, locale_t locale) __ASMNAME("wcstold_l");
__LIBC char16_t *LIBCCALL _wgetenv(char16_t const *__restrict __varname);
__LIBC errno_t LIBCCALL _wgetenv_s(size_t *__restrict __psize, char16_t *__restrict buf, size_t buflen, char16_t const *__restrict __varname);
__LIBC errno_t LIBCCALL _wdupenv_s(char16_t **__restrict __pbuf, size_t *__restrict __pbuflen, char16_t const *__restrict __varname);
__LIBC int LIBCCALL _wsystem(char16_t const *__restrict __cmd);
__LIBC double LIBCCALL wcstod(char16_t const *__restrict __s, char16_t **__pend);
__LIBC float LIBCCALL wcstof(char16_t const *__restrict __s, char16_t **__pend);
__LIBC long double LIBCCALL wcstold(char16_t const *__restrict __s, char16_t **__pend);
__LIBC char16_t *LIBCCALL _wfullpath(char16_t *__abspath, char16_t const *__path, size_t maxlen);
__LIBC int LIBCCALL _wputenv(char16_t const *__envstr);
__LIBC errno_t LIBCCALL _wputenv_s(char16_t const *__restrict __name, char16_t const *__restrict val);

__LIBC void LIBCCALL _wperror(char16_t const *__restrict __errmsg);
__LIBC char16_t *LIBCCALL _wcserror(int __errnum);
__LIBC char16_t *LIBCCALL __wcserror(char16_t const *__restrict str);
__LIBC errno_t LIBCCALL _wcserror_s(char16_t *__restrict buf, size_t maxlen, int __errnum);
__LIBC errno_t LIBCCALL __wcserror_s(char16_t *__restrict buf, size_t maxlen, char16_t const *__restrict __errmsg);
__LIBC errno_t LIBCCALL _wasctime_s(char16_t buf[26], size_t maxlen, struct tm const *__restrict __ptm);
__LIBC errno_t LIBCCALL _wstrdate_s(char16_t buf[9], size_t maxlen);
__LIBC errno_t LIBCCALL _wstrtime_s(char16_t buf[9], size_t maxlen);
__LIBC char16_t *LIBCCALL _wasctime(struct tm const *__restrict __ptm);
__LIBC char16_t *LIBCCALL _wstrdate(char16_t *__restrict buf);
__LIBC char16_t *LIBCCALL _wstrtime(char16_t *__restrict buf);
__LIBC char16_t *LIBCCALL _wctime32(__time32_t const *__restrict __timer);
__LIBC char16_t *LIBCCALL _wctime64(__time64_t const *__restrict __timer);
__LIBC errno_t LIBCCALL _wctime32_s(char16_t buf[26], size_t maxlen, __time32_t const *__restrict __timer);
__LIBC errno_t LIBCCALL _wctime64_s(char16_t buf[26], size_t maxlen, __time64_t const *__restrict __timer);
#endif

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_STRING_H */
