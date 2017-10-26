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
#ifndef GUARD_INCLUDE_KERNEL_VERSION_H
#define GUARD_INCLUDE_KERNEL_VERSION_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

#define KERNEL_VERSION(maj,min,pat) ((maj) << 20 | (min) << 10 | (pat))

/* Expanded kernel version numbers. */
#define KOS_VERSION_MAJOR 0
#define KOS_VERSION_MINOR 1
#define KOS_VERSION_PATCH 0

#define KOS_VERSION_CODE     \
  KERNEL_VERSION(KOS_VERSION_MAJOR,KOS_VERSION_MINOR,KOS_VERSION_PATCH)
#define KOS_VERSION_NAME     "devan" /* Is it pronounced Deven, or Devaaan? */

#ifdef __CC__
struct utsname;

/* Unix-time timestamp when the kernel was built. */
DATDEF time_t   const kernel_timestamp;
DATDEF time32_t const kernel_timestamp32 ASMNAME("kernel_timestamp");
DATDEF time64_t const kernel_timestamp64 ASMNAME("kernel_timestamp");

/* Effective kernel version.
 * Usually the same as 'KOS_VERSION_*', but can be used by
 * modules to detect version differences between what they
 * were compiled for and what they are actually running under.
 * HINT: This value changes every time the kernel core is relinked! */
DATDEF u32 const kernel_version;

/* 'uname -a' style information, identical to what is available from user-space.
 *  NOTE: This block of data is allocated in the user-share segment! */
DATDEF struct utsname const kernel_uname ASMNAME("uname");
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_VERSION_H */
