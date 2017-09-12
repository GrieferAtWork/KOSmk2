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
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <hybrid/compiler.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>

DECL_BEGIN

DEFINE_PUBLIC_ALIAS(readdir64,readdir);
DEFINE_PUBLIC_ALIAS(alphasort64,alphasort);
DEFINE_PUBLIC_ALIAS(readdir64_r,readdir_r);
DEFINE_PUBLIC_ALIAS(scandir64,scandir);
DEFINE_PUBLIC_ALIAS(scandirat64,scandirat);
DEFINE_PUBLIC_ALIAS(getdirentries64,getdirentries);
DEFINE_PUBLIC_ALIAS(versionsort64,versionsort);


struct __dirstream {
 int            d_fd;
 struct dirent *d_buf;   /*< [0..1][owned] Allocated directory entry buffer. */
 size_t         d_bufsz; /*< Allocated buffer size for 'd_buf' */
 /* Inline-allocated directory entry buffer (Used as initial buffer). */
 char           d_inl[offsetof(struct dirent,d_name)+(256+1)*sizeof(char)];
};

PUBLIC ssize_t (LIBCCALL xreaddir)(int fd, struct dirent *buf,
                                   size_t bufsize, int mode) {
 ssize_t result = sys_xreaddir(fd,buf,bufsize,mode);
 //sys_xpaused("PAUSE:" __PP_STR(__LINE__));
 if (E_ISERR(result)) { __set_errno(-result); return -1; }
 return result;
}

PUBLIC DIR *(LIBCCALL fdopendir)(int fd) {
 DIR *result = omalloc(DIR);
 if (result) {
  result->d_fd    = fd;
  result->d_buf   = (struct dirent *)result->d_inl;
  result->d_bufsz = sizeof(result->d_inl);
 }
 return result;
}
PUBLIC DIR *(LIBCCALL opendirat)(int dfd, char const *name) {
 DIR *result;
 int fd = openat(dfd,name,O_RDONLY|O_DIRECTORY);
 if (fd < 0) return NULL;
 result = fdopendir(fd);
 if unlikely(!result) sys_close(fd);
 return result;
}
PUBLIC DIR *(LIBCCALL opendir)(char const *name) {
 return opendirat(AT_FDCWD,name);
}
PUBLIC int (LIBCCALL closedir)(DIR *dirp) {
 if unlikely(!dirp) { __set_errno(EBADF); return -1; }
 /* Free an extended directory entry buffer. */
 if (dirp->d_buf != (struct dirent *)dirp->d_inl) free(dirp->d_buf);
 sys_close(dirp->d_fd);
 free(dirp);
 return 0;
}
PUBLIC struct dirent *(LIBCCALL readdir)(DIR *dirp) {
 ssize_t error; struct dirent *result;
 if unlikely(!dirp) { __set_errno(EBADF); return NULL; }
read_again:
 error = xreaddir(dirp->d_fd,(result = dirp->d_buf),
                  dirp->d_bufsz,READDIR_DEFAULT);
 //syslog(LOG_CONFIRM,"xreaddir(%d,%p,%Iu,%d) -> %Id\n",
 //        dirp->d_fd,result,dirp->d_bufsz,
 //        READDIR_DEFAULT,error);
 //sys_xpaused("PAUSE");
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
PUBLIC void (LIBCCALL rewinddir)(DIR *dirp) {
 if (dirp) sys_lseek(dirp->d_fd,(off64_t)0,SEEK_SET);
}
PUBLIC int (LIBCCALL readdir_r)(DIR *__restrict dirp,
                                struct dirent *__restrict entry,
                                struct dirent **__restrict result) {
 ssize_t error;
 error = xreaddir(dirp->d_fd,entry,
                  offsetof(struct dirent,d_name)+
                 (256+1)*sizeof(char),READDIR_CONTINUE);
 *result = error ? entry : NULL;
 return error < 0 ? error : 0;

}
PUBLIC void (LIBCCALL seekdir)(DIR *dirp, long int pos) {
 if (dirp) sys_lseek(dirp->d_fd,(off64_t)pos,SEEK_SET);
}
PUBLIC long int (LIBCCALL telldir)(DIR *dirp) {
 if unlikely(!dirp) { __set_errno(EBADF); return -1; }
 return (long int)lseek(dirp->d_fd,0,SEEK_CUR);
}
PUBLIC int (LIBCCALL dirfd)(DIR *dirp) {
 if unlikely(!dirp) { __set_errno(EINVAL); return -1; }
 return dirp->d_fd;
}


PUBLIC int (LIBCCALL scandir)
      (char const *__restrict dir, struct dirent ***__restrict namelist,
       int (*selector) (struct dirent const *),
       int (*cmp) (struct dirent const **, struct dirent const **)) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC int (LIBCCALL alphasort)(struct dirent const **__e1, struct dirent const **__e2) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC int (LIBCCALL scandirat)
      (int __dfd, char const *__restrict dir, struct dirent ***__restrict namelist,
       int (*selector) (struct dirent const *),
       int (*cmp) (struct dirent const **, struct dirent const **)) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC ssize_t (LIBCCALL getdirentries)
      (int fd, char *__restrict buf, size_t __nbytes, __FS_TYPE(off) *__restrict __basep) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC int (LIBCCALL versionsort)
      (struct dirent const **__e1, struct dirent const **__e2) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_DIRENT_C */
