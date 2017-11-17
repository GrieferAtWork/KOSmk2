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

#include <hybrid/compiler.h>
#ifndef CONFIG_NO_SIGNALS
#include <asm/instx.h>
#include <asm/unistd.h>
#include <bits/signum.h>
#include <bits/waitstatus.h>
#include <dev/rtc.h>
#include <errno.h>
#include <asm/cpu-flags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/traceback.h>
#include <arch/gdt.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <linker/coredump.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <sched/types.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/syslog.h>
#include <sys/ucontext.h>
#include <kos/thread.h>
#include <arch/hints.h>
#include <arch/asm.h>

DECL_BEGIN

STATIC_ASSERT(sizeof(siginfo_t) <= __SI_MAX_SIZE);

STATIC_ASSERT(offsetof(mcontext_t,gregs) == __MCONTEXT_OFFSETOF_GREGS);
STATIC_ASSERT(offsetof(mcontext_t,fpregs) == __MCONTEXT_OFFSETOF_FPREGS);
#ifdef __MCONTEXT_OFFSETOF_FS_BASE
STATIC_ASSERT(offsetof(mcontext_t,fs_base) == __MCONTEXT_OFFSETOF_FS_BASE);
STATIC_ASSERT(offsetof(mcontext_t,gs_base) == __MCONTEXT_OFFSETOF_GS_BASE);
#endif
STATIC_ASSERT(sizeof(mcontext_t) == __MCONTEXT_SIZE);
STATIC_ASSERT(offsetof(ucontext_t,uc_flags) == __UCONTEXT_OFFSETOF_FLAGS);
STATIC_ASSERT(offsetof(ucontext_t,uc_link) == __UCONTEXT_OFFSETOF_LINK);
STATIC_ASSERT(offsetof(ucontext_t,uc_stack) == __UCONTEXT_OFFSETOF_STACK);
STATIC_ASSERT(offsetof(ucontext_t,uc_mcontext) == __UCONTEXT_OFFSETOF_MCONTEXT);
STATIC_ASSERT(offsetof(ucontext_t,uc_sigmask) == __UCONTEXT_OFFSETOF_SIGMASK);
STATIC_ASSERT(offsetof(ucontext_t,__fpregs_mem) == __UCONTEXT_OFFSETOF_FPREGS);
STATIC_ASSERT(sizeof(ucontext_t) == __UCONTEXT_SIZE);
STATIC_ASSERT(sizeof(struct sigenter) == SIGENTER_SIZE);



