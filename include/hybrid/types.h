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
#ifndef GUARD_HYBRID_TYPES_H
#define GUARD_HYBRID_TYPES_H 1

#include "compiler.h"
#include "typecore.h"
#include <bits/types.h>
#include <stddef.h>

#ifdef __CC__
DECL_BEGIN

#define KTYPE(x) __##x,x
typedef __INT8_TYPE__   KTYPE(s8);
typedef __INT16_TYPE__  KTYPE(s16);
typedef __INT32_TYPE__  KTYPE(s32);
typedef __INT64_TYPE__  KTYPE(s64);
typedef __UINT8_TYPE__  KTYPE(u8);
typedef __UINT16_TYPE__ KTYPE(u16);
typedef __UINT32_TYPE__ KTYPE(u32);
typedef __UINT64_TYPE__ KTYPE(u64);
#ifdef __INTELLISENSE__
typedef ____INTELLISENSE_integer<1234,__UINT16_TYPE__> KTYPE(le16);
typedef ____INTELLISENSE_integer<4321,__UINT16_TYPE__> KTYPE(be16);
typedef ____INTELLISENSE_integer<1234,__UINT32_TYPE__> KTYPE(le32);
typedef ____INTELLISENSE_integer<4321,__UINT32_TYPE__> KTYPE(be32);
typedef ____INTELLISENSE_integer<1234,__UINT64_TYPE__> KTYPE(le64);
typedef ____INTELLISENSE_integer<4321,__UINT64_TYPE__> KTYPE(be64);
#else
typedef __UINT16_TYPE__ KTYPE(le16),KTYPE(be16);
typedef __UINT32_TYPE__ KTYPE(le32),KTYPE(be32);
typedef __UINT64_TYPE__ KTYPE(le64),KTYPE(be64);
#endif
#undef KTYPE

#ifndef __byte_t_defined
#define __byte_t_defined 1
typedef __BYTE_TYPE__ byte_t;
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
typedef __FS_TYPE(off) off_t;
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
#endif /* !__off_t_defined */

#ifndef __loff_t_defined
#define __loff_t_defined 1
typedef __loff_t    loff_t;
#endif /* !__off_t_defined */

#ifndef __dev_t_defined
#define __dev_t_defined 1
typedef __dev_t     dev_t;
#endif /* !__dev_t_defined */

#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t     pid_t;
#endif /* !__pid_t_defined */

#ifndef __ino_t_defined
#define __ino_t_defined 1
typedef __FS_TYPE(ino) ino_t;
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
typedef __clock_t clock_t;
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

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_HYBRID_TYPES_H */
