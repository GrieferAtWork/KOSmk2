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
#ifndef GUARD_KERNEL_DEV_DEVICE_C
#define GUARD_KERNEL_DEV_DEVICE_C 1
#define _KOS_SOURCE 2

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#include <dev/blkdev.h>
#include <dev/chrdev.h>
#include <dev/device.h>
#include <dev/rtc.h>
#include <fs/access.h>
#include <fs/dentry.h>
#include <fs/superblock.h>
#include <fs/vfs.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/boot.h>
#include <kernel/export.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <malloc.h>
#include <sched/task.h>
#include <stddef.h>
#include <string.h>

/* Define the ABI for the address tree used by mman. */
#define ATREE(x) devns_major_##x
#define Tkey     major_t
#define T        struct devns_major
#define path     ma_node
#include <hybrid/list/atree-abi.h>

DECL_BEGIN

/* Define the global device filesystem superblock. */
PUBLIC struct vsuperblock dev_fs = VSUPERBLOCK_INIT(&dev_fs,NULL,0755,0,NULL);
PUBLIC REF struct dentry *_devfs_root __ASMNAME("devfs_root") = NULL;
PRIVATE ATTR_FREEDATA char const *devfs_path = "/dev";
DEFINE_EARLY_SETUP("devfs=",set_devfs_path) { devfs_path = arg; return true; }


INTERN ATTR_FREETEXT void KCALL devfs_mount_initialize(void) {
 struct dentry_walker walker;
 DENTRY_WALKER_SETKERNEL(walker);
 _devfs_root = fs_xmount(&walker,&fs_root,devfs_path,
                          strlen(devfs_path),&dev_fs.v_super);
 if (E_ISERR(_devfs_root)) {
  if (E_GTERR(_devfs_root) == -ENOENT) {
   /* XXX: Recursively create the directory path described by `devfs_path'? */
  }
  syslog(LOG_DEBUG,
         FREESTR("[DEVFS] Failed to mount dev-fs superblock at %q: %[errno]\n"),
         devfs_path,-E_GTERR(_devfs_root));
  _devfs_root = NULL;
 }
}

#ifdef CONFIG_DEBUG
INTERN ATTR_COLDTEXT void KCALL devfs_mount_finalize(void) {
 REF struct dentry *root;
 /* Unmount the device filesystem. */
 if ((root = ATOMIC_XCH(_devfs_root,NULL)) != NULL) {
  struct fsaccess ac;
  FSACCESS_SETHOST(ac);
  dentry_umount(root,&ac);
  DENTRY_DECREF(root);
 }
}
#endif


PUBLIC DEFINE_ATOMIC_OWNER_RWLOCK(irqctl_lock);
PRIVATE LIST_HEAD(struct device) irqctl_list = NULL;
PUBLIC void KCALL device_irqctl(unsigned int cmd) {
 struct device *dev;
 assert(atomic_owner_rwlock_reading(&irqctl_lock));
 LIST_FOREACH(dev,irqctl_list,d_irq_dev) {
  assert(dev->d_irq_ctl);
  (*dev->d_irq_ctl)(dev,cmd);
 }
}


PUBLIC struct device *KCALL
device_cinit(struct device *self) {
 if (self) {
  CHECK_HOST_DOBJ(self);
  assert(self->d_irq_ctl == NULL);
  inode_cinit(&self->d_node);
  assert(self->d_node.i_attr.ia_uid == 0);
  assert(self->d_node.i_attr.ia_gid == 0);
  self->d_node.__i_nlink     = 1;
  self->d_node.i_attr.ia_siz = 4096;
  self->d_node.i_attr_disk   = self->d_node.i_attr;
 }
 return self;
}

