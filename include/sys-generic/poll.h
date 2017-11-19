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
#ifndef _SYS_GENERIC_POLL_H
#define _SYS_GENERIC_POLL_H 1

#include <features.h>
#include <bits/poll.h>
#include <hybrid/typecore.h>
#ifdef __USE_GNU
#include <bits/sigset.h>
#include <hybrid/timespec.h>
#endif /* __USE_GNU */

#ifndef __CRT_GLC
#error "<sys/poll.h> is not supported by the linked libc"
#endif /* !__CRT_GLC */

__SYSDECL_BEGIN

#ifndef __nfds_t_defined
#define __nfds_t_defined 1
typedef __UINTPTR_TYPE__ nfds_t;
#endif /* !__nfds_t_defined */

#ifndef __pollfd_defined
#define __pollfd_defined 1
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("fd")
#pragma push_macro("events")
#pragma push_macro("revents")
#endif
#undef fd
#undef events
#undef revents
struct pollfd {
    int            fd;      /*< File descriptor to poll.  */
    __INT16_TYPE__ events;  /*< Types of events poller cares about (Set of 'POLL*'). */
    __INT16_TYPE__ revents; /*< Types of events that actually occurred (Set of 'POLL*'). */
};
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("revents")
#pragma pop_macro("events")
#pragma pop_macro("fd")
#endif
#endif /* !__pollfd_defined */

#ifndef __KERNEL__
__LIBC int (__LIBCCALL poll)(struct pollfd *__fds, nfds_t __nfds, int __timeout);
#ifdef __USE_GNU
#ifdef __GLC_COMPAT__
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,,int,__LIBCCALL,__ppoll32,
          (struct pollfd *__fds, nfds_t __nfds,
           struct __timespec32 const *__timeout,
           __sigset_t const *__ss),
           ppoll,(__fds,__nfds,__timeout,__ss))
__LOCAL int (__LIBCCALL ppoll)(struct pollfd *__fds, nfds_t __nfds,
                               struct timespec const *__timeout,
                               __sigset_t const *__ss) {
    struct __timespec32 __tmo;
    if (__timeout) __tmo.tv_sec = (__time32_t)__timeout->tv_sec,
                   __tmo.tv_nsec = (__time32_t)__timeout->tv_nsec;
    return __ppoll32(__fds,__nfds,__timeout ? &__tmo : 0,__ss);
}
#ifdef __USE_TIME64
__LOCAL int (__LIBCCALL ppoll64)(struct pollfd *__fds, nfds_t __nfds,
                                 struct __timespec64 const *__timeout,
                                 __sigset_t const *__ss) {
    return ppoll(__fds,__nfds,__timeout,__ss);
}
#endif /* __USE_TIME64 */
#else /* __USE_TIME_BITS64 */
__LIBC int (__LIBCCALL ppoll)(struct pollfd *__fds, nfds_t __nfds,
                              struct timespec const *__timeout,
                              __sigset_t const *__ss);
#ifdef __USE_TIME64
__LOCAL int (__LIBCCALL ppoll64)(struct pollfd *__fds, nfds_t __nfds,
                                 struct __timespec64 const *__timeout,
                                 __sigset_t const *__ss) {
    struct timespec __tmo;
    if (__timeout) __tmo.tv_sec = (__time32_t)__timeout->tv_sec,
                   __tmo.tv_nsec = (__time32_t)__timeout->tv_nsec;
    return ppoll(__fds,__nfds,__timeout ? &__tmo : 0,__ss);
}
#endif /* __USE_TIME64 */
#endif /* !__USE_TIME_BITS64 */
#else /* __GLC_COMPAT__ */
__REDIRECT_TM_FUNC(__LIBC,,int,__LIBCCALL,ppoll,
                  (struct pollfd *__fds, nfds_t __nfds,
                   struct timespec const *__timeout,
                   __sigset_t const *__ss),
                   ppoll,(__fds,__nfds,__timeout,__ss))
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL ppoll64)(struct pollfd *__fds, nfds_t __nfds,
                                struct __timespec64 const *__timeout,
                                __sigset_t const *__ss);
#endif /* __USE_TIME64 */
#endif /* !__GLC_COMPAT__ */
#endif /* __USE_GNU */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_SYS_GENERIC_POLL_H */
