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
#ifndef _ARM_KOS_BITS_PTHREADTYPES_H
#define _ARM_KOS_BITS_PTHREADTYPES_H 1

/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <endian.h>

#define __SIZEOF_PTHREAD_ATTR_T        36
#define __SIZEOF_PTHREAD_MUTEX_T       24
#define __SIZEOF_PTHREAD_MUTEXATTR_T   4
#define __SIZEOF_PTHREAD_COND_T        48
#define __SIZEOF_PTHREAD_COND_COMPAT_T 12
#define __SIZEOF_PTHREAD_CONDATTR_T    4
#define __SIZEOF_PTHREAD_RWLOCK_T      32
#define __SIZEOF_PTHREAD_RWLOCKATTR_T  8
#define __SIZEOF_PTHREAD_BARRIER_T     20
#define __SIZEOF_PTHREAD_BARRIERATTR_T 4


#ifdef __CC__
/* Thread identifiers. The structure of the attribute type is not exposed on purpose. */
typedef __ULONGPTR_TYPE__ pthread_t;
union pthread_attr_t {
  __INT8_TYPE__ __size[__SIZEOF_PTHREAD_ATTR_T];
  __LONGPTR_TYPE__ __align;
};
#ifndef __have_pthread_attr_t
#define __have_pthread_attr_t 1
typedef union pthread_attr_t pthread_attr_t;
#endif

typedef struct __pthread_internal_slist {
 struct __pthread_internal_slist *__next;
} __pthread_slist_t;


/* Data structures for mutex handling.
 * The structure of the attribute type is not exposed on purpose. */
typedef union {
  struct __pthread_mutex_s {
    __INT32_TYPE__  __lock;
    __UINT32_TYPE__ __count;
    __INT32_TYPE__  __owner;
    __INT32_TYPE__  __kind; /* KIND must stay at this position in the structure to maintain binary compatibility. */
    __UINT32_TYPE__ __nusers;
    union {
      int               __spins;
      __pthread_slist_t __list;
    };
  } __data;
  __INT8_TYPE__    __size[__SIZEOF_PTHREAD_MUTEX_T];
  __LONGPTR_TYPE__ __align;
} pthread_mutex_t;

/* Mutex __spins initializer used by PTHREAD_MUTEX_INITIALIZER. */
#define __PTHREAD_SPINS 0

typedef union {
  __INT8_TYPE__    __size[__SIZEOF_PTHREAD_MUTEXATTR_T];
  __LONGPTR_TYPE__ __align;
} pthread_mutexattr_t;


/* Data structure for conditional variable handling.
 * The structure of the attribute type is not exposed on purpose. */
typedef union {
  struct {
    __INT32_TYPE__  __lock;
    __UINT32_TYPE__ __futex;
    __UINT64_TYPE__ __total_seq;
    __UINT64_TYPE__ __wakeup_seq;
    __UINT64_TYPE__ __woken_seq;
    void           *__mutex;
    __UINT32_TYPE__ __nwaiters;
    __UINT32_TYPE__ __broadcast_seq;
  } __data;
  __INT8_TYPE__     __size[__SIZEOF_PTHREAD_COND_T];
  __INT64_TYPE__    __align;
} pthread_cond_t;

typedef union {
  __INT8_TYPE__    __size[__SIZEOF_PTHREAD_CONDATTR_T];
  __LONGPTR_TYPE__ __align;
} pthread_condattr_t;

typedef __UINT32_TYPE__ pthread_key_t; /* Keys for thread-specific data */
typedef __INT32_TYPE__  pthread_once_t; /* Once-only execution */

#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K)
/* Data structure for read-write lock variable handling.
 * The structure of the attribute type is not exposed on purpose. */
typedef union {
  struct {
    __INT32_TYPE__  __lock;
    __UINT32_TYPE__ __nr_readers;
    __UINT32_TYPE__ __readers_wakeup;
    __UINT32_TYPE__ __writer_wakeup;
    __UINT32_TYPE__ __nr_readers_queued;
    __UINT32_TYPE__ __nr_writers_queued;
#if __BYTE_ORDER == __BIG_ENDIAN
    __UINT8_TYPE__ __pad1;
    __UINT8_TYPE__ __pad2;
    __UINT8_TYPE__ __shared;
    __UINT8_TYPE__ __flags; /* FLAGS must stay at this position in the structure to maintain binary compatibility. */
#else /* __BYTE_ORDER == __BIG_ENDIAN */
    __UINT8_TYPE__ __flags; /* FLAGS must stay at this position in the structure to maintain binary compatibility. */
    __UINT8_TYPE__ __shared;
    __UINT8_TYPE__ __pad1;
    __UINT8_TYPE__ __pad2;
#endif /* __BYTE_ORDER != __BIG_ENDIAN */
    __INT32_TYPE__ __writer;
  } __data;
  __INT8_TYPE__    __size[__SIZEOF_PTHREAD_RWLOCK_T];
  __LONGPTR_TYPE__ __align;
} pthread_rwlock_t;

#define __PTHREAD_RWLOCK_ELISION_EXTRA 0

typedef union {
  __INT8_TYPE__    __size[__SIZEOF_PTHREAD_RWLOCKATTR_T];
  __LONGPTR_TYPE__ __align;
} pthread_rwlockattr_t;
#endif


#ifdef __USE_XOPEN2K
/* POSIX spinlock data type. */
typedef volatile int pthread_spinlock_t;

/* POSIX barriers data type.
 * The structure of the type is deliberately not exposed. */
typedef union {
  __INT8_TYPE__    __size[__SIZEOF_PTHREAD_BARRIER_T];
  __LONGPTR_TYPE__ __align;
} pthread_barrier_t;

typedef union {
  __INT8_TYPE__  __size[__SIZEOF_PTHREAD_BARRIERATTR_T];
  __INT32_TYPE__ __align;
} pthread_barrierattr_t;
#endif
#endif /* __CC__ */

#endif /* !_ARM_KOS_BITS_PTHREADTYPES_H */
