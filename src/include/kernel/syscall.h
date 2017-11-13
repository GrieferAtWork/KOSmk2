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
#ifndef GUARD_INCLUDE_KERNEL_SYSCALL_H
#define GUARD_INCLUDE_KERNEL_SYSCALL_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <arch/cpustate.h>
#include <sched/percpu.h>

DECL_BEGIN

/* Low-level, assembly syscall function.
 *  - All registers, (except for `ESP', `EFLAGS' and `CS')
 *    will match those of the caller in userspace, essentially
 *    mirroring normal IRQ behavior exactly.
 *  - The kernel stack has been installed, and `IRET' can be used to return to userspace.
 *    With that in mind, `ESP' will point to an IRET-tail describing the user's EIP, etc.
 * NOTE: Executing user-level interrupts from kernel-space is allowed to cause undefined behavior.
 *       Implementors of different system calls may decide for them self if their system
 *       call is allowed to be called from ring#0
 *    -> The lack of this being mentioned in documentations of individual system calls
 *       usually implies that the call is not supported from kernel-space, even when
 *       if might work for some reason.
 * NOTE: When userspace tries to call a missing interrupt, `-ENOSYS' is usually returned.
 */
typedef syscall_ulong_t (ASMCALL *syscall_t)(void);
#define SYSCALL_INT 0x80


#ifdef CONFIG_DEBUG
#undef  CONFIG_SYSCALL_CHECK_SEGMENTS
#ifndef CONFIG_NO_SYSCALL_CHECK_SEGMENTS
#define CONFIG_SYSCALL_CHECK_SEGMENTS
#endif
#endif
#define __STACKBASE_TASK     THIS_TASK

/* While inside a system-call, these helper macros can be used
 * for accessing saved user-space registers (for reading & writing),
 * such as the user-space return address (EIP)
 * WARNING: These macros only work for system-calls originating from user-space!
 */
#ifdef __x86_64__
/* x86_64: IN(rdi, rsi, rdx, r10, r8,  r9)  OUT(rax[,rdx]) */
struct syscall_descr {
    /* TODO: Don't actually safe these on the stack! - This isn't i386 any more. */
    __COMMON_REG2(di);
    __COMMON_REG2(si);
    __COMMON_REG1(d);
    __COMMON_REG3(10);
    __COMMON_REG3(8);
    __COMMON_REG3(9);
    u64 gs,fs;
#ifdef CONFIG_DEBUG
    __COMMON_REG2_EX(__initial_,bp);
#endif /* CONFIG_DEBUG */
    __COMMON_REG2(ip);
    IRET_SEGMENT(cs);
    __COMMON_REG2(flags);
    /* Only for system-calls originating from user-space. */
    __COMMON_REG2_EX(user,sp);
    IRET_SEGMENT(ss);
};

#define __STACKBASE_VALUE(type,off) \
     (*(type *)((uintptr_t)__STACKBASE_TASK->t_hstack.hs_end+(off)))

#define THIS_SYSCALL_SS      __STACKBASE_VALUE(u16,-8)
#define THIS_SYSCALL_USERESP __STACKBASE_VALUE(void *,-16)
#define THIS_SYSCALL_EFLAGS  __STACKBASE_VALUE(u64,-24)
#define THIS_SYSCALL_CS      __STACKBASE_VALUE(u16,-32)
#define THIS_SYSCALL_EIP     __STACKBASE_VALUE(void *,-40)
/* WARNING: Everything that follows doesn't reliably work in assembly system calls. */
#ifdef CONFIG_DEBUG
#define THIS_SYSCALL_FS      __STACKBASE_VALUE(u16,-56)
#define THIS_SYSCALL_GS      __STACKBASE_VALUE(u16,-64)
#define THIS_SYSCALL_R9      __STACKBASE_VALUE(u64,-72)
#define THIS_SYSCALL_R8      __STACKBASE_VALUE(u64,-80)
#define THIS_SYSCALL_R10     __STACKBASE_VALUE(u64,-88)
#define THIS_SYSCALL_RDX     __STACKBASE_VALUE(u64,-96)
#define THIS_SYSCALL_RSI     __STACKBASE_VALUE(u64,-104)
#define THIS_SYSCALL_RDI     __STACKBASE_VALUE(u64,-112)
#else /* CONFIG_DEBUG */
#define THIS_SYSCALL_FS      __STACKBASE_VALUE(u16,-48)
#define THIS_SYSCALL_GS      __STACKBASE_VALUE(u16,-56)
#define THIS_SYSCALL_R9      __STACKBASE_VALUE(u64,-64)
#define THIS_SYSCALL_R8      __STACKBASE_VALUE(u64,-72)
#define THIS_SYSCALL_R10     __STACKBASE_VALUE(u64,-80)
#define THIS_SYSCALL_RDX     __STACKBASE_VALUE(u64,-88)
#define THIS_SYSCALL_RSI     __STACKBASE_VALUE(u64,-96)
#define THIS_SYSCALL_RDI     __STACKBASE_VALUE(u64,-104)
#endif /* !CONFIG_DEBUG */

