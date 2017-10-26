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
#ifndef GUARD_KERNEL_CORE_VERSION_C
#define GUARD_KERNEL_CORE_VERSION_C 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>
#include <sys/utsname.h>
#include <kernel/version.h>

DECL_BEGIN

INTDEF byte_t __kernel_timestamp_lo[];
INTDEF byte_t __kernel_timestamp_hi[];

/* Kernel build time is powered by magic(.dee) and the linker script. */
PUBLIC struct { u32 lo,hi; }
kernel_timestamp_ ASMNAME("kernel_timestamp") = {
    .lo = (u32)(uintptr_t)__kernel_timestamp_lo,
    .hi = (u32)(uintptr_t)__kernel_timestamp_hi,
};

PUBLIC u32 const kernel_version = KOS_VERSION_CODE;

/* Define global kernel information available through 'uname()' in userspace.
 * HINT: This information is shared through the user-share facility,
 *       meaning there is only ever a single instance of this variable! */
PUBLIC ATTR_USED ATTR_SECTION(".rodata.user")
struct utsname const kernel_uname ASMNAME("uname") = {
    .sysname    = "KOS",
    .release    = PP_STR(KOS_VERSION_MAJOR),
    .version    = PP_STR(KOS_VERSION_MINOR),
    .nodename   = KOS_VERSION_NAME,
    .domainname = KOS_VERSION_NAME "-net",
#ifdef __i386__
    .machine    = "i386",
#elif defined(__x86_64__)
    .machine    = "x86-64",
#else
#warning "Unknown host architecture"
    .machine    = "UNKNOWN",
#endif
};

DECL_END

#endif /* !GUARD_KERNEL_CORE_VERSION_C */
