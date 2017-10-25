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
#ifndef GUARD_INCLUDE_FS_DENTRY_H
#define GUARD_INCLUDE_FS_DENTRY_H 1

#include <errno.h>
#include <fcntl.h>
#include <fs/basic_types.h>
#include <hybrid/atomic.h>
#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>

DECL_BEGIN

struct dentry;
struct dentry_walker;
struct dentryname;
struct device;
struct fsaccess;
struct iattr;
struct superblock;

/* Fill the 'dn_hash' field of the given directory entry name. */
FUNDEF void KCALL dentryname_loadhash(struct dentryname *__restrict self);

struct dhashtab {
 /* Directory entry hash table. */
 size_t          ht_tabc; /*< Tracked amount of cached child entries. */
 size_t          ht_taba; /*< Allocated amount of cached child entries. */
 struct dentry **ht_tabv; /*< [0..1][owned|owned(->d_next)][0..ht_taba][owned]
                           *  [index = ->d_name.dn_hash % ht_taba]
                           *   Hash-table of child directory entries. */
};
#define DHASHTAB_SHOULDREHASH(self) (((self)->ht_tabc*3)/2 > (self)->ht_taba)

struct dentry {
 ref_t              d_refcnt; /*< Reference counter. */
 struct dentryname  d_name;   /*< [const] Directory entry name. */
#ifdef __INTELLISENSE__
     struct inode  *d_inode;  /*< [0..1][lock(d_inode_lock)][SET(ONCE)|CLEAR(AFTER)]
                               *   The effective INode associated with this directory entry. */
#else
 REF struct inode  *d_inode;  /*< [0..1][lock(d_inode_lock)][SET(ONCE)|CLEAR(AFTER)]
                               *   The effective INode associated with this directory entry. */
#endif
 atomic_rwlock_t    d_inode_lock;/*< R/W-lock for the associated INode. */
 REF struct dentry *d_parent; /*< [const][0..1] Parent directory entry (Only NULL for 'fs_root'). */
 /* Parent-child directory hashing... */
 atomic_rwlock_t    d_subs_lock;/*< R/W-lock for the associated INode. */
 struct dentry     *d_next;   /*< [lock(d_parent->d_subs_lock)][0..1] Next directory entry with the same hash ('d_name.dn_hash'). */
 struct dhashtab    d_subs;   /*< [lock(d_subs_lock)] Known child directory entries + virtual children.
                               *   NOTE: Once unlocked through 'd_subs_lock', this hash-map must not
                               *         contain directory entries with a 'd_inode' set to NULL. */
 REF LIST_NODE(struct dentry)
                    d_cache;  /*< [0..1][lock(INTERNAL(dentry_cache_lock))] Internal chain of cached directory entries. - Don't touch! */
};

#define dentry_new() \
        dentry_init((struct dentry *)calloc(1,sizeof(struct dentry)))
FUNDEF struct dentry *KCALL dentry_init(struct dentry *self);

FUNDEF void KCALL dentry_destroy(struct dentry *__restrict self);
#define DENTRY_INCREF(self) (void)(ATOMIC_FETCHINC((self)->d_refcnt))
#define DENTRY_DECREF(self) (void)(ATOMIC_DECFETCH((self)->d_refcnt) || (dentry_destroy(self),0))

/* Mark the given directory as recently used, potentially adding it to a
 * limited set of entries manually kept alive to improve access time.
 * NOTE: This function should only be called for top-level directories.
 *       Calling it on something like "/" is just a waste of resources. */
FUNDEF void KCALL dentry_used(struct dentry *__restrict self);
/* The reverse of 'dentry_used()'; called when the directory entry is removed. */
FUNDEF void KCALL dentry_unused(struct dentry *__restrict self);

/* Clear the global cache of directory entries. */
FUNDEF size_t KCALL dentry_clearcache(void); /* Return number of removed cache slots. */
FUNDEF size_t KCALL dentry_clearcache_freemem(void); /* Return number of freed bytes & try-acquire locks. */