/* Signal default actions. */
typedef u8 dact_t; enum{
    DA_TERM, /* Terminate application immediately. */
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

#define SIGSET_WORDS (__SIZEOF_SIGSET_T__ / SIGWORD_SIZE)
#if ((__SIZEOF_SIGSET_T__ % 8) == 0) && __SIZEOF_BUSINT__ >= 8
typedef u64 sigword_t;
#define SIGWORD_SIZE 8
#elif ((__SIZEOF_SIGSET_T__ % 4) == 0) && __SIZEOF_BUSINT__ >= 4
typedef u32 sigword_t;
#define SIGWORD_SIZE 4
#elif ((__SIZEOF_SIGSET_T__ % 2) == 0) && __SIZEOF_BUSINT__ >= 2
typedef u16 sigword_t;
#define SIGWORD_SIZE 2
#else
typedef u8  sigword_t;
#define SIGWORD_SIZE 1
#endif







#ifndef CONFIG_USE_OLD_SIGNALS

PRIVATE void KCALL
coredump_user_task(struct task *__restrict t,
                   siginfo_t const *__restrict reason,
                   greg_t reg_trapno, greg_t reg_err) {
 /* TODO */
}

PRIVATE void KCALL
coredump_host_task(struct task *__restrict t,
                   siginfo_t const *__restrict reason,
                   greg_t reg_trapno, greg_t reg_err) {
 /* TODO */
}



/* Apart of the user-share segment:
 * >> This function is called to return from a signal handler.
 * AKA: This is our signal trampoline code. */
INTDEF ATTR_NORETURN void ASMCALL signal_return(void);
GLOBAL_ASM(
L(.section .text.user                                                            )
L(PRIVATE_ENTRY(signal_return)                                                   )
#ifdef __x86_64__ /* Load `struct sigenter_tail' into the first syscal register. */
L(    leaq -SIGENTER_TAIL_OFFSETOF_OLDBP(%rbp), %rdi                             )
#else
L(    leal -SIGENTER_TAIL_OFFSETOF_OLDBP(%ebp), %ebx                             )
#endif
L(1:  movx $(__NR_sigreturn),      %xax                                          )
L(    int  $0x80                                                                 )
L(    jmp  1b /* Keep repeating in case of recursion. */                         )
L(SYM_END(signal_return)                                                         )
L(.previous                                                                      )
);

PRIVATE errno_t KCALL raise_segfault(void *fault_addr) {
 siginfo_t info;
 THIS_TASK->t_lastcr2 = fault_addr;
 memset(&info,0,sizeof(siginfo_t));
 info.si_signo = SIGSEGV;
 info.si_code  = SI_KERNEL;
 info.si_addr  = fault_addr;
 info.si_lower = fault_addr;
 info.si_upper = fault_addr;
 /* XXX: exc_code is hard-coded as `0' - That shouldn't be. */
 return task_kill2(THIS_TASK,&info,EXC_PAGE_FAULT,0);
}

/* Return the real IRET tail active in the given task/caller. */
LOCAL struct irregs *KCALL signal_irregs(void) {
 struct task *t = THIS_TASK;
 assertf(!PREEMPTION_ENABLED(),"The real IRET may change when preemption is enabled.");
 if (t->t_sigenter.se_count) return (struct irregs *)&t->t_sigenter.se_xip;
 return &IRREGS_SYSCALL_GET()->tail;
}
LOCAL struct irregs *KCALL signal_irregs_of(struct task *__restrict t) {
 assertf(!PREEMPTION_ENABLED(),"The real IRET may change when preemption is enabled.");
 assertf(TASK_CPU(t) == THIS_CPU,"The given task isn't hosted by the current CPU.");
 if (t->t_sigenter.se_count) return (struct irregs *)&t->t_sigenter;
 if (t == THIS_TASK) return &IRREGS_SYSCALL_GET()->tail;
 return &t->t_cstate->iret;
}

#define RFLAGS_USER_MASK  (EFLAGS_CF|EFLAGS_PF|EFLAGS_AF|EFLAGS_ZF|EFLAGS_SF)

SYSCALL_SDEFINE(sigreturn,state) {
 ucontext_t ctx; size_t copy_error;
 struct task *caller = THIS_TASK;
 assert(&state->iret == &IRREGS_SYSCALL_GET()->tail);

 /* Copy the context into kernel-space. */
 if ((copy_error = copy_from_user(&ctx,
     (void *)(GPREGS_SYSCALL_ARG1(state->gp)+SIGENTER_TAIL_OFFSETOF_CTX),
                                  sizeof(ucontext_t))) != 0) {
  raise_segfault((byte_t *)(GPREGS_SYSCALL_ARG1(state->gp)+SIGENTER_TAIL_OFFSETOF_CTX)+
                 (sizeof(ucontext_t)-copy_error));
  return;
 }
 assert(PREEMPTION_ENABLED());
 PREEMPTION_DISABLE();

 /* Check for other signals, which can happen in cases where another one was raised
  * during the time that `signal_return' invoked the system call and us getting here.
  * In that case, don't restore anything until the other signal hander was executed. */
 if unlikely(caller->t_sigenter.se_count) {
  /* Assert that our effective return address was overwritten (by `sigenter'). */
  assert(!(state->iret.cs&3));
  assert(addr_isglob(state->iret.xip));
  goto end;
 }

 /* With interrupts disabled, */

 state->iret.xflags &= ~(RFLAGS_USER_MASK);
 state->iret.xflags |= ctx.uc_mcontext.gregs[REG_EFL];
 caller->t_flags &= ~(TASKFLAG_DELAYSIGS);
 if (!(ctx.uc_mcontext.gregs[REG_EFL]&EFLAGS_IF))
       caller->t_flags |= TASKFLAG_DELAYSIGS;
#ifdef __x86_64__
 state->iret.rip     = ctx.uc_mcontext.gregs[REG_RIP];
 state->iret.userrsp = ctx.uc_mcontext.gregs[REG_RSP];
 state->gp.r8        = ctx.uc_mcontext.gregs[REG_R8];
 state->gp.r9        = ctx.uc_mcontext.gregs[REG_R9];
 state->gp.r10       = ctx.uc_mcontext.gregs[REG_R10];
 state->gp.r11       = ctx.uc_mcontext.gregs[REG_R11];
 state->gp.r12       = ctx.uc_mcontext.gregs[REG_R12];
 state->gp.r13       = ctx.uc_mcontext.gregs[REG_R13];
 state->gp.r14       = ctx.uc_mcontext.gregs[REG_R14];
 state->gp.r15       = ctx.uc_mcontext.gregs[REG_R15];
 state->gp.rdi       = ctx.uc_mcontext.gregs[REG_RDI];
 state->gp.rsi       = ctx.uc_mcontext.gregs[REG_RSI];
 state->gp.rbp       = ctx.uc_mcontext.gregs[REG_RBP];
 state->gp.rbx       = ctx.uc_mcontext.gregs[REG_RBX];
 state->gp.rdx       = ctx.uc_mcontext.gregs[REG_RDX];
 state->gp.rax       = ctx.uc_mcontext.gregs[REG_RAX];
 state->gp.rcx       = ctx.uc_mcontext.gregs[REG_RCX];
 state->sg.fs_base   = ctx.uc_mcontext.fs_base;
 state->sg.gs_base   = ctx.uc_mcontext.gs_base;
#else /* __x86_64__ */
 state->iret.eip     = ctx.uc_mcontext.gregs[REG_EIP];
 state->iret.useresp = ctx.uc_mcontext.gregs[REG_UESP];
#if GPREGS_OFFSETOF_EDI == (REG_EDI-REG_EDI)*4 && GPREGS_OFFSETOF_ESI == (REG_ESI-REG_EDI)*4 && \
    GPREGS_OFFSETOF_EBP == (REG_EBP-REG_EDI)*4 && GPREGS_OFFSETOF_ESP == (REG_ESP-REG_EDI)*4 && \
    GPREGS_OFFSETOF_EBX == (REG_EBX-REG_EDI)*4 && GPREGS_OFFSETOF_EDX == (REG_EDX-REG_EDI)*4 && \
    GPREGS_OFFSETOF_ECX == (REG_ECX-REG_EDI)*4 && GPREGS_OFFSETOF_EAX == (REG_EAX-REG_EDI)*4
 memcpy(&state->gp,&ctx.uc_mcontext.gregs[REG_EDI],sizeof(struct gpregs));
#else
 state->gp.edi       = ctx.uc_mcontext.gregs[REG_EDI];
 state->gp.esi       = ctx.uc_mcontext.gregs[REG_ESI];
 state->gp.ebp       = ctx.uc_mcontext.gregs[REG_EBP];
 state->gp.ebx       = ctx.uc_mcontext.gregs[REG_EBX];
 state->gp.edx       = ctx.uc_mcontext.gregs[REG_EDX];
 state->gp.eax       = ctx.uc_mcontext.gregs[REG_EAX];
 state->gp.ecx       = ctx.uc_mcontext.gregs[REG_ECX];
 /* TODO: Restore segment registers. */
#endif
#endif /* !__x86_64__ */
 /* TODO: Restore FPU state. */

 /* Restore the old signal mask.
  * NOTE: This may raise more signals, but that's ok.
  * >> Due to the quite high chance of this raising more signals, this part
  *    is done _AFTER_ we've updated registers, quite simply to keep the
  *    number of required indirections when reloading return registers to
  *    a bare minimum, as is the case when this is done last, with the only
  *    possibility of additional signals occurring before preemption is disabled
  *    above, being from sporadic interrupts that may occur until then. */
 sigdelset(&ctx.uc_sigmask,SIGKILL);
 sigdelset(&ctx.uc_sigmask,SIGSTOP);
 task_set_sigblock(&ctx.uc_sigmask);

end:
 PREEMPTION_ENABLE();
}

#ifndef CONFIG_NO_FPU
PRIVATE void KCALL
safe_fpu(struct _libc_fpstate *__restrict state,
         struct fpustate const *fpu) {
 if (fpu != FPUSTATE_NULL) {
#ifdef __x86_64__
  STATIC_ASSERT(sizeof(struct fpustate) == sizeof(struct _libc_fpstate));
  memcpy(state,fpu,sizeof(struct fpustate));
#else
  struct fpu_reg const *src; struct _libc_fpreg *dst,*end;
  STATIC_ASSERT(sizeof(struct _libc_fpreg) <= sizeof(struct fpu_reg));
  state->cw      = (__ULONGPTR_TYPE__)fpu->fp_fcw;
  state->sw      = (__ULONGPTR_TYPE__)fpu->fp_fsw;
  state->tag     = (__ULONGPTR_TYPE__)fpu->fp_ftw;
  state->ipoff   = (__ULONGPTR_TYPE__)fpu->fp_fpuip;
  state->cssel   = (__ULONGPTR_TYPE__)fpu->fp_fpucs;
  state->dataoff = (__ULONGPTR_TYPE__)fpu->fp_fpudp;
  state->datasel = (__ULONGPTR_TYPE__)fpu->fp_fpuds;
  state->status  = (__ULONGPTR_TYPE__)fpu->fp_mxcsr; /* ??? Is this correct? */
  src = fpu->fp_regs;
  end = (dst = state->_st)+COMPILER_LENOF(state->_st);
  for (; dst != end; ++dst,++src)
         memcpy(dst,src,sizeof(struct _libc_fpreg));
#endif
 } else {
  memset(state,0,sizeof(struct _libc_fpstate));
 }
}
#else
#define safe_fpu(state,fpu) memset((state),0,sizeof(struct _libc_fpstate));
#endif


PRIVATE errno_t KCALL
deliver_signal_to_task_in_user(struct task *__restrict t,
                               struct sigaction const *__restrict action,
                               siginfo_t const *__restrict signal_info,
                               greg_t reg_trapno, greg_t reg_err) {
 USER struct sigenter_info *dst; struct mman *omm;
 struct cpustate *state = t->t_cstate;
 struct sigenter_tail tail; size_t copy_error;
 assert(!PREEMPTION_ENABLED());
 assert(TASK_CPU(t) == THIS_CPU);
 assert(t != THIS_TASK);
 assertf(state->iret.cs&3,"The task isn't running in user-space.");

 tail.t_oldbp = (USER void *)state->gp.xbp;
 tail.t_oldip = (USER void *)state->iret.xip;
 tail.t_ctx.uc_flags = 0;

 /* Save stack information. */
 if (t->t_ustack) {
  tail.t_ctx.uc_stack.ss_sp    = t->t_ustack->s_begin;
  tail.t_ctx.uc_stack.ss_size  = ((uintptr_t)t->t_ustack->s_end-
                                  (uintptr_t)t->t_ustack->s_begin);
  tail.t_ctx.uc_stack.ss_flags = SS_ONSTACK;
 } else {
  tail.t_ctx.uc_stack.ss_sp    = (void *)state->iret.userxsp;
  tail.t_ctx.uc_stack.ss_size  = 0;
  tail.t_ctx.uc_stack.ss_flags = SS_DISABLE;
 }

 /* Save signal masking information. */
 memcpy(&tail.t_ctx.uc_sigmask,
        &t->t_sigblock,sizeof(sigset_t));

 /* Save FPU information. */
 safe_fpu(&tail.t_ctx.__fpregs_mem,t->t_arch.at_fpu);

 /* Save general purpose registers. */
#ifdef __x86_64__
 tail.t_signo = signal_info->si_signo;
 tail.t_ctx.uc_mcontext.gregs[REG_R8]      = state->gp.r8;
 tail.t_ctx.uc_mcontext.gregs[REG_R9]      = state->gp.r9;
 tail.t_ctx.uc_mcontext.gregs[REG_R10]     = state->gp.r10;
 tail.t_ctx.uc_mcontext.gregs[REG_R11]     = state->gp.r11;
 tail.t_ctx.uc_mcontext.gregs[REG_R12]     = state->gp.r12;
 tail.t_ctx.uc_mcontext.gregs[REG_R13]     = state->gp.r13;
 tail.t_ctx.uc_mcontext.gregs[REG_R14]     = state->gp.r14;
 tail.t_ctx.uc_mcontext.gregs[REG_R15]     = state->gp.r15;
 tail.t_ctx.uc_mcontext.gregs[REG_RDI]     = state->gp.rdi;
 tail.t_ctx.uc_mcontext.gregs[REG_RSI]     = state->gp.rsi;
 tail.t_ctx.uc_mcontext.gregs[REG_RBP]     = state->gp.rbp;
 tail.t_ctx.uc_mcontext.gregs[REG_RBX]     = state->gp.rbx;
 tail.t_ctx.uc_mcontext.gregs[REG_RDX]     = state->gp.rdx;
 tail.t_ctx.uc_mcontext.gregs[REG_RAX]     = state->gp.rax;
 tail.t_ctx.uc_mcontext.gregs[REG_RCX]     = state->gp.rcx;
 tail.t_ctx.uc_mcontext.gregs[REG_RSP]     = state->iret.userrsp;
 tail.t_ctx.uc_mcontext.gregs[REG_RIP]     = state->iret.rip;
 tail.t_ctx.uc_mcontext.gregs[REG_CSGSFS]  = (((u64)__USER_CS)|
                                              ((u64)__USER_GS << 16)|
                                              ((u64)__USER_FS << 32));
 tail.t_ctx.uc_mcontext.gregs[REG_OLDMASK] = *(s64 *)&t->t_sigblock;
 tail.t_ctx.uc_mcontext.gregs[REG_CR2]     = (u64)t->t_lastcr2;
 tail.t_ctx.uc_mcontext.fs_base            = state->sg.fs_base;
 tail.t_ctx.uc_mcontext.gs_base            = state->sg.gs_base;
#else
 tail.t_ctx.uc_mcontext.gregs[REG_GS]     = (u32)state->sg.gs;
 tail.t_ctx.uc_mcontext.gregs[REG_FS]     = (u32)state->sg.fs;
 tail.t_ctx.uc_mcontext.gregs[REG_ES]     = (u32)state->sg.es;
 tail.t_ctx.uc_mcontext.gregs[REG_DS]     = (u32)state->sg.ds;
#if GPREGS_OFFSETOF_EDI == (REG_EDI-REG_EDI)*4 && GPREGS_OFFSETOF_ESI == (REG_ESI-REG_EDI)*4 && \
    GPREGS_OFFSETOF_EBP == (REG_EBP-REG_EDI)*4 && GPREGS_OFFSETOF_ESP == (REG_ESP-REG_EDI)*4 && \
    GPREGS_OFFSETOF_EBX == (REG_EBX-REG_EDI)*4 && GPREGS_OFFSETOF_EDX == (REG_EDX-REG_EDI)*4 && \
    GPREGS_OFFSETOF_ECX == (REG_ECX-REG_EDI)*4 && GPREGS_OFFSETOF_EAX == (REG_EAX-REG_EDI)*4
 memcpy(&tail.t_ctx.uc_mcontext.gregs[REG_EDI],&state->gp,sizeof(struct gpregs));
#else
 tail.t_ctx.uc_mcontext.gregs[REG_EDI]    = state->gp.edi;
 tail.t_ctx.uc_mcontext.gregs[REG_ESI]    = state->gp.esi;
 tail.t_ctx.uc_mcontext.gregs[REG_EBP]    = state->gp.ebp;
 tail.t_ctx.uc_mcontext.gregs[REG_EBX]    = state->gp.ebx;
 tail.t_ctx.uc_mcontext.gregs[REG_EDX]    = state->gp.edx;
 tail.t_ctx.uc_mcontext.gregs[REG_EAX]    = state->gp.eax;
 tail.t_ctx.uc_mcontext.gregs[REG_ECX]    = state->gp.ecx;
#endif
 tail.t_ctx.uc_mcontext.gregs[REG_ESP]    = state->iret.userxsp;
 tail.t_ctx.uc_mcontext.gregs[REG_EIP]    = state->iret.xip;
#if 1 /* KOS doesn't allow secondary user-space code segments! */
 tail.t_ctx.uc_mcontext.gregs[REG_CS]     = __USER_CS;
#else
 tail.t_ctx.uc_mcontext.gregs[REG_CS]     = state->iret.cs;
#endif
 tail.t_ctx.uc_mcontext.gregs[REG_UESP]   = state->iret.userxsp;
 tail.t_ctx.uc_mcontext.gregs[REG_SS]     = state->iret.ss;
 tail.t_ctx.uc_mcontext.cr2               = (__ULONGPTR_TYPE__)t->t_lastcr2;
#endif
 tail.t_ctx.uc_mcontext.gregs[REG_ERR]    = reg_err;
 tail.t_ctx.uc_mcontext.gregs[REG_TRAPNO] = reg_trapno;
 tail.t_ctx.uc_mcontext.gregs[REG_EFL]    = state->iret.xflags;
 assert(tail.t_ctx.uc_mcontext.gregs[REG_EFL]&EFLAGS_IF);
 if (t->t_flags&TASKFLAG_DELAYSIGS)
     tail.t_ctx.uc_mcontext.gregs[REG_EFL] &= ~(EFLAGS_IF);

#ifdef __x86_64__
 /* Setup the first argument for the signal handler. */
 GPREGS_SYSV_ARG1(state->gp) = signal_info->si_signo;
#endif

 tail.t_ctx.uc_link = NULL; /* XXX: Are we supposed to fill this with something? */

 /* TODO: Use sigaltstack() here, if it was ever set! */
 //state->iret.userxsp = GET_SIGALT_STACK();
#if USER_REDZONE_SIZE != 0
 /* Skip memory required for a `red' zone. */
 *(uintptr_t *)&state->iret.userxsp -= USER_REDZONE_SIZE;
#endif

 if (action->sa_flags&SA_SIGINFO) {
  byte_t head_data[offsetof(struct sigenter_fhead,sh_info)];
#define HEAD  (*(struct sigenter_fhead *)head_data)
  state->iret.userxsp -= SIGENTER_FULL_SIZE;
  dst                  = (USER struct sigenter_info *)state->iret.userxsp;
  state->gp.xbp        = (uintptr_t)&dst->se_full.f_tail.t_oldbp;
  HEAD.sh_return       = (void *)&signal_return;
  tail.t_ctx.uc_mcontext.fpregs = &dst->se_full.f_tail.t_ctx.__fpregs_mem;
#ifdef __x86_64__
  GPREGS_SYSV_ARG2(state->gp) = (u64)&dst->se_full.f_info;
  GPREGS_SYSV_ARG3(state->gp) = (u64)&dst->se_full.f_tail.t_ctx;
#else
  HEAD.sh_signo  = signal_info->si_signo;
  HEAD.sh_pinfo  = &dst->se_full.f_info;
  HEAD.sh_pctx   = &dst->se_full.f_tail.t_ctx;
#endif
#undef HEAD
  /* Copy all the collected data onto the user-space stack. */
  TASK_PDIR_BEGIN(omm,t->t_real_mman);
  copy_error = (copy_to_user(&dst->se_full.f_tail,&tail,sizeof(struct sigenter_tail))+
                copy_to_user(&dst->se_full.f_info,signal_info,sizeof(siginfo_t))+
                copy_to_user(dst,head_data,sizeof(head_data)));
  TASK_PDIR_END(omm,t->t_real_mman);
  if (copy_error) goto err_fault;
 } else {
  struct sigenter_bhead head;
  state->iret.userxsp -= SIGENTER_BASE_SIZE;
  dst                  = (USER struct sigenter_info *)state->iret.userxsp;
  state->gp.xbp        = (uintptr_t)&dst->se_base.b_tail.t_oldbp;
  head.sh_return       = (void *)&signal_return; /* Assign the trampoline address. */
  tail.t_ctx.uc_mcontext.fpregs = &dst->se_base.b_tail.t_ctx.__fpregs_mem;
#ifndef __x86_64__
  head.sh_signo  = signal_info->si_signo;
#endif
  /* Copy all the collected data onto the user-space stack. */
  TASK_PDIR_BEGIN(omm,t->t_real_mman);
  copy_error = (copy_to_user(&dst->se_base.b_tail,&tail,sizeof(struct sigenter_tail)) ||
                copy_to_user(dst,&head,sizeof(struct sigenter_bhead)));
  TASK_PDIR_END(omm,t->t_real_mman);
  if (copy_error) goto err_fault;
 }

 /* Finally, redirect the task's XIP pointer to the signal handler. */
 state->iret.xip = (uintptr_t)action->sa_handler;

 return -EOK;
err_fault:
 syslog(LOG_WARN,
        "[SIG] Failed to deliver signal to process %d/%d: Target stack at %p is faulty\n",
       (int)t->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
       (int)t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,dst);
 return -EFAULT;
}

#define SIGENTER_MODE_NORMAL       0 /* Don't modify any registers. */
#define SIGENTER_MODE_RESTORE      1 /* Fill `XAX' with `struct sigenter::se_xax' */
#ifdef __SYSCALL_TYPE_LONGBIT
#define SIGENTER_MODE_RESTORE_LONG 3 /* Fill `XAX' with `struct sigenter::se_xax' and 
                                      * fill `XDX' with `struct sigenter::se_xdx' */
#endif /* __SYSCALL_TYPE_LONGBIT */

STATIC_ASSERT(offsetof(struct sigenter_tail,t_ctx) == SIGENTER_TAIL_OFFSETOF_CTX);
STATIC_ASSERT(offsetof(struct sigenter_tail,t_oldbp) == SIGENTER_TAIL_OFFSETOF_OLDBP);
STATIC_ASSERT(offsetof(struct sigenter_tail,t_oldip) == SIGENTER_TAIL_OFFSETOF_OLDIP);
STATIC_ASSERT(sizeof(struct sigenter_tail) == SIGENTER_TAIL_SIZE);


/* Enter a signal handler without register modifications.
 * NOTE: This is the only sigenter function that must be executed with interrupts enabled. */
INTDEF void ASMCALL sigenter(void);
INTDEF void ASMCALL sigenter_restore(void);
INTDEF void ASMCALL sigenter_restart(void); /* Return -ERESTART instead of -EINTR */
#ifdef __SYSCALL_TYPE_LONGBIT
/* Also restore XDX with `struct sigenter::se_xdx' */
INTDEF void ASMCALL sigenter_restore_long(void);
INTDEF void ASMCALL sigenter_restart_long(void); /* Return -ERESTART instead of -EINTR */
#endif


GLOBAL_ASM(
L(.section .text                                                                 )
#ifdef __SYSCALL_TYPE_LONGBIT
L(PRIVATE_ENTRY(sigenter_restart_long)                                           )
L(    cmpx   $(-EINTR),    %xax                                                  )
L(    jne    sigenter_restore_long                                               )
L(    cmpx   $(-1),        %xdx                                                  )
L(    jne    sigenter_restore_long                                               )
L(    movx   $(-ERESTART), %xax                                                  )
L(    movx   $(-1),        %xdx                                                  )
L(PRIVATE_ENTRY(sigenter_restore_long)                                           )
L(    pushx  $(SIGENTER_MODE_RESTORE_LONG)                                       )
L(    jmp    1f                                                                  )
L(SYM_END(sigenter_restore_long)                                                 )
L(SYM_END(sigenter_restart_long)                                                 )
#endif /* __SYSCALL_TYPE_LONGBIT */
L(PRIVATE_ENTRY(sigenter_restore)                                                )
L(    pushx  $(SIGENTER_MODE_RESTORE)                                            )
L(    jmp    1f                                                                  )
L(PRIVATE_ENTRY(sigenter_restart)                                                )
L(    cmpx   $(-EINTR),    %xax                                                  )
L(    jne    sigenter                                                            )
L(    movx   $(-ERESTART), %xax                                                  )
L(PRIVATE_ENTRY(sigenter)                                                        )
L(    pushx  $(SIGENTER_MODE_NORMAL)                                             )
L(1:                                                                             )
#ifdef __x86_64__
L(    swapgs                                                                     )
L(    sti                                                                        )
#endif
L(    pushx  %xax                                                                )
L(    pushx  %xcx                                                                )
L(    pushx  %xdi                                                                )
L(    pushx  %xsi                                                                )
#ifdef __x86_64__
L(    pushx  %r10                                                                )
L(    pushx  %r11                                                                )
L(    ASM_RDFSBASE(r10)                                                          )
L(    movl   $(IA32_KERNEL_GS_BASE), %ecx                                        )
L(    pushq  %rdx                                                                )
L(    rdmsr                                                                      )
L(    shlq   $32,  %rdx                                                          )
L(    movq   %rdx, %r11                                                          )
L(    orq    %rax, %r11                                                          )
L(    popq   %rdx                                                                )
#define FS_BASE  %r10
#define GS_BASE  %r11
#else
L(    __ASM_PUSH_SGREGS                                                          )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                   )
#endif
L(                                                                               )
#define CALLER     %xsi
#define ITER       %xdi
#define COUNT      %xcx
L(    /* Fill all entires in the chain of raised signals with data. */           )
L(    movx   ASM_CPU(CPU_OFFSETOF_RUNNING),                          CALLER      )
L(    movx   TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_NEXT(CALLER),  ITER        )
L(    movx   TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_COUNT(CALLER), COUNT       )
L(                                                                               )
L(    /* Use a local exception handler to guard against invalid pointers. */     )
L(    pushx  $sigfault                                                           )
L(    pushx  $(EXC_PAGE_FAULT)                                                   )
L(    pushx  $0 /* There musn't been any other handlers left. */                 )
L(    movx   %xsp, TASK_OFFSETOF_IC(CALLER)                                      )
L(                                                                               )
#ifdef __x86_64__
#   define L_OFFSETOF_MODE   (3*XSZ+6*XSZ)
#   define L_OFFSETOF_XAX    (3*XSZ+5*XSZ)
#   define L_OFFSETOF_XCX    (3*XSZ+4*XSZ)
#   define L_OFFSETOF_XDI    (3*XSZ+3*XSZ)
#   define L_OFFSETOF_XSI    (3*XSZ+2*XSZ)
#   define L_OFFSETOF_R10    (3*XSZ+1*XSZ)
#   define L_OFFSETOF_R11    (3*XSZ+0*XSZ)
#else
#   define L_OFFSETOF_SS     (3*XSZ+SGREGS_SIZE+6*XSZ)
/* #define L_OFFSETOF_USERSP (3*XSZ+SGREGS_SIZE+5*XSZ) // Original useresp. - Redundant information left by the original IRET tail. */
#   define L_OFFSETOF_MODE   (3*XSZ+SGREGS_SIZE+4*XSZ)
#   define L_OFFSETOF_XAX    (3*XSZ+SGREGS_SIZE+3*XSZ)
#   define L_OFFSETOF_XCX    (3*XSZ+SGREGS_SIZE+2*XSZ)
#   define L_OFFSETOF_XDI    (3*XSZ+SGREGS_SIZE+1*XSZ)
#   define L_OFFSETOF_XSI    (3*XSZ+SGREGS_SIZE+0*XSZ)
#   define L_OFFSETOF_SGREGS (3*XSZ)
#endif
L(                                                                               )
L(    testb $(SIGENTER_MODE_RESTORE), L_OFFSETOF_MODE(%xsp)                      )
L(    jz    1f                                                                   )
L(    movx  TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XAX(CALLER), %xax           )
L(    movx  %xax, L_OFFSETOF_XAX(%xsp)                                           )
L(1:                                                                             )
#ifdef __SYSCALL_TYPE_LONGBIT
L(    testb $(SIGENTER_MODE_RESTORE_LONG&~(SIGENTER_MODE_RESTORE)), L_OFFSETOF_MODE(%xsp))
L(    jz    1f                                                                   )
L(    movx  TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XDX(CALLER), %xdx           )
L(1:                                                                             )
#endif /* __SYSCALL_TYPE_LONGBIT */
L(    /* Assert that the number of raised signals isn't ZERO(0). */              )
#ifdef CONFIG_DEBUG
L(    testx  %xcx, %xcx                                                          )
L(    jnz    1f                                                                  )
L(    int    $3 /* ASSERTION_FAILURE: 'Signal recursion was ZERO(0)' */          )
L(2:  hlt;   jmp 2b                                                              )
L(1:                                                                             )
#endif /* CONFIG_DEBUG */
L(77:                                                                            )
#ifdef __x86_64__
L(    movabs $(ASM_USER_END - SIGENTER_TAIL_SIZE), %rax                          )
L(    cmpq   %rax,                                 ITER                          )
#else
L(    cmpx   $(ASM_USER_END - SIGENTER_TAIL_SIZE), ITER                          )
#endif
L(    ja     .pointer_out_of_bounds                                              )
L(                                                                               )
#define MCONTEXT(offset) (SIGENTER_TAIL_OFFSETOF_CTX+__UCONTEXT_OFFSETOF_MCONTEXT+offset)(ITER)
#define UCONTEXT_REG(i)   MCONTEXT(__MCONTEXT_OFFSETOF_GREGS+(i)*__SIZEOF_GREG_T__)
L(    movx   %xbp, SIGENTER_TAIL_OFFSETOF_OLDBP(ITER)                            )
L(                                                                               )
#ifdef __x86_64__
L(    movq   FS_BASE, MCONTEXT(__MCONTEXT_OFFSETOF_FS_BASE)                      )
L(    movq   GS_BASE, MCONTEXT(__MCONTEXT_OFFSETOF_GS_BASE)                      )
L(    movq   %r8,  UCONTEXT_REG(REG_R8)  /* R8 */                                )
L(    movq   %r9,  UCONTEXT_REG(REG_R9)  /* R9 */                                )
L(    movx   L_OFFSETOF_R10(%xsp), %xax                                          )
L(    movq   %rax, UCONTEXT_REG(REG_R10) /* R10 */                               )
L(    movx   L_OFFSETOF_R11(%xsp), %xax                                          )
L(    movq   %rax, UCONTEXT_REG(REG_R11) /* R11 */                               )
L(    movq   %r12, UCONTEXT_REG(REG_R12) /* R12 */                               )
L(    movq   %r13, UCONTEXT_REG(REG_R13) /* R13 */                               )
L(    movq   %r14, UCONTEXT_REG(REG_R14) /* R14 */                               )
L(    movq   %r15, UCONTEXT_REG(REG_R15) /* R15 */                               )
L(    movx   L_OFFSETOF_XDI(%xsp), %xax                                          )
L(    movq   %rax, UCONTEXT_REG(REG_RDI) /* RDI */                               )
L(    movx   L_OFFSETOF_XSI(%xsp), %xax                                          )
L(    movq   %rax, UCONTEXT_REG(REG_RSI) /* RSI */                               )
L(    movq   %rbp, UCONTEXT_REG(REG_RBP) /* RBP */                               )
L(    movq   %rbx, UCONTEXT_REG(REG_RBX) /* RBX */                               )
L(    movx   L_OFFSETOF_XCX(%xsp), %xax                                          )
L(    movq   %rax, UCONTEXT_REG(REG_RCX) /* RCX */                               )
L(    movq   %rdx, UCONTEXT_REG(REG_RDX) /* RDX */                               )
L(    movx   L_OFFSETOF_XAX(%xsp), %xax                                          )
L(    movq   %rax, UCONTEXT_REG(REG_RAX) /* RAX */                               )
#else
L(    xorx   %xax, %xax                                                          )
L(    movw   L_OFFSETOF_SGREGS+SGREGS_OFFSETOF_DS(%xsp), %ax                     )
L(    movl   %eax, UCONTEXT_REG(REG_DS)  /* DS */                                )
L(    movw   L_OFFSETOF_SGREGS+SGREGS_OFFSETOF_ES(%xsp), %ax                     )
L(    movl   %eax, UCONTEXT_REG(REG_ES)  /* ES */                                )
L(    movw   L_OFFSETOF_SGREGS+SGREGS_OFFSETOF_FS(%xsp), %ax                     )
L(    movl   %eax, UCONTEXT_REG(REG_FS)  /* FS */                                )
L(    movw   L_OFFSETOF_SGREGS+SGREGS_OFFSETOF_GS(%xsp), %ax                     )
L(    movl   %eax, UCONTEXT_REG(REG_GS)  /* GS */                                )
L(    movx   L_OFFSETOF_XDI(%xsp), %xax                                          )
L(    movl   %eax, UCONTEXT_REG(REG_EDI) /* EDI */                               )
L(    movx   L_OFFSETOF_XSI(%xsp), %xax                                          )
L(    movl   %eax, UCONTEXT_REG(REG_ESI) /* ESI */                               )
L(    movl   %ebp, UCONTEXT_REG(REG_EBP) /* EBP */                               )
L(    movl   %ebx, UCONTEXT_REG(REG_EBX) /* EBX */                               )
L(    movx   L_OFFSETOF_XCX(%xsp), %xax                                          )
L(    movl   %eax, UCONTEXT_REG(REG_ECX) /* ECX */                               )
L(                                                                               )
L(    /* Since we've got redirected to the kernel and this is i386 mode,
       * the bottom of the stack still contains the desired SS value. */         )
L(    movx   L_OFFSETOF_SS(%xsp), %xax                                           )
L(    movl   %eax, UCONTEXT_REG(REG_SS)  /* SS */                                )
L(    movl   %edx, UCONTEXT_REG(REG_EDX) /* EDX */                               )
L(    movx   L_OFFSETOF_XAX(%xsp), %xax                                          )
L(    movl   %eax, UCONTEXT_REG(REG_EAX) /* EAX */                               )
#endif
L(                                                                               )
L(    leax   SIGENTER_TAIL_OFFSETOF_OLDBP(ITER), %xbp                            )
L(    movx   SIGENTER_TAIL_OFFSETOF_CTX+__UCONTEXT_OFFSETOF_LINK(ITER), ITER     )
#if 1
L(    subx   $1, %xcx                                                            )
L(    jnz    77b /* if (--XCX != 0) continue; */                                 )
#else
L(    loop   77b                                                                 )
#endif
L(                                                                               )
L(    /* Reset the number of raised signals now that they're all setup. */       )
L(    movx   $0, TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_COUNT(CALLER)          )
L(    movx   $0, TASK_OFFSETOF_IC(CALLER)                                        )
L(                                                                               )
L(    /* Create an IRET-tail to jump to the top-level signal handler. */         )
L(    pushx  $(__USER_DS)                                            /* ss */    )
L(    pushx  TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XSP(CALLER)    /* userxsp */)
L(    pushx  TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XFLAGS(CALLER) /* xflags */)
L(    pushx  $(__USER_CS)                                            /* cs */    )
L(    pushx  TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XIP(CALLER)    /* xip */   )
L(                                                                               )
L(    /* Load the proper base address. */                                        )
L(    movx   TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_NEXT(CALLER), ITER         )
L(    leax   SIGENTER_TAIL_OFFSETOF_OLDBP(ITER),                    %xbp         )
L(                                                                               )
L(    /* Restore our own working registers. */                                   )
#ifdef __x86_64__
#define SIGENTER_TAIL(offset) ((offset)-SIGENTER_TAIL_OFFSETOF_OLDBP)(%xbp)
L(    movx   SIGENTER_TAIL(SIGENTER_TAIL_OFFSETOF_SIGNO), %xdi /* signo */       )
L(    leax   SIGENTER_TAIL(-__SI_MAX_SIZE),               %xsi /* siginfo_t */   )
L(    leax   SIGENTER_TAIL(SIGENTER_TAIL_OFFSETOF_CTX),   %xdx /* ucontext_t */  )
#else
L(    movx   ((5*XSZ)+L_OFFSETOF_XSI)(%xsp), %xsi                                )
L(    movx   ((5*XSZ)+L_OFFSETOF_XDI)(%xsp), %xdi                                )
#endif
L(    movx   ((5*XSZ)+L_OFFSETOF_XCX)(%xsp), %xcx                                )
L(    movx   ((5*XSZ)+L_OFFSETOF_XAX)(%xsp), %xax                                )
L(                                                                               )
L(    /* Switch segment register values back to user-space mode. */              )
#ifndef __x86_64__
L(    __ASM_LOAD_SGREGS((5*XSZ)+L_OFFSETOF_SGREGS(%xsp))                         )
#else
L(    cli                                                                        )
L(    swapgs                                                                     )
L(    movx   GS_BASE, %r10                                                       )
L(    ASM_WRGSBASE(r10)                                                          )
L(    movq   ((5*XSZ)+L_OFFSETOF_R11)(%rsp), %r11                                )
L(    movq   ((5*XSZ)+L_OFFSETOF_R10)(%rsp), %r10                                )
#endif
L(                                                                               )
L(    /* Finally, perform the IRET to the first signal handler. */               )
L(    ASM_IRET                                                                   )
L(.pointer_out_of_bounds:                                                        )
L(    movx   ITER, TASK_OFFSETOF_LASTCR2(CALLER)                                 )
L(    jmp    sigfault                                                            )
L(SYM_END(sigenter_restore)                                                      )
L(SYM_END(sigenter)                                                    )
L(.previous                                                                      )
#undef GS_BASE
#undef FS_BASE
#undef COUNT
#undef ITER
#undef CALLER
);

