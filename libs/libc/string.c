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
#ifndef GUARD_LIBS_LIBC_STRING_C
#define GUARD_LIBS_LIBC_STRING_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1 /* Get the compatible header version of strerror_r */

#include "libc.h"
#include "string.h"
#include "ctype.h"
#include "stdio.h"
#include "malloc.h"
#include "errno.h"

#include <alloca.h>
#include <assert.h>
#include <bits/signum.h>
#include <hybrid/section.h>
#include <errno.h>
#include <hybrid/asm.h>
#include <hybrid/atomic.h>
#include <hybrid/byteorder.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/debuginfo.h>
#include <hybrid/limits.h>
#include <hybrid/swap.h>
#include <hybrid/types.h>
#include <hybrid/minmax.h>
#include <hybrid/byteswap.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <errno.h>

#if defined(__i386__) || defined(__x86_64__)
#include <asm/cpu-flags.h>
#endif

#ifndef __KERNEL__
#include "system.h"
#include "unicode.h"
#include "format-printer.h"
#include <stdlib.h>
#include <wchar.h>
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include <bits/dos-errno.h>
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */

DECL_BEGIN

#define DECL INTERN
#ifndef CONFIG_NO_64BIT_STRING
#ifdef CONFIG_NATIVE_64BIT_STRING
#define BITS 64
#include "templates/memory.code"
#endif
#endif
#define BITS 32
#include "templates/memory.code"
#define BITS 16
#include "templates/memory.code"
#define BITS 8
#include "templates/memory.code"
#ifndef CONFIG_NO_64BIT_STRING
#ifndef CONFIG_NATIVE_64BIT_STRING
#define BITS 64
#include "templates/memory.code"
#endif
#endif
#undef DECL

#define T            char
#define Tneedle      int
#define Ts           signed char
#define Tu           unsigned char
#define Tn           int
#define X(x)         libc_##x
#define Xstr(x)      libc_str##x
#define Xstp(x)      libc_stp##x
#define Xa(x)        libc_a##x
#define Xxa(x)       libc_##x##a
#define Xxa_s(x)     libc_##x##a_s
#define TOLOWER(x)   libc_tolower(x)
#define TOUPPER(x)   libc_toupper(x)
#define S            __SIZEOF_CHAR__
#define DECL         INTERN
#if !defined(__KERNEL__) && !defined(CONFIG_LIBC_NO_DOS_LIBC)
#define DOS_DECL     INTERN ATTR_DOSTEXT
#endif
#define DEFINE_ALIAS DEFINE_INTERN_ALIAS

/* Select optional functions. */
#  define WANT_STREND         /* strend() */
#  define WANT_STRNEND        /* strnend() */
#  define WANT_STRLEN         /* strlen() */
#  define WANT_STRNLEN        /* strnlen() */
#  define WANT_STRCHRNUL      /* strchrnul() */
#  define WANT_STRCHR         /* strchr() */
#  define WANT_STRRCHR        /* strrchr() */
#  define WANT_STRRCHRNUL     /* strrchrnul() */
#  define WANT_STRNCHR        /* strnchr() */
#  define WANT_STRNRCHR       /* strnrchr() */
#  define WANT_STRNCHRNUL     /* strnchrnul() */
#  define WANT_STRNRCHRNUL    /* strnrchrnul() */
#  define WANT_STROFF         /* stroff() */
#  define WANT_STRROFF        /* strroff() */
#  define WANT_STRNOFF        /* strnoff() */
#  define WANT_STRNROFF       /* strnroff() */
#  define WANT_STPCPY         /* stpcpy() */
#  define WANT_STPNCPY        /* stpncpy() */
#  define WANT_STRCMP         /* strcmp() */
#  define WANT_STRNCMP        /* strncmp() */
#  define WANT_STRCASECMP     /* strcasecmp() */
#  define WANT_STRNCASECMP    /* strncasecmp() */
#  define WANT_STRSTR         /* strstr() */
#  define WANT_STRCASESTR     /* strcasestr() */
#  define WANT_STRCPY         /* strcpy() */
#  define WANT_STRNCPY        /* strncpy() */
#  define WANT_STRCAT         /* strcat() */
#  define WANT_STRNCAT        /* strncat() */
#  define WANT_STRCSPN        /* strcspn() */
#  define WANT_STRSPN         /* strspn() */
#  define WANT_STRPBRK        /* strpbrk() */
#ifndef __KERNEL__
#  define WANT_STRTOK_R       /* strtok_r() */
#  define WANT_STRTOK         /* strtok() */
#  define WANT_STRFRY         /* strfry() */
#  define WANT_STRLWR         /* strlwr() */
#  define WANT_STRLWR_L       /* strlwr_l() */
#  define WANT_STRUPR         /* strupr() */
#  define WANT_STRUPR_L       /* strupr_l() */
//#define WANT_STRNLWR        /* strnlwr() */
//#define WANT_STRNLWR_L      /* strnlwr_l() */
//#define WANT_STRNUPR        /* strnupr() */
//#define WANT_STRNUPR_L      /* strnupr_l() */
#  define WANT_STRSET         /* strset() */
#  define WANT_STRNSET        /* strnset() */
#  define WANT_STRREV         /* strrev() */
//#define WANT_STRNREV        /* strnrev() */
#  define WANT_STRCOLL        /* strcoll() */
#  define WANT_STRCOLL_L      /* strcoll_l() */
#  define WANT_STRNCOLL       /* strncoll() */
#  define WANT_STRNCOLL_L     /* strncoll_l() */
#  define WANT_STRCASECOLL    /* strcasecoll() */
#  define WANT_STRCASECOLL_L  /* strcasecoll_l() */
#  define WANT_STRNCASECOLL   /* strncasecoll() */
#  define WANT_STRNCASECOLL_L /* strncasecoll_l() */
#  define WANT_STRXFRM        /* strxfrm       () */
#  define WANT_STRXFRM_L      /* strxfrm_l() */
#  define WANT_STRCASECMP     /* strcasecmp() */
#  define WANT_STRCASECMP_L   /* strcasecmp_l() */
#  define WANT_STRNCASECMP    /* strncasecmp() */
#  define WANT_STRNCASECMP_L  /* strncasecmp_l() */
//#define WANT_STRDUP         /* strdup() */
//#define WANT_STRNDUP        /* strndup() */
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#  define WANT_STRCAT_S       /* strcat_s() */
#  define WANT_STRCPY_S       /* strcpy_s() */
#  define WANT_STRLWR_S       /* strlwr_s() */
#  define WANT_STRLWR_S_L     /* strlwr_s_l() */
#  define WANT_STRUPR_S       /* strupr_s() */
#  define WANT_STRUPR_S_L     /* strupr_s_l() */
#  define WANT_STRNCAT_S      /* strncat_s() */
#  define WANT_STRNCPY_S      /* strncpy_s() */
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#  define WANT_STRSET_S       /* strset_s() */
#  define WANT_STRNSET_S      /* strnset_s() */
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
//#define WANT_MBLEN          /* ... */
//#define WANT_MBTOWC         /* ... */
//#define WANT_WCTOMB         /* ... */
//#define WANT_WCWIDTH        /* ... */
//#define WANT_MBSRTOWCS_S    /* ... */
//#define WANT_WCSRTOMBS_S    /* ... */
//#define WANT_WCRTOMB_S      /* ... */
#endif /* !__KERNEL__ */
#  define WANT_STRTOU32       /* strtou32() */
#  define WANT_STRTOU64       /* strtou64() */
#  define WANT_STRTO32        /* strto32() */
#  define WANT_STRTO64        /* strto64() */
#ifndef __KERNEL__
#  define WANT_STRTOU32_L     /* strtou32_l() */
#  define WANT_STRTOU64_L     /* strtou64_l() */
#  define WANT_STRTO32_L      /* strto32_l() */
#  define WANT_STRTO64_L      /* strto64_l() */
//#define WANT_ATOF           /* atof() */
#endif /* !__KERNEL__ */
#  define WANT_ATO32          /* ato32() */
#  define WANT_ATO64          /* ato64() */
#if !defined(__KERNEL__) && !defined(CONFIG_LIBC_NO_DOS_LIBC)
#  define WANT_ATOF_L         /* atof_l() */
#  define WANT_ATO32_L        /* ato32_l() */
#  define WANT_ATO64_L        /* ato64_l() */
#endif
#ifndef __KERNEL__
#  define WANT_STRTOLD        /* strtold() */
#  define WANT_STRTOF         /* strtof() */
#  define WANT_STRTOD         /* strtod() */
#  define WANT_STRTOLD_L      /* strtold_l() */
#  define WANT_STRTOF_L       /* strtof_l() */
#  define WANT_STRTOD_L       /* strtod_l() */
#endif /* !__KERNEL__ */
#if !defined(__KERNEL__) && !defined(CONFIG_LIBC_NO_DOS_LIBC)
#  define WANT_U32TOA_S       /* u32toa_s() */
#  define WANT_U64TOA_S       /* u64toa_s() */
#  define WANT_S32TOA_S       /* s32toa_s() */
#  define WANT_S64TOA_S       /* s64toa_s() */
#  define WANT_U32TOA         /* u32toa() */
#  define WANT_U64TOA         /* u64toa() */
#  define WANT_S32TOA         /* s32toa() */
#  define WANT_S64TOA         /* s64toa() */
#  define WANT_SPLITPATH_S    /* splitpath_s() */
#  define WANT_SPLITPATH      /* splitpath() */
#endif

#include "templates/string.code"

INTERN void *LIBCCALL libc_memmem(void const *haystack, size_t haystacklen,
                                  void const *needle, size_t needlelen) {
 byte_t *iter,*end;
 if unlikely(needlelen > haystacklen) return NULL;
 end = (iter = (byte_t *)haystack)+(haystacklen-needlelen);
 for (;;) {
  if (libc_memcmp(iter,needle,needlelen) == 0)
      return iter;
  if (iter == end) break;
  ++iter;
 }
 return NULL;
}


#ifndef __KERNEL__
#define NOCASE
#include "templates/fuzzy_memcmp.code"
#include "templates/fuzzy_memcmp.code"

INTERN size_t LIBCCALL libc_fuzzy_strcmp(char const *a, char const *b) {
 return libc_fuzzy_memcmp(a,libc_strlen(a)*sizeof(char),
                          b,libc_strlen(b)*sizeof(char));
}
INTERN size_t LIBCCALL libc_fuzzy_strncmp(char const *a, size_t max_a_chars,
                                          char const *b, size_t max_b_chars) {
 return libc_fuzzy_memcmp(a,libc_strnlen(a,max_a_chars)*sizeof(char),
                          b,libc_strnlen(b,max_b_chars)*sizeof(char));
}
INTERN size_t LIBCCALL libc_fuzzy_strcasecmp(char const *a, char const *b) {
 return libc_fuzzy_memcasecmp(a,libc_strlen(a)*sizeof(char),
                              b,libc_strlen(b)*sizeof(char));
}
INTERN size_t LIBCCALL libc_fuzzy_strncasecmp(char const *a, size_t max_a_chars,
                                              char const *b, size_t max_b_chars) {
 return libc_fuzzy_memcasecmp(a,libc_strnlen(a,max_a_chars)*sizeof(char),
                              b,libc_strnlen(b,max_b_chars)*sizeof(char));
}

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_fuzzy_memcasecmp_l,libc_fuzzy_memcasecmp);
DEFINE_INTERN_ALIAS(libc_fuzzy_strcasecmp_l,libc_fuzzy_strcasecmp);
DEFINE_INTERN_ALIAS(libc_fuzzy_strncasecmp_l,libc_fuzzy_strncasecmp);
#else /* CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP */
INTERN size_t LIBCCALL libc_fuzzy_memcasecmp_l(void const *a, size_t a_bytes, void const *b, size_t b_bytes, locale_t UNUSED(locale)) { return libc_fuzzy_memcasecmp(a,a_bytes,b,b_bytes); }
INTERN size_t LIBCCALL libc_fuzzy_strcasecmp_l(char const *a, char const *b, locale_t UNUSED(locale)) { return libc_fuzzy_strcasecmp(a,b); }
INTERN size_t LIBCCALL libc_fuzzy_strncasecmp_l(char const *a, size_t max_a_chars, char const *b, size_t max_b_chars, locale_t UNUSED(locale)) { return libc_fuzzy_strncasecmp(a,max_a_chars,b,max_b_chars); }
#endif /* !CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP */


INTERN int LIBCCALL
libc_wildstrcmp(char const *pattern, char const *string) {
 char card_post;
 for (;;) {
  if (!*string) {
   /* End of string (if the patter is empty, or only contains '*', we have a match) */
   while (*pattern == '*') ++pattern;
   return -(int)*pattern;
  }
  if (!*pattern) return (int)*string; /* Pattern end doesn't match */
  if (*pattern == '*') {
   /* Skip starts */
   do ++pattern; while (*pattern == '*');
   if ((card_post = *pattern++) == '\0')
        return 0; /* Pattern ends with '*' (matches everything) */
   if (card_post == '?') goto next; /* Match any --> already found */
   for (;;) {
    char ch = *string++;
    if (ch == card_post) {
     /* Recursively check if the rest of the string and pattern match */
     if (!libc_wildstrcmp(string,pattern)) return 0;
    } else if (!ch) {
     return -(int)card_post; /* Wildcard suffix not found */
    }
   }
  }
  if (*pattern == *string || *pattern == '?') {
next: ++string,++pattern;
   continue; /* single character match */
  }
  break; /* mismatch */
 }
 return *string-*pattern;
}

INTERN int LIBCCALL
libc_wildstrcasecmp(char const *pattern, char const *string) {
#define UNIFORM(x) libc_tolower(x)
 char card_post;
 for (;;) {
  if (!*string) {
   /* End of string (if the patter is empty, or only contains '*', we have a match) */
   while (*pattern == '*') ++pattern;
   return -(int)UNIFORM(*pattern);
  }
  if (!*pattern) return (int)*string; /* Pattern end doesn't match */
  if (*pattern == '*') {
   /* Skip starts */
   do ++pattern; while (*pattern == '*');
   if ((card_post = *pattern++) == '\0')
        return 0; /* Pattern ends with '*' (matches everything) */
   if (card_post == '?') goto next; /* Match any --> already found */
   card_post = UNIFORM(card_post);
   for (;;) {
    char ch = *string++;
    if (UNIFORM(ch) == card_post) {
     /* Recursively check if the rest of the string and pattern match */
     if (!libc_wildstrcasecmp(string,pattern)) return 0;
    } else if (!ch) {
     return -(int)card_post; /* Wildcard suffix not found */
    }
   }
  }
  if (UNIFORM(*pattern) == UNIFORM(*string) || *pattern == '?') {
next: ++string,++pattern;
   continue; /* single character match */
  }
  break; /* mismatch */
 }
 return UNIFORM(*string)-UNIFORM(*pattern);
#undef UNIFORM
}
#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_wildstrcasecmp_l,libc_wildstrcasecmp);
#else /* CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP */
INTERN int LIBCCALL libc_wildstrcasecmp_l(char const *pattern, char const *string, locale_t UNUSED(locale)) { return libc_wildstrcasecmp(pattern,string); }
#endif /* !CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP */
#endif /* !__KERNEL__ */


#define DO_FFS(i) \
{ int result; \
  if (!i) return 0; \
  for (result = 1; !(i&1); ++result) i >>= 1; \
  return result; \
}
INTERN int LIBCCALL libc___ffs8(s8 i) { DO_FFS(i) }
INTERN int LIBCCALL libc___ffs16(s16 i) { DO_FFS(i) }
INTERN int LIBCCALL libc___ffs32(s32 i) { DO_FFS(i) }
INTERN int LIBCCALL libc___ffs64(s64 i) { DO_FFS(i) }
#undef DO_FFS

/* TODO */INTERN int LIBCCALL libc_strverscmp(char const *s1, char const *s2) { NOT_IMPLEMENTED(); return 0; }
/* TODO */INTERN char *LIBCCALL libc_strsep(char **__restrict stringp, char const *__restrict delim) { NOT_IMPLEMENTED(); return NULL; }

