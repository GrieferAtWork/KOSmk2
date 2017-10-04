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
#ifndef GUARD_LIBS_LIBC_MALLOC_DOS_C_INL
#define GUARD_LIBS_LIBC_MALLOC_DOS_C_INL 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include "libc.h"
#include "malloc.h"
#include "string.h"
#include "fcntl.h"

#include <uchar.h>
#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <bits/dos-errno.h>

DECL_BEGIN

/* Export the regular malloc API, either linked against MALL
 * without debug info, dlmalloc or an external malloc API. */
#ifndef EXPORT_MALLOC
#ifdef CONFIG_DEBUG_MALLOC
#define EXPORT_MALLOC             libc_malloc
#define EXPORT_FREE               libc_free
#define EXPORT_CALLOC             libc_calloc
#define EXPORT_REALLOC            libc_realloc
#define EXPORT_REALLOC_IN_PLACE   libc_realloc_in_place
#define EXPORT_MEMALIGN           libc_memalign
#define EXPORT_VALLOC             libc_valloc
#define EXPORT_PVALLOC            libc_pvalloc
#define EXPORT_POSIX_MEMALIGN     libc_posix_memalign
#define EXPORT_MALLOPT            libc_mallopt
#define EXPORT_MALLOC_TRIM        libc_malloc_trim
#define EXPORT_MALLOC_USABLE_SIZE libc_malloc_usable_size
#else
#define EXPORT_MALLOC             dlmalloc
#define EXPORT_FREE               dlfree
#define EXPORT_CALLOC             dlcalloc
#define EXPORT_REALLOC            dlrealloc
#define EXPORT_REALLOC_IN_PLACE   dlrealloc_in_place
#define EXPORT_MEMALIGN           dlmemalign
#define EXPORT_VALLOC             dlvalloc
#define EXPORT_PVALLOC            dlpvalloc
#define EXPORT_POSIX_MEMALIGN     dlposix_memalign
#define EXPORT_MALLOPT            dlmallopt
#define EXPORT_MALLOC_TRIM        dlmalloc_trim
#define EXPORT_MALLOC_USABLE_SIZE dlmalloc_usable_size
#endif
#endif /* !EXPORT_MALLOC */

#ifndef CONFIG_LIBC_NO_DOS_LIBC

 /* Define the DOS debug-malloc API. */
