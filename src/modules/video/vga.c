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
#ifndef GUARD_MODULES_VIDEO_VGA_C
#define GUARD_MODULES_VIDEO_VGA_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/check.h>
#include <kernel/export.h>
#include <kernel/mman.h>
#include <modules/vga.h>
#include <kernel/malloc.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <sched/paging.h>
#include <sched/cpu.h>

DECL_BEGIN

/* Base address and size of video memory. */
PRIVATE PHYS byte_t *vram_addr = (byte_t *)0xA0000;
PRIVATE      size_t  vram_size = 8192*4; /* 32K */

#define VGA_CMAP_SIZE 768
#define DATA_PLANE(i) i
#define DATA_PLANE0   0
#define DATA_PLANE1   1
#define DATA_PLANE2   2
#define DATA_PLANE3   3
#define DATA_CMAP     4


/* Save/load the VGA color map. */
PRIVATE void KCALL
save_cmap(MMIO void *regbase, byte_t *__restrict buffer) {
 unsigned int i;
 vga_w(regbase,VGA_PEL_MSK,0xff);
 vga_w(regbase,VGA_PEL_IR,0x00);
 for (i = 0; i < VGA_CMAP_SIZE; ++i)
      buffer[i] = vga_r(regbase,VGA_PEL_D);
}
PRIVATE void KCALL
load_cmap(MMIO void *regbase, byte_t const *__restrict buffer) {
 unsigned int i;
 vga_w(regbase,VGA_PEL_MSK,0xff);
 vga_w(regbase,VGA_PEL_IW,0x00);
 for (i = 0; i < VGA_CMAP_SIZE; ++i)
      vga_w(regbase,VGA_PEL_D,buffer[i]);
}



