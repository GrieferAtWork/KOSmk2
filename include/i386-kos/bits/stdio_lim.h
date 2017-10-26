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
#ifndef _BITS_STDIO_LIM_H
#define _BITS_STDIO_LIM_H 1

#include <features.h>

#ifdef __USE_DOS
#define L_tmpnam     14
#define FILENAME_MAX 260
#define TMP_MAX      32767
#else
#define L_tmpnam     20
#define FILENAME_MAX 4096
#define TMP_MAX      238328
#endif

#ifdef __USE_POSIX
#   define L_ctermid 9
#if !defined(__USE_XOPEN2K) || defined(__USE_GNU)
#   define L_cuserid 9
#endif
#endif
#undef  FOPEN_MAX
#ifdef __USE_DOS
#define FOPEN_MAX 20
#else
#define FOPEN_MAX 16
#endif
#ifndef IOV_MAX
#define IOV_MAX   1024
#endif

#endif /* !_BITS_STDIO_LIM_H */
