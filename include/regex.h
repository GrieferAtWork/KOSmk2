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
#ifndef _REGEX_H
#define _REGEX_H 1

/* WARNING: KOS doesn't yet implement regex.
 * >> Functions from this header are currently exported as stubs. */

#include <features.h>
#include <sys/types.h>

/* Definitions for data structures and routines for the regular
   expression library.
   Copyright (C) 1985, 1989-2016 Free Software Foundation, Inc.
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

#if !defined(__CRT_GLC) && !defined(__CRT_CYG)
#error "<regex.h> is not supported by the linked libc"
#endif /* !__CRT_GLC && !__CRT_CYG */

__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */


#ifdef __CYG_COMPAT__
typedef __LONGPTR_TYPE__ regoff_t;
struct re_guts;
#undef re_magic
#undef re_nsub
#undef re_endp
#undef re_g
typedef struct {
    int             re_magic;
    size_t          re_nsub;
    const char     *re_endp;
    struct re_guts *re_g;
} regex_t;

typedef struct {
    regoff_t rm_so;
    regoff_t rm_eo;
} regmatch_t;

#define REG_BASIC    0000
#define REG_EXTENDED    0001
#define REG_ICASE    0002
#define REG_NOSUB    0004
#define REG_NEWLINE    0010
#define REG_NOSPEC    0020
#define REG_PEND    0040
#define REG_DUMP    0200

#if defined(_XOPEN_SOURCE) || defined(__USE_XOPEN2K)
#define REG_ENOSYS    (-1)
#endif
#define REG_NOERROR     0
#define REG_NOMATCH     1
#define REG_BADPAT      2
#define REG_ECOLLATE    3
#define REG_ECTYPE      4
#define REG_EESCAPE     5
#define REG_ESUBREG     6
#define REG_EBRACK      7
#define REG_EPAREN      8
#define REG_EBRACE      9
#define REG_BADBR      10
#define REG_ERANGE     11
#define REG_ESPACE     12
#define REG_BADRPT     13
#define REG_EMPTY      14
#define REG_ASSERT     15
#define REG_INVARG     16
#define REG_ILLSEQ     17
#define REG_ATOI      255
#define REG_ITOA     0400

#define REG_NOTBOL   00001
#define REG_NOTEOL   00002
#define REG_STARTEND 00004
#define REG_TRACE    00400
#define REG_LARGE    01000
#define REG_BACKR    02000
#else

typedef long int          s_reg_t;
typedef unsigned long int active_reg_t;
typedef unsigned long int reg_syntax_t;

#ifdef __USE_GNU
#   define RE_BACKSLASH_ESCAPE_IN_LISTS ((reg_syntax_t)1)
#   define RE_BK_PLUS_QM                (RE_BACKSLASH_ESCAPE_IN_LISTS << 1)
#   define RE_CHAR_CLASSES              (RE_BK_PLUS_QM << 1)
#   define RE_CONTEXT_INDEP_ANCHORS     (RE_CHAR_CLASSES << 1)
#   define RE_CONTEXT_INDEP_OPS         (RE_CONTEXT_INDEP_ANCHORS << 1)
#   define RE_CONTEXT_INVALID_OPS       (RE_CONTEXT_INDEP_OPS << 1)
#   define RE_DOT_NEWLINE               (RE_CONTEXT_INVALID_OPS << 1)
#   define RE_DOT_NOT_NULL              (RE_DOT_NEWLINE << 1)
#   define RE_HAT_LISTS_NOT_NEWLINE     (RE_DOT_NOT_NULL << 1)
#   define RE_INTERVALS                 (RE_HAT_LISTS_NOT_NEWLINE << 1)
#   define RE_LIMITED_OPS               (RE_INTERVALS << 1)
#   define RE_NEWLINE_ALT               (RE_LIMITED_OPS << 1)
#   define RE_NO_BK_BRACES              (RE_NEWLINE_ALT << 1)
#   define RE_NO_BK_PARENS              (RE_NO_BK_BRACES << 1)
#   define RE_NO_BK_REFS                (RE_NO_BK_PARENS << 1)
#   define RE_NO_BK_VBAR                (RE_NO_BK_REFS << 1)
#   define RE_NO_EMPTY_RANGES           (RE_NO_BK_VBAR << 1)
#   define RE_UNMATCHED_RIGHT_PAREN_ORD (RE_NO_EMPTY_RANGES << 1)
#   define RE_NO_POSIX_BACKTRACKING     (RE_UNMATCHED_RIGHT_PAREN_ORD << 1)
#   define RE_NO_GNU_OPS                (RE_NO_POSIX_BACKTRACKING << 1)
#   define RE_INVALID_INTERVAL_ORD      (RE_DEBUG << 1)
#   define RE_ICASE                     (RE_INVALID_INTERVAL_ORD << 1)
#   define RE_CARET_ANCHORS_HERE        (RE_ICASE << 1)
#   define RE_CONTEXT_INVALID_DUP       (RE_CARET_ANCHORS_HERE << 1)
#   define RE_NO_SUB                    (RE_CONTEXT_INVALID_DUP << 1)
#endif

