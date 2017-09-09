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
#ifndef GUARD_INCLUDE_DEV_BLKDEV_H
#define GUARD_INCLUDE_DEV_BLKDEV_H 1

#include <dev/device.h>
#include <errno.h>
#include <fs/fs.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <sync/rwlock.h>
#ifndef __INTELLISENSE__
#include <hybrid/check.h>
#endif

DECL_BEGIN

struct blkdev;
struct instance;

typedef u32 blksys_t; /*< s.a.: 'BLKSYS_*' */
#define BLKSYS_ACTIVE  0x80000000 /*< Flag: The partition/device was marked as active. */
#define BLKSYS_MASK    0x0000ffff /*< Mask for the filesystem type. */
#define BLKSYS_GET(x)  ((x)&BLKSYS_MASK)

/* See full list: 'http://www.win.tue.nl/~aeb/partitions/partition_types-1.html' */
#define BLKSYS_ANY              0x00 /*< Any system type (May be used to register an MBR parser with 'autopart_register'). */
#define BLKSYS_UNKNOWN          0x00
#define BLKSYS_EMPTY            0x00

#define BLKSYS_FAT12            0x01
#define BLKSYS_FAT16ALT         0x04
#define BLKSYS_EXTENDED1        0x05 /* Extended partition (these describe another MBR) */
#define BLKSYS_FAT16            0x06
#define BLKSYS_NTFS             0x07
#define BLKSYS_FAT32            0x0b
#define BLKSYS_FAT32_LBA        0x0c
#define BLKSYS_FAT16_LBA        0x0e
#define BLKSYS_EXTENDED2        0x0f
#define BLKSYS_FAT12_HIDDEN     0x11
#define BLKSYS_FAT16_32M_HIDDEN 0x14
#define BLKSYS_FAT16_HIDDEN     0x16
#define BLKSYS_NTFS_HIDDEN      0x17
#define BLKSYS_FAT32_HIDDEN     0x1b
#define BLKSYS_FAT32_LBA_HIDDEN 0x1c
#define BLKSYS_FAT16_LBA_HIDDEN 0x1e
#define BLKSYS_LINUX_SWAP1      0x42
#define BLKSYS_LINUX_SWAP2      0x82
#define BLKSYS_EFI              0xee



/* Additional filesystem types for EFI-compatibility. */
#define BLKSYS_EFI_UNUSED           0x10000 /*< Placeholder used internally to skip empty EFI partitions. */
#define BLKSYS_EFI_PARTEND          0x10001 /*< An empty EFI partition used to terminate the parser. */
#define BLKSYS_EXPLICIT             0x10002 /*< A filesystem type that must be explicitly mounted, and is never auto-detected. */
/* One of: BLKSYS_FAT12, BLKSYS_FAT16ALT, BLKSYS_FAT16, BLKSYS_NTFS,
 *         BLKSYS_FAT32, BLKSYS_FAT32_LBA, BLKSYS_FAT16_LBA, BLKSYS_FAT12_HIDDEN,
 *         BLKSYS_FAT16_32M_HIDDEN, BLKSYS_FAT16_HIDDEN, BLKSYS_NTFS_HIDDEN,
 *         BLKSYS_FAT32_HIDDEN, BLKSYS_FAT32_LBA_HIDDEN; BLKSYS_FAT16_LBA_HIDDEN */
#define BLKSYS_MICROSOFT_BASIC_DATA 0x10003

#define BLKSYS_ISEXTENDED(sysid) ((sysid) == BLKSYS_EXTENDED1 || (sysid) == BLKSYS_EXTENDED2)




#define BLOCKBUF_FLAG_NONE 0x00000000
#define BLOCKBUF_FLAG_LOAD 0x00000001 /*< 'bb_data' has been filled with disk data. */
#define BLOCKBUF_FLAG_CHNG 0x00000002 /*< Changes were made to the buffer and must be flushed before deletion. */