PUBLIC errno_t KCALL
device_setup(struct device *__restrict self,
             struct instance *__restrict owner) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(owner);
 assert(!self->d_node.i_owner);
 assertf(self->d_node.i_ops != NULL,
         "Missing INode callbacks in device at %p",
         self);
 /* Register the given module instance as owner. */
 if (!INSTANCE_INCREF(owner))
      return -EPERM;
 self->d_node.i_owner = owner;

 /* Setup the super-block pointer of the device's INode. */
 self->d_node.i_super = &dev_fs.v_super;
 INODE_INCREF(&dev_fs.v_super.sb_root);

 /* Insert the device into the DEV filesystem. */
 atomic_rwlock_write(&dev_fs.v_super.sb_nodes_lock);
 LIST_INSERT(dev_fs.v_super.sb_nodes,&self->d_node,i_nodes);
 atomic_rwlock_endwrite(&dev_fs.v_super.sb_nodes_lock);

 /* If the device implements IRQ controls, track it. */
 if (self->d_irq_ctl) {
  atomic_owner_rwlock_write(&irqctl_lock);
  LIST_INSERT(irqctl_list,self,d_irq_dev);
  atomic_owner_rwlock_endwrite(&irqctl_lock);
 }
 return -EOK;
}

PRIVATE REF struct device *KCALL
devns_delete_unlocked(struct devns *__restrict self,
                      struct device *dev, dev_t id, bool release);

PUBLIC void KCALL
device_fini(struct device *__restrict self) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->d_node.i_owner);
 assert(INODE_ISDEV(&self->d_node));
 if (self->d_irq_ctl) {
  atomic_owner_rwlock_write(&irqctl_lock);
  LIST_REMOVE(self,d_irq_dev);
  atomic_owner_rwlock_endwrite(&irqctl_lock);
 }
 if (self->d_flags&DEVICE_FLAG_WEAKID) {
  struct device *del_dev;
  /* Weak devices must delete their own device namespace entry. */
  del_dev = devns_delete_unlocked(DEVICE_ISBLK(self) ? &ns_blkdev : &ns_chrdev,
                                  self,ATOMIC_READ(self->d_id),true);
  assert(!del_dev || del_dev == self);
 }
 if (DEVICE_ISBLK(self) &&
     BLKDEV_ISPART(container_of(self,struct blkdev,bd_device))) {
  /* Unlink the partition from the chain of partition
   * within the referenced block device. */
  struct diskpart *part = container_of(self,struct diskpart,dp_device.bd_device);
  struct blkdev *part_disk = part->dp_ref;
  CHECK_HOST_DOBJ(part);
  CHECK_HOST_DOBJ(part_disk);
  atomic_rwlock_write(&part_disk->bd_partlock);
  LIST_REMOVE(part,dp_chain);
  atomic_rwlock_endwrite(&part_disk->bd_partlock);
  /* Drop a reference from the referenced block device. */
  BLKDEV_DECREF(part->dp_ref);
 }
}

PUBLIC struct devns ns_blkdev = {
    .d_lock  = ATOMIC_RWLOCK_INIT,
    .d_tree  = NULL,
    .d_sort  = NULL,
    .d_elock = RWLOCK_INIT,
    .d_event = NULL,
};
PUBLIC struct devns ns_chrdev = {
    .d_lock  = ATOMIC_RWLOCK_INIT,
    .d_tree  = NULL,
    .d_sort  = NULL,
    .d_elock = RWLOCK_INIT,
    .d_event = NULL,
};

PUBLIC void KCALL
devns_addevent(struct devns *__restrict self,
               struct devns_event *__restrict e) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(e);
 task_nointr();
 rwlock_write(&self->d_elock);
 /* Silently ignore requests from instances while they're unloading. */
 if likely(!INSTANCE_ISUNLOADING(e->de_owner)) {
  INSTANCE_WEAK_INCREF(e->de_owner);
  SLIST_INSERT(self->d_event,e,de_chain);
 }
 rwlock_endwrite(&self->d_elock);
 task_endnointr();
}
PUBLIC bool KCALL
devns_delevent(struct devns *__restrict self,
               struct devns_event *__restrict e) {
 bool result = false;
 struct devns_event **piter,*iter;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(e);
 task_nointr();
 rwlock_write(&self->d_elock);
 piter = &self->d_event;
 while ((iter = *piter) != NULL) {
  if (iter == e) {
   /* Found it! */
   *piter = iter->de_chain.le_next;
   result = true;
   break;
  }
  piter = &iter->de_chain.le_next;
 }
 rwlock_endwrite(&self->d_elock);
 task_endnointr();
 if (result) INSTANCE_WEAK_DECREF(e->de_owner);
 return result;
}

