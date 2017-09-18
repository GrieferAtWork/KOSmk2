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
#ifndef GUARD_LIBS_LIBC_SCHED_C
#define GUARD_LIBS_LIBC_SCHED_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include "libc.h"
#include "system.h"

#include <errno.h>
#include <hybrid/compiler.h>
#include <sched.h>

DECL_BEGIN

PUBLIC int (LIBCCALL unshare)(int flags) {
 int result = sys_unshare(flags);
 if (E_ISERR(result)) { SET_ERRNO(-result); return -1; }
 return 0;
}
PUBLIC int (LIBCCALL clone)(int (LIBCCALL *fn)(void *arg),
                            void *child_stack, int flags,
                            void *arg, ...) {
 NOT_IMPLEMENTED();
 return -1;
}
PUBLIC int (LIBCCALL sched_yield)(void) { return sys_sched_yield(); }
PUBLIC int (LIBCCALL sched_getcpu)(void) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setns)(int fd, int nstype) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sched_setparam)(pid_t pid, struct sched_param const *param) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sched_getparam)(pid_t pid, struct sched_param *param) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sched_setscheduler)(pid_t pid, int policy, struct sched_param const *param) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sched_getscheduler)(pid_t pid) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sched_get_priority_max)(int algorithm) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sched_get_priority_min)(int algorithm) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sched_rr_get_interval)(pid_t pid, struct timespec *t) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sched_setaffinity)(pid_t pid, size_t cpusetsize, const cpu_set_t *cpuset) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sched_getaffinity)(pid_t pid, size_t cpusetsize, cpu_set_t *cpuset) { NOT_IMPLEMENTED(); return -1; }

DECL_END

#endif /* !GUARD_LIBS_LIBC_SCHED_C */