/* ~Real~ system call return values.
 * >> Still point into user-space after a signal
 *    handler overwrote the return address. */
#define THIS_SYSCALL_REAL_XIP     (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_xip : THIS_SYSCALL_EIP)
#define THIS_SYSCALL_REAL_CS      (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_cs : THIS_SYSCALL_CS)
#define THIS_SYSCALL_REAL_XFLAGS  (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_xflags : THIS_SYSCALL_EFLAGS)
#define THIS_SYSCALL_REAL_USERXSP (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_userxsp : THIS_SYSCALL_USERESP)
#define THIS_SYSCALL_REAL_SS      (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_ss : THIS_SYSCALL_SS)
#define SET_THIS_SYSCALL_REAL_XIP(x)     (*(THIS_TASK->t_sigenter.se_count ? &THIS_TASK->t_sigenter.se_xip : &THIS_SYSCALL_EIP) = (x))
#define SET_THIS_SYSCALL_REAL_CS(x)      (*(THIS_TASK->t_sigenter.se_count ? &THIS_TASK->t_sigenter.se_cs : &THIS_SYSCALL_CS) = (x))
#define SET_THIS_SYSCALL_REAL_XFLAGS(x)  (*(THIS_TASK->t_sigenter.se_count ? &THIS_TASK->t_sigenter.se_xflags : &THIS_SYSCALL_EFLAGS) = (x))
#define SET_THIS_SYSCALL_REAL_USERXSP(x) (*(THIS_TASK->t_sigenter.se_count ? &THIS_TASK->t_sigenter.se_userxsp : &THIS_SYSCALL_USERESP) = (x))
#define SET_THIS_SYSCALL_REAL_SS(x)      (*(THIS_TASK->t_sigenter.se_count ? &THIS_TASK->t_sigenter.se_ss : &THIS_SYSCALL_SS) = (x))

#elif defined(__i386__)

/* i386:   IN(ebx, ecx, edx, esi, edi, ebp) OUT(eax[,edx]) */
struct syscall_descr {
 __COMMON_REG1(b);
 __COMMON_REG1(c);
 __COMMON_REG1(d);
 __COMMON_REG2(si);
 __COMMON_REG2(di);
 __COMMON_REG2(bp);
#ifdef CONFIG_DEBUG
 __COMMON_REG2_EX(__initial_,ip);
#endif /* CONFIG_DEBUG */
 u16 gs,fs,es,ds;
 __COMMON_REG2(ip); IRET_SEGMENT(cs); __COMMON_REG2(flags);
 /* Only for system-calls originating from user-space. */
 __COMMON_REG2_EX(user,sp); IRET_SEGMENT(ss);
};

#define __STACKBASE_VALUE(type,off) \
     (*(type *)((uintptr_t)__STACKBASE_TASK->t_hstack.hs_end+(off)))

