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
#ifndef GUARD_KERNEL_FS_SYS_FS_C
#define GUARD_KERNEL_FS_SYS_FS_C 1
#define _KOS_SOURCE 1

#include <dev/blkdev.h>
#include <dev/rtc.h>
#include <fcntl.h>
#include <fs/access.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/timespec.h>
#include <hybrid/types.h>
#include <kernel/mman.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <kos/syslog.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <signal.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/stat.h>

DECL_BEGIN


#ifdef __ARCH_EXTENDED_FS_SYSCALLS
SYSCALL_DEFINE3(truncate,USER char const *,path,
                syscall_ulong_t,len_hi,syscall_ulong_t,len_lo)
#define length  ((lpos_t)len_hi << 32 | (lpos_t)len_lo)
#else
SYSCALL_DEFINE2(truncate,USER char const *,path,lpos_t,length)
#endif
{
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct dentry *cwd; errno_t result;
 struct dentry *truncate_file;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = false;
 task_crit();
 result = fdman_read(fdm);
 if (E_ISERR(result)) goto end;
 walker.dw_root = fdm->fm_root;
 cwd            = fdm->fm_cwd;
 DENTRY_INCREF(walker.dw_root);
 DENTRY_INCREF(cwd);
 fdman_endread(fdm);

 truncate_file = dentry_user_xwalk(cwd,&walker,path);
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);

 if (E_ISERR(truncate_file))
  result = E_GTERR(truncate_file);
 else {
  struct inode *truncate_node;
  truncate_node = dentry_inode(truncate_file);
  DENTRY_DECREF(truncate_file);
  if unlikely(!truncate_node)
   result = -ENOENT;
  else {
   result = inode_mayaccess(truncate_node,&walker.dw_access,W_OK);
   if (E_ISOK(result)) {
    struct iattr attr;
    attr.ia_siz = (pos_t)length;
    /* Actually truncate the INode. */
    result = inode_setattr(truncate_node,&attr,IATTR_SIZ);
   }
   INODE_DECREF(truncate_node);
  }
 }
end:
 task_endcrit();
 return result;
}
#undef length

#ifdef __ARCH_EXTENDED_FS_SYSCALLS
SYSCALL_DEFINE3(ftruncate,int,fd,
                syscall_ulong_t,len_hi,syscall_ulong_t,len_lo)
#define length  ((lpos_t)len_hi << 32 | (lpos_t)len_lo)
#else
SYSCALL_DEFINE2(ftruncate,int,fd,lpos_t,length)
#endif
{
 errno_t result;
 struct inode *truncate_node;
 struct fdman *fdm = THIS_FDMAN;
 task_crit();
 truncate_node = fdman_get_inode(fdm,fd);
 if (E_ISERR(truncate_node))
  result = E_GTERR(truncate_node);
 else {
  struct fsaccess access;
  FSACCESS_SETUSER(access);
  result = inode_mayaccess(truncate_node,&access,W_OK);
  if (E_ISOK(result)) {
   struct iattr attr;
   attr.ia_siz = (pos_t)length;
   /* Actually truncate the INode. */
   result = inode_setattr(truncate_node,&attr,IATTR_SIZ);
  }
  INODE_DECREF(truncate_node);
 }
 task_endcrit();
 return result;
}
#undef length

#ifdef __ARCH_EXTENDED_FS_SYSCALLS
SYSCALL_DEFINE6(fallocate,int,fd,int,mode,
                syscall_ulong_t,off_hi,
                syscall_ulong_t,off_lo,
                syscall_ulong_t,len_hi,
                syscall_ulong_t,len_lo)
#define offset  ((lpos_t)off_hi << 32 | (lpos_t)off_lo)
#define length  ((lpos_t)len_hi << 32 | (lpos_t)len_lo)
#else
SYSCALL_DEFINE4(fallocate,int,fd,int,mode,lpos_t,offset,lpos_t,length)
#endif
{
 REF struct file *fp;
 errno_t error;
 task_crit();
 fp = fdman_get_file(THIS_FDMAN,fd);
 if (E_ISERR(fp)) error = E_GTERR(fp);
 else {
  error = file_allocate(fp,(fallocmode_t)mode,offset,length);
  FILE_DECREF(fp);
 }
 task_endcrit();
 return error;
}
#undef length
#undef offset


SYSCALL_DEFINE4(faccessat,int,dfd,USER char const *,filename,int,mode,int,flag) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct dentry *access_dentry;
 struct dentry *cwd; errno_t result;
 if (!(flag&(AT_SYMLINK_NOFOLLOW|AT_SYMLINK_FOLLOW)) && filename) return -EINVAL;

 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = !!(flag&AT_SYMLINK_NOFOLLOW);
 task_crit();

 cwd = fdman_get_dentry(fdm,dfd);
 if (E_ISERR(cwd)) { result = E_GTERR(cwd); goto end; }
 result = fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(cwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);

 access_dentry = dentry_user_xwalk(cwd,&walker,filename);
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);

 if (E_ISERR(access_dentry))
  result = E_GTERR(access_dentry);
 else {
  REF struct inode *access_inode;
  access_inode = dentry_inode(access_dentry);
  DENTRY_DECREF(access_dentry);
  if (!access_inode) result = -ENOENT;
  else {
   result = inode_mayaccess(access_inode,&walker.dw_access,mode);
   INODE_DECREF(access_inode);
  }
 }

