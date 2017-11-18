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
#ifndef GUARD_KERNEL_ARCH_SYSCALL_C
#define GUARD_KERNEL_ARCH_SYSCALL_C 1

#include <hybrid/compiler.h>
#include <arch/cpustate.h>
#include <arch/syscall.h>
#include <hybrid/asm.h>
#include <asm/instx.h>
#include <kernel/interrupt.h>
#include <kernel/syscall.h>

DECL_BEGIN

STATIC_ASSERT(offsetof(struct syscall,sc_number) == SYSCALL_OFFSETOF_NUMBER);
STATIC_ASSERT(offsetof(struct syscall,sc_type) == SYSCALL_OFFSETOF_TYPE);
STATIC_ASSERT(offsetof(struct syscall,sc_flags) == SYSCALL_OFFSETOF_FLAGS);
STATIC_ASSERT(offsetof(struct syscall,sc_callback) == SYSCALL_OFFSETOF_CALLBACK);
STATIC_ASSERT(offsetof(struct syscall,sc_closure) == SYSCALL_OFFSETOF_CLOSURE);
STATIC_ASSERT(offsetof(struct syscall,sc_hits) == SYSCALL_OFFSETOF_HITS);
STATIC_ASSERT(offsetof(struct syscall,sc_fini) == SYSCALL_OFFSETOF_FINI);
STATIC_ASSERT(offsetof(struct syscall,sc_owner) == SYSCALL_OFFSETOF_OWNER);
STATIC_ASSERT(sizeof(struct syscall) == SYSCALL_SIZE);

#define CONSTANT_FFS(x) \
 ((x) ==   2 ? 1 : (x) ==   4 ? 2 : (x) ==   8 ? 3 : \
  (x) ==  16 ? 4 : (x) ==  32 ? 5 : (x) ==  64 ? 6 : \
  (x) == 128 ? 7 : (x) == 256 ? 8 : (x) == 512 ? 9 : 0)

/* Assert that our ffs() function at least works for `SYSCALL_SIZE' */
STATIC_ASSERT(SYSCALL_SIZE == (1 << CONSTANT_FFS(SYSCALL_SIZE)));

#if CONSTANT_FFS(SYSCALL_SIZE) == 7
#   define SYSCALL_SIZE_FFS   7
#elif CONSTANT_FFS(SYSCALL_SIZE) == 6
#   define SYSCALL_SIZE_FFS   6
#elif CONSTANT_FFS(SYSCALL_SIZE) == 5
#   define SYSCALL_SIZE_FFS   5
#elif CONSTANT_FFS(SYSCALL_SIZE) == 4
#   define SYSCALL_SIZE_FFS   4
#else
#error FIME
#endif


/* System-call extension table. */
#define SYSCALL_TABLE_OFFSETOF_NEXT     0
#define SYSCALL_TABLE_OFFSETOF_LOCK     __SIZEOF_POINTER__
#define SYSCALL_TABLE_OFFSETOF_BEGIN (2*__SIZEOF_POINTER__)
#define SYSCALL_TABLE_OFFSETOF_END   (3*__SIZEOF_POINTER__)
#define SYSCALL_TABLE_OFFSETOF_TAB   (3*__SIZEOF_POINTER__+ATOMIC_RWLOCK_SIZE)
struct PACKED syscall_table {
    struct syscall_table *st_next;
    atomic_rwlock_t       st_lock;
    register_t            st_begin;
    register_t            st_end;
    struct syscall       *st_tab[1];
};

PRIVATE struct PACKED __syscall_extensions {
 struct syscall_table *head; /* [0..1][owned] System-call extension table. */
 atomic_rwlock_t       lock;
} syscall_extensions = { NULL, ATOMIC_RWLOCK_INIT };

STATIC_ASSERT(offsetof(struct __syscall_extensions,head) == SYSCALL_TABLE_OFFSETOF_NEXT);
STATIC_ASSERT(offsetof(struct __syscall_extensions,lock) == SYSCALL_TABLE_OFFSETOF_LOCK);

#define ASM_ATOMIC_RWLOCK_READ(lock)
#define ASM_ATOMIC_RWLOCK_ENDREAD(lock)


#ifdef __x86_64__
#define DS  /* nothing */
#else
/* Since the kernel's SS segment follows the flat memory model just like all
 * others, we can safely use it to access memory before loading all others.
 * >> This is a sort-of hack to work around having to save/load user-space
 *    segments before doing so really becomes necessary. */
#define DS  %ss:
#endif

