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
#ifndef GUARD_KERNEL_SCHED_SIGNAL_C
#define GUARD_KERNEL_SCHED_SIGNAL_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <asm/unistd.h>
#include <bits/signum.h>
#include <bits/waitstatus.h>
#include <errno.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/traceback.h>
#include <kernel/arch/gdt.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <kos/syslog.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <sched/types.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/ucontext.h>

DECL_BEGIN

PUBLIC struct sigshare sigshare_kernel = {
    /* Reference counter explaination:
     *   - sigshare_kernel
     *   - inittask.t_sigshare
     *   - __bootcpu.c_idle.t_sigshare
     */
#ifdef CONFIG_DEBUG
    .ss_refcnt = 3,
#else
    .ss_refcnt = 0x80000003,
#endif
    .ss_pending = SIGPENDING_INIT,
};

PUBLIC REF struct sigshare *KCALL sigshare_new(void) {
 REF struct sigshare *result;
 result = ocalloc(REF struct sigshare);
 if unlikely(!result) return NULL;
 result->ss_refcnt = 1;
 sigpending_cinit(&result->ss_pending);
 return result;
}

PUBLIC void KCALL
sigshare_destroy(struct sigshare *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(!self->ss_refcnt);
 sigpending_fini(&self->ss_pending);
 free(self);
}
PUBLIC void KCALL
sigpending_fini(struct sigpending *__restrict self) {
 struct sigqueue *iter,*next;
 CHECK_HOST_DOBJ(self);
 iter = self->sp_queue;
 while (iter) {
  next = iter->sq_chain.le_next;
  free(iter);
  iter = next;
 }
}

PUBLIC errno_t KCALL
sigpending_enqueue_unlocked(struct sigpending *__restrict self,
                            siginfo_t const *__restrict signal_info) {
 struct sigqueue *entry;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(signal_info);
 assert(sigpending_writing(self));
 assert(signal_info->si_signo > 0 &&
        signal_info->si_signo < _NSIG);
 entry = omalloc(struct sigqueue);
 if unlikely(!entry) return -ENOMEM;
 /* Fill in the new queue entry and add it to the chain. */
 memcpy(&entry->sq_info,signal_info,sizeof(siginfo_t));
 SLIST_INSERT(self->sp_queue,entry,sq_chain);
 /* Mark the signal as pending in this controller. */
 __sigaddset(&self->sp_mask,signal_info->si_signo);
 return -EOK;
}
PUBLIC errno_t KCALL
sigpending_enqueue(struct sigpending *__restrict self,
                   siginfo_t const *__restrict signal_info) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 sigpending_write(self);
 error = sigpending_enqueue_unlocked(self,signal_info);
 /* Broadcast upon success. */
 if (E_ISOK(error)) sig_broadcast_unlocked(&self->sp_newsig);
 sigpending_endwrite(self);
 return error;
}

PUBLIC size_t KCALL
sigpending_discard(struct sigpending *__restrict self, int signo) {
 bool has_write_lock = false;
 size_t result = 0;
 struct sigqueue **piter,*iter;
 CHECK_HOST_DOBJ(self);
 sigpending_read(self);
 assert(signo > 0 && signo < _NSIG);
check_again:
 if (!sigismember(&self->sp_mask,signo)) goto end;
 if (!has_write_lock) {
  has_write_lock = true;
  if (!sigpending_upgrade(self))
       goto check_again;
 }
 /* Unset the signal in the pending mask. */
 sigdelset(&self->sp_mask,signo);
 piter = &self->sp_queue;
 while ((iter = *piter) != NULL) {
  if (iter->sq_info.si_signo == signo) {
   /* Found one! */
   *piter = iter->sq_chain.le_next;
   free(iter); /* Simply delete this entry. */
   ++result;
  } else {
   piter = &iter->sq_chain.le_next;
  }
 }
 /* Make sure to consider the possibility of an undefined signal. */
 if (!result) result = 1;
end:
 if (has_write_lock)
      sigpending_endwrite(self);
 else sigpending_endread(self);
 return result;
}


PUBLIC /*inherit*/struct sigqueue *KCALL
sigpending_try_dequeue_unlocked(struct sigpending *__restrict self,
                                sigset_t const *__restrict deque_mask,
                                bool *__restrict has_write_lock) {
 u8 *iter,*end,*right; int i;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(deque_mask);
 CHECK_HOST_DOBJ(has_write_lock);
 assert(*has_write_lock ? sigpending_writing(self)
                        : sigpending_reading(self));
 end = (iter = (u8 *)&self->sp_mask)+sizeof(sigset_t);
 right = (u8 *)deque_mask;
 for (; iter != end; ++iter,++right) {
  if (*iter & *right) {
   /* Figure out where exactly the match occurred. */
   for (i = 0;; ++i) {
    assert(i < 8);
    if ((*iter  & (1 << i)) &&
        (*right & (1 << i)))
        break;
   }
   i += ((right-(u8 *)deque_mask)*8)+1;
   /* Now search for the associated pending entry.
    * NOTE: It may not exist if the signal was send without data. */
   {
    struct sigqueue **pfirst,**piter,*iter;
    pfirst = NULL;
    piter = &self->sp_queue;
    while ((iter = *piter) != NULL) {
     if (iter->sq_info.si_signo == i) {
      /* Found a match!
       * >> This means we'll have to try and remove it,
       *    also meaning that we need write-access. */
      if (!*has_write_lock) {
       *has_write_lock = true;
       if (!sigpending_upgrade(self))
            return E_PTR(-ERELOAD);
      }
      if (pfirst) goto remove_non_last;
      pfirst = piter;
     }
     piter = &iter->sq_chain.le_next;
    }
    /* Remove the signal set entry for a last/undefined match. */
    sigdelset(&self->sp_mask,i);

    /* With no associated entry, return an undefined queue entry. */
    if (!pfirst) return SIGPENDING_UNDEFINED(i);

    /* If we didn't find secondary entries, this was the last,
     * meaning we have to delete the  */

remove_non_last:
    CHECK_HOST_DOBJ(pfirst);
    CHECK_HOST_DOBJ(*pfirst);
    /* Unlink the first entry we've found. */
    iter = *pfirst;
    *pfirst = iter->sq_chain.le_next;
    return iter;
   }
  }
 }
 /* Nothing was found... */
 return NULL;
}












PUBLIC struct sighand sighand_kernel = {
    /* Reference counter explanation:
     *   - sighand_kernel
     *   - inittask.t_sighand
     *   - __bootcpu.c_idle.t_sighand
     */
#ifdef CONFIG_DEBUG
    .sh_refcnt = 3,
#else
    .sh_refcnt = 0x80000003,
#endif
    .sh_actions = {
    },
};

PUBLIC void KCALL
sighand_destroy(struct sighand *__restrict self) {
 CHECK_HOST_DOBJ(self);
 free(self);
}


PUBLIC REF struct sighand *KCALL sighand_new(void) {
 REF struct sighand *result;
 result = ocalloc(REF struct sighand);
 if unlikely(!result) return NULL;
 result->sh_refcnt = 1;
 /* NOTE: At this point, all signals have already
  *       been pre-initialized to 'SIG_DFL' */
 return result;
}
PUBLIC REF struct sighand *KCALL
sighand_copy(struct sighand *__restrict self) {
 REF struct sighand *result;
 CHECK_HOST_DOBJ(self);
 result = (REF struct sighand *)memdup(self,sizeof(struct sighand));
 if (result) result->sh_refcnt = 1;
 return result;
}
PUBLIC void KCALL
sighand_reset(struct sighand *__restrict self) {
 CHECK_HOST_DOBJ(self);
 /* As simple as that! */
 memset(&self->sh_actions,0,sizeof(self->sh_actions));
}






















