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
#ifndef _SYS_FCNTL_H
#define _SYS_FCNTL_H 1

#include <fcntl.h>

#ifndef SEEK_SET
#   define SEEK_SET 0 /*< Seek from beginning of file. */
#   define SEEK_CUR 1 /*< Seek from current position. */
#   define SEEK_END 2 /*< Seek from end of file. */
#endif /* !SEEK_SET */

#ifndef L_SET
#   define L_SET  SEEK_SET /*< Seek from beginning of file. */
#   define L_CURR SEEK_CUR /*< Seek from current position. */
#   define L_INCR SEEK_CUR /*< Seek from current position. */
#   define L_XTND SEEK_END /*< Seek from end of file. */
#endif /* !L_SET */


#endif /* !_SYS_FCNTL_H */
