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
#include <arch/cpustate.h>
#include <arch/syscall.h>
#include <hybrid/types.h>
#include <kernel/interrupt.h>
#include <sched/percpu.h>
#include <stdbool.h>

#define __SC_DECL0(v)                                    v
#define __SC_DECL1(t1,a1)                                t1 a1
#define __SC_DECL2(t2,a2,t1,a1)                          t2 a2, __SC_DECL1(t1,a1)
#define __SC_DECL3(t3,a3,t2,a2,t1,a1)                    t3 a3, __SC_DECL2(t2,a2,t1,a1)
#define __SC_DECL4(t4,a4,t3,a3,t2,a2,t1,a1)              t4 a4, __SC_DECL3(t3,a3,t2,a2,t1,a1)
#define __SC_DECL5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)        t5 a5, __SC_DECL4(t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_DECL6(t6,a6,t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)  t6 a6, __SC_DECL5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_LONG0(v)                                    v
#define __SC_LONG1(t1,a1)                                syscall_slong_t a1
#define __SC_LONG2(t2,a2,t1,a1)                          syscall_slong_t a2, __SC_LONG1(t1,a1)
#define __SC_LONG3(t3,a3,t2,a2,t1,a1)                    syscall_slong_t a3, __SC_LONG2(t2,a2,t1,a1)
#define __SC_LONG4(t4,a4,t3,a3,t2,a2,t1,a1)              syscall_slong_t a4, __SC_LONG3(t3,a3,t2,a2,t1,a1)
#define __SC_LONG5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)        syscall_slong_t a5, __SC_LONG4(t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_LONG6(t6,a6,t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)  syscall_slong_t a6, __SC_LONG5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_CAST0(v)                                    /* Nothing */
#define __SC_CAST1(t1,a1)                               (t1)a1
#define __SC_CAST2(t2,a2,t1,a1)                         (t2)a2, __SC_CAST1(t1,a1)
#define __SC_CAST3(t3,a3,t2,a2,t1,a1)                   (t3)a3, __SC_CAST2(t2,a2,t1,a1)
#define __SC_CAST4(t4,a4,t3,a3,t2,a2,t1,a1)             (t4)a4, __SC_CAST3(t3,a3,t2,a2,t1,a1)
#define __SC_CAST5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)       (t5)a5, __SC_CAST4(t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_CAST6(t6,a6,t5,a5,t4,a4,t3,a3,t2,a2,t1,a1) (t6)a6, __SC_CAST5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_TEST(type)                                  STATIC_ASSERT(sizeof(type) <= sizeof(syscall_slong_t))
#define __SC_TEST0(v)                                    /* Nothing */
#define __SC_TEST1(t1,a1)                                __SC_TEST(t1)
#define __SC_TEST2(t2,a2,t1,a1)                          __SC_TEST(t2); __SC_TEST1(t1,a1)
#define __SC_TEST3(t3,a3,t2,a2,t1,a1)                    __SC_TEST(t3); __SC_TEST2(t2,a2,t1,a1)
#define __SC_TEST4(t4,a4,t3,a3,t2,a2,t1,a1)              __SC_TEST(t4); __SC_TEST3(t3,a3,t2,a2,t1,a1)
#define __SC_TEST5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)        __SC_TEST(t5); __SC_TEST4(t4,a4,t3,a3,t2,a2,t1,a1)
#define __SC_TEST6(t6,a6,t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)  __SC_TEST(t6); __SC_TEST5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)

DECL_BEGIN

#define SYSCALL_TYPE_ARG       0x80
#define SYSCALL_TYPE_MASK      0x0f
/* NOTE: All custom system-call handlers _MUST_ always return normally!
 * HINT: But since these handlers are called while `task_iscrit()' is true,
 *       operations such as `task_terminate(THIS_TASK,...)' are still allowed. */
