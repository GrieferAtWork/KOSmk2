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
#ifndef GUARD_INCLUDE_KERNEL_STACK_H
#define GUARD_INCLUDE_KERNEL_STACK_H 1

#include <assert.h>
#include <errno.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwptr.h>
#include <kernel/malloc.h>
#include <kernel/memory.h>
#include <hybrid/types.h>
#include <sched/task.h>

DECL_BEGIN


#define STACK_OFFSETOF_REFCNT     0
#define STACK_OFFSETOF_BRANCH     __SIZEOF_REF_T__
#define STACK_OFFSETOF_BEGIN   (2*__SIZEOF_REF_T__)
#define STACK_OFFSETOF_END     (2*__SIZEOF_REF_T__+__SIZEOF_POINTER__)
#define STACK_OFFSETOF_TASK    (2*__SIZEOF_REF_T__+2*__SIZEOF_POINTER__)
#define STACK_SIZE             (2*__SIZEOF_REF_T__+3*__SIZEOF_POINTER__)


/* Fully automatic stack controller, including
 * support for automatic growth by use of guard pages. */
struct task;
struct stack {
 ATOMIC_DATA ref_t s_refcnt; /*< [!0] Reference counter for the control structure. */
 ATOMIC_DATA ref_t s_branch; /*< Amount of branches mapped to this stack.
                              *  NOTE: This value is updated using the `mbranch_notity' facility.
                              *  NOTE: While non-zero, this counter holds a reference to `s_refcnt'. */
 /* NOTE: Locking the following two is a bit complicated:
  *       If the stack was created with a guard page, the caller must
  *       hold a read-lock on the associated mman ([lock(:struct mman::m_lock)])
  *       Otherwise, both members are [const].
  * WARNING: Neither case can guaranty that some part of the stack wasn't un/re-mapped,
  *          as `stack_mnotify' doesn't handle the associated events, not is this controller
  *          capable of representing a stack that's been split into multiple parts.
  *          Note though, that using the `struct stack' itself as closure, unmapping
  *          a stack is always a safe operation, in that no branches not mapped
  *          for the stack, or through guard access will ever be deleted.
  */
 VIRT ppage_t      s_begin;  /*< [1..1][valid_if(s_branch != 0)][<= s_end] Lowest memory address. */
 VIRT ppage_t      s_end;    /*< [1..1][valid_if(s_branch != 0)][>= s_end] First invalid memory address. */
 atomic_rwptr_t    s_task;   /*< [TYPE(struct task)][weak][0..1] The thread associated with this stack.
                              *   WARNING: This is a weak reference, meaning that `TASK_TRYINCREF'
                              *            must be used to acquire a reference.
                              *   NOTE: When non-NULL, this is the task that gets signaled with a STACK_OVERFLOW
                              *         error when guard pages run out of funds, or cannot be allocated.
                              */
};
#define STACK_INCREF(self) (void)(ATOMIC_FETCHINC((self)->s_refcnt))
#define STACK_DECREF(self) (void)(ATOMIC_DECFETCH((self)->s_refcnt) || (stack_destroy(self),0))

LOCAL void KCALL stack_destroy(struct stack *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(!self->s_branch);
 assert(!self->s_refcnt);
 kffree(self,GFP_NOFREQ);
}


/* Return the task associated with the given stack, or NULL if none is. */
LOCAL REF struct task *KCALL stack_task(struct stack *__restrict self);

/* mman notification used for tracking stack mappings, size & guard position in branches.
 * @param: closure: The `struct instance' object associated with the instance itself. */
FUNDEF ssize_t KCALL stack_mnotify(unsigned int type, void *__restrict closure,
                                   struct mman *mm, ppage_t addr, size_t size);


LOCAL REF struct task *KCALL
stack_task(struct stack *__restrict self) {
 REF struct task *result;
 atomic_rwptr_read(&self->s_task);
 result = (REF struct task *)ATOMIC_RWPTR_GET(self->s_task);
 /* Try to acquire a reference to the stored task.
  * If the reference counter is ZERO(0), the task is currently being destroyed,
  * during which the atomic R/W poitner will be locked for writing and be cleared. */
 if (result && !TASK_TRYINCREF(result)) result = NULL;
 atomic_rwptr_endread(&self->s_task);
 return result;
}


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_STACK_H */