end: task_endcrit();
 return result;
}


PRIVATE errno_t KCALL
user_chfd(int fd, USER char const *path) {
 struct dentry_walker walker;
 struct dentry *new_path;
 struct dentry *cwd; errno_t result;
 struct fdman *fdm = THIS_FDMAN;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = false;
 task_crit();

 result = fdman_read(fdm);
 if (E_ISERR(result)) goto end;
 walker.dw_root = fdm->fm_root;
 cwd            = fdm->fm_cwd;
 DENTRY_INCREF(walker.dw_root);
 DENTRY_INCREF(cwd);
 fdman_endread(fdm);

 new_path = dentry_user_xwalk(cwd,&walker,path);
 DENTRY_DECREF(cwd);
 DENTRY_DECREF(walker.dw_root);

 if (E_ISERR(new_path))
  result = E_GTERR(new_path);
 else {
  /* Make sure it's actually a directory. */
  struct inode *path_inode;
  result = -EOK;
  path_inode = dentry_inode(new_path);
  if (!path_inode) result = -ENOENT;
  else {
   if (!INODE_ISDIR(path_inode)) result = -ENOTDIR;
   INODE_DECREF(path_inode);
  }
  if (E_ISOK(result)) {
   struct dentry *old_path;
   /* Override the directory! */
   result = fdman_write(fdm);
   if (E_ISERR(result)) goto drop_new_path;
   CHECK_HOST_DOBJ(new_path);
   if (fd == AT_FDCWD) {
    old_path = fdm->fm_cwd;
    fdm->fm_cwd = new_path;
   } else {
    old_path = fdm->fm_root;
    fdm->fm_root = new_path;
   }
   fdman_endwrite(fdm);
   CHECK_HOST_DOBJ(old_path);
   DENTRY_DECREF(old_path);
  } else {
drop_new_path:
   DENTRY_DECREF(new_path);
  }
 }
end: task_endcrit();
 return result;
}

SYSCALL_DEFINE1(chdir,USER char const *,path) {
 return user_chfd(AT_FDCWD,path);
}
SYSCALL_DEFINE1(chroot,USER char const *,path) {
 return user_chfd(AT_FDROOT,path);
}

SYSCALL_DEFINE1(fchdir,int,fd) {
 struct fdman *fdm = THIS_FDMAN;
 REF struct dentry *new_path;
 errno_t result;
 task_crit();
 new_path = fdman_get_dentry(fdm,fd);
 if (E_ISERR(new_path))
  result = E_GTERR(new_path);
 else {
  /* Make sure it's actually a directory. */
  struct inode *path_inode;
  result = -EOK;
  path_inode = dentry_inode(new_path);
  if (!path_inode) result = -ENOENT;
  else {
   if (!INODE_ISDIR(path_inode)) result = -ENOTDIR;
   INODE_DECREF(path_inode);
  }
  if (E_ISOK(result)) {
   struct dentry *old_path;
   /* Override the directory! */
   result = fdman_write(fdm);
   if (E_ISERR(result)) goto drop_new_path;
   CHECK_HOST_DOBJ(new_path);
   if (fd == AT_FDCWD) {
    old_path = fdm->fm_cwd;
    fdm->fm_cwd = new_path;
   } else {
    old_path = fdm->fm_root;
    fdm->fm_root = new_path;
   }
   fdman_endwrite(fdm);
   CHECK_HOST_DOBJ(old_path);
   DENTRY_DECREF(old_path);
  } else {
drop_new_path:
   DENTRY_DECREF(new_path);
  }
 }
 task_endcrit();
 return result;
}


PRIVATE errno_t KCALL
user_fsetattr(int fd, struct iattr const *__restrict new_attr,
              iattrset_t attr_valid) {
 REF struct inode *chmod_inode;
 errno_t result;
 task_crit();
 chmod_inode = fdman_get_inode(THIS_FDMAN,fd);
 if (E_ISERR(chmod_inode))
  result = E_GTERR(chmod_inode);
 else {
  struct fsaccess ac;
  FSACCESS_SETUSER(ac);
  result = inode_mayaccess(chmod_inode,&ac,W_OK);
  if (E_ISOK(result))
      result = inode_setattr(chmod_inode,new_attr,attr_valid);
  INODE_DECREF(chmod_inode);
 }
 task_endcrit();
 return result;
}