#undef re_syntax_options
__LIBC reg_syntax_t re_syntax_options;

#ifdef __USE_GNU
#define RE_SYNTAX_EMACS 0
#define RE_SYNTAX_AWK   \
  (RE_BACKSLASH_ESCAPE_IN_LISTS|RE_DOT_NOT_NULL|RE_NO_BK_PARENS|RE_NO_BK_REFS|\
   RE_NO_BK_VBAR|RE_NO_EMPTY_RANGES|RE_DOT_NEWLINE|RE_CONTEXT_INDEP_ANCHORS|\
   RE_CHAR_CLASSES|RE_UNMATCHED_RIGHT_PAREN_ORD|RE_NO_GNU_OPS)
#define RE_SYNTAX_GNU_AWK   \
  ((RE_SYNTAX_POSIX_EXTENDED|RE_BACKSLASH_ESCAPE_IN_LISTS|RE_INVALID_INTERVAL_ORD) & \
  ~(RE_DOT_NOT_NULL|RE_CONTEXT_INDEP_OPS|RE_CONTEXT_INVALID_OPS))
#define RE_SYNTAX_POSIX_AWK   \
  (RE_SYNTAX_POSIX_EXTENDED|RE_BACKSLASH_ESCAPE_IN_LISTS| \
   RE_INTERVALS|RE_NO_GNU_OPS|RE_INVALID_INTERVAL_ORD)
#define RE_SYNTAX_GREP   \
  (RE_BK_PLUS_QM|RE_CHAR_CLASSES|RE_HAT_LISTS_NOT_NEWLINE|RE_INTERVALS|RE_NEWLINE_ALT)
#define RE_SYNTAX_EGREP   \
  (RE_CHAR_CLASSES|RE_CONTEXT_INDEP_ANCHORS|RE_CONTEXT_INDEP_OPS| \
   RE_HAT_LISTS_NOT_NEWLINE|RE_NEWLINE_ALT|RE_NO_BK_PARENS|RE_NO_BK_VBAR)
#define RE_SYNTAX_POSIX_EGREP   \
  (RE_SYNTAX_EGREP|RE_INTERVALS|RE_NO_BK_BRACES|RE_INVALID_INTERVAL_ORD)
#define RE_SYNTAX_ED  RE_SYNTAX_POSIX_BASIC
#define RE_SYNTAX_SED RE_SYNTAX_POSIX_BASIC
#define _RE_SYNTAX_POSIX_COMMON   \
  (RE_CHAR_CLASSES|RE_DOT_NEWLINE|RE_DOT_NOT_NULL|RE_INTERVALS|RE_NO_EMPTY_RANGES)
#define RE_SYNTAX_POSIX_BASIC   \
  (_RE_SYNTAX_POSIX_COMMON|RE_BK_PLUS_QM|RE_CONTEXT_INVALID_DUP)
#define RE_SYNTAX_POSIX_MINIMAL_BASIC    \
  (_RE_SYNTAX_POSIX_COMMON|RE_LIMITED_OPS)
#define RE_SYNTAX_POSIX_EXTENDED    \
  (_RE_SYNTAX_POSIX_COMMON|RE_CONTEXT_INDEP_ANCHORS| \
   RE_CONTEXT_INDEP_OPS|RE_NO_BK_BRACES|RE_NO_BK_PARENS| \
   RE_NO_BK_VBAR|RE_CONTEXT_INVALID_OPS|RE_UNMATCHED_RIGHT_PAREN_ORD)
#define RE_SYNTAX_POSIX_MINIMAL_EXTENDED    \
  (_RE_SYNTAX_POSIX_COMMON|RE_CONTEXT_INDEP_ANCHORS| \
   RE_CONTEXT_INVALID_OPS|RE_NO_BK_BRACES|RE_NO_BK_PARENS| \
   RE_NO_BK_REFS|RE_NO_BK_VBAR|RE_UNMATCHED_RIGHT_PAREN_ORD)

#ifdef RE_DUP_MAX
#undef RE_DUP_MAX
#endif
#define RE_DUP_MAX   0x7fff
#endif


#define REG_EXTENDED 1
#define REG_ICASE   (REG_EXTENDED << 1)
#define REG_NEWLINE (REG_ICASE << 1)
#define REG_NOSUB   (REG_NEWLINE << 1)

