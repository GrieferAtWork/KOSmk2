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
#ifndef GUARD_LIBS_LIBC_MALLOC_C
#define GUARD_LIBS_LIBC_MALLOC_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1
#define __ptbwalker     ptbwalker

#include "libc.h"
#include "errno.h"
#include "format-printer.h"
#include "malloc.h"
#include "misc.h"
#include "file.h"
#include "stdio.h"
#include "string.h"
#include "string.h"
#include "sysconf.h"
#include "system.h"

#include <string.h>
#include <assert.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/debuginfo.h>
#include <hybrid/types.h>
#include <hybrid/limits.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#include <stdarg.h>



/* Force dlmalloc to use libc internal functions. */
#undef memcpy
#define memcpy             libc_memcpy
#define memset             libc_memset
#define memmove            libc_memmove
#define sysconf            libc_sysconf
#define mremap             libc_mremap
/* Invoke the system call directly; 'free()' must never modify 'errno'! */
#define munmap(p,s)       (E_ISERR(sys_munmap(p,s)) ? -1 : 0)
#define mmap               libc_mmap
#define malloc_getpagesize PAGESIZE
#define fprintf            libc_fprintf
#define sched_yield        libc_sched_yield
#undef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION SET_ERRNO(ENOMEM);

/* Use dlmalloc to define the low-level malloc API used as default for non-debug functions. */
#define USE_DL_PREFIX
#define DLMALLOC_EXPORT  INTDEF
#include "dlmalloc.c.inl"



#ifndef CONFIG_DEBUG_MALLOC

DECL_BEGIN

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
/* Directly link against dlmalloc, assuming caller-cleanup of superfluous function arguments. */
DEFINE_INTERN_ALIAS(libc__malloc_d,dlmalloc);
DEFINE_INTERN_ALIAS(libc__free_d,dlfree);
DEFINE_INTERN_ALIAS(libc__calloc_d,dlcalloc);
DEFINE_INTERN_ALIAS(libc__realloc_d,dlrealloc);
DEFINE_INTERN_ALIAS(libc__realloc_in_place_d,dlrealloc_in_place);
DEFINE_INTERN_ALIAS(libc__memalign_d,dlmemalign);
DEFINE_INTERN_ALIAS(libc__valloc_d,dlvalloc);
DEFINE_INTERN_ALIAS(libc__pvalloc_d,dlpvalloc);
DEFINE_INTERN_ALIAS(libc__posix_memalign_d,dlposix_memalign);
DEFINE_INTERN_ALIAS(libc__mallopt_d,dlmallopt);
DEFINE_INTERN_ALIAS(libc__malloc_trim_d,dlmalloc_trim);
DEFINE_INTERN_ALIAS(libc__malloc_usable_size_d,dlmalloc_usable_size);
#else
/* Call external malloc functions. */
INTERN SAFE void  *(LIBCCALL libc__malloc_d)(size_t n_bytes, DEBUGINFO_UNUSED) { return libc_malloc(n_bytes); }
INTERN SAFE void   (LIBCCALL libc__free_d)(void *__restrict ptr, DEBUGINFO_UNUSED) { return libc_free(ptr); }
INTERN SAFE void  *(LIBCCALL libc__calloc_d)(size_t count, size_t n_bytes, DEBUGINFO_UNUSED) { return libc_calloc(count,n_bytes); }
INTERN SAFE void  *(LIBCCALL libc__realloc_d)(void *__restrict ptr, size_t n_bytes, DEBUGINFO_UNUSED) { return libc_realloc(ptr,n_bytes); }
INTERN SAFE void  *(LIBCCALL libc__realloc_in_place_d)(void *__restrict ptr, size_t n_bytes, DEBUGINFO_UNUSED) { return libc_realloc_in_place(ptr,n_bytes); }
INTERN SAFE void  *(LIBCCALL libc__memalign_d)(size_t alignment, size_t n_bytes, DEBUGINFO_UNUSED) { return libc_memalign(alignment,n_bytes); }
INTERN SAFE void  *(LIBCCALL libc__valloc_d)(size_t n_bytes, DEBUGINFO_UNUSED) { return libc_valloc(n_bytes); }
INTERN SAFE void  *(LIBCCALL libc__pvalloc_d)(size_t n_bytes, DEBUGINFO_UNUSED) { return libc_pvalloc(n_bytes); }
INTERN SAFE int    (LIBCCALL libc__posix_memalign_d)(void **__restrict pp, size_t alignment, size_t n_bytes, DEBUGINFO_UNUSED) { return libc_posix_memalign(pp,alignment,n_bytes); }
INTERN SAFE int    (LIBCCALL libc__mallopt_d)(int parameter_number, int parameter_value, DEBUGINFO_UNUSED) { return libc_mallopt(parameter_number,parameter_value); }
INTERN SAFE int    (LIBCCALL libc__malloc_trim_d)(size_t pad, DEBUGINFO_UNUSED) { return libc_malloc_trim(pad); }
INTERN SAFE size_t (LIBCCALL libc__malloc_usable_size_d)(void *__restrict ptr, DEBUGINFO_UNUSED) { return libc_malloc_usable_size(ptr); }
#endif

#undef libc_memdup
#undef libc_strdup
#undef libc_strndup
#undef libc_strdupf
#undef libc_vstrdupf
#undef libc_memcdup
#undef libc__memdup_d
#undef libc__strdup_d
#undef libc__strndup_d
#undef libc__strdupf_d
#undef libc__vstrdupf_d
#undef libc__memcdup_d
#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_PUBLIC_ALIAS(libc__memdup_d,libc_memdup);
DEFINE_PUBLIC_ALIAS(libc__strdup_d,libc_strdup);
DEFINE_PUBLIC_ALIAS(libc__strndup_d,libc_strndup);
DEFINE_PUBLIC_ALIAS(libc__strdupf_d,libc_strdupf);
DEFINE_PUBLIC_ALIAS(libc__vstrdupf_d,libc_vstrdupf);
DEFINE_PUBLIC_ALIAS(libc__memcdup_d,libc_memcdup);
#else
INTERN SAFE void  *(LIBCCALL libc__memdup_d)(void const *__restrict ptr, size_t n_bytes, DEBUGINFO_UNUSED) { return libc_memdup(ptr,n_bytes); }
INTERN SAFE char  *(LIBCCALL libc__strdup_d)(char const *__restrict str, DEBUGINFO_UNUSED) { return libc_strdup(str); }
INTERN SAFE char  *(LIBCCALL libc__strndup_d)(char const *__restrict str, size_t max_chars, DEBUGINFO_UNUSED) { return libc_strndup(str,max_chars); }
INTERN SAFE char  *(ATTR_CDECL libc__strdupf_d)(DEBUGINFO_UNUSED, char const *__restrict format, ...) { char *result; va_list args; va_start(args,format); result = libc_vstrdupf(format,args); va_end(args); return result; }
INTERN SAFE char  *(LIBCCALL libc__vstrdupf_d)(char const *__restrict format, va_list args, DEBUGINFO_UNUSED) { return libc_vstrdupf(format,args); }
INTERN SAFE void  *(LIBCCALL libc__memcdup_d)(void const *__restrict ptr, int needle, size_t n_bytes, DEBUGINFO_UNUSED) { return libc_memcdup(ptr,needle,n_bytes); }
#endif

#ifndef __ptbwalker_defined
#define __ptbwalker_defined 1
typedef __SSIZE_TYPE__ (__LIBCCALL *__ptbwalker)(void const *__restrict __instruction_pointer,
                                                 void const *__restrict __frame_address,
                                                 size_t __frame_index, void *__closure);
#endif

/* MALL Api stubs. */
#if defined(CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP) && \
   (defined(__i386__) || defined(__x86_64__))
