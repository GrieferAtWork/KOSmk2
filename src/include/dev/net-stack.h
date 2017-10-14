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

#include <dev/net.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

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

#endif /* !GUARD_INCLUDE_DEV_NET_STACK_H */
