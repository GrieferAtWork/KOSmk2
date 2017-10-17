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
#ifndef _ERRNO_H
#define _ERRNO_H 1

#include "__stdinc.h"
#include <features.h>

#ifdef __USE_DOS
#include <bits/dos-errno.h>
#endif /* __USE_DOS */

__SYSDECL_BEGIN

#ifdef __CC__
typedef int __errno_t;
#endif /* __CC__ */

#if defined(__KERNEL__) || defined(__USE_KOS)
#ifdef __CC__
#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */
#endif /* __CC__ */
#define EOK          0 /* Operation completed successfully */
#endif


#ifndef __USE_DOS

/* NOTE: Linux error codes are taken from comments
 *      in /usr/include/asm-generic/errno.h
 *     and /usr/include/asm-generic/errno-base.h
 * While those files didn't include any copyright notice,
 * I'm still adding the notice found in /usr/include/errno.h:*/

/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

#define EPERM           1   /* Operation not permitted */
#define ENOENT          2   /* No such file or directory */
#define ESRCH           3   /* No such process */
#define EINTR           4   /* Interrupted system call */
#define EIO             5   /* I/O error */
#define ENXIO           6   /* No such device or address */
#define E2BIG           7   /* Argument list too long */
#define ENOEXEC         8   /* Exec format error */
#define EBADF           9   /* Bad file number */
#define ECHILD          10  /* No child processes */
#define EAGAIN          11  /* Try again */
#define ENOMEM          12  /* Out of memory */
#define EACCES          13  /* Permission denied */
#define EFAULT          14  /* Bad address */
#define ENOTBLK         15  /* Block device required */
#define EBUSY           16  /* Device or resource busy */
#define EEXIST          17  /* File exists */
#define EXDEV           18  /* Cross-device link */
#define ENODEV          19  /* No such device */
#define ENOTDIR         20  /* Not a directory */
#define EISDIR          21  /* Is a directory */
#define EINVAL          22  /* Invalid argument */
#define ENFILE          23  /* File table overflow */
#define EMFILE          24  /* Too many open files */
#define ENOTTY          25  /* Not a typewriter */
#define ETXTBSY         26  /* Text file busy */
#define EFBIG           27  /* File too large */
#define ENOSPC          28  /* No space left on device */
#define ESPIPE          29  /* Illegal seek */
#define EROFS           30  /* Read-only file system */
#define EMLINK          31  /* Too many links */
#define EPIPE           32  /* Broken pipe */
#define EDOM            33  /* Math argument out of domain of func */
#define ERANGE          34  /* Math result not representable */
#define EDEADLK         35  /* Resource deadlock would occur */
#define ENAMETOOLONG    36  /* File name too long */
#define ENOLCK          37  /* No record locks available */

/*
 * This error code is special: arch syscall entry code will return
 * -ENOSYS if users try to call a syscall that doesn't exist.  To keep
 * failures of syscalls that really do exist distinguishable from
 * failures due to attempts to use a nonexistent syscall, syscall
 * implementations should refrain from returning -ENOSYS.
 */