struct blockbuf {
 blkaddr_t bb_id;   /*< [lock(:bs_lock)] Block ID that this buffer is describing. */
 u32       bb_flag; /*< [lock(:bs_lock)] Buffer flags (Set of 'BLOCKBUF_FLAG_*') */
 byte_t   *bb_data; /*< [lock(:bs_lock)][1..:bd_blocksize][owned] Pre-allocated block buffer. */
};


#define BLOCKBUFFER_DEFAULT_MAX 16 /* Default max buffer count. */
struct blockbuffers {
 rwlock_t         bs_lock; /*< [order(BEFORE(:bd_hwlock))] Lock for controlling buffer usage. */
 size_t           bs_bufc; /*< [lock(bs_lock)][<= bs_bufa] Amount of buffered blocks. */
 size_t           bs_bufa; /*< [lock(bs_lock)][<= bs_bufm] Allocated amount of buffers. */
 size_t           bs_bufm; /*< [lock(bs_lock)][!0] Max amount of blocks that should be buffered. */
 struct blockbuf *bs_bufv; /*< [lock(bs_lock)][0..bs_bufc|alloc(bs_bufa)] Vector of allocated block buffers. */
};


struct diskpart;
struct blkdev {
 struct device              bd_device;     /*< The underlying device. */
 blksize_t                  bd_blocksize;  /*< [const] Size of a single block (in bytes). */
 blkcnt_t                   bd_blockcount; /*< [const] Amount of available blocks. */
 blksys_t                   bd_system;     /*< [const] The type of system stored on this device. */
 atomic_rwlock_t            bd_partlock;   /*< Lock for accessing 'bd_partitions'. */
 LIST_HEAD(struct diskpart) bd_partitions; /*< [lock(bd_partlock)][weak][0..1] List of sub-partitions of this block-device (Sorted by partition number).
                                            *  WARNING: Pointers in this chain are weakly linked, meaning that you need to try-incref
                                            *           them, or check their reference counter before unlocking 'bd_partlock'
                                            *  NOTE: This chain is _always_ empty for partition devices themself. */
#ifdef __INTELLISENSE__
 struct { struct blkdev    *le_next,**le_pself; }
                            bd_neighbor;   /*< [const][head(bd_contains)] Neighboring block devices. */
 struct blkdev             *bd_contains;   /*< [const][list(bd_neighbor)] List of contained block devices. */
#else
 LIST_NODE(struct blkdev)   bd_neighbor;   /*< [const][head(bd_contains)] Neighboring block devices. */
 LIST_HEAD(struct blkdev)   bd_contains;   /*< [const][list(bd_neighbor)] List of contained block devices. */
#endif
 struct blockbuffers        bd_buffer;     /*< Buffer/byte-wise abstractor for this block-device. */
 rwlock_t                   bd_hwlock;     /*< [order(AFTER(:bd_hwlock))] Device lock held when reading/writing. */
 /* Read/write full blocks to/from the device (bufsize is floor-aligned to 'bd_blocksize')
  * NOTE: These functions are caller-synchronized using 'bd_hwlock'.
  * NOTE: These functions must _always_ be assigned!
  * @return: * :         The actual amount of blocks read/written.
  * @return: -EFAULT:    A faulty buffer pointer was provided.
  * @return: E_ISERR(*): Failed to read/write data for some reason. */
 ssize_t (KCALL *bd_read)(struct blkdev *__restrict self, blkaddr_t block,
                          USER void *buf, size_t n_blocks);
 ssize_t (KCALL *bd_write)(struct blkdev *__restrict self, blkaddr_t block,
                           USER void const *buf, size_t n_blocks);

};
#define BLKDEV_FOREACH_PARTITION(part,self) \
          LIST_FOREACH(part,(self)->bd_partitions,dp_chain)

