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
#ifndef GUARD_APPS_TERMINAL_VGA_MAIN_C
#define GUARD_APPS_TERMINAL_VGA_MAIN_C 1
#define _KOS_SOURCE 1

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <format-printer.h>
#include <hybrid/atomic.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>
#include <kos/keyboard.h>
#include <sys/syslog.h>
#include <limits.h>
#include <pty.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include "libterm.h"
#include "../../src/include/modules/vga.h"


DECL_BEGIN

PRIVATE struct term_rgba vga_colors[VTTY_COLORS] = {
#if 0 /* Perfect match for xterm 256 */
    [VTTY_COLOR_BLACK         ] = TERM_RGBA_INIT(0x00,0x00,0x00,0xff), // Black
    [VTTY_COLOR_BLUE          ] = TERM_RGBA_INIT(0x00,0x00,0x80,0xff), // Navy
    [VTTY_COLOR_GREEN         ] = TERM_RGBA_INIT(0x00,0x80,0x00,0xff), // Green
    [VTTY_COLOR_CYAN          ] = TERM_RGBA_INIT(0x00,0x80,0x80,0xff), // Teal
    [VTTY_COLOR_RED           ] = TERM_RGBA_INIT(0x80,0x00,0x00,0xff), // Maroon
    [VTTY_COLOR_MAGENTA       ] = TERM_RGBA_INIT(0x80,0x00,0x80,0xff), // Purple
    [VTTY_COLOR_BROWN         ] = TERM_RGBA_INIT(0x80,0x80,0x00,0xff), // Olive
    [VTTY_COLOR_LIGHT_GREY    ] = TERM_RGBA_INIT(0xc0,0xc0,0xc0,0xff), // Silver
    [VTTY_COLOR_DARK_GREY     ] = TERM_RGBA_INIT(0x80,0x80,0x80,0xff), // Grey
    [VTTY_COLOR_LIGHT_BLUE    ] = TERM_RGBA_INIT(0x00,0x00,0xff,0xff), // Blue
    [VTTY_COLOR_LIGHT_GREEN   ] = TERM_RGBA_INIT(0x00,0xff,0x00,0xff), // Lime
    [VTTY_COLOR_LIGHT_CYAN    ] = TERM_RGBA_INIT(0x00,0xff,0xff,0xff), // Aqua
    [VTTY_COLOR_LIGHT_RED     ] = TERM_RGBA_INIT(0xff,0x00,0x00,0xff), // Red
    [VTTY_COLOR_LIGHT_MAGENTA ] = TERM_RGBA_INIT(0xff,0x00,0xff,0xff), // Fuchsia
    [VTTY_COLOR_LIGHT_BROWN   ] = TERM_RGBA_INIT(0xff,0xff,0x00,0xff), // Yellow
    [VTTY_COLOR_WHITE         ] = TERM_RGBA_INIT(0xff,0xff,0xff,0xff), // White
#elif 0 /* Actual colors used by QEMU */
    [VTTY_COLOR_BLACK         ] = TERM_RGBA_INIT(0x00,0x00,0x00,0xff), // Black
    [VTTY_COLOR_BLUE          ] = TERM_RGBA_INIT(0x00,0x00,0xa8,0xff), // Navy
    [VTTY_COLOR_GREEN         ] = TERM_RGBA_INIT(0x00,0xa8,0x00,0xff), // Green
    [VTTY_COLOR_CYAN          ] = TERM_RGBA_INIT(0x00,0xa8,0xa8,0xff), // Teal
    [VTTY_COLOR_RED           ] = TERM_RGBA_INIT(0xa8,0x00,0x00,0xff), // Maroon
    [VTTY_COLOR_MAGENTA       ] = TERM_RGBA_INIT(0xa8,0x00,0xa8,0xff), // Purple
    [VTTY_COLOR_BROWN         ] = TERM_RGBA_INIT(0xa8,0x57,0x00,0xff), // Olive
    [VTTY_COLOR_LIGHT_GREY    ] = TERM_RGBA_INIT(0xa8,0xa8,0xa8,0xff), // Silver
    [VTTY_COLOR_DARK_GREY     ] = TERM_RGBA_INIT(0x57,0x57,0x57,0xff), // Grey
    [VTTY_COLOR_LIGHT_BLUE    ] = TERM_RGBA_INIT(0x57,0x57,0xff,0xff), // Blue
    [VTTY_COLOR_LIGHT_GREEN   ] = TERM_RGBA_INIT(0x57,0xff,0x57,0xff), // Lime
    [VTTY_COLOR_LIGHT_CYAN    ] = TERM_RGBA_INIT(0x57,0xff,0xff,0xff), // Aqua
    [VTTY_COLOR_LIGHT_RED     ] = TERM_RGBA_INIT(0xff,0x57,0x57,0xff), // Red
    [VTTY_COLOR_LIGHT_MAGENTA ] = TERM_RGBA_INIT(0xff,0x57,0xff,0xff), // Fuchsia
    [VTTY_COLOR_LIGHT_BROWN   ] = TERM_RGBA_INIT(0xff,0xff,0x57,0xff), // Yellow
    [VTTY_COLOR_WHITE         ] = TERM_RGBA_INIT(0xff,0xff,0xff,0xff), // White
#elif 1 /* Aproximation for best results (Use this!) */
    [VTTY_COLOR_BLACK         ] = TERM_RGBA_INIT(0x00,0x00,0x00,0xff), // Black
    [VTTY_COLOR_BLUE          ] = TERM_RGBA_INIT(0x00,0x00,0xAA,0xff), // Navy
    [VTTY_COLOR_GREEN         ] = TERM_RGBA_INIT(0x00,0xAA,0x00,0xff), // Green
    [VTTY_COLOR_CYAN          ] = TERM_RGBA_INIT(0x00,0xAA,0xAA,0xff), // Teal
    [VTTY_COLOR_RED           ] = TERM_RGBA_INIT(0xAA,0x00,0x00,0xff), // Maroon
    [VTTY_COLOR_MAGENTA       ] = TERM_RGBA_INIT(0xAA,0x00,0xAA,0xff), // Purple
    [VTTY_COLOR_BROWN         ] = TERM_RGBA_INIT(0xAA,0xAA,0x00,0xff), // Olive
    [VTTY_COLOR_LIGHT_GREY    ] = TERM_RGBA_INIT(0xAA,0xAA,0xAA,0xff), // Silver
    [VTTY_COLOR_DARK_GREY     ] = TERM_RGBA_INIT(0x80,0x80,0x80,0xff), // Grey
    [VTTY_COLOR_LIGHT_BLUE    ] = TERM_RGBA_INIT(0x00,0x00,0xff,0xff), // Blue
    [VTTY_COLOR_LIGHT_GREEN   ] = TERM_RGBA_INIT(0x00,0xff,0x00,0xff), // Lime
    [VTTY_COLOR_LIGHT_CYAN    ] = TERM_RGBA_INIT(0x00,0xff,0xff,0xff), // Aqua
    [VTTY_COLOR_LIGHT_RED     ] = TERM_RGBA_INIT(0xff,0x00,0x00,0xff), // Red
    [VTTY_COLOR_LIGHT_MAGENTA ] = TERM_RGBA_INIT(0xff,0x00,0xff,0xff), // Fuchsia
    [VTTY_COLOR_LIGHT_BROWN   ] = TERM_RGBA_INIT(0xff,0xff,0x00,0xff), // Yellow
    [VTTY_COLOR_WHITE         ] = TERM_RGBA_INIT(0xff,0xff,0xff,0xff), // White
#else /* Color codes according to VGA? (I think...) */
    [VTTY_COLOR_BLACK         ] = TERM_RGBA_INIT(0x00,0x00,0x00,0xff),
    [VTTY_COLOR_BLUE          ] = TERM_RGBA_INIT(0x00,0x00,0xaa,0xff),
    [VTTY_COLOR_GREEN         ] = TERM_RGBA_INIT(0x00,0xaa,0x00,0xff),
    [VTTY_COLOR_CYAN          ] = TERM_RGBA_INIT(0x00,0xaa,0xaa,0xff),
    [VTTY_COLOR_RED           ] = TERM_RGBA_INIT(0xaa,0x00,0x00,0xff),
    [VTTY_COLOR_MAGENTA       ] = TERM_RGBA_INIT(0xaa,0x00,0xaa,0xff),
    [VTTY_COLOR_BROWN         ] = TERM_RGBA_INIT(0xaa,0x55,0x00,0xff),
    [VTTY_COLOR_LIGHT_GREY    ] = TERM_RGBA_INIT(0xaa,0xaa,0xaa,0xff),
    [VTTY_COLOR_DARK_GREY     ] = TERM_RGBA_INIT(0x80,0x55,0x55,0xff),
    [VTTY_COLOR_LIGHT_BLUE    ] = TERM_RGBA_INIT(0x55,0x55,0xff,0xff),
    [VTTY_COLOR_LIGHT_GREEN   ] = TERM_RGBA_INIT(0x55,0xaa,0x55,0xff),
    [VTTY_COLOR_LIGHT_CYAN    ] = TERM_RGBA_INIT(0x55,0xff,0xff,0xff),
    [VTTY_COLOR_LIGHT_RED     ] = TERM_RGBA_INIT(0xff,0x55,0x55,0xff),
    [VTTY_COLOR_LIGHT_MAGENTA ] = TERM_RGBA_INIT(0xff,0x55,0xff,0xff),
    [VTTY_COLOR_LIGHT_BROWN   ] = TERM_RGBA_INIT(0xff,0xff,0x55,0xff),
    [VTTY_COLOR_WHITE         ] = TERM_RGBA_INIT(0xff,0xff,0xff,0xff),
#endif
};

