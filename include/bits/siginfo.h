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
#ifndef _BITS_SIGINFO_H
#define _BITS_SIGINFO_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <bits/sigevent.h>

/* siginfo_t, sigevent and constants.  Linux x86-64 version.
   Copyright (C) 2012-2016 Free Software Foundation, Inc.
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

#ifndef __have_sigval_t
#define __have_sigval_t 1
/* Type for data associated with a signal. */
typedef union sigval {
 int   sival_int;
 void *sival_ptr;
} sigval_t;
#endif

#ifndef __have_siginfo_t
#define __have_siginfo_t 1
#define __SI_MAX_SIZE    128
#if __SIZEOF_POINTER__ == 8
#   define __SI_PAD_SIZE  ((__SI_MAX_SIZE/__SIZEOF_INT__)-4)
#else
#   define __SI_PAD_SIZE  ((__SI_MAX_SIZE/__SIZEOF_INT__)-3)
#endif
#if defined(__x86_64__) && __SIZEOF_POINTER__ == 4
#define __SI_ALIGNMENT __attribute__((__aligned__(8)))
typedef __clock_t __attribute__((__aligned__(4))) __sigchld_clock_t;
#else
#define __SI_ALIGNMENT
typedef __clock_t __sigchld_clock_t;
#endif

typedef struct __siginfo_struct {
 int si_signo; /*< Signal number. */
 int si_errno; /*< If non-zero, an errno value associated with this signal, as defined in <errno.h>. */
 int si_code;  /*< Signal code. */
 union {
#ifndef __KERNEL__
  int _pad[__SI_PAD_SIZE];
#endif
  struct { /* kill(). */
   __pid_t si_pid; /*< Sending process ID. */
   __uid_t si_uid; /*< Real user ID of sending process. */
  } _kill;
  struct { /* POSIX.1b timers. */
   int      si_tid;     /*< Timer ID. */
   int      si_overrun; /*< Overrun count. */
   sigval_t si_sigval;  /*< Signal value. */
  } _timer;
  struct { /* POSIX.1b signals. */
   __pid_t  si_pid;    /*< Sending process ID. */
   __uid_t  si_uid;    /*< Real user ID of sending process. */
   sigval_t si_sigval; /*< Signal value. */
  } _rt;
  struct { /* SIGCHLD. */
   __pid_t           si_pid;    /*< Which child. */
   __uid_t           si_uid;    /*< Real user ID of sending process. */
   int               si_status; /*< Exit value or signal. */
   __sigchld_clock_t si_utime;
   __sigchld_clock_t si_stime;
  } _sigchld;
  struct { /* SIGILL, SIGFPE, SIGSEGV, SIGBUS. */
   void     *si_addr;     /*< Faulting insn/memory ref. */
   short int si_addr_lsb; /*< Valid LSB of the reported address. */
   struct {
    void *_lower;
    void *_upper;
   } si_addr_bnd;
  } _sigfault;
  struct { /* SIGPOLL. */
   long int si_band; /*< Band event for SIGPOLL. */
   int      si_fd;
  } _sigpoll;
  struct { /* SIGSYS. */
   void        *_call_addr; /*< Calling user insn. */
   int          _syscall;   /*< Triggering system call number. */
   unsigned int _arch;      /*< AUDIT_ARCH_* of syscall. */
  } _sigsys;
 } _sifields;
} siginfo_t __SI_ALIGNMENT;

/* X/Open requires some more fields with fixed names. */
#define si_pid       _sifields._kill.si_pid
#define si_uid       _sifields._kill.si_uid
#define si_timerid   _sifields._timer.si_tid
#define si_overrun   _sifields._timer.si_overrun
#define si_status    _sifields._sigchld.si_status
#define si_utime     _sifields._sigchld.si_utime
#define si_stime     _sifields._sigchld.si_stime
#define si_value     _sifields._rt.si_sigval
#define si_int       _sifields._rt.si_sigval.sival_int
#define si_ptr       _sifields._rt.si_sigval.sival_ptr
#define si_addr      _sifields._sigfault.si_addr
#define si_addr_lsb  _sifields._sigfault.si_addr_lsb
#define si_lower     _sifields._sigfault.si_addr_bnd._lower
#define si_upper     _sifields._sigfault.si_addr_bnd._upper
#define si_band      _sifields._sigpoll.si_band
#define si_fd        _sifields._sigpoll.si_fd
#define si_call_addr _sifields._sigsys._call_addr
#define si_syscall   _sifields._sigsys._syscall
#define si_arch      _sifields._sigsys._arch


