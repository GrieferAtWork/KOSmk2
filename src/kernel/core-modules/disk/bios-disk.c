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
#ifndef GUARD_KERNEL_CORE_MODULES_DISK_BIOS_DISK_C
#define GUARD_KERNEL_CORE_MODULES_DISK_BIOS_DISK_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <dev/blkdev.h>
#include <hybrid/align.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/arch/cpustate.h>
#include <kernel/arch/realmode.h>
#include <kernel/boot.h>
#include <kernel/export.h>
#include <kernel/malloc.h>
#include <kernel/memory.h>
#include <kernel/mman.h>
#include <kernel/user.h>
#include <kos/syslog.h>
#include <malloc.h>
#include <modules/bios-disk.h>
#include <sched/paging.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>

#define READONLY 1 /* Ignore attempts of writing to disk. */

DECL_BEGIN

typedef struct biosblkdev bd_t;

#define CPU16(x) struct cpustate16 x; memset(&x,0,sizeof(x))

#if 1 /* TODO: Disable me */
#define BDISK_DATA  INTERN
#else
#define BDISK_DATA  PRIVATE
#endif

BDISK_DATA DEFINE_RWLOCK(bios_lock);         /*< Global device-access lock. */
BDISK_DATA ppage_t bios_page_p = PAGE_ERROR; /*< [1..1|null(PAGE_ERROR)][lock(bios_lock)] Buffer page used for data exchange with the bios (allocated below the 1Mb mark). */
BDISK_DATA ppage_t bios_page_v = PAGE_ERROR; /*< [1..1|null(PAGE_ERROR)][lock(bios_lock)] Virtual mapping of 'bios_page_p'. */
BDISK_DATA size_t bios_page_sz = 0;          /*< [valid_if(bios_page_p != PAGE_ERROR)][lock(bios_lock)] Allocated amount of bytes starting at 'bios_page_p'. */

struct PACKED bd_13h42h_buffer {
 u8    b_bufsize;      /*< [== sizeof(struct bd_13h42h_buffer)] */
 u8    b_zero;         /*< Unused (Set to ZERO(0)). */
 u16   b_seccnt;       /*< Number of sectors to read (<= 127). */
 u16   b_dst_off;      /*< Destination buffer offset. */
 u16   b_dst_seg;      /*< Destination buffer segment. */
 u64   b_start_lba;    /*< Starting LBA number. */
};

struct PACKED bd_13h48h_buffer {
 u16   b_bufsize;      /*< [== sizeof(struct bd_13h48h_buffer)] */
 u16   b_iflags;       /*< information flags. */
 u32   b_phys_cylinum; /*< physical number of cylinders = last index + 1 (because index starts with 0). */
 u32   b_phys_headnum; /*< physical number of heads = last index + 1 (because index starts with 0). */
 u32   b_phys_sectnum; /*< physical number of sectors per track = last index (because index starts with 1). */
 u64   b_abs_secnum;   /*< absolute number of sectors = last index + 1 (because index starts with 0). */
 u16   b_abs_secsiz;   /*< bytes per sector. */
 void *b_opt_ptr;      /*< optional pointer to Enhanced Disk Drive (EDD) configuration parameters
                        *  which may be used for subsequent interrupt 13h Extension calls (if supported). */
};

PRIVATE ssize_t KCALL
bd_access_chs(bd_t *__restrict self, blkaddr_t block,
              USER void *__restrict buf, size_t n_blocks, bool write) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
#if READONLY
 if (write) return n_blocks;
