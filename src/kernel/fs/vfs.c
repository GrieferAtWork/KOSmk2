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
#ifndef GUARD_KERNEL_FS_VFS_C
#define GUARD_KERNEL_FS_VFS_C 1
#define _KOS_SOURCE 2

#define idata    vnode_common

#include <assert.h>
#include <dev/device.h>
#include <dirent.h>
#include <fcntl.h>
#include <fs/basic_types.h>
#include <fs/dentry.h>
#include <fs/file.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <fs/vfs.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/types.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <malloc.h>
#include <unistd.h>

DECL_BEGIN

#define SELF ((struct vfile_dir *)fp)
#define DATA ((struct vnode_data *)fp->f_node->i_data)
PRIVATE off_t KCALL
vfile_dir_seek(struct file *__restrict fp,
               off_t off, int whence) {
 off_t new_pos;
 switch (whence) {
 case SEEK_SET: new_pos = off; break;
 case SEEK_CUR: new_pos = (off_t)SELF->vf_didx+off; break;
 {
  struct vnode_data *data;
 case SEEK_END:
  data = DATA;
  atomic_rwlock_read(&data->v_lock);
  new_pos = (off_t)data->v_entc-off;
  atomic_rwlock_endread(&data->v_lock);
 } break;

 default:
  return -EINVAL;
 }
 if (new_pos < 0
#if __SIZEOF_SIZE_T__ < __SIZEOF_OFF_T__
 || (new_pos > (off_t)(size_t)-1)
#endif
     )
     return -EINVAL;
 SELF->vf_didx = (size_t)new_pos;
 return new_pos;
}
PRIVATE ssize_t KCALL
vfile_dir_readdir(struct file *__restrict fp,
                  USER struct dirent *buf,
                  size_t bufsize, rdmode_t mode) {
 ssize_t result = 0;
 struct vnode_data *data = DATA;
 struct vnode_dirent entry;
 struct dirent ent;
 REF struct inode *parent_inode = NULL;
 atomic_rwlock_read(&data->v_lock);
 if (SELF->vf_didx < 2) {
  /* Special directory entries for '.' and '..' */
  entry.d_node = fp->f_node;
  if (SELF->vf_didx == 0) {
   entry.d_name.dn_name = ".";
   entry.d_name.dn_size = 1;
  } else {
   struct dentry *parent = fp->f_dent->d_parent;
   entry.d_name.dn_name = "..";
   entry.d_name.dn_size = 2;
   if (parent &&
      (parent_inode = dentry_inode(parent)) != NULL &&
       parent_inode->i_super == entry.d_node->i_super)
       entry.d_node = parent_inode;
  }
  goto copy_entry;
 } else if (SELF->vf_didx-2 < data->v_entc) {
  entry = data->v_entv[SELF->vf_didx-2];
copy_entry:
  ent.d_ino    = entry.d_node->i_ino;
  ent.d_type   = IFTODT(entry.d_node->i_attr.ia_mode);
  ent.d_namlen = entry.d_name.dn_size;
  result       = offsetof(struct dirent,d_name)+(ent.d_namlen+1)*sizeof(char);
  if (bufsize < offsetof(struct dirent,d_name)) {
   if (copy_to_user(buf,&ent,bufsize))
       result = -EFAULT;
  } else {
   /* Copy the directory entry and name to the user-space buffer. */
   if (copy_to_user(buf,&ent,offsetof(struct dirent,d_name)) ||
       copy_to_user(buf->d_name,entry.d_name.dn_name,
                    MIN((entry.d_name.dn_size+1)*sizeof(char),
                         bufsize-offsetof(struct dirent,d_name))))
       result = -EFAULT;
  }
 }
 atomic_rwlock_endread(&data->v_lock);
 if (parent_inode) INODE_DECREF(parent_inode);

 /* Increment the directory index if need be. */
 if (E_ISOK(result) &&
     FILE_READDIR_SHOULDINC(mode,bufsize,result))
     ++SELF->vf_didx;
 return result;
}
#undef SELF
#undef DATA

