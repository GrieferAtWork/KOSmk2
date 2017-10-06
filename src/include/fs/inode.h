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
#ifndef GUARD_INCLUDE_FS_INODE_H
#define GUARD_INCLUDE_FS_INODE_H 1

#include <bits/stat.h>
#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/timespec.h>
#include <hybrid/types.h>
#include <sync/rwlock.h>
#include <sync/sig.h>

DECL_BEGIN

struct stat64;
struct device;
struct blkdev;
struct chrdev;
struct file;
struct inode;
struct inodeops;
struct ifile;
struct ifilelock;
struct idata;    /* File-system specific INode data. */
struct module;
struct dentry;
struct fsaccess;
struct dirent;
struct sig;

#ifndef __iattrset_t_defined
#define __iattrset_t_defined 1
typedef u32 iattrset_t; /* Set of 'IATTR_*' */
#endif /* !__iattrset_t_defined */

#ifndef __rdmode_t_defined
#define __rdmode_t_defined 1
typedef int rdmode_t; /* readdir-mode (One of 'FILE_READDIR_*') */
#endif /* !__rdmode_t_defined */

#ifndef __pollmode_t_defined
#define __pollmode_t_defined 1
typedef int pollmode_t; /* poll()-mode (Set of 'POLLIN|POLLPRI|POLLOUT|POLLERR'). */
#endif /* !__pollmode_t_defined */

#ifndef __fallocmode_t_defined
#define __fallocmode_t_defined 1
typedef int fallocmode_t; /* fallocate()-mode. */
#endif /* !__fallocmode_t_defined */
#define FALLOCMODE_NORMAL 0 /* ??? */


/* INode-changed flags for 'ino_setattr'. */
#define IATTR_NONE    0
#define IATTR_MODE   (1 << 0)
#define IATTR_UID    (1 << 1)
#define IATTR_GID    (1 << 2)
#define IATTR_SIZ    (1 << 3)
#define IATTR_ATIME  (1 << 4)
#define IATTR_MTIME  (1 << 5)
#define IATTR_CTIME  (1 << 6)
#define IATTR_TRUNC  (1 << 29) /* FLAG: For 'ino_mkreg' - When the named INode already exists, clear its contents and return it instead. */
#define IATTR_EXISTS (1 << 30) /* FLAG: For 'ino_mkreg' - When the named INode already exists, return it instead. */
#define IATTR_NOW    (1 << 31) /* FLAG: For 'inode_setattr' - Attributes changes _must_ be mirrored immediately. */

#define IATTR_MODE_VMASK 07777 /* Mask of bits that may be changed in 'ia_mode' */

struct iattr {
 /* INode attributes. */
 mode_t          ia_mode;  /*< [const(~IATTR_MODE_VMASK)] General-purpose file kind, etc.
                            *  NOTE: The find-kind portion of this is constant! */
 uid_t           ia_uid;   /*< File owner UID. */
 gid_t           ia_gid;   /*< File owner GID. */
 pos_t           ia_siz;   /*< File size. */
 struct timespec ia_atime; /*< Last access time. */
 struct timespec ia_mtime; /*< Last modification time. */
 struct timespec ia_ctime; /*< File creation time. */
};

/* NOTE: Keep these mode constants in sync with 'READDIR_*' from <dirent.h> */
#define FILE_READDIR_DEFAULT  0 /*< Yield to next entry when 'buf' was of sufficient size. */
#define FILE_READDIR_CONTINUE 1 /*< Always yield to next entry. */
#define FILE_READDIR_PEEK     2 /*< Never yield to next entry. */
#define FILE_READDIR_SHOULDINC(mode,bufsize,return_value) \
 ((mode) != FILE_READDIR_PEEK && \
 ((mode) != FILE_READDIR_DEFAULT || (bufsize) >= (size_t)(return_value)))

/* Check/Cast to derived classes. */
#define INODE_ISDEV(self)  __S_ISDEV((self)->i_attr_disk.ia_mode)
#define INODE_ISBLK(self)  __S_ISBLK((self)->i_attr_disk.ia_mode)
#define INODE_ISCHR(self)  __S_ISCHR((self)->i_attr_disk.ia_mode)
#define INODE_TODEV(self)  ((struct device *)(self))
#define INODE_TOBLK(self)  ((struct blkdev *)(self))
#define INODE_TOCHR(self)  ((struct chrdev *)(self))

