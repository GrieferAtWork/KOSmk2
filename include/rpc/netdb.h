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
#ifndef _RPC_NETDB_H
#define _RPC_NETDB_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

struct rpcent {
    char  *r_name;    /*< Name of server for this rpc program. */
    char **r_aliases; /*< Alias list. */
    int    r_number;  /*< RPC program number. */
};

#ifndef __KERNEL__
__LIBC void (__LIBCCALL setrpcent)(int __stayopen);
__LIBC void (__LIBCCALL endrpcent)(void);
__LIBC struct rpcent *(__LIBCCALL getrpcbyname)(char const *__name);
__LIBC struct rpcent *(__LIBCCALL getrpcbynumber)(int __number);
__LIBC struct rpcent *(__LIBCCALL getrpcent)(void);
#ifdef __USE_MISC
__LIBC int (__LIBCCALL getrpcbyname_r)(char const *__name, struct rpcent *__result_buf, char *__buffer, size_t __buflen, struct rpcent **__result);
__LIBC int (__LIBCCALL getrpcbynumber_r)(int __number, struct rpcent *__result_buf, char *__buffer, size_t __buflen, struct rpcent **__result);
__LIBC int (__LIBCCALL getrpcent_r)(struct rpcent *__result_buf, char *__buffer, size_t __buflen, struct rpcent **__result);
#endif /* __USE_MISC */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_RPC_NETDB_H */
