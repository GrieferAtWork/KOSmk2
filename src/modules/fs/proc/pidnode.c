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
#ifndef GUARD_MODULES_FS_PROC_PIDNODE_C
#define GUARD_MODULES_FS_PROC_PIDNODE_C 1
#define _KOS_SOURCE 1

#include "taskutil.h"
#include "procfs.h"

#include <dev/blkdev.h>
#include <dirent.h>
#include <fcntl.h>
#include <fs/basic_types.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <fs/textfile.h>
#include <fs/vfs.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kernel/export.h>
#include <kernel/mman.h>
#include <kernel/user.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/task.h>
#include <sched/types.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <sched/paging.h>
#include <linker/module.h>

DECL_BEGIN

#define SELF container_of(ino,struct pidnode,p_node)

PRIVATE SAFE ssize_t KCALL
pid_fd_readlink(struct pidnode *__restrict self,
                USER char *__restrict buf,
                size_t bufsize, int fdno) {
 ssize_t result; REF struct dentry *dent;
 REF struct fdman *fdm = task_getfdman(self->p_task);
 if (E_ISERR(fdm)) return E_GTERR(fdm);
 /* XXX: What amount other things, but as sockets? */
 dent = fdman_get_dentry(fdm,fdno);
 if (E_ISERR(dent))
  result = E_GTERR(dent);
 else {
  /* Simply use printf to generate the correct, absolute path for the dentry.
   * NOTE: This will automatically use the proper root directory to
   *       prevent information leaks within chroot()-jails. */
  result = snprintf_user(buf,bufsize,"%[dentry]",dent);
  DENTRY_DECREF(dent);
 }
 FDMAN_DECREF(fdm);
 return result;
}
PRIVATE SAFE ssize_t KCALL
pid_cwd_readlink(struct inode *__restrict ino,
                 USER char *__restrict buf, size_t bufsize) {
 return pid_fd_readlink(SELF,buf,bufsize,AT_FDCWD);
}
PRIVATE SAFE ssize_t KCALL
pid_root_readlink(struct inode *__restrict ino,
                  USER char *__restrict buf, size_t bufsize) {
 return pid_fd_readlink(SELF,buf,bufsize,AT_FDROOT);
}
PRIVATE SAFE ssize_t KCALL
pid_exe_readlink(struct inode *__restrict ino,
                 USER char *__restrict buf, size_t bufsize) {
 REF struct mman *mm; ssize_t result;
 REF struct instance *inst;
 mm = task_getmman(SELF->p_task);
 if (E_ISERR(mm)) return E_GTERR(mm);
 inst = mman_getexe(mm);
 MMAN_DECREF(mm);
 if (E_ISERR(inst)) return E_GTERR(inst);
 if (inst == &kernel_instance) {
  PRIVATE char const kernel_exe[] = "[KERNEL]";
  /* Special case: The kernel core has no executable. */
  result = sizeof(kernel_exe);
  if (copy_to_user(buf,kernel_exe,MIN(bufsize,sizeof(kernel_exe))))
      result = -EFAULT;
 } else {
  /* Simply print the filename of the underlying executable. */
  result = snprintf_user(buf,bufsize,"%[file]",inst->i_module->m_file);
 }
 INSTANCE_DECREF(inst);
 return result;
}
#undef SELF
PRIVATE REF struct file *KCALL
maps_fopen(struct inode *__restrict ino,
           struct dentry *__restrict node_ent,
           oflag_t oflags) {
 REF struct textfile *result;
 REF struct mman *mm; errno_t error;
 result = textfile_new();
 if unlikely(!result) return E_PTR(-ENOMEM);
 mm = task_getmman(((struct pidnode *)ino)->p_task);
 if (E_ISERR(mm)) { error = E_GTERR(mm); goto err; }
 /* NOTE: Must acquire a write-lock because this code may not be locked in-core. */
 error = mman_write(mm);
 if (E_ISERR(error)) goto err2;
 if (mm == &mman_kernel) {
  struct mman *omm;
  TASK_PDIR_KERNEL_BEGIN(omm);
  error = mman_print_unlocked(mm,&textfile_printer,result);
  TASK_PDIR_KERNEL_END(omm);
 } else {
  error = mman_print_unlocked(mm,&textfile_printer,result);
 }
 if (E_ISERR(error)) goto err3;
 mman_endwrite(mm);
 MMAN_DECREF(mm);
 textfile_truncate(result);
 file_setup(&result->tf_file,ino,node_ent,oflags);
 return &result->tf_file;
err3: rwlock_endread(&fstype_lock);
err2: MMAN_DECREF(mm);
err:  textfile_delete(result);
 return E_PTR(error);
}

INTERN struct procnode const pid_content[] = {
 {0,S_IFLNK|0444,/*[[[deemon DNAM("cwd"); ]]]*/{"cwd",3,H(6584163u,6584163llu)}/*[[[end]]]*/,
 { .ino_readlink = &pid_cwd_readlink,
 }},
 {1,S_IFLNK|0444,/*[[[deemon DNAM("root"); ]]]*/{"root",4,H(401271554u,1953460082llu)}/*[[[end]]]*/,
 { .ino_readlink = &pid_root_readlink,
 }},
 {2,S_IFLNK|0444,/*[[[deemon DNAM("exe"); ]]]*/{"exe",3,H(6649957u,6649957llu)}/*[[[end]]]*/,
 { .ino_readlink = &pid_exe_readlink,
 }},
 {3,S_IFREG|0444,/*[[[deemon DNAM("maps"); ]]]*/{"maps",4,H(250834133u,1936744813llu)}/*[[[end]]]*/,
 { .ino_fopen = &maps_fopen, TEXTFILE_OPS_INIT
 }},
};