PUBLIC errno_t KCALL
save_vga(struct vgastate *__restrict state,
         MMIO void *regbase, u8 save) {
 struct mman *omm;
 unsigned int i; errno_t error = -EOK;
 TASK_PDIR_KERNEL_BEGIN(omm);

 state->vs_mmio  = regbase;
 state->vs_saved = save;
 memset(state->vs_data,0,sizeof(state->vs_data));

 /* Allocate necessary data. */
 if (save&VGA_SAVE_PLANE0 && (state->vs_data[DATA_PLANE0] = (byte_t *)kmalloc(vram_size,GFP_KERNEL)) == NULL) goto err_nomem;
 if (save&VGA_SAVE_PLANE1 && (state->vs_data[DATA_PLANE1] = (byte_t *)kmalloc(vram_size,GFP_KERNEL)) == NULL) goto err_nomem;
 if (save&VGA_SAVE_PLANE2 && (state->vs_data[DATA_PLANE2] = (byte_t *)kmalloc(vram_size,GFP_KERNEL)) == NULL) goto err_nomem;
 if (save&VGA_SAVE_PLANE3 && (state->vs_data[DATA_PLANE3] = (byte_t *)kmalloc(vram_size,GFP_KERNEL)) == NULL) goto err_nomem;
 if (save&VGA_SAVE_CMAP     && (state->vs_data[DATA_CMAP] = (byte_t *)kmalloc(VGA_CMAP_SIZE,GFP_KERNEL)) == NULL) goto err_nomem;

 if (save&VGA_SAVE_CMAP)
     save_cmap(regbase,state->vs_data[DATA_CMAP]);
 if (save&VGA_SAVE_MODE) {
  u16 iobase;
  state->vs_mis[0] = vga_r(regbase,VGA_MIS_R);
  iobase = state->vs_mis[0] & VGA_MIS_COLOR ? VGA_CRT_IC : VGA_CRT_IM;
  for (i = 0; i < VGA_CRT_C; ++i) {
   vga_w(regbase,iobase,i);
   state->vs_crt[i] = vga_r(regbase,iobase+1);
  }
  vga_r(regbase,iobase+(VGA_IS1_RC-VGA_CRT_IC));
  vga_w(regbase,VGA_ATT_W,0x00);
  for (i = 0; i < VGA_ATT_C; ++i) {
   vga_r(regbase,iobase+(VGA_IS1_RC-VGA_CRT_IC));
   state->vs_att[i] = vga_rattr(regbase,i);
  }
  vga_r(regbase,iobase+(VGA_IS1_RC-VGA_CRT_IC));
  vga_w(regbase,VGA_ATT_W,0x20);
  for (i = 0; i < VGA_GFX_C; ++i)
       state->vs_gfx[i] = vga_rgfx(regbase,i);
  for (i = 0; i < VGA_SEQ_C; ++i)
       state->vs_seq[i] = vga_rseq(regbase,i);
 }
 if (save&VGA_SAVE_FONTS) {
  u8 gr4,gr5,gr6,qr1,sr2,sr4;
  if (save&VGA_SAVE_MODE) {
   gr4 = state->vs_gfx[VGA_GFX_PLANE_READ];
   gr5 = state->vs_gfx[VGA_GFX_MODE];
   gr6 = state->vs_gfx[VGA_GFX_MISC];
   sr2 = state->vs_seq[VGA_SEQ_PLANE_WRITE];
   sr4 = state->vs_seq[VGA_SEQ_MEMORY_MODE];
   qr1 = state->vs_seq[VGA_SEQ_CLOCK_MODE];
  } else {
   gr4 = vga_rgfx(regbase,VGA_GFX_PLANE_READ);
   gr5 = vga_rgfx(regbase,VGA_GFX_MODE);
   gr6 = vga_rgfx(regbase,VGA_GFX_MISC);
   sr2 = vga_rseq(regbase,VGA_SEQ_PLANE_WRITE);
   sr4 = vga_rseq(regbase,VGA_SEQ_MEMORY_MODE);
   qr1 = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);
  }

  /* Turn off the screen to hide what we're doing. */
  vga_wseq(regbase,VGA_SEQ_RESET,0x1);
  vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
  vga_wseq(regbase,VGA_SEQ_RESET,0x3);

  for (i = 0; i < 4; ++i) {
   if (!(save&VGA_SAVE_PLANE(i))) continue;
   vga_wseq(regbase,VGA_SEQ_PLANE_WRITE,VGA_SR02_PLANE(i));
   vga_wseq(regbase,VGA_SEQ_MEMORY_MODE,VGA_SR04_EXT_MEM|VGA_SR04_SEQ_MODE);
   vga_wgfx(regbase,VGA_GFX_PLANE_READ,i);
   vga_wgfx(regbase,VGA_GFX_MODE,0x0);
   vga_wgfx(regbase,VGA_GFX_MISC,0x5);
   memcpy(state->vs_data[DATA_PLANE(i)],vram_addr,vram_size);
  }

  /* With all planes restored, restore registers and turn the screen back on. */
  vga_wseq(regbase,VGA_SEQ_PLANE_WRITE,sr2);
  vga_wseq(regbase,VGA_SEQ_MEMORY_MODE,sr4);
  vga_wgfx(regbase,VGA_GFX_PLANE_READ,gr4);
  vga_wgfx(regbase,VGA_GFX_MODE,gr5);
  vga_wgfx(regbase,VGA_GFX_MISC,gr6);
  vga_wseq(regbase,VGA_SEQ_RESET,0x1);
  vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1 & ~VGA_SR01_SCREEN_OFF);
  vga_wseq(regbase,VGA_SEQ_RESET,0x3);
  vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1);
 }

end:
 TASK_PDIR_KERNEL_END(omm);
 return error;
err_nomem:
 error = -ENOMEM;
 for (i = 0; i < COMPILER_LENOF(state->vs_data); ++i)
      kfree(state->vs_data[i]);
 goto end;
}



PRIVATE void KCALL
do_free_vga(struct vgastate *__restrict state) {
 unsigned int i;
 for (i = 0; i < COMPILER_LENOF(state->vs_data); ++i)
      kfree(state->vs_data[i]);
}