INTERN ATTR_NORETURN void KCALL sigfault(void) {
 siginfo_t info;
 assert(!THIS_TASK->t_critical);
 task_crit();
 syslog(LOG_ERROR,"[SIG] Terminating thread %d:%d with faulty signal stack pointer at %p\n",
        GET_THIS_PID(),GET_THIS_TID(),THIS_TASK->t_lastcr2);
 memset(&info,0,sizeof(siginfo_t));
 info.si_signo = SIGSEGV;
 info.si_code  = SEGV_MAPERR;
 coredump_host_task(THIS_TASK,&info,0,0);
 task_endcrit();
 task_terminate(THIS_TASK,(void *)(__WCOREFLAG|__W_EXITCODE(1,0)));
 __builtin_unreachable();
}


PRIVATE errno_t KCALL
deliver_signal_to_task_in_host(struct task *__restrict t,
                               struct sigaction const *__restrict action,
                               siginfo_t const *__restrict signal_info,
                               greg_t reg_trapno, greg_t reg_err) {
 /* XXX: Assume that `t' is currently executing a system-call. */
 USER struct sigenter_info *dst; struct mman *omm;
 struct irregs_syscall *return_registers;
 struct sigenter_tail tail; size_t copy_error;
 assert(!PREEMPTION_ENABLED());
 assert(TASK_CPU(t) == THIS_CPU);
 if (++t->t_sigenter.se_count == 1) {
  /* The first signal enter. */
  void (ASMCALL *used_sigenter)(void);
  byte_t code[2];
  /* Terminate the chain of user-space signal handler contexts.
   * (Thus allowing userspace to detect invocation recursion) */
  t->t_sigenter.se_next = container_of((ucontext_t *)NULL,struct sigenter_tail,t_ctx);
  return_registers = IRREGS_SYSCALL_GET_FOR(t);
  /* Check if the task is currently attempting to execute a system call. */
  used_sigenter = &sigenter;
  if (syscall_is_norestart(return_registers->sysno));
  else {
   TASK_PDIR_BEGIN(omm,t->t_real_mman);
   copy_error = copy_from_user(code,(void *)(return_registers->xip-2),sizeof(code));
   TASK_PDIR_END(omm,t->t_real_mman);
   if (!copy_error && code[0] == 0xcd && code[1] == INTNO_SYSCALL /* int $0x80 */
        /* XXX: Must also detect other ways of invoking system calls here! */) {
    if (action->sa_flags&SA_RESTART) {
     t->t_sigenter.se_xax = return_registers->sysno;
     /* Adjust the instruction pointer to repeat the system-call. */
     return_registers->xip -= 2;
#ifdef __SYSCALL_TYPE_LONGBIT
     if (syscall_is_long(return_registers->sysno)) {
      /* Reverse engineer the original XDX value. (This one's utterly ugly...)
       * NOTE: Because we only do this for long system-calls, which have a very
       *       unique (and most importantly unanimously) calling convention,
       *       we can break the mold by accessing the implementation-specific
       *       scratch-safe register area. */
#ifdef __x86_64__
      t->t_sigenter.se_xdx = *(u64 *)(((uintptr_t)return_registers-__ASM_SCRATCH_NOXAX_SIZE)+
                                                                   __ASM_SCRATCH_NOXAX_OFFSETOF_RDX);
#else
      /* NOTE: The offsets used here reference system-call
       *       stack offsets documented in `__SYSCALL_SDEFINE()' */
#ifdef CONFIG_DEBUG
      /* 5*sizeof(u32) == offsetof(sg_a) - offsetof(edx) */
      t->t_sigenter.se_xdx = *(u32 *)((uintptr_t)return_registers-(SGREGS_SIZE+5*sizeof(u32)));
#else
      /* 4*sizeof(u32) == offsetof(sg_a) - offsetof(edx) */
      t->t_sigenter.se_xdx = *(u32 *)((uintptr_t)return_registers-(SGREGS_SIZE+4*sizeof(u32)));
#endif
#endif
      used_sigenter = &sigenter_restore_long;
     } else
#endif /* __SYSCALL_TYPE_LONGBIT */
     {
      /* Don't attempt to restore long return registers here.
       * Without knowing for certain that the current system-call is actually a long-call,
       * we have no way of knowing where the XDX value is located, it it was even saved at all!
       * Instead, assume that the system call _only_ returns on XAX, which has already been
       * saved above as the lookup process for that register actually is standardized:
       * >> ORIG_EAX = IRREGS_SYSCALL_GET_FOR(t)->sysno; */
      used_sigenter = &sigenter_restore;
     }
    } else {
#ifdef __SYSCALL_TYPE_LONGBIT
     if (syscall_is_long(return_registers->sysno)) {
      used_sigenter = &sigenter_restart_long;
     } else
#endif
     {
      used_sigenter = &sigenter_restart;
     }
    }
   } else {
    /* After not finding `int $0x80', or something equivalent, we must assume
     * that the task isn't actually executing a system-call, meaning we must
     * not attempt to modify any registers once the kernel attempts to switch
     * back to running the signal-handler.
     * This likely easily happens the the we got here because the task threw an
     * exception such as `EXC_DIVIDE_BY_ZERO', in which case we obviously wouldn't
     * find `int $0x80' at the source assembly, and just as well must not attempt
     * to ~restart~ a system call, or modify any registers accordingly. */
    used_sigenter = &sigenter;
   }
  }
  /* Safe the relevant parts of the original IRET tail. */
  t->t_sigenter.se_xip      = return_registers->xip;
  t->t_sigenter.se_xflags   = return_registers->xflags;
  t->t_sigenter.se_xsp      = return_registers->userxsp;
  return_registers->xip     = (uintptr_t)used_sigenter;
  return_registers->cs      = __KERNEL_CS;
#ifdef __x86_64__
  /* x86_64 always requires the full IRET tail,
   * meaning we must update these members, too. */
  return_registers->userrsp = (u64)(return_registers+1);
  return_registers->ss      = __KERNEL_DS;
#endif

#ifdef __x86_64__ /* Must disable interrupts before `swapgs' is called. */
  return_registers->xflags = 0;
#else
  return_registers->xflags = EFLAGS_IF;
#endif
 } else {
  /* Secondary signal. */
 }

 tail.t_oldip = (USER void *)t->t_sigenter.se_xip;
 tail.t_ctx.uc_flags = 0;

 /* Save stack information. */
 if (t->t_ustack) {
  tail.t_ctx.uc_stack.ss_sp    = t->t_ustack->s_begin;
  tail.t_ctx.uc_stack.ss_size  = ((uintptr_t)t->t_ustack->s_end-
                                  (uintptr_t)t->t_ustack->s_begin);
  tail.t_ctx.uc_stack.ss_flags = SS_ONSTACK;
 } else {
  tail.t_ctx.uc_stack.ss_sp    = (void *)t->t_sigenter.se_xsp;
  tail.t_ctx.uc_stack.ss_size  = 0;
  tail.t_ctx.uc_stack.ss_flags = SS_DISABLE;
 }

 /* Save signal masking information. */
 memcpy(&tail.t_ctx.uc_sigmask,
        &t->t_sigblock,sizeof(sigset_t));

 /* Save FPU information. */
 safe_fpu(&tail.t_ctx.__fpregs_mem,t->t_arch.at_fpu);

 /* Fill in registers that we already know about. */
 tail.t_ctx.uc_mcontext.gregs[REG_EFL]    = t->t_sigenter.se_xflags;
 tail.t_ctx.uc_mcontext.gregs[REG_ERR]    = reg_err;
 tail.t_ctx.uc_mcontext.gregs[REG_TRAPNO] = reg_trapno;
#ifdef __x86_64__
 tail.t_ctx.uc_mcontext.gregs[REG_RIP]    = t->t_sigenter.se_xip;
 tail.t_ctx.uc_mcontext.gregs[REG_RSP]    = t->t_sigenter.se_xsp;
 tail.t_ctx.uc_mcontext.gregs[REG_CSGSFS] = (((u64)__USER_CS)|
                                             ((u64)__USER_GS << 16)|
                                             ((u64)__USER_FS << 32));
 tail.t_ctx.uc_mcontext.gregs[REG_OLDMASK] = *(s64 *)&t->t_sigblock;
 tail.t_ctx.uc_mcontext.gregs[REG_CR2]     = (u64)t->t_lastcr2;
#else
 tail.t_ctx.uc_mcontext.gregs[REG_EIP]  = t->t_sigenter.se_xip;
 tail.t_ctx.uc_mcontext.gregs[REG_ESP]  = t->t_sigenter.se_xsp;
 tail.t_ctx.uc_mcontext.gregs[REG_UESP] = t->t_sigenter.se_xsp;
 tail.t_ctx.uc_mcontext.gregs[REG_CS]   = __USER_CS;
 tail.t_ctx.uc_mcontext.cr2             = (__ULONGPTR_TYPE__)t->t_lastcr2;
#endif

 if (t->t_sigenter.se_count == 1) {
  /* TODO: Use sigaltstack() here, if it was ever set! */
  //t->t_sigenter.se_xsp = GET_SIGALT_STACK();
#if USER_REDZONE_SIZE != 0
  /* Skip memory required for the `red' zone when pushing the first handler. */
  t->t_sigenter.se_xsp -= USER_REDZONE_SIZE;
#endif
 }

#define NEXT_SIGNAL \
  container_of(t->t_sigenter.se_next,struct sigenter_tail,t_ctx)

#ifdef __x86_64__
 tail.t_signo = signal_info->si_signo;
#endif /* __x86_64__ */

 /* Link the ucontext structures together. */
 tail.t_ctx.uc_link = &t->t_sigenter.se_next->t_ctx;
 if (action->sa_flags&SA_SIGINFO) {
  byte_t head_data[offsetof(struct sigenter_fhead,sh_info)];
#define HEAD  (*(struct sigenter_fhead *)head_data)
  t->t_sigenter.se_xsp         -= SIGENTER_FULL_SIZE;
  dst                           = (USER struct sigenter_info *)t->t_sigenter.se_xsp;
  tail.t_ctx.uc_mcontext.fpregs = &dst->se_full.f_tail.t_ctx.__fpregs_mem;
  t->t_sigenter.se_next         = &dst->se_full.f_tail;
  HEAD.sh_return                = (void *)&signal_return;
#ifndef __x86_64__
  HEAD.sh_signo  = signal_info->si_signo;
  HEAD.sh_pinfo  = &dst->se_full.f_info;
  HEAD.sh_pctx   = &dst->se_full.f_tail.t_ctx;
#endif
#undef HEAD
  /* Copy all the collected data onto the user-space stack. */
  TASK_PDIR_BEGIN(omm,t->t_real_mman);
  copy_error = (copy_to_user(&dst->se_full.f_tail,&tail,sizeof(struct sigenter_tail))+
                copy_to_user(&dst->se_full.f_info,signal_info,sizeof(siginfo_t))+
                copy_to_user(dst,head_data,sizeof(head_data)));
  TASK_PDIR_END(omm,t->t_real_mman);
  if (copy_error) goto err_fault;
 } else {
  struct sigenter_bhead head;
  t->t_sigenter.se_xsp         -= SIGENTER_BASE_SIZE;
  dst                           = (USER struct sigenter_info *)t->t_sigenter.se_xsp;
  head.sh_return                = (void *)&signal_return; /* Assign the trampoline address. */
  tail.t_ctx.uc_mcontext.fpregs = &dst->se_base.b_tail.t_ctx.__fpregs_mem;
  t->t_sigenter.se_next         = &dst->se_base.b_tail;
#ifndef __x86_64__
  head.sh_signo = signal_info->si_signo;
#endif
  /* Copy all the collected data onto the user-space stack. */
  TASK_PDIR_BEGIN(omm,t->t_real_mman);
  copy_error = (copy_to_user(&dst->se_base.b_tail,&tail,sizeof(struct sigenter_tail)) ||
                copy_to_user(dst,&head,sizeof(struct sigenter_bhead)));
  TASK_PDIR_END(omm,t->t_real_mman);
  if (copy_error) goto err_fault;
 }

 /* Userspace will return to the handler of this action. */
 t->t_sigenter.se_xip = (register_t)action->sa_handler;

 return -EOK;
err_fault:
 syslog(LOG_WARN,
        "[SIG] Failed to deliver signal to process %d/%d: Target stack at %p is faulty\n",
       (int)t->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
       (int)t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,dst);
 return -EFAULT;
}



