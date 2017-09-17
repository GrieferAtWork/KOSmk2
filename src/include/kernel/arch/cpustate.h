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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_CPUSTATE_H
#define GUARD_INCLUDE_KERNEL_ARCH_CPUSTATE_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

#define __COMMON_REG1(n) union{ u32 e##n##x; u16 n##x; struct { u8 n##l,n##h; }; };
#define __COMMON_REG2(n) union{ u32 e##n; u16 n; };

/* General-purpose registers. */
/* HINT: pushad/popad-compatible */
#define __COMMON_32BIT    \
        __COMMON_REG2(di) \
        __COMMON_REG2(si) \
        __COMMON_REG2(bp) \
        __COMMON_REG2(sp) \
        __COMMON_REG1(b)  \
        __COMMON_REG1(d)  \
        __COMMON_REG1(c)  \
        __COMMON_REG1(a) 


struct PACKED cpustate16 {
 /* Common CPU state. */
 __COMMON_32BIT
 u16 gs,fs,es,ds,ss;
 u32 eflags;
};

struct PACKED ccpustate {
 /* Common CPU state. */
 __COMMON_32BIT
 u16 gs,fs,es,ds;     /* Segment registers */
};


struct PACKED host_cpustate {
union PACKED{
 struct ccpustate com;
struct PACKED{
 __COMMON_32BIT
 u16 gs,fs,es,ds;         /* Segment registers */
};};
 /* iret-compatible return block (place ESP on '&eip' and execute 'iret'). */
 u32 eip;                 /* iret tail: Instruction pointer. */
 u16 cs,_n1; u32 eflags;  /* iret tail: misc. data */
};
struct PACKED cpustate {
 struct host_cpustate host; /* host CPU state. */
 u32 useresp; u16 ss,_n2;   /* [valid_if(CPUSTATE_ISUSER(this))]
                             * iret tail: user-space stack pointer. */
};


struct PACKED host_cpustate_irq_c {
union PACKED{
 struct ccpustate com;
struct PACKED{
 __COMMON_32BIT
 u16 gs,fs,es,ds;         /* Segment registers */
};};
 u32 exc_code;            /* Exception code (Must be popped). */
 /* iret-compatible return block (place ESP on '&eip' and execute 'iret'). */
 u32 eip;                 /* iret tail: Instruction pointer. */
 u16 cs,_n1; u32 eflags;  /* iret tail: misc. data */
};

struct PACKED cpustate_irq_c {
 struct host_cpustate_irq_c host; /* host CPU state. */
 u32 useresp; u16 ss,_n2;         /* [valid_if(CPUSTATE_ISUSER(this))]
                                   * iret tail: user-space stack pointer. */
};

#define CPUSTATE_ISUSER(x) (((x)->host.cs&3) == 3)

#undef __COMMON_32BIT
#undef __COMMON_REG2
#undef __COMMON_REG1

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_CPUSTATE_H */
