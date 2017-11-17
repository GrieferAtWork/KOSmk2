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
#ifndef _BIT_SIGEVENT_H
#define _BIT_SIGEVENT_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

/* siginfo_t, sigevent and constants.  Linux x86-64 version.
   Copyright (C) 2012-2016 Free Software Foundation, Inc.
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

__SYSDECL_BEGIN

#define __SIGEV_MAX_SIZE    64
#if __SIZEOF_POINTER__ == 8
#   define __SIGEV_PAD_SIZE    ((__SIGEV_MAX_SIZE/sizeof(int))-4)
#else
#   define __SIGEV_PAD_SIZE    ((__SIGEV_MAX_SIZE/sizeof(int))-3)
#endif

#ifdef __CC__
#ifndef __have_sigval_t
#define __have_sigval_t 1
/* Type for data associated with a signal. */
typedef union sigval {
    int   sival_int;
    void *sival_ptr;
} sigval_t;
#endif /* !__have_sigval_t */

#ifndef __have_pthread_attr_t
#define __have_pthread_attr_t 1
typedef union pthread_attr_t pthread_attr_t;
#endif /* !__have_pthread_attr_t */

typedef struct sigevent {
    sigval_t sigev_value;
    int      sigev_signo;
    int      sigev_notify;
#if defined(__COMPILER_HAVE_TRANSPARENT_STRUCT) && \
    defined(__COMPILER_HAVE_TRANSPARENT_UNION)
#ifndef __USE_KOS
    struct{
#endif /* !__USE_KOS */
    union {
        int _pad[__SIGEV_PAD_SIZE];
        /* When SIGEV_SIGNAL and SIGEV_THREAD_ID set, LWP ID of the thread to receive the signal. */
        __pid_t _tid;
        struct {
            void (__ATTR_CDECL *sigev_notify_function)(sigval_t __val); /* Function to start. */
            pthread_attr_t     *sigev_notify_attributes;                /* Thread attributes. */
        };
    };
#ifndef __USE_KOS
    union {
        int _pad[__SIGEV_PAD_SIZE];
        /* When SIGEV_SIGNAL and SIGEV_THREAD_ID set, LWP ID of the thread to receive the signal. */
        __pid_t _tid;
        struct {
            void (__ATTR_CDECL *_function)(sigval_t __val); /* Function to start. */
            pthread_attr_t *_attribute;                     /* Thread attributes. */
        } _sigev_thread;
    } _sigev_un;
    };
#endif /* !__USE_KOS */
#else
    union {
        int _pad[__SIGEV_PAD_SIZE];
        /* When SIGEV_SIGNAL and SIGEV_THREAD_ID set, LWP ID of the thread to receive the signal. */
        __pid_t _tid;
        struct {
            void (__ATTR_CDECL *_function)(sigval_t __val); /* Function to start. */
            pthread_attr_t *_attribute;                     /* Thread attributes. */
        } _sigev_thread;
    } _sigev_un;
#define sigev_notify_function   _sigev_un._sigev_thread._function
#define sigev_notify_attributes _sigev_un._sigev_thread._attribute
#endif
} sigevent_t;
#endif /* __CC__ */

/* `sigev_notify' values. */
#ifdef __COMPILER_PREFERR_ENUMS
enum {
    SIGEV_SIGNAL    = 0, /*< Notify via signal. */
    SIGEV_NONE      = 1, /*< Other notification: meaningless. */
    SIGEV_THREAD    = 2, /*< Deliver via thread creation. */
    SIGEV_THREAD_ID = 4  /*< Send signal to specific thread. */
};
#define SIGEV_SIGNAL    SIGEV_SIGNAL
#define SIGEV_NONE      SIGEV_NONE
#define SIGEV_THREAD    SIGEV_THREAD
#define SIGEV_THREAD_ID SIGEV_THREAD_ID
#else /* __COMPILER_PREFERR_ENUMS */
#define SIGEV_SIGNAL    0 /*< Notify via signal. */
#define SIGEV_NONE      1 /*< Other notification: meaningless. */
#define SIGEV_THREAD    2 /*< Deliver via thread creation. */
#define SIGEV_THREAD_ID 4 /*< Send signal to specific thread. */
#endif /* !__COMPILER_PREFERR_ENUMS */

__SYSDECL_END

#endif /* !_BIT_SIGEVENT_H */
