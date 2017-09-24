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
#ifndef GUARD_KERNEL_FS_FD_C
#define GUARD_KERNEL_FS_FD_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <assert.h>
#include <dev/blkdev.h>
#include <fcntl.h>
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
#include <hybrid/types.h>
#include <kernel/mman.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <kos/fcntl.h>
#include <sys/syslog.h>
#include <limits.h>
#include <linker/module.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/task.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

DECL_BEGIN

/* FD alias options */
#define FD_DENTRY_ALIASING_INODE    1 /* Any D-Entry can be used as an INode. */
#define FD_FILE_ALIASING_INODE      1 /* Any File can be used as an INode. */
#define FD_FILE_ALIASING_DENTRY     1 /* Any File can be used as an D-Entry. */


PRIVATE void    KCALL null_incref_decref(void *__restrict UNUSED(o)) {}
PRIVATE ssize_t KCALL null_read_write(void *__restrict UNUSED(o), USER void *UNUSED(buf), size_t UNUSED(bufsize)) { return -EBADF; }
PRIVATE ssize_t KCALL null_pread_pwrite(void *__restrict UNUSED(o), pos_t UNUSED(pos), USER void *UNUSED(buf), size_t UNUSED(bufsize)) { return -EBADF; }
PRIVATE off_t   KCALL null_seek(void *__restrict UNUSED(o), off_t UNUSED(off), int UNUSED(whence)) { return -EBADF; }
PRIVATE errno_t KCALL null_ioctl(void *__restrict UNUSED(o), int UNUSED(name), USER void *UNUSED(arg)) { return -EBADF; }
PRIVATE ssize_t KCALL null_readdir(void *__restrict UNUSED(o), USER struct dirent *UNUSED(buf), size_t UNUSED(bufsize), rdmode_t UNUSED(mode)) { return -EBADF; }
PRIVATE errno_t KCALL null_sync(void *__restrict UNUSED(o)) { return -EBADF; }

PRIVATE void KCALL fd_inode_incref(struct inode *__restrict self) { INODE_INCREF(self); }
PRIVATE void KCALL fd_inode_decref(struct inode *__restrict self) { INODE_DECREF(self); }
PRIVATE void KCALL fd_file_incref(struct file *__restrict self) { FILE_INCREF(self); }
PRIVATE void KCALL fd_file_decref(struct file *__restrict self) { FILE_DECREF(self); }
PRIVATE void KCALL fd_dentry_incref(struct dentry *__restrict self) { DENTRY_INCREF(self); }
PRIVATE void KCALL fd_dentry_decref(struct dentry *__restrict self) { DENTRY_DECREF(self); }
PRIVATE void KCALL fd_task_incref(struct task *__restrict self) { TASK_WEAK_INCREF(self); }
PRIVATE void KCALL fd_task_decref(struct task *__restrict self) { TASK_WEAK_DECREF(self); }
PRIVATE void KCALL fd_mman_incref(struct mman *__restrict self) { MMAN_INCREF(self); }
PRIVATE void KCALL fd_mman_decref(struct mman *__restrict self) { MMAN_DECREF(self); }
PRIVATE void KCALL fd_stack_incref(struct stack *__restrict self) { STACK_INCREF(self); }
PRIVATE void KCALL fd_stack_decref(struct stack *__restrict self) { STACK_DECREF(self); }

/* This is a requirement to ensure that all ops (not just the first) are properly aligned. */
STATIC_ASSERT(IS_ALIGNED(sizeof(struct fdops),FDOPS_ALIGN));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
PUBLIC struct fdops const fd_ops[FD_TYPE_COUNT] = {
    [0 ... FD_TYPE_COUNT-1] = {
        .fd_incref  = &null_incref_decref,
        .fd_decref  = &null_incref_decref,
        .fd_read    = (ssize_t(KCALL *)(void *__restrict,USER void *,size_t))&null_read_write,
        .fd_write   = (ssize_t(KCALL *)(void *__restrict,USER void const *,size_t))&null_read_write,
        .fd_pread   = (ssize_t(KCALL *)(void *__restrict,USER void *,size_t,pos_t))&null_pread_pwrite,
        .fd_pwrite  = (ssize_t(KCALL *)(void *__restrict,USER void const *,size_t,pos_t))&null_pread_pwrite,
        .fd_seek    = &null_seek,
        .fd_ioctl   = &null_ioctl,
        .fd_readdir = &null_readdir,
        .fd_sync    = &null_sync,
    },
    [FD_TYPE_INODE] = {
        .fd_incref  = (void(KCALL *)(void *__restrict))&fd_inode_incref,
        .fd_decref  = (void(KCALL *)(void *__restrict))&fd_inode_decref,
        .fd_sync    = (errno_t(KCALL *)(void *__restrict))&inode_syncattr,
    },
    [FD_TYPE_FILE] = {
        .fd_incref  = (void(KCALL *)(void *__restrict))&fd_file_incref,
        .fd_decref  = (void(KCALL *)(void *__restrict))&fd_file_decref,
        .fd_read    = (ssize_t(KCALL *)(void *__restrict,USER void *,size_t))&file_read,
        .fd_write   = (ssize_t(KCALL *)(void *__restrict,USER void const *,size_t))&file_write,
        .fd_pread   = (ssize_t(KCALL *)(void *__restrict,USER void *,size_t,pos_t))&file_pread,
        .fd_pwrite  = (ssize_t(KCALL *)(void *__restrict,USER void const *,size_t,pos_t))&file_pwrite,
        .fd_seek    = (off_t(KCALL *)(void *__restrict,off_t,int))&file_seek,
        .fd_ioctl   = (errno_t(KCALL *)(void *__restrict,int,USER void *))&file_ioctl,
        .fd_readdir = (ssize_t(KCALL *)(void *__restrict,USER struct dirent *,size_t,rdmode_t))&file_readdir,
        .fd_sync    = (errno_t(KCALL *)(void *__restrict))&file_sync,
    },
    [FD_TYPE_DENTRY] = {
        .fd_incref  = (void(KCALL *)(void *__restrict))&fd_dentry_incref,
        .fd_decref  = (void(KCALL *)(void *__restrict))&fd_dentry_decref,
    },
    [FD_TYPE_TASK] = {
        .fd_incref  = (void(KCALL *)(void *__restrict))&fd_task_incref,
        .fd_decref  = (void(KCALL *)(void *__restrict))&fd_task_decref,
    },
    [FD_TYPE_MMAN] = {
        .fd_incref  = (void(KCALL *)(void *__restrict))&fd_mman_incref,
        .fd_decref  = (void(KCALL *)(void *__restrict))&fd_mman_decref,
    },
    [FD_TYPE_STACK] = {
        .fd_incref  = (void(KCALL *)(void *__restrict))&fd_stack_incref,
        .fd_decref  = (void(KCALL *)(void *__restrict))&fd_stack_decref,
    },
};
#pragma GCC diagnostic pop

