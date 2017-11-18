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
#ifndef _BITS_GENERIC_TYPES_H
#define _BITS_GENERIC_TYPES_H 1
#define _BITS_TYPES_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#define __SIZEOF_SYSCALL_LONG__ __SIZEOF_REGISTER__
#if __SIZEOF_REGISTER__ == __SIZEOF_LONG__
#   define __SYSCALL_SLONG_TYPE signed long
#   define __SYSCALL_ULONG_TYPE unsigned long
#else
#   define __SYSCALL_SLONG_TYPE __SREGISTER_TYPE__
#   define __SYSCALL_ULONG_TYPE __REGISTER_TYPE__
#endif


#define __SIZEOF_BLKADDR32_T__  4
#define __SIZEOF_BLKADDR64_T__  8
#define __SIZEOF_BLKCNT32_T__   4
#define __SIZEOF_BLKCNT64_T__   8
#define __SIZEOF_BLKSIZE_T__    __SIZEOF_SYSCALL_LONG__
#define __SIZEOF_BYTE_T__       1
#define __SIZEOF_CADDR_T__      __SIZEOF_POINTER__
#define __SIZEOF_CLOCKID_T__    4
#define __SIZEOF_CLOCK_T__      __SIZEOF_SYSCALL_LONG__
#define __SIZEOF_CPUID_T__      2
#define __SIZEOF_DADDR_T__      4
#define __SIZEOF_FSBLKCNT32_T__ 4
#define __SIZEOF_FSBLKCNT64_T__ 8
#define __SIZEOF_FSFILCNT32_T__ 4
#define __SIZEOF_FSFILCNT64_T__ 8
#define __SIZEOF_FSID_T__       8
#define __SIZEOF_FSINT32_T__    4
#define __SIZEOF_FSINT64_T__    8
#define __SIZEOF_FSUINT32_T__   4
#define __SIZEOF_FSUINT64_T__   8
#define __SIZEOF_FSWORD32_T__   4
#define __SIZEOF_FSWORD64_T__   8
#define __SIZEOF_GID_T__        4
#define __SIZEOF_ID_T__         4
#define __SIZEOF_INO32_T__      4
#define __SIZEOF_INO64_T__      8
#define __SIZEOF_JTIME32_T__    4
#define __SIZEOF_JTIME64_T__    8
#define __SIZEOF_KEY_T__        4
#define __SIZEOF_LOFF_T__       __SIZEOF_OFF64_T__
#define __SIZEOF_LPOS_T__       __SIZEOF_POS64_T__
#define __SIZEOF_MODE_T__       4
#define __SIZEOF_NLINK_T__      4
#define __SIZEOF_OFF32_T__      4
#define __SIZEOF_OFF64_T__      8
#define __SIZEOF_PID_T__        4
#define __SIZEOF_POS32_T__      4
#define __SIZEOF_POS64_T__      8
#define __SIZEOF_QUAD_T__       8
#define __SIZEOF_RLIM32_T__     4
#define __SIZEOF_RLIM64_T__     8
#define __SIZEOF_RLIM_T__       __SIZEOF_SYSCALL_LONG__
#define __SIZEOF_SOCKLEN_T__    4
#define __SIZEOF_SUSECONDS_T__  __SIZEOF_SYSCALL_LONG__
#define __SIZEOF_TIME32_T__     4
#define __SIZEOF_TIME64_T__     8
#define __SIZEOF_TIMER_T__      __SIZEOF_POINTER__
#define __SIZEOF_UID_T__        4
#define __SIZEOF_USECONDS_T__   4