#define THIS_SYSCALL_SS      __STACKBASE_VALUE(u16,-4)
#define THIS_SYSCALL_USERESP __STACKBASE_VALUE(void *,-8)
#define THIS_SYSCALL_EFLAGS  __STACKBASE_VALUE(u32,-12)
#define THIS_SYSCALL_CS      __STACKBASE_VALUE(u16,-16)
#define THIS_SYSCALL_EIP     __STACKBASE_VALUE(void *,-20)
/* WARNING: Everything that follows doesn't reliably work in assembly system calls. */
#define THIS_SYSCALL_DS      __STACKBASE_VALUE(u16,-22)
#define THIS_SYSCALL_ES      __STACKBASE_VALUE(u16,-24)
#define THIS_SYSCALL_FS      __STACKBASE_VALUE(u16,-26)
#define THIS_SYSCALL_GS      __STACKBASE_VALUE(u16,-28)
#ifdef CONFIG_DEBUG
#define THIS_SYSCALL_EBP     __STACKBASE_VALUE(u32,-36)
#define THIS_SYSCALL_EDI     __STACKBASE_VALUE(u32,-40)
#define THIS_SYSCALL_ESI     __STACKBASE_VALUE(u32,-44)
#define THIS_SYSCALL_EDX     __STACKBASE_VALUE(u32,-48)
#define THIS_SYSCALL_ECX     __STACKBASE_VALUE(u32,-52)
#define THIS_SYSCALL_EBX     __STACKBASE_VALUE(u32,-56)
#else /* CONFIG_DEBUG */
#define THIS_SYSCALL_EBP     __STACKBASE_VALUE(u32,-32)
#define THIS_SYSCALL_EDI     __STACKBASE_VALUE(u32,-36)
#define THIS_SYSCALL_ESI     __STACKBASE_VALUE(u32,-40)
#define THIS_SYSCALL_EDX     __STACKBASE_VALUE(u32,-44)
#define THIS_SYSCALL_ECX     __STACKBASE_VALUE(u32,-48)
#define THIS_SYSCALL_EBX     __STACKBASE_VALUE(u32,-52)
#endif /* !CONFIG_DEBUG */

/* ~Real~ system call return values.
 * >> Still point into user-space after a signal
 *    handler overwrote the return address. */
#define THIS_SYSCALL_REAL_XIP              (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_xip : THIS_SYSCALL_EIP)
#define THIS_SYSCALL_REAL_CS               (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_cs : THIS_SYSCALL_CS)
#define THIS_SYSCALL_REAL_XFLAGS           (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_xflags : THIS_SYSCALL_EFLAGS)
#define THIS_SYSCALL_REAL_USERXSP          (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_userxsp : THIS_SYSCALL_USERESP)
#define THIS_SYSCALL_REAL_SS               (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_ss : THIS_SYSCALL_SS)
#define SET_THIS_SYSCALL_REAL_XIP(x)     (*(THIS_TASK->t_sigenter.se_count ? &THIS_TASK->t_sigenter.se_xip : &THIS_SYSCALL_EIP) = (x))
#define SET_THIS_SYSCALL_REAL_CS(x)      (*(THIS_TASK->t_sigenter.se_count ? &THIS_TASK->t_sigenter.se_cs : &THIS_SYSCALL_CS) = (x))
#define SET_THIS_SYSCALL_REAL_XFLAGS(x)  (*(THIS_TASK->t_sigenter.se_count ? &THIS_TASK->t_sigenter.se_xflags : &THIS_SYSCALL_EFLAGS) = (x))
#define SET_THIS_SYSCALL_REAL_USERXSP(x) (*(THIS_TASK->t_sigenter.se_count ? &THIS_TASK->t_sigenter.se_userxsp : &THIS_SYSCALL_USERESP) = (x))
#define SET_THIS_SYSCALL_REAL_SS(x)      (*(THIS_TASK->t_sigenter.se_count ? &THIS_TASK->t_sigenter.se_ss : &THIS_SYSCALL_SS) = (x))

#else
#error "FIXME: Need at least `THIS_SYSCALL_EIP'"
#endif



#if defined(CONFIG_SYSCALL_CHECK_SEGMENTS) && 0
/* Check that the segment register state is correct before a sysreturn is performed. */
FUNDEF void ASMCALL sysreturn_check_segments(void);
#define SYSRETURN_CHECK_SEGMENTS()    sysreturn_check_segments()
#define ASM_SYSRETURN_CHECK_SEGMENTS  call sysreturn_check_segments
#else
#define SYSRETURN_CHECK_SEGMENTS()    (void)0
#define ASM_SYSRETURN_CHECK_SEGMENTS  ; /* Nothing */
#endif