PUBLIC struct fd const fd_invalid = {
    .fo_ops = &fd_ops[FD_TYPE_NULL],
    .fo_ptr = NULL,
};


PUBLIC struct fdman fdman_kernel = {
    /* Create 3 references initially:
     *  - 'fdman_kernel'
     *  - 'inittask.t_fdman'
     *  - '__bootcpu.c_idle.t_fdman' */
#ifdef CONFIG_DEBUG
    .fm_refcnt = 3,
#else
    .fm_refcnt = 0x80000003,
#endif
    .fm_cwd    = &fs_root,
    .fm_root   = &fs_root,
    .fm_umask  = 0022,
    .fm_hint   = 0,
    .fm_veca   = 0,
    .fm_vecc   = 0,
    .fm_vecm   = 0, /* You want to use it anyways?
                     * >> You better work harder for something
                     *    you're not supposed to do! */
    .fm_vecv   = NULL,
};




PUBLIC struct fdman *KCALL fdman_init(struct fdman *self) {
 if (self) {
  CHECK_HOST_DOBJ(self);
  self->fm_refcnt = 1;
  rwlock_cinit(&self->fm_lock);   /*< R/W-lock for accessing this file descriptor manager. */
  self->fm_cwd  = &fs_root;
  self->fm_root = &fs_root;
  ATOMIC_FETCHADD(fs_root.d_refcnt,2);
  self->fm_hint = 0;
  self->fm_veca = 0;
  self->fm_vecc = 0;
  self->fm_vecm = FDMAN_DEFAULT_VECM;
  self->fm_vecv = NULL;
  self->fm_umask = 0022;
 }
 return self;
}
PUBLIC void KCALL fdman_destroy(struct fdman *__restrict self) {
 struct fd *iter,*end;
 CHECK_HOST_DOBJ(self);
 assert(self != THIS_FDMAN);
 assert(!self->fm_refcnt);
 DENTRY_DECREF(self->fm_cwd);
 DENTRY_DECREF(self->fm_root);
 end = (iter = self->fm_vecv)+self->fm_veca;
 for (; iter != end; ++iter) FD_SAFE_DECREF(*iter);
 free(self->fm_vecv);
 free(self);
}

PUBLIC SAFE WUNUSED REF struct fd KCALL
fdman_get(struct fdman *__restrict self, int no) {
 struct fd result;
 CHECK_HOST_DOBJ(self);
 task_nointr();
 fdman_read(self);
 if likely((unsigned int)no < self->fm_veca) {
  result = self->fm_vecv[no];
  result.fo_flags &= ~FD_MASK;
 } else {
  switch (no) {

  case AT_FDCWD:
   result.fo_ops = &fd_ops[FD_TYPE_DENTRY];
   result.fo_obj.fo_dentry = self->fm_cwd;
   break;

  case AT_FDROOT:
   result.fo_ops = &fd_ops[FD_TYPE_DENTRY];
   result.fo_obj.fo_dentry = self->fm_root;
   break;

  case AT_THIS_TASK:
   result.fo_ops = &fd_ops[FD_TYPE_TASK];
   result.fo_obj.fo_task = THIS_TASK;
   break;

  case AT_THIS_MMAN:
   result.fo_ops = &fd_ops[FD_TYPE_MMAN];
   result.fo_obj.fo_mman = THIS_MMAN;
   break;

  case AT_THIS_STACK:
   result.fo_obj.fo_stack = THIS_TASK->t_ustack;
   if (!result.fo_obj.fo_stack) goto inval;
   result.fo_ops = &fd_ops[FD_TYPE_MMAN];
   break;

  default:
inval:
   result = fd_invalid;
   break;
  }
 }
 FD_INCREF(result);
 fdman_endread(self);
 task_endnointr();
 return result;
}

