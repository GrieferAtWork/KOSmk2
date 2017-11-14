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
#ifndef GUARD_INCLUDE_KERNEL_INTERRUPT_H
#define GUARD_INCLUDE_KERNEL_INTERRUPT_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/list/list.h>
#include <arch/interrupt.h>
#include <errno.h>
#include <stdbool.h>
#ifdef CONFIG_SMP
#include <sched.h>
#endif

DECL_BEGIN


#define INTCALL  FCALL /* Calling convention used by interrupt callbacks. */


#define INTCODE_SEARCH  0 /* The interrupt was not handled. - Continue searching */
#define INTCODE_HANDLED 1
#define INTCODE_IGNORED 2


/* Offset from `XSP' to the iret tail in `INTTYPE_ASM_SEG' handlers. */
#ifdef __x86_64__
#define INTASM_SEG_IRET_OFFSET 16 /* u64 intno; u64 return_addr; */
#else
#define INTASM_SEG_IRET_OFFSET 16 /* u32 intno; u16 ds,es,fs,gs; u32 return_addr; */
#endif

/* Interrupt callback type codes. */

/* Basic interrupt handler types. */
#define INTTYPE_WITHARG    0x40 /*< FLAG: Pass a closure argument to the callback. */
#define INTTYPE_NOSHARE    0x80 /*< FLAG: The callback cannot be shared with other vectors and its return value is ignored. */
#define INTTYPE_MASK       0x0f /*< MASK: Basic type mask. */
#define INTTYPE_ASM        0x80 /*< Raw, low-level interrupt handler called
                                 *  directly without any register modifications:
                                 *  NOTE: This is the only interrupt callback type that
                                 *        must be exited using `ASM_IRET', rather than `ret'
                                 *  >> GLOBAL_ASM(L(my_int: ...; ASM_IRET)); */
#define INTTYPE_ASM_SEG    0x81 /*< Similar to `INTTYPE_ASM', but segment registers are
                                 *  saved and the kernel per-cpu base address is loaded.
                                 *  ADDITIONAL SETUP:
                                 *    - Spurious PIC interrupts have been discarded.
                                 *    - Segment registers are loaded (x86_64 )
                                 *  >> GLOBAL_ASM(L(my_int: ...; ret));
                                 */
#define INTTYPE_FAST       0x02 /*< Simplified C-function callback without any arguments and all clobber registers saved.
                                 *  NOTE: Implies `INTTYPE_ASM_SEG' and clear the the direction flag `EFLAGS_DF'.
                                 *  WARNING: The C-function is expected to restore any callee-preserve registers before returning.
                                 *  >> int INTCALL my_int(void) { ...; } */
#define INTTYPE_BASIC      0x03 /*< Similar to `INTTYPE_FAST', but pass basic register information.
                                 *  >> int INTCALL my_int(struct irregs_i[e] *__restrict info) { ...; }
                                 */
#define INTTYPE_STATE      0x04 /*< Push and pass a complete `cpustate'.
                                 *  NOTE: Implies `INTTYPE_FAST' behavior.
                                 *  >> int INTCALL my_int(struct cpustate_i[e] *__restrict state) { ...; } */

#define INTTYPE_FAST_ARG   (INTTYPE_WITHARG|INTTYPE_FAST)   /* Same as `INTTYPE_FAST', but also passes the closure parameter given to `int_set'
                                                             * >> int INTCALL my_int(void *closure) { ...; } */
#define INTTYPE_BASIC_ARG  (INTTYPE_WITHARG|INTTYPE_BASIC)  /* Same as `INTTYPE_BASIC', but also passes the closure parameter given to `int_set'
                                                             * >> int INTCALL my_int(struct irregs_i[e] *__restrict info, void *closure) { ...; } */
#define INTTYPE_STATE_ARG  (INTTYPE_WITHARG|INTTYPE_STATE)  /* Same as `INTTYPE_STATE', but also pass a user-defined closure.
                                                             * >> int INTCALL my_int(struct cpustate_i[e] *__restrict state, void *closure) { ...; } */
typedef u8 inttype_t; /* Interrupt callback type (One of `INTTYPE_*'). */




