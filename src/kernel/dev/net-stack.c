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
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/check.h>
#include <hybrid/minmax.h>
#include <syslog.h>
#include <dev/net.h>
#include <dev/net-stack.h>

#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <hybrid/byteswap.h>

DECL_BEGIN

PUBLIC void KCALL
nethand_packet(void *__restrict packet, size_t size) {
 struct ethhdr *header;
 CHECK_HOST_DATA(packet,size);
 if (size < sizeof(struct ethhdr)) return;
 header = (struct ethhdr *)packet;
 syslog(LOG_DEBUG,"NET: %[mac] -> %[mac]: proto %.4I16x; size: %Iu\n%.?[hex]\n",
       &header->h_source_mac,&header->h_dest_mac,
        BSWAP_N2H16(header->h_proto),size,size,packet);
 /* TODO */

}


PUBLIC errno_t KCALL
netdev_send_ether_unlocked(struct netdev *__restrict self, struct macaddr dst,
                           be16 type, struct opacket *__restrict pck) {
 struct ethhdr header = {
     .h_dest_mac   = dst,
     .h_source_mac = self->n_mac,
     .h_proto      = type,
 };
 struct opacket container = OPACKET_HEAD(&header,sizeof(header),pck);
 return netdev_send_unlocked(self,&container);
}



PUBLIC errno_t KCALL
netdev_send_ip_unlocked(struct netdev *__restrict self,
                        be32 src, be32 dst, u8 protocol,
                        struct opacket *__restrict pck) {
 be16 datagram_id; ssize_t error = -EOK;
 size_t packet_size = OPACKET_SIZE(pck);
 if unlikely(packet_size > (size_t)((u16)0-sizeof(struct iphdr)))
    return -EMSGSIZE;
 /* Allocate a unique datagram ID. */
 datagram_id = BSWAP_H2N16(self->n_ip_datagram++);
 {
  struct iphdr header = {
      .version  = IPVERSION,
      .ihl      = sizeof(struct iphdr)/4,
      .tos      = (u_int8_t)0,
      .tot_len  = (u_int16_t)BSWAP_H2N16(sizeof(struct iphdr)+packet_size),
      .id       = (u_int16_t)datagram_id,
    //.frag_off = (u_int16_t)0, /* Filled later. */
      .ttl      = (u_int8_t)0x10,
      .protocol = (u_int8_t)protocol,
    //.check    = (u_int16_t)0, /* Filled later. */
      .saddr    = (u_int32_t)src,
      .daddr    = (u_int32_t)dst,
  };
  struct opacket container;
  container.op_head       = &header;
  container.op_hsiz       = sizeof(header);
  container.op_wrap       = pck;
  container.op_wrap_start = 0;
  container.op_tsiz       = 0;
  while (packet_size) {
   be16 *iter,*end; u16 chksum = 0;
   header.frag_off = (u_int16_t)BSWAP_H2N16(container.op_wrap_start);
   /* Calculate the fragment size. */
   if (packet_size < self->n_ip_mtu) {
    container.op_wrap_size = packet_size;
   } else {
    container.op_wrap_size = self->n_ip_mtu;
    /* Set the more-fragments flag. */
    header.frag_off |= (u_int16_t)BSWAP_H2N16(IP_MF);
   }

#if 0 /* TO-DO: Must be enable when a 666-bit integer overflows.
       * >> https://tools.ietf.org/html/rfc3514 */
   header.frag_off |= BSWAP_H2N16_C(IP_EVIL);
#endif

   /* Calculate the checksum. */
   header.check = 0;
   end = (iter = (be16 *)&header)+10;
   for (; iter != end; ++iter) chksum += BSWAP_N2H16(*iter);
   header.check = (u_int16_t)BSWAP_H2N16(~chksum);
   error = netdev_send_ether_unlocked(self,ETHADDR_BROADCAST,
                                      BSWAP_H2N16(ETH_P_IP),&container);
   if (E_ISERR(error)) break;
   container.op_wrap_start += (u_int16_t)container.op_wrap_size;
   packet_size             -= container.op_wrap_size;
  }
 }
 return error;
}



DECL_END

#endif /* !GUARD_KERNEL_DEV_NET_STACK_C */
