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
#ifndef GUARD_KERNEL_CORE_STACK_C
#define GUARD_KERNEL_CORE_STACK_C 1

#include <assert.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/mman.h>
#include <kernel/stack.h>
#include <sched/percpu.h>
#include <kernel/user.h>
#include <sched/signal.h>
#include <bits/signum.h>
#include <kernel/syscall.h>
#include <kos/thread.h>

DECL_BEGIN

PUBLIC ssize_t KCALL
stack_mnotify(unsigned int type, void *__restrict closure,
              struct mman *UNUSED(mm), ppage_t addr, size_t size) {
#define STACK  ((struct stack *)closure)
 CHECK_HOST_DOBJ(STACK);
 assert(STACK->s_refcnt != 0);
 switch (type) {

 /* Increment/Decrement the branch counter of the given instance.
  * NOTE: The caller must be holding a write-lock on the associated memory manager! */
 case MNOTIFY_INCREF:
#ifdef CONFIG_DEBUG
  assert(STACK->s_branch != 0);
#endif
  ++STACK->s_branch;
  break;

 case MNOTIFY_DECREF:
  assert(STACK->s_branch);
  if (!--STACK->s_branch) {
   STACK_DECREF(STACK);
  }
  break;

 {
#ifndef CONFIG_NO_TLB
  REF struct task *t;
#endif /* !CONFIG_NO_TLB */
 case MNOTIFY_GUARD_ALLOCATE:
  /* Update the stack begin/end */
  if (STACK->s_begin > addr)
      STACK->s_begin = addr;
  *(uintptr_t *)&addr += size;
  if (STACK->s_end < addr)
      STACK->s_end = addr;

#ifndef CONFIG_NO_TLB
  if ((t = stack_task(STACK)) != NULL) {
   struct {
    void *hi,*lo;
   } stck;
   /* Update the user-space thread information block. */
   if (t->t_tlb != PAGE_ERROR) {
    stck.hi = STACK->s_end;
    stck.lo = STACK->s_begin;
    copy_to_user(&t->t_tlb->tl_tib.ti_stackhi,
                 &stck,2*sizeof(void *));
   }
   TASK_DECREF(t);
  }
#endif /* !CONFIG_NO_TLB */

 } break;

 {
  REF struct task *owner;
 case MNOTIFY_GUARD_INUSE:
 case MNOTIFY_GUARD_END:
  owner = stack_task(STACK);
  if (owner) {
   /* Signal a stack overflow. */
   siginfo_t info;
   info.si_signo    = SIGSEGV;
   info.si_code     = SEGV_MAPERR;
   info.si_addr     = THIS_SYSCALL_REAL_EIP;
   info.si_addr_lsb = 0;
   info.si_lower    = STACK->s_begin;
   info.si_upper    = STACK->s_end;
   task_kill2(owner,&info,0,0);
   TASK_DECREF(owner);
  }
 } break;

 case MNOTIFY_UNSHARE_DROP:
  /* Drop all stack mappings, but that of the
   * calling thread during un-sharing (aka. fork()). */
  if (STACK != THIS_TASK->t_ustack)
      return 1;
  break;


 default: break;
 }
 return -EOK;
}


DECL_END

#endif /* !GUARD_KERNEL_CORE_STACK_C */