PRIVATE SAFE errno_t KCALL
devns_exec_adddev(struct devns *__restrict self,
                  struct device *__restrict dev, dev_t id) {
 struct devns_event *iter,*iter2; errno_t result;
 result = rwlock_read(&self->d_elock);
 if (E_ISERR(result)) return result;
 result = -EOK;
 SLIST_FOREACH(iter,self->d_event,de_chain) {
  if (iter->de_add && INSTANCE_LOCKWEAK(iter->de_owner)) {
   result = (*iter->de_add)(self,dev,id);
   assertef(ATOMIC_DECFETCH(iter->de_owner->i_refcnt) != 0,
            "How can this drop to ZERO(0)? - Attempting to delete the module should "
            "have required acquiring `d_elock' on this namespace during the module hook-deletion phase.");
   if (E_ISERR(result)) {
    /* Undo everything we did so far. */
    SLIST_FOREACH(iter2,self->d_event,de_chain) {
     if (iter2 == iter) break;
     if (iter2->de_del && INSTANCE_LOCKWEAK(iter->de_owner)) {
      (*iter2->de_del)(self,dev,id);
      assertef(ATOMIC_DECFETCH(iter2->de_owner->i_refcnt) != 0,
               "How can this drop to ZERO(0)? - Attempting to delete the module should "
               "have required acquiring `d_elock' on this namespace during the module hook-deletion phase.");
     }
    }
    break;
   }
  }
 }
 rwlock_endread(&self->d_elock);
 return result;
}
PRIVATE SAFE void KCALL
devns_exec_deldev(struct devns *__restrict self,
                  struct device *__restrict dev, dev_t id) {
 struct devns_event *iter;
 task_nointr();
 rwlock_read(&self->d_elock);
 SLIST_FOREACH(iter,self->d_event,de_chain) {
  if (iter->de_del && INSTANCE_LOCKWEAK(iter->de_owner)) {
   (*iter->de_del)(self,dev,id);
   assertef(ATOMIC_DECFETCH(iter->de_owner->i_refcnt) != 0,
            "How can this drop to ZERO(0)? - Attempting to delete the module should "
            "have required acquiring `d_elock' on this namespace during the module hook-deletion phase.");
  }
 }
 rwlock_endread(&self->d_elock);
 task_endnointr();
}


#define BTREE_VAL(p) MINOR((p)->d_id)
LOCAL BTREE_MKHAS(devns_device_has,struct device,minor_t,d_devnode);
LOCAL BTREE_MKGET(devns_device_get,struct device,minor_t,d_devnode);
LOCAL BTREE_MKPGET(devns_device_pget,struct device,minor_t,d_devnode);
LOCAL BTREE_MKINS(devns_device_ins,struct device,minor_t,d_devnode);
LOCAL BTREE_MKDEL(devns_device_del,struct device,minor_t,d_devnode);

PRIVATE struct device *KCALL
devns_lookup_unlocked(struct devns *__restrict self, dev_t id) {
 REF struct device *result = NULL;
 struct devns_major *major;
 CHECK_HOST_DOBJ(self);
 assert(atomic_rwlock_reading(&self->d_lock));
 major = devns_major_locate(self->d_tree,MAJOR(id));
 if unlikely(!major) goto end;
 result = devns_device_get(major->ma_bvec[MAJOR(id)-DEVNS_MAJOR_MIN(major)],MINOR(id));
 assert(!result || result->d_id == id);
end:
 return result;
}


PUBLIC REF struct device *KCALL
devns_lookup(struct devns *__restrict self, dev_t id) {
 REF struct device *result;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->d_lock);
 result = devns_lookup_unlocked(self,id);
 /* NOTE: Must try incref() the device, because it may be weakly referenced. */
 if (result && !DEVICE_TRYINCREF(result))
     result = NULL;
 atomic_rwlock_endread(&self->d_lock);
 return result;
}

