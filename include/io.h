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

__DECL_BEGIN

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
#ifndef __remove_defined
#define __remove_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __NONNULL((1)) int (__LIBCCALL remove)(char const *__file) __UFS_FUNC(remove);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(remove)
#endif /* !__remove_defined */
#ifndef __rename_defined
#define __rename_defined 1
__NAMESPACE_STD_BEGIN
__LIBC int (__LIBCCALL rename)(char const *__old, char const *__new) __UFS_FUNC(rename);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(rename)
#endif /* !__rename_defined */
#ifndef __unlink_defined
#define __unlink_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL unlink)(char const *__name) __UFS_FUNC_OLDPEA(unlink);
#endif /* !__unlink_defined */
#ifndef __open_defined
#define __open_defined 1
__LIBC __NONNULL((1)) int (__ATTR_CDECL open)(char const *__file, int __oflag, ...) __UFS_FUNCn_OLDPEA(open);
#endif /* !__open_defined */
#ifndef __creat_defined
#define __creat_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL creat)(char const *__file, int __pmode) __UFS_FUNCn_OLDPEA(creat);
#endif /* !__creat_defined */
#ifndef __access_defined
#define __access_defined 1
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL access)(char const *__name, int __type) __UFS_FUNC_OLDPEA(access);
#endif /* !__access_defined */
#ifndef __chmod_defined
#define __chmod_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL chmod)(char const *__file, int __mode) __UFS_FUNC_OLDPEA(chmod);
#endif /* !__chmod_defined */
#ifndef __close_defined
#define __close_defined 1
__LIBC int (__LIBCCALL close)(int __fd) __PE_FUNC_OLDPEA(close);
#endif /* !__close_defined */
#ifndef __dup_defined
#define __dup_defined 1
__LIBC __WUNUSED int (__LIBCCALL dup)(int __fd) __PE_FUNC_OLDPEA(dup);
#endif /* !__dup_defined */
#ifndef __dup2_defined
#define __dup2_defined 1
__LIBC int (__LIBCCALL dup2)(int __ofd, int __nfd) __PE_FUNC_OLDPEA(dup2);
#endif /* !__dup2_defined */
#ifndef __isatty_defined
#define __isatty_defined 1
__LIBC __WUNUSED int (__LIBCCALL isatty)(int __fd) __PE_FUNC_OLDPEA(isatty);
#endif /* !__isatty_defined */
#ifndef __lseek_defined
#define __lseek_defined 1
__LIBC __LONG32_TYPE__ (__LIBCCALL lseek)(int __fd, __LONG32_TYPE__ __offset, int __whence) __PE_FUNC_OLDPEA(lseek);
#endif /* !__lseek_defined */
#ifndef __mktemp_defined
#define __mktemp_defined 1
__LIBC __NONNULL((1)) char *(__LIBCCALL mktemp)(char *__template) __PE_FUNC_OLDPEA(mktemp);
#endif /* !__mktemp_defined */
#ifndef __umask_defined
#define __umask_defined 1
__LIBC int (__LIBCCALL umask)(int __mode) __PE_ASMNAME("_uname");
#endif /* !__umask_defined */
#ifndef __read_defined
#define __read_defined 1
#if __SIZEOF_SIZE_T__ == 4
__LIBC __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL read)(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize) __PE_FUNC_OLDPEA(read);
#else /* __SIZEOF_SIZE_T__ == 4 */
__LIBC __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL read)(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize) __ASMNAME("_read");
#endif /* __SIZEOF_SIZE_T__ != 4 */
#endif /* !__read_defined */
#ifndef __write_defined
#define __write_defined 1
#if __SIZEOF_SIZE_T__ == 4
__LIBC __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL write)(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize) __PE_FUNC_OLDPEA(write);
#else /* __SIZEOF_SIZE_T__ == 4 */
__LIBC __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL write)(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize) __ASMNAME("_write");
#endif /* __SIZEOF_SIZE_T__ != 4 */
#endif /* !__write_defined */


