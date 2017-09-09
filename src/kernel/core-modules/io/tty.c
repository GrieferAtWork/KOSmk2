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
#ifndef GUARD_KERNEL_CORE_MODULES_TTY_C
#define GUARD_KERNEL_CORE_MODULES_TTY_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>
#include <kernel/boot.h>
#include <kernel/export.h>
#include <kernel/paging.h>
#include <modules/tty.h>
#include <stdint.h>
#include <string.h>
#include <sys/io.h>

DECL_BEGIN

PUBLIC              vtty_char_t  tty_color  = TTY_DEFAULT_COLOR << 8;
PRIVATE ATOMIC_DATA vtty_char_t *tty_curpos = TTY_ADDR;
#define MKCHAR(c) ((vtty_char_t)tty_color | (vtty_char_t)(c))

INTERN MODULE_INIT void KCALL tty_init(void) {
 /* Clear any on-screen bios leftovers. */
 memsetw(tty_curpos,tty_entry(' ',TTY_DEFAULT_COLOR),
        (TTY_ADDR+TTY_WIDTH*TTY_HEIGHT)-tty_curpos);
}


PUBLIC void KCALL tty_putc(char c) {
 vtty_char_t *old_curpus,*curpos;
 if (BOOT_EMULATION_HASLOGPORT(boot_emulation))
     outb(boot_emulation_logport,(unsigned char)c);
again:
#if defined(CONFIG_SMP) && 0
 /* Make sure to read a fully valid value. */
 old_curpus = ATOMIC_CMPXCH_VAL(tty_curpos,0,0);
#else
 old_curpus = ATOMIC_READ(tty_curpos);
#endif
 assert(old_curpus >= TTY_ADDR &&
        old_curpus <= TTY_ENDADDR);
 curpos = old_curpus;
 if (curpos == TTY_ENDADDR) {
  memmove(TTY_ADDR,TTY_ADDR+TTY_WIDTH,
         (TTY_HEIGHT-1)*TTY_WIDTH*
          sizeof(vtty_char_t));
  curpos = TTY_ENDADDR-TTY_WIDTH;
  memsetw(curpos,tty_entry(' ',TTY_DEFAULT_COLOR),TTY_WIDTH);
 }
 switch (c) {
 case '\r':
  curpos = TTY_ADDR+(((curpos-TTY_ADDR)/TTY_WIDTH)*TTY_WIDTH);
  break;
 {
  vtty_char_t *newcur;
 case '\n':
  newcur = TTY_ADDR+((((curpos-TTY_ADDR)+TTY_WIDTH)/TTY_WIDTH)*TTY_WIDTH);
  memsetw(curpos,tty_entry(' ',TTY_DEFAULT_COLOR),(size_t)(newcur-curpos));
  curpos = newcur;
 } break;
 default:
  *curpos++ = MKCHAR(c);
  break;
 }
 if (!ATOMIC_CMPXCH(tty_curpos,old_curpus,curpos))
      goto again;
}

PUBLIC void KCALL
tty_print(char const *__restrict str, size_t len) {
 char const *end = str+len;
 CHECK_HOST_TEXT(str,len);
 for (; str != end; ++str) tty_putc(*str);
}

PUBLIC ssize_t KCALL
tty_printer(char const *__restrict str, size_t len,
            void *UNUSED(closure)) {
 tty_print(str,len);
 return (ssize_t)len;
}

DECL_END

#endif /* !GUARD_KERNEL_CORE_MODULES_TTY_C */
