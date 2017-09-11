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

#include <fs/inode.h>
#include <fs/fs.h>
#include <fs/vfs.h>
#include <malloc.h>
#include <fs/superblock.h>
#include <kernel/user.h>
#include <dev/blkdev.h>
#include <kernel/export.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <sched/types.h>
#include <hybrid/minmax.h>
#include <fcntl.h>

DECL_BEGIN

struct procnode {
 mode_t            n_mode; /*< The type and mode of this node. */
 struct dentryname n_name; /*< Name of this node. */
 struct inodeops   n_ops;  /*< Operators for this node. */
};


PRIVATE struct procnode root_content[] = {
 {0,{NULL,0,DENTRYNAME_EMPTY_HASH},{}},
};

struct rootfile {
 /* Open file descriptor within the /proc root directory. */
 struct file               rf_file;   /*< Underlying file descriptor. */
 REF struct pid_namespace *rf_proc;   /*< PID namespace being enumerated by this file. */
 size_t                    rf_diridx; /*< [lock(rf_file->f_lock)] Current absolute in-directory position. */
 size_t                    rf_bucket; /*< [lock(rf_file->f_lock)] Current bucket index. */
 size_t                    rf_bucpos; /*< [lock(rf_file->f_lock)] Current position within that bucket. */
 size_t                    rf_pidcnt; /*< [const] == rf_proc->pn_mapc (When the file was opened initially) */
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
 PRIVATE char const message[] = "This is not an easter-egg (but totally is)\n";
 if (ATOMIC_FETCHOR(fp->f_flag,bufsize >= sizeof(message)-sizeof(char) ?
                               FILE_FLAG_EASTER : 0)&(FILE_FLAG_EASTER))
     return 0;
 if (copy_to_user(buf,message,MIN(bufsize,sizeof(message)-sizeof(char))))
     return -EFAULT;
 return (ssize_t)(sizeof(message)-sizeof(char));
}

PRIVATE struct inodeops rootops = {
    .ino_fopen  = &root_fopen,
    .ino_fclose = &root_fclose,
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
