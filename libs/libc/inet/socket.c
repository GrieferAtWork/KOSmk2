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
#include <hybrid/compiler.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <hybrid/timespec.h>
#include <net/if.h>

DECL_BEGIN

#if __BYTE_ORDER == __BIG_ENDIAN
PUBLIC uint32_t (LIBCCALL ntohl)(uint32_t netlong) { return netlong; }
PUBLIC uint16_t (LIBCCALL ntohs)(uint16_t netshort) { return netshort; }
DEFINE_PUBLIC_ALIAS(htonl,ntohl);
DEFINE_PUBLIC_ALIAS(htons,ntohs);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
PUBLIC uint32_t (LIBCCALL ntohl)(uint32_t netlong) { return __bswap_32(netlong); }
PUBLIC uint16_t (LIBCCALL ntohs)(uint16_t netshort) { return __bswap_16(netshort); }
DEFINE_PUBLIC_ALIAS(htonl,ntohl);
DEFINE_PUBLIC_ALIAS(htons,ntohs);
#else
#error FIXME
#endif
PUBLIC int (LIBCCALL socket)(int domain, int type, int protocol) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL socketpair)(int domain, int type, int protocol, int fds[2]) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL bind)(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getsockname)(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL connect)(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getpeername)(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC ssize_t (LIBCCALL send)(int fd, void const *buf, size_t n, int flags) { NOT_IMPLEMENTED(); return -1; }
PUBLIC ssize_t (LIBCCALL recv)(int fd, void *buf, size_t n, int flags) { NOT_IMPLEMENTED(); return -1; }
PUBLIC ssize_t (LIBCCALL sendto)(int fd, void const *buf, size_t n, int flags, __CONST_SOCKADDR_ARG addr, socklen_t addr_len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC ssize_t (LIBCCALL recvfrom)(int fd, void *__restrict buf, size_t n, int flags, __SOCKADDR_ARG addr, socklen_t *__restrict addr_len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC ssize_t (LIBCCALL sendmsg)(int fd, const struct msghdr *message, int flags) { NOT_IMPLEMENTED(); return -1; }
PUBLIC ssize_t (LIBCCALL recvmsg)(int fd, struct msghdr *message, int flags) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL getsockopt)(int fd, int level, int optname, void *__restrict optval, socklen_t *__restrict optlen) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL setsockopt)(int fd, int level, int optname, void const *optval, socklen_t optlen) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL listen)(int fd, int n) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL accept)(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict addr_len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL shutdown)(int fd, int how) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sendmmsg)(int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL recvmmsg)(int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags, struct timespec *tmo) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL accept4)(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict addr_len, int flags) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL sockatmark)(int fd) { NOT_IMPLEMENTED(); return -1; }
PUBLIC int (LIBCCALL isfdtype)(int fd, int fdtype) { NOT_IMPLEMENTED(); return -1; }
PUBLIC in_addr_t (LIBCCALL inet_addr)(char const *cp) { NOT_IMPLEMENTED(); return -1; }
PUBLIC in_addr_t (LIBCCALL inet_lnaof)(struct in_addr in) { NOT_IMPLEMENTED(); return -1; }
PUBLIC struct in_addr (LIBCCALL inet_makeaddr)(in_addr_t net, in_addr_t host) { NOT_IMPLEMENTED(); return (struct in_addr){ -1 }; }
PUBLIC in_addr_t (LIBCCALL inet_netof)(struct in_addr in) { NOT_IMPLEMENTED(); return -1; }
PUBLIC in_addr_t (LIBCCALL inet_network)(char const *cp) { NOT_IMPLEMENTED(); return -1; }
PUBLIC char *(LIBCCALL inet_ntoa)(struct in_addr in) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL inet_pton)(int af, char const *__restrict cp, void *__restrict buf) { NOT_IMPLEMENTED(); return -1; }
PUBLIC char const *(LIBCCALL inet_ntop)(int af, void const *__restrict cp, char *__restrict buf, socklen_t len) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL inet_aton)(char const *cp, struct in_addr *inp) { NOT_IMPLEMENTED(); return -1; }
PUBLIC char *(LIBCCALL inet_neta)(in_addr_t net, char *buf, size_t len) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC char *(LIBCCALL inet_net_ntop)(int af, void const *cp, int bits, char *buf, size_t len) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL inet_net_pton)(int af, char const *cp, void *buf, size_t len) { NOT_IMPLEMENTED(); return -1; }
PUBLIC unsigned int (LIBCCALL inet_nsap_addr)(char const *cp, unsigned char *buf, int len) { NOT_IMPLEMENTED(); return 0; }
PUBLIC char *(LIBCCALL inet_nsap_ntoa)(int len, const unsigned char *cp, char *buf) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC unsigned int (LIBCCALL if_nametoindex)(char const *ifname) { NOT_IMPLEMENTED(); return 0; }
PUBLIC char *(LIBCCALL if_indextoname)(unsigned int ifindex, char *ifname) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct if_nameindex *(LIBCCALL if_nameindex)(void) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC void (LIBCCALL if_freenameindex)(struct if_nameindex *ptr) { NOT_IMPLEMENTED(); }

DECL_END

#endif /* !GUARD_LIBS_LIBC_INET_SOCKET_C */