/* Safely return a new reference to the INode
 * associated with the given directory entry. */
LOCAL REF struct inode *KCALL dentry_inode(struct dentry *__restrict self);

/* Clear the INode associated with a given directory entry. */
LOCAL void KCALL dentry_clsnode(struct dentry *__restrict self);




/* Walk the given directory entry, searching for a child
 * entry and returning a reference to it when found.
 * $ cd "/opt/my_path" # dentry_open(DENTRY("/"),"opt",...);
 *                     # dentry_open(DENTRY("/opt"),"my_path",...);
 * @param: always_follow_links: When TRUE, always follow symbolic links,
 *                              disregardless of what 'walker->dw_nofollow'
 *                              may want to. (Set to true to follow links
 *                              in all intermediate path components when
 *                              walking a cross-directory filename)
 * >> This is where the difference between the following comes to effect:
 *    $ ln -s /  /dev/foo
 *    $ ls -la /dev/foo  # Only prints the symlink "/dev/foo"
 *    $ ls -la /dev/foo/ # Prints the entire directory "/"
 * @return: * :         A new reference to a 'dentry' object.
 * @return: -ENOENT:    The given 'dir_ent' has no associated INode.
 * @return: -ELOOP:     Too many symbolic link indirections.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): Failed to walk the given directory entry for some reason. */
FUNDEF REF struct dentry *KCALL
dentry_walk(struct dentry *__restrict self,
            struct dentry_walker *__restrict walker,
            struct dentryname const *__restrict name,
            bool always_follow_links);

/* Same as 'dentry_walk', but also recognize
 * special directory names, such as '.' and  '..' */
FUNDEF REF struct dentry *KCALL
dentry_walk2(struct dentry *__restrict self,
             struct dentry_walker *__restrict walker,
             struct dentryname const *__restrict name,
             bool always_follow_links);

/* Walk all components of the given path, respecting special
 * path names such as '.' and '..', as well as splitting the
 * path at every occurrence of a '/' character.
 * NOTE: Passing ZERO(0) for 'path_len' will not change the directory.
 * NOTE: Multiple consecutive '/'-characters are ignored.
 * @return: * :         A new reference to a 'dentry' object.
 * @return: -ENOENT:    The given 'dir_ent' has no associated INode.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -ELOOP:     Too many symbolic link indirections.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): Failed to walk the given directory entry for some reason. */
FUNDEF REF struct dentry *KCALL
dentry_xwalk(struct dentry *__restrict self,
             struct dentry_walker *__restrict walker,
             HOST char const *__restrict path_str, size_t path_len);
FUNDEF REF struct dentry *KCALL
dentry_xwalk_internal(struct dentry *__restrict self,
                      struct dentry_walker *__restrict walker,
                      HOST char const *__restrict path_str, size_t path_len);

/* Same as 'dentry_xwalk()', but walk a given user-string instead. */
FUNDEF REF struct dentry *KCALL
dentry_user_xwalk(struct dentry *__restrict self,
                  struct dentry_walker *__restrict walker,
                  USER char const *path);

/* Perform the last step of opening/creating a file in 'dir_ent' named 'ent_name'
 * $ cat "/opt/my_file" # dentry_open(DENTRY("/opt"),"my_file",...);
 * NOTE: This function handles the case of 'oflags&O_CREAT' to create a new regular file.
 * @return: * :         A new reference to a 'file' object.
 * @return: -EACCES:   'walker->dw_access' describes insufficient permissions.
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
 * @return: E_ISERR(*): Failed to open the given file for some reason. */
FUNDEF REF struct file *KCALL
dentry_open(struct dentry *__restrict dir_ent,
            struct dentryname const *__restrict ent_name,
            struct dentry_walker *__restrict walker,
            struct iattr const *__restrict attr,
            iattrset_t attr_valid, oflag_t oflags);
