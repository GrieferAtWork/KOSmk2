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
#ifndef GUARD_LIBS_LIBC_FCNTL_H
#define GUARD_LIBS_LIBC_FCNTL_H 1

#include "libc.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <uchar.h>
#include <errno.h>

DECL_BEGIN

struct virtinfo;

#ifndef __KERNEL__
INTDEF ssize_t ATTR_CDECL libc_ioctl(int fd, unsigned long int request, ...);
INTDEF ssize_t ATTR_CDECL libc_fcntl(int fd, int cmd, ...);
INTDEF int ATTR_CDECL libc_openat(int fd, char const *file, int oflag, ...);
INTDEF int ATTR_CDECL libc_open(char const *file, int oflag, ...);
INTDEF int LIBCCALL libc_creat(char const *file, mode_t mode);
INTDEF char *LIBCCALL libc_getcwd(char *buf, size_t bufsize);
INTDEF char *LIBCCALL libc_get_current_dir_name(void);
INTDEF char *LIBCCALL libc_getwd(char *buf);
INTDEF int LIBCCALL libc_posix_fadvise(int fd, off_t offset, off_t len, int advise);
INTDEF int LIBCCALL libc_posix_fallocate(int fd, off_t offset, off_t len);

INTDEF ssize_t LIBCCALL libc_xfdname2(int fd, int type, char *buf, size_t bufsize);
INTDEF char *LIBCCALL libc_xfdname(int fd, int type, char *buf, size_t bufsize);
INTERN ssize_t LIBCCALL libc_xvirtinfo2(VIRT void *addr, USER struct virtinfo *buf, size_t bufsize, u32 flags);
INTERN struct virtinfo *LIBCCALL libc_xvirtinfo(VIRT void *addr, USER struct virtinfo *buf, size_t bufsize, u32 flags);


#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF oflag_t LIBCCALL libc_dos_getoflags(oflag_t unix_flags);

INTDEF char *LIBCCALL libc_getdcwd(int drive, char *buf, size_t size);
INTDEF char16_t *LIBCCALL libc_16wgetcwd(char16_t *dstbuf, int elemcount);
INTDEF char32_t *LIBCCALL libc_32wgetcwd(char32_t *dstbuf, int elemcount);
INTDEF char16_t *LIBCCALL libc_16wgetdcwd(int drive, char16_t *dstbuf, int elemcount);
INTDEF char32_t *LIBCCALL libc_32wgetdcwd(int drive, char32_t *dstbuf, int elemcount);
INTDEF int LIBCCALL libc_dos_creat(char const *file, mode_t mode);
INTDEF int ATTR_CDECL libc_dos_open(char const *file, int oflag, ...);
INTDEF int ATTR_CDECL libc_dos_sopen(char const *file, int oflag, int sflag, ...);
INTDEF __errno_t LIBCCALL libc_dos_sopen_s(int *fd, char const *file, int oflag, int sflag, int pmode);

INTDEF int LIBCCALL libc_16wcreat(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_32wcreat(char32_t const *file, mode_t mode);
INTDEF int ATTR_CDECL libc_16wopen(char16_t const *file, int oflag, ...);
INTDEF int ATTR_CDECL libc_32wopen(char32_t const *file, int oflag, ...);
INTDEF int ATTR_CDECL libc_16wsopen(char16_t const *file, int oflag, int sflag, ...);
INTDEF int ATTR_CDECL libc_32wsopen(char32_t const *file, int oflag, int sflag, ...);
INTDEF int LIBCCALL libc_16wsopen_s(int *fd, char16_t const *file, int oflag, int sflag, mode_t cmode);
INTDEF int LIBCCALL libc_32wsopen_s(int *fd, char32_t const *file, int oflag, int sflag, mode_t cmode);
INTDEF int LIBCCALL libc_dos_16wcreat(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_32wcreat(char32_t const *file, mode_t mode);
INTDEF int ATTR_CDECL libc_dos_16wopen(char16_t const *file, int oflag, ...);
INTDEF int ATTR_CDECL libc_dos_32wopen(char32_t const *file, int oflag, ...);
INTDEF int ATTR_CDECL libc_dos_16wsopen(char16_t const *file, int oflag, int sflag, ...);
INTDEF int ATTR_CDECL libc_dos_32wsopen(char32_t const *file, int oflag, int sflag, ...);
INTDEF int LIBCCALL libc_dos_16wsopen_s(int *fd, char16_t const *file, int oflag, int sflag, mode_t cmode);
INTDEF int LIBCCALL libc_dos_32wsopen_s(int *fd, char32_t const *file, int oflag, int sflag, mode_t cmode);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_FCNTL_H */
