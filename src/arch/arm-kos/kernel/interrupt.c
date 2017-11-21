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
#ifndef GUARD_KERNEL_ARCH_INTERRUPT_C
#define GUARD_KERNEL_ARCH_INTERRUPT_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <kernel/interrupt.h>
#include <sched/cpu.h>
#include <sched/smp.h>
#include <linker/module.h>
#include <kernel/malloc.h>
#include <hybrid/panic.h>

DECL_BEGIN

STATIC_ASSERT(offsetof(struct interrupt,i_intno) == INTERRUPT_OFFSETOF_INTNO);
STATIC_ASSERT(offsetof(struct interrupt,i_mode) == INTERRUPT_OFFSETOF_MODE);
STATIC_ASSERT(offsetof(struct interrupt,i_type) == INTERRUPT_OFFSETOF_TYPE);
STATIC_ASSERT(offsetof(struct interrupt,i_prio) == INTERRUPT_OFFSETOF_PRIO);
STATIC_ASSERT(offsetof(struct interrupt,i_flags) == INTERRUPT_OFFSETOF_FLAGS);
STATIC_ASSERT(offsetof(struct interrupt,i_callback) == INTERRUPT_OFFSETOF_CALLBACK);
STATIC_ASSERT(offsetof(struct interrupt,i_closure) == INTERRUPT_OFFSETOF_CLOSURE);
STATIC_ASSERT(offsetof(struct interrupt,i_owner) == INTERRUPT_OFFSETOF_OWNER);
STATIC_ASSERT(offsetof(struct interrupt,i_fini) == INTERRUPT_OFFSETOF_FINI);
STATIC_ASSERT(offsetof(struct interrupt,i_hits) == INTERRUPT_OFFSETOF_HITS);
STATIC_ASSERT(offsetof(struct interrupt,i_miss) == INTERRUPT_OFFSETOF_MISS);
STATIC_ASSERT(offsetof(struct interrupt,i_link) == INTERRUPT_OFFSETOF_LINK);
STATIC_ASSERT(sizeof(struct interrupt) == INTERRUPT_SIZE);


#ifdef CONFIG_SMP
/* Lock that must be held when modifying any `inttab'. */
PRIVATE DEFINE_ATOMIC_RWLOCK(inttab_lock);
#define IFDEF_SMP(x) x
#else
#define IFDEF_SMP(x) (void)0
#endif


#define MAX_VECTOR_NUMBER 7
struct entry {
 LIST_HEAD(struct interrupt) e_head; /*< [0..1] Chain of interrupt handlers in this entry. */
};
struct PACKED interrupt_table {
 struct entry it_tab[8]; /*< Per-cpu + per-vector descriptors. */
};
INTERN CPU_BSS struct PACKED interrupt_table inttab;

PRIVATE void KCALL
receive_rpc_update_idt(irq_t intno) {
 /* TODO */
}



INTERN ATTR_FREETEXT void KCALL
cpu_interrupt_initialize(struct cpu *__restrict c) {
}

INTERN ATTR_FREETEXT void KCALL interrupt_initialize(void) {
 /* Initialize the default IDT vector. */
 cpu_interrupt_initialize(BOOTCPU);
}


/* --------------- End of ARCH-dependent code --------------- */
#ifdef CONFIG_SMP
LOCAL bool KCALL int_is_unused(irq_t intno, cpu_set_t *__restrict affinity) {
 /* TODO */
 return true;
}
LOCAL bool KCALL int_is_unused2(irq_t intno, cpu_set_t *__restrict affinity) {
 /* TODO */
 return true;
}
#else
#define int_is_unused(intno,affinity) true /* TODO */
#define int_is_unused2(intno,affinity) true /* TODO */
#endif

PRIVATE void KCALL
interrupt_delete(struct interrupt *__restrict entry) {
 if (entry->i_fini) (*entry->i_fini)(entry);
 INSTANCE_DECREF(entry->i_owner);
 if (entry->i_flags&INTFLAG_FREEDESCR)
     kfree(entry);
}