PUBLIC SAFE WUNUSED REF struct dentry *KCALL
fdman_get_dentry(struct fdman *__restrict self, int no) {
 REF struct dentry *result;
 CHECK_HOST_DOBJ(self);
 result = E_PTR(fdman_read(self));
 if (E_ISERR(result)) goto end;
 if likely((unsigned int)no < self->fm_veca) {
  struct fd entry = self->fm_vecv[no];
  if (FD_SAFE_ISTYPE(entry,FD_TYPE_DENTRY))
   result = entry.fo_obj.fo_dentry;
#if FD_FILE_ALIASING_DENTRY
  else if (FD_SAFE_ISTYPE(entry,FD_TYPE_FILE))
   result = entry.fo_obj.fo_file->f_dent;
#endif /* FD_FILE_ALIASING_DENTRY */
  else { result = E_PTR(-ENOTDIR); goto end2; }
 } else if (no == AT_FDCWD) {
  result = self->fm_cwd;
 } else if (no == AT_FDROOT) {
  result = self->fm_root;
 } else {
  result = E_PTR(-EBADF);
  goto end2;
 }
 DENTRY_INCREF(result);
end2: fdman_endread(self);
end:  return result;
}
PUBLIC SAFE WUNUSED REF struct inode *KCALL
fdman_get_inode(struct fdman *__restrict self, int no) {
 REF struct inode *result;
 CHECK_HOST_DOBJ(self);
 result = E_PTR(fdman_read(self));
 if (E_ISERR(result)) goto end;
 if likely((unsigned int)no < self->fm_veca) {
  struct fd entry = self->fm_vecv[no];
  if (FD_SAFE_ISTYPE(entry,FD_TYPE_INODE))
   result = entry.fo_obj.fo_inode;
#if FD_DENTRY_ALIASING_INODE
  else if (FD_SAFE_ISTYPE(entry,FD_TYPE_DENTRY)) {
   result = dentry_inode(entry.fo_obj.fo_dentry);
   goto end2; }
#if FD_FILE_ALIASING_INODE
  else if (FD_SAFE_ISTYPE(entry,FD_TYPE_FILE))
   result = entry.fo_obj.fo_file->f_node;
#elif FD_FILE_ALIASING_DENTRY
  else if (FD_SAFE_ISTYPE(entry,FD_TYPE_FILE)) {
   result = dentry_inode(entry.fo_obj.fo_file->f_dent);
   goto end2; }
#endif /* FD_FILE_ALIASING_INODE */
#endif /* FD_DENTRY_ALIASING_INODE */
  else { result = E_PTR(-ENOTDIR); goto end2; }
  INODE_INCREF(result);
#if FD_DENTRY_ALIASING_INODE
 } else if (no == AT_FDCWD) {
  result = dentry_inode(self->fm_cwd);
 } else if (no == AT_FDROOT) {
  result = dentry_inode(self->fm_root);
#endif /* FD_DENTRY_ALIASING_INODE */
 } else {
  result = E_PTR(-EBADF);
 }
end2: fdman_endread(self);
end:  return result;
}
PUBLIC SAFE WUNUSED REF struct file *KCALL
fdman_get_file(struct fdman *__restrict self, int no) {
 REF struct file *result;
 CHECK_HOST_DOBJ(self);
 result = E_PTR(fdman_read(self));
 if (E_ISERR(result)) goto end;
 if likely((unsigned int)no < self->fm_veca) {
  struct fd entry = self->fm_vecv[no];
  if (FD_SAFE_ISTYPE(entry,FD_TYPE_FILE))
   result = entry.fo_obj.fo_file;
  else { result = E_PTR(-ENOTDIR); goto end2; }
 } else {
  result = E_PTR(-EBADF);
  goto end2;
 }
 FILE_INCREF(result);
end2: fdman_endread(self);
end:  return result;
}
PUBLIC SAFE WUNUSED REF struct task *KCALL
fdman_get_task(struct fdman *__restrict self, int no) {
 REF struct task *result;
 CHECK_HOST_DOBJ(self);
 result = E_PTR(fdman_read(self));
 if (E_ISERR(result)) goto end;
 if likely((unsigned int)no < self->fm_veca) {
  struct fd entry = self->fm_vecv[no];
  if (FD_SAFE_ISTYPE(entry,FD_TYPE_TASK))
   result = entry.fo_obj.fo_task;
  else { result = E_PTR(-ENOTDIR); goto end2; }
 } else if (no == AT_THIS_TASK) {
  result = THIS_TASK;
 } else {
  result = E_PTR(-EBADF);
  goto end2;
 }
 if (!TASK_TRYINCREF(result))
      result = E_PTR(-EINVAL);
end2: fdman_endread(self);
end:  return result;
}
PUBLIC SAFE WUNUSED REF struct mman *KCALL
fdman_get_mman(struct fdman *__restrict self, int no) {
 REF struct mman *result;
 CHECK_HOST_DOBJ(self);
 result = E_PTR(fdman_read(self));
 if (E_ISERR(result)) goto end;
 if likely((unsigned int)no < self->fm_veca) {
  struct fd entry = self->fm_vecv[no];
  if (FD_SAFE_ISTYPE(entry,FD_TYPE_MMAN))
   result = entry.fo_obj.fo_mman;
  else { result = E_PTR(-ENOTDIR); goto end2; }
  MMAN_INCREF(result);
 } else {
  result = E_PTR(-EBADF);
 }
end2: fdman_endread(self);
end:  return result;
}


