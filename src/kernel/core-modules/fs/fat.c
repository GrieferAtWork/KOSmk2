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

#if 0
#   define FAT_DEBUG(x) x
#else
#   define FAT_DEBUG(x) (void)0
#endif

STATIC_ASSERT(sizeof(fat_header_t) == 512);

/* Load/Save the given fat-table-sector (that is: The sector associated with the FAT table itself) */
PRIVATE errno_t KCALL fat_loadtable_unlocked(fat_t *__restrict self, sector_t fat_sector_index, size_t n_sectors);
PRIVATE errno_t KCALL fat_savetable_unlocked(fat_t *__restrict self, sector_t fat_sector_index, size_t n_sectors);

/* Flush all changes to the FAT table. */
PRIVATE errno_t KCALL fat_flushtable(fat_t *__restrict self);

/* High-level get/set fat entries, while ensuring that they are automatically loaded/marked as dirty. */
PRIVATE errno_t KCALL fat_get(fat_t *__restrict self, fatid_t id, fatid_t *__restrict result);
PRIVATE ATTR_UNUSED errno_t KCALL fat_set(fat_t *__restrict self, fatid_t id, fatid_t value);
#if 0
PRIVATE errno_t KCALL fat_get_unlocked(fat_t *__restrict self, fatid_t id, fatid_t *__restrict result);
PRIVATE errno_t KCALL fat_set_unlocked(fat_t *__restrict self, fatid_t id, fatid_t value);
#endif

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
#define fat_atime_encode(t,val)   filedate_encodetime(&(t),(val)->tv_sec)
#define fat_mtime_encode(t,val)  (filedate_encodetime(&(t).fc_date,(val)->tv_sec), \
                                  filetime_encodetime(&(t).fc_time,(val)->tv_sec))
#define fat_ctime_encode(t,val)  (filedate_encodetime(&(t).fc_date,(val)->tv_sec), \
                                  filetime_encodetime(&(t).fc_time,(val)->tv_sec), \
                                 (t).fc_sectenth = (u8)(((val)->tv_nsec % 1000000000l)/(1000000000l/200l)))

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
PRIVATE ssize_t KCALL fat_fread(struct file *__restrict fp, USER void *buf, size_t bufsize);
PRIVATE ssize_t KCALL fat_fwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize);
PRIVATE off_t   KCALL fat_fseek(struct file *__restrict fp, off_t off, int whence);
PRIVATE ssize_t KCALL fat_fpread(struct file *__restrict fp, USER void *buf, size_t bufsize, pos_t pos);
PRIVATE ssize_t KCALL fat_fpwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize, pos_t pos);
PRIVATE ssize_t KCALL fat_freaddir(struct file *__restrict fp, USER struct dirent *buf, size_t bufsize, rdmode_t mode);
PRIVATE errno_t KCALL fat_fflush(struct file *__restrict fp);
PRIVATE REF struct inode *KCALL fat_lookup(struct inode *__restrict dir_node, struct dentry *__restrict result_path);
PRIVATE REF struct file *KCALL fat16_root_fopen(struct inode *__restrict ino, struct dentry *__restrict node_ent, oflag_t oflags);
PRIVATE ssize_t KCALL fat16_root_fread(struct file *__restrict fp, USER void *buf, size_t bufsize);
PRIVATE ssize_t KCALL fat16_root_fwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize);
PRIVATE off_t   KCALL fat16_root_fseek(struct file *__restrict fp, off_t off, int whence);
PRIVATE ssize_t KCALL fat16_root_fpread(struct file *__restrict fp, USER void *buf, size_t bufsize, pos_t pos);
PRIVATE ssize_t KCALL fat16_root_fpwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize, pos_t pos);
PRIVATE REF struct inode *KCALL fat16_root_lookup(struct inode *__restrict dir_node, struct dentry *__restrict result_path);
PRIVATE ssize_t KCALL fat16_root_freaddir(struct file *__restrict fp, USER struct dirent *buf, size_t bufsize, rdmode_t mode);
PRIVATE errno_t KCALL fat_setattr(struct inode *__restrict ino, iattrset_t changed);
PRIVATE errno_t KCALL fat_fssync(struct superblock *__restrict sb);
PRIVATE void    KCALL fat_fsfini(struct superblock *__restrict sb);