#define REG_NOTBOL    1
#define REG_NOTEOL   (1 << 1)
#define REG_STARTEND (1 << 2)

#ifdef __COMPILER_PREFERR_ENUMS
typedef enum {
#if defined(_XOPEN_SOURCE) || defined(__USE_XOPEN2K)
  REG_ENOSYS   = -1,
#define REG_ENOSYS REG_ENOSYS
#endif
  REG_NOERROR  =  0, /*< Success. */
  REG_NOMATCH  =  1, /*< Didn't find a match (for regexec). */
  REG_BADPAT   =  2, /*< Invalid pattern. */
  REG_ECOLLATE =  3, /*< Invalid collating element. */
  REG_ECTYPE   =  4, /*< Invalid character class name. */
  REG_EESCAPE  =  5, /*< Trailing backslash. */
  REG_ESUBREG  =  6, /*< Invalid back reference. */
  REG_EBRACK   =  7, /*< Unmatched left bracket. */
  REG_EPAREN   =  8, /*< Parenthesis imbalance. */
  REG_EBRACE   =  9, /*< Unmatched \{. */
  REG_BADBR    = 10, /*< Invalid contents of \{\}. */
  REG_ERANGE   = 11, /*< Invalid range end. */
  REG_ESPACE   = 12, /*< Ran out of memory. */
  REG_BADRPT   = 13, /*< No preceding re for repetition op. */
  REG_EEND     = 14, /*< Premature end. */
  REG_ESIZE    = 15, /*< Compiled pattern bigger than 2^16 bytes. */
  REG_ERPAREN  = 16  /*< Unmatched ) or \); not returned from regcomp. */
#define REG_NOERROR  REG_NOERROR  /*< Success. */
#define REG_NOMATCH  REG_NOMATCH  /*< Didn't find a match (for regexec). */
#define REG_BADPAT   REG_BADPAT   /*< Invalid pattern. */
#define REG_ECOLLATE REG_ECOLLATE /*< Invalid collating element. */
#define REG_ECTYPE   REG_ECTYPE   /*< Invalid character class name. */
#define REG_EESCAPE  REG_EESCAPE  /*< Trailing backslash. */
#define REG_ESUBREG  REG_ESUBREG  /*< Invalid back reference. */
#define REG_EBRACK   REG_EBRACK   /*< Unmatched left bracket. */
#define REG_EPAREN   REG_EPAREN   /*< Parenthesis imbalance. */
#define REG_EBRACE   REG_EBRACE   /*< Unmatched \{. */
#define REG_BADBR    REG_BADBR    /*< Invalid contents of \{\}. */
#define REG_ERANGE   REG_ERANGE   /*< Invalid range end. */
#define REG_ESPACE   REG_ESPACE   /*< Ran out of memory. */
#define REG_BADRPT   REG_BADRPT   /*< No preceding re for repetition op. */
#define REG_EEND     REG_EEND     /*< Premature end. */
#define REG_ESIZE    REG_ESIZE    /*< Compiled pattern bigger than 2^16 bytes. */
#define REG_ERPAREN  REG_ERPAREN  /*< Unmatched ) or \); not returned from regcomp. */
} reg_errcode_t;
#else
typedef int reg_errcode_t;
#if defined(_XOPEN_SOURCE) || defined(__USE_XOPEN2K)
#define REG_ENOSYS  (-1)
#endif
#define REG_NOERROR   0 /*< Success. */
#define REG_NOMATCH   1 /*< Didn't find a match (for regexec). */
#define REG_BADPAT    2 /*< Invalid pattern. */
#define REG_ECOLLATE  3 /*< Invalid collating element. */
#define REG_ECTYPE    4 /*< Invalid character class name. */
#define REG_EESCAPE   5 /*< Trailing backslash. */
#define REG_ESUBREG   6 /*< Invalid back reference. */
#define REG_EBRACK    7 /*< Unmatched left bracket. */
#define REG_EPAREN    8 /*< Parenthesis imbalance. */
#define REG_EBRACE    9 /*< Unmatched \{. */
#define REG_BADBR    10 /*< Invalid contents of \{\}. */
#define REG_ERANGE   11 /*< Invalid range end. */
#define REG_ESPACE   12 /*< Ran out of memory. */
#define REG_BADRPT   13 /*< No preceding re for repetition op. */
#define REG_EEND     14 /*< Premature end. */
#define REG_ESIZE    15 /*< Compiled pattern bigger than 2^16 bytes. */
#define REG_ERPAREN  16 /*< Unmatched ) or \); not returned from regcomp. */
#endif

#ifndef RE_TRANSLATE_TYPE
#define __RE_TRANSLATE_TYPE unsigned char *
#ifdef __USE_GNU
#   define RE_TRANSLATE_TYPE __RE_TRANSLATE_TYPE
#endif /* __USE_GNU */
#endif /* !RE_TRANSLATE_TYPE */

