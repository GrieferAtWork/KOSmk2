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
#ifndef GUARD_INCLUDE_MODULES_PCI_H
#define GUARD_INCLUDE_MODULES_PCI_H 1

#include <assert.h>
#include <bits/endian.h>
#include <hybrid/byteorder.h>
#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>
#include <kernel/export.h>
#include <errno.h>
#include <malloc.h>
#include <sys/io.h>

/* NOTE: Using this header requires you to link against `/mod/libpci.so' */

DECL_BEGIN

typedef u8 pci_bus_t; /* PCI Bus number. */
typedef u8 pci_dev_t; /* PCI Device number. (Aka. `slot') */
typedef u8 pci_fun_t; /* PCI Function number. */
typedef u8 pci_reg_t; /* PCI Register number (Lowest 2 bits are always ZERO). */
#define PCI_ADDR_ENABLED  0x80000000 /* Address-enabled bit (Must always be set) */
#define PCI_ADDR_BUSMASK  0x00ff0000
#define PCI_ADDR_DEVMASK  0x0000f800
#define PCI_ADDR_FUNMASK  0x00000700
#define PCI_ADDR_FUNCOUNT 8
#define PCI_ADDR_REGMASK  0x000000ff /* Technically `0x000000fc', but lowest 2 bits must always be ZERO. */
#define PCI_ADDR_BUSSHIFT 16
#define PCI_ADDR_DEVSHIFT 11
#define PCI_ADDR_FUNSHIFT 8
#define PCI_ADDR_REGSHIFT 0 /* Assumed to always be ZERO(0). */
#define PCI_ADDR_ALIGN    4 /* 4-byte alignment to always clear lower 2 bits. */

typedef u32 pci_addr_t; /* PCI Address (Following configuration space mechanism #1) */

/* Build a PCI address using the given arguments.
 * The register number to-be read may then be or'd to the generated number. */
#define PCI_ADDR(bus,dev,fun) \
  ((pci_addr_t)(bus) << PCI_ADDR_BUSSHIFT | \
   (pci_addr_t)(dev) << PCI_ADDR_DEVSHIFT | \
   (pci_addr_t)(fun) << PCI_ADDR_FUNSHIFT | \
    PCI_ADDR_ENABLED)
#define PCI_ADDR_GETBUS(x) (((x)&PCI_ADDR_BUSMASK) >> PCI_ADDR_BUSSHIFT)
#define PCI_ADDR_GETDEV(x) (((x)&PCI_ADDR_DEVMASK) >> PCI_ADDR_DEVSHIFT)
#define PCI_ADDR_GETFUN(x) (((x)&PCI_ADDR_FUNMASK) >> PCI_ADDR_FUNSHIFT)
#define PCI_ADDR_GETREG(x)  ((x)&PCI_ADDR_REGMASK)

/* General purpose PCI read/write functions. */
FUNDEF u32 FCALL pci_readaddr(pci_addr_t addr);
FUNDEF void FCALL pci_writeaddr(pci_addr_t addr, u32 value);
LOCAL u32 FCALL pci_read(pci_addr_t base, pci_reg_t reg) { assert(!(base&PCI_ADDR_REGMASK)); return pci_readaddr(base|reg); }
LOCAL void FCALL pci_write(pci_addr_t base, pci_reg_t reg, u32 value) { assert(!(base&PCI_ADDR_REGMASK)); pci_writeaddr(base|reg,value); }