/* Load the correct cluster within the given file when
 * NOTE: In addition, also handles the special case of an empty file
 *       becoming non-empty, when 'fd_cls_act == 0' and 'fd_cluster' is EOF.
 * @return: 0:          The cluster doesn't exist (the selected position is located past the file's end)
 * @return: 1:          Successfully loaded the cluster.
 * @return: E_ISERR(*): Failed to load the cluster for some reason. */
PRIVATE errno_t KCALL filedata_load(struct filedata *__restrict self, fat_t *__restrict fs, struct inode *__restrict node);

/* Similar to 'filedata_load', but allocate missing chunks  */
PRIVATE ATTR_UNUSED errno_t KCALL filedata_alloc(struct filedata *__restrict self, fat_t *__restrict fs, struct inode *__restrict node);

/* Perform high-level reading/writing, starting at the file position specified within 'self' */
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
                               *  Vector of long name entries (sorted by 'ne_namepos'). */
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

/* Search for a directory entry 'name' within the given memory region.
 * NOTE: Multiple different vectors may be passed, so long as
 *       their ordering is correct and the same 'lookupdata'
 *       is passed every time.
 * @return: NULL:  The given 'name' wasn't found (yet?)
 * @return: !NULL: An E_PTR()-errorcode, or the requested INode. */
PRIVATE REF struct inode *KCALL
fat_lookup_memory(struct lookupdata *__restrict lookupdata,
                  struct dentryname const *__restrict name,
                  file_t const *__restrict filev, size_t filec,
                  pos_t filev_pos, cluster_t cluster_id,
                  fat_t *__restrict fatfs);

/* Trim whitespace at the front and back of 'buf'. */
PRIVATE void KCALL trimspecstring(char *__restrict buf, size_t size);


/* Support cygwin-style symbolic links as an extension, thus kind-of
 * cheating the system a bit by getting symlinks without a filesystem
 * actually designed to support them... */
PRIVATE bool support_cygwin_symlinks = 1;
DEFINE_EARLY_SETUP_VAR("fat-cygwin-symlinks",support_cygwin_symlinks);
PRIVATE byte_t const symlnk_magic[] = {'!','<','s','y','m','l','i','n','k','>',0xff,0xfe};

/* Check if the given FAT-inode is a cygwin-style symbolic link. */
PRIVATE bool KCALL fat_cluster_is_symlnk(fat_t *__restrict fat, cluster_t id);
PRIVATE ssize_t KCALL fat_readlink(struct inode *__restrict ino, USER char *__restrict buf, size_t bufsize);











