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
#ifndef _BITS_STAT_H
#define _BITS_STAT_H 1

#include <__stdinc.h>
#include <bits/types.h>
#ifdef __USE_XOPEN2K8
#include <hybrid/timespec.h>
#endif


__DECL_BEGIN

struct stat {
    __dev_t           st_dev;
union{
    __FS_TYPE(ino)    st_ino;
#ifdef __USE_KOS
    __ino32_t         st_ino32;
    __ino64_t         st_ino64;
#else
    __ino64_t       __st_ino64;
#endif
};
    __mode_t          st_mode;
    __nlink_t         st_nlink;
    __uid_t           st_uid;
    __gid_t           st_gid;
    __dev_t           st_rdev;
union{
#ifdef __USE_KOS
    __FS_TYPE(pos)    st_size;
    __pos32_t         st_size32;
    __pos64_t         st_size64;
#else
    __FS_TYPE(off)    st_size;
    __pos64_t       __st_size64;
#endif
};
    __blksize_t       st_blksize;
union{
    __FS_TYPE(blkcnt) st_blocks;
#ifdef __USE_KOS
    __blkcnt32_t      st_blocks32;
    __blkcnt64_t      st_blocks64;
#else
    __blkcnt64_t    __st_blocks64;
#endif
};
#ifdef __USE_XOPEN2K8
#ifdef __USE_TIME_BITS64
#ifdef __USE_KOS
    struct __timespec32   st_atim32;
    struct __timespec32   st_mtim32;
    struct __timespec32   st_ctim32;
#else /* __USE_KOS */
    struct __timespec32 __st_atim32;
    struct __timespec32 __st_mtim32;
    struct __timespec32 __st_ctim32;
#endif /* !__USE_KOS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_KOS
union{struct __timespec32 st_atim;
      struct __timespec32 st_atim32;};
union{struct __timespec32 st_mtim;
      struct __timespec32 st_mtim32;};
union{struct __timespec32 st_ctim;
      struct __timespec32 st_ctim32;};
#else /* __USE_KOS */
    struct __timespec32 st_atim;
    struct __timespec32 st_mtim;
    struct __timespec32 st_ctim;
#endif /* !__USE_KOS */
#endif /* !__USE_TIME_BITS64 */
#else /* __USE_XOPEN2K8 */
#ifdef __USE_TIME_BITS64
#ifdef __USE_KOS
    __time32_t        st_atime32;
    __syscall_ulong_t st_atimensec32;
    __time32_t        st_mtime32;
    __syscall_ulong_t st_mtimensec32;
    __time32_t        st_ctime32;
    __syscall_ulong_t st_ctimensec32;
#else /* __USE_KOS */
    __time32_t        __st_atime32;
    __syscall_ulong_t __st_atimensec32;
    __time32_t        __st_mtime32;
    __syscall_ulong_t __st_mtimensec32;
    __time32_t        __st_ctime32;
    __syscall_ulong_t __st_ctimensec32;
#endif /* !__USE_KOS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_KOS
union{__time32_t      st_atime;
      __time32_t      st_atime32;};
union{__syscall_ulong_t st_atimensec;
      __syscall_ulong_t st_atimensec32;};
union{__time32_t      st_mtime;
      __time32_t      st_mtime32;};
union{__syscall_ulong_t st_mtimensec;
      __syscall_ulong_t st_mtimensec32;};
union{__time32_t      st_ctime;
      __time32_t      st_ctime32;};
union{__syscall_ulong_t st_ctimensec;
      __syscall_ulong_t st_ctimensec32;};
#else
    __time32_t        st_atime;
    __syscall_ulong_t st_atimensec;
    __time32_t        st_mtime;
    __syscall_ulong_t st_mtimensec;
    __time32_t        st_ctime;
    __syscall_ulong_t st_ctimensec;
#endif
#endif /* !__USE_TIME_BITS64 */
#endif /* !__USE_XOPEN2K8 */
#ifdef __USE_XOPEN2K8
#ifndef __USE_TIME_BITS64
#ifdef __USE_KOS
    struct __timespec64   st_atim64;
    struct __timespec64   st_mtim64;
    struct __timespec64   st_ctim64;
#else /* __USE_KOS */
    struct __timespec64 __st_atim64;
    struct __timespec64 __st_mtim64;
    struct __timespec64 __st_ctim64;
#endif /* !__USE_KOS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_KOS
union{struct __timespec64 st_atim;
      struct __timespec64 st_atim64;};
union{struct __timespec64 st_mtim;
      struct __timespec64 st_mtim64;};
union{struct __timespec64 st_ctim;
      struct __timespec64 st_ctim64;};
#else /* __USE_KOS */
    struct __timespec64 st_atim;
    struct __timespec64 st_mtim;
    struct __timespec64 st_ctim;
#endif /* !__USE_KOS */
#endif /* !__USE_TIME_BITS64 */
#else /* __USE_XOPEN2K8 */
#ifndef __USE_TIME_BITS64
#ifdef __USE_KOS
    __time64_t        st_atime64;
    __syscall_ulong_t st_atimensec64;
    __time64_t        st_mtime64;
    __syscall_ulong_t st_mtimensec64;
    __time64_t        st_ctime64;
    __syscall_ulong_t st_ctimensec64;
#else /* __USE_KOS */
    __time64_t        __st_atime64;
    __syscall_ulong_t __st_atimensec64;
    __time64_t        __st_mtime64;
    __syscall_ulong_t __st_mtimensec64;
    __time64_t        __st_ctime64;
    __syscall_ulong_t __st_ctimensec64;
#endif /* !__USE_KOS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_KOS
union{__time64_t      st_atime;
      __time64_t      st_atime64;};
union{__syscall_ulong_t st_atimensec;
      __syscall_ulong_t st_atimensec64;};
union{__time64_t      st_mtime;
      __time64_t      st_mtime64;};
union{__syscall_ulong_t st_mtimensec;
      __syscall_ulong_t st_mtimensec64;};
union{__time64_t      st_ctime;
      __time64_t      st_ctime64;};
union{__syscall_ulong_t st_ctimensec;
      __syscall_ulong_t st_ctimensec64;};
#else
    __time64_t        st_atime;
    __syscall_ulong_t st_atimensec;
    __time64_t        st_mtime;
    __syscall_ulong_t st_mtimensec;
    __time64_t        st_ctime;
    __syscall_ulong_t st_ctimensec;
#endif
#endif /* !__USE_TIME_BITS64 */
#endif /* !__USE_XOPEN2K8 */
};



