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
#ifndef GUARD_INCLUDE_FS_ACCESS_H
#define GUARD_INCLUDE_FS_ACCESS_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdbool.h>

DECL_BEGIN

struct dentry;
struct dentry_walker;
struct fsaccess;


struct fsaccess {
 uid_t           fa_uid;   /*< Effective fs-UID of the accessor. */
 gid_t           fa_gid;   /*< Effective fs-GID of the accessor. */
};
#define FSACCESS_SETUSER(self) ((self).fa_uid = (self).fa_gid = 0) /* TODO */
#define FSACCESS_SETHOST(self) ((self).fa_uid = (self).fa_gid = 0)
#define FSACCESS_CAP_WRITEDIR(ac) ((ac)->fa_uid == 0) /* Allow write-access to directories. */


struct dentry_walker {
 struct dentry  *dw_root;     /*< [1..1] The effective root directory not escapable and returned to following a '/' link. */
 u32             dw_nlink;    /*< Amount of symbolic links that the walker has already followed. */
 struct fsaccess dw_access;   /*< Directory access information. */
 bool            dw_nofollow; /*< When true, don't follow symbolic links. */
};


DATDEF struct dentry fs_root;
#define DENTRY_WALKER_SETKERNEL(self) \
 ((self).dw_root          = &fs_root, \
  (self).dw_nlink         = 0, \
  (self).dw_access.fa_uid = 0, \
  (self).dw_access.fa_gid = 0,\
  (self).dw_nofollow      = false)


DECL_END

#endif /* !GUARD_INCLUDE_FS_ACCESS_H */