#ifdef __USE_GNU
#   define __REPB_PREFIX(name)     name
#else
#   define __REPB_PREFIX(name) __##name
#endif

struct re_pattern_buffer {
  unsigned char      *__REPB_PREFIX(buffer);
  unsigned long int   __REPB_PREFIX(allocated);
  unsigned long int   __REPB_PREFIX(used);
  reg_syntax_t        __REPB_PREFIX(syntax);
  char *              __REPB_PREFIX(fastmap);
  __RE_TRANSLATE_TYPE __REPB_PREFIX(translate);
  size_t                            re_nsub;
  unsigned            __REPB_PREFIX(can_be_null) : 1;
#ifdef __USE_GNU
#    define REGS_UNALLOCATED 0
#    define REGS_REALLOCATE 1
#    define REGS_FIXED 2
#endif
  unsigned            __REPB_PREFIX(regs_allocated) : 2;
  unsigned            __REPB_PREFIX(fastmap_accurate) : 1;
  unsigned            __REPB_PREFIX(no_sub) : 1;
  unsigned            __REPB_PREFIX(not_bol) : 1;
  unsigned            __REPB_PREFIX(not_eol) : 1;
  unsigned            __REPB_PREFIX(newline_anchor) : 1;
};
typedef struct re_pattern_buffer regex_t;
typedef int                      regoff_t;

#ifdef __USE_GNU
struct re_registers {
    unsigned  num_regs;
    regoff_t *start;
    regoff_t *end;
};

#ifndef RE_NREGS
#define RE_NREGS 30
#endif /* !RE_NREGS */
#endif /* __USE_GNU */
#endif


typedef struct {
  regoff_t rm_so; /*< Byte offset from string's start to substring's start. */
  regoff_t rm_eo; /*< Byte offset from string's start to substring's end. */
} regmatch_t;

#ifdef __CRT_GLC
#ifdef __USE_GNU
__LIBC __PORT_NOCYG reg_syntax_t (__LIBCCALL re_set_syntax)(reg_syntax_t __syntax);
__LIBC __PORT_NOCYG char const *(__LIBCCALL re_compile_pattern)(char const *__pattern, size_t __length, struct re_pattern_buffer *__buffer);
__LIBC __PORT_NOCYG int (__LIBCCALL re_compile_fastmap)(struct re_pattern_buffer *__buffer);
__LIBC __PORT_NOCYG int (__LIBCCALL re_search)(struct re_pattern_buffer *__buffer, char const *__string, int __length, int __start, int __range, struct re_registers *__regs);
__LIBC __PORT_NOCYG int (__LIBCCALL re_search_2)(struct re_pattern_buffer *__buffer, char const *__string1, int __length1, char const *__string2, int __length2, int __start, int __range, struct re_registers *__regs, int __stop);
__LIBC __PORT_NOCYG int (__LIBCCALL re_match)(struct re_pattern_buffer *__buffer, char const *__string, int __length, int __start, struct re_registers *__regs);
__LIBC __PORT_NOCYG int (__LIBCCALL re_match_2)(struct re_pattern_buffer *__buffer, char const *__string1, int __length1, char const *__string2, int __length2, int __start, struct re_registers *__regs, int __stop);
__LIBC __PORT_NOCYG void (__LIBCCALL re_set_registers)(struct re_pattern_buffer *__buffer, struct re_registers *__regs, unsigned int __num_regs, regoff_t *__starts, regoff_t *__ends);
#endif /* __USE_GNU */

#if defined(_REGEX_RE_COMP) || (defined(_LIBC) && defined(__USE_MISC))
#ifndef _CRAY
__LIBC __PORT_NOCYG char *(__LIBCCALL re_comp)(char const *);
__LIBC __PORT_NOCYG int (__LIBCCALL re_exec)(char const *);
#endif /* !_CRAY */
#endif /* _REGEX_RE_COMP || (_LIBC && __USE_MISC) */
#endif /* __CRT_GLC */

__LIBC int (__LIBCCALL regcomp)(regex_t *__restrict __preg, char const *__restrict __pattern, int __cflags);
__LIBC int (__LIBCCALL regexec)(regex_t const *__restrict __preg, char const *__restrict __string, size_t __nmatch, regmatch_t __pmatch[__restrict_arr], int __eflags);
__LIBC size_t (__LIBCCALL regerror)(int __errcode, regex_t const *__restrict __preg, char *__restrict __errbuf, size_t __errbuf_size);
__LIBC void (__LIBCCALL regfree)(regex_t *__preg);

__SYSDECL_END

#endif /* !_REGEX_H */