#define __VA_ARGS(...) __VA_ARGS__
#define __SC_DECL0(v)                                   v
#define __SC_DECL1(t1,a1)                               t1 a1
#define __SC_DECL2(t2,a2,t1,a1)                         t2 a2, __SC_DECL1(t1,a1)
#define __SC_DECL3(t3,a3,t2,a2,t1,a1)                   t3 a3, __SC_DECL2(t2,a2,t1,a1)
#define __SC_DECL4(t4,a4,t3,a3,t2,a2,t1,a1)             t4 a4, __SC_DECL3(t3,a3,t2,a2,t1,a1)
#define __SC_DECL5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)       t5 a5, __SC_DECL4(t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_DECL6(t6,a6,t5,a5,t4,a4,t3,a3,t2,a2,t1,a1) t6 a6, __SC_DECL5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_LONG0(v)                                   v
#define __SC_LONG1(t1,a1)                               syscall_slong_t a1
#define __SC_LONG2(t2,a2,t1,a1)                         syscall_slong_t a2, __SC_LONG1(t1,a1)
#define __SC_LONG3(t3,a3,t2,a2,t1,a1)                   syscall_slong_t a3, __SC_LONG2(t2,a2,t1,a1)
#define __SC_LONG4(t4,a4,t3,a3,t2,a2,t1,a1)             syscall_slong_t a4, __SC_LONG3(t3,a3,t2,a2,t1,a1)
#define __SC_LONG5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)       syscall_slong_t a5, __SC_LONG4(t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_LONG6(t6,a6,t5,a5,t4,a4,t3,a3,t2,a2,t1,a1) syscall_slong_t a6, __SC_LONG5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_CAST0(v)                                   /* Nothing */
#define __SC_CAST1(t1,a1)                               (t1)a1
#define __SC_CAST2(t2,a2,t1,a1)                         (t2)a2, __SC_CAST1(t1,a1)
#define __SC_CAST3(t3,a3,t2,a2,t1,a1)                   (t3)a3, __SC_CAST2(t2,a2,t1,a1)
#define __SC_CAST4(t4,a4,t3,a3,t2,a2,t1,a1)             (t4)a4, __SC_CAST3(t3,a3,t2,a2,t1,a1)
#define __SC_CAST5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)       (t5)a5, __SC_CAST4(t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_CAST6(t6,a6,t5,a5,t4,a4,t3,a3,t2,a2,t1,a1) (t6)a6, __SC_CAST5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_TEST(type)        STATIC_ASSERT(sizeof(type) <= sizeof(syscall_slong_t))
#define __SC_TEST0(v)                                    /* Nothing */
#define __SC_TEST1(t1,a1)                                __SC_TEST(t1)
#define __SC_TEST2(t2,a2,t1,a1)                          __SC_TEST(t2); __SC_TEST1(t1,a1)
#define __SC_TEST3(t3,a3,t2,a2,t1,a1)                    __SC_TEST(t3); __SC_TEST2(t2,a2,t1,a1)
#define __SC_TEST4(t4,a4,t3,a3,t2,a2,t1,a1)              __SC_TEST(t4); __SC_TEST3(t3,a3,t2,a2,t1,a1)
#define __SC_TEST5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)        __SC_TEST(t5); __SC_TEST4(t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_TEST6(t6,a6,t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)  __SC_TEST(t6); __SC_TEST5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)

#define __SYSCALL_DEFINEx(n,name,args) \
  FORCELOCAL syscall_slong_t ATTR_CDECL SYSC##name(__SC_DECL##n args); \
  INTERN syscall_slong_t ATTR_CDECL sys##name(__SC_LONG##n args) { \
    __SC_TEST##n args; \
    return (syscall_slong_t)SYSC##name(__SC_CAST##n args); \
  } \
  FORCELOCAL syscall_slong_t ATTR_CDECL SYSC##name(__SC_DECL##n args)


