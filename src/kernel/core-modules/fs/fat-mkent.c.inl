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
#include "fat.c"
#include <hybrid/timespec.h>
#define FAT16_ROOT
#endif


/* Search for an existing directory entry `path->d_name' and return
 * it alongside `*is_new' == false, or `-EEXIST' when `IATTR_EXISTS' isn't set in `mode'.
 * Otherwise create a new entry and pre-initialize it using `result_attr',
 * as well as set `*is_new' to true.
 * NOTE: The caller is responsible for holding a write-lock to
 *      `dir_node->i_data->i_dirlock', which must be a FAT-directory node.
 * @return: * :         A reference to either a previously existing, or a newly allocated INode.
 * @return: E_ISERR(*): Failed to create a new directory entry for some reason.
 */
#ifdef FAT16_ROOT
PRIVATE REF struct fatnode *KCALL
fat16_root_mkent_unlocked(struct inode *__restrict dir_node,
                          struct dentry *__restrict path, u8 ent_attr,
                          struct iattr const *__restrict result_attr,
                          iattrset_t mode, bool *__restrict is_new)
#else
PRIVATE REF struct fatnode *KCALL
fat_mkent_unlocked(struct inode *__restrict dir_node,
                   struct dentry *__restrict path, u8 ent_attr,
                   struct iattr const *__restrict result_attr,
                   iattrset_t mode, bool *__restrict is_new)
