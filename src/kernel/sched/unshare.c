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
#ifndef GUARD_KERNEL_SCHED_UNSHARE_C
#define GUARD_KERNEL_SCHED_UNSHARE_C 1
#define _KOS_SOURCE 1

#include <fcntl.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/mman.h>
#include <kernel/stack.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <sched/paging.h>
#include <sched/task.h>
#include <sched/types.h>
#include <string.h>
#include <sys/mman.h>

#include "../mman/intern.h"

DECL_BEGIN

INTERN errno_t KCALL
dup_mbranch(struct mman *__restrict self,
            struct mman *__restrict old_mm,
            struct mbranch const *__restrict old_branch) {
 errno_t error; rsize_t branch_size;
 struct mbranch *branch_copy;
 CHECK_HOST_DOBJ(self);
again:
 CHECK_HOST_DOBJ(old_branch);
 assert(IS_ALIGNED(MBRANCH_BEGIN(old_branch),PAGESIZE));
 assert(IS_ALIGNED(MBRANCH_END(old_branch),PAGESIZE));
 assert(MBRANCH_MIN(old_branch) < MBRANCH_MAX(old_branch));
 if (old_branch->mb_prot&PROT_LOOSE) goto loose_branch;
 /* Check if this branch should be duplicated. */
 branch_size = MBRANCH_SIZE(old_branch);
 assert(IS_ALIGNED(old_branch->mb_start,PAGESIZE));
 assert(IS_ALIGNED(branch_size,PAGESIZE));

 if (old_branch->mb_notify) {
  error = (errno_t)(*old_branch->mb_notify)(MNOTIFY_UNSHARE_DROP,
                                            old_branch->mb_closure,old_mm,
                                           (ppage_t)MBRANCH_BEGIN(old_branch),
                                            branch_size);
  if (E_ISERR(error)) return error;
  if (error > 0) goto loose_branch;
 }
 /* Copy this branch. */
 branch_copy = (struct mbranch *)kmalloc(sizeof(struct mbranch),MMAN_UNIGFP);
 if unlikely(!branch_copy) return -ENOMEM;
 error = mregion_incref(old_branch->mb_region,
                        old_branch->mb_start,
                        branch_size);
 if (E_ISERR(error)) { err: kfree(branch_copy); return error; }

 branch_copy->mb_node.a_vmin = old_branch->mb_node.a_vmin;
 branch_copy->mb_node.a_vmax = old_branch->mb_node.a_vmax;
 branch_copy->mb_prot        = old_branch->mb_prot;
 branch_copy->mb_start       = old_branch->mb_start;
 branch_copy->mb_region      = old_branch->mb_region; /* Inherit reference. */
 branch_copy->mb_notify      = old_branch->mb_notify;
 branch_copy->mb_closure     = old_branch->mb_closure;
 if (branch_copy->mb_notify != NULL) {
  /* Signal the branch duplication. */
  /* ............ */ error = (errno_t)(*branch_copy->mb_notify)(MNOTIFY_INCREF,branch_copy->mb_closure,self,0,0);
  if (E_ISOK(error)) error = (errno_t)(*branch_copy->mb_notify)(MNOTIFY_CLONE,branch_copy->mb_closure,old_mm,
                                                               (ppage_t)MBRANCH_BEGIN(branch_copy),branch_size);
  if (E_ISOK(error)) error = (errno_t)(*branch_copy->mb_notify)(MNOTIFY_RESTORE,branch_copy->mb_closure,self,
                                                               (ppage_t)MBRANCH_BEGIN(branch_copy),branch_size);
  if (E_ISERR(error)) {
err_remap:
   task_nointr();
   mregion_decref(old_branch->mb_region,
                  old_branch->mb_start,
                  branch_size);
   task_endnointr();
   goto err;
  }
 }

 /* Remap the new and old branches. */
 error = mbranch_remap(branch_copy,&self->m_pdir,true);
 if (E_ISERR(error)) goto err_remap;
 error = mbranch_remap(old_branch,&old_mm->m_pdir,true);
 if (E_ISERR(error)) goto err_remap;

 /* Insert the branch copy into the memory manager. */
 mman_insbranch_unlocked(self,branch_copy);

check_more:
 if (old_branch->mb_node.a_min) {
  if (old_branch->mb_node.a_max) {
   error = dup_mbranch(self,old_mm,old_branch->mb_node.a_max);
   if (E_ISERR(error)) return error;
  }
  old_branch = old_branch->mb_node.a_min;
  goto again;
 }
 if (old_branch->mb_node.a_max) {
  old_branch = old_branch->mb_node.a_max;
  goto again;
 }
 return -EOK;
loose_branch:
 /* Delete the page-directory mapping for
  * this branch in the new mman's directory. */
#if 1
 syslog(LOG_DEBUG,"Deleting branch %p...%p\n",
        MBRANCH_MIN(old_branch),MBRANCH_MAX(old_branch));
#endif
 error = pdir_munmap(&self->m_pdir,
                    (ppage_t)MBRANCH_BEGIN(old_branch),
                     MBRANCH_SIZE(old_branch),
                     PDIR_FLAG_NOFLUSH);
 if (E_ISERR(error)) return error;
 goto check_more;
}


