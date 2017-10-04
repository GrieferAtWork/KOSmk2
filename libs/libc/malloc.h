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
#ifndef GUARD_LIBS_LIBC_MALLOC_H
#define GUARD_LIBS_LIBC_MALLOC_H 1

#ifndef __KERNEL__

#undef CONFIG_LIBC_ALLOW_EXTERN_MALLOC
#undef CONFIG_DEBUG_MALLOC
//#define CONFIG_LIBC_ALLOW_EXTERN_MALLOC 1
#ifdef CONFIG_DEBUG
//#define CONFIG_DEBUG_MALLOC 1
#endif

#include "libc.h"
#include <hybrid/compiler.h>
#include <hybrid/debuginfo.h>
#include <hybrid/types.h>
#include <stdarg.h>

/* Must include both malloc & stdlib, so we can delete
 * all malloc-macros before dlmalloc is included. */
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include <uchar.h>
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_BEGIN

#undef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT   ((size_t)__MALL_MIN_ALIGNMENT)

#undef malloc
#undef calloc
#undef aligned_alloc
#undef memalign
#undef realloc
#undef realloc_in_place
#undef pvalloc
#undef valloc
#undef posix_memalign
#undef free
#undef cfree
#undef mallopt
#undef malloc_trim
#undef malloc_usable_size
#undef __memdup
#undef __memcdup
#undef memdup
#undef memcdup
#undef strdup
#undef strndup
#undef strdupf
#undef vstrdupf


#ifndef __ptbwalker_defined
#define __ptbwalker_defined 1
typedef __SSIZE_TYPE__ (__LIBCCALL *__ptbwalker)(void const *__restrict __instruction_pointer,
                                                 void const *__restrict __frame_address,
                                                 size_t __frame_index, void *__closure);
#endif

#if 0
/* Malloc API overridable by user-applications, using ELF public symbol linkage.
 * >> If you want to implement your own MALLOC API and have libc use it, you must implement all
 *    of the following functions using these exact function names within the global namespace.
 * Yes: You just code a function 'malloc' with default visibility, and KOS's ELF
 *      linker will bind libc in such a way that your function will be called.
 * NOTE: This requires libc having been built with 'CONFIG_LIBC_ALLOW_EXTERN_MALLOC' enabled. */
FUNDEF void  *(LIBCCALL malloc)(size_t n_bytes);
FUNDEF void   (LIBCCALL free)(void *__restrict ptr);
FUNDEF void  *(LIBCCALL calloc)(size_t count, size_t n_bytes);
FUNDEF void  *(LIBCCALL realloc)(void *__restrict ptr, size_t n_bytes);
FUNDEF void  *(LIBCCALL realloc_in_place)(void *__restrict ptr, size_t n_bytes);
FUNDEF void  *(LIBCCALL memalign)(size_t alignment, size_t n_bytes);
FUNDEF void  *(LIBCCALL valloc)(size_t n_bytes);
FUNDEF void  *(LIBCCALL pvalloc)(size_t n_bytes);
FUNDEF int    (LIBCCALL posix_memalign)(void **__restrict pp, size_t alignment, size_t n_bytes);
FUNDEF int    (LIBCCALL mallopt)(int parameter_number, int parameter_value);
FUNDEF int    (LIBCCALL malloc_trim)(size_t pad);
FUNDEF size_t (LIBCCALL malloc_usable_size)(void *__restrict ptr);
#endif

