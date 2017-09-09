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
#ifndef GUARD_KERNEL_FS_FS_C
#define GUARD_KERNEL_FS_FS_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1 /* memrchr */

#include <assert.h>
#include <dev/blkdev.h>
#include <fcntl.h>
#include <fs/access.h>
#include <fs/basic_types.h>
#include <fs/dentry.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <hybrid/atomic.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <kernel/boot.h>
#include <kernel/user.h>
#include <kos/syslog.h>
#include <linker/module.h>
#include <malloc.h>
#include <sched/cpu.h>
#include <sched/task.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <hybrid/align.h>

DECL_BEGIN

/* Filesystem root directory entry. */
PUBLIC struct dentry fs_root = {
    /* NOTE: Start out with 3 references (+2 for 'fdman_kernel') */
#ifdef NDEBUG
    .d_refcnt = 0x80000003, /* Be a bit more robust in case something's actually wrong. */
#else
    .d_refcnt = 0x00000003,
#endif
    .d_name = {
        .dn_name = "",
        .dn_size = 0,
        .dn_hash = DENTRYNAME_EMPTY_HASH,
    },
    .d_inode      = NULL, /* Filled later, once the an initial filesystem is mounted. */
    .d_inode_lock = ATOMIC_RWLOCK_INIT,
    .d_parent     = NULL, /* The one and only directory entry _without_ a parent! */
    .d_subs_lock  = ATOMIC_RWLOCK_INIT,
    .d_next       = NULL, /* This'll always be NULL, too. */
    .d_subs = {
        .ht_tabc = 0,
        .ht_taba = 0,
        .ht_tabv = NULL,
    },
};

PRIVATE DEFINE_ATOMIC_RWLOCK(fs_mountlock); /*< Lock for the global mounting point list. */
PRIVATE LIST_HEAD(struct superblock) fs_mountlist; /*< [lock(fs_mountlock)] Global mounting point list. */

LOCAL bool KCALL dentry_rehashsubs_unlocked(struct dentry *__restrict self);
LOCAL struct dentry *KCALL dentry_getsub_unlocked(struct dentry const *__restrict self, struct dentryname const *__restrict name);
LOCAL void KCALL dentry_delsub_unlocked(struct dentry *__restrict self, struct dentry *__restrict sub);
LOCAL REF struct dentry *KCALL dentry_addsub_unlocked(struct dentry *__restrict self, struct dentryname const *__restrict name);
PRIVATE REF struct dentry *KCALL dentry_walklnk(struct dentry *__restrict link_dir, struct dentry_walker *__restrict walker, struct inode *__restrict link_node);


INTERN ATTR_FREETEXT
void KCALL mount_root_filesystem(void) {
 struct superblock *rootfs;
 struct blkdev *bootpart = get_bootpart();
 rootfs = blkdev_mksuper(bootpart,NULL,0);
 if (E_ISERR(rootfs)) {
  syslogf(LOG_FS|LOG_ERROR,
          FREESTR("[FS] Failed to create root-fs superblock for %[dev_t]: %[errno]\n"),
          bootpart->bd_device.d_id,-E_GTERR(rootfs));
 } else {
  errno_t error; struct fsaccess ac = {0,0};
  error = dentry_mount(&fs_root,&ac,rootfs);
  SUPERBLOCK_DECREF(rootfs);
  if (E_ISERR(error)) {
   syslogf(LOG_FS|LOG_ERROR,
           FREESTR("[FS] Failed to mount root filesystem: %[errno]\n"),
           -error);
  }
 }
 BLKDEV_DECREF(bootpart);
}


PUBLIC struct inode *KCALL
inode_cinit(struct inode *self) {
 if (self) {
  CHECK_HOST_DOBJ(self);
  self->i_refcnt = 1;
  rwlock_cinit(&self->i_attr_lock);
  atomic_rwlock_cinit(&self->i_file.i_files_lock);
  atomic_rwlock_cinit(&self->i_file.i_flock.fl_lock);
  sig_cinit(&self->i_file.i_flock.fl_avail);
  sig_cinit(&self->i_file.i_flock.fl_unlock);
 }
 return self;
}
PUBLIC errno_t KCALL
inode_setup(struct inode *__restrict self,
            struct superblock *__restrict sb,
            struct instance *__restrict owner) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(sb);
 CHECK_HOST_DOBJ(owner);
 assert(self != &sb->sb_root);
 if (!INSTANCE_INCREF(owner))
      return -EPERM;
 SUPERBLOCK_INCREF(sb);
 self->i_owner = owner; /* Inherit reference. */
 self->i_super = sb; /* Inherit reference. */
 /* Insert the new node into the superblock's list of them. */
 atomic_rwlock_write(&sb->sb_nodes_lock);
 LIST_INSERT(sb->sb_nodes,self,i_nodes);
 atomic_rwlock_endwrite(&sb->sb_nodes_lock);
 return -EOK;
}
PUBLIC SAFE void KCALL
inode_invalidate(struct inode *__restrict self) {
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->i_file.i_files_lock);
 if (!self->i_file.i_module) {
  atomic_rwlock_endread(&self->i_file.i_files_lock);
  return;
 }
 atomic_rwlock_upgrade(&self->i_file.i_files_lock);
 self->i_file.i_module = NULL;
 atomic_rwlock_endwrite(&self->i_file.i_files_lock);
}

PUBLIC SAFE struct superblock *KCALL
superblock_cinit(struct superblock *self) {
 if (self) {
  CHECK_HOST_DOBJ(self);
  inode_cinit(&self->sb_root);
  atomic_rwlock_cinit(&self->sb_nodes_lock);
  atomic_rwlock_cinit(&self->sb_achng_lock);
  atomic_rwlock_cinit(&self->sb_mount.sm_mount_lock);
  self->sb_root.i_super = self;
 }
 return self;
}
PUBLIC SAFE errno_t KCALL
superblock_setup(struct superblock *__restrict self,
                 struct instance *__restrict owner) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(owner);
 assert(!self->sb_root.i_owner);
 if (!INSTANCE_INCREF(owner))
      return -EPERM;
 self->sb_root.i_owner = owner; /* Inherit reference. */
 return -EOK;
}
PUBLIC errno_t KCALL
superblock_flush(struct superblock *__restrict self) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->sb_ops);
 CHECK_HOST_DOBJ(self->sb_root.i_owner);
 /* Flush all modified INodes. */
 for (;;) {
  struct inode *changed;
  atomic_rwlock_write(&self->sb_achng_lock);
  changed = self->sb_achng;
  if (changed) INODE_INCREF(changed);
  atomic_rwlock_endwrite(&self->sb_achng_lock);
  if (!changed) break;
  /* Flush INode attributes.
   * NOTE: Upon success, this will remove the
   *       node from the 'sb_achng' chain. */
  error = inode_flushattr(changed);
  INODE_DECREF(changed);
  if (E_ISERR(error)) return error;
 }
 if (self->sb_ops->sb_sync) {
  error = (*self->sb_ops->sb_sync)(self);
  if (E_ISERR(error)) return error;
 }
 return self->sb_blkdev ? blkdev_flush(self->sb_blkdev) : -EOK;
}

PUBLIC ssize_t KCALL
superblock_remove_inode(struct superblock *__restrict self,
                        struct inode *__restrict node) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(node);
 assertf(node->i_super == self,"Invalid INode isn't apart of this superblock");
 assertf(node != &self->sb_root,"Can't remove the filesystem root INode");
 if (self->sb_ops->sb_remove_inode)
     return (*self->sb_ops->sb_remove_inode)(self,node);
 /* Unsupported operation. */
 return -EPERM;
}


PUBLIC struct dentry *KCALL
dentry_init(struct dentry *self) {
 if (self) {
  CHECK_HOST_DOBJ(self);
  atomic_rwlock_cinit(&self->d_inode_lock);
  atomic_rwlock_cinit(&self->d_subs_lock);

 }
 return self;
}

PUBLIC void KCALL
dentry_destroy(struct dentry *__restrict self) {
 struct dentry *parent;
 CHECK_HOST_DOBJ(self);
 parent = self->d_parent;
 assertf(parent,
         "Only the root node may be missing a parent, "
         "but that node must never be destroyed!");
 CHECK_HOST_DOBJ(parent);
 CHECK_HOST_DATA(self->d_name.dn_name,
                 self->d_name.dn_size);
 assert(!self->d_subs.ht_tabc);
 /* Remove this directory entry from its parent's hash-table. */
 atomic_rwlock_write(&parent->d_subs_lock);
 dentry_delsub_unlocked(parent,self);
 atomic_rwlock_endwrite(&parent->d_subs_lock);

 if (self->d_inode) {
  CHECK_HOST_DOBJ(self->d_inode);
  INODE_DECREF(self->d_inode);
 }
 free(self->d_subs.ht_tabv);
 DENTRY_DECREF(self->d_parent);
 free(self->d_name.dn_name);
 free(self);
}




LOCAL iattrset_t KCALL
iattr_compare(struct iattr const *__restrict a,
              struct iattr const *__restrict b) {
 iattrset_t result = IATTR_NONE;
 CHECK_HOST_TOBJ(a);
 CHECK_HOST_TOBJ(b);
 if ((a->ia_mode&IATTR_MODE_VMASK) != 
     (b->ia_mode&IATTR_MODE_VMASK))
      result |= IATTR_MODE;
 if (a->ia_uid != b->ia_uid) result |= IATTR_UID;
 if (a->ia_gid != b->ia_gid) result |= IATTR_GID;
 if (a->ia_siz != b->ia_siz) result |= IATTR_SIZ;
 if (a->ia_atime.tv_sec  != b->ia_atime.tv_sec ||
     a->ia_atime.tv_nsec != b->ia_atime.tv_nsec)
     result |= IATTR_ATIME;
 if (a->ia_mtime.tv_sec  != b->ia_mtime.tv_sec ||
     a->ia_mtime.tv_nsec != b->ia_mtime.tv_nsec)
     result |= IATTR_MTIME;
 if (a->ia_ctime.tv_sec  != b->ia_ctime.tv_sec ||
     a->ia_ctime.tv_nsec != b->ia_ctime.tv_nsec)
     result |= IATTR_CTIME;
 return result;
}
LOCAL void KCALL
iattr_mirror(struct iattr *__restrict dst,
             struct iattr const *__restrict src,
             iattrset_t mirror) {
 CHECK_HOST_DOBJ(dst);
 CHECK_HOST_TOBJ(src);
 if (mirror&IATTR_MODE) {
  dst->ia_mode &= ~(IATTR_MODE_VMASK);
  dst->ia_mode |=  (src->ia_mode&IATTR_MODE_VMASK);
 }
 if (mirror&IATTR_UID) dst->ia_uid = src->ia_uid;
 if (mirror&IATTR_GID) dst->ia_gid = src->ia_gid;
 if (mirror&IATTR_SIZ) dst->ia_siz = src->ia_siz;
 if (mirror&IATTR_ATIME) dst->ia_atime = src->ia_atime;
 if (mirror&IATTR_MTIME) dst->ia_mtime = src->ia_mtime;
 if (mirror&IATTR_CTIME) dst->ia_ctime = src->ia_ctime;
}