PUBLIC REF struct file *KCALL
dentry_open_with_inode(struct dentry *__restrict dir_ent,
                   REF struct inode *__restrict res_ino,
                       struct fsaccess const *__restrict access,
                       struct iattr const *__restrict attr,
                       iattrset_t attr_valid, oflag_t oflags);

/* Open an existing file within 'dir_ent'
 * @return: * :         A new reference to a 'file' object.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ENOENT:    The given 'dir_ent' has no associated INode.
 * @return: -EEXIST:   'oflags' contains 'O_CREAT|O_EXCL'.
 * @return: -ENOMEM:    Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR:  'oflags' contains 'O_DIRECTORY', but 'dir_ent' isn't a directory.
 * @return: -ENOSTR:    The INode associated with the given 'dir_ent' cannot be opened as a stream.
 * @return: -EROFS:    'oflags' contains either 'O_WRONLY' or 'O_RDWR', but the
 *                      INode or superblock associated with 'dir_ent' is read-only.
 * @return: -EBUSY:     The INode associated with the file to-be opened is marked for deletion.
 * @return: E_ISERR(*): Failed to open the given file for some reason. */
FUNDEF REF struct file *KCALL
dentry_openthis(struct dentry *__restrict dir_ent,
                struct fsaccess const *__restrict access,
                struct iattr const *__restrict attr,
                iattrset_t attr_valid, oflag_t oflags);

/* Create an empty, regular file 'ent_name' within 'dir_ent'
 * $ > /opt/foo # dentry_mkreg(DENTRY("/opt"),"foo",...);
 * @param: result_inode: When non-NULL, store a reference to the INode associated
 *                       with the returned directory entry in '*result_inode'.
 *                       HINT: When NULL, the internally generated INode reference is dropped.
 * @return: * :         A reference to the DENTRY of the newly created file.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -EEXIST:    Another entry with the same name already exists.
 * @return: -ENOENT:    The given 'dir_ent' has no associated INode.
 * @return: -EROFS:     The INode or superblock associated with 'dir_ent' is read-only.
 * @return: E_ISERR(*): Failed to create a regular file for some reason. */
FUNDEF REF struct dentry *KCALL
dentry_mkreg(struct dentry *__restrict dir_ent,
             struct dentryname const *__restrict ent_name,
             struct fsaccess const *__restrict access,
             struct iattr const *__restrict attr,
             REF struct inode **result_inode);

/* Create a new directory 'ent_name' within 'dir_ent'
 * NOTE: 'ia_siz' from `attr' is ignored.
 * $ mkdir "/opt/foo" # dentry_mkdir(DENTRY("/opt"),"foo",...);
 * @param: result_inode: When non-NULL, store a reference to the INode associated
 *                       with the returned directory entry in '*result_inode'.
 *                       HINT: When NULL, the internally generated INode reference is dropped.
 * @return: * :         A reference to the DENTRY of the newly created directory.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -EEXIST:    Another entry with the same name already exists.
 * @return: -ENOENT:    The given 'dir_ent' has no associated INode.
 * @return: -EROFS:     The INode or superblock associated with 'dir_ent' is read-only.
 * @return: E_ISERR(*): Failed to create a directory for some reason. */
FUNDEF REF struct dentry *KCALL
dentry_mkdir(struct dentry *__restrict dir_ent,
             struct dentryname const *__restrict ent_name,
             struct fsaccess const *__restrict access,
             struct iattr const *__restrict attr,
             REF struct inode **result_inode);

/* Insert a given INode 'node' into the directory 'dir_ent' under 'ent_name'.
 * NOTE: The create directory entry is allocated virtually, meaning that
 *       the underlying filesystem of 'dir_ent' will not be informed of this.
 * WARNING: The inserted node is only guarantied to remain accessible
 *          until the caller drops all references from the returned pointer.
 *          With that in mind, the call should keep the returned directory
 *          around until a time when they wish to remove the node.
 * @return: * :         A reference to the DENTRY of the newly created node.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -EEXIST:    Another entry with the same name already exists.
 * @return: E_ISERR(*): Failed to create a directory for some reason. */