#define DATA   container_of(ino->i_data,struct vnode_data,v_common)
LOCAL void KCALL
vsuperblock_add_node(struct vsuperblock *__restrict sb,
                     struct inode *__restrict virtual_node) {
 atomic_rwlock_write(&sb->v_vlock);
 if (!virtual_node->__i_nlink++) {
  LIST_INSERT(sb->v_chain,virtual_node,i_data->v_entry);
  INODE_INCREF(virtual_node);
 }
 atomic_rwlock_endwrite(&sb->v_vlock);
}
LOCAL void KCALL
vsuperblock_del_node(struct vsuperblock *__restrict sb,
                     struct inode *__restrict virtual_node) {
 bool must_decref = false;
 atomic_rwlock_write(&sb->v_vlock);
 if (!--virtual_node->__i_nlink) {
  LIST_REMOVE(virtual_node,i_data->v_entry);
  must_decref = true;
 }
 atomic_rwlock_endwrite(&sb->v_vlock);
 if (must_decref)
     INODE_DECREF(virtual_node);
}

PRIVATE void KCALL
vnode_fini(struct inode *__restrict ino) {
 struct vnode_dirent *iter,*end;
 struct vnode_data *data = DATA;
 struct vsuperblock *sb = (struct vsuperblock *)ino->i_super;
 if (VDATA_ISDYNAMIC(data->v_common.v_flag)) {
  end = (iter = data->v_entv)+data->v_entc;
  for (; iter != end; ++iter) {
   free(iter->d_name.dn_name);
   assert(ATOMIC_READ(iter->d_node->i_nlink) != 0);
   assertef(ATOMIC_DECFETCH(iter->d_node->i_refcnt) != 0,
            "This must not destroy the INode, because the "
            "node must still be referenced by the superblock");
   /* decrement the node's link counter within the associated superblock. */
   vsuperblock_del_node(sb,iter->d_node);
  }
  free(data->v_entv);
 }
}
PRIVATE REF struct file *KCALL
vnode_fopen(struct inode *__restrict ino,
            struct dentry *__restrict node_ent,
            oflag_t oflags) {
 REF struct vfile_dir *result;
 /* Can only open virtual directory files for reading. */
 if ((oflags&O_ACCMODE) != O_RDONLY) return E_PTR(-EPERM);
 result = (REF struct vfile_dir *)file_new(sizeof(struct vfile_dir));
 if unlikely(!result) return E_PTR(-ENOMEM);
 file_setup(&result->vf_file,ino,node_ent,oflags);
 return &result->vf_file;
}
#undef DATA
#define DATA container_of(dir_node->i_data,struct vnode_data,v_common)
PRIVATE REF struct inode *KCALL
vnode_lookup(struct inode *__restrict dir_node,
             struct dentry *__restrict result_path) {
 struct vnode_data *data = DATA;
 struct vnode_dirent *iter,*end;
 REF struct inode *result = E_PTR(-ENOENT);
 atomic_rwlock_read(&data->v_lock);
 end = (iter = data->v_entv)+data->v_entc;
 for (; iter != end; ++iter) {
  if (iter->d_name.dn_hash == result_path->d_name.dn_hash &&
      iter->d_name.dn_size == result_path->d_name.dn_size &&
      memcmp(iter->d_name.dn_name,result_path->d_name.dn_name,
             result_path->d_name.dn_size*sizeof(char)) == 0) {
   /* Found it! */
   result = iter->d_node;
   INODE_INCREF(result);
   break;
  }
 }
 atomic_rwlock_endread(&data->v_lock);
 return result;
}

PRIVATE errno_t KCALL
vnode_data_make_dynamic_unlocked(struct vnode_data *__restrict self) {
 struct vnode_dirent *new_vector,*iter,*end;
 assert(atomic_rwlock_writing(&self->v_lock));
 if (VDATA_ISDYNAMIC(self->v_common.v_flag)) return -EOK;
 if unlikely(!self->v_entc) { new_vector = NULL; goto done; }
 /* Copy the base vector itself. */
 new_vector = tmalloc(struct vnode_dirent,self->v_entc);
 if unlikely(!new_vector) return -ENOMEM;
 memcpy(new_vector,self->v_entv,self->v_entc*sizeof(struct vnode_dirent));
 /* Allocate copies of all entry names and create new INode references. */
 end = (iter = new_vector)+self->v_entc;
 for (; iter != end; ++iter) {
  char *name_copy;
  name_copy = tmalloc(char,iter->d_name.dn_size+1);
  if unlikely(!name_copy) {
   while (iter-- != new_vector) {
    free(iter->d_name.dn_name);
    INODE_DECREF(iter->d_node);
   }
   free(new_vector);
   return -ENOMEM;
  }
  memcpy(name_copy,iter->d_name.dn_name,
        (iter->d_name.dn_size+1)*sizeof(char));
  iter->d_name.dn_name = name_copy;
  INODE_INCREF(iter->d_node);
 }
done:
 self->v_entv = new_vector;
#if VDATA_STATIC
 self->v_common.v_flag &= ~VDATA_STATIC;
#endif
#if VDATA_DYNAMIC
 self->v_common.v_flag |=  VDATA_DYNAMIC;
#endif
 return -EOK;
}


