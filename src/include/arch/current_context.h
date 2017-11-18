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
#ifndef GUARD_INCLUDE_ARCH_CURRENT_CONTEXT_H
#define GUARD_INCLUDE_ARCH_CURRENT_CONTEXT_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <arch/interrupt.h>
#include <sched/types.h>
#include <sched/percpu.h>
#include <hybrid/xch.h>
#include <arch/preemption.h>
#include <assert.h>

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

/* Arch-specific X86 IRET register access.
 * WARNING: userflags assumes that the current interrupt originates from user-space. */
LOCAL WUNUSED register_t KCALL get_current_userflags(void);
LOCAL NOIRQ WUNUSED register_t KCALL get_current_userflags_noirq(void);
LOCAL WUNUSED register_t KCALL get_current_userflags_for(struct task *__restrict self);
LOCAL register_t KCALL set_current_userflags(register_t new_flags);
LOCAL NOIRQ register_t KCALL set_current_userflags_noirq(register_t new_flags);
LOCAL register_t KCALL set_current_userflags_for(struct task *__restrict self, register_t new_flags);

/* X86-specific: Get/set real current user-space pointer.
 * The difference between this and the functions above comes
 * into play when POSIX signal handlers have been invoked.
 * `get_current_*_r()' will return the ~real~ values as will
 * be used by the CPU, while the functions above return the
 * original values actually pointing into user-space.
 * WARNING: Unless preemption was disabled, these values may change randomly. */
LOCAL WUNUSED void *KCALL get_current_userip_r(void);
LOCAL WUNUSED void *KCALL get_current_usersp_r(void);
LOCAL WUNUSED register_t KCALL get_current_userflags_r(void);
LOCAL WUNUSED u16 KCALL get_current_userss_r(void);
LOCAL WUNUSED u16 KCALL get_current_usercs_r(void);
LOCAL NOIRQ WUNUSED void *KCALL get_current_userip_noirq_r(void);
LOCAL NOIRQ WUNUSED void *KCALL get_current_usersp_noirq_r(void);
LOCAL NOIRQ WUNUSED register_t KCALL get_current_userflags_noirq_r(void);
LOCAL NOIRQ WUNUSED u16 KCALL get_current_userss_noirq_r(void);
LOCAL NOIRQ WUNUSED u16 KCALL get_current_usercs_noirq_r(void);
LOCAL WUNUSED void *KCALL get_current_userip_for_r(struct task *__restrict self);
LOCAL WUNUSED void *KCALL get_current_usersp_for_r(struct task *__restrict self);
LOCAL WUNUSED register_t KCALL get_current_userflags_for_r(struct task *__restrict self);
LOCAL WUNUSED u16 KCALL get_current_userss_for_r(struct task *__restrict self);
LOCAL WUNUSED u16 KCALL get_current_usercs_for_r(struct task *__restrict self);

LOCAL void *KCALL set_current_userip_r(void *new_ip);
LOCAL void *KCALL set_current_usersp_r(void *new_sp);
LOCAL void KCALL get_current_useripsp_r(void **__restrict pip, void **__restrict psp);
LOCAL void KCALL set_current_useripsp_r(void *new_ip, void *new_sp);
LOCAL register_t KCALL set_current_userflags_r(register_t new_flags);
LOCAL u16 KCALL set_current_userss_r(u16 new_ss);
LOCAL u16 KCALL set_current_usercs_r(u16 new_cs);
LOCAL NOIRQ void *KCALL set_current_userip_noirq_r(void *new_ip);
LOCAL NOIRQ void *KCALL set_current_usersp_noirq_r(void *new_sp);
LOCAL NOIRQ void KCALL get_current_useripsp_noirq_r(void **__restrict pip, void **__restrict psp);
LOCAL NOIRQ void KCALL set_current_useripsp_noirq_r(void *new_ip, void *new_sp);
LOCAL NOIRQ register_t KCALL set_current_userflags_noirq_r(register_t new_flags);
LOCAL NOIRQ u16 KCALL set_current_userss_noirq_r(u16 new_ss);
LOCAL NOIRQ u16 KCALL set_current_usercs_noirq_r(u16 new_cs);
LOCAL void *KCALL set_current_userip_for_r(struct task *__restrict self, void *new_ip);
LOCAL void *KCALL set_current_usersp_for_r(struct task *__restrict self, void *new_sp);
LOCAL void KCALL get_current_useripsp_for_r(struct task *__restrict self, void **__restrict pip, void **__restrict psp);
LOCAL void KCALL set_current_useripsp_for_r(struct task *__restrict self, void *new_ip, void *new_sp);
LOCAL register_t KCALL set_current_userflags_for_r(struct task *__restrict self, register_t new_flags);
LOCAL u16 KCALL set_current_userss_for_r(struct task *__restrict self, u16 new_ss);
LOCAL u16 KCALL set_current_usercs_for_r(struct task *__restrict self, u16 new_cs);





