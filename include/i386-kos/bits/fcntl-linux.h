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
#ifndef _BITS_FCNTL_LINUX_H
#define _BITS_FCNTL_LINUX_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <features.h>

__SYSDECL_BEGIN

#ifdef __USE_GNU
//#include <bits/uio.h>
#endif

/* Open flags that are universal (binary-wise) */
#define O_ACCMODE     000000003
#define O_RDONLY      000000000
#define O_WRONLY      000000001
#define O_RDWR        000000002
/*      O_RDWR        000000003 // Implemented as an alias! */
#define O_TRUNC       000001000 /* This one was probably just a coincidence... */

/* Flags with common binary compatibility between DOS and UNIX. */
#define __DOS_O_COMMON  (O_TRUNC|O_ACCMODE)



/* DOS open flag values. */
#define __DOS_O_APPEND       000000010
#define __DOS_O_RANDOM       000000020
#define __DOS_O_SEQUENTIAL   000000040
#define __DOS_O_TEMPORARY    000000100
#define __DOS_O_NOINHERIT    000000200
#define __DOS_O_CREAT        000000400
#define __DOS_O_TRUNC        000001000
#define __DOS_O_EXCL         000002000
#define __DOS_O_SHORT_LIVED  000010000
#define __DOS_O_OBTAIN_DIR   000020000
#define __DOS_O_TEXT         000040000
#define __DOS_O_BINARY       000100000
#define __DOS_O_WTEXT        000200000
#define __DOS_O_U16TEXT      000400000
#define __DOS_O_U8TEXT       001000000
#define __DOS_O_RAW          __DOS_O_BINARY

#ifdef __USE_DOS
/* DOS name aliases */
#define _O_RDONLY       O_RDONLY
#define _O_WRONLY       O_WRONLY
#define _O_RDWR         O_RDWR
#define _O_APPEND       O_APPEND
#define _O_BINARY       O_BINARY
#define _O_CREAT        O_CREAT
#define _O_EXCL         O_EXCL
#define _O_RANDOM       O_RANDOM
#define _O_RAW          O_RAW
#define _O_SEQUENTIAL   O_SEQUENTIAL
#define _O_TEXT         O_TEXT
#define _O_TRUNC        O_TRUNC
#define _O_NOINHERIT    __O_CLOEXEC
#define _O_TEMPORARY    __O_TMPFILE
#define _O_OBTAIN_DIR   __O_DIRECTORY
#define O_NOINHERIT     __O_CLOEXEC
#define O_TEMPORARY     __O_TMPFILE
#endif /* __USE_DOS */

#ifdef __USE_DOSFS
#define __O_DIRECTORY   __DOS_O_OBTAIN_DIR
#define O_APPEND        __DOS_O_APPEND
#define O_CREAT         __DOS_O_CREAT
#define O_EXCL          __DOS_O_EXCL
#define __O_CLOEXEC     __DOS_O_NOINHERIT
#define __O_TMPFILE     __DOS_O_TEMPORARY
/* Unix flags not supported on DOS. */
#define O_NOCTTY        0
#define O_NONBLOCK      0
#define O_SYNC          0
#define O_ASYNC         0
#define __O_DSYNC       0
#define __O_DIRECT      0
#define __O_LARGEFILE   0
#define __O_NOFOLLOW    0
#define __O_NOATIME     0
#ifdef __USE_DOS
/* DOS extension flags. */
#define O_RAW           __DOS_O_RAW
#define O_TEXT          __DOS_O_TEXT
#define O_BINARY        __DOS_O_BINARY
#define O_SEQUENTIAL    __DOS_O_SEQUENTIAL
#define O_RANDOM        __DOS_O_RANDOM
#define _O_SHORT_LIVED  __DOS_O_SHORT_LIVED
#define _O_U16TEXT      __DOS_O_U16TEXT
#define _O_U8TEXT       __DOS_O_U8TEXT
#define _O_WTEXT        __DOS_O_WTEXT
#endif /* __USE_DOS */
#else /* __USE_DOSFS */
#define O_CREAT        000000100
#define O_EXCL         000000200
#define O_NOCTTY       000000400
#define O_APPEND       000002000
#define O_NONBLOCK     000004000
#define O_SYNC         000010000
#define __O_DSYNC      000010000
#define O_ASYNC        000020000
#define __O_DIRECT     000040000
#define __O_LARGEFILE  000100000
#define __O_DIRECTORY  000200000
#define __O_NOFOLLOW   000400000
#define __O_NOATIME    001000000
#define __O_CLOEXEC    002000000
#define __O_PATH       010000000
#define __O_TMPFILE   (020000000|__O_DIRECTORY)
#ifdef __CRT_KOS
#define __O_CLOFORK    004000000
#define __O_DOSPATH   0400000000 /* Interpret '\\' as '/', and ignore casing during path resolution. */
#endif /* __CRT_KOS */

