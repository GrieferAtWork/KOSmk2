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
#ifndef GUARD_KERNEL_ARCH_SIGNAL_C
#define GUARD_KERNEL_ARCH_SIGNAL_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#ifndef CONFIG_NO_SIGNALS
#include <arch/cpustate.h>
#include <arch/fpu.h>
#include <arch/preemption.h>
#include <arch/preemption.h>
#include <arch/signal.h>
#include <arch/syscall.h>
#include <asm/instx.h>
#include <asm/unistd.h>
#include <assert.h>
#include <bits/sigaction.h>
#include <bits/siginfo.h>
#include <bits/signum.h>
#include <bits/sigset.h>
#include <bits/sigstack.h>
#include <bits/waitstatus.h>
#include <errno.h>
#include <hybrid/asm.h>
#include <hybrid/panic.h>
#include <hybrid/section.h>
#include <hybrid/typecore.h>
#include <hybrid/types.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <linker/coredump.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/percpu.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <sched/types.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <sys/ucontext.h>
#include <syslog.h>

DECL_BEGIN

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

STATIC_ASSERT(offsetof(struct sigenter,se_count) == SIGENTER_OFFSETOF_COUNT);
STATIC_ASSERT(offsetof(struct sigenter,se_next) == SIGENTER_OFFSETOF_NEXT);
STATIC_ASSERT(offsetof(struct sigenter,se_xip) == SIGENTER_OFFSETOF_XIP);
STATIC_ASSERT(offsetof(struct sigenter,se_xax) == SIGENTER_OFFSETOF_XAX);
STATIC_ASSERT(offsetof(struct sigenter,se_xflags) == SIGENTER_OFFSETOF_XFLAGS);
STATIC_ASSERT(offsetof(struct sigenter,se_xsp) == SIGENTER_OFFSETOF_XSP);
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
STATIC_ASSERT(offsetof(struct sigenter,se_xdx) == SIGENTER_OFFSETOF_XDX);
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */
STATIC_ASSERT(sizeof(struct sigenter) == SIGENTER_SIZE);


PRIVATE void KCALL
coredump_user_task(struct task *__restrict t,
                   siginfo_t const *__restrict reason,
                   greg_t reg_trapno, greg_t reg_err) {
 assert(!PREEMPTION_ENABLED());
 assert(t != THIS_TASK);
 assert(TASK_CPU(t) == THIS_CPU);
 assertf(t->t_flags&TASKFLAG_WILLTERM,
         "The task must be scheduled for termination");
 if (INTNO_HAS_EXC_CODE(reg_trapno)) {
  struct cpustate_ie state;
  CPUSTATE_TO_CPUSTATE_IE(*t->t_cstate,state,IRREGS_ENCODE_INTNO(reg_trapno),reg_err);
  core_dodump(t->t_real_mman,t,&state,reason,COREDUMP_FLAG_NORMAL);
 } else {
  struct cpustate_i state;
  CPUSTATE_TO_CPUSTATE_I(*t->t_cstate,state,IRREGS_ENCODE_INTNO(reg_trapno));
  core_dodump(t->t_real_mman,t,
             (struct cpustate_ie *)&state,
              reason,COREDUMP_FLAG_NORMAL);
 }
}


PRIVATE SAFE bool KCALL
task_is_running_syscall(struct mman *__restrict mm, uintptr_t xip) {
 byte_t code[2]; struct mman *omm; size_t copy_error;
 TASK_PDIR_BEGIN(omm,mm);
 copy_error = copy_from_user(code,(void *)(xip-2),sizeof(code));
 TASK_PDIR_END(omm,mm);
 if (copy_error) return false;
 if (code[0] == 0xcd && code[1] == INTNO_SYSCALL /* int $0x80 */)
     return true;
 /* XXX: Must also detect other ways of invoking system calls here! */
 return false;
}


