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
#ifndef _IO_H
#define _IO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __size_t size_t;
#endif /* !__size_t_defined */

#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t intptr_t;
#endif /* !__intptr_t_defined */

#ifndef _FSIZE_T_DEFINED
#define _FSIZE_T_DEFINED
typedef __UINT32_TYPE__ _fsize_t;
#endif /* _FSIZE_T_DEFINED */

struct _finddata32_t;
struct __finddata64_t; /* I guess something else already using the more obvious choice... */
struct _finddata32i64_t;
struct _finddata64i32_t;

#ifndef _A_NORMAL
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20
#endif /* !_A_NORMAL */

#ifndef __KERNEL__

/* Functions with the correct names, also present in other headers. */
#ifndef __std_remove_defined
#define __std_remove_defined 1
__NAMESPACE_STD_BEGIN
__REDIRECT_UFS(__LIBC,__NONNULL((1)),int,__LIBCCALL,remove,(char const *__file),remove,(__file))
__NAMESPACE_STD_END
#endif /* !__std_remove_defined */
#ifndef __remove_defined
#define __remove_defined 1
__NAMESPACE_STD_USING(remove)
#endif /* !__remove_defined */
#ifndef __std_rename_defined
#define __std_rename_defined 1
__NAMESPACE_STD_BEGIN
__REDIRECT_UFS(__LIBC,__NONNULL((1)),int,__LIBCCALL,rename,(char const *__old, char const *__new),rename,(__old,__new))
__NAMESPACE_STD_END
#endif /* !__std_rename_defined */
#ifndef __rename_defined
#define __rename_defined 1
__NAMESPACE_STD_USING(rename)
#endif /* !__rename_defined */
#ifndef __unlink_defined
#define __unlink_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,unlink,(char const *__name),unlink,(__name))
#endif /* !__unlink_defined */
#ifndef __open_defined
#define __open_defined 1 /* TODO: Use redirection. */
__LIBC __NONNULL((1)) int (__ATTR_CDECL open)(char const *__file, int __oflag, ...) __UFS_FUNCn_OLDPEA(open);
#endif /* !__open_defined */
#ifndef __creat_defined
#define __creat_defined 1
__REDIRECT_UFS_FUNCn_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,creat,(char const *__file, mode_t __mode),creat,(__file,__mode))
#endif /* !__creat_defined */
#ifndef __access_defined
#define __access_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,access,(char const *__name, int __type),access,(__name,__type))
#endif /* !__access_defined */
#ifndef __chmod_defined
#define __chmod_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,chmod,(char const *__file, int __mode),chmod,(__file,__mode))
#endif /* !__chmod_defined */
#ifndef __close_defined
#define __close_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,,int,__LIBCCALL,close,(int __fd),close,(__fd))
#endif /* !__close_defined */
#ifndef __dup_defined
#define __dup_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,__WUNUSED,int,__LIBCCALL,dup,(int __fd),dup,(__fd))
#endif /* !__dup_defined */
#ifndef __dup2_defined
#define __dup2_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,,int,__LIBCCALL,dup2,(int __ofd, int __nfd),dup2,(__ofd,__nfd))
#endif /* !__dup2_defined */
#ifndef __isatty_defined
#define __isatty_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,__WUNUSED,int,__LIBCCALL,isatty,(int __fd),isatty,(__fd))
#endif /* !__isatty_defined */
#ifndef __lseek_defined
#define __lseek_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,,__LONG32_TYPE__,__LIBCCALL,lseek,(int __fd, __LONG32_TYPE__ __offset, int __whence),lseek,(__fd,__offset,__whence))
#endif /* !__lseek_defined */
#ifndef __mktemp_defined
#define __mktemp_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,__NONNULL((1)),char *,__LIBCCALL,mktemp,(char *__template),mktemp,(__template))
#endif /* !__mktemp_defined */
#ifndef __umask_defined
#define __umask_defined 1
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,,__mode_t,__LIBCCALL,umask,(__mode_t __mode),umask,(__mode))
#endif /* !__umask_defined */
#ifndef __read_defined
#define __read_defined 1
#if !defined(__DOS_COMPAT__) && __SIZEOF_SIZE_T__ == 4
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,__WUNUSED_SUGGESTED __NONNULL((2)),__INT32_TYPE__,__LIBCCALL,read,(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize),read,(__fd,__dstbuf,__bufsize))
#elif defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __NONNULL((2)),__INT32_TYPE__,__LIBCCALL,read,(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize),_read,(__fd,__dstbuf,__bufsize))
#else
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __NONNULL((2)),__SSIZE_TYPE__,__LIBCCALL,__readsz,(int __fd, void *__dstbuf, size_t __bufsize),read,(__fd,__dstbuf,__bufsize))
__LOCAL __WUNUSED_SUGGESTED __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL read)(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize) { return __readsz(__fd,__dstbuf,__bufsize); }
#endif
#endif /* !__read_defined */
#ifndef __write_defined
#define __write_defined 1
#if !defined(__DOS_COMPAT__) && __SIZEOF_SIZE_T__ == 4
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,__NONNULL((2)),__INT32_TYPE__,__LIBCCALL,write,(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize),write,(__fd,__buf,__bufsize))
#elif defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__NONNULL((2)),__INT32_TYPE__,__LIBCCALL,write,(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize),_write,(__fd,__buf,__bufsize))
#else
__REDIRECT(__LIBC,__NONNULL((2)),__SSIZE_TYPE__,__LIBCCALL,__writesz,(int __fd, void const *__buf, size_t __bufsize),write,(__fd,__buf,__bufsize))
__LOCAL __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL write)(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize) { return __writesz(__fd,__buf,__bufsize); }
#endif
#endif /* !__write_defined */


