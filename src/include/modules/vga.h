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
#ifndef GUARD_INCLUDE_MODULES_VGA_H
#define GUARD_INCLUDE_MODULES_VGA_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/byteorder.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/io.h>

DECL_BEGIN

/* Taken from linux, before changes were made. */

/*
 * linux/include/video/vga.h -- standard VGA chipset interaction
 *
 * Copyright 1999 Jeff Garzik <jgarzik@pobox.com>
 * 
 * Copyright history from vga16fb.c:
 *	Copyright 1999 Ben Pfaff and Petr Vandrovec
 *	Based on VGA info at http://www.osdever.net/FreeVGA/home.htm 
 *	Based on VESA framebuffer (c) 1998 Gerd Knorr
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.  
 *
 */

/* Some of the code below is taken from SVGAlib.  The original,
   unmodified copyright notice for that code is below. */
/* VGAlib version 1.2 - (c) 1993 Tommy Frandsen                    */
/*                                                                 */
/* This library is free software; you can redistribute it and/or   */
/* modify it without any restrictions. This library is distributed */
/* in the hope that it will be useful, but without any warranty.   */

/* Multi-chipset support Copyright 1993 Harm Hanemaayer */
/* partially copyrighted (C) 1993 by Hartmut Schirmer */

/* VGA data register ports */
#define VGA_CRT_DC      0x3d5 /*< CRT Controller Data Register - color emulation. */
#define VGA_CRT_DM      0x3b5 /*< CRT Controller Data Register - mono emulation. */
#define VGA_ATT_R       0x3c1 /*< Attribute Controller Data Read Register. */
#define VGA_ATT_W       0x3c0 /*< Attribute Controller Data Write Register. */
#define VGA_GFX_D       0x3cf /*< Graphics Controller Data Register. */
#define VGA_SEQ_D       0x3c5 /*< Sequencer Data Register. */
#define VGA_MIS_R       0x3cc /*< Misc Output Read Register. */
#define VGA_MIS_W       0x3c2 /*< Misc Output Write Register. */
#   define VGA_MIS_COLOR            0x01
#   define VGA_MIS_ENB_MEM_ACCESS   0x02
#   define VGA_MIS_DCLK_28322_720   0x04
#   define VGA_MIS_ENB_PLL_LOAD    (0x04|0x08)
#   define VGA_MIS_SEL_HIGH_PAGE    0x20
#define VGA_FTC_R       0x3ca /*< Feature Control Read Register. */
#define VGA_IS1_RC      0x3da /*< Input Status Register 1 - color emulation. */
#define VGA_IS1_RM      0x3ba /*< Input Status Register 1 - mono emulation. */
#define VGA_PEL_D       0x3c9 /*< PEL Data Register. */
#define VGA_PEL_MSK     0x3c6 /*< PEL mask register. */

/* EGA-specific registers */
#define EGA_GFX_E0      0x3cc /*< Graphics enable processor 0. */
#define EGA_GFX_E1      0x3ca /*< Graphics enable processor 1. */

/* VGA index register ports */
#define VGA_CRT_IC      0x3d4 /*< CRT Controller Index - color emulation. */
#define VGA_CRT_IM      0x3b4 /*< CRT Controller Index - mono emulation. */
#define VGA_ATT_IW      0x3c0 /*< Attribute Controller Index & Data Write Register. */
#define VGA_GFX_I       0x3ce /*< Graphics Controller Index. */
#define VGA_SEQ_I       0x3c4 /*< Sequencer Index. */
#define VGA_PEL_IW      0x3c8 /*< PEL Write Index. */
#define VGA_PEL_IR      0x3c7 /*< PEL Read Index. */

/* standard VGA indexes max counts */
#define VGA_CRT_C       0x19  /*< Number of CRT Controller Registers. */
#define VGA_ATT_C       0x15  /*< Number of Attribute Controller Registers. */
#define VGA_GFX_C       0x09  /*< Number of Graphics Controller Registers. */
#define VGA_SEQ_C       0x05  /*< Number of Sequencer Registers. */
#define VGA_MIS_C       0x01  /*< Number of Misc Output Register. */

