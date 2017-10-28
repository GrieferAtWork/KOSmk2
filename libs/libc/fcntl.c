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
#ifndef GUARD_LIBS_LIBC_FCNTL_C
#define GUARD_LIBS_LIBC_FCNTL_C 1
#define _GNU_SOURCE       1
#define _KOS_SOURCE       1
#define _ATFILE_SOURCE    1
#define _FILE_OFFSET_BITS 32

#include "libc.h"
#include "system.h"
#include "malloc.h"
#include "errno.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "unicode.h"
#include "unistd.h"

#include <hybrid/compiler.h>
#include <kos/fcntl.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <bits/fcntl-linux.h>

DECL_BEGIN

INTERN ssize_t ATTR_CDECL libc_ioctl(int fd, unsigned long int request, ...) {
 va_list args; int result;
 va_start(args,request);
 result = sys_ioctl(fd,request,va_arg(args,void *));
 va_end(args);
 if (E_ISERR(result)) {
  SET_ERRNO(-result);
  result = -1;
 }
 return result;
}
INTERN ssize_t ATTR_CDECL libc_fcntl(int fd, int cmd, ...) {
 va_list args; int result;
 va_start(args,cmd);
 result = sys_fcntl(fd,cmd,va_arg(args,void *));
 va_end(args);
 if (E_ISERR(result)) {
  SET_ERRNO(-result);
  result = -1;
 }
 return result;
}
INTERN int ATTR_CDECL libc_openat(int fd, char const *file, int oflag, ...) {
 va_list args; int result;
 va_start(args,oflag);
 result = sys_openat(fd,file,oflag,va_arg(args,mode_t));
 va_end(args);
 if (E_ISERR(result)) {
  SET_ERRNO(-result);
  result = -1;
 }
 return result;
}
INTERN int ATTR_CDECL libc_open(char const *file, int oflag, ...) {
 va_list args; int result;
 va_start(args,oflag);
 result = sys_openat(AT_FDCWD,file,oflag,va_arg(args,mode_t));
 va_end(args);
 if (E_ISERR(result)) {
  SET_ERRNO(-result);
  result = -1;
 }
 return result;
}
INTERN int LIBCCALL libc_creat(char const *file, mode_t mode) {
 return libc_open(file,O_CREAT|O_WRONLY|O_TRUNC,mode);
}


INTERN ssize_t LIBCCALL
libc_xvirtinfo2(VIRT void *addr, USER struct virtinfo *buf,
                size_t bufsize, u32 flags) {
 return FORWARD_SYSTEM_VALUE(sys_xvirtinfo(addr,buf,bufsize,flags));
}
INTERN struct virtinfo *LIBCCALL
libc_xvirtinfo(VIRT void *addr, USER struct virtinfo *buf,
               size_t bufsize, u32 flags) {
 ssize_t reqsize; bool is_libc_buffer = false;
 if (!buf && bufsize && (is_libc_buffer = true,
      buf = (struct virtinfo *)libc_malloc(bufsize)) == NULL) return NULL;
 reqsize = libc_xvirtinfo2(addr,buf,bufsize,flags);
 if (E_ISERR(reqsize)) {
  if (is_libc_buffer) libc_free(buf);
  SET_ERRNO((errno_t)-reqsize);
  return NULL;
 }
 if ((size_t)reqsize > bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
   do {
    struct virtinfo *new_buf;
    bufsize = (size_t)reqsize;
    new_buf = (struct virtinfo *)libc_realloc(buf,bufsize);
    if unlikely(!new_buf) { libc_free(buf); return NULL; }
    buf = new_buf;
   } while ((reqsize = libc_xvirtinfo2(addr,buf,bufsize,flags),
             E_ISOK(reqsize) && (size_t)reqsize != bufsize));
   if (E_ISERR(reqsize)) { libc_free(buf); SET_ERRNO(-reqsize); return NULL; }
   return buf;
  }
  SET_ERRNO(-ERANGE);
  return NULL;
 }
 return buf;
}