/* Values for `si_code'. Positive values are reserved for kernel-generated signals. */
enum {
 SI_ASYNCNL = -60, /*< Sent by asynch name lookup completion. */
 SI_TKILL = -6,    /*< Sent by tkill. */
 SI_SIGIO,         /*< Sent by queued SIGIO. */
 SI_ASYNCIO,       /*< Sent by AIO completion. */
 SI_MESGQ,         /*< Sent by real time mesq state change. */
 SI_TIMER,         /*< Sent by timer expiration. */
 SI_QUEUE,         /*< Sent by sigqueue. */
 SI_USER,          /*< Sent by kill, sigsend. */
 SI_KERNEL = 0x80  /*< Send by kernel. */
};
#define SI_ASYNCNL SI_ASYNCNL
#define SI_TKILL   SI_TKILL
#define SI_SIGIO   SI_SIGIO
#define SI_ASYNCIO SI_ASYNCIO
#define SI_MESGQ   SI_MESGQ
#define SI_TIMER   SI_TIMER
#define SI_QUEUE   SI_QUEUE
#define SI_USER    SI_USER
#define SI_KERNEL  SI_KERNEL

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
/* `si_code' values for SIGILL signal. */
enum {
 ILL_ILLOPC = 1, /*< Illegal opcode. */
 ILL_ILLOPN,     /*< Illegal operand. */
 ILL_ILLADR,     /*< Illegal addressing mode. */
 ILL_ILLTRP,     /*< Illegal trap. */
 ILL_PRVOPC,     /*< Privileged opcode. */
 ILL_PRVREG,     /*< Privileged register. */
 ILL_COPROC,     /*< Coprocessor error. */
 ILL_BADSTK      /*< Internal stack error. */
};
#define ILL_ILLOPC    ILL_ILLOPC
#define ILL_ILLOPN    ILL_ILLOPN
#define ILL_ILLADR    ILL_ILLADR
#define ILL_ILLTRP    ILL_ILLTRP
#define ILL_PRVOPC    ILL_PRVOPC
#define ILL_PRVREG    ILL_PRVREG
#define ILL_COPROC    ILL_COPROC
#define ILL_BADSTK    ILL_BADSTK

/* `si_code' values for SIGFPE signal. */
enum {
 FPE_INTDIV = 1, /*< Integer divide by zero. */
 FPE_INTOVF,     /*< Integer overflow. */
 FPE_FLTDIV,     /*< Floating point divide by zero. */
 FPE_FLTOVF,     /*< Floating point overflow. */
 FPE_FLTUND,     /*< Floating point underflow. */
 FPE_FLTRES,     /*< Floating point inexact result. */
 FPE_FLTINV,     /*< Floating point invalid operation. */
 FPE_FLTSUB      /*< Subscript out of range. */
};
#define FPE_INTDIV    FPE_INTDIV
#define FPE_INTOVF    FPE_INTOVF
#define FPE_FLTDIV    FPE_FLTDIV
#define FPE_FLTOVF    FPE_FLTOVF
#define FPE_FLTUND    FPE_FLTUND
#define FPE_FLTRES    FPE_FLTRES
#define FPE_FLTINV    FPE_FLTINV
#define FPE_FLTSUB    FPE_FLTSUB

/* `si_code' values for SIGSEGV signal. */
enum {
 SEGV_MAPERR = 1, /*< Address not mapped to object. */
 SEGV_ACCERR      /*< Invalid permissions for mapped object. */
};
#define SEGV_MAPERR    SEGV_MAPERR
#define SEGV_ACCERR    SEGV_ACCERR

/* `si_code' values for SIGBUS signal. */
enum {
 BUS_ADRALN = 1, /*< Invalid address alignment. */
 BUS_ADRERR,     /*< Non-existant physical address. */
 BUS_OBJERR,     /*< Object specific hardware error. */
 BUS_MCEERR_AR,  /*< Hardware memory error: action required. */
 BUS_MCEERR_AO   /*< Hardware memory error: action optional. */
};
#define BUS_ADRALN    BUS_ADRALN
#define BUS_ADRERR    BUS_ADRERR
#define BUS_OBJERR    BUS_OBJERR
#define BUS_MCEERR_AR BUS_MCEERR_AR
#define BUS_MCEERR_AO BUS_MCEERR_AO
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */

#ifdef __USE_XOPEN_EXTENDED
enum { /* `si_code' values for SIGTRAP signal. */
 TRAP_BRKPT = 1, /*< Process breakpoint. */
 TRAP_TRACE      /*< Process trace trap. */
};
#  define TRAP_BRKPT    TRAP_BRKPT
#  define TRAP_TRACE    TRAP_TRACE
#endif /* __USE_XOPEN_EXTENDED */

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
enum { /* `si_code' values for SIGCHLD signal. */
 CLD_EXITED = 1, /*< Child has exited. */
 CLD_KILLED,     /*< Child was killed. */
 CLD_DUMPED,     /*< Child terminated abnormally. */
 CLD_TRAPPED,    /*< Traced child has trapped. */
 CLD_STOPPED,    /*< Child has stopped. */
 CLD_CONTINUED   /*< Stopped child has continued. */
};
#define CLD_EXITED    CLD_EXITED
#define CLD_KILLED    CLD_KILLED
#define CLD_DUMPED    CLD_DUMPED
#define CLD_TRAPPED   CLD_TRAPPED
#define CLD_STOPPED   CLD_STOPPED
#define CLD_CONTINUED CLD_CONTINUED

enum { /* `si_code' values for SIGPOLL signal. */
 POLL_IN = 1, /*< Data input available. */
 POLL_OUT,    /*< Output buffers available. */
 POLL_MSG,    /*< Input message available.   */
 POLL_ERR,    /*< I/O error. */
 POLL_PRI,    /*< High priority input available. */
 POLL_HUP     /*< Device disconnected. */
};
#define POLL_IN   POLL_IN
#define POLL_OUT  POLL_OUT
#define POLL_MSG  POLL_MSG
#define POLL_ERR  POLL_ERR
#define POLL_PRI  POLL_PRI
#define POLL_HUP  POLL_HUP
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#endif /* !__have_siginfo_t */

__DECL_END

#endif /* !_BITS_SIGINFO_H */