/* UGH... Doing this correctly would have waaaay too huge of an overhead.
   But I have to admit that this approximation is pretty CRAP... */
#define sqr(x) ((x)*(x))
#if 0
#define rgba_distance(a,b) (unsigned int)\
 (sqr(((int)(b).sr-(int)(a).sr)*30)+\
  sqr(((int)(b).sg-(int)(a).sg)*59)+\
  sqr(((int)(b).sb-(int)(a).sb)*11))
#elif 1
#define rgba_distance(a,b) (unsigned int)\
 (sqr((a).sr-(b).sr)+\
  sqr((a).sg-(b).sg)+\
  sqr((a).sb-(b).sb))
#else
#define rgba_distance(a,b) (unsigned int)\
 (abs((a).sr-(b).sr)+\
  abs((a).sg-(b).sg)+\
  abs((a).sb-(b).sb))
#endif


LOCAL u8 vga_color(struct term_rgba const cl) {
 unsigned int d,winner = UINT_MAX;
 u8 i = 0,result = 0;
 for (; i < VTTY_COLORS; ++i) {
  d = rgba_distance(cl,vga_colors[i]);
  if (d < winner) result = i,winner = d;
 }
#if 0
 k_syslogf(KLOG_MSG,
           "COLOR: %u {%I8x,%I8x,%I8x} -> {%I8x,%I8x,%I8x}\n",
           winner,cl.r,cl.g,cl.b,
           vga_colors[result].r,
           vga_colors[result].g,
           vga_colors[result].b);
#endif
 return result;
}


