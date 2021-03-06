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
#ifndef _BITS_GENERIC_SELECT_H
#define _BITS_GENERIC_SELECT_H 1
#define _BITS_SELECT_H 1

#ifndef __FD_SETSIZE
#define __FD_SETSIZE 1024
#endif

#ifndef __FD_ZERO
#define __FD_ZERO(set) \
do{ __size_t __i; fd_set *const __arr = (set); \
    for (__i = 0; __i < sizeof(fd_set)/sizeof(__fd_mask); ++__i) \
         __FDS_BITS(__arr)[__i] = 0; \
}__WHILE0
#endif /* !__FD_ZERO */

#define __FD_SET(d,set)   ((void)(__FDS_BITS(set)[__FD_ELT(d)] |= __FD_MASK(d)))
#define __FD_CLR(d,set)   ((void)(__FDS_BITS(set)[__FD_ELT(d)] &= ~__FD_MASK(d)))
#define __FD_ISSET(d,set) ((__FDS_BITS(set)[__FD_ELT(d)]&__FD_MASK(d))!=0)

#endif /* !_BITS_GENERIC_SELECT_H */
