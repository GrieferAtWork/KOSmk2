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
#ifndef GUARD_INCLUDE_DEV_DEVICE_H
#define GUARD_INCLUDE_DEV_DEVICE_H 1

#include <errno.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <hybrid/atomic.h>
#include <hybrid/compiler.h>
#include <hybrid/kdev_t.h>
#include <hybrid/list/atree.h>
#include <hybrid/list/btree.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/sync/atomic-owner-rwlock.h>
#include <hybrid/types.h>
#include <sync/rwlock.h>

DECL_BEGIN

struct instance;
struct devns;
struct vsuperblock;

#define IRQCTL_DISABLE  0 /*< Disable interrupts caused by the device. */
#define IRQCTL_ENABLE   1 /*< Disable interrupts and check for lost interrupts. */
#define IRQCTL_TEST     2 /*< Check for lost interrupts. */
typedef void (KCALL *pirqctl)(struct device *__restrict dev, unsigned int cmd);

struct device {
 struct inode                  d_node;   /*< Underlying INode. */
 REF BTREE_NODE(struct device) d_devnode;/*< [KEY(minor_t)][lock(:d_lock)] Major-number namespace node. */
#define DEVICE_FLAG_NORMAL 0x00000000
#define DEVICE_FLAG_WEAKID 0x00000001    /*< The device is weakly linked, meaning that the device namespace
                                          *  does not carry a reference and the device will therefor be
                                          *  unloaded once all reference pointing to it are destroyed.
                                          *  NOTE: This also affects the '/dev' filesystem, causing it
                                          *        to create weak indirection device nodes, rather than
                                          *        strong ones. */
 u32                           d_flags;  /*< [const] Set of 'DEVICE_FLAG_*' */
 ATOMIC_DATA dev_t             d_id;     /*< [const][valid_if(WAS_REGISTERED(self))] Device ID. */
 /* [0..1][const] Implemented by IRQ-driven devices: Check if an interrupt was lost.
  *  NOTE: Since KOS core drivers uses BIOS interrupts to implement a fundamental
  *        driver environment that is at least capable of loading additional drivers,
  *        depending on whether or not it manages to find suitable replacements for
  *        those drivers, there is a chance that they will remain in use even once
  *        user-space applications being executing.
  *        At this point, it is possible that interrupts such as keyboard key presses
  *        get swallowed while execution resides within the BIOS, leaving the system
  *        open to soft-locking due to loss of said interrupts.
  *        For that reason, any driver required to handle interrupts (such as PS/2)
  *        should implement this function to quickly check if there are pending
  *        interrupts that could not be handled (See PS/2 for an example of this). */
 pirqctl                       d_irq_ctl;
 LIST_NODE(struct device)      d_irq_dev; /*< [valid_if(d_irq_ctl != NULL)][lock(INTERNAL)] Chain of devices that implement IRQ controls. */
};
#define DEVICE_TRYINCREF(self)  INODE_TRYINCREF(&(self)->d_node)
#define DEVICE_INCREF(self)     INODE_INCREF(&(self)->d_node)
#define DEVICE_DECREF(self)     INODE_DECREF(&(self)->d_node)

#define DEVICE_ISBLK(self)      INODE_ISBLK(&(self)->d_node)
#define DEVICE_ISCHR(self)      INODE_ISCHR(&(self)->d_node)
#define DEVICE_ISREADONLY(self) INODE_ISREADONLY(&(self)->d_node)
#define DEVICE_ID(self)         ATOMIC_READ((self)->d_id)

/* Allocate/Initialize a new device.
 * The caller must fill in:
 *  - d_id (Use 'devns_insert')
 *  - d_flags (Optionally; pre-initialized to 'DEVICE_FLAG_NORMAL')
 *  - d_irq_lost (Optionally; pre-initialized to 'NULL')
 *  - d_node.i_ops
 *  - d_node.i_attr.ia_mode
 *  - d_node.i_attr_disk.ia_mode (Copy of 'd_node.i_attr.ia_mode')
 *  - d_node.i_data (Optionally)
 *  - d_node.i_super (Use 'device_setup')
 */
#define device_new(type_size) \
        device_cinit((struct device *)calloc(1,type_size))
FUNDEF struct device *KCALL device_cinit(struct device *self);

/* Mark the given device as weak (That is: The kernel will try to refrain
 * from creating real reference, in favor of weakly aliasing the device).
 * NOTE: This function may only be called after 'device_cinit()',
 *       but before 'device_setup()' */
