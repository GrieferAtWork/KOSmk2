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


__SYSDECL_BEGIN

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("stat")
#ifdef __USE_LARGEFILE64
#pragma push_macro("stat64")
#endif /* __USE_LARGEFILE64 */
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#undef stat
#ifdef __USE_LARGEFILE64
#undef stat64
#endif /* __USE_LARGEFILE64 */

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


#undef _STATBUF_ST_TIM      /* .st_atim, .st_ctim, .st_mtim */
#undef _STATBUF_ST_TIME     /* .st_atime, .st_ctime, .st_mtime */
#undef _STATBUF_ST_NSEC     /* .st_atimensec, .st_ctimensec, .st_mtimensec */
#undef _STATBUF_ST_TIMESPEC /* .st_atimespec, .st_ctimespec, .st_mtimespec */
#undef _STATBUF_ST_BLKSIZE  /* .st_blksize */
#undef _STATBUF_ST_RDEV     /* .st_rdev */
#undef _STATBUF_ST_INO32    /* .st_ino32 */
#undef _STATBUF_ST_INO64    /* .st_ino64 */

#ifdef __USE_KOS
#define _STATBUF_ST_INO32
#define _STATBUF_ST_INO64
#endif

#ifdef __DOS_COMPAT__
/* DOS compatibility mode. */

/* Optional stat features. */
#define _STATBUF_ST_TIME     /* Always defined. */
#define _STATBUF_ST_RDEV
#undef _STATBUF_ST_INO32     /* DOS only defines 16-bit INode numbers. */
#undef _STATBUF_ST_INO64

#define __dos_stat       stat
#if defined(__USE_TIME_BITS64) || 1
#define __dos_stat32i64  _stat32i64
#define __dos_stat64     stat64
#else /* __USE_TIME_BITS64 */
#define __dos_stat64     _stat64
#define __dos_stat32i64  stat64
#endif /* !__USE_TIME_BITS64 */
#define __dos_stat32     _stat32
#define __dos_stat64i32  _stat64i32
#elif defined(__GLC_COMPAT__)
/* GLibc compatibility mode. */

/* Optional stat features. */
#ifdef __USE_XOPEN2K8
#define _STATBUF_ST_TIM
#endif
#define _STATBUF_ST_TIME     /* Always defined. */
#define _STATBUF_ST_NSEC
#define _STATBUF_ST_TIMESPEC /* Apple extension? */
#define _STATBUF_ST_BLKSIZE
#define _STATBUF_ST_RDEV

#define __glc_stat       stat
#ifdef __USE_LARGEFILE64
#define __glc_stat64     stat64
#endif /* __USE_LARGEFILE64 */
#else /* __DOS_COMPAT__ */

/* Optional stat features. */
#ifdef __USE_XOPEN2K8
#define _STATBUF_ST_TIM
#endif
#define _STATBUF_ST_TIME     /* Always defined. */
#define _STATBUF_ST_NSEC
#define _STATBUF_ST_TIMESPEC /* Apple extension? */
#define _STATBUF_ST_BLKSIZE
#define _STATBUF_ST_RDEV

