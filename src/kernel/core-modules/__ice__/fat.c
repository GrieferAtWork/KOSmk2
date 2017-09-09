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
#ifndef GUARD_KERNEL_MOD_FAT_C
#define GUARD_KERNEL_MOD_FAT_C 1
#define _KOS_SOURCE 1

#include <alloca.h>
#include <assert.h>
#include <ctype.h>
#include <hybrid/align.h>
#include <hybrid/byteswap.h>
#include <hybrid/check.h>
#include <kernel/blkdev.h>
#include <kernel/fs.h>
#include <kernel/syslog.h>
#include <kernel/types.h>
#include <kernel/user.h>
#include <malloc.h>
#include <modules/fat.h>
#include <stdint.h>
#include <string.h>
#include <sync/rwlock.h>
#include <sys/stat.h>

#include "fat.h"

/* Read-only FAT filesystem kernel module. */

DECL_BEGIN


/* Bits used in FAT metadata. */
#define FATMETA_CHANGED  01 /*< The  */
#define FATMETA_LOADED   02
#define FATMETA_BITS      2
#define FATMETA_PERBYTE  (8/FATMETA_BITS)

#define FATMETA_ADDR(cluster)                          ((cluster)/FATMETA_PERBYTE)
#define FATMETA_MASK(cluster,bits) ((byte_t)((bits) << ((cluster)%FATMETA_PERBYTE)*FATMETA_BITS))

#if FATMETA_BITS > 8 || 8 % FATMETA_BITS
#error "FIXME: Invalid amount of FAT meta bits"
#endif

#define SELF                        self
#define FAT_VALIDCLUSTER(cluster) ((cluster) < CEILDIV(SELF->f_fatsize,SELF->f_sec4fat))

#define FATBIT_ON(cluster,bits) \
 (assertf(FAT_VALIDCLUSTER(cluster),"Invalid cluster: %Iu",(size_t)(cluster)),\
  SELF->f_fatmeta[FATMETA_ADDR(cluster)] |= FATMETA_MASK(cluster,bits))
#define FATBIT_OFF(cluster,bits) \
 (assertf(FAT_VALIDCLUSTER(cluster),"Invalid cluster: %Iu",(size_t)(cluster)),\
  SELF->f_fatmeta[FATMETA_ADDR(cluster)] &= ~FATMETA_MASK(cluster,bits))
#define FAT_CHANGED(cluster) \
 (FATBIT_ON(cluster,FATMETA_CHANGED),\
  SELF->f_flags |= KFATFS_FLAG_FATCHANGED)\

#define FAT_WRITE(cluster,value)  ((*SELF->f_writefat)(SELF,cluster,value))
#define FAT_WRITE_AND_UPDATE(cluster,value) \
 (FAT_WRITE(cluster,value),FAT_CHANGED(cluster))

/* Returns the sector offset associated with a given cluster. */
#define /*fatoff_t*/FAT_GETCLUSTEROFFSET(cluster) (*SELF->f_getfatsec)(SELF,cluster)
#define /*fatsec_t*/FAT_GETCLUSTERADDR(offset)    (SELF->f_firstfatsec+(offset))

#define LOCK_WRITE_BEGIN()   rwlock_write(&SELF->f_fatlock)
#define LOCK_WRITE_END()     rwlock_endwrite(&SELF->f_fatlock)
#define LOCK_READ_BEGIN()    rwlock_read(&SELF->f_fatlock)
#define LOCK_READ_END()      rwlock_endread(&SELF->f_fatlock)
#define LOCK_DOWNGRADE()     rwlock_downgrade(&SELF->f_fatlock)

PRIVATE errno_t KCALL
kfatfs_fat_freeall(fat_t *__restrict self, fatcls_t first) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 if (E_ISERR(error = LOCK_WRITE_BEGIN())) return error;
 error = kfatfs_fat_freeall_unlocked(self,first);
 LOCK_WRITE_END();
 return error;
}

