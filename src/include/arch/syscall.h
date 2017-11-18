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
#ifndef GUARD_INCLUDE_ARCH_SYSCALL_H
#define GUARD_INCLUDE_ARCH_SYSCALL_H 1

#include <hybrid/compiler.h>
#include <arch/cpustate.h>
#include <hybrid/list/list.h>
#include <asm/unistd.h>

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Required for extended system call hacks. */
#include <hybrid/asm.h>
#include <asm/registers.h>
#endif

DECL_BEGIN

#undef CONFIG_HAVE_SYSCALL_LONGBIT
#if !defined(__x86_64__) || 0 /* XXX: Enable if we ever need a system call that returns 128 bits. */
#define CONFIG_HAVE_SYSCALL_LONGBIT 1
#endif


#define NR_syscalls     (__NR_syscall_max+1)

/* The calling convention used by all
 * high-level system call handlers. */
#ifdef __x86_64__
#define SYSCALL_HANDLER  ATTR_SYSVABI
#else
#define SYSCALL_HANDLER  ATTR_CDECL
#endif
/* The calling convention used for `SYSCALL_TYPE_STATE' and `SYSCALL_TYPE_STATE_ARG' */
#define SYSCALL_STATE_HANDLER FCALL


#define IRREGS_SYSCALL_OFFSETOF_ORIG_XAX  0
#define IRREGS_SYSCALL_OFFSETOF_ORIG_EAX  0
#define IRREGS_SYSCALL_OFFSETOF_ORIG_AX   0
#define IRREGS_SYSCALL_OFFSETOF_ORIG_AL   0
#define IRREGS_SYSCALL_OFFSETOF_ORIG_AH   1
#define IRREGS_SYSCALL_OFFSETOF_SYSNO     0
#ifdef __x86_64__
#define IRREGS_SYSCALL_OFFSETOF_ORIG_RAX  0
#define IRREGS_SYSCALL_OFFSETOF_TAIL      8 /* +IRREGS_OFFSETOF_* */
#define IRREGS_SYSCALL_OFFSETOF_HOST      8 /* +IRREGS_HOST_OFFSETOF_* */
#define IRREGS_SYSCALL_OFFSETOF_IP        8
#define IRREGS_SYSCALL_OFFSETOF_CS        16
#define IRREGS_SYSCALL_OFFSETOF_FLAGS     24
#define IRREGS_SYSCALL_OFFSETOF_USERSP    32
#define IRREGS_SYSCALL_OFFSETOF_SS        40
#define IRREGS_SYSCALL_SIZE               48
#else
#define IRREGS_SYSCALL_OFFSETOF_TAIL      4 /* +IRREGS_OFFSETOF_* */
#define IRREGS_SYSCALL_OFFSETOF_HOST      4 /* +IRREGS_HOST_OFFSETOF_* */
#define IRREGS_SYSCALL_OFFSETOF_IP        4
#define IRREGS_SYSCALL_OFFSETOF_CS        8
#define IRREGS_SYSCALL_OFFSETOF_FLAGS     12
#define IRREGS_SYSCALL_OFFSETOF_USERSP    16
#define IRREGS_SYSCALL_OFFSETOF_SS        20
#define IRREGS_SYSCALL_SIZE               24
#endif

#ifdef __CC__
/* Common system call IRET tail. (Common stack-base data-structure used by all system calls) */
struct PACKED irregs_syscall { union PACKED { __COMMON_REG1_EX(orig_,a); register_t sysno; };
                               union PACKED { struct irregs tail; struct PACKED {
                               union PACKED { struct irregs_host host; struct PACKED {
                               __COMMON_REG2(ip); IRET_SEGMENT(cs); __COMMON_REG2(flags); };};
                               __COMMON_REG2_EX(user,sp); IRET_SEGMENT(ss); };};};

/* Return a pointer to the effective system-call IRREGS-IRET tail,
 * assuming that the calling thread is currently inside a system-call.
 * HINT: This data structure can be modified to override system-call
 *       the system-call return address, a behavior that is used by
 *       KOS's POSIX-signal() implementation to capture user-space
 *       register values. */