#ifdef __CC__
#undef  __blkaddr32_t
#undef  __blkaddr64_t
#undef  __blkcnt32_t
#undef  __blkcnt64_t
#undef  __blksize_t
#undef  __byte_t
#undef  __caddr_t
#undef  __clock_t
#undef  __clockid_t
#undef  __cpuid_t
#undef  __daddr_t
#undef  __fsblkcnt32_t
#undef  __fsblkcnt64_t
#undef  __fsfilcnt32_t
#undef  __fsfilcnt64_t
#undef  __fsid_t
#undef  __fsint32_t
#undef  __fsint64_t
#undef  __fsuint32_t
#undef  __fsuint64_t
#undef  __fsword32_t
#undef  __fsword64_t
#undef  __gid_t
#undef  __id_t
#undef  __ino32_t
#undef  __ino64_t
#undef  __int16_t
#undef  __int32_t
#undef  __int64_t
#undef  __int8_t
#undef  __intptr_t
#undef  __jtime32_t
#undef  __jtime64_t
#undef  __key_t
#undef  __loff_t
#undef  __lpos_t
#undef  __mode_t
#undef  __nlink_t
#undef  __off32_t
#undef  __off64_t
#undef  __pid_t
#undef  __pos32_t
#undef  __pos64_t
#undef  __ptrdiff_t
#undef  __qaddr_t
#undef  __quad_t
#undef  __register_t
#undef  __rlim32_t
#undef  __rlim64_t
#undef  __rlim_t
#undef  __size_t
#undef  __socklen_t
#undef  __ssize_t
#undef  __suseconds_t
#undef  __syscall_slong_t
#undef  __syscall_ulong_t
#undef  __time32_t
#undef  __time64_t
#undef  __timer_t
#undef  __u_char
#undef  __u_int
#undef  __u_long
#undef  __u_quad_t
#undef  __u_short
#undef  __uid_t
#undef  __uint16_t
#undef  __uint32_t
#undef  __uint64_t
#undef  __uint8_t
#undef  __uintptr_t
#undef  __uregister_t
#undef  __useconds_t

