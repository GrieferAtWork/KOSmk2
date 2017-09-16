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
#ifndef GUARD_MODULES_FS_PROC_PROCFS_C
#define GUARD_MODULES_FS_PROC_PROCFS_C 1
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
#include <fs/vfs.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kernel/export.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/task.h>
#include <sched/types.h>
#include <stdlib.h>
#include <fs/textfile.h>
#include <kernel/boot.h>
#include <fs/memfile.h>
#include <kernel/mman.h>

DECL_BEGIN


PRIVATE SAFE ssize_t KCALL
self_readlink(struct inode *__restrict UNUSED(ino),
              USER char *__restrict buf, size_t bufsize) {
 return snprintf_user(buf,bufsize,PID_FMT,GET_THIS_PID());
}
PRIVATE errno_t KCALL
write_typechain(struct textfile *__restrict buf,
                struct fstype *chain, int *is_first) {
 ssize_t error;
 for (; chain; chain = chain->f_chain.le_next) {
  PRIVATE char const prefix[2][12] = {"\n        ","\nnodev   "};
  if (!chain->f_name || chain->f_flags&FSTYPE_HIDDEN)
      continue;
#if FSTYPE_NODEV == 1
  error = textfile_printer(prefix[chain->f_flags&FSTYPE_NODEV]+*is_first,11-*is_first,buf);
#else
  error = textfile_printer(prefix[chain->f_flags&FSTYPE_NODEV ? 1 : 0]+*is_first,11-*is_first,buf);
#endif
  if (E_ISERR(error)) return (errno_t)error;
  error = textfile_printer(chain->f_name,strlen(chain->f_name),buf);
  if (E_ISERR(error)) return (errno_t)error;
  *is_first = 0;
 }
 return -EOK;
}

PRIVATE REF struct file *KCALL
filesystems_fopen(struct inode *__restrict ino,
                  struct dentry *__restrict node_ent,
                  oflag_t oflags) {
 REF struct textfile *result; errno_t error; int is_first = 1;
 result = textfile_new();
 if unlikely(!result) return E_PTR(-ENOMEM);
 error = rwlock_read(&fstype_lock);
 if (E_ISERR(error)) goto err;
 if ((error = write_typechain(result,fstype_none,&is_first),E_ISERR(error))) goto err2;
 if ((error = write_typechain(result,fstype_auto,&is_first),E_ISERR(error))) goto err2;
 if ((error = write_typechain(result,fstype_any,&is_first),E_ISERR(error))) goto err2;
 if (!is_first && (error = textfile_printer("\n",1,result),E_ISERR(error))) goto err2;
 rwlock_endread(&fstype_lock);
 textfile_truncate(result);
 file_setup(&result->tf_file,ino,node_ent,oflags);
 return &result->tf_file;
err2: rwlock_endread(&fstype_lock);
err:  textfile_delete(result);
 return E_PTR(error);
}

PRIVATE REF struct file *KCALL
cmdline_fopen(struct inode *__restrict ino,
              struct dentry *__restrict node_ent,
              oflag_t oflags) {
 if unlikely((oflags&O_ACCMODE) != O_RDONLY) return E_PTR(-EACCES);
 return (REF struct file *)make_weak_textfile(ino,node_ent,oflags,
                                              kernel_commandline.cl_text,
                                              kernel_commandline.cl_size);
}

FUNDEF REF struct file *KCALL
kcore_fopen(struct inode *__restrict node,
            struct dentry *__restrict dent, oflag_t oflags) {
 return make_memfile(node,dent,oflags,&mman_kernel,
                    (uintptr_t)0,(uintptr_t)-1);
}


/* Misc. contents of the /proc root directory. */
INTERN struct procnode const root_content[] = {
 {0,S_IFLNK|0444,/*[[[deemon DNAM("self"); ]]]*/{"self",4,H(2580517131u,1718379891llu)}/*[[[end]]]*/,
 { .ino_readlink = &self_readlink,
 }},
 {1,S_IFREG|0444,/*[[[deemon DNAM("filesystems"); ]]]*/{"filesystems",11,H(802163638u,1733680310429884923llu)}/*[[[end]]]*/,
 { .ino_fopen = &filesystems_fopen, TEXTFILE_OPS_INIT
 }},
 {2,S_IFREG|0444,/*[[[deemon DNAM("cmdline"); ]]]*/{"cmdline",7,H(3488433892u,28550371716918627llu)}/*[[[end]]]*/,
 { .ino_fopen = &cmdline_fopen, TEXTFILE_OPS_INIT
 }},
 {3,S_IFREG|0400,/*[[[deemon DNAM("kcore"); ]]]*/{"kcore",5,H(99254056u,435711599467llu)}/*[[[end]]]*/,
 { .ino_fopen = &kcore_fopen, MEMFILE_OPS_INIT
 }},
};

STATIC_ASSERT(COMPILER_LENOF(root_content) <= PROC_ROOT_NUMNODES);


