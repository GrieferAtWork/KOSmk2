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
#ifndef GUARD_INCLUDE_SCHED_PERCPU_H
#define GUARD_INCLUDE_SCHED_PERCPU_H 1

#include <hybrid/compiler.h>
#ifdef __CC__
#include <hybrid/types.h>
#include <hybrid/host.h>
#include <sched/types.h>
#endif /* __CC__ */

DECL_BEGIN

#ifdef __CC__
/* PER-cpu template symbols. */
extern byte_t const __percpu_template[]; /*< Per-cpu data template, containing `__percpu_datsize' size bytes. */
extern byte_t __percpu_datsize[];        /*< [size_t] Amount of data bytes that should be copied from `__percpu_template'. */
extern byte_t __percpu_bsssize[];        /*< [size_t] Amount of zero-initialized bytes following `__percpu_datsize'. */
extern byte_t __percpu_allsize[];        /*< [size_t] == __percpu_datsize + __percpu_bsssize. */
extern byte_t __percpu_dat_dwords[];     /*< == __percpu_datsize / 4 */
extern byte_t __percpu_bss_dwords[];     /*< == __percpu_bsssize / 4 */
extern byte_t __percpu_all_dwords[];     /*< == __percpu_allsize / 4 */
#endif /* __CC__ */

#define PERCPU_TEMPLATE   __percpu_template   /*< The starting address of the per-cpu data template. */
#define PERCPU_DAT_DWORDS __percpu_dat_dwords /*< Amount of DWORDs that must be copied from `PERCPU_TEMPLATE'. */
#define PERCPU_BSS_DWORDS __percpu_bss_dwords /*< After `PERCPU_DAT_DWORDS': Amount of DWORDs that must be ZERO-initialized. */
#define PERCPU_ALL_DWORDS __percpu_all_dwords /*< The sum of `PERCPU_DAT_DWORDS' and `PERCPU_BSS_DWORDS' */

#ifdef __CC__
#ifdef CONFIG_BUILDING_KERNEL_CORE
/* BOOT-cpu location symbols. */
extern byte_t __bootcpu_begin[];
extern byte_t __bootcpu_end[];
#endif
#endif /* __CC__ */

/* Per-cpu data.
 * WARNING: .bss data must not be initialized. */
#define CPU_TEXT   PERCPU ATTR_SECTION(".text.cpu")
#define CPU_RODATA PERCPU ATTR_SECTION(".rodata.cpu")
#define CPU_DATA   PERCPU ATTR_SECTION(".data.cpu")
#define CPU_BSS    PERCPU ATTR_SECTION(".bss.cpu")


#define PERCPU_ALIGNMENT 16
#define ALIGNED_CPUSIZE  ((CPU_SIZE+(PERCPU_ALIGNMENT-1)) & ~(PERCPU_ALIGNMENT-1))
#if ALIGNED_CPUSIZE == 272
#   undef ALIGNED_CPUSIZE
#   define ALIGNED_CPUSIZE 272
#elif ALIGNED_CPUSIZE == 384
#   undef ALIGNED_CPUSIZE
#   define ALIGNED_CPUSIZE 384
#endif


#ifdef __CC__
#define CPU_OFFSETOF(sym)        ((uintptr_t)&(sym))
#define CPUTMPL_ADDR(offset)     (__percpu_template+((offset)))
#define THATCPU_ADDR(cpu,offset) ((byte_t *)(cpu)+(offset))

