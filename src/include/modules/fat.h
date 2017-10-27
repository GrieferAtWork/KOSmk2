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
#ifndef GUARD_INCLUDE_MODULES_FAT_H
#define GUARD_INCLUDE_MODULES_FAT_H 1

#include <hybrid/compiler.h>
#include <fs/file.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <sync/rwlock.h>
#include <stddef.h>

DECL_BEGIN

struct blkdev;
struct superblock;

#define FAT12 0
#define FAT16 1
#define FAT32 2
typedef u16 fattype_t; /*< FAT filesystem type (One of `FAT12', `FAT16' or `FAT32'). */

#define FAT12_MAXCLUSTERS 0xff4      /* 4084 */
#define FAT16_MAXCLUSTERS 0xfff4     /* 65524 */
#define FAT32_MAXCLUSTERS 0xfffffff4 /* 4294967284 */


/* File attribute flags for `fat_file_t::f_attr' */
#define ATTR_READONLY      0x01
#define ATTR_HIDDEN        0x02
#define ATTR_SYSTEM        0x04
#define ATTR_VOLUMEID      0x08
#define ATTR_DIRECTORY     0x10
#define ATTR_ARCHIVE       0x20
#define ATTR_DEVICE        0x40
/*                         0x80 */
#define ATTR_LONGFILENAME (ATTR_READONLY|ATTR_HIDDEN|ATTR_SYSTEM|ATTR_VOLUMEID)


/* FAT time types. */
typedef struct {
union PACKED { struct PACKED {
  unsigned int ft_sec  : 5;
  unsigned int ft_min  : 6;
  unsigned int ft_hour : 5;
};u16          fd_hash; };
} PACKED filetime_t;
typedef struct {
union PACKED {struct PACKED {
  unsigned int fd_day   : 5;
  unsigned int fd_month : 4;
  unsigned int fd_year  : 7;
};u16          fd_hash; };
} PACKED filedate_t;
typedef struct {
 u8         fc_sectenth; /*< Creation time in 10ms resolution (0-199). */
 filetime_t fc_time;     /*< Creation time. */
 filedate_t fc_date;     /*< Creation date. */
} PACKED filectime_t;
typedef struct {
 filetime_t fc_time;     /*< Modification time. */
 filedate_t fc_date;     /*< Modification date. */
} PACKED filemtime_t;

typedef filedate_t fileatime_t;


/* Good old DOS 8.3 names... */
#define FAT_NAMEMAX 8
#define FAT_EXTMAX  3

typedef u16 usc2ch_t; /* USC2-character. */


#define MARKER_DIREND 0x00 /*< End of directory. */
#define MARKER_IS0XE5 0x05 /*< Character: First character is actually 0xE5. */
#define MARKER_UNUSED 0xE5 /*< Unused entry (ignore). */

typedef struct PACKED {
 /* FAT directory file entry */
union PACKED {struct PACKED {
union PACKED {struct PACKED {union PACKED {
 u8                    f_marker; /*< Special directory entry marker (One of `MARKER_*'). */
 char                  f_name[FAT_NAMEMAX]; /*< Short (8-character) filename. */};
 char                  f_ext[FAT_EXTMAX]; /*< File extension. */};
 char                  f_nameext[FAT_NAMEMAX+FAT_EXTMAX];/*< Name+extension. */};
 u8                    f_attr; /*< File attr. */
 /* https://en.wikipedia.org/wiki/8.3_filename
  * NOTE: After testing, the flags specified by that link are wrong.
  *       >> The following lowercase flags are correct though! */
#define NTFLAG_NONE    0x00
#define NTFLAG_LOWBASE 0x08 /*< Lowercase basename. */
#define NTFLAG_LOWEXT  0x10 /*< Lowercase extension. */
 u8                    f_ntflags;   /*< NT Flags (Set of `NTFLAG_*'). */
 filectime_t           f_ctime;     /*< Creation time. */
 fileatime_t           f_atime;     /*< Last access time (or rather date...). */
 le16                  f_clusterhi; /*< High 2 bytes of the file's cluster. */
 filemtime_t           f_mtime;     /*< Last modification time. */
 le16                  f_clusterlo; /*< Lower 2 bytes of the file's cluster. */
 le32                  f_size;      /*< File size. */
};struct PACKED { /* Long filename entry. */
#define LFN_SEQNUM_MIN        0x01
#define LFN_SEQNUM_MAX        0x14
#define LFN_SEQNUM_MAXCOUNT ((LFN_SEQNUM_MAX-LFN_SEQNUM_MIN)+1)
 u8                    lfn_seqnum; /*< Sequence number (KOS uses it as hint for where a name part should go). */
 /* Sizes of the three name portions. */
#define LFN_NAME1      5
#define LFN_NAME2      6
#define LFN_NAME3      2
#define LFN_NAME      (LFN_NAME1+LFN_NAME2+LFN_NAME3)
 usc2ch_t              lfn_name_1[LFN_NAME1];
 u8                    lfn_attr;   /*< Attributes (always `ATTR_LONGFILENAME') */
 u8                    lfn_type;   /*< Long directory entry type (set to ZERO(0)) */
 u8                    lfn_csum;   /*< Checksum of DOS filename (s.a.: `fat_LFNchksum'). */
 usc2ch_t              lfn_name_2[LFN_NAME2];
 le16                  lfn_clus;   /*< First cluster (Always 0x0000). */
 usc2ch_t              lfn_name_3[LFN_NAME3];
};
};
} file_t;


