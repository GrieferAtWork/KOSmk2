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
#ifndef GUARD_KERNEL_DEV_CHRDEV_C
#define GUARD_KERNEL_DEV_CHRDEV_C 1

#include <dev/chrdev.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <sys/stat.h>

DECL_BEGIN

PUBLIC struct chrdev *KCALL
chrdev_cinit(struct chrdev *self) {
 if (self) {
  CHECK_HOST_DOBJ(self);
  CHECK_HOST_DOBJ(self);
  device_cinit(&self->cd_device);
  self->cd_device.d_node.i_attr.ia_mode      = S_IFCHR|0660;
  self->cd_device.d_node.i_attr_disk.ia_mode = S_IFCHR|0660;
 }
 return self;
}

PUBLIC void KCALL
chrdev_fini(struct chrdev *__restrict self) {
 device_fini(&self->cd_device);
}



DECL_END

#endif /* !GUARD_KERNEL_DEV_CHRDEV_C */