INTERN int LIBCCALL libc_memcasecmp(void const *s1, void const *s2, size_t n_bytes) {
 char a,b; int result;
 char *p1 = (char *)s1,*p2 = (char *)s2;
 char *end = p1+n_bytes;
 if (!n_bytes) return 0;
 do a = libc_tolower(*p1++),
    b = libc_tolower(*p2++),
    result = (int)a-(int)b;
 while (result == 0 && p1 != end);
 return result;
}


/* String functions deemed to unimportant to include in the kernel core. (libk) */
#ifndef __KERNEL__
INTERN void LIBCCALL libc_bcopy(void const *src, void *dst, size_t n) { libc_memmove(dst,src,n); }
INTERN void LIBCCALL libc_bzero(void *s, size_t n) { libc_memset(s,0,n); }
INTERN void LIBCCALL
libc_swab(void const *__restrict from, void *__restrict to, size_t n_bytes) {
 NOT_IMPLEMENTED();
}
DEFINE_INTERN_ALIAS(libc_index,libc_strchr);
DEFINE_INTERN_ALIAS(libc_rindex,libc_strrchr);
INTERN char *LIBCCALL libc_dirname(char *path) {
 char *iter;
 if (!path || !*path) ret_cwd: return ".";
 iter = libc_strend(path)-1;
 while (*iter == '/') {
  if (iter == path) { iter[1] = '\0'; return path; }
  --iter;
 }
 while (iter >= path && *iter != '/') --iter;
 if (iter < path) goto ret_cwd;
 if (iter == path) ++iter;
 *iter = '\0';
 return path;
}
/* TODO */INTERN char *LIBCCALL libc___xpg_basename(char *path) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_basename(char const *__restrict path) {
 char ch,*iter = (char *)path,*result = NULL;
 if (!path || !*path) return ".";
 do if ((ch = *iter++) == '/') result = iter;
 while (ch);
 if unlikely(!result) return (char *)path; /* Path doesn't contain '/'. */
 if (*result) return result; /* Last character isn't a '/'. */
 iter = result;
 while (iter != path && iter[-1] == '/') --iter;
 if (iter == path) return result-1; /* Only `'/'"-characters. */
 *iter = '\0'; /* Trim all ending `'/'"-characters. */
 while (iter != path && iter[-1] != '/') --iter; /* Scan until the previous '/'. */
 return iter; /* Returns string after previous '/'. */
}
INTERN void *LIBCCALL libc_memccpy(void *__restrict dst,
                                   void const *__restrict src,
                                   int c, size_t n) {
 byte_t *dst_iter,*end;
 byte_t const *src_iter;
 end = (dst_iter = (byte_t *)dst)+n;
 src_iter = (byte_t const *)src;
 while (dst_iter != end) {
  if ((*dst_iter++ = *src_iter++) == c)
        return dst_iter;
 }
 return NULL;
}

#ifndef __KERNEL__
INTERN char *LIBCCALL libc_gcvt(double value, int ndigit, char *buf) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_qgcvt(long double value, int ndigit, char *buf) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_ecvt_r(double value, int ndigit, int *__restrict decptr, int *__restrict sign, char *__restrict buf, size_t len) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_fcvt_r(double value, int ndigit, int *__restrict decptr, int *__restrict sign, char *__restrict buf, size_t len) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_qecvt_r(long double value, int ndigit, int *__restrict decptr, int *__restrict sign, char *__restrict buf, size_t len) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_qfcvt_r(long double value, int ndigit, int *__restrict decptr, int *__restrict sign, char *__restrict buf, size_t len) { NOT_IMPLEMENTED(); return 0; }
#define FLOAT_BUFFER_SIZE 64
INTERN char *LIBCCALL libc_qecvt(long double value, int ndigit, int *__restrict decptr, int *__restrict sign) { PRIVATE char buffer[FLOAT_BUFFER_SIZE]; return libc_qecvt_r(value,ndigit,decptr,sign,buffer,sizeof(buffer)) ? NULL : buffer; }
INTERN char *LIBCCALL libc_qfcvt(long double value, int ndigit, int *__restrict decptr, int *__restrict sign) { PRIVATE char buffer[FLOAT_BUFFER_SIZE]; return libc_qfcvt_r(value,ndigit,decptr,sign,buffer,sizeof(buffer)) ? NULL : buffer; }
INTERN char *LIBCCALL libc_ecvt(double value, int ndigit, int *__restrict decptr, int *__restrict sign) { PRIVATE char buffer[FLOAT_BUFFER_SIZE]; return libc_ecvt_r(value,ndigit,decptr,sign,buffer,sizeof(buffer)) ? NULL : buffer; }
INTERN char *LIBCCALL libc_fcvt(double value, int ndigit, int *__restrict decptr, int *__restrict sign) { PRIVATE char buffer[FLOAT_BUFFER_SIZE]; return libc_fcvt_r(value,ndigit,decptr,sign,buffer,sizeof(buffer)) ? NULL : buffer; }
#undef FLOAT_BUFFER_SIZE
INTERN double LIBCCALL libc_atof(char const *__restrict nptr) { return libc_strtod(nptr,NULL); }
INTERN float LIBCCALL libc_strtof_l(char const *__restrict nptr, char **__restrict endptr, locale_t UNUSED(loc)) { return libc_strtof(nptr,endptr); }
INTERN double LIBCCALL libc_strtod_l(char const *__restrict nptr, char **__restrict endptr, locale_t UNUSED(loc)) { return libc_strtod(nptr,endptr); }
INTERN long double LIBCCALL libc_strtold_l(char const *__restrict nptr, char **__restrict endptr, locale_t UNUSED(loc)) { return libc_strtold(nptr,endptr); }
#endif /* !__KERNEL__ */

/* TODO: Kernel-share data? */
INTERN ATTR_RARERODATA char const signal_names[][10] = {
#define ENTRY(x) [x-1] = #x
    ENTRY(SIGHUP),
    ENTRY(SIGINT),
    ENTRY(SIGQUIT),
    ENTRY(SIGILL),
    ENTRY(SIGTRAP),
    ENTRY(SIGABRT),
#if defined(SIGIOT) && SIGIOT != SIGABRT
    ENTRY(SIGIOT),
#endif
    ENTRY(SIGBUS),
    ENTRY(SIGFPE),
    ENTRY(SIGKILL),
    ENTRY(SIGUSR1),
    ENTRY(SIGSEGV),
    ENTRY(SIGUSR2),
    ENTRY(SIGPIPE),
    ENTRY(SIGALRM),
    ENTRY(SIGTERM),
    ENTRY(SIGSTKFLT),
#if defined(SIGCLD) && SIGCLD != SIGCHLD
    ENTRY(SIGCLD),
#endif
    ENTRY(SIGCHLD),
    ENTRY(SIGCONT),
    ENTRY(SIGSTOP),
    ENTRY(SIGTSTP),
    ENTRY(SIGTTIN),
    ENTRY(SIGTTOU),
    ENTRY(SIGURG),
    ENTRY(SIGXCPU),
    ENTRY(SIGXFSZ),
    ENTRY(SIGVTALRM),
    ENTRY(SIGPROF),
    ENTRY(SIGWINCH),
#if defined(SIGPOLL) && SIGPOLL != SIGIO
    ENTRY(SIGPOLL),
#endif
    ENTRY(SIGIO),
    ENTRY(SIGPWR),
    ENTRY(SIGSYS),
#if defined(SIGUNUSED) && SIGUNUSED != SIGSYS
    ENTRY(SIGUNUSED),
#endif
#undef ENTRY
};
INTERN char *LIBCCALL libc_strsignal(int sig) {
 PRIVATE char buffer[20];
 if (sig < 1 || (unsigned)sig >= COMPILER_LENOF(signal_names))
  libc_sprintf(buffer,"unknown(%d)",sig);
 else {
  libc_strcpy(buffer,signal_names[sig-1]);
 }
 return buffer;
}


/* TODO */INTERN void *LIBCCALL libc_memfrob(void *s, size_t n) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_strerror_l(int errnum, locale_t l) { NOT_IMPLEMENTED(); return libc_strerror(errnum); }
/* TODO */INTERN char *ATTR_CDECL libc_strdupaf(char const *__restrict format, ...) { NOT_IMPLEMENTED(); return NULL; }
/* TODO */INTERN char *LIBCCALL libc_vstrdupaf(char const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return NULL; }
#endif /* !__KERNEL__ */

#ifndef __KERNEL__
INTERN struct errnotext_data const *p_errnotext = NULL;
#else
DATDEF struct errnotext_data const errnotext;
#endif

INTERN char const *LIBCCALL strerror_get(uintptr_t kind, int no) {
 struct errnotext_data const *data;
 struct errnotext_entry const *entry; char const *string;
#ifndef __KERNEL__
 if ((data = p_errnotext) == NULL) {
  data = (struct errnotext_data *)sys_xsharesym("errnotext");
  if (data) ATOMIC_WRITE(p_errnotext,data);
 }
 if unlikely(!data || E_ISERR(data)) goto unknown; /* The kernel doesn't define this information. */
#else
 data = &errnotext;
#endif
 if unlikely(no < 0 || (size_t)no >= data->etd_enocnt) goto unknown;
 entry   = (struct errnotext_entry const *)((uintptr_t)data+(intptr_t)data->etd_enotab+
                                            (uintptr_t)((size_t)no*data->etd_enoent));
 string  = (char const *)((uintptr_t)data+(intptr_t)data->etd_strtab);
 string += *(u16 *)((uintptr_t)entry+kind);
 return string;
unknown:
#ifdef __KERNEL__
 switch (no) {
#define CASE(id,text) case id: return kind == ERRNOSTR_NAME ? #id : text
 CASE(ERELOAD,"Resource must be reloaded");
 CASE(ELOST,  "Resource was lost");
 CASE(ENOREL, "Invalid Relocation");
#undef CASE
 default: break;
 }
#endif
 return NULL;
}

INTERN char const *LIBCCALL libc_strerror_s(int errnum) { return strerror_get(ERRNOSTR_TEXT,errnum); }
INTERN char const *LIBCCALL libc_strerrorname_s(int errnum) { return strerror_get(ERRNOSTR_NAME,errnum); }

#ifdef __KERNEL__
GLOBAL_ASM(
#include "strerror.inl"
);
#else /* __KERNEL__ */
PRIVATE char strerror_buf[64];
INTERN char *LIBCCALL libc_strerror(int errnum) {
 char const *string = strerror_get(ERRNOSTR_TEXT,errnum);
 if (string) {
  /* Copy the descriptor text. */
  strerror_buf[COMPILER_LENOF(strerror_buf)-1] = '\0';
  libc_strncpy(strerror_buf,string,COMPILER_LENOF(strerror_buf)-1);
 } else {
  libc_sprintf(strerror_buf,"Unknown error %d",errnum);
 }
 return strerror_buf;
}
INTERN int LIBCCALL libc___xpg_strerror_r(int errnum, char *buf,
                                          size_t buflen) {
 size_t msg_len; char const *string;
 string = strerror_get(ERRNOSTR_TEXT,errnum);
 if (!buf) buflen = 0;
 if (!string) return EINVAL;
 /* Copy the descriptor text. */
 msg_len = (libc_strlen(string)+1)*sizeof(char);
 if (msg_len > buflen) return ERANGE;
 libc_memcpy(buf,string,msg_len);
 return 0;
}
INTERN char *LIBCCALL libc_strerror_r(int errnum, char *buf, size_t buflen) {
 char const *string = strerror_get(ERRNOSTR_TEXT,errnum);
 if (!buf || !buflen) {
  buf    = strerror_buf;
  buflen = sizeof(strerror_buf);
 }
 if (string) {
  /* Copy the descriptor text. */
  size_t msg_len = (libc_strlen(string)+1)*sizeof(char);
  if (msg_len > buflen) {
   buf    = strerror_buf;
   buflen = sizeof(strerror_buf);
   if unlikely(msg_len > buflen) {
    msg_len = buflen-1;
    buf[msg_len] = '\0';
   }
  }
  libc_memcpy(buf,string,msg_len);
 } else {
again_unknown:
  if (libc_snprintf(buf,buflen,"Unknown error %d",errnum) >= buflen) {
   assert(buf != strerror_buf);
   buf    = strerror_buf;
   buflen = sizeof(strerror_buf);
   goto again_unknown;
  }
 }
 return buf;
}
INTERN int LIBCCALL libc_memcasecmp_l(void const *a, void const *b,
                                      size_t n_bytes, locale_t lc) {
 NOT_IMPLEMENTED();
 return libc_memcasecmp(a,b,n_bytes);
}
#endif /* !__KERNEL__ */