/* PCI Device data structure (Accessed as registers of any PCI-compliant device). */
#define PCI_DEV0                       0x0 /*< Device/Vendor IDs. */
#define    PCI_DEV0_DEVICEMASK         0xffff0000
#define    PCI_DEV0_VENDORMASK         0x0000ffff
#define    PCI_DEV0_DEVICESHIFT        16
#define    PCI_DEV0_VENDORSHIFT        0
#define    PCI_DEV0_DEVICE(x)       (((x)&PCI_DEV0_DEVICEMASK) >> PCI_DEV0_DEVICESHIFT)
#define    PCI_DEV0_VENDOR(x)        ((x)&PCI_DEV0_VENDORMASK)
#define    PCI_DEV0_VENDOR_NODEV       0xffff /* Returned by non-existent devices. */
#define PCI_DEV4                       0x4 /*< Status/Command register. */
#define    PCI_DEV4_STATMASK           0xffff0000
#define       PCI_CDEV4_STAT_DETECT_PARITY_ERROR 0x8000 /*< Set on parity error. */
#define       PCI_CDEV4_STAT_SIGNAL_SYSTEM_ERROR 0x4000 /*< Set when SERR# is asserted. */
#define       PCI_CDEV4_STAT_MASTER_ABORT        0x2000 /*< Set by the master when a transaction is aborted with Master-Abort. */
#define       PCI_CDEV4_STAT_TARGET_ABORT        0x1000 /*< Set by the master when a transaction is aborted with Target-Abort. */
#define       PCI_CDEV4_STAT_SIGNAL_TARGET_ABORT 0x0800 /*< Set when the target terminates a transaction with Target-Abort. */
#define       PCI_CDEV4_STAT_DEVSEL              0x0600 /*< Mask for device selection timings. */
#define          PCI_CDEV4_STAT_DEVSEL_LO        0x0400 /*< Slow. */
#define          PCI_CDEV4_STAT_DEVSEL_MD        0x0200 /*< medium. */
#define          PCI_CDEV4_STAT_DEVSEL_HI        0x0000 /*< Fast. */
#define       PCI_CDEV4_STAT_PARITY_ERROR        0x0100 /*< Set on parity error when `PCI_CDEV4_PARITIY_ERROR' is enabled. */
#define       PCI_CDEV4_STAT_FAST_B2B            0x0080 /*< The device is capable of fast back-to-back transactions. */
#define       PCI_CDEV4_STAT_66MHZ               0x0020 /*< The device is capable of running at 66 MHZ rather than 33. */
#define       PCI_CDEV4_STAT_HAVE_CAPLINK_34     0x0010 /*< The device implements a capability list as offset 0x34. */
#define       PCI_CDEV4_STAT_IRQ_STATUS          0x0004 /*< When set alongside `PCI_CDEV4_NOIRQ' being disabled, interrupts can be generated. */
#define    PCI_DEV4_CMDMASK            0x0000ffff
#define       PCI_DEV4_CMD_DISCONNECT  0 /*< When written to `PCI_DEV_STATCMD_CMDMASK', disconnect the device from the PCI bus. */
#define       PCI_CDEV4_NOIRQ          0x0400 /*< Interrupt disabled. */
#define       PCI_CDEV4_FAST_B2B       0x0200 /*< Allow fast back-to-back transactions. */
#define       PCI_CDEV4_SERR_ENABLED   0x0100 /*< SERR# Enabled. */
#define       PCI_CDEV4_PARITIY_ERROR  0x0080 /*< Set `PCI_CDEV4_STAT_PARITY_ERROR' on parity error. */
#define       PCI_CDEV4_VGA_PALLSNOOP  0x0020 /*< Don't respond to VGA palette writes, but snoop data. */
#define       PCI_CDEV4_ALLOW_MEMWRITE 0x0010 /*< Allow the device write-access to main memory. */
#define       PCI_CDEV4_SPECIAL_CYCLES 0x0008 /*< Allow the device to monitor Special Cycle operations. */
#define       PCI_CDEV4_BUSMASTER      0x0004 /*< Allow the device to behave as bus Master. */
#define       PCI_CDEV4_ALLOW_MEMTOUCH 0x0002 /*< Allow the device to receive notifications when the CPU touches memory. */
#define       PCI_CDEV4_ALLOW_IOTOUCH  0x0001 /*< Allow the device to receive notifications when the CPU writes to I/O space. */
#define    PCI_DEV4_STATSHIFT          16
#define    PCI_DEV4_CMDSHIFT           0
#define PCI_DEV8                       0x8 /*< Class code, subclass, Prog IF and Revision ID. */
#define    PCI_DEV8_CLASS(x)        (((x)&PCI_DEV8_CLASSMASK) >> PCI_DEV8_CLASSSHIFT)
#define    PCI_DEV8_CLASSMASK          0xff000000
#define       PCI_DEV8_CLASS_NOCLASS       0x00 /*< Device was built prior definition of the class code field. */
#define       PCI_DEV8_CLASS_STORAGE       0x01 /*< Mass Storage Controller. */
#define       PCI_DEV8_CLASS_NETWORK       0x02 /*< Network Controller. */
#define       PCI_DEV8_CLASS_DISPLAY       0x03 /*< Display Controller. */
#define       PCI_DEV8_CLASS_MUMEDIA       0x04 /*< Multimedia Controller. */
#define       PCI_DEV8_CLASS_MEMORY        0x05 /*< Memory Controller. */
#define       PCI_DEV8_CLASS_BRIDGE        0x06 /*< Bridge Device. */
#define       PCI_DEV8_CLASS_COMMUNICATION 0x07 /*< Simple Communication Controllers. */
#define       PCI_DEV8_CLASS_SYSPERIPHERAL 0x08 /*< Base System Peripherals. */
#define       PCI_DEV8_CLASS_INPUT         0x09 /*< Input Devices. */
#define       PCI_DEV8_CLASS_DOCK          0x0A /*< Docking Stations. */
#define       PCI_DEV8_CLASS_PROCESSOR     0x0B /*< Processors. */
#define       PCI_DEV8_CLASS_SERIAL        0x0C /*< Serial Bus Controllers. */
#define       PCI_DEV8_CLASS_WIRELESS      0x0D /*< Wireless Controllers. */
#define       PCI_DEV8_CLASS_INTELL_IO     0x0E /*< Intelligent I/O Controllers. */
#define       PCI_DEV8_CLASS_SATELLITE     0x0F /*< Satellite Communication Controllers. */
#define       PCI_DEV8_CLASS_ENCRYPTION    0x10 /*< Encryption/Decryption Controllers. */
#define       PCI_DEV8_CLASS_DATASIGNAL    0x11 /*< Data Acquisition and Signal Processing Controllers. */
#define       PCI_DEV8_CLASS_OTHER         0xFF /*< Device does not fit any defined class. */
#define    PCI_DEV8_SUBCLASS(x)     (((x)&PCI_DEV8_SUBCLASSMASK) >> PCI_DEV8_SUBCLASSSHIFT)
#define    PCI_DEV8_SUBCLASSMASK       0x00ff0000
#define    PCI_DEV8_PROGIF(x)       (((x)&PCI_DEV8_PROGIFMASK) >> PCI_DEV8_PROGIFSHIFT)
#define    PCI_DEV8_PROGIFMASK         0x0000ff00
#define    PCI_DEV8_REVIDMASK          0x000000ff
#define    PCI_DEV8_CLASSSHIFT         24
#define    PCI_DEV8_SUBCLASSSHIFT      16
#define    PCI_DEV8_PROGIFSHIFT        8
#define    PCI_DEV8_REVIDSHIFT         0
#define PCI_DEVC                       0xc
#define    PCI_DEVC_BISTMASK           0xff000000 /*< Built-in self test. */
#define       PCI_DEVC_BIST_CAPABLE    0x80 /*< FLAG: The device is capable of performing a self-test. */
#define       PCI_DEVC_BIST_START      0x40 /*< FLAG: Set to start a self-test. (Unset on completion; When waiting, timeout after 2 seconds) */
#define       PCI_DEVC_BIST_EXITCODE   0x0f /*< MASK: Self-test exit code. */
#define       PCI_DEVC_BIST_EXITCODE_OK 0   /*< Return value: self-test was successful. */
#define    PCI_DEVC_HEADERMASK         0x00ff0000 /*< Header type (Determines layout of register 0x10 onwards). */
#define    PCI_DEVC_HEADER(x)       (((x)&PCI_DEVC_HEADERMASK) >> PCI_DEVC_HEADERSHIFT)
#define       PCI_DEVC_HEADER_GENERIC  0x00 /*< General device (PCI_GDEV*). */
#define       PCI_DEVC_HEADER_BRIDGE   0x01 /*< PCI-to-PCI bridge (PCI_BDEV*). */
#define       PCI_DEVC_HEADER_CARDBUS  0x02 /*< CardBus bridge (PCI_CDEV*). */
/*            PCI_DEVC_HEADER_...      0x03  */
#define       PCI_DEVC_HEADER_TYPEMASK 0x7f /*< Mask for device types. */
#define       PCI_DEVC_HEADER_MULTIDEV 0x80 /*< FLAG: Set if the device fulfills multiple functions. */
#define    PCI_DEVC_LATENCY_TIMERMASK  0x0000ff00 /*< latency timer in units of PCI bus clocks. */
#define    PCI_DEVC_CACHELINESIZEMASK  0x000000ff /*< System cache line size. */
#define    PCI_DEVC_BISTSHIFT          24
#define    PCI_DEVC_HEADERSHIFT        16
#define    PCI_DEVC_LATENCY_TIMERSHIFT 8
#define    PCI_DEVC_CACHELINESIZESHIFT 0

