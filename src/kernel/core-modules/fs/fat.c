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
#ifndef GUARD_KERNEL_CORE_MODULES_FS_FAT_C
#define GUARD_KERNEL_CORE_MODULES_FS_FAT_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <alloca.h>
#include <ctype.h>
#include <dev/blkdev.h>
#include <dirent.h>
#include <fcntl.h>
#include <fs/dentry.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <hybrid/align.h>
#include <hybrid/byteswap.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/timespec.h>
#include <hybrid/timeutil.h>
#include <hybrid/types.h>
#include <kernel/boot.h>
#include <kernel/export.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <malloc.h>
#include <modules/fat.h>
#include <sched/task.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

DECL_BEGIN

#define FAT_ISSPACE(c) __isctype((c),(_ISspace|_ISblank|_IScntrl))
#if 1
#define LFN_ISTRAIL(c) ((c) == '\0' || (c) == '\xff')
#else
#define LFN_ISTRAIL(c) __isctype((c),(_IScntrl))
#endif

#if defined(CONFIG_DEBUG) && 0
#   define FAT_DEBUG(x) x
#else
#   define FAT_DEBUG(x) (void)0
#endif

#ifndef __INTELLISENSE__
STATIC_ASSERT(sizeof(fat_header_t) == 512);
#endif

/* Load/Save the given fat-table-sector (that is: The sector associated with the FAT table itself) */
PRIVATE errno_t KCALL fat_loadtable_unlocked(fat_t *__restrict self, sector_t fat_sector_index, size_t n_sectors);
PRIVATE errno_t KCALL fat_savetable_unlocked(fat_t *__restrict self, sector_t fat_sector_index, size_t n_sectors);

/* Sync all changes to the FAT table. */
PRIVATE errno_t KCALL fat_synctable(fat_t *__restrict self);

/* High-level get/set fat entries, while ensuring that they are automatically loaded/marked as dirty. */
PRIVATE errno_t KCALL fat_get(fat_t *__restrict self, fatid_t id, fatid_t *__restrict result);
PRIVATE ATTR_UNUSED errno_t KCALL fat_set(fat_t *__restrict self, fatid_t id, fatid_t value);
#if 1
PRIVATE errno_t KCALL fat_get_unlocked(fat_t *__restrict self, fatid_t id, fatid_t *__restrict result);
PRIVATE errno_t KCALL fat_set_unlocked(fat_t *__restrict self, fatid_t id, fatid_t value);
#endif

/* Find an unused FAT table entry, preferably located at `hint'.
 * @return: -EOK:       Successfully found an unused FAT entry that was stored in `*result'
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ENOSPC:    The disk is full. - There are literally no more unused fat entries...
 * @return: E_ISERR(*): Failed to find an unused entry for some reason. */
PRIVATE errno_t KCALL fat_get_unused_unlocked(fat_t *__restrict self,
                                              fatid_t hint,
                                              fatid_t *__restrict result);
/* Given a cluster id, walk the chain it describes while
 * marking all entries along the way as unused, effectively
 * allowing them to be returned by `fat_get_unused_unlocked'.
 * @return: * :         The actual amount of deleted entries.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): Failed to delete at least one cluster for some reason. */
PRIVATE ssize_t KCALL fat_delall_unlocked(fat_t *__restrict self, fatid_t start);
PRIVATE ssize_t KCALL fat_delall(fat_t *__restrict self, fatid_t start);

/* FAT time decode/encode. */
DEFINE_MONTH_STARTING_DAY_OF_YEAR
LOCAL time_t KCALL filedate_decodetime(filedate_t self);
LOCAL void   KCALL filedate_encodetime(filedate_t *__restrict self, time_t tmt);
LOCAL time_t KCALL filetime_decodetime(filetime_t self);
LOCAL void   KCALL filetime_encodetime(filetime_t *__restrict self, time_t tmt);
#define fat_atime_decode(t,val) ((val)->tv_sec = filedate_decodetime(t),(val)->tv_nsec = 0)
#define fat_mtime_decode(t,val) ((val)->tv_sec  = filedate_decodetime((t).fc_date)+ \
                                                  filetime_decodetime((t).fc_time), \
                                 (val)->tv_nsec = 0l)
#define fat_ctime_decode(t,val) ((val)->tv_sec  = filedate_decodetime((t).fc_date), \
                                 (val)->tv_sec += filetime_decodetime((t).fc_time), \
                                 (val)->tv_nsec = (long)(t).fc_sectenth*(1000000000l/200l))
#if 0
#define fat_atime_encode(t,val)  (void)memset(&(t),0,sizeof(t));
#define fat_mtime_encode(t,val)  (void)memset(&(t),0,sizeof(t));
#define fat_ctime_encode(t,val)  (void)memset(&(t),0,sizeof(t));
#else
#define fat_atime_encode(t,val)   filedate_encodetime(&(t),(val)->tv_sec)
#define fat_mtime_encode(t,val)  (filedate_encodetime(&(t).fc_date,(val)->tv_sec), \
                                  filetime_encodetime(&(t).fc_time,(val)->tv_sec))
#define fat_ctime_encode(t,val)  (filedate_encodetime(&(t).fc_date,(val)->tv_sec), \
                                  filetime_encodetime(&(t).fc_time,(val)->tv_sec), \
                                 (t).fc_sectenth = (u8)(((val)->tv_nsec % 1000000000l)/(1000000000l/200l)))
#endif

/* The fat get/set implementation for different table sizes. */
PRIVATE sector_t KCALL fat_sec12(fat_t const *__restrict self, fatid_t id);
PRIVATE fatid_t  KCALL fat_get12(fat_t const *__restrict self, fatid_t id);
PRIVATE void     KCALL fat_set12(fat_t *__restrict self, fatid_t id, fatid_t value);
PRIVATE sector_t KCALL fat_sec16(fat_t const *__restrict self, fatid_t id);
PRIVATE fatid_t  KCALL fat_get16(fat_t const *__restrict self, fatid_t id);
PRIVATE void     KCALL fat_set16(fat_t *__restrict self, fatid_t id, fatid_t value);
PRIVATE sector_t KCALL fat_sec32(fat_t const *__restrict self, fatid_t id);
PRIVATE fatid_t  KCALL fat_get32(fat_t const *__restrict self, fatid_t id);
PRIVATE void     KCALL fat_set32(fat_t *__restrict self, fatid_t id, fatid_t value);

/* FAT INode/File/Superblock operations. */
PRIVATE REF struct file *KCALL fat_fopen(struct inode *__restrict ino, struct dentry *__restrict node_ent, oflag_t oflags);
PRIVATE void KCALL fat_finvalidate(struct file *__restrict fp, pos_t start, pos_t size);
PRIVATE ssize_t KCALL fat_fread(struct file *__restrict fp, USER void *buf, size_t bufsize);
PRIVATE ssize_t KCALL fat_fwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize);
PRIVATE off_t   KCALL fat_fseek(struct file *__restrict fp, off_t off, int whence);
PRIVATE ssize_t KCALL fat_fpread(struct file *__restrict fp, USER void *buf, size_t bufsize, pos_t pos);
PRIVATE ssize_t KCALL fat_fpwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize, pos_t pos);
PRIVATE ssize_t KCALL fat_freaddir(struct file *__restrict fp, USER struct dirent *buf, size_t bufsize, rdmode_t mode);
PRIVATE REF struct inode *KCALL fat_lookup(struct inode *__restrict dir_node, struct dentry *__restrict result_path, int flags);
PRIVATE REF struct inode *KCALL fat_mkreg(struct inode *__restrict dir_node, struct dentry *__restrict path, struct iattr const *__restrict result_attr, iattrset_t mode);
PRIVATE REF struct inode *KCALL fat_symlink(struct inode *__restrict dir_node, struct dentry *__restrict target_ent, USER char const *target_text, struct iattr const *__restrict result_attr);
PRIVATE REF struct inode *KCALL fat_mkdir(struct inode *__restrict dir_node, struct dentry *__restrict target_ent, struct iattr const *__restrict result_attr);
PRIVATE REF struct inode *KCALL fat_rename(struct inode *__restrict dst_dir, struct dentry *__restrict dst_path, struct inode *__restrict src_dir, struct dentry *__restrict src_path, struct inode *__restrict src_node);
PRIVATE errno_t KCALL fat_remove(struct inode *__restrict dir_node, struct dentry *__restrict file_path, struct inode *__restrict file_node);
PRIVATE errno_t KCALL fat_stat_dir(struct inode *__restrict ino, struct stat64 *__restrict statbuf);

PRIVATE REF struct file *KCALL fat16_root_fopen(struct inode *__restrict ino, struct dentry *__restrict node_ent, oflag_t oflags);
PRIVATE ssize_t KCALL fat16_root_fread(struct file *__restrict fp, USER void *buf, size_t bufsize);
PRIVATE ssize_t KCALL fat16_root_fwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize);
PRIVATE off_t   KCALL fat16_root_fseek(struct file *__restrict fp, off_t off, int whence);
PRIVATE ssize_t KCALL fat16_root_fpread(struct file *__restrict fp, USER void *buf, size_t bufsize, pos_t pos);
PRIVATE ssize_t KCALL fat16_root_fpwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize, pos_t pos);
PRIVATE ssize_t KCALL fat16_root_freaddir(struct file *__restrict fp, USER struct dirent *buf, size_t bufsize, rdmode_t mode);
PRIVATE REF struct inode *KCALL fat16_root_lookup(struct inode *__restrict dir_node, struct dentry *__restrict result_path, int flags);
PRIVATE REF struct inode *KCALL fat16_root_mkreg(struct inode *__restrict dir_node, struct dentry *__restrict path, struct iattr const *__restrict result_attr, iattrset_t mode);
PRIVATE REF struct inode *KCALL fat16_root_symlink(struct inode *__restrict dir_node, struct dentry *__restrict target_ent, USER char const *target_text, struct iattr const *__restrict result_attr);
PRIVATE REF struct inode *KCALL fat16_root_mkdir(struct inode *__restrict dir_node, struct dentry *__restrict target_ent, struct iattr const *__restrict result_attr);
PRIVATE REF struct inode *KCALL fat16_root_rename(struct inode *__restrict dst_dir, struct dentry *__restrict dst_path, struct inode *__restrict src_dir, struct dentry *__restrict src_path, struct inode *__restrict src_node);
PRIVATE errno_t KCALL fat16_root_remove(struct inode *__restrict dir_node, struct dentry *__restrict file_path, struct inode *__restrict file_node);


PRIVATE errno_t KCALL fat_setattr(struct inode *__restrict ino, iattrset_t changed);
PRIVATE errno_t KCALL fat_fssync(struct superblock *__restrict sb);
PRIVATE void    KCALL fat_fsfini(struct superblock *__restrict sb);

/* Search for an existing directory entry `path->d_name' and return
 * it alongside `*is_new' == false, or `-EEXIST' when `IATTR_EXISTS' isn't set in `mode'.
 * Otherwise create a new entry and pre-initialize it using `result_attr',
 * as well as set `*is_new' to true.
 * NOTE: The caller is responsible for holding a write-lock to
 *      `dir_node->i_data->i_dirlock', which must be a FAT-directory node.
 * @return: * : A reference to either a previously existing, or a newly allocated INode.
 */
PRIVATE REF struct fatnode *KCALL
fat_mkent_unlocked(struct inode *__restrict dir_node,
                   struct dentry *__restrict path, u8 ent_attr,
                   struct iattr const *__restrict result_attr,
                   iattrset_t mode, bool *__restrict is_new);
PRIVATE REF struct fatnode *KCALL
fat16_root_mkent_unlocked(struct inode *__restrict dir_node,
                          struct dentry *__restrict path, u8 ent_attr,
                          struct iattr const *__restrict result_attr,
                          iattrset_t mode, bool *__restrict is_new);
PRIVATE errno_t KCALL
fat_rment_unlocked(struct inode *__restrict dir_node,
                   struct fatnode *__restrict file_node);
PRIVATE errno_t KCALL
fat16_root_rment_unlocked(struct inode *__restrict dir_node,
                          struct fatnode *__restrict file_node);

/* Check if a given directory `node' is empty.
 * @return: -EOK:       The directory is empty.
 * @return: -ENOTEMPTY: The directory isn't empty.
 * @return: E_ISERR(*): The check failed for some reason. */
PRIVATE errno_t KCALL fat_is_empty_directory(fat_t *__restrict fs, struct fatnode *__restrict node);

/* Truncate (clear) a given FAT INode, as is done when `O_TRUNC' is passed to `open()'. */
PRIVATE errno_t KCALL fatnode_truncate_for_open(struct fatnode *__restrict open_node, struct iattr const *__restrict result_attr, iattrset_t mode);

/* Load the correct cluster within the given file when
 * NOTE: In addition, also handles the special case of an empty file
 *       becoming non-empty, when `fd_cls_act == 0' and `fd_cluster' is EOF.
 * @return: 0:          The cluster doesn't exist (the selected position is located past the file's end)
 * @return: 1:          Successfully loaded the cluster.
 * @return: E_ISERR(*): Failed to load the cluster for some reason. */
PRIVATE errno_t KCALL filedata_load(struct filedata *__restrict self, fat_t *__restrict fs, struct inode *__restrict node);

/* Similar to `filedata_load', but allocate missing chunks  */
PRIVATE ATTR_UNUSED errno_t KCALL filedata_alloc(struct filedata *__restrict self, fat_t *__restrict fs, struct inode *__restrict node);

/* Perform high-level reading/writing, starting at the file position specified within `self' */
PRIVATE ssize_t KCALL filedata_read(struct filedata *__restrict self, fat_t *__restrict fat, struct inode *__restrict node, USER void *buf, size_t bufsize);
PRIVATE ssize_t KCALL filedata_write(struct filedata *__restrict self, fat_t *__restrict fat, struct inode *__restrict node, USER void const *buf, size_t bufsize);


struct lfn_entry {
 u8   le_namepos;
 char le_namedata[LFN_NAME];
};
struct lookupdata {
 size_t            ld_entryc; /*< Amount of long name entries. */
 size_t            ld_entrya; /*< Allocated amount of LFN entries. */
 struct lfn_entry *ld_entryv; /*< [0..ln_entryc|alloc(ld_entrya)][owned]
                               *  Vector of long name entries (sorted by `ne_namepos'). */
 struct fatfpos    ld_fpos;   /*< File position information about the next entry. */
};
#define lookupdata_init(self) memset(self,0,sizeof(struct lookupdata))
#define lookupdata_fini(self) free((self)->ld_entryv)

/* Add a given long-filename entry to lookup data.
 * @return: -EOK:    Successfully added the given entry.
 * @return: -ENOMEM: Not enough available memory. */
PRIVATE errno_t KCALL lookupdata_addlfn(struct lookupdata *__restrict self,
                                        file_t const *__restrict entry);
/* Compare the stored long-filename to the given name.
 * @return: true:  The names are equal.
 * @return: false: The names are not equal. */
PRIVATE bool KCALL lookupdata_cmplfn(struct lookupdata *__restrict self,
                                     struct dentryname const *__restrict name);
/* Clear the long-filename buffer of the given lookup data. */
#define lookupdata_clrlfn(self) ((self)->ld_entryc = 0)
/* Check if long-filename data is available. */
#define lookupdata_haslfn(self) ((self)->ld_entryc != 0)

/* Search for a directory entry `name' within the given memory region.
 * NOTE: Multiple different vectors may be passed, so long as
 *       their ordering is correct and the same 'lookupdata'
 *       is passed every time.
 * @return: NULL:  The given `name' wasn't found (yet?)
 * @return: !NULL: An E_PTR()-errorcode, or the requested INode. */
PRIVATE REF struct inode *KCALL
fat_lookup_memory(struct lookupdata *__restrict lookupdata,
                  struct dentryname *__restrict name,
                  file_t const *__restrict filev, size_t filec,
                  pos_t filev_pos, cluster_t cluster_id,
                  fat_t *__restrict fatfs, bool *has_used_entries,
                  int flags);

/* Trim whitespace at the front and back of `buf'. */
PRIVATE void KCALL trimspecstring(char *__restrict buf, size_t size);


/* Support cygwin-style symbolic links as an extension, thus kind-of
 * cheating the system a bit by getting symlinks without a filesystem
 * actually designed to support them... */
PRIVATE bool support_cygwin_symlinks = true;
DEFINE_EARLY_SETUP_VAR("fat-cygwin-symlinks",support_cygwin_symlinks);
PRIVATE byte_t const symlnk_magic[] = {'!','<','s','y','m','l','i','n','k','>',0xff,0xfe};

/* Check if the given FAT-inode is a cygwin-style symbolic link. */
PRIVATE bool KCALL fat_cluster_is_symlnk(fat_t *__restrict fat, cluster_t id);
PRIVATE ssize_t KCALL fat_readlink(struct inode *__restrict ino, USER char *__restrict buf, size_t bufsize);



