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
#ifndef GUARD_MODULES_FS_DEVFS_C
#define GUARD_MODULES_FS_DEVFS_C 1
#define _KOS_SOURCE 1

#include <dev/device.h>
#include <fs/access.h>
#include <fs/dentry.h>
#include <fs/fs.h>
#include <fs/vfs.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <kernel/boot.h>
#include <kernel/export.h>
#include <sys/syslog.h>
#include <string.h>
#include <hybrid/section.h>
#include <stdio.h>

/* Other module headers that define device ids. */
#include <modules/ata.h>
#include <modules/bios-disk.h>
#include <modules/memdev.h>
#include <modules/ps2.h>
#include <modules/vga.h>
#include <fs/pty.h>


/* NOTE: This driver does not implement the device filesystem itself.
 *       The device filesystem is just an ordinary 'vsuperblock'
 *      (s.a.: '/src/include/fs/vfs.h'), with the addition of all
 *       device driver nodes being registered within it, meaning that
 *       it is the _only_ superblock in which device nodes can be
 *       allocated directly. - All other superblocks can only create
 *       indirect nodes that create weak aliases for device ids.
 *    >> Makes sense. - What devices are available is only known
 *       at runtime, meaning that a ~real~ filesystem implementing
 *       support for device files must somehow be able to to represent
 *       invalid device ids, whilst the /dev filesystem must not be
 *       able to to this! (s.a.: 'vnode_mknod()' in '/src/kernel/fs/vfs.c')
 */

DECL_BEGIN

#define DN_INT 0x0000 /*< Append an integer decimal for the device offset, excluding 'dn_min'. (e.g.: '/dev/hda', '/dev/hda1') */
#define DN_ALL 0x8000 /*< FLAG: Append the name-suffix to all entries, including 'dn_min' itself. */
struct devname {
 dev_t       dn_min;  /*< The lowest device number described by this. */
 u16         dn_num;  /*< [!0] The amount of device ids described by this. NOTE: ZERO in this field indicates a sentinel. */
 u16         dn_flag; /*< A set of 'DN_*' describing how unique name ids are generated. */
 char const *dn_name; /*< [1..1] The base name of the device. */
};

/* The magical device file-name table.
 * XXX: Load additional entries from a config file? */
PRIVATE struct devname const dnam_blk[] = {
    {BIOS_DISK_A,         64,DN_INT,"dos_hda"},
    {BIOS_DISK_B,         64,DN_INT,"dos_hdb"},
    {BIOS_DISK_C,         64,DN_INT,"dos_hdc"},
    {BIOS_DISK_D,         64,DN_INT,"dos_hdd"},
    {ATA_PRIMARY_MASTER,  64,DN_INT,"hda"},
    {ATA_PRIMARY_SLAVE,   64,DN_INT,"hdb"},
    {ATA_SECONDARY_MASTER,64,DN_INT,"hdc"},
    {ATA_SECONDARY_SLAVE, 64,DN_INT,"hdd"},
    {0,0,0,NULL},
};
PRIVATE struct devname const dnam_chr[] = {
    {DV_PS2_KEYBOARD,     1, DN_INT,"keyboard"},
    {DV_JIFFY_RTC,        1, DN_INT,"rtc"},
    {MD_MEM,              1, DN_INT,"mem"},
    {MD_KMEM,             1, DN_INT,"kmem"},
    {MD_NULL,             1, DN_INT,"null"},
    {MD_PORT,             1, DN_INT,"port"},
    {MD_ZERO,             1, DN_INT,"zero"},
    {MD_CORE,             1, DN_INT,"core"},
    {MD_FULL,             1, DN_INT,"full"},
    {MD_RANDOM,           1, DN_INT,"random"},
    {MD_URANDOM,          1, DN_INT,"urandom"},
    {MD_AIO,              1, DN_INT,"aio"},
    {MD_KMSG,             1, DN_INT,"kmsg"},
    {VGA_TTY,             1, DN_INT,"vga-tty"},
    {0,0,0,NULL},
};

