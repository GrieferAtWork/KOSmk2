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
#ifdef __INTELLISENSE__
#include "ata.c"
#define LBA48 1
//#define WRITE 1
#endif

#ifdef LBA48
#define ATA_RD   ata_lba48_read
#define ATA_WR   ata_lba48_write
#elif defined(LBA28)
#define ATA_RD   ata_lba28_read
#define ATA_WR   ata_lba28_write
#elif defined(CHS)
#define ATA_RD   ata_chs_read
#define ATA_WR   ata_chs_write
#else
#error "Invalid mode"
#endif

#ifdef WRITE
#define ATA_RDWR  ATA_WR
#define BUF_CONST const
#else
#define ATA_RDWR  ATA_RD
#define BUF_CONST /* nothing */
#endif

DECL_BEGIN

#define SELF container_of(self,ata_t,a_device)
PRIVATE ssize_t KCALL
ATA_RDWR(struct blkdev *__restrict self, blkaddr_t block,
         USER void BUF_CONST *buf, size_t n_blocks) {
 struct bus *b = BUS(SELF);
 ssize_t error,result = (ssize_t)n_blocks;
 while (n_blocks) {
#ifdef LBA48
  u16 part = (u16)MIN(n_blocks,UINT16_MAX);
#else
  u8 part = (u8)MIN(n_blocks,UINT8_MAX);
#ifdef CHS
  u32 temp     =  block / (u8)SELF->a_sectors_per_track;
  u8  sector   = (block % (u8)SELF->a_sectors_per_track)+1;
  u8  head     =  temp  % (u8)SELF->a_number_of_heads;
  u16 cylinder =  temp  / (u8)SELF->a_number_of_heads;
#endif
#endif /* ... */

  /* Lock the bus for the duration of our operation. */
  error = rwlock_write(&b->b_lock);
  if (E_ISERR(error)) return error;

  assert(!b->b_active);
  b->b_active = SELF;
  COMPILER_WRITE_BARRIER();

  /* Wait for the bus to stop being busy. */
  error = ata_status_wait(SELF->a_ctrl,ATA_SR_BSY,0,
                          SELF->a_cm_timeout);
  if (E_ISERR(error)) goto cmd_end;

  /* Clear any old event. */
  ATOMIC_WRITE(b->b_signaled.s_ticket,0);

  /* Select our drive. */
#ifdef LBA48
  outb(SELF->a_iobase+ATA_HDDEVSEL,
       SELF->a_drive-(ATA_DRIVE_MASTER-0x40));
#elif defined(LBA28)
  outb(SELF->a_iobase+ATA_HDDEVSEL,
      (SELF->a_drive+(0xe0-ATA_DRIVE_MASTER)) |
      ((block >> 24) & 0x0f));
#else
  outb(SELF->a_iobase+ATA_HDDEVSEL,
      (SELF->a_drive+(0xe0-ATA_DRIVE_MASTER)) |
      (head & 0x0f));
#endif
  ata_sleep(SELF->a_iobase);

  /* Tell the drive what we want to read. */
#ifdef LBA48
  outb(SELF->a_iobase+ATA_SECCOUNT0,(u8)(part >> 8));
  outb(SELF->a_iobase+ATA_LBALO,(u8)(block >> 24));
  outb(SELF->a_iobase+ATA_LBAMD,(u8)(block >> 32));
  outb(SELF->a_iobase+ATA_LBAHI,(u8)(block >> 40));
#endif /* LBA48 */
  outb(SELF->a_iobase+ATA_SECCOUNT0,(u8)part);
#ifdef CHS
  outb(SELF->a_iobase+ATA_LBALO,sector); /* Sector number. */
  outb(SELF->a_iobase+ATA_LBAMD,(u8)(cylinder)); /* Cylinder low */
  outb(SELF->a_iobase+ATA_LBAHI,(u8)(cylinder >> 8)); /* Cylinder high */
#else /* CHS */
  outb(SELF->a_iobase+ATA_LBALO,(u8)block);
  outb(SELF->a_iobase+ATA_LBAMD,(u8)(block >> 8));
  outb(SELF->a_iobase+ATA_LBAHI,(u8)(block >> 16));
#endif /* !CHS */
#ifdef LBA48
  outb(SELF->a_iobase+ATA_CMD,ATA_CMD_READ_PIO_EXT);
#else /* LBA48 */
  outb(SELF->a_iobase+ATA_CMD,ATA_CMD_READ_PIO);
#endif /* !LBA48 */

  /* Now's a good time to update counters for later. */
  n_blocks -= (size_t)part;
  block    += (size_t)part;

  /* The command has been uttered. - Now to wait for it to complete.
   * NOTE: This is where we wait for the drive to spin up. */
  do {
   size_t copy_error;
   error = sem_timedwait(&b->b_signaled,jiffies+SELF->a_io_timeout);
   if (E_ISOK(error))
       error = ata_status_wait(SELF->a_ctrl,ATA_SR_DRQ,
                               ATA_SR_DRQ,SELF->a_cm_timeout);
   if (E_ISERR(error)) {
    if (error == -ETIMEDOUT || error == -EIO)
        goto cmd_end; /* The device didn't respond. */
    goto abort;
   }

   /* All right! Now to actually read all that schweet data. */
#ifdef WRITE
   copy_error = outsw_user(SELF->a_iobase+ATA_DATA,buf,ATA_BLOCKSIZE/2);
#else
   copy_error = insw_user(SELF->a_iobase+ATA_DATA,buf,ATA_BLOCKSIZE/2);
#endif
   if (copy_error) { error = -EFAULT; goto abort; }

   *(uintptr_t *)&buf += ATA_BLOCKSIZE;
  } while (--part);
#ifdef WRITE
#ifdef LBA48
   outb(SELF->a_iobase+ATA_CMD,ATA_CMD_CACHE_FLUSH_EXT);
   ata_sleep(SELF->a_iobase);
#endif /* LBA48 */
   outb(SELF->a_iobase+ATA_CMD,ATA_CMD_CACHE_FLUSH);
   ata_sleep(SELF->a_iobase);
#endif /* WRITE */
  __IF0 {
abort:
   /* Do a hard-reset on the bus to abort the command. */
   outb(SELF->a_ctrl,ATA_CTRL_SRST);
   ata_sleep(SELF->a_iobase);
   outb(SELF->a_ctrl,0);
   ata_sleep(SELF->a_iobase);
  }
cmd_end:

  assert(b->b_active == SELF);
  b->b_active = NULL;
  COMPILER_WRITE_BARRIER();

  rwlock_endwrite(&b->b_lock);
  if (E_ISERR(error)) return error;
 }
 return result;
}
#undef SELF

DECL_END

#undef BUF_CONST
#undef ATA_RDWR
#undef ATA_WR
#undef ATA_RD
#undef WRITE
#undef LBA48
#undef LBA28
#undef CHS


