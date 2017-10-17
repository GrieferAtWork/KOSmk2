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
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef _DISKFREE_T_DEFINED
#define _DISKFREE_T_DEFINED 1
struct _diskfree_t {
 __UINT32_TYPE__ total_clusters;
 __UINT32_TYPE__ avail_clusters;
 __UINT32_TYPE__ sectors_per_cluster;
 __UINT32_TYPE__ bytes_per_sector;
};
#endif /* !_DISKFREE_T_DEFINED */

#ifndef __KERNEL__

__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,__WUNUSED_SUGGESTED,char *,__LIBCCALL,_getcwd,(char *__buf, size_t __size),getcwd,(__buf,__size))
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,__NONNULL((1)),int,__LIBCCALL,_chdir,(char const *__path),chdir,(__path));
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,__NONNULL((1)),int,__LIBCCALL,_rmdir,(char const *__path),rmdir,(__path));

#ifdef __CRT_DOS
#define _getdcwd_nolock _getdcwd
__LIBC __PORT_NODOS_ALT(getcwd) __WUNUSED_SUGGESTED
char *(__LIBCCALL _getdcwd)(int __drive, char *__buf, size_t __size);
__LIBC __PORT_NODOS int (__LIBCCALL _chdrive)(int __drive);
__LIBC __WUNUSED __PORT_NODOS int (__LIBCCALL _getdrive)(void);
__LIBC __WUNUSED __PORT_NODOS unsigned long (__LIBCCALL _getdrives)(void);
#ifndef _GETDISKFREE_DEFINED
#define _GETDISKFREE_DEFINED 1
#ifdef __USE_KOS
__LIBC __PORT_NODOS unsigned int (__LIBCCALL _getdiskfree)(int __drive, struct _diskfree_t *__restrict __diskfree);
#else /* __USE_KOS */
__LIBC __PORT_NODOS unsigned int (__LIBCCALL _getdiskfree)(unsigned int __drive, struct _diskfree_t *__diskfree);
#endif /* !__USE_KOS */
#endif /* !_GETDISKFREE_DEFINED */
#endif /* __CRT_DOS */

/* A small hand full of functions defined in '<direct.h>' */
#ifndef __getcwd_defined
#define __getcwd_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__WUNUSED_SUGGESTED,char *,__LIBCCALL,getcwd,(char *__buf, size_t __size),getcwd,(__buf,__size))
#endif /* !__getcwd_defined */
#ifndef __chdir_defined
#define __chdir_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,chdir,(char const *__path),chdir,(__path));
#endif /* !__chdir_defined */
#ifndef __rmdir_defined
#define __rmdir_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,rmdir,(char const *__path),rmdir,(__path))
#endif /* !__rmdir_defined */

#if defined(__CRT_DOS) && defined(__USE_DOSFS)
__LIBC __NONNULL((1)) int (__LIBCCALL _mkdir)(char const *__path);
#else /* __CRT_DOS && __USE_DOSFS */
__REDIRECT_UFS_(__LIBC,__NONNULL((1)),int,__LIBCCALL,__kos_mkdir,(char const *__path, int __mode),mkdir,(__path,__mode))
__LOCAL __NONNULL((1)) int (__LIBCCALL _mkdir)(char const *__path) { return __kos_mkdir(__path,0755); }
#endif /* !__CRT_DOS || !__USE_DOSFS */

#ifndef __mkdir_defined
#define __mkdir_defined 1
#if defined(__CRT_DOS) && defined(__USE_DOSFS)
__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,mkdir,(char const *__path),_mkdir,(__path))
#else /* __USE_DOSFS */
__LOCAL __NONNULL((1)) int (__LIBCCALL mkdir)(char const *__path) { return __kos_mkdir(__path,0755); }
#endif /* !__USE_DOSFS */
#endif /* !__mkdir_defined */

#ifdef __USE_DOS
#ifdef __CRT_DOS
#ifndef _WDIRECT_DEFINED
#define _WDIRECT_DEFINED 1
__LIBC __WUNUSED_SUGGESTED __PORT_NODOS __NONNULL((1)) wchar_t *(__LIBCCALL _wgetcwd)(wchar_t *__dstbuf, int __dstlen);
__LIBC __WUNUSED_SUGGESTED __PORT_NODOS __NONNULL((2)) wchar_t *(__LIBCCALL _wgetdcwd)(int __drive, wchar_t *__dstbuf, int __dstlen);
#define _wgetdcwd_nolock    _wgetdcwd
__REDIRECT_WFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,_wchdir,(wchar_t const *__path),_wchdir,(__path));
__REDIRECT_WFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,_wrmdir,(wchar_t const *__path),_wrmdir,(__path));
#ifdef __USE_DOSFS
__REDIRECT_WFS(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,_wmkdir,(wchar_t const *__path),_wmkdir,(__path));
#else /* __USE_DOSFS */
__REDIRECT_WFS_(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,__libc_wmkdir,(wchar_t const *__path, int __mode),_wmkdir,(__path,__mode));
__LOCAL __PORT_NODOS __NONNULL((1)) int (__LIBCCALL _wmkdir)(wchar_t const *__path) { return __libc_wmkdir(__path,0755); }
#endif /* !__USE_DOSFS */
#endif /* !_WDIRECT_DEFINED */
#endif /* __CRT_DOS */
#endif /* __USE_DOS */


#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_DIRECT_H */