/* Interrupt priority (Used by callbacks that share vectors).
 *   - Higher priority handler are executed before lower priority ones.
 *   - If two or more shareable callbacks use the priority and the same vector,
 *     order of execution depends on the `INTFLAG_PRIMARY' flag when the
 *     second handler is set (If set, the second handler is executed first,
 *     otherwise it is executed after the first returned `INTCODE_SEARCH')
 * */
#define INTPRIO_MIN   (-128)
#define INTPRIO_MAX     127
#define INTPRIO_NORMAL    0 /* Normal priority (Always ZERO) */
typedef s8 intprio_t; /* Interrupt priority (One of `INTPRIO_NORMAL') */

/* Interrupt override rules (For If either EXISTING or NEW handler can't be shared; `INTTYPE_NOSHARE'):
 * EXISTING          NEW                OVERRIDE
 * INTFLAG_DYNAMIC   INTFLAG_DYNAMIC    No
 * INTFLAG_DYNAMIC   INTFLAG_PRIMARY    Yes
 * INTFLAG_DYNAMIC   INTFLAG_SECONDARY  No
 * INTFLAG_PRIMARY   INTFLAG_DYNAMIC    No
 * INTFLAG_PRIMARY   INTFLAG_PRIMARY    No
 * INTFLAG_PRIMARY   INTFLAG_SECONDARY  No
 * INTFLAG_SECONDARY INTFLAG_DYNAMIC    Yes
 * INTFLAG_SECONDARY INTFLAG_PRIMARY    Yes
 * INTFLAG_SECONDARY INTFLAG_SECONDARY  Yes
 */

#define INTFLAG_DYNAMIC   0x00 /* Dynamically move handlers of the same priority around, so
                                * that those called more often are executed before others. */
#define INTFLAG_PRIMARY   0x01 /* This is a primary interrupt handler that is (based on its priority) executed before others of the same priority.
                                * NOTE: In the event that when setting the handler, another one already exists
                                *       that execution cannot be shared with, replace the existing one (See above).
                                *      (Only happens when `INTFLAG_DYNINTNO' isn't set).
                                * HINT: An interrupt handler with this flag can never be overwritten. */
#define INTFLAG_SECONDARY 0x02 /* This is a secondary interrupt handler that is (based on its priority) executed after others of the same priority.
                                * NOTE: In the event that when setting the handler, another one already exists
                                *       that execution cannot be shared with, never replace the existing one (See above).
                                *      (Only happens when `INTFLAG_DYNINTNO' isn't set). */
#define INTFLAG_ORDERMASK 0x03 /* Mask of flag bits used to control execution order of shared handlers. */
#define INTFLAG_DYNINTNO  0x10 /* Dynamically allocate/free `i_intno' when adding/deleting the handler. */
#ifdef CONFIG_BUILDING_KERNEL_CORE
#define INTFLAG_FREEDESCR 0x80 /* Call `kfree()' to delete the `struct interrupt' handler when deleting it.
                                * NOTE: This flag is not allowed for public use and is internally set when `int_register()' is called. */
#endif
#define INTFLAG_NORMAL    INTFLAG_DYNAMIC /* Default flags for registering an interrupt handler. */
typedef u8 intflag_t; /* Interrupt handler flags (Set of `INTFLAG_*'). */


#define INTERRUPT_OFFSETOF_INTNO          0
#define INTERRUPT_OFFSETOF_MODE           1
#define INTERRUPT_OFFSETOF_TYPE           2
#define INTERRUPT_OFFSETOF_PRIO           3
#define INTERRUPT_OFFSETOF_FLAGS          4
#define INTERRUPT_OFFSETOF_CALLBACK       8
#define INTERRUPT_OFFSETOF_CLOSURE       (8+__SIZEOF_POINTER__)
#define INTERRUPT_OFFSETOF_OWNER         (8+2*__SIZEOF_POINTER__)
#define INTERRUPT_OFFSETOF_FINI          (8+3*__SIZEOF_POINTER__)
#define INTERRUPT_OFFSETOF_HITS          (8+4*__SIZEOF_POINTER__)
#define INTERRUPT_OFFSETOF_MISS         (16+4*__SIZEOF_POINTER__)
#define INTERRUPT_OFFSETOF_LINK         (16+5*__SIZEOF_POINTER__)
#define INTERRUPT_SIZE                  (16+7*__SIZEOF_POINTER__)

