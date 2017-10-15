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
#ifndef GUARD_KERNEL_LINKER_DRIVER_MODULE_C
#define GUARD_KERNEL_LINKER_DRIVER_MODULE_C 1
#define _KOS_SOURCE 1

#include <dev/device.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/mman.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <linker/patch.h>
#include <sched/task.h>
#include <sys/mman.h>

DECL_BEGIN

/* [lock(mman_kernel.m_lock)] Module load hint to speed up
 *                            installation of new kernel modules. */
PRIVATE ppage_t module_loadhint = (ppage_t)0xe0000000;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

PUBLIC SAFE REF struct instance *KCALL
kernel_insmod(struct module *__restrict mod,
              USER char const *cmdline, u32 mode) {
 REF struct instance *result;
 ppage_t module_base;
 errno_t error;
 CHECK_HOST_DOBJ(mod);
 /* Make sure all memory regions of the module are loaded. */
 error = module_mkregions(mod);
 if (E_ISERR(error)) return E_PTR(error);

 /* Acquire a write-lock to the kernel memory manager. */
 error = mman_write(&mman_kernel);
 if (E_ISERR(error)) return E_PTR(error);
 if ((mode&(INSMOD_REUSE|INSMOD_SECONDARY)) != INSMOD_SECONDARY) {
  /* Search for existing instances of 'mod'. */
  struct instance *iter = mman_kernel.m_inst;
  for (; iter; iter = iter->i_chain.le_next) {
   if (iter->i_module != mod) continue;
   /* Found an existing instance. */
   if (mode&INSMOD_REUSE) {
    result = iter;
    if (!INSTANCE_INCREF(result))
         result = E_PTR(-EPERM);
   } else {
    result = E_PTR(-EEXIST);
   }
   mman_endwrite(&mman_kernel);
   return result;
  }
 }

 /* Create a new driver instance of the module. */
 result = instance_new_driver(mod);
 if unlikely(!result) goto local_nomem;

 module_base = mman_findspace_unlocked(&mman_kernel,module_loadhint,
                                        mod->m_size,PAGESIZE,0,
                                        MMAN_FINDSPACE_ABOVE);
 if unlikely((uintptr_t)module_base < KERNEL_BASE) module_base = PAGE_ERROR;
 if unlikely(module_base == PAGE_ERROR) {
local_nomem:
  error = -ENOMEM;
 } else {
  result->i_base = module_base;
  error = mman_mmap_instance_unlocked(&mman_kernel,result,(u32)-1,
                                       mod->m_flag&MODFLAG_TEXTREL
                                     ? PROT_NOUSER|PROT_WRITE
                                     : PROT_NOUSER);
  /* Update the module load hint to point after the instance we've just mapped. */
  if (E_ISOK(error))
      module_loadhint = (ppage_t)((uintptr_t)module_base+CEIL_ALIGN(mod->m_size,PAGESIZE));
  assert(IS_ALIGNED((uintptr_t)module_base,PAGESIZE));
  assert(IS_ALIGNED((uintptr_t)module_loadhint,PAGESIZE));
 }
 mman_endwrite(&mman_kernel);
 if (E_ISERR(error)) goto err;
 /* At this point we've completely (and successfully) mapped the module at 'result->i_base'. */

 { struct modpatch patch;
   modpatch_init_host(&patch,result,module_loadhint);
   CHECK_HOST_DOBJ(mod->m_ops);
   CHECK_HOST_TEXT(mod->m_ops->o_patch,1);
   /* Patch instance relocations & load dependencies. */
   error = modpatch_patch(&patch);
   modpatch_fini(&patch);
   if (E_ISERR(error)) goto err2;
   /* If mapping the module required text-relocations, restore ~real~ segment protection. */
   if (mod->m_flag&MODFLAG_TEXTREL) {
    error = module_restore_readonly(mod,module_base);
    if (E_ISERR(error)) goto err2;
   }
   /* Inherit the patching hint to speed up the next insmod()-call. */
   module_loadhint = patch.p_dephint;
 }

 syslog(LOG_EXEC|LOG_INFO,"[MOD] Loaded kernel module '%[file]' at %p...%p\n",
        mod->m_file,result->i_base,
       (uintptr_t)result->i_base+mod->m_size-1);

 /* Run the module initialization function.
  * TODO: Module initializers must somehow be able to return error codes...
  * TODO: Must somehow pass 'cmdline' to the driver. */
 instance_callinit(result);

 return result;
err2:
 /* Must set the did-unload flag to allow the instance being munmap()'ed */
 ATOMIC_FETCHOR(result->i_flags,INSTANCE_FLAG_DID_UNLOAD);
 /* Delete the module memory mappings we've created above. */
 task_nointr();
 mman_write(&mman_kernel);
 mman_munmap_unlocked(&mman_kernel,module_base,mod->m_size,
                       MMAN_MUNMAP_TAG,result);
 mman_endwrite(&mman_kernel);
 task_endnointr();
err:
 INSTANCE_DECREF(result);
 return E_PTR(error);
}

