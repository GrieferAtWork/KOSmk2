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
#ifndef GUARD_LIBS_LIBC_TERMIOS_H
#define GUARD_LIBS_LIBC_TERMIOS_H 1

#include "libc.h"
#include "system.h"
#include <hybrid/compiler.h>
#include <bits/termios.h>

DECL_BEGIN

struct termios;

INTDEF int LIBCCALL libc_login_tty(int fd);
INTDEF int LIBCCALL libc_openpty(int *amaster, int *aslave, char *name, struct termios const *termp, struct winsize const *winp);
INTDEF int LIBCCALL libc_forkpty(int *amaster, char *name, struct termios const *termp, struct winsize const *winp);
INTDEF char *LIBCCALL libc_ttyname(int fd);
INTDEF int LIBCCALL libc_ttyname_r(int fd, char *buf, size_t buflen);
INTDEF int LIBCCALL libc_isatty(int fd);
INTDEF pid_t LIBCCALL libc_tcgetpgrp(int fd);
INTDEF int LIBCCALL libc_tcsetpgrp(int fd, pid_t pgrp_id);
INTDEF speed_t LIBCCALL libc_cfgetospeed(struct termios const *termios_p);
INTDEF speed_t LIBCCALL libc_cfgetispeed(struct termios const *termios_p);
INTDEF int LIBCCALL libc_cfsetospeed(struct termios *termios_p, speed_t speed);
INTDEF int LIBCCALL libc_cfsetispeed(struct termios *termios_p, speed_t speed);
INTDEF int LIBCCALL libc_cfsetspeed(struct termios *termios_p, speed_t speed);
INTDEF int LIBCCALL libc_tcgetattr(int fd, struct termios *termios_p);
INTDEF int LIBCCALL libc_tcsetattr(int fd, int optional_actions, struct termios const *termios_p);
INTDEF int LIBCCALL libc_tcsendbreak(int fd, int duration);
INTDEF int LIBCCALL libc_tcdrain(int fd);
INTDEF int LIBCCALL libc_tcflush(int fd, int queue_selector);
INTDEF int LIBCCALL libc_tcflow(int fd, int action);
INTDEF pid_t LIBCCALL libc_tcgetsid(int fd);
INTDEF void LIBCCALL libc_cfmakeraw(struct termios *termios_p);

DECL_END

#endif /* !GUARD_LIBS_LIBC_TERMIOS_H */