struct inodeops {
 void *o_tag; /* Operations tag (May be used to identify operation groups) */
 /* File operators. (Return negative error codes on failure)
  * NOTE: Read/Write permissions must already be handled by the caller of these functions. */
 u32             f_flags; /*< Set of 'INODE_FILE_*' */
#define INODE_FILE_NORMAL   0x00000000
#define INODE_FILE_LOCKLESS 0x80000000 /*< Allow for interlocked read/write, meaning that 'f_read', 'f_write',
                                        * 'f_seek', 'f_readdir' and 'f_sync' are no longer caller-synchronized.
                                        *  WARNING: When this flag is set, the file itself must implement
                                        *           locking capabilities, as well as support for 'O_APPEND'. */
 ssize_t (KCALL *f_read)(struct file *__restrict fp, USER void *buf, size_t bufsize); /* NOTE: Caller-synchronized:write */
 ssize_t (KCALL *f_write)(struct file *__restrict fp, USER void const *buf, size_t bufsize); /* NOTE: Caller-synchronized:write */
 ssize_t (KCALL *f_pread)(struct file *__restrict fp, USER void *buf, size_t bufsize, pos_t pos);
 ssize_t (KCALL *f_pwrite)(struct file *__restrict fp, USER void const *buf, size_t bufsize, pos_t pos);

 /* File-level callbacks for creating a new memory region for a given range within a file.
  * NOTE: Set to NULL to use the (much more versatile) generic file mapping,
  *       making use of lazy allocation & initialization using 'f_pread/f_read'.
  * >> Only device drivers providing physical memory mappings should ever implement this.
  *    Never implement this for regular files, such as in filesystem drivers!
  * @param: mode:        One of 'MREGION_FILE_*'
  * @return: * :         A new reference to the mapped memory region. (Must be properly set up)
  * @return: E_ISERR(*): Failed to create the new memory region for some reason. */
 REF struct mregion *(KCALL *f_mmap)(struct file *__restrict fp, pos_t pos, size_t size);

 /* NOTE: When 'f_seek' is missing, 'ESPIPE' is generated for
  *       'S_ISFIFO', 'S_ISCHR' and 'S_ISSOCK' files. - 'EPERM' otherwise. */
 off_t   (KCALL *f_seek)(struct file *__restrict fp, off_t off, int whence); /* NOTE: Caller-synchronized:write */
 errno_t (KCALL *f_ioctl)(struct file *__restrict fp, int name, USER void *arg);
 /* NOTE: 'f_readdir' returns ZERO(0) when the end of the directory is reached (ZERO: no more data can be read) */
 ssize_t (KCALL *f_readdir)(struct file *__restrict fp, USER struct dirent *buf, size_t bufsize, rdmode_t mode); /* NOTE: Caller-synchronized:write */
 errno_t (KCALL *f_sync)(struct file *__restrict fp); /* Sync all unwritten data. */ /* NOTE: Caller-synchronized:write */
 errno_t (KCALL *f_allocate)(struct file *__restrict fp, fallocmode_t mode, pos_t start, pos_t size);
 /* Poll for events on this file descriptor. This function should do:
  * >> bool got_unsupported_signals = false;
  * >> FOREACH_BIT_IN(b,mode) {
  * >>     struct sig *s;
  * >>     if (!IS_SIGNAL_SUPPORTED(b)) {
  * >>         got_unsupported_signals = true;
  * >>         continue;
  * >>     }
  * >>     s = GET_SIGNAL_SEND_BY_MODE(b);
  * >>     sig_write(s);
  * >>     if (CHECK_DATA_AVAILABLE_FOR_MODE(b)) {
  * >>         result |= b;
  * >>     } else {
  * >>         // Add the signal to the waiting-set if we havn't discovered any data yet.
  * >>         // NOTE: It'd be just as correct if always added the signal
  * >>         if (!result) {
  * >>             errno_t error = task_addwait(s);
  * >>             if (E_ISERR(error)) {
  * >>                 sig_endwrite(s);
  * >>                 return error;
  * >>             }
  * >>         }
  * >>     }
  * >>     sig_endwrite(s);
  * >> }
  * >> if (!result && got_unsupported_signals)
  * >>     result = -EWOULDBLOCK;
  * >> return result;
  * WARNING: Regardless of return value, any number of signals way have been
  *          added to the calling thread's sigwait set, meaning that the caller
  *          is always responsible for cleanup by either calling 'task_clrwait()'
  *          or 'task_waitfor()'.
  * @assume((mode&(POLLIN|POLLPRI|POLLOUT|POLLERR)) != 0);
  * @param: mode: Set of 'POLLIN|POLLPRI|POLLOUT|POLLERR'
  * @return: * :           The set of signals currently available. (At least one of 'mode')
  *                  NOTE: In this event, signals may have still been added to waiting 'task_addwait()',
  *                        meaning that the caller is responsible for cleanup, which must include a call
  *                        to 'task_clrwait()'
  * @return: 0 :           At least one signal has been added to the caller's waiting set (using 'task_addwait()').
  * @return: -EWOULDBLOCK: No signal signal requested for polling actually exists.
  * @return: -ENOMEM:      Not enough available memory (likely occurred when
  *                        trying to allocate a signal slot within 'task_addwait()')
  * @return: E_ISERR(*):   Failed to poll data for some reason. */
 pollmode_t (KCALL *f_poll)(struct file *__restrict fp, pollmode_t mode);