#else /* !CONFIG_USE_OLD_SIGNALS */
STATIC_ASSERT(offsetof(struct sigenter_info,ei_return) == SIGENTER_INFO_OFFSETOF_RETURN);
STATIC_ASSERT(offsetof(struct sigenter_info,ei_signo) == SIGENTER_INFO_OFFSETOF_SIGNO);
STATIC_ASSERT(offsetof(struct sigenter_info,ei_pinfo) == SIGENTER_INFO_OFFSETOF_PINFO);
STATIC_ASSERT(offsetof(struct sigenter_info,ei_pctx) == SIGENTER_INFO_OFFSETOF_PCTX);
STATIC_ASSERT(offsetof(struct sigenter_info,ei_old_xbp) == SIGENTER_INFO_OFFSETOF_OLD_XBP);
STATIC_ASSERT(offsetof(struct sigenter_info,ei_old_xip) == SIGENTER_INFO_OFFSETOF_OLD_XIP);
STATIC_ASSERT(offsetof(struct sigenter_info,ei_info) == SIGENTER_INFO_OFFSETOF_INFO);
STATIC_ASSERT(offsetof(struct sigenter_info,ei_ctx) == SIGENTER_INFO_OFFSETOF_CTX);
STATIC_ASSERT(sizeof(struct sigenter_info) == SIGENTER_INFO_SIZE);


