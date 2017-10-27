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
#ifndef __GUARD_HYBRID_TYPES_H
#define __GUARD_HYBRID_TYPES_H 1

#include "compiler.h"
#include "typecore.h"
#include <bits/types.h>
#include <stddef.h>

#ifdef __CC__
DECL_BEGIN

#ifndef ____suX_defined
#define ____suX_defined 1
#define KTYPE(x) __##x,x
#else
#define KTYPE(x) x
#endif
typedef __INT8_TYPE__   KTYPE(s8);
typedef __INT16_TYPE__  KTYPE(s16);
typedef __INT32_TYPE__  KTYPE(s32);
typedef __INT64_TYPE__  KTYPE(s64);
typedef __UINT8_TYPE__  KTYPE(u8);
typedef __UINT16_TYPE__ KTYPE(u16);
typedef __UINT32_TYPE__ KTYPE(u32);
typedef __UINT64_TYPE__ KTYPE(u64);
#undef KTYPE

#ifndef ____lebesuX_defined
#define ____lebesuX_defined 1
#define KTYPE(x) __##x,x
#else
#define KTYPE(x) x
#endif

#ifdef ____INTELLISENSE_STDINC_SYNTAX_H
typedef ::__int::____INTELLISENSE_integer<1234,__UINT16_TYPE__> KTYPE(le16);
typedef ::__int::____INTELLISENSE_integer<4321,__UINT16_TYPE__> KTYPE(be16);
typedef ::__int::____INTELLISENSE_integer<1234,__UINT32_TYPE__> KTYPE(le32);
typedef ::__int::____INTELLISENSE_integer<4321,__UINT32_TYPE__> KTYPE(be32);
typedef ::__int::____INTELLISENSE_integer<1234,__UINT64_TYPE__> KTYPE(le64);
typedef ::__int::____INTELLISENSE_integer<4321,__UINT64_TYPE__> KTYPE(be64);
#else /* ____INTELLISENSE_STDINC_SYNTAX_H */
typedef __UINT16_TYPE__ KTYPE(le16),KTYPE(be16);
typedef __UINT32_TYPE__ KTYPE(le32),KTYPE(be32);
typedef __UINT64_TYPE__ KTYPE(le64),KTYPE(be64);
#endif /* !____INTELLISENSE_STDINC_SYNTAX_H */
#undef KTYPE

#ifndef __byte_t_defined
#define __byte_t_defined 1
typedef __byte_t      byte_t;
#endif /* !__byte_t_defined */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __size_t      size_t;
#endif /* !__size_t_defined */

#ifndef __ssize_t_defined
#define __ssize_t_defined 1
typedef __ssize_t     ssize_t;
#endif /* !__ssize_t_defined */

#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t    intptr_t;
#endif /* !__intptr_t_defined */

#ifndef __uintptr_t_defined
#define __uintptr_t_defined 1
typedef __uintptr_t   uintptr_t;
#endif /* !__uintptr_t_defined */

#ifndef __cpuid_t_defined
#define __cpuid_t_defined 1
typedef __cpuid_t   cpuid_t;
#endif /* !__cpuid_t_defined */

#ifndef __fsint_t_defined
#define __fsint_t_defined 1
typedef __fsint_t   fsint_t;
#endif /* !__fsint_t_defined */

#ifndef __fsuint_t_defined
#define __fsuint_t_defined 1
typedef __fsuint_t  fsuint_t;
#endif /* !__fsuint_t_defined */

#ifndef __time_t_defined
#define __time_t_defined 1
typedef __TM_TYPE(time) time_t;
#endif /* !__time_t_defined */

#ifndef __time32_t_defined
#define __time32_t_defined 1
typedef __time32_t time32_t;
#endif /* !__time32_t_defined */

#ifndef __time64_t_defined
#define __time64_t_defined 1
typedef __time64_t time64_t;
#endif /* !__time64_t_defined */

