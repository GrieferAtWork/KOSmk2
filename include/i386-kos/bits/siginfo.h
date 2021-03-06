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
#ifndef _I386_KOS_BITS_SIGINFO_H
#define _I386_KOS_BITS_SIGINFO_H 1
#define _BITS_SIGINFO_H 1

#include <__stdinc.h>
#include <features.h>
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

__SYSDECL_BEGIN

#ifdef __CC__
#ifndef __have_sigval_t
#define __have_sigval_t 1
/* Type for data associated with a signal. */
typedef union sigval {
    int   sival_int;
    void *sival_ptr;
} sigval_t;
#endif /* !__have_sigval_t */
#endif /* __CC__ */

#ifndef __have_siginfo_t
#define __have_siginfo_t 1
#define __SI_MAX_SIZE    128
#define __SI_PAD_SIZE   (__SI_MAX_SIZE / 4)

#ifdef __CYG_COMPAT__
#ifdef __CC__
#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(push,4)
#endif

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("_sigcommune")
#pragma push_macro("_si_code")
#pragma push_macro("_si_read_handle")
#pragma push_macro("_si_write_handle")
#pragma push_macro("_si_process_handle")
#pragma push_macro("_si_fd")
#pragma push_macro("_si_pipe_unique_id")
#pragma push_macro("_si_str")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#undef _sigcommune
#undef _si_code
#undef _si_read_handle
#undef _si_write_handle
#undef _si_process_handle
#undef _si_fd
#undef _si_pipe_unique_id
#undef _si_str

struct __PACKED _sigcommune {
    __uint32_t       _si_code;
    void            *_si_read_handle;
    void            *_si_write_handle;
    void            *_si_process_handle;
    union __PACKED {
        __INT32_TYPE__ _si_fd;
        int64_t        _si_pipe_unique_id;
        char          *_si_str;
    };
};

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("_si_str")
#pragma pop_macro("_si_pipe_unique_id")
#pragma pop_macro("_si_fd")
#pragma pop_macro("_si_process_handle")
#pragma pop_macro("_si_write_handle")
#pragma pop_macro("_si_read_handle")
#pragma pop_macro("_si_code")
#pragma pop_macro("_sigcommune")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

typedef struct __PACKED {
    __INT32_TYPE__ si_signo;
    __INT32_TYPE__ si_code;
    __pid_t        si_pid;
    __uid_t        si_uid;
    __INT32_TYPE__ si_errno;
    union __PACKED {
        __uint32_t __pad[__SI_PAD_SIZE];
        struct _sigcommune _si_commune;
        struct __PACKED {
            union __PACKED {
                sigval_t        si_sigval;
                sigval_t        si_value;
            };
            struct __PACKED {
                timer_t         si_tid;
                __UINT32_TYPE__ si_overrun;
            };
        };
        struct __PACKED {
            __INT32_TYPE__  si_status;
            clock_t         si_utime;
            clock_t         si_stime;
        };
        void               *si_addr;
    };
} siginfo_t;

#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif
#endif /* __CC__ */

#else /* __CYG_COMPAT__ */

#if __SIZEOF_POINTER__ == 8
#   define __SI_PAD_SIZE  ((__SI_MAX_SIZE/4)-4)
#else
#   define __SI_PAD_SIZE  ((__SI_MAX_SIZE/4)-3)
#endif

#ifdef __CC__
#if defined(__x86_64__) && __SIZEOF_POINTER__ == 4
typedef __typedef_clock_t __ATTR_ALIGNED(4) __sigchld_clock_t;
#else /* ... */
typedef __typedef_clock_t __sigchld_clock_t;
#endif /* !... */
#endif /* __CC__ */
#define __SIZEOF_SIGCHLD_CLOCK_T__ __SIZEOF_CLOCK_T__

