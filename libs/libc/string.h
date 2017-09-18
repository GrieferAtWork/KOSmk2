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

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/debuginfo.h>
#include <xlocale.h>
#include <stdarg.h>

DECL_BEGIN

INTDEF void *LIBCCALL libc_memcpy(void *__restrict dst, void const *__restrict src, size_t n);
INTDEF void *LIBCCALL libc__memcpy_d(void *__restrict dst, void const *__restrict src, size_t n, DEBUGINFO);
INTDEF void *LIBCCALL libc_memset(void *dst, int byte, size_t n);
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
INTDEF void *LIBCCALL libc_memsetw(void *dst, u16 word, size_t n_words);
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
INTDEF void *LIBCCALL libc_memsetl(void *dst, u32 dword, size_t n_dwords);
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
INTDEF void *LIBCCALL libc_memsetq(void *dst, u64 qword, size_t n_qwords);
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

INTDEF void *LIBCCALL libc_memmove(void *dst, void const *src, size_t n_bytes);
INTDEF char *LIBCCALL libc_strend(char const *__restrict str);
INTDEF char *LIBCCALL libc_strnend(char const *__restrict str, size_t maxlen);
INTDEF size_t LIBCCALL libc_strlen(char const *__restrict str);
INTDEF size_t LIBCCALL libc_strnlen(char const *__restrict str, size_t maxlen);
INTDEF char *LIBCCALL libc_strchrnul(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_strchr(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_strrchr(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_strrchrnul(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_strnchr(char const *__restrict haystack, int needle, size_t max_chars);
INTDEF char *LIBCCALL libc_strnrchr(char const *__restrict haystack, int needle, size_t max_chars);
INTDEF char *LIBCCALL libc_strnchrnul(char const *__restrict haystack, int needle, size_t max_chars);
INTDEF char *LIBCCALL libc_strnrchrnul(char const *__restrict haystack, int needle, size_t max_chars);
INTDEF size_t LIBCCALL libc_stroff(char const *__restrict haystack, int needle);
INTDEF size_t LIBCCALL libc_strroff(char const *__restrict haystack, int needle);
INTDEF size_t LIBCCALL libc_strnoff(char const *__restrict haystack, int needle, size_t max_chars);
INTDEF size_t LIBCCALL libc_strnroff(char const *__restrict haystack, int needle, size_t max_chars);
INTDEF char *LIBCCALL libc_stpcpy(char *__restrict dst, char const *__restrict src);
INTDEF char *LIBCCALL libc_stpncpy(char *__restrict dst, char const *__restrict src, size_t n);
INTDEF int LIBCCALL libc_strcmp(char const *s1, char const *s2);
INTDEF int LIBCCALL libc_strncmp(char const *s1, char const *s2, size_t n);
INTDEF int LIBCCALL libc_strcasecmp(char const *s1, char const *s2);
INTDEF int LIBCCALL libc_strncasecmp(char const *s1, char const *s2, size_t n);
INTDEF void *LIBCCALL libc_mempcpy(void *__restrict dst, void const *__restrict src, size_t n);
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
#ifndef __KERNEL__
INTDEF void LIBCCALL libc_bcopy(void const *src, void *dst, size_t n);
INTDEF void LIBCCALL libc_bzero(void *s, size_t n);
INTDEF char *LIBCCALL libc_strcpy(char *__restrict dst, char const *__restrict src);
INTDEF char *LIBCCALL libc_strncpy(char *__restrict dst, char const *__restrict src, size_t n);
INTDEF char *LIBCCALL libc_index(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_rindex(char const *__restrict haystack, int needle);
INTDEF char *LIBCCALL libc_dirname(char *path);
INTDEF char *LIBCCALL libc___xpg_basename(char *path);
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
INTDEF char *LIBCCALL libc_strfry(char *string);
INTDEF void *LIBCCALL libc_memfrob(void *s, size_t n);
INTDEF int LIBCCALL libc_strcoll_l(char const *s1, char const *s2, locale_t l);
INTDEF size_t LIBCCALL libc_strxfrm_l(char *dst, char const *src, size_t n, locale_t l);
INTDEF char *LIBCCALL libc_strerror_l(int errnum, locale_t l);
INTDEF int LIBCCALL libc_strcasecmp_l(char const *s1, char const *s2, locale_t loc);
INTDEF int LIBCCALL libc_strncasecmp_l(char const *s1, char const *s2, size_t n, locale_t loc);
INTDEF char *ATTR_CDECL libc_strdupaf(char const *__restrict format, ...);
INTDEF char *LIBCCALL libc_vstrdupaf(char const *__restrict format, va_list args);
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
INTDEF struct errnotext_data const errnotext;
#endif
INTDEF char const *LIBCCALL strerror_get(uintptr_t kind, int no);
INTDEF char const *LIBCCALL libc_strerror_s(int errnum);
INTDEF char const *LIBCCALL libc_strerrorname_s(int errnum);
INTDEF char *LIBCCALL libc_strerror(int errnum);
INTDEF int LIBCCALL libc___xpg_strerror_r(int errnum, char *buf, size_t buflen);
INTDEF char *LIBCCALL libc_strerror_r(int errnum, char *buf, size_t buflen);

#ifndef __KERNEL__
#ifndef CONFIG_LIBC_NO_DOS_EXTENSIONS
INTDEF char *LIBCCALL libc_strlwr(char *str);
INTDEF char *LIBCCALL libc_strupr(char *str);
INTDEF char *LIBCCALL libc_strset(char *str, int chr);
INTDEF char *LIBCCALL libc_strnset(char *str, int chr, size_t max_chars);
INTDEF char *LIBCCALL libc_strrev(char *str);
INTDEF int LIBCCALL libc_strcasecoll(char const *str1, char const *str2);
INTDEF int LIBCCALL libc_strncoll(char const *str1, char const *str2, size_t max_chars);
INTDEF int LIBCCALL libc_strncasecoll(char const *str1, char const *str2, size_t max_chars);
INTDEF char *LIBCCALL libc_strlwr_l(char *str, locale_t lc);
INTDEF char *LIBCCALL libc_strupr_l(char *str, locale_t lc);
INTDEF int LIBCCALL libc_strcasecoll_l(char const *str1, char const *str2, locale_t lc);
INTDEF int LIBCCALL libc_strncoll_l(char const *str1, char const *str2, size_t max_chars, locale_t lc);
INTDEF int LIBCCALL libc_strncasecoll_l(char const *str1, char const *str2, size_t max_chars, locale_t lc);
INTDEF int LIBCCALL libc_memcasecmp(void const *s1, void const *s2, size_t n_bytes);
INTDEF int LIBCCALL libc_memcasecmp_l(void const *a, void const *b, size_t n_bytes, locale_t lc);

#ifndef CONFIG_LIBC_NO_WCHAR_STRING
typedef u16 dosch_t;
INTDEF int LIBCCALL libc_dos_wcscasecmp(dosch_t const *str1, dosch_t const *str2);
INTDEF int LIBCCALL libc_dos_wcscasecoll(dosch_t const *str1, dosch_t const *str2);
INTDEF int LIBCCALL libc_dos_wcscoll(dosch_t const *str1, dosch_t const *str2);
INTDEF int LIBCCALL libc_dos_wcsncasecmp(dosch_t const *str1, dosch_t const *str2, size_t max_chars);
INTDEF int LIBCCALL libc_dos_wcsncasecoll(dosch_t const *str1, dosch_t const *str2, size_t max_chars);
INTDEF int LIBCCALL libc_dos_wcscmp(dosch_t const *str1, dosch_t const *str2);
INTDEF int LIBCCALL libc_dos_wcsncmp(dosch_t const *str1, dosch_t const *str2, size_t max_chars);
INTDEF int LIBCCALL libc_dos_wcsncoll(dosch_t const *str1, dosch_t const *str2, size_t max_chars);
INTDEF size_t LIBCCALL libc_dos_wcscspn(dosch_t const *str, dosch_t const *reject);
INTDEF size_t LIBCCALL libc_dos_wcslen(dosch_t const *str);
INTDEF size_t LIBCCALL libc_dos_wcsnlen(dosch_t const *src, size_t max_chars);
INTDEF size_t LIBCCALL libc_dos_wcsspn(dosch_t const *str, dosch_t const *reject);
INTDEF size_t LIBCCALL libc_dos_wcsxfrm(dosch_t *dst, dosch_t const *src, size_t max_chars);
INTDEF dosch_t *LIBCCALL libc_dos_wcscpy(dosch_t *dst, dosch_t const *src);
INTDEF dosch_t *LIBCCALL libc_dos_wcscat(dosch_t *dst, dosch_t const *src);
INTDEF dosch_t *LIBCCALL libc_dos_wcsncat(dosch_t *dst, dosch_t const *src, size_t max_chars);
INTDEF dosch_t *LIBCCALL libc_dos_wcsncpy(dosch_t *dst, dosch_t const *src, size_t max_chars);
INTDEF dosch_t *LIBCCALL libc_dos_wcsdup(dosch_t const *str);
INTDEF dosch_t *LIBCCALL libc_dos_wcsnset(dosch_t *str, dosch_t chr, size_t max_chars);
INTDEF dosch_t *LIBCCALL libc_dos_wcschr(dosch_t const *str, dosch_t needle);
INTDEF dosch_t *LIBCCALL libc_dos_wcsrchr(dosch_t const *str, dosch_t needle);
INTDEF dosch_t *LIBCCALL libc_dos_wcslwr(dosch_t *str);
INTDEF dosch_t *LIBCCALL libc_dos_wcspbrk(dosch_t const *str, dosch_t const *reject);
INTDEF dosch_t *LIBCCALL libc_dos_wcsrev(dosch_t *str);
INTDEF dosch_t *LIBCCALL libc_dos_wcsset(dosch_t *str, dosch_t chr);
INTDEF dosch_t *LIBCCALL libc_dos_wcsstr(dosch_t const *haystack, dosch_t const *needle);
INTDEF dosch_t *LIBCCALL libc_dos_wcstok(dosch_t *str, dosch_t const *delim);
INTDEF dosch_t *LIBCCALL libc_dos_wcsupr(dosch_t *str);
INTDEF int LIBCCALL libc_dos_wcscasecmp_l(dosch_t const *str1, dosch_t const *str2, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcscasecoll_l(dosch_t const *str1, dosch_t const *str2, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcscoll_l(dosch_t const *str1, dosch_t const *str2, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcsncasecmp_l(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcsncasecoll_l(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcsncoll_l(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc);
INTDEF size_t LIBCCALL libc_dos_wcsxfrm_l(dosch_t *dst, dosch_t const *src, size_t max_chars, locale_t lc);
INTDEF dosch_t *LIBCCALL libc_dos_wcslwr_l(dosch_t *str, locale_t lc);
INTDEF dosch_t *LIBCCALL libc_dos_wcsupr_l(dosch_t *str, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcscasecmp(dosch_t const *str1, dosch_t const *str2);
INTDEF int LIBCCALL libc_dos_wcscasecoll(dosch_t const *str1, dosch_t const *str2);
INTDEF int LIBCCALL libc_dos_wcscoll(dosch_t const *str1, dosch_t const *str2);
INTDEF int LIBCCALL libc_dos_wcsncasecmp(dosch_t const *str1, dosch_t const *str2, size_t max_chars);
INTDEF int LIBCCALL libc_dos_wcsncasecoll(dosch_t const *str1, dosch_t const *str2, size_t max_chars);
INTDEF int LIBCCALL libc_dos_wcscmp(dosch_t const *str1, dosch_t const *str2);
INTDEF int LIBCCALL libc_dos_wcsncmp(dosch_t const *str1, dosch_t const *str2, size_t max_chars);
INTDEF int LIBCCALL libc_dos_wcsncoll(dosch_t const *str1, dosch_t const *str2, size_t max_chars);
INTDEF size_t LIBCCALL libc_dos_wcscspn(dosch_t const *str, dosch_t const *reject);
INTDEF size_t LIBCCALL libc_dos_wcslen(dosch_t const *str);
INTDEF size_t LIBCCALL libc_dos_wcsnlen(dosch_t const *src, size_t max_chars);
INTDEF size_t LIBCCALL libc_dos_wcsspn(dosch_t const *str, dosch_t const *reject);
INTDEF size_t LIBCCALL libc_dos_wcsxfrm(dosch_t *dst, dosch_t const *src, size_t max_chars);
INTDEF dosch_t *LIBCCALL libc_dos_wcscpy(dosch_t *dst, dosch_t const *src);
INTDEF dosch_t *LIBCCALL libc_dos_wcscat(dosch_t *dst, dosch_t const *src);
INTDEF dosch_t *LIBCCALL libc_dos_wcsncat(dosch_t *dst, dosch_t const *src, size_t max_chars);
INTDEF dosch_t *LIBCCALL libc_dos_wcsncpy(dosch_t *dst, dosch_t const *src, size_t max_chars);
INTDEF dosch_t *LIBCCALL libc_dos_wcsdup(dosch_t const *str);
INTDEF dosch_t *LIBCCALL libc_dos_wcsnset(dosch_t *str, dosch_t chr, size_t max_chars);
INTDEF dosch_t *LIBCCALL libc_dos_wcschr(dosch_t const *str, dosch_t needle);
INTDEF dosch_t *LIBCCALL libc_dos_wcsrchr(dosch_t const *str, dosch_t needle);
INTDEF dosch_t *LIBCCALL libc_dos_wcslwr(dosch_t *str);
INTDEF dosch_t *LIBCCALL libc_dos_wcspbrk(dosch_t const *str, dosch_t const *reject);
INTDEF dosch_t *LIBCCALL libc_dos_wcsrev(dosch_t *str);
INTDEF dosch_t *LIBCCALL libc_dos_wcsset(dosch_t *str, dosch_t chr);
INTDEF dosch_t *LIBCCALL libc_dos_wcsstr(dosch_t const *haystack, dosch_t const *needle);
INTDEF dosch_t *LIBCCALL libc_dos_wcstok(dosch_t *str, dosch_t const *delim);
INTDEF dosch_t *LIBCCALL libc_dos_wcsupr(dosch_t *str);
INTDEF int LIBCCALL libc_dos_wcscasecmp_l(dosch_t const *str1, dosch_t const *str2, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcscasecoll_l(dosch_t const *str1, dosch_t const *str2, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcscoll_l(dosch_t const *str1, dosch_t const *str2, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcsncasecmp_l(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcsncasecoll_l(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc);
INTDEF int LIBCCALL libc_dos_wcsncoll_l(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc);
INTDEF size_t LIBCCALL libc_dos_wcsxfrm_l(dosch_t *dst, dosch_t const *src, size_t max_chars, locale_t lc);
INTDEF dosch_t *LIBCCALL libc_dos_wcslwr_l(dosch_t *str, locale_t lc);
INTDEF dosch_t *LIBCCALL libc_dos_wcsupr_l(dosch_t *str, locale_t lc);
#endif /* !CONFIG_LIBC_NO_WCHAR_STRING */
#endif /* !CONFIG_LIBC_NO_DOS_EXTENSIONS */
#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_STRING_H */
