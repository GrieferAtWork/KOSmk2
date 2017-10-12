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
#ifndef GUARD_MODULES_NET_NE2K_C
#define GUARD_MODULES_NET_NE2K_C 1
#define _KOS_SOURCE 2

#include <dev/rtc.h>
#include <errno.h>
#include <hybrid/byteswap.h>
#include <hybrid/compiler.h>
#include <kernel/export.h>
#include <linux/if_ether.h>
#include <modules/pci.h>
#include <string.h>
#include <syslog.h>

#include "ne2k.h"

DECL_BEGIN

PRIVATE errno_t KCALL ne2k_reset(u16 iobase) {
 u32 start;
 outb(iobase+NE_RESET,inb(iobase+NE_RESET));
 /* Wait for the reset to be completed. */
 start = jiffies;
 while (!(inb(iobase+EN0_ISR) & ENISR_RESET)) {
  if (jiffies-start > 2) {
   syslog(LOG_WARN,"[NE2K] Card failed to acknowledge reset.\n");
   return -ETIMEDOUT;
  }
 }
 return -EOK;
}

PRIVATE errno_t KCALL
ne2k_send(ne2k_t *__restrict self,
          void const *__restrict buf, size_t len) {
 u32 start; u8 status;
 size_t transmission_size;
 /* TODO: This only checks the hard limit, but I'm guessing
  *       a connected device is more limited that this... */
 if (len > 0xffff) return -EINVAL;
 transmission_size = len;
 if (transmission_size&1) ++transmission_size;

 /* Initialize a send sequence. */
 outb(self->n_iobase+E8390_CMD,E8390_PAGE0|E8390_NODMA|E8390_START);
 outb(self->n_iobase+EN0_ISR,ENISR_RDC); /* ??? */
 outb(self->n_iobase+EN0_RCNTLO,transmission_size & 0xff);
 outb(self->n_iobase+EN0_RCNTHI,transmission_size >> 8);
 outb(self->n_iobase+EN0_RSARLO,0x00);
 outb(self->n_iobase+EN0_RSARHI,64); /* TODO: Start page. */
 outb(self->n_iobase+E8390_CMD,E8390_RWRITE|E8390_START);
 outsw(self->n_iobase+NE_DATAPORT,buf,len/2);
 if (len&1) outw(self->n_iobase+NE_DATAPORT,(u16)(((u8 *)buf)[len-1]));

 start = jiffies;
 while (!((status = inb(self->n_iobase+EN0_ISR))&ENISR_RDC)) {
  if (jiffies-start > 2) {
   syslog(LOG_WARN,"[NE2K] Timeout during DMA write\n");
   /* Reset the card, then time out. */
   ne2k_reset(self->n_iobase);
   return -ETIMEDOUT;
  }
  task_yield();
 }
 outb(self->n_iobase+EN0_ISR,ENISR_RDC); /* Acknowledge */

 outb(self->n_iobase+E8390_CMD,E8390_NODMA|E8390_START);
 outb(self->n_iobase+EN0_TPSR,64); /* TODO: Start page. */
 outb(self->n_iobase+EN0_TCNTLO,transmission_size & 0xff);
 outb(self->n_iobase+EN0_TCNTHI,transmission_size >> 8);
 outb(self->n_iobase+E8390_CMD,E8390_TRANS|E8390_NODMA|E8390_START);

 start = jiffies;
 while (!((status = inb(self->n_iobase+EN0_ISR))&ENISR_TX)) {
  if (jiffies-start > 16) {
   syslog(LOG_WARN,"[NE2K] Timeout during send (%.2I8x)\n",status);
   ne2k_reset(self->n_iobase);
   return -ETIMEDOUT;
  }
  task_yield();
 }
 outb(self->n_iobase+EN0_ISR,ENISR_RDC); /* Acknowledge */

 /* Check if the status register indicates an error. */
 if ((status&(ENISR_TX_ERR|ENISR_TX)) != ENISR_TX)
     return -EIO;

 return -EOK;
}



#define NE2K   container_of(ino,ne2k_t,n_dev.n_dev.cd_device.d_node)
PRIVATE void KCALL
ne2k_fini(struct inode *__restrict ino) {
 PCI_DEVICE_DECREF(NE2K->n_pcidev);
}
#undef NE2K


PRIVATE struct inodeops ne2k_ops = {
    .ino_fini = &ne2k_fini,
};

