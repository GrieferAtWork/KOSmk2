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
#ifndef _KOS_KSYM_H
#define _KOS_KSYM_H 1

#include <__stdinc.h>
#include <features.h>

#define __KSYM_PREFIX  ".kern."

#ifdef __KERNEL__
#   define __KSYM(name) __ASMNAME(#name)
#elif defined(__INTELLISENSE__)
#   define __KSYM(name) /* Nothing */
#else
#   define __KSYM(name) __ASMNAME(".kern." #name) __ATTR_WEAK
#endif


#if !defined(__KERNEL__) && defined(__USE_KOS)
__DECL_BEGIN
/* Load the address of a user-share symbol 'name', or NULL with errno set. */
__LIBC void *(__LIBCCALL xsharesym)(char const *name);
__DECL_END
#endif

#endif /* !_KOS_KSYM_H */
