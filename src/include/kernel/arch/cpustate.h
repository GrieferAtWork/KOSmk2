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
#include <hybrid/host.h>

DECL_BEGIN

#define __COMMON_REG32_2(n) union PACKED { u32 x##n; u32 e##n; u16 n; };
#define __COMMON_REG32_1(n) union PACKED { u32 x##n##x; u32 e##n##x; u16 n##x; struct PACKED { u8 n##l,n##h; }; };
#ifdef __x86_64__
#define __COMMON_REG3(n) union PACKED { u64 x##n; u64 r##n; }
#define __COMMON_REG2(n) union PACKED { u64 x##n; u64 r##n; u32 e##n; u16 n; };
#define __COMMON_REG1(n) union PACKED { u64 x##n##x; u64 r##n##x; u32 e##n##x; u16 n##x; struct PACKED { u8 n##l,n##h; }; };
#else
#define __COMMON_REG2(n) __COMMON_REG32_2(n)
#define __COMMON_REG1(n) __COMMON_REG32_1(n)
#endif


#ifdef __x86_64__
#else
#endif

/* General purpose registers. */
/* HINT: Use `__ASM_PUSH_GPREGS' / `__ASM_POP_GPREGS' to push/pop this structure. */
#ifdef __CC__
struct PACKED gpregs {
#ifdef __x86_64__
 __COMMON_REG3(15)
 __COMMON_REG3(14)
 __COMMON_REG3(13)
 __COMMON_REG3(12)
 __COMMON_REG3(11)
 __COMMON_REG3(10)
 __COMMON_REG3(9)
 __COMMON_REG3(8)
#endif
 __COMMON_REG2(di)
 __COMMON_REG2(si)
 __COMMON_REG2(bp)
#ifndef __x86_64__
 __COMMON_REG2(sp)
#endif
 __COMMON_REG1(b)
 __COMMON_REG1(d)
 __COMMON_REG1(c)
 __COMMON_REG1(a)
};
struct PACKED gpregs32 {
 __COMMON_REG32_2(di)
 __COMMON_REG32_2(si)
 __COMMON_REG32_2(bp)
 __COMMON_REG32_2(sp)
 __COMMON_REG32_1(b)
 __COMMON_REG32_1(d)
 __COMMON_REG32_1(c)
 __COMMON_REG32_1(a)
};
#endif /* __CC__ */
#undef __COMMON_REG2
#undef __COMMON_REG1

#ifdef __x86_64__
#define __ASM_PUSH_GPREGS \
    pushq %rax; pushq %rcx; pushq %rdx; pushq %rbx; \
                pushq %rbp; pushq %rsi; pushq %rdi; \
    pushq %r8;  pushq %r9;  pushq %r10; pushq %r11; \
    pushq %r12; pushq %r13; pushq %r14; pushq %r15;
#define __ASM_POP_GPREGS \
    popq %r15; popq %r14; popq %r13; popq %r12; \
    popq %r11; popq %r10; popq %r9;  popq %r8;  \
    popq %rdi; popq %rsi; popq %rbp;            \
    popq %rbx; popq %rdx; popq %rcx; popq %rax;
#else
#define __ASM_PUSH_GPREGS   pushal;
#define __ASM_POP_GPREGS    popal;
#endif
#define __ASM_PUSH_GPREGS32 pushal;
#define __ASM_POP_GPREGS32  popal;

/* Segment registers */
#ifdef __x86_64__
#ifdef __CC__
struct PACKED sgregs { u64 gs,fs; }; /* Use 64-bit types to preserve 8-byte alignment. (There is no `pushl %fs'. - Else we could use that) */
#endif /* __CC__ */
#define __ASM_PUSH_SREGS pushq %fs; pushq %gs;
#define __ASM_POP_SREGS  popq %gs;  popq %fs;
#else
#ifdef __CC__
struct PACKED sgregs { u16 gs,fs,es,ds; };
#endif /* __CC__ */
#define __ASM_PUSH_SREGS   pushw %ds; pushw %es; pushw %fs; pushw %gs;
#define __ASM_POP_SREGS    popw  %gs; popw  %fs; popw  %es; popw  %ds;
#endif
#ifdef __CC__
struct PACKED sgregs32 { u16 gs,fs,es,ds; };
#endif /* __CC__ */
#define __ASM_PUSH_SREGS32 pushw %ds; pushw %es; pushw %fs; pushw %gs;
#define __ASM_POP_SREGS32  popw  %gs; popw  %fs; popw  %es; popw  %ds;


#ifdef __x86_64__
#define IRET_SEGMENT(x)   union PACKED { struct PACKED { u16 x##16,__##x##16hi; }; struct PACKED { u32 x##32,__##x##32hi; }; u64 x; }
#else
#define IRET_SEGMENT(x)   union PACKED { struct PACKED { u16 x##16,__##x##16hi; }; u32 x##32; u32 x; }
#endif

/* Interrupt return registers (_MUST_ be iret-compatible). */
#define IRREGS_ISUSER(x) (((x).cs&3) == 3) /* When true, must use `struct irregs' */
#ifdef __CC__
struct PACKED irregs_host     { register_t eip; IRET_SEGMENT(cs); register_t eflags; };
struct PACKED irregs          { union PACKED { struct irregs_host host; struct PACKED {
                                register_t eip; IRET_SEGMENT(cs); register_t eflags; };};
                                register_t useresp; IRET_SEGMENT(ss); };
struct PACKED irregs_host_e   { register_t exc_code; union PACKED { struct irregs_host tail; struct PACKED {
                                register_t eip; IRET_SEGMENT(cs); register_t eflags; };};};
struct PACKED irregs_e        { union PACKED { struct irregs_host_e host; struct PACKED {
                                register_t exc_code; union PACKED { struct irregs tail; struct PACKED {
                                register_t eip; IRET_SEGMENT(cs); register_t eflags;
                                register_t useresp; IRET_SEGMENT(ss); };};};};};
#ifdef __x86_64__
#define __ASM_IRET   iretq
#else
#define __ASM_IRET   iret
#endif

/* CPU state for 16-bit realmode interrupts. */
struct PACKED cpustate16 {
 struct gpregs32 gp;
 struct sgregs32 sg;
 u16             ss;
 u32             eflags;
};

/* Common CPU state. */
struct PACKED comregs {
 struct gpregs gp;
 struct sgregs sg;
};
struct PACKED comregs32 {
 struct gpregs32 gp;
 struct sgregs32 sg;
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
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_CPUSTATE_H */