__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,__NONNULL((1)),int,__LIBCCALL,_access,(char const *__file, int __type),access,(__file,__type))
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,_creat,(char const *__file, int __pmode),creat,(__file,__pmode));
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,__NONNULL((1)),int,__LIBCCALL,_chmod,(char const *__file, int __mode),chmod,(__file,__mode));
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,_access_s,(char const *__file, int __type),access_s,(__file,__type));
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_chsize,(int __fd, __LONG32_TYPE__ __size),ftruncate,(__fd,__size));
__REDIRECT_IFKOS(__LIBC,,errno_t,__LIBCCALL,_chsize_s,(int __fd, __INT64_TYPE__ __size),ftruncate64,(__fd,__size));
#ifndef ___unlink_defined
#define ___unlink_defined 1
__REDIRECT_UFS_FUNC_OLDPEB(__LIBC,__NONNULL((1)),int,__LIBCCALL,_unlink,(char const *__name),unlink,(__name))
#endif /* !___unlink_defined */
__REDIRECT_PE_FUNC_OLDPEB(__LIBC,,int,__LIBCCALL,_close,(int __fd),close,(__fd))
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_commit,(int __fd),fdatasync,(__fd))
__REDIRECT_PE_FUNC_OLDPEB(__LIBC,,int,__LIBCCALL,_dup,(int __fd),dup,(__fd))
__REDIRECT_PE_FUNC_OLDPEB(__LIBC,,int,__LIBCCALL,_dup2,(int __ofd, int __nfd),dup2,(__ofd,__nfd))
__REDIRECT_IFKOS(__LIBC,,__LONG32_TYPE__,__LIBCCALL,_lseek,(int __fd, __LONG32_TYPE__ __offset, int __whence),lseek,(__fd,__offset,__whence))
__REDIRECT_IFKOS(__LIBC,,__INT64_TYPE__,__LIBCCALL,_lseeki64,(int __fd, __INT64_TYPE__ __offset, int __whence),lseek64,(__fd,__offset,__whence))
#if __SIZEOF_SIZE_T__ == 4 && !defined(__DOS_COMPAT__)
__REDIRECT_PE_FUNC_OLDPEB(__LIBC,__WUNUSED_SUGGESTED __NONNULL((2)),__INT32_TYPE__,__LIBCCALL,_read,(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize),read,(__fd,__dstbuf,__bufsize))
__REDIRECT_PE_FUNC_OLDPEB(__LIBC,__NONNULL((2)),__INT32_TYPE__,__LIBCCALL,_write,(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize),write,(__fd,__dstbuf,__bufsize))
#elif defined(__CRT_DOS)
__LIBC __WUNUSED_SUGGESTED __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL _read)(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize);
__LIBC __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL _write)(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize);
#else
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED,__INT32_TYPE__,__LIBCCALL,__read_sz,(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize),read,(__fd,__dstbuf,__bufsize))
__REDIRECT(__LIBC,,__INT32_TYPE__,__LIBCCALL,__write_sz,(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize),write,(__fd,__dstbuf,__bufsize))
__LOCAL __WUNUSED_SUGGESTED __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL _read)(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize) { return (__INT32_TYPE__)__read_sz(__fd,__dstbuf,(size_t)__bufsize); }
__LOCAL __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL _write)(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize) { return (__INT32_TYPE__)__write_sz(__fd,__dstbuf,(size_t)__bufsize); }
#endif