#endif
 if unlikely(!n_blocks) return 0;
 result = (ssize_t)rwlock_write(&bios_lock);
 if (E_ISERR(result)) goto end;
 result = 0;
 while (n_blocks) {
  bool did_retry = false;
  u32 temp     =  block / self->b_sectors_per_track;
  u8  sector   = (block % self->b_sectors_per_track)+1;
  u8  head     =  temp  % self->b_number_of_heads;
  u16 cylinder =  temp  / self->b_number_of_heads;

  /* Copy data from the user-buffer. */
  if (write && copy_from_user(bios_page_v,buf,self->b_device.bd_blocksize)) { result = -EFAULT; break; }
retry:
  { CPU16(c);
    c.dl = self->b_drive;
    c.sp = realmode_stack;
    rm_interrupt(&c,0x13);
  }
  { CPU16(c);
    c.ah = 0x2+!!write;
    c.al = 1; /* Total sector count (Only read one at a time to keep things simple) */
    c.ch = cylinder & 0xff;
    c.cl = sector | ((cylinder >> 2) & 0xc0);
    c.dh = head;
    c.dl = self->b_drive;
    c.es = (u16)(((uintptr_t)bios_page_p >> 16) << 12);
    c.bx = (u16)((uintptr_t)bios_page_p & 0xffff);
    c.sp = realmode_stack;
    /* Execute the BIOS interrupt. */
    rm_interrupt(&c,0x13);
    if (c.eflags&EFLAGS_CF) {
     syslogf(LOG_HW|LOG_ERROR,
             "[BIOS] Disk access failed: drive %#.2I8x al:%.2I8x, ah:%.2I8x "
             "(block:%I64u, cylinder: %I16u, head: %I8u, sector: %I8u)\n",
             self->b_drive,c.al,c.ah,block,cylinder,head,sector);
     if (!did_retry) {
      CPU16(c);
      c.dl = self->b_drive;
      c.sp = realmode_stack;
      rm_interrupt(&c,0x13);
      syslogf(LOG_HW|LOG_ERROR,
              "[BIOS] Reset drive: %#.2I8x: %s %d\n",
              self->b_drive,c.eflags&EFLAGS_CF ? "ERR" : "OK",c.ah);
      did_retry = true;
      goto retry;
     }
     result = -EIO;
     break;
    }
  }
  /* Copy data to the user-buffer. */
  if (!write && copy_to_user(buf,bios_page_v,self->b_device.bd_blocksize)) { result = -EFAULT; break; }

  *(uintptr_t *)&buf += self->b_device.bd_blocksize;
  --n_blocks;
  ++result;
 }
 rwlock_endwrite(&bios_lock);
end:
 return result;
}
PRIVATE ssize_t KCALL
bd_access_lba(bd_t *__restrict self, blkaddr_t block,
              USER void *__restrict buf, size_t n_blocks, bool write) {
 ssize_t result; size_t max_sectors;
 VIRT struct bd_13h42h_buffer *buffer;
 VIRT void *data_buffer;
 PHYS void *data_buffer_p;
 CHECK_HOST_DOBJ(self);
#if READONLY
 if (write) return n_blocks;
#endif
 if unlikely(!n_blocks) return 0;
 result = (ssize_t)rwlock_write(&bios_lock);
 if (E_ISERR(result)) goto end;
 result      = 0;
 buffer      = (struct bd_13h42h_buffer *)bios_page_v;
 max_sectors = (size_t)(bios_page_sz/self->b_device.bd_blocksize);
 if (max_sectors > 127)
     max_sectors = 127;
 data_buffer   = (void *)CEIL_ALIGN((uintptr_t)bios_page_v+sizeof(struct bd_13h42h_buffer),16);
 data_buffer_p = (void *)CEIL_ALIGN((uintptr_t)bios_page_p+sizeof(struct bd_13h42h_buffer),16);
 while (n_blocks) {
  size_t copy_size;
  CPU16(c);
  c.ah = 0x42+!!write;
  c.dl = self->b_drive;
  if (max_sectors > n_blocks)
      max_sectors = n_blocks;
  c.ds = (u16)(((uintptr_t)bios_page_p >> 16) << 12);
  c.si = (u16)((uintptr_t)bios_page_p & 0xffff);
  c.sp = realmode_stack;

  assert(max_sectors != 0);

  /* Initialize the packet buffer. */
  buffer->b_bufsize   = sizeof(struct bd_13h42h_buffer);
  buffer->b_zero      = 0;
  buffer->b_seccnt    = max_sectors;
  buffer->b_dst_off   = (u16)((uintptr_t)data_buffer_p & 0xf);
  buffer->b_dst_seg   = (u16)((uintptr_t)data_buffer_p >> 4);
  buffer->b_start_lba = (u64)block;

  /* Copy data from the user-buffer. */
  if (write && copy_from_user(data_buffer,buf,max_sectors*
                              self->b_device.bd_blocksize))
  { result = -EFAULT; break; }

  /* Execute the BIOS interrupt. */
  rm_interrupt(&c,0x13);
  if (c.eflags&EFLAGS_CF) { result = -EIO; break; } /* Stop on error. */
  if (!buffer->b_seccnt) break; /* Stop if nothing was read/written. */

  /* Copy data to the user-buffer. */
  copy_size = buffer->b_seccnt*self->b_device.bd_blocksize;
  if (!write && copy_to_user(buf,data_buffer,copy_size))
  { result = -EFAULT; break; }
  n_blocks           -= buffer->b_seccnt;
  result             += buffer->b_seccnt;
  block              += buffer->b_seccnt;
  *(uintptr_t *)&buf += copy_size;
 }
 rwlock_endwrite(&bios_lock);
end:
 return result;
}