struct irregs_i;
struct irregs_ie;
struct cpustate_i;
struct cpustate_ie;

/* Returns true if `a' should be executed before `b' */
#define INTERRUPT_BEFORE(a,b) \
  ((a)->i_prio >  (b)->i_prio || \
  ((a)->i_prio == (b)->i_prio && \
  ((a)->i_flags&INTFLAG_PRIMARY || \
   (b)->i_flags&INTFLAG_SECONDARY || \
  (!((a)->i_flags&INTFLAG_SECONDARY) && \
   !((b)->i_flags&INTFLAG_PRIMARY) && \
     (a)->i_hits >= (b)->i_hits))))

struct PACKED interrupt {
    irq_t                i_intno;       /*< [const][owned_if(IS_LINKED && INTFLAG_DYNINTNO)]
                                         *   Interrupt vector number used by this handler. */
    intmode_t            i_mode;        /*< [const] Interrupt mode (One of `INTMODE_*') */
    inttype_t            i_type;        /*< [const] Interrupt callback type (One of `INTTYPE_*'). */
    intprio_t            i_prio;        /*< [const] Interrupt priority (One of `INTPRIO_*') */
    intflag_t            i_flags;       /*< [const] Interrupt handler flags (Set of `INTFLAG_*'). */
#ifdef __x86_64__
    u8                   i_ist;         /*< [const] Alternative interrupt execution stack. (Set to ZERO to use the primary stack)
                                         *   NOTE:  Only used if all interrupt handlers of any given vector use the same
                                         *          IST, which can easily be achieved by installing a non-sharable handler. */
    u8                 __i_pad[2];      /* Pad up to an offset of 8. */
#else
    u8                 __i_pad[3];      /* Pad up to an offset of 8. */
#endif
union PACKED {
    void                *i_callback;    /*< [1..1][const] Interrupt handler entry point. (exact prototype depends on `i_type') */
union PACKED {
    void       (ASMCALL *p_asm)(void);     /*< [valid_if(:i_type == INTTYPE_ASM)] Returns using `ASM_IRET'. */
    void       (ASMCALL *p_asm_seg)(void); /*< [valid_if(:i_type == INTTYPE_ASM_SEG)] Returns using `ret'. */
    int        (INTCALL *p_fast)(void);                                                               /*< [valid_if(:i_type == INTTYPE_FAST)]. */
    int        (INTCALL *p_fast_arg)(void *closure);                                                  /*< [valid_if(:i_type == INTTYPE_FAST_ARG)]. */
    int        (INTCALL *p_basic)(struct irregs_i *__restrict info);                                  /*< [valid_if(:i_type == INTTYPE_BASIC)]. */
    int        (INTCALL *p_basic_arg)(struct irregs_i *__restrict info, void *closure);               /*< [valid_if(:i_type == INTTYPE_BASIC_ARG)]. */
    int        (INTCALL *p_baseexcept)(struct irregs_ie *__restrict info);                            /*< [valid_if(:i_type == INTTYPE_BASIC)]. */
    int        (INTCALL *p_baseexcept_arg)(struct irregs_ie *__restrict info, void *closure);         /*< [valid_if(:i_type == INTTYPE_BASIC_ARG)]. */
    int        (INTCALL *p_state)(struct cpustate_i *__restrict state);                               /*< [valid_if(:i_type == INTTYPE_STATE)]. */
    int        (INTCALL *p_state_arg)(struct cpustate_i *__restrict state, void *closure);            /*< [valid_if(:i_type == INTTYPE_STATE_ARG)]. */
    int        (INTCALL *p_except)(struct cpustate_ie *__restrict state);                             /*< [valid_if(:i_type == INTTYPE_STATE)]. */
    int        (INTCALL *p_except_arg)(struct cpustate_ie *__restrict state, void *closure);          /*< [valid_if(:i_type == INTTYPE_STATE_ARG)]. */
    void       (INTCALL *p_noshare_fast)(void);                                                       /*< [valid_if(:i_type == INTTYPE_NOSHARE|INTTYPE_FAST)]. */
    void       (INTCALL *p_noshare_fast_arg)(void *closure);                                          /*< [valid_if(:i_type == INTTYPE_NOSHARE|INTTYPE_FAST_ARG)]. */
    void       (INTCALL *p_noshare_basic)(struct irregs_i *__restrict info);                          /*< [valid_if(:i_type == INTTYPE_NOSHARE|INTTYPE_BASIC)]. */
    void       (INTCALL *p_noshare_basic_arg)(struct irregs_i *__restrict info, void *closure);       /*< [valid_if(:i_type == INTTYPE_NOSHARE|INTTYPE_BASIC_ARG)]. */
    void       (INTCALL *p_noshare_baseexcept)(struct irregs_ie *__restrict info);                    /*< [valid_if(:i_type == INTTYPE_NOSHARE|INTTYPE_BASIC)]. */
    void       (INTCALL *p_noshare_baseexcept_arg)(struct irregs_ie *__restrict info, void *closure); /*< [valid_if(:i_type == INTTYPE_NOSHARE|INTTYPE_BASIC_ARG)]. */
    void       (INTCALL *p_noshare_state)(struct cpustate_i *__restrict state);                       /*< [valid_if(:i_type == INTTYPE_NOSHARE|INTTYPE_STATE)]. */
    void       (INTCALL *p_noshare_state_arg)(struct cpustate_i *__restrict state, void *closure);    /*< [valid_if(:i_type == INTTYPE_NOSHARE|INTTYPE_STATE_ARG)]. */
    void       (INTCALL *p_noshare_except)(struct cpustate_ie *__restrict state);                     /*< [valid_if(:i_type == INTTYPE_NOSHARE|INTTYPE_STATE)]. */
    void       (INTCALL *p_noshare_except_arg)(struct cpustate_ie *__restrict state, void *closure);  /*< [valid_if(:i_type == INTTYPE_NOSHARE|INTTYPE_STATE_ARG)]. */
}                        i_proto; };    /*< Callback prototypes. */
    void                *i_closure;     /*< [valid_if(i_type&INTTYPE_WITHARG)] Closure argument passed to `i_callback' */
    REF struct instance *i_owner;       /*< [1..1] Owner of this interrupt handler. */
    /* [0..1] Optional callback that is executed after the interrupt handler is deleted. */
    void               (*i_fini)(struct interrupt *__restrict entry);
    ATOMIC_DATA u64      i_hits;        /*< [valid_if(!INTTYPE_NOSHARE)] Amount of times an interrupt was handled by this callback. (Used by `INTFLAG_DYNAMIC') */
#if __SIZEOF_POINTER__ >= 8
    ATOMIC_DATA u64      i_miss;        /*< [valid_if(!INTTYPE_NOSHARE)] Amount of times an interrupt could not be processed by this handler. */
#else
union PACKED{uintptr_t __i_align;       /* Align by pointers. */
    ATOMIC_DATA u32      i_miss;        /*< [valid_if(!INTTYPE_NOSHARE)] Amount of times an interrupt could not be processed by this handler. */};
#endif
    LIST_NODE(struct interrupt)
                         i_link;        /*< [0..1][lock(PREEMPTION_DISABLED && (CONFIG_SMP ? INTERNAL(...) : NONE))]
                                         *  Chain of shared interrupt handlers. (HINT: `NULL' if this handler cannot be shared) */
};

