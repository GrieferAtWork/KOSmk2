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
#ifndef GUARD_ARCH_ARM_KOS_KERNEL_MEMORY_DECTECT_C
#define GUARD_ARCH_ARM_KOS_KERNEL_MEMORY_DECTECT_C 1
#define _KOS_SOURCE 1

#include <hybrid/align.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/boot.h>
#include <syslog.h>

DECL_BEGIN

INTDEF ATTR_FREETEXT SAFE KPD
size_t KCALL memory_load_detect(void) {
 /* TODO */

 return 0;
}


DEFINE_EARLY_SETUP_NOARG("detect-memory",detect_memory) {
 /* Detect additional memory. */
 if (!memory_load_detect())
      syslog(LOG_MEM|LOG_INFO,SETUPSTR("[CMD] No additional memory detected"));
 return true;
}

DECL_END

#endif /* !GUARD_ARCH_ARM_KOS_KERNEL_MEMORY_DECTECT_C */