INTERN ssize_t LIBCCALL libc_xfdname2(int fd, int type, char *buf, size_t bufsize) {
 return FORWARD_SYSTEM_ERROR(sys_xfdname(fd,type,buf,bufsize));
}
INTERN char *LIBCCALL libc_xfdname(int fd, int type, char *buf, size_t bufsize) {
 ssize_t reqsize; bool is_libc_buffer = false;
 if (!buf && bufsize && (is_libc_buffer = true,
      buf = (char *)libc_malloc(bufsize)) == NULL) return NULL;
 reqsize = libc_xfdname2(fd,type,buf,bufsize);
 if (E_ISERR(reqsize)) {
  if (is_libc_buffer) libc_free(buf);
  SET_ERRNO((errno_t)-reqsize);
  return NULL;
 }
 if ((size_t)reqsize > bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
   do {
    char *new_buf;
    bufsize = (size_t)reqsize;
    new_buf = (char *)libc_realloc(buf,bufsize);
    if unlikely(!new_buf) { libc_free(buf); return NULL; }
    buf = new_buf;
   } while ((reqsize = sys_xfdname(fd,type,buf,bufsize),
             E_ISOK(reqsize) && (size_t)reqsize != bufsize));
   if (E_ISERR(reqsize)) { libc_free(buf); SET_ERRNO(-reqsize); return NULL; }
   return buf;
  }
  SET_ERRNO(-ERANGE);
  return NULL;
 }
 return buf;
}
INTERN char *LIBCCALL libc_getcwd(char *buf, size_t bufsize) {
#ifndef __OPTIMIZE_SIZE__
 ssize_t reqsize; bool is_libc_buffer = false;
 if (!buf && bufsize && (is_libc_buffer = true,
      buf = (char *)libc_malloc(bufsize)) == NULL) return NULL;
 reqsize = sys_getcwd(buf,bufsize);
 if (E_ISERR(reqsize)) {
  if (is_libc_buffer) libc_free(buf);
  SET_ERRNO((errno_t)-reqsize);
  return NULL;
 }
 if ((size_t)reqsize > bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
   do {
    char *new_buf;
    bufsize = (size_t)reqsize;
    new_buf = (char *)libc_realloc(buf,bufsize);
    if unlikely(!new_buf) { libc_free(buf); return NULL; }
    buf = new_buf;
   } while ((reqsize = sys_getcwd(buf,bufsize),
             E_ISOK(reqsize) && (size_t)reqsize != bufsize));
   if (E_ISERR(reqsize)) { libc_free(buf); SET_ERRNO(-reqsize); return NULL; }
   return buf;
  }
  SET_ERRNO(-ERANGE);
  return NULL;
 }
 return buf;
#else
 return xfdname(AT_FDCWD,FDNAME_PATH,buf,bufsize);
#endif
}
INTERN char *LIBCCALL libc_get_current_dir_name(void) { return libc_getcwd(NULL,0); }
INTERN char *LIBCCALL libc_getwd(char *buf) { return libc_getcwd(buf,(size_t)-1); }
INTERN int LIBCCALL libc_posix_fadvise(int UNUSED(fd), off_t UNUSED(offset), off_t UNUSED(len), int UNUSED(advise)) { return 0; }
INTERN int LIBCCALL libc_posix_fallocate(int UNUSED(fd), off_t UNUSED(offset), off_t UNUSED(len)) { return 0; }

DEFINE_PUBLIC_ALIAS(ioctl,libc_ioctl);
DEFINE_PUBLIC_ALIAS(fcntl,libc_fcntl);
DEFINE_PUBLIC_ALIAS(openat,libc_openat);
DEFINE_PUBLIC_ALIAS(open,libc_open);
DEFINE_PUBLIC_ALIAS(creat,libc_creat);
DEFINE_PUBLIC_ALIAS(getcwd,libc_getcwd);
DEFINE_PUBLIC_ALIAS(get_current_dir_name,libc_get_current_dir_name);
DEFINE_PUBLIC_ALIAS(getwd,libc_getwd);
DEFINE_PUBLIC_ALIAS(posix_fadvise,libc_posix_fadvise);
DEFINE_PUBLIC_ALIAS(posix_fallocate,libc_posix_fallocate);
DEFINE_PUBLIC_ALIAS(open64,libc_open);
DEFINE_PUBLIC_ALIAS(creat64,libc_creat);
DEFINE_PUBLIC_ALIAS(openat64,libc_openat);
DEFINE_PUBLIC_ALIAS(posix_fadvise64,libc_posix_fadvise);
DEFINE_PUBLIC_ALIAS(posix_fallocate64,libc_posix_fallocate);

