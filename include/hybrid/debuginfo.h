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
#ifndef GUARD_HYBRID_DEBUGINFO_H
#define GUARD_HYBRID_DEBUGINFO_H 1

#include <__stdinc.h>

__DECL_BEGIN

#ifdef __KERNEL__
#ifndef THIS_INSTANCE
#ifdef __CC__
struct instance;
#ifdef CONFIG_BUILDING_KERNEL_CORE
__INTDEF struct instance __this_instance;
#else
__PUBDEF struct instance __this_instance;
#endif
#endif /* __CC__ */
#define THIS_INSTANCE  (&__this_instance)
#endif /* !THIS_INSTANCE */
#   define __DEBUGINFO         char const *__file, int __line, char const *__func, struct instance *__inst
#   define __DEBUGINFO_GEN     __FILE__,__LINE__,__FUNCTION__,THIS_INSTANCE
#   define __DEBUGINFO_MK(file,line,func) file,line,func,THIS_INSTANCE
#   define __DEBUGINFO_MUNUSED char const *__file, int __line, char const *__func, struct instance *__UNUSED(__inst)
#   define __DEBUGINFO_UNUSED  char const *__UNUSED(__file), int __UNUSED(__line), char const *__UNUSED(__func), struct instance *__UNUSED(__inst)
#   define __DEBUGINFO_FWD     __file,__line,__func,__inst
#   define __DEBUGINFO_NUL     NULL,0,NULL,NULL
#else /* __KERNEL__ */
#   define __DEBUGINFO         char const *__file, int __line, char const *__func
#   define __DEBUGINFO_GEN     __FILE__,__LINE__,__FUNCTION__
#   define __DEBUGINFO_MK(file,line,func) file,line,func
#   define __DEBUGINFO_MUNUSED char const *__file, int __line, char const *__func
#   define __DEBUGINFO_UNUSED  char const *__UNUSED(__file), int __UNUSED(__line), char const *__UNUSED(__func)
#   define __DEBUGINFO_FWD     __file,__line,__func
#   define __DEBUGINFO_NUL     NULL,0,NULL
#endif /* !__KERNEL__ */

#ifdef GUARD_HYBRID_COMPILER_H
#   define DEBUGINFO         __DEBUGINFO
#   define DEBUGINFO_MK(file,line,func) __DEBUGINFO_MK(file,line,func)
#   define DEBUGINFO_GEN     __DEBUGINFO_GEN
#   define DEBUGINFO_MUNUSED __DEBUGINFO_MUNUSED
#   define DEBUGINFO_UNUSED  __DEBUGINFO_UNUSED
#   define DEBUGINFO_FWD     __DEBUGINFO_FWD
#   define DEBUGINFO_NUL     __DEBUGINFO_NUL
#endif

__DECL_END

#endif /* !GUARD_HYBRID_DEBUGINFO_H */