PRIVATE struct devns_major *KCALL
devns_alloc_major(struct devns *__restrict self, major_t id_major) {
 struct devns_major *result,**presult;
 result = devns_major_locate(self->d_tree,id_major);
 if (!result) {
  ATREE_LEVEL_T addr_level;
  ATREE_SEMI_T(major_t) addr_semi;
  /* Create, or extend a major leaf. */
  if (id_major != 0 &&
     (addr_level = ATREE_LEVEL0(major_t),
      addr_semi  = ATREE_SEMI0 (major_t),
     (presult = devns_major_plocate_at(&self->d_tree,id_major-1,
                                       &addr_semi,&addr_level)) != NULL)) {
   struct device **new_vec;
   struct devns_major **pnext,*next;
   /* Extend this major leaf above. */
   result = *presult;
   assert(result->ma_node.a_vmax+1 == id_major);
   devns_major_pop(presult);
   new_vec = trealloc(struct device *,result->ma_bvec,DEVNS_MAJOR_CNT(result)+1);
   if unlikely(!new_vec) return NULL;
   result->ma_bvec                  = new_vec;
   new_vec[DEVNS_MAJOR_CNT(result)] = NULL; /* Pre-initialize the major slot to NULL. */
   ++result->ma_node.a_vmax;
   /* Check if we can merge this slot with the following. */
   addr_level = ATREE_LEVEL0(major_t);
   addr_semi  = ATREE_SEMI0 (major_t);
   pnext      = devns_major_plocate_at(&self->d_tree,id_major+1,
                                       &addr_semi,&addr_level);
   if (pnext) {
    /* Yes: Merge it with its successor. */
    next = *pnext;
    assert(next);
    assert(next->ma_node.a_vmin == result->ma_node.a_vmax+1);
    new_vec = trealloc(struct device *,result->ma_bvec,
                       DEVNS_MAJOR_CNT(result)+
                       DEVNS_MAJOR_CNT(next));
    if likely(new_vec) {
     result->ma_bvec = new_vec;
     /* Transfer all device-minor-trees. */
     memcpy(new_vec+DEVNS_MAJOR_CNT(result),next->ma_bvec,
            DEVNS_MAJOR_CNT(next)*sizeof(struct device *));
     /* Pop the next slot. */
     devns_major_pop(pnext);
     result->ma_node.a_vmax = next->ma_node.a_vmax;
     result->ma_sort.le_next = next->ma_sort.le_next;
     if (result->ma_sort.le_next)
         result->ma_sort.le_next->ma_sort.le_pself = &result->ma_sort.le_next;
     /* Free controller of the next slot (who's contents we've inherited). */
     free(next->ma_bvec);
     free(next);
    }
   }
   /* Re-insert the leaf. */
   devns_major_insert(&self->d_tree,result);
   asserte(devns_major_locate(self->d_tree,id_major) == result);
  } else if (id_major != MAJORMASK &&
            (addr_level = ATREE_LEVEL0(major_t),
             addr_semi  = ATREE_SEMI0 (major_t),
            (presult = devns_major_plocate_at(&self->d_tree,id_major+1,
                                              &addr_semi,&addr_level)) != NULL)) {
   struct device **new_vec;
   /* Extend this major leaf below. */
   result = *presult;
   assert(result->ma_node.a_vmin-1 == id_major);
   devns_major_pop(presult);
   new_vec = trealloc(struct device *,result->ma_bvec,DEVNS_MAJOR_CNT(result)+1);
   if unlikely(!new_vec) return NULL;
   memmove(new_vec+1,new_vec,DEVNS_MAJOR_CNT(result)*sizeof(struct device *));
   --result->ma_node.a_vmin;
   result->ma_bvec = new_vec;
   new_vec[0]      = NULL; /* Pre-initialize the major slot to NULL. */
   /* Re-insert the leaf. */
   devns_major_insert(&self->d_tree,result);
   asserte(devns_major_locate(self->d_tree,id_major-1) == NULL);
   asserte(devns_major_locate(self->d_tree,id_major)   == result);
  } else {
   struct devns_major **psort,*sort;
   /* Create a new major leaf. */
   result = omalloc(struct devns_major);
   if unlikely(!result) return NULL;
   result->ma_node.a_vmin = id_major;
   result->ma_node.a_vmax = id_major;
   result->ma_bvec        = tcalloc(struct device *,1);
   if unlikely(!result->ma_bvec) { free(result); return NULL; }
   /* Insert the new leaf. */
   devns_major_insert(&self->d_tree,result);
   psort = &self->d_sort;
   while ((sort = *psort) != NULL &&
           DEVNS_MAJOR_MAX(sort) < id_major)
           psort = &sort->ma_sort.le_next;
   /* Insert the leaf into the sorted chain of them. */
   result->ma_sort.le_pself = psort;
   result->ma_sort.le_next = sort;
   if (sort) sort->ma_sort.le_pself = &result->ma_sort.le_next;
   *psort = result;
  }
 }
 return result;
}


