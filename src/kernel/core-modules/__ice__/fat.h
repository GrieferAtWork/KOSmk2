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
#ifndef GUARD_KERNEL_MODULES_FAT_H
#define GUARD_KERNEL_MODULES_FAT_H 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kernel/fs.h>
#include <kernel/types.h>

/* Read-only FAT filesystem kernel module. */

DECL_BEGIN

struct idata {
};

#define FAT12_MAXCLUSTERS 4084
#define FAT16_MAXCLUSTERS 65524
#define FAT32_MAXCLUSTERS 4294967284


/* File attribute flags for 'fat_file_t::f_attr' */
#define FATFILE_ATTR_READONLY  0x01
#define FATFILE_ATTR_HIDDEN    0x02
#define FATFILE_ATTR_SYSTEM    0x04
#define FATFILE_ATTR_VOLUMEID  0x08
#define FATFILE_ATTR_DIRECTORY 0x10
#define FATFILE_ATTR_ARCHIVE   0x20
#define FATFILE_ATTR_DEVICE    0x40
#define FATFILE_ATTR_LONGFILENAME \
 (FATFILE_ATTR_READONLY|FATFILE_ATTR_HIDDEN\
 |FATFILE_ATTR_SYSTEM|FATFILE_ATTR_VOLUMEID)

typedef struct {
union PACKED { struct PACKED {
  unsigned int ft_sec  : 5;
  unsigned int ft_min  : 6;
  unsigned int ft_hour : 5;
};u16          fd_hash; };
} PACKED fat_filetime_t;
typedef struct {
union PACKED {struct PACKED {
  unsigned int fd_day   : 5;
  unsigned int fd_month : 4;
  unsigned int fd_year  : 7;
};u16          fd_hash; };
} PACKED fat_filedate_t;


typedef struct {
 u8             fc_sectenth; /*< Creation time in 10ms resolution (0-199). */
 fat_filetime_t fc_time;     /*< Creation time. */
 fat_filedate_t fc_date;     /*< Creation date. */
} PACKED fat_filectime_t;

typedef struct {
 fat_filetime_t fc_time;     /*< Modification time. */
 fat_filedate_t fc_date;     /*< Modification date. */
} PACKED fat_filemtime_t;

#define FAT_NAMEMAX 8
#define FAT_EXTMAX  3


typedef u16 usc2ch_t; /* USC2-character. */

typedef struct {
union PACKED {struct PACKED {
union PACKED {struct PACKED {union PACKED {
#define KFATFILE_MARKER_DIREND 0x00 /*< End of directory. */
#define KFATFILE_MARKER_IS0XE5 0x05 /*< Character: First character is actually 0xE5. */
#define KFATFILE_MARKER_UNUSED 0xE5 /*< Unused entry (ignore). */
 u8                    f_marker;    /*< Special directory entry marker. */
 char                  f_name[FAT_NAMEMAX]; /*< Short (8-character) filename. */};
 char                  f_ext[FAT_EXTMAX]; /*< File extension. */};
 char                  f_nameext[FAT_NAMEMAX+FAT_EXTMAX];/*< Name+extension. */};
 u8                    f_attr;      /*< File attr. */
 /* https://en.wikipedia.org/wiki/8.3_filename
  * NOTE: After testing, the flags specified by that link are wrong.
  *       >> The following lowercase flags are correct though! */
#define FATFILE_NFLAG_NONE     0x00
#define FATFILE_NFLAG_LOWBASE  0x08 /*< Lowercase basename. */
#define FATFILE_NFLAG_LOWEXT   0x10 /*< Lowercase extension. */
 u8                    f_ntflags;   /*< NT Flags (Set of 'FATFILE_NFLAG_*'). */
 fat_filectime_t       f_ctime;     /*< Creation time. */
 fat_filedate_t        f_atime;     /*< Last access time (or rather date...). */
 le16                  f_clusterhi; /*< High 2 bytes of the file's cluster. */
 fat_filemtime_t       f_mtime;     /*< Last modification time. */
 le16                  f_clusterlo; /*< Lower 2 bytes of the file's cluster. */
 le32                  f_size;      /*< File size. */
};struct PACKED { /* Long directory entry. */
#define KFAT_LFN_SEQNUM_MIN        0x01
#define KFAT_LFN_SEQNUM_MAX        0x14
#define KFAT_LFN_SEQNUM_MAXCOUNT ((KFAT_LFN_SEQNUM_MAX-KFAT_LFN_SEQNUM_MIN)+1)
 u8                    lfn_seqnum; /*< Sequence number (KOS uses it as hint for where a name part should go). */
 /* Sizes of the three name portions. */
#define KFAT_LFN_NAME1      5
#define KFAT_LFN_NAME2      6
#define KFAT_LFN_NAME3      2
#define KFAT_LFN_NAME      (KFAT_LFN_NAME1+KFAT_LFN_NAME2+KFAT_LFN_NAME3)
 usc2ch_t              lfn_name_1[KFAT_LFN_NAME1];
 u8                    lfn_attr;   /*< Attributes (always 'KFATFILE_ATTR_LONGFILENAME') */
 u8                    lfn_type;   /*< Long directory entry type (set to ZERO(0)) */
 u8                    lfn_csum;   /*< Checksum of DOS filename (s.a.: 'kfat_genlfnchecksum'). */
 usc2ch_t              lfn_name_2[KFAT_LFN_NAME2];
 le16                  lfn_clus;   /*< First cluster (Always 0x0000). */
 usc2ch_t              lfn_name_3[KFAT_LFN_NAME3];
};
};
} PACKED fat_file_t;



