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
#ifndef _SYS_GENERIC_SHM_H
#define _SYS_GENERIC_SHM_H 1
#define _SYS_SHM_H 1

#include <features.h>
#include <sys/ipc.h>
#include <bits/shm.h>

__SYSDECL_BEGIN

#ifdef __USE_XOPEN
#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif /* __pid_t_defined */
#endif /* __USE_XOPEN */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef __time_t_defined
#define __time_t_defined 1
typedef __time_t time_t;
#endif /* !__time_t_defined */

__LIBC int __NOTHROW((__LIBCCALL shmctl)(int __shmid, int __cmd, struct shmid_ds *__buf));
__LIBC int __NOTHROW((__LIBCCALL shmget)(key_t __key, size_t __size, int __shmflg));
__LIBC void *__NOTHROW((__LIBCCALL shmat)(int __shmid, const void *__shmaddr, int __shmflg));
__LIBC int __NOTHROW((__LIBCCALL shmdt)(const void *__shmaddr));

__SYSDECL_END

#endif /* !_SYS_GENERIC_SHM_H */
