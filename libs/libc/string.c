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

#include <alloca.h>
#include <assert.h>
#include <bits/signum.h>
#include <hybrid/section.h>
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
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef __KERNEL__
#include "system.h"
#include <wchar.h>
#endif

DECL_BEGIN

#define DECL INTERN
#ifdef CONFIG_64BIT_STRING
#define BITS 64
#include "templates/memory.code"
#endif
#define BITS 32
#include "templates/memory.code"
#define BITS 16
#include "templates/memory.code"
#define BITS 8
#include "templates/memory.code"

#define T          char
#define Ts         signed char
#define Tu         unsigned char
#define Tn         int
#define Xstr(x)    libc_str##x
#define Xstp(x)    libc_stp##x
#define TOLOWER(x) libc_tolower(x)
#define TOUPPER(x) libc_toupper(x)
#define S          __SIZEOF_CHAR__
#include "templates/string.code"
#undef DECL

INTERN void *LIBCCALL libc_mempcpy(void *__restrict dst,
                                   void const *__restrict src, size_t n) {
 return (void *)((uintptr_t)libc_memcpy(dst,src,n)+n);
}
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


/* String functions deemed to unimportant to include in the kernel core. (libk) */
#ifndef __KERNEL__
INTERN void LIBCCALL libc_bcopy(void const *src, void *dst, size_t n) { libc_memmove(dst,src,n); }
INTERN void LIBCCALL libc_bzero(void *s, size_t n) { libc_memset(s,0,n); }