PRIVATE file_t const fat_newdir_template[3] = {
    [0] = { /* '.' */
        .f_name    = {'.', [1 ... FAT_NAMEMAX-1] = ' ' },
        .f_ext     = {[0 ... FAT_EXTMAX-1] = ' ' },
        .f_attr    = ATTR_DIRECTORY,
        .f_ntflags = NTFLAG_NONE,
        .f_size    = 0,
    },
    [1] = { /* '..' */
        .f_name    = {'.','.', [2 ... FAT_NAMEMAX-1] = ' ' },
        .f_ext     = {[0 ... FAT_EXTMAX-1] = ' ' },
        .f_attr    = ATTR_DIRECTORY,
        .f_ntflags = NTFLAG_NONE,
        .f_size    = 0,
    },
    [2] = {
        .f_marker  = MARKER_DIREND,
    },
};














/* Fat INode/File operation descriptors. */
PRIVATE struct inodeops const fatops_reg = {
    .f_read       = &fat_fread,
    .f_write      = &fat_fwrite,
    .f_pread      = &fat_fpread,
    .f_pwrite     = &fat_fpwrite,
    .f_seek       = &fat_fseek,
    .f_invalidate = &fat_finvalidate,
    .ino_fopen    = &fat_fopen,
    .ino_setattr  = &fat_setattr,
    .ino_readlink = &fat_readlink,
};
PRIVATE struct inodeops const fatops_dir = {
    .f_read       = &fat_fread,
    .f_write      = &fat_fwrite,
    .f_pread      = &fat_fpread,
    .f_pwrite     = &fat_fpwrite,
    .f_seek       = &fat_fseek,
    .f_readdir    = &fat_freaddir,
    .f_invalidate = &fat_finvalidate,
    .ino_fopen    = &fat_fopen,
    .ino_setattr  = &fat_setattr,
    .ino_lookup   = &fat_lookup,
    .ino_mkreg    = &fat_mkreg,
    .ino_symlink  = &fat_symlink,
    .ino_mkdir    = &fat_mkdir,
    .ino_remove   = &fat_remove,
    .ino_rename   = &fat_rename,
    .ino_stat     = &fat_stat_dir,
};
PRIVATE struct inodeops const fatops_root_16 = {
    .f_read      = &fat16_root_fread,
    .f_write     = &fat16_root_fwrite,
    .f_pread     = &fat16_root_fpread,
    .f_pwrite    = &fat16_root_fpwrite,
    .f_seek      = &fat16_root_fseek,
    .f_readdir   = &fat16_root_freaddir,
    .ino_fopen   = &fat16_root_fopen,
    .ino_lookup  = &fat16_root_lookup,
    .ino_mkreg   = &fat16_root_mkreg,
    .ino_symlink = &fat16_root_symlink,
    .ino_mkdir   = &fat16_root_mkdir,
    .ino_remove  = &fat16_root_remove,
    .ino_rename  = &fat16_root_rename,
    .ino_stat    = &fat_stat_dir,
};
PRIVATE struct inodeops const fatops_root_32 = {
    .f_read       = &fat_fread,
    .f_write      = &fat_fwrite,
    .f_pread      = &fat_fpread,
    .f_pwrite     = &fat_fpwrite,
    .f_seek       = &fat_fseek,
    .f_readdir    = &fat_freaddir,
    .f_invalidate = &fat_finvalidate,
    .ino_fopen    = &fat_fopen,
    .ino_lookup   = &fat_lookup,
    .ino_mkreg    = &fat_mkreg,
    .ino_symlink  = &fat_symlink,
    .ino_mkdir    = &fat_mkdir,
    .ino_remove   = &fat_remove,
    .ino_rename   = &fat_rename,
    .ino_stat     = &fat_stat_dir,
};
PRIVATE struct superblockops const fatops_super = {
    .sb_sync = &fat_fssync,
    .sb_fini = &fat_fsfini,
};


















PRIVATE bool KCALL
fat_cluster_is_symlnk(fat_t *__restrict fat, cluster_t id) {
 byte_t magic[sizeof(symlnk_magic)]; ssize_t error;
 if (!support_cygwin_symlinks) return false;
 task_nointr();
 HOSTMEMORY_BEGIN {
  error = blkdev_read(fat->f_super.sb_blkdev,
                      FAT_SECTORADDR(fat,FAT_CLUSTERSTART(fat,id)),
                      magic,sizeof(magic));
 }
 HOSTMEMORY_END;
 task_endnointr();
 if ((size_t)error != sizeof(symlnk_magic)) return false;
 return memcmp(magic,symlnk_magic,sizeof(symlnk_magic)) == 0;
}

PRIVATE void KCALL
utf16_to_utf8_inplace(u16 *__restrict start, size_t n_chars) {
 u16 const *src; u8 *dst;
 src = start,dst = (u8 *)start;
 while (n_chars--) *dst++ = (u8)*src++;
}

PRIVATE ssize_t KCALL
fat_readlink(struct inode *__restrict ino,
             USER char *__restrict buf, size_t bufsize) {
 struct filedata diskpos; size_t file_size;
 byte_t *temp; ssize_t error;
 size_t tempsize,read_total,read_ok,read_chars;
 struct idata *data = ino->i_data; fat_t *fat;
 if unlikely(!support_cygwin_symlinks) return -EINVAL;
 file_size = (size_t)ino->i_attr_disk.ia_siz;
 if unlikely(file_size <= sizeof(symlnk_magic)) return -EINVAL;
 read_total = (file_size-sizeof(symlnk_magic))/2;
 fat = container_of(ino->i_super,fat_t,f_super);
 diskpos.fd_cluster = data->i_cluster;
 if (diskpos.fd_cluster >= fat->f_cluster_eof) return -EINVAL;
 diskpos.fd_cls_sel = 0;
 diskpos.fd_cls_act = 0;
 diskpos.fd_begin   = FAT_SECTORADDR(fat,FAT_CLUSTERSTART(fat,diskpos.fd_cluster));
 diskpos.fd_end     = diskpos.fd_begin+fat->f_clustersize;
 diskpos.fd_max     = diskpos.fd_begin+MIN(file_size,fat->f_clustersize);
 diskpos.fd_pos     = diskpos.fd_begin;
 tempsize = MIN(file_size,fat->f_super.sb_blkdev->bd_blocksize);
 temp = (byte_t *)amalloc(tempsize);
 if unlikely(!temp) return -ENOMEM;
 HOSTMEMORY_BEGIN {
  error = filedata_read(&diskpos,fat,ino,temp,tempsize);
 }
 HOSTMEMORY_END;
 if (E_ISERR(error)) goto err;
 /* Re-validate the symlink header. */
 if ((size_t)error < sizeof(symlnk_magic) ||
     memcmp(temp,symlnk_magic,sizeof(symlnk_magic)) != 0)
 { error = -EINVAL; goto end; }
 

 /* Skip doing a partial copy if the buffer isn't large enough. */
 /* Never read more than a single block (thus preventing buffer overflow exploits).
  * HINT: Checking this here also allows us to skip re-checking `bufsize' below. */
 if unlikely(bufsize < read_total) goto end;
 read_chars = ((size_t)error-sizeof(symlnk_magic))/2;
 utf16_to_utf8_inplace((u16 *)(temp+sizeof(symlnk_magic)),read_chars);
 FAT_DEBUG(syslog(LOG_DEBUG,"SYMLINK: %$q\n",
                  read_chars,temp+sizeof(symlnk_magic)));
 if (copy_to_user(buf,temp+sizeof(symlnk_magic),read_chars*sizeof(char)))
     goto err_fault;
 read_ok = read_total;
 while (read_ok < read_total) {
  *(uintptr_t *)&buf += read_chars;
  HOSTMEMORY_BEGIN {
   error = filedata_read(&diskpos,fat,ino,temp,tempsize);
  }
  HOSTMEMORY_END;
  if (E_ISERR(error)) goto err;
  read_chars = (size_t)error/2;
  if unlikely(!read_chars) goto err_inval;
  utf16_to_utf8_inplace((u16 *)temp,read_chars);
  if (copy_to_user(buf,temp,read_chars*sizeof(char)))
      goto err_fault;
  read_ok += read_chars;
 }
 if unlikely(!read_total) {
  /* Make sure never to return ZERO(0). */
  read_total = sizeof(char);
  if (bufsize && copy_to_user(buf,"",sizeof(char)))
      read_total = (size_t)-EFAULT;
 }
end: afree(temp);
 return (ssize_t)read_total;
err_inval: read_total = (size_t)-EINVAL; goto end;
err_fault: read_total = (size_t)-EFAULT; goto end;
err: read_total = (size_t)error; goto end;
}


PRIVATE errno_t KCALL
lookupdata_addlfn(struct lookupdata *__restrict self,
                  file_t const *__restrict entry) {
 struct lfn_entry *inspos,*insend; u8 prio;
 inspos = self->ld_entryv;
 if (self->ld_entryc == self->ld_entrya) {
  if (!self->ld_entrya) self->ld_entrya = 1;
  self->ld_entrya *= 2;
  inspos = trealloc(struct lfn_entry,inspos,self->ld_entrya);
  if __unlikely(!inspos) return -ENOMEM;
  self->ld_entryv = inspos;
 }
 insend = inspos+(self->ld_entryc++);
 prio = entry->lfn_seqnum;
 /* Search for where we need to insert this long filename entry */
 while (inspos != insend) {
  if (inspos->le_namepos > prio) {
   memmove(inspos+1,inspos,
          (size_t)(insend-inspos)*
           sizeof(struct lfn_entry));
   break;
  }
  ++inspos;
 }
 inspos->le_namepos = prio;
 /* Fill the name data of the long filename */
#define charat(base,i) ((char *)entry)[base+i*2]
 inspos->le_namedata[0]  = charat(1,0);
 inspos->le_namedata[1]  = charat(1,1);
 inspos->le_namedata[2]  = charat(1,2);
 inspos->le_namedata[3]  = charat(1,3);
 inspos->le_namedata[4]  = charat(1,4);
 inspos->le_namedata[5]  = charat(14,0);
 inspos->le_namedata[6]  = charat(14,1);
 inspos->le_namedata[7]  = charat(14,2);
 inspos->le_namedata[8]  = charat(14,3);
 inspos->le_namedata[9]  = charat(14,4);
 inspos->le_namedata[10] = charat(14,5);
 inspos->le_namedata[11] = charat(28,0);
 inspos->le_namedata[12] = charat(28,1);
#undef charat
 return -EOK;
}
PRIVATE bool KCALL
lookupdata_cmplfn(struct lookupdata *__restrict self,
                  struct dentryname const *__restrict name) {
 char const *name_iter,*name_end;
 char const *last_begin,*last_end;
 struct lfn_entry *lfn_iter,*lfn_end;
 assert(self->ld_entryc);
 name_end = (name_iter = name->dn_name)+name->dn_size;
 lfn_end = (lfn_iter = self->ld_entryv)+self->ld_entryc;
 FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] LONG filename: %$q (Looking for %$q)\n",
                  self->ld_entryc*sizeof(struct lfn_entry),self->ld_entryv,
                  name->dn_size,name->dn_name));
 while (lfn_iter < lfn_end-1) {
  /* Compare full long-name entries. */
  if ((name_end-name_iter) < LFN_NAME) return false;
  if (memcmp(lfn_iter->le_namedata,name_iter,LFN_NAME*sizeof(char)) != 0) return false;
  name_iter += LFN_NAME;
  ++lfn_iter;
 }
 if ((name_end-name_iter) > LFN_NAME) return false;
 last_end = (last_begin = lfn_iter->le_namedata)+LFN_NAME;
 while (last_end != last_begin && LFN_ISTRAIL(last_end[-1])) --last_end;
 if ((last_end-last_begin) != (name_end-name_iter)) return false;
 return memcmp(last_begin,name_iter,
              (size_t)(last_end-last_begin)) == 0;
}

PRIVATE REF struct inode *KCALL
fat_lookup_memory(struct lookupdata *__restrict data,
                  struct dentryname *__restrict name,
                  file_t const *__restrict filev, size_t filec,
                  pos_t filev_pos, cluster_t cluster_id,
                  fat_t *__restrict fatfs, bool *has_used_entries,
                  int flags) {
 file_t const *iter,*end;
 REF struct fatnode *result;
 (void)flags;
 end = (iter = filev)+filec;
 for (; iter != end; ++iter) {
  if (iter->f_marker == MARKER_DIREND) return E_PTR(-ENOENT);
  if (iter->f_marker == MARKER_UNUSED) continue;
  if (has_used_entries) *has_used_entries = true;
  if (iter->f_attr == ATTR_LONGFILENAME) {
   if (!data->ld_entryc) {
    /* Track information about the first entry of an LFN chain. */
    data->ld_fpos.fp_namepos = filev_pos+((uintptr_t)iter-(uintptr_t)filev);
    data->ld_fpos.fp_namecls = cluster_id;
   }
   result = E_PTR(lookupdata_addlfn(data,iter));
   if (E_ISERR(result)) return (REF struct inode *)result;
   continue;
  }
  /* Fill in header position information. */
  data->ld_fpos.fp_headpos = filev_pos+((uintptr_t)iter-(uintptr_t)filev);
  if (lookupdata_haslfn(data)) {
   /* Check against LFN entry. */
   if (lookupdata_cmplfn(data,name))
       goto found_entry;
#ifndef CONFIG_NO_DOSFS
   /* TODO: Lookup case-insensitive. */
#endif /* !CONFIG_NO_DOSFS */
   lookupdata_clrlfn(data);
  }
  if (name->dn_size <= FAT_NAMEMAX+1+FAT_EXTMAX) {
   /* Check against regular FAT entry. */
   char *filenameiter,filename[FAT_NAMEMAX+1+FAT_EXTMAX];
   memcpy(filename,iter->f_name,FAT_NAMEMAX*sizeof(char));
   filenameiter = filename+FAT_NAMEMAX;
   while (filenameiter != filename && FAT_ISSPACE(filenameiter[-1])) --filenameiter;
   if (iter->f_ntflags&NTFLAG_LOWBASE) {
    char *tempiter;
    for (tempiter = filename; tempiter != filenameiter;
         ++tempiter) *tempiter = tolower(*tempiter);
   }
   *filenameiter++ = '.';
   memcpy(filenameiter,iter->f_ext,FAT_EXTMAX*sizeof(char));
   filenameiter += FAT_EXTMAX;
   while (filenameiter != filename && FAT_ISSPACE(filenameiter[-1])) --filenameiter;
   if (iter->f_ntflags&NTFLAG_LOWEXT) {
    char *tempiter = filenameiter;
    while (tempiter[-1] != '.') {
     --tempiter;
     *tempiter = tolower(*tempiter);
    }
   }
   if (filenameiter != filename &&
       filenameiter[-1] == '.')
       --filenameiter;
   if (*(u8 *)filename == MARKER_IS0XE5) *(u8 *)filename = 0xE5;
   FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] Short filename: %$q (Looking for %$q)\n",
                   (size_t)(filenameiter-filename),filename,
                    name->dn_size,name->dn_name));
   if ((size_t)(filenameiter-filename) == name->dn_size) {
    if (!memcmp(filename,name->dn_name,name->dn_size*sizeof(char))) {
#ifndef CONFIG_NO_DOSFS
found_lfn:
#endif /* !CONFIG_NO_DOSFS */
     data->ld_fpos.fp_namecls = cluster_id;
     data->ld_fpos.fp_namepos = filev_pos+((uintptr_t)iter-(uintptr_t)filev);
found_entry:
     /* GOTI! */
     result = fatnode_new();
     if unlikely(!result) result = E_PTR(-ENOMEM);
     else {
      /* Fill in INode information about the directory entry. */
      result->f_idata.i_cluster = (BSWAP_LE2H16(iter->f_clusterhi) << 16 |
                                   BSWAP_LE2H16(iter->f_clusterlo));
      result->f_idata.i_pos     = data->ld_fpos;
      rwlock_cinit(&result->f_idata.i_dirlock);
      result->f_inode.__i_nlink = 1;
      result->f_inode.i_data    = &result->f_idata;
      /* We (ab-)use the header position as INode number. (It's better than nothing...) */
      result->f_inode.i_ino     = (ino_t)data->ld_fpos.fp_headpos;
      fat_atime_decode(iter->f_atime,&result->f_inode.i_attr.ia_atime);
      fat_mtime_decode(iter->f_mtime,&result->f_inode.i_attr.ia_mtime);
      fat_ctime_decode(iter->f_ctime,&result->f_inode.i_attr.ia_ctime);
      result->f_inode.i_attr.ia_siz  = BSWAP_LE2H32(iter->f_size);
      result->f_inode.i_attr.ia_uid  = fatfs->f_uid;
      result->f_inode.i_attr.ia_gid  = fatfs->f_gid;
      result->f_inode.i_attr.ia_mode = fatfs->f_mode;
      if (iter->f_attr&ATTR_DIRECTORY) {
       result->f_inode.i_attr.ia_mode |= S_IFDIR;
       result->f_inode.i_ops           = &fatops_dir;
      } else {
       /* Check if this INode is a cygwin-style symbolic link. */
       if (result->f_inode.i_attr.ia_siz > sizeof(symlnk_magic) &&
           fat_cluster_is_symlnk(fatfs,result->f_idata.i_cluster))
            result->f_inode.i_attr.ia_mode |= S_IFLNK;
       else result->f_inode.i_attr.ia_mode |= S_IFREG;
       result->f_inode.i_ops = &fatops_reg;
      }
      result->f_inode.i_attr_disk    = result->f_inode.i_attr;
      /* Finally, publish the node into the superblock! */
      asserte(E_ISOK(inode_setup((struct inode *)result,
                                 (struct superblock *)fatfs,
                                  THIS_INSTANCE)));
     }
     FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] Found INode for %$q\n",
                      name->dn_size,name->dn_name));
     return &result->f_inode;
    }
