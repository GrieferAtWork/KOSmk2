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
#ifndef GUARD_INCLUDE_LINKER_DEBUG_C
#define GUARD_INCLUDE_LINKER_DEBUG_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/check.h>
#include <hybrid/section.h>
#include <linker/module.h>
#include <kernel/mman.h>
#include <linker/debug.h>
#include <sync/rwlock.h>
#include <sched/task.h>
#include <fs/file.h>
#include <syslog.h>
#include <assert.h>
#include <string.h>
#include <sched/paging.h>

DECL_BEGIN

/* [lock(moddebug_loader_lock)] List of loaders with magic. */
PRIVATE SLIST_HEAD(struct moddebug_loader) moddebug_loader_ymaglist = NULL;
/* [lock(moddebug_loader_lock)] List of loaders without magic. */
PRIVATE SLIST_HEAD(struct moddebug_loader) moddebug_loader_nmaglist = NULL;
PRIVATE DEFINE_RWLOCK(moddebug_loader_lock);


PUBLIC REF struct moddebug *KCALL
module_create_debug(struct module *__restrict self) {
 REF struct moddebug *result = E_PTR(-ENODATA);
 struct moddebug_loader *iter; ssize_t magsz;
 errno_t error; byte_t magic[MODLOADER_MAX_MAGIC];
 CHECK_HOST_DOBJ(self);
 if unlikely(!module_file(self)) return NULL;
 magsz = file_kpread(self->m_file,magic,MODLOADER_MAX_MAGIC,0);
 if (E_ISERR(magsz)) return E_PTR(magsz);
 if (!magsz) goto end;
 /* Sadly, we need to keep this lock while executing the loaders... */
 error = rwlock_read(&moddebug_loader_lock);
 if (E_ISERR(error)) return E_PTR(error);
 if (magsz) {
  for (iter = moddebug_loader_ymaglist; iter;
       iter = iter->mdl_chain.le_next) {
   if ((size_t)magsz >= iter->mdl_magsz &&
       !memcmp(iter->mdl_magic,magic,iter->mdl_magsz) &&
        INSTANCE_LOCKWEAK(iter->mdl_owner)) {
    /* Found a match! */
    result = (*iter->mdl_loader)(self);
    INSTANCE_DECREF(iter->mdl_owner);
    assert(result);
    if (result != E_PTR(-ENOEXEC)) goto end2;
   }
  }
 }
 /* Check the magic-less loaders. */
 for (iter = moddebug_loader_nmaglist; iter;
      iter = iter->mdl_chain.le_next) {
  if (INSTANCE_LOCKWEAK(iter->mdl_owner)) {
   result = (*iter->mdl_loader)(self);
   INSTANCE_DECREF(iter->mdl_owner);
   assert(result);
   if (result != E_PTR(-ENOEXEC)) goto end2;
  }
 }
end2: rwlock_endread(&moddebug_loader_lock);
end:
 if (E_ISERR(result)) {
  /* Warn about unexpected problems. */
  if (result != E_PTR(-ENODATA)) {
   syslog(LOG_WARN,
          COLDSTR("[MODDEBUG] Failed to load debug information for '%[file]': %[errno]\n"),
          self->m_file,-E_GTERR(result));
  }
  result = NULL;
 }
 /* Make sure we either return NULL, or a debug descriptor. */
 assert(!E_ISERR(result));
 return result;
}


PUBLIC SAFE REF struct moddebug *KCALL
module_debug(struct module *__restrict self) {
 REF struct moddebug *result,*other;
 CHECK_HOST_DOBJ(self);
 atomic_rwptr_read(&self->m_debug);
 result = (REF struct moddebug *)ATOMIC_RWPTR_GET(self->m_debug);
 if (!result && !(self->m_flag&MODFLAG_NODEBUG)) {
  atomic_rwptr_endread(&self->m_debug);

  /* Allocate a new debug controller. */
  result = module_create_debug(self);

  /* Store the newly created controller. */
  atomic_rwptr_write(&self->m_debug);
  other = (REF struct moddebug *)ATOMIC_RWPTR_GET(self->m_debug);
  if (result) {
   if (other) {
    /* Someone else was faster. (use their results) */
use_other:
    MODDEBUG_INCREF(other);
    atomic_rwptr_endwrite(&self->m_debug);
    if (result) MODDEBUG_DECREF(result);
    return other;
   }
   MODDEBUG_INCREF(result); /* Create a secondary reference. */
   ATOMIC_WRITE(self->m_debug.ap_ptr,result); /* Write+unlock. */
   return result;
  } else if (other) {
   goto use_other; /* Shouldn't happen in a consistent system... */
  } else {
   /* No information exists. - Don't attempt to load some the next time! */
   self->m_flag |= MODFLAG_NODEBUG;
   COMPILER_WRITE_BARRIER();
   atomic_rwptr_endwrite(&self->m_debug);
   return NULL;
  }
 }
 /* Create the reference to-be returned. */
 if (result) MODDEBUG_INCREF(result);
 atomic_rwptr_endread(&self->m_debug);
 return result;
}