typedef uint16_t cell_t;

/* user-level mappings of the VGA physical memory.
 * NOTE: This memory should be considered write-only. */
PRIVATE int     vga_fd;
PRIVATE cell_t *vga_dev;
PRIVATE cell_t *vga_buf,*vga_bufpos,*vga_bufend;
PRIVATE cell_t *vga_bufsecondline,*vga_lastline;
#define vga_spaceline   vga_bufend
PRIVATE cell_t  vga_attrib;
#define VGA_SIZE   (VTTY_WIDTH*VTTY_HEIGHT)
#define CHR(x)    ((cell_t)(unsigned char)(x)|vga_attrib)
#define SPACE       CHR(' ')
#define INVCHR(x) ((cell_t)(unsigned char)(x)\
   | ((cell_t)vga_invert[(vga_attrib&0xf000) >> 12] << 12)\
   | ((cell_t)vga_invert[(vga_attrib&0x0f00) >> 8] << 8))


// Cell/Line access
#define DEV_CELL(x,y)  (vga_dev+(x)+(y)*VTTY_WIDTH)
#define BUF_CELL(x,y)  (vga_buf+(x)+(y)*VTTY_WIDTH)
#define DEV_LINE(y)    (vga_dev+(y)*VTTY_WIDTH)
#define BUF_LINE(y)    (vga_buf+(y)*VTTY_WIDTH)
#define DEV_CELL_CUR() (vga_dev+(vga_bufpos-vga_buf))
#define BUF_CELL_CUR() (vga_bufpos)

// Blit commands (swap buffers)
#define BLIT_CNT(start,count)  memcpy(vga_dev+(start),vga_buf+(start),(count)*sizeof(cell_t))
#define BLIT()                 BLIT_CNT(0,VGA_SIZE)
#define BLIT_LINE(y)           memcpy(DEV_LINE(y),BUF_LINE(y),VTTY_WIDTH*sizeof(cell_t))
#define BLIT_CELL(x,y)        (*DEV_CELL(x,y) = *BUF_CELL(x,y))
#define BLIT_CUR()            (*DEV_CELL_CUR() = *BUF_CELL_CUR())
#define BLIT_BUF(buf)         (vga_dev[(buf)-vga_buf] = *(buf))
#define BLIT_CUR_INV()        (*DEV_CELL_CUR() = INVERT(*BUF_CELL_CUR()))

// Get/Set/Move the cursor
#define GET_CUR_X()    (coord_t)((vga_bufpos-vga_buf)%VTTY_WIDTH)
#define GET_CUR_Y()    (coord_t)((vga_bufpos-vga_buf)/VTTY_WIDTH)
#define SET_CUR(x,y)   (vga_bufpos = vga_buf+((x)+(y)*VTTY_WIDTH))
#define SET_CUR_X(x)   (vga_bufpos -= (((vga_bufpos-vga_buf) % VTTY_WIDTH)-(x)))
#define SET_CUR_Y(y)    SET_CUR(GET_CUR_X(),y)
#define MOV_CUR(ox,oy) (vga_bufpos += ((ox)+(oy)*VTTY_WIDTH))
#define MOV_CUR_X(ox)  (vga_bufpos += (ox))
#define MOV_CUR_Y(oy)  (vga_bufpos += (ox)*VTTY_WIDTH)
#define CUR_LINE()     (vga_bufpos-((vga_bufpos-vga_buf) % VTTY_WIDTH))

#define CR()            SET_CUR_X(0)
#define LF()           (vga_bufpos += (VTTY_WIDTH-((vga_bufpos-vga_buf) % VTTY_WIDTH)))
#define BACK()         (vga_bufpos != vga_buf ? --vga_bufpos : vga_bufpos)

/* Terminal configuration */
#define HAVE_CURSOR_BLINK        1
#define HAVE_CURSOR_BLINK_THREAD 0
#define HAVE_CURSOR_LOCK         1
#define INVERT_CURSOR_AFTER_MOVE 1
#define TTY_DEVNAME              "/dev/vga-tty"
#undef TERM_BELL


#if !HAVE_CURSOR_BLINK
#undef HAVE_CURSOR_BLINK_THREAD
#define HAVE_CURSOR_BLINK_THREAD 0
#undef INVERT_CURSOR_AFTER_MOVE
#define INVERT_CURSOR_AFTER_MOVE 0
#undef HAVE_CURSOR_LOCK
#define HAVE_CURSOR_LOCK 0
#endif

PRIVATE             pid_t relay_incoming_thread;
#if HAVE_CURSOR_BLINK_THREAD
PRIVATE             pid_t cursor_blink_thread;
#endif
#if HAVE_CURSOR_BLINK
PRIVATE ATOMIC_DATA int   cursor_blink_enabled = 1;
PRIVATE             int   cursor_inverted = 0;
#endif