PRIVATE errno_t KCALL
ne2k_probe(struct pci_device *dev) {
 ne2k_t *netdev; errno_t error;
 u8 prom[32]; unsigned int i;
 u16 ioaddr = dev->pd_resources[0].pr_begin;
 if (inb(ioaddr) == 0xff) return -ENODEV;

 /* Reset the card. */
 error = ne2k_reset(ioaddr);
 if (E_ISERR(error)) return error;

 /* Execute a sequence of startup instructions. */
 { PRIVATE struct { u8 off,val; } startup[] = {
       {EN0_ISR,   0xff}, /* Acknowledge interrupts. (From before) */
       {E8390_CMD, E8390_PAGE0|E8390_STOP|E8390_NODMA},
       {EN0_DCFG,  ENDCFG_FT1|ENDCFG_LS|ENDCFG_WTS}, /* Set word-wide mode. */
       {EN0_RCNTLO,0}, /* Clear count registers. */
       {EN0_RCNTHI,0}, /* Clear count registers. */
       {EN0_IMR,   0}, /* Mask completion irq. */
       {EN0_ISR,   0xff}, /* ... */
       {EN0_RXCR,  ERXCR_MON}, /* Enable monitoring mode (Don't receive packages for now). */
       {EN0_TXCR,  ETXCR_LOOPBACK_INTERN}, /* Set loopback mode to intern (Don't send packages for now). */
       /* At this point, we'll be trying to read the first 32 bytes of PROM. */
       {EN0_RCNTLO,32}, /* Setup 32 bytes for reading. */
       {EN0_RCNTHI,0},  /* ... */
       {EN0_RSARLO,0},  /* Select DMA 0x0000 for reading. */
       {EN0_RSARHI,0},  /* ... */
       {E8390_CMD, E8390_RREAD|E8390_START}, /* Execute a remove read. */
   };
   for (i = 0; i < COMPILER_LENOF(startup); ++i)
        outb(ioaddr+startup[i].off,startup[i].val);
 }

 /* Read PROM. */
 for (i = 0; i < 32; i++)
      prom[i] = inb(ioaddr+0x10);

 outb(ioaddr+EN0_DCFG,ENDCFG_FT1|ENDCFG_LS|ENDCFG_WTS);

 netdev = (ne2k_t *)netdev_new(sizeof(ne2k_t));
 if unlikely(!netdev) return -ENOMEM;
 memcpy(netdev->n_dev.n_mac.ma_bytes,prom,6);
 netdev->n_iobase = ioaddr;
 netdev->n_pcidev = dev;
 PCI_DEVICE_INCREF(dev);
 netdev->n_dev.n_dev.cd_device.d_node.i_ops = &ne2k_ops;

 error = device_setup(&netdev->n_dev.n_dev.cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) { free(netdev); return error; }

 /* Register the device. */
 /* TODO: Tracking of Ethernet card numbers for dynamic allocation. */
 error = CHRDEV_REGISTER(netdev,DV_ETHERNET);

 syslog(LOG_DEBUG,"[NE2K] Found network card (I/O %.4I16x; MAC: %[mac])\n",
        ioaddr,&netdev->n_dev.n_mac);
 syslog(LOG_DEBUG,"%.?[hex]\n",32,prom);

#if 1
 ne2k_reset(ioaddr);
 { struct data {
    struct PACKED ethhdr h;
    char                 text[64];
   } data = {
    {
        .h_dest   = {1,2,3,4,5,6},
        .h_source = {1,2,3,4,5,6},
        .h_proto  = BSWAP_H2N16(0xba73),
    },
    "THIS IS DATA TO BE SENT\n"
    "THIS IS DATA TO BE SENT\n"
   };
   /* Test sending some data. - Should appear in 'dump.dat' */
   ne2k_send(netdev,&data,sizeof(struct data));
 }
#endif

 NETDEV_DECREF(&netdev->n_dev);
 return error;
}



PRIVATE void MODULE_INIT KCALL ne2k_init(void) {
 struct pci_device *dev;
 PCI_READ();
 /* Search for network cards. */
 PCI_FOREACH_CLASS(dev,PCI_DEV8_CLASS_NETWORK,
                   PCI_DEV8_CLASS_NOCLASS) {
  if (!(dev->pd_resources[0].pr_flags&PCI_RESOURCE_IO)) continue;
  if (dev->pd_resources[0].pr_size < 0xff) continue;
  ne2k_probe(dev);
 }
 PCI_ENDREAD();
}

DECL_END

#endif /* !GUARD_MODULES_NET_NE2K_C */
