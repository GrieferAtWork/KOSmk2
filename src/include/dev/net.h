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
#ifndef GUARD_INCLUDE_DEV_NET_H
#define GUARD_INCLUDE_DEV_NET_H 1

#include <dev/chrdev.h>
#include <dev/rtc.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/malloc.h>

DECL_BEGIN

#ifndef __macaddr_defined
#define __macaddr_defined 1
struct PACKED macaddr { u8 ma_bytes[6]; };
#endif /* !__macaddr_defined */

#ifdef __INTELLISENSE__
#define ETHADDR(a,b,c,d,e,f) (a,b,c,d,e,f,macaddr())
#else
#define ETHADDR(a,b,c,d,e,f) (struct macaddr){{a,b,c,d,e,f}}
#endif

/* Broadcast ethernet address. */
#define ETHADDR_BROADCAST  \
        ETHADDR(0xff,0xff,0xff,0xff,0xff,0xff)

struct ipacket {
 /* Input packet data structure. */
 struct ipacket  *ip_next;    /*< [0..1][owned(ipacket_free)] Next packet. */
 size_t           ip_size;    /*< [!0] Size of the packet (in bytes) */
 byte_t           ip_data[1]; /*< Package data. */
};

/* Allocate/Free packages for net devices. */
FUNDEF struct ipacket *KCALL ipacket_alloc(size_t data_bytes);
FUNDEF void KCALL ipacket_free(struct ipacket *__restrict pck);


struct opacket {
 /* Output packet data structure. */
 void           *op_data; /*< [0..op_size] Package data/header. */
 size_t          op_size; /*< Package data/header size. */
 struct opacket *op_wrap; /*< [0..1] Wrapped (inner) package. */
 void           *op_tail; /*< [0..op_tsiz] Package tail data. */
 size_t          op_tsiz; /*< Package tail data size. */
};
#define OPACKET_INIT(p,s) {p,s,NULL,NULL,0}

/* Returns the absolute package size of the given output package. */
FUNDEF size_t KCALL opacket_abssize(struct opacket const *__restrict self);



#define NETDEV_DEFAULT_STIMEOUT      SEC_TO_JIFFIES(1)
#define NETDEV_DEFAULT_PACKETS_MAX (16*4096) /* 64K bytes */
struct netdev {
 struct chrdev   n_dev;           /*< Underlying character device. */
 struct macaddr  n_mac;           /*< MAC Address of the device. */
 u8              n_pad[2];        /* ... */
 atomic_rwlock_t n_recv_lock;     /*< Lock used to protect the incoming packet buffer. */
 struct ipacket *n_recv;          /*< [lock(n_recv_lock)][owned(ipacket_free)] Linked list of received packages (In receive order). */
 struct ipacket *n_recv_last;     /*< [lock(n_recv_lock)] The back of the linked packet list (New packages are appended here). */
 size_t          n_recv_siz;      /*< [lock(n_recv_lock)][<= n_recv_max] The total amount of data bytes (Sum of 'p_size' of all packets) currently buffered. */
 size_t          n_recv_max;      /*< [lock(n_recv_lock)] The max size of a single package supported by the adapter. */
 rwlock_t        n_send_lock;     /*< Lock that must held when sending data. */
 jtime32_t       n_send_timeout;  /*< [lock(n_send_lock)] Timeout when waiting for a package to be commit (In jiffies) */
 size_t          n_send_maxsize;  /*< [const] Max package size that can be transmitted. */

 /* NOTE: The given packet must be allocated in HOST memory!
  * @assume(rwlock_writing(&self->n_send_lock));
  * @assume(packet_size <= n_send_maxsize);
  * @assume(packet_size == opacket_abssize(packet));
  * @return: packet_size: Successfully written the packet into the buffer.
  * @return: -IO:         Data transmission failed due to an internal adapter error.
  * @return: -ETIMEDOUT:  The adapter failed to acknowledge data having been send in time.
  * @return: E_ISERR(*):  Failed to write the packet to buffer for some reason. */
 ssize_t (KCALL *n_send)(struct netdev *__restrict self,
                         struct opacket const *__restrict packet,
                         size_t packet_size); /* [1..1] */
 /* Network card finalizer. */
 void (KCALL *n_fini)(struct netdev *__restrict self); /* [0..1] */
};

