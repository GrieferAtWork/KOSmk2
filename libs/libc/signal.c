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
#ifndef GUARD_LIBS_LIBC_SIGNAL_C
#define GUARD_LIBS_LIBC_SIGNAL_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include "libc.h"
#include "system.h"
#include <hybrid/compiler.h>
#include <stddef.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

DECL_BEGIN

DEFINE_PUBLIC_ALIAS(__sysv_signal,sysv_signal);

#if 0 /* TODO: Move this list into shared kernel memory. */
DEFINE_PUBLIC_ALIAS(_sys_siglist,sys_siglist);
PUBLIC char const *const (sys_siglist)[_NSIG] = {
    [SIGHUP]    = "HUP",    /* "Hangup" */
    [SIGINT]    = "INT",    /* "Interrupt" */
    [SIGQUIT]   = "QUIT",   /* "Quit" */
    [SIGILL]    = "ILL",    /* "Illegal instruction" */
    [SIGTRAP]   = "TRAP",   /* "Trace/breakpoint trap" */
    [SIGABRT]   = "ABRT",   /* "Aborted" */
    [SIGFPE]    = "FPE",    /* "Floating point exception" */
    [SIGKILL]   = "KILL",   /* "Killed" */
    [SIGBUS]    = "BUS",    /* "Bus error" */
    [SIGSYS]    = "SYS",    /* "Bad system call" */
    [SIGSEGV]   = "SEGV",   /* "Segmentation fault" */
    [SIGPIPE]   = "PIPE",   /* "Broken pipe" */
    [SIGALRM]   = "ALRM",   /* "Alarm clock" */
    [SIGTERM]   = "TERM",   /* "Terminated" */
    [SIGURG]    = "URG",    /* "Urgent I/O condition" */
    [SIGSTOP]   = "STOP",   /* "Stopped (signal)" */
    [SIGTSTP]   = "TSTP",   /* "Stopped" */
    [SIGCONT]   = "CONT",   /* "Continued" */
    [SIGCHLD]   = "CHLD",   /* "Child exited" */
    [SIGTTIN]   = "TTIN",   /* "Stopped (tty input)" */
    [SIGTTOU]   = "TTOU",   /* "Stopped (tty output)" */
    [SIGPOLL]   = "POLL",   /* "I/O possible" */
    [SIGXCPU]   = "XCPU",   /* "CPU time limit exceeded" */
    [SIGXFSZ]   = "XFSZ",   /* "File size limit exceeded" */
    [SIGVTALRM] = "VTALRM", /* "Virtual timer expired" */
    [SIGPROF]   = "PROF",   /* "Profiling timer expired" */
    [SIGUSR1]   = "USR1",   /* "User defined signal 1" */
    [SIGUSR2]   = "USR2",   /* "User defined signal 2" */
    [SIGWINCH]  = "WINCH",  /* "Window changed" */
#ifdef SIGEMT
    [SIGEMT]    = "EMT",    /* "EMT trap" */
#endif
#ifdef SIGSTKFLT
    [SIGSTKFLT] = "STKFLT", /* "Stack fault" */
#endif
#ifdef SIGPWR
    [SIGPWR]    = "PWR",    /* "Power failure" */
#endif
#if defined(SIGINFO) && (!defined(SIGPWR) || SIGPWR != SIGINFO)
    [SIGINFO]   = "INFO",   /* "Information request" */
#endif
#if defined(SIGLOST) && (!defined(SIGPWR) || SIGPWR != SIGLOST)
    [SIGLOST]   = "LOST",   /* "Resource lost" */
#endif
};
#endif
DEFINE_PUBLIC_ALIAS(signal,bsd_signal);

/* We do just as linux and alias these to raise/signal,
 * even though doing so isn't 100% correct. */
DEFINE_PUBLIC_ALIAS(gsignal,raise);
DEFINE_PUBLIC_ALIAS(ssignal,bsd_signal);



