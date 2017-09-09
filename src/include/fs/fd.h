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
#ifndef GUARD_INCLUDE_FS_FD_H
#define GUARD_INCLUDE_FS_FD_H 1

#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/malloc.h>
#include <kos/fcntl.h>
#include <sched/percpu.h>
#include <sync/rwlock.h>

DECL_BEGIN

struct blkdev;
struct chrdev;
struct dentry;
struct device;
struct dirent;
struct file;
struct inode;
struct instance;
struct mman;
struct module;
struct stack;
struct superblock;
struct task;

#ifndef __rdmode_t_defined
#define __rdmode_t_defined 1
typedef int rdmode_t;   /* readdir-mode (One of 'FILE_READDIR_*') */
#endif

#define FD_MASK      0x03 /*< Mask of file descriptor flags stored alongside the pointer to 'fdops' ('FD_CLOEXEC', 'FD_CLOFORK', etc...). */
#define FDOPS_ALIGN (FD_MASK+1)


ATTR_ALIGNED(FDOPS_ALIGN)
struct fdops {
 /* Common file descriptor operators.
  * NOTE: All operators _MUST_ be implemented!
  * @param: o: The pointed-to object (e.g.: 'struct inode *__restrict o') */
 void    (KCALL *fd_incref)(void *__restrict o);
 void    (KCALL *fd_decref)(void *__restrict o);
 ssize_t (KCALL *fd_read)(void *__restrict o, USER void *buf, size_t bufsize);
 ssize_t (KCALL *fd_write)(void *__restrict o, USER void const *buf, size_t bufsize);
 ssize_t (KCALL *fd_pread)(void *__restrict o, USER void *buf, size_t bufsize, pos_t pos);
 ssize_t (KCALL *fd_pwrite)(void *__restrict o, USER void const *buf, size_t bufsize, pos_t pos);
 off_t   (KCALL *fd_seek)(void *__restrict o, off_t off, int whence);
 errno_t (KCALL *fd_ioctl)(void *__restrict o, int name, USER void *arg);
 ssize_t (KCALL *fd_readdir)(void *__restrict o, USER struct dirent *buf, size_t bufsize, rdmode_t mode);
 errno_t (KCALL *fd_flush)(void *__restrict o);
};

struct fd {
union {
 uintptr_t              fo_hdata;      /*< File descriptor header data. */
 uintptr_t              fo_flags;      /*< [lock(:fm_lock)][mask(FD_MASK)] File descriptor flags.
                                        *   HINT: Always ZERO(0) in descriptors returned by 'fdman_get()' */
 struct fdops const    *fo_ops;        /*< [1..1][const][mask(~FD_MASK)][in(fd_ops)] File descriptor operations
                                        *  NOTE: Also used to track descriptor type ids. */
};
union PACKED {void     *fo_ptr;        /*< [?..?][ALWAYS_VALID] Pointed-to file descriptor object. */
union PACKED {
 void                  *fo_null;       /*< [FD_TYPE_NULL][?..?] Unused descriptor. */
 REF struct inode      *fo_inode;      /*< [FD_TYPE_INODE][1..1]. */
 REF struct superblock *fo_superblock; /*< [FD_TYPE_INODE][1..1]. */
 REF struct device     *fo_device;     /*< [FD_TYPE_INODE][1..1]. */
 REF struct blkdev     *fo_blkdev;     /*< [FD_TYPE_INODE][1..1]. */
 REF struct chrdev     *fo_chrdev;     /*< [FD_TYPE_INODE][1..1]. */
 REF struct file       *fo_file;       /*< [FD_TYPE_FILE][1..1]. */
 REF struct dentry     *fo_dentry;     /*< [FD_TYPE_DENTRY][1..1]. */
 WEAK REF struct task  *fo_task;       /*< [FD_TYPE_TASK][1..1] Must be a weak reference to prevent a reference loop.
                                        *                       This is OK though, because 'join()' can still be
                                        *                       executed and simply detect that it can't capture a
                                        *                       new reference to the task, causing it to read the
                                        *                       exitcode field and return immediately. */
 REF struct mman       *fo_mman;       /*< [FD_TYPE_MMAN][1..1]. */
 REF struct stack      *fo_stack;      /*< [FD_TYPE_STACK][1..1]. */
}                       fo_obj;        /*< Individual descriptor objects. */
};};
#define FD_SAFE_OPS(self)      ((struct fdops const *)((self).fo_hdata&~FD_MASK))
#define FD_SAFE_ISTYPE(self,t)                        (FD_SAFE_OPS(self) == &fd_ops[t])
#define FD_SAFE_ISNULL(self)             FD_SAFE_ISTYPE(self,FD_TYPE_NULL)
#define FD_SAFE_INCREF(self)              (*FD_SAFE_OPS(self)->fd_incref)((self).fo_ptr)
#define FD_SAFE_DECREF(self)              (*FD_SAFE_OPS(self)->fd_decref)((self).fo_ptr)
#define FD_IS_CLOFORK(self)                           ((self).fo_flags&FD_CLOFORK)
#define FD_TYPE(self)                                 ((self).fo_ops-fd_ops)
#define FD_ISTYPE(self,t)                             ((self).fo_ops == &fd_ops[t])
#define FD_ISNULL(self)                       FD_ISTYPE(self,FD_TYPE_NULL)
#define FD_INCREF(self)                              (*(self).fo_ops->fd_incref)((self).fo_ptr)
#define FD_DECREF(self)                              (*(self).fo_ops->fd_decref)((self).fo_ptr)

