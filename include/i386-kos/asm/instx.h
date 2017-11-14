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
#ifndef _X86_KOS_ASM_INSTX_H
#define _X86_KOS_ASM_INSTX_H 1

#include <__stdinc.h>
#include <hybrid/host.h>

#ifdef __x86_64__
#define __INSTX(x) x##q
#define XSZ        8
#else
#define __INSTX(x) x##l
#define XSZ        4
#endif

#define adcx     __INSTX(adc)
#define addx     __INSTX(add)
#define andx     __INSTX(and)
#define callx    __INSTX(call)
#define cmpsx    __INSTX(cmps)
#define cmpx     __INSTX(cmp)
#define cmpxchgx __INSTX(cmpxchg)
#define decx     __INSTX(dec)
#define incx     __INSTX(inc)
#define jmpx     __INSTX(jmp)
#define leax     __INSTX(lea)
#define lodsx    __INSTX(lods)
#define movsbx   __INSTX(movsb)
#define movswx   __INSTX(movsw)
#define movsx    __INSTX(movs)
#define movx     __INSTX(mov)
#define movzbx   __INSTX(movzb)
#define movzwx   __INSTX(movzw)
#define negx     __INSTX(neg)
#define notx     __INSTX(not)
#define orx      __INSTX(or)
#define popfx    __INSTX(popf)
#define popx     __INSTX(pop)
#define pushfx   __INSTX(pushf)
#define pushx    __INSTX(push)
#define salx     __INSTX(sal)
#define sarx     __INSTX(sar)
#define sbbx     __INSTX(sbb)
#define shlx     __INSTX(shl)
#define shrx     __INSTX(shr)
#define stosx    __INSTX(stos)
#define subx     __INSTX(sub)
#define testx    __INSTX(test)
#define xchgx    __INSTX(xchg)
#define xorx     __INSTX(xor)

#ifdef __x86_64__
#define movslx   movslq
#define movzlx   movzlq
#else
#define movslx   movl
#define movzlx   movl
#endif

#ifdef __x86_64__
/* x86_64 doesn't have FASTCALL, so we alias
 * SYSV_ABI registers for arguments #1, #2. */
#define FASTCALL_REG1  rdi
#define FASTCALL_REG2  rsi
#else
#define FASTCALL_REG1  ecx
#define FASTCALL_REG2  edx
#endif

/* Common register names. */
#ifndef __INTELLISENSE__
#ifdef __x86_64__
#   define xax    rax
#   define xcx    rcx
#   define xdx    rdx
#   define xbx    rbx
#   define xsp    rsp
#   define xbp    rbp
#   define xsi    rsi
#   define xdi    rdi
#   define xip    rip
#   define xflags rflags
#else
#   define xax    eax
#   define xcx    ecx
#   define xdx    edx
#   define xbx    ebx
#   define xsp    esp
#   define xbp    ebp
#   define xsi    esi
#   define xdi    edi
#   define xip    eip
#   define xflags eflags
#endif
#else /* __INTELLISENSE__ */
#   define xax    xax
#   define xcx    xcx
#   define xdx    xdx
#   define xbx    xbx
#   define xsp    xsp
#   define xbp    xbp
#   define xsi    xsi
#   define xdi    xdi
#   define xip    xip
#   define xflags xflags
#endif /* !__INTELLISENSE__ */

#ifdef __x86_64__
#   define leax_rel(sym,reg)  leaq sym(%rip), reg
#   define ileax_rel(sym,reg) leaq sym(%%rip), reg
#   define movx_rel(sym,reg)  movq sym(%rip), reg
#   define imovx_rel(sym,reg) movq sym(%%rip), reg
#else
#   define leax_rel(sym,reg)  leal sym, reg
#   define ileax_rel(sym,reg) leal sym, reg
#   define movx_rel(sym,reg)  movl sym, reg
#   define imovx_rel(sym,reg) movl sym, reg
#endif

/* TODO: Get rid of all of this stuff and everything that is below.
 *       The kernel is stay at -2Gb which always allows for the best
 *       code to be generated that doesn't need any of these work-arounds. */
#if defined(__KERNEL__) && defined(__x86_64__) && 0
#if 0
#   define ASM_USE_MOVABS 1
#   define pushx_sym(clobber,sym)          movabs $(sym), clobber; pushq clobber
#   define ipushx_sym(clobber,sym)         movabs $(sym), clobber; pushq clobber
#   define asmx_sym(clobber,inst,sym,reg)  movabs $(sym), clobber; inst clobber, reg
#   define iasmx_sym(clobber,inst,sym,reg) movabs $(sym), clobber; inst clobber, reg
#else
#   define ASM_USE_LEAIP 1
#   define pushx_sym(clobber,sym)          leaq sym(%rip),  clobber; clobber
#   define ipushx_sym(clobber,sym)         leaq sym(%%rip), clobber; clobber
#   define asmx_sym(clobber,inst,sym,reg)  leaq sym(%rip),  clobber; inst clobber, reg
#   define iasmx_sym(clobber,inst,sym,reg) leaq sym(%%rip), clobber; inst clobber, reg
#endif
#else
#   define pushx_sym(clobber,sym)          pushx $(sym)
#   define ipushx_sym(clobber,sym)         pushx $(sym)
#   define asmx_sym(clobber,inst,sym,reg)  inst $(sym), reg
#   define iasmx_sym(clobber,inst,sym,reg) inst $(sym), reg
#endif

#define cmpx_sym(clobber,sym,reg)   asmx_sym(clobber,cmpx,sym,reg)
#define icmpx_sym(clobber,sym,reg)  asmx_sym(clobber,cmpx,sym,reg)
#define addx_sym(clobber,sym,reg)   asmx_sym(clobber,addx,sym,reg)
#define iaddx_sym(clobber,sym,reg)  asmx_sym(clobber,addx,sym,reg)
#define subx_sym(clobber,sym,reg)   asmx_sym(clobber,subx,sym,reg)
#define isubx_sym(clobber,sym,reg)  asmx_sym(clobber,subx,sym,reg)


#endif /* !_X86_KOS_ASM_INSTX_H */
