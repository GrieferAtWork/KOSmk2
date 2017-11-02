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
#ifndef _KOS_VGA_H
#define _KOS_VGA_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

#define VGA_DEVICE  "/dev/vga"

/* ioctl() commands. */
#define VIO_SETMODE 0x3701 /* Set the current video mode. */
#   define VIO_MODE_TEXT_COLOR_80X25 0 /* 80x25 color text mode. */
#   define VIO_MODE_GFX_256_320X200  1 /* 320x200, standard VGA 256-color graphics mode. */
#   define VIO_MODE_COUNT            2
#define VIO_SETPAL  0x3702 /* Set the current video palette. (The argument is a pointer to 'vio_pal_t') */
typedef byte_t vio_pal_t[256][3];
/* use mmap() to map VGA memory in userspace! */


DECL_END

#endif /* !_KOS_VGA_H */