PUBLIC SAFE void KCALL
moddebug_destroy(struct moddebug *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assertf(!self->md_module,"The debug descriptor is still assigned to a module");
 INSTANCE_WEAK_DECREF(self->md_owner);
 free(self);
}
PUBLIC struct moddebug *KCALL
moddebug_cinit(struct moddebug *self) {
 if (self) {
  assert(self->md_module == NULL);
  assert(self->md_owner == NULL);
  assert(self->md_ops == NULL);
  assert(self->md_refcnt == 0);
  rwlock_cinit(&self->md_lock);
  self->md_refcnt = 1;
 }
 return self;
}

PUBLIC void KCALL
moddebug_setup(struct moddebug *__restrict self,
               struct instance *__restrict owner) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(owner);
 assert(self->md_refcnt >= 1);
 assert(self->md_owner == NULL);
 assertf(self->md_module != NULL,"Forgot to initialize 'md_module'");
 assertf(self->md_ops != NULL,"Forgot to initialize 'md_ops'");
 INSTANCE_WEAK_INCREF(owner);
 self->md_owner = owner;
}


PUBLIC SAFE ssize_t KCALL
moddebug_virtinfo(struct moddebug *__restrict self,
                  struct instance *__restrict inst,
                  VIRT void *addr, USER struct virtinfo *buf,
                  size_t bufsize, u32 flags) {
 ssize_t error;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(inst);
 error = rwlock_write(&self->md_lock);
 if (E_ISERR(error)) return error;
 /* Make sure the debug descriptor is in an active
  * state and implements address information. */
 if (!self->md_module || !self->md_module->m_refcnt ||
     !self->md_ops->mo_virtinfo || !INSTANCE_LOCKWEAK(self->md_owner)) {
  rwlock_endwrite(&self->md_lock);
  return -ENODATA;
 }
 error = (*self->md_ops->mo_virtinfo)(self,inst,addr,buf,bufsize,flags);
 rwlock_endwrite(&self->md_lock);
 INSTANCE_DECREF(self->md_owner);
 return error;
}

PUBLIC SAFE ssize_t KCALL
instance_virtinfo(struct instance *__restrict inst,
                  VIRT void *addr, USER struct virtinfo *buf,
                  size_t bufsize, u32 flags) {
 REF struct moddebug *info; ssize_t result;
 CHECK_HOST_DOBJ(inst);
 info = module_debug(inst->i_module);
 if unlikely(!info) return -ENODATA;
 result = moddebug_virtinfo(info,inst,addr,buf,bufsize,flags);
 MODDEBUG_DECREF(info);
 return result;
}
FUNDEF SAFE ssize_t KCALL
mman_virtinfo(VIRT void *addr, USER struct virtinfo *buf,
              size_t bufsize, u32 flags) {
 ssize_t result; struct mman *mm = THIS_MMAN;
 struct instance *instance_at; bool has_write_lock = false;
 CHECK_HOST_DOBJ(mm);
 if ((uintptr_t)addr >= KERNEL_BASE && mm != &mman_kernel) {
  /* Special case: Load virtual address information in shared memory. */
  THIS_TASK->t_mman = &mman_kernel;
  COMPILER_WRITE_BARRIER();
  PDIR_STCURR(&pdir_kernel);

  /* Call ourselves again, now that we've loaded another memory manager. */
  result = mman_virtinfo(addr,buf,bufsize,flags);

  /* Restore the old page mapping. (`TASK_PDIR_KERNEL_END()') */
  THIS_TASK->t_mman = mm;
  COMPILER_WRITE_BARRIER();
  PDIR_STCURR(mm->m_ppdir);
  return result;
 }

 result = mman_read(mm);
 if (E_ISERR(result)) return result;
scan_again:
 instance_at = mman_instance_at_unlocked(mm,addr);
 if (!instance_at) {
  if (has_write_lock)
       mman_endwrite(mm);
  else mman_endread(mm);
  return -ENODATA;
 }
 if (!INSTANCE_TRYINCREF(instance_at)) {
  /* If we fail to acquire a reference to the instance,
   * still proceed but keep the mman locked. */
  has_write_lock = true;
  result = mman_upgrade(mm);
  if (E_ISERR(result)) {
   if (result == -ERELOAD) goto scan_again;
   return result;
  }
 } else if unlikely(has_write_lock) {
  mman_endwrite(mm);
  has_write_lock = false;
 } else {
  mman_endread(mm);
 }
 result = instance_virtinfo(instance_at,addr,buf,bufsize,flags);
 if (has_write_lock) mman_endwrite(mm);
 else INSTANCE_DECREF(instance_at);
 return result;
}




