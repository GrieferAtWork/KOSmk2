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
#ifndef GUARD_INCLUDE_MODULES_EFI_H
#define GUARD_INCLUDE_MODULES_EFI_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/byteswap.h>
#include <hybrid/guid.h>

DECL_BEGIN

#define SEFIMAG 8
#define EFIMAG0 0x45
#define EFIMAG1 0x46
#define EFIMAG2 0x49
#define EFIMAG3 0x20
#define EFIMAG4 0x50
#define EFIMAG5 0x41
#define EFIMAG6 0x52
#define EFIMAG7 0x54

/* NOTE: Information about EFI structures can be found here:
 * http://rawdisk.readthedocs.io/en/latest/filesystem_diagrams.html */
typedef struct PACKED efi_struct {
 u8     gpt_signature[SEFIMAG]; /*< EFI boot magic. */
 le32   gpt_revision;           /*< GPT revision number. */
 le32   gpt_hdrsize;            /*< Header size (Usually 92) */
 le32   gpt_hdrcrc32;           /*< CRC-32 header checksum. */
 u32    gpt_reserved;           /*< Reserved (must be ZERO(0)). */
 le64   gpt_currlba;            /*< Current LBA. */
 le64   gpt_backlba;            /*< Backup LBA. */
 le64   gpt_firstpart;          /*< First usable LBA for partitions. */
 le64   gpt_lastpart;           /*< Last usable LBA for partitions. */
 guid_t gpt_guid;               /*< Disk GUID. */
 le64   gpt_partition_start;    /*< Starting LBA for partition entries. */
 le32   gpt_partition_count;    /*< Size of the partition vector (in entires). */
 le32   gpt_partition_entsz;    /*< Size of a single partition entry (Usually 128). */
 le32   gpt_partition_crc32;    /*< CRC-32 checksum for the partition vector. */
 /* NOTE: When creating a GPT partition table, remaining sector data is zeroed out. */
} efi_t;


typedef struct PACKED efi_part_struct {
 guid_t p_type_guid; /*< Partition type GUID. */
 guid_t p_part_guid; /*< Unique partition GUID. */
 le64   p_part_min;  /*< [<= p_part_max] First partition LBA index. */
 le64   p_part_max;  /*< [>= p_part_min] First partition LBA index. */
#define EFI_PART_F_ACTIVE      (1ull << 2)
#define EFI_PART_F_READONLY    (1ull << 60)
#define EFI_PART_F_HIDDEN      (1ull << 62)
#define EFI_PART_F_NOAUTOMOUNT (1ull << 63)
 le64   p_flags;     /*< Partition flags (Set of 'EFI_PART_F_*'). */
 le16   p_name[36];  /*< Partition name. */
} efi_part_t;


DECL_END

#endif /* !GUARD_INCLUDE_MODULES_EFI_H */
