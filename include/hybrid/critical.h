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
#ifndef __GUARD_HYBRID_CRITICAL_H
#define __GUARD_HYBRID_CRITICAL_H 1

#include <__stdinc.h>

#if defined(__CC__) && (defined(__KERNEL__) && !defined(__INTELLISENSE__))
__SYSDECL_BEGIN
__PUBDEF      __BOOL (__KCALL task_issafe)(void);
__PUBDEF      __BOOL (__KCALL task_iscrit)(void);
__PUBDEF        void (__KCALL task_crit)(void);
__PUBDEF __SAFE void (__KCALL task_endcrit)(void);
__SYSDECL_END
#   define TASK_ISSAFE()    task_issafe() /* task_iscrit() || !PREEMPTION_ENABLED() */
#   define TASK_ISCRIT()    task_iscrit()
#   define TASK_CRIT()      task_crit()
#   define TASK_ENDCRIT()   task_endcrit()
#else
#   define TASK_ISSAFE()    1 /* Not really, but satisfies assertions. */
#   define TASK_ISCRIT()    0
#   define TASK_CRIT()     (void)0
#   define TASK_ENDCRIT()  (void)0
#endif


#endif /* !__GUARD_HYBRID_CRITICAL_H */
