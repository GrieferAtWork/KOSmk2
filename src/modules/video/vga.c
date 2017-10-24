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
#include <hybrid/section.h>
#include <kernel/boot.h>
#include <kernel/arch/realmode.h>
#include <kernel/arch/cpustate.h>

DECL_BEGIN

#define VRAM_PLANE  (8192*4) /* 32K */

/* Base address and size of video memory. */
PRIVATE ATTR_USED PHYS byte_t *vram_addr = (byte_t *)0xA0000;
PRIVATE ATTR_USED      size_t  vram_size = VRAM_PLANE*4; /* 128K */

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
 if (save&VGA_SAVE_PLANE0 && (state->vs_data[DATA_PLANE0] = (byte_t *)kmalloc(VRAM_PLANE,GFP_KERNEL)) == NULL) goto err_nomem;
 if (save&VGA_SAVE_PLANE1 && (state->vs_data[DATA_PLANE1] = (byte_t *)kmalloc(VRAM_PLANE,GFP_KERNEL)) == NULL) goto err_nomem;
 if (save&VGA_SAVE_PLANE2 && (state->vs_data[DATA_PLANE2] = (byte_t *)kmalloc(VRAM_PLANE,GFP_KERNEL)) == NULL) goto err_nomem;
 if (save&VGA_SAVE_PLANE3 && (state->vs_data[DATA_PLANE3] = (byte_t *)kmalloc(VRAM_PLANE,GFP_KERNEL)) == NULL) goto err_nomem;
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
   memcpy(state->vs_data[DATA_PLANE(i)],vram_addr,VRAM_PLANE);
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
   memcpy(vram_addr,state->vs_data[DATA_PLANE(i)],VRAM_PLANE);
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
vga_setmode(MMIO void *regbase,
            struct vga_mode const *__restrict mode) {
 /* Disable preemption to prevent interference. */
 pflag_t was = PREEMPTION_PUSH();
 u8 temp,qr1 = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);

 /* Validate the given mode. */
 /* TODO: Stuff like this: assert(!(mode->vm_att_mode&0x10)); */

 /* Turn off the screen. */
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);

 /* Enable graphics mode. */
 vga_r(regbase,VGA_IS1_RC),vga_w(regbase,VGA_ATT_W,0x00);
 vga_r(regbase,VGA_IS1_RC),temp = vga_rattr(regbase,VGA_ATC_MODE);
 vga_r(regbase,VGA_IS1_RC),vga_wattr(regbase,VGA_ATC_MODE,(temp&VGA_AT10_RESERVED)|mode->vm_att_mode);
 vga_r(regbase,VGA_IS1_RC),vga_wattr(regbase,VGA_ATC_OVERSCAN,mode->vm_att_overscan);
 vga_r(regbase,VGA_IS1_RC),temp = vga_rattr(regbase,VGA_ATC_PLANE_ENABLE);
 vga_r(regbase,VGA_IS1_RC),vga_wattr(regbase,VGA_ATC_PLANE_ENABLE,(temp&~VGA_AT12_MASK)|mode->vm_att_plane_enable);
 vga_r(regbase,VGA_IS1_RC),temp = vga_rattr(regbase,VGA_ATC_PEL);
 vga_r(regbase,VGA_IS1_RC),vga_wattr(regbase,VGA_ATC_PEL,(temp&VGA_AT13_RESERVED)|mode->vm_att_pel);
 vga_r(regbase,VGA_IS1_RC),temp = vga_rattr(regbase,VGA_ATC_COLOR_PAGE);
 vga_r(regbase,VGA_IS1_RC),vga_wattr(regbase,VGA_ATC_COLOR_PAGE,(temp&VGA_AT14_RESERVED)|mode->vm_att_color_page);
 vga_r(regbase,VGA_IS1_RC),vga_w(regbase,VGA_ATT_W,0x20);

 temp = vga_r(regbase,VGA_MIS_R);
 vga_w(regbase,VGA_MIS_W,(temp&VGA_MIS_RESERVED)|mode->vm_mis);

 temp = vga_rseq(regbase,VGA_SEQ_PLANE_WRITE);
 vga_wseq(regbase,VGA_SEQ_PLANE_WRITE,(temp&VGA_SR02_RESERVED)|mode->vm_seq_plane_write);
 temp = vga_rseq(regbase,VGA_SEQ_CHARACTER_MAP);
 vga_wseq(regbase,VGA_SEQ_CHARACTER_MAP,(temp&VGA_SR03_RESERVED)|mode->vm_seq_character_map);
 temp = vga_rseq(regbase,VGA_SEQ_MEMORY_MODE);
 vga_wseq(regbase,VGA_SEQ_MEMORY_MODE,(temp&VGA_SR04_RESERVED)|mode->vm_seq_memory_mode);

 temp = vga_rgfx(regbase,VGA_GFX_SR_VALUE),vga_wgfx(regbase,VGA_GFX_SR_VALUE,(temp&VGA_GR00_RESERVED)|mode->vm_gfx_sr_value);
 temp = vga_rgfx(regbase,VGA_GFX_SR_ENABLE),vga_wgfx(regbase,VGA_GFX_SR_ENABLE,(temp&VGA_GR01_RESERVED)|mode->vm_gfx_sr_enable);
 temp = vga_rgfx(regbase,VGA_GFX_COMPARE_VALUE),vga_wgfx(regbase,VGA_GFX_COMPARE_VALUE,(temp&VGA_GR02_RESERVED)|mode->vm_gfx_compare_value);
 temp = vga_rgfx(regbase,VGA_GFX_DATA_ROTATE),vga_wgfx(regbase,VGA_GFX_DATA_ROTATE,(temp&VGA_GR03_RESERVED)|mode->vm_gfx_data_rotate);
 temp = vga_rgfx(regbase,VGA_GFX_MODE),vga_wgfx(regbase,VGA_GFX_MODE,(temp&VGA_GR05_RESERVED)|mode->vm_gfx_mode);
 temp = vga_rgfx(regbase,VGA_GFX_MISC),vga_wgfx(regbase,VGA_GFX_MISC,(temp&VGA_GR06_RESERVED)|mode->vm_gfx_misc);
 temp = vga_rgfx(regbase,VGA_GFX_COMPARE_MASK),vga_wgfx(regbase,VGA_GFX_COMPARE_MASK,(temp&VGA_GR07_RESERVED)|mode->vm_gfx_compare_mask);
 vga_wgfx(regbase,VGA_GFX_BIT_MASK,mode->vm_gfx_bit_mask);

 /* Apply new graphics settings. */
 vga_wcrt(regbase,VGA_CRTC_H_TOTAL,mode->vm_crt_h_total);
 vga_wcrt(regbase,VGA_CRTC_H_DISP,mode->vm_crt_h_disp);
 vga_wcrt(regbase,VGA_CRTC_H_BLANK_START,mode->vm_crt_h_blank_start);
 vga_wcrt(regbase,VGA_CRTC_H_BLANK_END,mode->vm_crt_h_blank_end);
 vga_wcrt(regbase,VGA_CRTC_H_SYNC_START,mode->vm_crt_h_sync_start);
 vga_wcrt(regbase,VGA_CRTC_H_SYNC_END,mode->vm_crt_h_sync_end);
 vga_wcrt(regbase,VGA_CRTC_V_TOTAL,mode->vm_crt_v_total);
 vga_wcrt(regbase,VGA_CRTC_OVERFLOW,mode->vm_crt_overflow);
 temp = vga_rcrt(regbase,VGA_CRTC_PRESET_ROW);
 vga_wcrt(regbase,VGA_CRTC_PRESET_ROW,(temp&VGA_CR8_RESERVED)|mode->vm_crt_preset_row);
 vga_wcrt(regbase,VGA_CRTC_MAX_SCAN,mode->vm_crt_max_scan);
 vga_wcrt(regbase,VGA_CRTC_V_SYNC_START,mode->vm_crt_v_sync_start);
 temp = vga_rcrt(regbase,VGA_CRTC_V_SYNC_END);
 vga_wcrt(regbase,VGA_CRTC_V_SYNC_END,(temp&VGA_CR11_RESERVED)|mode->vm_crt_v_sync_end);
 vga_wcrt(regbase,VGA_CRTC_V_DISP_END,mode->vm_crt_v_disp_end);
 vga_wcrt(regbase,VGA_CRTC_OFFSET,mode->vm_crt_offset);
 vga_wcrt(regbase,VGA_CRTC_UNDERLINE,mode->vm_crt_underline);
 vga_wcrt(regbase,VGA_CRTC_V_BLANK_START,mode->vm_crt_v_blank_start);
 temp = vga_rcrt(regbase,VGA_CRTC_V_BLANK_END);
 vga_wcrt(regbase,VGA_CRTC_V_BLANK_END,(temp&VGA_CR16_RESERVED)|mode->vm_crt_v_blank_end);
 temp = vga_rcrt(regbase,VGA_CRTC_MODE);
 vga_wcrt(regbase,VGA_CRTC_MODE,(temp&VGA_CR17_RESERVED)|mode->vm_crt_mode);
 vga_wcrt(regbase,VGA_CRTC_LINE_COMPARE,mode->vm_crt_line_compare);

 /* Turn the screen back on. */
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,(qr1&VGA_SR01_RESERVED)|mode->vm_seq_clock_mode);
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);
 PREEMPTION_POP(was);
}