#if HAVE_CURSOR_LOCK
PRIVATE DEFINE_ATOMIC_RWLOCK(cursor_lock);
#define CURSOR_LOCK   atomic_rwlock_write(&cursor_lock);
#define CURSOR_UNLOCK atomic_rwlock_endwrite(&cursor_lock);
#else
#define CURSOR_LOCK   /* nothing */
#define CURSOR_UNLOCK /* nothing */
#endif



#if defined(TERM_BELL) || HAVE_CURSOR_BLINK
PRIVATE u8 vga_invert[VTTY_COLORS] = {
    [VTTY_COLOR_BLACK]          = VTTY_COLOR_WHITE,
    [VTTY_COLOR_BLUE]           = VTTY_COLOR_LIGHT_BROWN,
    [VTTY_COLOR_GREEN]          = VTTY_COLOR_LIGHT_MAGENTA,
    [VTTY_COLOR_CYAN]           = VTTY_COLOR_LIGHT_RED,
    [VTTY_COLOR_RED]            = VTTY_COLOR_LIGHT_CYAN,
    [VTTY_COLOR_MAGENTA]        = VTTY_COLOR_LIGHT_GREEN,
    [VTTY_COLOR_BROWN]          = VTTY_COLOR_LIGHT_BLUE,
    [VTTY_COLOR_LIGHT_GREY]     = VTTY_COLOR_DARK_GREY,
    [VTTY_COLOR_DARK_GREY]      = VTTY_COLOR_LIGHT_GREY,
    [VTTY_COLOR_LIGHT_BLUE]     = VTTY_COLOR_BROWN,
    [VTTY_COLOR_LIGHT_GREEN]    = VTTY_COLOR_MAGENTA,
    [VTTY_COLOR_LIGHT_CYAN]     = VTTY_COLOR_RED,
    [VTTY_COLOR_LIGHT_RED]      = VTTY_COLOR_CYAN,
    [VTTY_COLOR_LIGHT_MAGENTA]  = VTTY_COLOR_GREEN,
    [VTTY_COLOR_LIGHT_BROWN]    = VTTY_COLOR_BLUE,
    [VTTY_COLOR_WHITE]          = VTTY_COLOR_BLACK,
};
#endif

#if HAVE_CURSOR_BLINK
PRIVATE void cursor_enable(void) {
 if (!ATOMIC_CMPXCH(cursor_blink_enabled,0,1)) return;
#if HAVE_CURSOR_BLINK_THREAD
 kill(cursor_blink_thread,SIGCONT);
#endif
}
PRIVATE void cursor_disable(void) {
 if (!ATOMIC_CMPXCH(cursor_blink_enabled,1,0)) return;
#if HAVE_CURSOR_BLINK_THREAD
 kill(cursor_blink_thread,SIGSTOP);
#endif
 if (cursor_inverted) {
  /* Fix inverted cursor */
  BLIT_CUR();
  cursor_inverted = 0;
 }
}
#define BEGIN_MOVE_CUR()      { CURSOR_LOCK if (cursor_inverted) BLIT_CUR(); }
#define END_MOVE_CUR()        { CURSOR_UNLOCK }
#else
#define cursor_enable()  (void)0
#define cursor_disable() (void)0
#define BEGIN_MOVE_CUR()      { CURSOR_LOCK }
#define END_MOVE_CUR()        { CURSOR_UNLOCK }
#endif

#define INVERT(cell) \
 (((cell)&0xff)\
   | ((cell_t)vga_invert[((cell)&0xf000) >> 12] << 12)\
   | ((cell_t)vga_invert[((cell)&0x0f00) >> 8] << 8))


#if HAVE_CURSOR_BLINK_THREAD
PRIVATE void blink_cur(void) {
 cell_t cur;
 CURSOR_LOCK
 assert(vga_bufpos <= vga_bufend);
 if (vga_bufpos != vga_bufend) {
  cur = *BUF_CELL_CUR();
  cursor_inverted ^= 1;
  if (cursor_inverted) cur = INVERT(cur);
  *DEV_CELL_CUR() = cur;
 }
 CURSOR_UNLOCK
}
PRIVATE ATTR_NORETURN void cursor_blink_threadmain(void) {
 for (;;) { usleep(300000); blink_cur(); }
}
#endif /* HAVE_CURSOR_BLINK_THREAD */

#ifdef TERM_BELL
PRIVATE void blit_inverted(void) {
 cell_t *iter,*end,*dst,cell;
 iter = vga_buf,end = vga_bufend;
 dst = vga_dev;
 for (; iter != end; ++iter,++dst) {
  cell = *iter;
  cell = INVERT(cell);
  *dst = cell;
 }
}
#endif


PRIVATE void term_doscroll(void) {
 memmove(vga_buf,vga_bufsecondline,
        (VGA_SIZE-VTTY_WIDTH)*sizeof(cell_t));
 memcpy(vga_lastline,vga_spaceline,VTTY_WIDTH*sizeof(cell_t));
 vga_bufpos = vga_lastline;
}
PRIVATE void term_doput(char ch) {
 if (vga_bufpos == vga_bufend) {
  /* Scroll at the end of the terminal */
  term_doscroll();
#if INVERT_CURSOR_AFTER_MOVE
  if (cursor_blink_enabled) {
   cursor_inverted = 1;
   *vga_bufpos++ = INVCHR(ch);
   BLIT();
   vga_bufpos[-1] = CHR(ch);
  } else
#endif
  {
   *vga_bufpos++ = CHR(ch);
   BLIT();
  }
 } else {
  *vga_bufpos = CHR(ch);
  BLIT_CUR();
#if INVERT_CURSOR_AFTER_MOVE
  if (cursor_blink_enabled) {
   cursor_inverted = 1;
   ++vga_bufpos;
   if (vga_bufpos != vga_bufend) {
    BLIT_CUR_INV();
   }
  } else
#endif
  {
   ++vga_bufpos;
   BLIT_CUR();
  }
 }
}

