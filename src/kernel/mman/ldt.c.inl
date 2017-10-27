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
#ifndef GUARD_KERNEL_MMAN_LDT_C_INL
#define GUARD_KERNEL_MMAN_LDT_C_INL 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <kernel/arch/gdt.h>
#include <kernel/mman.h>
#include <sched.h>
#include <sched/cpu.h>
#include <sched/smp.h>
#include <sched/task.h>
#include <sched/types.h>
#include <string.h>

#ifndef CONFIG_NO_LDT
DECL_BEGIN

DEFINE_PUBLIC_ALIAS(ldt_kernel,ldt_empty);
PUBLIC struct ldt ldt_empty = {
    .l_idt    = {0,{NULL}},
    .l_gdt    = SEG_KERNEL_LDT,
#ifdef CONFIG_DEBUG
    .l_refcnt = 2, /* `ldt_empty', `mman_kernel.m_ldt' */
#else
    .l_refcnt = 0x80000002,
#endif
    .l_lock   = ATOMIC_RWLOCK_INIT,
#ifdef CONFIG_SMP
    /* The empty LDT is valid on all CPUs by default. */
    .l_valid  = CPU_SETALL,
#endif
    .l_tasks  = &inittask,
};

PUBLIC void KCALL ldt_destroy(struct ldt *__restrict self) {
 cpuid_t cid;
 CHECK_HOST_DOBJ(self);
 assert(self != &ldt_empty);
 assertf(self->l_tasks == NULL,
         "But if there are still task using it, how are we being destroyed?");
#if defined(CONFIG_DEBUG) && defined(CONFIG_SMP)
 /* Clear our the entries in all CPUs that allocated it.
  * >> Doing this prevents dangling (and therefor illegal)
  *    pointers from causing undefined behavior. */
 { struct segment empty_seg = SEGMENT_INIT(0,0,0);
   for (cid = 0; cid < SMP_COUNT; ++cid) {
    if (CPU_ISSET(cid,&self->l_valid))
        vgdt_set(CPUI(cid),self->l_gdt,empty_seg,NULL);
   }
 }
#endif
 gdt_del(self->l_gdt);
 free(self->l_idt.ip_ldt);
 free(self);
}

#ifndef __INTELLISENSE__
#define NEW
#include "ldt-set-new.c.inl"
#include "ldt-set-new.c.inl"
#endif

FUNDEF struct segment KCALL
mman_getldt_unlocked(struct mman *__restrict self, ldt_t id) {
 struct segment result; struct ldt *ldt;
 CHECK_HOST_DOBJ(self);
 assert(mman_reading(self));
 ldt = self->m_ldt;
 CHECK_HOST_DOBJ(ldt);
 ldt_read(ldt);
 assert(id/sizeof(struct segment) < ldt->l_idt.ip_limit/sizeof(struct segment));
 assert((id % sizeof(struct segment)) == 0);
 result = *(struct segment *)((byte_t *)ldt->l_idt.ip_ldt+id);
 ldt_endread(ldt);
 return result;
}


DECL_END
#endif /* !CONFIG_NO_LDT */

#endif /* !GUARD_KERNEL_MMAN_LDT_C_INL */
