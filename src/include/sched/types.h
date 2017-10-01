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
#ifndef GUARD_INCLUDE_SCHED_TYPES_H
#define GUARD_INCLUDE_SCHED_TYPES_H 1

#include <bits/sigset.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/sync/atomic-rwptr.h>
#include <hybrid/timespec.h>
#include <hybrid/types.h>
#include <kernel/arch/cpu.h>
#include <kernel/arch/task.h>
#include <kernel/memory.h>
#include <sched.h>
#include <sync/sig.h>
#ifdef __CC__
#include <hybrid/list/list.h>
#include <hybrid/list/ring.h>
#endif /* __CC__ */

DECL_BEGIN

#ifdef __CC__
struct cpu;
struct sig;
struct cpustate;
struct mman;
struct stack;
struct sigqueue;
#endif /* __CC__ */

#define TASKSIGSLOT_OFFSETOF_SELF    0
#define TASKSIGSLOT_OFFSETOF_SIG       __SIZEOF_POINTER__
#define TASKSIGSLOT_OFFSETOF_CHAIN  (2*__SIZEOF_POINTER__)
#define TASKSIGSLOT_OFFSETOF_BUF    (4*__SIZEOF_POINTER__)
#define TASKSIGSLOT_OFFSETOF_SIZ    (5*__SIZEOF_POINTER__)
#define TASKSIGSLOT_OFFSETOF_LAST   (5*__SIZEOF_POINTER__+__SIZEOF_SIZE_T__)
#define _TASKSIGSLOT_SIZE           (6*__SIZEOF_POINTER__+__SIZEOF_SIZE_T__)
#ifdef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
#define TASKSIGSLOT_SIZE           ((_TASKSIGSLOT_SIZE+(ATOMIC_RWPTR_ALIGN-1)) & ~(ATOMIC_RWPTR_ALIGN-1))
#else
#define TASKSIGSLOT_SIZE             _TASKSIGSLOT_SIZE
#endif

#ifdef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
/* Task signal slots can and are used as atomic R/W pointers in some places,
 * meaning that any allocated task must aligned by 'TASK_ALIGNMENT' */
#define TASKSIGSLOT_ALIGN    ATOMIC_RWPTR_ALIGN
#else
#define TASKSIGSLOT_ALIGN    __SIZEOF_POINTER__
#endif


#ifdef __CC__
ATTR_ALIGNED(TASKSIGSLOT_ALIGN)
struct tasksigslot {
 struct task        *tss_self; /*< [1..1][const][== :.:this] Task-self pointer. */
 struct sig         *tss_sig;  /*< [0..1][lock(PRIVATE(THIS_TASK))] When non-NULL, pointer to the signal this slot is receiving. */
 LIST_NODE(struct tasksigslot)
                     tss_chain;/*< [0..1][lock(tss_sig->s_lock)][valid_if(tss_sig != NULL)]
                                *   Chain of signal slots that where the next was activated after this one. */
 USER void          *tss_buf;  /*< [0..tss_siz][const][valid_if(tss_sig != NULL)] An optional buffer into which received data is written. */
 size_t              tss_siz;  /*< [const][valid_if(tss_sig != NULL)] The size of 'tss_buf' in bytes. */
 struct tasksigslot *tss_last; /*< [1..1][lock(tss_sig->s_lock)][->tss_chain.le_next == NULL]
                                *  [valid_if(tss_sig != NULL && tss_sig->s_task == this)]
                                *   Pointer to the signal slot of a different task that was added last.
                                *   This field is used to invert re-scheduling order during signal
                                *   broadcasting, in order to improve the first-come-first-serve
                                *   principle by always appending new signal slots at the end of
                                *   the chain of signal slots.
                                *   Technically, this pointer should be placed in 'struct sig',
                                *   creating a double-ended, singly-linked list there, but
                                *   since that structure must exist in _a_ _lot_ of places, it
                                *   is much more efficient to place the back-end of said list
                                *   here and only keep its front in 'struct sig'.
                                *   >> FIRST_RECV = sig->s_task;
                                *   >> LAST_RECV  = sig->s_task ? sig->s_task->tss_last : NULL;
                                *   >> ADD_RECV(x) {
                                *   >>     x->tss_next = NULL;
                                *   >>     if (!sig->s_task) {
                                *   >>         sig->s_task = x;
                                *   >>         x->tss_last = x;
                                *   >>     } else {
                                *   >>#ifdef CONFIG_DEBUG
                                *   >>         x->tss_last = NULL;
                                *   >>#endif
                                *   >>         CHECK_HOST_DOBJ(sig->s_task);
                                *   >>         CHECK_HOST_DOBJ(sig->s_task->tss_last);
                                *   >>         sig->s_task->tss_last->tss_next = x;
                                *   >>         sig->s_task->tss_last = x;
                                *   >>     }
                                *   >> }
                                */
#if TASKSIGSLOT_SIZE != _TASKSIGSLOT_SIZE
 /* Make sure this structure is properly aligned. */
 byte_t tss_padding[TASKSIGSLOT_SIZE-_TASKSIGSLOT_SIZE];
#endif
};
#endif /* __CC__ */

#define TASKSIG_OFFSETOF_FIRST   0
#define TASKSIG_OFFSETOF_SLOTC   TASKSIGSLOT_SIZE
#define TASKSIG_OFFSETOF_SLOTA  (TASKSIGSLOT_SIZE+__SIZEOF_SIZE_T__)
#define TASKSIG_OFFSETOF_SLOTV  (TASKSIGSLOT_SIZE+2*__SIZEOF_SIZE_T__)
#define TASKSIG_OFFSETOF_RECV   (TASKSIGSLOT_SIZE+2*__SIZEOF_SIZE_T__+__SIZEOF_POINTER__)
#define TASKSIG_SIZE            (TASKSIGSLOT_SIZE+2*__SIZEOF_SIZE_T__+2*__SIZEOF_POINTER__)
#ifdef __CC__
struct tasksig {
 struct tasksigslot  ts_first; /*< A single task slot behaving identical to 'ts_slotv',
                                *  that is used to prevent allocation errors/overhead when
                                *  only receiving a single signal (as is the case most often)
                                *  NOTE: The 'tss_self' field of this is _always_ pre-initialized! */
 size_t              ts_slotc; /*< [lock(THIS_TASK)] Amount of signal slots currently in use (excluding 'ts_first'). */
 size_t              ts_slota; /*< [lock(THIS_TASK)] Allocated amount of signal slots */
 struct tasksigslot *ts_slotv; /*< [lock(THIS_TASK)][0..ts_slotc|alloc(ts_slota)][owned] Vector of additional signal slots. */
#ifdef __INTELLISENSE__
             struct sig *ts_recv; /*< [0..1] The first signal that was received and caused the thread to wake up. */
#else
 ATOMIC_DATA struct sig *ts_recv; /*< [0..1] The first signal that was received and caused the thread to wake up. */
#endif
};
#endif /* __CC__ */

