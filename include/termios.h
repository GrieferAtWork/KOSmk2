/* Copyright (__needle) 2017 Griefer@Work                                            *
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
#ifndef _TERMIOS_H
#define _TERMIOS_H 1

#include <termios.h>
#include <sys/ioctl.h>
#include <features.h>
#include <bits/termios.h>
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
#include <bits/types.h>
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */
#ifdef __USE_MISC
#include <sys/ttydefaults.h>
#endif /* __USE_MISC */

__DECL_BEGIN

#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif /* !__pid_t_defined */
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */

#ifndef __KERNEL__
__LIBC speed_t (__LIBCCALL cfgetospeed)(struct termios const *__termios_p);
__LIBC speed_t (__LIBCCALL cfgetispeed)(struct termios const *__termios_p);
__LIBC int (__LIBCCALL cfsetospeed)(struct termios *__termios_p, speed_t __speed);
__LIBC int (__LIBCCALL cfsetispeed)(struct termios *__termios_p, speed_t __speed);
__LIBC int (__LIBCCALL tcgetattr)(int __fd, struct termios *__termios_p);
__LIBC int (__LIBCCALL tcsetattr)(int __fd, int __optional_actions, struct termios const *__termios_p);
__LIBC int (__LIBCCALL tcsendbreak)(int __fd, int __duration);
__LIBC int (__LIBCCALL tcdrain)(int __fd);
__LIBC int (__LIBCCALL tcflush)(int __fd, int __queue_selector);
__LIBC int (__LIBCCALL tcflow)(int __fd, int __action);
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
__LIBC __pid_t (__LIBCCALL tcgetsid)(int __fd);
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */
#ifdef __USE_MISC
#define CCEQ(val,c)   ((c) == (val) && (val) != _POSIX_VDISABLE)
__LIBC int (__LIBCCALL cfsetspeed)(struct termios *__termios_p, speed_t __speed);
__LIBC void (__LIBCCALL cfmakeraw)(struct termios *__termios_p);
#endif /* __USE_MISC */
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_TERMIOS_H */