#define CD_STACKTOP_INFO_OFFSETOF_REASON      0
#define CD_STACKTOP_INFO_OFFSETOF_REG_TRAPNO  __SIGINFO_SIZE
#define CD_STACKTOP_INFO_OFFSETOF_REG_ERR    (__SIGINFO_SIZE+__SIZEOF_GREG_T__)
#define CD_STACKTOP_INFO_SIZE                (__SIGINFO_SIZE+2*__SIZEOF_GREG_T__)
struct cd_stacktop_info {
 siginfo_t reason;
 greg_t    reg_trapno;
 greg_t    reg_err;
};

STATIC_ASSERT(offsetof(struct cd_stacktop_info,reason) == CD_STACKTOP_INFO_OFFSETOF_REASON);
STATIC_ASSERT(offsetof(struct cd_stacktop_info,reg_trapno) == CD_STACKTOP_INFO_OFFSETOF_REG_TRAPNO);
STATIC_ASSERT(offsetof(struct cd_stacktop_info,reg_err) == CD_STACKTOP_INFO_OFFSETOF_REG_ERR);
STATIC_ASSERT(sizeof(struct cd_stacktop_info) == CD_STACKTOP_INFO_SIZE);



INTDEF ATTR_NORETURN void ASMCALL coredump_enter(void);
GLOBAL_ASM(
L(.section .text.cold                                                            )
L(PRIVATE_ENTRY(coredump_enter)                                                  )
L(    /* This is where we transition after a host-level task was configured for a coredump. */)
L(    /* Reserve space for 2 IRET tails. */                                      )
L(    /* NOTE: The first tail is required to fake a working `IRREGS_SYSCALL_GET_FOR()' */)
L(    subx $(IRREGS_SYSCALL_SIZE+IRREGS_IE_SIZE), %xsp                           )
L(    __ASM_PUSH_COMREGS /* Save all user-space registers. */                    )
#ifdef __x86_64__
L(    swapgs                                                                     )
#else /* __x86_64__ */
L(    __ASM_LOAD_SEGMENTS(%dx)                                                   )
#endif /* !__x86_64__ */
#define IRREGS_SYSCALL(offset) (offset+CPUSTATE_IE_SIZE)(%xsp)
#define CPUSTATE(offset)       (offset)(%xsp)
L(                                                                               )
L(    movx   %xax, IRREGS_SYSCALL(IRREGS_SYSCALL_OFFSETOF_ORIG_XAX)              )
L(    movx   ASM_CPU(CPU_OFFSETOF_RUNNING), %xsi                                 )
L(    /* Check if we've managed to store the original XAX before. */             )
L(    testb  $1, TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_NEXT(%xsi)             )
L(    jz     1f                                                                  )
L(    /* Yes, we did. - Overwrite the orig_xax field in our IRET tail. */        )
L(    movx   TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XAX(%xsi), %xax            )
L(    movx   %xax, IRREGS_SYSCALL(IRREGS_SYSCALL_OFFSETOF_ORIG_XAX)              )
L(1:  /* Now fill in the remainder of registers in both tails, using sigenter data. */)
L(    movx   TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XIP(%xsi), %xax            )
L(    movx   %xax, IRREGS_SYSCALL(IRREGS_SYSCALL_OFFSETOF_IP)                    )
L(    movx   %xax, CPUSTATE(CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_IP)     )
L(    movx   TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XFLAGS(%xsi), %xax         )
L(    movx   %xax, IRREGS_SYSCALL(IRREGS_SYSCALL_OFFSETOF_FLAGS)                 )
L(    movx   %xax, CPUSTATE(CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_FLAGS)  )
L(    movx   TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XSP(%xsi), %xax            )
L(    movx   %xax, IRREGS_SYSCALL(IRREGS_SYSCALL_OFFSETOF_USERSP)                )
L(    movx   %xax, CPUSTATE(CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_USERSP) )
L(    movx   $(__USER_CS), %xax                                                  )
L(    movx   %xax, IRREGS_SYSCALL(IRREGS_SYSCALL_OFFSETOF_CS)                    )
L(    movx   %xax, CPUSTATE(CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_CS)     )
L(    movx   $(__USER_DS), %xax                                                  )
L(    movx   %xax, IRREGS_SYSCALL(IRREGS_SYSCALL_OFFSETOF_SS)                    )
L(    movx   %xax, CPUSTATE(CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_SS)     )
L(    sti    /* With a valid IRET tail at the base of our stack, we can re-enable interrupts. */)
L(                                                                               )
L(    /* Load the signal information from this task's stack-stop. */             )
L(    movx   TASK_OFFSETOF_HSTACK+HSTACK_OFFSETOF_BEGIN(%xsi), %FASTCALL_REG2    )
L(    /* Save the trap_no/error registers in our new cpustate. */                )
L(    movx   CD_STACKTOP_INFO_OFFSETOF_REG_TRAPNO(%FASTCALL_REG2), %xax          )
L(    movx   %xax, CPUSTATE(CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_INTNO)  )
L(    movx   CD_STACKTOP_INFO_OFFSETOF_REG_ERR(%FASTCALL_REG2), %xax             )
L(    movx   %xax, CPUSTATE(CPUSTATE_IE_OFFSETOF_IRET+IRREGS_IE_OFFSETOF_EXC_CODE))
L(                                                                               )
L(    leax   CPUSTATE(0),                         %FASTCALL_REG1 /* state */     )
#if CD_STACKTOP_INFO_OFFSETOF_REASON != 0
L(    addx   $(CD_STACKTOP_INFO_OFFSETOF_REASON), %FASTCALL_REG2 /* reason */    )
#endif
L(    /* We can use `jmp' because all arguments are passed through registers. */ )
L(    jmp    coredump_after_iret                                                 )
L(SYM_END(coredump_enter)                                                        )
L(.previous                                                                      )
);