FUNDEF REF struct dentry *KCALL
dentry_insnod(struct dentry *__restrict dir_ent,
              struct dentryname const *__restrict ent_name,
              struct fsaccess const *__restrict access,
              struct device *__restrict dev,
              REF struct inode **result_inode);

/* Mount the given 'filesystem' within `self'.
 * @return: -EOK:     Successfully mounted the given filesystem.
 * @return: -EACCES: 'access' describes insufficient permissions.
 * @return: -EINTR:   The calling thread was interrupted.
 * @return: -ENOTDIR: The old INode associated with `self' wasn't a directory, or was another superblock.
 */
FUNDEF errno_t KCALL
dentry_mount(struct dentry *__restrict self,
             struct fsaccess const *__restrict access,
             struct superblock *__restrict filesystem);

/* Mirror all attributes from `attr' marked by 'valid'
 * NOTE: 'ia_siz' from `attr' is ignored for directories.
 * $ chmod 0777 "/opt/foo" # dentry_setattr(DENTRY("/opt/foo"),...);
 * @return: -EOK:       Successfully updated INode attributes.
 * @return: -ENOENT:    The given `self' has no associated INode.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -EROFS:     The INode or superblock associated with `self' is read-only.
 * @return: E_ISERR(*): Failed to set the given attributes for some reason. */
FUNDEF errno_t KCALL
dentry_setattr(struct dentry *__restrict self,
               struct fsaccess const *__restrict access,
               struct iattr const *__restrict attr,
               iattrset_t valid);

/* Read a symbol link located at `self'.
 * @return: * :         The required buffer size.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ENOENT:    The given `self' has no associated INode.
 * @return: -EFAULT:    A faulty buffer pointer was provided.
 * @return: E_ISERR(*): Failed to read a link for some reason. */
FUNDEF ssize_t KCALL
dentry_readlink(struct dentry *__restrict self,
                struct fsaccess const *__restrict access,
                USER char *__restrict buf, size_t bufsize);

/* Create a hard link directory entry 'ent_name' in the directory 'dir_ent',
 * pointing at the INode associated with the provided 'dst_node'.
 * @return: * :         A new reference to a 'dentry' for 'ent_name' inside of 'dir_ent'.
 * @return: -ENOENT:    The given 'dir_ent' has no associated INode.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ENOTDIR:   The given directory entry 'dir_ent' isn't a directory itself.
 * @return: -EPERM:     The INode associated with 'dir_ent' doesn't support hard links.
 * @return: -EROFS:     The INode or superblock associated with 'dir_ent' is read-only.
 * @return: -EXDEV:     The given 'dst_node' is apart of a different superblock than that of 'dir_ent'
 * @return: E_ISERR(*): Failed to create a hard link for some reason. */
FUNDEF REF struct dentry *KCALL
dentry_hrdlink(struct dentry *__restrict dir_ent,
               struct dentryname const *__restrict ent_name,
               struct fsaccess const *__restrict access,
               struct inode *__restrict dst_node);

/* Create a symbolic link 'ent_name' within 'dir_ent' containing
 * the text described by 'target_text'.
 * @param: result_inode: When non-NULL, store a reference to the INode associated
 *                       with the returned directory entry upon success.
 * @return: * :         A reference to the directory now apart of 'dir_ent'
 * @return: -ENOENT:    The given `self' has no associated INode.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ENOTDIR:   The given directory entry 'dir_ent' isn't a directory itself.
 * @return: -EPERM:     The INode associated with 'dir_ent' doesn't support symbolic links.
 * @return: -EROFS:     The INode or superblock associated with 'dir_ent' is read-only.
 * @return: E_ISERR(*): Failed to create a directory for some reason. */