/* Special jiffi time values. */
#define JTIME_DONTWAIT   ((jtime_t)0)
#define JTIME_INFINITE   ((jtime_t)-1)
#ifndef __jtime_t_defined
#define __jtime_t_defined 1
/* Jiffy time (measured in seconds*HZ having passed since booting) */
typedef __TM_TYPE(jtime) jtime_t;
#endif /* !__jtime_t_defined */

#ifndef __jtime32_t_defined
#define __jtime32_t_defined 1
typedef __jtime32_t jtime32_t;
#endif /* !__jtime32_t_defined */

#ifndef __jtime64_t_defined
#define __jtime64_t_defined 1
typedef __jtime64_t jtime64_t;
#endif /* !__jtime64_t_defined */

#ifndef __irq_t_defined
#define __irq_t_defined 1
typedef __irq_t     irq_t; /* Processor interrupt number. */
#endif /* !__irq_t_defined */

#ifndef __sysno_t_defined
#define __sysno_t_defined 1
typedef __sysno_t   sysno_t; /* System call number. */
#endif /* !__sysno_t_defined */

#ifndef __oflag_t_defined
#define __oflag_t_defined 1
typedef __oflag_t   oflag_t;
#endif /* !__oflag_t_defined */

#ifndef __mode_t_defined
#define __mode_t_defined 1
typedef __mode_t    mode_t;
#endif /* !__mode_t_defined */

#ifndef __pos_t_defined
#define __pos_t_defined 1
typedef __FS_TYPE(pos) pos_t; /* File position. */
#endif /* !__pos_t_defined */

#ifndef __off_t_defined
#define __off_t_defined 1
typedef __typedef_off_t off_t;
#endif /* !__off_t_defined */

#ifndef __pos32_t_defined
#define __pos32_t_defined 1
typedef __pos32_t   pos32_t; /* File position. */
#endif /* !__pos32_t_defined */

#ifndef __pos64_t_defined
#define __pos64_t_defined 1
typedef __pos64_t   pos64_t; /* File position. */
#endif /* !__pos64_t_defined */

#ifndef __off32_t_defined
#define __off32_t_defined 1
typedef __off32_t   off32_t;
#endif /* !__off32_t_defined */

#ifndef __off64_t_defined
#define __off64_t_defined 1
typedef __off64_t   off64_t;
#endif /* !__off64_t_defined */

#ifndef __lpos_t_defined
#define __lpos_t_defined 1
typedef __lpos_t    lpos_t;
#endif /* !__lpos_t_defined */

#ifndef __loff_t_defined
#define __loff_t_defined 1
typedef __loff_t    loff_t;
#endif /* !__loff_t_defined */

#ifndef __dev_t_defined
#define __dev_t_defined 1
typedef __typedef_dev_t dev_t;
#endif /* !__dev_t_defined */

#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t     pid_t;
#endif /* !__pid_t_defined */

#ifndef __ino_t_defined
#define __ino_t_defined 1
typedef __typedef_ino_t ino_t;
#endif /* !__ino_t_defined */

#ifndef __blkcnt_t_defined
#define __blkcnt_t_defined 1
typedef __FS_TYPE(blkcnt) blkcnt_t;
#endif /* __blkcnt_t_defined */

#ifndef __blkaddr_t_defined
#define __blkaddr_t_defined 1
typedef __FS_TYPE(blkaddr) blkaddr_t;
#endif /* !__blkaddr_t_defined */

#ifndef __blksize_t_defined
#define __blksize_t_defined 1
typedef __blksize_t blksize_t;
#endif /* !__blksize_t_defined */

#ifndef __nlink_t_defined
#define __nlink_t_defined 1
typedef __nlink_t   nlink_t;
#endif /* !__nlink_t_defined */

#ifndef __uid_t_defined
#define __uid_t_defined 1
typedef __uid_t     uid_t;
#endif /* !__uid_t_defined */

