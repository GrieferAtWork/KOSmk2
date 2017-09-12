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
#ifndef GUARD_MODULES_FS_PROC_PROCFS_H
#define GUARD_MODULES_FS_PROC_PROCFS_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <fs/superblock.h>
#include <fs/inode.h>
#include <fs/dentry.h>
#include <fs/file.h>

DECL_BEGIN

#if __SIZEOF_PID_T__ == __SIZEOF_INT__
#   define PID_FMT  "%u"
#elif __SIZEOF_PID_T__ == __SIZEOF_LONG__
#   define PID_FMT  "%lu"
#elif __SIZEOF_PID_T__ == __SIZEOF_LONG_LONG__
#   define PID_FMT  "%llu"
#elif !defined(__DEEMON__)
#   error FIXME
#endif

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
   local temp = (uint32 *)(char *)x; \
   for (local i = 0; i < #(x) / 4; ++i) { hash += temp[i]; hash *= 9; } \
   local temp = x[#(x) - #(x) % 4 : #(x)]; \
   switch (#temp) { \
   case 3:  hash += (uint32)temp[2].ord() << 16; \
   case 2:  hash += (uint32)temp[1].ord() << 8; \
   case 1:  hash += (uint32)temp[0].ord(); \
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
   case 7:  hash += (uint64)temp[6].ord() << 48; \
   case 6:  hash += (uint64)temp[5].ord() << 40; \
   case 5:  hash += (uint64)temp[4].ord() << 32; \
   case 4:  hash += (uint64)temp[3].ord() << 24; \
   case 3:  hash += (uint64)temp[2].ord() << 16; \
   case 2:  hash += (uint64)temp[1].ord() << 8; \
   case 1:  hash += (uint64)temp[0].ord(); \
   default: break; \
   } \
   hash;\
})
#define DNAM(x) \
({ local _x = x; \
   print "{"+repr(_x)+","+#_x+",H("+DNAME_HASH32(_x)+"u,"+DNAME_HASH64(_x)+"llu)}",; \
})
#endif



struct procnode {
 ino_t             n_ino;  /*< Inode number of this entry. */
 mode_t            n_mode; /*< The type and mode of this node. */
 struct dentryname n_name; /*< Name of this node. */
 struct inodeops   n_ops;  /*< Operators for this node. */
};



/* Per-PID Inode descriptor. */
struct pidnode {
 struct inode          p_node; /*< Underlying INode. */
 WEAK REF struct task *p_task; /*< [1..1][const] A weak reference to the associated task.
                                *  NOTE: This _MUST_ be a weak reference to cancel the following loop:
                                *  >> p_task->t_fdman->[...(struct file)]->f_node->p_task. */
};

struct pidfile {
 struct file p_file; /*< Underlying file. */
 size_t      p_dirx; /*< [lock(p_file.f_lock)] Current directory position within 'pid_content'. */
};
INTDEF struct procnode const pid_content[];
INTDEF struct inodeops const pidops;


/* Create a new PID INode and _ALWAYS_ inherit a weak reference to 't' */
INTDEF REF struct pidnode *KCALL
pidnode_new_inherited(struct superblock *__restrict procfs,
                      WEAK REF struct task *__restrict t);




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

INTDEF struct inodeops const rootops;
INTDEF struct superblockops const sb_rootops;
INTDEF struct procnode const root_content[]; /* Misc. contents of the /proc root directory. */

INTERN SAFE REF struct superblock *KCALL
procfs_make(struct blkdev *__restrict dev, u32 flags,
            char const *devname, USER void *data, void *closure);


DECL_END

#endif /* !GUARD_MODULES_FS_PROC_PROCFS_H */