STATIC_ASSERT(sizeof(siginfo_t)                      <= __SI_MAX_SIZE);
STATIC_ASSERT(sizeof(ucontext_t)                     == __UCONTEXT_SIZE);
STATIC_ASSERT(sizeof(struct sigenter_info)           == SIGENTER_INFO_SIZE);
STATIC_ASSERT(offsetof(struct sigenter_info,ei_info) == SIGENTER_INFO_OFFSETOF_INFO);
STATIC_ASSERT(offsetof(struct sigenter_info,ei_ctx)  == SIGENTER_INFO_OFFSETOF_CTX);

/* Exception handler for managing broken signal stacks. */
INTERN ATTR_NORETURN void KCALL sigfault(void) {
 assert(!THIS_TASK->t_critical);
 syslogf(LOG_ERROR,"[SIG] Terminating thread %d:%d with faulty signal stack pointer at %p\n",
         GET_THIS_PID(),GET_THIS_TID(),THIS_TASK->t_lastcr2);
 task_terminate(THIS_TASK,(void *)(__WCOREFLAG|__W_EXITCODE(1,0)));
 __builtin_unreachable();
}
INTERN ATTR_NORETURN void KCALL sigill(char const *__restrict format, ...) {
 va_list args;
 assert(!THIS_TASK->t_critical);
 syslogf(LOG_ERROR,"[SIG] Terminating thread %d:%d with illegal signal context: Invalid ",
         GET_THIS_PID(),GET_THIS_TID());
 va_start(args,format);
 vsyslogf(LOG_DEBUG,format,args);
 va_end(args);
 syslogf(LOG_ERROR,"\n");
 task_terminate(THIS_TASK,(void *)(__WCOREFLAG|__W_EXITCODE(2,0)));
 __builtin_unreachable();
}

FUNDEF ATTR_NORETURN void ASMCALL sigenter(void);
STATIC_ASSERT(offsetof(struct task,t_sigenter.se_count)   == TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_COUNT);
STATIC_ASSERT(offsetof(struct task,t_sigenter.se_useresp) == TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_USERESP);

GLOBAL_ASM(
L(.section .text                                                                 )
L(PUBLIC_ENTRY(sigenter)                                                         )
      /* Having gotten here, out stack is currently undefined, but we know that
       * we're still in kernel-space and that accessing per-cpu data will be OK.
       * In addition, all main registers have been restored to what they're ought
       * to be once we enventually do return to user-space. */
L(    pushl %eax                                                                 )
L(    pushl %ebx                                                                 )
L(    pushl %ecx                                                                 )
L(    movl ASM_CPU(CPU_OFFSETOF_RUNNING), %eax                                   )
L(                                                                               )
      /* Load the address of the user-space stack containing the signal-enter
       * descriptor, as well as how many recursive signal contexts we must fill in. */
L(    movl  (TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_USERESP)(%eax), %ebx       )
L(    movl  (TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_COUNT)(%eax), %ecx         )
L(    movl  $0, (TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_COUNT)(%eax)           )
#ifdef CONFIG_DEBUG
L(    testl %ecx, %ecx                                                           )
L(    jnz   1f                                                                   )
L(    int   $3 /* ASSERTION_FAILURE: 'Signal recursion was ZERO(0)' */           )
L(2:  hlt                                                                        )
L(    jmp 2b                                                                     )
L(1:                                                                             )
#endif
L(                                                                               )
      /* As we're about to start working with the user-space stack, filling in
       * remaining register data may cause a segfault which we must handle. */
L(    pushl $sigfault                                                            )
L(    pushl $(EXC_PAGE_FAULT)                                                    )
L(    pushl $0  /*TASK_OFFSETOF_IC(%eax) -- There mustn't be any more */         )
L(    movl  %esp, TASK_OFFSETOF_IC(%eax)                                         )
L(                                                                               )
      /* Fill in all the missing user-space registers. */
#define REG(i) (SIGENTER_INFO_OFFSETOF_CTX + \
                   __UCONTEXT_OFFSETOF_MCONTEXT + \
                   __MCONTEXT_OFFSETOF_GREGS+(i)*4)(%ebx)
L(1:  pushl %edi                                                                 )
L(    movl  %ebx, %edi                                                           )
L(    addl  $(__UCONTEXT_SIZE), %edi                                             )
L(    jo    9f                                                                   )
L(    cmpl  $(KERNEL_BASE), %edi                                                 )
L(    ja    10f                                                                  )
L(    popl  %edi                                                                 )
L(    movw  %gs,  REG(REG_GS)                                                    )
L(    movw  %fs,  REG(REG_FS)                                                    )
L(    movw  %es,  REG(REG_ES)                                                    )
L(    movw  %ds,  REG(REG_DS)                                                    )
L(    movl  %edi, REG(REG_EDI)                                                   )
L(    movl  %esi, REG(REG_ESI)                                                   )
L(    movl  %ebp, REG(REG_EBP)                                                   )
L(    movl  %edx, REG(REG_EDX)                                                   )
L(    movl  12(%esp), %edx  /* ECX */                                            )
L(    movl  %edx, REG(REG_ECX)                                                   )
L(    movl  16(%esp), %edx  /* EBX */                                            )
L(    movl  %edx, REG(REG_EBX)                                                   )
L(    movl  20(%esp), %edx  /* EAX */                                            )
L(    movl  %edx, REG(REG_EAX)                                                   )
      /* Already filled: REG_EIP */
      /* Already filled: REG_CS */
      /* Already filled: REG_EFL */
      /* Already filled: REG_UESP */
      /* Already filled: REG_SS */
L(    movl  %ebp, SIGENTER_INFO_OFFSETOF_OLD_EBP(%ebx)                           )
L(    subl  $1, %ecx                                                             )
L(    jz    2f                                                                   )
      /* Recursively fill secondary context structures. */
L(    movl  REG(REG_UESP), %ebx                                                  )
L(    jmp   1b                                                                   )
#undef REG                   
L(2:  movl  $0, TASK_OFFSETOF_IC(%eax)                                           )
      /* At this point, all context structures are in a valid state.
       * >> Now it's time to setup the last jump to the first signal handler! */
#define SIGENTER(x) (TASK_OFFSETOF_SIGENTER+x)(%eax)
L(    pushl SIGENTER(SIGENTER_OFFSETOF_SS)                                       )
L(    pushl SIGENTER(SIGENTER_OFFSETOF_USERESP)                                  )
L(    pushl SIGENTER(SIGENTER_OFFSETOF_EFLAGS)                                   )
L(    pushl SIGENTER(SIGENTER_OFFSETOF_CS)                                       )
L(    pushl SIGENTER(SIGENTER_OFFSETOF_EIP)                                      )
#undef SIGENTER
      /* To prevent kernel data leaks, we must re-initialize all registers we've modified.
       * Also: Store a pointer to 'ei_old_ebp' in EBP to fix user-space tracebacks. */
L(    movl  (TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_USERESP)(%eax), %ebx       )
L(    leal  SIGENTER_INFO_OFFSETOF_OLD_EBP(%ebx), %ebp                           )
L(    movl  40(%esp), %eax                                                       )
L(    movl  36(%esp), %ebx                                                       )
L(    movl  32(%esp), %ecx                                                       )
L(                                                                               )
      /* Now just jump back to user-space. */
L(    iret                                                                       )
L(                                                                               )
      /* Handle illegal-user-stack errors. */
L(9:  movl %ebx, TASK_OFFSETOF_LASTCR2(%eax)                                     )
L(    call sigfault                                                              )
L(10: movl %edi, TASK_OFFSETOF_LASTCR2(%eax)                                     )
L(    call sigfault                                                              )
L(SYM_END(sigenter)                                                              )
L(.previous                                                                      )
);








/* Signal default actions. */
typedef u8 dact_t; enum{
 DA_TERM, /* Terminate application immediatly. */
 DA_CORE, /* Terminate + generate core dump. */
 DA_IGN,  /* Ignore signal. */
 DA_STOP, /* Suspend application. */
 DA_CONT, /* Continue application. */
};

