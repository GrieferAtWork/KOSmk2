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
#ifndef _MNTENT_H
#define _MNTENT_H 1

#include <features.h>
#include <paths.h>

#ifndef __CRT_GLC
#error "<mntent.h> is not supported by the linked libc"
#endif /* !__CRT_GLC */

__SYSDECL_BEGIN

#define MNTTAB          _PATH_MNTTAB
#define MOUNTED         _PATH_MOUNTED
#define MNTTYPE_IGNORE  "ignore"   /*< Ignore this entry. */
#define MNTTYPE_NFS     "nfs"      /*< Network file system. */
#define MNTTYPE_SWAP    "swap"     /*< Swap device. */
#define MNTOPT_DEFAULTS "defaults" /*< Use all default options. */
#define MNTOPT_RO       "ro"       /*< Read only. */
#define MNTOPT_RW       "rw"       /*< Read/write. */
#define MNTOPT_SUID     "suid"     /*< Set uid allowed. */
#define MNTOPT_NOSUID   "nosuid"   /*< No set uid allowed. */
#define MNTOPT_NOAUTO   "noauto"   /*< Do not auto mount. */

struct mntent {
 char *mnt_fsname; /*< Device or server for filesystem. */
 char *mnt_dir;    /*< Directory mounted on. */
 char *mnt_type;   /*< Type of filesystem: ufs, nfs, etc. */
 char *mnt_opts;   /*< Comma-separated options for fs. */
 int   mnt_freq;   /*< Dump frequency (in days). */
 int   mnt_passno; /*< Pass number for `fsck'. */
};

#ifndef __KERNEL__
#ifndef __std_FILE_defined
#define __std_FILE_defined 1
__NAMESPACE_STD_BEGIN
typedef __FILE FILE;
__NAMESPACE_STD_END
#endif /* !__std_FILE_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __FILE_defined
#define __FILE_defined 1
__NAMESPACE_STD_USING(FILE)
#endif /* !__FILE_defined */
#endif /* !__CXX_SYSTEM_HEADER */

__LIBC FILE *(__LIBCCALL setmntent)(char const *__file, char const *__mode) __UFS_FUNC(setmntent);
__LIBC struct mntent *(__LIBCCALL getmntent)(FILE *__stream);
__LIBC int (__LIBCCALL addmntent)(FILE *__restrict __stream, struct mntent const *__restrict __mnt);
__LIBC int (__LIBCCALL endmntent)(FILE *__stream);
__LIBC char *(__LIBCCALL hasmntopt)(struct mntent const *__mnt, char const *__opt);
#ifdef __USE_MISC
__LIBC struct mntent *(__LIBCCALL getmntent_r)(FILE *__restrict __stream, struct mntent *__restrict __result, char *__restrict __buffer, int __bufsize);
#endif /* __USE_MISC */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_MNTENT_H */