/* Internal MALLOC implementation, using dlmalloc. */
INTDEF void  *(LIBCCALL dlmalloc)(size_t n_bytes);
INTDEF void   (LIBCCALL dlfree)(void *__restrict ptr);
INTDEF void  *(LIBCCALL dlcalloc)(size_t count, size_t n_bytes);
INTDEF void  *(LIBCCALL dlrealloc)(void *__restrict ptr, size_t n_bytes);
INTDEF void  *(LIBCCALL dlrealloc_in_place)(void *__restrict ptr, size_t n_bytes);
INTDEF void  *(LIBCCALL dlmemalign)(size_t alignment, size_t n_bytes);
INTDEF void  *(LIBCCALL dlvalloc)(size_t n_bytes);
INTDEF void  *(LIBCCALL dlpvalloc)(size_t n_bytes);
INTDEF int    (LIBCCALL dlposix_memalign)(void **__restrict pp, size_t alignment, size_t n_bytes);
INTDEF int    (LIBCCALL dlmallopt)(int parameter_number, int parameter_value);
INTDEF int    (LIBCCALL dlmalloc_trim)(size_t pad);
INTDEF size_t (LIBCCALL dlmalloc_usable_size)(void *__restrict ptr);

/* Generic malloc functions, linked against MALL or dlmalloc based on 'CONFIG_DEBUG_MALLOC'. */
#ifdef CONFIG_DEBUG_MALLOC
INTDEF SAFE void  *(LIBCCALL libc_malloc)(size_t n_bytes);
INTDEF SAFE void   (LIBCCALL libc_free)(void *__restrict ptr);
INTDEF SAFE void  *(LIBCCALL libc_calloc)(size_t count, size_t n_bytes);
INTDEF SAFE void  *(LIBCCALL libc_realloc)(void *__restrict ptr, size_t n_bytes);
INTDEF SAFE void  *(LIBCCALL libc_realloc_in_place)(void *__restrict ptr, size_t n_bytes);
INTDEF SAFE void  *(LIBCCALL libc_memalign)(size_t alignment, size_t n_bytes);
INTDEF SAFE void  *(LIBCCALL libc_valloc)(size_t n_bytes);
INTDEF SAFE void  *(LIBCCALL libc_pvalloc)(size_t n_bytes);
INTDEF SAFE int    (LIBCCALL libc_posix_memalign)(void **__restrict pp, size_t alignment, size_t n_bytes);
INTDEF SAFE int    (LIBCCALL libc_mallopt)(int parameter_number, int parameter_value);
INTDEF SAFE int    (LIBCCALL libc_malloc_trim)(size_t pad);
INTDEF SAFE size_t (LIBCCALL libc_malloc_usable_size)(void *__restrict ptr);
#define __libc_malloc(n_bytes)                           libc_malloc(n_bytes)
#define __libc_free(ptr)                                 libc_free(ptr)
#define __libc_calloc(count,n_bytes)                     libc_calloc(count,n_bytes)
#define __libc_realloc(ptr,n_bytes)                      libc_realloc(ptr,n_bytes)
#define __libc_realloc_in_place(ptr,n_bytes)             libc_realloc_in_place(ptr,n_bytes)
#define __libc_memalign(alignment,n_bytes)               libc_memalign(alignment,n_bytes)
#define __libc_valloc(n_bytes)                           libc_valloc(n_bytes)
#define __libc_pvalloc(n_bytes)                          libc_pvalloc(n_bytes)
#define __libc_posix_memalign(pp,alignment,n_bytes)      libc_posix_memalign(pp,alignment,n_bytes)
#define __libc_mallopt(parameter_number,parameter_value) libc_mallopt(parameter_number,parameter_value)
#define __libc_malloc_trim(pad)                          libc_malloc_trim(pad)
#define __libc_malloc_usable_size(ptr)                   libc_malloc_usable_size(ptr)
#else /* !CONFIG_DEBUG_MALLOC */
#define __libc_malloc(n_bytes)                           dlmalloc(n_bytes)
#define __libc_free(ptr)                                 dlfree(ptr)
#define __libc_calloc(count,n_bytes)                     dlcalloc(count,n_bytes)
#define __libc_realloc(ptr,n_bytes)                      dlrealloc(ptr,n_bytes)
#define __libc_realloc_in_place(ptr,n_bytes)             dlrealloc_in_place(ptr,n_bytes)
#define __libc_memalign(alignment,n_bytes)               dlmemalign(alignment,n_bytes)
#define __libc_valloc(n_bytes)                           dlvalloc(n_bytes)
#define __libc_pvalloc(n_bytes)                          dlpvalloc(n_bytes)
#define __libc_posix_memalign(pp,alignment,n_bytes)      dlposix_memalign(pp,alignment,n_bytes)
#define __libc_mallopt(parameter_number,parameter_value) dlmallopt(parameter_number,parameter_value)
#define __libc_malloc_trim(pad)                          dlmalloc_trim(pad)
#define __libc_malloc_usable_size(ptr)                   dlmalloc_usable_size(ptr)
#endif /* !CONFIG_DEBUG_MALLOC */

