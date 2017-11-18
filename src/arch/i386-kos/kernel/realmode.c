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
#ifndef GUARD_KERNEL_ARCH_REALMODE_C
#define GUARD_KERNEL_ARCH_REALMODE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <arch/realmode.h>
#include <hybrid/align.h>
#include <hybrid/limits.h>
#include <kernel/memory.h>
#include <hybrid/panic.h>
#include <string.h>
#include <sys/syslog.h>
#include <hybrid/asm.h>
#include <asm/cpu-flags.h>
#include <arch/gdt.h>
#include <arch/cpustate.h>
#include <kernel/paging.h>
#include <dev/device.h>
#include <kernel/interrupt.h>
#include <hybrid/section.h>
#include <sched/task.h>
#include <kernel/mman.h>
#include <sys/mman.h>
#include <sched/paging.h>
#include <sched/cpu.h>
#include <asm/instx.h>
#include <arch/hints.h>

DECL_BEGIN

GLOBAL_ASM(
L(.code16                                         )
L(.section .realmode                              )
L(.hidden rm_stack                                )
L(.local rm_stack                                 )
L(rm_stack:                                       )
L(    .space REALMODE_STACK_SIZE                  )
L(.size rm_stack, . - rm_stack                    )
L(.previous                                       )
#ifdef __x86_64__
L(.code64                                         )
#else
L(.code32                                         )
#endif
);