PUBLIC void KCALL
load_vga(struct vgastate *__restrict state, bool call_free_vga) {
 unsigned int i; MMIO void *regbase;
 struct mman *omm;
 CHECK_HOST_DOBJ(state);
 TASK_PDIR_KERNEL_BEGIN(omm);
 regbase = state->vs_mmio;

 if (state->vs_saved&VGA_SAVE_MODE) {
  u16 iobase;
  vga_w(regbase,VGA_MIS_W,state->vs_mis[0]);
  iobase = state->vs_mis[0] & VGA_MIS_COLOR ? VGA_CRT_IC : VGA_CRT_IM;
  /* Turn off display */
  vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,state->vs_seq[VGA_SEQ_CLOCK_MODE]|0x20);
  /* Disable sequencer */
  vga_wseq(regbase,VGA_SEQ_RESET,0x01);
  /* Enable palette addressing */
  vga_r(regbase,iobase+(VGA_IS1_RC-VGA_CRT_IC));
  vga_w(regbase,VGA_ATT_W,0x00);
  for (i = 2; i < VGA_SEQ_C; ++i)
       vga_wseq(regbase,i,state->vs_seq[i]);
  /* Unprotect VGA regs */
  vga_w(regbase,iobase,VGA_CRTC_V_SYNC_END);
  vga_w(regbase,iobase+1,state->vs_crt[VGA_CRTC_V_SYNC_END] & ~(VGA_CR11_LOCK_CR0_CR7));
  for (i = 0; i < VGA_CRT_C; ++i) {
   vga_w(regbase,iobase,i);
   vga_w(regbase,iobase+1,state->vs_crt[i]);
  }
  for (i = 0; i < VGA_GFX_C; ++i)
       vga_wgfx(regbase,i,state->vs_gfx[i]);
  for (i = 0; i < VGA_ATT_C; ++i) {
   vga_r(regbase,iobase+(VGA_IS1_RC-VGA_CRT_IC));
   vga_wattr(regbase,i,state->vs_att[i]);
  }
  /* Reenable sequencer */
  vga_wseq(regbase,VGA_SEQ_RESET,0x03);
  /* Turn display on */
  vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,
           state->vs_seq[VGA_SEQ_CLOCK_MODE] & ~VGA_SR01_SCREEN_OFF);
  /* Disable video/palette source */
  vga_r(regbase,iobase+(VGA_IS1_RC-VGA_CRT_IC));
  vga_w(regbase,VGA_ATT_W,0x20);
 }

 if (state->vs_saved&VGA_SAVE_FONTS) {
  u8 gr1,gr3,gr4,gr5,gr6,gr8,qr1,sr2,sr4;
  if (state->vs_saved&VGA_SAVE_MODE) {
   gr1 = state->vs_gfx[VGA_GFX_SR_ENABLE];
   gr3 = state->vs_gfx[VGA_GFX_DATA_ROTATE];
   gr4 = state->vs_gfx[VGA_GFX_PLANE_READ];
   gr5 = state->vs_gfx[VGA_GFX_MODE];
   gr6 = state->vs_gfx[VGA_GFX_MISC];
   gr8 = state->vs_gfx[VGA_GFX_BIT_MASK];
   sr2 = state->vs_seq[VGA_SEQ_PLANE_WRITE];
   sr4 = state->vs_seq[VGA_SEQ_MEMORY_MODE];
   qr1 = state->vs_seq[VGA_SEQ_CLOCK_MODE];
  } else {
   gr1 = vga_rgfx(regbase,VGA_GFX_SR_ENABLE);
   gr3 = vga_rgfx(regbase,VGA_GFX_DATA_ROTATE);
   gr4 = vga_rgfx(regbase,VGA_GFX_PLANE_READ);
   gr5 = vga_rgfx(regbase,VGA_GFX_MODE);
   gr6 = vga_rgfx(regbase,VGA_GFX_MISC);
   gr8 = vga_rgfx(regbase,VGA_GFX_BIT_MASK);
   sr2 = vga_rseq(regbase,VGA_SEQ_PLANE_WRITE);
   sr4 = vga_rseq(regbase,VGA_SEQ_MEMORY_MODE);
   qr1 = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);
  }

  /* Turn off the screen. */
  vga_wseq(regbase,VGA_SEQ_RESET,0x1);
  vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
  vga_wseq(regbase,VGA_SEQ_RESET,0x3);

