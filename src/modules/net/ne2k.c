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
#include <hybrid/align.h>

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
INTERN void KCALL net_reset_high(ne2k_t *__restrict dev) {
 /* Clear dangling interrupt messages. */
 ATOMIC_WRITE(dev->n_sendend.s_ticket,0);
 /* Reset buffer pointers. */
 dev->n_nextpck = dev->n_rx_begin;
 /* Don't turn the card back on if ISUP isn't set. */
 if (!(dev->n_dev.n_flags&IFF_UP)) return;
 { unsigned int i;
   struct { u8 off,val; } reset[] = {
       {EN0_ISR,   0xff}, /* Acknowledge interrupts. (From `net_reset_base()') */
       {E8390_CMD, E8390_PAGE0|E8390_STOP|E8390_NODMA},
       {EN0_DCFG,  ENDCFG_FT1|ENDCFG_LS|ENDCFG_WTS}, /* Set word-wide mode. */
       {EN0_RCNTLO,0}, /* Clear count registers. */
       {EN0_RCNTHI,0}, /* ... */
       {EN0_RXCR,  (dev->n_dev.n_flags&IFF_LOOPBACK ? ERXCR_MON : 0)|
                   (dev->n_dev.n_flags&IFF_PROMISC
                 ? (ERXCR_AR|ERXCR_AB|ERXCR_AM|ERXCR_PRO) /* Accept all packages. */
                    /* Accept broadcast packages. */
                 : (dev->n_dev.n_flags&IFF_ALLMULTI ? ERXCR_AM : 0)|
                   (dev->n_dev.n_flags&IFF_BROADCAST ? ERXCR_AB : 0))
       },
       {EN0_TXCR,  ETXCR_LOOPBACK_INTERN}, /* Disable loopback for all packages. */
       /* Configure the card's receive buffer. */
       {EN0_BOUNDARY,dev->n_nextpck},
       {EN0_STARTPG,dev->n_rx_begin},
       {EN0_STOPPG,dev->n_rx_end},
       {EN0_ISR,   0xff}, /* Acknowledge anything that may still be dangling. */
       {EN0_IMR,   ENISR_ALL}, /* Enable all the interrupts that we are handling. */
       {E8390_CMD, E8390_PAGE1|E8390_STOP|E8390_NODMA}, /* Switch to page 1. */
   };
   for (i = 0; i < COMPILER_LENOF(reset); ++i)
        outb(dev->n_iobase+reset[i].off,reset[i].val);
   for (i = 0; i < 6; ++i) /* Configure the hardware address to listen for. */
        outb(dev->n_iobase+EN1_PHYS_SHIFT(i),dev->n_dev.n_macaddr.ma_bytes[i]);
   for (i = 0; i < 8; ++i) /* XXX: Multicast? */
        outb(dev->n_iobase+EN1_MULT_SHIFT(i),0xff);
   /* Still being in page #1, set the next-package pointer. */
   outb(dev->n_iobase+EN1_CURPAG,dev->n_nextpck);

   /* Finally, initialize the card. */
   outb(dev->n_iobase+E8390_CMD,E8390_PAGE0|E8390_NODMA|E8390_START);

   /* And disable loopback, allowing for outgoing packages. */
   if (!(dev->n_dev.n_flags&IFF_LOOPBACK))
       outb(dev->n_iobase+EN0_TXCR,ETXCR_LOOPBACK_NORMAL);

   /* XXX: Only set if the ethernet cable is plugged in. */
   dev->n_dev.n_flags |= IFF_RUNNING;
 }
}
INTERN errno_t KCALL net_reset(ne2k_t *__restrict dev) {
 errno_t error = net_reset_base(dev->n_iobase);
 /* Execute the re-initialization command sequence
  * if the interface is supposed to be enabled. */
 if (E_ISOK(error)) net_reset_high(dev);
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
 /* All right! we've got the package. Now to handle it. */
 if (E_ISERR(netdev_write(&self->n_dev)))
     return; /* Shouldn't happen... */
 /* Don't process anything if the interface is supposed to be down. */
 if (!(self->n_dev.n_flags&IFF_UP)) goto end;

 for (;;) {
  STATIC_ASSERT(!(sizeof(pck_header_t) & 1));
  errno_t error; byte_t *buffer;
  pck_header_t header; u8 curr_page,packet_page;
  u16 curr_addr,high_data,low_data,aligned_size;

  /* Signal start. */
  outb(iobase+E8390_CMD,E8390_PAGE1|E8390_NODMA|E8390_START);
  /* Stop if the current read-header is where the next package will eventually be. */
  curr_page   = inb(iobase+EN1_CURPAG);
  packet_page = self->n_nextpck;
  if (curr_page == packet_page) break;
  curr_addr = curr_page*NET_PAGESIZE;

  /* Read the package header. */
  outb(iobase+E8390_CMD,E8390_PAGE0|E8390_NODMA|E8390_START);
  outb(iobase+EN0_RCNTLO,sizeof(pck_header_t)); /* Only read the header (for now). */
  outb(iobase+EN0_RCNTHI,0);
  outb(iobase+EN0_RSARLO,0); /* Start reading at the page base address. */
  outb(iobase+EN0_RSARHI,packet_page);
  outb(iobase+E8390_CMD,E8390_PAGE0|E8390_RREAD|E8390_START); /* initiate the read. */
  insw(iobase+NE_DATAPORT,&header,sizeof(pck_header_t)/2);

  if unlikely(!header.ph_size) {
   /* Shouldn't  */
   syslog(LOG_ERR,"[NE2K] Card indicates empty packet (Reset)\n");
   net_reset(self);
   goto end;
  }

  /* Inherit the next-package pointer. */
  self->n_nextpck = header.ph_nextpg;

  /* Now we know what this package looks like, we can start processing it. */
  if unlikely(header.ph_size == 0xffff) {
fail_msgsize:
   error = -EMSGSIZE;
fail:
   syslog(LOG_ERR,"[NE2K] Cannot receive package of %I16u bytes: %[errno]\n",
          header.ph_size,-error);
   continue;
  }

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
   if unlikely(low_data > curr_addr) goto fail_msgsize;
  }

  buffer = self->n_ibufv;
  if unlikely(header.ph_size > self->n_ibufa) {
   size_t new_size = CEIL_ALIGN(header.ph_size,64);
do_realloc:
   buffer = (byte_t *)realloc(buffer,new_size);
   if unlikely(!buffer) {
    if (new_size == header.ph_size) { error = -ENOMEM; goto fail; }
    new_size = header.ph_size;
    buffer = self->n_ibufv;
    goto do_realloc;
   }
   self->n_ibufv = buffer;
   self->n_ibufa = new_size;
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
   outb(iobase+EN0_RSARHI,packet_page);
   outb(iobase+E8390_CMD,E8390_RREAD|E8390_START); /* Initiate the read. */
   insw(iobase+NE_DATAPORT,buffer,high_data/2);
   /* Also read the last byte. */
   if (high_data&1) {
    u16 last = inw(iobase+NE_DATAPORT);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    buffer[high_data-1] = last >> 8;
#else
    buffer[high_data-1] = last & 0xff;
#endif
   }
  }

  /* Read low memory */
  if (low_data) {
   byte_t *dst = buffer+high_data;
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

  /* Update the boundary pointer. */
  outb(iobase+E8390_CMD,E8390_PAGE0|E8390_NODMA|E8390_START);
  if (self->n_nextpck == self->n_rx_begin)
       outb(iobase+EN0_BOUNDARY,self->n_rx_end-1);
  else outb(iobase+EN0_BOUNDARY,self->n_nextpck-1);

  COMPILER_WRITE_BARRIER();

  /* Process the package. */
  netdev_recv_unlocked(&self->n_dev,buffer,(size_t)header.ph_size);

 }
 /* Switch back to page 0. */
 outb(iobase+E8390_CMD,E8390_PAGE0|E8390_NODMA|E8390_START);
 /* Acknowledge receive interrupts. */
 outb(iobase+EN0_ISR,ENISR_RX|ENISR_RX_ERR);
 /* Re-enable all interrupts. */
 outb(iobase+EN0_IMR,ENISR_ALL);
end:
 netdev_endwrite(&self->n_dev);
}



