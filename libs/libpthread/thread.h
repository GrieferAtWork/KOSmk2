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
#ifndef GUARD_LIBS_LIBPTHREAD_THREAD_H
#define GUARD_LIBS_LIBPTHREAD_THREAD_H 1

#include "libp.h"
#include <hybrid/compiler.h>

DECL_BEGIN

INTDEF int LIBPCALL thread_create(pthread_t *__restrict newthread,
                                  pthread_attr_t const *__restrict attr,
                                  void *(*start_routine)(void *__arg),
                                  void *__restrict arg);
INTDEF ATTR_NORETURN void LIBPCALL thread_exit(void *retval);
INTDEF int LIBPCALL thread_join(pthread_t th, void **thread_return);
INTDEF pthread_t LIBPCALL thread_self(void);
INTDEF int LIBPCALL thread_equal(pthread_t thread1, pthread_t thread2);

DECL_END

#endif /* !GUARD_LIBS_LIBPTHREAD_THREAD_H */