#ifdef CONFIG_SMP
PUBLIC ssize_t KCALL
int_add(struct interrupt *__restrict entry) {
 cpu_set_t affinity; CPU_ZERO(&affinity);
 return int_addset(entry,&affinity);
}
PUBLIC ssize_t KCALL
int_addset(struct interrupt *__restrict entry,
            cpu_set_t *__restrict affinity)
#else
PUBLIC ssize_t KCALL
int_add(struct interrupt *__restrict entry)
#endif
{
 struct cpu *c;
 struct interrupt *delete_chain = NULL;
 ssize_t result = 0;
 pflag_t was = PREEMPTION_PUSH();
 IFDEF_SMP(atomic_rwlock_write(&inttab_lock));
 assertf(entry->i_owner,"No interrupt owner assigned");
 assertf((entry->i_type&INTTYPE_MASK) <= (INTTYPE_ASM_SEG&INTTYPE_MASK)
         ? (entry->i_flags&INTFLAG_PRIMARY) && (entry->i_prio == INTPRIO_MAX) : 1,
         "Assembly-level interrupt handler _must_ always be `INTFLAG_PRIMARY' + `INTPRIO_MAX'");

#ifdef CONFIG_SMP
 if (CPU_ISEMPTY(affinity))
     CPU_SET(THIS_CPU->c_id,affinity);
 else {
  FOREACH_CPU(c) {
   /* Make sure that the interrupt describes an existing CPU. */
   if (CPU_ISSET(c->c_id,affinity))
    goto cpu_exists;
  }
  result = -ESRCH;
  goto end;
 }
cpu_exists:
#endif
 if (entry->i_flags&INTFLAG_DYNINTNO) {
  /* Dynamically lookup an unused interrupt number. */
  result = MAX_VECTOR_NUMBER;
  do if (int_is_unused(result,affinity))
         goto got_intno;
  while (result--);
  /* TODO: Only search for shared handlers if the caller's can be.
   *       Otherwise, check if we can merge other handlers to free
   *       up a unique slot for the caller.
   */
  result = MAX_VECTOR_NUMBER;
  /* TODO: Use the interrupt with the least amount
   *       of shared handlers, rather than the first! */
  do if (int_is_unused2(result,affinity))
         goto got_intno;
  while (result--);
  result = -ENOMEM;
  goto end;
got_intno:
  entry->i_intno = (irq_t)result;
 } else {
  /* Check for any prior uses of the specified interrupt number. */
  FOREACH_CPU(c) {
   struct interrupt *handler;
#ifdef CONFIG_SMP
   if (!CPU_ISSET(c->c_id,affinity)) continue;
#endif
   handler = VCPU(c,inttab).it_tab[entry->i_intno].e_head;
   /* No existing handler. - The entry can be used just like that. */
   if (handler) {
    /* Doesn't matter: The two handlers can share. */
    if ((handler->i_type&INTTYPE_NOSHARE) ||
        (entry->i_type&INTTYPE_NOSHARE)) {
     /* Never override primary handlers. */
     if (handler->i_flags&INTFLAG_PRIMARY) goto err_eexist;
     /* Don't override non-secondary handlers with non-primary. */
     if (!(handler->i_flags&INTFLAG_SECONDARY) &&
         !(entry->i_flags&INTFLAG_PRIMARY)) goto err_eexist;
    }
   }
  }
 }

 /* Add the interrupt handler to all affected CPUs. */
#ifndef CONFIG_SMP
 if (!INSTANCE_INCREF(entry->i_owner))
      goto err_eperm;
 result = 1;
#else
 result = 0;
 FOREACH_CPU(c) {
  if (!CPU_ISSET(c->c_id,affinity)) continue;
  ++result;
 }
 if (result) {
  if (!INSTANCE_INCREF_N(entry->i_owner,result))
       goto err_eperm;
 }
#endif

 FOREACH_CPU(c) {
  struct interrupt **piter,*iter;
#ifdef CONFIG_SMP
  if (!CPU_ISSET(c->c_id,affinity)) continue;
#endif
  piter = &VCPU(c,inttab).it_tab[entry->i_intno].e_head;
  if ((iter = *piter) == NULL) {
   /* The first link. */
   entry->i_link.le_next = NULL;
  } else if (entry->i_type&INTTYPE_NOSHARE ||
             iter->i_type &INTTYPE_NOSHARE) {
   /* Override existing handlers. */
#ifdef CONFIG_SMP
   if (delete_chain) {
    /* Append an existing delete-chain at the end of what's getting added. */
    struct interrupt *last = iter;
    while (last->i_link.le_next)
           last = last->i_link.le_next;
    last->i_link.le_next = delete_chain;
    delete_chain->i_link.le_pself = &last->i_link.le_next;
    delete_chain = iter;
   }
#endif
   delete_chain = iter;
   entry->i_link.le_next = NULL;
  } else {
   while ((iter = *piter) != NULL && INTERRUPT_BEFORE(iter,entry))
           piter = &iter->i_link.le_next;
   /* Insert the new handler into this chain. */
   if ((entry->i_link.le_next = *piter) != NULL)
        entry->i_link.le_next->i_link.le_pself = &entry->i_link.le_next;
  }
  (*piter = entry)->i_link.le_pself = piter;
 }

#ifdef CONFIG_SMP
 atomic_rwlock_endwrite(&inttab_lock);
 /* TODO: Send an RPC command to all CPUs apart of `affinity' */
 if (CPU_ISSET(THIS_CPU->c_id,affinity))
#endif
 {
  receive_rpc_update_idt(entry->i_intno);
 }
done:
 PREEMPTION_POP(was);
 /* Delete any interrupts that were overwritten. */
 while (delete_chain) {
  entry = delete_chain->i_link.le_next;
  interrupt_delete(delete_chain);
  delete_chain = entry;
 }
 return result;
err_eperm:  result = -EPERM; goto end;
err_eexist: result = -EEXIST; /*goto end;*/
end: IFDEF_SMP(atomic_rwlock_endwrite(&inttab_lock)); goto done;
}

