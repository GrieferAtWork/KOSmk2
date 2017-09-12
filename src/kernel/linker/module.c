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
#ifndef GUARD_KERNEL_LINKER_MODULE_C
#define GUARD_KERNEL_LINKER_MODULE_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <fcntl.h>
#include <fs/access.h>
#include <fs/basic_types.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwptr.h>
#include <hybrid/types.h>
#include <kernel/boot.h>
#include <kernel/export.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <malloc.h>
#include <sched/task.h>
#include <string.h>
#include <sync/rwlock.h>
#include <sys/mman.h>
#include <unistd.h>

DECL_BEGIN

STATIC_ASSERT(offsetof(struct moduleops,o_patch) == MODULEOPS_OFFSETOF_PATCH);
STATIC_ASSERT(sizeof(struct moduleops)           == MODULEOPS_SIZE);
STATIC_ASSERT(offsetof(struct module,m_ops)      == MODULE_OFFSETOF_OPS);
STATIC_ASSERT(offsetof(struct module,m_align)    == MODULE_OFFSETOF_ALIGN);
STATIC_ASSERT(sizeof(struct module)              == MODULE_SIZE);
STATIC_ASSERT(offsetof(struct instance,i_module) == INSTANCE_OFFSETOF_MODULE);
STATIC_ASSERT(sizeof(struct instance)            == INSTANCE_SIZE);


PUBLIC u32 KCALL
sym_hashname(char const *__restrict name) {
 /* HINT: This is the same hashing algorithm used by ELF. */
 u32 h = 0,g;
 while (*name) {
  h = (h << 4) + *name++;
  g = h & 0xf0000000;
  if (g) h ^= g >> 24;
  h &= ~g;
 }
 return h;
}


PUBLIC SAFE void KCALL
module_setup(struct module *__restrict self,
             struct file *__restrict fp,
             struct moduleops const *__restrict ops) {
 struct modseg *iter,*end;
 maddr_t addr_min = (maddr_t)-1;
 maddr_t addr_max = 0;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(fp);
 CHECK_HOST_DOBJ(fp->f_dent);
 CHECK_HOST_DOBJ(ops);
 end = (iter = self->m_segv)+self->m_segc;
 for (; iter != end; ++iter) {
  if (addr_min > iter->ms_paddr)
      addr_min = iter->ms_paddr;
  if (addr_max < iter->ms_paddr+iter->ms_msize)
      addr_max = iter->ms_paddr+iter->ms_msize;
 }
 /* Check for special case: 'self->m_segc == 0' */
 if (addr_min > addr_max)
     addr_min = addr_max;
 FILE_INCREF(fp);
 self->m_refcnt = 1;
 self->m_file   = fp;
 self->m_ops    = ops;
 self->m_begin  = addr_min;
 self->m_end    = addr_max;
 self->m_size   = addr_max-addr_min;
 atomic_rwlock_init(&self->m_rlock);
 /*syslog(LOG_DEBUG,"addr_min = %p\n",addr_min);*/
 /*syslog(LOG_DEBUG,"addr_max = %p\n",addr_max);*/
 if (!self->m_name)
      self->m_name = &fp->f_dent->d_name;
}

PUBLIC SAFE void KCALL
module_destroy(struct module *__restrict self) {
 struct inode *node;
 CHECK_HOST_DOBJ(self);
 assert(self != &__this_module);
 assert(!self->m_refcnt);
 if (self->m_ops->o_fini)
   (*self->m_ops->o_fini)(self);

 node = self->m_file->f_node;
 atomic_rwlock_write(&node->i_file.i_files_lock);
 /* NOTE: Must check if we're still the module
  *       as it's a weak cache pointer! */
 if (node->i_file.i_module == self)
     node->i_file.i_module = NULL;
 atomic_rwlock_endwrite(&node->i_file.i_files_lock);
 FILE_DECREF(self->m_file);

 /* Delete segments. */
 { struct modseg *iter,*end;
   end = (iter = self->m_segv)+self->m_segc;
   for (; iter != end; ++iter) {
    if (iter->ms_region)
        MREGION_DECREF(iter->ms_region);
   }
   free(self->m_segv);
 }
 free(self);
}



