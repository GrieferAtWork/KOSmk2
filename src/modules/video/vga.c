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
#include <string.h>
#include <sched/paging.h>

DECL_BEGIN

/* Base address and size of video memory. */
PRIVATE byte_t *vram_addr = (byte_t *)0xA0000;
PRIVATE size_t  vram_size = 8192*4; /* 32K */

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




PRIVATE MODULE_INIT void KCALL vga_init(void) {
#if 0
 struct vgastate st;
 save_vga(&st,NULL,0xff);

 {
#define TEST_PLANE 2
  u8 gr4,gr5,gr6,qr1,sr2,sr4;
  gr4 = vga_rgfx(NULL,VGA_GFX_PLANE_READ);
  gr5 = vga_rgfx(NULL,VGA_GFX_MODE);
  gr6 = vga_rgfx(NULL,VGA_GFX_MISC);
  sr2 = vga_rseq(NULL,VGA_SEQ_PLANE_WRITE);
  sr4 = vga_rseq(NULL,VGA_SEQ_MEMORY_MODE);
  qr1 = vga_rseq(NULL,VGA_SEQ_CLOCK_MODE);
  vga_wseq(NULL,VGA_SEQ_RESET,0x1);
  vga_wseq(NULL,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
  vga_wseq(NULL,VGA_SEQ_RESET,0x3);

  vga_wseq(NULL,VGA_SEQ_PLANE_WRITE,VGA_SR02_PLANE(TEST_PLANE));
  vga_wseq(NULL,VGA_SEQ_MEMORY_MODE,VGA_SR04_EXT_MEM|VGA_SR04_SEQ_MODE);
  vga_wgfx(NULL,VGA_GFX_PLANE_READ,TEST_PLANE);
  vga_wgfx(NULL,VGA_GFX_MODE,0x0);
  vga_wgfx(NULL,VGA_GFX_MISC,0x5);
  memset(vram_addr,0xff,vram_size); /* Break character sprites. */

  vga_wseq(NULL,VGA_SEQ_PLANE_WRITE,sr2);
  vga_wseq(NULL,VGA_SEQ_MEMORY_MODE,sr4);
  vga_wgfx(NULL,VGA_GFX_PLANE_READ,gr4);
  vga_wgfx(NULL,VGA_GFX_MODE,gr5);
  vga_wgfx(NULL,VGA_GFX_MISC,gr6);
  vga_wseq(NULL,VGA_SEQ_RESET,0x1);
  vga_wseq(NULL,VGA_SEQ_CLOCK_MODE,qr1 & ~VGA_SR01_SCREEN_OFF);
  vga_wseq(NULL,VGA_SEQ_RESET,0x3);
  vga_wseq(NULL,VGA_SEQ_CLOCK_MODE,qr1);
 }

#if 1
 load_vga(&st,true);
#else
 free_vga(&st);
#endif
#endif
}


DECL_END

#endif /* !GUARD_MODULES_VIDEO_VGA_C */
