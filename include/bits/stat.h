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
#include <hybrid/timespec.h>


__DECL_BEGIN

#if defined(__PE__) && defined(__USE_DOS)
/* DOS-filesystem mode while hosting a PE binary.
 * >> Where, we must maintain binary compatibility with DOS. */

/* Rename all the DOS stat-buffers below to their proper names. */
#define __dos_stat       stat
#define __dos_stat32     _stat32
#define __dos_stat32i64  _stat32i64
#define __dos_stat64i32  _stat64i32
#define __dos_stat64     _stat64
#else /* __PE__ && __USE_DOS */

#undef _STATBUF_ST_TIM
#undef _STATBUF_ST_TIME
#undef _STATBUF_ST_NSEC
#undef _STATBUF_ST_TIMESPEC
#undef _STATBUF_ST_BLKSIZE
#undef _STATBUF_ST_RDEV


/* Optional stat features. */
#ifdef __USE_XOPEN2K8
#define _STATBUF_ST_TIM
#endif
#define _STATBUF_ST_TIME
#define _STATBUF_ST_NSEC
#define _STATBUF_ST_TIMESPEC /* Apple extension? */
#define _STATBUF_ST_BLKSIZE
#define _STATBUF_ST_RDEV

#ifdef __USE_DOS
/* DOS doesn't have these. */
#undef _STATBUF_ST_BLKSIZE
#undef _STATBUF_ST_NSEC
#undef _STATBUF_ST_TIM
#undef _STATBUF_ST_TIMESPEC
#endif