PUBLIC SAFE REF struct module *KCALL
module_open_d(struct dentry *__restrict dent) {
 struct iattr attr;
 struct file *module_stream;
 struct inode *node;
 REF struct module *result;
 struct fsaccess access;
 node = dentry_inode(dent);
 if (!node) return E_PTR(-ENOENT);
 FSACCESS_SETUSER(access);
 /* Check if the user is allowed to access this node. */
 result = E_PTR(inode_mayaccess(node,&access,R_OK));
 if (E_ISERR(result)) { INODE_DECREF(node); return E_PTR(-EACCES); }

 /* Check the file node's module cache entry. */
 atomic_rwlock_read(&node->i_file.i_files_lock);
 result = node->i_file.i_module;
 if (result) {
  if (result == IFILE_MODULE_LOADING) {
   atomic_rwlock_endread(&node->i_file.i_files_lock);
   return E_PTR(-ELOOP);
  }
  if (MODULE_TRYINCREF(result)) {
   atomic_rwlock_endread(&node->i_file.i_files_lock);
   return result;
  }
 }
 if unlikely(!atomic_rwlock_upgrade(&node->i_file.i_files_lock)) {
  result = node->i_file.i_module;
  if (result) {
   if (result == IFILE_MODULE_LOADING) {
    atomic_rwlock_endwrite(&node->i_file.i_files_lock);
    return E_PTR(-ELOOP);
   }
   if (MODULE_TRYINCREF(result)) {
    atomic_rwlock_endwrite(&node->i_file.i_files_lock);
    return result;
   }
  }
 }
 atomic_rwlock_endwrite(&node->i_file.i_files_lock);
 /* Open a new stream. */
 module_stream = dentry_open_with_inode(dent,node,&access,&attr,
                                        IATTR_NONE,O_RDONLY);
 if (E_ISERR(module_stream)) {
  INODE_DECREF(node);
  return E_PTR(E_GTERR(module_stream));
 }
 /* NOTE: Upon success (now confirmed), 'dentry_open_with_inode'
  *       has inherited a reference to 'node'. */

 /* Actually open the module's stream. */
 result = module_open(module_stream);
 FILE_DECREF(module_stream);
 return result;
}

FUNDEF SAFE REF struct module *KCALL
module_open_in_path(HOST char const *__restrict path, size_t pathlen,
                    struct dentryname const *__restrict filename,
                    bool use_user_fs) {
 struct module *result;
 struct dentry_walker walker;
 struct dentry *module_file;
 struct dentry *cwd;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = false;

 if (use_user_fs) {
  struct fdman *fdm = THIS_FDMAN;
  result = E_PTR(fdman_read(fdm));
  if (E_ISERR(result)) goto end;
  cwd            = fdm->fm_cwd;
  walker.dw_root = fdm->fm_root;
  DENTRY_INCREF(cwd);
  DENTRY_INCREF(walker.dw_root);
  fdman_endread(fdm);
 } else {
  /* Use the true filesystem root */
  cwd            = &fs_root;
  walker.dw_root = &fs_root;
  ATOMIC_FETCHADD(fs_root.d_refcnt,2);
 }

 /* Walk the given path. */
 module_file = dentry_xwalk(cwd,&walker,path,pathlen);
 DENTRY_DECREF(cwd);
 if (E_ISOK(module_file)) {
  /* Walk a single step towards the given file.
   * NOTE: Not doing a full walk here prevents exploits
   *       like this: '/lib/../../../../hidden/module/file.so'
   *                   ^^^^ ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   *                SEARCH-PATH   SEARCH-FILE */
  cwd = dentry_walk(module_file,&walker,filename);
  DENTRY_DECREF(module_file);
  module_file = cwd;
 }
 DENTRY_DECREF(walker.dw_root);
 if (E_ISERR(module_file)) { result = E_PTR(E_GTERR(module_file)); goto end; }

 result = module_open_d(module_file);
 DENTRY_DECREF(module_file);

 /* Fix no-exec errors to no-ent. */
 if (E_GTERR(result) == -ENOEXEC)
     result = E_PTR(-ENOENT);
end:
 return result;
}
FUNDEF SAFE REF struct module *KCALL
module_open_in_paths(HOST char const *__restrict paths,
                     struct dentryname const *__restrict filename,
                     bool use_user_fs) {
 char const *iter = paths;
 struct module *result = E_PTR(-ENOENT);
 for (;; ++iter) {
  if (!*iter || *iter == ':') {
   if (iter != paths) {
    result = module_open_in_path(paths,(size_t)(iter-paths),
                                 filename,use_user_fs);
    if (E_GTERR(result) != -ENOENT) break;
   }
   if (!*iter) break;
   paths = iter+1;
  }
 }
 return result;
}



