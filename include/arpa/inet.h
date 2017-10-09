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
#ifndef _ARPA_INET_H
#define _ARPA_INET_H 1

#include <features.h>
#include <bits/types.h>
#include <netinet/in.h>

__SYSDECL_BEGIN

#ifndef __socklen_t_defined
#define __socklen_t_defined 1
typedef __socklen_t socklen_t;
#endif /* !__socklen_t_defined */

#ifndef __KERNEL__
__LIBC in_addr_t (__LIBCCALL inet_addr)(char const *__cp);
__LIBC in_addr_t (__LIBCCALL inet_lnaof)(struct in_addr __in);
__LIBC struct in_addr (__LIBCCALL inet_makeaddr)(in_addr_t __net, in_addr_t __host);
__LIBC in_addr_t (__LIBCCALL inet_netof)(struct in_addr __in);
__LIBC in_addr_t (__LIBCCALL inet_network)(char const *__cp);
__LIBC char *(__LIBCCALL inet_ntoa)(struct in_addr __in);
__LIBC int (__LIBCCALL inet_pton)(int __af, char const *__restrict __cp, void *__restrict __buf);
__LIBC char const *(__LIBCCALL inet_ntop)(int __af, void const *__restrict __cp, char *__restrict __buf, socklen_t __len);
#ifdef __USE_MISC
__LIBC int (__LIBCCALL inet_aton)(char const *__cp, struct in_addr *__inp);
__LIBC char *(__LIBCCALL inet_neta)(in_addr_t __net, char *__buf, size_t __len);
__LIBC char *(__LIBCCALL inet_net_ntop)(int __af, void const *__cp, int __bits, char *__buf, size_t __len);
__LIBC int (__LIBCCALL inet_net_pton)(int __af, char const *__cp, void *__buf, size_t __len);
__LIBC unsigned int (__LIBCCALL inet_nsap_addr)(char const *__cp, unsigned char *__buf, int __len);
__LIBC char *(__LIBCCALL inet_nsap_ntoa)(int __len, const unsigned char *__cp, char *__buf);
#endif /* __USE_MISC */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_ARPA_INET_H */