__LIBC __NONNULL((1)) int (__LIBCCALL _access)(char const *__file, int __type) __UFS_FUNC_OLDPEB(access);
__LIBC __NONNULL((1)) int (__LIBCCALL _creat)(char const *__file, int __pmode) __UFS_FUNC_OLDPEB(creat);
__LIBC __NONNULL((1)) int (__LIBCCALL _chmod)(char const *__file, int __mode) __UFS_FUNC_OLDPEB(chmod);
__LIBC __NONNULL((1)) errno_t (__LIBCCALL _access_s)(char const *__file, int __type) __UFS_FUNC_OLDPEB(access_s);
__LIBC int (__LIBCCALL _chsize)(int __fd, __LONG32_TYPE__ __size) __KOS_ASMNAME("ftruncate");
__LIBC errno_t (__LIBCCALL _chsize_s)(int __fd, __INT64_TYPE__ __size) __KOS_ASMNAME("ftruncate64");
#ifndef ___unlink_defined
#define ___unlink_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL _unlink)(char const *__file) __UFS_FUNC_OLDPEB(unlink);
#endif /* !___unlink_defined */
__LIBC int (__LIBCCALL _close)(int __fd) __PE_FUNC_OLDPEB(close);
__LIBC int (__LIBCCALL _commit)(int __fd) __KOS_ASMNAME("fdatasync");
__LIBC int (__LIBCCALL _dup)(int __fd) __PE_FUNC_OLDPEB(dup);
__LIBC int (__LIBCCALL _dup2)(int __filehandlesrc, int __filehandledst) __PE_FUNC_OLDPEB(dup2);
__LIBC int (__LIBCCALL _eof)(int __fd);
__LIBC __LONG32_TYPE__ (__LIBCCALL _filelength)(int __fd);
__LIBC __INT64_TYPE__ (__LIBCCALL _filelengthi64)(int __fd);
__LIBC int (__LIBCCALL _isatty)(int __fd) __PE_FUNC_OLDPEB(isatty);
__LIBC int (__LIBCCALL _locking)(int __fd, int __lockmode, __LONG32_TYPE__ __numofbytes) __KOS_ASMNAME("lockf");
__LIBC __NONNULL((1)) errno_t (__LIBCCALL _mktemp_s)(char *__templatename, size_t __size);
__LIBC __NONNULL((1)) int (__LIBCCALL _pipe)(int __pipedes[2], __UINT32_TYPE__ __pipesize, int __textmode);
__LIBC __LONG32_TYPE__ (__LIBCCALL _lseek)(int __fd, __LONG32_TYPE__ __offset, int __whence) __KOS_ASMNAME("lseek");
__LIBC __INT64_TYPE__ (__LIBCCALL _lseeki64)(int __fd, __INT64_TYPE__ __offset, int __whence) __KOS_ASMNAME("lseek64");
#if __SIZEOF_SIZE_T__ == 4
__LIBC __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL _read)(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize) __PE_FUNC_OLDPEB(read);
__LIBC __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL _write)(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize) __PE_FUNC_OLDPEB(write);
#else /* __SIZEOF_SIZE_T__ == 4 */
__LIBC __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL _read)(int __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize);
__LIBC __NONNULL((2)) __INT32_TYPE__ (__LIBCCALL _write)(int __fd, void const *__buf, __UINT32_TYPE__ __bufsize);
#endif /* __SIZEOF_SIZE_T__ != 4 */

__LIBC __LONG32_TYPE__ (__LIBCCALL _tell)(int __fd);
__LIBC __INT64_TYPE__ (__LIBCCALL _telli64)(int __fd);
__LIBC int (__LIBCCALL _setmode)(int __fd, int __mode) __PE_FUNC_OLDPEB(setmode);
__LIBC int (__LIBCCALL _umask)(int __mode) __PE_FUNC_OLDPEB(umask);
__LIBC errno_t (__LIBCCALL _umask_s)(int __newmode, int *__oldmode);
__LIBC __NONNULL((1,2)) errno_t (__LIBCCALL _sopen_s)(int *__fd, char const *__file, int __oflag, int __sflag, int __pmode) __UFS_FUNC_(_sopen_s);
__LIBC __NONNULL((1,2)) errno_t (__LIBCCALL _sopen_s_nolock)(int *__fd, char const *__file, int __oflag, int __sflag, int __pmode) __UFS_FUNC_(_sopen_s);