PRIVATE void TERM_CALL term_putc(struct term *UNUSED(t), char ch) {
 /* v This introducing lag is actually something that's currently intended. */
 //k_syslogf(KLOG_MSG,"%c",ch);
 BEGIN_MOVE_CUR()
 switch (ch) {
 case TERM_CR: CR(); break;
 case TERM_LF:
  //printf(":AFTER LS: %d\n",vga_bufpos == vga_bufend);
  if (vga_bufpos != vga_bufend) LF();
  else term_doscroll();
  break;
 case TERM_BACK:
  BACK();
#if INVERT_CURSOR_AFTER_MOVE
  if (cursor_blink_enabled) {
   cursor_inverted = 1;
   BLIT_CUR_INV();
  }
#endif
  break;
 {
  size_t chrs;
 case TERM_TAB:
  chrs = TERM_TABSIZE-(GET_CUR_X() % TERM_TABSIZE);
  while (chrs--) term_doput(' ');
 } break;
#ifdef TERM_BELL
 case TERM_BELL: // Bell
  /* Wait until a current retrace has ended. */
  while (inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE) task_yield();
  /* Draw the screen inverted. */
  blit_inverted();
  while (!(inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE)) task_yield();
  while (inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE) task_yield();
  usleep(1000);
  while (inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE) task_yield();
  while (!(inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE)) task_yield();
  /* Draw the screen normal again. */
  BLIT();
  /* Wait until the next retrace starts. */
  while (inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE) task_yield();
  while (!(inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE)) task_yield();
  usleep(1000);
#else
 case '\a':
#endif
#if INVERT_CURSOR_AFTER_MOVE
  if (cursor_blink_enabled) {
   cursor_inverted = 1;
   BLIT_CUR_INV();
  }
#endif
  break;
 default: term_doput(ch); break;
 }
 END_MOVE_CUR()
}

PRIVATE uint8_t const vga_box_chars[] = {
 178,0,0,0,0,248,241,0,0,217,191,218,192,197,196,
 196,196,196,196,195,180,193,194,179,243,242,220
};

PRIVATE void TERM_CALL term_putb(struct term *UNUSED(t), char ch) {
 assert(ch >= 'a' && ch <= 'z');
 ch = vga_box_chars[ch-'a'];
 /*if (ch)*/ term_putc(NULL,ch);
}

PRIVATE void TERM_CALL
term_set_color(struct term *UNUSED(t),
               struct term_rgba fg,
               struct term_rgba bg) {
 vga_attrib = (vga_color(fg) | vga_color(bg) << 4) << 8;
}
PRIVATE void TERM_CALL term_set_cursor(struct term *UNUSED(t), coord_t x, coord_t y) {
 if (x >= VTTY_WIDTH)  x = *(__s32 *)&x < 0 ? 0 : VTTY_WIDTH-1;
 if (y >= VTTY_HEIGHT) y = *(__s32 *)&y < 0 ? 0 : VTTY_HEIGHT-1;
 BEGIN_MOVE_CUR()
 SET_CUR(x,y);
#if INVERT_CURSOR_AFTER_MOVE
 if (cursor_blink_enabled) {
  cursor_inverted = 1;
  BLIT_CUR_INV();
 }
#endif
 END_MOVE_CUR()
}
PRIVATE void TERM_CALL term_get_cursor(struct term *UNUSED(t), coord_t *x, coord_t *y) {
 *x = GET_CUR_X();
 *y = GET_CUR_Y();
}
PRIVATE void TERM_CALL term_show_cursor(struct term *UNUSED(t), int cmd) {
 if (cmd == TERM_SHOWCURSOR_YES) {
  cursor_enable();
 } else {
  cursor_disable();
 }
}
PRIVATE void TERM_CALL term_cls(struct term *UNUSED(t), int mode) {
 cell_t *begin,*end,*iter,filler = SPACE;
 switch (mode) {
 case TERM_CLS_BEFORE: begin = vga_buf; end = vga_bufpos; break;
 case TERM_CLS_AFTER : begin = vga_bufpos; end = vga_bufend; break;
 default             : begin = vga_buf; end = vga_bufend; break;
 }
 for (iter = begin; iter != end; ++iter) *iter = filler;
 BLIT_CNT(begin-vga_buf,end-begin);
#if INVERT_CURSOR_AFTER_MOVE
 if (cursor_blink_enabled) {
  cursor_inverted = 1;
  BLIT_CUR_INV();
 }
#endif
}
PRIVATE void TERM_CALL term_el(struct term *UNUSED(t), int mode) {
 cell_t *begin,*end,*iter,filler = SPACE;
 bool refresh_all = vga_bufpos == vga_bufend;
 if (refresh_all) term_doscroll();
 switch (mode) {
 case TERM_EL_BEFORE: begin = CUR_LINE(); end = vga_bufpos; break;
 case TERM_EL_AFTER : begin = vga_bufpos; end = CUR_LINE()+VTTY_WIDTH; break;
 default            : begin = CUR_LINE(); end = begin+VTTY_WIDTH; break;
 }
 for (iter = begin; iter != end; ++iter) *iter = filler;
 if (refresh_all) BLIT();
 else BLIT_CNT(begin-vga_buf,end-begin);
#if INVERT_CURSOR_AFTER_MOVE
 if (cursor_blink_enabled) {
  cursor_inverted = 1;
  BLIT_CUR_INV();
 }
#endif
}
PRIVATE void TERM_CALL term_scroll(struct term *UNUSED(t), offset_t offset) {
 cell_t filler = SPACE;
 cell_t *new_begin,*new_end;
 size_t copycells;
 if (!offset) return;
 if (offset >= VTTY_HEIGHT || offset <= -VTTY_HEIGHT) {
  term_cls(NULL,TERM_CLS_ALL);
  return;
 }
 if (offset > 0) {
  // Shift lines upwards
  copycells = (VTTY_HEIGHT-offset)*VTTY_WIDTH;
  memmove(vga_buf,vga_buf+offset*VTTY_WIDTH,copycells*sizeof(cell_t));
  new_begin = vga_buf+copycells;
  new_end = vga_bufend;
 } else {
  // Shift lines downwards
  copycells = (VTTY_HEIGHT+offset)*VTTY_WIDTH;
  memmove(vga_buf-offset*VTTY_WIDTH,vga_buf,copycells*sizeof(cell_t));
  new_end = (new_begin = vga_buf)+copycells;
 }
 for (; new_begin != new_end; ++new_begin)
  *new_begin = filler;
 BLIT();
}