PRIVATE errno_t KCALL
kfatfs_fat_freeall_unlocked(fat_t *__restrict self, fatcls_t first) {
 errno_t error; fatcls_t next;
 CHECK_HOST_DOBJ(self);
 error = kfatfs_fat_read_unlocked(self,first,&next);
 if (E_ISERR(error)) return error;
 if (!fat_iseofcluster(self,next)) {
  error = kfatfs_fat_freeall_unlocked(self,next);
  if (E_ISERR(error)) return error;
 }
 /* Actually mark the cluster as unused. */
 FAT_WRITE_AND_UPDATE(first,KFATFS_CUSTER_UNUSED);
 return -EOK;
}

PRIVATE errno_t KCALL
kfatfs_fat_allocfirst(fat_t *__restrict self, fatcls_t *__restrict target) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 if (E_ISERR(error = LOCK_WRITE_BEGIN())) return error;
 error = kfatfs_fat_getfreecluster_unlocked(self,target,0);
 if (E_ISOK(error)) {
  /* If a cluster was found, link the previous one,
   * and mark the new one as pointing to EOF */
  FAT_WRITE_AND_UPDATE(*target,self->f_clseof_marker);
 }
 LOCK_WRITE_END();
 return error;
}
PRIVATE errno_t KCALL
kfatfs_fat_readandalloc(fat_t *__restrict self, fatcls_t index, fatcls_t *__restrict target) {
 errno_t error; size_t meta_addr;
 byte_t meta_mask,meta_val; fatoff_t fat_index;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DATA(self->f_fatv,self->f_fatsize);
 assert(self->f_secsize);
 assertf(index < self->f_clseof,"Given FAT byte is out-of-bounds");
 fat_index = FAT_GETCLUSTEROFFSET(index);
 meta_addr = FATMETA_ADDR(fat_index);
 meta_mask = FATMETA_MASK(fat_index,FATMETA_LOADED);
 if (E_ISERR(error = rwlock_read(&self->f_fatlock))) return error;
 meta_val = self->f_fatmeta[meta_addr];
 if (meta_val&meta_mask) {
  /* Entry is already cached (no need to start writing) */
  *target = (*self->f_readfat)(self,index);
  if (!fat_iseofcluster(self,*target)) {
   rwlock_endread(&self->f_fatlock);
   goto end;
  }
 }
 error = rwlock_upgrade(&self->f_fatlock);
 if (E_ISERR(error)) return error;
 meta_val = self->f_fatmeta[meta_addr];
 /* Make sure that no other task loaded the
  * FAT while we were switching to write-mode. */
 if (meta_val&meta_mask) {
  *target = (*self->f_readfat)(self,index);
  error   = -EOK;
 } else {
  /* Entry is not cached --> Load the fat's entry. */
  error = fat_rsec(self,FAT_GETCLUSTERADDR(fat_index),
                         (void *)((uintptr_t)self->f_fatv+fat_index*self->f_secsize),
                          self->f_secsize);
  if (E_ISERR(error)) goto end_write;
  *target = (*self->f_readfat)(self,index);
  /* Mark the FAT cache entry as loaded. */
  assert(self->f_fatmeta[meta_addr] == meta_val);
  self->f_fatmeta[meta_addr] = meta_val|meta_mask;
 }
 if (fat_iseofcluster(self,*target)) {
  syslogf(LOG_FS|LOG_INFO,
          "Allocating FAT cluster after EOF %I32u from %I32u (out-of-bounds for %I32u)\n",
          *target,index,self->f_clseof);
  /* Search for a free cluster (We prefer to use the nearest chunk to reduce fragmentation) */
  error = kfatfs_fat_getfreecluster_unlocked(self,target,index+1);
  if (E_ISOK(error)) {
   /* If a cluster was found, link the previous one,
    * and mark the new one as pointing to EOF. */
   FAT_SETFAT(self,index,*target);
   FAT_SETFAT(self,*target,self->f_clseof_marker);
   /* Don't forget to mark the cache as modified */
   self->f_fatmeta[meta_addr] |= meta_mask|(meta_mask >> 1);
   /* Also need to mark the newly allocated cluster as modified */
   fat_index = FAT_GETCLUSTEROFFSET(*target);
   meta_addr = FATMETA_ADDR(fat_index);
   meta_mask = FATMETA_MASK(fat_index,FATMETA_CHANGED);
   self->f_fatmeta[meta_addr] |= meta_mask;
   /* Finally, set the global flag indicating a change */
   self->f_flags |= KFATFS_FLAG_FATCHANGED;
  }
 }
end_write:
 LOCK_WRITE_END();
end:
 return error;
}