/* VGA CRT controller register indices */
#define VGA_CRTC_H_TOTAL       0
#define VGA_CRTC_H_DISP        1
#define VGA_CRTC_H_BLANK_START 2
#define VGA_CRTC_H_BLANK_END   3
#   define VGA_CR3_MASK            0x0f /* Mask of bits used for hblank-end (Added by GrieferAtWork) */
#   define VGA_CR3_ALWAYS1         0x80 /* Always set this bit when writing this register (backward compatibility) */
#define VGA_CRTC_H_SYNC_START  4
#define VGA_CRTC_H_SYNC_END    5
#   define VGA_CR5_MASK            0x0f /* Mask of bits used for hsync-end (Added by GrieferAtWork) */
#   define VGA_CR5_H_BLANK_END_5   0x80 /* 5th bit for `VGA_CRTC_H_BLANK_END' (Added by GrieferAtWork) */
#define VGA_CRTC_V_TOTAL       6
#define VGA_CRTC_OVERFLOW      7
#   define VGA_CR7_V_TOTAL_8       0x01 /* 8th bit for `VGA_CRTC_V_TOTAL' (Added by GrieferAtWork) */
#   define VGA_CR7_V_DISP_END_8    0x02 /* 8th bit for `VGA_CRTC_V_DISP_END' (Added by GrieferAtWork) */
#   define VGA_CR7_V_SYNC_START_8  0x04 /* 8th bit for `VGA_CRTC_V_SYNC_START' (Added by GrieferAtWork) */
#   define VGA_CR7_V_BLANK_START_8 0x08 /* 8th bit for `VGA_CRTC_V_BLANK_START' (Added by GrieferAtWork) */
#   define VGA_CR7_V_TOTAL_9       0x20 /* 9th bit for `VGA_CRTC_V_TOTAL' (Added by GrieferAtWork) */
#   define VGA_CR7_V_DISP_END_9    0x40 /* 9th bit for `VGA_CRTC_V_DISP_END' (Added by GrieferAtWork) */
#   define VGA_CR7_V_SYNC_START_9  0x80 /* 9th bit for `VGA_CRTC_V_SYNC_START' (Added by GrieferAtWork) */
#define VGA_CRTC_PRESET_ROW    8
#define VGA_CRTC_MAX_SCAN      9
#   define VGA_CR9_V_BLANK_START_9 0x10 /* 9th bit for `VGA_CRTC_V_BLANK_START' (Added by GrieferAtWork) */
#   define VGA_CR9_SCANDOUBLE      0x80 /* Better don't set... (Don't really understand what this done) (Added by GrieferAtWork) */
#define VGA_CRTC_CURSOR_START  0x0a
#   define VGA_CRTC_CURSOR_DISABLE 0x20 /* Disable the text-mode cursor (Added by GrieferAtWork) */
#define VGA_CRTC_CURSOR_END    0x0b
#define VGA_CRTC_START_HI      0x0c
#define VGA_CRTC_START_LO      0x0d
#define VGA_CRTC_CURSOR_HI     0x0e
#define VGA_CRTC_CURSOR_LO     0x0f
#define VGA_CRTC_V_SYNC_START  0x10
#define VGA_CRTC_V_SYNC_END    0x11
#   define VGA_CR11_MASK         0x0f /* Mask of bits used for vsync-end (Added by GrieferAtWork) */
#   define VGA_CR11_LOCK_CR0_CR7 0x80 /* lock writes to CR0 - CR7. */
#define VGA_CRTC_V_DISP_END    0x12
#define VGA_CRTC_OFFSET        0x13
#define VGA_CRTC_UNDERLINE     0x14
#define VGA_CRTC_V_BLANK_START 0x15
#define VGA_CRTC_V_BLANK_END   0x16
#define VGA_CRTC_MODE          0x17
#   define VGA_CR17_H_V_SIGNALS_ENABLED 0x80
#define VGA_CRTC_LINE_COMPARE  0x18
#define VGA_CRTC_REGS          VGA_CRT_C