PRIVATE void KCALL
ucontext_from_usertask(ucontext_t *__restrict result,
                       struct task *__restrict t,
                       greg_t reg_trapno, greg_t reg_err);
PRIVATE void KCALL
fpstate_from_task(struct _libc_fpstate *__restrict result,
                  struct task *__restrict t);

PRIVATE void KCALL
coredump_user_task(struct task *__restrict t,
                   siginfo_t const *__restrict reason,
                   greg_t reg_trapno, greg_t reg_err) {
 ucontext_t context;
 ucontext_from_usertask(&context,t,reg_trapno,reg_err);
 core_dodump(t->t_real_mman,t,&context,reason,COREDUMP_FLAG_NORMAL);
}
PRIVATE void KCALL
coredump_host_task(struct task *__restrict t,
                   siginfo_t const *__restrict reason,
                   greg_t reg_trapno, greg_t reg_err) {
 ucontext_t ctx;
#if defined(__i386__) || defined(__x86_64__)
#undef __STACKBASE_TASK
#define __STACKBASE_TASK     t
 ctx.uc_flags = 0; /* ??? */
 ctx.uc_link  = NULL;
 if (t->t_ustack) {
  ctx.uc_stack.ss_sp    = (void *)t->t_ustack->s_begin;
  ctx.uc_stack.ss_size  = ((uintptr_t)t->t_ustack->s_end-
                           (uintptr_t)t->t_ustack->s_begin);
  ctx.uc_stack.ss_flags = 0;
 } else {
  ctx.uc_stack.ss_sp    = THIS_SYSCALL_REAL_USERXSP;
  ctx.uc_stack.ss_size  = 0;
  ctx.uc_stack.ss_flags = 0;
 }
 /* Fill in what we still know about user-space registers.
  * XXX: The way this function acquires user-space registers isn't quite safe... */
#ifdef __x86_64__
 /* TODO: Override the return address to get all registers. */
#ifdef CONFIG_USE_OLD_SYSCALL
 ctx.uc_mcontext.gregs[REG_CSGSFS] = ((greg_t)THIS_SYSCALL_REAL_CS |
                                     ((greg_t)THIS_SYSCALL_GS << 16) |
                                     ((greg_t)THIS_SYSCALL_FS << 24));
 ctx.uc_mcontext.gregs[REG_R9]     = THIS_SYSCALL_R9;
 ctx.uc_mcontext.gregs[REG_R8]     = THIS_SYSCALL_R8;
 ctx.uc_mcontext.gregs[REG_R10]    = THIS_SYSCALL_R10;
 ctx.uc_mcontext.gregs[REG_RDX]    = THIS_SYSCALL_RDX;
 ctx.uc_mcontext.gregs[REG_RSI]    = THIS_SYSCALL_RSI;
 ctx.uc_mcontext.gregs[REG_RDI]    = THIS_SYSCALL_RDI;
#endif
 ctx.uc_mcontext.gregs[REG_RSP]    = (greg_t)THIS_SYSCALL_REAL_USERXSP;
 ctx.uc_mcontext.gregs[REG_RAX]    = -EOK; /* Simulate an OK system call. */
 ctx.uc_mcontext.gregs[REG_RIP]    = (greg_t)THIS_SYSCALL_REAL_XIP;
#else
 ctx.uc_mcontext.cr2               = (unsigned long int)t->t_lastcr2;
 ctx.uc_mcontext.oldmask           = 0; /* ??? */
 /* TODO: Override the return address to get all registers. */
#ifdef CONFIG_USE_OLD_SYSCALL
 ctx.uc_mcontext.gregs[REG_GS]     = (greg_t)THIS_SYSCALL_GS;
 ctx.uc_mcontext.gregs[REG_FS]     = (greg_t)THIS_SYSCALL_FS;
 ctx.uc_mcontext.gregs[REG_ES]     = (greg_t)THIS_SYSCALL_ES;
 ctx.uc_mcontext.gregs[REG_DS]     = (greg_t)THIS_SYSCALL_DS;
 ctx.uc_mcontext.gregs[REG_EDI]    = THIS_SYSCALL_EDI;
 ctx.uc_mcontext.gregs[REG_ESI]    = THIS_SYSCALL_ESI;
 ctx.uc_mcontext.gregs[REG_EBP]    = THIS_SYSCALL_EBP;
 ctx.uc_mcontext.gregs[REG_ESP]    = (greg_t)THIS_SYSCALL_REAL_USERXSP;
 ctx.uc_mcontext.gregs[REG_EBX]    = THIS_SYSCALL_EBX;
 ctx.uc_mcontext.gregs[REG_EDX]    = THIS_SYSCALL_EDX;
 ctx.uc_mcontext.gregs[REG_ECX]    = THIS_SYSCALL_ECX;
 ctx.uc_mcontext.gregs[REG_EAX]    = -EOK; /* Simulate an OK system call. */
 ctx.uc_mcontext.gregs[REG_EIP]    = (greg_t)THIS_SYSCALL_REAL_XIP;
 ctx.uc_mcontext.gregs[REG_CS]     = THIS_SYSCALL_REAL_CS;
 ctx.uc_mcontext.gregs[REG_UESP]   = ctx.uc_mcontext.gregs[REG_ESP];
 ctx.uc_mcontext.gregs[REG_SS]     = THIS_SYSCALL_REAL_SS;
#endif
#endif
 ctx.uc_mcontext.gregs[REG_TRAPNO] = reg_trapno;
 ctx.uc_mcontext.gregs[REG_ERR]    = reg_err;
#ifdef CONFIG_USE_OLD_SYSCALL
 ctx.uc_mcontext.gregs[REG_EFL]    = THIS_SYSCALL_REAL_XFLAGS;
#endif
 ctx.uc_mcontext.fpregs = &ctx.__fpregs_mem;
 fpstate_from_task(&ctx.__fpregs_mem,t);
 memcpy(&ctx.uc_sigmask,&t->t_sigblock,sizeof(sigset_t));
#undef __STACKBASE_TASK
#define __STACKBASE_TASK     THIS_TASK
#else
#error FIXME
#endif
 core_dodump(t->t_real_mman,t,&ctx,reason,COREDUMP_FLAG_NORMAL);
}


/* Exception handler for managing broken signal stacks. */
INTERN ATTR_NORETURN void KCALL sigfault(void) {
 siginfo_t info;
 assert(!THIS_TASK->t_critical);
 syslog(LOG_ERROR,"[SIG] Terminating thread %d:%d with faulty signal stack pointer at %p\n",
        GET_THIS_PID(),GET_THIS_TID(),THIS_TASK->t_lastcr2);
 memset(&info,0,sizeof(siginfo_t));
 info.si_signo = SIGSEGV;
 info.si_code  = SEGV_MAPERR;
 coredump_host_task(THIS_TASK,&info,0,0);
 task_terminate(THIS_TASK,(void *)(__WCOREFLAG|__W_EXITCODE(1,0)));
 __builtin_unreachable();
}
INTERN ATTR_NORETURN void KCALL sigill(char const *__restrict format, ...) {
 siginfo_t info; va_list args;
 assert(!THIS_TASK->t_critical);
 syslog(LOG_ERROR,"[SIG] Terminating thread %d:%d with illegal signal context: Invalid ",
        GET_THIS_PID(),GET_THIS_TID());
 va_start(args,format);
 vsyslog(LOG_DEBUG,format,args);
 va_end(args);
 syslog(LOG_ERROR,"\n");

 /* Create a core dump for the thread. */
 memset(&info,0,sizeof(siginfo_t));
 info.si_signo = SIGILL;
 info.si_code  = ILL_PRVREG;
 coredump_host_task(THIS_TASK,&info,0,0);

 task_terminate(THIS_TASK,(void *)(__WCOREFLAG|__W_EXITCODE(2,0)));
 __builtin_unreachable();
}

FUNDEF ATTR_NORETURN void ASMCALL sigenter(void);
STATIC_ASSERT(offsetof(struct task,t_sigenter) == TASK_OFFSETOF_SIGENTER);
STATIC_ASSERT(offsetof(struct task,t_sigenter.se_count) == TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_COUNT);
STATIC_ASSERT(offsetof(struct task,t_sigenter.se_userxsp) == TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_USERXSP);

GLOBAL_ASM(
L(.section .text                                                                 )
L(PUBLIC_ENTRY(sigenter)                                                         )
      /* Having gotten here, out stack is currently undefined, but we know that
       * we're still in kernel-space and that accessing per-cpu data will be OK.
       * In addition, all main registers have been restored to what they're ought
       * to be once we enventually do return to user-space. */
L(    __ASM_PUSH_SGREGS                                                          )
L(    pushx %xax                                                                 )
L(    pushx %xbx                                                                 )
L(    pushx %xcx                                                                 )
L(    pushx %xdx                                                                 )
#ifdef __x86_64__
#define __SREGS_OFFSET 32
#else
#define __SREGS_OFFSET 16
#endif
L(                                                                               )
L(    /* Re-load kernel segment registers */                                     )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                   )
L(                                                                               )
L(    movx ASM_CPU(CPU_OFFSETOF_RUNNING), %xax                                   )
L(                                                                               )
      /* Load the address of the user-space stack containing the signal-enter
       * descriptor, as well as how many recursive signal contexts we must fill in. */
L(    movx  (TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_USERXSP)(%xax), %xbx       )
L(    movx  (TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_COUNT)(%xax), %xcx         )
L(    movx  $0, (TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_COUNT)(%xax)           )
#ifdef CONFIG_DEBUG
L(    testx %xcx, %xcx                                                           )
L(    jnz   1f                                                                   )
L(    int   $3 /* ASSERTION_FAILURE: 'Signal recursion was ZERO(0)' */           )
L(2:  hlt                                                                        )
L(    jmp 2b                                                                     )
L(1:                                                                             )
#endif /* CONFIG_DEBUG */
L(                                                                               )
      /* As we're about to start working with the user-space stack, filling in
       * remaining register data may cause a segfault which we must handle. */
L(    pushx_sym(%rdx,sigfault)                                                   )
L(    pushx $(EXC_PAGE_FAULT)                                                    )
L(    pushx $0 /* TASK_OFFSETOF_IC(%xax) -- There mustn't be any more */         )
L(    movx  %xsp, TASK_OFFSETOF_IC(%xax)                                         )
L(                                                                               )
      /* Fill in all the missing user-space registers. */