PRIVATE errno_t KCALL
kfatfs_fat_read(fat_t *__restrict self, fatcls_t index, fatcls_t *__restrict target) {
 errno_t error; size_t meta_addr;
 byte_t meta_mask,meta_val; fatoff_t fat_index;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DATA(self->f_fatv,self->f_fatsize);
 assert(self->f_secsize);
 assertf(index < self->f_clseof,"Given FAT byte is out-of-bounds");
 fat_index = FAT_GETCLUSTEROFFSET(index);
 meta_addr = FATMETA_ADDR(fat_index);
 meta_mask = FATMETA_MASK(fat_index,FATMETA_LOADED);
 if (E_ISERR(error = rwlock_read(&self->f_fatlock))) return error;
 meta_val = self->f_fatmeta[meta_addr];
 if (meta_val&meta_mask) {
  /* Entry is already cached (no need to start writing) */
  *target = (*self->f_readfat)(self,index);
  rwlock_endread(&self->f_fatlock);
  return -EOK;
 }
 error = rwlock_upgrade(&self->f_fatlock);
 if (E_ISERR(error)) return error;
 meta_val = self->f_fatmeta[meta_addr];
 /* Make sure that no other task loaded the
  * FAT while we were switching to write-mode. */
 if __unlikely(meta_val&meta_mask) {
  *target = (*self->f_readfat)(self,index);
  LOCK_WRITE_END();
  return -EOK;
 }
 /* Entry is not cached --> Load the fat's entry. */
 error = fat_rsec(self,FAT_GETCLUSTERADDR(fat_index),
                        (void *)((uintptr_t)self->f_fatv+fat_index*self->f_secsize),
                         self->f_secsize);
 if (E_ISOK(error)) {
  *target = (*self->f_readfat)(self,index);
  /* Mark the FAT cache entry as loaded. */
  assert(self->f_fatmeta[meta_addr] == meta_val);
  self->f_fatmeta[meta_addr] = meta_val|meta_mask;
 }
 LOCK_WRITE_END();
 return error;
}
PRIVATE errno_t KCALL
kfatfs_fat_read_unlocked(fat_t *__restrict self, fatcls_t index, fatcls_t *__restrict target) {
 errno_t error; size_t meta_addr;
 byte_t meta_mask,meta_val; fatoff_t fat_index;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DATA(self->f_fatv,self->f_fatsize);
 assert(self->f_secsize);
 assertf(index < self->f_clseof,"Given FAT byte is out-of-bounds");
 assert(FAT_VALIDCLUSTER(index));
 fat_index = FAT_GETCLUSTEROFFSET(index);
 meta_addr = FATMETA_ADDR(fat_index);
 meta_mask = FATMETA_MASK(fat_index,FATMETA_LOADED);
 meta_val  = self->f_fatmeta[meta_addr];
 if (meta_val&meta_mask) {
  /* Entry is already cached */
  *target = (*self->f_readfat)(self,index);
  return -EOK;
 }
 meta_val = self->f_fatmeta[meta_addr];
 /* Make sure that no other task loaded the
  * FAT while we were switching to write-mode. */
 if __unlikely(meta_val&meta_mask) {
  *target = (*self->f_readfat)(self,index);
  LOCK_WRITE_END();
  return -EOK;
 }
 /* Entry is not cached --> Load the fat's entry. */
 error = fat_rsec(self,FAT_GETCLUSTERADDR(fat_index),
                        (void *)((uintptr_t)self->f_fatv+fat_index*self->f_secsize),
                         self->f_secsize);
 if (E_ISOK(error)) {
  *target = (*self->f_readfat)(self,index);
  /* Mark the FAT cache entry as loaded. */
  assert(self->f_fatmeta[meta_addr] == meta_val);
  self->f_fatmeta[meta_addr] = meta_val|meta_mask;
 }
 return error;
}
PRIVATE errno_t KCALL
kfatfs_fat_write(fat_t *__restrict self, fatcls_t index, fatcls_t target) {
 errno_t error; size_t meta_addr; byte_t meta_mask,meta_val;
 fatoff_t fat_index;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DATA(self->f_fatv,self->f_fatsize);
 assert(self->f_secsize);
 assertf(index < self->f_clseof,"Given FAT byte is out-of-bounds");
 fat_index = FAT_GETCLUSTEROFFSET(index);
 meta_addr = FATMETA_ADDR(fat_index);
 meta_mask = FATMETA_MASK(fat_index,FATMETA_LOADED);

 /* Make sure the FAT cache is loaded */
 if (E_ISERR(error = LOCK_WRITE_BEGIN())) return error;
 meta_val = self->f_fatmeta[meta_addr];
 if (!(meta_val&meta_mask)) { /* Load the fat's entry. */
  error = fat_rsec(self,FAT_GETCLUSTERADDR(fat_index),
                         (void *)((uintptr_t)self->f_fatv+fat_index*self->f_secsize),
                          self->f_secsize);
  if (E_ISERR(error)) { LOCK_WRITE_END(); return error; }
  meta_val |= meta_mask;
 }
 meta_mask >>= 1; /* Switch to the associated changed-mask */
 assert(meta_mask);
 /* Actually write the cache */
 FAT_SETFAT(self,index,target);
 if (!(meta_val&meta_mask)) {
  /* New change --> mark the chunk as such
   * Also update the changed flag in the FS (which is used to speed up flushing) */
  self->f_flags |= KFATFS_FLAG_FATCHANGED;
  meta_val |= meta_mask;
 }
 /* Update the metadata flags */
 self->f_fatmeta[meta_addr] = meta_val;
 LOCK_WRITE_END();
 return -EOK;
}