/* Use some assembly magic to reduce the memory footprint even further. */
GLOBAL_ASM(
L(INTERN_ENTRY(libc__mall_getattrib)                                                      )
L(INTERN_ENTRY(libc__mall_traceback)                                                      )
L(INTERN_ENTRY(libc__mall_enum)                                                           )
L(INTERN_ENTRY(libc__mall_getattrib_d)                                                    )
L(INTERN_ENTRY(libc__mall_traceback_d)                                                    )
L(INTERN_ENTRY(libc__mall_enum_d)                                                         )
L(    xor %eax, %eax                                                                      )
L(INTERN_ENTRY(libc__mall_printleaks)                                                     )
L(INTERN_ENTRY(libc__mall_validate)                                                       )
L(INTERN_ENTRY(libc__mall_printleaks_d)                                                   )
L(INTERN_ENTRY(libc__mall_validate_d)                                                     )
L(    ret                                                                                 )
L(SYM_END(libc__mall_validate_d)                                                          )
L(SYM_END(libc__mall_printleaks_d)                                                        )
L(SYM_END(libc__mall_validate)                                                            )
L(SYM_END(libc__mall_printleaks)                                                          )
L(SYM_END(libc__mall_enum_d)                                                              )
L(SYM_END(libc__mall_traceback_d)                                                         )
L(SYM_END(libc__mall_getattrib_d)                                                         )
L(SYM_END(libc__mall_enum)                                                                )
L(SYM_END(libc__mall_traceback)                                                           )
L(SYM_END(libc__mall_getattrib)                                                           )
L(INTERN_ENTRY(libc__mall_untrack)                                                        )
L(INTERN_ENTRY(libc__mall_nofree)                                                         )
L(INTERN_ENTRY(libc__mall_untrack_d)                                                      )
L(INTERN_ENTRY(libc__mall_nofree_d)                                                       )
L(    movl 4(%esp), %eax                                                                  )
L(    ret                                                                                 )
L(SYM_END(libc__mall_nofree_d)                                                            )
L(SYM_END(libc__mall_untrack_d)                                                           )
L(SYM_END(libc__mall_nofree)                                                              )
L(SYM_END(libc__mall_untrack)                                                             )
);
#else
INTERN void   *(LIBCCALL _mall_getattrib)(void *__restrict UNUSED(ptr), int UNUSED(attrib)) { return NULL; }
INTERN ssize_t (LIBCCALL _mall_traceback)(void *__restrict UNUSED(ptr), ptbwalker UNUSED(callback), void *UNUSED(closure)) { return 0; }
INTERN void    (LIBCCALL _mall_printleaks)(void) {}
INTERN void    (LIBCCALL _mall_validate)(void) {}
INTERN ssize_t (LIBCCALL _mall_enum)(void *UNUSED(checkpoint), ssize_t (*callback)(void *__restrict ptr,void *closure), void *UNUSED(closure)) { (void)callback; return 0; }
INTERN void   *(LIBCCALL _mall_untrack)(void *mallptr) { return mallptr; }
INTERN void   *(LIBCCALL _mall_nofree)(void *mallptr) { return mallptr; }
INTERN void   *(LIBCCALL _mall_getattrib_d)(void *__restrict UNUSED(ptr), int UNUSED(attrib), DEBUGINFO_UNUSED) { return NULL; }
INTERN ssize_t (LIBCCALL _mall_traceback_d)(void *__restrict UNUSED(ptr), ptbwalker UNUSED(callback), void *UNUSED(closure), DEBUGINFO_UNUSED) { return 0; }
INTERN void    (LIBCCALL _mall_printleaks_d)(DEBUGINFO_UNUSED) {}
INTERN void    (LIBCCALL _mall_validate_d)(DEBUGINFO_UNUSED) {}
INTERN ssize_t (LIBCCALL _mall_enum_d)(void *UNUSED(checkpoint), ssize_t (*callback)(void *__restrict ptr,void *closure), void *UNUSED(closure), DEBUGINFO_UNUSED) { (void)callback; return 0; }
INTERN void   *(LIBCCALL _mall_untrack_d)(void *mallptr, DEBUGINFO_UNUSED) { return mallptr; }
INTERN void   *(LIBCCALL _mall_nofree_d)(void *mallptr, DEBUGINFO_UNUSED) { return mallptr; }
#endif

/* Extended malloc functions. */
INTERN SAFE void *(LIBCCALL libc_memdup)(void const *__restrict ptr, size_t n_bytes) {
 void *result = libc_malloc(n_bytes);
 if (result) libc_memcpy(result,ptr,n_bytes);
 return result;
}

INTERN SAFE char *(LIBCCALL libc_strdup)(char const *__restrict str) {
 char *result; size_t len = libc_strlen(str)+1;
 result = (char *)libc_malloc(len*sizeof(char));
 if (result) libc_memcpy(result,str,len*sizeof(char));
 return result;
}
INTERN SAFE char *(LIBCCALL libc_strndup)(char const *__restrict str, size_t max_chars) {
 char *result;
 max_chars = libc_strnlen(str,max_chars);
 result = (char *)libc_malloc((max_chars+1)*sizeof(char));
 if (result) {
  libc_memcpy(result,str,max_chars*sizeof(char));
  result[max_chars] = '\0';
 }
 return result;
}
INTERN SAFE void *(LIBCCALL libc_memcdup)(void const *__restrict ptr, int needle, size_t n_bytes) {
 if (n_bytes) {
  void const *endaddr = libc_memchr(ptr,needle,n_bytes-1);
  if (endaddr) n_bytes = ((uintptr_t)endaddr-(uintptr_t)ptr)+1;
 }
 return libc_memdup(ptr,n_bytes);
}


struct strdup_formatdata { char *start,*iter,*end; };
PRIVATE NONNULL((1,3)) ssize_t
LIBCCALL strdupf_printer(char const *__restrict data, size_t datalen,
                         struct strdup_formatdata *__restrict fmt) {
 char *newiter;
 newiter = fmt->iter+datalen;
 if (newiter > fmt->end) {
  size_t newsize = (size_t)(fmt->end-fmt->start);
  assert(newsize);
  do newsize *= 2;
  while (fmt->start+newsize < newiter);
  /* Realloc the strdup string */
  newiter = (char *)libc_realloc(fmt->start,(newsize+1)*sizeof(char));
  if unlikely(!newiter) {
   /* If there isn't enough memory, retry
    * with a smaller buffer before giving up. */
   newsize = (fmt->end-fmt->start)+datalen;
   newiter = (char *)libc_realloc(fmt->start,(newsize+1)*sizeof(char));
   if unlikely(!newiter) return -1; /* Nothing we can do (out of memory...) */
  }
  fmt->iter = newiter+(fmt->iter-fmt->start);
  fmt->start = newiter;
  fmt->end = newiter+newsize;
 }
 libc_memcpy(fmt->iter,data,datalen);
 fmt->iter += datalen;
 return datalen;
}


INTERN SAFE char *(LIBCCALL libc_vstrdupf)(char const *__restrict format, va_list args) {
 struct strdup_formatdata data;
 /* Try to do a (admittedly very bad) prediction on the required size. */
 size_t format_length = (libc_strlen(format)*3)/2;
 data.start = (char *)libc_malloc((format_length+1)*sizeof(char));
 if unlikely(!data.start) {
  /* Failed to allocate initial buffer (try with a smaller one) */
  format_length = 1;
  data.start = (char *)libc_malloc(2*sizeof(char));
  if unlikely(!data.start) return NULL;
 }
 data.end = data.start+format_length;
 data.iter = data.start;
 if unlikely(libc_format_vprintf((pformatprinter)&strdupf_printer,
                                 &data,format,args) < 0) {
  libc_free(data.start); /* Out-of-memory */
  return NULL;
 }
 *data.iter = '\0';
 if likely(data.iter != data.end) {
  /* Try to realloc the string one last time to save up on memory */
  data.end = (char *)libc_realloc(data.start,((data.iter-data.start)+1)*sizeof(char));
  if likely(data.end) data.start = data.end;
 }
 return data.start;
}
INTERN SAFE char *(ATTR_CDECL libc_strdupf)(char const *__restrict format, ...) {
 char *result;
 va_list args;
 va_start(args,format);
 result = libc_vstrdupf(format,args);
 va_end(args);
 return result;
}

DECL_END
#else

#include <assert.h>
#include <hybrid/align.h>
#include <hybrid/atomic.h>
#include <hybrid/check.h>
#include <hybrid/limits.h>
#include <hybrid/list/list.h>
#include <hybrid/minmax.h>
#include <hybrid/sched/crit.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/traceback.h>
#include <stdbool.h>
#include <stdint.h>

DECL_BEGIN

#define CORE_MALLOC(n_bytes)                           dlmalloc(n_bytes)
#define CORE_FREE(ptr)                                 dlfree(ptr)
#define CORE_CALLOC(count,n_bytes)                     dlcalloc(count,n_bytes)
#define CORE_REALLOC(ptr,n_bytes)                      dlrealloc(ptr,n_bytes)
#define CORE_REALLOC_IN_PLACE(ptr,n_bytes)             dlrealloc_in_place(ptr,n_bytes)
#define CORE_MEMALIGN(alignment,n_bytes)               dlmemalign(alignment,n_bytes)
#define CORE_VALLOC(n_bytes)                           dlvalloc(n_bytes)
#define CORE_PVALLOC(n_bytes)                          dlpvalloc(n_bytes)
#define CORE_POSIX_MEMALIGN(pp,alignment,n_bytes)      dlposix_memalign(pp,alignment,n_bytes)
#define CORE_MALLOPT(parameter_number,parameter_value) dlmallopt(parameter_number,parameter_value)
#define CORE_MALLOC_TRIM(pad)                          dlmalloc_trim(pad)
#define CORE_MALLOC_USABLE_SIZE(ptr)                   dlmalloc_usable_size(ptr)

#define MALLDECL     PRIVATE __ATTR_NOINLINE

#define MALL_TBMIN        8 /* The min amount of traceback entries to track for any allocation. */
#define MALL_HEADERSIZE   8
#define MALL_FOOTERSIZE   8