#define FDMAN_VECTOR_THRESHOLD 8

PRIVATE errno_t KCALL
fdman_minsize_unlocked(struct fdman *__restrict self,
                       unsigned int cnt) {
 struct fd *iter,*end; 
 unsigned int new_alloc;
 assert(cnt < self->fm_vecm);
 if (cnt <= self->fm_veca) return -EOK;
 new_alloc = MIN(CEIL_ALIGN(cnt,FDMAN_VECTOR_THRESHOLD),self->fm_vecm);
again:
 assertf(new_alloc >= self->fm_veca,"%u < %u",new_alloc,self->fm_veca);
 iter = trealloc(struct fd,self->fm_vecv,new_alloc);
 if unlikely(!iter && new_alloc != cnt) { new_alloc = cnt; goto again; }
 if unlikely(!iter) return -ENOMEM;
 end           = (self->fm_vecv = iter)+new_alloc;
 iter         +=  self->fm_veca;
 self->fm_veca =  new_alloc;
 assert(iter < end);
 for (; iter != end; ++iter) *iter = fd_invalid;
 return -EOK;
}

PUBLIC SAFE errno_t KCALL
fdman_set(struct fdman *__restrict self,
          int no, struct fd const fp) {
 struct fd old_fp;
 errno_t error = -EOK;
 CHECK_HOST_DOBJ(self);
 assertf(!FD_SAFE_ISNULL(fp),"Cannot set NULL-descriptor");
 error = fdman_write(self);
 assert(self->fm_vecm <= INT_MAX);
 if (E_ISERR(error)) goto err;
 /* Handle special/illegal fd numbers. */
 if unlikely((unsigned int)no >= self->fm_vecm) {
  REF struct dentry *dent;
  if (FD_SAFE_ISTYPE(fp,FD_TYPE_DENTRY))
      dent = fp.fo_obj.fo_dentry,
      DENTRY_INCREF(dent);
  else if (FD_SAFE_ISTYPE(fp,FD_TYPE_FILE))
      dent = fp.fo_obj.fo_file->f_dent,
      DENTRY_INCREF(dent);
  else { error = -EINVAL; goto err2; }
  if (no == AT_FDCWD) {
   old_fp.fo_obj.fo_dentry = self->fm_cwd;
   self->fm_cwd            = dent;
  } else if (no == AT_FDROOT) {
   old_fp.fo_obj.fo_dentry = self->fm_root;
   self->fm_root           = dent;
  } else {
   error = -ENFILE;
   asserte(ATOMIC_DECFETCH(dent->d_refcnt) != 0);
   goto err2;
  }
  fdman_endwrite(self);
  CHECK_HOST_DOBJ(old_fp.fo_obj.fo_dentry);
  DENTRY_DECREF(old_fp.fo_obj.fo_dentry);
  return error;
 }
 /* Make sure the fd-vector is large enough. */
 if unlikely((unsigned int)no+1 >= self->fm_vecm) { error = -EBADF; goto err2; }
 error = fdman_minsize_unlocked(self,(unsigned int)no+1);
 if (E_ISERR(error)) goto err2;

 /* Exchange the file descriptor under the given number. */
 old_fp = self->fm_vecv[no];
 self->fm_vecv[no] = fp;
 FD_SAFE_INCREF(fp);

 if (FD_SAFE_ISNULL(old_fp))
     ++self->fm_vecc;
 fdman_endwrite(self);

 /* Drop the reference from the old descriptor. */
 FD_SAFE_DECREF(old_fp);

 return error;
err2: fdman_endwrite(self);
err:  return error;
}



PUBLIC SAFE struct fd KCALL
fdman_del_unlocked(struct fdman *__restrict self, int no) {
 struct fd old_fd = fd_invalid;
 CHECK_HOST_DOBJ(self);
 assert(fdman_writing(self));
 if ((unsigned int)no < self->fm_veca) {
  struct fd *fd_end,*pfd = &self->fm_vecv[(unsigned int)no];
  old_fd = *pfd;
  old_fd.fo_flags &= ~(FD_MASK);
  if (FD_ISNULL(old_fd)) goto end;
  assert(self->fm_vecc);
  --self->fm_vecc;
  *pfd = fd_invalid;
  /* Check if we can reduce the fd-vector size. */
  fd_end = self->fm_vecv+self->fm_veca;
  assert(pfd < fd_end);
  if (pfd+FDMAN_VECTOR_THRESHOLD > fd_end) goto end;
  do if ((++pfd,!FD_SAFE_ISNULL(*pfd))) goto end;
  while (pfd != fd_end);
  /* Trim the vector size. */
  while (no && FD_SAFE_ISNULL(self->fm_vecv[no-1])) --no;
  if (!no) {
   assert(self->fm_vecc == 0);
   free(self->fm_vecv);
   self->fm_veca = 0;
   self->fm_vecv = NULL;
  } else {
   assert(self->fm_vecc != 0);
   no = MIN(CEIL_ALIGN(no,FDMAN_VECTOR_THRESHOLD),self->fm_veca);
   if ((unsigned int)no < self->fm_veca) {
    pfd = trealloc(struct fd,self->fm_vecv,(unsigned int)no);
    if likely(pfd) {
     self->fm_vecv = pfd;
     self->fm_veca = (unsigned int)no;
    }
   }
  }
 }
end:
 return old_fd;
}

