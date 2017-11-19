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
#ifndef GUARD_ARCH_I386_KOS_INCLUDE_ARCH_TSS_H
#define GUARD_ARCH_I386_KOS_INCLUDE_ARCH_TSS_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>

DECL_BEGIN

#ifdef __x86_64__

#define TSS_OFFSETOF_RSP0  4
#define TSS_OFFSETOF_RSP1  12
#define TSS_OFFSETOF_RSP2  20
#define TSS_OFFSETOF_IST   36
#define TSS_OFFSETOF_IOMAP 102
#define TSS_SIZE           104
#define TSS_OFFSETOF_XSP0  TSS_OFFSETOF_RSP0
#define TSS_OFFSETOF_XSP1  TSS_OFFSETOF_RSP1
#define TSS_OFFSETOF_XSP2  TSS_OFFSETOF_RSP2
#ifdef __CC__
struct PACKED tss {
 u32 __reserved1;
 /* Very important: The stack that is switched to when an
  *                 interrupt switches to the kernel (Ring #0) */
union PACKED {
struct PACKED { u64 xsp0,xsp1,xsp2; };
struct PACKED { u64 rsp0,rsp1,rsp2; }; };
 u64 __reserved2;
 u64 ist[7]; /* Interrupt stack table.
              * Alternative stacks used for handling interrupts when
              * the vector's `ie_ist' member is initialized as non-ZERO.
              * To illustrate, the cpu does the following in 64-bit mode:
              * >> void *target_stack;
              * >> struct idtentry vector;
              * >>
              * >> vector = GET_VECTOR_FOR_INTERRUPT(GET_CURRENT_INTERRUPT());
              * >>
              * >> if (vector.ie_ist != 0) {
              * >>     target_stack = THIS_CPU->c_arch.ac_tss.ist[vector.ie_ist-1];
              * >> } else if (COMES_FROM_USERSPACE()) {
              * >>     // rsp1/rsp2  not used because KOS only uses ring #0 and #3
              * >>     target_stack = THIS_CPU->c_arch.ac_tss.rsp0;
              * >> } else {
              * >>     target_stack = caller->%RSP;
              * >> }
              * >>
              * >> if (vector.ie_ist != 0 || COMES_FROM_USERSPACE()) {
              * >>     // NOTE: `3' is the old privilege level (AGAIN: KOS doesn't use 1, or 2).
              * >>     PUSHQ(target_stack,caller->%SS|3);
              * >>     PUSHQ(target_stack,caller->%RSP);
              * >> }
              * >>
              * >> PUSHQ(target_stack,caller->%RFLAGS);
              * >> PUSHQ(target_stack,caller->%CS);
              * >> PUSHQ(target_stack,caller->%RIP);
              * >>
              * >> if (HAS_EXC_CODE(GET_CURRENT_INTERRUPT()))
              * >>     PUSHQ(target_stack,GET_EXC_CODE(GET_CURRENT_INTERRUPT()));
              * >>
              * >> %RSP = target_stack;
              * >> goto GET_VECTOR_TARGET(vector);
              * >>
              */
 u64 __reserved3;
 u16 __reserved4;
 u16 iomap_base;
};
#endif /* __CC__ */
#else
#define TSS_OFFSETOF_LINK    0
#define TSS_OFFSETOF_ESP0    4
#define TSS_OFFSETOF_SS0     8
#define TSS_OFFSETOF_ESP1    12
#define TSS_OFFSETOF_SS1     16
#define TSS_OFFSETOF_ESP2    20
#define TSS_OFFSETOF_SS2     24
#define TSS_OFFSETOF_CR3     28
#define TSS_OFFSETOF_EIP     32
#define TSS_OFFSETOF_EFLAGS  36
#define TSS_OFFSETOF_EAX     40
#define TSS_OFFSETOF_ECX     44
#define TSS_OFFSETOF_EDX     48
#define TSS_OFFSETOF_EBX     52
#define TSS_OFFSETOF_ESP     56
#define TSS_OFFSETOF_EBP     60
#define TSS_OFFSETOF_ESI     64
#define TSS_OFFSETOF_EDI     68
#define TSS_OFFSETOF_ES      72
#define TSS_OFFSETOF_CS      76
#define TSS_OFFSETOF_SS      80
#define TSS_OFFSETOF_DS      84
#define TSS_OFFSETOF_FS      88
#define TSS_OFFSETOF_GS      92
#define TSS_OFFSETOF_LDTR    96
#define TSS_OFFSETOF_IOMAP   102
#define TSS_SIZE             104
#define TSS_OFFSETOF_XSP0    TSS_OFFSETOF_ESP0
#define TSS_OFFSETOF_XSP1    TSS_OFFSETOF_ESP1
#define TSS_OFFSETOF_XSP2    TSS_OFFSETOF_ESP2

#ifdef __CC__
struct PACKED tss {
 u16 link,__reserved1;
 /* Very important: The stack that is switched to when an interrupt switches to the kernel (Ring #0) */
 union PACKED { u32 xsp0; u32 esp0; }; u16 ss0,__reserved2;
 union PACKED { u32 xsp1; u32 esp1; }; u16 ss1,__reserved3;
 union PACKED { u32 xsp2; u32 esp2; }; u16 ss2,__reserved4;
 /* NOTE: As you can see, there is no `esp3' (ring #3 is userspace). So with that in mind,
  *       the existing stack is re-used when an interrupt happens while inside the kernel,
  *       meaning that kernel-space IRQ recursion is implicitly possible, yet one has to
  *       keep in mind that the stack must always be of sufficient size! */
 u32 cr3,eip,eflags;
 u32 eax,ecx,edx,ebx;
 u32 esp,ebp,esi,edi;
 u16 es,__reserved5;
 u16 cs,__reserved6;
 u16 ss,__reserved7;
 u16 ds,__reserved8;
 u16 fs,__reserved9;
 u16 gs,__reservedA;
 u16 ldtr,__reservedB;
 u16 __reservedC,iomap_base;
};
#endif /* __CC__ */
#endif

DECL_END

#endif /* !GUARD_ARCH_I386_KOS_INCLUDE_ARCH_TSS_H */