PRIVATE dact_t const default_actions[_NSIG-1] = {
#define ENTRY(i) [i-1]
    ENTRY(SIGHUP)    = DA_TERM,
    ENTRY(SIGINT)    = DA_TERM,
    ENTRY(SIGQUIT)   = DA_CORE,
    ENTRY(SIGILL)    = DA_CORE,
    ENTRY(SIGABRT)   = DA_CORE,
    ENTRY(SIGFPE)    = DA_CORE,
    ENTRY(SIGKILL)   = DA_TERM,
    ENTRY(SIGSEGV)   = DA_CORE,
    ENTRY(SIGPIPE)   = DA_TERM,
    ENTRY(SIGALRM)   = DA_TERM,
    ENTRY(SIGTERM)   = DA_TERM,
    ENTRY(SIGUSR1)   = DA_TERM,
    ENTRY(SIGUSR2)   = DA_TERM,
    ENTRY(SIGCHLD)   = DA_IGN,
    ENTRY(SIGCONT)   = DA_CONT,
    ENTRY(SIGSTOP)   = DA_STOP,
    ENTRY(SIGTSTP)   = DA_STOP,
    ENTRY(SIGTTIN)   = DA_STOP,
    ENTRY(SIGTTOU)   = DA_STOP,
    ENTRY(SIGBUS)    = DA_CORE,
    ENTRY(SIGPOLL)   = DA_TERM,
    ENTRY(SIGPROF)   = DA_TERM,
    ENTRY(SIGSYS)    = DA_CORE,
    ENTRY(SIGTRAP)   = DA_CORE,
    ENTRY(SIGURG)    = DA_IGN,
    ENTRY(SIGVTALRM) = DA_TERM,
    ENTRY(SIGXCPU)   = DA_CORE,
    ENTRY(SIGXFSZ)   = DA_CORE,
#if defined(SIGIOT) && SIGIOT != SIGABRT
    ENTRY(SIGIOT)    = DA_CORE,
#endif
#ifdef SIGEMT
    ENTRY(SIGEMT)    = DA_TERM,
#endif
    ENTRY(SIGSTKFLT) = DA_TERM,
#if defined(SIGIO) && SIGIO != SIGPOLL
    ENTRY(SIGIO)     = DA_TERM,
#endif
#if defined(SIGCLD) && SIGCLD != SIGCHLD
    ENTRY(SIGCLD)    = DA_IGN,
#endif
    ENTRY(SIGPWR)    = DA_TERM,
#ifdef SIGLOST
    ENTRY(SIGLOST)   = DA_TERM,
#endif
    ENTRY(SIGWINCH)  = DA_IGN,
#if defined(SIGUNUSED) && SIGUNUSED != SIGSYS
    ENTRY(SIGUNUSED) = DA_CORE,
#endif
#undef ENTRY
};

INTDEF void (ASMCALL signal_return)(void);


PRIVATE errno_t KCALL
deliver_signal_to_task_in_user(struct task *__restrict t,
                               struct sigaction const *__restrict action,
                               siginfo_t const *__restrict signal_info,
                               greg_t reg_trapno, greg_t reg_err) {
 struct cpustate *cs_descr;
 USER struct sigenter_info *user_info;
 struct sigenter_info info;
 /* This is some thread that was pre-empted while in user-space.
  * >> Therefor, 't_cstate' describes the CPU state before it was pre-empted. */
 cs_descr = t->t_cstate;

 /* TODO: Use sigaltstack() here, if it was ever set! */
 user_info = ((USER struct sigenter_info *)cs_descr->useresp)-1;

#ifdef __i386__
 info.ei_ctx.uc_flags = 0; /* ??? */
 info.ei_ctx.uc_link  = NULL;
 if (t->t_ustack) {
  info.ei_ctx.uc_stack.ss_sp    = (void *)t->t_ustack->s_begin;
  info.ei_ctx.uc_stack.ss_size  = ((uintptr_t)t->t_ustack->s_end-
                                (uintptr_t)t->t_ustack->s_begin);
  info.ei_ctx.uc_stack.ss_flags = 0;
 } else {
  info.ei_ctx.uc_stack.ss_sp    = (void *)cs_descr->useresp;
  info.ei_ctx.uc_stack.ss_size  = 0;
  info.ei_ctx.uc_stack.ss_flags = 0;
 }
 info.ei_ctx.uc_mcontext.cr2     = (unsigned long int)t->t_lastcr2;
 info.ei_ctx.uc_mcontext.oldmask = 0; /* ??? */
 info.ei_ctx.uc_mcontext.gregs[REG_GS] = (u32)cs_descr->host.gs;
 info.ei_ctx.uc_mcontext.gregs[REG_FS] = (u32)cs_descr->host.fs;
 info.ei_ctx.uc_mcontext.gregs[REG_ES] = (u32)cs_descr->host.es;
 info.ei_ctx.uc_mcontext.gregs[REG_DS] = (u32)cs_descr->host.ds;
 info.ei_ctx.uc_mcontext.gregs[REG_EDI] = cs_descr->host.edi;
 info.ei_ctx.uc_mcontext.gregs[REG_ESI] = cs_descr->host.esi;
 info.ei_ctx.uc_mcontext.gregs[REG_EBP] = cs_descr->host.ebp;
 info.ei_ctx.uc_mcontext.gregs[REG_ESP] = cs_descr->host.esp;
 info.ei_ctx.uc_mcontext.gregs[REG_EBX] = cs_descr->host.ebx;
 info.ei_ctx.uc_mcontext.gregs[REG_EDX] = cs_descr->host.edx;
 info.ei_ctx.uc_mcontext.gregs[REG_ECX] = cs_descr->host.ecx;
 info.ei_ctx.uc_mcontext.gregs[REG_EAX] = cs_descr->host.eax;
 info.ei_ctx.uc_mcontext.gregs[REG_TRAPNO] = reg_trapno;
 info.ei_ctx.uc_mcontext.gregs[REG_ERR] = reg_err;
 info.ei_ctx.uc_mcontext.gregs[REG_EIP] = cs_descr->host.eip;
 info.ei_ctx.uc_mcontext.gregs[REG_CS] = cs_descr->host.cs;
 info.ei_ctx.uc_mcontext.gregs[REG_EFL] = cs_descr->host.eflags;
 info.ei_ctx.uc_mcontext.gregs[REG_UESP] = cs_descr->useresp;
 info.ei_ctx.uc_mcontext.gregs[REG_SS] = cs_descr->ss;
 info.ei_ctx.uc_mcontext.fpregs = &user_info->ei_ctx.__fpregs_mem;
#else
#error FIXME
#endif

 memset(&info.ei_ctx.__fpregs_mem,0,sizeof(struct _libc_fpstate)); /* TODO */
 memcpy(&info.ei_ctx.uc_sigmask,&t->t_sigblock,sizeof(sigset_t));

 memset((byte_t *)&info.__ei_info_pad+sizeof(siginfo_t),0,__SI_MAX_SIZE-sizeof(siginfo_t));
 memcpy(&info.ei_info,signal_info,sizeof(siginfo_t));
 info.ei_return  = (USER void *)&signal_return;
 info.ei_signo   = signal_info->si_signo;
 info.ei_pinfo   = &user_info->ei_info;
 info.ei_pctx    = &user_info->ei_ctx;
 info.ei_old_ebp = (USER void *)cs_descr->host.ebp;
 info.ei_old_eip = (USER void *)cs_descr->host.eip;
 /* Copy all the information we've gathered onto the user-space stack. */
 { struct mman *omm;
   size_t copy_error;
   TASK_PDIR_BEGIN(omm,t->t_real_mman);
   copy_error = copy_to_user(user_info,&info,sizeof(info));
   TASK_PDIR_END(omm,t->t_real_mman);
   if unlikely(copy_error) {
    syslogf(LOG_WARN,
            "[SIG] Failed to deliver signal to process %d/%d: Target stack at %p is faulty\n",
           (int)t->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
           (int)t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,user_info);
    return -EFAULT;
   }
 }

 /* Setup the register state to-be used when the task will execute.
  * NOTE: We keep all registers but EBP, EIP and ESP
  *       as they were before the interrupt occurred. */
 cs_descr->host.ebp = (u32)&user_info->ei_old_ebp;
 cs_descr->host.eip = (u32)action->sa_handler;
 cs_descr->useresp  = (u32)user_info;

 /* Interrupt (wake) the task, so-as to execute the signal handler. */
 return -EOK;
}