#ifndef ___defined
#define __gid_t_defined 1
typedef __gid_t     gid_t;
#endif /* !__gid_t_defined */

#ifndef __ref_t_defined
#define __ref_t_defined 1
typedef __ref_t     ref_t; /* Reference counter. */
#endif /* !__ref_t_defined */

#ifndef __syscall_slong_t_defined
#define __syscall_slong_t_defined 1
typedef __syscall_slong_t syscall_slong_t;
#endif /* !__syscall_slong_t */

#ifndef __syscall_ulong_t_defined
#define __syscall_ulong_t_defined 1
typedef __syscall_ulong_t syscall_ulong_t;
#endif /* !__syscall_ulong_t */

#ifndef __id_t_defined
#define __id_t_defined 1
typedef __id_t id_t;
#endif /* !__id_t_defined */

#ifndef __clock_t_defined
#define __clock_t_defined 1
typedef __typedef_clock_t clock_t;
#endif /* !__clock_t_defined */

#ifndef __clockid_t_defined
#define __clockid_t_defined 1
typedef __clockid_t clockid_t;
#endif /* !__clockid_t_defined */

#ifndef __timer_t_defined
#define __timer_t_defined 1
typedef __timer_t timer_t;
#endif /* !__timer_t_defined */

#ifndef __useconds_t_defined
#define __useconds_t_defined 1
typedef __useconds_t useconds_t;
#endif /* !__useconds_t_defined */

#ifndef __socklen_t_defined
#define __socklen_t_defined 1
typedef __socklen_t socklen_t;
#endif /* !__socklen_t_defined */

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __key_t_defined
#define __key_t_defined 1
typedef __key_t key_t;
#endif /* !__key_t_defined */

#ifndef __blkaddr32_t_defined
#define __blkaddr32_t_defined 1
typedef __blkaddr32_t blkaddr32_t;
#endif /* !__blkaddr32_t_defined */

#ifndef __blkaddr64_t_defined
#define __blkaddr64_t_defined 1
typedef __blkaddr64_t blkaddr64_t;
#endif /* !__blkaddr64_t_defined */

#ifndef __blkcnt32_t_defined
#define __blkcnt32_t_defined 1
typedef __blkcnt32_t blkcnt32_t;
#endif /* !__blkcnt32_t_defined */

#ifndef __blkcnt64_t_defined
#define __blkcnt64_t_defined 1
typedef __blkcnt64_t blkcnt64_t;
#endif /* !__blkcnt64_t_defined */

#ifndef __caddr_t_defined
#define __caddr_t_defined 1
typedef __caddr_t caddr_t;
#endif /* !__caddr_t_defined */

#ifndef __daddr_t_defined
#define __daddr_t_defined 1
typedef __daddr_t daddr_t;
#endif /* !__daddr_t_defined */

#ifndef __fsblkcnt32_t_defined
#define __fsblkcnt32_t_defined 1
typedef __fsblkcnt32_t fsblkcnt32_t;
#endif /* !__fsblkcnt32_t_defined */

#ifndef __fsblkcnt64_t_defined
#define __fsblkcnt64_t_defined 1
typedef __fsblkcnt64_t fsblkcnt64_t;
#endif /* !__fsblkcnt64_t_defined */

#ifndef __fsfilcnt32_t_defined
#define __fsfilcnt32_t_defined 1
typedef __fsfilcnt32_t fsfilcnt32_t;
#endif /* !__fsfilcnt32_t_defined */

#ifndef __fsfilcnt64_t_defined
#define __fsfilcnt64_t_defined 1
typedef __fsfilcnt64_t fsfilcnt64_t;
#endif /* !__fsfilcnt64_t_defined */

#ifndef __fsid_t_defined
#define __fsid_t_defined 1
typedef __fsid_t fsid_t;
#endif /* !__fsid_t_defined */

#ifndef __fsint32_t_defined
#define __fsint32_t_defined 1
typedef __fsint32_t fsint32_t;
#endif /* !__fsint32_t_defined */