DEFINE_INTERN_ALIAS(libc_dos_expand,EXPORT_REALLOC_IN_PLACE);
INTERN ATTR_DOSTEXT void *LIBCCALL libc_dos_malloc_dbg(size_t size, int UNUSED(btype), char const *file, int lno) { return libc__malloc_d(size,DEBUGINFO_MK(file,lno,NULL)); }
INTERN ATTR_DOSTEXT void *LIBCCALL libc_dos_calloc_dbg(size_t count, size_t size, int UNUSED(btype), char const *file, int lno) { return libc__calloc_d(count,size,DEBUGINFO_MK(file,lno,NULL)); }
INTERN ATTR_DOSTEXT void *LIBCCALL libc_dos_realloc_dbg(void *mptr, size_t size, int UNUSED(btype), char const *file, int lno) { return libc__realloc_d(mptr,size,DEBUGINFO_MK(file,lno,NULL)); }
INTERN ATTR_DOSTEXT void *LIBCCALL libc_dos_expand_dbg(void *mptr, size_t size, int UNUSED(btype), char const *file, int lno) { return libc__realloc_in_place_d(mptr,size,DEBUGINFO_MK(file,lno,NULL)); }
INTERN ATTR_DOSTEXT void *LIBCCALL libc_dos_recalloc_dbg(void *mptr, size_t count, size_t size, int UNUSED(btype), char const *file, int lno) {
 size_t oldsize = libc__malloc_usable_size_d(mptr,DEBUGINFO_MK(file,lno,NULL));
 void *result = (size *= count,libc__realloc_d(mptr,size,DEBUGINFO_MK(file,lno,NULL)));
 if (result && size > oldsize) libc_memset((byte_t *)result+oldsize,0,size-oldsize);
 return result;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_recalloc(void *mptr, size_t count, size_t size) {
 size_t oldsize = EXPORT_MALLOC_USABLE_SIZE(mptr);
 void *result = (size *= count,EXPORT_REALLOC(mptr,size));
 if (result && size > oldsize) libc_memset((byte_t *)result+oldsize,0,size-oldsize);
 return result;
}


DEFINE_INTERN_ALIAS(libc_dos_msize,EXPORT_MALLOC_USABLE_SIZE);
#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_dos_free_dbg,EXPORT_FREE);
DEFINE_INTERN_ALIAS(libc_dos_msize_dbg,EXPORT_MALLOC_USABLE_SIZE);
#else
INTERN ATTR_DOSTEXT void LIBCCALL libc_dos_free_dbg(void *mptr, int UNUSED(btype)) { return EXPORT_FREE(mptr); }
INTERN ATTR_DOSTEXT size_t LIBCCALL libc_dos_msize_dbg(void *mptr, int UNUSED(btype)) { return EXPORT_MALLOC_USABLE_SIZE(mptr); }
#endif

/* DOS's aligned memory API.
 * NOTE: It's not pretty, but it should work... */
struct aptr { void *a_base; /*< Address of the real, underlying pointer. */ };
#define APTR(p) (((struct aptr *)(p))-1)
INTERN ATTR_DOSTEXT void LIBCCALL
libc_dos_aligned_free_dbg(void *mptr) {
 if (mptr) EXPORT_FREE(APTR(mptr)->a_base);
}
INTERN ATTR_DOSTEXT size_t LIBCCALL
libc_dos_aligned_msize_dbg(void *mptr, size_t UNUSED(align), size_t UNUSED(off)) {
 size_t result;
 if (!mptr) return 0;
 result  = EXPORT_MALLOC_USABLE_SIZE(APTR(mptr)->a_base);
 result -= (uintptr_t)mptr-(uintptr_t)APTR(mptr)->a_base;
 return result;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_malloc_dbg(size_t size, size_t align, char const *file, int lno) {
 struct aptr *result; void *base;
 if (align < sizeof(struct aptr))
     align = sizeof(struct aptr);
 base = libc__malloc_d(size+align,DEBUGINFO_MK(file,lno,NULL));
 if (!base) return NULL;
 result = (struct aptr *)CEIL_ALIGN((uintptr_t)base,align)-1;
 result->a_base = base;
 return result+1;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_offset_malloc_dbg(size_t size, size_t align,
                                   size_t off, char const *file, int lno) {
 struct aptr *result; void *base;
 if (align < sizeof(struct aptr))
     align = sizeof(struct aptr);
 base = libc__malloc_d(size+align+off,DEBUGINFO_MK(file,lno,NULL));
 if (!base) return NULL;
 result = (struct aptr *)(CEIL_ALIGN((uintptr_t)base,align)+off)-1;
 result->a_base = base;
 return result+1;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_realloc_dbg(void *mptr, size_t size, size_t align,
                             char const *file, int lno) {
 size_t oldsize; void *result;
 result = libc_dos_aligned_malloc_dbg(size,align,file,lno);
 if (!result) return NULL;
 oldsize = libc_dos_aligned_msize_dbg(mptr,0,0);
 libc_memcpy(result,mptr,MIN(size,oldsize));
 libc_dos_aligned_free_dbg(mptr);
 return result;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_recalloc_dbg(void *mptr, size_t count, size_t size,
                              size_t align, char const *file, int lno) {
 size_t oldsize; void *result; size *= count;
 result = libc_dos_aligned_malloc_dbg(size,align,file,lno);
 if (!result) return NULL;
 oldsize = libc_dos_aligned_msize_dbg(mptr,0,0);
 libc_memcpy(result,mptr,MIN(size,oldsize));
 if (size > oldsize)
     libc_memset((byte_t *)result+oldsize,0,size-oldsize);
 libc_dos_aligned_free_dbg(mptr);
 return result;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_offset_realloc_dbg(void *mptr, size_t size, size_t align,
                                    size_t off, char const *file, int lno) {
 size_t oldsize; void *result;
 result = libc_dos_aligned_offset_malloc_dbg(size,align,off,file,lno);
 if (!result) return NULL;
 oldsize = libc_dos_aligned_msize_dbg(mptr,0,0);
 libc_memcpy(result,mptr,MIN(size,oldsize));
 libc_dos_aligned_free_dbg(mptr);
 return result;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_offset_recalloc_dbg(void *mptr, size_t count, size_t size,
                                     size_t align, size_t off, char const *file, int lno) {
 size_t oldsize; void *result; size *= count;
 result = libc_dos_aligned_offset_malloc_dbg(size,align,off,file,lno);
 if (!result) return NULL;
 oldsize = libc_dos_aligned_msize_dbg(mptr,0,0);
 libc_memcpy(result,mptr,MIN(size,oldsize));
 if (size > oldsize)
     libc_memset((byte_t *)result+oldsize,0,size-oldsize);
 libc_dos_aligned_free_dbg(mptr);
 return result;
}


DEFINE_INTERN_ALIAS(libc_dos_aligned_free,libc_dos_aligned_free_dbg);
DEFINE_INTERN_ALIAS(libc_dos_aligned_msize,libc_dos_aligned_msize_dbg);
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_malloc(size_t size, size_t align) {
 struct aptr *result; void *base;
 if (align < sizeof(struct aptr))
     align = sizeof(struct aptr);
 base = EXPORT_MALLOC(size+align);
 if (!base) return NULL;
 result = (struct aptr *)CEIL_ALIGN((uintptr_t)base,align)-1;
 result->a_base = base;
 return result+1;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_offset_malloc(size_t size, size_t align, size_t off) {
 struct aptr *result; void *base;
 if (align < sizeof(struct aptr))
     align = sizeof(struct aptr);
 base = EXPORT_MALLOC(size+align+off);
 if (!base) return NULL;
 result = (struct aptr *)(CEIL_ALIGN((uintptr_t)base,align)+off)-1;
 result->a_base = base;
 return result+1;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_realloc(void *mptr, size_t size, size_t align) {
 size_t oldsize; void *result;
 result = libc_dos_aligned_malloc(size,align);
 if (!result) return NULL;
 oldsize = libc_dos_aligned_msize(mptr,0,0);
 libc_memcpy(result,mptr,MIN(size,oldsize));
 libc_dos_aligned_free_dbg(mptr);
 return result;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_recalloc(void *mptr, size_t count, size_t size, size_t align) {
 size_t oldsize; void *result; size *= count;
 result = libc_dos_aligned_malloc(size,align);
 if (!result) return NULL;
 oldsize = libc_dos_aligned_msize(mptr,0,0);
 libc_memcpy(result,mptr,MIN(size,oldsize));
 if (size > oldsize)
     libc_memset((byte_t *)result+oldsize,0,size-oldsize);
 libc_dos_aligned_free_dbg(mptr);
 return result;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_offset_realloc(void *mptr, size_t size, size_t align, size_t off) {
 size_t oldsize; void *result;
 result = libc_dos_aligned_offset_malloc(size,align,off);
 if (!result) return NULL;
 oldsize = libc_dos_aligned_msize_dbg(mptr,0,0);
 libc_memcpy(result,mptr,MIN(size,oldsize));
 libc_dos_aligned_free_dbg(mptr);
 return result;
}
INTERN ATTR_DOSTEXT void *LIBCCALL
libc_dos_aligned_offset_recalloc(void *mptr, size_t count, size_t size,
                                 size_t align, size_t off) {
 size_t oldsize; void *result; size *= count;
 result = libc_dos_aligned_offset_malloc(size,align,off);
 if (!result) return NULL;
 oldsize = libc_dos_aligned_msize_dbg(mptr,0,0);
 libc_memcpy(result,mptr,MIN(size,oldsize));
 if (size > oldsize)
     libc_memset((byte_t *)result+oldsize,0,size-oldsize);
 libc_dos_aligned_free_dbg(mptr);
 return result;
}


INTERN ATTR_DOSTEXT char *LIBCCALL
libc_dos_strdup_dbg(char const *str, int UNUSED(btype), char const *file, int lno) {
 return libc__strdup_d(str,DEBUGINFO_MK(file,lno,NULL));
}
INTERN ATTR_DOSTEXT char *LIBCCALL
libc_dos_tempnam_dbg(char const *dnam, char const *prefix,
                     int UNUSED(btype), char const *file, int lno) {
 NOT_IMPLEMENTED();
 return NULL;
}
INTERN ATTR_DOSTEXT char *LIBCCALL
libc_dos_fullpath_dbg(char *abspath, char const *path, size_t buflen,
                      int UNUSED(btype), char const *file, int lno) {
 NOT_IMPLEMENTED();
 return NULL;
}
INTERN ATTR_DOSTEXT char *LIBCCALL
libc_dos_getcwd_dbg(char *buf, int buflen, int UNUSED(btype),
                    char const *UNUSED(file), int UNUSED(lno)) {
 return libc_getcwd(buf,(size_t)buflen);
}
INTERN ATTR_DOSTEXT char *LIBCCALL
libc_dos_getdcwd_dbg(int drive, char *buf, int buflen,
                     int UNUSED(btype), char const *UNUSED(file), int UNUSED(lno)) {
 return libc_getdcwd(drive,buf,(size_t)buflen);
}
DEFINE_INTERN_ALIAS(libc_dos_getdcwd_lk_dbg,libc_dos_getdcwd_dbg);
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_dos_dupenv_s_dbg(char **pbuf, size_t *pbuflen, char const *varname,
                      int UNUSED(btype), char const *file, int lno) {
 NOT_IMPLEMENTED();
 return __DOS_ENOTSUP;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_dos_wdupenv_s_dbg(char16_t **pbuf, size_t *pbuflen,
                       char16_t const *varname, int UNUSED(btype),
                       char const *file, int lno) {
 NOT_IMPLEMENTED();
 return __DOS_ENOTSUP;
}
INTERN ATTR_DOSTEXT char16_t *LIBCCALL
libc_dos_wcsdup_dbg(char16_t const *str, int UNUSED(btype),
                    char const *UNUSED(file), int UNUSED(lno)) {
 return libc_16wcsdup(str);
}
INTERN ATTR_DOSTEXT char16_t *LIBCCALL
libc_dos_wtempnam_dbg(char16_t const *dnam, char16_t const *prefix,
                      int UNUSED(btype), char const *file, int lno) {
 NOT_IMPLEMENTED();
 return NULL;
}
INTERN ATTR_DOSTEXT char16_t *LIBCCALL
libc_dos_wfullpath_dbg(char16_t *abspath, char16_t const *path, size_t buflen,
                       int UNUSED(btype), char const *file, int lno) {
 NOT_IMPLEMENTED();
 return NULL;
}
INTERN ATTR_DOSTEXT char16_t *LIBCCALL
libc_dos_wgetcwd_dbg(char16_t *buf, int buflen, int UNUSED(btype),
                     char const *UNUSED(file), int UNUSED(lno)) {
 return libc_16wgetcwd(buf,buflen);
}
INTERN ATTR_DOSTEXT char16_t *LIBCCALL
libc_dos_wgetdcwd_dbg(int drive, char16_t *buf, int buflen,
                      int UNUSED(btype), char const *UNUSED(file), int UNUSED(lno)) {
 return libc_16wgetdcwd(drive,buf,buflen);
}
DEFINE_INTERN_ALIAS(libc_dos_wgetdcwd_lk_dbg,libc_dos_wgetdcwd_dbg);


DEFINE_PUBLIC_ALIAS(_CrtCheckMemory,libc__mall_validate);
DEFINE_PUBLIC_ALIAS(_recalloc,libc_dos_recalloc);
DEFINE_PUBLIC_ALIAS(_strdup,libc_strdup);
DEFINE_PUBLIC_ALIAS(_msize,libc_dos_msize);
DEFINE_PUBLIC_ALIAS(_aligned_msize,libc_dos_aligned_msize);
DEFINE_PUBLIC_ALIAS(_aligned_malloc,libc_dos_aligned_malloc);
DEFINE_PUBLIC_ALIAS(_aligned_realloc,libc_dos_aligned_realloc);
DEFINE_PUBLIC_ALIAS(_aligned_recalloc,libc_dos_aligned_recalloc);
DEFINE_PUBLIC_ALIAS(_aligned_offset_malloc,libc_dos_aligned_offset_malloc);
DEFINE_PUBLIC_ALIAS(_aligned_offset_realloc,libc_dos_aligned_offset_realloc);
DEFINE_PUBLIC_ALIAS(_aligned_offset_recalloc,libc_dos_aligned_offset_recalloc);
DEFINE_PUBLIC_ALIAS(_aligned_free,libc_dos_aligned_free);
DEFINE_PUBLIC_ALIAS(_expand,libc_dos_expand);

DEFINE_PUBLIC_ALIAS(_malloc_dbg,libc_dos_malloc_dbg);
DEFINE_PUBLIC_ALIAS(_calloc_dbg,libc_dos_calloc_dbg);
DEFINE_PUBLIC_ALIAS(_realloc_dbg,libc_dos_realloc_dbg);
DEFINE_PUBLIC_ALIAS(_recalloc_dbg,libc_dos_recalloc_dbg);
DEFINE_PUBLIC_ALIAS(_expand_dbg,libc_dos_expand_dbg);
DEFINE_PUBLIC_ALIAS(_free_dbg,libc_dos_free_dbg);
//DEFINE_PUBLIC_ALIAS(_msize_dbg,libc_dos_msize_dbg);
DEFINE_PUBLIC_ALIAS(_msize_debug,libc_dos_msize_dbg); /* For some reason, this is the real name... */
DEFINE_PUBLIC_ALIAS(_aligned_msize_dbg,libc_dos_aligned_msize_dbg);
DEFINE_PUBLIC_ALIAS(_aligned_malloc_dbg,libc_dos_aligned_malloc_dbg);
DEFINE_PUBLIC_ALIAS(_aligned_realloc_dbg,libc_dos_aligned_realloc_dbg);
DEFINE_PUBLIC_ALIAS(_aligned_recalloc_dbg,libc_dos_aligned_recalloc_dbg);
DEFINE_PUBLIC_ALIAS(_aligned_offset_malloc_dbg,libc_dos_aligned_offset_malloc_dbg);
DEFINE_PUBLIC_ALIAS(_aligned_offset_realloc_dbg,libc_dos_aligned_offset_realloc_dbg);
DEFINE_PUBLIC_ALIAS(_aligned_offset_recalloc_dbg,libc_dos_aligned_offset_recalloc_dbg);
DEFINE_PUBLIC_ALIAS(_aligned_free_dbg,libc_dos_aligned_free_dbg);
DEFINE_PUBLIC_ALIAS(mbsdup_dbg,libc_dos_strdup_dbg); /* Dunno know why this one's exported (Much less under that name...) */
DEFINE_PUBLIC_ALIAS(_strdup_dbg,libc_dos_strdup_dbg);
DEFINE_PUBLIC_ALIAS(_tempnam_dbg,libc_dos_tempnam_dbg);
DEFINE_PUBLIC_ALIAS(_fullpath_dbg,libc_dos_fullpath_dbg);
DEFINE_PUBLIC_ALIAS(_getcwd_dbg,libc_dos_getcwd_dbg);
DEFINE_PUBLIC_ALIAS(_getdcwd_dbg,libc_dos_getdcwd_dbg);
DEFINE_PUBLIC_ALIAS(_getdcwd_lk_dbg,libc_dos_getdcwd_lk_dbg);
DEFINE_PUBLIC_ALIAS(_dupenv_s_dbg,libc_dos_dupenv_s_dbg);
DEFINE_PUBLIC_ALIAS(_wdupenv_s_dbg,libc_dos_wdupenv_s_dbg);
DEFINE_PUBLIC_ALIAS(_wcsdup_dbg,libc_dos_wcsdup_dbg);
DEFINE_PUBLIC_ALIAS(_wtempnam_dbg,libc_dos_wtempnam_dbg);
DEFINE_PUBLIC_ALIAS(_wfullpath_dbg,libc_dos_wfullpath_dbg);
DEFINE_PUBLIC_ALIAS(_wgetcwd_dbg,libc_dos_wgetcwd_dbg);
DEFINE_PUBLIC_ALIAS(_wgetdcwd_dbg,libc_dos_wgetdcwd_dbg);
DEFINE_PUBLIC_ALIAS(_wgetdcwd_lk_dbg,libc_dos_wgetdcwd_lk_dbg);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_MALLOC_DOS_C_INL */