PUBLIC void KCALL
inode_destroy(struct inode *__restrict self) {
 errno_t error;
 struct superblock *super;
 CHECK_HOST_DOBJ(self);
 super = self->i_super;
 CHECK_HOST_DOBJ(super);
 error = inode_flushattr(self);
 if (E_ISERR(error)) {
  syslogf(LOG_ERROR|LOG_FS,
          "[FS] Failed to flush INode %ld before unloading: %[errno]\n",
         (long)self->i_ino,-error);
 }
 if (INODE_ISSUPERBLOCK(self)) {
  assert(super == INODE_TOSUPERBLOCK(self));
  assertf(!super->sb_nodes,"But any nodes at all should have kept the superblock alive!");
  assertf(!super->sb_achng,"But any changed nodes should have kept the superblock alive!");
  assertf(!super->sb_root.__i_nlink,"But any mounting point should have kept the superblock alive!");
  /* Flush the superblock one last time. */
  if (super->sb_ops->sb_sync) {
   error = (*super->sb_ops->sb_sync)(super);
   if (E_ISERR(error)) {
    syslogf(LOG_ERROR|LOG_FS,
            "[FS] Failed to synchronize superblock on %[dev_t] before unloading: %[errno]\n",
            super->sb_blkdev ? super->sb_blkdev->bd_device.d_id : 0,-error);
   }
  }
  /* Call the superblock fini-function. */
  if (super->sb_ops->sb_fini)
    (*super->sb_ops->sb_fini)(super);

  assertf(!super->sb_root.i_refcnt,"You can't just revive a superblock!");
  assertf(!super->sb_nodes,"But any nodes at all should have kept the superblock alive!");
  assertf(!super->sb_achng,"But any changed nodes should have kept the superblock alive!");
  assertf(!super->sb_root.__i_nlink,"But any mounting point should have kept the superblock alive!");
  free(super->sb_mount.sm_mountv);

  /* Drop the optional block-device reference from the superblock. */
  if (super->sb_blkdev)
      BLKDEV_DECREF(super->sb_blkdev);
  super = NULL;
 } else {
  atomic_rwlock_write(&super->sb_nodes_lock);
  LIST_REMOVE(self,i_nodes);
  atomic_rwlock_endwrite(&super->sb_nodes_lock);
 }
 if (self->i_ops->ino_fini)
   (*self->i_ops->ino_fini)(self);
 CHECK_HOST_DOBJ(self->i_owner);
 INSTANCE_DECREF(self->i_owner);
 free(self);
 /* Drop a reference from the associated superblock. */
 if (super) INODE_DECREF(&super->sb_root);
}


PUBLIC errno_t KCALL
inode_mayaccess(struct inode *__restrict self,
                struct fsaccess const *__restrict access,
                unsigned int rwx) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 error = rwlock_read(&self->i_attr_lock);
 if (E_ISERR(error)) return error;
 if ((self->i_attr.ia_mode&rwx) == rwx) goto ok;
 if (!access->fa_uid || !access->fa_gid) goto ok; /* Always allow ROOT access. */

 error = -EACCES;
ok:
 rwlock_endread(&self->i_attr_lock);
 return error;
}

PUBLIC errno_t KCALL
inode_stat(struct inode *__restrict self,
           struct stat64 *__restrict statbuf) {
 errno_t error;
 struct blkdev *block;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(statbuf);
 memset(statbuf,0,sizeof(struct stat64));

 /* Read attribute-specific statbuf-> */
 error = rwlock_read(&self->i_attr_lock);
 if (E_ISERR(error)) return error;
 statbuf->st_mode   = self->i_attr.ia_mode;
 statbuf->st_uid    = self->i_attr.ia_uid;
 statbuf->st_gid    = self->i_attr.ia_gid;
#ifdef CONFIG_32BIT_TIME
 statbuf->st_atim32 = self->i_attr.ia_atime;
 statbuf->st_mtim32 = self->i_attr.ia_mtime;
 statbuf->st_ctim32 = self->i_attr.ia_ctime;
#else
 statbuf->st_atim64 = self->i_attr.ia_atime;
 statbuf->st_mtim64 = self->i_attr.ia_mtime;
 statbuf->st_ctim64 = self->i_attr.ia_ctime;
#endif
 statbuf->st_size64 = self->i_attr.ia_siz;
 rwlock_endread(&self->i_attr_lock);

 /* XXX: KOS doesn't implement holes (yet?) */
 statbuf->st_blocks64 = CEILDIV(statbuf->st_size64,S_BLKSIZE);
 statbuf->st_ino64 = self->i_ino;
 statbuf->st_nlink = ATOMIC_READ(self->i_nlink);

#ifdef CONFIG_32BIT_TIME
 statbuf->st_atim64.tv_sec  = (time64_t)statbuf->st_atim32.tv_sec;
 statbuf->st_atim64.tv_nsec = statbuf->st_atim32.tv_sec;
 statbuf->st_mtim64.tv_sec  = (time64_t)statbuf->st_mtim32.tv_sec;
 statbuf->st_mtim64.tv_nsec = statbuf->st_mtim32.tv_sec;
 statbuf->st_ctim64.tv_sec  = (time64_t)statbuf->st_ctim32.tv_sec;
 statbuf->st_ctim64.tv_nsec = statbuf->st_ctim32.tv_sec;
#else
 statbuf->st_atim32.tv_sec  = (time32_t)statbuf->st_atim64.tv_sec;
 statbuf->st_atim32.tv_nsec = statbuf->st_atim64.tv_sec;
 statbuf->st_mtim32.tv_sec  = (time32_t)statbuf->st_mtim64.tv_sec;
 statbuf->st_mtim32.tv_nsec = statbuf->st_mtim64.tv_sec;
 statbuf->st_ctim32.tv_sec  = (time32_t)statbuf->st_ctim64.tv_sec;
 statbuf->st_ctim32.tv_nsec = statbuf->st_ctim64.tv_sec;
#endif

 if ((block = self->i_super->sb_blkdev) != NULL) {
  statbuf->st_dev     = block->bd_device.d_id;
  statbuf->st_blksize = block->bd_blocksize;
 } else {
  statbuf->st_blksize = S_BLKSIZE; /* Default */
 }

 if (INODE_ISDEV(self))
     statbuf->st_rdev = INODE_TODEV(self)->d_id;

 /* Call a custom stat-callback and allow it override stat information. */
 if (self->i_ops->ino_stat)
     error = (*self->i_ops->ino_stat)(self,statbuf);

 return error;
}


PRIVATE errno_t KCALL
inode_mirror_diskattr_unlocked(struct inode *__restrict self,
                               iattrset_t changed) {
 struct superblock *sb;
 errno_t error = -EOK;
 CHECK_HOST_DOBJ(self);
 if (changed != IATTR_NONE) {
  assert(self->i_ops);
  if (self->i_ops->ino_setattr) {
   /* Force a disk update of the new INode attributes. */
   error = (*self->i_ops->ino_setattr)(self,changed);
  } else if (changed&IATTR_SIZ) {
   error = -EROFS; /* Size cannot be changed symbolically. */
  }
  if (E_ISERR(error)) goto end;
  /* Update the disk cache representation. */
  iattr_mirror(&self->i_attr_disk,
               &self->i_attr,
                changed);
  sb = self->i_super;
  atomic_rwlock_write(&sb->sb_achng_lock);
  /* Unbind the INode from the list of changes
   * within the associated superblock. */
  LIST_UNBIND(self,i_attr_chng);
  atomic_rwlock_endwrite(&sb->sb_achng_lock);
 }
end:
 return error;
}

PUBLIC errno_t KCALL
inode_setattr(struct inode *__restrict self,
              struct iattr const *__restrict attr,
              iattrset_t valid) {
 struct superblock *sb;
 size_t delete_later_a = 0;
 size_t delete_later_c = 0;
 REF struct file **delete_later_v = NULL;
 errno_t error;
 iattrset_t changed;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_TOBJ(attr);
 if unlikely(INODE_ISREADONLY_OR_CLOSING(self)) return -EROFS;
 /* Make sure to ignore setting the size-attribute in directories. */
 if (INODE_ISDIR(self))
     valid &= ~(IATTR_SIZ);
 error = rwlock_read(&self->i_attr_lock);
 if (E_ISERR(error)) goto end;
 changed = iattr_compare(&self->i_attr,attr);
 if (changed == IATTR_NONE) goto rend;
 error = rwlock_upgrade(&self->i_attr_lock);
 if (error == -ERELOAD) {
  error = -EOK;
  changed = iattr_compare(&self->i_attr,attr);
  if (changed == IATTR_NONE) goto wend;
 }
 if (E_ISERR(error)) goto rend;
 /* Mirror new attributes into cached data. */
 iattr_mirror(&self->i_attr,attr,changed);
 if ((changed&IATTR_SIZ) &&
     (self->i_attr.ia_siz < self->i_attr_disk.ia_siz) &&
      self->i_ops->f_invalidate) {
  /* Invalidate open file data when truncating. */
  struct file *fp;
  pos_t iv_start = self->i_attr.ia_siz;
  pos_t iv_size  = self->i_attr_disk.ia_siz-iv_start;
  atomic_rwlock_read(&self->i_file.i_files_lock);
  IFILE_FOREACH_OPEN(fp,&self->i_file) {
   if (FILE_TRYINCREF(fp)) {
    assert(fp->f_ops == self->i_ops);
    (*fp->f_ops->f_invalidate)(fp,iv_start,iv_size);
    if unlikely(!ATOMIC_DECFETCH(fp->f_refcnt)) {
     /* Special handling, because we can't safely delete the file now.
      * >> Instead, we must track pointers to all files that
      *    must be deleted, and destroy them later! */
     if (delete_later_a == delete_later_c) {
      struct file **temp;
      if (!delete_later_a) delete_later_a = 1;
      delete_later_a *= 2;
      temp = trealloc(struct file *,
                      delete_later_v,
                      delete_later_a);
      if unlikely(!temp) {
       atomic_rwlock_endread(&self->i_file.i_files_lock);
       error = -ENOMEM;
       goto wend;
      }
     }
     delete_later_v[delete_later_c++] = fp;
    }
   }
  }
  atomic_rwlock_endread(&self->i_file.i_files_lock);
 }
 if (valid&(IATTR_NOW|IATTR_SIZ)) {
  /* Mirror changes _now_! (Also goes for changes to file size) */
  changed = iattr_compare(&self->i_attr_disk,&self->i_attr);
  error   = inode_mirror_diskattr_unlocked(self,changed);
  /* Undo changes when mirroring fails. */
  if (E_ISERR(error))
      iattr_mirror(&self->i_attr,&self->i_attr_disk,changed);
 } else if (LIST_ISUNBOUND(self,i_attr_chng)) {
  sb = self->i_super;
  atomic_rwlock_write(&sb->sb_achng_lock);
  LIST_INSERT(sb->sb_achng,self,i_attr_chng);
  atomic_rwlock_endwrite(&sb->sb_achng_lock);
 }
wend: rwlock_endwrite(&self->i_attr_lock);
end:
 while (delete_later_c) file_destroy(delete_later_v[--delete_later_c]);
 free(delete_later_v);
 return error;
rend: rwlock_endread(&self->i_attr_lock); goto end;
}


PUBLIC errno_t KCALL
inode_flushattr(struct inode *__restrict self) {
 struct superblock *sb;
 errno_t error;
 bool has_write_lock = false;
 iattrset_t changes;
 CHECK_HOST_DOBJ(self);
 error = rwlock_read(&self->i_attr_lock);
 if (E_ISERR(error)) goto end;
again:
 changes = iattr_compare(&self->i_attr,
                         &self->i_attr_disk);
 if (!has_write_lock) {
  if (changes == IATTR_NONE) goto rend;
  has_write_lock = true;
  error = rwlock_upgrade(&self->i_attr_lock);
  if (error == -ERELOAD) goto again;
  if (E_ISERR(error)) goto wend;
 } else {
  if (changes == IATTR_NONE) goto wend;
 }
 assert(!LIST_ISUNBOUND(self,i_attr_chng));
 /* Mirror changes on-disk. */
 error = inode_mirror_diskattr_unlocked(self,changes);
 if (E_ISERR(error)) {
  /* Don't change attributes randomly.
   * >> There might be a (security) reason they've been changed,
   *    so even if we can't mirror those changes on-disk, keep
   *    them within the cached INode. */
#if 0
  /* Undo changes when mirroring fails. */
  iattr_mirror(&self->i_attr,&self->i_attr_disk,changes);
#endif
  goto wend;
 }
 sb = self->i_super;
 atomic_rwlock_write(&sb->sb_achng_lock);
 LIST_REMOVE(self,i_attr_chng);
 LIST_MKUNBOUND(self,i_attr_chng);
 atomic_rwlock_endwrite(&sb->sb_achng_lock);
wend: rwlock_endwrite(&self->i_attr_lock); goto end;
end: return error;
rend: rwlock_endread(&self->i_attr_lock); goto end;
}


