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
#ifndef GUARD_INCLUDE_MODULES_VGA_TTY_H
#define GUARD_INCLUDE_MODULES_VGA_TTY_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/kdev_t.h>

DECL_BEGIN

typedef u16 vtty_char_t;
#define VTTY_COLOR_BLACK          0
#define VTTY_COLOR_BLUE           1
#define VTTY_COLOR_GREEN          2
#define VTTY_COLOR_CYAN           3
#define VTTY_COLOR_RED            4
#define VTTY_COLOR_MAGENTA        5
#define VTTY_COLOR_BROWN          6
#define VTTY_COLOR_LIGHT_GREY     7
#define VTTY_COLOR_DARK_GREY      8
#define VTTY_COLOR_LIGHT_BLUE     9
#define VTTY_COLOR_LIGHT_GREEN    10
#define VTTY_COLOR_LIGHT_CYAN     11
#define VTTY_COLOR_LIGHT_RED      12
#define VTTY_COLOR_LIGHT_MAGENTA  13
#define VTTY_COLOR_LIGHT_BROWN    14
#define VTTY_COLOR_WHITE          15
#define VTTY_COLORS               16
#define vtty_entry_color(fg,bg) ((fg)|(bg) << 4)
#define vtty_entry(uc,color)    ((vtty_char_t)(uc)|(vtty_char_t)(color) << 8)

#define VTTY_DEFAULT_COLOR        vtty_entry_color(VTTY_COLOR_LIGHT_GREY,VTTY_COLOR_BLACK)
#define VTTY_ADDR               ((vtty_char_t *)phys_to_virt(0xB8000))
#define VTTY_WIDTH                80
#define VTTY_HEIGHT               25
#define VTTY_TABSIZE               4
#define VTTY_ENDADDR            (VTTY_ADDR+(VTTY_WIDTH*VTTY_HEIGHT))

DECL_END

#endif /* !GUARD_INCLUDE_MODULES_VGA_TTY_H */