INTERN ATTR_NORETURN void FCALL
coredump_after_iret(struct cpustate_ie *__restrict state,
                    siginfo_t *__restrict reason) {
 /* Since the assembly above always generates a cpustate_ie,
  * even when the associated interrupt number doesn't correspond
  * to an interrupt that includes an error code, we must adjust
  * the given state to delete the exc_code field if it shouldn't be present. */
 if (!INTNO_HAS_EXC_CODE(state->iret.intno)) {
  /* Shift data in the given state to delete the exception code. */
  memmove((byte_t *)state+sizeof(register_t),
          (byte_t *)state,
          offsetof(struct cpustate_ie,iret.exc_code));
  /* Adjust to point to the new state base address. */
  *(byte_t **)&state += sizeof(register_t);
 }
 /* Encode the interrupt number in the correct format. */
 state->iret.intno = IRREGS_ENCODE_INTNO(state->iret.intno);

 /* Actually invoke the coredump. */
 core_dodump(THIS_TASK->t_real_mman,THIS_TASK,
             state,reason,COREDUMP_FLAG_NORMAL);

 task_endcrit(); /* Close the critical block that kept the task alive. */
 PANIC("Task was not terminated after critical block was "
       "closed in `coredump_after_iret()' (critical = %I32u)",
       THIS_TASK->t_critical);
}