INTERN errno_t KCALL
mman_init_copy_unlocked(struct mman *__restrict nm,
                        struct mman *__restrict om) {
 errno_t error = -EOK;
 if likely(om->m_map) {
  struct instance *iter,*inst_copy,**pdst;
  error = dup_mbranch(nm,om,om->m_map);
  if (E_ISERR(error)) goto err;
  /* With the branch map cloned, we must manually copy
   * all instances loaded into the old memory manger,
   * while replacing all references to them in the new one. */
  pdst = &nm->m_inst;
  assert(!*pdst);
  /* Must check if there is still a map, in case _everything_ was marked with `PROT_LOOSE' */
  if likely(nm->m_map) MMAN_FOREACH_INST(iter,om) {
   struct mbranch *branch;
   assertf(!INSTANCE_INKERNEL(iter),
           "Driver module %[file] loaded into non-kernel memory manager",
           iter->i_module->m_file);
   inst_copy = (struct instance *)kmalloc(offsetof(struct instance,i_driver),
                                          MMAN_UNIGFP);
   if unlikely(!inst_copy) {err_nomem2: *pdst = NULL; goto err_nomem; }
   inst_copy->i_branch = 0; /* Will be fixed later. */
   MMAN_FOREACH(branch,nm) {
    if (branch->mb_notify  == &instance_mnotify &&
        branch->mb_closure == iter) {
#if 0
     syslog(LOG_DEBUG,"Updating instance branch %p...%p for %[file]\n",
            MBRANCH_MIN(branch),MBRANCH_MAX(branch),
            iter->i_module->m_file);
#endif
     instance_mnotify(MNOTIFY_DECREF,iter,nm,0,0);
     /* No need to use atomic instructions here. - The branch isn't yet visible. */
     ++inst_copy->i_branch;
     branch->mb_closure = inst_copy;
    }
   }

   if unlikely(!inst_copy->i_branch) {
    /* Shouldn't happen, but could if a module memory is re-mapped with `PROT_LOOSE',
     * causing it not to be copied above and leading to the module instance itself
     * being dropped during the process of unsharing. */
    kfree(inst_copy);
   } else {
    iter->i_temp = inst_copy; /* Track the new instance pointers. */
    inst_copy->i_weakcnt = 1;
    inst_copy->i_module  = iter->i_module;
    inst_copy->i_base    = iter->i_base;
    inst_copy->i_refcnt  = 1;
    inst_copy->i_flags   = iter->i_flags;
    /* NOTE: The sets themself are fixed in later. */
    atomic_rwlock_init(&inst_copy->i_used.is_lock);
    atomic_rwlock_init(&inst_copy->i_deps.is_lock);
    inst_copy->i_used.is_setc = iter->i_used.is_setc;
    inst_copy->i_deps.is_setc = iter->i_deps.is_setc;
    if (inst_copy->i_used.is_setc) {
     inst_copy->i_used.is_setv = (WEAK struct instance **)memdup(iter->i_used.is_setv,
                                                                 inst_copy->i_used.is_setc*
                                                                 sizeof(WEAK struct instance *));
     if unlikely(!inst_copy->i_used.is_setv) { inst_copy->i_deps.is_setv = NULL; goto err_nomem2; }
    } else {
     inst_copy->i_used.is_setv = NULL;
    }
    if (inst_copy->i_deps.is_setc) {
     inst_copy->i_deps.is_setv = (WEAK struct instance **)memdup(iter->i_deps.is_setv,
                                                                 inst_copy->i_deps.is_setc*
                                                                 sizeof(WEAK struct instance *));
     if unlikely(!inst_copy->i_deps.is_setv) goto err_nomem2;
    } else {
     inst_copy->i_deps.is_setv = NULL;
    }

    MODULE_INCREF(inst_copy->i_module);
    inst_copy->i_chain.le_pself = pdst;
    *pdst = inst_copy;
    pdst = &inst_copy->i_chain.le_next;
   }
  }
  /* Terminate the chain of module instances associated with this mman. */
  *pdst = NULL;

  /* Fix dependency/used-by instance sets. */
  MMAN_FOREACH_INST(iter,nm) {
   struct instance **fix_iter,**fix_end;
   /* Replace pointers with the new instances. */
   fix_end = (fix_iter = iter->i_used.is_setv)+iter->i_used.is_setc;
   for (; fix_iter != fix_end; ++fix_iter) {
    *fix_iter = (WEAK struct instance *)(*fix_iter)->i_temp;
   }
   fix_end = (fix_iter = iter->i_deps.is_setv)+iter->i_deps.is_setc;
   for (; fix_iter != fix_end; ++fix_iter) {
    *fix_iter = (WEAK struct instance *)(*fix_iter)->i_temp;
   }
  }
 }

#ifndef CONFIG_NO_LDT
 assert(nm->m_ldt == &ldt_empty);
 assert(ldt_empty.l_refcnt >= 2);
 ATOMIC_FETCHDEC(ldt_empty.l_refcnt);
 nm->m_ldt = om->m_ldt;
 assert(nm->m_ldt);
 CHECK_HOST_DOBJ(nm->m_ldt);
 LDT_INCREF(nm->m_ldt);
#endif

 /* Copy environment context and linear allocation hints. */
 nm->m_uheap   = om->m_uheap;
 nm->m_ustck   = om->m_ustck;
 nm->m_environ = om->m_environ;
 nm->m_envsize = om->m_envsize;
 nm->m_envenvc = om->m_envenvc;
 nm->m_envetxt = om->m_envetxt;
err:
 return error;
err_nomem:
 error = -ENOMEM;
 goto err;
}