typedef struct {
 /* FAT BIOS Parameter Block (common header) */
 u8   bpb_jmp[3];              /*< Jump instructions (executable). */
 char bpb_oem[8];              /*< OEM identifier */
 le16 bpb_bytes_per_sector;    /*< The number of Bytes per sector. */
 u8   bpb_sectors_per_cluster; /*< Number of sectors per cluster. */
 le16 bpb_reserved_sectors;    /*< Number of reserved sectors. */
 u8   bpb_fatc;                /*< Number of File Allocation Tables (FAT's) on the storage media (1..4). */
 le16 bpb_maxrootsize;         /*< [!FAT32] Max number of entries in the root directory. */
 le16 bpb_shortsectorc;        /*< The total sectors in the logical volume (If 0, use `bpb_numheads' instead). */
 u8   bpb_mediatype;           /*< Indicates the media descriptor type. */
 le16 bpb_sectors_per_fat;     /*< [!FAT32] Number of sectors per FAT. */
 le16 bpb_sectors_per_track;   /*< Number of sectors per track. */
 le16 bpb_numheads;            /*< Number of heads or sides on the storage media. */
 le32 bpb_hiddensectors;       /*< Absolute sector address of the fat header (lba of the fat partition). */
 le32 bpb_longsectorc;         /*< Large amount of sector on media (Used for more than `65535' sectors) */
} PACKED bios_parameter_block_t;

typedef struct {
 bios_parameter_block_t f16_bpb; /*< BIOS Parameter Block. */
 u8           f16_driveno;       /*< Drive number. The value here should be identical to the value returned by BIOS interrupt 0x13,
                                  *  or passed in the DL register; i.e. 0x00 for a floppy disk and 0x80 for hard disks.
                                  *  This number is useless because the media is likely to be moved to another
                                  *  machine and inserted in a drive with a different drive number. */
 u8           f16_ntflags;       /*< Windows NT Flags. (Set to 0) */
 u8           f16_signature;     /*< Signature (Must be 0x28 or 0x29). */
 le32         f16_volid;         /*< VolumeID ~Serial~ number. Used for tracking volumes between computers. */
 char         f16_label[11];     /*< Volume label string. This field is padded with spaces. */
 char         f16_sysname[8];    /*< System identifier string. This field is a string representation of the FAT file system type.
                                  *  It is padded with spaces. The spec says never to trust the contents of this string for any use. */
 u8           f16_bootcode[448]; /*< Boot code. */
 u8           f16_bootsig[2];    /*< Bootable partition signature (0x55, 0xAA). */
} PACKED fat16_header_t;