INTERN char *LIBCCALL libc_index(char const *__restrict haystack, int needle) {
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
INTERN char *LIBCCALL libc_rindex(char const *__restrict haystack, int needle) {
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

INTERN char *LIBCCALL libc_dirname(char *path) {
 char *iter;
 if (!path || !*path) ret_cwd: return ".";
 iter = strend(path)-1;
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
 if (iter == path) return result-1; /* Only '/'-characters. */
 *iter = '\0'; /* Trim all ending '/'-characters. */
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


/* TODO */INTERN int LIBCCALL libc_strcoll(char const *s1, char const *s2) { NOT_IMPLEMENTED(); return 0; }
/* TODO */INTERN size_t LIBCCALL libc_strxfrm(char *__restrict dst, char const *__restrict src, size_t n) { NOT_IMPLEMENTED(); return 0; }
/* TODO */INTERN char *LIBCCALL libc_strfry(char *string) { NOT_IMPLEMENTED(); return NULL; }
/* TODO */INTERN void *LIBCCALL libc_memfrob(void *s, size_t n) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_strcoll_l(char const *s1, char const *s2, locale_t l) { NOT_IMPLEMENTED(); return libc_strcoll(s1,s2); }
INTERN size_t LIBCCALL libc_strxfrm_l(char *dst, char const *src, size_t n, locale_t l) { NOT_IMPLEMENTED(); return libc_strxfrm(dst,src,n); }
INTERN char *LIBCCALL libc_strerror_l(int errnum, locale_t l) { NOT_IMPLEMENTED(); return libc_strerror(errnum); }
INTERN int LIBCCALL libc_strcasecmp_l(char const *s1, char const *s2, locale_t loc) { NOT_IMPLEMENTED(); return libc_strcasecmp(s1,s2); }
INTERN int LIBCCALL libc_strncasecmp_l(char const *s1, char const *s2, size_t n, locale_t loc) { NOT_IMPLEMENTED(); return libc_strncasecmp(s1,s2,n); }
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
 entry   = (struct errnotext_entry const *)((uintptr_t)data+data->etd_enotab+
                                            (size_t)no*data->etd_enoent);
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



INTERN char *LIBCCALL libc_strlwr(char *str) { char *result = str; for (; *str; ++str) *str = libc_tolower(*str); return result; }
INTERN char *LIBCCALL libc_strupr(char *str) { char *result = str; for (; *str; ++str) *str = libc_toupper(*str); return result; }
INTERN char *LIBCCALL libc_strset(char *str, int chr) { char *result = str; while (*str) *str++ = (char)chr; return result; }
INTERN char *LIBCCALL libc_strnset(char *str, int chr, size_t max_chars) { char *result = str,*end = str+max_chars; while (str != end && *str) *str++ = (char)chr; return result; }
INTERN char *LIBCCALL libc_strrev(char *str) { NOT_IMPLEMENTED(); return str; }
INTERN int LIBCCALL libc_strcasecoll(char const *str1, char const *str2) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_strncoll(char const *str1, char const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_strncasecoll(char const *str1, char const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return 0; }
INTERN char *LIBCCALL libc_strlwr_l(char *str, locale_t lc) { NOT_IMPLEMENTED(); return libc_strlwr(str); }
INTERN char *LIBCCALL libc_strupr_l(char *str, locale_t lc) { NOT_IMPLEMENTED(); return libc_strupr(str); }
INTERN int LIBCCALL libc_strcasecoll_l(char const *str1, char const *str2, locale_t lc) { NOT_IMPLEMENTED(); return libc_strcasecoll(str1,str2); }
INTERN int LIBCCALL libc_strncoll_l(char const *str1, char const *str2, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return libc_strncoll(str1,str2,max_chars); }
INTERN int LIBCCALL libc_strncasecoll_l(char const *str1, char const *str2, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return libc_strncasecoll(str1,str2,max_chars); }

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
INTERN int LIBCCALL libc_memcasecmp_l(void const *a, void const *b,
                                      size_t n_bytes, locale_t lc) {
 NOT_IMPLEMENTED();
 return libc_memcasecmp(a,b,n_bytes);
}

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN int LIBCCALL libc_dos_wcscasecmp(dosch_t const *str1, dosch_t const *str2) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_dos_wcscasecoll(dosch_t const *str1, dosch_t const *str2) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_dos_wcscoll(dosch_t const *str1, dosch_t const *str2) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_dos_wcsncasecmp(dosch_t const *str1, dosch_t const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_dos_wcsncasecoll(dosch_t const *str1, dosch_t const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_dos_wcscmp(dosch_t const *str1, dosch_t const *str2) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_dos_wcsncmp(dosch_t const *str1, dosch_t const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_dos_wcsncoll(dosch_t const *str1, dosch_t const *str2, size_t max_chars) { NOT_IMPLEMENTED(); return -1; }
INTERN size_t LIBCCALL libc_dos_wcscspn(dosch_t const *str, dosch_t const *reject) { NOT_IMPLEMENTED(); return 0; }
INTERN size_t LIBCCALL libc_dos_wcslen(dosch_t const *str) { dosch_t const *end = str; while (*end) ++end; return (size_t)(end-str); }
INTERN size_t LIBCCALL libc_dos_wcsnlen(dosch_t const *src, size_t max_chars) { dosch_t const *end = src; while (max_chars-- && *end) ++end; return (size_t)(end-src); }
INTERN size_t LIBCCALL libc_dos_wcsspn(dosch_t const *str, dosch_t const *reject) { NOT_IMPLEMENTED(); return 0; }
INTERN size_t LIBCCALL libc_dos_wcsxfrm(dosch_t *dst, dosch_t const *src, size_t max_chars) { NOT_IMPLEMENTED(); return 0; }
INTERN dosch_t *LIBCCALL libc_dos_wcscpy(dosch_t *dst, dosch_t const *src) { libc_memcpy(dst,src,(libc_dos_wcslen(src)+1)*sizeof(dosch_t)); return dst; }
INTERN dosch_t *LIBCCALL libc_dos_wcscat(dosch_t *dst, dosch_t const *src) { libc_memcpy(dst+libc_dos_wcslen(dst),src,(libc_dos_wcslen(src)+1)*sizeof(dosch_t)); return dst; }
INTERN dosch_t *LIBCCALL libc_dos_wcsncat(dosch_t *dst, dosch_t const *src, size_t max_chars) {
 size_t maxlen = libc_dos_wcsnlen(src,max_chars);
 dosch_t *target = dst+libc_dos_wcslen(dst);
 libc_memcpy(target,src,maxlen*sizeof(dosch_t));
 if (maxlen < max_chars) target[maxlen] = 0;
 return dst;
}
INTERN dosch_t *LIBCCALL libc_dos_wcsncpy(dosch_t *dst, dosch_t const *src, size_t max_chars) { return (dosch_t *)libc_memcpy(dst,src,(libc_dos_wcsnlen(src,max_chars)+1)*sizeof(dosch_t)); }
INTERN dosch_t *LIBCCALL libc_dos_wcsdup(dosch_t const *str) { return (dosch_t *)libc_memdup(str,libc_dos_wcslen(str)*sizeof(dosch_t)); }
INTERN dosch_t *LIBCCALL libc_dos_wcsnset(dosch_t *str, dosch_t chr, size_t max_chars) { dosch_t *result = str; while (max_chars-- && *str) *str++ = chr; return result; }
INTERN dosch_t *LIBCCALL libc_dos_wcschr(dosch_t const *str, dosch_t needle) { return (dosch_t *)libc_memchrw(str,needle,libc_dos_wcslen(str)); }
INTERN dosch_t *LIBCCALL libc_dos_wcsrchr(dosch_t const *str, dosch_t needle) { return (dosch_t *)libc_memrchrw(str,needle,libc_dos_wcslen(str)); }
INTERN dosch_t *LIBCCALL libc_dos_wcslwr(dosch_t *str) { NOT_IMPLEMENTED(); return str; }
INTERN dosch_t *LIBCCALL libc_dos_wcspbrk(dosch_t const *str, dosch_t const *reject) { NOT_IMPLEMENTED(); return (dosch_t *)str; }
INTERN dosch_t *LIBCCALL libc_dos_wcsrev(dosch_t *str) { NOT_IMPLEMENTED(); return str; }
INTERN dosch_t *LIBCCALL libc_dos_wcsset(dosch_t *str, dosch_t chr) { NOT_IMPLEMENTED(); return str; }
INTERN dosch_t *LIBCCALL libc_dos_wcsstr(dosch_t const *haystack, dosch_t const *needle) { NOT_IMPLEMENTED(); return NULL; }
INTERN dosch_t *LIBCCALL libc_dos_wcstok(dosch_t *str, dosch_t const *delim) { NOT_IMPLEMENTED(); return str; }
INTERN dosch_t *LIBCCALL libc_dos_wcsupr(dosch_t *str) { NOT_IMPLEMENTED(); return str; }
INTERN int LIBCCALL libc_dos_wcscasecmp_l(dosch_t const *str1, dosch_t const *str2, locale_t lc) { NOT_IMPLEMENTED(); return libc_dos_wcscasecmp(str1,str2); }
INTERN int LIBCCALL libc_dos_wcscasecoll_l(dosch_t const *str1, dosch_t const *str2, locale_t lc) { NOT_IMPLEMENTED(); return libc_dos_wcscasecoll(str1,str2); }
INTERN int LIBCCALL libc_dos_wcscoll_l(dosch_t const *str1, dosch_t const *str2, locale_t lc) { NOT_IMPLEMENTED(); return libc_dos_wcscoll(str1,str2); }
INTERN int LIBCCALL libc_dos_wcsncasecmp_l(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return libc_dos_wcsncasecmp(str1,str2,max_chars); }
INTERN int LIBCCALL libc_dos_wcsncasecoll_l(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return libc_dos_wcsncasecoll(str1,str2,max_chars); }
INTERN int LIBCCALL libc_dos_wcsncoll_l(dosch_t const *str1, dosch_t const *str2, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return libc_dos_wcsncoll(str1,str2,max_chars); }
INTERN size_t LIBCCALL libc_dos_wcsxfrm_l(dosch_t *dst, dosch_t const *src, size_t max_chars, locale_t lc) { NOT_IMPLEMENTED(); return libc_dos_wcsxfrm(dst,src,max_chars); }
INTERN dosch_t *LIBCCALL libc_dos_wcslwr_l(dosch_t *str, locale_t lc) { NOT_IMPLEMENTED(); return libc_dos_wcslwr(str); }
INTERN dosch_t *LIBCCALL libc_dos_wcsupr_l(dosch_t *str, locale_t lc) { NOT_IMPLEMENTED(); return libc_dos_wcsupr(str); }
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */


/* Define public string functions */
#undef memcpy
DEFINE_PUBLIC_ALIAS(memcpy,libc_memcpy);
DEFINE_PUBLIC_ALIAS(_memcpy_d,libc__memcpy_d);
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
DEFINE_PUBLIC_ALIAS(memcpyw,libc_memcpyw);
DEFINE_PUBLIC_ALIAS(_memcpyw_d,libc__memcpyw_d);
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
DEFINE_PUBLIC_ALIAS(memcpyl,libc_memcpyl);
DEFINE_PUBLIC_ALIAS(_memcpyl_d,libc__memcpyl_d);
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
#ifdef CONFIG_64BIT_STRING
DEFINE_PUBLIC_ALIAS(memcpyq,libc_memcpyq);
DEFINE_PUBLIC_ALIAS(_memcpyq_d,libc__memcpyq_d);
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
#endif /* CONFIG_64BIT_STRING */
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
DEFINE_PUBLIC_ALIAS(mempcpy,libc_mempcpy);
DEFINE_PUBLIC_ALIAS(strstr,libc_strstr);
DEFINE_PUBLIC_ALIAS(strcasestr,libc_strcasestr);
DEFINE_PUBLIC_ALIAS(memmem,libc_memmem);
DEFINE_PUBLIC_ALIAS(fuzzy_memcasecmp,libc_fuzzy_memcasecmp);
DEFINE_PUBLIC_ALIAS(fuzzy_memcmp,libc_fuzzy_memcmp);
DEFINE_PUBLIC_ALIAS(fuzzy_strcmp,libc_fuzzy_strcmp);
DEFINE_PUBLIC_ALIAS(fuzzy_strncmp,libc_fuzzy_strncmp);
DEFINE_PUBLIC_ALIAS(fuzzy_strcasecmp,libc_fuzzy_strcasecmp);
DEFINE_PUBLIC_ALIAS(fuzzy_strncasecmp,libc_fuzzy_strncasecmp);
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

#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(bcopy,libc_bcopy);
DEFINE_PUBLIC_ALIAS(bzero,libc_bzero);
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


DEFINE_PUBLIC_ALIAS(strlwr,libc_strlwr);
DEFINE_PUBLIC_ALIAS(strupr,libc_strupr);
DEFINE_PUBLIC_ALIAS(strset,libc_strset);
DEFINE_PUBLIC_ALIAS(strnset,libc_strnset);
DEFINE_PUBLIC_ALIAS(strrev,libc_strrev);
DEFINE_PUBLIC_ALIAS(strcasecoll,libc_strcasecoll);
DEFINE_PUBLIC_ALIAS(strncoll,libc_strncoll);
DEFINE_PUBLIC_ALIAS(strncasecoll,libc_strncasecoll);
DEFINE_PUBLIC_ALIAS(strlwr_l,libc_strlwr_l);
DEFINE_PUBLIC_ALIAS(strupr_l,libc_strupr_l);
DEFINE_PUBLIC_ALIAS(strcasecoll_l,libc_strcasecoll_l);
DEFINE_PUBLIC_ALIAS(strncoll_l,libc_strncoll_l);
DEFINE_PUBLIC_ALIAS(strncasecoll_l,libc_strncasecoll_l);
DEFINE_PUBLIC_ALIAS(memcasecmp,libc_memcasecmp);
DEFINE_PUBLIC_ALIAS(memcasecmp_l,libc_memcasecmp_l);

/* DOS libc functions. */
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#undef libc_strdup
DEFINE_PUBLIC_ALIAS(_strdup,libc_strdup);
DEFINE_PUBLIC_ALIAS(_strlwr,libc_strlwr);
DEFINE_PUBLIC_ALIAS(_strlwr_l,libc_strlwr_l);
DEFINE_PUBLIC_ALIAS(_strnset,libc_strnset);
DEFINE_PUBLIC_ALIAS(_strrev,libc_strrev);
DEFINE_PUBLIC_ALIAS(_strset,libc_strset);
DEFINE_PUBLIC_ALIAS(_strupr,libc_strupr);
DEFINE_PUBLIC_ALIAS(_strupr_l,libc_strupr_l);
DEFINE_PUBLIC_ALIAS(_memicmp,libc_memcasecmp);
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
DEFINE_PUBLIC_ALIAS(memicmp,libc_memcasecmp);
DEFINE_PUBLIC_ALIAS(strcmpi,libc_strcasecmp);
DEFINE_PUBLIC_ALIAS(stricmp,libc_strcasecmp);
DEFINE_PUBLIC_ALIAS(strnicmp,libc_strncasecmp);
DEFINE_PUBLIC_ALIAS(_strxfrm_l,libc_strxfrm_l);
DEFINE_PUBLIC_ALIAS(_memccpy,libc_memccpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsicmp),libc_dos_wcscasecmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsicoll),libc_dos_wcscasecoll);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscoll),libc_dos_wcscoll);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnicmp),libc_dos_wcsncasecmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnicoll),libc_dos_wcsncasecoll);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscmp),libc_dos_wcscmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncmp),libc_dos_wcsncmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncoll),libc_dos_wcsncoll);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscspn),libc_dos_wcscspn);
DEFINE_PUBLIC_ALIAS(__DSYM(wcslen),libc_dos_wcslen);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnlen),libc_dos_wcsnlen);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsspn),libc_dos_wcsspn);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsxfrm),libc_dos_wcsxfrm);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscpy),libc_dos_wcscpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscat),libc_dos_wcscat);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncat),libc_dos_wcsncat);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncpy),libc_dos_wcsncpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsdup),libc_dos_wcsdup);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnset),libc_dos_wcsnset);
DEFINE_PUBLIC_ALIAS(__DSYM(wcschr),libc_dos_wcschr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsrchr),libc_dos_wcsrchr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcslwr),libc_dos_wcslwr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcspbrk),libc_dos_wcspbrk);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsrev),libc_dos_wcsrev);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsset),libc_dos_wcsset);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsstr),libc_dos_wcsstr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcstok),libc_dos_wcstok);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsupr),libc_dos_wcsupr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsicmp_l),libc_dos_wcscasecmp_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsicoll_l),libc_dos_wcscasecoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscoll_l),libc_dos_wcscoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnicmp_l),libc_dos_wcsncasecmp_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnicoll_l),libc_dos_wcsncasecoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncoll_l),libc_dos_wcsncoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsxfrm_l),libc_dos_wcsxfrm_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcslwr_l),libc_dos_wcslwr_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsupr_l),libc_dos_wcsupr_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsicmp),libc_dos_wcscasecmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsicoll),libc_dos_wcscasecoll);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscoll),libc_dos_wcscoll);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnicmp),libc_dos_wcsncasecmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnicoll),libc_dos_wcsncasecoll);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscmp),libc_dos_wcscmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncmp),libc_dos_wcsncmp);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncoll),libc_dos_wcsncoll);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscspn),libc_dos_wcscspn);
DEFINE_PUBLIC_ALIAS(__DSYM(wcslen),libc_dos_wcslen);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnlen),libc_dos_wcsnlen);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsspn),libc_dos_wcsspn);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsxfrm),libc_dos_wcsxfrm);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscpy),libc_dos_wcscpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscat),libc_dos_wcscat);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncat),libc_dos_wcsncat);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncpy),libc_dos_wcsncpy);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsdup),libc_dos_wcsdup);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnset),libc_dos_wcsnset);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsupr_l),libc_dos_wcsupr_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcschr),libc_dos_wcschr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsrchr),libc_dos_wcsrchr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcslwr),libc_dos_wcslwr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcspbrk),libc_dos_wcspbrk);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsrev),libc_dos_wcsrev);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsset),libc_dos_wcsset);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsstr),libc_dos_wcsstr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcstok),libc_dos_wcstok);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsupr),libc_dos_wcsupr);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsicmp_l),libc_dos_wcscasecmp_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsicoll_l),libc_dos_wcscasecoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcscoll_l),libc_dos_wcscoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnicmp_l),libc_dos_wcsncasecmp_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsnicoll_l),libc_dos_wcsncasecoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsncoll_l),libc_dos_wcsncoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcsxfrm_l),libc_dos_wcsxfrm_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcslwr_l),libc_dos_wcslwr_l);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcscoll_l),libc_dos_wcscoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsicmp),libc_dos_wcscasecmp);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsicmp_l),libc_dos_wcscasecmp_l);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsicoll),libc_dos_wcscasecoll);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsicoll_l),libc_dos_wcscasecoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsncoll),libc_dos_wcsncoll);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsncoll_l),libc_dos_wcsncoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsnicmp),libc_dos_wcsncasecmp);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsnicmp_l),libc_dos_wcsncasecmp_l);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsnicoll),libc_dos_wcsncasecoll);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsnicoll_l),libc_dos_wcsncasecoll_l);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsxfrm_l),libc_dos_wcsxfrm_l);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsdup),libc_dos_wcsdup);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcslwr),libc_dos_wcslwr);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcslwr_l),libc_dos_wcslwr_l);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsnset),libc_dos_wcsnset);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsrev),libc_dos_wcsrev);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsset),libc_dos_wcsset);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsupr),libc_dos_wcsupr);
DEFINE_PUBLIC_ALIAS(__DSYM(_wcsupr_l),libc_dos_wcsupr_l);
DEFINE_PUBLIC_ALIAS(__DSYM(wcswcs),libc_dos_wcsstr);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */


/* Wide-string API */
#ifndef __KERNEL__
#if __SIZEOF_WCHAR_T__ == 4
DEFINE_INTERN_ALIAS(libc_wmemcpy,libc_memcpyl);
DEFINE_INTERN_ALIAS(libc_wmemset,libc_memsetl);
DEFINE_INTERN_ALIAS(libc_wmemmove,libc_memmovel);
DEFINE_INTERN_ALIAS(libc_wmemcmp,libc_memcmpl);
DEFINE_INTERN_ALIAS(libc_wmemchr,libc_memchrl);
#elif __SIZEOF_WCHAR_T__ == 2
DEFINE_INTERN_ALIAS(libc_wmemcpy,libc_memcpyw);
DEFINE_INTERN_ALIAS(libc_wmemset,libc_memsetw);
DEFINE_INTERN_ALIAS(libc_wmemmove,libc_memmovew);
DEFINE_INTERN_ALIAS(libc_wmemcmp,libc_memcmpw);
DEFINE_INTERN_ALIAS(libc_wmemchr,libc_memchrw);
#elif __SIZEOF_WCHAR_T__ == 8
DEFINE_INTERN_ALIAS(libc_wmemcpy,libc_memcpyq);
DEFINE_INTERN_ALIAS(libc_wmemset,libc_memsetq);
DEFINE_INTERN_ALIAS(libc_wmemmove,libc_memmoveq);
DEFINE_INTERN_ALIAS(libc_wmemcmp,libc_memcmpq);
DEFINE_INTERN_ALIAS(libc_wmemchr,libc_memchrq);
#else
INTERN wchar_t *LIBCCALL libc_wmemcpy(wchar_t *__restrict s1, wchar_t const *__restrict s2, size_t n) { return (wchar_t *)libc_memcpy(s1,s2,n*sizeof(wchar_t)); }
INTERN wchar_t *LIBCCALL libc_wmemmove(wchar_t *s1, wchar_t const *s2, size_t n) { return (wchar_t *)libc_memmove(s1,s2,n*sizeof(wchar_t)); }
INTERN wchar_t *LIBCCALL libc_wmemset(wchar_t *s, wchar_t c, size_t n) { return (wchar_t *)libc_memset(s,(int)c,n*sizeof(wchar_t)); }
INTERN int LIBCCALL libc_wmemcmp(wchar_t const *s1, wchar_t const *s2, size_t n) { return libc_memcmp(s1,s2,n*sizeof(wchar_t)); }
INTERN wchar_t *LIBCCALL libc_wmemchr(wchar_t const *s, wchar_t c, size_t n) { return (wchar_t *)libc_memchr(s,(int)c,n*sizeof(wchar_t)); }
#endif

