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
#ifndef GUARD_INCLUDE_MODULES_BIOS_DISK_H
#define GUARD_INCLUDE_MODULES_BIOS_DISK_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <dev/blkdev.h>
#include <stdbool.h>

DECL_BEGIN

struct biosblkdev {
 /* BIOS Disk Device driver (Used for interfacing with 'int $0x13' bios functions). */ 
 struct blkdev b_device;            /*< Underlying block device. */
 u8            b_sectors_per_track; /*< [const] Amount of sectors per track. */
 u8            b_number_of_heads;   /*< [const][!0] The number of read/write heads. */
 u8            b_drive;             /*< [const] BIOS Drive number of this device (e.g.: '0x80' for 'C:'). */
 u8            b_padding;           /*< ... */
};

#define BIOS_DISK_A  MKDEV(14,0)   /*< /dev/dos_hda */
#define BIOS_DISK_B  MKDEV(14,64)  /*< /dev/dos_hdb */
#define BIOS_DISK_C  MKDEV(14,128) /*< /dev/dos_hdc */
#define BIOS_DISK_D  MKDEV(14,192) /*< /dev/dos_hdd */
/* NOTE: We've got more than 256 minor numbers. - So we can do this:
 * >> Support for more than 4 bios drives _AND_ a clear
 *    way of mapping device ids to drive numbers! */
#define BIOS_DISK(drive)  MKDEV(14,((drive)-0x80)*64)

DECL_END

#endif /* !GUARD_INCLUDE_MODULES_BIOS_DISK_H */
