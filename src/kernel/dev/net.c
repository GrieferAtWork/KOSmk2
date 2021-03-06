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

#include <hybrid/compiler.h>
#ifndef CONFIG_NO_NET
#include <dev/net.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/minmax.h>
#include <stddef.h>
#include <string.h>
#include <malloc.h>

DECL_BEGIN

#define NDEV   container_of(ino,struct netdev,n_dev.cd_device.d_node)
PRIVATE void KCALL
netdev_fini(struct inode *__restrict ino) {
 /* Execute a driver-specific finalizer. */
 if (NDEV->n_ops->n_fini) (*NDEV->n_ops->n_fini)(NDEV);

}
#undef NDEV

PUBLIC struct inodeops netdev_ops = {
    .ino_fini  = &netdev_fini,
    .ino_fopen = &inode_fopen_default,
    /* TODO: errno_t (KCALL *f_ioctl)(struct file *__restrict fp, int name, USER void *arg);
     * >> Required for ifup/ifdown functionality, etc. */
};

PUBLIC struct netdev *KCALL
netdev_cinit(struct netdev *self) {
 if (self) {
  chrdev_cinit(&self->n_dev);
  assert(self->n_send_timeout == 0);
  assert(self->n_send_maxsize == 0);
  assert(self->n_hand.nh_packet == NULL);
  assert(self->n_hand.nh_closure == NULL);
  assert(self->n_ops == NULL);
  assert(self->n_rx_total == 0);
  assert(self->n_tx_total == 0);
  /* Setup default routing and restrictions. */
  self->n_hand.nh_packet             = &nethand_packet;
  self->n_send_timeout               = NETDEV_DEFAULT_STIMEOUT;
  self->n_ip_mtu                     = NETDEV_DEFAULT_IO_MTU;
  self->n_dev.cd_device.d_node.i_ops = &netdev_ops;
  rwlock_cinit(&self->n_lock);
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


FUNDEF void KCALL
netdev_recv_unlocked(struct netdev *__restrict self,
                     void *__restrict packet,
                     size_t packet_size) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DATA(packet,packet_size);
 assert(netdev_writing(self));

 /* Track the total number of received bytes. */
 self->n_rx_total += packet_size;
 (*self->n_hand.nh_packet)(self,packet,packet_size,
                           self->n_hand.nh_closure);
}


PUBLIC errno_t KCALL
netdev_send_unlocked(struct netdev *__restrict self,
                     struct opacket *__restrict pck) {
 errno_t error; size_t packet_size;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(pck);
 assert(INODE_ISNETDEV(&self->n_dev.cd_device.d_node));
 assert(self->n_ops->n_send != NULL);
 assert(netdev_writing(self));
 if unlikely(NETDEV_ISDOWN(self))
    return -ENETDOWN;
 packet_size = OPACKET_SIZE(pck);
 if unlikely(packet_size > self->n_send_maxsize)
    return -EMSGSIZE;
 error = (*self->n_ops->n_send)(self,pck);
 /* Track the total number of bytes transmitted. */
 if (E_ISOK(error))
     self->n_tx_total += packet_size;
 return error;
}

PUBLIC errno_t KCALL
netdev_send(struct netdev *__restrict self,
            struct opacket *__restrict pck) {
 errno_t result;
 CHECK_HOST_DOBJ(self);
 result = netdev_write(self);
 if (E_ISERR(result)) return result;
 result = netdev_send_unlocked(self,pck);
 netdev_endwrite(self);
 return result;
}

PUBLIC errno_t KCALL
netdev_ifup_unlocked(struct netdev *__restrict self) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 assert(netdev_writing(self));
 assert(self->n_ops->n_ifup != NULL);
 /* Check if the device is already up. */
 if (NETDEV_ISUP(self))
     return -EALREADY;
 /* Set the interface-up flag. */
 self->n_flags |= IFF_UP;
 COMPILER_WRITE_BARRIER();
 error = (*self->n_ops->n_ifup)(self);
 /* Undo the interface-up flag if something went wrong. */
 if (E_ISERR(error))
     self->n_flags &= ~IFF_UP;
 return error;
}
PUBLIC errno_t KCALL
netdev_ifdown_unlocked(struct netdev *__restrict self) {
 errno_t error; u32 old_flags;
 CHECK_HOST_DOBJ(self);
 assert(netdev_writing(self));
 assert(self->n_ops->n_ifdown != NULL);
 /* Check if the device is already down. */
 if (NETDEV_ISDOWN(self))
     return -EALREADY;
 /* Clear the interface-up flag. */
 old_flags = self->n_flags;
 self->n_flags &= ~(IFF_UP|IFF_RUNNING);
 COMPILER_WRITE_BARRIER();
 error = (*self->n_ops->n_ifdown)(self);
 /* Restore the interface-up flag if something went wrong. */
 if (E_ISERR(error))
     self->n_flags |= old_flags&(IFF_UP|IFF_RUNNING);
 return error;
}