#define REG(i) (SIGENTER_INFO_OFFSETOF_CTX + \
                   __UCONTEXT_OFFSETOF_MCONTEXT + \
                   __MCONTEXT_OFFSETOF_GREGS+(i)*XSZ)(%xbx)
L(1:  pushx %xdi                                                                 )
L(    movx  %xbx, %xdi                                                           )
L(    addx  $(__UCONTEXT_SIZE), %xdi                                             )
L(    jo    9f                                                                   )
#ifdef __x86_64__
L(    movabs $(ASM_USER_END), %rdx                                               )
L(    cmpq  %rdx, %rdi                                                           )
#else
L(    cmpl  $(ASM_USER_END), %edi                                                )
#endif
L(    ja    10f                                                                  )
L(    popx  %xdi                                                                 )
#ifdef __x86_64__
#if 0 /* XXX: FS/GS base? */
L(    movw  (7*XSZ+SGREGS_OFFSETOF_GS)(%rsp), %dx /* GS */                       )
L(    movw  %dx,  2+REG(REG_CSGSFS)                                              )
L(    movw  (7*XSZ+SGREGS_OFFSETOF_FS)(%rsp), %dx /* FS */                       )
L(    movw  %dx,  4+REG(REG_CSGSFS)                                              )
#endif
L(    movq  %r15, REG(REG_R15)                                                   )
L(    movq  %r14, REG(REG_R14)                                                   )
L(    movq  %r13, REG(REG_R13)                                                   )
L(    movq  %r12, REG(REG_R12)                                                   )
L(    movq  %r11, REG(REG_R11)                                                   )
L(    movq  %r10, REG(REG_R10)                                                   )
L(    movq  %r9,  REG(REG_R9)                                                    )
L(    movq  %r8,  REG(REG_R8)                                                    )
L(    movq  %rdi, REG(REG_RDI)                                                   )
L(    movq  %rsi, REG(REG_RSI)                                                   )
L(    movq  %rbp, REG(REG_RBP)                                                   )
L(    movq  (3*XSZ)(%rsp), %rdx    /* RDX */                                     )
L(    movq  %rdx, REG(REG_RDX)                                                   )
L(    movq  (4*XSZ)(%rsp), %rdx    /* RCX */                                     )
L(    movq  %rdx, REG(REG_RCX)                                                   )
L(    movq  (5*XSZ)(%rsp), %rdx    /* RBX */                                     )
L(    movq  %rdx, REG(REG_RBX)                                                   )
L(    movq  (6*XSZ)(%rsp), %rdx    /* RAX */                                     )
L(    movq  %rdx, REG(REG_RAX)                                                   )
      /* Already filled: REG_RIP */
      /* Already filled: REG_EFL */
#else
L(    movw  (7*XSZ+SGREGS_OFFSETOF_GS)(%esp), %dx /* GS */                       )
L(    movw  %dx,  REG(REG_GS)                                                    )
L(    movw  (7*XSZ+SGREGS_OFFSETOF_FS)(%esp), %dx /* FS */                       )
L(    movw  %dx,  REG(REG_FS)                                                    )
L(    movw  (7*XSZ+SGREGS_OFFSETOF_ES)(%esp), %dx /* ES */                       )
L(    movw  %dx,  REG(REG_ES)                                                    )
L(    movw  (7*XSZ+SGREGS_OFFSETOF_DS)(%esp), %dx /* DS */                       )
L(    movw  %dx,  REG(REG_DS)                                                    )
L(    movl  %edi, REG(REG_EDI)                                                   )
L(    movl  %esi, REG(REG_ESI)                                                   )
L(    movl  %ebp, REG(REG_EBP)                                                   )
L(    movl  (3*XSZ)(%esp), %edx    /* EDX */                                     )
L(    movl  %edx, REG(REG_EDX)                                                   )
L(    movl  (4*XSZ)(%esp), %edx    /* ECX */                                     )
L(    movl  %edx, REG(REG_ECX)                                                   )
L(    movl  (5*XSZ)(%xsp), %edx    /* EBX */                                     )
L(    movl  %edx, REG(REG_EBX)                                                   )
L(    movl  (6*XSZ)(%xsp), %edx    /* EAX */                                     )
L(    movl  %edx, REG(REG_EAX)                                                   )
      /* Already filled: REG_EIP */
      /* Already filled: REG_CS */
      /* Already filled: REG_EFL */
      /* Already filled: REG_UESP */
      /* Already filled: REG_SS */
#endif
L(    movx  %xbp, SIGENTER_INFO_OFFSETOF_OLD_XBP(%xbx)                           )
L(    subx  $1, %xcx                                                             )
L(    jz    2f                                                                   )
      /* Recursively fill secondary context structures. */
#ifdef __x86_64__
L(    movx  REG(REG_RSP), %xbx                                                   )
#else
L(    movx  REG(REG_UESP), %xbx                                                  )
#endif
L(    jmp   1b                                                                   )
#undef REG                   
L(2:  movx  $0, TASK_OFFSETOF_IC(%xax)                                           )
      /* At this point, all context structures are in a valid state.
       * >> Now it's time to setup the last jump to the first signal handler! */
#define SIGENTER(x) (TASK_OFFSETOF_SIGENTER+x)(%xax)
L(    pushx SIGENTER(SIGENTER_OFFSETOF_SS)                                       )
L(    pushx SIGENTER(SIGENTER_OFFSETOF_USERXSP)                                  )
L(    pushx SIGENTER(SIGENTER_OFFSETOF_XFLAGS)                                   )
L(    pushx SIGENTER(SIGENTER_OFFSETOF_CS)                                       )
L(    pushx SIGENTER(SIGENTER_OFFSETOF_XIP)                                      )
#undef SIGENTER
      /* To prevent kernel data leaks, we must re-initialize all registers we've modified.
       * Also: Store a pointer to 'ei_old_xbp' in EBP to fix user-space tracebacks. */
L(    movx  (TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_USERXSP)(%xax), %xbx       )
L(    leax  SIGENTER_INFO_OFFSETOF_OLD_XBP(%xbx), %xbp                           )
L(    movx   (8*XSZ)(%xsp), %xdx                                                 )
L(    movx   (9*XSZ)(%xsp), %xcx                                                 )
L(    movx  (10*XSZ)(%xsp), %xbx                                                 )
L(    movx  (11*XSZ)(%xsp), %xax                                                 )
L(    __ASM_LOAD_SGREGS32((12*XSZ)(%xsp))                                        )
L(                                                                               )
L(    /* Now just jump back to user-space. */                                    )
L(    ASM_IRET                                                                   )
L(                                                                               )
L(    /* Handle illegal-user-stack errors. */                                    )
L(9:  movx %xbx, TASK_OFFSETOF_LASTCR2(%xax)                                     )
L(    call sigfault                                                              )
L(10: movx %xdi, TASK_OFFSETOF_LASTCR2(%xax)                                     )
L(    call sigfault                                                              )
L(SYM_END(sigenter)                                                              )
L(.previous                                                                      )
);



INTDEF void (ASMCALL signal_return)(void);

PRIVATE void KCALL
fpstate_from_task(struct _libc_fpstate *__restrict result,
                  struct task *__restrict t) {
#ifndef CONFIG_NO_FPU
 if (t == THIS_TASK) {
  /* TODO: Update/safe current register state. */
 }
 if (t->t_arch.at_fpu != NULL) {
#ifdef __x86_64__
  STATIC_ASSERT(sizeof(struct fpustate) ==
                sizeof(struct _libc_fpstate));
  memcpy(result,t->t_arch.at_fpu,sizeof(struct fpustate));
#else
  struct _libc_fpreg *dst,*end; struct fpu_reg *src;
  result->cw      = t->t_arch.at_fpu->fp_fcw;
  result->sw      = t->t_arch.at_fpu->fp_fsw;
  result->tag     = t->t_arch.at_fpu->fp_ftw;
  result->ipoff   = t->t_arch.at_fpu->fp_fpuip;
  result->cssel   = t->t_arch.at_fpu->fp_fpucs;
  result->dataoff = t->t_arch.at_fpu->fp_fpudp;
  result->datasel = t->t_arch.at_fpu->fp_fpuds;
  result->status  = result->sw; /* ??? */
  end = (dst = result->_st)+COMPILER_LENOF(result->_st);
  src = t->t_arch.at_fpu->fp_regs;
  for (; dst != end; ++dst,++src) memcpy(dst,src,sizeof(src->f_data));
#endif
 } else
#endif
 {
  memset(result,0,sizeof(struct _libc_fpstate)); /* TODO */
 }
}

PRIVATE void KCALL
ucontext_from_usertask(ucontext_t *__restrict result,
                       struct task *__restrict t,
                       greg_t reg_trapno, greg_t reg_err) {
 struct cpustate *cs_descr = t->t_cstate;
 result->uc_flags = 0; /* ??? */
 result->uc_link  = NULL;
 if (t->t_ustack) {
  result->uc_stack.ss_sp    = (void *)t->t_ustack->s_begin;
  result->uc_stack.ss_size  = ((uintptr_t)t->t_ustack->s_end-
                               (uintptr_t)t->t_ustack->s_begin);
  result->uc_stack.ss_flags = 0;
 } else {
  result->uc_stack.ss_sp    = (void *)cs_descr->iret.userxsp;
  result->uc_stack.ss_size  = 0;
  result->uc_stack.ss_flags = 0;
 }
#ifdef __x86_64__
 result->uc_mcontext.gregs[REG_R8]      = cs_descr->gp.r8;
 result->uc_mcontext.gregs[REG_R9]      = cs_descr->gp.r9;
 result->uc_mcontext.gregs[REG_R10]     = cs_descr->gp.r10;
 result->uc_mcontext.gregs[REG_R11]     = cs_descr->gp.r11;
 result->uc_mcontext.gregs[REG_R12]     = cs_descr->gp.r12;
 result->uc_mcontext.gregs[REG_R13]     = cs_descr->gp.r13;
 result->uc_mcontext.gregs[REG_R14]     = cs_descr->gp.r14;
 result->uc_mcontext.gregs[REG_R15]     = cs_descr->gp.r15;
 result->uc_mcontext.gregs[REG_RDI]     = cs_descr->gp.rdi;
 result->uc_mcontext.gregs[REG_RSI]     = cs_descr->gp.rsi;
 result->uc_mcontext.gregs[REG_RBP]     = cs_descr->gp.rbp;
 result->uc_mcontext.gregs[REG_RBX]     = cs_descr->gp.rbx;
 result->uc_mcontext.gregs[REG_RDX]     = cs_descr->gp.rdx;
 result->uc_mcontext.gregs[REG_RAX]     = cs_descr->gp.rax;
 result->uc_mcontext.gregs[REG_RCX]     = cs_descr->gp.rcx;
 result->uc_mcontext.gregs[REG_RSP]     = cs_descr->iret.userrsp;
 result->uc_mcontext.gregs[REG_RIP]     = cs_descr->iret.rip;
 result->uc_mcontext.gregs[REG_EFL]     = cs_descr->iret.eflags;
 result->uc_mcontext.gregs[REG_CSGSFS]  = (((u64)cs_descr->iret.cs) |
                                           ((u64)__USER_GS << 16) |
                                           ((u64)__USER_FS << 32));
 result->uc_mcontext.gregs[REG_ERR]     = reg_err;
 result->uc_mcontext.gregs[REG_TRAPNO]  = reg_trapno;
 result->uc_mcontext.gregs[REG_OLDMASK] = 0; /* ??? */
 result->uc_mcontext.gregs[REG_CR2]     = (u64)t->t_lastcr2;
 fpstate_from_task(&result->__fpregs_mem,t);
 memcpy(&result->uc_sigmask,&t->t_sigblock,sizeof(sigset_t));
#elif defined(__i386__)
 result->uc_mcontext.cr2               = (__ULONGPTR_TYPE__)t->t_lastcr2;
 result->uc_mcontext.oldmask           = 0; /* ??? */
 result->uc_mcontext.gregs[REG_GS]     = (u32)cs_descr->sg.gs;
 result->uc_mcontext.gregs[REG_FS]     = (u32)cs_descr->sg.fs;
 result->uc_mcontext.gregs[REG_ES]     = (u32)cs_descr->sg.es;
 result->uc_mcontext.gregs[REG_DS]     = (u32)cs_descr->sg.ds;
 result->uc_mcontext.gregs[REG_EDI]    = cs_descr->gp.edi;
 result->uc_mcontext.gregs[REG_ESI]    = cs_descr->gp.esi;
 result->uc_mcontext.gregs[REG_EBP]    = cs_descr->gp.ebp;
 result->uc_mcontext.gregs[REG_ESP]    = cs_descr->gp.esp;
 result->uc_mcontext.gregs[REG_EBX]    = cs_descr->gp.ebx;
 result->uc_mcontext.gregs[REG_EDX]    = cs_descr->gp.edx;
 result->uc_mcontext.gregs[REG_ECX]    = cs_descr->gp.ecx;
 result->uc_mcontext.gregs[REG_EAX]    = cs_descr->gp.eax;
 result->uc_mcontext.gregs[REG_TRAPNO] = reg_trapno;
 result->uc_mcontext.gregs[REG_ERR]    = reg_err;
 result->uc_mcontext.gregs[REG_EIP]    = cs_descr->iret.eip;
 result->uc_mcontext.gregs[REG_CS]     = cs_descr->iret.cs;
 result->uc_mcontext.gregs[REG_EFL]    = cs_descr->iret.eflags;
 result->uc_mcontext.gregs[REG_UESP]   = cs_descr->iret.useresp;
 result->uc_mcontext.gregs[REG_SS]     = cs_descr->iret.ss;
 fpstate_from_task(&result->__fpregs_mem,t);
 memcpy(&result->uc_sigmask,&t->t_sigblock,sizeof(sigset_t));
#else
#error FIXME
#endif
}


