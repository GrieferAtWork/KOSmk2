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
#ifndef GUARD_INCLUDE_FS_SUPERBLOCK_H
#define GUARD_INCLUDE_FS_SUPERBLOCK_H 1

#include <errno.h>
#include <fs/inode.h>
#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>

DECL_BEGIN

struct superblock;
struct superblockops;
struct supermount;
struct dentry;

struct superblockops {
 /* Synchronize all buffers to mirror disk. */
 void    (KCALL *sb_fini)(struct superblock *__restrict sb);
 /* Synchronize internal superblock caches.
  * NOTE: Usually called after all changed INodes were synced. */
 errno_t (KCALL *sb_sync)(struct superblock *__restrict sb);
 /* Called after the last mounting point is deleted. */
 void (KCALL *sb_umount)(struct superblock *__restrict sb);
 /* Try to remove all hard links to a given INode.
  * NOTE: This operation must not be implemented by all filesystem
  *       types and should be set to NULL when not supported, yet
  *       is required for at least the /dev filesystem in order
  *       to delete devices as they get removed on-the-fly.
  * @return: * :         The amount of instances of 'node' that have been removed.
  * @return: E_ISERR(*): Failed to remove all instances for some reason. */
 ssize_t (KCALL *sb_remove_inode)(struct superblock *__restrict sb,
                                  struct inode *__restrict node);
};

struct supermount {
 LIST_NODE(struct superblock)      sm_chain;      /*< [lock(::fs_mountlock)] Chain entry for mounted superblocks. */
 atomic_rwlock_t                   sm_mount_lock; /*< R/W-lock for superblock mounting points. */
 REF struct dentry               **sm_mountv;     /*< [1..1][0..:sb_root.i_nlink][owned][lock(sm_mount_lock)] Vector of mounting points (Used to keep the superblock alive). */
};

struct superblock {
 /* NOTE: A superblock is always implicitly an INode. */
 struct inode            sb_root;       /*< [.i_super == self] Superblock root INode ('.i_super' isn't a reference). */
 struct superblockops const *sb_ops;    /*< [const][1..1] Additional, filesystem-specific superblock operations. */
 atomic_rwlock_t         sb_nodes_lock; /*< R/W-lock for INodes within this superblock. */
 LIST_HEAD(struct inode) sb_nodes;      /*< [lock(sb_nodes_lock)] Linked list of all nodes within this superblock (NOTE: Does not include 'sb_root'). */
 atomic_rwlock_t         sb_achng_lock; /*< [order(AFTER(inode::i_attr_lock))] R/W-lock for INodes with changed attributes. */
 LIST_HEAD(struct inode) sb_achng;      /*< [lock(sb_achng_lock)] Linked list of all INodes with changed attributes. */
 struct supermount       sb_mount;      /*< Superblock mounting point information. */
#ifdef __INTELLISENSE__
     struct blkdev      *sb_blkdev;     /*< [0..1][const] A bock device associated with this superblock (if any) */
#else
 REF struct blkdev      *sb_blkdev;     /*< [0..1][const] A bock device associated with this superblock (if any) */
#endif
};
#define SUPERBLOCK_INCREF(self) INODE_INCREF(&(self)->sb_root)
#define SUPERBLOCK_DECREF(self) INODE_DECREF(&(self)->sb_root)

DATDEF rwlock_t fs_mountlock; /*< Lock for the global mounting point list. */
DATDEF LIST_HEAD(struct superblock) fs_mountlist; /*< [lock(fs_mountlock)] Global mounting point list. */
#define FS_FOREACH_MOUNT(sblock) \
      LIST_FOREACH(sblock,fs_mountlist,sb_mount.sm_chain)

#define superblock_new(sizeof_type) \
        superblock_cinit((struct superblock *)calloc(1,sizeof_type))
FUNDEF SAFE struct superblock *KCALL superblock_cinit(struct superblock *self);

/* Finalize initialization of a given superblock.
 * @return: -EOK:   Successfully setup the given superblock.
 * @return: -EPERM: The module instance 'owner' doesn't permit new references being created. */
FUNDEF SAFE WUNUSED errno_t KCALL superblock_setup(struct superblock *__restrict self,
                                                   struct instance *__restrict owner);

/* Sync all data within the given superblock.
 * NOTE: When the superblock makes use of a block device,
 *       the block device will be synced as well!
 * @return: -EOK:       Successfully synced the superblock.
 * @return: E_ISERR(*): Failed to sync the superblock for some reason. */
FUNDEF errno_t KCALL superblock_sync(struct superblock *__restrict self);

/* Remove a given INode from from a superblock.
 * @requires(node->i_super == self);
 * @requires(node != &self->sb_root);
 * @return: * :         The amount of node instances removed from the superblock.
 *                      This number should be '<=' the old 'i_nlink' value of the
 *                      inode. - It is most likely equal to it, but in the event
 *                      that 2 threads call this function on the same node, at
 *                      the same time, one may remove all, or both may remove
 *                      apart of the node's links.
 *                   >> Upon success, the node's 'i_nlink' counter will equal ZERO(0),
 *                      a state that the filesystem driver should respect in a way
 *                      that will force it to allocate a new node and no longer use
 *                      the previous, meaning that following a call to this function,
 *                      the only thing still using 'node' are any remaining file
 *                      streams still making use of it.
 * @return: -EPERM:     The superblock doesn't support this feature.
 * @return: E_ISERR(*): Failed to remove all instances. */
FUNDEF ssize_t KCALL superblock_remove_inode(struct superblock *__restrict self,
                                             struct inode *__restrict node);


DECL_END

#endif /* !GUARD_INCLUDE_FS_SUPERBLOCK_H */
