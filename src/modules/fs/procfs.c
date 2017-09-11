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
#ifndef GUARD_MODULES_FS_PROCFS_C
#define GUARD_MODULES_FS_PROCFS_C 1
#define _KOS_SOURCE 1

#include <dev/blkdev.h>
#include <dirent.h>
#include <fcntl.h>
#include <fs/fs.h>
#include <fs/vfs.h>
#include <malloc.h>
#include <fs/superblock.h>
#include <kernel/user.h>
#include <dev/blkdev.h>
#include <kernel/export.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kernel/export.h>
#include <kernel/user.h>
#include <malloc.h>
#include <sched/types.h>
#include <stdlib.h>

DECL_BEGIN

struct procnode {
 ino_t             n_ino;  /*< Inode number of this entry. */
 mode_t            n_mode; /*< The type and mode of this node. */
 struct dentryname n_name; /*< Name of this node. */
 struct inodeops   n_ops;  /*< Operators for this node. */
};

#if __SIZEOF_SIZE_T__ == 4
#   define H(h32,h64) h32
#elif __SIZEOF_SIZE_T__ == 8
#   define H(h32,h64) h64
#elif !defined(__DEEMON__)
#   error FIXME
#endif

#ifdef __DEEMON__
#define DNAME_HASH32(x) \
({ local hash = uint32(0); \
   local temp = string.utf32.from_data((char *)x,#(x)/4); \
   for (local c: temp) { hash += c.ord(); hash *= 9; } \
   local temp = x[#(x) - #(x) % 4 : #(x)]; \
   switch (#temp) { \
   case 3:  hash += x[2].ord() << 16; \
   case 2:  hash += x[1].ord() << 8; \
   case 1:  hash += x[0].ord(); \
   default: break; \
   } \
   hash;\
})
#define DNAME_HASH64(x) \
({ local hash = uint64(0); \
   local temp = (uint64 *)(char *)x; \
   for (local i = 0; i < #(x) / 8; ++i) { hash += temp[i]; hash *= 9; } \
   local temp = x[#(x) - #(x) % 8 : #(x)]; \
   switch (#temp) { \
   case 7:  hash += x[6].ord() << 48; \
   case 6:  hash += x[5].ord() << 40; \
   case 5:  hash += x[4].ord() << 32; \
   case 4:  hash += x[3].ord() << 24; \
   case 3:  hash += x[2].ord() << 16; \
   case 2:  hash += x[1].ord() << 8; \
   case 1:  hash += x[0].ord(); \
   default: break; \
   } \
   hash;\
})
#define DNAM(x) ({ local _x = x; print "{"+repr(_x)+","+#_x+",H("+DNAME_HASH32(_x)+"u,"+DNAME_HASH64(_x)+"llu)}",; })
#endif
  

PRIVATE SAFE ssize_t KCALL
self_readlink(struct inode *__restrict UNUSED(ino),
              USER char *__restrict buf, size_t bufsize) {
 return snprintf_user(buf,bufsize,"%d",GET_THIS_PID());
}

PRIVATE struct procnode root_content[] = {
 {0,S_IFLNK|0555,/*[[[deemon DNAM("self"); ]]]*/{"self",4,H(2580517131u,1718379891llu)}/*[[[end]]]*/,
 { .ino_readlink = &self_readlink,
 }},
};

struct rootfile {
 /* Open file descriptor within the /proc root directory. */
 struct file               rf_file;   /*< Underlying file descriptor. */
 REF struct pid_namespace *rf_proc;   /*< PID namespace being enumerated by this file. */
 size_t                    rf_diridx; /*< [lock(rf_file->f_lock)] Current absolute in-directory position. */
 size_t                    rf_bucket; /*< [lock(rf_file->f_lock)] Current bucket index. */
 size_t                    rf_bucpos; /*< [lock(rf_file->f_lock)] Current position within that bucket. */
 size_t                    rf_pidcnt; /*< [const] == rf_proc->pn_mapc (When the file was opened initially)
                                       *   HINT: When 'rf_diridx >= rf_pidcnt', read from 'root_content' */
};

PRIVATE REF struct file *KCALL
root_fopen(struct inode *__restrict ino,
           struct dentry *__restrict node_ent,
           oflag_t oflags) {
 REF struct rootfile *result;
 result = (struct rootfile *)file_new(sizeof(struct rootfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 /* Use the caller's effective PID namespace. */
 result->rf_proc = THIS_NAMESPACE;
 CHECK_HOST_DOBJ(result->rf_proc);
 result->rf_pidcnt = ATOMIC_READ(result->rf_proc->pn_mapc);
 PID_NAMESPACE_INCREF(result->rf_proc);
 result->rf_pidcnt = 0; /* TODO: Remove me. */
 file_setup(&result->rf_file,ino,node_ent,oflags);
 return &result->rf_file;
}
PRIVATE void KCALL
root_fclose(struct inode *__restrict UNUSED(ino),
            struct file *__restrict fp) {
 /* Drop a reference from the associated PID namespace. */
 PID_NAMESPACE_DECREF(container_of(fp,struct rootfile,rf_file)->rf_proc);
}

PRIVATE ssize_t KCALL
root_read(struct file *__restrict fp,
          USER void *buf, size_t bufsize) {
#define FILE_FLAG_EASTER (FILE_FLAG_LOCKLESS >> 1)
 PRIVATE char const *msgs[] = {
  "This is not an easter-egg (wait. - It totally is)\n",
  "You've just read the wrong directory...\n",
  "formatting all connected devices (please wait...)\n",};
 char const *m = msgs[rand() % COMPILER_LENOF(msgs)]; size_t len = strlen(m)*sizeof(char);
 if (ATOMIC_FETCHOR(fp->f_flag,bufsize >= len ? FILE_FLAG_EASTER : 0)&(FILE_FLAG_EASTER))
     return 0;
 if (copy_to_user(buf,m,MIN(bufsize,len)))
     return -EFAULT;
 return (ssize_t)len;
}
PRIVATE ssize_t KCALL
root_readdir(struct file *__restrict fp,
             USER struct dirent *buf,
             size_t bufsize, rdmode_t mode) {
 struct rootfile *self = container_of(fp,struct rootfile,rf_file);
 ssize_t result = 0;
 if (self->rf_diridx >= self->rf_pidcnt) {
  struct dirent header; size_t name_avail;
  struct procnode *node; size_t index;
  index = self->rf_diridx-self->rf_pidcnt;
  if (index >= COMPILER_LENOF(root_content))
      return 0; /* End of directory. */
  node = &root_content[index];
  header.d_ino    = node->n_ino;
  header.d_namlen = node->n_name.dn_size;
  header.d_type   = IFTODT(node->n_mode);
  result = offsetof(struct dirent,d_name)+(1+node->n_name.dn_size)*sizeof(char);
  name_avail = 0;
  if (bufsize >= offsetof(struct dirent,d_name)) {
   if (copy_to_user(buf,&header,offsetof(struct dirent,d_name)))
       return -EFAULT;
   name_avail = bufsize-offsetof(struct dirent,d_name);
   *(uintptr_t *)&buf += offsetof(struct dirent,d_name);
  }
  if (copy_to_user(buf,node->n_name.dn_name,
                   MIN((node->n_name.dn_size+1)*sizeof(char),
                   name_avail)))
      return -EFAULT;
 } else {
  /* TODO: Enumerate running PIDs */
 }
 if (FILE_READDIR_SHOULDINC(mode,bufsize,result)) {
  ++self->rf_diridx;
  ++self->rf_bucpos;
 }
 return result;
}

PRIVATE REF struct inode *KCALL
root_lookup(struct inode *__restrict dir_node,
            struct dentry *__restrict result_path) {
 struct procnode *iter; errno_t error;
 REF struct inode *result;
 for (iter = root_content;
      iter != COMPILER_ENDOF(root_content); ++iter) {
#ifdef CONFIG_DEBUG
  struct dentryname name_copy;
  name_copy.dn_name = iter->n_name.dn_name;
  name_copy.dn_size = iter->n_name.dn_size;
  dentryname_loadhash(&name_copy);
  assertf(name_copy.dn_hash == iter->n_name.dn_hash,
          "Broken hash generator (%Ix != %Ix)",
          name_copy.dn_hash,iter->n_name.dn_hash);
#endif
  if (iter->n_name.dn_hash == result_path->d_name.dn_hash &&
      iter->n_name.dn_size == result_path->d_name.dn_size &&
      memcmp(iter->n_name.dn_name,result_path->d_name.dn_name,
             result_path->d_name.dn_size*sizeof(char)) == 0) {
   /* Found it! */
   result = inode_new(sizeof(struct inode));
   if unlikely(!result) return E_PTR(-ENOMEM);
   result->i_ino = iter->n_ino;
   result->i_ops = &iter->n_ops;
   result->i_attr.ia_mode = iter->n_mode;
   result->i_attr_disk.ia_mode = iter->n_mode;
   error = inode_setup(result,dir_node->i_super,THIS_INSTANCE);
   if (E_ISERR(error)) { free(result); result = E_PTR(error); }
   return result;
  }
 }
 /* TODO: Decode PID decimals. */
 return E_PTR(-ENOENT);
}

PRIVATE struct inodeops rootops = {
    .ino_fopen  = &root_fopen,
    .ino_fclose = &root_fclose,
    .ino_lookup = &root_lookup,
    .f_readdir  = &root_readdir,
    .f_read     = &root_read,
};
PRIVATE struct superblockops sb_rootops = {
};


PRIVATE SAFE REF struct superblock *KCALL
procfs_make(struct blkdev *__restrict UNUSED(dev), u32 flags,
            char const *UNUSED(devname),
            USER void *UNUSED(data), void *UNUSED(closure)) {
 REF struct superblock *result; errno_t error;
 /* Create a new superblock. */
 result = (REF struct superblock *)superblock_new(sizeof(struct superblock));
 if unlikely(!result) return E_PTR(-ENOMEM);

 /* Initialize/fill-in basic data. */
 result->sb_ops        = &sb_rootops;
 result->sb_root.i_ops = &rootops;
 result->sb_root.i_attr.ia_mode      = S_IFDIR|0555;
 result->sb_root.i_attr_disk.ia_mode = S_IFDIR|0555;

 /* Finally, setup the superblock. */
 error = superblock_setup(result,THIS_INSTANCE);
 if (E_ISERR(error)) {
  free(result);
  return E_PTR(error);
 }
 return result;
}


PRIVATE struct fstype procfs = {
    .f_owner    = THIS_INSTANCE,
    .f_sysid    = BLKSYS_EXPLICIT,
    .f_callback = &procfs_make,
    .f_flags    = FSTYPE_NODEV, /* Doesn't make use of devices. */
    .f_name     = "proc",
};


PRIVATE MODULE_INIT errno_t KCALL procfs_init(void) {
 fs_addtype(&procfs);
 return -EOK;
}


DECL_END

#endif /* !GUARD_MODULES_FS_PROCFS_C */