__LIBC __WUNUSED int (__ATTR_CDECL _open)(char const *__file, int __oflag, ...) __UFS_FUNC_OLDPEB(open); /* TODO: Use redirection. */
__REDIRECT_PE_FUNC_OLDPEB(__LIBC,,int,__LIBCCALL,_setmode,(int __fd, int __mode),setmode,(__fd,__mode))
__REDIRECT_PE_FUNC_OLDPEB(__LIBC,,__mode_t,__LIBCCALL,_umask,(__mode_t __mode),umask,(__mode))
__REDIRECT_PE_FUNC_OLDPEB(__LIBC,__WUNUSED,int,__LIBCCALL,_isatty,(int __fd),isatty,(__fd))
__REDIRECT_IFKOS(__LIBC,,int,__LIBCCALL,_locking,(int __fd, int __lockmode, __LONG32_TYPE__ __numofbytes),lockf,(__fd,__lockmode,__numofbytes))

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY_ALT(closedir) int (__LIBCCALL _findclose)(intptr_t __findfd);
#ifdef __USE_DOS_LINKOLDFINDSTAT
__REDIRECT_UFS_(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst32,(char const *__file, struct _finddata32_t *__finddata),_findfirst,(__file,__finddata))
__REDIRECT_UFS_(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst32i64,(char const *__file, struct _finddata32i64_t *__finddata),_findfirsti64,(__file,__finddata))
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)),int,__LIBCCALL,_findnext32,(intptr_t __findfd, struct _finddata32_t *__finddata),_findnext,(__findfd,__finddata))
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)),int,__LIBCCALL,_findnext32i64,(intptr_t __findfd, struct _finddata32i64_t *__finddata),_findnexti64,(__findfd,__finddata))
#else /* __USE_DOS_LINKOLDFINDSTAT */
__REDIRECT_UFS(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst32,(char const *__file, struct _finddata32_t *__finddata),_findfirst32,(__file,__finddata))
__REDIRECT_UFS(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst32i64,(char const *__file, struct _finddata32i64_t *__finddata),_findfirst32i64,(__file,__finddata))
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)) int (__LIBCCALL _findnext32)(intptr_t __findfd, struct _finddata32_t *__finddata);
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)) int (__LIBCCALL _findnext32i64)(intptr_t __findfd, struct _finddata32i64_t *__finddata);
#endif /* !__USE_DOS_LINKOLDFINDSTAT */
__REDIRECT_UFS_(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst64i32,(char const *__file, struct _finddata64i32_t *__finddata),_findfirst64,(__file,__finddata))
__REDIRECT_UFS(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst64,(char const *__file, struct __finddata64_t *__finddata),_findfirst64,(__file,__finddata))
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)) int (__LIBCCALL _findnext64)(intptr_t __findfd, struct __finddata64_t *__finddata);
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)),int,__LIBCCALL,_findnext64i32,(intptr_t __findfd, struct _finddata64i32_t *__finddata),_findnext64,(__findfd,__finddata))

__REDIRECT_UFS_(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(open) __NONNULL((1,2)),errno_t,__LIBCCALL,_sopen_s,(int *__fd, char const *__file, int __oflag, int __sflag, int __pmode),_sopen_s,(__fd,__file,__oflag,__sflag,__pmode))
__REDIRECT_UFS_(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(open) __NONNULL((1,2)),errno_t,__LIBCCALL,_sopen_s_nolock,(int *__fd, char const *__file, int __oflag, int __sflag, int __pmode),_sopen_s,(__fd,__file,__oflag,__sflag,__pmode))
__LIBC __WUNUSED __PORT_DOSONLY_ALT(lseek) int (__LIBCCALL _eof)(int __fd);
__LIBC __PORT_DOSONLY_ALT(mktemp) __NONNULL((1)) errno_t (__LIBCCALL _mktemp_s)(char *__templatename, size_t __size);
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(pipe) __NONNULL((1)) int (__LIBCCALL _pipe)(int __pipedes[2], __UINT32_TYPE__ __pipesize, int __textmode); /* TODO: Emulate outside of DOS. */
__LIBC __WUNUSED __PORT_DOSONLY_ALT(open) int (__ATTR_CDECL _sopen)(char const *__file, int __oflag, int __sflag, ...) __UFS_FUNC_OLDPEB(sopen); /* TODO: Emulate outside of DOS. */

