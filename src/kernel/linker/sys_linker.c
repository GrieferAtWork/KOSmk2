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
#ifndef GUARD_KERNEL_LINKER_SYS_LINKER_C
#define GUARD_KERNEL_LINKER_SYS_LINKER_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/syscall.h>
#include <linker/module.h>
#include <linker/patch.h>
#include <syslog.h>
#include <string.h>
#include <fs/dentry.h>
#include <sched/task.h>
#include <sys/mman.h>
#include <kernel/mman.h>
#include <kernel/user.h>
#include <dlfcn.h>
#include <fs/fd.h>

DECL_BEGIN

PRIVATE SAFE USER void *KCALL
do_dlopen(struct module *__restrict mod, int flags) {
 REF struct instance *inst;
 struct modpatch patch;
 void *result;
 modpatch_init_user(&patch,THIS_MMAN->m_exe);
 /* NOTE: Since 'p_inst' is allowed to be NULL, no
  *       need to handle the case of a missing 'm_exe' */
#if RTLD_DEEPBIND == MODPATCH_FLAG_DEEPBIND
 patch.p_pflags |= flags&MODPATCH_FLAG_DEEPBIND;
#else
 patch.p_pflags |= flags&RTLD_DEEPBIND ? MODPATCH_FLAG_DEEPBIND : 0;
#endif

 /* We make our lives simple by loading the given
  * module as a dependency of the root executable. */
 inst = modpatch_dldep(&patch,mod);
 /* We use instance base addresses as module handles. */
 if (E_ISOK(inst)) {
  /* Increment the instance's dlopen() recursion counter. */
  ATOMIC_FETCHINC(inst->i_openrec);
  result = (void *)((uintptr_t)inst->i_base+mod->m_begin);
 } else {
  result = E_PTR(E_GTERR(inst));
 }

 if (flags&RTLD_GLOBAL && THIS_MMAN->m_exe) {
  if (flags&RTLD_NODELETE) {
   instance_add_dependency(THIS_MMAN->m_exe,inst);
  } else {
   /* TODO: The above is exactly what must happen here, but if we
    *       did this, dlclose() could no longer be used to delete
    *       the module, as it would then have explicit dependencies... */
  }
 }

 modpatch_fini(&patch);

 return result;
}

SYSCALL_DEFINE2(xdlopen,USER char *,name,int,flags) {
 struct dentryname dname; int state;
 REF struct module *mod; void *result;
 task_crit();
 /* Copy 'name' from userspace */
 dname.dn_name = ACQUIRE_FS_STRING(name,&dname.dn_size,&state);
 if (E_ISERR(dname.dn_name)) { result = E_PTR(E_GTERR(dname.dn_name)); goto end; }
 dentryname_loadhash(&dname);

 /* TODO: 'LD_LIBRARY_PATH'? */
 /* TODO: Extended library search paths, as specified by various executables already loaded? */
 mod = module_open_default(&dname,true);
 RELEASE_STRING(dname.dn_name,state);

 if (E_ISERR(mod)) { result = E_PTR(E_GTERR(mod)); goto end; }

 /* Now simply open the module. */
 result = do_dlopen(mod,flags);
 MODULE_DECREF(mod);
end:
 task_endcrit();
 return (syscall_slong_t)result;
}

SYSCALL_DEFINE2(xfdlopen,int,fd,int,flags) {
 REF struct dentry *module_dent;
 REF struct module *mod; void *result;
 task_crit();
 /* Simply request a directory entry from the FD-manager. */
 module_dent = fdman_get_dentry(THIS_FDMAN,fd);
 if (E_ISERR(module_dent)) { result = E_PTR(E_GTERR(module_dent)); goto end; }
 mod = module_open_d(module_dent,false);
 DENTRY_DECREF(module_dent);
 if (E_ISERR(mod)) { result = E_PTR(E_GTERR(mod)); goto end; }

 /* Now simply open the module. */
 result = do_dlopen(mod,flags);
 MODULE_DECREF(mod);
end:
 task_endcrit();
 return (syscall_slong_t)result;
}

