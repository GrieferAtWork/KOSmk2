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
#ifndef GUARD_INCLUDE_DEV_NET_STACK_H
#define GUARD_INCLUDE_DEV_NET_STACK_H 1

#include <hybrid/compiler.h>

#ifndef CONFIG_NO_NET
#include <dev/net.h>
#include <hybrid/types.h>
#include <hybrid/list/list.h>

DECL_BEGIN

struct ethandler {
 /* Ethernet protocol handler. */
 SLIST_NODE(struct ethandler) e_chain;    /*< [0..1] Internal list of ethernet handlers. */
 WEAK REF struct instance    *e_owner;    /*< [1..1] Owner instance reference. */
 ethandler_callback           e_callback; /*< [1..1] Handler callback. */
 void                        *e_closure;  /*< [?..?] Closure passed to `e_callback' during execution. */
 be16                         e_proto;    /*< [const] Prototype ID processed by this handler. */
 u16                        __e_pad;      /* ... */
};

/* Register/Delete a given ethernet packet handler.
 * NOTE: Ethernet handlers are automatically deleted during module unloading. */
FUNDEF void KCALL ethandler_addhandler(struct ethandler *__restrict hand);
/* @return: true:  Successfully removed the handler callback.
 * @return: false: The given handler callback was never added. */
FUNDEF bool KCALL ethandler_delhandler(struct ethandler *__restrict hand);




/* Send layer #2 ethernet packages. */
FUNDEF errno_t KCALL
netdev_send_ether_unlocked(struct netdev *__restrict self,
                           struct macaddr dst, be16 type,
                           struct opacket *__restrict pck);

/* Send layer #3 IP packages. */
FUNDEF errno_t KCALL
netdev_send_ip_unlocked(struct netdev *__restrict self,
                        be32 src, be32 dst, u8 protocol,
                        struct opacket *__restrict pck);

DECL_END
#endif /* !CONFIG_NO_NET */

#endif /* !GUARD_INCLUDE_DEV_NET_STACK_H */