/* Fat INode/File operation descriptors. */
PRIVATE struct inodeops const fatops_reg = {
    .f_read       = &fat_fread,
    .f_write      = &fat_fwrite,
    .f_pread      = &fat_fpread,
    .f_pwrite     = &fat_fpwrite,
    .f_seek       = &fat_fseek,
    .f_flush      = &fat_fflush,
    .ino_fopen    = &fat_fopen,
    .ino_setattr  = &fat_setattr,
    .ino_readlink = &fat_readlink,
};
PRIVATE struct inodeops const fatops_dir = {
    .f_read      = &fat_fread,
    .f_write     = &fat_fwrite,
    .f_pread     = &fat_fpread,
    .f_pwrite    = &fat_fpwrite,
    .f_seek      = &fat_fseek,
    .f_readdir   = &fat_freaddir,
    .f_flush     = &fat_fflush,
    .ino_fopen   = &fat_fopen,
    .ino_setattr = &fat_setattr,
    .ino_lookup  = &fat_lookup,
};
PRIVATE struct inodeops const fatops_root_16 = {
    .f_read     = &fat16_root_fread,
    .f_write    = &fat16_root_fwrite,
    .f_pread    = &fat16_root_fpread,
    .f_pwrite   = &fat16_root_fpwrite,
    .f_seek     = &fat16_root_fseek,
    .f_readdir  = &fat16_root_freaddir,
    .f_flush    = &fat_fflush,
    .ino_fopen  = &fat16_root_fopen,
    .ino_lookup = &fat16_root_lookup,
};
PRIVATE struct inodeops const fatops_root_32 = {
    .f_read     = &fat_fread,
    .f_write    = &fat_fwrite,
    .f_pread    = &fat_fpread,
    .f_pwrite   = &fat_fpwrite,
    .f_seek     = &fat_fseek,
    .f_readdir  = &fat_freaddir,
    .f_flush    = &fat_fflush,
    .ino_fopen  = &fat_fopen,
    .ino_lookup = &fat_lookup,
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
  * HINT: Checking this here also allows us to skip re-checking 'bufsize' below. */
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
 /* Make sure to exclude the terminating \0-character. */
 if (read_total) --read_total;
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
 prio = entry->f_marker;
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
                  struct dentryname const *__restrict name,
                  file_t const *__restrict filev, size_t filec,
                  pos_t filev_pos, cluster_t cluster_id,
                  fat_t *__restrict fatfs) {
 file_t const *iter,*end;
 REF struct fatnode *result;
 end = (iter = filev)+filec;
 for (; iter != end; ++iter) {
  if (iter->f_marker == MARKER_DIREND) return E_PTR(-ENOENT);
  if (iter->f_marker == MARKER_UNUSED) continue;
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
   lookupdata_clrlfn(data);
  }
  if (name->dn_size <= FAT_NAMEMAX+1+FAT_EXTMAX) {
   /* Check against regular FAT entry. */
   char *filenameiter,filename[FAT_NAMEMAX+1+FAT_EXTMAX];
   memcpy(filename,iter->f_name,FAT_NAMEMAX*sizeof(char));
   filenameiter = filename+FAT_NAMEMAX;
   while (filenameiter != filename && isspace(filenameiter[-1])) --filenameiter;
   if (iter->f_ntflags&NTFLAG_LOWBASE) {
    char *tempiter;
    for (tempiter = filename; tempiter != filenameiter;
         ++tempiter) *tempiter = tolower(*tempiter);
   }
   *filenameiter++ = '.';
   memcpy(filenameiter,iter->f_ext,FAT_EXTMAX*sizeof(char));
   filenameiter += FAT_EXTMAX;
   while (filenameiter != filename && isspace(filenameiter[-1])) --filenameiter;
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
   if ((size_t)(filenameiter-filename) == name->dn_size &&
       !memcmp(filename,name->dn_name,name->dn_size)) {
    data->ld_fpos.fp_namecls = cluster_id;
    data->ld_fpos.fp_namepos = filev_pos+((uintptr_t)iter-(uintptr_t)filev);
found_entry:
    /* GOTI! */
    result = (struct fatnode *)inode_new(sizeof(struct fatnode));
    if unlikely(!result) result = E_PTR(-ENOMEM);
    else {
     /* Fill in INode information about the directory entry. */
     result->f_idata.i_cluster = (BSWAP_LE2H16(iter->f_clusterhi) << 16 |
                                  BSWAP_LE2H16(iter->f_clusterlo));
     result->f_idata.i_pos     = data->ld_fpos;
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
      result->f_inode.i_ops           = &fatops_reg;
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
 self->fd_year = year-1980;
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
/* Load the correct cluster within the given file when
 * NOTE: In addition, also handles the special case of an empty file
 *       becoming non-empty, when 'fd_cls_act == 0' and 'fd_cluster' is EOF.
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
  FAT_DEBUG(syslog(LOG_DEBUG,"[FAT] Sector jump: %Iu -> %Iu\n",
                    self->fd_cls_act,self->fd_cls_sel));
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
   if likely(file_size <= file_pos) return 0;
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
  n_ahead = (size_t)(self->fd_cls_sel-
                     self->fd_cls_act);
  self->fd_cls_act = self->fd_cls_sel;
  assert(n_ahead);
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
  if (!max_read) break;
  if (max_read > bufsize)
      max_read = bufsize;
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
               fat_t *__restrict fs, struct inode *__restrict node) {
 return -ENOSYS; /* TODO */
}
PRIVATE ssize_t KCALL
filedata_write(struct filedata *__restrict self,
               fat_t *__restrict fs, struct inode *__restrict node,
               USER void const *buf, size_t bufsize) {
 return -ENOSYS; /* TODO */
}


PRIVATE ssize_t KCALL
fat_fread(struct file *__restrict fp,
          USER void *buf, size_t bufsize) {
 return filedata_read(&FILE->f_data,FILE->f_fs,fp->f_node,buf,bufsize);
}
PRIVATE ssize_t KCALL
fat_fwrite(struct file *__restrict fp,
           USER void const *buf, size_t bufsize) {
 return filedata_write(&FILE->f_data,FILE->f_fs,fp->f_node,buf,bufsize);
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
 struct filedata data;
 errno_t error = rwlock_read(&FILE->f_file.f_lock);
 if (E_ISERR(error)) return error;
 memcpy(&data,&FILE->f_data,sizeof(struct filedata));
 rwlock_endread(&FILE->f_file.f_lock);
 FATFILE_FSEEK_SET(&data,FILE->f_fs,pos);
 //data.fd_cls_act = (size_t)-1;
 return filedata_read(&data,FILE->f_fs,fp->f_node,buf,bufsize);
}
PRIVATE ssize_t KCALL
fat_fpwrite(struct file *__restrict fp,
            USER void const *buf, size_t bufsize,
            pos_t pos) {
 struct filedata data;
 errno_t error = rwlock_read(&FILE->f_file.f_lock);
 if (E_ISERR(error)) return error;
 memcpy(&data,&FILE->f_data,sizeof(struct filedata));
 rwlock_endread(&FILE->f_file.f_lock);
 FATFILE_FSEEK_SET(&data,FILE->f_fs,pos);
 return filedata_write(&data,FILE->f_fs,fp->f_node,buf,bufsize);
}
#undef FILE


#define FILE ((struct fatfile_root16 *)fp)
PRIVATE REF struct file *KCALL
fat16_root_fopen(struct inode *__restrict ino,
                 struct dentry *__restrict node_ent,
                 oflag_t oflags) {
#define FAT   ((fat_t *)ino)
 struct fatfile_root16 *result;
 assert(INODE_ISSUPERBLOCK(ino));
 result = (struct fatfile_root16 *)file_new(sizeof(struct fatfile_root16));
 if unlikely(!result) return E_PTR(-ENOMEM);
 /* Setup the start/end pointers according to FAT specifications. */
 result->f_begin = FAT_SECTORADDR(FAT,FAT->f_idata.i_fat16_root);
 result->f_pos   = result->f_begin;
 result->f_end   = result->f_begin+FAT->f_fat16_rootmax*sizeof(file_t);
 /* Setup the file itself. */
 file_setup((struct file *)result,ino,node_ent,oflags);
 return (struct file *)result;
#undef FAT
}
PRIVATE ssize_t KCALL
fat16_root_fread(struct file *__restrict fp,
                 USER void *buf, size_t bufsize) {
 pos_t max_read; ssize_t result;
 assert(FILE->f_begin <= FILE->f_end);
 assert(FILE->f_pos >= FILE->f_begin);
 assert(FILE->f_pos <= FILE->f_end);
 assert(INODE_ISSUPERBLOCK(fp->f_node));
 max_read = FILE->f_end-FILE->f_pos;
 if (max_read > bufsize)
     max_read = bufsize;
 result = blkdev_read(INODE_TOSUPERBLOCK(fp->f_node)->sb_blkdev,
                      FILE->f_pos,buf,(size_t)max_read);
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
 pos_t max_read; ssize_t result;
 assert(FILE->f_begin <= FILE->f_end);
 assert(FILE->f_pos >= FILE->f_begin);
 assert(FILE->f_pos <= FILE->f_end);
 assert(INODE_ISSUPERBLOCK(fp->f_node));
 max_read = FILE->f_end-FILE->f_pos;
 if (max_read > bufsize)
     max_read = bufsize;
 result = blkdev_write(INODE_TOSUPERBLOCK(fp->f_node)->sb_blkdev,
                       FILE->f_pos,buf,(size_t)max_read);
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
 assert(FILE->f_begin <= FILE->f_end);
 assert(INODE_ISSUPERBLOCK(fp->f_node));
 start_offset = FILE->f_begin+pos;
 if (start_offset < FILE->f_begin ||
     start_offset >= FILE->f_end)
     return 0;
 max_read = (pos_t)(FILE->f_end-start_offset);
 if (max_read > bufsize)
     max_read = bufsize;
 return blkdev_read(INODE_TOSUPERBLOCK(fp->f_node)->sb_blkdev,
                    start_offset,buf,bufsize);
}
PRIVATE ssize_t KCALL
fat16_root_fpwrite(struct file *__restrict fp,
                   USER void const *buf, size_t bufsize,
                   pos_t pos) {
 pos_t start_offset,max_read;
 assert(FILE->f_begin <= FILE->f_end);
 assert(INODE_ISSUPERBLOCK(fp->f_node));
 start_offset = FILE->f_begin+pos;
 if (start_offset < FILE->f_begin ||
     start_offset >= FILE->f_end)
     return 0;
 max_read = (pos_t)(FILE->f_end-start_offset);
 if (max_read > bufsize)
     max_read = bufsize;
 return blkdev_write(INODE_TOSUPERBLOCK(fp->f_node)->sb_blkdev,
                     start_offset,buf,bufsize);
}
#undef FILE

PRIVATE errno_t KCALL
fat_fflush(struct file *__restrict fp) {
 CHECK_HOST_DOBJ(fp);
 CHECK_HOST_DOBJ(fp->f_node);
 CHECK_HOST_DOBJ(fp->f_node->i_super);
 CHECK_HOST_DOBJ(fp->f_node->i_super->sb_blkdev);
 return blkdev_flush(fp->f_node->i_super->sb_blkdev);
}

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
   error = blkdev_writeall(ino->i_super->sb_blkdev,
                           NODE->f_idata.i_pos.fp_headpos+
                           offsetof(file_t,f_ctime),
                           &buf,sizeof(buf));
  } else if (changed&IATTR_ATIME) {
   /* Access time changed. */
   fileatime_t atime;
   fat_atime_encode(atime,&ino->i_attr.ia_atime);
   error = blkdev_writeall(ino->i_super->sb_blkdev,
                           NODE->f_idata.i_pos.fp_headpos+
                           offsetof(file_t,f_atime),
                           &atime,sizeof(atime));
  } else {
   /* Creation time changed. */
   filectime_t ctime;
   fat_ctime_encode(ctime,&ino->i_attr.ia_atime);
   error = blkdev_writeall(ino->i_super->sb_blkdev,
                           NODE->f_idata.i_pos.fp_headpos+
                           offsetof(file_t,f_ctime),
                           &ctime,sizeof(ctime));
  }
  if (E_ISERR(error)) return error;
 }
 if (changed&IATTR_MTIME) {
  filemtime_t mtime;
  fat_mtime_encode(mtime,&ino->i_attr.ia_mtime);
  error = blkdev_writeall(ino->i_super->sb_blkdev,
                          NODE->f_idata.i_pos.fp_headpos+
                          offsetof(file_t,f_mtime),
                          &mtime,sizeof(mtime));
  if (E_ISERR(error)) return error;
 }
 if (changed&IATTR_SIZ) {
  le32 size = BSWAP_H2LE32((u32)ino->i_attr.ia_siz);
  /* TODO: Change FAT link length. */
  error = blkdev_writeall(ino->i_super->sb_blkdev,
                          NODE->f_idata.i_pos.fp_headpos+
                          offsetof(file_t,f_size),
                          &size,sizeof(size));
  if (E_ISERR(error)) return error;
 }
 return -EOK;
}
PRIVATE REF struct inode *KCALL
fat_lookup(struct inode *__restrict dir_node,
           struct dentry *__restrict result_path) {
 struct lookupdata data; pos_t begin,end;
 size_t sector_size; byte_t *buffer;
 struct dentryname *name = &result_path->d_name;
 REF struct inode *result = NULL;
 fat_t *fat = (fat_t *)dir_node->i_super;
 cluster_t cluster_id;
 sector_size = fat->f_sectorsize;
 cluster_id  = dir_node->i_data->i_cluster;
 /* Special case: Root-directory references on FAT12/16 filesystems. */
 if (cluster_id == FAT_CUSTER_FAT16_ROOT && fat->f_type != FAT32)
     return fat16_root_lookup(&dir_node->i_super->sb_root,result_path);
 /* XXX: This buffer allocation is very expensive,
  *      but the stack might not be large enough. */
 buffer = (byte_t *)malloc(sector_size);
 if unlikely(!buffer) return E_PTR(-ENOMEM);
 lookupdata_init(&data);
 while (cluster_id < fat->f_cluster_eof) {
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
                              begin,cluster_id,fat);
   if (result != NULL) break;
   begin += sector_size;
  }
  if (result != NULL) break;
  /* Go to the next cluster. */
  result = E_PTR(fat_get(fat,cluster_id,&cluster_id));
  if (E_ISERR(result)) break;
 }
 lookupdata_fini(&data);
 /* Return '-ENOENT' when the entry wasn't found. */
 if (!result) result = E_PTR(-ENOENT);
 free(buffer);
 return result;
}