#define SYSCALL_DEFINE0(name)     __SYSCALL_DEFINEx(0,_##name,(void))
#define SYSCALL_DEFINE1(name,...) __SYSCALL_DEFINEx(1,_##name,(__VA_ARGS__))
#define SYSCALL_DEFINE2(name,...) __SYSCALL_DEFINEx(2,_##name,(__VA_ARGS__))
#define SYSCALL_DEFINE3(name,...) __SYSCALL_DEFINEx(3,_##name,(__VA_ARGS__))
#define SYSCALL_DEFINE4(name,...) __SYSCALL_DEFINEx(4,_##name,(__VA_ARGS__))
#define SYSCALL_DEFINE5(name,...) __SYSCALL_DEFINEx(5,_##name,(__VA_ARGS__))
#define SYSCALL_DEFINE6(name,...) __SYSCALL_DEFINEx(6,_##name,(__VA_ARGS__))


#if __SIZEOF_SYSCALL_LONG__ < 8
#define __LSYSCALL_DEFINEx(n,name,args) \
  FORCELOCAL s64 ATTR_CDECL SYSC##name(__SC_DECL##n args); \
  INTERN s64 ATTR_CDECL sys##name(__SC_LONG##n args) { \
    __SC_TEST##n args; \
    return (s64)SYSC##name(__SC_CAST##n args); \
  } \
  FORCELOCAL s64 ATTR_CDECL SYSC##name(__SC_DECL##n args)
#define SYSCALL_LDEFINE1(name,...) __SYSCALL_DEFINEx(1,_##name,(__VA_ARGS__))
#define SYSCALL_LDEFINE2(name,...) __SYSCALL_DEFINEx(2,_##name,(__VA_ARGS__))
#define SYSCALL_LDEFINE3(name,...) __SYSCALL_DEFINEx(3,_##name,(__VA_ARGS__))
#define SYSCALL_LDEFINE4(name,...) __SYSCALL_DEFINEx(4,_##name,(__VA_ARGS__))
#define SYSCALL_LDEFINE5(name,...) __SYSCALL_DEFINEx(5,_##name,(__VA_ARGS__))
#define SYSCALL_LDEFINE6(name,...) __SYSCALL_DEFINEx(6,_##name,(__VA_ARGS__))
#else
#define SYSCALL_LDEFINE1(name,...) __SYSCALL_DEFINEx(1,_##name,(__VA_ARGS__))
#define SYSCALL_LDEFINE2(name,...) __SYSCALL_DEFINEx(2,_##name,(__VA_ARGS__))
#define SYSCALL_LDEFINE3(name,...) __SYSCALL_DEFINEx(3,_##name,(__VA_ARGS__))
#define SYSCALL_LDEFINE4(name,...) __SYSCALL_DEFINEx(4,_##name,(__VA_ARGS__))
#define SYSCALL_LDEFINE5(name,...) __SYSCALL_DEFINEx(5,_##name,(__VA_ARGS__))
#define SYSCALL_LDEFINE6(name,...) __SYSCALL_DEFINEx(6,_##name,(__VA_ARGS__))
#endif

DECL_END

#include <hybrid/host.h>
#include <arch/cpustate.h>
#include <hybrid/asm.h>
#include <asm/instx.h>
#include <kernel/irq.h>

DECL_BEGIN

/* System call with full register state. */
#define SYSCALL_RDEFINE(name,regs) \
PRIVATE ATTR_USED void FCALL SYSC_##name(struct cpustate *__restrict regs); \
GLOBAL_ASM(L(.section .text                                                       ) \
           L(INTERN_ENTRY(sys_##name)                                             ) \
           L(    __ASM_PUSH_COMREGS                                               ) \
           L(    __ASM_LOAD_SEGMENTS(%dx)                                         ) \
           L(    movx %xsp, %FASTCALL_REG1                                        ) \
           L(    call SYSC_##name                                                 ) \
           L(    __ASM_POP_COMREGS                                                ) \
           L(    __ASM_IRET                                                       ) \
           L(SYM_END(sys_##name)                                                  ) \
           L(.previous)); \
PRIVATE ATTR_USED void FCALL SYSC_##name(struct cpustate *__restrict regs)


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_SYSCALL_H */
