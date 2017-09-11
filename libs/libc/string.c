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
#include <alloca.h>
#include <assert.h>
#include <bits/signum.h>
#include <ctype.h>
#include <errno.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/atomic.h>
#include <hybrid/byteorder.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/debuginfo.h>
#include <hybrid/limits.h>
#include <hybrid/swap.h>
#include <hybrid/types.h>
#include <kos/ksym.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#ifndef __KERNEL__
#include "system.h"
#endif

DECL_BEGIN

#define DECL PUBLIC
#ifdef CONFIG_64BIT_STRING
#define BITS 64
#include "templates/string.code"
#endif
#define BITS 32
#include "templates/string.code"
#define BITS 16
#include "templates/string.code"
#define BITS 8
#include "templates/string.code"
#include <hybrid/section.h>
#undef DECL

PUBLIC void *(LIBCCALL memmove)(void *dst, void const *src, size_t n_bytes) {
 byte_t *iter,*end; byte_t const *siter;
 CHECK_HOST_DATA(dst,n_bytes);
 CHECK_HOST_TEXT(src,n_bytes);
 if (dst < src) {
  siter = (byte_t const *)src;
  end = (iter = (byte_t *)dst)+n_bytes;
  while (iter != end) *iter++ = *siter++;
 } else {
  siter = (byte_t const *)src+n_bytes;
  iter = (end = (byte_t *)dst)+n_bytes;
  while (iter != end) *--iter = *--siter;
 }
 return dst;
}


PUBLIC char *(LIBCCALL strend)(char const *__restrict str) {
 CHECK_HOST_TEXT(str,1);
 for (;;) {
  if (!*str) break;
  ++str;
#ifdef CONFIG_DEBUG
  /* Re-validate the first pointer of the next page. */
  if (!((uintptr_t)str & (PAGESIZE-1)))
         CHECK_HOST_TEXT(str,1);
#endif
 }
 return (char *)str;
}
PUBLIC char *(LIBCCALL strnend)(char const *__restrict str, size_t maxlen) {
 char *end = (char *)str+maxlen;
#ifdef CONFIG_DEBUG
 if (maxlen) {
  CHECK_HOST_TEXT(str,1);
  for (;;) {
   if (str == end || !*str) break;
   ++str;
   /* Re-validate the first pointer of the next page. */
   if (!((uintptr_t)str & (PAGESIZE-1)))
          CHECK_HOST_TEXT(str,1);
  }
 }
#else
 for (;;) {
  if (str == end || !*str) break;
  ++str;
 }
#endif
 return (char *)str;
}
PUBLIC size_t (LIBCCALL strlen)(char const *__restrict str) {
 return (size_t)(strend(str)-str);
}
PUBLIC size_t (LIBCCALL strnlen)(char const *__restrict str, size_t maxlen) {
 return (size_t)(strnend(str,maxlen)-str);
}

PUBLIC char *(LIBCCALL strchrnul)(char const *__restrict haystack, int needle) {
 char *iter = (char *)haystack;
 CHECK_HOST_TEXT(iter,1);
 while (*iter && *iter != needle) {
  ++iter;
#ifdef CONFIG_DEBUG
  if (!((uintptr_t)iter & (PAGESIZE-1)))
         CHECK_HOST_TEXT(iter,1);
#endif
 }
 return iter;
}
PUBLIC char *(LIBCCALL strchr)(char const *__restrict haystack, int needle) {
 char *iter = (char *)haystack;
 CHECK_HOST_TEXT(iter,1);
 while (*iter) {
  if (*iter == needle) return iter;
  ++iter;
#ifdef CONFIG_DEBUG
  if (!((uintptr_t)iter & (PAGESIZE-1)))
         CHECK_HOST_TEXT(iter,1);
#endif
 }
 return NULL;
}
PUBLIC char *(LIBCCALL strrchr)(char const *__restrict haystack, int needle) {
 char *iter = (char *)haystack;
 char *result = NULL;
 CHECK_HOST_TEXT(iter,1);
 while (*iter) {
  if (*iter == needle) result = iter;
  ++iter;
#ifdef CONFIG_DEBUG
  if (!((uintptr_t)iter & (PAGESIZE-1)))
         CHECK_HOST_TEXT(iter,1);
#endif
 }
 return result;
}
PUBLIC char *(LIBCCALL strrchrnul)(char const *__restrict haystack, int needle) {
 char *iter = (char *)haystack;
 char *result = NULL;
 CHECK_HOST_TEXT(iter,1);
 while (*iter) {
  if (*iter == needle) result = iter;
  ++iter;
#ifdef CONFIG_DEBUG
  if (!((uintptr_t)iter & (PAGESIZE-1)))
         CHECK_HOST_TEXT(iter,1);
#endif
 }
 return result ? result : (char *)haystack-1;
}
PUBLIC char *(LIBCCALL strnchr)(char const *__restrict haystack,
                                int needle, size_t max_chars) {
 char *iter = (char *)haystack;
 char *end = iter+max_chars;
 if (iter != end) CHECK_HOST_TEXT(iter,1);
 while (iter != end && *iter) {
  if (*iter == needle) return iter;
  ++iter;
#ifdef CONFIG_DEBUG
  if (!((uintptr_t)iter & (PAGESIZE-1)))
         CHECK_HOST_TEXT(iter,1);
#endif
 }
 return NULL;
}
PUBLIC char *(LIBCCALL strnrchr)(char const *__restrict haystack,
                                 int needle, size_t max_chars) {
 char *iter = (char *)haystack;
 char *end = iter+max_chars;
 char *result = NULL;
 if (iter != end) CHECK_HOST_TEXT(iter,1);
 while (iter != end && *iter) {
  if (*iter == needle) result = iter;
  ++iter;
#ifdef CONFIG_DEBUG
  if (!((uintptr_t)iter & (PAGESIZE-1)))
         CHECK_HOST_TEXT(iter,1);
#endif
 }
 return result;
}
PUBLIC char *(LIBCCALL strnchrnul)(char const *__restrict haystack,
                                   int needle, size_t max_chars) {
 char *iter = (char *)haystack;
 char *end = iter+max_chars;
 if (iter != end) CHECK_HOST_TEXT(iter,1);
 while (iter != end && *iter && *iter != needle) {
  ++iter;
#ifdef CONFIG_DEBUG
  if (!((uintptr_t)iter & (PAGESIZE-1)))
         CHECK_HOST_TEXT(iter,1);
#endif
 }
 return iter;
}
PUBLIC char *(LIBCCALL strnrchrnul)(char const *__restrict haystack,
                                    int needle, size_t max_chars) {
 char *iter = (char *)haystack;
 char *end = iter+max_chars;
 char *result = NULL;
 if (iter != end) CHECK_HOST_TEXT(iter,1);
 while (iter != end && *iter) {
  if (*iter == needle) result = iter;
  ++iter;
#ifdef CONFIG_DEBUG
  if (!((uintptr_t)iter & (PAGESIZE-1)))
         CHECK_HOST_TEXT(iter,1);
#endif
 }
 return result ? result : (char *)haystack-1;
}
PUBLIC size_t (LIBCCALL stroff)(char const *__restrict haystack, int needle) { return strchrnul(haystack,needle)-haystack; }
PUBLIC size_t (LIBCCALL strroff)(char const *__restrict haystack, int needle) { return strrchrnul(haystack,needle)-haystack; }
PUBLIC size_t (LIBCCALL strnoff)(char const *__restrict haystack, int needle, size_t max_chars) { return strnchrnul(haystack,needle,max_chars)-haystack; }
PUBLIC size_t (LIBCCALL strnroff)(char const *__restrict haystack, int needle, size_t max_chars) { return strnrchrnul(haystack,needle,max_chars)-haystack; }