#ifdef __USE_LARGEFILE64
struct stat64 {
    __dev_t           st_dev;
#ifdef __USE_KOS
union{
    __ino64_t         st_ino;
    __ino32_t         st_ino32;
    __ino64_t         st_ino64;
};
#else
    __ino64_t         st_ino;
#endif
    __mode_t          st_mode;
    __nlink_t         st_nlink;
    __uid_t           st_uid;
    __gid_t           st_gid;
    __dev_t           st_rdev;
#ifdef __USE_KOS
union{
    __pos64_t         st_size;
    __pos32_t         st_size32;
    __pos64_t         st_size64;
};
#else
    __off64_t         st_size;
#endif
    __blksize_t       st_blksize;
#ifdef __USE_KOS
union{
    __blkcnt64_t      st_blocks;
    __blkcnt32_t      st_blocks32;
    __blkcnt64_t      st_blocks64;
};
#else
    __blkcnt64_t      st_blocks;
#endif
#ifdef __USE_XOPEN2K8
#ifdef __USE_TIME_BITS64
#ifdef __USE_KOS
    struct __timespec32   st_atim32;
    struct __timespec32   st_mtim32;
    struct __timespec32   st_ctim32;
#else /* __USE_KOS */
    struct __timespec32 __st_atim32;
    struct __timespec32 __st_mtim32;
    struct __timespec32 __st_ctim32;
#endif /* !__USE_KOS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_KOS
union{struct __timespec32 st_atim;
      struct __timespec32 st_atim32;};
union{struct __timespec32 st_mtim;
      struct __timespec32 st_mtim32;};
union{struct __timespec32 st_ctim;
      struct __timespec32 st_ctim32;};
#else /* __USE_KOS */
    struct __timespec32 st_atim;
    struct __timespec32 st_mtim;
    struct __timespec32 st_ctim;
#endif /* !__USE_KOS */
#endif /* !__USE_TIME_BITS64 */
#else /* __USE_XOPEN2K8 */
#ifdef __USE_TIME_BITS64
#ifdef __USE_KOS
    __time32_t        st_atime32;
    __syscall_ulong_t st_atimensec32;
    __time32_t        st_mtime32;
    __syscall_ulong_t st_mtimensec32;
    __time32_t        st_ctime32;
    __syscall_ulong_t st_ctimensec32;
#else /* __USE_KOS */
    __time32_t        __st_atime32;
    __syscall_ulong_t __st_atimensec32;
    __time32_t        __st_mtime32;
    __syscall_ulong_t __st_mtimensec32;
    __time32_t        __st_ctime32;
    __syscall_ulong_t __st_ctimensec32;
#endif /* !__USE_KOS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_KOS
union{__time32_t      st_atime;
      __time32_t      st_atime32;};
union{__syscall_ulong_t st_atimensec;
      __syscall_ulong_t st_atimensec32;};
union{__time32_t      st_mtime;
      __time32_t      st_mtime32;};
union{__syscall_ulong_t st_mtimensec;
      __syscall_ulong_t st_mtimensec32;};
union{__time32_t      st_ctime;
      __time32_t      st_ctime32;};
union{__syscall_ulong_t st_ctimensec;
      __syscall_ulong_t st_ctimensec32;};
#else
    __time32_t        st_atime;
    __syscall_ulong_t st_atimensec;
    __time32_t        st_mtime;
    __syscall_ulong_t st_mtimensec;
    __time32_t        st_ctime;
    __syscall_ulong_t st_ctimensec;
#endif
#endif /* !__USE_TIME_BITS64 */
#endif /* !__USE_XOPEN2K8 */
#ifdef __USE_XOPEN2K8
#ifndef __USE_TIME_BITS64
#ifdef __USE_KOS
    struct __timespec64   st_atim64;
    struct __timespec64   st_mtim64;
    struct __timespec64   st_ctim64;
#else /* __USE_KOS */
    struct __timespec64 __st_atim64;
    struct __timespec64 __st_mtim64;
    struct __timespec64 __st_ctim64;
#endif /* !__USE_KOS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_KOS
union{struct __timespec64 st_atim;
      struct __timespec64 st_atim64;};
union{struct __timespec64 st_mtim;
      struct __timespec64 st_mtim64;};
union{struct __timespec64 st_ctim;
      struct __timespec64 st_ctim64;};
#else /* __USE_KOS */
    struct __timespec64 st_atim;
    struct __timespec64 st_mtim;
    struct __timespec64 st_ctim;
#endif /* !__USE_KOS */
#endif /* !__USE_TIME_BITS64 */
#else /* __USE_XOPEN2K8 */
#ifndef __USE_TIME_BITS64
#ifdef __USE_KOS
    __time64_t        st_atime64;
    __syscall_ulong_t st_atimensec64;
    __time64_t        st_mtime64;
    __syscall_ulong_t st_mtimensec64;
    __time64_t        st_ctime64;
    __syscall_ulong_t st_ctimensec64;
#else /* __USE_KOS */
    __time64_t        __st_atime64;
    __syscall_ulong_t __st_atimensec64;
    __time64_t        __st_mtime64;
    __syscall_ulong_t __st_mtimensec64;
    __time64_t        __st_ctime64;
    __syscall_ulong_t __st_ctimensec64;
#endif /* !__USE_KOS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_KOS
union{__time64_t      st_atime;
      __time64_t      st_atime64;};
union{__syscall_ulong_t st_atimensec;
      __syscall_ulong_t st_atimensec64;};
union{__time64_t      st_mtime;
      __time64_t      st_mtime64;};
union{__syscall_ulong_t st_mtimensec;
      __syscall_ulong_t st_mtimensec64;};
union{__time64_t      st_ctime;
      __time64_t      st_ctime64;};
union{__syscall_ulong_t st_ctimensec;
      __syscall_ulong_t st_ctimensec64;};
#else
    __time64_t        st_atime;
    __syscall_ulong_t st_atimensec;
    __time64_t        st_mtime;
    __syscall_ulong_t st_mtimensec;
    __time64_t        st_ctime;
    __syscall_ulong_t st_ctimensec;
#endif
#endif /* !__USE_TIME_BITS64 */
#endif /* !__USE_XOPEN2K8 */
};
#endif