#ifdef __USE_DOS
/* DOS names for some misc./ignored flags. */
#define O_RAW           0
#define O_TEXT          0
#define O_BINARY        0
#define O_SEQUENTIAL    0
#define O_RANDOM        0
#define _O_SHORT_LIVED  0
#define _O_U16TEXT      0
#define _O_U8TEXT       0
#define _O_WTEXT        0
#endif /* __USE_DOS */
#endif /* !__USE_DOSFS */

#define O_NDELAY       O_NONBLOCK
#define O_FSYNC        O_SYNC

#ifdef __USE_LARGEFILE64
#   define O_LARGEFILE  __O_LARGEFILE
#endif
#ifdef __USE_XOPEN2K8
#   define O_DIRECTORY  __O_DIRECTORY
#   define O_NOFOLLOW   __O_NOFOLLOW
#   define O_CLOEXEC    __O_CLOEXEC
#endif
#ifdef __USE_KOS
#   define O_CLOFORK    __O_CLOFORK
#   define O_DOSPATH    __O_DOSPATH
#endif
#ifdef __USE_GNU
#   define O_DIRECT     __O_DIRECT
#   define O_NOATIME    __O_NOATIME
#   define O_PATH       __O_PATH
#   define O_TMPFILE    __O_TMPFILE
#endif
#if defined(__USE_POSIX199309) || defined(__USE_UNIX98)
#   define O_DSYNC      __O_DSYNC
#   define O_RSYNC      O_SYNC
#endif

#ifndef __DOS_COMPAT__
#ifndef __USE_FILE_OFFSET64
#   define F_GETLK     5 /*< Get record locking info. */
#   define F_SETLK     6 /*< Set record locking info (non-blocking). */
#   define F_SETLKW    7 /*< Set record locking info (blocking).    */
#else
#   define F_GETLK     F_GETLK64  /*< Get record locking info.    */
#   define F_SETLK     F_SETLK64  /*< Set record locking info (non-blocking).*/
#   define F_SETLKW    F_SETLKW64 /*< Set record locking info (blocking). */
#endif
#ifndef F_GETLK64
#   define F_GETLK64   12 /*< Get record locking info. */
#   define F_SETLK64   13 /*< Set record locking info (non-blocking). */
#   define F_SETLKW64  14 /*< Set record locking info (blocking).    */
#endif
#ifdef __USE_GNU
#   define F_OFD_GETLK  36
#   define F_OFD_SETLK  37
#   define F_OFD_SETLKW 38
#endif

#define F_DUPFD        0 /*< Duplicate file descriptor. */
#define F_GETFD        1 /*< Get file descriptor flags. */
#define F_SETFD        2 /*< Set file descriptor flags. */
#define F_GETFL        3 /*< Get file status flags. */
#define F_SETFL        4 /*< Set file status flags. */

#ifndef __F_SETOWN
#   define __F_SETOWN  8
#   define __F_GETOWN  9
#endif

