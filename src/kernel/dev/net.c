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
#ifndef GUARD_KERNEL_DEV_NET_C
#define GUARD_KERNEL_DEV_NET_C 1
#define _KOS_SOURCE 1

#include <dev/net.h>
#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/minmax.h>
#include <stddef.h>
#include <malloc.h>

DECL_BEGIN

#define NDEV   container_of(ino,struct netdev,n_dev.cd_device.d_node)
PRIVATE void KCALL
netdev_fini(struct inode *__restrict ino) {
 /* Execute a driver-specific finalizer. */
 if (NDEV->n_fini) (*NDEV->n_fini)(NDEV);

}
#undef NDEV

PUBLIC struct inodeops netdev_ops = {
    .ino_fini = &netdev_fini,
};

PUBLIC struct netdev *KCALL
netdev_cinit(struct netdev *self) {
 if (self) {
  chrdev_cinit(&self->n_dev);
  assert(self->n_send_timeout == 0);
  assert(self->n_send_maxsize == 0);
  self->n_hand.nh_packet = nethand_packet;
  self->n_send_timeout   = NETDEV_DEFAULT_STIMEOUT;
  self->n_ip_mtu         = NETDEV_DEFAULT_IO_MTU;
  atomic_rwlock_cinit(&self->n_hand_lock);
  rwlock_cinit(&self->n_send_lock);
  self->n_dev.cd_device.d_node.i_ops = &netdev_ops;
 }
 return self;
}

PUBLIC void KCALL
opacket_do_enum(struct opacket const *__restrict self,
                uintptr_t *__restrict pstart,
                size_t *__restrict psize,
                op_printer printer, void *closure) {
 byte_t *data; size_t size;
 /* Print the header. */
 if (self->op_hsiz) {
  data = (byte_t *)self->op_head;
  size = self->op_hsiz;
  if (*pstart) {
   data += *pstart;
   if (*pstart >= size)
       *pstart -= size;
   else {
    size -= *pstart;
    *pstart = 0;
   }
  }
  if (size <= *psize) {
   *psize -= size;
  } else {
   size = *psize;
   *psize = 0;
  }
  if (size) (*printer)(data,size,closure);
  if (!*psize) return;
 }
 /* Recursively print the wrapped package. */
 if (self->op_wrap) {
  uintptr_t orig_size = MIN(self->op_wrap_size,*psize);
  size_t inner_size = orig_size;
  assert(self->op_wrap_size+self->op_tsiz >= *psize);
  *pstart += self->op_wrap_start;
  opacket_do_enum(self->op_wrap,pstart,&inner_size,printer,closure);
  assert(orig_size >= inner_size);
  assertf(*psize >= (orig_size-inner_size),
          "%Iu < %Iu",*psize,(orig_size-inner_size));
  assert(*pstart >= self->op_wrap_start);
  *pstart -= self->op_wrap_start;
  *psize  -= orig_size-inner_size;
 }
 assert(*pstart+*psize <= self->op_tsiz);
 /* print the tail. */
 if (self->op_tsiz) {
  data = (byte_t *)self->op_tail;
  size = self->op_tsiz;
  if (*pstart) {
   data += *pstart;
   if (*pstart >= size)
       *pstart -= size;
   else {
    size -= *pstart;
    *pstart = 0;
   }
  }
  if (size <= *psize) {
   *psize -= size;
  } else {
   size = *psize;
   *psize = 0;
  }
  if (size) (*printer)(data,size,closure);
 }
}

PUBLIC void KCALL
opacket_print(struct opacket const *__restrict self,
              op_printer printer, void *closure) {
 if (self->op_hsiz) (*printer)((byte_t *)self->op_head,self->op_hsiz,closure);
 if (self->op_wrap) {
  uintptr_t start = self->op_wrap_start;
  size_t    size  = self->op_wrap_size;
  opacket_do_enum(self->op_wrap,&start,&size,printer,closure);
 }
 if (self->op_tsiz) (*printer)((byte_t *)self->op_tail,self->op_tsiz,closure);
}


PUBLIC errno_t KCALL
netdev_send_unlocked(struct netdev *__restrict self,
                     struct opacket *__restrict pck) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(pck);
 assert(INODE_ISNETDEV(&self->n_dev.cd_device.d_node));
 assert(self->n_send != NULL);
 assert(rwlock_writing(&self->n_send_lock));
 return (*self->n_send)(self,pck);
}

PUBLIC errno_t KCALL
netdev_send(struct netdev *__restrict self,
            struct opacket *__restrict pck) {
 errno_t result;
 result = rwlock_write(&self->n_send_lock);
 if (E_ISERR(result)) return result;
 result = netdev_send_unlocked(self,pck);
 rwlock_endwrite(&self->n_send_lock);
 return result;
}


PRIVATE DEFINE_ATOMIC_RWLOCK(adapter_lock);
PRIVATE REF struct netdev *default_adapter = NULL;

INTERN void KCALL
delete_default_adapter(struct device *__restrict dev) {
 if (!INODE_ISNETDEV(&dev->d_node)) return;
 atomic_rwlock_read(&adapter_lock);
 if (&default_adapter->n_dev.cd_device == dev) {
  if (!atomic_rwlock_upgrade(&adapter_lock)) {
   if (&default_adapter->n_dev.cd_device != dev) {
    atomic_rwlock_endwrite(&adapter_lock);
    return;
   }
  }
  assert(&default_adapter->n_dev.cd_device == dev);
  default_adapter = NULL;
  atomic_rwlock_endwrite(&adapter_lock);
  DEVICE_DECREF(dev);
  return;
 }
 atomic_rwlock_endread(&adapter_lock);
}
PUBLIC REF struct netdev *
KCALL get_default_adapter(void) {
 REF struct netdev *result;
 atomic_rwlock_read(&adapter_lock);
 result = default_adapter;
 if (result) NETDEV_INCREF(result);
 atomic_rwlock_endread(&adapter_lock);
 return result;
}
PUBLIC bool KCALL
set_default_adapter(struct netdev *__restrict kbd,
                    bool replace_existing) {
 REF struct netdev *old_device = NULL;
 CHECK_HOST_DOBJ(kbd);
 atomic_rwlock_write(&adapter_lock);
 if (replace_existing || !default_adapter) {
  NETDEV_INCREF(kbd);
  old_device      = default_adapter;
  default_adapter = kbd;
 }
 atomic_rwlock_endwrite(&adapter_lock);
 if (old_device) NETDEV_DECREF(old_device);
 return old_device != NULL;
}


DECL_END

#endif /* !GUARD_KERNEL_DEV_NET_C */