#define SYSCALL_TYPE_ASM       0x00 /* An assembly-level system call handler.
                                     * This type of handler is equivalent to `INTTYPE_ASM_SEG', in
                                     * that user-space segments are saved and the kernel segments
                                     * have been loaded.
                                     * Additionally, all registers except for `XAX' will contain
                                     * the same values, as were given through user-space.
                                     * To return from this type of system call, `ret' must be used.
                                     * Upon entry to the system call handler, the stack looks as follows:
                                     * >>     0(%xsp) - System caller cleanup return address (Popped by `ret')
                                     * >>   XSZ(%xsp) - ORIG_EAX
                                     * >> 2*XSZ(%xsp) - %xip
                                     * >> 3*XSZ(%xsp) - %cs
                                     * >> 4*XSZ(%xsp) - %xflags
                                     * >> 5*XSZ(%xsp) - %userxsp
                                     * >> 6*XSZ(%xsp) - %ss
                                     * HINT: This kind of stack-layout can easily be
                                     *       addressed using `struct irregs_syscall'. */
#define SYSCALL_TYPE_FAST      0x01 /* The most widely used type of system call, suitable for c-level callbacks.
                                     * Registers are passed according to the `SYSCALL_HANDLER' calling convention.
                                     * To define a system call of this type, use the `SYSCALL_DEFINE*' macros below. */
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
#define __SYSCALL_TYPE_LONGBIT 0x02
#define SYSCALL_TYPE_LONG      0x03 /* Same as `SYSCALL_TYPE_FAST', but allows the interrupt
                                     * handler to return a 64/128-bit value in XAX:XDX (on x86). */
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */

#if __SIZEOF_REGISTER__ >= 8
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
#   define SYSCALL_TYPE_128BIT    SYSCALL_TYPE_LONG
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */
#   define SYSCALL_TYPE_64BIT     SYSCALL_TYPE_FAST
#   define SYSCALL_TYPE_32BIT     SYSCALL_TYPE_FAST
#elif __SIZEOF_REGISTER__ >= 4
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
#   define SYSCALL_TYPE_64BIT     SYSCALL_TYPE_LONG
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */
#   define SYSCALL_TYPE_32BIT     SYSCALL_TYPE_FAST
#elif __SIZEOF_REGISTER__ >= 2
#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
#   define SYSCALL_TYPE_32BIT     SYSCALL_TYPE_LONG
#endif /* CONFIG_HAVE_SYSCALL_LONGBIT */
#endif


#define SYSCALL_TYPE_STATE     0x04 /* The most expensive type of system call: All user-space registers are
                                     * saved in a `struct cpustate', a pointer to which is passed as the first argument.
                                     * WARNING: This type of system call is invoked using the `SYSCALL_STATE_HANDLER' calling convention.
                                     * >> void SYSCALL_STATE_HANDLER my_syscall(struct cpustate *__restrict state) { ... }
                                     * HINT: Yes this type of handler still conforms to `struct irregs_syscall', because
                                     *       `struct cpustate' and `struct irregs_syscall' share binary compatibility
                                     *       due to `XAX' being the first register saved after past the CPU iret-tail. */
#define SYSCALL_TYPE_STATE_ARG 0x84 /* Same as `SYSCALL_TYPE_STATE', but also pass an additional closure-argument.
                                     * >> void SYSCALL_STATE_HANDLER my_syscall(struct cpustate *__restrict state, void *ext) { ... } */



#define SYSCALL_FLAG_NORMAL    0x00 /* Just a regular, old system-call. */
#define SYSCALL_FLAG_NORESTART 0x01 /* The system-call should not be restarted if interrupted, regardless of
                                     * `SA_RESTART' potentially being set in the user-space signal handler. */

#ifdef __CC__
typedef u8 syscall_type_t; /* One of `SYSCALL_TYPE_*' */
typedef u8 syscall_flag_t; /* Set of `SYSCALL_FLAG_*' */
#endif /* __CC__ */