__LIBC __NONNULL((1,2)) intptr_t (__LIBCCALL _findfirst32)(char const *__file, struct _finddata32_t *__finddata) __UFS_FUNC_(_findfirst32);
__LIBC __NONNULL((1,2)) intptr_t (__LIBCCALL _findfirst64)(char const *__file, struct __finddata64_t *__finddata) __UFS_FUNC_(_findfirst64);
__LIBC __NONNULL((1,2)) intptr_t (__LIBCCALL _findfirst32i64)(char const *__file, struct _finddata32i64_t *__finddata) __UFS_FUNC_(_findfirst32i64);
__LIBC __NONNULL((1,2)) intptr_t (__LIBCCALL _findfirst64i32)(char const *__file, struct _finddata64i32_t *__finddata) __UFS_FUNC_(_findfirst64i32);
__LIBC __NONNULL((2)) int (__LIBCCALL _findnext32)(intptr_t __findfd, struct _finddata32_t *__finddata);
__LIBC __NONNULL((2)) int (__LIBCCALL _findnext64)(intptr_t __findfd, struct __finddata64_t *__finddata);
__LIBC __NONNULL((2)) int (__LIBCCALL _findnext32i64)(intptr_t __findfd, struct _finddata32i64_t *__finddata);
__LIBC __NONNULL((2)) int (__LIBCCALL _findnext64i32)(intptr_t __findfd, struct _finddata64i32_t *__finddata);
__LIBC int (__LIBCCALL _findclose)(intptr_t __findfd);

__LIBC int (__ATTR_CDECL _open)(char const *__file, int __oflag, ...) __UFS_FUNC_OLDPEB(open);
__LIBC int (__ATTR_CDECL _sopen)(char const *__file, int __oflag, int __sflag, ...) __UFS_FUNC_OLDPEB(sopen);

__LIBC int  (__LIBCCALL __lock_fhandle)(int __fd);
__LIBC void (__LIBCCALL _unlock_fhandle)(int __fd);
__LIBC intptr_t (__LIBCCALL _get_osfhandle)(int __fd); /* return __fd */
__LIBC int (__LIBCCALL _open_osfhandle)(intptr_t __osfd, int __flags); /* dup() */

/* Weird, new functions not apart of any standard. */
__LIBC __LONG32_TYPE__ (__LIBCCALL filelength)(int __fd) __ASMNAME("_filelength"); /* lseek(fd,SEEK_END,0) */
__LIBC int (__LIBCCALL chsize)(int __fd, __LONG32_TYPE__ __size) __ASMNAME2("ftruncate","_chsize");
__LIBC int (__LIBCCALL locking)(int __fd, int __lockmode, __LONG32_TYPE__ __numofbytes) __ASMNAME2("lockf","_locking");
__LIBC int (__ATTR_CDECL sopen)(char const *__file, int __oflag, int __sflag, ...) __UFS_FUNC_OLDPEA(sopen);
__LIBC int (__LIBCCALL setmode)(int __fd, int __mode) __PE_FUNC_OLDPEA(setmode); /* F_SETFL */
__LIBC __LONG32_TYPE__ (__LIBCCALL tell)(int __fd) __ASMNAME("_tell"); /* lseek(fd,SEEK_CUR,0) */
__LIBC int (__LIBCCALL eof)(int __fd) __ASMNAME("_eof"); /* lseek(fd,SEEK_CUR,0) == lseek(fd,SEEK_END,0) */

#ifndef _WIO_DEFINED
#define _WIO_DEFINED 1
struct _wfinddata32_t;
struct _wfinddata64_t;
struct _wfinddata32i64_t;
struct _wfinddata64i32_t;

