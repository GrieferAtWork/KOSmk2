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
#include <kernel/irq.h>
#include <linux/if_ether.h>
#include <modules/pci.h>
#include <sched/cpu.h>
#include <sched/rpc.h>
#include <sched/smp.h>
#include <string.h>
#include <syslog.h>

#include "ne2k.h"

#define NE2K_IRQ IRQ_PIC2_FREE3 /* TODO: Dynamically allocate. */
DECL_BEGIN

INTERN errno_t KCALL net_resetdev(ne2k_t *__restrict dev) {
 errno_t error = net_reset(dev->n_iobase);
 if (E_ISOK(error)) {
  /* Clear dangling interrupt messages. */
  ATOMIC_WRITE(dev->n_sendend.s_ticket,0);
 }
 return error;
}
INTERN errno_t KCALL net_reset_base(u16 iobase) {
 u32 start;
 outb(iobase+NE_RESET,inb(iobase+NE_RESET));
 /* Wait for the reset to be completed. */
 start = jiffies32;
 while (!(inb(iobase+EN0_ISR) & ENISR_RESET)) {
  if (jiffies32-start > 2) {
   syslog(LOG_WARN,"[NE2K] Card failed to acknowledge reset.\n");
   return -ETIMEDOUT;
  }
 }
 return -EOK;
}
INTERN errno_t KCALL net_reset(u16 iobase) {
 errno_t error = net_reset_base(iobase);
 /* Enable interrupts that we are handling. */
 if (E_ISOK(error)) outb(iobase+EN0_IMR,ENISR_ALL);
 return error;
}
INTERN s32 KCALL net_waitdma(ne2k_t *__restrict dev) {
 u32 start = jiffies32; u8 status;
 while (!((status = inb(dev->n_iobase+EN0_ISR))&ENISR_RDC)) {
  if (jiffies32-start > dev->n_dev.n_write_timeout) {
   syslog(LOG_WARN,"[NE2K] Timeout during DMA wait\n");
   return -ETIMEDOUT;
  }
  task_yield();
 }
 /* Acknowledge remote DMA completion. */
 outb(dev->n_iobase+EN0_ISR,ENISR_RDC);
 return (s32)status;
}





PRIVATE void KCALL ne2k_recv(ne2k_t *dev);
PRIVATE struct job ne2k_recv_job = JOB_INIT((void(KCALL *)(void *))&ne2k_recv,NULL);
PRIVATE void KCALL ne2k_recv(ne2k_t *dev) {
 syslog(LOG_WARN,"NE2K: Receive data\n");
 /* TODO */
}

DEFINE_INT_HANDLER(ne2k_irq,ne2k_int);
PRIVATE isr_t ne2k_isr = ISR_DEFAULT(NE2K_IRQ,&ne2k_irq);
INTERN void ne2k_int(void) {
 u8 status; u16 iobase; ne2k_t *dev;
 if (IRQ_PIC_SPURIOUS(NE2K_IRQ)) return;
 dev    = (ne2k_t *)ne2k_recv_job.j_data;
 iobase = dev->n_iobase;
 outb(iobase+E8390_CMD,E8390_NODMA|E8390_START); /* Start receiving data. (XXX: Is this really required?) */

 /* Read the interrupt status register. */
 status = inb(iobase+EN0_ISR) & ENISR_ALL;
 syslog(LOG_WARN,"NE2K: Interrupt %.2I8x\n",status);

 if (status & ENISR_RX) {
  /* Schedule a job to safely receive data. */
  schedule_work(&ne2k_recv_job);
  /* Since we're going to re-enable interrupts before
   * data will actually be read, we must mask all
   * interrupts caused by the chip, so-as not to deal
   * with multiple incoming packets at once.
   */
  outb(iobase+EN0_IMR,ENISR_ALL);
  /* Don't acknowledge the receive signal below.
   * It will only be acknowledged once the data loader job has finished. */
  status &= ~ENISR_RX;
 }

 if (status & ENISR_RX_ERR) {
  u8 recv_error = inb(iobase+EN0_TSR);
  syslog(LOG_ERR,"[NE2K] Receive error: %.2I8x\n",recv_error);
  /* XXX: Wake receiver? */
 }

 if (status & (ENISR_TX|ENISR_TX_ERR)) {
  u8 transmit_status;
  if (status&ENISR_TX_ERR) {
   transmit_status = inb(iobase+EN0_TSR);
   if (transmit_status&ETSR_EMASK)
       syslog(LOG_ERR,"[NE2K] Transmit failed: %.2I8x\n",transmit_status);
  } else {
   transmit_status = ETSR_PTX; /* Successful transmission. */
  }
  dev->n_senderr = transmit_status;
  /* Acknowledge */
  if (status&ENISR_TX_ERR) outb(iobase+EN0_ISR,ENISR_TX);
  if (status&ENISR_TX) outb(iobase+EN0_ISR,ENISR_TX);
  COMPILER_WRITE_BARRIER();
  sem_release(&dev->n_sendend,1);
 }
    
 if unlikely(status & ENISR_OVER)
    syslog(LOG_ERR,"[NE2K] Receiver overwrote the ring\n");
 if unlikely(status & ENISR_COUNTERS)
    syslog(LOG_ERR,"[NE2K] Counters need emptying\n");

 /* Acknowledge all handled signals. */
 if (status)
     outb(iobase+EN0_ISR,status);

 /* Acknowledge the interrupt within the PIC. */
 IRQ_PIC_EOI(NE2K_IRQ);
}