PRIVATE errno_t KCALL
devns_insert_unlocked(struct devns *__restrict self,
                      struct device *__restrict dev,
                      dev_t id) {
 struct devns_major *major;
 struct device **pminor_tree;
 assert(DEVICE_ISBLK(dev) ? self == &ns_blkdev : 1);
 assert(DEVICE_ISCHR(dev) ? self == &ns_chrdev : 1);
 major = devns_alloc_major(self,MAJOR(id));
 if unlikely(!major) return -ENOMEM;
 pminor_tree = &major->ma_bvec[MAJOR(id)-DEVNS_MAJOR_MIN(major)];
 /* Check if the device already exists. */
 if (devns_device_has(*pminor_tree,MINOR(id))) return -EEXIST;
 /* Now just insert the device into the tree! */
 ATOMIC_WRITE(dev->d_id,id);
 devns_device_ins(pminor_tree,dev);
 return -EOK;
}

#define DEVNS_DELETE_ANY  NULL
PRIVATE REF struct device *KCALL
devns_delete_unlocked(struct devns *__restrict self,
                      struct device *dev, dev_t id, bool release) {
 ATREE_LEVEL_T addr_level = ATREE_LEVEL0(major_t);
 ATREE_SEMI_T(major_t) addr_semi = ATREE_SEMI0(major_t);
 struct devns_major *major,**pmajor,*upper_major;
 struct device **pdev_tree,**pdevice;
 struct device *result;
 CHECK_HOST_DOBJ(self);
 pmajor = devns_major_plocate_at(&self->d_tree,MAJOR(id),
                                 &addr_semi,&addr_level);
 if unlikely(!pmajor) return NULL;
 major = *pmajor;
 CHECK_HOST_DOBJ(major);
 /* Scan the minor-tree. */
 pdev_tree = &major->ma_bvec[MAJOR(id)-DEVNS_MAJOR_MIN(major)];
 pdevice   = devns_device_pget(pdev_tree,MINOR(id));
 if unlikely(!pdevice) return NULL;
 result = *pdevice;
 if (dev != DEVNS_DELETE_ANY && result != dev) return NULL;
 assertf(result->d_id == id,
         "Incorrectly linked device %[dev_t] != %[dev_t]",
         result->d_id,id);
 devns_device_del(pdevice);
 if (!*pdev_tree && release) {
  /* This was the last minor device for the given major number.
   * >> And since we're supposed to release the major number now, do so! */
  devns_major_pop(pmajor);
  if (MAJOR(id) == DEVNS_MAJOR_MIN(major)) {
   if (MAJOR(id) == DEVNS_MAJOR_MAX(major)) {
    /* Delete the major node. */
    LIST_REMOVE(major,ma_sort);
    free(major->ma_bvec);
    free(major);
   } else {
    /* Trim the major node at its bottom. */
    ++major->ma_node.a_vmin;
    memmove(major->ma_bvec,major->ma_bvec+1,
            DEVNS_MAJOR_CNT(major)*sizeof(struct device *));
    pdev_tree = trealloc(struct device *,major->ma_bvec,DEVNS_MAJOR_CNT(major));
    if (pdev_tree) major->ma_bvec = pdev_tree;
    /* re-insert the major node. */
    devns_major_insert(&self->d_tree,major);
   }
  } else if (MAJOR(id) == DEVNS_MAJOR_MAX(major)) {
   /* Trim the major node at its top. */
   --major->ma_node.a_vmax;
   pdev_tree = trealloc(struct device *,major->ma_bvec,DEVNS_MAJOR_CNT(major));
   if (pdev_tree) major->ma_bvec = pdev_tree;
   devns_major_insert(&self->d_tree,major);
  } else {
   /* Split the major node in two, leaving out the part originally used by `id'. */
   upper_major = omalloc(struct devns_major);
   if unlikely(!upper_major) goto err_upper_fail;
   upper_major->ma_node.a_vmin = MAJOR(id)+1;
   upper_major->ma_node.a_vmax = major->ma_node.a_vmax;
   major->ma_node.a_vmax       = MAJOR(id)-1;
   upper_major->ma_bvec        = (struct device **)memdup(major->ma_bvec+DEVNS_MAJOR_CNT(major),
                                                          DEVNS_MAJOR_CNT(upper_major)*
                                                          sizeof(struct device *));
   if unlikely(!upper_major->ma_bvec) {
    major->ma_node.a_vmax = upper_major->ma_node.a_vmax;
    free(upper_major);
    goto err_upper_fail;
   }
   pdev_tree = trealloc(struct device *,major->ma_bvec,DEVNS_MAJOR_CNT(major));
   if (pdev_tree) major->ma_bvec = pdev_tree;

   /* Connect the lower & upper major nodes. */
   upper_major->ma_sort.le_next  = major->ma_sort.le_next;
   if (upper_major->ma_sort.le_next)
       upper_major->ma_sort.le_next->ma_sort.le_pself = &upper_major->ma_sort.le_next;
   upper_major->ma_sort.le_pself = &major->ma_sort.le_next;
   major->ma_sort.le_next        = upper_major;

   /* Insert the second-half upper major node. */
   devns_major_insert(&self->d_tree,upper_major);
end_upper_fail:
   devns_major_insert(&self->d_tree,major);
  }
 }
 return result;
err_upper_fail:
 syslog(LOG_HW|LOG_INFO,"[DEV] Failed to release major device number #%d: %[errno]\n",
       (int)MAJOR(id),ENOMEM);
 goto end_upper_fail;
}

