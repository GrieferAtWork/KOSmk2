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

#include <hybrid/compiler.h>
#ifndef CONFIG_NO_NET
#include <dev/chrdev.h>
#include <dev/rtc.h>
#include <net/if.h>
#include <hybrid/types.h>
#include <kernel/malloc.h>
//#include <bits/socket.h>

DECL_BEGIN

struct netdev;

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

struct opacket {
 /* Output packet data structure. */
 void           *op_head;       /*< [0..op_hsiz] Package data/header. */
 size_t          op_hsiz;       /*< Package data/header size. */
 struct opacket *op_wrap;       /*< [0..1] Wrapped (inner) package. */
 uintptr_t       op_wrap_start; /*< [valid_if(op_wrap != NULL)] Offset into 'op_wrap', where the package starts. */
 size_t          op_wrap_size;  /*< [valid_if(op_wrap != NULL)] Amount of bytes from 'op_wrap' that should actually be written. */
 void           *op_tail;       /*< [0..op_tsiz] Package tail data. */
 size_t          op_tsiz;       /*< Package tail data size. */
};
#define OPACKET_INIT(p,s)      {p,s,NULL,0,0,NULL,0}
#define OPACKET_HEAD(p,s,base) {p,s,base,0,OPACKET_SIZE(base),NULL,0}
#define OPACKET_TAIL(p,s,base) {NULL,0,base,0,OPACKET_SIZE(base),p,s}
#define OPACKET_SIZE(pck)      ((pck)->op_hsiz+((pck)->op_wrap ? (pck)->op_wrap_size : 0)+(pck)->op_tsiz)

/* Enumerate all data parts of a package in proper order. */
typedef void (KCALL *op_printer)(byte_t const *__restrict data, size_t size, void *closure);
FUNDEF void KCALL opacket_print(struct opacket const *__restrict self, op_printer printer, void *closure);


typedef void (KCALL *ethandler_callback)(struct netdev *__restrict dev, void *__restrict packet,
                                         size_t packet_size, void *closure);

struct nethand {
 /* NOTE: All function pointers are [1..1][lock(:n_lock)] */
 ethandler_callback nh_packet;  /*< Ethernet packet handler (Defaults to 'nethand_packet()')
                                 *  NOTE: This callback is overwritten when userspace
                                 *        opens a socket for RAW network access. */
 void              *nh_closure; /*< Closure parameter passed to packet callbacks. */
};

/* Default network handling functions.
 * This function will execute custom handlers registered by 'ethandler_addhandler()'.
 * NOTE: Implemented in '/src/kernel/dev/net-stack.c' */
FUNDEF void KCALL nethand_packet(struct netdev *__restrict dev, void *__restrict packet,
                                 size_t packet_size, void *UNUSED(closure));


struct netops {
 /* Network card finalizer. */
 void (KCALL *n_fini)(struct netdev *__restrict self); /* [0..1] */

 /* NOTE: The given packet must be allocated in HOST memory!
  * @assume(netdev_writing(self));
  * @assume(self->n_flags&IFF_UP);
  * @assume(OPACKET_SIZE(packet) <= self->n_send_maxsize);
  * @return: -EOK:        Successfully send the packet.
  * @return: -EIO:        Data transmission failed due to an internal adapter error.
  * @return: -ETIMEDOUT:  The adapter failed to acknowledge data having
  *                       been sent before 'n_send_timeout' has passed.
  * @return: E_ISERR(*):  Failed to write the packet to buffer for some reason. */
 errno_t (KCALL *n_send)(struct netdev *__restrict self,
                         struct opacket const *__restrict packet); /* [1..1] */

 /* Callbacks for turning the adapter on/off.
  * NOTE: When called, 'n_flags&IFF_UP' already contains the
  *       new card state, which will be reverted when an error is returned.
  * @function n_ifup: assume(!(self->n_flags&IFF_UP));
  * @function n_ifdown: assume(self->n_flags&IFF_UP);
  * @assume(netdev_writing(self));
  * @return: -EOK:        Successfully turned the card on/off.
  * @return: E_ISERR(*):  Failed to turn the card on/off for some reason. */
 errno_t (KCALL *n_ifup)(struct netdev *__restrict self); /* [1..1] */
 errno_t (KCALL *n_ifdown)(struct netdev *__restrict self); /* [1..1] */

