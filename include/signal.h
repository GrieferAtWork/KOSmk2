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
#ifndef _SIGNAL_H
#define _SIGNAL_H 1

#include <features.h>
#include <bits/types.h>
#include <bits/signum.h>

#include <bits/sigset.h>
#ifdef __USE_POSIX199309
#include <hybrid/timespec.h>
#endif /* __USE_POSIX199309 */
#if defined(__USE_POSIX199309) || defined(__USE_XOPEN_EXTENDED)
#include <bits/siginfo.h>
#endif /* __USE_POSIX199309 || __USE_XOPEN_EXTENDED */
#ifdef __USE_POSIX
#include <bits/sigaction.h>
#endif /* __USE_POSIX */
#ifdef __USE_MISC
#include <bits/sigcontext.h>
#endif /* __USE_MISC */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#include <bits/sigstack.h>
#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#include <sys/ucontext.h>
#endif /* __USE_XOPEN || __USE_XOPEN2K8 */
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#if defined(__USE_POSIX199506) || defined(__USE_UNIX98)
#include <bits/pthreadtypes.h>
#include <bits/sigthread.h>
#endif /* __USE_POSIX199506 || __USE_UNIX98 */

__SYSDECL_BEGIN

/* An integral type that can be modified atomically, without the
   possibility of a signal arriving in the middle of the operation.  */

__NAMESPACE_STD_BEGIN
typedef __sig_atomic_t sig_atomic_t;
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(sig_atomic_t)
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __CRT_GLC
#ifndef __sigset_t_defined
#define __sigset_t_defined 1
typedef __sigset_t sigset_t;
#endif
#endif /* __CRT_GLC */

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K)
#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif /* !__pid_t_defined */
#ifndef __uid_t_defined
#define __uid_t_defined 1
typedef __uid_t uid_t;
#endif /* !__uid_t_defined */
#endif /* __USE_XOPEN || __USE_XOPEN2K */

#ifdef __USE_GNU
#ifndef __sighandler_t_defined
#define __sighandler_t_defined 1
typedef __sighandler_t sighandler_t;
#endif /* !__sighandler_t_defined */
#endif /* __USE_GNU */
#ifdef __USE_MISC
#define NSIG    _NSIG
typedef __sighandler_t sig_t;
#endif /* __USE_MISC */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#ifndef __size_t_defined
#define __size_t_defined 1
typedef __size_t size_t;
#endif /* !__size_t_defined */
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */


#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC int (__LIBCCALL raise)(int __sig);
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(raise)
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __CRT_GLC
__REDIRECT(__LIBC,,__sighandler_t,__LIBCCALL,__sysv_signal,(int __sig, __sighandler_t __handler),sysv_signal,(__sig,__handler))
#ifdef __USE_GNU
__LIBC __sighandler_t (__LIBCCALL sysv_signal)(int __sig, __sighandler_t __handler);
#endif /* __USE_GNU */
#endif /* __CRT_GLC */

#ifdef __USE_MISC
__NAMESPACE_STD_BEGIN
__REDIRECT_DOS_FUNC(__LIBC,,__sighandler_t,__LIBCCALL,signal,
                   (int __sig, __sighandler_t __handler),signal,(__sig,__handler))