#ifdef __x86_64__
LOCAL NOIRQ USER void *KCALL get_current_userip_noirq_r(void) { assert(!PREEMPTION_ENABLED()); return (void *)IRREGS_INTERRUPT_GET()->rip; }
LOCAL NOIRQ USER void *KCALL get_current_usersp_noirq_r(void) { assert(!PREEMPTION_ENABLED()); return (void *)IRREGS_INTERRUPT_GET()->userrsp; }
LOCAL NOIRQ register_t KCALL get_current_userflags_noirq_r(void) { assert(!PREEMPTION_ENABLED()); return IRREGS_INTERRUPT_GET()->rflags; }
LOCAL NOIRQ void *KCALL set_current_userip_noirq_r(void *new_ip) { assert(!PREEMPTION_ENABLED()); return (void *)XCH(IRREGS_INTERRUPT_GET()->rip,(u64)new_ip); }
LOCAL NOIRQ void *KCALL set_current_usersp_noirq_r(void *new_sp) { assert(!PREEMPTION_ENABLED()); return (void *)XCH(IRREGS_INTERRUPT_GET()->userrsp,(u64)new_sp); }
LOCAL NOIRQ void KCALL get_current_useripsp_noirq_r(void **__restrict pip, void **__restrict psp) { struct irregs *irr = IRREGS_INTERRUPT_GET(); assert(!PREEMPTION_ENABLED()); *pip = (void *)irr->rip; *psp = (void *)irr->userrsp; }
LOCAL NOIRQ void KCALL set_current_useripsp_noirq_r(void *new_ip, void *new_sp) { struct irregs *irr = IRREGS_INTERRUPT_GET(); assert(!PREEMPTION_ENABLED()); irr->rip = (u64)new_ip; irr->userrsp = (u64)new_sp; }
LOCAL NOIRQ register_t KCALL set_current_userflags_noirq_r(register_t new_flags) { assert(!PREEMPTION_ENABLED()); return XCH(IRREGS_INTERRUPT_GET()->rflags,new_flags); }
LOCAL void *KCALL get_current_userip_for_r(struct task *__restrict self) { return (void *)IRREGS_INTERRUPT_GET_FOR(self)->rip; }
LOCAL void *KCALL get_current_usersp_for_r(struct task *__restrict self) { return (void *)IRREGS_INTERRUPT_GET_FOR(self)->userrsp; }
LOCAL register_t KCALL get_current_userflags_for_r(struct task *__restrict self) { return IRREGS_INTERRUPT_GET_FOR(self)->rflags; }
LOCAL void *KCALL set_current_userip_for_r(struct task *__restrict self, void *new_ip) { return (void *)XCH(IRREGS_INTERRUPT_GET_FOR(self)->rip,(u64)new_ip); }
LOCAL void *KCALL set_current_usersp_for_r(struct task *__restrict self, void *new_sp) { return (void *)XCH(IRREGS_INTERRUPT_GET_FOR(self)->userrsp,(u64)new_sp); }
LOCAL void KCALL get_current_useripsp_for_r(struct task *__restrict self, void **__restrict pip, void **__restrict psp) { struct irregs *irr = IRREGS_INTERRUPT_GET_FOR(self); *pip = (void *)irr->rip; *psp = (void *)irr->userrsp; }
LOCAL void KCALL set_current_useripsp_for_r(struct task *__restrict self, void *new_ip, void *new_sp) { struct irregs *irr = IRREGS_INTERRUPT_GET_FOR(self); irr->rip = (u64)new_ip; irr->userrsp = (u64)new_sp; }
LOCAL register_t KCALL set_current_userflags_for_r(struct task *__restrict self, register_t new_flags) { return XCH(IRREGS_INTERRUPT_GET_FOR(self)->rflags,new_flags); }
#else /* __x86_64__ */
LOCAL NOIRQ USER void *KCALL get_current_userip_noirq_r(void) { assert(!PREEMPTION_ENABLED()); return (void *)IRREGS_INTERRUPT_GET()->eip; }
LOCAL NOIRQ USER void *KCALL get_current_usersp_noirq_r(void) { assert(!PREEMPTION_ENABLED()); return (void *)IRREGS_INTERRUPT_GET()->useresp; }
LOCAL NOIRQ register_t KCALL get_current_userflags_noirq_r(void) { assert(!PREEMPTION_ENABLED()); return IRREGS_INTERRUPT_GET()->eflags; }
LOCAL NOIRQ void *KCALL set_current_userip_noirq_r(void *new_ip) { assert(!PREEMPTION_ENABLED()); return (void *)XCH(IRREGS_INTERRUPT_GET()->eip,(u32)new_ip); }
LOCAL NOIRQ void *KCALL set_current_usersp_noirq_r(void *new_sp) { assert(!PREEMPTION_ENABLED()); return (void *)XCH(IRREGS_INTERRUPT_GET()->useresp,(u32)new_sp); }
LOCAL NOIRQ void KCALL get_current_useripsp_noirq_r(void **__restrict pip, void **__restrict psp) { struct irregs *irr = IRREGS_INTERRUPT_GET(); assert(!PREEMPTION_ENABLED()); *pip = (void *)irr->eip; *psp = (void *)irr->useresp; }
LOCAL NOIRQ void KCALL set_current_useripsp_noirq_r(void *new_ip, void *new_sp) { struct irregs *irr = IRREGS_INTERRUPT_GET(); assert(!PREEMPTION_ENABLED()); irr->eip = (u32)new_ip; irr->useresp = (u32)new_sp; }
LOCAL NOIRQ register_t KCALL set_current_userflags_noirq_r(register_t new_flags) { assert(!PREEMPTION_ENABLED()); return XCH(IRREGS_INTERRUPT_GET()->eflags,new_flags); }
LOCAL void *KCALL get_current_userip_for_r(struct task *__restrict self) { return (void *)IRREGS_INTERRUPT_GET_FOR(self)->eip; }
LOCAL void *KCALL get_current_usersp_for_r(struct task *__restrict self) { return (void *)IRREGS_INTERRUPT_GET_FOR(self)->useresp; }
LOCAL register_t KCALL get_current_userflags_for_r(struct task *__restrict self) { return IRREGS_INTERRUPT_GET_FOR(self)->eflags; }
LOCAL void *KCALL set_current_userip_for_r(struct task *__restrict self, void *new_ip) { return (void *)XCH(IRREGS_INTERRUPT_GET_FOR(self)->eip,(u32)new_ip); }
LOCAL void *KCALL set_current_usersp_for_r(struct task *__restrict self, void *new_sp) { return (void *)XCH(IRREGS_INTERRUPT_GET_FOR(self)->useresp,(u32)new_sp); }
LOCAL void KCALL get_current_useripsp_for_r(struct task *__restrict self, void **__restrict pip, void **__restrict psp) { struct irregs *irr = IRREGS_INTERRUPT_GET_FOR(self); *pip = (void *)irr->eip; *psp = (void *)irr->useresp; }
LOCAL void KCALL set_current_useripsp_for_r(struct task *__restrict self, void *new_ip, void *new_sp) { struct irregs *irr = IRREGS_INTERRUPT_GET_FOR(self); irr->eip = (u32)new_ip; irr->useresp = (u32)new_sp; }
LOCAL register_t KCALL set_current_userflags_for_r(struct task *__restrict self, register_t new_flags) { return XCH(IRREGS_INTERRUPT_GET_FOR(self)->eflags,new_flags); }
#endif /* !__x86_64__ */
LOCAL NOIRQ u16 KCALL get_current_userss_noirq_r(void) { assert(!PREEMPTION_ENABLED()); return IRREGS_INTERRUPT_GET()->ss; }
LOCAL NOIRQ u16 KCALL set_current_userss_noirq_r(u16 new_ss) { assert(!PREEMPTION_ENABLED()); return XCH(IRREGS_INTERRUPT_GET()->ss,new_ss); }
LOCAL u16 KCALL get_current_userss_for_r(struct task *__restrict self) { return IRREGS_INTERRUPT_GET_FOR(self)->ss; }
LOCAL u16 KCALL set_current_userss_for_r(struct task *__restrict self, u16 new_ss) { return XCH(IRREGS_INTERRUPT_GET_FOR(self)->ss,new_ss); }
LOCAL NOIRQ u16 KCALL get_current_usercs_noirq_r(void) { assert(!PREEMPTION_ENABLED()); return IRREGS_INTERRUPT_GET()->cs; }
LOCAL NOIRQ u16 KCALL set_current_usercs_noirq_r(u16 new_cs) { assert(!PREEMPTION_ENABLED()); return XCH(IRREGS_INTERRUPT_GET()->cs,new_cs); }
LOCAL u16 KCALL get_current_usercs_for_r(struct task *__restrict self) { return IRREGS_INTERRUPT_GET_FOR(self)->cs; }
LOCAL u16 KCALL set_current_usercs_for_r(struct task *__restrict self, u16 new_cs) { return XCH(IRREGS_INTERRUPT_GET_FOR(self)->cs,new_cs); }
LOCAL NOIRQ USER void *KCALL get_current_userip_noirq(void) { struct task *caller = THIS_TASK; return caller->t_sigenter.se_count ? (void *)caller->t_sigenter.se_xip : get_current_userip_noirq_r(); }
LOCAL NOIRQ USER void *KCALL set_current_userip_noirq(USER void *new_ip) { struct task *caller = THIS_TASK; return caller->t_sigenter.se_count ? (USER void *)XCH(caller->t_sigenter.se_xip,(uintptr_t)new_ip) : set_current_userip_noirq_r(new_ip);  }
LOCAL NOIRQ USER void *KCALL get_current_usersp_noirq(void) { struct task *caller = THIS_TASK; return caller->t_sigenter.se_count ? (void *)caller->t_sigenter.se_xsp : get_current_usersp_noirq_r(); }
LOCAL NOIRQ USER void *KCALL set_current_usersp_noirq(USER void *new_sp) { struct task *caller = THIS_TASK; return caller->t_sigenter.se_count ? (USER void *)XCH(caller->t_sigenter.se_xsp,(uintptr_t)new_sp) : set_current_usersp_noirq_r(new_sp);  }
LOCAL NOIRQ USER void KCALL get_current_useripsp_noirq(USER void **__restrict pip, USER void **__restrict psp) { struct task *caller = THIS_TASK; if (caller->t_sigenter.se_count) *pip = (void *)caller->t_sigenter.se_xip,*psp = (void *)caller->t_sigenter.se_xsp; else get_current_useripsp_noirq_r(pip,psp);  }
LOCAL NOIRQ USER void KCALL set_current_useripsp_noirq(USER void *new_ip, USER void *new_sp) { struct task *caller = THIS_TASK; if (caller->t_sigenter.se_count) caller->t_sigenter.se_xip = (uintptr_t)new_ip,caller->t_sigenter.se_xsp = (uintptr_t)new_sp; else set_current_useripsp_noirq_r(new_ip,new_sp);  }
LOCAL NOIRQ register_t KCALL get_current_userflags_noirq(void) { struct task *caller = THIS_TASK; return caller->t_sigenter.se_count ? caller->t_sigenter.se_xflags : get_current_userflags_noirq_r(); }
LOCAL NOIRQ register_t KCALL set_current_userflags_noirq(register_t new_flags) { struct task *caller = THIS_TASK; return caller->t_sigenter.se_count ? XCH(caller->t_sigenter.se_xflags,new_flags) : set_current_userflags_noirq_r(new_flags);  }