PUBLIC char *(LIBCCALL stpcpy)(char *__restrict dst,
                               char const *__restrict src) {
 size_t len = strlen(src);
 memcpy(dst,src,(len+1)*sizeof(char));
 return dst+len;
}
PUBLIC char *(LIBCCALL stpncpy)(char *__restrict dst,
                                char const *__restrict src, size_t n) {
 size_t len = strnlen(src,n);
 memcpy(dst,src,(len+!src[len])*sizeof(char));
 return dst+len;
}

PUBLIC int (LIBCCALL strcmp)(char const *s1, char const *s2) {
 char a,b; int result;
 do a = *s1++,b = *s2++,
    result = (int)a-(int)b;
 while (result == 0 && a);
 return result;
}
PUBLIC int (LIBCCALL strncmp)(char const *s1, char const *s2, size_t n) {
 char a,b,*end = (char *)s1+n; int result;
 if (n) do a = *s1++,b = *s2++,
           result = (int)a-(int)b;
 while (result == 0 && a && s1 != end);
 return result;
}
PUBLIC int (LIBCCALL strcasecmp)(char const *s1, char const *s2) {
 char a,b; int result;
 do a = tolower(*s1++),
    b = tolower(*s2++),
    result = (int)a-(int)b;
 while (result == 0 && a);
 return result;
}
PUBLIC int (LIBCCALL strncasecmp)(char const *s1, char const *s2, size_t n) {
 char a,b,*end = (char *)s1+n; int result;
 if (n) do a = tolower(*s1++),
           b = tolower(*s2++),
           result = (int)a-(int)b;
 while (result == 0 && a && s1 != end);
 return result;
}

PUBLIC void *(LIBCCALL mempcpy)(void *__restrict dst,
                                void const *__restrict src, size_t n) {
 return (void *)((uintptr_t)memcpy(dst,src,n)+n);
}
PUBLIC char *(LIBCCALL strstr)(char const *haystack,
                               char const *needle) {
 char *hay_iter,*hay2; char const *ned_iter; char ch,needle_start;
 assert(haystack);
 assert(needle);
 hay_iter = (char *)haystack;
 needle_start = *needle++;
 while ((CHECK_HOST_TEXT(hay_iter,1),ch = *hay_iter++) != '\0') {
  if (ch == needle_start) {
   hay2 = hay_iter,ned_iter = needle;
   while ((CHECK_HOST_TEXT(ned_iter,1),ch = *ned_iter++) != '\0') {
    if (*hay2++ != ch) goto miss;
   }
   return hay_iter-1;
  }
miss:;
 }
 return NULL;
}

PUBLIC char *(LIBCCALL strcasestr)(char const *haystack,
                                   char const *needle) {
 char *hay_iter,*hay2; char const *ned_iter; char ch,needle_start;
 assert(haystack);
 assert(needle);
 hay_iter = (char *)haystack;
 needle_start = *needle++;
 while ((CHECK_HOST_TEXT(hay_iter,1),ch = *hay_iter++) != '\0') {
  if (ch == needle_start) {
   hay2 = hay_iter,ned_iter = needle;
   while ((CHECK_HOST_TEXT(ned_iter,1),ch = *ned_iter++) != '\0') {
    if (tolower(*hay2++) != tolower(ch)) goto miss;
   }
   return hay_iter-1;
  }
miss:;
 }
 return NULL;
}

