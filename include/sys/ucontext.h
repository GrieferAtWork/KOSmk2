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
#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/sigset.h>
#include <bits/sigcontext.h>

/* Copyright (C) 2001-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

__DECL_BEGIN

#ifdef __x86_64__
#define __SIZEOF_GREG_T__  8
typedef __LONGLONG greg_t;
#define NGREG  23 /*< Number of general registers. */
typedef greg_t gregset_t[NGREG]; /*< Container for all general registers. */

#ifdef __USE_GNU
/* Number of each register in the `gregset_t' array. */
enum {
  REG_R8 = 0,
  REG_R9,
  REG_R10,
  REG_R11,
  REG_R12,
  REG_R13,
  REG_R14,
  REG_R15,
  REG_RDI,
  REG_RSI,
  REG_RBP,
  REG_RBX,
  REG_RDX,
  REG_RAX,
  REG_RCX,
  REG_RSP,
  REG_RIP,
  REG_EFL,
  REG_CSGSFS, /* Actually short cs, gs, fs, __pad0. */
  REG_ERR,
  REG_TRAPNO,
  REG_OLDMASK,
  REG_CR2
};
#define REG_R8      REG_R8
#define REG_R9      REG_R9
#define REG_R10     REG_R10
#define REG_R11     REG_R11
#define REG_R12     REG_R12
#define REG_R13     REG_R13
#define REG_R14     REG_R14
#define REG_R15     REG_R15
#define REG_RDI     REG_RDI
#define REG_RSI     REG_RSI
#define REG_RBP     REG_RBP
#define REG_RBX     REG_RBX
#define REG_RDX     REG_RDX
#define REG_RAX     REG_RAX
#define REG_RCX     REG_RCX
#define REG_RSP     REG_RSP
#define REG_RIP     REG_RIP
#define REG_EFL     REG_EFL
#define REG_CSGSFS  REG_CSGSFS
#define REG_ERR     REG_ERR
#define REG_TRAPNO  REG_TRAPNO
#define REG_OLDMASK REG_OLDMASK
#define REG_CR2     REG_CR2
#endif /* __USE_GNU */

struct _libc_fpxreg {
 unsigned short int significand[4];
 unsigned short int exponent;
 unsigned short int padding[3];
};
struct _libc_xmmreg {
 __uint32_t element[4];
};
struct _libc_fpstate {
 /* 64-bit FXSAVE format. */
 __uint16_t          cwd;
 __uint16_t          swd;
 __uint16_t          ftw;
 __uint16_t          fop;
 __uint64_t          rip;
 __uint64_t          rdp;
 __uint32_t          mxcsr;
 __uint32_t          mxcr_mask;
 struct _libc_fpxreg _st[8];
 struct _libc_xmmreg _xmm[16];
 __uint32_t          padding[24];
};
/* Structure to describe FPU registers. */
typedef struct _libc_fpstate *fpregset_t;

/* Context to describe whole processor state. */
typedef struct {
 gregset_t   gregs;
 fpregset_t  fpregs; /* Note that fpregs is a pointer. */
 __ULONGLONG __reserved1[8];
} mcontext_t;

/* Userlevel context. */
typedef struct ucontext {
 unsigned long int    uc_flags;
 struct ucontext     *uc_link;
 stack_t              uc_stack;
 mcontext_t           uc_mcontext;
 __sigset_t           uc_sigmask;
 struct _libc_fpstate __fpregs_mem;
} ucontext_t;
#else /* __x86_64__ */

#define __SIZEOF_GREG_T__  4
typedef int greg_t; /*< Type for general register. */
#define NGREG    19 /*< Number of general registers. */
typedef greg_t gregset_t[NGREG]; /* Container for all general registers. */

#ifdef __USE_GNU
enum { /* Number of each register is the `gregset_t' array. */
  REG_GS = 0,
  REG_FS,
  REG_ES,
  REG_DS,
  REG_EDI,
  REG_ESI,
  REG_EBP,
  REG_ESP,
  REG_EBX,
  REG_EDX,
  REG_ECX,
  REG_EAX,
  REG_TRAPNO,
  REG_ERR,
  REG_EIP,
  REG_CS,
  REG_EFL,
  REG_UESP,
  REG_SS
};

/* WARNING: Changes to this order of registers must be mirrored by
 *          'signal_return' in "/src/kernel/sched/signal.c.inl" */
