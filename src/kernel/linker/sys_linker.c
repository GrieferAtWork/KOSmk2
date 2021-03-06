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

#include <dlfcn.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>
#include <kernel/mman.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <linker/module.h>
#include <linker/patch.h>
#include <linker/debug.h>
#include <sched/task.h>
#include <string.h>
#include <sys/mman.h>
#include <syslog.h>
#include <kernel/interrupt.h>
#include <arch/cpustate.h>
#include <sched/cpu.h>
#include <arch/current_context.h>
#if defined(__x86_64__) || defined(__i386__)
#include <asm/instx.h>
#endif

DECL_BEGIN

#define DID_INIT  INSTANCE_FLAG_RESERVED0
#define DID_FINI  INSTANCE_FLAG_DID_FINI


struct dl_saved_registers {
#if defined(__x86_64__) || defined(__i386__)
 struct gpregs gp;
 __COMMON_REG2(flags);
 __COMMON_REG2(ip);
#else
 struct cpustate state;
#endif
};

GLOBAL_ASM(
L(.section .text.user                                                            )
/* Pushed as the last function to restore registers and return
 * to the caller after the xdlopen() system call was executed.
 * @ENTRY: ESP is `struct dl_saved_registers *' */
L(PRIVATE_ENTRY(dl_restore_regs)                                                 )
#if defined(__x86_64__) || defined(__i386__)
L(    __ASM_POP_GPREGS /* gpregs */                                              )
L(    popfx            /* xflags */                                              )
L(    ret              /* xip */                                                 )
#elif defined(__arm__)
L(    /* TODO */                                                                 )
#else
#error "ERROR: Unsupported architecture."
#endif
L(SYM_END(dl_restore_regs)                                                       )
L(.previous                                                                      )
);

INTDEF byte_t dl_restore_regs[];

struct init_closure {
 void HOST       *pbase_return_value;
 struct cpustate *state;
};

PRIVATE ssize_t KCALL
dl_do_pushinit(VIRT void *pfun,
               modfun_t UNUSED(single_type),
               void *closure) {
 struct init_closure *arg = (struct init_closure *)closure;
 byte_t USER *user_stack = (byte_t USER *)get_current_usersp();
 if (arg->pbase_return_value != (void *)-1) {
  USER struct dl_saved_registers *regs;
  /* Special handling after the last initializer: restore registers. */
  user_stack -= sizeof(struct dl_saved_registers);
  regs = (struct dl_saved_registers *)user_stack;
#if defined(__x86_64__) || defined(__i386__)
  regs->xip    = arg->state->iret.xip;
  regs->xflags = arg->state->iret.xflags;
  memcpy(&regs->gp,&arg->state->gp,sizeof(struct gpregs));
#ifndef __x86_64__
  regs->gp.esp = arg->state->iret.useresp;
#endif
  /* Return the base address of the first module to the original caller. */
  GPREGS_SYSCALL_RET1(regs->gp) = (uintptr_t)arg->pbase_return_value;
#else
  memcpy(&regs->state,&arg->state,sizeof(struct cpustate));
  /* Return the base address of the first module to the original caller. */
  GPREGS_SYSCALL_RET1(regs->state.gp) = (uintptr_t)arg->pbase_return_value;
#endif

  
  /* Return to this special location. */
  set_current_userip((void *)dl_restore_regs);
  arg->pbase_return_value = (void *)-1;
 }

 /* Now just push the given callback onto the user-stack. */
 user_stack -= sizeof(USER void *);
 *(USER void **)user_stack = get_current_userip();
 set_current_useripsp(pfun,user_stack);

 return 0;
}

PRIVATE ssize_t KCALL
dl_do_loadinit(struct instance *__restrict inst,
               struct init_closure *arg) {
 struct instance **begin,**end;
 ssize_t (KCALL *pmodfun)(struct instance *__restrict self,
                          modfun_t types, penummodfun callback, void *closure);
 /* Make sure to only run initializers once. */
 if (ATOMIC_FETCHOR(inst->i_flags,DID_INIT)&DID_INIT)
     return 0;
 pmodfun = inst->i_module->m_ops->o_modfun;
 if (pmodfun) (*pmodfun)(inst,MODFUN_INIT|MODFUN_REVERSE,
                        &dl_do_pushinit,arg);

 /* Recursively load initializers for dependencies. */
 end = (begin = inst->i_deps.is_setv)+inst->i_deps.is_setc;
 while (end-- != begin) dl_do_loadinit(*end,arg);

 return 0;
}