#pragma GCC diagnostic pop










PUBLIC SAFE REF struct instance *KCALL
kernel_getmod(struct module *__restrict mod) {
 REF struct instance *result;
 CHECK_HOST_DOBJ(mod);
 task_nointr();
 mman_write(&mman_kernel);
 /* Find & incref() the first instance matching the given module. */
 for (result = mman_kernel.m_inst; result;
      result = result->i_chain.le_next) {
  if (result->i_module == mod &&
      INSTANCE_INCREF(result))
      break;
 }
 mman_endwrite(&mman_kernel);
 task_endnointr();
 return result;
}















INTDEF void KCALL autopart_delete_from_instance(struct instance *__restrict inst);
INTDEF void KCALL fstype_delete_from_instance(struct instance *__restrict inst);
INTDEF void KCALL devns_delete_from_instance(struct devns *__restrict self,
                                             struct instance *__restrict inst);
INTDEF void KCALL irq_delete_from_instance(struct instance *__restrict inst);
INTDEF void KCALL coredump_delete_from_instance(struct instance *__restrict inst);
INTDEF void KCALL modloader_delete_from_instance(struct instance *__restrict inst);
#ifndef CONFIG_NO_NET
INTDEF void KCALL ethandler_delete_from_instance(struct instance *__restrict inst);
#endif /* !CONFIG_NO_NET */
#ifndef CONFIG_NO_JOBS
INTDEF void KCALL jobs_delete_from_instance(struct instance *__restrict inst);
#endif /* !CONFIG_NO_JOBS */

PRIVATE SAFE void KCALL
driver_delete_system_hooks(struct instance *__restrict inst) {
 CHECK_HOST_DOBJ(inst);
 /* Delete kernel hooks such as devices, or filesystem/auto-partition callbacks. */
 autopart_delete_from_instance(inst);
 fstype_delete_from_instance(inst);
 devns_delete_from_instance(&ns_chrdev,inst);
 devns_delete_from_instance(&ns_blkdev,inst);
 irq_delete_from_instance(inst);
 coredump_delete_from_instance(inst);
 modloader_delete_from_instance(inst);
#ifndef CONFIG_NO_NET
 ethandler_delete_from_instance(inst);
#endif /* !CONFIG_NO_NET */
#ifndef CONFIG_NO_JOBS
 jobs_delete_from_instance(inst);
#endif /* !CONFIG_NO_JOBS */

 /* TODO: Unmount any filesystem superblocks mounted using the given instance. */
}