PRIVATE errno_t KCALL
user_fsetattrat(int dfd, USER char const *filename,
                struct iattr const *__restrict new_attr,
                iattrset_t attr_valid, bool no_follow) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct dentry *update_dentry;
 struct dentry *cwd; errno_t result;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = no_follow;
 task_crit();

 cwd = fdman_get_dentry(fdm,dfd);
 if (E_ISERR(cwd)) { result = E_GTERR(cwd); goto end; }
 result = fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(cwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);

 update_dentry = dentry_user_xwalk(cwd,&walker,filename);
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);

 if (E_ISERR(update_dentry))
  result = E_GTERR(update_dentry);
 else {
  REF struct inode *update_inode;
  update_inode = dentry_inode(update_dentry);
  DENTRY_DECREF(update_dentry);
  if (!update_inode) result = -ENOENT;
  else {
   result = inode_mayaccess(update_inode,&walker.dw_access,W_OK);
   if (E_ISOK(result))
       result = inode_setattr(update_inode,new_attr,attr_valid);
   INODE_DECREF(update_inode);
  }
 }

end: task_endcrit();
 return result;
}


SYSCALL_DEFINE2(fchmod,int,fd,mode_t,mode) {
 struct iattr new_attr;
 new_attr.ia_mode = mode;
 return user_fsetattr(fd,&new_attr,IATTR_MODE);
}
SYSCALL_DEFINE3(fchown,int,fd,uid_t,user,gid_t,group) {
 struct iattr new_attr;
 new_attr.ia_uid = user;
 new_attr.ia_gid = group;
 return user_fsetattr(fd,&new_attr,IATTR_UID|IATTR_GID);
}
SYSCALL_DEFINE3(fchmodat,int,dfd,USER char const *,filename,mode_t,mode) {
 struct iattr new_attr;
 new_attr.ia_mode = mode;
 return user_fsetattrat(dfd,filename,&new_attr,IATTR_MODE,mode&O_NOFOLLOW);
}
SYSCALL_DEFINE5(fchownat,int,dfd,USER char const *,filename,uid_t,user,gid_t,group,int,flag) {
 struct iattr new_attr;
 new_attr.ia_uid = user;
 new_attr.ia_gid = group;
 if (!(flag&(AT_SYMLINK_NOFOLLOW|AT_SYMLINK_FOLLOW)) &&
       filename) return -EINVAL;
 return user_fsetattrat(dfd,filename,&new_attr,
                        IATTR_UID|IATTR_GID,
                        flag&AT_SYMLINK_NOFOLLOW);
}

SYSCALL_DEFINE4(readlinkat,int,dfd,USER char const *,path,USER char *,buf,size_t,len) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct dentry *cwd; ssize_t result;
 struct dentry *link_file;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = true;
 task_crit();
 cwd = fdman_get_dentry(fdm,dfd);
 if (E_ISERR(cwd)) { result = E_GTERR(cwd); goto end; }
 result = (ssize_t)fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(cwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);
 link_file = dentry_user_xwalk(cwd,&walker,path);
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);
 if (E_ISERR(link_file))
  result = E_GTERR(link_file);
 else {
  /* Read the link of this directory entry. */
  result = dentry_readlink(link_file,&walker.dw_access,buf,len);
  DENTRY_DECREF(link_file);
 }
end:
 task_endcrit();
 return result;
}

#define NO_STAT 0

#if NO_STAT
SYSCALL_DEFINE2(fstat64,int,fd,USER struct stat64 *,statbuf) {
 return -EOK;
}
SYSCALL_DEFINE4(fstatat64,int,dfd,USER char const *,filename,
                USER struct stat64 *,statbuf,int,flag) {
 return -EOK;
}
#else
PRIVATE SAFE errno_t KCALL
inode_stat(struct inode *__restrict ino,
           USER struct stat64 *buf) {
 struct stat64 data;
 struct blkdev *block;
 errno_t error;
 CHECK_HOST_DOBJ(ino);
 CHECK_HOST_DOBJ(ino->i_super);

 /*memset(&data,0,sizeof(struct stat64));*/

 /* Read attribute-specific data. */
 error = rwlock_read(&ino->i_attr_lock);
 if (E_ISERR(error)) return error;
 data.st_mode   = ino->i_attr.ia_mode;
 data.st_uid    = ino->i_attr.ia_mode;
 data.st_gid    = ino->i_attr.ia_mode;
#ifdef CONFIG_32BIT_TIME
 data.st_atim32 = ino->i_attr.ia_atime;
 data.st_mtim32 = ino->i_attr.ia_mtime;
 data.st_ctim32 = ino->i_attr.ia_ctime;
#else
 data.st_atim64 = ino->i_attr.ia_atime;
 data.st_mtim64 = ino->i_attr.ia_mtime;
 data.st_ctim64 = ino->i_attr.ia_ctime;
#endif
 data.st_size64 = ino->i_attr.ia_siz;
 rwlock_endread(&ino->i_attr_lock);

 /* XXX: KOS doesn't implement holes (yet?) */
 data.st_blocks64 = CEILDIV(data.st_size64,512);
 data.st_ino64    = ino->i_ino;
 data.st_nlink    = ATOMIC_READ(ino->i_nlink);

#ifdef CONFIG_32BIT_TIME
 data.st_atim64.tv_sec  = (time64_t)data.st_atim32.tv_sec;
 data.st_atim64.tv_nsec = data.st_atim32.tv_sec;
 data.st_mtim64.tv_sec  = (time64_t)data.st_mtim32.tv_sec;
 data.st_mtim64.tv_nsec = data.st_mtim32.tv_sec;
 data.st_ctim64.tv_sec  = (time64_t)data.st_ctim32.tv_sec;
 data.st_ctim64.tv_nsec = data.st_ctim32.tv_sec;
#else
 data.st_atim32.tv_sec  = (time32_t)data.st_atim64.tv_sec;
 data.st_atim32.tv_nsec = data.st_atim64.tv_sec;
 data.st_mtim32.tv_sec  = (time32_t)data.st_mtim64.tv_sec;
 data.st_mtim32.tv_nsec = data.st_mtim64.tv_sec;
 data.st_ctim32.tv_sec  = (time32_t)data.st_ctim64.tv_sec;
 data.st_ctim32.tv_nsec = data.st_ctim64.tv_sec;
#endif

 if ((block = ino->i_super->sb_blkdev) != NULL) {
  data.st_dev     = block->bd_device.d_id;
  data.st_blksize = block->bd_blocksize;
 } else {
  data.st_blksize = 512; /* Default */
 }
 if (INODE_ISDEV(ino))
     data.st_rdev = container_of(ino,struct device,d_node)->d_id;
 if (copy_to_user(buf,&data,sizeof(struct stat64)))
     return -EFAULT;
 return -EOK;
}