PUBLIC void KCALL
vga_getmode(MMIO void *regbase, struct vga_mode *__restrict mode) {
 /* Disable preemption to prevent interference. */
 pflag_t was = PREEMPTION_PUSH();
 vga_r(regbase,VGA_IS1_RC),vga_w(regbase,VGA_ATT_W,0x00);
 vga_r(regbase,VGA_IS1_RC),mode->vm_att_mode         = vga_rattr(regbase,VGA_ATC_MODE) & ~VGA_AT10_RESERVED;
 vga_r(regbase,VGA_IS1_RC),mode->vm_att_overscan     = vga_rattr(regbase,VGA_ATC_OVERSCAN);
 vga_r(regbase,VGA_IS1_RC),mode->vm_att_plane_enable = vga_rattr(regbase,VGA_ATC_PLANE_ENABLE) & ~VGA_AT12_RESERVED;
 vga_r(regbase,VGA_IS1_RC),mode->vm_att_pel          = vga_rattr(regbase,VGA_ATC_PEL) & ~VGA_AT13_RESERVED;
 vga_r(regbase,VGA_IS1_RC),mode->vm_att_color_page   = vga_rattr(regbase,VGA_ATC_COLOR_PAGE) & ~VGA_AT14_RESERVED;
 vga_r(regbase,VGA_IS1_RC),vga_w(regbase,VGA_ATT_W,0x20);

 mode->vm_mis               = vga_r(regbase,VGA_MIS_R) & ~VGA_MIS_RESERVED;
 mode->vm_gfx_sr_value      = vga_rgfx(regbase,VGA_GFX_SR_VALUE) & ~VGA_GR00_RESERVED;
 mode->vm_gfx_sr_enable     = vga_rgfx(regbase,VGA_GFX_SR_ENABLE) & ~VGA_GR01_RESERVED;
 mode->vm_gfx_compare_value = vga_rgfx(regbase,VGA_GFX_COMPARE_VALUE) & ~VGA_GR02_RESERVED;
 mode->vm_gfx_data_rotate   = vga_rgfx(regbase,VGA_GFX_DATA_ROTATE) & ~VGA_GR03_RESERVED;
 mode->vm_gfx_mode          = vga_rgfx(regbase,VGA_GFX_MODE) & ~VGA_GR05_RESERVED;
 mode->vm_gfx_misc          = vga_rgfx(regbase,VGA_GFX_MISC) & ~VGA_GR06_RESERVED;
 mode->vm_gfx_compare_mask  = vga_rgfx(regbase,VGA_GFX_COMPARE_MASK) & ~VGA_GR07_RESERVED;
 mode->vm_gfx_bit_mask      = vga_rgfx(regbase,VGA_GFX_BIT_MASK);
 mode->vm_crt_h_total       = vga_rcrt(regbase,VGA_CRTC_H_TOTAL);
 mode->vm_crt_h_disp        = vga_rcrt(regbase,VGA_CRTC_H_DISP);
 mode->vm_crt_h_blank_start = vga_rcrt(regbase,VGA_CRTC_H_BLANK_START);
 mode->vm_crt_h_blank_end   = vga_rcrt(regbase,VGA_CRTC_H_BLANK_END);
 mode->vm_crt_h_sync_start  = vga_rcrt(regbase,VGA_CRTC_H_SYNC_START);
 mode->vm_crt_h_sync_end    = vga_rcrt(regbase,VGA_CRTC_H_SYNC_END);
 mode->vm_crt_v_total       = vga_rcrt(regbase,VGA_CRTC_V_TOTAL);
 mode->vm_crt_overflow      = vga_rcrt(regbase,VGA_CRTC_OVERFLOW);
 mode->vm_crt_preset_row    = vga_rcrt(regbase,VGA_CRTC_PRESET_ROW) & ~VGA_CR8_RESERVED;
 mode->vm_crt_max_scan      = vga_rcrt(regbase,VGA_CRTC_MAX_SCAN);
 mode->vm_crt_v_sync_start  = vga_rcrt(regbase,VGA_CRTC_V_SYNC_START);
 mode->vm_crt_v_sync_end    = vga_rcrt(regbase,VGA_CRTC_V_SYNC_END) & ~VGA_CR11_RESERVED;
 mode->vm_crt_v_disp_end    = vga_rcrt(regbase,VGA_CRTC_V_DISP_END);
 mode->vm_crt_offset        = vga_rcrt(regbase,VGA_CRTC_OFFSET);
 mode->vm_crt_underline     = vga_rcrt(regbase,VGA_CRTC_UNDERLINE);
 mode->vm_crt_v_blank_start = vga_rcrt(regbase,VGA_CRTC_V_BLANK_START);
 mode->vm_crt_v_blank_end   = vga_rcrt(regbase,VGA_CRTC_V_BLANK_END) & ~VGA_CR16_RESERVED;
 mode->vm_crt_mode          = vga_rcrt(regbase,VGA_CRTC_MODE) & ~VGA_CR17_RESERVED;
 mode->vm_crt_line_compare  = vga_rcrt(regbase,VGA_CRTC_LINE_COMPARE);
 mode->vm_seq_plane_write   = vga_rseq(regbase,VGA_SEQ_PLANE_WRITE) & ~VGA_SR02_RESERVED;
 mode->vm_seq_character_map = vga_rseq(regbase,VGA_SEQ_CHARACTER_MAP) & ~VGA_SR03_RESERVED;
 mode->vm_seq_memory_mode   = vga_rseq(regbase,VGA_SEQ_MEMORY_MODE) & ~VGA_SR04_RESERVED;
 mode->vm_seq_clock_mode    = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE) & ~VGA_SR01_RESERVED;
 PREEMPTION_POP(was);
}

