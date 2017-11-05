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
#ifndef _LINUX_UNISTD_H
#define _LINUX_UNISTD_H 1

#include <__stdinc.h>
#include <features.h>
#include <asm/unistd.h>
#include <hybrid/host.h>
#ifdef __x86_64__
#include <hybrid/typecore.h>
#endif

__SYSDECL_BEGIN

#define __PRIVATE_SYSCALL_ATTR6_     ,
#define __PRIVATE_SYSCALL_ATTR6_1    ,
#define __PRIVATE_SYSCALL_ATTR5(x,val,...) val
#define __PRIVATE_SYSCALL_ATTR4(x)    __PRIVATE_SYSCALL_ATTR5 x
#define __PRIVATE_SYSCALL_ATTR3(x)    __PRIVATE_SYSCALL_ATTR4((__PRIVATE_SYSCALL_ATTR6_##x 1,0))
#define __PRIVATE_SYSCALL_ATTR2(x)    __PRIVATE_SYSCALL_ATTR3(x)
#ifdef __x86_64__
#define __PRIVATE_IMPL__CLOB_0
#define __PRIVATE_IMPL__CLOB_1(x)    ,x
#define __PRIVATE_IMPL_CLOB2(x) __PRIVATE_IMPL##x
#define __PRIVATE_IMPL_CLOB(x) __PRIVATE_IMPL_CLOB2(x)
#define __PRIVATE_SYSCALL_CLOB6_C(x) ,__CLOB_1(x)
#define __PRIVATE_SYSCALL_CLOB3(x)    __PRIVATE_IMPL_CLOB(__PRIVATE_SYSCALL_ATTR4((__PRIVATE_SYSCALL_CLOB6_##x,__PRIVATE_SYSCALL_CLOB0)))
#define __PRIVATE_SYSCALL_CLOB2(x)    __PRIVATE_SYSCALL_CLOB3(x)
#define __PRIVATE_SYSCALL_CLOB0       __CLOB_0
#else
#define __PRIVATE_SYSCALL_CLOB6_C(x) ,x
#define __PRIVATE_SYSCALL_CLOB3(x)    __PRIVATE_SYSCALL_ATTR4((__PRIVATE_SYSCALL_CLOB6_##x,__PRIVATE_SYSCALL_CLOB0))
#define __PRIVATE_SYSCALL_CLOB2(x)    __PRIVATE_SYSCALL_CLOB3(x)
#define __PRIVATE_SYSCALL_CLOB0      /* Default clobber list. */
#endif

#define __PRIVATE_SYSCALL_ISASM(nr) __PRIVATE_SYSCALL_ATTR2(__SC_ATTRIB_ISASM_##nr)
#define __PRIVATE_SYSCALL_ISNRT(nr) __PRIVATE_SYSCALL_ATTR2(__SC_ATTRIB_ISNRT_##nr)
#define __PRIVATE_SYSCALL_ISLNG(nr) __PRIVATE_SYSCALL_ATTR2(__SC_ATTRIB_ISLNG_##nr)
#define __PRIVATE_SYSCALL_CLOBB(nr) __PRIVATE_SYSCALL_CLOB2(__SC_ATTRIB_CLOBB_##nr)

/* Query system call attributes */
#define __SYSCALL_ISASM(nr) __PRIVATE_SYSCALL_ISASM(nr)
#define __SYSCALL_ISNRT(nr) __PRIVATE_SYSCALL_ISNRT(nr)
#define __SYSCALL_ISLNG(nr) __PRIVATE_SYSCALL_ISLNG(nr)

/* Query system call additional clobber data. */
#define __SYSCALL_CLOBB(nr) __PRIVATE_SYSCALL_CLOBB(nr)

#define __SYSCALL_ENUM0(v)                                   /* Nothing */
#define __SYSCALL_ENUM1(t1,a1)                               a1
#define __SYSCALL_ENUM2(t2,a2,t1,a1)                         a2,a1
#define __SYSCALL_ENUM3(t3,a3,t2,a2,t1,a1)                   a3,a2,a1
#define __SYSCALL_ENUM4(t4,a4,t3,a3,t2,a2,t1,a1)             a4,a3,a2,a1
#define __SYSCALL_ENUM5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)       a5,a4,a3,a2,a1
#define __SYSCALL_ENUM6(t6,a6,t5,a5,t4,a4,t3,a3,t2,a2,t1,a1) a6,a5,a4,a3,a2,a1

#define __SYSCALL_LIST0(v)                 /* Nothing */
#define __SYSCALL_LIST1(a1)                a1
#define __SYSCALL_LIST2(a2,a1)             a2,a1
#define __SYSCALL_LIST3(a3,a2,a1)          a3,a2,a1
#define __SYSCALL_LIST4(a4,a3,a2,a1)       a4,a3,a2,a1
#define __SYSCALL_LIST5(a5,a4,a3,a2,a1)    a5,a4,a3,a2,a1
#define __SYSCALL_LIST6(a6,a5,a4,a3,a2,a1) a6,a5,a4,a3,a2,a1