typedef struct {
 /* FAT32 Extended boot record. */
 bios_parameter_block_t f32_bpb;   /*< BIOS Parameter Block. */
 le32         f32_sectors_per_fat; /*< Number of sectors per FAT. */
 le16         f32_flags;           /*< Flags. */
 le16         f32_version;         /*< FAT version number. The high byte is the major version and the low byte is the minor version. FAT drivers should respect this field. */
 le32         f32_root_cluster;    /*< The cluster number of the root directory. Often this field is set to 2. */
 le16         f32_fsinfo_cluster;  /*< The sector number of the FSInfo structure. */
 le16         f32_backup_cluster;  /*< The sector number of the backup boot sector. */
 u8           f32_set2zero[12];    /*< Reserved. When the volume is formated these bytes should be zero. */
 u8           f32_driveno;         /*< Drive number. The value here should be identical to the value returned by BIOS interrupt 0x13,
                                    *  or passed in the DL register; i.e. 0x00 for a floppy disk and 0x80 for hard disks.
                                    *  This number is useless because the media is likely to be moved to another
                                    *  machine and inserted in a drive with a different drive number. */
 u8           f32_ntflags;         /*< Windows NT Flags. (Set to 0) */
 u8           f32_signature;       /*< Signature (Must be 0x28 or 0x29). */
 le32         f32_volid;           /*< VolumeID ~Serial~ number. Used for tracking volumes between computers. */
 char         f32_label[11];       /*< Volume label string. This field is padded with spaces. */
 char         f32_sysname[8];      /*< System identifier string. This field is a string representation of the FAT file system type.
                                    *  It is padded with spaces. The spec says never to trust the contents of this string for any use. */
 u8           f32_bootcode[420];   /*< Boot code. */
 u8           f32_bootsig[2];      /*< Bootable partition signature (0x55, 0xAA). */
} PACKED fat32_header_t;

typedef union {
 bios_parameter_block_t bpb;
 fat16_header_t         fat16;
 fat32_header_t         fat32;
} PACKED fat_header_t;

typedef u32     sector_t;  /*< Sector number. */
typedef u32     fatid_t;   /*< FAT table index. */
typedef s32     efatid_t;  /*< FAT table index, or negative error. */
typedef fatid_t cluster_t; /*< Cluster/Fat index number. */
#define FAT_CUSTER_UNUSED     0 /*< Cluster number found in the FAT table, marking an unused cluster. */
#define FAT_CUSTER_FAT16_ROOT 0 /*< Cluster ID found in parent-directory entries referring to the ROOT directory. */


typedef struct _fat fat_t;

struct fatfpos {
 pos_t     fp_namepos; /*< Absolute on-disk location of the first name header. */
 pos_t     fp_headpos; /*< Absolute on-disk location of the file's `file_t'-header. */
 cluster_t fp_namecls; /*< The cluster in which the name starts.
                        *  NOTE: Set to `f_cluster_eof_marker' for
                        *        FAT12/16 root directory entries. */
};

struct idata {
 struct fatfpos i_pos;        /*< [valid_if(!INODE_ISUPERBLOCK(:))] On-disk locations of the file header.
                               *   >> Used when writing the file header, or renaming a file. */
union{cluster_t i_cluster;    /*< On-disk file starting cluster.
                               *  NOTE:    Set to >= :f_cluster_eof for empty files.
                               *  WARNING: On FAT12/16 filesystems, this field is set to
                               *           `FAT_CUSTER_FAT16_ROOT' for the root inode (aka. superblock).
                               *  NOTE: == BSWAP_LE2H16(file_t::f_clusterhi) << 16 | BSWAP_LE2H16(file_t::f_clusterlo)
                               */
      sector_t  i_fat16_root; /*< [const][valid_if(INODE_ISUPERBLOCK(:) && :f_type != FAT32)]
                               *   Sector of the root directory of non-FAT32 fat types. */};
 rwlock_t       i_dirlock;    /*< Lock for access to directory contents. */
};


struct fatnode {
 struct inode f_inode; /*< Underlying INode. */
 struct idata f_idata; /*< INode data (Linked in `f_inode.i_user'). */
};

#define fatnode_new()          ((struct fatnode *)inode_new(sizeof(struct fatnode)))
#define fatnode_setup(self,sb) ((self)->f_inode.i_data = &(self)->f_idata,\
                                 inode_setup(&(self)->f_inode,sb))

