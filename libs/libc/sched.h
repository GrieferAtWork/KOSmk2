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
#ifndef GUARD_LIBS_LIBC_SCHED_H
#define GUARD_LIBS_LIBC_SCHED_H 1

#include "libc.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <sched.h>

DECL_BEGIN

INTDEF int libc_unshare(int flags);
INTDEF pid_t libc_clone(int (LIBCCALL *fn)(void *arg), void *child_stack, int flags, void *arg, ...);
#ifndef __libc_sched_yield_defined
#define __libc_sched_yield_defined 1
INTDEF int libc_sched_yield(void);
#endif /* !__libc_sched_yield_defined */
INTDEF int libc_sched_getcpu(void);
INTDEF int libc_setns(int fd, int nstype);
INTDEF int libc_sched_setparam(pid_t pid, struct sched_param const *param);
INTDEF int libc_sched_getparam(pid_t pid, struct sched_param *param);
INTDEF int libc_sched_setscheduler(pid_t pid, int policy, struct sched_param const *param);
INTDEF int libc_sched_getscheduler(pid_t pid);
INTDEF int libc_sched_get_priority_max(int algorithm);
INTDEF int libc_sched_get_priority_min(int algorithm);
INTDEF int libc_sched_rr_get_interval(pid_t pid, struct timespec *t);
INTDEF int libc_sched_setaffinity(pid_t pid, size_t cpusetsize, __cpu_set_t const *cpuset);
INTDEF int libc_sched_getaffinity(pid_t pid, size_t cpusetsize, __cpu_set_t *cpuset);

DECL_END

#endif /* !GUARD_LIBS_LIBC_SCHED_H */