#define BLKDEV_TRYINCREF(self)  DEVICE_TRYINCREF(&(self)->bd_device)
#define BLKDEV_INCREF(self)     DEVICE_INCREF(&(self)->bd_device)
#define BLKDEV_DECREF(self)     DEVICE_DECREF(&(self)->bd_device)
#define BLKDEV_ISREADONLY(self) DEVICE_ISREADONLY(&(self)->bd_device)
#define BLKDEV_ISPART(self)   ((self)->bd_read == &diskpart_read)
#define BLKDEV_ID(self)       ((self)->bd_device.d_id)

FUNDEF ssize_t KCALL
diskpart_read(struct blkdev *__restrict self, blkaddr_t block,
              USER void *buf, size_t n_blocks);


/* Create a new block device. The caller must fill in:
 *  - bd_device.d_node.i_super (Use 'device_setup')
 *  - bd_read
 *  - bd_write
 *  - bd_blocksize
 *  - bd_blockcount
 *  - bd_system (Optionally) */
#define blkdev_new(type_size) \
        blkdev_cinit((struct blkdev *)calloc(1,type_size))
FUNDEF struct blkdev *KCALL blkdev_cinit(struct blkdev *self);

/* Destructor that must be called when destructing a block-device.
 * NOTE: This function must be called from 'bd_device.d_node.i_ops->ino_fini'
 * WARNING: This function implicitly calls 'device_fini()' */
FUNDEF void KCALL blkdev_fini(struct blkdev *__restrict self);

/* Read/write full blocks to/from a block device.
 * @return: * :         The actual amount of blocks read/written.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -EFAULT:    A faulty buffer pointer was provided.
 * @return: E_ISERR(*): Failed to read/write data for some reason. */
LOCAL ssize_t KCALL blkdev_raw_read(struct blkdev *__restrict self, blkaddr_t block, USER void *buf, size_t n_blocks);
LOCAL ssize_t KCALL blkdev_raw_write(struct blkdev *__restrict self, blkaddr_t block, USER void const *buf, size_t n_blocks);

/* Read/write low-level data to/from a block device.
 * NOTE: Unlike 'blkdev_raw_(read|write)', these functions control byte-wise,
 *       where 'offset == 0' is located at the start of the disk.
 * @return: * :         The actual amount of bytes read/written.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -EFAULT:    A faulty buffer pointer was provided.
 * @return: E_ISERR(*): Failed to read/write data for some reason. */
FUNDEF ssize_t KCALL blkdev_read(struct blkdev *__restrict self, pos_t offset, USER void *buf, size_t bufsize);
FUNDEF ssize_t KCALL blkdev_read2(struct blkdev *__restrict self, pos_t offset, USER void *buf, size_t bufsize);
FUNDEF ssize_t KCALL blkdev_write(struct blkdev *__restrict self, pos_t offset, USER void const *buf, size_t bufsize);
LOCAL errno_t KCALL blkdev_readall(struct blkdev *__restrict self, pos_t offset, USER void *buf, size_t bufsize);
LOCAL errno_t KCALL blkdev_writeall(struct blkdev *__restrict self, pos_t offset, USER void const *buf, size_t bufsize);

/* Flush all unwritten data to disk.
 * @return: -EOK:       Successfully flushed all data, or no data needed to be flushed.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): Failed to write data for some reason. */
FUNDEF errno_t KCALL blkdev_flush(struct blkdev *__restrict self);

/* Find the first partition of 'self' matching '(return->dp_device.bd_system & mask) == type'
 * NOTE: In the event that it was impossible to acquire a reference to a matching
 *       partition ('DISKPART_TRYINCREF()' failed), the partition is considered not to
 *       match the given requirements, meaning that the next match, or NULL will be returned.
 * @return: * :   A new reference to the first partition matching the above requirements.
 * @return: NULL: No partition matching the given requirements was found. */
FUNDEF SAFE REF struct diskpart *KCALL
blkdev_find_partition(struct blkdev *__restrict self,
                      blksys_t mask, blksys_t type);



