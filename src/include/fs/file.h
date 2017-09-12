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
#ifndef GUARD_INCLUDE_FS_FILE_H
#define GUARD_INCLUDE_FS_FILE_H 1

#include <hybrid/atomic.h>
#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>
#include <sync/rwlock.h>

DECL_BEGIN

struct inodeops;
struct inode;
struct dentry;
struct dirent;

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


/* FILE Api. */
struct lockrange {
 SLIST_NODE(struct lockrange)
       lr_chain; /*< [lock(:fl_lock)] Chain of known file locks. */
 pos_t lr_start; /*< [lock(:fl_lock)] Starting address of the locking range. */
 pos_t lr_size;  /*< [lock(:fl_lock)] Size of the locking range (in bytes). */
 bool  lr_write; /*< [lock(:fl_lock)] When true, this is a write-lock. */
};
struct filelock {
 atomic_rwlock_t               fl_lock;   /*< Lock for managing filelock ranges. */
 SLIST_HEAD(struct ilockrange) fl_ranges; /*< [0..1][lock(fl_lock)][owned] Chain of known file locks. */
};
struct file {
 ATOMIC_DATA ref_t       f_refcnt; /*< Reference counter. */
 struct inodeops const  *f_ops;    /*< [1..1][const][== f_node->i_ops] Local copy of file operations. */
 REF struct inode       *f_node;   /*< [1..1][const] INode associated with this file stream. */
 REF struct dentry      *f_dent;   /*< [1..1][const] Directory entry associated with this file stream.
                                    *   WARNING: This directory entry does not necessarily point to 'f_node'! */
 LIST_NODE(struct file)  f_open;   /*< [lock(f_node->i_file.i_files_lock)] List entry of streams opened for the associated node. */
 WEAK oflag_t            f_mode;   /*< [const(~(O_APPEND|O_ASYNC|O_DIRECT|O_NOATIME|O_NONBLOCK))]
                                    *   The mode under which this file was opened.
                                    *   NOTE: Any non-constant flag may be weakly changed at any point. */
 rwlock_t                f_lock;   /*< File lock for synchronizing various operations (Should not be used by f_ops-implementations).
                                    *  HINT: The state of this lock during operations is documented above,
                                    *        by the 'NOTE: Caller-synchronized:(read|write)' comments. */
 struct filelock         f_flock;  /*< Locks in 'f_node' owned by this file. */
#define FILE_FLAG_NONE     0x00000000
#define FILE_FLAG_BACK     0x00000001 /*< Set when the file pointer is located at the back (Used to implement 'O_APPEND' during writing) */
#define FILE_FLAG_DIDWRITE 0x00000002 /*< At some point, this file was used to write data (Used to implement automatic disk synchronization) */
#define FILE_FLAG_LOCKLESS 0x80000000 /*< [const] Local alias for 'f_ops->f_flags&INODE_FILE_LOCKLESS' */
 u32                     f_flag;   /*< [lock(f_lock)] File state flags (Set of 'FILE_FLAG_*'). */
 /* Additional user-file-data goes here. */
};
#define FILE_ISLOCKLESS(self) ((self)->f_flag&FILE_FLAG_LOCKLESS)


/* Simple, streamlined function for easily creating custom
 * file instances within 'struct inodeops::ino_fopen' */
#define file_new(type_size) ((struct file *)calloc(1,type_size))
FUNDEF void KCALL file_setup(struct file *__restrict fp,
                             struct inode *__restrict node,
                             struct dentry *__restrict dent,
                             oflag_t oflags);


FUNDEF void KCALL file_destroy(struct file *__restrict self);
#define FILE_TRYINCREF(self)  ATOMIC_INCIFNONZERO((self)->f_refcnt)
#define FILE_INCREF(self)    (void)(ATOMIC_FETCHINC((self)->f_refcnt))
#define FILE_DECREF(self)    (void)(ATOMIC_DECFETCH((self)->f_refcnt) || (file_destroy(self),0))

/* Acquire/Release/Override a file lock within the specified range.
 * @param: mode: A set of 'FILE_FLOCK_*'
 * @return: -EOK: Successfully acquired a file lock for the specified address range. */
FUNDEF errno_t KCALL file_flock(struct file *__restrict self, pos_t start, pos_t size, u32 mode);
#define FILE_FLOCK_UNLOCK 0x00 /*< Release the last lock. */
#define FILE_FLOCK_READ   0x01 /*< Acquire a shared lock. */
#define FILE_FLOCK_WRITE  0x02 /*< Acquire an exclusive lock. */
#define FILE_FLOCK_TEST   0x04 /*< [FLAG] Try to acquire a lock read/write; don't block until doing so succeeds. */


