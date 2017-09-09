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
#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H 1

#include <__stdinc.h>
#include <bits/types.h>

__DECL_BEGIN

__LIBC __ATTR_CONST __major_t (__LIBCCALL gnu_dev_major)(__dev_t __dev);
__LIBC __ATTR_CONST __minor_t (__LIBCCALL gnu_dev_minor)(__dev_t __dev);
__LIBC __ATTR_CONST __dev_t (__LIBCCALL gnu_dev_makedev)(__major_t __major, __minor_t __minor);

/* Access the functions with their traditional names.  */
#define major(dev)       gnu_dev_major(dev)
#define minor(dev)       gnu_dev_minor(dev)
#define makedev(maj,min) gnu_dev_makedev(maj,min)


__DECL_END

#endif /* !_SYS_SYSMACROS_H */
