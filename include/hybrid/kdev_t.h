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
#ifndef GUARD_HYBRID_KDEV_T_H
#define GUARD_HYBRID_KDEV_T_H 1

#include <hybrid/compiler.h>
#include <bits/types.h>

DECL_BEGIN

#ifndef __dev_t_defined
#define __dev_t_defined 1
typedef __typedef_dev_t dev_t;
#endif /* !__dev_t_defined */

#ifndef __major_t_defined
#define __major_t_defined 1
typedef __major_t   major_t;
#endif /* !__major_t_defined */

#ifndef __minor_t_defined
#define __minor_t_defined 1
typedef __minor_t   minor_t;
#endif /* !__minor_t_defined */

#define MINORBITS    20
#define MAJORBITS    12
#define MINORMASK  ((1 << MINORBITS)-1)
#define MAJORMASK  ((1 << MAJORBITS)-1)

#define MAJOR(dev)   ((dev) >> MINORBITS)
#define MINOR(dev)   ((dev) &  MINORMASK)
#define MKDEV(ma,mi) ((ma) << MINORBITS | (mi))


/* KOS non-standard device mappings (major numbers 0x100-0x1ff) */
#define DV_JIFFY_RTC      MKDEV(0x100,0) /* The default RTC synchronized using jiffies. */
#define DV_PS2_KEYBOARD   MKDEV(0x101,0) /* KOS character device for the PS2-compliant keyboards. */


DECL_END

#endif /* !GUARD_HYBRID_KDEV_T_H */