/* PCI Header-type (PCI_DEVC_HEADERMASK) specific registers. */
#define PCI_GDEV_BAR0    0x10 /*< Base address #0. */
#define PCI_GDEV_BAR1    0x14 /*< Base address #1. */
#define PCI_GDEV_BAR2    0x18 /*< Base address #2. */
#define PCI_GDEV_BAR3    0x1c /*< Base address #3. */
#define PCI_GDEV_BAR4    0x20 /*< Base address #4. */
#define PCI_GDEV_BAR5    0x24 /*< Base address #5. */
#define PCI_GDEV_CARDCIS 0x28 /*< Cardbus CIS Pointer. */
#define PCI_GDEV2C       0x2c /*< Subsystem ID / Subsystem Vendor ID. */
#define    PCI_GDEV3C_SSYSIDMASK  0xffff0000
#define    PCI_GDEV3C_VENDORMASK  0x0000ffff
#define    PCI_GDEV3C_SSYSIDSHIFT 16
#define    PCI_GDEV3C_VENDORSHIFT 0
#define PCI_GDEV_EXPROM  0x30 /*< Expansion ROM base address. */
#define PCI_GDEV_RES0    0x34 /*< Reserved + Capabilities pointer. */
#define    PCI_GDEV_RES0_CAPPTRMASK  0x000000ff
#define    PCI_GDEV_RES0_CAPPTRSHIFT 0
#define PCI_GDEV_RES1    0x38 /*< Reserved. */
#define PCI_GDEV3C       0x3c
#define    PCI_GDEV3C_MAXLATENCYMASK  0xff000000 /*< Specifies how often the device needs access to the PCI bus (in 1/4 microsecond units). */
#define    PCI_GDEV3C_MINGRANTMASK    0x00ff0000 /*< Specifies the burst period length, in 1/4 microsecond units, that the device needs (assuming a 33 MHz clock rate). */
#define    PCI_GDEV3C_IRQPINMASK      0x0000ff00 /*< Interrupt pin number. */
#define    PCI_GDEV3C_IRQLINE(x)   (((x)&PCI_GDEV3C_IRQLINEMASK) >> PCI_GDEV3C_IRQLINESHIFT)
#define    PCI_GDEV3C_IRQLINEMASK     0x000000ff /*< Interrupt line number. */
#define    PCI_GDEV3C_MAXLATENCYSHIFT 24
#define    PCI_GDEV3C_MINGRANTSHIFT   16
#define    PCI_GDEV3C_IRQPINSHIFT     8
#define    PCI_GDEV3C_IRQLINESHIFT    0