PRIVATE errno_t KCALL
kfatfs_fat_flush(fat_t *__restrict self) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DATA(self->f_fatv,self->f_fatsize);
 if (E_ISERR(error = LOCK_WRITE_BEGIN())) return error;
 if (self->f_flags&KFATFS_FLAG_FATCHANGED) {
  /* Changes were made, so we need to flush them! */
  error = kfatfs_fat_doflush_unlocked(self);
  if (E_ISOK(error)) self->f_flags &= ~(KFATFS_FLAG_FATCHANGED);
 }
 LOCK_WRITE_END();
 return error;
}
PRIVATE errno_t KCALL
kfatfs_fat_doflush_unlocked(fat_t *__restrict self) {
 errno_t error; byte_t *fat_data,*meta_data,metamask;
 fatsec_t i; size_t j;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DATA(self->f_fatv,self->f_fatsize);
 assert(self->f_fatsize == self->f_secsize*self->f_sec4fat);
 for (i = 0; i < self->f_sec4fat; ++i) {
  fat_data  = (byte_t *)self->f_fatv+(i*self->f_secsize);
  meta_data = (byte_t *)self->f_fatmeta+FATMETA_ADDR(i);
  metamask  = FATMETA_MASK(i,FATMETA_CHANGED);
  if (*meta_data&metamask) {
   /* Modified fat sector (flush it) */
   syslogf(LOG_FS|LOG_MESSAGE,
           "Saving modified FAT sector %I32u (%I8x %I8x)\n",
           i,*meta_data,metamask);
   /* Since there might be multiple FATs, we need to flush the changes to all of them!
    * todo: We can optimize this by flushing multiple consecutive sectors at once. */
   for (j = 0; j < self->f_fatcount; ++j) {
    error = fat_wsec(self,self->f_firstfatsec+i+j*self->f_sec4fat,
                            fat_data,self->f_secsize);
    if (E_ISERR(error)) return error;
   }
   *meta_data &= ~(metamask);
  }
 }
 return -EOK;
}

