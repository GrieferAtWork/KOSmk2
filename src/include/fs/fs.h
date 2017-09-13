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
#ifndef GUARD_INCLUDE_FS_FS_H
#define GUARD_INCLUDE_FS_FS_H 1

#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

#ifndef __iattrset_t_defined
#define __iattrset_t_defined 1
typedef u32 iattrset_t; /* Set of 'IATTR_*' */
#endif

struct superblock;
struct iattr;
struct dentry;
struct dentry_walker;
struct device;
struct inode;

/* Kernel option 'autosync=0/1': When enabled (!= 0), automatically
 * sync filesystems and block-devices after write-operations have
 * been performed (such as creating/deleting a file/dir/link, or
 * closing a file after some data has been written to it).
 * >> While this increases latency, as well as the amount ot data
 *    written to some storage device over time, it does greatly
 *    reduce the risk of data corruption or loss due to a system
 *    power outage, or simply unplugging some removable device
 *    without properly unmounting/syncing it.
 * NOTE: This option is enabled by default.
 * NOTE: Implementers of filesystems may ignore this flag. - The
 *       filesystem core API will automatically determine when
 *       changes (may) have been performed and will instigate
 *       sync operations when it deems them necessary. */
DATDEF __BOOL fs_autosync;


/* Opens a file, given its relative path name.
 * @return: * :         A new reference to a 'file' object.
 * @return: -ENOENT:    The file itself (when 'O_CREAT' isn't set), or a part of the file path doesn't exists.
 * @return: -EACCES:    The given permissions are insufficient.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ELOOP:     Too many symbolic link indirections.
 * @return: -EEXIST:   'oflags' contains 'O_CREAT|O_EXCL', but the file already existed.
 * @return: -ENOMEM:    Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR:   A part of the file's path isn't a directory, or
 *                      doesn't support a required directory interface.
 * @return: -ENOTDIR:  'oflags' contains 'O_DIRECTORY', but the file isn't a directory.
 * @return: -EROFS:    'oflags' contains 'O_CREAT' and the file didn't exist, but the
 *                      INode or superblock it resides within is marked as read-only.
 * @return: -EBUSY:     The INode associated with the file to-be opened is marked for deletion.
 * @return: E_ISERR(*): Failed to open the file for some reason. */
FUNDEF REF struct file *KCALL
fs_xopen(struct dentry_walker *__restrict walker,
         struct dentry *__restrict cwd,
         HOST char const *__restrict path, size_t pathlen,
         struct iattr const *__restrict attr,
         iattrset_t attr_valid, oflag_t oflags);
FUNDEF REF struct file *KCALL
fs_user_xopen(struct dentry_walker *__restrict walker,
              struct dentry *__restrict cwd, USER char const *path,
              struct iattr const *__restrict attr,
              iattrset_t attr_valid, oflag_t oflags);

/* Insert the given device as a node, given its relative path name.
 * NOTE: The underlying INode of the device may not be the INode actually
 *       loaded into the filesystem. - A proxy node may be used instead.
 * @return: * :         A new reference to the directory entry of the inserted node.
 * @return: -ENOENT:    A part of the file path doesn't exists.
 * @return: -EACCES:    The given permissions are insufficient.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ELOOP:     Too many symbolic link indirections.
 * @return: -EEXIST:    A node already exists under the specified name.
 * @return: -ENOMEM:    Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR:   A part of the file's path isn't a directory, or
 *                      doesn't support a required directory interface.
 * @return: E_ISERR(*): Failed to insert the node for some reason. */
FUNDEF REF struct dentry *KCALL
fs_xinsnod(struct dentry_walker *__restrict walker,
           struct dentry *__restrict cwd,
           HOST char const *__restrict path, size_t pathlen,
           struct device *__restrict dev,
           REF struct inode **result_inode);
FUNDEF REF struct dentry *KCALL
fs_user_xinsnod(struct dentry_walker *__restrict walker,
                struct dentry *__restrict cwd, USER char const *path,
                struct device *__restrict dev,
                REF struct inode **result_inode);

/* Create a new empty regular-file.
 * NOTE: The underlying INode of the device may not be the INode actually
 *       loaded into the filesystem. - A proxy node may be used instead.
 * @param: result_inode: When non-NULL, store a reference to the INode associated
 *                       with the returned directory entry in '*result_inode'.
 *                       HINT: When NULL, the internally generated INode reference is dropped.
 * @return: * :         A reference to the DENTRY of the newly created file.
 * @return: -ENOENT:    A part of the file path doesn't exists.
 * @return: -EACCES:    The given permissions are insufficient.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ELOOP:     Too many symbolic link indirections.
 * @return: -ENOMEM:    Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR:   A part of the file's path isn't a directory, or
 *                      doesn't support a required directory interface.
 * @return: -EEXIST:    Another entry with the same name already exists.
 * @return: -EROFS:     The INode or superblock associated with 'dir_ent' is read-only.
 * @return: E_ISERR(*): Failed to create a regular file for some reason. */