#endif
{
#ifdef FAT16_ROOT
#ifdef __INTELLISENSE__
 fat_t *fat;
#else
#define fat   container_of(dir_node,fat_t,f_super.sb_root)
#endif
 pos_t rw_pos,rw_end;
#define RW_BEGIN  FAT_SECTORADDR(fat,fat->f_idata.i_fat16_root)
#define RW_POS    rw_pos
#define RW_END    rw_end
#else
 fat_t *fat = container_of(dir_node->i_super,fat_t,f_super);
 struct filedata rw;
#define RW_BEGIN  rw.fd_begin
#define RW_POS    rw.fd_pos
#define RW_END    rw.fd_end
#endif
 struct lookupdata lookup_data;
 REF struct fatnode *result; file_t dirent,short_entry;
 size_t lfn_count,disambiguation = 0; errno_t error;
 size_t current_unused_score = 0; /* Amount of unused entries currently tracked. */
#ifndef FAT16_ROOT
 cluster_t target_cluster;
#endif
 pos_t target_pos; /* Suitable location within the directory to put our data. */
 bool got_suitable_target = false; /* True if a suitable target location was found. */
#define must_write_end_of_directory  (!got_suitable_target) /* True if the new entry is appended to the directory. */
 assert(rwlock_writing(&dir_node->i_data->i_dirlock));
retry_disambiguation:
 /* Figure out how many consecutive file-entries
  * are required for the file to-be created.
  * HINT: 'lfn_count == CONSECUTIVE_REQUIRED-1'. */
 lfn_count = fat_make8dot3(&short_entry,&path->d_name,disambiguation);
 /* Fill in generic file information. */
 fat_atime_encode(short_entry.f_atime,&result_attr->ia_atime);
 fat_ctime_encode(short_entry.f_ctime,&result_attr->ia_ctime);
 fat_mtime_encode(short_entry.f_mtime,&result_attr->ia_mtime);
 short_entry.f_size      = (le32)0;
 short_entry.f_attr      = ent_attr;
 short_entry.f_clusterhi = BSWAP_H2LE16((u16)((u32)fat->f_cluster_eof_marker >> 16));
 short_entry.f_clusterlo = BSWAP_H2LE16((u16)fat->f_cluster_eof_marker);
#ifdef FAT16_ROOT
 rw_pos = FAT_SECTORADDR(fat,fat->f_idata.i_fat16_root);
 rw_end = rw_pos+fat->f_fat16_rootmax*sizeof(file_t);
#else
 FILEDATA_OPENDIR(&rw,fat,dir_node->i_data);
#endif
 lookupdata_init(&lookup_data);
 for (;;) {
  HOSTMEMORY_BEGIN {
#ifdef FAT16_ROOT
   result = E_PTR(rw_pos == rw_end ? 0 :
                  blkdev_read(fat->f_super.sb_blkdev,rw_pos,
                             &dirent,sizeof(file_t)));
   if (E_ISOK(result)) rw_pos += (size_t)result;
#else
   result = E_PTR(filedata_read(&rw,fat,dir_node,&dirent,sizeof(file_t)));
#endif
  }
  HOSTMEMORY_END;
  if (E_ISERR(result)) goto end;
  if (!result) break;
  FAT_DEBUG(syslog(LOG_DEBUG,"DENTRY: %$q\n",sizeof(dirent.f_nameext),dirent.f_nameext));
  if (dirent.f_marker == MARKER_UNUSED) {
   /* Reclaim unused directory entries. */
   if (!current_unused_score) {
#ifndef FAT16_ROOT
    target_cluster = rw.fd_cluster;
#endif
    target_pos = RW_POS-sizeof(file_t);
    assert(target_pos >= RW_BEGIN);
    assert(target_pos <= RW_END);
   }
   if (!got_suitable_target) {
    /* Check if the unused entry are we've
     * discovered is of sufficient size. */
    if (++current_unused_score >= disambiguation+1)
        got_suitable_target = true;
   }
   continue;
  }
  if (!got_suitable_target) {
   current_unused_score = 0;
  }
  if (dirent.f_marker == MARKER_DIREND) {
   /* (Current) end of directory. */
   RW_POS -= sizeof(file_t);
   assert(RW_POS >= RW_BEGIN);
   assert(RW_POS <= RW_END);
   break;
  }
  result = (REF struct fatnode *)fat_lookup_memory(&lookup_data,&path->d_name,
                                                   &dirent,1,RW_POS-sizeof(file_t),
#ifdef FAT16_ROOT
                                                    fat->f_cluster_eof_marker,
#else
                                                    rw.fd_cluster,
#endif
                                                    fat,NULL,0);
  if (result != NULL) {
   /* Found an existing entry with the same name. */
   if (!(mode&IATTR_EXISTS)) {
    /* If we've not supposed to return existing nodes,
     * return `-EEXIST' (Already exists) instead. */
    INODE_DECREF(&result->f_inode);
    result = E_PTR(-EEXIST);
   }
   /* Return the existing Inode. */
   *is_new = false;
   goto end;
  }

  /* Check if out short entry collides with another existing one. */
  if (memcmp(short_entry.f_nameext,dirent.f_nameext,
             sizeof(dirent.f_nameext)) == 0) {
   assertf(lfn_count >= 1,
           "But if this one collides (%$q), how come `fat_lookup_memory()' didn't pick up on it?",
           sizeof(dirent.f_nameext),dirent.f_nameext);
   /* It does. (Try again with a greater disambiguation) */
   assert(disambiguation <= FAT_8DOT3_MAX_DISAMBIGUATION);
   if unlikely(disambiguation == FAT_8DOT3_MAX_DISAMBIGUATION) {
    /* _very_ unlikely: Too many collisions. */
    result = E_PTR(-EEXIST);
    goto end;
   }

   /* Retry with greater `disambiguation' */
   ++disambiguation;
   goto retry_disambiguation;
  }
 }
 /* No entry with the name we were looking for exists.
  * >> With that in mind, we are allowed to create our new entry! */
 if (got_suitable_target || current_unused_score) {
  /* If the directory ended just after a few unused entries, re-use them. */
#ifndef FAT16_ROOT
  rw.fd_cluster = target_cluster;
  rw.fd_begin   = FAT_SECTORADDR(fat,FAT_CLUSTERSTART(fat,target_cluster));
  rw.fd_end     = rw.fd_max = rw.fd_begin+fat->f_clustersize;
  rw.fd_pos     = target_pos;
#else
  rw_pos = target_pos;
#endif
  assert(RW_POS >= RW_BEGIN);
  assert(RW_POS <= RW_END);
 }
#ifdef FAT16_ROOT
 /* Make sure we still have enough space in the root directory. */
 if (rw_pos+(lfn_count+1)*sizeof(file_t) > rw_end) {
  /* XXX: Defragment root directory? */
  syslog(LOG_FS|LOG_WARN,"[FAT] FAT12/16 root directory is full\n");
  result = E_PTR(-ENOSPC);
  goto end;
 }
#endif

 /* Create the new INode we're going to return. */
 result = fatnode_new();
 if unlikely(!result) { result = E_PTR(-ENOMEM); goto end; }
 result->f_idata.i_cluster        = fat->f_cluster_eof_marker;
#ifdef FAT16_ROOT
 result->f_idata.i_pos.fp_namecls = fat->f_cluster_eof_marker;
#else
 result->f_idata.i_pos.fp_namecls = rw.fd_cluster;
#endif
 result->f_idata.i_pos.fp_namepos = RW_POS;
 //result->f_idata.i_pos.fp_headpos = ...; /* Filled later. */
 rwlock_cinit(&result->f_idata.i_dirlock);
 result->f_inode.__i_nlink = 1;
 result->f_inode.i_data    = &result->f_idata;
 memcpy(&result->f_inode.i_attr,result_attr,sizeof(struct iattr));
 result->f_inode.i_attr.ia_mode &= ~__S_IFMT;
 if (ent_attr&ATTR_DIRECTORY) {
  result->f_inode.i_ops = &fatops_dir;
  result->f_inode.i_attr.ia_mode |= S_IFDIR;
 } else {
  result->f_inode.i_ops = &fatops_reg;
  result->f_inode.i_attr.ia_mode |= S_IFREG;
 }
 /* Mirror attributes in disk cache. */
 memcpy(&result->f_inode.i_attr_disk,
        &result->f_inode.i_attr,
         sizeof(struct iattr));

 HOSTMEMORY_BEGIN {
  /* Start by writing all long-filename entries. */
  if (lfn_count) {
   size_t i = lfn_count; u8 chksum;
   chksum = fat_LFNchksum(short_entry.f_nameext);
   /* Apparently FAT wants this stuff reverse order (But KOS doesn't care...) */
   while (i--) {
    file_t buf; fat_makeLFN(&buf,&path->d_name,i,chksum);
    if (i == lfn_count-1) buf.lfn_seqnum |= 0x40; /* ??? (Marker for last logical, first physical; WHY??? WOULD??? WE??? NEED??? THIS???) */
#ifdef FAT16_ROOT
    error = (errno_t)blkdev_writeall(fat->f_super.sb_blkdev,
                                     rw_pos,&buf,sizeof(file_t));
    rw_pos += sizeof(file_t);
#else
    error = (errno_t)filedata_write(&rw,fat,dir_node,&buf,sizeof(file_t));
#endif
    if (E_ISERR(error)) goto stop_writing;
   }
  }
  /* Store the position of the actual file header in the result INode. */
  result->f_idata.i_pos.fp_headpos = RW_POS;
  /* Next write the short-name entry. */
#ifdef FAT16_ROOT
  error = (errno_t)blkdev_writeall(fat->f_super.sb_blkdev,
                                   rw_pos,&short_entry,sizeof(file_t));
  rw_pos += sizeof(file_t);
#else
  error = (errno_t)filedata_write(&rw,fat,dir_node,&short_entry,sizeof(file_t));
#endif
  if (E_ISERR(error)) goto stop_writing;
  /* And finally, if we have to, write a directory terminator. */
  if (must_write_end_of_directory) {
   memset(&short_entry,0,sizeof(file_t));
#if MARKER_DIREND != 0
   short_entry.f_marker = MARKER_DIREND;
#endif
#ifdef FAT16_ROOT
   error = (errno_t)blkdev_writeall(fat->f_super.sb_blkdev,
                                    rw_pos,&short_entry,sizeof(file_t));
   rw_pos += sizeof(file_t);
#else
   error = (errno_t)filedata_write(&rw,fat,dir_node,&short_entry,sizeof(file_t));
#endif
  }
stop_writing:;
 }
 HOSTMEMORY_END;
 if (E_ISERR(error)) { free(result); result = E_PTR(error); goto end; }
 *is_new = true;

 /* We (ab-)use the header position as INode number. (It's better than nothing...) */
 result->f_inode.i_ino = (ino_t)result->f_idata.i_pos.fp_headpos;

 /* Setup the resulting node to use. (NOTE: May never fail, because FAT is a core module) */
 asserte(E_ISOK(inode_setup(&result->f_inode,&fat->f_super,THIS_INSTANCE)));
end:
 lookupdata_fini(&lookup_data);
 return result;
#undef must_write_end_of_directory
#undef RW_BEGIN
#undef RW_POS
#undef RW_END
#undef fat
}