/* PCI Header-type (PCI_DEVC_HEADER_BRIDGE) specific registers. */
#define PCI_BDEV_BAR0    0x10 /*< Base address #0. */
#define PCI_BDEV_BAR1    0x14 /*< Base address #1. */
#define PCI_BDEV18       0x18
#define    PCI_BDEV18_SECONDARY_LATENCY_TIMER(x)   (((x)&PCI_BDEV18_SECONDARY_LATENCY_TIMERMASK) >> PCI_BDEV18_SECONDARY_LATENCY_TIMERSHIFT)
#define    PCI_BDEV18_SECONDARY_LATENCY_TIMERMASK  0xff000000
#define    PCI_BDEV18_SUBORDINATE_BUS(x)           (((x)&PCI_BDEV18_SUBORDINATE_BUSMASK) >> PCI_BDEV18_SUBORDINATE_BUSSHIFT)
#define    PCI_BDEV18_SUBORDINATE_BUSMASK          0x00ff0000
#define    PCI_BDEV18_SECONDARY_BUS(x)             (((x)&PCI_BDEV18_SECONDARY_BUSMASK) >> PCI_BDEV18_SECONDARY_BUSSHIFT)
#define    PCI_BDEV18_SECONDARY_BUSMASK            0x0000ff00
#define    PCI_BDEV18_PRIMARY_BUS(x)               ((x)&PCI_BDEV18_PRIMARY_BUSMASK)
#define    PCI_BDEV18_PRIMARY_BUSMASK              0x000000ff
#define    PCI_BDEV18_SECONDARY_LATENCY_TIMERSHIFT 24
#define    PCI_BDEV18_SUBORDINATE_BUSSHIFT         16
#define    PCI_BDEV18_SECONDARY_BUSSHIFT           8
#define    PCI_BDEV18_PRIMARY_BUSSHIFT             0
#define PCI_BDEV1C       0x1c
#define    PCI_BDEV1C_SECONDARY_STATUSMASK   0xffff0000
#define    PCI_BDEV1C_SECONDARY_STATUSSHIFT  16
#define    PCI_BDEV1C_IOLIMITMASK            0x0000ff00
#define    PCI_BDEV1C_IOBASEMASK             0x000000ff
#define    PCI_BDEV1C_IOLIMITSHIFT           8
#define    PCI_BDEV1C_IOBASESHIFT            0
#define PCI_BDEV20       0x20
#define    PCI_BDEV20_MEMLIMITMASK  0xffff0000
#define    PCI_BDEV20_MEMLIMITSHIFT 16
#define    PCI_BDEV20_MEMBASEMASK   0x0000ffff
#define    PCI_BDEV20_MEMBASESHIFT  0
#define PCI_BDEV24       0x24
#define    PCI_BDEV20_PREFETCH_MEMLIMITMASK  0xffff0000
#define    PCI_BDEV20_PREFETCH_MEMLIMITSHIFT 16
#define    PCI_BDEV20_PREFETCH_MEMBASEMASK   0x0000ffff
#define    PCI_BDEV20_PREFETCH_MEMBASESHIFT  0
#define PCI_BDEV_PREFETCHBASE_HI32  0x28
#define PCI_BDEV_PREFETCHLIMIT_HI32 0x2c
#define PCI_BDEV30 0x30
#define    PCI_BDEV30_IOLIMIT_HI16MASK  0xffff0000
#define    PCI_BDEV30_IOLIMIT_HI16SHIFT 16
#define    PCI_BDEV30_IOBASE_HI16MASK   0x0000ffff
#define    PCI_BDEV30_IOBASE_HI16SHIFT  0
#define PCI_BDEV_RES0    0x34 /*< Reserved + Capabilities pointer. */
#define    PCI_BDEV_RES0_CAPPTRMASK  0x000000ff
#define    PCI_BDEV_RES0_CAPPTRSHIFT 0
#define PCI_BDEV_EXPROM  0x38 /*< Expansion ROM base address. */
#define PCI_BDEV3C       0x3c
#define    PCI_BDEV3C_BRIDGECONTROLMASK  0xffff0000 /*< Bridge control. */
#define    PCI_BDEV3C_IRQPINMASK         0x0000ff00 /*< Interrupt pin number. */
#define    PCI_BDEV3C_IRQLINEMASK        0x000000ff /*< Interrupt line number. */
#define    PCI_BDEV3C_BRIDGECONTROLSHIFT 16
#define    PCI_BDEV3C_IRQPINSHIFT        8
#define    PCI_BDEV3C_IRQLINESHIFT       0

