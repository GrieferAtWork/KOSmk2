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
#ifndef GUARD_LIBS_LIBC_DIRENT_H
#define GUARD_LIBS_LIBC_DIRENT_H 1

#include "libc.h"
#include "system.h"
#include <hybrid/compiler.h>
#include <dirent.h>
#include <stddef.h>

DECL_BEGIN

struct __dirstream {
 int            d_fd;
 struct dirent *d_buf;   /*< [0..1][owned] Allocated directory entry buffer. */
 size_t         d_bufsz; /*< Allocated buffer size for 'd_buf' */
 /* Inline-allocated directory entry buffer (Used as initial buffer). */
 char           d_inl[offsetof(struct dirent,d_name)+(256+1)*sizeof(char)];
};

INTDEF ssize_t LIBCCALL libc_xreaddir(int fd, struct dirent *buf, size_t bufsize, int mode);
INTDEF DIR *LIBCCALL libc_fdopendir(int fd);
INTDEF DIR *LIBCCALL libc_opendirat(int dfd, char const *name);
INTDEF DIR *LIBCCALL libc_opendir(char const *name);
INTDEF int LIBCCALL libc_closedir(DIR *dirp);
INTDEF struct dirent *LIBCCALL libc_readdir(DIR *dirp);
INTDEF void LIBCCALL libc_rewinddir(DIR *dirp);
INTDEF int LIBCCALL libc_readdir_r(DIR *__restrict dirp, struct dirent *__restrict entry, struct dirent **__restrict result);
INTDEF void LIBCCALL libc_seekdir(DIR *dirp, long int pos);
INTDEF long int LIBCCALL libc_telldir(DIR *dirp);
INTDEF int LIBCCALL libc_dirfd(DIR *dirp);
INTDEF int LIBCCALL libc_scandir(char const *__restrict dir, struct dirent ***__restrict namelist, int (*selector)(struct dirent const *), int (*cmp)(struct dirent const **, struct dirent const **));
INTDEF int LIBCCALL libc_alphasort(struct dirent const **e1, struct dirent const **e2);
INTDEF int LIBCCALL libc_scandirat(int dfd, char const *__restrict dir, struct dirent ***__restrict namelist, int (*selector)(struct dirent const *), int (*cmp)(struct dirent const **, struct dirent const **));
INTDEF ssize_t LIBCCALL libc_getdirentries(int fd, char *__restrict buf, size_t nbytes, off_t *__restrict basep);
INTDEF int LIBCCALL libc_versionsort(struct dirent const **e1, struct dirent const **e2);

DECL_END

#endif /* !GUARD_LIBS_LIBC_DIRENT_H */
