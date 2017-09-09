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
#include "ata.c"

DECL_BEGIN

#ifdef PIO48
#define MAX_BLOCKS  0xffff
#define nblocks_t   u16
#else
#define MAX_BLOCKS  0xff
#define nblocks_t   u8
#endif

#ifdef PIO48
PRIVATE ssize_t KCALL
ata_pio48_readlba(ata_t const *__restrict self, blkaddr_t lba,
                  void *__restrict buf, size_t n_blocks)
#else
PRIVATE ssize_t KCALL
ata_pio28_readlba(ata_t const *__restrict self, blkaddr_t lba,
                  void *__restrict buf, size_t n_blocks)
#endif
{
 errno_t error;
 ssize_t result = 0;
 if (n_blocks) {
  u16 *iter = (u16 *)buf;
  error = rwlock_write(&self->a_descr->ad_lock);
  if (E_ISERR(error)) { PREEMPTION_ENABLE(); return error; }
  for (;;) {
   nblocks_t i,n;
   if (n_blocks >= MAX_BLOCKS) n = MAX_BLOCKS;
   else                        n = (nblocks_t)n_blocks;
   error = ata_poll(self->a_bus,ATA_STATUS_BSY,0);
   if (E_ISERR(error)) goto err;
#ifdef PIO48
   outb(ATA_IOPORT_HDDEVSEL(self->a_bus),
        self->a_drive-(ATA_DRIVE_MASTER-0x40));
#else
   outb(ATA_IOPORT_HDDEVSEL(self->a_bus),
       (self->a_drive+(0xe0-ATA_DRIVE_MASTER)) |
       ((lba >> 24) & 0x0f));
#endif
   ata_sleep(self->a_bus);
#ifdef PIO48
   outb(ATA_IOPORT_SECCOUNT0(self->a_bus),(u8)(n >> 8));
   outb(ATA_IOPORT_LBALO(self->a_bus),(u8)(lba >> 24));
   outb(ATA_IOPORT_LBAMD(self->a_bus),(u8)(lba >> 32));
   outb(ATA_IOPORT_LBAHI(self->a_bus),(u8)(lba >> 40));
#endif
   outb(ATA_IOPORT_SECCOUNT0(self->a_bus),(u8)n);
   outb(ATA_IOPORT_LBALO(self->a_bus),(u8)lba);
   outb(ATA_IOPORT_LBAMD(self->a_bus),(u8)(lba >> 8));
   outb(ATA_IOPORT_LBAHI(self->a_bus),(u8)(lba >> 16));
#ifdef PIO48
   outb(ATA_IOPORT_COMMAND(self->a_bus),ATA_CMD_READ_PIO_EXT);
#else
   outb(ATA_IOPORT_COMMAND(self->a_bus),ATA_CMD_READ_PIO);
#endif
   for (i = 0; i != n; ++i) {
    error = ata_poll(self->a_bus,ATA_STATUS_DRQ,ATA_STATUS_DRQ);
    if (E_ISERR(error)) goto err;
    insw(ATA_IOPORT_DATA(self->a_bus),iter,ATA_BLOCKSIZE/sizeof(u16));
    iter += ATA_BLOCKSIZE/sizeof(u16);
   }
   result += n;
   n_blocks -= n;
   if (!n_blocks) break;
   lba += n;
  }
end_unlock:
  rwlock_endwrite(&self->a_descr->ad_lock);
 }
 return result;
err: if (!result) result = error; goto end_unlock;
}

#ifdef PIO48
PRIVATE ssize_t KCALL
ata_pio48_writelba(ata_t *__restrict self, blkaddr_t lba,
                   void const *__restrict buf, size_t n_blocks)
#else
PRIVATE ssize_t KCALL
ata_pio28_writelba(ata_t *__restrict self, blkaddr_t lba,
                   void const *__restrict buf, size_t n_blocks)
#endif
{
#if ATA_READONLY
 /* Disarm the ATA driver's ability of writing. */
 (void)self;
 (void)lba;
 (void)buf;
 return (ssize_t)n_blocks;
#else
 errno_t error;
 ssize_t result = 0;
 if (n_blocks) {
  u16 *iter = (u16 *)buf;
  error = rwlock_write(&self->a_descr->ad_lock);
  if (E_ISERR(error)) { PREEMPTION_ENABLE(); return error; }
  for (;;) {
   nblocks_t i,n;
   if (n_blocks >= MAX_BLOCKS) n = MAX_BLOCKS;
   else                          n = (nblocks_t)n_blocks;
   error = ata_poll(self->a_bus,ATA_STATUS_BSY,0);
   if (E_ISERR(error)) goto err;
#ifdef PIO48
   outb(ATA_IOPORT_HDDEVSEL(self->a_bus),
        self->a_drive-(ATA_DRIVE_MASTER-0x40));
#else
   outb(ATA_IOPORT_HDDEVSEL(self->a_bus),
       (self->a_drive+(0xe0-ATA_DRIVE_MASTER)) |
       ((lba >> 24) & 0x0f));
#endif
   ata_sleep(self->a_bus);
#ifdef PIO48
   outb(ATA_IOPORT_SECCOUNT0(self->a_bus),(u8)(n >> 8));
   outb(ATA_IOPORT_LBALO(self->a_bus),(u8)(lba >> 24));
   outb(ATA_IOPORT_LBAMD(self->a_bus),(u8)(lba >> 32));
   outb(ATA_IOPORT_LBAHI(self->a_bus),(u8)(lba >> 40));
#endif
   outb(ATA_IOPORT_SECCOUNT0(self->a_bus),(u8)n);
   outb(ATA_IOPORT_LBALO(self->a_bus),(u8)lba);
   outb(ATA_IOPORT_LBAMD(self->a_bus),(u8)(lba >> 8));
   outb(ATA_IOPORT_LBAHI(self->a_bus),(u8)(lba >> 16));
#ifdef PIO48
   outb(ATA_IOPORT_COMMAND(self->a_bus),ATA_CMD_WRITE_PIO_EXT);
#else
   outb(ATA_IOPORT_COMMAND(self->a_bus),ATA_CMD_WRITE_PIO);
#endif
   for (i = 0; i != n; ++i) {
    error = ata_poll(self->a_bus,ATA_STATUS_DRQ,ATA_STATUS_DRQ);
    if (E_ISERR(error)) goto err;
#if 1
    outsw(ATA_IOPORT_DATA(self->a_bus),iter,ATA_BLOCKSIZE/sizeof(u16));
#else
    { u16 *end = iter+ATA_BLOCKSIZE/sizeof(u16);
      while (iter != end) outw(ATA_IOPORT_DATA(self->a_bus),*iter++);
    }
#endif
    iter += ATA_BLOCKSIZE/sizeof(u16);
   }
#ifdef PIO48
   outb(ATA_IOPORT_COMMAND(self->a_bus),ATA_CMD_CACHE_FLUSH_EXT);
   ata_sleep(self->a_bus);
#endif
   outb(ATA_IOPORT_COMMAND(self->a_bus),ATA_CMD_CACHE_FLUSH);
   ata_sleep(self->a_bus);
   result += n;
   n_blocks -= n;
   if (!n_blocks) break;
   lba += n;
  }
end_unlock:
  rwlock_endwrite(&self->a_descr->ad_lock);
 }
 return result;
err: if (!result) result = error; goto end_unlock;
#endif
}

#ifdef PIO48
#undef PIO48
#endif
#undef MAX_BLOCKS
#undef nblocks_t

DECL_END