#ifdef FAT16_ROOT
PRIVATE REF struct inode *KCALL
fat16_root_rename(struct inode *__restrict dst_dir, struct dentry *__restrict dst_path,
                  struct inode *__restrict src_dir, struct dentry *__restrict UNUSED(src_path),
                  struct inode *__restrict src_node)
#else
PRIVATE REF struct inode *KCALL
fat_rename(struct inode *__restrict dst_dir, struct dentry *__restrict dst_path,
           struct inode *__restrict src_dir, struct dentry *__restrict UNUSED(src_path),
           struct inode *__restrict src_node)
#endif
{
#define SRC_NODE container_of(src_node,struct fatnode,f_inode)
 bool is_new; REF struct fatnode *result_node; errno_t error;
 /* Lock the source node's attributes and the target directory. */
 result_node = E_PTR(rwlock_read(&src_node->i_attr_lock));
 if (E_ISERR(result_node)) goto end;
 result_node = E_PTR(rwlock_write(&dst_dir->i_data->i_dirlock));
 if (E_ISERR(result_node)) goto end2;
 /* Create a new node using the given target name in the dst-directory. */
#ifdef FAT16_ROOT
 result_node = fat16_root_mkent_unlocked(dst_dir,dst_path,INODE_ISDIR(src_node)
                                         ? ATTR_DIRECTORY : 0,&src_node->i_attr_disk,
                                         0,&is_new);
#else
 result_node = fat_mkent_unlocked(dst_dir,dst_path,INODE_ISDIR(src_node)
                                  ? ATTR_DIRECTORY : 0,&src_node->i_attr_disk,
                                  0,&is_new);
#endif
 if (E_ISERR(result_node)) goto err_unlock_dst;
 assert(is_new);
 result_node->f_idata.i_cluster = SRC_NODE->f_idata.i_cluster;
 /* Copy node attributes one-on-one. */
 memcpy(&result_node->f_inode.i_attr,&src_node->i_attr,sizeof(struct iattr));
 memcpy(&result_node->f_inode.i_attr_disk,&src_node->i_attr,sizeof(struct iattr));
 { struct {
    filectime_t f_ctime;
    fileatime_t f_atime;
    le16        f_clusterhi;
    filemtime_t f_mtime;
    le16        f_clusterlo;
    le32        f_size;
   } buf;
   /* Update the target file's on-disk file buffer. */
   fat_ctime_encode(buf.f_ctime,&src_node->i_attr.ia_ctime);
   fat_atime_encode(buf.f_atime,&src_node->i_attr.ia_atime);
   fat_mtime_encode(buf.f_mtime,&src_node->i_attr.ia_mtime);
   buf.f_clusterhi = BSWAP_H2LE16((u16)((u32)result_node->f_idata.i_cluster >> 16));
   buf.f_clusterlo = BSWAP_H2LE16((u16)result_node->f_idata.i_cluster);
   buf.f_size      = BSWAP_H2LE32((u32)result_node->f_inode.i_attr.ia_siz);
   /* Write the new directory entry. */
   HOSTMEMORY_BEGIN {
    error = blkdev_writeall(dst_dir->i_super->sb_blkdev,
                            result_node->f_idata.i_pos.fp_headpos+
                            offsetof(file_t,f_ctime),
                           &buf,sizeof(buf));
   }
   HOSTMEMORY_END;
   if (E_ISERR(error)) goto err_delete_dst;
 }
 /* All right. The target directory now contains the new file entry.
  * >> Now all that's left is to delete the old one from the old directory.
  * NOTE: At this point we try not to fail anymore as once we unlock
  *       the new directory, we allow anyone to access the file there.
  *    >> So with that in mind, try not to be interrupted when we're
  *       deleting the file's old directory entry. */
 task_nointr();
 if (dst_dir != src_dir) {
  /* Switch locks to acquire write-access to the old directory. */
  rwlock_endwrite(&dst_dir->i_data->i_dirlock);
  rwlock_write(&src_dir->i_data->i_dirlock);
 }
 /* Remove the source node from its old directory. */
#ifdef FAT16_ROOT
 fat16_root_rment_unlocked(src_dir,container_of(src_node,struct fatnode,f_inode));
#else
 fat_rment_unlocked(src_dir,container_of(src_node,struct fatnode,f_inode));
#endif
 task_endnointr();
 rwlock_endwrite(&src_dir->i_data->i_dirlock);
end2: rwlock_endread(&src_node->i_attr_lock);
end:  return (REF struct inode *)result_node;
err_delete_dst:
 task_nointr();
#ifdef FAT16_ROOT
 fat16_root_rment_unlocked(dst_dir,result_node);
#else
 fat_rment_unlocked(dst_dir,result_node);
#endif
 task_endnointr();
err_unlock_dst:
 rwlock_endwrite(&dst_dir->i_data->i_dirlock);
 goto end2;
}


#ifdef FAT16_ROOT
#undef FAT16_ROOT
#endif
