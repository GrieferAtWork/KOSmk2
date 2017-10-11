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
#ifndef GUARD_KERNEL_LINKER_PATCH_C
#define GUARD_KERNEL_LINKER_PATCH_C 1
#define _KOS_SOURCE 2

#include <assert.h>
#include <errno.h>
#include <hybrid/align.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/types.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/test.h>
#include <kos/ksym.h>
#include <linker/module.h>
#include <linker/patch.h>
#include <malloc.h>
#include <string.h>
#include <sys/mman.h>

DECL_BEGIN

#define NAME_FOR_THIS_INSTANCE    "__this_instance"
#define HASH_FOR_THIS_INSTANCE    257594421

#define NAME_FOR_THIS_MODULE      "__this_module"
#define HASH_FOR_THIS_MODULE      24071381

TEST(hash_constants) {
 assertf(HASH_FOR_THIS_INSTANCE == sym_hashname(NAME_FOR_THIS_INSTANCE),
                           "%I32d",sym_hashname(NAME_FOR_THIS_INSTANCE));
 assertf(HASH_FOR_THIS_MODULE == sym_hashname(NAME_FOR_THIS_MODULE),
                         "%I32d",sym_hashname(NAME_FOR_THIS_MODULE));
}


PUBLIC void KCALL
modpatch_fini(struct modpatch *__restrict self) {
 struct instance **iter,**end;
 CHECK_HOST_DOBJ(self);
 /* Free tracked dependencies. */
 end = (iter = self->p_depv)+self->p_depc;
 for (; iter != end; ++iter) INSTANCE_DECREF(*iter);
 free(self->p_depv);
}


PUBLIC void KCALL
modpatch_common_dlsym_impl(struct modpatch *__restrict self,
                           char const *__restrict name, u32 hash,
                           struct modsym *__restrict result,
                           struct modsym *__restrict weak_result,
                           bool search_current) {
 struct instance **iter,**end;
 struct modsym new_sym;
#if 0
 syslog(LOG_DEBUG,"DLSYM(%q,%I32d)\n",name,hash);
#endif

 /* Deep binding preferrs the module itself above others. */
 if (search_current && self->p_inst &&
    (self->p_pflags&MODPATCH_FLAG_DEEPBIND)) {
  CHECK_HOST_DOBJ(self->p_inst);
  CHECK_HOST_DOBJ(self->p_inst->i_module);
  CHECK_HOST_DOBJ(self->p_inst->i_module->m_ops);
  CHECK_HOST_TEXT(self->p_inst->i_module->m_ops->o_symaddr,1);
  new_sym = (*self->p_inst->i_module->m_ops->o_symaddr)(self->p_inst,name,hash);
  if (new_sym.ms_type != MODSYM_TYPE_INVALID) {
   if (new_sym.ms_type == MODSYM_TYPE_OK) { *result = new_sym; goto end; }
   *weak_result = new_sym;
  }
 }

 /* Search upper modules first. */
 if (self->p_prev) {
  modpatch_common_dlsym_impl(self->p_prev,name,hash,
                             result,weak_result,true);
  if (result->ms_type != MODSYM_TYPE_INVALID) goto end;
 }

 /* Search dependency vector in ascending order. */
 end = (iter = self->p_depv)+self->p_depc;
 for (; iter != end; ++iter) {
  struct instance *inst = *iter;
  CHECK_HOST_DOBJ(inst);
  CHECK_HOST_DOBJ(inst->i_module);
  CHECK_HOST_DOBJ(inst->i_module->m_ops);
  CHECK_HOST_TEXT(inst->i_module->m_ops->o_symaddr,1);
  new_sym = (*inst->i_module->m_ops->o_symaddr)(inst,name,hash);
  if (new_sym.ms_type != MODSYM_TYPE_INVALID) {
   if (new_sym.ms_type == MODSYM_TYPE_OK) { *result = new_sym; goto end; }
   *weak_result = new_sym;
  }
 }
 /* Search the module being patched itself. */
 if (search_current && self->p_inst && !(self->p_pflags&MODPATCH_FLAG_DEEPBIND)) {
  CHECK_HOST_DOBJ(self->p_inst);
  CHECK_HOST_DOBJ(self->p_inst->i_module);
  CHECK_HOST_DOBJ(self->p_inst->i_module->m_ops);
  CHECK_HOST_TEXT(self->p_inst->i_module->m_ops->o_symaddr,1);
  new_sym = (*self->p_inst->i_module->m_ops->o_symaddr)(self->p_inst,name,hash);
  if (new_sym.ms_type != MODSYM_TYPE_INVALID) {
   if (new_sym.ms_type == MODSYM_TYPE_OK) { *result = new_sym; goto end; }
   *weak_result = new_sym;
  }
 }
end:;
}

