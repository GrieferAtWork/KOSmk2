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
#ifndef GUARD_MODULES_FS_PROC_TASKUTIL_C
#define GUARD_MODULES_FS_PROC_TASKUTIL_C 1
#define _KOS_SOURCE 1

#include "taskutil.h"
#include <fs/fd.h>
#include <sched/task.h>
#include <errno.h>
#include <sched/cpu.h>
#include <sched/smp.h>

DECL_BEGIN

INTERN REF struct fdman *KCALL
task_getfdman(WEAK struct task *__restrict tsk) {
 REF struct fdman *result; pflag_t was; errno_t temp;
 if (!TASK_TRYINCREF(tsk)) return E_PTR(-ESRCH);
 was = PREEMPTION_PUSH();
#ifdef CONFIG_SMP
 if (ATOMIC_READ(tsk->t_cpu) != THIS_CPU) {
  assert(tsk != THIS_TASK);
  temp = task_suspend(tsk,TASK_SUSP_HOST);
  if (E_ISERR(temp)) return E_PTR(temp);
  result = ATOMIC_READ(tsk->t_fdman);
  /* NOTE: 't_fdman' may be NULL if the task was
   *       assigned a PID but is still being setup. */
  if (!result) result = E_PTR(-ESRCH);
  else FDMAN_INCREF(result);
  task_resume(tsk,TASK_SUSP_HOST);
 } else
#endif
 {
  /* Task on same CPU. -> With interrupts off,
   * we can simply read its fd-manager directly. */
  result = ATOMIC_READ(tsk->t_fdman);
  /* NOTE: Same reason as above... */
  if (!result) result = E_PTR(-ESRCH);
  else FDMAN_INCREF(result);
 }
 PREEMPTION_POP(was);
 TASK_DECREF(tsk);
 return result;
}


DECL_END

#endif /* !GUARD_MODULES_FS_PROC_TASKUTIL_C */