/* PCI Header-type (PCI_DEVC_HEADER_CARDBUS) specific registers. */
#define PCI_CDEV_SOCKET_BASEADDR  0x10 /*< CardBus socket / ExCa base address. */
#define PCI_CDEV14       0x14
#define    PCI_CDEV14_SECONDARY_STATUSMASK   0xffff0000
#define    PCI_CDEV14_SECONDARY_STATUSSHIFT  16
#define    PCI_CDEV14_CAPPTRMASK  0x000000ff
#define    PCI_CDEV14_CAPPTRSHIFT 0
#define PCI_CDEV18       0x18
#define    PCI_CDEV18_CARDBUS_LATENCY_TIMERMASK   0xff000000
#define    PCI_CDEV18_SUBORDINATE_BUSMASK         0x00ff0000
#define    PCI_CDEV18_SECONDARY_BUSMASK           0x0000ff00
#define    PCI_CDEV18_PRIMARY_BUSMASK             0x000000ff
#define    PCI_CDEV18_CARDBUS_LATENCY_TIMERSHIFT  24
#define    PCI_CDEV18_SUBORDINATE_BUSSHIFT        16
#define    PCI_CDEV18_SECONDARY_BUSSHIFT          8
#define    PCI_CDEV18_PRIMARY_BUSSHIFT            0
#define PCI_CDEV_MEMBASE0   0x1c
#define PCI_CDEV_MEMLIMIT0  0x20
#define PCI_CDEV_MEMBASE1   0x24
#define PCI_CDEV_MEMLIMIT1  0x28
#define PCI_CDEV_IOBASE0    0x2c
#define PCI_CDEV_IOLIMIT0   0x30
#define PCI_CDEV_IOBASE1    0x34
#define PCI_CDEV_IOLIMIT1   0x38
#define PCI_CDEV3C          0x3c
#define    PCI_CDEV3C_BRIDGECONTROLMASK  0xffff0000 /*< Bridge control. */
#define    PCI_CDEV3C_IRQPINMASK         0x0000ff00 /*< Interrupt pin number. */
#define    PCI_CDEV3C_IRQLINEMASK        0x000000ff /*< Interrupt line number. */
#define    PCI_CDEV3C_BRIDGECONTROLSHIFT 16
#define    PCI_CDEV3C_IRQPINSHIFT        8
#define    PCI_CDEV3C_IRQLINESHIFT       0
#define PCI_CDEV40          0x40 /*< Subsystem ID / Subsystem Vendor ID. */
#define    PCI_CDEV40_SSYSIDMASK  0xffff0000
#define    PCI_CDEV40_VENDORMASK  0x0000ffff
#define    PCI_CDEV40_SSYSIDSHIFT 16
#define    PCI_CDEV40_VENDORSHIFT 0
#define PCI_CDEV_LEGACY_BASEADDR16 0x44 /*< 16-bit PC Card legacy mode base address. */



