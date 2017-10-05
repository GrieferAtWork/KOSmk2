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
#ifndef GUARD_LIBS_LIBC_SCHED_C
#define GUARD_LIBS_LIBC_SCHED_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include "libc.h"
#include "system.h"
#include "sched.h"
#include "stdlib.h"

#include <errno.h>
#include <hybrid/compiler.h>
#include <sched.h>
#include <sys/syscall.h>

DECL_BEGIN

INTERN int LIBCCALL libc_unshare(int flags) {
 int result = sys_unshare(flags);
 if (E_ISERR(result)) { SET_ERRNO(-result); return -1; }
 return 0;
}
INTERN pid_t LIBCCALL
libc_clone(int (LIBCCALL *fn)(void *arg),
           void *child_stack, int flags,
           void *arg, ...) {
 void *newtls = NULL; va_list args;
 pid_t result,*ptid = NULL,*ctid = NULL;
 va_start(args,arg);
 if (flags&CLONE_PARENT_SETTID) ptid = va_arg(args,pid_t *);
 if (flags&CLONE_SETTLS) newtls = va_arg(args,void *);
 if (flags&CLONE_CHILD_SETTID) ctid = va_arg(args,pid_t *);
 va_end(args);
 (void)newtls; /* Not used by the kernel, and the argument slot is used to pass 'arg'. */

 /* NOTE: The clone() system call cannot be called safely without custom assembly,
  *       due to the fact that it returns to the same address as the parent thread. */
 __asm__ __volatile__("    pushl %%ebp\n"
                      "    movl  %[arg], %%ebp\n" /* Safe the argument in EBP */
                      "    int   $0x80\n"
                      /* This is where (presumably) two threads are going to show up,
                       * the only difference between the two being the value of EAX,
                       * which is supposed to be ZERO(0) for the child. */
                      "    testl %%eax, %%eax\n"
                      "    jnz 1f\n" /* Jump if this is the parent thread. */
                      /* Userspace child initialization. */
                      "    pushl %%ebp\n" /* 'arg' */
                      "    xorl  %%ebp, %%ebp\n" /* ZERO out EBP to terminate tracebacks. */
                      "    call *%[fn]\n" /* Call the user-defined function.  */
                      "    movl  %%eax, %%ebx\n" /* Load the thread's exitcode. */
                      /* NOTE: Don't be confused, the kernel calls 'pthread_exit()'
                       *       'exit()', and calls stdlib's 'exit()' 'exit_group()'.
                       *       The reason for this is historical and lies in the fact
                       *       that multithreading wasn't something you could always do.
                       *      (fork() came before clone(), and exit_group() was an afterthough). */
                      "    movl  $(" PP_STR(SYS_exit) "), %%eax\n"
                      "    int   $0x80\n" /* Exit the thread. */
                      /* unreachable */
                      "1:  popl  %%ebp\n" /* This is where the parent jumps */
                      : "=a" (result)
                      : "a" (SYS_clone)
                      , "b" (flags)
                      , "c" (child_stack)
                      , "d" (ptid)
                      , "S" (ctid)
#if 1
                      , [fn] "D" (fn)
                      , [arg] "m" (arg)
#else
                      , "D" (newtls)
                      , [stp] "m" (stp)
#endif
                      : "memory");
 /* Check for errors. */
 if (E_ISERR(result)) { SET_ERRNO(-result); return -1; }
 /* Return the child thread PID. */
 return result;
}
INTERN int LIBCCALL libc_sched_yield(void) {
 /*__asm__("int $3\n");*/
 return sys_sched_yield();
}
INTERN int LIBCCALL libc_sched_getcpu(void) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setns(int fd, int nstype) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sched_setparam(pid_t pid, struct sched_param const *param) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sched_getparam(pid_t pid, struct sched_param *param) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sched_setscheduler(pid_t pid, int policy, struct sched_param const *param) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sched_getscheduler(pid_t pid) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sched_get_priority_max(int algorithm) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sched_get_priority_min(int algorithm) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sched_rr_get_interval(pid_t pid, struct timespec *t) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sched_setaffinity(pid_t pid, size_t cpusetsize, cpu_set_t const *cpuset) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *cpuset) { NOT_IMPLEMENTED(); return -1; }

#undef sched_yield
DEFINE_PUBLIC_ALIAS(unshare,libc_unshare);
DEFINE_PUBLIC_ALIAS(clone,libc_clone);
DEFINE_PUBLIC_ALIAS(sched_yield,libc_sched_yield);
DEFINE_PUBLIC_ALIAS(sched_getcpu,libc_sched_getcpu);
DEFINE_PUBLIC_ALIAS(setns,libc_setns);
DEFINE_PUBLIC_ALIAS(sched_setparam,libc_sched_setparam);
DEFINE_PUBLIC_ALIAS(sched_getparam,libc_sched_getparam);
DEFINE_PUBLIC_ALIAS(sched_setscheduler,libc_sched_setscheduler);
DEFINE_PUBLIC_ALIAS(sched_getscheduler,libc_sched_getscheduler);
DEFINE_PUBLIC_ALIAS(sched_get_priority_max,libc_sched_get_priority_max);
DEFINE_PUBLIC_ALIAS(sched_get_priority_min,libc_sched_get_priority_min);
DEFINE_PUBLIC_ALIAS(sched_rr_get_interval,libc_sched_rr_get_interval);
DEFINE_PUBLIC_ALIAS(sched_setaffinity,libc_sched_setaffinity);
DEFINE_PUBLIC_ALIAS(sched_getaffinity,libc_sched_getaffinity);

DECL_END

#endif /* !GUARD_LIBS_LIBC_SCHED_C */