DEFINE_PUBLIC_ALIAS(xfdname2,libc_xfdname2);
DEFINE_PUBLIC_ALIAS(xfdname,libc_xfdname);
DEFINE_PUBLIC_ALIAS(xvirtinfo2,libc_xvirtinfo2);
DEFINE_PUBLIC_ALIAS(xvirtinfo,libc_xvirtinfo);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
/* Translate DOS flags to UNIX and append O_DOSPATH. */
INTERN ATTR_DOSTEXT oflag_t LIBCCALL
libc_dos_getoflags(oflag_t dos_flags) {
 oflag_t result = dos_flags & __DOS_O_COMMON;
 if (dos_flags&__DOS_O_APPEND)     result |= O_APPEND;
 if (dos_flags&__DOS_O_TEMPORARY)  result |= O_TMPFILE;
 if (dos_flags&__DOS_O_NOINHERIT)  result |= O_CLOEXEC;
 if (dos_flags&__DOS_O_CREAT)      result |= O_CREAT;
 if (dos_flags&__DOS_O_EXCL)       result |= O_EXCL;
 if (dos_flags&__DOS_O_OBTAIN_DIR) result |= O_DIRECTORY;
 return result|O_DOSPATH;
}

INTERN ATTR_DOSTEXT char *LIBCCALL libc_getdcwd(int UNUSED(drive), char *buf, size_t size) { return libc_getcwd(buf,size); }
INTERN ATTR_DOSTEXT char16_t *LIBCCALL libc_16wgetdcwd(int UNUSED(drive), char16_t *dstbuf, int elemcount) { return libc_16wgetcwd(dstbuf,elemcount); }
INTERN ATTR_DOSTEXT char32_t *LIBCCALL libc_32wgetdcwd(int UNUSED(drive), char32_t *dstbuf, int elemcount) { return libc_32wgetcwd(dstbuf,elemcount); }
INTERN ATTR_DOSTEXT char16_t *LIBCCALL libc_16wgetcwd(char16_t *dstbuf, int elemcount) {
 char *result;
 if (!elemcount) { SET_ERRNO(ERANGE); return NULL; }
 result = libc_getcwd((char *)dstbuf,(size_t)elemcount*2);
 if (!result) return NULL;
 return libc_api_utf8to16(dstbuf,(size_t)elemcount,result,libc_strlen(result));
}
INTERN ATTR_DOSTEXT char32_t *LIBCCALL libc_32wgetcwd(char32_t *dstbuf, int elemcount) {
 char *result;
 if (!elemcount) { SET_ERRNO(ERANGE); return NULL; }
 result = libc_getcwd((char *)dstbuf,(size_t)elemcount*2);
 if (!result) return NULL;
 return libc_api_utf8to32(dstbuf,(size_t)elemcount,result,libc_strlen(result));
}
INTERN ATTR_DOSTEXT int ATTR_CDECL libc_dos_open(char const *file, int oflag, ...) {
 va_list args; int result;
 va_start(args,oflag);
 result = sys_openat(AT_FDCWD,file,
                     libc_dos_getoflags(oflag),
                     va_arg(args,mode_t));
 va_end(args);
 if (E_ISERR(result)) {
  SET_ERRNO(-result);
  result = -1;
 }
 return result;
}
INTERN ATTR_DOSTEXT int ATTR_CDECL
libc_dos_sopen(char const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 va_start(args,sflag);
 result = sys_openat(AT_FDCWD,file,
                     libc_dos_getoflags(oflag),
                     va_arg(args,mode_t));
 va_end(args);
 if (E_ISERR(result)) {
  SET_ERRNO(-result);
  result = -1;
 }
 return result;
}
INTERN ATTR_DOSTEXT int LIBCCALL
libc_dos_creat(char const *file, mode_t mode) {
 return libc_creat(file,libc_dos_getoflags(mode));
}
INTERN ATTR_DOSTEXT __errno_t LIBCCALL
libc_dos_sopen_s(int *fd, char const *file,
                 int oflag, int UNUSED(sflag), int pmode) {
 int result;
 if (!fd) return EINVAL;
 result = sys_openat(AT_FDCWD,file,libc_dos_getoflags(oflag),pmode);
 if (E_ISERR(result)) return -result;
 *fd = result;
 return -EOK;
}


