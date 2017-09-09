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
#ifndef _BITS_FCNTL_H
#define _BITS_FCNTL_H 1

#include <__stdinc.h>
#include <bits/types.h>
#include <features.h>

__DECL_BEGIN

struct flock {
 short int      l_type;   /*< Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.	*/
 short int      l_whence; /*< Where `l_start' is relative to (like `lseek'). */
 __FS_TYPE(off) l_start;  /*< Offset where the lock begins. */
 __FS_TYPE(off) l_len;    /*< Size of the locked area; zero means until EOF. */
 __pid_t        l_pid;    /*< Process holding the lock. */
};

#ifdef __USE_LARGEFILE64
struct flock64 {
 short int l_type;   /*< Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK. */
 short int l_whence; /*< Where `l_start' is relative to (like `lseek'). */
 __off64_t l_start;  /*< Offset where the lock begins. */
 __off64_t l_len;    /*< Size of the locked area; zero means until EOF. */
 __pid_t   l_pid;    /*< Process holding the lock. */
};
#endif

__DECL_END

#include "fcntl-linux.h"

#endif /* !_BITS_FCNTL_H */