#define HSTACK_OFFSETOF_BEGIN  0
#define HSTACK_OFFSETOF_END    __SIZEOF_POINTER__
#define HSTACK_SIZE           (__SIZEOF_POINTER__*2)
#ifdef __CC__
struct hstack {
 /* Controller for host (kernel) stacks. (mapped in '&mman_kernel')
  * NOTE: The kernel uses this simplified version of a stack, since it is
  *       impossible to create stacks using guard pages in kernel-space,
  *       as the pagefault that accessing such a page causes also requires some
  *       stack (So we'd end up needing a 3rd stack for handling guard pages in kernel-space).
  *    >> Additional, it is impossible to specify a custom stack that the CPU
  *       should switch to before handling the page fault, since the TSS structure
  *       that is used for specifying a custom stack when entering ring#0 from
  *       somewhere different cannot be configured to use a custom stack when
  *       the privilege level doesn't change (such as is the case when handling
  *       interrupts while already inside the kernel)
  * With that in mind, the best we can do for kernel-level stacks, is allocated
  * a fixed-length section of locked, virtual (shared) memory above 3GB and just
  * hope that we never actually overflow it (because it's physically impossible to
  * create any sort of protection that would fault again to end up in a triple fault)
  * WARNING: Kernel stacks must be allocated within 'mman_kernel' and have
  *          to be locked + tagged with the address of the associated thread
  *         (When calling 'mman_mmap_unlocked()', the address of the thread
  *          must be passed as argument for 'closure', which is later used as
  *          key for ensuring safe unmapping of the stack!)
  */
 ppage_t hs_begin; /*< [const][<= hs_end] First usable stack page. */
 ppage_t hs_end;   /*< [const][>= hs_begin] First unusable stack page
                    *  (After a privilege switch, this is the initial ESP value). */
};
#endif /* __CC__ */

#define TASKFLAG_NONE        0x0000
#define TASKFLAG_SUSP_TIMED  0x0002 /*< Set if a task was suspended from 'TASKMODE_SLEEPING' using 'task_suspend()'.
                                     *  When set, 'task_resume()' must re-schedule the task as sleeping. */
#define TASKFLAG_WILLTERM    0x0004 /*< The task currently resides within a critical section,
                                     *  but will terminate as soon as that section is left. */
#define TASKFLAG_TIMEDOUT    0x0008 /*< The task was awoken, because it timed out (NOTE: Only THIS_TASK may remove this flag once set). */
#define TASKFLAG_INTERRUPT   0x0010 /*< The task was awoken due to an interrupt (NOTE: Only THIS_TASK may remove this flag once set). */
#define TASKFLAG_NOTALEADER  0x4000 /*< [const] The task cannot be used as a thread/process group leader. */
#define TASKFLAG_SIGSTOPCONT 0x8000 /*< [lock(t_cpu->c_lock)] The task has been stopped or continued (NOTE: Not set by forced suspend/resume). */
#ifdef __CC__
typedef u16 taskflag_t; /*< Task flags (Set of 'TASKFLAG_*'). */
typedef u8 taskmode_t;  /*< Task mode (One of 'TASKMODE_*'). */
#endif /* __CC__ */

#define TASKMODE_RUNNING    0x00 /*< [PRIVATE(THIS_CPU)] The task is running (may or may not be in the foreground).
                                  *   NOTE: Only 'TASK_CPU(self)' may enter/leave 'TASKMODE_RUNNING'-mode */
#define TASKMODE_SLEEPING   0x01 /*< The task is sleeping, but will re-awake once a timeout expires. */
#define TASKMODE_SUSPENDED  0x02 /*< The task is suspended and is never supplied with a quantum. */
#define TASKMODE_WAKEUP     0x03 /*< Similar to 'TASKMODE_SLEEPING' when a timeout has expired, but doesn't set the timed-out task flag. */
#define TASKMODE_NOTSTARTED 0xfe /*< The thread hasn't started yet. (Some components of the task may be in a less-consistent state; treat the task as terminated) */
#define TASKMODE_TERMINATED 0xff /*< The thread has terminated (This state cannot be altered, or reverted) */

/* IDLE semantics: Never execute the task, unless no other non-idle
 *                 task and no idle task with a greater priority exists.
 * >> TASK #1: TASKPRIO_DEFAULT
 * >> TASK #2: TASKPRIO_DEFAULT
 * >> TASK #3: TASKPRIO_MAXIDLE
 * >> TASK #4: TASKPRIO_MINIDLE
 * EXECUTION ORDER: [#1 <> #2], #4, #3
 * IDLE tasks are ~parked~ when at least one non-idle task is discovered.
 * The IDLE task itself can choose to delay getting parked by either:
 *   - Disabling interrupts (The IDLE task will be parked during the next PIT-IRQ)
 *   - Switching to critical-mode (critical tasks will not be parked until they're no longer that)
 *  -> Essentially, anything that is 'SAFE' cannot be parked no matter what it's priority is
 * WARNING: Even though 'SAFE' tasks cannot be parked doesn't mean they can't be pre-empted.
 * And even worse: priority work just like normal, meaning it may take quite
 *                 a while before the SAFE, IDLE task gets another quantum.
 * HINT: You can always prolong your quantum by disabling interrupts, and while never
 *       truly recommended, there may be certain situations that could benefit from
 *       not being pre-empted (especially short operations requiring an atomic r/w-lock.)
 */
#define TASKPRIO_MINIDLE 0x00 /*< Lowest priority for IDLE-semantics */
#define TASKPRIO_MAXIDLE 0x7f /*< Highest priority for IDLE-semantics */
#define TASKPRIO_ISIDLE(x) (!((x)&0x80))
/* Not the following situation, explaining why idle
 * tasks may still sometimes run alongside non-idle ones:
 *   - thread #1: Suspending; Waiting for 'TASK_OFFSETOF_EVENT'
 *   - thread #2: Is terminating itself
 *   - thread #3: The IDLE thread
 *  #1: thread #2 removes itself from the CPU (thread #3 is set as running)
 *  #2: The CPU switches context to thread #3
 *  #3: thread #3 broadcasts 'TASK_OFFSETOF_EVENT'
 *  #4: thread #1 is re-added as running, causing thread #3 to be parked.
 *   >> 'sig_send()' returns with an invalid THIS_THREAD set!
 *  How do we even solve this?
 *   -> We need to relax the whole never-executing IDLE rule:
 *      Tasks can only be parked when they're not critical
 *      and don't have the interrupt flag set!
 */


#define TASKPRIO_MIN     0x80 /*< Lowest priority. */
#define TASKPRIO_DEFAULT 0xbf /*< Default priority. */
#define TASKPRIO_MAX     0xff /*< Greatest priority. */
#ifdef __CC__
typedef u8 taskprio_t;
#endif /* __CC__ */