#define NETDEV_TRYINCREF(self)  CHRDEV_TRYINCREF(&(self)->n_dev)
#define NETDEV_INCREF(self)     CHRDEV_INCREF(&(self)->n_dev)
#define NETDEV_DECREF(self)     CHRDEV_DECREF(&(self)->n_dev)

#define netdev_new(type_size) netdev_cinit((struct netdev *)kcalloc(type_size,GFP_SHARED))
FUNDEF struct netdev *KCALL netdev_cinit(struct netdev *self);


#define INODE_ISNETDEV(self) ((self)->i_ops == &netdev_ops)
DATDEF struct inodeops netdev_ops;


/* Check if adding a new package of 'packet_size' bytes is allowed.
 * NOTE: The caller must be holding a read-lock to 'self->n_recv_lock'
 * @return: true:  The packet may be allocated, filled and added using 'netdev_addpck_unlocked()'
 * @return: false: The packet may not be received at this time, though a small may.
 *                 The packet is empty. */
#define netdev_pushok_unlocked(self,packet_size) \
  ((self)->n_recv_siz+(packet_size) >  (self)->n_recv_siz && \
   (self)->n_recv_siz+(packet_size) <= (self)->n_recv_max)
#define netdev_pushok_atomic(self,packet_size) \
 XBLOCK({ size_t const _old_size = ATOMIC_READ((self)->n_recv_siz); \
          XRETURN _old_size+(packet_size) >  _old_size && \
                  _old_size+(packet_size) <= ATOMIC_READ((self)->n_recv_max); })

/* Add a packet 'pck' to the receiver buffer of the given network device.
 * The caller is responsible to ensure that 'netdev_pushok_unlocked()' for the packet succeeds.
 * NOTE: The caller must be holding a write-lock to 'self->n_recv_lock' */
FUNDEF void KCALL netdev_addpck_unlocked(struct netdev *__restrict self, struct ipacket *__restrict pck);

/* Pop the oldest package from the buffer of incoming data, returning
 * it or NULL when no more packages are queued for processing.
 * NOTE: The caller must be holding a write-lock to 'self->n_recv_lock' */
FUNDEF struct ipacket *KCALL netdev_poppck_unlocked(struct netdev *__restrict self);

/* Send the given package over the network.
 * @assume(rwlock_writing(&self->n_send_lock));
 * @assume(packet_size == opacket_abssize(packet));
 * @return: packet_size: Successfully written the packet into the buffer.
 * @return: -IO:         Data transmission failed due to an internal adapter error.
 * @return: -EMSGSIZE:   The package is too large to be transmit in one piece.
 * @return: -ETIMEDOUT:  The adapter failed to acknowledge data having been send in time.
 * @return: E_ISERR(*):  Failed to write the packet to buffer for some reason. */
FUNDEF ssize_t KCALL
netdev_send_unlocked(struct netdev *__restrict self,
                     struct opacket *__restrict pck,
                     size_t packet_size);
/* Same as 'netdev_send_unlocked()', but also performs locking.
 * @return: -EINTR: The calling thread was interrupted. */
FUNDEF ssize_t KCALL
netdev_send(struct netdev *__restrict self,
            struct opacket *__restrict pck,
            size_t packet_size);


/* Get/Set the default adapter device, or NULL if none is installed. */
FUNDEF REF struct netdev *KCALL get_default_adapter(void);
FUNDEF bool KCALL set_default_adapter(struct netdev *__restrict dev, bool replace_existing);


DECL_END

#endif /* !GUARD_INCLUDE_DEV_NET_H */