/* Poll for events on this file descriptor.
 * This function does something like this:
 * >> bool got_unsupported_signals = false;
 * >> pollmode_t result = 0;
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
 * Use of this function should look something like this:
 * >> if (FDSET_IS_EMPTY(set)) return -EOK; // Do nothing.
 * >> FOREACH_FILE_IN_FDSET(entry,set) {
 * >>     pollmode_t temp = file_poll(entry->fp,mode);
 * >>     entry->error = temp;
 * >>     if (temp&(POLLIN|POLLPRI|POLLOUT|POLLERR)) {
 * >>         // Data is already available.
 * >>         task_clrwait();
 * >>         return -EOK;
 * >>     }
 * >> }
 * >> // Wait for the data to become available in any file apart of the set.
 * >> task_waitfor(NULL);
 * WARNING: Regardless of return value, any number of signals may have been
 *          added to the calling thread's sigwait set, meaning that the caller
 *          is always responsible for cleanup by either calling 'task_clrwait()'
 *          or 'task_waitfor()'.
 * @param: mode: Set of 'POLLIN|POLLPRI|POLLOUT|POLLERR'
 * @return: * :           The set of signals currently available. (At least one of 'mode')
 *                  NOTE: In this event, signals may have still been added to waiting 'task_addwait()',
 *                        meaning that the caller is responsible for cleanup, which must include a call
 *                        to 'task_clrwait()'
 * @return: 0 :           At least one signal has been added to the caller's waiting set (using 'task_addwait()').
 * @return: -EWOULDBLOCK: No signal signal requested for polling actually exists.
 * @return: -EWOULDBLOCK: The given 'mode' set does not contain any of 'POLLIN|POLLPRI|POLLOUT|POLLERR'
 * @return: -ENOMEM:      Not enough available memory (likely occurred when
 *                        trying to allocate a signal slot within 'task_addwait()')
 * @return: -EPERM:       The oflags used to open 'fp' are absolutely incompatible with 'mode'.
 *                       (e.g.: Trying to wait for data to become available
 *                              for reading on a file opened only for writing)
 * @return: E_ISERR(*):   Failed to poll data for some reason. */
FUNDEF pollmode_t KCALL file_poll(struct file *__restrict fp, pollmode_t mode);

/* Read/Write data to the given file stream.
 * @return: * :         The amount of bytes read-from/written-to 'buf'.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -EPERM:     The file stream doesn't implement reading/writing, or
 *                      the file wasn't opened with read/write permissions.
 * @return: -EFAULT:    A faulty buffer pointer was provided.
 * @return: E_ISERR(*): Some error prevented the read/write from succeeding. */
FUNDEF ssize_t KCALL file_read(struct file *__restrict self, USER void *buf, size_t bufsize);
FUNDEF ssize_t KCALL file_write(struct file *__restrict self, USER void const *buf, size_t bufsize);
FUNDEF ssize_t KCALL file_pread(struct file *__restrict self, USER void *buf, size_t bufsize, pos_t pos);
FUNDEF ssize_t KCALL file_pwrite(struct file *__restrict self, USER void const *buf, size_t bufsize, pos_t pos);

/* Same as the functions above, but return '-ENOSPC' when not all data could be read. */
LOCAL errno_t KCALL file_readall(struct file *__restrict self, USER void *buf, size_t bufsize);
LOCAL errno_t KCALL file_writeall(struct file *__restrict self, USER void const *buf, size_t bufsize);
LOCAL errno_t KCALL file_preadall(struct file *__restrict self, USER void *buf, size_t bufsize, pos_t pos);
LOCAL errno_t KCALL file_pwriteall(struct file *__restrict self, USER void const *buf, size_t bufsize, pos_t pos);

/* Hint the file-system to pre-allocate 'size' bytes within 'fp', starting at 'start'.
 * NOTE: No-op if the given data area has already been allocated for some reason.
 * @param: mode:        One of 'FALLOCMODE_*'
 * @return: -EOK:       Data was successfully pre-allocated
 *                      WARNING: Internal re-use semantics may undo this at
 *                               any time before real data is actually written!
 * @return: E_ISERR(*): Failed to pre-allocate file data for some reason. */
FUNDEF errno_t KCALL file_allocate(struct file *__restrict fp,
                                   fallocmode_t mode, pos_t start, pos_t size);