PRIVATE errno_t KCALL
vnode_hrdlink(struct inode *__restrict dir_node,
              struct dentry *__restrict target_ent,
              struct inode *__restrict dst_node) {
 struct vnode_data *data; errno_t error = -EOK;
 struct vnode_dirent *new_entv,*iter,*end; char *name_copy;
 assert(dir_node->i_super == dst_node->i_super);
 /* Install the given node within this virtual filesystem. */
 vsuperblock_add_node((struct vsuperblock *)dir_node->i_super,dst_node);
 data = container_of(dir_node->i_data,struct vnode_data,v_common);
 atomic_rwlock_write(&data->v_lock);
 /* Ensure that the directory doesn't already contain the given name. */
 end = (iter = data->v_entv)+data->v_entc;
 for (; iter != end; ++iter) {
  if unlikely(iter->d_name.dn_hash == target_ent->d_name.dn_hash &&
              iter->d_name.dn_size == target_ent->d_name.dn_size &&
              memcmp(iter->d_name.dn_name,target_ent->d_name.dn_name,
                     target_ent->d_name.dn_size*sizeof(char)) == 0) {
   /* The entry _does_ already exist. */
   error = -EEXIST;
   goto done;
  }
 }

 /* Allocate the directory dynamically if it was allocated statically before. */
 error = vnode_data_make_dynamic_unlocked(data);
 if (E_ISERR(error)) goto done;

 /* Allocate a directory entry. */
 new_entv = trealloc(struct vnode_dirent,data->v_entv,data->v_entc+1);
 if unlikely(!new_entv) { error = -ENOMEM; goto done; }
 data->v_entv = new_entv;
 new_entv    += data->v_entc++;
 /* Fill in the new entry. */
 memcpy(&new_entv->d_name,&target_ent->d_name,sizeof(struct dentryname));
 /* Duplicate the name string itself. */
 name_copy = tmalloc(char,new_entv->d_name.dn_size+1);
 if unlikely(!name_copy) { --data->v_entc; error = -ENOMEM; goto done; }
 memcpy(name_copy,new_entv->d_name.dn_name,
       (new_entv->d_name.dn_size+1)*sizeof(char));
 new_entv->d_name.dn_name = name_copy;

 /* Fill in the INode field of the entry with a new reference. */
 new_entv->d_node = dst_node;
 INODE_INCREF(dst_node);
 
done:
 atomic_rwlock_endwrite(&data->v_lock);
 /* Remove the node from the filesystem. */
 if (E_ISERR(error))
     vsuperblock_del_node((struct vsuperblock *)dir_node->i_super,dst_node);
 return error;
}