#define SYSCALL_OFFSETOF_SYSNO       0
#define SYSCALL_OFFSETOF_TYPE        __SIZEOF_REGISTER__
#define SYSCALL_OFFSETOF_FLAGS      (__SIZEOF_REGISTER__+1)
#define SYSCALL_OFFSETOF_CALLBACK (2*__SIZEOF_REGISTER__)
#define SYSCALL_OFFSETOF_CLOSURE  (2*__SIZEOF_REGISTER__+__SIZEOF_POINTER__)
#define SYSCALL_OFFSETOF_HITS     (2*__SIZEOF_REGISTER__+2*__SIZEOF_POINTER__)
#ifdef CONFIG_BUILDING_KERNEL_CORE
#define SYSCALL_OFFSETOF_REFCNT   (2*__SIZEOF_REGISTER__+3*__SIZEOF_POINTER__)
#endif
#define SYSCALL_OFFSETOF_FINI     (2*__SIZEOF_REGISTER__+4*__SIZEOF_POINTER__)
#define SYSCALL_OFFSETOF_OWNER    (2*__SIZEOF_REGISTER__+5*__SIZEOF_POINTER__)
#define SYSCALL_SIZE              (2*__SIZEOF_REGISTER__+6*__SIZEOF_POINTER__)
#ifdef __CC__
struct syscall {
    register_t              sc_sysno;    /*< System call number. */
union PACKED {
    register_t            __sc_align;    /*< Align by registers. */
struct PACKED {
    syscall_type_t          sc_type;     /*< The type of system-call handler (One of `SYSCALL_TYPE_*'). */
    syscall_flag_t          sc_flags;    /*< Set of `SYSCALL_FLAG_*'. */
};};
union PACKED {
    void                   *sc_callback; /*< [1..1] System call callback pointer. */
union PACKED {
    CRIT void                          (ASMCALL *p_asm)(void);                                                   /*< [valid_if(:sc_type == SYSCALL_TYPE_ASM)] */
    CRIT syscall_slong_t       (SYSCALL_HANDLER *p_fast)(void);                                                  /*< [valid_if(:sc_type == SYSCALL_TYPE_FAST)] */
    CRIT void            (SYSCALL_STATE_HANDLER *p_state)(struct cpustate *__restrict state);                    /*< [valid_if(:sc_type == SYSCALL_TYPE_STATE)] */
    CRIT void            (SYSCALL_STATE_HANDLER *p_state_arg)(struct cpustate *__restrict state, void *closure); /*< [valid_if(:sc_type == SYSCALL_TYPE_STATE_ARG)] */
}                           sc_proto; };
    void                   *sc_closure;  /*< [?..?][const] Closure argument potentially passed to `p_state_arg'. */
    ATOMIC_DATA uintptr_t   sc_hits;     /*< Amount of times that this system-call was executed. */
#ifdef CONFIG_BUILDING_KERNEL_CORE
    ATOMIC_DATA uintptr_t   sc_refcnt;   /*< Reference counter. */
#else
    ATOMIC_DATA uintptr_t __sc_refcnt;   /*< Reference counter. */
#endif
    void                  (*sc_fini)(void *closure); /*< [0..1] Optional closure finalizer. */
    REF struct instance    *sc_owner;    /*< [1..1] Owner module of this system-call. */
};


/* Add/Delete a given system call descriptor.
 * @return: -EOK:    Successfully registered the given descriptor.
 * @return: -ENOMEM: Not enough available memory.
 * @return: -EPERM:  Failed to increment the reference counter of `descriptor->sc_owner'
 * @return: -EEXIST: Another descriptor has already been registered as `descriptor->sc_sysno'
 * @return: -EINVAL: The given `descriptor->sc_sysno' is used/reserved by the kernel and cannot be modified.
 *                   For a list of reserved system call numbers, see `<asm/syscallno.ci>' */