/* Kernel interrupt handler chain.
 * - Handlers defined through this chain are executed
 *   whenever an unknown interrupt occurs while in kernel-space.
 * - Using this, local exception handling can be implemented,
 *   such as premature termination upon faulting when accessing
 *   a user-space pointer.
 * - Execution of such a handler occurs as follows:
 *   >> struct cpustate state;
 *   >> struct intchain *ic = THIS_TASK->t_ic;
 *   >> for (; ic; ic = ic->ic_prev) {
 *   >>    if (!SHOULD_EXECUTE_IC(ic)) continue; // Search for an IC that should be executed.
 *   >>    state           = GET_EXCEPTION_CPUSTATE();
 *   >>    THIS_TASK->t_ic = ic->ic_prev; // Restore the previous IC
 *   >>    state.host.esp  = (uintptr_t)ic; // Use the address of the IC itself as value for ESP
 *   >>    state.host.eip  = (uintptr_t)ic->ic_int;
 *   >>    return EXECUTE_CPU_STATE(&state);
 *   >> }
 *   >> UNHANDLED_EXCEPTION(GET_EXCEPTION_CPUSTATE());
 * - Mainly designed for use from assembly:
 *   >>
 *   >>    # Create a new INTCHAIN link.
 *   >>    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %eax
 *   >>    pushl $handler
 *   >>    pushl $(EXC_PAGE_FAULT)
 *   >>    pushl TASK_OFFSETOF_IC(%eax)
 *   >>    movl  %esp, TASK_OFFSETOF_IC(%eax)
 *   >> 
 *   >>    # Potentially dangerous code that wants
 *   >>    # to handle 'EXC_PAGE_FAULT' locally
 *   >>    ...
 *   >> 
 *   >>    # Cleanup if no error occurred.
 *   >>    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %eax
 *   >>    popl  TASK_OFFSETOF_IC(%eax)
 *   >>    addl  $8, %esp
 *   >>    
 *   >>handler:
 *   >>    # Code that is executed if an interrupt did occurr
 *   >>    ...
 */
#ifdef __CC__
struct intchain {
 HOST struct intchain *ic_prev;       /*< [0..1] Previous interrupt chain entry. */
 /* HINT: The following 3 can be set at once as a dword. */
 irq_t                 ic_irq;        /*< Interrupt number that is handled by this chain entry. */
 u8                    ic_opt;        /*< Interrupt options (Set of 'INTCHAIN_OPT_*') */
 u16                   ic_padding;    /* ... */
 void        (ASMCALL *ic_int)(void); /*< [1..1] Temporary interrupt handler. */
};
#endif /* __CC__ */

#define INTCHAIN_OPT_IRQ 0x00 /*< Execute the handler when 'ic_irq' is triggered. */
#define INTCHAIN_OPT_EXC 0x01 /*< Ignore 'ic_irq' and handle all exceptions. */
#define INTCHAIN_OPT_PIC 0x02 /*< Ignore 'ic_irq' and handle all PIC interrupts. */
#define INTCHAIN_OPT_USR 0x04 /*< Ignore 'ic_irq' and handle all user interrupts. */
#define INTCHAIN_OPT_ALL 0x07 /*< Ignore 'ic_irq' and handle all interrupts. */

#ifdef __CC__
struct ccpustate;
/* Trigger & execute a local interrupt.
 * NOTE: When executed, this function will not return.
 * @param: pchain: The pointer to an intchain head. - When a handler is
 *                 found, update this pointer with the next older handler.
 * @param: cstate: CPU state used for loading general purpose registers
 *                 before execution ('GET_EXCEPTION_CPUSTATE'). */
FUNDEF void KCALL intchain_trigger(struct intchain **__restrict pchain, irq_t irq,
                                   struct ccpustate const *__restrict cstate, u32 eflags);
#endif /* __CC__ */














#ifdef __CC__
typedef int pidtype_t;
#endif
#define PIDTYPE_GPID  0 /*< Global thread-id. (PID number within the init-pid namespace) */
#define PIDTYPE_PID   1 /*< Local thread-id. (returned by 'gettid()'; Value of 'tp_leader' returned by 'getpid()') */
#define PIDTYPE_COUNT 2

#define THREAD_LINK_OFFSETOF_PID      0
#define THREAD_LINK_OFFSETOF_LINK     __SIZEOF_PID_T__
#define THREAD_LINK_OFFSETOF_NS      (__SIZEOF_PID_T__+2*__SIZEOF_POINTER__)
#define THREAD_LINK_SIZE             (__SIZEOF_PID_T__+3*__SIZEOF_POINTER__)

#define THREAD_PID_OFFSETOF_PARLOCK      0
#define THREAD_PID_OFFSETOF_PARENT       ATOMIC_RWLOCK_SIZE
#define THREAD_PID_OFFSETOF_LEADLOCK    (__SIZEOF_POINTER__+ATOMIC_RWLOCK_SIZE)
#define THREAD_PID_OFFSETOF_LEADER      (__SIZEOF_POINTER__+2*ATOMIC_RWLOCK_SIZE)
#define THREAD_PID_OFFSETOF_ID        (2*__SIZEOF_POINTER__+2*ATOMIC_RWLOCK_SIZE)
#define THREAD_PID_OFFSETOF_CHILDLOCK (2*__SIZEOF_POINTER__+2*ATOMIC_RWLOCK_SIZE+PIDTYPE_COUNT*THREAD_LINK_SIZE)
#define THREAD_PID_OFFSETOF_CHILDREN  (2*__SIZEOF_POINTER__+3*ATOMIC_RWLOCK_SIZE+PIDTYPE_COUNT*THREAD_LINK_SIZE)
#define THREAD_PID_OFFSETOF_SIBLINGS  (3*__SIZEOF_POINTER__+3*ATOMIC_RWLOCK_SIZE+PIDTYPE_COUNT*THREAD_LINK_SIZE)
#define THREAD_PID_OFFSETOF_GROUPLOCK (5*__SIZEOF_POINTER__+3*ATOMIC_RWLOCK_SIZE+PIDTYPE_COUNT*THREAD_LINK_SIZE)
#define THREAD_PID_OFFSETOF_GROUP     (5*__SIZEOF_POINTER__+4*ATOMIC_RWLOCK_SIZE+PIDTYPE_COUNT*THREAD_LINK_SIZE)
#define THREAD_PID_OFFSETOF_GROUPLINK (6*__SIZEOF_POINTER__+4*ATOMIC_RWLOCK_SIZE+PIDTYPE_COUNT*THREAD_LINK_SIZE)
#define THREAD_PID_SIZE               (8*__SIZEOF_POINTER__+4*ATOMIC_RWLOCK_SIZE+PIDTYPE_COUNT*THREAD_LINK_SIZE)
#ifdef __CC__
struct thread_link {
 pid_t                       tl_pid;  /*< Global/Local thread-id. */
 WEAK LIST_NODE(struct task) tl_link; /*< [0..1][lock(tp_ns->pn_lock)][valid_if(tl_ns != NULL)] PID namespace hash-map entry within 'tl_ns'. */
 REF struct pid_namespace   *tl_ns;   /*< [0..1][lock(WRITE(:t_mode == TASKMODE_TERMINATED))] The pid namespace used to map 'tl_pid'.
                                       *   NOTE: When 'CLONE_NEWPID' is set during 'clone()' or 'fork()' is
                                       *         used, a new PID namespace is created for 'PIDTYPE_PID'.
                                       *   NOTE: 'tl_ns->pn_type == INDEXOF(self,:tp_ids[:])'
                                       *   NOTE: This reference is still valid, even when ':t_refcnt == 0' */
};