struct filedata {
 cluster_t fd_cluster; /*< The fat index of the current cluster. */
 size_t    fd_cls_act; /*< The cluster number currently loaded in `f_cluster'
                        *  NOTE: Set to (size_t)-1 when no valid cluster could be selected. */
 size_t    fd_cls_sel; /*< The selected cluster number to-be loaded when starting to read/write data. */
 pos_t     fd_begin;   /*< [== FAT_SECTORADDR(...,FAT_CLUSTERSTART(...,fd_cluster_id))]
                        *   Start of the currently selected cluster, or undefined when an EOF cluster is selected. */
 pos_t     fd_end;     /*< [== f_cluster_begin+...->f_clustersize] The absolute end of the current cluster. */
 pos_t     fd_max;     /*< [<= fd_end] The logical end of the current cluster (may be smaller within the last cluster). */
 pos_t     fd_pos;     /*< [>= f_cluster_begin && < f_cluster_end] Current, absolute on-disk position.
                        *   NOTE: When the current cluster isn't loaded, this field is an offset
                        *         from f_cluster_begin in the theoretical cluster #f_cluster_num */
};

#define FILEDATA_OPENDIR(self,fat,idata) \
 (void)((self)->fd_cluster = (idata)->i_cluster, \
        (self)->fd_cls_sel = 0, \
        (self)->fd_cls_act = (self)->fd_cluster >= (fat)->f_cluster_eof ? (size_t)-1 : 0, \
        (self)->fd_begin = FAT_SECTORADDR(fat,FAT_CLUSTERSTART(fat,(self)->fd_cluster)), \
        (self)->fd_max = (self)->fd_end = (self)->fd_begin+(fat)->f_clustersize, \
        (self)->fd_pos = (self)->fd_begin)

/* Returns (as a `pos_t') the absolute, logical in-file position. */
#define FILEDATA_FPOS(self,fs) ((self)->fd_cls_sel*(fs)->f_clustersize+\
                               ((self)->fd_pos-(self)->fd_begin))
/* Set the absolute, logical in-file position. */
#define FATFILE_FSEEK_SET(self,fs,offset) \
 ((self)->fd_cls_sel =                  (offset)/(fs)->f_clustersize,\
  (self)->fd_pos     = (self)->fd_begin+(offset)%(fs)->f_clustersize)
#define FATFILE_FSEEK_CUR(self,fs,offset) \
 ((self)->fd_pos     -= (self)->fd_begin, \
  (self)->fd_pos     += (offset), \
  (self)->fd_cls_sel += (self)->fd_pos/(fs)->f_clustersize, \
  (self)->fd_pos     %= (fs)->f_clustersize, \
  (self)->fd_pos     += (self)->fd_begin)

/* Check if the current cluster is actually loaded. */
#define FATFILE_ISCLUSTERLOADED(self) \
 ((self)->fd_cls_act == (self)->fd_cls_sel)




struct fatfile {
 struct file     f_file; /*< Underlying file. */
 fat_t          *f_fs;   /*< [1..1][== f_file.f_node->i_super][const] Convenience link for the FAT filesystem. */
 struct filedata f_data; /*< [lock(f_file.f_lock)] Associated file data. */
};


struct fatfile_root16 {
 struct file f_file;  /*< Underlying file. */
 pos_t       f_begin; /*< [const] The absolute on-disk start position of the root directory. */
 pos_t       f_pos;   /*< [lock(f_file.f_lock)] The current in-file position. */
 pos_t       f_end;   /*< [const] The max end at which the root directory _must_ be terminated. */
};


typedef fatid_t (KCALL *pgetfat)(fat_t const *__restrict self, fatid_t id);
typedef void    (KCALL *psetfat)(fat_t       *__restrict self, fatid_t id, fatid_t value);

/* Returns a sector number offset from `f_fat_start', within
 * which the data associated with the given `id' is stored. */
typedef sector_t (KCALL *pfatsec)(fat_t const *__restrict self, fatid_t id);

#define FAT_METALOAD  0x1 /*< When set, the associated sector has been loaded. */
#define FAT_METACHNG  0x2 /*< When set, the associated sector has been changed (Write data when syncing the filesystem). */
#define FAT_METABITS    2