PUBLIC errno_t KCALL
devns_insert(struct devns *__restrict self,
             struct device *__restrict dev,
             dev_t id) {
 errno_t error;
 atomic_rwlock_write(&self->d_lock);
 error = devns_insert_unlocked(self,dev,id);
 atomic_rwlock_downgrade(&self->d_lock);
 if (E_ISOK(error)) {
  if (!(dev->d_flags&DEVICE_FLAG_WEAKID))
        DEVICE_INCREF(dev);
  error = devns_exec_adddev(self,dev,id);
  if (E_ISERR(error)) {
   atomic_rwlock_upgrade(&self->d_lock);
   dev = devns_delete_unlocked(self,dev,id,true);
   if (dev && dev->d_flags&DEVICE_FLAG_WEAKID) dev = NULL;
   atomic_rwlock_endwrite(&self->d_lock);
   if (dev) DEVICE_DECREF(dev);
   return error;
  }
 }
 atomic_rwlock_endread(&self->d_lock);
 return error;
}

PUBLIC bool KCALL
devns_remove(struct devns *__restrict self,
             struct device *__restrict dev,
             bool release) {
 struct device *result;
 dev_t id;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_write(&self->d_lock);
 id = ATOMIC_READ(dev->d_id);
 result = devns_delete_unlocked(self,dev,id,release);
 assert(result == NULL || result == dev);
 if (result && (result->d_flags&DEVICE_FLAG_WEAKID) &&
    !DEVICE_TRYINCREF(result)) {
  devns_exec_deldev(self,result,id);
  atomic_rwlock_endwrite(&self->d_lock);
  return true;
 }
 atomic_rwlock_endwrite(&self->d_lock);
 if (result) {
  devns_exec_deldev(self,result,id);
  DEVICE_DECREF(result);
 }
 return result != NULL;
}
PUBLIC bool KCALL
devns_erase(struct devns *__restrict self,
            dev_t id, u32 mode) {
 struct device *result;
 bool has_lock = false;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_write(&self->d_lock);
 result = devns_delete_unlocked(self,DEVNS_DELETE_ANY,id,
                                mode&DEVNS_ERASE_RELEASE);
 if (result && result->d_flags&DEVICE_FLAG_WEAKID &&
    !DEVICE_TRYINCREF(result))
 { has_lock = true; goto handle_result; }
 atomic_rwlock_endwrite(&self->d_lock);
 assert(result == NULL || result->d_id == id);
 if (result) {
handle_result:
  if (mode&DEVNS_ERASE_PARTITIONS &&
      DEVICE_ISBLK(result)) {
   struct blkdev *bdev = (struct blkdev *)result;
   struct diskpart *iter,*part = NULL; struct device *del_part;
   /* Delete sub-partitions as well. */
   for (;;) {
    atomic_rwlock_read(&bdev->bd_partlock);
    iter = bdev->bd_partitions;
    if (!part) part = iter;
    else {
     /* Validate the last part and start over upon mismatch. */
     while (iter && iter != part)
            iter = iter->dp_chain.le_next;
     if (!iter) part = bdev->bd_partitions;
     else assert(iter == part),
          part = iter->dp_chain.le_next;
    }
    if (part) BLKDEV_INCREF(&part->dp_device);
    atomic_rwlock_endread(&bdev->bd_partlock);
    if (!part) break;

    /* Delete this partition. */
    { dev_t part_id;
      atomic_rwlock_write(&self->d_lock);
      part_id  = ATOMIC_READ(part->dp_device.bd_device.d_id);
      del_part = devns_delete_unlocked(self,&part->dp_device.bd_device,
                                       part_id,mode&DEVNS_ERASE_RELEASE);
      atomic_rwlock_endwrite(&self->d_lock);
      /* NOTE: `del_part' may potentially be NULL due to race conditions. */
      assert(del_part == NULL ||
             del_part == &part->dp_device.bd_device);
      if (del_part && !(part->dp_device.bd_device.d_flags&DEVICE_FLAG_WEAKID))
          DEVICE_DECREF(del_part);
      devns_exec_deldev(self,&part->dp_device.bd_device,part_id);
      BLKDEV_DECREF(&part->dp_device);
    }
   }
  }
  devns_exec_deldev(self,result,id);
  DEVICE_DECREF(result);
  if (has_lock)
      atomic_rwlock_endwrite(&self->d_lock);
 }
 return result != NULL;
}