#define VGA_FONTPLANE      2
#define VGA_CHARSIZE_MIN   8
#define VGA_CHARSIZE_MAX  32

PUBLIC bool KCALL
vga_setfont(MMIO void *regbase, struct vga_font const *__restrict font) {
 u8 gr1,gr3,gr4,gr5,gr6,gr8,qr1,sr2,sr4; pflag_t was;
 byte_t *src,*dst,*end;
 CHECK_HOST_DOBJ(font);
 if (!font->vf_data) return false;
 assert(font->vf_cheight >= VGA_CHARSIZE_MIN &&
        font->vf_cheight <= VGA_CHARSIZE_MAX);
 was = PREEMPTION_PUSH();
 gr1 = vga_rgfx(regbase,VGA_GFX_SR_ENABLE);
 gr3 = vga_rgfx(regbase,VGA_GFX_DATA_ROTATE);
 gr4 = vga_rgfx(regbase,VGA_GFX_PLANE_READ);
 gr5 = vga_rgfx(regbase,VGA_GFX_MODE);
 gr6 = vga_rgfx(regbase,VGA_GFX_MISC);
 gr8 = vga_rgfx(regbase,VGA_GFX_BIT_MASK);
 sr2 = vga_rseq(regbase,VGA_SEQ_PLANE_WRITE);
 sr4 = vga_rseq(regbase,VGA_SEQ_MEMORY_MODE);
 qr1 = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);

 /* Turn off the screen. */
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);

 vga_wseq(regbase,VGA_SEQ_PLANE_WRITE,VGA_SR02_PLANE(VGA_FONTPLANE));
 vga_wseq(regbase,VGA_SEQ_MEMORY_MODE,VGA_SR04_EXT_MEM|VGA_SR04_SEQ_MODE);
 vga_wgfx(regbase,VGA_GFX_PLANE_READ,VGA_FONTPLANE);
 vga_wgfx(regbase,VGA_GFX_MODE,0x0);
 vga_wgfx(regbase,VGA_GFX_MISC,0x5);
 PREEMPTION_POP(was);

 src = font->vf_data;
 dst = vram_addr;
 end = src+256*font->vf_cheight;

 /* Copy information into VRAM. */
 for (; src != end; src += font->vf_cheight,dst += VGA_CHARSIZE_MAX) {
  memcpy(dst,src,font->vf_cheight);
  memset(dst+font->vf_cheight,0,VGA_CHARSIZE_MAX-font->vf_cheight);
 }

 /* Turn on the screen. */
 was = PREEMPTION_PUSH();
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
 PREEMPTION_POP(was);
 return true;
}

