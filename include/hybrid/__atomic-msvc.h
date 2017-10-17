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
#ifndef __GUARD_HYBRID___ATOMIC_MSVC_H
#define __GUARD_HYBRID___ATOMIC_MSVC_H 1

#include <__stdinc.h>

#ifndef __GUARD_HYBRID___ATOMIC_H
#error "Never include this file directly. - Always include `<hybrid/__atomic.h>' instead"
#endif

__DECL_BEGIN

#ifdef _M_X64
#define __CDECL_OR_DEFAULT /* nothing */
#else
#define __CDECL_OR_DEFAULT __cdecl
#endif

extern char (_InterlockedCompareExchange8)(char volatile *__px, char __newv, char __oldv);
extern short (_InterlockedCompareExchange16)(short volatile *__px, short __newv, short __oldv);
extern long (__CDECL_OR_DEFAULT _InterlockedCompareExchange)(long volatile *__px, long __newv, long __oldv);
extern __int64 (_InterlockedCompareExchange64)(__int64 volatile *__px, __int64 __newv, __int64 __oldv);
#pragma intrinsic(_InterlockedCompareExchange8)
#pragma intrinsic(_InterlockedCompareExchange16)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedCompareExchange64)

#ifdef __cplusplus
extern "C++" { template<class __T, class __OV, class __NV>
#define __impl_hybrid_atomic_cmpxch_val_seqcst __impl_hybrid_atomic_cmpxch_val_seqcst
__T __FORCELOCAL (__impl_hybrid_atomic_cmpxch_val_seqcst)(__T &__x, __OV __oldv, __NV __newv) {
    __STATIC_IF(sizeof(__T) == 1) { return (__T)_InterlockedCompareExchange8((char volatile *)&__x,(char)__newv,(char)__oldv); } __STATIC_ELSE(sizeof(__T) == 1) {
    __STATIC_IF(sizeof(__T) == 2) { return (__T)_InterlockedCompareExchange16((short volatile *)&__x,(short)__newv,(short)__oldv); } __STATIC_ELSE(sizeof(__T) == 2) {
    __STATIC_IF(sizeof(__T) == 4) { return (__T)_InterlockedCompareExchange((long volatile *)&__x,(long)__newv,(long)__oldv); } __STATIC_ELSE(sizeof(__T) == 4) {
    return (__T)_InterlockedCompareExchange64((__int64 volatile *)&__x,(__int64)__newv,(__int64)__oldv);
} } } } }
#else /* __cplusplus */
#define __impl_hybrid_atomic_cmpxch_val_seqcst(x,oldv,newv) \
  __TYPEOF_RECAST(x,sizeof(x) == 1 ? (__UINT8_TYPE__)_InterlockedCompareExchange8((char volatile *)&(x),(char)(newv),(char)(oldv)) : \
                    sizeof(x) == 2 ? (__UINT16_TYPE__)_InterlockedCompareExchange16((short volatile *)&(x),(short)(newv),(short)(oldv)) : \
                    sizeof(x) == 4 ? (__UINT32_TYPE__)_InterlockedCompareExchange((long volatile *)&(x),(long)(newv),(long)(oldv)) : \
                                     (__UINT64_TYPE__)_InterlockedCompareExchange64((__int64 volatile *)&(x),(__int64)(newv),(__int64)(oldv)))
#endif /* !__cplusplus */


/* TODO: Implement these for much faster atomic operations. */
extern char (_InterlockedAnd8)(char volatile *__px, char __v);
extern short (_InterlockedAnd16)(short volatile *__px, short __v);
extern long (_InterlockedAnd)(long volatile *__px, long __v);

extern short (_InterlockedDecrement16)(short volatile *__px);
extern long (__CDECL_OR_DEFAULT _InterlockedDecrement)(long volatile *__px);
extern short (_InterlockedIncrement16)(short volatile *__px);
extern long (__CDECL_OR_DEFAULT _InterlockedIncrement)(long volatile *__px);

extern char (_InterlockedExchange8)(char volatile *__px, char __v);
extern short (_InterlockedExchange16)(short volatile *__px, short __v);
extern long (__CDECL_OR_DEFAULT _InterlockedExchange)(long volatile *__px, long __v);

extern char (_InterlockedExchangeAdd8)(char volatile *__px, char __v);
extern short (_InterlockedExchangeAdd16)(short volatile *__px, short __v);
extern long (__CDECL_OR_DEFAULT _InterlockedExchangeAdd)(long volatile *__px, long __v);

extern char (_InterlockedOr8)(char volatile *__px, char __v);
extern short (_InterlockedOr16)(short volatile *__px, short __v);
extern long (_InterlockedOr)(long volatile *__px, long __v);

extern char (_InterlockedXor8)(char volatile *__px, char __v);
extern short (_InterlockedXor16)(short volatile *__px, short __v);
extern long (_InterlockedXor)(long volatile *__px, long __v);


#pragma intrinsic(_InterlockedAnd8)
#pragma intrinsic(_InterlockedAnd)
#pragma intrinsic(_InterlockedAnd16)
#pragma intrinsic(_InterlockedDecrement16)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedIncrement16)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedExchange8)
#pragma intrinsic(_InterlockedExchange16)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangeAdd8)
#pragma intrinsic(_InterlockedExchangeAdd16)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedOr8)
#pragma intrinsic(_InterlockedOr16)
#pragma intrinsic(_InterlockedOr)
#pragma intrinsic(_InterlockedXor8)
#pragma intrinsic(_InterlockedXor16)
#pragma intrinsic(_InterlockedXor)


__DECL_END

#endif /* !__GUARD_HYBRID___ATOMIC_MSVC_H */