 /* Invalidate cached data within the specified data range, after some other file modified it.
  * NOTE: This function may also be called after a file has been truncated, in order to notify open files of the changes. */
 void (KCALL *f_invalidate)(struct file *__restrict fp, pos_t start, pos_t size); /* NOTE: Caller-synchronized:write */
 /* Called just before a given INode is destroyed for the lack of references. */
 void (KCALL *ino_fini)(struct inode *__restrict ino);
 /* Return a reference to the effective INode.
  *  - When implemented, this operator is used before dereference
  *    an INode for calling any of the other 'ino_*' operators.
  *  - This operator should be implemented for proxy-nodes meant to weakly alias others, such as a device.
  * NOTE: An INode is at most dereferenced once.
  * HINT: The general consensus to indicate a dead effective node, is to return a new reference to 'SELF'
  * @return: * : A new reference to the dereferenced, effective INode.
  * @assume(E_ISOK(return));
  * @assume(return != NULL);
  * @assume(result == ino || INODE_ISEFFECTIVE(return)); */
 REF struct inode *(KCALL *ino_effective)(struct inode *__restrict ino);
 /* Allocate memory for a file container + open it as a file stream reading from 'inode'.
  * @assume(!(oflags&O_CREAT));
  * NOTE: This function may not return NULL, but must return some E_PTR(*) value instead! */
 REF struct file *(KCALL *ino_fopen)(struct inode *__restrict ino,
                                     struct dentry *__restrict node_ent,
                                     oflag_t oflags);
 /* Notify an associated inode that a file that had opened it was closed. */
 void (KCALL *ino_fclose)(struct inode *__restrict ino, struct file *__restrict fp);
 /* Free a given file pointer previously opened by 'ino_fopen'.
  * WARNING: Unlike by the time 'ino_fclose' is called, 'fp' is no longer in a valid state! */
 void (KCALL *ino_ffree)(struct inode *__restrict ino, struct file *__restrict fp);
 /* HINT: All operands may be defined as NULL for default behavior. */
 /* Mirror attribute from 'inode->i_attr' on-disk, copying all fields part of 'changed'.
  * @requires: changed != IATTR_NONE
  * WARNING: A read/write-lock on 'i_attr_lock' is held when this function is called.
  * NOTE: This operator also implements ftrunc/fallocate semantics using the 'IATTR_SIZ' field.
  * NOTE: This operator must NOT actually change the contents of 'ino->i_attr_disk'.
  *       Doing so is instead the responsibility of the caller. */
 errno_t (KCALL *ino_setattr)(struct inode *__restrict ino, iattrset_t changed);
 /* Copy the text of a symlink to the given user-space
  * buffer 'buf', including a terminating \0-character.
  * >> Upon success, and given a buffer of sufficient size,
  *    the caller can assume that "buf[return/sizeof(char)-1] == '\0'".
  * @assume(return != 0);
  * @assume(S_ISLNK(inode->i_attr.ia_mode));
  * @return: * :         The amount of required characters (including the terminating \0-character)
  * @return: E_ISERR(*): Failed to read the link for some reason. */
 ssize_t (KCALL *ino_readlink)(struct inode *__restrict ino,
                               USER char *__restrict buf, size_t bufsize);
 /* An optional operator to generate dynamica stat() information, such as a context-dependent INode size.
  * NOTE: When called, 'statbuf' is filled as it would be used if this operator were not implemented,
  *       meaning that this function must only override stat()-members that it wishes to fill with
  *       custom data. - Note though, that stat information generated by this function _ONLY_ affects
  *       what is visible when user-space calls 'sys_stat()'. - The data filled in by this is not
  *       used when the kernel tries to determine INode access permissions, or the likes.
  *    >> So keep in mind that one of the only meaningful uses of this function is for
  *       calculating context-dependent buffer sizes, such as read/write queue size for PTY drivers.
  * @return: -EOK:       Successfully modified the given buffer, or did nothing (same as not implementing this operator).
  * @return: E_ISERR(*): Failed to generate dynamic stat()-information. - the 'sys_stat()'
  *                      system call will fail with the error returned by this function. */
 errno_t (KCALL *ino_stat)(struct inode *__restrict ino, struct stat64 *__restrict statbuf);