#ifndef __fsint64_t_defined
#define __fsint64_t_defined 1
typedef __fsint64_t fsint64_t;
#endif /* !__fsint64_t_defined */

#ifndef __fsuint32_t_defined
#define __fsuint32_t_defined 1
typedef __fsuint32_t fsuint32_t;
#endif /* !__fsuint32_t_defined */

#ifndef __fsuint64_t_defined
#define __fsuint64_t_defined 1
typedef __fsuint64_t fsuint64_t;
#endif /* !__fsuint64_t_defined */

#ifndef __fsword32_t_defined
#define __fsword32_t_defined 1
typedef __fsword32_t fsword32_t;
#endif /* !__fsword32_t_defined */

#ifndef __fsword64_t_defined
#define __fsword64_t_defined 1
typedef __fsword64_t fsword64_t;
#endif /* !__fsword64_t_defined */

#ifndef __ino32_t_defined
#define __ino32_t_defined 1
typedef __ino32_t ino32_t;
#endif /* !__ino32_t_defined */

#ifndef __ino64_t_defined
#define __ino64_t_defined 1
typedef __ino64_t ino64_t;
#endif /* !__ino64_t_defined */

#ifndef __qaddr_t_defined
#define __qaddr_t_defined 1
typedef __qaddr_t qaddr_t;
#endif /* !__qaddr_t_defined */

#ifndef __rlim32_t_defined
#define __rlim32_t_defined 1
typedef __rlim32_t rlim32_t;
#endif /* !__rlim32_t_defined */

#ifndef __rlim64_t_defined
#define __rlim64_t_defined 1
typedef __rlim64_t rlim64_t;
#endif /* !__rlim64_t_defined */

#ifndef __rlim_t_defined
#define __rlim_t_defined 1
typedef __FS_TYPE(rlim) rlim_t;
#endif /* !__rlim_t_defined */

#ifndef __suseconds_t_defined
#define __suseconds_t_defined 1
typedef __suseconds_t suseconds_t;
#endif /* !__suseconds_t_defined */

#ifndef __major_t_defined
#define __major_t_defined 1
typedef __major_t major_t;
#endif /* !__major_t_defined */

#ifndef __minor_t_defined
#define __minor_t_defined 1
typedef __minor_t minor_t;
#endif /* !__minor_t_defined */

#ifndef __fsblkcnt_t_defined
#define __fsblkcnt_t_defined 1
typedef __FS_TYPE(fsblkcnt) fsblkcnt_t;
#endif /* !__fsblkcnt_t_defined */
#ifndef __fsfilcnt_t_defined
#define __fsfilcnt_t_defined 1
typedef __FS_TYPE(fsfilcnt) fsfilcnt_t;
#endif /* !__fsfilcnt_t_defined */
#ifndef __fsword_t_defined
#define __fsword_t_defined 1
typedef __FS_TYPE(fsword) fsword_t;
#endif /* !__fsword_t_defined */

#if 0
#ifndef __u_char_defined
#define __u_char_defined 1
typedef __u_char u_char;
#endif /* !__u_char_defined */
#ifndef __u_int_defined
#define __u_int_defined 1
typedef __u_int u_int;
#endif /* !__u_int_defined */
#ifndef __u_long_defined
#define __u_long_defined 1
typedef __u_long u_long;
#endif /* !__u_long_defined */
#ifndef __u_quad_t_defined
#define __u_quad_t_defined 1
typedef __u_quad_t u_quad_t;
#endif /* !__u_quad_t_defined */
#ifndef __u_short_defined
#define __u_short_defined 1
typedef __u_short u_short;
#endif /* !__u_short_defined */
#ifndef __quad_t_defined
#define __quad_t_defined 1
typedef __quad_t quad_t;
#endif /* !__quad_t_defined */
#endif

DECL_END
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_TYPES_H */