struct thread_pid {
 /* HINT: 'inittask', as well as CPU-idle tasks are their own parent & leader. */
 atomic_rwlock_t             tp_parlock;   /*< Lock for accessing 'tp_parent' */
 REF struct task            *tp_parent;    /*< [lock(tp_parlock)][1..1][REF_IF(!= :self)][valid_if(:t_mode != TASKMODE_NOTSTARTED)]
                                            *   Parent process (Of which we are a child) and receiver of 'SIGCHLD' signals.
                                            *   NOTE: The parent process if 'fork()' was used, or 'CLONE_PARENT' wasn't
                                            *         set during 'clone()'. - Otherwise '== <Parent process>->t_pid.tp_parent'.
                                            *   NOTE: This reference is still valid, even when ':t_refcnt == 0'  */
 atomic_rwlock_t             tp_leadlock;  /*< Lock for accessing 'tp_leader' */
 REF struct task            *tp_leader;    /*< [lock(tp_leadlock)][1..1][REF_IF(!= :self)] Process leader.
                                            *   NOTE: == 'tp_parent->t_pid.tp_leader', unless 'CLONE_THREAD' wasn't set
                                            *         during 'clone()' or if 'fork()' was used. - Otherwise '== :self'.
                                            *   NOTE: Only a reference when '!= :self' */
 struct thread_link tp_ids[PIDTYPE_COUNT]; /*< [const] Array of process IDs (Index is one of 'PIDTYPE_*'). */
 /* Access to all of this task's children. (This is where zombies life) */
 atomic_rwlock_t             tp_childlock; /*< Lock for the child/sibling chain. */
 WEAK REF LIST_HEAD(struct task) tp_children; /*< [0..1][lock(tp_childlock)][KEY(t_pid.tp_siblings)]
                                            *  [[*]->t_pid.tp_parent == :self] List of child tasks.
                                            *   NOTE: Children remove themself from this list during
                                            *         free(), meaning that list elements are weakly linked.
                                            *    YES: This chain of tasks remains valid even after 't_refcnt' drops to ZERO(0).
                                            *      >> This is done in order to delete any remaining zombie tasks
                                            *         once a parent task itself is terminated, thus preventing
                                            *   NOTE: This is the list of tasks that will send 'SIGCHLD' to ':self'. */
 WEAK REF LIST_NODE(struct task) tp_siblings; /*< [0..1][lock(tp_parent->tp_childlock)] Linked list of sibling tasks.
                                            *   NOTE: This chain is still valid, even when ':t_refcnt == 0' */
 /* Access to all tasks within the group of a task-group leader. */
 atomic_rwlock_t             tp_grouplock; /*< Lock for the group task chain. */
 WEAK LIST_HEAD(struct task) tp_group;     /*< [0..1][lock(tp_grouplock)][KEY(t_pid.tp_grplink)]
                                            *  [[*]->t_pid.tp_leader == :self] List of child tasks.
                                            *   NOTE: Children remove themself from this list during
                                            *         destruction, meaning that list elements are weakly linked.
                                            *   NOTE: This is the list of tasks that will send 'SIGCHLD' to ':self'. */
 WEAK LIST_NODE(struct task) tp_grplink;   /*< [0..1][lock(tp_leader->tp_grouplock)] Linked list of grouped tasks. */
};

/* Add/Remove a given thread from the child/group list.
 * NOTE: The caller is responsible for holding a write-lock
 *       to 'tp_childlock' or 'tp_grouplock respectively. */
#define THREAD_PID_ADDCHILD(self,thread)  LIST_INSERT((self)->tp_children,thread,t_pid.tp_siblings)
#define THREAD_PID_DELCHILD(self,thread) (LIST_REMOVE(thread,t_pid.tp_siblings),LIST_MKUNBOUND(thread,t_pid.tp_siblings))
#define THREAD_PID_ISCHILD(self,thread) (!LIST_ISUNBOUND(thread,t_pid.tp_siblings))
#define THREAD_PID_ADDGROUP(self,thread)  LIST_INSERT((self)->tp_group,thread,t_pid.tp_grplink)
#define THREAD_PID_DELGROUP(self,thread) (LIST_REMOVE(thread,t_pid.tp_grplink),LIST_MKUNBOUND(thread,t_pid.tp_grplink))
#define THREAD_PID_ISGROUP(self,thread) (!LIST_ISUNBOUND(thread,t_pid.tp_grplink))
#define TASK_CHILDREN_FOREACH(child,self) LIST_FOREACH(child,(self)->t_pid.tp_children,t_pid.tp_siblings)


/* Return the PID descriptor for the current task. */
#define THIS_THREAD_PID          (&THIS_TASK->t_pid)

/* Return the local pid-namespace of the current task. */
#define THIS_NAMESPACE            (THIS_TASK->t_pid.tp_ids[PIDTYPE_PID].tl_ns)

/* Get tid/pid/ppid as seen from user-space. */
#define THREAD_PID_GETTID(self)         ((self)->tp_ids[PIDTYPE_PID].tl_pid)
#define THREAD_PID_GETTGID(self)          thread_pid_getpgid(self,PIDTYPE_PID)
#define THREAD_PID_GETPID(self)           thread_pid_getpgid(self,PIDTYPE_PID)
#define THREAD_PID_GETPGID(self)          thread_pid_getpgid(self,PIDTYPE_PID)
#define THREAD_PID_GETPPID(self)          thread_pid_getppid(self,PIDTYPE_PID)
#define THREAD_PID_GLOBAL_GETTID(self)  ((self)->tp_ids[PIDTYPE_GPID].tl_pid)
#define THREAD_PID_GLOBAL_GETTGID(self)   thread_pid_getpgid(self,PIDTYPE_GPID)
#define THREAD_PID_GLOBAL_GETPID(self)    thread_pid_getpgid(self,PIDTYPE_GPID)
#define THREAD_PID_GLOBAL_GETPGID(self)   thread_pid_getpgid(self,PIDTYPE_GPID)
#define THREAD_PID_GLOBAL_GETPPID(self)   thread_pid_getppid(self,PIDTYPE_GPID)
#define GET_THIS_TID()             THREAD_PID_GETTID(THIS_THREAD_PID) 
#define GET_THIS_TGID()            THREAD_PID_GETTGID(THIS_THREAD_PID) 
#define GET_THIS_PID()             THREAD_PID_GETPID(THIS_THREAD_PID) 
#define GET_THIS_PGID()            THREAD_PID_GETPGID(THIS_THREAD_PID) 
#define GET_THIS_PPID()            THREAD_PID_GETPPID(THIS_THREAD_PID)
LOCAL pid_t KCALL thread_pid_getpgid(struct thread_pid *__restrict self, pidtype_t type);
LOCAL pid_t KCALL thread_pid_getppid(struct thread_pid *__restrict self, pidtype_t type);

#define GET_THIS_UID()    TASK_GETUID(THIS_TASK)
#define GET_THIS_GID()    TASK_GETGID(THIS_TASK)
#define GET_THIS_EUID()   TASK_GETEUID(THIS_TASK)
#define GET_THIS_EGID()   TASK_GETEGID(THIS_TASK)

