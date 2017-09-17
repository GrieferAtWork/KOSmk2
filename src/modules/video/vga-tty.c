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
#ifndef GUARD_MODULES_VIDEO_VGA_TTY_C
#define GUARD_MODULES_VIDEO_VGA_TTY_C 1
#define _KOS_SOURCE 1

#include <dev/chrdev.h>
#include <hybrid/compiler.h>
#include <kernel/export.h>
#include <malloc.h>
#include <modules/tty.h>
#include <modules/vga.h>
#include <fs/inode.h>
#include <kernel/mman.h>
#include <hybrid/align.h>
#include <hybrid/limits.h>
#include <sys/syslog.h>

DECL_BEGIN

#define VGA_BASEADDR 0xB8000
#define VGA_PAGESIZE CEIL_ALIGN(VTTY_WIDTH*VTTY_HEIGHT*2,PAGESIZE)
STATIC_ASSERT(IS_ALIGNED(VGA_BASEADDR,PAGESIZE));





PRIVATE REF struct mregion *KCALL
vga_mmap(struct file *__restrict UNUSED(fp),
         pos_t pos, size_t size) {
 /* Allow mapping the VGA display to memory. */
 if (pos != 0 || size != VGA_PAGESIZE)
     return E_PTR(-EINVAL);
 return mregion_new_phys(MMAN_UNIGFP,(ppage_t)VGA_BASEADDR,VGA_PAGESIZE);
}


PRIVATE struct inodeops const vga_ops = {
    .ino_fopen = &inode_fopen_default,
    .f_mmap    = &vga_mmap,
};



PRIVATE MODULE_INIT errno_t KCALL vga_init(void) {
 struct chrdev *dev; errno_t error;
 dev = chrdev_new(sizeof(struct chrdev));
 if unlikely(!dev) return -ENOMEM;

 dev->cd_device.d_node.i_ops = &vga_ops;
 error = device_setup(&dev->cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) goto err;
 CHRDEV_REGISTER(dev,VGA_TTY);
 CHRDEV_DECREF(dev);
 return -EOK;
err:
 free(dev);
 return error;
}


DECL_END

#endif /* !GUARD_MODULES_VIDEO_VGA_TTY_C */
