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
#ifndef GUARD_KERNEL_LINKER_EXEC_C
#define GUARD_KERNEL_LINKER_EXEC_C 1
#define _KOS_SOURCE 2

#include <fs/access.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <hybrid/align.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/arch/gdt.h>
#include <kernel/mman.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <kos/environ.h>
#include <kos/syslog.h>
#include <linker/module.h>
#include <linker/patch.h>
#include <sched/cpu.h>
#include <sched/task.h>
#include <string.h>
#include <sys/mman.h>
#include <sched/signal.h>

DECL_BEGIN

#define TERMINATE_ON_EXEC_CODE ((void *)-1)

/* NOTE: Upon success, this function does not
 *       return and will inherit a reference to 'mod'. */
PRIVATE SAFE errno_t KCALL
user_execve(REF struct module *__restrict mod,
            USER char const *const USER *argv,
            USER char const *const USER *envp) {
 errno_t error; struct modpatch patch;
 struct task *exec_task = THIS_TASK;
 struct mman *mm = exec_task->t_mman;
 struct instance *inst;
 USER struct envdata *environ;
 struct mman_maps env_maps = {NULL};
 CHECK_HOST_DOBJ(mod);
 syslogf(LOG_DEBUG,"Begin exec: '%[file]'\n",mod->m_file);
 assertf(mm != &mman_kernel,"You can't exec() with the kernel memory manager set!");

 /* Make sure the module is really an executable. */
 if (!(mod->m_flag&MODFLAG_EXEC)) return -ELIBEXEC;

 /* Make sure all module regions are cached. */
 error = module_mkregions(mod);
 if (E_ISERR(error)) goto end;

 /* Create the new instance object for the given module. */
 inst = instance_new_user(mod);
 if unlikely(!inst) { error = -ENOMEM; goto end; }

 /* Figure out where this initial instance should be loaded at. */
 inst->i_base = (VIRT ppage_t)FLOOR_ALIGN(mod->m_load,PAGESIZE);

relock_tasks_again:
 /* Terminate all threads other than the calling 'mm' */
 while (!atomic_rwlock_trywrite(&mm->m_tasks_lock)) {
  error = task_testintr();
  if (E_ISERR(error)) goto end;
  SCHED_YIELD();
 }

 /* No-interrupt can only be enabled _AFTER_ we've acquired
  * a write-lock to the chain of threads using the memory
  * manager, in order to prevent a deadlock when two
  * threads try to exec() at the same time, otherwise
  * causing both of them to become uninterruptible,
  * with one is waiting for a lock the other is holding
  * while the other is trying to join the first below.
  * >> When we only enable no-interrupt here, one of
  *    the two tasks will still be dangling in the
  *    atomic lock above, when the other will terminate
  *    the first, causing the testintr() to trigger
  *    and the first one to terminate.
  */
 COMPILER_BARRIER();
 task_nointr();

 { REF struct task *term_task;
   for (term_task = mm->m_tasks; term_task;
        term_task = term_task->t_mman_tasks.le_next) {
    if (term_task == exec_task) continue;
    error = task_terminate(term_task,TERMINATE_ON_EXEC_CODE);
    if (error == -ECOMM) {
     atomic_rwlock_endwrite(&mm->m_tasks_lock);
     task_endnointr();
     goto end;
    }
   }
   /* With all tasks but the caller signaled to
    * terminate, join each of them to ensure we're alone. */
   for (term_task = mm->m_tasks; term_task;
        term_task = term_task->t_mman_tasks.le_next) {
    if (term_task == exec_task) continue;
    if (!TASK_TRYINCREF(term_task)) continue;
    atomic_rwlock_endwrite(&mm->m_tasks_lock);
    task_endnointr();
    assert(term_task != THIS_TASK);

    /* Signal the task to terminate again (in case new ones got added...)
     * This operation is likely a no-op, but prevents the join() below from hanging. */
    task_terminate(term_task,TERMINATE_ON_EXEC_CODE);

    /* Join the task. */
    error = task_join(term_task,NULL,NULL);
    TASK_DECREF(term_task);
    if (E_ISERR(error)) goto end;

    /* Having joined another task, loop around and start over. */
    goto relock_tasks_again;
   }
 }
 task_endnointr();

 /* At this point, we should be the only remaining task still
  * executing within the associated memory manager. - Assert this! */
#if defined(CONFIG_DEBUG) && 1 /* TODO: Disable me! */
 /* Can't really be asserted: Tasks should all be
 *  terminated, but may not have been cleaned up yet. */
 assert(exec_task->t_mman_tasks.le_pself == &mm->m_tasks);
 assert(exec_task->t_mman_tasks.le_next  == NULL);
#endif
 atomic_rwlock_endwrite(&mm->m_tasks_lock);

 error = mman_write(mm);
 if (E_ISERR(error)) goto end;

 /* Generate the new environment table. */
 environ = mman_setenviron_unlocked(mm,(char **)argv,(char **)envp);
 if (E_ISERR(environ)) { error = E_GTERR(environ); goto endwrite; }
 assert(environ == mm->m_environ);

 /* Extract all pages associated with the environment table. */
 error = (errno_t)mman_mextract_unlocked(mm,&env_maps,(ppage_t)environ,mm->m_envsize,true);
 if (E_ISERR(error)) goto endwrite;

 /* Unmap _EVERYTHING_ from the calling process.
  * NOTE: This this isn't the kernel page directory,
  *       only mappings below KERNEL_BASE should exist. */
 mman_munmap_unlocked(mm,(ppage_t)0,(size_t)-1,
                      MMAN_MUNMAP_ALL,NULL);

 /* Map the new instance into the (currently) empty address space. */
 error = mman_mmap_instance_unlocked(mm,inst,(u32)-1,
                                     mod->m_flag&MODFLAG_TEXTREL
                                   ? PROT_CLEAN|PROT_WRITE
                                   : PROT_CLEAN);
endwrite:
 mman_endwrite(mm);

 if (E_ISERR(error)) goto end_too_late;

 assert(mm == THIS_MMAN);

 /* Delete the old user-space task. */
 if (exec_task->t_ustack) {
  STACK_DECREF(exec_task->t_ustack);
  exec_task->t_ustack = NULL;
 }

 /* Patch relocations & load dependencies. */
 modpatch_init_user(&patch,inst);
 error = modpatch_patch(&patch);

 if (E_ISERR(error)) goto end_too_late_patch;
 if (mod->m_flag&MODFLAG_TEXTREL) {
  error = module_restore_readonly(mod,inst->i_base);
  if (E_ISERR(error)) goto end_too_late_patch;
 }

 /* Restore environment pages. */
 error = mman_write(mm);
 if (E_ISERR(error)) goto end_too_late;
 if (!mman_inuse_unlocked(mm,environ,mm->m_envsize)) {
  error = (errno_t)mman_mrestore_unlocked(mm,&env_maps,(ppage_t)environ,true);
 } else {
  environ = (USER struct envdata *)mman_findspace_unlocked(mm,(ppage_t)((KERNEL_BASE-(4*PAGESIZE))-mm->m_envsize),
                                                           mm->m_envsize,PAGESIZE,0,MMAN_FINDSPACE_BELOW);
  if unlikely(environ == PAGE_ERROR) error = -ENOMEM;
  else error = (errno_t)mman_mrestore_unlocked(mm,&env_maps,
                                              (ppage_t)environ,true);
 }

 if (unlikely(environ != mm->m_environ) && E_ISOK(error)) {
  /* Must relocate the environment table. */
  uintptr_t offset = (uintptr_t)environ-(uintptr_t)mm->m_environ;
  char **iter,**end;
  *(uintptr_t *)&environ->e_envp += offset;
  *(uintptr_t *)&environ->e_argv += offset;
  environ->e_self = environ;
  /* NOTE: It is safe to dereference the environment array, because
   *       we're the only running thread that actually has access. */
  end = (iter = ENVDATA_ENVV(*environ))+mm->m_envenvc;
  for (; iter != end; ++iter) *iter += offset;
  end = (iter = ENVDATA_ARGV(*environ))+ENVDATA_ARGC(*environ);
  for (; iter != end; ++iter) *iter += offset;
  /* Finally, update the stored environment pointer itself. */
  mm->m_environ = environ;
 }
 mman_endwrite(mm);
 if (E_ISERR(error)) goto end;

 /* Setup the root address of the core binary.
  * >> Return a pointer to the start of the first core instance's first segment. */
 environ->e_root = (void *)((uintptr_t)inst->i_base+mod->m_begin);

 /* At this point, the entire execute has been loaded.
  * >> Now all that's left is to allocate a new user-space stack. */

 /* Allocate a new user-space stack. */
 error = task_mkustack(exec_task,
                       TASK_USERSTACK_DEFAULTSIZE,
                       TASK_USERSTACK_GUARDSIZE,
                       TASK_USERSTACK_FUNDS);
 if (E_ISERR(error)) goto end_too_late_patch;

 /* Reset signal handlers. */
 sighand_reset(exec_task->t_sighand);

 /* TODO: call module constructors? (Push the real entry point and all
  *       constructors but the first in reverse order on the user-stack,
  *       then simply jump to the first. - When it rets, it will execute
  *       the next constructor, and so on.
  *    >> With that in mind, we'll probably have to move the
  *       environment block to a callee-save register such as EBX.
  */

 /* All right! Everything's been set up!
  * >> The last thing remaining now, is to actually start execution of the module! */
 { struct cpustate state;
   memset(&state,0,sizeof(struct cpustate));
#if 0
   state.host.gs     = SEG(SEG_USER_DATA)|3;
   state.host.fs     = SEG(SEG_USER_DATA)|3;
#elif defined(__i386__)
   state.host.gs     = SEG(SEG_USER_DATA)|3;
   state.host.fs     = SEG(SEG_CPUSELF);
#else
   state.host.gs     = SEG(SEG_CPUSELF);
   state.host.fs     = SEG(SEG_USER_DATA)|3;
#endif
   state.host.es     = SEG(SEG_USER_DATA)|3;
   state.host.ds     = SEG(SEG_USER_DATA)|3;
   state.host.cs     = SEG(SEG_USER_CODE)|3;
   state.host.ecx    = (uintptr_t)environ; /* Pass the environment block through ECX. */
   state.host._n1    = 0;
   state.useresp     = (uintptr_t)exec_task->t_ustack->s_end;
   state.ss          = SEG(SEG_USER_DATA)|3;
   state._n2         = 0;
   state.host.eip    = (uintptr_t)inst->i_base+mod->m_entry;
#ifdef CONFIG_ALLOW_USER_IO
   state.host.eflags = EFLAGS_IF|EFLAGS_IOPL(3);
#else
   state.host.eflags = EFLAGS_IF;
#endif


   syslogf(LOG_EXEC|LOG_INFO,"[APP] Starting user app '%[file]' at %p\n",
           mod->m_file,state.host.eip);

   /* Last phase: actually switch to the new task! */
   MODULE_DECREF(mod);
   task_endcrit(); /* End the last remaining critical block. */

   /* Now we just need to switch to the new cpu-state. */
   PREEMPTION_DISABLE();
   assert(exec_task == THIS_TASK);
   exec_task->t_cstate = &state;
   __asm__ __volatile__("jmp cpu_sched_setrunning\n" : : : "memory");
   __builtin_unreachable();
 }

end_too_late_patch:
 modpatch_fini(&patch);
end_too_late:
 /* It's too late to rewind, after failing to start the application.
  * >> Log failure and mark the caller to termination once its critical block ends. */
 syslogf(LOG_EXEC|LOG_ERROR,
         "[EXEC] Failed to execute module '%[file]': %[errno]\n",
         mod->m_file,-error);
 mman_maps_fini(&env_maps);
 INSTANCE_DECREF(inst);
 assert(exec_task->t_critical == 1);
 task_terminate(exec_task,(void *)-1);
end:
 return error;
}