/* Register/delete an interrupt handler.
 * NOTE: Interrupt handlers are automatically deleted when their module is unloaded.
 * NOTE: Assembly-level interrupt handlers (such as `INTTYPE_ASM' and `INTTYPE_ASM_SEG')
 *       must always be `INTPRIO_MAX' + `INTFLAG_PRIMARY', so-as to ensure that they
 *       are execute before all others.
 *       This behavior is asserted internally.
 * @param: affinity: Set of CPUs on which the interrupt should be executed.
 *             HINT: If this set is empty or `int_add()' is called, or `CONFIG_SMP'
 *                   is disabled, the vector is only registered in the calling CPU. 
 * @return: * :      The actual amount of CPUs that the handler was registered on.
 * @return: -EPERM:  Failed to increment the reference counter of `entry->i_owner'
 * @return: -ENOMEM: Failed to dynamically reserve/re-use an interrupt vector.
 * @return: -EEXIST: `entry' is a non-primary, non-shareable and non-dynamic
 *                   interrupt handler, but the specified vector is already
 *                   in use on at least one target CPU.
 * @return: -ESRCH: [CONFIG_SMP] `affinity' does not contain any existent CPU.
 * @return: -ECOMM: [CONFIG_SMP] Failed to communicate with at least one of the CPUs contained in `affinity' */