PRIVATE ssize_t KCALL
bd_chs_read(struct blkdev *__restrict self, blkaddr_t block,
            USER void *buf, size_t n_blocks) {
 return bd_access_chs((bd_t *)self,block,buf,n_blocks,false);
}
PRIVATE ssize_t KCALL
bd_lba_read(struct blkdev *__restrict self, blkaddr_t block,
            USER void *buf, size_t n_blocks) {
 return bd_access_lba((bd_t *)self,block,buf,n_blocks,false);
}
PRIVATE ssize_t KCALL
bd_chs_write(struct blkdev *__restrict self, blkaddr_t block,
             USER void const *buf, size_t n_blocks) {
 return bd_access_chs((bd_t *)self,block,(void *)buf,n_blocks,true);
}
PRIVATE ssize_t KCALL
bd_lba_write(struct blkdev *__restrict self, blkaddr_t block,
             USER void const *buf, size_t n_blocks) {
 return bd_access_lba((bd_t *)self,block,(void *)buf,n_blocks,true);
}


PRIVATE ATTR_FREETEXT KPD bool KCALL
bd_set_bios_page_size(PAGE_ALIGNED size_t n_bytes) {
 ppage_t new_page;
 assert(IS_ALIGNED(n_bytes,PAGESIZE));
 new_page = page_realloc(bios_page_p,bios_page_sz,n_bytes,
                         PAGEATTR_NONE,MZONE_1MB);
 if unlikely(new_page == PAGE_ERROR) return false;
 task_nointr();
 mman_write(&mman_kernel);
 if (bios_page_sz != 0) {
  /* Delete the old virtual mman-mapping. */
  mman_munmap_unlocked(&mman_kernel,bios_page_v,bios_page_sz,
                       MMAN_MUNMAP_TAG,bios_page_p);
 }
 bios_page_p  = new_page;
 bios_page_sz = n_bytes;

 /* Create a new virtual mman-mapping. */
 { struct mregion *region = NULL;
   /* Map the bios page region into virtual, shared kernel memory. */
   bios_page_v = (ppage_t)mman_findspace_unlocked(&mman_kernel,
                                                 (ppage_t)(0-n_bytes),n_bytes,
                                                  PAGESIZE,0,MMAN_FINDSPACE_BELOW);
   if (bios_page_v == PAGE_ERROR || bios_page_v < (ppage_t)KERNEL_BASE ||
      (region = _mall_untrack(mregion_new_phys(MMAN_DATAGFP(&mman_kernel),new_page,n_bytes))) == NULL ||
       E_ISERR(mman_mmap_unlocked(&mman_kernel,bios_page_v,n_bytes,0,region,
                                   PROT_READ|PROT_WRITE|PROT_NOUSER,NULL,new_page))) {
    mman_endwrite(&mman_kernel);
    if (region) MREGION_DECREF(region);
    task_endnointr();
    page_free(new_page,bios_page_sz);
    bios_page_v  = PAGE_ERROR;
    bios_page_p  = PAGE_ERROR;
    bios_page_sz = 0;
    return false;
   }
   MREGION_DECREF(region);
 }
 mman_endwrite(&mman_kernel);
 task_endnointr();

 syslogf(LOG_HW|LOG_DEBUG,
         FREESTR("[BIOS] Allocated Bios communication page: %p...%p (%p...%p)\n"),
         bios_page_p,(uintptr_t)bios_page_p+n_bytes-1,
         bios_page_v,(uintptr_t)bios_page_v+n_bytes-1);
 return true;
}

/* [const] Override for the bios's capability of
 *         supporting int 13h extensions for all drives. */
PRIVATE ATTR_COLDDATA int bios_exti13h = -1;
DEFINE_EARLY_SETUP_VAR("bios-exti13h",bios_exti13h);

