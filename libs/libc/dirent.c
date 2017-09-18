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
#ifndef GUARD_LIBS_LIBC_DIRENT_C
#define GUARD_LIBS_LIBC_DIRENT_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include "libc.h"
#include "system.h"
#include "dirent.h"
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <hybrid/compiler.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>

DECL_BEGIN

INTERN ssize_t LIBCCALL
libc_xreaddir(int fd, struct dirent *buf, size_t bufsize, int mode) {
 ssize_t result = sys_xreaddir(fd,buf,bufsize,mode);
 //sys_xpaused("PAUSE:" __PP_STR(__LINE__));
 if (E_ISERR(result)) { SET_ERRNO(-result); return -1; }
 return result;
}

INTERN DIR *LIBCCALL libc_fdopendir(int fd) {
 DIR *result = omalloc(DIR);
 if (result) {
  result->d_fd    = fd;
  result->d_buf   = (struct dirent *)result->d_inl;
  result->d_bufsz = sizeof(result->d_inl);
 }
 return result;
}
INTERN DIR *LIBCCALL libc_opendirat(int dfd, char const *name) {
 DIR *result;
 int fd = openat(dfd,name,O_RDONLY|O_DIRECTORY);
 if (fd < 0) return NULL;
 result = libc_fdopendir(fd);
 if unlikely(!result) sys_close(fd);
 return result;
}
INTERN DIR *LIBCCALL libc_opendir(char const *name) {
 return libc_opendirat(AT_FDCWD,name);
}
INTERN int LIBCCALL libc_closedir(DIR *dirp) {
 if unlikely(!dirp) { SET_ERRNO(EBADF); return -1; }
 /* Free an extended directory entry buffer. */
 if (dirp->d_buf != (struct dirent *)dirp->d_inl) free(dirp->d_buf);
 sys_close(dirp->d_fd);
 free(dirp);
 return 0;
}
INTERN struct dirent *LIBCCALL libc_readdir(DIR *dirp) {
 ssize_t error; struct dirent *result;
 if unlikely(!dirp) { SET_ERRNO(EBADF); return NULL; }
read_again:
 error = libc_xreaddir(dirp->d_fd,(result = dirp->d_buf),
                       dirp->d_bufsz,READDIR_DEFAULT);
#if 0
 syslog(LOG_CONFIRM,"xreaddir(%d,%p,%Iu,%d) -> %Id\n",
        dirp->d_fd,result,dirp->d_bufsz,
        READDIR_DEFAULT,error);
 sys_xpaused("PAUSE");
#endif
 if (error <= 0) return NULL; /* Error, or end-of-directory. */
 if unlikely((size_t)error > dirp->d_bufsz) {
  /* Must allocate more buffer memory. */
  if (dirp->d_buf == (struct dirent *)dirp->d_inl) {
   /* Try to release the inline-allocated buffer. */
   realloc_in_place(dirp,offsetof(DIR,d_inl));
   result = (struct dirent *)malloc((size_t)error);
   if unlikely(!result) {
    /* Prevent another attempt from calling 'realloc_in_place()' again. */
    dirp->d_buf   = NULL;
    dirp->d_bufsz = 0;
    goto end;
   }
  } else {
   result = (struct dirent *)realloc(dirp->d_buf,(size_t)error);
   if unlikely(!result) goto end;
  }
  /* Update the buffer  */
  dirp->d_buf   = result;
  dirp->d_bufsz = (size_t)error;
  /* Read the directory entry again.
   * HINT: Because 'READDIR_DEFAULT' was used, this second
   *       read-attempt will try to load the same data! */
  goto read_again;
 }
end:
 return result;
}
INTERN void LIBCCALL libc_rewinddir(DIR *dirp) {
 if (dirp) sys_lseek(dirp->d_fd,(off64_t)0,SEEK_SET);
}
INTERN int LIBCCALL libc_readdir_r(DIR *__restrict dirp,
                                   struct dirent *__restrict entry,
                                   struct dirent **__restrict result) {
 ssize_t error;
 error = libc_xreaddir(dirp->d_fd,entry,
                       offsetof(struct dirent,d_name)+
                      (256+1)*sizeof(char),READDIR_CONTINUE);
 *result = error ? entry : NULL;
 return error < 0 ? error : 0;

}
INTERN void LIBCCALL libc_seekdir(DIR *dirp, long int pos) {
 if (dirp) sys_lseek(dirp->d_fd,(off64_t)pos,SEEK_SET);
}
INTERN long int LIBCCALL libc_telldir(DIR *dirp) {
 if unlikely(!dirp) { SET_ERRNO(EBADF); return -1; }
 return (long int)lseek(dirp->d_fd,0,SEEK_CUR);
}
INTERN int LIBCCALL libc_dirfd(DIR *dirp) {
 if unlikely(!dirp) { SET_ERRNO(EINVAL); return -1; }
 return dirp->d_fd;
}