PUBLIC void KCALL
dentryname_loadhash(struct dentryname *__restrict self) {
 size_t hash = DENTRYNAME_EMPTY_HASH;
 size_t const *iter,*end;
 CHECK_HOST_DOBJ(self);
 end = (iter = (size_t const *)self->dn_name)+(self->dn_size/sizeof(size_t));
 while (iter != end) hash += *iter++,hash *= 9;
 switch (self->dn_size % sizeof(size_t)) {
#if __SIZEOF_SIZE_T__ > 4
  case 7:  hash += (size_t)((byte_t *)iter)[6] << 48;
  case 6:  hash += (size_t)((byte_t *)iter)[5] << 40;
  case 5:  hash += (size_t)((byte_t *)iter)[4] << 32;
  case 4:  hash += (size_t)((byte_t *)iter)[3] << 24;
#endif
  case 3:  hash += (size_t)((byte_t *)iter)[2] << 16;
  case 2:  hash += (size_t)((byte_t *)iter)[1] << 8;
  case 1:  hash += (size_t)((byte_t *)iter)[0];
  default: break;
 }
 self->dn_hash = hash;
}


LOCAL bool KCALL
dentry_rehashsubs_unlocked(struct dentry *__restrict self) {
 size_t new_hash_size;
 struct dentry **new_hash_map,**bucket;
 struct dentry **miter,**mend,*iter,*next;
 CHECK_HOST_DOBJ(self);
 new_hash_size = self->d_subs.ht_taba;
 if unlikely(!new_hash_size) new_hash_size = 1;
 new_hash_size *= 2;
 new_hash_map   = tcalloc(struct dentry *,new_hash_size);
 if unlikely(!new_hash_map) return false;
 assert((self->d_subs.ht_tabv != NULL) ==
        (self->d_subs.ht_taba != 0));
 mend = (miter = self->d_subs.ht_tabv)+
                 self->d_subs.ht_taba;
 for (; miter != mend; ++miter) {
  iter = *miter;
  while (iter) {
   /* Transfer this entry into the new hash map. */
   next         = iter->d_next;
   bucket       = &new_hash_map[iter->d_name.dn_hash % new_hash_size];
   iter->d_next = *bucket;
   *bucket      = iter;
   iter = next;
  }
 }
 free(self->d_subs.ht_tabv);
 self->d_subs.ht_taba = new_hash_size;
 self->d_subs.ht_tabv = new_hash_map;
 return true;
}


/* Find a new child directory entry for the given 'name'
 * @return: NULL: Failed to find an entry. */
LOCAL struct dentry *KCALL
dentry_getsub_unlocked(struct dentry const *__restrict self,
                       struct dentryname const *__restrict name) {
 struct dentry *result = NULL;
 CHECK_HOST_TOBJ(self);
 CHECK_HOST_TOBJ(name);
 if (self->d_subs.ht_taba) {
  result = self->d_subs.ht_tabv[name->dn_hash % self->d_subs.ht_taba];
  while (result) {
   if (result->d_name.dn_size == name->dn_size &&
      !memcmp(result->d_name.dn_name,name->dn_name,name->dn_size))
       break;
   result = result->d_next;
  }
 }
 return result;
}

/* Remove 'sub' from the given directory entry 'self'. */
LOCAL void KCALL
dentry_delsub_unlocked(struct dentry *__restrict self,
                       struct dentry *__restrict sub) {
 struct dentry **piter,*iter;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(sub);
 assert(sub->d_parent == self);
 if (self->d_subs.ht_taba) {
  piter = &self->d_subs.ht_tabv[sub->d_name.dn_hash % self->d_subs.ht_taba];
  while ((iter = *piter) != NULL) {
   if (iter == sub) {
    --self->d_subs.ht_tabc;
    *piter = iter->d_next;
    break;
   }
   piter = &iter->d_next;
  }
 }
 sub->d_next = NULL;
}

/* Inherit cache-behavior from parent directories. */
#define DENTRY_ADDSUM_FINALIZE(parent,parent_node,sub,sub_node) \
{ (sub_node)->i_state |= (parent_node)->i_state&INODE_STATE_DONTCACHE; \
}

/* Allocate, add and return a new sub-entry 'name' to 'self'. */
LOCAL REF struct dentry *KCALL
dentry_addsub_unlocked(struct dentry *__restrict self,
                       struct dentryname const *__restrict name) {
 struct dentry *result,**ptab;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_TOBJ(name);
 if likely((result = dentry_new()) != NULL) {
  ++self->d_subs.ht_tabc;
  if (DHASHTAB_SHOULDREHASH(&self->d_subs) &&
     !dentry_rehashsubs_unlocked(self) &&
     !self->d_subs.ht_taba) goto err_r;
  result->d_name.dn_name = tmalloc(char,name->dn_size+1);
  if unlikely(!result->d_name.dn_name) {
err_r:
   --self->d_subs.ht_tabc;
   return NULL;
  }
  memcpy(result->d_name.dn_name,
         name->dn_name,
         name->dn_size*sizeof(char));
  result->d_name.dn_name[name->dn_size] = '\0';
  result->d_refcnt       = 1;
  result->d_parent       = self;
  result->d_name.dn_hash = name->dn_hash;
  result->d_name.dn_size = name->dn_size;
  /* Insert the newly created entry into the dentry-hash-table. */
  ptab           = &self->d_subs.ht_tabv[name->dn_hash % self->d_subs.ht_taba];
  result->d_next = *ptab;
  *ptab          = result;
  DENTRY_INCREF(self);
 }
 return result;
}

PRIVATE u32 fs_maxlink = MAXSYMLINKS;
DEFINE_EARLY_SETUP_VAR("maxsymlinks",fs_maxlink);

PRIVATE REF struct dentry *KCALL
dentry_walklnk(struct dentry *__restrict link_dir,
               struct dentry_walker *__restrict walker,
               struct inode *__restrict link_node) {
 char buf[128],*mbuf = buf,*newbuf; ssize_t buflen;
 REF struct dentry *result;
 assert(INODE_ISLNK(link_node));
 assert(link_node->i_ops->ino_readlink);
 if (++walker->dw_nlink >= fs_maxlink) return E_PTR(-ELOOP);
 buflen = sizeof(buf);
again:
 HOSTMEMORY_BEGIN {
  buflen = (*link_node->i_ops->ino_readlink)(link_node,mbuf,buflen);
 }
 HOSTMEMORY_END;
 if (E_ISERR(buflen)) { result = E_PTR(buflen); goto end; }
 if ((size_t)buflen > sizeof(buf)) {
  newbuf = mbuf == buf ? tmalloc(char,buflen) : trealloc(char,mbuf,buflen);
  if unlikely(!newbuf) { result = E_PTR(-ENOMEM); goto end; }
  mbuf = newbuf;
  goto again;
 }
 result = dentry_xwalk_internal(link_dir,walker,mbuf,(size_t)buflen);
end:
 if (mbuf != buf) free(mbuf);
 return result;
}

PUBLIC REF struct dentry *KCALL
dentry_walk(struct dentry *__restrict self,
            struct dentry_walker *__restrict walker,
            struct dentryname const *__restrict name) {
 REF struct dentry *result;
 REF struct inode *ino,*res_ino;
 bool has_write_lock = false;
 CHECK_HOST_TOBJ(name);
 CHECK_HOST_DOBJ(walker);
 CHECK_HOST_TOBJ(name);
 ino = dentry_inode(self);
 if unlikely(!ino) return E_PTR(-ENOENT);
 /* Check for read (read directory) and execute (enumerate directory) permissions. */
 result = E_PTR(inode_mayaccess(ino,&walker->dw_access,R_OK|X_OK));
 if (E_ISERR(result)) goto end;
 atomic_rwlock_read(&self->d_subs_lock);
reload_subs:
 result = dentry_getsub_unlocked(self,name);
 if (result) {
  res_ino = dentry_inode(result);
  if unlikely(!res_ino)
   result = E_PTR(-ENOENT);
  else {
   DENTRY_INCREF(result);
  }
 } else if (!INODE_ISDIR(ino) ||
            !ino->i_ops->ino_lookup) {
  result = E_PTR(-ENOTDIR);
 } else {
  if (!has_write_lock) {
   has_write_lock = true;
   if (atomic_rwlock_upgrade(&self->d_subs_lock))
       goto reload_subs;
  }
  if unlikely((result = dentry_addsub_unlocked(self,name)) == NULL)
   result = E_PTR(-ENOMEM);
  else {
   /* Lookup directory entry in INode. */
   res_ino = (*ino->i_ops->ino_lookup)(ino,result);
   assert(res_ino != 0);
   assert(result != 0);
   if likely(E_ISOK(res_ino)) {
    INODE_GET_EFFECTIVE(res_ino);
    result->d_inode = res_ino; /* Inherit reference. */
    INODE_INCREF(res_ino);
    DENTRY_ADDSUM_FINALIZE(self,ino,result,res_ino);
   } else {
    assert(has_write_lock);
    /* Delete the entry of a filed node. */
    dentry_delsub_unlocked(self,result);
    atomic_rwlock_endwrite(&self->d_subs_lock);;
    DENTRY_DECREF(result);
    result = E_PTR(res_ino);
    goto end;
   }
  }
 }
 assert(result != 0);
 if (has_write_lock)
      atomic_rwlock_endwrite(&self->d_subs_lock);
 else atomic_rwlock_endread(&self->d_subs_lock);
 if (E_ISOK(result)) {
  CHECK_HOST_TOBJ(res_ino);
  if (!walker->dw_nofollow && INODE_ISLNK(res_ino) &&
       res_ino->i_ops->ino_readlink) {
   /* Read a symbolic link. */
   struct dentry *link_result;
   dentry_used(result);
   assert(result->d_parent == self);
   link_result = dentry_walklnk(self,walker,res_ino);
   assert(link_result != 0);
   DENTRY_DECREF(result);
   result = link_result;
  }
  INODE_DECREF(res_ino);
 }
end:
 INODE_DECREF(ino);
 assert(result != 0);
 return result;
}

PUBLIC REF struct dentry *KCALL
dentry_walk2(struct dentry *__restrict self,
             struct dentry_walker *__restrict walker,
             struct dentryname const *__restrict name) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(walker);
 CHECK_HOST_TOBJ(name);
#if 1
 switch (name->dn_size) {
 case 2: if (name->dn_name[1] != '.') break;
 case 1: if (name->dn_name[0] != '.') break;
  if (name->dn_size == 1) {
   /* Directory self-reference. */
 case 0:
return_self:
   DENTRY_INCREF(self);
   return self;
  } else {
   /* Parse-directory reference. */
   if (self != walker->dw_root)
       self  = self->d_parent;
   goto return_self;
  }
  break;
 default: break;
 }
#endif
#if 0
 syslogf(LOG_DEBUG|LOG_FS,"FS::WALK(%$q)\n",
         name->dn_size,name->dn_name);
#endif
 return dentry_walk(self,walker,name);
}