#define __SYSCALL_DECL0(v)                                   v
#define __SYSCALL_DECL1(t1,a1)                               t1 a1
#define __SYSCALL_DECL2(t2,a2,t1,a1)                         t2 a2, __SYSCALL_DECL1(t1,a1)
#define __SYSCALL_DECL3(t3,a3,t2,a2,t1,a1)                   t3 a3, __SYSCALL_DECL2(t2,a2,t1,a1)
#define __SYSCALL_DECL4(t4,a4,t3,a3,t2,a2,t1,a1)             t4 a4, __SYSCALL_DECL3(t3,a3,t2,a2,t1,a1)
#define __SYSCALL_DECL5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)       t5 a5, __SYSCALL_DECL4(t4,a4,t3,a3,t2,a2,t1,a1)
#define __SYSCALL_DECL6(t6,a6,t5,a5,t4,a4,t3,a3,t2,a2,t1,a1) t6 a6, __SYSCALL_DECL5(t5,a5,t4,a4,t3,a3,t2,a2,t1,a1)

/* i386:   IN(ebx, ecx, edx, esi, edi, ebp) OUT(eax[,edx]) */
/* x86_64: IN(rdi, rsi, rdx, r10, r8,  r9)  OUT(rax[,rdx]) */

/* x86_64 (CDECL): rdi, rsi, rdx, rcx, r8, r9 */

#ifdef __x86_64__
#define __SYSCALL_ASMREG0(...)                                                 /* Nothing */
#define __SYSCALL_ASMREG1(a1)                                                  , "D" (a1)
#define __SYSCALL_ASMREG2(a1,a2)              __SYSCALL_ASMREG1(a1)            , "S" (a2)
#define __SYSCALL_ASMREG3(a1,a2,a3)           __SYSCALL_ASMREG2(a1,a2)         , "d" (a3)
#define __SYSCALL_ASMREG4(a1,a2,a3,a4)        __SYSCALL_ASMREG3(a1,a2,a3)      , "r" (__sc_r10)
#define __SYSCALL_ASMREG5(a1,a2,a3,a4,a5)     __SYSCALL_ASMREG4(a1,a2,a3,a4)   , "r" (__sc_r8)
#define __SYSCALL_ASMREG6(a1,a2,a3,a4,a5,a6)  __SYSCALL_ASMREG5(a1,a2,a3,a4,a5), "r" (__sc_r9)
#ifdef __INTELLISENSE__
#define __PRIVATE_SYSCALL_ASM_0(n,res,id,args) { res,id; __SYSCALL_LIST##n args; }
#define __PRIVATE_SYSCALL_ASM_1(n,id,args)     { id; __SYSCALL_LIST##n args; }
#else
#define __SYSCALL_ASMLOC0(...)
#define __SYSCALL_ASMLOC1(a1)
#define __SYSCALL_ASMLOC2(a1,a2)
#define __SYSCALL_ASMLOC3(a1,a2,a3)
#define __SYSCALL_ASMLOC4(a1,a2,a3,a4)                                          register __UINTPTR_TYPE__ __sc_r10 asm("r10") = (__UINTPTR_TYPE__)(a4);
#define __SYSCALL_ASMLOC5(a1,a2,a3,a4,a5)     __SYSCALL_ASMLOC4(a1,a2,a3,a4)    register __UINTPTR_TYPE__ __sc_r8 asm("r8") = (__UINTPTR_TYPE__)(a5);
#define __SYSCALL_ASMLOC6(a1,a2,a3,a4,a5,a6)  __SYSCALL_ASMLOC5(a1,a2,a3,a4,a5) register __UINTPTR_TYPE__ __sc_r9 asm("r9") = (__UINTPTR_TYPE__)(a6);
#define __PRIVATE_SYSCALL_ASM_0(n,res,id,args) \
  { __SYSCALL_ASMLOC##n args \
    __asm__ __volatile__("int {$}0x80\n" \
                         : "=a" (res) \
                         : "a" (id) __SYSCALL_ASMREG##n args \
                         : "r11" __SYSCALL_CLOBB(id)); \
  }
#define __PRIVATE_SYSCALL_ASM_1(n,id,args) \
  { __SYSCALL_ASMLOC##n args \
    __asm__ __volatile__("int {$}0x80\n" \
                         : : "a" (id) __SYSCALL_ASMREG##n args \
                         : "r11" __SYSCALL_CLOBB(id)); \
  }
