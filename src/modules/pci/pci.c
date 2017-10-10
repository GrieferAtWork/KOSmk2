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
#include <string.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <kernel/malloc.h>
#include <hybrid/align.h>
#include <modules/pci.h>
#include <kernel/iomgr.h>
#include <kernel/memory.h>

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










PUBLIC errno_t KCALL
pci_enable(struct pci_device *__restrict dev) {
 /* TODO? */
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
PRIVATE struct pci_device *KCALL register_device(pci_addr_t addr, u32 pci_dev8);
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

PRIVATE bool KCALL
pci_alloc_resource(struct pci_device *__restrict dev,
                   struct pci_resource *__restrict res) {
 /* Allocate PCI resources (memory and I/O ranges). */
 if (res->pr_flags&PCI_RESOURCE_IO) {
  /* Try to allocate I/O space at the pre-configured address. */
  if (res->pr_begin+res->pr_size   >= res->pr_begin &&
      res->pr_begin+res->pr_size-1 <= (ioport_t)-1 &&
      E_ISOK(io_malloc_at((ioport_t)res->pr_begin,(iosize_t)res->pr_size,THIS_INSTANCE)));
  else {
   ioaddr_t addr; /* If that failed, allocate at the default address. */
   addr = io_malloc((ioport_t)res->pr_size,(ioport_t)-1,THIS_INSTANCE);
   if (E_ISERR(addr)) {
    syslog(LOG_DEBUG,FREESTR("[PCI] Failed to allocate %Iu bytes of I/O address space for BAR #%d of PCI device at %p\n"),
           res->pr_size,(int)(res-dev->pd_resources),dev->pd_addr);
    /* Delete the BAR to hide our failure. */
    res->pr_flags = 0;
    return false;
   } else {
    res->pr_begin = (PHYS uintptr_t)addr;
   }
  }
 } else if (res->pr_flags&PCI_RESOURCE_MEM) {
  /* Allocate physical memory for the device. */
  ppage_t addr;
  addr = page_memalign(res->pr_align,res->pr_size,PAGEATTR_NONE,
                       res->pr_flags&PCI_RESOURCE_MEM16 ? MZONE_1MB : MZONE_ANY);
  if (addr == PAGE_ERROR) {
   syslog(LOG_DEBUG,FREESTR("[PCI] Failed to allocate %Iu bytes of physical memory for BAR #%d of PCI device at %p\n"),
          res->pr_size,(int)(res-dev->pd_resources),dev->pd_addr);
   /* Delete the BAR to hide our failure. */
   res->pr_flags = 0;
   return false;
  } else {
   /* Assign memory. */
   res->pr_begin = (PHYS uintptr_t)addr;
  }
 } else {
  return false;
 }
 return true;
}


PRIVATE ATTR_FREETEXT struct pci_device *KCALL
register_device(pci_addr_t addr, u32 pci_dev8) {
 REF struct pci_device *result;
 int i; struct pci_resource *iter;
 u32 maxsize,state;
 /* Make sure that the device wasn't already registered. */
 if (pci_getdevice_at_unlocked(addr))
     return NULL;
 result = (REF struct pci_device *)kcalloc(sizeof(struct pci_device),GFP_NORMAL);
 if unlikely(!result) return NULL;
 result->pd_refcnt = 1;
 result->pd_addr   = addr;
 result->pd_dev0   = pci_read(addr,PCI_DEV0);
 result->pd_dev8   = pci_dev8;
 result->pd_devc   = pci_read(addr,PCI_DEVC);
 result->pd_vendor = pci_db_getvendor(result->pd_vendorid);
 result->pd_device = pci_db_getdevice(result->pd_vendorid,result->pd_deviceid);
 result->pd_class  = pci_db_getclass(result->pd_classid,result->pd_subclassid,result->pd_progif);

 /* Read device-specific configuration fields. */
 iter = result->pd_resources;
 switch (result->pd_header) {

 case PCI_DEVC_HEADER_GENERIC:
  for (i = 0; i <= 6; ++i,++iter) {
   pci_reg_t reg = i == 6 ? PCI_GDEV_EXPROM : PCI_GDEV_BAR0+i*(PCI_GDEV_BAR1-PCI_GDEV_BAR0);
   state = pci_read(addr,reg);
   if (!(state&~0xf)) continue; /* Unused. */
   pci_write(addr,reg,(u32)-1);
   maxsize = pci_read(addr,reg);
   if (state&1) {
    /* I/O range. */
    iter->pr_begin  = state&~0x3;
    iter->pr_size   = (~(maxsize&~0x3))+1;
    iter->pr_flags |= PCI_RESOURCE_IO;
   } else {
    /* Memory range. */
    iter->pr_begin  = state&~0xf;
    iter->pr_size   = (~(maxsize&~0xf))+1;
    iter->pr_flags |= PCI_RESOURCE_MEM;
    iter->pr_flags |= (state&0x6); /* Memory type. */
    /* A 64-bit memory range takes up 2 BARs. */
    if (iter->pr_flags&PCI_RESOURCE_MEM64 && !(i&1) &&
        reg <= PCI_GDEV_BAR5) ++i,++iter;
   }
   if (iter->pr_size)
       iter->pr_align = 1 << (ffs(iter->pr_size)-1);
   else iter->pr_flags = 0; /* Ignore this case */
   /* Allocate PCI resources */
   if (pci_alloc_resource(result,iter)) {
    state = (u32)((state&1)|iter->pr_begin);
#if __SIZEOF_POINTER__ > 4
    if (iter->pr_flags&PCI_RESOURCE_MEM64 && !(i&1) &&
        reg <= PCI_GDEV_BAR5) {
     pci_write(addr,reg+(PCI_GDEV_BAR1-PCI_GDEV_BAR0),
              (u32)((state&1)|(iter->pr_begin >> 32)));
    }
#endif
   }
   pci_write(addr,reg,state);
  }
  break;

 case PCI_DEVC_HEADER_BRIDGE:
  for (i = 0; i < 2; ++i,++iter) {
   pci_reg_t reg = PCI_BDEV_BAR0+i*(PCI_BDEV_BAR1-PCI_BDEV_BAR0);
   state = pci_read(addr,reg);
   if (!(state&~0xf)) continue; /* Unused. */
   pci_write(addr,reg,(u32)-1);
   maxsize = pci_read(addr,reg);
   if (state&1) {
    /* I/O range. */
    iter->pr_begin  = state&~0x3;
    iter->pr_size   = (~(maxsize&0x3))+1;
    iter->pr_flags |= PCI_RESOURCE_IO;
   } else {
    /* Memory range. */
    iter->pr_begin  = state&~0xf;
    iter->pr_size   = (~(maxsize&0xf))+1;
    iter->pr_flags |= PCI_RESOURCE_MEM;
    iter->pr_flags |= (state&0x6); /* Memory type. */
    /* A 64-bit memory range takes up 2 BARs. */
    if (iter->pr_flags&PCI_RESOURCE_MEM64 && !(i&1)) ++i,++iter;
   }
   if (iter->pr_size)
       iter->pr_align = 1 << (ffs(iter->pr_size)-1);
   /* Allocate PCI resources */
   if (pci_alloc_resource(result,iter)) {
    state = (u32)((state&1)|iter->pr_begin);
#if __SIZEOF_POINTER__ > 4
    if (iter->pr_flags&PCI_RESOURCE_MEM64 && !(i&1) &&
        reg <= PCI_GDEV_BAR5) {
     pci_write(addr,reg+(PCI_BDEV_BAR1-PCI_BDEV_BAR0),
              (u32)((state&1)|(iter->pr_begin >> 32)));
    }
#endif
   }
   pci_write(addr,reg,state);
  }
  break;

 case PCI_DEVC_HEADER_CARDBUS:
  /* XXX: 'PCI_CDEV_MEMBASE0' and friends? */
  break;

 default: break;
 }

 

 LIST_INSERT(pci_list,result,pd_link);

 syslog(LOG_DEBUG,FREESTR("[PCI] Device at %I32p (Vendor: %s; Device: %s; Class: %s,%s,%s)\n"),
        addr,
        result->pd_vendor ? result->pd_vendor->VenShort : FREESTR("?"),
        result->pd_device ? result->pd_device->Chip : FREESTR("?"),
        result->pd_class  ? result->pd_class->BaseDesc : FREESTR("?"),
        result->pd_class  ? result->pd_class->SubDesc : FREESTR("?"),
        result->pd_class  ? result->pd_class->ProgDesc : FREESTR("?"));
 return result;
}

PUBLIC struct pci_resource *KCALL
pci_find_iobar(struct pci_device *__restrict dev) {
 struct pci_resource *result = dev->pd_resources;
 for (; result <= dev->pd_resources+PD_RESOURCE_BAR5; ++result) {
  if (result->pr_flags&PCI_RESOURCE_IO) return result;
 }
 return NULL;
}
PUBLIC struct pci_resource *KCALL
pci_find_membar(struct pci_device *__restrict dev) {
 struct pci_resource *result = dev->pd_resources;
 for (; result <= dev->pd_resources+PD_RESOURCE_BAR5; ++result) {
  if (result->pr_flags&PCI_RESOURCE_MEM) return result;
 }
 return NULL;
}


PRIVATE ATTR_FREETEXT void KCALL check_fun(pci_addr_t addr) {
 u32 resp = pci_read(addr,PCI_DEV8);
 struct pci_device *dev = register_device(addr,resp);
 if (!dev) return;
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