INTDEF SAFE void  *(LIBCCALL libc_memdup)(void const *__restrict ptr, size_t n_bytes);
INTDEF SAFE char  *(LIBCCALL libc_strdup)(char const *__restrict str);
INTDEF SAFE char  *(LIBCCALL libc_strndup)(char const *__restrict str, size_t max_chars);
INTDEF SAFE void  *(LIBCCALL libc_memcdup)(void const *__restrict ptr, int needle, size_t n_bytes);
INTDEF SAFE char  *(LIBCCALL libc_vstrdupf)(char const *__restrict format, va_list args);
INTDEF SAFE char  *(ATTR_CDECL libc_strdupf)(char const *__restrict format, ...);

/* Mall debug-malloc API overlay. (Implemented using the '__libc_*' macros) */
#ifdef CONFIG_DEBUG_MALLOC
INTDEF SAFE void  *(LIBCCALL libc__malloc_d)(size_t n_bytes, DEBUGINFO);
INTDEF SAFE void   (LIBCCALL libc__free_d)(void *__restrict ptr, DEBUGINFO);
INTDEF SAFE void  *(LIBCCALL libc__calloc_d)(size_t count, size_t n_bytes, DEBUGINFO);
INTDEF SAFE void  *(LIBCCALL libc__realloc_d)(void *__restrict ptr, size_t n_bytes, DEBUGINFO);
INTDEF SAFE void  *(LIBCCALL libc__realloc_in_place_d)(void *__restrict ptr, size_t n_bytes, DEBUGINFO);
INTDEF SAFE void  *(LIBCCALL libc__memalign_d)(size_t alignment, size_t n_bytes, DEBUGINFO);
INTDEF SAFE void  *(LIBCCALL libc__valloc_d)(size_t n_bytes, DEBUGINFO);
INTDEF SAFE void  *(LIBCCALL libc__pvalloc_d)(size_t n_bytes, DEBUGINFO);
INTDEF SAFE int    (LIBCCALL libc__posix_memalign_d)(void **__restrict pp, size_t alignment, size_t n_bytes, DEBUGINFO);
INTDEF SAFE int    (LIBCCALL libc__mallopt_d)(int parameter_number, int parameter_value, DEBUGINFO);
INTDEF SAFE int    (LIBCCALL libc__malloc_trim_d)(size_t pad, DEBUGINFO);
INTDEF SAFE size_t (LIBCCALL libc__malloc_usable_size_d)(void *__restrict ptr, DEBUGINFO);
#else
#define libc__malloc_d(n_bytes,...)                           __libc_malloc(n_bytes)
#define libc__free_d(ptr,...)                                 __libc_free(ptr)
#define libc__calloc_d(count,n_bytes,...)                     __libc_calloc(count,n_bytes)
#define libc__realloc_d(ptr,n_bytes,...)                      __libc_realloc(ptr,n_bytes)
#define libc__realloc_in_place_d(ptr,n_bytes,...)             __libc_realloc_in_place(ptr,n_bytes)
#define libc__memalign_d(alignment,n_bytes,...)               __libc_memalign(alignment,n_bytes)
#define libc__valloc_d(n_bytes,...)                           __libc_valloc(n_bytes)
#define libc__pvalloc_d(n_bytes,...)                          __libc_pvalloc(n_bytes)
#define libc__posix_memalign_d(pp,alignment,n_bytes,...)      __libc_posix_memalign(pp,alignment,n_bytes)
#define libc__mallopt_d(parameter_number,parameter_value,...) __libc_mallopt(parameter_number,parameter_value)
#define libc__malloc_trim_d(pad,...)                          __libc_malloc_trim(pad)
#define libc__malloc_usable_size_d(ptr,...)                   __libc_malloc_usable_size(ptr)
#endif

