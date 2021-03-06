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
#ifndef _I386_KOS_BITS_IPCTYPES_H
#define _I386_KOS_BITS_IPCTYPES_H 1
#define _BITS_IPCTYPES_H 1

#include <hybrid/host.h>
#include <hybrid/typecore.h>

/* Used by `struct shmid_ds'. */
#ifdef __x86_64__
typedef __INT32_TYPE__ __ipc_pid_t;
#else /* __x86_64__ */
typedef __INT16_TYPE__ __ipc_pid_t;
#endif /* !__x86_64__ */

#endif /* !_I386_KOS_BITS_IPCTYPES_H */