/* Set the file pointer of a given file stream.
 * NOTE: In directories, seeking should move the file index in a
 *       way that simulates each file having a size of '1' byte:
 *    >> struct file *fp = fs_kopen("/",...);
 *    >> file_seek(fp,7,SEEK_SET); // Select the 7th file in the root directory.
 *    >> file_readdir(fp,...);     // Read the 7th file.
 * @return: * :         The new file pointer location, offset from 'SEEK_SET'.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ESPIPE:   'self' doesn't support seeking and is a pipe/character/socket-style stream.
 * @return: -EPERM:    'self' doesn't support seeking and isn't a pipe/character/socket-style stream.
 * @return: E_ISERR(*): Some error prevented the seek operation from succeeding. */
FUNDEF off_t KCALL file_seek(struct file *__restrict self, off_t off, int whence);

/* Perform a misc. I/O control function 'name' on the given file stream 'self'.
 * @return: -EINVAL:    The specified command 'name' isn't known or unsupported.
 * @return: -EFAULT:    A faulty buffer pointer was provided.
 * @return: E_ISERR(*): Some error prevented the I/Octl operation from succeeding. */
FUNDEF errno_t KCALL file_ioctl(struct file *__restrict self, int name, USER void *arg);

/* Read a directory entry from an opened directory stream 'self'.
 * @return: * :         The amount of bytes required from the 'buf...+=bufsize' buffer.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -EPERM:     The given file stream 'self' does not support reading directory
 *                      entries, or the file 'self' wasn't opened for reading.
 * @return: -EFAULT:    A faulty buffer pointer was provided.
 * @return: E_ISERR(*): Some error prevented a directory from being read. */
FUNDEF ssize_t KCALL file_readdir(struct file *__restrict self,
                                  USER struct dirent *buf,
                                  size_t bufsize, rdmode_t mode);

/* Flush any unwritten data from the given file stream 'self'.
 * @return: -EOK:       Successfully flushed some, or no data.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -EROFS:     The given file stream 'self' was not opened for writing,
 *                      meaning that no unwritten data needed to be flushed.
 * @return: E_ISERR(*): Some error prevented data from being flushed. */
FUNDEF errno_t KCALL file_flush(struct file *__restrict self);

/* Kernel-space equivalents of some of the above functions using user-space buffers. */
LOCAL ssize_t KCALL file_kread(struct file *__restrict self, HOST void *__restrict buf, size_t bufsize);
LOCAL ssize_t KCALL file_kwrite(struct file *__restrict self, HOST void const *__restrict buf, size_t bufsize);
LOCAL ssize_t KCALL file_kpread(struct file *__restrict self, HOST void *__restrict buf, size_t bufsize, pos_t pos);
LOCAL ssize_t KCALL file_kpwrite(struct file *__restrict self, HOST void const *__restrict buf, size_t bufsize, pos_t pos);
LOCAL errno_t KCALL file_kreadall(struct file *__restrict self, HOST void *__restrict buf, size_t bufsize);
LOCAL errno_t KCALL file_kwriteall(struct file *__restrict self, HOST void const *__restrict buf, size_t bufsize);
LOCAL errno_t KCALL file_kpreadall(struct file *__restrict self, HOST void *__restrict buf, size_t bufsize, pos_t pos);
LOCAL errno_t KCALL file_kpwriteall(struct file *__restrict self, HOST void const *__restrict buf, size_t bufsize, pos_t pos);
LOCAL ssize_t KCALL file_kreaddir(struct file *__restrict self, HOST struct dirent *__restrict buf, size_t bufsize, rdmode_t mode);

DECL_END

#ifndef __INTELLISENSE__
#include <hybrid/check.h>
#include <kernel/user.h>
DECL_BEGIN