PRIVATE ATTR_FREETEXT bd_t *KCALL bd_new(u8 drive) {
 bd_t *result;
 bool has_extensions = false;
 //return E_PTR(-ENODEV);

 if (!bios_page_sz) {
  /* Allocate the communication page during the first pass. */
  if unlikely(!bd_set_bios_page_size(PAGESIZE)) {
   syslogf(LOG_HW|LOG_ERROR,
           FREESTR("[BIOS] Failed to allocate communication page\n"));
   return E_PTR(-ENOMEM);
  }
 }
 if (bios_exti13h < 0) {
  CPU16(c);
  c.ah = 0x41;
  c.bx = 0x55aa;
  c.dl = drive;
  rm_interrupt(&c,0x13);
  /* If 'CF' isn't set, the bios does support the extensions. */
  has_extensions = !(c.eflags&EFLAGS_CF);
 } else {
  has_extensions = !!bios_exti13h;
 }

#if 1
 if (has_extensions) {
  struct bd_13h48h_buffer *buf;
  CPU16(c);
  buf = (struct bd_13h48h_buffer *)bios_page_v;
  memset(buf,0,sizeof(struct bd_13h48h_buffer));
  buf->b_bufsize = sizeof(struct bd_13h48h_buffer);
  c.ah = 0x48;
  c.dl = drive;
  c.ds = (u16)(((uintptr_t)bios_page_p >> 16) << 12);
  c.si = (u16)((uintptr_t)bios_page_p & 0xffff);
  c.sp = realmode_stack;
  rm_interrupt(&c,0x13);
  if (c.eflags&EFLAGS_CF || !buf->b_abs_secnum || !buf->b_abs_secsiz) {
   //syslogf(LOG_HW|LOG_WARN,
   //        FREESTR("[BIOS] Attempting legacy initialization for drive %#.2I8x\n"),
   //        drive);
   goto try_legacy;
  }

  result = (bd_t *)blkdev_new(sizeof(bd_t));
  if unlikely(!result) return E_PTR(-ENOMEM);

  result->b_device.bd_blockcount = (blkaddr_t)buf->b_abs_secnum;
  result->b_device.bd_blocksize  = (blksize_t)buf->b_abs_secsiz;
  result->b_device.bd_read       = &bd_lba_read;
  result->b_device.bd_write      = &bd_lba_write;
 } else try_legacy:
#endif
 {
  /* Figure out the drive geometry. */
  u16 max_cylinder;
  CPU16(c);
  c.ah = 8;
  c.dl = drive;
  c.sp = realmode_stack;
  rm_interrupt(&c,0x13);
  if (c.eflags&EFLAGS_CF) return E_PTR(-ENODEV);
  if (!(c.cl & 0x3f)) return E_PTR(-ENODEV);
  max_cylinder = 1+(c.ch | ((c.cl&0xc0) >> 2));
  if (!max_cylinder) return E_PTR(-ENODEV);

  result = (bd_t *)blkdev_new(sizeof(bd_t));
  if unlikely(!result) return E_PTR(-ENOMEM);

  result->b_sectors_per_track = c.cl & 0x3f;
  result->b_number_of_heads   = c.dh+1;
  syslogf(LOG_DEBUG,"result->b_sectors_per_track = %I8u\n",result->b_sectors_per_track);
  syslogf(LOG_DEBUG,"result->b_number_of_heads   = %I8u\n",result->b_number_of_heads);
  result->b_device.bd_blockcount = (blkaddr_t)max_cylinder*
                                   (blkaddr_t)result->b_sectors_per_track*
                                   (blkaddr_t)result->b_number_of_heads;
  result->b_device.bd_blocksize  = 512;
  result->b_device.bd_read       = &bd_chs_read;
  result->b_device.bd_write      = &bd_chs_write;
 }
 result->b_drive = drive;
 
 /* Make sure the buffer page is at least the size of a single sector! */
 if unlikely(result->b_device.bd_blocksize+
             sizeof(struct bd_13h42h_buffer) >
             bios_page_sz) {
  size_t newsize = CEIL_ALIGN(result->b_device.bd_blocksize+
                              sizeof(struct bd_13h42h_buffer),
                              PAGESIZE);
  assert(newsize != bios_page_sz);
  syslogf(LOG_HW|LOG_INFO,
          FREESTR("[BIOS] Resizing Bios page for large sector size %I64u for drive %#.2I8x\n"),
          result->b_device.bd_blocksize,drive);
  if unlikely(!bd_set_bios_page_size(newsize)) {
   syslogf(LOG_HW|LOG_INFO,FREESTR("[BIOS] Failed to resize Bios page\n"));
   free(result);
   return E_PTR(-ENOMEM);
  }
 }
 /* NOTE: The kernel's own 'THIS_INSTANCE' must never get unloaded! */
 asserte(E_ISOK(device_setup(&result->b_device.bd_device,THIS_INSTANCE)));
 return result;
}


PRIVATE ATTR_FREETEXT bool KCALL
bios_probe_disk(dev_t id, u8 drive) {
 REF bd_t *dev = bd_new(drive);
 if (E_ISOK(dev)) {
  errno_t error;
  syslogf(LOG_HW|LOG_INFO,
          FREESTR("[BIOS] Created Bios disk driver %[dev_t] for drive %#.2I8x (%I64ux%Iu bytes; %s)\n"),
          id,drive,dev->b_device.bd_blockcount,dev->b_device.bd_blocksize,
          dev->b_device.bd_read == &bd_lba_read ? "LBA" : "CHS");
  error = devns_insert(&ns_blkdev,&dev->b_device.bd_device,id);
  if (E_ISERR(error)) {
   syslogf(LOG_HW|LOG_ERROR,
           FREESTR("[BIOS] Failed to register Bios disk driver %[dev_t]: %[errno]\n"),
           id,-error);
  } else {
   blkdev_autopart(&dev->b_device,(MINOR(BIOS_DISK_B)-MINOR(BIOS_DISK_A))-1);
  }
  BLKDEV_DECREF(&dev->b_device);
  return true;
 }
 return false;
}

