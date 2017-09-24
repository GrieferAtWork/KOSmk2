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

INTERN int ATTR_CDECL libc_ioctl(int fd, unsigned long int request, ...) {
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
INTERN int ATTR_CDECL libc_fcntl(int fd, int cmd, ...) {
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
INTERN ssize_t LIBCCALL libc_xfdname2(int fd, int type, char *buf, size_t bufsize) {
 return FORWARD_SYSTEM_ERROR(sys_xfdname(fd,type,buf,bufsize));
}
INTERN char *LIBCCALL libc_xfdname(int fd, int type, char *buf, size_t bufsize) {
 ssize_t reqsize;
 if (!buf && bufsize && (buf = (char *)libc_malloc(bufsize)) == NULL) return NULL;
 reqsize = libc_xfdname2(fd,type,buf,bufsize);
 if (E_ISERR(reqsize)) { SET_ERRNO((errno_t)-reqsize); return NULL; }
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
 ssize_t reqsize;
 if (!buf && bufsize && (buf = (char *)libc_malloc(bufsize)) == NULL) return NULL;
 reqsize = sys_getcwd(buf,bufsize);
 if (E_ISERR(reqsize)) { SET_ERRNO((errno_t)-reqsize); return NULL; }
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
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN char *LIBCCALL libc_getdcwd(int UNUSED(drive), char *buf, size_t size) { return libc_getcwd(buf,size); }
INTERN char16_t *LIBCCALL libc_16getdcwd(int UNUSED(drive), char16_t *dstbuf, int elemcount) { return libc_16getcwd(dstbuf,elemcount); }
INTERN char32_t *LIBCCALL libc_32getdcwd(int UNUSED(drive), char32_t *dstbuf, int elemcount) { return libc_32getcwd(dstbuf,elemcount); }
INTERN char16_t *LIBCCALL libc_16getcwd(char16_t *dstbuf, int elemcount) {
 char *result;
 if (!elemcount) { SET_ERRNO(ERANGE); return NULL; }
 result = libc_getcwd((char *)dstbuf,(size_t)elemcount*2);
 if (!result) return NULL;
 return libc_api_utf8to16(dstbuf,(size_t)elemcount,result,libc_strlen(result));
}
INTERN char32_t *LIBCCALL libc_32getcwd(char32_t *dstbuf, int elemcount) {
 char *result;
 if (!elemcount) { SET_ERRNO(ERANGE); return NULL; }
 result = libc_getcwd((char *)dstbuf,(size_t)elemcount*2);
 if (!result) return NULL;
 return libc_api_utf8to32(dstbuf,(size_t)elemcount,result,libc_strlen(result));
}
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DEFINE_PUBLIC_ALIAS(ioctl,libc_ioctl);
DEFINE_PUBLIC_ALIAS(fcntl,libc_fcntl);
DEFINE_PUBLIC_ALIAS(openat,libc_openat);
DEFINE_PUBLIC_ALIAS(open,libc_open);
DEFINE_PUBLIC_ALIAS(creat,libc_creat);
DEFINE_PUBLIC_ALIAS(xfdname2,libc_xfdname2);
DEFINE_PUBLIC_ALIAS(xfdname,libc_xfdname);
DEFINE_PUBLIC_ALIAS(getcwd,libc_getcwd);
DEFINE_PUBLIC_ALIAS(get_current_dir_name,libc_get_current_dir_name);
DEFINE_PUBLIC_ALIAS(getwd,libc_getwd);
#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(_getdcwd,libc_getdcwd);
DEFINE_PUBLIC_ALIAS(getdcwd,libc_getdcwd);
DEFINE_PUBLIC_ALIAS(wgetdcwd,libc_32getdcwd);
DEFINE_PUBLIC_ALIAS(wgetcwd,libc_32getcwd);
DEFINE_PUBLIC_ALIAS(__DSYM(_wgetdcwd),libc_16getdcwd);
DEFINE_PUBLIC_ALIAS(__DSYM(_wgetcwd),libc_16getcwd);
DEFINE_PUBLIC_ALIAS(__DSYM(_wchdir),libc_16wchdir);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
DEFINE_PUBLIC_ALIAS(posix_fadvise,libc_posix_fadvise);
DEFINE_PUBLIC_ALIAS(posix_fallocate,libc_posix_fallocate);
DEFINE_PUBLIC_ALIAS(open64,libc_open);
DEFINE_PUBLIC_ALIAS(creat64,libc_creat);
DEFINE_PUBLIC_ALIAS(openat64,libc_openat);
DEFINE_PUBLIC_ALIAS(posix_fadvise64,libc_posix_fadvise);
DEFINE_PUBLIC_ALIAS(posix_fallocate64,libc_posix_fallocate);

DECL_END

#endif /* !GUARD_LIBS_LIBC_FCNTL_C */
