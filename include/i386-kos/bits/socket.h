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
#ifndef _BITS_SOCKET_H
#define _BITS_SOCKET_H 1
#ifndef __BITS_SOCKET_H
#define __BITS_SOCKET_H 1

/* System-specific socket constants and types.  Linux version.
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
#include <sys/types.h>
#include <bits/types.h>
#include <bits/socket_type.h>
#include <bits/sockaddr.h>
#include <asm/socket.h>

__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */
#ifndef __socklen_t_defined
#define __socklen_t_defined 1
typedef __socklen_t socklen_t;
#endif /* !__socklen_t_defined */

/* Protocol families. */
#define PF_UNSPEC       0  /*< Unspecified. */
#define PF_LOCAL        1  /*< Local to host (pipes and file-domain). */
#define PF_UNIX         PF_LOCAL /*< POSIX name for PF_LOCAL. */
#define PF_FILE         PF_LOCAL /*< Another non-standard name for PF_LOCAL. */
#define PF_INET         2  /*< IP protocol family. */
#define PF_AX25         3  /*< Amateur Radio AX.25. */
#define PF_IPX          4  /*< Novell Internet Protocol. */
#define PF_APPLETALK    5  /*< Appletalk DDP. */
#define PF_NETROM       6  /*< Amateur radio NetROM. */
#define PF_BRIDGE       7  /*< Multiprotocol bridge. */
#define PF_ATMPVC       8  /*< ATM PVCs. */
#define PF_X25          9  /*< Reserved for X.25 project. */
#define PF_INET6        10 /*< IP version 6. */
#define PF_ROSE         11 /*< Amateur Radio X.25 PLP. */
#define PF_DECnet       12 /*< Reserved for DECnet project. */
#define PF_NETBEUI      13 /*< Reserved for 802.2LLC project. */
#define PF_SECURITY     14 /*< Security callback pseudo AF. */
#define PF_KEY          15 /*< PF_KEY key management API. */
#define PF_NETLINK      16
#define PF_ROUTE        PF_NETLINK /*< Alias to emulate 4.4BSD. */
#define PF_PACKET       17 /*< Packet family. */
#define PF_ASH          18 /*< Ash. */
#define PF_ECONET       19 /*< Acorn Econet. */
#define PF_ATMSVC       20 /*< ATM SVCs. */
#define PF_RDS          21 /*< RDS sockets. */
#define PF_SNA          22 /*< Linux SNA Project */
#define PF_IRDA         23 /*< IRDA sockets. */
#define PF_PPPOX        24 /*< PPPoX sockets. */
#define PF_WANPIPE      25 /*< Wanpipe API sockets. */
#define PF_LLC          26 /*< Linux LLC. */
#define PF_IB           27 /*< Native InfiniBand address. */
#define PF_MPLS         28 /*< MPLS. */
#define PF_CAN          29 /*< Controller Area Network. */
#define PF_TIPC         30 /*< TIPC sockets. */
#define PF_BLUETOOTH    31 /*< Bluetooth sockets. */
#define PF_IUCV         32 /*< IUCV sockets. */
#define PF_RXRPC        33 /*< RxRPC sockets. */
#define PF_ISDN         34 /*< mISDN sockets. */
#define PF_PHONET       35 /*< Phonet sockets. */
#define PF_IEEE802154   36 /*< IEEE 802.15.4 sockets. */
#define PF_CAIF         37 /*< CAIF sockets. */
#define PF_ALG          38 /*< Algorithm sockets. */
#define PF_NFC          39 /*< NFC sockets. */
#define PF_VSOCK        40 /*< vSockets. */
#define PF_MAX          41 /*< For now.. */

/* Address families. */
#define AF_UNSPEC       PF_UNSPEC
#define AF_LOCAL        PF_LOCAL
#define AF_UNIX         PF_UNIX
#define AF_FILE         PF_FILE
#define AF_INET         PF_INET
#define AF_AX25         PF_AX25
#define AF_IPX          PF_IPX
#define AF_APPLETALK    PF_APPLETALK
#define AF_NETROM       PF_NETROM
#define AF_BRIDGE       PF_BRIDGE
#define AF_ATMPVC       PF_ATMPVC
#define AF_X25          PF_X25
#define AF_INET6        PF_INET6
#define AF_ROSE         PF_ROSE
#define AF_DECnet       PF_DECnet
#define AF_NETBEUI      PF_NETBEUI
#define AF_SECURITY     PF_SECURITY
#define AF_KEY          PF_KEY
#define AF_NETLINK      PF_NETLINK
#define AF_ROUTE        PF_ROUTE
#define AF_PACKET       PF_PACKET
#define AF_ASH          PF_ASH
#define AF_ECONET       PF_ECONET
#define AF_ATMSVC       PF_ATMSVC
#define AF_RDS          PF_RDS
#define AF_SNA          PF_SNA
#define AF_IRDA         PF_IRDA
#define AF_PPPOX        PF_PPPOX
#define AF_WANPIPE      PF_WANPIPE
#define AF_LLC          PF_LLC
#define AF_IB           PF_IB
#define AF_MPLS         PF_MPLS
#define AF_CAN          PF_CAN
#define AF_TIPC         PF_TIPC
#define AF_BLUETOOTH    PF_BLUETOOTH
#define AF_IUCV         PF_IUCV
#define AF_RXRPC        PF_RXRPC
#define AF_ISDN         PF_ISDN
#define AF_PHONET       PF_PHONET
#define AF_IEEE802154   PF_IEEE802154
#define AF_CAIF         PF_CAIF
#define AF_ALG          PF_ALG
#define AF_NFC          PF_NFC
#define AF_VSOCK        PF_VSOCK
#define AF_MAX          PF_MAX

