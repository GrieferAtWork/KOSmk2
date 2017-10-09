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
#ifndef ___AMALLOCA_H
#define ___AMALLOCA_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <hybrid/alloca.h>
#include <hybrid/malloc.h>
#include <hybrid/string.h>


#define __AMALLOC_ALIGN    8
#ifdef __KERNEL__
#   define __AMALLOC_MAX   256 /* Without guard pages, don't be excessive */
#else
#   define __AMALLOC_MAX   4096
#endif

#ifdef NDEBUG
#   define __AMALLOC_KEY_ALLOCA        0
#   define __AMALLOC_KEY_MALLOC        1
#   define __AMALLOC_SKEW_ALLOCA(p,s) (p)
#   define __AMALLOC_GETKEY(p)       ((__UINT8_TYPE__ *)(p))[-__AMALLOC_ALIGN]
#   define __AMALLOC_MUSTFREE(p)      (__AMALLOC_GETKEY(p) != __AMALLOC_KEY_ALLOCA)
#else
#   define __AMALLOC_KEY_ALLOCA        0x7c
#   define __AMALLOC_KEY_MALLOC        0xb3
#   define __AMALLOC_GETKEY(p)       ((__UINT8_TYPE__ *)(p))[-__AMALLOC_ALIGN]
#ifdef assert
#   define __AMALLOC_MUSTFREE(p) \
   (assert(__AMALLOC_GETKEY(p) == __AMALLOC_KEY_ALLOCA || \
           __AMALLOC_GETKEY(p) == __AMALLOC_KEY_MALLOC), \
           __AMALLOC_GETKEY(p) == __AMALLOC_KEY_MALLOC)
#else
#   define __AMALLOC_MUSTFREE(p) \
          (__AMALLOC_GETKEY(p) == __AMALLOC_KEY_MALLOC)
#endif
#   define __AMALLOC_SKEW_ALLOCA(p,s)  __hybrid_memset(p,0xcd,s)
#endif

/* A hybrid between alloca and malloc, using alloca for
 * small allocations, but malloc() for larger ones.
 * NOTE: In all cases, 'afree()' should be used to clean up a
 *       pointer previously allocated using 'amalloc()' and
 *       friends. */
#ifndef __NO_XBLOCK
#define __amalloc(s) \
__XBLOCK({ __SIZE_TYPE__ const __s = (s)+__AMALLOC_ALIGN; \
           __UINT8_TYPE__ *__res; \
           if (__s > __AMALLOC_MAX) { \
             __res = (__UINT8_TYPE__ *)__hybrid_malloc(__s); \
             if (__res) { \
               *__res = __AMALLOC_KEY_MALLOC; \
               __res += __AMALLOC_ALIGN; \
             } \
           } else { \
             __res = (__UINT8_TYPE__ *)__ALLOCA(__s); \
             *__res = __AMALLOC_KEY_ALLOCA; \
             __res += __AMALLOC_ALIGN; \
             (void)__AMALLOC_SKEW_ALLOCA(__res,__s-__AMALLOC_ALIGN); \
           } \
           __XRETURN (void *)__res; \
})
#define __acalloc(s) \
__XBLOCK({ __SIZE_TYPE__ const __s = (s)+__AMALLOC_ALIGN; \
           __UINT8_TYPE__ *__res; \
           if (__s > __AMALLOC_MAX) { \
             __res = (__UINT8_TYPE__ *)__hybrid_calloc(1,__s); \
             if (__res) { \
               *__res = __AMALLOC_KEY_MALLOC; \
               __res += __AMALLOC_ALIGN; \
             } \
           } else { \
             __res = (__UINT8_TYPE__ *)__ALLOCA(__s); \
             *__res = __AMALLOC_KEY_ALLOCA; \
             __res += __AMALLOC_ALIGN; \
             __hybrid_memset(__res,0,__s-__AMALLOC_ALIGN); \
           } \
           __XRETURN (void *)__res;\
})
#define __afree(p) \
__XBLOCK({ void *const __p = (p); \
           if (__AMALLOC_MUSTFREE(__p)) \
               __hybrid_free((void *)((__UINT8_TYPE__ *)__p-__AMALLOC_ALIGN)); \
           (void)0; \
})
#else
__LOCAL void *(__LIBCCALL __libc_amalloc_heap)(__SIZE_TYPE__ __s) {
 __UINT8_TYPE__ *__res;
 __res = (__UINT8_TYPE__ *)__hybrid_malloc(__s+__AMALLOC_ALIGN);
 if (__res) { *__res = __AMALLOC_KEY_MALLOC; __res += __AMALLOC_ALIGN; }
 return (void *)__res;
}
__LOCAL void *(__LIBCCALL __libc_acalloc_heap)(__SIZE_TYPE__ __s) {
 __UINT8_TYPE__ *__res;
 __res = (__UINT8_TYPE__ *)__hybrid_calloc(1,__s+__AMALLOC_ALIGN);
 if (__res) { *__res = __AMALLOC_KEY_MALLOC; __res += __AMALLOC_ALIGN; }
 return (void *)__res;
}
__LOCAL void *(__LIBCCALL __libc_aminit_stack)(void *__p, __SIZE_TYPE__ __s) {
 *(__BYTE_TYPE__ *)__p = __AMALLOC_KEY_ALLOCA;
 return __AMALLOC_SKEW_ALLOCA((__BYTE_TYPE__ *)__p+__AMALLOC_ALIGN,__s);
}
__LOCAL void *(__LIBCCALL __libc_acinit_stack)(void *__p, __SIZE_TYPE__ __s) {
 *(__BYTE_TYPE__ *)__p = __AMALLOC_KEY_ALLOCA;
 return __hybrid_memset((__BYTE_TYPE__ *)__p+__AMALLOC_ALIGN,0,__s);
}
__LOCAL void (__LIBCCALL __libc_afree)(void *__p) {
 if (__AMALLOC_MUSTFREE(__p))
     __hybrid_free((void *)((__UINT8_TYPE__ *)__p-__AMALLOC_ALIGN));
}
/* Implementation that doesn't make use of X-blocks. */
#define __amalloc(s) \
  ((s) > __AMALLOC_MAX-__AMALLOC_ALIGN ? __libc_amalloc_heap(s) : \
         __libc_aminit_stack(__ALLOCA((s)+__AMALLOC_ALIGN),(s)))
#define __acalloc(s) \
  ((s) > __AMALLOC_MAX-__AMALLOC_ALIGN ? __libc_acalloc_heap(s) : \
         __libc_acinit_stack(__ALLOCA((s)+__AMALLOC_ALIGN),(s)))
#define __afree(p) __libc_afree(p)
#endif

#endif /* !___AMALLOCA_H */