/* Define public string functions */
#undef memcpy
#undef mempcpy
DEFINE_PUBLIC_ALIAS(memcpy,libc_memcpy);
DEFINE_PUBLIC_ALIAS(_memcpy_d,libc__memcpy_d);
DEFINE_PUBLIC_ALIAS(mempcpy,libc_mempcpy);
DEFINE_PUBLIC_ALIAS(_mempcpy_d,libc__mempcpy_d);
DEFINE_PUBLIC_ALIAS(memmove,libc_memmove);
DEFINE_PUBLIC_ALIAS(memset,libc_memset);
DEFINE_PUBLIC_ALIAS(memcmp,libc_memcmp);
DEFINE_PUBLIC_ALIAS(memchr,libc_memchr);
DEFINE_PUBLIC_ALIAS(memrchr,libc_memrchr);
DEFINE_PUBLIC_ALIAS(memend,libc_memend);
DEFINE_PUBLIC_ALIAS(memrend,libc_memrend);
DEFINE_PUBLIC_ALIAS(memlen,libc_memlen);
DEFINE_PUBLIC_ALIAS(memrlen,libc_memrlen);
DEFINE_PUBLIC_ALIAS(rawmemchr,libc_rawmemchr);
DEFINE_PUBLIC_ALIAS(rawmemrchr,libc_rawmemrchr);
DEFINE_PUBLIC_ALIAS(rawmemlen,libc_rawmemlen);
DEFINE_PUBLIC_ALIAS(rawmemrlen,libc_rawmemrlen);
#undef memcpyw
#undef mempcpyw
DEFINE_PUBLIC_ALIAS(memcpyw,libc_memcpyw);
DEFINE_PUBLIC_ALIAS(_memcpyw_d,libc__memcpyw_d);
DEFINE_PUBLIC_ALIAS(mempcpyw,libc_mempcpyw);
DEFINE_PUBLIC_ALIAS(_mempcpyw_d,libc__mempcpyw_d);
DEFINE_PUBLIC_ALIAS(memmovew,libc_memmovew);
DEFINE_PUBLIC_ALIAS(memsetw,libc_memsetw);
DEFINE_PUBLIC_ALIAS(memcmpw,libc_memcmpw);
DEFINE_PUBLIC_ALIAS(memchrw,libc_memchrw);
DEFINE_PUBLIC_ALIAS(memrchrw,libc_memrchrw);
DEFINE_PUBLIC_ALIAS(memendw,libc_memendw);
DEFINE_PUBLIC_ALIAS(memrendw,libc_memrendw);
DEFINE_PUBLIC_ALIAS(memlenw,libc_memlenw);
DEFINE_PUBLIC_ALIAS(memrlenw,libc_memrlenw);
DEFINE_PUBLIC_ALIAS(rawmemchrw,libc_rawmemchrw);
DEFINE_PUBLIC_ALIAS(rawmemrchrw,libc_rawmemrchrw);
DEFINE_PUBLIC_ALIAS(rawmemlenw,libc_rawmemlenw);
DEFINE_PUBLIC_ALIAS(rawmemrlenw,libc_rawmemrlenw);
#undef memcpyl
#undef mempcpyl
DEFINE_PUBLIC_ALIAS(memcpyl,libc_memcpyl);
DEFINE_PUBLIC_ALIAS(_memcpyl_d,libc__memcpyl_d);
DEFINE_PUBLIC_ALIAS(mempcpyl,libc_mempcpyl);
DEFINE_PUBLIC_ALIAS(_mempcpyl_d,libc__mempcpyl_d);
DEFINE_PUBLIC_ALIAS(memmovel,libc_memmovel);
DEFINE_PUBLIC_ALIAS(memsetl,libc_memsetl);
DEFINE_PUBLIC_ALIAS(memcmpl,libc_memcmpl);
DEFINE_PUBLIC_ALIAS(memchrl,libc_memchrl);
DEFINE_PUBLIC_ALIAS(memrchrl,libc_memrchrl);
DEFINE_PUBLIC_ALIAS(memendl,libc_memendl);
DEFINE_PUBLIC_ALIAS(memrendl,libc_memrendl);
DEFINE_PUBLIC_ALIAS(memlenl,libc_memlenl);
DEFINE_PUBLIC_ALIAS(memrlenl,libc_memrlenl);
DEFINE_PUBLIC_ALIAS(rawmemchrl,libc_rawmemchrl);
DEFINE_PUBLIC_ALIAS(rawmemrchrl,libc_rawmemrchrl);
DEFINE_PUBLIC_ALIAS(rawmemlenl,libc_rawmemlenl);
DEFINE_PUBLIC_ALIAS(rawmemrlenl,libc_rawmemrlenl);
#ifndef CONFIG_NO_64BIT_STRING
#undef memcpyq
#undef mempcpyq
DEFINE_PUBLIC_ALIAS(memcpyq,libc_memcpyq);
DEFINE_PUBLIC_ALIAS(_memcpyq_d,libc__memcpyq_d);
DEFINE_PUBLIC_ALIAS(mempcpyq,libc_mempcpyq);
DEFINE_PUBLIC_ALIAS(_mempcpyq_d,libc__mempcpyq_d);
DEFINE_PUBLIC_ALIAS(memmoveq,libc_memmoveq);
DEFINE_PUBLIC_ALIAS(memsetq,libc_memsetq);
DEFINE_PUBLIC_ALIAS(memcmpq,libc_memcmpq);
DEFINE_PUBLIC_ALIAS(memchrq,libc_memchrq);
DEFINE_PUBLIC_ALIAS(memrchrq,libc_memrchrq);
DEFINE_PUBLIC_ALIAS(memendq,libc_memendq);
DEFINE_PUBLIC_ALIAS(memrendq,libc_memrendq);
DEFINE_PUBLIC_ALIAS(memlenq,libc_memlenq);
DEFINE_PUBLIC_ALIAS(memrlenq,libc_memrlenq);
DEFINE_PUBLIC_ALIAS(rawmemchrq,libc_rawmemchrq);
DEFINE_PUBLIC_ALIAS(rawmemrchrq,libc_rawmemrchrq);
DEFINE_PUBLIC_ALIAS(rawmemlenq,libc_rawmemlenq);
DEFINE_PUBLIC_ALIAS(rawmemrlenq,libc_rawmemrlenq);
DEFINE_PUBLIC_ALIAS(mempatq,libc_mempatq);
#endif /* !CONFIG_NO_64BIT_STRING */
DEFINE_PUBLIC_ALIAS(mempatw,libc_mempatw);
DEFINE_PUBLIC_ALIAS(mempatl,libc_mempatl);
DEFINE_PUBLIC_ALIAS(strend,libc_strend);
DEFINE_PUBLIC_ALIAS(strnend,libc_strnend);
DEFINE_PUBLIC_ALIAS(strlen,libc_strlen);
DEFINE_PUBLIC_ALIAS(strnlen,libc_strnlen);
DEFINE_PUBLIC_ALIAS(strchrnul,libc_strchrnul);
DEFINE_PUBLIC_ALIAS(strchr,libc_strchr);
DEFINE_PUBLIC_ALIAS(strrchr,libc_strrchr);
DEFINE_PUBLIC_ALIAS(strrchrnul,libc_strrchrnul);
DEFINE_PUBLIC_ALIAS(strnchr,libc_strnchr);
DEFINE_PUBLIC_ALIAS(strnrchr,libc_strnrchr);
DEFINE_PUBLIC_ALIAS(strnchrnul,libc_strnchrnul);
DEFINE_PUBLIC_ALIAS(strnrchrnul,libc_strnrchrnul);
DEFINE_PUBLIC_ALIAS(stroff,libc_stroff);
DEFINE_PUBLIC_ALIAS(strroff,libc_strroff);
DEFINE_PUBLIC_ALIAS(strnoff,libc_strnoff);
DEFINE_PUBLIC_ALIAS(strnroff,libc_strnroff);
DEFINE_PUBLIC_ALIAS(stpcpy,libc_stpcpy);
DEFINE_PUBLIC_ALIAS(stpncpy,libc_stpncpy);
DEFINE_PUBLIC_ALIAS(strcmp,libc_strcmp);
DEFINE_PUBLIC_ALIAS(strncmp,libc_strncmp);
DEFINE_PUBLIC_ALIAS(strcasecmp,libc_strcasecmp);
DEFINE_PUBLIC_ALIAS(strncasecmp,libc_strncasecmp);
DEFINE_PUBLIC_ALIAS(strstr,libc_strstr);
DEFINE_PUBLIC_ALIAS(strcasestr,libc_strcasestr);
DEFINE_PUBLIC_ALIAS(memmem,libc_memmem);
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(fuzzy_memcmp,libc_fuzzy_memcmp);
DEFINE_PUBLIC_ALIAS(fuzzy_strcmp,libc_fuzzy_strcmp);
DEFINE_PUBLIC_ALIAS(fuzzy_strncmp,libc_fuzzy_strncmp);
DEFINE_PUBLIC_ALIAS(fuzzy_memcasecmp,libc_fuzzy_memcasecmp);
DEFINE_PUBLIC_ALIAS(fuzzy_strcasecmp,libc_fuzzy_strcasecmp);
DEFINE_PUBLIC_ALIAS(fuzzy_strncasecmp,libc_fuzzy_strncasecmp);
DEFINE_PUBLIC_ALIAS(fuzzy_memcasecmp_l,libc_fuzzy_memcasecmp_l);
DEFINE_PUBLIC_ALIAS(fuzzy_strcasecmp_l,libc_fuzzy_strcasecmp_l);
DEFINE_PUBLIC_ALIAS(fuzzy_strncasecmp_l,libc_fuzzy_strncasecmp_l);
DEFINE_PUBLIC_ALIAS(wildstrcmp,libc_wildstrcmp);
DEFINE_PUBLIC_ALIAS(wildstrcasecmp,libc_wildstrcasecmp);
DEFINE_PUBLIC_ALIAS(wildstrcasecmp_l,libc_wildstrcasecmp_l);
#endif /* !__KERNEL__ */
DEFINE_PUBLIC_ALIAS(__ffs8,libc___ffs8);
DEFINE_PUBLIC_ALIAS(__ffs16,libc___ffs16);
DEFINE_PUBLIC_ALIAS(__ffs32,libc___ffs32);
DEFINE_PUBLIC_ALIAS(__ffs64,libc___ffs64);
DEFINE_PUBLIC_ALIAS(strverscmp,libc_strverscmp);
DEFINE_PUBLIC_ALIAS(strsep,libc_strsep);
DEFINE_PUBLIC_ALIAS(strerror_s,libc_strerror_s);
DEFINE_PUBLIC_ALIAS(strerrorname_s,libc_strerrorname_s);
DEFINE_PUBLIC_ALIAS(strerror,libc_strerror);
DEFINE_PUBLIC_ALIAS(__xpg_strerror_r,libc___xpg_strerror_r);
DEFINE_PUBLIC_ALIAS(strerror_r,libc_strerror_r);
DEFINE_PUBLIC_ALIAS(_memicmp,libc_memcasecmp);

#if __SIZEOF_INT__ == 4
#define __INTFUN(x)   x##32
#define __INTFUN_L(x) x##32##_l
#elif __SIZEOF_INT__ == 8
#define __INTFUN(x)   x##64
#define __INTFUN_L(x) x##64##_l
#else
#error FIXME
#endif

#define __DOS_LONGFUN(x)   x##32
#define __DOS_LONGFUN_L(x) x##32##_l

#if __SIZEOF_LONG__ == 4
#define __LONGFUN(x)   x##32
#define __LONGFUN_L(x) x##32##_l
#elif __SIZEOF_LONG__ == 8
#define __LONGFUN(x)   x##64
#define __LONGFUN_L(x) x##64##_l
#else
#error FIXME
#endif

#if __SIZEOF_LONG_LONG__ == 4
#define __LONGLONGFUN(x)   x##32
#define __LONGLONGFUN_L(x) x##32##_l
#elif __SIZEOF_LONG_LONG__ == 8
#define __LONGLONGFUN(x)   x##64
#define __LONGLONGFUN_L(x) x##64##_l
#else
#error FIXME
#endif

/* Export string --> integer converters. */
DEFINE_PUBLIC_ALIAS(strtol,__LONGFUN(libc_strto));
DEFINE_PUBLIC_ALIAS(strtoul,__LONGFUN(libc_strtou));
DEFINE_PUBLIC_ALIAS(strtoll,__LONGLONGFUN(libc_strto));
DEFINE_PUBLIC_ALIAS(strtoull,__LONGLONGFUN(libc_strtou));
DEFINE_PUBLIC_ALIAS(atoi,__INTFUN(libc_ato));
DEFINE_PUBLIC_ALIAS(atol,__LONGFUN(libc_ato));
DEFINE_PUBLIC_ALIAS(atoll,__LONGLONGFUN(libc_ato));
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(gcvt,libc_gcvt);
DEFINE_PUBLIC_ALIAS(qgcvt,libc_qgcvt);
DEFINE_PUBLIC_ALIAS(ecvt_r,libc_ecvt_r);
DEFINE_PUBLIC_ALIAS(fcvt_r,libc_fcvt_r);
DEFINE_PUBLIC_ALIAS(qecvt_r,libc_qecvt_r);
DEFINE_PUBLIC_ALIAS(qfcvt_r,libc_qfcvt_r);
DEFINE_PUBLIC_ALIAS(atof,libc_atof);
DEFINE_PUBLIC_ALIAS(strtod,libc_strtod);
DEFINE_PUBLIC_ALIAS(strtof,libc_strtof);
DEFINE_PUBLIC_ALIAS(strtold,libc_strtold);
DEFINE_PUBLIC_ALIAS(qecvt,libc_qecvt);
DEFINE_PUBLIC_ALIAS(qfcvt,libc_qfcvt);
DEFINE_PUBLIC_ALIAS(ecvt,libc_ecvt);
DEFINE_PUBLIC_ALIAS(fcvt,libc_fcvt);
DEFINE_PUBLIC_ALIAS(strtof_l,libc_strtof_l);
DEFINE_PUBLIC_ALIAS(strtod_l,libc_strtod_l);
DEFINE_PUBLIC_ALIAS(strtold_l,libc_strtold_l);
DEFINE_PUBLIC_ALIAS(strtol_l,__LONGFUN_L(libc_strto));
DEFINE_PUBLIC_ALIAS(strtoul_l,__LONGFUN_L(libc_strtou));
DEFINE_PUBLIC_ALIAS(strtoll_l,__LONGLONGFUN_L(libc_strto));
DEFINE_PUBLIC_ALIAS(strtoull_l,__LONGLONGFUN_L(libc_strtou));
/* Don't export this alias within the kernel. */
DEFINE_PUBLIC_ALIAS(strtoq,__LONGLONGFUN(libc_strto));
DEFINE_PUBLIC_ALIAS(strtouq,__LONGLONGFUN(libc_strtou));
#endif /* !__KERNEL__ */

#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(bcopy,libc_bcopy);
DEFINE_PUBLIC_ALIAS(bzero,libc_bzero);
DEFINE_PUBLIC_ALIAS(swab,libc_swab);
DEFINE_PUBLIC_ALIAS(strcpy,libc_strcpy);
DEFINE_PUBLIC_ALIAS(strncpy,libc_strncpy);
DEFINE_PUBLIC_ALIAS(index,libc_index);
DEFINE_PUBLIC_ALIAS(rindex,libc_rindex);
DEFINE_PUBLIC_ALIAS(dirname,libc_dirname);
DEFINE_PUBLIC_ALIAS(__xpg_basename,libc___xpg_basename);
#undef basename
DEFINE_PUBLIC_ALIAS(basename,libc_basename);
DEFINE_PUBLIC_ALIAS(strcat,libc_strcat);
DEFINE_PUBLIC_ALIAS(strncat,libc_strncat);
DEFINE_PUBLIC_ALIAS(strcspn,libc_strcspn);
DEFINE_PUBLIC_ALIAS(strpbrk,libc_strpbrk);
DEFINE_PUBLIC_ALIAS(strtok_r,libc_strtok_r);
DEFINE_PUBLIC_ALIAS(strtok,libc_strtok);
DEFINE_PUBLIC_ALIAS(strspn,libc_strspn);
DEFINE_PUBLIC_ALIAS(memccpy,libc_memccpy);
DEFINE_PUBLIC_ALIAS(strsignal,libc_strsignal);
DEFINE_PUBLIC_ALIAS(strcoll,libc_strcoll);
DEFINE_PUBLIC_ALIAS(strxfrm,libc_strxfrm);
DEFINE_PUBLIC_ALIAS(strfry,libc_strfry);
DEFINE_PUBLIC_ALIAS(memfrob,libc_memfrob);
DEFINE_PUBLIC_ALIAS(strcoll_l,libc_strcoll_l);
DEFINE_PUBLIC_ALIAS(strxfrm_l,libc_strxfrm_l);
DEFINE_PUBLIC_ALIAS(strerror_l,libc_strerror_l);
DEFINE_PUBLIC_ALIAS(strcasecmp_l,libc_strcasecmp_l);
DEFINE_PUBLIC_ALIAS(strncasecmp_l,libc_strncasecmp_l);
DEFINE_PUBLIC_ALIAS(strdupaf,libc_strdupaf);
DEFINE_PUBLIC_ALIAS(vstrdupaf,libc_vstrdupaf);
#undef ffs
DEFINE_PUBLIC_ALIAS(ffs,PP_CAT2(libc___ffs,PP_MUL8(__SIZEOF_INT__)));
DEFINE_PUBLIC_ALIAS(ffsl,PP_CAT2(libc___ffs,PP_MUL8(__SIZEOF_LONG__)));
DEFINE_PUBLIC_ALIAS(ffsll,PP_CAT2(libc___ffs,PP_MUL8(__SIZEOF_LONG_LONG__)));
DEFINE_PUBLIC_ALIAS(__stpcpy,libc_stpcpy);
DEFINE_PUBLIC_ALIAS(__stpncpy,libc_stpncpy);
DEFINE_PUBLIC_ALIAS(__mempcpy,libc_mempcpy);
#undef memcpyb
DEFINE_PUBLIC_ALIAS(memcpyb,libc_memcpy);
DEFINE_PUBLIC_ALIAS(memmoveb,libc_memmove);
DEFINE_PUBLIC_ALIAS(memsetb,libc_memset);
DEFINE_PUBLIC_ALIAS(memcmpb,libc_memcmp);
DEFINE_PUBLIC_ALIAS(mempatb,libc_memset);
DEFINE_PUBLIC_ALIAS(memlenb,libc_memlen);
DEFINE_PUBLIC_ALIAS(memrlenb,libc_memrlen);
DEFINE_PUBLIC_ALIAS(rawmemlenb,libc_rawmemlen);
DEFINE_PUBLIC_ALIAS(rawmemrlenb,libc_rawmemrlen);
DEFINE_PUBLIC_ALIAS(memchrb,libc_memchr);
DEFINE_PUBLIC_ALIAS(memrchrb,libc_memrchr);
DEFINE_PUBLIC_ALIAS(memendb,libc_memend);
DEFINE_PUBLIC_ALIAS(memrendb,libc_memrend);
DEFINE_PUBLIC_ALIAS(rawmemchrb,libc_rawmemchr);
DEFINE_PUBLIC_ALIAS(rawmemrchrb,libc_rawmemrchr);
DEFINE_PUBLIC_ALIAS(_memcpyb_d,libc__memcpy_d);
DEFINE_PUBLIC_ALIAS(bcmp,libc_memcmp);
DEFINE_PUBLIC_ALIAS(__bzero,libc_bzero);
DEFINE_PUBLIC_ALIAS(__strtok_r,libc_strtok_r);