SYSCALL_DEFINE2(fstat64,int,fd,USER struct stat64 *,statbuf) {
 REF struct inode *ino; errno_t result;
 task_crit();
 ino = fdman_get_inode(THIS_FDMAN,fd);
 if (E_ISERR(ino))
  result = E_GTERR(ino);
 else {
  struct fsaccess ac;
  FSACCESS_SETUSER(ac);
  result = inode_mayaccess(ino,&ac,R_OK);
  if (E_ISOK(result)) result = inode_stat(ino,statbuf);
  INODE_DECREF(ino);
 }
 task_endcrit();
 return result;
}
SYSCALL_DEFINE4(fstatat64,int,dfd,USER char const *,filename,
                USER struct stat64 *,statbuf,int,flag) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct dentry *cwd; errno_t result;
 struct dentry *stat_file;
 if (!(flag&(AT_SYMLINK_NOFOLLOW|AT_SYMLINK_FOLLOW)) &&
       filename) return -EINVAL;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = !!(flag&AT_SYMLINK_NOFOLLOW);
 task_crit();
 cwd = fdman_get_dentry(fdm,dfd);
 if (E_ISERR(cwd)) { result = E_GTERR(cwd); goto end; }
 result = fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(cwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);
 stat_file = dentry_user_xwalk(cwd,&walker,filename);
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);
 if (E_ISERR(stat_file))
  result = E_GTERR(stat_file);
 else {
  /* Read the link of this directory entry. */
  REF struct inode *ino;
  ino = dentry_inode(stat_file);
  DENTRY_DECREF(stat_file);
  if unlikely(!ino)
   result = -ENOENT;
  else {
   result = inode_mayaccess(ino,&walker.dw_access,R_OK);
   if (E_ISOK(result)) result = inode_stat(ino,statbuf);
   INODE_DECREF(ino);
  }
 }
end:
 task_endcrit();
 return result;
}
#endif




SYSCALL_DEFINE4(mknodat,int,dfd,USER char const *,filename,mode_t,mode,dev_t,dev) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 REF struct dentry *cwd; errno_t result;
 REF struct dentry *result_entry;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = !!(mode&O_NOFOLLOW);
 task_crit();
 cwd = fdman_get_dentry(fdm,dfd);
 if (E_ISERR(cwd)) { result = E_GTERR(cwd); goto end; }
 result = fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(cwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);
 /* Setup file attributes. */
 assert(result == -EOK);
 if (S_ISCHR(mode) || S_ISBLK(mode)) {
  REF struct device *node_device;
  node_device = S_ISCHR(mode) ? (REF struct device *)CHRDEV_LOOKUP(dev)
                              : (REF struct device *)BLKDEV_LOOKUP(dev);
  if unlikely(!node_device)
     result = -ENODEV;
  else {
   result_entry = fs_user_xinsnod(&walker,cwd,filename,node_device,NULL);
   DEVICE_DECREF(node_device);
check_result_entry:
   if (E_ISERR(result_entry))
    result = E_GTERR(result_entry);
   else {
    DENTRY_DECREF(result_entry);
   }
  }
 } else if (S_ISREG(mode) || (mode&__S_IFMT) == 0) {
  struct iattr file_attr;
  sysrtc_get(&file_attr.ia_ctime);
  file_attr.ia_mode  = (mode&0777);
  file_attr.ia_uid   = walker.dw_access.fa_uid;
  file_attr.ia_gid   = walker.dw_access.fa_gid;
  file_attr.ia_siz   = 0; /* Create an empty file. */
  file_attr.ia_atime = file_attr.ia_ctime;
  file_attr.ia_mtime = file_attr.ia_ctime;
  result_entry = fs_user_xmkreg(&walker,cwd,filename,&file_attr,NULL);
  goto check_result_entry;
 } else if (S_ISFIFO(mode)) {
  result = -ENOSYS; /* TODO: Create a named pipe. */
 } else if (S_ISSOCK(mode)) {
  result = -ENOSYS; /* TODO: What even is this? */
 } else {
  result = -EINVAL;
 }
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);
end: task_endcrit();
 return result;
}


