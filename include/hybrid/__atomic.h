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
#ifndef __GUARD_HYBRID___ATOMIC_H
#define __GUARD_HYBRID___ATOMIC_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <hybrid/xch.h>

#if defined(__DCC_VERSION__) || \
   (__has_builtin(__builtin_min) && __has_builtin(__builtin_max))
#   define __MIN(a,b) __builtin_min(a,b)
#   define __MAX(a,b) __builtin_max(a,b)
#elif defined(__COMPILER_HAVE_TYPEOF) && !defined(__NO_XBLOCK)
#   define __MIN(a,b) __XBLOCK({ __typeof__(a) _a = (a),_b = (b); __XRETURN _a < _b ? _a : _b; })
#   define __MAX(a,b) __XBLOCK({ __typeof__(a) _a = (a),_b = (b); __XRETURN _b < _a ? _a : _b; })
#else
#   define __MIN(a,b) ((a) < (b) ? (a) : (b))
#   define __MAX(a,b) ((b) < (a) ? (a) : (b))
#endif

#ifndef __cplusplus
#ifndef __COMPILER_HAVE_TYPEOF
#define __TYPEOF_RECAST(x,y)                 (y)
#else /* !__COMPILER_HAVE_TYPEOF */
#define __TYPEOF_RECAST(x,y) ((__typeof__(x))(y))
#endif /* __COMPILER_HAVE_TYPEOF */
#endif /* !__cplusplus */

/* Define atomic memory order constants. */
#ifndef __ATOMIC_RELAXED
#define __ATOMIC_RELAXED 0
#define __ATOMIC_CONSUME 1
#define __ATOMIC_ACQUIRE 2
#define __ATOMIC_RELEASE 3
#define __ATOMIC_ACQ_REL 4
#define __ATOMIC_SEQ_CST 5
#endif /* !__ATOMIC_RELAXED */

#define __hybrid_atomic_lockfree(x) 1 /* TODO */

#ifdef _MSC_VER
#ifndef __cplusplus
#pragma warning(push) /* Keep `4197' disabled in C because of its use in macros. */
#endif /* __cplusplus */
#pragma warning(disable: 4197) /* Casting away `volatile' */

#include "__atomic-msvc.h"
#endif

__SYSDECL_BEGIN

#if defined(__CC__) || defined(__DEEMON__)

#if __GCC_VERSION(4,7,0)
#define __hybrid_atomic_load(x,order)                          __atomic_load_n(&(x),order)
#define __hybrid_atomic_store(x,v,order)                       __atomic_store_n(&(x),v,order)
#define __hybrid_atomic_xch(x,v,order)                         __atomic_exchange_n(&(x),v,order)
#define __hybrid_atomic_cmpxch(x,oldv,newv,succ,fail)          __XBLOCK({ __typeof__(x) __oldv=(oldv); __XRETURN __atomic_compare_exchange_n(&(x),&__oldv,newv,0,fail,succ); })
#define __hybrid_atomic_cmpxch_weak(x,oldv,newv,succ,fail)     __XBLOCK({ __typeof__(x) __oldv=(oldv); __XRETURN __atomic_compare_exchange_n(&(x),&__oldv,newv,1,fail,succ); })
#define __hybrid_atomic_cmpxch_val(x,oldv,newv,succ,fail)      __XBLOCK({ __typeof__(x) __oldv=(oldv); __atomic_compare_exchange_n(&(x),&__oldv,newv,0,fail,succ); __XRETURN __oldv; })
#define __hybrid_atomic_cmpxch_val_weak(x,oldv,newv,succ,fail) __XBLOCK({ __typeof__(x) __oldv=(oldv); __atomic_compare_exchange_n(&(x),&__oldv,newv,1,fail,succ); __XRETURN __oldv; })
#define __hybrid_atomic_addfetch(x,v,order)                    __atomic_add_fetch(&(x),v,order)
#define __hybrid_atomic_subfetch(x,v,order)                    __atomic_sub_fetch(&(x),v,order)
#define __hybrid_atomic_andfetch(x,v,order)                    __atomic_and_fetch(&(x),v,order)
#define __hybrid_atomic_orfetch(x,v,order)                     __atomic_or_fetch(&(x),v,order)
#define __hybrid_atomic_xorfetch(x,v,order)                    __atomic_xor_fetch(&(x),v,order)
#define __hybrid_atomic_nandfetch(x,v,order)                   __atomic_nand_fetch(&(x),v,order)
#define __hybrid_atomic_fetchadd(x,v,order)                    __atomic_fetch_add(&(x),v,order)
#define __hybrid_atomic_fetchsub(x,v,order)                    __atomic_fetch_sub(&(x),v,order)
#define __hybrid_atomic_fetchand(x,v,order)                    __atomic_fetch_and(&(x),v,order)
#define __hybrid_atomic_fetchor(x,v,order)                     __atomic_fetch_or(&(x),v,order)
#define __hybrid_atomic_fetchxor(x,v,order)                    __atomic_fetch_xor(&(x),v,order)
#define __hybrid_atomic_fetchnand(x,v,order)                   __atomic_fetch_nand(&(x),v,order)
#else /* Modern GCC... */

#if defined(__GNUC__) || defined(__DCC_VERSION__)
#define __impl_hybrid_atomic_addfetch_seqcst(x,v)           __sync_add_and_fetch(&(x),v)
#define __impl_hybrid_atomic_subfetch_seqcst(x,v)           __sync_sub_and_fetch(&(x),v)
#define __impl_hybrid_atomic_orfetch_seqcst(x,v)            __sync_or_and_fetch(&(x),v)
#define __impl_hybrid_atomic_andfetch_seqcst(x,v)           __sync_and_and_fetch(&(x),v)
#define __impl_hybrid_atomic_xorfetch_seqcst(x,v)           __sync_xor_and_fetch(&(x),v)
#define __impl_hybrid_atomic_fetchadd_seqcst(x,v)           __sync_fetch_and_add(&(x),v)
#define __impl_hybrid_atomic_fetchsub_seqcst(x,v)           __sync_fetch_and_sub(&(x),v)
#define __impl_hybrid_atomic_fetchor_seqcst(x,v)            __sync_fetch_and_or(&(x),v)
#define __impl_hybrid_atomic_fetchand_seqcst(x,v)           __sync_fetch_and_and(&(x),v)
#define __impl_hybrid_atomic_fetchxor_seqcst(x,v)           __sync_fetch_and_xor(&(x),v)
#if !defined(__GNUC__) || __GCC_VERSION(4,4,0)
#define __impl_hybrid_atomic_fetchnand_seqcst(x,v)          __sync_fetch_and_nand(&(x),v)
#define __impl_hybrid_atomic_nandfetch_seqcst(x,v)          __sync_nand_and_fetch(&(x),v)
#endif /* GCC 4.4 */
#define __impl_hybrid_atomic_cmpxch_seqcst(x,oldv,newv)     __sync_bool_compare_and_swap(&(x),oldv,newv)
#define __impl_hybrid_atomic_cmpxch_val_seqcst(x,oldv,newv) __sync_val_compare_and_swap(&(x),oldv,newv)
#elif defined(__COMPILER_HAVE_GCC_ASM)
__SYSDECL_END
#include "__atomic-gasm.h"
__SYSDECL_BEGIN
#endif


#ifndef __impl_hybrid_atomic_cmpxch_seqcst
#ifdef __impl_hybrid_atomic_cmpxch_val_seqcst
#define __impl_hybrid_atomic_cmpxch_seqcst(x,oldv,newv) \
       (__impl_hybrid_atomic_cmpxch_val_seqcst(x,oldv,newv) == (oldv))
#endif /* __impl_hybrid_atomic_cmpxch_val_seqcst */
#endif /* !__impl_hybrid_atomic_cmpxch_seqcst */

