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
#ifndef GUARD_ARCH_I386_KOS_INCLUDE_ARCH_PERCPU_H
#define GUARD_ARCH_I386_KOS_INCLUDE_ARCH_PERCPU_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>

DECL_BEGIN

#ifdef __CC__
#define CPU_OFFSETOF(sym)        ((__UINTPTR_TYPE__)&(sym))
#define CPUTMPL_ADDR(offset)     (__percpu_template+((offset)))
#define THATCPU_ADDR(cpu,offset) ((__BYTE_TYPE__ *)(cpu)+(offset))

#ifdef __INTELLISENSE__
#define THISCPU_ADDR(offset)     ((__BYTE_TYPE__ *)(offset))
#define __THISCPU_GET(T,s,offset)           (*(T *)(offset))
#define __THISCPU_PUT(T,s,offset,val) (void)(*(T *)(offset) = (val))
#elif defined(__x86_64__)
#define THISCPU_ADDR(offset)  \
 XBLOCK({ register __BYTE_TYPE__ *__res; \
          __asm__ __volatile__("add {%%gs:" PP_STR(CPU_OFFSETOF_SELF) ", %0|%0, gs:" PP_STR(CPU_OFFSETOF_SELF) "}\n" \
                               : "=r" (__res) : "0" (offset)); \
          XRETURN __res; \
 })
#ifdef __SEG_GS
#define __THISCPU_GET(T,s,offset)           (*(T __seg_gs *)(offset))
#define __THISCPU_PUT(T,s,offset,val) (void)(*(T __seg_gs *)(offset) = (val))
#else
#define __THISCPU_GET(T,s,offset) \
 XBLOCK({ register T __res; \
          __asm__ __volatile__("mov" s " {%%gs:%c1, %0|%0, gs:%c1}\n" \
                               : "=g" (__res) : "ir" (offset)); \
          XRETURN __res; \
 })
#define __THISCPU_PUT(T,s,offset,val) \
 XBLOCK({ __asm__ __volatile__("mov" s " {%0, %%gs:%c1|gs:%c1, %0}\n" \
                               : : "g" (val), "ir" (offset)); \
          (void)0; \
 })
#endif
#elif defined(__i386__)
#define THISCPU_ADDR(offset)  \
 XBLOCK({ register __BYTE_TYPE__ *__res; \
          __asm__ __volatile__("add {%%fs:" PP_STR(CPU_OFFSETOF_SELF) ", %0|%0, fs:" PP_STR(CPU_OFFSETOF_SELF) "}\n" \
                               : "=g" (__res) : "0" (offset)); \
          XRETURN __res; \
 })
#ifdef __SEG_FS
#define __THISCPU_GET(T,s,offset)           (*(T __seg_fs *)(offset))
#define __THISCPU_PUT(T,s,offset,val) (void)(*(T __seg_fs *)(offset) = (val))
#else
#define __THISCPU_GET(T,s,offset) \
 XBLOCK({ register T __res; \
          __asm__ __volatile__("mov" s " {%%fs:%c1, %0|%0, fs:%c1}\n" \
                               : "=g" (__res) : "ir" (offset)); \
          XRETURN __res; \
 })
#define __THISCPU_PUT(T,s,offset,val) \
 XBLOCK({ __asm__ __volatile__("mov" s " {%0, %%fs:%c1|fs:%c1, %0}\n" \
                               : : "g" (val), "ir" (offset)); \
          (void)0; \
 })
#endif
#endif
#define THISCPU_T_GETB(T,offset)     __THISCPU_GET(T,"b",offset)
#define THISCPU_T_GETW(T,offset)     __THISCPU_GET(T,"w",offset)
#define THISCPU_T_GETL(T,offset)     __THISCPU_GET(T,"l",offset)
#define THISCPU_T_PUTB(T,offset,val) __THISCPU_PUT(T,"b",offset,val)
#define THISCPU_T_PUTW(T,offset,val) __THISCPU_PUT(T,"w",offset,val)
#define THISCPU_T_PUTL(T,offset,val) __THISCPU_PUT(T,"l",offset,val)
#ifdef __x86_64__
#define THISCPU_T_GETQ(T,offset)     __THISCPU_GET(T,"q",offset)
#define THISCPU_T_PUTQ(T,offset,val) __THISCPU_PUT(T,"q",offset,val)
#endif

#ifdef __i386__
#   define CPU_BASEREGISTER fs
#else
#   define CPU_BASEREGISTER gs
#endif
#endif /* __CC__ */

/* taking a register, memory location, or integral constant without
 * '$'-prefix as argument describing an offset from '', generate a valid operand describing the given
 * offset as a cpu-local memory location. */
#define ASM_CPU(o)   %CPU_BASEREGISTER:o
#define ASM_CPU2(o) %%CPU_BASEREGISTER:o

DECL_END

#endif /* !GUARD_ARCH_I386_KOS_INCLUDE_ARCH_PERCPU_H */
