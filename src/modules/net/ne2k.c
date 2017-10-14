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

#include <bits/endian.h>
#include <dev/rtc.h>
#include <endian.h>
#include <errno.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <modules/pci.h>
#include <sched/cpu.h>
#include <sched/rpc.h>
#include <sched/smp.h>
#include <string.h>
#include <syslog.h>

#include "ne2k.h"

#define NE2K_IRQ IRQ_PIC2_FREE3 /* TODO: Dynamically allocate. */
DECL_BEGIN

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
INTERN errno_t KCALL net_reset(ne2k_t *__restrict dev) {
 errno_t error;
 error = net_reset_base(dev->n_iobase);
 /* Execute the re-initialization command sequence */
 if (E_ISOK(error)) {
  /* Clear dangling interrupt messages. */
  ATOMIC_WRITE(dev->n_sendend.s_ticket,0);
  /* Reset buffer pointers. */
  dev->n_nextpck = dev->n_rx_begin;

  { unsigned int i;
    struct { u8 off,val; } reset[] = {
        {E8390_CMD, E8390_PAGE0|E8390_STOP|E8390_NODMA},
        {EN0_DCFG,  ENDCFG_FT1|ENDCFG_LS|ENDCFG_WTS}, /* Set word-wide mode. */
        {EN0_RCNTLO,0}, /* Clear count registers. */
        {EN0_RCNTHI,0}, /* ... */
        {EN0_RXCR,  ERXCR_AB}, /* Accept broadcast packages. */
        {EN0_TXCR,  ETXCR_LOOPBACK_INTERN}, /* Disable loopback for all packages. */
        /* Configure the card's receive buffer. */
        {EN0_BOUNDARY,dev->n_nextpck},
        {EN0_STARTPG,dev->n_rx_begin},
        {EN0_STOPPG,dev->n_rx_end},
        {EN0_ISR,   0xff}, /* Acknowledge anything that may still be dangling. */
        {EN0_IMR,   ENISR_ALL}, /* Enable all the interrupts that we are handling. */
        {E8390_CMD, E8390_PAGE1|E8390_NODMA|E8390_STOP}, /* Switch to page 1. */
    };
    for (i = 0; i < COMPILER_LENOF(reset); ++i)
         outb(dev->n_iobase+reset[i].off,reset[i].val);
    for (i = 0; i < 6; ++i) /* Configure the hardware address to listen for. */
         outb(dev->n_iobase+EN1_PHYS_SHIFT(i),dev->n_dev.n_mac.ma_bytes[i]);
    for (i = 0; i < 8; ++i) /* XXX: Multicast? */
         outb(dev->n_iobase+EN1_MULT_SHIFT(i),0);
    /* Still being in page #1, set the next-package pointer. */
    outb(dev->n_iobase+EN1_CURPAG,dev->n_nextpck);

    /* Finally, initialize the card. */
    outb(dev->n_iobase+E8390_CMD,E8390_PAGE0|E8390_NODMA|E8390_START);

    /* And disable loopback, enabling outgoing packages. */
    outb(dev->n_iobase+EN0_TXCR,ETXCR_LOOPBACK_NORMAL);
  }
 }
 return error;
}
INTERN s32 KCALL net_waitdma(ne2k_t *__restrict dev) {
 u32 start = jiffies32; u8 status;
 while (!((status = inb(dev->n_iobase+EN0_ISR))&ENISR_RDC)) {
  if (jiffies32-start > dev->n_dma_timeout) {
   syslog(LOG_WARN,"[NE2K] Timeout during DMA wait\n");
   return -ETIMEDOUT;
  }
  task_yield();
 }
 /* Acknowledge remote DMA completion. */
 outb(dev->n_iobase+EN0_ISR,ENISR_RDC);
 return (s32)status;
}





