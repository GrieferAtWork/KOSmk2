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
#ifndef GUARD_INCLUDE_MODULES_MBR_H
#define GUARD_INCLUDE_MODULES_MBR_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

/* Flags used by pt_bootable */
#define PART_BOOTABLE_NONE   0x00
#define PART_BOOTABLE_ACTICE 0x80
#define PART_BOOTABLE_LBA48  0x01

/* http://wiki.osdev.org/Partition_Table */
struct PACKED part_com { /*< HDD Partition */
 u8           pt_bootable; /*< Boot indicator bit flag: 0 = no, 0x80 = bootable (or "active"). */
 u8           data1[3];
 u8           pt_sysid;    /*< System ID (s.a.: `BLKSYS_*'). */
 u8           data2[11];
};

struct PACKED part_32 { /*< HDD Partition */
 u8           pt_bootable;       /*< Boot indicator bit flag: 0 = no, 0x80 = bootable (or "active"). */
 u8           pt_headstart;      /*< Starting Head. */
 unsigned int pt_sectstart : 6;  /*< Starting Sector. */
 unsigned int pt_cylistart : 10; /*< Starting Cylinder. */
 u8           pt_sysid;          /*< System ID (s.a.: `BLKSYS_*'). */
 u8           pt_headend;        /*< Ending Head. */
 unsigned int pt_sectend : 6;    /*< Ending Sector. */
 unsigned int pt_cyliend : 10;   /*< Ending Cylinder. */
 u32          pt_lbastart;       /*< Relative Sector (to start of partition -- also equals the partition's starting LBA value). */
 u32          pt_lbasize;        /*< Total Sectors in partition. */
};

/* NOTE: This one should really be a standard!
 *      (Though some say it isn't, this kernel uses it) */
#define PART48_SIG1 0x14
#define PART48_SIG2 0xeb
struct PACKED part_48 { /*< LBA-48 HDD Partition */
 u8           pt_bootable;   /*< Boot indicator bit flag: 1 = no, 0x81 = bootable (or "active"). */
 u8           pt_sig1;       /*< Signature #1 (== 0x14). */
 u16          pt_lbastarthi; /*< High 2 bytes for pt_lbastart. */
 u8           pt_sysid;      /*< System ID (s.a.: `BLKSYS_*'). */
 u8           pt_sig2;       /*< Signature #2 (== 0xeb). */
 u16          pt_lbasizehi;  /*< High 2 bytes for pt_lbasize. */
 u32          pt_lbastart;   /*< Relative Sector (to start of partition -- also equals the partition's starting LBA value). */
 u32          pt_lbasize;    /*< Total Sectors in partition. */
};

union part {
 struct part_com pt;
 struct part_32  pt_32;
 struct part_48  pt_48;
};


/* http://wiki.osdev.org/MBR_(x86) */
typedef struct PACKED mbr_struct { /*< Master boot record */
 u8         mbr_bootstrap[436]; /*< MBR Bootstrap (flat binary executable code). */
 char       mbr_diskuid[10];    /*< Optional "unique" disk ID1. */
 union part mbr_part[4];        /*< Partition table entries. */
 u8         mbr_sig[2];         /*< "Valid bootsector" signature bytes (== 0x55, 0xAA) */
} mbr_t;

DECL_END

#endif /* !GUARD_INCLUDE_MODULES_MBR_H */
