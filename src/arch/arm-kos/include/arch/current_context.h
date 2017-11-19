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
#ifndef GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_CURRENT_CONTEXT_H
#define GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_CURRENT_CONTEXT_H 1

#include <hybrid/compiler.h>
#include <arch/cpustate.h>
#include <arch/interrupt.h>
#include <arch/preemption.h>
#include <assert.h>
#include <hybrid/types.h>
#include <hybrid/xch.h>
#include <sched/percpu.h>
#include <sched/types.h>

DECL_BEGIN

/* Get/Set current user-space IP/SP (Instruction/Stack) pointers. */
LOCAL WUNUSED USER void *KCALL get_current_userip(void);
LOCAL WUNUSED USER void *KCALL get_current_usersp(void);
LOCAL NOIRQ WUNUSED USER void *KCALL get_current_userip_noirq(void);
LOCAL NOIRQ WUNUSED USER void *KCALL get_current_usersp_noirq(void);
LOCAL WUNUSED USER void *KCALL get_current_userip_for(struct task *__restrict self);
LOCAL WUNUSED USER void *KCALL get_current_usersp_for(struct task *__restrict self);

LOCAL USER void *KCALL set_current_userip(USER void *new_ip);
LOCAL USER void *KCALL set_current_usersp(USER void *new_sp);
LOCAL USER void KCALL get_current_useripsp(USER void **__restrict pip, USER void **__restrict psp);
LOCAL USER void KCALL set_current_useripsp(USER void *new_ip, USER void *new_sp);
LOCAL NOIRQ USER void *KCALL set_current_userip_noirq(USER void *new_ip);
LOCAL NOIRQ USER void *KCALL set_current_usersp_noirq(USER void *new_sp);
LOCAL NOIRQ USER void KCALL set_current_useripsp_noirq(USER void *new_ip, USER void *new_sp);
LOCAL USER void *KCALL set_current_userip_for(struct task *__restrict self, USER void *new_ip);
LOCAL USER void *KCALL set_current_usersp_for(struct task *__restrict self, USER void *new_sp);
LOCAL USER void KCALL get_current_useripsp_for(struct task *__restrict self, USER void **__restrict pip, USER void **__restrict psp);
LOCAL USER void KCALL set_current_useripsp_for(struct task *__restrict self, USER void *new_ip, USER void *new_sp);

/* Get/set real current user-space pointer.
 * The difference between this and the functions above comes
 * into play when POSIX signal handlers have been invoked.
 * `get_current_*_r()' will return the ~real~ values as will
 * be used by the CPU, while the functions above return the
 * original values actually pointing into user-space.
 * WARNING: Unless preemption was disabled, these values may change randomly. */
LOCAL WUNUSED void *KCALL get_current_userip_r(void);
LOCAL WUNUSED void *KCALL get_current_usersp_r(void);
LOCAL NOIRQ WUNUSED void *KCALL get_current_userip_noirq_r(void);
LOCAL NOIRQ WUNUSED void *KCALL get_current_usersp_noirq_r(void);
LOCAL WUNUSED void *KCALL get_current_userip_for_r(struct task *__restrict self);
LOCAL WUNUSED void *KCALL get_current_usersp_for_r(struct task *__restrict self);

LOCAL void *KCALL set_current_userip_r(void *new_ip);
LOCAL void *KCALL set_current_usersp_r(void *new_sp);
LOCAL void KCALL get_current_useripsp_r(void **__restrict pip, void **__restrict psp);
LOCAL void KCALL set_current_useripsp_r(void *new_ip, void *new_sp);
LOCAL NOIRQ void *KCALL set_current_userip_noirq_r(void *new_ip);
LOCAL NOIRQ void *KCALL set_current_usersp_noirq_r(void *new_sp);
LOCAL NOIRQ void KCALL get_current_useripsp_noirq_r(void **__restrict pip, void **__restrict psp);
LOCAL NOIRQ void KCALL set_current_useripsp_noirq_r(void *new_ip, void *new_sp);
LOCAL void *KCALL set_current_userip_for_r(struct task *__restrict self, void *new_ip);
LOCAL void *KCALL set_current_usersp_for_r(struct task *__restrict self, void *new_sp);
LOCAL void KCALL get_current_useripsp_for_r(struct task *__restrict self, void **__restrict pip, void **__restrict psp);
LOCAL void KCALL set_current_useripsp_for_r(struct task *__restrict self, void *new_ip, void *new_sp);