PRIVATE errno_t KCALL
dl_loadinit(struct instance *__restrict inst,
            struct init_closure *arg) {
 errno_t error; struct mman *mm = THIS_MMAN;
 void *safed_ip,*safed_sp;
 /* Disable preemption to prevent the signal delivery from
  * interfering with us modifying system-call return information. */
 pflag_t was = PREEMPTION_PUSH();
 get_current_useripsp(&safed_ip,&safed_sp);

 /* Make sure to lock the memory manager to prevent
  * changes to the instance's dependency tree. */
 error = mman_write(mm);
 if (E_ISERR(error)) return error;
 error = (errno_t)call_user_worker(&dl_do_loadinit,2,inst,arg);
 mman_endwrite(mm);
 if (E_ISERR(error)) {
  /* Restore the saved system call registers */
  set_current_useripsp(safed_ip,safed_sp);
 }
 PREEMPTION_POP(was);
 return error;
}



PRIVATE SAFE USER void *KCALL
do_dlopen(struct module *__restrict mod, int flags,
          struct cpustate *__restrict state) {
 struct instance *inst;
 struct modpatch patch;
 void *result;
 bool inst_is_ref = false;

 if (flags&RTLD_NOLOAD) {
  struct mman *mm = THIS_MMAN;
  /* Search for an instance already resident in memory. */
  result = E_PTR(mman_read(mm));
  if (E_ISERR(result)) return result;
  inst = mman_instance_of_unlocked(mm,mod);
  mman_endread(mm);
  if (!inst) return E_PTR(-ENOENT); /* Module not loaded. */
  inst_is_ref = true;
  goto got_inst_noload;
 }

 modpatch_init_user(&patch,THIS_MMAN->m_exe);
 /* NOTE: Since `p_inst' is allowed to be NULL, no
  *       need to handle the case of a missing `m_exe' */
#if RTLD_DEEPBIND == MODPATCH_FLAG_DEEPBIND
 patch.p_pflags |= flags&MODPATCH_FLAG_DEEPBIND;
#else
 patch.p_pflags |= flags&RTLD_DEEPBIND ? MODPATCH_FLAG_DEEPBIND : 0;
#endif

 /* We make our lives simple by loading the given
  * module as a dependency of the root executable. */
 inst = modpatch_dldep(&patch,mod);
 modpatch_fini(&patch);
 if (E_ISERR(inst)) return E_PTR(E_GTERR(inst));

 /* Increment the instance's dlopen() recursion counter. */
 ATOMIC_FETCHINC(inst->i_openrec);
got_inst_noload:
 /* We use instance base addresses as module handles. */
 result = (void *)((uintptr_t)inst->i_base+mod->m_begin);

 if (flags&RTLD_GLOBAL && THIS_MMAN->m_exe) {
  if (flags&RTLD_NODELETE) {
   instance_add_dependency(THIS_MMAN->m_exe,inst);
  } else {
   /* TODO: The above is exactly what must happen here, but if we
    *       did this, dlclose() could no longer be used to delete
    *       the module, as it would then have explicit dependencies... */
  }
 }

 if (!(flags&RTLD_NOINIT)) {
  /* Call initializers on `inst' and all
   * dependencies that don't have 'DID_INIT' set.
   * >> Just like in all other places, we need the system call to
   *    return to user-space by executing a long list of custom
   *    callbacks, yet this time we also need to preserve all
   *    user-space registers except for 'EAX'.
   * HINT: We can determine their values by looking at the `THIS_SYSCALL_*' macros. */
  struct init_closure arg = {
      .pbase_return_value = result,
      .state              = state,
  };
  errno_t error = dl_loadinit(inst,&arg);
  if (E_ISERR(error)) result = E_PTR(error);
 }

 if (inst_is_ref)
     INSTANCE_DECREF(inst);
 return result;
}

SYSCALL_SDEFINE(xdlopen,regs) {
 struct dentryname dname; int state;
 REF struct module *mod; void *result;
 task_crit();
 /* Copy `name' from userspace */
 dname.dn_name = ACQUIRE_FS_STRING((char *)GPREGS_SYSCALL_ARG1(regs->gp),&dname.dn_size,&state);
 if (E_ISERR(dname.dn_name)) { result = E_PTR(E_GTERR(dname.dn_name)); goto end; }
 dentryname_loadhash(&dname);

 /* TODO: 'LD_LIBRARY_PATH'? */
 /* TODO: Extended library search paths, as specified by various executables already loaded? */
 mod = module_open_default(&dname,true);
 RELEASE_STRING(dname.dn_name,state);

 if (E_ISERR(mod)) { result = E_PTR(E_GTERR(mod)); goto end; }

 /* Now simply open the module. */
 result = do_dlopen(mod,(int)GPREGS_SYSCALL_ARG2(regs->gp),regs);
 MODULE_DECREF(mod);
end:
 task_endcrit();
 GPREGS_SYSCALL_RET1(regs->gp) = (syscall_slong_t)result;
}

