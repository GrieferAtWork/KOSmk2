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
#ifndef _I386_KOS_SYS_UCONTEXT_H
#define _I386_KOS_SYS_UCONTEXT_H 1
#define _SYS_UCONTEXT_H 1

#include <__stdinc.h>
#include <features.h>
/*#include <signal.h>*/
#include <bits/sigstack.h>
#include <asm/sigcontext.h>

/* Copyright (C) 1998-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* System V/ARM ABI compliant context switching support.  */

#define __SIZEOF_GREG_T__  __SIZEOF_REGISTER__
#define NGREG    18 /* Number of general registers.  */

#ifdef __CC__
#ifndef __greg_t_defined
#define __greg_t_defined 1
typedef __SREGISTER_TYPE__ greg_t;
#endif /* !__greg_t_defined */
typedef greg_t gregset_t[NGREG]; /* Container for all general registers. */
#endif /* __CC__ */


/* Number of each register is the `gregset_t' array. */
#ifdef __CC__
enum {
    REG_R0  = 0,
    REG_R1  = 1,
    REG_R2  = 2,
    REG_R3  = 3,
    REG_R4  = 4,
    REG_R5  = 5,
    REG_R6  = 6,
    REG_R7  = 7,
    REG_R8  = 8,
    REG_R9  = 9,
    REG_R10 = 10,
    REG_R11 = 11,
    REG_R12 = 12,
    REG_R13 = 13,
    REG_R14 = 14,
    REG_R15 = 15
};
#endif /* __CC__ */
#define REG_R0  0
#define REG_R1  1
#define REG_R2  2
#define REG_R3  3
#define REG_R4  4
#define REG_R5  5
#define REG_R6  6
#define REG_R7  7
#define REG_R8  8
#define REG_R9  9
#define REG_R10 10
#define REG_R11 11
#define REG_R12 12
#define REG_R13 13
#define REG_R14 14
#define REG_R15 15


#define __LIBC_FPSTATE_OFFSETOF_FPREGS    0
#define __LIBC_FPSTATE_OFFSETOF_FPSR      96 /* 3*4*8 */
#define __LIBC_FPSTATE_OFFSETOF_FPCR      100
#define __LIBC_FPSTATE_OFFSETOF_FTYPE     104
#define __LIBC_FPSTATE_OFFSETOF_INIT_FLAG 112
#define __LIBC_FPSTATE_SIZE               116
#ifdef __CC__
struct _libc_fpstate {
  struct {
    unsigned int sign1:1;
    unsigned int unused:15;
    unsigned int sign2:1;
    unsigned int exponent:14;
    unsigned int j:1;
    unsigned int mantissa1:31;
    unsigned int mantissa0:32;
  } fpregs[8];
  unsigned int    fpsr:32;
  unsigned int    fpcr:32;
  __UINT8_TYPE__  ftype[8];
  __UINT32_TYPE__ init_flag;
};

/* Structure to describe FPU registers. */
typedef struct _libc_fpstate fpregset_t;

/* Context to describe whole processor state.  This only describes
 * the core registers; coprocessor registers get saved elsewhere
 * (e.g. in uc_regspace, or somewhere unspecified on the stack
 * during non-RT signal handlers). */
typedef struct sigcontext mcontext_t;
#endif /* __CC__ */

#define __UCONTEXT_OFFSETOF_FLAGS       0
#define __UCONTEXT_OFFSETOF_LINK        __SIZEOF_POINTER__
#define __UCONTEXT_OFFSETOF_STACK    (2*__SIZEOF_POINTER__)
#define __UCONTEXT_OFFSETOF_MCONTEXT (2*__SIZEOF_POINTER__+__STACK_SIZE)
#define __UCONTEXT_OFFSETOF_SIGMASK  (2*__SIZEOF_POINTER__+__STACK_SIZE+__SIGCONTEXT_SIZE)
#define __PRIV_UCONTEXT_OFFSETOF_REGSPACE (2*__SIZEOF_POINTER__+__STACK_SIZE+__SIGCONTEXT_SIZE+__SIZEOF_SIGSET_T__)
#define __UCONTEXT_OFFSETOF_REGSPACE ((__PRIV_UCONTEXT_OFFSETOF_REGSPACE+7)&~7)
#define __UCONTEXT_ALIGN              8
#define __UCONTEXT_SIZE              (__UCONTEXT_OFFSETOF_REGSPACE+128*__SIZEOF_POINTER__)
#ifdef __CC__
/* Userlevel context. */
typedef __ATTR_ALIGNED(__UCONTEXT_ALIGN) struct ucontext {
  __ULONGPTR_TYPE__                   uc_flags;
  struct ucontext                    *uc_link;
  stack_t                             uc_stack;
  mcontext_t                          uc_mcontext;
  __sigset_t                          uc_sigmask;
#if __PRIV_UCONTEXT_OFFSETOF_REGSPACE != __UCONTEXT_OFFSETOF_REGSPACE
  __BYTE_TYPE__                     __uc_padding[__UCONTEXT_OFFSETOF_REGSPACE-__PRIV_UCONTEXT_OFFSETOF_REGSPACE];
#endif
  __ULONGPTR_TYPE__                   uc_regspace[128];
} ucontext_t;
#endif /* __CC__ */


#endif /* sys/ucontext.h */