#define __SIGINFO_OFFSETOF_SIGNO        0
#define __SIGINFO_OFFSETOF_ERRNO        4
#define __SIGINFO_OFFSETOF_CODE      (2*4)
#define __SIGINFO_OFFSETOF_PID       (4*4)
#define __SIGINFO_OFFSETOF_UID       (4*4+__SIZEOF_PID_T__)
#define __SIGINFO_OFFSETOF_TIMERID   (4*4)
#define __SIGINFO_OFFSETOF_OVERRUN   (5*4)
#define __SIGINFO_OFFSETOF_VALUE     (6*4)
#define __SIGINFO_OFFSETOF_INT       (6*4)
#define __SIGINFO_OFFSETOF_PTR       (6*4)
#define __SIGINFO_OFFSETOF_STATUS    (4*4+2*__SIZEOF_PID_T__)
#if __SIZEOF_SIGCHLD_CLOCK_T__ >= 8
#define __SIGINFO_OFFSETOF_UTIME     (4*__SIZEOF_POINTER__)
#define __SIGINFO_OFFSETOF_STIME     (4*__SIZEOF_POINTER__+__SIZEOF_SIGCHLD_CLOCK_T__)
#else
#define __SIGINFO_OFFSETOF_UTIME     (5*4+2*__SIZEOF_PID_T__)
#define __SIGINFO_OFFSETOF_STIME     (5*4+2*__SIZEOF_PID_T__+__SIZEOF_SIGCHLD_CLOCK_T__)
#endif
#define __SIGINFO_OFFSETOF_ADDR      (4*4)
#define __SIGINFO_OFFSETOF_ADDR_LSB  (4*4+__SIZEOF_POINTER__)
#define __SIGINFO_OFFSETOF_LOWER     (4*4+2*__SIZEOF_POINTER__)
#define __SIGINFO_OFFSETOF_UPPER     (4*4+3*__SIZEOF_POINTER__)
#define __SIGINFO_OFFSETOF_BAND      (4*4)
#define __SIGINFO_OFFSETOF_FD        (4*4+__SIZEOF_POINTER__)
#define __SIGINFO_OFFSETOF_CALL_ADDR (4*4)
#define __SIGINFO_OFFSETOF_SYSCALL   (4*4+__SIZEOF_POINTER__)
#define __SIGINFO_OFFSETOF_ARCH      (5*4+__SIZEOF_POINTER__)
#ifdef __KERNEL__
#define __SIGINFO_SIZE               (__SIGINFO_OFFSETOF_STIME+__SIZEOF_SIGCHLD_CLOCK_T__)
#else
#define __SIGINFO_SIZE                __SI_MAX_SIZE
#endif


