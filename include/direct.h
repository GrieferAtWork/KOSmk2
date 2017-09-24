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
#ifndef _DIRECT_H
#define _DIRECT_H 1

#include "__stdinc.h"
#include <features.h>

__DECL_BEGIN

#ifndef _DISKFREE_T_DEFINED
#define _DISKFREE_T_DEFINED 1
struct _diskfree_t {
 unsigned total_clusters;
 unsigned avail_clusters;
 unsigned sectors_per_cluster;
 unsigned bytes_per_sector;
};
#endif /* !_DISKFREE_T_DEFINED */

#ifndef __KERNEL__
__LIBC char *(__LIBCCALL _getcwd)(char *__buf, size_t __size) __ASMNAME("getcwd");
__LIBC char *(__LIBCCALL _getdcwd)(int __drive, char *__buf, size_t __size) __ASMNAME("getdcwd");
#define _getdcwd_nolock  _getdcwd
__LIBC __NONNULL((1)) int (__LIBCCALL _chdir)(char const *__path) __ASMNAME("chdir");
__LIBC __NONNULL((1)) int (__LIBCCALL _mkdir)(char const *__path) __DOSNAME("mkdir");
__LIBC __NONNULL((1)) int (__LIBCCALL _rmdir)(char const *__path) __ASMNAME("rmdir");
__LIBC int (__LIBCCALL _chdrive)(int __drive);
__LIBC int (__LIBCCALL _getdrive)(void);
__LIBC unsigned long (__LIBCCALL _getdrives)(void);
#ifndef _GETDISKFREE_DEFINED
#define _GETDISKFREE_DEFINED 1
__LIBC unsigned (__LIBCCALL _getdiskfree)(unsigned __drive, struct _diskfree_t *__diskfree);
#endif /* !_GETDISKFREE_DEFINED */

#ifdef __USE_DOS
__LIBC wchar_t *(__LIBCCALL _wgetcwd)(wchar_t *__dstbuf, int __sizeinwchars) __ASMNAME2("wgetcwd","_wgetcwd");
__LIBC wchar_t *(__LIBCCALL _wgetdcwd)(int __drive, wchar_t *__dstbuf, int __sizeinwchars) __ASMNAME2("wgetdcwd","_wgetdcwd");
#define _wgetdcwd_nolock    _wgetdcwd
__LIBC int (__LIBCCALL _wchdir)(wchar_t const *__path) __ASMNAME2("wchdir","_wchdir");
__LIBC int (__LIBCCALL _wmkdir)(wchar_t const *__path) __ASMNAME2("wmkdir","_wmkdir");
__LIBC int (__LIBCCALL _wrmdir)(wchar_t const *__path) __ASMNAME2("wrmdir","_wrmdir");
#endif /* __USE_DOS */

/* A small hand full of functions defined in '<direct.h>' */
#ifndef __getcwd_defined
#define __getcwd_defined 1
__LIBC char *(__LIBCCALL getcwd)(char *__buf, size_t __size);
#endif /* !__getcwd_defined */
#ifndef __chdir_defined
#define __chdir_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL chdir)(char const *__path);
#endif /* !__chdir_defined */
#ifndef __mkdir_defined
#define __mkdir_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL mkdir)(char const *__path) __DOSNAME("mkdir");
#endif /* !__mkdir_defined */
#ifndef __rmdir_defined
#define __rmdir_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL rmdir)(char const *__path);
#endif /* !__rmdir_defined */

#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_DIRECT_H */