PRIVATE errno_t KCALL
ne2k_send(ne2k_t *__restrict self,
          void const *__restrict buf, size_t len) {
 size_t transmission_size; errno_t error;
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

 error = net_waitdma(self);
 if (E_ISERR(error)) goto reset;

 outb(self->n_iobase+E8390_CMD,E8390_NODMA|E8390_START);
 outb(self->n_iobase+EN0_TPSR,64); /* TODO: Start page. */
 outb(self->n_iobase+EN0_TCNTLO,transmission_size & 0xff);
 outb(self->n_iobase+EN0_TCNTHI,transmission_size >> 8);
 outb(self->n_iobase+E8390_CMD,E8390_TRANS|E8390_NODMA|E8390_START);

 error = sem_timedwait(&self->n_sendend,jiffies+
                       self->n_dev.n_send_timeout);
 if (E_ISERR(error)) goto reset;

 /* Check if the status register indicated an error. */
 if ((self->n_senderr&(ENISR_TX_ERR|ENISR_TX)) != ENISR_TX)
      return -EIO;

 return -EOK;
reset:
 return error;
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
 error = net_reset_base(ioaddr);
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
 netdev->n_dev.n_dev.cd_device.d_node.i_ops = &ne2k_ops;
 netdev->n_iobase = ioaddr;
 netdev->n_pcidev = dev;
 PCI_DEVICE_INCREF(dev);
 sem_cinit(&netdev->n_sendend,0);

 /* Setup the device. */
 error = device_setup(&netdev->n_dev.n_dev.cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) { PCI_DEVICE_DECREF(dev); free(netdev); return error; }

 /* Register the device. */
 /* TODO: Tracking of Ethernet card numbers for dynamic allocation. */
 error = CHRDEV_REGISTER(netdev,DV_ETHERNET);
 if (E_ISERR(error)) goto end;

 /* Setup an IRQ handler for incoming packages. */
 { u32 v = pci_read(dev->pd_addr,PCI_GDEV3C);
   ne2k_recv_job.j_data = netdev; /* TODO: This is unsafe */
   pci_write(dev->pd_addr,PCI_GDEV3C,
            (v & ~(PCI_GDEV3C_IRQLINEMASK|PCI_GDEV3C_IRQPINMASK))|
            ((NE2K_IRQ >= IRQ_PIC2_BASE ? 1 : 0) << PCI_GDEV3C_IRQPINSHIFT)|
            ((NE2K_IRQ-IRQ_PIC1_BASE) << PCI_GDEV3C_IRQLINESHIFT));
   irq_vset(BOOTCPU,&ne2k_isr,NULL,IRQ_SET_RELOAD);
 }

 syslog(LOG_DEBUG,"[NE2K] Found network card (I/O %.4I16x; MAC: %[mac])\n",
        ioaddr,&netdev->n_dev.n_mac);
 syslog(LOG_DEBUG,"%.?[hex]\n",32,prom);

 /* Reset the device. */
 net_reset(ioaddr);

#if 1
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
end:
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
