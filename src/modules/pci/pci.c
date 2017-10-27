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






/* Database functions */
PUBLIC struct db_vendor const *KCALL
pci_db_getvendor(u16 vendor_id) {
 struct db_vendor const *iter,*end;
 struct db_vendorbucket const *bucket;
 /* Lookup the vendor associated with the given ID. */
 assertf(DB_VENDOR_BUCKETS,"The database is empty?");
 bucket = &db_vendor_hashmap[DB_HASH_VENDOR(vendor_id) % DB_VENDOR_BUCKETS];
 end = (iter = DB_VENDORBUCKET_VENDORV(bucket))+bucket->vb_vendorc;
 for (; iter != end; ++iter) {
  assert((DB_HASH_VENDOR(iter->v_pci_id) % DB_VENDOR_BUCKETS) ==
         (DB_HASH_VENDOR(vendor_id) % DB_VENDOR_BUCKETS));
  if (iter->v_pci_id == vendor_id)
      return iter;
 }
 return NULL;
}
PUBLIC struct db_device const *KCALL
pci_db_getdevice(struct db_vendor const *vendor, u16 device_id) {
 struct db_devicebucket const *bucket;
 struct db_device const *iter,*end;
 /* Make sure the given vendor is valid and has produced devices. */
 if unlikely(!vendor || !vendor->v_device_bucket_count) return NULL;
 /* Lookup a device produced by the given vendor. */
 bucket = &DB_VENDOR_DEVICE_BUCKETS(vendor)[DB_HASH_DEVICE(device_id) %
                                            vendor->v_device_bucket_count];
 end = (iter = DB_DEVICEBUCKET_DEVICEV(bucket))+bucket->db_devicec;
 for (; iter != end; ++iter) {
  assert(iter->d_pci_vendor_id == vendor->v_pci_id);
  assert((DB_HASH_DEVICE(iter->d_pci_device_id) % vendor->v_device_bucket_count) ==
         (DB_HASH_DEVICE(device_id) % vendor->v_device_bucket_count));
  if (iter->d_pci_device_id == device_id)
      return iter;
 }
 return NULL;
}

#ifndef CONFIG_NO_PCI_CLASSES
PUBLIC struct db_class const *KCALL pci_db_getclass(u8 classid) {
 struct db_class const *iter,*end;
 end = (iter = db_classes)+DB_CLASSES_COUNT;
 for (; iter != end; ++iter) {
  if (iter->c_class_id >= classid) {
   if (iter->c_class_id == classid)
       return iter;
   break;
  }
 }
 return NULL;
}
PUBLIC struct db_subclass const *KCALL
pci_db_getsubclass(struct db_class const *class_, u8 subclassid) {
 struct db_subclass const *iter,*end;
 if (!class_) return NULL;
 end = (iter = DB_CLASS_SUBCLASSES(class_))+class_->c_subclass_count;
 for (; iter != end; ++iter) {
  assert(DB_SUBCLASS_CLASS(iter) == class_);
  if (iter->sc_subclass_id >= subclassid) {
   if (iter->sc_subclass_id == subclassid)
       return iter;
   break;
  }
 }
 return NULL;
}
PUBLIC struct db_progif const *KCALL
pci_db_getprogif(struct db_subclass const *subclass, u8 progifid) {
 struct db_progif const *iter,*end;
 if (!subclass) return NULL;
 end = (iter = DB_SUBCLASS_PROGIFS(subclass))+subclass->sc_progif_count;
 for (; iter != end; ++iter) {
  assert(DB_PROGIF_SUBCLASS(iter) == subclass);
  if (iter->pi_progif_id >= progifid) {
   if (iter->pi_progif_id == progifid)
       return iter;
   break;
  }
 }
 return NULL;
}
#endif /* !CONFIG_NO_PCI_CLASSES */



/* PCI Discovery (Executed during module initialization, but may be re-executed later). */
PRIVATE struct pci_device *KCALL register_device(pci_addr_t addr);
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
#if 0
 return false;
