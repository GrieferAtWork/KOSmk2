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
#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>
#include <bits/select.h>
#include <bits/sigset.h>
#include <hybrid/timespec.h> /* struct timespec */
#include <bits/time.h>       /* struct timeval */

__SYSDECL_BEGIN

#ifndef __time_t_defined
#define __time_t_defined  1
typedef __TM_TYPE(time) time_t;
#endif /* !__time_t_defined */

#ifndef __sigset_t_defined
#define __sigset_t_defined 1
typedef __sigset_t sigset_t;
#endif

#ifndef __suseconds_t_defined
#define __suseconds_t_defined 1
typedef __suseconds_t suseconds_t;
#endif

typedef __intptr_t __fd_mask;

#undef  __NFDBITS
#define __NFDBITS    (8*(int)sizeof(__fd_mask))
#define __FD_ELT(d)  ((d)/__NFDBITS)
#define __FD_MASK(d) ((__fd_mask)(1UL<<((d)%__NFDBITS)))

typedef struct {
#ifdef __USE_XOPEN
    __fd_mask fds_bits[__FD_SETSIZE/__NFDBITS];
#define __FDS_BITS(set) ((set)->fds_bits)
#else
    __fd_mask __fds_bits[__FD_SETSIZE/__NFDBITS];
#define __FDS_BITS(set) ((set)->__fds_bits)
#endif
} fd_set;
#define FD_SETSIZE __FD_SETSIZE

#ifdef __USE_MISC
typedef __fd_mask fd_mask;
#define NFDBITS __NFDBITS
#endif

#define FD_SET(fd,fdsetp)   __FD_SET(fd,fdsetp)
#define FD_CLR(fd,fdsetp)   __FD_CLR(fd,fdsetp)
#define FD_ISSET(fd,fdsetp) __FD_ISSET(fd,fdsetp)
#define FD_ZERO(fdsetp)     __FD_ZERO(fdsetp)

#ifndef __KERNEL__
__LIBC int (__LIBCCALL select)(int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds,
                               fd_set *__restrict __exceptfds, struct timeval *__restrict __timeout) __TM_FUNC(select);
#ifdef __USE_XOPEN2K
__LIBC int (__LIBCCALL pselect)(int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds,
                                fd_set *__restrict __exceptfds, struct timespec const *__restrict __timeout,
                                __sigset_t const *__restrict __sigmask) __TM_FUNC(pselect);
#endif /* __USE_XOPEN2K */
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL select64)(int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds,
                                 fd_set *__restrict __exceptfds, struct timeval64 *__restrict __timeout);
#ifdef __USE_XOPEN2K
__LIBC int (__LIBCCALL pselect64)(int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds,
                                  fd_set *__restrict __exceptfds, struct __timespec64 const *__restrict __timeout,
                                  __sigset_t const *__restrict __sigmask);
#endif /* __USE_XOPEN2K */
#endif /* __USE_TIME64 */
#endif /* !__KERNEL__ */

__SYSDECL_END


#endif /* !_SYS_SELECT_H */
