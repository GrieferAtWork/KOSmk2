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
#ifndef GUARD_LIBS_LIBPTHREAD_THREAD_C
#define GUARD_LIBS_LIBPTHREAD_THREAD_C 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <errno.h>
#include <sched.h>
#include <sys/wait.h>

#include "thread.h"
#include "../libc/system.h"

DECL_BEGIN

__asm__(".weak __stack_chk_fail_local\n"
        ".hidden __stack_chk_fail_local\n"
        ".set __stack_chk_fail_local, __stack_chk_fail\n");

INTERN int LIBPCALL
thread_create(pthread_t *__restrict newthread,
              pthread_attr_t const *__restrict attr,
              void *(*start_routine)(void *arg),
              void *__restrict arg) {
 pid_t child;
 if (!newthread || !start_routine) return EINVAL;
 child = clone((int(*)(void *))start_routine,CLONE_CHILDSTACK_AUTO,
                CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD,arg);
 if (child < 0) return __get_errno();
 *newthread = child;
 return 0;
}
INTERN ATTR_NORETURN
void LIBPCALL thread_exit(void *retval) {
 sys_exit((int)(uintptr_t)retval);
}
INTERN int LIBPCALL
thread_join(pthread_t th, void **thread_return) {
 int status; pid_t error;
 error = waitpid((pid_t)th,&status,WEXITED);
 if (error < 0) return __get_errno();
 if (thread_return) *thread_return = (void *)(uintptr_t)WEXITSTATUS(status);
 return 0;
}
INTERN pthread_t LIBPCALL thread_self(void) {
 return (pthread_t)sys_gettid();
}
INTERN int LIBPCALL
thread_equal(pthread_t thread1, pthread_t thread2) {
 return thread1 == thread2;
}

DEFINE_PUBLIC_ALIAS(pthread_create,thread_create);
DEFINE_PUBLIC_ALIAS(pthread_exit,thread_exit);
DEFINE_PUBLIC_ALIAS(pthread_join,thread_join);
DEFINE_PUBLIC_ALIAS(pthread_self,thread_self);
DEFINE_PUBLIC_ALIAS(pthread_equal,thread_equal);

DECL_END

#endif /* !GUARD_LIBS_LIBPTHREAD_THREAD_C */
