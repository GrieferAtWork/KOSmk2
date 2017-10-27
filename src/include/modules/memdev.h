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
#ifndef GUARD_INCLUDE_MODULES_MEMDEV_H
#define GUARD_INCLUDE_MODULES_MEMDEV_H 1

#include <hybrid/compiler.h>
#include <hybrid/kdev_t.h>

DECL_BEGIN

/* Special memory devices. */
#define MD_MEM     MKDEV(1,1)  /*< /dev/mem      Physical memory access. */
#define MD_KMEM    MKDEV(1,2)  /*< /dev/kmem     Kernel virtual memory access. */
#define MD_NULL    MKDEV(1,3)  /*< /dev/null     Null device. */
#define MD_PORT    MKDEV(1,4)  /*< /dev/port     I/O port access. */
#define MD_ZERO    MKDEV(1,5)  /*< /dev/zero     Null byte source. */
#define MD_CORE    MKDEV(1,6)  /*< /dev/core     OBSOLETE - replaced by /proc/kcore. */
#define MD_FULL    MKDEV(1,7)  /*< /dev/full     Returns ENOSPC on write. */
#define MD_RANDOM  MKDEV(1,8)  /*< /dev/random   Nondeterministic random number gen. */
#define MD_URANDOM MKDEV(1,9)  /*< /dev/urandom  Faster, less secure random number gen. */
#define MD_AIO     MKDEV(1,10) /*< /dev/aio      Asynchronous I/O notification interface. */
#define MD_KMSG    MKDEV(1,11) /*< /dev/kmsg     Writes to this come out as `syslog()'s. */

DECL_END

#endif /* !GUARD_INCLUDE_MODULES_MEMDEV_H */