PRIVATE int LIBCCALL libc_16wopen_impl(char16_t const *file, int oflag, mode_t cmode) {
 int result = -1; char *utf8file = libc_utf16to8m(file);
 if (utf8file) result = libc_open(utf8file,oflag,cmode),libc_free(utf8file);
 return result;
}
PRIVATE int LIBCCALL libc_32wopen_impl(char32_t const *file, int oflag, mode_t cmode) {
 int result = -1; char *utf8file = libc_utf32to8m(file);
 if (utf8file) result = libc_open(utf8file,oflag,cmode),libc_free(utf8file);
 return result;
}
INTERN int LIBCCALL libc_16wcreat(char16_t const *file, mode_t mode) { return libc_16wopen_impl(file,O_CREAT|O_WRONLY|O_TRUNC,mode); }
INTERN int LIBCCALL libc_32wcreat(char32_t const *file, mode_t mode) { return libc_32wopen_impl(file,O_CREAT|O_WRONLY|O_TRUNC,mode); }
INTERN int LIBCCALL libc_dos_16wcreat(char16_t const *file, mode_t mode) { return libc_16wopen_impl(file,O_DOSPATH|O_CREAT|O_WRONLY|O_TRUNC,mode); }
INTERN int LIBCCALL libc_dos_32wcreat(char32_t const *file, mode_t mode) { return libc_32wopen_impl(file,O_DOSPATH|O_CREAT|O_WRONLY|O_TRUNC,mode); }
INTERN int ATTR_CDECL libc_16wopen(char16_t const *file, int oflag, ...) { int result; va_list args; va_start(args,oflag); result = libc_16wopen_impl(file,oflag,va_arg(args,mode_t)); va_end(args); return result; }
INTERN int ATTR_CDECL libc_32wopen(char32_t const *file, int oflag, ...) { int result; va_list args; va_start(args,oflag); result = libc_32wopen_impl(file,oflag,va_arg(args,mode_t)); va_end(args); return result; }
INTERN int ATTR_CDECL libc_dos_16wopen(char16_t const *file, int oflag, ...) { int result; va_list args; va_start(args,oflag); result = libc_16wopen_impl(file,libc_dos_getoflags(oflag),va_arg(args,mode_t)); va_end(args); return result; }
INTERN int ATTR_CDECL libc_dos_32wopen(char32_t const *file, int oflag, ...) { int result; va_list args; va_start(args,oflag); result = libc_32wopen_impl(file,libc_dos_getoflags(oflag),va_arg(args,mode_t)); va_end(args); return result; }
INTERN int ATTR_CDECL libc_16wsopen(char16_t const *file, int oflag, int sflag, ...) { int result; va_list args; va_start(args,sflag); result = libc_16wopen_impl(file,oflag,va_arg(args,mode_t)); va_end(args); return result; }
INTERN int ATTR_CDECL libc_32wsopen(char32_t const *file, int oflag, int sflag, ...) { int result; va_list args; va_start(args,sflag); result = libc_32wopen_impl(file,oflag,va_arg(args,mode_t)); va_end(args); return result; }
INTERN int ATTR_CDECL libc_dos_16wsopen(char16_t const *file, int oflag, int sflag, ...) { int result; va_list args; va_start(args,sflag); result = libc_16wopen_impl(file,libc_dos_getoflags(oflag),va_arg(args,mode_t)); va_end(args); return result; }
INTERN int ATTR_CDECL libc_dos_32wsopen(char32_t const *file, int oflag, int sflag, ...) { int result; va_list args; va_start(args,sflag); result = libc_32wopen_impl(file,libc_dos_getoflags(oflag),va_arg(args,mode_t)); va_end(args); return result; }
INTERN int LIBCCALL libc_16wsopen_s(int *fd, char16_t const *file, int oflag, int UNUSED(sflag), mode_t cmode) { if (!fd) { SET_ERRNO(EINVAL); return -1; } return (*fd = libc_16wopen_impl(file,oflag,cmode)) >= 0; }
INTERN int LIBCCALL libc_32wsopen_s(int *fd, char32_t const *file, int oflag, int UNUSED(sflag), mode_t cmode) { if (!fd) { SET_ERRNO(EINVAL); return -1; } return (*fd = libc_32wopen_impl(file,oflag,cmode)) >= 0; }
INTERN int LIBCCALL libc_dos_16wsopen_s(int *fd, char16_t const *file, int oflag, int UNUSED(sflag), mode_t cmode) { if (!fd) { SET_ERRNO(EINVAL); return -1; } return (*fd = libc_16wopen_impl(file,libc_dos_getoflags(oflag),cmode)) >= 0; }
INTERN int LIBCCALL libc_dos_32wsopen_s(int *fd, char32_t const *file, int oflag, int UNUSED(sflag), mode_t cmode) { if (!fd) { SET_ERRNO(EINVAL); return -1; } return (*fd = libc_32wopen_impl(file,libc_dos_getoflags(oflag),cmode)) >= 0; }