#define T          wchar_t
#if __SIZEOF_WCHAR_T__ == 4
#define Ts         s32
#define Tu         u32
#elif __SIZEOF_WCHAR_T__ == 2
#define Ts         s16
#define Tu         u16
#else
#error FIXME
#endif
#define Tn         wint_t
#define Xstr(x)    libc_wcs##x
#define Xstp(x)    libc_wcp##x
#define TOLOWER(x) libc_towlower(x)
#define TOUPPER(x) libc_towupper(x)
#define S          __SIZEOF_CHAR__
#define IS_WIDE    1
#define DECL       INTERN
#include "templates/string.code"
#undef DECL


INTERN wchar_t *LIBCCALL libc_wmempcpy(wchar_t *__restrict s1, wchar_t const *__restrict s2, size_t n) { return libc_wmemcpy(s1,s2,n)+n; }
INTERN int LIBCCALL libc_wcscasecmp_l(wchar_t const *s1, wchar_t const *s2, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcscasecmp(s1,s2); }
INTERN int LIBCCALL libc_wcsncasecmp_l(wchar_t const *s1, wchar_t const *s2, size_t n, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcsncasecmp(s1,s2,n); }
INTERN int LIBCCALL libc_wcscoll(wchar_t const *s1, wchar_t const *s2) { NOT_IMPLEMENTED(); return 0; }
INTERN size_t LIBCCALL libc_wcsxfrm(wchar_t *__restrict s1, wchar_t const *__restrict s2, size_t n) { NOT_IMPLEMENTED(); return 0; }
INTERN wint_t LIBCCALL libc_btowc(int c) { /*NOT_IMPLEMENTED();*/ return (wint_t)c; }
INTERN int LIBCCALL libc_wctob(wint_t c) { /*NOT_IMPLEMENTED();*/ return (int)c; }
INTERN int LIBCCALL libc_mbsinit(struct __mbstate const *ps) { NOT_IMPLEMENTED(); return -1; }
INTERN size_t LIBCCALL libc_mbrtowc(wchar_t *__restrict pwc, char const *__restrict s, size_t n, struct __mbstate *__restrict p) { /*NOT_IMPLEMENTED();*/ if (!n || !s) return 0; if (pwc) *pwc = (wchar_t)*s; return *s ? 1 : 0; }
INTERN size_t LIBCCALL libc_wcrtomb(char *__restrict s, wchar_t wc, struct __mbstate *__restrict ps) { /*NOT_IMPLEMENTED();*/ if (s) *s = wc; return 1; }
INTERN size_t LIBCCALL libc_mbrlen(char const *__restrict s, size_t n, struct __mbstate *__restrict ps) { NOT_IMPLEMENTED(); return n; }
INTERN size_t LIBCCALL libc_mbsrtowcs(wchar_t *__restrict dst, char const **__restrict src, size_t len, struct __mbstate *__restrict ps) { return libc_mbsnrtowcs(dst,src,(size_t)-1,len,ps); }
INTERN size_t LIBCCALL libc_wcsrtombs(char *__restrict dst, wchar_t const **__restrict src, size_t len, struct __mbstate *__restrict ps) { return libc_wcsnrtombs(dst,src,(size_t)-1,len,ps); }
INTERN size_t LIBCCALL libc_mbsnrtowcs(wchar_t *__restrict dst, char const **__restrict src, size_t nmc, size_t len, struct __mbstate *__restrict ps) { size_t result = 0; NOT_IMPLEMENTED(); while (len-- && nmc--) ++result,*dst++ = (wchar_t)*(*src++); return result; }
INTERN size_t LIBCCALL libc_wcsnrtombs(char *__restrict dst, wchar_t const **__restrict src, size_t nwc, size_t len, struct __mbstate *__restrict ps) { size_t result = 0; NOT_IMPLEMENTED(); while (len-- && nwc--) ++result,*dst++ = (char)*(*src++); return result; }
INTERN double LIBCCALL libc_wcstod(wchar_t const *__restrict nptr, wchar_t **__restrict endptr) { NOT_IMPLEMENTED(); return 0; }
INTERN long int LIBCCALL libc_wcstol(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, int base) { NOT_IMPLEMENTED(); return 0; }
INTERN unsigned long int LIBCCALL libc_wcstoul(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, int base) { NOT_IMPLEMENTED(); return 0; }
INTERN size_t LIBCCALL libc_wcsftime(wchar_t *__restrict s, size_t maxsize, wchar_t const *__restrict format, struct tm const *__restrict tp) { NOT_IMPLEMENTED(); return 0; }
INTERN float LIBCCALL libc_wcstof(wchar_t const *__restrict nptr, wchar_t **__restrict endptr) { NOT_IMPLEMENTED(); return 0; }
INTERN long double LIBCCALL libc_wcstold(wchar_t const *__restrict nptr, wchar_t **__restrict endptr) { NOT_IMPLEMENTED(); return 0; }
INTERN __LONGLONG LIBCCALL libc_wcstoll(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, int base) { NOT_IMPLEMENTED(); return 0; }
INTERN __ULONGLONG LIBCCALL libc_wcstoull(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, int base) { NOT_IMPLEMENTED(); return 0; }
INTERN __LONGLONG LIBCCALL libc_wcstoq(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, int base) { NOT_IMPLEMENTED(); return 0; }
INTERN __ULONGLONG LIBCCALL libc_wcstouq(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, int base) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_wcscoll_l(wchar_t const *s1, wchar_t const *s2, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcscoll(s1,s2); }
INTERN size_t LIBCCALL libc_wcsxfrm_l(wchar_t *s1, wchar_t const *s2, size_t n, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcsxfrm(s1,s2,n); }
INTERN wchar_t *LIBCCALL libc_wcsdup(wchar_t const *__restrict str) { return (wchar_t *)libc_memdup(str,(libc_wcslen(str)+1)*sizeof(wchar_t)); }
INTERN ssize_t LIBCCALL libc_wcwidth(wchar_t c) { NOT_IMPLEMENTED(); return 1; }
INTERN ssize_t LIBCCALL libc_wcswidth(wchar_t const *s, size_t n) { NOT_IMPLEMENTED(); return (ssize_t)n; }
INTERN long int LIBCCALL libc_wcstol_l(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, int base, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcstol(nptr,endptr,base); }
INTERN unsigned long int LIBCCALL libc_wcstoul_l(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, int base, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcstoul(nptr,endptr,base); }
INTERN __LONGLONG LIBCCALL libc_wcstoll_l(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, int base, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcstoll(nptr,endptr,base); }
INTERN __ULONGLONG LIBCCALL libc_wcstoull_l(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, int base, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcstoull(nptr,endptr,base); }
INTERN double LIBCCALL libc_wcstod_l(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcstod(nptr,endptr); }
INTERN float LIBCCALL libc_wcstof_l(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcstof(nptr,endptr); }
INTERN long double LIBCCALL libc_wcstold_l(wchar_t const *__restrict nptr, wchar_t **__restrict endptr, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcstold(nptr,endptr); }
INTERN size_t LIBCCALL libc_wcsftime_l(wchar_t *__restrict s, size_t maxsize, wchar_t const *__restrict format, struct tm const *__restrict tp, locale_t loc) { NOT_IMPLEMENTED(); return libc_wcsftime(s,maxsize,format,tp); }