/* Already exported (under different names) by DOS. */
// DEFINE_PUBLIC_ALIAS(strlwr,libc_strlwr);
// DEFINE_PUBLIC_ALIAS(strupr,libc_strupr);
// DEFINE_PUBLIC_ALIAS(strset,libc_strset);
// DEFINE_PUBLIC_ALIAS(strnset,libc_strnset);
// DEFINE_PUBLIC_ALIAS(strrev,libc_strrev);
// DEFINE_PUBLIC_ALIAS(strcasecoll,libc_strcasecoll);
// DEFINE_PUBLIC_ALIAS(strncoll,libc_strncoll);
// DEFINE_PUBLIC_ALIAS(strncasecoll,libc_strncasecoll);
// DEFINE_PUBLIC_ALIAS(strlwr_l,libc_strlwr_l);
// DEFINE_PUBLIC_ALIAS(strupr_l,libc_strupr_l);
// DEFINE_PUBLIC_ALIAS(strcasecoll_l,libc_strcasecoll_l);
// DEFINE_PUBLIC_ALIAS(strncoll_l,libc_strncoll_l);
// DEFINE_PUBLIC_ALIAS(strncasecoll_l,libc_strncasecoll_l);
// DEFINE_PUBLIC_ALIAS(memcasecmp_l,libc_memcasecmp_l);

/* Define 32-bit wide string libc functions. */
#define T            char32_t
#define Ts           s32
#define Tu           u32
#define Tn           wint_t
#define Tneedle      T
#define X(x)         libc_32w##x
#define Xstr(x)      libc_32wcs##x
#define Xstp(x)      libc_32wcp##x
#define Xmb(x)       libc_32mb##x
#define Xwc(x)       libc_32wc##x
#define Xa(x)        libc_32w##x
#define Xxa(x)       libc_32##x##w
#define Xxa_s(x)     libc_32##x##w_s
#define TOLOWER(x)   libc_towlower(x)
#define TOUPPER(x)   libc_towupper(x)
#define S            4
#define DECL         INTERN ATTR_RARETEXT
#define DEFINE_ALIAS DEFINE_INTERN_ALIAS

/* Select API-set for 32-bit strings. */
#  define WANT_STREND         /* wcsend() */
#  define WANT_STRNEND        /* wcsnend() */
#  define WANT_STRLEN         /* wcslen() */
#  define WANT_STRNLEN        /* wcsnlen() */
#  define WANT_STRCHRNUL      /* wcschrnul() */
#  define WANT_STRCHR         /* wcschr() */
#  define WANT_STRRCHR        /* wcsrchr() */
//#define WANT_STRRCHRNUL     /* wcsrchrnul() */
//#define WANT_STRNCHR        /* wcsnchr() */
//#define WANT_STRNRCHR       /* wcsnrchr() */
//#define WANT_STRNCHRNUL     /* wcsnchrnul() */
//#define WANT_STRNRCHRNUL    /* wcsnrchrnul() */
//#define WANT_STROFF         /* wcsoff() */
//#define WANT_STRROFF        /* wcsroff() */
//#define WANT_STRNOFF        /* wcsnoff() */
//#define WANT_STRNROFF       /* wcsnroff() */
#  define WANT_STPCPY         /* wcpcpy() */
#  define WANT_STPNCPY        /* wcpncpy() */
#  define WANT_STRCMP         /* wcscmp() */
#  define WANT_STRNCMP        /* wcsncmp() */
#  define WANT_STRCASECMP     /* wcscasecmp() */
#  define WANT_STRNCASECMP    /* wcsncasecmp() */
#  define WANT_STRSTR         /* wcsstr() */
//#define WANT_STRCASESTR     /* wcscasestr() */
#  define WANT_STRCPY         /* wcscpy() */
#  define WANT_STRNCPY        /* wcsncpy() */
#  define WANT_STRCAT         /* wcscat() */
#  define WANT_STRNCAT        /* wcsncat() */
#  define WANT_STRCSPN        /* wcscspn() */
#  define WANT_STRSPN         /* wcsspn() */
#  define WANT_STRPBRK        /* wcspbrk() */
#  define WANT_STRTOK_R       /* wcstok_r() */
#  define WANT_STRTOK         /* wcstok() */
//#define WANT_STRFRY         /* wcsfry() */
#  define WANT_STRLWR         /* wcslwr() */
#  define WANT_STRLWR_L       /* wcslwr_l() */
#  define WANT_STRUPR         /* wcsupr() */
#  define WANT_STRUPR_L       /* wcsupr_l() */
//#define WANT_STRNLWR        /* wcsnlwr() */
//#define WANT_STRNLWR_L      /* wcsnlwr_l() */
//#define WANT_STRNUPR        /* wcsnupr() */
//#define WANT_STRNUPR_L      /* wcsnupr_l() */
#  define WANT_STRSET         /* wcsset() */
#  define WANT_STRNSET        /* wcsnset() */
#  define WANT_STRREV         /* wcsrev() */
//#define WANT_STRNREV        /* wcsnrev() */
#  define WANT_STRCOLL        /* wcscoll() */
#  define WANT_STRCOLL_L      /* wcscoll_l() */
#  define WANT_STRNCOLL       /* wcsncoll() */
#  define WANT_STRNCOLL_L     /* wcsncoll_l() */
#  define WANT_STRCASECOLL    /* wcscasecoll() */
#  define WANT_STRCASECOLL_L  /* wcscasecoll_l() */
#  define WANT_STRNCASECOLL   /* wcsncasecoll() */
#  define WANT_STRNCASECOLL_L /* wcsncasecoll_l() */
#  define WANT_STRXFRM        /* wcsxfrm       () */
#  define WANT_STRXFRM_L      /* wcsxfrm_l() */
#  define WANT_STRCASECMP     /* wcscasecmp() */
#  define WANT_STRCASECMP_L   /* wcscasecmp_l() */
#  define WANT_STRNCASECMP    /* wcsncasecmp() */
#  define WANT_STRNCASECMP_L  /* wcsncasecmp_l() */
#  define WANT_STRDUP         /* wcsdup() */
//#define WANT_STRNDUP        /* wcsndup() */
#  define WANT_STRCAT_S       /* wcscat_s() */
#  define WANT_STRCPY_S       /* wcscpy_s() */
#  define WANT_STRLWR_S       /* wcslwr_s() */
#  define WANT_STRLWR_S_L     /* wcslwr_s_l() */
#  define WANT_STRUPR_S       /* wcsupr_s() */
#  define WANT_STRUPR_S_L     /* wcsupr_s_l() */
#  define WANT_STRNCAT_S      /* wcsncat_s() */
#  define WANT_STRNCPY_S      /* wcsncpy_s() */
#  define WANT_STRSET_S       /* wcsset_s() */
#  define WANT_STRNSET_S      /* wcsnset_s() */
#  define WANT_MBLEN          /* mblen(), mbrlen() */
#  define WANT_MBLEN_L        /* mblen_l() */
#  define WANT_MBTOWC         /* mbtowc(), mbrtowc(), mbsnrtowcs(), mbsrtowcs(), mbstowcs() */
#  define WANT_WCTOMB         /* wctomb(), wcrtomb(), wcsnrtombs(), wcsrtombs(), wcstombs() */
#  define WANT_WCWIDTH        /* wcwidth(), wcswidth() */
#  define WANT_MBSRTOWCS_S    /* mbsrtowcs_s() */
#  define WANT_WCSRTOMBS_S    /* wcsrtombs_s() */
#  define WANT_WCRTOMB_S      /* wcrtomb_s() */
#  define WANT_STRTOU32       /* wcstou32() */
#  define WANT_STRTOU64       /* wcstou64() */
#  define WANT_STRTO32        /* wcsto32() */
#  define WANT_STRTO64        /* wcsto64() */
#  define WANT_STRTOU32_L     /* wcstou32_l() */
#  define WANT_STRTOU64_L     /* wcstou64_l() */
#  define WANT_STRTO32_L      /* wcsto32_l() */
#  define WANT_STRTO64_L      /* wcsto64_l() */
#  define WANT_ATOF           /* wtof() */
#  define WANT_ATO32          /* wto32() */
#  define WANT_ATO64          /* wto64() */
#  define WANT_ATOF_L         /* wtof_l() */
#  define WANT_ATO32_L        /* wto32_l() */
#  define WANT_ATO64_L        /* wto64_l() */
#  define WANT_STRTOLD        /* wcstold() */
#  define WANT_STRTOF         /* wcstof() */
#  define WANT_STRTOD         /* wcstod() */
#  define WANT_STRTOLD_L      /* wcstold_l() */
#  define WANT_STRTOF_L       /* wcstof_l() */
#  define WANT_STRTOD_L       /* wcstod_l() */
#  define WANT_U32TOA_S       /* u32tow_s() */
#  define WANT_U64TOA_S       /* u64tow_s() */
#  define WANT_S32TOA_S       /* s32tow_s() */
#  define WANT_S64TOA_S       /* s64tow_s() */
#  define WANT_U64TOA         /* u64tow() */
#  define WANT_U32TOA         /* u32tow() */
#  define WANT_S64TOA         /* s64tow() */
#  define WANT_S32TOA         /* s32tow() */
#  define WANT_SPLITPATH_S    /* splitpath_s() */
#  define WANT_SPLITPATH      /* splitpath() */


/* Disable API functions only used in DOS mode.
 * s.a.: 'Additional DOS 32-bit string functions' in '/libs/libc/string.h' */
#ifdef CONFIG_LIBC_NO_DOS_LIBC
#undef WANT_STRLWR
#undef WANT_STRLWR_L
#undef WANT_STRNSET
#undef WANT_STRREV
#undef WANT_STRSET
#undef WANT_STRUPR
#undef WANT_STRUPR_L
#undef WANT_STRCASECOLL
#undef WANT_STRCASECOLL_L
#undef WANT_STRNCASECOLL
#undef WANT_STRNCASECOLL_L
#undef WANT_STRNCOLL
#undef WANT_STRNCOLL_L
#undef WANT_STRCAT_S
#undef WANT_STRCPY_S
#undef WANT_STRLWR_S
#undef WANT_STRLWR_S_L
#undef WANT_STRUPR_S
#undef WANT_STRUPR_S_L
#undef WANT_STRNCAT_S
#undef WANT_STRNCPY_S
#undef WANT_STRSET_S
#undef WANT_STRNSET_S
#undef WANT_MBLEN_L
#undef WANT_MBSRTOWCS_S
#undef WANT_WCSRTOMBS_S
#undef WANT_WCRTOMB_S
#undef WANT_ATOF
#undef WANT_ATO32
#undef WANT_ATO64
#undef WANT_ATOF_L
#undef WANT_ATO32_L
#undef WANT_ATO64_L
#undef WANT_U32TOA_S
#undef WANT_U64TOA_S
#undef WANT_S32TOA_S
#undef WANT_S64TOA_S
#undef WANT_U32TOA
#undef WANT_U64TOA
#undef WANT_S32TOA
#undef WANT_S64TOA
#undef WANT_SPLITPATH_S
#undef WANT_SPLITPATH
#endif /* CONFIG_LIBC_NO_DOS_LIBC */

DEFINE_INTERN_ALIAS(libc_32wmemcpy,libc_memcpyl);
DEFINE_INTERN_ALIAS(libc_32wmempcpy,libc_mempcpyl);
DEFINE_INTERN_ALIAS(libc_32wmemset,libc_memsetl);
DEFINE_INTERN_ALIAS(libc_32wmemmove,libc_memmovel);
DEFINE_INTERN_ALIAS(libc_32wmemcmp,libc_memcmpl);
DEFINE_INTERN_ALIAS(libc_32wmemchr,libc_memchrl);
#include "templates/string.code"

DEFINE_PUBLIC_ALIAS(wcstok,libc_32wcstok_r);
DEFINE_PUBLIC_ALIAS(__wcstok_f,libc_32wcstok_r);
DEFINE_PUBLIC_ALIAS(__mbrlen,libc_32mbrlen);
DEFINE_PUBLIC_ALIAS(mbrlen,libc_32mbrlen);
DEFINE_PUBLIC_ALIAS(mblen,libc_32mblen);
DEFINE_PUBLIC_ALIAS(mbrtowc,libc_32mbrtowc);
DEFINE_PUBLIC_ALIAS(mbsnrtowcs,libc_32mbsnrtowcs);
DEFINE_PUBLIC_ALIAS(mbsrtowcs,libc_32mbsrtowcs);
DEFINE_PUBLIC_ALIAS(mbstowcs,libc_32mbstowcs);
DEFINE_PUBLIC_ALIAS(mbtowc,libc_32mbtowc);
DEFINE_PUBLIC_ALIAS(wcpcpy,libc_32wcpcpy);
DEFINE_PUBLIC_ALIAS(wcpncpy,libc_32wcpncpy);
DEFINE_PUBLIC_ALIAS(wcrtomb,libc_32wcrtomb);
DEFINE_PUBLIC_ALIAS(wcscasecmp,libc_32wcscasecmp);
DEFINE_PUBLIC_ALIAS(wcscasecmp_l,libc_32wcscasecmp_l);
DEFINE_PUBLIC_ALIAS(wcscat,libc_32wcscat);
DEFINE_PUBLIC_ALIAS(wcschr,libc_32wcschr);
DEFINE_PUBLIC_ALIAS(wcschrnul,libc_32wcschrnul);
DEFINE_PUBLIC_ALIAS(wcscmp,libc_32wcscmp);
DEFINE_PUBLIC_ALIAS(wcscoll,libc_32wcscoll);
DEFINE_PUBLIC_ALIAS(wcscoll_l,libc_32wcscoll_l);
DEFINE_PUBLIC_ALIAS(wcscpy,libc_32wcscpy);
DEFINE_PUBLIC_ALIAS(wcscspn,libc_32wcscspn);
DEFINE_PUBLIC_ALIAS(wcsdup,libc_32wcsdup);
DEFINE_PUBLIC_ALIAS(wcsend,libc_32wcsend);
DEFINE_PUBLIC_ALIAS(wcslen,libc_32wcslen);
DEFINE_PUBLIC_ALIAS(wcsncasecmp,libc_32wcsncasecmp);
DEFINE_PUBLIC_ALIAS(wcsncasecmp_l,libc_32wcsncasecmp_l);
DEFINE_PUBLIC_ALIAS(wcsncat,libc_32wcsncat);
DEFINE_PUBLIC_ALIAS(wcsncmp,libc_32wcsncmp);
DEFINE_PUBLIC_ALIAS(wcsncpy,libc_32wcsncpy);
DEFINE_PUBLIC_ALIAS(wcsnend,libc_32wcsnend);
DEFINE_PUBLIC_ALIAS(wcsnlen,libc_32wcsnlen);
DEFINE_PUBLIC_ALIAS(wcsnrtombs,libc_32wcsnrtombs);
DEFINE_PUBLIC_ALIAS(wcspbrk,libc_32wcspbrk);
DEFINE_PUBLIC_ALIAS(wcsrchr,libc_32wcsrchr);
DEFINE_PUBLIC_ALIAS(wcsrtombs,libc_32wcsrtombs);
DEFINE_PUBLIC_ALIAS(wcsspn,libc_32wcsspn);
DEFINE_PUBLIC_ALIAS(wcsstr,libc_32wcsstr);
DEFINE_PUBLIC_ALIAS(wcswcs,libc_32wcsstr);
DEFINE_PUBLIC_ALIAS(wcstombs,libc_32wcstombs);
DEFINE_PUBLIC_ALIAS(wcwidth,libc_32wcwidth);
DEFINE_PUBLIC_ALIAS(wcswidth,libc_32wcswidth);
DEFINE_PUBLIC_ALIAS(wcsxfrm,libc_32wcsxfrm);
DEFINE_PUBLIC_ALIAS(wcsxfrm_l,libc_32wcsxfrm_l);
DEFINE_PUBLIC_ALIAS(wctomb,libc_32wctomb);
DEFINE_PUBLIC_ALIAS(wmemchr,libc_32wmemchr);
DEFINE_PUBLIC_ALIAS(wmemcmp,libc_32wmemcmp);
DEFINE_PUBLIC_ALIAS(wmemcpy,libc_32wmemcpy);
DEFINE_PUBLIC_ALIAS(wmemmove,libc_32wmemmove);
DEFINE_PUBLIC_ALIAS(wmempcpy,libc_32wmempcpy);
DEFINE_PUBLIC_ALIAS(wmemset,libc_32wmemset);

