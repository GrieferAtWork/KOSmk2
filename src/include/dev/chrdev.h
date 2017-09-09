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
#ifndef GUARD_INCLUDE_DEV_CHRDEV_H
#define GUARD_INCLUDE_DEV_CHRDEV_H 1

#include <dev/device.h>
#include <hybrid/compiler.h>

DECL_BEGIN

struct chrdev {
 struct device cd_device; /*< The underlying device. */
};
#define CHRDEV_TRYINCREF(self)  DEVICE_TRYINCREF(&(self)->cd_device)
#define CHRDEV_INCREF(self)     DEVICE_INCREF(&(self)->cd_device)
#define CHRDEV_DECREF(self)     DEVICE_DECREF(&(self)->cd_device)

/* Create a new block device. The caller must fill in:
 *  - cd_device.d_node.i_super (Use 'device_setup')
 *  - cd_device.d_node.i_data (Optionally)
 *  - cd_device.d_node.i_ops
 */
#define chrdev_new(type_size) \
        chrdev_cinit((struct chrdev *)calloc(1,type_size))
FUNDEF struct chrdev *KCALL chrdev_cinit(struct chrdev *self);

/* Character device finalization (Must be called from 'cd_device.d_node.i_ops->ino_fini') */
FUNDEF void KCALL chrdev_fini(struct chrdev *__restrict self);

/* Helper macro for opening a character device for reading/writing. */
#define chrdev_open(self,oflags) inode_kopen(&(self)->cd_device.d_node,oflags)


DECL_END

#endif /* !GUARD_INCLUDE_DEV_CHRDEV_H */
