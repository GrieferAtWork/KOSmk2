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
#ifndef GUARD_KERNEL_FS_PIPE_C
#define GUARD_KERNEL_FS_PIPE_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <hybrid/compiler.h>
#include <fs/inode.h>
#include <fs/iobuffer.h>
#include <fs/pipe.h>
#include <malloc.h>
#include <fcntl.h>
#include <sched/types.h>
#include <fs/superblock.h>
#include <fs/file.h>
#include <kernel/syscall.h>
#include <sched/task.h>
#include <kernel/user.h>
#include <fs/fd.h>

DECL_BEGIN


PRIVATE void KCALL
pipe_fclose(struct inode *__restrict ino,
            struct file *__restrict fp) {
 if ((fp->f_mode&O_ACCMODE) != O_RDONLY) {
  struct pipe *p = container_of(ino,struct pipe,p_node);
  /* The writer has been closed.
   * >> Mark the INode as closing and interrupts any readers. */
  ATOMIC_FETCHOR(p->p_node.i_state,INODE_STATE_CLOSING);
  task_nointr();
  iobuffer_interrupt(&p->p_data);
  task_endnointr();
 }
}
PRIVATE ssize_t KCALL
pipe_read(struct file *__restrict fp,
          USER void *buf, size_t bufsize) {
 struct pipe *p = container_of(fp->f_node,struct pipe,p_node);
 if (p->p_node.i_state&INODE_STATE_CLOSING) return 0;
 return iobuffer_read(&p->p_data,buf,bufsize,
                      /* Don't block if we're not supposed to. */
                      fp->f_mode&O_NONBLOCK ? IO_BLOCKNONE
                                            : IO_BLOCKFIRST);
}
PRIVATE ssize_t KCALL
pipe_write(struct file *__restrict fp,
           USER void const *buf, size_t bufsize) {
 struct pipe *p = container_of(fp->f_node,struct pipe,p_node);
 return iobuffer_write(&p->p_data,buf,bufsize,
                       /* Don't block if we're not supposed to. */
                       fp->f_mode&O_NONBLOCK ? IO_BLOCKNONE
                                             : IO_BLOCKFIRST);
}

PRIVATE off_t KCALL
pipe_seek(struct file *__restrict fp,
          off_t off, int whence) {
 struct pipe *p = container_of(fp->f_node,struct pipe,p_node);
 /* As an extension, KOS allows seek operations in pipes to undo
  * unread writes, or skip written data instead of reading it. */
 if (whence != SEEK_CUR) return -EINVAL;

 if ((fp->f_mode&O_ACCMODE) == O_RDONLY) {
  /* Seek the read-pointer. */
  return (off_t)iobuffer_seek_read(&p->p_data,(ssize_t)off);
 } else {
  /* Seek the write-pointer. */
  return (off_t)iobuffer_seek_write(&p->p_data,(ssize_t)off);
 }
}


/* Pipe INode operations. */
PUBLIC struct inodeops pipe_ops = {
    /* XXX: Support for packet-mode (O_DIRECT?) */
    .ino_fopen  = &inode_fopen_default,
    .ino_fclose = &pipe_fclose,
    .f_flags    = INODE_FILE_LOCKLESS,
    .f_read     = &pipe_read,
    .f_write    = &pipe_write,
    .f_seek     = &pipe_seek,
};





PUBLIC struct inodeops pipefs_rootops = {
 /* XXX: Emulate a virtual directory that enumerates 'pipe_fs.sb_nodes' */
};
PUBLIC struct superblockops pipefs_ops = {
};
PUBLIC struct superblock pipe_fs = {
    .sb_root = {
#ifdef CONFIG_DEBUG
        .i_refcnt = 1,
#else
        .i_refcnt = 0x80000001,
#endif
        .i_super = &pipe_fs,
        .i_ops   = &pipefs_rootops,
        .i_owner = THIS_INSTANCE,
        .i_ino   = 0,
        .i_nlink = 1,
        .i_attr_lock = RWLOCK_INIT,
        .i_attr = {
            .ia_mode = S_IFDIR|0770,
            .ia_uid  = 0,
            .ia_gid  = 0,
        },
        .i_attr_disk = {
            .ia_mode = S_IFDIR|0770,
            .ia_uid  = 0,
            .ia_gid  = 0,
        },
        .i_state = INODE_STATE_GENERIC,
        .i_file = {
            .i_files_lock = ATOMIC_RWLOCK_INIT,
            .i_flock = {
                .fl_lock = ATOMIC_RWLOCK_INIT,
                .fl_avail = SIG_INIT,
                .fl_unlock = SIG_INIT,
            },
        },
    },
    .sb_ops        = &pipefs_ops,
    .sb_nodes_lock = ATOMIC_RWLOCK_INIT,
    .sb_achng_lock = ATOMIC_RWLOCK_INIT,
    .sb_mount = {
        .sm_mount_lock = ATOMIC_RWLOCK_INIT,
        .sm_mountv     = NULL,
    },
    .sb_blkdev = NULL,
};