#define __STAT_TIMESPEC32_MEMB(id,suffix) \
        struct __timespec32 __##id##tim##suffix; \
        __IFTHEN(_STATBUF_ST_TIM)(struct __timespec32 id##tim##suffix;) \
        __IFTHEN(_STATBUF_ST_TIMESPEC)(struct __timespec32 id##timespec##suffix;) \
struct{ __time32_t id##time##suffix; \
        __IFTHEN(_STATBUF_ST_NSEC)(__syscall_ulong_t id##timensec##suffix;) };
#define __STAT_TIMESPEC64_MEMB(id,suffix) \
        struct __timespec64 __##id##tim##suffix; \
        __IFTHEN(_STATBUF_ST_TIM)(struct __timespec64 id##tim##suffix;) \
        __IFTHEN(_STATBUF_ST_TIMESPEC)(struct __timespec64 id##timespec##suffix;) \
struct{ __time64_t id##time##suffix; \
        __IFTHEN(_STATBUF_ST_NSEC)(__syscall_ulong_t id##timensec##suffix;) };

#ifdef __USE_TIME_BITS64
#ifdef __USE_KOS
#   define __STAT_TIMESPEC32(id)     union{ __STAT_TIMESPEC32_MEMB(st_##id,32) }
#   define __STAT_TIMESPEC64(id)     union{ __STAT_TIMESPEC64_MEMB(st_##id,) __STAT_TIMESPEC64_MEMB(__st_##id,64) }
#   define __STAT_TIMESPEC64_ALT(id) union{ __STAT_TIMESPEC64_MEMB(st_##id,) __STAT_TIMESPEC64_MEMB(st_##id,64) }
#else /* __USE_KOS */
#   define __STAT_TIMESPEC32(id)     struct __timespec32 __st_##id##tim32
#   define __STAT_TIMESPEC64(id)     union{ __STAT_TIMESPEC64_MEMB(st_##id,) }
#   define __STAT_TIMESPEC64_ALT(id) union{ __STAT_TIMESPEC64_MEMB(st_##id,) }
#endif /* !__USE_KOS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_KOS
#   define __STAT_TIMESPEC32(id)     union{ __STAT_TIMESPEC32_MEMB(st_##id,) __STAT_TIMESPEC32_MEMB(st_##id,32) }
#   define __STAT_TIMESPEC64(id)     union{ __STAT_TIMESPEC64_MEMB(__st_##id,64) }
#   define __STAT_TIMESPEC64_ALT(id) union{ __STAT_TIMESPEC64_MEMB(st_##id,64) }
#else /* __USE_KOS */
#   define __STAT_TIMESPEC32(id)     union{ __STAT_TIMESPEC32_MEMB(st_##id,) }
#   define __STAT_TIMESPEC64(id)     struct __timespec64 __st_##id##tim64
#   define __STAT_TIMESPEC64_ALT(id) struct __timespec64 __st_##id##tim64
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
#endif
    __ino64_t       __st_ino64;
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
#else
    __FS_TYPE(off)    st_size;
#endif
    __pos64_t       __st_size64;
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
#endif
    __blkcnt64_t    __st_blocks64;
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
    __STAT_TIMESPEC64_ALT(a);
    __STAT_TIMESPEC64_ALT(m);
    __STAT_TIMESPEC64_ALT(c);
};
#endif /* __USE_LARGEFILE64 */

#undef __STAT_TIMESPEC32_MEMB
#undef __STAT_TIMESPEC64_MEMB
#undef __STAT_TIMESPEC64_ALT
#undef __STAT_TIMESPEC32
#undef __STAT_TIMESPEC64

#ifdef __USE_DOS

/* DOS doesn't have these. */
#undef _STATBUF_ST_BLKSIZE
#undef _STATBUF_ST_NSEC
#undef _STATBUF_ST_TIM
#undef _STATBUF_ST_TIMESPEC

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
#endif /* KOS... */

#if defined(__CRT_GLC) && \
   (defined(__GLC_COMPAT__) || defined(__BUILDING_LIBC))

struct __glc_stat {
    /* +0  */__UINT64_TYPE__   st_dev;
    /* +8  */__UINT32_TYPE__ __pad0;
#ifdef __USE_KOS
union{
    /* +12 */__UINT32_TYPE__   st_ino32;
#ifndef __USE_FILE_OFFSET64
    /* +12 */__UINT32_TYPE__   st_ino;
#endif /* !__USE_FILE_OFFSET64 */
};
#elif defined(__USE_FILE_OFFSET64)
    /* +12 */__UINT32_TYPE__ __st_ino32;
#else /* ... */
    /* +12 */__UINT32_TYPE__   st_ino;
#endif /* !... */
    /* +16 */__UINT32_TYPE__   st_mode;
    /* +20 */__UINT32_TYPE__   st_nlink;
    /* +24 */__UINT32_TYPE__   st_uid;
    /* +28 */__UINT32_TYPE__   st_gid;
    /* +32 */__UINT64_TYPE__   st_rdev;
    /* +40 */__UINT32_TYPE__ __pad1;
#ifdef __USE_FILE_OFFSET64
#ifdef __USE_KOS
union{
    /* +44 */__UINT64_TYPE__   st_size;
    /* +44 */__UINT32_TYPE__   st_size32;
    /* +44 */__UINT64_TYPE__   st_size64;
};
#else /* __USE_KOS */
    /* +44 */__INT64_TYPE__    st_size;
#endif /* !__USE_KOS */
    /* +52 */__INT32_TYPE__    st_blksize;
    /* +56 */__INT64_TYPE__    st_blocks;
#ifdef _STATBUF_ST_TIM
union{/* +64 */struct __timespec32 st_atim; struct{__time32_t st_atime; __syscall_ulong_t st_atimensec;};};
union{/* +72 */struct __timespec32 st_mtim; struct{__time32_t st_mtime; __syscall_ulong_t st_mtimensec;};};
union{/* +80 */struct __timespec32 st_ctim; struct{__time32_t st_ctime; __syscall_ulong_t st_ctimensec;};};
#else /* _STATBUF_ST_TIM */
    /* +64 */__time32_t st_atime; __syscall_ulong_t st_atimensec;
    /* +72 */__time32_t st_mtime; __syscall_ulong_t st_mtimensec;
    /* +80 */__time32_t st_ctime; __syscall_ulong_t st_ctimensec;
#endif /* !_STATBUF_ST_TIM */
    /* +88 */__UINT64_TYPE__   st_ino;
#else /* __USE_FILE_OFFSET64 */
#ifdef __USE_KOS
union{
    /* +44 */__INT32_TYPE__    st_size;
    /* +44 */__INT32_TYPE__    st_size32;
};
#else
    /* +44 */__INT32_TYPE__    st_size;
#endif
    /* +48 */__INT32_TYPE__    st_blksize;
    /* +52 */__INT32_TYPE__    st_blocks;
#ifdef _STATBUF_ST_TIM
union{/* +56 */struct __timespec32 st_atim; struct{__time32_t st_atime; __syscall_ulong_t st_atimensec;};};
union{/* +64 */struct __timespec32 st_mtim; struct{__time32_t st_mtime; __syscall_ulong_t st_mtimensec;};};
union{/* +72 */struct __timespec32 st_ctim; struct{__time32_t st_ctime; __syscall_ulong_t st_ctimensec;};};
#else /* _STATBUF_ST_TIM */
    /* +56 */__time32_t st_atime; __syscall_ulong_t st_atimensec;
    /* +64 */__time32_t st_mtime; __syscall_ulong_t st_mtimensec;
    /* +72 */__time32_t st_ctime; __syscall_ulong_t st_ctimensec;
#endif /* !_STATBUF_ST_TIM */
    /* +80 */__UINT32_TYPE__ __pad2;
    /* +84 */__UINT32_TYPE__ __pad3;
#endif /* !__USE_FILE_OFFSET64 */
};

#ifdef __USE_LARGEFILE64
struct __glc_stat64 {
    /* +0  */__UINT64_TYPE__   st_dev;
    /* +8  */__UINT32_TYPE__ __pad0;
#ifdef __USE_KOS
    /* +12 */__UINT32_TYPE__   st_ino32;
#else
    /* +12 */__UINT32_TYPE__ __st_ino32;
#endif
    /* +16 */__UINT32_TYPE__   st_mode;
    /* +20 */__UINT32_TYPE__   st_nlink;
    /* +24 */__UINT32_TYPE__   st_uid;
    /* +28 */__UINT32_TYPE__   st_gid;
    /* +32 */__UINT64_TYPE__   st_rdev;
    /* +40 */__UINT32_TYPE__ __pad1;
#ifdef __USE_KOS
union{
    /* +44 */__INT64_TYPE__    st_size;
    /* +44 */__INT32_TYPE__    st_size32;
    /* +44 */__INT64_TYPE__    st_size64;
};
#else /* __USE_KOS */
    /* +44 */__INT64_TYPE__    st_size;
#endif /* !__USE_KOS */
    /* +52 */__INT32_TYPE__    st_blksize;
    /* +56 */__INT64_TYPE__    st_blocks;
#ifdef _STATBUF_ST_TIM
union{/* +64 */struct __timespec32 st_atim; struct{__time32_t st_atime; __syscall_ulong_t st_atimensec;};};
union{/* +72 */struct __timespec32 st_mtim; struct{__time32_t st_mtime; __syscall_ulong_t st_mtimensec;};};
union{/* +80 */struct __timespec32 st_ctim; struct{__time32_t st_ctime; __syscall_ulong_t st_ctimensec;};};
#else /* _STATBUF_ST_TIM */
    /* +64 */__time32_t st_atime; __syscall_ulong_t st_atimensec;
    /* +72 */__time32_t st_mtime; __syscall_ulong_t st_mtimensec;
    /* +80 */__time32_t st_ctime; __syscall_ulong_t st_ctimensec;
#endif /* !_STATBUF_ST_TIM */
#ifdef __USE_KOS
union{
    /* +88 */__UINT64_TYPE__   st_ino;
    /* +88 */__UINT64_TYPE__   st_ino64;
};
#else
    /* +88 */__UINT64_TYPE__   st_ino;
#endif
};
#endif /* __USE_LARGEFILE64 */
#endif /* GLC... */

#if defined(__CRT_DOS) && \
   (defined(__USE_DOS) || defined(__DOS_COMPAT__) || \
    defined(__BUILDING_LIBC))
#if defined(__DOS_COMPAT__) || defined(__BUILDING_LIBC)
/* Define the binary layout of DOS's stat buffers. */
struct __dos_stat {
    __dos_dev_t    st_dev;
    __dos_ino_t    st_ino;
    __uint16_t     st_mode;
    __int16_t      st_nlink;
    __int16_t      st_uid;
    __int16_t      st_gid;
    __int16_t    __st_pad0;
    __dos_dev_t    st_rdev;
#ifdef __USE_FILE_OFFSET64
#ifdef __USE_KOS
union{
    __UINT64_TYPE__ st_size;
    __UINT32_TYPE__ st_size32;
};
#else /* __USE_KOS */
    __INT64_TYPE__ st_size;
#endif /* !__USE_KOS */
#elif defined(__USE_TIME_BITS64)
    __int32_t    __st_pad1;
union{ /* binary compatibility to `stat64i32' and `stat64'. */
#ifdef __USE_KOS
    __UINT32_TYPE__ st_size;
    __UINT32_TYPE__ st_size32;
#else /* __USE_KOS */
    __dos_off_t    st_size;
#endif /* !__USE_KOS */
    __INT64_TYPE__ __st_pad2;
};
#elif defined(__USE_KOS)
union{
    __UINT32_TYPE__ st_size;
    __UINT32_TYPE__ st_size32;
};
#else
    __dos_off_t    st_size;
#endif
#ifdef __USE_KOS
union{__TM_TYPE(time) st_atime; __time32_t st_atime32; };
union{__TM_TYPE(time) st_mtime; __time32_t st_mtime32; };
union{__TM_TYPE(time) st_ctime; __time32_t st_ctime32; };
#else /* __USE_KOS */
    __TM_TYPE(time) st_atime;
    __TM_TYPE(time) st_mtime;
    __TM_TYPE(time) st_ctime;
#endif /* !__USE_KOS */
};

struct __dos_stat32 {
    __dos_dev_t    st_dev;
    __dos_ino_t    st_ino;
    __uint16_t     st_mode;
    __int16_t      st_nlink;
    __int16_t      st_uid;
    __int16_t      st_gid;
    __int16_t    __st_pad0;
    __dos_dev_t    st_rdev;
#ifdef __USE_KOS
union{
    __UINT32_TYPE__ st_size;
    __UINT32_TYPE__ st_size32;
};
union{__time32_t   st_atime; __time32_t st_atime32;};
union{__time32_t   st_mtime; __time32_t st_mtime32;};
union{__time32_t   st_ctime; __time32_t st_ctime32;};
#else /* __USE_KOS */
    __dos_off_t    st_size;
    __time32_t     st_atime;
    __time32_t     st_mtime;
    __time32_t     st_ctime;
#endif /* !__USE_KOS */
};
struct __dos_stat32i64 {
    __dos_dev_t    st_dev;
    __dos_ino_t    st_ino;
    __uint16_t     st_mode;
    __int16_t      st_nlink;
    __int16_t      st_uid;
    __int16_t      st_gid;
    __int16_t    __st_pad0;
    __dos_dev_t    st_rdev;
    __int32_t    __st_pad1;
#ifdef __USE_KOS
union{
    __UINT64_TYPE__ st_size;
    __UINT32_TYPE__ st_size32;
    __UINT64_TYPE__ st_size64;
};
union{__time32_t   st_atime; __time32_t st_atime32;};
union{__time32_t   st_mtime; __time32_t st_mtime32;};
union{__time32_t   st_ctime; __time32_t st_ctime32;};
#else /* __USE_KOS */
    __INT64_TYPE__ st_size;
    __time32_t     st_atime;
    __time32_t     st_mtime;
    __time32_t     st_ctime;
#endif /* !__USE_KOS */
    __int32_t    __st_pad2;
};
struct __dos_stat64i32 {
    __dos_dev_t    st_dev;
    __dos_ino_t    st_ino;
    __uint16_t     st_mode;
    __int16_t      st_nlink;
    __int16_t      st_uid;
    __int16_t      st_gid;
    __int16_t    __st_pad0;
    __dos_dev_t    st_rdev;
    __int32_t    __st_pad1;
#ifdef __USE_KOS
union{
    __UINT32_TYPE__ st_size;
    __UINT32_TYPE__ st_size32;
    __INT64_TYPE__ __st_pad2; /* This is what DOS silently does to match the
                               * binary layout of `stat64i32' with `stat64'. */
};
union{__time64_t   st_atime; __time64_t st_atime32; __time64_t st_atime64; };
union{__time64_t   st_mtime; __time64_t st_mtime32; __time64_t st_mtime64; };
union{__time64_t   st_ctime; __time64_t st_ctime32; __time64_t st_ctime64; };
#else /* __USE_KOS */
union{
    __dos_off_t    st_size;
    __INT64_TYPE__ __st_pad2; /* This is what DOS silently does to match the
                               * binary layout of `stat64i32' with `stat64'. */
};
    __time64_t     st_atime;
    __time64_t     st_mtime;
    __time64_t     st_ctime;
#endif /* !__USE_KOS */
};
struct __dos_stat64 {
    __dos_dev_t    st_dev;
    __dos_ino_t    st_ino;
    __uint16_t     st_mode;
    __int16_t      st_nlink;
    __int16_t      st_uid;
    __int16_t      st_gid;
    __int16_t    __st_pad0;
    __dos_dev_t    st_rdev;
    __int32_t    __st_pad1;
#ifdef __USE_KOS
union{
    __UINT64_TYPE__ st_size;
    __UINT32_TYPE__ st_size32;
    __UINT64_TYPE__ st_size64;
};
union{__time64_t   st_atime; __time64_t st_atime32; __time64_t st_atime64; };
union{__time64_t   st_mtime; __time64_t st_mtime32; __time64_t st_mtime64; };
union{__time64_t   st_ctime; __time64_t st_ctime32; __time64_t st_ctime64; };
#else /* __USE_KOS */
    __INT64_TYPE__ st_size;
    __time64_t     st_atime;
    __time64_t     st_mtime;
    __time64_t     st_ctime;
#endif /* !__USE_KOS */
};
#endif /* __DOS_COMPAT__ || __BUILDING_LIBC */

#endif /* DOS... */

#ifdef __USE_DOS
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
#endif /* __USE_DOS */

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("stat")
#ifdef __USE_LARGEFILE64
#pragma pop_macro("stat64")
#endif /* __USE_LARGEFILE64 */
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

__SYSDECL_END

#endif /* !_BITS_STAT_H */
