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
#ifndef GUARD_PLAYGROUND_C
#define GUARD_PLAYGROUND_C 1
#define _GNU_SOURCE 1


#include <sched.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <syslog.h>
#include <errno.h>


static int shared_variable = 0;

int thread_function(void *arg) {
 printf("thread_function: gettid() == %d\n",syscall(SYS_gettid));
 printf("arg = %p\n",arg);
 shared_variable = 199;
 return 42;
}


int main(int argc, char *argv[]) {
 pid_t child,waitno; int status;
 printf("shared_variable = %d\n",shared_variable);
 printf("main: gettid() == %d\n",syscall(SYS_gettid));
 child = clone(&thread_function,CLONE_CHILDSTACK_AUTO,
               CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD,
              (void *)0x12345678);
 if (child < 0) err(1,"clone() failed");
 printf("clone() returned '%d'\n",child);
 while ((waitno = wait(&status)) == -1 && errno == EINTR);
 if (waitno < 0) err(1,"wait() failed");
 printf("wait() returned %d\n",waitno);
 printf("status = %d (%d)\n",status,WEXITSTATUS(status));
 printf("shared_variable = %d\n",shared_variable);
 return 0;
}

#endif /* !GUARD_PLAYGROUND_C */