PRIVATE ATTR_COLDTEXT REF struct device *KCALL
device_delete_from_instance(struct devns *__restrict ns,
                            struct device *__restrict self,
                            struct instance *__restrict inst) {
 REF struct device *result;
 CHECK_HOST_DOBJ(self);
again:
 if (self->d_node.i_owner == inst) {
  result = devns_delete_unlocked(ns,self,DEVICE_ID(self),true);
  asserte(self == result);
  return result;
 }
 /* Recursively delete all devices registered using the given module instance. */
 if (self->d_devnode.bt_min) {
  if (self->d_devnode.bt_max) {
   result = device_delete_from_instance(ns,self->d_devnode.bt_max,inst);
   if (result) return result;
   goto again;
  }
  self = self->d_devnode.bt_min;
  goto again;
 }
 if (self->d_devnode.bt_max) {
  self = self->d_devnode.bt_max;
  goto again;
 }
 return NULL;
}
PRIVATE ATTR_COLDTEXT REF struct device *KCALL
devns_major_delete_from_instance(struct devns *__restrict ns,
                                 struct devns_major *__restrict self,
                                 struct instance *__restrict inst) {
 REF struct device *result;
 struct device **iter,**end,*dev;
 CHECK_HOST_DOBJ(self);
again:
 /* Recursively delete all devices registered using the given module instance. */
 end = (iter = DEVNS_MAJOR_VEC(self))+DEVNS_MAJOR_CNT(self);
 for (; iter != end; ++iter) {
  if ((dev = *iter) != NULL &&
      (result = device_delete_from_instance(ns,dev,inst)) != NULL)
       return result;
 }
 if (self->ma_node.a_min) {
  if (self->ma_node.a_max) {
   result = devns_major_delete_from_instance(ns,self->ma_node.a_max,inst);
   if (result) return result;
  }
  self = self->ma_node.a_min;
  goto again;
 }
 if (self->ma_node.a_max) {
  self = self->ma_node.a_max;
  goto again;
 }
 return NULL;
}

