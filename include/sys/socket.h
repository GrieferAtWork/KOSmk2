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
#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H 1

/* Declarations of socket constants, types, and functions.
   Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>
#include <hybrid/timespec.h>
#include <sys/uio.h>
#include <bits/socket.h>
#ifdef __USE_GNU
#include <bits/sigset.h>
#endif /* __USE_GNU */

__DECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif

#ifdef __USE_MISC
struct osockaddr {
 unsigned short int sa_family;
 unsigned char      sa_data[14];
};
#endif /* __USE_MISC */

enum {
  SHUT_RD = 0,     /*< No more receptions. */
  SHUT_WR,         /*< No more transmissions. */
  SHUT_RDWR        /*< No more receptions or transmissions. */
#define SHUT_RD    SHUT_RD
#define SHUT_WR    SHUT_WR
#define SHUT_RDWR  SHUT_RDWR
};

#if defined(__cplusplus) || !__GCC_VERSION(2,7,0) || !defined(__USE_GNU)
#   define __SOCKADDR_ARG       struct sockaddr *__restrict
#   define __CONST_SOCKADDR_ARG const struct sockaddr *
#else
# define __SOCKADDR_ALLTYPES \
  __SOCKADDR_ONETYPE(sockaddr) \
  __SOCKADDR_ONETYPE(sockaddr_at) \
  __SOCKADDR_ONETYPE(sockaddr_ax25) \
  __SOCKADDR_ONETYPE(sockaddr_dl) \
  __SOCKADDR_ONETYPE(sockaddr_eon) \
  __SOCKADDR_ONETYPE(sockaddr_in) \
  __SOCKADDR_ONETYPE(sockaddr_in6) \
  __SOCKADDR_ONETYPE(sockaddr_inarp) \
  __SOCKADDR_ONETYPE(sockaddr_ipx) \
  __SOCKADDR_ONETYPE(sockaddr_iso) \
  __SOCKADDR_ONETYPE(sockaddr_ns) \
  __SOCKADDR_ONETYPE(sockaddr_un) \
  __SOCKADDR_ONETYPE(sockaddr_x25)
#define __SOCKADDR_ONETYPE(type) struct type *__restrict __##type##__;
typedef union { __SOCKADDR_ALLTYPES } __SOCKADDR_ARG __attribute__ ((__transparent_union__));
#undef __SOCKADDR_ONETYPE
#define __SOCKADDR_ONETYPE(type) const struct type *__restrict __##type##__;
typedef union { __SOCKADDR_ALLTYPES } __CONST_SOCKADDR_ARG __attribute__ ((__transparent_union__));
#undef __SOCKADDR_ONETYPE
#endif

#ifdef __USE_GNU
/* For `recvmmsg' and `sendmmsg'. */
struct mmsghdr {
 struct msghdr msg_hdr; /*< Actual message header. */
 unsigned int  msg_len; /*< Number of received or sent bytes for the entry. */
};
#endif

#ifndef __KERNEL__
__LIBC int (__LIBCCALL socket)(int __domain, int __type, int __protocol);
__LIBC int (__LIBCCALL socketpair)(int __domain, int __type, int __protocol, int __fds[2]);
__LIBC int (__LIBCCALL bind)(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
__LIBC int (__LIBCCALL getsockname)(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __len);
__LIBC int (__LIBCCALL connect)(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
__LIBC int (__LIBCCALL getpeername)(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __len);
__LIBC ssize_t (__LIBCCALL send)(int __fd, void const *__buf, size_t __n, int __flags);
__LIBC ssize_t (__LIBCCALL recv)(int __fd, void *__buf, size_t __n, int __flags);
__LIBC ssize_t (__LIBCCALL sendto)(int __fd, void const *__buf, size_t __n, int __flags, __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len);
__LIBC ssize_t (__LIBCCALL recvfrom)(int __fd, void *__restrict __buf, size_t __n, int __flags, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len);
__LIBC ssize_t (__LIBCCALL sendmsg)(int __fd, const struct msghdr *__message, int __flags);
__LIBC ssize_t (__LIBCCALL recvmsg)(int __fd, struct msghdr *__message, int __flags);
__LIBC int (__LIBCCALL getsockopt)(int __fd, int __level, int __optname, void *__restrict __optval, socklen_t *__restrict __optlen);
__LIBC int (__LIBCCALL setsockopt)(int __fd, int __level, int __optname, void const *__optval, socklen_t __optlen);
__LIBC int (__LIBCCALL listen)(int __fd, int __n);
__LIBC int (__LIBCCALL accept)(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len);
__LIBC int (__LIBCCALL shutdown)(int __fd, int __how);
#ifdef __USE_GNU
__LIBC int (__LIBCCALL sendmmsg)(int __fd, struct mmsghdr *__vmessages, unsigned int __vlen, int __flags);
__LIBC int (__LIBCCALL recvmmsg)(int __fd, struct mmsghdr *__vmessages, unsigned int __vlen, int __flags, struct timespec *__tmo) __TM_FUNC(recvmmsg);
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL recvmmsg64)(int __fd, struct mmsghdr *__vmessages, unsigned int __vlen, int __flags, struct __timespec64 *__tmo);
#endif /* __USE_TIME64 */
__LIBC int (__LIBCCALL accept4)(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len, int __flags);
#endif /* __USE_GNU */
#ifdef __USE_XOPEN2K
__LIBC int (__LIBCCALL sockatmark)(int __fd);
#endif /* __USE_XOPEN2K */
#ifdef __USE_MISC
__LIBC int (__LIBCCALL isfdtype)(int __fd, int __fdtype);
#endif /* __USE_MISC */
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_SYS_SOCKET_H */