PRIVATE errno_t KCALL
deliver_signal_to_task_in_host(struct task *__restrict t,
                               struct sigaction const *__restrict action,
                               siginfo_t const *__restrict signal_info,
                               greg_t reg_trapno, greg_t reg_err) {
 struct syscall_descr *ss_descr;
 USER struct sigenter_info *user_info;
 struct sigenter_info info;

#if 0
 if (t != THIS_TASK) {
  __assertion_tbprintl((void *)t->t_cstate->host.eip,
                       (void *)t->t_cstate->host.ebp,0);
 }
#endif

 /* This is us, or the task was pre-empted while in kernel-space. */
 ss_descr = (struct syscall_descr *)t->t_hstack.hs_end-1;

 /* Copy the original, unmodified IRET tail. */
 if (++t->t_sigenter.se_count == 1) {
  memcpy(&t->t_sigenter.se_eip,&ss_descr->eip,20);
  /* Setup the register state to not return to user-space, but to 'sigenter()' instead. */
#if 1
  syslogf(LOG_DEBUG,"Override system call return EIP %p with sigenter %p\n",
          ss_descr->eip,&sigenter);
#endif
  ss_descr->eip    = (u32)&sigenter;
  ss_descr->cs     = SEG(SEG_KERNEL_CODE);
  ss_descr->eflags = EFLAGS_IF|EFLAGS_IOPL(3);
  /* TODO: Use sigaltstack() here, if it was ever set! */
  user_info = ((USER struct sigenter_info *)ss_descr->useresp)-1;
 } else {
  /* Allocate an additional entry on the signal stack already in use. */
  assert(ss_descr->eip == (u32)&sigenter);
  user_info = ((USER struct sigenter_info *)t->t_sigenter.se_useresp)-1;
 }


#ifdef __i386__
 info.ei_ctx.uc_flags = 0; /* ??? */
 info.ei_ctx.uc_link  = NULL;
 if (t->t_ustack) {
  info.ei_ctx.uc_stack.ss_sp    = (void *)t->t_ustack->s_begin;
  info.ei_ctx.uc_stack.ss_size  = ((uintptr_t)t->t_ustack->s_end-
                                (uintptr_t)t->t_ustack->s_begin);
  info.ei_ctx.uc_stack.ss_flags = 0;
 } else {
  info.ei_ctx.uc_stack.ss_sp    = (void *)user_info;
  info.ei_ctx.uc_stack.ss_size  = 0;
  info.ei_ctx.uc_stack.ss_flags = 0;
 }
 info.ei_ctx.uc_mcontext.cr2     = (unsigned long int)t->t_lastcr2;
 info.ei_ctx.uc_mcontext.oldmask = 0; /* ??? */
 info.ei_ctx.uc_mcontext.gregs[REG_GS] = 0; /* Clear upper 16 bit. */
 info.ei_ctx.uc_mcontext.gregs[REG_FS] = 0; /* *ditto* */
 info.ei_ctx.uc_mcontext.gregs[REG_ES] = 0; /* *ditto* */
 info.ei_ctx.uc_mcontext.gregs[REG_DS] = 0; /* *ditto* */
#if 0
 info.ei_ctx.uc_mcontext.gregs[REG_EDI] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_ESI] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EBP] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EBX] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EDX] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_ECX] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EAX] = /* Filled later... */;
#endif
 info.ei_ctx.uc_mcontext.gregs[REG_ESP]    = t->t_sigenter.se_useresp;
 info.ei_ctx.uc_mcontext.gregs[REG_EIP]    = t->t_sigenter.se_eip;
 info.ei_ctx.uc_mcontext.gregs[REG_CS]     = t->t_sigenter.se_cs;
 info.ei_ctx.uc_mcontext.gregs[REG_EFL]    = t->t_sigenter.se_eflags;
 info.ei_ctx.uc_mcontext.gregs[REG_UESP]   = t->t_sigenter.se_useresp;
 info.ei_ctx.uc_mcontext.gregs[REG_SS]     = t->t_sigenter.se_ss;
 info.ei_ctx.uc_mcontext.gregs[REG_TRAPNO] = reg_trapno;
 info.ei_ctx.uc_mcontext.gregs[REG_ERR]    = reg_err;
 info.ei_ctx.uc_mcontext.fpregs = &user_info->ei_ctx.__fpregs_mem;
 info.ei_old_eip = (USER void *)t->t_sigenter.se_eip;
 memset(&info.ei_ctx.__fpregs_mem,0,sizeof(struct _libc_fpstate)); /* TODO */
 memcpy(&info.ei_ctx.uc_sigmask,&t->t_sigblock,sizeof(sigset_t));

 /* Fixup execution of the signal handler itself. */
 t->t_sigenter.se_useresp = (u32)user_info;
 t->t_sigenter.se_eip     = (u32)action->sa_handler;

#else
#error FIXME
#endif
#if 0 /* TODO: Restart system call? */
 if (action->sa_flags&SA_RESTART) {
 }
#endif

 memset((byte_t *)&info.__ei_info_pad+sizeof(siginfo_t),0,__SI_MAX_SIZE-sizeof(siginfo_t));
 memcpy(&info.ei_info,signal_info,sizeof(siginfo_t));
 info.ei_return = (USER void *)&signal_return;
 info.ei_signo  = signal_info->si_signo;
 info.ei_pinfo  = &user_info->ei_info;
 info.ei_pctx   = &user_info->ei_ctx;
 /* Copy all the information we've gathered onto the user-space stack. */
 { struct mman *omm;
   size_t copy_error;
   TASK_PDIR_BEGIN(omm,t->t_real_mman);
   copy_error = copy_to_user(user_info,&info,sizeof(info));
   TASK_PDIR_END(omm,t->t_real_mman);
   if unlikely(copy_error) {
    syslogf(LOG_WARN,
            "[SIG] Failed to deliver signal to process %d/%d: Target stack at %p is faulty\n",
           (int)t->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
           (int)t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,user_info);
    return -EFAULT;
   }
 }

 return -EOK;
}



#define SIGSET_WORDS (__SIZEOF_SIGSET_T__ / SIGWORD_SIZE)
#if (__SIZEOF_SIGSET_T__ % 4) == 0
typedef u32 sigword_t;
#define SIGWORD_SIZE 4
#elif (__SIZEOF_SIGSET_T__ % 2) == 0
typedef u16 sigword_t;
#define SIGWORD_SIZE 2
#else
typedef u8  sigword_t;
#define SIGWORD_SIZE 1
#endif