INTDEF SAFE void  *(LIBCCALL libc__memdup_d)(void const *__restrict ptr, size_t n_bytes, DEBUGINFO);
INTDEF SAFE char  *(LIBCCALL libc__strdup_d)(char const *__restrict str, DEBUGINFO);
INTDEF SAFE char  *(LIBCCALL libc__strndup_d)(char const *__restrict str, size_t max_chars, DEBUGINFO);
INTDEF SAFE char  *(ATTR_CDECL libc__strdupf_d)(DEBUGINFO, char const *__restrict format, ...);
INTDEF SAFE char  *(LIBCCALL libc__vstrdupf_d)(char const *__restrict format, va_list args, DEBUGINFO);
INTDEF SAFE void  *(LIBCCALL libc__memcdup_d)(void const *__restrict ptr, int needle, size_t n_bytes, DEBUGINFO);

/* MALL debug API. */
INTDEF void   *(LIBCCALL libc__mall_getattrib)(void *__restrict ptr, int attrib);
INTDEF ssize_t (LIBCCALL libc__mall_traceback)(void *__restrict ptr, __ptbwalker callback, void *closure);
INTDEF void    (LIBCCALL libc__mall_printleaks)(void);
INTDEF void    (LIBCCALL libc__mall_validate)(void);
INTDEF ssize_t (LIBCCALL libc__mall_enum)(void *checkpoint, ssize_t (*callback)(void *__restrict ptr, void *closure), void *closure);
INTDEF void   *(LIBCCALL libc__mall_untrack)(void *mallptr);
INTDEF void   *(LIBCCALL libc__mall_nofree)(void *mallptr);
INTDEF void   *(LIBCCALL libc__mall_getattrib_d)(void *__restrict ptr, int attrib, DEBUGINFO);
INTDEF ssize_t (LIBCCALL libc__mall_traceback_d)(void *__restrict ptr, __ptbwalker callback, void *closure, DEBUGINFO);
INTDEF void    (LIBCCALL libc__mall_printleaks_d)(DEBUGINFO);
INTDEF void    (LIBCCALL libc__mall_validate_d)(DEBUGINFO);
INTDEF ssize_t (LIBCCALL libc__mall_enum_d)(void *checkpoint, ssize_t (*callback)(void *__restrict ptr, void *closure), void *closure, DEBUGINFO);
INTDEF void   *(LIBCCALL libc__mall_untrack_d)(void *mallptr, DEBUGINFO);
INTDEF void   *(LIBCCALL libc__mall_nofree_d)(void *mallptr, DEBUGINFO);

#ifndef CONFIG_DEBUG_MALLOC
/* All of the mall-functions are configured as no-ops.
 * >> No need to actually generate calls to them! */
#define libc__mall_getattrib(ptr,attrib)                   ((void *)0)
#define libc__mall_traceback(ptr,callback,closure)         ((ssize_t)0)
#define libc__mall_printleaks()                            ((void)0)
#define libc__mall_validate()                              ((void)0)
#define libc__mall_enum(checkpoint,callback,closure)       ((ssize_t)0)
#define libc__mall_untrack(mallptr)                         (mallptr)
#define libc__mall_nofree(mallptr)                          (mallptr)
#define libc__mall_getattrib_d(ptr,attrib,...)             ((void *)0)
#define libc__mall_traceback_d(ptr,callback,closure,...)   ((ssize_t)0)
#define libc__mall_printleaks_d(...)                       ((void)0)
#define libc__mall_validate_d(...)                         ((void)0)
#define libc__mall_enum_d(checkpoint,callback,closure,...) ((ssize_t)0)
#define libc__mall_untrack_d(mallptr,...)                   (mallptr)
#define libc__mall_nofree_d(mallptr,...)                    (mallptr)
#endif