PUBLIC void *(LIBCCALL memmem)(void const *haystack, size_t haystacklen,
                               void const *needle, size_t needlelen) {
 byte_t *iter,*end;
 if unlikely(needlelen > haystacklen) return NULL;
 end = (iter = (byte_t *)haystack)+(haystacklen-needlelen);
 for (;;) {
  if (memcmp(iter,needle,needlelen) == 0)
      return iter;
  if (iter == end) break;
  ++iter;
 }
 return NULL;
}


#define NOCASE
#include "templates/fuzzy_memcmp.code"
#include "templates/fuzzy_memcmp.code"

PUBLIC size_t (LIBCCALL fuzzy_strcmp)(char const *a, char const *b) {
 return fuzzy_memcmp(a,strlen(a)*sizeof(char),
                     b,strlen(b)*sizeof(char));
}
PUBLIC size_t (LIBCCALL fuzzy_strncmp)(char const *a, size_t max_a_chars,
                                       char const *b, size_t max_b_chars) {
 return fuzzy_memcmp(a,strnlen(a,max_a_chars)*sizeof(char),
                     b,strnlen(b,max_b_chars)*sizeof(char));
}
PUBLIC size_t (LIBCCALL fuzzy_strcasecmp)(char const *a, char const *b) {
 return fuzzy_memcasecmp(a,strlen(a)*sizeof(char),
                         b,strlen(b)*sizeof(char));
}
PUBLIC size_t (LIBCCALL fuzzy_strncasecmp)(char const *a, size_t max_a_chars,
                                           char const *b, size_t max_b_chars) {
 return fuzzy_memcasecmp(a,strnlen(a,max_a_chars)*sizeof(char),
                         b,strnlen(b,max_b_chars)*sizeof(char));
}

#define DO_FFS(i) \
{ int result; \
  if (!i) return 0; \
  for (result = 1; !(i&1); ++result) i >>= 1; \
  return result; \
}
PUBLIC int (LIBCCALL __ffs8)(s8 i) { DO_FFS(i) }
PUBLIC int (LIBCCALL __ffs16)(s16 i) { DO_FFS(i) }
PUBLIC int (LIBCCALL __ffs32)(s32 i) { DO_FFS(i) }
PUBLIC int (LIBCCALL __ffs64)(s64 i) { DO_FFS(i) }
#undef DO_FFS

#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(ffs,PP_CAT2(__ffs,PP_MUL8(__SIZEOF_INT__)));
DEFINE_PUBLIC_ALIAS(ffsl,PP_CAT2(__ffs,PP_MUL8(__SIZEOF_LONG__)));
DEFINE_PUBLIC_ALIAS(ffsll,PP_CAT2(__ffs,PP_MUL8(__SIZEOF_LONG_LONG__)));
#endif /* !__KERNEL__ */



/* TODO */PUBLIC int (LIBCCALL strverscmp)(char const *s1, char const *s2) { NOT_IMPLEMENTED(); return 0; }
/* TODO */PUBLIC char *(LIBCCALL strsep)(char **__restrict stringp, char const *__restrict delim) { NOT_IMPLEMENTED(); return NULL; }


/* String functions deemed to unimportant to include in the kernel core. (libk) */
#ifndef __KERNEL__
PUBLIC void (LIBCCALL bcopy)(void const *src, void *dst, size_t n) { memmove(dst,src,n); }
PUBLIC void (LIBCCALL bzero)(void *s, size_t n) { memset(s,0,n); }
PUBLIC char *(LIBCCALL strcpy)(char *__restrict dst, char const *__restrict src) { return (char *)memcpy(dst,src,(strlen(src)+1)*sizeof(char)); }
PUBLIC char *(LIBCCALL strncpy)(char *__restrict dst, char const *__restrict src, size_t n) { return (char *)memcpy(dst,src,(strnlen(src,n)+1)*sizeof(char)); }

PUBLIC char *(LIBCCALL index)(char const *__restrict haystack, int needle) {
 char *iter = (char *)haystack;
 CHECK_HOST_TEXT(iter,1);
 while (*iter != needle) {
  if (!*iter) return NULL;
  ++iter;
#ifdef CONFIG_DEBUG
  if (!((uintptr_t)iter & (PAGESIZE-1)))
         CHECK_HOST_TEXT(iter,1);
#endif
 }
 return iter;
}
PUBLIC char *(LIBCCALL rindex)(char const *__restrict haystack, int needle) {
 char *iter = (char *)haystack;
 char *result = NULL;
 CHECK_HOST_TEXT(iter,1);
 for (;;) {
  if (*iter == needle) result = iter;
  if (!*iter) break;
  ++iter;
#ifdef CONFIG_DEBUG
  if (!((uintptr_t)iter & (PAGESIZE-1)))
         CHECK_HOST_TEXT(iter,1);
#endif
 }
 return result;
}

