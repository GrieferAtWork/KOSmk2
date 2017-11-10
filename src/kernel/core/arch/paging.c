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
#ifndef GUARD_KERNEL_CORE_ARCH_PAGING_C
#define GUARD_KERNEL_CORE_ARCH_PAGING_C 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>

#ifdef __x86_64__
#include "paging64.c.inl"
#else
#include "paging32.c.inl"
#endif
#include <kernel/paging.h>
#include <kernel/mman.h>
#include <hybrid/types.h>
#include <format-printer.h>
#include <asm/instx.h>
#include <kernel/arch/hints.h>
#include <kernel/export.h>

DECL_BEGIN

#ifdef __x86_64__
/* Page directory entries for mapping the last 2Gb of virtual memory to the first physical 2. */
INTERN ATTR_ALIGNED(PAGESIZE) ATTR_SECTION(".bss") e3_t coreboot_e2_80000000[PDIR_E2_COUNT];
INTERN ATTR_ALIGNED(PAGESIZE) ATTR_SECTION(".bss") e3_t coreboot_e2_c0000000[PDIR_E2_COUNT];
INTERN ATTR_ALIGNED(PAGESIZE) ATTR_SECTION(".bss") e4_t coreboot_e3[PDIR_E3_COUNT]
#if 0
= {
    [PDIR_E3_COUNT-2] = { ((uintptr_t)coreboot_e2_80000000 - CORE_BASE)+(PDIR_ATTR_GLOBAL|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT) },
    [PDIR_E3_COUNT-1] = { ((uintptr_t)coreboot_e2_c0000000 - CORE_BASE)+(PDIR_ATTR_GLOBAL|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT) },
}
#endif
;
#endif

INTERN ATTR_SECTION(".data.phys")
       ATTR_ALIGNED(MMAN_ALIGN)
PHYS struct mman __mman_kernel_p = {
    /* After boot, memory is mapped like this:
     * >> 0x00000000..0xbfffffff  -->  0x00000000..0xbfffffff
     * >> 0xc0000000..0xffffffff  -->  0x00000000..0x3fffffff
     * Aka: Physical memory pointers is mirrored in upper memory.
     * Once paging has been fully initialized, the lower
     * mappings from 0x00000000 to 0xbfffffff are deleted.
     * HINT: 0x83 == PDIR_ATTR_4MIB|PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE */
#ifndef __INTELLISENSE__
    .m_pdir = {
        .pd_directory = {
#ifdef __x86_64__
            /* The x86_64 bootstrap code identity-maps the first Gb of physical memory
             * within a level #4 entry past the physical end of the kernel core.
             * Here, we simply hook those entries so that the bootstrap process is not
             * required to re-write the associated page directory entries afterwards.
             * NOTE: This assumes that the kernel core won't ever be larger than
             *       1Gb, but I think that's a pretty safe assumption to make (right?) */
            [0] = { ((uintptr_t)KERNEL_END - CORE_BASE)+(PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT) },
            [PDIR_E4_COUNT-1] = { ((uintptr_t)coreboot_e3 - CORE_BASE)+(PDIR_ATTR_GLOBAL|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT) },
#else /* __x86_64__ */
#define PD(phys) \
           {phys+0x00000083},{phys+0x00400083},{phys+0x00800083},{phys+0x00c00083}, \
           {phys+0x01000083},{phys+0x01400083},{phys+0x01800083},{phys+0x01c00083}, \
           {phys+0x02000083},{phys+0x02400083},{phys+0x02800083},{phys+0x02c00083}, \
           {phys+0x03000083},{phys+0x03400083},{phys+0x03800083},{phys+0x03c00083}, \
           {phys+0x04000083},{phys+0x04400083},{phys+0x04800083},{phys+0x04c00083}, \
           {phys+0x05000083},{phys+0x05400083},{phys+0x05800083},{phys+0x05c00083}, \
           {phys+0x06000083},{phys+0x06400083},{phys+0x06800083},{phys+0x06c00083}, \
           {phys+0x07000083},{phys+0x07400083},{phys+0x07800083},{phys+0x07c00083},
            PD(0x00000000) PD(0x08000000) PD(0x10000000) PD(0x18000000) /* 0x18000000 */
            PD(0x20000000) PD(0x28000000) PD(0x30000000) PD(0x38000000) /* 0x38000000 */
            PD(0x40000000) PD(0x48000000) PD(0x50000000) PD(0x58000000) /* 0x58000000 */
            PD(0x60000000) PD(0x68000000) PD(0x70000000) PD(0x78000000) /* 0x78000000 */
            PD(0x80000000) PD(0x88000000) PD(0x90000000) PD(0x98000000) /* 0x98000000 */
            PD(0xa0000000) PD(0xa8000000) PD(0xb0000000) PD(0xb8000000) /* 0xb8000000 */
            /* Map lower memory a second time. (NOTE: 0x100 is `PDIR_ATTR_GLOBAL') */
            PD(0x00000100) PD(0x08000100) PD(0x10000100) PD(0x18000100) /* 0xd8000000 */
            PD(0x20000100) PD(0x28000100) PD(0x30000100) PD(0x38000100) /* 0xf8000000 */
#if CORE_BASE != 0xc0000000
#error "FIXME: Fix the bootstrap page table above"
#endif
#undef PD
#endif /* !__x86_64__ */
        },
    },
#endif /* !__INTELLISENSE__ */
    .m_ppdir  = &pdir_kernel,
    /* Reference counter:
     *   - mman_kernel
     *   - __boottask.t_mman
     *   - __bootcpu.c_idle.t_mman
     *   - [!CONFIG_NO_JOBS] __bootcpu.c_work.t_mman
     */
#ifndef CONFIG_NO_JOBS
#ifdef CONFIG_DEBUG
    .m_refcnt = 4,
#else
    .m_refcnt = 0x80000004,
#endif
#else /* !CONFIG_NO_JOBS */
#ifdef CONFIG_DEBUG
    .m_refcnt = 3,
#else
    .m_refcnt = 0x80000003,
#endif
#endif /* CONFIG_NO_JOBS */
    .m_lock   = OWNER_RWLOCK_INIT,
    .m_tasks_lock = ATOMIC_RWLOCK_INIT,
    .m_tasks  = &inittask,
    .m_inst   = THIS_INSTANCE,
#ifndef CONFIG_NO_LDT
    .m_ldt    = &ldt_empty,
#endif
    .m_uheap  = (ppage_t)USER_HEAP_ADDRHINT,
    .m_ustck  = (ppage_t)USER_STCK_ADDRHINT,
#ifndef CONFIG_NO_VM_EXE
    .m_exe    = THIS_INSTANCE, /* The kernel is its own executable. - mind=blown; */
#endif /* !CONFIG_NO_VM_EXE */
};

