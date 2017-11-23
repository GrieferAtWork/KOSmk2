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
#ifndef _NETDB_H
#define _NETDB_H 1

/* Disclaimer: Comments are based on those found in /usr/include/netdb.h.
 * The following is the original copyright notice found in that file: */

/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
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
#include <stdint.h>
#include <netinet/in.h>
#include <bits/netdb.h>
#ifdef __USE_MISC
#include <rpc/netdb.h>
#endif /* __USE_MISC */
#ifdef __USE_GNU
#include <bits/sigevent.h>
#include <hybrid/timespec.h>
#endif /* __USE_GNU */

#if !defined(__CRT_GLC) && !defined(__CRT_CYG)
#error "<netdb.h> is not supported by the linked libc"
#endif /* !__CRT_GLC && !__CRT_CYG */

__SYSDECL_BEGIN

struct timespec;

/* Absolute file name for network data base files. */
#define _PATH_HEQUIV        "/etc/hosts.equiv"
#define _PATH_HOSTS         "/etc/hosts"
#define _PATH_NETWORKS      "/etc/networks"
#define _PATH_NSSWITCH_CONF "/etc/nsswitch.conf"
#define _PATH_PROTOCOLS     "/etc/protocols"
#define _PATH_SERVICES      "/etc/services"

#if defined(__USE_MISC) || !defined(__USE_XOPEN2K8)
#ifndef __KERNEL__
#define h_errno                    (*__h_errno_location())
__LIBC __ATTR_CONST int *(__LIBCCALL __h_errno_location)(void);
#endif /* !__KERNEL__ */
#define HOST_NOT_FOUND  1 /* Authoritative Answer Host not found. */
#define TRY_AGAIN       2 /* Non-Authoritative Host not found, or SERVERFAIL. */
#define NO_RECOVERY     3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP. */
#define NO_DATA         4 /* Valid name, no data record of requested type. */
#endif
#ifdef __USE_MISC
#define NETDB_INTERNAL (-1)      /*< See errno. */
#define NETDB_SUCCESS    0       /*< No problem. */
#define NO_ADDRESS       NO_DATA /*< No address, look for MX record. */
#endif
#if defined __USE_XOPEN2K || defined __USE_XOPEN_EXTENDED
#define IPPORT_RESERVED    1024  /*< Highest reserved Internet port number. */
#endif

#ifdef __USE_GNU
#define SCOPE_DELIMITER    '%'   /*< Scope delimiter for getaddrinfo(), getnameinfo(). */
#endif

#ifdef __USE_MISC
#ifndef __KERNEL__
__LIBC void (__LIBCCALL herror)(char const *__str);
#endif /* !__KERNEL__ */
__LIBC char const *(__LIBCCALL hstrerror)(int __err_num);
#endif

/* Description of data base entry for a single host. */
struct hostent {
  char  *h_name;               /*< Official name of host. */
  char **h_aliases;            /*< Alias list. */
  int    h_addrtype;           /*< Host address type. */
  int    h_length;             /*< Length of address. */
  char **h_addr_list;          /*< List of addresses from name server. */
#ifdef __USE_MISC
# define h_addr h_addr_list[0] /*< Address, for backward compatibility.*/
#endif
};