PRIVATE errno_t KCALL
vnode_remove(struct inode *__restrict dir_node,
             struct dentry *__restrict file_path,
             struct inode *__restrict file_node) {
 bool has_write_lock = false;
 struct inode *drop_node = file_node;
 struct vnode_dirent *iter,*end;
 struct vnode_data *data; errno_t error = -ENOENT;
 assert(dir_node->i_super == file_node->i_super);
 data = container_of(dir_node->i_data,struct vnode_data,v_common);
 atomic_rwlock_read(&data->v_lock);
search_again:
 end = (iter = data->v_entv)+data->v_entc;
 for (; iter != end; ++iter) {
  if (iter->d_name.dn_hash == file_path->d_name.dn_hash &&
      iter->d_name.dn_size == file_path->d_name.dn_size &&
      memcmp(iter->d_name.dn_name,file_path->d_name.dn_name,
             file_path->d_name.dn_size*sizeof(char)) == 0) {
   /* NOTE: We can't assume that 'iter->d_node == file_node' due to weak links. */

   /* Found the entry in question. */
   if (!has_write_lock) {
    has_write_lock = true;
    if (!atomic_rwlock_upgrade(&data->v_lock))
         goto search_again;
   }
   /* Make sure that we can modify this virtual directory. */
   *(uintptr_t *)&iter -= (uintptr_t)data->v_entv;
   error = vnode_data_make_dynamic_unlocked(data);
   if (E_ISERR(error)) goto done;
   *(uintptr_t *)&iter += (uintptr_t)data->v_entv;
   
   assert(data->v_entc);

   /* Now delete this directory entry.
    * NOTE: The INode-reference is dropped below. */
   drop_node = iter->d_node;
   free(iter->d_name.dn_name);
   end = data->v_entv+data->v_entc;
   memmove(iter,iter+1,(size_t)((end-1)-iter)*
           sizeof(struct vnode_dirent));

   /* Update the directory vector size. */
   if unlikely(!--data->v_entc) {
    free(data->v_entv);
    data->v_entv = NULL;
   } else {
    iter = trealloc(struct vnode_dirent,data->v_entv,data->v_entc);
    if (iter) data->v_entv = iter;
   }

   error = -EOK;
   goto done;
  }
 }
done:
 if (has_write_lock)
      atomic_rwlock_endwrite(&data->v_lock);
 else atomic_rwlock_endread (&data->v_lock);
 if (E_ISOK(error)) {
  /* Drop the reference previously stored within the node. */
  assert(drop_node);
  asserte(ATOMIC_FETCHDEC(drop_node->i_refcnt) >= 1);
  /* Remove one link reference from the associated INode. */
  vsuperblock_del_node((struct vsuperblock *)dir_node->i_super,
                        drop_node);
 }
 return error;
}
PRIVATE REF struct inode *KCALL
vnode_mkreg(struct inode *__restrict dir_node,
            struct dentry *__restrict path,
            struct iattr const *__restrict result_attr,
            iattrset_t mode) {
 /* TODO: Create text files? */
 if (!(mode&IATTR_EXISTS)) return E_PTR(-EROFS);
 return vnode_lookup(dir_node,path);
}


