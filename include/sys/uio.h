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
#ifndef _SYS_UIO_H
#define _SYS_UIO_H 1

#include <features.h>
#include <bits/types.h>
#include <sys/types.h>
#include <bits/uio.h>

__SYSDECL_BEGIN

#ifdef __USE_GNU
__LIBC ssize_t (__LIBCCALL process_vm_readv)(__pid_t __pid, const struct iovec *__lvec, unsigned long int __liovcnt, const struct iovec *__rvec, unsigned long int __riovcnt, unsigned long int __flags);
__LIBC ssize_t (__LIBCCALL process_vm_writev)(__pid_t __pid, const struct iovec *__lvec, unsigned long int __liovcnt, const struct iovec *__rvec, unsigned long int __riovcnt, unsigned long int __flags);
#endif
__LIBC __WUNUSED ssize_t (__LIBCCALL readv)(int __fd, const struct iovec *__iovec, int __count);
__LIBC __WUNUSED ssize_t (__LIBCCALL writev)(int __fd, const struct iovec *__iovec, int __count);
#ifdef __USE_MISC
__LIBC __WUNUSED ssize_t (__LIBCCALL preadv)(int __fd, const struct iovec *__iovec, int __count, __FS_TYPE(off) __offset) __FS_FUNC(preadv);
__LIBC __WUNUSED ssize_t (__LIBCCALL pwritev)(int __fd, const struct iovec *__iovec, int __count, __FS_TYPE(off) __offset) __FS_FUNC(preadv);
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED ssize_t (__LIBCCALL preadv64)(int __fd, const struct iovec *__iovec, int __count, __off64_t __offset);
__LIBC __WUNUSED ssize_t (__LIBCCALL pwritev64)(int __fd, const struct iovec *__iovec, int __count, __off64_t __offset);
#endif /* __USE_LARGEFILE64 */
#endif /* !__USE_MISC */

__SYSDECL_END

#endif /* !_SYS_UIO_H */