PUBLIC SAFE int KCALL
fdman_put_nearby_unlocked(struct fdman *__restrict self,
                          unsigned int hint, struct fd const fp) {
 unsigned int start_hint = hint;
 CHECK_HOST_DOBJ(self);
 assert(self->fm_vecc <= self->fm_veca);
 /* Do the initial pass, searching at 'hint'. */
 for (; hint < self->fm_veca; ++hint) {
  if (FD_SAFE_ISNULL(self->fm_vecv[hint])) goto got_slot;
 }
 if (self->fm_vecc != self->fm_veca) {
  /* Do the second pass, scanning for empty slots below 'hint'. */
  hint       = FDMAN_PUT_RESCAN_START;
  start_hint = MIN(start_hint,self->fm_veca);
  for (; hint < start_hint; ++hint) {
   if (FD_SAFE_ISNULL(self->fm_vecv[hint])) goto got_slot;
  }
 }
 /* Allocate more slots in the back. */
 hint = self->fm_veca;
 if (hint < self->fm_vecm) goto got_slot;
 return -EMFILE;
got_slot:
 /* We've got a slot we can use! */
 start_hint = (unsigned int)fdman_minsize_unlocked(self,hint+1);
 if (E_ISERR(start_hint)) return (int)start_hint;
 self->fm_hint = hint;
 assert(hint < self->fm_veca);
 assert(FD_SAFE_ISNULL(self->fm_vecv[hint]));
 ++self->fm_vecc;
 self->fm_vecv[hint] = fp;
 FD_SAFE_INCREF(fp);
 return hint;
}

PUBLIC SAFE errno_t KCALL
fdman_del(struct fdman *__restrict self, int no) {
 struct fd old_fd; errno_t error;
 CHECK_HOST_DOBJ(self);
 error = fdman_write(self);
 if (E_ISERR(error)) return error;
 old_fd = fdman_del_unlocked(self,no);
 fdman_endwrite(self);
 if (FD_ISNULL(old_fd)) return -EBADF;
 FD_DECREF(old_fd);
 return error;
}

PUBLIC SAFE int KCALL
fdman_put(struct fdman *__restrict self,
          struct fd const fp) {
 int result = fdman_write(self);
 if (E_ISOK(result))
     result = fdman_put_nearby_unlocked(self,self->fm_hint,fp);
 fdman_endwrite(self);
 return result;
}
PUBLIC SAFE int KCALL
fdman_put_nearby(struct fdman *__restrict self,
                 unsigned int hint, struct fd const fp) {
 int result = fdman_write(self);
 if (E_ISOK(result))
     result = fdman_put_nearby_unlocked(self,hint,fp);
 fdman_endwrite(self);
 return result;
}


SYSCALL_DEFINE3(ioctl,int,fd,int,cmd,USER void *,arg) {
 syscall_slong_t result; struct fd f;
 struct fdman *fdm = THIS_FDMAN;
 task_crit();
 switch (cmd) {

  /* Set/Clear the close-on-exec flag. */
 case FIOCLEX:
 case FIONCLEX:
  result = fdman_write(fdm);
  if (E_ISERR(result)) goto end;
  if ((unsigned int)fd >= fdm->fm_veca) {
   result = -EBADF;
  } else {
   struct fd *fp = &fdm->fm_vecv[(unsigned int)fd];
   if (cmd == FIOCLEX) fp->fo_flags |=  FD_CLOEXEC;
   else                fp->fo_flags &= ~FD_CLOEXEC;
   result = -EOK;
  }
  fdman_endwrite(fdm);
  break;

 {
  int on;
 case FIOASYNC:
 case FIONBIO:
  if (copy_from_user(&on,arg,sizeof(on)))
      result = -EFAULT;
  else {
   oflag_t old_mode,new_mode,chflag;
   struct file *fp = fdman_get_file(fdm,fd);
   if (E_ISERR(fp)) { result = E_GTERR(fp); goto end; }
   if (cmd == FIOASYNC) chflag = FASYNC;
   else                 chflag = O_NONBLOCK;
   do old_mode = ATOMIC_READ(fp->f_mode),
      new_mode = (old_mode&~chflag) | (on ? chflag : 0);
   while (!ATOMIC_CMPXCH_WEAK(fp->f_mode,old_mode,new_mode));
   FILE_DECREF(fp);
   result = -EOK;
  }
 } break;

 default:
  /* Call the generic IOCTL callback. */
  f = fdman_get(fdm,fd);
  result = (*f.fo_ops->fd_ioctl)(f.fo_ptr,cmd,arg);
  FD_DECREF(f);
  break;
 }
end: task_endcrit();
 return result;
//ebadf: result = -EBADF; goto end;
}