PRIVATE void KCALL ne2k_recv(ne2k_t *self);
PRIVATE struct job ne2k_recv_job = JOB_INIT((void(KCALL *)(void *))&ne2k_recv,NULL);
PRIVATE void KCALL ne2k_recv(ne2k_t *self) {
 u16 iobase = self->n_iobase;
 syslog(LOG_WARN,"NE2K: Receive data\n");
 for (;;) {
  STATIC_ASSERT(!(sizeof(pck_header_t) & 1));
  struct ipacket *pck; errno_t error;
  pck_header_t header; u8 curr_page;
  u16 curr_addr,high_data,low_data,aligned_size;

  /* Signal start. */
  outb(iobase+E8390_CMD,E8390_PAGE1|E8390_NODMA|E8390_START);
  /* Stop if the current read-header is where the next package will eventually be. */
  curr_page = inb(iobase+EN1_CURPAG);
  if (curr_page == self->n_nextpck) break;
  curr_addr = curr_page*NET_PAGESIZE;

  /* Read the package header. */
  outb(iobase+E8390_CMD,E8390_NODMA|E8390_START);
  outb(iobase+EN0_RCNTLO,sizeof(pck_header_t)); /* Only read the header (for now). */
  outb(iobase+EN0_RCNTHI,0);
  outb(iobase+EN0_RSARLO,0); /* Start reading at the page base address. */
  outb(iobase+EN0_RSARHI,curr_page);
  outb(iobase+E8390_CMD,E8390_RREAD|E8390_START); /* initiate the read. */
  insw(iobase+NE_DATAPORT,&header,sizeof(pck_header_t)/2);

  /* Inherit the next-package pointer. */
  self->n_nextpck = header.ph_nextpg;

  /* Now we know what this package looks like, we can start processing it. */
  if unlikely(!netdev_pushok_atomic(&self->n_dev,header.ph_size) ||
               header.ph_size == 0xffff) {
   error = -EMSGSIZE;
fail:
   syslog(LOG_ERR,"[NE2K] Cannot receive package of %I16u bytes: %[errno]\n",
          header.ph_size,-error);
   continue;
  }

  /* Allocate the package we're going to fill. */
  pck = ipacket_alloc((size_t)header.ph_size);
  if unlikely(!pck) { error = -ENOMEM; goto fail; }

  /* Now read the package data.
   * First, we must figure out how must should be read from high memory, and how much from low memory.
   * Since Ne2000 uses a ring-buffer, what is done here is pretty much the same as done KOS's I/O buffers. */
  high_data = self->n_rx_end-curr_addr;
  if (high_data >= header.ph_size) {
   high_data = header.ph_size;
   low_data  = 0;
  } else {
   /* Read the remainder from low memory. */
   low_data  = header.ph_size-high_data;
   /* Make sure low memory doesn't wrap around again. */
   if unlikely(low_data > curr_addr) {
err_packet_size:
    error = -EMSGSIZE;
    ipacket_free(pck);
    goto fail;
   }
  }
  /* At this point we must read up to 2 areas of memory:
   * READ: DEVICE(curr_addr...+=high_data)       --> PACKET(0...+=high_data)
   * READ: DEVICE(self->n_rx_begin...+=low_data) --> PACKET(high_data...+=low_data)
   */
  assert(high_data || low_data);
  /* Read high memory */
  if (high_data) {
   outb(iobase+E8390_CMD,E8390_NODMA|E8390_START);
   aligned_size = high_data;
   if (aligned_size&1) ++aligned_size;
   outb(iobase+EN0_RCNTLO,aligned_size & 0xff);
   outb(iobase+EN0_RCNTHI,aligned_size >> 16);
   outb(iobase+EN0_RSARLO,sizeof(pck_header_t)); /* Read data after the package header. */
   outb(iobase+EN0_RSARHI,curr_page);
   outb(iobase+E8390_CMD,E8390_RREAD|E8390_START); /* Initiate the read. */
   insw(iobase+NE_DATAPORT,pck->ip_data,high_data/2);
   /* Also read the last byte. */
   if (high_data&1) {
    u16 last = inw(iobase+NE_DATAPORT);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    pck->ip_data[high_data-1] = last >> 8;
#else
    pck->ip_data[high_data-1] = last & 0xff;
#endif
   }
  }

  /* Read low memory */
  if (low_data) {
   byte_t *dst = pck->ip_data+high_data;
   outb(iobase+E8390_CMD,E8390_NODMA|E8390_START);
   aligned_size = low_data;
   if (aligned_size&1) ++aligned_size;
   outb(iobase+EN0_RCNTLO,aligned_size & 0xff);
   outb(iobase+EN0_RCNTHI,aligned_size >> 16);
   outb(iobase+EN0_RSARLO,0); /* Read data from the start of the receive buffer. */
   outb(iobase+EN0_RSARHI,self->n_rx_begin);
   outb(iobase+E8390_CMD,E8390_RREAD|E8390_START); /* Initiate the read. */
   insw(iobase+NE_DATAPORT,dst,low_data/2);
   /* Also read the last byte. */
   if (low_data&1) {
    u16 last = inw(iobase+NE_DATAPORT);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    dst[low_data-1] = last >> 8;
#else
    dst[low_data-1] = last & 0xff;
#endif
   }
  }

  COMPILER_WRITE_BARRIER();

  /* All right! we've got the package.
   * Now to lock the packet buffer and store it. */
  atomic_rwlock_write(&self->n_dev.n_recv_lock);
#if 0
  assert(netdev_pushok_unlocked(&self->n_dev,header.ph_size));
#else
  /* Must check the package size again in case buffer space
   * was taken away due to spoofed packages being queued by someone else. */
  if unlikely(!netdev_pushok_unlocked(&self->n_dev,header.ph_size)) {
   atomic_rwlock_endwrite(&self->n_dev.n_recv_lock);
   goto err_packet_size;
  }
#endif
  /* Finally, register the package! */
  netdev_addpck_unlocked(&self->n_dev,pck);
  atomic_rwlock_endwrite(&self->n_dev.n_recv_lock);
 }
}



