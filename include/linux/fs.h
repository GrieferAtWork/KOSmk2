#ifndef _LINUX_FS_H
#define _LINUX_FS_H

#include <linux/ioctl.h>

#define BLKROSET     _IO(0x12,93)  /*< Set device read-only (0 = read-write) */
#define BLKROGET     _IO(0x12,94)  /*< Get read-only status (0 = read_write) */
#define BLKRRPART    _IO(0x12,95)  /*< Re-read partition table */
#define BLKGETSIZE   _IO(0x12,96)  /*< Return device size /512 (long *arg) */
#define BLKFLSBUF    _IO(0x12,97)  /*< Flush buffer cache */
#define BLKRASET     _IO(0x12,98)  /*< Set read ahead for block device */
#define BLKRAGET     _IO(0x12,99)  /*< Get current read ahead setting */
#define BLKFRASET    _IO(0x12,100) /*< set filesystem read-ahead. */
#define BLKFRAGET    _IO(0x12,101) /*< get filesystem read-ahead. */
#define BLKSSZGET    _IO(0x12,104) /*< Get block device sector size */
#define BLKBSZGET    _IOR(0x12,112,size_t)
//#define BLKBSZSET    _IOW(0x12,113,size_t)
#define BLKGETSIZE64 _IOR(0x12,114,size_t) /* return device size in bytes (u64 *arg) */

#endif /* _LINUX_FS_H */