GLOBAL_ASM(
L(RM_BEGIN                                        )
L(SYM_PRIVATE(rm_bios_int_begin)                  )
L(SYM_PRIVATE(rm_bios_int_num)                    )
L(SYM_PRIVATE(rm_bios_int)                        )
L(SYM_PRIVATE(rm_bios_int_end)                    )
L(rm_bios_int_begin:                              )
L(DEFINE_BSS(rm_bios_int_cpu,CPUSTATE16_SIZE)     )
L(DEFINE_BSS(rm_bios_int_cr0,__SIZEOF_REGISTER__) )
L(DEFINE_BSS(rm_bios_int_cr3,__SIZEOF_REGISTER__) )
L(DEFINE_BSS(rm_bios_int_esp,__SIZEOF_REGISTER__) )
L(DEFINE_BSS(rm_bios_int_gdt,IDT_POINTER_SIZE)    )
L(rm_bios_int:                                    )
#ifdef __x86_64__
L(    /* Compatibility mode with 16-bit code segment. */)
L(                                                )
L(    /* Disable paging & protected mode. */      )
L(    movl %cr0, %eax                             )
L(    RM_MOVL_M(eax, rm_bios_int_cr0)             )
L(    andl $(~(CR0_PE|CR0_PG)), %eax              )
L(    movl %eax, %cr0                             )
L(                                                )
L(    movl %cr3, %eax                             )
L(    RM_MOVL_M(eax, rm_bios_int_cr3)             )
L(    xorl %eax, %eax                             )
L(    movl %eax, %cr3                             )
L(                                                )
L(    RM_LJMPW(0,1f)                              )
L(1:  RM_LEAL(rm_bios_int_cpu, esp)               )
L(                                                )
L(    /* Load registers */                        )
L(    popal                                       )
L(    popw %gs                                    )
L(    popw %fs                                    )
L(    popw %es                                    )
L(    popw %ds                                    )
L(    popw %ss                                    )
L(    popfl                                       )
L(    RM_MOVL_R(rm_bios_int_cpu+CPUSTATE16_OFFSETOF_GP+GPREGS32_OFFSETOF_ESP,esp))
L(                                                )
L(    .byte 0xcd                                  )
L(rm_bios_int_num:                                )
L(    .byte 0x00                                  )
L(                                                )
L(    /* Save registers */                        )
L(    RM_MOVL_M(esp,rm_bios_int_cpu+CPUSTATE16_OFFSETOF_GP+GPREGS32_OFFSETOF_ESP))
L(    RM_LEAL(rm_bios_int_cpu+CPUSTATE16_SIZE,esp))
L(    pushfl                                      )
L(    pushw %ss                                   )
L(    pushw %ds                                   )
L(    pushw %es                                   )
L(    pushw %fs                                   )
L(    pushw %gs                                   )
L(    pushl %eax                                  )
L(    pushl %ecx                                  )
L(    pushl %edx                                  )
L(    pushl %ebx                                  )
L(    subl  $4, %esp /* Original SP */            )
L(    pushl %ebp                                  )
L(    pushl %esi                                  )
L(    pushl %edi                                  )
L(                                                )
L(    /* Disable interrupts again */              )
L(    cli                                         )
L(                                                )
L(    /* Re-enable paging & protected mode. */    )
L(    RM_MOVL_R(rm_bios_int_cr3, eax)             )
L(    movl %eax, %cr3                             )
L(    RM_MOVL_R(rm_bios_int_cr0, eax)             )
L(    movl %eax, %cr0                             )
L(                                                )
L(    ljmpl $(__KERNEL_CS), $(rm_bios_int_exit - ASM_CORE_BASE))
#else
#if 1 /* TODO: Disable this and test if real hardware still works. */
L(    movw $(SEG(SEG_HOST_DATA16)), %ax           )
L(    movw %ax, %ds                               )
L(    movw %ax, %es                               )
L(    movw %ax, %fs                               )
L(    movw %ax, %gs                               )
L(    movw %ax, %ss                               )
#endif
L(                                                )
L(    movl %cr0, %eax                             )
L(    andl $~(CR0_PG|CR0_PE), %eax                )
L(    movl %eax, %cr0                             )
L(                                                )
L(    RM_LJMPW(0,1f)                              )
L(1:  RM_LEAL(rm_bios_int_cpu, esp)               )
L(                                                )
L(    /* Load registers */                        )
L(    popal                                       )
L(    popw %gs                                    )
L(    popw %fs                                    )
L(    popw %es                                    )
L(    popw %ds                                    )
L(    popw %ss                                    )
L(    popfl                                       )
L(    RM_MOVL_R(rm_bios_int_cpu+CPUSTATE16_OFFSETOF_GP+GPREGS32_OFFSETOF_ESP,esp))
L(                                                )
L(    .byte 0xcd                                  )
L(rm_bios_int_num:                                )
L(    .byte 0x00                                  )
L(                                                )
L(    /* Save registers */                        )
L(    RM_MOVL_M(esp,rm_bios_int_cpu+CPUSTATE16_OFFSETOF_GP+GPREGS32_OFFSETOF_ESP))
L(    RM_LEAL(rm_bios_int_cpu+CPUSTATE16_SIZE,esp))
L(    pushfl                                      )
L(    pushw %ss                                   )
L(    pushw %ds                                   )
L(    pushw %es                                   )
L(    pushw %fs                                   )
L(    pushw %gs                                   )
L(    pushl %eax                                  )
L(    pushl %ecx                                  )
L(    pushl %edx                                  )
L(    pushl %ebx                                  )
L(    subl  $4, %esp /* Original SP */            )
L(    pushl %ebp                                  )
L(    pushl %esi                                  )
L(    pushl %edi                                  )
L(                                                )
L(    /* Disable interrupts again */              )
L(    cli                                         )
L(                                                )
L(    RM_MOVL_R(rm_bios_int_cr0, eax)             )
L(    andl $~(CR0_PG), %eax                       )
L(    movl %eax, %cr0                             )
L(                                                )
/* No need to temporarily switch to 16-bit protected mode.
 * >> We can just jump ahead into 32-bit. */
#if 0
L(    RM_LJMPW(SEG(SEG_HOST_CODE16),1f)           )
L(1:  movw $(SEG(SEG_HOST_DATA16)), %bx           )
L(    movw %bx, %ds                               )
L(    movw %bx, %es                               )
L(    movw %bx, %fs                               )
L(    movw %bx, %gs                               )
L(    movw %bx, %ss                               )
#endif
L(                                                )
L(    ljmpl $(__KERNEL_CS), $rm_bios_int_exit     )
#endif
L(rm_bios_int_end:                                )
L(.size rm_bios_int, . - rm_bios_int              )
L(.size rm_bios_int_end, 0                        )
L(RM_END                                          )
);