/* TODO */PUBLIC char *(LIBCCALL dirname)(char *path) { NOT_IMPLEMENTED(); return NULL; }
/* TODO */PUBLIC char *(LIBCCALL __xpg_basename)(char *path) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC char *(LIBCCALL basename)(char const *__restrict path) {
 char ch,*iter = (char *)path,*result = NULL;
 if (!path || !*path) return ".";
 do if ((ch = *iter++) == '/') result = iter;
 while (ch);
 if unlikely(!result) return (char *)path; /* Path doesn't contain '/'. */
 if (*result) return result; /* Last character isn't a '/'. */
 iter = result;
 while (iter != path && iter[-1] == '/') --iter;
 if (iter == path) return result-1; /* Only '/'-characters. */
 *iter = '\0'; /* Trim all ending '/'-characters. */
 while (iter != path && iter[-1] != '/') --iter; /* Scan until the previous '/'. */
 return iter; /* Returns string after previous '/'. */
}
PUBLIC char *(LIBCCALL strcat)(char *__restrict dst, char const *__restrict src) { memcpy(strend(dst),src,(strlen(src)+1)*sizeof(char)); return dst; }
PUBLIC char *(LIBCCALL strncat)(char *__restrict dst, char const *__restrict src, size_t n) {
 size_t maxlen = strnlen(src,n);
 char *target = strend(dst);
 memcpy(target,src,maxlen*sizeof(char));
 if (maxlen < n) target[maxlen] = '\0';
 return dst;
}
PUBLIC size_t(LIBCCALL strcspn)(char const *s, char const *reject) {
 char const *iter = s;
 while (*iter && !strchr(reject,*iter)) ++iter;
 return (size_t)(iter-s);
}
PUBLIC char *(LIBCCALL strpbrk)(char const *s, char const *accept) {
 char *hay_iter = (char *)s;
 char const *ned_iter; char haych,ch;
 while ((haych = *hay_iter++) != '\0') {
  ned_iter = accept;
  while ((ch = *ned_iter++) != '\0') {
   if (haych == ch) return hay_iter-1;
  }
 }
 return NULL;
}
PUBLIC char *(LIBCCALL strtok_r)(char *__restrict s, char const *__restrict delim,
                                 char **__restrict save_ptr) {
 char *end;
 if (!s) s = *save_ptr;
 if (!*s) { *save_ptr = s; return NULL; }
 s += strspn(s,delim);
 if (!*s) { *save_ptr = s; return NULL; }
 end = s+strcspn(s,delim);
 if (!*end) { *save_ptr = end; return s; }
 *end = '\0';
 *save_ptr = end+1;
 return s;
}
PUBLIC char *(LIBCCALL strtok)(char *__restrict s, char const *__restrict delim) {
 PRIVATE char *safe = NULL;
 return strtok_r(s,delim,&safe);
}
PUBLIC size_t (LIBCCALL strspn)(char const *s, char const *accept) {
 char const *iter = s;
 while (strchr(accept,*iter)) ++iter;
 return (size_t)(iter-s);
}
PUBLIC void *(LIBCCALL memccpy)(void *__restrict dst,
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

/* TODO: Kernel-share data? */
PRIVATE ATTR_RARERODATA char const signal_names[][10] = {
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
PUBLIC char *(LIBCCALL strsignal)(int sig) {
 PRIVATE char buffer[20];
 if (sig < 1 || (unsigned)sig >= COMPILER_LENOF(signal_names))
  sprintf(buffer,"unknown(%d)",sig);
 else {
  strcpy(buffer,signal_names[sig-1]);
 }
 return buffer;
}


/* TODO */PUBLIC int (LIBCCALL strcoll)(char const *s1, char const *s2) { NOT_IMPLEMENTED(); return 0; }
/* TODO */PUBLIC size_t (LIBCCALL strxfrm)(char *__restrict dst, char const *__restrict src, size_t n) { NOT_IMPLEMENTED(); return 0; }
/* TODO */PUBLIC char *(LIBCCALL strfry)(char *string) { NOT_IMPLEMENTED(); return NULL; }
/* TODO */PUBLIC void *(LIBCCALL memfrob)(void *s, size_t n) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL strcoll_l)(char const *s1, char const *s2, locale_t l) { NOT_IMPLEMENTED(); return strcoll(s1,s2); }
PUBLIC size_t (LIBCCALL strxfrm_l)(char *dst, char const *src, size_t n, locale_t l) { NOT_IMPLEMENTED(); return strxfrm(dst,src,n); }
PUBLIC char *(LIBCCALL strerror_l)(int errnum, locale_t l) { NOT_IMPLEMENTED(); return strerror(errnum); }
PUBLIC int (LIBCCALL strcasecmp_l)(char const *s1, char const *s2, locale_t loc) { NOT_IMPLEMENTED(); return strcasecmp(s1,s2); }
PUBLIC int (LIBCCALL strncasecmp_l)(char const *s1, char const *s2, size_t n, locale_t loc) { NOT_IMPLEMENTED(); return strncasecmp(s1,s2,n); }
/* TODO */PUBLIC char *(ATTR_CDECL strdupaf)(char const *__restrict format, ...) { NOT_IMPLEMENTED(); return NULL; }
/* TODO */PUBLIC char *(LIBCCALL vstrdupaf)(char const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return NULL; }
#endif /* !__KERNEL__ */


/* String function aliases */
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(__stpcpy,stpcpy);
DEFINE_PUBLIC_ALIAS(__stpncpy,stpncpy);
DEFINE_PUBLIC_ALIAS(__mempcpy,mempcpy);
DEFINE_PUBLIC_ALIAS(memcpyb,memcpy);
DEFINE_PUBLIC_ALIAS(memsetb,memset);
DEFINE_PUBLIC_ALIAS(memcmpb,memcmp);
DEFINE_PUBLIC_ALIAS(mempatb,memset);
DEFINE_PUBLIC_ALIAS(memlenb,memlen);
DEFINE_PUBLIC_ALIAS(memrlenb,memrlen);
DEFINE_PUBLIC_ALIAS(rawmemlenb,rawmemlen);
DEFINE_PUBLIC_ALIAS(rawmemrlenb,rawmemrlen);
DEFINE_PUBLIC_ALIAS(memchrb,memchr);
DEFINE_PUBLIC_ALIAS(memrchrb,memrchr);
DEFINE_PUBLIC_ALIAS(memendb,memend);
DEFINE_PUBLIC_ALIAS(memrendb,memrend);
DEFINE_PUBLIC_ALIAS(rawmemchrb,rawmemchr);
DEFINE_PUBLIC_ALIAS(rawmemrchrb,rawmemrchr);
DEFINE_PUBLIC_ALIAS(_memcpyb_d,_memcpy_d);
DEFINE_PUBLIC_ALIAS(bcmp,memcmp);
DEFINE_PUBLIC_ALIAS(__bzero,bzero);
DEFINE_PUBLIC_ALIAS(__strtok_r,strtok_r);
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

#ifdef __KERNEL__
#define LIBC_ERRNOTEXT_LAZY_LINK 0
#else
#define LIBC_ERRNOTEXT_LAZY_LINK 1
#endif


#if LIBC_ERRNOTEXT_LAZY_LINK
PRIVATE struct errnotext_data const *p_errnotext = NULL;
#else
DATDEF struct errnotext_data const errnotext __KSYM(errnotext);
#endif

PRIVATE char const *LIBCCALL strerror_get(uintptr_t kind, int no) {
 struct errnotext_data const *data;
 struct errnotext_entry const *entry; char const *string;
#if LIBC_ERRNOTEXT_LAZY_LINK
 if ((data = p_errnotext) == NULL) {
  data = (struct errnotext_data *)sys_xsharesym("errnotext");
  if (data) ATOMIC_WRITE(p_errnotext,data);
 }
#else
 data = &errnotext;
#endif
#ifndef __KERNEL__
 if unlikely(!data || E_ISERR(data)) goto unknown; /* The kernel doesn't define this information. */
#endif
 if unlikely(no < 0 || (size_t)no >= data->etd_enocnt) goto unknown;
 entry   = (struct errnotext_entry const *)((uintptr_t)data+data->etd_enotab+(size_t)no*data->etd_enoent);
 string  = (char const *)((uintptr_t)data+data->etd_strtab);
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

PUBLIC char const *(LIBCCALL strerror_s)(int errnum) { return strerror_get(ERRNOSTR_TEXT,errnum); }
PUBLIC char const *(LIBCCALL strerrorname_s)(int errnum) { return strerror_get(ERRNOSTR_NAME,errnum); }


#ifdef __KERNEL__
GLOBAL_ASM(
#include "strerror.inl"
);
#else /* __KERNEL__ */
PRIVATE char strerror_buf[64];
PUBLIC char *(LIBCCALL strerror)(int errnum) {
 char const *string = strerror_get(ERRNOSTR_TEXT,errnum);
 if (string) {
  /* Copy the descriptor text. */
  strerror_buf[COMPILER_LENOF(strerror_buf)-1] = '\0';
  strncpy(strerror_buf,string,COMPILER_LENOF(strerror_buf)-1);
 } else {
  sprintf(strerror_buf,"Unknown error %d",errnum);
 }
 return strerror_buf;
}
PUBLIC int (LIBCCALL __xpg_strerror_r)(int errnum, char *buf,
                                       size_t buflen) {
 size_t msg_len; char const *string;
 string = strerror_get(ERRNOSTR_TEXT,errnum);
 if (!buf) buflen = 0;
 if (!string) return EINVAL;
 /* Copy the descriptor text. */
 msg_len = (strlen(string)+1)*sizeof(char);
 if (msg_len > buflen) return ERANGE;
 memcpy(buf,string,msg_len);
 return 0;
}
PUBLIC char *(LIBCCALL strerror_r)(int errnum, char *buf, size_t buflen) {
 char const *string = strerror_get(ERRNOSTR_TEXT,errnum);
 if (!buf || !buflen) {
  buf    = strerror_buf;
  buflen = sizeof(strerror_buf);
 }
 if (string) {
  /* Copy the descriptor text. */
  size_t msg_len = (strlen(string)+1)*sizeof(char);
  if (msg_len > buflen) {
   buf    = strerror_buf;
   buflen = sizeof(strerror_buf);
   if unlikely(msg_len > buflen) {
    msg_len = buflen-1;
    buf[msg_len] = '\0';
   }
  }
  memcpy(buf,string,msg_len);
 } else {
again_unknown:
  if (snprintf(buf,buflen,"Unknown error %d",errnum) >= buflen) {
   assert(buf != strerror_buf);
   buf    = strerror_buf;
   buflen = sizeof(strerror_buf);
   goto again_unknown;
  }
 }
 return buf;
}

#ifndef CONFIG_LIBC_NO_DOS_EXTENSIONS
#ifndef CONFIG_LIBC_NO_DOS_ALIASES
/* Define aliases for binary compatibility. */
DEFINE_PUBLIC_ALIAS(_strdup,strdup);
DEFINE_PUBLIC_ALIAS(_strlwr,strlwr);
DEFINE_PUBLIC_ALIAS(_strlwr_l,strlwr_l);
DEFINE_PUBLIC_ALIAS(_strnset,strnset);
DEFINE_PUBLIC_ALIAS(_strrev,strrev);
DEFINE_PUBLIC_ALIAS(_strset,strset);
DEFINE_PUBLIC_ALIAS(_strupr,strupr);
DEFINE_PUBLIC_ALIAS(_strupr_l,strupr_l);
DEFINE_PUBLIC_ALIAS(_memicmp,memcasecmp);
DEFINE_PUBLIC_ALIAS(_memicmp_l,memcasecmp_l);
DEFINE_PUBLIC_ALIAS(_strcmpi,strcasecmp);
DEFINE_PUBLIC_ALIAS(_strcoll_l,strcoll_l);
DEFINE_PUBLIC_ALIAS(_stricmp,strcasecmp);
DEFINE_PUBLIC_ALIAS(_stricmp_l,strcasecmp_l);
DEFINE_PUBLIC_ALIAS(_stricoll,strcasecoll);
DEFINE_PUBLIC_ALIAS(_stricoll_l,strcasecoll_l);
DEFINE_PUBLIC_ALIAS(_strncoll,strncoll);
DEFINE_PUBLIC_ALIAS(_strncoll_l,strncoll_l);
DEFINE_PUBLIC_ALIAS(_strnicmp,strncasecmp);
DEFINE_PUBLIC_ALIAS(_strnicmp_l,strncasecmp_l);
DEFINE_PUBLIC_ALIAS(_strnicoll,strncasecoll);
DEFINE_PUBLIC_ALIAS(_strnicoll_l,strncasecoll_l);
DEFINE_PUBLIC_ALIAS(memicmp,memcasecmp);
DEFINE_PUBLIC_ALIAS(strcmpi,strcasecmp);
DEFINE_PUBLIC_ALIAS(stricmp,strcasecmp);
DEFINE_PUBLIC_ALIAS(strnicmp,strncasecmp);
DEFINE_PUBLIC_ALIAS(_strxfrm_l,strxfrm_l);
DEFINE_PUBLIC_ALIAS(_memccpy,memccpy);
#ifndef CONFIG_LIBC_NO_WCHAR_STRING
DEFINE_PUBLIC_ALIAS(__DSYM(_wcscoll_l),__DSYM(wcscoll_l));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsicmp),__DSYM(wcscasecmp));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsicmp_l),__DSYM(wcscasecmp_l));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsicoll),__DSYM(wcscasecoll));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsicoll_l),__DSYM(wcscasecoll_l));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsncoll),__DSYM(wcsncoll));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsncoll_l),__DSYM(wcsncoll_l));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsnicmp),__DSYM(wcsncasecmp));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsnicmp_l),__DSYM(wcsncasecmp_l));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsnicoll),__DSYM(wcsncasecoll));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsnicoll_l),__DSYM(wcsncasecoll_l));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsxfrm_l),__DSYM(wcsxfrm_l));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsdup),__DSYM(wcsdup));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcslwr),__DSYM(wcslwr));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcslwr_l),__DSYM(wcslwr_l));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsnset),__DSYM(wcsnset));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsrev),__DSYM(wcsrev));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsset),__DSYM(wcsset));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsupr),__DSYM(wcsupr));
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsupr_l),__DSYM(wcsupr_l));
DEFINE_PUBLIC_ALIAS(__DSYM(wcswcs),__DSYM(wcsstr));
#endif /* !CONFIG_LIBC_NO_WCHAR_STRING */
#endif /* !CONFIG_LIBC_NO_DOS_ALIASES */