PRIVATE errno_t KCALL
kfatfs_fat_getfreecluster_unlocked(fat_t *__restrict self,
                                    fatcls_t *__restrict result,
                                    fatcls_t hint) {
 errno_t error; fatcls_t cls; size_t meta_addr;
 byte_t meta_mask,meta_val; fatoff_t fat_index;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DATA(self->f_fatv,self->f_fatsize);
 assert(self->f_secsize);
 /* TODO: Start searching for an empty cluster at 'hint' (but only if it's 'hint > 2') */
 for (cls = 2; cls < self->f_clseof; ++cls) {
  fat_index = FAT_GETCLUSTEROFFSET(cls);
  meta_addr = FATMETA_ADDR(fat_index);
  meta_mask = FATMETA_MASK(fat_index,FATMETA_LOADED);
  meta_val = self->f_fatmeta[meta_addr];
  if __unlikely(!(meta_val&meta_mask)) {
   /* Load the FAT entry, if it wasn't already */
   error = fat_rsec(self,FAT_GETCLUSTERADDR(fat_index),
                          (void *)((uintptr_t)self->f_fatv+fat_index*self->f_secsize),
                           self->f_secsize);
   if (E_ISERR(error)) return error;
   assert(self->f_fatmeta[meta_addr] == meta_val);
   self->f_fatmeta[meta_addr] = meta_val|meta_mask;
  }
  if ((*self->f_readfat)(self,cls) == KFATFS_CUSTER_UNUSED) {
   /* Found an unused cluster */
   *result = cls;
   return -EOK;
  }
 }
 return -ENOSPC;
}
#undef SELF
#undef FAT_VALIDCLUSTER
#undef FATBIT_ON
#undef FATBIT_OFF
#undef FAT_CHANGED
#undef FAT_WRITE
#undef FAT_WRITE_AND_UPDATE
#undef FAT_GETCLUSTEROFFSET
#undef FAT_GETCLUSTERADDR
#undef LOCK_WRITE_BEGIN
#undef LOCK_WRITE_END
#undef LOCK_READ_BEGIN
#undef LOCK_READ_END
#undef LOCK_DOWNGRADE





#define SECTOR_READ(i,buf,s)  (*FAT->f_readsec)(FAT,i,buf,s)
#define SECTOR_WRITE(i,buf,s) (*FAT->f_writesec)(FAT,i,buf,s)
#define BLOCKDEV                FAT->f_sb.sb_blkdev
#define FAT                     self

/* Read/Write callbacks for section interfacing. */
PRIVATE errno_t KCALL fat_rsec(fat_t *__restrict self, fatsec_t index,
                               USER void *buf, size_t bufsize) {
 ssize_t result;
 result = blkdev_read(BLOCKDEV,index*self->f_secsize,buf,bufsize);
 if (E_ISOK(result) && (result < bufsize)) result = -ENOSPC;
 return (errno_t)result;
}
PRIVATE errno_t KCALL fat_wsec(fat_t *__restrict self, fatsec_t index,
                               USER void const *buf, size_t bufsize) {
 ssize_t result;
 result = blkdev_write(BLOCKDEV,index*self->f_secsize,buf,bufsize);
 if (E_ISOK(result) && (result < bufsize)) result = -ENOSPC;
 return (errno_t)result;
}

