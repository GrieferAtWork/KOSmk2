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
#include <kernel/arch/cpustate.h>
#include <sched/percpu.h>

DECL_BEGIN

/* Low-level, assembly syscall function.
 *  - All registers, (except for 'ESP', 'EFLAGS' and 'CS')
 *    will match those of the caller in userspace, essentially
 *    mirroring normal IRQ behavior exactly.
 *  - The kernel stack has been installed, and 'IRET' can be used to return to userspace.
 *    With that in mind, 'ESP' will point to an IRET-tail describing the user's EIP, etc.
 * NOTE: Executing user-level interrupts from kernel-space is allowed to cause undefined behavior.
 *       Implementors of different system calls may decide for them self if their system
 *       call is allowed to be called from ring#0
 *    -> The lack of this being mentioned in documentations of individual system calls
 *       usually implies that the call is not supported from kernel-space, even when
 *       if might work for some reason.
 * NOTE: When userspace tries to call a missing interrupt, '-ENOSYS' is usually returned.
 */
typedef syscall_ulong_t (ASMCALL *syscall_t)(void);
#define SYSCALL_INT 0x80


/* While inside a system-call, these helper macros can be used
 * for accessing saved user-space registers (for reading & writing),
 * such as the user-space return address (EIP)
 * WARNING: These macros only work for system-calls originating from user-space!
 */
#ifdef __i386__
#ifdef CONFIG_DEBUG
#undef  CONFIG_SYSCALL_CHECK_SEGMENTS
#ifndef CONFIG_NO_SYSCALL_CHECK_SEGMENTS
#define CONFIG_SYSCALL_CHECK_SEGMENTS
#endif
#endif

#define __STACKBASE_TASK     THIS_TASK
struct syscall_descr {
 u32 ebx,ecx,edx,esi,edi,ebp;
#ifdef CONFIG_DEBUG
 u32 __initial_eip;
#endif /* CONFIG_DEBUG */
 u16 gs,fs,es,ds;
 u32 eip; u16 cs,__n0; u32 eflags;
#if 1 /* Only for system-calls originating from user-space. */
 u32 useresp; u16 ss,__n1;
#endif
};

#define __STACKBASE_VALUE(type,off) \
     (*(type *)((uintptr_t)__STACKBASE_TASK->t_hstack.hs_end+(off)))

#define THIS_SYSCALL_SS      __STACKBASE_VALUE(u16,-4)
#define THIS_SYSCALL_USERESP __STACKBASE_VALUE(void *,-8)
#define THIS_SYSCALL_EFLAGS  __STACKBASE_VALUE(u32,-12)
#define THIS_SYSCALL_CS      __STACKBASE_VALUE(u16,-16)
/* NOTE: 'THIS_SYSCALL_EIP' */
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
#define THIS_SYSCALL_REAL_EIP     (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_eip : THIS_SYSCALL_EIP)
#define THIS_SYSCALL_REAL_CS      (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_cs : THIS_SYSCALL_CS)
#define THIS_SYSCALL_REAL_EFLAGS  (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_eflags : THIS_SYSCALL_EFLAGS)
#define THIS_SYSCALL_REAL_USERESP (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_useresp : THIS_SYSCALL_USERESP)
#define THIS_SYSCALL_REAL_SS      (THIS_TASK->t_sigenter.se_count ? THIS_TASK->t_sigenter.se_ss : THIS_SYSCALL_SS)

#else
#error "FIXME: Need at least 'THIS_SYSCALL_EIP'"
#endif



#if defined(CONFIG_SYSCALL_CHECK_SEGMENTS) && 0
/* Check that the segment register state is correct before a sysreturn is performed. */
FUNDEF void ASMCALL sysreturn_check_segments(void);
#define SYSRETURN_CHECK_SEGMENTS()    sysreturn_check_segments()
#define ASM_SYSRETURN_CHECK_SEGMENTS  call sysreturn_check_segments
#else
#define SYSRETURN_CHECK_SEGMENTS()    (void)0
#define ASM_SYSRETURN_CHECK_SEGMENTS  ; /* nothing */
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

#endif /* !GUARD_INCLUDE_KERNEL_SYSCALL_H */