#define ENOSYS          38  /* Invalid system call number */
#define ENOTEMPTY       39  /* Directory not empty */
#define ELOOP           40  /* Too many symbolic links encountered */
#define EWOULDBLOCK     EAGAIN /* Operation would block */
#define ENOMSG          42  /* No message of desired type */
#define EIDRM           43  /* Identifier removed */
#define ECHRNG          44  /* Channel number out of range */
#define EL2NSYNC        45  /* Level 2 not synchronized */
#define EL3HLT          46  /* Level 3 halted */
#define EL3RST          47  /* Level 3 reset */
#define ELNRNG          48  /* Link number out of range */
#define EUNATCH         49  /* Protocol driver not attached */
#define ENOCSI          50  /* No CSI structure available */
#define EL2HLT          51  /* Level 2 halted */
#define EBADE           52  /* Invalid exchange */
#define EBADR           53  /* Invalid request descriptor */
#define EXFULL          54  /* Exchange full */
#define ENOANO          55  /* No anode */
#define EBADRQC         56  /* Invalid request code */
#define EBADSLT         57  /* Invalid slot */
#define EDEADLOCK       EDEADLK
#define EBFONT          59  /* Bad font file format */
#define ENOSTR          60  /* Device not a stream */
#define ENODATA         61  /* No data available */
#define ETIME           62  /* Timer expired */
#define ENOSR           63  /* Out of streams resources */
#define ENONET          64  /* Machine is not on the network */
#define ENOPKG          65  /* Package not installed */
#define EREMOTE         66  /* Object is remote */
#define ENOLINK         67  /* Link has been severed */
#define EADV            68  /* Advertise error */
#define ESRMNT          69  /* Srmount error */
#define ECOMM           70  /* Communication error on send */
#define EPROTO          71  /* Protocol error */
#define EMULTIHOP       72  /* Multihop attempted */
#define EDOTDOT         73  /* RFS specific error */
#define EBADMSG         74  /* Not a data message */
#define EOVERFLOW       75  /* Value too large for defined data type */
#define ENOTUNIQ        76  /* Name not unique on network */
#define EBADFD          77  /* File descriptor in bad state */
#define EREMCHG         78  /* Remote address changed */
#define ELIBACC         79  /* Can not access a needed shared library */
#define ELIBBAD         80  /* Accessing a corrupted shared library */
#define ELIBSCN         81  /* .lib section in a.out corrupted */
#define ELIBMAX         82  /* Attempting to link in too many shared libraries */
#define ELIBEXEC        83  /* Cannot exec a shared library directly */
#define EILSEQ          84  /* Illegal byte sequence */
#define ERESTART        85  /* Interrupted system call should be restarted */
#define ESTRPIPE        86  /* Streams pipe error */
#define EUSERS          87  /* Too many users */
#define ENOTSOCK        88  /* Socket operation on non-socket */
#define EDESTADDRREQ    89  /* Destination address required */
#define EMSGSIZE        90  /* Message too long */
#define EPROTOTYPE      91  /* Protocol wrong type for socket */
#define ENOPROTOOPT     92  /* Protocol not available */
#define EPROTONOSUPPORT 93  /* Protocol not supported */
#define ESOCKTNOSUPPORT 94  /* Socket type not supported */
#define EOPNOTSUPP      95  /* Operation not supported on transport endpoint */
#define EPFNOSUPPORT    96  /* Protocol family not supported */
#define EAFNOSUPPORT    97  /* Address family not supported by protocol */
#define EADDRINUSE      98  /* Address already in use */
#define EADDRNOTAVAIL   99  /* Cannot assign requested address */
#define ENETDOWN        100 /* Network is down */
#define ENETUNREACH     101 /* Network is unreachable */
#define ENETRESET       102 /* Network dropped connection because of reset */
#define ECONNABORTED    103 /* Software caused connection abort */
#define ECONNRESET      104 /* Connection reset by peer */
#define ENOBUFS         105 /* No buffer space available */
#define EISCONN         106 /* Transport endpoint is already connected */
#define ENOTCONN        107 /* Transport endpoint is not connected */
#define ESHUTDOWN       108 /* Cannot send after transport endpoint shutdown */
#define ETOOMANYREFS    109 /* Too many references: cannot splice */
#define ETIMEDOUT       110 /* Connection timed out */
#define ECONNREFUSED    111 /* Connection refused */
#define EHOSTDOWN       112 /* Host is down */
#define EHOSTUNREACH    113 /* No route to host */
#define EALREADY        114 /* Operation already in progress */
#define EINPROGRESS     115 /* Operation now in progress */
#define ESTALE          116 /* Stale file handle */
#define EUCLEAN         117 /* Structure needs cleaning */
#define ENOTNAM         118 /* Not a XENIX named type file */
#define ENAVAIL         119 /* No XENIX semaphores available */
#define EISNAM          120 /* Is a named type file */
#define EREMOTEIO       121 /* Remote I/O error */
#define EDQUOT          122 /* Quota exceeded */

