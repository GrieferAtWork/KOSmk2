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

__SYSDECL_BEGIN

#ifdef __USE_XOPEN
#ifndef __ino_t_defined
#define __ino_t_defined 1
typedef __typedef_ino_t ino_t;
#endif /* !__ino_t_defined */
#ifdef __USE_LARGEFILE64
#ifndef __ino64_t_defined
#define __ino64_t_defined 1
typedef __ino64_t ino64_t;
#endif /* !__ino64_t_defined */
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

__SYSDECL_END

#ifdef __CRT_GLC
#include <bits/dirent.h>
__SYSDECL_BEGIN

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
__LIBC __PORT_KOSONLY_ALT(opendir) __WUNUSED __NONNULL((2)) DIR *(__LIBCCALL opendirat)(int __dfd, char const *__name);
#endif /* __USE_KOS && __USE_ATFILE */
__LIBC __NONNULL((1)) int (__LIBCCALL closedir)(DIR *__dirp);
__REDIRECT_FS_FUNC(__LIBC,__NONNULL((1)),struct dirent *,__LIBCCALL,readdir,(DIR *__dirp),readdir,(__dirp))
__LIBC __PORT_NODOS __NONNULL((1)) void (__LIBCCALL rewinddir)(DIR *__dirp);
#ifdef __USE_XOPEN2K8
__LIBC __PORT_NODOS __WUNUSED DIR *(__LIBCCALL fdopendir)(int __fd);
#endif /* __USE_XOPEN2K8 */
#ifdef __USE_LARGEFILE64
__LIBC __NONNULL((1)) struct dirent64 *(__LIBCCALL readdir64)(DIR *__dirp);
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_POSIX
__REDIRECT_FS_FUNC_R(__LIBC,__PORT_NODOS __NONNULL((1,2,3)),int,__LIBCCALL,readdir_r,
                    (DIR *__restrict __dirp, struct dirent *__restrict __entry,
                     struct dirent **__restrict __result),readdir,
                    (__dirp,__entry,__result))
#ifdef __USE_LARGEFILE64
/* NOTE: This ~reentrant~ version of readdir() is strongly discouraged from being used in KOS, as the
 *       kernel does not impose a limit on the length of a single directory entry name (s.a. 'xreaddir')
 * >> Instead, simply use 'readdir()'/'readdir64()', which will automatically (re-)allocate an internal,
 *    per-directory buffer of sufficient size to house any directory entry (s.a.: 'READDIR_DEFAULT') */
__LIBC __PORT_NODOS __NONNULL((1,2,3)) int (__LIBCCALL readdir64_r)(DIR *__restrict __dirp, struct dirent64 *__restrict __entry, struct dirent64 **__restrict __result);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_POSIX */
#if defined(__USE_MISC) || defined(__USE_XOPEN)
__LIBC __PORT_NODOS __NONNULL((1)) void (__LIBCCALL seekdir)(DIR *__dirp, long int __pos);
__LIBC __PORT_NODOS __NONNULL((1)) long int (__LIBCCALL telldir)(DIR *__dirp);
#endif /* __USE_MISC || __USE_XOPEN */

#ifdef __USE_XOPEN2K8
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL dirfd)(DIR *__dirp);
#if defined(__USE_MISC) && !defined(MAXNAMLEN)
#define MAXNAMLEN    255 /*< == 'NAME_MAX' from <linux/limits.h> */
#endif

__REDIRECT_FS_FUNC(__LIBC,__PORT_NODOS __NONNULL((1,2)),int,__LIBCCALL,scandir,
                  (char const *__restrict __dir, struct dirent ***__restrict __namelist,
                   int (*__selector) (struct dirent const *),
                   int (*__cmp) (struct dirent const **, struct dirent const **)),
                   scandir,(__dir,__namelist,__selector,__cmp))
__REDIRECT_FS_FUNC(__LIBC,__PORT_NODOS __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,alphasort,
                  (struct dirent const **__e1, struct dirent const **__e2),
                   alphasort,(__e1,__e2))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL alphasort64)
      (struct dirent64 const **__e1, struct dirent64 const **__e2);
#endif /* __USE_LARGEFILE64 */

#ifdef __USE_GNU
__REDIRECT_FS_FUNC(__LIBC,__PORT_NODOS __NONNULL((2,3)),int,__LIBCCALL,scandirat,
                  (int __dfd, char const *__restrict __dir, struct dirent ***__restrict __namelist,
                   int (*__selector) (struct dirent const *),
                   int (*__cmp) (struct dirent const **, struct dirent const **)),
                   scandirat,(__dfd,__dir,__namelist,__selector,__cmp))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL scandir64)
      (char const *__restrict __dir, struct dirent64 ***__restrict __namelist,
       int (*__selector) (const struct dirent64 *),
       int (*__cmp) (const struct dirent64 **, const struct dirent64 **));
__LIBC __PORT_NODOS __NONNULL((2,3)) int (__LIBCCALL scandirat64)
      (int __dfd, char const *__restrict __dir, struct dirent64 ***__restrict __namelist,
       int (*__selector) (const struct dirent64 *),
       int (*__cmp) (const struct dirent64 **, const struct dirent64 **));
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_GNU */
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_MISC
__REDIRECT_FS_FUNC(__LIBC,__PORT_NODOS __NONNULL((2,4)),__ssize_t,__LIBCCALL,getdirentries,
                  (int __fd, char *__restrict __buf, size_t __nbytes, __FS_TYPE(off) *__restrict __basep),
                   getdirentries,(__fd,__buf,__nbytes,__basep))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS __NONNULL((2,4)) __ssize_t (__LIBCCALL getdirentries64)
      (int __fd, char *__restrict __buf, size_t __nbytes, __off64_t *__restrict __basep);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_MISC */