#if 0
  if (state->depth == 4) {
   vga_wgfx(regbase,VGA_GFX_DATA_ROTATE,0x0);
   vga_wgfx(regbase,VGA_GFX_BIT_MASK,0xff);
   vga_wgfx(regbase,VGA_GFX_SR_ENABLE,0x00);
  }
#endif

  for (i = 0; i < 4; ++i) {
   if (!(state->vs_saved&VGA_SAVE_PLANE(i))) continue;
   vga_wseq(regbase,VGA_SEQ_PLANE_WRITE,VGA_SR02_PLANE(i));
   vga_wseq(regbase,VGA_SEQ_MEMORY_MODE,VGA_SR04_EXT_MEM|VGA_SR04_SEQ_MODE);
   vga_wgfx(regbase,VGA_GFX_PLANE_READ,i);
   vga_wgfx(regbase,VGA_GFX_MODE,0x0);
   vga_wgfx(regbase,VGA_GFX_MISC,0x5);
   memcpy(vram_addr,state->vs_data[DATA_PLANE(i)],vram_size);
  }
  /* Turn on the screen. */
  vga_wseq(regbase,VGA_SEQ_RESET,0x1);
  vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1 & ~VGA_SR01_SCREEN_OFF);
  vga_wseq(regbase,VGA_SEQ_RESET,0x3);
  /* Restore registers. */
  vga_wgfx(regbase,VGA_GFX_SR_ENABLE,gr1);
  vga_wgfx(regbase,VGA_GFX_DATA_ROTATE,gr3);
  vga_wgfx(regbase,VGA_GFX_PLANE_READ,gr4);
  vga_wgfx(regbase,VGA_GFX_MODE,gr5);
  vga_wgfx(regbase,VGA_GFX_MISC,gr6);
  vga_wgfx(regbase,VGA_GFX_BIT_MASK,gr8);
  vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1);
  vga_wseq(regbase,VGA_SEQ_PLANE_WRITE,sr2);
  vga_wseq(regbase,VGA_SEQ_MEMORY_MODE,sr4);
 }

 if (state->vs_saved&VGA_SAVE_CMAP)
     load_cmap(state->vs_mmio,state->vs_data[DATA_CMAP]);

 if (call_free_vga) do_free_vga(state);
 TASK_PDIR_KERNEL_END(omm);
}

PUBLIC void KCALL
free_vga(struct vgastate *__restrict state) {
 struct mman *omm;
 TASK_PDIR_KERNEL_BEGIN(omm);
 do_free_vga(state);
 TASK_PDIR_KERNEL_END(omm);
}



