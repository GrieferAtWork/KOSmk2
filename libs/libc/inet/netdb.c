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
#ifndef GUARD_LIBS_LIBC_INET_NETDB_C
#define GUARD_LIBS_LIBC_INET_NETDB_C 1
#define _GNU_SOURCE 1

#include "../libc.h"
#include "netdb.h"
#include <hybrid/compiler.h>
#include <stddef.h>
#include <netdb.h>

DECL_BEGIN

PRIVATE int h_errno_val = 0;
INTERN int *LIBCCALL libc___h_errno_location(void) { return &h_errno_val; }
INTERN void LIBCCALL libc_herror(char const *str) { NOT_IMPLEMENTED(); }
INTERN char const *LIBCCALL libc_hstrerror(int err_num) { NOT_IMPLEMENTED(); return NULL; }
INTERN void LIBCCALL libc_sethostent(int stay_open) { NOT_IMPLEMENTED(); }
INTERN void LIBCCALL libc_endhostent(void) { NOT_IMPLEMENTED(); }
INTERN struct hostent *LIBCCALL libc_gethostent(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct hostent *LIBCCALL libc_gethostbyaddr(void const *__addr, socklen_t len, int type) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct hostent *LIBCCALL libc_gethostbyname(char const *name) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct hostent *LIBCCALL libc_gethostbyname2(char const *name, int af) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_gethostent_r(struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_gethostbyaddr_r(void const *__restrict __addr, socklen_t len, int type, struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_gethostbyname_r(char const *__restrict name, struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_gethostbyname2_r(char const *__restrict name, int af, struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
INTERN void LIBCCALL libc_setnetent(int stay_open) { NOT_IMPLEMENTED(); }
INTERN void LIBCCALL libc_endnetent(void) { NOT_IMPLEMENTED(); }
INTERN struct netent *LIBCCALL libc_getnetent(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct netent *LIBCCALL libc_getnetbyaddr(uint32_t net, int type) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct netent *LIBCCALL libc_getnetbyname(char const *name) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_getnetent_r(struct netent *__restrict result_buf, char *__restrict buf, size_t buflen, struct netent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_getnetbyaddr_r(uint32_t net, int type, struct netent *__restrict result_buf, char *__restrict buf, size_t buflen, struct netent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_getnetbyname_r(char const *__restrict name, struct netent *__restrict result_buf, char *__restrict buf, size_t buflen, struct netent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
INTERN void LIBCCALL libc_setservent(int stay_open) { NOT_IMPLEMENTED(); }
INTERN void LIBCCALL libc_endservent(void) { NOT_IMPLEMENTED(); }
INTERN struct servent *LIBCCALL libc_getservent(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct servent *LIBCCALL libc_getservbyname(char const *name, char const *proto) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct servent *LIBCCALL libc_getservbyport(int port, char const *proto) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_getservent_r(struct servent *__restrict result_buf, char *__restrict buf, size_t buflen, struct servent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_getservbyname_r(char const *__restrict name, char const *__restrict proto, struct servent *__restrict result_buf, char *__restrict buf, size_t buflen, struct servent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_getservbyport_r(int port, char const *__restrict proto, struct servent *__restrict result_buf, char *__restrict buf, size_t buflen, struct servent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN void LIBCCALL libc_setprotoent(int stay_open) { NOT_IMPLEMENTED(); }
INTERN void LIBCCALL libc_endprotoent(void) { NOT_IMPLEMENTED(); }
INTERN struct protoent *LIBCCALL libc_getprotoent(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct protoent *LIBCCALL libc_getprotobyname(char const *name) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct protoent *LIBCCALL libc_getprotobynumber(int proto) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_getprotoent_r(struct protoent *__restrict result_buf, char *__restrict buf, size_t buflen, struct protoent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_getprotobyname_r(char const *__restrict name, struct protoent *__restrict result_buf, char *__restrict buf, size_t buflen, struct protoent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_getprotobynumber_r(int proto, struct protoent *__restrict result_buf, char *__restrict buf, size_t buflen, struct protoent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_setnetgrent(char const *netgroup) { NOT_IMPLEMENTED(); return 0; }
INTERN void LIBCCALL libc_endnetgrent(void) { NOT_IMPLEMENTED(); }
INTERN int LIBCCALL libc_getnetgrent(char **__restrict hostp, char **__restrict userp, char **__restrict domainp) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_innetgr(char const *netgroup, char const *host, char const *user, char const *domain) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_getnetgrent_r(char **__restrict hostp, char **__restrict userp, char **__restrict domainp, char *__restrict buffer, size_t buflen) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_rcmd(char **__restrict ahost, unsigned short int rport, char const *__restrict locuser, char const *__restrict remuser, char const *__restrict cmd, int *__restrict fd2p) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_rcmd_af(char **__restrict ahost, unsigned short int rport, char const *__restrict locuser, char const *__restrict remuser, char const *__restrict cmd, int *__restrict fd2p, sa_family_t af) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_rexec(char **__restrict ahost, int rport, char const *__restrict name, char const *__restrict pass, char const *__restrict cmd, int *__restrict fd2p) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_rexec_af(char **__restrict ahost, int rport, char const *__restrict name, char const *__restrict pass, char const *__restrict cmd, int *__restrict fd2p, sa_family_t af) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_ruserok(char const *rhost, int suser, char const *remuser, char const *locuser) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_ruserok_af(char const *rhost, int suser, char const *remuser, char const *locuser, sa_family_t af) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_iruserok(uint32_t raddr, int suser, char const *remuser, char const *locuser) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_iruserok_af(void const *raddr, int suser, char const *remuser, char const *locuser, sa_family_t af) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_rresvport(int *alport) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_rresvport_af(int *alport, sa_family_t af) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_getaddrinfo(char const *__restrict name, char const *__restrict service, struct addrinfo const *__restrict req, struct addrinfo **__restrict pai) { NOT_IMPLEMENTED(); return 0; }
INTERN void LIBCCALL libc_freeaddrinfo(struct addrinfo *ai) { NOT_IMPLEMENTED(); }
INTERN char const *LIBCCALL libc_gai_strerror(int ecode) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_getnameinfo(struct sockaddr const *__restrict sa, socklen_t salen, char *__restrict host, socklen_t __hostlen, char *__restrict __serv, socklen_t __servlen, int __flags) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_getaddrinfo_a(int mode, struct gaicb *list[__restrict_arr], int ent, struct sigevent *__restrict sig) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_gai_suspend(struct gaicb const *const list[], int ent, struct timespec const *timeout) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_gai_error(struct gaicb *req) { NOT_IMPLEMENTED(); return 0; }

INTERN int LIBCCALL libc_gai_cancel(struct gaicb *gaicbp) { NOT_IMPLEMENTED(); return 0; }
INTERN in_addr_t LIBCCALL libc_inet_addr(char const *cp) { NOT_IMPLEMENTED(); return -1; }
INTERN in_addr_t LIBCCALL libc_inet_lnaof(struct in_addr in) { NOT_IMPLEMENTED(); return -1; }
INTERN struct in_addr LIBCCALL libc_inet_makeaddr(in_addr_t net, in_addr_t host) { NOT_IMPLEMENTED(); return (struct in_addr){ -1 }; }
INTERN in_addr_t LIBCCALL libc_inet_netof(struct in_addr in) { NOT_IMPLEMENTED(); return -1; }
INTERN in_addr_t LIBCCALL libc_inet_network(char const *cp) { NOT_IMPLEMENTED(); return -1; }
INTERN char *LIBCCALL libc_inet_ntoa(struct in_addr in) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_inet_pton(int af, char const *__restrict cp, void *__restrict buf) { NOT_IMPLEMENTED(); return -1; }
INTERN char const *LIBCCALL libc_inet_ntop(int af, void const *__restrict cp, char *__restrict buf, socklen_t len) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_inet_aton(char const *cp, struct in_addr *inp) { NOT_IMPLEMENTED(); return -1; }
INTERN char *LIBCCALL libc_inet_neta(in_addr_t net, char *buf, size_t len) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_inet_net_ntop(int af, void const *cp, int bits, char *buf, size_t len) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_inet_net_pton(int af, char const *cp, void *buf, size_t len) { NOT_IMPLEMENTED(); return -1; }
INTERN unsigned int LIBCCALL libc_inet_nsap_addr(char const *cp, unsigned char *buf, int len) { NOT_IMPLEMENTED(); return 0; }
INTERN char *LIBCCALL libc_inet_nsap_ntoa(int len, const unsigned char *cp, char *buf) { NOT_IMPLEMENTED(); return NULL; }
INTERN unsigned int LIBCCALL libc_if_nametoindex(char const *ifname) { NOT_IMPLEMENTED(); return 0; }
INTERN char *LIBCCALL libc_if_indextoname(unsigned int ifindex, char *ifname) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct if_nameindex *LIBCCALL libc_if_nameindex(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN void LIBCCALL libc_if_freenameindex(struct if_nameindex *ptr) { NOT_IMPLEMENTED(); }

INTERN char *LIBCCALL libc_ether_ntoa(struct ether_addr const *addr) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_ether_ntoa_r(struct ether_addr const *addr, char *buf) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct ether_addr *LIBCCALL libc_ether_aton(char const *asc) { NOT_IMPLEMENTED(); return NULL; }
INTERN struct ether_addr *LIBCCALL libc_ether_aton_r(char const *asc, struct ether_addr *addr) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_ether_ntohost(char *hostname, struct ether_addr const *addr) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_ether_hostton(char const *hostname, struct ether_addr *addr) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_ether_line(char const *line, struct ether_addr *addr, char *hostname) { NOT_IMPLEMENTED(); return -1; }

INTERN int LIBCCALL libc_inet6_option_space(int nbytes) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_option_init(void *bp, struct cmsghdr **cmsgp, int type) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_option_append(struct cmsghdr *cmsg, const uint8_t *typep, int multx, int plusy) { NOT_IMPLEMENTED(); return -1; }
INTERN uint8_t *LIBCCALL libc_inet6_option_alloc(struct cmsghdr *cmsg, int datalen, int multx, int plusy) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_inet6_option_next(const struct cmsghdr *cmsg, uint8_t **tptrp) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_option_find(const struct cmsghdr *cmsg, uint8_t **tptrp, int type) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_opt_init(void *extbuf, socklen_t extlen) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_opt_append(void *extbuf, socklen_t extlen, int offset, uint8_t type, socklen_t len, uint8_t align, void **databufp) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_opt_finish(void *extbuf, socklen_t extlen, int offset) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_opt_set_val(void *databuf, int offset, void *val, socklen_t vallen) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_opt_next(void *extbuf, socklen_t extlen, int offset, uint8_t *typep, socklen_t *lenp, void **databufp) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_opt_find(void *extbuf, socklen_t extlen, int offset, uint8_t type, socklen_t *lenp, void **databufp) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_opt_get_val(void *databuf, int offset, void *val, socklen_t vallen) { NOT_IMPLEMENTED(); return -1; }
INTERN socklen_t LIBCCALL libc_inet6_rth_space(int type, int segments) { NOT_IMPLEMENTED(); return sizeof(struct sockaddr); }
INTERN void *LIBCCALL libc_inet6_rth_init(void *bp, socklen_t bp_len, int type, int segments) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_inet6_rth_add(void *bp, struct in6_addr const *addr) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_rth_reverse(void const *in, void *out) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_inet6_rth_segments(void const *bp) { NOT_IMPLEMENTED(); return -1; }
INTERN struct in6_addr *LIBCCALL libc_inet6_rth_getaddr(void const *bp, int index) { NOT_IMPLEMENTED(); return NULL; }

INTERN int LIBCCALL libc_getipv4sourcefilter(int s, struct in_addr interface_addr, struct in_addr group, uint32_t *fmode, uint32_t *numsrc, struct in_addr *slist) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setipv4sourcefilter(int s, struct in_addr interface_addr, struct in_addr group, uint32_t fmode, uint32_t numsrc, const struct in_addr *slist) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_getsourcefilter(int s, uint32_t interface_addr, struct sockaddr const *group, socklen_t grouplen, uint32_t *fmode, uint32_t *numsrc, struct sockaddr_storage *slist) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_setsourcefilter(int s, uint32_t interface_addr, struct sockaddr const *group, socklen_t grouplen, uint32_t fmode, uint32_t numsrc, const struct sockaddr_storage *slist) { NOT_IMPLEMENTED(); return -1; }

DEFINE_PUBLIC_ALIAS(__h_errno_location,libc___h_errno_location);
DEFINE_PUBLIC_ALIAS(herror,libc_herror);
DEFINE_PUBLIC_ALIAS(hstrerror,libc_hstrerror);
DEFINE_PUBLIC_ALIAS(sethostent,libc_sethostent);
DEFINE_PUBLIC_ALIAS(endhostent,libc_endhostent);
DEFINE_PUBLIC_ALIAS(gethostent,libc_gethostent);
DEFINE_PUBLIC_ALIAS(gethostbyaddr,libc_gethostbyaddr);
DEFINE_PUBLIC_ALIAS(gethostbyname,libc_gethostbyname);
DEFINE_PUBLIC_ALIAS(gethostbyname2,libc_gethostbyname2);
DEFINE_PUBLIC_ALIAS(gethostent_r,libc_gethostent_r);
DEFINE_PUBLIC_ALIAS(gethostbyaddr_r,libc_gethostbyaddr_r);
DEFINE_PUBLIC_ALIAS(gethostbyname_r,libc_gethostbyname_r);
DEFINE_PUBLIC_ALIAS(gethostbyname2_r,libc_gethostbyname2_r);
DEFINE_PUBLIC_ALIAS(setnetent,libc_setnetent);
DEFINE_PUBLIC_ALIAS(endnetent,libc_endnetent);
DEFINE_PUBLIC_ALIAS(getnetent,libc_getnetent);
DEFINE_PUBLIC_ALIAS(getnetbyaddr,libc_getnetbyaddr);
DEFINE_PUBLIC_ALIAS(getnetbyname,libc_getnetbyname);
DEFINE_PUBLIC_ALIAS(getnetent_r,libc_getnetent_r);
DEFINE_PUBLIC_ALIAS(getnetbyaddr_r,libc_getnetbyaddr_r);
DEFINE_PUBLIC_ALIAS(getnetbyname_r,libc_getnetbyname_r);
DEFINE_PUBLIC_ALIAS(setservent,libc_setservent);
DEFINE_PUBLIC_ALIAS(endservent,libc_endservent);
DEFINE_PUBLIC_ALIAS(getservent,libc_getservent);
DEFINE_PUBLIC_ALIAS(getservbyname,libc_getservbyname);
DEFINE_PUBLIC_ALIAS(getservbyport,libc_getservbyport);
DEFINE_PUBLIC_ALIAS(getservent_r,libc_getservent_r);
DEFINE_PUBLIC_ALIAS(getservbyname_r,libc_getservbyname_r);
DEFINE_PUBLIC_ALIAS(getservbyport_r,libc_getservbyport_r);
DEFINE_PUBLIC_ALIAS(setprotoent,libc_setprotoent);
DEFINE_PUBLIC_ALIAS(endprotoent,libc_endprotoent);
DEFINE_PUBLIC_ALIAS(getprotoent,libc_getprotoent);
DEFINE_PUBLIC_ALIAS(getprotobyname,libc_getprotobyname);
DEFINE_PUBLIC_ALIAS(getprotobynumber,libc_getprotobynumber);
DEFINE_PUBLIC_ALIAS(getprotoent_r,libc_getprotoent_r);
DEFINE_PUBLIC_ALIAS(getprotobyname_r,libc_getprotobyname_r);
DEFINE_PUBLIC_ALIAS(getprotobynumber_r,libc_getprotobynumber_r);
DEFINE_PUBLIC_ALIAS(setnetgrent,libc_setnetgrent);
DEFINE_PUBLIC_ALIAS(endnetgrent,libc_endnetgrent);
DEFINE_PUBLIC_ALIAS(getnetgrent,libc_getnetgrent);
DEFINE_PUBLIC_ALIAS(innetgr,libc_innetgr);
DEFINE_PUBLIC_ALIAS(getnetgrent_r,libc_getnetgrent_r);
DEFINE_PUBLIC_ALIAS(rcmd,libc_rcmd);
DEFINE_PUBLIC_ALIAS(rcmd_af,libc_rcmd_af);
DEFINE_PUBLIC_ALIAS(rexec,libc_rexec);
DEFINE_PUBLIC_ALIAS(rexec_af,libc_rexec_af);
DEFINE_PUBLIC_ALIAS(ruserok,libc_ruserok);
DEFINE_PUBLIC_ALIAS(ruserok_af,libc_ruserok_af);
DEFINE_PUBLIC_ALIAS(iruserok,libc_iruserok);
DEFINE_PUBLIC_ALIAS(iruserok_af,libc_iruserok_af);
DEFINE_PUBLIC_ALIAS(rresvport,libc_rresvport);
DEFINE_PUBLIC_ALIAS(rresvport_af,libc_rresvport_af);
DEFINE_PUBLIC_ALIAS(getaddrinfo,libc_getaddrinfo);
DEFINE_PUBLIC_ALIAS(freeaddrinfo,libc_freeaddrinfo);
DEFINE_PUBLIC_ALIAS(gai_strerror,libc_gai_strerror);
DEFINE_PUBLIC_ALIAS(getnameinfo,libc_getnameinfo);
DEFINE_PUBLIC_ALIAS(getaddrinfo_a,libc_getaddrinfo_a);
DEFINE_PUBLIC_ALIAS(gai_suspend,libc_gai_suspend);
DEFINE_PUBLIC_ALIAS(gai_error,libc_gai_error);
DEFINE_PUBLIC_ALIAS(gai_cancel,libc_gai_cancel);
DEFINE_PUBLIC_ALIAS(inet_addr,libc_inet_addr);
DEFINE_PUBLIC_ALIAS(inet_lnaof,libc_inet_lnaof);
DEFINE_PUBLIC_ALIAS(inet_makeaddr,libc_inet_makeaddr);
DEFINE_PUBLIC_ALIAS(inet_netof,libc_inet_netof);
DEFINE_PUBLIC_ALIAS(inet_network,libc_inet_network);
DEFINE_PUBLIC_ALIAS(inet_ntoa,libc_inet_ntoa);
DEFINE_PUBLIC_ALIAS(inet_pton,libc_inet_pton);
DEFINE_PUBLIC_ALIAS(inet_ntop,libc_inet_ntop);
DEFINE_PUBLIC_ALIAS(inet_aton,libc_inet_aton);
DEFINE_PUBLIC_ALIAS(inet_neta,libc_inet_neta);
DEFINE_PUBLIC_ALIAS(inet_net_ntop,libc_inet_net_ntop);
DEFINE_PUBLIC_ALIAS(inet_net_pton,libc_inet_net_pton);
DEFINE_PUBLIC_ALIAS(inet_nsap_addr,libc_inet_nsap_addr);
DEFINE_PUBLIC_ALIAS(inet_nsap_ntoa,libc_inet_nsap_ntoa);
DEFINE_PUBLIC_ALIAS(if_nametoindex,libc_if_nametoindex);
DEFINE_PUBLIC_ALIAS(if_indextoname,libc_if_indextoname);
DEFINE_PUBLIC_ALIAS(if_nameindex,libc_if_nameindex);
DEFINE_PUBLIC_ALIAS(if_freenameindex,libc_if_freenameindex);
DEFINE_PUBLIC_ALIAS(ether_ntoa,libc_ether_ntoa);
DEFINE_PUBLIC_ALIAS(ether_ntoa_r,libc_ether_ntoa_r);
DEFINE_PUBLIC_ALIAS(ether_aton,libc_ether_aton);
DEFINE_PUBLIC_ALIAS(ether_aton_r,libc_ether_aton_r);
DEFINE_PUBLIC_ALIAS(ether_ntohost,libc_ether_ntohost);
DEFINE_PUBLIC_ALIAS(ether_hostton,libc_ether_hostton);
DEFINE_PUBLIC_ALIAS(ether_line,libc_ether_line);
DEFINE_PUBLIC_ALIAS(inet6_option_space,libc_inet6_option_space);
DEFINE_PUBLIC_ALIAS(inet6_option_init,libc_inet6_option_init);
DEFINE_PUBLIC_ALIAS(inet6_option_append,libc_inet6_option_append);
DEFINE_PUBLIC_ALIAS(inet6_option_alloc,libc_inet6_option_alloc);
DEFINE_PUBLIC_ALIAS(inet6_option_next,libc_inet6_option_next);
DEFINE_PUBLIC_ALIAS(inet6_option_find,libc_inet6_option_find);
DEFINE_PUBLIC_ALIAS(inet6_opt_init,libc_inet6_opt_init);
DEFINE_PUBLIC_ALIAS(inet6_opt_append,libc_inet6_opt_append);
DEFINE_PUBLIC_ALIAS(inet6_opt_finish,libc_inet6_opt_finish);
DEFINE_PUBLIC_ALIAS(inet6_opt_set_val,libc_inet6_opt_set_val);
DEFINE_PUBLIC_ALIAS(inet6_opt_next,libc_inet6_opt_next);
DEFINE_PUBLIC_ALIAS(inet6_opt_find,libc_inet6_opt_find);
DEFINE_PUBLIC_ALIAS(inet6_opt_get_val,libc_inet6_opt_get_val);
DEFINE_PUBLIC_ALIAS(inet6_rth_space,libc_inet6_rth_space);
DEFINE_PUBLIC_ALIAS(inet6_rth_init,libc_inet6_rth_init);
DEFINE_PUBLIC_ALIAS(inet6_rth_add,libc_inet6_rth_add);
DEFINE_PUBLIC_ALIAS(inet6_rth_reverse,libc_inet6_rth_reverse);
DEFINE_PUBLIC_ALIAS(inet6_rth_segments,libc_inet6_rth_segments);
DEFINE_PUBLIC_ALIAS(inet6_rth_getaddr,libc_inet6_rth_getaddr);
DEFINE_PUBLIC_ALIAS(getipv4sourcefilter,libc_getipv4sourcefilter);
DEFINE_PUBLIC_ALIAS(setipv4sourcefilter,libc_setipv4sourcefilter);
DEFINE_PUBLIC_ALIAS(getsourcefilter,libc_getsourcefilter);
DEFINE_PUBLIC_ALIAS(setsourcefilter,libc_setsourcefilter);

DECL_END

#endif /* !GUARD_LIBS_LIBC_INET_NETDB_C */