typedef __INT16_TYPE__          __int16_t;
typedef __INT32_TYPE__          __clockid_t;
typedef __INT32_TYPE__          __daddr_t;
typedef __INT32_TYPE__          __int32_t;
typedef __INT32_TYPE__          __key_t;
typedef __INT32_TYPE__          __pid_t;
typedef __INT32_TYPE__          __time32_t;
typedef __INT64_TYPE__          __int64_t;
typedef __INT64_TYPE__          __quad_t;
typedef __INT64_TYPE__          __time64_t;
typedef __INT8_TYPE__           __int8_t;
typedef __INTPTR_TYPE__         __intptr_t;
typedef __LONG32_TYPE__         __blkcnt32_t;
typedef __LONG32_TYPE__         __fsint32_t;
typedef __LONG32_TYPE__         __fsword32_t;
typedef __LONG32_TYPE__         __off32_t;
typedef __LONG64_TYPE__         __blkcnt64_t;
typedef __LONG64_TYPE__         __fsint64_t;
typedef __LONG64_TYPE__         __fsword64_t;
typedef __LONG64_TYPE__         __off64_t;
typedef __PTRDIFF_TYPE__        __ptrdiff_t;
typedef __REGISTER_TYPE__       __register_t;
typedef __SIZE_TYPE__           __size_t;
typedef __SREGISTER_TYPE__      __sregister_t;
typedef __SSIZE_TYPE__          __ssize_t;
typedef __SYSCALL_SLONG_TYPE    __clock_t;
typedef __SYSCALL_SLONG_TYPE    __suseconds_t;
typedef __SYSCALL_SLONG_TYPE    __syscall_slong_t;
typedef __SYSCALL_ULONG_TYPE    __blksize_t;
typedef __SYSCALL_ULONG_TYPE    __rlim_t;
typedef __SYSCALL_ULONG_TYPE    __syscall_ulong_t;
typedef __UINT16_TYPE__         __cpuid_t;
typedef __UINT16_TYPE__         __uint16_t;
typedef __UINT32_TYPE__         __gid_t;
typedef __UINT32_TYPE__         __id_t;
typedef __UINT32_TYPE__         __jtime32_t;
typedef __ULONG32_TYPE__        __blkaddr32_t;
typedef __ULONG32_TYPE__        __fsblkcnt32_t;
typedef __ULONG32_TYPE__        __fsfilcnt32_t;
typedef __ULONG32_TYPE__        __fsuint32_t;
typedef __ULONG32_TYPE__        __ino32_t;
#ifdef __USE_DOS
/* Simple enough: DOS defines this one as signed, rather than unsigned. */
typedef __INT32_TYPE__          __mode_t;
#else /* __DOS_COMPAT__ */
typedef __UINT32_TYPE__         __mode_t;
#endif /* !__DOS_COMPAT__ */
typedef __UINT32_TYPE__         __nlink_t;
typedef __UINT32_TYPE__         __rlim32_t;
typedef __UINT32_TYPE__         __socklen_t;
typedef __UINT32_TYPE__         __uid_t;
typedef __UINT32_TYPE__         __uint32_t;
typedef __UINT32_TYPE__         __useconds_t;
typedef __UINT64_TYPE__         __jtime64_t;
typedef __UINT64_TYPE__         __rlim64_t;
typedef __UINT64_TYPE__         __u_quad_t;
typedef __UINT64_TYPE__         __uint64_t;
typedef __UINT8_TYPE__          __byte_t;
typedef __UINT8_TYPE__          __uint8_t;
typedef __UINTPTR_TYPE__        __uintptr_t;
typedef __ULONG32_TYPE__        __pos32_t;
typedef __ULONG64_TYPE__        __blkaddr64_t;
typedef __ULONG64_TYPE__        __fsblkcnt64_t;
typedef __ULONG64_TYPE__        __fsfilcnt64_t;
typedef __ULONG64_TYPE__        __fsuint64_t;
typedef __ULONG64_TYPE__        __ino64_t;
typedef __ULONG64_TYPE__        __pos64_t;
typedef __off64_t               __loff_t;
typedef __pos64_t               __lpos_t;
typedef __quad_t                __qaddr_t;
typedef char                   *__caddr_t;
typedef struct { __INT32_TYPE__ __val[2]; } __fsid_t;
typedef unsigned char           __u_char;
typedef unsigned int            __u_int;
typedef unsigned long           __u_long;
typedef unsigned short          __u_short;
typedef void                   *__timer_t;
#define __blkaddr32_t           __blkaddr32_t
#define __blkaddr64_t           __blkaddr64_t
#define __blkcnt32_t            __blkcnt32_t
#define __blkcnt64_t            __blkcnt64_t
#define __blksize_t             __blksize_t
#define __byte_t                __byte_t
#define __caddr_t               __caddr_t
#define __clock_t               __clock_t
#define __clockid_t             __clockid_t
#define __cpuid_t               __cpuid_t
#define __daddr_t               __daddr_t
#define __fsblkcnt32_t          __fsblkcnt32_t
#define __fsblkcnt64_t          __fsblkcnt64_t
#define __fsfilcnt32_t          __fsfilcnt32_t
#define __fsfilcnt64_t          __fsfilcnt64_t
#define __fsid_t                __fsid_t
#define __fsint32_t             __fsint32_t
#define __fsint64_t             __fsint64_t
#define __fsuint32_t            __fsuint32_t
#define __fsuint64_t            __fsuint64_t
#define __fsword32_t            __fsword32_t
#define __fsword64_t            __fsword64_t
#define __gid_t                 __gid_t
#define __id_t                  __id_t
#define __ino32_t               __ino32_t
#define __ino64_t               __ino64_t
#define __int16_t               __int16_t
#define __int32_t               __int32_t
#define __int64_t               __int64_t
#define __int8_t                __int8_t
#define __intptr_t              __intptr_t
#define __jtime32_t             __jtime32_t
#define __jtime64_t             __jtime64_t
#define __key_t                 __key_t
#define __loff_t                __loff_t
#define __lpos_t                __lpos_t
#define __mode_t                __mode_t
#define __nlink_t               __nlink_t
#define __off32_t               __off32_t
#define __off64_t               __off64_t
#define __pid_t                 __pid_t
#define __pos32_t               __pos32_t
#define __pos64_t               __pos64_t
#define __ptrdiff_t             __ptrdiff_t
#define __qaddr_t               __qaddr_t
#define __quad_t                __quad_t
#define __register_t            __register_t
#define __rlim32_t              __rlim32_t
#define __rlim64_t              __rlim64_t
#define __rlim_t                __rlim_t
#define __size_t                __size_t
#define __socklen_t             __socklen_t
#define __sregister_t           __sregister_t
#define __ssize_t               __ssize_t
#define __suseconds_t           __suseconds_t
#define __syscall_slong_t       __syscall_slong_t
#define __syscall_ulong_t       __syscall_ulong_t
#define __time32_t              __time32_t
#define __time64_t              __time64_t
#define __timer_t               __timer_t
#define __u_char                __u_char
#define __u_int                 __u_int
#define __u_long                __u_long
#define __u_quad_t              __u_quad_t
#define __u_short               __u_short
#define __uid_t                 __uid_t
#define __uint16_t              __uint16_t
#define __uint32_t              __uint32_t
#define __uint64_t              __uint64_t
#define __uint8_t               __uint8_t
#define __uintptr_t             __uintptr_t
#define __useconds_t            __useconds_t
#endif /* __CC__ */

