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
#include <hybrid/compiler.h>
#include <stddef.h>
#include <netdb.h>

DECL_BEGIN

PRIVATE int h_errno_val = 0;
PUBLIC int *(LIBCCALL __h_errno_location)(void) { return &h_errno_val; }
PUBLIC void (LIBCCALL herror)(char const *str) { NOT_IMPLEMENTED(); }
PUBLIC char const *(LIBCCALL hstrerror)(int err_num) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC void (LIBCCALL sethostent)(int stay_open) { NOT_IMPLEMENTED(); }
PUBLIC void (LIBCCALL endhostent)(void) { NOT_IMPLEMENTED(); }
PUBLIC struct hostent *(LIBCCALL gethostent)(void) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct hostent *(LIBCCALL gethostbyaddr)(void const *__addr, socklen_t __len, int type) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct hostent *(LIBCCALL gethostbyname)(char const *name) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct hostent *(LIBCCALL gethostbyname2)(char const *name, int af) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL gethostent_r)(struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL gethostbyaddr_r)(void const *__restrict __addr, socklen_t __len, int type, struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL gethostbyname_r)(char const *__restrict name, struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL gethostbyname2_r)(char const *__restrict name, int af, struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
PUBLIC void (LIBCCALL setnetent)(int stay_open) { NOT_IMPLEMENTED(); }
PUBLIC void (LIBCCALL endnetent)(void) { NOT_IMPLEMENTED(); }
PUBLIC struct netent *(LIBCCALL getnetent)(void) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct netent *(LIBCCALL getnetbyaddr)(uint32_t net, int type) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct netent *(LIBCCALL getnetbyname)(char const *name) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL getnetent_r)(struct netent *__restrict result_buf, char *__restrict buf, size_t buflen, struct netent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL getnetbyaddr_r)(uint32_t net, int type, struct netent *__restrict result_buf, char *__restrict buf, size_t buflen, struct netent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL getnetbyname_r)(char const *__restrict name, struct netent *__restrict result_buf, char *__restrict buf, size_t buflen, struct netent **__restrict result, int *__restrict h_errnop) { NOT_IMPLEMENTED(); return 0; }
PUBLIC void (LIBCCALL setservent)(int stay_open) { NOT_IMPLEMENTED(); }
PUBLIC void (LIBCCALL endservent)(void) { NOT_IMPLEMENTED(); }
PUBLIC struct servent *(LIBCCALL getservent)(void) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct servent *(LIBCCALL getservbyname)(char const *name, char const *proto) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct servent *(LIBCCALL getservbyport)(int port, char const *proto) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL getservent_r)(struct servent *__restrict result_buf, char *__restrict buf, size_t buflen, struct servent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL getservbyname_r)(char const *__restrict name, char const *__restrict proto, struct servent *__restrict result_buf, char *__restrict buf, size_t buflen, struct servent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL getservbyport_r)(int port, char const *__restrict proto, struct servent *__restrict result_buf, char *__restrict buf, size_t buflen, struct servent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC void (LIBCCALL setprotoent)(int stay_open) { NOT_IMPLEMENTED(); }
PUBLIC void (LIBCCALL endprotoent)(void) { NOT_IMPLEMENTED(); }
PUBLIC struct protoent *(LIBCCALL getprotoent)(void) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct protoent *(LIBCCALL getprotobyname)(char const *name) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC struct protoent *(LIBCCALL getprotobynumber)(int proto) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL getprotoent_r)(struct protoent *__restrict result_buf, char *__restrict buf, size_t buflen, struct protoent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL getprotobyname_r)(char const *__restrict name, struct protoent *__restrict result_buf, char *__restrict buf, size_t buflen, struct protoent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL getprotobynumber_r)(int proto, struct protoent *__restrict result_buf, char *__restrict buf, size_t buflen, struct protoent **__restrict result) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL setnetgrent)(char const *netgroup) { NOT_IMPLEMENTED(); return 0; }
PUBLIC void (LIBCCALL endnetgrent)(void) { NOT_IMPLEMENTED(); }
PUBLIC int (LIBCCALL getnetgrent)(char **__restrict hostp, char **__restrict userp, char **__restrict domainp) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL innetgr)(char const *netgroup, char const *host, char const *user, char const *domain) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL getnetgrent_r)(char **__restrict hostp, char **__restrict userp, char **__restrict domainp, char *__restrict buffer, size_t buflen) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL rcmd)(char **__restrict ahost, unsigned short int rport, char const *__restrict locuser, char const *__restrict remuser, char const *__restrict cmd, int *__restrict fd2p) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL rcmd_af)(char **__restrict ahost, unsigned short int rport, char const *__restrict locuser, char const *__restrict remuser, char const *__restrict cmd, int *__restrict fd2p, sa_family_t af) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL rexec)(char **__restrict ahost, int rport, char const *__restrict name, char const *__restrict pass, char const *__restrict cmd, int *__restrict fd2p) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL rexec_af)(char **__restrict ahost, int rport, char const *__restrict name, char const *__restrict pass, char const *__restrict cmd, int *__restrict fd2p, sa_family_t af) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL ruserok)(char const *rhost, int suser, char const *remuser, char const *locuser) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL ruserok_af)(char const *rhost, int suser, char const *remuser, char const *locuser, sa_family_t af) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL iruserok)(uint32_t raddr, int suser, char const *remuser, char const *locuser) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL iruserok_af)(void const *raddr, int suser, char const *remuser, char const *locuser, sa_family_t af) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL rresvport)(int *alport) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL rresvport_af)(int *alport, sa_family_t af) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL getaddrinfo)(char const *__restrict name, char const *__restrict service, const struct addrinfo *__restrict req, struct addrinfo **__restrict pai) { NOT_IMPLEMENTED(); return 0; }
PUBLIC void (LIBCCALL freeaddrinfo)(struct addrinfo *ai) { NOT_IMPLEMENTED(); }
PUBLIC char const *(LIBCCALL gai_strerror)(int ecode) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL getnameinfo)(const struct sockaddr *__restrict sa, socklen_t salen, char *__restrict host, socklen_t __hostlen, char *__restrict __serv, socklen_t __servlen, int __flags) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL getaddrinfo_a)(int mode, struct gaicb *list[__restrict_arr], int ent, struct sigevent *__restrict sig) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL gai_suspend)(const struct gaicb *const list[], int ent, struct timespec const *timeout) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL gai_error)(struct gaicb *req) { NOT_IMPLEMENTED(); return 0; }
PUBLIC int (LIBCCALL gai_cancel)(struct gaicb *gaicbp) { NOT_IMPLEMENTED(); return 0; }

DECL_END

#endif /* !GUARD_LIBS_LIBC_INET_NETDB_C */