PUBLIC char *(LIBCCALL strlwr)(char *str) { char *result = str; for (; *str; ++str) *str = tolower(*str); return result; }
PUBLIC char *(LIBCCALL strupr)(char *str) { char *result = str; for (; *str; ++str) *str = toupper(*str); return result; }
PUBLIC char *(LIBCCALL strset)(char *str, int chr) { char *result = str; while (*str) *str++ = (char)chr; return result; }
PUBLIC char *(LIBCCALL strnset)(char *str, int chr, size_t max_chars) { char *result = str,*end = str+max_chars; while (str != end && *str) *str++ = (char)chr; return result; }
PUBLIC char *(LIBCCALL strrev)(char *str) { NOT_IMPLEMENTED(); return str; }
PUBLIC int (LIBCCALL strcasecoll)(char const *str1, char const *str2) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL strncoll)(char const *str1, char const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL strncasecoll)(char const *str1, char const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return 0; }
PUBLIC char *(LIBCCALL strlwr_l)(char *str, locale_t lc) { NOT_IMPLEMENTED(); return strlwr(str); }
PUBLIC char *(LIBCCALL strupr_l)(char *str, locale_t lc) { NOT_IMPLEMENTED(); return strupr(str); }
PUBLIC int (LIBCCALL strcasecoll_l)(char const *str1, char const *str2, locale_t lc) { NOT_IMPLEMENTED(); return strcasecoll(str1,str2); }
PUBLIC int (LIBCCALL strncoll_l)(char const *str1, char const *str2, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return strncoll(str1,str2,max_chars); }
PUBLIC int (LIBCCALL strncasecoll_l)(char const *str1, char const *str2, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return strncasecoll(str1,str2,max_chars); }

