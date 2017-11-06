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
#ifdef __INTELLISENSE__
#include "ldt.c.inl"
#define NEW
#endif

#ifndef LDT_HELPERS_DEFINED
#define LDT_HELPERS_DEFINED 1
PRIVATE ldt_t KCALL
ldt_find_empty_slot(struct ldt const *__restrict self) {
 struct segment *iter,*end;
 CHECK_HOST_DOBJ(self);
 assert(ldt_reading(self));
 iter = self->l_idt.ip_ldt;
 end  = (struct segment *)((byte_t *)iter+self->l_idt.ip_limit);
 for (; iter < end; ++iter) {
  if (!iter->u) {
   /* Found an empty slot! */
   return (ldt_t)((byte_t *)iter-
                  (byte_t *)self->l_idt.ip_ldt);
  }
 }
 return LDT_ERROR;
}
#endif


#ifdef NEW
FUNDEF s32 KCALL
mman_newldt_unlocked(struct mman *__restrict self, struct segment seg)
#else
FUNDEF errno_t KCALL
mman_setldt_unlocked(struct mman *__restrict self, ldt_t id,
                     struct segment seg,
                     struct segment *__restrict oldseg)
#endif
{
 struct ldt *oldt,*nldt;
 errno_t error;
#ifdef NEW
 ldt_t result_index;
#endif
again:
 if ((oldt = self->m_ldt)->l_refcnt > 1) {
  struct task *iter,**piter;
  bool should_destroy;
  /* Other memory managers are using this LDT.
   * We must lazily create a new local copy. */
  nldt = ocalloc(struct ldt);
  if (!nldt) return -ENOMEM;
  atomic_rwlock_cinit(&nldt->l_lock);
  ldt_write(nldt);
  nldt->l_refcnt = 1;
#ifdef CONFIG_SMP
  CPU_ZERO(&nldt->l_valid);
#endif /* CONFIG_SMP */

  /* Acquire read-access to the LDT we're trying to copy. */
  ldt_read(oldt);
  nldt->l_idt.ip_limit = oldt->l_idt.ip_limit;
#ifdef NEW
  result_index = ldt_find_empty_slot(oldt);
  /* Allocate a new entry if we didn't find an empty one */
  if (result_index == LDT_ERROR) {
   result_index = nldt->l_idt.ip_limit;
   nldt->l_idt.ip_limit += sizeof(struct segment);
  }
#endif
  nldt->l_idt.ip_ldt = (struct segment *)kmalloc(nldt->l_idt.ip_limit,
                                                 GFP_SHARED);
  if unlikely(!nldt->l_idt.ip_ldt) {
   error = -ENOMEM;
err_copy:
   ldt_endread(oldt);
   free(nldt);
   return error;
  }
  memcpy(nldt->l_idt.ip_ldt,oldt->l_idt.ip_ldt,
         oldt->l_idt.ip_limit);
 
  /* Put the new segment into the LDT vector. */
#ifdef NEW
  memcpy((byte_t *)nldt->l_idt.ip_ldt+result_index,
         (byte_t *)&seg,sizeof(struct segment));
#else
  assertf(id/sizeof(struct segment) <
          nldt->l_idt.ip_limit/sizeof(struct segment),
          "id = %I16x(%I16u)",id,id);
  assertf((id % sizeof(struct segment)) == 0,
          "id = %I16x(%I16u)",id,id);
  memcpy((byte_t *)nldt->l_idt.ip_ldt+id,
         (byte_t *)&seg,sizeof(struct segment));
#endif
  /* Allocate a new GDT entry for this LDT */
  nldt->l_gdt = gdt_new();
  if unlikely(nldt->l_gdt == SEG_NULL) {
   error = -ENOMEM;
   goto err_copy;
  }

  piter = &oldt->l_tasks;
  while ((iter = *piter) != NULL) {
   assert(piter == iter->t_arch.at_ldt_tasks.le_pself);
   if (iter->t_real_mman == self) {
    /* Suspend all tasks using us as memory manager. */
    if (iter != THIS_TASK) {
     error = task_suspend(iter,TASK_SUSP_HOST);
     if (E_ISERR(error)) {
      /* Resume all tasks that we've suspended. */
err_delgdt:
      LDT_FOREACH_TASK(iter,nldt) {
       if (iter != THIS_TASK)
           task_resume(iter,TASK_SUSP_HOST);
      }
      gdt_del(nldt->l_gdt);
      goto err_copy;
     }
    }
    /* Transfer all tasks to the new ldt */
    *piter = iter->t_arch.at_ldt_tasks.le_next;
    /* Insert the task into the copy's list. */
    LIST_INSERT(nldt->l_tasks,iter,t_arch.at_ldt_tasks);
    continue;
   }
   piter = &iter->t_arch.at_ldt_tasks.le_next;
  }

  { struct segment seg;
    seg = make_segment((uintptr_t)nldt->l_idt.ip_ldt,
                       (uintptr_t)nldt->l_idt.ip_limit,
                        SEG_LDT);
#ifndef CONFIG_SMP
    error = gdt_set(nldt->l_gdt,seg,NULL);
    if (E_ISERR(error)) goto err_delgdt;
#else /* !CONFIG_SMP */
    /* Update the GDT within all associated CPUs */
    LDT_FOREACH_TASK(iter,nldt) {
     struct cpu *c = iter->t_cpu;
     cpuid_t target_id = c->c_id;
     if (!CPU_ISSET(target_id,&nldt->l_valid)) {
      CPU_SET(target_id,&nldt->l_valid);
      error = vgdt_set(c,nldt->l_gdt,seg,NULL);
      if (E_ISERR(error)) {
       struct task *iter2;
       /* Undo all changes */
       seg = make_segment((uintptr_t)oldt->l_idt.ip_ldt,
                          (uintptr_t)oldt->l_idt.ip_limit,
                           SEG_LDT);
       /* Restore the original segment in all affected tasks. */
       LDT_FOREACH_TASK(iter2,nldt) {
        if (iter2 == iter) break;
        vgdt_set(c,nldt->l_gdt,seg,NULL);
       }
       goto err_delgdt;
      }
     }
    }
#endif /* CONFIG_SMP */
  }
  /* Decref() the old LDT.
   * NOTE: Due to race conditions it is possible that we'll have to inherit it anyways... */
  should_destroy = ATOMIC_DECFETCH(oldt->l_refcnt) == 0;
  assert(nldt->l_refcnt);
  ldt_endread(oldt);
  if unlikely(should_destroy) ldt_destroy(oldt);

  /* Override the existing LDT entry with the copy we've just created. */
  self->m_ldt = nldt;
  COMPILER_WRITE_BARRIER();
  ldt_downgrade(nldt);

  /* Resume all tasks that were suspended and
   * update their LDT->GDT index caches. */
  { bool i_am_apart = false;
    LDT_FOREACH_TASK(iter,nldt) {
     ATOMIC_WRITE(iter->t_arch.at_ldt_gdt,nldt->l_gdt);
     if (iter != THIS_TASK)
         task_resume(iter,TASK_SUSP_HOST);
     else i_am_apart = true;
    }
    /* If the calling thread was apart of the set,
     * we must make sure to update them as well! */
    if (i_am_apart)
        __asm__ __volatile__("lldt %w0\n" : : "g" (nldt->l_gdt));
  }
  ldt_endread(nldt);
 } else {
  struct task *iter,*iter2;
#ifdef NEW
  struct segment *new_vector;
  bool must_suspend = false;
#else
  /* Must suspend all task using this LDT when modifying existing entries! */
#define must_suspend  true
#endif
  ldt_write(oldt);
  COMPILER_READ_BARRIER();
  /* The following can happen if another thread fork()'ed before we got here. */
  if unlikely(oldt->l_refcnt > 1) {
   ldt_endwrite(oldt);
   goto again;
  }
  /* No other memory manager is using this LDT.
   * That means we are allowed to modify it's vector to our liking! */
#ifdef NEW
  result_index = ldt_find_empty_slot(oldt);
  /* Allocate a new entry if we didn't find an empty one */
  if (result_index == LDT_ERROR) {
   result_index = oldt->l_idt.ip_limit;
   oldt->l_idt.ip_limit += sizeof(struct segment);
   /* Must relocate the LDT vector to allocate a new entry. */
   new_vector = oldt->l_idt.ip_ldt ? (struct segment *)realloc_in_place(oldt->l_idt.ip_ldt,
                                                                        oldt->l_idt.ip_limit)
                                   : (struct segment *)NULL;
   if unlikely(!new_vector) {
    /* Must allocate a whole new vector. */
    new_vector = (struct segment *)kmalloc(oldt->l_idt.ip_limit,GFP_SHARED);
    if unlikely(!new_vector) { ldt_endwrite(oldt); return -ENOMEM; }
   }
  }
#endif

  if (must_suspend) {
   /* Suspend all affected tasks. */
   LDT_FOREACH_TASK(iter,oldt) {
    if (iter != THIS_TASK) {
     error = task_suspend(iter,TASK_SUSP_HOST);
     if (E_ISERR(error)) {
      LDT_FOREACH_TASK(iter2,oldt) {
       if (iter2 == iter) break;
       task_resume(iter,TASK_SUSP_HOST);
       ldt_endwrite(oldt);
       return error;
      }
     }
    }
   }
  }

#ifdef NEW
  /* Must the LDT vector. */
  if (new_vector != oldt->l_idt.ip_ldt) {
   struct segment seg;
   seg = make_segment((uintptr_t)oldt->l_idt.ip_ldt,
                      (uintptr_t)oldt->l_idt.ip_limit,
                       SEG_LDT);
#ifndef CONFIG_SMP
   /* Reload the LDT. */
   error = gdt_set(oldt->l_gdt,seg,NULL);
   if (E_ISERR(error)) {
    /* Undo all changes */
    oldt->l_idt.ip_limit -= sizeof(struct segment);
    ldt_downgrade(oldt);
    free(new_vector);
    LDT_FOREACH_TASK(iter,oldt) {
     if (iter != THIS_TASK)
         task_resume(iter,TASK_SUSP_HOST);
    }
    ldt_endread(oldt);
    return error;
   }
#else
   /* Reload the LDT on all affected CPUs. */
   CPU_ZERO(&oldt->l_valid);
   LDT_FOREACH_TASK(iter,oldt) {
    struct cpu *c = iter->t_cpu;
    cpuid_t target_id = c->c_id;
    if (!CPU_ISSET(target_id,&oldt->l_valid)) {
     CPU_SET(target_id,&oldt->l_valid);
     error = vgdt_set(c,oldt->l_gdt,seg,NULL);
     if (E_ISERR(error)) {
      /* Undo all changes */
      oldt->l_idt.ip_limit -= sizeof(struct segment);
      ldt_downgrade(oldt);
      seg = make_segment((uintptr_t)oldt->l_idt.ip_ldt,
                         (uintptr_t)oldt->l_idt.ip_limit,
                          SEG_LDT);
      /* Restore the original segment in all affected tasks. */
      LDT_FOREACH_TASK(iter2,oldt) {
       if (iter2 == iter) break;
       vgdt_set(c,oldt->l_gdt,seg,NULL);
      }
      free(new_vector);
      LDT_FOREACH_TASK(iter,oldt) {
       if (iter != THIS_TASK)
           task_resume(iter,TASK_SUSP_HOST);
      }
      ldt_endread(oldt);
      return error;
     }
    }
   }
#endif
   /* Install the new vectors. */
   free(oldt->l_idt.ip_ldt);
   oldt->l_idt.ip_ldt = new_vector;
  }
#endif

  /* With the new vector installed, we can now update its contents. */
#ifdef NEW
  memcpy((byte_t *)oldt->l_idt.ip_ldt+result_index,
         (byte_t *)&seg,sizeof(struct segment));
#else
  assert(id/sizeof(struct segment) <
         oldt->l_idt.ip_limit/sizeof(struct segment));
  assert((id % sizeof(struct segment)) == 0);
  memcpy((byte_t *)oldt->l_idt.ip_ldt+id,
         (byte_t *)&seg,sizeof(struct segment));
#endif

  /* The LDT has been updated and now we can safely
   * resume all tasks we've previously suspended. */
  if (must_suspend) {
   /* Resume all task that were suspended before. */
   LDT_FOREACH_TASK(iter,oldt) {
    if (iter != THIS_TASK)
        task_resume(iter,TASK_SUSP_HOST);
   }
  }
  ldt_endwrite(oldt);
#undef must_suspend
 }
#ifdef NEW
 return (s32)result_index;
#else
 return -EOK;
#endif
}
#undef NO_MEMORY
#ifdef NEW
#undef NEW
#endif
