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
#ifndef GUARD_KERNEL_DEV_DISKPART_C
#define GUARD_KERNEL_DEV_DISKPART_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <dev/blkdev.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/traceback.h>
#include <kernel/malloc.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <malloc.h>
#include <sched/task.h>
#include <sync/rwlock.h>

DECL_BEGIN

#define DP ((struct diskpart *)self)
PUBLIC ssize_t KCALL
diskpart_read(struct blkdev *__restrict self, blkaddr_t block,
              USER void *buf, size_t n_blocks) {
 if (block < DP->dp_device.bd_blockcount)
      n_blocks = MIN(n_blocks,DP->dp_device.bd_blockcount-block);
 else n_blocks = 0;
 return blkdev_raw_read(DP->dp_ref,block+DP->dp_start,buf,n_blocks);
}
INTERN ssize_t KCALL
diskpart_write(struct blkdev *__restrict self, blkaddr_t block,
               USER void const *buf, size_t n_blocks) {
 if (block < DP->dp_device.bd_blockcount)
      n_blocks = MIN(n_blocks,DP->dp_device.bd_blockcount-block);
 else n_blocks = 0;
 return blkdev_raw_write(DP->dp_ref,block+DP->dp_start,buf,n_blocks);
}



FUNDEF SAFE REF struct diskpart *KCALL
blkdev_mkpart(struct blkdev *__restrict self,
              blkaddr_t start, blkaddr_t size,
              blksys_t sysid, size_t partid) {
 REF struct diskpart *result; errno_t error;
 dev_t partno; struct diskpart **ppart,*part;
 result = (REF struct diskpart *)blkdev_new(sizeof(struct diskpart));
 if unlikely(!result) return E_PTR(-ENOMEM);
 if (start+size < start) size = 0-start;
 for (;;) {
  /* Clamp start & size by the underlying disk's parameters. */
  if (start >= self->bd_blockcount) size = 0;
  else size = MIN(size,self->bd_blockcount-start);

#ifdef CONFIG_DEBUG
  /* Assert that the given block-device is properly registered. */
  { REF struct blkdev *reg_dev;
    reg_dev = BLKDEV_LOOKUP(self->bd_device.d_id);
    assertf(self == reg_dev,"The given block-device %[dev_t] isn't registered (%p != %p)",
            self->bd_device.d_id,self,reg_dev);
    BLKDEV_DECREF(reg_dev);
  }
#endif

  /* Optimize recursive partitions. */
  if (!BLKDEV_ISPART(self)) break;
  part = (struct diskpart *)self;
  assert(part->dp_ref->bd_blocksize == self->bd_blocksize);

  /* Check for overflow when adding the sub-partition's offset. */
  if (start+part->dp_start < start) size = 0;
  start += part->dp_start;

  self = part->dp_ref;
 }

 if unlikely(!size) start = 0;
 result->dp_device.bd_read       = &diskpart_read;
 result->dp_device.bd_write      = &diskpart_write;
 result->dp_device.bd_blocksize  = self->bd_blocksize;
 result->dp_device.bd_blockcount = size;
 result->dp_device.bd_system     = sysid;
 result->dp_start                = start;
 result->dp_ref                  = self;
 BLKDEV_INCREF(self);
 /* Setup the block device for use.
  * NOTE: Use the same module as the underlying block-device as owner. */
 error = device_setup(&result->dp_device.bd_device,
                       self->bd_device.d_node.i_owner);
 if (E_ISERR(error)) goto err2;

 /* Add the device as a sub-partition of 'self'. */
 atomic_rwlock_write(&self->bd_partlock);
 ppart = &self->bd_partitions;
 while ((part = *ppart) != NULL &&
        (assert(part->dp_ref == self),
         DISKPART_ID(part) < partid))
         assert(part->dp_chain.le_pself == ppart),
         ppart = &part->dp_chain.le_next;
 /* Insert the new partition into the chain within the associated block-device. */
 result->dp_chain.le_pself = ppart;
 result->dp_chain.le_next  = part;
 if (part) part->dp_chain.le_pself = &result->dp_chain.le_next;
 *ppart = result;
 atomic_rwlock_endwrite(&self->bd_partlock);

 /* Install the partition device in the block-device namespace. */
 partno = self->bd_device.d_id+partid+1;
 error  = devns_insert(&ns_blkdev,&result->dp_device.bd_device,partno);
 if (E_ISERR(error)) goto err;

 return result;
err:
 BLKDEV_DECREF(&result->dp_device);
 return E_PTR(error);
err2:
 BLKDEV_DECREF(self);
 free(result);
 return E_PTR(error);
}

PUBLIC SAFE REF struct diskpart *KCALL
blkdev_find_partition(struct blkdev *__restrict self,
                      blksys_t mask, blksys_t type) {
 REF struct diskpart *result;
 CHECK_HOST_DOBJ(self);
 atomic_rwlock_read(&self->bd_partlock);
 BLKDEV_FOREACH_PARTITION(result,self) {
  if ((result->dp_device.bd_system&mask) == type &&
       DISKPART_TRYINCREF(result)) break;
 }
 atomic_rwlock_endread(&self->bd_partlock);
 return result;
}