#ifndef __KERNEL__
__LIBC void (__LIBCCALL sethostent)(int __stay_open);
__LIBC void (__LIBCCALL endhostent)(void);
__LIBC struct hostent *(__LIBCCALL gethostent)(void);
__LIBC struct hostent *(__LIBCCALL gethostbyaddr)(void const *__addr, __socklen_t __len, int __type);
__LIBC struct hostent *(__LIBCCALL gethostbyname)(char const *__name);
#ifdef __USE_MISC
__LIBC struct hostent *(__LIBCCALL gethostbyname2)(char const *__name, int __af);
__LIBC int (__LIBCCALL gethostent_r)(struct hostent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct hostent **__restrict __result, int *__restrict __h_errnop);
__LIBC int (__LIBCCALL gethostbyaddr_r)(void const *__restrict __addr, __socklen_t __len, int __type, struct hostent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct hostent **__restrict __result, int *__restrict __h_errnop);
__LIBC int (__LIBCCALL gethostbyname_r)(char const *__restrict __name, struct hostent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct hostent **__restrict __result, int *__restrict __h_errnop);
__LIBC int (__LIBCCALL gethostbyname2_r)(char const *__restrict __name, int __af, struct hostent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct hostent **__restrict __result, int *__restrict __h_errnop);
#endif /* __USE_MISC */
__LIBC void (__LIBCCALL setnetent)(int __stay_open);
__LIBC void (__LIBCCALL endnetent)(void);
__LIBC struct netent *(__LIBCCALL getnetent)(void);
__LIBC struct netent *(__LIBCCALL getnetbyaddr)(uint32_t __net, int __type);
__LIBC struct netent *(__LIBCCALL getnetbyname)(char const *__name);
#ifdef __USE_MISC
__LIBC int (__LIBCCALL getnetent_r)(struct netent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct netent **__restrict __result, int *__restrict __h_errnop);
__LIBC int (__LIBCCALL getnetbyaddr_r)(uint32_t __net, int __type, struct netent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct netent **__restrict __result, int *__restrict __h_errnop);
__LIBC int (__LIBCCALL getnetbyname_r)(char const *__restrict __name, struct netent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct netent **__restrict __result, int *__restrict __h_errnop);
#endif /* __USE_MISC */
#endif /* !__KERNEL__ */

struct servent {
  char  *s_name;    /*< Official service name. */
  char **s_aliases; /*< Alias list. */
  int    s_port;    /*< Port number. */
  char  *s_proto;   /*< Protocol to use. */
};

#ifndef __KERNEL__
__LIBC void (__LIBCCALL setservent)(int __stay_open);
__LIBC void (__LIBCCALL endservent)(void);
__LIBC struct servent *(__LIBCCALL getservent)(void);
__LIBC struct servent *(__LIBCCALL getservbyname)(char const *__name, char const *__proto);
__LIBC struct servent *(__LIBCCALL getservbyport)(int __port, char const *__proto);
#ifdef __USE_MISC
__LIBC int (__LIBCCALL getservent_r)(struct servent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct servent **__restrict __result);
__LIBC int (__LIBCCALL getservbyname_r)(char const *__restrict __name, char const *__restrict __proto, struct servent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct servent **__restrict __result);
__LIBC int (__LIBCCALL getservbyport_r)(int __port, char const *__restrict __proto, struct servent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct servent **__restrict __result);
#endif /* __USE_MISC */
#endif /* !__KERNEL__ */

struct protoent {
  char  *p_name;    /*< Official protocol name. */
  char **p_aliases; /*< Alias list. */
  int    p_proto;   /*< Protocol number. */
};