PRIVATE REF struct inode *KCALL
vnode_mknod(struct inode *__restrict dir_node,
            struct dentry *__restrict target_ent,
            struct device *__restrict dev) {
 errno_t error;
 /* NOTE: Don't create real directory entries for weak devices. */
 if (dev->d_node.i_super == dir_node->i_super &&
   !(dev->d_flags&DEVICE_FLAG_WEAKID)) {
  error = vnode_hrdlink(dir_node,target_ent,&dev->d_node);
  if (E_ISERR(error)) return E_PTR(error);
  INODE_INCREF(&dev->d_node);
  return &dev->d_node;
 } else {
  REF struct vdev *weak;
  assertf(DEVICE_ISBLK(dev) || DEVICE_ISCHR(dev),"Invalid device");
  /* Create weakly aliasing proxy node, holding a weak reference to 'dev' */
  weak = (REF struct vdev *)inode_new(sizeof(struct vdev));
  if unlikely(!weak) return E_PTR(-ENOMEM);
  weak->v_node.i_ops = &vdev_ops;
  /* NOTE: We use 'ia_mode' to store the kind of device reference (chr/dev) */
  memcpy(&weak->v_node.i_attr,&dev->d_node.i_attr,sizeof(struct iattr));
  memcpy(&weak->v_node.i_attr_disk,
         &weak->v_node.i_attr,sizeof(struct iattr));
  weak->v_node.i_data   = &weak->v_data.v_common;
  weak->v_data.v_device = DEVICE_ID(dev);
  error = inode_setup(&weak->v_node,dir_node->i_super,dir_node->i_owner);
  if (E_ISERR(error)) { free(weak); return E_PTR(error); }
  error = vnode_hrdlink(dir_node,target_ent,&weak->v_node);
  if (E_ISERR(error)) return E_PTR(error);
  INODE_INCREF(&dev->d_node);
  return &dev->d_node;
 }
}
PRIVATE REF struct inode *KCALL
vnode_symlink(struct inode *__restrict dir_node,
              struct dentry *__restrict target_ent,
              USER char const *target_text,
              struct iattr const *__restrict result_attr) {
 REF struct vlink *link; errno_t error;
 char const *target_text_end = strend_user(target_text);
 size_t target_text_size;
 if unlikely(!target_text_end) return E_PTR(-EFAULT);
 target_text_size = (size_t)(target_text_end-target_text);
 /* Make sure the link text isn't too long. */
 if unlikely(target_text_size > ATOMIC_READ(((struct vsuperblock *)dir_node->i_super)->v_lnkmax))
    return E_PTR(-EPERM);
 link = (struct vlink *)inode_new(sizeof(struct vlink));
 if unlikely(!link) return E_PTR(-ENOMEM);
 if likely(target_text_size)
      link->v_data.v_text = (char *)malloc((target_text_size+1)*sizeof(char));
 else link->v_data.v_text = NULL;
 if unlikely(!link->v_data.v_text) { free(link); return E_PTR(-ENOMEM); }
 link->v_node.i_ops = &vlink_ops;
 assert(link->v_node.i_nlink == 0);
 memcpy(&link->v_node.i_attr,result_attr,sizeof(struct iattr));
 link->v_node.i_attr.ia_mode &= ~__S_IFMT;
 link->v_node.i_attr.ia_mode |= S_IFLNK;
 memcpy(&link->v_node.i_attr_disk,&link->v_node.i_attr,sizeof(struct iattr));
 link->v_node.i_data = &link->v_data.v_common;
#if VDATA_DYNAMIC != 0
 link->v_data.v_common.v_flag = VDATA_DYNAMIC;
#endif
 link->v_data.v_size = target_text_size+1;
 assert((link->v_data.v_text != NULL) ==
        (link->v_data.v_size != 0));
 /* Clone the link name from user-space. */
 if (copy_from_user((char *)link->v_data.v_text,target_text,
                     target_text_size*sizeof(char))) {
  /* Failed to copy user-text. */
  error = -EFAULT;
err_link:
  free((char *)link->v_data.v_text);
  free(link);
  return E_PTR(error);
 }
 /* Ensure that the target text is ZERO-terminated. */
 ((char *)link->v_data.v_text)[target_text_size] = '\0';

 /* Setup the node within the same superblock as the directory. */
 error = inode_setup(&link->v_node,dir_node->i_super,dir_node->i_owner);
 if (E_ISERR(error)) goto err_link;

 /* Finally, add the node to the parent directory using a hard-link. */
 error = vnode_hrdlink(dir_node,target_ent,&link->v_node);
 if (E_ISOK(error)) return &link->v_node;
 INODE_DECREF(&link->v_node);
 return E_PTR(error);
}
PRIVATE REF struct inode *KCALL
vnode_mkdir(struct inode *__restrict dir_node,
            struct dentry *__restrict target_ent,
            struct iattr const *__restrict result_attr) {
 REF struct vnode *dir; errno_t error;
 /* Make sure the link text isn't too long. */
 dir = (struct vnode *)inode_new(sizeof(struct vnode));
 if unlikely(!dir) return E_PTR(-ENOMEM);
 dir->v_node.i_ops = &vnode_ops;
 assert(dir->v_node.i_nlink == 0);
 memcpy(&dir->v_node.i_attr,result_attr,sizeof(struct iattr));
 dir->v_node.i_attr.ia_mode &= ~__S_IFMT;
 dir->v_node.i_attr.ia_mode |= S_IFDIR;
 memcpy(&dir->v_node.i_attr_disk,&dir->v_node.i_attr,sizeof(struct iattr));
 dir->v_node.i_data = &dir->v_data.v_common;
#if VDATA_DYNAMIC != 0
 dir->v_data.v_common.v_flag = VDATA_DYNAMIC;
#endif
 dir->v_data.v_entc = 0;
 dir->v_data.v_entv = NULL;

 /* Setup the node within the same superblock as the directory. */
 error = inode_setup(&dir->v_node,dir_node->i_super,dir_node->i_owner);
 if (E_ISERR(error)) { free(dir); return E_PTR(error); }

 /* Finally, add the node to the parent directory using a hard-link. */
 error = vnode_hrdlink(dir_node,target_ent,&dir->v_node);
 if (E_ISOK(error)) return &dir->v_node;
 INODE_DECREF(&dir->v_node);
 return E_PTR(error);
}
PRIVATE REF struct inode *KCALL
vnode_rename(struct inode *__restrict dst_dir, struct dentry *__restrict dst_path,
             struct inode *__restrict src_dir, struct dentry *__restrict src_path,
             struct inode *__restrict src_node) {
 errno_t error = vnode_hrdlink(dst_dir,dst_path,src_node);
 if (E_ISOK(error)) {
  error = vnode_remove(src_dir,src_path,src_node);
  if (E_ISERR(error))
      vnode_remove(dst_dir,dst_path,src_node);
 }
 if (E_ISERR(error))
     return E_PTR(error);
 /* Supporting hard links, we can simply re-return 'src_node' */
 INODE_INCREF(src_node);
 return src_node;
}
#undef DATA



