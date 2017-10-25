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
#ifndef GUARD_INCLUDE_FS_BASIC_TYPES_H
#define GUARD_INCLUDE_FS_BASIC_TYPES_H 1

#include <errno.h>
#include <hybrid/atomic.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>

DECL_BEGIN

#ifndef __iattrset_t_defined
#define __iattrset_t_defined 1
typedef u32 iattrset_t; /* Set of 'IATTR_*' */
#endif

#ifndef __rdmode_t_defined
#define __rdmode_t_defined 1
typedef int rdmode_t;   /* readdir-mode (One of `FILE_READDIR_*') */
#endif

#define DENTRYNAME_OFFSETOF_NAME  0
#define DENTRYNAME_OFFSETOF_SIZE  __SIZEOF_POINTER__
#define DENTRYNAME_OFFSETOF_HASH (__SIZEOF_POINTER__+__SIZEOF_SIZE_T__)
#define DENTRYNAME_SIZE          (__SIZEOF_POINTER__+2*__SIZEOF_SIZE_T__)
struct dentryname {
 char  *dn_name; /*< [1..1|0..dn_size][owned] Name of the directory entry. */
 size_t dn_size; /*< Size of 'dn_name' in characters.
                  *  NOTE: Only NULL for the filesystem root directory entry. */
 size_t dn_hash; /*< Filename hash for 'dn_name'. */
};

#define DENTRYNAME_EMPTY_HASH 0 /* Directory entry hash for an empty filename. */

DECL_END

#endif /* !GUARD_INCLUDE_FS_BASIC_TYPES_H */
