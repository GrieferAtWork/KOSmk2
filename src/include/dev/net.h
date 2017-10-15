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
#include <hybrid/types.h>
#include <kernel/malloc.h>

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
 /* NOTE: All function pointers are [1..1][lock(:n_hand_lock)] */
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



#define NETDEV_DEFAULT_IO_MTU       1480
#define NETDEV_DEFAULT_STIMEOUT     SEC_TO_JIFFIES(1)
struct netdev {
 struct chrdev   n_dev;           /*< Underlying character device. */
 struct macaddr  n_mac;           /*< MAC Address of the device. */
 u8              n_pad[2];        /* ... */
 atomic_rwlock_t n_hand_lock;     /*< Lock used to protect the incoming packet buffer. */
 struct nethand  n_hand;          /*< Network package handler. */
 rwlock_t        n_send_lock;     /*< Lock that must held when sending data. */
 size_t          n_send_maxsize;  /*< [const] Max package size that can be transmitted. */
 jtime32_t       n_send_timeout;  /*< [lock(n_send_lock)] Timeout when waiting for a package to be commit (In jiffies) */
 u16             n_ip_datagram;   /*< [lock(n_send_lock)] Next IP datagram id. */
 u16             n_ip_mtu;        /*< [lock(n_send_lock)] Max IP packet fragment size. */
 /* NOTE: The given packet must be allocated in HOST memory!
  * @assume(rwlock_writing(&self->n_send_lock));
  * @return: -EOK:        Successfully written the packet into the buffer.
  * @return: -IO:         Data transmission failed due to an internal adapter error.
  * @return: -ETIMEDOUT:  The adapter failed to acknowledge data having been send in time.
  * @return: E_ISERR(*):  Failed to write the packet to buffer for some reason. */
 errno_t (KCALL *n_send)(struct netdev *__restrict self,
                         struct opacket const *__restrict packet); /* [1..1] */
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


/* Send the given package over the network.
 * @assume(rwlock_writing(&self->n_send_lock));
 * @assume(packet_size == opacket_abssize(packet));
 * @return: packet_size: Successfully written the packet into the buffer.
 * @return: -IO:         Data transmission failed due to an internal adapter error.
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


/* Get/Set the default adapter device, or NULL if none is installed. */
FUNDEF REF struct netdev *KCALL get_default_adapter(void);
FUNDEF bool KCALL set_default_adapter(struct netdev *__restrict dev, bool replace_existing);

DECL_END
#endif /* !CONFIG_NO_NET */

#endif /* !GUARD_INCLUDE_DEV_NET_H */
