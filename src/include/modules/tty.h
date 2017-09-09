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
#ifndef GUARD_INCLUDE_MODULES_TTY_H
#define GUARD_INCLUDE_MODULES_TTY_H 1

#include <format-printer.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stddef.h>
#include <hybrid/atomic.h>
#include <modules/vga.h>

DECL_BEGIN

FUNDEF void KCALL tty_putc(char c);
FUNDEF void KCALL tty_print(char const *__restrict str, size_t len);
FUNDEF ssize_t KCALL tty_printer(char const *__restrict str, size_t len, void *UNUSED(closure));

#define tty_printf(...)          format_printf(&tty_printer,NULL,__VA_ARGS__)
#define tty_vprintf(format,args) format_vprintf(&tty_printer,NULL,format,args)


#define TTY_COLOR_BLACK         VTTY_COLOR_BLACK
#define TTY_COLOR_BLUE          VTTY_COLOR_BLUE
#define TTY_COLOR_GREEN         VTTY_COLOR_GREEN
#define TTY_COLOR_CYAN          VTTY_COLOR_CYAN
#define TTY_COLOR_RED           VTTY_COLOR_RED
#define TTY_COLOR_MAGENTA       VTTY_COLOR_MAGENTA
#define TTY_COLOR_BROWN         VTTY_COLOR_BROWN
#define TTY_COLOR_LIGHT_GREY    VTTY_COLOR_LIGHT_GREY
#define TTY_COLOR_DARK_GREY     VTTY_COLOR_DARK_GREY
#define TTY_COLOR_LIGHT_BLUE    VTTY_COLOR_LIGHT_BLUE
#define TTY_COLOR_LIGHT_GREEN   VTTY_COLOR_LIGHT_GREEN
#define TTY_COLOR_LIGHT_CYAN    VTTY_COLOR_LIGHT_CYAN
#define TTY_COLOR_LIGHT_RED     VTTY_COLOR_LIGHT_RED
#define TTY_COLOR_LIGHT_MAGENTA VTTY_COLOR_LIGHT_MAGENTA
#define TTY_COLOR_LIGHT_BROWN   VTTY_COLOR_LIGHT_BROWN
#define TTY_COLOR_WHITE         VTTY_COLOR_WHITE
#define TTY_COLORS              VTTY_COLORS
#define tty_entry_color         vtty_entry_color
#define tty_entry               vtty_entry
#define TTY_DEFAULT_COLOR       VTTY_DEFAULT_COLOR
#define TTY_ADDR                VTTY_ADDR
#define TTY_WIDTH               VTTY_WIDTH
#define TTY_HEIGHT              VTTY_HEIGHT
#define TTY_TABSIZE             VTTY_TABSIZE
#define TTY_ENDADDR             VTTY_ENDADDR

DATDEF vtty_char_t tty_color;

#define TTY_PUSHCOLOR(c) \
do{ vtty_char_t const _old_color = ATOMIC_XCH(tty_color,(vtty_char_t)(c) << 8)
#define TTY_POPCOLOR() \
    ATOMIC_WRITE(tty_color,_old_color); \
}while(0)


DECL_END

#endif /* !GUARD_INCLUDE_MODULES_TTY_H */