#ifndef __KERNEL__
__LIBC void (__LIBCCALL setprotoent)(int __stay_open);
__LIBC void (__LIBCCALL endprotoent)(void);
__LIBC struct protoent *(__LIBCCALL getprotoent)(void);
__LIBC struct protoent *(__LIBCCALL getprotobyname)(char const *__name);
__LIBC struct protoent *(__LIBCCALL getprotobynumber)(int __proto);
#ifdef __USE_MISC
__LIBC int (__LIBCCALL getprotoent_r)(struct protoent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct protoent **__restrict __result);
__LIBC int (__LIBCCALL getprotobyname_r)(char const *__restrict __name, struct protoent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct protoent **__restrict __result);
__LIBC int (__LIBCCALL getprotobynumber_r)(int __proto, struct protoent *__restrict __result_buf, char *__restrict __buf, size_t __buflen, struct protoent **__restrict __result);
__LIBC int (__LIBCCALL setnetgrent)(char const *__netgroup);
__LIBC void (__LIBCCALL endnetgrent)(void);
__LIBC int (__LIBCCALL getnetgrent)(char **__restrict __hostp, char **__restrict __userp, char **__restrict __domainp);
__LIBC int (__LIBCCALL innetgr)(char const *__netgroup, char const *__host, char const *__user, char const *__domain);
__LIBC int (__LIBCCALL getnetgrent_r)(char **__restrict __hostp, char **__restrict __userp, char **__restrict __domainp, char *__restrict __buffer, size_t __buflen);
__LIBC int (__LIBCCALL rcmd)(char **__restrict __ahost, unsigned short int __rport, char const *__restrict __locuser, char const *__restrict __remuser, char const *__restrict __cmd, int *__restrict __fd2p);
__LIBC int (__LIBCCALL rcmd_af)(char **__restrict __ahost, unsigned short int __rport, char const *__restrict __locuser, char const *__restrict __remuser, char const *__restrict __cmd, int *__restrict __fd2p, sa_family_t __af);
__LIBC int (__LIBCCALL rexec)(char **__restrict __ahost, int __rport, char const *__restrict __name, char const *__restrict __pass, char const *__restrict __cmd, int *__restrict __fd2p);
__LIBC int (__LIBCCALL rexec_af)(char **__restrict __ahost, int __rport, char const *__restrict __name, char const *__restrict __pass, char const *__restrict __cmd, int *__restrict __fd2p, sa_family_t __af);
__LIBC int (__LIBCCALL ruserok)(char const *__rhost, int __suser, char const *__remuser, char const *__locuser);
__LIBC int (__LIBCCALL ruserok_af)(char const *__rhost, int __suser, char const *__remuser, char const *__locuser, sa_family_t __af);
__LIBC int (__LIBCCALL iruserok)(uint32_t __raddr, int __suser, char const *__remuser, char const *__locuser);
__LIBC int (__LIBCCALL iruserok_af)(void const *__raddr, int __suser, char const *__remuser, char const *__locuser, sa_family_t __af);
__LIBC int (__LIBCCALL rresvport)(int *__alport);
__LIBC int (__LIBCCALL rresvport_af)(int *__alport, sa_family_t __af);
#endif /* __USE_MISC */
#endif /* !__KERNEL__ */

#ifdef __USE_XOPEN2K
struct addrinfo {
  int              ai_flags;     /*< Input flags. */
  int              ai_family;    /*< Protocol family for socket. */
  int              ai_socktype;  /*< Socket type. */
  int              ai_protocol;  /*< Protocol for socket. */
  socklen_t        ai_addrlen;   /*< Length of socket address. */
  struct sockaddr *ai_addr;      /*< Socket address for socket. */
  char            *ai_canonname; /*< Canonical name for service location. */
  struct addrinfo *ai_next;      /*< Pointer to next in list. */
};

#ifdef __USE_GNU
struct gaicb {
  char const            *ar_name;    /*< Name to look up. */
  char const            *ar_service; /*< Service name. */
  struct addrinfo const *ar_request; /*< Additional request specification. */
  struct addrinfo       *ar_result;  /*< Pointer to result. */
  int __return;
  int __glibc_reserved[5];
};
#define GAI_WAIT   0
#define GAI_NOWAIT 1
#endif /* __USE_GNU */

/* Possible values for `ai_flags' field in `addrinfo' structure. */
#define AI_PASSIVE     0x0001 /*< Socket address is intended for `bind'. */
#define AI_CANONNAME   0x0002 /*< Request for canonical name. */
#define AI_NUMERICHOST 0x0004 /*< Don't use name resolution. */
#define AI_V4MAPPED    0x0008 /*< IPv4 mapped addresses are acceptable. */
#define AI_ALL         0x0010 /*< Return IPv4 mapped and IPv6 addresses. */
#define AI_ADDRCONFIG  0x0020 /*< Use configuration of this host to choose returned address type.. */
#ifdef __USE_GNU
#   define AI_IDN      0x0040 /*< IDN encode input (assuming it is encoded in the current locale's character set) before looking it up. */
#   define AI_CANONIDN 0x0080 /*< Translate canonical name from IDN format. */
#   define AI_IDN_ALLOW_UNASSIGNED 0x0100 /*< Don't reject unassigned Unicode code points. */
#   define AI_IDN_USE_STD3_ASCII_RULES 0x0200 /*< Validate strings according to STD3 rules. */
#endif
#define AI_NUMERICSERV 0x0400 /*< Don't use name resolution. */