#if 0
#define MALL_HEADERBYTE(i)   0x65
#define MALL_FOOTERBYTE(i)   0xB6
#else
PRIVATE byte_t mall_header_seed[4] = {0x65,0xB6,0xBD,0x5A};
PRIVATE byte_t mall_footer_seed[4] = {0xCF,0x6A,0xB7,0x97};
#define MALL_HEADERBYTE(i)   (mall_header_seed[(i) % 4]^(byte_t)((0xff >> (i) % 8)*(i))) /* Returns the i-th control byte for mall-headers. */
#define MALL_FOOTERBYTE(i)   (mall_footer_seed[(i) % 4]^(byte_t)((0xff >> (i) % 7)*((i)+1))) /* Returns the i-th control byte for mall-footers. */
#if 0
/* Put a twist on random number generation, making it impossible for
 * code to intentionally guess the correct sequence twice in a row. */
#define MALL_INITIALIZE() \
 (*(uint32_t *)mall_header_seed ^=  (uint32_t)time(NULL),\
  *(uint32_t *)mall_footer_seed ^= ~(uint32_t)time(NULL))
#endif
#endif


#define SIZEOF_DINFO  (__SIZEOF_INT__+2*__SIZEOF_POINTER__)

struct dinfo {
 char const *i_file;
 int         i_line;
 char const *i_func;
};

struct dsetup {
 struct dinfo s_info;   /*< Basic debug information for tracking. */
 unsigned int s_tbskip; /*< Amount of traceback entries to skip. */
};


#define MALL_STARTSUM  0xE77A61C3 /* <MAGIC> */

#define SIZEOF_MALLHEAD_UNALIGNED \
   (9+4*__SIZEOF_POINTER__+SIZEOF_DINFO+MALL_HEADERSIZE)
struct PACKED mallhead {
 u32              mh_chksum; /*< [const] Checksum of all mh_tbsize...mh_tail + mt_tb. */
 LIST_NODE(struct mallhead)
                  mh_chain;  /*< [lock(mall_lock)] Chain of all other MALL headers. */
 u16              mh_refcnt; /*< [lock(mall_lock)] Reference counter (required for handling free() while enumerating). */
 u16              mh_tbsize; /*< [const] Amount of entries in the 'mt_tb' field. */
 void            *mh_base;   /*< [const][1..1] Actually allocated pointer. */
 struct dinfo     mh_info;   /*< [lock(mall_lock)] Basic debug information for tracking. */
 struct malltail *mh_tail;   /*< [const] Pointer to the start of the mall tail. */
#define MALLFLAG_NONE      0x00
#define MALLFLAG_UNTRACKED 0x01 /*< The pointer was untracked. */
#define MALLFLAG_NOFREE    0x02 /*< The pointer must not be freed or reallocated. */
 u8               mh_flag;   /*< [lock(mall_lock)] Mall flags. */
#if SIZEOF_MALLHEAD_UNALIGNED % __MALL_MIN_ALIGNMENT
 /* make sure user-data is properly aligned by default. */
 byte_t           mh_padding[__MALL_MIN_ALIGNMENT-(SIZEOF_MALLHEAD_UNALIGNED % __MALL_MIN_ALIGNMENT)];
#endif
 byte_t           mh_data[MALL_HEADERSIZE];
 /* This is where userdata is located at. */
};
struct malltail {
 byte_t           mt_data[MALL_FOOTERSIZE];
 void            *mt_tb[1]; /*< [const][0..mh_tbsize] Malloc traceback instruction pointers. */
};
#define mall_istracked(head)      (!((head)->mh_flag&MALLFLAG_UNTRACKED))
#define mall_user2head(p)         ((struct mallhead *)(p)-1)
#define mall_head2user(p) (void *)((struct mallhead *)(p)+1)
#define mall_head2tail(p)         ((p)->mh_tail)
#define mall_usablesize(p)        ((uintptr_t)mall_head2tail(p)-(uintptr_t)mall_head2user(p))
#define mall_sizeof(n_bytes)      ((n_bytes)+(sizeof(struct mallhead)+MALL_FOOTERSIZE+MALL_TBMIN*sizeof(void *)))

struct frame {
 struct frame *f_caller;
 void         *f_return;
};

/* MALL Utility functions. */
MALLDECL u16 LIBCCALL mall_capturetb(struct malltail *__restrict tail, u16 tb_max, size_t skip);
PRIVATE  u32 LIBCCALL mall_chksum(void const *__restrict p, size_t s, u32 sum);
MALLDECL struct mallhead *LIBCCALL mall_loadptr(struct dsetup *__restrict setup, void *p);
MALLDECL ATTR_NORETURN void LIBCCALL mall_panic(struct dsetup *__restrict setup,
                                                struct mallhead *info_header,
                                                char const *__restrict format, ...);

/* MALL Allocation functions. */
MALLDECL SAFE void   LIBCCALL mall_free(struct dsetup *__restrict setup, void *__restrict ptr);
MALLDECL SAFE void  *LIBCCALL mall_realloc(struct dsetup *__restrict setup, void *__restrict ptr, size_t n_bytes);
MALLDECL SAFE void  *LIBCCALL mall_realloc_in_place(struct dsetup *__restrict setup, void *__restrict ptr, size_t n_bytes);
MALLDECL SAFE void  *LIBCCALL mall_memalign(struct dsetup *__restrict setup, size_t alignment, size_t n_bytes, bool cleared);
MALLDECL SAFE int    LIBCCALL mall_posix_memalign(struct dsetup *__restrict setup, void **__restrict pp, size_t alignment, size_t n_bytes);
MALLDECL SAFE int    LIBCCALL mall_mallopt(struct dsetup *__restrict setup, int parameter_number, int parameter_value);
MALLDECL SAFE int    LIBCCALL mall_malloc_trim(struct dsetup *__restrict setup, size_t pad);
MALLDECL SAFE size_t LIBCCALL mall_malloc_usable_size(struct dsetup *__restrict setup, void *__restrict ptr);
MALLDECL SAFE char  *LIBCCALL mall_strdup(struct dsetup *__restrict setup, char const *__restrict str);
MALLDECL SAFE char  *LIBCCALL mall_strndup(struct dsetup *__restrict setup, char const *__restrict str, size_t max_chars);
MALLDECL SAFE char  *LIBCCALL mall_vstrdupf(struct dsetup *__restrict setup, char const *__restrict format, va_list args);
MALLDECL SAFE void  *LIBCCALL mall_memdup(struct dsetup *__restrict setup, void const *__restrict ptr, size_t n_bytes);
MALLDECL SAFE void  *LIBCCALL mall_memcdup(struct dsetup *__restrict setup, void const *__restrict ptr, int needle, size_t n_bytes);

MALLDECL void    LIBCCALL mall_scramble(void *__restrict ptr, size_t size);
MALLDECL void   *LIBCCALL mall_getattrib(struct dsetup *__restrict setup, void *__restrict ptr, int attrib);
MALLDECL ssize_t LIBCCALL mall_traceback(struct dsetup *__restrict setup, void *__restrict ptr, ptbwalker callback, void *closure);
MALLDECL void   *LIBCCALL mall_untrack(struct dsetup *__restrict setup, void *__restrict ptr, u8 flags);
MALLDECL ssize_t LIBCCALL mall_enum(struct dsetup *__restrict setup,  void *checkpoint, ssize_t (LIBCCALL *callback)(void *__restrict ptr, void *closure), void *closure);
MALLDECL void    LIBCCALL mall_printleaks(struct dsetup *__restrict setup);
MALLDECL void    LIBCCALL mall_validate(struct dsetup *__restrict setup);

#define MALL_USE_REALLOC_INPLACE 0x00000001
#define MALL_NO_SCRABLE_ON_FREE  0x00000002
PRIVATE DEFINE_ATOMIC_RWLOCK      (mall_lock);            /*< Locking mechanism used by MALL. */
PRIVATE ATOMIC_DATA u32            mall_config    = 0;    /*< Generic MALL configuration (Set of 'MALL_*'). */
PRIVATE u32                        mall_check     = 1024; /*< Next time MALL is checked for inconsistencies. */
PRIVATE u32                        mall_checkfreq = 1024; /*< MALL consistency check frequency. */
PRIVATE LIST_HEAD(struct mallhead) mall_top       = NULL; /*< [0..1] Last allocated pointer. */
#define MALL_FREQ() (mall_checkfreq && --mall_check ? \
                    (mall_check = mall_checkfreq,\
                     ++setup->s_tbskip,mall_validate(setup),\
                     --setup->s_tbskip) : (void)0)