PUBLIC bool KCALL
vga_getfont(MMIO void *regbase, struct vga_font *__restrict font) {
 u8 gr4,gr5,gr6,qr1,sr2,sr4; struct mman *omm;
 pflag_t was = PREEMPTION_PUSH();
 byte_t *dst,*src,*end;
 TASK_PDIR_KERNEL_BEGIN(omm);
 gr4 = vga_rgfx(regbase,VGA_GFX_PLANE_READ);
 gr5 = vga_rgfx(regbase,VGA_GFX_MODE);
 gr6 = vga_rgfx(regbase,VGA_GFX_MISC);
 sr2 = vga_rseq(regbase,VGA_SEQ_PLANE_WRITE);
 sr4 = vga_rseq(regbase,VGA_SEQ_MEMORY_MODE);
 qr1 = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);

 /* Turn off the screen to hide what we're doing. */
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);
 vga_wseq(regbase,VGA_SEQ_PLANE_WRITE,VGA_SR02_PLANE(VGA_FONTPLANE));
 vga_wseq(regbase,VGA_SEQ_MEMORY_MODE,VGA_SR04_EXT_MEM|VGA_SR04_SEQ_MODE);
 vga_wgfx(regbase,VGA_GFX_PLANE_READ,VGA_FONTPLANE);
 vga_wgfx(regbase,VGA_GFX_MODE,0x0);
 vga_wgfx(regbase,VGA_GFX_MISC,0x5);
 PREEMPTION_POP(was);

 /* Probe character db (y: 13, x: 11), which should be a fully colored block. */
 { byte_t *probe_char = vram_addr+(0xdb*VGA_CHARSIZE_MAX);
   byte_t *iter = probe_char,*char_end = probe_char+VGA_CHARSIZE_MAX;
   while (iter != char_end && *iter) ++iter;
   /* unlikely case, but could happen if the BIOS doesn't support character > 127 */
   if unlikely(iter == probe_char) {
    probe_char = vram_addr+((int)'_'*VGA_CHARSIZE_MAX);
    iter = probe_char,char_end = probe_char+VGA_CHARSIZE_MAX;
    while (iter != char_end && *iter) ++iter;
   }
   font->vf_cheight = (u8)(iter-probe_char);
   /* Ensure sure that character data is at least 'VGA_CHARSIZE_MIN' pixels tall.
    * NOTE: This also handles the case where we could find information on '_'. */
   if (font->vf_cheight < VGA_CHARSIZE_MIN) font->vf_cheight = VGA_CHARSIZE_MIN;
   if (font->vf_cheight&1) ++font->vf_cheight;
   assert(font->vf_cheight <= VGA_CHARSIZE_MAX);
   font->vf_data = (byte_t *)kmalloc(256*font->vf_cheight,GFP_KERNEL);
 }
 if unlikely(!font->vf_data) goto done_font;

 dst = font->vf_data;
 src = vram_addr;
 end = dst+256*font->vf_cheight;

 /* Copy information into memory. */
 for (; dst != end; dst += font->vf_cheight,src += VGA_CHARSIZE_MAX)
        memcpy(dst,src,font->vf_cheight);

 /* With font information stored, restore registers and turn the screen back on. */
