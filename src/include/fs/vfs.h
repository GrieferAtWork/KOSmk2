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
#ifndef GUARD_INCLUDE_FS_VFS_H
#define GUARD_INCLUDE_FS_VFS_H 1

#include <fs/basic_types.h>
#include <fs/file.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <sys/stat.h>

DECL_BEGIN

/* Virtual filesystem data structures. */
struct vnode_dirent {
 struct dentryname d_name; /*< [const][owned] Node entry name. */
 REF struct inode *d_node; /*< [1..1][const] INode associated with this directory entry (Can be anything).
                            *   NOTE: This pointer also holds a reference to 'd_node->i_nlink' */
};

#define VDATA_DYNAMIC 0x00000000 /*< Set when data is dynamically allocated, rather than statically. */
#define VDATA_STATIC  0x00000001 /*< Set when the VNode entry vector is allocated statically. */
#define VDATA_ISSTATIC(x)    ((x)&VDATA_STATIC)
#define VDATA_ISDYNAMIC(x) (!((x)&VDATA_STATIC))
typedef u32 vflag_t; /*< A set of 'VDATA_*' */

struct vnode_common {
 REF LIST_NODE(struct inode) v_entry; /*< [0..1][lock(vd_node->i_super->v_vlock)]
                                       *  Chain of existing virtual INodes within the associated vsuperblock.
                                       *  >> Used to keep virtually allocated INodes alive, even */
 vflag_t                     v_flag;  /*< A set of 'VDATA_*'. */
};

struct vnode_data {
 struct vnode_common  v_common; /*< Common V-node data header. */
 atomic_rwlock_t      v_lock;   /*< Lock for the virtual directory. */
 size_t               v_entc;   /*< [lock(vf_lock)] Amount of directory entries. */
 struct vnode_dirent *v_entv;   /*< [0..vf_entc][owned_if(VDATA_ISDYNAMIC)][lock(vf_lock)] Vector of directory entries. */
};

struct vfile_dir {
 struct file       vf_file; /*< Underlying file (NOTE: 'f_node->i_data' points to a 'struct vnode_data' object). */
 size_t            vf_didx; /*< [lock(vf_file.f_lock)] Current virtual directory index. (f_node->i_data->vf_entv) */
};
struct vnode {
 struct inode      v_node;  /*< Underlying INode ('i_data' points at '&v_data') */
 struct vnode_data v_data;  /*< Virtual directory data. */
};

/* Virtual, symbolic link structures. */
struct vlink_data {
 struct vnode_common  v_common; /*< Common V-node data header. */
 size_t               v_size;   /*< [const] Size of the link text (Including the terminating \0-character). */
 HOST char const     *v_text;   /*< [0..vl_size][owned_if(VDATA_ISDYNAMIC)][const] Vector of directory entries. */
};
struct vlink {
 struct inode      v_node; /*< Underlying INode ('i_data' points at '&v_data') */
 struct vlink_data v_data; /*< Virtual link data. */
};

struct vdev_data {
 struct vnode_common v_common; /*< Common V-node data header. */
 dev_t               v_device; /*< [const] device id. (Of same chr/blk-class as the associated node) */
};
struct vdev {
 struct inode     v_node; /*< Underlying INode ('i_data' points at '&v_data') */
 struct vdev_data v_data; /*< Virtual device data. */
};



#define VSUPERBLOCK_DEFAULT_LNKMAX 256

/* Virtual filesystem superblock (also a virtual directory). */
struct vsuperblock {
 struct superblock v_super;  /*< Underlying superblock ('sb_root.i_data' points at '&v_data') */
 struct vnode_data v_data;   /*< Virtual directory data. */
 atomic_rwlock_t   v_vlock;  /*< Lock for the chain of virtual nodes associated with this superblock. */
 WEAK size_t       v_lnkmax; /*< Max length of a dynamically allocated symbolic link. */
 /* [lock(v_vlock)][0..1] Chain of all INodes linked to this superblock.
  *   NOTE: This chain is used to keep the controller structures alive,
  *         ensuring that virtual nodes don't randomly disappear when
  *         no-one is using them anymore, but rather stay until the
  *         superblock is eventually unmounted.
  *   NOTE: The underlying 'sb_nodes' chain cannot be used, as it
  *         is meant to represent ~active~ INodes, not existing ones. */
#define v_chain   v_data.v_common.v_entry.le_next
};
#define VFS_FOREACH(node,self) \
        LIST_FOREACH(node,(self)->v_chain,i_data->v_entry)