STATIC_ASSERT(offsetof(file_t,f_marker) == 0);


#define FAT  ((fat_t *)dir_node)
PRIVATE REF struct inode *KCALL
fat16_root_lookup(struct inode *__restrict dir_node,
                  struct dentry *__restrict result_path) {
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
                             FAT->f_cluster_eof_marker,FAT);
  if (result != NULL) break;
  begin += sector_size;
 }
 lookupdata_fini(&data);
 /* Return '-ENOENT' when the entry wasn't found. */
 if (!result) result = E_PTR(-ENOENT);
 free(buffer);
 return result;
}
#undef FAT
#undef NODE

PRIVATE errno_t KCALL
fat_fssync(struct superblock *__restrict sb) {
 errno_t error = fat_flushtable((fat_t *)sb);
 return E_ISERR(error) ? error : blkdev_flush(sb->sb_blkdev);
}
#define FAT ((fat_t *)sb)
PRIVATE void KCALL
fat_fsfini(struct superblock *__restrict sb) {
 free(FAT->f_fat_meta);
 free(FAT->f_fat_table);
}
#undef FAT
























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
fat_flushtable(fat_t *__restrict self) {
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
  if unlikely(!self->f_fat_changed) {
   rwlock_endwrite(&self->f_fat_lock);
   return -EOK;
  }
  error = -EOK;
 }
 if (E_ISERR(error)) return error;
 /* Let's do this! */
 changed_begin = 0;
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
  FAT_DEBUG(syslog(LOG_DEBUG,"READ_CLUSTER (%I32u -> %I32u) %d (LOAD)\n",
                    id,*result,self->f_type));
  return error;
 }
 /* Now just read the FAT entry. */
 *result = FAT_TABLEGET(self,id);
 FAT_DEBUG(syslog(LOG_DEBUG,"READ_CLUSTER (%I32u -> %I32u) %d\n",
                   id,*result,self->f_type));
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
 /* Now just read the FAT entry. */
 FAT_TABLESET(self,id,value);

 /* Mark the metadata associated with the sector as changed. */
 FAT_META_STCHNG(self,table_sector);
 self->f_fat_changed = true;