PUBLIC char const *_module_search_path __ASMNAME("module_search_path") = "/lib:/usr/lib:/usr/local/lib";
PUBLIC char const *_driver_search_path __ASMNAME("driver_search_path") = "/mod";
DEFINE_SETUP("libpath=",linker_libpath) { _module_search_path = arg; return true; }
DEFINE_SETUP("drvpath=",linker_drvpath) { _driver_search_path = arg; return true; }




PUBLIC SAFE REF struct module *KCALL
module_open(struct file *__restrict fp) {
 struct inode *node;
 REF struct module *result;
 CHECK_HOST_DOBJ(fp);
 CHECK_HOST_DOBJ(fp->f_node);
 node = fp->f_node;
 /* Check the file node's module cache entry. */
 atomic_rwlock_read(&node->i_file.i_files_lock);
 result = node->i_file.i_module;
 if (result) {
  if (result == IFILE_MODULE_LOADING) {
   /* TODO: What if another thread is currently loading the module?
    *    >> As it is right now, we'd indicate a loop error... */
   atomic_rwlock_endread(&node->i_file.i_files_lock);
   return E_PTR(-ELOOP);
  }
  if (MODULE_TRYINCREF(result)) {
   atomic_rwlock_endread(&node->i_file.i_files_lock);
   return result;
  }
 }
 if unlikely(!atomic_rwlock_upgrade(&node->i_file.i_files_lock)) {
  result = node->i_file.i_module;
  if (result) {
   if (result == IFILE_MODULE_LOADING) {
    atomic_rwlock_endwrite(&node->i_file.i_files_lock);
    return E_PTR(-ELOOP);
   }
   if (MODULE_TRYINCREF(result)) {
    atomic_rwlock_endwrite(&node->i_file.i_files_lock);
    return result;
   }
  }
 }
 /* Switch the module state to loading. */
 node->i_file.i_module = IFILE_MODULE_LOADING;
 atomic_rwlock_endwrite(&node->i_file.i_files_lock);

 /* If it didn't contain a module, simply load a new module! */
 result = module_open_new(fp);
 assert(result);
 if (E_ISOK(result)) {
  atomic_rwlock_write(&node->i_file.i_files_lock);
  if likely(node->i_file.i_module == IFILE_MODULE_LOADING ||
           !node->i_file.i_module) { /* May be set to NULL if the node was unloaded? */
   node->i_file.i_module = result;
  } else if (!MODULE_TRYINCREF(node->i_file.i_module)) {
   node->i_file.i_module = result;
  } else {
   /* Well... Looks like we've loaded it twice due to race conditions...
    * >> Use the one already cached to keep stuff consistent. */
   struct module *newresult = node->i_file.i_module;
   atomic_rwlock_endwrite(&node->i_file.i_files_lock);
   MODULE_DECREF(result);
   return newresult;
  }
  /* NOTE: Don't create a reference for the cache entry, as it is
   *       only weakly cached to prevent a reference loop:
   *       #1 module  ->  REF(m_file)
   *       #2 m_file  ->  REF(f_node)
   *       #3 f_node  ->  i_file.i_module // Can't be a reference to prevent a loop with #1.
   */
  atomic_rwlock_endwrite(&node->i_file.i_files_lock);
 }
 return result;
}


