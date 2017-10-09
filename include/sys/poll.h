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
#ifndef _SYS_POLL_H
#define _SYS_POLL_H 1

#include <features.h>
#include <bits/poll.h>
#ifdef __USE_GNU
#include <bits/sigset.h>
#include <hybrid/timespec.h>
#endif /* __USE_GNU */

__SYSDECL_BEGIN

typedef unsigned long int nfds_t;

struct pollfd {
 int       fd;      /*< File descriptor to poll.  */
 short int events;  /*< Types of events poller cares about.  */
 short int revents; /*< Types of events that actually occurred.  */
};

#ifndef __KERNEL__
__LIBC int (__LIBCCALL poll)(struct pollfd *__fds, nfds_t __nfds, int __timeout);
#ifdef __USE_GNU
__LIBC int (__LIBCCALL ppoll)(struct pollfd *__fds, nfds_t __nfds, struct timespec const *__timeout, __sigset_t const *__ss) __TM_FUNC(ppoll);
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL ppoll64)(struct pollfd *__fds, nfds_t __nfds, struct __timespec64 const *__timeout, __sigset_t const *__ss);
#endif /* __USE_TIME64 */
#endif /* __USE_GNU */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_SYS_POLL_H */