 /* Load the inode associated with the directory entry 'name' inside of 'dir_node' at 'path':
  * $ stat "/opt/my_file"
  * >> ino_lookup(EFFECTIVE_INODE("/opt"),DENTRY("/opt/my_file")) -> INODE("/opt/my_file")
  * NOTE: Don't return NULL if you didn't find a node. - return 'E_PTR(-ENOENT)' instead!
  * @assume(S_ISDIR(inode->i_attr.ia_mode));
  * @return: * :         A reference to the INode that should be associated with 'result_path'.
  * @return: -ENOENT:    No INode exists under the name associated with 'result_path'
  * @return: E_ISERR(*): An error occurred. */
 REF struct inode *(KCALL *ino_lookup)(struct inode *__restrict dir_node,
                                       struct dentry *__restrict result_path);
 /* $ ln "/opt/my_file" "/home/my_link"
  * >> ino_hrdlink(EFFECTIVE_INODE("/home"),
  * >>             DENTRY("/home/my_file"),
  * >>             EFFECTIVE_INODE("/opt/my_file"));
  * @assume(S_ISDIR(dir_node->i_attr.ia_mode));
  * @assume(dst_node->i_super == dir_node->i_super);
  * @return: -EOK:       Successfully created a hard link.
  * @return: E_ISERR(*): An error occurred. */
 errno_t (KCALL *ino_hrdlink)(struct inode *__restrict dir_node, struct dentry *__restrict target_ent,
                              struct inode *__restrict dst_node);
 /* $ ln -s "/opt/my_file" "/home/my_link"
  * >> ino_symlink(EFFECTIVE_INODE("/home"),DENTRY("/home/my_link"),"/opt/my_file",...) -> INODE("/home/my_link");
  * @assume(S_ISDIR(dir_node->i_attr.ia_mode));
  * @return: -EOK:       Successfully created a symbolic link.
  * @return: E_ISERR(*): An error occurred. */
 REF struct inode *(KCALL *ino_symlink)(struct inode *__restrict dir_node, struct dentry *__restrict target_ent,
                                        USER char const *target_text, struct iattr const *__restrict result_attr);
 /* $ mkdir "/foo/bar"
  * >> ino_mkdir(EFFECTIVE_INODE("/foo"),DENTRY("/foo/bar")) -> INODE("/foo/bar");
  * @assume(S_ISDIR(dir_node->i_attr.ia_mode));
  * @return: -EOK:       Successfully created a directory.
  * @return: E_ISERR(*): An error occurred. */
 REF struct inode *(KCALL *ino_mkdir)(struct inode *__restrict dir_node,
                                      struct dentry *__restrict target_ent,
                                      struct iattr const *__restrict result_attr);

 /* $ > "/foo/bar"
  * >> ino_mkreg(EFFECTIVE_INODE("/foo"),DENTRY("/foo/bar")) -> INODE("/foo/bar");
  * @assume(S_ISDIR(dir_node->i_attr.ia_mode));
  * @assume(mode&IATTR_MODE);
  * @assume(mode&IATTR_UID);
  * @assume(mode&IATTR_GID);
  * @assume(mode&IATTR_SIZ);
  * @assume(mode&IATTR_ATIME);
  * @assume(mode&IATTR_MTIME);
  * @assume(mode&IATTR_CTIME);
  * @param: mode: When 'IATTR_TRUNC|IATTR_EXISTS' is set, allow existing files to
  *               be opened, and mirror attributes specified by this argument.
  *               WARNING: When the file doesn't exists, all attributes specified (except
  *                        for the node type, which is set to 'S_IFREG') are used as template.
  * @return: * :         A new reference to the INode for the regular file.
  * @return: E_ISERR(*): An error occurred. */
 REF struct inode *(KCALL *ino_mkreg)(struct inode *__restrict dir_node,
                                      struct dentry *__restrict path,
                                      struct iattr const *__restrict result_attr,
                                      iattrset_t mode);

 /* $ mknod /dev/dos_hda b 14 0
  * >> ino_mknod(EFFECTIVE_INODE("/dev"),DENTRY("/dev/dos_hda"),...) -> INODE("/dev/dos_hda");
  * @assume(S_ISDIR(dir_node->i_attr.ia_mode));
  * @return: * :         A new reference to the INode hosting the given device.
  *                NOTE: When mounting within the dev-fs, this may even be the device itself again!
  * @return: E_ISERR(*): An error occurred. */
 REF struct inode *(KCALL *ino_mknod)(struct inode *__restrict dir_node, struct dentry *__restrict target,
                                      struct device *__restrict dev);