#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
#   define F_SETOWN    __F_SETOWN /*< Get owner (process receiving SIGIO). */
#   define F_GETOWN    __F_GETOWN /*< Set owner (process receiving SIGIO). */
#endif

#ifndef __F_SETSIG
#   define __F_SETSIG    10 /*< Set number of signal to be sent. */
#   define __F_GETSIG    11 /*< Get number of signal to be sent. */
#endif
#ifndef __F_SETOWN_EX
#   define __F_SETOWN_EX 15 /*< Get owner (thread receiving SIGIO). */
#   define __F_GETOWN_EX 16 /*< Set owner (thread receiving SIGIO). */
#endif

#ifdef __USE_GNU
#   define F_SETSIG      __F_SETSIG    /*< Set number of signal to be sent. */
#   define F_GETSIG      __F_GETSIG    /*< Get number of signal to be sent. */
#   define F_SETOWN_EX   __F_SETOWN_EX /*< Get owner (thread receiving SIGIO). */
#   define F_GETOWN_EX   __F_GETOWN_EX /*< Set owner (thread receiving SIGIO). */
#endif

#ifdef __USE_GNU
#   define F_SETLEASE   1024 /*< Set a lease. */
#   define F_GETLEASE   1025 /*< Enquire what lease is active. */
#   define F_NOTIFY     1026 /*< Request notifications on a directory. */
#   define F_SETPIPE_SZ 1031 /*< Set pipe page size array. */
#   define F_GETPIPE_SZ 1032 /*< Set pipe page size array. */
#endif
#ifdef __USE_XOPEN2K8
#   define F_DUPFD_CLOEXEC 1030 /*< Duplicate file descriptor with close-on-exit set. */
#endif
#ifdef __USE_KOS
#   define F_SETFL_XCH  5163 /*< Same as 'F_SETFL', but return the old set of flags instead of `-EOK' upon success. */
#endif

#define FD_CLOEXEC   0x01 /*< FLAG: Close the descriptor on `exec()'. */
#ifdef __USE_KOS
#define FD_CLOFORK   0x02 /*< FLAG: Close the descriptor during unsharing after `fork()' (Similar to `PROT_LOOSE' for memory). */
#endif /* __USE_KOS */

#ifndef F_RDLCK
#   define F_RDLCK 0 /*< Read lock. */
#   define F_WRLCK 1 /*< Write lock. */
#   define F_UNLCK 2 /*< Remove lock. */
#endif

#ifndef F_EXLCK
#   define F_EXLCK 4
#   define F_SHLCK 8
#endif

#ifdef __USE_MISC
#   define LOCK_SH 1 /* Shared lock. */
#   define LOCK_EX 2 /* Exclusive lock. */
#   define LOCK_NB 4 /* Or'd with one of the above to prevent blocking. */
#   define LOCK_UN 8 /* Remove lock. */
#endif

#ifdef __USE_GNU
#   define LOCK_MAND  32  /* This is a mandatory flock: */
#   define LOCK_READ  64  /* ... which allows concurrent read operations. */
#   define LOCK_WRITE 128 /* ... which allows concurrent write operations. */
#   define LOCK_RW    192 /* ... Which allows concurrent read & write operations. */
#endif

#ifdef __USE_GNU
#   define DN_ACCESS    0x00000001 /*< File accessed. */
#   define DN_MODIFY    0x00000002 /*< File modified. */
#   define DN_CREATE    0x00000004 /*< File created. */
#   define DN_DELETE    0x00000008 /*< File removed. */
#   define DN_RENAME    0x00000010 /*< File renamed. */
#   define DN_ATTRIB    0x00000020 /*< File changed attributes. */
#   define DN_MULTISHOT 0x80000000 /*< Don't remove notifier. */
#endif