PRIVATE SAFE errno_t KCALL
kernel_delmod_unlocked(REF struct instance *__restrict inst, u32 mode) {
 ref_t ok_refcnt = 1;
 CHECK_HOST_DOBJ(inst);

 /* Make sure the instance is allowed to be unloaded. */
 if (inst->i_flags&INSTANCE_FLAG_NOUNLOAD && !(mode&DELMOD_FORCE))
     return -EPERM;
 if (inst->i_module->m_size) ++ok_refcnt;
 /* Set the unload flag for the instance, thus preventing
  * anyone from creating new references to it. */
 if (!(ATOMIC_FETCHOR(inst->i_flags,INSTANCE_FLAG_UNLOAD)&
                                   (INSTANCE_FLAG_UNLOAD))) {
  driver_delete_system_hooks(inst);
  /* TODO: Call some kind of module-implemented callback
   *       that can safely be called while the module is
   *       still usable, with the purpose of deleting
   *       custom reference holders such as global device
   *       variables carrying THIS_INSTANCE-references.
   *    >> Call it 'cleanup()'
   */
 }


 /* Check if we can already unload the module safely. */
 if (ATOMIC_READ(inst->i_refcnt) <= ok_refcnt)
     goto can_unload;

 if (mode&DELMOD_DELDEP) {
  /* TODO: Delete module dependencies. */
  /* NOTE: Ignore failure of deleting dependencies when 'DELMOD_IGNORE_DEP' is set. */
  if (ATOMIC_READ(inst->i_refcnt) <= ok_refcnt)
      goto can_unload;
 }

 if (mode&DELMOD_FORCENOW) {
  if unlikely(ATOMIC_READ(inst->i_refcnt) <= ok_refcnt) goto can_unload;
  /* XXX: Try just a few more things? */
  goto can_unload;
 }
 return -EWOULDBLOCK;
can_unload:
 { ref_t refcnt = ATOMIC_READ(inst->i_refcnt);
   REF struct module *mod = inst->i_module;
   MODULE_INCREF(mod);
   if (!(mode&DELMOD_NOFINI)) {
    /* Call the module finalizers, if allowed to. */
    if (!(ATOMIC_FETCHOR(inst->i_flags,INSTANCE_FLAG_DID_FINI)&
                                      (INSTANCE_FLAG_DID_FINI))) {
     instance_callfini(inst);
     assertf(ATOMIC_READ(inst->i_refcnt) <= refcnt,
             "Module destructors in '%[file]' shouldn't be able to add new "
             "references because 'INSTANCE_FLAG_UNLOAD' is set (i_refcnt = %#x)",
             mod->m_file,ATOMIC_READ(inst->i_refcnt));
     refcnt = ATOMIC_READ(inst->i_refcnt);
    }
   }
   /* Log an error message if the reference counter is still too great. */
   if (refcnt > ok_refcnt) {
    syslog(LOG_EXEC|LOG_ERROR,
           "[MOD] DANGER: Unloading module %.?q (from '%[file]') despite non-zero use counter of %d\n",
           mod->m_name->dn_size,
           mod->m_name->dn_name,
           mod->m_file,refcnt-ok_refcnt);
   }
   assert(!(inst->i_flags&INSTANCE_FLAG_DID_UNLOAD));
   inst->i_flags |= INSTANCE_FLAG_DID_UNLOAD;
   COMPILER_WRITE_BARRIER();

   mman_munmap_unlocked(&mman_kernel,inst->i_base,inst->i_module->m_size,
                         MMAN_MUNMAP_TAG,inst);
   assertf(ATOMIC_READ(inst->i_refcnt) != 0,"Was the caller really holding a reference?");
   assert(ATOMIC_READ(inst->i_branch) == 0);
   ATOMIC_WRITE(inst->i_refcnt,0);
   instance_destroy(inst);
   syslog(LOG_EXEC|LOG_INFO,"[MOD] Unloaded module %.?q from '%[file]'\n",
          mod->m_name->dn_size,mod->m_name->dn_name,mod->m_file);
   MODULE_DECREF(mod);
 }
 return -EOK;
}