PRIVATE REF struct file *KCALL
root_fopen(struct inode *__restrict ino,
           struct dentry *__restrict node_ent,
           oflag_t oflags) {
 REF struct rootfile *result;
 result = (struct rootfile *)file_new(sizeof(struct rootfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 /* Use the caller's effective PID namespace. */
 result->rf_proc = THIS_NAMESPACE;
 CHECK_HOST_DOBJ(result->rf_proc);
 result->rf_pidcnt = ATOMIC_READ(result->rf_proc->pn_mapc);
 PID_NAMESPACE_INCREF(result->rf_proc);
 file_setup(&result->rf_file,ino,node_ent,oflags);
 return &result->rf_file;
}
#define SELF container_of(fp,struct rootfile,rf_file)
PRIVATE void KCALL
root_fclose(struct inode *__restrict UNUSED(ino),
            struct file *__restrict fp) {
 /* Drop a reference from the associated PID namespace. */
 PID_NAMESPACE_DECREF(SELF->rf_proc);
}

PRIVATE ssize_t KCALL
root_read(struct file *__restrict fp,
          USER void *buf, size_t bufsize) {
#define FILE_FLAG_EASTER (FILE_FLAG_LOCKLESS >> 1)
 PRIVATE char const *msgs[] = {
  "This is not an easter-egg (wait. - It totally is)\n",
  "You've just read the wrong directory...\n",
  "formatting all connected devices (please wait...)\n",};
 char const *m = msgs[rand() % COMPILER_LENOF(msgs)]; size_t len = strlen(m)*sizeof(char);
 if (ATOMIC_FETCHOR(fp->f_flag,bufsize >= len ? FILE_FLAG_EASTER : 0)&(FILE_FLAG_EASTER))
     return 0;
 if (copy_to_user(buf,m,MIN(bufsize,len)))
     return -EFAULT;
 return (ssize_t)len;
}
PRIVATE off_t KCALL
root_seek(struct file *__restrict fp,
          off_t off, int whence) {
 size_t new_position;
 switch (whence) {
 case SEEK_SET: new_position = (size_t)(off); break;
 case SEEK_CUR: new_position = (size_t)(SELF->rf_diridx+off); break;
 case SEEK_END: new_position = (size_t)((SELF->rf_pidcnt+COMPILER_LENOF(root_content))-off); break;
 default: return -EINVAL;
 }
 SELF->rf_diridx = new_position;
 if (new_position < SELF->rf_pidcnt) {
  SELF->rf_bucket = 0;
  SELF->rf_bucpos = 0;
  atomic_rwlock_read(&SELF->rf_proc->pn_lock);
  for (;;) {
   struct task *iter;
   if (SELF->rf_bucket >= SELF->rf_proc->pn_mapa) {
    /* Simply switch to the non-pid portion. */
    SELF->rf_diridx = SELF->rf_pidcnt;
    break;
   }
   iter = SELF->rf_proc->pn_map[SELF->rf_bucket].pb_chain;
   while (iter) {
    if (!new_position) goto gotpos;
    ++SELF->rf_bucpos,--new_position;
    iter = iter->t_pid.tp_ids[PIDTYPE_PID].tl_link.le_next;
   }
   SELF->rf_bucpos = 0;
   ++SELF->rf_bucket;
  }
gotpos:
  atomic_rwlock_endread(&SELF->rf_proc->pn_lock);
 }
 return new_position;
}
PRIVATE ssize_t KCALL
root_readdir(struct file *__restrict fp,
             USER struct dirent *buf,
             size_t bufsize, rdmode_t mode) {
 ssize_t result = 0;
 if (SELF->rf_diridx >= SELF->rf_pidcnt) {
  struct dirent header; size_t name_avail;
  struct procnode const *node; size_t index;
enum_content:
  index = SELF->rf_diridx-SELF->rf_pidcnt;
  if (index >= COMPILER_LENOF(root_content))
      return 0; /* End of directory. */
  node = &root_content[index];
  header.d_ino    = node->n_ino;
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
 } else {
  /* Enumerate running PIDs */
  struct pid_namespace *ns = SELF->rf_proc;
  struct task *iter; size_t position; pid_t taskpid;
  atomic_rwlock_read(&ns->pn_lock);
  for (;;) {
   if (SELF->rf_bucket >= ns->pn_mapa) {
    atomic_rwlock_endread(&ns->pn_lock);
    SELF->rf_diridx = SELF->rf_pidcnt;
    goto enum_content;
   }
   iter = ns->pn_map[SELF->rf_bucket].pb_chain;
   position = SELF->rf_bucpos;
   for (;;) {
    if (!iter) break;
#if 0 /* Only enumerate leaders. */
    if (iter->t_pid.tp_leader == iter)
#endif
#if 1 /* Don't enumerate zombies. */
    if (iter->t_refcnt != 0)
#endif
    {
     if (!position) goto got_task;
     --position;
    }
    iter = iter->t_pid.tp_ids[PIDTYPE_PID].tl_link.le_next;
   }
   /* Switch to the next bucket. */
   ++SELF->rf_bucket;
   SELF->rf_bucpos = 0;
  }
got_task:
  taskpid = iter->t_pid.tp_ids[PIDTYPE_PID].tl_pid;
  atomic_rwlock_endread(&ns->pn_lock);
  { struct dirent header; size_t name_avail;
    /* All right! */
    header.d_ino  = INO_FROM_PID(taskpid);
    header.d_type = DT_DIR;
    name_avail = 0;
    if (bufsize >= offsetof(struct dirent,d_name))
        name_avail = bufsize-offsetof(struct dirent,d_name);
    result = snprintf_user(buf->d_name,name_avail,PID_FMT,taskpid)*sizeof(char);
    if (bufsize >= offsetof(struct dirent,d_name)) {
     header.d_namlen = result-1;
     if (copy_to_user(buf,&header,offsetof(struct dirent,d_name)))
         return -EFAULT;
    }
    result += offsetof(struct dirent,d_name);
  }
 }
 if (FILE_READDIR_SHOULDINC(mode,bufsize,result)) {
  ++SELF->rf_diridx;
  ++SELF->rf_bucpos;
 }
 return result;
}
PRIVATE REF struct inode *KCALL
root_lookup(struct inode *__restrict dir_node,
            struct dentry *__restrict result_path) {
 struct procnode const *iter; errno_t error;
 REF struct inode *result; pid_t refpid;
 for (iter = root_content;
      iter != COMPILER_ENDOF(root_content); ++iter) {
#ifdef CONFIG_DEBUG
  struct dentryname name_copy;
  name_copy.dn_name = iter->n_name.dn_name;
  name_copy.dn_size = iter->n_name.dn_size;
  dentryname_loadhash(&name_copy);
  assertf(name_copy.dn_hash == iter->n_name.dn_hash,
          "Broken hash generator (%$q - %Ix != %Ix)",
          iter->n_name.dn_size,iter->n_name.dn_name,
          iter->n_name.dn_hash,name_copy.dn_hash);
#endif
  if (iter->n_name.dn_hash == result_path->d_name.dn_hash &&
      iter->n_name.dn_size == result_path->d_name.dn_size &&
      memcmp(iter->n_name.dn_name,result_path->d_name.dn_name,
             result_path->d_name.dn_size*sizeof(char)) == 0) {
   /* Found it! */
   result = inode_new(sizeof(struct inode));
   if unlikely(!result) return E_PTR(-ENOMEM);
   result->i_ino = iter->n_ino;
   result->i_ops = &iter->n_ops;
   result->i_attr.ia_mode = iter->n_mode;
   result->i_attr_disk.ia_mode = iter->n_mode;
   error = inode_setup(result,dir_node->i_super,THIS_INSTANCE);
   if (E_ISERR(error)) { free(result); result = E_PTR(error); }
   return result;
  }
 }
 /* Decode a decimals PID. */
 refpid = pid_from_string(result_path->d_name.dn_name,
                          result_path->d_name.dn_size);
 if (refpid >= 0) {
  WEAK REF struct task *tsk = pid_namespace_lookup_weak(THIS_NAMESPACE,refpid);
  if (tsk) return (struct inode *)pidnode_new_inherited(dir_node->i_super,tsk);
 }
 return E_PTR(-ENOENT);
}

INTERN struct inodeops const rootops = {
    .ino_fopen  = &root_fopen,
    .ino_fclose = &root_fclose,
    .ino_lookup = &root_lookup,
    .f_readdir  = &root_readdir,
    .f_read     = &root_read,
    .f_seek     = &root_seek,
};
INTERN struct superblockops const sb_rootops = {
};


INTERN SAFE REF struct superblock *KCALL
procfs_make(struct blkdev *__restrict UNUSED(dev), u32 flags,
            char const *UNUSED(devname),
            USER void *UNUSED(data), void *UNUSED(closure)) {
 REF struct superblock *result; errno_t error;
 /* Create a new superblock. */
 result = (REF struct superblock *)superblock_new(sizeof(struct superblock));
 if unlikely(!result) return E_PTR(-ENOMEM);

 /* Initialize/fill-in basic data. */
 result->sb_ops        = &sb_rootops;
 result->sb_root.i_ops = &rootops;
 result->sb_root.i_attr.ia_mode      = S_IFDIR|0555;
 result->sb_root.i_attr_disk.ia_mode = S_IFDIR|0555;

 /* Finally, setup the superblock. */
 error = superblock_setup(result,THIS_INSTANCE);
 if (E_ISERR(error)) {
  free(result);
  return E_PTR(error);
 }
 return result;
}




PRIVATE struct fstype procfs = {
    .f_owner    = THIS_INSTANCE,
    .f_sysid    = BLKSYS_EXPLICIT,
    .f_callback = &procfs_make,
    .f_flags    = FSTYPE_NODEV, /* Doesn't make use of devices. */
    .f_name     = "proc",
};

PRIVATE MODULE_INIT errno_t KCALL procfs_init(void) {
 /* Simply register our new filesystem type. */
 fs_addtype(&procfs);
 return -EOK;
}


DECL_END

#endif /* !GUARD_MODULES_FS_PROC_PROCFS_C */