#define TASK_GETUID(self)   0 /* TODO */
#define TASK_GETGID(self)   0 /* TODO */
#define TASK_GETEUID(self)  0 /* TODO */
#define TASK_GETEGID(self)  0 /* TODO */

#define TASK_GETTID(self)   THREAD_PID_GETTID(&(self)->t_pid)
#define TASK_GETTGID(self)  THREAD_PID_GETTGID(&(self)->t_pid)
#define TASK_GETPID(self)   THREAD_PID_GETPID(&(self)->t_pid)
#define TASK_GETPGID(self)  THREAD_PID_GETPGID(&(self)->t_pid)
#define TASK_GETPPID(self)  THREAD_PID_GETPPID(&(self)->t_pid)


struct pid_bucket {
 WEAK REF LIST_HEAD(struct task) pb_chain; /*< [0..1][lock(:pn_lock)] Linked list of tasks sharing this modulated PID-hash. */
};
struct pid_namespace {
 ATOMIC_DATA ref_t  pn_refcnt; /*< Reference counter for this PID namespace. */
 pidtype_t          pn_type;   /*< [const] The type of process identifier managed by this namespace.
                                *   NOTE: Used as index into the 'tp_ids' vector of all managed tasks. */
 pid_t              pn_min;    /*< [const] Lowest PID that this namespace should be handing out
                                *  (To prevent special IDs such as ZERO(0) from being used
                                *   unless explicitly required, such as by 'inittask') */
 pid_t              pn_max;    /*< [const] The greatest valid PID within this namespace. */
 atomic_rwlock_t    pn_lock;   /*< Lock used to access the namespace's PID hash-map. */
 pid_t              pn_next;   /*< [lock(pn_lock)] The next PID to hand out during automatic PID generation. */
 size_t             pn_mapa;   /*< [lock(pn_lock)] Allocated amount of buckets. */
 size_t             pn_mapc;   /*< [lock(pn_lock)] Amount of threads tracked by 'pn_map'. */
 struct pid_bucket *pn_map;    /*< [0..pn_mapa][lock(pn_lock)][INDEX(PID % pn_mapa)][owned]
                                *   Hash-map to map threads to their respective process ids. */
};
#define PID_NAMESPACE_INCREF(self) (void)(ATOMIC_FETCHINC((self)->pn_refcnt))
#define PID_NAMESPACE_DECREF(self) (void)(ATOMIC_DECFETCH((self)->pn_refcnt) || (pid_namespace_destroy(self),0))

FUNDEF REF struct pid_namespace *KCALL pid_namespace_new(pidtype_t type);
FUNDEF void KCALL pid_namespace_destroy(struct pid_namespace *__restrict self);

/* Lookup a given PID 'id' within the specified PID-namespace.
 * NOTE: 'pid_namespace_lookup()' returns NULL if the task's weak reference cannot be locked.
 * @return: NULL: Failed to find a valid task matching the given id. */
FUNDEF      REF struct task *KCALL pid_namespace_lookup(struct pid_namespace *__restrict self, pid_t id);
FUNDEF WEAK REF struct task *KCALL pid_namespace_lookup_weak(struct pid_namespace *__restrict self, pid_t id);

#define TASK_IS_ZOMBIE(self) ((self)->t_mode == TASKMODE_TERMINATED)

/* Global/Init-pid namespace IDs of bootstrap tasks. */
#define BOOTCPU_IDLE_PID 0 /*< This task is actually kept forever! */
#define BOOTTASK_PID     1 /*< WARNING: This task is later replaced by '/bin/init'. */

DATDEF struct pid_namespace pid_global; /* The global PID namespace. */
DATDEF struct pid_namespace pid_init;   /* The init PID namespace (Initial namespace used for local pids). */

#ifdef CONFIG_BUILDING_KERNEL_CORE
INTDEF void KCALL pid_initialize(void);
#endif /* CONFIG_BUILDING_KERNEL_CORE */

#endif /* __CC__ */


















#define SIGPENDING_OFFSETOF_NEWSIG  0
#define SIGPENDING_OFFSETOF_QUEUE   SIG_SIZE
#define SIGPENDING_OFFSETOF_MASK   (SIG_SIZE+__SIZEOF_POINTER__)
#define SIGPENDING_SIZE            (SIG_SIZE+__SIZEOF_POINTER__+__SIZEOF_SIGSET_T__)
#ifdef __CC__
struct sigqueue;
struct sigshare;
struct sigpending {
 /* NOTE: When locking both the shared and local sigpending, the local must be locked first! */
 struct sig                  sp_newsig; /*< Signal used to lock this structure and broadcast addition of new signals. */
 SLIST_HEAD(struct sigqueue) sp_queue;  /*< [0..1][owned][lock(sp_newsig)] List of queued signals. */
 __sigset_t                  sp_mask;   /*< [lock(sp_newsig)] Mask of all signals found in 'sp_queue'. */
};
#define SIGPENDING_INIT              {SIG_INIT,NULL,{}}
#define sigpending_init(self)  (void)(sig_init(&(self)->sp_newsig),(self)->sp_queue = NULL,memset(&(self)->sp_mask,0,sizeof(__sigset_t)))
#define sigpending_cinit(self)       (sig_cinit(&(self)->sp_newsig),assert((self)->sp_queue == NULL))
FUNDEF void KCALL sigpending_fini(struct sigpending *__restrict self);
#endif


#ifdef __i386__
#   define SIGENTER_OFFSETOF_COUNT   0
#   define SIGENTER_OFFSETOF_EIP     4
#   define SIGENTER_OFFSETOF_CS      8
#   define SIGENTER_OFFSETOF_EFLAGS  12
#   define SIGENTER_OFFSETOF_USERESP 16
#   define SIGENTER_OFFSETOF_SS      20
#   define SIGENTER_SIZE             24
#else
#   error FIXME
#endif

#ifdef __CC__
struct sigenter_info;

struct sigenter {
 size_t     se_count; /*< Amount of Signals that need handling. */
 /* Return register values (These were overwritten near the kernel stack base).
  * NOTE: Basically, this is the original iret tail that was used when the kernel was entered. */
 u32        se_eip;
 u16        se_cs,__n0;
 u32        se_eflags;
union{
 u32        se_useresp;
 /* NOTE: 'useresp' has been manipulated to point to
  *        the kernel-generated signal information, compatible
  *        with a user-level call to a sigaction handler. */
 USER struct sigenter_info
           *se_siginfo; /*< Either located on the user-stack, or the signal-alt-stack. */
};
 u16        se_ss,__n1;
};


/* Special in-assembly implementation of the user-space return override
 * used to return to a signal handler instead of the user-space caller's
 * origin.
 * To achieve this, the original user-space iret tail is stored in the
 * current task's 't_sigenter' field, before the system call return
 * information (as available through 'THIS_SYSCALL_*') is overwritten,
 * so-as to stay in kernel-space and call simply 'sigenter()' with
 * all registers except for 'EIP', 'CS', 'EFLAGS', 'USERESP' and 'SS'
 * as they are ought to be when returning to user-space is eventually
 * went through with.
 * >> This work-around is required due to the fact that depending on
 *    which system-call is used (if it even is a system-call that
 *    caused the CPU to switch from user-space to the kernel), there
 *    is (intentionally) no standardized way of determining how
 *    user-space registers are saved (if at all), meaning that the
 *    only thing that persistently is consistent, are the low-level
 *    data fields pushed by the CPU during entry (aka. the iret-tail)
 * >> And it is exactly that tail which we (ab-)use to hack universal
 *    signal enter support for any kind of system interrupt, be it a
 *    system call, an exception, or something different such as a
 *    hardware interrupt. */