 /* Set 'self->n_macaddr' as the mac address actively being listened for.
  * @assume(self->n_flags&IFF_UP);
  * @assume(netdev_writing(self));
  * @return: -EOK:        Successfully setup the new mac address.
  * @return: E_ISERR(*):  Failed to spoof the mac address for some reason. */
 errno_t (KCALL *n_setmac)(struct netdev *__restrict self); /* [0..1] */

 /* Reset 'self->n_macaddr' the the default, hard-wired mac address of
  * the adapter, and set that address as the one currently listened for.
  * WARNING: This function may be called when the adapter is offline.
  * @return: -EOK:        Successfully reset the mac address.
  * @return: E_ISERR(*):  Failed to reset the mac address for some reason. */
 errno_t (KCALL *n_resetmac)(struct netdev *__restrict self); /* [0..1] */
};

/* Extended (kernel-only) interface flags. */
#define IFF_USERMAC 0x80000000 /*< Set if a user-defined mac address is being used. */

struct netdev {
 struct chrdev   n_dev;           /*< Underlying character device. */
 struct netops  *n_ops;           /*< [const][1..1] Network operation callbacks. */
 rwlock_t        n_lock;          /*< Lock used to protect use of the adapter (send/recv). */
 struct nethand  n_hand;          /*< Network package handler. */
 size_t          n_send_maxsize;  /*< [const] Max package size that can be transmitted. */
#define NETDEV_DEFAULT_STIMEOUT   SEC_TO_JIFFIES(1)
 jtime32_t       n_send_timeout;  /*< [lock(n_lock)] Timeout when waiting for a package to be commit (In jiffies) */
 u32             n_flags;         /*< [lock(n_lock)] Adapter state & flags (Set of 'IFF_*'). */
 u16             n_ip_datagram;   /*< [lock(n_lock)] Next IP datagram id. */
#define NETDEV_DEFAULT_IO_MTU     1480
 u16             n_ip_mtu;        /*< [lock(n_lock)] Max IP packet fragment size. */
 struct macaddr  n_macaddr;       /*< [lock(n_lock)] MAC Address of the device. */
 size_t          n_tx_total;      /*< [lock(n_lock)] Total number of bytes transmitted. */
 size_t          n_rx_total;      /*< [lock(n_lock)] Total number of bytes received. */
};

/* Locking control. */
#define netdev_reading(x)      rwlock_reading(&(x)->n_lock)
#define netdev_writing(x)      rwlock_writing(&(x)->n_lock)
#define netdev_tryread(x)      rwlock_tryread(&(x)->n_lock)
#define netdev_trywrite(x)     rwlock_trywrite(&(x)->n_lock)
#define netdev_tryupgrade(x)   rwlock_tryupgrade(&(x)->n_lock)
#define netdev_read(x)         rwlock_read(&(x)->n_lock)
#define netdev_write(x)        rwlock_write(&(x)->n_lock)
#define netdev_upgrade(x)      rwlock_upgrade(&(x)->n_lock)
#define netdev_downgrade(x)    rwlock_downgrade(&(x)->n_lock)
#define netdev_endread(x)      rwlock_endread(&(x)->n_lock)
#define netdev_endwrite(x)     rwlock_endwrite(&(x)->n_lock)

/* Reference counting control. */
#define NETDEV_TRYINCREF(self)  CHRDEV_TRYINCREF(&(self)->n_dev)
#define NETDEV_INCREF(self)     CHRDEV_INCREF(&(self)->n_dev)
#define NETDEV_DECREF(self)     CHRDEV_DECREF(&(self)->n_dev)