DEFINE_INT_HANDLER(ne2k_irq,ne2k_int);
PRIVATE isr_t ne2k_isr = ISR_DEFAULT(NE2K_IRQ,&ne2k_irq);
INTERN void ne2k_do_int(ne2k_t *dev) {
 u8 status; u16 iobase;
 iobase = dev->n_iobase;
 /* Start receiving data. (XXX: Is this really required?) */
 outb(iobase+E8390_CMD,E8390_NODMA|E8390_START);

 /* Read the interrupt status register. */
 status = inb(iobase+EN0_ISR) & ENISR_ALL;
 //syslog(LOG_DEBUG,"NE2K: Interrupt %.2I8x\n",status);

 if (status & ENISR_RX) {
  /* Schedule a job to safely receive data. */
  schedule_work(&ne2k_recv_job);
  /* Since we're going to re-enable interrupts before
   * data will actually be read, we must mask all
   * interrupts caused by the chip, so-as not to deal
   * with multiple incoming packets at once. */
  outb(iobase+EN0_IMR,ENISR_ALL&~(ENISR_RX));
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
}
INTERN void ne2k_int(void) {
 if (IRQ_PIC_SPURIOUS(NE2K_IRQ)) return;
 ne2k_do_int((ne2k_t *)ne2k_recv_job.j_data);
 /* Acknowledge the interrupt within the PIC. */
 IRQ_PIC_EOI(NE2K_IRQ);
}


struct print_data {
 int  has_safe;
 u16  out_port;
 u8   out_safe[2];
};

PRIVATE void KCALL
emit_package(byte_t const *data, size_t n, void *closure) {
 struct print_data *pd = (struct print_data *)closure;
 if (pd->has_safe && n) {
  pd->has_safe = 0;
  pd->out_safe[1] = *data++,--n;
  outw(pd->out_port,*(u16 *)pd->out_safe);
 }
 outsw(pd->out_port,data,n/2);
 if (n&1) {
  pd->has_safe    = 1;
  pd->out_safe[0] = data[n-1];
 }
}

#define NE2K   container_of(self,ne2k_t,n_dev)
INTERN errno_t KCALL
net_send(struct netdev *__restrict self,
         struct opacket const *__restrict packet) {
 errno_t error; u16 aligned_size;
 aligned_size = (u16)OPACKET_SIZE(packet);
 assert(aligned_size <= self->n_send_maxsize);
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
 { struct print_data data;
   data.out_port = NE2K->n_iobase+NE_DATAPORT;
   data.has_safe = 0;
   opacket_print(packet,&emit_package,&data);
   if (data.has_safe) data.out_safe[1] = 0,
       outw(data.out_port,*(u16 *)data.out_safe);
 }

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

 return error;
err:
 /* Reset the card on error. */
 net_reset(NE2K);
 return error;
}
#undef NE2K

#define NE2K   container_of(dev,ne2k_t,n_dev.n_dev.cd_device)
INTERN void KCALL
net_irqctl(struct device *__restrict dev, unsigned int cmd) {
 u16 iobase = NE2K->n_iobase;
 switch (cmd) {
 case IRQCTL_DISABLE:
  /* Disable all interrupts raised by the card. */
  outb(iobase+EN0_IMR,0);
  break;
 case IRQCTL_ENABLE:
  /* Enable all interrupts that we handle. */
  outb(iobase+EN0_IMR,ENISR_ALL);
 case IRQCTL_TEST:
  /* Check for interrupts.
   * NOTE: There should be no race condition between this and the package recv
   *       job, as attempting to schedule an already running job is a no-op. */
  ne2k_do_int(NE2K);
  break;
 default: break;
 }
}
#undef NE2K

#define NE2K   container_of(self,ne2k_t,n_dev)
PRIVATE void KCALL
net_fini(struct netdev *__restrict self) {
 PCI_DEVICE_DECREF(NE2K->n_pcidev);
}
#undef NE2K

INTERN errno_t KCALL
net_load_mac(u16 iobase, u8 prom[32]) {
 errno_t error; unsigned int i;
 /* Reset the card. */
 error = net_reset_base(iobase);
 if (E_ISERR(error)) return error;

 /* Execute a sequence of startup instructions. */
 { PRIVATE struct { u8 off,val; } startup[] = {
       {EN0_ISR,   0xff}, /* Acknowledge interrupts. (From before) */
       {E8390_CMD, E8390_PAGE0|E8390_NODMA|E8390_STOP},
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
 //outb(iobase+EN0_DCFG,ENDCFG_FT1|ENDCFG_LS|ENDCFG_WTS);
 return error;
}
INTERN errno_t KCALL
net_reset_mac(ne2k_t *__restrict dev) {
 u8 prom[32]; errno_t error;
 /* Turn off the card. */
 error = net_load_mac(dev->n_iobase,prom);
 if (E_ISERR(error)) return error;
 memcpy(&dev->n_dev.n_macaddr,prom,sizeof(struct macaddr));
 net_reset_high(dev);
 return error;
}


PRIVATE struct netops net_ops = {
    .n_send   = &net_send,
    .n_fini   = &net_fini,
    /* Many network device operations can simply be
     * implemented using a general purpose reset. */
    .n_ifup     = (errno_t(KCALL *)(struct netdev *__restrict self))&net_reset,
    .n_ifdown   = (errno_t(KCALL *)(struct netdev *__restrict self))&net_reset,
    .n_setmac   = (errno_t(KCALL *)(struct netdev *__restrict self))&net_reset,
    .n_resetmac = (errno_t(KCALL *)(struct netdev *__restrict self))&net_reset_mac,
};


PRIVATE errno_t KCALL
ne2k_probe(struct pci_device *dev) {
 ne2k_t *self; errno_t error; u8 prom[32];
 u16 iobase = dev->pd_resources[0].pr_begin;
 if (inb(iobase) == 0xff) return -ENODEV;

 /* Reset the card and load the default mac address. */
 error = net_load_mac(iobase,prom);
 if (E_ISERR(error)) return error;

 self = (ne2k_t *)netdev_new(sizeof(ne2k_t));
 if unlikely(!self) return -ENOMEM;
 memcpy(self->n_dev.n_macaddr.ma_bytes,prom,6);
 self->n_dev.n_ops                     = &net_ops;
 self->n_dev.n_flags                  |= IFF_BROADCAST|IFF_MULTICAST;
 self->n_dev.n_dev.cd_device.d_irq_ctl = &net_irqctl;
 self->n_iobase                        = iobase;
 self->n_pcidev                        = dev;
 self->n_dma_timeout                   = NET_DEFAULT_DMATIMEOUT;
 self->n_dev.n_send_maxsize            = NET_TX_PAGES*NET_PAGESIZE;

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

 atomic_owner_rwlock_write(&irqctl_lock);

 /* Setup the device. */
 error = device_setup(&self->n_dev.n_dev.cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) {
  atomic_owner_rwlock_endwrite(&irqctl_lock);
  PCI_DEVICE_DECREF(dev);
  free(self);
  return error;
 }

 /* Reset the device into the default (off) state. */
 error = net_reset(self);
 atomic_owner_rwlock_endwrite(&irqctl_lock);

 if (E_ISERR(error)) goto end;

 /* Register the device. */
 /* TODO: Tracking of Ethernet card numbers for dynamic allocation. */
 error = CHRDEV_REGISTER(self,DV_ETHERNET);
 if (E_ISERR(error)) {
  /* Disable the device. */
  net_reset_base(iobase);
  outb(iobase+EN0_ISR,0xff); /* Acknowledge interrupts. (From `net_reset_base()') */
  goto end;
 }

 /* Setup the adapter as the default network device. */
 set_default_adapter(&self->n_dev,false);

 syslog(LOG_INFO,"[NE2K] Found network card (I/O %.4I16x; MAC: %[mac])\n",
        iobase,&self->n_dev.n_macaddr);
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