#define SELF  ((struct vlink *)ino)
PRIVATE void KCALL
vlink_fini(struct inode *__restrict ino) {
 if (VDATA_ISDYNAMIC(SELF->v_data.v_common.v_flag))
     free((void *)SELF->v_data.v_text);
}
PRIVATE ssize_t KCALL
vlink_readlink(struct inode *__restrict ino,
               USER char *__restrict buf, size_t bufsize) {
 if (bufsize > SELF->v_data.v_size)
     bufsize = SELF->v_data.v_size;
 /* Copy the link text to the provided user-space buffer. */
 if (copy_to_user(buf,SELF->v_data.v_text,bufsize))
     return -EFAULT;
 return (ssize_t)SELF->v_data.v_size;
}
#undef SELF


#define SELF ((struct vsuperblock *)sb)
PRIVATE ssize_t KCALL
vnode_data_remove_inode(struct vnode_data *__restrict self,
                        struct vsuperblock *__restrict super,
                        struct inode *__restrict node,
                        bool *__restrict is_done) {
 struct vnode_dirent *iter,*end;
 ssize_t result = 0; errno_t error;
 bool has_write_lock = false;
 atomic_rwlock_read(&self->v_lock);
search_again:
 end = (iter = self->v_entv)+self->v_entc;
 for (; iter != end;) {
  assert(iter < end);
  if (iter->d_node != node) { ++iter; continue; }
  /* found one. */
  if (!has_write_lock) {
   has_write_lock = true;
   if (!atomic_rwlock_upgrade(&self->v_lock))
        goto search_again;
  }
  
  /* Allocate the directory dynamically if it was allocated statically before. */
  *(uintptr_t *)&iter -= (uintptr_t)self->v_entv;
  error = vnode_data_make_dynamic_unlocked(self);
  if (E_ISERR(error)) { result = (ssize_t)error; goto done; }
  *(uintptr_t *)&iter += (uintptr_t)self->v_entv;
  end = self->v_entv+self->v_entc;
  free(iter->d_name.dn_name);
  memmove(iter,iter+1,(size_t)(--end-iter)*
          sizeof(struct vnode_dirent));
  asserte(ATOMIC_FETCHDEC(node->i_refcnt) != 0);
  if (!--self->v_entc) {
   free(self->v_entv);
  }
  ++result;

  /* Decrement the node's link counter.
   * NOTE: Same as 'vsuperblock_del_node()', but without the lock,
   *       which is already held by the caller (don't want a deadlock, now would we?) */
  if (!--node->__i_nlink) {
   LIST_REMOVE(node,i_data->v_entry);
   /* We can stop if this was the last link associated with the node. */
   *is_done = true;
   break;
  }
 }
done:
 if (has_write_lock)
      atomic_rwlock_endwrite(&self->v_lock);
 else atomic_rwlock_endread (&self->v_lock);
 return result;
}