PRIVATE DEFINE_RWLOCK(modloader_lock);
PRIVATE SLIST_HEAD(struct modloader) modloader_ymaglist = NULL; /*< [lock(modloader_lock)] List of loaders with magic. */
PRIVATE SLIST_HEAD(struct modloader) modloader_nmaglist = NULL; /*< [lock(modloader_lock)] List of loaders without magic. */

PUBLIC SAFE REF struct module *KCALL
module_open_new(struct file *__restrict fp) {
 REF struct module *result = E_PTR(-ENOEXEC);
 struct modloader *iter; ssize_t magsz;
 errno_t error; byte_t magic[MODLOADER_MAX_MAGIC];
 CHECK_HOST_DOBJ(fp);
 magsz = file_kread(fp,magic,MODLOADER_MAX_MAGIC);
 if (E_ISERR(magsz)) return E_PTR(magsz);
 if (!magsz) goto end;
 /* Seek back before the magic header. */
 error = (errno_t)file_seek(fp,(off_t)-magsz,SEEK_CUR);
 if (E_ISERR(error)) return E_PTR(error);
 /* Sadly, we need to keep this lock while executing the loaders... */
 error = rwlock_read(&modloader_lock);
 if (E_ISERR(error)) return E_PTR(error);
 if (magsz) {
  for (iter = modloader_ymaglist; iter;
       iter = iter->ml_chain.le_next) {
   if ((size_t)magsz >= iter->ml_magsz &&
       !memcmp(iter->ml_magic,magic,iter->ml_magsz) &&
        INSTANCE_LOCKWEAK(iter->ml_owner)) {
    /* Found a match! */
    result = (*iter->ml_loader)(fp);
    INSTANCE_DECREF(iter->ml_owner);
    assert(result);
    if (result != E_PTR(-ENOEXEC)) goto end2;
   }
  }
 }
 /* Check the magic-less loaders. */
 for (iter = modloader_nmaglist; iter;
      iter = iter->ml_chain.le_next) {
  if (INSTANCE_LOCKWEAK(iter->ml_owner)) {
   result = (*iter->ml_loader)(fp);
   INSTANCE_DECREF(iter->ml_owner);
   assert(result);
   if (result != E_PTR(-ENOEXEC)) goto end2;
  }
 }
end2: rwlock_endread(&modloader_lock);
end:  return result;
}

PUBLIC SAFE void KCALL
module_addloader(struct modloader *__restrict loader, int mode) {
 CHECK_HOST_DOBJ(loader);
 assert(TASK_ISSAFE());
 assertf(loader->ml_owner,"No loader owner set (Use 'THIS_INSTANCE')");
 CHECK_HOST_DOBJ(loader->ml_owner);
 CHECK_HOST_TEXT(loader->ml_loader,1);
 assert(loader->ml_magsz <= MODLOADER_MAX_MAGIC);
 task_nointr();
 rwlock_write(&modloader_lock);
 /* Lets not deal with interrupts for this... */
 if likely(!INSTANCE_ISUNLOADING(loader->ml_owner)) {
  INSTANCE_WEAK_INCREF(loader->ml_owner);
  if (loader->ml_magsz) {
   if (!(mode&MODULE_LOADER_SECONDARY)) {
    /* Override a primary module loader when secondary is disabled. */
    struct modloader **piter,*iter;
    piter = &modloader_ymaglist;
    while ((iter = *piter) != NULL) {
     if (iter->ml_magsz == loader->ml_magsz &&
        !memcmp(iter->ml_magic,loader->ml_magic,loader->ml_magsz)) {
      struct instance *old_owner;
      /* Overwrite this entry. */
      old_owner                = iter->ml_owner;
      loader->ml_chain.le_next = iter->ml_chain.le_next;
      *piter                   = loader;
      rwlock_endwrite(&modloader_lock);
      task_endnointr();
      INSTANCE_WEAK_DECREF(old_owner);
      return;
     }
     piter = &iter->ml_chain.le_next;
    }
   }
   SLIST_INSERT(modloader_ymaglist,loader,ml_chain);
  } else {
   SLIST_INSERT(modloader_nmaglist,loader,ml_chain);
  }
 }
 rwlock_endwrite(&modloader_lock);
 task_endnointr();
}
PUBLIC bool KCALL
module_delloader(struct modloader *__restrict loader) {
 struct modloader **piter,*iter;
 bool result = false;
 CHECK_HOST_DOBJ(loader);
 assertf(loader->ml_owner,"No loader owner set (Use 'THIS_INSTANCE')");
 CHECK_HOST_DOBJ(loader->ml_owner);
 CHECK_HOST_TEXT(loader->ml_loader,1);
 assert(ATOMIC_READ(loader->ml_owner->i_refcnt) >= 1);
 assert(loader->ml_magsz <= MODLOADER_MAX_MAGIC);
 /* Lets not deal with interrupts for this... */
 task_nointr();
 rwlock_write(&modloader_lock);
 piter = loader->ml_magsz ? &modloader_ymaglist : &modloader_nmaglist;
 while ((iter = *piter) != NULL) {
  if (iter == loader) {
   /* Unlink this entry. */
   *piter = loader->ml_chain.le_next;
   result = true;
   goto done;
  }
  piter = &iter->ml_chain.le_next;
 }
done:
 rwlock_endwrite(&modloader_lock);
 task_endnointr();
 if (result) INSTANCE_WEAK_DECREF(loader->ml_owner);
 return result;
}






