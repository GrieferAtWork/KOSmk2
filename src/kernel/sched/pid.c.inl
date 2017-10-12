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
#ifndef GUARD_KERNEL_SCHED_PID_C_INL
#define GUARD_KERNEL_SCHED_PID_C_INL 1
#define _KOS_SOURCE 2

#include <assert.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/panic.h>
#include <hybrid/section.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/task.h>
#include <sched/types.h>

DECL_BEGIN

PUBLIC struct pid_namespace pid_global = {
    /* 3/4 initial references:
     *    - pid_global
     *    - inittask.t_pid.tp_ids[PIDTYPE_GPID].tl_ns
     *    - __bootcpu.c_idle.t_pid.tp_ids[PIDTYPE_GPID].tl_ns
     *    - [!CONFIG_NO_JOBS] __bootcpu.c_work.t_pid.tp_ids[PIDTYPE_GPID].tl_ns
     */
#ifndef CONFIG_NO_JOBS
#ifdef CONFIG_DEBUG
    .pn_refcnt = 4,
#else
    .pn_refcnt = 0x80000004,
#endif
#else /* !CONFIG_NO_JOBS */
#ifdef CONFIG_DEBUG
    .pn_refcnt = 3,
#else
    .pn_refcnt = 0x80000003,
#endif
#endif /* CONFIG_NO_JOBS */
    .pn_type   = PIDTYPE_GPID,
    .pn_min    = BOOSTRAP_PID_COUNT,
    .pn_max    = (1 << 16)-1,
    .pn_lock   = ATOMIC_RWLOCK_INIT,
    .pn_next   = BOOSTRAP_PID_COUNT,
    .pn_mapa   = BOOSTRAP_PID_COUNT,
    .pn_mapc   = BOOSTRAP_PID_COUNT,
    .pn_map    = NULL,
};
PUBLIC struct pid_namespace pid_init = {
#ifndef CONFIG_NO_JOBS
#ifdef CONFIG_DEBUG
    .pn_refcnt = 4,
#else
    .pn_refcnt = 0x80000004,
#endif
#else /* !CONFIG_NO_JOBS */
#ifdef CONFIG_DEBUG
    .pn_refcnt = 3,
#else
    .pn_refcnt = 0x80000003,
#endif
#endif /* CONFIG_NO_JOBS */
    .pn_type   = PIDTYPE_PID,
    .pn_min    = BOOSTRAP_PID_COUNT,
    .pn_max    = (1 << 16)-1,
    .pn_lock   = ATOMIC_RWLOCK_INIT,
    .pn_next   = BOOSTRAP_PID_COUNT,
    .pn_mapa   = BOOSTRAP_PID_COUNT,
    .pn_mapc   = BOOSTRAP_PID_COUNT,
    .pn_map    = NULL,
};

PUBLIC REF struct pid_namespace *
KCALL pid_namespace_new(pidtype_t type) {
 REF struct pid_namespace *result;
 assert(type < PIDTYPE_COUNT);
 result = omalloc(struct pid_namespace);
 if unlikely(!result) return NULL;
 result->pn_refcnt = 1;
 result->pn_type   = type;
 result->pn_min    = 1;
 result->pn_max    = (1 << 16)-1;
 atomic_rwlock_init(&result->pn_lock);
 result->pn_next   = 0;
 result->pn_mapa   = 0;
 result->pn_mapc   = 0;
 result->pn_map    = NULL;
 return result;
}

PUBLIC void KCALL
pid_namespace_destroy(struct pid_namespace *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(self != &pid_global);
 assert(self != &pid_init);
 assertf(self->pn_mapc == 0,"But any task should be holding a reference...");
 assert((self->pn_map != NULL) == (self->pn_mapa != 0));
 free(self->pn_map);
 free(self);
}


PUBLIC REF struct task *KCALL
pid_namespace_lookup(struct pid_namespace *__restrict self, pid_t id) {
 REF struct task *result = NULL;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->pn_lock);
 if likely(self->pn_mapa) {
  result = self->pn_map[id % self->pn_mapa].pb_chain;
  while (result && result->t_pid.tp_ids[self->pn_type].tl_pid != id)
         result = result->t_pid.tp_ids[self->pn_type].tl_link.le_next;
  if (result && !TASK_TRYINCREF(result)) result = NULL;
 }
 atomic_rwlock_endread(&self->pn_lock);
 return result;
}
PUBLIC WEAK REF struct task *KCALL
pid_namespace_lookup_weak(struct pid_namespace *__restrict self, pid_t id) {
 REF struct task *result = NULL;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->pn_lock);
 if likely(self->pn_mapa) {
  result = self->pn_map[id % self->pn_mapa].pb_chain;
  while (result && result->t_pid.tp_ids[self->pn_type].tl_pid != id)
         result = result->t_pid.tp_ids[self->pn_type].tl_link.le_next;
  /* Must only create a new weak reference. */
  if (result) TASK_WEAK_INCREF(result);
 }
 atomic_rwlock_endread(&self->pn_lock);
 return result;
}

PRIVATE ATTR_FREETEXT void KCALL
setup_pid_namespace(struct pid_namespace *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(self->pn_map == NULL);
 assert(self->pn_mapa == BOOSTRAP_PID_COUNT);
 assert(self->pn_mapc == BOOSTRAP_PID_COUNT);
 self->pn_map = _mall_untrack(tmalloc(struct pid_bucket,BOOSTRAP_PID_COUNT));
 if unlikely(!self->pn_map)
    PANIC("PID namespace setup failed: %[errno]",ENOMEM);
 /* Link the initial namespace entries. */
 self->pn_map[BOOTTASK_PID % BOOSTRAP_PID_COUNT].pb_chain     = &inittask;
 self->pn_map[BOOTCPU_IDLE_PID % BOOSTRAP_PID_COUNT].pb_chain = &__bootcpu.c_idle;
 inittask.t_pid.tp_ids[self->pn_type].tl_link.le_pself = &self->pn_map[BOOTTASK_PID % BOOSTRAP_PID_COUNT].pb_chain;
 __bootcpu.c_idle.t_pid.tp_ids[self->pn_type].tl_link.le_pself = &self->pn_map[BOOTCPU_IDLE_PID % BOOSTRAP_PID_COUNT].pb_chain;
#ifndef CONFIG_NO_JOBS
 self->pn_map[BOOTCPU_WORK_PID % BOOSTRAP_PID_COUNT].pb_chain = &__bootcpu.c_work;
 __bootcpu.c_work.t_pid.tp_ids[self->pn_type].tl_link.le_pself = &self->pn_map[BOOTCPU_WORK_PID % BOOSTRAP_PID_COUNT].pb_chain;
#endif
}


INTERN ATTR_FREETEXT void KCALL pid_initialize(void) {
 setup_pid_namespace(&pid_global);
 setup_pid_namespace(&pid_init);
}



DECL_END

#endif /* !GUARD_KERNEL_SCHED_PID_C_INL */