/* Socket level values. Others are defined in the appropriate headers.
 * XXX These definitions also should go into the appropriate headers as
 * far as they are available. */
#define SOL_RAW         255
#define SOL_DECNET      261
#define SOL_X25         262
#define SOL_PACKET      263
#define SOL_ATM         264 /*< ATM layer (cell level). */
#define SOL_AAL         265 /*< ATM Adaption Layer (packet level). */
#define SOL_IRDA        266
#define SOMAXCONN       128 /*< Maximum queue length specifiable by listen. */

/* Structure describing a generic socket address. */
struct sockaddr {
    __SOCKADDR_COMMON(sa_); /*< Common data: address family and length. */
    char sa_data[14];       /*< Address data. */
};

/* Structure large enough to hold any socket address (with the historical exception of AF_UNIX). */
#define __ss_aligntype   unsigned long int
#define _SS_PADSIZE     (_SS_SIZE-__SOCKADDR_COMMON_SIZE-sizeof (__ss_aligntype))

struct sockaddr_storage {
    __SOCKADDR_COMMON(ss_);    /*< Address family, etc. */
    char           __ss_padding[_SS_PADSIZE];
    __ss_aligntype __ss_align; /*< Force desired alignment. */
};


/* Bits in the FLAGS argument to `send', `recv', et al. */
enum {
    MSG_OOB          = 0x00000001, /*< Process out-of-band data. */
    MSG_PEEK         = 0x00000002, /*< Peek at incoming messages. */
    MSG_DONTROUTE    = 0x00000004, /*< Don't use local routing. */
#ifdef __USE_GNU
    MSG_TRYHARD      = MSG_DONTROUTE, /*< DECnet uses a different name. */
#endif
    MSG_CTRUNC       = 0x00000008, /*< Control data lost before delivery. */
    MSG_PROXY        = 0x00000010, /*< Supply or ask second address. */
    MSG_TRUNC        = 0x00000020,
    MSG_DONTWAIT     = 0x00000040, /*< Nonblocking IO. */
    MSG_EOR          = 0x00000080, /*< End of record. */
    MSG_WAITALL      = 0x00000100, /*< Wait for a full request. */
    MSG_FIN          = 0x00000200,
    MSG_SYN          = 0x00000400,
    MSG_CONFIRM      = 0x00000800, /*< Confirm path validity. */
    MSG_RST          = 0x00001000,
    MSG_ERRQUEUE     = 0x00002000, /*< Fetch message from error queue. */
    MSG_NOSIGNAL     = 0x00004000, /*< Do not generate SIGPIPE. */
    MSG_MORE         = 0x00008000, /*< Sender will send more. */
    MSG_WAITFORONE   = 0x00010000, /*< Wait for at least one packet to return.*/
    MSG_FASTOPEN     = 0x20000000, /*< Send data in TCP SYN. */
    MSG_CMSG_CLOEXEC = 0x40000000, /*< Set close_on_exit for file descriptor received through SCM_RIGHTS. */
};

#define MSG_OOB          MSG_OOB
#define MSG_PEEK         MSG_PEEK
#define MSG_DONTROUTE    MSG_DONTROUTE
#ifdef __USE_GNU
#define MSG_TRYHARD      MSG_DONTROUTE
#endif
#define MSG_CTRUNC       MSG_CTRUNC
#define MSG_PROXY        MSG_PROXY
#define MSG_TRUNC        MSG_TRUNC
#define MSG_DONTWAIT     MSG_DONTWAIT
#define MSG_EOR          MSG_EOR
#define MSG_WAITALL      MSG_WAITALL
#define MSG_FIN          MSG_FIN
#define MSG_SYN          MSG_SYN
#define MSG_CONFIRM      MSG_CONFIRM
#define MSG_RST          MSG_RST
#define MSG_ERRQUEUE     MSG_ERRQUEUE
#define MSG_NOSIGNAL     MSG_NOSIGNAL
#define MSG_MORE         MSG_MORE
#define MSG_WAITFORONE   MSG_WAITFORONE
#define MSG_FASTOPEN     MSG_FASTOPEN
#define MSG_CMSG_CLOEXEC MSG_CMSG_CLOEXEC