#ifdef __CC__
#if defined(__x86_64__) && __SIZEOF_POINTER__ == 4
__ATTR_ALIGNED(8)
#endif
typedef struct __siginfo_struct {
    __INT32_TYPE__ si_signo; /*< Signal number. */
    __INT32_TYPE__ si_errno; /*< If non-zero, an errno value associated with this signal, as defined in <errno.h>. */
    __INT32_TYPE__ si_code;  /*< Signal code. */
    __INT32_TYPE__ __si_pad0; /*< Invisible padding made visible. */
#if defined(__COMPILER_HAVE_TRANSPARENT_STRUCT) && \
    defined(__COMPILER_HAVE_TRANSPARENT_UNION)
#ifndef __USE_KOS
    struct {
#endif /* !__USE_KOS */
    union {
        struct { /* kill(). */
            __pid_t  si_pid; /*< Sending process ID. */
            __uid_t  si_uid; /*< Real user ID of sending process. */
        };
        struct { /* POSIX.1b timers. */
            __INT32_TYPE__ si_timerid; /*< Timer ID. */
            __INT32_TYPE__ si_overrun; /*< Overrun count. */
            union {
                sigval_t       si_value;   /*< Signal value. */
                __INT32_TYPE__ si_int;
                void          *si_ptr;
            };
        };
        struct { /* POSIX.1b signals. */
            __pid_t  __sig_si_pid;    /*< Sending process ID. */
            __uid_t  __sig_si_uid;    /*< Real user ID of sending process. */
            sigval_t __sig_si_sigval; /*< Signal value. */
        };
        struct { /* SIGCHLD. */
            __pid_t      __cld_si_pid;    /*< Which child. */
            __uid_t      __cld_si_uid;    /*< Real user ID of sending process. */
            __INT32_TYPE__     si_status; /*< Exit value or signal. */
#if __SIZEOF_SIGCHLD_CLOCK_T__ >= 8
            __UINT32_TYPE__  __si_pad1;   /*< Invisible padding made visible. */
#endif
            __sigchld_clock_t  si_utime;
            __sigchld_clock_t  si_stime;
        };
        struct { /* SIGILL, SIGFPE, SIGSEGV, SIGBUS. */
            void          *si_addr;     /*< Faulting insn/memory ref. */
            __INT16_TYPE__ si_addr_lsb; /*< Valid LSB of the reported address. */
            __INT16_TYPE__ __si_pad2[(sizeof(void *)/sizeof(__INT16_TYPE__))-1];
            void          *si_lower;
            void          *si_upper;
        };
        struct { /* SIGPOLL. */
            __LONGPTR_TYPE__ si_band; /*< Band event for SIGPOLL. */
            __INT32_TYPE__   si_fd;
        };
        struct { /* SIGSYS. */
            void           *si_call_addr; /*< Calling user insn. */
            __INT32_TYPE__  si_syscall;   /*< Triggering system call number. */
            __UINT32_TYPE__ si_arch;      /*< AUDIT_ARCH_* of syscall. */
        };
    };
#endif /* Transparent struct/union */
#if !defined(__COMPILER_HAVE_TRANSPARENT_STRUCT) || \
    !defined(__COMPILER_HAVE_TRANSPARENT_UNION) || \
    !defined(__USE_KOS)
    union {
#ifndef __KERNEL__
        __INT32_TYPE__ _pad[__SI_PAD_SIZE];
#endif
        struct { /* kill(). */
            __pid_t si_pid; /*< Sending process ID. */
            __uid_t si_uid; /*< Real user ID of sending process. */
        } _kill;
        struct { /* POSIX.1b timers. */
            __INT32_TYPE__ si_tid;     /*< Timer ID. */
            __INT32_TYPE__ si_overrun; /*< Overrun count. */
            sigval_t       si_sigval;  /*< Signal value. */
        } _timer;
        struct { /* POSIX.1b signals. */
            __pid_t  si_pid;    /*< Sending process ID. */
            __uid_t  si_uid;    /*< Real user ID of sending process. */
            sigval_t si_sigval; /*< Signal value. */
        } _rt;
        struct { /* SIGCHLD. */
            __pid_t           si_pid;    /*< Which child. */
            __uid_t           si_uid;    /*< Real user ID of sending process. */
            __INT32_TYPE__    si_status; /*< Exit value or signal. */
#if __SIZEOF_POINTER__ >= 8
            __UINT32_TYPE__  __si_pad1;   /*< Invisible padding made visible. */
#endif
            __sigchld_clock_t si_utime;
            __sigchld_clock_t si_stime;
        } _sigchld;
        struct { /* SIGILL, SIGFPE, SIGSEGV, SIGBUS. */
            void          *si_addr;     /*< Faulting insn/memory ref. */
            __INT16_TYPE__ si_addr_lsb; /*< Valid LSB of the reported address. */
            __INT16_TYPE__ __si_pad2[(sizeof(void *)/sizeof(__INT16_TYPE__))-1];
            struct {
                void *_lower;
                void *_upper;
            } si_addr_bnd;
        } _sigfault;
        struct { /* SIGPOLL. */
            __LONGPTR_TYPE__ si_band; /*< Band event for SIGPOLL. */
            __INT32_TYPE__   si_fd;
        } _sigpoll;
        struct { /* SIGSYS. */
            void           *_call_addr; /*< Calling user insn. */
            __INT32_TYPE__  _syscall;   /*< Triggering system call number. */
            __UINT32_TYPE__ _arch;      /*< AUDIT_ARCH_* of syscall. */
        } _sigsys;
    } _sifields;
#endif /* ... */
#if defined(__COMPILER_HAVE_TRANSPARENT_STRUCT) && \
    defined(__COMPILER_HAVE_TRANSPARENT_UNION)
#ifndef __USE_KOS
    };
#endif /* !__USE_KOS */
#else
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
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
} siginfo_t;
#endif /* !__CYG_COMPAT__ */
#endif /* __CC__ */