/* The master callbacks for adding/deleting a given device to the /dev filesystem. */
PRIVATE errno_t KCALL device_add(struct device *__restrict dev, dev_t id);
PRIVATE void    KCALL device_del(struct device *__restrict dev, dev_t id);
PRIVATE errno_t KCALL devs_add(struct devns *__restrict UNUSED(self), struct device *__restrict dev, dev_t id) { return device_add(dev,id); }
PRIVATE void    KCALL devs_del(struct devns *__restrict UNUSED(self), struct device *__restrict dev, dev_t id) {        device_del(dev,id); }


PRIVATE struct devns_event devevent_chr = {
    .de_add   = &devs_add,
    .de_del   = &devs_del,
    .de_owner = THIS_INSTANCE,
};
PRIVATE struct devns_event devevent_blk = {
    .de_add   = &devs_add,
    .de_del   = &devs_del,
    .de_owner = THIS_INSTANCE,
};


PRIVATE void KCALL
devfs_load_existing_device(struct device *__restrict dev) {
again:
 CHECK_HOST_DOBJ(dev);
 /* Install this device. */
 device_add(dev,dev->d_id);

 if (dev->d_devnode.bt_min) {
  if (dev->d_devnode.bt_max)
      devfs_load_existing_device(dev);
  dev = dev->d_devnode.bt_min;
  goto again;
 }
 if (dev->d_devnode.bt_max) {
  dev = dev->d_devnode.bt_max;
  goto again;
 }
}

PRIVATE void KCALL
devfs_load_namespace(struct devns *__restrict ns,
                     struct devns_event *__restrict ev) {
 struct devns_major *iter;
 atomic_rwlock_read(&ns->d_lock);
 /* Enumerate all devices that have already been registered. */
 LIST_FOREACH(iter,ns->d_sort,ma_sort) {
  struct device **diter,**dend;
  dend = (diter = DEVNS_MAJOR_VEC(iter))+DEVNS_MAJOR_CNT(iter);
  for (; diter != dend; ++diter) {
   if (*diter) devfs_load_existing_device(*diter);
  }
 }
 devns_addevent(ns,ev);
 atomic_rwlock_endread(&ns->d_lock);
}

PRIVATE MODULE_INIT errno_t KCALL devfs_init(void) {
 /* Register the device namespace events, so we always get notified
  * when a new device is registered, or an existing one is removed. */
 devfs_load_namespace(&ns_chrdev,&devevent_chr);
 devfs_load_namespace(&ns_blkdev,&devevent_blk);
 return -EOK;
}

PRIVATE MODULE_FINI void KCALL devfs_fini(void) {
 devns_delevent(&ns_blkdev,&devevent_blk);
 devns_delevent(&ns_chrdev,&devevent_chr);
}


PRIVATE void KCALL
device_del(struct device *__restrict dev, dev_t id) {
 ssize_t error;
 /* Much simpler than add: Simply remove all bindings of the
  *                        device from the /dev filesystem. */
 error = superblock_remove_inode(&dev_fs.v_super,
                                 &dev->d_node);
 if (E_ISERR(error)) {
  syslog(LOG_DEBUG,
          FREESTR("[DEVFS] Failed to remove superblock at %q: %[errno]\n"),
          -error);
 }
}