FUNDEF ssize_t KCALL int_add(struct interrupt *__restrict entry);
#ifndef CONFIG_SMP
#define int_addset(entry,affinity) int_add(entry,affinity)
#define int_addall(entry)          int_add(entry)
#define int_addboot(entry)         int_add(entry)
#else
FUNDEF ssize_t KCALL int_addset(struct interrupt *__restrict entry, __cpu_set_t *__restrict affinity);
LOCAL ssize_t KCALL int_addall(struct interrupt *__restrict entry) {
 /* Same as `int_addset' with a fully affinity. */
 __cpu_set_t affinity;
 __CPU_FILL_S(sizeof(__cpu_set_t),&affinity);
 return int_addset(entry,&affinity);
}
LOCAL ssize_t KCALL int_addboot(struct interrupt *__restrict entry) {
 /* Same as `int_addset' with a fully affinity. */
 __cpu_set_t affinity;
 __CPU_ZERO_S(sizeof(__cpu_set_t),&affinity);
 __CPU_SET_S(0,sizeof(__cpu_set_t),&affinity);
 return int_addset(entry,&affinity);
}
#endif

/* Delete an interrupt handler.
 * @return: * :      The actual amount of CPUs that the handler was deleted from.
 * @return: -ENXIO:  The handler was never added.
 * @return: -ECOMM: [CONFIG_SMP] Failed to communicate with at least one of the CPUs contained in `affinity' */
FUNDEF ssize_t KCALL int_del(struct interrupt *__restrict entry);

/* Same as `int_add', but register a heap-allocated copy of `entry'
 * that is automatically free'd when the handler is deleted.
 * WARNING: The registered interrupt handler cannot be deleted thereafter,
 *          but is still automatically deleted when the owner module is deleted.
 * WARNING: If a custom callback is assigned to `i_fini', that callback
 *          will be passed a pointer to the internal copy of `entry'.
 * @return: * :      The interrupt vector number under which the handler was registered.
 * @return: -EPERM:  Failed to increment the reference counter of `entry->i_owner'
 * @return: -ENOMEM: Failed to dynamically reserve/re-use an interrupt vector.
 * @return: -ENOMEM: Failed to allocate memory for the copy of `entry'
 * @return: -EEXIST: `entry' is a non-primary, non-shareable and non-dynamic
 *                   interrupt handler, but the specified vector is already
 *                   in use on at least one target CPU.
 * @return: -ESRCH: [CONFIG_SMP] `affinity' does not contain any existent CPU.
 * @return: -ECOMM: [CONFIG_SMP] Failed to communicate with at least one of the CPUs contained in `entry->i_affinity' */
FUNDEF int KCALL int_register(struct interrupt const *__restrict entry);
#ifndef CONFIG_SMP
#define int_register_set(entry,affinity) int_add(entry,affinity)
#define int_register_all(entry)          int_add(entry)
#else
FUNDEF int KCALL int_register_set(struct interrupt const *__restrict entry, __cpu_set_t *__restrict affinity);
LOCAL int KCALL int_register_all(struct interrupt const *__restrict entry) {
 /* Same as `int_addset' with a fully affinity. */
 __cpu_set_t affinity;
 __CPU_FILL_S(sizeof(__cpu_set_t),&affinity);
 return int_register_set(entry,&affinity);
}
#endif


#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Initialize default kernel interrupts and load the default IDT table. */
INTDEF INITCALL void KCALL interrupt_initialize(void);
#endif

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_INTERRUPT_H */
