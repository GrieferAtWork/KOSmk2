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
#ifndef GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_SYSCALL_H
#define GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_SYSCALL_H 1

#include <hybrid/compiler.h>
#include <arch/cpustate.h>
#include <asm/unistd.h>

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Required for extended system call hacks. */
#include <hybrid/asm.h>
#endif

DECL_BEGIN

#undef CONFIG_HAVE_SYSCALL_LONGBIT
#define CONFIG_HAVE_SYSCALL_LONGBIT 1

#define NR_syscalls     (__NR_syscall_max+1)

/* The calling convention used by all high-level system call handlers. */
#define SYSCALL_HANDLER       /* nothing */
/* The calling convention used for `SYSCALL_TYPE_STATE' and `SYSCALL_TYPE_STATE_ARG' */
#define SYSCALL_STATE_HANDLER /* nothing */


/* System-call definitions macros:
 *  - __SYSCALL_NDEFINE: Define a ~normal~ c-level system call handler.
 *  - __SYSCALL_LDEFINE: Define a c-level system call handler that returns double the data (May not be defined).
 *  - __SYSCALL_SDEFINE: Define a c-level system call handler that takes a full `struct cpustate *' as argument.
 */
#ifdef CONFIG_BUILDING_KERNEL_CORE

#define __SYSCALL_LDEFINE(visibility,n,name,args) \
  LOCAL s64 (SYSCALL_HANDLER SYSC##name)(__SC_DECL##n args); \
  visibility syscall_slong_t (SYSCALL_HANDLER sys##name)(__SC_LONG##n args) { \
    register s64 __result = SYSC##name(__SC_CAST##n args); \
    /* TODO */ \
    return (syscall_slong_t)__result; \
  } \
  LOCAL s64 (SYSCALL_HANDLER SYSC##name)(__SC_DECL##n args)
#define __SYSCALL_SDEFINE(visibility,name,state) \
  GLOBAL_ASM(L(.section .text                                                              ) \
             L(visibility##_ENTRY(sys##name)                                               ) \
             L(    /* TODO */                                                              ) \
             L(SYM_END(sys##name)                                                          ) \
             L(.previous                                                                   )); \
  INTERN void (SYSCALL_STATE_HANDLER SYSC##name)(struct cpustate *__restrict state)
#define __SYSCALL_DEFINE64  __SYSCALL_LDEFINE


#ifdef __CC__
/* Initialize system-call facilities. */
INTDEF INITCALL void KCALL syscall_initialize(void);
#endif /* __CC__ */
#endif /* CONFIG_BUILDING_KERNEL_CORE */

DECL_END

#endif /* !GUARD_ARCH_ARM_KOS_INCLUDE_ARCH_SYSCALL_H */