PUBLIC errno_t KCALL
module_mkregions(struct module *__restrict self) {
 struct modseg *iter,*end;
 struct mregion *region;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->m_rlock);
 /* Start mapping! */
 end = (iter = self->m_segv)+self->m_segc;
 for (; iter != end; ++iter) {
  if (!iter->ms_region) {
   uintptr_t page_offset;
   if (atomic_rwlock_upgrade(&self->m_rlock) &&
       iter->ms_region != NULL) {
    atomic_rwlock_downgrade(&self->m_rlock);
    continue;
   }
   assert(!iter->ms_region);
   /* Allocate a new region usable by any memory manager. */
   region = mregion_new(MMAN_UNIGFP);
   if unlikely(!region) {
    atomic_rwlock_endwrite(&self->m_rlock);
    return -ENOMEM;
   }
   /* Setup the region. */
   region->mr_size = iter->ms_msize;
   iter->ms_region = region; /* Inherit reference. */
   page_offset = iter->ms_paddr & (PAGESIZE-1);
   /* XXX: These are incorrect. - We must start mapping the file deeper into the region. */
   if (iter->ms_fsize == 0) {
    /* Pure .bss section. */
    if (iter->ms_fill) {
     region->mr_init           = MREGION_INIT_BYTE;
     region->mr_setup.mri_byte = iter->ms_fill;
    } else {
     region->mr_init = MREGION_INIT_ZERO;
    }
   } else {
    region->mr_init             = MREGION_INIT_FILE;
    region->mr_setup.mri_file   = self->m_file;
    region->mr_setup.mri_start  = iter->ms_fpos;
    region->mr_setup.mri_size   = iter->ms_fsize;
    region->mr_setup.mri_byte   = iter->ms_fill;
    region->mr_setup.mri_begin  = page_offset;
    FILE_INCREF(self->m_file);
   }
   region->mr_size += page_offset;
   region->mr_size  = CEIL_ALIGN(region->mr_size,PAGESIZE);
#if 0
   syslog(LOG_EXEC|LOG_DEBUG,"[MOD] SEGMENT '%[file]' - %p...%p from %I64X + %Ix\n",
           region->mr_setup.mri_file,
           FLOOR_ALIGN(iter->ms_paddr,PAGESIZE)+region->mr_setup.mri_begin,
           FLOOR_ALIGN(iter->ms_paddr,PAGESIZE)+region->mr_size-1,
           region->mr_setup.mri_start,
           region->mr_setup.mri_size);
#endif
   mregion_setup(region);
   atomic_rwlock_downgrade(&self->m_rlock);
  }
 }
 atomic_rwlock_endread(&self->m_rlock);
 return -EOK;
}