#ifdef __CYG_COMPAT__

#define SI_USER       0
#define SI_ASYNCIO    2
#define SI_MESGQ      3
#define SI_TIMER      4
#define SI_QUEUE      5
#define SI_KERNEL     6
#define ILL_ILLOPC    7
#define ILL_ILLOPN    8
#define ILL_ILLADR    9
#define ILL_ILLTRP    10
#define ILL_PRVOPC    11
#define ILL_PRVREG    12
#define ILL_COPROC    13
#define ILL_BADSTK    14
#define FPE_INTDIV    15
#define FPE_INTOVF    16
#define FPE_FLTDIV    17
#define FPE_FLTOVF    18
#define FPE_FLTUND    19
#define FPE_FLTRES    20
#define FPE_FLTINV    21
#define FPE_FLTSUB    22
#define SEGV_MAPERR   23
#define SEGV_ACCERR   24
#define BUS_ADRALN    25
#define BUS_ADRERR    26
#define BUS_OBJERR    27
#define CLD_EXITED    28
#define CLD_KILLED    29
#define CLD_DUMPED    30
#define CLD_TRAPPED   31
#define CLD_STOPPED   32
#define CLD_CONTINUED 33

#define SIGEV_SIGNAL  0
#define SIGEV_NONE    1
#define SIGEV_THREAD  2

#else /* __CYG_COMPAT__ */