GLOBAL_ASM(
L(.section .text.hot                                                          )
#ifdef CONFIG_DEBUG
L(1:  int    $3 /* ASSERTION_FAILURE: syscall doesn't originate from kernel-space */)
L(2:  cli;   hlt; jmp 2b                                                      )
L(INTERN_ENTRY(syscall_interrupt_handler)                                     )
L(    testx  $3, DS IRREGS_OFFSETOF_CS(%xsp)                                  )
L(    jz 1b                                                                   )
#else
L(INTERN_ENTRY(syscall_interrupt_handler)                                     )
#endif
#ifdef __x86_64__
L(    swapgs                                                                  )
L(    sti    /* Enable interrupts once the kernel segment has been loaded. */ )
#endif
L(                                                                            )
L(    /* TODO: Fast-callahead to builtin system calls. */                     )
L(                                                                            )
L(                                                                            )
L(    pushx %xsi                                                              )
L(    pushx %xdi                                                              )
L(    leax_rel(syscall_extensions, %xsi)                                      )
#define PREV   %xsi
#define CURR   %xdi
L(    ASM_ATOMIC_RWLOCK_READ(SYSCALL_TABLE_OFFSETOF_LOCK(PREV))               )
L(.cont:                                                                      )
L(    movx  DS SYSCALL_TABLE_OFFSETOF_NEXT(PREV), CURR                        )
L(    testx CURR, CURR                                                        )
L(    jz    .bad                                                              )
L(                                                                            )
L(    /* Acquire a lock to the next extension table. */                       )
L(    ASM_ATOMIC_RWLOCK_READ   (SYSCALL_TABLE_OFFSETOF_LOCK(CURR))            )
L(    ASM_ATOMIC_RWLOCK_ENDREAD(SYSCALL_TABLE_OFFSETOF_LOCK(PREV))            )
L(                                                                            )
L(    cmpx  DS SYSCALL_TABLE_OFFSETOF_BEGIN(CURR), %xax                       )
L(    jb    .next /* if (XAX < XSI->st_begin) goto next; */                   )
L(    cmpx  DS SYSCALL_TABLE_OFFSETOF_END(CURR),   %xax                       )
L(    jae   .next /* if (XAX >= XSI->st_end) goto next; */                    )
L(                                                                            )
#if 1
L(    /* All right! We've found the associated system-call extension table! */)
#define SYSCALL  %xax
L(    subx  DS SYSCALL_TABLE_OFFSETOF_BEGIN(%xsi),         SYSCALL            )
#if SYSCALL_SIZE_FFS != 0
L(    shlx  $(SYSCALL_SIZE_FFS),                           SYSCALL            )
#endif
L(    movx  DS SYSCALL_TABLE_OFFSETOF_TAB(%xsi,SYSCALL,1), SYSCALL            )
L(                                                                            )
L(    ASM_ATOMIC_RWLOCK_ENDREAD(SYSCALL_TABLE_OFFSETOF_LOCK(CURR))            )
L(    popx  %xdi                                                              )
L(    popx  %xsi                                                              )
L(                                                                            )
L(    /* Trace the number of times this system-call was run. */               )
#ifdef __x86_64__
L(    incq  DS SYSCALL_OFFSETOF_HITS(%xax)                                    )
#else
L(    addl  $1, DS SYSCALL_OFFSETOF_HITS(%eax)                                )
L(    adcl  $1, DS SYSCALL_OFFSETOF_HITS+4(%eax)                              )
#endif
L(                                                                            )
L(    /* Test for `SYSCALL_TYPE_FAST' system calls. */                        )
L(    testb $(SYSCALL_TYPE_FAST&SYSCALL_TYPE_MASK), DS SYSCALL_OFFSETOF_TYPE(%xax))
L(    jz    3f                                                                )
L(                                                                            )
L(    /* Run a `SYSCALL_TYPE_FAST'-compatible system call. */                 )
L(                                                                            )
#ifdef __x86_64__
L(    addq  $8, %rsp /* Pop orig_eax */                                       )
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs                                                                  )
L(    ASM_IRET                                                                )
#else
L(    addl  $4, %esp /* Pop orig_eax */                                       )
L(    ASM_IRET                                                                )
#endif
L(                                                                            )
L(3:  testb $(SYSCALL_TYPE_STATE&SYSCALL_TYPE_MASK), DS SYSCALL_OFFSETOF_TYPE(%xax))
L(    jz    3f                                                                )
L(                                                                            )
L(    /* Run a `SYSCALL_TYPE_STATE_ARG'-compatible system call. */            )
L(    pushx %xcx                                                              )
L(    pushx %xdx                                                              )
L(    pushx %xbx                                                              )
#ifndef __x86_64__
L(    pushx %xsp  /* This is basically just padding... */                     )
#endif
L(    pushx %xbp                                                              )
L(    pushx %xsi                                                              )
L(    pushx %xdi                                                              )
#ifdef __x86_64__
L(    /* Save x86_64-specific registers. */                                   )
L(    pushq %r8                                                               )
L(    pushq %r9                                                               )
L(    pushq %r10                                                              )
L(    pushq %r11                                                              )
L(    pushq %r12                                                              )
L(    pushq %r13                                                              )
L(    pushq %r14                                                              )
L(    pushq %r15                                                              )
#endif
L(    /* Load the closure now because EAX may be clobbered by the segment-safe code */)
L(    movx SYSCALL_OFFSETOF_CLOSURE(%xax), %FASTCALL_REG2 /* closure */       )
#ifdef __x86_64__
L(    ASM_RDFSBASE(r10); pushq %r10;                                          )
L(    /* Special case: Since we've already switched to the kernel's GS segment,
       *               we must still make sure to push the user's here, meaning
       *               we can't just use `__ASM_PUSH_SGREGS'. */                                                                     )