#define ENOMEDIUM       123 /* No medium found */
#define EMEDIUMTYPE     124 /* Wrong medium type */
#define ECANCELED       125 /* Operation Canceled */
#define ENOKEY          126 /* Required key not available */
#define EKEYEXPIRED     127 /* Key has expired */
#define EKEYREVOKED     128 /* Key has been revoked */
#define EKEYREJECTED    129 /* Key was rejected by service */

/* for robust mutexes */
#define EOWNERDEAD      130 /* Owner died */
#define ENOTRECOVERABLE 131 /* State not recoverable */

#define ERFKILL         132 /* Operation not possible due to RF-kill */

#define EHWPOISON       133 /* Memory page has hardware error */
#define __EBASEMAX      133
#endif /* !__USE_DOS */


#ifdef __CC__
#ifndef __KERNEL__
#ifndef _CRT_ERRNO_DEFINED
#define _CRT_ERRNO_DEFINED 1
#define errno                                            (*__errno())
__REDIRECT_DOS_FUNC_NOTHROW_(__LIBC,__WUNUSED,__errno_t *,__LIBCCALL,__errno,(void),_errno,())
__REDIRECT_DOS_FUNC_NOTHROW_(__LIBC,,__errno_t,__LIBCCALL,__set_errno,(__errno_t __err),_set_errno,(__err))
#if defined(__CRT_KOS) && (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__LIBC __WUNUSED __errno_t __NOTHROW((__LIBCCALL __get_errno)(void));
#else /* Builtin... */
__LOCAL __WUNUSED __errno_t __NOTHROW((__LIBCCALL __get_errno)(void)) { return errno; }
#endif /* Compat... */
#if defined(__CRT_DOS) && !defined(__GLC_COMPAT__)
__LIBC __errno_t __NOTHROW((__LIBCCALL _get_errno)(__errno_t *__perr));
__LIBC __errno_t __NOTHROW((__LIBCCALL _set_errno)(__errno_t __err));
#else /* Builtin... */
__LOCAL __errno_t __NOTHROW((__LIBCCALL _get_errno)(__errno_t *__perr)) { if (__perr) *__perr = errno; return 0; }
__LOCAL __errno_t __NOTHROW((__LIBCCALL _set_errno)(__errno_t __err)) { return (errno = __err); }
#endif /* Compat... */
#endif /* !_CRT_ERRNO_DEFINED */

#ifdef __USE_GNU
#undef program_invocation_name
#undef program_invocation_short_name
#if defined(__CRT_KOS) && (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
#define program_invocation_name       __libc_program_invocation_name()
#define program_invocation_short_name __libc_program_invocation_short_name()
__LIBC __ATTR_CONST char *__NOTHROW((__LIBCCALL __libc_program_invocation_name)(void));
__LIBC __ATTR_CONST char *__NOTHROW((__LIBCCALL __libc_program_invocation_short_name)(void));
#elif defined(__CRT_GLC) && !defined(__GLC_COMPAT__)
__LIBC char *program_invocation_name;
__LIBC char *program_invocation_short_name;
#elif defined(__CRT_DOS)
__LIBC char **__NOTHROW((__LIBCCALL __p__pgmptr)(void));
#define program_invocation_name        (*__p__pgmptr())
#define program_invocation_short_name  (*__p__pgmptr())
#endif
#endif /* __USE_GNU */
#endif /* !__KERNEL__ */
#endif /* __CC__ */