PRIVATE ssize_t KCALL
vfs_remove_inode(struct superblock *__restrict sb,
                 struct inode *__restrict node) {
 bool is_done = false;
 struct inode *iter,*next; ssize_t temp,result = 0;
 if unlikely(!ATOMIC_READ(node->i_nlink)) goto end_now;
 temp = vnode_data_remove_inode(&SELF->v_data,SELF,node,&is_done);
 if (E_ISERR(temp)) return temp;
 result += temp;
 if (is_done) goto end_now;
 atomic_rwlock_write(&SELF->v_vlock);
 for (iter = SELF->v_chain; iter; ) {
  if (iter->i_ops != &vnode_ops) {
   iter = iter->i_data->v_entry.le_next;
   continue;
  }
  next = iter;
  for (;;) {
   assert(next);
   next = next->i_data->v_entry.le_next;
   if (next != node) break;
   /* Manually remove links  */
   temp = vnode_data_remove_inode(container_of(next->i_data,struct vnode_data,v_common),
                                  SELF,node,&is_done);
   if (E_ISERR(temp)) { result = (ssize_t)temp; goto end; }
   result += temp;
   if (is_done) goto end_now;
  }
  temp = vnode_data_remove_inode(container_of(iter->i_data,struct vnode_data,v_common),
                                 SELF,node,&is_done);
  if (E_ISERR(temp)) { result = (ssize_t)temp; goto end; }
  result += temp;
  if (is_done) goto end_now;
  iter = next;
 }
end: atomic_rwlock_endwrite(&SELF->v_vlock);
end_now:
 /* Drop the reference originally held by the 'v_chain' linked list. */
 if (is_done) INODE_DECREF(node);
 return result;
}

PRIVATE void KCALL
vfs_clear_directory(struct vsuperblock *__restrict sb,
                    struct vnode_data *__restrict dir) {
 struct vnode_dirent *iter,*end,*vec;
 atomic_rwlock_write(&dir->v_lock);
 if (VDATA_ISDYNAMIC(dir->v_common.v_flag)) {
  end = (iter = vec = dir->v_entv)+dir->v_entc;
 } else {
  end = iter = vec = NULL;
#if VDATA_STATIC
  dir->v_common.v_flag &= ~(VDATA_STATIC);
#endif
#if VDATA_DYNAMIC
  dir->v_common.v_flag |= VDATA_DYNAMIC;
#endif
 }
 dir->v_entc = 0;
 dir->v_entv = NULL;
 atomic_rwlock_endwrite(&dir->v_lock);
 for (; iter != end; ++iter) {
  free(iter->d_name.dn_name);
  vsuperblock_del_node(sb,iter->d_node);
  INODE_DECREF(iter->d_node);
 }
 free(vec);
}


PRIVATE void KCALL
vfs_umount(struct superblock *__restrict sb) {
 for (;;) {
  struct inode *next,*chain;
  vfs_clear_directory(SELF,&SELF->v_data);
  atomic_rwlock_write(&SELF->v_vlock);
  if ((chain = SELF->v_chain) == NULL) {
   atomic_rwlock_endwrite(&SELF->v_vlock);
   break;
  }
  SELF->v_chain = NULL;
  /* Fix the chain start self-pointer. */
  chain->i_data->v_entry.le_pself = &chain;
  atomic_rwlock_endwrite(&SELF->v_vlock);
  /* Go through and decref() all chain entries,
   * recursively clearing all directories. */
  do {
   assert(INODE_ISVNODE(chain));
   if (chain->i_ops == &vnode_ops)
       vfs_clear_directory(SELF,container_of(chain->i_data,struct vnode_data,v_common));
   next = chain->i_data->v_entry.le_next;
   INODE_DECREF(chain);
  } while ((chain = next) != NULL);
  atomic_rwlock_write(&SELF->v_vlock);
 }
}
#undef SELF

PRIVATE REF struct inode *KCALL
vdev_effective(struct inode *__restrict ino) {
 dev_t id = container_of(ino,struct vdev,v_node)->v_data.v_device;
 struct devns *ns = S_ISCHR(ino->i_attr.ia_mode) ? &ns_chrdev : &ns_blkdev;
 REF struct device *result = devns_lookup(ns,id);
 if (result) return &result->d_node;
 /* Re-return 'ino', thus allowing other operations to return '-ENODEV' */
 INODE_INCREF(ino);
 return ino;
}