LOCAL errno_t KCALL
file_readall(struct file *__restrict self,
             USER void *buf, size_t bufsize) {
 while (bufsize) {
  ssize_t temp = file_read(self,buf,bufsize);
  if unlikely(!temp) temp = -ENOSPC;
  if (E_ISERR(temp)) return temp;
  *(uintptr_t *)&buf += temp;
  bufsize            -= temp;
 }
 return -EOK;
}
LOCAL errno_t KCALL
file_writeall(struct file *__restrict self,
              USER void const *buf, size_t bufsize) {
 while (bufsize) {
  ssize_t temp = file_write(self,buf,bufsize);
  if unlikely(!temp) temp = -ENOSPC;
  if (E_ISERR(temp)) return temp;
  *(uintptr_t *)&buf += temp;
  bufsize            -= temp;
 }
 return -EOK;
}
LOCAL errno_t KCALL
file_preadall(struct file *__restrict self,
              USER void *buf, size_t bufsize,
              pos_t pos) {
 while (bufsize) {
  ssize_t temp = file_pread(self,buf,bufsize,pos);
  if unlikely(!temp) temp = -ENOSPC;
  if (E_ISERR(temp)) return temp;
  *(uintptr_t *)&buf += temp;
  bufsize            -= temp;
  pos                += temp;
 }
 return -EOK;
}
LOCAL errno_t KCALL
file_pwriteall(struct file *__restrict self,
               USER void const *buf, size_t bufsize,
               pos_t pos) {
 while (bufsize) {
  ssize_t temp = file_pwrite(self,buf,bufsize,pos);
  if unlikely(!temp) temp = -ENOSPC;
  if (E_ISERR(temp)) return temp;
  *(uintptr_t *)&buf += temp;
  bufsize            -= temp;
  pos                += temp;
 }
 return -EOK;
}

LOCAL ssize_t KCALL
file_kread(struct file *__restrict self,
           HOST void *__restrict buf, size_t bufsize) {
 ssize_t result; CHECK_HOST_DATA(buf,bufsize);
 HOSTMEMORY_BEGIN { result = file_read(self,buf,bufsize); }
 HOSTMEMORY_END;
 return result;
}
LOCAL ssize_t KCALL
file_kwrite(struct file *__restrict self,
            HOST void const *__restrict buf, size_t bufsize) {
 ssize_t result; CHECK_HOST_TEXT(buf,bufsize);
 HOSTMEMORY_BEGIN { result = file_write(self,buf,bufsize); }
 HOSTMEMORY_END;
 return result;
}
LOCAL ssize_t KCALL
file_kpread(struct file *__restrict self,
            HOST void *__restrict buf,
            size_t bufsize, pos_t pos) {
 ssize_t result; CHECK_HOST_DATA(buf,bufsize);
 HOSTMEMORY_BEGIN { result = file_pread(self,buf,bufsize,pos); }
 HOSTMEMORY_END;
 return result;
}
LOCAL ssize_t KCALL
file_kpwrite(struct file *__restrict self,
             HOST void const *__restrict buf,
             size_t bufsize, pos_t pos) {
 ssize_t result; CHECK_HOST_TEXT(buf,bufsize);
 HOSTMEMORY_BEGIN { result = file_pwrite(self,buf,bufsize,pos); }
 HOSTMEMORY_END;
 return result;
}
LOCAL errno_t KCALL
file_kreadall(struct file *__restrict self,
              HOST void *__restrict buf, size_t bufsize) {
 errno_t result; CHECK_HOST_DATA(buf,bufsize);
 HOSTMEMORY_BEGIN { result = file_readall(self,buf,bufsize); }
 HOSTMEMORY_END;
 return result;
}
LOCAL errno_t KCALL
file_kwriteall(struct file *__restrict self,
               HOST void const *__restrict buf, size_t bufsize) {
 errno_t result; CHECK_HOST_TEXT(buf,bufsize);
 HOSTMEMORY_BEGIN { result = file_writeall(self,buf,bufsize); }
 HOSTMEMORY_END;
 return result;
}
LOCAL errno_t KCALL
file_kpreadall(struct file *__restrict self,
               HOST void *__restrict buf,
               size_t bufsize, pos_t pos) {
 errno_t result; CHECK_HOST_DATA(buf,bufsize);
 HOSTMEMORY_BEGIN { result = file_preadall(self,buf,bufsize,pos); }
 HOSTMEMORY_END;
 return result;
}
LOCAL errno_t KCALL
file_kpwriteall(struct file *__restrict self,
                HOST void const *__restrict buf,
                size_t bufsize, pos_t pos) {
 errno_t result; CHECK_HOST_TEXT(buf,bufsize);
 HOSTMEMORY_BEGIN { result = file_pwriteall(self,buf,bufsize,pos); }
 HOSTMEMORY_END;
 return result;
}
LOCAL ssize_t KCALL
file_kreaddir(struct file *__restrict self,
              HOST struct dirent *__restrict buf,
              size_t bufsize, rdmode_t mode) {
 ssize_t result; CHECK_HOST_DATA(buf,bufsize);
 HOSTMEMORY_BEGIN { result = file_readdir(self,buf,bufsize,mode); }
 HOSTMEMORY_END;
 return result;
}
DECL_END
#endif

#endif /* !GUARD_INCLUDE_FS_FILE_H */