DEFINE_PUBLIC_ALIAS(wcstol,__LONGFUN(libc_32wcsto));
DEFINE_PUBLIC_ALIAS(wcstoul,__LONGFUN(libc_32wcstou));
DEFINE_PUBLIC_ALIAS(wcstoll,__LONGLONGFUN(libc_32wcsto));
DEFINE_PUBLIC_ALIAS(wcstoull,__LONGLONGFUN(libc_32wcstou));
DEFINE_PUBLIC_ALIAS(wcstol_l,__LONGFUN_L(libc_32wcsto));
DEFINE_PUBLIC_ALIAS(wcstoul_l,__LONGFUN_L(libc_32wcstou));
DEFINE_PUBLIC_ALIAS(wcstoll_l,__LONGLONGFUN_L(libc_32wcsto));
DEFINE_PUBLIC_ALIAS(wcstoull_l,__LONGLONGFUN_L(libc_32wcstou));

DEFINE_PUBLIC_ALIAS(wcstof,libc_32wcstof);
DEFINE_PUBLIC_ALIAS(wcstod,libc_32wcstod);
DEFINE_PUBLIC_ALIAS(wcstold,libc_32wcstold);
DEFINE_PUBLIC_ALIAS(wcstof_l,libc_32wcstof_l);
DEFINE_PUBLIC_ALIAS(wcstod_l,libc_32wcstod_l);
DEFINE_PUBLIC_ALIAS(wcstold_l,libc_32wcstold_l);

#if __SIZEOF_INTMAX_T__ == 8
DEFINE_PUBLIC_ALIAS(strtoimax,libc_strto64);
DEFINE_PUBLIC_ALIAS(strtoumax,libc_strtou64);
DEFINE_PUBLIC_ALIAS(wcstoimax,libc_32wcsto64);
DEFINE_PUBLIC_ALIAS(wcstoumax,libc_32wcstou64);
#elif __SIZEOF_INTMAX_T__ == 4
DEFINE_PUBLIC_ALIAS(strtoimax,libc_strto32);
DEFINE_PUBLIC_ALIAS(strtoumax,libc_strtou32);
DEFINE_PUBLIC_ALIAS(wcstoimax,libc_32wcsto32);
DEFINE_PUBLIC_ALIAS(wcstoumax,libc_32wcstou32);
#else
#error FIXME
#endif

#ifndef CONFIG_LIBC_NO_DOS_LIBC
/* DOS does some trickery to not have to export these.
 * >> So we don't need to export them. */
//DEFINE_PUBLIC_ALIAS(__DSYM(_wcstoimax),libc_16wcsto64);
//DEFINE_PUBLIC_ALIAS(__DSYM(_wcstoumax),libc_16wcstou64);
//DEFINE_PUBLIC_ALIAS(__DSYM(_wcstoimax_l),libc_16wcsto64_l);
//DEFINE_PUBLIC_ALIAS(__DSYM(_wcstoumax_l),libc_16wcstou64_l);
#endif



/* DOS libc functions. */
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#undef libc_strdup
/* Define DOS-mode string function aliases. */
DEFINE_PUBLIC_ALIAS(_strdup,libc_strdup);
DEFINE_PUBLIC_ALIAS(_strlwr,libc_strlwr);
DEFINE_PUBLIC_ALIAS(_strlwr_l,libc_strlwr_l);
DEFINE_PUBLIC_ALIAS(_strnset,libc_strnset);
DEFINE_PUBLIC_ALIAS(_strrev,libc_strrev);
DEFINE_PUBLIC_ALIAS(_strset,libc_strset);
DEFINE_PUBLIC_ALIAS(_strupr,libc_strupr);
DEFINE_PUBLIC_ALIAS(_strupr_l,libc_strupr_l);
//DEFINE_PUBLIC_ALIAS(_memicmp,libc_memcasecmp); /* Always exported (Primary name) */
DEFINE_PUBLIC_ALIAS(_memicmp_l,libc_memcasecmp_l);
DEFINE_PUBLIC_ALIAS(_strcmpi,libc_strcasecmp);
DEFINE_PUBLIC_ALIAS(_strcoll_l,libc_strcoll_l);
DEFINE_PUBLIC_ALIAS(_stricmp,libc_strcasecmp);
DEFINE_PUBLIC_ALIAS(_stricmp_l,libc_strcasecmp_l);
DEFINE_PUBLIC_ALIAS(_stricoll,libc_strcasecoll);
DEFINE_PUBLIC_ALIAS(_stricoll_l,libc_strcasecoll_l);
DEFINE_PUBLIC_ALIAS(_strncoll,libc_strncoll);
DEFINE_PUBLIC_ALIAS(_strncoll_l,libc_strncoll_l);
DEFINE_PUBLIC_ALIAS(_strnicmp,libc_strncasecmp);
DEFINE_PUBLIC_ALIAS(_strnicmp_l,libc_strncasecmp_l);
DEFINE_PUBLIC_ALIAS(_strnicoll,libc_strncasecoll);
DEFINE_PUBLIC_ALIAS(_strnicoll_l,libc_strncasecoll_l);
DEFINE_PUBLIC_ALIAS(_strxfrm_l,libc_strxfrm_l);
DEFINE_PUBLIC_ALIAS(_memccpy,libc_memccpy);
DEFINE_PUBLIC_ALIAS(__strncnt,libc_strnlen);
DEFINE_PUBLIC_ALIAS(_gcvt,libc_gcvt);
DEFINE_PUBLIC_ALIAS(_ecvt,libc_ecvt);
DEFINE_PUBLIC_ALIAS(_fcvt,libc_fcvt);
DEFINE_PUBLIC_ALIAS(_strlwr_s,libc_strlwr_s);
DEFINE_PUBLIC_ALIAS(_strlwr_s_l,libc_strlwr_s_l);
DEFINE_PUBLIC_ALIAS(_strupr_s,libc_strupr_s);
DEFINE_PUBLIC_ALIAS(_strupr_s_l,libc_strupr_s_l);
DEFINE_PUBLIC_ALIAS(_strset_s,libc_strset_s);
DEFINE_PUBLIC_ALIAS(_strnset_s,libc_strnset_s);
DEFINE_PUBLIC_ALIAS(strcat_s,libc_strcat_s);
DEFINE_PUBLIC_ALIAS(strcpy_s,libc_strcpy_s);
DEFINE_PUBLIC_ALIAS(strncat_s,libc_strncat_s);
DEFINE_PUBLIC_ALIAS(strncpy_s,libc_strncpy_s);
DEFINE_PUBLIC_ALIAS(strtok_s,libc_strtok_r);
DEFINE_PUBLIC_ALIAS(__DSYM(strtold_l),libc_strtod_l);
DEFINE_PUBLIC_ALIAS(_strtof_l,libc_strtof_l);
DEFINE_PUBLIC_ALIAS(_strtoimax_l,libc_strto64_l);
DEFINE_PUBLIC_ALIAS(_strtold_l,libc_strtod_l);
DEFINE_PUBLIC_ALIAS(_strtoll_l,libc_strto64_l);
DEFINE_PUBLIC_ALIAS(_strtoull_l,libc_strtou64_l);
DEFINE_PUBLIC_ALIAS(_strtoumax_l,libc_strtou64_l);

INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_ecvt_s(char *buf, size_t buflen, double val, int ndigit, int *__restrict decptr, int *__restrict sign) { NOT_IMPLEMENTED(); return libc_ecvt_r(val,ndigit,decptr,sign,buf,buflen) ? EOK : GET_DOS_ERRNO(); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_fcvt_s(char *buf, size_t buflen, double val, int ndigit, int *__restrict decptr, int *__restrict sign) { NOT_IMPLEMENTED(); return libc_fcvt_r(val,ndigit,decptr,sign,buf,buflen) ? EOK : GET_DOS_ERRNO(); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_gcvt_s(char *buf, size_t buflen, double val, int ndigit) { NOT_IMPLEMENTED(); return libc_gcvt(val,ndigit,buf) ? EOK : GET_DOS_ERRNO(); }
DEFINE_PUBLIC_ALIAS(_ecvt_s,libc_ecvt_s);
DEFINE_PUBLIC_ALIAS(_fcvt_s,libc_fcvt_s);
DEFINE_PUBLIC_ALIAS(_gcvt_s,libc_gcvt_s);

/* Define 16-bit wide string libc functions. */
#define T            char16_t
#define Ts           s16
#define Tu           u16
#define Tn           wint_t
#define Tneedle      T
#define X(x)         libc_16w##x
#define Xstr(x)      libc_16wcs##x
#define Xstp(x)      libc_16wcp##x
#define Xmb(x)       libc_16mb##x
#define Xwc(x)       libc_16wc##x
#define Xa(x)        libc_16w##x
#define Xxa(x)       libc_16##x##w
#define Xxa_s(x)     libc_16##x##w_s
#define TOLOWER(x)   libc_towlower(x)
#define TOUPPER(x)   libc_towupper(x)
#define S            2
#define DECL         INTERN ATTR_DOSTEXT
#define DEFINE_ALIAS DEFINE_INTERN_ALIAS

/* Select API-set for 16-bit strings. */
#  define WANT_STREND         /* wcsend() */
#  define WANT_STRNEND        /* wcsnend() */
#  define WANT_STRLEN         /* wcslen() */
#  define WANT_STRNLEN        /* wcsnlen() */
#  define WANT_STRCHRNUL      /* wcschrnul() */
#  define WANT_STRCHR         /* wcschr() */
#  define WANT_STRRCHR        /* wcsrchr() */
//#define WANT_STRRCHRNUL     /* wcsrchrnul() */
//#define WANT_STRNCHR        /* wcsnchr() */
//#define WANT_STRNRCHR       /* wcsnrchr() */
//#define WANT_STRNCHRNUL     /* wcsnchrnul() */
//#define WANT_STRNRCHRNUL    /* wcsnrchrnul() */
//#define WANT_STROFF         /* wcsoff() */
//#define WANT_STRROFF        /* wcsroff() */
//#define WANT_STRNOFF        /* wcsnoff() */
//#define WANT_STRNROFF       /* wcsnroff() */
#  define WANT_STPCPY         /* wcpcpy() */
#  define WANT_STPNCPY        /* wcpncpy() */
#  define WANT_STRCMP         /* wcscmp() */
#  define WANT_STRNCMP        /* wcsncmp() */
#  define WANT_STRCASECMP     /* wcscasecmp() */
#  define WANT_STRNCASECMP    /* wcsncasecmp() */
#  define WANT_STRSTR         /* strstr() */
//#define WANT_STRCASESTR     /* strcasestr() */
#  define WANT_STRCPY         /* wcscpy() */
#  define WANT_STRNCPY        /* wcsncpy() */
#  define WANT_STRCAT         /* wcscat() */
#  define WANT_STRNCAT        /* wcsncat() */
#  define WANT_STRCSPN        /* wcscspn() */
#  define WANT_STRSPN         /* wcsspn() */
#  define WANT_STRPBRK        /* wcspbrk() */
#  define WANT_STRTOK_R       /* wcstok_r() */
#  define WANT_STRTOK         /* wcstok() */
//#define WANT_STRFRY         /* wcsfry() */
#  define WANT_STRLWR         /* wcslwr() */
#  define WANT_STRLWR_L       /* wcslwr_l() */
#  define WANT_STRUPR         /* wcsupr() */
#  define WANT_STRUPR_L       /* wcsupr_l() */
//#define WANT_STRNLWR        /* wcsnlwr() */
//#define WANT_STRNLWR_L      /* wcsnlwr_l() */
//#define WANT_STRNUPR        /* wcsnupr() */
//#define WANT_STRNUPR_L      /* wcsnupr_l() */
#  define WANT_STRSET         /* wcsset() */
#  define WANT_STRNSET        /* wcsnset() */
#  define WANT_STRREV         /* wcsrev() */
#  define WANT_STRNREV        /* wcsnrev() */
#  define WANT_STRCOLL        /* wcscoll() */
#  define WANT_STRCOLL_L      /* wcscoll_l() */
#  define WANT_STRNCOLL       /* wcsncoll() */
#  define WANT_STRNCOLL_L     /* wcsncoll_l() */
#  define WANT_STRCASECOLL    /* wcscasecoll() */
#  define WANT_STRCASECOLL_L  /* wcscasecoll_l() */
#  define WANT_STRNCASECOLL   /* wcsncasecoll() */
#  define WANT_STRNCASECOLL_L /* wcsncasecoll_l() */
#  define WANT_STRXFRM        /* wcsxfrm       () */
#  define WANT_STRXFRM_L      /* wcsxfrm_l() */
#  define WANT_STRCASECMP     /* wcscasecmp() */
#  define WANT_STRCASECMP_L   /* wcscasecmp_l() */
#  define WANT_STRNCASECMP    /* wcsncasecmp() */
#  define WANT_STRNCASECMP_L  /* wcsncasecmp_l() */
#  define WANT_STRDUP         /* wcsdup() */
//#define WANT_STRNDUP        /* wcsndup() */
#  define WANT_STRCAT_S       /* wcscat_s() */
#  define WANT_STRCPY_S       /* wcscpy_s() */
#  define WANT_STRLWR_S       /* wcslwr_s() */
#  define WANT_STRLWR_S_L     /* wcslwr_s_l() */
#  define WANT_STRUPR_S       /* wcsupr_s() */
#  define WANT_STRUPR_S_L     /* wcsupr_s_l() */
#  define WANT_STRNCAT_S      /* wcsncat_s() */
#  define WANT_STRNCPY_S      /* wcsncpy_s() */
#  define WANT_STRSET_S       /* wcsset_s() */
#  define WANT_STRNSET_S      /* wcsnset_s() */
#  define WANT_MBLEN          /* mblen(), mbrlen() */
#  define WANT_MBLEN_L        /* mblen_l() */
#  define WANT_MBTOWC         /* mbtowc(), mbrtowc(), mbsnrtowcs(), mbsrtowcs(), mbstowcs() */
#  define WANT_WCTOMB         /* wctomb(), wcrtomb(), wcsnrtombs(), wcsrtombs(), wcstombs() */
#  define WANT_WCWIDTH        /* wcwidth(), wcswidth() */
#  define WANT_MBSRTOWCS_S    /* mbsrtowcs_s() */
#  define WANT_WCSRTOMBS_S    /* wcsrtombs_s() */
#  define WANT_WCRTOMB_S      /* wcrtomb_s() */
#  define WANT_STRTOU32       /* wcstou32() */
#  define WANT_STRTOU64       /* wcstou64() */
#  define WANT_STRTO32        /* wcsto32() */
#  define WANT_STRTO64        /* wcsto64() */
#  define WANT_STRTOU32_L     /* wcstou32_l() */
#  define WANT_STRTOU64_L     /* wcstou64_l() */
#  define WANT_STRTO32_L      /* wcsto32_l() */
#  define WANT_STRTO64_L      /* wcsto64_l() */
#  define WANT_ATOF           /* wtof() */
#  define WANT_ATO32          /* wto32() */
#  define WANT_ATO64          /* wto64() */
#  define WANT_ATOF_L         /* wtof_l() */
#  define WANT_ATO32_L        /* wto32_l() */
#  define WANT_ATO64_L        /* wto64_l() */
#  define WANT_STRTOLD        /* wcstold() */
#  define WANT_STRTOF         /* wcstof() */
#  define WANT_STRTOD         /* wcstod() */
#  define WANT_STRTOLD_L      /* wcstold_l() */
#  define WANT_STRTOF_L       /* wcstof_l() */
#  define WANT_STRTOD_L       /* wcstod_l() */
#  define WANT_U32TOA_S       /* u32tow_s() */
#  define WANT_U64TOA_S       /* u64tow_s() */
#  define WANT_S32TOA_S       /* s32tow_s() */
#  define WANT_S64TOA_S       /* s64tow_s() */
#  define WANT_U32TOA         /* u32tow() */
#  define WANT_U64TOA         /* u64tow() */
#  define WANT_S32TOA         /* s32tow() */
#  define WANT_S64TOA         /* s64tow() */
#  define WANT_SPLITPATH_S    /* splitpath_s() */
#  define WANT_SPLITPATH      /* splitpath() */

DEFINE_INTERN_ALIAS(libc_16wmemcpy,libc_memcpyw);
DEFINE_INTERN_ALIAS(libc_16wmempcpy,libc_mempcpyw);
DEFINE_INTERN_ALIAS(libc_16wmemset,libc_memsetw);
DEFINE_INTERN_ALIAS(libc_16wmemmove,libc_memmovew);
DEFINE_INTERN_ALIAS(libc_16wmemcmp,libc_memcmpw);
DEFINE_INTERN_ALIAS(libc_16wmemchr,libc_memchrw);
#include "templates/string.code"

DEFINE_PUBLIC_ALIAS(__DSYM(wcstok),libc_16wcstok); /* This one's a workaround for a bug in DOS. */
DEFINE_PUBLIC_ALIAS(wcstok_s,libc_16wcstok_r);
DEFINE_PUBLIC_ALIAS(__DSYM(mbrlen),libc_16mbrlen);

DEFINE_PUBLIC_ALIAS(_atof_l,libc_atof_l);
DEFINE_PUBLIC_ALIAS(_atoi_l,__INTFUN_L(libc_ato));
DEFINE_PUBLIC_ALIAS(_atol_l,__DOS_LONGFUN_L(libc_ato));
DEFINE_PUBLIC_ALIAS(_atoll_l,__LONGLONGFUN_L(libc_ato));
DEFINE_PUBLIC_ALIAS(_atoi64,libc_ato64);
DEFINE_PUBLIC_ALIAS(_atoi64_l,libc_ato64_l);
DEFINE_PUBLIC_ALIAS(_strtod_l,libc_strtod_l);
DEFINE_PUBLIC_ALIAS(_strtoi64,libc_strto64);
DEFINE_PUBLIC_ALIAS(_strtoi64_l,libc_strto64_l);
DEFINE_PUBLIC_ALIAS(_strtol_l,__DOS_LONGFUN_L(libc_strto));
DEFINE_PUBLIC_ALIAS(_strtoui64,libc_strtou64);
DEFINE_PUBLIC_ALIAS(_strtoui64_l,libc_strtou64_l);
DEFINE_PUBLIC_ALIAS(_strtoul_l,__DOS_LONGFUN_L(libc_strtou));
DEFINE_PUBLIC_ALIAS(_swab,libc_swab_int);


/* Integer to ascii/utf-16/32 functions. */
DEFINE_PUBLIC_ALIAS(_itoa,libc_s32toa);
DEFINE_PUBLIC_ALIAS(_ltoa,libc_s32toa);
DEFINE_PUBLIC_ALIAS(_i64toa,libc_s64toa);
DEFINE_PUBLIC_ALIAS(_ultoa,libc_u32toa);
DEFINE_PUBLIC_ALIAS(_ui64toa,libc_u64toa);
DEFINE_PUBLIC_ALIAS(_itoa_s,libc_s32toa_s);
DEFINE_PUBLIC_ALIAS(_ltoa_s,libc_s32toa_s);
DEFINE_PUBLIC_ALIAS(_i64toa_s,libc_s64toa_s);
DEFINE_PUBLIC_ALIAS(_ultoa_s,libc_u32toa_s);
DEFINE_PUBLIC_ALIAS(_ui64toa_s,libc_u64toa_s);

/* NOTE: We intentionally _ALWAYS_ export the 32-bit variants, even when `sizeof(long) != 4'!
 *    >> This behavior is fixed by the system headers that dynamically link e.g.: '_ltow'
 *       against either '[_]i64tow' when `sizeof(long) == 8' or '[_]ltow' when `sizeof(long) == 4'
 */
DEFINE_PUBLIC_ALIAS(itow,libc_16s32tow);
DEFINE_PUBLIC_ALIAS(i64tow,libc_16s64tow);
DEFINE_PUBLIC_ALIAS(ui64tow,libc_16u64tow);
DEFINE_PUBLIC_ALIAS(itow_s,libc_16s32tow_s);
DEFINE_PUBLIC_ALIAS(i64tow_s,libc_16s64tow_s);
DEFINE_PUBLIC_ALIAS(ui64tow_s,libc_16u64tow_s);
DEFINE_PUBLIC_ALIAS(ltow,libc_16s32tow);
DEFINE_PUBLIC_ALIAS(ultow,libc_16u32tow);
DEFINE_PUBLIC_ALIAS(ltow_s,libc_16s32tow_s);
DEFINE_PUBLIC_ALIAS(ultow_s,libc_16u32tow_s);

DEFINE_PUBLIC_ALIAS(_itow,libc_16s32tow);
DEFINE_PUBLIC_ALIAS(_i64tow,libc_16s64tow);
DEFINE_PUBLIC_ALIAS(_ltow,libc_16s32tow);
DEFINE_PUBLIC_ALIAS(_ultow,libc_16u32tow);
DEFINE_PUBLIC_ALIAS(_ui64tow,libc_16u64tow);
DEFINE_PUBLIC_ALIAS(_itow_s,libc_16s32tow_s);
DEFINE_PUBLIC_ALIAS(_i64tow_s,libc_16s64tow_s);
DEFINE_PUBLIC_ALIAS(_ltow_s,libc_16s32tow_s);
DEFINE_PUBLIC_ALIAS(_ultow_s,libc_16u32tow_s);
DEFINE_PUBLIC_ALIAS(_ui64tow_s,libc_16u64tow_s);


DEFINE_PUBLIC_ALIAS(__DSYM(mblen),libc_16mblen);
DEFINE_PUBLIC_ALIAS(__DSYM(mbrtowc),libc_16mbrtowc);
DEFINE_PUBLIC_ALIAS(__DSYM(mbsnrtowcs),libc_16mbsnrtowcs);
DEFINE_PUBLIC_ALIAS(__DSYM(mbsrtowcs),libc_16mbsrtowcs);
DEFINE_PUBLIC_ALIAS(__DSYM(mbstowcs),libc_16mbstowcs);
DEFINE_PUBLIC_ALIAS(__DSYM(mbtowc),libc_16mbtowc);
DEFINE_PUBLIC_ALIAS(__DSYM(wcpcpy),libc_16wcpcpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wcpncpy),libc_16wcpncpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wcrtomb),libc_16wcrtomb);
DEFINE_PUBLIC_ALIAS(_wcsicmp,libc_16wcscasecmp);
DEFINE_PUBLIC_ALIAS(_wcsicmp_l,libc_16wcscasecmp_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscat),libc_16wcscat);
DEFINE_PUBLIC_ALIAS(__DSYM(wcschr),libc_16wcschr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcschrnul),libc_16wcschrnul);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscmp),libc_16wcscmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscoll),libc_16wcscoll);
DEFINE_PUBLIC_ALIAS(_wcscoll_l,libc_16wcscoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscpy),libc_16wcscpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscspn),libc_16wcscspn);
DEFINE_PUBLIC_ALIAS(_wcsdup,libc_16wcsdup);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsend),libc_16wcsend);
DEFINE_PUBLIC_ALIAS(__DSYM(wcslen),libc_16wcslen);
DEFINE_PUBLIC_ALIAS(_wcsnicmp,libc_16wcsncasecmp);
DEFINE_PUBLIC_ALIAS(_wcsnicmp_l,libc_16wcsncasecmp_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncat),libc_16wcsncat);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncmp),libc_16wcsncmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncpy),libc_16wcsncpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnend),libc_16wcsnend);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnlen),libc_16wcsnlen);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnrtombs),libc_16wcsnrtombs);
DEFINE_PUBLIC_ALIAS(__DSYM(wcspbrk),libc_16wcspbrk);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsrchr),libc_16wcsrchr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsrtombs),libc_16wcsrtombs);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsspn),libc_16wcsspn);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsstr),libc_16wcsstr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcswcs),libc_16wcsstr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcstombs),libc_16wcstombs);
DEFINE_PUBLIC_ALIAS(__DSYM(wcwidth),libc_16wcwidth);
DEFINE_PUBLIC_ALIAS(__DSYM(wcswidth),libc_16wcswidth);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsxfrm),libc_16wcsxfrm);
DEFINE_PUBLIC_ALIAS(_wcsxfrm_l,libc_16wcsxfrm_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wctomb),libc_16wctomb);
DEFINE_PUBLIC_ALIAS(__DSYM(wmemchr),libc_16wmemchr);
DEFINE_PUBLIC_ALIAS(__DSYM(wmemcmp),libc_16wmemcmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wmemcpy),libc_16wmemcpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wmemmove),libc_16wmemmove);
DEFINE_PUBLIC_ALIAS(__DSYM(wmempcpy),libc_16wmempcpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wmemset),libc_16wmemset);
DEFINE_PUBLIC_ALIAS(__wcsncnt,libc_16wcsnlen); /* Exported by DOS. */

DEFINE_PUBLIC_ALIAS(mblen_l,libc_32mblen_l);
DEFINE_PUBLIC_ALIAS(_mblen_l,libc_16mblen_l);

DEFINE_PUBLIC_ALIAS(wcslwr,libc_32wcslwr);
DEFINE_PUBLIC_ALIAS(wcslwr_l,libc_32wcslwr_l);
DEFINE_PUBLIC_ALIAS(_wcslwr,libc_16wcslwr);
DEFINE_PUBLIC_ALIAS(_wcslwr_l,libc_16wcslwr_l);

DEFINE_PUBLIC_ALIAS(wcsupr,libc_32wcsupr);
DEFINE_PUBLIC_ALIAS(wcsupr_l,libc_32wcsupr_l);
DEFINE_PUBLIC_ALIAS(_wcsupr,libc_16wcsupr);
DEFINE_PUBLIC_ALIAS(_wcsupr_l,libc_16wcsupr_l);

DEFINE_PUBLIC_ALIAS(wcsset,libc_32wcsset);
DEFINE_PUBLIC_ALIAS(wcsnset,libc_32wcsnset);
DEFINE_PUBLIC_ALIAS(_wcsset,libc_16wcsset);
DEFINE_PUBLIC_ALIAS(_wcsnset,libc_16wcsnset);

DEFINE_PUBLIC_ALIAS(wcsrev,libc_32wcsrev);
DEFINE_PUBLIC_ALIAS(_wcsrev,libc_16wcsrev);

DEFINE_PUBLIC_ALIAS(wcscasecoll,libc_32wcscasecoll);
DEFINE_PUBLIC_ALIAS(wcscasecoll_l,libc_32wcscasecoll_l);
DEFINE_PUBLIC_ALIAS(_wcsicoll,libc_16wcscasecoll);
DEFINE_PUBLIC_ALIAS(_wcsicoll_l,libc_16wcscasecoll_l);

DEFINE_PUBLIC_ALIAS(wcsncoll,libc_32wcsncoll);
DEFINE_PUBLIC_ALIAS(wcsncoll_l,libc_32wcsncoll_l);
DEFINE_PUBLIC_ALIAS(_wcsncoll,libc_16wcsncoll);
DEFINE_PUBLIC_ALIAS(_wcsncoll_l,libc_16wcsncoll_l);

DEFINE_PUBLIC_ALIAS(wcsnicoll,libc_32wcsncasecoll);
DEFINE_PUBLIC_ALIAS(wcsnicoll_l,libc_32wcsncasecoll_l);
DEFINE_PUBLIC_ALIAS(_wcsnicoll,libc_16wcsncasecoll);
DEFINE_PUBLIC_ALIAS(_wcsnicoll_l,libc_16wcsncasecoll_l);

/* DOS SLib functions. */
DEFINE_PUBLIC_ALIAS(mbsrtowcs_s,libc_32mbsrtowcs_s);
DEFINE_PUBLIC_ALIAS(__DSYM(mbsrtowcs_s),libc_16mbsrtowcs_s);

DEFINE_PUBLIC_ALIAS(wcsrtombs_s,libc_32wcsrtombs_s);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsrtombs_s),libc_16wcsrtombs_s);

DEFINE_PUBLIC_ALIAS(wcsset_s,libc_32wcsset_s);
DEFINE_PUBLIC_ALIAS(_wcsset_s,libc_16wcsset_s);

DEFINE_PUBLIC_ALIAS(wcsnset_s,libc_32wcsnset_s);
DEFINE_PUBLIC_ALIAS(_wcsnset_s,libc_16wcsnset_s);

DEFINE_PUBLIC_ALIAS(wcslwr_s,libc_32wcslwr_s);
DEFINE_PUBLIC_ALIAS(_wcslwr_s,libc_16wcslwr_s);

DEFINE_PUBLIC_ALIAS(wcsupr_s,libc_32wcsupr_s);
DEFINE_PUBLIC_ALIAS(_wcsupr_s,libc_16wcsupr_s);

DEFINE_PUBLIC_ALIAS(wcslwr_s_l,libc_32wcslwr_s_l);
DEFINE_PUBLIC_ALIAS(_wcslwr_s_l,libc_16wcslwr_s_l);

DEFINE_PUBLIC_ALIAS(wcsupr_s_l,libc_32wcsupr_s_l);
DEFINE_PUBLIC_ALIAS(_wcsupr_s_l,libc_16wcsupr_s_l);

DEFINE_PUBLIC_ALIAS(wcscat_s,libc_32wcscat_s);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscat_s),libc_16wcscat_s);

DEFINE_PUBLIC_ALIAS(wcscpy_s,libc_32wcscpy_s);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscpy_s),libc_16wcscpy_s);

DEFINE_PUBLIC_ALIAS(wcsncat_s,libc_32wcsncat_s);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncat_s),libc_16wcsncat_s);

DEFINE_PUBLIC_ALIAS(wcsncpy_s,libc_32wcsncpy_s);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncpy_s),libc_16wcsncpy_s);

DEFINE_PUBLIC_ALIAS(wcrtomb_s,libc_32wcrtomb_s);
DEFINE_PUBLIC_ALIAS(__DSYM(wcrtomb_s),libc_16wcrtomb_s);

DEFINE_PUBLIC_ALIAS(wtoi,__INTFUN(libc_32wto));
DEFINE_PUBLIC_ALIAS(wtol,__LONGFUN(libc_32wto));
DEFINE_PUBLIC_ALIAS(wtoi_l,__INTFUN_L(libc_32wto));
DEFINE_PUBLIC_ALIAS(wtol_l,__DOS_LONGFUN_L(libc_32wto));
DEFINE_PUBLIC_ALIAS(_wtoi,__INTFUN(libc_16wto));
DEFINE_PUBLIC_ALIAS(_wtol,__LONGFUN(libc_16wto));
DEFINE_PUBLIC_ALIAS(_wtoi_l,__INTFUN_L(libc_16wto));
DEFINE_PUBLIC_ALIAS(_wtol_l,__DOS_LONGFUN_L(libc_16wto));
DEFINE_PUBLIC_ALIAS(__DSYM(wcstol),__LONGFUN(libc_16wcsto));
DEFINE_PUBLIC_ALIAS(__DSYM(wcstoul),__LONGFUN(libc_16wcstou));
DEFINE_PUBLIC_ALIAS(_wcstol_l,__DOS_LONGFUN_L(libc_16wcsto));
DEFINE_PUBLIC_ALIAS(_wcstoul_l,__DOS_LONGFUN_L(libc_16wcstou));
DEFINE_PUBLIC_ALIAS(_wcstoi64_l,libc_16wcsto64_l);
DEFINE_PUBLIC_ALIAS(_wcstoui64_l,libc_16wcstou64_l);
DEFINE_PUBLIC_ALIAS(wtof,libc_32wtof);
DEFINE_PUBLIC_ALIAS(wtof_l,libc_32wtof_l);
DEFINE_PUBLIC_ALIAS(_wtof,libc_16wtof);
DEFINE_PUBLIC_ALIAS(_wtof_l,libc_16wtof_l);