#if __SIZEOF_LOFF_T__ > __SIZEOF_SYSCALL_LONG__
/* When defined, the kernel provides extended file-system system calls. */
#define __ARCH_EXTENDED_FS_SYSCALLS 1
#endif


/* Additional types used inside the KOS kernel. */
#if 1
#ifdef CONFIG_32BIT_FILESYSTEM
#   define __SIZEOF_FS_LONG__ 4
#   define __FS_SLONG_TYPE    __LONG32_TYPE__
#   define __FS_ULONG_TYPE    __ULONG32_TYPE__
#else
#   define __SIZEOF_FS_LONG__ 8
#   define __FS_SLONG_TYPE    __LONG64_TYPE__
#   define __FS_ULONG_TYPE    __ULONG64_TYPE__
#endif
#define __SIZEOF_KERNEL_BLKCNT_T__   __SIZEOF_FS_LONG__
#define __SIZEOF_KERNEL_BLKADDR_T__  __SIZEOF_FS_LONG__
#define __SIZEOF_KERNEL_DEV_T__      4
#define __SIZEOF_KERNEL_FSBLKCNT_T__ __SIZEOF_FS_LONG__
#define __SIZEOF_KERNEL_FSFILCNT_T__ __SIZEOF_FS_LONG__
#define __SIZEOF_KERNEL_FSINT_T__    __SIZEOF_FS_LONG__
#define __SIZEOF_KERNEL_FSUINT_T__   __SIZEOF_FS_LONG__
#define __SIZEOF_KERNEL_FSWORD_T__   __SIZEOF_FS_LONG__
#define __SIZEOF_KERNEL_INO_T__      __SIZEOF_FS_LONG__
#define __SIZEOF_KERNEL_IRQ_T__      1
#define __SIZEOF_KERNEL_MAJOR_T__    2
#define __SIZEOF_KERNEL_MINOR_T__    4
#define __SIZEOF_KERNEL_OFF_T__      __SIZEOF_FS_LONG__
#define __SIZEOF_KERNEL_OFLAG_T__    4
#define __SIZEOF_KERNEL_POS_T__      __SIZEOF_FS_LONG__
#if 1
#define __SIZEOF_KERNEL_REF_T__      4
#else
#define __SIZEOF_KERNEL_REF_T__      __SIZEOF_POINTER__
#endif
#define __SIZEOF_KERNEL_SYSNO_T__    __SIZEOF_SYSCALL_LONG__
#ifdef CONFIG_32BIT_TIME
#   define __SIZEOF_KERNEL_TIME_T__  __SIZEOF_TIME32_T__
#   define __SIZEOF_KERNEL_JTIME_T__ __SIZEOF_JTIME32_T__
#else
#   define __SIZEOF_KERNEL_TIME_T__  __SIZEOF_TIME64_T__
#   define __SIZEOF_KERNEL_JTIME_T__ __SIZEOF_JTIME64_T__
#endif