#else
 /* Allocate PCI resources (memory and I/O ranges). */
 if (res->pr_flags&PCI_RESOURCE_IO) {
  ioaddr_t addr;
  /* Try to allocate I/O space at the pre-configured address. */
  if (res->pr_begin+res->pr_size   >= res->pr_begin &&
      res->pr_begin+res->pr_size-1 <= (ioport_t)-1 &&
      E_ISOK(io_malloc_at((ioport_t)res->pr_begin,(iosize_t)res->pr_size,THIS_INSTANCE)))
      return false; /* No need to update */
  /* If that failed, allocate a new range. */
  addr = io_malloc((iosize_t)res->pr_align,
                   (iosize_t)res->pr_size,
                   (ioport_t)-1,THIS_INSTANCE);
  if (E_ISERR(addr)) {
   syslog(LOG_DEBUG,FREESTR("[PCI] Failed to allocate %Iu bytes of I/O address space for BAR #%d of PCI device at %p\n"),
          res->pr_size,(int)(res-dev->pd_resources),dev->pd_addr);
   /* Delete the BAR to hide our failure. */
   res->pr_flags = 0;
   return false;
  } else {
   res->pr_begin = (PHYS uintptr_t)addr;
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
#endif
}


PRIVATE ATTR_FREETEXT struct pci_device *KCALL
register_device(pci_addr_t addr) {
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
 result->pd_dev8   = pci_read(addr,PCI_DEV8);
 result->pd_devc   = pci_read(addr,PCI_DEVC);
 result->pd_vendor = pci_db_getvendor(result->pd_vendorid);
 result->pd_device = pci_db_getdevice(result->pd_vendor,result->pd_deviceid);
#ifndef CONFIG_NO_PCI_CLASSES
 result->pd_class    = pci_db_getclass(result->pd_classid);
 result->pd_subclass = pci_db_getsubclass(result->pd_class,result->pd_subclassid);
 result->pd_progif   = pci_db_getprogif(result->pd_subclass,result->pd_progifid);
#endif /* !CONFIG_NO_PCI_CLASSES */

 /* Read device-specific configuration fields. */
 iter = result->pd_resources;
 switch (result->pd_header) {

 case PCI_DEVC_HEADER_GENERIC:
  for (i = 0; i < 6; ++i,++iter) {
   pci_reg_t reg = PCI_GDEV_BAR0+i*(PCI_GDEV_BAR1-PCI_GDEV_BAR0);
   state = pci_read(addr,reg);
   if (!(state&~0xf)) continue; /* Unused. */
   pci_write(addr,reg,(u32)-1);
   maxsize = pci_read(addr,reg);
   //syslog(LOG_DEBUG,FREESTR("state = %p/%p\n"),state,maxsize);
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
    if (iter->pr_flags&PCI_RESOURCE_MEM64 && !(i&1)) ++i,++iter;
   }
   if (iter->pr_size)
       iter->pr_align = 1 << (ffs(iter->pr_size)-1);
   else iter->pr_flags = 0; /* Ignore this case */
   /* Allocate PCI resources */
   if (reg <= PCI_GDEV_BAR5 && pci_alloc_resource(result,iter)) {
    state = (u32)((state&1)|iter->pr_begin);
#if __SIZEOF_POINTER__ > 4
    if (iter->pr_flags&PCI_RESOURCE_MEM64 && !(i&1)) {
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
  /* XXX: `PCI_CDEV_MEMBASE0' and friends? */
  break;

 default: break;
 }

 LIST_INSERT(pci_list,result,pd_link);
#ifndef CONFIG_NO_PCI_CLASSES
 syslog(LOG_DEBUG,FREESTR("[PCI] Device at %I32p - %.4I16x:%.4I16x (Vendor: %s; Device: %s; Class: %s:%s:%s)\n"),
        addr,result->pd_vendorid,result->pd_deviceid,
        result->pd_vendor ? DB_VENDOR_NAME(result->pd_vendor) : FREESTR("?"),
        result->pd_device ? DB_DEVICE_NAME(result->pd_device) : FREESTR("?"),
        result->pd_class ? DB_CLASS_NAME(result->pd_class) : FREESTR("?"),
        result->pd_subclass ? DB_SUBCLASS_NAME(result->pd_subclass) : FREESTR("?"),
        result->pd_progif ? DB_PROGIF_NAME(result->pd_progif) : FREESTR("?"));
#else
 syslog(LOG_DEBUG,FREESTR("[PCI] Device at %I32p - %.4I16x:%.4I16x (Vendor: %s; Device: %s)\n"),
        addr,result->pd_vendorid,result->pd_deviceid,
        result->pd_vendor ? DB_VENDOR_NAME(result->pd_vendor) : FREESTR("?"),
        result->pd_device ? DB_DEVICE_NAME(result->pd_device) : FREESTR("?"));
#endif
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
 struct pci_device *dev = register_device(addr);
 if (!dev) return;
 if (dev->pd_classid == PCI_DEV8_CLASS_BRIDGE/* &&
     dev->pd_subclassid == PCI_DEV8_CLASS_MUMEDIA*/) {
  //syslog(LOG_DEBUG,"ENTER_BRIDGE\n");
  /* Recursively enumerate a bridge device. */
  check_bus(PCI_BDEV18_SECONDARY_BUS(pci_read(addr,PCI_BDEV18)));
  //syslog(LOG_DEBUG,"LEAVE_BRIDGE\n");
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
