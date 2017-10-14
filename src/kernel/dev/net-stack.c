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
#ifndef GUARD_KERNEL_DEV_NET_STACK_C
#define GUARD_KERNEL_DEV_NET_STACK_C 1

#include <hybrid/compiler.h>
#include <dev/net.h>
#include <dev/net-stack.h>
#include <linux/if_ether.h>

DECL_BEGIN

PUBLIC ssize_t KCALL
netdev_send_ether_unlocked(struct netdev *__restrict self, struct macaddr dst, be16 type,
                           struct opacket *__restrict pck, size_t packet_size) {
 struct ethhdr header = {
     .h_dest_mac   = dst,
     .h_source_mac = self->n_mac,
     .h_proto      = type,
 };
 struct opacket container = {
     .op_data = &header,
     .op_size = sizeof(header),
     .op_wrap = pck,
     .op_tsiz = 0,
 };
 return netdev_send_unlocked(self,&container,packet_size+sizeof(header));
}


DECL_END

#endif /* !GUARD_KERNEL_DEV_NET_STACK_C */