#ifdef __CC__
#undef  __blkcnt_t
#undef  __blkaddr_t
#undef  __dev_t
#undef  __fsblkcnt_t
#undef  __fsfilcnt_t
#undef  __fsint_t
#undef  __fsuint_t
#undef  __fsword_t
#undef  __ino_t
#undef  __irq_t
#undef  __jtime_t
#undef  __major_t
#undef  __minor_t
#undef  __off_t
#undef  __oflag_t
#undef  __pos_t
#undef  __ref_t
#undef  __sysno_t
#undef  __time_t
typedef __FS_SLONG_TYPE      __fsint_t;
typedef __FS_SLONG_TYPE      __fsword_t;
typedef __FS_SLONG_TYPE      __off_t; /* File offset. */
typedef __FS_ULONG_TYPE      __blkaddr_t;
typedef __FS_ULONG_TYPE      __blkcnt_t;
typedef __FS_ULONG_TYPE      __fsblkcnt_t;
typedef __FS_ULONG_TYPE      __fsfilcnt_t;
typedef __FS_ULONG_TYPE      __fsuint_t;
typedef __FS_ULONG_TYPE      __ino_t;
typedef __FS_ULONG_TYPE      __pos_t; /* File position/size. */
typedef __SYSCALL_ULONG_TYPE __sysno_t; /* System call number. */
typedef __UINT16_TYPE__      __major_t;
typedef __UINT32_TYPE__      __dev_t;
typedef __UINT32_TYPE__      __minor_t;
typedef __UINT32_TYPE__      __oflag_t;
typedef __UINT8_TYPE__       __irq_t; /* Processor interrupt number. */
#if __SIZEOF_KERNEL_REF_T__ == __SIZEOF_POINTER__
typedef __UINTPTR_TYPE__     __ref_t; /* Reference counter. */
#else
typedef __UINT32_TYPE__      __ref_t; /* Reference counter. */
#endif
#ifdef CONFIG_32BIT_TIME
typedef __time32_t           __time_t;
typedef __jtime32_t          __jtime_t;
#else
typedef __time64_t           __time_t;
typedef __jtime64_t          __jtime_t;
#endif
#define __blkcnt_t           __blkcnt_t
#define __blkaddr_t          __blkaddr_t
#define __dev_t              __dev_t
#define __fsblkcnt_t         __fsblkcnt_t
#define __fsfilcnt_t         __fsfilcnt_t
#define __fsint_t            __fsint_t
#define __fsuint_t           __fsuint_t
#define __fsword_t           __fsword_t
#define __ino_t              __ino_t
#define __irq_t              __irq_t
#define __major_t            __major_t
#define __minor_t            __minor_t
#define __off_t              __off_t
#define __oflag_t            __oflag_t
#define __pos_t              __pos_t
#define __ref_t              __ref_t
#define __sysno_t            __sysno_t
#define __time_t             __time_t
#endif /* __CC__ */

#undef __FS_SLONG_TYPE
#undef __FS_ULONG_TYPE

#define __SIZEOF_DEV_T__      __SIZEOF_KERNEL_DEV_T__
#define __SIZEOF_IRQ_T__      __SIZEOF_KERNEL_IRQ_T__
#define __SIZEOF_MAJOR_T__    __SIZEOF_KERNEL_MAJOR_T__
#define __SIZEOF_MINOR_T__    __SIZEOF_KERNEL_MINOR_T__
#define __SIZEOF_OFLAG_T__    __SIZEOF_KERNEL_OFLAG_T__
#define __SIZEOF_REF_T__      __SIZEOF_KERNEL_REF_T__
#define __SIZEOF_SYSNO_T__    __SIZEOF_KERNEL_SYSNO_T__
#define __SIZEOF_REAL_DEV_T__ __SIZEOF_KERNEL_DEV_T__