__LIBC __WUNUSED __LONG32_TYPE__ (__LIBCCALL _filelength)(int __fd);
__LIBC __WUNUSED __INT64_TYPE__ (__LIBCCALL _filelengthi64)(int __fd);
__LIBC __WUNUSED __LONG32_TYPE__ (__LIBCCALL _tell)(int __fd);
__LIBC __WUNUSED __INT64_TYPE__ (__LIBCCALL _telli64)(int __fd);
__LIBC errno_t (__LIBCCALL _umask_s)(int __newmode, int *__oldmode);
__LIBC int (__LIBCCALL __lock_fhandle)(int __fd);
__LIBC void (__LIBCCALL _unlock_fhandle)(int __fd);
__LIBC __WUNUSED intptr_t (__LIBCCALL _get_osfhandle)(int __fd); /* return __fd */
__LIBC __WUNUSED int (__LIBCCALL _open_osfhandle)(intptr_t __osfd, int __flags); /* dup() */
#else /* __CRT_DOS */
__LOCAL __WUNUSED __INT64_TYPE__ (__LIBCCALL _filelengthi64)(int __fd) {
 __INT64_TYPE__ __oldpos = _lseeki64(__fd,0,1);
 __INT64_TYPE__ __length = __oldpos >= 0 ? _lseeki64(__fd,0,2) : -1;
 return __oldpos >= 0 ? (_lseeki64(__fd,__oldpos,0) >= 0 ? __length : -1) : -1;
}
__LOCAL __WUNUSED __LONG32_TYPE__ (__LIBCCALL _filelength)(int __fd) { return (__LONG32_TYPE__)_filelengthi64(__fd); }
__LOCAL __WUNUSED __LONG32_TYPE__ (__LIBCCALL _tell)(int __fd) { return _lseek(__fd,0,1); }
__LOCAL __WUNUSED __INT64_TYPE__ (__LIBCCALL _telli64)(int __fd) { return _lseeki64(__fd,0,1); }
__LOCAL errno_t (__LIBCCALL _umask_s)(int __newmode, int *__oldmode) { *__oldmode = _umask(__newmode); return 0; }
__LOCAL int (__LIBCCALL __lock_fhandle)(int __UNUSED(__fd)) { return 0; }
__LOCAL void (__LIBCCALL _unlock_fhandle)(int __UNUSED(__fd)) { }
__LOCAL __WUNUSED intptr_t (__LIBCCALL _get_osfhandle)(int __fd) { return __fd; }
__LOCAL int (__LIBCCALL _open_osfhandle)(intptr_t __osfd, int __UNUSED(__flags)) { return _dup((int)__osfd); }
#endif /* __CRT_DOS */

/* Weird, new functions not apart of any standard. */
__REDIRECT2(__LIBC,,int,__LIBCCALL,chsize,(int __fd, __LONG32_TYPE__ __size),ftruncate,_chsize,(__fd,__size))
__REDIRECT2(__LIBC,,int,__LIBCCALL,locking,(int __fd, int __lockmode, __LONG32_TYPE__ __numofbytes),lockf,_locking,(__fd,__lockmode,__numofbytes))
__REDIRECT_PE_FUNC_OLDPEA(__LIBC,,int,__LIBCCALL,setmode,(int __fd, int __mode),setmode,(__fd,__mode)) /* F_SETFL */
#ifdef __CRT_DOS
__LIBC __WUNUSED __PORT_DOSONLY_ALT(open) int (__ATTR_CDECL sopen)(char const *__file, int __oflag, int __sflag, ...) __UFS_FUNC_OLDPEA(sopen); /* TODO: Emulate outside of DOS. */
__REDIRECT(__LIBC,__WUNUSED,__LONG32_TYPE__,__LIBCCALL,filelength,(int __fd),_filelength,(__fd)) /* lseek(fd,SEEK_END,0) */
__REDIRECT(__LIBC,__WUNUSED,__LONG32_TYPE__,__LIBCCALL,tell,(int __fd),_tell,(__fd)) /* lseek(fd,SEEK_CUR,0) */
__REDIRECT(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(lseek),int,__LIBCCALL,eof,(int __fd),_eof,(__fd)) /* lseek(fd,SEEK_CUR,0) == lseek(fd,SEEK_END,0) */
#else /* __CRT_DOS */
__LOCAL __WUNUSED __LONG32_TYPE__ (__LIBCCALL tell)(int __fd) { return _tell(__fd); }
__LOCAL __WUNUSED __LONG32_TYPE__ (__LIBCCALL filelength)(int __fd) { return _filelength(__fd); }
#endif /* __CRT_DOS */