GLOBAL_ASM(
/* Define physical/virtual versions of the kernel mman/pdir. */
L(.global mman_kernel, pdir_kernel, pdir_kernel_v                   )
L(mman_kernel   = __mman_kernel_p+ASM_CORE_BASE                   )
L(pdir_kernel   = __mman_kernel_p+MMAN_OFFSETOF_PDIR                )
L(pdir_kernel_v = __mman_kernel_p+ASM_CORE_BASE+MMAN_OFFSETOF_PDIR)
);



struct pdr_print_pck {
 pformatprinter printer;
 void          *closure;
};
PRIVATE ssize_t KCALL
pdir_println(VIRT ppage_t v_addr, PHYS ppage_t p_addr,
             size_t n_bytes, pdir_attr_t attr, void *data) {
 if (!(attr&PDIR_ATTR_PRESENT) && (uintptr_t)p_addr == ~PDIR_ATTR_MASK) return 0;
 if ((uintptr_t)p_addr+n_bytes < (uintptr_t)p_addr)
      n_bytes = 0-(uintptr_t)p_addr;
 return format_printf(((struct pdr_print_pck *)data)->printer,
                      ((struct pdr_print_pck *)data)->closure,
                      "%p...%p --> %p...%p [%c%c%c] (%.3Ix)\n",
                      v_addr,(uintptr_t)v_addr+n_bytes-1,
                      p_addr,(uintptr_t)p_addr+n_bytes-1,
                      attr&PDIR_ATTR_USER ? 'U' : '-',
                      attr&PDIR_ATTR_WRITE ? 'W' : '-',
                      attr&PDIR_ATTR_PRESENT ? 'P' : '-',
                      attr);
}

PUBLIC ssize_t KCALL
pdir_print(pdir_t *__restrict self,
           pformatprinter printer,
           void *closure) {
 struct pdr_print_pck pck;
 pck.printer = printer;
 pck.closure = closure;
 return pdir_enum(self,&pdir_println,&pck);
}



INTDEF byte_t pdir_flush_386[];
INTDEF byte_t pdir_flush_386_end[];

GLOBAL_ASM(
L(.section .text.free                                                         )
L(PRIVATE_ENTRY(pdir_flush_386)                                               )
L(    movx %cr3, %xax                                                         )
L(    movx %xax, %cr3                                                         )
L(    ret                                                                     )
L(SYM_END(pdir_flush_386)                                                     )
L(pdir_flush_386_end:                                                         )
L(.previous                                                                   )
);

GLOBAL_ASM(
L(.section .text                                                              )
L(PUBLIC_ENTRY(pdir_flush)                                                    )
/* NOTE: When the host is a 386, we'll copy data from `pdir_flush_386' here. */
L(    cmpx   $(PAGESIZE*256), %FASTCALL_REG2                                  )
L(    jae    1f                                                               )
L(    invlpg (%FASTCALL_REG1)                                                 )
L(    subx   $(PAGESIZE), %FASTCALL_REG2                                      )
L(    jo     2f  /* Stop if n_bytes underflowed. */                           )
L(    addx   $(PAGESIZE), %FASTCALL_REG1                                      )
L(    jmp    pdir_flush                                                       )
L(1:  movx   %cr3, %xax  /* Do a regular, full flush for larger quantities. */)
L(    movx   %xax, %cr3                                                       )
L(2:  ret                                                                     )
L(SYM_END(pdir_flush)                                                         )
L(.previous                                                                   )
);