PUBLIC SAFE errno_t KCALL
task_unshare_mman(bool unmap_old_ustack) {
 struct task *self = THIS_TASK;
 struct mman *om,*nm;
 struct mbranch *branch;
 errno_t error;
#define LOCK_NONE  0
#define LOCK_READ  1
#define LOCK_WRITE 2
 int om_lock = LOCK_NONE;
 assertf(self->t_mman != &mman_kernel,"Cannot unshare kernel memory managers!");
 assertf(self->t_mman == self->t_real_mman,"You must use your real memory manager.");
 /* Switch to the kernel page directory. */
 TASK_PDIR_KERNEL_BEGIN(om);
again:
 { register ref_t refcnt = ATOMIC_READ(om->m_refcnt);
   assert(refcnt != 0);
   if likely(refcnt == 1) return -EOK;
 }
 nm = mman_new();
 if unlikely(!nm) return -ENOMEM;

 if (om_lock < LOCK_READ) {
  /* Clone the page directory & all mappings. */
  error = mman_read(om);
  if (E_ISERR(error)) goto err;
  om_lock = LOCK_READ;
 }

 /* Start with the simple stuff: Clone the page directory. */
 if (!pdir_load_copy(&nm->m_pdir,&om->m_pdir)) goto err_nomem;

 /* OK! The page directory was copied. - Time to move on to the next part!
  * >> The big, ol' mapping table. */
 assertf((om->m_map != NULL) == (om->m_order != NULL),
         "Existance of a map must match existance of the order chain");
 assertf((om->m_map != NULL) || (om->m_inst == NULL),
         "Without a map, there can be no instances");
 assert(!nm->m_map);
 assert(!nm->m_order);
 assert(!nm->m_inst);
 error = mman_init_copy_unlocked(nm,om);
 if (E_ISERR(error)) goto err;

 /* Duplicate the user-space stack descriptor. */
 if unlikely(!self->t_ustack) {
  assert(om_lock == LOCK_READ);
  mman_endread(om);
  om_lock = LOCK_NONE;
 } else {
  struct stack *old_descr;
  struct stack *new_descr;
  old_descr = self->t_ustack;
  new_descr = (struct stack *)kmalloc(sizeof(struct stack),
                                      GFP_SHARED);
  if unlikely(!new_descr) goto err_nomem;
  if (unmap_old_ustack) {
   /* Must temporarily update the lock on the old memory manager. (*sigh*) */
   if (om_lock < LOCK_WRITE) {
    error = mman_upgrade(om);
    if (E_ISERR(error)) {
     free(new_descr);
     if (error == -ERELOAD) {
      MMAN_DECREF(nm);
      om_lock = LOCK_WRITE;
      goto again;
     }
     om_lock = LOCK_NONE;
     goto err;
    }
   }
   assert(om_lock == LOCK_WRITE);
   error = (errno_t)mman_munmap_stack_unlocked(om,self->t_ustack);
   if (E_ISERR(error)) { free(new_descr); goto err; }
   mman_endwrite(om);
   om_lock = LOCK_NONE;
  }
  /* WARNING: After this point, no more errors can be handled
   *         (because we may have deleted the old user-stack). */
  new_descr->s_refcnt      = 0;
  new_descr->s_branch      = 0;
  new_descr->s_task.ap_ptr = self;
  new_descr->s_begin       = old_descr->s_begin;
  new_descr->s_end         = old_descr->s_end;
  /* Go through all the branch mappings and
   * update stack entries to the caller's stack.
   * NOTE: Technically, the only stack that should be mapped in the new
   *       mman at this point should be 'old_descr', simply because of
   *       how `stack_mnotify' responds to 'MNOTIFY_UNSHARE_DROP'
   *      (it will want to drop all stack branches but that of the calling task).
   *       But it still doesn't hurt to be careful and also check
   *      'branch->mb_closure != old_descr' below (even though they
   *       should always match for stack branches).
   */
  MMAN_FOREACH(branch,nm) {
   if (branch->mb_notify  != &stack_mnotify ||
       branch->mb_closure != old_descr) continue;
   asserte(ATOMIC_DECFETCH(old_descr->s_refcnt) >= 1);
   branch->mb_closure = new_descr;
   ++new_descr->s_branch;
  }
  /* Drop the user-stack reference owned by the task itself. */
  STACK_DECREF(old_descr);
  assert(!new_descr->s_refcnt);
  if unlikely(!new_descr->s_branch) {
   /* Again: This can happen when the caller's stack was marked as `PROT_LOOSE'.
    *     >> In this case, the task will be left without a stack. */
   self->t_ustack = NULL;
   free(new_descr);
  } else {
   new_descr->s_refcnt = 2; /* One for the branches, +1 for the task. */
   self->t_ustack = new_descr; /* Inherit reference. */
  }
 }

 /* Remove `self' from the chain of tasks in the old memory manager. */
 atomic_rwlock_write(&om->m_tasks_lock);
 LIST_REMOVE(self,t_mman_tasks);
 atomic_rwlock_endwrite(&om->m_tasks_lock);

 /* Set `self' as the sole user of `mm_clone'. */
 nm->m_tasks                 = self;
 self->t_mman_tasks.le_pself = &nm->m_tasks;
 self->t_mman_tasks.le_next  = NULL;

 assert(nm->m_refcnt == 1);

 /* NOTE: Theoretically, the old `mm' may still get destroyed here,
  *       in the event that all of its other uses got the idea to
  *       unshare it at the same time... */
 MMAN_DECREF(om);
 error = -EOK;
 om = nm; /* Inherit reference. */
 goto end;
err_nomem: error = -ENOMEM;
err: MMAN_DECREF(nm);
end:
 switch (om_lock) {
 case LOCK_WRITE: mman_endwrite(om); break;
 case LOCK_READ:  mman_endread(om); break;
 default: break;
 }
 TASK_PDIR_KERNEL_END(om);
 return error;
}