#define DEVICE_SETWEAK(self) \
 (void)((self)->d_flags |= DEVICE_FLAG_WEAKID, \
        (self)->d_node.i_state |= INODE_STATE_DONTCACHE)


/* Perform final initialization on the given device.
 * @return: -EOK:   Successfully initialize the given device.
 * @return: -EPERM: The given instance 'owner' doesn't permit new references being created. */
FUNDEF WUNUSED errno_t KCALL device_setup(struct device *__restrict self,
                                          struct instance *__restrict owner);

/* Destructor that must be called when destructing a device.
 * NOTE: This function must be called from 'bd_device.d_node.i_ops->ino_fini' */
FUNDEF void KCALL device_fini(struct device *__restrict self);

/* Call this function when you think that device IRQs may have been lost.
 * This function will then go through all registered devices and manually
 * check for any new notifications that may have been skipped.
 * NOTE: The caller must be holding a read-lock to 'irqctl_lock'
 * HINT: This function is called before and after a realmode BIOS interrupt.
 * @param: cmd: One of 'IRQCTL_*' */
FUNDEF void KCALL device_irqctl(unsigned int cmd);

/* Lock held when icqctl commands are fired. */
DATDEF atomic_owner_rwlock_t irqctl_lock;

/* The device filesystem superblock ("/dev"). */
DATDEF struct vsuperblock dev_fs;
/* [0..1][const] The main mounting point for the device-filesystem root. */
DATDEF REF struct dentry *const devfs_root;

#ifdef CONFIG_BUILDING_KERNEL_CORE
INTDEF INITCALL void KCALL devfs_mount_initialize(void);
#ifdef CONFIG_DEBUG
INTERN void KCALL devfs_mount_finalize(void);
#endif
#endif



struct devns_major {
 ATREE_NODE(struct devns_major,major_t) ma_node; /*< [lock(:d_lock)][owned] Tree node. */
 REF BTREE_HEAD(struct device)         *ma_bvec; /*< [0..1][1..DEVNS_MAJOR_VEC(self)][owned][lock(:d_lock)]
                                                  *   Device tree. NOTE: NULL-entries count as reserved. */
 LIST_NODE(struct devns_major)          ma_sort; /*< [sort(ASCENDING(DEVNS_MAJOR_MIN(*)))][lock(:d_lock)][0..1] Ordered list of major device numbers. */
};
#define DEVNS_MAJOR_MIN(x) (x)->ma_node.a_vmin
#define DEVNS_MAJOR_MAX(x) (x)->ma_node.a_vmax
#define DEVNS_MAJOR_CNT(x) (DEVNS_MAJOR_MAX(x)-DEVNS_MAJOR_MIN(x)+1)
#define DEVNS_MAJOR_VEC(x) (x)->ma_bvec


/* Callbacks executed after devices were added/removed from a given device namespace.
 * NOTE: In the event of 'pdevns_added' returning an error, 'devns_insert()' will fail with that same code. */
typedef errno_t (KCALL *pdevns_added)(struct devns *__restrict self, struct device *__restrict dev, dev_t id);
typedef void    (KCALL *pdevns_deleted)(struct devns *__restrict self, struct device *__restrict dev, dev_t id);

struct devns_event {
 SLIST_NODE(struct devns_event) de_chain; /*< [lock(:d_elock)] Chain of namespace events. (Executed in order) */
 pdevns_added                   de_add;   /*< [0..1][const] Callback executed after devices are added. */
 pdevns_deleted                 de_del;   /*< [0..1][const] Callback executed after devices are removed.
                                           *   NOTE: This event may also be executed after 'de_add', when
                                           *         some later chain callback fails for some reason. */
 WEAK REF struct instance      *de_owner; /*< [1..1][const] The owner instance of this event callback. */
};

struct devns {
 /* Device namespace. */
 atomic_rwlock_t                d_lock;  /*< Lock for this device namespace. */
 ATREE_HEAD(struct devns_major) d_tree;  /*< [KEY(major_t)][lock(d_lock)][0..1][owned] Major-number address tree root. */
 LIST_HEAD(struct devns_major)  d_sort;  /*< [sort(ASCENDING(DEVNS_MAJOR_MIN(*)))][lock(d_lock)][0..1] Ordered list of major device numbers. */
 rwlock_t                       d_elock; /*< Lock for the event chain 'd_event'. */
 SLIST_HEAD(struct devns_event) d_event; /*< [0..1][lock(d_elock)] Chain of namespace event callbacks. */
};

