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
#ifndef _ARM_KOS_BITS_FCNTL_LINUX_H
#define _ARM_KOS_BITS_FCNTL_LINUX_H 1
#define _BITS_FCNTL_LINUX_H 1

#define __O_DIRECTORY 0040000 /*< Must be a directory. */
#define __O_NOFOLLOW  0100000 /*< Do not follow links. */
#define __O_DIRECT    0200000 /*< Direct disk access. */
#define __O_LARGEFILE 0400000

#include <bits-generic/fcntl-linux.h>

#endif /* !_ARM_KOS_BITS_FCNTL_LINUX_H */
