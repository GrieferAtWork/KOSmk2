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
#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

struct macaddr {
 u8  ma_bytes[6];
};

struct netdev {
 struct chrdev   n_dev;           /*< Underlying character device. */
 struct macaddr  n_mac;           /*< MAC Address of the device. */
 u32             n_write_timeout; /*< Timeout when writing to buffer (In jiffies; Ignored by software-buffered devices) */
 u32             n_send_timeout;  /*< Timeout when waiting for a package to be commit (In jiffies) */
 rwlock_t        n_send_lock;     /*< Lock that must held when sending data. */

 /* Begin/end sending packets.
  * The usual implementation look like this:
  * >> n_send_packet(packet) { WRITE_TO_BUFFER(packet); }
  * >> n_send_commit()       { TRANSFER_SEND_BUFFER(); }
  * >> n_send_discard()      { CLEAR_BUFFER(); }
  * NOTE: The buffer described here may either be implemented in hardware, or software. */

 /* NOTE: The given packet must be allocated in HOST memory!
  * @assume(rwlock_writing(&dev->n_buffer_lock));
  * @return: packet_size:  Successfully written the packet into the buffer.
  * @return: -EINVAL:      The packet is too large to be handled by the adapter.
  * @return: -ENOBUFS:     The buffer is full and already written packages must
  *                        be commit before new can be added (The caller must
  *                        execute 'n_send_commit()' followed by 'n_send_begin()',
  *                        before attempting to write more data.
  * @return: -EIO:         An I/O error occurred while writing to a buffer.
  * @return: -ETIMEDOUT:   A hardware buffer failed to acknowledge the packet with 'n_write_timeout' ticks.
  * @return: E_ISERR(*):   Failed to write the packet to buffer for some reason. */
 ssize_t (KCALL *n_send_packet)(struct netdev *__restrict dev,
                                HOST void const *__restrict packet,
                                size_t packet_size); /* [1..1] */
 /* NOTE: No matter the return value, this function always unlocks the adapter's internal buffer lock.
  * @assume(rwlock_writing(&dev->n_buffer_lock));
  * @return: * :           The total amount of previously buffered bytes that were committed.
  * @return: -IO:          Data transmission failed for some reason.
  * @return: -ETIMEDOUT:   The adapter failed to acknowledge data having been send in time.
  * @return: E_ISERR(*):   Failed to send data for some reason. */
 ssize_t (KCALL *n_send_commit)(struct netdev *__restrict dev); /* [1..1] */
 /* @assume(rwlock_writing(&dev->n_buffer_lock)); */
 void    (KCALL *n_send_discard)(struct netdev *__restrict dev); /* [1..1] */



};
#define NETDEV_TRYINCREF(self)  CHRDEV_TRYINCREF(&(self)->n_dev)
#define NETDEV_INCREF(self)     CHRDEV_INCREF(&(self)->n_dev)
#define NETDEV_DECREF(self)     CHRDEV_DECREF(&(self)->n_dev)

#define netdev_new(type_size) ((struct netdev *)chrdev_new(type_size))


DECL_END

#endif /* !GUARD_INCLUDE_DEV_NET_H */