PRIVATE errno_t KCALL
deliver_signal_to_task_in_user(struct task *__restrict t,
                               struct sigaction const *__restrict action,
                               siginfo_t const *__restrict signal_info,
                               greg_t reg_trapno, greg_t reg_err) {
 struct cpustate *cs_descr;
 USER struct sigenter_info *user_info;
 struct sigenter_info info;
 /* This is some thread that was pre-empted while in user-space.
  * >> Therefor, `t_cstate' describes the CPU state before it was pre-empted. */
 cs_descr = t->t_cstate;


 /* TODO: Use sigaltstack() here, if it was ever set! */
 user_info = ((USER struct sigenter_info *)cs_descr->iret.userxsp)-1;
#if USER_REDZONE_SIZE != 0
 /* Skip memory required for a `red' zone. */
 *(uintptr_t *)&user_info -= USER_REDZONE_SIZE;
#endif

 ucontext_from_usertask(&info.ei_ctx,t,reg_trapno,reg_err);
 info.ei_ctx.uc_mcontext.fpregs = &user_info->ei_ctx.__fpregs_mem;

 memset((byte_t *)&info.__ei_info_pad+sizeof(siginfo_t),0,__SI_MAX_SIZE-sizeof(siginfo_t));
 memcpy(&info.ei_info,signal_info,sizeof(siginfo_t));
 info.ei_return  = (USER void *)&signal_return;
 info.ei_signo   = signal_info->si_signo;
 info.ei_pinfo   = &user_info->ei_info;
 info.ei_pctx    = &user_info->ei_ctx;
 info.ei_old_xbp = (USER void *)cs_descr->gp.xbp;
 info.ei_old_xip = (USER void *)cs_descr->iret.xip;
 /* Copy all the information we've gathered onto the user-space stack. */
 { struct mman *omm;
   size_t copy_error;
   TASK_PDIR_BEGIN(omm,t->t_real_mman);
   copy_error = copy_to_user(user_info,&info,sizeof(info));
   TASK_PDIR_END(omm,t->t_real_mman);
   if unlikely(copy_error) {
    syslog(LOG_WARN,
           "[SIG] Failed to deliver signal to process %d/%d: Target stack at %p is faulty\n",
          (int)t->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
          (int)t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,user_info);
    return -EFAULT;
   }
 }

 /* Setup the register state to-be used when the task will execute.
  * NOTE: We keep all registers but EBP, EIP and ESP
  *       as they were before the interrupt occurred. */
 cs_descr->gp.xbp       = (uintptr_t)&user_info->ei_old_xbp;
 cs_descr->iret.xip     = (uintptr_t)action->sa_handler;
 cs_descr->iret.userxsp = (uintptr_t)user_info;

 /* Interrupt (wake) the task, so-as to execute the signal handler. */
 return -EOK;
}


PRIVATE errno_t KCALL
deliver_signal_to_task_in_host(struct task *__restrict t,
                               struct sigaction const *__restrict action,
                               siginfo_t const *__restrict signal_info,
                               greg_t reg_trapno, greg_t reg_err) {
#ifdef CONFIG_USE_OLD_SYSCALL
 struct syscall_descr *ss_descr;
#else
 struct irregs_syscall *ss_descr;
#endif
 USER struct sigenter_info *user_info;
 struct sigenter_info info;

#if 0
 if (t != THIS_TASK) {
  debug_tbprintl((void *)t->t_cstate->iret.xip,
                 (void *)t->t_cstate->gp.xbp,0);
 }
#endif

 /* This is us, or the task was pre-empted while in kernel-space. */
#ifdef CONFIG_USE_OLD_SYSCALL
 ss_descr = (struct syscall_descr *)t->t_hstack.hs_end-1;
#else
 ss_descr = IRREGS_SYSCALL_GET_FOR(t);
#endif

 /* Copy the original, unmodified IRET tail. */
 if (++t->t_sigenter.se_count == 1) {
  memcpy(&t->t_sigenter.se_xip,&ss_descr->xip,
         sizeof(struct sigenter)-
         offsetof(struct sigenter,se_xip));
  /* Setup the register state to not return to user-space, but to `sigenter()' instead. */
#if 1
  syslog(LOG_DEBUG,"Override system call return " REGISTER_PREFIX "IP %p with sigenter %p\n",
         ss_descr->xip,&sigenter);
#endif
  ss_descr->xip    = (uintptr_t)&sigenter;
  ss_descr->cs     = __KERNEL_CS;
  ss_descr->xflags = EFLAGS_IF|EFLAGS_IOPL(3);
  /* TODO: Use sigaltstack() here, if it was ever set! */
  user_info = ((USER struct sigenter_info *)ss_descr->userxsp)-1;
#if USER_REDZONE_SIZE != 0
  /* Skip memory required for a `red' zone. */
  *(uintptr_t *)&user_info -= USER_REDZONE_SIZE;
#endif
 } else {
  /* Allocate an additional entry on the signal stack already in use. */
  assert(ss_descr->xip == (uintptr_t)&sigenter);
  user_info = ((USER struct sigenter_info *)t->t_sigenter.se_userxsp)-1;
 }


#if defined(__i386__) || defined(__x86_64__)
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
#ifdef __x86_64__
 memset(&info.ei_ctx.uc_mcontext.__reserved1,0,
        sizeof(info.ei_ctx.uc_mcontext.__reserved1));
#if 0
 info.ei_ctx.uc_mcontext.gregs[REG_R8]      = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_R9]      = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_R10]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_R11]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_R12]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_R13]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_R14]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_R15]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_RDI]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_RSI]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_RBP]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_RBX]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_RDX]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_RAX]     = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_RCX]     = /* Filled later... */;
#endif
 info.ei_ctx.uc_mcontext.gregs[REG_RSP]     = (greg_t)t->t_sigenter.se_userxsp;
 info.ei_ctx.uc_mcontext.gregs[REG_RIP]     = (greg_t)t->t_sigenter.se_xip;
 info.ei_ctx.uc_mcontext.gregs[REG_CSGSFS]  = (greg_t)t->t_sigenter.se_cs;
 info.ei_ctx.uc_mcontext.gregs[REG_OLDMASK] = 0; /* ??? */
 info.ei_ctx.uc_mcontext.gregs[REG_CR2]     = (unsigned long int)t->t_lastcr2;
#if 0
 info.ei_ctx.uc_mcontext.gregs[REG_EDI] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_ESI] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EBP] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EBX] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EDX] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_ECX] = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EAX] = /* Filled later... */;
#endif
#else
 info.ei_ctx.uc_mcontext.cr2             = (unsigned long int)t->t_lastcr2;
 info.ei_ctx.uc_mcontext.oldmask         = 0; /* ??? */
 info.ei_ctx.uc_mcontext.gregs[REG_GS]   = 0; /* Clear upper 16 bit. */
 info.ei_ctx.uc_mcontext.gregs[REG_FS]   = 0; /* *ditto* */
 info.ei_ctx.uc_mcontext.gregs[REG_ES]   = 0; /* *ditto* */
 info.ei_ctx.uc_mcontext.gregs[REG_DS]   = 0; /* *ditto* */
 info.ei_ctx.uc_mcontext.gregs[REG_ESP]  = (greg_t)t->t_sigenter.se_userxsp;
 info.ei_ctx.uc_mcontext.gregs[REG_EIP]  = (greg_t)t->t_sigenter.se_xip;
 info.ei_ctx.uc_mcontext.gregs[REG_CS]   = t->t_sigenter.se_cs;
 info.ei_ctx.uc_mcontext.gregs[REG_UESP] = (greg_t)t->t_sigenter.se_userxsp;
 info.ei_ctx.uc_mcontext.gregs[REG_SS]   = t->t_sigenter.se_ss;
#if 0
 info.ei_ctx.uc_mcontext.gregs[REG_EDI]  = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_ESI]  = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EBP]  = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EBX]  = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EDX]  = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_ECX]  = /* Filled later... */;
 info.ei_ctx.uc_mcontext.gregs[REG_EAX]  = /* Filled later... */;
#endif
#endif
 info.ei_ctx.uc_mcontext.gregs[REG_EFL]    = t->t_sigenter.se_xflags;
 info.ei_ctx.uc_mcontext.gregs[REG_TRAPNO] = reg_trapno;
 info.ei_ctx.uc_mcontext.gregs[REG_ERR]    = reg_err;
 info.ei_ctx.uc_mcontext.fpregs = &user_info->ei_ctx.__fpregs_mem;
 info.ei_old_xip = (USER void *)t->t_sigenter.se_xip;
 fpstate_from_task(&info.ei_ctx.__fpregs_mem,t);
 memcpy(&info.ei_ctx.uc_sigmask,&t->t_sigblock,sizeof(sigset_t));

 /* Fixup execution of the signal handler itself. */
 t->t_sigenter.se_userxsp = (void *)user_info;
 t->t_sigenter.se_xip     = (void *)action->sa_handler;

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
    syslog(LOG_WARN,
           "[SIG] Failed to deliver signal to process %d/%d: Target stack at %p is faulty\n",
          (int)t->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
          (int)t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,user_info);
    return -EFAULT;
   }
 }

 return -EOK;
}



/* Apart of the user-share segment:
 * >> This function is called to return from a signal handler.
 * AKA: This is our signal trampoline code. */
GLOBAL_ASM(
L(.section .text.user                                                            )
L(PRIVATE_ENTRY(signal_return)                                                   )
L(    movx $(__NR_sigreturn), %xax                                               )
#ifdef __x86_64__
L(    leaq (SIGENTER_INFO_OFFSETOF_CTX - \
            SIGENTER_INFO_OFFSETOF_OLD_XBP)(%rbp), %rdi                          )
#else
L(    leal (SIGENTER_INFO_OFFSETOF_CTX - \
            SIGENTER_INFO_OFFSETOF_OLD_XBP)(%ebp), %ebx                          )
#endif
L(    int  $0x80                                                                 )
L(SYM_END(signal_return)                                                         )
L(.previous                                                                      )
);

#ifdef __x86_64__
#define IGNORE_SEGMENT_REGISTERS 1
#else
#define IGNORE_SEGMENT_REGISTERS 0
#endif