#define VFS_MKINO(self_pointer) ((uintptr_t)(self_pointer))


/* Builtin INode/Superblock operators for virtual filesystems. */
DATDEF struct inodeops      vnode_ops;
DATDEF struct inodeops      vlink_ops;
DATDEF struct inodeops      vdev_ops;
/* NOTE: Also used as INode ops tag to indicate 'i_data' pointing to a 'vnode_common' structure. */
DATDEF struct superblockops vsuperblock_ops;
#define INODE_ISVNODE(self) ((self)->i_ops->o_tag == &vsuperblock_ops)




/* Default initializers for virtual filesystem elements.
 * NOTE: The first INode should pass '&v_superblock->v_data' for 'prev_data' */
#define VNODE_INIT(prev,self_pointer,next,superblock_pointer,mode,ent_c,ent_v) \
        VNODE_INIT_EX((prev)->v_data,self_pointer,(struct inode *)(next),superblock_pointer,mode,ent_c,ent_v)
#define VNODE_INIT_EX(prev_data,self_pointer,pnext_inode,superblock_pointer,mode,ent_c,ent_v) \
{ \
    .v_node = {\
        .i_refcnt    = 0x80000002u, /* Some high value to safely deal with re-use and  */ \
        .i_super     = (struct superblock *)(superblock_pointer), \
        .i_ops       = &vnode_ops, \
        .i_ino_id    = VFS_MKINO(self_pointer), \
        .i_nlink     = 1, \
        .i_attr_lock = RWLOCK_INIT, \
        .i_state     = INODE_STATE_GENERIC, \
        .i_owner     = THIS_INSTANCE, \
        .i_attr = { \
            .ia_mode  =  S_IFDIR|((mode)&0777), \
            .ia_uid   =  0, \
            .ia_gid   =  0, \
            .ia_siz   =  4096, \
            .ia_atime = {0,0}, \
            .ia_mtime = {0,0}, \
            .ia_ctime = {0,0}, \
        }, \
        .i_attr_disk = { \
            .ia_mode  =  S_IFDIR|((mode)&0777), \
            .ia_uid   =  0, \
            .ia_gid   =  0, \
            .ia_siz   =  4096, \
            .ia_atime = {0,0}, \
            .ia_mtime = {0,0}, \
            .ia_ctime = {0,0}, \
        }, \
        .i_file = { \
            .i_files_lock = ATOMIC_RWLOCK_INIT, \
            .i_flock = { \
                .fl_lock   = ATOMIC_RWLOCK_INIT, \
                .fl_avail  = SIG_INIT, \
                .fl_unlock = SIG_INIT, \
            }, \
        }, \
        .i_data = (struct idata *)&(self_pointer)->v_data,\
    }, \
    .v_data = { \
        .v_common = { \
            .v_entry = { \
                .le_next  = (pnext_inode), \
                .le_pself = &(prev_data).v_common.v_entry.le_next, \
            }, \
            .v_flag = (ent_c) ? VDATA_STATIC : VDATA_DYNAMIC, \
        }, \
        .v_lock = ATOMIC_RWLOCK_INIT, \
        .v_entc = (ent_c), \
        .v_entv = (ent_c) ? (ent_v) : NULL, \
    }, \
}
#define VLINK_INIT(prev,self_pointer,next,superblock_pointer,mode,text_size,text_ptr) \
        VLINK_INIT_EX((prev)->v_data,self_pointer,(struct inode *)(next),superblock_pointer,mode,text_size,text_ptr)