 /* $mv "/foo/bar" "/opt/xz"
  * >> ino_rename(EFFECTIVE_INODE("/opt"),DENTRY("/opt/xz"),
  * >>            EFFECTIVE_INODE("/foo"),DENTRY("/foo/bar"),
  * >>            EFFECTIVE_INODE("/foo/bar"));
  * @assume(S_ISDIR(src_dir->i_attr.ia_mode));
  * @assume(S_ISDIR(dst_dir->i_attr.ia_mode));
  * @assume(dst_dir->i_super == src_dir->i_super);
  * @assume(src_node != src_dir && src_node != dst_dir);
  * @assume(GET_PARENT_DIRECTORY_NODE(src_node) == src_dir);
  * @return: * :         A reference to the INode now apart of 'dst_dir'
  *                     (If the filesystem supports hardlinks, this is likely to be 'src_node')
  * @return: E_ISERR(*): An error occurred. */
 REF struct inode *(KCALL *ino_rename)(struct inode *__restrict dst_dir, struct dentry *__restrict dst_path,
                                       struct inode *__restrict src_dir, struct dentry *__restrict src_path,
                                       struct inode *__restrict src_node);

 /* $rm "/opt/xz"
  * >> ino_remove(EFFECTIVE_INODE("/opt"),DENTRY("/opt/xz"),EFFECTIVE_INODE("/opt/xz"));
  * @assume(S_ISDIR(dir_node->i_attr.ia_mode));
  * @assume(file_node->i_super == dir_node->i_super);
  * @return: -EOK:         Successfully removed the file/directory.
  * @return: -ENOTEMPTY:   The given 'file_node' is a non-empty directory.
  * @return: E_ISERR(*):   The given 'file_node' is a non-empty directory.
  */
 errno_t (KCALL *ino_remove)(struct inode *__restrict dir_node,
                             struct dentry *__restrict file_path,
                             struct inode *__restrict file_node);
};

/* May be set as default callback for the 'ino_fopen' function.
 * >> This callback simply allocates and initializes an otherwise empty file structure. */
FUNDEF REF struct file *KCALL
inode_fopen_default(struct inode *__restrict ino,
                    struct dentry *__restrict node_ent,
                    oflag_t oflags);



typedef u32 istate_t; /*< INode state flags (Set of 'INODE_STATE_*'). */
#define INODE_STATE_GENERIC   0x00000000 /*< No special state behavior. */
#define INODE_STATE_READONLY  0x00000001 /*< Any attempt at performing a write-operation on the INode will fail with '-EROFS'. */
#define INODE_STATE_CLOSING   0x00000002 /*< The inode is being closed. - No new streams may be opened. */
#define INODE_STATE_DONTCACHE 0x00000004 /*< Don't cache the INode (Required to prevent weak devices from being kept alive through caches). */
#define INODE_STATE_NOSUID    0x00004000 /*< Ignore SUID bits (Don't allow root acquisition). */
#define INODE_STATE_NOEXEC    0x00008000 /*< Ignore execute permissions (Don't allow execute). */
#define INODE_STATE_SUPERMASK 0xffffffff /*< Mask of Inode state flags that propagate from an associated superblock. */
/* Additional INode state flags only applicable to superblocks. */
/*      INODE_STATE_SB_XXX    0x00010000 */

#if __SIZEOF_SIZE_T__ == 4
#   define LOCKRANGE_RMASK 0x7fffffff
#   define LOCKRANGE_WFLAG 0x80000000
#elif __SIZEOF_SIZE_T__ == 8
#   define LOCKRANGE_RMASK 0x7fffffffffffffffull
#   define LOCKRANGE_WFLAG 0x8000000000000000ull
#else
#   error FIXME
#endif

struct ilockrange {
#ifdef __INTELLISENSE__
 struct { struct ilockrange *le_next; }
#else
 SLIST_NODE(struct ilockrange)
#endif
        lr_chain; /*< [lock(:fl_lock)][sort(< lr_start)] Sorted chain of file-lock ranges. */
 pos_t  lr_start; /*< [lock(:fl_lock)] Starting address of this locking range. */
 pos_t  lr_size;  /*< [lock(:fl_lock)] Length of this lock range (in bytes). */
 size_t lr_locks; /*< [lock(:fl_lock)][!0] The effective locking mode of this range (active read/write locks).
                   *   NOTE: When 'LOCKRANGE_WFLAG' is set, the range is in write-mode. */
};

struct ifilelock {
 atomic_rwlock_t   fl_lock;   /*< Lock for managing filelock ranges. */
 SLIST_HEAD(struct ilockrange)
                   fl_ranges; /*< [0..1][lock(fl_lock)][owned] Chain of active file locks. */
 struct sig        fl_avail;  /*< Signal broadcast when a some portion of the file becomes available again. */
 struct sig        fl_unlock; /*< Signal broadcast when any kind of lock is released. */
};


struct ifile {
 /* INode file data. */
 atomic_rwlock_t         i_files_lock; /*< [order(AFTER(:i_attr_lock))] Lock for file data. */
 LIST_HEAD(struct file)  i_files;      /*< [lock(i_files_lock)] List of files opened on this INode. */
#define IFILE_MODULE_LOADING  ((struct module *)-1)
 struct module          *i_module;     /*< [0..1][lock(i_files_lock)] A module that can be loaded from this INode.
                                        *   WARNING: Set to 'IFILE_MODULE_LOADING' while the module is loading. */
 struct ifilelock        i_flock;      /*< File locking information. */
};
#define IFILE_FOREACH_OPEN(fp,o) LIST_FOREACH(fp,(o)->i_files,f_open)

