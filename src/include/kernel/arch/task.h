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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_TASK_H
#define GUARD_INCLUDE_KERNEL_ARCH_TASK_H 1

#include <hybrid/compiler.h>

#ifndef CONFIG_NO_LDT
#include <hybrid/typecore.h>
#include <hybrid/types.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/arch/idt_pointer.h>
#include <kernel/arch/gdt.h>
#include <bits/sched.h>
#endif /* !CONFIG_NO_LDT */
#ifndef CONFIG_NO_FPU
#include <kernel/arch/fpu.h>
#endif /* !CONFIG_NO_FPU */

DECL_BEGIN

#ifndef CONFIG_NO_LDT
#define LDT_OFFSETOF_IDT    0
#define LDT_OFFSETOF_GDT    6
#define LDT_OFFSETOF_REFCNT 8

#ifdef __CC__

#ifndef __segid_t_defined
#define __segid_t_defined 1
typedef u16 segid_t;
#endif /* !__segid_t_defined */

/* Local descriptor table index*8 */
typedef segid_t ldt_t;
#define LDT_ERROR ((ldt_t)-1)

struct segment;
struct ldt {
 struct idt_pointer          l_idt;    /*< [lock(l_lock && l_refcnt == 1)]
                                        *   IDT pointer (Keep at offset ZERO(0) to allow for better optimizations) */
 segid_t                     l_gdt;    /*< [const] GDT segment index*8 */
 ATOMIC_DATA ref_t           l_refcnt; /*< Reference counter. */
 atomic_rwlock_t             l_lock;   /*< Lock for this LDT controller. */
#ifdef CONFIG_SMP
 __cpu_set_t                 l_valid;  /*< [lock(SET_ONCE)] The set of CPUs on which this LDT is allocated. */
#endif
 WEAK LIST_HEAD(struct task) l_tasks;  /*< [lock(l_lock)][0..1] The list of tasks currently using this LDT. */
};
#define LDT_FOREACH_TASK(t,self) \
       LIST_FOREACH(t,(self)->l_tasks,t_arch.at_ldt_tasks)


#define ldt_reading(x)     atomic_rwlock_reading(&(x)->l_lock)
#define ldt_writing(x)     atomic_rwlock_writing(&(x)->l_lock)
#define ldt_tryread(x)     atomic_rwlock_tryread(&(x)->l_lock)
#define ldt_trywrite(x)    atomic_rwlock_trywrite(&(x)->l_lock)
#define ldt_tryupgrade(x)  atomic_rwlock_tryupgrade(&(x)->l_lock)
#define ldt_read(x)        atomic_rwlock_read(&(x)->l_lock)
#define ldt_write(x)       atomic_rwlock_write(&(x)->l_lock)
#define ldt_upgrade(x)     atomic_rwlock_upgrade(&(x)->l_lock)
#define ldt_downgrade(x)   atomic_rwlock_downgrade(&(x)->l_lock)
#define ldt_endread(x)     atomic_rwlock_endread(&(x)->l_lock)
#define ldt_endwrite(x)    atomic_rwlock_endwrite(&(x)->l_lock)

/* NOTE: There is no `ldt_new()' function.
 *       Instead, use a memory manager to manipulate a LDT. */
#define LDT_INCREF(self)     (void)(ATOMIC_FETCHINC((self)->l_refcnt))
#define LDT_DECREF(self)     (void)(ATOMIC_DECFETCH((self)->l_refcnt) || (ldt_destroy(self),0))
FUNDEF void KCALL ldt_destroy(struct ldt *__restrict self);

struct mman;
/* LDT accessor functions.
 * NOTE: The caller must be holding a read/write-lock on `self' respectively.
 * NOTE: If the given `id' is invalid, undefined behavior is invoked.
 *       To circumvent this, only pass ids that _you_ have allocated using `mman_newldt()' */
FUNDEF struct segment KCALL mman_getldt_unlocked(struct mman *__restrict self, ldt_t id);
FUNDEF errno_t KCALL mman_setldt_unlocked(struct mman *__restrict self, ldt_t id,
                                          struct segment seg, struct segment *oldseg);

/* Allocate/Delete LDT entries.
 * NOTE: The caller must be holding a write-lock on `self'
 * @return: * :         The new segment id.
 * @return: E_ISERR(*): Failed to allocate a segment for some reason. */
FUNDEF s32 KCALL mman_newldt_unlocked(struct mman *__restrict self, struct segment seg);
#define mman_delldt_unlocked(self,id) mman_setldt_unlocked(self,id,SEGMENT_DELETE,NULL)
#define SEGMENT_DELETE                make_segment(0,0,0)


DATDEF struct ldt ldt_empty;  /*< The default, empty LDT descriptor table. - Used whenever an LDT controller would become empty. */
DATDEF struct ldt ldt_kernel; /*< The LDT controller used by the kernel (Currently an alias for `ldt_empty') */

#endif /* __CC__ */
#endif /* !CONFIG_NO_LDT */


#if !defined(CONFIG_NO_LDT) || !defined(CONFIG_NO_FPU)
#ifndef CONFIG_NO_LDT
#   define ARCHTASK_OFFSETOF_LDT_TASKS  0
#   define ARCHTASK_OFFSETOF_LDT_GDT   (2*__SIZEOF_POINTER__)
#   define __ARCHTASK_OFFSETAFTER_LDT  (3*__SIZEOF_POINTER__)
#else
#   define __ARCHTASK_OFFSETAFTER_LDT   0
#endif
#ifndef CONFIG_NO_FPU
#   define ARCHTASK_OFFSETOF_FPU        __ARCHTASK_OFFSETAFTER_LDT
#   define __ARCHTASK_OFFSETAFTER_FPU  (__ARCHTASK_OFFSETAFTER_LDT+__SIZEOF_POINTER__)
#else
#   define __ARCHTASK_OFFSETAFTER_FPU   __ARCHTASK_OFFSETAFTER_LDT
#endif
#   define ARCHTASK_SIZE                __ARCHTASK_OFFSETAFTER_FPU

#ifdef __CC__
struct archtask {
#ifndef CONFIG_NO_LDT
 WEAK LIST_NODE(struct task) at_ldt_tasks; /*< [0..1][lock(:t_mman->m_ldt->l_lock)] Linked list of tasks using the associated LDT descriptor. */
union PACKED { struct PACKED {
 WEAK segid_t                at_ldt_gdt;   /*< [== :t_mman->m_ldt->l_gdt][valid_if(:t_mode != TASKMODE_NOTSTARTED)] Local copy of this task's LDT index. */
}; uintptr_t               __at_align;};   /* Ensure pointer-alignment. */
#endif /* !CONFIG_NO_LDT */
#ifndef CONFIG_NO_FPU
 struct fpustate            *at_fpu;       /*< [lock(PRIVATE(THIS_TASK))][0..1][owned]
                                            *   Stored FPU state after this task executed FPU
                                            *   instructions, followed by another task doing the same. */
#endif /* !CONFIG_NO_FPU */
};
#endif /* __CC__ */
#endif /* CONFIG_NO_LDT || CONFIG_NO_FPU */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_TASK_H */
