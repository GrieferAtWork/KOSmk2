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
#include <stddef.h>
#include <malloc.h>

DECL_BEGIN

#define NDEV   container_of(ino,struct netdev,n_dev.cd_device.d_node)
PRIVATE void KCALL
netdev_fini(struct inode *__restrict ino) {
 struct ipacket *next,*iter;
 /* Execute a driver-specific finalizer. */
 if (NDEV->n_fini) (*NDEV->n_fini)(NDEV);
 /* Free all packets that were never handled. */
 iter = NDEV->n_recv;
 while (iter) {
  next = iter->ip_next;
  ipacket_free(iter);
  iter = next;
 }
}
#undef NDEV

PUBLIC struct inodeops netdev_ops = {
    .ino_fini = &netdev_fini,
};

PUBLIC struct netdev *KCALL
netdev_cinit(struct netdev *self) {
 if (self) {
  chrdev_cinit(&self->n_dev);
  assert(self->n_recv == NULL);
  assert(self->n_recv_last == NULL);
  assert(self->n_recv_siz == 0);
  assert(self->n_recv_max == 0);
  assert(self->n_send_timeout == 0);
  assert(self->n_send_maxsize == 0);
  self->n_recv_max = NETDEV_DEFAULT_PACKETS_MAX;
  self->n_send_timeout = NETDEV_DEFAULT_STIMEOUT;
  atomic_rwlock_cinit(&self->n_recv_lock);
  rwlock_cinit(&self->n_send_lock);
  self->n_dev.cd_device.d_node.i_ops = &netdev_ops;
 }
 return self;
}


PUBLIC void KCALL
netdev_addpck_unlocked(struct netdev *__restrict self,
                       struct ipacket *__restrict pck) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(pck);
 CHECK_HOST_TEXT(pck->ip_data,pck->ip_size);
 assertf(pck->ip_size != 0,"Invalid package is empty");
 assert(atomic_rwlock_writing(&self->n_recv_lock));
 assert(netdev_pushok_unlocked(self,pck->ip_size));
 assert((self->n_recv != NULL) == (self->n_recv_last != NULL));
 assert((self->n_recv != NULL) == (self->n_recv_siz != 0));
 self->n_recv_siz += pck->ip_size;
 pck->ip_next = NULL;
 if (self->n_recv_last) {
  self->n_recv_last->ip_next = pck;
  self->n_recv_last = pck;
 } else {
  self->n_recv      = pck;
  self->n_recv_last = pck;
 }
}
PUBLIC struct ipacket *KCALL
netdev_poppck_unlocked(struct netdev *__restrict self) {
 struct ipacket *result;
 CHECK_HOST_DOBJ(self);
 assert(atomic_rwlock_writing(&self->n_recv_lock));
 assert((self->n_recv != NULL) == (self->n_recv_last != NULL));
 assert((self->n_recv != NULL) == (self->n_recv_siz != 0));
 result = self->n_recv;
 if (result) {
  /* Unlink this package. */
  assert((result->ip_next == NULL) == (result == self->n_recv_last));
  assert(result->ip_size >= self->n_recv_siz);
  self->n_recv = result->ip_next;
  self->n_recv_siz -= result->ip_size;
  if (!self->n_recv) self->n_recv_last = NULL;
  assert((self->n_recv != NULL) == (self->n_recv_siz != 0));
 }
 return result;
}




PUBLIC struct ipacket *KCALL
ipacket_alloc(size_t data_bytes) {
 struct ipacket *result;
 size_t alloc_size = CEIL_ALIGN(data_bytes,256);
 assertf(alloc_size != 0,"Invalid packet size");
 /* TODO: Dedicated buffering. */
 result = (struct ipacket *)malloc(offsetof(struct ipacket,ip_data)+data_bytes);
 if (result) result->ip_size = data_bytes;
 return result;
}
PUBLIC void KCALL
ipacket_free(struct ipacket *__restrict pck) {
 free(pck);
}



PUBLIC size_t KCALL
opacket_abssize(struct opacket const *__restrict self) {
 size_t result = 0;
 do result += self->op_size+self->op_tsiz;
 while ((self = self->op_wrap) != NULL);
 return result;
}

PUBLIC ssize_t KCALL
netdev_send_unlocked(struct netdev *__restrict self,
                     struct opacket *__restrict pck,
                     size_t packet_size) {
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(pck);
 assert(INODE_ISNETDEV(&self->n_dev.cd_device.d_node));
 assertf(packet_size == opacket_abssize(pck),
         "Invalid packet size (Given: %Iu; Correct: %Iu)",
         packet_size,opacket_abssize(pck));
 assert(self->n_send != NULL);
 assert(rwlock_writing(&self->n_send_lock));
 if unlikely(packet_size > self->n_send_maxsize)
    return -EMSGSIZE;
 return (*self->n_send)(self,pck,packet_size);
}

PUBLIC ssize_t KCALL
netdev_send(struct netdev *__restrict self,
            struct opacket *__restrict pck,
            size_t packet_size) {
 ssize_t result;
 result = (ssize_t)rwlock_write(&self->n_send_lock);
 if (E_ISERR(result)) return result;
 result = netdev_send_unlocked(self,pck,packet_size);
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
