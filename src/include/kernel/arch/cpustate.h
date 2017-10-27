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

#define __COMMON_REG1(n) union PACKED { u32 e##n##x; u16 n##x; struct PACKED { u8 n##l,n##h; }; };
#define __COMMON_REG2(n) union PACKED { u32 e##n; u16 n; };

/* General purpose registers. */
/* HINT: pushad/popad-compatible */
struct PACKED gpregs {
 __COMMON_REG2(di)
 __COMMON_REG2(si)
 __COMMON_REG2(bp)
 __COMMON_REG2(sp)
 __COMMON_REG1(b)
 __COMMON_REG1(d)
 __COMMON_REG1(c)
 __COMMON_REG1(a)
};
#undef __COMMON_REG2
#undef __COMMON_REG1

#define __SEGMENT32(x) union PACKED { struct PACKED { u16 x##16,__##x##16hi; }; u32 x; }

/* Segment registers */
struct PACKED sgregs { u16 gs,fs,es,ds; };

/* Interrupt return registers (_MUST_ be iret-compatible). */
#define IRREGS_ISUSER(x) (((x).cs&3) == 3) /* When true, must use `struct irregs' */
struct PACKED irregs_host   { u32 eip; __SEGMENT32(cs); u32 eflags; };
struct PACKED irregs        { union PACKED { struct irregs_host host; struct PACKED {
                              u32 eip; __SEGMENT32(cs); u32 eflags; };};
                              u32 useresp; __SEGMENT32(ss); };
struct PACKED irregs_host_e { u32 exc_code; union PACKED { struct irregs_host tail; struct PACKED {
                              u32 eip; __SEGMENT32(cs); u32 eflags; };};};
struct PACKED irregs_e      { union PACKED { struct irregs_host_e host; struct PACKED {
                              u32 exc_code; union PACKED { struct irregs tail; struct PACKED {
                              u32 eip; __SEGMENT32(cs); u32 eflags;
                              u32 useresp; __SEGMENT32(ss); };};};};};
#undef __SEGMENT32

/* CPU state for 16-bit realmode interrupts. */
struct PACKED cpustate16 {
 struct gpregs gp;
 struct sgregs sg;
 u16           ss;
 u32           eflags;
};

/* Common CPU state. */
struct PACKED comregs {
 struct gpregs gp;
 struct sgregs sg;
};


struct PACKED host_cpustate {
union PACKED {
 struct comregs com;
struct PACKED {
 struct gpregs gp;
 struct sgregs sg;
};};
 struct irregs_host iret;
};
struct PACKED cpustate {union PACKED {
 struct host_cpustate host; /* host CPU state. */
struct PACKED {
union PACKED {
 struct comregs com;
struct PACKED {
 struct gpregs gp;
 struct sgregs sg;
};};
 struct irregs iret;
};};};


struct PACKED host_cpustate_e {
union PACKED {
 struct comregs com;
struct PACKED {
 struct gpregs gp;
 struct sgregs sg;
};};
 struct irregs_host_e iret;
};

struct PACKED cpustate_e {union PACKED {
 struct host_cpustate_e host; /* host CPU state. */
struct PACKED {
union PACKED {
 struct comregs  com;
struct PACKED {
 struct gpregs   gp;
 struct sgregs   sg;
};};
 struct irregs_e iret;
};};};

#define CPUSTATE_TO_CPUSTATE_E(cs,cse,ecode) \
 ((cse).com = (cs).com, \
  (cse).iret.tail = (cs).iret, \
  (cse).iret.exc_code = (ecode))
#define CPUSTATE_E_TO_CPUSTATE(cse,cs) \
 ((cs).com = (cse).com, \
  (cs).iret.tail = (cse).iret)

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_CPUSTATE_H */