INTERN void KCALL
moddebug_loader_delete_from_instance(struct instance *__restrict inst) {
 struct moddebug_loader **piter,*iter;
 struct moddebug_loader **plists[2];
 size_t i,num_refs = 0;
 bool has_write_lock = false;
 CHECK_HOST_DOBJ(inst);
 plists[0] = &moddebug_loader_ymaglist;
 plists[1] = &moddebug_loader_nmaglist;
 task_nointr();
 rwlock_read(&moddebug_loader_lock);
restart:
 for (i = 0; i < COMPILER_LENOF(plists); ++i) {
  piter = plists[i];
  while ((iter = *piter) != NULL) {
   if (iter->mdl_owner == inst) {
    if (!has_write_lock) {
     has_write_lock = true;
     if (rwlock_upgrade(&moddebug_loader_lock) == -ERELOAD)
         goto restart;
    }
    /* Delete this hook. */
    *piter = iter->mdl_chain.le_next;
    ++num_refs;
   } else {
    piter = &iter->mdl_chain.le_next;
   }
  }
 }
 if (has_write_lock)
      rwlock_endwrite(&moddebug_loader_lock);
 else rwlock_endread(&moddebug_loader_lock);
 if (num_refs) {
  assert(num_refs >= ATOMIC_READ(inst->i_weakcnt));
  ATOMIC_FETCHSUB(inst->i_weakcnt,num_refs);
 }
 task_endnointr();
}


PUBLIC SAFE void KCALL
moddebug_addloader(struct moddebug_loader *__restrict loader, int mode) {
 CHECK_HOST_DOBJ(loader);
 assert(TASK_ISSAFE());
 assertf(loader->mdl_owner,"No loader owner set (Use 'THIS_INSTANCE')");
 CHECK_HOST_DOBJ(loader->mdl_owner);
 CHECK_HOST_TEXT(loader->mdl_loader,1);
 assert(loader->mdl_magsz <= MODDEBUG_LOADER_MAX_MAGIC);
 task_nointr();
 rwlock_write(&moddebug_loader_lock);

 /* Lets not deal with interrupts for this... */
 if likely(!INSTANCE_ISUNLOADING(loader->mdl_owner)) {
  INSTANCE_WEAK_INCREF(loader->mdl_owner);
  if (loader->mdl_magsz) {
   if (!(mode&MODDEBUG_LOADER_SECONDARY)) {
    /* Override a primary module loader when secondary is disabled. */
    struct moddebug_loader **piter,*iter;
    piter = &moddebug_loader_ymaglist;
    while ((iter = *piter) != NULL) {
     if (iter->mdl_magsz == loader->mdl_magsz &&
        !memcmp(iter->mdl_magic,loader->mdl_magic,loader->mdl_magsz)) {
      struct instance *old_owner;
      /* Overwrite this entry. */
      old_owner                 = iter->mdl_owner;
      loader->mdl_chain.le_next = iter->mdl_chain.le_next;
      *piter                    = loader;
      rwlock_endwrite(&moddebug_loader_lock);
      task_endnointr();
      INSTANCE_WEAK_DECREF(old_owner);
      return;
     }
     piter = &iter->mdl_chain.le_next;
    }
   }
   SLIST_INSERT(moddebug_loader_ymaglist,loader,mdl_chain);
  } else {
   SLIST_INSERT(moddebug_loader_nmaglist,loader,mdl_chain);
  }
 }
 rwlock_endwrite(&moddebug_loader_lock);
 task_endnointr();
}
PUBLIC bool KCALL
moddebug_delloader(struct moddebug_loader *__restrict loader) {
 struct moddebug_loader **piter,*iter;
 bool result = false;
 CHECK_HOST_DOBJ(loader);
 assertf(loader->mdl_owner,"No loader owner set (Use 'THIS_INSTANCE')");
 CHECK_HOST_DOBJ(loader->mdl_owner);
 CHECK_HOST_TEXT(loader->mdl_loader,1);
 assert(ATOMIC_READ(loader->mdl_owner->i_refcnt) >= 1);
 assert(loader->mdl_magsz <= MODLOADER_MAX_MAGIC);
 /* Lets not deal with interrupts for this... */
 task_nointr();
 rwlock_write(&moddebug_loader_lock);
 piter = loader->mdl_magsz ? &moddebug_loader_ymaglist : &moddebug_loader_nmaglist;
 while ((iter = *piter) != NULL) {
  if (iter == loader) {
   /* Unlink this entry. */
   *piter = loader->mdl_chain.le_next;
   result = true;
   goto done;
  }
  piter = &iter->mdl_chain.le_next;
 }
done:
 rwlock_endwrite(&moddebug_loader_lock);
 if (result) INSTANCE_WEAK_DECREF(loader->mdl_owner);
 task_endnointr();
 return result;
}


DECL_END

#endif /* !GUARD_INCLUDE_LINKER_DEBUG_C */