/* Select the effective malloc API used by libc itself. */
#ifdef CONFIG_DEBUG_MALLOC
#ifdef CONFIG_LIBC_ALLOW_EXTERN_MALLOC
#   define libc_malloc(n_bytes)                           _malloc_d(n_bytes,DEBUGINFO_GEN)
#   define libc_free(ptr)                                 _free_d(ptr,DEBUGINFO_GEN)
#   define libc_calloc(count,n_bytes)                     _calloc_d(count,n_bytes,DEBUGINFO_GEN)
#   define libc_realloc(ptr,n_bytes)                      _realloc_d(ptr,n_bytes,DEBUGINFO_GEN)
#   define libc_realloc_in_place(ptr,n_bytes)             _realloc_in_place_d(ptr,n_bytes,DEBUGINFO_GEN)
#   define libc_memalign(alignment,n_bytes)               _memalign_d(alignment,n_bytes,DEBUGINFO_GEN)
#   define libc_valloc(n_bytes)                           _valloc_d(n_bytes,DEBUGINFO_GEN)
#   define libc_pvalloc(n_bytes)                          _pvalloc_d(n_bytes,DEBUGINFO_GEN)
#   define libc_posix_memalign(pp,alignment,n_bytes)      _posix_memalign_d(pp,alignment,n_bytes,DEBUGINFO_GEN)
#   define libc_mallopt(parameter_number,parameter_value) _mallopt_d(parameter_number,parameter_value,DEBUGINFO_GEN)
#   define libc_malloc_trim(pad)                          _malloc_trim_d(pad,DEBUGINFO_GEN)
#   define libc_malloc_usable_size(ptr)                   _malloc_usable_size_d(ptr,DEBUGINFO_GEN)
#   define libc_memdup(ptr,n_bytes)                       _memdup_d(ptr,n_bytes,DEBUGINFO_GEN)
#   define libc_strdup(str)                               _strdup_d(str,DEBUGINFO_GEN)
#   define libc_strndup(str,max_chars)                    _strndup_d(str,max_chars,DEBUGINFO_GEN)
#   define libc_memcdup(ptr,needle,n_bytes)               _memcdup_d(ptr,needle,n_bytes,DEBUGINFO_GEN)
#   define libc_vstrdupf(format,args)                     _vstrdupf_d(format,args,DEBUGINFO_GEN)
#   define libc_strdupf(...)                              _strdupf_d(DEBUGINFO_GEN,__VA_ARGS__)
#else
#   define libc_malloc(n_bytes)                           libc__malloc_d(n_bytes,DEBUGINFO_GEN)
#   define libc_free(ptr)                                 libc__free_d(ptr,DEBUGINFO_GEN)
#   define libc_calloc(count,n_bytes)                     libc__calloc_d(count,n_bytes,DEBUGINFO_GEN)
#   define libc_realloc(ptr,n_bytes)                      libc__realloc_d(ptr,n_bytes,DEBUGINFO_GEN)
#   define libc_realloc_in_place(ptr,n_bytes)             libc__realloc_in_place_d(ptr,n_bytes,DEBUGINFO_GEN)
#   define libc_memalign(alignment,n_bytes)               libc__memalign_d(alignment,n_bytes,DEBUGINFO_GEN)
#   define libc_valloc(n_bytes)                           libc__valloc_d(n_bytes,DEBUGINFO_GEN)
#   define libc_pvalloc(n_bytes)                          libc__pvalloc_d(n_bytes,DEBUGINFO_GEN)
#   define libc_posix_memalign(pp,alignment,n_bytes)      libc__posix_memalign_d(pp,alignment,n_bytes,DEBUGINFO_GEN)
#   define libc_mallopt(parameter_number,parameter_value) libc__mallopt_d(parameter_number,parameter_value,DEBUGINFO_GEN)
#   define libc_malloc_trim(pad)                          libc__malloc_trim_d(pad,DEBUGINFO_GEN)
#   define libc_malloc_usable_size(ptr)                   libc__malloc_usable_size_d(ptr,DEBUGINFO_GEN)
#   define libc_memdup(ptr,n_bytes)                       libc__memdup_d(ptr,n_bytes,DEBUGINFO_GEN)
#   define libc_strdup(str)                               libc__strdup_d(str,DEBUGINFO_GEN)
#   define libc_strndup(str,max_chars)                    libc__strndup_d(str,max_chars,DEBUGINFO_GEN)
#   define libc_memcdup(ptr,needle,n_bytes)               libc__memcdup_d(ptr,needle,n_bytes,DEBUGINFO_GEN)
#   define libc_vstrdupf(format,args)                     libc__vstrdupf_d(format,args,DEBUGINFO_GEN)
#   define libc_strdupf(...)                              libc__strdupf_d(DEBUGINFO_GEN,__VA_ARGS__)
#endif
#else
#ifdef CONFIG_LIBC_ALLOW_EXTERN_MALLOC
#   define libc_malloc(n_bytes)                           malloc(n_bytes)
#   define libc_free(ptr)                                 free(ptr)
#   define libc_calloc(count,n_bytes)                     calloc(count,n_bytes)
#   define libc_realloc(ptr,n_bytes)                      realloc(ptr,n_bytes)
#   define libc_realloc_in_place(ptr,n_bytes)             realloc_in_place(ptr,n_bytes)
#   define libc_memalign(alignment,n_bytes)               memalign(alignment,n_bytes)
#   define libc_valloc(n_bytes)                           valloc(n_bytes)
#   define libc_pvalloc(n_bytes)                          pvalloc(n_bytes)
#   define libc_posix_memalign(pp,alignment,n_bytes)      posix_memalign(pp,alignment,n_bytes)
#   define libc_mallopt(parameter_number,parameter_value) mallopt(parameter_number,parameter_value)
#   define libc_malloc_trim(pad)                          malloc_trim(pad)
#   define libc_malloc_usable_size(ptr)                   malloc_usable_size(ptr)
#   define libc_memdup(ptr,n_bytes)                       memdup(ptr,n_bytes)
#   define libc_strdup(str)                               strdup(str)
#   define libc_strndup(str,max_chars)                    strndup(str,max_chars)
#   define libc_memcdup(ptr,needle,n_bytes)               memcdup(ptr,needle,n_bytes)
#   define libc_vstrdupf(format,args)                     vstrdupf(format,args)
#   define libc_strdupf(...)                              strdupf(__VA_ARGS__)
#else
#   define libc_malloc(n_bytes)                           __libc_malloc(n_bytes)
#   define libc_free(ptr)                                 __libc_free(ptr)
#   define libc_calloc(count,n_bytes)                     __libc_calloc(count,n_bytes)
#   define libc_realloc(ptr,n_bytes)                      __libc_realloc(ptr,n_bytes)
#   define libc_realloc_in_place(ptr,n_bytes)             __libc_realloc_in_place(ptr,n_bytes)
#   define libc_memalign(alignment,n_bytes)               __libc_memalign(alignment,n_bytes)
#   define libc_valloc(n_bytes)                           __libc_valloc(n_bytes)
#   define libc_pvalloc(n_bytes)                          __libc_pvalloc(n_bytes)
#   define libc_posix_memalign(pp,alignment,n_bytes)      __libc_posix_memalign(pp,alignment,n_bytes)
#   define libc_mallopt(parameter_number,parameter_value) __libc_mallopt(parameter_number,parameter_value)
#   define libc_malloc_trim(pad)                          __libc_malloc_trim(pad)
#   define libc_malloc_usable_size(ptr)                   __libc_malloc_usable_size(ptr)
#   define libc_memdup(ptr,n_bytes)                       libc_memdup(ptr,n_bytes)
#   define libc_strdup(str)                               libc_strdup(str)
#   define libc_strndup(str,max_chars)                    libc_strndup(str,max_chars)
#   define libc_memcdup(ptr,needle,n_bytes)               libc_memcdup(ptr,needle,n_bytes)
#   define libc_vstrdupf(format,args)                     libc_vstrdupf(format,args)
#   define libc_strdupf(...)                              libc_strdupf(__VA_ARGS__)
#endif
#endif

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

