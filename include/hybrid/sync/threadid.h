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
#ifndef GUARD_HYBRID_SYNC_THREADID_H
#define GUARD_HYBRID_SYNC_THREADID_H 1

#include <hybrid/compiler.h>

#ifdef __KERNEL__
#include <sched/percpu.h>

DECL_BEGIN

#define THREADID_SIZE  __SIZEOF_POINTER__
typedef struct task *threadid_t;
#define THREADID_INVALID_IS_ZERO 1
#define THREADID_INVALID         NULL
#define THREADID_SELF()          THIS_TASK

DECL_END

#else
#include <hybrid/types.h>
DECL_BEGIN

__LIBC pid_t (__LIBCCALL __gettid)(void);

typedef pid_t threadid_t;
#if 0
#define THREADID_INVALID       (-1)
#else
#define THREADID_INVALID_IS_ZERO 1 /* Not always, but good enough? */
#define THREADID_INVALID         0
#endif
#define THREADID_SELF()        __gettid()

DECL_END
#endif


#endif /* !GUARD_HYBRID_SYNC_THREADID_H */