LOCAL NOIRQ USER void *KCALL get_current_userip_noirq_r(void) { assert(!PREEMPTION_ENABLED()); return (void *)XCREGS_INTERRUPT_GET()->pc; }
LOCAL NOIRQ USER void *KCALL get_current_usersp_noirq_r(void) { assert(!PREEMPTION_ENABLED()); return (void *)XCREGS_INTERRUPT_GET()->sp; }
LOCAL NOIRQ void *KCALL set_current_userip_noirq_r(void *new_ip) { assert(!PREEMPTION_ENABLED()); return (void *)XCH(XCREGS_INTERRUPT_GET()->pc,(u32)new_ip); }
LOCAL NOIRQ void *KCALL set_current_usersp_noirq_r(void *new_sp) { assert(!PREEMPTION_ENABLED()); return (void *)XCH(XCREGS_INTERRUPT_GET()->sp,(u32)new_sp); }
LOCAL NOIRQ void KCALL get_current_useripsp_noirq_r(void **__restrict pip, void **__restrict psp) { struct xcregs *xcr = XCREGS_INTERRUPT_GET(); assert(!PREEMPTION_ENABLED()); *pip = (void *)xcr->pc; *psp = (void *)xcr->sp; }
LOCAL NOIRQ void KCALL set_current_useripsp_noirq_r(void *new_ip, void *new_sp) { struct xcregs *xcr = XCREGS_INTERRUPT_GET(); assert(!PREEMPTION_ENABLED()); xcr->pc = (u32)new_ip; xcr->sp = (u32)new_sp; }
LOCAL void *KCALL get_current_userip_for_r(struct task *__restrict self) { return (void *)XCREGS_INTERRUPT_GET_FOR(self)->pc; }
LOCAL void *KCALL get_current_usersp_for_r(struct task *__restrict self) { return (void *)XCREGS_INTERRUPT_GET_FOR(self)->sp; }
LOCAL void *KCALL set_current_userip_for_r(struct task *__restrict self, void *new_ip) { return (void *)XCH(XCREGS_INTERRUPT_GET_FOR(self)->pc,(u32)new_ip); }
LOCAL void *KCALL set_current_usersp_for_r(struct task *__restrict self, void *new_sp) { return (void *)XCH(XCREGS_INTERRUPT_GET_FOR(self)->sp,(u32)new_sp); }
LOCAL void KCALL get_current_useripsp_for_r(struct task *__restrict self, void **__restrict pip, void **__restrict psp) { struct xcregs *xcr = XCREGS_INTERRUPT_GET_FOR(self); *pip = (void *)xcr->pc; *psp = (void *)xcr->sp; }
LOCAL void KCALL set_current_useripsp_for_r(struct task *__restrict self, void *new_ip, void *new_sp) { struct xcregs *xcr = XCREGS_INTERRUPT_GET_FOR(self); xcr->pc = (u32)new_ip; xcr->sp = (u32)new_sp; }
LOCAL NOIRQ USER void *KCALL get_current_userip_noirq(void) { struct task *caller = THIS_TASK; return caller->t_sigenter.se_count ? (void *)caller->t_sigenter.se_ip : get_current_userip_noirq_r(); }
LOCAL NOIRQ USER void *KCALL set_current_userip_noirq(USER void *new_ip) { struct task *caller = THIS_TASK; return caller->t_sigenter.se_count ? (USER void *)XCH(caller->t_sigenter.se_ip,(uintptr_t)new_ip) : set_current_userip_noirq_r(new_ip);  }
LOCAL NOIRQ USER void *KCALL get_current_usersp_noirq(void) { struct task *caller = THIS_TASK; return caller->t_sigenter.se_count ? (void *)caller->t_sigenter.se_sp : get_current_usersp_noirq_r(); }
LOCAL NOIRQ USER void *KCALL set_current_usersp_noirq(USER void *new_sp) { struct task *caller = THIS_TASK; return caller->t_sigenter.se_count ? (USER void *)XCH(caller->t_sigenter.se_sp,(uintptr_t)new_sp) : set_current_usersp_noirq_r(new_sp);  }
LOCAL NOIRQ USER void KCALL get_current_useripsp_noirq(USER void **__restrict pip, USER void **__restrict psp) { struct task *caller = THIS_TASK; if (caller->t_sigenter.se_count) *pip = (void *)caller->t_sigenter.se_ip,*psp = (void *)caller->t_sigenter.se_sp; else get_current_useripsp_noirq_r(pip,psp);  }
LOCAL NOIRQ USER void KCALL set_current_useripsp_noirq(USER void *new_ip, USER void *new_sp) { struct task *caller = THIS_TASK; if (caller->t_sigenter.se_count) caller->t_sigenter.se_ip = (uintptr_t)new_ip,caller->t_sigenter.se_sp = (uintptr_t)new_sp; else set_current_useripsp_noirq_r(new_ip,new_sp);  }