#ifndef __KERNEL__
#include <features.h>
#if defined(__USE_FILE_OFFSET64) || __SIZEOF_SYSCALL_LONG__ >= 8
#   define __SIZEOF_BLKCNT_T__   __SIZEOF_BLKCNT64_T__
#   define __SIZEOF_BLKADDR_T__  __SIZEOF_BLKADDR64_T__
#   define __SIZEOF_FSBLKCNT_T__ __SIZEOF_FSBLKCNT64_T__
#   define __SIZEOF_FSFILCNT_T__ __SIZEOF_FSFILCNT64_T__
#   define __SIZEOF_FSINT_T__    __SIZEOF_FSINT64_T__
#   define __SIZEOF_FSUINT_T__   __SIZEOF_FSUINT64_T__
#   define __SIZEOF_FSWORD_T__   __SIZEOF_FSWORD64_T__
#   define __SIZEOF_INO_T__      __SIZEOF_INO64_T__
#   define __SIZEOF_OFF_T__      __SIZEOF_OFF64_T__
#   define __SIZEOF_POS_T__      __SIZEOF_POS64_T__
#   define __SIZEOF_REAL_INO_T__ __SIZEOF_INO64_T__
#   define __SIZEOF_REAL_OFF_T__ __SIZEOF_OFF64_T__
#else
#   define __SIZEOF_BLKCNT_T__   __SIZEOF_BLKCNT32_T__
#   define __SIZEOF_BLKADDR_T__  __SIZEOF_BLKADDR32_T__
#   define __SIZEOF_FSBLKCNT_T__ __SIZEOF_FSBLKCNT32_T__
#   define __SIZEOF_FSFILCNT_T__ __SIZEOF_FSFILCNT32_T__
#   define __SIZEOF_FSINT_T__    __SIZEOF_FSINT32_T__
#   define __SIZEOF_FSUINT_T__   __SIZEOF_FSUINT32_T__
#   define __SIZEOF_FSWORD_T__   __SIZEOF_FSWORD32_T__
#   define __SIZEOF_INO_T__      __SIZEOF_INO32_T__
#   define __SIZEOF_OFF_T__      __SIZEOF_OFF32_T__
#   define __SIZEOF_POS_T__      __SIZEOF_POS32_T__
#   define __SIZEOF_REAL_INO_T__ __SIZEOF_INO32_T__
#   define __SIZEOF_REAL_OFF_T__ __SIZEOF_OFF32_T__
#endif
#ifdef __USE_TIME_BITS64
#   define __SIZEOF_TIME_T__     __SIZEOF_TIME64_T__
#   define __SIZEOF_JTIME_T__    __SIZEOF_JTIME64_T__
#else
#   define __SIZEOF_TIME_T__     __SIZEOF_TIME32_T__
#   define __SIZEOF_JTIME_T__    __SIZEOF_JTIME32_T__
#endif
#else
#   define __SIZEOF_BLKCNT_T__   __SIZEOF_KERNEL_BLKCNT_T__
#   define __SIZEOF_BLKADDR_T__  __SIZEOF_KERNEL_BLKADDR_T__
#   define __SIZEOF_FSBLKCNT_T__ __SIZEOF_KERNEL_FSBLKCNT_T__
#   define __SIZEOF_FSFILCNT_T__ __SIZEOF_KERNEL_FSFILCNT_T__
#   define __SIZEOF_FSINT_T__    __SIZEOF_KERNEL_FSINT_T__
#   define __SIZEOF_FSUINT_T__   __SIZEOF_KERNEL_FSUINT_T__
#   define __SIZEOF_FSWORD_T__   __SIZEOF_KERNEL_FSWORD_T__
#   define __SIZEOF_INO_T__      __SIZEOF_KERNEL_INO_T__
#   define __SIZEOF_OFF_T__      __SIZEOF_KERNEL_OFF_T__
#   define __SIZEOF_POS_T__      __SIZEOF_KERNEL_POS_T__
#   define __SIZEOF_TIME_T__     __SIZEOF_KERNEL_TIME_T__
#   define __SIZEOF_JTIME_T__    __SIZEOF_KERNEL_JTIME_T__
#   define __SIZEOF_REAL_INO_T__ __SIZEOF_KERNEL_INO_T__
#   define __SIZEOF_REAL_OFF_T__ __SIZEOF_KERNEL_OFF_T__
#endif
#endif /* Additional types... */

#undef __SYSCALL_SLONG_TYPE
#undef __SYSCALL_ULONG_TYPE

__SYSDECL_END

#ifndef __KERNEL__
#include <features.h>
#if defined(__USE_FILE_OFFSET64) || __SIZEOF_SYSCALL_LONG__ >= 8
#   define __FS_TYPE(x)   __##x##64_t
#   define __FS_SIZEOF(x) __SIZEOF_##x##64_T__
#else
#   define __FS_TYPE(x)   __##x##32_t
#   define __FS_SIZEOF(x) __SIZEOF_##x##32_T__
#endif
#ifdef __USE_TIME_BITS64
#   define __TM_TYPE(x)   __##x##64_t
#   define __TM_SIZEOF(x) __SIZEOF_##x##64_T__
#else
#   define __TM_TYPE(x)   __##x##32_t
#   define __TM_SIZEOF(x) __SIZEOF_##x##32_T__
#endif
#else
#   define __FS_TYPE(x)   __##x##_t
#   define __FS_SIZEOF(x) __SIZEOF_##x##_T__
#   define __TM_TYPE(x)   __##x##_t
#   define __TM_SIZEOF(x) __SIZEOF_##x##_T__
#endif