PRIVATE void KCALL
coredump_host_task(struct task *__restrict t,
                   siginfo_t const *__restrict reason,
                   greg_t reg_trapno, greg_t reg_err) {
 struct irregs_syscall *return_registers;
 assert(!PREEMPTION_ENABLED());
 assert(TASK_CPU(t) == THIS_CPU);
 assertf(t->t_flags&TASKFLAG_WILLTERM,
         "The task must be scheduled for termination");
 return_registers = IRREGS_SYSCALL_GET_FOR(t);
 /* Delete the task's SIGENTER chain. */
 if (t->t_sigenter.se_count) {
  return_registers->xip     = t->t_sigenter.se_xip;
  return_registers->cs      = __USER_CS;
  return_registers->xflags  = t->t_sigenter.se_xflags;
  return_registers->userxsp = t->t_sigenter.se_xsp;
  return_registers->ss      = __USER_DS;
  t->t_sigenter.se_count    = 0;
 }

 t->t_sigenter.se_next = NULL;
 if (task_is_running_syscall(t->t_real_mman,return_registers->xip)) {
  /* Save the original XAX (system call number) for the coredump. */
  t->t_sigenter.se_xax  = return_registers->orig_xax;
  /* Indicator that orig_xax is valid. (Checked in `coredump_enter') */
  t->t_sigenter.se_next = (USER struct sigenter_tail *)-1;
 }

 /* Dirty hack -- Don't look. */
 /* Store the trap number, error register and reason at the end of the task's kernel stack.
  * While that stack is still in use, we can pretty much assume that if it ever
  * came to the task using that portion of its data, something already went wrong. */
 { struct cd_stacktop_info *save;
   /* Stacks grow down on x86_64, so use `hs_begin'. */
   save = (struct cd_stacktop_info *)t->t_hstack.hs_begin;
   memcpy(&save->reason,reason,sizeof(siginfo_t));
   save->reg_trapno = reg_trapno;
   save->reg_err    = reg_err;
 }

 return_registers->xflags &= ~EFLAGS_IF;
 return_registers->xip     = (uintptr_t)&coredump_enter;
 return_registers->cs      = __KERNEL_CS;
#ifdef __x86_64__
 /* x86_64 always requires the full IRET tail,
  * meaning we must update these members, too. */
 return_registers->userrsp = (u64)t->t_hstack.hs_end;
 return_registers->ss      = __KERNEL_DS;
#endif

 /* Make to task critical to prolong its termination until after
  * its fake transition to user-space and the following coredump.
  * NOTE: This critical block is closed in `coredump_after_iret()' */
 ATOMIC_FETCHINC(t->t_critical);
}



/* Apart of the user-share segment:
 * >> This function is called to return from a signal handler.
 * AKA: This is our signal trampoline code. */
INTDEF ATTR_NORETURN void ASMCALL signal_return(void);
GLOBAL_ASM(
L(.section .text.user                                                            )
L(PRIVATE_ENTRY(signal_return)                                                   )
#ifdef __x86_64__ /* Load `struct sigenter_tail' into the first syscal register. */
L(    leaq  -SIGENTER_TAIL_OFFSETOF_OLDBP(%rbp), %rdi                            )
#else
L(    leal  -SIGENTER_TAIL_OFFSETOF_OLDBP(%ebp), %ebx                            )
#endif
L(1:  movx   $(__NR_sigreturn),                  %xax                            )
L(    int    $0x80                                                               )
L(    jmp    1b /* Keep repeating in case of recursion. */                       )
L(SYM_END(signal_return)                                                         )
L(.previous                                                                      )
);

PRIVATE errno_t KCALL raise_segfault(void *fault_addr) {
 siginfo_t info;
 THIS_TASK->t_lastcr2 = fault_addr;
 memset(&info,0,sizeof(siginfo_t));
 info.si_signo = SIGSEGV;
 info.si_code  = SI_KERNEL;
 PREEMPTION_DISABLE();
 info.si_addr  = (void *)(THIS_TASK->t_sigenter.se_count
                        ? THIS_TASK->t_sigenter.se_xip
                        : IRREGS_SYSCALL_GET()->xip);
 PREEMPTION_ENABLE();
 info.si_lower = fault_addr;
 info.si_upper = fault_addr;
 /* XXX: exc_code is hard-coded as `0' - That shouldn't be. */
 return task_kill2(THIS_TASK,&info,EXC_PAGE_FAULT,0);
}


#ifndef CONFIG_NO_FPU
PRIVATE void KCALL
load_fpu(struct _libc_fpstate const *__restrict state,
         struct fpustate *fpu) {
#ifdef __x86_64__
 STATIC_ASSERT(sizeof(struct fpustate) == sizeof(struct _libc_fpstate));
 memcpy(fpu,state,sizeof(struct fpustate));
#else
 struct fpu_reg *dst,*end; struct _libc_fpreg const *src;
 STATIC_ASSERT(sizeof(struct _libc_fpreg) <= sizeof(struct fpu_reg));
 fpu->fp_fcw   = state->cw;
 fpu->fp_fsw   = state->sw;
 fpu->fp_ftw   = state->tag;
 fpu->fp_fpuip = state->ipoff;
 fpu->fp_fpucs = state->cssel;
 fpu->fp_fpudp = state->dataoff;
 fpu->fp_fpuds = state->datasel;
 fpu->fp_mxcsr = state->status; /* ??? Is this correct? */
 src = state->_st;
 end = (dst = fpu->fp_regs)+COMPILER_LENOF(fpu->fp_regs);
 for (; dst != end; ++dst,++src)
        memcpy(dst,src,sizeof(struct _libc_fpreg));
#endif
}
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
#define load_fpu(state,fpu) (void)0
#define safe_fpu(state,fpu)  memset((state),0,sizeof(struct _libc_fpstate));
#endif