SYSCALL_DEFINE3(mkdirat,int,dfd,USER char const *,pathname,mode_t,mode) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct iattr file_attr;
 REF struct dentry *cwd; errno_t result;
 REF struct dentry *result_entry;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = !!(mode&O_NOFOLLOW);
 task_crit();
 cwd = fdman_get_dentry(fdm,dfd);
 if (E_ISERR(cwd)) { result = E_GTERR(cwd); goto end; }
 result = fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(cwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);
 /* Setup file attributes. */
 assert(result == -EOK);
 sysrtc_get(&file_attr.ia_ctime);
 file_attr.ia_mode  = (mode&0777);
 file_attr.ia_uid   = walker.dw_access.fa_uid;
 file_attr.ia_gid   = walker.dw_access.fa_gid;
 file_attr.ia_atime = file_attr.ia_ctime;
 file_attr.ia_mtime = file_attr.ia_ctime;
 result_entry = fs_user_xmkdir(&walker,cwd,pathname,&file_attr,NULL);
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);
 if (E_ISERR(result_entry))
  result = E_GTERR(result_entry);
 else {
  DENTRY_DECREF(result_entry);
 }
end: task_endcrit();
 return result;
}

SYSCALL_DEFINE3(unlinkat,int,dfd,USER char const *,pathname,int,flag) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct dentry *cwd; errno_t result;
 struct dentry *unlink_file;
 if (!(flag&(AT_SYMLINK_NOFOLLOW|AT_SYMLINK_FOLLOW)) &&
       pathname) return -EINVAL;
 if (!(flag&(AT_REMOVEDIR|AT_REMOVEREG|AT_REMOVEMNT)))
       flag |= AT_REMOVEREG; /* Default: Unlink regular files. */
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = !!(flag&AT_SYMLINK_NOFOLLOW);
 task_crit();
 cwd = fdman_get_dentry(fdm,dfd);
 if (E_ISERR(cwd)) { result = E_GTERR(cwd); goto end; }
 result = fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(cwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);
 unlink_file = dentry_user_xwalk(cwd,&walker,pathname);
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);
 if (E_ISERR(unlink_file))
  result = E_GTERR(unlink_file);
 else {
  /* Unlink this directory entry. */
  result = dentry_remove(unlink_file,&walker.dw_access,
                         flag&(AT_REMOVEDIR|AT_REMOVEREG|AT_REMOVEMNT));
  DENTRY_DECREF(unlink_file);
 }
end:
 task_endcrit();
 return result;
}
SYSCALL_DEFINE3(symlinkat,USER char const *,oldname,int,newdfd,USER char const *,newname) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct iattr file_attr;
 REF struct dentry *cwd; errno_t result;
 REF struct dentry *result_entry;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = false;
 task_crit();
 cwd = fdman_get_dentry(fdm,newdfd);
 if (E_ISERR(cwd)) { result = E_GTERR(cwd); goto end; }
 result = fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(cwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);
 /* Setup file attributes. */
 assert(result == -EOK);
 sysrtc_get(&file_attr.ia_ctime);
 file_attr.ia_mode  = 0777;
 file_attr.ia_uid   = walker.dw_access.fa_uid;
 file_attr.ia_gid   = walker.dw_access.fa_gid;
 file_attr.ia_atime = file_attr.ia_ctime;
 file_attr.ia_mtime = file_attr.ia_ctime;
 result_entry = fs_user_xsymlink(&walker,cwd,newname,oldname,&file_attr,NULL);
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);
 if (E_ISERR(result_entry))
  result = E_GTERR(result_entry);
 else {
  DENTRY_DECREF(result_entry);
 }