done_font:
 was = PREEMPTION_PUSH();
 vga_wseq(regbase,VGA_SEQ_PLANE_WRITE,sr2);
 vga_wseq(regbase,VGA_SEQ_MEMORY_MODE,sr4);
 vga_wgfx(regbase,VGA_GFX_PLANE_READ,gr4);
 vga_wgfx(regbase,VGA_GFX_MODE,gr5);
 vga_wgfx(regbase,VGA_GFX_MISC,gr6);
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1 & ~VGA_SR01_SCREEN_OFF);
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1);
 PREEMPTION_POP(was);
 TASK_PDIR_KERNEL_END(omm);
 return font->vf_data != NULL;
}


PUBLIC struct vga_font *vf_current = &vf_bios;
PUBLIC struct vga_font vf_bios = {
   .vf_data    = NULL,
   .vf_cheight = 0,
};
PUBLIC struct vga_mode vm_text = {
    .vm_att_mode          = 0x0c,
    .vm_att_overscan      = 0x00,
    .vm_att_plane_enable  = 0x0f,
    .vm_att_pel           = 0x08,
    .vm_att_color_page    = 0x00,
    .vm_mis               = 0xe3,
    .vm_gfx_sr_value      = 0x00,
    .vm_gfx_sr_enable     = 0x00,
    .vm_gfx_compare_value = 0x00,
    .vm_gfx_data_rotate   = 0x00,
    .vm_gfx_mode          = 0x10,
    .vm_gfx_misc          = 0x0e,
    .vm_gfx_compare_mask  = 0x0f,
    .vm_gfx_bit_mask      = 0xff,
    .vm_crt_h_total       = 0x5f,
    .vm_crt_h_disp        = 0x4f,
    .vm_crt_h_blank_start = 0x50,
    .vm_crt_h_blank_end   = 0x82,
    .vm_crt_h_sync_start  = 0x55,
    .vm_crt_h_sync_end    = 0x81,
    .vm_crt_v_total       = 0xbf,
    .vm_crt_overflow      = 0x1f,
    .vm_crt_preset_row    = 0x00,
    .vm_crt_max_scan      = 0x4f,
    .vm_crt_v_sync_start  = 0x9c,
    .vm_crt_v_sync_end    = 0x8e,
    .vm_crt_v_disp_end    = 0x8f,
    .vm_crt_offset        = 0x28,
    .vm_crt_underline     = 0x1f,
    .vm_crt_v_blank_start = 0x96,
    .vm_crt_v_blank_end   = 0xb9,
    .vm_crt_mode          = 0xa3,
    .vm_crt_line_compare  = 0xff,
    .vm_seq_plane_write   = 0x3,
    .vm_seq_character_map = 0x00,
    .vm_seq_memory_mode   = 0x02,
    .vm_seq_clock_mode    = 0x00,
};
PUBLIC struct vga_mode vm_modeX = {
    .vm_att_mode          = 0x41,
    .vm_att_overscan      = 0x00,
    .vm_att_plane_enable  = 0x0f,
    .vm_att_pel           = 0x00,
    .vm_att_color_page    = 0x00,
    .vm_mis               = 0xe3,
    .vm_gfx_sr_value      = 0x00,
    .vm_gfx_sr_enable     = 0x00,
    .vm_gfx_compare_value = 0x00,
    .vm_gfx_data_rotate   = 0x00,
    .vm_gfx_mode          = 0x40,
    .vm_gfx_misc          = 0x01,
    .vm_gfx_compare_mask  = 0x00,
    .vm_gfx_bit_mask      = 0xff,
    .vm_crt_h_total       = 0x5f,
    .vm_crt_h_disp        = 0x4f,
    .vm_crt_h_blank_start = 0x50,
    .vm_crt_h_blank_end   = 0x82,
    .vm_crt_h_sync_start  = 0x54,
    .vm_crt_h_sync_end    = 0x80,
    .vm_crt_v_total       = 0x0d,
    .vm_crt_overflow      = 0x3e,
    .vm_crt_preset_row    = 0x00,
    .vm_crt_max_scan      = 0x41,
    .vm_crt_v_sync_start  = 0xea,
    .vm_crt_v_sync_end    = 0xac,
    .vm_crt_v_disp_end    = 0xdf,
    .vm_crt_offset        = 0x28,
    .vm_crt_underline     = 0x00,
    .vm_crt_v_blank_start = 0xe7,
    .vm_crt_v_blank_end   = 0x06,
    .vm_crt_mode          = 0xe3,
    .vm_crt_line_compare  = 0xff,
    .vm_seq_plane_write   = VGA_SR02_ALL_PLANES,
    .vm_seq_character_map = 0x00,
    .vm_seq_memory_mode   = 0x0e,
    .vm_seq_clock_mode    = 0x01,
};