struct diskpart {
 struct blkdev              dp_device; /*< Underlying block-device. */
 REF struct blkdev         *dp_ref;    /*< [1..1][const] Reference to the underlying block-device. */
 blkaddr_t                  dp_start;  /*< [const] Partition start block id. */
 LIST_NODE(struct diskpart) dp_chain;  /*< [lock(dp_ref->bd_partlock)][sort(ASCENDING(DISKPART_ID(*)))]
                                        *   Sorted chain of partitions.
                                        *   WARNING: Disk partition devices may not be linked
                                        *            at times, even though 'dp_ref' always is. */
};
#define DISKPART_TRYINCREF(self) BLKDEV_TRYINCREF(&(self)->dp_device)
#define DISKPART_INCREF(self)    BLKDEV_INCREF(&(self)->dp_device)
#define DISKPART_DECREF(self)    BLKDEV_DECREF(&(self)->dp_device)
#define DISKPART_ID(self) ((MINOR((self)->dp_device.bd_device.d_id)- \
                            MINOR((self)->dp_ref->bd_device.d_id))-1)

/* Create and return a new, unregistered partition within 'self'.
 * NOTE: When 'self' is also a partition device (BLKDEV_ISPART(self) == true),
 *       the partition is created within the actual underlying block device.
 * NOTE: Partition start & size are automatically clamped to the limitations
 *       of the underlying block-device, and when out-of-bounds, an empty
 *       partition is created instead.
 * NOTE: The partition will automatically be registered within
 *      'ns_blkdev' upon success, using a device id offset from
 *       the underlying block-device by 'partid'.
 * NOTE: This function is usually only executed from 'autopart_callback' callbacks.
 * WARNING: The caller is responsible for registering the underlying block-device
 *         (using 'devns_insert') before attempting to create sub-partitions.
 * @param: self:     The block device to sub-partition.
 * @param: start:    The first block apart of the partition.
 * @param: size:     The amount of blocks apart of the partition.
 * @param: sysid:    One of 'BLKSYS_*', optionally or'd with flags describing the kind of partition.
 * @param: partid:   The id of the partition to-be created (effects order with the block-devices )
 * @return: * :      A reference to a newly allocated disk-partition device.
 *             NOTE: Since the partition is also allocate within the block-device namespace,
 *                   the caller is safe to decref() this pointer without needing to do
 *                   anything else to finalize its setup.
 * @return: -EEXIST: The given block-device 'self' already contained a partition at 'partid'
 * @return: -EPERM:  The module instance associated with 'self' doesn't permit new references being created.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF SAFE REF struct diskpart *KCALL blkdev_mkpart(struct blkdev *__restrict self,
                                                     blkaddr_t start, blkaddr_t size,
                                                     blksys_t sysid, size_t partid);

/* Automatically partition the given block-device, registering partitions.
 * WARNING: The caller is responsible for registering the given block-device
 *         (using 'devns_insert') before attempting to call this function.
 * NOTES:
 *   - No-op when 'max_parts' is ZERO(0).
 *   - In the event of an error, a system log entry has been written,
 *     meaning that the caller may safely ignore errors returned by
 *     this function.
 *   - This function will look at the 'self->bd_system' to determine
 *     the proper actions to-be taken for automatically partitioning
 *     the given block-device:
 *     #1: Call all registered auto-partitioning callbacks registered
 *         using 'struct autopart::ap_sysid&BLKSYS_MASK == self->bd_system&BLKSYS_MASK'
 *         NOTE: Execution is performed in an undefined order.
 *     #2: If 'self->bd_system&BLKSYS_MASK' isn't 'BLKSYS_ANY',
 *         execute all auto-partitioning callbacks registered for 'BLKSYS_ANY'.
 *     #3: Return ZERO(0) (Nothing could be partitioned).
 *     HINT: A callback ('autopart_callback') must indicate its
 *           inability to partition a device by returning '-EINVAL'.
 * @param: max_parts:    The max amount of partitions possible (based on device-id restrictions, usually '63')
 * @return: 0 :          Failed to partition the block-device:
 *                        - It may be empty
 *                        - It may not be partitioned at all (maybe try operating on the drive itself?)
 *                        - Drivers required for loading its partition table may be missing
 * @return: * :         [<= max_parts] The actual amount of partitions created.
 * @return: -ENOMEM:    [max_parts != 0] Not enough available memory.
 * @return: E_ISERR(*): [max_parts != 0] Failed to create partitions for some reason. */