/* VGA attribute controller register indices */
#define VGA_ATC_PALETTE0       0x00
#define VGA_ATC_PALETTE1       0x01
#define VGA_ATC_PALETTE2       0x02
#define VGA_ATC_PALETTE3       0x03
#define VGA_ATC_PALETTE4       0x04
#define VGA_ATC_PALETTE5       0x05
#define VGA_ATC_PALETTE6       0x06
#define VGA_ATC_PALETTE7       0x07
#define VGA_ATC_PALETTE8       0x08
#define VGA_ATC_PALETTE9       0x09
#define VGA_ATC_PALETTEA       0x0a
#define VGA_ATC_PALETTEB       0x0b
#define VGA_ATC_PALETTEC       0x0c
#define VGA_ATC_PALETTED       0x0d
#define VGA_ATC_PALETTEE       0x0e
#define VGA_ATC_PALETTEF       0x0f
#define VGA_ATC_MODE           0x10
#   define VGA_AT10_GRAPHICS      0x01 /*< Enable graphics, rather than alphanumeric mode (Added by GrieferAtWork). */
#   define VGA_AT10_DUP9          0x04 /*< Duplicate the 8'th text dot into the 9'th when 'VGA_SR01_CHAR_CLK_8DOTS' isn't set, instead of filling it with background (Added by GrieferAtWork). */
#   define VGA_AT10_BLINK         0x08 /*< Set to cause character attribute bit #7 to be used for blinking text (Added by GrieferAtWork). */
#   define VGA_AT10_8BITPAL       0x40 /*< 8-bit palette index (Added by GrieferAtWork). */
#define VGA_ATC_OVERSCAN       0x11
#define VGA_ATC_PLANE_ENABLE   0x12
#define VGA_ATC_PEL            0x13
#define VGA_ATC_COLOR_PAGE     0x14

#define VGA_AR_ENABLE_DISPLAY  0x20

/* VGA sequencer register indices */
#define VGA_SEQ_RESET          0x00
#define VGA_SEQ_CLOCK_MODE     0x01
#   define VGA_SR01_CHAR_CLK_8DOTS 0x01 /* bit 0: character clocks 8 dots wide are generated */
#   define VGA_SR01_SCREEN_OFF     0x20 /* bit 5: Screen is off */
#define VGA_SEQ_PLANE_WRITE    0x02
#   define VGA_SR02_PLANE(i)     ((1) << i) /* Added by GrieferAtWork */
#   define VGA_SR02_ALL_PLANES     0x0f /* bits 3-0: enable access to all planes */
#define VGA_SEQ_CHARACTER_MAP  0x03
#define VGA_SEQ_MEMORY_MODE    0x04
#   define VGA_SR04_EXT_MEM        0x02 /* bit 1: allows complete mem access to 256K */
#   define VGA_SR04_SEQ_MODE       0x04 /* bit 2: directs system to use a sequential addressing mode */
#   define VGA_SR04_CHN_4M         0x08 /* bit 3: selects modulo 4 addressing for CPU access to display memory */

/* VGA graphics controller register indices */
#define VGA_GFX_SR_VALUE        0x00
#define VGA_GFX_SR_ENABLE       0x01
#define VGA_GFX_COMPARE_VALUE   0x02
#define VGA_GFX_DATA_ROTATE     0x03
#define VGA_GFX_PLANE_READ      0x04
#define VGA_GFX_MODE            0x05
#define VGA_GFX_MISC            0x06
#   define VGA_GR06_GRAPHICS_MODE  0x01
#define VGA_GFX_COMPARE_MASK    0x07
#define VGA_GFX_BIT_MASK        0x08

/* macro for composing an 8-bit VGA register
 * index and value into a single 16-bit quantity */
#define VGA_OUT16VAL(v,r)    (((v) << 8)|(r))

/* decide whether we should enable the faster 16-bit VGA register writes */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define VGA_OUTW_WRITE 1
#endif