/* Define fixed-length 64-bit aliases used in PE-mode. */
DEFINE_PUBLIC_ALIAS(wtoi64,libc_32wto64);
DEFINE_PUBLIC_ALIAS(wcstoi64,libc_32wcsto64);
DEFINE_PUBLIC_ALIAS(wcstoui64,libc_32wcstou64);
DEFINE_PUBLIC_ALIAS(wtoi64_l,libc_32wto64_l);
DEFINE_PUBLIC_ALIAS(wcstoi64_l,libc_32wcsto64_l);
DEFINE_PUBLIC_ALIAS(wcstoui64_l,libc_32wcstou64_l);
DEFINE_PUBLIC_ALIAS(_wtoll,libc_16wto64);
DEFINE_PUBLIC_ALIAS(_wtoi64,libc_16wto64);
DEFINE_PUBLIC_ALIAS(_wcstoi64,libc_16wcsto64);
DEFINE_PUBLIC_ALIAS(_wcstoui64,libc_16wcstou64);
DEFINE_PUBLIC_ALIAS(_wtoll_l,libc_16wto64_l);
DEFINE_PUBLIC_ALIAS(_wtoi64_l,libc_16wto64_l);
DEFINE_PUBLIC_ALIAS(_wcstoll_l,libc_16wcsto64_l);
DEFINE_PUBLIC_ALIAS(_wcstoi64_l,libc_16wcsto64_l);
DEFINE_PUBLIC_ALIAS(_wcstoimax_l,libc_16wcsto64_l);
DEFINE_PUBLIC_ALIAS(_wcstoull_l,libc_16wcstou64_l);
DEFINE_PUBLIC_ALIAS(_wcstoui64_l,libc_16wcstou64_l);
DEFINE_PUBLIC_ALIAS(_wcstoumax_l,libc_16wcstou64_l);

DEFINE_PUBLIC_ALIAS(__DSYM(wcstof),libc_16wcstof);
DEFINE_PUBLIC_ALIAS(__DSYM(wcstod),libc_16wcstod);
DEFINE_PUBLIC_ALIAS(__DSYM(wcstold),libc_16wcstod);
DEFINE_PUBLIC_ALIAS(__DSYM(wcstold96),libc_16wcstold);
DEFINE_PUBLIC_ALIAS(_wcstof_l,libc_16wcstof_l);
DEFINE_PUBLIC_ALIAS(_wcstod_l,libc_16wcstod_l);
DEFINE_PUBLIC_ALIAS(_wcstold_l,libc_16wcstod_l);
DEFINE_PUBLIC_ALIAS(_wcstold96_l,libc_16wcstold_l);


#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
DEFINE_INTERN_ALIAS(libc_swab_int,libc_swab);
#else
INTDEF ATTR_DOSTEXT void LIBCCALL
libc_swab_int(void const *__restrict from, void *__restrict to, int n_bytes) {
 libc_swab(from,to,(size_t)n_bytes);
}
#endif

INTERN errno_t LIBCCALL libc_memcpy_s(void *__restrict dst, size_t dstsize, void const *__restrict src, size_t srcsize) { if (dstsize < srcsize) return ERANGE; libc_memcpy(dst,src,srcsize); return EOK; }
INTERN errno_t LIBCCALL libc_memmove_s(void *dst, size_t dstsize, void const *src, size_t srcsize) { if (dstsize < srcsize) return ERANGE; libc_memmove(dst,src,srcsize); return EOK; }
INTERN errno_t LIBCCALL libc_16wmemcpy_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src, size_t srcsize) { if (dstsize < srcsize) return ERANGE; libc_16wmemcpy(dst,src,srcsize); return EOK; }
INTERN errno_t LIBCCALL libc_16wmemmove_s(char16_t *dst, size_t dstsize, char16_t const *src, size_t srcsize) { if (dstsize < srcsize) return ERANGE; libc_16wmemmove(dst,src,srcsize); return EOK; }
INTERN errno_t LIBCCALL libc_32wmemcpy_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src, size_t srcsize) { if (dstsize < srcsize) return ERANGE; libc_32wmemcpy(dst,src,srcsize); return EOK; }
INTERN errno_t LIBCCALL libc_32wmemmove_s(char32_t *dst, size_t dstsize, char32_t const *src, size_t srcsize) { if (dstsize < srcsize) return ERANGE; libc_32wmemmove(dst,src,srcsize); return EOK; }
DEFINE_PUBLIC_ALIAS(memcpy_s,libc_memcpy_s);
DEFINE_PUBLIC_ALIAS(memmove_s,libc_memmove_s);
DEFINE_PUBLIC_ALIAS(wmemcpy_s,libc_32wmemcpy_s);
DEFINE_PUBLIC_ALIAS(wmemmove_s,libc_32wmemmove_s);
DEFINE_PUBLIC_ALIAS(__DSYM(wmemcpy_s),libc_16wmemcpy_s);
DEFINE_PUBLIC_ALIAS(__DSYM(wmemmove_s),libc_16wmemmove_s);



