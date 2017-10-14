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
#ifndef GUARD_MODULES_NET_NE2K_H
#define GUARD_MODULES_NET_NE2K_H 1
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#include <kernel/export.h>
#include <dev/net.h>
#include <modules/pci.h>
#include <sync/sem.h>

DECL_BEGIN

/* Some of the definitions and macros below are taken from
 * linux sources: "/drivers/net/ethernet/8390/8390.h"
 * The comment below can be found in the original source.
 * NOTICE: Changes, as well as additions have been made!
 */
/* This file is part of Donald Becker's 8390 drivers, and is distributed
   under the same license. Auto-loading of 8390.o only in v2.2 - Paul G.
   Some of these names and comments originated from the Crynwr
   packet drivers, which are distributed under the GPL. */


#ifndef EI_SHIFT
#define EI_SHIFT(x)    (x)
#endif

#define E8390_CMD    EI_SHIFT(0x00) /* The command register (for all pages) */

/* Page 0 register offsets. */
#define EN0_CLDALO   EI_SHIFT(0x01) /* Low byte of current local dma addr  RD */
#define EN0_STARTPG  EI_SHIFT(0x01) /* Starting page of ring bfr WR */
#define EN0_CLDAHI   EI_SHIFT(0x02) /* High byte of current local dma addr  RD */
#define EN0_STOPPG   EI_SHIFT(0x02) /* Ending page +1 of ring bfr WR */
#define EN0_BOUNDARY EI_SHIFT(0x03) /* Boundary page of ring bfr RD WR */
#define EN0_TSR      EI_SHIFT(0x04) /* Transmit status reg RD */
#define EN0_TPSR     EI_SHIFT(0x04) /* Transmit starting page WR */
#define EN0_NCR      EI_SHIFT(0x05) /* Number of collision reg RD */
#define EN0_TCNTLO   EI_SHIFT(0x05) /* Low  byte of tx byte count WR */
#define EN0_FIFO     EI_SHIFT(0x06) /* FIFO RD */
#define EN0_TCNTHI   EI_SHIFT(0x06) /* High byte of tx byte count WR */
#define EN0_ISR      EI_SHIFT(0x07) /* Interrupt status reg RD WR */
#define EN0_CRDALO   EI_SHIFT(0x08) /* low byte of current remote dma address RD */
#define EN0_RSARLO   EI_SHIFT(0x08) /* Remote start address reg 0 */
#define EN0_CRDAHI   EI_SHIFT(0x09) /* high byte, current remote dma address RD */
#define EN0_RSARHI   EI_SHIFT(0x09) /* Remote start address reg 1 */
#define EN0_RCNTLO   EI_SHIFT(0x0a) /* Remote byte count reg WR */
#define EN0_RCNTHI   EI_SHIFT(0x0b) /* Remote byte count reg WR */
#define EN0_RSR      EI_SHIFT(0x0c) /* rx status reg RD */
#define EN0_RXCR     EI_SHIFT(0x0c) /* RX configuration reg WR */
#define EN0_TXCR     EI_SHIFT(0x0d) /* TX configuration reg WR */
#define EN0_COUNTER0 EI_SHIFT(0x0d) /* Rcv alignment error counter RD */
#define EN0_DCFG     EI_SHIFT(0x0e) /* Data configuration reg WR */
//#define //T1 FT0 ARM LS LAS BOS WTS 
#define EN0_COUNTER1 EI_SHIFT(0x0e) /* Rcv CRC error counter RD */
#define EN0_IMR      EI_SHIFT(0x0f) /* Interrupt mask reg WR */
#define EN0_COUNTER2 EI_SHIFT(0x0f) /* Rcv missed frame error counter RD */

/* Page 1 register offsets. */
#define EN1_PHYS          EI_SHIFT(0x01) /*< This board's physical enet addr RD WR. */
#define EN1_PHYS_SHIFT(i) EI_SHIFT(i+1)  /*< Get and set mac address. */
#define EN1_CURPAG        EI_SHIFT(0x07) /*< Current memory page RD WR. */
#define EN1_MULT          EI_SHIFT(0x08) /*< Multicast filter mask array (8 bytes) RD WR. */
#define EN1_MULT_SHIFT(i) EI_SHIFT(8+i)  /*< Get and set multicast filter .*/


#define NE_DATAPORT  EI_SHIFT(0x10) /* NatSemi-defined port window offset. */
#define NE_RESET     EI_SHIFT(0x1f) /* Issue a read to reset, a write to clear. */


/* Register accessed at EN_CMD, the 8390 base addr. */
#define E8390_STOP   0x01 /* Stop and reset the chip */
#define E8390_START  0x02 /* Start the chip, clear reset */
#define E8390_TRANS  0x04 /* Transmit a frame */
#define E8390_RREAD  0x08 /* Remote read */
#define E8390_RWRITE 0x10 /* Remote write  */
#define E8390_NODMA  0x20 /* Remote DMA */
#define E8390_PAGE0  0x00 /* Select page chip registers */
#define E8390_PAGE1  0x40 /* using the two high-order bits */
#define E8390_PAGE2  0x80 /* Page 3 is invalid. */