#ifdef __USE_KOS
#if defined(__CRT_KOS) && !defined(__GLC_COMPAT__) && !defined(__DOS_COMPAT__)
__LIBC __PORT_KOSONLY __WUNUSED errno_t __NOTHROW((__LIBCCALL errno_dos2kos)(errno_t __eno));
__LIBC __PORT_KOSONLY __WUNUSED errno_t __NOTHROW((__LIBCCALL errno_kos2dos)(errno_t __eno));
__LIBC __PORT_KOSONLY __WUNUSED __UINT32_TYPE__ __NOTHROW((__LIBCCALL errno_kos2nt)(errno_t __eno));
__LIBC __PORT_KOSONLY __WUNUSED errno_t __NOTHROW((__LIBCCALL errno_nt2kos)(__UINT32_TYPE__ __eno));
#endif /* __CRT_KOS && !__GLC_COMPAT__ && !__DOS_COMPAT__ */
#if defined(__CRT_DOS) && !defined(__GLC_COMPAT__)
__REDIRECT_NOTHROW(__LIBC,__PORT_DOSONLY __WUNUSED,errno_t,__LIBCCALL,errno_nt2dos,(__UINT32_TYPE__ __eno),_dosmaperr,(__eno))
#endif /* __CRT_DOS && !__GLC_COMPAT__ */
#endif /* __USE_KOS */

#ifdef __KERNEL__
#define ERELOAD         500 /* Resource must be reloaded (after a lock was temporarily lost). */
#define ELOST           501 /* Resource was lost (During an attempt to acquire a resource, another was lost). */
#define ENOREL          502 /* Invalid Relocation. */
#endif

#define __EMAX          502

#define __ERRNO_THRESHOLD            0xfffffc00
#define __ERRNO_THRESHOLD64  0xfffffffffffffc00ull

#if defined(__USE_KOS) || defined(__KERNEL__)

/* Make sure that the errno-threshold is within a valid range.
 * NOTE: To ensure that 'E_PTR()' works correctly, we reserve part
 *       of the last page of the virtual address space '0xfffff000'
 *       for error codes, excluding the aligned page base '0xfffff000'
 *       itself.
 */
#if ((0-__EMAX) & 0xffffffff) < __ERRNO_THRESHOLD
#   error "'__ERRNO_THRESHOLD' is too large"
#elif (((0-(4096-1)) & 0xffffffff) > __ERRNO_THRESHOLD)
#   error "'__ERRNO_THRESHOLD' is too small"
#endif

/* Helper macros for errors in kernel-space. */
#   define E_PTR(x)    __COMPILER_UNIPOINTER(x)
#ifndef __CC__
#   define E_GTERR(x)  (x)
#   define E_ISERR(x)  __unlikely((x) >= __ERRNO_THRESHOLD)
#   define E_ISOK(x)     __likely((x) < __ERRNO_THRESHOLD)
#elif __SIZEOF_POINTER__ == __SIZEOF_INT__
#   define E_GTERR(x)  (int)(x)
#   define E_ISERR(x)  __unlikely(((unsigned int)(x)) >= __ERRNO_THRESHOLD)
#   define E_ISOK(x)     __likely(((unsigned int)(x)) < __ERRNO_THRESHOLD)
#elif __SIZEOF_POINTER__ <= 4
#   define E_GTERR(x)  (int)(__INTPTR_TYPE__)(__UINTPTR_TYPE__)(x)
#   define E_ISERR(x)  __unlikely(((__UINTPTR_TYPE__)(x)) >= __ERRNO_THRESHOLD)
#   define E_ISOK(x)     __likely(((__UINTPTR_TYPE__)(x)) < __ERRNO_THRESHOLD)
#else
#   define E_GTERR(x)  (int)(__INTPTR_TYPE__)(__UINTPTR_TYPE__)(x)
#   define E_ISERR(x)  __unlikely(((__UINTPTR_TYPE__)(x)) >= __ERRNO_THRESHOLD64)
#   define E_ISOK(x)     __likely(((__UINTPTR_TYPE__)(x)) < __ERRNO_THRESHOLD64)
#endif
#endif

__SYSDECL_END

#endif /* !_ERRNO_H */
