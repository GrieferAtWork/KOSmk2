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
#ifndef _DLFCN_H
#define _DLFCN_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/dlfcn.h>

/* NOTE: To use anything from this file, you must link with '-ldl' */

__SYSDECL_BEGIN

#define __DLFCN      __IMPDEF
#define __DLFCN_CALL __ATTR_CDECL

/* User functions for run-time dynamic loading.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
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

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifdef __USE_GNU
/* If the first argument of `dlsym' or `dlvsym' is set to RTLD_NEXT
 * the run-time address of the symbol called NAME in the next shared
 * object is returned.  The "next" relation is defined by the order
 * the shared objects were loaded. */
# define RTLD_NEXT    ((void *) -1l)
/* If the first argument to `dlsym' or `dlvsym' is set to RTLD_DEFAULT
 * the run-time address of the symbol called NAME in the global scope
 * is returned.
 * HINT: The global scope is the same as that of the root executable,
 *       as seen under /proc/PID/exe. - 'RTLD_DEFAULT' simply behaves
 *       as though a pointer that is apart of the root executable was
 *       passed instead. (Module handles are implemented as pointers
 *       that must be apart of the instance they are describing) */
#define RTLD_DEFAULT  ((void *) 0)

/* Type for namespace indeces. */
typedef long int Lmid_t;

/* Special namespace ID values. */
#define LM_ID_BASE    0  /* Initial namespace.  */
#define LM_ID_NEWLM (-1) /* For dlmopen: request new namespace.  */
#endif

#ifndef __KERNEL__
__DLFCN __WUNUSED void *(__DLFCN_CALL dlopen)(char const *__file, int __mode);
__DLFCN __NONNULL((1)) int (__DLFCN_CALL dlclose)(void *__handle);
__DLFCN __NONNULL((2)) void *(__DLFCN_CALL dlsym)(void *__restrict __handle, char const *__restrict __name);
__DLFCN char *(__DLFCN_CALL dlerror)(void);
#ifdef __USE_KOS
__DLFCN void *(__DLFCN_CALL fdlopen)(int __fd, int __mode);
#endif /* __USE_KOS */
#endif /* !__KERNEL__ */

#ifdef __USE_GNU

typedef struct {
  char const *dli_fname; /*< File name of defining object. */
  void       *dli_fbase; /*< Load address of that object. */
  char const *dli_sname; /*< Name of nearest symbol. */
  void       *dli_saddr; /*< Exact value of nearest symbol. */
} Dl_info;

enum {
 RTLD_DL_SYMENT  = 1,
 RTLD_DL_LINKMAP = 2
};
enum {
 RTLD_DI_LMID        = 1,
 RTLD_DI_LINKMAP     = 2,
 RTLD_DI_CONFIGADDR  = 3,
 RTLD_DI_SERINFO     = 4,
 RTLD_DI_SERINFOSIZE = 5,
 RTLD_DI_ORIGIN      = 6,
 RTLD_DI_PROFILENAME = 7,
 RTLD_DI_PROFILEOUT  = 8,
 RTLD_DI_TLS_MODID   = 9,
 RTLD_DI_TLS_DATA    = 10,
 RTLD_DI_MAX         = 10
};

typedef struct {
  char        *dls_name;
  unsigned int dls_flags;
} Dl_serpath;

typedef struct {
  size_t       dls_size;
  unsigned int dls_cnt;
  Dl_serpath   dls_serpath[1];
} Dl_serinfo;

#ifndef __KERNEL__
__DLFCN void *(__DLFCN_CALL dlmopen)(Lmid_t __nsid, char const *__file, int __mode);
__DLFCN __NONNULL((2,3)) void *(__DLFCN_CALL dlvsym)(void *__restrict __handle, char const *__restrict __name, char const *__restrict __version);
__DLFCN __NONNULL((2)) int (__DLFCN_CALL dladdr)(const void *__address, Dl_info *__info);
__DLFCN __NONNULL((2)) int (__DLFCN_CALL dladdr1)(const void *__address, Dl_info *__info, void **__extra_info, int __flags);
__DLFCN __NONNULL((1,3)) int (__DLFCN_CALL dlinfo)(void *__restrict __handle, int __request, void *__restrict __arg);
#endif /* !__KERNEL__ */

#endif /* __USE_GNU */

__SYSDECL_END

#endif /* !_DLFCN_H */
