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
#ifndef _BITS_DIRENT_H
#define _BITS_DIRENT_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>

__DECL_BEGIN

struct dirent {
union {
    __FS_TYPE(ino)     d_ino;
#ifdef __USE_KOS
    __ino32_t          d_ino32;
    __ino64_t          d_ino64;
#else /* __USE_KOS */
    __ino64_t        __padding0;
#endif /* !__USE_KOS */
};
    unsigned char      d_type;
    unsigned short int d_namlen; /*< == strlen(d_name) */
#ifdef __USE_KOS
#if defined(__KERNEL__) && defined(__GNUC__)
    char               d_name[0]; /*< Allocated as required. */
#else
    char               d_name[1]; /*< Allocated as required. */
#endif
#else
    char               d_name[256];
#endif
};

#ifdef __USE_LARGEFILE64
struct dirent64 {
union {
    __ino64_t          d_ino;
#ifdef __USE_KOS
    __ino32_t          d_ino32;
    __ino64_t          d_ino64;
#endif /* __USE_KOS */
};
    unsigned char      d_type;
    unsigned short int d_namlen; /*< == strlen(d_name) */
#ifdef __USE_KOS
#if defined(__KERNEL__) && defined(__GNUC__)
    char               d_name[0]; /*< Allocated as required. */
#else
    char               d_name[1]; /*< Allocated as required. */
#endif
#else
    char               d_name[256];
#endif
};
#endif /* __USE_LARGEFILE64 */

#define d_fileno d_ino /*< Backwards compatibility. */

#undef  _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_TYPE
#define _DIRENT_MATCHES_DIRENT64 1

__DECL_END

#endif /* !_BITS_DIRENT_H */
