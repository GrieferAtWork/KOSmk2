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
#include "termios.h"
#include "errno.h"
#include "unistd.h"
#include "fcntl.h"
#include "stdlib.h"
#include "dirent.h"
#include "string.h"

#include <sys/ioctl.h>
#include <bits/stat.h>
#include <sys/stat.h>


DECL_BEGIN

INTERN int LIBCCALL libc_login_tty(int fd) {
 libc_setsid();
 if (libc_ioctl(fd,TIOCSCTTY,(char *)NULL) == -1)
     return -1;
 while (libc_dup2(fd,0) == -1 && GET_ERRNO() == EBUSY);
 while (libc_dup2(fd,1) == -1 && GET_ERRNO() == EBUSY);
 while (libc_dup2(fd,2) == -1 && GET_ERRNO() == EBUSY);
 if (fd > 2) libc_close(fd);
 return 0;
}

INTERN int LIBCCALL
libc_openpty(int *amaster, int *aslave, char *name,
             struct termios const *termp,
             struct winsize const *winp) {
#if __SIZEOF_INT__ > 4
#error FIXME
#endif
 s64 result = sys_xopenpty(name,termp,winp);
 if (E_ISERR(result)) { SET_ERRNO(-(errno_t)result); return -1; }
 *amaster = (int)(result);
 *aslave  = (int)(result >> 32);
 return 0;
}

INTERN int LIBCCALL
libc_forkpty(int *amaster, char *name,
             struct termios const *termp,
             struct winsize const *winp) {
 int master,slave,pid;
 if (libc_openpty(&master,&slave,name,termp,winp) == -1)
     return -1;
 switch (pid = libc_fork()) {
 case -1:
  libc_close(master);
  libc_close(slave);
  return -1;
 case 0:
  /* Child process. */
  libc_close(master);
  if (libc_login_tty(slave))
      libc__exit(1);
  return 0;
 default:
  /* Parent process.  */
  *amaster = master;
  libc_close(slave);
  return pid;
 }
}

PRIVATE char ttyname_buffer[32];
INTERN char *LIBCCALL libc_ttyname(int fd) {
 return libc_ttyname_r(fd,ttyname_buffer,sizeof(ttyname_buffer)) ? NULL : ttyname_buffer;
}

PRIVATE char const dev[] = "/dev";
INTERN int LIBCCALL libc_ttyname_r(int fd, char *buf, size_t buflen) {
 struct stat64 st; struct dirent *d; DIR *dirstream;
 int safe; dev_t rdev;
 if unlikely(buflen < (COMPILER_STRLEN(dev)+1)*sizeof(char)) { SET_ERRNO(ERANGE); return ERANGE; }
 if unlikely(!libc_isatty(fd)) { SET_ERRNO(ENOTTY); return ENOTTY; }
 if unlikely(libc_kfstat64(fd,&st) < 0) return GET_ERRNO();
 if ((dirstream = libc_opendir(dev)) == NULL) return GET_ERRNO();
 libc_memcpy(buf,dev,COMPILER_STRLEN(dev)*sizeof(char));
 buf[COMPILER_STRLEN(dev)] = '/';
 buflen -= (COMPILER_STRLEN(dev)+1)*sizeof(char);
 safe = GET_ERRNO();
 rdev = st.st_rdev;
 while ((d = libc_readdir(dirstream)) != NULL) {
  if (d->d_ino64 == st.st_ino64 &&
      libc_strcmp(d->d_name,"stdin") == 0 &&
      libc_strcmp(d->d_name,"stdout") == 0 &&
      libc_strcmp(d->d_name,"stderr") == 0) {
   size_t needed = _D_EXACT_NAMLEN(d)+1;
   if (needed > buflen) {
    libc_closedir(dirstream);
    SET_ERRNO(ERANGE);
    return ERANGE;
   }
   libc_memcpy(&buf[sizeof(dev)],d->d_name,(needed+1)*sizeof(char));
   if (libc_kstat64(buf,&st) == 0 && S_ISCHR(st.st_mode) && st.st_rdev == rdev) {
    /* Found it! */
    libc_closedir(dirstream);
    SET_ERRNO(safe);
    return 0;
   }
  }
 }
 libc_closedir(dirstream);
 SET_ERRNO(safe);
 return ENOTTY;
}

INTERN int LIBCCALL libc_isatty(int fd) {
 struct termios term;
 return libc_tcgetattr(fd,&term) == 0;
}
INTERN pid_t LIBCCALL libc_tcgetpgrp(int fd) {
 pid_t pgrp;
 return libc_ioctl(fd,TIOCGPGRP,&pgrp) < 0 ? -1 : pgrp;
}
INTERN int LIBCCALL libc_tcsetpgrp(int fd, pid_t pgrp_id) { return libc_ioctl(fd,TIOCSPGRP,&pgrp_id); }
INTERN speed_t LIBCCALL libc_cfgetospeed(struct termios const *termios_p) { return termios_p->c_ospeed; }
INTERN speed_t LIBCCALL libc_cfgetispeed(struct termios const *termios_p) { return termios_p->c_ispeed; }
INTERN int LIBCCALL libc_cfsetospeed(struct termios *termios_p, speed_t speed) { termios_p->c_ospeed = speed; return 0; }
INTERN int LIBCCALL libc_cfsetispeed(struct termios *termios_p, speed_t speed) { termios_p->c_ispeed = speed; return 0; }
INTERN int LIBCCALL libc_cfsetspeed(struct termios *termios_p, speed_t speed) { termios_p->c_ospeed = termios_p->c_ispeed = speed; return 0; }
INTERN int LIBCCALL libc_tcgetattr(int fd, struct termios *termios_p) { return libc_ioctl(fd,TCGETS,termios_p); }