PRIVATE struct term_operations const term_ops = {
    .to_putc          = &term_putc,
    .to_putb          = &term_putb,
    .to_set_color     = &term_set_color,
    .to_set_cursor    = &term_set_cursor,
    .to_get_cursor    = &term_get_cursor,
    .to_show_cursor   = &term_show_cursor,
    .to_cls           = &term_cls,
    .to_el            = &term_el,
    .to_scroll        = &term_scroll,
    .to_set_title     = NULL,
    .to_putimg        = NULL,
    .to_get_cell_size = NULL,
    .to_output        = NULL,
};

PRIVATE struct winsize const winsize = {
    .ws_row    = VTTY_HEIGHT,
    .ws_col    = VTTY_WIDTH,
    .ws_xpixel = 1,
    .ws_ypixel = 1,
};

PRIVATE ATTR_NORETURN void
usage(char const *name, int exitcode) {
 printf("Usage: %s EXEC-AFTER\n",name);
 exit(exitcode);
}

PRIVATE struct keymap const *current_keymap;
PRIVATE struct term pty;
PRIVATE int amaster,aslave;
PRIVATE keystate_t keystate;

/* Parse and relay keyboard-style inputs from the terminal driver's STDIN */
PRIVATE ATTR_NORETURN void relay_incoming_threadmain(void) {
 char text[8]; char *iter; key_t key; ssize_t s;
 while ((s = read(STDIN_FILENO,&key,sizeof(key_t))) ==
                           (ssize_t)sizeof(key_t)) {
rescan:
  switch (key) {
#define MIRROR(key,state) \
  case KEYDOWN(key): KEYSTATE_ADD(keystate,state); break; \
  case KEYUP  (key): KEYSTATE_DEL(keystate,state); break;
#define TOGGLE(key,state) \
  case KEYDOWN(key): KEYSTATE_XOR(keystate,state); break;

   /* Update the keyboard state. */
  MIRROR(KEY_LSHIFT,KEYSTATE_LSHIFT)
  MIRROR(KEY_RSHIFT,KEYSTATE_RSHIFT)
  MIRROR(KEY_LCTRL,KEYSTATE_LCTRL)
  MIRROR(KEY_RCTRL,KEYSTATE_RCTRL)
  MIRROR(KEY_LGUI,KEYSTATE_LGUI)
  MIRROR(KEY_RGUI,KEYSTATE_RGUI)
  MIRROR(KEY_LALT,KEYSTATE_LALT)
  MIRROR(KEY_RALT,KEYSTATE_RALT)
  MIRROR(KEY_ESC,KEYSTATE_ESC)
  MIRROR(KEY_TAB,KEYSTATE_TAB)
  MIRROR(KEY_SPACE,KEYSTATE_SPACE)
  MIRROR(KEY_APPS,KEYSTATE_APPS)
  MIRROR(KEY_ENTER,KEYSTATE_ENTER)
  MIRROR(KEY_KP_ENTER,KEYSTATE_KP_ENTER)
  MIRROR(KEY_INSERT,KEYSTATE_OVERRIDE)

  TOGGLE(KEY_CAPSLOCK,KEYSTATE_CAPSLOCK)
  TOGGLE(KEY_NUMLOCK,KEYSTATE_NUMLOCK)
  TOGGLE(KEY_SCROLLLOCK,KEYSTATE_SCROLLLOCK)

#undef TOGGLE
#undef MIRROR
  default: break;
  }

  if (!KEY_ISDOWN(key)) continue;
  iter = text;

  if (!KEYSTATE_ISNUMLOCK(keystate)) {
   /* Translate numpad keys to secondary actions unless numlock is on. */
   switch (key) {
   case KEY_KP_0:   key = KEY_INSERT; goto rescan;
   case KEY_KP_1:   key = KEY_END;    goto rescan;
   case KEY_KP_2:   key = KEY_DOWN;   goto rescan;
   case KEY_KP_3:   key = KEY_PGDOWN; goto rescan;
   case KEY_KP_4:   key = KEY_LEFT;   goto rescan;
   case KEY_KP_6:   key = KEY_RIGHT;  goto rescan;
   case KEY_KP_7:   key = KEY_HOME;   goto rescan;
   case KEY_KP_8:   key = KEY_UP;     goto rescan;
   case KEY_KP_9:   key = KEY_PGUP;   goto rescan;
   case KEY_KP_DOT: key = KEY_DELETE; goto rescan;
   default: break;
   }
  }

  if (KEYSTATE_ISALT(keystate) &&
     (!KEYSTATE_ISALTGR(keystate) ||
       key >= 256 ||
      !current_keymap->km_altgr[key]))
     *iter++ = TERM_ESCAPE;

  switch (key) {
  case KEY_F1    : iter = stpcpy(text,TERM_ESCAPE_S "[11~"); break;
  case KEY_F2    : iter = stpcpy(text,TERM_ESCAPE_S "[12~"); break;
  case KEY_F3    : iter = stpcpy(text,TERM_ESCAPE_S "[13~"); break;
  case KEY_F4    : iter = stpcpy(text,TERM_ESCAPE_S "[14~"); break;
  case KEY_F5    : iter = stpcpy(text,TERM_ESCAPE_S "[15~"); break;
  case KEY_F6    : iter = stpcpy(text,TERM_ESCAPE_S "[17~"); break;
  case KEY_F7    : iter = stpcpy(text,TERM_ESCAPE_S "[18~"); break;
  case KEY_F8    : iter = stpcpy(text,TERM_ESCAPE_S "[19~"); break;
  case KEY_F9    : iter = stpcpy(text,TERM_ESCAPE_S "[20~"); break;
  case KEY_F10   : iter = stpcpy(text,TERM_ESCAPE_S "[21~"); break;
  case KEY_F11   : iter = stpcpy(text,TERM_ESCAPE_S "[23~"); break;
  case KEY_F12   : iter = stpcpy(text,TERM_ESCAPE_S "[24~"); break;
  case KEY_UP    : iter = KEYSTATE_ISCTRL(keystate) ? stpcpy(text,TERM_ESCAPE_S "OA") : stpcpy(text,TERM_ESCAPE_S "[A"); break;
  case KEY_DOWN  : iter = KEYSTATE_ISCTRL(keystate) ? stpcpy(text,TERM_ESCAPE_S "OB") : stpcpy(text,TERM_ESCAPE_S "[B"); break;
  case KEY_RIGHT : iter = KEYSTATE_ISCTRL(keystate) ? stpcpy(text,TERM_ESCAPE_S "OC") : stpcpy(text,TERM_ESCAPE_S "[C"); break;
  case KEY_LEFT  : iter = KEYSTATE_ISCTRL(keystate) ? stpcpy(text,TERM_ESCAPE_S "OD") : stpcpy(text,TERM_ESCAPE_S "[D"); break;
  case KEY_PGUP  : iter = stpcpy(text,TERM_ESCAPE_S "[5~"); break;
  case KEY_PGDOWN: iter = stpcpy(text,TERM_ESCAPE_S "[6~"); break;
  case KEY_HOME  : iter = stpcpy(text,TERM_ESCAPE_S "OH"); break;
  case KEY_END   : iter = stpcpy(text,TERM_ESCAPE_S "OF"); break;
  case KEY_DELETE: iter = stpcpy(text,TERM_ESCAPE_S "[3~"); break;
  case KEY_ENTER:  iter = stpcpy(text,"\r"); break; /* Apparently this is a thing... */

  case KEY_LALT:
  case KEY_RALT:
   goto send_nothing;

  case KEY_TAB:
   if (KEYSTATE_ISCAPS(keystate)) {
    *iter++ = TERM_ESCAPE;
    *iter++ = '[';
    *iter++ = 'Z';
    break;
   }
  default:
   if (key < 256) {
    char ch;
    if (KEYSTATE_ISALTGR(keystate)) {
     ch = current_keymap->km_altgr[key];
     if (!ch) goto check_other;
    } else check_other: if (KEYSTATE_ISCAPS(keystate))
     ch = current_keymap->km_shift[key];
    else {
     ch = current_keymap->km_press[key];
    }
    if (ch) {
     if (KEYSTATE_ISCTRL(keystate) &&
        (!KEYSTATE_ISALTGR(keystate) ||
         !current_keymap->km_altgr[key])) {
      /* Create control character (e.g.: CTRL+C --> '\03') */
      ch = toupper(ch)-'@';
     }
     *iter++ = ch;
    }
   }
   break;
  }
  if (iter != text) {
#if 0
   printf("[TERMINAL-VGA] Relay: %Iu %.?q\n",
           iter-text,(size_t)(iter-text),text);
#endif
   if (write(amaster,text,(size_t)(iter-text)) == -1) {
    perror("Relay failed");
    break;
   }
  }
send_nothing:;
 }
 err(EXIT_FAILURE,"Unexpected input relay failure");
}