SYSCALL_DEFINE1(xdlclose,USER void *,handle) {
 errno_t result;
 struct mman *mm = THIS_MMAN;
 bool has_write_lock = false;
 struct instance *inst;
 task_crit();
 result = mman_read(mm);
 if (E_ISERR(result)) goto end;
search_module:

 result = -EFAULT;

 /* Lookup the module at the given handle address. */
 inst = mman_instance_at_unlocked(mm,handle);
 if unlikely(!inst) goto end2;

 /* Decrement the dlopen()/dlclose() recursion counter.
  * >> Only actually unmap the module when it hits ZERO(0). */
 { ref_t old_counter;
   do {
    old_counter = ATOMIC_READ(inst->i_openrec);
    if (!old_counter) break;
   } while (!ATOMIC_CMPXCH_WEAK(inst->i_openrec,old_counter,old_counter-1));
   result = -EOK;
   /* Handle recursion (Only unmap the instance when the dlopen() counter hits ZERO(0).) */
   if (old_counter != 1) goto end2;
 }

 atomic_rwlock_read(&inst->i_used.is_lock);

 /* Don't unload the module if it is used by other modules.
  * >> If the user ~really~ wants to delete it, they can simply munmap() it! */
 if (inst->i_used.is_setc != 0) {
  atomic_rwlock_endread(&inst->i_used.is_lock);
  goto end2;
 }

 /* Set the unloading flag, thus preventing this instance from
  * being used if another thread is currently trying to open it.
  * NOTE: If the flag is already set, that means that the instance is
  *       currently being unloaded for some reason, in which case we
  *       simply consider it as already unloaded by returning -EFAULT. */
 if (!(ATOMIC_FETCHOR(inst->i_flags,INSTANCE_FLAG_UNLOAD)&INSTANCE_FLAG_UNLOAD)) {
  atomic_rwlock_endread(&inst->i_used.is_lock);
  result = -EFAULT;
  goto end2;
 }

 /* With the unload-flag set, we can release the used-by instance set lock. */
 atomic_rwlock_endread(&inst->i_used.is_lock);

 if (!has_write_lock) {
  has_write_lock = true;
  result = mman_upgrade(mm);
  if (E_ISERR(result)) {
   if (result == -ERELOAD) goto search_module;
   goto end;
  }
 }

 /* Simply unmap the entire instance. */
 assert(has_write_lock);
 result = (errno_t)mman_munmap_unlocked(mm,inst->i_base,inst->i_module->m_size,
                                        MMAN_MUNMAP_TAG,inst);

end2:
 if (has_write_lock)
      mman_endwrite(mm);
 else mman_endread(mm);
end:  task_endcrit();
 return result;
}

SYSCALL_DEFINE2(xdlsym,USER void *,handle,USER char const *,symbol) {
 void *result; int state; char *name;
 struct mman *mm = THIS_MMAN;
 struct instance *inst;
 task_crit();
 /* NOTE: Need a write-lock in case accessing 'symbol' invokes ALOA. */
 result = E_PTR(mman_write(mm));
 if (E_ISERR(result)) goto end;

 if (handle == RTLD_DEFAULT)
  inst = mm->m_exe;
 else {
  /* Lookup the module at the given handle address. */
  inst = mman_instance_at_unlocked(mm,handle);
 }
 if unlikely(!inst) { result = E_PTR(-EFAULT); goto end2; }

 /* Lookup the symbol. */
 name = ACQUIRE_STRING(symbol,&state);
 if (E_ISERR(name)) result = E_PTR(E_GTERR(name));
 else {
  result = instance_dlsym(inst,name,sym_hashname(name));
  RELEASE_STRING(name,state);
 }

end2: mman_endwrite(mm);
end:  task_endcrit();
 return (syscall_slong_t)result;
}


DECL_END

#endif /* !GUARD_KERNEL_LINKER_SYS_LINKER_C */