end:
 rwlock_endwrite(&self->f_fat_lock);
 return error;
}

#if 0
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
 /* Now just read the FAT entry. */
 FAT_TABLESET(self,id,value);

 /* Mark the metadata associated with the sector as changed. */
 FAT_META_STCHNG(self,table_sector);
 self->f_fat_changed = true;
 return -EOK;
}
#endif





PRIVATE void KCALL trimspecstring(char *__restrict buf, size_t size) {
 while (size && isspace(*buf)) { memmove(buf,buf+1,--size); buf[size] = '\0'; }
 while (size && isspace(buf[size-1])) buf[--size] = '\0';
}




/* Create a FAT filesystem from the given block device.
 * @return: * :         A reference to a new FAT superblock.
 * @return: -EINVAL:    The given 'blkdev' doesn't contain a FAT filesystem.
 * @return: -ENOMEM:    Not enough available kernel memory.
 * @return: E_ISERR(*): Failed to create a FAT superblock for some reason. */
PRIVATE REF struct superblock *KCALL
fat_mksuper(struct blkdev *__restrict dev, u32 UNUSED(flags),
            char const *UNUSED(devname),
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
 if (E_ISERR(error)) return E_PTR(error);
 if unlikely(header.fat32.f32_bootsig[0] != 0x55 ||
             header.fat32.f32_bootsig[1] != 0xAA) ERROR(-EINVAL);
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
  result->f_idata.i_cluster    = BSWAP_LE2H32(header.fat32.f32_root_cluster);
  result->f_fat_get            = &fat_get32;
  result->f_fat_set            = &fat_set32;
  result->f_fat_sector         = &fat_sec32;
 } else {
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

 if (result->f_cluster_eof > result->f_cluster_eof_marker)
     result->f_cluster_eof = result->f_cluster_eof_marker;
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
 result->f_super.sb_root.i_data         = &result->f_idata;
 result->f_super.sb_root.__i_nlink      = 0;
 result->f_super.sb_root.i_attr.ia_mode = S_IFDIR|0777;
 result->f_super.sb_root.i_attr_disk    = result->f_super.sb_root.i_attr;
 result->f_super.sb_root.i_state        = INODE_STATE_READONLY; /* Read-only. */
 result->f_super.sb_ops                 = &fatops_super;
 result->f_mode                         = 0777;

 result->f_fat_table = malloc(result->f_fat_size);
 if unlikely(!result->f_fat_table) ERROR(-ENOMEM);
 result->f_fat_meta  = (byte_t *)calloc(1,CEILDIV(result->f_fat_size,result->f_sec4fat*(8/FAT_METABITS)));
 if unlikely(!result->f_fat_meta) { free(result->f_fat_table); ERROR(-ENOMEM); }

 /* Register the block-device with the superblock. */
 result->f_super.sb_blkdev = dev;
 BLKDEV_INCREF(dev);

 /* NOTE: The kernel's own 'THIS_INSTANCE' must never get unloaded! */
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
#define HOOK(name,id) {{NULL},THIS_INSTANCE,id,&fat_mksuper,FSTYPE_NORMAL,NULL,name}
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
 HOOK("fat",BLKSYS_MICROSOFT_BASIC_DATA),
#if 1 /* Given how generic it is, try to use FAT as a default-loader. */
 HOOK("fat",BLKSYS_ANY),
#endif
#undef HOOK
};

PRIVATE MODULE_INIT void KCALL fat_init(void) {
 struct fstype *iter;
 for (iter = fat_fshooks; iter != COMPILER_ENDOF(fat_fshooks); ++iter)
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