/* Define misc. functions found in DOS's <stdlib.h> header. */
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_mbstrnlen(char const *str, size_t maxlen) { mbstate_t state = MBSTATE_INIT; return libc_utf8to32(str,maxlen,NULL,0,&state,UNICODE_F_NOZEROTERM|UNICODE_F_STOPONNUL|UNICODE_F_SETERRNO); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_mbstrlen(char const *str) { return libc_mbstrnlen(str,(size_t)-1); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_16mbstowcs_s(size_t *presult, char16_t *buf, size_t buflen, char const *src, size_t maxlen) {
 mbstate_t state = MBSTATE_INIT; size_t result;
 result = libc_16mbsnrtowcs(buf,&src,maxlen,buflen,&state);
 if (presult) *presult = result;
 return result != UNICODE_ERROR ? EOK : __DOS_EILSEQ;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_32mbstowcs_s(size_t *presult, char32_t * buf, size_t buflen, char const *src, size_t maxlen) {
 mbstate_t state = MBSTATE_INIT; size_t result;
 result = libc_32mbsnrtowcs(buf,&src,maxlen,buflen,&state);
 if (presult) *presult = result;
 return result != UNICODE_ERROR ? EOK : __DOS_EILSEQ;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_16wctomb_s(int *presult, char *buf, size_t buflen, char16_t wc) {
 char16_t temp[2] = {wc,0};
 size_t result = libc_16wcstombs(buf,temp,buflen);
 if (presult) *presult = (int)result;
 return result != UNICODE_ERROR ? EOK : __DOS_EILSEQ;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_32wctomb_s(int *presult, char *buf, size_t buflen, char32_t wc) {
 char32_t temp[2] = {wc,0};
 size_t result = libc_32wcstombs(buf,temp,buflen);
 if (presult) *presult = (int)result;
 return result != UNICODE_ERROR ? EOK : __DOS_EILSEQ;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_16wcstombs_s(size_t *presult, char *buf, size_t buflen,
                  char16_t const *src, size_t maxlen) {
 mbstate_t state = MBSTATE_INIT; size_t result;
 result = libc_16wcsnrtombs(buf,&src,maxlen,buflen,&state);
 if (presult) *presult = result;
 return result != UNICODE_ERROR ? EOK : __DOS_EILSEQ;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_32wcstombs_s(size_t *presult, char *buf, size_t buflen,
                  char32_t const *src, size_t maxlen) {
 mbstate_t state = MBSTATE_INIT; size_t result;
 result = libc_32wcsnrtombs(buf,&src,maxlen,buflen,&state);
 if (presult) *presult = result;
 return result != UNICODE_ERROR ? EOK : __DOS_EILSEQ;
}

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_mbstrlen_l,libc_mbstrlen);
DEFINE_INTERN_ALIAS(libc_mbstrnlen_l,libc_mbstrnlen);
DEFINE_INTERN_ALIAS(libc_16mbtowc_l,libc_16mbtowc);
DEFINE_INTERN_ALIAS(libc_32mbtowc_l,libc_32mbtowc);
DEFINE_INTERN_ALIAS(libc_16mbstowcs_s_l,libc_16mbstowcs_s);
DEFINE_INTERN_ALIAS(libc_32mbstowcs_s_l,libc_32mbstowcs_s);
DEFINE_INTERN_ALIAS(libc_16mbstowcs_l,libc_16mbstowcs);
DEFINE_INTERN_ALIAS(libc_32mbstowcs_l,libc_32mbstowcs);
DEFINE_INTERN_ALIAS(libc_16wctomb_l,libc_16wctomb);
DEFINE_INTERN_ALIAS(libc_32wctomb_l,libc_32wctomb);
DEFINE_INTERN_ALIAS(libc_16wctomb_s_l,libc_16wctomb_s);
DEFINE_INTERN_ALIAS(libc_32wctomb_s_l,libc_32wctomb_s);
DEFINE_INTERN_ALIAS(libc_16wcstombs_s_l,libc_16wcstombs_s);
DEFINE_INTERN_ALIAS(libc_32wcstombs_s_l,libc_32wcstombs_s);
DEFINE_INTERN_ALIAS(libc_16wcstombs_l,libc_16wcstombs);
DEFINE_INTERN_ALIAS(libc_32wcstombs_l,libc_32wcstombs);
#else
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_mbstrlen_l(char const *str, locale_t UNUSED(locale)) { return libc_mbstrlen(str); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_mbstrnlen_l(char const *str, size_t maxlen, locale_t UNUSED(locale)) { return libc_mbstrnlen(str,maxlen); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_16mbtowc_l(char16_t *dst, char const *src, size_t srclen, locale_t UNUSED(locale)) { return libc_16mbtowc(dst,src,srclen); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_32mbtowc_l(char32_t *dst, char const *src, size_t srclen, locale_t UNUSED(locale)) { return libc_32mbtowc(dst,src,srclen); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_16mbstowcs_s_l(size_t *presult, char16_t *buf, size_t buflen, char const *src, size_t maxlen, locale_t UNUSED(locale)) { return libc_16mbstowcs_s(presult,buf,buflen,src,maxlen); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_32mbstowcs_s_l(size_t *presult, char32_t *buf, size_t buflen, char const *src, size_t maxlen, locale_t UNUSED(locale)) { return libc_32mbstowcs_s(presult,buf,buflen,src,maxlen); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_16mbstowcs_l(char16_t *buf, char const *src, size_t maxlen, locale_t UNUSED(locale)) { return libc_16mbstowcs(buf,src,maxlen); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_32mbstowcs_l(char32_t *buf, char const *src, size_t maxlen, locale_t UNUSED(locale)) { return libc_32mbstowcs(buf,src,maxlen); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_16wctomb_l(char *buf, char16_t wc, locale_t UNUSED(locale)) { return libc_16wctomb(buf,wc); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_32wctomb_l(char *buf, char32_t wc, locale_t UNUSED(locale)) { return libc_32wctomb(buf,wc); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_16wctomb_s_l(int *presult, char *buf, size_t buflen, char16_t wc, locale_t UNUSED(locale)) { return libc_16wctomb_s(presult,buf,buflen,wc); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_32wctomb_s_l(int *presult, char *buf, size_t buflen, char32_t wc, locale_t UNUSED(locale)) { return libc_32wctomb_s(presult,buf,buflen,wc); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_16wcstombs_s_l(size_t *presult, char *buf, size_t buflen, char16_t const *src, size_t maxlen, locale_t UNUSED(locale)) { return libc_16wcstombs_s(presult,buf,buflen,src,maxlen); }
INTERN ATTR_DOSTEXT errno_t LIBCCALL libc_32wcstombs_s_l(size_t *presult, char *buf, size_t buflen, char32_t const *src, size_t maxlen, locale_t UNUSED(locale)) { return libc_32wcstombs_s(presult,buf,buflen,src,maxlen); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_16wcstombs_l(char *dst, char16_t const *src, size_t maxlen, locale_t UNUSED(locale)) { return libc_16wcstombs(dst,src,maxlen); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_32wcstombs_l(char *dst, char32_t const *src, size_t maxlen, locale_t UNUSED(locale)) { return libc_32wcstombs(dst,src,maxlen); }
#endif

DEFINE_PUBLIC_ALIAS(_mbstrlen,libc_mbstrlen);
DEFINE_PUBLIC_ALIAS(_mbstrlen_l,libc_mbstrlen_l);
DEFINE_PUBLIC_ALIAS(_mbstrnlen,libc_mbstrnlen);
DEFINE_PUBLIC_ALIAS(_mbstrnlen_l,libc_mbstrnlen_l);
DEFINE_PUBLIC_ALIAS(mbtowc_l,libc_32mbtowc_l);
DEFINE_PUBLIC_ALIAS(_mbtowc_l,libc_16mbtowc_l);
DEFINE_PUBLIC_ALIAS(mbstowcs_s,libc_32mbstowcs_s);
DEFINE_PUBLIC_ALIAS(__DSYM(mbstowcs_s),libc_16mbstowcs_s);
DEFINE_PUBLIC_ALIAS(mbstowcs_s_l,libc_32mbstowcs_s_l);
DEFINE_PUBLIC_ALIAS(_mbstowcs_s_l,libc_16mbstowcs_s_l);
DEFINE_PUBLIC_ALIAS(mbstowcs_l,libc_32mbstowcs_l);
DEFINE_PUBLIC_ALIAS(_mbstowcs_l,libc_16mbstowcs_l);

DEFINE_PUBLIC_ALIAS(wctomb_l,libc_32wctomb_l);
DEFINE_PUBLIC_ALIAS(_wctomb_l,libc_16wctomb_l);
DEFINE_PUBLIC_ALIAS(wctomb_s,libc_32wctomb_s);
DEFINE_PUBLIC_ALIAS(__DSYM(wctomb_s),libc_16wctomb_s);
DEFINE_PUBLIC_ALIAS(wctomb_s_l,libc_32wctomb_s);
DEFINE_PUBLIC_ALIAS(_wctomb_s_l,libc_16wctomb_s_l);
DEFINE_PUBLIC_ALIAS(wcstombs_s,libc_32wcstombs_s);
DEFINE_PUBLIC_ALIAS(__DSYM(wcstombs_s),libc_16wcstombs_s);
DEFINE_PUBLIC_ALIAS(wcstombs_s_l,libc_32wcstombs_s_l);
DEFINE_PUBLIC_ALIAS(_wcstombs_s_l,libc_16wcstombs_s_l);
DEFINE_PUBLIC_ALIAS(wcstombs_l,libc_32wcstombs_l);
DEFINE_PUBLIC_ALIAS(_wcstombs_l,libc_16wcstombs_l);

#if 1
INTDEF char const dos_str0[];
INTDEF char16_t const dos_16wstr0[];
INTDEF char32_t const dos_32wstr0[];
INTDEF char const dos_str_col[];
INTDEF char16_t const dos_16wstr_col[];
INTDEF char32_t const dos_32wstr_col[];
INTDEF char const dos_str_slash[];
INTDEF char16_t const dos_16wstr_slash[];
INTDEF char32_t const dos_32wstr_slash[];
INTDEF char const dos_str_dot[];
INTDEF char16_t const dos_16wstr_dot[];
INTDEF char32_t const dos_32wstr_dot[];
GLOBAL_ASM(
L(.section .rodata.dos                                                        )
#if BYTE_ORDER == LITTLE_ENDIAN_ORDER
L(INTERN_ENTRY(dos_str_slash)                                                 )
L(INTERN_ENTRY(dos_16wstr_slash)                                              )
L(INTERN_ENTRY(dos_32wstr_slash)                                              )
L(    .byte '\\'                                                              )
L(    .byte 0x00                                                              )
L(SYM_END(dos_str_slash)                                                      )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(SYM_END(dos_16wstr_slash)                                                   )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(SYM_END(dos_32wstr_slash)                                                   )
#else
L(INTERN_ENTRY(dos_32wstr_slash)                                              )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(INTERN_ENTRY(dos_16wstr_slash)                                              )
L(    .byte 0x00                                                              )
L(INTERN_ENTRY(dos_str_slash)                                                 )
L(    .byte '\\'                                                              )
L(    .byte 0x00                                                              )
L(SYM_END(dos_str_slash)                                                      )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(SYM_END(dos_16wstr_slash)                                                   )
L(    .byte 0x00                                                              )
L(SYM_END(dos_32wstr_slash)                                                   )
#endif
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .rodata.dos                                                        )
#if BYTE_ORDER == LITTLE_ENDIAN_ORDER
L(INTERN_ENTRY(dos_str_dot)                                                   )
L(INTERN_ENTRY(dos_16wstr_dot)                                                )
L(INTERN_ENTRY(dos_32wstr_dot)                                                )
L(    .byte '.'                                                               )
L(    .byte 0x00                                                              )
L(SYM_END(dos_str_dot)                                                        )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(SYM_END(dos_16wstr_dot)                                                     )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(SYM_END(dos_32wstr_dot)                                                     )
#else
L(INTERN_ENTRY(dos_32wstr_dot)                                                )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(INTERN_ENTRY(dos_16wstr_dot)                                                )
L(    .byte 0x00                                                              )
L(INTERN_ENTRY(dos_str_dot)                                                   )
L(    .byte '.'                                                               )
L(    .byte 0x00                                                              )
L(SYM_END(dos_str_dot)                                                        )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(SYM_END(dos_16wstr_dot)                                                     )
L(    .byte 0x00                                                              )
L(SYM_END(dos_32wstr_dot)                                                     )
#endif
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .rodata.dos                                                        )
#if BYTE_ORDER == LITTLE_ENDIAN_ORDER
L(INTERN_ENTRY(dos_str_col)                                                   )
L(INTERN_ENTRY(dos_16wstr_col)                                                )
L(INTERN_ENTRY(dos_32wstr_col)                                                )
L(    .byte ':'                                                               )
L(    .byte 0x00                                                              )
L(SYM_END(dos_str_col)                                                        )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(SYM_END(dos_16wstr_col)                                                     )
#else
L(INTERN_ENTRY(dos_32wstr_col)                                                )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(INTERN_ENTRY(dos_16wstr_col)                                                )
L(    .byte 0x00                                                              )
L(INTERN_ENTRY(dos_str_col)                                                   )
L(    .byte ':'                                                               )
#endif
L(INTERN_ENTRY(dos_str0)                                                      )
L(INTERN_ENTRY(dos_16wstr0)                                                   )
L(INTERN_ENTRY(dos_32wstr0)                                                   )
L(    .byte 0x00                                                              )
#if BYTE_ORDER != LITTLE_ENDIAN_ORDER
L(SYM_END(dos_str_col)                                                        )
#endif
L(SYM_END(dos_str0)                                                           )
L(    .byte 0x00                                                              )
#if BYTE_ORDER != LITTLE_ENDIAN_ORDER
L(SYM_END(dos_16wstr_col)                                                     )
#endif
L(SYM_END(dos_16wstr0)                                                        )
L(    .byte 0x00                                                              )
L(    .byte 0x00                                                              )
L(SYM_END(dos_32wstr0)                                                        )
L(SYM_END(dos_32wstr_col)                                                     )
L(.previous                                                                   )
);
#else
INTERN ATTR_DOSRODATA char const dos_str0[] = {0};
INTERN ATTR_DOSRODATA char16_t const dos_16wstr0[] = {0};
INTERN ATTR_DOSRODATA char32_t const dos_32wstr0[] = {0};
INTERN ATTR_DOSRODATA char const dos_str_col[] = {':',0};
INTERN ATTR_DOSRODATA char16_t const dos_16wstr_col[] = {':',0};
INTERN ATTR_DOSRODATA char32_t const dos_32wstr_col[] = {':',0};
INTERN ATTR_DOSRODATA char const dos_str_slash[] = {'\\',0};
INTERN ATTR_DOSRODATA char16_t const dos_16wstr_slash[] = {'\\',0};
INTERN ATTR_DOSRODATA char32_t const dos_32wstr_slash[] = {'\\',0};
INTERN ATTR_DOSRODATA char const dos_str_dot[] = {'.',0};
INTERN ATTR_DOSRODATA char16_t const dos_16wstr_dot[] = {'.',0};
INTERN ATTR_DOSRODATA char32_t const dos_32wstr_dot[] = {'.',0};
#endif

INTERN ATTR_DOSRODATA char const dos_makepath_format[] = {'%','s','%','s','%','s','%','s','%','s','%','s','%','s',0};
INTERN ATTR_DOSRODATA char16_t const dos_16wmakepath_format[] = {'%','l','s','%','l','s','%','l','s','%','l','s','%','l','s','%','l','s','%','l','s',0};
INTERN ATTR_DOSRODATA char32_t const dos_32wmakepath_format[] = {'%','l','s','%','l','s','%','l','s','%','l','s','%','l','s','%','l','s','%','l','s',0};

INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_makepath_s(char *__restrict buf, size_t buflen, char const *drive,
                char const *dir, char const *file, char const *ext) {
 char const *dir_end; size_t reqsize;
 if (!drive) dir = dos_str0;
 if (!dir) dir = dos_str0;
 if (!file) file = dos_str0;
 if (!ext) ext = dos_str0;
 while (*ext == '.') ++ext;
 dir_end = libc_strend(dir);
 reqsize = libc_snprintf(buf,buflen,dos_makepath_format,drive,*drive ? dos_str_col : dos_str0,dir,
                         dir_end[-1] == '/' || dir_end[-1] == '\\' ? dos_str0 :
                         dos_str_slash,file,*ext ? dos_str_dot : dos_str0,ext);
 return reqsize >= buflen ? __DOS_ERANGE : EOK;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_16wmakepath_s(char16_t *__restrict buf, size_t buflen,
                   char16_t const *drive, char16_t const *dir,
                   char16_t const *file, char16_t const *ext) {
 char16_t const *dir_end; size_t reqsize;
 if (!drive) dir = dos_16wstr0;
 if (!dir) dir = dos_16wstr0;
 if (!file) file = dos_16wstr0;
 if (!ext) ext = dos_16wstr0;
 while (*ext == '.') ++ext;
 dir_end = libc_16wcsend(dir);
 reqsize = libc_dos_16swprintf(buf,buflen,dos_16wmakepath_format,drive,*drive ? dos_16wstr_col : dos_16wstr0,dir,
                               dir_end[-1] == '/' || dir_end[-1] == '\\' ? dos_16wstr0 :
                               dos_16wstr_slash,file,*ext ? dos_16wstr_dot : dos_16wstr0,ext);
 return reqsize >= buflen ? __DOS_ERANGE : EOK;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_32wmakepath_s(char32_t *__restrict buf, size_t buflen,
                   char32_t const *drive, char32_t const *dir,
                   char32_t const *file, char32_t const *ext) {
 char32_t const *dir_end; size_t reqsize;
 if (!drive) dir = dos_32wstr0;
 if (!dir) dir = dos_32wstr0;
 if (!file) file = dos_32wstr0;
 if (!ext) ext = dos_32wstr0;
 while (*ext == '.') ++ext;
 dir_end = libc_32wcsend(dir);
 reqsize = libc_32swprintf(buf,buflen,dos_32wmakepath_format,drive,*drive ? dos_32wstr_col : dos_32wstr0,dir,
                           dir_end[-1] == '/' || dir_end[-1] == '\\' ? dos_32wstr0 :
                           dos_32wstr_slash,file,*ext ? dos_32wstr_dot : dos_32wstr0,ext);
 return reqsize >= buflen ? __DOS_ERANGE : EOK;
}

INTERN ATTR_DOSTEXT void LIBCCALL
libc_makepath(char *__restrict buf, char const *drive,
              char const *dir, char const *file,
              char const *ext) {
 libc_makepath_s(buf,__DOS_MAX_PATH,drive,dir,file,ext);
}
INTERN ATTR_DOSTEXT void LIBCCALL
libc_16wmakepath(char16_t *__restrict buf,
                 char16_t const *drive, char16_t const *dir,
                 char16_t const *file, char16_t const *ext) {
 libc_16wmakepath_s(buf,__DOS_MAX_PATH,drive,dir,file,ext);
}
INTERN ATTR_DOSTEXT void LIBCCALL
libc_32wmakepath(char32_t *__restrict buf,
                 char32_t const *drive, char32_t const *dir,
                 char32_t const *file, char32_t const *ext) {
 libc_32wmakepath_s(buf,__DOS_MAX_PATH,drive,dir,file,ext);
}

DEFINE_PUBLIC_ALIAS(_makepath,libc_makepath);
DEFINE_PUBLIC_ALIAS(_makepath_s,libc_makepath_s);
DEFINE_PUBLIC_ALIAS(_wmakepath,libc_16wmakepath);
DEFINE_PUBLIC_ALIAS(wmakepath,libc_32wmakepath);
DEFINE_PUBLIC_ALIAS(_wmakepath_s,libc_16wmakepath_s);
DEFINE_PUBLIC_ALIAS(wmakepath_s,libc_32wmakepath_s);


DEFINE_PUBLIC_ALIAS(_splitpath,libc_splitpath);
DEFINE_PUBLIC_ALIAS(_wsplitpath,libc_16wsplitpath);
DEFINE_PUBLIC_ALIAS(wsplitpath,libc_32wsplitpath);
DEFINE_PUBLIC_ALIAS(_splitpath_s,libc_splitpath_s);
DEFINE_PUBLIC_ALIAS(_wsplitpath_s,libc_16wsplitpath_s);
DEFINE_PUBLIC_ALIAS(wsplitpath_s,libc_32wsplitpath_s);

INTERN ATTR_DOSTEXT int LIBCCALL libc_atoflt(float *__restrict result, char const *__restrict nptr) { *result = libc_strtof(nptr,NULL); return 0; }
INTERN ATTR_DOSTEXT int LIBCCALL libc_atodbl(double *__restrict result, char const *__restrict nptr) { *result = libc_atof(nptr); return 0; }
INTERN ATTR_DOSTEXT int LIBCCALL libc_atoldbl(long double *__restrict result, char const *__restrict nptr) { *result = libc_strtold(nptr,NULL); return 0; }
#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_atoflt_l,libc_atoflt);
DEFINE_INTERN_ALIAS(libc_atodbl_l,libc_atodbl);
DEFINE_INTERN_ALIAS(libc_atoldbl_l,libc_atoldbl);
#else
INTERN ATTR_DOSTEXT int LIBCCALL libc_atoflt_l(float *__restrict result, char const *__restrict nptr, locale_t UNUSED(locale)) { return libc_atoflt(result,nptr); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_atodbl_l(double *__restrict result, char const *__restrict nptr, locale_t UNUSED(locale)) { return libc_atodbl(result,nptr); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_atoldbl_l(long double *__restrict result, char const *__restrict nptr, locale_t UNUSED(locale)) { return libc_atoldbl(result,nptr); }
#endif
DEFINE_PUBLIC_ALIAS(_atoflt,libc_atoflt);
DEFINE_PUBLIC_ALIAS(_atodbl,libc_atodbl);
DEFINE_PUBLIC_ALIAS(_atoflt_l,libc_atoflt_l);
DEFINE_PUBLIC_ALIAS(_atodbl_l,libc_atodbl_l);
DEFINE_PUBLIC_ALIAS(_atoldbl,libc_atoldbl);
DEFINE_PUBLIC_ALIAS(_atoldbl_l,libc_atoldbl_l);
#ifdef CONFIG_PE_LDOUBLE_IS_DOUBLE
DEFINE_PUBLIC_ALIAS(__DSYM(_atoldbl),libc_atodbl);
DEFINE_PUBLIC_ALIAS(__DSYM(_atoldbl_l),libc_atodbl_l);
#endif /* CONFIG_PE_LDOUBLE_IS_DOUBLE */

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */


/* Wide-string API */
#ifndef __KERNEL__
INTERN size_t LIBCCALL libc___ctype_get_mb_cur_max(void) { return UNICODE_MB_MAX; }
INTERN wint_t LIBCCALL libc_btowc(int c) { return c < 192 ? (wint_t)c : (wint_t)EOF; }
INTERN int LIBCCALL libc_wctob(wint_t c) { return c < 192 ? (int)c : (int)WEOF; }
INTERN int LIBCCALL libc_mbsinit(struct __mbstate const *ps) { return (!ps || !libc_memchr(ps,0,sizeof(mbstate_t))); }
DEFINE_PUBLIC_ALIAS(__ctype_get_mb_cur_max,libc___ctype_get_mb_cur_max);
DEFINE_PUBLIC_ALIAS(btowc,libc_btowc);
DEFINE_PUBLIC_ALIAS(wctob,libc_wctob);
DEFINE_PUBLIC_ALIAS(mbsinit,libc_mbsinit);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
PRIVATE ATTR_DOSDATA int dos_mb_cur_max = UNICODE_MB_MAX;
INTERN ATTR_DOSTEXT int *LIBCCALL libc_p_mb_cur_max(void) { return &dos_mb_cur_max; }
DEFINE_PUBLIC_ALIAS(__mb_cur_max,dos_mb_cur_max);
DEFINE_PUBLIC_ALIAS(___mb_cur_max_func,libc___ctype_get_mb_cur_max);
DEFINE_PUBLIC_ALIAS(__p___mb_cur_max,libc_p_mb_cur_max);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

INTERN size_t LIBCCALL libc_mbrtoc16(char16_t *__restrict pc16, char const *__restrict s,
                                     size_t n, struct __mbstate *__restrict p) {
 size_t result; char const *start = s;
 if (!s) { mbstate_reset(p); return 0; }
 result = libc_utf8to16((char const *)&s,(size_t)&n,pc16,pc16 ? 1 : 0,p,
                         UNICODE_F_STOPONNUL|UNICODE_F_DOSINGLE|
                         UNICODE_F_NOZEROTERM|UNICODE_F_UTF16HALF|
                         UNICODE_F_UPDATESRC|UNICODE_F_SETERRNO);
 /* Forward error/signal codes as required. */
 if (result == UNICODE_ERROR || result == UNICODE_UTF16HALF) return result;
 assert(result <= 1);
 if (p->__count) {
  /* Return -2 if the given input sequence is incomplete. */
  p->__count = 0;
  return (size_t)-2;
 }
 /* Otherwise, return the amount of used source-characters. */
 return (size_t)(s-start);
}
INTERN size_t LIBCCALL libc_mbrtoc32(char32_t *__restrict pc32, char const *__restrict s,
                                     size_t n, struct __mbstate *__restrict p) {
 size_t result; char const *start = s;
 if (!s) { mbstate_reset(p); return 0; }
 result = libc_utf8to32((char const *)&s,(size_t)&n,pc32,UNICODE_MB_MAX,p,
                         UNICODE_F_STOPONNUL|UNICODE_F_DOSINGLE|
                         UNICODE_F_NOZEROTERM|UNICODE_F_UPDATESRC|
                         UNICODE_F_SETERRNO);
 /* NOTE: STDC says we should also return -3 for half-characters.
  *       But those don't actually exist... */
 if (result == UNICODE_ERROR) return result;
 assert(result <= 1);
 if (p->__count) {
  /* Return -2 if the given input sequence is incomplete. */
  p->__count = 0;
  return (size_t)-2;
 }
 /* Otherwise, return the amount of used source-characters. */
 return (size_t)(s-start);
}

INTERN size_t LIBCCALL libc_c16rtomb(char *__restrict s, char16_t c16,
                                     struct __mbstate *__restrict ps) {
 if (!s) { mbstate_reset(ps); return 0; }
 return libc_utf16to8(&c16,1,s,UNICODE_MB_MAX,ps,
                      UNICODE_F_SETERRNO|UNICODE_F_STOPONNUL|
                      UNICODE_F_NOZEROTERM);
}
INTERN size_t LIBCCALL libc_c32rtomb(char *__restrict s, char32_t c32,
                                     struct __mbstate *__restrict ps) {
 if (!s) { mbstate_reset(ps); return 0; }
 return libc_utf32to8(&c32,1,s,UNICODE_MB_MAX,ps,
                      UNICODE_F_SETERRNO|UNICODE_F_STOPONNUL|
                      UNICODE_F_NOZEROTERM);
}
DEFINE_PUBLIC_ALIAS(mbrtoc16,libc_mbrtoc16);
DEFINE_PUBLIC_ALIAS(mbrtoc32,libc_mbrtoc32);
DEFINE_PUBLIC_ALIAS(c16rtomb,libc_c16rtomb);
DEFINE_PUBLIC_ALIAS(c32rtomb,libc_c32rtomb);

#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_STRING_C */