INTERN size_t LIBCCALL libc_dos_msize(void *mptr);
INTERN void *LIBCCALL libc_dos_expand(void *mptr, size_t size);
INTERN void *LIBCCALL libc_dos_recalloc(void *mptr, size_t count, size_t size);
INTERN size_t LIBCCALL libc_dos_aligned_msize(void *mptr, size_t align, size_t off);
INTERN void *LIBCCALL libc_dos_aligned_malloc(size_t size, size_t align);
INTERN void *LIBCCALL libc_dos_aligned_realloc(void *mptr, size_t size, size_t align);
INTERN void *LIBCCALL libc_dos_aligned_recalloc(void *mptr, size_t count, size_t size, size_t align);
INTERN void *LIBCCALL libc_dos_aligned_offset_malloc(size_t size, size_t align, size_t off);
INTERN void *LIBCCALL libc_dos_aligned_offset_realloc(void *mptr, size_t size, size_t align, size_t off);
INTERN void *LIBCCALL libc_dos_aligned_offset_recalloc(void *mptr, size_t count, size_t size, size_t align, size_t off);
INTERN void LIBCCALL libc_dos_aligned_free(void *mptr);
INTERN int LIBCCALL libc_dos_mall_validate(void); /* '_CrtCheckMemory' */
INTERN void LIBCCALL libc_dos_mall_enum(void (ATTR_CDECL *callback)(void *ptr, void *closure), void *closure); /* '_CrtDoForAllClientObjects' */