PRIVATE fatoff_t KCALL fat_getfatsec_12(fat_t *__restrict self, fatcls_t index) { return (index+(index/2))/self->f_secsize; }
PRIVATE fatoff_t KCALL fat_getfatsec_16(fat_t *__restrict self, fatcls_t index) { return (index*2)/self->f_secsize; }
PRIVATE fatoff_t KCALL fat_getfatsec_32(fat_t *__restrict self, fatcls_t index) { return (index*4)/self->f_secsize; }
PRIVATE fatcls_t KCALL fat_readfat_12(fat_t *__restrict self, fatcls_t cluster) {
 size_t fatoff;
 fatoff = (cluster+(cluster/2)) % self->f_fatsize;
 __u16 val = (*(__u16 *)((uintptr_t)self->f_fatv+fatoff));
 if (cluster&1) val >>= 4; else val &= 0x0fff;
 return val;
}
PRIVATE fatcls_t KCALL
fat_readfat_16(fat_t *__restrict self, fatcls_t cluster) {
 size_t fatoff;
 fatoff = (cluster*2) % self->f_fatsize;
 return (*(__u16 *)((uintptr_t)self->f_fatv+fatoff));
}
PRIVATE fatcls_t KCALL
fat_readfat_32(fat_t *__restrict self, fatcls_t cluster) {
 size_t fatoff;
 fatoff = (size_t)((cluster*4) % self->f_fatsize);
 return (*(__u32 *)((uintptr_t)self->f_fatv+fatoff)) & 0x0FFFFFFF;
}
PRIVATE void KCALL
fat_writefat_12(fat_t *__restrict self,
                fatcls_t cluster, fatcls_t value) {
 size_t fatoff;
 fatoff = (cluster+(cluster/2)) % self->f_fatsize;
 __u16 *val = ((__u16 *)((uintptr_t)self->f_fatv+fatoff));
 if (cluster&1) {
  *val = (*val&0xf)|(value << 4);
 } else {
  *val |= value&0xfff;
 }
}
PRIVATE void KCALL
fat_writefat_16(fat_t *__restrict self,
                fatcls_t cluster, fatcls_t value) {
 size_t fatoff;
 fatoff = (cluster*2) % self->f_fatsize;
 (*(__u16 *)((uintptr_t)self->f_fatv+fatoff)) = value;
}
PRIVATE void KCALL
fat_writefat_32(fat_t *__restrict self,
                fatcls_t cluster, fatcls_t value) {
 size_t fatoff;
 fatoff = (size_t)((cluster*4) % self->f_fatsize);
 *(__u32 *)((uintptr_t)self->f_fatv+fatoff) = value & 0x0FFFFFFF;
}


#undef FAT
#define FAT                   ((fat_t *)ino->i_super)


#undef FAT
#define FAT                   ((fat_t *)ino)
PRIVATE void KCALL fatroot_fini(struct inode *__restrict ino) {
 free(FAT->f_fatmeta);
 free(FAT->f_fatv);
}

PRIVATE struct inodeops fat_rootnode = {
 .ino_fini = &fatroot_fini,
};
PRIVATE struct superblockops fat_rootblock = {
 .sb_sync = (errno_t (KCALL *)(struct superblock *__restrict))&kfatfs_fat_flush,
};



PRIVATE void KCALL trimspecstring(char *__restrict buf, size_t size) {
 while (size && isspace(*buf)) { memmove(buf,buf+1,--size); buf[size] = '\0'; }
 while (size && isspace(buf[size-1])) buf[--size] = '\0';
}