#ifdef __USE_GNU
enum __pid_type {
    F_OWNER_TID  = 0, /*< Kernel thread. */
    F_OWNER_PID  = 1, /*< Process. */
    F_OWNER_PGRP = 2, /*< Process group. */
    F_OWNER_GID  = 2  /*< Alternative, obsolete name. */
};
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("type")
#pragma push_macro("pid")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef type
#undef pid
struct f_owner_ex {
    enum __pid_type type; /*< Owner type of ID. */
    __pid_t         pid;  /*< ID of owner. */
};
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("pid")
#pragma pop_macro("type")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif

#ifdef __USE_MISC
#   define FAPPEND   O_APPEND
#   define FFSYNC    O_FSYNC
#   define FASYNC    O_ASYNC
#   define FNONBLOCK O_NONBLOCK
#   define FNDELAY   O_NDELAY
#endif
#ifndef __POSIX_FADV_DONTNEED
#   define __POSIX_FADV_DONTNEED 4
#   define __POSIX_FADV_NOREUSE  5
#endif
#ifdef __USE_XOPEN2K
#   define POSIX_FADV_NORMAL     0 /*< No further special treatment. */
#   define POSIX_FADV_RANDOM     1 /*< Expect random page references. */
#   define POSIX_FADV_SEQUENTIAL 2 /*< Expect sequential page references. */
#   define POSIX_FADV_WILLNEED   3 /*< Will need these pages. */
#   define POSIX_FADV_DONTNEED   __POSIX_FADV_DONTNEED /*< Don't need these pages. */
#   define POSIX_FADV_NOREUSE    __POSIX_FADV_NOREUSE  /*< Data will be accessed once. */
#endif

#ifdef __USE_GNU
#   define SYNC_FILE_RANGE_WAIT_BEFORE 1 /*< Wait upon writeout of all pages in the range before performing the write. */
#   define SYNC_FILE_RANGE_WRITE       2 /*< Initiate writeout of all those dirty pages in the range which are not presently under writeback. */
#   define SYNC_FILE_RANGE_WAIT_AFTER  4 /*< Wait upon writeout of all pages in the range after performing the write. */

#   define SPLICE_F_MOVE               1 /*< Move pages instead of copying. */
#   define SPLICE_F_NONBLOCK           2 /*< Don't block on the pipe splicing (but we may still block on the fd we splice from/to). */
#   define SPLICE_F_MORE               4 /*< Expect more data. */
#   define SPLICE_F_GIFT               8 /*< Pages passed in are a gift. */

#   define FALLOC_FL_KEEP_SIZE         1 /*< Don't extend size of file even if offset + len is greater than file size. */
#   define FALLOC_FL_PUNCH_HOLE        2 /*< Create a hole in the file. */
#   define FALLOC_FL_COLLAPSE_RANGE    8 /*< Remove a range of a file without leaving a hole. */
#   define FALLOC_FL_ZERO_RANGE        16 /*< Convert a range of a file to zeros. */

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("handle_bytes")
#pragma push_macro("handle_type")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef handle_bytes
#undef handle_type
struct file_handle {
    unsigned int  handle_bytes;
    int           handle_type;
    unsigned char f_handle[0]; /* File identifier. */
};
#define MAX_HANDLE_SZ    128
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("handle_type")
#pragma pop_macro("handle_bytes")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif

#ifdef __USE_ATFILE
#   define AT_FDCWD              (-100)/* Special value used to indicate the *at functions should use the current working directory. */
#   define AT_SYMLINK_NOFOLLOW  0x0100 /* Do not follow symbolic links. */
#   define AT_REMOVEDIR         0x0200 /* Remove directory instead of unlinking file. */
#   define AT_SYMLINK_FOLLOW    0x0400 /* Follow symbolic links. */
#ifdef __USE_GNU
#   define AT_NO_AUTOMOUNT      0x0800 /* Suppress terminal automount traversal. */
#   define AT_EMPTY_PATH        0x1000 /* Allow empty relative pathname. */
#endif /* __USE_GNU */
#   define AT_EACCESS           0x0200 /* Test access permitted for effective IDs, not real IDs. */
#ifdef __USE_KOS
#   define AT_REMOVEREG        0x40000 /* Explicitly allow removing anything that unlink() removes. (Default; Set in addition to `AT_REMOVEDIR' to implement `remove()' semantics). */
#ifdef __KERNEL__
#   define AT_REMOVEMNT        0x80000 /* Used internally by the kernel: Delete a mounting point. (Userspace must use `unmount()') */
#endif /* __KERNEL__ */
#   define AT_DOSPATH         0x100000 /* Interpret '\\' as '/', and ignore casing during path resolution. */
#endif