SYSCALL_DEFINE3(fcntl,int,fd,int,cmd,USER void *,arg) {
 syscall_slong_t result;
 struct fd f;
 struct fdman *fdm = THIS_FDMAN;
 task_crit();
 switch (cmd) {

 case FCNTL_KOS_TYPE:
  result = fdman_read(fdm);
  if (E_ISERR(result)) goto end;
  assert(result == 0);
  if ((unsigned int)arg < fdm->fm_veca) {
   result = FD_SAFE_OPS(fdm->fm_vecv[(unsigned int)arg])-fd_ops;
  } else {
   /* Special/symbolic file descriptor types. */
   switch ((int)arg) {
   case AT_FDCWD:
   case AT_FDROOT:
    result = FD_TYPE_DENTRY;
    break;
   case AT_THIS_TASK:
    result = FD_TYPE_TASK;
    break;
   case AT_THIS_MMAN:
    result = FD_TYPE_MMAN;
    break;
   case AT_THIS_STACK:
    result = FD_TYPE_STACK;
    break;
   default: break;
   }
  }
  if (!result) result = -EBADF;
  fdman_endread(fdm);
  break;

 case F_DUPFD:
 case F_DUPFD_CLOEXEC:
  f = fdman_get(fdm,fd);
  if (FD_SAFE_ISNULL(f)) goto ebadf;
  if (cmd == F_DUPFD_CLOEXEC)
      f.fo_flags |= FD_CLOEXEC;
  /* Use the given argument as hint when placing new descriptors. */
  /* TODO: Unix says that this kind of copy _MUST_ use descriptors
   *       '>= arg', meaning that we must somehow disable
   *       fdman_put_nearby's behavior of preferring to re-use low-index
   *       descriptors in favor of allocating more high-index ones.
   */
  result = fdman_put_nearby(fdm,(unsigned int)(uintptr_t)arg,f);
  FD_SAFE_DECREF(f);
  break;

 case F_GETFD:
  result = fdman_read(fdm);
  if (E_ISERR(result)) goto end;
  if ((unsigned int)fd >= fdm->fm_veca) {
   result = -EBADF;
  } else {
   result = fdm->fm_vecv[(unsigned int)fd].fo_flags & FD_MASK;
  }
  fdman_endread(fdm);
  break;

 case F_SETFD:
  result = fdman_write(fdm);
  if (E_ISERR(result)) goto end;
  if ((unsigned int)fd >= fdm->fm_veca) {
   result = -EBADF;
  } else {
   struct fd *fp = &fdm->fm_vecv[(unsigned int)fd];
   fp->fo_flags &= ~FD_MASK;
   fp->fo_flags |= (uintptr_t)arg & FD_MASK;
   result = -EOK;
  }
  fdman_endwrite(fdm);
  break;

#define F_SETFL_MASK  (O_APPEND|O_ASYNC|O_DIRECT|O_NOATIME|O_NONBLOCK)
 {
  REF struct file *fp;
 case F_GETFL:
 case F_SETFL:
 case F_SETFL_XCH:
  fp = fdman_get_file(fdm,fd);
  if (E_ISERR(fp)) { result = E_GTERR(fp); goto end; }
  if (cmd == F_GETFL) result = ATOMIC_READ(fp->f_mode);
  else {
   oflag_t old_mode,new_mode;
   do old_mode = ATOMIC_READ(fp->f_mode),
      new_mode = (old_mode&~(F_SETFL_MASK)) | ((oflag_t)arg & F_SETFL_MASK);
   while (!ATOMIC_CMPXCH_WEAK(fp->f_mode,old_mode,new_mode));
   result = cmd == F_SETFL_XCH ? old_mode : -EOK;
  }
  FILE_DECREF(fp);
 } break;

 case F_SETPIPE_SZ:
 case F_GETPIPE_SZ:
  /* TODO: Implement when pipes are added. */
  result = -EBADF;
  break;

 case F_GETLK:
 case F_SETLK:
 case F_SETLKW:
 case F_GETOWN:
 case F_SETOWN:
 case F_GETOWN_EX:
 case F_SETOWN_EX:
 case F_GETSIG:
 case F_SETSIG:
 case F_GETLEASE:
 case F_SETLEASE:
 case F_NOTIFY:
  /* TODO */
  result = -EINVAL;
  break;

 default:
  result = -EINVAL;
  break;
 }
end: task_endcrit();
 return result;
ebadf: result = -EBADF; goto end;
}

