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
#ifndef _NETINET_ETHER_H
#define _NETINET_ETHER_H 1

#include <features.h>
#include <netinet/if_ether.h>

__SYSDECL_BEGIN

#ifndef __KERNEL__
__LIBC char *(__LIBCCALL ether_ntoa)(struct ether_addr const *__addr);
__LIBC char *(__LIBCCALL ether_ntoa_r)(struct ether_addr const *__addr, char *__buf);
__LIBC struct ether_addr *(__LIBCCALL ether_aton)(char const *__asc);
__LIBC struct ether_addr *(__LIBCCALL ether_aton_r)(char const *__asc, struct ether_addr *__addr);
__LIBC int (__LIBCCALL ether_ntohost)(char *__hostname, struct ether_addr const *__addr);
__LIBC int (__LIBCCALL ether_hostton)(char const *__hostname, struct ether_addr *__addr);
__LIBC int (__LIBCCALL ether_line)(char const *__line, struct ether_addr *__addr, char *__hostname);
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_NETINET_ETHER_H */
