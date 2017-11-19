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
#ifndef _SYS_GENERIC_UIO_H
#define _SYS_GENERIC_UIO_H 1
#define _SYS_UIO_H 1

#include <features.h>
#include <bits/types.h>
#include <sys/types.h>
#include <bits/uio.h>

#ifndef __CRT_GLC
#error "<sys/uio.h> is not supported by the linked libc"
#endif /* !__CRT_GLC */

__SYSDECL_BEGIN

#ifdef __USE_GNU
__LIBC ssize_t (__LIBCCALL process_vm_readv)(__pid_t __pid, struct iovec const *__lvec, unsigned long int __liovcnt, struct iovec const *__rvec, unsigned long int __riovcnt, unsigned long int __flags);
__LIBC ssize_t (__LIBCCALL process_vm_writev)(__pid_t __pid, struct iovec const *__lvec, unsigned long int __liovcnt, struct iovec const *__rvec, unsigned long int __riovcnt, unsigned long int __flags);
#endif /* __USE_GNU */
__LIBC __WUNUSED ssize_t (__LIBCCALL readv)(int __fd, struct iovec const *__iovec, int __count);
__LIBC __WUNUSED ssize_t (__LIBCCALL writev)(int __fd, struct iovec const *__iovec, int __count);
#ifdef __USE_MISC
__REDIRECT_FS_FUNC(__LIBC,__WUNUSED,ssize_t,__LIBCCALL,preadv,(int __fd, struct iovec const *__iovec, int __count, __FS_TYPE(off) __offset),preadv,(__fd,__iovec,__count,__offset))
__REDIRECT_FS_FUNC(__LIBC,__WUNUSED,ssize_t,__LIBCCALL,pwritev,(int __fd, struct iovec const *__iovec, int __count, __FS_TYPE(off) __offset),preadv,(__fd,__iovec,__count,__offset))
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED ssize_t (__LIBCCALL preadv64)(int __fd, struct iovec const *__iovec, int __count, __off64_t __offset);
__LIBC __WUNUSED ssize_t (__LIBCCALL pwritev64)(int __fd, struct iovec const *__iovec, int __count, __off64_t __offset);
#endif /* __USE_LARGEFILE64 */
#endif /* !__USE_MISC */

__SYSDECL_END

#endif /* !_SYS_GENERIC_UIO_H */