struct _fat {
 struct superblock f_super;         /*< The underlying superblock. */
 struct idata      f_idata;         /*< INode data for the superblock itself (Linked in `f_super.sb_root.i_user'). */
 mode_t            f_mode;          /*< Default permissions for every file on this filesystem (Defaults to 0777). */
 uid_t             f_uid;           /*< Owner UID for every file on this filesystem (Defaults to 0). */
 gid_t             f_gid;           /*< Owner GID for every file on this filesystem (Defaults to 0). */
 fattype_t         f_type;          /*< [const] Fat type. */
 char              f_oem[9];        /*< [const] OEM identifier. */
 char              f_label[12];     /*< [const] Volume label string (zero-terminated). */
 char              f_sysname[9];    /*< [const] System identifier string (zero-terminated). */
 u16               f_fat16_rootmax; /*< [const] Max amount of entries within the root directory (max_size parameter when enumerating the root directory).
                                     *   NOTE: This field is unused when `f_type' is `FATFS_32' */
 u8                f_fat_count;     /*< [const] Amount of redundant FAT copies. */
 bool              f_fat_changed;   /*< [lock(f_fat_lock)] Set to true/false to indicate changes within `f_fat_meta'. */
 u8                f_padding[2];    /*< ... */
 size_t            f_sectorsize;    /*< [const] Size of a sector in bytes. */
 size_t            f_clustersize;   /*< [const][== f_sec4clus*f_sectorsize] Size of a cluster in bytes. */
 sector_t          f_sec4clus;      /*< [const] Amount of sectors per cluster. */
 sector_t          f_sec4fat;       /*< [const] Amount of sectors per FAT table. */
 sector_t          f_dat_start;     /*< [const] First data sector. */
 sector_t          f_fat_start;     /*< [const] Sector number of the first FAT. */
 size_t            f_fat_size;      /*< [const][== f_sec4fat*f_sectorsize] Size of a single FAT table. */
union{
 fatid_t           f_fat_length;    /*< [const] The max number of FAT indirection entries. */
 cluster_t         f_cluster_eof;   /*< [const] Cluster indices greater than or equal to this are considered EOF. */
};
 cluster_t         f_cluster_eof_marker; /*< [const] Marker that should be used to mark EOF entries in the FAT. */
 /* FAT table indirection cache.
  *  - This table is a flat linked list that points clusters
  *    towards each other, allowing one to create chains of
  *    files that virtually act as one continuous string of data.
  *  - The first cluster of any file is always stored within the
  *    `file_t' header that can be found by enumerating a directory.
  *  - To read file or directory data, one does the following:
  *    >> fat_t    *fatfs   = (fat_t *)inode->i_super;
  *    >> cluster_t cluster = inode->i_data->i_cluster;
  *    >> while (cluster < fatfs->f_cluster_eof) {
  *    >>    sector_t sector = FAT_CLUSTERSTART(fatfs,i_cluster);
  *    >>    PROCESS_FILE_DATA(FAT_SECTORADDR(sector),fatfs->f_clustersize);
  *    >>    cluster = FAT_TABLEGET(fatfs,cluster);
  *    >> }
  *  - Reading the root directory on FAT32 is done the same way,
  *    but reading it on FAT12/FAT16 is a bit more complicated:
  *    >> fat_t *fatfs = (fat_t *)inode->i_super;
  *    >> assertf(INODE_ISSUPERBLOCK(inode),"Only required for the root");
  *    >> assertf(fatfs->f_type != FAT32,"Done differently on FAT32");
  *    >> PROCESS_FILE_DATA(FAT_SECTORADDR(sector),
  *    >>                  (size_t)fatfs->f_fat16_rootmax*
  *    >>                   sizeof(file_t));
  */
 struct rwlock     f_fat_lock;
 void             *f_fat_table;     /*< [lock(f_fat_lock)][1..f_fat_size|LOGICAL_LENGTH(f_fat_length)][const]
                                     *   Memory-cached version of the FAT table.
                                     *   NOTE: The way that entries within this table are read/written depends
                                     *         on the type of FAT, but `f_fat_(g|s)et' can be used for convenience.
                                     *   NOTE: The amount of entries can be read from `f_fat_length'.
                                     *   NOTE: The amount of bytes can be read from `f_fat_size'.
                                     */
 byte_t           *f_fat_meta;      /*< [lock(f_fat_lock)][1..CEILDIV(f_sec4fat,8/FAT_METABITS)|LOCIAL_LENGTH(f_sec4fat)]
                                     *  [bitset(FAT_METABITS)][const]
                                     *   A bitset used to track the load/change status of `f_fat_table'.
                                     *   Stored inside this, one can find information about what FAT
                                     *   sectors have already been loaded, and which have changed.
                                     *   NOTE: This bitset contains one entry of `FAT_METABITS'
                                     *         for each sector within the FAT lookup table.
                                     *   NOTE: `f_fat_changed' must be set to `true' while changed fat entries exist. */
 pgetfat           f_fat_get;       /*< [1..1][const] Read an entry from the FAT table. */
 psetfat           f_fat_set;       /*< [1..1][const] Write an entry to the FAT table. */
 pfatsec           f_fat_sector;    /*< [1..1][const] Return a sector offset from `f_fat_start' of a given FAT table index.
                                     *               (That is the sector in which that part of the the FAT table is stored on-disk).
                                     *   HINT: The number returned by this may be used to interact with the
                                     *        `f_fat_meta' bitset-vector, preferably using the `FAT_META_*' macros. */
};