PUBLIC errno_t KCALL
task_kill2_cpu_endwrite(struct task *__restrict t,
                        siginfo_t const *__restrict signal_info,
                        greg_t reg_trapno, greg_t reg_err,
                        pflag_t was) {
 errno_t error = -EOK; int signo;
 struct cpu *c;
 CHECK_HOST_DOBJ(t);
 CHECK_HOST_DOBJ(signal_info);
 c = TASK_CPU(t);
 CHECK_HOST_DOBJ(c);
 assert(cpu_writing(c));
 assert(!PREEMPTION_ENABLED());
 signo = signal_info->si_signo;
 if (signo <= 0 || --signo >= (_NSIG-1)) { error = -EINVAL; goto done; }
 if (signo == SIGKILL || signo == SIGSTOP)
     goto do_term;
#ifdef CONFIG_SMP
 if (c != THIS_CPU) {
  /* TODO: Send IPC communication. */
 } else
#endif
 {
  struct sigaction *action;
  struct sighand *hand;
  /* Check if the signal is currently being blocked. */
  if (sigismember(&t->t_sigblock,signo+1)) {
   /* Enqueue the signal to be send at a later time. */
   cpu_endwrite(c);
   PREEMPTION_POP(was);
   return sigpending_enqueue(&t->t_sigpend,signal_info);
  }
  syslogf(LOG_DEBUG,"[SIG] kill(%d,%d) %p -> %p\n",
         (int)t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,signo,
          THIS_TASK,t);
#if 0
  __assertion_tbprint(0);
#endif

  /* NOTE: We know the task is running on our CPU, and with interrupts disabled
   *       we are either that task, or know that it won't be running until we've
   *       re-enabled interrupt, meaning we can full access to 't_sighand', knowing
   *       it will not be exchanged until we're done here.
   * >> In either case, the task isn't actually active in
   *    user-space, and won't randomly change into being! */
  hand = t->t_sighand;
  CHECK_HOST_DOBJ(hand);
  action = &hand->sh_actions[signo];
  if (action->sa_handler == SIG_DFL) {
   switch (default_actions[signo]) {

   case DA_IGN:
    goto ignore_signal;

   case DA_STOP:
    return task_suspend_now_cpu_endwrite(t,was);

   case DA_CONT:
    return task_resume_now_cpu_endwrite(t,was);

   default:
    goto do_term;

   }
  }
  if (action->sa_handler == SIG_IGN) {
   /* Nothing to do here... */
   /* XXX: What about exception signals?
    *      We should probably still do something about those... */
ignore_signal:;
  } else {
   sigword_t *dst,*end,*src;

   if (t != THIS_TASK && (t->t_cstate->host.cs&3) == 3) {
    /* The task isn't active, and currently running in user-space. */
    error = deliver_signal_to_task_in_user(t,action,signal_info,reg_trapno,reg_err);
   } else {
    error = deliver_signal_to_task_in_host(t,action,signal_info,reg_trapno,reg_err);
   }
   /* Mask all blocked signals in this task */
   end = (dst = (sigword_t *)&t->t_sigblock)+SIGSET_WORDS;
   src = (sigword_t *)&action->sa_mask;
   for (; dst != end; ++dst,++src) *dst |= *src;

   /* Delete the handler before it will be executed. */
   if (E_ISOK(error) && action->sa_flags&SA_RESETHAND)
       memset(action,0,sizeof(struct sigaction));
  }
  /* Interrupt (wake) the task, so-as to execute the signal handler. */
  if (E_ISOK(error)) { error = task_interrupt_cpu_endwrite(t); goto ppop_end; }
 }
done:
 cpu_endwrite(c);
ppop_end:
 PREEMPTION_POP(was);
 return error;
do_term:
 error = task_terminate_cpu_endwrite(c,t,(void *)__W_EXITCODE(0,signal_info->si_signo+1));
 goto ppop_end;
}

PUBLIC errno_t KCALL
task_kill2(struct task *__restrict t,
           siginfo_t const *__restrict signal_info,
           greg_t reg_trapno, greg_t reg_err) {
 struct cpu *c; pflag_t was; errno_t error;
 CHECK_HOST_DOBJ(t);
 task_crit();
 was = PREEMPTION_PUSH();
 for (;;) {
  c = TASK_CPU(t);
  COMPILER_READ_BARRIER();
  cpu_write(c);
  if (TASK_CPU(t) == c) break;
  cpu_endwrite(c);
 }
 error = task_kill2_cpu_endwrite(t,signal_info,reg_trapno,reg_err,was);
 task_endcrit();
 return error;
}

PUBLIC errno_t KCALL
task_kill2_ok(struct task *__restrict target,
              siginfo_t const *__restrict signal_info) {
 CHECK_HOST_DOBJ(target);
 CHECK_HOST_DOBJ(signal_info);
 /* TODO */
 return -EOK;
}

PUBLIC errno_t KCALL
task_kill(struct task *__restrict self, int signo) {
 siginfo_t info;
 memset(&info,0,sizeof(siginfo_t));
 info.si_signo = signo;
 info.si_pid   = GET_THIS_PID();
 info.si_uid   = GET_THIS_UID();
 return task_kill2(self,&info,0,0);
}



SYSCALL_DEFINE4(sigaction,int,sig,
                USER struct sigaction const *,act,
                USER struct sigaction *,oact,
                size_t,sigsetsize) {
 struct sighand *hand = THIS_SIGHAND;
 struct sigaction *action;
 errno_t error = -EINVAL;
 if (sigsetsize != sizeof(sigset_t)) goto end;
 if (sig == SIGKILL || sig == SIGSTOP ||
     sig <= 0 || --sig >= (_NSIG-1)) goto end;
 error = -EOK;
 action = &hand->sh_actions[sig];
 /* Copy the old signal action to user-space. */
 if (oact && copy_to_user(oact,action,sizeof(struct sigaction)))
     return -EFAULT;
 if (act) {
  struct sigaction new_action; pflag_t was;
  if (copy_from_user(&new_action,act,sizeof(struct sigaction)))
      return -EFAULT;
  was = PREEMPTION_PUSH();
  /* Make sure never to unmask KILL or STOP */
  sigdelset(&new_action.sa_mask,SIGKILL);
  sigdelset(&new_action.sa_mask,SIGSTOP);
  /* Add the signal itself to the list of signals
   * being masked during execution of the handler. */
  if (!(new_action.sa_flags&SA_NODEFER))
        sigaddset(&new_action.sa_mask,sig+1);
  /* Now load the new action into the kernel. */
  memcpy(action,&new_action,sizeof(struct sigaction));
  PREEMPTION_POP(was);

  /*
   * POSIX 3.3.1.3:
   *  "Setting a signal action to SIG_IGN for a signal that is
   *   pending shall cause the pending signal to be discarded,
   *   whether or not it is blocked."
   *
   *  "Setting a signal action to SIG_DFL for a signal that is
   *   pending and whose default action is to ignore the signal
   *   (for example, SIGCHLD), shall cause the pending signal to
   *   be discarded, whether or not it is blocked"
   */
  if (new_action.sa_handler == SIG_IGN ||
     (new_action.sa_handler == SIG_DFL &&
      default_actions[sig] == DA_IGN)) {
   struct task *t = THIS_TASK;
   /* Discard pending signals. */
   task_crit();
   sigpending_discard(&t->t_sigpend,sig+1);
   sigpending_discard(&t->t_sigshare->ss_pending,sig+1);
   task_endcrit();
  }
 }
end:
 return error;
}

PRIVATE SAFE errno_t KCALL
sig_deque_changes(sigset_t *__restrict changes) {
 siginfo_t return_info; ssize_t num_signals = 0;
 struct sigqueue *return_signal; struct task *t = THIS_TASK;
 sigword_t *iter,*end,*dst;
 for (;;) {
  bool share_write_lock,local_write_lock = false;
  sigpending_read(&t->t_sigpend);
  while ((return_signal = sigpending_try_dequeue_unlocked
         (&t->t_sigpend,changes,&local_write_lock)) == E_PTR(-ERELOAD));
  if (!return_signal) {
   share_write_lock = false;
   sigpending_read(&t->t_sigshare->ss_pending);
   do return_signal = sigpending_try_dequeue_unlocked(&t->t_sigshare->ss_pending,
                                                      changes,&share_write_lock);
   while (return_signal == E_PTR(-ERELOAD));
   if (local_write_lock)
        sigpending_endwrite(&t->t_sigshare->ss_pending);
   else sigpending_endread(&t->t_sigshare->ss_pending);
  }
  if (local_write_lock)
       sigpending_endwrite(&t->t_sigpend);
  else sigpending_endread(&t->t_sigpend);
  if (!return_signal) break;
  /* Managed to deque a return signal. - Handle it now. */
  if (SIGPENDING_IS_UNDEFINED(return_signal)) {
   memset(&return_info,0,sizeof(siginfo_t));
   return_info.si_signo = SIGPENDING_GT_UNDEFINED(return_signal);
   return_info.si_code  = SI_USER;
  } else {
   memcpy(&return_info,&return_signal->sq_info,sizeof(siginfo_t));
   free(return_signal);
  }
  /* Assert this to prevent an infinite loop. */
  assertf(sigismember(changes,return_info.si_signo),
          "signal %d wasn't changed",return_info.si_signo);
  assertf(!sigismember(&t->t_sigblock,return_info.si_signo),
          "Cannot deque signal %d currently being blocked",
          return_info.si_signo);
  task_kill2(t,&return_info,0,0);
  ++num_signals;

  /* Must unmask all potentially changed signals based on what
   * is being in the calling thread was the last signal was raised. */
  end = (iter = (sigword_t *)&t->t_sigblock)+SIGSET_WORDS;
  dst = (sigword_t *)changes;
  for (; iter != end; ++iter,++dst) *dst &= ~*iter;
 }
 return num_signals ? -EINTR : -EOK;
}