#ifndef CONFIG_NO_DOSFS
    if (flags&AT_DOSPATH &&
       !memcasecmp(filename,name->dn_name,name->dn_size*sizeof(char))) {
     /* Case-insensitive match. */
     memcpy(name->dn_name,filename,name->dn_size*sizeof(char));
     goto found_lfn;
    }
#endif /* !CONFIG_NO_DOSFS */
   }
  }
 }
 return NULL;
}



LOCAL time_t KCALL
filedate_decodetime(filedate_t self) {
 time_t result; unsigned int year;
 year    = self.fd_year+1980;
 result  = YEARS2DAYS(year-LINUX_TIME_START_YEAR);
 result += MONTH_STARTING_DAY_OF_YEAR(ISLEAPYEAR(year),(self.fd_month-1) % 12);
 result += self.fd_day-1;
 return result*SECONDS_PER_DAY;
}
LOCAL void KCALL
filedate_encodetime(filedate_t *__restrict self, time_t tmt) {
 u8 i; time_t const *monthvec; unsigned int year;
 tmt /= SECONDS_PER_DAY;
 tmt += YEARS2DAYS(LINUX_TIME_START_YEAR);
 year = DAYS2YEARS(tmt);
 monthvec = __time_monthstart_yday[ISLEAPYEAR(year)];
 tmt -= YEARS2DAYS(year);
 self->fd_year = year > 1980 ? year-1980 : 0;
 /* Find the appropriate month. */
 for (i = 1; i < 12; ++i) if (monthvec[i] >= tmt) break;
 self->fd_month = i;
 self->fd_day = (tmt-monthvec[i-1])+1;
}
LOCAL time_t KCALL
filetime_decodetime(filetime_t self) {
 return ((time_t)self.ft_hour*60*60)+
        ((time_t)self.ft_min*60)+
        ((time_t)self.ft_sec*2);
}
LOCAL void KCALL
filetime_encodetime(filetime_t *__restrict self, time_t tmt) {
 self->ft_sec  = (tmt % 60)/2;
 self->ft_min  = ((tmt/60) % 60);
 self->ft_hour = ((tmt/(60*60)) % 24);
}








PRIVATE errno_t KCALL
fat_stat_dir(struct inode *__restrict ino,
         struct stat64 *__restrict statbuf) {
 /* Fat directories have a size of ZERO(0) by default.
  * This looks kind-of ugly, so we stat() them as 4096 bytes. */
 assert(INODE_ISDIR(ino));
 statbuf->st_size   = 4096;
 statbuf->st_blocks = CEILDIV(4096,512);
 return -EOK;
}
PRIVATE REF struct file *KCALL
fat_fopen(struct inode *__restrict ino,
          struct dentry *__restrict node_ent,
          oflag_t oflags) {
 struct fatfile *result; pos_t file_size;
 fat_t *fat = (fat_t *)ino->i_super;
 struct idata *idata = ino->i_data;
 /* Special case: Root-directory references on FAT12/16 filesystems. */
 if (idata->i_cluster == FAT_CUSTER_FAT16_ROOT && fat->f_type != FAT32)
     return fat16_root_fopen(&ino->i_super->sb_root,node_ent,oflags);
 if (S_ISDIR(ino->i_attr.ia_mode)) 
  file_size = ((pos_t)-1)/2;
 else {
  result = E_PTR(rwlock_read(&ino->i_attr_lock));
  if (E_ISERR(result)) return (struct file *)result;
  file_size = ino->i_attr.ia_siz;
  rwlock_endread(&ino->i_attr_lock);
 }
 result = (struct fatfile *)file_new(sizeof(struct fatfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 /* Fill in file information. */
 result->f_fs              = fat;
 result->f_data.fd_cluster = idata->i_cluster;
 result->f_data.fd_cls_sel = 0;
 result->f_data.fd_cls_act = 0;
 if (result->f_data.fd_cluster >= fat->f_cluster_eof)
     result->f_data.fd_cls_act  = (size_t)-1;
 result->f_data.fd_begin   = FAT_SECTORADDR(fat,FAT_CLUSTERSTART(fat,idata->i_cluster));
 result->f_data.fd_max     = result->f_data.fd_begin+MIN(file_size,fat->f_clustersize);
 result->f_data.fd_end     = result->f_data.fd_begin+fat->f_clustersize;
 result->f_data.fd_pos     = result->f_data.fd_begin;
 /* Publish the new stream within the associated INode. */
 file_setup((struct file *)result,ino,node_ent,oflags);
 return (struct file *)result;
}
#define FILE ((struct fatfile *)fp)
PRIVATE void KCALL
fat_finvalidate(struct file *__restrict fp,
                pos_t start, pos_t UNUSED(size)) {
 /* Simply mark the file as invalid. */
 if (FILE->f_data.fd_pos >= start)
     FILE->f_data.fd_cls_act = (size_t)-1;
}

/* Load the correct cluster within the given file when
 * NOTE: In addition, also handles the special case of an empty file
 *       becoming non-empty, when `fd_cls_act == 0' and `fd_cluster' is EOF.
 * @return: 0:          The cluster doesn't exist (the selected position is located past the file's end)
 * @return: 1:          Successfully loaded the cluster.
 * @return: E_ISERR(*): Failed to load the cluster for some reason. */
PRIVATE errno_t KCALL
filedata_load(struct filedata *__restrict self, fat_t *__restrict fs,
              struct inode *__restrict node) {
 if (self->fd_cls_act != self->fd_cls_sel) {
  errno_t temp; size_t n_ahead;
  pos_t file_pos,file_size,clus_offset;
  cluster_t file_start;
#if 0
  FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] Sector jump: %Iu -> %Iu\n",
                   self->fd_cls_act,self->fd_cls_sel));
#endif
  /* A different cluster has been selected. */
  file_pos = FILEDATA_FPOS(self,fs);
  temp = rwlock_read(&node->i_attr_lock);
  if (E_ISERR(temp)) return temp;
  if (S_ISDIR(node->i_attr.ia_mode)) 
       file_size = ((pos_t)-1)/2;
  else file_size = node->i_attr.ia_siz;
  file_start = node->i_data->i_cluster;
  rwlock_endread(&node->i_attr_lock);

  if (self->fd_cls_act == (size_t)-1) {
   /* Special case: Illegal cluster selected. - Check if it exists now. */
   if (file_size <= file_pos) return 0;
   self->fd_cluster = file_start; /* It no longer is! */
   /* Ups. - it actually is, still... (race condition?) */
   if unlikely(self->fd_cluster >= fs->f_cluster_eof) return 0;
   self->fd_pos    -= self->fd_begin;
   self->fd_cls_act = 0;
   self->fd_begin   = FAT_SECTORADDR(fs,FAT_CLUSTERSTART(fs,self->fd_cluster));
   self->fd_end     = self->fd_begin+fs->f_clustersize;
   self->fd_max     = self->fd_begin+MIN(file_size,fs->f_clustersize);
   self->fd_pos    += self->fd_begin;
   assertf(self->fd_pos < self->fd_max,
           "But the file should be large enough now (%I64u > %I64u)",
           file_size,file_pos);
   if (!self->fd_cls_sel) goto sel_this;
  }
  
  if (self->fd_cls_act > self->fd_cls_sel) {
   /* Cluster is located below. - restart from the beginning. */
   if (!file_size) return 0;
   self->fd_cluster = file_start;
   self->fd_cls_act = 0;
   if (!self->fd_cls_sel) goto sel_this;
  }

  /* Cluster is located behind. - got forward. */
  assert(self->fd_cls_sel > self->fd_cls_act);
  n_ahead = (size_t)(self->fd_cls_sel-
                     self->fd_cls_act);
  self->fd_cls_act = self->fd_cls_sel;
  do {
   /* Seek ahead a couple of clusters. */
   if (self->fd_cluster >= fs->f_cluster_eof) {
    /* NOPE! This cluster is already part of EOF! */
    self->fd_cls_act = (size_t)-1;
    return 0;
   }
   temp = fat_get(fs,self->fd_cluster,&self->fd_cluster);
   if (E_ISERR(temp)) { self->fd_cls_act = (size_t)-1; return temp; }
  } while (--n_ahead);
sel_this:
  self->fd_pos  -= self->fd_begin;
  self->fd_begin = FAT_SECTORADDR(fs,FAT_CLUSTERSTART(fs,self->fd_cluster));
  self->fd_end   = self->fd_begin+fs->f_clustersize;
  self->fd_pos  += self->fd_begin;
  assert(self->fd_pos >= self->fd_begin);
  assert(self->fd_pos <  self->fd_end);
  clus_offset  = (pos_t)self->fd_cls_sel*fs->f_clustersize;
  self->fd_max = self->fd_begin;
  /* Check if file meta-data actually allows this distance. */
  if (file_size <= clus_offset) return 0;
  /* Update the cluster's max-position. */
  assert(self->fd_max <= self->fd_end);
  self->fd_max += MIN(file_size-clus_offset,fs->f_clustersize);
  assert(self->fd_max <= self->fd_end);
 }
 assert(self->fd_pos <= self->fd_max);
 assert(self->fd_max <= self->fd_end);
 assert(self->fd_cls_act == self->fd_cls_sel);
 return 1;
}

PRIVATE ssize_t KCALL
filedata_read(struct filedata *__restrict self, fat_t *__restrict fs,
              struct inode *__restrict node, USER void *buf, size_t bufsize) {
 ssize_t temp; size_t result = 0;
 if (bufsize) for (;;) {
  size_t max_read;
  temp = filedata_load(self,fs,node);
  if (E_ISERR(temp)) return temp;
  if (temp == 0) break;
  assert(self->fd_pos >= self->fd_begin);
  assert(self->fd_pos <= self->fd_max);
  max_read = (size_t)(self->fd_max-self->fd_pos);
  if (!max_read) goto next_cluster;
  if (max_read >= bufsize)
   max_read = bufsize;
  else {
   /* TODO: Optimization: Scan ahead for non-fragmented consecutive sectors,
    *                     allowing for faster disk access when more than a
    *                     single sector can be read at once.
    * >> This could drastically improve ALLOA load times:
    *    4096 byte pages --> 8 sectors (512-byte sector size)
    *    Currently this requires up to 9 (+1 when unaligned) calls
    *    to the underlying disk driver, which if properly aligned
    *    could in theory be optimized into a single operation. */
  }
  temp = blkdev_read(fs->f_super.sb_blkdev,
                     self->fd_pos,buf,max_read);
  if unlikely(temp < 0) return temp;
  assert((size_t)temp <= max_read);
  result             += (size_t)temp;
  self->fd_pos       += (size_t)temp;
  if ((size_t)temp == bufsize || !temp) break;
  bufsize            -= (size_t)temp;
  *(uintptr_t *)&buf += (size_t)temp;
  assert(self->fd_pos <= self->fd_max);
  assert(self->fd_max <= self->fd_end);
  if (self->fd_pos == self->fd_max) {
next_cluster:
   if (self->fd_max != self->fd_end) {
    pos_t file_pos;
    pos_t file_size;
    /* The current cluster wasn't fully filled.
     * >> Check if the file grew larger since the last time this limit was set. */
    if (S_ISDIR(node->i_attr.ia_mode))
     file_size = ((pos_t)-1)/2;
    else {
     temp = rwlock_read(&node->i_attr_lock);
     if (E_ISERR(temp)) return temp;
     file_size = node->i_attr.ia_siz;
     rwlock_endread(&node->i_attr_lock);
    }
    file_pos = FILEDATA_FPOS(self,fs);
    /* If the file didn't get larger, stop. */
    if (file_pos >= file_size) {
#if 0 /* Update the file's end according to a (now) smaller file. */
     self->fd_max -= (file_pos-file_size);
     if (self->fd_max < self->fd_begin)
         self->fd_max = self->fd_begin;
#endif
     break;
    }
    /* Update the max-pointer accordingly. */
    self->fd_max += (file_size-file_pos);
    if (self->fd_max > self->fd_end)
        self->fd_max = self->fd_end;
    assert(self->fd_pos >= self->fd_begin);
    assert(self->fd_pos <= self->fd_end);
    assert(self->fd_pos <  self->fd_max);
    assert(self->fd_max <= self->fd_end);
   } else {
    /* Go to the next cluster. */
    ++self->fd_cls_sel;
    self->fd_pos = self->fd_begin;
   }
  }
 }
 return (ssize_t)result;
}

PRIVATE errno_t KCALL
filedata_alloc(struct filedata *__restrict self,
               fat_t *__restrict fs,
               struct inode *__restrict node) {
 errno_t temp; size_t n_ahead;
 pos_t file_size,clus_offset;
 cluster_t file_start; bool has_fat_lock;
 if (self->fd_cls_act != self->fd_cls_sel) {
  has_fat_lock = false;
#if 0
  FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] Sector jump: %Iu -> %Iu\n",
                   self->fd_cls_act,self->fd_cls_sel));
#endif
  /* A different cluster has been selected. */
  if (self->fd_cls_act == (size_t)-1) {
   bool has_write_lock = false;
   temp = rwlock_read(&node->i_attr_lock);
   if (E_ISERR(temp)) goto err;
load_node_again:
   if (S_ISDIR(node->i_attr.ia_mode)) 
        file_size = ((pos_t)-1)/2;
   else file_size = node->i_attr.ia_siz;
   file_start = node->i_data->i_cluster;
   /* Check if the file's initial cluster must be allocated. */
   if unlikely(file_start >= fs->f_cluster_eof) {
    /* Must create missing clusters. */
    if (!has_write_lock) {
     has_write_lock = true;
     temp = rwlock_upgrade(&node->i_attr_lock);
     if (E_ISERR(temp)) {
      if (temp == -ERELOAD) goto load_node_again;
      goto err;
     }
    }
    assert(!has_fat_lock);
    temp = rwlock_write(&fs->f_fat_lock);
    if (E_ISERR(temp)) goto err;
    has_fat_lock = true;

    /* Allocate the initial cluster. */
    file_start = node->i_data->i_pos.fp_namecls+1;
    if (file_start >= fs->f_cluster_eof) file_start = 0;
    temp = fat_get_unused_unlocked(fs,file_start,&file_start);
    if (E_ISERR(temp)) goto err;

    /* Mark the cluster as used by pointing it to EOF. */
    temp = fat_set_unlocked(fs,file_start,fs->f_cluster_eof_marker);
    if (E_ISERR(temp)) goto err;

    node->i_data->i_cluster = file_start;
    rwlock_downgrade(&node->i_attr_lock);
    /* Write the file's first cluster number to the underlying system. */
    { struct {
        le16        f_clusterhi; /*< High 2 bytes of the file's cluster. */
        filemtime_t f_mtime;     /*< Last modification time. */
        le16        f_clusterlo; /*< Lower 2 bytes of the file's cluster. */
      } buf;
      buf.f_clusterhi = BSWAP_H2LE16((u16)((u32)file_start >> 16));
      buf.f_clusterlo = BSWAP_H2LE16((u16)file_start);
      fat_mtime_encode(buf.f_mtime,&node->i_attr_disk.ia_mtime);
      HOSTMEMORY_BEGIN {
       /* Write the new directory table entry to disk. */
       temp = blkdev_writeall(fs->f_super.sb_blkdev,
                              node->i_data->i_pos.fp_headpos+
                              offsetof(file_t,f_clusterhi),
                              &buf,sizeof(buf));
      } HOSTMEMORY_END;
    }
    rwlock_endread(&node->i_attr_lock);
    if (E_ISERR(temp)) goto err;
   } else {
    if (has_write_lock)
         rwlock_endread(&node->i_attr_lock);
    else rwlock_endwrite(&node->i_attr_lock);
   }
   self->fd_cluster = file_start;
   self->fd_pos    -= self->fd_begin;
   self->fd_cls_act = 0;
   self->fd_begin   = FAT_SECTORADDR(fs,FAT_CLUSTERSTART(fs,self->fd_cluster));
   self->fd_end     = self->fd_begin+fs->f_clustersize;
   self->fd_max     = self->fd_begin+MIN(file_size,fs->f_clustersize);
   self->fd_pos    += self->fd_begin;
   assertf(self->fd_pos < self->fd_end,"%I64u >= %I64u",self->fd_pos,self->fd_end);
   if (!self->fd_cls_sel) goto sel_this;
  }
  FAT_DEBUG(syslog(LOG_DEBUG,"SELECT %I32u -> %I32u\n",self->fd_cls_act,self->fd_cls_sel));

  /* Cluster is located behind. - got forward. */
  assert(self->fd_cls_sel > self->fd_cls_act);
  n_ahead = (size_t)(self->fd_cls_sel-
                     self->fd_cls_act);
  self->fd_cls_act = self->fd_cls_sel;
  /* Acquire read/write access to the FAT table. */
  if (!has_fat_lock) {
   temp = rwlock_write(&fs->f_fat_lock);
   if (E_ISERR(temp)) goto err;
   has_fat_lock = true;
  }
  do {
   cluster_t next_cluster;
   /* Seek ahead a couple of clusters. */
   assert(self->fd_cluster < fs->f_cluster_eof);
   temp = fat_get_unlocked(fs,self->fd_cluster,&next_cluster);
   if (E_ISERR(temp)) {err_noact: self->fd_cls_act = (size_t)-1; goto err; }
   if (next_cluster >= fs->f_cluster_eof) {
    /* Allocate a new cluster. */
    temp = fat_get_unused_unlocked(fs,self->fd_cluster+1,&next_cluster);
    if (E_ISERR(temp)) goto err_noact;
    /* Point this cluster to EOF and point the previous to this one. */
    temp = fat_set_unlocked(fs,next_cluster,fs->f_cluster_eof_marker);
    if (E_ISERR(temp)) goto err_noact;
    temp = fat_set_unlocked(fs,self->fd_cluster,next_cluster);
    if (E_ISERR(temp)) { fat_set_unlocked(fs,next_cluster,FAT_CUSTER_UNUSED); goto err_noact; }
   }
   self->fd_cluster = next_cluster;
  } while (--n_ahead);