/* generic VGA port read/write */
LOCAL u8   KCALL vga_io_r(u16 port) { return inb_p(port); }
LOCAL void KCALL vga_io_w(u16 port, u8 val) { outb_p(port,val); }
LOCAL void KCALL vga_io_w_fast(u16 port, u8 reg, u8 val) { outw(port,VGA_OUT16VAL(val,reg)); }
LOCAL u8   KCALL vga_mm_r(MMIO void *regbase, u16 port) { return readb((MMIO byte_t *)regbase+port); }
LOCAL void KCALL vga_mm_w(MMIO void *regbase, u16 port, u8 val) { writeb((MMIO byte_t *)regbase+port,val); }
LOCAL void KCALL vga_mm_w_fast(MMIO void *regbase, u16 port, u8 reg, u8 val) { writew((MMIO byte_t *)regbase+port,VGA_OUT16VAL(val,reg)); }
LOCAL u8   KCALL vga_r(MMIO void *regbase, u16 port) { return regbase ? vga_mm_r(regbase,port) : vga_io_r(port); }
LOCAL void KCALL vga_w(MMIO void *regbase, u16 port, u8 val) { regbase ? vga_mm_w(regbase,port,val) : vga_io_w(port,val); }
LOCAL void KCALL vga_w_fast(MMIO void *regbase, u16 port, u8 reg, u8 val) { regbase ? vga_mm_w_fast(regbase,port,reg,val) : vga_io_w_fast(port,reg,val); }

#ifdef VGA_OUTW_WRITE
#define __VGA_OUTW_SELECTOR(func,port_i,port_d,reg,val)          ((port_i+1 == port_d) ? func##_fast(port_i,reg,val)         : (func(port_i,reg),func(port_d,val)))
#define __VGA_OUTW_SELECTOR2(func,regbase,port_i,port_d,reg,val) ((port_i+1 == port_d) ? func##_fast(regbase,port_i,reg,val) : (func(regbase,port_i,reg),func(regbase,port_d,val)))
#else
#define __VGA_OUTW_SELECTOR(func,port_i,port_d,reg,val)          (func(port_i,reg),func(port_d,val))
#define __VGA_OUTW_SELECTOR2(func,regbase,port_i,port_d,reg,val) (func(regbase,port_i,reg),func(regbase,port_d,val))
#endif

/* VGA CRTC register read/write */
LOCAL u8   KCALL vga_rcrt(MMIO void *regbase, u8 reg) { vga_w(regbase,VGA_CRT_IC,reg); return vga_r(regbase,VGA_CRT_DC); }
LOCAL void KCALL vga_wcrt(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_w,regbase,VGA_CRT_IC,VGA_CRT_DC,reg,val); }
LOCAL u8   KCALL vga_io_rcrt(u8 reg) { vga_io_w(VGA_CRT_IC,reg); return vga_io_r(VGA_CRT_DC); }
LOCAL void KCALL vga_io_wcrt(u8 reg, u8 val) { __VGA_OUTW_SELECTOR(vga_io_w,VGA_CRT_IC,VGA_CRT_DC,reg,val); }
LOCAL u8   KCALL vga_mm_rcrt(MMIO void *regbase, u8 reg) { vga_mm_w(regbase,VGA_CRT_IC,reg); return vga_mm_r(regbase,VGA_CRT_DC); }
LOCAL void KCALL vga_mm_wcrt(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_mm_w,regbase,VGA_CRT_IC,VGA_CRT_DC,reg,val); }


/* VGA sequencer register read/write */
LOCAL u8   KCALL vga_rseq(MMIO void *regbase, u8 reg) { vga_w(regbase,VGA_SEQ_I,reg); return vga_r(regbase,VGA_SEQ_D); }
LOCAL void KCALL vga_wseq(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_w,regbase,VGA_SEQ_I,VGA_SEQ_D,reg,val); }
LOCAL u8   KCALL vga_io_rseq(u8 reg) { vga_io_w(VGA_SEQ_I,reg); return vga_io_r(VGA_SEQ_D); }
LOCAL void KCALL vga_io_wseq(u8 reg, u8 val) { __VGA_OUTW_SELECTOR(vga_io_w,VGA_SEQ_I,VGA_SEQ_D,reg,val); }
LOCAL u8   KCALL vga_mm_rseq(MMIO void *regbase, u8 reg) { vga_mm_w(regbase,VGA_SEQ_I,reg); return vga_mm_r(regbase,VGA_SEQ_D); }
LOCAL void KCALL vga_mm_wseq(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_mm_w,regbase,VGA_SEQ_I,VGA_SEQ_D,reg,val); }