FUNDEF SAFE ssize_t KCALL blkdev_autopart(struct blkdev *__restrict self, size_t max_parts);





/* Automatically partition a given block device, using a partitioning
 * technique specific to the implementer (e.g.: 'MBR' auto-partitioning)
 * NOTE: The control flow of this function usually executes as follows:
 *       >> ssize_t temp; size_t result = 0;
 *       >> sector = READ_FIRST_SECTOR(self);
 *       >> if (!IS_VALID_PARTITION_SECTOR(sector))
 *       >>      return -EINVAL;
 *       >> FOREACH_PARTITION_IN_SECTOR(partition,sector) {
 *       >>     struct diskpart *part;
 *       >>     if (!max_parts) break;
 *       >>     
 *       >>     // Skip empty partitions.
 *       >>     if (!GET_PARTITION_SIZE(partition)) continue;
 *       >>     
 *       >>     // Create the new partition.
 *       >>     part = blkdev_mkpart(self,
 *       >>                          GET_PARTITION_START(partition),
 *       >>                          GET_PARTITION_SIZE(partition),
 *       >>                          GET_PARTITION_SYSID(partition),
 *       >>                          result);
 *       >>     if (E_ISERR(part)) return E_GTERR(part);
 *       >>     ++result,--max_parts;
 *       >>     // Must not read sub-partitions that start where we being ourself.
 *       >>     // Without this check, 'blkdev_autopart()' may call us again, causing
 *       >>     // an infinite loop that'll result in a kernel-level stack overflow,
 *       >>     // causing a triple fault and the computer to reboot.
 *       >>     temp = 0;
 *       >>     if (GET_PARTITION_START(partition) != 0) {
 *       >>         // Automatically sub-partition our new partition.
 *       >>         temp = blkdev_autopart(&part->dp_device,max_parts);
 *       >>     }
 *       >>     // Drop the reference returned by 'blkdev_mkpart()'
 *       >>     DISKPART_DECREF(part);
 *       >>     if (E_ISERR(temp)) return temp;
 *       >>     result    += temp;
 *       >>     max_parts -= temp;
 *       >> }
 *       >> return (ssize_t)result;
 * @return: * :         The actual amount of newly created partitions.
 * @return: -EINVAL:    The given block-device is corrupt or unrecognized by the driver.
 * @return: -ENOMEM:    Failed to allocate memory (such as for a new sub-partition)
 * @return: E_ISERR(*): Failed to create partitions for some reason.
 * NOTE: Even in the event of an error, some partitions may have already been created. */
typedef SAFE ssize_t (KCALL *autopart_callback)(struct blkdev *__restrict self,
                                                size_t max_parts, void *closure);
struct autopart {
 SLIST_NODE(struct autopart)
                           ap_chain;    /*< [lock(INTERNAL(::autopart_lock))] Chain of registered loaders. */
 WEAK REF struct instance *ap_owner;    /*< [1..1][const] Owner module (set to 'THIS_INSTANCE'). */
 blksys_t                  ap_sysid;    /*< [const] System id or 'BLKSYS_ANY', for which 'ap_callback' should
                                         *          be executed to automatically partition a block-device. */
 autopart_callback         ap_callback; /*< [1..1][const] Automatic partitioning callback. */
 void                     *ap_closure;  /*< [?..?][const] User-defined closure callback for 'ap_callback'. */
};

