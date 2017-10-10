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
#ifndef GUARD_MODULES_PCI_PCI_C
#define GUARD_MODULES_PCI_PCI_C 1
#define _KOS_SOURCE 2

#include <assert.h>
#include <syslog.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <hybrid/align.h>
#include <modules/pci.h>

DECL_BEGIN

/* TODO: Compatibility with Memory mapped PCI (PCI Express; Much faster than I/O ports) */
/* TODO: Compatibility with Configuration Space Access Mechanism #2 */

#define PCI_ADDR_PORT 0xcf8
#define PCI_DATA_PORT 0xcfc

PUBLIC u32 FCALL pci_readaddr(pci_addr_t addr) {
 assert(IS_ALIGNED(addr,PCI_ADDR_ALIGN));
 outl(PCI_ADDR_PORT,addr);
 return inl(PCI_DATA_PORT);
}
PUBLIC void FCALL pci_writeaddr(pci_addr_t addr, u32 value) {
 assert(IS_ALIGNED(addr,PCI_ADDR_ALIGN));
 outl(PCI_ADDR_PORT,addr);
 outl(PCI_DATA_PORT,value);
}










PUBLIC errno_t KCALL pci_enable(pci_addr_t baseaddr) {
 /* TODO */
 return -EOK;
}












/* Database functions */
PUBLIC PPCI_VENTABLE KCALL pci_db_getvendor(u16 vendor_id) {
 PPCI_VENTABLE iter,end;
 end = (iter = PciVenTable)+PCI_VENTABLE_LEN;
 for (; iter != end; ++iter) {
  if (iter->VenId == vendor_id)
      return iter;
 }
 return NULL;
}
PUBLIC PPCI_DEVTABLE KCALL pci_db_getdevice(u16 vendor_id, u16 device_id) {
 PPCI_DEVTABLE iter,end;
 end = (iter = PciDevTable)+PCI_DEVTABLE_LEN;
 for (; iter != end; ++iter) {
  if (iter->VenId == vendor_id &&
      iter->DevId == device_id)
      return iter;
 }
 return NULL;
}
PUBLIC PPCI_CLASSCODETABLE KCALL
pci_db_getclass(u8 base_class, u8 sub_class, u8 prog_if) {
 PPCI_CLASSCODETABLE iter,end;
 end = (iter = PciClassCodeTable)+PCI_CLASSCODETABLE_LEN;
 for (; iter != end; ++iter) {
  if (iter->BaseClass == base_class &&
      iter->SubClass  == sub_class &&
      iter->ProgIf    == prog_if)
      return iter;
 }
 return NULL;
}


/* PCI Discovery (Executed during module initilization, but may be re-executed later). */
PRIVATE bool KCALL register_device(pci_addr_t addr, u32 pci_dev8);
PRIVATE void KCALL check_fun(pci_addr_t addr);
PRIVATE void KCALL check_dev(pci_bus_t bus, pci_dev_t dev);
PRIVATE void KCALL check_bus(pci_bus_t bus);
PRIVATE void KCALL discover(void);


PUBLIC LIST_HEAD(struct pci_device) pci_list = NULL;
PUBLIC DEFINE_ATOMIC_RWLOCK(pci_lock);

PUBLIC struct pci_device *KCALL
pci_getdevice_at_unlocked(pci_addr_t addr) {
 struct pci_device *iter = NULL;
 PCI_FOREACH(iter) if (iter->pd_addr == addr) break;
 return iter;
}

PUBLIC REF struct pci_device *KCALL
pci_getdevice_at(pci_addr_t addr) {
 REF struct pci_device *result;
 atomic_rwlock_read(&pci_lock);
 result = pci_getdevice_at_unlocked(addr);
 /* Return a reference to the caller. */
 if (result) PCI_DEVICE_INCREF(result);
 atomic_rwlock_endread(&pci_lock);
 return result;
}