/* NOTE: `fat_sector_index' should be obtained by calling `f_fat_sector'. */
#define FAT_META_GTLOAD(self,fat_sector_index) ((self)->f_fat_meta[(fat_sector_index)/(8/FAT_METABITS)] &   (FAT_METALOAD << (((fat_sector_index)%(8/FAT_METABITS))*FAT_METABITS)))
#define FAT_META_STLOAD(self,fat_sector_index) ((self)->f_fat_meta[(fat_sector_index)/(8/FAT_METABITS)] |=  (FAT_METALOAD << (((fat_sector_index)%(8/FAT_METABITS))*FAT_METABITS)))
#define FAT_META_UTLOAD(self,fat_sector_index) ((self)->f_fat_meta[(fat_sector_index)/(8/FAT_METABITS)] &= ~(FAT_METALOAD << (((fat_sector_index)%(8/FAT_METABITS))*FAT_METABITS)))
#define FAT_META_GTCHNG(self,fat_sector_index) ((self)->f_fat_meta[(fat_sector_index)/(8/FAT_METABITS)] &   (FAT_METACHNG << (((fat_sector_index)%(8/FAT_METABITS))*FAT_METABITS)))
#define FAT_META_STCHNG(self,fat_sector_index) ((self)->f_fat_meta[(fat_sector_index)/(8/FAT_METABITS)] |=  (FAT_METACHNG << (((fat_sector_index)%(8/FAT_METABITS))*FAT_METABITS)))
#define FAT_META_UTCHNG(self,fat_sector_index) ((self)->f_fat_meta[(fat_sector_index)/(8/FAT_METABITS)] &= ~(FAT_METACHNG << (((fat_sector_index)%(8/FAT_METABITS))*FAT_METABITS)))

/* Get/Set FAT table entires.
 * NOTE: The caller is responsible for ensuring that
 *       the associated FAT meta-data entry indicates
 *       that the requested portion of the FAT is loaded.
 *       >> assert(FAT_META_GTLOAD(self,FAT_TABLESECTOR(self,fat_id)));
 *       Similarly, the caller should mark the metadata as
 *       changed after using `FAT_TABLESET()' to write to the FAT:
 *       >> FAT_META_STCHNG(self,FAT_TABLESECTOR(self,fat_id));
 * WARNING: When the intend is to write to the fat, the
 *          caller must still ensure that the associated
 *          FAT sector is loaded, as loading it later would
 *          overwrite any changes that were made before then.
 */
#define FAT_TABLEGET(self,fat_id)       (*(self)->f_fat_get)(self,fat_id)
#define FAT_TABLESET(self,fat_id,value) (*(self)->f_fat_set)(self,fat_id,value)

#define FAT_TABLESECTOR(self,fat_id)    (*(self)->f_fat_sector)(self,fat_id)

/* Returns the on-disk address of a given sector number. */
#define FAT_SECTORADDR(self,sector_num)   \
  ((pos_t)(sector_num)*(self)->f_sectorsize)

/* Returns the sector number of a given cluster, that then spans `self->f_sec4clus' sectors. */
#define FAT_CLUSTERSTART(self,cluster_id) \
  ((sector_t)((self)->f_dat_start+((cluster_id)-2)*(self)->f_sec4clus))



/* How to mount a FAT filesystem in kernel space:
 * >> struct superblock *sb = blkdev_mksuper(bootpart,"fat",3);
 * >> fs_kmount("/",...,sb); */

DECL_END

#endif /* !GUARD_INCLUDE_MODULES_FAT_H */