sel_this:
  /* Unlock the FAT table if we've locked it before. */
  if (has_fat_lock)
      rwlock_endwrite(&fs->f_fat_lock);
  self->fd_pos  -= self->fd_begin;
  self->fd_begin = FAT_SECTORADDR(fs,FAT_CLUSTERSTART(fs,self->fd_cluster));
  self->fd_end   = self->fd_begin+fs->f_clustersize;
  self->fd_pos  += self->fd_begin;
  assert(self->fd_pos >= self->fd_begin);
  assert(self->fd_pos <  self->fd_end);
  clus_offset  = (pos_t)self->fd_cls_sel*fs->f_clustersize;
  self->fd_max = self->fd_begin;
  /* Update the cluster's max-position according to what is currently allocated. */
  if (file_size > clus_offset)
      self->fd_max += MIN(file_size-clus_offset,fs->f_clustersize);
  assert(self->fd_max <= self->fd_end);
 }
 assert(self->fd_pos <= self->fd_max);
 assert(self->fd_max <= self->fd_end);
 assert(self->fd_cls_act == self->fd_cls_sel);
 return -EOK;
err:
 if (has_fat_lock) rwlock_endwrite(&fs->f_fat_lock);
 return temp;
}
PRIVATE ssize_t KCALL
filedata_write(struct filedata *__restrict self,
               fat_t *__restrict fs, struct inode *__restrict node,
               USER void const *buf, size_t bufsize) {
 ssize_t temp; size_t result = 0;
 while (bufsize) {
  size_t max_write;
  temp = filedata_alloc(self,fs,node);
  if (E_ISERR(temp)) return temp;
  assert(self->fd_pos >= self->fd_begin);
  assert(self->fd_pos <= self->fd_max);
  max_write = (size_t)(self->fd_end-self->fd_pos);
  assert(max_write <= fs->f_clustersize);
  if (!max_write) break;
  if (max_write > bufsize)
      max_write = bufsize;
  temp = blkdev_write(fs->f_super.sb_blkdev,
                      self->fd_pos,buf,max_write);
  if unlikely(temp < 0) return temp;
  assert((size_t)temp <= max_write);
  result             += (size_t)temp;
  self->fd_pos       += (size_t)temp;
  if (!temp) { if (!result) return -ENOSPC; break; }
  bufsize            -= (size_t)temp;
  *(uintptr_t *)&buf += (size_t)temp;
  if (self->fd_pos > self->fd_max ||
     (self->fd_pos == self->fd_max &&
      self->fd_max == self->fd_end)) {
   pos_t new_size;
   bool have_write_lock = false;
   /* Update the stored file size. */
   temp = rwlock_read(&node->i_attr_lock);
update_size_again:
   if (E_ISERR(temp)) return temp;
   new_size = ((pos_t)self->fd_cls_sel*fs->f_clustersize)+
               (self->fd_pos-self->fd_begin);
   /* XXX: Why not use lazy INode attributes here? */
   if (new_size > node->i_attr_disk.ia_siz) {
#if __SIZEOF_POS_T__ > 4
    /* Handle special case: The absolute FAT file size limit (4Gb) */
    if (new_size > (pos_t)(u32)-1) {
     assert(result >= new_size-(pos_t)(u32)-1);
     result -= new_size-(pos_t)(u32)-1;
     if (node->i_attr_disk.ia_siz != (pos_t)(u32)-1) {
      /* Write the absolutely greatest possible size. */
      new_size = (pos_t)(u32)-1;
     } else {
      if (have_write_lock)
           rwlock_endwrite(&node->i_attr_lock);
      else rwlock_endread(&node->i_attr_lock);
      return (ssize_t)result;
     }
    }
#endif
    if (!have_write_lock) {
     have_write_lock = true;
     temp = rwlock_upgrade(&node->i_attr_lock);
     if (E_ISERR(temp)) {
      if (temp == -ERELOAD) goto update_size_again;
      return temp;
     }
    }
    assert(node->i_attr_disk.ia_siz < new_size);
    node->i_attr.ia_siz = new_size;
    if (!INODE_ISDIR(node)) {
     /* Write the new size to the FAT directory table. */
     le32 ent_size = BSWAP_H2LE32((u32)new_size);
     FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] Extending file %p to %I32u bytes\n",node,(u32)new_size));
     HOSTMEMORY_BEGIN {
      temp = blkdev_writeall(fs->f_super.sb_blkdev,node->i_data->i_pos.fp_headpos+
                             offsetof(file_t,f_size),&ent_size,sizeof(ent_size));
     }
     HOSTMEMORY_END;
    }
    /* If the write was OK, mirror the disk size-attribute. */
    if (E_ISOK(temp))
        node->i_attr_disk.ia_siz = new_size;
    rwlock_endwrite(&node->i_attr_lock);
    if (E_ISERR(temp)) return temp;
   } else {
    if (have_write_lock)
         rwlock_endwrite(&node->i_attr_lock);
    else rwlock_endread(&node->i_attr_lock);
   }
   self->fd_max = self->fd_pos;
  }
  assert(self->fd_max <= self->fd_end);
  if (self->fd_pos == self->fd_end) {
   /* Go to the next cluster. */
   ++self->fd_cls_sel;
   self->fd_pos = self->fd_begin;
  }
 }
 return (ssize_t)result;
}


PRIVATE ssize_t KCALL
fat_fread(struct file *__restrict fp,
          USER void *buf, size_t bufsize) {
 ssize_t result;
 struct inode *node = fp->f_node;
 if (INODE_ISDIR(node)) {
  result = (ssize_t)rwlock_read(&node->i_data->i_dirlock);
  if (E_ISERR(result)) return result;
  result = filedata_read(&FILE->f_data,FILE->f_fs,node,buf,bufsize);
  rwlock_endread(&node->i_data->i_dirlock);
 } else {
  result = filedata_read(&FILE->f_data,FILE->f_fs,node,buf,bufsize);
 }
 return result;
}
PRIVATE ssize_t KCALL
fat_fwrite(struct file *__restrict fp,
           USER void const *buf, size_t bufsize) {
 ssize_t result;
 struct inode *node = fp->f_node;
 if (INODE_ISDIR(node)) {
  result = (ssize_t)rwlock_write(&node->i_data->i_dirlock);
  if (E_ISERR(result)) return result;
  result = filedata_write(&FILE->f_data,FILE->f_fs,node,buf,bufsize);
  rwlock_endwrite(&node->i_data->i_dirlock);
 } else {
  result = filedata_write(&FILE->f_data,FILE->f_fs,node,buf,bufsize);
 }
 return result;
}
PRIVATE off_t KCALL
fat_fseek(struct file *__restrict fp, off_t off, int whence) {
 fat_t *fs = FILE->f_fs;
 switch (whence) {
 case SEEK_SET: FATFILE_FSEEK_SET(&FILE->f_data,fs,off); break;
 case SEEK_CUR: FATFILE_FSEEK_CUR(&FILE->f_data,fs,off); break;
 {
  pos_t file_size;
  struct inode *node;
  errno_t error;
 case SEEK_END:
  node = fp->f_node;
  error = rwlock_read(&node->i_attr_lock);
  if (E_ISERR(error)) return error;
  file_size = node->i_attr.ia_siz;
  rwlock_endread(&node->i_attr_lock);
  file_size -= off;
  FATFILE_FSEEK_SET(&FILE->f_data,fs,file_size);
 } break;
 default: return -EINVAL;
 }
 return (off_t)FILEDATA_FPOS(&FILE->f_data,fs);
}
PRIVATE ssize_t KCALL
fat_fpread(struct file *__restrict fp,
           USER void *buf, size_t bufsize,
           pos_t pos) {
 struct filedata data; ssize_t result;
 struct inode *node = fp->f_node;
 errno_t error = rwlock_read(&FILE->f_file.f_lock);
 if (E_ISERR(error)) return error;
 memcpy(&data,&FILE->f_data,sizeof(struct filedata));
 rwlock_endread(&FILE->f_file.f_lock);
 FATFILE_FSEEK_SET(&data,FILE->f_fs,pos);
 /* data.fd_cls_act = (size_t)-1; */
 if (INODE_ISDIR(node)) {
  result = (ssize_t)rwlock_read(&node->i_data->i_dirlock);
  if (E_ISERR(result)) return result;
  result = filedata_read(&data,FILE->f_fs,node,buf,bufsize);
  rwlock_endread(&node->i_data->i_dirlock);
 } else {
  result = filedata_read(&data,FILE->f_fs,node,buf,bufsize);
 }
 return result;
}
PRIVATE ssize_t KCALL
fat_fpwrite(struct file *__restrict fp,
            USER void const *buf, size_t bufsize,
            pos_t pos) {
 struct filedata data; ssize_t result;
 struct inode *node = fp->f_node;
 errno_t error = rwlock_read(&FILE->f_file.f_lock);
 if (E_ISERR(error)) return error;
 memcpy(&data,&FILE->f_data,sizeof(struct filedata));
 rwlock_endread(&FILE->f_file.f_lock);
 FATFILE_FSEEK_SET(&data,FILE->f_fs,pos);
 if (INODE_ISDIR(node)) {
  result = (ssize_t)rwlock_read(&node->i_data->i_dirlock);
  if (E_ISERR(result)) return result;
  result = filedata_write(&data,FILE->f_fs,node,buf,bufsize);
  rwlock_endread(&node->i_data->i_dirlock);
 } else {
  result = filedata_write(&data,FILE->f_fs,node,buf,bufsize);
 }
 return result;
}
#undef FILE


#define FILE ((struct fatfile_root16 *)fp)
PRIVATE REF struct file *KCALL
fat16_root_fopen(struct inode *__restrict ino,
                 struct dentry *__restrict node_ent,
                 oflag_t oflags) {
#define FAT   ((fat_t *)ino)
 REF struct fatfile_root16 *result;
 assert(INODE_ISSUPERBLOCK(ino));
 result = (REF struct fatfile_root16 *)file_new(sizeof(struct fatfile_root16));
 if unlikely(!result) return E_PTR(-ENOMEM);
 /* Setup the start/end pointers according to FAT specifications. */
 result->f_begin = FAT_SECTORADDR(FAT,FAT->f_idata.i_fat16_root);
 result->f_pos   = result->f_begin;
 result->f_end   = result->f_begin+FAT->f_fat16_rootmax*sizeof(file_t);
 /* Setup the file itself. */
 file_setup(&result->f_file,ino,node_ent,oflags);
 return &result->f_file;
#undef FAT
}
PRIVATE ssize_t KCALL
fat16_root_fread(struct file *__restrict fp,
                 USER void *buf, size_t bufsize) {
 pos_t max_read; ssize_t result; fat_t *fat;
 assert(FILE->f_begin <= FILE->f_end);
 assert(FILE->f_pos >= FILE->f_begin);
 assert(FILE->f_pos <= FILE->f_end);
 assert(INODE_ISSUPERBLOCK(fp->f_node));
 max_read = FILE->f_end-FILE->f_pos;
 if (max_read > bufsize)
     max_read = bufsize;
 fat = container_of(INODE_TOSUPERBLOCK(fp->f_node),fat_t,f_super);
 result = (ssize_t)rwlock_read(&fat->f_idata.i_dirlock);
 if (E_ISERR(result)) return result;
 result = blkdev_read(fat->f_super.sb_blkdev,
                      FILE->f_pos,buf,(size_t)max_read);
 rwlock_endread(&fat->f_idata.i_dirlock);
 if (E_ISOK(result)) {
  FILE->f_pos += result;
  assert(FILE->f_pos >= FILE->f_begin);
  assert(FILE->f_pos <= FILE->f_end);
 }
 return result;
}
PRIVATE ssize_t KCALL
fat16_root_fwrite(struct file *__restrict fp,
                  USER void const *buf, size_t bufsize) {
 pos_t max_write; ssize_t result; fat_t *fat;
 assert(FILE->f_begin <= FILE->f_end);
 if (FILE->f_pos >= FILE->f_end) return 0;
 assert(FILE->f_pos >= FILE->f_begin);
 assert(FILE->f_pos <= FILE->f_end);
 assert(INODE_ISSUPERBLOCK(fp->f_node));
 max_write = FILE->f_end-FILE->f_pos;
 if (max_write > bufsize)
     max_write = bufsize;
 fat = container_of(INODE_TOSUPERBLOCK(fp->f_node),fat_t,f_super);
 result = (ssize_t)rwlock_write(&fat->f_idata.i_dirlock);
 if (E_ISERR(result)) return result;
 result = blkdev_write(fat->f_super.sb_blkdev,
                       FILE->f_pos,buf,(size_t)max_write);
 rwlock_endwrite(&fat->f_idata.i_dirlock);
 if (E_ISOK(result)) {
  FILE->f_pos += result;
  assert(FILE->f_pos >= FILE->f_begin);
  assert(FILE->f_pos <= FILE->f_end);
 }
 return result;
}
PRIVATE off_t KCALL
fat16_root_fseek(struct file *__restrict fp, off_t off, int whence) {
 pos_t new_pos;
 switch (whence) {
 case SEEK_SET: new_pos = FILE->f_begin+off; break;
 case SEEK_CUR: new_pos = FILE->f_pos+off; break;
 case SEEK_END: new_pos = FILE->f_end-off; break;
 default: return -EINVAL;
 }
 /* Make sure to clamp the new position to the root-directory boundaries. */
      if (new_pos < FILE->f_begin) new_pos = FILE->f_begin;
 else if (new_pos > FILE->f_end) new_pos = FILE->f_end;
 FILE->f_pos = new_pos;
 return (off_t)new_pos;
}

PRIVATE ssize_t KCALL
fat16_root_fpread(struct file *__restrict fp,
                  USER void *buf, size_t bufsize,
                  pos_t pos) {
 pos_t start_offset,max_read;
 ssize_t result; fat_t *fat;
 assert(FILE->f_begin <= FILE->f_end);
 assert(INODE_ISSUPERBLOCK(fp->f_node));
 start_offset = FILE->f_begin+pos;
 if (start_offset < FILE->f_begin ||
     start_offset >= FILE->f_end)
     return 0;
 max_read = (pos_t)(FILE->f_end-start_offset);
 if (max_read > bufsize)
     max_read = bufsize;
 fat = container_of(INODE_TOSUPERBLOCK(fp->f_node),fat_t,f_super);
 result = (ssize_t)rwlock_read(&fat->f_idata.i_dirlock);
 if (E_ISERR(result)) return result;
 result = blkdev_read(fat->f_super.sb_blkdev,
                      start_offset,buf,max_read);
 rwlock_endread(&fat->f_idata.i_dirlock);
 return result;
}
PRIVATE ssize_t KCALL
fat16_root_fpwrite(struct file *__restrict fp,
                   USER void const *buf, size_t bufsize,
                   pos_t pos) {
 pos_t start_offset,max_write;
 fat_t *fat; ssize_t result;
 assert(FILE->f_begin <= FILE->f_end);
 assert(INODE_ISSUPERBLOCK(fp->f_node));
 start_offset = FILE->f_begin+pos;
 if (start_offset < FILE->f_begin ||
     start_offset >= FILE->f_end)
     return 0;
 max_write = (pos_t)(FILE->f_end-start_offset);
 if (max_write > bufsize)
     max_write = bufsize;
 fat = container_of(INODE_TOSUPERBLOCK(fp->f_node),fat_t,f_super);
 result = (ssize_t)rwlock_write(&fat->f_idata.i_dirlock);
 if (E_ISERR(result)) return result;
 result = blkdev_write(fat->f_super.sb_blkdev,
                       start_offset,buf,max_write);
 rwlock_endwrite(&fat->f_idata.i_dirlock);
 return result;
}
#undef FILE