SYSCALL_SDEFINE(sigreturn,cs) {
 ucontext_t ctx; size_t context_indirection;
 USER struct sigenter_info *last_context;
 if (copy_from_user(&ctx,(ucontext_t *)GPREGS_SYSCALL_ARG1(cs->host.gp),sizeof(ucontext_t)))
     sigfault();
#if !IGNORE_SEGMENT_REGISTERS
#if 1
#define CHECK_SEGMENT(id,name,value) \
 if ((ctx.uc_mcontext.gregs[id]&3) != 3) \
     sigill("%%%s (Illegal RPL privilege level)", \
            name,ctx.uc_mcontext.gregs[id])
#else
#define CHECK_SEGMENT(id,name,value) \
 if (ctx.uc_mcontext.gregs[id] != (value)) \
     sigill("%%" name " (%.4I16x != %.4I16x)", \
            ctx.uc_mcontext.gregs[id],value)
#endif
 CHECK_SEGMENT(REG_GS,"gs",__USER_GS);
 CHECK_SEGMENT(REG_FS,"fs",__USER_FS);
 CHECK_SEGMENT(REG_ES,"es",__USER_DS);
 CHECK_SEGMENT(REG_DS,"ds",__USER_DS);
 CHECK_SEGMENT(REG_CS,"cs",__USER_CS);
 CHECK_SEGMENT(REG_SS,"ss",__USER_DS);
#undef CHECK_SEGMENT
#endif /* !IGNORE_SEGMENT_REGISTERS */
 if (!(ctx.uc_mcontext.gregs[REG_EFL]&EFLAGS_IF))
       sigill("EFLAGS (%.8I32x misses EFLAGS_IF:%.8I32x)",
               ctx.uc_mcontext.gregs[REG_EFL],EFLAGS_IF);
#ifndef CONFIG_ALLOW_USER_IO
 if (ctx.uc_mcontext.gregs[REG_EFL]&EFLAGS_IOPL(3))
     sigill("EFLAGS (%.8I32x contains EFLAGS_IOPL:%.8I32x)",
             ctx.uc_mcontext.gregs[REG_EFL],EFLAGS_IOPL(3));
#endif

 /* Load the new machine context as register state upon return to user-space.
  * NOTE: We don't jump directly, because that would break execution of
  *       additional signals that may have been unblocked by `task_set_sigblock()'. */
#ifdef __x86_64__
 /* XXX: Context-based? */
 cs->host.sg.fs_base = TASK_DEFAULT_FS_BASE(THIS_TASK);
 cs->host.sg.gs_base = TASK_DEFAULT_GS_BASE(THIS_TASK);
#else /* __x86_64__ */
#if IGNORE_SEGMENT_REGISTERS
 cs->host.sg.gs     = __USER_GS;
 cs->host.sg.fs     = __USER_FS;
 cs->host.sg.es     = __USER_DS;
 cs->host.sg.ds     = __USER_DS;
#else /* IGNORE_SEGMENT_REGISTERS */
 cs->host.sg.gs     = ctx.uc_mcontext.gregs[REG_GS];
 cs->host.sg.fs     = ctx.uc_mcontext.gregs[REG_FS];
 cs->host.sg.es     = ctx.uc_mcontext.gregs[REG_ES];
 cs->host.sg.ds     = ctx.uc_mcontext.gregs[REG_DS];
#endif /* !IGNORE_SEGMENT_REGISTERS */
#endif /* !__x86_64__ */

#ifdef __x86_64__
 cs->host.gp.r8     = ctx.uc_mcontext.gregs[REG_R8];
 cs->host.gp.r9     = ctx.uc_mcontext.gregs[REG_R9];
 cs->host.gp.r10    = ctx.uc_mcontext.gregs[REG_R10];
 cs->host.gp.r11    = ctx.uc_mcontext.gregs[REG_R11];
 cs->host.gp.r12    = ctx.uc_mcontext.gregs[REG_R12];
 cs->host.gp.r13    = ctx.uc_mcontext.gregs[REG_R13];
 cs->host.gp.r14    = ctx.uc_mcontext.gregs[REG_R14];
 cs->host.gp.r15    = ctx.uc_mcontext.gregs[REG_R15];
 cs->host.gp.rdi    = ctx.uc_mcontext.gregs[REG_RDI];
 cs->host.gp.rsi    = ctx.uc_mcontext.gregs[REG_RSI];
 cs->host.gp.rbp    = ctx.uc_mcontext.gregs[REG_RBP];
 cs->host.gp.rbx    = ctx.uc_mcontext.gregs[REG_RBX];
 cs->host.gp.rdx    = ctx.uc_mcontext.gregs[REG_RDX];
 cs->host.gp.rcx    = ctx.uc_mcontext.gregs[REG_RCX];
 cs->host.gp.rax    = ctx.uc_mcontext.gregs[REG_RAX];
#else
 cs->host.gp.edi    = ctx.uc_mcontext.gregs[REG_EDI];
 cs->host.gp.esi    = ctx.uc_mcontext.gregs[REG_ESI];
 cs->host.gp.ebp    = ctx.uc_mcontext.gregs[REG_EBP];
 cs->host.gp.esp    = ctx.uc_mcontext.gregs[REG_ESP];
 cs->host.gp.ebx    = ctx.uc_mcontext.gregs[REG_EBX];
 cs->host.gp.edx    = ctx.uc_mcontext.gregs[REG_EDX];
 cs->host.gp.ecx    = ctx.uc_mcontext.gregs[REG_ECX];
 cs->host.gp.eax    = ctx.uc_mcontext.gregs[REG_EAX];
#endif
 
 /* NOTE: Disable preemption to prevent more signals from being raised,
  *       now that we'll be attempting to update the bottom-most entry
  *       of the signal-handler-execution chain.
  *    >> Having had interrupts enabled until now, more signals may
  *       have been raised, and even more though: Restoring the old
  *       signal mask may (quite likely) re-raised more signals.
  *       But sadly, the bottom-most signal context is using the address
  *       of this where `sys_sigreturn()' was called from, which is quite
  *       incorrect as this function isn't supposed to return to the caller,
  *       but instead return to where the first interrupt was called from.
  *    >> Therefor we must find that bottom-most entry and update it
  *       to mirror the register data we've been passed through the
  *       given user-space CPU context.
  * HINT: We know the exact amount of indirections required, as this
  *       value is safely stored within `THIS_TASK->t_sigenter.se_count'.
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
#ifdef __x86_64__
  if (copy_to_user(&last_context->ei_ctx.uc_mcontext.gregs[REG_RIP],
                   &ctx.uc_mcontext.gregs[REG_RIP],
                 ((REG_CSGSFS-REG_RIP)+1)*sizeof(greg_t)))
      sigfault();
#else
  if (copy_to_user(&last_context->ei_ctx.uc_mcontext.gregs[REG_EIP],
                   &ctx.uc_mcontext.gregs[REG_EIP],
                 ((REG_SS-REG_EIP)+1)*sizeof(greg_t)))
      sigfault();
#endif
 } else {
  /* Simple case: no indirection. - This signal will
   * switch back to regular user-space code flow. */
#ifdef __x86_64__
  cs->iret.rip     = ctx.uc_mcontext.gregs[REG_RIP];
#else
  cs->iret.eip     = ctx.uc_mcontext.gregs[REG_EIP];
#endif
#if IGNORE_SEGMENT_REGISTERS
  cs->iret.cs      = __USER_CS;
#else
  cs->iret.cs      = ctx.uc_mcontext.gregs[REG_CS];
#endif
  cs->iret.xflags  = ctx.uc_mcontext.gregs[REG_EFL];
#ifdef __x86_64__
  cs->iret.userrsp = ctx.uc_mcontext.gregs[REG_RSP];
#else
  cs->iret.useresp = ctx.uc_mcontext.gregs[REG_UESP];
#endif
#if IGNORE_SEGMENT_REGISTERS
  cs->iret.ss      = __USER_DS;
#else
  cs->iret.ss      = ctx.uc_mcontext.gregs[REG_SS];
#endif
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

#endif /* CONFIG_USE_OLD_SIGNALS */




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

#ifdef CONFIG_SMP
 if (c != THIS_CPU) {
  /* TODO: Send IPC communication. */
 } else
#endif
 {
  struct sigaction *action;
  struct sighand *hand;
  /* Check if the signal is currently being blocked. */
  if (TASK_ISBLOCKING(t,signo+1)) {
enqueue_later:
   /* Enqueue the signal to be send at a later time. */
   cpu_endwrite(c);
   PREEMPTION_POP(was);
   /* Check if we're actually allowed to send signals to this task. */
   if (t->t_flags&TASKFLAG_NOSIGNALS)
       return -EPERM;
   return sigpending_enqueue(&t->t_sigpend,signal_info);
  }
  syslog(LOG_DEBUG,"[SIG] kill(%d,%d) %p -> %p\n",
        (int)t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,signo,
         THIS_TASK,t);
#if 0
  debug_tbprint(0);
#endif

  /* NOTE: We know the task is running on our CPU, and with interrupts disabled
   *       we are either that task, or know that it won't be running until we've
   *       re-enabled interrupt, meaning we can full access to `t_sighand', knowing
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
do_stop:
    return task_suspend_cpu_endwrite(t,TASK_SUSP_USER|TASK_SUSP_NOW,was);

   case DA_CONT:
do_cont:
    return task_resume_cpu_endwrite(t,TASK_SUSP_USER|TASK_SUSP_NOW,was);

   case DA_CORE:
do_core:
    error = task_terminate_cpu_endwrite(c,t,(void *)(uintptr_t)
                                       (__WCOREFLAG|__W_EXITCODE(0,signal_info->si_signo)));
    if (t != THIS_TASK && (t->t_cstate->iret.cs&3) == 3)
         coredump_user_task(t,signal_info,reg_trapno,reg_err);
    else coredump_host_task(t,signal_info,reg_trapno,reg_err);
    goto ppop_end;

   default:
    error = task_terminate_cpu_endwrite(c,t,(void *)(uintptr_t)__W_EXITCODE(0,signal_info->si_signo));
    goto ppop_end;

   }
  }
  /* KOS extension builtin signal handlers. */
  if (action->sa_handler == SIG_CONT) goto do_cont;
  if (action->sa_handler == SIG_STOP) goto do_stop;
  if (action->sa_handler == SIG_CORE) goto do_core;
  if (action->sa_handler == SIG_HOLD) {
   /* Add the signal to the hold mask (blocking). */
   unsigned long int mask = __sigmask(signo+1);
   unsigned long int word = __sigword(signo+1);
   if (ATOMIC_FETCHOR(t->t_sigblock.__val[word],mask)&mask)
       goto enqueue_later;
   goto done;
  }

  if (action->sa_handler == SIG_IGN) {
   /* Nothing to do here... */
   /* XXX: What about exception signals?
    *      We should probably still do something about those... */
ignore_signal:;
  } else {
   sigword_t *dst,*end,*src;

   if (t != THIS_TASK &&
      !t->t_sigenter.se_count &&
      (t->t_cstate->iret.cs&3)) {
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

PUBLIC SAFE errno_t KCALL
task_check_signals(sigset_t *__restrict changes) {
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
   return_info.si_signo = SIGPENDING_GT_UNDEFINED((uintptr_t)return_signal);
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
   /* NOTE: Store a mask of all changes in `src' */
   for (; dst != end; ++dst,++src) {
    byte_t changed = *dst & *src;
    *dst &= ~*src;
    *src = changed;
   }
deque_changes:
   error = task_check_signals(&new_set);
   break;

   /* Difficult: Must return to the first signal that gets unblocked. */
  case SIG_SETMASK:
   /* NOTE: Store a mask of all changes in `src' */
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
 error = task_check_signals(newset);
 task_endcrit();
 return error;
}



SYSCALL_DEFINE4(sigtimedwait,USER sigset_t const *,uthese,USER siginfo_t *,uinfo,
                USER struct timespec const *,uts,size_t,sigsetsize) {
 sigset_t wait_mask,old_block; jtime_t timeout = JTIME_INFINITE;
 struct task *t = THIS_TASK; errno_t error; struct sigqueue *signal;
 bool local_write_lock,share_write_lock,is_blocking = false;
 if (sigsetsize > sizeof(sigset_t)) return -EINVAL;
 memset((u8 *)&wait_mask+sigsetsize,0,sizeof(sigset_t)-sigsetsize);
 if (copy_from_user(&wait_mask,uthese,sigsetsize))
     return -EFAULT;
 if (uts) {
  struct timespec tmo;
  if (copy_from_user(&tmo,uts,sizeof(struct timespec)))
      return -EFAULT;
  timeout = jiffies+TIMESPEC_OFF_TO_JIFFIES(tmo);
 }
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
    *       been implemented, we can simply lock + `task_addwait()' + unlock
    *       any number of signals consecutively and have `task_waitfor()'
    *       return a pointer to the pending set to which a new signal was
    *       added.
    * >> Thus what we're currently doing by locking both sets
    *    at the same time is both wasteful and unnecessary! */
   sig_endwrite(&t->t_sigpend.sp_newsig);
   sig_endwrite(&t->t_sigshare->ss_pending.sp_newsig);

   wait_error = task_waitfor(timeout);
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
   return_info.si_signo = SIGPENDING_GT_UNDEFINED((uintptr_t)signal);
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
  /* Kill The process matching `pid'. */
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
   /* NOTE: Don't deliver signal `0'. - It's used to test access. */
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

SYSCALL_DEFINE2(sigsuspend,USER sigset_t const *,unewset,size_t,sigsetsize) {
 sigset_t newset,oldset; errno_t result,temp;
 struct task *t = THIS_TASK;
 if (sigsetsize > sizeof(sigset_t))
     return -EINVAL;
 if (copy_from_user(&newset,unewset,sigsetsize))
     return -EFAULT;
 memset((byte_t *)&newset+sigsetsize,0,
         sizeof(sigset_t)-sigsetsize);
 memcpy(&oldset,&t->t_sigblock,sizeof(sigset_t));
 /* Make sure never to block these two... */
 sigdelset(&newset,SIGKILL);
 sigdelset(&newset,SIGSTOP);

 /* Time to do this! */
 task_crit();

 /* Set the new signal mask. */
 result = task_set_sigblock(&newset);
 if (E_ISERR(result)) goto end;

 /* Wait until we're sent a signal we can handle. */
 { struct sig *s = task_waitfor(JTIME_INFINITE); /* XXX: Timeout? */
   assert(E_ISERR(s));
   result = E_GTERR(s);
 }

 /* Restore the old signal mask. */
 temp = task_set_sigblock(&oldset);
 if (E_ISOK(result)) result = temp;
end:
 task_endcrit();
 return result;
}

/*
#define __NR_sigaltstack  132
__SYSCALL(__NR_sigaltstack,sys_sigaltstack)
#define __NR_sigqueueinfo 138
__SYSCALL(__NR_sigqueueinfo,sys_sigqueueinfo)
*/

DECL_END

#ifndef __INTELLISENSE__
#include "sighand.c.inl"
#endif
#endif /* !CONFIG_NO_SIGNALS */


#endif /* !GUARD_KERNEL_SCHED_SIGNAL_C */