SYSCALL_DEFINE4(sigprocmask,int,how,USER sigset_t const *,set,
                USER sigset_t *,oldset,size_t,sigsetsize) {
 struct task *t = THIS_TASK; errno_t error = -EOK;
 sigset_t new_set; byte_t *dst,*end,*src;
 if (sigsetsize > sizeof(sigset_t)) return -EINVAL;
 if (set) {
  if (copy_from_user(&new_set,set,sigsetsize))
      return -EFAULT;
  sigdelset(&new_set,SIGKILL);
  sigdelset(&new_set,SIGSTOP);
 }
 task_crit();
 if (oldset &&
     copy_to_user(oldset,&t->t_sigblock,
                  sigsetsize))
 { error = -EFAULT; goto end; }
 if (set) {
  dst = (byte_t *)&t->t_sigblock;
  end = dst+sigsetsize;
  src = (byte_t *)&new_set;
  switch (how) {

   /* Simple case: Add to blocking mask. */
  case SIG_BLOCK:
   for (; dst != end; ++dst,++src) *dst |= *src;
   break;

   /* Difficult: Must return to the first signal that gets unblocked. */
  case SIG_UNBLOCK:
   /* NOTE: Store a mask of all changes in 'src' */
   for (; dst != end; ++dst,++src) {
    byte_t changed = *dst & *src;
    *dst &= ~*src;
    *src = changed;
   }
deque_changes:
   error = sig_deque_changes(&new_set);
   break;

   /* Difficult: Must return to the first signal that gets unblocked. */
  case SIG_SETMASK:
   /* NOTE: Store a mask of all changes in 'src' */
   for (; dst != end; ++dst,++src) if (*dst != *src) {
    byte_t changed = *dst & ~*src;
    *dst = *src;
    *src = changed;
   }
   goto deque_changes;

  default:
   error = -EINVAL;
   break;
  }
 }
end:
 task_endcrit();
 return error;
}



PUBLIC errno_t KCALL
task_set_sigblock(sigset_t *__restrict newset) {
 sigword_t *dst,*end,*src; errno_t error;
 dst = (sigword_t *)&THIS_TASK->t_sigblock;
 end = dst+SIGSET_WORDS;
 src = (sigword_t *)newset;
 for (; dst != end; ++dst,++src) if (*dst != *src) {
  sigword_t changed = *dst & ~*src;
  *dst = *src;
  *src = changed;
 }
 COMPILER_WRITE_BARRIER();
 task_crit();
 error = sig_deque_changes(newset);
 task_endcrit();
 return error;
}



/* Apart of the user-share segment:
 * >> This function is called to return from a signal handler.
 * AKA: This is our signal trampoline code. */
GLOBAL_ASM(
L(.section .text.user                                                            )
L(PRIVATE_ENTRY(signal_return)                                                   )
L(    movl $(__NR_sigreturn), %eax                                               )
L(    leal (SIGENTER_INFO_OFFSETOF_CTX - \
            SIGENTER_INFO_OFFSETOF_OLD_EBP)(%ebp), %ebx                          )
L(    int $0x80                                                                  )
L(SYM_END(signal_return)                                                         )
L(.previous                                                                      )
);

GLOBAL_ASM(
L(.section .text                                                                 )
L(PUBLIC_ENTRY(sys_sigreturn)                                                    )
L(    pushw %ds                                                                  )
L(    pushw %es                                                                  )
L(    pushw %fs                                                                  )
L(    pushw %gs                                                                  )
L(    pushal                                                                     )
L(    movl  %esp, %ecx                                                           )
L(    call  SYSC_sigreturn                                                       )
L(    popal                                                                      )
L(    popw %gs                                                                   )
L(    popw %fs                                                                   )
L(    popw %es                                                                   )
L(    popw %ds                                                                   )
L(    iret                                                                       )
L(SYM_END(sys_sigreturn)                                                         )
L(.previous                                                                      )
);

#define IGNORE_SEGMENT_REGISTERS 1