end: task_endcrit();
 return result;
}
SYSCALL_DEFINE5(linkat,int,olddfd,USER char const *,oldname,
                       int,newdfd,USER char const *,newname,int,flags) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 REF struct dentry *oldcwd,*newcwd;
 REF struct dentry *link_file,*result_entry;
 REF struct inode *link_node;
 errno_t result;
 if (!(flags&(AT_SYMLINK_NOFOLLOW|AT_SYMLINK_FOLLOW))) return -EINVAL;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = !!(flags&AT_SYMLINK_NOFOLLOW);
 task_crit();
 oldcwd = fdman_get_dentry(fdm,olddfd);
 if (E_ISERR(oldcwd)) { result = E_GTERR(oldcwd); goto end; }
 result = fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(oldcwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);
 /* Lookup the INode we're supposed to link against. */
 link_file = dentry_user_xwalk(oldcwd,&walker,oldname);
 DENTRY_DECREF(oldcwd);
 assert(result == -EOK);
 if (E_ISERR(link_file))
  result = E_GTERR(link_file);
 else {
  link_node = dentry_inode(link_file);
  DENTRY_DECREF(link_file);
  if unlikely(!link_node)
   result = -ENOENT;
  else {
   /* Link this directory entry. */
   walker.dw_nlink = 0;
   newcwd = fdman_get_dentry(fdm,newdfd);
   if (E_ISERR(newcwd))
    result = E_GTERR(newcwd);
   else {
    result_entry = fs_user_xhrdlink(&walker,newcwd,newname,link_node);
    DENTRY_DECREF(newcwd);
    if (E_ISERR(result_entry))
     result = E_GTERR(result_entry);
    else DENTRY_DECREF(result_entry);
   }
   INODE_DECREF(link_node);
  }
 }
 DENTRY_DECREF(walker.dw_root);
end:
 task_endcrit();
 return result;
}
SYSCALL_DEFINE4(renameat,int,olddfd,USER char const *,oldname,
                         int,newdfd,USER char const *,newname) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 REF struct dentry *oldcwd,*newcwd;
 REF struct dentry *rename_file,*result_entry;
 errno_t result;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = true;
 task_crit();
 oldcwd = fdman_get_dentry(fdm,olddfd);
 if (E_ISERR(oldcwd)) { result = E_GTERR(oldcwd); goto end; }
 result = fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(oldcwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);
 /* Lookup the INode we're supposed to link against. */
 rename_file = dentry_user_xwalk(oldcwd,&walker,oldname);
 DENTRY_DECREF(oldcwd);
 assert(result == -EOK);
 if (E_ISERR(rename_file))
  result = E_GTERR(rename_file);
 else {
  /* Rename this directory entry. */
  walker.dw_nlink = 0;
  newcwd = fdman_get_dentry(fdm,newdfd);
  if (E_ISERR(newcwd))
   result = E_GTERR(newcwd);
  else {
   result_entry = fs_user_xrename(&walker,newcwd,newname,rename_file,NULL);
   DENTRY_DECREF(newcwd);
   if (E_ISERR(result_entry))
    result = E_GTERR(result_entry);
   else DENTRY_DECREF(result_entry);
  }
  DENTRY_DECREF(rename_file);
 }
 DENTRY_DECREF(walker.dw_root);
end:
 task_endcrit();
 return result;
}

SYSCALL_DEFINE4(utimensat,int,dfd,USER char const *,filename,
                USER struct timespec const *,utimes,int,flags) {
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct dentry *cwd; errno_t result;
 struct dentry *stat_file;
 if (!(flags&(AT_SYMLINK_NOFOLLOW|AT_SYMLINK_FOLLOW)) &&
       filename) return -EINVAL;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = !!(flags&AT_SYMLINK_NOFOLLOW);
 task_crit();
 cwd = fdman_get_dentry(fdm,dfd);
 if (E_ISERR(cwd)) { result = E_GTERR(cwd); goto end; }
 result = fdman_read(fdm);
 if (E_ISERR(result)) { DENTRY_DECREF(cwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);
 stat_file = dentry_user_xwalk(cwd,&walker,filename);
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);
 if (E_ISERR(stat_file))
  result = E_GTERR(stat_file);
 else {
  /* Read the link of this directory entry. */
  REF struct inode *ino;
  ino = dentry_inode(stat_file);
  DENTRY_DECREF(stat_file);
  if unlikely(!ino)
   result = -ENOENT;
  else {
   result = inode_mayaccess(ino,&walker.dw_access,W_OK);
   if (E_ISOK(result)) {
    struct iattr new_attrib;
    iattrset_t   new_valid = IATTR_ATIME|IATTR_MTIME;
    /* Update file times. */
    if (utimes) {
     if (copy_from_user(&new_attrib.ia_atime,utimes,
                        2*sizeof(struct timespec)))
     { result = -EFAULT; goto done; }
     if (new_attrib.ia_atime.tv_nsec == UTIME_OMIT) new_valid &= ~(IATTR_ATIME);
     if (new_attrib.ia_mtime.tv_nsec == UTIME_OMIT) new_valid &= ~(IATTR_MTIME);
     if (new_attrib.ia_atime.tv_nsec == UTIME_NOW ||
         new_attrib.ia_mtime.tv_nsec == UTIME_NOW) {
      if (new_attrib.ia_atime.tv_nsec == UTIME_NOW) {
       sysrtc_get(&new_attrib.ia_atime);
       if (new_attrib.ia_mtime.tv_nsec == UTIME_NOW)
           new_attrib.ia_mtime = new_attrib.ia_atime;
      } else {
       sysrtc_get(&new_attrib.ia_mtime);
      }
     }
    } else {
     /* Use NOW for both access & modification. */
     sysrtc_get(&new_attrib.ia_atime);
     new_attrib.ia_mtime = new_attrib.ia_atime;
    }
    result = inode_setattr(ino,&new_attrib,new_valid);
   }
done:
   INODE_DECREF(ino);
  }
 }
end:
 task_endcrit();
 return result;
}