INTERN errno_t KCALL
fdman_duplicate_table_unlocked(struct fdman *__restrict dst,
                               struct fdman *__restrict src) {
 struct fd *iter,*begin,*end;
 assert(fdman_reading(src));
 /* Figure out exactly how long the descriptor table needs to be. */
 iter = (begin = src->fm_vecv)+src->fm_veca;
 while (iter != begin && (FD_SAFE_ISNULL(iter[-1]) ||
                          FD_IS_CLOFORK(iter[-1])))
        --iter;
 dst->fm_veca = (unsigned int)(iter-begin);
 assert(dst->fm_veca >= src->fm_vecc);
 dst->fm_vecc = src->fm_vecc;
 /* Duplicate the descriptor table. */
 if (dst->fm_veca) {
  dst->fm_vecv = (struct fd *)memdup(src->fm_vecv,sizeof(struct fd)*dst->fm_veca);
  if unlikely(!dst->fm_vecv) return -ENOMEM;
 
  /* Close all FD_CLOFORK-descriptors, and incref() the rest. */
  end = (iter = dst->fm_vecv)+dst->fm_veca;
  for (; iter != end; ++iter) {
   if (FD_IS_CLOFORK(*iter)) {
    assert(dst->fm_vecc);
    --dst->fm_vecc; /* Make sure to keep track of actually existing descriptors. */
    memcpy(iter,&fd_invalid,sizeof(struct fd));
   } else {
    FD_SAFE_INCREF(*iter);
   }
  }
 } else {
  dst->fm_vecv = NULL;
 }

 /* Finish up with the noexcept stuff: cwd & root. */
 CHECK_USER_DOBJ(src->fm_cwd);
 CHECK_USER_DOBJ(src->fm_root);
 dst->fm_cwd  = src->fm_cwd;
 dst->fm_root = src->fm_root;
 DENTRY_INCREF(src->fm_cwd);
 DENTRY_INCREF(src->fm_root);

 /* Any finally: Copy hint and limits. */
 dst->fm_umask  = ATOMIC_READ(src->fm_umask);
 dst->fm_hint   = src->fm_hint;
 dst->fm_vecm   = src->fm_vecm;
 dst->fm_fsmask = src->fm_fsmask;
 dst->fm_fsmode = src->fm_fsmode;

 return -EOK;
}