#define __dos_dev_t            __uint32_t
#define __dos_ino_t            __uint16_t
#define __dos_off_t            __int32_t
#define __dos_clock_t          __LONG32_TYPE__
#define __SIZEOF_DOS_DEV_T__   4
#define __SIZEOF_DOS_INO_T__   2
#define __SIZEOF_DOS_OFF_T__   4
#define __SIZEOF_DOS_CLOCK_T__ 4

#ifdef __USE_DOSFS
/* DOS filesystem headers contain different types for these... */
#define __typedef_dev_t            __dos_dev_t
#define __typedef_ino_t            __dos_ino_t
#define __typedef_off_t            __dos_off_t
#define __typedef_clock_t          __dos_clock_t
#define __SIZEOF_TYPEDEF_DEV_T__   __SIZEOF_DOS_DEV_T__
#define __SIZEOF_TYPEDEF_INO_T__   __SIZEOF_DOS_INO_T__
#define __SIZEOF_TYPEDEF_OFF_T__   __SIZEOF_DOS_OFF_T__
#define __SIZEOF_TYPEDEF_CLOCK_T__ __SIZEOF_DOS_CLOCK_T__
#else
#define __typedef_dev_t            __dev_t
#define __typedef_ino_t            __FS_TYPE(ino)
#define __typedef_off_t            __FS_TYPE(off)
#define __typedef_clock_t          __clock_t
#define __SIZEOF_TYPEDEF_DEV_T__   __SIZEOF_DEV_T__
#define __SIZEOF_TYPEDEF_INO_T__   __SIZEOF_INO_T__
#define __SIZEOF_TYPEDEF_OFF_T__   __SIZEOF_OFF_T__
#define __SIZEOF_TYPEDEF_CLOCK_T__ __SIZEOF_CLOCK_T__
#endif



/* === File-system related type, function and assembly names explained ===
 *   - __off32_t:            32-bit filesystem type
 *   - __off64_t:            64-bit filesystem type
 *   - __off_t:              Filesystem type used internally by the kernel
 *                           This type is that used in system calls, which may
 *                           lead to arguments being split among multiple registers.
 *                           The typing of this is determined by `CONFIG_32BIT_FILESYSTEM'.
 *                           WARNING: This type is _NOT_ necessary identical to `off_t'!
 *                           NOTE: User-space applications should never use this type.
 *                                 Instead, they may choose their own filesystem bits
 *                                 by defining `_FILE_OFFSET_BITS'.
 *                             >> `libc' is meant as the interface between kernel and
 *                                 userspace, that is supposed to act as the middle-man
 *                                 for providing an abstract interface that knows how the
 *                                 kernel was compiled in concerns about `CONFIG_32BIT_FILESYSTEM',
 *                                 which determines the ~real~ filesystem type used by the kernel
 *                                 and therefor any kind of system call.
 *   - __FS_TYPE:            Linkage controller for filesystem types (depends on `_FILE_OFFSET_BITS=32|64')
 *   - __FS_TYPE(off):       The effective user-space type.
 *                           This is the implicitly-defined type that headers will
 *                           link c-functions using the type as argument again.
 *   - __REDIRECTFS_FUNC:    Same as `__FS_TYPE', but for assembly symbols linked against c-functions (lseek vs. lseek64)
 *   - off_t:                An alias for `__FS_TYPE(off)' defined by various headers.
 *   - off64_t:              An alias for `__off64_t', defined 
 *   - __ASMNAME("lseek"):   A libc function using __off32_t as argument.
 *   - __ASMNAME("lseek64"): A libc function using __off32_t as argument.
 *   - lseek:                The high-level c-function either linked against
 *                          'lseek' or 'lseek', always taking `off_t' as argument.
 *   - lseek64:              An optional secondary c-function that always links against
 *                           the 64-bit library function, taking `__off64_t' as argument.
 * Within the kernel rules are similar, except that `off64_t'
 * is never used, and `off_t' is always equal to `__off_t'.
 * NOTE: Exactly the same thing is true for 64-bit `time_t'.
 */


#endif /* !_BITS_GENERIC_TYPES_H */