#ifdef __USE_KOS
#   define AT_FDROOT        (-101) /*< Same as `AT_FDCWD' but sets the filesystem root (using this, you can `chroot()' with 'dup2()'!) */
/* Special, symbolic file numbers. 
 * These descriptors cannot be overwritten,
 * and their meaning is context-sensible. */
#   define AT_THIS_TASK     (-180)
#   define AT_THIS_MMAN     (-181)
#   define AT_THIS_STACK    (-182)
#endif /* __USE_KOS */
#endif /* __USE_ATFILE */
#endif /* !__DOS_COMPAT__ */

#ifdef __USE_GNU
#ifdef __DOS_COMPAT__
/* Ignore these as no-ops under DOS. */
__LOCAL __ssize_t (__LIBCCALL readahead)(int __UNUSED(__fd), __off64_t __UNUSED(__offset), __size_t __count) { return (__size_t)__count; }
__LOCAL int (__LIBCCALL sync_file_range)(int __UNUSED(__fd), __off64_t __UNUSED(__offset), __off64_t __UNUSED(__count), unsigned int __UNUSED(__flags)) { return 0; }
__LOCAL int (__LIBCCALL fallocate)(int __UNUSED(__fd), int __UNUSED(__mode), __off_t __UNUSED(__offset), __off_t __UNUSED(__len)) { return 0; }
#ifdef __USE_LARGEFILE64
__LOCAL int (__LIBCCALL fallocate64)(int __UNUSED(__fd), int __UNUSED(__mode), __off64_t __UNUSED(__offset), __off64_t __UNUSED(__len)) { return 0; }
#endif /* __USE_LARGEFILE64 */
#else /* __DOS_COMPAT__ */
struct iovec;
__LIBC __ssize_t (__LIBCCALL readahead)(int __fd, __off64_t __offset, __size_t __count);
__LIBC int (__LIBCCALL sync_file_range)(int __fd, __off64_t __offset, __off64_t __count, unsigned int __flags);
__LIBC __PORT_NODOS __ssize_t (__LIBCCALL vmsplice)(int __fdout, struct iovec const *__iov, __size_t __count, unsigned int __flags);
__LIBC __PORT_NODOS __ssize_t (__LIBCCALL splice)(int __fdin, __off64_t *__offin, int __fdout, __off64_t *__offout, __size_t __len, unsigned int __flags);
__LIBC __PORT_NODOS __ssize_t (__LIBCCALL tee)(int __fdin, int __fdout, __size_t __len, unsigned int __flags);
__LIBC __PORT_NODOS int (__LIBCCALL name_to_handle_at)(int __dfd, char const *__name, struct file_handle *__handle, int *__mnt_id, int __flags);
__LIBC __PORT_NODOS int (__LIBCCALL open_by_handle_at)(int __mountdirfd, struct file_handle *__handle, int __flags);
__REDIRECT_FS_FUNC(__LIBC,,int,__LIBCCALL,fallocate,(int __fd, int __mode, __FS_TYPE(off) __offset, __FS_TYPE(off) __len),fallocate,(__fd,__mode,__offset,__len))
#ifdef __USE_LARGEFILE64
__LIBC int (__LIBCCALL fallocate64)(int __fd, int __mode, __off64_t __offset, __off64_t __len);
#endif /* __USE_LARGEFILE64 */
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_GNU */

__SYSDECL_END

#endif /* !_BITS_FCNTL_LINUX_H */
