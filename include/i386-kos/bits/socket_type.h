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
#ifndef _I386_KOS_BITS_SOCKET_TYPE_H
#define _I386_KOS_BITS_SOCKET_TYPE_H 1
#define _BITS_SOCKET_TYPE_H 1

#include <__stdinc.h>

/* Define enum __socket_type for generic Linux.
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

__SYSDECL_BEGIN

/* Types of sockets.  */
enum __socket_type {
    SOCK_STREAM    = 1,  /*< Sequenced, reliable, connection-based byte streams.  */
    SOCK_DGRAM     = 2,  /*< Connectionless, unreliable datagrams of fixed maximum length.  */
    SOCK_RAW       = 3,  /*< Raw protocol interface.  */
    SOCK_RDM       = 4,  /*< Reliably-delivered messages.  */
    SOCK_SEQPACKET = 5,  /*< Sequenced, reliable, connection-based, datagrams of fixed maximum length.  */
    SOCK_DCCP      = 6,  /*< Datagram Congestion Control Protocol.  */
    SOCK_PACKET    = 10, /*< Linux specific way of getting packets at the dev level.  For writing rarp and other similar things on the user level. */
    /* Flags to be ORed into the type parameter of socket and
     * socketpair and used for the flags parameter of paccept.  */
    SOCK_CLOEXEC   = 02000000, /*< Atomically set close-on-exec flag for the new descriptor(s).  */
    SOCK_NONBLOCK  = 00004000, /*< Atomically mark descriptor(s) as non-blocking.  */
};

#define SOCK_STREAM    SOCK_STREAM
#define SOCK_DGRAM     SOCK_DGRAM
#define SOCK_RAW       SOCK_RAW
#define SOCK_RDM       SOCK_RDM
#define SOCK_SEQPACKET SOCK_SEQPACKET
#define SOCK_DCCP      SOCK_DCCP
#define SOCK_PACKET    SOCK_PACKET
#define SOCK_CLOEXEC   SOCK_CLOEXEC
#define SOCK_NONBLOCK  SOCK_NONBLOCK

__SYSDECL_END

#endif /* !_I386_KOS_BITS_SOCKET_TYPE_H */