INTERN ATTR_FREETEXT void KCALL pdir_initialize(void) {
 assert(addr_isphys(&pdir_kernel));
 assert(addr_isvirt(&pdir_kernel_v));
 assert(addr_isvirt(&mman_kernel));

 if (!(THIS_CPU->c_arch.ac_flags&CPUFLAG_486)) {
  /* Replace `pdir_flush' with its fallback counterpart!
   * (The `invlpg' instruction is only available starting at 486) */
  memcpy((void *)&pdir_flush,pdir_flush_386,
         (size_t)(pdir_flush_386_end-pdir_flush_386));
 }

#ifdef PDIR_KERNEL_REMAP_EARLY_IDENTITY
 /* Remap all early identity mappings in general purpose physical memory. */
 PDIR_KERNEL_REMAP_EARLY_IDENTITY();
#endif

#ifdef PDIR_KERNEL_MAP_IDENTITY
 /* Create identity mappings for all physical memory below KERNEL_BASE. */
 PDIR_KERNEL_MAP_IDENTITY();
#endif

#ifdef PDIR_KERNEL_TRANSFORM_TABLES
 /* Transform kernel tables, ensuring level#1 indirection for
  * pages, which in return allow for page sharing with user-space. */
 PDIR_KERNEL_TRANSFORM_TABLES();
#endif

#ifdef CONFIG_PDIR_SELFMAP
#ifdef PDIR_KERNEL_INITIALIZE_SELFMAP
 /* With all page-tables allocated, setup the
  * page-directory self-map for the kernel itself. */
 PDIR_KERNEL_INITIALIZE_SELFMAP();
#endif
#endif /* CONFIG_PDIR_SELFMAP */

#ifdef PDIR_KERNEL_UNMAP_UNUSED
 /* Time to clean up the kernel's own page directory! */
 PDIR_KERNEL_UNMAP_UNUSED();
#endif

#ifdef PDIR_KERNEL_UNMAP_AFTEREND
 /* Unmap all virtual memory past the kernel core. */
 PDIR_KERNEL_UNMAP_AFTEREND();
#endif

#ifdef PDIR_KERNEL_UNMAP_BEFOREBEGIN
 /* Unmap the virtual copy of physical memory before device data.
  * NOTE: Device memory itself must not be unmapped,
  *       as the TTY driver wouldn't work otherwise. */
 PDIR_KERNEL_UNMAP_BEFOREBEGIN();
#endif

#ifdef PDIR_KERNEL_CHECK_INTEGRITY_AFTER_SETUP
 /* Make sure that all virtual kernel pages are still allocated! */
 PDIR_KERNEL_CHECK_INTEGRITY_AFTER_SETUP();
#endif

#if 1
 /* Change the protection of the kernel's text/rodata segment to read-only. */
 { ssize_t error;
   error = pdir_mprotect(&pdir_kernel,
                        (ppage_t)KERNEL_RO_BEGIN,KERNEL_RO_SIZE,
                         PDIR_ATTR_PRESENT|PDIR_ATTR_GLOBAL|PDIR_FLAG_NOFLUSH);
   if (E_ISERR(error)) {
    syslog(LOG_MEM|LOG_ERROR,
           FREESTR("[PD] Failed to mark kernel text %p...%p as read-only: %[errno]\n"),
           KERNEL_RO_BEGIN,KERNEL_RO_END-1,-error);
   }
#if 1
   /* Make the kernel's user-share segment readable from user-space. */
   error = pdir_mprotect(&pdir_kernel,
                        (ppage_t)__kernel_user_start,(size_t)__kernel_user_size,
                         PDIR_ATTR_USER|PDIR_ATTR_GLOBAL|PDIR_ATTR_PRESENT|PDIR_FLAG_NOFLUSH);
   if (E_ISERR(error)) {
    syslog(LOG_MEM|LOG_ERROR,
           FREESTR("[PD] Failed to mark user-share %p...%p as readable: %[errno]\n"),
          (uintptr_t)__kernel_user_start,(uintptr_t)__kernel_user_end-1,-error);
   }
#endif
 }
#endif

 /* Reload the kernel page directory completely. */
 pdir_flushall();

 assert(PDIR_TRANSLATE(&pdir_kernel_v,&pdir_kernel_v) == &pdir_kernel);
 assert(PDIR_TRANSLATE(&pdir_kernel  ,&pdir_kernel_v) == &pdir_kernel);
}

DECL_END

#endif /* !GUARD_KERNEL_CORE_ARCH_PAGING_C */