PUBLIC REF struct dentry *KCALL
dentry_xwalk(struct dentry *__restrict start,
             struct dentry_walker *__restrict walker,
             HOST char const *__restrict path_str, size_t path_len) {
 REF struct dentry *result;
 result = dentry_xwalk_internal(start,walker,path_str,path_len);
 if (E_ISOK(result)) dentry_used(result);
 return result;
}
PUBLIC REF struct dentry *KCALL
dentry_xwalk_internal(struct dentry *__restrict start,
                      struct dentry_walker *__restrict walker,
                      HOST char const *__restrict path_str,
                      size_t path_len) {
 char const *iter,*end;
 struct dentryname part;
 REF struct dentry *result,*newresult;
 CHECK_HOST_DOBJ(start);
 CHECK_HOST_DOBJ(walker);
 CHECK_HOST_DATA(path_str,path_len);
 end = (iter = path_str)+path_len;
 assert(iter <= end);
 if (iter != end && *iter == '/') {
  /* Root-reference at the front. */
  do ++iter; while (iter != end && *iter == '/');
  start = walker->dw_root;
 }
 assert(iter <= end);
 assert(start);
 part.dn_name = (char *)iter;
 DENTRY_INCREF(start),result = start;
 for (;;) {
  assert(iter <= end);
  if (iter == end || *iter == '/') {
   part.dn_size = (size_t)((char *)iter-part.dn_name);
   dentryname_loadhash(&part);
   newresult = dentry_walk2(result,walker,&part);
   assert(newresult != 0);
   DENTRY_DECREF(result);
   result = newresult;
   if (E_ISERR(result)) break;
   while (iter != end) if (*++iter != '/') break;
   if (iter == end) break;
   part.dn_name = (char *)iter;
   continue;
  }
  ++iter;
 }
 return result;
}



PRIVATE int const access_flags[4] = {
 /* [O_RDONLY]        = */R_OK,
 /* [O_WRONLY]        = */W_OK,
 /* [O_RDWR]          = */R_OK|W_OK,
 /* [O_WRONLY|O_RDWR] = */R_OK|W_OK, /* ??? */
};

PUBLIC REF struct file *KCALL
dentry_open(struct dentry *__restrict dir_ent,
            struct dentryname const *__restrict ent_name,
            struct dentry_walker *__restrict walker,
            struct iattr const *__restrict attr,
            iattrset_t attr_valid, oflag_t oflags) {
 REF struct file *result;
 REF struct inode *ino,*res_ino;
 struct dentry *res_entry;
 bool has_write_lock = false;
 CHECK_HOST_DOBJ(dir_ent);
 CHECK_HOST_TOBJ(ent_name);
 CHECK_HOST_TOBJ(walker);
 ino = dentry_inode(dir_ent);
 if unlikely(!ino) return E_PTR(-ENOENT);
 /* Check for read (read directory) and execute (enumerate directory) permissions. */
 result = E_PTR(inode_mayaccess(ino,&walker->dw_access,R_OK|X_OK));
 if (E_ISERR(result)) { err_acces: INODE_DECREF(ino); return result; }
 if unlikely(INODE_ISCLOSING(ino)) { result = E_PTR(-ENOENT); goto err_acces; }
 atomic_rwlock_read(&dir_ent->d_subs_lock);
reload_subs:
 switch (ent_name->dn_size) {
 case 2: if (ent_name->dn_name[1] != '.') goto def_name;
 case 1: if (ent_name->dn_name[0] != '.') goto def_name;
  res_entry = dir_ent;
  if (ent_name->dn_size == 2)
      res_entry = dir_ent->d_parent;
  break;
def_name:default:
  res_entry = dentry_getsub_unlocked(dir_ent,ent_name);
  break;
 }
 if (res_entry != NULL &&
    (res_ino = dentry_inode(res_entry)) != NULL) {
  if (has_write_lock)
       atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
  else atomic_rwlock_endread(&dir_ent->d_subs_lock);
  INODE_DECREF(ino);
  /* An entry for 'ent_name' already exists. */
  if ((oflags&(O_CREAT|O_EXCL)) ==
              (O_CREAT|O_EXCL)) {
   INODE_DECREF(res_ino);
   /* Special case: Attempting to create a new  fails,
    *               because another already exists. */
   return E_PTR(-EEXIST);
  }
  DENTRY_INCREF(res_entry);
  goto got_res_ino;
 }
 if (!INODE_ISDIR(ino) || (
    ((oflags&O_CREAT) && !INODE_ISREADONLY(ino))
     ? !ino->i_ops->ino_mkreg
     : !ino->i_ops->ino_lookup)) {
  /* Missing operators. */
  INODE_DECREF(ino);
  return E_PTR(-ENOTDIR);
 }
 if (!has_write_lock) {
#if 0
  /* Check for write permissions on the directory. */
  result = E_PTR(inode_mayaccess(ino,access,W_OK));
  if (E_ISERR(result)) {
   atomic_rwlock_endread(&dir_ent->d_subs_lock);
   goto err_acces;
  }
#endif
  has_write_lock = true;
  if (atomic_rwlock_upgrade(&dir_ent->d_subs_lock))
      goto reload_subs;
 }
 res_entry = dentry_addsub_unlocked(dir_ent,ent_name);
 if unlikely(!res_entry) {
  /* Return ENOMEM if we've failed to add a new sub-descriptor. */
  res_ino = E_PTR(-ENOMEM);
 } else {
  if (oflags&O_CREAT) {
   /* Create missing INodes. */
   if unlikely(INODE_ISREADONLY(ino)) {
    res_ino = E_PTR(-EROFS);
   } else {
    iattrset_t attr_mode = attr_valid;
    if (!(oflags&O_EXCL)) attr_mode |= IATTR_EXISTS;
    if (oflags&O_TRUNC)   attr_mode |= IATTR_TRUNC;
    res_ino = (*ino->i_ops->ino_mkreg)(ino,res_entry,attr,attr_mode);
    assert(res_ino);
   }
  } else {
   /* Lookup existing nodes. */
   res_ino = (*ino->i_ops->ino_lookup)(ino,res_entry);
   assert(res_ino);
  }
  /* NOTE: Since we're write-locking the sub-cache of the paring directory,
   *       there can be no way the new sub-entry was already accessed,
   *       meaning we are still safe to initialize its node-link. */
  assert(!res_entry->d_inode);
  if (E_ISOK(res_ino)) {
   INODE_GET_EFFECTIVE(res_ino);
   INODE_INCREF(res_ino); /* Create reference. */
   res_entry->d_inode = res_ino; /* Inherit reference. */
   DENTRY_ADDSUM_FINALIZE(dir_ent,ino,res_entry,res_ino);
  } else {
   /* Remove 'res_entry' from the hash-table. */
   dentry_delsub_unlocked(dir_ent,res_entry);
   atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
   DENTRY_DECREF(res_entry);
#ifndef NDEBUG
   res_entry = NULL;
#endif
   goto drop_inode;
  }
 }
 atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
drop_inode:
 INODE_DECREF(ino);
 /* Drop a reference from */
 if (E_ISERR(res_ino)) {
#ifndef NDEBUG
  assert(!res_entry);
#endif
  result = E_PTR(res_ino);
  goto end;
 }
got_res_ino:
 CHECK_HOST_DOBJ(res_entry);
 CHECK_HOST_DOBJ(res_ino);
 /* Check for proper permissions in the result INode. */
 if (INODE_ISCLOSING(res_ino))
  result = E_PTR(-EBUSY);
 else if (!(oflags&O_NOFOLLOW) && INODE_ISLNK(res_ino) &&
            res_ino->i_ops->ino_readlink) {
  struct dentry *link_result; /* Follow this symbolic link. */
  syslogf(LOG_DEBUG,"WALK_SYMLINK('%[dentry]')\n",dir_ent);
  dentry_used(res_entry);
  link_result = dentry_walklnk(dir_ent,walker,res_ino);
  if (E_ISOK(link_result))
       syslogf(LOG_DEBUG,"OPEN_SYMLINK('%[dentry]')\n",link_result);
  else syslogf(LOG_DEBUG,"OPEN_SYMLINK(?) -> %[errno]\n",-E_GTERR(link_result));
  assert(link_result != 0);
  INODE_DECREF(res_ino);
  DENTRY_DECREF(res_entry);
  /* Open the file at the target of this symbolic link. */
  return dentry_openthis(link_result,&walker->dw_access,
                         attr,attr_valid,oflags);
 } else if ((result = E_PTR(inode_mayaccess(res_ino,&walker->dw_access,
                                            access_flags[oflags&O_ACCMODE])),
             E_ISERR(result))) {
  /* Make sure the requested access is permitted. */
 } else if ((oflags&O_DIRECTORY) && !INODE_ISDIR(res_ino))
  /* Return ENOTDIR when attempting to open anything
   * other than a directory with 'O_DIRECTORY'. */
  result = E_PTR(-ENOTDIR);
 else if (!res_ino->i_ops->ino_fopen)
  /* Return ENOSTR ??? When the INode cannot be opened? */
  result = E_PTR(-ENOSTR);
 else if ((oflags&O_ACCMODE) != O_RDONLY && INODE_ISREADONLY(res_ino))
  /* Don't allow write-access to read-only files. */
  result = E_PTR(-EROFS);
 else if (INODE_ISCLOSING(res_ino))
  /* Don't allow access to an INode that has been marked for closing. */
  result = E_PTR(-ENOENT);
 else if ((oflags&O_ACCMODE) != O_RDONLY && INODE_ISDIR(res_ino) &&
          !FSACCESS_CAP_WRITEDIR(&walker->dw_access))
  /* Write access to directories requires special permissions. */
  result = E_PTR(-EACCES);
 else {
  /* Actually open the INode. */
  result = (*res_ino->i_ops->ino_fopen)(res_ino,res_entry,oflags&~(O_CREAT));
  assertf(result != NULL,"Return -ENOMEM when the allocation failed");
  /* TODO: EPERM: The O_NOATIME flag was specified, but the effective user ID
   *              of the caller did not match the owner of the file and the
   *              caller was not privileged (CAP_FOWNER). */
  if (E_ISOK(result) && !(oflags&(O_CREAT|O_NOATIME)) &&
     (attr_valid&IATTR_ATIME) && !INODE_ISREADONLY_OR_CLOSING(res_ino)) {
   /* Update file access time when we're supposed to. */
   task_nointr();
   inode_setattr(res_ino,attr,IATTR_ATIME);
   task_endnointr();
  }
 }
 INODE_DECREF(res_ino);
 DENTRY_DECREF(res_entry);
end:
 return result;
}

PUBLIC REF struct file *KCALL
dentry_open_with_inode(struct dentry *__restrict dir_ent,
                   REF struct inode *__restrict res_ino,
                       struct fsaccess const *__restrict access,
                       struct iattr const *__restrict attr,
                       iattrset_t attr_valid, oflag_t oflags) {
 REF struct file *result;
 CHECK_HOST_DOBJ(dir_ent);
 CHECK_HOST_DOBJ(access);
 CHECK_HOST_DOBJ(attr);
 CHECK_HOST_DOBJ(res_ino);
 result = E_PTR(inode_mayaccess(res_ino,access,access_flags[oflags&O_ACCMODE]));
 if (E_ISERR(result));
 else if (INODE_ISCLOSING(res_ino))
  result = E_PTR(-EBUSY);
 else if (!(oflags&O_NOFOLLOW) && INODE_ISLNK(res_ino) &&
            res_ino->i_ops->ino_readlink) {
  result = E_PTR(-ENOSYS); /* TODO: read symbolic link. */
 } else if ((oflags&O_DIRECTORY) && !INODE_ISDIR(res_ino))
  /* Return ENOTDIR when attempting to open anything
   * other than a directory with 'O_DIRECTORY'. */
  result = E_PTR(-ENOTDIR);
 else if (!res_ino->i_ops->ino_fopen)
  /* Return ENOSTR ??? When the INode cannot be opened? */
  result = E_PTR(-ENOSTR);
 else if ((oflags&O_ACCMODE) != O_RDONLY && INODE_ISREADONLY(res_ino))
  /* Don't allow write-access to read-only files. */
  result = E_PTR(-EROFS);
 else if (INODE_ISCLOSING(res_ino))
  /* Don't allow access to an INode that has been marked for closing. */
  result = E_PTR(-ENOENT);
 else if ((oflags&O_ACCMODE) != O_RDONLY && INODE_ISDIR(res_ino) &&
          !FSACCESS_CAP_WRITEDIR(access))
  /* Write access to directories requires special permissions. */
  result = E_PTR(-EACCES);
 else {
  /* Actually open the INode. */
  result = (*res_ino->i_ops->ino_fopen)(res_ino,dir_ent,oflags&~(O_CREAT));
  /* TODO: EPERM: The O_NOATIME flag was specified, but the effective user ID
   *              of the caller did not match the owner of the file and the
   *              caller was not privileged (CAP_FOWNER). */
  if (E_ISOK(result) && !(oflags&(O_CREAT|O_NOATIME)) &&
     (attr_valid&IATTR_ATIME) && !INODE_ISREADONLY_OR_CLOSING(res_ino)) {
   /* Update file access time when we're supposed to. */
   task_nointr();
   inode_setattr(res_ino,attr,IATTR_ATIME);
   task_endnointr();
  }
 }
 INODE_DECREF(res_ino);
 return result;
}