SYSCALL_DEFINE3(execve,USER char const *,filename,
                USER char const *const USER *,argv,
                USER char const *const USER *,envp) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct dentry *module_dentry;
 struct dentry *cwd; errno_t result;
 struct module *mod;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = false;
 assert(!task_iscrit());
 task_crit();

 result = fdman_read(fdm);
 if (E_ISERR(result)) goto end;
 cwd            = fdm->fm_cwd;
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(cwd);
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);

 module_dentry = dentry_user_xwalk(cwd,&walker,filename);

 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);
 if (E_ISERR(module_dentry)) { result = E_GTERR(module_dentry); goto end; }
 mod = module_open_d(module_dentry);
 DENTRY_DECREF(module_dentry);
 if (E_ISERR(mod)) { result = E_GTERR(mod); goto end; }

 /* We've got the module. - Time to execute it! */
 result = user_execve(mod,argv,envp);

 /* At this point, something must have went wrong... */
 MODULE_DECREF(mod);
end:
 task_endcrit();
 assert(!task_iscrit());
#if 1
 syslogf(LOG_EXEC|LOG_DEBUG,"[EXEC] exec failed: %[errno]\n",-result);
#endif
 return result;
}



DECL_END

#endif /* !GUARD_KERNEL_LINKER_EXEC_C */
