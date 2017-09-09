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
#ifndef _DIRENT_H
#define _DIRENT_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>
#include <bits/dirent.h>

__DECL_BEGIN

#ifdef __USE_XOPEN
#ifndef __ino_t_defined
#define __ino_t_defined 1
typedef __FS_TYPE(ino) ino_t;
#endif /* !__ino_t_defined */
#ifdef __USE_LARGEFILE64
#ifndef __ino64_t_defined
#define __ino64_t_defined 1
typedef __ino64_t ino64_t;
#endif /* !__ino64_t_defined */
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN */

#ifdef __USE_MISC
#ifndef d_fileno
#   define d_ino d_fileno
#endif /* !d_fileno */
#endif


#ifdef _DIRENT_HAVE_D_NAMLEN
#   define _D_EXACT_NAMLEN(d) ((d)->d_namlen)
#   define _D_ALLOC_NAMLEN(d) (_D_EXACT_NAMLEN(d)+1)
#else
#   define _D_EXACT_NAMLEN(d) strlen((d)->d_name)
#ifdef _DIRENT_HAVE_D_RECLEN
#   define _D_ALLOC_NAMLEN(d) (((char *)(d)+(d)->d_reclen)-&(d)->d_name[0])
#else
#   define _D_ALLOC_NAMLEN(d) (sizeof((d)->d_name) > 1 ? sizeof((d)->d_name) : _D_EXACT_NAMLEN(d)+1)
# endif
#endif


#ifdef __USE_MISC
enum {
#undef  DT_UNKNOWN
        DT_UNKNOWN = 0,
#define DT_UNKNOWN   0
#undef  DT_FIFO
        DT_FIFO    = 1,
#define DT_FIFO      1
#undef  DT_CHR
        DT_CHR     = 2,
#define DT_CHR       2
#undef  DT_DIR
        DT_DIR     = 4,
#define DT_DIR       4
#undef  DT_BLK
        DT_BLK     = 6,
#define DT_BLK       6
#undef  DT_REG
        DT_REG     = 8,
#define DT_REG       8
#undef  DT_LNK
        DT_LNK     = 10,
#define DT_LNK       10
#undef  DT_SOCK
        DT_SOCK    = 12,
#define DT_SOCK      12
#undef  DT_WHT
        DT_WHT     = 14
#define DT_WHT       14
};

/* Convert between stat structure types and directory types. */
#define IFTODT(mode)    (((mode) & 0170000) >> 12)
#define DTTOIF(dirtype) ((dirtype) << 12)
#endif /* __USE_MISC */

#ifndef __KERNEL__
typedef struct __dirstream DIR;

__LIBC __WUNUSED __NONNULL((1)) DIR *(__LIBCCALL opendir)(char const *__name);
#if defined(__USE_KOS) && defined(__USE_ATFILE)
__LIBC __WUNUSED __NONNULL((2)) DIR *(__LIBCCALL opendirat)(int __dfd, char const *__name);
#endif /* __USE_KOS && __USE_ATFILE */
__LIBC __NONNULL((1)) int (__LIBCCALL closedir)(DIR *__dirp);
__LIBC __NONNULL((1)) struct dirent *(__LIBCCALL readdir)(DIR *__dirp) __FS_FUNC(readdir);
__LIBC __NONNULL((1)) void (__LIBCCALL rewinddir)(DIR *__dirp);
#ifdef __USE_XOPEN2K8
__LIBC __WUNUSED DIR *(__LIBCCALL fdopendir)(int __fd);
#endif /* __USE_XOPEN2K8 */
#ifdef __USE_LARGEFILE64
__LIBC __NONNULL((1)) struct dirent64 *(__LIBCCALL readdir64)(DIR *__dirp);
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_POSIX
__LIBC __NONNULL((1,2,3)) int (__LIBCCALL readdir_r)(DIR *__restrict __dirp, struct dirent *__restrict __entry, struct dirent **__restrict __result) __FS_FUNC_R(readdir);
#ifdef __USE_LARGEFILE64
/* NOTE: This ~reentrant~ version of readdir() is strongly discouraged from being used in KOS, as the
 *       kernel does not impose a limit on the length of a single directory entry name (s.a. 'xreaddir')
 * >> Instead, simply use 'readdir()'/'readdir64()', which will automatically (re-)allocate an internal,
 *    per-directory buffer of sufficient size to house any directory entry (s.a.: 'READDIR_DEFAULT') */