#define XFLAGS_USER_MASK  (EFLAGS_CF|EFLAGS_PF|EFLAGS_AF|EFLAGS_ZF|EFLAGS_SF)

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

 state->iret.xflags &= ~(XFLAGS_USER_MASK);
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
#ifndef CONFIG_NO_FPU
 /* Restore FPU state. */
 if (caller->t_arch.at_fpu != FPUSTATE_NULL) {
  load_fpu(&ctx.__fpregs_mem,caller->t_arch.at_fpu);

  /* Invalidate the FPU context if it was associated with the
   * caller (Force a register reload from modified information). */
  if (CPU(fpu_current) == caller) {
   CPU(fpu_current) = NULL;
   FPUSTATE_DISABLE();
  }
 }
#endif

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

 /* Use sigaltstack() if requested to.
  * NOTE: If the calling stack pointer is already apart of the
  *       sigaltstack, don't actually switch, so-as to prevent
  *       overwriting already running signal handlers. */
 if (action->sa_flags&SA_ONSTACK &&
     KERNEL_SIGSTACK_ISVALID(t->t_sigstack) &&
    !KERNEL_SIGSTACK_CONTAINS(t->t_sigstack,state->iret.userxsp)) {
  state->iret.userxsp = ((uintptr_t)t->t_sigstack.ss_base+
                                    t->t_sigstack.ss_size);
 } else {
#if USER_REDZONE_SIZE != 0
  /* Skip memory required for a `red' zone. */
  *(uintptr_t *)&state->iret.userxsp -= USER_REDZONE_SIZE;
#endif
 }

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
        COLDSTR("[SIG] Failed to deliver signal to process %d/%d: Target stack at %p is faulty\n"),
       (int)t->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
       (int)t->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,dst);
 return -EFAULT;
}

#define SIGENTER_MODE_NORMAL       0 /* Don't modify any registers. */
#define SIGENTER_MODE_RESTORE      1 /* Fill `XAX' with `struct sigenter::se_xax' */
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
#define SIGENTER_MODE_RESTORE_LONG 3 /* Fill `XAX' with `struct sigenter::se_xax' and 
                                      * fill `XDX' with `struct sigenter::se_xdx' */
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */

STATIC_ASSERT(offsetof(struct sigenter_tail,t_ctx) == SIGENTER_TAIL_OFFSETOF_CTX);
STATIC_ASSERT(offsetof(struct sigenter_tail,t_oldbp) == SIGENTER_TAIL_OFFSETOF_OLDBP);
STATIC_ASSERT(offsetof(struct sigenter_tail,t_oldip) == SIGENTER_TAIL_OFFSETOF_OLDIP);
STATIC_ASSERT(sizeof(struct sigenter_tail) == SIGENTER_TAIL_SIZE);


#undef SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR
/* XXX: I don't actually think that the kernel
 *      is ever supposed to return -ERESTART, right?
 *      I mean, the return value shouldn't change ~just~
 *      because a user-space signal handler got invoked. */
//#define SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR 1

/* Enter a signal handler without register modifications.
 * NOTE: This is the only sigenter function that must be executed with interrupts enabled. */
INTDEF void ASMCALL sigenter(void);
INTDEF void ASMCALL sigenter_restore(void);
#ifdef SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR
INTDEF void ASMCALL sigenter_restart(void); /* Return -ERESTART instead of -EINTR */
#endif /* SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR */
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
/* Also restore XDX with `struct sigenter::se_xdx' */
INTDEF void ASMCALL sigenter_restore_long(void);
#ifdef SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR
INTDEF void ASMCALL sigenter_restart_long(void); /* Return -ERESTART instead of -EINTR */
#endif /* SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR */
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */


GLOBAL_ASM(
L(.section .text                                                                 )
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
#ifdef SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR
L(PRIVATE_ENTRY(sigenter_restart_long)                                           )
L(    cmpx   $(-EINTR),    %xax                                                  )
L(    jne    sigenter_restore_long                                               )
L(    cmpx   $(-1),        %xdx                                                  )
L(    jne    sigenter_restore_long                                               )
L(    movx   $(-ERESTART), %xax                                                  )
L(    movx   $(-1),        %xdx                                                  )
#endif /* SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR */
L(PRIVATE_ENTRY(sigenter_restore_long)                                           )
L(    pushx  $(SIGENTER_MODE_RESTORE_LONG)                                       )
L(    jmp    1f                                                                  )
L(SYM_END(sigenter_restore_long)                                                 )
#ifdef SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR
L(SYM_END(sigenter_restart_long)                                                 )
#endif /* SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR */
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */
L(PRIVATE_ENTRY(sigenter_restore)                                                )
L(    pushx  $(SIGENTER_MODE_RESTORE)                                            )
L(    jmp    1f                                                                  )
#ifdef SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR
L(PRIVATE_ENTRY(sigenter_restart)                                                )
L(    cmpx   $(-EINTR),    %xax                                                  )
L(    jne    sigenter                                                            )
L(    movx   $(-ERESTART), %xax                                                  )
#endif /* SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR */
L(PRIVATE_ENTRY(sigenter)                                                        )
L(    pushx  $(SIGENTER_MODE_NORMAL)                                             )
L(1:                                                                             )
L(    pushx  %xax                                                                )
L(    pushx  %xcx                                                                )
L(    pushx  %xdi                                                                )
L(    pushx  %xsi                                                                )
#ifdef __x86_64__
L(    pushx  %r10                                                                )
L(    pushx  %r11                                                                )
L(    ASM_RDGSBASE(r10) /* XXX: ASM_RDGSBASE(r11)? */                            )
L(    movq   %r10, %r11                                                          )
L(    ASM_RDFSBASE(r10)                                                          )
/* TODO: For some reason, GS_BASE is correct at this point
 *      (containing the kernel's GS_BASE rather than the user's)
 *       The only reason this isn't a critical bug is because nothing's
 *       really using the GS_BASE register in userspace, yet. But this
 *       should really be invenstigated and fixed ASAP! */
#define FS_BASE  %r10
#define GS_BASE  %r11
L(    swapgs                                                                     )
#else
L(    __ASM_PUSH_SGREGS                                                          )
L(    __ASM_LOAD_SEGMENTS(%ax)                                                   )
#endif
//L(  sti    /* TODO: Cannot be enabled unless we place a valid IRET tail at the stack base... */)
L(           /*      (In case further signals are raised while we're entering the current) */)
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
L(    testb  $(SIGENTER_MODE_RESTORE), L_OFFSETOF_MODE(%xsp)                     )
L(    jz     1f                                                                  )
L(    movx   TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XAX(CALLER), %xax          )
L(    movx   %xax, L_OFFSETOF_XAX(%xsp)                                          )
L(1:                                                                             )
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
L(    testb  $(SIGENTER_MODE_RESTORE_LONG&~(SIGENTER_MODE_RESTORE)), L_OFFSETOF_MODE(%xsp))
L(    jz     1f                                                                  )
L(    movx   TASK_OFFSETOF_SIGENTER+SIGENTER_OFFSETOF_XDX(CALLER), %xdx          )
L(1:                                                                             )
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */
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
L(    movabs $((VM_USER_MAX_A - SIGENTER_TAIL_SIZE)+1), %rax                     )
L(    cmpq   %rax,                                     ITER                      )
#else
L(    cmpx   $((VM_USER_MAX_A - SIGENTER_TAIL_SIZE)+1), ITER                     )
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
L(SYM_END(sigenter)                                                              )
#ifdef SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR
L(SYM_END(sigenter_restart)                                                      )
#endif /* SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR */
L(SYM_END(sigenter_restore)                                                      )
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
 USER struct sigenter_info *dst; struct mman *omm;
 struct irregs_syscall *return_registers;
 struct sigenter_tail tail; size_t copy_error;
 assert(!PREEMPTION_ENABLED());
 assert(TASK_CPU(t) == THIS_CPU);
 if (++t->t_sigenter.se_count == 1) {
  /* The first signal enter. */
  void (ASMCALL *used_sigenter)(void);
  /* Terminate the chain of user-space signal handler contexts.
   * (Thus allowing userspace to detect invocation recursion) */
  t->t_sigenter.se_next = container_of((ucontext_t *)NULL,struct sigenter_tail,t_ctx);
  return_registers = IRREGS_SYSCALL_GET_FOR(t);
  /* Check if the task is currently attempting to execute a system call. */
  used_sigenter = &sigenter;
  if (syscall_is_norestart(return_registers->sysno));
  else {
   if (task_is_running_syscall(t->t_real_mman,return_registers->xip)) {
    if (action->sa_flags&SA_RESTART) {
     t->t_sigenter.se_xax = return_registers->sysno;
     /* Adjust the instruction pointer to repeat the system-call. */
     return_registers->xip -= 2;
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
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
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */
     {
      /* Don't attempt to restore long return registers here.
       * Without knowing for certain that the current system-call is actually a long-call,
       * we have no way of knowing where the XDX value is located, it it was even saved at all!
       * Instead, assume that the system call _only_ returns on XAX, which has already been
       * saved above as the lookup process for that register actually is standardized:
       * >> ORIG_EAX = IRREGS_SYSCALL_GET_FOR(t)->sysno; */
      used_sigenter = &sigenter_restore;
     }
    }
#ifdef SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR
    else {
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
     if (syscall_is_long(return_registers->sysno)) {
      used_sigenter = &sigenter_restart_long;
     } else
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */
     {
      used_sigenter = &sigenter_restart;
     }
    }
#endif /* SIGENTER_RESTORE_RESTART_INSTEAD_OF_INTR */
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
  return_registers->userrsp = (u64)t->t_hstack.hs_end;
  return_registers->ss      = __KERNEL_DS;
#endif
  return_registers->xflags &= ~EFLAGS_IF;
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
  /* Use sigaltstack() if it requested to. */
  if (action->sa_flags&SA_ONSTACK &&
      KERNEL_SIGSTACK_ISVALID(t->t_sigstack) &&
     !KERNEL_SIGSTACK_CONTAINS(t->t_sigstack,t->t_sigenter.se_xsp)) {
   t->t_sigenter.se_xsp = ((uintptr_t)t->t_sigstack.ss_base+
                                      t->t_sigstack.ss_size);
  } else {
#if USER_REDZONE_SIZE != 0
   /* Skip memory required for the `red' zone when pushing the first handler. */
   t->t_sigenter.se_xsp -= USER_REDZONE_SIZE;
#endif
  }
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

INTERN void KCALL
coredump_task(struct task *__restrict t,
              siginfo_t const *__restrict reason,
              greg_t reg_trapno, greg_t reg_err) {
 if (t != THIS_TASK && !t->t_sigenter.se_count && (t->t_cstate->iret.cs&3)) {
  coredump_user_task(t,reason,reg_trapno,reg_err);
 } else {
  coredump_host_task(t,reason,reg_trapno,reg_err);
 }
}

INTERN errno_t KCALL
deliver_signal(struct task *__restrict t,
               struct sigaction const *__restrict action,
               struct __siginfo_struct const *__restrict signal_info,
               greg_t reg_trapno, greg_t reg_err) {
 if (t != THIS_TASK && !t->t_sigenter.se_count && (t->t_cstate->iret.cs&3)) {
  /* The task isn't active, and currently running in user-space. */
  return deliver_signal_to_task_in_user(t,action,signal_info,reg_trapno,reg_err);
 } else {
  return deliver_signal_to_task_in_host(t,action,signal_info,reg_trapno,reg_err);
 }
}

DECL_END

#endif /* !CONFIG_NO_SIGNALS */


#endif /* !GUARD_KERNEL_ARCH_SIGNAL_C */
