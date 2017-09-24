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
#include "fcntl.h"
#include "malloc.h"
#include "string.h"
#include "unistd.h"
#include "stdlib.h"

#include <fcntl.h>
#include <assert.h>
#include <dirent.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include <io.h>
#include "unicode.h"
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_BEGIN

INTERN ssize_t LIBCCALL
libc_xreaddir(int fd, struct dirent *buf, size_t bufsize, int mode) {
 ssize_t result = sys_xreaddir(fd,buf,bufsize,mode);
 //sys_xpaused("PAUSE:" __PP_STR(__LINE__));
 if (E_ISERR(result)) { SET_ERRNO(-result); return -1; }
 return result;
}

INTERN DIR *LIBCCALL libc_fdopendir(int fd) {
 DIR *result = (DIR *)libc_malloc(sizeof(DIR));
 if (result) {
  result->d_fd    = fd;
  result->d_buf   = (struct dirent *)result->d_inl;
  result->d_bufsz = sizeof(result->d_inl);
 }
 return result;
}
INTERN DIR *LIBCCALL libc_opendirat(int dfd, char const *name) {
 DIR *result;
 int fd = libc_openat(dfd,name,O_RDONLY|O_DIRECTORY);
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
 if (dirp->d_buf != (struct dirent *)dirp->d_inl)
     libc_free(dirp->d_buf);
 sys_close(dirp->d_fd);
 libc_free(dirp);
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
   libc_realloc_in_place(dirp,offsetof(DIR,d_inl));
   result = (struct dirent *)libc_malloc((size_t)error);
   if unlikely(!result) {
    /* Prevent another attempt from calling 'realloc_in_place()' again. */
    dirp->d_buf   = NULL;
    dirp->d_bufsz = 0;
    goto end;
   }
  } else {
   result = (struct dirent *)libc_realloc(dirp->d_buf,(size_t)error);
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
 return (long int)libc_lseek(dirp->d_fd,0,SEEK_CUR);
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


#ifndef CONFIG_LIBC_NO_DOS_LIBC
/* This one's actually kind-of useful: It's a dir-stream with the
 * added functionality of performing a Wildcard match against filenames.
 * The downside is, that the we're bound to NT's finddata structures... */
struct findfd {
 struct __dirstream f_dir;     /*< Underlying  */
 char              *f_wcard;   /*< [1..1][owned][const] Wildcard used to match filenames. */
 bool               f_dosmode; /*< [const] When true, use DOS-mode. */
};

#define FINDFD_INVALID ((struct findfd *)-1)

#define FFD(hnd) ((struct findfd *)(hnd))
INTERN ATTR_DOSTEXT struct findfd *LIBCCALL
libc_findopen(char const *query, bool dosmode) {
 struct findfd *result; int fd;
 char const *query_end,*query_end2;
 result = (struct findfd *)libc_malloc(sizeof(struct findfd));
 if (result) {
  query_end = libc_strrchr(query,'\\');
  query_end2 = libc_strrchr(query,'/');
  if (query_end2 > query_end) query_end = query_end2;
  if (!query_end) fd = libc_openat(AT_FDCWD,NULL,O_RDONLY|O_DIRECTORY);
  else {
   char *path; size_t pathlen;
   pathlen = (size_t)(query_end-query);
   if (pathlen > 1) --pathlen; /* Remove trailing slash if not root path. */
   pathlen = (pathlen+1)*sizeof(char);
   if (pathlen > __AMALLOC_MAX) {
    path = (char *)libc_malloc(pathlen);
    if unlikely(!path) goto err;
    libc_memcpy(path,query,pathlen);
    path[pathlen] = '\0';
   } else {
    path = (char *)alloca(pathlen);
    libc_memcpy(path,query,pathlen);
    path[pathlen] = '\0';
   }
   /* Open the query path. */
   fd = libc_openat(AT_FDCWD,path,O_DOSPATH|O_RDONLY|O_DIRECTORY);
   if (pathlen > __AMALLOC_MAX)
       libc_free(path);
  }
  if (fd == -1) goto err;
  result->f_dir.d_fd    = fd;
  result->f_dir.d_buf   = (struct dirent *)result->f_dir.d_inl;
  result->f_dir.d_bufsz = sizeof(result->f_dir.d_inl);
  result->f_dosmode     = dosmode;
 }
 return result;
err:
 libc_free(result);
 return FINDFD_INVALID;
}
INTERN ATTR_DOSTEXT struct findfd *LIBCCALL
libc_16findopen(char16_t const *query, bool dosmode) {
 char *utf8_query = libc_utf16to8m(query,libc_16wcslen(query));
 struct findfd *result = NULL;
 if (utf8_query) {
  result = libc_findopen(utf8_query,dosmode);
  libc_free(utf8_query);
 }
 return result;
}
INTERN ATTR_DOSTEXT struct findfd *LIBCCALL
libc_32findopen(char32_t const *query, bool dosmode) {
 char *utf8_query = libc_utf32to8m(query,libc_32wcslen(query));
 struct findfd *result = NULL;
 if (utf8_query) {
  result = libc_findopen(utf8_query,dosmode);
  libc_free(utf8_query);
 }
 return result;
}

INTERN ATTR_DOSTEXT int LIBCCALL
libc_findclose(intptr_t hdn) {
 if (hdn == (intptr_t)FINDFD_INVALID)
     return -1;
 libc_free(FFD(hdn)->f_wcard);
 return libc_closedir(&FFD(hdn)->f_dir);
}


PRIVATE ATTR_DOSTEXT struct dirent *LIBCCALL
libc_findread(struct findfd *__restrict fd,
              struct stat64 *__restrict st) {
 struct dirent *result;
 if unlikely(fd == FINDFD_INVALID) return NULL;
 do {
  SET_ERRNO(ENOENT);
  /* Read a directory entry. */
  result = libc_readdir(&fd->f_dir);
  if unlikely(!result) return NULL;
 }
#if 1
 while (0);
#else /* TODO: Wildcard string matching. */
 while (fd->f_dosmode ? libc_wcasematch(fd->f_wcard,result->d_name/*,result->d_namlen*/)
                      : libc_wmatch(fd->f_wcard,result->d_name/*,result->d_namlen*/));
#endif

 /* Stat the read file. */
 if (libc_kfstatat64(fd->f_dir.d_fd,result->d_name,st,
                     AT_SYMLINK_NOFOLLOW))
     libc_memset(st,0,sizeof(struct stat64));
 return result;
}

#define FINDFIRST(open,base,dosmode) \
{ \
 struct findfd *result = open(file,dosmode); \
 if unlikely(result != FINDFD_INVALID) { \
  if (base((intptr_t)result,finddata) != 0) \
      libc_findclose((intptr_t)result); \
 } \
 return (intptr_t)result; \
}
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_findfirst32(char const *file, struct _finddata32_t *finddata) FINDFIRST(libc_findopen,libc_findnext32,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_findfirst64(char const *file, struct __finddata64_t *finddata) FINDFIRST(libc_findopen,libc_findnext64,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_findfirst32i64(char const *file, struct _finddata32i64_t *finddata) FINDFIRST(libc_findopen,libc_findnext32i64,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_findfirst64i32(char const *file, struct _finddata64i32_t *finddata) FINDFIRST(libc_findopen,libc_findnext64i32,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_16findfirst32(char16_t const *file, struct _wfinddata32_t *finddata) FINDFIRST(libc_16findopen,libc_16findnext32,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_16findfirst64(char16_t const *file, struct _wfinddata64_t *finddata) FINDFIRST(libc_16findopen,libc_16findnext64,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_16findfirst32i64(char16_t const *file, struct _wfinddata32i64_t *finddata) FINDFIRST(libc_16findopen,libc_16findnext32i64,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_16findfirst64i32(char16_t const *file, struct _wfinddata64i32_t *finddata) FINDFIRST(libc_16findopen,libc_16findnext64i32,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_32findfirst32(char32_t const *file, struct _wfinddata32_t *finddata) FINDFIRST(libc_32findopen,libc_32findnext32,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_32findfirst64(char32_t const *file, struct _wfinddata64_t *finddata) FINDFIRST(libc_32findopen,libc_32findnext64,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_32findfirst32i64(char32_t const *file, struct _wfinddata32i64_t *finddata) FINDFIRST(libc_32findopen,libc_32findnext32i64,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_32findfirst64i32(char32_t const *file, struct _wfinddata64i32_t *finddata) FINDFIRST(libc_32findopen,libc_32findnext64i32,false)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_findfirst32(char const *file, struct _wfinddata32_t *finddata) FINDFIRST(libc_findopen,libc_16findnext32,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_findfirst64(char const *file, struct _wfinddata64_t *finddata) FINDFIRST(libc_findopen,libc_16findnext64,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_findfirst32i64(char const *file, struct _wfinddata32i64_t *finddata) FINDFIRST(libc_findopen,libc_16findnext32i64,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_findfirst64i32(char const *file, struct _wfinddata64i32_t *finddata) FINDFIRST(libc_findopen,libc_16findnext64i32,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_16findfirst32(char16_t const *file, struct _wfinddata32_t *finddata) FINDFIRST(libc_16findopen,libc_16findnext32,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_16findfirst64(char16_t const *file, struct _wfinddata64_t *finddata) FINDFIRST(libc_16findopen,libc_16findnext64,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_16findfirst32i64(char16_t const *file, struct _wfinddata32i64_t *finddata) FINDFIRST(libc_16findopen,libc_16findnext32i64,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_16findfirst64i32(char16_t const *file, struct _wfinddata64i32_t *finddata) FINDFIRST(libc_16findopen,libc_16findnext64i32,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_32findfirst32(char32_t const *file, struct _wfinddata32_t *finddata) FINDFIRST(libc_32findopen,libc_32findnext32,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_32findfirst64(char32_t const *file, struct _wfinddata64_t *finddata) FINDFIRST(libc_32findopen,libc_32findnext64,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_32findfirst32i64(char32_t const *file, struct _wfinddata32i64_t *finddata) FINDFIRST(libc_32findopen,libc_32findnext32i64,true)
INTERN ATTR_DOSTEXT intptr_t LIBCCALL libc_dos_32findfirst64i32(char32_t const *file, struct _wfinddata64i32_t *finddata) FINDFIRST(libc_32findopen,libc_32findnext64i32,true)
#undef FINDFIRST


#define GET_ATTRIB(ent,st) ((ent)->d_type == DT_DIR ? _A_SUBDIR : _A_NORMAL)

#define FINDNEXT(tm_suffix,sz_suffix,load_name) \
{ \
 struct stat64 st; struct dirent *ent; \
 ent = libc_findread(FFD(hdn),&st); \
 if (!ent) return -1; \
 finddata->attrib      = GET_ATTRIB(ent,&st); \
 finddata->time_create = st.st_ctime##tm_suffix; \
 finddata->time_access = st.st_atime##tm_suffix; \
 finddata->time_write  = st.st_mtime##tm_suffix; \
 finddata->size        = st.st_size##sz_suffix; \
 load_name(finddata->name,COMPILER_LENOF(finddata->name), \
           ent->d_name,ent->d_namlen); \
 return 0; \
}
#define MEMCPY_NAME(dst,dst_elem,src,src_len) \
   libc_memcpy(dst,src,MIN(dst_elem,(src_len)+1)*sizeof(char))
INTERN ATTR_DOSTEXT int LIBCCALL libc_findnext32(intptr_t hdn, struct _finddata32_t *finddata) FINDNEXT(32,32,MEMCPY_NAME)
INTERN ATTR_DOSTEXT int LIBCCALL libc_findnext64(intptr_t hdn, struct __finddata64_t *finddata) FINDNEXT(64,64,MEMCPY_NAME)
INTERN ATTR_DOSTEXT int LIBCCALL libc_findnext32i64(intptr_t hdn, struct _finddata32i64_t *finddata) FINDNEXT(32,64,MEMCPY_NAME)
INTERN ATTR_DOSTEXT int LIBCCALL libc_findnext64i32(intptr_t hdn, struct _finddata64i32_t *finddata) FINDNEXT(64,32,MEMCPY_NAME)
#undef MEMCPY_NAME
#define UTF8TO16_NAME(dst,dst_elem,src,src_len) \
 { mbstate_t state = MBSTATE_INIT; \
   libc_utf8to16(src,src_len,(char16_t *)(dst), \
                 dst_elem,&state,UNICODE_F_NORMAL); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_16findnext32(intptr_t hdn, struct _wfinddata32_t *finddata) FINDNEXT(32,32,UTF8TO16_NAME)
INTERN ATTR_DOSTEXT int LIBCCALL libc_16findnext64(intptr_t hdn, struct _wfinddata64_t *finddata) FINDNEXT(64,64,UTF8TO16_NAME)
INTERN ATTR_DOSTEXT int LIBCCALL libc_16findnext32i64(intptr_t hdn, struct _wfinddata32i64_t *finddata) FINDNEXT(32,64,UTF8TO16_NAME)
INTERN ATTR_DOSTEXT int LIBCCALL libc_16findnext64i32(intptr_t hdn, struct _wfinddata64i32_t *finddata) FINDNEXT(64,32,UTF8TO16_NAME)
#undef UTF8TO32_NAME
#define UTF8TO32_NAME(dst,dst_elem,src,src_len) \
 { mbstate_t state = MBSTATE_INIT; \
   libc_utf8to32(src,src_len,(char32_t *)(dst), \
                 dst_elem,&state,UNICODE_F_NORMAL); }
INTERN ATTR_DOSTEXT int LIBCCALL libc_32findnext32(intptr_t hdn, struct _wfinddata32_t *finddata) FINDNEXT(32,32,UTF8TO32_NAME)
INTERN ATTR_DOSTEXT int LIBCCALL libc_32findnext64(intptr_t hdn, struct _wfinddata64_t *finddata) FINDNEXT(64,64,UTF8TO32_NAME)
INTERN ATTR_DOSTEXT int LIBCCALL libc_32findnext32i64(intptr_t hdn, struct _wfinddata32i64_t *finddata) FINDNEXT(32,64,UTF8TO32_NAME)
INTERN ATTR_DOSTEXT int LIBCCALL libc_32findnext64i32(intptr_t hdn, struct _wfinddata64i32_t *finddata) FINDNEXT(64,32,UTF8TO32_NAME)
#undef UTF8TO32_NAME


DEFINE_PUBLIC_ALIAS(_findclose,libc_findclose);

DEFINE_PUBLIC_ALIAS(__KSYM(_findfirst32),libc_findfirst32);
DEFINE_PUBLIC_ALIAS(__KSYM(_findfirst64),libc_findfirst64);
DEFINE_PUBLIC_ALIAS(__KSYM(_findfirst32i64),libc_findfirst32i64);
DEFINE_PUBLIC_ALIAS(__KSYM(_findfirst64i32),libc_findfirst64i32);
DEFINE_PUBLIC_ALIAS(__DSYM(_findfirst32),libc_dos_findfirst32);
DEFINE_PUBLIC_ALIAS(__DSYM(_findfirst64),libc_dos_findfirst64);
DEFINE_PUBLIC_ALIAS(__DSYM(_findfirst32i64),libc_dos_findfirst32i64);
DEFINE_PUBLIC_ALIAS(__DSYM(_findfirst64i32),libc_dos_findfirst64i32);

DEFINE_PUBLIC_ALIAS(__KSYMw16(_wfindfirst32),libc_16findfirst32);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wfindfirst64),libc_16findfirst64);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wfindfirst32i64),libc_16findfirst32i64);
DEFINE_PUBLIC_ALIAS(__KSYMw16(_wfindfirst64i32),libc_16findfirst64i32);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wfindfirst32),libc_dos_16findfirst32);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wfindfirst64),libc_dos_16findfirst64);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wfindfirst32i64),libc_dos_16findfirst32i64);
DEFINE_PUBLIC_ALIAS(__DSYMw16(_wfindfirst64i32),libc_dos_16findfirst64i32);

DEFINE_PUBLIC_ALIAS(__KSYMw32(_wfindfirst32),libc_32findfirst32);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wfindfirst64),libc_32findfirst64);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wfindfirst32i64),libc_32findfirst32i64);
DEFINE_PUBLIC_ALIAS(__KSYMw32(_wfindfirst64i32),libc_32findfirst64i32);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wfindfirst32),libc_dos_32findfirst32);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wfindfirst64),libc_dos_32findfirst64);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wfindfirst32i64),libc_dos_32findfirst32i64);
DEFINE_PUBLIC_ALIAS(__DSYMw32(_wfindfirst64i32),libc_dos_32findfirst64i32);

DEFINE_PUBLIC_ALIAS(_findnext32,libc_findnext32);
DEFINE_PUBLIC_ALIAS(_findnext64,libc_findnext64);
DEFINE_PUBLIC_ALIAS(_findnext32i64,libc_findnext32i64);
DEFINE_PUBLIC_ALIAS(_findnext64i32,libc_findnext64i32);
DEFINE_PUBLIC_ALIAS(_wfindnext32,libc_32findnext32);
DEFINE_PUBLIC_ALIAS(_wfindnext64,libc_32findnext64);
DEFINE_PUBLIC_ALIAS(_wfindnext32i64,libc_32findnext32i64);
DEFINE_PUBLIC_ALIAS(_wfindnext64i32,libc_32findnext64i32);
DEFINE_PUBLIC_ALIAS(__DSYM(_wfindnext32),libc_16findnext32);
DEFINE_PUBLIC_ALIAS(__DSYM(_wfindnext64),libc_16findnext64);
DEFINE_PUBLIC_ALIAS(__DSYM(_wfindnext32i64),libc_16findnext32i64);
DEFINE_PUBLIC_ALIAS(__DSYM(_wfindnext64i32),libc_16findnext64i32);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


DECL_END

#endif /* !GUARD_LIBS_LIBC_DIRENT_C */