#ifdef __CRT_DOS
#ifndef _WIO_DEFINED
#define _WIO_DEFINED 1
struct _wfinddata32_t;
struct _wfinddata64_t;
struct _wfinddata32i64_t;
struct _wfinddata64i32_t;

__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY __NONNULL((1)) int (__ATTR_CDECL _wopen)(wchar_t const *__restrict __file, int __oflag, ...) __WFS_FUNC(_wopen); /* TODO: Use redirection */
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY __NONNULL((1)) int (__ATTR_CDECL _wsopen)(wchar_t const *__restrict __file, int __oflag, int __sflag, ...) __WFS_FUNC(_wsopen); /* TODO: Use redirection */
__REDIRECT_WFS(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,_wcreat,(wchar_t const *__restrict __file, int __pmode),_wcreat,(__file,__pmode))
__REDIRECT_WFS(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY __NONNULL((1,2)),errno_t,__LIBCCALL,_wsopen_s,(int *__restrict __fd, wchar_t const *__restrict __file, int __oflag, int __sflag, int __pflags),_wsopen_s,(__fd,__file,__oflag,__sflag,__pflags))
__REDIRECT_WFS(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,_waccess,(wchar_t const *__restrict __file, int __type),_waccess,(__file,__type))
__REDIRECT_WFS(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),errno_t,__LIBCCALL,_waccess_s,(wchar_t const *__restrict __file, int __type),_waccess,(__file,__type))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,_wchmod,(wchar_t const *__restrict __file, int __mode),_wchmod,(__file,__mode))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,_wunlink,(wchar_t const *__restrict __file),_wunlink,(__file))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,_wrename,(wchar_t const *__restrict __oldname, wchar_t const *__restrict __newname),_wrename,(__oldname,__newname))
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY __NONNULL((1)) errno_t (__LIBCCALL _wmktemp_s)(wchar_t *__restrict __templatename, size_t __sizeinwords);
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_wfindfirst32,(wchar_t const *__restrict __file, struct _wfinddata32_t *__restrict __finddata),_wfindfirst32,(__file,__finddata))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_wfindfirst64,(wchar_t const *__restrict __file, struct _wfinddata64_t *__restrict __finddata),_wfindfirst64,(__file,__finddata))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_wfindfirst32i64,(wchar_t const *__restrict __file, struct _wfinddata32i64_t *__restrict __finddata),_wfindfirst32i64,(__file,__finddata))
__REDIRECT_WFS(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_wfindfirst64i32,(wchar_t const *__restrict __file, struct _wfinddata64i32_t *__restrict __finddata),_wfindfirst64i32,(__file,__finddata))
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY __NONNULL((2)) int (__LIBCCALL _wfindnext32)(intptr_t __findfd, struct _wfinddata32_t *__restrict __finddata);
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY __NONNULL((2)) int (__LIBCCALL _wfindnext64)(intptr_t __findfd, struct _wfinddata64_t *__restrict __finddata);
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY __NONNULL((2)) int (__LIBCCALL _wfindnext32i64)(intptr_t __findfd, struct _wfinddata32i64_t *__restrict __finddata);
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY __NONNULL((2)) int (__LIBCCALL _wfindnext64i32)(intptr_t __findfd, struct _wfinddata64i32_t *__restrict __finddata);
#endif /* !_WIO_DEFINED */
#endif /* __CRT_DOS */
#endif /* !__KERNEL__ */

/* Safely first! */
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("attrib")
#pragma push_macro("time_create")
#pragma push_macro("time_access")
#pragma push_macro("time_write")
#pragma push_macro("size")
#pragma push_macro("name")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#undef attrib
#undef time_create
#undef time_access
#undef time_write
#undef size
#undef name