/* Bits in EN0_ISR/EN0_IMR - Interrupt status register */
#define ENISR_RX       0x01 /* Receiver, no error */
#define ENISR_TX       0x02 /* Transmitter, no error */
#define ENISR_RX_ERR   0x04 /* Receiver, with error */
#define ENISR_TX_ERR   0x08 /* Transmitter, with error */
#define ENISR_OVER     0x10 /* Receiver overwrote the ring */
#define ENISR_COUNTERS 0x20 /* Counters need emptying */
#define ENISR_RDC      0x40 /* remote dma complete */
#define ENISR_RESET    0x80 /* Reset completed */
#define ENISR_ALL      0x3f /* Interrupts we will enable */

/* Additions, derived from Realtek documentation found here:
 * >> http://www.ethernut.de/pdf/8019asds.pdf  */

/* Bits in EN0_DCFG - Data Configuration Register. */
#define ENDCFG_WTS 0x01 /*< Word-transfer select (0: byte-wide, 1: word-wide) */
#define ENDCFG_BOS 0x02 /*< Byte order select (Leave at ZERO) */
#define ENDCFG_LAS 0x04 /*< Leave at ZERO */
#define ENDCFG_LS  0x08 /*< Lookback Select. (Set of 1 for Normal Operation) */
#define ENDCFG_ARM 0x10 /*< Auto-initialize Remote. (1: Send Packet Command executed) */
#define ENDCFG_FT0 0x20 /*< FIFO threshold select bit 0. */
#define ENDCFG_FT1 0x40 /*< FIFO threshold select bit 1. */

/* Bits in EN0_RXCR - Receive Configuration Register. */
#define ERXCR_SEP  0x01 /* Accept packages with receive errors. */
#define ERXCR_AR   0x02 /* Accept packages smaller than 64 bytes. */
#define ERXCR_AB   0x04 /* Accept broadcast packages. */
#define ERXCR_AM   0x08 /* Accept multicast packages. */
#define ERXCR_PRO  0x10 /* Accept all packets with physical destination address. */
#define ERXCR_MON  0x20 /* Check incoming packages, but don't buffer them. */

/* Bits in EN0_TXCR - Transmit Configuration Register. */
#define ETXCR_CRC  0x01 /*< Set to inhibit CRC within the transmitter. */
#define ETXCR_LB0  0x02 /*< Loopback mode bit 0. */
#define ETXCR_LB1  0x04 /*< Loopback mode bit 1. */
#   define ETXCR_LOOPBACK_NORMAL   0x00
#   define ETXCR_LOOPBACK_INTERN   ETXCR_LB0
#   define ETXCR_LOOPBACK_EXTERN   ETXCR_LB1
#   define ETXCR_LOOPBACK_EXTERN2 (ETXCR_LB0|ETXCR_LB1) /* Same behavior as 'ETXCR_LOOPBACK_EXTERN' */
#define ETXCR_ATD  0x08 /* Auto Transmit Disable. (Leave at ZERO) */
#define ETXCR_OFST 0x10 /* Collision Offset Enable. */


/* Bits in EN0_TSR - Transmit status Register. */
#define ETSR_OWC    0x80 /*< ERROR: Out of Window Collision. */
#define ETSR_CHD    0x40 /*< ERROR: CD Heartbeat. (I think this bit indicates a collision?) */
#define ETSR_CRS    0x10 /*< ERROR: Carrier is lost during packet transmission. (Whatever that means...) */
#define ETSR_ABT    0x08 /*< ERROR: Transmission aborted due to excessive collisions. */
#define ETSR_COL    0x04 /*< ERROR: Collision with some other station on the network. */
#define ETSR_PTX    0x01 /*< OK: Transmission completed with no errors. */
#define ETSR_EMASK (ETSR_OWC|ETSR_CHD|ETSR_CRS|ETSR_ABT|ETSR_COL) /*< Mask of all error bits. */


#define NET_PAGESIZE 256
typedef struct ne2k ne2k_t;
struct ne2k {
 struct netdev          n_dev;     /*< Underlying net device. */
 REF struct pci_device *n_pcidev;  /*< [const] Associated PCI device. */
 u16                    n_iobase;  /*< [const] I/O Base address. */
 u8                     n_nextpck; /*< [lock(IN_IRQ)] Next package page pointer. */
 u8                     n_senderr; /*< [lock(n_dev.n_send_lock)] Error code after transmission completion (Set of 'ETSR_*'). */
 sem_t                  n_sendend; /*< [lock(n_dev.n_send_lock)] Semaphore used to notify transmission completion. */
};

INTERN errno_t KCALL net_reset_base(u16 iobase); /* Reset the card. */
INTDEF errno_t KCALL net_reset(u16 iobase); /* Reset the card and interrupts. */

/* Wait for DMA completion, returning an error or the status register. */
INTDEF s32 KCALL net_waitdma(ne2k_t *__restrict dev);
INTDEF errno_t KCALL net_resetdev(ne2k_t *__restrict dev);

DECL_END

#endif /* !GUARD_MODULES_NET_NE2K_H */