PUBLIC sighandler_t (LIBCCALL sysv_signal)(int sig, sighandler_t handler) {
 struct sigaction act,oact;
 if (handler == SIG_ERR || sig <= 0 ||
     sig >= NSIG) { __set_errno(EINVAL); return SIG_ERR; }
 act.sa_handler = handler;
 __sigemptyset(&act.sa_mask);
 act.sa_flags  = SA_ONESHOT|SA_NOMASK|SA_INTERRUPT;
 act.sa_flags &= ~SA_RESTART;
 if (sigaction(sig,&act,&oact) < 0) return SIG_ERR;
 return oact.sa_handler;
}
PRIVATE sigset_t __sigintr;
PUBLIC sighandler_t (LIBCCALL bsd_signal)(int sig, sighandler_t handler) {
 struct sigaction act,oact;
 if (handler == SIG_ERR || sig <= 0 ||
     sig >= NSIG) { __set_errno(EINVAL); return SIG_ERR; }
 act.sa_handler = handler;
 __sigemptyset(&act.sa_mask);
 __sigaddset(&act.sa_mask,sig);
 act.sa_flags = __sigismember(&__sigintr,sig) ? 0 : SA_RESTART;
 if (sigaction(sig,&act,&oact) < 0) return SIG_ERR;
 return oact.sa_handler;
}
PUBLIC int (LIBCCALL siginterrupt)(int sig, int interrupt) {
 struct sigaction action;
 if (sigaction(sig,(struct sigaction *) NULL,&action) < 0) return -1;
 if (interrupt) {
  __sigaddset(&__sigintr,sig);
  action.sa_flags &= ~SA_RESTART;
 } else {
  __sigdelset(&__sigintr,sig);
  action.sa_flags |= SA_RESTART;
 }
 if (sigaction(sig,&action,(struct sigaction *) NULL) < 0)
     return -1;
 return 0;
}


PUBLIC sighandler_t (LIBCCALL sigset)(int sig, sighandler_t disp) {
 struct sigaction act,oact; sigset_t set,oset;
 if (disp == SIG_ERR || sig <= 0 ||
     sig >= NSIG) { __set_errno (EINVAL); return SIG_ERR; }
 __sigemptyset(&set);
 __sigaddset(&set,sig);
 if (disp == SIG_HOLD) {
  if (sigprocmask(SIG_BLOCK,&set,&oset) < 0) return SIG_ERR;
  if (__sigismember(&oset,sig)) return SIG_HOLD;
  if (sigaction(sig,NULL,&oact) < 0) return SIG_ERR;
  return oact.sa_handler;
 }
 act.sa_handler = disp;
 __sigemptyset(&act.sa_mask);
 act.sa_flags = 0;
 if (sigaction(sig,&act,&oact) < 0) return SIG_ERR;
 if (sigprocmask(SIG_UNBLOCK,&set,&oset) < 0) return SIG_ERR;
 return __sigismember(&oset,sig) ? SIG_HOLD : oact.sa_handler;
}