#define NODE ((struct fatnode *)ino)
PRIVATE errno_t KCALL
fat_setattr(struct inode *__restrict ino, iattrset_t changed) {
 errno_t error;
 if (changed&(IATTR_CTIME|IATTR_ATIME)) {
  if ((changed&(IATTR_CTIME|IATTR_ATIME)) ==
               (IATTR_CTIME|IATTR_ATIME)) {
   /* Both changed. */
   struct PACKED {
    filectime_t ctime;
    fileatime_t atime;
   } buf;
   fat_ctime_encode(buf.ctime,&ino->i_attr.ia_ctime);
   fat_atime_encode(buf.atime,&ino->i_attr.ia_atime);
   HOSTMEMORY_BEGIN {
    error = blkdev_writeall(ino->i_super->sb_blkdev,
                            NODE->f_idata.i_pos.fp_headpos+
                            offsetof(file_t,f_ctime),
                            &buf,sizeof(buf));
   }
   HOSTMEMORY_END;
  } else if (changed&IATTR_ATIME) {
   /* Access time changed. */
   fileatime_t atime;
   fat_atime_encode(atime,&ino->i_attr.ia_atime);
   HOSTMEMORY_BEGIN {
    error = blkdev_writeall(ino->i_super->sb_blkdev,
                            NODE->f_idata.i_pos.fp_headpos+
                            offsetof(file_t,f_atime),
                            &atime,sizeof(atime));
   }
   HOSTMEMORY_END;
  } else {
   /* Creation time changed. */
   filectime_t ctime;
   fat_ctime_encode(ctime,&ino->i_attr.ia_atime);
   HOSTMEMORY_BEGIN {
    error = blkdev_writeall(ino->i_super->sb_blkdev,
                            NODE->f_idata.i_pos.fp_headpos+
                            offsetof(file_t,f_ctime),
                            &ctime,sizeof(ctime));
   }
   HOSTMEMORY_END;
  }
  if (E_ISERR(error)) return error;
 }
 if (changed&IATTR_MTIME) {
  filemtime_t mtime;
  fat_mtime_encode(mtime,&ino->i_attr.ia_mtime);
  HOSTMEMORY_BEGIN {
   error = blkdev_writeall(ino->i_super->sb_blkdev,
                           NODE->f_idata.i_pos.fp_headpos+
                           offsetof(file_t,f_mtime),
                           &mtime,sizeof(mtime));
  }
  HOSTMEMORY_END;
  if (E_ISERR(error)) return error;
 }
 if (changed&IATTR_SIZ && !INODE_ISDIR(ino)) {
  le32 size = BSWAP_H2LE32((u32)ino->i_attr.ia_siz);
  /* TODO: Change FAT link length. */
  HOSTMEMORY_BEGIN {
   error = blkdev_writeall(ino->i_super->sb_blkdev,
                           NODE->f_idata.i_pos.fp_headpos+
                           offsetof(file_t,f_size),
                           &size,sizeof(size));
  }
  HOSTMEMORY_END;
  if (E_ISERR(error)) return error;
 }
 return -EOK;
}
PRIVATE REF struct inode *KCALL
fat_lookup(struct inode *__restrict dir_node,
           struct dentry *__restrict result_path,
           int flags) {
 struct lookupdata data; pos_t begin,end;
 size_t sector_size,cluster_num = 0; byte_t *buffer;
 struct dentryname *name = &result_path->d_name;
 REF struct inode *result = NULL;
 fat_t *fat = container_of(dir_node->i_super,fat_t,f_super);
 cluster_t cluster_id,next_cluster_id,prev_cluster_id;
 prev_cluster_id = fat->f_cluster_eof_marker;
 cluster_id  = dir_node->i_data->i_cluster;
 sector_size = fat->f_sectorsize;
 /* Special case: Root-directory references on FAT12/16 filesystems. */
 if (cluster_id == FAT_CUSTER_FAT16_ROOT && fat->f_type != FAT32)
     return fat16_root_lookup(&dir_node->i_super->sb_root,result_path,flags);
 /* XXX: This buffer allocation is very expensive,
  *      but the stack might not be large enough. */
 buffer = (byte_t *)malloc(sector_size);
 if unlikely(!buffer) return E_PTR(-ENOMEM);
 lookupdata_init(&data);
 result = E_PTR(rwlock_read(&dir_node->i_data->i_dirlock));
 if (E_ISERR(result)) goto done;
 while (cluster_id < fat->f_cluster_eof) {
  bool has_used_entries = false;
  /* Figure out from where to where this cluster goes. */
  begin  = FAT_SECTORADDR(fat,FAT_CLUSTERSTART(fat,cluster_id));
  end    = begin+fat->f_clustersize;
  while (begin < end) {
   pos_t part_size = (pos_t)(end-begin);
   /* Read the directory one sector at a time. */
   HOSTMEMORY_BEGIN {
    result = E_PTR(blkdev_readall(fat->f_super.sb_blkdev,
                                  begin,buffer,sector_size));
   }
   HOSTMEMORY_END;
   if (E_ISERR(result)) break;
   if (part_size > sector_size)
       part_size = sector_size;
   result = fat_lookup_memory(&data,name,(file_t *)buffer,
                              part_size/sizeof(file_t),
                              begin,cluster_id,fat,
                             &has_used_entries,flags);
   if (result != NULL) break;
   begin += sector_size;
  }
  if (result != NULL) break;
  /* Go to the next cluster. */
  result = E_PTR(fat_get(fat,cluster_id,&next_cluster_id));
  if (E_ISERR(result)) break;
  if (!has_used_entries) {
   errno_t unlink_error;
   /* Without any used entries, we can unlink this cluster. */
   if (prev_cluster_id == fat->f_cluster_eof_marker) {
    dir_node->i_data->i_cluster = next_cluster_id;
    if (INODE_ISSUPERBLOCK(dir_node)) {
     /* Save the new root directory starting cluster in the FAT header. */
     le32 buf = BSWAP_H2LE32(next_cluster_id);
     assert(fat->f_type == FAT32);
     HOSTMEMORY_BEGIN {
      unlink_error = blkdev_writeall(fat->f_super.sb_blkdev,
                                     0+offsetof(fat32_header_t,f32_root_cluster),
                                    &buf,sizeof(buf));
     }
     HOSTMEMORY_END;
    } else {
     /* Save the new directory start in the parent entry. */
     struct {
      le16        f_clusterhi;
      filemtime_t f_mtime;
      le16        f_clusterlo;
     } buf;
     fat_mtime_encode(buf.f_mtime,&dir_node->i_attr_disk.ia_mtime);
     buf.f_clusterlo = BSWAP_H2LE16((u16)((u32)next_cluster_id >> 16));
     buf.f_clusterhi = BSWAP_H2LE16((u16)next_cluster_id);
     HOSTMEMORY_BEGIN {
      unlink_error = blkdev_writeall(fat->f_super.sb_blkdev,
                                     dir_node->i_data->i_pos.fp_headpos+
                                     offsetof(file_t,f_clusterhi),
                                    &buf,sizeof(buf));
     }
     HOSTMEMORY_END;
    }
   } else {
    /* Simply unlink an intermediate cluster
     * by pointing the previous to the next. */
    unlink_error = fat_set(fat,prev_cluster_id,next_cluster_id);
   }
   /* Now mark the cluster we've just unlinked as unused. */
   if (E_ISOK(unlink_error)) {
    /* Invalid the INode to prevent files still pointing at the (now) invalid cluster. */
    task_nointr();
    inode_invalidate_data(dir_node,(pos_t)cluster_num*fat->f_clustersize,(pos_t)-1);
    task_endnointr();
    unlink_error = fat_set(fat,cluster_id,FAT_CUSTER_UNUSED);
   }
   if (E_ISOK(unlink_error))  {
    syslog(LOG_FS|LOG_INFO,"[FAT] Unlinked unused cluster %I32u\n",cluster_id);
   } else {
    syslog(LOG_FS|LOG_ERROR,"[FAT] Failed to unlink unused cluster %I32u: %[errno]\n",
           cluster_id,-unlink_error);
   }
   goto keep_prev_clusterid;
  }
  ++cluster_num;
  prev_cluster_id = cluster_id;
keep_prev_clusterid:
  cluster_id = next_cluster_id;
 }
 rwlock_endread(&dir_node->i_data->i_dirlock);
done:
 lookupdata_fini(&data);
 /* Return `-ENOENT' when the entry wasn't found. */
 if (!result) result = E_PTR(-ENOENT);
 free(buffer);
 return result;
}

STATIC_ASSERT(offsetof(file_t,f_marker) == 0);

#define FAT  ((fat_t *)dir_node)
PRIVATE REF struct inode *KCALL
fat16_root_lookup(struct inode *__restrict dir_node,
                  struct dentry *__restrict result_path, int flags) {
 struct lookupdata data; pos_t begin,end;
 size_t sector_size; byte_t *buffer;
 struct dentryname *name = &result_path->d_name;
 REF struct inode *result = NULL;
 sector_size = FAT->f_sectorsize;
 /* XXX: This buffer allocation is very expensive,
  *      but the stack might not be large enough. */
 buffer = (byte_t *)malloc(sector_size);
 if unlikely(!buffer) return E_PTR(-ENOMEM);
 begin = FAT_SECTORADDR(FAT,FAT->f_idata.i_fat16_root);
 end   = begin+FAT->f_fat16_rootmax*sizeof(file_t);
 lookupdata_init(&data);
 result = E_PTR(rwlock_read(&FAT->f_idata.i_dirlock));
 if (E_ISERR(result)) goto done;
 while (begin < end) {
  pos_t part_size = (pos_t)(end-begin);
  /* Read the directory one sector at a time. */
  HOSTMEMORY_BEGIN {
   result = E_PTR(blkdev_readall(FAT->f_super.sb_blkdev,
                                 begin,buffer,sector_size));
  }
  HOSTMEMORY_END;
  if (E_ISERR(result)) break;
  if (part_size > sector_size)
      part_size = sector_size;
  result = fat_lookup_memory(&data,name,(file_t *)buffer,
                             part_size/sizeof(file_t),begin,
                             FAT->f_cluster_eof_marker,FAT,NULL,flags);
  if (result != NULL) break;
  begin += sector_size;
 }
 rwlock_endread(&FAT->f_idata.i_dirlock);
done:
 lookupdata_fini(&data);
 /* Return `-ENOENT' when the entry wasn't found. */
 if (!result) result = E_PTR(-ENOENT);
 free(buffer);
 return result;
}
#undef FAT
#undef NODE

LOCAL ATTR_CONST int KCALL dos8dot3_isvalid(char ch) {
 if (ch <= 31 || ch == 127) return 0;
 return !strchr("\"*+,/:;<=>?\\[]|.",ch);
}

/* Fill a short filename entry 'f', using the provided information
 * and return the number of LFN entries required to-be prepended.
 * when the entry is written.
 * NOTE: When the filename described by the short directory entry
 *       already exists, the function should be re-called with
 *      `disambiguation' incremented by one (start out with ZERO(0) the first time)
 *       This can be repeated until `disambiguation' is equal to
 *      `FAT_8DOT3_MAX_DISAMBIGUATION+1', at which point all possible
 *       filename combination have been taken up and the file cannot
 *       actually be created. */
#define FAT_8DOT3_MAX_DISAMBIGUATION  (0xffff*9)
PRIVATE size_t KCALL fat_make8dot3(file_t *__restrict f,
                                   struct dentryname *__restrict dname,
                                   size_t disambiguation) {
 char const *extstart,*iter,*end; char *dst,ch,*name;
 size_t basesize,extsize,matchsize,namesize,result;
 int retry_hex,retry_dig; bool has_mixed_case = false;
 assert(disambiguation < FAT_8DOT3_MAX_DISAMBIGUATION);
 f->f_ntflags = NTFLAG_NONE;
 name = dname->dn_name,namesize = dname->dn_size;
 /* Strip leading+terminating dots & space. */
 while (namesize && (FAT_ISSPACE(*name) || *name == '.')) ++name,--namesize;
 while (namesize && (FAT_ISSPACE(name[namesize-1]))) --namesize;
 extstart = name+namesize;
 while (extstart != name && (extstart[-1] != '.')) --extstart;
 if (extstart == name) {
  extstart = name+namesize; /* No extension */
  extsize  = 0;
  basesize = namesize;
 } else {
  basesize = (size_t)(extstart-name)-1;
  extsize  = (namesize-basesize)-1;
 }
 memset(f->f_nameext,' ',sizeof(f->f_nameext));

 /* Strip space characters before the extension and after the name. */
 while (namesize && FAT_ISSPACE(name[namesize-1])) ++name,--namesize;
 while (extsize && FAT_ISSPACE(*extstart)) ++extstart,--extsize;

 /* Generate the extension */
 if (extsize) {
  dst = f->f_nameext+FAT_NAMEMAX;
  end = (iter = extstart)+MIN(extsize,FAT_EXTMAX);
  f->f_ntflags |= NTFLAG_LOWEXT;
  while (iter != end) {
   ch = *iter++;
   if (islower(ch)) {
    if (!(f->f_ntflags&NTFLAG_LOWEXT))
        has_mixed_case = true;
    ch = toupper(ch);
   } else {
    if (f->f_ntflags&NTFLAG_LOWEXT && iter != extstart)
        has_mixed_case = true;
    f->f_ntflags &= ~(NTFLAG_LOWEXT);
   }
   if unlikely(!dos8dot3_isvalid(ch)) ch = '~';
   *dst++ = ch;
  }
 }

 /* Confirm that the name and extension fit DOS8.3 */
 if (basesize <= FAT_NAMEMAX && extsize <= FAT_EXTMAX &&
     name == dname->dn_name && name+basesize == extstart-!!extsize &&
     extstart+extsize == dname->dn_name+dname->dn_size) {
  /* We can generate a (possibly mixed-case) 8.3-compatible filename */
  end = (iter = name)+basesize,dst = f->f_name;
  f->f_ntflags |= NTFLAG_LOWBASE;
  while (iter != end) {
   ch = *iter++;
   if (islower(ch)) {
    if (!(f->f_ntflags&NTFLAG_LOWBASE))
        has_mixed_case = true;
    ch = toupper(ch);
   } else {
    if (f->f_ntflags&NTFLAG_LOWBASE && iter != extstart)
        has_mixed_case = true;
    f->f_ntflags &= ~(NTFLAG_LOWBASE);
   }
   if unlikely(!dos8dot3_isvalid(ch)) ch = '~';
   *dst++ = ch;
  }
  /* Fix 0xE5 --> 0x05 (srsly, dos?) */
  if (((u8 *)f->f_name)[0] == 0xE5) ((u8 *)f->f_name)[0] = 0x05;
  if (has_mixed_case) goto need_lfn;
  result = 0;
 } else {
need_lfn:
  f->f_ntflags = NTFLAG_NONE;
  /* Must __MUST__ generate a long filename, also
   * taking the value of 'retry' into consideration.
   * Now for the hard part: The filename itself... */
  retry_hex = disambiguation/9,retry_dig = (disambiguation % 9);

  /* The first 2 short characters always match the
   * first 2 characters of the original base (in uppercase).
   * If no hex-range retries are needed, the first 6 match. */
  matchsize = retry_hex ? 2 : 6;
  if (matchsize > basesize) matchsize = basesize;
  end = (iter = name)+matchsize,dst = f->f_nameext;
  while (iter != end) {
   ch = toupper(*iter++);
   *dst++ = dos8dot3_isvalid(ch) ? ch : '~';
  }
  if (retry_hex) {
   PRIVATE char const xch[16] = {'0','1','2','3','4','5','6','7',
                                 '8','9','A','B','C','D','E','F'};
   /* Following the matching characters are 4 hex-chars
    * whenever more than 9 retry attempts have failed
    * >> This multiplies the amount of available names by 0xffff */
   *dst++ = xch[(retry_hex & 0xf000) >> 12];
   *dst++ = xch[(retry_hex & 0x0f00) >> 8];
   *dst++ = xch[(retry_hex & 0x00f0) >> 4];
   *dst++ = xch[(retry_hex & 0x000f)];
  }
  assert(dst <= f->f_nameext+6);
  /* Following the shared name and the hex part is always a tilde '~' */
  *dst++ = '~';
  /* The last character then, is the non-hex digit (1..9) */
  *dst = '1'+retry_dig;
  /* Fix 0xE5 --> 0x05 (srsly, dos?) */
  if (((u8 *)f->f_nameext)[0] == 0xE5)
      ((u8 *)f->f_nameext)[0] = 0x05;

  result = CEILDIV(dname->dn_size,LFN_NAME);
 }

 /* And we're done! */
 return result;
}



PRIVATE u8 KCALL fat_LFNchksum(char const *short_name) {
 u8 result = 0; char const *iter,*end;
 /* Algorithm can be found here:
  * https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system */
 end = (iter = short_name)+(FAT_NAMEMAX+FAT_EXTMAX);
 for (; iter != end; ++iter) result = ((result & 1) << 7) + (result >> 1) + *iter;
 return result;
}