INTERN int LIBCCALL
libc_scandir(char const *__restrict dir, struct dirent ***__restrict namelist,
             int (*selector)(struct dirent const *),
             int (*cmp)(struct dirent const **, struct dirent const **)) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
INTERN int LIBCCALL
libc_alphasort(struct dirent const **e1, struct dirent const **e2) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
INTERN int LIBCCALL
libc_scandirat(int dfd, char const *__restrict dir, struct dirent ***__restrict namelist,
               int (*selector)(struct dirent const *),
               int (*cmp)(struct dirent const **, struct dirent const **)) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
INTERN ssize_t LIBCCALL
libc_getdirentries(int fd, char *__restrict buf,
                   size_t nbytes, off_t *__restrict basep) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
INTERN int LIBCCALL
libc_versionsort(struct dirent const **e1, struct dirent const **e2) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}

DEFINE_PUBLIC_ALIAS(xreaddir,libc_xreaddir);
DEFINE_PUBLIC_ALIAS(fdopendir,libc_fdopendir);
DEFINE_PUBLIC_ALIAS(opendirat,libc_opendirat);
DEFINE_PUBLIC_ALIAS(opendir,libc_opendir);
DEFINE_PUBLIC_ALIAS(closedir,libc_closedir);
DEFINE_PUBLIC_ALIAS(readdir,libc_readdir);
DEFINE_PUBLIC_ALIAS(rewinddir,libc_rewinddir);
DEFINE_PUBLIC_ALIAS(readdir_r,libc_readdir_r);
DEFINE_PUBLIC_ALIAS(seekdir,libc_seekdir);
DEFINE_PUBLIC_ALIAS(telldir,libc_telldir);
DEFINE_PUBLIC_ALIAS(dirfd,libc_dirfd);
DEFINE_PUBLIC_ALIAS(scandir,libc_scandir);
DEFINE_PUBLIC_ALIAS(alphasort,libc_alphasort);
DEFINE_PUBLIC_ALIAS(scandirat,libc_scandirat);
DEFINE_PUBLIC_ALIAS(getdirentries,libc_getdirentries);
DEFINE_PUBLIC_ALIAS(versionsort,libc_versionsort);
DEFINE_PUBLIC_ALIAS(readdir64,libc_readdir);
DEFINE_PUBLIC_ALIAS(alphasort64,libc_alphasort);
DEFINE_PUBLIC_ALIAS(readdir64_r,libc_readdir_r);
DEFINE_PUBLIC_ALIAS(scandir64,libc_scandir);
DEFINE_PUBLIC_ALIAS(scandirat64,libc_scandirat);
DEFINE_PUBLIC_ALIAS(getdirentries64,libc_getdirentries);
DEFINE_PUBLIC_ALIAS(versionsort64,libc_versionsort);

DECL_END

#endif /* !GUARD_LIBS_LIBC_DIRENT_C */