#ifndef __hybrid_atomic_cmpxch
#ifdef __impl_hybrid_atomic_cmpxch_seqcst
#ifndef __NO_XBLOCK
#define __hybrid_atomic_cmpxch(x,oldv,newv,succ,fail) \
 __XBLOCK({ register __BOOL __xb_res; \
            int const __order = __MAX(succ,fail); \
            if (__order >= __ATOMIC_SEQ_CST) { \
                __xb_res = __impl_hybrid_atomic_cmpxch_seqcst(x,oldv,newv); \
            } else {\
                if (__order == __ATOMIC_ACQUIRE || \
                    __order >= __ATOMIC_ACQ_REL) \
                    __COMPILER_READ_BARRIER(); \
                __xb_res = (x) == (oldv); \
                if (__xb_res) { \
                  (x) = (newv); \
                  if ((succ) >= __ATOMIC_RELEASE) \
                       __COMPILER_WRITE_BARRIER(); \
                } else { \
                  if ((fail) >= __ATOMIC_ACQUIRE) \
                       __COMPILER_READ_BARRIER(); \
                } \
            } \
            __XRETURN __xb_res; \
 })
#else /* !__NO_XBLOCK */
#define __DEFINE_WRAPPER(n) \
__FORCELOCAL __BOOL \
(__impl_hybrid_atomic_cmpxch##n)(void *__x, \
                                 __UINT##n##_TYPE__ __oldv, \
                                 __UINT##n##_TYPE__ __newv, \
                                 int __succ, int __fail) { \
    register __BOOL __res; \
    int const __order = __MAX(__succ,__fail); \
    if (__order >= __ATOMIC_SEQ_CST) { \
        __res = __impl_hybrid_atomic_cmpxch_seqcst(*(__UINT##n##_TYPE__ *)__x,__oldv,__newv); \
    } else {\
        if (__order == __ATOMIC_ACQUIRE || \
            __order >= __ATOMIC_ACQ_REL) \
            __COMPILER_READ_BARRIER(); \
        __res = *(__UINT##n##_TYPE__ *)__x == __oldv; \
        if (__res) { \
          *(__UINT##n##_TYPE__ *)__x = __newv; \
          if (__succ >= __ATOMIC_RELEASE) \
              __COMPILER_WRITE_BARRIER(); \
        } else { \
          if (__fail >= __ATOMIC_ACQUIRE) \
              __COMPILER_READ_BARRIER(); \
        } \
    } \
    return __res; \
}
__NAMESPACE_INT_BEGIN
__DEFINE_WRAPPER(8)
__DEFINE_WRAPPER(16)
__DEFINE_WRAPPER(32)
__DEFINE_WRAPPER(64)
__NAMESPACE_INT_END
#undef __DEFINE_WRAPPER
#ifdef __cplusplus
extern "C++" { template<class __T, class __OV, class __NV>
#define __hybrid_atomic_cmpxch  __hybrid_atomic_cmpxch
__FORCELOCAL __BOOL (__hybrid_atomic_cmpxch)(__T &__x, __OV __oldv, __NV __newv, int __succ, int __fail) {
    __STATIC_IF(sizeof(__T) == 1) { return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch8((void *)&__x,(__UINT8_TYPE__)__oldv,(__UINT8_TYPE__)__newv,__succ,__fail); } __STATIC_ELSE(sizeof(__T) == 1) {
    __STATIC_IF(sizeof(__T) == 2) { return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch16((void *)&__x,(__UINT16_TYPE__)__oldv,(__UINT16_TYPE__)__newv,__succ,__fail); } __STATIC_ELSE(sizeof(__T) == 2) {
    __STATIC_IF(sizeof(__T) == 4) { return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch32((void *)&__x,(__UINT32_TYPE__)__oldv,(__UINT32_TYPE__)__newv,__succ,__fail); } __STATIC_ELSE(sizeof(__T) == 4) {
    return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch64((void *)&__x,(__UINT64_TYPE__)__oldv,(__UINT64_TYPE__)__newv,__succ,__fail);
} } } } }
#else
#define __hybrid_atomic_cmpxch(x,oldv,newv,succ,fail) \
 (sizeof(x) == 1 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch8((void *)&(x),(__UINT8_TYPE__)(oldv),(__UINT8_TYPE__)(newv),succ,fail) : \
  sizeof(x) == 2 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch16((void *)&(x),(__UINT16_TYPE__)(oldv),(__UINT16_TYPE__)(newv),succ,fail) : \
  sizeof(x) == 4 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch32((void *)&(x),(__UINT32_TYPE__)(oldv),(__UINT32_TYPE__)(newv),succ,fail) : \
                   __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch64((void *)&(x),(__UINT64_TYPE__)(oldv),(__UINT64_TYPE__)(newv),succ,fail))
#endif
#endif /* __NO_XBLOCK */
#endif
#endif /* !__hybrid_atomic_cmpxch */

#ifndef __hybrid_atomic_cmpxch_val
#ifdef __impl_hybrid_atomic_cmpxch_val_seqcst
#if !defined(__NO_XBLOCK) && defined(__COMPILER_HAVE_TYPEOF)
#define __hybrid_atomic_cmpxch_val(x,oldv,newv,succ,fail) \
 __XBLOCK({ __typeof__(x) __xv_res; \
            int const __order = __MAX(succ,fail); \
            if (__order >= __ATOMIC_SEQ_CST) { \
                __xv_res = (__typeof__(__xv_res))__impl_hybrid_atomic_cmpxch_val_seqcst(x,oldv,newv); \
            } else {\
                if (__order == __ATOMIC_ACQUIRE || \
                    __order >= __ATOMIC_ACQ_REL) \
                    __COMPILER_READ_BARRIER(); \
                __xv_res = (x); \
                if (__xv_res == (oldv)) { \
                  (x) = (newv); \
                  if ((succ) >= __ATOMIC_RELEASE) \
                       __COMPILER_WRITE_BARRIER(); \
                } else { \
                  if ((fail) >= __ATOMIC_ACQUIRE) \
                       __COMPILER_READ_BARRIER(); \
                } \
            } \
            __XRETURN __xv_res; \
 })
#else /* !__NO_XBLOCK && __COMPILER_HAVE_TYPEOF */
#define __DEFINE_WRAPPER(n) \
__FORCELOCAL __UINT##n##_TYPE__ \
(__impl_hybrid_atomic_cmpxch_val##n)(void *__x, \
                                     __UINT##n##_TYPE__ __oldv, \
                                     __UINT##n##_TYPE__ __newv, \
                                     int __succ, int __fail) { \
    register __UINT##n##_TYPE__ __res; \
    int const __order = __MAX(__succ,__fail); \
    if (__order >= __ATOMIC_SEQ_CST) { \
        __res = (__UINT##n##_TYPE__)__impl_hybrid_atomic_cmpxch_val_seqcst(*(__UINT##n##_TYPE__ *)__x,__oldv,__newv); \
    } else {\
        if (__order == __ATOMIC_ACQUIRE || \
            __order >= __ATOMIC_ACQ_REL) \
            __COMPILER_READ_BARRIER(); \
        __res = *(__UINT##n##_TYPE__ *)__x; \
        if (__res == __oldv) { \
          *(__UINT##n##_TYPE__ *)__x = __newv; \
          if (__succ >= __ATOMIC_RELEASE) \
              __COMPILER_WRITE_BARRIER(); \
        } else { \
          if (__fail >= __ATOMIC_ACQUIRE) \
              __COMPILER_READ_BARRIER(); \
        } \
    } \
    return __res; \
}
__NAMESPACE_INT_BEGIN
__DEFINE_WRAPPER(8)
__DEFINE_WRAPPER(16)
__DEFINE_WRAPPER(32)
__DEFINE_WRAPPER(64)
__NAMESPACE_INT_END
#undef __DEFINE_WRAPPER
#ifdef __cplusplus
extern "C++" { template<class __T, class __OV, class __NV>
#define __hybrid_atomic_cmpxch_val __hybrid_atomic_cmpxch_val
__FORCELOCAL __T (__hybrid_atomic_cmpxch_val)(__T &__x, __OV __oldv, __NV __newv, int __succ, int __fail) {
    __STATIC_IF(sizeof(__T) == 1) { return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val8((void *)&__x,(__UINT8_TYPE__)__oldv,(__UINT8_TYPE__)__newv,__succ,__fail); } __STATIC_ELSE(sizeof(__T) == 1) {
    __STATIC_IF(sizeof(__T) == 2) { return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val16((void *)&__x,(__UINT16_TYPE__)__oldv,(__UINT16_TYPE__)__newv,__succ,__fail); } __STATIC_ELSE(sizeof(__T) == 2) {
    __STATIC_IF(sizeof(__T) == 4) { return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val32((void *)&__x,(__UINT32_TYPE__)__oldv,(__UINT32_TYPE__)__newv,__succ,__fail); } __STATIC_ELSE(sizeof(__T) == 4) {
    return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val64((void *)&__x,(__UINT64_TYPE__)__oldv,(__UINT64_TYPE__)__newv,__succ,__fail);
} } } } }
#else /* __cplusplus */
#define __hybrid_atomic_cmpxch_val(x,oldv,newv,succ,fail) \
 __TYPEOF_RECAST(x,sizeof(x) == 1 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val8((void *)&(x),(__UINT8_TYPE__)(oldv),(__UINT8_TYPE__)(newv),succ,fail) : \
                   sizeof(x) == 2 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val16((void *)&(x),(__UINT16_TYPE__)(oldv),(__UINT16_TYPE__)(newv),succ,fail) : \
                   sizeof(x) == 4 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val32((void *)&(x),(__UINT32_TYPE__)(oldv),(__UINT32_TYPE__)(newv),succ,fail) : \
                                    __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val64((void *)&(x),(__UINT64_TYPE__)(oldv),(__UINT64_TYPE__)(newv),succ,fail))
#endif /* !__cplusplus */
#endif /* __NO_XBLOCK || !__COMPILER_HAVE_TYPEOF */
#endif
#endif /* !__hybrid_atomic_cmpxch */

#ifndef __hybrid_atomic_cmpxch
#ifdef __hybrid_atomic_cmpxch_val
#define __hybrid_atomic_cmpxch(x,oldv,newv,succ,fail) \
       (__hybrid_atomic_cmpxch_val(x,oldv,newv,succ,fail) == (oldv))
#else /* __hybrid_atomic_cmpxch_val */
/* Need at least `__hybrid_atomic_cmpxch()' or `__hybrid_atomic_cmpxch_val()' */
#ifndef __DEEMON__
#error "ERROR: Not atomic support by this compiler/on this platform."
#endif
#endif /* !__hybrid_atomic_cmpxch_val */
#endif /* !__hybrid_atomic_cmpxch */
#endif /* !GCC... */



/* Define value-based atomic compare-exchange operations. */
#ifndef __hybrid_atomic_cmpxch_val
#if !defined(__NO_XBLOCK) && defined(__COMPILER_HAVE_TYPEOF)
#define __hybrid_atomic_cmpxch_val(x,oldv,newv,succ,fail) \
 __XBLOCK({ register __typeof__(x) __xv_res; \
            do { __COMPILER_READ_BARRIER(); \
                 __xv_res = (x); /* __ATOMIC_ACQUIRE */ \
                 if (__hybrid_atomic_cmpxch(x,__xv_res,__xv_res == (oldv) \
                                            ? (newv) : __xv_res,succ,fail)) \
                     break;  \
            } __WHILE1; \
            __XRETURN __xv_res; \
 })
#else /* __NO_XBLOCK */
#define __DEFINE_WRAPPER(n) \
__FORCELOCAL __UINT##n##_TYPE__ \
(__impl_hybrid_atomic_cmpxch_val##n)(void *__x, \
                                     __UINT##n##_TYPE__ __oldv, \
                                     __UINT##n##_TYPE__ __newv, \
                                     int __succ, int __fail) { \
    register __UINT##n##_TYPE__ __res; \
    do { __COMPILER_READ_BARRIER(); \
         __res = *(__UINT##n##_TYPE__ *)__x; /* __ATOMIC_ACQUIRE */ \
         if (__hybrid_atomic_cmpxch(*(__UINT##n##_TYPE__ *)__x,__res,__res == __oldv \
                                    ? __newv : __res,__succ,__fail)) \
             break;  \
    } __WHILE1; \
    return __res; \
}
__NAMESPACE_INT_BEGIN
__DEFINE_WRAPPER(8)
__DEFINE_WRAPPER(16)
__DEFINE_WRAPPER(32)
__DEFINE_WRAPPER(64)
__NAMESPACE_INT_END
#undef __DEFINE_WRAPPER
#ifdef __cplusplus
extern "C++" { template<class __T, class __V>
__FORCELOCAL __T (__hybrid_atomic_cmpxch_val)(__T &__x, __V __oldv, __V __newv, int __succ, int __fail) {
    __STATIC_IF(sizeof(__T) == 1) { return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val8((void *)&__x,(__UINT8_TYPE__)__oldv,(__UINT8_TYPE__)__newv,__succ,__fail); } __STATIC_ELSE(sizeof(__T) == 1) {
    __STATIC_IF(sizeof(__T) == 2) { return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val16((void *)&__x,(__UINT16_TYPE__)__oldv,(__UINT16_TYPE__)__newv,__succ,__fail); } __STATIC_ELSE(sizeof(__T) == 2) {
    __STATIC_IF(sizeof(__T) == 4) { return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val32((void *)&__x,(__UINT32_TYPE__)__oldv,(__UINT32_TYPE__)__newv,__succ,__fail); } __STATIC_ELSE(sizeof(__T) == 4) {
    return __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val64((void *)&__x,(__UINT64_TYPE__)__oldv,(__UINT64_TYPE__)__newv,__succ,__fail);
} } } } }
#else /* __cplusplus */
#define __hybrid_atomic_cmpxch_val(x,oldv,newv,succ,fail) \
 __TYPEOF_RECAST(x,sizeof(x) == 1 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val8((void *)&(x),(__UINT8_TYPE__)(oldv),(__UINT8_TYPE__)(newv),succ,fail) : \
                   sizeof(x) == 2 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val16((void *)&(x),(__UINT16_TYPE__)(oldv),(__UINT16_TYPE__)(newv),succ,fail) : \
                   sizeof(x) == 4 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val32((void *)&(x),(__UINT32_TYPE__)(oldv),(__UINT32_TYPE__)(newv),succ,fail) : \
                                    __NAMESPACE_INT_SYM __impl_hybrid_atomic_cmpxch_val64((void *)&(x),(__UINT64_TYPE__)(oldv),(__UINT64_TYPE__)(newv),succ,fail))
#endif /* __cplusplus */
#endif /* !__NO_XBLOCK */
#endif /* !__hybrid_atomic_cmpxch_val */
#ifndef __hybrid_atomic_cmpxch_weak
#define __hybrid_atomic_cmpxch_weak(x,oldv,newv,succ,fail) \
        __hybrid_atomic_cmpxch(x,oldv,newv,succ,fail)
#endif
#ifndef __hybrid_atomic_cmpxch_val_weak
#define __hybrid_atomic_cmpxch_val_weak(x,oldv,newv,succ,fail) \
        __hybrid_atomic_cmpxch_val(x,oldv,newv,succ,fail)
#endif /* !__hybrid_atomic_cmpxch_val_weak */


/* Define atomic load/store operations. */

#ifndef __hybrid_atomic_load
#ifdef __impl_hybrid_atomic_fetchadd_seqcst
#   define __impl_hybrid_atomic_load_seqcst(x) __impl_hybrid_atomic_fetchadd_seqcst(x,0)
#elif defined(__impl_hybrid_atomic_fetchsub_seqcst)
#   define __impl_hybrid_atomic_load_seqcst(x) __impl_hybrid_atomic_fetchsub_seqcst(x,0)
#elif defined(__impl_hybrid_atomic_fetchor_seqcst)
#   define __impl_hybrid_atomic_load_seqcst(x) __impl_hybrid_atomic_fetchor_seqcst(x,0)
#elif defined(__impl_hybrid_atomic_cmpxch_val_seqcst)
#   define __impl_hybrid_atomic_load_seqcst(x) __impl_hybrid_atomic_cmpxch_val_seqcst(x,0,0)
#else
#   define __impl_hybrid_atomic_load_seqcst(x) __hybrid_atomic_cmpxch_val(x,0,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)
#endif
#if !defined(__NO_XBLOCK) && defined(__COMPILER_HAVE_TYPEOF)
#define __hybrid_atomic_load(x,order) \
 __XBLOCK({ register __typeof__(x) __ld_res; \
            if ((order) >= __ATOMIC_SEQ_CST) \
                __ld_res = __impl_hybrid_atomic_load_seqcst(x); \
            else { \
                if ((order) == __ATOMIC_ACQUIRE || \
                    (order) == __ATOMIC_ACQ_REL) \
                    __COMPILER_READ_BARRIER(); \
                __ld_res = (x); \
                if ((order) >= __ATOMIC_RELEASE) \
                    __COMPILER_WRITE_BARRIER(); \
            } \
            __XRETURN __ld_res; \
 })
#else /* !__NO_XBLOCK && __COMPILER_HAVE_TYPEOF */
#define __DEFINE_WRAPPER(n) \
__FORCELOCAL __UINT##n##_TYPE__ \
(__impl_hybrid_atomic_load##n)(void *__x, int __order) { \
    register __UINT##n##_TYPE__ __res; \
    if (__order >= __ATOMIC_SEQ_CST) \
        __res = (__UINT##n##_TYPE__)__impl_hybrid_atomic_load_seqcst(*(__UINT##n##_TYPE__ *)__x); \
    else { \
        if (__order == __ATOMIC_ACQUIRE || \
            __order == __ATOMIC_ACQ_REL) \
            __COMPILER_READ_BARRIER(); \
        __res = *(__UINT##n##_TYPE__ *)__x; \
        if (__order >= __ATOMIC_RELEASE) \
            __COMPILER_WRITE_BARRIER(); \
    } \
    return __res; \
}
__NAMESPACE_INT_BEGIN
__DEFINE_WRAPPER(8)
__DEFINE_WRAPPER(16)
__DEFINE_WRAPPER(32)
__DEFINE_WRAPPER(64)
__NAMESPACE_INT_END
#undef __DEFINE_WRAPPER
#ifdef __cplusplus
extern "C++" { template<class __T>
#define __hybrid_atomic_load __hybrid_atomic_load
__FORCELOCAL __T (__hybrid_atomic_load)(__T &__x, int __order) {
    __STATIC_IF(sizeof(__T) == 1) { return (__T)__NAMESPACE_INT_SYM __impl_hybrid_atomic_load8((void *)&__x,__order); } __STATIC_ELSE(sizeof(__T) == 1) {
    __STATIC_IF(sizeof(__T) == 2) { return (__T)__NAMESPACE_INT_SYM __impl_hybrid_atomic_load16((void *)&__x,__order); } __STATIC_ELSE(sizeof(__T) == 2) {
    __STATIC_IF(sizeof(__T) == 4) { return (__T)__NAMESPACE_INT_SYM __impl_hybrid_atomic_load32((void *)&__x,__order); } __STATIC_ELSE(sizeof(__T) == 4) {
    return (__T)__NAMESPACE_INT_SYM __impl_hybrid_atomic_load64((void *)&__x,__order);
} } } } }
#else /* __cplusplus */
#define __hybrid_atomic_load(x,order) \
 __TYPEOF_RECAST(x,sizeof(x) == 1 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_load8((void *)&(x),order) : \
                   sizeof(x) == 2 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_load16((void *)&(x),order) : \
                   sizeof(x) == 4 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_load32((void *)&(x),order) : \
                                    __NAMESPACE_INT_SYM __impl_hybrid_atomic_load64((void *)&(x),order))
#endif /* !__cplusplus */
#endif /* __NO_XBLOCK || !__COMPILER_HAVE_TYPEOF */
#endif /* !__hybrid_atomic_load */

#ifndef __hybrid_atomic_store
#ifdef __impl_hybrid_atomic_xch_seqcst
#define __impl_hybrid_atomic_store_seqcst(x,v) \
        __impl_hybrid_atomic_xch_seqcst(x,v)
#else
#define __impl_hybrid_atomic_store_seqcst(x,v) \
        __hybrid_atomic_xch(x,v,__ATOMIC_SEQ_CST)
#endif
#ifndef __NO_XBLOCK
#define __hybrid_atomic_store(x,v,order) \
 __XBLOCK({ if ((order) >= __ATOMIC_SEQ_CST) \
                __impl_hybrid_atomic_store_seqcst(x,v); \
            else { \
                if ((order) == __ATOMIC_ACQUIRE || \
                    (order) == __ATOMIC_ACQ_REL) \
                    __COMPILER_READ_BARRIER(); \
                (x) = (v); \
                if ((order) >= __ATOMIC_RELEASE) \
                    __COMPILER_WRITE_BARRIER(); \
            } \
            (void)0; \
 })
#else /* !__NO_XBLOCK */
#define __DEFINE_WRAPPER(n) \
__FORCELOCAL void \
(__impl_hybrid_atomic_store##n)(void *__x, __UINT##n##_TYPE__ __v, int __order) {\
    if (__order >= __ATOMIC_SEQ_CST) \
        __impl_hybrid_atomic_store_seqcst(*(__UINT##n##_TYPE__ *)__x,__v); \
    else { \
        if (__order == __ATOMIC_ACQUIRE || \
            __order == __ATOMIC_ACQ_REL) \
            __COMPILER_READ_BARRIER(); \
        *(__UINT##n##_TYPE__ *)__x = __v; \
        if (__order >= __ATOMIC_RELEASE) \
            __COMPILER_WRITE_BARRIER(); \
    } \
}
__NAMESPACE_INT_BEGIN
__DEFINE_WRAPPER(8)
__DEFINE_WRAPPER(16)
__DEFINE_WRAPPER(32)
__DEFINE_WRAPPER(64)
__NAMESPACE_INT_END
#undef __DEFINE_WRAPPER
#ifdef __cplusplus
extern "C++" { template<class __T, class __V>
#define __hybrid_atomic_store __hybrid_atomic_store
__FORCELOCAL void (__hybrid_atomic_store)(__T &__x, __V __v, int __order) {
    __STATIC_IF(sizeof(__T) == 1) { __NAMESPACE_INT_SYM __impl_hybrid_atomic_store8((void *)&__x,(__UINT8_TYPE__)__v,__order); } __STATIC_ELSE(sizeof(__T) == 1) {
    __STATIC_IF(sizeof(__T) == 2) { __NAMESPACE_INT_SYM __impl_hybrid_atomic_store16((void *)&__x,(__UINT16_TYPE__)__v,__order); } __STATIC_ELSE(sizeof(__T) == 2) {
    __STATIC_IF(sizeof(__T) == 4) { __NAMESPACE_INT_SYM __impl_hybrid_atomic_store32((void *)&__x,(__UINT32_TYPE__)__v,__order); } __STATIC_ELSE(sizeof(__T) == 4) {
    __NAMESPACE_INT_SYM __impl_hybrid_atomic_store64((void *)&__x,(__UINT64_TYPE__)__v,order);
} } } } }
#else /* __cplusplus */
#define __hybrid_atomic_store(x,v,order) \
 (sizeof(x) == 1 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_store8((void *)&(x),(__UINT8_TYPE__)(v),order) : \
  sizeof(x) == 2 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_store16((void *)&(x),(__UINT16_TYPE__)(v),order) : \
  sizeof(x) == 4 ? __NAMESPACE_INT_SYM __impl_hybrid_atomic_store32((void *)&(x),(__UINT32_TYPE__)(v),order) : \
                   __NAMESPACE_INT_SYM __impl_hybrid_atomic_store64((void *)&(x),(__UINT64_TYPE__)(v),order))
#endif /* !__cplusplus */
#endif /* __NO_XBLOCK */
#endif /* !__hybrid_atomic_store */


#ifndef __impl_hybrid_atomic_fetchadd_seqcst
#ifdef __impl_hybrid_atomic_fetchsub_seqcst
#define __impl_hybrid_atomic_fetchadd_seqcst(x,v) \
        __impl_hybrid_atomic_fetchsub_seqcst(x,0-(v))
#endif /* __impl_hybrid_atomic_fetchsub_seqcst */
#else /* !__impl_hybrid_atomic_fetchadd_seqcst */
#ifndef __impl_hybrid_atomic_fetchsub_seqcst
#define __impl_hybrid_atomic_fetchsub_seqcst(x,v) \
        __impl_hybrid_atomic_fetchadd_seqcst(x,0-(v))
#endif /* !__impl_hybrid_atomic_fetchsub_seqcst */
#endif /* __impl_hybrid_atomic_fetchadd_seqcst */


/* Perform logical substitution. */
#define __hybrid_opfun_xch(x,y)         (y)
#define __hybrid_opfun_add(x,y)    ((x)+(y))
#define __hybrid_opfun_sub(x,y)    ((x)-(y))
#define __hybrid_opfun_and(x,y)    ((x)&(y))
#define __hybrid_opfun_or(x,y)     ((x)|(y))
#define __hybrid_opfun_xor(x,y)    ((x)^(y))
#define __hybrid_opfun_nand(x,y) (~((x)&(y)))
#if !defined(__NO_XBLOCK) && defined(__COMPILER_HAVE_TYPEOF)
#define __XBLOCK_hybrid_atomic_fetchop_seqcst(opfun,x,v) \
 __XBLOCK({ register __typeof__(x) __sc_res; \
            do __sc_res = (x); \
            while (!__hybrid_atomic_cmpxch_weak(x,__sc_res, \
                  (__typeof__(__sc_res))opfun(__sc_res,v),__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)); \
            __XRETURN __sc_res; \
 })
#define __XBLOCK_hybrid_atomic_fetchop(opfun,seqcst,x,v,order) \
 __XBLOCK({ register __typeof__(x) __fo_res; \
            if ((order) >= __ATOMIC_SEQ_CST) { \
                __fo_res = seqcst(x,v); \
            } else { \
                if ((order) == __ATOMIC_ACQUIRE || \
                    (order) == __ATOMIC_ACQ_REL) \
                    __COMPILER_READ_BARRIER(); \
                __fo_res = (x); \
                (x) = (__typeof__(__fo_res))opfun(__fo_res,v); \
                if ((order) >= __ATOMIC_RELEASE) \
                    __COMPILER_WRITE_BARRIER(); \
            } \
            __XRETURN __fo_res; \
 })
#else /* !__NO_XBLOCK && __COMPILER_HAVE_TYPEOF */
#define __DECL_INLINE_hybrid_atomic_fetchop_seqcst(name,n,opfun) \
__FORCELOCAL __UINT##n##_TYPE__ \
(name##n##_seqcst)(void *__x, __UINT##n##_TYPE__ __v) { \
    register __UINT##n##_TYPE__ __res; \
    do { __COMPILER_READ_BARRIER(); \
         __res = *(__UINT##n##_TYPE__ *)__x; \
    } while (!__hybrid_atomic_cmpxch_weak(*(__UINT##n##_TYPE__ *)__x,__res,\
            (__UINT##n##_TYPE__)opfun(__res,__v),__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)); \
    return __res; \
}
#define __DECL_INLINE_hybrid_atomic_fetchop(name,name_seqcst,n,opfun) \
__FORCELOCAL __UINT##n##_TYPE__ \
(name##n)(void *__x, __UINT##n##_TYPE__ __v, int __order) { \
    register __UINT##n##_TYPE__ __res; \
    if (__order >= __ATOMIC_SEQ_CST) { \
        __res = name_seqcst(*(__UINT##n##_TYPE__ *)__x,__v); \
    } else { \
        if (__order == __ATOMIC_ACQUIRE || \
            __order == __ATOMIC_ACQ_REL) \
            __COMPILER_READ_BARRIER(); \
        __res = *(__UINT##n##_TYPE__ *)__x; \
        *(__UINT##n##_TYPE__ *)__x = (__UINT##n##_TYPE__)opfun(__res,__v); \
        if (__order >= __ATOMIC_RELEASE) \
            __COMPILER_WRITE_BARRIER(); \
    } \
    return __res; \
}
#ifndef __cplusplus
#define __INLINE_hybrid_atomic_fetchop_seqcst(name,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop_seqcst(name,8,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop_seqcst(name,16,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop_seqcst(name,32,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop_seqcst(name,64,opfun)
#define __CALL_hybrid_atomic_fetchop_seqcst(name,x,v) \
 __TYPEOF_RECAST(x,sizeof(x) == 1 ? name##8_seqcst((void *)&(x),(__UINT8_TYPE__)(v)) : \
                   sizeof(x) == 2 ? name##16_seqcst((void *)&(x),(__UINT16_TYPE__)(v)) : \
                   sizeof(x) == 4 ? name##32_seqcst((void *)&(x),(__UINT32_TYPE__)(v)) : \
                                    name##64_seqcst((void *)&(x),(__UINT64_TYPE__)(v)))
#define __INLINE_hybrid_atomic_fetchop(name,name_seqcst,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop(__impl_##name,name_seqcst,8,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop(__impl_##name,name_seqcst,16,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop(__impl_##name,name_seqcst,32,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop(__impl_##name,name_seqcst,64,opfun)
#define __CALL_hybrid_atomic_fetchop(name,x,v,order) \
 __TYPEOF_RECAST(x,sizeof(x) == 1 ? __impl_##name##8((void *)&(x),(__UINT8_TYPE__)(v),order) : \
                   sizeof(x) == 2 ? __impl_##name##16((void *)&(x),(__UINT16_TYPE__)(v),order) : \
                   sizeof(x) == 4 ? __impl_##name##32((void *)&(x),(__UINT32_TYPE__)(v),order) : \
                                    __impl_##name##64((void *)&(x),(__UINT64_TYPE__)(v),order))
#else /* !__cplusplus */
#define __INLINE_hybrid_atomic_fetchop_seqcst(name,opfun) \
 __NAMESPACE_INT_BEGIN \
 __DECL_INLINE_hybrid_atomic_fetchop_seqcst(name,8,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop_seqcst(name,16,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop_seqcst(name,32,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop_seqcst(name,64,opfun) \
 __NAMESPACE_INT_END \
extern "C++" { template<class __T, class __V> \
__FORCELOCAL __T (name##_seqcst)(__T &__x, __V __v) { \
    __STATIC_IF(sizeof(__T) == 1) { return (__T)__NAMESPACE_INT_SYM name##8_seqcst((void *)&__x,(__UINT8_TYPE__)__v); } __STATIC_ELSE(sizeof(__T) == 1) { \
    __STATIC_IF(sizeof(__T) == 2) { return (__T)__NAMESPACE_INT_SYM name##16_seqcst((void *)&__x,(__UINT16_TYPE__)__v); } __STATIC_ELSE(sizeof(__T) == 2) { \
    __STATIC_IF(sizeof(__T) == 4) { return (__T)__NAMESPACE_INT_SYM name##32_seqcst((void *)&__x,(__UINT64_TYPE__)__v); } __STATIC_ELSE(sizeof(__T) == 4) { \
    return (__T)__NAMESPACE_INT_SYM name##64_seqcst((void *)&__x,(__UINT64_TYPE__)__v); \
} } } } }
#define __INLINE_hybrid_atomic_fetchop(name,name_seqcst,opfun) \
 __NAMESPACE_INT_BEGIN \
 __DECL_INLINE_hybrid_atomic_fetchop(__impl_##name,name_seqcst,8,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop(__impl_##name,name_seqcst,16,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop(__impl_##name,name_seqcst,32,opfun) \
 __DECL_INLINE_hybrid_atomic_fetchop(__impl_##name,name_seqcst,64,opfun) \
 __NAMESPACE_INT_END \
extern "C++" { template<class __T, class __V> \
__FORCELOCAL __T (name)(__T &__x, __V __v, int __order) { \
    __STATIC_IF(sizeof(__T) == 1) { return (__T)__NAMESPACE_INT_SYM __impl_##name##8((void *)&__x,(__UINT8_TYPE__)__v,__order); } __STATIC_ELSE(sizeof(__T) == 1) { \
    __STATIC_IF(sizeof(__T) == 2) { return (__T)__NAMESPACE_INT_SYM __impl_##name##16((void *)&__x,(__UINT16_TYPE__)__v,__order); } __STATIC_ELSE(sizeof(__T) == 2) { \
    __STATIC_IF(sizeof(__T) == 4) { return (__T)__NAMESPACE_INT_SYM __impl_##name##32((void *)&__x,(__UINT64_TYPE__)__v,__order); } __STATIC_ELSE(sizeof(__T) == 4) { \
    return (__T)__NAMESPACE_INT_SYM __impl_##name##64((void *)&__x,(__UINT64_TYPE__)__v,__order); \
} } } } }
#endif /* __cplusplus */
#endif /* __NO_XBLOCK || !__COMPILER_HAVE_TYPEOF */

/*[[[deemon
function atomic_operation(name,opname) {
    print "#ifndef __hybrid_atomic_"+name;
    print "#ifdef __XBLOCK_hybrid_atomic_fetchop";
    print "#ifndef __impl_hybrid_atomic_"+name+"_seqcst";
    print "#define __impl_hybrid_atomic_"+name+"_seqcst(x,v) \\";
    print " __XBLOCK_hybrid_atomic_fetchop_seqcst(__hybrid_opfun_"+opname+",x,v)";
    print "#endif /" "* __impl_hybrid_atomic_"+name+"_seqcst *" "/";
    print "#define __hybrid_atomic_"+name+"(x,v,order) \\";
    print "  __XBLOCK_hybrid_atomic_fetchop(__hybrid_opfun_"+opname+",__impl_hybrid_atomic_"+name+"_seqcst,x,v,order)";
    print "#else /" "* __XBLOCK_hybrid_atomic_fetchop *" "/";
    print "#ifndef __impl_hybrid_atomic_"+name+"_seqcst";
    print "#ifdef __CALL_hybrid_atomic_fetchop_seqcst";
    print "#define __impl_hybrid_atomic_"+name+"_seqcst(x,v)  \\";
    print "  __CALL_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_"+name+",x,v)";
    print "#else /" "* __CALL_hybrid_atomic_fetchop_seqcst *" "/";
    print "#define __impl_hybrid_atomic_"+name+"_seqcst  __impl_hybrid_atomic_"+name+"_seqcst";
    print "#endif /" "* !__CALL_hybrid_atomic_fetchop_seqcst *" "/";
    print "__INLINE_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_"+name+",__hybrid_opfun_"+opname+")";
    print "#endif /" "* __impl_hybrid_atomic_"+name+"_seqcst *" "/";
    print "__INLINE_hybrid_atomic_fetchop(__hybrid_atomic_"+name+",__impl_hybrid_atomic_"+name+"_seqcst,__hybrid_opfun_"+opname+")";
    print "#ifdef __CALL_hybrid_atomic_fetchop";
    print "#define __hybrid_atomic_"+name+"(x,v,order) \\";
    print "  __CALL_hybrid_atomic_fetchop(__hybrid_atomic_"+name+",x,v,order)";
    print "#else /" "* __CALL_hybrid_atomic_fetchop *" "/";
    print "#define __hybrid_atomic_"+name+" __hybrid_atomic_"+name;
    print "#endif /" "* !__CALL_hybrid_atomic_fetchop *" "/";
    print "#endif /" "* !__XBLOCK_hybrid_atomic_fetchop *" "/";
    print "#endif /" "* !__hybrid_atomic_"+name+" *" "/";
    print;
}
atomic_operation("xch","xch");
atomic_operation("fetchadd","add");
atomic_operation("fetchsub","sub");
atomic_operation("fetchand","and");
atomic_operation("fetchor","or");
atomic_operation("fetchxor","xor");
atomic_operation("fetchnand","nand");
]]]*/
#ifndef __hybrid_atomic_xch
#ifdef __XBLOCK_hybrid_atomic_fetchop
#ifndef __impl_hybrid_atomic_xch_seqcst
#define __impl_hybrid_atomic_xch_seqcst(x,v) \
 __XBLOCK_hybrid_atomic_fetchop_seqcst(__hybrid_opfun_xch,x,v)
#endif /* __impl_hybrid_atomic_xch_seqcst */
#define __hybrid_atomic_xch(x,v,order) \
  __XBLOCK_hybrid_atomic_fetchop(__hybrid_opfun_xch,__impl_hybrid_atomic_xch_seqcst,x,v,order)
#else /* __XBLOCK_hybrid_atomic_fetchop */
#ifndef __impl_hybrid_atomic_xch_seqcst
#ifdef __CALL_hybrid_atomic_fetchop_seqcst
#define __impl_hybrid_atomic_xch_seqcst(x,v)  \
  __CALL_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_xch,x,v)
#else /* __CALL_hybrid_atomic_fetchop_seqcst */
#define __impl_hybrid_atomic_xch_seqcst  __impl_hybrid_atomic_xch_seqcst
#endif /* !__CALL_hybrid_atomic_fetchop_seqcst */
__INLINE_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_xch,__hybrid_opfun_xch)
#endif /* __impl_hybrid_atomic_xch_seqcst */
__INLINE_hybrid_atomic_fetchop(__hybrid_atomic_xch,__impl_hybrid_atomic_xch_seqcst,__hybrid_opfun_xch)
#ifdef __CALL_hybrid_atomic_fetchop
#define __hybrid_atomic_xch(x,v,order) \
  __CALL_hybrid_atomic_fetchop(__hybrid_atomic_xch,x,v,order)
#else /* __CALL_hybrid_atomic_fetchop */
#define __hybrid_atomic_xch __hybrid_atomic_xch
#endif /* !__CALL_hybrid_atomic_fetchop */
#endif /* !__XBLOCK_hybrid_atomic_fetchop */
#endif /* !__hybrid_atomic_xch */

#ifndef __hybrid_atomic_fetchadd
#ifdef __XBLOCK_hybrid_atomic_fetchop
#ifndef __impl_hybrid_atomic_fetchadd_seqcst
#define __impl_hybrid_atomic_fetchadd_seqcst(x,v) \
 __XBLOCK_hybrid_atomic_fetchop_seqcst(__hybrid_opfun_add,x,v)
#endif /* __impl_hybrid_atomic_fetchadd_seqcst */
#define __hybrid_atomic_fetchadd(x,v,order) \
  __XBLOCK_hybrid_atomic_fetchop(__hybrid_opfun_add,__impl_hybrid_atomic_fetchadd_seqcst,x,v,order)
#else /* __XBLOCK_hybrid_atomic_fetchop */
#ifndef __impl_hybrid_atomic_fetchadd_seqcst
#ifdef __CALL_hybrid_atomic_fetchop_seqcst
#define __impl_hybrid_atomic_fetchadd_seqcst(x,v)  \
  __CALL_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchadd,x,v)
#else /* __CALL_hybrid_atomic_fetchop_seqcst */
#define __impl_hybrid_atomic_fetchadd_seqcst  __impl_hybrid_atomic_fetchadd_seqcst
#endif /* !__CALL_hybrid_atomic_fetchop_seqcst */
__INLINE_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchadd,__hybrid_opfun_add)
#endif /* __impl_hybrid_atomic_fetchadd_seqcst */
__INLINE_hybrid_atomic_fetchop(__hybrid_atomic_fetchadd,__impl_hybrid_atomic_fetchadd_seqcst,__hybrid_opfun_add)
#ifdef __CALL_hybrid_atomic_fetchop
#define __hybrid_atomic_fetchadd(x,v,order) \
  __CALL_hybrid_atomic_fetchop(__hybrid_atomic_fetchadd,x,v,order)
#else /* __CALL_hybrid_atomic_fetchop */
#define __hybrid_atomic_fetchadd __hybrid_atomic_fetchadd
#endif /* !__CALL_hybrid_atomic_fetchop */
#endif /* !__XBLOCK_hybrid_atomic_fetchop */
#endif /* !__hybrid_atomic_fetchadd */

#ifndef __hybrid_atomic_fetchsub
#ifdef __XBLOCK_hybrid_atomic_fetchop
#ifndef __impl_hybrid_atomic_fetchsub_seqcst
#define __impl_hybrid_atomic_fetchsub_seqcst(x,v) \
 __XBLOCK_hybrid_atomic_fetchop_seqcst(__hybrid_opfun_sub,x,v)
#endif /* __impl_hybrid_atomic_fetchsub_seqcst */
#define __hybrid_atomic_fetchsub(x,v,order) \
  __XBLOCK_hybrid_atomic_fetchop(__hybrid_opfun_sub,__impl_hybrid_atomic_fetchsub_seqcst,x,v,order)
#else /* __XBLOCK_hybrid_atomic_fetchop */
#ifndef __impl_hybrid_atomic_fetchsub_seqcst
#ifdef __CALL_hybrid_atomic_fetchop_seqcst
#define __impl_hybrid_atomic_fetchsub_seqcst(x,v)  \
  __CALL_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchsub,x,v)
#else /* __CALL_hybrid_atomic_fetchop_seqcst */
#define __impl_hybrid_atomic_fetchsub_seqcst  __impl_hybrid_atomic_fetchsub_seqcst
#endif /* !__CALL_hybrid_atomic_fetchop_seqcst */
__INLINE_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchsub,__hybrid_opfun_sub)
#endif /* __impl_hybrid_atomic_fetchsub_seqcst */
__INLINE_hybrid_atomic_fetchop(__hybrid_atomic_fetchsub,__impl_hybrid_atomic_fetchsub_seqcst,__hybrid_opfun_sub)
#ifdef __CALL_hybrid_atomic_fetchop
#define __hybrid_atomic_fetchsub(x,v,order) \
  __CALL_hybrid_atomic_fetchop(__hybrid_atomic_fetchsub,x,v,order)
#else /* __CALL_hybrid_atomic_fetchop */
#define __hybrid_atomic_fetchsub __hybrid_atomic_fetchsub
#endif /* !__CALL_hybrid_atomic_fetchop */
#endif /* !__XBLOCK_hybrid_atomic_fetchop */
#endif /* !__hybrid_atomic_fetchsub */

#ifndef __hybrid_atomic_fetchand
#ifdef __XBLOCK_hybrid_atomic_fetchop
#ifndef __impl_hybrid_atomic_fetchand_seqcst
#define __impl_hybrid_atomic_fetchand_seqcst(x,v) \
 __XBLOCK_hybrid_atomic_fetchop_seqcst(__hybrid_opfun_and,x,v)
#endif /* __impl_hybrid_atomic_fetchand_seqcst */
#define __hybrid_atomic_fetchand(x,v,order) \
  __XBLOCK_hybrid_atomic_fetchop(__hybrid_opfun_and,__impl_hybrid_atomic_fetchand_seqcst,x,v,order)
#else /* __XBLOCK_hybrid_atomic_fetchop */
#ifndef __impl_hybrid_atomic_fetchand_seqcst
#ifdef __CALL_hybrid_atomic_fetchop_seqcst
#define __impl_hybrid_atomic_fetchand_seqcst(x,v)  \
  __CALL_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchand,x,v)
#else /* __CALL_hybrid_atomic_fetchop_seqcst */
#define __impl_hybrid_atomic_fetchand_seqcst  __impl_hybrid_atomic_fetchand_seqcst
#endif /* !__CALL_hybrid_atomic_fetchop_seqcst */
__INLINE_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchand,__hybrid_opfun_and)
#endif /* __impl_hybrid_atomic_fetchand_seqcst */
__INLINE_hybrid_atomic_fetchop(__hybrid_atomic_fetchand,__impl_hybrid_atomic_fetchand_seqcst,__hybrid_opfun_and)
#ifdef __CALL_hybrid_atomic_fetchop
#define __hybrid_atomic_fetchand(x,v,order) \
  __CALL_hybrid_atomic_fetchop(__hybrid_atomic_fetchand,x,v,order)
#else /* __CALL_hybrid_atomic_fetchop */
#define __hybrid_atomic_fetchand __hybrid_atomic_fetchand
#endif /* !__CALL_hybrid_atomic_fetchop */
#endif /* !__XBLOCK_hybrid_atomic_fetchop */
#endif /* !__hybrid_atomic_fetchand */

#ifndef __hybrid_atomic_fetchor
#ifdef __XBLOCK_hybrid_atomic_fetchop
#ifndef __impl_hybrid_atomic_fetchor_seqcst
#define __impl_hybrid_atomic_fetchor_seqcst(x,v) \
 __XBLOCK_hybrid_atomic_fetchop_seqcst(__hybrid_opfun_or,x,v)
#endif /* __impl_hybrid_atomic_fetchor_seqcst */
#define __hybrid_atomic_fetchor(x,v,order) \
  __XBLOCK_hybrid_atomic_fetchop(__hybrid_opfun_or,__impl_hybrid_atomic_fetchor_seqcst,x,v,order)
#else /* __XBLOCK_hybrid_atomic_fetchop */
#ifndef __impl_hybrid_atomic_fetchor_seqcst
#ifdef __CALL_hybrid_atomic_fetchop_seqcst
#define __impl_hybrid_atomic_fetchor_seqcst(x,v)  \
  __CALL_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchor,x,v)
#else /* __CALL_hybrid_atomic_fetchop_seqcst */
#define __impl_hybrid_atomic_fetchor_seqcst  __impl_hybrid_atomic_fetchor_seqcst
#endif /* !__CALL_hybrid_atomic_fetchop_seqcst */
__INLINE_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchor,__hybrid_opfun_or)
#endif /* __impl_hybrid_atomic_fetchor_seqcst */
__INLINE_hybrid_atomic_fetchop(__hybrid_atomic_fetchor,__impl_hybrid_atomic_fetchor_seqcst,__hybrid_opfun_or)
#ifdef __CALL_hybrid_atomic_fetchop
#define __hybrid_atomic_fetchor(x,v,order) \
  __CALL_hybrid_atomic_fetchop(__hybrid_atomic_fetchor,x,v,order)
#else /* __CALL_hybrid_atomic_fetchop */
#define __hybrid_atomic_fetchor __hybrid_atomic_fetchor
#endif /* !__CALL_hybrid_atomic_fetchop */
#endif /* !__XBLOCK_hybrid_atomic_fetchop */
#endif /* !__hybrid_atomic_fetchor */

#ifndef __hybrid_atomic_fetchxor
#ifdef __XBLOCK_hybrid_atomic_fetchop
#ifndef __impl_hybrid_atomic_fetchxor_seqcst
#define __impl_hybrid_atomic_fetchxor_seqcst(x,v) \
 __XBLOCK_hybrid_atomic_fetchop_seqcst(__hybrid_opfun_xor,x,v)
#endif /* __impl_hybrid_atomic_fetchxor_seqcst */
#define __hybrid_atomic_fetchxor(x,v,order) \
  __XBLOCK_hybrid_atomic_fetchop(__hybrid_opfun_xor,__impl_hybrid_atomic_fetchxor_seqcst,x,v,order)
#else /* __XBLOCK_hybrid_atomic_fetchop */
#ifndef __impl_hybrid_atomic_fetchxor_seqcst
#ifdef __CALL_hybrid_atomic_fetchop_seqcst
#define __impl_hybrid_atomic_fetchxor_seqcst(x,v)  \
  __CALL_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchxor,x,v)
#else /* __CALL_hybrid_atomic_fetchop_seqcst */
#define __impl_hybrid_atomic_fetchxor_seqcst  __impl_hybrid_atomic_fetchxor_seqcst
#endif /* !__CALL_hybrid_atomic_fetchop_seqcst */
__INLINE_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchxor,__hybrid_opfun_xor)
#endif /* __impl_hybrid_atomic_fetchxor_seqcst */
__INLINE_hybrid_atomic_fetchop(__hybrid_atomic_fetchxor,__impl_hybrid_atomic_fetchxor_seqcst,__hybrid_opfun_xor)
#ifdef __CALL_hybrid_atomic_fetchop
#define __hybrid_atomic_fetchxor(x,v,order) \
  __CALL_hybrid_atomic_fetchop(__hybrid_atomic_fetchxor,x,v,order)
#else /* __CALL_hybrid_atomic_fetchop */
#define __hybrid_atomic_fetchxor __hybrid_atomic_fetchxor
#endif /* !__CALL_hybrid_atomic_fetchop */
#endif /* !__XBLOCK_hybrid_atomic_fetchop */
#endif /* !__hybrid_atomic_fetchxor */

#ifndef __hybrid_atomic_fetchnand
#ifdef __XBLOCK_hybrid_atomic_fetchop
#ifndef __impl_hybrid_atomic_fetchnand_seqcst
#define __impl_hybrid_atomic_fetchnand_seqcst(x,v) \
 __XBLOCK_hybrid_atomic_fetchop_seqcst(__hybrid_opfun_nand,x,v)
#endif /* __impl_hybrid_atomic_fetchnand_seqcst */
#define __hybrid_atomic_fetchnand(x,v,order) \
  __XBLOCK_hybrid_atomic_fetchop(__hybrid_opfun_nand,__impl_hybrid_atomic_fetchnand_seqcst,x,v,order)
#else /* __XBLOCK_hybrid_atomic_fetchop */
#ifndef __impl_hybrid_atomic_fetchnand_seqcst
#ifdef __CALL_hybrid_atomic_fetchop_seqcst
#define __impl_hybrid_atomic_fetchnand_seqcst(x,v)  \
  __CALL_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchnand,x,v)
#else /* __CALL_hybrid_atomic_fetchop_seqcst */
#define __impl_hybrid_atomic_fetchnand_seqcst  __impl_hybrid_atomic_fetchnand_seqcst
#endif /* !__CALL_hybrid_atomic_fetchop_seqcst */
__INLINE_hybrid_atomic_fetchop_seqcst(__impl_hybrid_atomic_fetchnand,__hybrid_opfun_nand)
#endif /* __impl_hybrid_atomic_fetchnand_seqcst */
__INLINE_hybrid_atomic_fetchop(__hybrid_atomic_fetchnand,__impl_hybrid_atomic_fetchnand_seqcst,__hybrid_opfun_nand)
#ifdef __CALL_hybrid_atomic_fetchop
#define __hybrid_atomic_fetchnand(x,v,order) \
  __CALL_hybrid_atomic_fetchop(__hybrid_atomic_fetchnand,x,v,order)
#else /* __CALL_hybrid_atomic_fetchop */
#define __hybrid_atomic_fetchnand __hybrid_atomic_fetchnand
#endif /* !__CALL_hybrid_atomic_fetchop */
#endif /* !__XBLOCK_hybrid_atomic_fetchop */
#endif /* !__hybrid_atomic_fetchnand */

//[[[end]]]

#undef __DECL_INLINE_hybrid_atomic_fetchop
#undef __INLINE_hybrid_atomic_fetchop

#ifndef __hybrid_atomic_addfetch
#define __hybrid_atomic_addfetch(x,v,order) (__hybrid_atomic_fetchadd(x,v,order)+(v))
#endif /* !__hybrid_atomic_addfetch */
#ifndef __hybrid_atomic_subfetch
#define __hybrid_atomic_subfetch(x,v,order) (__hybrid_atomic_fetchadd(x,v,order)-(v))
#endif /* !__hybrid_atomic_subfetch */
#ifndef __hybrid_atomic_andfetch
#define __hybrid_atomic_andfetch(x,v,order) (__hybrid_atomic_fetchand(x,v,order)&(v))
#endif /* !__hybrid_atomic_andfetch */
#ifndef __hybrid_atomic_orfetch
#define __hybrid_atomic_orfetch(x,v,order)  (__hybrid_atomic_fetchor(x,v,order)|(v))
#endif /* !__hybrid_atomic_orfetch */
#ifndef __hybrid_atomic_xorfetch
#define __hybrid_atomic_xorfetch(x,v,order) (__hybrid_atomic_fetchxor(x,v,order)^(v))
#endif /* !__hybrid_atomic_xorfetch */
#ifndef __hybrid_atomic_nandfetch
#define __hybrid_atomic_nandfetch(x,v,order) (~(__hybrid_atomic_fetchnand(x,v,order)&(v)))
#endif /* !__hybrid_atomic_nandfetch */

#ifndef __hybrid_atomic_incfetch
#define __hybrid_atomic_incfetch(x,order) __hybrid_atomic_addfetch(x,1,order)
#define __hybrid_atomic_fetchinc(x,order) __hybrid_atomic_fetchadd(x,1,order)
#endif /* !__hybrid_atomic_fetchinc */

#ifndef __hybrid_atomic_decfetch
#define __hybrid_atomic_decfetch(x,order) __hybrid_atomic_subfetch(x,1,order)
#define __hybrid_atomic_fetchdec(x,order) __hybrid_atomic_fetchsub(x,1,order)
#endif /* !__hybrid_atomic_decfetch */

#endif /* __CC__ */

__SYSDECL_END

#ifdef _MSC_VER
#ifndef __cplusplus
#pragma warning(pop)
#endif /* __cplusplus */
#endif

#endif /* !__GUARD_HYBRID___ATOMIC_H */