/* Values for `si_code'. Positive values are reserved for kernel-generated signals. */
#ifdef __COMPILER_PREFERR_ENUMS
enum {
    SI_ASYNCNL = -60, /*< Sent by asynch name lookup completion. */
    SI_TKILL   = -6,  /*< Sent by tkill. */
    SI_SIGIO   = -5,  /*< Sent by queued SIGIO. */
    SI_ASYNCIO = -4,  /*< Sent by AIO completion. */
    SI_MESGQ   = -3,  /*< Sent by real time mesq state change. */
    SI_TIMER   = -2,  /*< Sent by timer expiration. */
    SI_QUEUE   = -1,  /*< Sent by sigqueue. */
    SI_USER    = 0,   /*< Sent by kill, sigsend. */
    SI_KERNEL  = 0x80 /*< Send by kernel. */
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
#else /* __COMPILER_PREFERR_ENUMS */
#define SI_ASYNCNL (-60)  /*< Sent by asynch name lookup completion. */
#define SI_TKILL   (-6)   /*< Sent by tkill. */
#define SI_SIGIO   (-5)   /*< Sent by queued SIGIO. */
#define SI_ASYNCIO (-4)   /*< Sent by AIO completion. */
#define SI_MESGQ   (-3)   /*< Sent by real time mesq state change. */
#define SI_TIMER   (-2)   /*< Sent by timer expiration. */
#define SI_QUEUE   (-1)   /*< Sent by sigqueue. */
#define SI_USER      0    /*< Sent by kill, sigsend. */
#define SI_KERNEL    0x80 /*< Send by kernel. */
#endif /* !__COMPILER_PREFERR_ENUMS */

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
/* `si_code' values for SIGILL signal. */
#ifdef __COMPILER_PREFERR_ENUMS
enum {
    ILL_ILLOPC = 1, /*< Illegal opcode. */
    ILL_ILLOPN = 2, /*< Illegal operand. */
    ILL_ILLADR = 3, /*< Illegal addressing mode. */
    ILL_ILLTRP = 4, /*< Illegal trap. */
    ILL_PRVOPC = 5, /*< Privileged opcode. */
    ILL_PRVREG = 6, /*< Privileged register. */
    ILL_COPROC = 7, /*< Coprocessor error. */
    ILL_BADSTK = 8  /*< Internal stack error. */
};
#define ILL_ILLOPC    ILL_ILLOPC
#define ILL_ILLOPN    ILL_ILLOPN
#define ILL_ILLADR    ILL_ILLADR
#define ILL_ILLTRP    ILL_ILLTRP
#define ILL_PRVOPC    ILL_PRVOPC
#define ILL_PRVREG    ILL_PRVREG
#define ILL_COPROC    ILL_COPROC
#define ILL_BADSTK    ILL_BADSTK
#else /* __COMPILER_PREFERR_ENUMS */
#define ILL_ILLOPC    1 /*< Illegal opcode. */
#define ILL_ILLOPN    2 /*< Illegal operand. */
#define ILL_ILLADR    3 /*< Illegal addressing mode. */
#define ILL_ILLTRP    4 /*< Illegal trap. */
#define ILL_PRVOPC    5 /*< Privileged opcode. */
#define ILL_PRVREG    6 /*< Privileged register. */
#define ILL_COPROC    7 /*< Coprocessor error. */
#define ILL_BADSTK    8 /*< Internal stack error. */
#endif /* !__COMPILER_PREFERR_ENUMS */

/* `si_code' values for SIGFPE signal. */
#ifdef __COMPILER_PREFERR_ENUMS
enum {
    FPE_INTDIV = 1, /*< Integer divide by zero. */
    FPE_INTOVF = 2, /*< Integer overflow. */
    FPE_FLTDIV = 3, /*< Floating point divide by zero. */
    FPE_FLTOVF = 4, /*< Floating point overflow. */
    FPE_FLTUND = 5, /*< Floating point underflow. */
    FPE_FLTRES = 6, /*< Floating point inexact result. */
    FPE_FLTINV = 7, /*< Floating point invalid operation. */
    FPE_FLTSUB = 8  /*< Subscript out of range. */
};
#define FPE_INTDIV    FPE_INTDIV
#define FPE_INTOVF    FPE_INTOVF
#define FPE_FLTDIV    FPE_FLTDIV
#define FPE_FLTOVF    FPE_FLTOVF
#define FPE_FLTUND    FPE_FLTUND
#define FPE_FLTRES    FPE_FLTRES
#define FPE_FLTINV    FPE_FLTINV
#define FPE_FLTSUB    FPE_FLTSUB
#else /* __COMPILER_PREFERR_ENUMS */
#define FPE_INTDIV    1 /*< Integer divide by zero. */
#define FPE_INTOVF    2 /*< Integer overflow. */
#define FPE_FLTDIV    3 /*< Floating point divide by zero. */
#define FPE_FLTOVF    4 /*< Floating point overflow. */
#define FPE_FLTUND    5 /*< Floating point underflow. */
#define FPE_FLTRES    6 /*< Floating point inexact result. */
#define FPE_FLTINV    7 /*< Floating point invalid operation. */
#define FPE_FLTSUB    8 /*< Subscript out of range. */
#endif /* !__COMPILER_PREFERR_ENUMS */

/* `si_code' values for SIGSEGV signal. */
#ifdef __COMPILER_PREFERR_ENUMS
enum {
    SEGV_MAPERR = 1, /*< Address not mapped to object. */
    SEGV_ACCERR = 2  /*< Invalid permissions for mapped object. */
};
#define SEGV_MAPERR    SEGV_MAPERR
#define SEGV_ACCERR    SEGV_ACCERR
#else /* __COMPILER_PREFERR_ENUMS */
#define SEGV_MAPERR 1 /*< Address not mapped to object. */
#define SEGV_ACCERR 2 /*< Invalid permissions for mapped object. */
#endif /* !__COMPILER_PREFERR_ENUMS */

/* `si_code' values for SIGBUS signal. */
#ifdef __COMPILER_PREFERR_ENUMS
enum {
    BUS_ADRALN    = 1, /*< Invalid address alignment. */
    BUS_ADRERR    = 2, /*< Non-existant physical address. */
    BUS_OBJERR    = 3, /*< Object specific hardware error. */
    BUS_MCEERR_AR = 4, /*< Hardware memory error: action required. */
    BUS_MCEERR_AO = 5  /*< Hardware memory error: action optional. */
};
#define BUS_ADRALN    BUS_ADRALN
#define BUS_ADRERR    BUS_ADRERR
#define BUS_OBJERR    BUS_OBJERR
#define BUS_MCEERR_AR BUS_MCEERR_AR
#define BUS_MCEERR_AO BUS_MCEERR_AO
#else /* __COMPILER_PREFERR_ENUMS */
#define BUS_ADRALN    1 /*< Invalid address alignment. */
#define BUS_ADRERR    2 /*< Non-existant physical address. */
#define BUS_OBJERR    3 /*< Object specific hardware error. */
#define BUS_MCEERR_AR 4 /*< Hardware memory error: action required. */
#define BUS_MCEERR_AO 5 /*< Hardware memory error: action optional. */
#endif /* !__COMPILER_PREFERR_ENUMS */
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */

#ifdef __USE_XOPEN_EXTENDED
/* `si_code' values for SIGTRAP signal. */
#ifdef __COMPILER_PREFERR_ENUMS
enum {
    TRAP_BRKPT = 1, /*< Process breakpoint. */
    TRAP_TRACE = 2  /*< Process trace trap. */
};
#define TRAP_BRKPT    TRAP_BRKPT
#define TRAP_TRACE    TRAP_TRACE
#else /* __COMPILER_PREFERR_ENUMS */
#define TRAP_BRKPT 1 /*< Process breakpoint. */
#define TRAP_TRACE 2 /*< Process trace trap. */
#endif /* !__COMPILER_PREFERR_ENUMS */
#endif /* __USE_XOPEN_EXTENDED */

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
/* `si_code' values for SIGCHLD signal. */
#ifdef __COMPILER_PREFERR_ENUMS
enum {
    CLD_EXITED    = 1, /*< Child has exited. */
    CLD_KILLED    = 2, /*< Child was killed. */
    CLD_DUMPED    = 3, /*< Child terminated abnormally. */
    CLD_TRAPPED   = 4, /*< Traced child has trapped. */
    CLD_STOPPED   = 5, /*< Child has stopped. */
    CLD_CONTINUED = 6  /*< Stopped child has continued. */
};
#define CLD_EXITED    CLD_EXITED
#define CLD_KILLED    CLD_KILLED
#define CLD_DUMPED    CLD_DUMPED
#define CLD_TRAPPED   CLD_TRAPPED
#define CLD_STOPPED   CLD_STOPPED
#define CLD_CONTINUED CLD_CONTINUED
#else /* __COMPILER_PREFERR_ENUMS */
#define CLD_EXITED    1 /*< Child has exited. */
#define CLD_KILLED    2 /*< Child was killed. */
#define CLD_DUMPED    3 /*< Child terminated abnormally. */
#define CLD_TRAPPED   4 /*< Traced child has trapped. */
#define CLD_STOPPED   5 /*< Child has stopped. */
#define CLD_CONTINUED 6 /*< Stopped child has continued. */
#endif /* !__COMPILER_PREFERR_ENUMS */

/* `si_code' values for SIGPOLL signal. */
#ifdef __COMPILER_PREFERR_ENUMS
enum {
    POLL_IN  = 1, /*< Data input available. */
    POLL_OUT = 2, /*< Output buffers available. */
    POLL_MSG = 3, /*< Input message available.   */
    POLL_ERR = 4, /*< I/O error. */
    POLL_PRI = 5, /*< High priority input available. */
    POLL_HUP = 6  /*< Device disconnected. */
};
#define POLL_IN   POLL_IN
#define POLL_OUT  POLL_OUT
#define POLL_MSG  POLL_MSG
#define POLL_ERR  POLL_ERR
#define POLL_PRI  POLL_PRI
#define POLL_HUP  POLL_HUP
#else /* __COMPILER_PREFERR_ENUMS */
#define POLL_IN  1 /*< Data input available. */
#define POLL_OUT 2 /*< Output buffers available. */
#define POLL_MSG 3 /*< Input message available.   */
#define POLL_ERR 4 /*< I/O error. */
#define POLL_PRI 5 /*< High priority input available. */
#define POLL_HUP 6 /*< Device disconnected. */
#endif /* !__COMPILER_PREFERR_ENUMS */
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#endif /* !__CYG_COMPAT__ */
#endif /* !__have_siginfo_t */

__SYSDECL_END

#endif /* !_I386_KOS_BITS_SIGINFO_H */