#define IRREGS_SYSCALL_GET()         (((struct irregs_syscall *)THIS_CPU->c_arch.ac_tss.xsp0)-1)
#define IRREGS_SYSCALL_GET_FOR(task) (((struct irregs_syscall *)(task)->t_hstack.hs_end)-1)

/* Standardized: The current system-call return address. */
#ifdef __x86_64__
#define THIS_SYSCALL_GETIP() ((void *)IRREGS_SYSCALL_GET()->rip)
#else
#define THIS_SYSCALL_GETIP() ((void *)IRREGS_SYSCALL_GET()->eip)
#endif

#endif /* __CC__ */



/* System-call defintions macros:
 *  - __SYSCALL_NDEFINE: Define a ~normal~ c-level system call handler.
 *  - __SYSCALL_LDEFINE: Define a c-level system call handler that returns double the data (May not be defined).
 *  - __SYSCALL_SDEFINE: Define a c-level system call handler that takes a full `struct cpustate *' as argument.
 */
#ifdef CONFIG_BUILDING_KERNEL_CORE
#if 0
#include <syslog.h>
#define __SYSCALL_NDEFINE(visibility,n,name,args) \
  LOCAL syscall_slong_t (SYSCALL_HANDLER SYSC##name)(__SC_DECL##n args); \
  visibility syscall_slong_t (SYSCALL_HANDLER sys##name)(__SC_LONG##n args) { \
    __SC_TEST##n args; syscall_slong_t __res; \
    __res = (SYSC##name)(__SC_CAST##n args); \
    syslog(LOG_DEBUG,"[SYSCALL] END sys" #name "() -> %p\n",(void *)__res); \
    return __res; \
  } \
  LOCAL syscall_slong_t (SYSCALL_HANDLER SYSC##name)(__SC_DECL##n args)
#else
#define __SYSCALL_NDEFINE(visibility,n,name,args) \
  LOCAL syscall_slong_t (SYSCALL_HANDLER SYSC##name)(__SC_DECL##n args); \
  visibility syscall_slong_t (SYSCALL_HANDLER sys##name)(__SC_LONG##n args) { \
    __SC_TEST##n args; \
    return (SYSC##name)(__SC_CAST##n args); \
  } \
  LOCAL syscall_slong_t (SYSCALL_HANDLER SYSC##name)(__SC_DECL##n args)
#endif

/* Because the kernel doesn't make use of system-call extensions, but rather
 * hard-codes it's system calls, optimizing them for default usage, we need
 * some wrapper magic.
 * WARNING: Keep this in sync with how `syscall_interrupt_handler' operates!
 *          This relies on some very intricate implementation details of that
 *          assembly-implemented function.
 */
#ifdef __x86_64__

/* NEVER USE THE FOLLOWING MACROS FOR ANYTHING BUT THE HACKS USED IN THIS HEADER! */
#define __PRIVATE_IRREGS_SYSCALL_GET_RCX()  (*(u64 *)((u64)IRREGS_SYSCALL_GET()-(__ASM_SCRATCH_NOXAX_SIZE-__ASM_SCRATCH_NOXAX_OFFSETOF_RCX)))
#define __PRIVATE_IRREGS_SYSCALL_GET_RDX()  (*(u64 *)((u64)IRREGS_SYSCALL_GET()-(__ASM_SCRATCH_NOXAX_SIZE-__ASM_SCRATCH_NOXAX_OFFSETOF_RDX)))
#define __PRIVATE_IRREGS_SYSCALL_GET_RSI()  (*(u64 *)((u64)IRREGS_SYSCALL_GET()-(__ASM_SCRATCH_NOXAX_SIZE-__ASM_SCRATCH_NOXAX_OFFSETOF_RSI)))
#define __PRIVATE_IRREGS_SYSCALL_GET_RDI()  (*(u64 *)((u64)IRREGS_SYSCALL_GET()-(__ASM_SCRATCH_NOXAX_SIZE-__ASM_SCRATCH_NOXAX_OFFSETOF_RDI)))
#define __PRIVATE_IRREGS_SYSCALL_GET_R8()   (*(u64 *)((u64)IRREGS_SYSCALL_GET()-(__ASM_SCRATCH_NOXAX_SIZE-__ASM_SCRATCH_NOXAX_OFFSETOF_R8)))
#define __PRIVATE_IRREGS_SYSCALL_GET_R9()   (*(u64 *)((u64)IRREGS_SYSCALL_GET()-(__ASM_SCRATCH_NOXAX_SIZE-__ASM_SCRATCH_NOXAX_OFFSETOF_R9)))
#define __PRIVATE_IRREGS_SYSCALL_GET_R10()  (*(u64 *)((u64)IRREGS_SYSCALL_GET()-(__ASM_SCRATCH_NOXAX_SIZE-__ASM_SCRATCH_NOXAX_OFFSETOF_R10)))
#define __PRIVATE_IRREGS_SYSCALL_GET_R11()  (*(u64 *)((u64)IRREGS_SYSCALL_GET()-(__ASM_SCRATCH_NOXAX_SIZE-__ASM_SCRATCH_NOXAX_OFFSETOF_R11)))

#define __SYSCALL_LDEFINE(visibility,n,name,args) \
  LOCAL __int128 (SYSCALL_HANDLER SYSC##name)(__SC_DECL##n args); \
  visibility syscall_slong_t (SYSCALL_HANDLER sys##name)(__SC_LONG##n args) { \
    __SC_TEST##n args; \
    register __int128 __result = SYSC##name(__SC_CAST##n args); \
    /* Save the upper 64 bits in RDX (By overwriting the caller's scratch area) */ \
    __PRIVATE_IRREGS_SYSCALL_GET_RDX() = (u64)((unsigned __int128)__result >> 32); \
    return (syscall_slong_t)__result; \
  } \
  LOCAL __int128 (SYSCALL_HANDLER SYSC##name)(__SC_DECL##n args)
#ifdef CONFIG_DEBUG
#define __PRIVATE_SYSCALL_RSPOFF  24 /* RAX, RIP, RBP; For tracebacks. */
#else
#define __PRIVATE_SYSCALL_RSPOFF  8  /* RAX */
#endif
#ifdef CONFIG_DEBUG
#define __SYSCALL_SDEFINE(visibility,name,state) \
  GLOBAL_ASM(L(.section .text                                                              ) \
             L(visibility##_ENTRY(sys##name)                                               ) \
             L(    addq  $(__PRIVATE_SYSCALL_RSPOFF), %rsp /* Adjust stack offset */       ) \
             L(    __ASM_SCRACH_XSP_TO_GPREGS(%rcx)                                        ) \
             L(    __ASM_PUSH_SGREGS                                                       ) \
             L(    movq  %rsp, %rdi /* `struct cpustate *state' */                         ) \
             L(    pushq CPUSTATE_OFFSETOF_IRET+IRREGS_OFFSETOF_IP(%rdi)                   ) \
             L(    pushq CPUSTATE_OFFSETOF_GP+GPREGS_OFFSETOF_RBP(%rdi)                    ) \
             L(    movq  %rsp, %rbp                                                        ) \
             L(    call  SYSC##name                                                        ) \
             L(    addq  $16,  %rsp                                                        ) \
             L(    cli                                                                     ) \
             L(    swapgs                                                                  ) \
             L(    __ASM_POP_COMREGS                                                       ) \
             L(    ASM_IRET                                                                ) \
             L(SYM_END(sys##name)                                                          ) \
             L(.previous                                                                   )); \
  PRIVATE ATTR_USED void (SYSCALL_STATE_HANDLER SYSC##name)(struct cpustate *__restrict state)
#else
#define __SYSCALL_SDEFINE(visibility,name,state) \
  GLOBAL_ASM(L(.section .text                                                              ) \
             L(visibility##_ENTRY(sys##name)                                               ) \
             L(    addq  $(__PRIVATE_SYSCALL_RSPOFF), %rsp /* Adjust stack offset */       ) \
             L(    __ASM_SCRACH_XSP_TO_GPREGS(%rcx)                                        ) \
             L(    __ASM_PUSH_SGREGS                                                       ) \
             L(    movq  %rsp, %rdi /* `struct cpustate *state' */                         ) \
             L(    call  SYSC##name                                                        ) \
             L(    cli                                                                     ) \
             L(    swapgs                                                                  ) \
             L(    __ASM_POP_COMREGS                                                       ) \
             L(    ASM_IRET                                                                ) \
             L(SYM_END(sys##name)                                                          ) \
             L(.previous                                                                   )); \
  PRIVATE ATTR_USED void (SYSCALL_STATE_HANDLER SYSC##name)(struct cpustate *__restrict state)
#endif

#else /* __x86_64__ */

/* NEVER USE THE FOLLOWING MACROS FOR ANYTHING BUT THE HACKS USED IN THIS HEADER! */
#ifdef CONFIG_DEBUG
#define __PRIVATE_IRREGS_SYSCALL_GET_EIP()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+4)))
#define __PRIVATE_IRREGS_SYSCALL_GET_EBP()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+8)))
#define __PRIVATE_IRREGS_SYSCALL_GET_EDI()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+12)))
#define __PRIVATE_IRREGS_SYSCALL_GET_ESI()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+16)))
#define __PRIVATE_IRREGS_SYSCALL_GET_EDX()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+20)))
#define __PRIVATE_IRREGS_SYSCALL_GET_ECX()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+24)))
#define __PRIVATE_IRREGS_SYSCALL_GET_EBX()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+28)))
#else
#define __PRIVATE_IRREGS_SYSCALL_GET_EBP()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+4)))
#define __PRIVATE_IRREGS_SYSCALL_GET_EDI()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+8)))
#define __PRIVATE_IRREGS_SYSCALL_GET_ESI()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+12)))
#define __PRIVATE_IRREGS_SYSCALL_GET_EDX()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+16)))
#define __PRIVATE_IRREGS_SYSCALL_GET_ECX()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+20)))
#define __PRIVATE_IRREGS_SYSCALL_GET_EBX()  (*(u32 *)((u32)IRREGS_SYSCALL_GET()-(SGREGS_SIZE+24)))
#endif

#define __SYSCALL_LDEFINE(visibility,n,name,args) \
  LOCAL s64 (SYSCALL_HANDLER SYSC##name)(__SC_DECL##n args); \
  visibility syscall_slong_t (SYSCALL_HANDLER sys##name)(__SC_LONG##n args) { \
    register s64 __result = SYSC##name(__SC_CAST##n args); \
    /* Save the upper 32 bits in EDX (By overwriting the caller's scratch area) */ \
    __PRIVATE_IRREGS_SYSCALL_GET_EDX() = (u32)((u64)__result >> 32); \
    return (syscall_slong_t)__result; \
  } \
  LOCAL s64 (SYSCALL_HANDLER SYSC##name)(__SC_DECL##n args)

#ifdef CONFIG_DEBUG
/* i386 system-call stack transformation:
 *   0(%esp):   u32 ---  -> ebp  -
 *   4(%esp):   u32 ---  -> eip  -
 *   8(%esp):   u32 ebx  -> sg_a -
 *  12(%esp):   u32 ecx  -> sg_b -
 *  16(%esp):   u32 edx  -> edi  -
 *  20(%esp):   u32 esi  -> esi  KEEP
 *  24(%esp):   u32 edi  -> ebp  -
 *  28(%esp):   u32 ebp  -> esp  IGNORE
 *  32(%esp):   u32 eip  -> ebx  -
 *  36(%esp):   u32 sg_a -> edx  -
 *  40(%esp):   u32 sg_b -> ecx  -
 *  44(%esp):   u32 eax  -> eax  KEEP
 */
#if SGREGS_SIZE != 8 /* Make sure that the transformation below can work. */
#error "Must adjust temporary register storage below!"
#endif
#define __SYSCALL_SDEFINE(visibility,name,state) \
  GLOBAL_ASM(L(.section .text                                                              ) \
             L(visibility##_ENTRY(sys##name)                                               ) \
             L(/* This is a bit complicated, so I'd suggest to look at the digram above. */) \
             L(    movl 28-4(%esp), %eax; pushl %eax         /* ebp */                     ) \
             L(                         xchgl %eax, 24(%esp) /* ebp -> edi */              ) \
             L(                         xchgl %eax, 16(%esp) /* edi -> edx */              ) \
             L(                         xchgl %eax, 36(%esp) /* edx -> sg_a */             ) \
             L(                         xchgl %eax,  8(%esp) /* sg_a -> ebx */             ) \
             L(                         xchgl %eax, 32(%esp) /* ebx -> eip */              ) \
             L(                         movl  %eax,  4(%esp) /* eip */                     ) \
             L(    movl 12(%esp), %eax; xchgl %eax, 40(%esp) /* ecx -> sg_b */             ) \
             L(                         movl  %eax, 12(%esp) /* sg_b */                    ) \
             L(    leal  8(%esp), %ecx /* `struct cpustate *state' */                      ) \
             L(    call  SYSC##name                                                        ) \
             L(    addl  $8, %esp /* SKIP EBP, EIP  */                                     ) \
             L(    __ASM_POP_COMREGS                                                       ) \
             L(    ASM_IRET                                                                ) \
             L(SYM_END(sys##name)                                                          ) \
             L(.previous                                                                   )); \
  INTERN void (SYSCALL_STATE_HANDLER SYSC##name)(struct cpustate *__restrict state)
#else /* CONFIG_DEBUG */
/* i386 system-call stack transformation:
 *   0(%esp):   u32 ---  -> sg_a -
 *   4(%esp):   u32 ebx  -> sg_b -
 *   8(%esp):   u32 ecx  -> edi  -
 *  12(%esp):   u32 edx  -> esi  -
 *  16(%esp):   u32 esi  -> ebp  -
 *  20(%esp):   u32 edi  -> esp  IGNORE
 *  24(%esp):   u32 ebp  -> ebx  -
 *  28(%esp):   u32 sg_a -> edx  -
 *  32(%esp):   u32 sg_b -> ecx  -
 *  36(%esp):   u32 eax  -> eax  KEEP
 */
#if SGREGS_SIZE != 8 /* Make sure that the transformation below can work. */
#error "Must adjust temporary register storage below!"
#endif
#define __SYSCALL_SDEFINE(visibility,name,state) \
  GLOBAL_ASM(L(.section .text                                                              ) \
             L(visibility##_ENTRY(sys##name)                                               ) \
             L(/* This is a bit complicated, so I'd suggest to look at the digram above. */) \
             L(    pushl 28(%esp)                            /* sg_a */                    ) \
             L(    movl 32(%esp), %eax; xchgl %eax,  4(%esp) /* sg_b -> ebx */             ) \
             L(                         xchgl %eax, 24(%esp) /* ebx -> ebp */              ) \
             L(                         xchgl %eax, 16(%esp) /* ebp -> esi */              ) \
             L(                         xchgl %eax, 12(%esp) /* esi -> edx */              ) \
             L(                         movl  %eax, 28(%esp) /* edx */                     ) \
             L(    movl  8(%esp), %eax; movl  %eax, 32(%esp) /* ecx */                     ) \
             L(    movl 20(%esp), %eax; movl  %eax,  8(%esp) /* edi */                     ) \
             L(                                                                            ) \
             L(    leal  8(%esp), %ecx /* `struct cpustate *state' */                      ) \
             L(    call  SYSC##name                                                        ) \
             L(    __ASM_POP_COMREGS                                                       ) \
             L(    ASM_IRET                                                                ) \
             L(SYM_END(sys##name)                                                          ) \
             L(.previous                                                                   )); \
  INTERN void (SYSCALL_STATE_HANDLER SYSC##name)(struct cpustate *__restrict state)
#endif /* !CONFIG_DEBUG */
#endif /* !__x86_64__ */

#ifdef __x86_64__
#define __SYSCALL_DEFINE64  __SYSCALL_NDEFINE
#else
#define __SYSCALL_DEFINE64  __SYSCALL_LDEFINE
#endif


#ifdef __CC__
/* Initialize system-call facilities. */
INTDEF INITCALL void KCALL syscall_initialize(void);
#endif /* __CC__ */
#endif /* CONFIG_BUILDING_KERNEL_CORE */

DECL_END

#endif /* !GUARD_INCLUDE_ARCH_SYSCALL_H */