PRIVATE REF struct file *KCALL vdev_fopen(struct inode *__restrict UNUSED(ino), struct dentry *__restrict UNUSED(node_ent), oflag_t UNUSED(oflags)) { return E_PTR(-ENODEV); }
PRIVATE errno_t KCALL vdev_setattr(struct inode *__restrict UNUSED(ino), iattrset_t UNUSED(changed)) { return -ENODEV; }
PRIVATE ssize_t KCALL vdev_readlink(struct inode *__restrict UNUSED(ino), USER char *__restrict UNUSED(buf), size_t UNUSED(bufsize)) { return -ENODEV; }
PRIVATE REF struct inode *KCALL vdev_lookup(struct inode *__restrict UNUSED(dir_node), struct dentry *__restrict UNUSED(result_path)) { return E_PTR(-ENODEV); }
PRIVATE errno_t KCALL vdev_hrdlink(struct inode *__restrict UNUSED(dir_node), struct dentry *__restrict UNUSED(target_ent), struct inode *__restrict UNUSED(dst_node)) { return -ENODEV; }
PRIVATE REF struct inode *KCALL vdev_symlink(struct inode *__restrict UNUSED(dir_node), struct dentry *__restrict UNUSED(target_ent), USER char const *UNUSED(target_text), struct iattr const *__restrict UNUSED(result_attr)) { return E_PTR(-ENODEV); }
PRIVATE REF struct inode *KCALL vdev_mkdir(struct inode *__restrict UNUSED(dir_node), struct dentry *__restrict UNUSED(target_ent), struct iattr const *__restrict UNUSED(result_attr)) { return E_PTR(-ENODEV); }
PRIVATE REF struct inode *KCALL vdev_mkreg(struct inode *__restrict UNUSED(dir_node), struct dentry *__restrict UNUSED(path), struct iattr const *__restrict UNUSED(result_attr), iattrset_t UNUSED(mode)) { return E_PTR(-ENODEV); }
PRIVATE REF struct inode *KCALL vdev_mknod(struct inode *__restrict UNUSED(dir_node), struct dentry *__restrict UNUSED(target), struct device *__restrict UNUSED(dev)) { return E_PTR(-ENODEV); }
PRIVATE REF struct inode *KCALL vdev_rename(struct inode *__restrict UNUSED(dst_dir), struct dentry *__restrict UNUSED(dst_path), struct inode *__restrict UNUSED(src_dir), struct dentry *__restrict UNUSED(src_path), struct inode *__restrict UNUSED(src_node)) { return E_PTR(-ENODEV); }
PRIVATE errno_t KCALL vdev_remove(struct inode *__restrict UNUSED(dir_node), struct dentry *__restrict UNUSED(file_path), struct inode *__restrict UNUSED(file_node)) { return -ENODEV; }




/* Builtin INode/Superblock operators for virtual filesystems.
 * NOTE: All vnode files are directories, as anything
 *       but requires custom implementations. */
PUBLIC struct inodeops const vnode_ops = {
    .o_tag       = (void *)&vsuperblock_ops,
    .f_seek      = &vfile_dir_seek,
    .f_readdir   = &vfile_dir_readdir,
    .ino_fini    = &vnode_fini,
    .ino_fopen   = &vnode_fopen,
    .ino_lookup  = &vnode_lookup,
    .ino_hrdlink = &vnode_hrdlink,
    .ino_symlink = &vnode_symlink,
    .ino_mkdir   = &vnode_mkdir,
    .ino_mknod   = &vnode_mknod,
    .ino_mkreg   = &vnode_mkreg,
    .ino_rename  = &vnode_rename,
    .ino_remove  = &vnode_remove,
};
PUBLIC struct inodeops const vlink_ops = {
    .o_tag        = (void *)&vsuperblock_ops,
    .ino_fini     = &vlink_fini,
    .ino_readlink = &vlink_readlink,
};
PUBLIC struct inodeops const vdev_ops = {
    .o_tag         = (void *)&vsuperblock_ops,
    .ino_effective = &vdev_effective,
    .ino_fopen     = &vdev_fopen,
    .ino_setattr   = &vdev_setattr,
    .ino_readlink  = &vdev_readlink,
    .ino_lookup    = &vdev_lookup,
    .ino_hrdlink   = &vdev_hrdlink,
    .ino_symlink   = &vdev_symlink,
    .ino_mkdir     = &vdev_mkdir,
    .ino_mkreg     = &vdev_mkreg,
    .ino_mknod     = &vdev_mknod,
    .ino_rename    = &vdev_rename,
    .ino_remove    = &vdev_remove,
};
PUBLIC struct superblockops const vsuperblock_ops = {
    .sb_remove_inode = &vfs_remove_inode,
    .sb_umount       = &vfs_umount,
};


DECL_END

#endif /* !GUARD_KERNEL_FS_VFS_C */
