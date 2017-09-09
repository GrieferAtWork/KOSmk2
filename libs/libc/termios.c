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
#ifndef GUARD_LIBS_LIBC_TERMIOS_C
#define GUARD_LIBS_LIBC_TERMIOS_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include "libc.h"
#include "system.h"
#include <errno.h>
#include <hybrid/compiler.h>
#include <termios.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

DECL_BEGIN

PUBLIC int (LIBCCALL login_tty)(int fd) {
 setsid();
 if (ioctl(fd,TIOCSCTTY,(char *)NULL) == -1)
     return -1;
 while (dup2(fd,0) == -1 && errno == EBUSY);
 while (dup2(fd,1) == -1 && errno == EBUSY);
 while (dup2(fd,2) == -1 && errno == EBUSY);
 if (fd > 2) close(fd);
 return 0;
}

PUBLIC int (LIBCCALL openpty)(int *amaster, int *aslave, char *name,
                              struct termios const *termp,
                              struct winsize const *winp) {
#if __SIZEOF_INT__ > 4
#error FIXME
#endif
 s64 result = sys_xopenpty(name,termp,winp);
 if (E_ISERR(result)) { __set_errno(-(errno_t)result); return -1; }
 *amaster = (int)(result);
 *aslave  = (int)(result >> 32);
 return 0;
}

PUBLIC int (LIBCCALL forkpty)(int *amaster, char *name,
                              struct termios const *termp,
                              struct winsize const *winp) {
 int master,slave,pid;
 if (openpty(&master,&slave,name,termp,winp) == -1)
     return -1;
 switch (pid = fork()) {
 case -1:
  close(master);
  close(slave);
  return -1;
 case 0:
  /* Child process. */
  close(master);
  if (login_tty(slave)) _exit(1);
  return 0;
 default:
  /* Parent process.  */
  *amaster = master;
  close(slave);
  return pid;
 }
}

PRIVATE char ttyname_buffer[32];
PUBLIC char *(LIBCCALL ttyname)(int fd) {
 return ttyname_r(fd,ttyname_buffer,
                  sizeof(ttyname_buffer))
      ? NULL : ttyname_buffer;
}


PRIVATE char const dev[] = "/dev";
PUBLIC int (LIBCCALL ttyname_r)(int fd, char *buf, size_t buflen) {
 struct stat st; struct dirent *d; DIR *dirstream;
 int safe; dev_t rdev;
 if unlikely(buflen < (COMPILER_STRLEN(dev)+1)*sizeof(char)) { __set_errno(ERANGE); return ERANGE; }
 if unlikely(!isatty(fd)) { __set_errno(ENOTTY); return ENOTTY; }
 if unlikely(fstat(fd,&st) < 0) return errno;
 if ((dirstream = opendir(dev)) == NULL) return errno;
 memcpy(buf,dev,COMPILER_STRLEN(dev)*sizeof(char));
 buf[COMPILER_STRLEN(dev)] = '/';
 buflen -= (COMPILER_STRLEN(dev)+1)*sizeof(char);
 safe = errno;
 rdev = st.st_rdev;
 while ((d = readdir(dirstream)) != NULL) {
  if (d->d_ino64 == st.st_ino64 &&
      strcmp(d->d_name,"stdin") == 0 &&
      strcmp(d->d_name,"stdout") == 0 &&
      strcmp(d->d_name,"stderr") == 0) {
   size_t needed = _D_EXACT_NAMLEN(d)+1;
   if (needed > buflen) {
    closedir(dirstream);
    __set_errno(ERANGE);
    return ERANGE;
   }
   memcpy(&buf[sizeof(dev)],d->d_name,(needed+1)*sizeof(char));
   if (stat(buf,&st) == 0 && S_ISCHR(st.st_mode) && st.st_rdev == rdev) {
    /* Found it! */
    closedir(dirstream);
    __set_errno(safe);
    return 0;
   }
  }
 }
 closedir(dirstream);
 __set_errno(safe);
 return ENOTTY;
}