PRIVATE DEFINE_ATOMIC_RWLOCK(realmode_intlock);
INTDEF irq_t              rm_bios_int_num;
INTDEF struct cpustate16  rm_bios_int_cpu;
INTDEF register_t         rm_bios_int_cr0;
INTDEF register_t         rm_bios_int_cr3;
INTDEF register_t         rm_bios_int_esp;
INTDEF struct idt_pointer rm_bios_int_gdt;
INTERN struct idt_pointer rm_bios_int_idt = { .ip_limit = 0x03ff, .ip_idt = NULL, };

PRIVATE SAFE void KCALL
do_rm_interrupt(struct cpustate16 *__restrict state, irq_t intno) {
 struct mman *old_mm;
 /* NOTE: Need to switch to the kernel page-directory, because
  *       we need an environment in which there is a 1-on-1
  *       mapping between physical and virtual data (as is
  *       a requirement for safely disabling paging before
  *       switching to realmode)
  */
 TASK_PDIR_KERNEL_BEGIN(old_mm);
 *(struct cpustate16 *)REALMODE_SYM(rm_bios_int_cpu) = *state;
 *(irq_t *)REALMODE_SYM(rm_bios_int_num) = intno;
#define RM_SAFE_SEGMENTS
 __asm__ __volatile__(
#ifdef RM_SAFE_SEGMENTS
     L(    __ASM_IPUSH_SGREGS                          )
#endif
     L(    pushfx                                      )
     L(    cli                                         )
     L(    pushx %%xbp                                 )
     L(    subx $(2*XSZ), %%xsp                        )
     L(    sidt (%%xsp)                                )
     L(    call pic_bios_begin                         )
     L(    lidt rm_bios_int_idt                        )
     L(                                                )
#ifdef __x86_64__
     L(    ASM_RDFSBASE(r10)                           )
     L(    pushq %%r10                                 )
     L(    ASM_RDGSBASE(r10)                           )
     L(    pushq %%r10                                 )
     L(                                                )
     L(                                                )
     L(    movzwq realmode_base, %%rdi                 )
     L(    subq $__rm_core_start, %%rdi                )
     L(    movq %%rsp, rm_bios_int_esp(%%rdi)          ) /* Safe our original long-mode RSP */
     L(                                                )
     L(    sgdt rm_bios_int_gdt(%%rdi)                 )
     L(    subq $(ASM_CORE_BASE), rm_bios_int_gdt+2(%%rdi))
     L(    lgdt rm_bios_int_gdt(%%rdi)                 ) /* Reload our own GDT with a physical base address. */
     L(                                                )
     L(    pushq  $(SEG(SEG_HOST_CODE16))              )
     L(    leaq   rm_bios_int(%%rdi), %%rax            )
     L(    pushq  %%rax                                )
     L(    lretq                                       )
     L(SYM_PRIVATE(rm_bios_int_exit)                   )
     L(rm_bios_int_exit:                               )
     L(    movabs $1f, %%rax                           )
     L(    jmp    *%%rax                               )
     L(1:  movzwq realmode_base, %%rdi                 )
     L(    subq $__rm_core_start, %%rdi                )
     L(    movq rm_bios_int_esp(%%rdi), %%rsp          )
     L(                                                )
     L(    addq $(ASM_CORE_BASE), rm_bios_int_gdt+2(%%rdi))
     L(    lgdt rm_bios_int_gdt(%%rdi)                 ) /* Reload our own GDT with a virtual base address. */
     L(                                                )
     L(    popq %%r10                                  )
     L(    ASM_WRGSBASE(r10)                           )
     L(    popq %%r10                                  )
     L(    ASM_WRFSBASE(r10)                           )
#else
     L(                                                )
     L(    jmp 1f                                      )
     L(.section .text.phys                             )
     L(1:                                              )
     L(                                                )
     L(    movzwl realmode_base, %%edi                 )
     L(    subl $__rm_core_start, %%edi                )
     L(    movl %%esp, rm_bios_int_esp(%%edi)          )
     L(                                                )
     L(    sgdt rm_bios_int_gdt(%%edi)                 )
     L(    subl $(ASM_CORE_BASE), rm_bios_int_gdt+2(%%edi))
     L(    lgdt rm_bios_int_gdt(%%edi)                 )
     L(                                                )
     L(    movl %%cr0, %%eax                           )
     L(    movl %%eax, rm_bios_int_cr0(%%edi)          )
     L(    andl $~(CR0_PG), %%eax                      )
     L(    movl %%eax, %%cr0                           )
     L(    movl %%cr3, %%eax                           )
     L(    movl %%eax, rm_bios_int_cr3(%%edi)          )
     L(    movl $0, %%eax                              )
     L(    movl %%eax, %%cr3                           )
     L(                                                )
     L(    /* Jump into 16-bit code. */                )
     L(    /* ljmp $(SEG(SEG_HOST_CODE16)), rm_bios_int(%%edi) */)
     L(    addl $rm_bios_int, %%edi                    )
     L(    movl %%edi, 1f                              )
     L(    .byte 0xea                                  )
     L(1:  .long 0x00000000                            )
     L(    .word SEG(SEG_HOST_CODE16)                  )
     L(SYM_PRIVATE(rm_bios_int_exit)                   )
     L(rm_bios_int_exit:                               )
     L(    /* Returning from 16-bit code. */           )
     L(                                                )
     L(    /* Fix DS to ensure safe memory access. */  )
     L(    /* NOTE: QEMU doesn't break if we don't */  )
     L(    /*       do this, but real hardware     */  )
     L(    /*       triple-faults!                 */  )
     L(    movw  $(SEG(SEG_HOST_DATA)), %%ax           )
     L(    movw  %%ax, %%ds                            )
     L(                                                )
     L(    /* Reload where realmode code begins */     )
     L(    movzwl (realmode_base - ASM_CORE_BASE), %%edi)
     L(    subl $__rm_core_start, %%edi                )
     L(                                                )
     L(    movl rm_bios_int_cr3(%%edi), %%eax          )
     L(    movl %%eax, %%cr3                           )
     L(    movl rm_bios_int_cr0(%%edi), %%eax          )
     L(    movl %%eax, %%cr0                           )
     L(                                                )
     L(    jmp 1f                                      )
     L(.previous                                       )
     L(1:  addl $(ASM_CORE_BASE), rm_bios_int_gdt+2(%%edi))
     L(    lgdt rm_bios_int_gdt(%%edi)                 )
     L(                                                )
     /* TODO: The general segment register should be restored to their proper values immediatly,
      *       which means that the values saved at the start should be reloaded here, instead
      *       of what is currently happening with the double-reloading with the popw's below. */
     L(    movw  $(SEG(SEG_HOST_DATA)), %%ax           )
     L(    movw  $(SEG(SEG_CPUSELF)), %%bx             )
     L(    movw  %%ax, %%ds                            )
     L(    movw  %%ax, %%es                            )
#ifdef __i386__
     L(    movw  %%bx, %%fs                            )
     L(    movw  %%ax, %%gs                            )
#else
     L(    movw  %%ax, %%fs                            )
     L(    movw  %%bx, %%gs                            )
#endif
     L(    movw  %%ax, %%ss                            )
     L(                                                )
     L(    movl rm_bios_int_esp(%%edi), %%esp          )
#endif
     L(                                                )
     L(    lidt (%%xsp)                                )
     L(    addx $(2*XSZ), %%xsp                        )
     L(                                                )
     L(    call pic_bios_end                           )
     L(                                                )
     L(    popx %%xbp                                  )
     L(    popfx                                       )
#ifdef RM_SAFE_SEGMENTS
     L(    __ASM_IPOP_SGREGS                           )
#endif
     :
     :
#ifdef __x86_64__
     : "memory",   "rax", "rbx", "rcx", "rdx", "rsi", "rdi"
     , "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
#else
     : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi"
#endif
 );
 *state = *(struct cpustate16 *)REALMODE_SYM(rm_bios_int_cpu);
 TASK_PDIR_KERNEL_END(old_mm);
}