#define SIGSETFN(name,body,const) \
PUBLIC int (LIBCCALL name)(sigset_t const *set, int sig) { \
 unsigned long int mask = __sigmask(sig); \
 unsigned long int word = __sigword(sig); \
 return body; \
}
SIGSETFN(sigismember,(set->__val[word] & mask) ? 1 : 0,const)
SIGSETFN(sigaddset,((set->__val[word] |= mask), 0),)
SIGSETFN(sigdelset,((set->__val[word] &= ~mask), 0),)
#undef SIGSETFN
DEFINE_PUBLIC_ALIAS(__sigismember,sigismember);
DEFINE_PUBLIC_ALIAS(__sigaddset,sigaddset);
DEFINE_PUBLIC_ALIAS(__sigdelset,sigdelset);
PUBLIC int (LIBCCALL sigemptyset)(sigset_t *set) { return __sigemptyset(set); }
PUBLIC int (LIBCCALL sigfillset)(sigset_t *set) { return __sigfillset(set); }
PUBLIC int (LIBCCALL sigisemptyset)(sigset_t const *set) { return __sigisemptyset(set); }
PUBLIC int (LIBCCALL sigandset)(sigset_t *set, sigset_t const *left, sigset_t const *right) { return __sigandset(set,left,right); }
PUBLIC int (LIBCCALL sigorset)(sigset_t *set, sigset_t const *left, sigset_t const *right) { return __sigorset(set,left,right); }
PUBLIC int (LIBCCALL sigaction)(int sig, struct sigaction const *__restrict act, struct sigaction *__restrict oact) { return FORWARD_SYSTEM_ERROR(sys_sigaction(sig,act,oact,sizeof(sigset_t))); }
PUBLIC int (LIBCCALL sigprocmask)(int how, sigset_t const *__restrict set, sigset_t *__restrict oset) { return FORWARD_SYSTEM_ERROR(sys_sigprocmask(how,set,oset,sizeof(sigset_t))); }
PUBLIC int (LIBCCALL pthread_sigmask)(int how, sigset_t const *__restrict newmask, sigset_t *__restrict oldmask) { return sigprocmask(how,newmask,oldmask); /* ??? */ }
PUBLIC int (LIBCCALL sigwaitinfo)(sigset_t const *__restrict set, siginfo_t *__restrict info) { return sigtimedwait(set,info,NULL); }
PUBLIC int (LIBCCALL sigpending)(sigset_t *set) { return FORWARD_SYSTEM_ERROR(sys_sigpending(set,sizeof(sigset_t))); }
PUBLIC int (LIBCCALL sigwait)(sigset_t const *__restrict set, int *__restrict sig) { siginfo_t info; int error = sigtimedwait(set,&info,NULL); if (!error) *sig = info.si_signo; return error; }
PUBLIC int (LIBCCALL kill)(pid_t pid, int sig) { return FORWARD_SYSTEM_ERROR(sys_kill(pid,sig)); }
PUBLIC int (LIBCCALL killpg)(pid_t pgrp, int sig) { return kill(-pgrp,sig); }
PUBLIC int (LIBCCALL raise)(int sig) { return kill(getpid(),sig); }
PUBLIC ATTR_NORETURN void (LIBCCALL sigreturn)(struct sigcontext const *scp) { sys_sigreturn(scp); }
PUBLIC int (LIBCCALL sigblock)(int mask) { return FORWARD_SYSTEM_ERROR(sys_sigprocmask(SIG_BLOCK,(sigset_t *)&mask,NULL,sizeof(mask))); }
PUBLIC int (LIBCCALL sigsetmask)(int mask) { return FORWARD_SYSTEM_ERROR(sys_sigprocmask(SIG_SETMASK,(sigset_t *)&mask,NULL,sizeof(mask))); }
PUBLIC int (LIBCCALL siggetmask)(void) {
 int value,result = FORWARD_SYSTEM_ERROR(sys_sigprocmask(SIG_SETMASK,NULL,
                                        (sigset_t *)&value,sizeof(value)));
 if (!result) result = value;
 return result;
}
PRIVATE int (LIBCCALL set_single_signal_action)(int sig, int how) {
 sigset_t set;
 __sigemptyset(&set);
 __sigaddset(&set,sig);
 return sigprocmask(SIG_BLOCK,&set,NULL);
}
PUBLIC int (LIBCCALL sighold)(int sig) { return set_single_signal_action(sig,SIG_BLOCK); }
PUBLIC int (LIBCCALL sigrelse)(int sig) { return set_single_signal_action(sig,SIG_UNBLOCK); }
PUBLIC int (LIBCCALL sigignore)(int sig) { return signal(sig,SIG_IGN) == SIG_ERR ? -1 : 0; }
PUBLIC int (LIBCCALL __libc_current_sigrtmin)(void) { return __SIGRTMIN; }
PUBLIC int (LIBCCALL __libc_current_sigrtmax)(void) { return __SIGRTMAX; }

PUBLIC int (LIBCCALL sigaltstack)(struct sigaltstack const *__restrict ss,
                                  struct sigaltstack *__restrict oss) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC int (LIBCCALL sigsuspend)(sigset_t const *set) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
PUBLIC int (LIBCCALL pthread_kill)(pthread_t threadid, int signo) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL pthread_sigqueue)(pthread_t threadid, int signo, union sigval const value) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL __xpg_sigpause)(int sig) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL sigqueue)(pid_t pid, int sig, union sigval const val) { NOT_IMPLEMENTED(); return 0; }
PUBLIC void (LIBCCALL psignal)(int sig, char const *s) { NOT_IMPLEMENTED(); }
PUBLIC void (LIBCCALL psiginfo)(siginfo_t const *pinfo, char const *s) { NOT_IMPLEMENTED(); }
PUBLIC int (LIBCCALL sigstack)(struct sigstack *ss, struct sigstack *oss) {
 struct sigaltstack ass,aoss; int result;
 if (ss) {
  ass.ss_flags = ss->ss_onstack ? SS_ONSTACK : SS_DISABLE;
  ass.ss_sp    = ss->ss_sp;
  ass.ss_size  = (size_t)-1;
 }
 result = sigaltstack(ss ? &ass : NULL,oss ? &aoss : NULL);
 if (!result && oss) {
  oss->ss_onstack = !!(aoss.ss_flags&SS_ONSTACK);
  oss->ss_sp      = aoss.ss_sp;
 }
 return result;
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_SIGNAL_C */