L(    movl  $(IA32_KERNEL_GS_BASE), %ecx /* Currently, this is the user-gc-base */)
L(    rdmsr                                                                   )
L(    pushq %rax          /* Lower 32 bits (+ invalid upper 32 bits). */      )
L(    movl  %edx, 4(%rsp) /* Upper 32 bits. */                                )
#else
L(    __ASM_PUSH_SGREGS                                                       )
L(    __ASM_LOAD_SEGMENTS(%dx)                                                )
#endif
L(                                                                            )
L(    /* At this point, we've created a full `struct cpustate' */             )
L(    movx  %xsp, %FASTCALL_REG1 /* state */                                  )
L(                                                                            )
L(    /* Actually invoke the system-call. */                                  )
L(    callx *SYSCALL_OFFSETOF_CALLBACK(%xax)                                  )
L(                                                                            )
L(    __ASM_POP_COMREGS                                                       )
#ifdef __x86_64__
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs                                                                  )
#endif
L(    ASM_IRET                                                                )
L(                                                                            )
L(3:  /* Run a `SYSCALL_TYPE_STATE_ARG'-compatible system call. */            )
L(    callx *SYSCALL_OFFSETOF_CALLBACK(%xax)                                  )
L(99: addx  $(XSZ),  %xsp /* Pop orig_eax */                                  )
#ifdef __x86_64__
L(    cli    /* Prevent race-condition involving `swapgs' */                  )
L(    swapgs                                                                  )
#endif
L(    ASM_IRET                                                                )
#endif
L(                                                                            )
L(.next:                                                                      )
L(    movx  CURR, PREV                                                        )
L(    jmp   .cont                                                             )
L(.bad:                                                                       )
L(    /* Invalid system call number... */                                     )
L(    ASM_ATOMIC_RWLOCK_ENDREAD(SYSCALL_TABLE_OFFSETOF_LOCK(PREV))            )
L(    popx  %xdi                                                              )
L(    popx  %xsi                                                              )
L(    /* XXX: raise(SIGSYS)? */                                               )
L(    movx  $(-ENOSYS), %xax /* return -ENOSYS; */                            )
L(    jmp   .cont                                                             )
L(SYM_END(syscall_interrupt_handler)                                          )
L(.previous                                                                   )
);



/* The system-call interrupt handler. */
INTDEF void ASMCALL syscall_interrupt_handler(void);
PRIVATE struct interrupt syscall_interrupt = {
    .i_intno = INTNO_SYSCALL,
    /* Leave interrupts enabled on i386, but disable them on x86_64.
     * This is required due to a race condition that could otherwise
     * occur if a hardware interrupt is triggered before `swapgs' loads
     * the kernel's internal segments. */
#ifdef __x86_64__
    .i_mode  = (IDTFLAG_PRESENT|IDTTYPE_80386_32_INTERRUPT_GATE|IDTFLAG_DPL(3)),
#else
    .i_mode  = (IDTFLAG_PRESENT|IDTTYPE_80386_32_TRAP_GATE|IDTFLAG_DPL(3)),
#endif
    .i_type  =  INTTYPE_ASM,     /* The system call interrupt handler is implemented in assembly. */
    .i_prio  =  INTPRIO_MAX,     /* System call interrupts cannot be shared... */
    .i_flags =  INTFLAG_PRIMARY, /* ... or be overwritten. */
    .i_proto = {
        .p_asm = &syscall_interrupt_handler,
    },
};



DECL_END

#endif /* !GUARD_KERNEL_ARCH_SYSCALL_C */