#define REG_GS     0
#define REG_FS     1
#define REG_ES     2
#define REG_DS     3
#define REG_EDI    4
#define REG_ESI    5
#define REG_EBP    6
#define REG_ESP    7
#define REG_EBX    8
#define REG_EDX    9
#define REG_ECX    10
#define REG_EAX    11
#define REG_TRAPNO 12
#define REG_ERR    13
#define REG_EIP    14
#define REG_CS     15
#define REG_EFL    16
#define REG_UESP   17
#define REG_SS     18
#endif

#define __LIBC_FPREG_OFFSETOF_SIGNIFICAND 0
#define __LIBC_FPREG_OFFSETOF_EXPONENT    8
#define __LIBC_FPREG_SIZE                 10
struct _libc_fpreg {
 /* Definitions taken from the kernel headers. */
 unsigned short int significand[4];
 unsigned short int exponent;
};

#define __LIBC_FPSTATE_OFFSETOF_CW       0
#define __LIBC_FPSTATE_OFFSETOF_SW       4
#define __LIBC_FPSTATE_OFFSETOF_TAG      8
#define __LIBC_FPSTATE_OFFSETOF_IPOFF    12
#define __LIBC_FPSTATE_OFFSETOF_CSSEL    16
#define __LIBC_FPSTATE_OFFSETOF_DATAOFF  20
#define __LIBC_FPSTATE_OFFSETOF_DATASEL  24
#define __LIBC_FPSTATE_OFFSETOF_ST       28
#define __LIBC_FPSTATE_OFFSETOF_STATUS   108
#define __LIBC_FPSTATE_SIZE              112
struct _libc_fpstate {
 unsigned long int  cw;
 unsigned long int  sw;
 unsigned long int  tag;
 unsigned long int  ipoff;
 unsigned long int  cssel;
 unsigned long int  dataoff;
 unsigned long int  datasel;
 struct _libc_fpreg _st[8];
 unsigned long int  status;
};
/* Structure to describe FPU registers. */
typedef struct _libc_fpstate *fpregset_t;


#define __MCONTEXT_OFFSETOF_GREGS      0
#define __MCONTEXT_OFFSETOF_FPREGS    (__SIZEOF_GREG_T__*NGREG)
#define __MCONTEXT_OFFSETOF_OLDMASK   (__SIZEOF_GREG_T__*NGREG+4)
#define __MCONTEXT_OFFSETOF_CR2       (__SIZEOF_GREG_T__*NGREG+8)
#define __MCONTEXT_SIZE               (__SIZEOF_GREG_T__*NGREG+12)
typedef struct {
 /* Context to describe whole processor state. */
 gregset_t gregs;
 /* Due to Linux's history we have to use a pointer here.
  * The SysV/i386 ABI requires a struct with the values. */
 fpregset_t fpregs;
 unsigned long int oldmask;
 unsigned long int cr2;
} mcontext_t;

#define __UCONTEXT_OFFSETOF_FLAGS      0
#define __UCONTEXT_OFFSETOF_LINK       __SIZEOF_LONG__
#define __UCONTEXT_OFFSETOF_STACK     (__SIZEOF_LONG__+__SIZEOF_POINTER__)
#define __UCONTEXT_OFFSETOF_MCONTEXT  (__SIZEOF_LONG__+__SIZEOF_POINTER__+__STACK_SIZE)
#define __UCONTEXT_OFFSETOF_SIGMASK   (__SIZEOF_LONG__+__SIZEOF_POINTER__+__STACK_SIZE+__MCONTEXT_SIZE)
#define __UCONTEXT_OFFSETOF_FPREGS    (__SIZEOF_LONG__+__SIZEOF_POINTER__+__STACK_SIZE+__MCONTEXT_SIZE+__SIZEOF_SIGSET_T__)
#define __UCONTEXT_SIZE               (__SIZEOF_LONG__+__SIZEOF_POINTER__+__STACK_SIZE+__MCONTEXT_SIZE+__SIZEOF_SIGSET_T__+__LIBC_FPSTATE_SIZE)

/* Userlevel context. */
typedef struct ucontext {
 unsigned long int    uc_flags;
 struct ucontext     *uc_link;
 stack_t              uc_stack;
 mcontext_t           uc_mcontext;
 __sigset_t           uc_sigmask;
 struct _libc_fpstate __fpregs_mem;
} ucontext_t;
#endif /* !__x86_64__ */

__DECL_END

#endif /* !_SYS_UCONTEXT_H */
