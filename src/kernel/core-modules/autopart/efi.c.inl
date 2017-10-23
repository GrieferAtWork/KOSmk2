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
#ifndef GUARD_KERNEL_CORE_MODULES_AUTOPART_EFI_C_INL
#define GUARD_KERNEL_CORE_MODULES_AUTOPART_EFI_C_INL 1
#define _KOS_SOURCE 2

#include <dev/blkdev.h>
#include <dev/device.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <hybrid/byteswap.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <modules/efi.h>

DECL_BEGIN


typedef struct sysmap {
 guid_t   sm_guid;
 blksys_t sm_sysid;
} sysmap_t;

PRIVATE ATTR_COLDRODATA sysmap_t const efi_sysmap[] = {
    {INITIALIZE_GUID(EBD0A0A2,B9E5,4433,87C0,68B6B72699C7),BLKSYS_MICROSOFT_BASIC_DATA},
    {INITIALIZE_GUID(A2A0D0EB,E5B9,3344,87C0,68B6B72699C7),BLKSYS_MICROSOFT_BASIC_DATA},
    {INITIALIZE_GUID(6DFD5706,ABA4,C443,84E5,0933C84B4F4F),BLKSYS_MICROSOFT_BASIC_DATA},
    {INITIALIZE_GUID(E3C9E316,0B5C,4DB8,817D,F92DF00215AE),BLKSYS_FAT32_LBA},
    {INITIALIZE_GUID(734E5AFE,F61A,11E6,BC64,92361F002671),BLKSYS_FAT32},
    {INITIALIZE_GUID(c12a7328,f81f,11d2,ba4b,00a0c93ec93b),BLKSYS_FAT32}, /* EFI System partition. */
    {INITIALIZE_GUID(0657FD6D,A4AB,43C4,84E5,0933C84B4F4F),BLKSYS_LINUX_SWAP2},
    {INITIALIZE_GUID(48616821,4964,6F6E,744E,656564454649),BLKSYS_LINUX_SWAP2}, /* NOPE! This is grub */
    {INITIALIZE_GUID(00000000,0000,0000,0000,000000000000),BLKSYS_EFI_PARTEND},
//  {INITIALIZE_GUID(00000000,0000,0000,0000,000000000000),BLKSYS_EFI_UNUSED},
};