PUBLIC REF struct file *KCALL
inode_open(struct inode *__restrict ino,
           struct fsaccess const *__restrict access,
           struct iattr const *__restrict attr,
           iattrset_t attr_valid, oflag_t oflags) {
 INODE_INCREF(ino);
 return dentry_open_with_inode(&fs_root,ino,access,
                                attr,attr_valid,oflags);
}

PUBLIC REF struct file *KCALL
dentry_openthis(struct dentry *__restrict dir_ent,
                struct fsaccess const *__restrict access,
                struct iattr const *__restrict attr,
                iattrset_t attr_valid, oflag_t oflags) {
 REF struct inode *res_ino;
 CHECK_HOST_DOBJ(dir_ent);
 res_ino = dentry_inode(dir_ent);
 if unlikely(!res_ino) return E_PTR(-ENOENT);
 return dentry_open_with_inode(dir_ent,res_ino,access,attr,attr_valid,oflags);
}



PUBLIC REF struct dentry *KCALL
dentry_mkreg(struct dentry *__restrict dir_ent,
             struct dentryname const *__restrict ent_name,
             struct fsaccess const *__restrict access,
             struct iattr const *__restrict attr,
             REF struct inode **result_inode) {
 struct inode *ino;
 REF struct dentry *result;
 REF struct inode *res_ino;
 CHECK_HOST_DOBJ(dir_ent);
 CHECK_HOST_TOBJ(ent_name);
 CHECK_HOST_TOBJ(access);
 CHECK_HOST_TOBJ(attr);
#ifndef NDEBUG
 if (result_inode)
     CHECK_HOST_DOBJ(result_inode);
#endif
 ino = dentry_inode(dir_ent);
 if unlikely(!ino) return E_PTR(-ENOENT);
 result = E_PTR(inode_mayaccess(ino,access,W_OK|X_OK));
 if (E_ISERR(result)) goto end;
 if unlikely(!INODE_ISDIR(ino)) { result = E_PTR(-ENOTDIR); goto end; }
 if unlikely(!ino->i_ops->ino_mkreg || INODE_ISREADONLY_OR_CLOSING(ino)) { result = E_PTR(-EROFS); goto end; }
 atomic_rwlock_read(&dir_ent->d_subs_lock);
 result = dentry_getsub_unlocked(dir_ent,ent_name);
 if (result) {
  result = E_PTR(-EEXIST);
  atomic_rwlock_endread(&dir_ent->d_subs_lock);
  goto end;
 }
 if (!atomic_rwlock_upgrade(&dir_ent->d_subs_lock)) {
  result = dentry_getsub_unlocked(dir_ent,ent_name);
  if (result) {
   result = E_PTR(-EEXIST);
wend:
   atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
   goto end;
  }
 }
 /* Create the new directory entry. */
 result = dentry_addsub_unlocked(dir_ent,ent_name);
 if unlikely(!result) { result = E_PTR(-ENOMEM); goto wend; }
 /* NOTE: The attribute-valid mask will cause mkreg to fail when the file already exists. */
 res_ino = (*ino->i_ops->ino_mkreg)(ino,result,attr,
                                    IATTR_MODE|IATTR_UID|IATTR_GID|IATTR_SIZ|
                                    IATTR_ATIME|IATTR_MTIME|IATTR_CTIME);
 if (E_ISOK(res_ino)) {
  /* Load the generated directory INode into the associated directory entry. */
  INODE_GET_EFFECTIVE(res_ino);
  if (result_inode) {
   *result_inode = res_ino;
   INODE_INCREF(res_ino);
  }
  assert(!result->d_inode);
  result->d_inode = res_ino;
  atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
 } else {
  /* Delete the miss-leading directory entry. */
  dentry_delsub_unlocked(dir_ent,result);
  atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
  DENTRY_DECREF(result);
  result = E_PTR(res_ino);
 }
end:
 INODE_DECREF(ino);
 return result;
}

PUBLIC REF struct dentry *KCALL
dentry_mkdir(struct dentry *__restrict dir_ent,
             struct dentryname const *__restrict ent_name,
             struct fsaccess const *__restrict access,
             struct iattr const *__restrict attr,
             REF struct inode **result_inode) {
 struct inode *ino;
 REF struct dentry *result;
 REF struct inode *res_ino;
 CHECK_HOST_DOBJ(dir_ent);
 CHECK_HOST_TOBJ(ent_name);
 CHECK_HOST_TOBJ(access);
 CHECK_HOST_TOBJ(attr);
#ifndef NDEBUG
 if (result_inode)
     CHECK_HOST_DOBJ(result_inode);
#endif
 ino = dentry_inode(dir_ent);
 if unlikely(!ino) return E_PTR(-ENOENT);
 result = E_PTR(inode_mayaccess(ino,access,W_OK|X_OK));
 if (E_ISERR(result)) goto end;
 if unlikely(!INODE_ISDIR(ino)) { result = E_PTR(-ENOTDIR); goto end; }
 if unlikely(!ino->i_ops->ino_mkdir || INODE_ISREADONLY_OR_CLOSING(ino)) { result = E_PTR(-EROFS); goto end; }
 atomic_rwlock_read(&dir_ent->d_subs_lock);
 result = dentry_getsub_unlocked(dir_ent,ent_name);
 if (result) {
  result = E_PTR(-EEXIST);
  atomic_rwlock_endread(&dir_ent->d_subs_lock);
  goto end;
 }
 if (!atomic_rwlock_upgrade(&dir_ent->d_subs_lock)) {
  result = dentry_getsub_unlocked(dir_ent,ent_name);
  if (result) {
   result = E_PTR(-EEXIST);
wend:
   atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
   goto end;
  }
 }
 /* Create the new directory entry. */
 result = dentry_addsub_unlocked(dir_ent,ent_name);
 if unlikely(!result) { result = E_PTR(-ENOMEM); goto wend; }
 res_ino = (*ino->i_ops->ino_mkdir)(ino,result,attr);
 if (E_ISOK(res_ino)) {
  /* Load the generated directory INode into the associated directory entry. */
  INODE_GET_EFFECTIVE(res_ino);
  if (result_inode) {
   *result_inode = res_ino;
   INODE_INCREF(res_ino);
  }
  assert(!result->d_inode);
  result->d_inode = res_ino;
  DENTRY_ADDSUM_FINALIZE(dir_ent,ino,result,res_ino);
  atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
 } else {
  /* Delete the miss-leading directory entry. */
  dentry_delsub_unlocked(dir_ent,result);
  atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
  DENTRY_DECREF(result);
  result = E_PTR(res_ino);
 }
end:
 INODE_DECREF(ino);
 return result;
}

PUBLIC REF struct dentry *KCALL
dentry_insnod(struct dentry *__restrict dir_ent,
              struct dentryname const *__restrict ent_name,
              struct fsaccess const *__restrict access,
              struct device *__restrict dev,
              REF struct inode **result_inode) {
 REF struct dentry *result;
 REF struct inode *ino,*res_ino;
 CHECK_HOST_DOBJ(dir_ent);
 CHECK_HOST_TOBJ(ent_name);
 CHECK_HOST_TOBJ(access);
 CHECK_HOST_DOBJ(dev);
 ino = dentry_inode(dir_ent);
 if unlikely(!ino) return E_PTR(-ENOENT);
 if unlikely(!S_ISDIR(ino->i_attr.ia_mode)) { result = E_PTR(-ENOTDIR); goto end; }
 if unlikely(INODE_ISREADONLY_OR_CLOSING(ino)) { result = E_PTR(-EROFS); goto end; }
 if unlikely(!ino->i_ops->ino_mknod) { result = E_PTR(-EPERM); goto end; }
 atomic_rwlock_read(&dir_ent->d_subs_lock);
 /* Check if a directory entry for the given name already exists. */
 result = dentry_getsub_unlocked(dir_ent,ent_name);
 if (result) {
  result = E_PTR(-EEXIST);
  atomic_rwlock_endread(&dir_ent->d_subs_lock);
  goto end;
 }
 if (!atomic_rwlock_upgrade(&dir_ent->d_subs_lock)) {
  result = dentry_getsub_unlocked(dir_ent,ent_name);
  if (result) {
   result = E_PTR(-EEXIST);
wend:
   atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
   goto end;
  }
 }
 /* Create the new directory entry. */
 result = dentry_addsub_unlocked(dir_ent,ent_name);
 if unlikely(!result) { result = E_PTR(-ENOMEM); goto wend; }
 /* Execute the INode mknod operator. */
 res_ino = (*ino->i_ops->ino_mknod)(ino,result,dev);
 assert(res_ino != 0);
 if (E_ISOK(res_ino)) {
  INODE_GET_EFFECTIVE(res_ino);
  if (result_inode) {
   INODE_INCREF(res_ino);
   *result_inode = res_ino;
  }
  /* Finalize the setup of the resulting directory entry. */
  result->d_inode = res_ino; /* Inherit reference. */
  DENTRY_ADDSUM_FINALIZE(dir_ent,ino,result,res_ino);
  atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
 } else {
  /* Delete the directory entry after the operator failed. */
  dentry_delsub_unlocked(dir_ent,result);
  atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
  DENTRY_DECREF(result);
  result = E_PTR(res_ino);
 }
end:
 INODE_DECREF(ino);
 return result;
}

PUBLIC errno_t KCALL
dentry_mount(struct dentry *__restrict self,
             struct fsaccess const *__restrict access,
             struct superblock *__restrict filesystem) {
 REF errno_t error = -EOK;
 REF struct inode *old_node = NULL;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_write(&self->d_inode_lock);
 if ((old_node = self->d_inode) != NULL) {
  /* xxx: Check if the node's directory is empty? */
  if (INODE_ISSUPERBLOCK(old_node) ||
     !S_ISDIR(old_node->i_attr.ia_mode))
      error = -ENOTDIR;
 }
 if (E_ISOK(error)) {
  self->d_inode = &filesystem->sb_root;
  INODE_INCREF(&filesystem->sb_root);
 }
 atomic_rwlock_endwrite(&self->d_inode_lock);
 if (E_ISOK(error)) {
  struct dentry **newvec;
  if (old_node) INODE_DECREF(old_node);
  /* Add a tracking entry for the new mounting point. */
  atomic_rwlock_write(&filesystem->sb_mount.sm_mount_lock);
  newvec = trealloc(struct dentry *,
                    filesystem->sb_mount.sm_mountv,
                    filesystem->sb_root.__i_nlink+1);
  if unlikely(!newvec) {
   atomic_rwlock_endwrite(&filesystem->sb_mount.sm_mount_lock);
   dentry_umount(self,access);
   return -ENOMEM;
  }
  filesystem->sb_mount.sm_mountv = newvec;
  newvec += filesystem->sb_root.__i_nlink++;
  DENTRY_INCREF(self); /* Create reference. */
  *newvec = self; /* Inherit reference. */
  atomic_rwlock_endwrite(&filesystem->sb_mount.sm_mount_lock);
  atomic_rwlock_write(&fs_mountlock);
  /* Add the filesystem to the list of those
   * mounted, if it was previously unbound. */
  if (LIST_ISUNBOUND(filesystem,sb_mount.sm_chain))
      LIST_INSERT(fs_mountlist,filesystem,sb_mount.sm_chain);
  atomic_rwlock_endwrite(&fs_mountlock);
 }
 return error;
}