INTERN size_t LIBCCALL libc_mbrtoc16(char16_t *__restrict pc16, const char *__restrict s, size_t n, struct __mbstate *__restrict p) { /*NOT_IMPLEMENTED();*/ if (!n || !s) return 0; if (pc16) *pc16 = (char16_t)*s; return *s ? 1 : 0; }
INTERN size_t LIBCCALL libc_mbrtoc32(char32_t *__restrict pc32, const char *__restrict s, size_t n, struct __mbstate *__restrict p) { /*NOT_IMPLEMENTED();*/ if (!n || !s) return 0; if (pc32) *pc32 = (char32_t)*s; return *s ? 1 : 0; }
INTERN size_t LIBCCALL libc_c16rtomb(char *__restrict s, char16_t c16, struct __mbstate *__restrict ps) { /*NOT_IMPLEMENTED();*/ if (s) *s = c16; return 1; }
INTERN size_t LIBCCALL libc_c32rtomb(char *__restrict s, char32_t c32, struct __mbstate *__restrict ps) { /*NOT_IMPLEMENTED();*/ if (s) *s = c32; return 1; }

DEFINE_PUBLIC_ALIAS(wcscpy,libc_wcscpy);
DEFINE_PUBLIC_ALIAS(wcsncpy,libc_wcsncpy);
DEFINE_PUBLIC_ALIAS(wcscat,libc_wcscat);
DEFINE_PUBLIC_ALIAS(wcsncat,libc_wcsncat);
DEFINE_PUBLIC_ALIAS(wcpcpy,libc_wcpcpy);
DEFINE_PUBLIC_ALIAS(wcpncpy,libc_wcpncpy);
DEFINE_PUBLIC_ALIAS(wcscmp,libc_wcscmp);
DEFINE_PUBLIC_ALIAS(wcsncmp,libc_wcsncmp);
DEFINE_PUBLIC_ALIAS(wcscoll,libc_wcscoll);
DEFINE_PUBLIC_ALIAS(wcsxfrm,libc_wcsxfrm);
DEFINE_PUBLIC_ALIAS(btowc,libc_btowc);
DEFINE_PUBLIC_ALIAS(wctob,libc_wctob);
DEFINE_PUBLIC_ALIAS(mbsinit,libc_mbsinit);
DEFINE_PUBLIC_ALIAS(mbrtowc,libc_mbrtowc);
DEFINE_PUBLIC_ALIAS(wcrtomb,libc_wcrtomb);
DEFINE_PUBLIC_ALIAS(mbrlen,libc_mbrlen);
DEFINE_PUBLIC_ALIAS(mbsrtowcs,libc_mbsrtowcs);
DEFINE_PUBLIC_ALIAS(wcsrtombs,libc_wcsrtombs);
DEFINE_PUBLIC_ALIAS(wcstod,libc_wcstod);
DEFINE_PUBLIC_ALIAS(wcstol,libc_wcstol);
DEFINE_PUBLIC_ALIAS(wcstoul,libc_wcstoul);
DEFINE_PUBLIC_ALIAS(wcsftime,libc_wcsftime);
DEFINE_PUBLIC_ALIAS(wcstok,libc_wcstok);
DEFINE_PUBLIC_ALIAS(wcslen,libc_wcslen);
DEFINE_PUBLIC_ALIAS(wcsnlen,libc_wcsnlen);
DEFINE_PUBLIC_ALIAS(wcsend,libc_wcsend);
DEFINE_PUBLIC_ALIAS(wcsnend,libc_wcsnend);
DEFINE_PUBLIC_ALIAS(wcsspn,libc_wcsspn);
DEFINE_PUBLIC_ALIAS(wcscspn,libc_wcscspn);
DEFINE_PUBLIC_ALIAS(wmemcmp,libc_wmemcmp);
DEFINE_PUBLIC_ALIAS(wmemcpy,libc_wmemcpy);
DEFINE_PUBLIC_ALIAS(wmemmove,libc_wmemmove);
DEFINE_PUBLIC_ALIAS(wmemset,libc_wmemset);
DEFINE_PUBLIC_ALIAS(wcschr,libc_wcschr);
DEFINE_PUBLIC_ALIAS(wcsrchr,libc_wcsrchr);
DEFINE_PUBLIC_ALIAS(wcspbrk,libc_wcspbrk);
DEFINE_PUBLIC_ALIAS(wcsstr,libc_wcsstr);
DEFINE_PUBLIC_ALIAS(wmemchr,libc_wmemchr);
DEFINE_PUBLIC_ALIAS(wcstof,libc_wcstof);
DEFINE_PUBLIC_ALIAS(wcstold,libc_wcstold);
DEFINE_PUBLIC_ALIAS(wcstoll,libc_wcstoll);
DEFINE_PUBLIC_ALIAS(wcstoull,libc_wcstoull);
DEFINE_PUBLIC_ALIAS(wcscasecmp,libc_wcscasecmp);
DEFINE_PUBLIC_ALIAS(wcsncasecmp,libc_wcsncasecmp);
DEFINE_PUBLIC_ALIAS(wcscasecmp_l,libc_wcscasecmp_l);
DEFINE_PUBLIC_ALIAS(wcsncasecmp_l,libc_wcsncasecmp_l);
DEFINE_PUBLIC_ALIAS(wcscoll_l,libc_wcscoll_l);
DEFINE_PUBLIC_ALIAS(wcsxfrm_l,libc_wcsxfrm_l);
DEFINE_PUBLIC_ALIAS(wcsdup,libc_wcsdup);
DEFINE_PUBLIC_ALIAS(mbsnrtowcs,libc_mbsnrtowcs);
DEFINE_PUBLIC_ALIAS(wcsnrtombs,libc_wcsnrtombs);
DEFINE_PUBLIC_ALIAS(wcwidth,libc_wcwidth);
DEFINE_PUBLIC_ALIAS(wcswidth,libc_wcswidth);
DEFINE_PUBLIC_ALIAS(wcschrnul,libc_wcschrnul);
DEFINE_PUBLIC_ALIAS(wmempcpy,libc_wmempcpy);
DEFINE_PUBLIC_ALIAS(wcstoq,libc_wcstoq);
DEFINE_PUBLIC_ALIAS(wcstouq,libc_wcstouq);
DEFINE_PUBLIC_ALIAS(wcstol_l,libc_wcstol_l);
DEFINE_PUBLIC_ALIAS(wcstoul_l,libc_wcstoul_l);
DEFINE_PUBLIC_ALIAS(wcstoll_l,libc_wcstoll_l);
DEFINE_PUBLIC_ALIAS(wcstoull_l,libc_wcstoull_l);
DEFINE_PUBLIC_ALIAS(wcstod_l,libc_wcstod_l);
DEFINE_PUBLIC_ALIAS(wcstof_l,libc_wcstof_l);
DEFINE_PUBLIC_ALIAS(wcstold_l,libc_wcstold_l);
DEFINE_PUBLIC_ALIAS(wcsftime_l,libc_wcsftime_l);

DEFINE_PUBLIC_ALIAS(__mbrlen,libc_mbrlen);
DEFINE_PUBLIC_ALIAS(wcswcs,libc_wcsstr);

DEFINE_PUBLIC_ALIAS(mbrtoc16,libc_mbrtoc16);
DEFINE_PUBLIC_ALIAS(mbrtoc32,libc_mbrtoc32);
DEFINE_PUBLIC_ALIAS(c16rtomb,libc_c16rtomb);
DEFINE_PUBLIC_ALIAS(c32rtomb,libc_c32rtomb);
#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_STRING_C */