PUBLIC void KCALL
vga_set_vmode(MMIO void *regbase, struct vmode *__restrict mode) {
 struct vga_vmode vvmode;
 vga_v2vvmode(mode,&vvmode);
 vga_set_vvmode(regbase,&vvmode);
}
PUBLIC void KCALL
vga_set_vvmode(MMIO void *regbase, struct vga_vmode const *__restrict mode) {
 u8 temp,qr1 = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);

 /* Turn off the screen. */
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);

 /* Apply new graphics settings. */
 vga_wcrt(regbase,VGA_CRTC_H_TOTAL,mode->vv_htotal);
 vga_wcrt(regbase,VGA_CRTC_H_DISP,mode->vv_hdisp);
 vga_wcrt(regbase,VGA_CRTC_H_BLANK_START,mode->vv_hblank_start);
 temp = vga_rcrt(regbase,VGA_CRTC_H_BLANK_END);
 vga_wcrt(regbase,VGA_CRTC_H_BLANK_END,
         (temp&~(VGA_CR3_MASK))|VGA_CR3_ALWAYS1|
         (mode->vv_hblank_end&(VGA_CR3_MASK)));
 vga_wcrt(regbase,VGA_CRTC_H_SYNC_START,mode->vv_hsync_start);
 temp = vga_rcrt(regbase,VGA_CRTC_H_SYNC_END);
 vga_wcrt(regbase,VGA_CRTC_H_SYNC_END,
         (temp&~(VGA_CR5_H_BLANK_END_5|VGA_CR5_MASK))|
         (mode->vv_hsync_end&(VGA_CR5_H_BLANK_END_5|VGA_CR5_MASK)));
 vga_wcrt(regbase,VGA_CRTC_V_TOTAL,mode->vv_vtotal);
 vga_wcrt(regbase,VGA_CRTC_OVERFLOW,mode->vv_overflow);
 temp = vga_rcrt(regbase,VGA_CRTC_MAX_SCAN);
 vga_wcrt(regbase,VGA_CRTC_MAX_SCAN,
         (temp&~(VGA_CR9_V_BLANK_START_9|VGA_CR9_SCANDOUBLE))|
         (mode->vv_max_scan&(VGA_CR9_V_BLANK_START_9)));
 vga_wcrt(regbase,VGA_CRTC_V_SYNC_START,mode->vv_vsync_start);
 temp = vga_rcrt(regbase,VGA_CRTC_V_SYNC_END);
 vga_wcrt(regbase,VGA_CRTC_V_SYNC_END,
         (temp&~(VGA_CR11_MASK))|
         (mode->vv_vsync_end&(VGA_CR11_MASK)));
 vga_wcrt(regbase,VGA_CRTC_V_DISP_END,mode->vv_vdisp_end);
 vga_wcrt(regbase,VGA_CRTC_V_BLANK_START,mode->vv_vblank_start);
 vga_wcrt(regbase,VGA_CRTC_V_BLANK_END,mode->vv_vblank_end);

 /* Enable graphics mode. */
 vga_r(NULL,VGA_IS1_RC);
 vga_w(NULL,VGA_ATT_W,0x00);

 vga_r(NULL,VGA_IS1_RC);
 temp = vga_rattr(regbase,VGA_ATC_MODE);
 vga_r(regbase,VGA_IS1_RC);
 vga_r(NULL,VGA_IS1_RC);
 vga_wattr(regbase,VGA_ATC_MODE,temp|
           VGA_AT10_GRAPHICS|VGA_AT10_8BITPAL|VGA_AT10_BLINK);

 vga_r(NULL,VGA_IS1_RC);
 vga_w(NULL,VGA_ATT_W,0x20);

 temp = vga_rgfx(regbase,VGA_GFX_MISC);
 vga_wgfx(regbase,VGA_GFX_MISC,(temp&0xf0)|VGA_GR06_GRAPHICS_MODE);

 vga_wgfx(regbase,VGA_GFX_MODE,0x40);
 temp = vga_rgfx(regbase,VGA_GFX_COMPARE_MASK);
 vga_wgfx(regbase,VGA_GFX_COMPARE_MASK,temp&0xf0);
 vga_wgfx(regbase,VGA_GFX_BIT_MASK,0xff);

 temp = vga_rseq(regbase,VGA_SEQ_PLANE_WRITE);
 vga_wseq(regbase,VGA_SEQ_PLANE_WRITE,temp|VGA_SR02_ALL_PLANES);
 temp = vga_rseq(regbase,VGA_SEQ_MEMORY_MODE);
 vga_wseq(regbase,VGA_SEQ_MEMORY_MODE,
         (temp&0xf1)|(VGA_SR04_EXT_MEM|VGA_SR04_SEQ_MODE|VGA_SR04_CHN_4M));

 temp = vga_rcrt(regbase,VGA_CRTC_MODE);
 vga_wcrt(regbase,VGA_CRTC_MODE,temp|VGA_CR17_H_V_SIGNALS_ENABLED);

 /* Turn the screen back on. */
 temp = vga_r(regbase,VGA_MIS_W);
 vga_w(regbase,VGA_MIS_W,
      (temp&~(VGA_MIS_ENB_PLL_LOAD))|
      (mode->vv_misc&(VGA_MIS_ENB_PLL_LOAD)));
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,
         (qr1&~(VGA_SR01_SCREEN_OFF|VGA_SR01_CHAR_CLK_8DOTS))|
         (mode->vv_clockmode&(VGA_SR01_CHAR_CLK_8DOTS)));
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);

}