typedef struct {
 /* FAT BIOS Parameter Block (common header) */
 u8   bpb_jmp[3];              /*< Jump instructions (executable). */
 char bpb_oem[8];              /*< OEM identifier */
 le16 bpb_bytes_per_sector;    /*< The number of Bytes per sector. */
 u8   bpb_sectors_per_cluster; /*< Number of sectors per cluster. */
 le16 bpb_reserved_sectors;    /*< Number of reserved sectors. */
 u8   bpb_fatc;                /*< Number of File Allocation Tables (FAT's) on the storage media (1..4). */
 le16 bpb_maxrootsize;         /*< [!FAT32] Max number of entries in the root directory. */
 le16 bpb_shortsectorc;        /*< The total sectors in the logical volume (If 0, use 'bpb_numheads' instead). */
 u8   bpb_mediatype;           /*< Indicates the media descriptor type. */
 le16 bpb_sectors_per_fat;     /*< [!FAT32] Number of sectors per FAT. */
 le16 bpb_sectors_per_track;   /*< Number of sectors per track. */
 le16 bpb_numheads;            /*< Number of heads or sides on the storage media. */
 le32 bpb_hiddensectors;       /*< Absolute sector address of the fat header (lba of the fat partition). */
 le32 bpb_longsectorc;         /*< Large amount of sector on media (Used for more than '65535' sectors) */
} PACKED fat_common_t;

#define FAT_COMMON_TOTALSECTORS(self) \
 ((self).bpb_shortsectorc\
  ? (u32)BSWAP_LE2H16((self).bpb_shortsectorc)\
  :      BSWAP_LE2H32((self).bpb_longsectorc))


typedef struct {
 fat_common_t x16_epb;           /*< BIOS Parameter Block. */
 u8           x16_driveno;       /*< Drive number. The value here should be identical to the value returned by BIOS interrupt 0x13,
                                  *  or passed in the DL register; i.e. 0x00 for a floppy disk and 0x80 for hard disks.
                                  *  This number is useless because the media is likely to be moved to another
                                  *  machine and inserted in a drive with a different drive number.  */
 u8           x16_ntflags;       /*< Windows NT Flags. (Set to 0) */
 u8           x16_signature;     /*< Signature (Must be 0x28 or 0x29). */
 le32         x16_volid;         /*< VolumeID 'Serial' number. Used for tracking volumes between computers. */
 char         x16_label[11];     /*< Volume label string. This field is padded with spaces. */
 char         x16_sysname[8];    /*< System identifier string. This field is a string representation of the FAT file system type.
                                  *  It is padded with spaces. The spec says never to trust the contents of this string for any use. */
/*
 u8           x16_bootcode[448]; / *< Boot code. * /
 u8           x16_bootsig[2];    / *< Bootable partition signature (0x55, 0xAA). * /
*/
} PACKED fat_x16_t;