SYSCALL_DEFINE3(dup3,int,oldfd,int,newfd,oflag_t,flags) {
 int result;
 struct fd f;
 struct fdman *fdm = THIS_FDMAN;
 /* Validate arguments. */
 if unlikely(flags&~(O_CLOEXEC|O_CLOFORK)) return -EINVAL;
 if unlikely(oldfd == newfd) return -EINVAL;
 task_crit();
 f = fdman_get(fdm,oldfd);
 if (FD_SAFE_ISNULL(f))
  result = -EBADF;
 else {
  if (flags&O_CLOEXEC) f.fo_flags |= FD_CLOEXEC;
  if (flags&O_CLOFORK) f.fo_flags |= FD_CLOFORK;
  result = fdman_set(fdm,newfd,f);
  FD_SAFE_DECREF(f);
 }
 task_endcrit();
 if (E_ISOK(result)) result = newfd;
 return result;
}
SYSCALL_DEFINE1(dup,int,fd) {
 int result;
 struct fd f;
 struct fdman *fdm = THIS_FDMAN;
 task_crit();
 f = fdman_get(fdm,fd);
 if (FD_SAFE_ISNULL(f))
  result = -EBADF;
 else {
  result = fdman_put(fdm,f);
  FD_DECREF(f);
 }
 task_endcrit();
 return result;
}
SYSCALL_DEFINE1(close,int,fd) {
 errno_t error;
 /* Very simple: Just close a file descriptor. */
 task_crit();
 error = fdman_del(THIS_FDMAN,fd);
 task_endcrit();
 return error;
}


#ifdef __ARCH_EXTENDED_FS_SYSCALLS
SYSCALL_LDEFINE4(lseek,int,fd,syscall_ulong_t,off_hi,
                              syscall_ulong_t,off_lo,
                       int,whence)
#define offset  ((loff_t)off_hi << 32 | (loff_t)off_lo)
#else
SYSCALL_DEFINE3(lseek,int,fd,loff_t,offset,int,whence)
#endif
{
 struct fd fent; loff_t res;
 task_crit();
 fent = fdman_get(THIS_FDMAN,fd);
 res = (loff_t)(*fent.fo_ops->fd_seek)(fent.fo_ptr,offset,whence);
 FD_DECREF(fent);
 task_endcrit();
 return (s64)res;
}
#undef offset

SYSCALL_DEFINE3(read,int,fd,USER void *,buf,size_t,bufsize) {
 struct fd fent; ssize_t res;
 task_crit();
 fent = fdman_get(THIS_FDMAN,fd);
 res = (*fent.fo_ops->fd_read)(fent.fo_ptr,buf,bufsize);
 FD_DECREF(fent);
 task_endcrit();
 cpu_validate_counters(true);
 return res;
}
SYSCALL_DEFINE3(write,int,fd,USER void const *,buf,size_t,bufsize) {
 struct fd fent; ssize_t res;
 task_crit();
 fent = fdman_get(THIS_FDMAN,fd);
 res = (*fent.fo_ops->fd_write)(fent.fo_ptr,buf,bufsize);
 FD_DECREF(fent);
 task_endcrit();
 cpu_validate_counters(true);
 return res;
}

SYSCALL_DEFINE4(xreaddir,int,fd,USER struct dirent *,buf,
                         size_t,bufsize,rdmode_t,mode) {
 struct fd fent; ssize_t res;
 task_crit();
 fent = fdman_get(THIS_FDMAN,fd);
 res = (*fent.fo_ops->fd_readdir)(fent.fo_ptr,buf,bufsize,mode);
 FD_DECREF(fent);
 task_endcrit();
 return res;
}


#ifdef __ARCH_EXTENDED_FS_SYSCALLS
SYSCALL_DEFINE5(pread64,int,fd,USER void *,buf,size_t,bufsize,
                        syscall_ulong_t,pos_hi,syscall_ulong_t,pos_lo)
#define pos  ((loff_t)pos_hi << 32 | (loff_t)pos_lo)
#else
SYSCALL_DEFINE4(pread64,int,fd,USER void *,buf,size_t,bufsize,lpos_t,pos)
#endif
{
 struct fd fent; ssize_t res;
 task_crit();
 fent = fdman_get(THIS_FDMAN,fd);
 res = (*fent.fo_ops->fd_pread)(fent.fo_ptr,buf,bufsize,pos);
 FD_DECREF(fent);
 task_endcrit();
 return res;
}
#undef pos

#ifdef __ARCH_EXTENDED_FS_SYSCALLS
SYSCALL_DEFINE5(pwrite64,int,fd,USER void const *,buf,size_t,bufsize,
                         syscall_ulong_t,pos_hi,syscall_ulong_t,pos_lo)
#define pos  ((loff_t)pos_hi << 32 | (loff_t)pos_lo)
#else
SYSCALL_DEFINE4(pwrite64,int,fd,USER void const *,buf,size_t,bufsize,lpos_t,pos)
#endif
{
 struct fd fent; ssize_t res;
 task_crit();
 fent = fdman_get(THIS_FDMAN,fd);
 res = (*fent.fo_ops->fd_pwrite)(fent.fo_ptr,buf,bufsize,pos);
 FD_DECREF(fent);
 task_endcrit();
 return res;
}
#undef pos

#if 0
PRIVATE SAFE ssize_t KCALL
do_syncfs(struct fdman *__restrict fdm,
          struct superblock *fs) {
 ssize_t result; errno_t error;
 struct fd *iter,*end;
 result = fdman_read(fdm);
 if (E_ISERR(result)) goto done;
 end = (iter = fdm->fm_vecv)+fdm->fm_veca;
 assert(result == 0);
 for (; iter != end; ++iter) {
  error = (*FD_SAFE_OPS(*iter)->fd_sync)(iter->fo_ptr);
  if (E_ISOK(error)) ++result;
  else if (error == -EINTR ||
           error == -ENOMEM) {
   /* Stop in critical error. */
   result = error;
   break;
  }
 }
 fdman_endread(fdm);
done:
 return result;
}
#endif

