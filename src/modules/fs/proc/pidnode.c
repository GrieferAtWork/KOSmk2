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
#include <fs/memfile.h>
#include <kos/environ.h>
#include <kernel/stack.h>

DECL_BEGIN
PRIVATE void KCALL pidnode_fini(struct inode *__restrict ino);

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
PRIVATE REF struct file *KCALL
maps_fopen(struct inode *__restrict ino,
           struct dentry *__restrict node_ent,
           oflag_t oflags) {
 REF struct textfile *result;
 REF struct mman *mm; errno_t error;
 result = textfile_new();
 if unlikely(!result) return E_PTR(-ENOMEM);
 mm = task_getmman(SELF->p_task);
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
FUNDEF REF struct file *KCALL
pid_cmdline_fopen(struct inode *__restrict ino,
                  struct dentry *__restrict dent,
                  oflag_t oflags) {
 REF struct mman *mm = task_getmman(SELF->p_task);
 REF struct file *result; USER char *argtxt; size_t argsiz;
 if unlikely(E_ISERR(mm)) return E_PTR(E_GTERR(mm));
 argtxt = MMAN_ENVIRON_ARGTXT(mm);
 argsiz = MMAN_ENVIRON_ARGSIZ(mm);
 if (!argsiz) argtxt = (USER char *)1;
 result = make_memfile(ino,dent,oflags,mm,
                      (uintptr_t)argtxt,
                      (uintptr_t)argtxt+(argsiz-1));
 MMAN_DECREF(mm);
 return result;
}
FUNDEF REF struct file *KCALL
pid_environ_fopen(struct inode *__restrict ino,
                  struct dentry *__restrict dent,
                  oflag_t oflags) {
 REF struct mman *mm = task_getmman(SELF->p_task);
 REF struct file *result; USER char *envtxt; size_t envsiz;
 if unlikely(E_ISERR(mm)) return E_PTR(E_GTERR(mm));
 envtxt = MMAN_ENVIRON_ENVTXT(mm);
 envsiz = MMAN_ENVIRON_ENVSIZ(mm);
 if (!envsiz) envtxt = (USER char *)1;
 result = make_memfile(ino,dent,oflags,mm,
                      (uintptr_t)envtxt,
                      (uintptr_t)envtxt+(envsiz-1));
 MMAN_DECREF(mm);
 return result;
}
FUNDEF REF struct file *KCALL
pid_mem_fopen(struct inode *__restrict ino,
              struct dentry *__restrict dent,
              oflag_t oflags) {
 REF struct mman *mm = task_getmman(SELF->p_task);
 REF struct file *result;
 if unlikely(E_ISERR(mm)) return E_PTR(E_GTERR(mm));
 result = make_memfile(ino,dent,oflags,mm,0,(uintptr_t)-1);
 MMAN_DECREF(mm);
 return result;
}
#undef SELF

#define SELF container_of(fp,struct taskfile,tf_file)
PRIVATE ssize_t KCALL
taskfile_readdir(struct file *__restrict fp,
                 USER struct dirent *buf,
                 size_t bufsize, rdmode_t mode,
                 bool read_children) {
 struct dirent header; size_t name_avail;
 ssize_t result; REF struct task *leader,*member;
 pid_t taskpid;
 leader = SELF->tf_leader;
 if unlikely(!TASK_TRYINCREF(leader)) return 0;
 name_avail = SELF->tf_grpidx;
 atomic_rwlock_read(read_children ? &leader->t_pid.tp_grouplock
                                  : &leader->t_pid.tp_childlock);
 member = read_children ? leader->t_pid.tp_children : leader->t_pid.tp_group;
 /* Prevent infinite recursion in recursive directory listings. */
 while (member && (member == leader || name_avail--)) {
  member = read_children ? member->t_pid.tp_siblings.le_next
                         : member->t_pid.tp_grplink.le_next;
 }
 if (member) {
  taskpid = member->t_pid.tp_ids[PIDTYPE_PID].tl_ns == THIS_NAMESPACE
          ? member->t_pid.tp_ids[PIDTYPE_PID].tl_pid : 0;
 }
 atomic_rwlock_endread(read_children ? &leader->t_pid.tp_grouplock
                                     : &leader->t_pid.tp_childlock);
 TASK_DECREF(leader);
 if (!member) return 0;
 header.d_ino  = INO_FROM_PID(taskpid);
 header.d_type = DT_DIR;
 name_avail    = 0;
 if (bufsize >= offsetof(struct dirent,d_name))
     name_avail = bufsize-offsetof(struct dirent,d_name);
 result = snprintf_user(buf->d_name,name_avail,PID_FMT,taskpid)*sizeof(char);
 if (bufsize >= offsetof(struct dirent,d_name)) {
  header.d_namlen = result-1;
  if (copy_to_user(buf,&header,offsetof(struct dirent,d_name)))
      return -EFAULT;
 }
 result += offsetof(struct dirent,d_name);
 if (FILE_READDIR_SHOULDINC(mode,bufsize,result))
   ++SELF->tf_grpidx;
 return result;
}
PRIVATE void KCALL
taskfile_fclose(struct inode *__restrict ino,
                struct file *__restrict fp) {
 /* Drop the weak reference from the task. */
 TASK_WEAK_DECREF(SELF->tf_leader);
}
#undef SELF

#define SELF container_of(ino,struct pidnode,p_node)
PRIVATE REF struct file *KCALL
taskfile_fopen(struct inode *__restrict ino,
               struct dentry *__restrict node_ent,
               oflag_t oflags) {
 /* Create a new file descriptor for /proc/PID/task or /proc/PID/children */
 REF struct taskfile *result;
 result = (REF struct taskfile *)file_new(sizeof(struct taskfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 /* Store a weak reference to the task being enumerated. */
 result->tf_leader = SELF->p_task;
 TASK_WEAK_INCREF(result->tf_leader);
 file_setup(&result->tf_file,ino,node_ent,oflags);
 return &result->tf_file;
}
#undef SELF

PRIVATE ssize_t KCALL
pid_task_readdir(struct file *__restrict fp,
                 USER struct dirent *buf,
                 size_t bufsize, rdmode_t mode) {
 return taskfile_readdir(fp,buf,bufsize,mode,false);
}
PRIVATE ssize_t KCALL
pid_children_readdir(struct file *__restrict fp,
                     USER struct dirent *buf,
                     size_t bufsize, rdmode_t mode) {
 return taskfile_readdir(fp,buf,bufsize,mode,true);
}

#define SELF container_of(dir_node,struct pidnode,p_node)
PRIVATE REF struct inode *KCALL
pid_task_lookup(struct inode *__restrict dir_node,
                struct dentry *__restrict result_path,
                int UNUSED(flags)) {
 pid_t refpid = pid_from_string(result_path->d_name.dn_name,
                                result_path->d_name.dn_size);
 if (refpid >= 0) {
  WEAK REF struct task *tsk = file_gettask_pid(SELF->p_task,refpid);
  if (tsk) return (struct inode *)pidnode_new_inherited(dir_node->i_super,tsk);
 }
 return E_PTR(-ENOENT);
}
PRIVATE REF struct inode *KCALL
pid_child_lookup(struct inode *__restrict dir_node,
                 struct dentry *__restrict result_path,
                 int UNUSED(flags)) {
 pid_t refpid = pid_from_string(result_path->d_name.dn_name,
                                result_path->d_name.dn_size);
 if (refpid >= 0) {
  WEAK REF struct task *tsk = file_getchild_pid(SELF->p_task,refpid);
  if (tsk) return (struct inode *)pidnode_new_inherited(dir_node->i_super,tsk);
 }
 return E_PTR(-ENOENT);
}
#undef SELF


#define SELF container_of(ino,struct pidnode,p_node)
FUNDEF REF struct file *KCALL
pid_fd_fopen(struct inode *__restrict ino,
             struct dentry *__restrict dent,
             oflag_t oflags) {
 REF struct pidfdfile *result;
 result = (REF struct pidfdfile *)file_new(sizeof(struct pidfdfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 assert(result->ff_fdidx == 0);
 /* Set the FD-file's associated task. */
 result->ff_task = SELF->p_task;
 TASK_WEAK_INCREF(result->ff_task);
 file_setup(&result->ff_file,ino,dent,oflags);
 return &result->ff_file;
}
#undef SELF

#define SELF container_of(fp,struct pidfdfile,ff_file)
PRIVATE void KCALL
pid_fd_fclose(struct inode *__restrict ino,
              struct file *__restrict fp) {
 /* Drop the weak reference from the task. */
 TASK_WEAK_DECREF(SELF->ff_task);
}
PRIVATE ssize_t KCALL
pid_fd_readdir(struct file *__restrict fp,
               USER struct dirent *buf,
               size_t bufsize, rdmode_t mode) {
 struct dirent header; bool is_eod;
 size_t name_avail; ssize_t result;
 REF struct fdman *fdm = task_getfdman(SELF->ff_task);
 if (E_ISERR(fdm)) return E_GTERR(fdm);
 result = fdman_read(fdm);
 if (E_ISERR(result)) goto end;
 /* Skip empty descriptors. */
 while (SELF->ff_fdidx < fdm->fm_veca &&
        FD_SAFE_ISNULL(fdm->fm_vecv[SELF->ff_fdidx]))
        ++SELF->ff_fdidx;
 is_eod = SELF->ff_fdidx >= fdm->fm_veca;
 fdman_endread(fdm);
 FDMAN_DECREF(fdm);
 if (is_eod) return 0;
 header.d_ino  = 0; /* ??? */
 header.d_type = DT_LNK;
 name_avail    = 0;
 if (bufsize >= offsetof(struct dirent,d_name))
     name_avail = bufsize-offsetof(struct dirent,d_name);
 /* Copy data to user-space. */
 result = snprintf_user(buf->d_name,name_avail,"%u",SELF->ff_fdidx)*sizeof(char);
 if (bufsize >= offsetof(struct dirent,d_name)) {
  header.d_namlen = result-1;
  if (copy_to_user(buf,&header,offsetof(struct dirent,d_name)))
  { result = -EFAULT; goto end; }
 }
 result += offsetof(struct dirent,d_name);
end:
 if (FILE_READDIR_SHOULDINC(mode,bufsize,result))
     ++SELF->ff_fdidx;
 return result;
}
#undef SELF
#define SELF container_of(dir_node,struct pidnode,p_node)
PRIVATE REF struct inode *KCALL
pid_fd_lookup(struct inode *__restrict dir_node,
              struct dentry *__restrict result_path,
              int UNUSED(flags)) {
 int fdno = fd_from_string(result_path->d_name.dn_name,
                           result_path->d_name.dn_size);
 if (fdno >= 0) {
  struct fd node_fd; REF struct fdman *fdm;
  fdm = task_getfdman(SELF->p_task);
  if (E_ISERR(fdm)) return E_PTR(E_GTERR(fdm));
  node_fd = fdman_get(fdm,fdno);
  FDMAN_DECREF(fdm);
  if (FD_ISNULL(node_fd)) goto noent;
  return (REF struct inode *)fdnode_new_inherited(dir_node->i_super,
                                                  SELF->p_task,
                                                  node_fd);
 }
noent:
 return E_PTR(-ENOENT);
}
#undef SELF


#define SELF container_of(ino,struct fdnode,fn_node)
PRIVATE void KCALL fdnode_fini(struct inode *__restrict ino) {
 FD_DECREF(SELF->fn_fd);
}
PRIVATE char const fdnull_text[] = "null";
PRIVATE char const vmnull_text[] = "vm:[?]";
PRIVATE char const vmkern_text[] = "vm:[KERNEL]";

PRIVATE ssize_t KCALL
fdnode_readlink(struct inode *__restrict ino,
                USER char *__restrict buf, size_t bufsize) {
#define FD  SELF->fn_fd
 switch (FD_TYPE(FD)) {

 case FD_TYPE_NULL:
  if (copy_to_user(buf,fdnull_text,MIN(bufsize,sizeof(fdnull_text))))
      return -EFAULT;
  return sizeof(fdnull_text);

 case FD_TYPE_FILE:
  return snprintf_user(buf,bufsize,"%[file]",FD.fo_obj.fo_file);

 case FD_TYPE_DENTRY:
  return snprintf_user(buf,bufsize,"%[dentry]",FD.fo_obj.fo_dentry);

 {
  struct superblock *super;
 case FD_TYPE_INODE:
  super = FD.fo_obj.fo_inode->i_super;
  return snprintf_user(buf,bufsize,"anon_inode:[%[dev_t],%I64u]",
                       super->sb_blkdev ? super->sb_blkdev->bd_device.d_id : 0,
                       FD.fo_obj.fo_inode->i_ino);
 } break;

 case FD_TYPE_TASK:
  return snprintf_user(buf,bufsize,"task:[" PID_FMT "]",
                       FD.fo_obj.fo_task->t_pid.tp_ids[PIDTYPE_PID].tl_ns == THIS_NAMESPACE
                     ? FD.fo_obj.fo_task->t_pid.tp_ids[PIDTYPE_PID].tl_pid : 0);

 {
  REF struct instance *vm_exe;
  ssize_t result;
 case FD_TYPE_MMAN:
  vm_exe = mman_getexe(FD.fo_obj.fo_mman);
  if (E_ISERR(vm_exe)) {
   if (copy_to_user(buf,vmnull_text,MIN(bufsize,sizeof(vmnull_text))))
       return -EFAULT;
   return sizeof(vmnull_text);
  }
  if (vm_exe == &kernel_instance) {
   if (copy_to_user(buf,vmkern_text,MIN(bufsize,sizeof(vmkern_text))))
       return -EFAULT;
   result = sizeof(vmkern_text);
  } else {
   result = snprintf_user(buf,bufsize,"vm:[%[file]]",
                          vm_exe->i_module->m_file);
  }
  INSTANCE_DECREF(vm_exe);
  return result;
 } break;

 case FD_TYPE_STACK:
  return snprintf_user(buf,bufsize,"stack:[%p,%p]",
                      (uintptr_t)ATOMIC_READ(FD.fo_obj.fo_stack->s_begin),
                      (uintptr_t)ATOMIC_READ(FD.fo_obj.fo_stack->s_end)-1);

 default: break;
 }
#ifdef CONFIG_DEBUG
 return snprintf_user(buf,bufsize,"fd:[UNKNOWN,%d,%p]",FD_TYPE(FD),FD.fo_ptr);
#else
 return snprintf_user(buf,bufsize,"fd:[UNKNOWN,%d]",FD_TYPE(FD));
#endif
#undef FD
}
#undef SELF

INTERN struct inodeops const fdnode_ops = {
   .ino_fini     = &fdnode_fini,
   .ino_readlink = &fdnode_readlink,
};


INTERN REF struct fdnode *KCALL
fdnode_new_inherited(struct superblock *__restrict procfs,
                     WEAK struct task *__restrict owner,
                     REF struct fd node_fd) {
 REF struct fdnode *result; errno_t error;
 result = (REF struct fdnode *)inode_new(sizeof(struct fdnode));
 if unlikely(!result) { result = E_PTR(-ENOMEM); goto err; }
 result->fn_fd                  = node_fd; /* Inherit reference. */
 result->fn_node.i_ops          = &fdnode_ops;
 result->fn_node.__i_nlink      = 1;
 result->fn_node.i_attr.ia_mode = 0700|S_IFLNK;
 result->fn_node.i_attr.ia_uid  = TASK_GETEUID(owner);
 result->fn_node.i_attr.ia_gid  = TASK_GETEGID(owner);
 memcpy(&result->fn_node.i_attr_disk,
        &result->fn_node.i_attr,
         sizeof(struct iattr));
 error = inode_setup(&result->fn_node,procfs,THIS_INSTANCE);
 if (E_ISERR(error)) { free(result); result = E_PTR(error); goto err; }
 return result;
err:
 FD_DECREF(node_fd);
 return result;
}



enum{__PID_FIRST_INO=(__COUNTER__+1)};
#define MKINO  (__COUNTER__-__PID_FIRST_INO)

INTERN struct procnode const pid_content[] = {
 {MKINO,S_IFLNK|0444,/*[[[deemon DNAM("cwd"); ]]]*/{"cwd",3,H(6584163u,6584163llu)}/*[[[end]]]*/,
 { .ino_fini = &pidnode_fini, .ino_readlink = &pid_cwd_readlink,
 }},
 {MKINO,S_IFLNK|0444,/*[[[deemon DNAM("root"); ]]]*/{"root",4,H(401271554u,1953460082llu)}/*[[[end]]]*/,
 { .ino_fini = &pidnode_fini, .ino_readlink = &pid_root_readlink,
 }},
 {MKINO,S_IFLNK|0444,/*[[[deemon DNAM("exe"); ]]]*/{"exe",3,H(6649957u,6649957llu)}/*[[[end]]]*/,
 { .ino_fini = &pidnode_fini, .ino_readlink = &pid_exe_readlink,
 }},
 {MKINO,S_IFREG|0444,/*[[[deemon DNAM("cmdline"); ]]]*/{"cmdline",7,H(3488433892u,28550371716918627llu)}/*[[[end]]]*/,
 { .ino_fini = &pidnode_fini, .ino_fopen = &pid_cmdline_fopen, MEMFILE_OPS_INIT
 }},
 {MKINO,S_IFREG|0444,/*[[[deemon DNAM("environ"); ]]]*/{"environ",7,H(3046658303u,31084784624496229llu)}/*[[[end]]]*/,
 { .ino_fini = &pidnode_fini, .ino_fopen = &pid_environ_fopen, MEMFILE_OPS_INIT
 }},
 {MKINO,S_IFREG|0600,/*[[[deemon DNAM("mem"); ]]]*/{"mem",3,H(7169389u,7169389llu)}/*[[[end]]]*/,
 { .ino_fini = &pidnode_fini, .ino_fopen = &pid_mem_fopen, MEMFILE_OPS_INIT
 }},
 {MKINO,S_IFDIR|0500,/*[[[deemon DNAM("fd"); ]]]*/{"fd",2,H(25702u,25702llu)}/*[[[end]]]*/,
 { .ino_fini = &pidnode_fini, .f_readdir = &pid_fd_readdir,
   .ino_fopen = &pid_fd_fopen, .ino_fclose = &pid_fd_fclose,
   .ino_lookup = &pid_fd_lookup, /* TODO: f_seek */
 }},
 {MKINO,S_IFDIR|0555,/*[[[deemon DNAM("task"); ]]]*/{"task",4,H(3339611412u,1802723700llu)}/*[[[end]]]*/,
 { .ino_fini = &pidnode_fini, .f_readdir = &pid_task_readdir,
   .ino_fopen = &taskfile_fopen, .ino_fclose = &taskfile_fclose,
   .ino_lookup = &pid_task_lookup, /* TODO: f_seek */
 }},
 {MKINO,S_IFDIR|0555,/*[[[deemon DNAM("children"); ]]]*/{"children",8,H(787156183u,16253778611020278651llu)}/*[[[end]]]*/,
 { .ino_fini = &pidnode_fini, .f_readdir = &pid_children_readdir,
   .ino_fopen = &taskfile_fopen, .ino_fclose = &taskfile_fclose,
   .ino_lookup = &pid_child_lookup, /* TODO: f_seek */
 }},
 {MKINO,S_IFREG|0444,/*[[[deemon DNAM("maps"); ]]]*/{"maps",4,H(250834133u,1936744813llu)}/*[[[end]]]*/,
 { .ino_fini = &pidnode_fini, .ino_fopen = &maps_fopen, TEXTFILE_OPS_INIT
 }},
 /* TODO: map_files */
 /* TODO: modules (Similar to `map_files', but a KOS extension directory listing
  *                all loaded modules as symlinks named by load-address) */
};
#undef MKINO

STATIC_ASSERT(COMPILER_LENOF(pid_content) <= PROC_ROOT_NUMNODES);


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
 { WEAK struct task *t = container_of(fp->f_node,struct pidnode,p_node)->p_task;
   pid_t pid = t->t_pid.tp_ids[PIDTYPE_PID].tl_ns == THIS_NAMESPACE
             ? t->t_pid.tp_ids[PIDTYPE_PID].tl_pid : 0;
   header.d_ino = node->n_ino+pid*PROC_PID_NUMNODES;
 }
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
               struct dentry *__restrict result_path,
               int flags) {
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
      iter->n_name.dn_size == result_path->d_name.dn_size) {
   if (memcmp(iter->n_name.dn_name,result_path->d_name.dn_name,
              result_path->d_name.dn_size*sizeof(char)) == 0) {
#ifndef CONFIG_NO_DOSFS
found_entry:
#endif /* !CONFIG_NO_DOSFS */
    /* Found it! */
    result = (struct pidnode *)inode_new(sizeof(struct pidnode));
    if unlikely(!result) return E_PTR(-ENOMEM);
    result->p_task = SELF->p_task;
    CHECK_HOST_DOBJ(result->p_task);
    TASK_WEAK_INCREF(result->p_task);
    { WEAK struct task *t = result->p_task;
      pid_t pid = t->t_pid.tp_ids[PIDTYPE_PID].tl_ns == THIS_NAMESPACE
                ? t->t_pid.tp_ids[PIDTYPE_PID].tl_pid : 0;
      result->p_node.i_ino = iter->n_ino+pid*PROC_PID_NUMNODES;
    }
    result->p_node.i_ops = &iter->n_ops;
    result->p_node.i_attr.ia_mode = iter->n_mode;
    result->p_node.i_attr_disk.ia_mode = iter->n_mode;
    error = inode_setup(&result->p_node,dir_node->i_super,THIS_INSTANCE);
    if (E_ISERR(error)) { free(result); result = E_PTR(error); }
    return &result->p_node;
   }
#ifndef CONFIG_NO_DOSFS
   if (flags&AT_DOSPATH &&
       memcasecmp(iter->n_name.dn_name,result_path->d_name.dn_name,
                  result_path->d_name.dn_size*sizeof(char)) == 0) {
    memcpy(result_path->d_name.dn_name,iter->n_name.dn_name,
           result_path->d_name.dn_size*sizeof(char));
    goto found_entry;
   }
#endif /* !CONFIG_NO_DOSFS */
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
 result->p_node.i_ino  = INO_FROM_PID(t->t_pid.tp_ids[PIDTYPE_PID].tl_ns == THIS_NAMESPACE
                                    ? t->t_pid.tp_ids[PIDTYPE_PID].tl_pid : 0);
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