#define VLINK_INIT_EX(prev_data,self_pointer,pnext_inode,superblock_pointer,mode,text_size,text_ptr) \
{ \
    .v_node = {\
        .i_refcnt    = 1, \
        .i_super     = (struct superblock *)(superblock_pointer), \
        .i_ops       = &vlink_ops, \
        .i_ino_id    = VFS_MKINO(self_pointer), \
        .i_nlink     = 1, \
        .i_attr_lock = RWLOCK_INIT, \
        .i_state     = INODE_STATE_GENERIC, \
        .i_owner     = THIS_INSTANCE, \
        .i_attr = { \
            .ia_mode  =  S_IFLNK|((mode)&0777), \
            .ia_uid   =  0, \
            .ia_gid   =  0, \
            .ia_siz   =  4096, \
            .ia_atime = {0,0}, \
            .ia_mtime = {0,0}, \
            .ia_ctime = {0,0}, \
        }, \
        .i_attr_disk = { \
            .ia_mode  =  S_IFLNK|((mode)&0777), \
            .ia_uid   =  0, \
            .ia_gid   =  0, \
            .ia_siz   =  4096, \
            .ia_atime = {0,0}, \
            .ia_mtime = {0,0}, \
            .ia_ctime = {0,0}, \
        }, \
        .i_file = { \
            .i_files_lock = ATOMIC_RWLOCK_INIT, \
            .i_flock = { \
                .fl_lock   = ATOMIC_RWLOCK_INIT, \
                .fl_avail  = SIG_INIT, \
                .fl_unlock = SIG_INIT, \
            }, \
        }, \
        .i_data = (struct idata *)&(self_pointer)->v_data,\
    }, \
    .v_data = { \
        .v_common = { \
            .v_entry = { \
                .le_next  = (pnext_inode), \
                .le_pself = &(prev_data).v_common.v_entry.le_next, \
            }, \
            .v_flag = (text_size) ? VDATA_STATIC : VDATA_DYNAMIC, \
        }, \
        .v_size = (text_size), \
        .v_text = (text_size) ? (text_ptr) : NULL, \
    }, \
}
#define VSUPERBLOCK_INIT(self_pointer,next,mode,ent_c,ent_v) \
        VSUPERBLOCK_INIT_EX(self_pointer,(struct inode *)(next),mode,ent_c,ent_v)
#define VSUPERBLOCK_INIT_EX(self_pointer,pfirst_inode,mode,ent_c,ent_v) \
{ \
    .v_super = { \
        .sb_root = { \
            .i_refcnt    = 0x80000001, \
            .i_super     = (struct superblock *)(self_pointer), \
            .i_ops       = &vnode_ops, \
            .i_ino_id    = VFS_MKINO(self_pointer), \
            .i_nlink     = 0, \
            .i_attr_lock = RWLOCK_INIT, \
            .i_state     = INODE_STATE_GENERIC, \
            .i_owner     = THIS_INSTANCE, \
            .i_attr = { \
                .ia_mode  =  S_IFDIR|((mode)&0777), \
                .ia_uid   =  0, \
                .ia_gid   =  0, \
                .ia_siz   =  4096, \
                .ia_atime = {0,0}, \
                .ia_mtime = {0,0}, \
                .ia_ctime = {0,0}, \
            }, \
            .i_attr_disk = { \
                .ia_mode  =  S_IFDIR|((mode)&0777), \
                .ia_uid   =  0, \
                .ia_gid   =  0, \
                .ia_siz   =  4096, \
                .ia_atime = {0,0}, \
                .ia_mtime = {0,0}, \
                .ia_ctime = {0,0}, \
            }, \
            .i_file = { \
                .i_files_lock = ATOMIC_RWLOCK_INIT, \
                .i_flock = { \
                    .fl_lock   = ATOMIC_RWLOCK_INIT, \
                    .fl_avail  = SIG_INIT, \
                    .fl_unlock = SIG_INIT, \
                }, \
            }, \
            .i_data = (struct idata *)&(self_pointer)->v_data, \
        }, \
        .sb_ops        = &vsuperblock_ops, \
        .sb_nodes_lock = ATOMIC_RWLOCK_INIT, \
        .sb_achng_lock = ATOMIC_RWLOCK_INIT, \
        .sb_mount = { \
            .sm_mount_lock = ATOMIC_RWLOCK_INIT, \
            .sm_mountv     = NULL, \
        }, \
        .sb_blkdev = NULL, \
    }, \
    .v_data = { \
        .v_common = { \
            .v_entry = { \
                .le_pself = NULL, \
                .le_next  = (pfirst_inode), \
            }, \
            .v_flag = (ent_c) ? VDATA_STATIC : VDATA_DYNAMIC, \
        }, \
        .v_lock = ATOMIC_RWLOCK_INIT, \
        .v_entc = (ent_c), \
        .v_entv = (ent_c) ? (ent_v) : NULL, \
    }, \
    .v_vlock = ATOMIC_RWLOCK_INIT, \
    .v_lnkmax = VSUPERBLOCK_DEFAULT_LNKMAX, \
}