int main(int argc, char *argv[]) {
 int result;
 pid_t child_proc;
 /* TODO: Re-write to support multiple pages allocated in a ring-buffer
  *       were appending a new line only requires shifting of some
  *       base-pointer, as well as duplication into VGA memory. */

 if (argc < 2) { usage(argv[0],EXIT_FAILURE); }
 current_keymap = GET_ACTIVE_KEYMAP();
 if (!current_keymap) err(EXIT_FAILURE,"Failed to load keyboard map");


 /* Map the x86 VGA terminal.
  * NOTE: We map it as loose, so that the device memory won't be
  *       mapped in the child process we're forking into below. */
 vga_fd = open(TTY_DEVNAME,O_RDWR);
 if (vga_fd < 0) err(EXIT_FAILURE,"Failed to open %q",TTY_DEVNAME);
 fcntl(vga_fd,F_SETFL,FD_CLOEXEC|FD_CLOFORK);

 vga_dev = (cell_t *)mmap(0,PAGESIZE,PROT_READ|PROT_WRITE,
                          MAP_SHARED,vga_fd,0);
 if (vga_dev == (cell_t *)MAP_FAILED)
     err(EXIT_FAILURE,"Failed to mmap() VGA terminal");
 /*printf("Mapped terminal driver to %p\n",vga_dev);*/
 vga_buf = (cell_t *)malloc((VGA_SIZE+VTTY_WIDTH)*sizeof(cell_t));
 if (!vga_buf) err(EXIT_FAILURE,"Failed to allocate VGA buffer");
 vga_bufend        = vga_buf+VGA_SIZE;
 vga_bufpos        = vga_buf;
 vga_bufsecondline = vga_buf+VTTY_WIDTH;
 vga_lastline      = vga_buf+(VGA_SIZE-VTTY_WIDTH);
 { cell_t *end = vga_bufend+VTTY_WIDTH;
   cell_t *iter = vga_buf,filler = SPACE;
   for (; iter != end; ++iter) *iter = filler;
 }
 if (!term_init(&pty,&term_ops,NULL,NULL))
      err(EXIT_FAILURE,"Failed to create terminal host");

 /* The terminal driver runtime is loaded and operational! */

 /* Create the PTY device. */
 if (openpty(&amaster,&aslave,NULL,NULL,&winsize) == -1)
     err(EXIT_FAILURE,"Failed to create PTY device");
 fcntl(amaster,F_SETFL,FD_CLOEXEC);

#if 1
 if ((child_proc = fork()) == 0) {
  /* === Child process */
  /* Redirect all standard files to the terminal */
  dup2(aslave,STDIN_FILENO);
  dup2(aslave,STDOUT_FILENO);
  dup2(aslave,STDERR_FILENO);
  /* Close all other descriptors */
  if (aslave >= 3) close(aslave);

  /* Set the child process as TTY foreground app. */
  tcsetpgrp(STDIN_FILENO,getpid());

  /* Exec the given process */
  execv(argv[1],argv+1);
  perror("Failed to exec given process");
  exit(errno);
 } else if (child_proc == -1)
#endif
 {
  perror("Failed to fork child process");
  exit(EXIT_FAILURE);
 }

 /* Close the slave end of the terminal driver */
 close(aslave);

 /* Spawn helper threads */
#if HAVE_CURSOR_BLINK_THREAD
 if ((cursor_blink_thread = fork()) == 0)
      cursor_blink_threadmain();
#endif
 if ((relay_incoming_thread = fork()) == 0) {
  relay_incoming_threadmain();
 } else if (relay_incoming_thread < 0) {
  err(EXIT_FAILURE,"Failed to start relay-incoming process");
 }

 /*k_syslogf(KLOG_DEBUG,"Updating screen for the first time\n");*/
 BLIT();

 for (;;) {
  ssize_t s; char buf[128];
  /* Relay everything being printed to the terminal towards the screen. */
  while ((s = read(amaster,buf,sizeof(buf))) > 0) {
#if 0
   format_printf(&term_printer,&pty,"%$q\n",s,buf);
   syslog(LOG_DEBUG,"TERM: %$q\n",s,buf);
#else
   term_printer(buf,(size_t)s,&pty);
#endif
  }
  if (s < 0) {
   if (errno == EINTR) {
    /* Check if we were interrupted because the child died. */
    siginfo_t info;
    if (waitid(P_PID,child_proc,&info,WNOHANG|WEXITED) >= 0) {
     result = WEXITSTATUS(info.si_status);
     goto child_joined;
    }
    continue;
   }
   result = errno;
   error(0,result,"Unexpected slave output read failure");
  }
  break;
 }
 /* The pipe was broken. - Wait for the child to terminate. */
 syslog(LOG_DEBUG,"TERMINAL-VGA: Waiting for child process %d\n",child_proc);
 waitpid(child_proc,&result,WEXITED);
 result = WEXITSTATUS(result);
child_joined:

#if HAVE_CURSOR_BLINK_THREAD
 if (kill(cursor_blink_thread,SIGKILL))
     perror("Failed to kill <cursor_blink_thread>");
#endif
 if (kill(relay_incoming_thread,SIGKILL))
     perror("Failed to kill <relay_incoming_thread>");

#if 1
 term_fini(&pty);
 free(vga_buf);
 munmap(vga_dev,VGA_SIZE*sizeof(cell_t));
 close(vga_fd);
#endif

 return result;
}

DECL_END

#endif /* !GUARD_APPS_TERMINAL_VGA_MAIN_C */