/* Set the current state of a network device. */
#define NETDEV_ISDOWN(self)   (!((self)->n_flags&IFF_UP))
#define NETDEV_ISUP(self)       ((self)->n_flags&IFF_UP)
#define NETDEV_ISMONITOR(self)  ((self)->n_flags&IFF_PROMISC)
#define NETDEV_ISUSERMAC(self)  ((self)->n_flags&IFF_USERMAC)

/* Allocate a new network device.
 * The caller must initialize:
 *   - n_macaddr
 *   - n_ops
 *   - n_send_maxsize */
#define netdev_new(type_size) netdev_cinit((struct netdev *)kcalloc(type_size,GFP_SHARED))
FUNDEF struct netdev *KCALL netdev_cinit(struct netdev *self);



DATDEF struct inodeops netdev_ops;
#define INODE_ISNETDEV(self) ((self)->i_ops == &netdev_ops)
#define INODE_TONETDEV(self)   container_of(self,struct netdev,n_dev.cd_device.d_node)


/* Handle incoming packet data.
 * Usually called from the driver implementation itself.
 * NOTE: This function must be called in a safe context, meaning that
 *       it must be allowed to preempting to other threads must be allowed
 *       and interrupts (if disabled to being with) are allowed to be
 *       re-enabled temporarily.
 * WARNING: Packet handling code may modify the provided 'packet' buffer! */
FUNDEF void KCALL
netdev_recv_unlocked(struct netdev *__restrict self,
                     void *__restrict packet,
                     size_t packet_size);

/* Send the given package over the network.
 * @assume(netdev_writing(self));
 * @return: packet_size: Successfully written the packet into the buffer.
 * @return: -EIO:        Data transmission failed due to an internal adapter error.
 * @return: -ENETDOWN:   The adapter is currently down. (Must call 'netdev_ifup_unlocked()' first)
 * @return: -EMSGSIZE:   The package is too large to be transmit in one piece.
 * @return: -ETIMEDOUT:  The adapter failed to acknowledge data having been send in time.
 * @return: E_ISERR(*):  Failed to write the packet to buffer for some reason. */
FUNDEF errno_t KCALL
netdev_send_unlocked(struct netdev *__restrict self,
                     struct opacket *__restrict pck);

/* Same as 'netdev_send_unlocked()', but also performs locking.
 * @return: -EINTR: The calling thread was interrupted. */
FUNDEF errno_t KCALL
netdev_send(struct netdev *__restrict self,
            struct opacket *__restrict pck);

/* Safely turn a network device on/off.
 * @assume(netdev_writing(self));
 * @return: -EOK:       Successfully changed the network device mode.
 * @return: -EPERM:    [netdev_ifup_unlocked] The card may not power on because special features are
 *                                            enabled, such as 'IFF_USERMAC' or 'IFF_PROMISC'.
 *                                            Clear those bits, then try again.
 *                                   WARNING: This error may also indicate other problems. Disabling
 *                                            special features does not guaranty success the next time.
 * @return: -EALREADY:  The device was already up/down.
 * @return: E_ISERR(*): Failed to turn the device on/off. */
FUNDEF errno_t KCALL netdev_ifup_unlocked(struct netdev *__restrict self);
FUNDEF errno_t KCALL netdev_ifdown_unlocked(struct netdev *__restrict self);

/* Set a custom/reset the default mac address of the given network device.
 * @assume(netdev_writing(self));
 * @return: -EOK:       Successfully set/reset the MAC address listened for.
 * @return: -EPERM:     The adapter cannot be reconfigured to listen for custom addresses.
 * @return: E_ISERR(*): Failed to turn the device on/off. */
FUNDEF errno_t KCALL netdev_setmac_unlocked(struct netdev *__restrict self, struct macaddr const *__restrict addr);
FUNDEF errno_t KCALL netdev_resetmac_unlocked(struct netdev *__restrict self);

/* Get/Set the default network adapter, or NULL if none is installed. */
FUNDEF REF struct netdev *KCALL get_default_adapter(void);
FUNDEF bool KCALL set_default_adapter(struct netdev *__restrict dev, bool replace_existing);

DECL_END
#endif /* !CONFIG_NO_NET */

#endif /* !GUARD_INCLUDE_DEV_NET_H */