PRIVATE ATTR_FREERODATA dev_t const bios_devids[] =
{ BIOS_DISK_A,BIOS_DISK_B,BIOS_DISK_C,BIOS_DISK_D, };


INTERN ATTR_FREETEXT
REF struct biosblkdev *KCALL bios_find_dev(u8 drive) {
 struct biosblkdev *result;
 dev_t devid; unsigned int i;
 switch (drive) {
  if (0) { case 0x80: devid = BIOS_DISK_A; }
  if (0) { case 0x81: devid = BIOS_DISK_B; }
  if (0) { case 0x82: devid = BIOS_DISK_C; }
  if (0) { case 0x83: devid = BIOS_DISK_D; }
  result = (REF struct biosblkdev *)BLKDEV_LOOKUP(devid);
  if (result) {
   if (result->b_drive == drive) return result;
   BLKDEV_DECREF(&result->b_device);
  }
  break;
 default: goto nodev;
 }
 /* Non-standard, or incomplete mapping. */
 for (i = 0; i < COMPILER_LENOF(bios_devids); ++i) {
  if (bios_devids[i] == devid) continue;
  result = (REF struct biosblkdev *)BLKDEV_LOOKUP(bios_devids[i]);
  if (result) {
   if (result->b_drive == drive) return result;
   BLKDEV_DECREF(&result->b_device);
  }
 }
nodev:
 /* The device wasn't part of the mapped device tree.
  * If this happens, it is most likely that the BIOS is faulty,
  * or the drive ID we've got isn't the correct one.
  * In both cases, try one last time to load the given drive directly! */
 syslogf(LOG_HW|LOG_ERROR,
         FREESTR("[BIOS] Failed to locate boot drive %#.2I8x (Try direct access)\n"),
         drive);
 result = bd_new(drive);
 /* WARNING: We do not register this device! */
 if (E_ISERR(result)) {
  syslogf(LOG_HW|LOG_ERROR,
          FREESTR("[BIOS] Failed to directly access boot drive %#.2I8x: %[errno]\n"),
          drive,-E_GTERR(result));
  /* Return NULL if load failed. */
  result = NULL;
 }
 return result;
}

PRIVATE MODULE_INIT void KCALL bios_disk_init(void) {
 bool ok;
 ok  = bios_probe_disk(BIOS_DISK_A,0x80);
 ok |= bios_probe_disk(BIOS_DISK_B,0x81);
 ok |= bios_probe_disk(BIOS_DISK_C,0x82);
 ok |= bios_probe_disk(BIOS_DISK_D,0x83);
 if (!ok) {
  u8 i; dev_t disk_id = BIOS_DISK_A;
  /* If we didn't find any disks, stalk the bios by searching for more. */
  syslogf(LOG_HW|LOG_WARN,
          FREESTR("[BIOS] Failed to find BIOS disk drivers. - Probing for more...\n"));
  for (i = 0x84; i < 0xff; ++i) {
   if (bios_probe_disk(disk_id,i)) {
    /**/ if (disk_id == BIOS_DISK_D) break;
    else if (disk_id == BIOS_DISK_C) disk_id = BIOS_DISK_D;
    else if (disk_id == BIOS_DISK_B) disk_id = BIOS_DISK_C;
    else                             disk_id = BIOS_DISK_B;
   }
  }
 }
}

#ifndef CONFIG_NO_MODULE_CLEANUP
PRIVATE MODULE_FINI void KCALL bios_disk_fini(void) {
 devns_erase(&ns_blkdev,BIOS_DISK_D,DEVNS_ERASE_NORMAL);
 devns_erase(&ns_blkdev,BIOS_DISK_C,DEVNS_ERASE_NORMAL);
 devns_erase(&ns_blkdev,BIOS_DISK_B,DEVNS_ERASE_NORMAL);
 devns_erase(&ns_blkdev,BIOS_DISK_A,DEVNS_ERASE_NORMAL);
}
#endif

DECL_END

#endif /* !GUARD_KERNEL_CORE_MODULES_DISK_BIOS_DISK_C */