__LIBC int (__LIBCCALL _wcreat)(wchar_t const *__file, int __pmode) __WFS_FUNC(_wcreat);
__LIBC int (__ATTR_CDECL _wopen)(wchar_t const *__file, int __oflag, ...) __WFS_FUNC(_wopen);
__LIBC int (__ATTR_CDECL _wsopen)(wchar_t const *__file, int __oflag, int __sflag, ...) __WFS_FUNC(_wsopen);
__LIBC errno_t (__LIBCCALL _wsopen_s)(int *__fd, wchar_t const *__file, int __oflag, int __sflag, int __pflags) __WFS_FUNC(_wsopen_s);

__LIBC int (__LIBCCALL _waccess)(wchar_t const *__file, int __type) __WFS_FUNC(_waccess);
__LIBC errno_t (__LIBCCALL _waccess_s)(wchar_t const *__file, int __type) __WFS_FUNC(_waccess);

__LIBC int (__LIBCCALL _wchmod)(wchar_t const *__file, int __mode) __WFS_FUNC(_wchmod);
__LIBC int (__LIBCCALL _wunlink)(wchar_t const *__file) __WFS_FUNC(_wunlink);
__LIBC int (__LIBCCALL _wrename)(wchar_t const *__oldname, wchar_t const *__newname) __WFS_FUNC(_wrename);
__LIBC errno_t (__LIBCCALL _wmktemp_s)(wchar_t *__templatename, size_t __sizeinwords);

__LIBC intptr_t (__LIBCCALL _wfindfirst32)(wchar_t const *__file, struct _wfinddata32_t *__finddata) __WFS_FUNC(_wfindfirst32);
__LIBC intptr_t (__LIBCCALL _wfindfirst64)(wchar_t const *__file, struct _wfinddata64_t *__finddata) __WFS_FUNC(_wfindfirst64);
__LIBC intptr_t (__LIBCCALL _wfindfirst32i64)(wchar_t const *__file, struct _wfinddata32i64_t *__finddata) __WFS_FUNC(_wfindfirst32i64);
__LIBC intptr_t (__LIBCCALL _wfindfirst64i32)(wchar_t const *__file, struct _wfinddata64i32_t *__finddata) __WFS_FUNC(_wfindfirst64i32);
__LIBC int (__LIBCCALL _wfindnext32)(intptr_t __findfd, struct _wfinddata32_t *__finddata);
__LIBC int (__LIBCCALL _wfindnext64)(intptr_t __findfd, struct _wfinddata64_t *__finddata);
__LIBC int (__LIBCCALL _wfindnext32i64)(intptr_t __findfd, struct _wfinddata32i64_t *__finddata);
__LIBC int (__LIBCCALL _wfindnext64i32)(intptr_t __findfd, struct _wfinddata64i32_t *__finddata);
#endif /* !_WIO_DEFINED */

#endif /* !__KERNEL__ */

#ifndef _FINDDATA_T_DEFINED
#define _FINDDATA_T_DEFINED 1
/* Safely first! */
#undef attrib
#undef time_create
#undef time_access
#undef time_write
#undef size
#undef name

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
  * A: "I mean: we could use an unsigned type for this, seeing as how _fsize_t also is unsigned."
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
#define _findfirst      _findfirst64i32
#define _findnext       _findnext64i32
#define _findfirsti64   _findfirst64
#define _findnexti64    _findnext64
#else /* __USE_TIME_BITS64 */
#define _finddata_t     _finddata32_t
#define _finddatai64_t  _finddata32i64_t
#define _findfirst      _findfirst32
#define _findnext       _findnext32
#define _findfirsti64   _findfirst32i64
#define _findnexti64    _findnext32i64
#endif /* !__USE_TIME_BITS64 */
#endif /* !_FINDDATA_T_DEFINED */

#ifndef _WFINDDATA_T_DEFINED
#define _WFINDDATA_T_DEFINED 1
/* Safely first! */
#undef attrib
#undef time_create
#undef time_access
#undef time_write
#undef size
#undef name

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


__DECL_END

#endif /* !_IO_H */