PUBLIC errno_t KCALL
dentry_setattr(struct dentry *__restrict dir_ent,
               struct fsaccess const *__restrict access,
               struct iattr const *__restrict attr,
               iattrset_t valid) {
 errno_t result;
 struct inode *ino;
 CHECK_HOST_DOBJ(dir_ent);
 CHECK_HOST_TOBJ(access);
 CHECK_HOST_TOBJ(attr);
 ino = dentry_inode(dir_ent);
 if unlikely(ino)
  result = -ENOENT;
 else {
  result = inode_mayaccess(ino,access,R_OK);
  if (E_ISOK(result))
     result = inode_setattr(ino,attr,valid);
  INODE_DECREF(ino);
 }
 return result;
}


PUBLIC ssize_t KCALL
dentry_readlink(struct dentry *__restrict self,
                struct fsaccess const *__restrict access,
                USER char *__restrict buf, size_t bufsize) {
 struct inode *link_node;
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_TOBJ(access);
 CHECK_HOST_DATA(buf,bufsize);
 link_node = dentry_inode(self);
 if unlikely(!link_node)
  result = -ENOENT;
 else {
  result = inode_mayaccess(link_node,access,R_OK);
  if (E_ISERR(result));
  else if unlikely(!INODE_ISLNK(link_node) ||
                   !link_node->i_ops->ino_readlink)
   result = -EINVAL;
  else {
   result = (*link_node->i_ops->ino_readlink)(link_node,buf,bufsize);
  }
  INODE_DECREF(link_node);
 }
 return result;
}


PUBLIC REF struct dentry *KCALL
dentry_hrdlink(struct dentry *__restrict dir_ent,
               struct dentryname const *__restrict ent_name,
               struct fsaccess const *__restrict access,
               struct inode *__restrict dst_node) {
 REF struct dentry *result;
 REF struct inode *ino;
 errno_t error;
 CHECK_HOST_DOBJ(dir_ent);
 CHECK_HOST_TOBJ(ent_name);
 CHECK_HOST_TOBJ(access);
 CHECK_HOST_DOBJ(dst_node);
 ino = dentry_inode(dir_ent);
 if unlikely(!ino) return E_PTR(-ENOENT);
 result = E_PTR(inode_mayaccess(ino,access,W_OK));
 if (E_ISERR(result)) goto end;
 if unlikely(!INODE_ISDIR(ino)) { result = E_PTR(-ENOTDIR); goto end; }
 if unlikely(!ino->i_ops->ino_hrdlink) { result = E_PTR(-EPERM); goto end; }
 if unlikely(INODE_ISREADONLY_OR_CLOSING(ino)) { result = E_PTR(-EROFS); goto end; }
 if unlikely(ino->i_super != dst_node->i_super) { result = E_PTR(-EXDEV); goto end; }
 atomic_rwlock_read(&dir_ent->d_subs_lock);
 result = dentry_getsub_unlocked(dir_ent,ent_name);
 if unlikely(result) {
  result = E_PTR(-EEXIST);
  atomic_rwlock_endread(&dir_ent->d_subs_lock);
  goto end;
 }
 if (!atomic_rwlock_upgrade(&dir_ent->d_subs_lock)) {
  result = dentry_getsub_unlocked(dir_ent,ent_name);
  if unlikely(result) {
   result = E_PTR(-EEXIST);
wend:
   atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
   goto end;
  }
 }
 /* Create the new directory entry. */
 result = dentry_addsub_unlocked(dir_ent,ent_name);
 if unlikely(!result) { result = E_PTR(-ENOMEM); goto wend; }
 error = (*ino->i_ops->ino_hrdlink)(ino,result,dst_node);
 if (E_ISOK(error)) {
  /* Load the generated directory INode into the associated directory entry. */
  assert(!result->d_inode);
  INODE_INCREF(dst_node);
  result->d_inode = dst_node;
  DENTRY_ADDSUM_FINALIZE(dir_ent,ino,result,dst_node);
  atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
 } else {
  /* Delete the miss-leading directory entry. */
  dentry_delsub_unlocked(dir_ent,result);
  atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
  DENTRY_DECREF(result);
  result = E_PTR(error);
 }
end:
 INODE_DECREF(ino);
 return result;
}


PUBLIC REF struct dentry *KCALL
dentry_symlink(struct dentry *__restrict dir_ent,
               struct dentryname const *__restrict ent_name,
               struct fsaccess const *__restrict access,
               USER char const *__restrict target_text,
               struct iattr const *__restrict result_attr,
               REF struct inode **result_inode) {
 REF struct inode *ino;
 REF struct dentry *result;
 REF struct inode *res_ino;
 CHECK_HOST_DOBJ(dir_ent);
 CHECK_HOST_TOBJ(ent_name);
 CHECK_HOST_TOBJ(access);
 CHECK_HOST_TOBJ(result_attr);
#ifndef NDEBUG
 if (result_inode)
     CHECK_HOST_TOBJ(result_inode);
#endif
 ino = dentry_inode(dir_ent);
 if unlikely(!ino) return E_PTR(-ENOENT);
 result = E_PTR(inode_mayaccess(ino,access,W_OK));
 if (E_ISERR(result)) goto end;
 if unlikely(!INODE_ISDIR(ino)) { result = E_PTR(-ENOTDIR); goto end; }
 if unlikely(!ino->i_ops->ino_symlink) { result = E_PTR(-EPERM); goto end; }
 if unlikely(INODE_ISREADONLY_OR_CLOSING(ino)) { result = E_PTR(-EROFS); goto end; }
 atomic_rwlock_read(&dir_ent->d_subs_lock);
 result = dentry_getsub_unlocked(dir_ent,ent_name);
 if unlikely(result) {
  result = E_PTR(-EEXIST);
  atomic_rwlock_endread(&dir_ent->d_subs_lock);
  goto end;
 }
 if (!atomic_rwlock_upgrade(&dir_ent->d_subs_lock)) {
  result = dentry_getsub_unlocked(dir_ent,ent_name);
  if unlikely(result) {
   result = E_PTR(-EEXIST);
wend:
   atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
   goto end;
  }
 }
 /* Create the new directory entry. */
 result = dentry_addsub_unlocked(dir_ent,ent_name);
 if unlikely(!result) { result = E_PTR(-ENOMEM); goto wend; }
 res_ino = (*ino->i_ops->ino_symlink)(ino,result,target_text,result_attr);
 if (E_ISOK(res_ino)) {
  /* Load the generated directory INode into the associated directory entry. */
  assert(!result->d_inode);
  INODE_GET_EFFECTIVE(res_ino);
  if (result_inode) {
   INODE_INCREF(res_ino);
   *result_inode = res_ino;
  }
  result->d_inode = res_ino; /* Inherit reference. */
  DENTRY_ADDSUM_FINALIZE(dir_ent,ino,result,res_ino);
  atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
 } else {
  /* Delete the miss-leading directory entry. */
  dentry_delsub_unlocked(dir_ent,result);
  atomic_rwlock_endwrite(&dir_ent->d_subs_lock);
  DENTRY_DECREF(result);
  result = E_PTR(res_ino);
 }
end:
 INODE_DECREF(ino);
 return result;
}


PUBLIC REF struct dentry *KCALL
dentry_rename(struct dentry *__restrict dst_dir,
              struct dentryname const *__restrict dst_name,
              struct fsaccess const *__restrict access,
              struct dentry *__restrict existing_ent,
              REF struct inode **result_inode) {
 REF struct inode *existing_ino;
 REF struct inode *result_ino;
 REF struct inode *dstdir_ino;
 REF struct dentry *result;
 struct dentry *src_dir;
 REF struct inode *existing_dir_ino;
 CHECK_HOST_DOBJ(dst_dir);
 CHECK_HOST_TOBJ(dst_name);
 CHECK_HOST_TOBJ(access);
 CHECK_HOST_DOBJ(existing_ent);
 if unlikely((existing_ino = dentry_inode(existing_ent)) == NULL) return E_PTR(-ENOENT);
 if unlikely((dstdir_ino = dentry_inode(dst_dir)) == NULL) { result = E_PTR(-ENOENT); goto end1; }
 if unlikely((result = E_PTR(inode_mayaccess(existing_ino,access,W_OK)),E_ISERR(result))) goto end2;
 if unlikely((result = E_PTR(inode_mayaccess(dstdir_ino,access,W_OK)),E_ISERR(result))) goto end2;
 if unlikely(!INODE_ISDIR(dstdir_ino)) { result = E_PTR(-ENOTDIR); goto end2; }
 if unlikely(INODE_ISREADONLY_OR_CLOSING(existing_ino)) { result = E_PTR(-EROFS); goto end2; }
 if unlikely(INODE_ISREADONLY_OR_CLOSING(dstdir_ino)) { result = E_PTR(-EROFS); goto end2; }
 if unlikely(!dstdir_ino->i_ops->ino_rename) { result = E_PTR(-EPERM); goto end2; }
 if unlikely((src_dir = existing_ent->d_parent) == NULL) { result = E_PTR(-EPERM); goto end2; }
 if unlikely((existing_dir_ino = dentry_inode(src_dir)) == NULL) { result = E_PTR(-ENOENT); goto end2; }
 if (existing_dir_ino != dstdir_ino) {
  result = E_PTR(inode_mayaccess(existing_dir_ino,access,W_OK));
  if (E_ISERR(result)) goto end3;
 }
 if unlikely(existing_dir_ino->i_super != existing_ino->i_super) { result = E_PTR(-EPERM); goto end3; }
 if unlikely(INODE_ISREADONLY_OR_CLOSING(existing_dir_ino)) { result = E_PTR(-EROFS); goto end2; }
 if (src_dir == dst_dir) {
  /* Rename within the same directory (Only lock subs of one folder) */
  atomic_rwlock_read(&dst_dir->d_subs_lock);
  result = dentry_getsub_unlocked(dst_dir,dst_name);
  if unlikely(result) {
   result = E_PTR(-EEXIST);
   atomic_rwlock_endread(&dst_dir->d_subs_lock);
   goto end2;
  }
  if (!atomic_rwlock_upgrade(&dst_dir->d_subs_lock)) {
   result = dentry_getsub_unlocked(dst_dir,dst_name);
   if unlikely(result) {
    result = E_PTR(-EEXIST);
samedir_wend:
    atomic_rwlock_endwrite(&dst_dir->d_subs_lock);
    goto end2;
   }
  }
  /* Create the new target directory entry. */
  result = dentry_addsub_unlocked(dst_dir,dst_name);
  if unlikely(!result) { result = E_PTR(-ENOMEM); goto samedir_wend; }
  result_ino = (*dstdir_ino->i_ops->ino_rename)(dstdir_ino,dst_dir,
                                                existing_dir_ino,existing_ent,
                                                existing_ino);
  if (E_ISOK(result_ino)) {
   /* Load the generated directory INode into the associated directory entry. */
   assert(!result->d_inode);
   INODE_GET_EFFECTIVE(result_ino);
   if (result_inode) {
    *result_inode = result_ino;
    INODE_INCREF(result_ino);
   }
   result->d_inode = result_ino; /* Inherit reference. */
   DENTRY_ADDSUM_FINALIZE(dst_dir,dstdir_ino,result,result_ino);
   /* Remove the existing directory entry. */
   dentry_delsub_unlocked(dst_dir,existing_ent);
   atomic_rwlock_endwrite(&dst_dir->d_subs_lock);
   INODE_DECREF(existing_ino);
   /* Delete the INode from the existing directory entry. */
   atomic_rwlock_write(&existing_ent->d_inode_lock);
   existing_ino = existing_ent->d_inode; /* This reference is dropped below. */
   existing_ent->d_inode = NULL;
   atomic_rwlock_endwrite(&existing_ent->d_inode_lock);
   /* Drop the reference created above. */
   DENTRY_DECREF(existing_ent);
  } else {
   /* Delete the miss-leading directory entry. */
   dentry_delsub_unlocked(dst_dir,result);
   atomic_rwlock_endwrite(&dst_dir->d_subs_lock);
   DENTRY_DECREF(result);
   result = E_PTR(E_GTERR(result_ino));
  }
 } else {
  /* Move into a different directory (lock subs of both folders) */
  bool has_existing_writelock = false;
lock_dst_again:
  if (has_existing_writelock) {
   if (!atomic_rwlock_tryread(&dst_dir->d_subs_lock)) {
    atomic_rwlock_endwrite(&src_dir->d_subs_lock);
    has_existing_writelock = false;
    goto lock_dst_again;
   }
  } else {
   atomic_rwlock_read(&dst_dir->d_subs_lock);
  }
  result = dentry_getsub_unlocked(dst_dir,dst_name);
  if unlikely(result) {
   result = E_PTR(-EEXIST);
   atomic_rwlock_endread(&dst_dir->d_subs_lock);
   goto diffdir_wend2;
  }
  if (!atomic_rwlock_upgrade(&dst_dir->d_subs_lock)) {
   result = dentry_getsub_unlocked(dst_dir,dst_name);
   if unlikely(result) {
    result = E_PTR(-EEXIST);
diffdir_wend:
    atomic_rwlock_endwrite(&dst_dir->d_subs_lock);
diffdir_wend2:
    if (has_existing_writelock)
        atomic_rwlock_endwrite(&src_dir->d_subs_lock);
    goto end2;
   }
  }
  if (!has_existing_writelock) {
   if (!atomic_rwlock_trywrite(&src_dir->d_subs_lock)) {
    atomic_rwlock_endwrite(&dst_dir->d_subs_lock);
    atomic_rwlock_write(&src_dir->d_subs_lock);
    has_existing_writelock = true;
    goto lock_dst_again;
   }
   has_existing_writelock = true;
  }
  assert(has_existing_writelock);
  /* Create the new target directory entry. */
  result = dentry_addsub_unlocked(dst_dir,dst_name);
  if unlikely(!result) { result = E_PTR(-ENOMEM); goto diffdir_wend; }
  result_ino = (*dstdir_ino->i_ops->ino_rename)(dstdir_ino,dst_dir,
                                                existing_dir_ino,existing_ent,
                                                existing_ino);
  if (E_ISOK(result_ino)) {
   /* Load the generated directory INode into the associated directory entry. */
   assert(!result->d_inode);
   if (result_inode) {
    *result_inode = result_ino;
    INODE_INCREF(result_ino);
   }
   result->d_inode = result_ino; /* Inherit reference. */
   DENTRY_ADDSUM_FINALIZE(dst_dir,dstdir_ino,result,result_ino);
   atomic_rwlock_endwrite(&dst_dir->d_subs_lock);

   /* Remove the existing directory entry. */
   dentry_delsub_unlocked(src_dir,existing_ent);
   atomic_rwlock_endwrite(&src_dir->d_subs_lock);
   /* Delete the INode from the existing directory entry. */
   atomic_rwlock_write(&existing_ent->d_inode_lock);
   existing_ino = existing_ent->d_inode; /* This reference is dropped below. */
   existing_ent->d_inode = NULL;
   atomic_rwlock_endwrite(&existing_ent->d_inode_lock);
   /* Drop the reference created above. */
   DENTRY_DECREF(existing_ent);
  } else {
   /* Delete the miss-leading directory entry. */
   dentry_delsub_unlocked(dst_dir,result);
   atomic_rwlock_endwrite(&src_dir->d_subs_lock);
   atomic_rwlock_endwrite(&dst_dir->d_subs_lock);
   DENTRY_DECREF(result);
   result = E_PTR(E_GTERR(result_ino));
  }
 }
end3: INODE_DECREF(existing_dir_ino);
end2: INODE_DECREF(dstdir_ino);
end1: INODE_DECREF(existing_ino);
 return result;
}