PRIVATE errno_t KCALL
device_add(struct device *__restrict dev, dev_t id) {
 REF struct dentry *mount_path;
 struct devname const *iter;
 struct dentryname name;
 struct fsaccess ac;
 /* Nothing to do if the device filesystem isn't mounted. */
 if unlikely(!devfs_root) return -EOK;
 if (DEVICE_ISBLK(dev)) {
  iter = dnam_blk;
 } else if (DEVICE_ISCHR(dev)) {
  iter = dnam_chr;
 } else {
  /* Shouldn't happen, but ignore everything
   * that isn't a block or character device. */
  return -EOK;
 }

 /* Search for the appropriate name and create a new directory entry. */
 for (;; ++iter) {
  if (!iter->dn_num) goto unknown;
  if (iter->dn_min > id) continue;
  if (iter->dn_min+iter->dn_num <= id) continue;
  /* got it! */
  break;
 }
 assert(iter->dn_min <= id);
 assert(iter->dn_min+iter->dn_num > id);

 FSACCESS_SETHOST(ac);
 /* Load the device name string into the directory entry name. */
 name.dn_size = strlen(iter->dn_name);
 if (id == iter->dn_min && !(iter->dn_flag&DN_ALL)) {
  name.dn_name = (char *)iter->dn_name;
 } else {
  /* Create a custom name. */
  unsigned char offset = (unsigned char)(id-iter->dn_min);
  size_t base_size = name.dn_size;
  assert(offset <= 99);
  ++name.dn_size;
  if (offset >= 10) ++name.dn_size;
  name.dn_name = (char *)alloca((name.dn_size+1)*sizeof(char));
  memcpy(name.dn_name,iter->dn_name,base_size*sizeof(char));
  if (offset >= 10) {
   name.dn_name[base_size]   = '0'+(offset/10);
   name.dn_name[base_size+1] = '0'+(offset%10);
   name.dn_name[base_size+2] = '\0';
  } else {
   name.dn_name[base_size]   = '0'+offset;
   name.dn_name[base_size+1] = '\0';
  }
 }
got_name:
 dentryname_loadhash(&name);
 /* Actually insert the device as a node within the virtual device filesystem. */
 mount_path = dentry_insnod(devfs_root,&name,&ac,dev,NULL);

 if (E_ISERR(mount_path)) {
  syslog(LOG_FS|LOG_ERROR,
          "[DEVFS] Failed to install %s-device %[dev_t] (%q): %[errno]\n",
          DEVICE_ISBLK(dev) ? "block" : "character",id,name.dn_name,
          -E_GTERR(mount_path));
 } else {
  syslog(LOG_FS|LOG_MESSAGE,
          "[DEVFS] Added %s-device %[dev_t] as %[dentry]\n",
          DEVICE_ISBLK(dev) ? "block" : "character",id,mount_path);
  DENTRY_DECREF(mount_path);
 }

 /* NOTE: Even though this function could return an error, it never should,
  *       because doing so would prevent the device from being registered,
  *       meaning that the entirety of the device-load chain would fail,
  *       instead of the device simply lacking the proper file in /dev,
  *       an error, which we log by the way... */
 return -EOK;
unknown:
 switch (MAJOR(id)) {

 {
  minor_t drive_letter;
  minor_t drive_part;
  char *iter;
 case 14:
  /* /dev/dos_hd[abcdef...][1-63] */
  drive_letter = MINOR(id)/64;
  drive_part   = MINOR(id)%64;
#define BUFSIZE 64 /* Should never overflow... */
  name.dn_name = (char *)alloca(BUFSIZE);
  iter = name.dn_name+BUFSIZE;
  *--iter = '\0';
  if (drive_part) {
   *--iter = '0'+(drive_part % 10);
   if (drive_part >= 10) *--iter = '0'+(drive_part / 10);
  }
  while (drive_letter) {
   *--iter = 'a'+(drive_part % 26);
   drive_letter /= 26;
  }
  *--iter = 'd';
  *--iter = 'h';
  *--iter = '_';
  *--iter = 's';
  *--iter = 'o';
  *--iter = 'd';
  name.dn_size = (name.dn_name+(BUFSIZE-1))-iter;
  name.dn_name = iter;
  goto got_name;
#undef BUFSIZE
 } break;

 default: break;
 }

 /* Custom device naming conventions. */
 syslog(LOG_FS|LOG_MESSAGE,
         "[DEVFS] Unknown %s-device %[dev_t] not added to dev-fs\n",
         DEVICE_ISBLK(dev) ? "block" : "character",id);
 return -EOK;
}

DECL_END

#endif /* !GUARD_MODULES_FS_DEVFS_C */
