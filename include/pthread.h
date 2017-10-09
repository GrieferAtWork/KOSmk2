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
#ifndef _PTHREAD_H
#define _PTHREAD_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/pthreadtypes.h>

#define __LIBP     __IMPDEF
#define __LIBPCALL __ATTR_CDECL

__SYSDECL_BEGIN

/* TODO */

__LIBP __NONNULL((1,3)) int (__LIBPCALL pthread_create)(pthread_t *__restrict __newthread, pthread_attr_t const *__restrict __attr, void *(*__start_routine)(void *__arg), void *__restrict __arg);
__LIBP __ATTR_NORETURN void (__LIBPCALL pthread_exit)(void *__retval);
__LIBP int (__LIBPCALL pthread_join)(pthread_t __th, void **__thread_return);
__LIBP __ATTR_CONST pthread_t __NOTHROW((__LIBPCALL pthread_self)(void));
__LIBP __ATTR_CONST int __NOTHROW((__LIBPCALL pthread_equal)(pthread_t __thread1, pthread_t __thread2));

__SYSDECL_END

#endif /* !_PTHREAD_H */