__LIBC __NONNULL((1,2,3)) int (__LIBCCALL readdir64_r)(DIR *__restrict __dirp, struct dirent64 *__restrict __entry, struct dirent64 **__restrict __result);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_POSIX */
#if defined(__USE_MISC) || defined(__USE_XOPEN)
__LIBC __NONNULL((1)) void (__LIBCCALL seekdir)(DIR *__dirp, long int __pos);
__LIBC __NONNULL((1)) long int (__LIBCCALL telldir)(DIR *__dirp);
#endif /* __USE_MISC || __USE_XOPEN */

#ifdef __USE_XOPEN2K8
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL dirfd)(DIR *__dirp);
#if defined(__USE_MISC) && !defined(MAXNAMLEN)
#define MAXNAMLEN    255 /*< == 'NAME_MAX' from <linux/limits.h> */
#endif

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

__LIBC __NONNULL((1,2)) int (__LIBCCALL scandir)
      (char const *__restrict __dir, struct dirent ***__restrict __namelist,
       int (*__selector) (struct dirent const *),
       int (*__cmp) (struct dirent const **, struct dirent const **)) __FS_FUNC(scandir);

__LIBC __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL alphasort)
      (struct dirent const **__e1, struct dirent const **__e2) __FS_FUNC(alphasort);
#ifdef __USE_LARGEFILE64
__LIBC __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL alphasort64)
      (struct dirent64 const **__e1, struct dirent64 const **__e2);
#endif /* __USE_LARGEFILE64 */

#ifdef __USE_GNU
__LIBC __NONNULL((2,3)) int (__LIBCCALL scandirat)
      (int __dfd, char const *__restrict __dir, struct dirent ***__restrict __namelist,
       int (*__selector) (struct dirent const *),
       int (*__cmp) (struct dirent const **, struct dirent const **)) __FS_FUNC(scandirat);
#ifdef __USE_LARGEFILE64
__LIBC __NONNULL((1,2)) int (__LIBCCALL scandir64)
      (char const *__restrict __dir, struct dirent64 ***__restrict __namelist,
       int (*__selector) (const struct dirent64 *),
       int (*__cmp) (const struct dirent64 **, const struct dirent64 **));
__LIBC __NONNULL((2,3)) int (__LIBCCALL scandirat64)
      (int __dfd, char const *__restrict __dir, struct dirent64 ***__restrict __namelist,
       int (*__selector) (const struct dirent64 *),
       int (*__cmp) (const struct dirent64 **, const struct dirent64 **));
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_GNU */
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_MISC
__LIBC __NONNULL((2,4)) __ssize_t (__LIBCCALL getdirentries)
      (int __fd, char *__restrict __buf, size_t __nbytes, __FS_TYPE(off) *__restrict __basep)
       __FS_FUNC(getdirentries);
#ifdef __USE_LARGEFILE64
__LIBC __NONNULL((2,4)) __ssize_t (__LIBCCALL getdirentries64)
      (int __fd, char *__restrict __buf, size_t __nbytes, __off64_t *__restrict __basep);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_MISC */

#ifdef __USE_GNU
__LIBC __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL versionsort)
      (struct dirent const **__e1, struct dirent const **__e2) __FS_FUNC(versionsort);
#ifdef __USE_LARGEFILE64
__LIBC __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL versionsort64)
      (struct dirent64 const **__e1, struct dirent64 const **__e2);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_GNU */

#ifdef __USE_KOS
/* NOTE: Keep these mode constants in sync with 'FILE_READDIR_*' from "/src/kernel/include/fs/inode.h" */
#define READDIR_DEFAULT  0 /*< Yield to next entry when 'buf' was of sufficient size. */
#define READDIR_CONTINUE 1 /*< Always yield to next entry. */
#define READDIR_PEEK     2 /*< Never yield to next entry. */

/* The KOS-specific system call for reading a single directory entry
 * from a file descriptor referring to an open directory stream.
 * @param: MODE: One of 'READDIR_*'
 * @return: * : The actually required buffer size for the directory entry (in bytes)
 *              NOTE: When 'READDIR_DEFAULT' was passed for 'MODE', the directory
 *                    stream will only be advanced when this value is >= 'BUFSIZE'
 * @return: 0 : The end of the directory has been reached.
 * @return: -1: Failed to read a directory entry for some reason (s.a.: 'errno') */
__LIBC __NONNULL((2)) __ssize_t (__LIBCCALL xreaddir)(int __fd, struct dirent *__buf, size_t __bufsize, int __mode);

#endif
#endif /* !__KERNEL__ */




__DECL_END


#endif /* !_DIRENT_H */