/* Add/delete device-namespace event callbacks.
 * NOTE: These functions will add/delete a weak reference to 'e->de_owner'. */
FUNDEF void KCALL devns_addevent(struct devns *__restrict self, struct devns_event *__restrict e);
/* @return: true:  Successfully removed the namespace-event callback.
 * @return: false: The given namespace-event callback was never added. */
FUNDEF bool KCALL devns_delevent(struct devns *__restrict self, struct devns_event *__restrict e);


struct blkdev;
struct chrdev;

#define BLKDEV_LOOKUP(id)       ((struct blkdev *)devns_lookup(&ns_blkdev,id))
#define CHRDEV_LOOKUP(id)       ((struct chrdev *)devns_lookup(&ns_chrdev,id))
#define BLKDEV_REGISTER(dev,id)   devns_insert(&ns_blkdev,(struct device *)(dev),id)
#define CHRDEV_REGISTER(dev,id)   devns_insert(&ns_chrdev,(struct device *)(dev),id)

/* Device namespaces (for block/character-devices) */
DATDEF struct devns ns_blkdev;
DATDEF struct devns ns_chrdev;

/* Lookup the device associated with a given device ID.
 * @return: * :   A new reference to the device associated with ID.
 * @return: NULL: No device is associated the the specified ID. */
FUNDEF REF struct device *KCALL devns_lookup(struct devns *__restrict self, dev_t id);

/* Register a device for the given device ID 'id'.
 * NOTE: Attempting to re-register a device under the same id is a no-op.
 * NOTE: 'devns_insert_r' should be used if the given ID should be reserved first.
 * @return: -EOK:       The given device `dev' is now registered under 'id'.
 * @return: -EEXIST:    Another device was already registered under the given 'id'.
 * @return: -ENOMEM:    Not enough available kernel memory.
 * @return: E_ISERR(*): A 'pdevns_added'-callback failed for some reason. */
FUNDEF errno_t KCALL devns_insert(struct devns *__restrict self,
                                  struct device *__restrict dev,
                                  dev_t id);

/* Remove the given device `dev' from the specified device namespace.
 * NOTE: When 'release' is true, delete the associated major id when
 *       when this operation removes the last associated minor device. */
FUNDEF bool KCALL devns_remove(struct devns *__restrict self,
                               struct device *__restrict dev,
                               bool release);

/* Similar to 'devns_remove', but delete devices by their id.
 * @param:  mode:  Set of 'DEVNS_ERASE_*'
 * @return: true:  Successfully removed the device associated with 'id'.
 * @return: false: Failed to remove the device (no such device). */
FUNDEF bool KCALL devns_erase(struct devns *__restrict self, dev_t id, u32 mode);
#define DEVNS_ERASE_DEVICE     0x00 /*< Simply erase the device associated with the given id. */
#define DEVNS_ERASE_RELEASE    0x01 /*< Release the associated major number when this operation removes the last associated minor device. */
#define DEVNS_ERASE_PARTITIONS 0x02 /*< Also erase all associated partition devices associated with a block-device. */


/* ~normal~ set of flags used to delete a device simply registered by a call
 * to 'devns_insert' without prior reservation of major device numbers.
 * In addition: Delete any partition devices that may have been created automatically. */
#define DEVNS_ERASE_NORMAL  (DEVNS_ERASE_DEVICE|DEVNS_ERASE_PARTITIONS|DEVNS_ERASE_RELEASE)

/* A faster set of flags that works for character
 * devices equally as well as 'DEVNS_ERASE_NORMAL'. */
#define DEVNS_ERASE_CHRDEV  (DEVNS_ERASE_DEVICE|DEVNS_ERASE_RELEASE)


/* Reserve/release major device numbers dynamically.
 * @return: * :                  The first major number of 'n_major' consecutive, reserved IDs.
 * @return: DEVNS_RESERVE_ERROR: Failed to reserve any more ids (no-mem/no-space) */
FUNDEF major_t KCALL devns_reserve(struct devns *__restrict self, size_t n_major);
FUNDEF major_t KCALL devns_reserve_at(struct devns *__restrict self, major_t start, size_t n_major);
FUNDEF void KCALL devns_release(struct devns *__restrict self, major_t start, size_t n_major);
#define DEVNS_RESERVE_ERROR ((major_t)-1)

DECL_END

#endif /* !GUARD_INCLUDE_DEV_DEVICE_H */
