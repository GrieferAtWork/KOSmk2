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

#include "system.h"

#include <errno.h>
#include <fcntl.h>
#include <hybrid/compiler.h>
#include <kos/fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/ioctl.h>

DECL_BEGIN

PUBLIC int (ATTR_CDECL ioctl)(int fd, unsigned long int request, ...) {
 va_list args; int result;
 va_start(args,request);
 result = sys_ioctl(fd,request,va_arg(args,void *));
 va_end(args);
 if (E_ISERR(result)) {
  __set_errno(-result);
  result = -1;
 }
 return result;
}
PUBLIC int (ATTR_CDECL fcntl)(int fd, int cmd, ...) {
 va_list args; int result;
 va_start(args,cmd);
 result = sys_fcntl(fd,cmd,va_arg(args,void *));
 va_end(args);
 if (E_ISERR(result)) {
  __set_errno(-result);
  result = -1;
 }
 return result;
}

PUBLIC int (ATTR_CDECL openat)(int fd, char const *file, int oflag, ...) {
 va_list args; int result;
 va_start(args,oflag);
 result = sys_openat(fd,file,oflag,va_arg(args,mode_t));
 va_end(args);
 if (E_ISERR(result)) {
  __set_errno(-result);
  result = -1;
 }
 return result;
}
PUBLIC int (ATTR_CDECL open)(char const *file, int oflag, ...) {
 va_list args; int result;
 va_start(args,oflag);
 result = sys_openat(AT_FDCWD,file,oflag,va_arg(args,mode_t));
 va_end(args);
 if (E_ISERR(result)) {
  __set_errno(-result);
  result = -1;
 }
 return result;
}
PUBLIC int (LIBCCALL creat)(char const *file, mode_t mode) {
 return open(file,O_CREAT|O_WRONLY|O_TRUNC,mode);
}

PUBLIC ssize_t (LIBCCALL xfdname2)(int fd, int type, char *buf, size_t bufsize) {
 return FORWARD_SYSTEM_ERROR(sys_xfdname(fd,type,buf,bufsize));
}
PUBLIC char *(LIBCCALL xfdname)(int fd, int type, char *buf, size_t bufsize) {
 ssize_t reqsize;
 if (!buf && bufsize && (buf = (char *)(malloc)(bufsize)) == NULL) return NULL;
 reqsize = xfdname2(fd,type,buf,bufsize);
 if (E_ISERR(reqsize)) { __set_errno((errno_t)-reqsize); return NULL; }
 if ((size_t)reqsize > bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
   do {
    char *new_buf;
    bufsize = (size_t)reqsize;
    new_buf = (char *)(realloc)(buf,bufsize);
    if unlikely(!new_buf) { (free)(buf); return NULL; }
    buf = new_buf;
   } while ((reqsize = sys_xfdname(fd,type,buf,bufsize),
             E_ISOK(reqsize) && (size_t)reqsize != bufsize));
   if (E_ISERR(reqsize)) { (free)(buf); __set_errno(-reqsize); return NULL; }
   return buf;
  }
  __set_errno(-ERANGE);
  return NULL;
 }
 return buf;
}
PUBLIC char *(LIBCCALL getcwd)(char *buf, size_t bufsize) {
#ifndef __OPTIMIZE_SIZE__
 ssize_t reqsize;
 if (!buf && bufsize && (buf = (char *)(malloc)(bufsize)) == NULL) return NULL;
 reqsize = sys_getcwd(buf,bufsize);
 if (E_ISERR(reqsize)) { __set_errno((errno_t)-reqsize); return NULL; }
 if ((size_t)reqsize > bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
   do {
    char *new_buf;
    bufsize = (size_t)reqsize;
    new_buf = (char *)(realloc)(buf,bufsize);
    if unlikely(!new_buf) { (free)(buf); return NULL; }
    buf = new_buf;
   } while ((reqsize = sys_getcwd(buf,bufsize),
             E_ISOK(reqsize) && (size_t)reqsize != bufsize));
   if (E_ISERR(reqsize)) { (free)(buf); __set_errno(-reqsize); return NULL; }
   return buf;
  }
  __set_errno(-ERANGE);
  return NULL;
 }
 return buf;
#else
 return xfdname(AT_FDCWD,FDNAME_PATH,buf,bufsize);
#endif
}
PUBLIC char *(LIBCCALL get_current_dir_name)(void) { return getcwd(NULL,0); }
PUBLIC char *(LIBCCALL getwd)(char *buf) { return getcwd(buf,(size_t)-1); }

PUBLIC int (LIBCCALL posix_fadvise)(int UNUSED(fd), off_t UNUSED(offset), off_t UNUSED(len), int UNUSED(advise)) { return 0; }
PUBLIC int (LIBCCALL posix_fallocate)(int UNUSED(fd), off_t UNUSED(offset), off_t UNUSED(len)) { return 0; }

DEFINE_PUBLIC_ALIAS(open64,open);
DEFINE_PUBLIC_ALIAS(creat64,creat);
DEFINE_PUBLIC_ALIAS(openat64,openat);
DEFINE_PUBLIC_ALIAS(posix_fadvise64,posix_fadvise);
DEFINE_PUBLIC_ALIAS(posix_fallocate64,posix_fallocate);

DECL_END

#endif /* !GUARD_LIBS_LIBC_FCNTL_C */