PRIVATE REF struct file *KCALL
pidnode_fopen(struct inode *__restrict ino,
              struct dentry *__restrict node_ent,
              oflag_t oflags) {
 REF struct pidfile *result;
 result = (REF struct pidfile *)file_new(sizeof(struct pidfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 assert(result->p_dirx == 0);
 file_setup(&result->p_file,ino,node_ent,oflags);
 return &result->p_file;
}
#define SELF container_of(fp,struct pidfile,p_file)
PRIVATE ssize_t KCALL
pidnode_readdir(struct file *__restrict fp,
                USER struct dirent *buf,
                size_t bufsize, rdmode_t mode) {
 struct dirent header; size_t name_avail;
 struct procnode const *node;
 ssize_t result = 0;
 if (SELF->p_dirx >= COMPILER_LENOF(pid_content))
     return 0; /* End of directory */
 node = &pid_content[SELF->p_dirx];
 header.d_ino    = node->n_ino; /* TODO: + pid*MAX_COUNT_OF_PER_PID_NODES; */
 header.d_namlen = node->n_name.dn_size;
 header.d_type   = IFTODT(node->n_mode);
 result = offsetof(struct dirent,d_name)+(1+node->n_name.dn_size)*sizeof(char);
 name_avail = 0;
 if (bufsize >= offsetof(struct dirent,d_name)) {
  if (copy_to_user(buf,&header,offsetof(struct dirent,d_name)))
      return -EFAULT;
  name_avail = bufsize-offsetof(struct dirent,d_name);
  *(uintptr_t *)&buf += offsetof(struct dirent,d_name);
 }
 if (copy_to_user(buf,node->n_name.dn_name,
                  MIN((node->n_name.dn_size+1)*sizeof(char),
                  name_avail)))
     return -EFAULT;
 /* Increment the directory pointer if we're supposed to. */
 if (FILE_READDIR_SHOULDINC(mode,bufsize,result))
   ++SELF->p_dirx;
 return result;
}
#undef SELF

#define SELF container_of(dir_node,struct pidnode,p_node)
PRIVATE REF struct inode *KCALL
pidnode_lookup(struct inode *__restrict dir_node,
               struct dentry *__restrict result_path) {
 struct procnode const *iter; errno_t error;
 REF struct pidnode *result;
 for (iter = pid_content;
      iter != COMPILER_ENDOF(pid_content); ++iter) {
#ifdef CONFIG_DEBUG
  struct dentryname name_copy;
  name_copy.dn_name = iter->n_name.dn_name;
  name_copy.dn_size = iter->n_name.dn_size;
  dentryname_loadhash(&name_copy);
  assertf(name_copy.dn_hash == iter->n_name.dn_hash,
          "Broken hash generator (%Ix != %Ix)",
          name_copy.dn_hash,iter->n_name.dn_hash);
#endif
  if (iter->n_name.dn_hash == result_path->d_name.dn_hash &&
      iter->n_name.dn_size == result_path->d_name.dn_size &&
      memcmp(iter->n_name.dn_name,result_path->d_name.dn_name,
             result_path->d_name.dn_size*sizeof(char)) == 0) {
   /* Found it! */
   result = (struct pidnode *)inode_new(sizeof(struct pidnode));
   if unlikely(!result) return E_PTR(-ENOMEM);
   result->p_task = SELF->p_task;
   CHECK_HOST_DOBJ(result->p_task);
   TASK_WEAK_INCREF(result->p_task);
   result->p_node.i_ino = iter->n_ino;
   result->p_node.i_ops = &iter->n_ops;
   result->p_node.i_attr.ia_mode = iter->n_mode;
   result->p_node.i_attr_disk.ia_mode = iter->n_mode;
   error = inode_setup(&result->p_node,dir_node->i_super,THIS_INSTANCE);
   if (E_ISERR(error)) { free(result); result = E_PTR(error); }
   return &result->p_node;
  }
 }
 return E_PTR(-ENOENT);
}
#undef SELF

#define SELF container_of(ino,struct pidnode,p_node)
PRIVATE void KCALL pidnode_fini(struct inode *__restrict ino) {
 /* Cleanup the weak reference to the represented task. */
 TASK_WEAK_DECREF(SELF->p_task);
}
#undef SELF

INTERN struct inodeops const pidops = {
    .ino_fini   = &pidnode_fini,
    .ino_fopen  = &pidnode_fopen,
    .ino_lookup = &pidnode_lookup,
    .f_readdir  = &pidnode_readdir,
};


/* Create a new PID INode and _ALWAYS_ inherit a weak reference to 't' */
INTERN REF struct pidnode *KCALL
pidnode_new_inherited(struct superblock *__restrict procfs,
                      WEAK REF struct task *__restrict t) {
 REF struct pidnode *result; errno_t error;
 result = (REF struct pidnode *)inode_new(sizeof(struct pidnode));
 if unlikely(!result) { result = E_PTR(-ENOMEM); goto err; }
 /* Initialize basic node members. */
 result->p_node.__i_nlink           = 1;
 result->p_node.i_ops               = &pidops;
 result->p_node.i_attr.ia_mode      = S_IFDIR|0555;
 result->p_node.i_attr_disk.ia_mode = S_IFDIR|0555;
 /* Store the given task as a weak reference within the node. */
 result->p_task = t;
 error = inode_setup(&result->p_node,procfs,THIS_INSTANCE);
 if (E_ISERR(error)) {
  free(result);
  result = E_PTR(error);
err:
  TASK_WEAK_DECREF(t);
 }
 return result;
}



DECL_END

#endif /* !GUARD_MODULES_FS_PROC_PIDNODE_C */