PUBLIC REF struct superblock *KCALL
fat_mksuperblock(struct blkdev *__restrict blkdev) {
 fat_header_t *header; fat_t *result;
 ssize_t header_size;
 CHECK_HOST_DOBJ(blkdev);
 if (blkdev->bd_blocksize < sizeof(fat_header_t)+2)
     return E_PTR(-EINVAL);
 header = (fat_header_t *)alloca(blkdev->bd_blocksize);
 /* Read the first block of data from the block device. */
 header_size = blkdev_raw_read(blkdev,0,header,1);
 if (E_ISERR(header_size)) return E_PTR(header_size);
 if unlikely(!header_size) return E_PTR(-EINVAL);
 
 /* Check the boot signature. */
 { byte_t *boot_signature;
   boot_signature = (byte_t *)header+(header_size-2);
   if unlikely(boot_signature[0] != 0x55 ||
               boot_signature[1] != 0xAA)
               return E_PTR(-EINVAL);
 }
 result = superblock_tnew(fat_t);
 if unlikely(!result) return E_PTR(-ENOMEM);
#define ERROR(x) { header_size = (x); goto err; }

 /* Check some other clear identifiers for an invalid FAT header. */
 result->f_secsize = BSWAP_LE2H16(header->com.bpb_bytes_per_sector);
 if unlikely(result->f_secsize != 512  && result->f_secsize != 1024 &&
             result->f_secsize != 2048 && result->f_secsize != 4096)
             ERROR(-EINVAL);
 if unlikely(!header->com.bpb_sectors_per_cluster) ERROR(-EINVAL);
 if unlikely(!header->com.bpb_reserved_sectors) ERROR(-EINVAL); /* What's the first sector, then? */
 if unlikely((result->f_secsize % sizeof(fat_file_t)) != 0) ERROR(-EINVAL);

 /* Extract some common information. */
 result->f_firstfatsec = (fatsec_t)BSWAP_LE2H16(header->com.bpb_reserved_sectors);
 result->f_sec4clus    = (size_t)header->com.bpb_sectors_per_cluster;
 result->f_fatcount    = (u32)header->com.bpb_fatc;

 /* Figure out what kind of FAT filesystem this is. */
 if (!header->com.bpb_sectors_per_fat ||
     !header->com.bpb_maxrootsize) {
  result->f_type         = FATFS_32;
  result->f_firstdatasec = result->f_firstfatsec+
   (BSWAP_LE2H32(header->x32.x32_sectors_per_fat)*header->com.bpb_fatc);
 } else {
  fatfs_t fstype;
  u32 fat_size,root_sectors,data_sectors,total_clusters;
  root_sectors = CEILDIV(BSWAP_LE2H16(header->com.bpb_maxrootsize)*
                         sizeof(fat_file_t),result->f_secsize);
  fat_size     = (header->com.bpb_fatc*BSWAP_LE2H16(header->com.bpb_sectors_per_fat));
  result->f_firstdatasec  = BSWAP_LE2H16(header->com.bpb_reserved_sectors);
  result->f_firstdatasec += fat_size;
  result->f_firstdatasec += root_sectors;
  /* Calculate the total number of data sectors. */
  data_sectors  = FAT_COMMON_TOTALSECTORS(header->com);
  data_sectors -= BSWAP_LE2H16(header->com.bpb_reserved_sectors);
  data_sectors -= fat_size;
  data_sectors -= root_sectors;
  /* Calculate the total number of data clusters. */
  total_clusters = data_sectors/header->com.bpb_sectors_per_cluster;
       if (total_clusters > FAT16_MAXCLUSTERS) fstype = FATFS_32;
  else if (total_clusters > FAT12_MAXCLUSTERS) fstype = FATFS_16;
  else                                         fstype = FATFS_12;
  result->f_type = fstype;
 }

 switch (result->f_type) {
 {
  if (0) {
 case FATFS_12:
   result->f_readfat   = &fat_readfat_12;
   result->f_writefat  = &fat_writefat_12;
   result->f_getfatsec = &fat_getfatsec_12;
   result->f_clseof_marker = 0xfff;
  }
  if (0) {
 case FATFS_16:
   result->f_readfat   = &fat_readfat_16;
   result->f_writefat  = &fat_writefat_16;
   result->f_getfatsec = &fat_getfatsec_16;
   result->f_clseof_marker = 0xffff;
  }
  if (header->x16.x16_signature != 0x28 &&
      header->x16.x16_signature != 0x29)
      ERROR(-EINVAL);
  memcpy(result->f_label,header->x16.x16_label,sizeof(header->x16.x16_label));
  memcpy(result->f_sysname,header->x16.x16_sysname,sizeof(header->x16.x16_sysname));
  result->f_rootsec  = BSWAP_LE2H16(header->com.bpb_reserved_sectors);
  result->f_rootsec += (header->com.bpb_fatc*BSWAP_LE2H16(header->com.bpb_sectors_per_fat));
  result->f_sec4fat  = BSWAP_LE2H16(header->com.bpb_sectors_per_fat);
  result->f_rootmax  = (__u32)BSWAP_LE2H16(header->com.bpb_maxrootsize);
  result->f_clseof   = (result->f_sec4fat*result->f_secsize)/2;
 } break;
 default:
  /* Check the FAT32 signature. */
  if (header->x32.x32_signature != 0x28 &&
      header->x32.x32_signature != 0x29)
      ERROR(-EINVAL);
  memcpy(result->f_label,header->x32.x32_label,sizeof(header->x32.x32_label));
  memcpy(result->f_sysname,header->x32.x32_sysname,sizeof(header->x32.x32_sysname));
  result->f_readfat       = &fat_readfat_32;
  result->f_writefat      = &fat_writefat_32;
  result->f_getfatsec     = &fat_getfatsec_32;
  result->f_sec4fat       = BSWAP_LE2H32(header->x32.x32_sectors_per_fat);
#ifdef __DEBUG__
  result->f_rootmax       = (__u32)-1;
#endif
  result->f_clseof        = (result->f_sec4fat*result->f_secsize)/4;
  result->f_clseof_marker = 0x0FFFFFFF;
  /* Must lookup the cluster of the root directory. */
  result->f_rootcls       = BSWAP_LE2H32(header->x32.x32_root_cluster);
  break;
 }

 if unlikely(result->f_clseof_marker < result->f_clseof)
             result->f_clseof = result->f_clseof_marker;
 memcpy(&result->f_oem,header->com.bpb_oem,8*sizeof(char));
 result->f_fatsize = result->f_sec4fat*result->f_secsize;
 result->f_oem[8]     = '\0',trimspecstring(result->f_oem,8);
 result->f_label[11]  = '\0',trimspecstring(result->f_label,11);
 result->f_sysname[8] = '\0',trimspecstring(result->f_sysname,8);

 /* Allocate memory for the actual FAT. */
 result->f_fatv = malloc(result->f_fatsize);
 if unlikely(!result->f_fatv) ERROR(-ENOMEM);

 /* Allocate the cache for loaded FAT entries.
  * >> The FAT can get pretty big, so it's faster to keep
  *    most of it unloaded until we actually need it.
  * PLUS: Unless the volume is completely full,
  *       we will only need a small portion of it. */
 result->f_fatmeta = (byte_t *)calloc(1,CEILDIV(result->f_fatsize,result->f_sec4fat*4));
 if unlikely(!result->f_fatmeta) { free(result->f_fatv); ERROR(-ENOMEM); }

 result->f_flags = KFATFS_FLAG_NONE;

 result->f_sb.sb_root.i_refcnt       = 1;
 result->f_sb.sb_root.i_super        = &result->f_sb;
 result->f_sb.sb_root.i_ops          = &fat_rootnode;
 result->f_sb.sb_root.i_nlink        = 1;
 result->f_sb.sb_root.i_attr.ia_mode = S_IFDIR|0777;
 result->f_sb.sb_root.i_attr_disk    = result->f_sb.sb_root.i_attr;
 result->f_sb.sb_root.i_state        = INODE_STATE_READONLY; /* Read-only. */
 result->f_sb.sb_ops                 = &fat_rootblock;
 result->f_sb.sb_blkdev            = blkdev;
 BLOCKDEV_INCREF(blkdev);

 return (struct superblock *)result;
#undef ERROR
err:
 free(result);
 return E_PTR(header_size);
}

DECL_END

#endif /* !GUARD_KERNEL_MOD_FAT_C */