PRIVATE struct inodeops vga_ops = {
};

PRIVATE ATTR_FREETEXT errno_t KCALL vga_probe(void) {
 vga_t *result; errno_t error;
 result = (vga_t *)chrdev_new(sizeof(vga_t));
 if unlikely(!result) return -ENOMEM;
 result->v_device.cd_device.d_node.i_ops = &vga_ops;
 result->v_crt_i = VGA_CRT_IC;
 result->v_crt_d = VGA_CRT_DC;
 result->v_is1_r = VGA_IS1_RC;
 if unlikely(!(vga_r(result->v_mmio,VGA_MIS_R)&VGA_MIS_COLOR)) {
  result->v_crt_i = VGA_CRT_IM;
  result->v_crt_d = VGA_CRT_DM;
  result->v_is1_r = VGA_IS1_RM;
 }
 error = device_setup(&result->v_device.cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) { free(result); return error; }
 error = CHRDEV_REGISTER(&result->v_device,DV_VGA);
 CHRDEV_DECREF(&result->v_device);
 return error;
}

PRIVATE ATTR_FREETEXT void KCALL
vga_disable_annoying_blinking(void) {
 u8 qr1; /* QEMU don't emulate this annoyingness, so no need. */
 if (boot_emulation == BOOT_EMULATION_QEMU) return;

 qr1 = vga_rseq(NULL,VGA_SEQ_CLOCK_MODE);
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
}