#define __STAT_TIMESPEC32_MEMB(id,suffix) \
        struct __timespec32 __##id##tim##suffix; \
        __IFTHEN(_STATBUF_ST_TIM)(struct __timespec32 id##tim##suffix;) \
        __IFTHEN(_STATBUF_ST_TIMESPEC)(struct __timespec32 id##timespec##suffix;) \
struct{ __IFELSE(_STATBUF_ST_TIME)(__time32_t __##id##time##suffix;) \
        __IFTHEN(_STATBUF_ST_TIME)(__time32_t id##time##suffix;) \
        __IFTHEN(_STATBUF_ST_NSEC)(__syscall_ulong_t id##timensec##suffix;) };
#define __STAT_TIMESPEC64_MEMB(id,suffix) \
        struct __timespec64 __##id##tim##suffix; \
        __IFTHEN(_STATBUF_ST_TIM)(struct __timespec64 id##tim##suffix;) \
        __IFTHEN(_STATBUF_ST_TIMESPEC)(struct __timespec64 id##timespec##suffix;) \
struct{ __IFELSE(_STATBUF_ST_TIME)(__time64_t __##id##time##suffix;) \
        __IFTHEN(_STATBUF_ST_TIME)(__time64_t id##time##suffix;) \
        __IFTHEN(_STATBUF_ST_NSEC)(__syscall_ulong_t id##timensec##suffix;) };

#ifdef __USE_TIME_BITS64
#ifdef __USE_KOS
#   define __STAT_TIMESPEC32(id) union{ __STAT_TIMESPEC32_MEMB(st_##id,32) }
#   define __STAT_TIMESPEC64(id) union{ __STAT_TIMESPEC64_MEMB(st_##id,) __STAT_TIMESPEC64_MEMB(st_##id,64) }
#else /* __USE_KOS */
#   define __STAT_TIMESPEC32(id) struct __timespec32 __st_##id##tim32
#   define __STAT_TIMESPEC64(id) union{ __STAT_TIMESPEC64_MEMB(st_##id,) }
#endif /* !__USE_KOS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_KOS
#   define __STAT_TIMESPEC32(id) union{ __STAT_TIMESPEC32_MEMB(st_##id,) __STAT_TIMESPEC32_MEMB(st_##id,32) }
#   define __STAT_TIMESPEC64(id) union{ __STAT_TIMESPEC64_MEMB(st_##id,64) }
#else /* __USE_KOS */
#   define __STAT_TIMESPEC32(id) union{ __STAT_TIMESPEC32_MEMB(st_##id,) }
#   define __STAT_TIMESPEC64(id) struct __timespec64 __st_##id##tim64
#endif /* !__USE_KOS */
#endif /* !__USE_TIME_BITS64 */

#ifdef __USE_DOS
struct __kos_stat
#else
struct stat
#endif
{
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
#ifdef _STATBUF_ST_RDEV
    __dev_t           st_rdev;
#else
    __dev_t         __st_rdev;
#endif
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
#ifdef _STATBUF_ST_BLKSIZE
    __blksize_t       st_blksize;
#else
    __blksize_t     __st_blksize;
#endif
union{
    __FS_TYPE(blkcnt) st_blocks;
#ifdef __USE_KOS
    __blkcnt32_t      st_blocks32;
    __blkcnt64_t      st_blocks64;
#else
    __blkcnt64_t    __st_blocks64;
#endif
};
    __STAT_TIMESPEC32(a);
    __STAT_TIMESPEC32(m);
    __STAT_TIMESPEC32(c);
    __STAT_TIMESPEC64(a);
    __STAT_TIMESPEC64(m);
    __STAT_TIMESPEC64(c);
};



#ifdef __USE_LARGEFILE64
#ifdef __USE_DOS
struct __kos_stat64
#else
struct stat64
#endif
{
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
#ifdef _STATBUF_ST_RDEV
    __dev_t           st_rdev;
#else
    __dev_t         __st_rdev;
#endif
#ifdef __USE_KOS
union{
    __pos64_t         st_size;
    __pos32_t         st_size32;
    __pos64_t         st_size64;
};
#else
    __off64_t         st_size;
#endif
#ifdef _STATBUF_ST_BLKSIZE
    __blksize_t       st_blksize;
#else
    __blksize_t     __st_blksize;
#endif
#ifdef __USE_KOS
union{
    __blkcnt64_t      st_blocks;
    __blkcnt32_t      st_blocks32;
    __blkcnt64_t      st_blocks64;
};
#else
    __blkcnt64_t      st_blocks;
#endif
    __STAT_TIMESPEC32(a);
    __STAT_TIMESPEC32(m);
    __STAT_TIMESPEC32(c);
    __STAT_TIMESPEC64(a);
    __STAT_TIMESPEC64(m);
    __STAT_TIMESPEC64(c);
};
#endif /* __USE_LARGEFILE64 */

#undef __STAT_TIMESPEC32_MEMB
#undef __STAT_TIMESPEC64_MEMB
#undef __STAT_TIMESPEC32
#undef __STAT_TIMESPEC64

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

#ifdef __USE_DOS
#ifndef _STAT_DEFINED
#define _STAT_DEFINED 1

/* DOS-syntax-compatible stat structures, following KOS data layout.
 * >> These are used in ELF-hosted DOS-mode, when it is clear that
 *    the application was compiled for KOS, meaning that binary
 *    compatibility with DOS is not mandatory. */

struct stat {
union{__typedef_dev_t st_dev;   __dev_t __st_dev; };
union{__typedef_ino_t st_ino;   __ino64_t __st_ino; };
union{__uint16_t      st_mode;  __mode_t __st_mode; };
union{__int16_t       st_nlink; __nlink_t __st_nlink;};
union{__int16_t       st_uid;   __uid_t __st_uid;};
union{__int16_t       st_gid;   __gid_t __st_gid;};
union{__typedef_dev_t st_rdev;  __dev_t __st_rdev;};
union{__typedef_off_t st_size;  __pos64_t __st_size;};
    __blksize_t     __st_blksize;
    __blkcnt64_t    __st_blocks;
#ifdef __USE_TIME_BITS64
    struct __timespec32 __st_atim32;
    struct __timespec32 __st_mtim32;
    struct __timespec32 __st_ctim32;
union{__time64_t      st_atime; struct __timespec64 __st_atim64;};
union{__time64_t      st_mtime; struct __timespec64 __st_mtim64;};
union{__time64_t      st_ctime; struct __timespec64 __st_ctim64;};
#else /* __USE_TIME_BITS64 */
union{__time32_t      st_atime; struct __timespec32 __st_atim32;};
union{__time32_t      st_mtime; struct __timespec32 __st_mtim32;};
union{__time32_t      st_ctime; struct __timespec32 __st_ctim32;};
    struct __timespec64 __st_atim64;
    struct __timespec64 __st_mtim64;
    struct __timespec64 __st_ctim64;
#endif /* !__USE_TIME_BITS64 */
};

struct _stat32 {
union{__typedef_dev_t st_dev;   __dev_t __st_dev; };
union{__typedef_ino_t st_ino;   __ino64_t __st_ino; };
union{__uint16_t      st_mode;  __mode_t __st_mode; };
union{__int16_t       st_nlink; __nlink_t __st_nlink;};
union{__int16_t       st_uid;   __uid_t __st_uid;};
union{__int16_t       st_gid;   __gid_t __st_gid;};
union{__typedef_dev_t st_rdev;  __dev_t __st_rdev;};
union{__typedef_off_t st_size;  __pos64_t __st_size;};
    __blksize_t     __st_blksize;
    __blkcnt64_t    __st_blocks;
union{__time32_t      st_atime; struct __timespec32 __st_atim32;};
union{__time32_t      st_mtime; struct __timespec32 __st_mtim32;};
union{__time32_t      st_ctime; struct __timespec32 __st_ctim32;};
    struct __timespec64 __st_atim64;
    struct __timespec64 __st_mtim64;
    struct __timespec64 __st_ctim64;
};

struct _stat32i64 {
union{__typedef_dev_t st_dev;   __dev_t __st_dev; };
union{__typedef_ino_t st_ino;   __ino64_t __st_ino; };
union{__uint16_t      st_mode;  __mode_t __st_mode; };
union{__int16_t       st_nlink; __nlink_t __st_nlink;};
union{__int16_t       st_uid;   __uid_t __st_uid;};
union{__int16_t       st_gid;   __gid_t __st_gid;};
union{__typedef_dev_t st_rdev;  __dev_t __st_rdev;};
union{__INT64_TYPE__  st_size;  __pos64_t __st_size;};
    __blksize_t     __st_blksize;
    __blkcnt64_t    __st_blocks;
union{__time32_t      st_atime; struct __timespec32 __st_atim32;};
union{__time32_t      st_mtime; struct __timespec32 __st_mtim32;};
union{__time32_t      st_ctime; struct __timespec32 __st_ctim32;};
    struct __timespec64 __st_atim64;
    struct __timespec64 __st_mtim64;
    struct __timespec64 __st_ctim64;
};

struct _stat64i32 {
union{__typedef_dev_t st_dev;   __dev_t __st_dev; };
union{__typedef_ino_t st_ino;   __ino64_t __st_ino; };
union{__uint16_t      st_mode;  __mode_t __st_mode; };
union{__int16_t       st_nlink; __nlink_t __st_nlink;};
union{__int16_t       st_uid;   __uid_t __st_uid;};
union{__int16_t       st_gid;   __gid_t __st_gid;};
union{__typedef_dev_t st_rdev;  __dev_t __st_rdev;};
union{__typedef_off_t st_size;  __pos64_t __st_size;};
    __blksize_t     __st_blksize;
    __blkcnt64_t    __st_blocks;
    struct __timespec32 __st_atim32;
    struct __timespec32 __st_mtim32;
    struct __timespec32 __st_ctim32;
union{__time64_t      st_atime; struct __timespec64 __st_atim64;};
union{__time64_t      st_mtime; struct __timespec64 __st_mtim64;};
union{__time64_t      st_ctime; struct __timespec64 __st_ctim64;};
};

struct _stat64 {
union{__typedef_dev_t st_dev;   __dev_t __st_dev; };
union{__typedef_ino_t st_ino;   __ino64_t __st_ino; };
union{__uint16_t      st_mode;  __mode_t __st_mode; };
union{__int16_t       st_nlink; __nlink_t __st_nlink;};
union{__int16_t       st_uid;   __uid_t __st_uid;};
union{__int16_t       st_gid;   __gid_t __st_gid;};
union{__typedef_dev_t st_rdev;  __dev_t __st_rdev;};
union{__INT64_TYPE__  st_size;  __pos64_t __st_size;};
    __blksize_t     __st_blksize;
    __blkcnt64_t    __st_blocks;
    struct __timespec32 __st_atim32;
    struct __timespec32 __st_mtim32;
    struct __timespec32 __st_ctim32;
union{__time64_t      st_atime; struct __timespec64 __st_atim64;};
union{__time64_t      st_mtime; struct __timespec64 __st_mtim64;};
union{__time64_t      st_ctime; struct __timespec64 __st_ctim64;};
};
#endif /* !_STAT_DEFINED */
#endif /* __USE_DOS */
#endif /* !__PE__ || !__USE_DOS */

#if defined(__USE_DOS) || defined(__BUILDING_LIBC)
#if defined(__PE__) || defined(__BUILDING_LIBC)
/* Define the binary layout of DOS's stat buffers. */
struct __dos_stat {
 __dos_dev_t    st_dev;
 __dos_ino_t    st_ino;
 __uint16_t     st_mode;
 __int16_t      st_nlink;
 __int16_t      st_uid;
 __int16_t      st_gid;
 __dos_dev_t    st_rdev;
 __dos_off_t    st_size;
 __TM_TYPE(time) st_atime;
 __TM_TYPE(time) st_mtime;
 __TM_TYPE(time) st_ctime;
};
struct __dos_stat32 {
 __dos_dev_t    st_dev;
 __dos_ino_t    st_ino;
 __uint16_t     st_mode;
 __int16_t      st_nlink;
 __int16_t      st_uid;
 __int16_t      st_gid;
 __dos_dev_t    st_rdev;
 __dos_off_t    st_size;
 __time32_t     st_atime;
 __time32_t     st_mtime;
 __time32_t     st_ctime;
};
struct __dos_stat32i64 {
 __dos_dev_t    st_dev;
 __dos_ino_t    st_ino;
 __uint16_t     st_mode;
 __int16_t      st_nlink;
 __int16_t      st_uid;
 __int16_t      st_gid;
 __dos_dev_t    st_rdev;
 __INT64_TYPE__ st_size;
 __time32_t     st_atime;
 __time32_t     st_mtime;
 __time32_t     st_ctime;
};
struct __dos_stat64i32 {
 __dos_dev_t    st_dev;
 __dos_ino_t    st_ino;
 __uint16_t     st_mode;
 __int16_t      st_nlink;
 __int16_t      st_uid;
 __int16_t      st_gid;
 __dos_dev_t    st_rdev;
union{
 __dos_off_t    st_size;
 __INT64_TYPE__ __st_pad; /* I think this is correct? */
};
 __time64_t     st_atime;
 __time64_t     st_mtime;
 __time64_t     st_ctime;
};
struct __dos_stat64 {
 __dos_dev_t    st_dev;
 __dos_ino_t    st_ino;
 __uint16_t     st_mode;
 __int16_t      st_nlink;
 __int16_t      st_uid;
 __int16_t      st_gid;
 __dos_dev_t    st_rdev;
 __INT64_TYPE__ st_size;
 __time64_t     st_atime;
 __time64_t     st_mtime;
 __time64_t     st_ctime;
};
#endif /* __PE__ || __BUILDING_LIBC */

#ifndef __BUILDING_LIBC
/* Define alias macros. */
#define __stat64    _stat64
#ifdef __USE_TIME_BITS64
#define _fstat      _fstat64i32
#define _fstati64   _fstat64
#define _stat       _stat64i32
#define _stati64    _stat64
#define _wstat      _wstat64i32
#define _wstati64   _wstat64
#else /* __USE_TIME_BITS64 */
#define _fstat      _fstat32
#define _fstati64   _fstat32i64
#define _stat       _stat32
#define _stati64    _stat32i64
#define _wstat      _wstat32
#define _wstati64   _wstat32i64
#endif /* !__USE_TIME_BITS64 */
#endif /* !__BUILDING_LIBC */

#endif /* __USE_DOS */

__DECL_END

#endif /* !_BITS_STAT_H */