/* To-be used in conjunction with `fat_make8dot3()', fill in
 * 'f' as the 'number`th' LFN entry for the given filename `name'.
 * WARNING: The caller must not call this function when `fat_make8dot3()'
 *          returned ZERO(0), or call it with a 'number' greater than
 *          the return value of `fat_make8dot3()' for the same `name'.
 * @param: chksum: == fat_LFNchksum(DOS83_ENTRY.f_nameext);
 */
PRIVATE void KCALL fat_makeLFN(file_t *__restrict f,
                               struct dentryname *__restrict dname,
                               size_t number, u8 chksum) {
 char part[LFN_NAME]; size_t partsize,offset;
 assertf(number < CEILDIV(dname->dn_size,LFN_NAME),"Invalid LFN index");
 offset = number*LFN_NAME;
 partsize = MIN(dname->dn_size-offset,LFN_NAME);
 memcpy(part,dname->dn_name+offset,partsize*sizeof(char));
 /* XXX: This is technically flawed: FAT only wants one
  *      ZERO-character, followed by the rest being 0xffff.
  *      But putting that aside: WHO CARES! */
 memset(part+partsize,'\0',(LFN_NAME-partsize)*sizeof(char));
 /* Fill in the LFN name entry. */
 f->lfn_seqnum    = LFN_SEQNUM_MIN+number;
 f->lfn_type      = 0;
 f->lfn_clus      = (le16)0;
 f->lfn_attr      = ATTR_LONGFILENAME;
 f->lfn_csum      = chksum;
 f->lfn_name_1[0] = (usc2ch_t)part[0];
 f->lfn_name_1[1] = (usc2ch_t)part[1];
 f->lfn_name_1[2] = (usc2ch_t)part[2];
 f->lfn_name_1[3] = (usc2ch_t)part[3];
 f->lfn_name_1[4] = (usc2ch_t)part[4];
 f->lfn_name_2[0] = (usc2ch_t)part[5];
 f->lfn_name_2[1] = (usc2ch_t)part[6];
 f->lfn_name_2[2] = (usc2ch_t)part[7];
 f->lfn_name_2[3] = (usc2ch_t)part[8];
 f->lfn_name_2[4] = (usc2ch_t)part[9];
 f->lfn_name_2[5] = (usc2ch_t)part[10];
 f->lfn_name_3[0] = (usc2ch_t)part[11];
 f->lfn_name_3[1] = (usc2ch_t)part[12];
}

#ifndef __INTELLISENSE__
#define FAT16_ROOT
#include "fat-mkent.c.inl"
#include "fat-mkent.c.inl"
#endif

PRIVATE errno_t KCALL
fat_rment_unlocked(struct inode *__restrict dir_node,
                   struct fatnode *__restrict file_node) {
 file_t empty_file; errno_t temp = -EOK;
 pos_t cluster_end,clear_pos = file_node->f_idata.i_pos.fp_namepos;
 cluster_t name_cluster = file_node->f_idata.i_pos.fp_namecls;
 fat_t *fs = container_of(dir_node->i_super,fat_t,f_super);
 assert(rwlock_writing(&dir_node->i_data->i_dirlock));
 memset(&empty_file,0,sizeof(file_t));
#if MARKER_UNUSED != 0
 empty_file.f_marker = MARKER_UNUSED;
#endif
 HOSTMEMORY_BEGIN {
load_cluster_end:
  assert(name_cluster < fs->f_cluster_eof);
  cluster_end = FAT_SECTORADDR(fs,FAT_CLUSTERSTART(fs,name_cluster))+fs->f_clustersize;
  for (;;) {
   assert(clear_pos != cluster_end);
   /* Write an empty file marked as unused. */
   temp = blkdev_writeall(fs->f_super.sb_blkdev,clear_pos,
                         &empty_file,sizeof(empty_file));
   if (E_ISERR(temp)) break;
   if (clear_pos == file_node->f_idata.i_pos.fp_headpos) break;
   clear_pos += sizeof(file_t);
   if (clear_pos == cluster_end) {
    temp = fat_get_unlocked(fs,name_cluster,&name_cluster);
    if (E_ISERR(temp) || name_cluster >= fs->f_cluster_eof) break;
    goto load_cluster_end;
   }
  }
  /* TODO: Check if `file_node's' cluster is now empty. If so,
   *       and if it is the last cluster, free now unused clusters. */
 }
 HOSTMEMORY_END;
 return temp;
}
PRIVATE errno_t KCALL
fat16_root_rment_unlocked(struct inode *__restrict dir_node,
                          struct fatnode *__restrict file_node) {
 file_t empty_file; errno_t temp = -EOK;
 fat_t *fs = container_of(dir_node,fat_t,f_super.sb_root);
 pos_t iter,last;
 memset(&empty_file,0,sizeof(file_t));
#if MARKER_UNUSED != 0
 empty_file.f_marker = MARKER_UNUSED;
#endif
 /* Very simple: All we have to do, is to mark the directory entry as unused. */
 assert(rwlock_writing(&dir_node->i_data->i_dirlock));
 iter = file_node->f_idata.i_pos.fp_namepos;
 last = file_node->f_idata.i_pos.fp_headpos;
 HOSTMEMORY_BEGIN {
  do {
   temp = blkdev_writeall(fs->f_super.sb_blkdev,iter,
                          &empty_file,sizeof(file_t));
   if (E_ISERR(temp)) break;
  } while ((iter += sizeof(file_t)) <= last);
 }
 HOSTMEMORY_END;
 return temp;
}

PRIVATE errno_t KCALL
fat_is_empty_directory(fat_t *__restrict fs,
                       struct fatnode *__restrict node) {
 struct filedata reader; ssize_t temp;
 reader.fd_cluster = node->f_idata.i_cluster;
 reader.fd_cls_sel = 0;
 reader.fd_cls_act = 0;
 if (reader.fd_cluster >= fs->f_cluster_eof)
     reader.fd_cls_act  = (size_t)-1;
 reader.fd_begin   = FAT_SECTORADDR(fs,FAT_CLUSTERSTART(fs,reader.fd_cluster));
 reader.fd_max     = reader.fd_begin+fs->f_clustersize;
 reader.fd_end     = reader.fd_begin+fs->f_clustersize;
 reader.fd_pos     = reader.fd_begin;
 for (;;) {
  file_t fp;
  HOSTMEMORY_BEGIN {
   temp = filedata_read(&reader,fs,&node->f_inode,&fp,sizeof(file_t));
  }
  HOSTMEMORY_END;
  if (E_ISERR(temp)) return (errno_t)temp;
  if ((size_t)temp < sizeof(file_t)) break;
  /* Simple case: The entry marks the directory end. */
  if (fp.f_marker == MARKER_DIREND) break;
  /* Simple case: The entry is marked as unused. */
  if (fp.f_marker == MARKER_UNUSED) continue;
  /* According to wikipedia, VOLUMEID entires shouldn't not prevent directory deletion.
   * >> This makes sense, because LFN entries are marked as VOLUMEID, but
   *    shouldn't be responsible for keeping a directory from being removed. */
  if (fp.f_attr&ATTR_VOLUMEID) continue;
  /* Special case: Ignore `"."' and  `".."' entries. */
  if (fp.f_attr&ATTR_DIRECTORY) {
   if (FAT_ISSPACE(fp.f_ext[0])) {
    if (FAT_ISSPACE(fp.f_name[0])) continue;
    if (fp.f_name[0] == '.') {
     if (FAT_ISSPACE(fp.f_name[1])) continue; /* '.' */
     if (fp.f_name[1] == '.' &&
         FAT_ISSPACE(fp.f_name[2])) continue; /* '..' */
    }
   } else if (fp.f_ext[0] == '.') {
    if (FAT_ISSPACE(fp.f_ext[1])) {
     if (FAT_ISSPACE(fp.f_name[0])) continue; /* '.' */
     if (fp.f_name[0] == '.' &&
         FAT_ISSPACE(fp.f_name[1])) continue; /* '..' */
    } else {
     if (FAT_ISSPACE(fp.f_ext[2]) &&
         FAT_ISSPACE(fp.f_name[0])) continue; /* '..' */
    }
   }
  }

  /* XXX: Can there be invisible, broken entries that manage to get here? */
  /* Not empty! */
  /*FAT_DEBUG*/(syslog(LOG_DEBUG,"Directory not empty (still contains %.2I8X - %$q)\n",
                   fp.f_attr,sizeof(fp.f_nameext),fp.f_nameext));
  return -ENOTEMPTY;
 }
 return -EOK;
}

PRIVATE errno_t KCALL
fat_remove(struct inode *__restrict dir_node,
           struct dentry *__restrict file_path,
           struct inode *__restrict file_node) {
 errno_t error; fat_t *fat;
 fat = container_of(dir_node->i_super,fat_t,f_super);
 error = rwlock_write(&dir_node->i_data->i_dirlock);
 if (E_ISERR(error)) return error;
 if (INODE_ISDIR(file_node)) {
  /* Make sure that a directory is empty before removing it. */
  error = rwlock_write(&container_of(file_node,struct fatnode,f_inode)->f_idata.i_dirlock);
  if (E_ISERR(error)) goto end;
  error = fat_is_empty_directory(fat,container_of(file_node,struct fatnode,f_inode));
  if (E_ISOK(error))
      error = fat_rment_unlocked(dir_node,container_of(file_node,struct fatnode,f_inode));
  rwlock_endwrite(&container_of(file_node,struct fatnode,f_inode)->f_idata.i_dirlock);
 } else {
  error = fat_rment_unlocked(dir_node,container_of(file_node,struct fatnode,f_inode));
 }
end:
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 /* Delete cluster data for this file. */
 if (E_ISOK(error))
     fat_delall(fat,file_node->i_data->i_cluster);
 return error;
}
PRIVATE errno_t KCALL
fat16_root_remove(struct inode *__restrict dir_node,
                  struct dentry *__restrict /*UNUSED*/(file_path),
                  struct inode *__restrict file_node) {
#define FAT  container_of(dir_node,fat_t,f_super.sb_root)
 errno_t error;
 assert(INODE_ISSUPERBLOCK(dir_node));
 error = rwlock_write(&FAT->f_idata.i_dirlock);
 if (E_ISERR(error)) return error;
 if (INODE_ISDIR(file_node)) {
  /* Make sure that a directory is empty before removing it. */
  error = rwlock_write(&container_of(file_node,struct fatnode,f_inode)->f_idata.i_dirlock);
  if (E_ISERR(error)) goto end;
  error = fat_is_empty_directory(FAT,container_of(file_node,struct fatnode,f_inode));
  if (E_ISOK(error))
      error = fat16_root_rment_unlocked(dir_node,container_of(file_node,struct fatnode,f_inode));
  rwlock_endwrite(&container_of(file_node,struct fatnode,f_inode)->f_idata.i_dirlock);
 } else {
  error = fat16_root_rment_unlocked(dir_node,container_of(file_node,struct fatnode,f_inode));
 }
end:
 rwlock_endwrite(&FAT->f_idata.i_dirlock);
 /* Delete cluster data for this file. */
 if (E_ISOK(error))
     fat_delall(FAT,file_node->i_data->i_cluster);
 return error;
#undef FAT
}


PRIVATE errno_t KCALL
fatnode_truncate_for_open(struct fatnode *__restrict open_node,
                          struct iattr const *__restrict result_attr,
                          iattrset_t mode) {
 ssize_t error;
 fat_t *fat = container_of(open_node->f_inode.i_super,fat_t,f_super);
 assert(open_node->f_inode.i_data == &open_node->f_idata);
 /* Truncate the file. */
 FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] Truncate INode for open\n"));
 /* Try not to get interrupted while we do this. */
 error = rwlock_write(&open_node->f_inode.i_attr_lock);
 if (E_ISERR(error)) return error;
 task_nointr();
 error = fat_delall(fat,open_node->f_idata.i_cluster);
 if (E_ISERR(error)) goto err;
 assert(error != 0 || open_node->f_idata.i_cluster >= fat->f_cluster_eof);
 if (error != 0) {
  struct {
   le16        f_clusterhi;
   filemtime_t f_mtime;
   le16        f_clusterlo;
   le32        f_size;
  } buf;
  /* Mirror the deletion within the directory itself. */
  open_node->f_idata.i_cluster = fat->f_cluster_eof_marker;
  buf.f_clusterhi = BSWAP_H2LE16((u16)((u32)open_node->f_idata.i_cluster >> 16));
  buf.f_clusterlo = BSWAP_H2LE16((u16)open_node->f_idata.i_cluster);
  buf.f_size      = (le32)0;
  /* Also encode the file's modification time. */
  fat_mtime_encode(buf.f_mtime,mode&IATTR_MTIME
                   ? &result_attr->ia_mtime
                   : &open_node->f_inode.i_attr_disk.ia_mtime);
  /* Try to prevent us from being interrupted here. */
  HOSTMEMORY_BEGIN {
   error = blkdev_writeall(fat->f_super.sb_blkdev,
                           open_node->f_idata.i_pos.fp_headpos+
                           offsetof(file_t,f_clusterhi),
                           &buf,sizeof(buf));
  }
  HOSTMEMORY_END;
  if (E_ISOK(error)) {
   /* Mirror the truncation within the file itself. */
   open_node->f_inode.i_attr.ia_siz      = 0;
   open_node->f_inode.i_attr_disk.ia_siz = 0;
  }
 }
err:
 task_endnointr();
 rwlock_endwrite(&open_node->f_inode.i_attr_lock);
 return (errno_t)error;
}

PRIVATE REF struct inode *KCALL
fat_mkreg(struct inode *__restrict dir_node,
          struct dentry *__restrict path,
          struct iattr const *__restrict result_attr,
          iattrset_t mode) {
 errno_t temp; bool is_new; REF struct fatnode *result;
 result = E_PTR(rwlock_write(&dir_node->i_data->i_dirlock));
 if (E_ISERR(result)) return (struct inode *)result;
 result = fat_mkent_unlocked(dir_node,path,0,result_attr,mode,&is_new);
 if (E_ISOK(result)) {
  assertf(is_new || (mode&IATTR_EXISTS),
          "No new entry created when `IATTR_EXISTS' wasn't set.");
  if (!is_new && mode&IATTR_TRUNC) {
   temp = fatnode_truncate_for_open(result,result_attr,mode);
   if (E_ISERR(temp)) goto err;
  }
 }
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return &result->f_inode;
err: INODE_DECREF(&result->f_inode);
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return E_PTR(temp);
}
PRIVATE REF struct inode *KCALL
fat16_root_mkreg(struct inode *__restrict dir_node,
                 struct dentry *__restrict path,
                 struct iattr const *__restrict result_attr,
                 iattrset_t mode) {
 errno_t temp; bool is_new = false;
 REF struct fatnode *result;
 result = E_PTR(rwlock_write(&dir_node->i_data->i_dirlock));
 if (E_ISERR(result)) return (struct inode *)result;
 result = fat16_root_mkent_unlocked(dir_node,path,0,result_attr,mode,&is_new);
 if (E_ISOK(result)) {
  assertf(is_new || (mode&IATTR_EXISTS),
          "No new entry created when `IATTR_EXISTS' wasn't set.");
  if (!is_new && mode&IATTR_TRUNC) {
   temp = fatnode_truncate_for_open(result,result_attr,mode);
   if (E_ISERR(temp)) goto err;
  }
 }
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return &result->f_inode;
err: INODE_DECREF(&result->f_inode);
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return E_PTR(temp);
}

PRIVATE errno_t KCALL
fat_alloc_symlink(fat_t *__restrict fs,
                  struct fatnode *__restrict linknode,
                  USER char const *target_text) {
 size_t link_size; ssize_t temp;
 /* Figure out how large the link file will have to be. */
 char *taget_end = strend_user(target_text);
 struct filedata writer; u16 *link_text;
 u8 *src; u16 *dst;
 if unlikely(!taget_end) return -EFAULT;
 /* XXX: Impose some upper limit on link length? */
 link_size = (size_t)(taget_end-target_text);
 writer.fd_cluster = linknode->f_idata.i_cluster;
 assert(writer.fd_cluster == fs->f_cluster_eof_marker);
 writer.fd_cls_sel = 0;
 writer.fd_cls_act = (size_t)-1;
 writer.fd_begin   = FAT_SECTORADDR(fs,FAT_CLUSTERSTART(fs,writer.fd_cluster));
 writer.fd_max     = writer.fd_begin;
 writer.fd_end     = writer.fd_begin+fs->f_clustersize;
 writer.fd_pos     = writer.fd_begin;
 /* Write the symlink magic header. */
 link_text = (u16 *)amalloc((link_size+1)*2);
 if (copy_from_user(link_text,target_text,link_size)) { temp = -EFAULT; goto err2; }
 link_text[link_size] = 0;
 src = (u8 *)link_text+link_size;
 dst = (u16 *)link_text+link_size;
 while (dst != link_text) *--dst = *--src;
 HOSTMEMORY_BEGIN {
  /* Write the symlink magic header. */
  temp = filedata_write(&writer,fs,&linknode->f_inode,
                        symlnk_magic,sizeof(symlnk_magic));
  if (E_ISOK(temp)) {
   /* Write the symlink content. */
   temp = filedata_write(&writer,fs,&linknode->f_inode,
                         link_text,(link_size+1)*2);
  }
 }
 HOSTMEMORY_END;
 afree(link_text);
 if (E_ISERR(temp)) goto err;
 return -EOK;
err2:
 afree(link_text);
err:
 task_nointr();
 fat_delall(fs,linknode->f_idata.i_cluster);
 task_endnointr();
 return (errno_t)temp;
}