PRIVATE ATTR_FREETEXT bool KCALL
register_device(pci_addr_t addr, u32 pci_dev8) {
 REF struct pci_device *device;
 /* Make sure that the device wasn't already registered. */
 if (pci_getdevice_at_unlocked(addr))
     return false;
 device = (REF struct pci_device *)malloc(sizeof(struct pci_device));
 if unlikely(!device) return false;
 device->pd_refcnt = 1;
 device->pd_addr   = addr;
 device->pd_dev0   = pci_read(addr,PCI_DEV0);
 device->pd_dev8   = pci_dev8;
 device->pd_vendor = pci_db_getvendor(device->pd_vendorid);
 device->pd_device = pci_db_getdevice(device->pd_vendorid,device->pd_deviceid);
 device->pd_class  = pci_db_getclass(device->pd_classid,device->pd_subclassid,device->pd_progif);
 LIST_INSERT(pci_list,device,pd_link);

 syslog(LOG_DEBUG,FREESTR("[PCI] Device at %I32p (Vendor: %s; Device: %s; Class: %s,%s,%s)\n"),
        addr,
        device->pd_vendor ? device->pd_vendor->VenShort : FREESTR("?"),
        device->pd_device ? device->pd_device->Chip : FREESTR("?"),
        device->pd_class  ? device->pd_class->BaseDesc : FREESTR("?"),
        device->pd_class  ? device->pd_class->SubDesc : FREESTR("?"),
        device->pd_class  ? device->pd_class->ProgDesc : FREESTR("?"));
 return true;
}


PRIVATE ATTR_FREETEXT void KCALL check_fun(pci_addr_t addr) {
 u32 resp = pci_read(addr,PCI_DEV8);
 if (!register_device(addr,resp)) return;
 if (PCI_DEV8_CLASS(resp)    == PCI_DEV8_CLASS_BRIDGE &&
     PCI_DEV8_SUBCLASS(resp) == PCI_DEV8_CLASS_MUMEDIA) {
  /* Recursively enumerate a bridge device. */
  check_bus(PCI_BDEV18_SECONDARY_BUS(pci_read(addr,PCI_BDEV18)));
 }
}
PRIVATE ATTR_FREETEXT void KCALL check_dev(pci_bus_t bus, pci_dev_t dev) {
 pci_addr_t addr = PCI_ADDR(bus,dev,0);
 if (PCI_DEV0_VENDOR(pci_read(addr,PCI_DEV0)) == PCI_DEV0_VENDOR_NODEV)
     return;
 check_fun(addr);
 if (PCI_DEVC_HEADER(pci_read(addr,PCI_DEVC)) &
     PCI_DEVC_HEADER_MULTIDEV) {
  u8 i; /* Recursively check secondary functions of a multi-device. */
  for (i = 0; i < PCI_ADDR_FUNCOUNT; ++i,addr += 1 << PCI_ADDR_FUNSHIFT) {
   if (PCI_DEV0_VENDOR(pci_read(addr,PCI_DEV0)) == PCI_DEV0_VENDOR_NODEV)
       continue;
   check_fun(addr);
  }
 }
}
PRIVATE ATTR_FREETEXT void KCALL check_bus(pci_bus_t bus) {
 u8 device;
 for (device = 0; device < 32; device++)
      check_dev(bus,device);
}

PRIVATE ATTR_FREETEXT void KCALL discover(void) {
#if 1
 /* Recursive scanner. */
 u32 resp = pci_read(PCI_ADDR(0,0,0),PCI_DEVC);
 if (PCI_DEVC_HEADER(resp)&PCI_DEVC_HEADER_MULTIDEV) {
  /* Single controller. */
  check_bus(0);
 } else {
  /* Multiple controllers. */
  pci_fun_t fun = 0;
  for (; fun < PCI_ADDR_FUNCOUNT; ++fun) {
   pci_addr_t addr = PCI_ADDR(0,0,fun);
   if (PCI_DEV0_VENDOR(pci_read(addr,PCI_DEV0)) == PCI_DEV0_VENDOR_NODEV)
       return;
   check_bus(addr);
  }
 }
#else
 /* Brute-force scanner (TODO: Add a commandline option for this). */
 u8 bus = 0;
 for (;;) {
  check_bus(bus);
  if (bus == 0xff) break;
  ++bus;
 }
#endif
}

PRIVATE MODULE_INIT void KCALL pci_init(void) {
 /* TODO: Check if PCI is even available? */
 /* TODO: Check which Configuration Space Access Mechanism can/must be used. */
 /* Perform a simple PCI device discovery. */
 discover();
}
PRIVATE MODULE_FINI void KCALL pci_fini(void) {
 struct pci_device *next,*iter = pci_list;
 /* Free up all registered PCI devices. */
 while (iter) {
  next = iter->pd_link.le_next;
  PCI_DEVICE_DECREF(iter);
  iter = next;
 }
}

DECL_END

#endif /* !GUARD_MODULES_PCI_PCI_C */