PRIVATE MODULE_INIT void KCALL vga_init(void) {
 errno_t error;
 /* Finally! No more seizures! */
 vga_disable_annoying_blinking();

#if 1
 /* Don't break real hardware until I'm confident this'll work. */
 if (boot_emulation == BOOT_EMULATION_REALHW)
     return;
#endif

 if ((error = vga_probe(),E_ISERR(error))) {
  syslog(LOG_ERROR,
         FREESTR("[VGA] Failed to initialize default adapater: %[errno]\n"),
         -error);
  return;
 }

 /* Save the BIOS VGA font. */
 if (!vga_getfont(NULL,&vf_bios)) {
  syslog(LOG_ERROR,
         FREESTR("[VGA] Failed to load BIOS VGA font: %[errno]\n"),
         ENOMEM);
 }

#if 1
#if 1
 struct vgastate st;
 save_vga(&st,NULL,0xff);
 { struct cpustate16 state;
   memset(&state,0,sizeof(state));
   state.gp.ah = 0;
   state.gp.al = 0x13; /* 320x200 256-color */
   rm_interrupt(&state,0x10);
   /* TODO: Since our modeX doesn't seem to work on real hardware (But this BIOS interrupt does),
    *       do a register dump at this point and analyze the differences to our modeX
    *       NOTE: Remember: This is 320x200, while modeX is supposed to be 320x240!
    */
 }

#else
 vga_setmode(NULL,&vm_modeX);
 memset(vram_addr,1,vram_size);
#endif

#if 0
#elif 1
 { unsigned int x,y;
   unsigned int w = 320;
   unsigned int h = 240;
   for (y = 0; y < 20; ++y) {
    for (x = 0; x < 20; ++x) {
     vram_addr[x+y*w] = (u8)(x+y);
    }
   }
   vram_addr[0] = 15;
   (void)h;
 }
#endif
 //PREEMPTION_FREEZE();                      
 //vga_setmode(NULL,&vm_text);
 //vga_setfont(NULL,&vf_bios);
 //PREEMPTION_FREEZE();                      
 load_vga(&st,true);
#endif
}


DECL_END

#endif /* !GUARD_MODULES_VIDEO_VGA_C */