/* Register/Delete a given auto-partition callback.
 * NOTE: For details, see 'autopart_callback' and 'blkdev_autopart()' */
FUNDEF SAFE void KCALL blkdev_addautopart(struct autopart *__restrict self);
/* @return: true:  Successfully removed the auto-partitioning callback.
 * @return: false: The given auto-partitioning callback was never added. */
FUNDEF SAFE bool KCALL blkdev_delautopart(struct autopart *__restrict self);





/*************************************************************************+
|*                                                                       *|
|*   Automatic/Manual filesystem superblock loading from block devices   *|
|*                                                                       *|
+*************************************************************************/



/* Automatically create a superblock for the given block-device.
 * @param: name:        The name of the filesystem type to mount, or NULL to use auto-mounting.
 * @param: namelen:     The length of 'name' is characters (Ignored when 'name' is NULL).
 * @return: * :         A new reference to the (newly created) filesystem, now connected to 'self'.
 * @return: -EINVAL:   [name == NULL] The given 'self' contains an invalid, or unknown filesystem
 *                     [name != NULL] The given device 'self' cannot be mounted as filesystem type 'name'.
 * @return: -ENODEV:   [name != NULL] Unknown filesystem type.
 * @return: -ENOMEM:    Not enough available kernel memory.
 * @return: E_ISERR(*): Failed to mount the block device for some reason. */
FUNDEF SAFE REF struct superblock *KCALL
blkdev_mksuper(struct blkdev *__restrict self,
               HOST char const *name, size_t namelen);



/* Try to create a filesystem superblock that using the given block-device 'dev'.
 * @return: * :         A reference to the (newly created) filesystem, now connected to 'self'.
 * @return: -EINVAL:    The given device 'self' cannot be mounted with the implementer's filesystem type.
 * @return: -ENOMEM:    Not enough available kernel memory.
 * @return: E_ISERR(*): Failed to create the filesystem for some reason. */
typedef SAFE REF struct superblock *(KCALL *fstype_callback)(struct blkdev *__restrict dev,
                                                             void *closure);

struct fstype {
 SLIST_NODE(struct fstype)
                           f_chain;    /*< [lock(INTERNAL(::fstype_lock))] Chain of registered loaders. */
 WEAK REF struct instance *f_owner;    /*< [1..1][const] Owner module (set to 'THIS_INSTANCE'). */
 blksys_t                  f_sysid;    /*< [const] System id or 'BLKSYS_ANY' or 'BLKSYS_EXPLICIT', for which 'f_callback'
                                        *          should be executed to create a filesystem driver for a block-device. */
 fstype_callback           f_callback; /*< [1..1][const] Automatic partitioning callback. */
 void                     *f_closure;  /*< [?..?][const] User-defined closure callback for 'f_callback'. */
 char const               *f_name;     /*< [0..1][const] Filesystem type name. */
};

/* Register/Delete a given filesystem type superblock-creation callback.
 * NOTE: For details, see 'fstype_callback' and 'blkdev_mksuper()' */
FUNDEF SAFE void KCALL fs_addtype(struct fstype *__restrict self);
/* @return: true:  Successfully removed the filesystem type callback.
 * @return: false: The given filesystem type callback was never added. */
FUNDEF SAFE bool KCALL fs_deltype(struct fstype *__restrict self);




/* [1..1][valid_if(WAS_CALLED(blkdev_bootdisk_initialize))]
 * [const] After detection during early boot, set to the
 *         disk device used by the BIOS to boot the kernel.
 *  NOTE: Although not guarantied, this device is usually an ata, or a bios-disk.
 *  NOTE: In the rare event that no disk was detected the first ATA driver
 *        available is used, or a pre-formated, small ramdisk is created.
 *  WARNING: Before 'blkdev_bootdisk_initialize' is called, this variable is NULL!
 *  NOTE: 'bootpart' is a partition of 'bootdisk', that is the effective, virtual drive used. */
