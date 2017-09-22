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
#ifndef GUARD_LIBS_LIBC_INET_NETDB_H
#define GUARD_LIBS_LIBC_INET_NETDB_H 1

#include "../libc.h"
#include <bits/sockaddr.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

struct ether_addr;
struct addrinfo;
struct gaicb;
struct sigevent;

INTDEF int *LIBCCALL libc___h_errno_location(void);
INTDEF void LIBCCALL libc_herror(char const *str);
INTDEF char const *LIBCCALL libc_hstrerror(int err_num);
INTDEF void LIBCCALL libc_sethostent(int stay_open);
INTDEF void LIBCCALL libc_endhostent(void);
INTDEF struct hostent *LIBCCALL libc_gethostent(void);
INTDEF struct hostent *LIBCCALL libc_gethostbyaddr(void const *addr, socklen_t len, int type);
INTDEF struct hostent *LIBCCALL libc_gethostbyname(char const *name);
INTDEF struct hostent *LIBCCALL libc_gethostbyname2(char const *name, int af);
INTDEF int LIBCCALL libc_gethostent_r(struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop);
INTDEF int LIBCCALL libc_gethostbyaddr_r(void const *__restrict addr, socklen_t len, int type, struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop);
INTDEF int LIBCCALL libc_gethostbyname_r(char const *__restrict name, struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop);
INTDEF int LIBCCALL libc_gethostbyname2_r(char const *__restrict name, int af, struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop);
INTDEF void LIBCCALL libc_setnetent(int stay_open);
INTDEF void LIBCCALL libc_endnetent(void);
INTDEF struct netent *LIBCCALL libc_getnetent(void);
INTDEF struct netent *LIBCCALL libc_getnetbyaddr(uint32_t net, int type);
INTDEF struct netent *LIBCCALL libc_getnetbyname(char const *name);
INTDEF int LIBCCALL libc_getnetent_r(struct netent *__restrict result_buf, char *__restrict buf, size_t buflen, struct netent **__restrict result, int *__restrict h_errnop);
INTDEF int LIBCCALL libc_getnetbyaddr_r(uint32_t net, int type, struct netent *__restrict result_buf, char *__restrict buf, size_t buflen, struct netent **__restrict result, int *__restrict h_errnop);
INTDEF int LIBCCALL libc_getnetbyname_r(char const *__restrict name, struct netent *__restrict result_buf, char *__restrict buf, size_t buflen, struct netent **__restrict result, int *__restrict h_errnop);
INTDEF void LIBCCALL libc_setservent(int stay_open);
INTDEF void LIBCCALL libc_endservent(void);
INTDEF struct servent *LIBCCALL libc_getservent(void);
INTDEF struct servent *LIBCCALL libc_getservbyname(char const *name, char const *proto);
INTDEF struct servent *LIBCCALL libc_getservbyport(int port, char const *proto);
INTDEF int LIBCCALL libc_getservent_r(struct servent *__restrict result_buf, char *__restrict buf, size_t buflen, struct servent **__restrict result);
INTDEF int LIBCCALL libc_getservbyname_r(char const *__restrict name, char const *__restrict proto, struct servent *__restrict result_buf, char *__restrict buf, size_t buflen, struct servent **__restrict result);
INTDEF int LIBCCALL libc_getservbyport_r(int port, char const *__restrict proto, struct servent *__restrict result_buf, char *__restrict buf, size_t buflen, struct servent **__restrict result);
INTDEF void LIBCCALL libc_setprotoent(int stay_open);
INTDEF void LIBCCALL libc_endprotoent(void);
INTDEF struct protoent *LIBCCALL libc_getprotoent(void);
INTDEF struct protoent *LIBCCALL libc_getprotobyname(char const *name);
INTDEF struct protoent *LIBCCALL libc_getprotobynumber(int proto);
INTDEF int LIBCCALL libc_getprotoent_r(struct protoent *__restrict result_buf, char *__restrict buf, size_t buflen, struct protoent **__restrict result);
INTDEF int LIBCCALL libc_getprotobyname_r(char const *__restrict name, struct protoent *__restrict result_buf, char *__restrict buf, size_t buflen, struct protoent **__restrict result);
INTDEF int LIBCCALL libc_getprotobynumber_r(int proto, struct protoent *__restrict result_buf, char *__restrict buf, size_t buflen, struct protoent **__restrict result);
INTDEF int LIBCCALL libc_setnetgrent(char const *netgroup);
INTDEF void LIBCCALL libc_endnetgrent(void);
INTDEF int LIBCCALL libc_getnetgrent(char **__restrict hostp, char **__restrict userp, char **__restrict domainp);
INTDEF int LIBCCALL libc_innetgr(char const *netgroup, char const *host, char const *user, char const *domain);
INTDEF int LIBCCALL libc_getnetgrent_r(char **__restrict hostp, char **__restrict userp, char **__restrict domainp, char *__restrict buffer, size_t buflen);
INTDEF int LIBCCALL libc_rcmd(char **__restrict ahost, unsigned short int rport, char const *__restrict locuser, char const *__restrict remuser, char const *__restrict cmd, int *__restrict fd2p);
INTDEF int LIBCCALL libc_rcmd_af(char **__restrict ahost, unsigned short int rport, char const *__restrict locuser, char const *__restrict remuser, char const *__restrict cmd, int *__restrict fd2p, sa_family_t af);
INTDEF int LIBCCALL libc_rexec(char **__restrict ahost, int rport, char const *__restrict name, char const *__restrict pass, char const *__restrict cmd, int *__restrict fd2p);
INTDEF int LIBCCALL libc_rexec_af(char **__restrict ahost, int rport, char const *__restrict name, char const *__restrict pass, char const *__restrict cmd, int *__restrict fd2p, sa_family_t af);
INTDEF int LIBCCALL libc_ruserok(char const *rhost, int suser, char const *remuser, char const *locuser);
INTDEF int LIBCCALL libc_ruserok_af(char const *rhost, int suser, char const *remuser, char const *locuser, sa_family_t af);
INTDEF int LIBCCALL libc_iruserok(uint32_t raddr, int suser, char const *remuser, char const *locuser);
INTDEF int LIBCCALL libc_iruserok_af(void const *raddr, int suser, char const *remuser, char const *locuser, sa_family_t af);
INTDEF int LIBCCALL libc_rresvport(int *alport);
INTDEF int LIBCCALL libc_rresvport_af(int *alport, sa_family_t af);
INTDEF int LIBCCALL libc_getaddrinfo(char const *__restrict name, char const *__restrict service, struct addrinfo const *__restrict req, struct addrinfo **__restrict pai);
INTDEF void LIBCCALL libc_freeaddrinfo(struct addrinfo *ai);
INTDEF char const *LIBCCALL libc_gai_strerror(int ecode);
INTDEF int LIBCCALL libc_getnameinfo(struct sockaddr const *__restrict sa, socklen_t salen, char *__restrict host, socklen_t __hostlen, char *__restrict __serv, socklen_t __servlen, int __flags);
INTDEF int LIBCCALL libc_getaddrinfo_a(int mode, struct gaicb *list[__restrict_arr], int ent, struct sigevent *__restrict sig);
INTDEF int LIBCCALL libc_gai_suspend(struct gaicb const *const list[], int ent, struct timespec const *timeout);
INTDEF int LIBCCALL libc_gai_error(struct gaicb *req);
INTDEF int LIBCCALL libc_gai_cancel(struct gaicb *gaicbp);
INTDEF in_addr_t LIBCCALL libc_inet_addr(char const *cp);
INTDEF in_addr_t LIBCCALL libc_inet_lnaof(struct in_addr in);
INTDEF struct in_addr LIBCCALL libc_inet_makeaddr(in_addr_t net, in_addr_t host);
INTDEF in_addr_t LIBCCALL libc_inet_netof(struct in_addr in);
INTDEF in_addr_t LIBCCALL libc_inet_network(char const *cp);
INTDEF char *LIBCCALL libc_inet_ntoa(struct in_addr in);
INTDEF int LIBCCALL libc_inet_pton(int af, char const *__restrict cp, void *__restrict buf);
INTDEF char const *LIBCCALL libc_inet_ntop(int af, void const *__restrict cp, char *__restrict buf, socklen_t len);
INTDEF int LIBCCALL libc_inet_aton(char const *cp, struct in_addr *inp);
INTDEF char *LIBCCALL libc_inet_neta(in_addr_t net, char *buf, size_t len);
INTDEF char *LIBCCALL libc_inet_net_ntop(int af, void const *cp, int bits, char *buf, size_t len);
INTDEF int LIBCCALL libc_inet_net_pton(int af, char const *cp, void *buf, size_t len);
INTDEF unsigned int LIBCCALL libc_inet_nsap_addr(char const *cp, unsigned char *buf, int len);
INTDEF char *LIBCCALL libc_inet_nsap_ntoa(int len, const unsigned char *cp, char *buf);
INTDEF unsigned int LIBCCALL libc_if_nametoindex(char const *ifname);
INTDEF char *LIBCCALL libc_if_indextoname(unsigned int ifindex, char *ifname);
INTDEF struct if_nameindex *LIBCCALL libc_if_nameindex(void);
INTDEF void LIBCCALL libc_if_freenameindex(struct if_nameindex *ptr);
INTDEF char *LIBCCALL libc_ether_ntoa(struct ether_addr const *addr);
INTDEF char *LIBCCALL libc_ether_ntoa_r(struct ether_addr const *addr, char *buf);
INTDEF struct ether_addr *LIBCCALL libc_ether_aton(char const *asc);
INTDEF struct ether_addr *LIBCCALL libc_ether_aton_r(char const *asc, struct ether_addr *addr);
INTDEF int LIBCCALL libc_ether_ntohost(char *hostname, struct ether_addr const *addr);
INTDEF int LIBCCALL libc_ether_hostton(char const *hostname, struct ether_addr *addr);
INTDEF int LIBCCALL libc_ether_line(char const *line, struct ether_addr *addr, char *hostname);
INTDEF int LIBCCALL libc_inet6_option_space(int nbytes);
INTDEF int LIBCCALL libc_inet6_option_init(void *bp, struct cmsghdr **cmsgp, int type);
INTDEF int LIBCCALL libc_inet6_option_append(struct cmsghdr *cmsg, const uint8_t *typep, int multx, int plusy);
INTDEF uint8_t *LIBCCALL libc_inet6_option_alloc(struct cmsghdr *cmsg, int datalen, int multx, int plusy);
INTDEF int LIBCCALL libc_inet6_option_next(const struct cmsghdr *cmsg, uint8_t **tptrp);
INTDEF int LIBCCALL libc_inet6_option_find(const struct cmsghdr *cmsg, uint8_t **tptrp, int type);
INTDEF int LIBCCALL libc_inet6_opt_init(void *extbuf, socklen_t extlen);
INTDEF int LIBCCALL libc_inet6_opt_append(void *extbuf, socklen_t extlen, int offset, uint8_t type, socklen_t len, uint8_t align, void **databufp);
INTDEF int LIBCCALL libc_inet6_opt_finish(void *extbuf, socklen_t extlen, int offset);
INTDEF int LIBCCALL libc_inet6_opt_set_val(void *databuf, int offset, void *val, socklen_t vallen);
INTDEF int LIBCCALL libc_inet6_opt_next(void *extbuf, socklen_t extlen, int offset, uint8_t *typep, socklen_t *lenp, void **databufp);
INTDEF int LIBCCALL libc_inet6_opt_find(void *extbuf, socklen_t extlen, int offset, uint8_t type, socklen_t *lenp, void **databufp);
INTDEF int LIBCCALL libc_inet6_opt_get_val(void *databuf, int offset, void *val, socklen_t vallen);
INTDEF socklen_t LIBCCALL libc_inet6_rth_space(int type, int segments);
INTDEF void *LIBCCALL libc_inet6_rth_init(void *bp, socklen_t bp_len, int type, int segments);
INTDEF int LIBCCALL libc_inet6_rth_add(void *bp, struct in6_addr const *addr);
INTDEF int LIBCCALL libc_inet6_rth_reverse(void const *in, void *out);
INTDEF int LIBCCALL libc_inet6_rth_segments(void const *bp);
INTDEF struct in6_addr *LIBCCALL libc_inet6_rth_getaddr(void const *bp, int index);
INTDEF int LIBCCALL libc_getipv4sourcefilter(int s, struct in_addr interface_addr, struct in_addr group, uint32_t *fmode, uint32_t *numsrc, struct in_addr *slist);
INTDEF int LIBCCALL libc_setipv4sourcefilter(int s, struct in_addr interface_addr, struct in_addr group, uint32_t fmode, uint32_t numsrc, const struct in_addr *slist);
INTDEF int LIBCCALL libc_getsourcefilter(int s, uint32_t interface_addr, struct sockaddr const *group, socklen_t grouplen, uint32_t *fmode, uint32_t *numsrc, struct sockaddr_storage *slist);
INTDEF int LIBCCALL libc_setsourcefilter(int s, uint32_t interface_addr, struct sockaddr const *group, socklen_t grouplen, uint32_t fmode, uint32_t numsrc, const struct sockaddr_storage *slist);

DECL_END

#endif /* !GUARD_LIBS_LIBC_INET_NETDB_H */