DEFINE_INT_HANDLER(ne2k_irq,ne2k_int);
PRIVATE isr_t ne2k_isr = ISR_DEFAULT(NE2K_IRQ,&ne2k_irq);
INTERN void ne2k_int(void) {
 u8 status; u16 iobase; ne2k_t *dev;
 if (IRQ_PIC_SPURIOUS(NE2K_IRQ)) return;
 dev    = (ne2k_t *)ne2k_recv_job.j_data;
 iobase = dev->n_iobase;
 /* Start receiving data. (XXX: Is this really required?) */
 outb(iobase+E8390_CMD,E8390_NODMA|E8390_START);

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
   transmit_status = ETSR_PTX|ENISR_TX; /* Successful transmission. */
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

PRIVATE byte_t *KCALL
emit_package(byte_t *safe, u16 port,
             struct opacket const *__restrict packet) {
 byte_t safe_data[2],*data; size_t size;
 byte_t *my_safe = safe;
 CHECK_HOST_DOBJ(packet);
again:
 size = packet->op_size;
 if (size) {
  data = (byte_t *)packet->op_data;
  if (my_safe) {
   my_safe[1] = *data++;
   outw(port,*(u16 *)my_safe);
  }
  outsw(port,data,size/2);
  if (size&1) {
   my_safe    = safe ? safe : safe_data;
   my_safe[0] = data[size-1];
  }
 }

 /* Emit the inner package. */
 if (packet->op_wrap) {
  if (!packet->op_tsiz) {
   packet = packet->op_wrap;
   goto again;
  }
  my_safe = emit_package(my_safe,port,packet->op_wrap);
 }

 /* Emit tail data. */
 size = packet->op_tsiz;
 if (size) {
  data = (byte_t *)packet->op_tail;
  if (my_safe) {
   my_safe[1] = *data++;
   outw(port,*(u16 *)my_safe);
   my_safe = NULL;
  }
  outsw(port,data,size/2);
  if (size&1) {
   if (safe) (my_safe = safe)[1] = data[size-1];
   else outw(port,(u16)data[size-1]);
  }
 } else if (my_safe && my_safe != safe) {
  assert(!safe);
  assert(my_safe == safe_data);
  safe_data[1] = 0;
  outw(port,*(u16 *)safe_data);
  my_safe = NULL;
 }
 return my_safe;
}

#define NE2K   container_of(self,ne2k_t,n_dev)
INTERN ssize_t KCALL
net_send(struct netdev *__restrict self,
         struct opacket const *__restrict packet,
         size_t packet_size) {
 errno_t error; u16 aligned_size;
 aligned_size = (u16)packet_size;
 if (aligned_size&1) ++aligned_size;

 /* Perform a read-before-write. */
 outb(NE2K->n_iobase+E8390_CMD,E8390_PAGE0|E8390_RREAD|E8390_START);
 outb(NE2K->n_iobase+EN0_RCNTLO,1);
 outb(NE2K->n_iobase+E8390_CMD,E8390_PAGE0|E8390_NODMA|E8390_START);
 //outb(NE2K->n_iobase+EN0_ISR,ENISR_RDC); /* ??? */
 outb(NE2K->n_iobase+EN0_RCNTLO,aligned_size & 0xff);
 outb(NE2K->n_iobase+EN0_RCNTHI,aligned_size >> 8);
 outb(NE2K->n_iobase+EN0_RSARLO,0);
 outb(NE2K->n_iobase+EN0_RSARHI,NE2K->n_tx_begin);
 outb(NE2K->n_iobase+E8390_CMD,E8390_PAGE0|E8390_RWRITE|E8390_START);

 /* Output packet data. */
 emit_package(NULL,NE2K->n_iobase+NE_DATAPORT,packet);

 /* Wait for data to be acknowledged. */
 error = net_waitdma(NE2K);
 if (E_ISERR(error)) goto err;

 /* Perform the transmission. */
 outb(NE2K->n_iobase+E8390_CMD,E8390_NODMA|E8390_START);
 outb(NE2K->n_iobase+EN0_TPSR,NE2K->n_tx_begin);
 outb(NE2K->n_iobase+EN0_TCNTLO,aligned_size & 0xff);
 outb(NE2K->n_iobase+EN0_TCNTHI,aligned_size >> 8);
 outb(NE2K->n_iobase+E8390_CMD,E8390_TRANS|E8390_NODMA|E8390_START);

 error = sem_timedwait(&NE2K->n_sendend,jiffies+
                        NE2K->n_dev.n_send_timeout);
 if (E_ISERR(error)) goto err;

 /* Check if the status register indicated an error. */
 if ((NE2K->n_senderr&(ENISR_TX_ERR|ENISR_TX)) != ENISR_TX)
      return -EIO;

 return (ssize_t)packet_size;
err:
 /* Reset the card on error. */
 net_reset(NE2K);
 return error;
}
#undef NE2K

#define NE2K   container_of(self,ne2k_t,n_dev)
PRIVATE void KCALL
ne2k_fini(struct netdev *__restrict self) {
 PCI_DEVICE_DECREF(NE2K->n_pcidev);
}
#undef NE2K






PRIVATE errno_t KCALL
ne2k_probe(struct pci_device *dev) {
 ne2k_t *self; errno_t error;
 u8 prom[32]; unsigned int i;
 u16 iobase = dev->pd_resources[0].pr_begin;
 if (inb(iobase) == 0xff) return -ENODEV;

 /* Reset the card. */
 error = net_reset_base(iobase);
 if (E_ISERR(error)) return error;

 /* Execute a sequence of startup instructions. */
 { PRIVATE struct { u8 off,val; } startup[] = {
       {EN0_ISR,   0xff}, /* Acknowledge interrupts. (From before) */
       {E8390_CMD, E8390_PAGE0|E8390_STOP|E8390_NODMA},
       {EN0_DCFG,  ENDCFG_FT1|ENDCFG_LS|ENDCFG_WTS}, /* Set word-wide mode. */
       {EN0_RCNTLO,0}, /* Clear count registers. */
       {EN0_RCNTHI,0}, /* ... */
       {EN0_IMR,   0}, /* Mask completion irq. */
       {EN0_ISR,   0xff}, /* ... */
       {EN0_RXCR,  ERXCR_MON}, /* Enable monitoring mode (Don't receive packages for now). */
       {EN0_TXCR,  ETXCR_LOOPBACK_INTERN}, /* Set loopback mode to intern (Don't send packages for now). */
       /* At this point, we'll be trying to read the first 32 bytes of PROM. */
       {EN0_RCNTLO,32}, /* Setup 32 bytes for reading. */
       {EN0_RCNTHI,0},  /* ... */
       {EN0_RSARLO,0},  /* Select DMA 0x0000 for reading. */
       {EN0_RSARHI,0},  /* ... */
       {E8390_CMD, E8390_RREAD|E8390_START}, /* Execute a remote read. */
   };
   for (i = 0; i < COMPILER_LENOF(startup); ++i)
        outb(iobase+startup[i].off,startup[i].val);
 }

 /* Read PROM. */
 for (i = 0; i < 32; i++)
      prom[i] = inb(iobase+0x10);

 outb(iobase+EN0_DCFG,ENDCFG_FT1|ENDCFG_LS|ENDCFG_WTS);

 self = (ne2k_t *)netdev_new(sizeof(ne2k_t));
 if unlikely(!self) return -ENOMEM;
 memcpy(self->n_dev.n_mac.ma_bytes,prom,6);
 self->n_dev.n_send         = &net_send;
 self->n_dev.n_fini         = &ne2k_fini;
 self->n_iobase             = iobase;
 self->n_pcidev             = dev;
 self->n_dma_timeout        = NET_DEFAULT_DMATIMEOUT;
 self->n_dev.n_send_maxsize = NET_TX_PAGES*NET_PAGESIZE;

 { u16 mem_pages = 64; /* XXX: Can the chip tell me this? */
   self->n_tx_begin = NET_TX_START;
   self->n_tx_end   = NET_TX_START+NET_TX_PAGES;
   self->n_rx_begin = self->n_tx_end;
   self->n_rx_end   = NET_TX_START+mem_pages;
 }

 PCI_DEVICE_INCREF(dev);
 sem_cinit(&self->n_sendend,0);

 /* Setup an IRQ handler for incoming packages. */
 { u32 v = pci_read(dev->pd_addr,PCI_GDEV3C);
   ne2k_recv_job.j_data = self; /* TODO: This is unsafe */
   pci_write(dev->pd_addr,PCI_GDEV3C,
            (v & ~(PCI_GDEV3C_IRQLINEMASK|PCI_GDEV3C_IRQPINMASK))|
            ((NE2K_IRQ >= IRQ_PIC2_BASE ? 1 : 0) << PCI_GDEV3C_IRQPINSHIFT)|
            ((NE2K_IRQ-IRQ_PIC1_BASE) << PCI_GDEV3C_IRQLINESHIFT));
   irq_vset(BOOTCPU,&ne2k_isr,NULL,IRQ_SET_RELOAD);
 }

 /* Setup the device. */
 error = device_setup(&self->n_dev.n_dev.cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) { PCI_DEVICE_DECREF(dev); free(self); return error; }

 /* Reset the device into a working & online state. */
 error = net_reset(self);
 if (E_ISERR(error)) goto end;

 /* Register the device. */
 /* TODO: Tracking of Ethernet card numbers for dynamic allocation. */
 error = CHRDEV_REGISTER(self,DV_ETHERNET);
 if (E_ISERR(error)) {
  /* Disable the device. */
  net_reset_base(iobase);
  goto end;
 }

 /* Setup the adapter as the default network device. */
 set_default_adapter(&self->n_dev,false);

 syslog(LOG_INFO,"[NE2K] Found network card (I/O %.4I16x; MAC: %[mac])\n",
        iobase,&self->n_dev.n_mac);
 syslog(LOG_DEBUG,"%.?[hex]\n",32,prom);
end:
 NETDEV_DECREF(&self->n_dev);
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
