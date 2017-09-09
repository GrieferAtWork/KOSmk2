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
#ifndef __GUARD_HYBRID_ATOMIC_H
#define __GUARD_HYBRID_ATOMIC_H 1

#include "compiler.h"

DECL_BEGIN

#ifdef __CC__
#ifndef __memory_order_defined
#define __memory_order_defined 1
typedef enum {
 memory_order_relaxed = __ATOMIC_RELAXED,
 memory_order_consume = __ATOMIC_CONSUME,
 memory_order_acquire = __ATOMIC_ACQUIRE,
 memory_order_release = __ATOMIC_RELEASE,
 memory_order_acq_rel = __ATOMIC_ACQ_REL,
 memory_order_seq_cst = __ATOMIC_SEQ_CST,
} memory_order;
#endif

#define OATOMIC_LOAD(x,order)                          __atomic_load_n(&(x),order)
#define OATOMIC_STORE(x,v,order)                       __atomic_store_n(&(x),v,order)
#define OATOMIC_XCH(x,v,order)                         __atomic_exchange_n(&(x),v,order)
#define OATOMIC_CMPXCH(x,oldv,newv,succ,fail)          XBLOCK({ __typeof__(x) __oldv=(oldv); XRETURN __atomic_compare_exchange_n(&(x),&__oldv,newv,0,fail,succ); })
#define OATOMIC_CMPXCH_WEAK(x,oldv,newv,succ,fail)     XBLOCK({ __typeof__(x) __oldv=(oldv); XRETURN __atomic_compare_exchange_n(&(x),&__oldv,newv,1,fail,succ); })
#define OATOMIC_CMPXCH_VAL(x,oldv,newv,succ,fail)      XBLOCK({ __typeof__(x) __oldv=(oldv); __atomic_compare_exchange_n(&(x),&__oldv,newv,0,fail,succ); XRETURN __oldv; })
#define OATOMIC_CMPXCH_VAL_WEAK(x,oldv,newv,succ,fail) XBLOCK({ __typeof__(x) __oldv=(oldv); __atomic_compare_exchange_n(&(x),&__oldv,newv,1,fail,succ); XRETURN __oldv; })
#define OATOMIC_ADDFETCH(x,v,order)                    __atomic_add_fetch(&(x),v,order)
#define OATOMIC_SUBFETCH(x,v,order)                    __atomic_sub_fetch(&(x),v,order)
#define OATOMIC_ANDFETCH(x,v,order)                    __atomic_and_fetch(&(x),v,order)
#define OATOMIC_ORFETCH(x,v,order)                     __atomic_or_fetch(&(x),v,order)
#define OATOMIC_XORFETCH(x,v,order)                    __atomic_xor_fetch(&(x),v,order)
#define OATOMIC_NANDFETCH(x,v,order)                   __atomic_nand_fetch(&(x),v,order)
#define OATOMIC_FETCHADD(x,v,order)                    __atomic_fetch_add(&(x),v,order)
#define OATOMIC_FETCHSUB(x,v,order)                    __atomic_fetch_sub(&(x),v,order)
#define OATOMIC_FETCHAND(x,v,order)                    __atomic_fetch_and(&(x),v,order)
#define OATOMIC_FETCHOR(x,v,order)                     __atomic_fetch_or(&(x),v,order)
#define OATOMIC_FETCHXOR(x,v,order)                    __atomic_fetch_xor(&(x),v,order)
#define OATOMIC_FETCHNAND(x,v,order)                   __atomic_fetch_nand(&(x),v,order)
#define OATOMIC_INCFETCH(x,order)                      OATOMIC_ADDFETCH(x,1,order)
#define OATOMIC_DECFETCH(x,order)                      OATOMIC_SUBFETCH(x,1,order)
#define OATOMIC_FETCHINC(x,order)                      OATOMIC_FETCHADD(x,1,order)
#define OATOMIC_FETCHDEC(x,order)                      OATOMIC_FETCHSUB(x,1,order)

#define ATOMIC_LOAD(x)                      OATOMIC_LOAD(x,__ATOMIC_SEQ_CST)
#define ATOMIC_STORE(x,v)                   OATOMIC_STORE(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_XCH(x,v)                     OATOMIC_XCH(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_CMPXCH(x,oldv,newv)          OATOMIC_CMPXCH(x,oldv,newv,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)
#define ATOMIC_CMPXCH_WEAK(x,oldv,newv)     OATOMIC_CMPXCH_WEAK(x,oldv,newv,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)
#define ATOMIC_CMPXCH_VAL(x,oldv,newv)      OATOMIC_CMPXCH_VAL(x,oldv,newv,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)
#define ATOMIC_CMPXCH_VAL_WEAK(x,oldv,newv) OATOMIC_CMPXCH_VAL_WEAK(x,oldv,newv,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)
#define ATOMIC_ADDFETCH(x,v)                OATOMIC_ADDFETCH(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_SUBFETCH(x,v)                OATOMIC_SUBFETCH(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_ANDFETCH(x,v)                OATOMIC_ANDFETCH(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_ORFETCH(x,v)                 OATOMIC_ORFETCH(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_XORFETCH(x,v)                OATOMIC_XORFETCH(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_NANDFETCH(x,v)               OATOMIC_NANDFETCH(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_FETCHADD(x,v)                OATOMIC_FETCHADD(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_FETCHSUB(x,v)                OATOMIC_FETCHSUB(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_FETCHAND(x,v)                OATOMIC_FETCHAND(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_FETCHOR(x,v)                 OATOMIC_FETCHOR(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_FETCHXOR(x,v)                OATOMIC_FETCHXOR(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_FETCHNAND(x,v)               OATOMIC_FETCHNAND(x,v,__ATOMIC_SEQ_CST)
#define ATOMIC_INCFETCH(x)                  OATOMIC_INCFETCH(x,__ATOMIC_SEQ_CST)
#define ATOMIC_DECFETCH(x)                  OATOMIC_DECFETCH(x,__ATOMIC_SEQ_CST)
#define ATOMIC_FETCHINC(x)                  OATOMIC_FETCHINC(x,__ATOMIC_SEQ_CST)
#define ATOMIC_FETCHDEC(x)                  OATOMIC_FETCHDEC(x,__ATOMIC_SEQ_CST)

#define ATOMIC_INCIFNONZERO(x) \
 XBLOCK({ register __typeof__(x) _temp; \
          do { _temp = ATOMIC_READ(x); \
               if (!_temp) break; \
          } while (!ATOMIC_CMPXCH_WEAK(x,_temp,_temp+1)); \
          XRETURN _temp != 0; \
 })


#define ATOMIC_READ(x)     OATOMIC_LOAD(x,__ATOMIC_ACQUIRE)
#define ATOMIC_WRITE(x,v)  OATOMIC_STORE(x,v,__ATOMIC_RELEASE)

#endif /* __CC__ */

DECL_END

#endif /* !__GUARD_HYBRID_ATOMIC_H */
