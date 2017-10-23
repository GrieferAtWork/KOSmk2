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
#ifndef GUARD_KERNEL_CORE_MODULES_AUTOPART_MBR_C
#define GUARD_KERNEL_CORE_MODULES_AUTOPART_MBR_C 1
#define _KOS_SOURCE 2

#include <dev/blkdev.h>
#include <hybrid/compiler.h>
#include <kernel/export.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <modules/mbr.h>

DECL_BEGIN


PRIVATE SAFE ssize_t KCALL
efi_autopart_at(struct blkdev *__restrict self,
                blkaddr_t start, size_t max_parts);


PRIVATE SAFE ssize_t KCALL
efi_autopart(struct blkdev *__restrict self,
             size_t max_parts, void *UNUSED(closure)) {
 return efi_autopart_at(self,0,max_parts);
}


PRIVATE SAFE ssize_t KCALL
mbr_autopart(struct blkdev *__restrict self,
             size_t max_parts, void *UNUSED(closure)) {
 ssize_t temp; size_t result = 0;
 mbr_t *mbr = omalloc(mbr_t);
 union part *iter,*end;
 if unlikely(!mbr) return -ENOMEM;
 HOSTMEMORY_BEGIN {
  temp = blkdev_readall(self,0,mbr,sizeof(mbr_t));
 }
 HOSTMEMORY_END;

 if (E_ISERR(temp)) {
  /* Handle small-disk errors as not supporting MBR partitioning. */
  if (temp == -ENOSPC)
      temp = -EINVAL;
  goto end;
 }

 /* Validate the MBR sector. */
 temp = -EINVAL;
 /* Validate the boot signature. */
 if (mbr->mbr_sig[0] != 0x55 ||
     mbr->mbr_sig[1] != 0xaa)
     goto end;

 result = 0;
 end = (iter = mbr->mbr_part)+COMPILER_LENOF(mbr->mbr_part);
 for (; iter != end; ++iter) {
  struct diskpart *part;
  blkaddr_t start,size; blksys_t sysid;
  if (!max_parts) break;

  sysid = iter->pt.pt_sysid;
  /* Ignore partitions with an empty system-id. */
  if (sysid == BLKSYS_EMPTY) continue;

  /* Figure out the start & size of this partition. */
  if (iter->pt.pt_bootable&PART_BOOTABLE_LBA48 &&
      iter->pt_48.pt_sig1 == PART48_SIG1 &&
      iter->pt_48.pt_sig2 == PART48_SIG2) {
   start = (blkaddr_t)iter->pt_48.pt_lbastart | ((blkaddr_t)iter->pt_48.pt_lbastarthi << 32);
   size  = (blkaddr_t)iter->pt_48.pt_lbasize  | ((blkaddr_t)iter->pt_48.pt_lbasizehi  << 32);
  } else {
   start = (blkaddr_t)iter->pt_32.pt_lbastart;
   size  = (blkaddr_t)iter->pt_32.pt_lbasize;
  }

  if (!size) continue;

  if (sysid == BLKSYS_EFI) {
   /* Special case: Load an EFI partition table.
    * NOTE: This overrules creation of an MBR partition, ignoring the MBR
    *       partition limit, thus allowing the EFI partition to grow beyond. */
   temp = efi_autopart_at(self,start,max_parts);
   /* NOTE: If EFI failed to load anything, still create a regular partition. */
   if (E_ISERR(temp)) syslog(LOG_WARN,"[MBR] Failed to load EFI partition table: %[errno]\n",(errno_t)-temp);
   if (temp != 0 && temp != -EINVAL) goto done_load;
   if (temp == 0) syslog(LOG_INFO,"[MBR] Loading empty EFI partition table as MBR partition\n");
  }
  /* NOTE: Although MBR partitions don't have names, we can
   *       still use the partitions table entry itself as UUID.
   *    >> As a matter of fact: 'union part' is 16 bytes long,
   *       which is the same size of a single EFI guid identifier...
   *       It still won't be perfect, but it sure as hell improves
   *       the odds of identifying duplicate disks once the boot
   *       process starts trying to get rid of the BIOS boot disk. */
  part = blkdev_mkpart(self,start,size,sysid,result,
                       NULL,iter,sizeof(union part));
  if (E_ISERR(part)) { temp = E_GTERR(part); goto end; }

  /* Log creation of the partition. */
  syslog(LOG_HW|LOG_INFO,
         "[MBR] Created partition #%d (%[dev_t]) for %I64u...%I64u of %[dev_t] (%I64ux%Iu bytes)\n",
        (int)DISKPART_ID(part),part->dp_device.bd_device.d_id,
        (u64)(part->dp_start),
        (u64)(part->dp_start+part->dp_device.bd_blockcount),
              part->dp_ref->bd_device.d_id,
        (u64)(part->dp_device.bd_blockcount),
              part->dp_device.bd_blocksize);

  ++result,--max_parts;
  /* Must not read sub-partitions that start where we being ourself.
   * Without this check, 'blkdev_autopart()' may call us again, causing
   * an infinite loop that'll result in a kernel-level stack overflow,
   * causing a triple fault and the computer to reboot. */
  temp = 0;
  if (start != 0 && sysid != BLKSYS_EFI) {
   /* Automatically sub-partition our new partition. */
   temp = blkdev_autopart(&part->dp_device,max_parts);
  }
  /* Drop the reference returned by 'blkdev_mkpart()' */
  DISKPART_DECREF(part);
done_load:
  if (E_ISERR(temp)) goto end;
  result    += temp;
  max_parts -= temp;
 }
 temp = (ssize_t)result;

end: 
 free(mbr);
 return temp;
}


PRIVATE struct autopart mbr_part = {
    .ap_owner    = THIS_INSTANCE,
    .ap_callback = &mbr_autopart,
    .ap_sysid    = BLKSYS_ANY,    /* MBR is a generic loader. */
};

/* Define 2 addition loaders for more precise
 * loading of extended partition tables. */
PRIVATE struct autopart mbr_extpart1 = {
    .ap_owner    = THIS_INSTANCE,
    .ap_callback = &mbr_autopart,
    .ap_sysid    = BLKSYS_EXTENDED1,
};
PRIVATE struct autopart mbr_extpart2 = {
    .ap_owner    = THIS_INSTANCE,
    .ap_callback = &mbr_autopart,
    .ap_sysid    = BLKSYS_EXTENDED2,
};

/* Still define a handler for EFI partition tables. */
PRIVATE struct autopart efi_part = {
    .ap_owner    = THIS_INSTANCE,
    .ap_callback = &efi_autopart,
    .ap_sysid    = BLKSYS_EFI,
};


PRIVATE MODULE_INIT void KCALL mbr_init(void) {
 blkdev_addautopart(&mbr_part);
 blkdev_addautopart(&mbr_extpart1);
 blkdev_addautopart(&mbr_extpart2);
 blkdev_addautopart(&efi_part);
}
#ifndef CONFIG_NO_MODULE_CLEANUP
PRIVATE MODULE_FINI void KCALL mbr_fini(void) {
 blkdev_delautopart(&efi_part);
 blkdev_delautopart(&mbr_extpart2);
 blkdev_delautopart(&mbr_extpart1);
 blkdev_delautopart(&mbr_part);
}
#endif

DECL_END

#ifndef __INTELLISENSE__
#include "efi.c.inl"
#endif

#endif /* !GUARD_KERNEL_CORE_MODULES_AUTOPART_MBR_C */
