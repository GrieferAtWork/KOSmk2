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
#ifndef GUARD_ARCH_I386_KOS_INCLUDE_ARCH_FPU_H
#define GUARD_ARCH_I386_KOS_INCLUDE_ARCH_FPU_H 1

#include <hybrid/compiler.h>
#ifndef CONFIG_NO_FPU
#include <hybrid/typecore.h>
#include <hybrid/types.h>
#include <hybrid/host.h>
#include <asm/cpu-flags.h>

DECL_BEGIN

#define FPUSTATE_SIZE  512
#define FPUSTATE_ALIGN 16

#ifdef __CC__

struct fpu_reg {
 /* ST(i) / MMi register. */
 u8 f_data[10];
 u8 f_pad[6];
};

#define FPUSTATE_LOAD(s)   __asm__ __volatile__("fxrstor %0\n" : : "m" (*(struct fpustate *)&(s)))
#define FPUSTATE_SAVE(s)   __asm__ __volatile__("fxsave %0\n" : "=m" (*(struct fpustate *)&(s)))
#define FPUSTATE_INIT()    __asm__ __volatile__("fninit\n")
#define FPUSTATE_ENABLE()  __asm__ __volatile__("clts\n")
#define FPUSTATE_DISABLE() \
 XBLOCK({ register register_t _temp; \
          __asm__ __volatile__("mov %%cr0, %0\n" \
                               "or  $(" __PP_STR(CR0_TS) "), %0\n" \
                               "mov %0, %%cr0\n" \
                               : "=&r" (_temp)); \
          (void)0; })

ATTR_ALIGNED(FPUSTATE_ALIGN)
struct fpustate {
 /* FPU state structure, as described here: 
  *   - http://asm.inightmare.org/opcodelst/index.php?op=FXSAVE
  *   - http://x86.renejeschke.de/html/file_module_x86_id_128.html */
 u16            fp_fcw;   /*< Floating point control word. */
 u16            fp_fsw;   /*< Floating point status word. */
 u8             fp_ftw;   /*< Floating point tag word. */
 u8             fp_res0;
 u16            fp_fop;   /*< Lower 11-bit f.p. opcode. */
 u32            fp_fpuip; /*< FPU instruction pointer. */
 u16            fp_fpucs; /*< FPU code segment selector. */
 u16            fp_res1;
 u32            fp_fpudp; /*< FPU data pointer. */
 u16            fp_fpuds; /*< FPU data segment selector. */
 u16            fp_res2;
 u32            fp_mxcsr;
 u32            fp_mxcsr_mask;
 struct fpu_reg fp_regs[8];
#ifdef __x86_64__
 struct fpu_reg fp_xmm[16];
 u8             fp_res3[96];
#else
 struct fpu_reg fp_xmm[8];
 u8             fp_res3[224];
#endif
};

/* Allocate/discard FPU states. */
#define FPUSTATE_ALLOC() \
      ((struct fpustate *)kmemalign(FPUSTATE_ALIGN,FPUSTATE_SIZE,GFP_SHARED))
#define FPUSTATE_FREE(p) free(p)
#define FPUSTATE_NULL    NULL

struct task;
/* [0..1] The task associated with the current
 *        FPU register contents, or NULL if none. */
DATDEF PERCPU struct task *fpu_current;

#endif /* __CC__ */

DECL_END

#endif /* !CONFIG_NO_FPU */

#endif /* !GUARD_ARCH_I386_KOS_INCLUDE_ARCH_FPU_H */