PUBLIC SAFE void KCALL
rm_interrupt(struct cpustate16 *__restrict state, irq_t intno) {
 atomic_owner_rwlock_read(&irqctl_lock);
 device_irqctl(IRQCTL_DISABLE);

 atomic_rwlock_write(&realmode_intlock);
 do_rm_interrupt(state,intno);
 atomic_rwlock_endwrite(&realmode_intlock);

 device_irqctl(IRQCTL_ENABLE);
 atomic_owner_rwlock_endread(&irqctl_lock);

 /* Due to the high chance that a PIT interrupt was
  * lost, manually preempt if we've allowed to.
  * >> Prevents soft-locking when the speed of interrupts
  *    match the speed at which the PIT should be running. */
 if (PREEMPTION_ENABLED())
     task_yield();
}


INTDEF byte_t rm_bios_int_begin[];
INTDEF byte_t rm_bios_int_end[];
PRIVATE ATTR_FREEBSS bool early_rm_loaded = false;
INTERN ATTR_FREETEXT SAFE void KCALL
early_rm_interrupt(struct cpustate16 *__restrict state, irq_t intno) {
 if (!early_rm_loaded) {
  /* Load only the RM-BIOS interrupt handler. */
  memcpy((void *)(REALMODE_STARTRELO),
         (void *)(rm_bios_int_begin),
         (size_t)(rm_bios_int_end-rm_bios_int_begin));
  syslog(LOG_DEBUG,FREESTR("[X86] Loaded early realmode interrupt code to %p...%p\n"),
        (uintptr_t)REALMODE_STARTRELO,
        (uintptr_t)REALMODE_STARTRELO+(size_t)(rm_bios_int_end-rm_bios_int_begin)-1);
  /* Setup the realmode so-as to indicate `rm_bios_int_begin' being loaded at `REALMODE_STARTRELO'. */
  realmode_base   = REALMODE_STARTRELO-(u16)(rm_bios_int_begin-__rm_core_start);
  early_rm_loaded = true;
 }
 do_rm_interrupt(state,intno);
}