#ifdef __USE_XOPEN2K8
#   define st_atime   st_atim.tv_sec
#   define st_mtime   st_mtim.tv_sec
#   define st_ctime   st_ctim.tv_sec
#endif /* !__USE_XOPEN2K8 */

#define _STATBUF_ST_BLKSIZE
#define _STATBUF_ST_RDEV
#define _STATBUF_ST_NSEC

#define __S_IFMT           0170000 /*< These bits determine file type. */
#define __S_IFDIR          0040000 /*< Directory. */
#define __S_IFCHR          0020000 /*< Character device. */
#define __S_IFBLK          0060000 /*< Block device. */
#define __S_IFREG          0100000 /*< Regular file. */
#define __S_IFIFO          0010000 /*< FIFO. */
#define __S_IFLNK          0120000 /*< Symbolic link. */
#define __S_IFSOCK         0140000 /*< Socket. */

#define __S_ISDIR(x)  (((x)&__S_IFMT) == __S_IFDIR)
#define __S_ISCHR(x)  (((x)&__S_IFMT) == __S_IFCHR)
#define __S_ISBLK(x)  (((x)&__S_IFMT) == __S_IFBLK)
#define __S_ISREG(x)  (((x)&__S_IFMT) == __S_IFREG)
#define __S_ISFIFO(x) (((x)&__S_IFMT) == __S_IFIFO)
#define __S_ISLNK(x)  (((x)&__S_IFMT) == __S_IFLNK)
#define __S_ISDEV(x)  (((x)&0130000) == 0020000) /* __S_IFCHR|__S_IFBLK */


#define __S_TYPEISMQ(buf)  ((buf)->st_mode - (buf)->st_mode)
#define __S_TYPEISSEM(buf) ((buf)->st_mode - (buf)->st_mode)
#define __S_TYPEISSHM(buf) ((buf)->st_mode - (buf)->st_mode)

#define __S_ISUID  04000 /*< Set user ID on execution. */
#define __S_ISGID  02000 /*< Set group ID on execution. */
#define __S_ISVTX  01000 /*< Save swapped text after use (sticky). */
#define __S_IREAD  00400 /*< Read by owner. */
#define __S_IWRITE 00200 /*< Write by owner. */
#define __S_IEXEC  00100 /*< Execute by owner. */

#ifdef __USE_ATFILE
#   define UTIME_NOW   ((1l << 30) - 1l)
#   define UTIME_OMIT  ((1l << 30) - 2l)
#endif

__DECL_END

#endif /* !_BITS_STAT_H */