struct inode {
#ifdef __INTELLISENSE__
 ref_t                   i_refcnt;    /*< Reference counter. */
 struct superblock      *i_super;     /*< [1..1] Associated superblock, or unreferenced self-pointer for superblocks them self.
                                       *   WARNING: In the event of a self-pointer, this field doesn't contain a reference. */
#else
 ATOMIC_DATA ref_t       i_refcnt;    /*< Reference counter. */
 REF struct superblock  *i_super;     /*< [1..1] Associated superblock, or unreferenced self-pointer for superblocks them self.
                                       *   WARNING: In the event of a self-pointer, this field doesn't contain a reference. */
#endif
 LIST_NODE(struct inode) i_nodes;     /*< [list(i_super->sb_nodes)][lock(i_super->sb_nodes_lock)]
                                       *   Per-superblock linked list of all loaded INodes.
                                       *   NOTE: The superblock itself is _NOT_ part of this
                                       *         list, but instead resembles an empty link. */
 struct inodeops const  *i_ops;       /*< [1..1][const] Additional INode operations & hardware I/O interfaces. */
 REF struct instance    *i_owner;     /*< [1..1][const] Owner module of this INode. */
union{
 ino_t                   i_ino;       /*< Per-filesystem, unique inode descriptor number. */
 uintptr_t               i_ino_id;    /*< INode ID that may be used by virtual filesystems. */
};
union{
 nlink_t                 __i_nlink;   /*< [lock(INODE_ISSUPERBLOCK(self)
                                       *      ? i_super->sb_mount.sm_mount_lock
                                       *      : i_super->[INTERNAL-DIFFERING]
                                       *        )]
                                       *   Amount of hard-links to this file, or amount of
                                       *   mounting points of this filesystem superblock. */
 nlink_t const           i_nlink;     /*< [ATOMIC_READ] Amount of hard-links to this file, or
                                       *   amount of mounting points of this filesystem superblock. */
};
 rwlock_t                i_attr_lock; /*< Lock for updating attributes. */
 struct iattr            i_attr;      /*< [lock(i_attr_lock)] Effective INode attributes. */
 struct iattr            i_attr_disk; /*< [lock(i_attr_lock)] INode attributes, as currently written on-disk.
                                       *   NOTE: Filesystem device files must not set the 'ia_mode' field of this
                                       *         to 'S_IFCHR' or 'S_IFBLK', but instead to any other type of mapping.
                                       *         A value of 'S_IFCHR' or 'S_IFBLK' inside this is used to identify
                                       *         device driver INodes themself, meaning that setting values as such
                                       *         may lead to kernel panic when code assumes that the inode can be
                                       *         casted to a 'struct device', 'struct blkdev' or 'struct chrdev'. */
 LIST_NODE(struct inode) i_attr_chng; /*< [list(i_super->sb_nodes)][lock(i_super->sb_nodes_lock)]
                                       *   Per-superblock linked list of all loaded INodes. */
 ATOMIC_DATA istate_t    i_state;     /*< INode state flags (Set of 'INODE_STATE_*'). */
 struct ifile            i_file;      /*< Open file data. */
union{
 dev_t                   i_devproxy;  /*< Device ID referenced by device-proxy INodes. */
 struct idata           *i_data;      /*< [?..?] Optional user-data (initialized to NULL) */
};
};

#define INODE_ISEFFECTIVE(self)  (!(self)->i_ops->ino_effective)

/* Replace 'self' with the effective INode.
 * NOTE: This function may replaces 'self' (a reference) with another reference.
 * @assume(BEFORE(IS_REFERENCE(self)));
 * @assume(AFTER(IS_REFERENCE(self))); */
#define INODE_GET_EFFECTIVE(self) \
do{ if (!INODE_ISEFFECTIVE(self)) { \
      struct inode *new_node; \
      new_node = (*(self)->i_ops->ino_effective)(self); \
      assert(new_node != NULL); \
      assert(E_ISOK(new_node)); \
      assert(new_node == self || INODE_ISEFFECTIVE(new_node)); \
      assert(new_node->i_refcnt > 0); \
      INODE_DECREF(self); \
      (self) = new_node; \
    } \
}while(0)

/* Create a new INode.
 * The caller must fill in:
 *  - i_super (Preferably using 'inode_setup')
 *  - i_owner (Preferably using 'inode_setup')
 *  - i_ops
 *  - i_nlink (Pre-initialized to ZERO(0))
 *  - i_attr
 *  - i_attr_disk
 *  - i_data (Optionally)
 */