struct reldescr {
 VIRT byte_t   *rd_virt_begin; /*< Virtual start address (in .free data) */
 VIRT u16      *rd_rel_begin;  /*< [1..1] Vector of relocation offsets from 'rd_virt_begin'. */
 VIRT u16      *rd_rel_end;    /*< [1..1] End of 'rd_rel_begin'. */
};

INTDEF byte_t rm_stack[];
INTDEF struct reldescr __rm_rel_start[];
INTDEF struct reldescr __rm_rel_end[];


/* Runtime-generated realmode base address. */
PUBLIC PHYS u16 realmode_base;
PUBLIC PHYS u16 realmode_stack;
#if 0
PUBLIC VIRT uintptr_t realmode_vbase;
PUBLIC VIRT uintptr_t realmode_vstack;
#endif

INTERN ATTR_FREETEXT void
KCALL realmode_initialize(void) {
 uintptr_t base; size_t alloc_size;
 alloc_size = (size_t)(__rm_core_end-__rm_core_start);
#ifdef REALMODE_PREFERRED
 base = (uintptr_t)page_malloc_at((ppage_t)FLOOR_ALIGN(REALMODE_PREFERRED,PAGESIZE),
                                   alloc_size+(REALMODE_PREFERRED & (PAGESIZE-1)),
                                   PAGEATTR_NONE);
 if (base != (uintptr_t)(void *)PAGE_ERROR) {
  base += (REALMODE_PREFERRED & (PAGESIZE-1));
  assert(base+alloc_size <= 0x10000);
 } else
#endif
 {
  base = (uintptr_t)page_malloc_in((ppage_t)FLOOR_ALIGN(0x00000000,PAGESIZE),
                                   (ppage_t)FLOOR_ALIGN(0x00010000-alloc_size,PAGESIZE),
                                    alloc_size,PAGEATTR_NONE);
  if unlikely(base == (uintptr_t)(void *)PAGE_ERROR)
     PANIC(FREESTR("Failed to allocate realmode data"));
 }
 realmode_base  = (u16)base;
 realmode_stack = (u16)REALMODE_SYM(rm_stack)+REALMODE_STACK_SIZE;
 /* Now execute relocations. */
 { struct reldescr *descr_iter,*descr_end;
   descr_iter = __rm_rel_start;
   descr_end  = __rm_rel_end;
   for (; descr_iter < descr_end; ++descr_iter) {
    u16 *iter,*end;
    u16 offset = (realmode_base - REALMODE_STARTRELO)+
            (u16)(descr_iter->rd_virt_begin - __rm_core_start);
    if (!offset) continue;
    iter = descr_iter->rd_rel_begin;
    end  = descr_iter->rd_rel_end;
    for (; iter < end; ++iter) {
#if 0
     syslog(LOG_MEM|LOG_INFO,"%p + %p\n",
            descr_iter->rd_virt_begin,*iter);
#endif
     *(u16 *)(descr_iter->rd_virt_begin+*iter) += offset;
    }
   }
 }
 /* Finally, copy all data to low memory. */
 memcpyl((void *)base,__rm_core_start,alloc_size/4);

#if 0
 /* Map the realmode segment into virtual kernel memory. */
 { struct mregion *rm_region; errno_t error;
   assert(task_isnointr());
   mman_write(&mman_kernel);
   alloc_size     = CEIL_ALIGN(alloc_size,PAGESIZE);
   realmode_vbase = (uintptr_t)mman_findspace_unlocked(&mman_kernel,
                                                      (ppage_t)(0-alloc_size),alloc_size,
                                                       PAGESIZE,0,MMAN_FINDSPACE_BELOW);
   if unlikely(realmode_vbase == (uintptr_t)(void *)PAGE_ERROR ||
               realmode_vbase <  (uintptr_t)(void *)KERNEL_BASE)
      PANIC(FREESTR("Failed to locate free memory from virtual realmode-segment mapping"));
   rm_region = mregion_new_phys(MMAN_DATAGFP(&mman_kernel),
                               (ppage_t)base,alloc_size);
   if unlikely(!rm_region)
      PANIC(FREESTR("Failed to allocate memory controller for realmode-segment mapping"));
   error = mman_mmap_unlocked(&mman_kernel,(ppage_t)realmode_vbase,alloc_size,
                              0,rm_region,PROT_READ|PROT_WRITE|PROT_NOUSER,
                              NULL,(void *)base);
   MREGION_DECREF(rm_region);
   if unlikely(E_ISERR(error))
      PANIC(FREESTR("Failed to map realmode-segment: %[errno]"),-error);
   mman_endwrite(&mman_kernel);

   realmode_vstack = realmode_vbase+(realmode_stack-realmode_base);
 }
 syslog(LOG_MEM|LOG_INFO,
        FREESTR("[X86] Relocating realmode segment to %.5I16X...%.5I16X (%p...%p)\n"),
       (u16)realmode_base,(u16)(realmode_base+alloc_size-1),
        realmode_vbase,(realmode_vbase+alloc_size-1));
#else
 syslog(LOG_MEM|LOG_INFO,
        FREESTR("[X86] Relocating realmode segment to %.5I16X...%.5I16X\n"),
       (u16)realmode_base,(u16)(realmode_base+alloc_size-1));
#endif
}


DECL_END

#endif /* !GUARD_KERNEL_ARCH_REALMODE_C */