PUBLIC SAFE errno_t KCALL task_unshare_fdman(void) {
 errno_t error;
 struct task *self = THIS_TASK;
 struct fdman *ofd = self->t_fdman;
 REF struct fdman *nfd;
 { register ref_t refcnt = ATOMIC_READ(ofd->fm_refcnt);
   assert(refcnt != 0);
   /* Nothing to do if the fd-manager isn't shared. */
   if likely(refcnt == 1) return -EOK;
 }
 nfd = fdman_new();
 if unlikely(!nfd) return -ENOMEM;

 error = fdman_read(ofd);
 if (E_ISERR(error)) goto err;
 /* Duplicate the descriptor table. */
 error = fdman_duplicate_table_unlocked(nfd,ofd);
 fdman_endread(ofd);
 if (E_ISERR(error)) goto err;

 /* Replace the old file descriptor table with the new one. */
 assert(self == THIS_TASK);
 assert(self->t_fdman == ofd);
 ATOMIC_WRITE(self->t_fdman,nfd); /* Inherit reference. */

 /* Drop a reference from the old descriptor table. */
 FDMAN_DECREF(ofd);

 return error;
err:
 FDMAN_DECREF(nfd);
 return error;
}


PUBLIC SAFE errno_t KCALL task_unshare_sighand(void) {
 /* TODO */
 return -EOK;
}



DECL_END

#endif /* !GUARD_KERNEL_SCHED_UNSHARE_C */
