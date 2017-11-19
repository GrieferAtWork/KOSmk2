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
#ifndef GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_PREEMPTION_H
#define GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_PREEMPTION_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>

DECL_BEGIN

#ifndef __pflag_t_defined
#define __pflag_t_defined 1
typedef register_t pflag_t; /* Push+disable/Pop preemption-enabled. */
#endif /* !__pflag_t_defined */

/* Thread/CPU-local preemption control. */
#define PREEMPTION_ENABLE()  (void)0 /* TODO */
#define PREEMPTION_DISABLE() (void)0 /* TODO */
#define PREEMPTION_ENABLED()       0 /* TODO */
#define PREEMPTION_IDLE()    (void)0 /* TODO */
#define PREEMPTION_FREEZE()  XBLOCK({ for (;;) {} }) /* TODO */

/* Relax the calling CPU. */
#define cpu_relax()          (void)0 /* TODO */

#define PREEMPTION_PUSH()           0  /* TODO */
#define PREEMPTION_PUSHON()         0  /* TODO */
#define PREEMPTION_POP(f)    (void)(f) /* TODO */

DECL_END

#endif /* !GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_PREEMPTION_H */
