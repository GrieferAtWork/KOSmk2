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
#ifndef GUARD_KERNEL_CORE_FILE_C
#define GUARD_KERNEL_CORE_FILE_C 1

#include <fcntl.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <hybrid/atomic.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <hybrid/types.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <poll.h>
#include <sched/task.h>
#include <sync/rwlock.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fs/superblock.h>
#include <kernel/boot.h>

DECL_BEGIN

PUBLIC REF struct file *KCALL
inode_fopen_sized(struct inode *__restrict ino,
                  struct dentry *__restrict node_ent,
                  oflag_t oflags, size_t file_size) {
 REF struct file *result;
 result = file_new(file_size);
 if unlikely(!result) return E_PTR(-ENOMEM);
 file_setup(result,ino,node_ent,oflags);
 return result;
}
PUBLIC REF struct file *KCALL
inode_fopen_default(struct inode *__restrict ino,
                    struct dentry *__restrict node_ent,
                    oflag_t oflags) {
 return inode_fopen_sized(ino,node_ent,oflags,sizeof(struct file));
}


PUBLIC void KCALL
file_setup(struct file *__restrict self,
           struct inode  *__restrict node,
           struct dentry *__restrict dent,
           oflag_t oflags) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(node);
 CHECK_HOST_DOBJ(dent);
 self->f_refcnt = 1;
 self->f_ops    = node->i_ops;
 self->f_mode   = oflags;
#if INODE_FILE_LOCKLESS == FILE_FLAG_LOCKLESS
 self->f_flag |= self->f_ops->f_flags&INODE_FILE_LOCKLESS;
#else
 if (self->f_ops->f_flags&INODE_FILE_LOCKLESS)
     self->f_flag |= FILE_FLAG_LOCKLESS;
#endif
 INODE_INCREF (node),self->f_node = node;
 DENTRY_INCREF(dent),self->f_dent = dent;
 rwlock_cinit(&self->f_lock);
 atomic_rwlock_cinit(&self->f_flock.fl_lock);
 /* Add the file to the list of open streams associated with the given node. */
 atomic_rwlock_write(&node->i_file.i_files_lock);
 LIST_INSERT(node->i_file.i_files,self,f_open);
 atomic_rwlock_endwrite(&node->i_file.i_files_lock);
}

INTDEF void KCALL
superblock_autosync(struct superblock *__restrict self);

PUBLIC void KCALL
file_destroy(struct file *__restrict self) {
 struct inode *ino;
 CHECK_HOST_DOBJ(self);
 assert(self->f_node);
 assert(self->f_open.le_pself);
 ino = self->f_node;
 /* Flush a file that we written to. */
 if (self->f_flag&FILE_FLAG_DIDWRITE &&
     self->f_ops->f_sync)
   (*self->f_ops->f_sync)(self);

 /* Invoke the file-close callback. */
 if (ino->i_ops->ino_fclose)
   (*ino->i_ops->ino_fclose)(ino,self);

 /* XXX: Maybe not flush everything? */
 if (fs_autosync && self->f_flag&FILE_FLAG_DIDWRITE)
     superblock_autosync(self->f_node->i_super);

 /* Remove the file from the list of open files. */
 atomic_rwlock_write(&ino->i_file.i_files_lock);
 LIST_REMOVE(self,f_open);
 atomic_rwlock_endwrite(&ino->i_file.i_files_lock);
 DENTRY_DECREF(self->f_dent);
 if (ino->i_ops->ino_ffree)
   (*ino->i_ops->ino_ffree)(ino,self);
 else free(self);
 INODE_DECREF(ino);
}

PUBLIC pollmode_t KCALL
file_poll(struct file *__restrict self, pollmode_t mode) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->f_ops);
 /* Check if the given mode is supported by the file. */
 if (mode&(POLLIN|POLLPRI)) {
  if ((self->f_mode&O_ACCMODE) == O_WRONLY)
       return -EPERM;
 } else if (mode&POLLOUT) {
  if ((self->f_mode&O_ACCMODE) == O_RDONLY)
       return -EPERM;
 } else {
  return -EWOULDBLOCK;
 }
 if (!self->f_ops->f_poll)
     return -EWOULDBLOCK;
 /* Execute the poll operator. */
 return (*self->f_ops->f_poll)(self,mode);
}