MALLDECL SAFE int LIBCCALL
mall_mallopt(struct dsetup *__restrict UNUSED(setup),
             int parameter_number, int parameter_value) {
 int result;
 switch (parameter_number) {
 { u32 flag;
   if (0) { case M_MALL_USE_INPLACE_REALLOC: flag = MALL_USE_REALLOC_INPLACE; }
   if (0) { case M_MALL_NO_SCRAMBLE_ON_FREE: flag = MALL_NO_SCRABLE_ON_FREE; }
        if (parameter_value < 0) result = !!(ATOMIC_LOAD(mall_config)&flag);
   else if (parameter_value > 0) result = !!(ATOMIC_FETCHOR(mall_config,flag)&flag);
   else                          result = !!(ATOMIC_FETCHAND(mall_config,~flag)&flag);
 } break;

 case M_MALL_CHECK_FREQUENCY:
  mall_check = (u32)parameter_value;
  result     = (int)ATOMIC_XCH(mall_checkfreq,(u32)parameter_value);
  break;

 default:
  result = CORE_MALLOPT(parameter_number,
                        parameter_value);
  break;
 }
 return result;
}

MALLDECL u16 LIBCCALL
mall_capturetb(struct malltail *__restrict tail,
               u16 tb_max, size_t skip) {
 /* TODO: Need local exception handlers for this... */
#if 1
 return 0;
#else
 struct frame *iter;
 void **dst,**begin,**end;
 __asm__ __volatile__("movl %%ebp, %0\n" : "=g" (iter));
 end = (dst = begin = tail->mt_tb)+tb_max;
 while (dst != end) {
  if (!OK_HOST_DATA(iter,sizeof(struct frame))) break;
  if (skip) --skip;
  else *dst++ = iter->f_return;
  iter = iter->f_caller;
 }
 return (u16)(dst-begin);
#endif
}
PRIVATE u32 LIBCCALL
mall_chksum(void const *__restrict p, size_t s, u32 sum) {
 byte_t *iter,*end;
 end = (iter = (byte_t *)p)+s;
 /* XXX: Better hashing? */
 for (; iter != end; ++iter) sum += *iter;
 return sum;
}
MALLDECL ATTR_NORETURN void LIBCCALL
mall_panic(struct dsetup *__restrict setup,
           struct mallhead *info_header,
           char const *__restrict format, ...) {
 va_list args;
 setup->s_tbskip += 3;
 va_start(args,format);
 debug_vprintf(format,args);
 debug_printf("\n");
 va_end(args);
 debug_printf("%s(%d) : %s : See reference to caller location\n",
                    setup->s_info.i_file,setup->s_info.i_line,
                    setup->s_info.i_func);
 debug_tbprint(setup->s_tbskip);
 if (info_header) {
  void **iter,**end; size_t pos;
  debug_printf("%s(%d) : %s : See reference to associated mall pointer\n",
                     info_header->mh_info.i_file,
                     info_header->mh_info.i_line,
                     info_header->mh_info.i_func);
  end = (iter = info_header->mh_tail->mt_tb)+info_header->mh_tbsize;
  pos = 0;
  while (iter != end) {
   debug_printf("#!$ addr2line(%p) '{file}({line}) : {func} : [%Ix] : %p'\n",
                     (uintptr_t)*iter-1,pos,*iter);
   ++iter,++pos;
  }
 }
 __afail("MALL PANIC",__DEBUGINFO_NUL);
}
MALLDECL struct mallhead *LIBCCALL
mall_loadptr(struct dsetup *__restrict setup, void *p) {
 struct mallhead *head = mall_user2head(p);
 struct malltail *tail;
 unsigned int i; u32 chksum;
 /* Validate generic constraints. */
 if (!OK_HOST_DATA(head,sizeof(struct mallhead))) mall_panic(setup,NULL,"Illegal pointer: %p",p);
 if (!ATOMIC_READ(head->mh_refcnt))               mall_panic(setup,NULL,"Invalid reference counter 0 at %p, offset %Id in %p",&head->mh_refcnt,(intptr_t)&head->mh_refcnt-(intptr_t)p,p);
 if (head->mh_base > (void *)head)                mall_panic(setup,NULL,"Invalid base address %p > %p at %p, offset %Id in %p",head->mh_base,head,&head->mh_base,(intptr_t)&head->mh_base-(intptr_t)p,p);
 if ((void *)(tail = head->mh_tail) <= p)         mall_panic(setup,NULL,"Invalid tail address %p <= %p at %p, offset %Id in %p",tail,p,&head->mh_tail,(intptr_t)&head->mh_tail-(intptr_t)p,p);
 /* Validate header bytes */
 for (i = 0; i < MALL_HEADERSIZE; ++i) {
  if (head->mh_data[i] != MALL_HEADERBYTE(i)) {
   mall_panic(setup,head,"Header corruption (%I8x != %I8x) at %p of %p (at offset %Id)",
              head->mh_data[i],MALL_HEADERBYTE(i),&head->mh_data[i],p,
             (intptr_t)&head->mh_data[i]-(intptr_t)p);
  }
 }
 /* Validate footer bytes */
 for (i = 0; i < MALL_FOOTERSIZE; ++i) {
  if (tail->mt_data[i] != MALL_FOOTERBYTE(i)) {
   mall_panic(setup,head,"Footer corruption (%I8x != %I8x) at %p of %p (at offset %Id; overflow by %d)",
              tail->mt_data[i],MALL_FOOTERBYTE(i),&tail->mt_data[i],p,
             (intptr_t)&tail->mt_data[i]-(intptr_t)p,i);
  }
 }
 /* Validate mall checksum */
 chksum = mall_chksum(&head->mh_tbsize,
                      offsetof(struct mallhead,mh_data)-
                      offsetof(struct mallhead,mh_tbsize),MALL_STARTSUM);
 chksum = mall_chksum(&tail->mt_tb,head->mh_tbsize*sizeof(void *),chksum);
 if (chksum != head->mh_chksum) {
  mall_panic(setup,head,"Invalid checksum (expected %I32x, got %I32x at %p, offset %Id) in %p",
             chksum,head->mh_chksum,&head->mh_chksum,
            (intptr_t)&head->mh_chksum-(intptr_t)p,p);
 }
 /* Validate mall linkage */
 if (atomic_rwlock_tryread(&mall_lock)) {
  if (mall_istracked(head)) {
   struct mallhead *linked_self;
   if (!OK_HOST_DATA(head->mh_chain.le_pself,sizeof(void *))) {
    mall_panic(setup,head,"Broken chain self-pointer (%p at &p, offset %Id) in %p",
               head->mh_chain.le_pself,&head->mh_chain.le_pself,
              (intptr_t)&head->mh_chain.le_pself-(intptr_t)p,p);
   }
   linked_self = *head->mh_chain.le_pself;
   if (head != linked_self) {
    mall_panic(setup,head,"Incorrect self-pointer (expected %p, got %p in at %p at offset %Id) in %p",
               head,linked_self,head->mh_chain.le_pself,
              (intptr_t)&head->mh_chain.le_pself-(intptr_t)p,p);
   }
   if (head->mh_chain.le_next) {
    struct mallhead *next = head->mh_chain.le_next;
    if (!OK_HOST_DATA(next,sizeof(struct mallhead))) {
     mall_panic(setup,head,"Broken chain next-pointer (%p at %p, offset %Id) in %p",
                next,&head->mh_chain.le_next,
               (intptr_t)&head->mh_chain.le_next-(intptr_t)p,p);
    }
    if (&head->mh_chain.le_next != next->mh_chain.le_pself) {
     mall_panic(setup,head,"Broken chain next-self-pointer (expected %p, got %p) following %p",
                &head->mh_chain.le_next,next->mh_chain.le_pself,p);
    }
   }
  }
  atomic_rwlock_endread(&mall_lock);
 }
 return head;
}