#ifdef __INTELLISENSE__
#define THISCPU_ADDR(offset)            ((byte_t *)(offset))
#define __THISCPU_GET(T,s,offset)           (*(T *)(offset))
#define __THISCPU_PUT(T,s,offset,val) (void)(*(T *)(offset) = (val))
#elif defined(__i386__)
#define THISCPU_ADDR(offset)  \
 XBLOCK({ register byte_t *__res; \
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
#else
#define THISCPU_ADDR(offset)  \
 XBLOCK({ register byte_t *__res; \
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

#ifndef THISCPU_T_GETB
#define THISCPU_T_GETB(T,offset)           (*(T *)THISCPU_ADDR(offset))
#define THISCPU_T_GETW(T,offset)           (*(T *)THISCPU_ADDR(offset))
#define THISCPU_T_GETL(T,offset)           (*(T *)THISCPU_ADDR(offset))
#endif
#ifndef THISCPU_T_PUTB
#define THISCPU_T_PUTB(T,offset,val) (void)(*(T *)THISCPU_ADDR(offset) = (val))
#define THISCPU_T_PUTW(T,offset,val) (void)(*(T *)THISCPU_ADDR(offset) = (val))
#define THISCPU_T_PUTL(T,offset,val) (void)(*(T *)THISCPU_ADDR(offset) = (val))
#endif

#ifndef THISCPU_T_GETQ
#define EMULATED_THISCPU_GETQ
#define THISCPU_T_GETQ(T,offset)           (*(T *)THISCPU_ADDR(offset))
#endif
#ifndef THISCPU_T_PUTQ
#define EMULATED_THISCPU_PUTQ
#define THISCPU_T_PUTQ(T,offset,val) (void)(*(T *)THISCPU_ADDR(offset) = (val))
#endif

#if __SIZEOF_POINTER__ == 4
#   define THISCPU_T_GETI THISCPU_T_GETL
#   define THISCPU_T_PUTI THISCPU_T_PUTL
#elif __SIZEOF_POINTER__ == 8
#   define THISCPU_T_GETI THISCPU_T_GETQ
#   define THISCPU_T_PUTI THISCPU_T_PUTQ
#else
#   error FIXME
#endif

#define THISCPU_GETB(offset)     THISCPU_T_GETB(u8,offset)
#define THISCPU_GETW(offset)     THISCPU_T_GETW(u16,offset)
#define THISCPU_GETL(offset)     THISCPU_T_GETL(u32,offset)
#define THISCPU_GETQ(offset)     THISCPU_T_GETQ(u64,offset)
#define THISCPU_GETI(offset)     THISCPU_T_GETI(uintptr_t,offset)
#define THISCPU_PUTB(offset,val) THISCPU_T_PUTB(u8,offset,val)
#define THISCPU_PUTW(offset,val) THISCPU_T_PUTW(u16,offset,val)
#define THISCPU_PUTL(offset,val) THISCPU_T_PUTL(u32,offset,val)
#define THISCPU_PUTQ(offset,val) THISCPU_T_PUTQ(u64,offset,val)
#define THISCPU_PUTI(offset,val) THISCPU_T_PUTI(uintptr_t,offset,val)

#ifdef __i386__
#   define CPU_BASEREGISTER fs
#else
#   define CPU_BASEREGISTER gs
#endif


/* taking a register, memory location, or integral constant without
 * '$'-prefix as argument describing an offset from '', generate a valid operand describing the given
 * offset as a cpu-local memory location. */
#define ASM_CPU(o)   %CPU_BASEREGISTER:o
#define ASM_CPU2(o) %%CPU_BASEREGISTER:o


/* >> T &CPU(T &x);
 *    Translate a given l-value expression `x' for direct access of per-cpu data.
 *   (Essentially, this simply adds the base-address of the per-cpu segment to `&x') */
#define CPU(x)          (*(__typeof__(&(x)))THISCPU_ADDR(CPU_OFFSETOF(x)))
#define CPU_TEMPLATE(x) (*(__typeof__(&(x)))CPUTMPL_ADDR(CPU_OFFSETOF(x)))
#define VCPU(cpu,x)     (*(__typeof__(&(x)))THATCPU_ADDR(cpu,CPU_OFFSETOF(x)))

#ifdef __INTELLISENSE__
/* Smaller code; same semantics... */
extern "C++" { template<class T> T ____INTELLISENSE_CPU_READ(T &x); }
#define CPU_READ        ____INTELLISENSE_CPU_READ
#define CPU_WRITE(x,v) (void)((x) = (v))
#else /* IDE... */

#if !defined(EMULATED_THISCPU_GETQ)
#define CPU_READ(x) \
 __builtin_choose_expr(sizeof(x) == 1,THISCPU_T_GETB(__typeof__(x),CPU_OFFSETOF(x)), \
 __builtin_choose_expr(sizeof(x) == 2,THISCPU_T_GETW(__typeof__(x),CPU_OFFSETOF(x)), \
 __builtin_choose_expr(sizeof(x) == 4,THISCPU_T_GETL(__typeof__(x),CPU_OFFSETOF(x)), \
 __builtin_choose_expr(sizeof(x) == 8,THISCPU_T_GETQ(__typeof__(x),CPU_OFFSETOF(x)), \
                                   *(__typeof__(&(x)))THISCPU_ADDR(CPU_OFFSETOF(x))))))
#define CPU_WRITE(x,v) \
 __builtin_choose_expr(sizeof(x) == 1,THISCPU_T_PUTB(__typeof__(x),CPU_OFFSETOF(x),v), \
 __builtin_choose_expr(sizeof(x) == 2,THISCPU_T_PUTW(__typeof__(x),CPU_OFFSETOF(x),v), \
 __builtin_choose_expr(sizeof(x) == 4,THISCPU_T_PUTL(__typeof__(x),CPU_OFFSETOF(x),v), \
 __builtin_choose_expr(sizeof(x) == 8,THISCPU_T_PUTQ(__typeof__(x),CPU_OFFSETOF(x),v), \
                      (void)(*(__typeof__(&(x)))THISCPU_ADDR(CPU_OFFSETOF(x)) = (v))))))
#else
#define CPU_READ(x) \
 __builtin_choose_expr(sizeof(x) == 1,THISCPU_T_GETB(__typeof__(x),CPU_OFFSETOF(x)), \
 __builtin_choose_expr(sizeof(x) == 2,THISCPU_T_GETW(__typeof__(x),CPU_OFFSETOF(x)), \
 __builtin_choose_expr(sizeof(x) == 4,THISCPU_T_GETL(__typeof__(x),CPU_OFFSETOF(x)), \
                                   *(__typeof__(&(x)))THISCPU_ADDR(CPU_OFFSETOF(x)))))
#define CPU_WRITE(x,v) \
 __builtin_choose_expr(sizeof(x) == 1,THISCPU_T_PUTB(__typeof__(x),CPU_OFFSETOF(x),v), \
 __builtin_choose_expr(sizeof(x) == 2,THISCPU_T_PUTW(__typeof__(x),CPU_OFFSETOF(x),v), \
 __builtin_choose_expr(sizeof(x) == 4,THISCPU_T_PUTL(__typeof__(x),CPU_OFFSETOF(x),v), \
                      (void)(*(__typeof__(&(x)))THISCPU_ADDR(CPU_OFFSETOF(x)) = (v)))))
#endif

#endif /* !IDE... */

#define THIS_CPU   THISCPU_T_GETI(struct cpu *,CPU_OFFSETOF_SELF)
#define THIS_TASK  THISCPU_T_GETI(struct task *,CPU_OFFSETOF_RUNNING)
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_INCLUDE_SCHED_PERCPU_H */