#ifdef __USE_GNU
__REDIRECT_FS_FUNC(__LIBC,__PORT_NODOS __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,versionsort,
                  (struct dirent const **__e1, struct dirent const **__e2),versionsort,(__e1,__e2))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL versionsort64)
      (struct dirent64 const **__e1, struct dirent64 const **__e2);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_GNU */

#if defined(__USE_KOS) && defined(__CRT_KOS)
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
__LIBC __PORT_KOSONLY_ALT(readdir) __NONNULL((2))
__ssize_t (__LIBCCALL xreaddir)(int __fd, struct dirent *__buf, size_t __bufsize, int __mode);
#endif
#endif /* !__KERNEL__ */

__SYSDECL_END
#elif defined(__CRT_DOS)

#include <errno.h>
#include <hybrid/malloc.h>
#include <hybrid/string.h>

__SYSDECL_BEGIN

#ifdef __USE_LARGEFILE64
#ifdef __INTELLISENSE__
struct dirent64 { __dos_ino_t d_ino; char d_name[260]; };
#else
#define dirent64 dirent
#endif
#endif /* __USE_LARGEFILE64 */

struct dirent {
 __dos_ino_t       d_ino; /* Mandatory */
#ifndef __INTELLISENSE__
 __UINT16_TYPE__ __d_pad;
 /* Members below are arranged for binary compatibility with 'struct _finddata32_t' */
 __UINT32_TYPE__ __d_attrib;
 __UINT32_TYPE__ __d_time_create;
 __UINT32_TYPE__ __d_time_access;
 __UINT32_TYPE__ __d_time_write;
 __UINT32_TYPE__ __d_size;
#endif /* !__INTELLISENSE__ */
 char              d_name[260];
};
#define d_fileno d_ino /*< Backwards compatibility. */

#undef  _DIRENT_HAVE_D_RECLEN
#undef  _DIRENT_HAVE_D_NAMLEN
#undef  _DIRENT_HAVE_D_TYPE
#define _DIRENT_MATCHES_DIRENT64 1

typedef struct __dirstream DIR;
struct __dirstream {
 __INTPTR_TYPE__ __d_hnd;
 int             __d_isfirst;
 struct dirent   __d_ent;
};

__REDIRECT(__LIBC,__NONNULL((1,2)),__INTPTR_TYPE__,__LIBCCALL,__dos_findfirst,(char const *__file, void *__buf),_findfirst32,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,__dos_findnext,(__INTPTR_TYPE__ __findfd, void *__buf),_findnext32,(__findfd,__buf))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_findclose,(__INTPTR_TYPE__ __findfd),_findclose,(__findfd))
__LOCAL __WUNUSED __NONNULL((1)) DIR *(__LIBCCALL opendir)(char const *__name);
__LOCAL __NONNULL((1)) int (__LIBCCALL closedir)(DIR *__dirp);
__LOCAL __NONNULL((1)) struct dirent *(__LIBCCALL readdir)(DIR *__dirp);
#ifdef __INTELLISENSE__
#ifdef __USE_LARGEFILE64
__LOCAL __NONNULL((1)) struct dirent64 *(__LIBCCALL readdir64)(DIR *__dirp);
#endif /* __USE_LARGEFILE64 */
#else
__LOCAL DIR *(__LIBCCALL opendir)(char const *__name) {
 DIR *__result; size_t __namelen = __hybrid_strlen(__name);
 char *__query = (char *)__hybrid_malloc((__namelen+3)*sizeof(char));
 if __unlikely(!__query) return 0;
 __result = (DIR *)__hybrid_malloc(sizeof(DIR));
 if __unlikely(!__result) goto __end;
 __hybrid_memcpy(__query,__name,__namelen*sizeof(char));
 __query[__namelen]   = '\\';
 __query[__namelen+1] = '*';
 __query[__namelen+2] = '\0';
 __result->__d_isfirst = 1;
 __result->__d_ent.d_ino = 0;
 __result->__d_hnd = __dos_findfirst(__query,&__result->__d_ent.__d_attrib);
 if __unlikely(__result->__d_hnd == -1) { __hybrid_free(__result); __result = 0; }
__end:
 __hybrid_free(__query);
 return __result;
}
__LOCAL int (__LIBCCALL closedir)(DIR *__dirp) {
 if __unlikely(!__dirp) { __set_errno(EINVAL); return -1; }
 __dos_findclose(__dirp->__d_hnd);
 __hybrid_free(__dirp);
 return 0;
}
__LOCAL struct dirent *(__LIBCCALL readdir)(DIR *__dirp) {
 if __unlikely(!__dirp) { __set_errno(EINVAL); return 0; }
 if (!__dirp->__d_isfirst) {
  if (__dos_findnext(__dirp->__d_hnd,&__dirp->__d_ent.__d_attrib))
      return 0;
  ++__dirp->__d_ent.d_ino;
 }
 __dirp->__d_isfirst = 0;
 return &__dirp->__d_ent;
}

#ifdef __USE_LARGEFILE64
#define readdir64(dir)                readdir(dir)
#endif /* __USE_LARGEFILE64 */
#endif /* !__INTELLISENSE__ */

__SYSDECL_END
#else
#error "<dirent.h> is not supported by the linked libc"
#endif /* __CRT_GLC */

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
#ifndef d_fileno
#   define d_ino d_fileno
#endif /* !d_fileno */
#endif

#endif /* !_DIRENT_H */