PUBLIC void KCALL
vga_v2vvmode(struct vmode *__restrict mode,
             struct vga_vmode *__restrict result) {
 u8 clock_x;
 memset(result,0,sizeof(struct vga_vmode));
 if (!(mode->v_resx % 8)) {
use_8dot:
  result->vv_clockmode |= VGA_SR01_CHAR_CLK_8DOTS;
  result->vv_hdisp = (u8)(mode->v_resx/8);
  clock_x = 8;
 } else if (!(mode->v_resx % 9)) {
use_9dot:
  result->vv_hdisp = (u8)(mode->v_resx/9);
  clock_x = 9;
 } else {
  /* Must adjust input width. */
  u8 a = (u8)(mode->v_resx/8);
  u8 b = (u8)(mode->v_resx/9);
  if (mode->v_resx-((u16)a*8) <
      mode->v_resx-((u16)b*9))
      goto use_8dot;
  goto use_9dot;
 }
 result->vv_vdisp_end = (u8)mode->v_resy;
 if (mode->v_resy&(1 << 8)) result->vv_overflow |= VGA_CR7_V_DISP_END_8;
 if (mode->v_resy&(1 << 9)) result->vv_overflow |= VGA_CR7_V_DISP_END_9;
 (void)clock_x;



 memset(result,0,sizeof(struct vga_vmode));

 /* 640x480 register values. */
 result->vv_htotal       = 0x5f;
 result->vv_hdisp        = 0x4f;
 result->vv_hblank_start = 0x50;
 result->vv_hblank_end   = 0x02;
 result->vv_hsync_start  = 0x52;
 result->vv_hsync_end    = VGA_CR5_H_BLANK_END_5;
 result->vv_vtotal       = 0x0b;
 result->vv_overflow     = (VGA_CR7_V_BLANK_START_8|
                            VGA_CR7_V_SYNC_START_8|
                            VGA_CR7_V_DISP_END_8|
                            VGA_CR7_V_TOTAL_9);
 result->vv_max_scan     = 0x00;
 result->vv_vsync_start  = 0xea;
 result->vv_vsync_end    = 0x8c;
 result->vv_vdisp_end    = 0xdf;
 result->vv_vblank_start = 0xe7;
 result->vv_vblank_end   = 0x04;
 result->vv_misc         = 0xe3;
 result->vv_clockmode    = 0x01;
}




PRIVATE MODULE_INIT void KCALL vga_init(void) {
#if 0
 struct vgastate st;
 save_vga(&st,NULL,0xff);

 struct vmode mode = {
     .v_resx  = 800,
     .v_resy  = 600,
     .v_pitch = 1024,
     .v_bpp   = 0,
     .v_hz    = 0,
 };
 vga_set_vmode(NULL,&mode);
 memset(vram_addr,0,vram_size);
 for(;;)
 { unsigned int i = 0;
   for (i = 0; i < 256; ++i) {
    vram_addr[i] = (u8)rand();
   }
 }
 for (;;) PREEMPTION_FREEZE();

#if 1
 load_vga(&st,true);
#else
 free_vga(&st);
#endif
#endif

#if 0 /* Disable blink-attribute */
 u8 qr1 = vga_rseq(NULL,VGA_SEQ_CLOCK_MODE);
 vga_wseq(NULL,VGA_SEQ_RESET,0x1);
 vga_wseq(NULL,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
 vga_wseq(NULL,VGA_SEQ_RESET,0x3);

 vga_r(NULL,VGA_IS1_RC);
 vga_w(NULL,VGA_ATT_W,0x00);

 vga_r(NULL,VGA_IS1_RC);
 u8 temp = vga_rattr(NULL,VGA_ATC_MODE);
 vga_r(NULL,VGA_IS1_RC);
 vga_wattr(NULL,VGA_ATC_MODE,temp & ~(VGA_AT10_BLINK));

 vga_r(NULL,VGA_IS1_RC);
 vga_w(NULL,VGA_ATT_W,0x20);

 vga_wseq(NULL,VGA_SEQ_RESET,0x1);
 vga_wseq(NULL,VGA_SEQ_CLOCK_MODE,qr1 & ~VGA_SR01_SCREEN_OFF);
 vga_wseq(NULL,VGA_SEQ_RESET,0x3);
#endif

}


DECL_END

#endif /* !GUARD_MODULES_VIDEO_VGA_C */