MALLDECL void *LIBCCALL
mall_memalign(struct dsetup *__restrict setup,
              size_t alignment, size_t n_bytes,
              bool cleared) {
 struct mallhead *head;
 struct malltail *tail; u32 chksum;
 void *base; unsigned int i;
 size_t usable_tb_size;
 MALL_FREQ();
 if (alignment <= MALLOC_ALIGNMENT) {
  base      = CORE_MALLOC(mall_sizeof(n_bytes));
  alignment = MALLOC_ALIGNMENT;
 } else {
  base = CORE_MALLOC(mall_sizeof(alignment+n_bytes));
 }
 if unlikely(!base) return NULL;
 /* Figure out where everything will go. */
 head = ((struct mallhead *)CEIL_ALIGN((uintptr_t)base+sizeof(struct mallhead),alignment))-1;
 tail = (struct malltail *)((uintptr_t)(head+1)+n_bytes);
 head->mh_base = base;
 head->mh_tail = tail;
 head->mh_flag = MALLFLAG_NONE;
 for (i = 0; i < MALL_HEADERSIZE; ++i)
      head->mh_data[i] = MALL_HEADERBYTE(i);
 for (i = 0; i < MALL_FOOTERSIZE; ++i)
      tail->mt_data[i] = MALL_FOOTERBYTE(i);
 /* Figure out how long the traceback can be (we re-use overallocated memory for this) */
 usable_tb_size = CORE_MALLOC_USABLE_SIZE(base);
 assert((uintptr_t)tail->mt_tb >= (uintptr_t)head);
 assert(usable_tb_size >= ((uintptr_t)tail->mt_tb-(uintptr_t)head));
 usable_tb_size -= (uintptr_t)tail->mt_tb-(uintptr_t)base;
 assertf(usable_tb_size >= MALL_TBMIN*sizeof(void *),
         "usable_tb_size = %Iu",usable_tb_size);
 head->mh_tbsize = mall_capturetb(tail,(u16)(usable_tb_size/sizeof(void *)),setup->s_tbskip+2);
 head->mh_refcnt = 1;
 head->mh_info   = setup->s_info;
 /* Generate the checksum. */
 chksum = mall_chksum(&head->mh_tbsize,
                      offsetof(struct mallhead,mh_data)-
                      offsetof(struct mallhead,mh_tbsize),MALL_STARTSUM);
 chksum = mall_chksum(&tail->mt_tb,head->mh_tbsize*sizeof(void *),chksum);
 head->mh_chksum = chksum;
 libc_memset(head+1,cleared ? 0 : CRTDBG_INIT_MALLOC,n_bytes);
 atomic_rwlock_write(&mall_lock);
 LIST_INSERT(mall_top,head,mh_chain);
 atomic_rwlock_endwrite(&mall_lock);
 return mall_head2user(head);
}
PRIVATE void LIBCCALL
mall_reload_chksum(struct mallhead *__restrict self) {
 u32 chksum;
 chksum = mall_chksum(&self->mh_tbsize,
                      offsetof(struct mallhead,mh_data)-
                      offsetof(struct mallhead,mh_tbsize),MALL_STARTSUM);
 chksum = mall_chksum(&self->mh_tail->mt_tb,self->mh_tbsize*sizeof(void *),chksum);
 self->mh_chksum = chksum;
}
MALLDECL void *LIBCCALL
mall_realloc_in_place(struct dsetup *__restrict setup,
                      void *__restrict ptr, size_t n_bytes) {
 void *base;
 struct mallhead *head;
 struct malltail *tail;
 struct malltail *new_tail;
 size_t old_usersize;
 size_t old_hostsize;
 size_t new_hostsize;
 size_t tail_size;
 MALL_FREQ();
 if unlikely(!ptr) return NULL;
 head          =  mall_loadptr(setup,ptr);
 base          =  head->mh_base;
 tail          =  head->mh_tail;
 old_usersize  = (uintptr_t)tail-(uintptr_t)(head+1);
 old_hostsize  = (uintptr_t)tail-(uintptr_t)base;
 tail_size     =  MALL_FOOTERSIZE+head->mh_tbsize*sizeof(void *);
 old_hostsize +=  tail_size;
 new_hostsize  = (old_hostsize-old_usersize)+n_bytes;
 new_tail      = (struct malltail *)((uintptr_t)(head+1)+n_bytes);
 if (n_bytes < old_usersize) {
  /* Reduce allocated size. */
  libc_memmove(new_tail,tail,tail_size);
  base = CORE_REALLOC_IN_PLACE(head->mh_base,new_hostsize);
  assertf(base == head->mh_base,"Why did a size reduction fail?");
 } else {
  /* Increase allocated size. */
  base = CORE_REALLOC_IN_PLACE(head->mh_base,new_hostsize);
  if unlikely(!base) return NULL;
  assert(base == head->mh_base);
  libc_memmove(new_tail,tail,tail_size);
  /* Pre-initialize newly allocated memory with debug values. */
  libc_memset(tail,CRTDBG_INIT_MALLOC,n_bytes-old_usersize);
 }
 head->mh_tail = new_tail;
 /* Re-calculate the checksum. */
 mall_reload_chksum(head);
 return ptr;
}
MALLDECL void LIBCCALL
mall_free(struct dsetup *__restrict setup,
          void *__restrict ptr) {
 struct mallhead *head;
 MALL_FREQ();
 if unlikely(!ptr) return;
 head = mall_loadptr(setup,ptr);
 atomic_rwlock_write(&mall_lock);
 /* Make sure that the given pointer isn't marked as NOFREE */
 if unlikely(head->mh_flag&MALLFLAG_NOFREE) {
  atomic_rwlock_endwrite(&mall_lock);
  mall_panic(setup,head,"Attempted to free() protected pointer %p marked as NOFREE\n",ptr);
 }
 --head->mh_refcnt;
 if (head->mh_refcnt) {
  atomic_rwlock_endwrite(&mall_lock);
 } else {
  if (mall_istracked(head))
      LIST_REMOVE(head,mh_chain);
  atomic_rwlock_endwrite(&mall_lock);
  mall_scramble(ptr,mall_usablesize(head));
  CORE_FREE(head->mh_base);
 }
}
MALLDECL void *LIBCCALL
mall_untrack(struct dsetup *__restrict setup,
             void *__restrict ptr, u8 flags) {
 struct mallhead *head;
 if (ptr) {
  head = mall_loadptr(setup,ptr);
  atomic_rwlock_read(&mall_lock);
  /* Check if the pointer is actually tracked. */
  if (mall_istracked(head)) {
   if (!atomic_rwlock_upgrade(&mall_lock)) {
    if unlikely(!mall_istracked(head)) goto wend;
   }
   LIST_REMOVE(head,mh_chain);
   head->mh_flag |= flags;
   /* Re-calculate the checksum. */
   mall_reload_chksum(head);
wend:
   atomic_rwlock_endwrite(&mall_lock);
  } else {
   atomic_rwlock_endread(&mall_lock);
  }
 }
 return ptr;
}
MALLDECL size_t LIBCCALL
mall_malloc_usable_size(struct dsetup *__restrict setup,
                        void *__restrict ptr) {
 struct mallhead *head;
 if unlikely(!ptr) return 0;
 head = mall_loadptr(setup,ptr);
 return mall_usablesize(head);
}



MALLDECL void *LIBCCALL
mall_realloc(struct dsetup *__restrict setup,
             void *__restrict ptr, size_t n_bytes) {
 void *result;
 ++setup->s_tbskip;
 if (ATOMIC_READ(mall_config)&MALL_USE_REALLOC_INPLACE) {
  result = mall_realloc_in_place(setup,ptr,n_bytes);
  if (result) return result;
 }
 result = mall_memalign(setup,MALLOC_ALIGNMENT,n_bytes,false);
 if (result) {
  size_t oldsize = mall_malloc_usable_size(setup,ptr);
  libc_memcpy(result,ptr,MIN(oldsize,n_bytes));
  ++setup->s_tbskip;
  mall_free(setup,ptr);
 }
 return result;
}
MALLDECL int LIBCCALL
mall_posix_memalign(struct dsetup *__restrict setup,
                    void **__restrict pp,
                    size_t alignment, size_t n_bytes) {
 void *result = NULL;
 CHECK_HOST_DOBJ(pp);
 ++setup->s_tbskip;
 if (alignment == MALLOC_ALIGNMENT)
  result = mall_memalign(setup,alignment,n_bytes,false);
 else {
  size_t d = alignment / sizeof(void*);
  size_t r = alignment % sizeof(void*);
  if (r != 0 || d == 0 || (d & (d-SIZE_T_ONE)) != 0) return EINVAL;
  else if (n_bytes <= MAX_REQUEST - alignment) {
   if (alignment < MIN_CHUNK_SIZE)
       alignment = MIN_CHUNK_SIZE;
   result = mall_memalign(setup,alignment,n_bytes,false);
  }
 }
 if (!result) return ENOMEM;
 *pp = result;
 return 0;
}
MALLDECL int LIBCCALL
mall_malloc_trim(struct dsetup *__restrict UNUSED(setup), size_t pad) {
 return dlmalloc_trim(pad);
}
MALLDECL char *LIBCCALL
mall_strdup(struct dsetup *__restrict setup,
            char const *__restrict str) {
 char *result;
 size_t len = libc_strlen(str);
 ++setup->s_tbskip;
 result = (char *)mall_memalign(setup,MALLOC_ALIGNMENT,
                               (len+1)*sizeof(char),false);
 if (result) {
  libc_memcpy(result,str,len*sizeof(char));
  result[len] = '\0';
 }
 return result;
}
MALLDECL char *LIBCCALL
mall_strndup(struct dsetup *__restrict setup,
             char const *__restrict str, size_t max_chars) {
 char *result;
 size_t len = libc_strnlen(str,max_chars);
 ++setup->s_tbskip;
 result = (char *)mall_memalign(setup,MALLOC_ALIGNMENT,
                               (len+1)*sizeof(char),false);
 if (result) {
  libc_memcpy(result,str,len*sizeof(char));
  result[len] = '\0';
 }
 return result;
}