/* Stuff exported by the PCI database. */
struct db_ref {
 u16 r_pci_vendor_id; /* PCI vendor ID. */
 u16 r_pci_device_id; /* PCI device ID. */
};

struct db_device {
 u16 d_pci_vendor_id; /* PCI ID of this device's vendor. */
 u16 d_pci_device_id; /* PCI ID of this device. */
#define DB_DEVICE_NAME(x) (db_strtab+(x)->d_nameaddr)
 u32 d_nameaddr;      /* Byte-offset into `db_strtab' containing the device name. */
 u16 d_parcount;      /* Number of parent devices (Aka. devices that this one is compatible with). */
 u16 d_subcount;      /* Number of sub-devices (Aka. devices that are compatible with this one). */
#define DB_DEVICE_PARDEVS(x) ((struct db_ref *)((uintptr_t)db_refs+(x)->d_refs))
#define DB_DEVICE_SUBDEVS(x) ((struct db_ref *)((uintptr_t)db_refs+(x)->d_refs)+(x)->d_parcount)
 u32 d_refs;          /* Byte-offset into `db_refs', containing a vector of
                       * `d_parcount' `struct db_ref', followed by `d_subcount' more. */
};

struct db_devicebucket {
 u32 db_devicec; /* Amount of devices in this bucket. */
#define DB_DEVICEBUCKET_DEVICEV(x) ((struct db_device *)((uintptr_t)db_devices+(x)->db_devicev))
 u32 db_devicev; /* Byte-offset from `db_devices', pointing at the start
                  * of a `db_devicec'-long vector of `struct pci_device'. */
};

struct db_vendor {
 u16   v_pci_id;              /* PCI ID of this vendor. */
 u16 __v_padding;             /* ... */
#define DB_VENDOR_NAME(x)    (db_strtab+(x)->v_nameaddr)
 u32   v_nameaddr;            /* Byte-offset into `db_strtab' containing the vendor name. */
 u32   v_device_count;        /* Amount of known devices of this vendor. */
 u32   v_device_bucket_count; /* Amount of buckets used to track devices. */
#define DB_VENDOR_DEVICE_BUCKETS(x) ((struct db_devicebucket *)((uintptr_t)db_device_hashmap+(x)->v_device_bucket_start))
 u32   v_device_bucket_start; /* Byte-offset from `db_device_hashmap', pointing at the start of this vendor's device hash-map. */
};

struct db_vendorbucket {
 u32 vb_vendorc; /* Amount of vendors in this bucket. */
#define DB_VENDORBUCKET_VENDORV(x) ((struct db_vendor *)((uintptr_t)db_vendors+(x)->vb_vendorv))
 u32 vb_vendorv; /* Offset from `db_vendors', pointing at the start of a
                  * `vb_vendorc'-long vector of `struct db_vendor'. */
};

#define DB_HASH_VENDOR(vendor_id) (vendor_id)
#define DB_HASH_DEVICE(device_id) (device_id)

DATDEF byte_t db_vendor_count[];
DATDEF byte_t db_vendor_buckets[];
#define DB_VENDOR_COUNT    ((uintptr_t)db_vendor_count)
#define DB_VENDOR_BUCKETS  ((uintptr_t)db_vendor_buckets)
DATDEF struct db_vendorbucket const db_vendor_hashmap[]; /* DATABASE ROOT! */

/* Relocation-independent indirection offsets. */
DATDEF char const db_strtab[];
DATDEF struct db_devicebucket const db_device_hashmap[];
DATDEF struct db_vendor const db_vendors[];
DATDEF struct db_device const db_devices[];
DATDEF struct db_ref const db_refs[];

/* Database lookup functions.
 * NOTE: All functions return NULL if no match exists. */
FUNDEF struct db_vendor const *KCALL pci_db_getvendor(u16 vendor_id);
FUNDEF struct db_device const *KCALL pci_db_getdevice(struct db_vendor const *vendor, u16 device_id);


#ifndef CONFIG_NO_PCI_CLASSES
/* PCI Class database. */
struct db_class {
 u8    c_class_id;
 u8    c_subclass_count;
#define DB_CLASS_SUBCLASSES(x) ((struct db_subclass *)((uintptr_t)db_subclasses+(x)->c_subclass_addr))
 u16   c_subclass_addr; /* Offset into db_subclasses */
#define DB_CLASS_NAME(x) (db_strtab+(x)->c_name)
 u32   c_name;          /* Offset into db_strtab */
};