#endif
#elif defined(__i386__)
#define __SYSCALL_ASMREG0(...)                                                 /* Nothing */
#define __SYSCALL_ASMREG1(a1)                                                  , "b" (a1)
#define __SYSCALL_ASMREG2(a1,a2)              __SYSCALL_ASMREG1(a1)            , "c" (a2)
#define __SYSCALL_ASMREG3(a1,a2,a3)           __SYSCALL_ASMREG2(a1,a2)         , "d" (a3)
#define __SYSCALL_ASMREG4(a1,a2,a3,a4)        __SYSCALL_ASMREG3(a1,a2,a3)      , "S" (a4)
#define __SYSCALL_ASMREG5(a1,a2,a3,a4,a5)     __SYSCALL_ASMREG4(a1,a2,a3,a4)   , "D" (a5)
#define __SYSCALL_ASMREG6(a1,a2,a3,a4,a5,a6)  __SYSCALL_ASMREG5(a1,a2,a3,a4,a5), "m" (a6)

#define __SYSCALL_ASMTXT00 "int {$}0x80\n"
#define __SYSCALL_ASMTXT01 __SYSCALL_ASMTXT00
#define __SYSCALL_ASMTXT02 __SYSCALL_ASMTXT00
#define __SYSCALL_ASMTXT03 __SYSCALL_ASMTXT00
#define __SYSCALL_ASMTXT04 __SYSCALL_ASMTXT00
#define __SYSCALL_ASMTXT05 __SYSCALL_ASMTXT00
#define __SYSCALL_ASMTXT06 "pushl %%ebp\n" \
                           "movl %7, %%ebp\n" \
                           "int $0x80\n" \
                           "popl %%ebp\n"
#define __SYSCALL_ASMTXT10 __SYSCALL_ASMTXT00
#define __SYSCALL_ASMTXT11 __SYSCALL_ASMTXT01
#define __SYSCALL_ASMTXT12 __SYSCALL_ASMTXT02
#define __SYSCALL_ASMTXT13 __SYSCALL_ASMTXT03
#define __SYSCALL_ASMTXT14 __SYSCALL_ASMTXT04
#define __SYSCALL_ASMTXT15 __SYSCALL_ASMTXT05
#define __SYSCALL_ASMTXT16 "pushl %%ebp\n" \
                           "movl %6, %%ebp\n" \
                           "int $0x80\n"
#define __SYSCALL_ASMTXT_2(rt,n) __SYSCALL_ASMTXT##rt##n
#define __SYSCALL_ASMTXT(rt,n) __SYSCALL_ASMTXT_2(rt,n)

#define __SYSCALL_ASMRET0     "=a"
#define __SYSCALL_ASMRET1     "=A"
#define __SYSCALL_ASMRET2(lg) __SYSCALL_ASMRET##lg
#define __SYSCALL_ASMRET(lg)  __SYSCALL_ASMRET2(lg)

#ifdef __INTELLISENSE__
#define __PRIVATE_SYSCALL_ASM_0(n,res,id,args) { res,id; __SYSCALL_LIST##n args; }
#define __PRIVATE_SYSCALL_ASM_1(n,id,args)     { id; __SYSCALL_LIST##n args; }
#else
#define __PRIVATE_SYSCALL_ASM_0(n,res,id,args) \
        __asm__ __volatile__(__SYSCALL_ASMTXT(__SYSCALL_ISNRT(id),n) \
                             : __SYSCALL_ASMRET(__SYSCALL_ISLNG(id)) (res) \
                             : "a" (id) __SYSCALL_ASMREG##n args \
                             : __SYSCALL_CLOBB(id))
#define __PRIVATE_SYSCALL_ASM_1(n,id,args) \
        __asm__ __volatile__(__SYSCALL_ASMTXT(__SYSCALL_ISNRT(id),n) \
                             : : "a" (id) __SYSCALL_ASMREG##n args \
                             : __SYSCALL_CLOBB(id))
#endif
#else
#error "Unsupported arch"
#endif

#define __SYSCALL_INL(n,type,id,args)                   __PRIVATE_SYSCALL_INL2(__SYSCALL_ISNRT(id),n,type,id,args)
#define __SYSCALL_FUN(n,cc,post_attr,type,id,name,decl) __PRIVATE_SYSCALL_FUN2(__SYSCALL_ISNRT(id),n,cc,post_attr,type,id,name,decl)

#define __PRIVATE_SYSCALL_INL2(nr,n,type,id,args)                   __PRIVATE_SYSCALL_INL3(nr,n,type,id,args)
#define __PRIVATE_SYSCALL_FUN2(nr,n,cc,post_attr,type,id,name,decl) __PRIVATE_SYSCALL_FUN3(nr,n,cc,post_attr,type,id,name,decl)
#define __PRIVATE_SYSCALL_INL3(nr,n,type,id,args)                   __PRIVATE_SYSCALL_INL4_##nr(n,type,id,args)
#define __PRIVATE_SYSCALL_FUN3(nr,n,cc,post_attr,type,id,name,decl) __PRIVATE_SYSCALL_FUN4_##nr(n,cc,post_attr,type,id,name,decl)

