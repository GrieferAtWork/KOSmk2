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
#ifndef __GUARD_HYBRID_CHECK_H
#define __GUARD_HYBRID_CHECK_H 1

#include <__stdinc.h>

__SYSDECL_BEGIN

#ifdef __KERNEL__
#   define __OK_USER_TEXT(p,s)       ((void)(p),(void)(s),1) /* TODO: Check for readable user memory. */
#   define __OK_HOST_TEXT(p,s)       ((void)(p),(void)(s),1) /* TODO: Check for readable host memory. */
#   define __OK_USER_DATA(p,s)       ((void)(p),(void)(s),1) /* TODO: Check for writable user memory. */
#   define __OK_HOST_DATA(p,s)       ((void)(p),(void)(s),1) /* TODO: Check for writable host memory. */
#   define __DO_CHECK_USER_TEXT(p,s) ((void)(p),(void)(s))
#   define __DO_CHECK_HOST_TEXT(p,s) ((void)(p),(void)(s))
#   define __DO_CHECK_USER_DATA(p,s) ((void)(p),(void)(s))
#   define __DO_CHECK_HOST_DATA(p,s) ((void)(p),(void)(s))
#else /* __KERNEL__ */
#   define __OK_HOST_TEXT(p,s)   ((void)(s),(p) != 0)
#   define __OK_HOST_DATA(p,s)   ((void)(s),(p) != 0)
#   define __OK_USER_TEXT          __OK_HOST_TEXT
#   define __OK_USER_DATA          __OK_HOST_DATA
#ifdef __INTELLISENSE__
#   define __DO_CHECK_HOST_TEXT(p,s)  (void)((p),(s))
#   define __DO_CHECK_HOST_DATA(p,s)  (void)((p),(s))
#else
#   include <assert.h>
#   define __DO_CHECK_HOST_TEXT(p,s)  assert(!(s) || (p) != 0)
#   define __DO_CHECK_HOST_DATA(p,s)  assert(!(s) || (p) != 0)
#endif
#   define __DO_CHECK_USER_TEXT       __DO_CHECK_HOST_TEXT
#   define __DO_CHECK_USER_DATA       __DO_CHECK_HOST_DATA
#endif /* !__KERNEL__ */

#ifndef CONFIG_DEBUG
#   define __CHECK_USER_TEXT(p,s)  (void)0
#   define __CHECK_HOST_TEXT(p,s)  (void)0
#   define __CHECK_USER_DATA(p,s)  (void)0
#   define __CHECK_HOST_DATA(p,s)  (void)0
#else
#   define __CHECK_USER_TEXT        __DO_CHECK_USER_TEXT
#   define __CHECK_HOST_TEXT        __DO_CHECK_HOST_TEXT
#   define __CHECK_USER_DATA        __DO_CHECK_USER_DATA
#   define __CHECK_HOST_DATA        __DO_CHECK_HOST_DATA
#endif

#define __CHECK_USER_TOBJ(o) __CHECK_USER_TEXT(o,sizeof(*(o)))
#define __CHECK_HOST_TOBJ(o) __CHECK_HOST_TEXT(o,sizeof(*(o)))
#define __CHECK_USER_DOBJ(o) __CHECK_USER_DATA(o,sizeof(*(o)))
#define __CHECK_HOST_DOBJ(o) __CHECK_HOST_DATA(o,sizeof(*(o)))

__SYSDECL_END

#ifdef __GUARD_HYBRID_COMPILER_H
#   define OK_USER_TEXT    __OK_USER_TEXT
#   define OK_HOST_TEXT    __OK_HOST_TEXT
#   define OK_USER_DATA    __OK_USER_DATA
#   define OK_HOST_DATA    __OK_HOST_DATA
#   define CHECK_HOST_TEXT __CHECK_HOST_TEXT
#   define CHECK_HOST_DATA __CHECK_HOST_DATA
#   define CHECK_USER_TEXT __CHECK_USER_TEXT
#   define CHECK_USER_DATA __CHECK_USER_DATA
#   define CHECK_USER_TOBJ __CHECK_USER_TOBJ
#   define CHECK_HOST_TOBJ __CHECK_HOST_TOBJ
#   define CHECK_USER_DOBJ __CHECK_USER_DOBJ
#   define CHECK_HOST_DOBJ __CHECK_HOST_DOBJ
#endif

#endif /* !__GUARD_HYBRID_CHECK_H */