INTERN void FCALL SYSC_sigreturn(struct cpustate *__restrict cs) {
 ucontext_t ctx; size_t context_indirection;
 USER struct sigenter_info *last_context;
 if (copy_from_user(&ctx,(ucontext_t *)cs->host.ebx,sizeof(ucontext_t)))
     sigfault();
 /* TODO: Accept TLS segments. */
#if !IGNORE_SEGMENT_REGISTERS
#define CHECK_SEGMENT(id,name,value) \
 if (ctx.uc_mcontext.gregs[id] != (value)) \
     sigill("%%" name " (%.4I16x != %.4I16x)", \
            ctx.uc_mcontext.gregs[id],value)
#ifdef __i386__
 CHECK_SEGMENT(REG_GS,"gs",SEG(SEG_USER_DATA)|3);
 CHECK_SEGMENT(REG_FS,"fs",SEG(SEG_CPUSELF));
#else
 CHECK_SEGMENT(REG_GS,"gs",SEG(SEG_CPUSELF));
 CHECK_SEGMENT(REG_FS,"fs",SEG(SEG_USER_DATA)|3);
#endif
 CHECK_SEGMENT(REG_ES,"es",SEG(SEG_USER_DATA)|3);
 CHECK_SEGMENT(REG_DS,"ds",SEG(SEG_USER_DATA)|3);
 CHECK_SEGMENT(REG_CS,"cs",SEG(SEG_USER_CODE)|3);
 CHECK_SEGMENT(REG_SS,"ss",SEG(SEG_USER_DATA)|3);
#undef CHECK_SEGMENT
#endif /* !IGNORE_SEGMENT_REGISTERS */
 if (!(ctx.uc_mcontext.gregs[REG_EFL]&EFLAGS_IF))
     sigill("EFLAGS (%.8I32x misses EFLAGS_IF:%.8I32x)",
             ctx.uc_mcontext.gregs[REG_EFL],EFLAGS_IF);
#if 0 /* TODO: Re-enable me. */
 if (ctx.uc_mcontext.gregs[REG_EFL]&EFLAGS_IOPL(3))
     sigill("EFLAGS (%.8I32x contains EFLAGS_IOPL:%.8I32x)",
             ctx.uc_mcontext.gregs[REG_EFL],EFLAGS_IOPL(3));
#endif

 /* Load the new machine context as register state upon return to user-space.
  * NOTE: We don't jump directly, because that would break execution of
  *       additional signals that may have been unblocked by 'task_set_sigblock()'. */
#if IGNORE_SEGMENT_REGISTERS
#ifdef __i386__
 cs->host.gs     = SEG(SEG_USER_DATA)|3;
 cs->host.fs     = SEG(SEG_CPUSELF);
#else
 cs->host.gs     = SEG(SEG_CPUSELF);
 cs->host.fs     = SEG(SEG_USER_DATA)|3;
#endif
 cs->host.es     = SEG(SEG_USER_DATA)|3;
 cs->host.ds     = SEG(SEG_USER_DATA)|3;
 ctx.uc_mcontext.gregs[REG_CS] = SEG(SEG_USER_CODE)|3;
 ctx.uc_mcontext.gregs[REG_SS] = SEG(SEG_USER_DATA)|3;
#else /* IGNORE_SEGMENT_REGISTERS */
 cs->host.gs     = (u16)ctx.uc_mcontext.gregs[REG_GS];
 cs->host.fs     = (u16)ctx.uc_mcontext.gregs[REG_FS];
 cs->host.es     = (u16)ctx.uc_mcontext.gregs[REG_ES];
 cs->host.ds     = (u16)ctx.uc_mcontext.gregs[REG_DS];
#endif /* !IGNORE_SEGMENT_REGISTERS */
 cs->host.edi    = ctx.uc_mcontext.gregs[REG_EDI];
 cs->host.esi    = ctx.uc_mcontext.gregs[REG_ESI];
 cs->host.ebp    = ctx.uc_mcontext.gregs[REG_EBP];
 cs->host.esp    = ctx.uc_mcontext.gregs[REG_ESP];
 cs->host.ebx    = ctx.uc_mcontext.gregs[REG_EBX];
 cs->host.edx    = ctx.uc_mcontext.gregs[REG_EDX];
 cs->host.ecx    = ctx.uc_mcontext.gregs[REG_ECX];
 cs->host.eax    = ctx.uc_mcontext.gregs[REG_EAX];
 
 /* NOTE: Disable preemption to prevent more signals from being raised,
  *       now that we'll be attempting to update the bottom-most entry
  *       of the signal-handler-execution chain.
  *    >> Having had interrupts enabled until now, more signals may
  *       have been raised, and even more though: Restoring the old
  *       signal mask may (quite likely) re-raised more signals.
  *       But sadly, the bottom-most signal context is using the address
  *       of this where 'sys_sigreturn()' was called from, which is quite
  *       incorrect as this function isn't supposed to return to the caller,
  *       but instead return to where the first interrupt was called from.
  *    >> Therefor we must find that bottom-most entry and update it
  *       to mirror the register data we've been passed through the
  *       given user-space CPU context.
  * HINT: We know the exact amount of indirections required, as this
  *       value is safely stored within 'THIS_TASK->t_sigenter.se_count'.
  */
 PREEMPTION_DISABLE();
 context_indirection = THIS_TASK->t_sigenter.se_count;
 if (context_indirection) {
  last_context = THIS_TASK->t_sigenter.se_siginfo;
  while (--context_indirection) {
   if (copy_from_user(&last_context,&last_context->ei_next,
                       sizeof(USER struct sigenter_info *)))
       sigfault();
  }
  if (copy_to_user(&last_context->ei_ctx.uc_mcontext.gregs[REG_EIP],
                   &ctx.uc_mcontext.gregs[REG_EIP],
                 ((REG_SS-REG_EIP)+1)*sizeof(greg_t)))
      sigfault();
 } else {
  /* Simple case: no indirection. - This signal will
   * switch back to regular user-space code flow. */
  cs->host.eip    = ctx.uc_mcontext.gregs[REG_EIP];
  cs->host.cs     = ctx.uc_mcontext.gregs[REG_CS];
  cs->host.eflags = ctx.uc_mcontext.gregs[REG_EFL];
  cs->useresp     = ctx.uc_mcontext.gregs[REG_UESP];
  cs->ss          = ctx.uc_mcontext.gregs[REG_SS];
 }
 /* NOTE: Technically, we don't have to re-enable interrupts now.
  *       But let's try to be as pre-emptive as possible... */
 PREEMPTION_ENABLE();

 /* Restore the old signal mask.
  * NOTE: This may raise more signals, but that's ok.
  * >> Due to the quite high chance of this raising more signals, this part
  *    is done _AFTER_ we've updated registers, quite simply to keep the
  *    number of required indirections when reloading return registers to
  *    a bare minimum, as is the case when this is done last, with the only
  *    possibility of additional signals occurring before preemption is disabled
  *    above, being from sporadic interrupts that may occur until then.
  */
 sigdelset(&ctx.uc_sigmask,SIGKILL);
 sigdelset(&ctx.uc_sigmask,SIGSTOP);
 task_set_sigblock(&ctx.uc_sigmask);
}




SYSCALL_DEFINE4(sigtimedwait,USER sigset_t const *,uthese,USER siginfo_t *,uinfo,
                USER struct timespec const *,uts,size_t,sigsetsize) {
 sigset_t wait_mask,old_block; struct timespec abstime;
 struct task *t = THIS_TASK; errno_t error; struct sigqueue *signal;
 bool local_write_lock,share_write_lock,is_blocking = false;
 if (sigsetsize > sizeof(sigset_t)) return -EINVAL;
 memset((u8 *)&wait_mask+sigsetsize,0,sizeof(sigset_t)-sigsetsize);
 if (copy_from_user(&wait_mask,uthese,sigsetsize))
     return -EFAULT;
 if (uts && copy_from_user(&abstime,uts,sizeof(struct timespec)))
     return -EFAULT;
 /* Make sure never to unmask these signals! */
 sigdelset(&wait_mask,SIGKILL);
 sigdelset(&wait_mask,SIGSTOP);
 task_crit();
scan_again:
 local_write_lock = false;
 share_write_lock = false;
 sigpending_read(&t->t_sigpend);
scan_again_local:
 do signal = sigpending_try_dequeue_unlocked(&t->t_sigpend,
                                             &wait_mask,&local_write_lock);
 while (signal == E_PTR(-ERELOAD));
 if (!signal) {
  sigpending_read(&t->t_sigshare->ss_pending);
scan_again_shared:
  do signal = sigpending_try_dequeue_unlocked(&t->t_sigshare->ss_pending,
                                              &wait_mask,&share_write_lock);
  while (signal == E_PTR(-ERELOAD));
  if (!signal) {
   /* Wait for more signals to arrive.
    * NOTE: Make sure we've got a write-lock on both pending controllers. */
   struct sig *wait_error;
   if (!local_write_lock) {
    local_write_lock = true;
    if (!sigpending_upgrade(&t->t_sigpend)) {
     /* Make sure to release the share-lock before reloading. */
     if (share_write_lock) share_write_lock = false,
          sigpending_endwrite(&t->t_sigshare->ss_pending);
     else sigpending_endread(&t->t_sigshare->ss_pending);
     goto scan_again_local;
    }
   }
   if (!share_write_lock &&
       !sigpending_upgrade(&t->t_sigshare->ss_pending))
        goto scan_again_shared;
   /* At this point we've confirmed that no signal is available
    * in either of the pending controllers, meaning now we have
    * to wait for it to arrive.
    * >> For this, we must also make sure that the intended signals
    *    will not execute handlers, but rather be scheduled as
    *    pending instead. */
   if (!is_blocking) {
    sigword_t *dst,*end,*src;
    /* Make sure to create a backup of the blocking-set, to-be restored later. */
    memcpy(&old_block,&t->t_sigblock,sizeof(sigset_t));
    /* Add all signals that we're waiting for to the blocking set. */
    dst = (sigword_t *)&THIS_TASK->t_sigblock;
    end = dst+SIGSET_WORDS;
    src = (sigword_t *)&wait_mask;
    for (; dst != end; ++dst,++src) *dst |= *src;
    is_blocking = true;
   }

   /* Wait for both signals at once. */
   task_addwait(&t->t_sigpend.sp_newsig,NULL,0);
   task_addwait(&t->t_sigshare->ss_pending.sp_newsig,NULL,0);

   /* TODO: This is totally unnecessary. - The way the new scheduler has
    *       been implemented, we can simply lock + 'task_addwait()' + unlock
    *       any number of signals consecutively and have 'task_waitfor()'
    *       return a pointer to the pending set to which a new signal was
    *       added.
    * >> This what we're currently doing by locking both sets
    *    at the same time is both wasteful and unnecessary! */
   sig_endwrite(&t->t_sigpend.sp_newsig);
   sig_endwrite(&t->t_sigshare->ss_pending.sp_newsig);

   wait_error = task_waitfor(uts ? &abstime : NULL);
   if (E_ISERR(wait_error)) { error = E_GTERR(wait_error); goto end; }
   goto scan_again;
  }
  if (share_write_lock)
       sigpending_endwrite(&t->t_sigshare->ss_pending);
  else sigpending_endread(&t->t_sigshare->ss_pending);
 }
 if (local_write_lock)
      sigpending_endwrite(&t->t_sigpend);
 else sigpending_endread(&t->t_sigpend);
 /* We've managed to find a signal! */
 assert(signal && signal != E_PTR(-ERELOAD));
 error = -EOK;
 /* Copy the collected signal information to user-space and free the signal buffer. */
 if (SIGPENDING_IS_UNDEFINED(signal)) {
  if (uinfo) {
   siginfo_t return_info;
   memset(&return_info,0,sizeof(return_info));
   return_info.si_signo = SIGPENDING_GT_UNDEFINED(signal);
   return_info.si_code  = SI_USER;
   if (copy_to_user(uinfo,&return_info,sizeof(siginfo_t)))
       error = -EFAULT;
  }
 } else {
  if (uinfo && copy_to_user(uinfo,&signal->sq_info,sizeof(siginfo_t)))
      error = -EFAULT;
  free(signal);
 }
end:
 task_endcrit();
 /* Restore the old blocking configuration, in the process handling
  * any additional signals that were not caught by the wait(). */
 if (is_blocking)
     task_set_sigblock(&old_block);
 return error;
}