#define inode_new(sizeof_type) \
        inode_cinit((struct inode *)calloc(1,sizeof_type))
FUNDEF struct inode *KCALL inode_cinit(struct inode *self);

/* Setup a given INode 'self' to be apart of 'sb'.
 * @return: -EOK:   Successfully added the new filesystem type.
 * @return: -EPERM: The module instance 'self->ap_owner' doesn't permit new references being created. */
FUNDEF WUNUSED errno_t KCALL inode_setup(struct inode *__restrict self,
                                         struct superblock *__restrict sb,
                                         struct instance *__restrict owner);

/* Invalidate all caches associated with the given INode. */
FUNDEF void KCALL inode_invalidate(struct inode *__restrict self);

/* Mark the given file-range as invalid, causing all open file streams to reload it.
 * @return: -EOK:    Successfully invalidated all open files within the given range.
 * @return: -ENOMEM: Not enough available memory.
 * @return: -EINTR:  The calling thread was interrupted. */
FUNDEF errno_t KCALL inode_invalidate_data(struct inode *__restrict self,
                                           pos_t start, pos_t size);

struct timespec;
/* Acquire a read/write locks on a given range within the specified INode.
 * WARNING: All errors returned 'inode_flock_upgrade', except for
 *         '-ENOMEM' will have released the previously held read-lock.
 * @return: -EOK:        Successfully acquired/upgraded a lock.
 * @return: -ENOMEM:     Failed to allocate controller blocks for locking ranges.
 * @return: -EAGAIN:    [abstime == INODE_FLOCK_TEST] Failed to immediately acquire a lock.
 * @return: -EINTR:     [abstime != INODE_FLOCK_TEST] The calling thread was interrupted.
 * @return: -ETIMEDOUT: [abstime != NULL && abstime != INODE_FLOCK_TEST] The specified 'abstime' has expired.
 * @return: -ERELOAD:   [inode_flock_upgrade][abstime != INODE_FLOCK_TEST]
 *                       No lock may have been held temporarily,
 *                       meaning that the caller should reload
 *                       local copies of affected resources.
 * @return: -ELOST:     [inode_flock_upgrade][abstime != INODE_FLOCK_TEST]
 *                       While attempting to upgrade a file lock it was
 *                       required to release the previously held read-lock.
 *                       But when later code attempted to acquire a new write-lock,
 *                       doing so failed because of insufficient memory.
 *                      (Based on meaning, this is the same as '-ENOMEM',
 *                       but in addition, the read-lock was lost) */
FUNDEF errno_t KCALL inode_flock_read(struct inode *__restrict ino, pos_t start, pos_t size, struct timespec const *abstime);
FUNDEF errno_t KCALL inode_flock_write(struct inode *__restrict ino, pos_t start, pos_t size, struct timespec const *abstime);
FUNDEF errno_t KCALL inode_flock_upgrade(struct inode *__restrict ino, pos_t start, pos_t size, struct timespec const *abstime);
#define INODE_FLOCK_TEST ((struct timespec const *)(uintptr_t)-1)

/* Downgrade a write-lock to a read-lock within the given range.
 * @return: -EOK:    Successfully downgraded the given range.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF errno_t KCALL inode_flock_downgrade(struct inode *__restrict ino, pos_t start, pos_t size);
/* Release a read/write lock within a specified range.
 * @return: -EOK:    Successfully downgraded the given range.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF errno_t KCALL inode_flock_endread(struct inode *__restrict ino, pos_t start, pos_t size);
FUNDEF errno_t KCALL inode_flock_endwrite(struct inode *__restrict ino, pos_t start, pos_t size);


/* NOTE: When destroying an INODE, changed attributes are written and warnings about  */
FUNDEF void KCALL inode_destroy(struct inode *__restrict self);
#define INODE_TRYINCREF(self)    ATOMIC_INCIFNONZERO((self)->i_refcnt)
#define INODE_INCREF(self)    (void)(ATOMIC_FETCHINC((self)->i_refcnt))
#define INODE_DECREF(self)    (void)(ATOMIC_DECFETCH((self)->i_refcnt) || (inode_destroy(self),0))