PUBLIC errno_t KCALL
netdev_setmac_unlocked(struct netdev *__restrict self,
                       struct macaddr const *__restrict addr) {
 struct macaddr old_addr;
 errno_t error = -EOK; u32 old_flags;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(addr);
 assert(netdev_writing(self));
 if (!self->n_ops->n_setmac)
      return -EPERM;
 /* Check if changes are required to-be performed. */
 if (memcmp(&self->n_macaddr,addr,sizeof(struct macaddr)) == 0)
     return -EOK;
 memcpy(&old_addr,&self->n_macaddr,sizeof(struct macaddr));
 old_flags = self->n_flags;
 memcpy(&self->n_macaddr,addr,sizeof(struct macaddr));
 self->n_flags |= IFF_USERMAC;
 if (NETDEV_ISUP(self)) {
  error = (*self->n_ops->n_setmac)(self);
  /* Restore the old mac address on error. */
  if (E_ISERR(error))
      memcpy(&self->n_macaddr,&old_addr,sizeof(struct macaddr)),
      self->n_flags = (self->n_flags&~IFF_USERMAC)|
                      (old_flags&IFF_USERMAC);
 }
 return error;
}
PUBLIC errno_t KCALL
netdev_resetmac_unlocked(struct netdev *__restrict self) {
 errno_t error = -EOK;
 struct macaddr old_mac;
 CHECK_HOST_DOBJ(self);
 /* If no user-mac is being used, nothing needs to be done. */
 if (!NETDEV_ISUSERMAC(self)) return -EOK;
 if (!self->n_ops->n_resetmac) return -EPERM;
 /* NOTE: Also store the current address, in case a failed
  *       reset would leave it in an undefined state. */
 memcpy(&old_mac,&self->n_macaddr,sizeof(struct macaddr));
 self->n_flags &= ~IFF_USERMAC;
 error = (*self->n_ops->n_resetmac)(self);
 /* Restore old properties on error. */
 if (E_ISERR(error))
     self->n_flags |= IFF_USERMAC,
     memcpy(&self->n_macaddr,&old_mac,sizeof(struct macaddr));
 return error;
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
set_default_adapter(struct netdev *__restrict dev,
                    bool replace_existing) {
 REF struct netdev *old_device = NULL;
 CHECK_HOST_DOBJ(dev);
 atomic_rwlock_write(&adapter_lock);
 if (replace_existing || !default_adapter) {
  NETDEV_INCREF(dev);
  old_device      = default_adapter;
  default_adapter = dev;
 }
 atomic_rwlock_endwrite(&adapter_lock);
 if (old_device) NETDEV_DECREF(old_device);
 return old_device != NULL;
}

DECL_END
#endif /* !CONFIG_NO_NET */

#endif /* !GUARD_KERNEL_DEV_NET_C */