FUNDEF errno_t KCALL syscall_add(struct syscall *__restrict descriptor);
/* Delete a system call descriptor associated with `sysno'.
 * @return: -EOK:    Successfully deleted the descriptor associated with `sysno'.
 * @return: -ENXIO:  No system call was associated with the given `sysno'.
 * @return: -EINVAL: The given `sysno' is reserved by the kernel and cannot be modified.
 *                   For a list of reserved system call numbers, see `<asm/syscallno.ci>' */
FUNDEF errno_t KCALL syscall_del(register_t sysno);

/* Same as `syscall_add', but register a copy of the given `descriptor'.
 * @return: -EOK:    Successfully registered the given descriptor.
 * @return: -ENOMEM: Not enough available memory.
 * @return: -EPERM:  Failed to increment the reference counter of `descriptor->sc_owner'
 * @return: -EEXIST: Another descriptor has already been registered as `descriptor->sc_sysno'
 * @return: -EINVAL: The given `descriptor->sc_sysno' is used/reserved by the kernel and cannot be modified.
 *                   For a list of reserved system call numbers, see <asm/syscallno.ci> */
FUNDEF errno_t KCALL syscall_register(struct syscall const *__restrict descriptor);

/* Check if a given system call behaves as `norestart' */
FUNDEF bool KCALL syscall_is_norestart(register_t sysno);

#ifdef CONFIG_HAVE_SYSCALL_LONGBIT
/* Check if a given system call returns in 2 registers. */
FUNDEF bool KCALL syscall_is_long(register_t sysno);
#else /* CONFIG_HAVE_SYSCALL_LONGBIT */
#define syscall_is_long(sysno)    false
#endif /* !CONFIG_HAVE_SYSCALL_LONGBIT */

#ifdef CONFIG_BUILDING_KERNEL_CORE
#define SYSCALL_SDEFINE(name,state) __SYSCALL_SDEFINE(INTERN,_##name,state)
#define SYSCALL_DEFINE0(name)       __SYSCALL_NDEFINE(INTERN,0,_##name,(void))
#define SYSCALL_DEFINE1(name,...)   __SYSCALL_NDEFINE(INTERN,1,_##name,(__VA_ARGS__))
#define SYSCALL_DEFINE2(name,...)   __SYSCALL_NDEFINE(INTERN,2,_##name,(__VA_ARGS__))
#define SYSCALL_DEFINE3(name,...)   __SYSCALL_NDEFINE(INTERN,3,_##name,(__VA_ARGS__))
#define SYSCALL_DEFINE4(name,...)   __SYSCALL_NDEFINE(INTERN,4,_##name,(__VA_ARGS__))
#define SYSCALL_DEFINE5(name,...)   __SYSCALL_NDEFINE(INTERN,5,_##name,(__VA_ARGS__))
#define SYSCALL_DEFINE6(name,...)   __SYSCALL_NDEFINE(INTERN,6,_##name,(__VA_ARGS__))
#define SYSCALL64_DEFINE0(name)     __SYSCALL_DEFINE64(INTERN,0,_##name,(void))
#define SYSCALL64_DEFINE1(name,...) __SYSCALL_DEFINE64(INTERN,1,_##name,(__VA_ARGS__))
#define SYSCALL64_DEFINE2(name,...) __SYSCALL_DEFINE64(INTERN,2,_##name,(__VA_ARGS__))
#define SYSCALL64_DEFINE3(name,...) __SYSCALL_DEFINE64(INTERN,3,_##name,(__VA_ARGS__))
#define SYSCALL64_DEFINE4(name,...) __SYSCALL_DEFINE64(INTERN,4,_##name,(__VA_ARGS__))
#define SYSCALL64_DEFINE5(name,...) __SYSCALL_DEFINE64(INTERN,5,_##name,(__VA_ARGS__))
#define SYSCALL64_DEFINE6(name,...) __SYSCALL_DEFINE64(INTERN,6,_##name,(__VA_ARGS__))
#endif /* CONFIG_BUILDING_KERNEL_CORE */
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_SYSCALL_H */