PUBLIC int (LIBCCALL memcasecmp)(void const *s1, void const *s2, size_t n_bytes) {
 char a,b; int result;
 char *p1 = (char *)s1,*p2 = (char *)s2;
 char *end = p1+n_bytes;
 if (!n_bytes) return 0;
 do a = tolower(*p1++),
    b = tolower(*p2++),
    result = (int)a-(int)b;
 while (result == 0 && p1 != end);
 return result;
}
PUBLIC int (LIBCCALL memcasecmp_l)(void const *a, void const *b, size_t n_bytes, locale_t lc) { NOT_IMPLEMENTED(); return 0; }

#ifndef CONFIG_LIBC_NO_WCHAR_STRING
typedef u16 dosch_t;

#define DS(x) __ASMNAME(__PP_STR(__DSYM(x)))

PUBLIC int (LIBCCALL wcscasecmp)(dosch_t const *str1, dosch_t const *str2) DS(wcscasecmp);
PUBLIC int (LIBCCALL wcscasecoll)(dosch_t const *str1, dosch_t const *str2) DS(wcscasecoll);
PUBLIC int (LIBCCALL wcscoll)(dosch_t const *str1, dosch_t const *str2) DS(wcscoll);
PUBLIC int (LIBCCALL wcsncasecmp)(dosch_t const *str1, dosch_t const *str2, size_t max_chars) DS(wcsncasecmp);
PUBLIC int (LIBCCALL wcsncasecoll)(dosch_t const *str1, dosch_t const *str2, size_t max_chars) DS(wcsncasecoll);
PUBLIC int (LIBCCALL wcscmp)(dosch_t const *str1, dosch_t const *str2) DS(wcscmp);
PUBLIC int (LIBCCALL wcsncmp)(dosch_t const *str1, dosch_t const *str2, size_t max_chars) DS(wcsncmp);
PUBLIC int (LIBCCALL wcsncoll)(dosch_t const *str1, dosch_t const *str2, size_t max_chars) DS(wcsncoll);
PUBLIC size_t (LIBCCALL wcscspn)(dosch_t const *str, dosch_t const *reject) DS(wcscspn);
PUBLIC size_t (LIBCCALL wcslen)(dosch_t const *str) DS(wcslen);
PUBLIC size_t (LIBCCALL wcsnlen)(dosch_t const *src, size_t max_chars) DS(wcsnlen);
PUBLIC size_t (LIBCCALL wcsspn)(dosch_t const *str, dosch_t const *reject) DS(wcsspn);
PUBLIC size_t (LIBCCALL wcsxfrm)(dosch_t *dst, dosch_t const *src, size_t max_chars) DS(wcsxfrm);
PUBLIC dosch_t *(LIBCCALL wcscpy)(dosch_t *dst, dosch_t const *src) DS(wcscpy);
PUBLIC dosch_t *(LIBCCALL wcscat)(dosch_t *dst, dosch_t const *src) DS(wcscat);
PUBLIC dosch_t *(LIBCCALL wcsncat)(dosch_t *dst, dosch_t const *src, size_t max_chars) DS(wcsncat);
PUBLIC dosch_t *(LIBCCALL wcsncpy)(dosch_t *dst, dosch_t const *src, size_t max_chars) DS(wcsncpy);
PUBLIC dosch_t *(LIBCCALL wcsdup)(dosch_t const *str) DS(wcsdup);
PUBLIC dosch_t *(LIBCCALL wcsnset)(dosch_t *str, dosch_t chr, size_t max_chars) DS(wcsnset);
PUBLIC dosch_t *(LIBCCALL wcschr)(dosch_t const *str, dosch_t needle) DS(wcschr);
PUBLIC dosch_t *(LIBCCALL wcsrchr)(dosch_t const *str, dosch_t needle) DS(wcsrchr);
PUBLIC dosch_t *(LIBCCALL wcslwr)(dosch_t *str) DS(wcslwr);
PUBLIC dosch_t *(LIBCCALL wcspbrk)(dosch_t const *str, dosch_t const *reject) DS(wcspbrk);
PUBLIC dosch_t *(LIBCCALL wcsrev)(dosch_t *str) DS(wcsrev);
PUBLIC dosch_t *(LIBCCALL wcsset)(dosch_t *str, dosch_t chr) DS(wcsset);
PUBLIC dosch_t *(LIBCCALL wcsstr)(dosch_t const *haystack, dosch_t const *needle) DS(wcsstr);
PUBLIC dosch_t *(LIBCCALL wcstok)(dosch_t *str, dosch_t const *delim) DS(wcstok);
PUBLIC dosch_t *(LIBCCALL wcsupr)(dosch_t *str) DS(wcsupr);
PUBLIC int (LIBCCALL wcscasecmp_l)(dosch_t const *str1, dosch_t const *str2, locale_t lc) DS(wcscasecmp_l);
PUBLIC int (LIBCCALL wcscasecoll_l)(dosch_t const *str1, dosch_t const *str2, locale_t lc) DS(wcscasecoll_l);
PUBLIC int (LIBCCALL wcscoll_l)(dosch_t const *str1, dosch_t const *str2, locale_t lc) DS(wcscoll_l);
PUBLIC int (LIBCCALL wcsncasecmp_l)(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc) DS(wcsncasecmp_l);
PUBLIC int (LIBCCALL wcsncasecoll_l)(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc) DS(wcsncasecoll_l);
PUBLIC int (LIBCCALL wcsncoll_l)(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc) DS(wcsncoll_l);
PUBLIC size_t (LIBCCALL wcsxfrm_l)(dosch_t *dst, dosch_t const *src, size_t max_chars, locale_t lc) DS(wcsxfrm_l);
PUBLIC dosch_t *(LIBCCALL wcslwr_l)(dosch_t *str, locale_t lc) DS(wcslwr_l);
PUBLIC dosch_t *(LIBCCALL wcsupr_l)(dosch_t *str, locale_t lc) DS(wcsupr_l);

