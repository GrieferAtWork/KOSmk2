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
#ifndef GUARD_LIBS_LIBC_INET_SOCKET_C
#define GUARD_LIBS_LIBC_INET_SOCKET_C 1
#define _GNU_SOURCE 1

#include "../libc.h"
#include "socket.h"
#include <hybrid/compiler.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <hybrid/timespec.h>
#include <net/if.h>

DECL_BEGIN

#if __BYTE_ORDER == __BIG_ENDIAN
INTERN uint32_t LIBCCALL libc_ntohl(uint32_t netlong) { return netlong; }
INTERN uint16_t LIBCCALL libc_ntohs(uint16_t netshort) { return netshort; }
DEFINE_INTERN_ALIAS(libc_htonl,libc_ntohl);
DEFINE_INTERN_ALIAS(libc_htons,libc_ntohs);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
INTERN uint32_t LIBCCALL libc_ntohl(uint32_t netlong) { return __bswap_32(netlong); }
INTERN uint16_t LIBCCALL libc_ntohs(uint16_t netshort) { return __bswap_16(netshort); }
DEFINE_INTERN_ALIAS(libc_htonl,libc_ntohl);
DEFINE_INTERN_ALIAS(libc_htons,libc_ntohs);
#else
#error FIXME
#endif
INTERN int LIBCCALL libc_socket(int domain, int type, int protocol) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_socketpair(int domain, int type, int protocol, int fds[2]) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_bind(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getsockname(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_connect(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getpeername(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict len) { NOT_IMPLEMENTED(); return -1; }
INTERN ssize_t LIBCCALL libc_send(int fd, void const *buf, size_t n, int flags) { NOT_IMPLEMENTED(); return -1; }
INTERN ssize_t LIBCCALL libc_recv(int fd, void *buf, size_t n, int flags) { NOT_IMPLEMENTED(); return -1; }
INTERN ssize_t LIBCCALL libc_sendto(int fd, void const *buf, size_t n, int flags, __CONST_SOCKADDR_ARG addr, socklen_t addr_len) { NOT_IMPLEMENTED(); return -1; }
INTERN ssize_t LIBCCALL libc_recvfrom(int fd, void *__restrict buf, size_t n, int flags, __SOCKADDR_ARG addr, socklen_t *__restrict addr_len) { NOT_IMPLEMENTED(); return -1; }
INTERN ssize_t LIBCCALL libc_sendmsg(int fd, const struct msghdr *message, int flags) { NOT_IMPLEMENTED(); return -1; }
INTERN ssize_t LIBCCALL libc_recvmsg(int fd, struct msghdr *message, int flags) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getsockopt(int fd, int level, int optname, void *__restrict optval, socklen_t *__restrict optlen) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setsockopt(int fd, int level, int optname, void const *optval, socklen_t optlen) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_listen(int fd, int n) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_accept(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict addr_len) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_shutdown(int fd, int how) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sendmmsg(int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_recvmmsg(int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags, struct timespec *tmo) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_accept4(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict addr_len, int flags) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_sockatmark(int fd) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_isfdtype(int fd, int fdtype) { NOT_IMPLEMENTED(); return -1; }

DEFINE_PUBLIC_ALIAS(ntohl,libc_ntohl);
DEFINE_PUBLIC_ALIAS(ntohs,libc_ntohs);
DEFINE_PUBLIC_ALIAS(htonl,libc_htonl);
DEFINE_PUBLIC_ALIAS(htons,libc_htons);
DEFINE_PUBLIC_ALIAS(socket,libc_socket);
DEFINE_PUBLIC_ALIAS(socketpair,libc_socketpair);
DEFINE_PUBLIC_ALIAS(bind,libc_bind);
DEFINE_PUBLIC_ALIAS(getsockname,libc_getsockname);
DEFINE_PUBLIC_ALIAS(connect,libc_connect);
DEFINE_PUBLIC_ALIAS(getpeername,libc_getpeername);
DEFINE_PUBLIC_ALIAS(send,libc_send);
DEFINE_PUBLIC_ALIAS(recv,libc_recv);
DEFINE_PUBLIC_ALIAS(sendto,libc_sendto);
DEFINE_PUBLIC_ALIAS(recvfrom,libc_recvfrom);
DEFINE_PUBLIC_ALIAS(sendmsg,libc_sendmsg);
DEFINE_PUBLIC_ALIAS(recvmsg,libc_recvmsg);
DEFINE_PUBLIC_ALIAS(getsockopt,libc_getsockopt);
DEFINE_PUBLIC_ALIAS(setsockopt,libc_setsockopt);
DEFINE_PUBLIC_ALIAS(listen,libc_listen);
DEFINE_PUBLIC_ALIAS(accept,libc_accept);
DEFINE_PUBLIC_ALIAS(shutdown,libc_shutdown);
DEFINE_PUBLIC_ALIAS(sendmmsg,libc_sendmmsg);
DEFINE_PUBLIC_ALIAS(recvmmsg,libc_recvmmsg);
DEFINE_PUBLIC_ALIAS(accept4,libc_accept4);
DEFINE_PUBLIC_ALIAS(sockatmark,libc_sockatmark);
DEFINE_PUBLIC_ALIAS(isfdtype,libc_isfdtype);

DECL_END

#endif /* !GUARD_LIBS_LIBC_INET_SOCKET_C */
