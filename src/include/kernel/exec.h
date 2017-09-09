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
#ifndef GUARD_INCLUDE_KERNEL_EXEC_H
#define GUARD_INCLUDE_KERNEL_EXEC_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

/* Create an empty user-space task that the caller then load & execute modules inside of.
 * NOTE: This function differs from 'task_new()' in that the
 *       following members have already been pre-initialized:
 *  - t_cstate (Set to point at 't_hstack.hs_end-sizeof(struct cpustate)')
 *  - t_affinity (Initialized to allow any CPU as host)
 *  - t_ustack (Initialized to NULL; If needed, the caller must allocate a user-stack)
 *  - t_hstack (Pre-initialized to 'TASK_HOSTSTACK_DEFAULTSIZE')
 *  - t_mman (Contains a new memory manager, or 'mm')
 *  - t_priority (Initialized to 'TASKPRIO_DEFAULT')
 * The caller must therefor only do the following:
 *  - Load code to-be executed into 't_mman'
 *  - Fill in members of 't_cstate' (arch-dependent)
 *  - Call 'task_start()' to schedule & start executing the task.
 * WARNING: Until 'task_start()' is called, the returned pointer is _NOT_ a valid
 *          task, meaning that 
 * @param: mm:  The memory manager to-be used by the thread, or NULL if a new one should be allocated.
 *              WARNING: It is illegal to pass '&mman_kernel' to this function.
 * @return: * :      A pointer to the newly allocated task.
 * @return: -ENOMEM: Not enough available memory.
 */
FUNDEF struct task *KCALL exec_mktask(struct mman *mm);
FUNDEF struct task *KCALL exec_dltask(struct mman *mm);


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_EXEC_H */