PUBLIC int (LIBCCALL wcscasecmp)(dosch_t const *str1, dosch_t const *str2) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL wcscasecoll)(dosch_t const *str1, dosch_t const *str2) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL wcscoll)(dosch_t const *str1, dosch_t const *str2) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL wcsncasecmp)(dosch_t const *str1, dosch_t const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL wcsncasecoll)(dosch_t const *str1, dosch_t const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL wcscmp)(dosch_t const *str1, dosch_t const *str2) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL wcsncmp)(dosch_t const *str1, dosch_t const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL wcsncoll)(dosch_t const *str1, dosch_t const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return -1; }
PUBLIC size_t (LIBCCALL wcscspn)(dosch_t const *str, dosch_t const *reject) { NOT_IMPLEMENTED(); return 0; }
PUBLIC size_t (LIBCCALL wcslen)(dosch_t const *str) { dosch_t const *end = str; while (*end) ++end; return (size_t)(end-str); }
PUBLIC size_t (LIBCCALL wcsnlen)(dosch_t const *src, size_t max_chars) { dosch_t const *end = src; while (max_chars-- && *end) ++end; return (size_t)(end-src); }
PUBLIC size_t (LIBCCALL wcsspn)(dosch_t const *str, dosch_t const *reject) { NOT_IMPLEMENTED(); return 0; }
PUBLIC size_t (LIBCCALL wcsxfrm)(dosch_t *dst, dosch_t const *src, size_t max_chars) { NOT_IMPLEMENTED(); return 0; }
PUBLIC dosch_t *(LIBCCALL wcscpy)(dosch_t *dst, dosch_t const *src) { memcpy(dst,src,(wcslen(src)+1)*sizeof(dosch_t)); return dst; }
PUBLIC dosch_t *(LIBCCALL wcscat)(dosch_t *dst, dosch_t const *src) { memcpy(dst+wcslen(dst),src,(wcslen(src)+1)*sizeof(dosch_t)); return dst; }
PUBLIC dosch_t *(LIBCCALL wcsncat)(dosch_t *dst, dosch_t const *src, size_t max_chars) {
 size_t maxlen = wcsnlen(src,max_chars);
 dosch_t *target = dst+wcslen(dst);
 memcpy(target,src,maxlen*sizeof(dosch_t));
 if (maxlen < max_chars) target[maxlen] = 0;
 return dst;
}
PUBLIC dosch_t *(LIBCCALL wcsncpy)(dosch_t *dst, dosch_t const *src, size_t max_chars) { return (dosch_t *)memcpy(dst,src,(wcsnlen(src,max_chars)+1)*sizeof(dosch_t)); }
PUBLIC dosch_t *(LIBCCALL wcsdup)(dosch_t const *str) { return (dosch_t *)(memdup)(str,wcslen(str)*sizeof(dosch_t)); }
PUBLIC dosch_t *(LIBCCALL wcsnset)(dosch_t *str, dosch_t chr, size_t max_chars) { dosch_t *result = str; while (max_chars-- && *str) *str++ = chr; return result; }
PUBLIC dosch_t *(LIBCCALL wcschr)(dosch_t const *str, dosch_t needle) { return (dosch_t *)memchrw(str,needle,wcslen(str)); }
PUBLIC dosch_t *(LIBCCALL wcsrchr)(dosch_t const *str, dosch_t needle) { return (dosch_t *)memrchrw(str,needle,wcslen(str)); }
PUBLIC dosch_t *(LIBCCALL wcslwr)(dosch_t *str) { NOT_IMPLEMENTED(); return str; }
PUBLIC dosch_t *(LIBCCALL wcspbrk)(dosch_t const *str, dosch_t const *reject) { NOT_IMPLEMENTED(); return (dosch_t *)str; }
PUBLIC dosch_t *(LIBCCALL wcsrev)(dosch_t *str) { NOT_IMPLEMENTED(); return str; }
PUBLIC dosch_t *(LIBCCALL wcsset)(dosch_t *str, dosch_t chr) { NOT_IMPLEMENTED(); return str; }
PUBLIC dosch_t *(LIBCCALL wcsstr)(dosch_t const *haystack, dosch_t const *needle) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC dosch_t *(LIBCCALL wcstok)(dosch_t *str, dosch_t const *delim) { NOT_IMPLEMENTED(); return str; }
PUBLIC dosch_t *(LIBCCALL wcsupr)(dosch_t *str) { NOT_IMPLEMENTED(); return str; }
PUBLIC int (LIBCCALL wcscasecmp_l)(dosch_t const *str1, dosch_t const *str2, locale_t lc) { NOT_IMPLEMENTED(); return wcscasecmp(str1,str2); }
PUBLIC int (LIBCCALL wcscasecoll_l)(dosch_t const *str1, dosch_t const *str2, locale_t lc) { NOT_IMPLEMENTED(); return wcscasecoll(str1,str2); }
PUBLIC int (LIBCCALL wcscoll_l)(dosch_t const *str1, dosch_t const *str2, locale_t lc) { NOT_IMPLEMENTED(); return wcscoll(str1,str2); }
PUBLIC int (LIBCCALL wcsncasecmp_l)(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return wcsncasecmp(str1,str2,max_chars); }
PUBLIC int (LIBCCALL wcsncasecoll_l)(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return wcsncasecoll(str1,str2,max_chars); }
PUBLIC int (LIBCCALL wcsncoll_l)(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return wcsncoll(str1,str2,max_chars); }
PUBLIC size_t (LIBCCALL wcsxfrm_l)(dosch_t *dst, dosch_t const *src, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return wcsxfrm(dst,src,max_chars); }
PUBLIC dosch_t *(LIBCCALL wcslwr_l)(dosch_t *str, locale_t lc) { NOT_IMPLEMENTED(); return wcslwr(str); }
PUBLIC dosch_t *(LIBCCALL wcsupr_l)(dosch_t *str, locale_t lc) { NOT_IMPLEMENTED(); return wcsupr(str); }
#endif /* !CONFIG_LIBC_NO_WCHAR_STRING */
#endif /* !CONFIG_LIBC_NO_DOS_EXTENSIONS */
#endif /* !__KERNEL__ */


DECL_END

//#ifndef __INTELLISENSE__
//#include "strerror.c.inl"
//#endif

#endif /* !GUARD_LIBS_LIBC_STRING_C */
