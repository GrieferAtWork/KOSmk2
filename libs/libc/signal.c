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
#include "signal.h"
#include "unistd.h"
#include "errno.h"

#include <hybrid/compiler.h>
#include <stddef.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

DECL_BEGIN

#if 0 /* TODO: Move this list into shared kernel memory. */
DEFINE_PUBLIC_ALIAS(sys_siglist,libc_sys_siglist);
DEFINE_PUBLIC_ALIAS(_sys_siglist,libc_sys_siglist);
INTERN char const *const libc_sys_siglist[_NSIG] = {
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

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN sighandler_t LIBCCALL
libc_dos_signal(int sig, sighandler_t handler) {
 sighandler_t result;
 /* DOS-specific signal handler:
  * Don't actually set, just return the current. */
 if (handler == __DOS_SIG_GET) {
  struct sigaction oact;
  if (libc_sigaction(sig,NULL,&oact) < 0)
      return SIG_ERR;
  return oact.sa_handler;
 }
 /* Translate extended DOS signal handlers. */
 if (handler == __DOS_SIG_SGE) handler = SIG_IGN; /* ??? */
 else if (handler == __DOS_SIG_ACK) handler = SIG_IGN; /* ??? */
 else if (handler == __DOS_SIG_HOLD) handler = SIG_HOLD;
 result = libc_sysv_signal(sig,handler);
 return result;
}
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

INTERN sighandler_t LIBCCALL
libc_sysv_signal(int sig, sighandler_t handler) {
 struct sigaction act,oact;
 if (handler == SIG_ERR || sig <= 0 ||
     sig >= NSIG) { SET_ERRNO(EINVAL); return SIG_ERR; }
 act.sa_handler = handler;
 libc_sigemptyset(&act.sa_mask);
 act.sa_flags  = SA_ONESHOT|SA_NOMASK|SA_INTERRUPT;
 act.sa_flags &= ~SA_RESTART;
 if (libc_sigaction(sig,&act,&oact) < 0) return SIG_ERR;
 return oact.sa_handler;
}
PRIVATE sigset_t __sigintr;
INTERN sighandler_t LIBCCALL libc_bsd_signal(int sig, sighandler_t handler) {
 struct sigaction act,oact;
 if (handler == SIG_ERR || sig <= 0 ||
     sig >= NSIG) { SET_ERRNO(EINVAL); return SIG_ERR; }
 act.sa_handler = handler;
 libc_sigemptyset(&act.sa_mask);
 libc_sigaddset(&act.sa_mask,sig);
 act.sa_flags = libc_sigismember(&__sigintr,sig) ? 0 : SA_RESTART;
 if (libc_sigaction(sig,&act,&oact) < 0) return SIG_ERR;
 return oact.sa_handler;
}
INTERN int LIBCCALL libc_siginterrupt(int sig, int interrupt) {
 struct sigaction action;
 if (libc_sigaction(sig,(struct sigaction *) NULL,&action) < 0) return -1;
 if (interrupt) {
  libc_sigaddset(&__sigintr,sig);
  action.sa_flags &= ~SA_RESTART;
 } else {
  libc_sigdelset(&__sigintr,sig);
  action.sa_flags |= SA_RESTART;
 }
 if (libc_sigaction(sig,&action,(struct sigaction *) NULL) < 0)
     return -1;
 return 0;
}
INTERN sighandler_t LIBCCALL libc_sigset(int sig, sighandler_t disp) {
 struct sigaction act,oact; sigset_t set,oset;
 if (disp == SIG_ERR || sig <= 0 ||
     sig >= NSIG) { SET_ERRNO (EINVAL); return SIG_ERR; }
 libc_sigemptyset(&set);
 libc_sigaddset(&set,sig);
 if (disp == SIG_HOLD) {
  if (libc_sigprocmask(SIG_BLOCK,&set,&oset) < 0) return SIG_ERR;
  if (libc_sigismember(&oset,sig)) return SIG_HOLD;
  if (libc_sigaction(sig,NULL,&oact) < 0) return SIG_ERR;
  return oact.sa_handler;
 }
 act.sa_handler = disp;
 libc_sigemptyset(&act.sa_mask);
 act.sa_flags = 0;
 if (libc_sigaction(sig,&act,&oact) < 0) return SIG_ERR;
 if (libc_sigprocmask(SIG_UNBLOCK,&set,&oset) < 0) return SIG_ERR;
 return libc_sigismember(&oset,sig) ? SIG_HOLD : oact.sa_handler;
}

#define SIGSETFN(name,body,const) \
INTERN int LIBCCALL name(sigset_t const *set, int sig) { \
 unsigned long int mask = __sigmask(sig); \
 unsigned long int word = __sigword(sig); \
 return body; \
}
SIGSETFN(libc_sigismember,(set->__val[word] & mask) ? 1 : 0,const)
SIGSETFN(libc_sigaddset,((set->__val[word] |= mask), 0),)
SIGSETFN(libc_sigdelset,((set->__val[word] &= ~mask), 0),)
#undef SIGSETFN
INTERN int LIBCCALL libc_sigemptyset(sigset_t *set) { return __sigemptyset(set); }
INTERN int LIBCCALL libc_sigfillset(sigset_t *set) { return __sigfillset(set); }
INTERN int LIBCCALL libc_sigisemptyset(sigset_t const *set) { return __sigisemptyset(set); }
INTERN int LIBCCALL libc_sigandset(sigset_t *set, sigset_t const *left, sigset_t const *right) { return __sigandset(set,left,right); }
INTERN int LIBCCALL libc_sigorset(sigset_t *set, sigset_t const *left, sigset_t const *right) { return __sigorset(set,left,right); }
INTERN int LIBCCALL libc_sigaction(int sig, struct sigaction const *__restrict act, struct sigaction *__restrict oact) { return FORWARD_SYSTEM_ERROR(sys_sigaction(sig,act,oact,sizeof(sigset_t))); }
INTERN int LIBCCALL libc_sigprocmask(int how, sigset_t const *__restrict set, sigset_t *__restrict oset) { return FORWARD_SYSTEM_ERROR(sys_sigprocmask(how,set,oset,sizeof(sigset_t))); }
INTERN int LIBCCALL libc_pthread_sigmask(int how, sigset_t const *__restrict newmask, sigset_t *__restrict oldmask) { return libc_sigprocmask(how,newmask,oldmask); /* ??? */ }
INTERN int LIBCCALL libc_sigpending(sigset_t *set) { return FORWARD_SYSTEM_ERROR(sys_sigpending(set,sizeof(sigset_t))); }
INTERN int LIBCCALL libc_kill(pid_t pid, int sig) { return FORWARD_SYSTEM_ERROR(sys_kill(pid,sig)); }
INTERN int LIBCCALL libc_killpg(pid_t pgrp, int sig) { return libc_kill(-pgrp,sig); }
INTERN int LIBCCALL libc_raise(int sig) { return libc_kill(libc_getpid(),sig); }
INTERN ATTR_NORETURN void LIBCCALL libc_sigreturn(struct sigcontext const *scp) { sys_sigreturn(scp); }
INTERN int LIBCCALL libc_sigblock(int mask) { return FORWARD_SYSTEM_ERROR(sys_sigprocmask(SIG_BLOCK,(sigset_t *)&mask,NULL,sizeof(mask))); }
INTERN int LIBCCALL libc_sigsetmask(int mask) { return FORWARD_SYSTEM_ERROR(sys_sigprocmask(SIG_SETMASK,(sigset_t *)&mask,NULL,sizeof(mask))); }
INTERN int LIBCCALL libc_siggetmask(void) {
 int value,result = FORWARD_SYSTEM_ERROR(sys_sigprocmask(SIG_SETMASK,NULL,
                                        (sigset_t *)&value,sizeof(value)));
 if (!result) result = value;
 return result;
}
PRIVATE int LIBCCALL set_single_signal_action(int sig, int how) {
 sigset_t set;
 libc_sigemptyset(&set);
 libc_sigaddset(&set,sig);
 return libc_sigprocmask(SIG_BLOCK,&set,NULL);
}
INTERN int LIBCCALL libc_sighold(int sig) { return set_single_signal_action(sig,SIG_BLOCK); }
INTERN int LIBCCALL libc_sigrelse(int sig) { return set_single_signal_action(sig,SIG_UNBLOCK); }
INTERN int LIBCCALL libc_sigignore(int sig) { return libc_bsd_signal(sig,SIG_IGN) == SIG_ERR ? -1 : 0; }
INTERN int LIBCCALL libc___libc_current_sigrtmin(void) { return __SIGRTMIN; }
INTERN int LIBCCALL libc___libc_current_sigrtmax(void) { return __SIGRTMAX; }

INTERN int LIBCCALL libc_sigaltstack(struct sigaltstack const *__restrict ss,
                                     struct sigaltstack *__restrict oss) {
 /* TODO */
 NOT_IMPLEMENTED();
 return 0;
}
INTERN int LIBCCALL libc_sigsuspend(sigset_t const *set) { return FORWARD_SYSTEM_ERROR(sys_sigsuspend(set,sizeof(sigset_t))); }
INTERN int LIBCCALL libc_pthread_kill(pthread_t threadid, int signo) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_pthread_sigqueue(pthread_t threadid, int signo, union sigval const value) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc___xpg_sigpause(int sig) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_sigqueue(pid_t pid, int sig, union sigval const val) { NOT_IMPLEMENTED(); return 0; }
INTERN void LIBCCALL libc_psignal(int sig, char const *s) { NOT_IMPLEMENTED(); }
INTERN void LIBCCALL libc_psiginfo(siginfo_t const *pinfo, char const *s) { NOT_IMPLEMENTED(); }
INTERN int LIBCCALL libc_sigstack(struct sigstack *ss, struct sigstack *oss) {
 struct sigaltstack ass,aoss; int result;
 if (ss) {
  ass.ss_flags = ss->ss_onstack ? SS_ONSTACK : SS_DISABLE;
  ass.ss_sp    = ss->ss_sp;
  ass.ss_size  = (size_t)-1;
 }
 result = libc_sigaltstack(ss ? &ass : NULL,oss ? &aoss : NULL);
 if (!result && oss) {
  oss->ss_onstack = !!(aoss.ss_flags&SS_ONSTACK);
  oss->ss_sp      = aoss.ss_sp;
 }
 return result;
}


/* Define public signal functions. */
DEFINE_PUBLIC_ALIAS(sysv_signal,libc_sysv_signal);
DEFINE_PUBLIC_ALIAS(bsd_signal,libc_bsd_signal);
DEFINE_PUBLIC_ALIAS(siginterrupt,libc_siginterrupt);
DEFINE_PUBLIC_ALIAS(sigset,libc_sigset);
DEFINE_PUBLIC_ALIAS(sigismember,libc_sigismember);
DEFINE_PUBLIC_ALIAS(sigaddset,libc_sigaddset);
DEFINE_PUBLIC_ALIAS(sigdelset,libc_sigdelset);
DEFINE_PUBLIC_ALIAS(sigemptyset,libc_sigemptyset);
DEFINE_PUBLIC_ALIAS(sigfillset,libc_sigfillset);
DEFINE_PUBLIC_ALIAS(sigisemptyset,libc_sigisemptyset);
DEFINE_PUBLIC_ALIAS(sigandset,libc_sigandset);
DEFINE_PUBLIC_ALIAS(sigorset,libc_sigorset);
DEFINE_PUBLIC_ALIAS(sigaction,libc_sigaction);
DEFINE_PUBLIC_ALIAS(sigprocmask,libc_sigprocmask);
DEFINE_PUBLIC_ALIAS(pthread_sigmask,libc_pthread_sigmask);
DEFINE_PUBLIC_ALIAS(sigwaitinfo,libc_sigwaitinfo);
DEFINE_PUBLIC_ALIAS(sigpending,libc_sigpending);
DEFINE_PUBLIC_ALIAS(sigwait,libc_sigwait);
DEFINE_PUBLIC_ALIAS(kill,libc_kill);
DEFINE_PUBLIC_ALIAS(killpg,libc_killpg);
DEFINE_PUBLIC_ALIAS(raise,libc_raise);
DEFINE_PUBLIC_ALIAS(sigreturn,libc_sigreturn);
DEFINE_PUBLIC_ALIAS(sigblock,libc_sigblock);
DEFINE_PUBLIC_ALIAS(sigsetmask,libc_sigsetmask);
DEFINE_PUBLIC_ALIAS(siggetmask,libc_siggetmask);
DEFINE_PUBLIC_ALIAS(sighold,libc_sighold);
DEFINE_PUBLIC_ALIAS(sigrelse,libc_sigrelse);
DEFINE_PUBLIC_ALIAS(sigignore,libc_sigignore);
DEFINE_PUBLIC_ALIAS(__libc_current_sigrtmin,libc___libc_current_sigrtmin);
DEFINE_PUBLIC_ALIAS(__libc_current_sigrtmax,libc___libc_current_sigrtmax);
DEFINE_PUBLIC_ALIAS(sigaltstack,libc_sigaltstack);
DEFINE_PUBLIC_ALIAS(sigsuspend,libc_sigsuspend);
DEFINE_PUBLIC_ALIAS(pthread_kill,libc_pthread_kill);
DEFINE_PUBLIC_ALIAS(pthread_sigqueue,libc_pthread_sigqueue);
DEFINE_PUBLIC_ALIAS(__xpg_sigpause,libc___xpg_sigpause);
DEFINE_PUBLIC_ALIAS(sigqueue,libc_sigqueue);
DEFINE_PUBLIC_ALIAS(psignal,libc_psignal);
DEFINE_PUBLIC_ALIAS(psiginfo,libc_psiginfo);
DEFINE_PUBLIC_ALIAS(sigstack,libc_sigstack);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(__DSYM(signal),libc_dos_signal);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
DEFINE_PUBLIC_ALIAS(__sysv_signal,libc_sysv_signal);
DEFINE_PUBLIC_ALIAS(signal,libc_bsd_signal);
DEFINE_PUBLIC_ALIAS(__sigismember,libc_sigismember);
DEFINE_PUBLIC_ALIAS(__sigaddset,libc_sigaddset);
DEFINE_PUBLIC_ALIAS(__sigdelset,libc_sigdelset);

/* We do just as linux and alias these to raise/signal,
 * even though doing so isn't 100% correct. */
DEFINE_PUBLIC_ALIAS(gsignal,libc_raise);
DEFINE_PUBLIC_ALIAS(ssignal,libc_bsd_signal);

DECL_END

#endif /* !GUARD_LIBS_LIBC_SIGNAL_C */
