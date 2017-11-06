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
#ifndef __GUARD_HYBRID_SCHED_YIELD_H
#define __GUARD_HYBRID_SCHED_YIELD_H 1

#include <hybrid/compiler.h>

DECL_BEGIN

#ifdef __CC__
#ifdef __KERNEL__

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __task_yield_defined
#define __task_yield_defined 1
/* Yield the remainder of the caller's quantum to the next
 * scheduled task (no-op if no task to switch to exists).
 * HINT: All registers but EAX are preserved across a call to this function.
 * @return: -EOK:       Another task was executed before this function returned.
 * @return: -EAGAIN:    There was no other task to switch to. */
FUNDEF errno_t (KCALL task_yield)(void);

#if !defined(__NO_XBLOCK) && defined(__COMPILER_HAVE_GCC_ASM)
/* Take advantage of the fact that `task_yield()' doesn't clobber anything. */
#define task_yield() \
 __XBLOCK({ register errno_t __y_err; \
            __asm__ __volatile__("call task_yield\n" : "=a" (__y_err)); \
            __XRETURN __y_err; \
 })
#endif
#endif /* !__task_yield_defined */

#define SCHED_YIELD() task_yield()
#else
__LIBC int (LIBCCALL sched_yield)(void);
#   define SCHED_YIELD() sched_yield()
#endif
#endif /* __CC__ */

DECL_END

#endif /* !__GUARD_HYBRID_SCHED_YIELD_H */