FUNDEF ATTR_NORETURN void ASMCALL sigenter(void);


#endif /* __CC__ */

















#define TASK_OFFSETOF_REFCNT          0
#define TASK_OFFSETOF_WEAKCNT         __SIZEOF_REF_T__
#define TASK_OFFSETOF_CSTATE       (2*__SIZEOF_REF_T__)
#ifdef CONFIG_SMP
#define TASK_OFFSETOF_AFFINITY_LOCK (2*__SIZEOF_REF_T__+__SIZEOF_POINTER__)
#define TASK_OFFSETOF_AFFINITY     (2*__SIZEOF_REF_T__+__SIZEOF_POINTER__+ATOMIC_RWLOCK_SIZE)
#define TASK_OFFSETOF_CPU          (2*__SIZEOF_REF_T__+__SIZEOF_POINTER__+ATOMIC_RWLOCK_SIZE+__SIZEOF_CPU_SET_T__)
#define TASK_OFFSETOF_FLAGS        (2*__SIZEOF_REF_T__+2*__SIZEOF_POINTER__+ATOMIC_RWLOCK_SIZE+__SIZEOF_CPU_SET_T__)
#else
#define TASK_OFFSETOF_FLAGS        (2*__SIZEOF_REF_T__+__SIZEOF_POINTER__)
#endif
#define TASK_OFFSETOF_STATE        (TASK_OFFSETOF_FLAGS+2)
#define TASK_OFFSETOF_SCHED        (TASK_OFFSETOF_FLAGS+4)
#define TASK_OFFSETOF_SIGNALS      (TASK_OFFSETOF_FLAGS+4+2*__SIZEOF_POINTER__)
#define TASK_OFFSETOF_TIMEOUT      (TASK_OFFSETOF_FLAGS+4+2*__SIZEOF_POINTER__+TASKSIG_SIZE)
#define TASK_OFFSETOF_PRIORITY     (TASK_OFFSETOF_FLAGS+4+2*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC)
#define TASK_OFFSETOF_PRIOSCORE    (TASK_OFFSETOF_FLAGS+5+2*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC)
#define TASK_OFFSETOF_EXITCODE     (TASK_OFFSETOF_FLAGS+8+2*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC)
#define TASK_OFFSETOF_EVENT        (TASK_OFFSETOF_FLAGS+8+3*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC)
#define TASK_OFFSETOF_CRITICAL     (TASK_OFFSETOF_FLAGS+8+3*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE)
#define TASK_OFFSETOF_NOINTR       (TASK_OFFSETOF_FLAGS+12+3*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE)
#define TASK_OFFSETOF_ADDRLIMIT    (TASK_OFFSETOF_FLAGS+16+3*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE)
#define TASK_OFFSETOF_IC           (TASK_OFFSETOF_FLAGS+16+4*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE)
#define TASK_OFFSETOF_SUSPEND      (TASK_OFFSETOF_FLAGS+16+5*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE)
#define TASK_OFFSETOF_PID          (TASK_OFFSETOF_FLAGS+24+5*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE)
#define TASK_OFFSETOF_HSTACK       (TASK_OFFSETOF_FLAGS+24+5*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE)
#define TASK_OFFSETOF_LASTCR2      (TASK_OFFSETOF_FLAGS+24+5*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE)
#define TASK_OFFSETOF_MMAN_TASKS   (TASK_OFFSETOF_FLAGS+24+6*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE)
#define TASK_OFFSETOF_REAL_MMAN    (TASK_OFFSETOF_FLAGS+24+8*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE)
#define TASK_OFFSETOF_MMAN         (TASK_OFFSETOF_FLAGS+24+9*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE)
#define TASK_OFFSETOF_FDMAN        (TASK_OFFSETOF_FLAGS+24+10*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE)
#define TASK_OFFSETOF_USTACK       (TASK_OFFSETOF_FLAGS+24+11*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE)
#define TASK_OFFSETOF_SIGHAND      (TASK_OFFSETOF_FLAGS+24+12*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE)
#define TASK_OFFSETOF_SIGBLOCK     (TASK_OFFSETOF_FLAGS+24+13*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE)
#define TASK_OFFSETOF_SIGPEND      (TASK_OFFSETOF_FLAGS+24+13*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE+__SIZEOF_SIGSET_T__)
#define TASK_OFFSETOF_SIGSHARE     (TASK_OFFSETOF_FLAGS+24+13*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE+SIGPENDING_SIZE+__SIZEOF_SIGSET_T__)
#define TASK_OFFSETOF_SIGENTER     (TASK_OFFSETOF_FLAGS+24+14*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE+SIGPENDING_SIZE+__SIZEOF_SIGSET_T__)
#ifndef CONFIG_NO_TLB
#define TASK_OFFSETOF_TLB          (TASK_OFFSETOF_FLAGS+24+14*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE+SIGPENDING_SIZE+__SIZEOF_SIGSET_T__+SIGENTER_SIZE)
#endif /* !CONFIG_NO_TLB */
#ifdef ARCHTASK_SIZE
#define TASK_OFFSETOF_ARCH         (TASK_OFFSETOF_FLAGS+24+15*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE+SIGPENDING_SIZE+__SIZEOF_SIGSET_T__+SIGENTER_SIZE)
#define TASK_SIZE                  (TASK_OFFSETOF_FLAGS+24+15*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE+SIGPENDING_SIZE+__SIZEOF_SIGSET_T__+SIGENTER_SIZE+ARCHTASK_SIZE)
#else
#define TASK_SIZE                  (TASK_OFFSETOF_FLAGS+24+15*__SIZEOF_POINTER__+TASKSIG_SIZE+__SIZEOF_TIMESPEC+SIG_SIZE+THREAD_PID_SIZE+HSTACK_SIZE+SIGPENDING_SIZE+__SIZEOF_SIGSET_T__+SIGENTER_SIZE)
#endif
#define TASK_ALIGN                  TASKSIGSLOT_ALIGN