/* Structure describing messages sent by `sendmsg' and received by `recvmsg'. */
struct msghdr {
    void         *msg_name;       /*< Address to send to/receive from. */
    socklen_t     msg_namelen;    /*< Length of address data. */
    struct iovec *msg_iov;        /*< Vector of data to send/receive into. */
    size_t        msg_iovlen;     /*< Number of elements in the vector. */
    void         *msg_control;    /*< Ancillary data (eg BSD filedesc passing). */
    size_t        msg_controllen; /*< Ancillary data buffer length. !! The type should be socklen_t but the definition of the kernel is incompatible with this. */
    int           msg_flags;      /*< Flags on received message. */
};

/* Structure used for storage of ancillary data object information. */
struct cmsghdr {
    size_t          cmsg_len;     /*< Length of data in cmsg_data plus length of cmsghdr structure. !! The type should be socklen_t but the definition of the kernel is incompatible with this. */
    int             cmsg_level;   /*< Originating protocol. */
    int             cmsg_type;    /*< Protocol specific type. */
    unsigned char __cmsg_data[1]; /*< Ancillary data. */
};

/* Ancillary data object manipulation macros. */
#define CMSG_DATA(cmsg)        ((cmsg)->__cmsg_data)
#define CMSG_FIRSTHDR(mhdr)    ((size_t)(mhdr)->msg_controllen >= sizeof(struct cmsghdr) ? (struct cmsghdr *)(mhdr)->msg_control : (struct cmsghdr *)0)
#define CMSG_ALIGN(len)       (((len)+sizeof(size_t)-1) & (size_t)~(sizeof(size_t)-1))
#define CMSG_SPACE(len)        (CMSG_ALIGN(len)+CMSG_ALIGN(sizeof(struct cmsghdr)))
#define CMSG_LEN(len)          (CMSG_ALIGN(sizeof(struct cmsghdr))+(len))
#define CMSG_NXTHDR(mhdr,cmsg)   __cmsg_nxthdr(mhdr, cmsg)
#ifdef __KERNEL__
__LOCAL struct cmsghdr *(__LIBCCALL __cmsg_nxthdr)(struct msghdr *__mhdr, struct cmsghdr *__cmsg) {
    if ((size_t)__cmsg->cmsg_len < sizeof(struct cmsghdr)) return (struct cmsghdr *) 0;
    __cmsg = (struct cmsghdr *)((unsigned char *)__cmsg+CMSG_ALIGN(__cmsg->cmsg_len));
    if ((unsigned char *)(__cmsg+1) >((unsigned char *)__mhdr->msg_control+__mhdr->msg_controllen) ||
       ((unsigned char *) __cmsg+CMSG_ALIGN(__cmsg->cmsg_len) >
       ((unsigned char *) __mhdr->msg_control+__mhdr->msg_controllen)))
         return (struct cmsghdr *)0;
    return __cmsg;
}
#else
__LIBC struct cmsghdr *(__LIBCCALL __cmsg_nxthdr)(struct msghdr *__mhdr, struct cmsghdr *__cmsg);
#endif

enum {
    SCM_RIGHTS      = 0x01, /*< Transfer file descriptors. */
#ifdef __USE_GNU
    SCM_CREDENTIALS = 0x02  /*< Credentials passing. */
#endif /* __USE_GNU */
};

#define SCM_RIGHTS      SCM_RIGHTS
#ifdef __USE_GNU
#define SCM_CREDENTIALS SCM_CREDENTIALS
#endif

#ifdef __USE_GNU
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("pid")
#pragma push_macro("uid")
#pragma push_macro("gid")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef pid
#undef uid
#undef gid

/* User visible structure for SCM_CREDENTIALS message */
struct ucred {
    pid_t pid; /*< PID of sending process. */
    uid_t uid; /*< UID of sending process. */
    gid_t gid; /*< GID of sending process. */
};

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("gid")
#pragma pop_macro("uid")
#pragma pop_macro("pid")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif

struct linger {
    int l_onoff;  /*< Nonzero to linger on close. */
    int l_linger; /*< Time to linger. */
};

__SYSDECL_END

#endif /* !__BITS_SOCKET_H */
#endif /* !_BITS_SOCKET_H */