struct db_subclass {
 u8    sc_class_id;
 u8    sc_subclass_id;
 u8    sc_progif_count;
 u8  __sc_pad0;
#define DB_SUBCLASS_NAME(x) (db_strtab+(x)->sc_name)
 u32   sc_name;         /* Offset into db_strtab */
#define DB_SUBCLASS_PROGIFS(x) ((struct db_progif *)((uintptr_t)db_progifs+(x)->sc_progif_addr))
 u16   sc_progif_addr;  /* Offset into db_progifs */
#define DB_SUBCLASS_CLASS(x) ((struct db_class *)((uintptr_t)db_classes+(x)->sc_class_addr))
 u16   sc_class_addr;   /* Offset into db_classes */
};
struct db_progif {
 u8    pi_class_id;
 u8    pi_subclass_id;
 u8    pi_progif_id;
 u8  __pi_pad0;
#define DB_PROGIF_NAME(x) (db_strtab+(x)->pi_name)
 u32   pi_name;         /* Offset into db_strtab */
#define DB_PROGIF_CLASS(x) ((struct db_class *)((uintptr_t)db_classes+(x)->pi_class_addr))
 u16   pi_class_addr;   /* Offset into db_classes */
#define DB_PROGIF_SUBCLASS(x) ((struct db_subclass *)((uintptr_t)db_subclasses+(x)->pi_subclass_addr))
 u16   pi_subclass_addr;/* Offset into db_subclasses */
};

DATDEF byte_t db_classes_count[];
#define DB_CLASSES_COUNT  ((uintptr_t)db_classes_count)
DATDEF struct db_class const db_classes[];
DATDEF struct db_subclass const db_subclasses[];
DATDEF struct db_progif const db_progifs[];

/* Lookup class descriptors from the PCI database. */
FUNDEF struct db_class const *KCALL pci_db_getclass(u8 classid);
FUNDEF struct db_subclass const *KCALL pci_db_getsubclass(struct db_class const *class_, u8 subclassid);
FUNDEF struct db_progif const *KCALL pci_db_getprogif(struct db_subclass const *subclass, u8 progifid);

#endif /* !CONFIG_NO_PCI_CLASSES */



struct pci_resource {
 PHYS uintptr_t pr_begin; /*< Base address of the resource (either in I/O, or in memory) */
 size_t         pr_size;  /*< Resource size (in bytes). */
 size_t         pr_align; /*< Resource alignment (in bytes). */
#define PCI_RESOURCE_MEM   0x0001 /*< Memory resource. */
#define PCI_RESOURCE_MEM16 0x0002 /*< Needs a 16-bit memory address. */
#define PCI_RESOURCE_MEM32 0x0000 /*< Needs a 32-bit memory address. */
#define PCI_RESOURCE_MEM64 0x0004 /*< Needs a 64-bit memory address. */
#define PCI_RESOURCE_IO    0x8000 /*< I/O resource. */
 uintptr_t      pr_flags; /*< Resource flags. */
};

/* PCI Resource IDs. */
#define PD_RESOURCE_BAR(i) i
#define PD_RESOURCE_BAR0   0
/*      PD_RESOURCE_BAR1-4 1-4 */
#define PD_RESOURCE_BAR5   5
#define PD_RESOURCE_EXPROM 6
#define PD_RESOURCE_COUNT  7