LOCAL USER void *KCALL get_current_userip(void) { pflag_t was = PREEMPTION_PUSH(); USER void *result = get_current_userip_noirq(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL USER void *KCALL set_current_userip(USER void *new_ip) { pflag_t was = PREEMPTION_PUSH(); USER void *result = set_current_userip_noirq(new_ip); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL USER void *KCALL get_current_userip_for(struct task *__restrict self) { return self->t_sigenter.se_count ? (void *)self->t_sigenter.se_xip : get_current_userip_for_r(self); }
LOCAL USER void *KCALL set_current_userip_for(struct task *__restrict self, USER void *new_ip) { return self->t_sigenter.se_count ? (void *)XCH(self->t_sigenter.se_xip,(uintptr_t)new_ip) : set_current_userip_for_r(self,new_ip); }
LOCAL USER void *KCALL get_current_usersp(void) { pflag_t was = PREEMPTION_PUSH(); USER void *result = get_current_usersp_noirq(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL USER void *KCALL set_current_usersp(USER void *new_sp) { pflag_t was = PREEMPTION_PUSH(); void *result = set_current_usersp_noirq(new_sp); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL USER void *KCALL get_current_usersp_for(struct task *__restrict self) { return self->t_sigenter.se_count ? (void *)self->t_sigenter.se_xsp : get_current_usersp_for_r(self); }
LOCAL USER void *KCALL set_current_usersp_for(struct task *__restrict self, USER void *new_sp) { return self->t_sigenter.se_count ? (void *)XCH(self->t_sigenter.se_xsp,(uintptr_t)new_sp) : set_current_usersp_for_r(self,new_sp); }
LOCAL USER void KCALL get_current_useripsp(USER void **__restrict pip, USER void **__restrict psp) { pflag_t was = PREEMPTION_PUSH(); get_current_useripsp_noirq(pip,psp); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); }
LOCAL USER void KCALL get_current_useripsp_for(struct task *__restrict self, USER void **__restrict pip, USER void **__restrict psp) { if (self->t_sigenter.se_count) *pip = (void *)self->t_sigenter.se_xip,*psp = (void *)self->t_sigenter.se_xsp; else get_current_useripsp_for_r(self,pip,psp); }
LOCAL USER void KCALL set_current_useripsp(USER void *new_ip, USER void *new_sp) { pflag_t was = PREEMPTION_PUSH(); set_current_useripsp_noirq(new_ip,new_sp); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); }
LOCAL USER void KCALL set_current_useripsp_for(struct task *__restrict self, USER void *new_ip, USER void *new_sp) { if (self->t_sigenter.se_count) self->t_sigenter.se_xip = (uintptr_t)new_ip,self->t_sigenter.se_xsp = (uintptr_t)new_sp; else set_current_useripsp_for_r(self,new_ip,new_sp); }
LOCAL register_t KCALL get_current_userflags(void) { pflag_t was = PREEMPTION_PUSH(); register_t result = get_current_userflags_noirq(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL register_t KCALL set_current_userflags(register_t new_flags) { pflag_t was = PREEMPTION_PUSH(); register_t result = set_current_userflags_noirq(new_flags); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL register_t KCALL get_current_userflags_for(struct task *__restrict self) { return self->t_sigenter.se_count ? self->t_sigenter.se_xflags : get_current_userflags_for_r(self); }
LOCAL register_t KCALL set_current_userflags_for(struct task *__restrict self, register_t new_flags) { return self->t_sigenter.se_count ? XCH(self->t_sigenter.se_xflags,new_flags) : set_current_userflags_for_r(self,new_flags); }

LOCAL void *KCALL get_current_userip_r(void) { pflag_t was = PREEMPTION_PUSH(); void *result = get_current_userip_noirq_r(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL void *KCALL set_current_userip_r(void *new_ip) { pflag_t was = PREEMPTION_PUSH(); void *result = set_current_userip_noirq_r(new_ip); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL void *KCALL get_current_usersp_r(void) { pflag_t was = PREEMPTION_PUSH(); void *result = get_current_usersp_noirq_r(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL void *KCALL set_current_usersp_r(void *new_sp) { pflag_t was = PREEMPTION_PUSH(); void *result = set_current_usersp_noirq_r(new_sp); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL void KCALL get_current_useripsp_r(void **__restrict pip, void **__restrict psp) { pflag_t was = PREEMPTION_PUSH(); get_current_useripsp_noirq_r(pip,psp); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); }
LOCAL void KCALL set_current_useripsp_r(void *new_ip, void *new_sp) { pflag_t was = PREEMPTION_PUSH(); set_current_useripsp_noirq_r(new_ip,new_sp); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); }
LOCAL register_t KCALL get_current_userflags_r(void) { pflag_t was = PREEMPTION_PUSH(); register_t result = get_current_userflags_noirq_r(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL register_t KCALL set_current_userflags_r(register_t new_flags) { pflag_t was = PREEMPTION_PUSH(); register_t result = set_current_userflags_noirq_r(new_flags); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL u16 KCALL get_current_userss_r(void) { pflag_t was = PREEMPTION_PUSH(); u16 result = get_current_userss_noirq_r(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL u16 KCALL set_current_userss_r(u16 new_ss) { pflag_t was = PREEMPTION_PUSH(); u16 result = set_current_userss_noirq_r(new_ss); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL u16 KCALL get_current_usercs_r(void) { pflag_t was = PREEMPTION_PUSH(); u16 result = get_current_usercs_noirq_r(); COMPILER_READ_BARRIER(); PREEMPTION_POP(was); return result; }
LOCAL u16 KCALL set_current_usercs_r(u16 new_cs) { pflag_t was = PREEMPTION_PUSH(); u16 result = set_current_usercs_noirq_r(new_cs); COMPILER_WRITE_BARRIER(); PREEMPTION_POP(was); return result; }

DECL_END

#endif /* !GUARD_INCLUDE_ARCH_CURRENT_CONTEXT_H */