FUNDEF REF struct dentry *KCALL
fs_xmkreg(struct dentry_walker *__restrict walker,
          struct dentry *__restrict cwd,
          HOST char const *__restrict path, size_t pathlen,
          struct iattr const *__restrict attr,
          REF struct inode **result_inode);
FUNDEF REF struct dentry *KCALL
fs_user_xmkreg(struct dentry_walker *__restrict walker,
               struct dentry *__restrict cwd, USER char const *path,
               struct iattr const *__restrict attr,
               REF struct inode **result_inode);

/* Create a new empty directory.
 * NOTE: 'ia_siz' from 'attr' is ignored.
 * $ mkdir "/opt/foo" # dentry_mkdir(DENTRY("/opt"),"foo",...);
 * @param: result_inode: When non-NULL, store a reference to the INode associated
 *                       with the returned directory entry in '*result_inode'.
 *                       HINT: When NULL, the internally generated INode reference is dropped.
 * @return: * :         A reference to the DENTRY of the newly created directory.
 * @return: -ENOENT:    A part of the file path doesn't exists.
 * @return: -EACCES:    The given permissions are insufficient.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ELOOP:     Too many symbolic link indirections.
 * @return: -ENOMEM:    Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR:   A part of the file's path isn't a directory, or
 *                      doesn't support a required directory interface.
 * @return: -EEXIST:    Another entry with the same name already exists.
 * @return: -EROFS:     The INode or superblock associated with 'dir_ent' is read-only.
 * @return: E_ISERR(*): Failed to create a directory for some reason. */
FUNDEF REF struct dentry *KCALL
fs_xmkdir(struct dentry_walker *__restrict walker,
          struct dentry *__restrict cwd,
          HOST char const *__restrict path, size_t pathlen,
          struct iattr const *__restrict attr,
          REF struct inode **result_inode);
FUNDEF REF struct dentry *KCALL
fs_user_xmkdir(struct dentry_walker *__restrict walker,
               struct dentry *__restrict cwd, USER char const *path,
               struct iattr const *__restrict attr,
               REF struct inode **result_inode);


/* Create a hard link.
 * @return: * :         A reference to the DENTRY of the newly created link.
 * @return: -ENOENT:    A part of the file path doesn't exists.
 * @return: -EACCES:    The given permissions are insufficient.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ELOOP:     Too many symbolic link indirections.
 * @return: -ENOMEM:    Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR:   A part of the file's path isn't a directory, or
 *                      doesn't support a required directory interface.
 * @return: -EPERM:     The INode associated with 'dir_ent' doesn't support hard links.
 * @return: -EROFS:     The INode or superblock associated with 'dir_ent' is read-only.
 * @return: -EXDEV:     The given 'dst_node' is apart of a different superblock than that of 'dir_ent'
 * @return: E_ISERR(*): Failed to create a hard link for some reason. */
FUNDEF REF struct dentry *KCALL
fs_xhrdlink(struct dentry_walker *__restrict walker,
            struct dentry *__restrict cwd,
            HOST char const *__restrict path, size_t pathlen,
            struct inode *__restrict dst_node);
FUNDEF REF struct dentry *KCALL
fs_user_xhrdlink(struct dentry_walker *__restrict walker,
                 struct dentry *__restrict cwd, USER char const *path,
                 struct inode *__restrict dst_node);

/* Create a symbolic link.
 * @param: result_inode: When non-NULL, store a reference to the INode associated
 *                       with the returned directory entry upon success.
 * @return: * :         A reference to the DENTRY of the newly created link.
 * @return: -ENOENT:    A part of the file path doesn't exists.
 * @return: -EACCES:    The given permissions are insufficient.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ELOOP:     Too many symbolic link indirections.
 * @return: -ENOMEM:    Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR:   A part of the file's path isn't a directory, or
 *                      doesn't support a required directory interface.
 * @return: -EPERM:     The effective INode associated doesn't support symbolic links.
 * @return: -EROFS:     The effective INode or superblock associated is read-only.
 * @return: E_ISERR(*): Failed to create a directory for some reason. */
FUNDEF REF struct dentry *KCALL
fs_xsymlink(struct dentry_walker *__restrict walker,
            struct dentry *__restrict cwd,
            HOST char const *__restrict path, size_t pathlen,
            USER char const *target_text,
            struct iattr const *__restrict result_attr,
            REF struct inode **result_inode);
FUNDEF REF struct dentry *KCALL
fs_user_xsymlink(struct dentry_walker *__restrict walker,
                 struct dentry *__restrict cwd,
                 USER char const *path,
                 USER char const *target_text,
                 struct iattr const *__restrict result_attr,
                 REF struct inode **result_inode);