typedef struct {
 /* FAT32 Extended boot record. */
 fat_common_t x32_epb;             /*< BIOS Parameter Block. */
 le32         x32_sectors_per_fat; /*< Number of sectors per FAT. */
 le16         x32_flags;           /*< Flags. */
 le16         x32_version;         /*< FAT version number. The high byte is the major version and the low byte is the minor version. FAT drivers should respect this field. */
 le32         x32_root_cluster;    /*< The cluster number of the root directory. Often this field is set to 2. */
 le16         x32_fsinfo_cluster;  /*< The sector number of the FSInfo structure. */
 le16         x32_backup_cluster;  /*< The sector number of the backup boot sector. */
 u8           x32_set2zero[12];    /*< Reserved. When the volume is formated these bytes should be zero. */
 u8           x32_driveno;         /*< Drive number. The value here should be identical to the value returned by BIOS interrupt 0x13,
                                    *  or passed in the DL register; i.e. 0x00 for a floppy disk and 0x80 for hard disks.
                                    *  This number is useless because the media is likely to be moved to another
                                    *  machine and inserted in a drive with a different drive number.  */
 u8           x32_ntflags;         /*< Windows NT Flags. (Set to 0) */
 u8           x32_signature;       /*< Signature (Must be 0x28 or 0x29). */
 le32         x32_volid;           /*< VolumeID 'Serial' number. Used for tracking volumes between computers. */
 char         x32_label[11];       /*< Volume label string. This field is padded with spaces. */
 char         x32_sysname[8];      /*< System identifier string. This field is a string representation of the FAT file system type.
                                    *  It is padded with spaces. The spec says never to trust the contents of this string for any use. */
/*
 u8           x32_bootcode[420];   / *< Boot code. * /
 u8           x32_bootsig[2];      / *< Bootable partition signature (0x55, 0xAA). * /
*/
} PACKED fat_x32_t;


typedef union {
 fat_common_t com;
 fat_x16_t    x12;
 fat_x16_t    x16;
 fat_x32_t    x32;
} fat_header_t;

typedef u32      fatsec_t; /*< Sector number. */
typedef u32      fatcls_t; /*< Cluster/Fat index number. */
typedef fatsec_t fatoff_t; /*< Sector offset in the FAT table (Add to 'f_firstfatsec' to get a 'kfatsec_t'). */
typedef fatsec_t fatdir_t;



#define FATFS_12 0
#define FATFS_16 1
#define FATFS_32 2
typedef u16 fatfs_t;  /*< FAT filesystem type (One of 'FATFS_*'). */


typedef struct fat_superblock fat_t;

#define KFATFS_CUSTER_UNUSED 0 /*< Cluster number found in the FAT, marking an unused cluster. */
typedef fatoff_t (KCALL *pfat_getfatsector)(fat_t *__restrict self, fatcls_t index);
typedef fatcls_t (KCALL *pfat_readfat)(fat_t *__restrict self, fatcls_t cluster);
typedef void     (KCALL *pfat_writefat)(fat_t *__restrict self, fatcls_t cluster, fatcls_t value);

#define fat_iseofcluster(self,cls) \
 ((cls) >= (self)->f_clseof/* || (cls) < 2*/)


struct fat_superblock {
 struct superblock   f_sb;            /*< Underlying superblock (NOTE: The 'sb_blkdev' of this is always set; aka. [1..1]). */
 fatfs_t             f_type;          /*< [const] The type of FAT filesystem. */
 char                f_oem[9];        /*< [const] OEM identifier. */
 char                f_label[12];     /*< [const] Volume label string (zero-terminated). */
 char                f_sysname[9];    /*< [const] System identifier string (zero-terminated). */
union{
 fatcls_t            f_rootcls;       /*< [const] Cluster of the root directory (FAT32 only). */
 fatsec_t            f_rootsec;       /*< [const] Sector of the root directory (non-FAT32 only). */
};
 size_t              f_secsize;       /*< [const] Size of a sector in bytes. */
 fatsec_t            f_sec4clus;      /*< [const] Amount of sectors for each cluster. */
 fatsec_t            f_sec4fat;       /*< [const] Amount of sectors for each FAT. */
 fatsec_t            f_firstdatasec;  /*< [const] First data sector. */
 fatsec_t            f_firstfatsec;   /*< [const] Sector number of the first FAT. */
 u32                 f_rootmax;       /*< [const] Max amount of entries within the root directory (max_size parameter when enumerating the root directory).
                                       *   NOTE: This field is unused when 'f_type' is 'FATFS_32' */
 u32                 f_fatcount;      /*< [const] Amount of consecutive FAT copies. */
 size_t              f_fatsize;       /*< [const] == f_sec4fat*f_secsize. */
 fatcls_t            f_clseof;        /*< [const] Cluster indices greater than or equal to this are considered EOF. */
 fatcls_t            f_clseof_marker; /*< [const] Marker that should be used to mark EOF entries in the FAT. */
 pfat_getfatsector   f_getfatsec;     /*< [const][1..1] Get the sector offset of a given FAT entry (From 'f_firstfatsec' where the FAT table portion is written to). */
 pfat_readfat        f_readfat;       /*< [const][1..1] Read a FAT entry at a given position. */
 pfat_writefat       f_writefat;      /*< [const][1..1] Write a FAT entry at a given position. */
 rwlock_t            f_fatlock;       /*< R/W lock for the CACHE (meaning to modify the cache, by reading from disk, you need to write-lock this) */
#define KFATFS_FLAG_NONE       0x00000000
#define KFATFS_FLAG_FATCHANGED 0x00000001 /*< The FAT contains changes that must be flushed. */
 u32                 f_flags;         /*< [lock(f_fatlock)] Flags describing the current fat state. */
 void               *f_fatv;          /*< [1..f_fatsize][owned][lock(f_fatlock)] FAT table. */
 byte_t             *f_fatmeta;       /*< [1..ceildiv(f_fatsize,f_sec4fat*4)][owned][lock(f_fatlock)] Bits masking valid fat sectors
                                       *  (Every first bit describes validity, and every second a changed cache). */
};
#define FAT_GETFAT(fat,cluster)       (*(fat)->f_readfat)(fat,cluster)
#define FAT_SETFAT(fat,cluster,value) (*(fat)->f_writefat)(fat,cluster,value)