/* VGA graphics controller register read/write */
LOCAL u8   KCALL vga_rgfx(MMIO void *regbase, u8 reg) { vga_w(regbase,VGA_GFX_I,reg); return vga_r(regbase,VGA_GFX_D); }
LOCAL void KCALL vga_wgfx(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_w,regbase,VGA_GFX_I,VGA_GFX_D,reg,val); }
LOCAL u8   KCALL vga_io_rgfx(u8 reg) { vga_io_w(VGA_GFX_I,reg); return vga_io_r(VGA_GFX_D); }
LOCAL void KCALL vga_io_wgfx(u8 reg, u8 val) { __VGA_OUTW_SELECTOR(vga_io_w,VGA_GFX_I,VGA_GFX_D,reg,val); }
LOCAL u8   KCALL vga_mm_rgfx(MMIO void *regbase, u8 reg) { vga_mm_w(regbase,VGA_GFX_I,reg); return vga_mm_r(regbase,VGA_GFX_D); }
LOCAL void KCALL vga_mm_wgfx(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_mm_w,regbase,VGA_GFX_I,VGA_GFX_D,reg,val); }

/* VGA attribute controller register read/write */
LOCAL u8   KCALL vga_rattr(MMIO void *regbase, u8 reg) { vga_w(regbase,VGA_ATT_IW,reg); return vga_r(regbase,VGA_ATT_R); }
LOCAL void KCALL vga_wattr(MMIO void *regbase, u8 reg, u8 val) { vga_w(regbase,VGA_ATT_IW,reg); vga_w(regbase,VGA_ATT_W,val); }
LOCAL u8   KCALL vga_io_rattr(u8 reg) { vga_io_w(VGA_ATT_IW,reg); return vga_io_r(VGA_ATT_R); }
LOCAL void KCALL vga_io_wattr(u8 reg, u8 val) { vga_io_w(VGA_ATT_IW,reg); vga_io_w(VGA_ATT_W,val); }
LOCAL u8   KCALL vga_mm_rattr(MMIO void *regbase, u8 reg) { vga_mm_w(regbase,VGA_ATT_IW,reg); return vga_mm_r(regbase,VGA_ATT_R); }
LOCAL void KCALL vga_mm_wattr(MMIO void *regbase, u8 reg, u8 val) { vga_mm_w(regbase,VGA_ATT_IW,reg); vga_mm_w(regbase,VGA_ATT_W,val); }


/* VGA State Save and Restore */
#define VGA_SAVE_PLANE(i) (1 << (i))
#define VGA_SAVE_PLANE0    0x01 /*< Save/restore plane 0. */
#define VGA_SAVE_PLANE1    0x02 /*< Save/restore plane 1. */
#define VGA_SAVE_PLANE2    0x04 /*< Save/restore plane 2. */
#define VGA_SAVE_PLANE3    0x08 /*< Save/restore plane 3. */
#define VGA_SAVE_FONT0     VGA_SAVE_PLANE2
#define VGA_SAVE_FONT1     VGA_SAVE_PLANE3
#define VGA_SAVE_TEXT     (VGA_SAVE_PLANE0|VGA_SAVE_PLANE1)
#define VGA_SAVE_FONTS     0x0f /*< Save/restore all fonts. */
#define VGA_SAVE_MODE      0x10 /*< Save/restore video mode. */
#define VGA_SAVE_CMAP      0x20 /*< Save/restore color map/DAC. */

