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
#ifndef GUARD_MODULES_FS_PROC_TASKUTIL_H
#define GUARD_MODULES_FS_PROC_TASKUTIL_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

struct fdman;
struct mman;
struct task;

#define PROC_ROOT_NUMNODES 256 /* Max amount of misc. nodes under /proc */
#define PROC_PID_NUMNODES  256 /* Max amount of nodes under /proc/PID */

/* General purpose utilities to safely load various parts of a given
 * task, that would otherwise be considered 'PRIVATE(THIS_TASK)'.
 * NOTE: These functions all return E_ISERR(*) upon error; NULL is never returned. */
INTDEF REF struct fdman *KCALL task_getfdman(WEAK struct task *__restrict t);
INTDEF REF struct mman *KCALL task_getmman(WEAK struct task *__restrict t);
INTDEF REF struct instance *KCALL mman_getexe(struct mman *__restrict mm);

/* Get a child/group member of a given task, given its PID. */
INTDEF WEAK REF struct task *KCALL file_gettask_pid(WEAK struct task *__restrict leader, pid_t pid);
INTDEF WEAK REF struct task *KCALL file_getchild_pid(WEAK struct task *__restrict parent, pid_t pid);

/* Unambiguously parse the given string as a PID, as seen in /proc.
 * @return: -1: The given string isn't a PID. */
INTDEF pid_t KCALL pid_from_string(char const *__restrict str, size_t str_len);

#define INO_FROM_PID(pid) (PROC_ROOT_NUMNODES+(pid)*PROC_PID_NUMNODES)


DECL_END

#endif /* !GUARD_MODULES_FS_PROC_TASKUTIL_H */
