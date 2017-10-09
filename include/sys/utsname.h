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
#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H 1

#include <features.h>
#include <bits/utsname.h>
#ifdef __USE_KOS
#include <kos/ksym.h>
#endif /* __USE_KOS */

__SYSDECL_BEGIN

#ifndef _UTSNAME_SYSNAME_LENGTH
#define _UTSNAME_SYSNAME_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_NODENAME_LENGTH
#define _UTSNAME_NODENAME_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_RELEASE_LENGTH
#define _UTSNAME_RELEASE_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_VERSION_LENGTH
#define _UTSNAME_VERSION_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_MACHINE_LENGTH
#define _UTSNAME_MACHINE_LENGTH _UTSNAME_LENGTH
#endif

struct utsname {
 char sysname[_UTSNAME_SYSNAME_LENGTH];     /*< Name of the implementation of the operating system. */
 char nodename[_UTSNAME_NODENAME_LENGTH];   /*< Name of this node on the network. */
 char release[_UTSNAME_RELEASE_LENGTH];     /*< Current release level of this implementation. */
 char version[_UTSNAME_VERSION_LENGTH];     /*< Current version level of this release. */
 char machine[_UTSNAME_MACHINE_LENGTH];     /*< Name of the hardware type the system is running on. */
#if (_UTSNAME_DOMAIN_LENGTH+0) != 0
#ifdef __USE_GNU
 char domainname[_UTSNAME_DOMAIN_LENGTH];   /*< Name of the domain of this node on the network. */
#else
 char __domainname[_UTSNAME_DOMAIN_LENGTH]; /*< Name of the domain of this node on the network. */
#endif
#endif
};

#ifdef __USE_KOS
/* System-global kernel variable (May be accessed directly). */
__PUBDEF struct utsname const active_uname __KSYM(uname);
#endif /* __USE_KOS */

#ifdef __USE_MISC
#define SYS_NMLN  _UTSNAME_LENGTH
#endif /* __USE_MISC */

#ifndef __KERNEL__
__LIBC int (__LIBCCALL uname)(struct utsname *__name);
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_SYS_UTSNAME_H */