/* Rename a given file 'existing_ent' to exist under a new name 'dst_name' within 'dst_dir'.
 * @param: result_inode: When non-NULL, store a reference to the returned INode.
 * @return: * :         A reference to the DENTRY of the newly created link.
 * @return: -ENOENT:    A part of the file path doesn't exists.
 * @return: -EACCES:    The given permissions are insufficient.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ELOOP:     Too many symbolic link indirections.
 * @return: -ENOMEM:    Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR:   A part of the file's path isn't a directory, or
 *                      doesn't support a required directory interface.
 * @return: -EPERM:     The INode associated with the target directory doesn't support renaming.
 * @return: -EROFS:     The target/source INode is marked as read-only or part of a read-only superblock.
 * @return: -EXDEV:     The target path is part of a different superblock than that of 'existing_ent'
 * @return: E_ISERR(*): Failed to create a directory for some reason. */
FUNDEF REF struct dentry *KCALL
fs_xrename(struct dentry_walker *__restrict walker,
           struct dentry *__restrict cwd,
           HOST char const *__restrict path, size_t pathlen,
           struct dentry *__restrict existing_ent,
           REF struct inode **result_inode);
FUNDEF REF struct dentry *KCALL
fs_user_xrename(struct dentry_walker *__restrict walker,
                struct dentry *__restrict cwd, USER char const *path,
                struct dentry *__restrict existing_ent,
                REF struct inode **result_inode);


/* Virtually mount the given superblock, given its relative path name.
 * @return: * :       A new reference to the dentry in which 'filesystem' got mounted.
 * @return: -ENOENT:  A part of the file path doesn't exists.
 * @return: -EACCES:  The given permissions are insufficient.
 * @return: -EINTR:   The calling thread was interrupted.
 * @return: -ELOOP:   Too many symbolic link indirections.
 * @return: -ENOMEM:  Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR: A part of the file's path isn't a directory, or
 *                    doesn't support a required directory interface. */
FUNDEF REF struct dentry *KCALL
fs_xmount(struct dentry_walker *__restrict walker,
          struct dentry *__restrict cwd,
          char const *__restrict path, size_t pathlen,
          struct superblock *__restrict filesystem);

/* TODO: fs_x* wrappers for all other dentry-functions. */


/* Clear unused filesystem memory buffers, releasing unused
 * memory and making it re-available for other uses. */
FUNDEF void KCALL fs_freeunused(void);

/* The kernel core filesystem root. */
DATDEF struct dentry fs_root;

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Mount the root filesystem:
 * >> dentry_mount(&fs_root,blkdev_mksuper(get_bootpart()));
 * WARNING: This function is an init-call and must not
 *          be called once free-data has been released! */
INTDEF void KCALL mount_root_filesystem(void);
#endif


/* A simplified set of FS-function for use by the kernel itself. */
LOCAL REF struct file *KCALL fs_kopen(char const *__restrict path, size_t pathlen, struct iattr const *attr, iattrset_t attr_valid, oflag_t oflags);
LOCAL REF struct dentry *KCALL fs_kinsnod(char const *__restrict path, size_t pathlen, struct device *__restrict dev, REF struct inode **result_inode);
LOCAL REF struct dentry *KCALL fs_kmount(char const *__restrict path, size_t pathlen, struct superblock *__restrict filesystem);

#define kopen(path,oflags) fs_kopen(path,strlen(path),NULL,IATTR_NONE,oflags)

DECL_END


#ifndef __INTELLISENSE__
#include <fs/access.h>
#include <fs/inode.h>
DECL_BEGIN

LOCAL REF struct file *KCALL
fs_kopen(char const *__restrict path, size_t pathlen,
         struct iattr const *attr,
         iattrset_t attr_valid, oflag_t oflags) {
 struct iattr fix_attr; struct dentry_walker walker;
 DENTRY_WALKER_SETKERNEL(walker);
 if (!attr) { attr = &fix_attr; attr_valid = IATTR_NONE; }
 return fs_xopen(&walker,&fs_root,path,pathlen,attr,attr_valid,oflags);
}
LOCAL REF struct dentry *KCALL
fs_kinsnod(char const *__restrict path, size_t pathlen,
           struct device *__restrict dev,
           REF struct inode **result_inode) {
 struct dentry_walker walker;
 DENTRY_WALKER_SETKERNEL(walker);
 return fs_xinsnod(&walker,&fs_root,path,pathlen,dev,result_inode);
}
LOCAL REF struct dentry *KCALL
fs_kmount(char const *__restrict path, size_t pathlen,
          struct superblock *__restrict filesystem) {
 struct dentry_walker walker;
 DENTRY_WALKER_SETKERNEL(walker);
 return fs_xmount(&walker,&fs_root,path,pathlen,filesystem);
}

DECL_END
#endif

#endif /* !GUARD_INCLUDE_FS_FS_H */