PUBLIC int (LIBCCALL isatty)(int fd) {
 struct termios term;
 return tcgetattr(fd,&term) == 0;
}
PUBLIC pid_t (LIBCCALL tcgetpgrp)(int fd) {
 pid_t pgrp;
 return ioctl(fd,TIOCGPGRP,&pgrp) < 0 ? -1 : pgrp;
}
PUBLIC int (LIBCCALL tcsetpgrp)(int fd, pid_t pgrp_id) { return ioctl(fd,TIOCSPGRP,&pgrp_id); }
PUBLIC speed_t (LIBCCALL cfgetospeed)(struct termios const *termios_p) { return termios_p->c_ospeed; }
PUBLIC speed_t (LIBCCALL cfgetispeed)(struct termios const *termios_p) { return termios_p->c_ispeed; }
PUBLIC int (LIBCCALL cfsetospeed)(struct termios *termios_p, speed_t speed) { termios_p->c_ospeed = speed; return 0; }
PUBLIC int (LIBCCALL cfsetispeed)(struct termios *termios_p, speed_t speed) { termios_p->c_ispeed = speed; return 0; }
PUBLIC int (LIBCCALL cfsetspeed)(struct termios *termios_p, speed_t speed) { termios_p->c_ospeed = termios_p->c_ispeed = speed; return 0; }
PUBLIC int (LIBCCALL tcgetattr)(int fd, struct termios *termios_p) { return ioctl(fd,TCGETS,termios_p); }


PRIVATE int const action[] = {
    [TCSANOW]   = TCSETS,
    [TCSADRAIN] = TCSETSW,
    [TCSAFLUSH] = TCSETSF,
};
PUBLIC int (LIBCCALL tcsetattr)(int fd, int optional_actions, struct termios const *termios_p) {
 if ((unsigned int)optional_actions >= COMPILER_LENOF(action)) { __set_errno(EINVAL); return -1; }
 return ioctl(fd,action[optional_actions],termios_p);
}
PUBLIC int (LIBCCALL tcsendbreak)(int fd, int duration) {
 if (duration <= 0) return ioctl(fd,TCSBRK,0);
 return ioctl(fd,TCSBRKP,(duration+99)/100);
}
PUBLIC int (LIBCCALL tcdrain)(int fd) { return ioctl(fd,TCSBRK,1); }
PUBLIC int (LIBCCALL tcflush)(int fd, int queue_selector) { return ioctl(fd,TCFLSH,queue_selector); }
PUBLIC int (LIBCCALL tcflow)(int fd, int action) { return ioctl(fd,TCXONC,action); }
PUBLIC pid_t (LIBCCALL tcgetsid)(int fd) {
 pid_t pgrp;
 pid_t sid;
#ifdef TIOCGSID
 static int tiocgsid_does_not_work = 0;
 if (!tiocgsid_does_not_work) {
  int serrno = errno,sid;
  if (ioctl(fd,TIOCGSID,&sid) < 0) {
   if (errno == EINVAL) {
    tiocgsid_does_not_work = 1;
    __set_errno(serrno);
   } else return (pid_t)-1;
  } else return (pid_t)sid;
 }
#endif
 pgrp = tcgetpgrp(fd);
 if (pgrp == -1) return (pid_t)-1;
 sid = getsid(pgrp);
 if (sid == -1 && errno == ESRCH)
     __set_errno(ENOTTY);
 return sid;
}
PUBLIC void (LIBCCALL cfmakeraw)(struct termios *termios_p) {
 termios_p->c_iflag    &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
 termios_p->c_oflag    &= ~(OPOST);
 termios_p->c_lflag    &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
 termios_p->c_cflag    &= ~(CSIZE|PARENB);
 termios_p->c_cflag    |= CS8;
 termios_p->c_cc[VMIN]  = 1; /* Read returns when one byte was read. */
 termios_p->c_cc[VTIME] = 0;
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_TERMIOS_C */