PRIVATE errno_t KCALL
fat_alloc_directory(fat_t *__restrict fs,
                    struct fatnode *__restrict dirnode,
                    struct inode *__restrict parnode) {
 file_t content[3]; ssize_t temp;
 struct filedata writer;
 STATIC_ASSERT(sizeof(content) == sizeof(fat_newdir_template));
 writer.fd_cluster = dirnode->f_idata.i_cluster;
 assert(writer.fd_cluster == fs->f_cluster_eof_marker);
 writer.fd_cls_sel = 0;
 writer.fd_cls_act = (size_t)-1;
 writer.fd_begin   = FAT_SECTORADDR(fs,FAT_CLUSTERSTART(fs,writer.fd_cluster));
 writer.fd_max     = writer.fd_begin;
 writer.fd_end     = writer.fd_begin+fs->f_clustersize;
 writer.fd_pos     = writer.fd_begin;
 memcpy(content,fat_newdir_template,sizeof(content));

 /* Directory self-reference. */
 content[0].f_clusterlo = BSWAP_H2LE16((u16)((u32)dirnode->f_idata.i_cluster >> 16));
 content[0].f_clusterhi = BSWAP_H2LE16((u16)dirnode->f_idata.i_cluster);
 fat_ctime_encode(content[0].f_ctime,&dirnode->f_inode.i_attr_disk.ia_ctime);
 fat_atime_encode(content[0].f_atime,&dirnode->f_inode.i_attr_disk.ia_atime);
 fat_mtime_encode(content[0].f_mtime,&dirnode->f_inode.i_attr_disk.ia_mtime);

 /* Parent directory reference. */
 content[1].f_clusterlo = BSWAP_H2LE16((u16)((u32)parnode->i_data->i_cluster >> 16));
 content[1].f_clusterhi = BSWAP_H2LE16((u16)parnode->i_data->i_cluster);
 content[1].f_ctime = content[0].f_ctime;
 content[1].f_atime = content[0].f_atime;
 content[1].f_mtime = content[0].f_mtime;

 /* Write the directory content. */
 HOSTMEMORY_BEGIN {
  temp = filedata_write(&writer,fs,&dirnode->f_inode,
                        content,sizeof(content));
 }
 HOSTMEMORY_END;
 if (E_ISERR(temp)) goto err;
 return -EOK;
err:
 task_nointr();
 fat_delall(fs,dirnode->f_idata.i_cluster);
 task_endnointr();
 return (errno_t)temp;
}

PRIVATE REF struct inode *KCALL
fat_symlink(struct inode *__restrict dir_node,
            struct dentry *__restrict target_ent,
            USER char const *target_text,
            struct iattr const *__restrict result_attr) {
 errno_t temp; bool is_new; REF struct fatnode *result;
 if unlikely(!support_cygwin_symlinks) return E_PTR(-EPERM);
 result = E_PTR(rwlock_write(&dir_node->i_data->i_dirlock));
 if (E_ISERR(result)) return (struct inode *)result;
 result = fat_mkent_unlocked(dir_node,target_ent,0,result_attr,0,&is_new);
 if (E_ISOK(result)) {
  assertf(is_new,"Not a new entry when one was required.");
  result->f_inode.i_attr.ia_mode &= ~__S_IFMT;
  result->f_inode.i_attr.ia_mode |= S_IFLNK;
  /* Allocate the symlink text. */
  temp = fat_alloc_symlink(container_of(dir_node->i_super,fat_t,f_super),
                           result,target_text);
  if (E_ISERR(temp)) goto err;
 }
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return &result->f_inode;
err:
 task_nointr();
 fat_rment_unlocked(dir_node,result);
 task_endnointr();
 INODE_DECREF(&result->f_inode);
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return E_PTR(temp);
}
PRIVATE REF struct inode *KCALL
fat16_root_symlink(struct inode *__restrict dir_node,
                   struct dentry *__restrict target_ent,
                   USER char const *target_text,
                   struct iattr const *__restrict result_attr) {
 errno_t temp; bool is_new = false; REF struct fatnode *result;
 if unlikely(!support_cygwin_symlinks) return E_PTR(-EPERM);
 result = E_PTR(rwlock_write(&dir_node->i_data->i_dirlock));
 if (E_ISERR(result)) return (struct inode *)result;
 result = fat16_root_mkent_unlocked(dir_node,target_ent,0,result_attr,0,&is_new);
 if (E_ISOK(result)) {
  assertf(is_new,"Not a new entry when one was required.");
  result->f_inode.i_attr.ia_mode &= ~__S_IFMT;
  result->f_inode.i_attr.ia_mode |= S_IFLNK;
  /* Allocate the symlink text. */
  temp = fat_alloc_symlink(container_of(dir_node->i_super,fat_t,f_super),
                           result,target_text);
  if (E_ISERR(temp)) goto err;
 }
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return &result->f_inode;
err:
 task_nointr();
 fat_rment_unlocked(dir_node,result);
 task_endnointr();
 INODE_DECREF(&result->f_inode);
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return E_PTR(temp);
}
PRIVATE REF struct inode *KCALL
fat_mkdir(struct inode *__restrict dir_node,
          struct dentry *__restrict target_ent,
          struct iattr const *__restrict result_attr) {
 errno_t temp; bool is_new; REF struct fatnode *result;
 if unlikely(!support_cygwin_symlinks) return E_PTR(-EPERM);
 result = E_PTR(rwlock_write(&dir_node->i_data->i_dirlock));
 if (E_ISERR(result)) return (struct inode *)result;
 result = fat_mkent_unlocked(dir_node,target_ent,ATTR_DIRECTORY,result_attr,0,&is_new);
 if (E_ISOK(result)) {
  assertf(is_new,"Not a new entry when one was required.");
  assert(INODE_ISDIR(&result->f_inode));
  /* Allocate the directory's contents. */
  temp = fat_alloc_directory(container_of(dir_node->i_super,fat_t,f_super),
                             result,dir_node);
  if (E_ISERR(temp)) goto err;
 }
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return &result->f_inode;
err:
 task_nointr();
 fat_rment_unlocked(dir_node,result);
 task_endnointr();
 INODE_DECREF(&result->f_inode);
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return E_PTR(temp);
}
PRIVATE REF struct inode *KCALL
fat16_root_mkdir(struct inode *__restrict dir_node,
                 struct dentry *__restrict target_ent,
                 struct iattr const *__restrict result_attr) {
 errno_t temp; bool is_new; REF struct fatnode *result;
 if unlikely(!support_cygwin_symlinks) return E_PTR(-EPERM);
 result = E_PTR(rwlock_write(&dir_node->i_data->i_dirlock));
 if (E_ISERR(result)) return (struct inode *)result;
 result = fat16_root_mkent_unlocked(dir_node,target_ent,ATTR_DIRECTORY,result_attr,0,&is_new);
 if (E_ISOK(result)) {
  assertf(is_new,"Not a new entry when one was required.");
  assert(INODE_ISDIR(&result->f_inode));
  /* Allocate the directory's contents. */
  temp = fat_alloc_directory(container_of(dir_node->i_super,fat_t,f_super),
                             result,dir_node);
  if (E_ISERR(temp)) goto err;
 }
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return &result->f_inode;
err:
 task_nointr();
 fat16_root_rment_unlocked(dir_node,result);
 task_endnointr();
 INODE_DECREF(&result->f_inode);
 rwlock_endwrite(&dir_node->i_data->i_dirlock);
 return E_PTR(temp);
}

PRIVATE errno_t KCALL
fat_fssync(struct superblock *__restrict sb) {
 return fat_synctable(container_of(sb,fat_t,f_super));
}
PRIVATE void KCALL
fat_fsfini(struct superblock *__restrict sb) {
 free(container_of(sb,fat_t,f_super)->f_fat_meta);
 free(container_of(sb,fat_t,f_super)->f_fat_table);
}
























PRIVATE errno_t KCALL
fat_loadtable_unlocked(fat_t *__restrict self,
                       sector_t fat_sector_index,
                       size_t n_sectors) {
 errno_t error;
 assert(rwlock_writing(&self->f_fat_lock));
 assertf(fat_sector_index+n_sectors <= self->f_sec4fat,
         "Out-of-bounds FAT sector index: %I32u+%I32u(%I32u) > %I32u",
         fat_sector_index,n_sectors,fat_sector_index+n_sectors,self->f_sec4fat);
 /* xxx: Validate redundant FAT copies? */
 HOSTMEMORY_BEGIN {
  error = blkdev_readall(self->f_super.sb_blkdev,
                         FAT_SECTORADDR(self,self->f_fat_start+fat_sector_index),
                        (void *)((uintptr_t)self->f_fat_table+(fat_sector_index*self->f_sectorsize)),
                        (size_t)(n_sectors*self->f_sectorsize));
 }
 HOSTMEMORY_END;
 return error;
}
PRIVATE errno_t KCALL
fat_savetable_unlocked(fat_t *__restrict self,
                       sector_t fat_sector_index,
                       size_t n_sectors) {
 errno_t error = -EOK; u8 n = self->f_fat_count;
 sector_t sector_start; void *sector_buffer; size_t sector_bytes;
 assert(rwlock_writing(&self->f_fat_lock));
 assertf(fat_sector_index+n_sectors <= self->f_sec4fat,
         "Out-of-bounds FAT sector index: %I32u+%I32u(%I32u) > %I32u",
         fat_sector_index,n_sectors,fat_sector_index+n_sectors,self->f_sec4fat);
 sector_buffer = (void *)((uintptr_t)self->f_fat_table+(fat_sector_index*self->f_sectorsize));
 sector_bytes  = (size_t)(n_sectors*self->f_sectorsize);
 sector_start  = self->f_fat_start+fat_sector_index;
 /* Write to all redundant FAT copies. */
 FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] Saving changed meta-sectors %I32u..%I32u of 0..%I32u (%I32u..%I32u)\n",
                  fat_sector_index,fat_sector_index+n_sectors-1,self->f_sec4fat-1,
                  sector_start,sector_start+n_sectors-1));
 HOSTMEMORY_BEGIN {
  while (n--) {
   error = blkdev_writeall(self->f_super.sb_blkdev,
                           FAT_SECTORADDR(self,sector_start+n*self->f_sec4fat),
                           sector_buffer,sector_bytes);
   if (E_ISERR(error)) break;
  }
 } HOSTMEMORY_END;
 return error;
}

PRIVATE errno_t KCALL
fat_synctable(fat_t *__restrict self) {
 errno_t error;
 sector_t changed_begin,changed_end;
 error = rwlock_read(&self->f_fat_lock);
 if (E_ISERR(error)) return error;
 /* Simple case: When nothing changed, we don't need to do anything! */
 if likely(!self->f_fat_changed) {
  rwlock_endread(&self->f_fat_lock);
  return -EOK;
 }
 error = rwlock_upgrade(&self->f_fat_lock);
 if (error == -ERELOAD) {
  COMPILER_READ_BARRIER();
  if unlikely(!self->f_fat_changed) {
   rwlock_endwrite(&self->f_fat_lock);
   return -EOK;
  }
  error = -EOK;
 }
 if (E_ISERR(error)) return error;
 /* Let's do this! */
 changed_begin = 0;
#if 0
 FAT_DEBUG(syslog(LOG_DEBUG,"FAT_META\n%.?[hex]\n",
                  CEILDIV(self->f_sec4fat,8/FAT_METABITS),
                  self->f_fat_meta));
#endif
 for (;;) {
  /* Search for chains for changed FAT entries and save them. */
  while (changed_begin != self->f_sec4fat &&
        !FAT_META_GTCHNG(self,changed_begin))
         ++changed_begin;
  changed_end = changed_begin;
  while (changed_end != self->f_sec4fat &&
         FAT_META_GTCHNG(self,changed_end))
         ++changed_end;
  if (changed_end == changed_begin) {
   assert(changed_begin == self->f_sec4fat);
   assert(changed_end   == self->f_sec4fat);
   break;
  }
  error = fat_savetable_unlocked(self,changed_begin,
                                (size_t)(changed_end-changed_begin));
  if (E_ISERR(error)) break;
  /* If changes have been saved successfully, delete all the change bits. */
  while (changed_begin != changed_end) {
   FAT_META_UTCHNG(self,changed_begin);
   ++changed_begin;
  }
 }
 self->f_fat_changed = false;
 rwlock_endwrite(&self->f_fat_lock);
 return error;
}


PRIVATE errno_t KCALL
fat_get(fat_t *__restrict self, fatid_t id,
        fatid_t *__restrict result) {
 sector_t table_sector; errno_t error;
 table_sector = FAT_TABLESECTOR(self,id);
 assertf(table_sector < self->f_sec4fat,
         "Out-of-bounds FAT sector index: %I32u >= %I32u",
         table_sector,self->f_sec4fat);
 error = rwlock_read(&self->f_fat_lock);
 if (E_ISERR(error)) return error;
 if (!FAT_META_GTLOAD(self,table_sector)) {
  /* Must load missing table sectors. */
  error = rwlock_upgrade(&self->f_fat_lock);
  if (error == -ERELOAD) {
   if (FAT_META_GTLOAD(self,table_sector)) { error = -EOK; goto is_loaded; }
  } else if (E_ISERR(error)) {
   return error;
  }
  error = fat_loadtable_unlocked(self,table_sector,1);
  if (E_ISOK(error)) {
is_loaded:
   FAT_META_STLOAD(self,table_sector);
   *result = FAT_TABLEGET(self,id);
  }
  rwlock_endwrite(&self->f_fat_lock);
#if 0
  FAT_DEBUG(syslog(LOG_DEBUG,"READ_CLUSTER (%I32u -> %I32u) %d (LOAD)\n",
                   id,*result,self->f_type));
#endif
  return error;
 }
 /* Now just read the FAT entry. */
 *result = FAT_TABLEGET(self,id);
#if 0
 FAT_DEBUG(syslog(LOG_DEBUG,"READ_CLUSTER (%I32u -> %I32u) %d\n",
                  id,*result,self->f_type));
#endif
 rwlock_endread(&self->f_fat_lock);
 return -EOK;
}
PRIVATE errno_t KCALL
fat_set(fat_t *__restrict self, fatid_t id, fatid_t value) {
 sector_t table_sector; errno_t error;
 table_sector = FAT_TABLESECTOR(self,id);
 assertf(table_sector < self->f_sec4fat,
         "Out-of-bounds FAT sector index: %I32u >= %I32u",
         table_sector,self->f_sec4fat);
 error = rwlock_write(&self->f_fat_lock);
 if (E_ISERR(error)) return error;
 if (!FAT_META_GTLOAD(self,table_sector)) {
  /* Must load missing table sectors. */
  error = fat_loadtable_unlocked(self,table_sector,1);
  if (E_ISERR(error)) goto end;
  FAT_META_STLOAD(self,table_sector);
 }
 FAT_DEBUG(syslog(LOG_FS|LOG_DEBUG,"[FAT] LINK(%I32u -> %I32u)\n",id,value));

 /* Now just read the FAT entry. */
 FAT_TABLESET(self,id,value);

 /* Mark the metadata associated with the sector as changed. */
 FAT_META_STCHNG(self,table_sector);
 self->f_fat_changed = true;
end:
 rwlock_endwrite(&self->f_fat_lock);
 return error;
}

#if 1
PRIVATE errno_t KCALL
fat_get_unlocked(fat_t *__restrict self, fatid_t id,
                 fatid_t *__restrict result) {
 sector_t table_sector; errno_t error;
 assert(rwlock_writing(&self->f_fat_lock));
 table_sector = FAT_TABLESECTOR(self,id);
 assertf(table_sector < self->f_sec4fat,
         "Out-of-bounds FAT sector index: %I32u >= %I32u",
         table_sector,self->f_sec4fat);
 if (!FAT_META_GTLOAD(self,table_sector)) {
  /* Must load missing table sectors. */
  error = fat_loadtable_unlocked(self,table_sector,1);
  if (E_ISERR(error)) return error;
  FAT_META_STLOAD(self,table_sector);
 }
 /* Now just read the FAT entry. */
 *result = FAT_TABLEGET(self,id);
 return -EOK;
}
PRIVATE errno_t KCALL
fat_set_unlocked(fat_t *__restrict self,
                 fatid_t id, fatid_t value) {
 sector_t table_sector; errno_t error;
 assert(rwlock_writing(&self->f_fat_lock));
 table_sector = FAT_TABLESECTOR(self,id);
 assertf(table_sector < self->f_sec4fat,
         "Out-of-bounds FAT sector index: %I32u >= %I32u",
         table_sector,self->f_sec4fat);
 if (!FAT_META_GTLOAD(self,table_sector)) {
  /* Must load missing table sectors. */
  error = fat_loadtable_unlocked(self,table_sector,1);
  if (E_ISERR(error)) return error;
  FAT_META_STLOAD(self,table_sector);
 }
 FAT_DEBUG(syslog(LOG_FS|LOG_DEBUG,"[FAT] LINK(%I32u -> %I32u)\n",id,value));

 /* Now just write the FAT entry. */
 FAT_TABLESET(self,id,value);

 /* Mark the metadata associated with the sector as changed. */
 FAT_META_STCHNG(self,table_sector);
 self->f_fat_changed = true;
 return -EOK;
}
#endif