/* File descriptor operator types (Index is one of 'FD_TYPE_*'). */
DATDEF struct fdops const fd_ops[FD_TYPE_COUNT];

/* A stub-fd for illegal file numbers. */
DATDEF struct fd const fd_invalid;


#define FDMAN_DEFAULT_VECM 0xffff

struct fdman {
 ATOMIC_DATA ref_t  fm_refcnt; /*< [!0] Reference counter. */
 rwlock_t           fm_lock;   /*< R/W-lock for accessing this file descriptor manager. */
 REF struct dentry *fm_cwd;    /*< [lock(fm_lock)][1..1] Current working directory. */
 REF struct dentry *fm_root;   /*< [lock(fm_lock)][1..1] Filesystem root directory. */
 unsigned int       fm_hint;   /*< [lock(fm_lock)] Search hint used by 'fdman_put()' (also updated by 'fdman_put_nearby()'). */
 unsigned int       fm_veca;   /*< [lock(fm_lock)] Allocated descriptor vector size. */
 unsigned int       fm_vecc;   /*< [lock(fm_lock)][<= fm_veca] Amount of descriptors currently in use (NOTE: Not necessarily continuous). */
 unsigned int       fm_vecm;   /*< [lock(fm_lock)][<= INT_MAX] Max amount of allowed descriptor numbers (Defaults to 'FDMAN_DEFAULT_VECM'). */
 struct fd         *fm_vecv;   /*< [lock(fm_lock)][0..fm_veca][owned] File descriptor vector. */
};
#define fdman_reading(x)     rwlock_reading(&(x)->fm_lock)
#define fdman_writing(x)     rwlock_writing(&(x)->fm_lock)
#define fdman_tryread(x)     rwlock_tryread(&(x)->fm_lock)
#define fdman_trywrite(x)    rwlock_trywrite(&(x)->fm_lock)
#define fdman_tryupgrade(x)  rwlock_tryupgrade(&(x)->fm_lock)
#define fdman_read(x)        rwlock_read(&(x)->fm_lock)
#define fdman_write(x)       rwlock_write(&(x)->fm_lock)
#define fdman_upgrade(x)     rwlock_upgrade(&(x)->fm_lock)
#define fdman_downgrade(x)   rwlock_downgrade(&(x)->fm_lock)
#define fdman_endread(x)     rwlock_endread(&(x)->fm_lock)
#define fdman_endwrite(x)    rwlock_endwrite(&(x)->fm_lock)


/* The kernel's own FD-manager (Set for kernel/IDLE tasks).
 * NOTE: You really should use it, though. - It's just here to keep the
 *      'task::t_fdman != NULL' assumption universal...
 * NOTE: Drivers are not required to respect
 *      'fdman_kernel.fm_root' or 'fdman_kernel.fm_cwd',
 *       and can instead always use 'fs_root' directly! */
DATDEF struct fdman fdman_kernel;

/* The effect fd-manager of the current thread. */
#define THIS_FDMAN (THIS_TASK->t_fdman)



#define FDMAN_INCREF(self) (void)(ATOMIC_FETCHINC((self)->fm_refcnt))
#define FDMAN_DECREF(self) (void)(ATOMIC_DECFETCH((self)->fm_refcnt) || (fdman_destroy(self),0))
/* Create a new fd-manager with default (full) privilege settings. */
#define fdman_new()        fdman_init((struct fdman *)kmalloc(sizeof(struct fdman),GFP_SHARED|GFP_CALLOC))
FUNDEF struct fdman *KCALL fdman_init(struct fdman *self);
FUNDEF void KCALL fdman_destroy(struct fdman *__restrict self);