PUBLIC errno_t KCALL
dentry_remove(struct dentry *__restrict self,
              struct fsaccess const *__restrict access,
              u32 mode) {
 errno_t result; struct dentry *parent;
 REF struct inode *del_inode;
 REF struct inode *parent_node;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_TOBJ(access);
again:
 atomic_rwlock_read(&self->d_inode_lock);
 if ((del_inode = self->d_inode) != NULL)
      INODE_INCREF(del_inode);
 atomic_rwlock_endread(&self->d_inode_lock);
 if unlikely(!del_inode) return -ENOENT;
 result = inode_mayaccess(del_inode,access,W_OK);
 if (E_ISERR(result)) goto end;
 /* Make sure the dentry has no sub-entires.
  * NOTE: Also keep the subs-lock held to prevent  */
 atomic_rwlock_read(&self->d_subs_lock);
 if unlikely(self->d_subs.ht_tabc) {
  atomic_rwlock_endread(&self->d_subs_lock);
not_empty:
  result = -ENOTEMPTY;
  goto end;
 }
 assert((self->d_subs.ht_taba != 0) ==
        (self->d_subs.ht_tabv != NULL));
 if (self->d_subs.ht_tabv) {
  /* Clear the sub-cache. */
  if unlikely(!atomic_rwlock_upgrade(&self->d_subs_lock)) {
   atomic_rwlock_endwrite(&self->d_subs_lock);
   goto not_empty;
  }
  free(self->d_subs.ht_tabv);
  self->d_subs.ht_tabv = NULL;
  self->d_subs.ht_taba = 0;
  atomic_rwlock_downgrade(&self->d_subs_lock);
 }

 /* Check for unmounting a superblock. */
 if (INODE_ISSUPERBLOCK(del_inode)) {
  /* Unmount a superblock. */
  struct superblock *sb; struct dentry **iter;
  if (!(mode&DENTRY_REMOVE_MNT)) { result = -EISDIR; goto end2; }
  atomic_rwlock_write(&self->d_inode_lock);
  if (self->d_inode != del_inode) {
   atomic_rwlock_endwrite(&self->d_inode_lock);
   INODE_DECREF(del_inode);
   goto again;
  }
  self->d_inode = NULL;
  atomic_rwlock_endwrite(&self->d_inode_lock);
  sb = INODE_TOSUPERBLOCK(del_inode);
  atomic_rwlock_write(&sb->sb_mount.sm_mount_lock);
  iter = sb->sb_mount.sm_mountv;
  for (;;) {
   assertf(iter != sb->sb_mount.sm_mountv+
                   sb->sb_root.__i_nlink,
           "Dangling mounting point isn't tracked properly");
   if (*iter == self) break;
   ++iter;
  }
  if (!--sb->sb_root.__i_nlink) {
   /* Set the INode closing flag to prevent new
    * streams from being opened on the filesystem. */
   ATOMIC_FETCHOR(sb->sb_root.i_state,
                  INODE_STATE_CLOSING);
   /* Last mounting point was deleted.
    * >> Actually unmount the filesystem! */
   free(sb->sb_mount.sm_mountv);
   sb->sb_mount.sm_mountv = NULL;
   atomic_rwlock_endwrite(&sb->sb_mount.sm_mount_lock);
   atomic_rwlock_write(&fs_mountlock);
   atomic_rwlock_write(&sb->sb_mount.sm_mount_lock);
   /* Unbind the last filesystem hook. */
   LIST_UNBIND(sb,sb_mount.sm_chain);
   atomic_rwlock_endwrite(&sb->sb_mount.sm_mount_lock);
   atomic_rwlock_endwrite(&fs_mountlock);
   if (sb->sb_ops->sb_umount)
     (*sb->sb_ops->sb_umount)(sb);
  } else {
   memmove(iter,iter+1,sb->sb_root.__i_nlink-
          (size_t)(iter-sb->sb_mount.sm_mountv));
   iter = trealloc(struct dentry *,
                   sb->sb_mount.sm_mountv,
                   sb->sb_root.__i_nlink);
   if (iter) sb->sb_mount.sm_mountv = iter;
   atomic_rwlock_endwrite(&sb->sb_mount.sm_mount_lock);
  }
  assert(ATOMIC_READ(self->d_refcnt) >= 2);
  ATOMIC_FETCHDEC(self->d_refcnt);
  assert(ATOMIC_READ(self->d_refcnt) >= 1);
  assert(ATOMIC_READ(del_inode->i_refcnt) >= 2);
  ATOMIC_FETCHDEC(del_inode->i_refcnt);
  assert(ATOMIC_READ(del_inode->i_refcnt) >= 1);
  result      = -EOK;
  parent      = self->d_parent;
  parent_node = NULL;
  if likely(parent) goto remove_parent;
  goto end2;
 }

 /* Confirm remove mode flags. */
 if (INODE_ISDIR(del_inode)) {
  if (!(mode&DENTRY_REMOVE_DIR)) { result = -EISDIR; goto end2; }
 } else {
  if (!(mode&DENTRY_REMOVE_REG)) { result = -ENOTDIR; goto end2; }
 }
 /* Make sure the entry has a parent. */
 if unlikely((parent = self->d_parent) == NULL) { result = -EPERM; goto end2; }
 /* Load the INode of the parent entry. */
 parent_node = dentry_inode(parent);
 if (parent_node) {
  /* Make sure the caller can access the parent node. */
  result = inode_mayaccess(parent_node,access,W_OK);
  if (E_ISERR(result)) goto end3;
 }
 result = -EOK;
 if (parent_node &&
     parent_node->i_super == del_inode->i_super &&
     parent_node->i_ops->ino_remove) {
  /* Only execute the remove operation for non-virtual elements. */
  result = (*parent_node->i_ops->ino_remove)(parent_node,self,del_inode);
 }
 if (E_ISOK(result)) {
  assert(ATOMIC_READ(self->d_refcnt) >= 1);
  atomic_rwlock_write(&self->d_inode_lock);
  if (self->d_inode == del_inode) {
   assert(ATOMIC_READ(del_inode->i_refcnt) >= 2);
   ATOMIC_FETCHDEC(del_inode->i_refcnt);
   self->d_inode = NULL; /* Delete the INode link here. */

   atomic_rwlock_endwrite(&self->d_inode_lock);
remove_parent:
   /* Remove the deleted directory entry from its parent folder. */
   atomic_rwlock_write(&parent->d_subs_lock);
   dentry_delsub_unlocked(parent,self);
   atomic_rwlock_endwrite(&parent->d_subs_lock);
   /* Clear any remaining sub-entries. */
  } else {
   atomic_rwlock_endwrite(&self->d_inode_lock);
  }
 }
end3: if (parent_node) INODE_DECREF(parent_node);
end2: atomic_rwlock_endread(&self->d_subs_lock);
end:  INODE_DECREF(del_inode);
 return result;
}

PUBLIC REF struct file *KCALL
fs_xopen(struct dentry_walker *__restrict walker,
         struct dentry *__restrict cwd,
         char const *__restrict path, size_t pathlen,
         struct iattr const *__restrict attr,
         iattrset_t attr_valid, oflag_t oflags) {
 REF struct file *result;
 struct dentryname filename;
 while (pathlen && path[0] == '/')
        cwd = walker->dw_root,--pathlen,++path;
 filename.dn_name = (char *)memrchr(path,'/',pathlen);
 if (filename.dn_name) {
  size_t leading_size = (size_t)(filename.dn_name-path);
  while (leading_size && path[leading_size-1] == '/') --leading_size;
  cwd = dentry_xwalk_internal(cwd,walker,path,leading_size);
  if (E_ISERR(cwd)) return E_PTR(E_GTERR(cwd));
  assert(cwd);
  filename.dn_size = (pathlen-(++filename.dn_name-path));
 } else {
  filename.dn_name = (char *)path;
  filename.dn_size = pathlen;
  DENTRY_INCREF(cwd);
 }
 if (filename.dn_size) {
  dentryname_loadhash(&filename);
  result = dentry_open(cwd,&filename,walker,
                       attr,attr_valid,oflags);
 } else {
  result = dentry_openthis(cwd,&walker->dw_access,
                           attr,attr_valid,oflags);
 }
 DENTRY_DECREF(cwd);
 if (E_ISOK(result)) dentry_used(result->f_dent);
 return result;
}