PRIVATE int const action[] = {
    [TCSANOW]   = TCSETS,
    [TCSADRAIN] = TCSETSW,
    [TCSAFLUSH] = TCSETSF,
};
INTERN int LIBCCALL libc_tcsetattr(int fd, int optional_actions, struct termios const *termios_p) {
 if ((unsigned int)optional_actions >= COMPILER_LENOF(action)) { SET_ERRNO(EINVAL); return -1; }
 return libc_ioctl(fd,action[optional_actions],termios_p);
}
INTERN int LIBCCALL libc_tcsendbreak(int fd, int duration) {
 if (duration <= 0) return libc_ioctl(fd,TCSBRK,0);
 return libc_ioctl(fd,TCSBRKP,(duration+99)/100);
}
INTERN int LIBCCALL libc_tcdrain(int fd) { return libc_ioctl(fd,TCSBRK,1); }
INTERN int LIBCCALL libc_tcflush(int fd, int queue_selector) { return libc_ioctl(fd,TCFLSH,queue_selector); }
INTERN int LIBCCALL libc_tcflow(int fd, int action) { return libc_ioctl(fd,TCXONC,action); }
INTERN pid_t LIBCCALL libc_tcgetsid(int fd) {
 pid_t pgrp;
 pid_t sid;
#ifdef TIOCGSID
 static int tiocgsid_does_not_work = 0;
 if (!tiocgsid_does_not_work) {
  int serrno = GET_ERRNO(),sid;
  if (libc_ioctl(fd,TIOCGSID,&sid) < 0) {
   if (GET_ERRNO() == EINVAL) {
    tiocgsid_does_not_work = 1;
    SET_ERRNO(serrno);
   } else {
    return (pid_t)-1;
   }
  } else {
   return (pid_t)sid;
  }
 }
#endif
 pgrp = libc_tcgetpgrp(fd);
 if (pgrp == -1) return (pid_t)-1;
 sid = libc_getsid(pgrp);
 if (sid == -1 && GET_ERRNO() == ESRCH)
     SET_ERRNO(ENOTTY);
 return sid;
}
INTERN void LIBCCALL libc_cfmakeraw(struct termios *termios_p) {
 termios_p->c_iflag    &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
 termios_p->c_oflag    &= ~(OPOST);
 termios_p->c_lflag    &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
 termios_p->c_cflag    &= ~(CSIZE|PARENB);
 termios_p->c_cflag    |= CS8;
 termios_p->c_cc[VMIN]  = 1; /* Read returns when one byte was read. */
 termios_p->c_cc[VTIME] = 0;
}

DEFINE_PUBLIC_ALIAS(login_tty,libc_login_tty);
DEFINE_PUBLIC_ALIAS(openpty,libc_openpty);
DEFINE_PUBLIC_ALIAS(forkpty,libc_forkpty);
DEFINE_PUBLIC_ALIAS(ttyname,libc_ttyname);
DEFINE_PUBLIC_ALIAS(ttyname_r,libc_ttyname_r);
DEFINE_PUBLIC_ALIAS(isatty,libc_isatty);
DEFINE_PUBLIC_ALIAS(tcgetpgrp,libc_tcgetpgrp);
DEFINE_PUBLIC_ALIAS(tcsetpgrp,libc_tcsetpgrp);
DEFINE_PUBLIC_ALIAS(cfgetospeed,libc_cfgetospeed);
DEFINE_PUBLIC_ALIAS(cfgetispeed,libc_cfgetispeed);
DEFINE_PUBLIC_ALIAS(cfsetospeed,libc_cfsetospeed);
DEFINE_PUBLIC_ALIAS(cfsetispeed,libc_cfsetispeed);
DEFINE_PUBLIC_ALIAS(cfsetspeed,libc_cfsetspeed);
DEFINE_PUBLIC_ALIAS(tcgetattr,libc_tcgetattr);
DEFINE_PUBLIC_ALIAS(tcsetattr,libc_tcsetattr);
DEFINE_PUBLIC_ALIAS(tcsendbreak,libc_tcsendbreak);
DEFINE_PUBLIC_ALIAS(tcdrain,libc_tcdrain);
DEFINE_PUBLIC_ALIAS(tcflush,libc_tcflush);
DEFINE_PUBLIC_ALIAS(tcflow,libc_tcflow);
DEFINE_PUBLIC_ALIAS(tcgetsid,libc_tcgetsid);
DEFINE_PUBLIC_ALIAS(cfmakeraw,libc_cfmakeraw);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(_isatty,libc_isatty);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_TERMIOS_C */