__NAMESPACE_STD_END
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __sighandler_t (__LIBCCALL ssignal)(int __sig, __sighandler_t __handler);
__LIBC __PORT_NODOS int (__LIBCCALL gsignal)(int __sig);
#define sigmask(sig)    __sigmask(sig)
__LIBC __ATTR_DEPRECATED("Using `sigprocmask()' instead") int (__LIBCCALL sigblock)(int __mask);
__LIBC __ATTR_DEPRECATED("Using `sigprocmask()' instead") int (__LIBCCALL sigsetmask)(int __mask);
__LIBC __ATTR_DEPRECATED("Using `sigprocmask()' instead") int (__LIBCCALL siggetmask)(void);
#undef _sys_siglist
#undef sys_siglist
__LIBC __PORT_NODOS char const *const (_sys_siglist)[_NSIG] __ASMNAME("sys_siglist");
__LIBC __PORT_NODOS char const *const (sys_siglist)[_NSIG];
__LIBC __PORT_NODOS __ATTR_NORETURN void (__LIBCCALL sigreturn)(struct sigcontext const *__scp);
#endif /* __CRT_GLC */
#else /* __USE_MISC */
__NAMESPACE_STD_BEGIN
#ifdef __USE_DOS
__REDIRECT_DOS_FUNC(__LIBC,,__sighandler_t,__LIBCCALL,signal,(int __sig, __sighandler_t __handler),signal,(__sig,__handler))
#else /* __USE_DOS */
__REDIRECT(__LIBC,,__sighandler_t,__LIBCCALL,signal,(int __sig, __sighandler_t __handler),sysv_signal,(__sig,__handler))
#endif /* !__USE_DOS */
__NAMESPACE_STD_END
#endif /* !__USE_MISC */
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(signal)
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __CRT_GLC
#ifdef __USE_XOPEN
__LIBC __PORT_NODOS __sighandler_t (__LIBCCALL bsd_signal)(int __sig, __sighandler_t __handler);
__REDIRECT(__LIBC,__PORT_NODOS,int,__LIBCCALL,sigpause,(int __sig),__xpg_sigpause,(__sig))
#endif /* __USE_XOPEN */
#ifdef __USE_POSIX
__LIBC __PORT_NODOS int (__LIBCCALL kill)(__pid_t __pid, int __sig);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sigemptyset)(sigset_t *__set);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sigfillset)(sigset_t *__set);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sigaddset)(sigset_t *__set, int __signo);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sigdelset)(sigset_t *__set, int __signo);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sigismember)(sigset_t const *__set, int __signo);
__LIBC __PORT_NODOS int (__LIBCCALL sigprocmask)(int __how, sigset_t const *__restrict __set, sigset_t *__restrict __oset);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sigsuspend)(sigset_t const *__set);
__LIBC __PORT_NODOS int (__LIBCCALL sigaction)(int __sig, struct sigaction const *__restrict __act, struct sigaction *__restrict __oact);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sigpending)(sigset_t *__set);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL sigwait)(sigset_t const *__restrict __set, int *__restrict __sig);
#ifdef __USE_GNU
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sigisemptyset)(sigset_t const *__set);
__LIBC __PORT_NODOS __NONNULL((1,2,3)) int (__LIBCCALL sigandset)(sigset_t *__set, sigset_t const *__left, sigset_t const *__right);
__LIBC __PORT_NODOS __NONNULL((1,2,3)) int (__LIBCCALL sigorset)(sigset_t *__set, sigset_t const *__left, sigset_t const *__right);
#endif /* __USE_GNU */
#ifdef __USE_POSIX199309
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sigwaitinfo)(sigset_t const *__restrict __set, siginfo_t *__restrict __info);
__REDIRECT_TM_FUNC(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,sigtimedwait,
                  (sigset_t const *__restrict __set, siginfo_t *__restrict __info,
                   struct timespec const *__timeout),sigtimedwait,(__set,__info,__timeout))
__LIBC __PORT_NODOS int (__LIBCCALL sigqueue)(__pid_t __pid, int __sig, union sigval const __val);
#ifdef __USE_TIME64
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL sigtimedwait64)(sigset_t const *__restrict __set, siginfo_t *__restrict __info, struct __timespec64 const *__timeout);
#endif /* __USE_TIME64 */
#endif /* __USE_POSIX199309 */
#endif /* __USE_POSIX */
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC __PORT_NODOS int (__LIBCCALL killpg)(__pid_t __pgrp, int __sig);
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_XOPEN2K8
__LIBC __PORT_NODOS void (__LIBCCALL psignal)(int __sig, char const *__s);
__LIBC __PORT_NODOS void (__LIBCCALL psiginfo)(siginfo_t const *__pinfo, char const *__s);
#endif /* __USE_XOPEN2K8 */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
__LIBC __PORT_NODOS int (__LIBCCALL siginterrupt)(int __sig, int __interrupt);
__LIBC __PORT_NODOS __ATTR_DEPRECATED_ int (__LIBCCALL sigstack)(struct sigstack *__ss, struct sigstack *__oss);
__LIBC __PORT_NODOS int (__LIBCCALL sigaltstack)(struct sigaltstack const *__restrict __ss, struct sigaltstack *__restrict __oss);
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_XOPEN_EXTENDED
__LIBC __PORT_NODOS int (__LIBCCALL sighold)(int __sig);
__LIBC __PORT_NODOS int (__LIBCCALL sigrelse)(int __sig);
__LIBC __PORT_NODOS int (__LIBCCALL sigignore)(int __sig);
__LIBC __PORT_NODOS __sighandler_t (__LIBCCALL sigset)(int __sig, __sighandler_t __disp);
#endif /* __USE_XOPEN_EXTENDED */
__LIBC __PORT_NODOS int (__LIBCCALL __libc_current_sigrtmin)(void);
__LIBC __PORT_NODOS int (__LIBCCALL __libc_current_sigrtmax)(void);
#endif /* __CRT_GLC */

#else /* !__KERNEL__ */
#define sigismember(set,sig) __sigismember(set,sig)
#define sigaddset(set,sig)   __sigaddset(set,sig)
#define sigdelset(set,sig)   __sigdelset(set,sig)
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_SIGNAL_H */