SYSCALL_DEFINE4(openat,int,dfd,USER char const *,filename,oflag_t,flags,mode_t,mode) {
 struct fd fp;
 struct fdman *fdm = THIS_FDMAN;
 struct dentry_walker walker;
 struct dentry *cwd; union { errno_t e; int f; } result;
 struct iattr file_attr;
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = !!(mode&O_NOFOLLOW);
 task_crit();
 cwd = fdman_get_dentry(fdm,dfd);
 if (E_ISERR(cwd)) { result.e = E_GTERR(cwd); goto end; }
 result.e = fdman_read(fdm);
 if (E_ISERR(result.e)) { DENTRY_DECREF(cwd); goto end; }
 walker.dw_root = fdm->fm_root;
 DENTRY_INCREF(walker.dw_root);
 fdman_endread(fdm);

 /* Setup file attributes. */
 file_attr.ia_mode = (mode&0777);
 file_attr.ia_uid  = walker.dw_access.fa_uid;
 file_attr.ia_gid  = walker.dw_access.fa_gid;

 /* Actually open the file. */
 fp.fo_obj.fo_file = fs_user_xopen(&walker,cwd,filename,&file_attr,
                                   IATTR_MODE|IATTR_UID|IATTR_GID,flags);

 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);

 /* Check for errors during open. */
 if (E_ISERR(fp.fo_obj.fo_file)) {
  result.e = E_GTERR(fp.fo_obj.fo_file);
  goto end;
 }

 /* Setup the descriptor type & flags. */
 fp.fo_ops = &fd_ops[FD_TYPE_FILE];
 if (flags&O_CLOEXEC) fp.fo_flags |= FD_CLOEXEC;
 if (flags&O_CLOFORK) fp.fo_flags |= FD_CLOFORK;

 /* At this point we've got the newly opened file. - Time to install it! */
 result.f = fdman_put(fdm,fp);
 FILE_DECREF(fp.fo_obj.fo_file);

end: task_endcrit();
 return result.f;
}

SYSCALL_DEFINE6(pselect6,size_t,n,USER fd_set *,inp,USER fd_set *,outp,
                USER fd_set *,exp,USER struct timespec const *,tsp,
                USER void *,sig) {
 struct timespec abstime; ssize_t result;
 sigset_t old_set,new_set; size_t fd_base;
 struct task *t = THIS_TASK;
 struct fdman *fdm = t->t_fdman;
 struct {
  USER sigset_t *pset;
  size_t         setsz;
 } signal_blocking;
 if (tsp) {
  struct timespec now;
  if (copy_from_user(&abstime,tsp,sizeof(struct timespec)))
      return -EFAULT;
  sysrtc_get(&now);
  TIMESPEC_ADD(abstime,now);
 }
 if (sig) {
  if (copy_from_user(&signal_blocking,sig,sizeof(signal_blocking)))
      return -EFAULT;
  if (signal_blocking.setsz > sizeof(sigset_t))
      return -EINVAL;
  if (copy_from_user(&new_set,signal_blocking.pset,signal_blocking.setsz))
      return -EFAULT;
  /* Inherit anything that isn't given. */
  memcpy(((byte_t *)&new_set)+signal_blocking.setsz,
         ((byte_t *)&t->t_sigblock)+signal_blocking.setsz,
           sizeof(sigset_t)-signal_blocking.setsz);
  sigdelset(&new_set,SIGKILL);
  sigdelset(&new_set,SIGSTOP);
  memcpy(&old_set,&t->t_sigblock,sizeof(sigset_t));
  /* Set the given signal mask as active. */
  result = task_set_sigblock(&new_set);
  if (E_ISERR(result)) return result;
 }
 task_crit();
scan_again:
 result = 0;

 for (fd_base = 0; fd_base < n; fd_base += 32){
  u32 part_i,part_o,part_e,mask; size_t fd_no;
  size_t part_bits = MIN(n-fd_base,32);
  size_t part_byte = CEILDIV(part_bits,8);
  /* Figure out which descriptors we're supposed to wait for. */
  part_i = part_o = part_e = 0;
  if ((inp  && copy_from_user(&part_i,(byte_t *)inp +(fd_base/8),part_byte)) ||
      (outp && copy_from_user(&part_o,(byte_t *)outp+(fd_base/8),part_byte)) ||
      (exp  && copy_from_user(&part_e,(byte_t *)exp +(fd_base/8),part_byte)))
       goto efault;
  /* Quick check: If we're not supposed to wait for anything, skip ahead. */
  if (!part_i && !part_o && !part_e) continue;
  for (fd_no = fd_base,mask = 1; mask; mask <<= 1,++fd_no) {
   pollmode_t mode = 0; REF struct file *fp;
   if (part_i&mask) mode |= POLLIN|POLLPRI;
   if (part_o&mask) mode |= POLLOUT;
   if (part_e&mask) mode |= POLLERR;
   assert(mode);
   fp = fdman_get_file(fdm,fd_no);
   if (E_ISERR(fp)) { result = E_GTERR(fp); goto done; }
   mode = file_poll(fp,mode);
   FILE_DECREF(fp);
   if (E_ISERR(mode)) {
    /* Ignore errors related to missing/unimplemented polling channels. */
    if (mode == -EWOULDBLOCK) continue;
    result = mode;
    goto done;
   }
   if (mode) ++result;
  }
 }

 /* Wait for one of the file descriptors to change state. */
 if (!result) {
  result = E_GTERR(task_waitfor(tsp ? &abstime : NULL));
  /* If we've received one of the signals in question, scan again. */
  if (E_ISOK(result)) goto scan_again;
  /* NOTE: pselect() must return ZERO(0) on timeout! */
  if (result == -ETIMEDOUT) result = 0;
 }
done:
 task_clrwait();
 task_endcrit();
 if (sig) {
  /* Restore the old signal mask. */
  errno_t temp = task_set_sigblock(&old_set);
  if (E_ISERR(temp)) result = (ssize_t)temp;
 }
 return result;
efault: result = -EFAULT; goto done;
}

