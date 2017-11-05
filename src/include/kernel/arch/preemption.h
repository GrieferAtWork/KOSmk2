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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_PREEMPTION_H
#define GUARD_INCLUDE_KERNEL_ARCH_PREEMPTION_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>

DECL_BEGIN

#ifndef __pflag_t_defined
#define __pflag_t_defined 1
typedef register_t pflag_t; /* Push+disable/Pop preemption-enabled. */
#endif /* !__pflag_t_defined */

/* Thread/CPU-local preemption control. */
#define PREEMPTION_ENABLE()  XBLOCK({ __asm__ __volatile__("sti"); (void)0; })
#define PREEMPTION_DISABLE() XBLOCK({ __asm__ __volatile__("cli"); (void)0; })
#ifdef __x86_64__
#define PREEMPTION_ENABLED() XBLOCK({ register register_t _efl; __asm__ __volatile__("pushfq\npopq %0" : "=g" (_efl)); XRETURN !!(_efl&0x00000200); })
#else
#define PREEMPTION_ENABLED() XBLOCK({ register register_t _efl; __asm__ __volatile__("pushfl\npopl %0" : "=g" (_efl)); XRETURN !!(_efl&0x00000200); })
#endif
#define PREEMPTION_IDLE()    XBLOCK({ __asm__ __volatile__("hlt\n" : : : "memory"); })
#define PREEMPTION_FREEZE()  XBLOCK({ __asm__ __volatile__("1: cli\nhlt\njmp 1b" : : : "memory"); __builtin_unreachable(); (void)0; })

/* Relax the calling CPU. */
#define cpu_relax()   XBLOCK({ __asm__("pause"); (void)0; })

#ifdef __x86_64__
#define PREEMPTION_PUSH()   XBLOCK({ register pflag_t _r; __asm__ __volatile__("pushfq\npopq %0\ncli" : "=g" (_r)); XRETURN _r; })
#define PREEMPTION_PUSHON() XBLOCK({ register pflag_t _r; __asm__ __volatile__("pushfq\npopq %0\nsti" : "=g" (_r)); XRETURN _r; })
#define PREEMPTION_POP(f)   XBLOCK({ __asm__ __volatile__("pushq %0\npopfq\n" : : "g" (f) : "cc"); (void)0; })
#else /* __x86_64__ */
#define PREEMPTION_PUSH()   XBLOCK({ register pflag_t _r; __asm__ __volatile__("pushfl\npopl %0\ncli" : "=g" (_r)); XRETURN _r; })
#define PREEMPTION_PUSHON() XBLOCK({ register pflag_t _r; __asm__ __volatile__("pushfl\npopl %0\nsti" : "=g" (_r)); XRETURN _r; })
#define PREEMPTION_POP(f)   XBLOCK({ __asm__ __volatile__("pushl %0\npopfl\n" : : "g" (f) : "cc"); (void)0; })
#endif /* !__x86_64__ */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_PREEMPTION_H */
