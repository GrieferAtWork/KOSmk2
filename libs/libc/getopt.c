/* MIT License
 *
 * Copyright (c) 2017 GrieferAtWork
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef GUARD_LIBS_LIBC_GETOPT_C
#define GUARD_LIBS_LIBC_GETOPT_C 1

#include "libc.h"
#include "environ.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "malloc.h"

#include <hybrid/compiler.h>
#include <getopt.h>

#undef optarg
#undef optind
#undef opterr
#undef optopt

#define NO_ARG       no_argument
#define REQUIRED_ARG required_argument
#define OPTIONAL_ARG optional_argument
#define fprintf      libc_fprintf
#define strncmp      libc_strncmp
#define strlen       libc_strlen
#define memset       libc_memset
#define strchr       libc_strchr
#define strcmp       libc_strcmp
#define flockfile    libc_flockfile
#define funlockfile  libc_funlockfile

DECL_BEGIN

/* DISCLAIMER: The implementation in this file is derived from that of glibc. */
/* Getopt for GNU.
   Copyright (C) 1987-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library and is also part of gnulib.
   Patches to this file should be submitted to both projects.

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

#define PERMUTE         0
#define RETURN_IN_ORDER 1
#define REQUIRE_ORDER   2

PUBLIC char *optarg;
PUBLIC int   optind = 1;
PUBLIC int   opterr = 1;
PUBLIC int   optopt = '?';

PRIVATE char *__nextchar;
PRIVATE int   __first_nonopt;
PRIVATE int   __last_nonopt;
PRIVATE int   __ordering;
PRIVATE int   __initialized;

PRIVATE void LIBCCALL exchange(char **argv) {
 int bottom = __first_nonopt;
 int middle = __last_nonopt;
 int top = optind;
 char *tem;
 while (top > middle && middle > bottom) {
  if (top - middle > middle - bottom) {
   int i,len = middle - bottom;
   for (i = 0; i < len; ++i) {
    tem = argv[bottom+i];
    argv[bottom+i] = argv[top-(middle-bottom)+i];
    argv[top-(middle-bottom)+i] = tem;
   }
   top -= len;
  } else {
   int i,len = top-middle;
   for (i = 0; i < len; ++i) {
    tem = argv[bottom+i];
    argv[bottom+i] = argv[middle+i];
    argv[middle+i] = tem;
   }
   bottom += len;
  }
 }
 __first_nonopt += (optind - __last_nonopt);
 __last_nonopt = optind;
}

PRIVATE int LIBCCALL
process_long_option(int argc, char **argv, char const *optstring,
                    struct option const *longopts, int *longind,
                    int long_only, int print_errors, char const *prefix) {
 char *nameend; size_t namelen;
 struct option const *p,*pfound = NULL;
 int n_options,option_index;
 for (nameend = __nextchar;
     *nameend && *nameend != '=';
    ++nameend);
 namelen = nameend-__nextchar;
 for (p = longopts,n_options = 0; p->name; p++,n_options++) {
  if (!strncmp(p->name,__nextchar,namelen) &&
       namelen == strlen(p->name)) {
   pfound = p;
   option_index = n_options;
   break;
  }
 }
 if (pfound == NULL) {
  unsigned char *ambig_set = NULL;
  int ambig_malloced = 0;
  int ambig_fallback = 0;
  int indfound = -1;
  for (p = longopts,option_index = 0; p->name; p++,option_index++) {
   if (!strncmp(p->name,__nextchar,namelen)) {
    if (pfound == NULL) {
     pfound = p;
     indfound = option_index;
    } else if (long_only ||
               pfound->has_arg != p->has_arg ||
               pfound->flag != p->flag ||
               pfound->val != p->val) {
     if (!ambig_fallback) {
      if (!print_errors)
       ambig_fallback = 1;
      else if (!ambig_set) {
       if (n_options < 512)
        ambig_set = (unsigned char *)alloca(n_options);
       else if ((ambig_set = (unsigned char *)libc_malloc(n_options)) == NULL)
        ambig_fallback = 1;
       else
        ambig_malloced = 1;
       if (ambig_set) {
        memset(ambig_set,0,n_options);
        ambig_set[indfound] = 1;
       }
      }
      if (ambig_set)
       ambig_set[option_index] = 1;
     }
    }
   }
  }
  if (ambig_set || ambig_fallback) {
   if (print_errors) {
    if (ambig_fallback)
     fprintf(stderr,"%s: option '%s%s' is ambiguous\n",argv[0],prefix,__nextchar);
    else {
     flockfile(stderr);
     fprintf(stderr,"%s: option '%s%s' is ambiguous; possibilities:",argv[0],prefix,__nextchar);
     for (option_index = 0; option_index < n_options; option_index++) {
      if (ambig_set[option_index])
          fprintf(stderr," '%s%s'",prefix,longopts[option_index].name);
     }
     fprintf(stderr,"\n");
     funlockfile(stderr);
    }
   }
   if (ambig_malloced) libc_free(ambig_set);
   __nextchar += strlen(__nextchar);
   ++optind;
   optopt = 0;
   return '?';
  }
  option_index = indfound;
 }
 if (pfound == NULL) {
  if (!long_only || argv[optind][1] == '-' ||
      strchr(optstring,*__nextchar) == NULL) {
   if (print_errors)
       fprintf(stderr,"%s: unrecognized option '%s%s'\n",argv[0],prefix,__nextchar);
   __nextchar = NULL;
   ++optind;
   optopt = 0;
   return '?';
  }
  return -1;
 }
 ++optind;
 __nextchar = NULL;
 if (*nameend) {
  if (pfound->has_arg)
   optarg = nameend+1;
  else {
   if (print_errors)
       fprintf(stderr,"%s: option '%s%s' doesn't allow an argument\n",argv[0],prefix,pfound->name);
   optopt = pfound->val;
   return '?';
  }
 } else if (pfound->has_arg == 1) {
  if (optind < argc)
   optarg = argv[optind++];
  else {
   if (print_errors)
       fprintf(stderr,"%s: option '%s%s' requires an argument\n",argv[0],prefix,pfound->name);
   optopt = pfound->val;
   return optstring[0] == ':' ? ':' : '?';
  }
 }
 if (longind != NULL)
    *longind = option_index;
 if (pfound->flag) {
  *pfound->flag = pfound->val;
  return 0;
 }
 return pfound->val;
}


PRIVATE char const *LIBCCALL
_getopt_initialize(int UNUSED(argc), char **UNUSED(argv),
                   char const *optstring, int posixly_correct) {
 if (optind == 0) optind = 1;
 __first_nonopt = __last_nonopt = optind;
 __nextchar     = NULL;
 if (optstring[0] == '-') {
  __ordering = RETURN_IN_ORDER;
  ++optstring;
 } else if (optstring[0] == '+') {
  __ordering = REQUIRE_ORDER;
  ++optstring;
 } else if (posixly_correct || !!libc_getenv("POSIXLY_CORRECT"))
  __ordering = REQUIRE_ORDER;
 else
  __ordering = PERMUTE;
 __initialized = 1;
 return optstring;
}

PRIVATE int LIBCCALL
_getopt_internal_r(int argc, char **argv, char const *optstring,
                   struct option const *longopts, int *longind,
                   int long_only, int posixly_correct) {
 int print_errors = opterr;
 if (argc < 1) return -1;
 optarg = NULL;
 if (optind == 0 || !__initialized)
  optstring = _getopt_initialize(argc,argv,optstring,posixly_correct);
 else if (optstring[0] == '-' || optstring[0] == '+')
  ++optstring;
 if (optstring[0] == ':')
     print_errors = 0;
#define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0')
 if (__nextchar == NULL || *__nextchar == '\0') {
  if (__last_nonopt > optind) __last_nonopt = optind;
  if (__first_nonopt > optind) __first_nonopt = optind;
  if (__ordering == PERMUTE) {
   if (__first_nonopt != __last_nonopt && __last_nonopt != optind)
    exchange(argv);
   else if (__last_nonopt != optind)
    __first_nonopt = optind;
   while (optind < argc && NONOPTION_P) ++optind;
   __last_nonopt = optind;
  }
  if (optind != argc && !strcmp(argv[optind],"--")) {
   ++optind;
   if (__first_nonopt != __last_nonopt &&
       __last_nonopt != optind) exchange(argv);
   else if (__first_nonopt == __last_nonopt)
       __first_nonopt = optind;
   __last_nonopt = argc;
   optind = argc;
  }
  if (optind == argc) {
   if (__first_nonopt != __last_nonopt)
       optind = __first_nonopt;
   return -1;
  }
  if (NONOPTION_P) {
   if (__ordering == REQUIRE_ORDER)
       return -1;
   optarg = argv[optind++];
   return 1;
  }
  if (longopts) {
   if (argv[optind][1] == '-') {
    __nextchar = argv[optind] + 2;
    return process_long_option(argc,argv,optstring,longopts,
                               longind,long_only,print_errors,"--");
   }
   if (long_only &&
      (argv[optind][2] || !strchr(optstring,argv[optind][1]))) {
    int code;
    __nextchar = argv[optind] + 1;
    code = process_long_option(argc,argv,optstring,longopts,
                               longind,long_only,
                               print_errors,"-");
    if (code != -1)
     return code;
   }
  }
  __nextchar = argv[optind] + 1;
 }
 {
  char c = *__nextchar++;
  char const *temp = strchr(optstring,c);
  if (*__nextchar == '\0') ++optind;
  if (temp == NULL || c == ':' || c == ';') {
   if (print_errors)
       fprintf(stderr,"%s: invalid option -- '%c'\n",argv[0],c);
   optopt = c;
   return '?';
  }
  if (temp[0] == 'W' && temp[1] == ';' && longopts != NULL) {
   if (*__nextchar != '\0') optarg = __nextchar;
   else if (optind == argc) {
    if (print_errors)
        fprintf(stderr,"%s: option requires an argument -- '%c'\n",argv[0],c);
    optopt = c;
    c = optstring[0] == ':' ? ':' : '?';
    return c;
   } else {
    optarg = argv[optind];
   }
   __nextchar = optarg;
   optarg = NULL;
   return process_long_option(argc,argv,optstring,longopts,longind,
                              0 /* long_only */,print_errors,"-W ");
  }
  if (temp[1] == ':') {
   if (temp[2] == ':') {
    if (*__nextchar != '\0') {
     optarg = __nextchar;
     optind++;
    } else {
     optarg = NULL;
    }
    __nextchar = NULL;
   } else {
    if (*__nextchar != '\0') {
     optarg = __nextchar;
     ++optind;
    } else if (optind == argc) {
     if (print_errors)
         fprintf(stderr,"%s: option requires an argument -- '%c'\n",argv[0],c);
     optopt = c;
     c = optstring[0] == ':' ? ':' : '?';
    } else {
     optarg = argv[optind++];
    }
    __nextchar = NULL;
   }
  }
  return c;
 }
}

PUBLIC int (LIBCCALL getopt)(int argc, char *const *argv, char const *optstring) { return _getopt_internal_r(argc,(char **)argv,optstring,NULL,NULL,0,0); }
PUBLIC int (LIBCCALL __posix_getopt)(int argc, char *const *argv, char const *optstring) { return _getopt_internal_r(argc,(char **)argv,optstring,NULL,NULL,0,1); }
PUBLIC int (LIBCCALL getopt_long)(int argc, char *const *argv, char const *options, struct option const *long_options, int *opt_index) { return _getopt_internal_r(argc,(char **)argv,options,long_options,opt_index,0,0); }
PUBLIC int (LIBCCALL getopt_long_only)(int argc, char *const *argv, char const *options, struct option const *long_options, int *opt_index) { return _getopt_internal_r(argc,(char **)argv,options,long_options,opt_index,1,0); }

DECL_END

#endif /* !GUARD_LIBS_LIBC_GETOPT_C */