SYSCALL_DEFINE5(ppoll,USER struct pollfd *,ufds,size_t,nfds,
                USER struct timespec const *,tsp,
                USER sigset_t const *,sigmask,size_t,sigsetsize) {
 struct pollfd buf[16];
 struct pollfd *iter,*end;
 USER struct pollfd *src;
 size_t partsize,remaining;
 ssize_t result,old_result;
 struct task *t = THIS_TASK;
 struct fdman *fdm = t->t_fdman;
 struct timespec abstime;
 struct sig *error = E_PTR(-EOK);
 sigset_t old_set,new_set;
 if (tsp) {
  struct timespec now;
  if (copy_from_user(&abstime,tsp,sizeof(struct timespec)))
      return -EFAULT;
  sysrtc_get(&now);
  TIMESPEC_ADD(abstime,now);
 }
 if (sigmask && sigsetsize) {
  if (sigsetsize > sizeof(sigset_t))
      return -EINVAL;
  if (copy_from_user(&new_set,sigmask,sigsetsize))
      return -EFAULT;
  /* Inherit anything that isn't given. */
  memcpy(((byte_t *)&new_set)+sigsetsize,
         ((byte_t *)&t->t_sigblock)+sigsetsize,
           sizeof(sigset_t)-sigsetsize);
  sigdelset(&new_set,SIGKILL);
  sigdelset(&new_set,SIGSTOP);
  memcpy(&old_set,&t->t_sigblock,sizeof(sigset_t));
  /* Set the given signal mask as active. */
  result = task_set_sigblock(&new_set);
  if (E_ISERR(result)) return result;
 }
 task_crit();
scan_again:
 src       = ufds;
 remaining = nfds;
 result    = 0;
 while (remaining) {
  partsize = MIN(COMPILER_LENOF(buf),remaining);
  if (copy_from_user(buf,src,partsize*sizeof(struct pollfd)))
      goto efault;
  end = (iter = buf)+partsize;
  old_result = result;
  for (; iter != end; ++iter) {
   pollmode_t temp;
   REF struct file *fp;
   if (iter->fd < 0) continue; /* Ignore negative descriptors. */
   fp = fdman_get_file(fdm,iter->fd);
   if (E_ISERR(fp)) temp = POLLERR;
   else {
    temp = file_poll(fp,iter->events);
    FILE_DECREF(fp);
    if (E_ISERR(temp))
        temp = temp == -EINVAL || temp == -EWOULDBLOCK ? POLLNVAL : POLLERR;
   }
   iter->revents = temp;
   if (temp) ++result;
  }
  if (result != old_result) {
   /* If something changed, copy the changes to user-space. */
   if (copy_to_user(src,buf,partsize*sizeof(struct pollfd)))
       goto efault;
  }
  /* Continue iterating all give pollfd's */
  remaining -= partsize;
  src       += partsize;
 }
 if (!result) {
  error = task_waitfor(tsp ? &abstime : NULL);
  /* If the wait was successful, some FD was signaled and we must scan again. */
  if (E_ISOK(error))
      goto scan_again;
  result = E_GTERR(error);
 }
done:
 task_clrwait();
 task_endcrit();
 if (sigmask && sigsetsize) {
  /* Restore the old signal mask. */
  errno_t temp = task_set_sigblock(&old_set);
  if (E_ISERR(temp)) result = (ssize_t)temp;
 }
 return result;
efault: result = -EFAULT; goto done;
}



DECL_END

#endif /* !GUARD_KERNEL_FS_SYS_FS_C */