PUBLIC REF struct pipe *KCALL pipe_new(void) {
 REF struct pipe *result;
 result = (REF struct pipe *)inode_new(sizeof(struct pipe));
 if unlikely(!result) return NULL;
 iobuffer_cinit(&result->p_data);
 result->p_node.i_ops = &pipe_ops;
 result->p_node.i_attr_disk.ia_mode = result->p_node.i_attr.ia_mode = S_IFIFO|0660;
 result->p_node.i_attr_disk.ia_uid  = result->p_node.i_attr.ia_uid  = GET_THIS_UID();
 result->p_node.i_attr_disk.ia_gid  = result->p_node.i_attr.ia_gid  = GET_THIS_GID();
 /* Setup the INode as part of the PIPE-filesystem. */
 asserte(E_ISOK(inode_setup(&result->p_node,&pipe_fs,THIS_INSTANCE)));
 return result;
}


struct pipefd {
union{
   errno_t error;
   int     fd_reader;
}; int     fd_writer; };

PRIVATE SAFE struct pipefd KCALL mkpipe(int flags) {
 struct pipefd result;
 REF struct pipe *pfd;
 struct fd reader,writer;
 struct fdman *fdm = THIS_FDMAN;
 assert(TASK_ISSAFE());
 /* Create a new pipe. */
 pfd = pipe_new();
 if unlikely(!pfd) { result.error = -ENOMEM; goto end; }
 /* Open the pipe twice for reading & writing. */
 reader.fo_obj.fo_file = inode_kopen(&pfd->p_node,O_RDONLY);
 if (E_ISERR(reader.fo_obj.fo_file)) { result.error = E_GTERR(reader.fo_obj.fo_file); goto end2; }
 writer.fo_obj.fo_file = inode_kopen(&pfd->p_node,O_WRONLY);
 if (E_ISERR(writer.fo_obj.fo_file)) { result.error = E_GTERR(writer.fo_obj.fo_file); goto end3; }

 /* Setup file descriptor typing and flags. */
 reader.fo_ops = &fd_ops[FD_TYPE_FILE];
 if (flags&O_CLOEXEC) reader.fo_flags |= FD_CLOEXEC;
 if (flags&O_CLOFORK) reader.fo_flags |= FD_CLOFORK;
 writer.fo_hdata = reader.fo_hdata;

 /* Register the descriptors within the current FD-manager. */
 result.fd_reader = fdman_put(fdm,reader);
 if (E_ISOK(result.fd_reader)) {
  result.fd_writer = fdman_put(fdm,writer);
  if (E_ISERR(result.fd_writer)) {
   task_nointr();
   fdman_del(fdm,result.fd_reader);
   task_endnointr();
   result.error = E_GTERR(result.fd_writer);
  }
 }

 FILE_DECREF(writer.fo_obj.fo_file);
end3: FILE_DECREF(reader.fo_obj.fo_file);
end2: PIPE_DECREF(pfd);
end:  return result;
}


SYSCALL_DEFINE2(pipe2,USER int *,fildes,int,flags) {
 struct pipefd fd;
 errno_t result = -EOK;
 /* Mainly here for ABI-compatibility with linux. - libc uses xpipe() below! */
 task_crit();
 fd = mkpipe(flags);
 if (E_ISERR(fd.error))
  result = fd.error;
 else if (copy_to_user(fildes,&fd.fd_reader,2*sizeof(int))) {
  struct fdman *fdm = THIS_FDMAN;
  result = -EFAULT;
  /* Delete the previously created descriptors (Ugh...) */
  task_nointr();
  fdman_del(fdm,fd.fd_reader);
  fdman_del(fdm,fd.fd_writer);
  task_endnointr();
 }
 task_endcrit();
 return result;
}

SYSCALL_LDEFINE1(xpipe,int,flags) {
 struct pipefd fd;
 /* Much simpler & doesn't suffer from potential FAULT errors. */
 task_crit();
 fd = mkpipe(flags);
 task_endcrit();
 if (E_ISERR(fd.error)) return (s64)fd.error;
 return (s64)fd.fd_writer << 32 | fd.fd_reader;
}


DECL_END

#endif /* !GUARD_KERNEL_FS_PIPE_C */