MALLDECL char *LIBCCALL
mall_vstrdupf(struct dsetup *__restrict setup,
              char const *__restrict format, va_list args) {
 /* Minimal implementation (Not meant for speed) */
 va_list args_copy; size_t result_size; char *result;
 va_copy(args_copy,args);
 result_size = (libc_vsnprintf(NULL,0,format,args_copy)+1)*sizeof(char);
 va_end(args_copy);
 ++setup->s_tbskip;
 result = (char *)mall_memalign(setup,MALLOC_ALIGNMENT,result_size,false);
 if (result) libc_vsnprintf(result,result_size,format,args);
 return result;
}
MALLDECL void *LIBCCALL
mall_memdup(struct dsetup *__restrict setup,
            void const *__restrict ptr, size_t n_bytes) {
 void *result;
 ++setup->s_tbskip;
 result = mall_memalign(setup,MALLOC_ALIGNMENT,
                        n_bytes*sizeof(char),false);
 if (result) libc_memcpy(result,ptr,n_bytes);
 return result;
}
MALLDECL void *LIBCCALL
mall_memcdup(struct dsetup *__restrict setup,
             void const *__restrict ptr, int needle, size_t n_bytes) {
 if (n_bytes) {
  void const *endaddr = libc_memchr(ptr,needle,n_bytes-1);
  if (endaddr) n_bytes = ((uintptr_t)endaddr-(uintptr_t)ptr)+1;
 }
 ++setup->s_tbskip;
 return mall_memdup(setup,ptr,n_bytes);
}




MALLDECL void LIBCCALL
mall_scramble(void *__restrict ptr, size_t size) {
 if (!(ATOMIC_READ(mall_config)&MALL_NO_SCRABLE_ON_FREE)) {
  byte_t *iter,*end;
  end = (iter = (byte_t *)ptr)+size;
  for (; iter != end; ++iter) *iter ^= 0xff;
 }
}
MALLDECL void *LIBCCALL
mall_getattrib(struct dsetup *__restrict setup,
               void *__restrict ptr, int attrib) {
 struct mallhead *head;
 void *result;
 if unlikely(!ptr) return NULL;
 head = mall_loadptr(setup,ptr);
 switch (attrib) {
 case __MALL_ATTRIB_FILE: result = (void *)head->mh_info.i_file; break;
 case __MALL_ATTRIB_LINE: result = (void *)(uintptr_t)head->mh_info.i_line; break;
 case __MALL_ATTRIB_FUNC: result = (void *)head->mh_info.i_func; break;
 case __MALL_ATTRIB_SIZE: result = (void *)mall_usablesize(head); break;
 default                : result = NULL; break;
 }
 return result;
}

MALLDECL ssize_t LIBCCALL
mall_traceback(struct dsetup *__restrict setup,
               void *__restrict ptr, ptbwalker callback,
               void *closure) {
 struct mallhead *head;
 void **iter,*end;
 size_t pos = 0; ssize_t result = 0,temp;
 if unlikely(!ptr) return 0;
 head = mall_loadptr(setup,ptr);
 end = (iter = head->mh_tail->mt_tb)+head->mh_tbsize;
 while (iter != end) {
  temp = (*callback)(*iter,NULL,pos,closure);
  if unlikely(temp < 0) return temp;
  result += temp;
  ++iter,++pos;
 }
 return result;
}
MALLDECL ssize_t LIBCCALL
mall_enum(struct dsetup *__restrict setup, 
          void *checkpoint, ssize_t (LIBCCALL *callback)(void *__restrict ptr, void *closure),
          void *closure) {
 struct mallhead *blocks,*iter,*next,*oldtop;
 ssize_t result = 0,temp;
 if unlikely(!callback) return 0;
 CRIT_BEGIN();
 atomic_rwlock_write(&mall_lock);
 oldtop = mall_top,mall_top = NULL;
 if (checkpoint) {
  struct mallhead *checkpoint_head;
  checkpoint_head = mall_loadptr(setup,checkpoint);
  blocks = checkpoint_head->mh_chain.le_next;
 } else {
  /* Enumerate all blocks. */
  blocks = oldtop;
 }
 iter = blocks;
 while (iter) {
  /* Perform a full validation of 'iter'. */
  mall_loadptr(setup,mall_head2user(iter));
  next = iter->mh_chain.le_next;
  ++iter->mh_refcnt;
  atomic_rwlock_endwrite(&mall_lock);
  /* Run the user-provided callback on the given block. */
  temp = (*callback)(mall_head2user(iter),closure);
  atomic_rwlock_write(&mall_lock);
  if (!--iter->mh_refcnt) {
   if (iter->mh_chain.le_next != next) break;
   LIST_REMOVE(iter,mh_chain);
   mall_scramble(mall_head2user(iter),
                 mall_usablesize(iter));
   CORE_FREE(iter->mh_base);
  }
  /* Stop iterating if the user's callback returned negative. */
  if unlikely(temp < 0) { result = temp; break; }
  result += temp;
  iter = next;
 }
 if (oldtop) {
  /* Restore the popped list of blocks. */
  if (mall_top) {
   /* Prepend the popped list of blocks before the new list. */
   iter = mall_top;
   while (iter->mh_chain.le_next)
          iter = iter->mh_chain.le_next;
   iter->mh_chain.le_next    = oldtop;
   oldtop->mh_chain.le_pself = &iter->mh_chain.le_next;
  } else {
   mall_top                  = oldtop;
   oldtop->mh_chain.le_pself = &mall_top;
  }
 }
 atomic_rwlock_endwrite(&mall_lock);
 CRIT_END();
 return result;
}


MALLDECL void LIBCCALL
mall_validate(struct dsetup *__restrict setup) {
 struct mallhead *iter;
 CRIT_BEGIN();
 atomic_rwlock_read(&mall_lock);
 iter = mall_top;
 while (iter) {
  /* Perform a full validation of 'iter'. */
  mall_loadptr(setup,mall_head2user(iter));
  iter = iter->mh_chain.le_next;
 }
 atomic_rwlock_endread(&mall_lock);
 CRIT_END();
}

PRIVATE ssize_t LIBCCALL
mall_printleak(struct mallhead const *__restrict head,
               char const *reason) {
 void **iter,**end;
 size_t pos = 0;
 debug_printf("##################################################\n"
                    "%s(%d) : %s : %s %Iu bytes at %p\n",
                    head->mh_info.i_file,head->mh_info.i_line,
                    head->mh_info.i_func,reason,mall_usablesize(head),
                    mall_head2user(head));
 end = (iter = head->mh_tail->mt_tb)+head->mh_tbsize;
 while (iter != end) {
  debug_printf("#!$ addr2line(%p) '{file}({line}) : {func} : [%Ix] : %p'\n",
                    (uintptr_t)*iter-1,pos,*iter);
  ++iter,++pos;
 }
 return 0;
}
PRIVATE ssize_t LIBCCALL
printleaks_callback(void *__restrict ptr,
                    void *UNUSED(closure)) {
 return mall_printleak(mall_user2head(ptr),"Leaked");
}
MALLDECL void LIBCCALL
mall_printleaks(struct dsetup *__restrict setup) {
 ++setup->s_tbskip;
 mall_enum(setup,NULL,&printleaks_callback,NULL);
}

#ifdef __KERNEL__
#define DEF_SETUP(name) struct dsetup name = {{NULL,0,NULL,NULL},1}
#else
#define DEF_SETUP(name) struct dsetup name = {{NULL,0,NULL},1}
#endif
INTERN void  *(LIBCCALL libc_malloc)(size_t n_bytes)                                                 { DEF_SETUP(setup); return mall_memalign(&setup,MALLOC_ALIGNMENT,n_bytes,false); }
INTERN void   (LIBCCALL libc_free)(void *__restrict ptr)                                             { DEF_SETUP(setup); return mall_free(&setup,ptr); }
INTERN void  *(LIBCCALL libc_calloc)(size_t count, size_t n_bytes)                                   { DEF_SETUP(setup); return mall_memalign(&setup,MALLOC_ALIGNMENT,count*n_bytes,true); }
INTERN void  *(LIBCCALL libc_realloc)(void *__restrict ptr, size_t n_bytes)                          { DEF_SETUP(setup); return mall_realloc(&setup,ptr,n_bytes); }
INTERN void  *(LIBCCALL libc_realloc_in_place)(void *__restrict ptr, size_t n_bytes)                 { DEF_SETUP(setup); return mall_realloc_in_place(&setup,ptr,n_bytes); }
INTERN void  *(LIBCCALL libc_memalign)(size_t alignment, size_t n_bytes)                             { DEF_SETUP(setup); return mall_memalign(&setup,alignment,n_bytes,false); }
INTERN void  *(LIBCCALL libc_valloc)(size_t n_bytes)                                                 { DEF_SETUP(setup); return mall_memalign(&setup,PAGESIZE,n_bytes,false); }
INTERN void  *(LIBCCALL libc_pvalloc)(size_t n_bytes)                                                { DEF_SETUP(setup); return mall_memalign(&setup,PAGESIZE,(n_bytes+PAGESIZE-1)&~(PAGESIZE-1),false); }
INTERN int    (LIBCCALL libc_posix_memalign)(void **__restrict pp, size_t alignment, size_t n_bytes) { DEF_SETUP(setup); return mall_posix_memalign(&setup,pp,alignment,n_bytes); }
INTERN int    (LIBCCALL libc_mallopt)(int parameter_number, int parameter_value)                     { DEF_SETUP(setup); return mall_mallopt(&setup,parameter_number,parameter_value); }
INTERN int    (LIBCCALL libc_malloc_trim)(size_t pad)                                                { DEF_SETUP(setup); return mall_malloc_trim(&setup,pad); }
INTERN size_t (LIBCCALL libc_malloc_usable_size)(void *__restrict ptr)                               { DEF_SETUP(setup); return mall_malloc_usable_size(&setup,ptr); }
INTERN void  *(LIBCCALL libc_memdup)(void const *__restrict ptr, size_t n_bytes)                     { DEF_SETUP(setup); return mall_memdup(&setup,ptr,n_bytes); }
INTERN void  *(LIBCCALL libc__mall_getattrib)(void *__restrict ptr, int attrib)                      { DEF_SETUP(setup); return mall_getattrib(&setup,ptr,attrib); }
INTERN ssize_t(LIBCCALL libc__mall_traceback)(void *__restrict ptr, ptbwalker callback, void *closure){DEF_SETUP(setup); return mall_traceback(&setup,ptr,callback,closure); }
INTERN void   (LIBCCALL libc__mall_printleaks)(void)                                                 { DEF_SETUP(setup);        mall_printleaks(&setup); }
INTERN void   (LIBCCALL libc__mall_validate)(void)                                                   { DEF_SETUP(setup);        mall_validate(&setup); }
INTERN ssize_t(LIBCCALL libc__mall_enum)(void *checkpoint, ssize_t(LIBCCALL *callback)(void *__restrict ptr, void *closure),
                                         void *closure)                                              { DEF_SETUP(setup); return mall_enum(&setup ,checkpoint,callback,closure); }