PUBLIC REF struct dentry *KCALL
fs_xinsnod(struct dentry_walker *__restrict walker,
           struct dentry *__restrict cwd,
           char const *__restrict path, size_t pathlen,
           struct device *__restrict dev,
           REF struct inode **result_inode) {
 REF struct dentry *result;
 struct dentryname filename;
 while (pathlen && path[0] == '/')
        cwd = walker->dw_root,--pathlen,++path;
 filename.dn_name = (char *)memrchr(path,'/',pathlen);
 if (filename.dn_name) {
  size_t leading_size = (size_t)(filename.dn_name-path);
  while (leading_size && path[leading_size-1] == '/') --leading_size;
  cwd = dentry_xwalk_internal(cwd,walker,path,leading_size);
  if (E_ISERR(cwd)) return E_PTR(E_GTERR(cwd));
  assert(cwd);
  filename.dn_size = (pathlen-leading_size)-1;
  ++filename.dn_name;
 } else {
  filename.dn_name = (char *)path;
  filename.dn_size = pathlen;
  DENTRY_INCREF(cwd);
 }
 dentryname_loadhash(&filename);
 result = dentry_insnod(cwd,&filename,&walker->dw_access,dev,result_inode);
 DENTRY_DECREF(cwd);
 if (E_ISOK(result)) dentry_used(result);
 return result;
}

PUBLIC REF struct dentry *KCALL
fs_xmkreg(struct dentry_walker *__restrict walker,
          struct dentry *__restrict cwd,
          HOST char const *__restrict path, size_t pathlen,
          struct iattr const *__restrict attr,
          REF struct inode **result_inode) {
 REF struct dentry *result;
 struct dentryname filename;
 while (pathlen && path[0] == '/')
        cwd = walker->dw_root,--pathlen,++path;
 filename.dn_name = (char *)memrchr(path,'/',pathlen);
 if (filename.dn_name) {
  size_t leading_size = (size_t)(filename.dn_name-path);
  while (leading_size && path[leading_size-1] == '/') --leading_size;
  cwd = dentry_xwalk_internal(cwd,walker,path,leading_size);
  if (E_ISERR(cwd)) return E_PTR(E_GTERR(cwd));
  assert(cwd);
  filename.dn_size = (pathlen-leading_size)-1;
  ++filename.dn_name;
 } else {
  filename.dn_name = (char *)path;
  filename.dn_size = pathlen;
  DENTRY_INCREF(cwd);
 }
 dentryname_loadhash(&filename);
 result = dentry_mkreg(cwd,&filename,&walker->dw_access,attr,result_inode);
 DENTRY_DECREF(cwd);
 if (E_ISOK(result)) dentry_used(result);
 return result;
}

PUBLIC REF struct dentry *KCALL
fs_xmkdir(struct dentry_walker *__restrict walker,
          struct dentry *__restrict cwd,
          HOST char const *__restrict path, size_t pathlen,
          struct iattr const *__restrict attr,
          REF struct inode **result_inode) {
 REF struct dentry *result;
 struct dentryname filename;
 while (pathlen && path[0] == '/')
        cwd = walker->dw_root,--pathlen,++path;
 filename.dn_name = (char *)memrchr(path,'/',pathlen);
 if (filename.dn_name) {
  size_t leading_size = (size_t)(filename.dn_name-path);
  while (leading_size && path[leading_size-1] == '/') --leading_size;
  cwd = dentry_xwalk_internal(cwd,walker,path,leading_size);
  if (E_ISERR(cwd)) return E_PTR(E_GTERR(cwd));
  assert(cwd);
  filename.dn_size = (pathlen-leading_size)-1;
  ++filename.dn_name;
 } else {
  filename.dn_name = (char *)path;
  filename.dn_size = pathlen;
  DENTRY_INCREF(cwd);
 }
 dentryname_loadhash(&filename);
 result = dentry_mkdir(cwd,&filename,&walker->dw_access,attr,result_inode);
 DENTRY_DECREF(cwd);
 if (E_ISOK(result)) dentry_used(result);
 return result;
}

PUBLIC REF struct dentry *KCALL
fs_xsymlink(struct dentry_walker *__restrict walker,
            struct dentry *__restrict cwd,
            HOST char const *__restrict path, size_t pathlen,
            USER char const *__restrict target_text,
            struct iattr const *__restrict result_attr,
            REF struct inode **result_inode) {
 REF struct dentry *result;
 struct dentryname filename;
 while (pathlen && path[0] == '/')
        cwd = walker->dw_root,--pathlen,++path;
 filename.dn_name = (char *)memrchr(path,'/',pathlen);
 if (filename.dn_name) {
  size_t leading_size = (size_t)(filename.dn_name-path);
  while (leading_size && path[leading_size-1] == '/') --leading_size;
  cwd = dentry_xwalk_internal(cwd,walker,path,leading_size);
  if (E_ISERR(cwd)) return E_PTR(E_GTERR(cwd));
  assert(cwd);
  filename.dn_size = (pathlen-leading_size)-1;
  ++filename.dn_name;
 } else {
  filename.dn_name = (char *)path;
  filename.dn_size = pathlen;
  DENTRY_INCREF(cwd);
 }
 dentryname_loadhash(&filename);
 result = dentry_symlink(cwd,&filename,&walker->dw_access,
                         target_text,result_attr,result_inode);
 DENTRY_DECREF(cwd);
 if (E_ISOK(result)) dentry_used(result);
 return result;
}
PUBLIC REF struct dentry *KCALL
fs_xhrdlink(struct dentry_walker *__restrict walker,
            struct dentry *__restrict cwd,
            HOST char const *__restrict path, size_t pathlen,
            struct inode *__restrict dst_node) {
 REF struct dentry *result;
 struct dentryname filename;
 while (pathlen && path[0] == '/')
        cwd = walker->dw_root,--pathlen,++path;
 filename.dn_name = (char *)memrchr(path,'/',pathlen);
 if (filename.dn_name) {
  size_t leading_size = (size_t)(filename.dn_name-path);
  while (leading_size && path[leading_size-1] == '/') --leading_size;
  cwd = dentry_xwalk_internal(cwd,walker,path,leading_size);
  if (E_ISERR(cwd)) return E_PTR(E_GTERR(cwd));
  assert(cwd);
  filename.dn_size = (pathlen-leading_size)-1;
  ++filename.dn_name;
 } else {
  filename.dn_name = (char *)path;
  filename.dn_size = pathlen;
  DENTRY_INCREF(cwd);
 }
 dentryname_loadhash(&filename);
 result = dentry_hrdlink(cwd,&filename,&walker->dw_access,dst_node);
 DENTRY_DECREF(cwd);
 if (E_ISOK(result)) dentry_used(result);
 return result;
}

PUBLIC REF struct dentry *KCALL
fs_xrename(struct dentry_walker *__restrict walker,
           struct dentry *__restrict cwd,
           HOST char const *__restrict path, size_t pathlen,
           struct dentry *__restrict existing_ent,
           REF struct inode **result_inode) {
 REF struct dentry *result;
 struct dentryname filename;
 while (pathlen && path[0] == '/')
        cwd = walker->dw_root,--pathlen,++path;
 filename.dn_name = (char *)memrchr(path,'/',pathlen);
 if (filename.dn_name) {
  size_t leading_size = (size_t)(filename.dn_name-path);
  while (leading_size && path[leading_size-1] == '/') --leading_size;
  cwd = dentry_xwalk_internal(cwd,walker,path,leading_size);
  if (E_ISERR(cwd)) return E_PTR(E_GTERR(cwd));
  assert(cwd);
  filename.dn_size = (pathlen-leading_size)-1;
  ++filename.dn_name;
 } else {
  filename.dn_name = (char *)path;
  filename.dn_size = pathlen;
  DENTRY_INCREF(cwd);
 }
 dentryname_loadhash(&filename);
 result = dentry_rename(cwd,&filename,&walker->dw_access,
                        existing_ent,result_inode);
 DENTRY_DECREF(cwd);
 if (E_ISOK(result)) dentry_used(result);
 return result;
}


PUBLIC REF struct dentry *KCALL
fs_xmount(struct dentry_walker *__restrict walker,
          struct dentry *__restrict cwd,
          char const *__restrict path, size_t pathlen,
          struct superblock *__restrict filesystem) {
 REF struct dentry *result;
 cwd = dentry_xwalk_internal(cwd,walker,path,pathlen);
 if (E_ISERR(cwd)) return E_PTR(E_GTERR(cwd));
 result = E_PTR(dentry_mount(cwd,&walker->dw_access,filesystem));
 if (E_ISERR(result))
      DENTRY_DECREF(cwd);
 else {
  result = cwd;
  dentry_used(cwd);
 }
 return result;
}


PUBLIC REF struct dentry *KCALL
dentry_user_xwalk(struct dentry *__restrict self,
                  struct dentry_walker *__restrict walker,
                  USER char const *path) {
 if (!path) { DENTRY_INCREF(self); return self; }
 /* TODO: Copy filename from userspace. */

 return dentry_xwalk(self,walker,path,strlen(path));
}
PUBLIC REF struct file *KCALL
fs_user_xopen(struct dentry_walker *__restrict walker,
              struct dentry *__restrict cwd, USER char const *path,
              struct iattr const *__restrict attr,
              iattrset_t attr_valid, oflag_t oflags) {
 if (!path) return dentry_openthis(cwd,&walker->dw_access,attr,attr_valid,oflags);
 /* TODO: Copy filename from userspace. */

 return fs_xopen(walker,cwd,path,strlen(path),attr,attr_valid,oflags);
}
PUBLIC REF struct dentry *KCALL
fs_user_xinsnod(struct dentry_walker *__restrict walker,
                struct dentry *__restrict cwd, USER char const *path,
                struct device *__restrict dev,
                REF struct inode **result_inode) {
 /* TODO: Copy filename from userspace. */
 return fs_xinsnod(walker,cwd,path,strlen(path),dev,result_inode);
}
PUBLIC REF struct dentry *KCALL
fs_user_xmkreg(struct dentry_walker *__restrict walker,
               struct dentry *__restrict cwd, USER char const *path,
               struct iattr const *__restrict attr,
               REF struct inode **result_inode) {
 /* TODO: Copy filename from userspace. */
 return fs_xmkreg(walker,cwd,path,strlen(path),attr,result_inode);
}
PUBLIC REF struct dentry *KCALL
fs_user_xmkdir(struct dentry_walker *__restrict walker,
               struct dentry *__restrict cwd, USER char const *path,
               struct iattr const *__restrict attr,
               REF struct inode **result_inode) {
 /* TODO: Copy filename from userspace. */
 return fs_xmkdir(walker,cwd,path,strlen(path),attr,result_inode);
}
PUBLIC REF struct dentry *KCALL
fs_user_xhrdlink(struct dentry_walker *__restrict walker,
                 struct dentry *__restrict cwd, USER char const *path,
                 struct inode *__restrict dst_node) {
 /* TODO: Copy filename from userspace. */
 return fs_xhrdlink(walker,cwd,path,strlen(path),dst_node);
}
PUBLIC REF struct dentry *KCALL
fs_user_xsymlink(struct dentry_walker *__restrict walker,
                 struct dentry *__restrict cwd,
                 USER char const *__restrict path,
                 USER char const *__restrict target_text,
                 struct iattr const *__restrict result_attr,
                 REF struct inode **result_inode) {
 /* TODO: Copy filename from userspace. */
 return fs_xsymlink(walker,cwd,path,strlen(path),
                    target_text,result_attr,result_inode);
}
PUBLIC REF struct dentry *KCALL
fs_user_xrename(struct dentry_walker *__restrict walker,
                struct dentry *__restrict cwd, USER char const *path,
                struct dentry *__restrict existing_ent,
                REF struct inode **result_inode) {
 /* TODO: Copy filename from userspace. */
 return fs_xrename(walker,cwd,path,strlen(path),
                   existing_ent,result_inode);
}

DECL_END

#endif /* !GUARD_KERNEL_FS_FS_C */