PUBLIC SAFE ssize_t KCALL
kernel_delmod_m(struct module *__restrict mod, u32 mode) {
 size_t result = 0; struct instance *iter,*next;
 ssize_t error = mman_write(&mman_kernel);
 if (E_ISERR(error)) return error;
 iter = mman_kernel.m_inst;
 while (iter) {
  next = iter->i_chain.le_next;
  if (iter->i_module == mod) {
   error = kernel_delmod_unlocked(iter,mode);
   if (E_ISERR(error)) goto end;
   ++result;
  }
  iter = next;
 }
 error = (ssize_t)result;
end:
 mman_endwrite(&mman_kernel);
 return error;
}
PUBLIC SAFE errno_t KCALL
kernel_delmod(REF struct instance *__restrict inst, u32 mode) {
 errno_t error = mman_write(&mman_kernel);
 if (E_ISERR(error)) return error;
 error = kernel_delmod_unlocked(inst,mode);
 mman_endwrite(&mman_kernel);
 return error;
}



PRIVATE ATTR_COLDRODATA
u32 const delmod_cleanup_modes[] = {
 /* Mode combinations used for module cleanup (ranked from least to most severe). */
 DELMOD_NOBLOCK,
 DELMOD_NOBLOCK|DELMOD_DELDEP,
 DELMOD_NOBLOCK|DELMOD_FORCE,
 DELMOD_NOBLOCK|DELMOD_FORCE|DELMOD_DELDEP,
#define DELMOD_CLEANUP_MODEOK(i) ((i) < 4)
 DELMOD_BLOCK|DELMOD_FORCENOW,
 DELMOD_BLOCK|DELMOD_FORCENOW|DELMOD_DELDEP,
 DELMOD_BLOCK|DELMOD_FORCENOW|DELMOD_FORCE,
 DELMOD_BLOCK|DELMOD_FORCENOW|DELMOD_FORCE|DELMOD_DELDEP,
 DELMOD_BLOCK|DELMOD_FORCENOW|DELMOD_IGNORE_DEP,
 DELMOD_BLOCK|DELMOD_FORCENOW|DELMOD_IGNORE_DEP|DELMOD_FORCE,
};


PUBLIC ATTR_COLDTEXT SAFE void
KCALL kernel_unload_all_modules(void) {
 struct instance *iter,*next; int level = 0;
 size_t last_fail_count = 0,fail_count; u32 mode;
 task_nointr();
 mman_write(&mman_kernel);
again:
 fail_count = 0;
 mode = delmod_cleanup_modes[level];
rechain:
 iter = mman_kernel.m_inst;
 while (iter) {
  next = iter->i_chain.le_next;
  if (iter != THIS_INSTANCE &&
      ATOMIC_INCIFNONZERO(iter->i_refcnt)) {
   if (E_ISOK(kernel_delmod_unlocked(iter,mode)))
       goto rechain;
   INSTANCE_DECREF(iter);
   ++fail_count;
  }
  iter = next;
 }
 if (fail_count) {
  if (last_fail_count == fail_count) {
   /* We're not getting anywhere here... -> Lets get more drastic! */
   ++level;
   if unlikely(level == COMPILER_LENOF(delmod_cleanup_modes)) {
    syslog(LOG_EXEC|LOG_ERROR,"[MOD] Failed to unload %Iu kernel modules:\n",level);
    for (iter = mman_kernel.m_inst; iter;
         iter = iter->i_chain.le_next) {
     if (iter != THIS_INSTANCE) {
      syslog(LOG_EXEC|LOG_ERROR,"[MOD] Module %.?q (from '%[file]')\n",
             iter->i_module->m_name->dn_size,
             iter->i_module->m_name->dn_name,
             iter->i_module->m_file);
     }
    }
    goto done;
   } else if (!DELMOD_CLEANUP_MODEOK(level)) {
    syslog(LOG_EXEC|LOG_WARN,
           "[MOD] Failed to unload %Iu kernel module%s. - Entering level #%d (mode %#.8I32x)\n",
           fail_count,fail_count > 1 ? "s" : "",level,delmod_cleanup_modes[level]);
   }
  }
  last_fail_count = fail_count;
  goto again;
 }
done:
 mman_endwrite(&mman_kernel);
 task_endnointr();
}

DECL_END

#endif /* !GUARD_KERNEL_LINKER_DRIVER_MODULE_C */
