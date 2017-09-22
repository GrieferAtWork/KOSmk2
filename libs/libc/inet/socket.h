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
#ifndef GUARD_LIBS_LIBC_INET_SOCKET_H
#define GUARD_LIBS_LIBC_INET_SOCKET_H 1

#include "../libc.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <sys/socket.h>

DECL_BEGIN

INTDEF u32 LIBCCALL libc_ntohl(u32 netlong);
INTDEF u16 LIBCCALL libc_ntohs(u16 netshort);
INTDEF u32 LIBCCALL libc_htonl(u32 netlong);
INTDEF u16 LIBCCALL libc_htons(u16 netshort);
INTDEF int LIBCCALL libc_socket(int domain, int type, int protocol);
INTDEF int LIBCCALL libc_socketpair(int domain, int type, int protocol, int fds[2]);
INTDEF int LIBCCALL libc_bind(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len);
INTDEF int LIBCCALL libc_getsockname(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict len);
INTDEF int LIBCCALL libc_connect(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len);
INTDEF int LIBCCALL libc_getpeername(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict len);
INTDEF ssize_t LIBCCALL libc_send(int fd, void const *buf, size_t n, int flags);
INTDEF ssize_t LIBCCALL libc_recv(int fd, void *buf, size_t n, int flags);
INTDEF ssize_t LIBCCALL libc_sendto(int fd, void const *buf, size_t n, int flags, __CONST_SOCKADDR_ARG addr, socklen_t addr_len);
INTDEF ssize_t LIBCCALL libc_recvfrom(int fd, void *__restrict buf, size_t n, int flags, __SOCKADDR_ARG addr, socklen_t *__restrict addr_len);
INTDEF ssize_t LIBCCALL libc_sendmsg(int fd, const struct msghdr *message, int flags);
INTDEF ssize_t LIBCCALL libc_recvmsg(int fd, struct msghdr *message, int flags);
INTDEF int LIBCCALL libc_getsockopt(int fd, int level, int optname, void *__restrict optval, socklen_t *__restrict optlen);
INTDEF int LIBCCALL libc_setsockopt(int fd, int level, int optname, void const *optval, socklen_t optlen);
INTDEF int LIBCCALL libc_listen(int fd, int n);
INTDEF int LIBCCALL libc_accept(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict addr_len);
INTDEF int LIBCCALL libc_shutdown(int fd, int how);
INTDEF int LIBCCALL libc_sendmmsg(int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags);
INTDEF int LIBCCALL libc_recvmmsg(int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags, struct timespec *tmo);
INTDEF int LIBCCALL libc_accept4(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict addr_len, int flags);
INTDEF int LIBCCALL libc_sockatmark(int fd);
INTDEF int LIBCCALL libc_isfdtype(int fd, int fdtype);

DECL_END

#endif /* !GUARD_LIBS_LIBC_INET_SOCKET_H */
