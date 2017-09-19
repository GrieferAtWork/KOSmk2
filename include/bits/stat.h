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

/* Fixed stat features */
#define _STATBUF_ST_BLKSIZE
#define _STATBUF_ST_RDEV


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
    __STAT_TIMESPEC32(a);
    __STAT_TIMESPEC32(m);
    __STAT_TIMESPEC32(c);
    __STAT_TIMESPEC64(a);
    __STAT_TIMESPEC64(m);
    __STAT_TIMESPEC64(c);
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
    __STAT_TIMESPEC32(a);
    __STAT_TIMESPEC32(m);
    __STAT_TIMESPEC32(c);
    __STAT_TIMESPEC64(a);
    __STAT_TIMESPEC64(m);
    __STAT_TIMESPEC64(c);
};
#endif

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

__DECL_END

#endif /* !_BITS_STAT_H */