/* Kernel-level descriptor for PCI devices. */
struct pci_device {
 ATOMIC_DATA ref_t             pd_refcnt;   /*< Reference counter. */
 REF LIST_NODE(struct pci_device)
                               pd_link;     /*< [0..1][lock(pci_lock)] Pointer to the next existing device. */
 pci_addr_t                    pd_addr;     /*< [const] The base address of the PCI device. */
 struct db_vendor const       *pd_vendor;   /*< [0..1][lock(pci_lock)] Database entry for `pd_vendorid'. */
 struct db_device const       *pd_device;   /*< [0..1][lock(pci_lock)] Database entry for `pd_vendorid' + `pd_deviceid'. */
#ifndef CONFIG_NO_PCI_CLASSES
 struct db_class const        *pd_class;    /*< [0..1][lock(pci_lock)] Database entry for `pd_classid'. */
 struct db_subclass const     *pd_subclass; /*< [0..1][lock(pci_lock)] Database entry for `pd_classid' + `pd_subclassid'. */
 struct db_progif const       *pd_progif;   /*< [0..1][lock(pci_lock)] Database entry for `pd_classid' + `pd_subclassid' + `pd_progifid'. */
#endif /* !CONFIG_NO_PCI_CLASSES */
 struct pci_resource           pd_resources[PD_RESOURCE_COUNT]; /*< [const] Resources. */
union PACKED { struct PACKED {
#if __BYTE_ORDER == __LITTLE_ENDIAN
 u16                           pd_vendorid; /*< Vendor ID. */
 u16                           pd_deviceid; /*< Device ID. */
#else
 u16                           pd_deviceid; /*< Device ID. */
 u16                           pd_vendorid; /*< Vendor ID. */
#endif
}; u32                         pd_dev0; };  /*< Value of the PCI register `PCI_DEV0' */
union PACKED { struct PACKED {
#if __BYTE_ORDER == __LITTLE_ENDIAN
 u8                            pd_revid;      /*< Revision ID. */
 u8                            pd_progifid;   /*< Prog-IF ID. */
 u8                            pd_subclassid; /*< Device sub-class (One of `PCI_DEV8_CLASS_*') */
 u8                            pd_classid;    /*< Device class (One of `PCI_DEV8_CLASS_*') */
#else
 u8                            pd_classid;    /*< Device class (One of `PCI_DEV8_CLASS_*') */
 u8                            pd_subclassid; /*< Device sub-class (One of `PCI_DEV8_CLASS_*') */
 u8                            pd_progifid;   /*< Prog-IF ID. */
 u8                            pd_revid;      /*< Revision ID. */
#endif
}; u32                         pd_dev8; };    /*< Value of the PCI register `PCI_DEV8' */
union PACKED { struct PACKED {
#if __BYTE_ORDER == __LITTLE_ENDIAN
 u8                            pd_pad0;   /*< ... */
 u8                            pd_pad1;   /*< ... */
 u8                            pd_header; /*< PCI_DEVC_HEADERMASK */
 u8                            pd_pad2;   /*< ... */
#else
 u8                            pd_pad0;   /*< ... */
 u8                            pd_header; /*< PCI_DEVC_HEADERMASK */
 u8                            pd_pad1;   /*< ... */
 u8                            pd_pad2;   /*< ... */
#endif
}; u32                         pd_devc; };    /*< Value of the PCI register `PCI_DEVC' */
};
DATDEF SLIST_HEAD(struct pci_device) pci_list; /*< [0..1][owned] Globally linked list of PCI devices. */
DATDEF atomic_rwlock_t               pci_lock; /*< Lock for `pci_list'. */
#define PCI_FOREACH(dev) LIST_FOREACH(dev,pci_list,pd_link)

/* PCI Locking control. */
#define PCI_TRYREAD()      atomic_rwlock_tryread(&pci_lock)
#define PCI_TRYWRITE()     atomic_rwlock_trywrite(&pci_lock)
#define PCI_TRYUPGRADE()   atomic_rwlock_tryupgrade(&pci_lock)
#define PCI_READ()         atomic_rwlock_read(&pci_lock)
#define PCI_WRITE()        atomic_rwlock_write(&pci_lock)
#define PCI_UPGRADE()      atomic_rwlock_upgrade(&pci_lock)
#define PCI_DOWNGRADE()    atomic_rwlock_downgrade(&pci_lock)
#define PCI_ENDREAD()      atomic_rwlock_endread(&pci_lock)
#define PCI_ENDWRITE()     atomic_rwlock_endwrite(&pci_lock)


/* Enumerate all PCI devices matching the given class and subclass ID. */
#define PCI_FOREACH_CLASS(dev,classid,subclassid) \
   PCI_FOREACH(dev) if (!((dev)->pd_classid == (classid) && \
                          (dev)->pd_subclassid == (subclassid))); \
   else


/* Lookup a PCI device associated with a given address.
 * @return: * :   Then device associated with the given address.
 * @return: NULL: No device associated with that address. */
FUNDEF REF struct pci_device *KCALL pci_getdevice_at(pci_addr_t addr);
FUNDEF struct pci_device *KCALL pci_getdevice_at_unlocked(pci_addr_t addr);


#define PCI_DEVICE_PRESENT(self)         (!LIST_ISUNBOUND(self,pd_link))
#define PCI_DEVICE_INCREF(self)    (void)(ATOMIC_FETCHINC((self)->pd_refcnt))
#define PCI_DEVICE_DECREF(self)    (void)(ATOMIC_DECFETCH((self)->pd_refcnt) || (free(self),0))

/* Find and return the first I/O or memory resource, or return NULL if no such resource exists. */
FUNDEF struct pci_resource *KCALL pci_find_iobar(struct pci_device *__restrict dev);
FUNDEF struct pci_resource *KCALL pci_find_membar(struct pci_device *__restrict dev);


DECL_END

#endif /* !GUARD_INCLUDE_MODULES_PCI_H */