LOCAL USER void *KCALL get_current_userip(void) { pflag_t was = PREEMPTION_PUSH(); USER void *result = get_current_userip_noirq(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL USER void *KCALL set_current_userip(USER void *new_ip) { pflag_t was = PREEMPTION_PUSH(); USER void *result = set_current_userip_noirq(new_ip); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL USER void *KCALL get_current_userip_for(struct task *__restrict self) { return self->t_sigenter.se_count ? (void *)self->t_sigenter.se_ip : get_current_userip_for_r(self); }
LOCAL USER void *KCALL set_current_userip_for(struct task *__restrict self, USER void *new_ip) { return self->t_sigenter.se_count ? (void *)XCH(self->t_sigenter.se_ip,(uintptr_t)new_ip) : set_current_userip_for_r(self,new_ip); }
LOCAL USER void *KCALL get_current_usersp(void) { pflag_t was = PREEMPTION_PUSH(); USER void *result = get_current_usersp_noirq(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL USER void *KCALL set_current_usersp(USER void *new_sp) { pflag_t was = PREEMPTION_PUSH(); void *result = set_current_usersp_noirq(new_sp); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL USER void *KCALL get_current_usersp_for(struct task *__restrict self) { return self->t_sigenter.se_count ? (void *)self->t_sigenter.se_sp : get_current_usersp_for_r(self); }
LOCAL USER void *KCALL set_current_usersp_for(struct task *__restrict self, USER void *new_sp) { return self->t_sigenter.se_count ? (void *)XCH(self->t_sigenter.se_sp,(uintptr_t)new_sp) : set_current_usersp_for_r(self,new_sp); }
LOCAL USER void KCALL get_current_useripsp(USER void **__restrict pip, USER void **__restrict psp) { pflag_t was = PREEMPTION_PUSH(); get_current_useripsp_noirq(pip,psp); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); }
LOCAL USER void KCALL get_current_useripsp_for(struct task *__restrict self, USER void **__restrict pip, USER void **__restrict psp) { if (self->t_sigenter.se_count) *pip = (void *)self->t_sigenter.se_ip,*psp = (void *)self->t_sigenter.se_sp; else get_current_useripsp_for_r(self,pip,psp); }
LOCAL USER void KCALL set_current_useripsp(USER void *new_ip, USER void *new_sp) { pflag_t was = PREEMPTION_PUSH(); set_current_useripsp_noirq(new_ip,new_sp); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); }
LOCAL USER void KCALL set_current_useripsp_for(struct task *__restrict self, USER void *new_ip, USER void *new_sp) { if (self->t_sigenter.se_count) self->t_sigenter.se_ip = (uintptr_t)new_ip,self->t_sigenter.se_sp = (uintptr_t)new_sp; else set_current_useripsp_for_r(self,new_ip,new_sp); }

LOCAL void *KCALL get_current_userip_r(void) { pflag_t was = PREEMPTION_PUSH(); void *result = get_current_userip_noirq_r(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL void *KCALL set_current_userip_r(void *new_ip) { pflag_t was = PREEMPTION_PUSH(); void *result = set_current_userip_noirq_r(new_ip); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL void *KCALL get_current_usersp_r(void) { pflag_t was = PREEMPTION_PUSH(); void *result = get_current_usersp_noirq_r(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL void *KCALL set_current_usersp_r(void *new_sp) { pflag_t was = PREEMPTION_PUSH(); void *result = set_current_usersp_noirq_r(new_sp); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL void KCALL get_current_useripsp_r(void **__restrict pip, void **__restrict psp) { pflag_t was = PREEMPTION_PUSH(); get_current_useripsp_noirq_r(pip,psp); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); }
LOCAL void KCALL set_current_useripsp_r(void *new_ip, void *new_sp) { pflag_t was = PREEMPTION_PUSH(); set_current_useripsp_noirq_r(new_ip,new_sp); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); }

DECL_END

#endif /* !GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_CURRENT_CONTEXT_H */