/* Error values for `getaddrinfo' function. */
#define EAI_BADFLAGS         (-1) /*< Invalid value for `ai_flags' field. */
#define EAI_NONAME           (-2) /*< NAME or SERVICE is unknown. */
#define EAI_AGAIN            (-3) /*< Temporary failure in name resolution. */
#define EAI_FAIL             (-4) /*< Non-recoverable failure in name res. */
#define EAI_FAMILY           (-6) /*< `ai_family' not supported. */
#define EAI_SOCKTYPE         (-7) /*< `ai_socktype' not supported. */
#define EAI_SERVICE          (-8) /*< SERVICE not supported for `ai_socktype'. */
#define EAI_MEMORY          (-10) /*< Memory allocation failure. */
#define EAI_SYSTEM          (-11) /*< System error returned in `errno'. */
#define EAI_OVERFLOW        (-12) /*< Argument buffer overflow. */
#ifdef __USE_GNU
#   define EAI_NODATA        (-5) /*< No address associated with NAME. */
#   define EAI_ADDRFAMILY    (-9) /*< Address family for NAME not supported. */
#   define EAI_INPROGRESS  (-100) /*< Processing request in progress. */
#   define EAI_CANCELED    (-101) /*< Request canceled. */
#   define EAI_NOTCANCELED (-102) /*< Request not canceled. */
#   define EAI_ALLDONE     (-103) /*< All requests done. */
#   define EAI_INTR        (-104) /*< Interrupted by a signal. */
#   define EAI_IDN_ENCODE  (-105) /*< IDN encoding failed. */
#endif
#ifdef __USE_MISC
#   define NI_MAXHOST      1025
#   define NI_MAXSERV      32
#endif
#define NI_NUMERICHOST     1  /*< Don't try to look up hostname. */
#define NI_NUMERICSERV     2  /*< Don't convert port number to name. */
#define NI_NOFQDN          4  /*< Only return nodename portion. */
#define NI_NAMEREQD        8  /*< Don't return numeric addresses. */
#define NI_DGRAM           16 /*< Look up UDP service rather than TCP. */
#ifdef __USE_GNU
#   define NI_IDN          32 /*< Convert name from IDN format. */
#   define NI_IDN_ALLOW_UNASSIGNED 64 /*< Don't reject unassigned Unicode code points. */
#   define NI_IDN_USE_STD3_ASCII_RULES 128 /*< Validate strings according to STD3 rules. */
#endif

#ifndef __KERNEL__
__LIBC int (__LIBCCALL getaddrinfo)(char const *__restrict __name, char const *__restrict __service, struct addrinfo const *__restrict __req, struct addrinfo **__restrict __pai);
__LIBC void (__LIBCCALL freeaddrinfo)(struct addrinfo *__ai);
__LIBC char const *(__LIBCCALL gai_strerror)(int __ecode);
__LIBC int (__LIBCCALL getnameinfo)(struct sockaddr const *__restrict __sa, socklen_t __salen, char *__restrict __host, socklen_t __hostlen, char *__restrict __serv, socklen_t __servlen, int __flags);
#endif /* !__KERNEL__ */
#endif /* __USE_XOPEN2K */

#ifndef __KERNEL__
#ifdef __USE_GNU
__LIBC int (__LIBCCALL getaddrinfo_a)(int __mode, struct gaicb *__list[__restrict_arr], int __ent, struct sigevent *__restrict __sig);
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,gai_suspend,(struct gaicb const *const __list[], int __ent, struct timespec const *__timeout),gai_suspend,(__list,__ent,__timeout))
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL gai_suspend64)(struct gaicb const *const __list[], int __ent, struct __timespec64 const *__timeout);
#endif /* __USE_TIME64 */
__LIBC int (__LIBCCALL gai_error)(struct gaicb *__req);
__LIBC int (__LIBCCALL gai_cancel)(struct gaicb *__gaicbp);
#endif /* __USE_GNU */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_NETDB_H */