INTDEF void KCALL delete_boot_device(struct device *__restrict dev);
INTDEF void KCALL delete_default_keyboard(struct device *__restrict dev);
INTDEF void KCALL delete_default_adapter(struct device *__restrict dev);
INTERN void KCALL delete_default_rtc(struct rtc *__restrict dev);

INTERN void KCALL
devns_delete_from_instance(struct devns *__restrict self,
                           struct instance *__restrict inst) {
 REF struct device *dev;
 CHECK_HOST_DOBJ(self);
 for (;;) {
  dev_t id; bool has_lock = false;
  atomic_rwlock_write(&self->d_lock);
  dev = self->d_tree ? devns_major_delete_from_instance(self,self->d_tree,inst) : NULL;
  if (dev) {
   id = DEVICE_ID(dev);
   if (dev->d_flags&DEVICE_FLAG_WEAKID &&
      !DEVICE_TRYINCREF(dev))
   { has_lock = true; goto handle_dev; }
  }
  atomic_rwlock_endwrite(&self->d_lock);
  if (!dev) break;
handle_dev:
  devns_exec_deldev(self,dev,id);
  /* Check for deleting special, global device hooks. */
  delete_boot_device(dev);
  delete_default_keyboard(dev);
  delete_default_adapter(dev);
  delete_default_rtc(container_of(dev,struct rtc,r_dev.cd_device));
  /* Remove any mapping of the device from `/dev' */
  superblock_remove_inode(&dev_fs.v_super,&dev->d_node);
  DEVICE_DECREF(dev);
  if (has_lock) atomic_rwlock_endwrite(&self->d_lock);
 }
 /* Delete event callbacks registered under the given instance. */
 { struct devns_event **piter,*iter;
   bool has_write_lock = false;
   task_nointr();
   rwlock_read(&self->d_elock);
event_again:
   piter = &self->d_event;
   while ((iter = *piter) != NULL) {
    if (iter->de_owner != inst) goto next;
    if (!has_write_lock) {
     has_write_lock = true;
     if (rwlock_upgrade(&self->d_elock) == -ERELOAD)
         goto event_again;
    }
    /* NOTE: Decrementing weak reference counters while holding
     *       global locks is ok, because weak destructors literally
     *       should only free any resiliently allocated data. */
    INSTANCE_WEAK_DECREF(iter->de_owner);
    *piter = iter->de_chain.le_next;
    continue;
next:
    piter = &iter->de_chain.le_next;
   }
   if (has_write_lock)
        rwlock_endwrite(&self->d_elock);
   else rwlock_endread (&self->d_elock);
   task_endnointr();
 }
}



PUBLIC major_t KCALL
devns_reserve(struct devns *__restrict self, size_t n_major) {
 CHECK_HOST_DOBJ(self);
 (void)self;
 (void)n_major;
 /* TODO */
 return DEVNS_RESERVE_ERROR;
}
PUBLIC major_t KCALL
devns_reserve_at(struct devns *__restrict self, major_t start, size_t n_major) {
 CHECK_HOST_DOBJ(self);
 (void)self;
 (void)start;
 (void)n_major;
 /* TODO */
 return DEVNS_RESERVE_ERROR;
}
PUBLIC void KCALL
devns_release(struct devns *__restrict self, major_t start, size_t n_major) {
 CHECK_HOST_DOBJ(self);
 (void)self;
 (void)start;
 (void)n_major;
 /* TODO */
}

DECL_END

#pragma GCC diagnostic pop

#endif /* !GUARD_KERNEL_DEV_DEVICE_C */