#ifdef __CC__
struct task {
 ATOMIC_DATA ref_t       t_refcnt;    /*< Task reference counter. */
 ATOMIC_DATA ref_t       t_weakcnt;   /*< Task weak reference counter. */
 HOST struct cpustate   *t_cstate;    /*< [1..1][in(t_hstack)][lock(PRIVATE(THIS_CPU))]
                                       *  [valid_if(t_mode != TASKMODE_RUNNING || t_cpu->c_running != this)]
                                       *  Pointer to a location on the host stack, describing the CPU state to return to when switching to the task. */
#ifdef CONFIG_SMP
 atomic_rwlock_t         t_affinity_lock; /*< [ORDER(AFTER(t_cpu->c_lock))] Task affinity lock. */
 __cpu_set_t             t_affinity;  /*< [lock(t_affinity_lock)] CPU Affinity of this task. */
 ATOMIC_DATA struct cpu *t_cpu;       /*< [1..1][lock(READ(t_cpu->c_lock))]
                                       *  [lock(WRITE(t_cpu->c_lock && (t_mode != TASKMODE_RUNNING || t_cpu == THIS_CPU)))]
                                       *   CPU That this task is running on/scheduled by. */
#define TASK_CPU(x) (x)->t_cpu
#else /* CONFIG_SMP */
#define TASK_CPU(x) (&__bootcpu)
#endif /* !CONFIG_SMP */
 taskflag_t              t_flags;     /*< [lock(THIS_TASK || (t_mode != TASKMODE_RUNNING && t_cpu->c_lock))]
                                       *   Task flags (Set of 'TASKFLAG_*'). */
 taskmode_t              t_mode;      /*< [lock(TASK_CPU(self)->c_lock)] The current task mode. */
 u8                      t_padding;   /*< ... */
 union {
  RING_NODE(struct task) sd_running;  /*< [valid_if(t_mode == TASKMODE_RUNNING)][lock(PRIVATE(t_cpu == THIS_CPU))] Ring entry for tasks running on 't_cpu'. */
  LIST_NODE(struct task) sd_sleeping; /*< [valid_if(t_mode == TASKMODE_WAKEUP || t_mode == TASKMODE_SLEEPING)][lock(t_cpu->c_lock)] List entry of wakeup/sleeping tasks. */
  LIST_NODE(struct task) sd_suspended;/*< [valid_if(t_mode == TASKMODE_SUSPENDED)][lock(t_cpu->c_lock)] List entry of suspended tasks. */
 }                       t_sched;     /*< Scheduling chain data. */
 struct tasksig          t_signals;   /*< [lock(THIS_TASK || (t_mode != TASKMODE_RUNNING && t_cpu->c_lock))] Signal wait controller. */
 struct timespec         t_timeout;   /*< [valid_if(t_mode == TASKMODE_WAKEUP ||t_mode == TASKMODE_SLEEPING)][lock(t_cpu->c_lock)]
                                       *  Timeout used when sleeping. When this time expires, the task is re-scheduled during the
                                       *  next IRQ of the associated CPU, and the last blocking operation will fail with '-ETIMEDOUT'. */
 taskprio_t              t_priority;  /*< [lock(t_cpu == THIS_CPU || READ(ATOMIC))] The effective task priority. */
 taskprio_t              t_prioscore; /*< [lock(t_cpu == THIS_CPU)] Current priority score. */
 u16                     t_padding2;  /*< ... */
 void                   *t_exitcode;  /*< [lock(t_cpu->c_lock)] Exitcode of the thread. */
 struct sig              t_event;     /*< Signal send when certain internal events are triggered: 
                                       *   - after 't_mode' is set to 'TASKMODE_TERMINATED'
                                       *   - XXX: vfork() complete?
                                       */
 u32                     t_critical;  /*< [lock(READ(WEAK),WRITE(THIS_TASK))]
                                       *  The task is critical and must not be terminated.
                                       *  NOTE: The lock must only be held when decrementing to ZERO(0). */
 u32                     t_nointr;    /*< [lock(PRIVATE(THIS_TASK))] While non-zero, don't receive interrupts ('-EINTR' is not accepted). */
 uintptr_t               t_addrlimit; /*< [lock(PRIVATE(THIS_TASK))] Only USER-pointers < than this are considered valid. */
 HOST struct intchain   *t_ic;        /*< [lock(PRIVATE(THIS_TASK))][0..1] Chain of local, kernel-level interrupt handlers. */
 s32                     t_suspend[2];/*< [lock(t_cpu->c_lock)] Recursion counter for task_(resume|suspend)
                                       *  NOTE: The lock must only be held when writing in the event that the
                                       *        updated value would require the task's state to be changed.
                                       *  >> When <= 0, 'TASKMODE_RUNNING' (Or other states, based on)
                                       *  >> When >  0, 'TASKMODE_SUSPENDED'
                                       *  NOTE: When a suspended task receives a signal, this field is
                                       *        checked to confirm that the task should really wake up.
                                       *        When it isn't allowed to wake up, the 'TASKFLAG_SUSP_NOCONT' flag is deleted.
                                       *  NOTE: The first counter is used for user-space and the second for kernel-space. 
                                       *        In addition, the second element is unsigned and does not allow for running-recursion! */
 /* TODO: TLS data (will also be used for raising exceptions) */

 /* Thread descriptor/context information. */
 struct thread_pid        t_pid;       /*< [const] Process descriptor information. */
 struct hstack            t_hstack;    /*< [const] Kernel stack. */
 VIRT void               *t_lastcr2;   /*< [lock(PRIVATE(THIS_TASK))] The exact address of the last unhandled page-fault.
                                        *   >> Stored here, as the register value is volatile and may
                                        *      be overwritten at any time due to preemption or ALOA/COW. */
 LIST_NODE(struct task)   t_mman_tasks;/*< [lock(t_mman->m_tasks_lock)] Chain of tasks using 't_mman' */
 VIRT     struct mman    *t_real_mman; /*< [const][1..1] The real memory manager of this task. */
 VIRT REF struct mman    *t_mman;      /*< [lock(PRIVATE(THIS_TASK))][1..1] The effective memory manager & page directory used by this thread.
                                        *   WARNING: Do _NOT_ try to suspend a task to gain access to this field.
                                        *   It may not contain the correct value! (s.a.: 'TASK_PDIR_KERNEL_BEGIN') */
 REF struct fdman        *t_fdman;     /*< [lock(PRIVATE(THIS_TASK))][1..1] This task's file descriptor manager. */
 REF struct stack        *t_ustack;    /*< [0..1][lock(PRIVATE(THIS_TASK))] Userspace stack (if allocated). */
 REF struct sighand      *t_sighand;   /*< [1..1][const] Userspace signal handlers. */
 __sigset_t               t_sigblock;  /*< [lock(PRIVATE(THIS_TASK))] Set of signals currently being blocked by this task. */
 struct sigpending        t_sigpend;   /*< Controller for pending signals. */
 REF struct sigshare     *t_sigshare;  /*< [1..1] Controller for shared signal data (Including the shared pending-signal list). */
 struct sigenter          t_sigenter;  /*< Signal enter controller. */
#ifndef CONFIG_NO_TLB
 PAGE_ALIGNED USER struct tlb *t_tlb;  /*< [0..1|null(PAGE_ERROR)][owned] A user-space memory mapping for this task's thread-local information block.
                                        *  NOTE: During context switching, the calling CPU's GDT is updated to the new task's tlb and tib.
                                        *  HINT: Apply negative offsets to this pointer to access ELF user-space thread-local memory. */
#else
 uintptr_t                t_padding3;  /*< ... */
#endif
#ifdef ARCHTASK_SIZE
 struct archtask          t_arch;      /*< Arch-specific task information controller. */
#endif
};
#define TASK_ISSUSPENDED(self) ((self)->t_suspend[0] > 0 || (self)->t_suspend[1] != 0)