INTERN void *LIBCCALL libc_dos_malloc_dbg(size_t size, int btype, char const *file, int lno);
INTERN void *LIBCCALL libc_dos_calloc_dbg(size_t count, size_t size, int btype, char const *file, int lno);
INTERN void *LIBCCALL libc_dos_realloc_dbg(void *mptr, size_t size, int btype, char const *file, int lno);
INTERN void *LIBCCALL libc_dos_recalloc_dbg(void *mptr, size_t count, size_t size, int btype, char const *file, int lno);
INTERN void *LIBCCALL libc_dos_expand_dbg(void *mptr, size_t size, int btype, char const *file, int lno);
INTERN void LIBCCALL libc_dos_free_dbg(void *mptr, int btype);
INTERN size_t LIBCCALL libc_dos_msize_dbg(void *mptr, int btype);
INTERN size_t LIBCCALL libc_dos_aligned_msize_dbg(void *mptr, size_t align, size_t off);
INTERN void *LIBCCALL libc_dos_aligned_malloc_dbg(size_t size, size_t align, char const *file, int lno);
INTERN void *LIBCCALL libc_dos_aligned_realloc_dbg(void *mptr, size_t size, size_t align, char const *file, int lno);
INTERN void *LIBCCALL libc_dos_aligned_recalloc_dbg(void *mptr, size_t count, size_t size, size_t align, char const *file, int lno);
INTERN void *LIBCCALL libc_dos_aligned_offset_malloc_dbg(size_t size, size_t align, size_t off, char const *file, int lno);
INTERN void *LIBCCALL libc_dos_aligned_offset_realloc_dbg(void *mptr, size_t size, size_t align, size_t off, char const *file, int lno);
INTERN void *LIBCCALL libc_dos_aligned_offset_recalloc_dbg(void *mptr, size_t count, size_t size, size_t align, size_t off, char const *file, int lno);
INTERN void LIBCCALL libc_dos_aligned_free_dbg(void *mptr);
INTERN char *LIBCCALL libc_dos_strdup_dbg(char const *str, int btype, char const *file, int lno);
INTERN char *LIBCCALL libc_dos_tempnam_dbg(char const *dnam, char const *prefix, int btype, char const *file, int lno);
INTERN char *LIBCCALL libc_dos_fullpath_dbg(char *abspath, char const *path, size_t buflen, int btype, char const *file, int lno);
INTERN char *LIBCCALL libc_dos_getcwd_dbg(char *buf, int buflen, int btype, char const *file, int lno);
INTERN char *LIBCCALL libc_dos_getdcwd_dbg(int drive, char *buf, int buflen, int btype, char const *file, int lno);
INTERN char *LIBCCALL libc_dos_getdcwd_lk_dbg(int drive, char *buf, int buflen, int btype, char const *file, int lno);
INTERN errno_t LIBCCALL libc_dos_dupenv_s_dbg(char **pbuf, size_t *pbuflen, char const *varname, int btype, char const *file, int lno);
INTERN errno_t LIBCCALL libc_dos_wdupenv_s_dbg(char16_t **pbuf, size_t *pbuflen, char16_t const *varname, int btype, char const *file, int lno);
INTERN char16_t *LIBCCALL libc_dos_wcsdup_dbg(char16_t const *str, int btype, char const *file, int lno);
INTERN char16_t *LIBCCALL libc_dos_wtempnam_dbg(char16_t const *dnam, char16_t const *prefix, int btype, char const *file, int lno);
INTERN char16_t *LIBCCALL libc_dos_wfullpath_dbg(char16_t *abspath, char16_t const *path, size_t buflen, int btype, char const *file, int lno);
INTERN char16_t *LIBCCALL libc_dos_wgetcwd_dbg(char16_t *buf, int buflen, int btype, char const *file, int lno);
INTERN char16_t *LIBCCALL libc_dos_wgetdcwd_dbg(int drive, char16_t *buf, int buflen, int btype, char const *file, int lno);
INTERN char16_t *LIBCCALL libc_dos_wgetdcwd_lk_dbg(int drive, char16_t *buf, int buflen, int btype, char const *file, int lno);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END
#else /* !__KERNEL__ */
#include <malloc.h>
#define libc_malloc(n_bytes)                           malloc(n_bytes)
#define libc_free(ptr)                                 free(ptr)
#define libc_calloc(count,n_bytes)                     calloc(count,n_bytes)
#define libc_realloc(ptr,n_bytes)                      realloc(ptr,n_bytes)
#define libc_realloc_in_place(ptr,n_bytes)             realloc_in_place(ptr,n_bytes)
#define libc_memalign(alignment,n_bytes)               memalign(alignment,n_bytes)
#define libc_valloc(n_bytes)                           valloc(n_bytes)
#define libc_pvalloc(n_bytes)                          pvalloc(n_bytes)
#define libc_posix_memalign(pp,alignment,n_bytes)      posix_memalign(pp,alignment,n_bytes)
#define libc_mallopt(parameter_number,parameter_value) mallopt(parameter_number,parameter_value)
#define libc_malloc_trim(pad)                          malloc_trim(pad)
#define libc_malloc_usable_size(ptr)                   malloc_usable_size(ptr)
#define libc_memdup(ptr,n_bytes)                       memdup(ptr,n_bytes)
#endif /* __KERNEL__ */

#endif /* !GUARD_LIBS_LIBC_MALLOC_H */