PRIVATE SAFE ssize_t KCALL
efi_autopart_at(struct blkdev *__restrict self,
                blkaddr_t start, size_t max_parts) {
 efi_t efi; ssize_t temp; size_t result = 0;
 pos_t partition_addr;
 size_t partition_entused;
 size_t partition_entsize;
 size_t partition_vecsize;
 HOSTMEMORY_BEGIN {
  temp = blkdev_read(self,start*self->bd_blocksize,&efi,sizeof(efi_t));
 }
 HOSTMEMORY_END;

 if (E_ISERR(temp)) goto end;

 /* Handle small-disk errors as not supporting MBR partitioning. */
 if unlikely((size_t)temp < offsetafter(efi_t,gpt_partition_count)) {
  temp = -EINVAL;
  goto end;
 }

 temp = -EINVAL;

 /* Valid the EFI header magic. */
 if (efi.gpt_signature[0] != EFIMAG0 ||
     efi.gpt_signature[1] != EFIMAG1 ||
     efi.gpt_signature[2] != EFIMAG2 ||
     efi.gpt_signature[3] != EFIMAG3 ||
     efi.gpt_signature[4] != EFIMAG4 ||
     efi.gpt_signature[5] != EFIMAG5 ||
     efi.gpt_signature[6] != EFIMAG6 ||
     efi.gpt_signature[7] != EFIMAG7)
     goto end;

 /* Warn if the header isn't large enough. */
 if (BSWAP_LE2H32(efi.gpt_hdrsize) <
     offsetafter(efi_t,gpt_partition_count)) {
  syslog(LOG_FS|LOG_WARN,"[EFI] GPT partition table header is too small (%I32u < %Iu)\n",
         BSWAP_LE2H32(efi.gpt_hdrsize),offsetafter(efi_t,gpt_partition_count));
 }
 /* Substitute missing members. */
 if (BSWAP_LE2H32(efi.gpt_hdrsize) < offsetafter(efi_t,gpt_partition_entsz))
     efi.gpt_partition_entsz = BSWAP_H2LE32(128);
 else if (BSWAP_LE2H32(efi.gpt_partition_entsz) < offsetafter(efi_part_t,p_part_max)) {
  syslog(LOG_FS|LOG_WARN,"[EFI] GPT partition table entries are too small (%I32u < %Iu)\n",
         BSWAP_LE2H32(efi.gpt_partition_entsz),offsetafter(efi_part_t,p_part_max));
  goto end;
 }
#if 1
 /* NOTE: Apparently (and kind-of correctly so), GPT addresses are absolute. */
#define GPT_ADDR(x)        (x)
#else
#define GPT_ADDR(x) (start+(x))
#endif

 partition_addr    = GPT_ADDR(BSWAP_LE2H64(efi.gpt_partition_start))*self->bd_blocksize;
 partition_vecsize = BSWAP_LE2H32(efi.gpt_partition_count);
 partition_entsize = BSWAP_LE2H32(efi.gpt_partition_entsz);
#if 0
 syslog(LOG_FS|LOG_MESSAGE,"partition_vecsize = %Iu\n",partition_vecsize);
 syslog(LOG_FS|LOG_MESSAGE,"partition_entsize = %Iu\n",partition_entsize);
#endif
 while (partition_vecsize) {
  efi_part_t part;
  partition_entused = sizeof(efi_part_t);
  if (partition_entused > partition_entsize)
      partition_entused = partition_entsize;
  HOSTMEMORY_BEGIN {
   temp = blkdev_read(self,partition_addr,&part,partition_entused);
  }
  HOSTMEMORY_END;
  if (E_ISERR(temp)) goto end;
  if unlikely((size_t)temp < partition_entused) {
   syslog(LOG_FS|LOG_WARN,"[EFI] Failed to read full partition entry (at %I64u, only read %Iu/%Iu)\n",
          partition_addr,temp,partition_entused);
   partition_entused = (size_t)temp;
  }
  if (partition_entused < offsetafter(efi_part_t,p_part_max)) {
   syslog(LOG_FS|LOG_ERROR,"[EFI] Partition entry at %I64u is too small (%Iu < %Iu)\n",
          partition_addr,partition_entused,offsetafter(efi_part_t,p_part_max));
  } else {
   REF struct diskpart *dp;
   blksys_t dp_sysid; sysmap_t const *iter;
   if (partition_entused < offsetafter(efi_part_t,p_flags))
       part.p_flags = BSWAP_H2LE64(0);

   dp_sysid = BLKSYS_UNKNOWN;
   /* Figure out the EFI system id. */
#if 0
   syslog(LOG_FS|LOG_WARN,"[EFI] Parition type GUID: {" GUID_PRINTF_FMT "}\n",
          GUID_PRINTF_ARG(&part.p_type_guid));
#endif
   for (iter = efi_sysmap; iter != COMPILER_ENDOF(efi_sysmap); ++iter) {
    if (!memcmp(&iter->sm_guid,&part.p_type_guid,sizeof(guid_t))) {
     dp_sysid = iter->sm_sysid;
     break;
    }
   }
   if (dp_sysid == BLKSYS_EFI_PARTEND) break;
   /* Skip unused partition entries. */
   if (dp_sysid == BLKSYS_EFI_UNUSED) goto next;
   if (dp_sysid == BLKSYS_UNKNOWN) {
    syslog(LOG_FS|LOG_WARN,"[EFI] Unknown partition type GUID: {" GUID_PRINTF_FMT "}\n",
           GUID_PRINTF_ARG(&part.p_type_guid));
   }

   if (part.p_part_max < part.p_part_min) {
    syslog(LOG_FS|LOG_ERROR,"[EFI] Partition entry at %I64u has invalid min/max (max:%Iu < min:%Iu)\n",
           partition_addr,part.p_part_max,part.p_part_min);
   } else if (part.p_part_max == part.p_part_min) {
    syslog(LOG_FS|LOG_WARN,"[EFI] Partition entry at %I64u is empty\n",partition_addr);
    goto next;
   }

#if 0
   /* XXX: This is technically correct, but if we use this, KOS will
    *      attempt to use the grub partition as filesystem root... */
   if (part.p_flags&EFI_PART_F_ACTIVE)
       dp_sysid |= BLKSYS_ACTIVE;
#else
   /* According to my tutorial, the boot partition should be named 'kos'.
    * Until we've got something better, use that as indicator. */
   if (BSWAP_LE2H16(part.p_name[0]) == 'k' &&
       BSWAP_LE2H16(part.p_name[1]) == 'o' &&
       BSWAP_LE2H16(part.p_name[2]) == 's')
       dp_sysid |= BLKSYS_ACTIVE;
#endif

   /* Create the new partition. */
   { char name[COMPILER_LENOF(part.p_name)+1],*dst; le16 *src;
     name[COMPILER_LENOF(part.p_name)] = '\0';
     src = part.p_name,dst = name;
     /* Convert the partition name (XXX: in-kernel Unicode support? Libc already has it...) */
     for (; src != COMPILER_ENDOF(part.p_name); ++src,++dst)
           *dst = (char)BSWAP_LE2H16(*src);
     dp = blkdev_mkpart(self,GPT_ADDR(BSWAP_LE2H64(part.p_part_min)),
                        BSWAP_LE2H64(part.p_part_max)-BSWAP_LE2H64(part.p_part_min),
                        dp_sysid,result,name,(u8 *)&part.p_part_guid,sizeof(guid_t));
   }
   if (E_ISERR(dp)) { temp = E_GTERR(dp); goto end; }

   /* Mark the device as read-only if requested, to. */
   if (BSWAP_LE2H64(part.p_flags)&EFI_PART_F_READONLY)
       dp->dp_device.bd_device.d_node.i_state |= INODE_STATE_READONLY;

   syslog(LOG_FS|LOG_INFO,
          "[EFI] Created partition #%d: %q (%[dev_t]) for %I64u...%I64u of %[dev_t] (%I64ux%Iu bytes%s; system %I32x)\n",
         (int)DISKPART_ID(dp),dp->dp_name,dp->dp_device.bd_device.d_id,
         (u64)(dp->dp_start),
         (u64)(dp->dp_start+dp->dp_device.bd_blockcount),
               dp->dp_ref->bd_device.d_id,
         (u64)(dp->dp_device.bd_blockcount),
               dp->dp_device.bd_blocksize,
          BLKDEV_ISREADONLY(&dp->dp_device) ? "; read-only" : "",
          dp_sysid);
   ++result,--max_parts;
   /* NOTE: Don't sub-partition this drive. - EFI doesn't do that! */

   /* Drop the reference returned by 'blkdev_mkpart()' */
   DISKPART_DECREF(dp);
  }
next:
  --partition_vecsize;
  partition_addr += partition_entsize;
 }
 temp = (ssize_t)result;
end: 
 return temp;
}


DECL_END

#endif /* !GUARD_KERNEL_CORE_MODULES_AUTOPART_EFI_C_INL */
