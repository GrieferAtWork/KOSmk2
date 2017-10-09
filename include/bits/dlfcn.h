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
#ifndef _BITS_DLFCN_H
#define _BITS_DLFCN_H 1

#include <__stdinc.h>
#include <features.h>

__SYSDECL_BEGIN

/* System dependent definitions for run-time dynamic loading.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
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


/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY         0x00001 /*< Lazy function call binding. */
#define RTLD_NOW          0x00002 /*< Immediate function call binding. */
#define RTLD_BINDING_MASK 0x00003 /*< Mask of binding time value. */

#define RTLD_NOLOAD       0x00004 /*< Do not load the object. */
#define RTLD_DEEPBIND     0x00008 /*< Use deep binding. */

/* If the following bit is set in the MODE argument to `dlopen',
 * the symbols of the loaded object and its dependencies are made
 * visible as if the object were linked directly into the program. */
#define RTLD_GLOBAL       0x00100

/* Unix98 demands the following flag which is the inverse to RTLD_GLOBAL.
 * The implementation does this by default and so we can define the
 * value to zero. */
#define RTLD_LOCAL        0x00000

/* Do not delete object when closed.  */
#define RTLD_NODELETE     0x01000

#ifdef __USE_GNU
/* To support profiling of shared objects it is a good idea to call
 * the function found using `dlsym' using the following macro since
 * these calls do not use the PLT.  But this would mean the dynamic
 * loader has no chance to find out when the function is called.  The
 * macro applies the necessary magic so that profiling is possible.
 * Rewrite
 *  foo = (*fctp)(arg1,arg2);
 * into
 *      foo = DL_CALL_FCT(fctp,(arg1,arg2));
 */
#ifdef __KERNEL__
#define DL_CALL_FCT(fctp,args) (*(fctp)) args)
#else
#define DL_CALL_FCT(fctp,args) \
       (_dl_mcount_wrapper_check((void *)(fctp)),(*(fctp)) args)
extern void (_dl_mcount_wrapper_check)(void *__selfpc);
#endif
#endif /* __USE_GNU */

__SYSDECL_END

#endif /* !_BITS_DLFCN_H */