struct vgastate {
 MMIO void  *vs_mmio;    /*< [0..1] MMIO Base address, or NULL if not available. */
 u8          vs_saved;   /*< Set of 'VGA_SAVE_*' describing what has been saved. */
 u8          vs_crt[VGA_CRT_C]; /*< [valid_if(VGA_SAVE_MODE)] Saved CRT Controller registers. */
 u8          vs_att[VGA_ATT_C]; /*< [valid_if(VGA_SAVE_MODE)] Saved Attribute Controller registers. */
 u8          vs_gfx[VGA_GFX_C]; /*< [valid_if(VGA_SAVE_MODE)] Saved Graphics Controller registers. */
 u8          vs_seq[VGA_SEQ_C]; /*< [valid_if(VGA_SAVE_MODE)] Saved Sequencer registers. */
 u8          vs_mis[VGA_MIS_C]; /*< [valid_if(VGA_SAVE_MODE)] Saved Misc Output register. */
 KPD byte_t *vs_data[5]; /*< [0..1][*][owned] Misc. data, the contents of which depend on 'vs_saved' */
};

/* Initialize 'state' with the current VGA state, saving information listed by 'save' mask.
 * @param: regbase: [0..1] The MMIO VGA base address, or NULL if not available.
 * @param: save:     A set of 'VGA_SAVE_*' describing what should be saved.
 * @return: -EOK:    Successfully saved the current VGA state.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF errno_t KCALL save_vga(struct vgastate *__restrict state, MMIO void *regbase, u8 save);
/* Replace the current VGA state with the one described by 'state'. */
FUNDEF void KCALL load_vga(struct vgastate *__restrict state, bool call_free_vga);
/* Free a VGA state created by 'save_vga()'. */
FUNDEF void KCALL free_vga(struct vgastate *__restrict state);


struct vmode {
 u16          v_resx;     /*< Horizontal resolution (In pixels). */
 u16          v_resy;     /*< Vertical resolution (In pixels). */
 u32          v_pitch;    /*< Size of a single line (In bytes). */
 unsigned int v_bpp : 3;  /*< Bits per pixel. */
 unsigned int v_pad : 13; /* ... */
 u16          v_hz;       /*< Max refresh speed (A lower value may be set, though). */
};

struct vga_vmode {
 u8 vv_htotal;       /* VGA_CRTC_H_TOTAL */
 u8 vv_hdisp;        /* VGA_CRTC_H_DISP */
 u8 vv_hblank_start; /* VGA_CRTC_H_BLANK_START */
 u8 vv_hblank_end;   /* [MASK(VGA_CR3_MASK)] VGA_CRTC_H_BLANK_END */
 u8 vv_hsync_start;  /* VGA_CRTC_H_SYNC_START */
 u8 vv_hsync_end;    /* [MASK(VGA_CR5_H_BLANK_END_5|VGA_CR5_MASK)] VGA_CRTC_H_SYNC_END */
 u8 vv_vtotal;       /* VGA_CRTC_V_TOTAL */
 u8 vv_overflow;     /* VGA_CRTC_OVERFLOW */
 u8 vv_max_scan;     /* [MASK(VGA_CR9_V_BLANK_START_9)] VGA_CRTC_MAX_SCAN */
 u8 vv_vsync_start;  /* VGA_CRTC_V_SYNC_START */
 u8 vv_vsync_end;    /* [MASK(VGA_CR11_MASK)] VGA_CRTC_V_SYNC_END */
 u8 vv_vdisp_end;    /* VGA_CRTC_V_DISP_END */
 u8 vv_vblank_start; /* VGA_CRTC_V_BLANK_START */
 u8 vv_vblank_end;   /* VGA_CRTC_V_BLANK_END */
 u8 vv_misc;         /* [MASK(VGA_MIS_ENB_PLL_LOAD)] VGA_MIS_R / VGA_MIS_W */
 u8 vv_clockmode;    /* [MASK(VGA_SR01_CHAR_CLK_8DOTS)] VGA_SEQ_CLOCK_MODE */
};


/* Set the current video mode. */
FUNDEF void KCALL vga_set_vmode(MMIO void *regbase, struct vmode *__restrict mode);
FUNDEF void KCALL vga_set_vvmode(MMIO void *regbase, struct vga_vmode const *__restrict mode);
FUNDEF void KCALL vga_v2vvmode(struct vmode *__restrict mode, struct vga_vmode *__restrict result);


DECL_END

#endif /* !GUARD_INCLUDE_MODULES_VGA_H */