PRIVATE errno_t KCALL fat_rsec(fat_t *__restrict self, fatsec_t index, USER void *buf, size_t bufsize);
PRIVATE errno_t KCALL fat_wsec(fat_t *__restrict self, fatsec_t index, USER void const *buf, size_t bufsize);

PRIVATE errno_t KCALL kfatfs_fat_freeall(fat_t *__restrict self, fatcls_t first);
PRIVATE errno_t KCALL kfatfs_fat_freeall_unlocked(fat_t *__restrict self, fatcls_t first);
PRIVATE errno_t KCALL kfatfs_fat_allocfirst(fat_t *__restrict self, fatcls_t *__restrict target);
PRIVATE errno_t KCALL kfatfs_fat_readandalloc(fat_t *__restrict self, fatcls_t index, fatcls_t *__restrict target);
PRIVATE errno_t KCALL kfatfs_fat_read(fat_t *__restrict self, fatcls_t index, fatcls_t *__restrict target);
PRIVATE errno_t KCALL kfatfs_fat_read_unlocked(fat_t *__restrict self, fatcls_t index, fatcls_t *__restrict target);
PRIVATE errno_t KCALL kfatfs_fat_write(fat_t *__restrict self, fatcls_t index, fatcls_t target);
PRIVATE errno_t KCALL kfatfs_fat_flush(fat_t *__restrict self);
PRIVATE errno_t KCALL kfatfs_fat_doflush_unlocked(fat_t *__restrict self);
PRIVATE errno_t KCALL kfatfs_fat_getfreecluster_unlocked(fat_t *__restrict self, fatcls_t *__restrict result, fatcls_t hint);

/* Read/Write callbacks for section interfacing. */
PRIVATE fatoff_t KCALL fat_getfatsec_12(fat_t *__restrict self, fatcls_t index);
PRIVATE fatoff_t KCALL fat_getfatsec_16(fat_t *__restrict self, fatcls_t index);
PRIVATE fatoff_t KCALL fat_getfatsec_32(fat_t *__restrict self, fatcls_t index);
PRIVATE fatcls_t KCALL fat_readfat_12(fat_t *__restrict self, fatcls_t cluster);
PRIVATE fatcls_t KCALL fat_readfat_16(fat_t *__restrict self, fatcls_t cluster);
PRIVATE fatcls_t KCALL fat_readfat_32(fat_t *__restrict self, fatcls_t cluster);
PRIVATE void KCALL fat_writefat_12(fat_t *__restrict self, fatcls_t cluster, fatcls_t value);
PRIVATE void KCALL fat_writefat_16(fat_t *__restrict self, fatcls_t cluster, fatcls_t value);
PRIVATE void KCALL fat_writefat_32(fat_t *__restrict self, fatcls_t cluster, fatcls_t value);
PRIVATE void KCALL fatroot_fini(struct inode *__restrict ino);
PRIVATE void KCALL trimspecstring(char *__restrict buf, size_t size);

DECL_END

#endif /* !GUARD_KERNEL_MODULES_FAT_H */