PUBLIC SAFE errno_t KCALL
module_restore_readonly(struct module *__restrict self,
                        ppage_t load_addr) {
 struct modseg *iter,*end;
 struct mman *mm = NULL;
 ssize_t error;
 /* Restore original memory permissions. */
 end = (iter = self->m_segv)+self->m_segc;
 error = -EOK;
 for (; iter != end; ++iter) {
  if (iter->ms_prot&PROT_WRITE) continue;
  if (!mm) {
   mm = THIS_TASK->t_mman;
   error = mman_write(mm);
   if (E_ISERR(error)) return (errno_t)error;
  }
  error = mman_mprotect_unlocked(mm,
                                (ppage_t)((uintptr_t)load_addr+FLOOR_ALIGN(iter->ms_paddr,PAGESIZE)),
                                 iter->ms_region->mr_size,(u32)~PROT_WRITE,0);
  if (E_ISERR(error)) break;
 }
 if (mm) mman_endwrite(mm);
 return (errno_t)error;
}



#ifdef CONFIG_TRACE_LEAKS
INTDEF void (KCALL debug_add2core)(struct instance *__restrict inst);
#endif

PUBLIC SAFE REF struct instance *KCALL
instance_new(struct module *__restrict mod, u32 flags) {
 REF struct instance *result;
 CHECK_HOST_DOBJ(mod);
 assert(mod->m_refcnt);
 result = (REF struct instance *)malloc(flags&INSTANCE_FLAG_KERNEL
                                        ? sizeof(struct instance)
                                        : offsetof(struct instance,i_driver));
 if unlikely(!result) return NULL;
 result->i_branch  = 0;
 result->i_refcnt  = 1;
 result->i_weakcnt = 1;
 result->i_module  = mod;
 result->i_flags   = flags;
 /* TODO: Set the 'INSTANCE_FLAG_NOREMAP' flag if the
  *       application module 'mod' is allowed to root-fork().
  *    >> Required to prevent tampering (and breaking the dirty-bit-check)
  *       when a malicious application loads a root-fork-able module:
  * NOTE: The 'INSTANCE_FLAG_NOUNMAP' flag must be set similarly,
  *       but unmapping must still be allowed when the entirety
  *       of the instance is being deleted.
  * >> void dirty_code(void) {
  * >>     // Use indirect function calls to remain position-independent
  * >>     int (*volatile prootfork)(void) = &rootfork;
  * >>     void (*volatile psystem)(char const *) = &system;
  * >>     if ((*prootfork)() == 0) {
  * >>        (*pfun)("chmod +s /bin/virus");
  * >>     }
  * >> }
  * >> 
  * >> ...
  * >> 
  * >> void *l = dlopen("/lib/passwd.so"); // 'l' is also the module's base address.
  * >> void *p = dlsym(l,".text");
  * >> // Acquire write-access to the library's text section.
  * >> mprotect((void *)FLOOR_ALIGN((uintptr_t)p,PAGESIZE),2*PAGESIZE);
  * >> 
  * >> // Copy our evil code inside. (This will set the dirty bits, preventing rootfork() from working)
  * >> memcpy(p,&dirty_code,PAGESIZE);
  * >> 
  * >> // Work-around (prevented by disallowing re-map for rootfork()-modules)
  * >> // >> When it is to do this, 'mremap()' will fail with 'EPERM'
  * >> p = mremap(p,PAGESIZE,PAGESIZE,MREMAP_FIXED,0x10000); // New address must differ from old...
  * >> 
  * >> // Execute the relocated copy of 'dirty_code'
  * >> (*(void(*)(void))p)();
  */
 if (flags&INSTANCE_FLAG_KERNEL) {
#ifdef CONFIG_TRACE_LEAKS
  atomic_rwlock_init(&result->i_driver.k_tlock);
  result->i_driver.k_trace = KINSTANCE_TRACE_NULL;
#endif
 }
 MODULE_INCREF(mod);
 return result;
}