PUBLIC void *KCALL
modpatch_common_dlsym(struct modpatch *__restrict self,
                      char const *__restrict name, u32 hash,
                      bool search_current) {
 struct modsym result;
 struct modsym weak_result;
 result.ms_type      = MODSYM_TYPE_INVALID;
 weak_result.ms_type = MODSYM_TYPE_INVALID;
 modpatch_common_dlsym_impl(self,name,hash,&result,&weak_result,search_current);
 if (result.ms_type      != MODSYM_TYPE_INVALID) return result.ms_addr;
 if (weak_result.ms_type != MODSYM_TYPE_INVALID) return weak_result.ms_addr;
 return NULL;
}


INTDEF void *KCALL kernel_symaddr(char const *__restrict name, u32 hash);
PUBLIC void *KCALL
modpatch_host_dlsym(struct modpatch *__restrict self,
                    char const *__restrict name, u32 hash,
                    bool search_current) {
 void *result;
 CHECK_HOST_DOBJ(self);
 /* Search for regular symbols. */
 result = modpatch_common_dlsym(self,name,hash,search_current);
 if (result) return result;

 /* Search the kernel symbol table for the given name. */
 result = kernel_symaddr(name,hash);
 if (result) return result;

 /* Resolve special driver symbols like '__this_instance'
  * >> Points to the module instance's controller block.
  * Yes: This is how modules know where 'THIS_INSTANCE' actually is! */
 if (hash == HASH_FOR_THIS_INSTANCE &&
    !strcmp(name,NAME_FOR_THIS_INSTANCE))
     return self->p_inst;
 if (hash == HASH_FOR_THIS_MODULE &&
    !strcmp(name,NAME_FOR_THIS_MODULE) &&
     self->p_inst != NULL)
     return self->p_inst->i_module;

 return NULL;
}
PUBLIC void *KCALL
modpatch_user_dlsym(struct modpatch *__restrict self,
                    char const *__restrict name, u32 hash,
                    bool search_current) {
 void *result;
 /* Search for regular symbols from dependency and other places. */
 result = modpatch_common_dlsym(self,name,hash,search_current);
 if (result) return result;

 /* Check for special kernel symbols. (e.g.: '.kern.keymap') */
 if (memcmp(name,__KSYM_PREFIX,sizeof(__KSYM_PREFIX)-sizeof(char)) == 0) {
  name  += COMPILER_STRLEN(__KSYM_PREFIX);
  result = kernel_symaddr(name,sym_hashname(name));
  /* Make sure to only return symbols from the kernel's user-data section. */
  if ((uintptr_t)result <  (uintptr_t)__kernel_user_start ||
      (uintptr_t)result >= (uintptr_t)__kernel_user_end)
       result = NULL;
 }
 return result;
}


STATIC_ASSERT(offsetof(struct modpatch,p_inst) == MODPATCH_OFFSETOF_INST);
STATIC_ASSERT(sizeof(struct modpatch)          == MODPATCH_SIZE);

/* Enabling this also swallows some ~real~ kernel errors.
 * >> Disable it in debug builds (for now), but must re-enable
 *    later once the system is fully stable. */