DEFINE_PUBLIC_ALIAS(__KSYMw16(_wcreat),libc_16wcreat);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wopen),libc_16wopen);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wsopen),libc_16wsopen);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wsopen_s),libc_16wsopen_s);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wcreat),libc_32wcreat);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wopen),libc_32wopen);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wsopen),libc_32wsopen);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wsopen_s),libc_32wsopen_s);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wcreat),libc_dos_16wcreat);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wopen),libc_dos_16wopen);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wsopen),libc_dos_16wsopen);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wsopen_s),libc_dos_16wsopen_s);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wcreat),libc_dos_32wcreat);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wopen),libc_dos_32wopen);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wsopen),libc_dos_32wsopen);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wsopen_s),libc_dos_32wsopen_s);

DEFINE_PUBLIC_ALIAS(_getcwd,libc_getcwd);
DEFINE_PUBLIC_ALIAS(_getdcwd,libc_getdcwd);
DEFINE_PUBLIC_ALIAS(_wgetcwd,libc_32wgetcwd);
DEFINE_PUBLIC_ALIAS(_wgetdcwd,libc_32wgetdcwd);
DEFINE_PUBLIC_ALIAS(__DSYM(_wgetdcwd),libc_16wgetdcwd);
DEFINE_PUBLIC_ALIAS(__DSYM(_wgetcwd),libc_16wgetcwd);
DEFINE_PUBLIC_ALIAS(_open,libc_dos_open);
DEFINE_PUBLIC_ALIAS(_creat,libc_dos_creat);
DEFINE_PUBLIC_ALIAS(_sopen,libc_dos_sopen);
DEFINE_PUBLIC_ALIAS(_sopen_s,libc_dos_sopen_s);
DEFINE_PUBLIC_ALIAS(_sopen_s_nolock,libc_dos_sopen_s);

DEFINE_PUBLIC_ALIAS("?_open%%YAHPEBDHH%Z",libc_dos_open);
DEFINE_PUBLIC_ALIAS("?_sopen%%YAHPBDHHH%Z",libc_dos_sopen);
DEFINE_PUBLIC_ALIAS("?_wopen%%YAHPEB_WHH%Z",libc_dos_16wopen);
DEFINE_PUBLIC_ALIAS("?_wsopen%%YAHPEB_WHHH%Z",libc_dos_16wsopen);


#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


DECL_END

#endif /* !GUARD_LIBS_LIBC_FCNTL_C */