PUBLIC SAFE void KCALL
instance_callinit(struct instance *__restrict self) {
 struct moduleops const *ops;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->i_module);
 assert(task_issafe());
 assert(self->i_refcnt);
 assert(self->i_module->m_refcnt);
 /* Ensure no-op on non-driver, or empty instances. */
 if unlikely(!INSTANCE_INKERNEL(self) ||
              INSTANCE_ISEMPTY(self)) return;
 ops = self->i_module->m_ops;
 CHECK_HOST_TOBJ(ops);
 if (ops->o_exec_init)
   (*ops->o_exec_init)(self->i_module,self->i_base);
}
PUBLIC SAFE void KCALL
instance_callfini(struct instance *__restrict self) {
 struct moduleops const *ops;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->i_module);
 assert(task_issafe());
 assert(self->i_refcnt);
 assert(self->i_module->m_refcnt);
 /* Ensure no-op on non-driver, or empty instances. */
 if unlikely(!INSTANCE_INKERNEL(self) ||
              INSTANCE_ISEMPTY(self)) return;
 ops = self->i_module->m_ops;
 CHECK_HOST_TOBJ(ops);
 if (ops->o_exec_fini)
   (*ops->o_exec_fini)(self->i_module,self->i_base);
}

PUBLIC ssize_t KCALL
instance_mnotify(unsigned int type, void *__restrict closure,
                 struct mman *mm, ppage_t UNUSED(addr),
                 size_t UNUSED(size)) {
#define I ((struct instance *)closure)
 CHECK_HOST_DOBJ(I);
 (void)mm;
 assert(MNOTIFY_HASM(type) ?
       (mm == &mman_kernel || !INSTANCE_INKERNEL(I)) : 1);
#ifdef CONFIG_DEBUG
 assert(I->i_branch != 0);
#else
 assert(type == MNOTIFY_INCREF || I->i_branch != 0);
#endif

 switch (type) {

 /* Increment/Decrement the branch counter of the given instance.
  * NOTE: The caller must be holding a write-lock on the associated memory manager! */
 case MNOTIFY_INCREF:
  /* Don't allow new branch nodes being created when the module is being unloaded. */
  if (INSTANCE_ISUNLOADING(I))
      return -EPERM;
  ++I->i_branch;
  break;

 case MNOTIFY_DECREF:
  if (!--I->i_branch) {
   assert(I->i_refcnt);
   LIST_REMOVE(I,i_chain);
   INSTANCE_DECREF(I);
  }
  break;

  /* Additional hooks for implementing unmap()/remap()-restrictions. */
 case MNOTIFY_EXTRACT:
  if (I->i_flags&INSTANCE_FLAG_NOREMAP)
      return -EPERM;
  break;

 case MNOTIFY_UNMAP:
  /* NOTE: Must allow 'INSTANCE_FLAG_DID_UNLOAD' to overrule no-unmap
   *       to allow the kernel to delmod() drivers once it determines
   *       that they've been unloaded (by confirming a reference
   *       counter of ONE(2) indicating that only its memory branches
   *       remain, as well as the last reference used for ) */
  if ((ATOMIC_READ(I->i_flags)&(INSTANCE_FLAG_NOUNMAP|
                                INSTANCE_FLAG_DID_UNLOAD)) == 
                               (INSTANCE_FLAG_NOUNMAP))
      return -EPERM;
  break;

 default: break;
 }
 return -EOK;
#undef I
}

PUBLIC SAFE void KCALL
instance_destroy(struct instance *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(!self->i_refcnt);
 assert(!self->i_branch);
 /* NOTE: Since instances are tracked through mman branches, the caller
  *       must have already unmapped all branches mapping parts of this
  *       instance. */
 if (INSTANCE_INKERNEL(self)) {
  /* Cleanup driver data. */
#ifdef CONFIG_TRACE_LEAKS
  debug_add2core(self);
#endif
 }
 CHECK_HOST_DOBJ(self->i_module);
 MODULE_DECREF(self->i_module);
 INSTANCE_WEAK_DECREF(self);
}

PUBLIC SAFE void KCALL
instance_destroy_weak(struct instance *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(!self->i_refcnt);
 assert(!self->i_branch);
 assert(!self->i_weakcnt);
 free(self);
}

DECL_END

#endif /* !GUARD_KERNEL_LINKER_MODULE_C */