#if !defined(CONFIG_DEBUG) || 0
GLOBAL_ASM(
L(.section .text                                                   )
L(PUBLIC_ENTRY(modpatch_patch)                                     )
L(    movl 4(%esp), %edx /* Load 'self' */                         )
L(    movl MODPATCH_OFFSETOF_INST(%edx),   %eax /* ->p_inst */     )
L(    testl %eax, %eax                                             )
L(    jz   3f /* No-op if no instance is set (NOTE: Return -EOK == 0). */)
L(    movl INSTANCE_OFFSETOF_MODULE(%eax), %eax /* ->i_module */   )
L(    movl MODULE_OFFSETOF_OPS(%eax),      %eax /* ->m_ops */      )
L(    movl MODULEOPS_OFFSETOF_PATCH(%eax), %eax /* ->o_patch */    )
L(                                                                 )
L(    /* Save calle-safe register (may be corrupted when a fault occurrs) */)
L(    pushl %edi                                                   )
L(    pushl %esi                                                   )
L(    pushl %ebx                                                   )
L(    pushl %ebp                                                   )
L(                                                                 )
L(    /* Now to push a page-fault handler! */                      )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %ebx                    )
L(    pushl $1f                                                    )
L(    pushl $(EXC_PAGE_FAULT)                                      )
L(    pushl TASK_OFFSETOF_IC(%ebx)                                 )
L(    movl  %esp, TASK_OFFSETOF_IC(%ebx)                           )
L(                                                                 )
#ifdef CONFIG_DEBUG
L(    /* Preserve traceback integrity */                           )
L(    pushl %ebp                                                   )
L(    movl  %esp, %ebp                                             )
#endif
L(    pushl %edx                                                   )
L(    call *%eax /* o_patch(self) */                               )
#ifdef CONFIG_DEBUG
L(    popl  %ebp                                                   )
#endif
L(                                                                 )
L(    /* Cleanup if no error occurred. */                          )
L(    /* NOTE: 'EBX' is callee-save, meaning it still contains 'THIS_TASK' */ )
L(    popl  TASK_OFFSETOF_IC(%ebx)                                 )
L(    addl  $8, %esp                                               )
L(                                                                 )
L(2:  /* Restore calle-safe register */                            )
L(    popl %ebp                                                    )
L(    popl %ebx                                                    )
L(    popl %esi                                                    )
L(    popl %edi                                                    )
L(3:  ret  $4                                                      )
L(1:  movl $(-EFAULT), %eax /* Simply return -EFAULT */            )
L(    jmp 2b                                                       )
L(SYM_END(modpatch_patch)                                          )
L(.previous                                                        )
);
#else
PUBLIC errno_t KCALL
modpatch_patch(struct modpatch *__restrict self) {
 /* NOTE: Must be implemented in assembly to
  *       use local interrupt handlers. */
 CHECK_HOST_DOBJ(self);
 if (!self->p_inst) return -EOK;
 CHECK_HOST_DOBJ(self->p_inst);
 CHECK_HOST_DOBJ(self->p_inst->i_module);
 CHECK_HOST_DOBJ(self->p_inst->i_module->m_ops);
 CHECK_HOST_TEXT(self->p_inst->i_module->m_ops->o_patch,1);
 return (*self->p_inst->i_module->m_ops->o_patch)(self);
}
#endif


PUBLIC struct instance *KCALL
modpatch_find_dep(struct modpatch *__restrict self,
                  struct module *__restrict dependency) {
 struct instance **iter,**begin;
 CHECK_HOST_DOBJ(self);
 if (self->p_inst) {
  CHECK_HOST_DOBJ(self->p_inst);
  CHECK_HOST_DOBJ(self->p_inst->i_module);
  /* Check if this patcher _is_ for that same module. */
  if (self->p_inst->i_module == dependency)
      return self->p_inst;
 }
 /* Search the dependency vector in reverse order. */
 iter = (begin = self->p_depv)+self->p_depc;
 while (iter-- != begin) {
  CHECK_HOST_DOBJ(*iter);
  CHECK_HOST_DOBJ((*iter)->i_module);
  if ((*iter)->i_module == dependency) return *iter;
 }
 return NULL;
}