#if 0

/* Sample of how a virtual filesystem can be defined:
 * 
 *  proc_fs
 *  |
 *  +-- self -> "42"
 *  |
 *  +-- sys
 *  |   |
 *  |   +-- foo -> INODE_AT(0x12345678)
 *  |
 *  +-- x --+
 *  +-- y --+
 *  +-- z --+
 *          |
 *          +-- bar -> INODE_AT(0x87654321)
 */
INTDEF struct vsuperblock proc_fs;
INTDEF struct vnode proc_sys;
INTDEF struct vlink proc_self;
INTDEF struct vnode proc_x;
INTDEF struct vnode proc_y;
INTDEF struct vnode proc_z;

INTERN struct vnode_dirent proc_sys_elem[] = {
    { .d_name = { .dn_name = "foo" }, .d_node = (REF struct inode *)0x12345678, },
    { .d_name = { .dn_name = "foo" }, .d_node = (REF struct inode *)0x12345678, },
};
INTERN struct vnode_dirent proc_xyz_elem[] = {
    { .d_name = { .dn_name = "bar" }, .d_node = (REF struct inode *)0x87654321, },
};
INTERN struct vnode_dirent proc_elem[] = {
    { .d_name = { .dn_name = "self" }, .d_node = &proc_self.v_node, },
    { .d_name = { .dn_name = "sys" }, .d_node = &proc_sys.v_node, },
    { .d_name = { .dn_name = "x" }, .d_node = &proc_x.v_node, },
    { .d_name = { .dn_name = "y" }, .d_node = &proc_y.v_node, },
    { .d_name = { .dn_name = "z" }, .d_node = &proc_z.v_node, },
};
INTERN char const proc_self_link[] = "42";


INTERN struct vsuperblock proc_fs   = VSUPERBLOCK_INIT(&proc_fs,&proc_self,0755,COMPILER_LENOF(proc_elem),proc_elem);
INTERN struct vlink       proc_self = VLINK_INIT(&proc_fs,  &proc_self, &proc_sys, &proc_fs,0644,COMPILER_STRLEN(proc_self_link),proc_self_link);
INTERN struct vnode       proc_sys  = VNODE_INIT(&proc_self,&proc_sys,  &proc_x,   &proc_fs,0755,COMPILER_LENOF(proc_sys_elem),proc_sys_elem);
INTERN struct vnode       proc_x    = VNODE_INIT(&proc_sys, &proc_x,    &proc_y,   &proc_fs,0755,COMPILER_LENOF(proc_xyz_elem),proc_xyz_elem);
INTERN struct vnode       proc_y    = VNODE_INIT(&proc_x,   &proc_y,    &proc_z,   &proc_fs,0755,COMPILER_LENOF(proc_xyz_elem),proc_xyz_elem);
INTERN struct vnode       proc_z    = VNODE_INIT(&proc_y,   &proc_z,    NULL,      &proc_fs,0755,COMPILER_LENOF(proc_xyz_elem),proc_xyz_elem);
#endif

DECL_END

#endif /* !GUARD_INCLUDE_FS_VFS_H */