INTERN void  *(LIBCCALL libc__mall_untrack)(void *mallptr)                                           { DEF_SETUP(setup); return mall_untrack(&setup,mallptr,MALLFLAG_UNTRACKED); }
INTERN void  *(LIBCCALL libc__mall_nofree)(void *mallptr)                                            { DEF_SETUP(setup); return mall_untrack(&setup,mallptr,MALLFLAG_UNTRACKED|MALLFLAG_NOFREE); }
#ifndef __KERNEL__
INTERN char  *(LIBCCALL libc_strdup)(char const *__restrict str)                                     { DEF_SETUP(setup); return mall_strdup(&setup,str); }
INTERN char  *(LIBCCALL libc_strndup)(char const *__restrict str, size_t max_chars)                  { DEF_SETUP(setup); return mall_strndup(&setup,str,max_chars); }
INTERN char  *(LIBCCALL libc_vstrdupf)(char const *__restrict format, va_list args)                  { DEF_SETUP(setup); return mall_vstrdupf(&setup,format,args); }
INTERN void  *(LIBCCALL libc_memcdup)(void const *__restrict ptr, int needle, size_t n_bytes)        { DEF_SETUP(setup); return mall_memcdup(&setup,ptr,needle,n_bytes); }
INTERN char  *(ATTR_CDECL libc_strdupf)(char const *__restrict format, ...) {
 char *result;
 va_list args;
 DEF_SETUP(setup);
 va_start(args,format);
 result = mall_vstrdupf(&setup,format,args);
 va_end(args);
 return result;
}
#endif /* !__KERNEL__ */
#undef DEF_SETUP

#ifdef __KERNEL__
#define DEF_SETUP(name) struct dsetup name = {{__file,__line,__func,__inst},1}
#else
#define DEF_SETUP(name) struct dsetup name = {{__file,__line,__func},1}
#endif
INTERN void  *(LIBCCALL libc__malloc_d)(size_t n_bytes, DEBUGINFO)                                                 { DEF_SETUP(setup); return mall_memalign(&setup,MALLOC_ALIGNMENT,n_bytes,false); }
INTERN void   (LIBCCALL libc__free_d)(void *__restrict ptr, DEBUGINFO)                                             { DEF_SETUP(setup); return mall_free(&setup,ptr); }
INTERN void  *(LIBCCALL libc__calloc_d)(size_t count, size_t n_bytes, DEBUGINFO)                                   { DEF_SETUP(setup); return mall_memalign(&setup,MALLOC_ALIGNMENT,count*n_bytes,true); }
INTERN void  *(LIBCCALL libc__realloc_d)(void *__restrict ptr, size_t n_bytes, DEBUGINFO)                          { DEF_SETUP(setup); return mall_realloc(&setup,ptr,n_bytes); }
INTERN void  *(LIBCCALL libc__realloc_in_place_d)(void *__restrict ptr, size_t n_bytes, DEBUGINFO)                 { DEF_SETUP(setup); return mall_realloc_in_place(&setup,ptr,n_bytes); }
INTERN void  *(LIBCCALL libc__memalign_d)(size_t alignment, size_t n_bytes, DEBUGINFO)                             { DEF_SETUP(setup); return mall_memalign(&setup,alignment,n_bytes,false); }
INTERN void  *(LIBCCALL libc__valloc_d)(size_t n_bytes, DEBUGINFO)                                                 { DEF_SETUP(setup); return mall_memalign(&setup,PAGESIZE,n_bytes,false); }
INTERN void  *(LIBCCALL libc__pvalloc_d)(size_t n_bytes, DEBUGINFO)                                                { DEF_SETUP(setup); return mall_memalign(&setup,PAGESIZE,(n_bytes+PAGESIZE-1)&~(PAGESIZE-1),false); }
INTERN int    (LIBCCALL libc__posix_memalign_d)(void **__restrict pp, size_t alignment, size_t n_bytes, DEBUGINFO) { DEF_SETUP(setup); return mall_posix_memalign(&setup,pp,alignment,n_bytes); }
INTERN int    (LIBCCALL libc__mallopt_d)(int parameter_number, int parameter_value, DEBUGINFO)                     { DEF_SETUP(setup); return mall_mallopt(&setup,parameter_number,parameter_value); }
INTERN int    (LIBCCALL libc__malloc_trim_d)(size_t pad, DEBUGINFO)                                                { DEF_SETUP(setup); return mall_malloc_trim(&setup,pad); }
INTERN size_t (LIBCCALL libc__malloc_usable_size_d)(void *__restrict ptr, DEBUGINFO)                               { DEF_SETUP(setup); return mall_malloc_usable_size(&setup,ptr); }
INTERN void  *(LIBCCALL libc__memdup_d)(void const *__restrict ptr, size_t n_bytes, DEBUGINFO)                     { DEF_SETUP(setup); return mall_memdup(&setup,ptr,n_bytes); }
INTERN void  *(LIBCCALL libc__mall_getattrib_d)(void *__restrict ptr, int attrib, DEBUGINFO)                       { DEF_SETUP(setup); return mall_getattrib(&setup,ptr,attrib); }
INTERN ssize_t(LIBCCALL libc__mall_traceback_d)(void *__restrict ptr, ptbwalker callback, void *closure, DEBUGINFO){ DEF_SETUP(setup); return mall_traceback(&setup,ptr,callback,closure); }
INTERN void   (LIBCCALL libc__mall_printleaks_d)(DEBUGINFO)                                                        { DEF_SETUP(setup);        mall_printleaks(&setup); }
INTERN void   (LIBCCALL libc__mall_validate_d)(DEBUGINFO)                                                          { DEF_SETUP(setup);        mall_validate(&setup); }
INTERN ssize_t(LIBCCALL libc__mall_enum_d)(void *checkpoint, ssize_t(LIBCCALL *callback)(void *__restrict ptr, void *closure),
                                           void *closure, DEBUGINFO)                                               { DEF_SETUP(setup); return mall_enum(&setup ,checkpoint,callback,closure); }
INTERN void  *(LIBCCALL libc__mall_untrack_d)(void *mallptr, DEBUGINFO)                                            { DEF_SETUP(setup); return mall_untrack(&setup,mallptr,MALLFLAG_UNTRACKED); }
INTERN void  *(LIBCCALL libc__mall_nofree_d)(void *mallptr, DEBUGINFO)                                             { DEF_SETUP(setup); return mall_untrack(&setup,mallptr,MALLFLAG_UNTRACKED|MALLFLAG_NOFREE); }
#ifndef __KERNEL__
INTERN char  *(LIBCCALL libc__strdup_d)(char const *__restrict str, DEBUGINFO)                                     { DEF_SETUP(setup); return mall_strdup(&setup,str); }
INTERN char  *(LIBCCALL libc__strndup_d)(char const *__restrict str, size_t max_chars, DEBUGINFO)                  { DEF_SETUP(setup); return mall_strndup(&setup,str,max_chars); }
INTERN char  *(LIBCCALL libc__vstrdupf_d)(char const *__restrict format, va_list args, DEBUGINFO)                  { DEF_SETUP(setup); return mall_vstrdupf(&setup,format,args); }
INTERN void  *(LIBCCALL libc__memcdup_d)(void const *__restrict ptr, int needle, size_t n_bytes, DEBUGINFO)        { DEF_SETUP(setup); return mall_memcdup(&setup,ptr,needle,n_bytes); }
INTERN char *(ATTR_CDECL libc__strdupf_d)(DEBUGINFO, char const *__restrict format, ...) {
 char *result;
 va_list args;
 DEF_SETUP(setup);
 va_start(args,format);
 result = mall_vstrdupf(&setup,format,args);
 va_end(args);
 return result;
}
#endif /* !__KERNEL__ */
#undef DEF_SETUP