SYSCALL_DEFINE0(sync) {
 ssize_t result = 0;
 errno_t temp = 0;
 struct superblock *sb;
 task_crit();
 result = rwlock_read(&fs_mountlock);
 if (E_ISERR(result)) goto end;
 FS_FOREACH_MOUNT(sb) {
  temp = superblock_sync(sb);
  if (E_ISOK(temp)) ++result;
  else if (temp == -ENOMEM || temp == -EINTR) {
   /* Stop for crucial errors. */
   result = temp;
   break;
  }
 }
 rwlock_endread(&fs_mountlock);
 /* If nothing else was synced, return the
  * last error that prevented anything. */
 if (!result) result = (ssize_t)temp;
end:
 task_endcrit();
 return result;
}
SYSCALL_DEFINE1(syncfs,int,fd) {
 ssize_t result;
 struct inode *ino;
 struct fdman *fdm = THIS_FDMAN;
 task_crit();
 ino = fdman_get_inode(fdm,fd);
 if (E_ISERR(ino))
  result = E_GTERR(ino);
 else {
  CHECK_HOST_DOBJ(ino);
  CHECK_HOST_DOBJ(ino->i_super);
  result = superblock_sync(ino->i_super);
  INODE_DECREF(ino);
 }
 task_endcrit();
 return result;
}

SYSCALL_DEFINE1(fsync,int,fd) {
 struct fd fent; errno_t res;
 task_crit();
 fent = fdman_get(THIS_FDMAN,fd);
 res = (*fent.fo_ops->fd_sync)(fent.fo_ptr);
 FD_DECREF(fent);
 task_endcrit();
 return res;
}

SYSCALL_DEFINE1(fdatasync,int,fd) {
 struct fd fent; errno_t res;
 task_crit();
 fent = fdman_get(THIS_FDMAN,fd);
 /* XXX: Different implementation from 'fsync?' */
 res = (*fent.fo_ops->fd_sync)(fent.fo_ptr);
 FD_DECREF(fent);
 task_endcrit();
 return res;
}

PRIVATE size_t KCALL
dentry_get_fdname2(struct dentry *__restrict self,
                   struct dentry *__restrict root,
                   USER char *buf, size_t size) {
 char *iter = buf,*end = buf+size;
 if (self != root)
     iter += dentry_get_fdname2(self->d_parent,root,buf,size);
 if (iter < end) { *iter = '/'; } ++iter;
 memcpy(iter,self->d_name.dn_name,
        MIN((size_t)(end-iter),
             self->d_name.dn_size)*
        sizeof(char));
 iter += self->d_name.dn_size;
 return (size_t)(iter-buf);
}
PRIVATE size_t KCALL
dentry_get_fdname(struct dentry *__restrict self,
                  struct dentry *__restrict root,
                  USER char *buf, size_t size) {
 char *iter = buf,*end = buf+size;
 iter += dentry_get_fdname2(self,root,buf,size);
 if (iter < end) { *iter = '\0'; } ++iter;
 return (size_t)(iter-buf);
}


SYSCALL_DEFINE4(xfdname,int,fd,int,type,USER char *,buf,size_t,bufsize) {
 REF struct dentry *ent;
 REF struct dentry *root; ssize_t res;
 struct fdman *fdm = THIS_FDMAN;
 task_crit();
 res = fdman_read(fdm);
 if (E_ISERR(res)) goto end;
 root = fdm->fm_root;
 CHECK_HOST_DOBJ(root);
 DENTRY_INCREF(root);
 ent = fdman_get_dentry(fdm,fd);
 fdman_endread(fdm);

 if (E_ISERR(ent)) { res = E_GTERR(ent); goto end2; }
 switch (type) {

 case FDNAME_PATH:
  res = call_user_worker(&dentry_get_fdname,4,ent,root,buf,bufsize);
  break;

 case FDNAME_HEAD:
  if (ent == root) {
empty_string:
   res = 1;
   if (bufsize && copy_to_user(buf,"",1))
       res = -EFAULT;
  } else {
   res = call_user_worker(&dentry_get_fdname,4,
                           ent->d_parent,root,buf,bufsize);
  }
  break;

 case FDNAME_TAIL:
  if (ent == root) goto empty_string;
  res = (ent->d_name.dn_size+1)*sizeof(char);
  if (copy_to_user(buf,ent->d_name.dn_name,MIN(bufsize,(size_t)res)))
      res = -EFAULT;
  break;

 default:
  res = -EINVAL;
  break;
 }
 DENTRY_DECREF(ent);
end2: DENTRY_DECREF(root);
end:  task_endcrit();
 return res;
}
SYSCALL_DEFINE2(getcwd,USER char *,buf,size_t,bufsize) {
 return SYSC_xfdname(AT_FDCWD,FDNAME_PATH,buf,bufsize);
}

SYSCALL_DEFINE1(umask,mode_t,mask) {
 /* Simply exchange the UMASK of the calling thread. */
 return ATOMIC_XCH(THIS_UMASK,mask & S_IRWXUGO);
}


DECL_END

#endif /* !GUARD_KERNEL_FS_FD_C */