#define __SYSCALL_TRACE(id) /* Nothing */

#define __PRIVATE_SYSCALL_INL4_1(n,type,id,args) \
        __XBLOCK({ __SYSCALL_TRACE(id) \
                   __PRIVATE_SYSCALL_ASM_1(n,id,args); \
                   __builtin_unreachable(); (void)0; })
#define __PRIVATE_SYSCALL_FUN4_1(n,cc,post_attr,type,id,name,decl) \
       __ATTR_NORETURN void (cc name)(__SYSCALL_DECL##n decl) post_attr { \
           __SYSCALL_TRACE(id) \
           __PRIVATE_SYSCALL_ASM_1(n,id,(__SYSCALL_ENUM##n decl)); \
           __builtin_unreachable(); \
       }
#define __PRIVATE_SYSCALL_INL4_0(n,type,id,args) \
        __XBLOCK({ register type __res; \
                   __SYSCALL_TRACE(id) \
                   __PRIVATE_SYSCALL_ASM_0(n,__res,id,args); \
                   XRETURN __res; })
#define __PRIVATE_SYSCALL_FUN4_0(n,cc,post_attr,type,id,name,decl) \
       type (cc name)(__SYSCALL_DECL##n decl) post_attr { \
           register type __res; \
           __SYSCALL_TRACE(id) \
           __PRIVATE_SYSCALL_ASM_0(n,__res,id,(__SYSCALL_ENUM##n decl)); \
           return __res; \
       }

/* Call a system-call in-line, or define a wrapper function. */
#define __SYSCALL_INL0(type,id,...) __SYSCALL_INL(0,type,id,(__VA_ARGS__))
#define __SYSCALL_INL1(type,id,...) __SYSCALL_INL(1,type,id,(__VA_ARGS__))
#define __SYSCALL_INL2(type,id,...) __SYSCALL_INL(2,type,id,(__VA_ARGS__))
#define __SYSCALL_INL3(type,id,...) __SYSCALL_INL(3,type,id,(__VA_ARGS__))
#define __SYSCALL_INL4(type,id,...) __SYSCALL_INL(4,type,id,(__VA_ARGS__))
#define __SYSCALL_INL5(type,id,...) __SYSCALL_INL(5,type,id,(__VA_ARGS__))
#define __SYSCALL_INL6(type,id,...) __SYSCALL_INL(6,type,id,(__VA_ARGS__))
#define __SYSCALL_FUN0(cc,post_attr,type,id,name,...) __SYSCALL_FUN(0,cc,post_attr,type,id,name,(__VA_ARGS__))
#define __SYSCALL_FUN1(cc,post_attr,type,id,name,...) __SYSCALL_FUN(1,cc,post_attr,type,id,name,(__VA_ARGS__))
#define __SYSCALL_FUN2(cc,post_attr,type,id,name,...) __SYSCALL_FUN(2,cc,post_attr,type,id,name,(__VA_ARGS__))
#define __SYSCALL_FUN3(cc,post_attr,type,id,name,...) __SYSCALL_FUN(3,cc,post_attr,type,id,name,(__VA_ARGS__))
#define __SYSCALL_FUN4(cc,post_attr,type,id,name,...) __SYSCALL_FUN(4,cc,post_attr,type,id,name,(__VA_ARGS__))
#define __SYSCALL_FUN5(cc,post_attr,type,id,name,...) __SYSCALL_FUN(5,cc,post_attr,type,id,name,(__VA_ARGS__))
#define __SYSCALL_FUN6(cc,post_attr,type,id,name,...) __SYSCALL_FUN(6,cc,post_attr,type,id,name,(__VA_ARGS__))



#define _syscall0(type,name,...) __SYSCALL_FUN0(,,type,__NR_##name,name,__VA_ARGS__)
#define _syscall1(type,name,...) __SYSCALL_FUN1(,,type,__NR_##name,name,__VA_ARGS__)
#define _syscall2(type,name,...) __SYSCALL_FUN2(,,type,__NR_##name,name,__VA_ARGS__)
#define _syscall3(type,name,...) __SYSCALL_FUN3(,,type,__NR_##name,name,__VA_ARGS__)
#define _syscall4(type,name,...) __SYSCALL_FUN4(,,type,__NR_##name,name,__VA_ARGS__)
#define _syscall5(type,name,...) __SYSCALL_FUN5(,,type,__NR_##name,name,__VA_ARGS__)
#define _syscall6(type,name,...) __SYSCALL_FUN6(,,type,__NR_##name,name,__VA_ARGS__)

__SYSDECL_END

#endif /* !_LINUX_UNISTD_H */
