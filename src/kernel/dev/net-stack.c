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

#ifndef CONFIG_NO_NET
#include <hybrid/check.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <syslog.h>
#include <malloc.h>
#include <dev/net.h>
#include <dev/net-stack.h>

#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <hybrid/byteswap.h>
#include <hybrid/section.h>
#include <linker/module.h>

DECL_BEGIN

struct hand_bucket {
 LIST_HEAD(struct ethandler) hb_chain; /*< [0..1] Chain of ethernet handlers. */
};

/* Define all the components of a hash-map used
 * to track user-defined ethernet packet handlers. */
PRIVATE DEFINE_ATOMIC_RWLOCK(hand_lock);
PRIVATE struct hand_bucket *hand_v = NULL;
PRIVATE size_t              hand_a = 0;
PRIVATE size_t              hand_c = 0;


INTERN ATTR_COLDTEXT void KCALL
ethandler_delete_from_instance(struct instance *__restrict inst) {
 struct hand_bucket *b_iter,*b_end;
 struct ethandler **piter,*iter;
 bool has_write_lock = false;
 atomic_rwlock_read(&hand_lock);
 /* Search for ethernet handlers registered by the given instance. */
again:
 b_end = (b_iter = hand_v)+hand_a;
 for (; b_iter != b_end; ++b_iter) {
  piter = &b_iter->hb_chain;
  while ((iter = *piter) != NULL) {
   if (iter->e_owner != inst) { piter = &iter->e_chain.le_next; continue; }
   /* Must upgrade to a write-lock. */
   if (!has_write_lock) {
    has_write_lock = true;
    if (!atomic_rwlock_upgrade(&hand_lock))
        goto again;
   }
   /* Remove this handler entry. */
   *piter = iter->e_chain.le_next;
   assert(hand_c != 0);
   assert(ATOMIC_READ(inst->i_weakcnt) >= 2);
   ATOMIC_FETCHDEC(inst->i_weakcnt);
   --hand_c;
  }
 }
 if (has_write_lock)
      atomic_rwlock_endwrite(&hand_lock);
 else atomic_rwlock_endread(&hand_lock);
}

PUBLIC void KCALL
ethandler_addhandler(struct ethandler *__restrict hand) {
 struct hand_bucket *bucket;
 atomic_rwlock_write(&hand_lock);
 assert((hand_a != 0) == (hand_v != NULL));
 if likely(!INSTANCE_ISUNLOADING(hand->e_owner)) {
  /* Check if the handler hash-map must be re-hashed. */
  if ((hand_c*3)/2 >= hand_a) {
   struct hand_bucket *new_map,*b_iter,*b_end;
   struct ethandler *iter,*next;
   size_t new_count = CEIL_ALIGN(hand_c+1,8);
do_malloc:
   assert(new_count);
   new_map = (struct hand_bucket *)calloc(new_count,sizeof(struct hand_bucket));
   if unlikely(!new_map) {
    if (new_count == hand_c+1) {
     if likely(hand_a) goto do_install;
     syslog(LOG_ERR,"[ETH] Failed to allocate packet handler hash-map\n");
     goto end;
    }
    new_count = hand_c+1;
    goto do_malloc;
   }
   /* Rehash the map. */
   b_end = (b_iter = hand_v)+hand_a;
   for (; b_iter != b_end; ++b_iter) {
    iter = b_iter->hb_chain;
    while (iter) {
     next = iter->e_chain.le_next;
     bucket = &new_map[(u16)iter->e_proto % new_count];
     SLIST_INSERT(bucket->hb_chain,iter,e_chain);
     iter = next;
    }
   }
   /* Free the old hash-map and replace it with the new. */
   free(hand_v);
   hand_a = new_count;
   hand_v = _mall_untrack(new_map);
  }
do_install:
  assert(hand_a);
  /* Insert the handler into the bucket. */
  bucket = &hand_v[(u16)hand->e_proto % hand_a];
  SLIST_INSERT(bucket->hb_chain,hand,e_chain);
  INSTANCE_WEAK_INCREF(hand->e_owner);
  ++hand_c;
 }
end:
 atomic_rwlock_endwrite(&hand_lock);
}

PUBLIC bool KCALL
ethandler_delhandler(struct ethandler *__restrict hand) {
 bool result = false;
 struct hand_bucket *bucket;
 struct ethandler **piter,*iter;
 atomic_rwlock_write(&hand_lock);
 assert((hand_a != 0) == (hand_v != NULL));
 if (hand_c) {
  assert(hand_a);
  bucket = &hand_v[(u16)hand->e_proto % hand_a];
  piter = &bucket->hb_chain;
  /* Search for the handler in question. */
  while ((iter = *piter) != NULL) {
   if (iter == hand) {
    *piter = iter->e_chain.le_next;
    result = true;
    --hand_c;
    break;
   }
   piter = &iter->e_chain.le_next;
  }
 }
 atomic_rwlock_endwrite(&hand_lock);
 return result;
}


PUBLIC void KCALL
nethand_packet(struct netdev *__restrict dev, void *__restrict packet,
               size_t packet_size, void *UNUSED(closure)) {
 struct ethhdr *header;
 struct ethandler *iter;
 CHECK_HOST_DATA(packet,packet_size);
 if (packet_size < sizeof(struct ethhdr)) return;
 header = (struct ethhdr *)packet;
 syslog(LOG_DEBUG,"NET: %[mac] -> %[mac]: proto %.4I16x; size: %Iu\n%$[hex]\n",
       &header->h_source_mac,&header->h_dest_mac,
        BSWAP_N2H16(header->h_proto),
        packet_size,packet_size,packet);
 atomic_rwlock_read(&hand_lock);
 if likely(hand_a) {
  iter = hand_v[(u16)header->h_proto % hand_a].hb_chain;
  while (iter) {
   if (iter->e_proto == header->h_proto) {
    /* Goti! */
    if (INSTANCE_TRYINCREF(iter->e_owner)) {
     (*iter->e_callback)(dev,packet,packet_size,iter->e_closure);
     INSTANCE_DECREF(iter->e_owner);
    }
    break;
   }
   iter = iter->e_chain.le_next;
  }
 }
 atomic_rwlock_endread(&hand_lock);
}


PUBLIC errno_t KCALL
netdev_send_ether_unlocked(struct netdev *__restrict self, struct macaddr dst,
                           be16 type, struct opacket *__restrict pck) {
 struct ethhdr header = {
     .h_dest_mac   = dst,
     .h_source_mac = self->n_macaddr,
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
#endif /* !CONFIG_NO_NET */

#endif /* !GUARD_KERNEL_DEV_NET_STACK_C */