FUNDEF SAFE ATTR_RETNONNULL REF struct blkdev *KCALL get_bootdisk(void);
FUNDEF SAFE ATTR_RETNONNULL REF struct blkdev *KCALL get_bootpart(void);
FUNDEF SAFE void KCALL get_bootdev(REF struct blkdev **__restrict pdisk,
                                   REF struct blkdev **__restrict ppart);

/* Override the automatically configured boot device (disk & partition).
 * NOTE: Useful when a late-boot disk driver notices that it can implement
 *       the (up until then) placeholder/fallback bios disk driver.
 * @requires: part == disk || (BLKDEV_ISPART(part) && ((struct diskpart *)part)->dp_ref == disk) */
FUNDEF SAFE void KCALL set_bootdev(struct blkdev *__restrict disk,
                                   struct blkdev *__restrict part);


#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Post module initialization: fill 'biosblkdev_boot' with the device associated
 *                             with the given drive, or NULL if no matching drive
 *                             was found, or exists.
 * NOTE: In case the given drive wasn't loaded during the initial
 *       pass, an attempt is made to create a new device.
 * NOTE: If the kernel can confirm that the boot drive can be accessed
 *       by use of some other driver not dependent on BIOS functions,
 *       that device will be used instead.
 * WARNING: This function is an init-call and must not
 *          be called once free-data has been released! */
INTDEF void KCALL blkdev_bootdisk_initialize(void);
#endif

#ifndef __INTELLISENSE__
#include <fs/superblock.h>
#include <fs/inode.h>

LOCAL ssize_t KCALL
blkdev_raw_read(struct blkdev *__restrict self, blkaddr_t block,
                  USER void *buf, size_t bufsize) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 CHECK_USER_DATA(buf,bufsize);
 CHECK_HOST_TEXT(self->bd_read,1);
 result = rwlock_write(&self->bd_hwlock);
 if (E_ISOK(result)) {
  result = (*self->bd_read)(self,block,buf,bufsize);
  rwlock_endwrite(&self->bd_hwlock);
 }
 return result;
}
LOCAL ssize_t KCALL
blkdev_raw_write(struct blkdev *__restrict self, blkaddr_t block,
                 USER void const *buf, size_t bufsize) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 CHECK_USER_TEXT(buf,bufsize);
 CHECK_HOST_TEXT(self->bd_write,1);
 if unlikely(BLKDEV_ISREADONLY(self)) return -EROFS;
 result = rwlock_write(&self->bd_hwlock);
 if (E_ISOK(result)) {
  result = (*self->bd_write)(self,block,buf,bufsize);
  rwlock_endwrite(&self->bd_hwlock);
 }
 return result;
}
LOCAL errno_t KCALL
blkdev_readall(struct blkdev *__restrict self,
               pos_t offset, USER void *buf,
               size_t bufsize) {
 while (bufsize) {
  ssize_t temp = blkdev_read(self,offset,buf,bufsize);
  if unlikely(!temp) temp = -ENOSPC;
  if (E_ISERR(temp)) return temp;
  *(uintptr_t *)&buf += temp;
  bufsize            -= temp;
  offset             += temp;
 }
 return -EOK;
}
LOCAL errno_t KCALL
blkdev_writeall(struct blkdev *__restrict self,
                pos_t offset, USER void const *buf,
                size_t bufsize) {
 while (bufsize) {
  ssize_t temp = blkdev_write(self,offset,buf,bufsize);
  if unlikely(!temp) temp = -ENOSPC;
  if (E_ISERR(temp)) return temp;
  *(uintptr_t *)&buf += temp;
  bufsize            -= temp;
  offset             += temp;
 }
 return -EOK;
}
#endif


DECL_END

#endif /* !GUARD_INCLUDE_DEV_BLKDEV_H */
