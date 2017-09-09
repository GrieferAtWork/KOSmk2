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
#ifndef GUARD_HYBRID_SCHED_YIELD_H
#define GUARD_HYBRID_SCHED_YIELD_H 1

#include <hybrid/compiler.h>

DECL_BEGIN

#ifdef __CC__
#ifdef __KERNEL__
/* Yield execution to another task.
 * NOTE: No-op when interrupts are disabled. */
FUNDEF void (KCALL task_yield)(void);
#   define SCHED_YIELD() task_yield()
#else
__LIBC int (LIBCCALL sched_yield)(void);
#   define SCHED_YIELD() sched_yield()
#endif
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_HYBRID_SCHED_YIELD_H */