SYSCALL_SDEFINE(xfdlopen,regs) {
 REF struct dentry *module_dent;
 REF struct module *mod; void *result;
 task_crit();
 /* Simply request a directory entry from the FD-manager. */
 module_dent = fdman_get_dentry(THIS_FDMAN,(int)GPREGS_SYSCALL_ARG1(regs->gp));
 if (E_ISERR(module_dent)) { result = E_PTR(E_GTERR(module_dent)); goto end; }
 mod = module_open_d(module_dent,false);
 DENTRY_DECREF(module_dent);
 if (E_ISERR(mod)) { result = E_PTR(E_GTERR(mod)); goto end; }

 /* Now simply open the module. */
 result = do_dlopen(mod,(int)GPREGS_SYSCALL_ARG2(regs->gp),regs);
 MODULE_DECREF(mod);
end:
 task_endcrit();
 GPREGS_SYSCALL_RET1(regs->gp) = (syscall_slong_t)result;
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
 /* NOTE: Need a write-lock in case accessing `symbol' invokes ALLOA. */
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


#if defined(CONFIG_DEBUG) && 1
#define DLFINI_DEBUG(x) x
#else
#define DLFINI_DEBUG(x) (void)0
#endif

PRIVATE ssize_t KCALL
dl_enum_fini(VIRT void *pfun, modfun_t UNUSED(single_type), void *closure) {
 void *user_ip,**user_sp;
 DLFINI_DEBUG(syslog(LOG_DEBUG,"[DLFINI] Push finalizer at %p\n",pfun));
 /* Push the previous return address and replace it with the finalizer. */
 get_current_useripsp(&user_ip,(USER void **)&user_sp);
 *--user_sp = user_ip;
 set_current_useripsp(pfun,user_sp);
 return 0;
}

PRIVATE void FCALL /* Use fastcall to optimize the loop calling this function. */
dl_do_loadfini_inst(struct instance *__restrict inst) {
 ssize_t (KCALL *pmodfun)(struct instance *__restrict self,
                          modfun_t types, penummodfun callback, void *closure);
 struct instance **iter,**end;
 /* Make sure we only call finalizers for any instance once.
  * NOTE: By also checking the DID_INIT-flag, we prevent
  *       finalizing a library that was never initialized. */
 if ((ATOMIC_FETCHOR(inst->i_flags,DID_FINI)&
     (DID_INIT|DID_FINI)) != DID_INIT) return;
 /* Reminder: We must push finalizers in reverse. */
 pmodfun = inst->i_module->m_ops->o_modfun;
 if (pmodfun) {
  DLFINI_DEBUG(syslog(LOG_DEBUG,"[DLFINI] Scan module `%[file]' for finalizers\n",inst->i_module->m_file));
  (*pmodfun)(inst,MODFUN_FINI|MODFUN_REVERSE,&dl_enum_fini,NULL);
 }

 /* NOTE: Holding a lock to the memory manager, there should
  *       be no way the dependency/using chains can change. */
 end = (iter = inst->i_deps.is_setv)+inst->i_deps.is_setc;
 for (; iter != end; ++iter) dl_do_loadfini_inst(*iter);
}

INTERN void KCALL
dl_do_loadfini(struct mman *__restrict mm) {
 struct instance *inst;
 /* Start out by pushing finalizers for the executable
  * itself, as well as its dependencies. */
 if (mm->m_exe)
     dl_do_loadfini_inst(mm->m_exe);
 /* Now go through the list of instances again and make sure
  * we didn't skip anything (Such as lazily loaded modules). */
 MMAN_FOREACH_INST(inst,mm) {
  dl_do_loadfini_inst(inst);
 }
}

SYSCALL_DEFINE0(xdlfini) {
 struct mman *mm = THIS_MMAN;
 task_crit();
 /* XXX: Clear all pending user-alarm()s. */

 /* NOTE: We need a write-lock in case pushing
  *       data onto the user-stack causes a #PF. */
 if (E_ISERR(mman_write(mm))) goto end;
 /* Use a user-worker to handle segfaults when pushing data onto the user-stack. */
 call_user_worker(&dl_do_loadfini,1,mm);
 mman_endwrite(mm);
end:
 task_endcrit();
 return -EOK;
}


SYSCALL_DEFINE4(xvirtinfo,VIRT void *,addr,
                USER struct virtinfo *,buf,
                size_t,bufsize,u32,flags) {
 ssize_t result;
 task_crit();
 result = user_virtinfo(addr,buf,bufsize,flags);
 task_endcrit();
 return result;
}

DECL_END

#endif /* !GUARD_KERNEL_LINKER_SYS_LINKER_C */