FUNDEF REF struct dentry *KCALL
dentry_symlink(struct dentry *__restrict dir_ent,
               struct dentryname const *__restrict ent_name,
               struct fsaccess const *__restrict access,
               USER char const *target_text,
               struct iattr const *__restrict result_attr,
               REF struct inode **result_inode);


/* Rename a given file 'existing_ent' to exist under a new name 'dst_name' within 'dst_dir'.
 * @param: result_inode: When non-NULL, store a reference to the returned INode.
 * @return: * :          A reference to the directory now apart of 'dst_dir'.
 * @return: -ENOENT:     The given 'dst_dir' or 'existing_ent' has no associated INode.
 * @return: -EACCES:    'access' describes insufficient permissions.
 * @return: -EINTR:      The calling thread was interrupted.
 * @return: -ENOTDIR:    The given directory entry 'dst_dir' isn't a directory itself.
 * @return: -EPERM:      The INode associated with 'dst_dir' doesn't support renaming.
 * @return: -EROFS:      The INode or superblock associated with 'dst_dir' or 'existing_ent' is read-only.
 * @return: -EXDEV:      The INode of 'dst_dir' is part of a different superblock than that of 'existing_ent'.
 * @return: E_ISERR(*):  Failed to create a directory for some reason. */
FUNDEF REF struct dentry *KCALL
dentry_rename(struct dentry *__restrict dst_dir,
              struct dentryname const *__restrict dst_name,
              struct fsaccess const *__restrict access,
              struct dentry *__restrict existing_ent,
              REF struct inode **result_inode);


/* Remove files or empty directories.
 * @param: mode: A set of 'DENTRY_REMOVE_*'
 * @return: -ENOENT:    The given directory entry `self' has no associated INode.
 * @return: -EACCES:   'access' describes insufficient permissions.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -EPERM:     The INode associated with the directory of `self' doesn't support removing.
 *                      The directory associated with `self' is part of a different superblock.
 * @return: -EISDIR:    The INode is a directory/mounting point, but 'DENTRY_REMOVE_DIR'/'DENTRY_REMOVE_MNT' wasn't specified
 * @return: -EROFS:     The INode or superblock associated with `self' is read-only.
 * @return: E_ISERR(*): Failed to remove the given directory entry for some reason. */
FUNDEF errno_t KCALL
dentry_remove(struct dentry *__restrict self,
              struct fsaccess const *__restrict access,
              u32 mode);
#define DENTRY_REMOVE_DIR AT_REMOVEDIR /*< Remove directories. */
#define DENTRY_REMOVE_REG AT_REMOVEREG /*< Remove regular files. */
#define DENTRY_REMOVE_MNT AT_REMOVEMNT /*< Remove mounting points. */

#define dentry_unlink(self,access) dentry_remove(self,access,DENTRY_REMOVE_REG)
#define dentry_rmdir(self,access)  dentry_remove(self,access,DENTRY_REMOVE_DIR)
#define dentry_umount(self,access) dentry_remove(self,access,DENTRY_REMOVE_MNT)

DECL_END

#ifndef __INTELLISENSE__
#include <fs/inode.h>
DECL_BEGIN

LOCAL REF struct inode *KCALL
dentry_inode(struct dentry *__restrict self) {
 struct inode *result;
 assert(self);
 atomic_rwlock_read(&self->d_inode_lock);
 if ((result = self->d_inode) != NULL)
      INODE_INCREF(result);
 atomic_rwlock_endread(&self->d_inode_lock);
 return result;
}

LOCAL void KCALL
dentry_clsnode(struct dentry *__restrict self) {
 struct inode *result;
 assert(self);
 atomic_rwlock_read(&self->d_inode_lock);
 result = self->d_inode;
 self->d_inode = NULL;
 atomic_rwlock_endread(&self->d_inode_lock);
 if (result) INODE_DECREF(result);
}

DECL_END
#endif

#endif /* !GUARD_INCLUDE_FS_DENTRY_H */