PUBLIC ssize_t KCALL
int_del(struct interrupt *__restrict entry) {
 ssize_t result = 0;
 pflag_t was = PREEMPTION_PUSH();
 IFDEF_SMP(atomic_rwlock_write(&inttab_lock));

 PANIC("Not implemented");

 IFDEF_SMP(atomic_rwlock_endwrite(&inttab_lock));
 PREEMPTION_POP(was);
 return result;
}

INTERN void KCALL
irq_delete_from_instance(struct instance *__restrict inst) {
 /* TODO */
}


#ifdef CONFIG_SMP
PUBLIC int KCALL
int_register(struct interrupt const *__restrict entry) {
 cpu_set_t affinity; CPU_ZERO(&affinity);
 return int_register_set(entry,&affinity);
}
PUBLIC int KCALL
int_register_set(struct interrupt const *__restrict entry,
                 cpu_set_t *__restrict affinity)
#else
PUBLIC int KCALL
int_register(struct interrupt const *__restrict entry)
#endif
{
 struct interrupt *copy; ssize_t result;
 copy = (struct interrupt *)memdup(entry,sizeof(struct interrupt));
 if unlikely(!copy) return -ENOMEM;
 copy->i_flags |= INTFLAG_FREEDESCR;
 result = int_add(copy);
 /* Return the interrupt number that was assigned. */
 if (E_ISERR(result)) free(copy);
 else result = copy->i_intno;
 return (int)result;
}


DECL_END

#endif /* !GUARD_KERNEL_ARCH_INTERRUPT_C */