SYSCALL_DEFINE2(sigpending,USER sigset_t *,uset,size_t,sigsetsize) {
 sigset_t set; sigword_t *dst,*end,*src;
 struct task *t = THIS_TASK;
 if unlikely(sigsetsize > sizeof(sigset_t))
    return -EINVAL;
 task_crit();
 sigpending_read(&t->t_sigpend);
 memcpy(&set,&t->t_sigpend.sp_mask,sizeof(sigset_t));
 sigpending_endread(&t->t_sigpend);
 sigpending_read(&t->t_sigshare->ss_pending);
 end = (dst = (sigword_t *)&set)+SIGSET_WORDS;
 src = (sigword_t *)&t->t_sigshare->ss_pending.sp_mask;
 for (; dst != end; ++dst,++src) *dst |= *src;
 sigpending_endread(&t->t_sigshare->ss_pending);
 task_endcrit();
 if (copy_to_user(uset,&set,sigsetsize))
     return -EFAULT;
 return -EOK;
}


PRIVATE ssize_t KCALL
killgrp_info(struct task *__restrict leader,
             siginfo_t const *__restrict info) {
 struct task *iter; errno_t temp;
 size_t result = 1,count = 0; REF struct task **buf,**dst;
 /* First off: Kill the leader in case the signal needs
  *            to prevent more children from spawning. */
 temp = TASK_KILL2_IFOK(leader,info,0,0);
 if (!E_ISOK(temp)) return (ssize_t)temp;
 atomic_rwlock_read(&leader->t_pid.tp_grouplock);
 for (iter = leader->t_pid.tp_group; iter;
      iter = iter->t_pid.tp_grplink.le_next)
      if (iter != leader) ++count;
 buf = tmalloc(REF struct task *,count);
 if (!buf) {
  result = -ENOMEM;
  atomic_rwlock_endread(&leader->t_pid.tp_grouplock);
 } else {
  for (iter = leader->t_pid.tp_group,dst = buf; iter;
       iter = iter->t_pid.tp_grplink.le_next)
       if (iter != leader && TASK_TRYINCREF(iter)) *dst++ = iter;
  atomic_rwlock_endread(&leader->t_pid.tp_grouplock);
  /* Now kill all tasks apart of the group. */
  temp = -EOK;
  for (; dst != buf; --dst) {
   iter = *dst;
   if (E_ISOK(temp))
       temp = TASK_KILL2_IFOK(iter,info,0,0);
   TASK_DECREF(iter);
   ++result;
  }
  free(buf);
  /* If we've failed to kill something, return the error that caused this. */
  if (!E_ISOK(temp))
       return temp;
 }
 return result;
}

PRIVATE SAFE ssize_t KCALL
killpid_info(pid_t pid, siginfo_t const *__restrict info) {
 REF struct task *target;
 ssize_t result;
 if (pid > 0) {
  /* Kill The process matching 'pid'. */
  target = pid_namespace_lookup(THIS_NAMESPACE,pid);
  if (!target) result = -ESRCH;
  else {
   result = TASK_KILL2_IFOK(target,info,0,0);
   TASK_DECREF(target);
   if (E_ISOK(result))
       result = 1;
  }
 } else if (pid == 0) {
  REF struct task *leader;
  /* Kill all processes in the calling thread's process group. */
  atomic_rwlock_read(&THIS_TASK->t_pid.tp_leadlock);
  leader = THIS_TASK->t_pid.tp_leader;
  TASK_INCREF(leader);
  atomic_rwlock_endread(&THIS_TASK->t_pid.tp_leadlock);
  result = killgrp_info(leader,info);
  TASK_DECREF(leader);
 } else if (pid == -1) {
  /* TODO: Kill all processes that we're allowed to. */
 } else {
  /* Kill the entirety of a process group. */
  target = pid_namespace_lookup(THIS_NAMESPACE,-pid);
  if (!target) result = -ESRCH;
  else {
   result = killgrp_info(target,info);
   TASK_DECREF(target);
  }
 }
 return result;
}

SYSCALL_DEFINE2(kill,pid_t,pid,int,sig) {
 siginfo_t info; ssize_t result;
 info.si_signo = sig;
 info.si_errno = 0;
 info.si_code  = SI_USER;
 task_crit();
 info.si_pid   = GET_THIS_PID();
 info.si_uid   = GET_THIS_UID();
 result = killpid_info(pid,&info);
 task_endcrit();

 /* NOTE: On success (At least one signal was sent), ZERO(0) is returned. */
 if (result > 1) result = 0;
 return result;
}

PRIVATE errno_t KCALL
tkill_info(pid_t tgid, pid_t pid, siginfo_t *__restrict info) {
 REF struct task *t;
 errno_t result = -ESRCH;
 /* Make sure we've been given a valid signal number. */
 if (info->si_signo < 0 || info->si_signo >= _NSIG)
     return -EINVAL;
 if ((t = pid_namespace_lookup(THIS_NAMESPACE,pid)) != NULL) {
  if (tgid <= 0 || TASK_GETTGID(t) == tgid) {
   result = task_kill2_ok(t,info);
   /* NOTE: Don't deliver signal '0'. - It's used to test access. */
   if (E_ISOK(result) && info->si_signo != 0)
       result = task_kill2(t,info,0,0);
  }
  TASK_DECREF(t);
 }
 return result;
}
PRIVATE errno_t KCALL
do_tkill(pid_t tgid, pid_t pid, int sig) {
 siginfo_t info; errno_t result;
 info.si_signo = sig;
 info.si_errno = 0;
 info.si_code  = SI_TKILL;
 task_crit();
 info.si_pid   = GET_THIS_PID();
 info.si_uid   = GET_THIS_UID();
 result = tkill_info(tgid,pid,&info);
 task_endcrit();
 return result;
}

SYSCALL_DEFINE3(tgkill,pid_t,tgid,pid_t,pid,int,sig) {
 if (pid <= 0 || tgid <= 0) return -EINVAL;
 return do_tkill(tgid,pid,sig);
}
SYSCALL_DEFINE2(tkill,pid_t,pid,int,sig) {
 if (pid <= 0) return -EINVAL;
 return do_tkill(0,pid,sig);
}


/*
#define __NR_sigaltstack  132
__SYSCALL(__NR_sigaltstack,sys_sigaltstack)
#define __NR_sigsuspend   133
__SYSCALL(__NR_sigsuspend,sys_sigsuspend)
#define __NR_sigqueueinfo 138
__SYSCALL(__NR_sigqueueinfo,sys_sigqueueinfo)
*/


DECL_END

#endif /* !GUARD_KERNEL_SCHED_SIGNAL_C */