PRIVATE DEFINE_RWLOCK(autopart_lock);
/* [0..1][lock(autopart_lock)] Chain of autopart callbacks for any situation. */
PRIVATE SLIST_HEAD(struct autopart) autopart_any = NULL;
/* [0..1][lock(autopart_lock)] Chain of autopart callbacks for specific system ids. */
PRIVATE SLIST_HEAD(struct autopart) autopart_sys = NULL;


INTERN ATTR_COLDTEXT void KCALL
autopart_delete_from_instance(struct instance *__restrict inst) {
 struct autopart **piter,*iter; int i;
 task_nointr();
 rwlock_write(&autopart_lock);
 for (i = 0; i < 2; ++i) {
  piter = i ? &autopart_any : &autopart_sys;
  while ((iter = *piter) != NULL) {
   if (iter->ap_owner == inst) {
    *piter = iter->ap_chain.le_next;
    INSTANCE_WEAK_DECREF(inst);
   } else {
    piter = &iter->ap_chain.le_next;
   }
  }
 }
 rwlock_endwrite(&autopart_lock);
 task_endnointr();
}


PUBLIC SAFE ssize_t KCALL
blkdev_autopart(struct blkdev *__restrict self,
                size_t max_parts) {
 struct autopart *ap; blksys_t sysid;
 ssize_t result = 0; CHECK_HOST_DOBJ(self);
 sysid = BLKSYS_GET(self->bd_system);
 assert(ATOMIC_READ(self->bd_device.d_node.i_refcnt) >= 1);
 if unlikely(!max_parts) return 0;

 result = rwlock_read(&autopart_lock);
 if (E_ISERR(result)) return result;
 if (sysid != BLKSYS_ANY) {
  /* Execute partition-specific loaders. */
  LIST_FOREACH(ap,autopart_sys,ap_chain) {
   if (BLKSYS_GET(ap->ap_sysid) != sysid) continue;
   if (INSTANCE_LOCKWEAK(ap->ap_owner)) {
    CHECK_HOST_TEXT(ap->ap_callback,1);
    result = (*ap->ap_callback)(self,max_parts,ap->ap_closure);
    INSTANCE_DECREF(ap->ap_owner);
    if (result != -EINVAL) goto done;
    result = 0;
   }
  }
 }
 /* Execute generic loaders. */
 LIST_FOREACH(ap,autopart_any,ap_chain) {
  if (INSTANCE_LOCKWEAK(ap->ap_owner)) {
   CHECK_HOST_TEXT(ap->ap_callback,1);
   result = (*ap->ap_callback)(self,max_parts,ap->ap_closure);
   INSTANCE_DECREF(ap->ap_owner);
   if (result != -EINVAL) goto done;
   result = 0;
  }
 }
done:
 rwlock_endread(&autopart_lock);
 if (E_ISERR(result)) {
  syslog(LOG_HW|LOG_ERROR,
          "[PART] Failed to partition block device %[dev_t] (sysid %#.2I8x): %[errno]\n",
          self->bd_device.d_id,(u8)sysid,(errno_t)-result);
 } else {
  syslog(LOG_HW|LOG_CONFIRM,
          "[PART] Created %Iu/%Iu partition devices for block device %[dev_t] (sysid %#.2I8x)\n",
         (size_t)result,max_parts,self->bd_device.d_id,(u8)sysid);
 }
 return result;
}


PUBLIC SAFE void KCALL blkdev_addautopart(struct autopart *__restrict self) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->ap_owner);
 CHECK_HOST_TEXT(self->ap_callback,1);
 task_nointr();
 rwlock_write(&autopart_lock);
 /* Silently ignore requests from instances while they're unloading. */
 if likely(!INSTANCE_ISUNLOADING(self->ap_owner)) {
  INSTANCE_WEAK_INCREF(self->ap_owner);
  if (BLKSYS_GET(self->ap_sysid) == BLKSYS_ANY)
       SLIST_INSERT(autopart_any,self,ap_chain);
  else SLIST_INSERT(autopart_sys,self,ap_chain);
 }
 rwlock_endwrite(&autopart_lock);
 task_endnointr();
}
PUBLIC SAFE bool KCALL blkdev_delautopart(struct autopart *__restrict self) {
 bool result = false;
 struct autopart **piter,*iter;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->ap_owner);
 CHECK_HOST_TEXT(self->ap_callback,1);
 task_nointr();
 rwlock_write(&autopart_lock);
 piter = (BLKSYS_GET(self->ap_sysid) == BLKSYS_ANY) ? &autopart_any : &autopart_sys;
 while ((iter = *piter) != NULL) {
  if (iter == self) { *piter = iter->ap_chain.le_next; result = true; break; }
  piter = &iter->ap_chain.le_next;
 }
 rwlock_endwrite(&autopart_lock);
 task_endnointr();
 if (result) INSTANCE_WEAK_DECREF(self->ap_owner);
 return result;
}

DECL_END

#endif /* !GUARD_KERNEL_DEV_DISKPART_C */