PUBLIC struct instance *KCALL
modpatch_dldep(struct modpatch *__restrict self,
               struct module *__restrict dependency) {
 REF struct instance *inst,*existing_inst; errno_t error;
 size_t new_alloc,min_alloc; bool has_write_lock;
 struct modpatch inst_patcher;
 struct modpatch *existing_check = self;
 struct mman *target_mman = THIS_MMAN;

 /* Search for an existing instance of 'dependency'. */
 do { inst = modpatch_find_dep(existing_check,dependency);
      if (inst) return inst;
 } while ((existing_check = existing_check->p_prev) != NULL);

 memcpy(&inst_patcher,self,sizeof(struct modpatch));
 inst_patcher.p_depa = 0;
 inst_patcher.p_depc = 0;
 inst_patcher.p_depv = NULL;

 /* Check if the module is already loaded in the target memory manager. */
 error = mman_read(target_mman);
 if (E_ISERR(error)) goto err;
 inst = mman_instance_of_unlocked(target_mman,dependency);
 mman_endread(target_mman);

 /* If the instance was already mapped, use the existing version instead. */
 if (inst) goto done;

 /* Create a new instance for the given dependency module. */
 inst = instance_new(dependency,self->p_iflags);
 if unlikely(!inst) return E_PTR(-ENOMEM);
 inst_patcher.p_inst = inst;
 inst_patcher.p_prev = self; /* Link the instance patcher recursion chain. */

 /* Make sure all regions for the given module are allocated. */
 error = module_mkregions(dependency);
 if (E_ISERR(error)) goto err;

 has_write_lock = false;
 error = mman_read(target_mman);
 if (E_ISERR(error)) goto err;

find_space:
 /* After having had the memory manager unlocked, it is possible
  * that the module got another instance in the mean time.
  * Therefor, we must scan the manager again to make sure this is still the first instance. */
 existing_inst = mman_instance_of_unlocked(target_mman,dependency);
 if unlikely(existing_inst) {
  /* Highly unlikely: The module somehow got an instance in the mean time. - Use it! */
  if (has_write_lock)
       mman_endwrite(target_mman);
  else mman_endread(target_mman);
  /* Destroy our temporary instance and replace it with the existing one. */
  INSTANCE_DECREF(inst);
  inst = existing_inst;
  goto done;
 }

 if (!(dependency->m_flag&MODFLAG_RELO) ||
       dependency->m_flag&MODFLAG_PREFERR) {
  struct modseg *iter,*end;
  /* The dependency module isn't relocatable. */
  /* TODO: Go through all of the dependencies segments
   *       and ensure that all of them are unused. */
  if (!IS_ALIGNED(dependency->m_load,PAGESIZE))
      goto need_custom_base;
  end = (iter = dependency->m_segv)+dependency->m_segc;
  for (; iter != end; ++iter) {
   if (mman_inuse_unlocked(target_mman,(void *)(dependency->m_load+iter->ms_paddr),
                           iter->ms_msize))
       goto need_custom_base;
  }
  syslog(LOG_DEBUG,"[PATCH] Preferred range %p...%p of module '%[file]' can be used\n",
         dependency->m_load+dependency->m_begin,
         dependency->m_load+dependency->m_end-1,
         dependency->m_file);
  /* The preferred base can be used. - Go ahead and load the module there. */
  inst->i_base = (ppage_t)dependency->m_load;
  goto got_space;

need_custom_base:
  /* If the default segment mappings are already in use, fail. */
  if (!(dependency->m_flag&MODFLAG_RELO))
        goto no_space;
 }

 /* Determine a suitable location to map the new module dependency. */
 inst->i_base = mman_findspace_unlocked(target_mman,inst_patcher.p_dephint,
                                        dependency->m_size,MIN(dependency->m_align,PAGESIZE),
                                        inst_patcher.p_mapgap,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_FORCEGAP);
 if (inst->i_base != PAGE_ERROR) goto got_space;
 inst->i_base = mman_findspace_unlocked(target_mman,inst_patcher.p_dephint,
                                        dependency->m_size,MIN(dependency->m_align,PAGESIZE),
                                        0,MMAN_FINDSPACE_ABOVE);
 if (inst->i_base != PAGE_ERROR) goto got_space;
 inst->i_base = mman_findspace_unlocked(target_mman,(ppage_t)0,
                                        dependency->m_size,MIN(dependency->m_align,PAGESIZE),
                                        inst_patcher.p_mapgap,MMAN_FINDSPACE_ABOVE|MMAN_FINDSPACE_FORCEGAP);
 if (inst->i_base != PAGE_ERROR) goto got_space;
 inst->i_base = mman_findspace_unlocked(target_mman,(ppage_t)0,
                                        dependency->m_size,MIN(dependency->m_align,PAGESIZE),
                                        0,MMAN_FINDSPACE_ABOVE);
 if (inst->i_base != PAGE_ERROR) goto got_space;
 /* Failed to find free memory... */
no_space:
 if (has_write_lock)
      mman_endwrite(target_mman);
 else mman_endread(target_mman);
 error = -ENOMEM;
 goto err;
got_space:
 if (!has_write_lock) {
  error = mman_upgrade(target_mman);
  if (E_ISERR(error)) {
   if (error == -ERELOAD) {
    has_write_lock = true;
    goto find_space;
   }
   goto err;
  }
 }
 /* Make sure to map everything as writable if the module uses text-relocations. */
 /* We've tracked down sufficient space. - Now to map to it!
  * NOTE: Initially, all sections are mapped as 'PROT_CLEAN', indicating
  *       that they have not been altered by user-space. */
 error = mman_mmap_instance_unlocked(target_mman,inst,(u32)-1,
                                    (dependency->m_flag&MODFLAG_TEXTREL)
                                     ? PROT_WRITE|PROT_CLEAN : PROT_CLEAN);
 mman_endwrite(target_mman);
 if (E_ISERR(error)) goto err;

 /* Update the instance patcher dependency hint to point after the newly added instance. */
 inst_patcher.p_dephint = (ppage_t)((uintptr_t)inst->i_base+inst_patcher.p_mapgap+
                                     CEIL_ALIGN(dependency->m_end,PAGESIZE));

 /* If the dependency module doesn't include relocations, we're already done! */
 if (!(dependency->m_flag&MODFLAG_RELO)) goto done;

 /* Actually patch the dependency module. */
 error = modpatch_patch(&inst_patcher);
 if (E_ISERR(error)) goto err;

 /* Restore all dependency segments to being read-only. */
 if (dependency->m_flag&MODFLAG_TEXTREL) {
  error = module_restore_readonly(dependency,inst->i_base);
  if (E_ISERR(error)) goto err;
 }

done:
 /* DONE! The dependency is now loaded.
  * >> Everything else is just re-assigning data controllers. */

 /* Inherit all additional dependencies. */
 min_alloc = self->p_depc+inst_patcher.p_depc+1;
 { REF struct instance **new_vec;
   new_alloc = self->p_depa;
   if (!new_alloc) new_alloc = 1;
   while (new_alloc < min_alloc) new_alloc *= 2;
   new_vec = trealloc(REF struct instance *,
                      self->p_depv,new_alloc);
   if (!new_vec) {
    new_alloc = min_alloc;
    if (min_alloc <= self->p_depa)
     new_vec   = self->p_depv,
     new_alloc = self->p_depa;
    else {
     new_vec = trealloc(REF struct instance *,
                        self->p_depv,new_alloc);
     if (!new_vec) { error = -ENOMEM; goto err; }
    }
   }
   assert(new_alloc >= min_alloc);
   self->p_depa  = new_alloc;
   self->p_depv  = new_vec;
   memcpy(new_vec+self->p_depc,
          inst_patcher.p_depv,
          inst_patcher.p_depc*
          sizeof(REF struct instance *));
   free(inst_patcher.p_depv);
   /* Load the newly loaded dependency itself. */
   new_vec[min_alloc-1] = inst; /* Inherit reference. */
   self->p_depc = min_alloc;
 }

 /* Inherit the load-address hint of dependency patcher. */
 self->p_dephint = inst_patcher.p_dephint;

 /* Run initializers for drivers. */
 if (MODPATCH_ISHOST(self)) {
  syslog(LOG_EXEC|LOG_INFO,"[MOD] Loaded kernel module '%[file]' at %p...%p\n",
         inst->i_module->m_file,inst->i_base,
        (uintptr_t)inst->i_base+inst->i_module->m_size-1);
  instance_callinit(inst);
 }

 /* Only return a weak pointer.
  * The real instance reference itself is stored in the
  * dependency vector, which is owned by the caller. */
 return inst;
err:
 INSTANCE_DECREF(inst);
 modpatch_fini(&inst_patcher);
 return E_PTR(error);
}

PUBLIC REF struct module *KCALL
modpatch_dlopen(struct modpatch const *__restrict self,
                struct dentryname const *__restrict name) {
 CHECK_HOST_TOBJ(self);
 CHECK_HOST_TOBJ(name);
 return MODPATCH_ISUSER(self)
      ? module_open_default(name,true)
      : module_open_driver(name,false);
}



DECL_END

#endif /* !GUARD_KERNEL_LINKER_PATCH_C */