PRIVATE errno_t KCALL
fat_get_unused_unlocked(fat_t *__restrict self,
                        fatid_t hint,
                        fatid_t *__restrict result) {
 fatid_t candidate,deref; errno_t temp;
 /* Starting at `hint', search for clusters marked as `FAT_CUSTER_UNUSED'
  * If we can't manage to find anything, wrap around and search lower half.
  * If that half is completely in use as well, return -ENOSPC. */
 for (candidate = hint;
      candidate < self->f_fat_length; ++candidate) {
  temp = fat_get_unlocked(self,candidate,&deref);
  if (E_ISERR(temp)) return temp;
  if (deref == FAT_CUSTER_UNUSED) {
gotit:
   /* Use this one! */
   *result = candidate;
   FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] GET_UNUSED_SECTOR(%I32u) -> %I32u (%I32u,%I32u)\n",
                    hint,candidate,hint,self->f_fat_length));
   return -EOK;
  }
 }
 for (candidate = 0;
      candidate < hint; ++candidate) {
  temp = fat_get_unlocked(self,candidate,&deref);
  if (E_ISERR(temp)) return temp;
  if (deref == FAT_CUSTER_UNUSED) {
   goto gotit;
  }
 }
 /* The disk is totally filled up... */
 return -ENOSPC;
}


PRIVATE ssize_t KCALL
fat_delall_unlocked(fat_t *__restrict self, fatid_t start) {
 ssize_t result = 0; errno_t temp;
 CHECK_HOST_DOBJ(self);
 assert(rwlock_writing(&self->f_fat_lock));
 while (start < self->f_cluster_eof) {
  fatid_t next;
  /* Read the next cluster in the chain. */
  temp = fat_get_unlocked(self,start,&next);
  FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] UNLINK(%I32u -> %I32u)\n",start,next));
  if (E_ISERR(temp)) return temp;
  /* Mark the cluster as unused. */
  temp = fat_set_unlocked(self,start,FAT_CUSTER_UNUSED);
  if (E_ISERR(temp)) return temp;
  ++result,start = next;
 }
 return result;
}
PRIVATE ssize_t KCALL
fat_delall(fat_t *__restrict self, fatid_t start) {
 ssize_t result;
 result = rwlock_write(&self->f_fat_lock);
 if (E_ISERR(result)) return result;
 result = fat_delall_unlocked(self,start);
 rwlock_endwrite(&self->f_fat_lock);
 return result;
}




PRIVATE void KCALL trimspecstring(char *__restrict buf, size_t size) {
 while (size && FAT_ISSPACE(*buf)) { memmove(buf,buf+1,--size); buf[size] = '\0'; }
 while (size && FAT_ISSPACE(buf[size-1])) buf[--size] = '\0';
}




/* Create a FAT filesystem from the given block device.
 * @return: * :         A reference to a new FAT superblock.
 * @return: -EINVAL:    The given `blkdev' doesn't contain a FAT filesystem.
 * @return: -ENOMEM:    Not enough available kernel memory.
 * @return: E_ISERR(*): Failed to create a FAT superblock for some reason. */
PRIVATE REF struct superblock *KCALL
fat_mksuper(struct blkdev *__restrict dev, u32 UNUSED(flags),
            USER char const *UNUSED(devname),
            USER void *UNUSED(data), void *UNUSED(closure)) {
 fat_t *result; fat_header_t header;
 errno_t error; fattype_t type;
 CHECK_HOST_DOBJ(dev);
#define ERROR(e) return E_PTR(e)
 memset(&header,0,sizeof(header));
 HOSTMEMORY_BEGIN {
  error = blkdev_readall(dev,0,&header,sizeof(header));
 }
 HOSTMEMORY_END;
 if (E_ISERR(error)) ERROR(error);
 /* Validate the boot signature. */
 if unlikely(header.fat32.f32_bootsig[0] != 0x55 ||
             header.fat32.f32_bootsig[1] != 0xAA)
    ERROR(-EINVAL);
 result = (fat_t *)superblock_new(sizeof(fat_t));
 if unlikely(!result) ERROR(-ENOMEM);
#undef ERROR
#define ERROR(e) { error = (e); goto err; }

 /* Check some other clear identifiers for an invalid FAT header. */
 result->f_sectorsize  = BSWAP_LE2H16(header.bpb.bpb_bytes_per_sector);
 if unlikely(result->f_sectorsize != 512  && result->f_sectorsize != 1024 &&
             result->f_sectorsize != 2048 && result->f_sectorsize != 4096)
             ERROR(-EINVAL);
 if unlikely(!header.bpb.bpb_sectors_per_cluster) ERROR(-EINVAL);
 if unlikely(!header.bpb.bpb_reserved_sectors) ERROR(-EINVAL); /* What's the first sector, then? */
 if unlikely((result->f_sectorsize % sizeof(file_t)) != 0) ERROR(-EINVAL);

 /* Extract some common information. */
 result->f_fat_start   = (sector_t)BSWAP_LE2H16(header.bpb.bpb_reserved_sectors);
 result->f_sec4clus    = (size_t)header.bpb.bpb_sectors_per_cluster;
 result->f_fat_count   = (u32)header.bpb.bpb_fatc;
 result->f_clustersize = (size_t)(result->f_sec4clus*result->f_sectorsize);

 /* Figure out what kind of FAT filesystem this is. */
 if (!header.bpb.bpb_sectors_per_fat ||
     !header.bpb.bpb_maxrootsize) {
  result->f_dat_start = result->f_fat_start+(BSWAP_LE2H32(header.fat32.f32_sectors_per_fat)*
                                             header.bpb.bpb_fatc);
  type                = FAT32;
 } else {
  u32 fat_size,root_sectors;
  u32 data_sectors,total_clusters;
  root_sectors = CEILDIV(BSWAP_LE2H16(header.bpb.bpb_maxrootsize)*
                         sizeof(file_t),result->f_sectorsize);
  fat_size = (header.bpb.bpb_fatc*BSWAP_LE2H16(header.bpb.bpb_sectors_per_fat));
  result->f_dat_start = BSWAP_LE2H16(header.bpb.bpb_reserved_sectors);
  result->f_dat_start += fat_size;
  result->f_dat_start += root_sectors;
  /* Calculate the total number of data sectors. */
  if (header.bpb.bpb_shortsectorc) {
   data_sectors = (u32)BSWAP_LE2H16(header.bpb.bpb_shortsectorc);
  } else {
   data_sectors = BSWAP_LE2H32(header.bpb.bpb_longsectorc);
  }
  data_sectors -= BSWAP_LE2H16(header.bpb.bpb_reserved_sectors);
  data_sectors -= fat_size;
  data_sectors -= root_sectors;
  /* Calculate the total number of data clusters. */
  total_clusters = data_sectors/header.bpb.bpb_sectors_per_cluster;
       if (total_clusters > FAT16_MAXCLUSTERS) type = FAT32;
  else if (total_clusters > FAT12_MAXCLUSTERS) type = FAT16;
  else                                         type = FAT12;
 }

 result->f_type = type;
 if (type == FAT32) {
  if (header.fat32.f32_signature != 0x28 &&
      header.fat32.f32_signature != 0x29)
      ERROR(-EINVAL);
  memcpy(result->f_label,header.fat32.f32_label,sizeof(header.fat32.f32_label));
  memcpy(result->f_sysname,header.fat32.f32_sysname,sizeof(header.fat32.f32_sysname));
  result->f_sec4fat            = BSWAP_LE2H32(header.fat32.f32_sectors_per_fat);
  result->f_cluster_eof        = (result->f_sec4fat*result->f_sectorsize)/4;
  result->f_cluster_eof_marker = 0xffffffff;
  /* Must lookup the cluster of the root directory. */
  result->f_idata.i_cluster    = BSWAP_LE2H32(header.fat32.f32_root_cluster);
  result->f_fat_get            = &fat_get32;
  result->f_fat_set            = &fat_set32;
  result->f_fat_sector         = &fat_sec32;
 } else {
#if FAT_CUSTER_FAT16_ROOT != 0
  result->f_idata.i_cluster = FAT_CUSTER_FAT16_ROOT;
#else
  assert(result->f_idata.i_cluster == FAT_CUSTER_FAT16_ROOT);
#endif
  if (type == FAT12) {
   result->f_cluster_eof_marker = 0xfff;
   result->f_fat_get            = &fat_get12;
   result->f_fat_set            = &fat_set12;
   result->f_fat_sector         = &fat_sec12;
  } else {
   result->f_cluster_eof_marker = 0xffff;
   result->f_fat_get            = &fat_get16;
   result->f_fat_set            = &fat_set16;
   result->f_fat_sector         = &fat_sec16;
  }
  if (header.fat16.f16_signature != 0x28 &&
      header.fat16.f16_signature != 0x29)
      ERROR(-EINVAL);
  memcpy(result->f_label,header.fat16.f16_label,sizeof(header.fat16.f16_label));
  memcpy(result->f_sysname,header.fat16.f16_sysname,sizeof(header.fat16.f16_sysname));
  result->f_idata.i_fat16_root  = BSWAP_LE2H16(header.bpb.bpb_reserved_sectors);
  result->f_idata.i_fat16_root += (header.bpb.bpb_fatc*BSWAP_LE2H16(header.bpb.bpb_sectors_per_fat));
  result->f_sec4fat             = BSWAP_LE2H16(header.bpb.bpb_sectors_per_fat);
  result->f_fat16_rootmax       = (__u32)BSWAP_LE2H16(header.bpb.bpb_maxrootsize);
  result->f_cluster_eof         = (result->f_sec4fat*result->f_sectorsize)/2;
 }

 if (result->f_cluster_eof_marker < result->f_cluster_eof)
     result->f_cluster_eof_marker = result->f_cluster_eof;


 memcpy(&result->f_oem,header.bpb.bpb_oem,8*sizeof(char));
 result->f_fat_size = result->f_sec4fat*result->f_sectorsize;
 trimspecstring(result->f_oem,8);
 trimspecstring(result->f_label,11);
 trimspecstring(result->f_sysname,8);

 if (result->f_type == FAT32) {
  result->f_super.sb_root.i_ops = &fatops_root_32;
 } else {
  result->f_super.sb_root.i_ops = &fatops_root_16;
 }
 rwlock_cinit(&result->f_idata.i_dirlock);
 result->f_super.sb_root.i_data         = &result->f_idata;
 result->f_super.sb_root.__i_nlink      = 0;
 result->f_super.sb_root.i_attr.ia_mode = S_IFDIR|0777;
 result->f_super.sb_root.i_attr_disk    = result->f_super.sb_root.i_attr;
 result->f_super.sb_root.i_state        = 0;
 result->f_super.sb_ops                 = &fatops_super;
 result->f_mode                         = 0777;

 result->f_fat_table = malloc(result->f_fat_size);
 if unlikely(!result->f_fat_table) ERROR(-ENOMEM);
 result->f_fat_meta = (byte_t *)kmalloc(CEILDIV(result->f_sec4fat,8/FAT_METABITS),
                                        GFP_SHARED|GFP_CALLOC);
 if unlikely(!result->f_fat_meta) { free(result->f_fat_table); ERROR(-ENOMEM); }

 /* Register the block-device with the superblock. */
 result->f_super.sb_blkdev = dev;
 BLKDEV_INCREF(dev);

 /* NOTE: The kernel's own `THIS_INSTANCE' must never get unloaded! */
 asserte(E_ISOK(superblock_setup(&result->f_super,THIS_INSTANCE)));

 return &result->f_super;
err: free(result);
 return E_PTR(error);
#undef ERROR
}

#define FAT_SEC12(self,id)  (((id)+((id)/2))/(self)->f_sectorsize)
#define FAT_SEC16(self,id)  (((id)*2)/(self)->f_sectorsize)
#define FAT_SEC32(self,id)  (((id)*4)/(self)->f_sectorsize)

PRIVATE sector_t KCALL fat_sec12(fat_t const *__restrict self, fatid_t id) { return FAT_SEC12(self,id); }
PRIVATE sector_t KCALL fat_sec16(fat_t const *__restrict self, fatid_t id) { return FAT_SEC16(self,id); }
PRIVATE sector_t KCALL fat_sec32(fat_t const *__restrict self, fatid_t id) { return FAT_SEC32(self,id); }

PRIVATE fatid_t KCALL
fat_get12(fat_t const *__restrict self, fatid_t id) {
 u16 val;
 assertf(id < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         id,self->f_fat_length);
 val = (*(u16 *)((uintptr_t)self->f_fat_table+(id+(id/2))));
 if (id&1) val >>= 4; else val &= 0x0fff;
 return val;
}
PRIVATE void KCALL
fat_set12(fat_t *__restrict self, fatid_t id, fatid_t value) {
 u16 *pval;
 assertf(id < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         id,self->f_fat_length);
 pval = ((u16 *)((uintptr_t)self->f_fat_table+(id+(id/2))));
 if (id&1) *pval  = (*pval&0xf)|(value << 4);
 else      *pval |= value&0xfff;
}



PRIVATE fatid_t KCALL
fat_get16(fat_t const *__restrict self, fatid_t id) {
 assertf(id < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         id,self->f_fat_length);
 return ((u16 *)self->f_fat_table)[id];
}
PRIVATE void KCALL
fat_set16(fat_t *__restrict self, fatid_t id, fatid_t value) {
 assertf(id < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         id,self->f_fat_length);
 ((u16 *)self->f_fat_table)[id] = (u16)value;
}



PRIVATE fatid_t KCALL
fat_get32(fat_t const *__restrict self, fatid_t id) {
 assertf(id < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         id,self->f_fat_length);
 return ((u32 *)self->f_fat_table)[id];
}
PRIVATE void KCALL
fat_set32(fat_t *__restrict self, fatid_t id, fatid_t value) {
 assertf(id < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         id,self->f_fat_length);
 ((u32 *)self->f_fat_table)[id] = value;
}




/* FAT system hooks. */
PRIVATE struct fstype fat_fshooks[] = {
#define HOOK(name,id)   {{NULL},THIS_INSTANCE,id,&fat_mksuper,FSTYPE_NORMAL|FSTYPE_HIDDEN,NULL,name}
#define HOOK_V(name,id) {{NULL},THIS_INSTANCE,id,&fat_mksuper,FSTYPE_NORMAL,NULL,name}
 HOOK("fat-12",BLKSYS_FAT12),
 HOOK("fat-16",BLKSYS_FAT16ALT),
 HOOK("fat-16",BLKSYS_FAT16),
 HOOK("fat-32",BLKSYS_FAT32),
 HOOK("fat-32-lba",BLKSYS_FAT32_LBA),
 HOOK("fat-16-lba",BLKSYS_FAT16_LBA),
 HOOK("fat-12",BLKSYS_FAT12_HIDDEN),
 HOOK("fat-16-32m",BLKSYS_FAT16_32M_HIDDEN),
 HOOK("fat-16",BLKSYS_FAT16_HIDDEN),
 HOOK("fat-32",BLKSYS_FAT32_HIDDEN),
 HOOK("fat-32-lba",BLKSYS_FAT32_LBA_HIDDEN),
 HOOK("fat-16-lba",BLKSYS_FAT16_LBA_HIDDEN),
 /* Only mark the generic FAT filesystem type as visible. - Don't clobber `/proc/filesystems' */
 HOOK_V("fat",BLKSYS_MICROSOFT_BASIC_DATA),
#if 1 /* Given how generic it is, try to use FAT as a default-loader. */
 HOOK("fat",BLKSYS_ANY),
#endif
#undef HOOK_V
#undef HOOK
};

PRIVATE MODULE_INIT void KCALL fat_init(void) {
 struct fstype *iter;
 for (iter = fat_fshooks;
      iter != COMPILER_ENDOF(fat_fshooks); ++iter)
      fs_addtype(iter);
}

#ifndef CONFIG_NO_MODULE_CLEANUP
PRIVATE MODULE_FINI void KCALL fat_fini(void) {
 struct fstype *iter = COMPILER_ENDOF(fat_fshooks);
 while (iter-- != fat_fshooks) fs_deltype(iter);
}
#endif

DECL_END

#ifndef __INTELLISENSE__
#define FAT16_ROOT
#include "fat-readdir.c.inl"
#include "fat-readdir.c.inl"
#endif

#endif /* !GUARD_KERNEL_CORE_MODULES_FS_FAT_C */