/* Lookup and return a new reference to the file descriptor 'no'.
 * NOTE: In the event of an invalid/unused 'no', 'fd_invalid' is returned.
 * NOTE: This function can properly handle 'AT_FDCWD' and 'AT_FDROOT'. */
FUNDEF SAFE WUNUSED REF struct fd KCALL fdman_get(struct fdman *__restrict self, int no);

/* Helper functions that automatically perform unwrapping on file descriptors.
 * @return: * :        A new reference to the managed kernel object.
 * @return: -EBADF:    Invalid file number / type.
 * @return: -EINTR:    The calling thread was interrupted.
 * @return: -ENOTDIR: [fdman_get_dentry] 'no' doesn't describe a dentry.
 * @return: -ENOTDIR: [fdman_get_inode] 'no' doesn't describe an inode.
 * @return: -EINVAL:  [fdman_get_task] The task has been destroyed (it has terminated). */
FUNDEF SAFE WUNUSED REF struct dentry *KCALL fdman_get_dentry(struct fdman *__restrict self, int no);
FUNDEF SAFE WUNUSED REF struct inode *KCALL fdman_get_inode(struct fdman *__restrict self, int no);
FUNDEF SAFE WUNUSED REF struct file *KCALL fdman_get_file(struct fdman *__restrict self, int no);
FUNDEF SAFE WUNUSED REF struct task *KCALL fdman_get_task(struct fdman *__restrict self, int no);
FUNDEF SAFE WUNUSED REF struct mman *KCALL fdman_get_mman(struct fdman *__restrict self, int no);

/* Set/Delete a file descriptor.
 * NOTE: 'fdman_set' will overwrite existing descriptors.
 * @return: -EOK:     Successfully set/deleted the given 'no'.
 * @return: -EINVAL:  'fp' is incompatible with the slot described by 'no' (Special 'AT_FD*' slots)
 * @return: -EBADF:  [fdman_set] The given 'no' was too high.
 * @return: -EBADF:  [fdman_del] The given 'no' was not in use.
 * @return: -ENFILE:  The given file descriptor number is too large.
 * @return: -EINTR:   The calling thread was interrupted. */
FUNDEF SAFE errno_t KCALL fdman_set(struct fdman *__restrict self, int no, struct fd const fp);
FUNDEF SAFE errno_t KCALL fdman_del(struct fdman *__restrict self, int no);
FUNDEF SAFE struct fd KCALL fdman_del_unlocked(struct fdman *__restrict self, int no);


/* Don't auto-use the first 3 descriptors, even when they've been closed.
 * >> This is done, because pretty much all unix applications are
 *    hard-coding use of 0,1 and 2 as STDIN_FILENO, 'STDOUT_FILENO' and 'STDERR_FILENO'.
 * Therefor, automatically re-using them could cause unexpected behavior. */
#define FDMAN_PUT_RESCAN_START 3

/* Insert 'fp' into the fd manager, choosing the next greater,
 * free descriptor number after that chosen during the last call
 * to either 'fdman_put()' or 'fdman_put_nearby()' (starting at ZERO(0)).
 * If attempting to use that greater number would require the
 * fd-vector to be re-allocated, first search for an unused slot
 * between 'FDMAN_PUT_RESCAN_START...'
 * NOTE: 'fdman_put_nearby()' behaves the same as
 *       'fdman_put', using 'hint' as search starting point.
 * @return: * :      The file descriptor number to-be used.
 * @return: -EMFILE: There are too many open file descriptors.
 * @return: -ENOMEM: Not enough available memory to re-allocate the FD-vector. */
FUNDEF SAFE int KCALL fdman_put(struct fdman *__restrict self, struct fd const fp);
FUNDEF SAFE int KCALL fdman_put_nearby(struct fdman *__restrict self, unsigned int hint, struct fd const fp);
FUNDEF SAFE int KCALL fdman_put_nearby_unlocked(struct fdman *__restrict self, unsigned int hint, struct fd const fp);
#define               fdman_put_unlocked(self,fp) fdman_put_nearby_unlocked(self,(self)->fm_hint,fp)

DECL_END

#endif /* !GUARD_INCLUDE_FS_FD_H */