DECL_END
#endif

#include <uchar.h>

DECL_BEGIN

/* Export the regular malloc API, either linked against MALL
 * without debug info, dlmalloc or an external malloc API. */
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

#ifndef __INTELLISENSE__
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include "malloc-dos.c.inl"
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif

/* Delete any remaining macros. */
#undef _malloc_d
#undef _free_d
#undef _calloc_d
#undef _realloc_d
#undef _realloc_in_place_d
#undef _memalign_d
#undef _valloc_d
#undef _pvalloc_d
#undef _posix_memalign_d
#undef _mallopt_d
#undef _malloc_trim_d
#undef _malloc_usable_size_d
#undef _memdup_d
#undef _strdup_d
#undef _strndup_d
#undef _strdupf_d
#undef _vstrdupf_d
#undef _memcdup_d
#undef libc__malloc_d
#undef libc__free_d
#undef libc__calloc_d
#undef libc__realloc_d
#undef libc__realloc_in_place_d
#undef libc__memalign_d
#undef libc__valloc_d
#undef libc__pvalloc_d
#undef libc__posix_memalign_d
#undef libc__mallopt_d
#undef libc__malloc_trim_d
#undef libc__malloc_usable_size_d
#undef libc__memdup_d
#undef libc__strdup_d
#undef libc__strndup_d
#undef libc__strdupf_d
#undef libc__vstrdupf_d
#undef libc__memcdup_d

#undef malloc
#undef free
#undef calloc
#undef realloc
#undef realloc_in_place
#undef memalign
#undef valloc
#undef pvalloc
#undef posix_memalign
#undef mallopt
#undef malloc_trim
#undef malloc_usable_size
#undef memdup
#undef strdup
#undef strndup
#undef memcdup
#undef vstrdupf
#undef strdupf
#undef libc_malloc
#undef libc_free
#undef libc_calloc
#undef libc_realloc
#undef libc_realloc_in_place
#undef libc_memalign
#undef libc_valloc
#undef libc_pvalloc
#undef libc_posix_memalign
#undef libc_mallopt
#undef libc_malloc_trim
#undef libc_malloc_usable_size
#undef libc_memdup
#undef libc_strdup
#undef libc_strndup
#undef libc_memcdup
#undef libc_vstrdupf
#undef libc_strdupf

#undef _mall_getattrib
#undef _mall_traceback
#undef _mall_printleaks
#undef _mall_validate
#undef _mall_enum
#undef _mall_untrack
#undef _mall_nofree
#undef _mall_getattrib_d
#undef _mall_traceback_d
#undef _mall_printleaks_d
#undef _mall_validate_d
#undef _mall_enum_d
#undef _mall_untrack_d
#undef _mall_nofree_d
#undef libc__mall_getattrib
#undef libc__mall_traceback
#undef libc__mall_printleaks
#undef libc__mall_validate
#undef libc__mall_enum
#undef libc__mall_untrack
#undef libc__mall_nofree
#undef libc__mall_getattrib_d
#undef libc__mall_traceback_d
#undef libc__mall_printleaks_d
#undef libc__mall_validate_d
#undef libc__mall_enum_d
#undef libc__mall_untrack_d
#undef libc__mall_nofree_d

/* Export the MALL debug API, or stub-links against dlmalloc/an external malloc API. */
DEFINE_PUBLIC_ALIAS(_malloc_d,libc__malloc_d);
DEFINE_PUBLIC_ALIAS(_free_d,libc__free_d);
DEFINE_PUBLIC_ALIAS(_calloc_d,libc__calloc_d);
DEFINE_PUBLIC_ALIAS(_realloc_d,libc__realloc_d);
DEFINE_PUBLIC_ALIAS(_realloc_in_place_d,libc__realloc_in_place_d);
DEFINE_PUBLIC_ALIAS(_memalign_d,libc__memalign_d);
DEFINE_PUBLIC_ALIAS(_valloc_d,libc__valloc_d);
DEFINE_PUBLIC_ALIAS(_pvalloc_d,libc__pvalloc_d);
DEFINE_PUBLIC_ALIAS(_posix_memalign_d,libc__posix_memalign_d);
DEFINE_PUBLIC_ALIAS(_mallopt_d,libc__mallopt_d);
DEFINE_PUBLIC_ALIAS(_malloc_trim_d,libc__malloc_trim_d);
DEFINE_PUBLIC_ALIAS(_malloc_usable_size_d,libc__malloc_usable_size_d);
DEFINE_PUBLIC_ALIAS(_memdup_d,libc__memdup_d);
DEFINE_PUBLIC_ALIAS(_strdup_d,libc__strdup_d);
DEFINE_PUBLIC_ALIAS(_strndup_d,libc__strndup_d);
DEFINE_PUBLIC_ALIAS(_strdupf_d,libc__strdupf_d);
DEFINE_PUBLIC_ALIAS(_vstrdupf_d,libc__vstrdupf_d);
DEFINE_PUBLIC_ALIAS(_memcdup_d,libc__memcdup_d);

DEFINE_PUBLIC_ALIAS(malloc,EXPORT_MALLOC);
DEFINE_PUBLIC_ALIAS(free,EXPORT_FREE);
DEFINE_PUBLIC_ALIAS(calloc,EXPORT_CALLOC);
DEFINE_PUBLIC_ALIAS(realloc,EXPORT_REALLOC);
DEFINE_PUBLIC_ALIAS(realloc_in_place,EXPORT_REALLOC_IN_PLACE);
DEFINE_PUBLIC_ALIAS(memalign,EXPORT_MEMALIGN);
DEFINE_PUBLIC_ALIAS(valloc,EXPORT_VALLOC);
DEFINE_PUBLIC_ALIAS(pvalloc,EXPORT_PVALLOC);
DEFINE_PUBLIC_ALIAS(posix_memalign,EXPORT_POSIX_MEMALIGN);
DEFINE_PUBLIC_ALIAS(mallopt,EXPORT_MALLOPT);
DEFINE_PUBLIC_ALIAS(malloc_trim,EXPORT_MALLOC_TRIM);
DEFINE_PUBLIC_ALIAS(malloc_usable_size,EXPORT_MALLOC_USABLE_SIZE);

DEFINE_PUBLIC_ALIAS(memdup,libc_memdup);
DEFINE_PUBLIC_ALIAS(strdup,libc_strdup);
DEFINE_PUBLIC_ALIAS(strndup,libc_strndup);
DEFINE_PUBLIC_ALIAS(memcdup,libc_memcdup);
DEFINE_PUBLIC_ALIAS(vstrdupf,libc_vstrdupf);
DEFINE_PUBLIC_ALIAS(strdupf,libc_strdupf);

/* Define malloc-related function aliases */
DEFINE_PUBLIC_ALIAS(aligned_alloc,EXPORT_MEMALIGN);
DEFINE_PUBLIC_ALIAS(cfree,EXPORT_FREE);
DEFINE_PUBLIC_ALIAS(_aligned_alloc_d,libc__memalign_d);
DEFINE_PUBLIC_ALIAS(_cfree_d,libc__free_d);

/* Export the MALL debug API (either as stubs, or as an
 * implementation built ontop dlmalloc, or an external allocator) */
DEFINE_PUBLIC_ALIAS(_mall_getattrib,libc__mall_getattrib);
DEFINE_PUBLIC_ALIAS(_mall_traceback,libc__mall_traceback);
DEFINE_PUBLIC_ALIAS(_mall_printleaks,libc__mall_printleaks);
DEFINE_PUBLIC_ALIAS(_mall_validate,libc__mall_validate);
DEFINE_PUBLIC_ALIAS(_mall_enum,libc__mall_enum);
DEFINE_PUBLIC_ALIAS(_mall_untrack,libc__mall_untrack);
DEFINE_PUBLIC_ALIAS(_mall_nofree,libc__mall_nofree);
DEFINE_PUBLIC_ALIAS(_mall_getattrib_d,libc__mall_getattrib_d);
DEFINE_PUBLIC_ALIAS(_mall_traceback_d,libc__mall_traceback_d);
DEFINE_PUBLIC_ALIAS(_mall_printleaks_d,libc__mall_printleaks_d);
DEFINE_PUBLIC_ALIAS(_mall_validate_d,libc__mall_validate_d);
DEFINE_PUBLIC_ALIAS(_mall_enum_d,libc__mall_enum_d);
DEFINE_PUBLIC_ALIAS(_mall_untrack_d,libc__mall_untrack_d);
DEFINE_PUBLIC_ALIAS(_mall_nofree_d,libc__mall_nofree_d);

DECL_END

#endif /* !GUARD_LIBS_LIBC_MALLOC_C */
