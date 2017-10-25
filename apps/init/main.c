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
#ifndef GUARD_APPS_INIT_MAIN_C
#define GUARD_APPS_INIT_MAIN_C 1

#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include <bits/types.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kos/environ.h>
#include <kos/thread.h>
#include <sys/syslog.h>
#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>

DECL_BEGIN

PRIVATE void open2(int fd, char const *file, int mode) {
 int f = open(file,mode);
 if (f == -1) perror("open2()");
 if (f != fd) dup2(fd,f),close(f);
}

volatile int did_trigger = 0;

void my_action(int signo, siginfo_t *info, void *ctx) {
 printf("my_action(%d,%p,%p)\n",signo,info,ctx);
 printf("info = %p\n",&info);
 did_trigger = 1;
}


int main(int argc, char **argv) {
 open2(STDIN_FILENO,"/dev/keyboard",O_RDONLY);
 open2(STDOUT_FILENO,"/dev/kmsg",O_WRONLY);
 open2(STDERR_FILENO,"/dev/kmsg",O_WRONLY);

 /* Setup some volatile components of a regular linux filesystem environment. */
 if (mount("proc","/proc","proc",0,NULL) &&
     errno == ENOENT) {
  mkdir("/proc",0777);
  mount("proc","/proc","proc",0,NULL);
 }
 symlink("kmsg","/dev/log");
 symlink("/proc/self/fd/" PP_STR(STDIN_FILENO),"/dev/stdin");
 symlink("/proc/self/fd/" PP_STR(STDOUT_FILENO),"/dev/stdout");
 symlink("/proc/self/fd/" PP_STR(STDERR_FILENO),"/dev/stderr");
 mkdir("/dev/shm",0777);
 //mount("tmpfs","/dev/shm","tmpfs",0,NULL); /* Not really required... */

 /* Mount the secondary disk passed to QEMU (TODO: Remove me) */
 mount("/dev/dos_hdb1","/mnt",NULL,0,NULL);

#if 1
 /* Try to exec() an exe (test the PDB debug parser) */
 if (fork() == 0) {
  execl("/bin/dos_userapp.exe","dos_userapp.exe",NULL);
  _exit(127);
 }
 while (wait(NULL) == -1 && errno == EINTR);
#endif

#if 0
 printf("appenv         = %p\n",appenv);
 printf("appenv->e_self = %p\n",appenv->e_self);
 printf("appenv->e_size = %p\n",appenv->e_size);
 printf("appenv->e_envc = %Iu\n",appenv->e_envc);
 printf("appenv->e_argc = %Iu\n",appenv->e_argc);
 printf("appenv->e_envp = %p\n",appenv->e_envp);
 printf("appenv->e_argv = %p\n",appenv->e_argv);
 printf("appenv->e_root = %p\n",appenv->e_root);

 printf("argc = %d\n",argc);
 printf("argv = %p\n",argv);
 while (argc--) printf("argv[%d] = %p:%q\n",argc,argv[argc],argv[argc]);
 putenv("HOME=bar");
 char **iter = environ;
 while (*iter) printf("ENV: %q\n",*iter++);
 //printf("foo = %q\n",getenv("foo"));
#endif

#if 1
 /* Since mtools can't copy cygwin's symbolic link (because it thinks they're real links),
  * the only place that we can currently create (and handle) those fake links in on my
  * trusty development USB-stick.
  * >> But since the whole of busybox is literally a single binary, we can
  *    execute it in the same matter that a symbolic '/bin/sh' would have done
  *   (By passing 'sh' through the first argument 'argv[0]') */
 execl("/bin/terminal-vga","terminal-vga","/bin/busybox","sh","-i",(char *)NULL);
 error(0,errno,"exec('/bin/terminal-vga') failed");
 /* If the terminal doesn't work for some reason, run directly off of the system log. */
 execl("/bin/busybox","sh","-i",(char *)NULL);
 error(0,errno,"exec('/bin/busybox') failed");
#endif

#if 0
 struct sigaction act;
 memset(&act,0,sizeof(struct sigaction));
 act.sa_flags     = SA_SIGINFO/*|SA_NODEFER*/;
 act.sa_sigaction = &my_action;
 if (sigaction(SIGCHLD,&act,NULL))
     perror("sigaction()");

#if 1
 pid_t child;
 if ((child = fork()) == 0) {
  printf("Child process: %d (%d)\n",getpid(),getppid());
  sched_yield();
  printf("Child process after yield\n");
  kill(getppid(),SIGCHLD);
  kill(getppid(),SIGCHLD);
  kill(getppid(),SIGCHLD);
  kill(getppid(),SIGCHLD);
  printf("Signal send from child\n");
  //sched_yield();
  exit(42);
 } else if (child == -1) {
  perror("fork() failed");
 } else {
  int code;
  printf("Parent process: %d\n",child);
  //while (!did_trigger) /*printf(".")*/;

  printf("Parent process - Begin wait\n");
  while ((child = wait(&code)) == -1 && errno == EINTR);
  if (child == -1) perror("wait() failed");
  else {
   printf("wait returned: %d - %x\n",child,code);
  }
 }
#endif
#endif

 return errno;
}

DECL_END

#endif /* !GUARD_APPS_INIT_MAIN_C */