#ifndef _FINDDATA_T_DEFINED
#define _FINDDATA_T_DEFINED 1
struct _finddata32_t {
 __UINT32_TYPE__ attrib;
 __time32_t      time_create;
 __time32_t      time_access;
 __time32_t      time_write;
 _fsize_t        size;
 char            name[260];
};
struct _finddata32i64_t {
 __UINT32_TYPE__ attrib;
 __time32_t      time_create;
 __time32_t      time_access;
 __time32_t      time_write;
 /* Microsoft:
  * A: "I mean: we could use an unsigned type for this, seeing as how _fsize_t is also unsigned."
  * B: "Nah! - Lets rather p1$$ off anyone that notices. - That'll be way more fun." */
 __INT64_TYPE__  size;
 char            name[260];
};
struct _finddata64i32_t {
 __UINT32_TYPE__ attrib;
 __time64_t      time_create;
 __time64_t      time_access;
 __time64_t      time_write;
union{
 _fsize_t        size;
 __INT64_TYPE__ __pad; /* I think this is correct? */
};
 char            name[260];
};
struct __finddata64_t {
 __UINT32_TYPE__ attrib;
 __time64_t      time_create;
 __time64_t      time_access;
 __time64_t      time_write;
 __INT64_TYPE__  size;
 char            name[260];
};


#ifdef __USE_TIME_BITS64
#define _finddata_t     _finddata64i32_t
#define _finddatai64_t  __finddata64_t
#define _findfirst(file,finddata)     _findfirst64i32(file,finddata)
#define _findnext(findfd,finddata)    _findnext64i32(findfd,finddata)
#define _findfirsti64(file,finddata)  _findfirst64(file,finddata)
#define _findnexti64(findfd,finddata) _findnext64(findfd,finddata)
#else /* __USE_TIME_BITS64 */
#define _finddata_t     _finddata32_t
#define _finddatai64_t  _finddata32i64_t
#define _findfirst(file,finddata)     _findfirst32(file,finddata)
#define _findnext(findfd,finddata)    _findnext32(findfd,finddata)
#define _findfirsti64(file,finddata)  _findfirst32i64(file,finddata)
#define _findnexti64(findfd,finddata) _findnext32i64(findfd,finddata)
#endif /* !__USE_TIME_BITS64 */
#endif /* !_FINDDATA_T_DEFINED */

#ifndef _WFINDDATA_T_DEFINED
#define _WFINDDATA_T_DEFINED 1
struct _wfinddata32_t {
 __UINT32_TYPE__ attrib;
 __time32_t      time_create;
 __time32_t      time_access;
 __time32_t      time_write;
 _fsize_t        size;
 wchar_t         name[260];
};

struct _wfinddata32i64_t {
 __UINT32_TYPE__ attrib;
 __time32_t      time_create;
 __time32_t      time_access;
 __time32_t      time_write;
 __INT64_TYPE__  size;
 wchar_t         name[260];
};

struct _wfinddata64i32_t {
 __UINT32_TYPE__ attrib;
 __time64_t      time_create;
 __time64_t      time_access;
 __time64_t      time_write;
 _fsize_t        size;
 wchar_t         name[260];
};

struct _wfinddata64_t {
 __UINT32_TYPE__ attrib;
 __time64_t      time_create;
 __time64_t      time_access;
 __time64_t      time_write;
 __INT64_TYPE__  size;
 wchar_t         name[260];
};

#ifdef __USE_TIME_BITS64
#define _wfinddata_t    _wfinddata64i32_t
#define _wfindfirst     _wfindfirst64i32
#define _wfindnext      _wfindnext64i32
#define _wfinddatai64_t _wfinddata64_t
#define _wfindfirsti64  _wfindfirst64
#define _wfindnexti64   _wfindnext64
#else /* __USE_TIME_BITS64 */
#define _wfinddata_t    _wfinddata32_t
#define _wfinddatai64_t _wfinddata32i64_t
#define _wfindfirst     _wfindfirst32
#define _wfindnext      _wfindnext32
#define _wfindfirsti64  _wfindfirst32i64
#define _wfindnexti64   _wfindnext32i64
#endif /* !__USE_TIME_BITS64 */
#endif /* !_WFINDDATA_T_DEFINED */

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("name")
#pragma pop_macro("size")
#pragma pop_macro("time_write")
#pragma pop_macro("time_access")
#pragma pop_macro("time_create")
#pragma pop_macro("attrib")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

__SYSDECL_END

#endif /* !_IO_H */