PUBLIC ssize_t KCALL
file_read(struct file *__restrict self,
          USER void *buf, size_t bufsize) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 CHECK_USER_DATA(buf,bufsize);
 if ((self->f_mode&O_ACCMODE) == O_WRONLY ||
     !self->f_ops->f_read) return -EPERM;
 if (FILE_ISLOCKLESS(self)) {
  result = (*self->f_ops->f_read)(self,buf,bufsize);
 } else {
  result = rwlock_write(&self->f_lock);
  if (E_ISERR(result)) return result;
  result = (*self->f_ops->f_read)(self,buf,bufsize);
  rwlock_endwrite(&self->f_lock);
 }
 return result;
}
PUBLIC ssize_t KCALL
file_write(struct file *__restrict self,
           USER void const *buf, size_t bufsize) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 CHECK_USER_TEXT(buf,bufsize);
 assert(self);
 /*assert(self->f_node->i_ops == self->f_ops);*/
 if ((self->f_mode&O_ACCMODE) == O_RDONLY ||
     !self->f_ops->f_write) return -EPERM;
 if (FILE_ISLOCKLESS(self)) {
  result = (*self->f_ops->f_write)(self,buf,bufsize);
 } else {
  result = rwlock_write(&self->f_lock);
  if (E_ISERR(result)) return result;
  inode_invalidate(self->f_node);
  if ((self->f_mode&O_APPEND) &&
     !(self->f_flag&FILE_FLAG_BACK)) {
   if (self->f_ops->f_seek) {
    result = (*self->f_ops->f_seek)(self,0,SEEK_END);
    if (E_ISERR(result)) goto end;
   }
   self->f_flag |= FILE_FLAG_BACK;
  }
  result = (*self->f_ops->f_write)(self,buf,bufsize);
end:
  rwlock_endwrite(&self->f_lock);
 }
 if (E_ISOK(result))
     self->f_flag |= FILE_FLAG_DIDWRITE;
 return result;
}
PUBLIC ssize_t KCALL
file_pread(struct file *__restrict self,
           USER void *buf, size_t bufsize, pos_t pos) {
 CHECK_HOST_DOBJ(self);
 CHECK_USER_DATA(buf,bufsize);
 if ((self->f_mode&O_ACCMODE) == O_WRONLY ||
     !self->f_ops->f_pread) return -EPERM;
 return (*self->f_ops->f_pread)(self,buf,bufsize,pos);
}
PUBLIC ssize_t KCALL
file_pwrite(struct file *__restrict self,
            USER void const *buf, size_t bufsize, pos_t pos) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 CHECK_USER_TEXT(buf,bufsize);
 assert(self->f_node);
 if ((self->f_mode&O_ACCMODE) == O_RDONLY ||
     !self->f_ops->f_pwrite) return -EPERM;
 inode_invalidate(self->f_node);
 result = (*self->f_ops->f_pwrite)(self,buf,bufsize,pos);
 if (E_ISOK(result))
     self->f_flag |= FILE_FLAG_DIDWRITE;
 return result;
}

PUBLIC errno_t KCALL
file_allocate(struct file *__restrict self, fallocmode_t mode,
              pos_t start, pos_t size) {
 errno_t result;
 CHECK_HOST_DOBJ(self);
 if ((self->f_mode&O_ACCMODE) == O_RDONLY ||
     !self->f_ops->f_allocate) return -EPERM;
 result = (*self->f_ops->f_allocate)(self,mode,start,size);
 if (E_ISOK(result))
     self->f_flag |= FILE_FLAG_DIDWRITE;
 return result;
}


PUBLIC off_t KCALL
file_seek(struct file *__restrict self,
          off_t off, int whence) {
 off_t result;
 CHECK_HOST_DOBJ(self);
 if (!self->f_ops->f_seek) {
  mode_t mode = (assert(self->f_node),
                 self->f_node->i_attr.ia_mode);
  if (S_ISFIFO(mode) || S_ISCHR(mode) || S_ISSOCK(mode))
       return -ESPIPE;
  else return -EPERM;
 }
 if (FILE_ISLOCKLESS(self)) {
  result = (*self->f_ops->f_seek)(self,off,whence);
 } else {
  result = rwlock_write(&self->f_lock);
  if (E_ISERR(result)) return result;
  /* Make sure to delete the back-flag when
   * seeking away from an O_APPEND state. */
  if (off != 0 || whence != SEEK_END)
      self->f_flag &= ~(FILE_FLAG_BACK);
  result = (*self->f_ops->f_seek)(self,off,whence);
  rwlock_endwrite(&self->f_lock);
 }
 return result;
}

PUBLIC errno_t KCALL
file_ioctl(struct file *__restrict self,
           int name, USER void *arg) {
 CHECK_HOST_DOBJ(self);
 if unlikely(!self->f_ops->f_ioctl)
    return -EINVAL;
 return (*self->f_ops->f_ioctl)(self,name,arg);
}

PUBLIC ssize_t KCALL
file_readdir(struct file *__restrict self,
             USER struct dirent *buf,
             size_t bufsize, rdmode_t mode) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 CHECK_USER_DATA(buf,bufsize);
 if ((self->f_mode&O_ACCMODE) == O_WRONLY ||
     !self->f_ops->f_readdir) return -EPERM;
 if (FILE_ISLOCKLESS(self)) {
  result = (*self->f_ops->f_readdir)(self,buf,bufsize,mode);
 } else {
  result = rwlock_write(&self->f_lock);
  if (E_ISERR(result)) return result;
  result = (*self->f_ops->f_readdir)(self,buf,bufsize,mode);
  rwlock_endwrite(&self->f_lock);
 }
 return result;
}

PUBLIC errno_t KCALL
file_sync(struct file *__restrict self) {
 errno_t result;
 CHECK_HOST_DOBJ(self);
 if ((self->f_mode&O_ACCMODE) == O_RDONLY) return -EROFS;
 if (!self->f_ops->f_sync) return -EOK; /* Don't error out in this case! */
 if (FILE_ISLOCKLESS(self)) {
  result = (*self->f_ops->f_sync)(self);
 } else {
  result = rwlock_write(&self->f_lock);
  if (E_ISERR(result)) return result;
  result = (*self->f_ops->f_sync)(self);
  rwlock_endwrite(&self->f_lock);
 }
 return result;
}

DECL_END

#endif /* !GUARD_KERNEL_CORE_FILE_C */