LOCAL SAFE pid_t KCALL thread_pid_getpgid(struct thread_pid *__restrict self, pidtype_t type) {
 pid_t result;
 atomic_rwlock_read(&self->tp_leadlock);
 result = self->tp_leader->t_pid.tp_ids[type].tl_pid;
 atomic_rwlock_endread(&self->tp_leadlock);
 return result;
}
LOCAL SAFE pid_t KCALL thread_pid_getppid(struct thread_pid *__restrict self, pidtype_t type) {
 pid_t result;
 atomic_rwlock_read(&self->tp_parlock);
 result = self->tp_parent->t_pid.tp_ids[type].tl_pid;
 atomic_rwlock_endread(&self->tp_parlock);
 return result;
}
#endif /* __CC__ */






#define CPU_OFFSETOF_SELF           0
#define CPU_OFFSETOF_RUNNING        __SIZEOF_POINTER__
#define CPU_OFFSETOF_IDLING      (2*__SIZEOF_POINTER__)
#define CPU_OFFSETOF_ID          (3*__SIZEOF_POINTER__)
#define CPU_OFFSETOF_PRIO_MIN    (3*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__)
#define CPU_OFFSETOF_PRIO_MAX    (3*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+1)
#define CPU_OFFSETOF_LOCK        (3*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+2)
#define CPU_OFFSETOF_SUSPENDED   (3*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+2+ATOMIC_RWLOCK_SIZE)
#define CPU_OFFSETOF_SLEEPING    (4*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+2+ATOMIC_RWLOCK_SIZE)
#define CPU_OFFSETOF_IDLE        (5*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+6+ATOMIC_RWLOCK_SIZE)
#define CPU_OFFSETOF_ARCH        (5*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+6+ATOMIC_RWLOCK_SIZE+TASK_SIZE)
#define CPU_OFFSETOF_N_RUN       (5*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+6+ATOMIC_RWLOCK_SIZE+TASK_SIZE+ARCHCPU_SIZE)
#define CPU_OFFSETOF_N_IDLE      (5*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+6+ATOMIC_RWLOCK_SIZE+TASK_SIZE+ARCHCPU_SIZE+__SIZEOF_SIZE_T__)
#define CPU_OFFSETOF_N_SUSP      (5*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+6+ATOMIC_RWLOCK_SIZE+TASK_SIZE+ARCHCPU_SIZE+2*__SIZEOF_SIZE_T__)
#define CPU_OFFSETOF_N_SLEEP     (5*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+6+ATOMIC_RWLOCK_SIZE+TASK_SIZE+ARCHCPU_SIZE+3*__SIZEOF_SIZE_T__)
#define CPU_SIZE                 (5*__SIZEOF_POINTER__+__SIZEOF_CPUID_T__+6+ATOMIC_RWLOCK_SIZE+TASK_SIZE+ARCHCPU_SIZE+4*__SIZEOF_SIZE_T__)

#ifdef __CC__
struct cpu {
 /* NOTE: This data structure is stored at '0(%CPU_BASEREGISTER)'
  *       per-cpu data is stored at negative offsets. */
 struct cpu                *c_self;      /*< [1..1][== self] CPU self-pointer. */
 REF RING_HEAD(struct task) c_running;   /*< [1..1][lock(PRIVATE(THIS_CPU))] Ring chain of running tasks.
                                          *   NOTE: This field is synchronized by being 100% private to the cpu itself.
                                          *         Exceptions to this only apply when the CPU isn't running.
                                          *   NOTE: This ring is never empty, as a special IDLE-task is always apart of it. */
 REF RING_HEAD(struct task) c_idling;    /*< [0..1][lock(PRIVATE(THIS_CPU))][sort(DESCENDING(t_priority),DESCENDING(t_mman))] Ring of idle tasks.
                                          *   WARNING: unlike normal rings, one's front/back pointers are set to NULL (making it a direct, doubly-linked list). */
 cpuid_t                    c_id;        /*< [smp_hwcpu.hw_cpuv[c_id] == self] ID of this CPU. */
 taskprio_t                 c_prio_min;  /*< [lock(PRIVATE(THIS_CPU))] Lowest priority of any task in 'c_running'. */
 taskprio_t                 c_prio_max;  /*< [lock(PRIVATE(THIS_CPU))] Greatest priority of any task in 'c_running'. */
 atomic_rwlock_t            c_lock;      /*< General purpose access lock for this CPU.
                                          *  HINT: During an IRQ switch, the CPU attempts to acquire
                                          *        a write-lock on this primitive. In the event that
                                          *        doing so fails (-EAGAIN case), no task wakeups are
                                          *        performed and no state changes are mirrored for that cycle. */
 REF LIST_HEAD(struct task) c_suspended; /*< [0..1][->t_cpu == this][lock(c_lock)][sort(DESCENDING(t_mman))] Chain of suspended tasks. */
 REF LIST_HEAD(struct task) c_sleeping;  /*< [0..1][->t_cpu == this][lock(c_lock)][sort(DESCENDING(t_mode == TASKMODE_WAKEUP),t_mode == TASKMODE_WAKEUP
                                          *                                          ?  DESCENDING(t_mman)
                                          *                                          : (DESCENDING(t_timeout),DESCENDING(t_mman)))]
                                          *   NOTE: This first part of this chain contains all tasks in 'TASKMODE_WAKEUP'-mode, sorted by associated 't_mman'.
                                          *         The second part contains all tasks in 'TASKMODE_SLEEPING'-mode, first sorted by timeout, then by 't_mman'.
                                          *   Chain of wakeup/sleeping tasks. */
 u32                        c_padding;   /*< ... */
 struct task                c_idle;      /*< A small, lightweight IDLE task for this CPU.
                                          *  HINT: This task is also (ab-)used when booting up a cpu. */
 struct archcpu             c_arch;      /*< Arch-specific per-cpu information. */
 WEAK size_t                c_n_run;     /*< [lock(PRIVATE(THIS_CPU))][!0] Total amount of tasks within 'c_running' */
 WEAK size_t                c_n_idle;    /*< [lock(PRIVATE(THIS_CPU))] Total amount of tasks within 'c_idling' */
 WEAK size_t                c_n_susp;    /*< [lock(c_lock)] Total amount of tasks within 'c_suspended' */
 WEAK size_t                c_n_sleep;   /*< [lock(c_lock)] Total amount of tasks within 'c_sleeping' */
};
#define CPU_FOREACH_RUNNING_DO(elem,self)    do{elem = (self)->c_running; do
#define CPU_FOREACH_RUNNING_WHILE(elem,self) while(((elem) = (elem)->t_sched.sd_running.re_next) != (self)->c_running);}while(0)
#define CPU_FOREACH_IDLING(elem,self)        for ((elem) = (self)->c_idling; (elem); (elem) = (elem)->t_sched.sd_running.re_next)
#define CPU_FOREACH_SUSPENDED(elem,self)     LIST_FOREACH(elem,(self)->c_suspended,t_sched.sd_suspended)
#define CPU_FOREACH_SLEEPING(elem,self)      LIST_FOREACH(elem,(self)->c_sleeping,t_sched.sd_sleeping)

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_INCLUDE_SCHED_TYPES_H */