#define INODE_ISDIR(self)             S_ISDIR((self)->i_attr.ia_mode)
#define INODE_ISREG(self)             S_ISREG((self)->i_attr.ia_mode)
#define INODE_ISLNK(self)             S_ISLNK((self)->i_attr.ia_mode)
#define INODE_ISSUPERBLOCK(self)            (&(self)->i_super->sb_root == (self))
#define INODE_GTSUPERBLOCK(self)              (self)->i_super
#define INODE_TOSUPERBLOCK(self)             ((struct superblock *)(self))
#if INODE_STATE_SUPERMASK != 0xffffffff
#define INODE_GTSTATE(self)                   (ATOMIC_READ((self)->i_state)|(ATOMIC_READ((self)->i_super->sb_root.i_state)&INODE_STATE_SUPERMASK))
#else
#define INODE_GTSTATE(self)                   (ATOMIC_READ((self)->i_state)|ATOMIC_READ((self)->i_super->sb_root.i_state))
#endif
#define INODE_ISREADONLY(self)               (INODE_GTSTATE(self)&INODE_STATE_READONLY)
#define INODE_ISNOSUID(self)                 (INODE_GTSTATE(self)&INODE_STATE_NOSUID)
#define INODE_ISNOEXEC(self)                 (INODE_GTSTATE(self)&INODE_STATE_NOEXEC)
#define INODE_ISCLOSING(self)                (INODE_GTSTATE(self)&INODE_STATE_CLOSING)
#define INODE_ISREADONLY_OR_CLOSING(self)    (INODE_GTSTATE(self)&(INODE_STATE_READONLY|INODE_STATE_CLOSING))

/* Check if 'struct fsaccess *ac' has been granted access to 'self'.
 * @param: self:        The EFFECTIVE inode to query.
 * @param: ac:          Access permission UID/GID.
 * @param: rwx:         A set of 'R_OK|W_OK|X_OK' (from <unistd.h>), describing requested permissions.
 * @return: -EOK:       Access granted.
 * @return: -EACCES:    Access denied.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): A misc. error caused a failure. */
FUNDEF errno_t KCALL
inode_mayaccess(struct inode *__restrict self,
                struct fsaccess const *__restrict access,
                unsigned int rwx);

/* Opens a given INode using the specified open flags.
 * NOTE: Since unspecified, the root directory entry is used as dentry of the file stream.
 * @param: ino:         The EFFECTIVE inode to open.
 * @return: * :         A new reference to the newly opened file stream.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ENOENT:    The given 'dir_ent' has no associated INode.
 * @return: -EEXIST:   'oflags' contains 'O_CREAT|O_EXCL', but the file already existed.
 * @return: -ENOMEM:    Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR:  'dir_ent' is not a directory, or doesn't support a required directory interface.
 * @return: -ENOTDIR:  'oflags' contains 'O_DIRECTORY', but the file isn't a directory.
 * @return: -ENOSTR:    The INode associated with the specified file cannot be opened as a stream.
 * @return: -EROFS:    'oflags' contains 'O_CREAT' and the file didn't exist, or either 'O_WRONLY' or
 *                     'O_RDWR', but the INode or superblock associated with the file is read-only.
 * @return: -EBUSY:     The INode associated with the file to-be opened is marked for deletion.
 * @return: E_ISERR(*): Failed to open the INode for some reason. */
FUNDEF REF struct file *KCALL
inode_open(struct inode *__restrict ino,
           struct fsaccess const *__restrict access,
           struct iattr const *__restrict attr,
           iattrset_t attr_valid, oflag_t oflags);

LOCAL REF struct file *KCALL
inode_kopen(struct inode *__restrict ino, oflag_t oflags);


/* Generate stat()-information for the given INode.
 * @return: -EOK:       Successfully generated stat()-information and filled the given 'statbuf'.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): Failed to stat() the given INode for some reason. */
FUNDEF errno_t KCALL
inode_stat(struct inode *__restrict self,
           struct stat64 *__restrict statbuf);

/* Update all INode attributes marked in 'valid' to mirror values from 'attr'
 * @param: self:        The EFFECTIVE inode to update.
 * @return: -EOK:       Successfully updated INode attributes.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): A misc. error caused a failure.
 * NOTE: Unless 'valid&IATTR_NOW' is set, attribute
 *       changes are allowed to be performed ~later~.
 * NOTE: 'ia_siz' from 'attr' is ignored for directories.
 */
FUNDEF errno_t KCALL
inode_setattr(struct inode *__restrict self,
              struct iattr const *__restrict attr,
              iattrset_t valid);

/* Sync non-mirrored INode attributes and unbind
 * the node from its superblock's attr-change list.
 * @param: self: The EFFECTIVE inode to sync attributes of. */
FUNDEF errno_t KCALL
inode_syncattr(struct inode *__restrict self);

DECL_END

#ifndef __INTELLISENSE__
#include <fs/access.h>
DECL_BEGIN

LOCAL REF struct file *KCALL
inode_kopen(struct inode *__restrict ino, oflag_t oflags) {
 struct fsaccess ac;
 struct iattr ignored_attr;
 FSACCESS_SETHOST(ac);
 return inode_open(ino,&ac,&ignored_attr,IATTR_NONE,oflags);
}

DECL_END
#endif


#endif /* !GUARD_INCLUDE_FS_INODE_H */
